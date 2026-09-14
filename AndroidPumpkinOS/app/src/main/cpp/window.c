#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "sys.h"
#include "script.h"
#include "pwindow.h"
#include "debug.h"
#include "xalloc.h"
#include "pitapp.h"

// 32-bit framebuffer pixel. Memory byte order is R,G,B,X (PumpkinOS's "ABGR"
// host encoding, selected in libos_start_direct), which is exactly what
// WINDOW_FORMAT_RGBX_8888 expects, so the shadow framebuffer can be memcpy'd
// straight into the surface buffer with no conversion.
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

// Touch/key queue actions (see window_event2).
#define ACTION_DOWN   0
#define ACTION_UP     1
#define ACTION_MOVE   2
#define ACTION_KEY    3
#define ACTION_EXPOSE 4

static window_provider_t window_provider;

// ---------------------------------------------------------------------------
// Direct rendering to the SurfaceView.
//
// The PumpkinOS compositor (wman/dia/taskbar) calls draw_texture and
// draw_texture_rect from the PumpkinOS render thread(s); those calls paint into
// a native *shadow framebuffer* of the full logical screen. When PumpkinOS then
// calls render(), the shadow framebuffer is copied into the next buffer of the
// SurfaceView's ANativeWindow and posted to the compositor. The Java side never
// touches pixels any more: no Bitmap, no Canvas.drawBitmap, no 100 ms
// invalidate() timer. Presenting only from render() also means one post per
// PumpkinOS frame rather than one per dirty rectangle.
//
// A shadow framebuffer is required (rather than painting into the surface
// buffer directly) because the ANativeWindow is double/triple buffered: each
// ANativeWindow_lock hands back a buffer whose previous contents are undefined,
// so the whole frame has to be rewritten on every post.
//
// ANativeWindow_setBuffersGeometry fixes the buffer size at the logical
// PumpkinOS screen size (320x544); the system compositor scales it to the
// SurfaceView's on-screen size in hardware, which replaces the scaled
// drawBitmap the old code did on the UI thread.
//
// The surface comes and goes with the Activity lifecycle (surfaceCreated /
// surfaceDestroyed on the UI thread) while the PumpkinOS thread keeps
// rendering, so every access to `native_window` and `fb` is serialised by
// `mutex`. ANativeWindow calls need no JNIEnv, which also removes the need to
// attach PumpkinOS's app threads to the JVM (and to detach them on exit).
// ---------------------------------------------------------------------------

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static ANativeWindow *native_window;
static int geometry_set;   // setBuffersGeometry done for the current native_window
static pixel_t *fb;        // shadow framebuffer, fb_width * fb_height pixels
static int fb_width, fb_height;
static int fb_dirty;       // shadow framebuffer changed since last post

#define MAX_EVENTS 16
static touch_event_t events[MAX_EVENTS];
static int numEvents;
static int idxIn;
static int idxOut;

// Must be called with mutex held.
static void window_add_event_locked(touch_event_t *event) {
  if (numEvents < MAX_EVENTS) {
    numEvents++;
    xmemcpy(&events[idxIn++], event, sizeof(touch_event_t));
    if (idxIn == MAX_EVENTS) idxIn = 0;
  } else {
    debug(DEBUG_ERROR, "MAIN", "window_add_event event queue overflow");
  }
}

static void window_add_event(touch_event_t *event) {
  pthread_mutex_lock(&mutex);
  window_add_event_locked(event);
  pthread_mutex_unlock(&mutex);
}

// Copy the shadow framebuffer into the next surface buffer and post it.
// Must be called with mutex held. Returns 0 on success.
static int window_present_locked(void) {
  ANativeWindow_Buffer buffer;
  pixel_t *dst, *src;
  int i, w, h, n;

  if (native_window == NULL || fb == NULL) return -1;

  if (!geometry_set) {
    // Buffers are allocated at the logical screen size; the compositor scales
    // them to the view. RGBX: PumpkinOS does not maintain a meaningful alpha
    // channel, and an opaque format keeps the SurfaceView from blending with
    // whatever is behind it.
    if (ANativeWindow_setBuffersGeometry(native_window, fb_width, fb_height, WINDOW_FORMAT_RGBX_8888) != 0) {
      debug(DEBUG_ERROR, "MAIN", "ANativeWindow_setBuffersGeometry failed");
      return -1;
    }
    geometry_set = 1;
  }

  // Full-frame lock (no dirty rect): see the note about undefined buffer
  // contents above.
  if (ANativeWindow_lock(native_window, &buffer, NULL) != 0) {
    debug(DEBUG_ERROR, "MAIN", "ANativeWindow_lock failed");
    return -1;
  }

  // The buffer normally matches fb_width x fb_height (we asked for that
  // geometry), but clamp anyway so a mismatch can never write out of bounds.
  w = buffer.width  < fb_width  ? buffer.width  : fb_width;
  h = buffer.height < fb_height ? buffer.height : fb_height;
  n = w * sizeof(pixel_t);
  dst = (pixel_t *)buffer.bits;
  src = fb;
  for (i = 0; i < h; i++) {
    xmemcpy(dst, src, n);
    dst += buffer.stride;   // stride is in pixels
    src += fb_width;
  }

  ANativeWindow_unlockAndPost(native_window);
  fb_dirty = 0;

  return 0;
}

// Called from the UI thread (JNI) with the SurfaceView's Surface, or NULL when
// the surface is destroyed. `env` must be the calling thread's JNIEnv.
void window_set_surface(JNIEnv *env, jobject surface) {
  ANativeWindow *nw = NULL;
  touch_event_t event;

  if (env != NULL && surface != NULL) {
    // Takes a reference on the underlying window; released below when replaced
    // or on surfaceDestroyed.
    nw = ANativeWindow_fromSurface(env, surface);
    if (nw == NULL) {
      debug(DEBUG_ERROR, "MAIN", "ANativeWindow_fromSurface failed");
    }
  }

  pthread_mutex_lock(&mutex);
  if (native_window != NULL) {
    ANativeWindow_release(native_window);
  }
  native_window = nw;
  geometry_set = 0;

  if (nw != NULL) {
    debug(DEBUG_INFO, "MAIN", "surface created");
    // Show the last composited frame immediately so the view is not black
    // until PumpkinOS's next tick (e.g. when returning from the background),
    // then ask PumpkinOS for a full repaint. The Android host never receives
    // expose/damage events otherwise, so synthesise one here.
    window_present_locked();
    event.action = ACTION_EXPOSE;
    event.x = event.y = event.key = 0;
    window_add_event_locked(&event);
  } else {
    debug(DEBUG_INFO, "MAIN", "surface destroyed");
  }
  pthread_mutex_unlock(&mutex);
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

static int window_destroy_texture(window_t *_window, texture_t *texture) {
  if (texture) {
    if (texture->buf) xfree(texture->buf);
    xfree(texture);
  }

  return 0;
}

// Copy a texture rectangle into the shadow framebuffer, clipping both the
// source rect (against the texture) and the destination rect (against the
// framebuffer), the way SDL_RenderCopy does. After an app launch
// pumpkin_changed_display can recreate the texture at a smaller size while
// wman still hands us the old (larger) region; without these clamps the copy
// would read past the texture buffer into adjacent heap.
static int window_draw_texture_rect(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  pixel_t *p, *src;
  int i, n;

  if (texture == NULL || texture->buf == NULL) return 0;
  if (tx < 0 || ty < 0 || tx >= texture->width || ty >= texture->height) return 0;

  pthread_mutex_lock(&mutex);
  if (fb != NULL) {
    if (x < 0) { tx -= x; w += x; x = 0; }
    if (y < 0) { ty -= y; h += y; y = 0; }
    if (tx + w > texture->width)  w = texture->width  - tx;
    if (ty + h > texture->height) h = texture->height - ty;
    if (x + w > fb_width)  w = fb_width  - x;
    if (y + h > fb_height) h = fb_height - y;

    if (w > 0 && h > 0) {
      p = &fb[y * fb_width + x];
      src = &texture->buf[ty * texture->width + tx];
      n = w * sizeof(pixel_t);
      for (i = 0; i < h; i++) {
        xmemcpy(p, src, n);
        p += fb_width;
        src += texture->width;
      }
      fb_dirty = 1;
    }
  }
  pthread_mutex_unlock(&mutex);

  return 0;
}

int window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  if (texture == NULL) return 0;
  return window_draw_texture_rect(_window, texture, 0, 0, texture->width, texture->height, x, y);
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

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *string, void *data) {
  android_window_t *w;
  pixel_t *newfb;

  if ((w = xcalloc(1, sizeof(android_window_t))) != NULL) {
    w->width = *width;
    w->height = *height;

    if ((newfb = xcalloc((size_t)w->width * w->height, sizeof(pixel_t))) == NULL) {
      xfree(w);
      return NULL;
    }

    pthread_mutex_lock(&mutex);
    if (fb != NULL) xfree(fb);
    fb = newfb;
    fb_width = w->width;
    fb_height = w->height;
    fb_dirty = 1;
    // The surface may already exist (surfaceCreated usually runs before the
    // PumpkinOS thread gets here); its buffer geometry must follow the new size.
    geometry_set = 0;
    pthread_mutex_unlock(&mutex);

    debug(DEBUG_INFO, "MAIN", "window %dx%d created", w->width, w->height);
  }

  return (window_t *)w;
}

static int window_destroy(window_t *window) {
  if (window) {
    pthread_mutex_lock(&mutex);
    if (fb != NULL) xfree(fb);
    fb = NULL;
    fb_width = fb_height = 0;
    fb_dirty = 0;
    pthread_mutex_unlock(&mutex);
    xfree(window);
  }
  return 0;
}

int window_erase(window_t *window, uint32_t bg) {
  int i, n;

  pthread_mutex_lock(&mutex);
  if (fb != NULL) {
    n = fb_width * fb_height;
    for (i = 0; i < n; i++) fb[i] = bg;
    fb_dirty = 1;
  }
  pthread_mutex_unlock(&mutex);

  return 0;
}

// Called by PumpkinOS once per frame after its draw_texture* calls.
int window_render(window_t *_window) {
  pthread_mutex_lock(&mutex);
  if (fb_dirty) {
    window_present_locked();
  }
  pthread_mutex_unlock(&mutex);

  return 0;
}

void window_status(window_t *_window, int *x, int *y, int *buttons) {
  android_window_t *window;

  window = (android_window_t *)_window;
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

int window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  touch_event_t event;
  android_window_t *window;
  int r = 0, have = 0;

  pthread_mutex_lock(&mutex);
  if (numEvents > 0) {
    xmemcpy(&event, &events[idxOut++], sizeof(touch_event_t));
    if (idxOut == MAX_EVENTS) idxOut = 0;
    numEvents--;
    have = 1;
  }
  pthread_mutex_unlock(&mutex);

  if (have) {
    switch (event.action) {
      case ACTION_DOWN:
        *arg1 = 1;
        r = WINDOW_BUTTONDOWN;
        break;
      case ACTION_UP:
        *arg1 = 1;
        r = WINDOW_BUTTONUP;
        break;
      case ACTION_MOVE:
        window = (android_window_t *)_window;
        window->x = event.x;
        window->y = event.y;
        *arg1 = event.x;
        *arg2 = event.y;
        r = WINDOW_MOTION;
        break;
      case ACTION_KEY:
        *arg1 = event.key;
        r = WINDOW_KEYUP;
        break;
      case ACTION_EXPOSE:
        r = WINDOW_EXPOSE;
        break;
    }
  }

  return r;
}

void pitTouch(int action, int x, int y) {
  touch_event_t event;

  //debug(1, "XXX", "pitTouch %d %d %d", action, x, y);
  if (action == ACTION_DOWN) {
    pitTouch(ACTION_MOVE, x, y);
  }
  event.action = action;
  event.x = x;
  event.y = y;
  event.key = 0;
  window_add_event(&event);
}

void pitKey(int key) {
  touch_event_t event;

  event.action = ACTION_KEY;
  event.x = event.y = 0;
  event.key = key;
  window_add_event(&event);
}

void window_init(int pe) {
  memset(&window_provider, 0, sizeof(window_provider_t));
  window_provider.create = window_create;
  window_provider.destroy = window_destroy;
  window_provider.erase = window_erase;
  window_provider.render = window_render;
  window_provider.create_texture = window_create_texture;
  window_provider.destroy_texture = window_destroy_texture;
  window_provider.update_texture = window_update_texture;
  window_provider.draw_texture = window_draw_texture;
  window_provider.status = window_status;
  window_provider.event2 = window_event2;
  window_provider.draw_texture_rect = window_draw_texture_rect;
  window_provider.update_texture_rect = window_update_texture_rect;

  pthread_mutex_lock(&mutex);
  numEvents = 0;
  idxIn = idxOut = 0;
  pthread_mutex_unlock(&mutex);

  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);
}
