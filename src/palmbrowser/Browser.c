/*
 * Browser - a small text-mode web browser for PumpkinOS.
 *
 * Fetches pages over HTTP (and HTTPS when PumpkinOS has a TLS provider
 * loaded), strips the HTML down to formatted text with tappable links,
 * and renders it in a scrollable page area.
 */

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include <PalmOS.h>

#include "sys.h"
#include "filter.h"
#include "secure.h"
#include "httpc.h"
#include "ptr.h"
#include "mutex.h"
#include "xalloc.h"
#include "pumpkin.h"
#include "debug.h"

#include "resource.h"

#define MAX_URL         512
#define MAX_TITLE       128
#define MAX_HISTORY     32
#define MAX_RESPONSE    (1024*1024)
#define MAX_WORDLEN        256
#define MAX_LINKS       30000
#define FETCH_TIMEOUT   30          /* seconds */
#define MAX_REDIRECTS   5
#ifndef DEFAULT_HOME
#define DEFAULT_HOME    "about:start"
#endif
#define FROGFIND_READ   "http://frogfind.com/read.php?a="
#define PREFS_ID        1
#define PREFS_VERSION   1
#define APP_TAG         "Browser"

static const char START_PAGE[] =
  "<h1>Browser</h1>"
  "<p>A small text browser for PumpkinOS. Type an address above and tap Go, or start here:</p>"
  "<ul>"
  "<li><a href=\"http://frogfind.com/\">FrogFind</a> - search engine and text proxy for old browsers</li>"
  "<li><a href=\"http://68k.news/\">68k.news</a> - news headlines in basic HTML</li>"
  "<li><a href=\"http://wiby.me/\">Wiby</a> - search engine for simple pages</li>"
  "<li><a href=\"http://theoldnet.com/\">theoldnet.com</a> - browse archived web pages</li>"
  "<li><a href=\"http://textfiles.com/\">textfiles.com</a></li>"
  "<li><a href=\"http://info.cern.ch/\">info.cern.ch</a> - the first web site</li>"
  "</ul>"
  "<p>Only http:// pages can be fetched unless PumpkinOS has a TLS provider loaded. "
  "For an https:// page use Page &gt; Open via FrogFind, which fetches it through an HTTP text proxy.</p>"
  "<p>Tap a link to follow it. Scroll with the scrollbar or the page up/down keys. Use Page &gt; Set as Home to make the current page your start page.</p>";

#define LINK_NONE       -1
#define DRAG_THRESHOLD  3           /* pixels of pen movement before a tap becomes a drag */
#define ITEM_GLUE       0x01        /* no space before this word */

typedef enum { itemWord = 0, itemBreak, itemPara } item_kind_t;

typedef struct {
  char home[MAX_URL];
} browser_prefs_t;

/* result of one HTTP request; filled in by the network thread */
typedef struct {
  mutex_t *mutex;
  volatile int done;
  volatile int abandoned;
  int code;
  int chunked;
  int error;
  char location[MAX_URL];
  char contentType[128];
  char *body;
  int len;
} fetch_t;

/* document model: a flat list of words and breaks */
typedef struct {
  UInt32 off;
  UInt16 len;
  UInt8 kind;
  UInt8 font;
  UInt8 flags;
  Int16 link;
} item_t;

/* layout model: lines made of positioned text segments */
typedef struct {
  Coord x;
  UInt32 off;
  UInt16 len;
  UInt8 font;
  Int16 link;
} seg_t;

typedef struct {
  Coord y;
  Coord h;
  UInt32 firstSeg;
  UInt16 nsegs;
} line_t;

typedef struct {
  browser_prefs_t prefs;
  char url[MAX_URL];
  char base[MAX_URL];
  char title[MAX_TITLE];
  char history[MAX_HISTORY][MAX_URL];
  int nhistory;

  char *text;    UInt32 textLen,  textSize;
  item_t *items; UInt32 nitems,   itemsSize;
  char **links;  UInt32 nlinks,   linksSize;

  seg_t *segs;   UInt32 nsegs,    segsSize;
  line_t *lines; UInt32 nlines,   linesSize;
  UInt32 topLine;
  RectangleType area;

  mutex_t *mutex;
  int abandonedFetches;
  Boolean stop;

  /* pen drag scrolling */
  Boolean dragging;
  Boolean dragMoved;
  Coord dragStartX, dragStartY;
  UInt32 dragStartTop;
  Coord stdLineHeight;
} browser_t;

/* parser state */
typedef struct {
  browser_t *b;
  char word[MAX_WORDLEN];
  int wlen;
  UInt8 font;
  Int16 link;
  int pendingSpace;
  int inPre;
  int inTitle;
  int bold;
  int heading;
  int lastBr;
  int tlen;
} parser_t;

/*
 * Small helpers
 */

static Boolean isSpaceChar(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static Boolean isAlphaChar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static Boolean isDigitChar(char c) {
  return c >= '0' && c <= '9';
}

static char lowerChar(char c) {
  return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

static Boolean strEq(const char *a, const char *b) {
  return StrCompare(a, b) == 0;
}

static void *growArray(void *p, UInt32 *size, UInt32 need, UInt32 elemSize) {
  UInt32 newSize;

  if (need <= *size) return p;
  newSize = *size ? *size : 64;
  while (newSize < need) newSize *= 2;
  p = xrealloc(p, newSize * elemSize);
  *size = newSize;

  return p;
}

static void catStr(char *out, int size, int *pos, const char *s, int n) {
  int i;

  for (i = 0; i < n && s[i] && *pos < size - 1; i++) {
    out[(*pos)++] = s[i];
  }
  out[*pos] = 0;
}

/*
 * Document
 */

static void docReset(browser_t *b) {
  UInt32 i;

  for (i = 0; i < b->nlinks; i++) {
    if (b->links[i]) xfree(b->links[i]);
  }
  b->nlinks = 0;
  b->textLen = 0;
  b->nitems = 0;
  b->nsegs = 0;
  b->nlines = 0;
  b->topLine = 0;
  b->title[0] = 0;
}

static void docFree(browser_t *b) {
  docReset(b);
  if (b->text) xfree(b->text);
  if (b->items) xfree(b->items);
  if (b->links) xfree(b->links);
  if (b->segs) xfree(b->segs);
  if (b->lines) xfree(b->lines);
  b->text = NULL;
  b->items = NULL;
  b->links = NULL;
  b->segs = NULL;
  b->lines = NULL;
  b->textSize = b->itemsSize = b->linksSize = b->segsSize = b->linesSize = 0;
}

static Int16 docAddLink(browser_t *b, const char *href) {
  if (b->nlinks >= MAX_LINKS) return LINK_NONE;
  b->links = growArray(b->links, &b->linksSize, b->nlinks + 1, sizeof(char *));
  b->links[b->nlinks] = xstrdup(href);
  return (Int16)b->nlinks++;
}

static void docAddItem(browser_t *b, UInt8 kind, const char *s, UInt16 len, UInt8 font, Int16 link, UInt8 flags) {
  item_t *it;

  b->items = growArray(b->items, &b->itemsSize, b->nitems + 1, sizeof(item_t));
  it = &b->items[b->nitems++];
  it->kind = kind;
  it->font = font;
  it->link = link;
  it->flags = flags;
  it->off = b->textLen;
  it->len = len;

  if (len) {
    b->text = growArray(b->text, &b->textSize, b->textLen + len, 1);
    MemMove(b->text + b->textLen, s, len);
    b->textLen += len;
  }
}

/* drop trailing line breaks */
static void docTrim(browser_t *b) {
  while (b->nitems > 0 && b->items[b->nitems - 1].kind != itemWord) b->nitems--;
}

/* insert a line break; a paragraph break leaves a blank line */
static void docBreak(browser_t *b, Boolean para) {
  item_t *last;

  if (b->nitems == 0) return;
  last = &b->items[b->nitems - 1];
  if (last->kind == itemPara) return;
  if (last->kind == itemBreak) {
    if (para) last->kind = itemPara;
    return;
  }
  docAddItem(b, para ? itemPara : itemBreak, NULL, 0, stdFont, LINK_NONE, 0);
}

/*
 * Character conversion
 */

/* write a Unicode code point as Latin-1-ish bytes, returns count (0..3) */
static int putCodepoint(UInt32 cp, char *out) {
  if (cp == 0xA0) { out[0] = ' '; return 1; }
  if (cp < 0x100) { out[0] = (char)cp; return 1; }

  switch (cp) {
    case 0x2010: case 0x2011: case 0x2012: case 0x2013: case 0x2014: case 0x2212:
      out[0] = '-'; return 1;
    case 0x2018: case 0x2019: case 0x201A: case 0x2032:
      out[0] = '\''; return 1;
    case 0x201C: case 0x201D: case 0x201E: case 0x2033:
      out[0] = '"'; return 1;
    case 0x2022: case 0x25E6: case 0x2023:
      out[0] = '*'; return 1;
    case 0x2026:
      out[0] = out[1] = out[2] = '.'; return 3;
    case 0x2122:
      out[0] = 'T'; out[1] = 'M'; return 2;
    case 0x20AC:
      out[0] = 'E'; out[1] = 'U'; out[2] = 'R'; return 3;
    case 0x2190:
      out[0] = '<'; out[1] = '-'; return 2;
    case 0x2192:
      out[0] = '-'; out[1] = '>'; return 2;
    case 0x200B: case 0x200C: case 0x200D: case 0xFEFF: case 0x00AD:
      return 0;
    default:
      out[0] = '?'; return 1;
  }
}

/* convert UTF-8 to Latin-1 in place; invalid sequences are passed through */
static int utf8ToLatin1(char *s, int len) {
  int in = 0, out = 0, n, i, k;
  UInt32 cp;
  unsigned char c;
  char tmp[4];
  Boolean valid;

  while (in < len) {
    c = (unsigned char)s[in];
    if (c < 0x80) {
      s[out++] = (char)c;
      in++;
      continue;
    }
    if ((c & 0xE0) == 0xC0) { n = 1; cp = c & 0x1F; }
    else if ((c & 0xF0) == 0xE0) { n = 2; cp = c & 0x0F; }
    else if ((c & 0xF8) == 0xF0) { n = 3; cp = c & 0x07; }
    else { s[out++] = (char)c; in++; continue; }

    valid = (in + n) < len;
    for (i = 1; valid && i <= n; i++) {
      if (((unsigned char)s[in + i] & 0xC0) != 0x80) {
        valid = false;
      } else {
        cp = (cp << 6) | ((unsigned char)s[in + i] & 0x3F);
      }
    }
    if (!valid) {
      s[out++] = (char)c;
      in++;
      continue;
    }
    k = putCodepoint(cp, tmp);
    for (i = 0; i < k; i++) s[out++] = tmp[i];
    in += n + 1;
  }

  return out;
}

typedef struct {
  const char *name;
  UInt16 code;
} entity_t;

static const entity_t entities[] = {
  { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' },
  { "nbsp", 0xA0 }, { "shy", 0xAD }, { "iexcl", 0xA1 }, { "cent", 0xA2 }, { "pound", 0xA3 }, { "yen", 0xA5 },
  { "sect", 0xA7 }, { "copy", 0xA9 }, { "laquo", 0xAB }, { "reg", 0xAE }, { "deg", 0xB0 },
  { "plusmn", 0xB1 }, { "para", 0xB6 }, { "middot", 0xB7 }, { "raquo", 0xBB }, { "frac14", 0xBC },
  { "frac12", 0xBD }, { "frac34", 0xBE }, { "iquest", 0xBF }, { "times", 0xD7 }, { "divide", 0xF7 },
  { "Agrave", 0xC0 }, { "Aacute", 0xC1 }, { "Acirc", 0xC2 }, { "Auml", 0xC4 }, { "Ccedil", 0xC7 },
  { "Egrave", 0xC8 }, { "Eacute", 0xC9 }, { "Ecirc", 0xCA }, { "Euml", 0xCB }, { "Iuml", 0xCF },
  { "Ntilde", 0xD1 }, { "Ouml", 0xD6 }, { "Oslash", 0xD8 }, { "Uuml", 0xDC }, { "szlig", 0xDF },
  { "agrave", 0xE0 }, { "aacute", 0xE1 }, { "acirc", 0xE2 }, { "atilde", 0xE3 }, { "auml", 0xE4 },
  { "aring", 0xE5 }, { "aelig", 0xE6 }, { "ccedil", 0xE7 }, { "egrave", 0xE8 }, { "eacute", 0xE9 },
  { "ecirc", 0xEA }, { "euml", 0xEB }, { "igrave", 0xEC }, { "iacute", 0xED }, { "icirc", 0xEE },
  { "iuml", 0xEF }, { "ntilde", 0xF1 }, { "ograve", 0xF2 }, { "oacute", 0xF3 }, { "ocirc", 0xF4 },
  { "ouml", 0xF6 }, { "oslash", 0xF8 }, { "ugrave", 0xF9 }, { "uacute", 0xFA }, { "ucirc", 0xFB },
  { "uuml", 0xFC }, { "yuml", 0xFF },
  { "ndash", 0x2013 }, { "mdash", 0x2014 }, { "lsquo", 0x2018 }, { "rsquo", 0x2019 },
  { "ldquo", 0x201C }, { "rdquo", 0x201D }, { "bull", 0x2022 }, { "hellip", 0x2026 },
  { "trade", 0x2122 }, { "euro", 0x20AC }, { "larr", 0x2190 }, { "rarr", 0x2192 },
  { NULL, 0 }
};

/*
 * HTML parser
 */

static void flushWord(parser_t *p) {
  if (p->wlen == 0) return;
  docAddItem(p->b, itemWord, p->word, (UInt16)p->wlen, p->font, p->link, p->pendingSpace ? 0 : ITEM_GLUE);
  p->wlen = 0;
  p->pendingSpace = 0;
  p->lastBr = 0;
}

static void addTitleChar(parser_t *p, char c) {
  browser_t *b = p->b;

  if (isSpaceChar(c)) {
    if (p->tlen > 0 && b->title[p->tlen - 1] != ' ' && p->tlen < MAX_TITLE - 1) {
      b->title[p->tlen++] = ' ';
      b->title[p->tlen] = 0;
    }
  } else if (p->tlen < MAX_TITLE - 1) {
    b->title[p->tlen++] = c;
    b->title[p->tlen] = 0;
  }
}

static void addChar(parser_t *p, char c) {
  if (p->inTitle) {
    addTitleChar(p, c);
    return;
  }
  if (p->wlen >= MAX_WORDLEN - 1) {
    flushWord(p);
  }
  p->word[p->wlen++] = c;
}

static void addSpace(parser_t *p) {
  if (p->inTitle) {
    addTitleChar(p, ' ');
    return;
  }
  flushWord(p);
  p->pendingSpace = 1;
}

static void addBreak(parser_t *p, Boolean para) {
  flushWord(p);
  docBreak(p->b, para);
  p->pendingSpace = 1;
}

static void addString(parser_t *p, const char *s) {
  for (; *s; s++) addChar(p, *s);
}

static void emitCodepoint(parser_t *p, UInt32 cp) {
  char tmp[4];
  int i, n;

  if (cp == 0xA0 || cp == ' ') {
    addSpace(p);
    return;
  }
  n = putCodepoint(cp, tmp);
  for (i = 0; i < n; i++) addChar(p, tmp[i]);
}

static int parseEntity(parser_t *p, char *s, int len, int i) {
  char name[12];
  int j = i + 1, k = 0, digits = 0;
  UInt32 cp = 0;
  Boolean hex = false;

  if (j < len && s[j] == '#') {
    j++;
    if (j < len && (s[j] == 'x' || s[j] == 'X')) { hex = true; j++; }
    while (j < len && digits < 8) {
      char c = s[j];
      if (isDigitChar(c)) cp = cp * (hex ? 16 : 10) + (c - '0');
      else if (hex && c >= 'a' && c <= 'f') cp = cp * 16 + (c - 'a' + 10);
      else if (hex && c >= 'A' && c <= 'F') cp = cp * 16 + (c - 'A' + 10);
      else break;
      digits++;
      j++;
    }
    if (digits == 0) {
      addChar(p, '&');
      return i + 1;
    }
    if (j < len && s[j] == ';') j++;
    emitCodepoint(p, cp);
    return j;
  }

  while (j < len && (isAlphaChar(s[j]) || isDigitChar(s[j])) && k < 10) {
    name[k++] = s[j++];
  }
  name[k] = 0;

  if (k > 0 && j < len && s[j] == ';') {
    for (k = 0; entities[k].name; k++) {
      if (strEq(entities[k].name, name)) {
        emitCodepoint(p, entities[k].code);
        return j + 1;
      }
    }
  }

  addChar(p, '&');
  return i + 1;
}

/* find "</tag" case-insensitively, returns index after its '>' or len */
static int skipElement(char *s, int len, int i, const char *tag) {
  int tlen = StrLen(tag), j;

  for (; i + tlen + 2 <= len; i++) {
    if (s[i] == '<' && s[i + 1] == '/') {
      for (j = 0; j < tlen; j++) {
        if (lowerChar(s[i + 2 + j]) != tag[j]) break;
      }
      if (j == tlen) {
        i += 2 + tlen;
        while (i < len && s[i] != '>') i++;
        return i < len ? i + 1 : len;
      }
    }
  }

  return len;
}

static void decodeAmp(char *s) {
  char *d = s;

  while (*s) {
    if (s[0] == '&' && s[1] == 'a' && s[2] == 'm' && s[3] == 'p' && s[4] == ';') {
      *d++ = '&';
      s += 5;
    } else if (s[0] == '&' && s[1] == '#' && s[2] == '3' && s[3] == '8' && s[4] == ';') {
      *d++ = '&';
      s += 5;
    } else {
      *d++ = *s++;
    }
  }
  *d = 0;
}

static void resolveUrl(const char *base, const char *rel, char *out, int size);

static void updateFont(parser_t *p) {
  if (p->heading) p->font = (UInt8)p->heading;
  else if (p->bold) p->font = boldFont;
  else p->font = stdFont;
}

static void handleTag(parser_t *p, char *tag, Boolean closing, char *href, char *alt, char *s, int len, int *pi) {
  browser_t *b = p->b;
  char tmp[MAX_URL];

  if (!closing && (strEq(tag, "script") || strEq(tag, "style") || strEq(tag, "noscript") ||
                   strEq(tag, "template") || strEq(tag, "svg") || strEq(tag, "iframe") ||
                   strEq(tag, "object") || strEq(tag, "textarea") || strEq(tag, "select"))) {
    *pi = skipElement(s, len, *pi, tag);
    return;
  }

  if (strEq(tag, "title")) {
    p->inTitle = !closing;
    if (closing) p->tlen = 0;
    return;
  }

  if (strEq(tag, "base")) {
    if (!closing && href[0]) {
      resolveUrl(b->url, href, tmp, sizeof(tmp));
      StrNCopy(b->base, tmp, MAX_URL - 1);
    }
    return;
  }

  if (strEq(tag, "a")) {
    if (closing) {
      p->link = LINK_NONE;
    } else {
      p->link = href[0] ? docAddLink(b, href) : LINK_NONE;
    }
    return;
  }

  if (strEq(tag, "img")) {
    if (!closing && alt[0]) {
      addSpace(p);
      addChar(p, '[');
      addString(p, alt);
      addChar(p, ']');
      addSpace(p);
    }
    return;
  }

  if (strEq(tag, "br")) {
    Boolean para = p->lastBr;
    addBreak(p, para);
    p->lastBr = 1;
    return;
  }

  if (strEq(tag, "hr")) {
    addBreak(p, false);
    addString(p, "--------------------");
    addBreak(p, false);
    return;
  }

  if (strEq(tag, "li")) {
    if (!closing) {
      addBreak(p, false);
      addChar(p, '*');
      addSpace(p);
    }
    return;
  }

  if (strEq(tag, "h1") || strEq(tag, "h2")) {
    addBreak(p, true);
    p->heading = closing ? 0 : largeBoldFont;
    updateFont(p);
    return;
  }

  if (tag[0] == 'h' && tag[1] >= '3' && tag[1] <= '6' && tag[2] == 0) {
    addBreak(p, true);
    p->heading = closing ? 0 : boldFont;
    updateFont(p);
    return;
  }

  if (strEq(tag, "b") || strEq(tag, "strong")) {
    flushWord(p);
    if (closing) { if (p->bold > 0) p->bold--; }
    else p->bold++;
    updateFont(p);
    return;
  }

  if (strEq(tag, "pre")) {
    addBreak(p, true);
    p->inPre = !closing;
    return;
  }

  if (strEq(tag, "p") || strEq(tag, "blockquote") || strEq(tag, "ul") || strEq(tag, "ol") ||
      strEq(tag, "dl") || strEq(tag, "table") || strEq(tag, "form") || strEq(tag, "fieldset") ||
      strEq(tag, "figure")) {
    addBreak(p, true);
    return;
  }

  if (strEq(tag, "div") || strEq(tag, "tr") || strEq(tag, "dt") || strEq(tag, "dd") ||
      strEq(tag, "section") || strEq(tag, "article") || strEq(tag, "header") || strEq(tag, "footer") ||
      strEq(tag, "nav") || strEq(tag, "aside") || strEq(tag, "main") || strEq(tag, "center") ||
      strEq(tag, "address") || strEq(tag, "figcaption") || strEq(tag, "details") || strEq(tag, "summary") ||
      strEq(tag, "option") || strEq(tag, "caption") || strEq(tag, "menu")) {
    addBreak(p, false);
    return;
  }

  if (strEq(tag, "td") || strEq(tag, "th") || strEq(tag, "input") || strEq(tag, "button") ||
      strEq(tag, "label")) {
    addSpace(p);
    return;
  }

  /* inline tags (span, i, em, code, ...) split words only if whitespace was seen */
  flushWord(p);
}

static int parseTag(parser_t *p, char *s, int len, int i) {
  char tag[16], aname[32], val[MAX_URL], href[MAX_URL], alt[MAX_WORDLEN];
  int tlen = 0, an, vlen;
  Boolean closing = false;
  char q;

  href[0] = 0;
  alt[0] = 0;

  i++;
  if (i < len && s[i] == '/') { closing = true; i++; }
  while (i < len && (isAlphaChar(s[i]) || isDigitChar(s[i]))) {
    if (tlen < 15) tag[tlen++] = lowerChar(s[i]);
    i++;
  }
  tag[tlen] = 0;

  while (i < len && s[i] != '>') {
    if (s[i] == '/' || isSpaceChar(s[i])) { i++; continue; }

    an = 0;
    while (i < len && s[i] != '=' && s[i] != '>' && s[i] != '/' && !isSpaceChar(s[i])) {
      if (an < 31) aname[an++] = lowerChar(s[i]);
      i++;
    }
    aname[an] = 0;
    while (i < len && isSpaceChar(s[i])) i++;

    vlen = 0;
    if (i < len && s[i] == '=') {
      i++;
      while (i < len && isSpaceChar(s[i])) i++;
      if (i < len && (s[i] == '"' || s[i] == '\'')) {
        q = s[i++];
        while (i < len && s[i] != q) {
          if (vlen < MAX_URL - 1) val[vlen++] = s[i];
          i++;
        }
        if (i < len) i++;
      } else {
        while (i < len && !isSpaceChar(s[i]) && s[i] != '>') {
          if (vlen < MAX_URL - 1) val[vlen++] = s[i];
          i++;
        }
      }
    }
    val[vlen] = 0;

    if (strEq(aname, "href")) {
      StrNCopy(href, val, MAX_URL - 1);
      href[MAX_URL - 1] = 0;
      decodeAmp(href);
    } else if (strEq(aname, "alt")) {
      StrNCopy(alt, val, MAX_WORDLEN - 1);
      alt[MAX_WORDLEN - 1] = 0;
    }
  }
  if (i < len) i++;

  if (tlen > 0) {
    flushWord(p);
    handleTag(p, tag, closing, href, alt, s, len, &i);
  }

  return i;
}

static void parserInit(parser_t *p, browser_t *b) {
  MemSet(p, sizeof(parser_t), 0);
  p->b = b;
  p->font = stdFont;
  p->link = LINK_NONE;
  p->pendingSpace = 1;
}

static void parseHtml(browser_t *b, char *s, int len) {
  parser_t p;
  int i = 0, e;
  char c;

  parserInit(&p, b);

  while (i < len) {
    c = s[i];

    if (c == '<') {
      if (i + 3 < len && s[i + 1] == '!' && s[i + 2] == '-' && s[i + 3] == '-') {
        for (e = i + 4; e + 2 < len; e++) {
          if (s[e] == '-' && s[e + 1] == '-' && s[e + 2] == '>') break;
        }
        i = (e + 2 < len) ? e + 3 : len;
        continue;
      }
      if (i + 1 < len && (s[i + 1] == '!' || s[i + 1] == '?')) {
        while (i < len && s[i] != '>') i++;
        i++;
        continue;
      }
      if (i + 1 < len && (isAlphaChar(s[i + 1]) || s[i + 1] == '/')) {
        i = parseTag(&p, s, len, i);
        continue;
      }
      addChar(&p, '<');
      i++;
      continue;
    }

    if (c == '&') {
      i = parseEntity(&p, s, len, i);
      continue;
    }

    if (isSpaceChar(c)) {
      if (p.inPre && c == '\n') {
        Boolean para = p.lastBr;
        addBreak(&p, para);
        p.lastBr = 1;
      } else {
        addSpace(&p);
      }
      i++;
      continue;
    }

    if ((unsigned char)c < 32) {
      i++;
      continue;
    }

    addChar(&p, c);
    i++;
  }

  flushWord(&p);
  docTrim(b);
}

static void parsePlainText(browser_t *b, char *s, int len) {
  parser_t p;
  int i;
  char c;

  parserInit(&p, b);
  p.inPre = 1;

  for (i = 0; i < len; i++) {
    c = s[i];
    if (c == '\n') {
      Boolean para = p.lastBr;
      addBreak(&p, para);
      p.lastBr = 1;
    } else if (isSpaceChar(c)) {
      addSpace(&p);
    } else if ((unsigned char)c >= 32) {
      addChar(&p, c);
    }
  }

  flushWord(&p);
}

/* replace the document with a message; '\n' starts a new line */
static void setMessagePage(browser_t *b, const char *heading, const char *msg) {
  parser_t p;

  docReset(b);
  parserInit(&p, b);
  p.font = boldFont;
  addString(&p, heading);
  flushWord(&p);
  p.font = stdFont;
  addBreak(&p, true);
  for (; *msg; msg++) {
    if (*msg == '\n') addBreak(&p, false);
    else if (*msg == ' ') addSpace(&p);
    else addChar(&p, *msg);
  }
  flushWord(&p);
}

/*
 * URL handling
 */

static Boolean hasScheme(const char *u) {
  int i;

  if (!isAlphaChar(u[0])) return false;
  for (i = 1; u[i]; i++) {
    if (u[i] == ':') return u[i + 1] == '/' && u[i + 2] == '/';
    if (!isAlphaChar(u[i]) && !isDigitChar(u[i]) && u[i] != '+' && u[i] != '-' && u[i] != '.') return false;
  }

  return false;
}

static Boolean unsupportedScheme(const char *u) {
  return !StrNCaselessCompare(u, "mailto:", 7) || !StrNCaselessCompare(u, "javascript:", 11) ||
         !StrNCaselessCompare(u, "tel:", 4) || !StrNCaselessCompare(u, "data:", 5);
}

/* collapse "." and ".." segments in the path part of an absolute URL */
static void normalizePath(char *url) {
  char out[MAX_URL];
  char *p, *pathStart, *q, *r;
  int o = 0, prefixLen, seglen, nseg = 0;
  int segStart[64];

  p = sys_strstr(url, "://");
  if (!p) return;
  pathStart = sys_strchr(p + 3, '/');
  if (!pathStart) return;

  prefixLen = (int)(pathStart - url);
  catStr(out, sizeof(out), &o, url, prefixLen);

  q = pathStart;
  while (*q == '/') {
    r = q + 1;
    while (*r && *r != '/' && *r != '?' && *r != '#') r++;
    seglen = (int)(r - (q + 1));

    if (seglen == 1 && q[1] == '.') {
      /* skip */
    } else if (seglen == 2 && q[1] == '.' && q[2] == '.') {
      if (nseg > 0) { nseg--; o = segStart[nseg]; out[o] = 0; }
    } else {
      if (nseg < 64) segStart[nseg++] = o;
      catStr(out, sizeof(out), &o, "/", 1);
      catStr(out, sizeof(out), &o, q + 1, seglen);
    }
    q = r;
    if (*q != '/') break;
  }
  if (o == prefixLen) catStr(out, sizeof(out), &o, "/", 1);
  catStr(out, sizeof(out), &o, q, StrLen(q));

  StrNCopy(url, out, MAX_URL - 1);
  url[MAX_URL - 1] = 0;
}

static void resolveUrl(const char *base, const char *rel, char *out, int size) {
  const char *p, *pathStart, *end, *q, *last;
  int o = 0;

  while (isSpaceChar(*rel)) rel++;
  out[0] = 0;

  if (hasScheme(rel) || unsupportedScheme(rel)) {
    catStr(out, size, &o, rel, StrLen(rel));
    return;
  }

  p = sys_strstr(base, "://");
  if (!p) {
    catStr(out, size, &o, rel, StrLen(rel));
    return;
  }
  pathStart = sys_strchr(p + 3, '/');
  if (!pathStart) pathStart = base + StrLen(base);

  if (rel[0] == '/' && rel[1] == '/') {
    catStr(out, size, &o, base, (int)(p + 1 - base));
    catStr(out, size, &o, rel, StrLen(rel));
    return;
  }

  if (rel[0] == '/') {
    catStr(out, size, &o, base, (int)(pathStart - base));
    catStr(out, size, &o, rel, StrLen(rel));
    normalizePath(out);
    return;
  }

  if (rel[0] == '?') {
    end = pathStart;
    while (*end && *end != '?' && *end != '#') end++;
    catStr(out, size, &o, base, (int)(end - base));
    if (end == pathStart) catStr(out, size, &o, "/", 1);
    catStr(out, size, &o, rel, StrLen(rel));
    return;
  }

  if (rel[0] == '#') {
    end = pathStart;
    while (*end && *end != '#') end++;
    catStr(out, size, &o, base, (int)(end - base));
    if (end == pathStart) catStr(out, size, &o, "/", 1);
    catStr(out, size, &o, rel, StrLen(rel));
    return;
  }

  /* relative path: replace everything after the last '/' of the base path */
  end = pathStart;
  while (*end && *end != '?' && *end != '#') end++;
  last = NULL;
  for (q = pathStart; q < end; q++) {
    if (*q == '/') last = q;
  }
  if (last) {
    catStr(out, size, &o, base, (int)(last - base + 1));
  } else {
    catStr(out, size, &o, base, (int)(pathStart - base));
    catStr(out, size, &o, "/", 1);
  }
  catStr(out, size, &o, rel, StrLen(rel));
  normalizePath(out);
}

static void stripFragment(char *url) {
  char *p = sys_strchr(url, '#');
  if (p) *p = 0;
}

/* turn what the user typed into something fetchable */
static void normalizeInput(const char *in, char *out, int size) {
  int o = 0, n;

  while (isSpaceChar(*in)) in++;
  n = StrLen(in);
  while (n > 0 && isSpaceChar(in[n - 1])) n--;

  if (!hasScheme(in) && StrNCaselessCompare(in, "about:", 6) != 0) catStr(out, size, &o, "http://", 7);
  catStr(out, size, &o, in, n);
}

static void urlEncode(const char *in, char *out, int size) {
  static const char hex[] = "0123456789ABCDEF";
  int o = 0;
  unsigned char c;

  for (; *in && o < size - 4; in++) {
    c = (unsigned char)*in;
    if (isAlphaChar(c) || isDigitChar(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out[o++] = c;
    } else {
      out[o++] = '%';
      out[o++] = hex[c >> 4];
      out[o++] = hex[c & 15];
    }
  }
  out[o] = 0;
}

/*
 * Fetching
 */

static void fetchFree(fetch_t *f) {
  if (f->body) xfree(f->body);
  xfree(f);
}

/* runs in the network thread when the response is complete */
static int fetchCallback(int ptr, void *_data) {
  fetch_t *f = (fetch_t *)_data;
  http_client_t *hc;
  char buf[4096];
  char *name, *value;
  int i, n, abandoned;

  if ((hc = ptr_lock(ptr, TAG_HTTP_CLIENT)) != NULL) {
    f->code = hc->response_code_found ? hc->response_code : 0;
    f->error = hc->response_error;

    for (i = 0; i < hc->response_num_headers; i++) {
      name = hc->response_header_name[i];
      value = hc->response_header_value[i];
      if (!name || !value) continue;
      if (!sys_strcasecmp(name, "Location")) {
        sys_strncpy(f->location, value, MAX_URL - 1);
      } else if (!sys_strcasecmp(name, "Content-Type")) {
        sys_strncpy(f->contentType, value, sizeof(f->contentType) - 1);
      } else if (!sys_strcasecmp(name, "Transfer-Encoding")) {
        if (sys_strstr(value, "chunked")) f->chunked = 1;
      }
    }

    if (hc->response_fd > 0) {
      for (;;) {
        n = sys_read(hc->response_fd, (uint8_t *)buf, sizeof(buf));
        if (n <= 0) break;
        if (f->len + n > MAX_RESPONSE) n = MAX_RESPONSE - f->len;
        if (n <= 0) break;
        f->body = xrealloc(f->body, f->len + n + 1);
        sys_memcpy(f->body + f->len, buf, n);
        f->len += n;
        f->body[f->len] = 0;
      }
    }
    ptr_unlock(ptr, TAG_HTTP_CLIENT);
  } else {
    f->error = 1;
  }

  mutex_lock(f->mutex);
  abandoned = f->abandoned;
  f->done = 1;
  mutex_unlock(f->mutex);

  if (abandoned) fetchFree(f);

  return 0;
}

/* decode HTTP chunked transfer encoding in place, returns new length */
static int dechunk(char *body, int len) {
  int in = 0, out = 0, size;
  char c;

  while (in < len) {
    size = 0;
    while (in < len) {
      c = body[in];
      if (isDigitChar(c)) size = size * 16 + (c - '0');
      else if (c >= 'a' && c <= 'f') size = size * 16 + (c - 'a' + 10);
      else if (c >= 'A' && c <= 'F') size = size * 16 + (c - 'A' + 10);
      else break;
      in++;
    }
    while (in < len && body[in] != '\n') in++;
    in++;
    if (size <= 0 || in >= len) break;
    if (in + size > len) size = len - in;
    MemMove(body + out, body + in, size);
    out += size;
    in += size;
    while (in < len && (body[in] == '\r' || body[in] == '\n')) in++;
  }
  body[out] = 0;

  return out;
}

typedef enum { fetchOk, fetchNoTls, fetchFailed, fetchTimeout, fetchCancelled, fetchStopped } fetch_status_t;

/*
 * Start a request and wait for it, keeping the UI alive.
 * A pen tap cancels the request; appStopEvent stops the application.
 */
static fetch_status_t doFetch(browser_t *b, const char *url, fetch_t **out) {
  fetch_t *f;
  EventType event;
  UInt32 start, tps;
  int handle, done = 0;
  fetch_status_t status = fetchOk;

  *out = NULL;

  f = xcalloc(1, sizeof(fetch_t));
  f->mutex = b->mutex;

  handle = pumpkin_http_get((char *)url, FETCH_TIMEOUT, fetchCallback, f);
  if (handle == -1) {
    fetchFree(f);
    return StrNCaselessCompare(url, "https://", 8) == 0 ? fetchNoTls : fetchFailed;
  }

  tps = SysTicksPerSecond();
  start = TimGetTicks();

  for (;;) {
    EvtGetEvent(&event, tps / 10);

    if (event.eType == appStopEvent) {
      b->stop = true;
      status = fetchStopped;
      break;
    }
    if (event.eType == penDownEvent && RctPtInRectangle(event.screenX, event.screenY, &b->area)) {
      status = fetchCancelled;
      break;
    }
    if (event.eType != nilEvent) SysHandleEvent(&event);

    mutex_lock(b->mutex);
    done = f->done;
    mutex_unlock(b->mutex);
    if (done) break;

    if (TimGetTicks() - start > (UInt32)(FETCH_TIMEOUT + 10) * tps) {
      status = fetchTimeout;
      break;
    }
  }

  if (!done) {
    mutex_lock(b->mutex);
    done = f->done;
    if (!done) {
      f->abandoned = 1;
      b->abandonedFetches++;
    }
    mutex_unlock(b->mutex);
    if (!done) {
      pumpkin_http_abort(handle);
      return status;
    }
    if (status == fetchStopped) {
      fetchFree(f);
      return status;
    }
  }

  *out = f;
  return fetchOk;
}

/*
 * Layout
 */

static UInt32 newLine(browser_t *b, Coord y) {
  line_t *ln;

  b->lines = growArray(b->lines, &b->linesSize, b->nlines + 1, sizeof(line_t));
  ln = &b->lines[b->nlines];
  ln->y = y;
  ln->h = 0;
  ln->firstSeg = b->nsegs;
  ln->nsegs = 0;

  return b->nlines++;
}

static void addSeg(browser_t *b, UInt32 li, Coord x, UInt32 off, UInt16 len, UInt8 font, Int16 link) {
  seg_t *sg;

  b->segs = growArray(b->segs, &b->segsSize, b->nsegs + 1, sizeof(seg_t));
  sg = &b->segs[b->nsegs++];
  sg->x = x;
  sg->off = off;
  sg->len = len;
  sg->font = font;
  sg->link = link;
  b->lines[li].nsegs++;
}

static void layoutDoc(browser_t *b) {
  item_t *it;
  UInt32 i, li;
  Coord x, y, maxW, w, sp, h, stdH;
  FontID oldFont;
  UInt16 len, n;
  char *t;

  b->nsegs = 0;
  b->nlines = 0;
  maxW = b->area.extent.x - 4;
  oldFont = FntSetFont(stdFont);
  stdH = FntLineHeight();

  y = 0;
  li = newLine(b, y);
  x = 2;

  for (i = 0; i < b->nitems; i++) {
    it = &b->items[i];

    if (it->kind != itemWord) {
      if (b->lines[li].h == 0) b->lines[li].h = stdH;
      y += b->lines[li].h;
      if (it->kind == itemPara) {
        li = newLine(b, y);
        b->lines[li].h = stdH / 2;
        y += b->lines[li].h;
      }
      li = newLine(b, y);
      x = 2;
      continue;
    }

    FntSetFont(it->font);
    h = FntLineHeight();
    t = b->text + it->off;
    len = it->len;
    sp = (x > 2 && !(it->flags & ITEM_GLUE)) ? FntCharWidth(' ') : 0;
    w = FntCharsWidth(t, len);

    if (x > 2 && x + sp + w > maxW) {
      if (b->lines[li].h == 0) b->lines[li].h = stdH;
      y += b->lines[li].h;
      li = newLine(b, y);
      x = 2;
      sp = 0;
    }

    while (len > 0) {
      n = len;
      w = FntCharsWidth(t, n);
      while (n > 1 && x + sp + w > maxW) {
        n--;
        w = FntCharsWidth(t, n);
      }
      addSeg(b, li, x + sp, (UInt32)(t - b->text), n, it->font, it->link);
      if (h > b->lines[li].h) b->lines[li].h = h;
      x += sp + w;
      sp = 0;
      t += n;
      len -= n;
      if (len > 0) {
        y += b->lines[li].h;
        li = newLine(b, y);
        x = 2;
      }
    }
  }

  if (b->lines[li].h == 0) b->lines[li].h = stdH;
  FntSetFont(oldFont);
}

/* number of lines that fit on screen starting at line 'from' */
static UInt32 linesFitting(browser_t *b, UInt32 from) {
  Coord avail = b->area.extent.y - 2, total = 0;
  UInt32 i, n = 0;

  for (i = from; i < b->nlines; i++) {
    total += b->lines[i].h;
    if (total > avail) break;
    n++;
  }

  return n;
}

static UInt32 maxTopLine(browser_t *b) {
  Coord avail = b->area.extent.y - 2, total = 0;
  UInt32 i;

  for (i = b->nlines; i > 0; i--) {
    total += b->lines[i - 1].h;
    if (total > avail) return i;
  }

  return 0;
}

/*
 * Drawing
 */

static void updateScrollbar(browser_t *b) {
  FormType *frm;
  ScrollBarType *scl;
  UInt32 max, page;

  frm = FrmGetActiveForm();
  if (!frm) return;
  scl = (ScrollBarType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pageScl));
  if (!scl) return;

  max = maxTopLine(b);
  if (b->topLine > max) b->topLine = max;
  page = linesFitting(b, b->topLine);
  if (page < 1) page = 1;

  SclSetScrollBar(scl, (Int16)b->topLine, 0, (Int16)max, (Int16)page);
  SclDrawScrollBar(scl);
}

static void drawPage(browser_t *b) {
  RectangleType clip;
  RGBColorType linkColor, oldColor;
  FontID oldFont;
  UnderlineModeType oldUl;
  line_t *ln;
  seg_t *sg;
  UInt32 i, j;
  Coord y, bottom;

  linkColor.index = 0;
  linkColor.r = 0x00;
  linkColor.g = 0x00;
  linkColor.b = 0xC0;

  WinGetClip(&clip);
  WinSetClip(&b->area);
  WinEraseRectangle(&b->area, 0);
  oldFont = FntSetFont(stdFont);

  y = b->area.topLeft.y + 1;
  bottom = b->area.topLeft.y + b->area.extent.y;

  for (i = b->topLine; i < b->nlines && y < bottom; i++) {
    ln = &b->lines[i];
    for (j = 0; j < ln->nsegs; j++) {
      sg = &b->segs[ln->firstSeg + j];
      FntSetFont(sg->font);
      if (sg->link != LINK_NONE) {
        WinSetTextColorRGB(&linkColor, &oldColor);
        oldUl = WinSetUnderlineMode(solidUnderline);
        WinDrawChars(b->text + sg->off, sg->len, b->area.topLeft.x + sg->x, y + (ln->h - FntLineHeight()));
        WinSetUnderlineMode(oldUl);
        WinSetTextColorRGB(&oldColor, NULL);
      } else {
        WinDrawChars(b->text + sg->off, sg->len, b->area.topLeft.x + sg->x, y + (ln->h - FntLineHeight()));
      }
    }
    y += ln->h;
  }

  FntSetFont(oldFont);
  WinSetClip(&clip);
}

static void showStatus(browser_t *b, const char *msg) {
  FontID oldFont;

  WinEraseRectangle(&b->area, 0);
  oldFont = FntSetFont(stdFont);
  WinDrawTruncChars(msg, StrLen(msg), b->area.topLeft.x + 2, b->area.topLeft.y + 2, b->area.extent.x - 4);
  FntSetFont(oldFont);
}

static Int16 hitLink(browser_t *b, Coord px, Coord py) {
  FontID oldFont;
  line_t *ln;
  seg_t *sg;
  UInt32 i, j;
  Coord y, bottom, x0, w;
  Int16 link = LINK_NONE;

  oldFont = FntSetFont(stdFont);
  y = b->area.topLeft.y + 1;
  bottom = b->area.topLeft.y + b->area.extent.y;

  for (i = b->topLine; i < b->nlines && y < bottom && link == LINK_NONE; i++) {
    ln = &b->lines[i];
    if (py >= y && py < y + ln->h) {
      for (j = 0; j < ln->nsegs; j++) {
        sg = &b->segs[ln->firstSeg + j];
        if (sg->link == LINK_NONE) continue;
        FntSetFont(sg->font);
        x0 = b->area.topLeft.x + sg->x;
        w = FntCharsWidth(b->text + sg->off, sg->len);
        if (px >= x0 - 1 && px < x0 + w + 1) {
          link = sg->link;
          break;
        }
      }
      break;
    }
    y += ln->h;
  }

  FntSetFont(oldFont);
  return link;
}

static void scrollTo(browser_t *b, Int32 line) {
  UInt32 max = maxTopLine(b);

  if (line < 0) line = 0;
  if ((UInt32)line > max) line = max;
  if ((UInt32)line == b->topLine) return;
  b->topLine = line;
  updateScrollbar(b);
  drawPage(b);
}

static void scrollPage(browser_t *b, Int16 dir) {
  UInt32 n = linesFitting(b, b->topLine);

  if (n < 1) n = 1;
  if (dir < 0) {
    scrollTo(b, (Int32)b->topLine - (Int32)n);
  } else {
    scrollTo(b, (Int32)b->topLine + (Int32)n);
  }
}

/*
 * Form field helpers
 */

static void setField(FormType *frm, UInt16 fieldId, const char *s) {
  FieldType *fld;
  UInt16 len;

  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldId));
  if (!fld) return;
  len = FldGetTextLength(fld);
  if (len) FldDelete(fld, 0, len);
  if (s && s[0]) FldInsert(fld, s, StrLen(s));
}

static void getField(FormType *frm, UInt16 fieldId, char *buf, int size) {
  FieldType *fld;
  char *s;

  buf[0] = 0;
  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldId));
  if (fld && (s = FldGetTextPtr(fld)) != NULL) {
    StrNCopy(buf, s, size - 1);
    buf[size - 1] = 0;
  }
}

/*
 * Navigation
 */

static void pushHistory(browser_t *b, const char *url) {
  if (!url[0]) return;
  if (b->nhistory == MAX_HISTORY) {
    MemMove(b->history[0], b->history[1], (MAX_HISTORY - 1) * MAX_URL);
    b->nhistory--;
  }
  StrNCopy(b->history[b->nhistory], url, MAX_URL - 1);
  b->history[b->nhistory][MAX_URL - 1] = 0;
  b->nhistory++;
}

static Boolean isHtml(fetch_t *f) {
  char *p;

  if (f->contentType[0]) {
    return sys_strstr(f->contentType, "html") != NULL || sys_strstr(f->contentType, "xml") != NULL;
  }
  for (p = f->body; p && *p && isSpaceChar(*p); p++);
  return p && *p == '<';
}

static Boolean isLatin1(fetch_t *f) {
  char ct[128];
  int i;

  for (i = 0; f->contentType[i] && i < 127; i++) ct[i] = lowerChar(f->contentType[i]);
  ct[i] = 0;

  return sys_strstr(ct, "iso-8859") != NULL || sys_strstr(ct, "windows-125") != NULL || sys_strstr(ct, "latin") != NULL;
}

static void navigate(browser_t *b, const char *target, Boolean push) {
  FormType *frm;
  fetch_t *f = NULL;
  fetch_status_t status;
  char url[MAX_URL], next[MAX_URL], msg[MAX_URL + 128];
  int redirects, len;

  StrNCopy(url, target, MAX_URL - 1);
  url[MAX_URL - 1] = 0;
  stripFragment(url);
  if (!url[0]) return;

  if (push) pushHistory(b, b->url);
  StrCopy(b->url, url);
  StrCopy(b->base, url);

  frm = FrmGetActiveForm();
  if (frm) setField(frm, urlFld, url);

  if (!StrNCaselessCompare(url, "about:", 6)) {
    docReset(b);
    parseHtml(b, (char *)START_PAGE, StrLen(START_PAGE));
    layoutDoc(b);
    b->topLine = 0;
    updateScrollbar(b);
    drawPage(b);
    return;
  }

  for (redirects = 0; ; redirects++) {
    StrCopy(msg, "Loading ");
    StrNCat(msg, url, sizeof(msg) - 16);
    StrCat(msg, "...");
    showStatus(b, msg);

    status = doFetch(b, url, &f);
    if (status != fetchOk || f == NULL) break;

    if (f->code >= 300 && f->code < 400 && f->location[0] && redirects < MAX_REDIRECTS) {
      resolveUrl(url, f->location, next, sizeof(next));
      stripFragment(next);
      fetchFree(f);
      f = NULL;
      if (!next[0] || !hasScheme(next)) break;
      StrCopy(url, next);
      StrCopy(b->url, url);
      StrCopy(b->base, url);
      if (frm) setField(frm, urlFld, url);
      continue;
    }
    break;
  }

  if (b->stop) {
    if (f) fetchFree(f);
    return;
  }

  docReset(b);

  switch (status) {
    case fetchNoTls:
      setMessagePage(b, "HTTPS not available",
        "This PumpkinOS build has no TLS provider loaded, so https:// pages cannot be fetched directly.\n\n"
        "Use Page > Open via FrogFind to read the page through an HTTP text proxy, or try the http:// version of the address.");
      break;
    case fetchFailed:
      setMessagePage(b, "Could not connect", "The address could not be reached. Check the URL and the network connection.");
      break;
    case fetchTimeout:
      setMessagePage(b, "Timed out", "The server did not answer in time.");
      break;
    case fetchCancelled:
      setMessagePage(b, "Cancelled", "The request was cancelled.");
      break;
    case fetchStopped:
      break;
    case fetchOk:
      if (f == NULL) {
        setMessagePage(b, "Error", "Too many redirects.");
        break;
      }
      if (f->error && f->len == 0) {
        setMessagePage(b, "Could not load page", "The connection was closed before any data arrived.");
        break;
      }
      if (f->code == 0 && f->len == 0) {
        setMessagePage(b, "Empty reply", "The server sent no data.");
        break;
      }
      len = f->len;
      if (f->body == NULL) {
        f->body = xcalloc(1, 1);
        len = 0;
      }
      if (f->chunked) len = dechunk(f->body, len);
      if (isHtml(f)) {
        if (!isLatin1(f)) len = utf8ToLatin1(f->body, len);
        parseHtml(b, f->body, len);
        if (b->nitems == 0) setMessagePage(b, "Empty page", "The page contains no text.");
      } else if (!StrNCaselessCompare(f->contentType, "text/", 5) || f->contentType[0] == 0) {
        if (!isLatin1(f)) len = utf8ToLatin1(f->body, len);
        parsePlainText(b, f->body, len);
      } else {
        StrCopy(msg, "Content type ");
        StrNCat(msg, f->contentType, sizeof(msg) - 64);
        StrCat(msg, " cannot be displayed by this browser.");
        setMessagePage(b, "Unsupported content", msg);
      }
      break;
  }

  if (f) fetchFree(f);

  layoutDoc(b);
  b->topLine = 0;
  updateScrollbar(b);
  drawPage(b);
}

static void goBack(browser_t *b) {
  char url[MAX_URL];

  if (b->nhistory == 0) return;
  b->nhistory--;
  StrCopy(url, b->history[b->nhistory]);
  navigate(b, url, false);
}

static void goFromField(browser_t *b) {
  FormType *frm = FrmGetActiveForm();
  char typed[MAX_URL], url[MAX_URL];

  getField(frm, urlFld, typed, sizeof(typed));
  normalizeInput(typed, url, sizeof(url));
  if (StrLen(url) <= 7) return;
  navigate(b, url, true);
}

static void followLink(browser_t *b, Int16 link) {
  char url[MAX_URL];

  if (link < 0 || (UInt32)link >= b->nlinks || !b->links[link]) return;
  resolveUrl(b->base, b->links[link], url, sizeof(url));
  if (!url[0] || unsupportedScheme(url) || !hasScheme(url)) return;
  navigate(b, url, true);
}

static void openViaFrogFind(browser_t *b) {
  char enc[MAX_URL * 3], url[MAX_URL];
  int o = 0;

  if (!b->url[0]) return;
  urlEncode(b->url, enc, sizeof(enc));
  url[0] = 0;
  catStr(url, sizeof(url), &o, FROGFIND_READ, StrLen(FROGFIND_READ));
  catStr(url, sizeof(url), &o, enc, StrLen(enc));
  navigate(b, url, true);
}

/*
 * Preferences
 */

static void loadPrefs(browser_t *b) {
  UInt16 size = sizeof(browser_prefs_t);
  Int16 version;

  MemSet(&b->prefs, sizeof(browser_prefs_t), 0);
  version = PrefGetAppPreferences(pumpkin_get_app_creator(), PREFS_ID, &b->prefs, &size, true);
  if (version == noPreferenceFound || version != PREFS_VERSION || b->prefs.home[0] == 0) {
    MemSet(&b->prefs, sizeof(browser_prefs_t), 0);
    StrCopy(b->prefs.home, DEFAULT_HOME);
  }
  b->prefs.home[MAX_URL - 1] = 0;
}

static void savePrefs(browser_t *b) {
  PrefSetAppPreferences(pumpkin_get_app_creator(), PREFS_ID, PREFS_VERSION, &b->prefs, sizeof(browser_prefs_t), true);
}

/*
 * Events
 */

static Boolean MainFormHandleEvent(EventType *event) {
  browser_t *b = (browser_t *)pumpkin_get_data();
  FormType *frm;
  UInt16 index;
  Int16 link;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, pageGad);
      FrmGetObjectBounds(frm, index, &b->area);
      FntSetFont(stdFont);
      b->stdLineHeight = FntLineHeight();
      setField(frm, urlFld, b->prefs.home);
      FrmSetFocus(frm, FrmGetObjectIndex(frm, urlFld));
      FrmDrawForm(frm);
      navigate(b, b->prefs.home, false);
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      drawPage(b);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case goBtn:
          goFromField(b);
          handled = true;
          break;
        case backBtn:
          goBack(b);
          handled = true;
          break;
      }
      break;

    case penDownEvent:
      if (RctPtInRectangle(event->screenX, event->screenY, &b->area)) {
        b->dragging = true;
        b->dragMoved = false;
        b->dragStartX = event->screenX;
        b->dragStartY = event->screenY;
        b->dragStartTop = b->topLine;
        handled = true;
      }
      break;

    case penMoveEvent:
      if (b->dragging && event->penDown) {
        Int32 dy = (Int32)b->dragStartY - (Int32)event->screenY;
        if (dy < -DRAG_THRESHOLD || dy > DRAG_THRESHOLD) b->dragMoved = true;
        if (b->dragMoved && b->stdLineHeight > 0) {
          scrollTo(b, (Int32)b->dragStartTop + dy / b->stdLineHeight);
        }
        handled = true;
      }
      break;

    case penUpEvent:
      if (b->dragging) {
        b->dragging = false;
        if (!b->dragMoved) {
          link = hitLink(b, b->dragStartX, b->dragStartY);
          if (link != LINK_NONE) followLink(b, link);
        }
        handled = true;
      }
      break;

    case sclRepeatEvent:
      scrollTo(b, event->data.sclRepeat.newValue);
      break;

    case sclExitEvent:
      scrollTo(b, event->data.sclExit.newValue);
      handled = true;
      break;

    case keyDownEvent:
      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (event->data.keyDown.chr) {
          case vchrPageUp:
            scrollPage(b, -1);
            handled = true;
            break;
          case vchrPageDown:
            scrollPage(b, 1);
            handled = true;
            break;
          case vchrRockerUp:
            scrollTo(b, (Int32)b->topLine - 1);
            handled = true;
            break;
          case vchrRockerDown:
            scrollTo(b, (Int32)b->topLine + 1);
            handled = true;
            break;
        }
      } else if (event->data.keyDown.chr == 10 || event->data.keyDown.chr == 13) {
        goFromField(b);
        handled = true;
      }
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case homeCmd:
          navigate(b, b->prefs.home, true);
          handled = true;
          break;
        case reloadCmd:
          if (b->url[0]) navigate(b, b->url, false);
          handled = true;
          break;
        case backCmd:
          goBack(b);
          handled = true;
          break;
        case setHomeCmd:
          if (b->url[0]) {
            StrCopy(b->prefs.home, b->url);
            savePrefs(b);
          }
          handled = true;
          break;
        case frogCmd:
          openViaFrogFind(b);
          handled = true;
          break;
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *frm;
  UInt16 formID;
  Boolean handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formID = event->data.frmLoad.formID;
      frm = FrmInitForm(formID);
      FrmSetActiveForm(frm);
      switch (formID) {
        case MainForm:
          FrmSetEventHandler(frm, MainFormHandleEvent);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(browser_t *b) {
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && !b->stop);
}

#ifdef ESP32
UInt32 BrowserPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#else
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#endif
{
  browser_t *b;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    b = xcalloc(1, sizeof(browser_t));
    b->mutex = mutex_create("browser");
    loadPrefs(b);
    pumpkin_set_data(b);

    FrmGotoForm(MainForm);
    EventLoop(b);
    FrmCloseAllForms();

    docFree(b);
    /* an abandoned request may still finish in the network thread and lock the mutex */
    if (b->abandonedFetches == 0) mutex_destroy(b->mutex);
    pumpkin_set_data(NULL);
    xfree(b);
  }

  return 0;
}
