// PumpkinOS for classic Mac OS (System 7.1 to Mac OS 9), 68K and PowerPC,
// built with Retro68.
//
// Like src/windows, this is a native host: the window provider draws with
// QuickDraw (the PumpkinOS screen lives in a 16-bit offscreen GWorld that is
// copied to a document window), the mouse and the keyboard come from the
// Event Manager and the audio provider plays through the Sound Manager. Like
// the console ports everything is linked statically into one application,
// there is no script engine (script.c holds the boot parameters) and the OS
// runs in single application mode, the Launcher and the applications being
// cooperative threads.
//
// Layout: the application sits in a folder next to vfs/ (the application
// PRCs in vfs/app_install are deployed on the first boot), cert.pem (the CA
// bundle for https) and the options file pumpkin.cfg. PumpkinOS keeps its
// storage in vfs/ and writes pumpkin.log / pumpkin.err there as well.
//
// The menus: the Apple menu (About, desk accessories), File (Quit), Edit
// (for the desk accessories) and Palm, whose items press the Palm buttons
// (Home, Menu, the four application buttons). The mouse is the pen.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <MacTypes.h>
#include <Quickdraw.h>
#include <Windows.h>
#include <Menus.h>
#include <Events.h>
#include <Fonts.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Sound.h>
#include <Gestalt.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Resources.h>
#include <LowMem.h>
#include <MacMemory.h>
#include <Processes.h>
#include <OSUtils.h>

#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "pwindow.h"
#include "audio.h"
#include "script.h"
#include "media.h"
#include "average.h"
#include "debug.h"
#include "macsys.h"
#include "retro68.h"

// (libpumpkin/pumpkin.h needs the PalmOS headers, whose types clash with
// the Toolbox ones)
extern void pumpkin_set_finish(int finish);

#define TAG "MAC"

#define MAX_PATH_LEN 512
#define MAX_EVENTS   64

// the options file, next to the application
#define CONFIG_FILE     "pumpkin.cfg"
#define CONFIG_MAX_ARGS 64
#define CONFIG_MAX_SIZE 4096

// the thread stacks (see libpit/macsys.c): a slot per PumpkinOS thread
// below a main stack of MAIN_STACK bytes
#define STACK_SLOT_SIZE (256 * 1024)
#define STACK_SLOTS     8
#define MAIN_STACK      (256 * 1024)

// resources (pumpkin.r)
#define ALRT_FATAL 128
#define ALRT_ABOUT 129

#define MENU_APPLE 128
#define MENU_FILE  129
#define MENU_EDIT  130
#define MENU_PALM  131

#define ITEM_ABOUT 1
#define ITEM_QUIT  1

#define WINDOW_TITLE "\pPumpkinOS"

// (missing from the Multiversal interfaces)
#ifndef suspendResumeMessage
#define suspendResumeMessage 0x01
#endif
#ifndef resumeFlag
#define resumeFlag 1
#endif
#ifndef initMono
#define initMono   0x0080
#endif
#ifndef initStereo
#define initStereo 0x00C0
#endif

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
extern void liblopenssl_set_ca_file(const char *path);
#endif

typedef struct {
  int ev;
  int arg1;
  int arg2;
} mac_event_t;

typedef struct {
  WindowPtr window;
  GWorldPtr gworld;
  PixMapHandle pixmap;
  uint8_t *pixels;
  int rowbytes;
  int width, height;
  Rect dirty;
  int has_dirty;
  mac_event_t events[MAX_EVENTS];
  int num_ev, ie, oe;
  int x, y, buttons;
  int tracking;
  int cursor_hidden;
} mac_window_t;

struct texture_t {
  int width, height, size;
  uint16_t *buffer;
};

static window_provider_t wp;
static audio_provider_t ap;
static mac_window_t *the_window;
static MenuHandle apple_menu, file_menu, edit_menu, palm_menu;
static int quit_requested;

// Mac Roman (keyboard) to the Palm character set (Windows-1252) for the
// codes 0x80 to 0xFF; 0 where there is no equivalent
static const uint8_t mac_to_palm[128] = {
  0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1, 0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
  0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3, 0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
  0x86, 0xB0, 0xA2, 0xA3, 0xA7, 0x95, 0xB6, 0xDF, 0xAE, 0xA9, 0x99, 0xB4, 0xA8, 0x00, 0xC6, 0xD8,
  0x00, 0xB1, 0x00, 0x00, 0xA5, 0xB5, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBA, 0x00, 0xE6, 0xF8,
  0xBF, 0xA1, 0xAC, 0x00, 0x83, 0x00, 0x00, 0xAB, 0xBB, 0x85, 0xA0, 0xC0, 0xC3, 0xD5, 0x8C, 0x9C,
  0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xF7, 0x00, 0xFF, 0x9F, 0x00, 0x80, 0x8B, 0x9B, 0x00, 0x00,
  0x87, 0xB7, 0x82, 0x84, 0x89, 0xC2, 0xCA, 0xC1, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xD3, 0xD4,
  0x00, 0xD2, 0xDA, 0xDB, 0xD9, 0x00, 0x88, 0x98, 0xAF, 0x00, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00
};

// --- small helpers ---

static void c2pstr_copy(const char *c, Str255 p) {
  int n = strlen(c);

  if (n > 255) n = 255;
  p[0] = n;
  memcpy(&p[1], c, n);
}

static void fatal_alert(const char *msg) {
  Str255 p;

  c2pstr_copy(msg, p);
  ParamText(p, "\p", "\p", "\p");
  InitCursor();
  StopAlert(ALRT_FATAL, NULL);
}

static void about_box(void) {
  NoteAlert(ALRT_ABOUT, NULL);
}

// --- events ---

static void put_event(mac_window_t *window, int ev, int arg1, int arg2) {
  if (window->num_ev < MAX_EVENTS) {
    window->events[window->ie].ev = ev;
    window->events[window->ie].arg1 = arg1;
    window->events[window->ie].arg2 = arg2;
    window->ie = (window->ie + 1) % MAX_EVENTS;
    window->num_ev++;
  }
}

static int get_event(mac_window_t *window, mac_event_t *ev) {
  if (window->num_ev > 0) {
    *ev = window->events[window->oe];
    window->oe = (window->oe + 1) % MAX_EVENTS;
    window->num_ev--;
    return 1;
  }

  return 0;
}

static void press_key(mac_window_t *window, int code) {
  put_event(window, WINDOW_KEYDOWN, code, 0);
  put_event(window, WINDOW_KEYUP, code, 0);
}

// the virtual key codes of the keys that have no character of their own
static int map_key(int keycode, int chr) {
  switch (keycode) {
    case 0x7A: return WINDOW_KEY_F1;
    case 0x78: return WINDOW_KEY_F2;
    case 0x63: return WINDOW_KEY_F3;
    case 0x76: return WINDOW_KEY_F4;
    case 0x60: return WINDOW_KEY_F5;
    case 0x61: return WINDOW_KEY_F6;
    case 0x62: return WINDOW_KEY_F7;
    case 0x64: return WINDOW_KEY_F8;
    case 0x65: return WINDOW_KEY_F9;
    case 0x6D: return WINDOW_KEY_F10;
    case 0x67: return WINDOW_KEY_F11;
    case 0x6F: return WINDOW_KEY_F12;
    case 0x73: return WINDOW_KEY_HOME;
    case 0x77: return WINDOW_KEY_END;
    case 0x74: return WINDOW_KEY_PGUP;
    case 0x79: return WINDOW_KEY_PGDOWN;
    case 0x72: return WINDOW_KEY_INS;     // Help
    case 0x75: return WINDOW_KEY_DEL;     // forward delete
    case 0x7B: return WINDOW_KEY_LEFT;
    case 0x7C: return WINDOW_KEY_RIGHT;
    case 0x7D: return WINDOW_KEY_DOWN;
    case 0x7E: return WINDOW_KEY_UP;
  }

  switch (chr) {
    case 0x1C: return WINDOW_KEY_LEFT;
    case 0x1D: return WINDOW_KEY_RIGHT;
    case 0x1E: return WINDOW_KEY_UP;
    case 0x1F: return WINDOW_KEY_DOWN;
    case 0x03: return 13;   // Enter
    case 0x0D: return 13;
    case 0x08: return 8;
    case 0x09: return 9;
    case 0x1B: return 27;
  }

  if (chr >= 0x80) return mac_to_palm[chr - 0x80];
  if (chr >= 0x20) return chr;

  return 0;
}

static void request_quit(void) {
  quit_requested = 1;
  pumpkin_set_finish(1);
}

static void do_menu(mac_window_t *window, long result) {
  int menu = (result >> 16) & 0xFFFF;
  int item = result & 0xFFFF;
  Str255 name;

  switch (menu) {
    case MENU_APPLE:
      if (item == ITEM_ABOUT) {
        about_box();
      } else {
        GetMenuItemText(apple_menu, item, name);
        OpenDeskAcc(name);
      }
      break;
    case MENU_FILE:
      if (item == ITEM_QUIT) request_quit();
      break;
    case MENU_EDIT:
      // only for the desk accessories
      SystemEdit(item - 1);
      break;
    case MENU_PALM:
      if (window) {
        switch (item) {
          case 1: press_key(window, WINDOW_KEY_HOME); break;
          case 2: press_key(window, WINDOW_KEY_F5);   break;  // Menu
          case 4: press_key(window, WINDOW_KEY_F1);   break;  // Date Book
          case 5: press_key(window, WINDOW_KEY_F2);   break;  // Address
          case 6: press_key(window, WINDOW_KEY_F3);   break;  // To Do
          case 7: press_key(window, WINDOW_KEY_F4);   break;  // Memo
        }
      }
      break;
  }
  HiliteMenu(0);
}

static void setup_menus(void) {
  apple_menu = NewMenu(MENU_APPLE, "\p\024");
  AppendMenu(apple_menu, "\pAbout PumpkinOS\311;(-");
  AppendResMenu(apple_menu, 'DRVR');
  InsertMenu(apple_menu, 0);

  file_menu = NewMenu(MENU_FILE, "\pFile");
  AppendMenu(file_menu, "\pQuit/Q");
  InsertMenu(file_menu, 0);

  edit_menu = NewMenu(MENU_EDIT, "\pEdit");
  AppendMenu(edit_menu, "\p(Undo/Z;(-;(Cut/X;(Copy/C;(Paste/V;(Clear");
  InsertMenu(edit_menu, 0);

  palm_menu = NewMenu(MENU_PALM, "\pPalm");
  AppendMenu(palm_menu, "\pHome/H;Menu/M;(-;Date Book/1;Address/2;To Do List/3;Memo Pad/4");
  InsertMenu(palm_menu, 0);

  DrawMenuBar();
}

// copies the dirty part of the offscreen picture to the window
static void flush_window(mac_window_t *window) {
  GrafPtr saved;
  Rect r;

  if (window == NULL || !window->has_dirty) return;
  r = window->dirty;
  window->has_dirty = 0;

  GetPort(&saved);
  SetPort(window->window);
  ForeColor(blackColor);
  BackColor(whiteColor);
  CopyBits((BitMap *)*window->pixmap, &window->window->portBits, &r, &r, srcCopy, NULL);
  SetPort(saved);
}

static void redraw_window(mac_window_t *window) {
  SetRect(&window->dirty, 0, 0, window->width, window->height);
  window->has_dirty = 1;
  flush_window(window);
}

static void handle_mouse_down(mac_window_t *window, EventRecord *ev) {
  WindowPtr w;
  GrafPtr saved;
  Point pt;
  short part;

  part = FindWindow(ev->where, &w);
  switch (part) {
    case inMenuBar:
      do_menu(window, MenuSelect(ev->where));
      break;
    case inSysWindow:
      SystemClick(ev, w);
      break;
    case inDrag:
      DragWindow(w, ev->where, &qd.screenBits.bounds);
      break;
    case inGoAway:
      if (TrackGoAway(w, ev->where)) request_quit();
      break;
    case inContent:
      if (w != FrontWindow()) {
        SelectWindow(w);
      } else if (window && w == window->window) {
        pt = ev->where;
        GetPort(&saved);
        SetPort(w);
        GlobalToLocal(&pt);
        SetPort(saved);
        window->x = pt.h;
        window->y = pt.v;
        window->buttons = 1;
        window->tracking = 1;
        put_event(window, WINDOW_MOTION, pt.h, pt.v);
        put_event(window, WINDOW_BUTTONDOWN, 1, 0);
      }
      break;
  }
}

static void handle_key(mac_window_t *window, EventRecord *ev) {
  int chr = ev->message & charCodeMask;
  int keycode = (ev->message & keyCodeMask) >> 8;
  int code;

  if (ev->modifiers & cmdKey) {
    if (ev->what == keyDown) do_menu(window, MenuKey(chr));
    return;
  }
  if (window == NULL) return;

  if ((code = map_key(keycode, chr)) != 0) {
    press_key(window, code);
  }
}

// processes the pending Mac events (sleep: ticks WaitNextEvent may give to
// the other applications when there is nothing to do)
static void pump_events(mac_window_t *window, uint32_t sleep) {
  EventRecord ev;
  GrafPtr saved;
  Point pt;

  flush_window(window);

  while (WaitNextEvent(everyEvent, &ev, sleep, NULL)) {
    sleep = 0;
    switch (ev.what) {
      case mouseDown:
        handle_mouse_down(window, &ev);
        break;
      case mouseUp:
        if (window && window->tracking) {
          window->tracking = 0;
          window->buttons = 0;
          put_event(window, WINDOW_BUTTONUP, 1, 0);
        }
        break;
      case keyDown:
      case autoKey:
        handle_key(window, &ev);
        break;
      case updateEvt:
        BeginUpdate((WindowPtr)ev.message);
        if (window && (WindowPtr)ev.message == window->window) {
          EndUpdate((WindowPtr)ev.message);
          redraw_window(window);
        } else {
          EndUpdate((WindowPtr)ev.message);
        }
        break;
      case osEvt:
        // resume: the window may have been covered
        if (((ev.message >> 24) & 0xFF) == suspendResumeMessage && (ev.message & resumeFlag) && window) {
          redraw_window(window);
        }
        break;
      case kHighLevelEvent:
        break;
    }
  }

  // the pen while the button is held down
  if (window && window->tracking) {
    GetPort(&saved);
    SetPort(window->window);
    GetMouse(&pt);
    SetPort(saved);
    if (pt.h != window->x || pt.v != window->y) {
      window->x = pt.h;
      window->y = pt.v;
      put_event(window, WINDOW_MOTION, pt.h, pt.v);
    }
    if (!StillDown() && !Button()) {
      window->tracking = 0;
      window->buttons = 0;
      put_event(window, WINDOW_BUTTONUP, 1, 0);
    }
  }
}

// --- window provider ---

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  mac_window_t *window;
  Rect bounds, screen, r;
  int left, top, mbar;
  QDErr err;

  debug(DEBUG_INFO, TAG, "window_create(%d,%d,%d)", encoding, *width, *height);

  if (the_window) return NULL;
  if ((window = sys_calloc(1, sizeof(mac_window_t))) == NULL) return NULL;

  window->width = *width;
  window->height = *height;

  // centered on the main screen, below the menu bar
  screen = qd.screenBits.bounds;
  mbar = GetMBarHeight();
  left = screen.left + ((screen.right - screen.left) - window->width) / 2;
  top = screen.top + mbar + 20 + ((screen.bottom - screen.top - mbar - 20) - window->height) / 2;
  if (left < screen.left + 2) left = screen.left + 2;
  if (top < screen.top + mbar + 20) top = screen.top + mbar + 20;
  SetRect(&bounds, left, top, left + window->width, top + window->height);

  window->window = NewCWindow(NULL, &bounds, WINDOW_TITLE, true, noGrowDocProc, (WindowPtr)-1, true, 0);
  if (window->window == NULL) {
    debug(DEBUG_ERROR, TAG, "NewCWindow failed");
    sys_free(window);
    return NULL;
  }

  // the picture, 16 bits per pixel (xRRRRRGGGGGBBBBB), in the partition or
  // else in temporary memory
  SetRect(&r, 0, 0, window->width, window->height);
  err = NewGWorld(&window->gworld, 16, &r, NULL, NULL, 0);
  if (err != noErr || window->gworld == NULL) {
    err = NewGWorld(&window->gworld, 16, &r, NULL, NULL, useTempMem);
  }
  if (err != noErr || window->gworld == NULL) {
    debug(DEBUG_ERROR, TAG, "NewGWorld failed (%d)", err);
    DisposeWindow(window->window);
    sys_free(window);
    return NULL;
  }
  window->pixmap = GetGWorldPixMap(window->gworld);
  LockPixels(window->pixmap);
  window->pixels = (uint8_t *)GetPixBaseAddr(window->pixmap);
  window->rowbytes = (*window->pixmap)->rowBytes & 0x3FFF;
  sys_memset(window->pixels, 0xFF, window->rowbytes * window->height);

  SetPort(window->window);
  the_window = window;
  redraw_window(window);

  return (window_t *)window;
}

static int window_destroy(window_t *_window) {
  mac_window_t *window = (mac_window_t *)_window;

  debug(DEBUG_INFO, TAG, "window_destroy");

  if (window) {
    if (window->cursor_hidden) ShowCursor();
    if (window->gworld) {
      UnlockPixels(window->pixmap);
      DisposeGWorld(window->gworld);
    }
    if (window->window) DisposeWindow(window->window);
    if (the_window == window) the_window = NULL;
    sys_free(window);
  }

  return 0;
}

static int window_render(window_t *_window) {
  flush_window((mac_window_t *)_window);
  return 0;
}

static texture_t *window_create_texture(window_t *_window, int width, int height) {
  texture_t *texture;

  if ((texture = sys_malloc(sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->size = width * height * sizeof(uint16_t);
    if ((texture->buffer = sys_malloc(texture->size)) == NULL) {
      sys_free(texture);
      texture = NULL;
    }
  }

  return texture;
}

static int window_destroy_texture(window_t *window, texture_t *texture) {
  if (texture) {
    if (texture->buffer) sys_free(texture->buffer);
    sys_free(texture);
  }

  return 0;
}

static int window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  uint16_t *buffer;
  int i, tpitch, tindex;

  if (texture && src) {
    buffer = (uint16_t *)src;
    tpitch = texture->width;
    tindex = ty * tpitch + tx;
    for (i = 0; i < h; i++) {
      sys_memcpy(&texture->buffer[tindex], &buffer[tindex], w * sizeof(uint16_t));
      tindex += tpitch;
    }
  }

  return 0;
}

static int window_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  if (texture && texture->buffer && raw) {
    sys_memcpy(texture->buffer, raw, texture->size);
  }

  return 0;
}

static int window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  mac_window_t *window = (mac_window_t *)_window;
  uint16_t *src, *dst, p;
  int i, j, d;

  if (texture == NULL || window == NULL || window->pixels == NULL) return 0;

  if (x < 0) { tx -= x; w += x; x = 0; }
  if (x + w > window->width) { d = x + w - window->width; w -= d; }
  if (y < 0) { ty -= y; h += y; y = 0; }
  if (y + h > window->height) { d = y + h - window->height; h -= d; }
  if (w <= 0 || h <= 0) return 0;

  // RGB565 (PumpkinOS) to xRGB1555 (QuickDraw)
  for (i = 0; i < h; i++) {
    src = &texture->buffer[(ty + i) * texture->width + tx];
    dst = (uint16_t *)(window->pixels + (y + i) * window->rowbytes) + x;
    for (j = 0; j < w; j++) {
      p = src[j];
      dst[j] = ((p >> 1) & 0x7FE0) | (p & 0x001F);
    }
  }

  if (window->has_dirty) {
    if (x < window->dirty.left) window->dirty.left = x;
    if (y < window->dirty.top) window->dirty.top = y;
    if (x + w > window->dirty.right) window->dirty.right = x + w;
    if (y + h > window->dirty.bottom) window->dirty.bottom = y + h;
  } else {
    SetRect(&window->dirty, x, y, x + w, y + h);
    window->has_dirty = 1;
  }

  return 0;
}

static int window_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  if (texture) {
    window_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return 0;
}

static int window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  mac_window_t *window = (mac_window_t *)_window;
  mac_event_t event;
  uint64_t t, end;

  if (window == NULL) return 0;

  pump_events(window, 0);

  if (window->num_ev == 0 && wait != 0) {
    end = wait > 0 ? sys_get_clock() + (uint64_t)wait * 1000 : 0;
    for (;;) {
      if (thread_must_end() || quit_requested) break;
      // a tick at a time: the other threads and the other applications
      // run in between
      sys_usleep(wait > 0 && wait < 17 ? 1000 : 16667);
      pump_events(window, 0);
      if (window->num_ev) break;
      if (wait > 0) {
        t = sys_get_clock();
        if (t >= end) break;
      }
    }
  }

  if (get_event(window, &event)) {
    *arg1 = event.arg1;
    *arg2 = event.arg2;
    return event.ev;
  }

  return 0;
}

static void window_status(window_t *_window, int *x, int *y, int *buttons) {
  mac_window_t *window = (mac_window_t *)_window;

  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static int window_show_cursor(window_t *_window, int show) {
  mac_window_t *window = (mac_window_t *)_window;

  if (show && window->cursor_hidden) {
    ShowCursor();
    window->cursor_hidden = 0;
  } else if (!show && !window->cursor_hidden) {
    HideCursor();
    window->cursor_hidden = 1;
  }

  return 0;
}

static void window_title(window_t *_window, char *title) {
  mac_window_t *window = (mac_window_t *)_window;
  Str255 p;

  if (window && title) {
    c2pstr_copy(title, p);
    SetWTitle(window->window, p);
  }
}

static int window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&wp, window, x, y, ms);
}

// --- audio provider (Sound Manager) ---

#define TAG_AUDIO "audio"
#define SND_NUM_BUFFERS 3

typedef struct {
  char *tag;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  int pcm, channels, rate;
  uint8_t *buffer;
  uint32_t bsize;   // bytes per channel in one chunk
} mac_audio_t;

// the sound channel owned by the audio thread
typedef struct {
  SndChannelPtr chan;
  SndCallbackUPP callback;
  int pcm, channels, rate;
  ExtSoundHeaderPtr hdr[SND_NUM_BUFFERS];
  uint8_t *obuf[SND_NUM_BUFFERS];
  uint32_t osize[SND_NUM_BUFFERS];
  int queued[SND_NUM_BUFFERS];
  volatile int done[SND_NUM_BUFFERS];
  int next;
} mac_snd_t;

typedef struct {
  int pcm, channels, rate;
} mac_audio_arg_t;

// called by the Sound Manager (at interrupt time) when a buffer has been
// played: only marks it, through the channel's userInfo (no globals)
static pascal void snd_callback(SndChannelPtr chan, SndCommand *cmd) {
  mac_snd_t *snd = (mac_snd_t *)chan->userInfo;
  int i = cmd->param1;

  if (snd && i >= 0 && i < SND_NUM_BUFFERS) snd->done[i] = 1;
}

static int audio_mixer_init(void) {
  return 0;
}

static int audio_mixer_play(uint8_t *buf, uint32_t len, int volume) {
  // (no MIDI synthesizer)
  return -1;
}

static int audio_mixer_stop(void) {
  return 0;
}

static void audio_destructor(void *p) {
  mac_audio_t *audio = (mac_audio_t *)p;

  if (audio) {
    if (audio->buffer) sys_free(audio->buffer);
    sys_free(audio);
  }
}

static int audio_sample_size(int pcm) {
  switch (pcm) {
    case PCM_U8:  return 1;
    case PCM_S16: return 2;
    case PCM_S32: return 4;
  }
  return 0;
}

static audio_t audio_create(int pcm, int channels, int rate, void *data) {
  mac_audio_t *audio;
  int ptr = -1;

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(mac_audio_t))) != NULL) {
      audio->pcm = pcm;
      audio->channels = channels;
      audio->rate = rate;
      // about a tenth of a second per chunk
      audio->bsize = audio_sample_size(pcm) * (rate / 10);
      if (audio->bsize > 8192) audio->bsize = 8192;
      audio->bsize -= audio->bsize % audio_sample_size(pcm);
      audio->buffer = sys_calloc(1, audio->bsize * audio->channels);
      audio->tag = TAG_AUDIO;
      if (audio->buffer == NULL || (ptr = ptr_new(audio, audio_destructor)) == -1) {
        if (audio->buffer) sys_free(audio->buffer);
        sys_free(audio);
        ptr = -1;
      }
    }
  }

  return ptr;
}

static int audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  mac_audio_t *audio;
  uint32_t ptr;
  int r = -1;

  if (handle && (audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    audio->getaudio = getaudio;
    audio->data = data;
    ptr_unlock(_audio, TAG_AUDIO);
    ptr = _audio;
    r = thread_client_write(handle, (uint8_t *)&ptr, sizeof(uint32_t)) == sizeof(uint32_t) ? 0 : -1;
  }

  return r;
}

static int audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

// all samples are handled internally as signed 32 bit full scale values

static int32_t audio_get_sample(uint8_t *buffer, int i, int pcm) {
  switch (pcm) {
    case PCM_U8:  return ((int32_t)buffer[i] - 128) << 24;
    case PCM_S16: return ((int32_t)((int16_t *)buffer)[i]) << 16;
    case PCM_S32: return ((int32_t *)buffer)[i];
  }
  return 0;
}

static void audio_put_sample(uint8_t *buffer, int32_t s, int i, int pcm) {
  switch (pcm) {
    case PCM_U8:  buffer[i] = (uint8_t)((s >> 24) + 128); break;
    case PCM_S16: ((int16_t *)buffer)[i] = (int16_t)(s >> 16); break;
  }
}

// converts len bytes of the stream format to the channel format (S32
// becomes S16, the Sound Manager plays 8 and 16 bit samples; the rate is
// left to the Sound Manager)
static uint32_t audio_convert(mac_snd_t *snd, int i, mac_audio_t *audio, uint32_t len) {
  uint32_t n, nframes, size, k;
  int ss_in, ss_out;

  ss_in = audio_sample_size(audio->pcm);
  ss_out = audio_sample_size(snd->pcm);
  nframes = len / (ss_in * audio->channels);
  if (nframes == 0) return 0;
  size = nframes * ss_out * snd->channels;

  if (snd->obuf[i] == NULL || snd->osize[i] < size) {
    if (snd->obuf[i]) DisposePtr((Ptr)snd->obuf[i]);
    snd->obuf[i] = (uint8_t *)NewPtr(size);
    snd->osize[i] = snd->obuf[i] ? size : 0;
    if (snd->obuf[i] == NULL) return 0;
  }

  if (audio->pcm == snd->pcm) {
    sys_memcpy(snd->obuf[i], audio->buffer, size);
  } else {
    n = nframes * audio->channels;
    for (k = 0; k < n; k++) {
      audio_put_sample(snd->obuf[i], audio_get_sample(audio->buffer, k, audio->pcm), k, snd->pcm);
    }
  }

  return size;
}

static int snd_open(mac_snd_t *snd) {
  OSErr err;
  int i;

  snd->callback = NewSndCallbackUPP(snd_callback);
  snd->chan = NULL;
  if ((err = SndNewChannel(&snd->chan, sampledSynth, snd->channels == 2 ? initStereo : initMono, snd->callback)) != noErr) {
    debug(DEBUG_ERROR, TAG, "SndNewChannel failed (%d)", err);
    snd->chan = NULL;
    return -1;
  }
  snd->chan->userInfo = (long)snd;

  for (i = 0; i < SND_NUM_BUFFERS; i++) {
    if ((snd->hdr[i] = (ExtSoundHeaderPtr)NewPtrClear(sizeof(ExtSoundHeader))) == NULL) return -1;
    snd->done[i] = 0;
    snd->queued[i] = 0;
  }

  debug(DEBUG_INFO, TAG, "open audio %d,%d,%d", snd->pcm, snd->channels, snd->rate);
  return 0;
}

// waits until buffer i (if queued) has been played
static int snd_wait(mac_snd_t *snd, int i) {
  if (snd->queued[i]) {
    while (!snd->done[i]) {
      if (thread_must_end()) return -1;
      sys_usleep(1000);
    }
    snd->queued[i] = 0;
  }

  return 0;
}

static int snd_write(mac_snd_t *snd, int i, uint32_t len) {
  ExtSoundHeaderPtr hdr = snd->hdr[i];
  SndCommand cmd;
  int ss = audio_sample_size(snd->pcm);
  OSErr err;

  sys_memset(hdr, 0, sizeof(ExtSoundHeader));
  hdr->samplePtr = (Ptr)snd->obuf[i];
  hdr->numChannels = snd->channels;
  hdr->sampleRate = (Fixed)((uint32_t)snd->rate << 16);
  hdr->encode = extSH;
  hdr->baseFrequency = 60;   // middle C
  hdr->numFrames = len / (ss * snd->channels);
  hdr->sampleSize = ss * 8;

  snd->done[i] = 0;
  cmd.cmd = bufferCmd;
  cmd.param1 = 0;
  cmd.param2 = (long)hdr;
  if ((err = SndDoCommand(snd->chan, &cmd, false)) != noErr) {
    debug(DEBUG_ERROR, TAG, "bufferCmd failed (%d)", err);
    return -1;
  }
  cmd.cmd = callBackCmd;
  cmd.param1 = i;
  cmd.param2 = 0;
  if ((err = SndDoCommand(snd->chan, &cmd, false)) != noErr) {
    debug(DEBUG_ERROR, TAG, "callBackCmd failed (%d)", err);
    return -1;
  }
  snd->queued[i] = 1;

  return 0;
}

static void snd_close(mac_snd_t *snd) {
  int i;

  if (snd->chan) {
    debug(DEBUG_INFO, TAG, "close audio");
    SndDisposeChannel(snd->chan, true);
    snd->chan = NULL;
  }
  for (i = 0; i < SND_NUM_BUFFERS; i++) {
    if (snd->hdr[i]) DisposePtr((Ptr)snd->hdr[i]);
    if (snd->obuf[i]) DisposePtr((Ptr)snd->obuf[i]);
    snd->hdr[i] = NULL;
    snd->obuf[i] = NULL;
    snd->osize[i] = 0;
  }
#if TARGET_RT_MAC_CFM
  if (snd->callback) DisposeSndCallbackUPP(snd->callback);
#endif
  snd->callback = NULL;
}

static int audio_action(void *_arg) {
  mac_audio_arg_t *arg = (mac_audio_arg_t *)_arg;
  mac_audio_t *audio;
  mac_snd_t *snd;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr, wait, olen;
  int len1, len2, r, i;

  debug(DEBUG_INFO, TAG, "audio thread starting");
  // (the callback reaches it at interrupt time: not on the task stack)
  if ((snd = (mac_snd_t *)NewPtrClear(sizeof(mac_snd_t))) == NULL) {
    sys_free(arg);
    return 0;
  }
  snd->pcm = arg->pcm == PCM_S32 ? PCM_S16 : arg->pcm;
  snd->channels = arg->channels;
  snd->rate = arg->rate;
  wait = 2000;
  ptr = 0;

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(wait, &msg, &msglen)) == -1) {
      debug(DEBUG_ERROR, TAG, "error receiving msg");
      break;
    }

    if (r == 1) {
      if (msg) {
        wait = 2000;
        if (msglen == 4) {
          ptr = *((uint32_t *)msg);
          wait = 0;
        }
        sys_free(msg);
      }
      continue;
    }

    if (ptr == 0) {
      wait = 2000;
      continue;
    }

    if ((audio = ptr_lock(ptr, TAG_AUDIO)) == NULL) {
      ptr = 0;
      wait = 2000;
      continue;
    }

    if (snd->chan == NULL && snd_open(snd) == -1) {
      snd_close(snd);
      ptr_unlock(ptr, TAG_AUDIO);
      ptr = 0;
      wait = 2000;
      continue;
    }

    len1 = audio->bsize * audio->channels;
    len2 = audio->getaudio ? audio->getaudio(audio->buffer, len1, audio->data) : -1;
    if (len2 > len1) len2 = len1;

    if (len2 > 0) {
      i = snd->next;
      // the oldest buffer must have been played before it is reused
      if (snd_wait(snd, i) == -1) {
        ptr_unlock(ptr, TAG_AUDIO);
        break;
      }
      if ((olen = audio_convert(snd, i, audio, (uint32_t)len2)) > 0) {
        if (snd_write(snd, i, olen) == 0) {
          snd->next = (i + 1) % SND_NUM_BUFFERS;
        }
      }
      wait = 0;
    }
    ptr_unlock(ptr, TAG_AUDIO);

    if (len2 <= 0) {
      ptr = 0;
      wait = 2000;
    }
  }

  snd_close(snd);
  DisposePtr((Ptr)snd);
  sys_free(arg);
  debug(DEBUG_INFO, TAG, "audio thread exiting");

  return 0;
}

static int audio_init(int pcm, int channels, int rate) {
  mac_audio_arg_t *arg;

  if (audio_sample_size(pcm) == 0 || (channels != 1 && channels != 2) || rate <= 0) {
    debug(DEBUG_ERROR, TAG, "audio_init invalid format %d,%d,%d", pcm, channels, rate);
    return -1;
  }

  if ((arg = sys_calloc(1, sizeof(mac_audio_arg_t))) == NULL) return -1;
  arg->pcm = pcm;
  arg->channels = channels;
  arg->rate = rate;

  return thread_begin("AUDIO", audio_action, arg);
}

static int audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

// --- start up ---

static int dir_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && (st.mode & SYS_IFDIR);
}

static int file_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && !(st.mode & SYS_IFDIR);
}

static void mkdir_if_missing(const char *path) {
  if (!dir_exists(path) && sys_mkdir(path) != 0) {
    debug(DEBUG_ERROR, TAG, "could not create folder \"%s\"", path);
  }
}

// an options file: the options separated by blanks or line breaks, '#'
// starts a comment. Returns the number of arguments stored in argv.
static int read_config(const char *dir, char **argv, int max, char *buf, int size) {
  char path[MAX_PATH_LEN];
  int fd, n, i, argc = 0, comment = 0;

  if (max <= 0) return 0;
  sys_snprintf(path, sizeof(path)-1, "%s%s", dir, CONFIG_FILE);
  if ((fd = sys_open(path, SYS_READ)) == -1) return 0;
  n = sys_read(fd, (uint8_t *)buf, size-1);
  sys_close(fd);
  if (n <= 0) return 0;
  buf[n] = 0;

  for (i = 0; i < n && argc < max; i++) {
    if (buf[i] == '#') comment = 1;
    if (buf[i] == '\n' || buf[i] == '\r') comment = 0;
    if (comment || buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
      buf[i] = 0;
    } else if (i == 0 || buf[i-1] == 0) {
      argv[argc++] = &buf[i];
    }
  }

  return argc;
}

static int init_toolbox(void) {
  long qdversion = 0;
  int i;

  // the thread stacks and a larger main stack, before the heap grows
  mac_stacks_reserve(STACK_SLOT_SIZE, STACK_SLOTS, MAIN_STACK);
  MaxApplZone();
  for (i = 0; i < 16; i++) MoreMasters();

#if !TARGET_API_MAC_CARBON
  InitGraf(&qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(NULL);
#endif
  FlushEvents(everyEvent, 0);
  InitCursor();

  // Color QuickDraw with offscreen GWorlds (32-Bit QuickDraw, System 7)
  if (Gestalt(gestaltQuickdrawVersion, &qdversion) != noErr || qdversion < gestalt32BitQD) {
    fatal_alert("PumpkinOS needs a Macintosh with Color QuickDraw (32-Bit QuickDraw).");
    return -1;
  }

  setup_menus();
  return 0;
}

int main(int argc, char *argv[]) {
  char home[MAX_PATH_LEN], vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], path[MAX_PATH_LEN];
  char logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], hfs[MAX_PATH_LEN];
  char *args[CONFIG_MAX_ARGS + 1];
  static char config[CONFIG_MAX_SIZE];
  const char *log_arg = NULL;
  Rect screen;
  int i, nargs = 0, level = DEBUG_INFO, net = 1, w, h, mbar, user_size = 0;
  char *d, *p;

  if (init_toolbox() != 0) return 1;

  mac_fs_init();
  sys_strncpy(home, mac_app_dir(), sizeof(home)-1);
  home[sizeof(home)-1] = 0;

  // the options: pumpkin.cfg next to the application (a Mac application
  // has no command line)
  args[nargs++] = "PumpkinOS";
  nargs += read_config(home, &args[nargs], CONFIG_MAX_ARGS - nargs, config, CONFIG_MAX_SIZE);
  for (i = 1; i < argc && nargs < CONFIG_MAX_ARGS; i++) args[nargs++] = argv[i];
  args[nargs] = NULL;

  for (i = 1; i < nargs; i++) {
    if (!strcmp(args[i], "-nodia")) {
      retro68_script_set("dia", 0);
    } else if (!strcmp(args[i], "-nonet")) {
      net = 0;
    } else if (i < nargs-1 && !strcmp(args[i], "-app")) {
      retro68_script_set_launcher(args[++i]);
    } else if (i < nargs-1 && !strcmp(args[i], "-log")) {
      log_arg = args[++i];
    } else if (i < nargs-1 && !strcmp(args[i], "-d")) {
      d = args[++i];
      if ((p = strchr(d, ':')) != NULL) {
        debug_setsyslevel(p+1, sys_atoi(d));
      } else {
        level = sys_atoi(d);
      }
    } else if (i < nargs-1 && args[i][0] == '-' && retro68_script_set(args[i]+1, sys_atoi(args[i+1])) == 0) {
      if (!strcmp(args[i], "-width") || !strcmp(args[i], "-height") || !strcmp(args[i], "-dia")) user_size = 1;
      i++;
    }
  }

  // the default 320x544 window does not fit a 640x480 screen: use the
  // 320x320 Palm screen without the input area there (the Palm menu and the
  // keyboard stand in for the buttons)
  screen = qd.screenBits.bounds;
  mbar = GetMBarHeight();
  w = retro68_script_get("width");
  h = retro68_script_get("height");
  if (!user_size && (h + mbar + 24 > screen.bottom - screen.top || w + 8 > screen.right - screen.left)) {
    retro68_script_set("height", 320);
    retro68_script_set("dia", 0);
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  sys_snprintf(vfsdir, sizeof(vfsdir)-1, "%svfs/", home);
  sys_snprintf(tmpdir, sizeof(tmpdir)-1, "%svfs/tmp", home);
  if (log_arg) {
    sys_strncpy(logfile, log_arg, sizeof(logfile)-1);
  } else {
    sys_snprintf(logfile, sizeof(logfile)-1, "%spumpkin.log", home);
  }
  sys_snprintf(errfile, sizeof(errfile)-1, "%spumpkin.err", home);

  // newlib's stdio takes HFS paths
  if (mac_hfs_path(errfile, hfs, sizeof(hfs)) == 0) freopen(hfs, "w", stderr);
  if (sys_strcmp(logfile, "stderr") && mac_hfs_path(logfile, hfs, sizeof(hfs)) == 0) {
    debug_init(hfs);
  } else {
    debug_init(NULL);
  }
  debug(DEBUG_INFO, TAG, "%s starting on %s (%s), home \"%s\"", SYSTEM_NAME, SYSTEM_OS,
#ifdef __m68k__
    "68K",
#else
    "PowerPC",
#endif
    home);
  for (i = 1; i < nargs; i++) debug(DEBUG_INFO, TAG, "option \"%s\"", args[i]);
  debug(DEBUG_INFO, TAG, "screen %dx%d, PumpkinOS screen %dx%d%s, %u KB free", screen.right - screen.left, screen.bottom - screen.top,
    retro68_script_get("width"), retro68_script_get("height"), retro68_script_get("dia") ? " with the input area" : "", sys_mac_heap_avail() / 1024);

  // the writable parts of the VFS
  mkdir_if_missing(vfsdir);
  sys_snprintf(path, sizeof(path)-1, "%sapp_install", vfsdir);
  mkdir_if_missing(path);
  sys_snprintf(path, sizeof(path)-1, "%sapp_storage", vfsdir);
  mkdir_if_missing(path);
  sys_snprintf(path, sizeof(path)-1, "%sapp_card", vfsdir);
  mkdir_if_missing(path);
  sys_snprintf(path, sizeof(path)-1, "%sapp_card/PALM", vfsdir);
  mkdir_if_missing(path);
  sys_snprintf(path, sizeof(path)-1, "%sapp_card/PALM/Programs", vfsdir);
  mkdir_if_missing(path);
  mkdir_if_missing(tmpdir);

  if (net) {
    if (mac_net_init() != 0) debug(DEBUG_INFO, TAG, "no TCP/IP: networking disabled");
  } else {
    debug(DEBUG_INFO, TAG, "networking disabled (-nonet)");
  }

  ptr_init();
  thread_init();
  vfs_init();

  sys_memset(&wp, 0, sizeof(window_provider_t));
  wp.create = window_create;
  wp.destroy = window_destroy;
  wp.render = window_render;
  wp.create_texture = window_create_texture;
  wp.destroy_texture = window_destroy_texture;
  wp.update_texture = window_update_texture;
  wp.draw_texture = window_draw_texture;
  wp.update_texture_rect = window_update_texture_rect;
  wp.draw_texture_rect = window_draw_texture_rect;
  wp.event2 = window_event2;
  wp.status = window_status;
  wp.show_cursor = window_show_cursor;
  wp.title = window_title;
  wp.average = window_average;

  sys_memset(&ap, 0, sizeof(audio_provider_t));
  ap.init = audio_init;
  ap.finish = audio_finish;
  ap.create = audio_create;
  ap.start = audio_start;
  ap.destroy = audio_destroy;
  ap.mixer_init = audio_mixer_init;
  ap.mixer_play = audio_mixer_play;
  ap.mixer_stop = audio_mixer_stop;

  script_set_pointer(0, WINDOW_PROVIDER, &wp);
  script_set_pointer(0, AUDIO_PROVIDER, &ap);

  vfs_local_mount(vfsdir, "/");
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // the CA bundle next to the application, as a POSIX path: liblopenssl
  // reads it through the File Manager layer (newlib's stdio, which OpenSSL
  // would use for SSL_CERT_FILE, needs HFS paths and does not find it)
  sys_snprintf(path, sizeof(path)-1, "%scert.pem", home);
  if (file_exists(path)) {
    debug(DEBUG_INFO, TAG, "CA bundle \"%s\"", path);
    liblopenssl_set_ca_file(path);
  } else {
    debug(DEBUG_ERROR, TAG, "no cert.pem next to the application: https cannot verify servers");
  }
  liblopenssl_load();
  liblopenssl_init(0, 0);
#endif

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  mac_net_finish();
  debug(DEBUG_INFO, TAG, "%s stopping", SYSTEM_NAME);
  debug_close();

  return 0;
}
