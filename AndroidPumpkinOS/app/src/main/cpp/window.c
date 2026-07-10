#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include <jni.h>
#include <android/bitmap.h>

#include "sys.h"
#include "script.h"
#include "pwindow.h"
#include "debug.h"
#include "xalloc.h"
#include "pitapp.h"

// 32-bit ARGB framebuffer (matches Bitmap.Config.ARGB_8888 and PumpkinOS's
// 32-bit ABGR host encoding). Was uint16_t (RGB565); the 16-bit host path had
// rendering/corruption issues, so the port now uses the 32-bit path like desktop.
typedef uint32_t pixel_t;

struct texture_t {
  int width, height;
  pixel_t *buf;
};

typedef struct {
  int width, height;
  int x, y, buttons, mods;
  uint32_t format;
  int64_t shift_up;
} android_window_t;

typedef struct {
  int action, x, y, key;
} touch_event_t;

static window_provider_t window_provider;
// A JNIEnv* is per-thread and must never be cached and reused on another
// thread. PumpkinOS renders from its own (long-lived) thread, which is not the
// thread that handed us the bitmap, and Android can also recreate that thread
// across onStop/onStart. So we cache the process-global JavaVM* (which IS
// shareable) plus a global reference to the bitmap, and resolve the calling
// thread's JNIEnv on demand. Previously the cached JNIEnv* was used cross-thread,
// which aborts under CheckJNI (debug) and silently corrupts memory in release.
static JavaVM *javaVM;
static jobject bitmap;

// The PumpkinOS compositor writes the shared bitmap from the render thread(s)
// via AndroidBitmap_lockPixels, while the UI thread reads it via
// Canvas.drawBitmap (CustomView.onDraw). AndroidBitmap_lockPixels does NOT
// exclude the Java-side draw, so without a lock the UI thread can composite a
// row that is being memcpy'd underneath it -> tearing/garbled scanlines. This
// mutex is taken by both the native blits below and (through
// window_lock_bitmap/window_unlock_bitmap, exposed to Java) around drawBitmap,
// making writer and reader mutually exclusive. It is non-recursive: the render
// thread never draws to the Canvas and the UI thread never blits a texture, so
// the two sides never nest.
static pthread_mutex_t bitmap_mutex = PTHREAD_MUTEX_INITIALIZER;

void window_lock_bitmap(void) {
  pthread_mutex_lock(&bitmap_mutex);
}

void window_unlock_bitmap(void) {
  pthread_mutex_unlock(&bitmap_mutex);
}

// PumpkinOS runs every launched Palm app on its own pthread (threadptr.c), and
// those threads render through this window provider. The first render on such a
// thread attaches it to the JVM via AttachCurrentThread. ART *aborts the whole
// process* ("native thread exited without detaching") if a thread that attached
// itself exits without calling DetachCurrentThread — and threadptr.c's thread
// wrapper has no knowledge of the JVM, so it never detaches. That is exactly why
// the Launcher (running on the already-attached executor thread, which we reach
// via GetEnv and therefore never attach) works, but launching a nested app —
// which spawns a new pthread that later exits — crashes with a SIGSEGV.
//
// Register a thread-specific key whose destructor runs when each such thread
// exits and detaches it from the JVM. Only threads we actually attached get the
// key set, so the destructor never touches threads owned by the JVM itself.
static pthread_key_t detach_key;
static pthread_once_t detach_key_once = PTHREAD_ONCE_INIT;

static void window_detach_thread(void *unused) {
  if (javaVM != NULL) {
    (*javaVM)->DetachCurrentThread(javaVM);
  }
}

static void window_make_detach_key(void) {
  pthread_key_create(&detach_key, window_detach_thread);
}

#define MAX_EVENTS 16
touch_event_t events[MAX_EVENTS];
int numEvents;
int idxIn;
int idxOut;

// Return the JNIEnv for the current thread, attaching it to the VM if needed.
static JNIEnv *window_get_env(void) {
  JNIEnv *e = NULL;

  if (javaVM == NULL) return NULL;
  if ((*javaVM)->GetEnv(javaVM, (void **)&e, JNI_VERSION_1_6) == JNI_OK) return e;
  if ((*javaVM)->AttachCurrentThread(javaVM, &e, NULL) == 0) {
    // Arrange for this thread to detach from the JVM when it exits. The value
    // must be non-NULL for the pthread destructor to fire.
    pthread_once(&detach_key_once, window_make_detach_key);
    pthread_setspecific(detach_key, (void *)1);
    return e;
  }

  return NULL;
}

void window_bitmap(JNIEnv *_env, jobject _bitmap) {
  if (_env != NULL && javaVM == NULL) {
    (*_env)->GetJavaVM(_env, &javaVM);
  }
  // _bitmap is a JNI *global* reference (created by the caller in
  // native-lib.cpp's pitUpdate). Release the previous one before overwriting it:
  // start() -> pitUpdate() runs on every onStart, so without this each
  // background/foreground cycle would leak a global ref, and the JNI global
  // reference table is finite (~51k) — exhausting it aborts the process.
  if (_env != NULL && bitmap != NULL && bitmap != _bitmap) {
    (*_env)->DeleteGlobalRef(_env, bitmap);
  }
  bitmap = _bitmap;
}

static texture_t *window_create_texture(window_t *window, int width, int height) {
  texture_t *texture;

  if ((texture = xcalloc(1, sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    if ((texture->buf = xcalloc(width*height, sizeof(pixel_t))) == NULL) {
      xfree(texture);
      texture = NULL;
    }

    xmemset(texture->buf, 0xFF, width * height * sizeof(pixel_t));
  }

  return texture;
}

int window_update_texture(window_t *_window, texture_t *texture, uint8_t *raw) {
  if (texture && raw) {
    xmemcpy(texture->buf, raw, texture->width * texture->height * sizeof(pixel_t));
  }

  return 0;
}

int window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  AndroidBitmapInfo bi;
  pixel_t *p, *src;
  void *pixels;
  JNIEnv *env = window_get_env();
  int i, dst_stride_px, n, w;

  if (env && bitmap && texture) {
    // On failure bi is left uninitialized; using its garbage width/height/stride
    // for the clamps and destination pointer below would corrupt memory. This
    // matters most during the onStop teardown race, when the Java bitmap can be
    // recycled while this render thread is still running.
    if (AndroidBitmap_getInfo(env, bitmap, &bi) != 0) return 0;

    // Clamp the copy width to whatever fits on the destination bitmap,
    // mirroring SDL_RenderCopy's automatic clipping. Without this an
    // over-wide memcpy spills past the row into adjacent memory.
    w = texture->width;
    if (x < 0 || y < 0 || x >= (int)bi.width) return 0;
    if (x + w > (int)bi.width) w = (int)bi.width - x;
    if (w <= 0) return 0;

    // Exclude the UI thread's Canvas.drawBitmap for the duration of the write
    // so it never reads a half-written scanline (see bitmap_mutex).
    window_lock_bitmap();
    // A failed lock (e.g. the bitmap is already locked by another compositing
    // thread) leaves pixels undefined; writing to it would segfault.
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != 0 || pixels == NULL) {
      window_unlock_bitmap();
      return 0;
    }
    dst_stride_px = bi.stride / sizeof(pixel_t);
    p = (pixel_t *)pixels;
    p = &p[y * dst_stride_px + x];
    src = texture->buf;
    n = w * sizeof(pixel_t);
    for (i = 0; i < texture->height && (y + i) < (int)bi.height; i++) {
      xmemcpy(p, src, n);
      p += dst_stride_px;
      src += texture->width;
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    window_unlock_bitmap();
  }

  return 0;
}

static int window_destroy_texture(window_t *_window, texture_t *texture) {
  if (texture) {
    if (texture->buf) xfree(texture->buf);
    xfree(texture);
  }

  return 0;
}

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *string, void *data) {
  android_window_t *w;

  if ((w = xcalloc(1, sizeof(android_window_t))) != NULL) {
    w->width = *width;
    w->height = *height;
  }

  return (window_t *)w;
}

static int window_destroy(window_t *window) {
  if (window) {
    xfree(window);
  }
  return 0;
}

int window_erase(window_t *window, uint32_t bg) {
  return 0;
}

int window_render(window_t *_window) {
  //debug(DEBUG_INFO, "MAIN", "window_render");
  return 0;
}

void window_status(window_t *_window, int *x, int *y, int *buttons) {
  android_window_t *window;

  window = (android_window_t *)_window;
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}


static void window_add_event(touch_event_t *event) {
  if (numEvents < MAX_EVENTS) {
    numEvents++;
    xmemcpy(&events[idxIn++], event, sizeof(touch_event_t));
    if (idxIn == MAX_EVENTS)idxIn = 0;
  } else {
    debug(DEBUG_ERROR, "MAIN", "window_add_event event queue overflow");
  }
}

int window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  touch_event_t event;
  android_window_t *window;
  int r = 0;

  if (numEvents > 0) {
    xmemcpy(&event, &events[idxOut++], sizeof(touch_event_t));
    if (idxOut == MAX_EVENTS) idxOut = 0;
    numEvents--;

    switch (event.action) {
      case 0:
        *arg1 = 1;
        r = WINDOW_BUTTONDOWN;
        break;
      case 1:
        *arg1 = 1;
        r = WINDOW_BUTTONUP;
        break;
      case 2:
        window = (android_window_t *)_window;
        window->x = event.x;
        window->y = event.y;
        *arg1 = event.x;
        *arg2 = event.y;
        r = WINDOW_MOTION;
        break;
      case 3:
        *arg1 = event.key;
        r = WINDOW_KEYUP;
        break;
    }
  }

  return r;
}

static int window_draw_texture_rect(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  AndroidBitmapInfo bi;
  void *pixels;
  pixel_t *p, *src;
  JNIEnv *env = window_get_env();
  int i, n, dst_stride_px;

  if (env && bitmap && texture && tx >= 0 && ty >= 0 && tx < texture->width && ty < texture->height) {
    // A failed getInfo leaves bi uninitialized; its garbage stride/dimensions
    // would drive out-of-bounds writes below (see window_draw_texture).
    if (AndroidBitmap_getInfo(env, bitmap, &bi) != 0) return 0;

    // Clamp both the source rect (against the texture) and the destination
    // rect (against the bitmap), the way SDL_RenderCopy does. After an app
    // launch pumpkin_changed_display can recreate the texture at a smaller
    // size while wman still hands us the old (larger) region; without these
    // clamps the memcpy reads past the texture buffer into adjacent heap,
    // which is what shows up on screen as garbled "blue blocks".
    if (x < 0) { tx -= x; w += x; x = 0; }
    if (y < 0) { ty -= y; h += y; y = 0; }
    if (tx + w > texture->width)  w = texture->width  - tx;
    if (ty + h > texture->height) h = texture->height - ty;
    if (x + w > (int)bi.width)    w = (int)bi.width   - x;
    if (y + h > (int)bi.height)   h = (int)bi.height  - y;
    if (w <= 0 || h <= 0) return 0;

    // Exclude the UI thread's Canvas.drawBitmap while writing (see bitmap_mutex).
    window_lock_bitmap();
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != 0 || pixels == NULL) {
      window_unlock_bitmap();
      return 0;
    }
    dst_stride_px = bi.stride / sizeof(pixel_t);
    p = (pixel_t *)pixels;
    p = &p[y * dst_stride_px + x];
    src = &texture->buf[ty * texture->width + tx];
    n = w * sizeof(pixel_t);
    for (i = 0; i < h; i++) {
        xmemcpy(p, src, n);
        p += dst_stride_px;
        src += texture->width;
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    window_unlock_bitmap();
  }

  return 0;
}

static int window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *raw, int tx, int ty, int w, int h) {
  pixel_t *p, *src;
  int i, n;

  if (texture && raw && tx >= 0 && ty >= 0 && tx < texture->width && ty < texture->height) {
    // Clamp the rect to the texture so a too-wide/too-tall dirty region
    // updates as much as fits instead of being skipped entirely (which
    // would leave stale pixels on screen).
    if (tx + w > texture->width)  w = texture->width  - tx;
    if (ty + h > texture->height) h = texture->height - ty;
    if (w <= 0 || h <= 0) return 0;

    p = (pixel_t *)&texture->buf[ty * texture->width + tx];
    src = (pixel_t *)raw;
    src = &src[ty * texture->width + tx];
    n = w * sizeof(pixel_t);
    for (i = 0; i < h; i++) {
      xmemcpy(p, src, n);
      p += texture->width;
      src += texture->width;
    }
  }

  return 0;
}

void pitTouch(int action, int x, int y) {
  touch_event_t event;

  //debug(1, "XXX", "pitTouch %d %d %d", action, x, y);
  if (action == 0) {
    pitTouch(2, x, y);
  }
  event.action = action;
  event.x = x;
  event.y = y;
  window_add_event(&event);
}

void pitKey(int key) {
  touch_event_t event;

  event.action = 3;
  event.key = key;
  window_add_event(&event);
}

void window_init(int pe) {
  memset(&window_provider, 0, sizeof(window_provider_t));
  window_provider.create = window_create;
  //window_provider.draw = window_draw;
  //window_provider.draw2 = window_draw2;
  //window_provider.event = window_event;
  window_provider.destroy = window_destroy;
  window_provider.erase = window_erase;
  window_provider.render = window_render;
  //window_provider.background = window_background;
  window_provider.create_texture = window_create_texture;
  window_provider.destroy_texture = window_destroy_texture;
  window_provider.update_texture = window_update_texture;
  window_provider.draw_texture = window_draw_texture;
  window_provider.status = window_status;
  //window_provider.title = window_title;
  //window_provider.clipboard = window_clipboard;
  window_provider.event2 = window_event2;
  window_provider.draw_texture_rect = window_draw_texture_rect;
  window_provider.update_texture_rect = window_update_texture_rect;

  numEvents = 0;

  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);
}
