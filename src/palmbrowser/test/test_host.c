/*
 * Host-side test harness: compiles Browser.c natively with stubbed PalmOS
 * calls so the URL, HTML, chunked-encoding, UTF-8 and layout code can be
 * exercised outside PumpkinOS.
 *
 *   make -f Makefile.test && ./test/test_host [file.html]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define PilotMain BrowserPilotMain
#include "../Browser.c"

/* ---- PalmOS stubs ---- */

static FontID curFont = stdFont;

Int16 StrCompare(const Char *a, const Char *b) { return strcmp(a, b); }
UInt16 StrLen(const Char *s) { return (UInt16)strlen(s); }
Char *StrCopy(Char *d, const Char *s) { return strcpy(d, s); }
Char *StrNCopy(Char *d, const Char *s, Int16 n) { return strncpy(d, s, n); }
Char *StrCat(Char *d, const Char *s) { return strcat(d, s); }
Char *StrNCat(Char *d, const Char *s, Int16 n) { size_t l = strlen(d); if ((Int16)l < n) strncat(d, s, n - 1 - l); return d; }
Int16 StrNCaselessCompare(const Char *a, const Char *b, Int32 n) { return strncasecmp(a, b, n); }
Err MemMove(void *d, const void *s, Int32 n) { memmove(d, s, n); return 0; }
Err MemSet(void *d, Int32 n, UInt8 v) { memset(d, v, n); return 0; }

FontID FntSetFont(FontID f) { FontID o = curFont; curFont = f; return o; }
Int16 FntLineHeight(void) { return (curFont == largeFont || curFont == largeBoldFont) ? 14 : 11; }
Int16 FntCharWidth(Char c) { return (curFont == largeFont || curFont == largeBoldFont) ? 8 : (curFont == boldFont ? 7 : 6); }
Int16 FntCharsWidth(const Char *s, Int16 len) { return len * FntCharWidth('x'); }

void WinGetClip(RectangleType *r) { memset(r, 0, sizeof(*r)); }
void WinSetClip(const RectangleType *r) {}
void WinEraseRectangle(const RectangleType *r, UInt16 d) {}
void WinSetTextColorRGB(const RGBColorType *n, RGBColorType *o) {}
UnderlineModeType WinSetUnderlineMode(UnderlineModeType m) { return noUnderline; }
void WinDrawChars(const Char *s, Int16 len, Coord x, Coord y) {}
void WinDrawTruncChars(const Char *s, Int16 len, Coord x, Coord y, Coord w) {}
Boolean RctPtInRectangle(Coord x, Coord y, const RectangleType *r) { return false; }
void RctSetRectangle(RectangleType *r, Coord l, Coord t, Coord w, Coord h) { r->topLeft.x = l; r->topLeft.y = t; r->extent.x = w; r->extent.y = h; }

FormType *FrmGetActiveForm(void) { return NULL; }
void *FrmGetObjectPtr(const FormType *f, UInt16 i) { return NULL; }
UInt16 FrmGetObjectIndex(const FormType *f, UInt16 id) { return 0; }
void FrmGetObjectBounds(const FormType *f, UInt16 i, RectangleType *r) {}
void FrmSetFocus(FormType *f, UInt16 i) {}
void FrmDrawForm(FormType *f) {}
FormType *FrmInitForm(UInt16 id) { return NULL; }
void FrmSetActiveForm(FormType *f) {}
void FrmSetEventHandler(FormType *f, FormEventHandlerType *h) {}
void FrmGotoForm(UInt16 id) {}
void FrmCloseAllForms(void) {}
Boolean FrmDispatchEvent(EventType *e) { return false; }
UInt16 FldGetTextLength(const FieldType *f) { return 0; }
void FldDelete(FieldType *f, UInt16 s, UInt16 e) {}
Boolean FldInsert(FieldType *f, const Char *s, UInt16 n) { return true; }
Char *FldGetTextPtr(const FieldType *f) { return NULL; }
void SclSetScrollBar(ScrollBarType *s, Int16 v, Int16 mn, Int16 mx, Int16 p) {}
void SclDrawScrollBar(ScrollBarType *s) {}
Int16 PrefGetAppPreferences(UInt32 c, UInt16 id, void *p, UInt16 *s, Boolean saved) { return noPreferenceFound; }
void PrefSetAppPreferences(UInt32 c, UInt16 id, Int16 v, const void *p, UInt16 s, Boolean saved) {}
void EvtGetEvent(EventType *e, Int32 t) { e->eType = nilEvent; }
Boolean SysHandleEvent(EventType *e) { return false; }
Boolean MenuHandleEvent(MenuBarType *m, EventType *e, UInt16 *err) { return false; }
UInt16 SysTicksPerSecond(void) { return 100; }
UInt32 TimGetTicks(void) { return 0; }
void AbtShowAbout(UInt32 c) {}
void *pumpkin_get_data(void) { return NULL; }
void pumpkin_set_data(void *d) {}
UInt32 pumpkin_get_app_creator(void) { return 'Brws'; }
int pumpkin_http_get(char *url, int timeout, int (*cb)(int, void *), void *d) { return -1; }
void pumpkin_http_abort(int h) {}

/* ---- tests ---- */

static int failures = 0;

static void expectStr(const char *what, const char *got, const char *want) {
  if (strcmp(got, want) != 0) {
    printf("FAIL %s: got \"%s\" want \"%s\"\n", what, got, want);
    failures++;
  } else {
    printf("ok   %s -> %s\n", what, got);
  }
}

static void testResolve(const char *base, const char *rel, const char *want) {
  char out[MAX_URL];
  resolveUrl(base, rel, out, sizeof(out));
  expectStr(rel, out, want);
}

static void testUrls(void) {
  char out[MAX_URL];

  printf("--- URL resolution\n");
  testResolve("http://a.com/b/c/d?q#f", "g", "http://a.com/b/c/g");
  testResolve("http://a.com/b/c/d?q#f", "./g", "http://a.com/b/c/g");
  testResolve("http://a.com/b/c/d?q#f", "g/", "http://a.com/b/c/g/");
  testResolve("http://a.com/b/c/d?q#f", "/g", "http://a.com/g");
  testResolve("http://a.com/b/c/d?q#f", "//h.org/x", "http://h.org/x");
  testResolve("http://a.com/b/c/d?q#f", "?y", "http://a.com/b/c/d?y");
  testResolve("http://a.com/b/c/d?q#f", "#s", "http://a.com/b/c/d?q#s");
  testResolve("http://a.com/b/c/d?q#f", "../g", "http://a.com/b/g");
  testResolve("http://a.com/b/c/d?q#f", "../../g", "http://a.com/g");
  testResolve("http://a.com/b/c/d?q#f", "../../../g", "http://a.com/g");
  testResolve("http://a.com/b/c/d", "https://x.y/z", "https://x.y/z");
  testResolve("http://a.com", "foo", "http://a.com/foo");
  testResolve("http://a.com", "/foo", "http://a.com/foo");
  testResolve("http://a.com", "?q=1", "http://a.com/?q=1");
  testResolve("http://a.com:8080/x/", "y", "http://a.com:8080/x/y");
  testResolve("http://a.com/x/", "..", "http://a.com/");
  testResolve("http://a.com/x/", "mailto:me@x.y", "mailto:me@x.y");

  printf("--- input normalisation\n");
  normalizeInput("  example.com/path  ", out, sizeof(out));
  expectStr("normalize", out, "http://example.com/path");
  normalizeInput("https://x.org", out, sizeof(out));
  expectStr("normalize https", out, "https://x.org");
  normalizeInput("localhost:8080/a", out, sizeof(out));
  expectStr("normalize port", out, "http://localhost:8080/a");

  urlEncode("http://a.com/x y?z=1&w", out, sizeof(out));
  expectStr("urlencode", out, "http%3A%2F%2Fa.com%2Fx%20y%3Fz%3D1%26w");
}

static void testChunked(void) {
  char body[] = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
  int n = dechunk(body, (int)strlen(body));
  body[n] = 0;
  printf("--- chunked\n");
  expectStr("dechunk", body, "hello world");
}

static void testUtf8(void) {
  char s[] = "caf\xC3\xA9 \xE2\x80\x94 \xE2\x80\x9Cq\xE2\x80\x9D \xE2\x80\xA6 \xF0\x9F\x98\x80 \xC3";
  int n = utf8ToLatin1(s, (int)strlen(s));
  s[n] = 0;
  printf("--- utf8\n");
  expectStr("utf8", s, "caf\xE9 - \"q\" ... ? \xC3");
}

static void dumpDoc(browser_t *b) {
  UInt32 i, j;
  line_t *ln;
  seg_t *sg;

  for (i = 0; i < b->nlines; i++) {
    ln = &b->lines[i];
    printf("%4u|", (unsigned)i);
    for (j = 0; j < ln->nsegs; j++) {
      sg = &b->segs[ln->firstSeg + j];
      if (j) printf(" ");
      if (sg->font == boldFont) printf("*");
      if (sg->font == largeBoldFont) printf("#");
      if (sg->link != LINK_NONE) printf("[");
      fwrite(b->text + sg->off, 1, sg->len, stdout);
      if (sg->link != LINK_NONE) printf("](%d)", sg->link);
    }
    printf("\n");
  }
}

static void testHtml(void) {
  browser_t b;
  char html[] =
    "<!DOCTYPE html><html><head><title>Test &amp; Title</title>"
    "<style>body{color:red}</style><script>var x = '<p>';</script>"
    "<base href=\"http://example.com/dir/\"></head><body>"
    "<h1>Head&shy;ing</h1><p>Hello <b>bold</b>world and <a href=\"page.html?a=1&amp;b=2\">a link</a>.</p>"
    "<!-- comment <p> --><ul><li>one</li><li>two &mdash; three</li></ul>"
    "<pre>line1\nline2</pre>foo<br><br>bar<img alt=\"pic\"><table><tr><td>c1</td><td>c2</td></tr></table>"
    "<p>A very long word: supercalifragilisticexpialidociousandthensomemorecharacters end.</p>"
    "<p>&#169; 2026 &#x41;&lt;&gt;</p></body></html>";

  memset(&b, 0, sizeof(b));
  strcpy(b.url, "http://example.com/x/y.html");
  strcpy(b.base, b.url);
  RctSetRectangle(&b.area, 0, 15, 152, 145);

  printf("--- html\n");
  parseHtml(&b, html, (int)strlen(html));
  layoutDoc(&b);
  dumpDoc(&b);
  expectStr("title", b.title, "Test & Title");
  expectStr("base", b.base, "http://example.com/dir/");
  if (b.nlinks == 1) {
    char out[MAX_URL];
    resolveUrl(b.base, b.links[0], out, sizeof(out));
    expectStr("link0", out, "http://example.com/dir/page.html?a=1&b=2");
  } else {
    printf("FAIL nlinks=%u\n", (unsigned)b.nlinks);
    failures++;
  }
  docFree(&b);
}

static void testFile(const char *path) {
  browser_t b;
  FILE *f;
  char *buf;
  long len;

  if ((f = fopen(path, "rb")) == NULL) { perror(path); return; }
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);
  buf = malloc(len + 1);
  fread(buf, 1, len, f);
  buf[len] = 0;
  fclose(f);

  memset(&b, 0, sizeof(b));
  strcpy(b.url, "http://68k.news/");
  strcpy(b.base, b.url);
  RctSetRectangle(&b.area, 0, 15, 152, 145);

  len = utf8ToLatin1(buf, (int)len);
  parseHtml(&b, buf, (int)len);
  layoutDoc(&b);
  printf("--- file %s: %u items, %u links, %u lines, title \"%s\", maxTop %u\n",
    path, (unsigned)b.nitems, (unsigned)b.nlinks, (unsigned)b.nlines, b.title, (unsigned)maxTopLine(&b));
  dumpDoc(&b);
  docFree(&b);
  free(buf);
}

int main(int argc, char *argv[]) {
  testUrls();
  testChunked();
  testUtf8();
  testHtml();
  {
    browser_t sb;
    memset(&sb, 0, sizeof(sb));
    strcpy(sb.url, "about:start");
    strcpy(sb.base, sb.url);
    RctSetRectangle(&sb.area, 0, 15, 152, 145);
    printf("--- start page\n");
    parseHtml(&sb, (char *)START_PAGE, (int)strlen(START_PAGE));
    layoutDoc(&sb);
    dumpDoc(&sb);
    if (sb.nlinks != 6) { printf("FAIL start page links=%u\n", (unsigned)sb.nlinks); failures++; }
    docFree(&sb);
  }
  if (argc > 1) testFile(argv[1]);
  printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
  return failures ? 1 : 0;
}
