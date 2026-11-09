#include <SDL3/SDL.h>

#ifdef SDL_OPENGL
#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#endif

#ifdef SDL_MIXER
#include <SDL3_mixer/SDL_mixer.h>
#endif

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "audio.h"
#include "ptr.h"
#include "average.h"
#include "debug.h"

#ifdef WII
#include <wiiuse/wpad.h>
#endif

// the audio provider runs in its own thread, which the ESP32 build does not have
#if defined(ESP32)
#define LIBSDL_NOAUDIO
#endif

// MS-DOS: PumpkinOS has only cooperative threads there (threadsc), so the
// audio provider cannot run its own (preemptive) thread. Instead the application's audio
// callback is pulled from the window provider event functions (which every
// PalmOS event loop calls continuously) and pushed into an SDL audio stream;
// SDL's own cooperative audio thread moves it to the Sound Blaster whenever
// the main thread yields (SDL_PumpEvents / SDL_Delay).
//
// The PlayStation Vita, Nintendo Switch and PSP builds use the same
// cooperative threads (and the same pump): there SDL has a real audio thread
// draining the stream.
#if defined(GAMECUBE) || defined(WII)
// Nintendo GameCube (and Wii): every SDL event pump scans the controller ports
// (PAD_ScanPads), and a scan made before the serial interface has new data
// (it polls the pads once per video frame) finds no controller; after four
// such scans in a row SDL's driver reports the pad removed, then added again
// on the next good scan, over and over (every event poll is a pump, and the
// event loops below poll every millisecond). So the queue is pumped at most
// once per frame; in between the events already queued are just read.
#define LIBSDL_PUMP_INTERVAL_US 16000

#ifdef WII
static void libsdl_wii_buttons(void);
#endif

static void libsdl_pump_events(void) {
  static uint64_t last = 0;
  uint64_t now = sys_get_clock();

  if (last == 0 || now - last >= LIBSDL_PUMP_INTERVAL_US) {
    last = now;
    SDL_PumpEvents();
#ifdef WII
    libsdl_wii_buttons();
#endif
  }
}

static bool libsdl_poll_event(SDL_Event *ev) {
  libsdl_pump_events();
  if (ev == NULL) return SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) > 0;
  return SDL_PeepEvents(ev, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) > 0;
}

#define SDL_PumpEvents() libsdl_pump_events()
#define SDL_PollEvent(ev) libsdl_poll_event(ev)
#endif

#if defined(DOS) || defined(VITA) || defined(SWITCH) || defined(PSP) || defined(PS2) || defined(GAMECUBE) || defined(WII)
#define LIBSDL_AUDIO_PUMP
#define LIBSDL_PUMP_LEAD_MS 100 // audio kept queued in the SDL stream
#define LIBSDL_PUMP_WAIT_MS 20  // longest event wait while a stream plays
static int libsdl_audio_pump(void);

// MS-DOS: PumpkinOS threads are cooperative coroutines (libpit threadsc),
// so the event wait must not block inside SDL for the whole timeout: the
// network tasks (HTTP client, mail, net install) would never run. Poll the
// SDL queue and yield to the other tasks in between.
static int libsdl_wait_event(SDL_Event *ev, int wait) {
  uint64_t t0, t;

  t0 = sys_get_clock();
  for (;;) {
    if (SDL_PollEvent(ev)) return 1;
    if (wait >= 0) {
      t = sys_get_clock();
      if ((t - t0) / 1000 >= (uint64_t)wait) return 0;
    }
    thread_yield(0);
    sys_usleep(1000);
  }
}
#define SDL_WaitEvent(ev) libsdl_wait_event(ev, -1)
#define SDL_WaitEventTimeout(ev, ms) libsdl_wait_event(ev, ms)
#endif

#if defined(VITA) || defined(SWITCH) || defined(PSP) || defined(PS2) || defined(GAMECUBE) || defined(WII)
// PlayStation Vita / Nintendo Switch: the touch screen is the pen (SDL
// emulates the mouse from it) and the gamepad buttons stand in for a few
// keyboard keys: D-pad = arrows, L/R = Page Up/Down, Start (+) = Home (back
// to the Launcher), Select (-) = Tab. The confirm/cancel pair follows the
// platform convention: Cross/Circle on the Vita (south = Enter, east =
// Escape), A/B on the Switch (east = Enter, south = Escape).
//
// PlayStation Portable / PlayStation 2: no touch screen, so the analog stick
// and the D-pad move a pointer drawn by this provider (LIBSDL_STICK_POINTER)
// and Cross is the pen (mouse button 1); Triangle = Enter, Circle = Escape,
// Square = Backspace, the rest as above (the D-pad is not an arrow key here).
//
// Nintendo GameCube: the same pointer, moved by the control stick and the
// D-pad, A is the pen (mouse button 1), B = Escape, X = Enter, Y =
// Backspace, Z = Tab, Start = Home, L/R = Page Up/Down. (SDL maps the
// GameCube pad with A south, B west, X east, Y north, Z back.)
//
// Nintendo Wii: GameCube controllers work as on the GameCube. The Wiimote
// pointed at the screen moves the same pointer (SDL turns its IR position
// into mouse motion), and its buttons are read here (libsdl_wii_buttons)
// and fed in as the GameCube buttons they stand for: A = the pen, B =
// Escape, + = Enter, - = Backspace, Home = Home, 1/2 = Page Up/Down; the
// D-pad sends arrow keys. SDL also reports A and B as mouse buttons and the
// Wiimote as a gamepad of its own (with a layout that depends on the
// extension plugged in): both are ignored.
#define LIBSDL_GAMEPAD

#if defined(SWITCH)
#define GAMEPAD_BUTTON_ENTER  SDL_GAMEPAD_BUTTON_EAST
#define GAMEPAD_BUTTON_ESCAPE SDL_GAMEPAD_BUTTON_SOUTH
#elif defined(PSP) || defined(PS2) || defined(GAMECUBE) || defined(WII)
#define LIBSDL_STICK_POINTER
#if defined(PS2)
// the PS2 gsKit renderer has no render targets (SDL_SetRenderTarget is a
// no-op there) and swaps between two undefined back buffers: the scene is
// composed in memory and uploaded as one texture per present instead
#define LIBSDL_SOFT_SCENE
#endif
#define GAMEPAD_BUTTON_PEN    SDL_GAMEPAD_BUTTON_SOUTH
#if defined(GAMECUBE) || defined(WII)
#define GAMEPAD_BUTTON_ENTER  SDL_GAMEPAD_BUTTON_EAST   // X
#define GAMEPAD_BUTTON_ESCAPE SDL_GAMEPAD_BUTTON_WEST   // B
#define GAMEPAD_BUTTON_BACKSP SDL_GAMEPAD_BUTTON_NORTH  // Y
#else
#define GAMEPAD_BUTTON_ENTER  SDL_GAMEPAD_BUTTON_NORTH
#define GAMEPAD_BUTTON_ESCAPE SDL_GAMEPAD_BUTTON_EAST
#define GAMEPAD_BUTTON_BACKSP SDL_GAMEPAD_BUTTON_WEST
#endif
#else
#define GAMEPAD_BUTTON_ENTER  SDL_GAMEPAD_BUTTON_SOUTH
#define GAMEPAD_BUTTON_ESCAPE SDL_GAMEPAD_BUTTON_EAST
#endif

static SDL_Gamepad *gamepad = NULL;

static int map_gamepad_button(int button) {
  switch (button) {
#ifndef LIBSDL_STICK_POINTER
    // with the emulated pointer the D-pad moves it instead (pointer_axis)
    case SDL_GAMEPAD_BUTTON_DPAD_UP:        return WINDOW_KEY_UP;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return WINDOW_KEY_DOWN;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return WINDOW_KEY_LEFT;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return WINDOW_KEY_RIGHT;
#endif
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return WINDOW_KEY_PGUP;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return WINDOW_KEY_PGDOWN;
    case SDL_GAMEPAD_BUTTON_START:          return WINDOW_KEY_HOME;
    case GAMEPAD_BUTTON_ENTER:              return '\n';
    case GAMEPAD_BUTTON_ESCAPE:             return 27;
#ifdef GAMEPAD_BUTTON_BACKSP
    case GAMEPAD_BUTTON_BACKSP:             return 8;
#endif
    case SDL_GAMEPAD_BUTTON_BACK:           return '\t';
  }
  return 0;
}

// one gamepad at a time: the first one plugged in drives PumpkinOS, the
// others are ignored until it is unplugged
static void open_gamepad(SDL_JoystickID id) {
  SDL_Gamepad *g;

  if (gamepad != NULL) return;
#ifdef WII
  {
    // the Wiimotes are read directly (libsdl_wii_buttons): only GameCube
    // controllers are opened as gamepads
    const char *name = SDL_GetGamepadNameForID(id);
    if (name && !sys_strncmp(name, "Wiimote", 7)) {
      debug(DEBUG_INFO, "SDL", "gamepad %d \"%s\" ignored (read as a Wiimote)", (int)id, name);
      return;
    }
  }
#endif
  if ((g = SDL_OpenGamepad(id)) != NULL) {
    gamepad = g;
    debug(DEBUG_INFO, "SDL", "gamepad %d \"%s\" opened", (int)id, SDL_GetGamepadName(gamepad));
  } else {
    debug(DEBUG_ERROR, "SDL", "SDL_OpenGamepad(%d) failed: %s", (int)id, SDL_GetError());
  }
}

static void close_gamepad(SDL_JoystickID id) {
  if (gamepad != NULL && SDL_GetGamepadID(gamepad) == id) {
    debug(DEBUG_INFO, "SDL", "gamepad %d removed", (int)id);
    SDL_CloseGamepad(gamepad);
    gamepad = NULL;
  }
}

#ifdef WII
// the Wiimote buttons and what they become: a GameCube gamepad button
// (handled exactly like one), or an arrow key for the D-pad
static const struct {
  uint32_t wpad;
  int gbutton;   // SDL_GAMEPAD_BUTTON_* or -1
  SDL_Keycode key;
  SDL_Scancode scancode;
} wii_buttons[] = {
  { WPAD_BUTTON_A,     GAMEPAD_BUTTON_PEN,                  0, 0 },
  { WPAD_BUTTON_B,     GAMEPAD_BUTTON_ESCAPE,               0, 0 },
  { WPAD_BUTTON_PLUS,  GAMEPAD_BUTTON_ENTER,                0, 0 },
  { WPAD_BUTTON_MINUS, GAMEPAD_BUTTON_BACKSP,               0, 0 },
  { WPAD_BUTTON_HOME,  SDL_GAMEPAD_BUTTON_START,            0, 0 },
  { WPAD_BUTTON_1,     SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,    0, 0 },
  { WPAD_BUTTON_2,     SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,   0, 0 },
  { WPAD_BUTTON_UP,    -1, SDLK_UP,    SDL_SCANCODE_UP },
  { WPAD_BUTTON_DOWN,  -1, SDLK_DOWN,  SDL_SCANCODE_DOWN },
  { WPAD_BUTTON_LEFT,  -1, SDLK_LEFT,  SDL_SCANCODE_LEFT },
  { WPAD_BUTTON_RIGHT, -1, SDLK_RIGHT, SDL_SCANCODE_RIGHT },
  { 0, -1, 0, 0 }
};

// The buttons held on all the Wiimotes, compared with the previous call:
// every change becomes an SDL event. SDL's joystick driver reads the
// Wiimotes (WPAD_ReadPending) in each pump; only the held state is used
// here, so it does not matter who reads them or how often.
// Home held this long quits PumpkinOS (back to the loader)
#define WII_HOME_QUIT_US 2000000

static void libsdl_wii_buttons(void) {
  static uint32_t prev = 0;
  static uint64_t home_t0 = 0;
  uint32_t held = 0, changed;
  WPADData *data;
  SDL_Event ev;
  uint64_t now;
  int i;

  for (i = 0; i < WPAD_MAX_WIIMOTES; i++) {
    data = WPAD_Data(i);
    if (data && data->err == WPAD_ERR_NONE && data->data_present) held |= data->btns_h;
  }

  if (held & WPAD_BUTTON_HOME) {
    now = sys_get_clock();
    if (home_t0 == 0) {
      home_t0 = now;
    } else if (home_t0 != (uint64_t)-1 && now - home_t0 >= WII_HOME_QUIT_US) {
      debug(DEBUG_INFO, "SDL", "Home held: quitting");
      home_t0 = (uint64_t)-1;   // once
      SDL_zero(ev);
      ev.type = SDL_EVENT_QUIT;
      SDL_PushEvent(&ev);
    }
  } else {
    home_t0 = 0;
  }

  if ((changed = held ^ prev) == 0) return;
  prev = held;
  debug(DEBUG_INFO, "SDL", "Wiimote buttons 0x%08X", held);

  for (i = 0; wii_buttons[i].wpad; i++) {
    if (!(changed & wii_buttons[i].wpad)) continue;
    SDL_zero(ev);
    if (wii_buttons[i].gbutton >= 0) {
      ev.type = (held & wii_buttons[i].wpad) ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
      ev.gbutton.which = 0;
      ev.gbutton.button = wii_buttons[i].gbutton;
      ev.gbutton.down = (held & wii_buttons[i].wpad) != 0;
    } else {
      ev.type = (held & wii_buttons[i].wpad) ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
      ev.key.key = wii_buttons[i].key;
      ev.key.scancode = wii_buttons[i].scancode;
      ev.key.down = (held & wii_buttons[i].wpad) != 0;
    }
    ev.common.timestamp = SDL_GetTicksNS();
    SDL_PushEvent(&ev);
  }
}
#endif
#endif

#define SDL_BUTTON1   1
#define SDL_BUTTON2   2

#define MAX_DRIVER 128

#define MAX_SHADER 16
#define MAX_VARS   16

#define TAG_AUDIO  "audio"

#ifdef SDL_MIXER
MIX_Mixer *mixer = NULL;
#endif

struct texture_t {
  SDL_Texture *t;
  int width, height;
  SDL_Texture *blur;
  uint32_t *buf;
};

typedef struct {
  uint8_t from;
  uint8_t mods;
  uint8_t to;
} libsdl_keymap_t;

typedef struct {
  int width, height, xfactor, yfactor, rotate, fullscreen, software, spixel;
  int x, y, buttons, mods, other;
  uint32_t format;
  char driver[MAX_DRIVER];
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *background;
  SDL_Texture *target;  // persistent scene for double-buffered renderers
  libsdl_keymap_t keymap[0x1000];
  int64_t shift_up;
  int opengl;
#ifdef SDL_OPENGL
  GLuint programId[MAX_SHADER];
  texture_t *texture;
  float (*getvar)(char *name, void *data);
  void *data;
#endif
  void (*drop_callback)(char *filename, void *data);
  void *drop_data;
#ifdef LIBSDL_STICK_POINTER
  double px, py;      // pointer position (window pixels)
  uint64_t ptime;     // last stick / D-pad sample (us)
  int pvisible;       // pointer drawn (always, once the window exists)
#endif
#ifdef LIBSDL_SOFT_SCENE
  SDL_Texture *scene_tex; // the scene: one streaming texture in a format native to the renderer, drawn whole at present
  uint32_t scene_fmt;     // its SDL pixel format (ABGR1555 for a 16-bit window, ABGR8888 for a 32-bit one)
  int scene_w, scene_h;
  uint8_t *bgraw;         // background pixels (window_background, window pixel format), drawn by window_erase
  int bg_w, bg_h;
#endif
} libsdl_window_t;

typedef struct {
  char *tag;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  int format, rate, channels;
  uint8_t *buffer;
  uint32_t bsize;
} sdl_audio_t;

typedef struct {
  int pcm, channels, rate;
} sdl_audio_arg_t;

static libsdl_keymap_t keymap[] = {
  { '\'', WINDOW_MOD_SHIFT, '"' },
  { '1', WINDOW_MOD_SHIFT, '!' },
  { '2', WINDOW_MOD_SHIFT, '@' },
  { '3', WINDOW_MOD_SHIFT, '#' },
  { '4', WINDOW_MOD_SHIFT, '$' },
  { '5', WINDOW_MOD_SHIFT, '%' },
  { '7', WINDOW_MOD_SHIFT, '&' },
  { '8', WINDOW_MOD_SHIFT, '*' },
  { '9', WINDOW_MOD_SHIFT, '(' },
  { '0', WINDOW_MOD_SHIFT, ')' },
  { '-', WINDOW_MOD_SHIFT, '_' },
  { '=', WINDOW_MOD_SHIFT, '+' },
  { '\\', WINDOW_MOD_SHIFT, '|' },
  { '[', WINDOW_MOD_SHIFT, '{' },
  { ']', WINDOW_MOD_SHIFT, '}' },
  { '~', WINDOW_MOD_SHIFT, '^' },
  { '/', WINDOW_MOD_SHIFT, '?' },
  { ';', WINDOW_MOD_SHIFT, ':' },
  { '.', WINDOW_MOD_SHIFT, '>' },
  { ',', WINDOW_MOD_SHIFT, '<' },

  // teclas com AltGr do acer
  { 'q', WINDOW_MOD_RALT, '/' },
  { 'w', WINDOW_MOD_RALT, '?' },

  { 'a', WINDOW_MOD_SHIFT, 'A' },
  { 'b', WINDOW_MOD_SHIFT, 'B' },
  { 'c', WINDOW_MOD_SHIFT, 'C' },
  { 'd', WINDOW_MOD_SHIFT, 'D' },
  { 'e', WINDOW_MOD_SHIFT, 'E' },
  { 'f', WINDOW_MOD_SHIFT, 'F' },
  { 'g', WINDOW_MOD_SHIFT, 'G' },
  { 'h', WINDOW_MOD_SHIFT, 'H' },
  { 'i', WINDOW_MOD_SHIFT, 'I' },
  { 'j', WINDOW_MOD_SHIFT, 'J' },
  { 'k', WINDOW_MOD_SHIFT, 'K' },
  { 'l', WINDOW_MOD_SHIFT, 'L' },
  { 'm', WINDOW_MOD_SHIFT, 'M' },
  { 'n', WINDOW_MOD_SHIFT, 'N' },
  { 'o', WINDOW_MOD_SHIFT, 'O' },
  { 'p', WINDOW_MOD_SHIFT, 'P' },
  { 'q', WINDOW_MOD_SHIFT, 'Q' },
  { 'r', WINDOW_MOD_SHIFT, 'R' },
  { 's', WINDOW_MOD_SHIFT, 'S' },
  { 't', WINDOW_MOD_SHIFT, 'T' },
  { 'u', WINDOW_MOD_SHIFT, 'U' },
  { 'v', WINDOW_MOD_SHIFT, 'V' },
  { 'w', WINDOW_MOD_SHIFT, 'W' },
  { 'x', WINDOW_MOD_SHIFT, 'X' },
  { 'y', WINDOW_MOD_SHIFT, 'Y' },
  { 'z', WINDOW_MOD_SHIFT, 'Z' },

  { 'a', WINDOW_MOD_CTRL, 1 },
  { 'b', WINDOW_MOD_CTRL, 2 },
  { 'c', WINDOW_MOD_CTRL, 3 },
  { 'd', WINDOW_MOD_CTRL, 4 },
  { 'e', WINDOW_MOD_CTRL, 5 },
  { 'f', WINDOW_MOD_CTRL, 6 },
  { 'g', WINDOW_MOD_CTRL, 7 },
  { 'h', WINDOW_MOD_CTRL, 8 },
  { 'i', WINDOW_MOD_CTRL, 9 },
  { 'j', WINDOW_MOD_CTRL, 10 },
  { 'k', WINDOW_MOD_CTRL, 11 },
  { 'l', WINDOW_MOD_CTRL, 12 },
  { 'm', WINDOW_MOD_CTRL, 13 },
  { 'n', WINDOW_MOD_CTRL, 14 },
  { 'o', WINDOW_MOD_CTRL, 15 },
  { 'p', WINDOW_MOD_CTRL, 16 },
  { 'q', WINDOW_MOD_CTRL, 17 },
  { 'r', WINDOW_MOD_CTRL, 18 },
  { 's', WINDOW_MOD_CTRL, 19 },
  { 't', WINDOW_MOD_CTRL, 20 },
  { 'u', WINDOW_MOD_CTRL, 21 },
  { 'v', WINDOW_MOD_CTRL, 22 },
  { 'w', WINDOW_MOD_CTRL, 23 },
  { 'x', WINDOW_MOD_CTRL, 24 },
  { 'y', WINDOW_MOD_CTRL, 25 },
  { 'z', WINDOW_MOD_CTRL, 26 },

  { 0, 0, 0 }
};

static window_provider_t window_provider;

#ifndef LIBSDL_NOAUDIO
static audio_provider_t audio_provider;

static int libsdl_init_audio(void) {
  const char *name;
  int i, n, r = -1;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
    debug(DEBUG_ERROR, "SDL", "init audio failed: %s", SDL_GetError());
  } else {
    if (SDL_Init(SDL_INIT_AUDIO) == true) {
      SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&n);
      if (n == -1) {
        debug(DEBUG_ERROR, "SDL", "SDL_GetAudioPlaybackDevices failed: %s", SDL_GetError());
      } else {
        debug(DEBUG_INFO, "SDL", "%d audio device(s) found", n);
        for (i = 0; i < n; i++) {
          name = SDL_GetAudioDeviceName(devices[i]);
          debug(DEBUG_INFO, "SDL", "audio device %d \"%s\"", i, name);
        }
        r = 0;
      }

    }
  }

  return r;
}
#endif

static int libsdl_present(libsdl_window_t *window);
#ifdef LIBSDL_SOFT_SCENE
static void libsdl_scene_clear(libsdl_window_t *window);
#endif

#ifdef LIBSDL_STICK_POINTER
// Gamepad pointer (PlayStation Portable): the left stick moves a pointer
// across the window at up to POINTER_SPEED pixels per second, a held D-pad
// direction moves it at full speed, and the pen button (Cross) presses
// mouse button 1 at its position. The pointer is drawn by libsdl_present()
// over the persistent render target, so it only exists with an accelerated
// renderer (the PSP one). It starts at the screen center and is always
// visible.
#define POINTER_DEADZONE 6000   // of 32767
#define POINTER_SPEED    360.0  // pixels per second at full deflection
#define POINTER_WAIT_MS  16     // longest event wait while the stick is deflected
#define POINTER_WIDTH    12
#define POINTER_HEIGHT   19

static SDL_Texture *pointer_texture = NULL;

// the classic arrow: 'X' outline, '.' fill, ' ' transparent
static const char *pointer_shape[POINTER_HEIGHT] = {
  "X           ",
  "XX          ",
  "X.X         ",
  "X..X        ",
  "X...X       ",
  "X....X      ",
  "X.....X     ",
  "X......X    ",
  "X.......X   ",
  "X........X  ",
  "X.........X ",
  "X......XXXXX",
  "X...X..X    ",
  "X..X X..X   ",
  "X.X  X..X   ",
  "XX    X..X  ",
  "X     X..X  ",
  "       XX   ",
  "            ",
};

// deflection of one pointer axis, in -(32767 - deadzone) .. +(32767 - deadzone):
// the analog stick past its dead zone, or else the D-pad at full deflection
static int pointer_axis(SDL_GamepadAxis axis) {
  int v, neg, pos;

  if (gamepad == NULL) return 0;
  v = SDL_GetGamepadAxis(gamepad, axis);
  if (v <= -POINTER_DEADZONE) return v + POINTER_DEADZONE;
  if (v >= POINTER_DEADZONE) return v - POINTER_DEADZONE;

  if (axis == SDL_GAMEPAD_AXIS_LEFTX) {
    neg = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    pos = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
  } else {
    neg = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    pos = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  }
  if (neg && !pos) return -(32767 - POINTER_DEADZONE);
  if (pos && !neg) return 32767 - POINTER_DEADZONE;

  return 0;
}

static int libsdl_stick_active(void) {
  return pointer_axis(SDL_GAMEPAD_AXIS_LEFTX) != 0 || pointer_axis(SDL_GAMEPAD_AXIS_LEFTY) != 0;
}

// moves the pointer with the stick / D-pad; returns 1 when its position
// changed (window->x / window->y hold the new position, and the frame was
// redrawn)
static int libsdl_pointer_poll(libsdl_window_t *window) {
  uint64_t now;
  double dt, scale;
  int ax, ay, x, y;

  now = sys_get_clock();
  if (window->ptime == 0) {
    window->ptime = now;
    return 0;
  }
  dt = (double)(now - window->ptime) / 1000000.0;
  window->ptime = now;
  if (dt > 0.1) dt = 0.1;

  ax = pointer_axis(SDL_GAMEPAD_AXIS_LEFTX);
  ay = pointer_axis(SDL_GAMEPAD_AXIS_LEFTY);
  if (ax == 0 && ay == 0) return 0;

  scale = POINTER_SPEED * dt / (32767.0 - POINTER_DEADZONE);
  window->px += ax * scale;
  window->py += ay * scale;
  if (window->px < 0) window->px = 0;
  if (window->py < 0) window->py = 0;
  if (window->px > window->width - 1) window->px = window->width - 1;
  if (window->py > window->height - 1) window->py = window->height - 1;
  window->pvisible = 1;

  x = (int)window->px;
  y = (int)window->py;
  if (x == window->x && y == window->y) return 0;

  window->x = x;
  window->y = y;
  libsdl_present(window);

  return 1;
}

static void libsdl_pointer_draw(libsdl_window_t *window) {
  uint32_t pixels[POINTER_WIDTH * POINTER_HEIGHT];
  SDL_FRect dst;
  int i, j;

  if (!window->pvisible) return;

  if (pointer_texture == NULL) {
    if ((pointer_texture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, POINTER_WIDTH, POINTER_HEIGHT)) == NULL) {
      debug(DEBUG_ERROR, "SDL", "SDL_CreateTexture (pointer) failed: %s", SDL_GetError());
      return;
    }
    for (i = 0; i < POINTER_HEIGHT; i++) {
      for (j = 0; j < POINTER_WIDTH; j++) {
        switch (pointer_shape[i][j]) {
          case 'X': pixels[i * POINTER_WIDTH + j] = 0xFF000000; break;
          case '.': pixels[i * POINTER_WIDTH + j] = 0xFFFFFFFF; break;
          default:  pixels[i * POINTER_WIDTH + j] = 0x00000000; break;
        }
      }
    }
    SDL_UpdateTexture(pointer_texture, NULL, pixels, POINTER_WIDTH * sizeof(uint32_t));
    SDL_SetTextureBlendMode(pointer_texture, SDL_BLENDMODE_BLEND);
  }

  dst.x = window->x;
  dst.y = window->y;
  dst.w = POINTER_WIDTH;
  dst.h = POINTER_HEIGHT;
  SDL_RenderTexture(window->renderer, pointer_texture, NULL, &dst);
}

#ifdef WII
// the Wiimote moved the pointer (window->x / window->y): the stick pointer
// continues from there, and the frame is redrawn with it
static void libsdl_wii_pointer(libsdl_window_t *window) {
  if (window->x < 0) window->x = 0;
  if (window->y < 0) window->y = 0;
  if (window->x > window->width - 1) window->x = window->width - 1;
  if (window->y > window->height - 1) window->y = window->height - 1;
  if (window->x == (int)window->px && window->y == (int)window->py && window->pvisible) return;
  window->px = window->x;
  window->py = window->y;
  window->pvisible = 1;
  libsdl_present(window);
}
#endif
#endif

static int libsdl_init_video(void) {
  SDL_Cursor *cursor;
  int flags, i, n, r = -1;
  const char *s;
 
  flags = SDL_INIT_VIDEO;
#ifdef SDL_OPENGL
  flags |= SDL_VIDEO_OPENGL;
#endif
#ifdef LIBSDL_GAMEPAD
  flags |= SDL_INIT_GAMEPAD;
#endif

#ifdef VITA
  // the front touch screen drives the mouse (pen), the rear touch pad is
  // ignored (it would move the pen with the fingers holding the device)
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
  SDL_SetHint(SDL_HINT_VITA_ENABLE_FRONT_TOUCH, "1");
  SDL_SetHint(SDL_HINT_VITA_ENABLE_BACK_TOUCH, "0");
#endif
#ifdef SWITCH
  // the touch screen drives the mouse (pen)
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
#endif
#if defined(PSP) || defined(PS2) || defined(GAMECUBE) || defined(WII)
  // the built-in controller is a gamepad (SDL has a mapping for it); the
  // pointer it drives is drawn by libsdl_present() (no mouse, no cursor)
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#endif

  if (SDL_InitSubSystem(flags) == false) {
    debug(DEBUG_ERROR, "SDL", "init video failed: %s", SDL_GetError());

  } else {
    if (SDL_Init(flags) == true) {
      if ((n = SDL_GetNumRenderDrivers()) == -1) {
        debug(DEBUG_ERROR, "SDL", "SDL_GetNumRenderDrivers failed: %s", SDL_GetError());
      } else {
        debug(DEBUG_INFO, "SDL", "%d driver(s) found", n);
        for (i = 0; i < n; i++) {
          if ((s = SDL_GetRenderDriver(i)) != NULL) {
            debug(DEBUG_INFO, "SDL", "driver %s found", s);
            if ((cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT)) != NULL) {
              SDL_SetCursor(cursor);
            }
#ifdef WII
            // the pointer is drawn by libsdl_present(), as on the GameCube
            SDL_HideCursor();
#endif
            //SDL_ShowCursor(SDL_DISABLE);
            r = 0;
          }
        }
      }
    }
  }

  return r;
}

static void libsdl_status(libsdl_window_t *window, int *x, int *y, int *buttons) {
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static void libsdl_title(libsdl_window_t *window, char *title) {
  SDL_SetWindowTitle(window->window, title);
}

static char *libsdl_clipboard(libsdl_window_t *window, char *clipboard, int len) {
  char *s;

  if (clipboard != NULL && len > 0) {
    if ((s = sys_calloc(1, len + 1)) != NULL) {
      sys_memcpy(s, clipboard, len);
      SDL_SetClipboardText(s);  // SDL_SetClipboardText puts UTF-8 text into the clipboard
      sys_free(s);
    }
  }

  return SDL_GetClipboardText();
}

/* The character a key produces in the *active* keyboard layout with the current
   modifiers (Shift, Caps Lock, AltGr), as computed by SDL.  The fixed keymap
   table below only knows the US layout, so on e.g. an AZERTY keyboard Shift
   would not give digits or symbols.  Returns 0 when SDL has no Latin-1
   character for the key (control keys, dead keys, shortcuts with Ctrl/Alt),
   in which case the caller falls back to the table. */
static int layout_key(libsdl_window_t *window, SDL_Event *ev) {
#if SDL_VERSION_ATLEAST(3, 1, 3)
  SDL_Keymod modstate;
  SDL_Keycode kc;

  if (window->mods & (WINDOW_MOD_CTRL | WINDOW_MOD_RCTRL | WINDOW_MOD_LALT)) return 0;
  modstate = ev->key.mod & (SDL_KMOD_CAPS | SDL_KMOD_RALT | SDL_KMOD_MODE | SDL_KMOD_LEVEL5);
  if (window->mods & WINDOW_MOD_SHIFT) modstate |= SDL_KMOD_SHIFT;
  kc = SDL_GetKeyFromScancode(ev->key.scancode, modstate, false);
  if ((kc >= 32 && kc < 127) || (kc >= 160 && kc < 256)) return (int)kc;
#endif
  return 0;
}

static int map_key(libsdl_window_t *window, SDL_Event *ev) {
  int index, shift = 0, key = 0;

  switch (ev->key.key) {
    case SDLK_LCTRL:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        window->mods |= WINDOW_MOD_CTRL;
      } else {
        window->mods &= ~WINDOW_MOD_CTRL;
      }
      key = WINDOW_KEY_CTRL;
      break;
    case SDLK_RCTRL:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        window->mods |= WINDOW_MOD_RCTRL;
      } else {
        window->mods &= ~WINDOW_MOD_RCTRL;
      }
      key = WINDOW_KEY_RCTRL;
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        window->mods |= WINDOW_MOD_SHIFT;
        window->other = 0;
      } else {
        window->mods &= ~WINDOW_MOD_SHIFT;
      }
      key = WINDOW_KEY_SHIFT;
      break;
    case SDLK_LALT:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        window->mods |= WINDOW_MOD_LALT;
      } else {
        window->mods &= ~WINDOW_MOD_LALT;
      }
      key = WINDOW_KEY_LALT;
      break;
    case SDLK_RALT:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        window->mods |= WINDOW_MOD_RALT;
      } else {
        window->mods &= ~WINDOW_MOD_RALT;
      }
      key = WINDOW_KEY_RALT;
      break;

    default:
      if (ev->type == SDL_EVENT_KEY_DOWN) {
        if (window->mods & WINDOW_MOD_SHIFT) window->other = 1;
      } else {
        if (window->other && !(window->mods & WINDOW_MOD_SHIFT)) {
          window->mods |= WINDOW_MOD_SHIFT;
          shift = 1;
        }
        window->other = 0;
      }

      if (ev->key.key < 256) {
        if ((key = layout_key(window, ev)) == 0) {
          index = (window->mods << 8) | ev->key.key;
          if (window->keymap[index].to && (window->keymap[index].mods & window->mods)) {
            key = window->keymap[index].to;
          } else {
            key = ev->key.key;
          }
        }
      } else {
        switch (ev->key.key) {
          case SDLK_RIGHT: key = WINDOW_KEY_RIGHT; break;
          case SDLK_LEFT: key = WINDOW_KEY_LEFT; break;
          case SDLK_DOWN: key = WINDOW_KEY_DOWN; break;
          case SDLK_UP: key = WINDOW_KEY_UP; break;
          case SDLK_PAGEUP: key = WINDOW_KEY_PGUP; break;
          case SDLK_PAGEDOWN: key = WINDOW_KEY_PGDOWN; break;
          case SDLK_HOME: key = WINDOW_KEY_HOME; break;
          case SDLK_END: key = WINDOW_KEY_END; break;
          case SDLK_INSERT: key = WINDOW_KEY_INS; break;
          case SDLK_F1: key = WINDOW_KEY_F1; break;
          case SDLK_F2: key = WINDOW_KEY_F2; break;
          case SDLK_F3: key = WINDOW_KEY_F3; break;
          case SDLK_F4: key = WINDOW_KEY_F4; break;
          case SDLK_F5: key = WINDOW_KEY_F5; break;
          case SDLK_F6: key = WINDOW_KEY_F6; break;
          case SDLK_F7: key = WINDOW_KEY_F7; break;
          case SDLK_F8: key = WINDOW_KEY_F8; break;
          case SDLK_F9: key = WINDOW_KEY_F9; break;
          case SDLK_F10: key = WINDOW_KEY_F10; break;
          case SDLK_F11: key = WINDOW_KEY_F11; break;
          case SDLK_F12: key = WINDOW_KEY_F12; break;
          case 0x40000000:
            switch (ev->key.raw) {
              case 0x34:
                key = (window->mods & WINDOW_MOD_SHIFT) ? '^' : '~';
                break;
              case 0x87:
                key = (window->mods & WINDOW_MOD_SHIFT) ? '?' : '/';
                break;
              default:
                debug(DEBUG_ERROR, "SDL", "unknown scancode 0x%02X", ev->key.raw);
                key = 0;
                break;
            }
            break;
          default:
            debug(DEBUG_ERROR, "SDL", "unmapped sym 0x%08X scancode 0x%08X", ev->key.key, ev->key.raw);
            break;
        }
      }

      if (shift) {
        window->mods &= ~WINDOW_MOD_SHIFT;
      }
  }

  return key;
}

static int map_button(libsdl_window_t *window, SDL_Event *ev) {
#ifdef WII
  // the Wiimote A / B as mouse buttons: A is read directly (the pen), B is
  // Escape (libsdl_wii_buttons)
  return 0;
#endif
  switch (ev->button.button) {
    case 1: return SDL_BUTTON1;
    case 3: return SDL_BUTTON2;
  }

  return 0;
}

static int libsdl_event2(libsdl_window_t *window, int wait, int *arg1, int *arg2) {
  SDL_Event ev;
  int has_ev, r = 0;

#ifdef LIBSDL_AUDIO_PUMP
  if (libsdl_audio_pump() && (wait < 0 || wait > LIBSDL_PUMP_WAIT_MS)) wait = LIBSDL_PUMP_WAIT_MS;
#endif
#ifdef LIBSDL_STICK_POINTER
  // the stick / D-pad moved the pointer: report the motion right away, and
  // while it is held keep the waits short so that the pointer keeps moving
  SDL_PumpEvents();
  if (libsdl_pointer_poll(window)) {
    *arg1 = window->x;
    *arg2 = window->y;
    return WINDOW_MOTION;
  }
  if (libsdl_stick_active() && (wait < 0 || wait > POINTER_WAIT_MS)) wait = POINTER_WAIT_MS;
#endif
  has_ev = wait < 0 ? SDL_WaitEvent(&ev) : SDL_WaitEventTimeout(&ev, wait);

  if (has_ev) {
    switch (ev.type) {
      case SDL_EVENT_KEY_DOWN:
        *arg1 = map_key(window, &ev);
        if (*arg1) r = WINDOW_KEYDOWN;
        break;
      case SDL_EVENT_KEY_UP:
        *arg1 = map_key(window, &ev);
        if (*arg1) r = WINDOW_KEYUP;
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        *arg1 = map_button(window, &ev);
        if (*arg1) {
          window->buttons |= *arg1;
          r = WINDOW_BUTTONDOWN;
        }
        debug(DEBUG_INFO, "SDL", "Mouse button \"%d\" down", window->buttons);
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        *arg1 = map_button(window, &ev);
        if (*arg1) {
          window->buttons &= ~(*arg1);
          r = WINDOW_BUTTONUP;
        }
        debug(DEBUG_INFO, "SDL", "Mouse button \"%d\" up", window->buttons);
        break;
      case SDL_EVENT_MOUSE_MOTION:
        window->x = ev.motion.x / window->xfactor;
        window->y = ev.motion.y / window->yfactor;
#ifdef WII
        libsdl_wii_pointer(window);
#endif
        *arg1 = window->x;
        *arg2 = window->y;
        debug(DEBUG_INFO, "SDL", "Mouse move to \"%dx%d\"", window->x, window->y);
        r = WINDOW_MOTION;
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        *arg1 = ev.wheel.x;
        *arg2 = ev.wheel.y;
        r = WINDOW_WHEEL;
        break;
#ifdef LIBSDL_GAMEPAD
      case SDL_EVENT_GAMEPAD_ADDED:
        open_gamepad(ev.gdevice.which);
        break;
      case SDL_EVENT_GAMEPAD_REMOVED:
        close_gamepad(ev.gdevice.which);
        break;
      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
#ifdef LIBSDL_STICK_POINTER
        if (ev.gbutton.button == GAMEPAD_BUTTON_PEN) {
          // the pen touches the screen at the pointer position
          *arg1 = SDL_BUTTON1;
          window->buttons |= SDL_BUTTON1;
          r = WINDOW_BUTTONDOWN;
          break;
        }
#endif
        *arg1 = map_gamepad_button(ev.gbutton.button);
        if (*arg1) r = WINDOW_KEYDOWN;
        break;
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
#ifdef LIBSDL_STICK_POINTER
        if (ev.gbutton.button == GAMEPAD_BUTTON_PEN) {
          *arg1 = SDL_BUTTON1;
          window->buttons &= ~SDL_BUTTON1;
          r = WINDOW_BUTTONUP;
          break;
        }
#endif
        *arg1 = map_gamepad_button(ev.gbutton.button);
        if (*arg1) r = WINDOW_KEYUP;
        break;
#endif
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        r = -1;
        break;
      case SDL_EVENT_DROP_FILE:
        if (window->drop_callback && ev.drop.data) {
          debug(DEBUG_INFO, "SDL", "file \"%s\" dropped", ev.drop.data);
          window->drop_callback((char *)ev.drop.data, window->drop_data);
        }
        //SDL_free(ev.drop.data);
        break;
      case SDL_EVENT_QUIT:
        r = -1;
        break;
    }
  }

  return r;
}

static int libsdl_event(libsdl_window_t *window, int wait, int remove, int *ekey, int *mods, int *ebuttons) {
  uint16_t index;
  int buttons, key;
  SDL_Event ev;
  int has_ev, shift, r = 0;

  buttons = window->buttons;

  for (;;) {
#ifdef LIBSDL_AUDIO_PUMP
    // feed the audio stream, and while a stream is playing return early
    // (with no event) instead of blocking, so that it is fed again soon
    if (libsdl_audio_pump() && (wait < 0 || wait > LIBSDL_PUMP_WAIT_MS)) wait = LIBSDL_PUMP_WAIT_MS;
#endif
#ifdef LIBSDL_STICK_POINTER
    SDL_PumpEvents();
    if (libsdl_pointer_poll(window)) {
      *ekey = window->x;
      *mods = window->y;
      *ebuttons = buttons;
      return WINDOW_MOTION;
    }
    if (libsdl_stick_active() && (wait < 0 || wait > POINTER_WAIT_MS)) wait = POINTER_WAIT_MS;
#endif
    if (wait) {
      // wait: timeout in ms, or -1 to wait forever
      has_ev = wait < 0 ? SDL_WaitEvent(&ev) : SDL_WaitEventTimeout(&ev, wait);
    } else {
      if (remove) {
        has_ev = SDL_PollEvent(&ev);
      } else {
        SDL_PumpEvents();
        has_ev = SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);
      }
    }

    if (has_ev) {
      switch (ev.type) {
      case SDL_EVENT_KEY_DOWN:
        switch (ev.key.key) {
          case SDLK_LCTRL:
            window->mods |= WINDOW_MOD_CTRL;
            debug(DEBUG_TRACE, "SDL", "left ctrl down");
            break;
          case SDLK_RCTRL:
            window->mods |= WINDOW_MOD_RCTRL;
            debug(DEBUG_TRACE, "SDL", "right ctrl down");
            break;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            window->mods |= WINDOW_MOD_SHIFT;
            debug(DEBUG_TRACE, "SDL", "shift down");
            break;
          case SDLK_LALT:
            window->mods |= WINDOW_MOD_LALT;
            debug(DEBUG_TRACE, "SDL", "lalt down");
            break;
          case SDLK_RALT:
            window->mods |= WINDOW_MOD_RALT;
            debug(DEBUG_TRACE, "SDL", "ralt down");
            break;
        }
        if (remove == 0) SDL_PollEvent(&ev);
        continue;

      case SDL_EVENT_KEY_UP:
        switch (ev.key.key) {
          case SDLK_LCTRL:
          case SDLK_RCTRL:
            window->mods &= ~WINDOW_MOD_CTRL;
            debug(DEBUG_TRACE, "SDL", "ctrl up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            window->shift_up = sys_get_clock();
            window->mods &= ~WINDOW_MOD_SHIFT;
            debug(DEBUG_TRACE, "SDL", "shift up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_LALT:
            window->mods &= ~WINDOW_MOD_LALT;
            debug(DEBUG_TRACE, "SDL", "lalt up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_RALT:
            window->mods &= ~WINDOW_MOD_RALT;
            debug(DEBUG_TRACE, "SDL", "ralt up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          default:
            shift = 0;
            if (!(window->mods & WINDOW_MOD_SHIFT)) {
              if ((sys_get_clock() - window->shift_up) < 100000) {
                window->mods |= WINDOW_MOD_SHIFT;
                shift = 1;
              }
            }
            debug(DEBUG_TRACE, "SDL", "keyup sym 0x%08X (0x%02X)", ev.key.key, window->mods);
            if (ev.key.key < 256) {
              if ((key = layout_key(window, &ev)) != 0) {
                debug(DEBUG_TRACE, "SDL", "layout key '%c' -> '%c'", ev.key.key, key);
              } else {
                index = (window->mods << 8) | ev.key.key;
                if (window->keymap[index].to && (window->keymap[index].mods & window->mods)) {
                  key = window->keymap[index].to;
                  debug(DEBUG_TRACE, "SDL", "change key '%c' -> '%c'", ev.key.key, key);
                } else {
                  key = ev.key.key;
                  debug(DEBUG_TRACE, "SDL", "key '%c'", key);
                }
              }
            } else {
              switch (ev.key.key) {
                case SDLK_RIGHT: key = WINDOW_KEY_RIGHT; break;
                case SDLK_LEFT: key = WINDOW_KEY_LEFT; break;
                case SDLK_DOWN: key = WINDOW_KEY_DOWN; break;
                case SDLK_UP: key = WINDOW_KEY_UP; break;
                case SDLK_PAGEUP: key = WINDOW_KEY_PGUP; break;
                case SDLK_PAGEDOWN: key = WINDOW_KEY_PGDOWN; break;
                case SDLK_HOME: key = WINDOW_KEY_HOME; break;
                case SDLK_END: key = WINDOW_KEY_END; break;
                case SDLK_F1: key = WINDOW_KEY_F1; break;
                case SDLK_F2: key = WINDOW_KEY_F2; break;
                case SDLK_F3: key = WINDOW_KEY_F3; break;
                case SDLK_F4: key = WINDOW_KEY_F4; break;
                case SDLK_F5: key = WINDOW_KEY_F5; break;
                case SDLK_F6: key = WINDOW_KEY_F6; break;
                case SDLK_F7: key = WINDOW_KEY_F7; break;
                case SDLK_F8: key = WINDOW_KEY_F8; break;
                case SDLK_F9: key = WINDOW_KEY_F9; break;
                case SDLK_F10: key = WINDOW_KEY_F10; break;
                case 0x40000000:
                  switch (ev.key.raw) {
                    case 0x34:
                      key = (window->mods & WINDOW_MOD_SHIFT) ? '^' : '~';
                      debug(DEBUG_TRACE, "SDL", "scancode 0x%02X -> '%c'", ev.key.raw, key);
                      break;
                    default:
                      debug(DEBUG_TRACE, "SDL", "unmapped scancode 0x%02X", ev.key.raw);
                      key = 0;
                      break;
                  }
                  break;
                default:
                  debug(DEBUG_TRACE, "SDL", "unmapped sym 0x%08X", ev.key.key);
                  key = 0;
                  break;
              }
            }
            if (key) {
              *ekey = key;
              *mods = window->mods;
              r = WINDOW_KEY;
            }
            if (shift) {
              window->mods &= ~WINDOW_MOD_SHIFT;
            }
        }
        break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        switch (map_button(window, &ev) ? ev.button.button : 0) {
          case 1:
            buttons |= SDL_BUTTON1;
            break;
          case 3:
            buttons |= SDL_BUTTON2;
            break;
        }
        debug(DEBUG_INFO, "SDL", "Mouse button \"%d\" down", ev.button.button);
        break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
        switch (map_button(window, &ev) ? ev.button.button : 0) {
          case 1:
            buttons &= ~SDL_BUTTON1;
            break;
          case 3:
            buttons &= ~SDL_BUTTON2;
            break;
        }
        debug(DEBUG_INFO, "SDL", "Mouse button \"%d\" up", ev.button.button);
        break;

      case SDL_EVENT_MOUSE_MOTION:
        window->x = ev.motion.x;
        window->y = ev.motion.y;
#ifdef WII
        libsdl_wii_pointer(window);
#endif
        *ekey = window->x;
        *mods = window->y;
        *ebuttons = buttons;
        r = WINDOW_MOTION;
        debug(DEBUG_INFO, "SDL", "Mouse move to \"%dx%d\"", window->x, window->y);
        break;

#ifdef LIBSDL_GAMEPAD
      case SDL_EVENT_GAMEPAD_ADDED:
        open_gamepad(ev.gdevice.which);
        if (remove == 0) SDL_PollEvent(&ev);
        continue;
      case SDL_EVENT_GAMEPAD_REMOVED:
        close_gamepad(ev.gdevice.which);
        if (remove == 0) SDL_PollEvent(&ev);
        continue;

#ifdef LIBSDL_STICK_POINTER
      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        if (ev.gbutton.button == GAMEPAD_BUTTON_PEN) {
          buttons |= SDL_BUTTON1;
        }
        break;
#endif

      case SDL_EVENT_GAMEPAD_BUTTON_UP:
#ifdef LIBSDL_STICK_POINTER
        if (ev.gbutton.button == GAMEPAD_BUTTON_PEN) {
          buttons &= ~SDL_BUTTON1;
          break;
        }
#endif
        if ((key = map_gamepad_button(ev.gbutton.button)) != 0) {
          *ekey = key;
          *mods = window->mods;
          r = WINDOW_KEY;
        }
        break;
#endif

      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        r = -1;
        break;

      case SDL_EVENT_QUIT:
        r = -1;
        break;

      default:
        if (wait == 0 && remove == 0) {
          SDL_PollEvent(&ev);
          continue;
        }
      }

      if (buttons != window->buttons) {
        window->buttons = buttons;
        *ebuttons = buttons;
        r = WINDOW_BUTTON;
      }
    }

    if (wait < 0) {
      if (r) break;
    } else {
      break;
    }
  }

  return r;
}

static int libsdl_window_show_cursor(window_t *_window, int show) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  SDL_SetWindowRelativeMouseMode(window->window, show ? false : true);
  return 0;
}

/*
  SDL_CreateWindow():
  a) Fullscreen window at the current desktop resolution: flags=SDL_WINDOW_FULLSCREEN_DESKTOP, ignores width and height parameters.
     Optional: to map a logical resolution into the real resolution:
     SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
     SDL_RenderSetLogicalSize(window->renderer, width, height);
  b) Fullscreen window at specified resolution: flags=SDL_WINDOW_FULLSCREEN, uses width and height parameters.
  c) Windowed mode at specified resolution: flags=0, uses width and height parameters.
*/

static int libsdl_video_setup(libsdl_window_t *window) {
  uint16_t index;
  uint32_t n, i, w, h;
  const char *s;

#if defined(ESP32)
  w = CONFIG_BSP_DISPLAY_WIDTH;
  h = CONFIG_BSP_DISPLAY_HEIGHT;

  window->fullscreen = 1;
  window->xfactor = 1;
  window->yfactor = 1;
#else
  w = window->width;
  h = window->height;

  if (!window->fullscreen) {
    w *= window->xfactor;
    h *= window->yfactor;
  }
#endif

  i = -1;
  s = NULL;

  if (window->driver[0]) {
    if ((n = SDL_GetNumRenderDrivers()) != -1) {
      for (i = 0; i < n; i++) {
        if ((s = SDL_GetRenderDriver(i)) != NULL) {
          if (!sys_strcmp(s, window->driver)) {
            debug(DEBUG_INFO, "SDL", "requested driver \"%s\" found", window->driver);
            break;
          }
          s = NULL;
        }
      }
      if (s == NULL) {
        debug(DEBUG_ERROR, "SDL", "requested driver \"%s\" not found, using default", window->driver);
      }
    }
  }

  debug(DEBUG_INFO, "SDL", "creating window (%d x %d) fullscreen %d", w, h, window->fullscreen);
#ifdef PS2
  sys_ps2_memlog("window creation");
#endif
  window->window = SDL_CreateWindow("", w, h, /*window->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP :*/ 0);
  if (window->window == NULL) {
    debug(DEBUG_ERROR, "SDL", "SDL_CreateWindow failed: %s", SDL_GetError());
    return -1;
  }

  //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

  debug(DEBUG_INFO, "SDL", "creating renderer");
  // index of the rendering driver to initialize, or -1 to initialize the first one supporting the requested flags.
#if defined(DARWIN) || defined(SW_RENDERER) || defined(ESP32)
  // default to the software renderer, but honor an explicit driver= request
  // from the boot script (e.g. driver = "metal" for 60fps game-like apps)
  window->renderer = SDL_CreateRenderer(window->window, s != NULL ? s : "software" /*, window->software ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE*/);
#else
  window->renderer = SDL_CreateRenderer(window->window, s /*, window->software ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE*/);
#endif

  if (window->renderer == NULL) {
    SDL_DestroyWindow(window->window);
    window->window = NULL;
    return -1;
  }

  if ((s = SDL_GetRendererName(window->renderer)) != NULL) {
    debug(DEBUG_INFO, "SDL", "using driver \"%s\"", s);
  }
#ifdef PS2
  sys_ps2_memlog("renderer created");
#endif

  if (window->fullscreen) {
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetRenderLogicalPresentation(window->renderer, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
  } else {
    debug(DEBUG_INFO, "SDL", "set window border");
    SDL_SetWindowBordered(window->window, true);
    if (window->xfactor > 1 || window->yfactor > 1) {
      //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
      SDL_SetRenderLogicalPresentation(window->renderer, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    }
  }

  SDL_SetRenderDrawBlendMode(window->renderer, SDL_BLENDMODE_BLEND);

  SDL_RenderClear(window->renderer);
  SDL_RenderPresent(window->renderer);

  // PumpkinOS draws incrementally (dirty rects), which assumes the drawing
  // surface persists across presents. Accelerated renderers (metal, opengl)
  // swap between undefined backbuffers on every SDL_RenderPresent, so
  // incremental drawing flickers. Route all drawing to a persistent target
  // texture; libsdl_window_render() blits it whole to the backbuffer.
#ifdef LIBSDL_SOFT_SCENE
  // the PS2 renderer has no render targets: the scene is one streaming
  // texture in a format the renderer supports natively (so that
  // SDL_LockTexture() gives the texture memory itself, no staging copy),
  // written by libsdl_scene_blit() and drawn whole by libsdl_scene_present()
  window->scene_w = w;
  window->scene_h = h;
  window->scene_fmt = window->spixel == 2 ? SDL_PIXELFORMAT_ABGR1555 : SDL_PIXELFORMAT_ABGR8888;
  if (window->spixel != 2 && window->spixel != 4) {
    debug(DEBUG_ERROR, "SDL", "scene: unsupported window format 0x%08X", window->format);
  } else if ((window->scene_tex = SDL_CreateTexture(window->renderer, window->scene_fmt, SDL_TEXTUREACCESS_STREAMING, w, h)) == NULL) {
    debug(DEBUG_ERROR, "SDL", "SDL_CreateTexture (scene) failed: %s", SDL_GetError());
  } else {
    SDL_SetTextureBlendMode(window->scene_tex, SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(window->scene_tex, SDL_SCALEMODE_NEAREST);
    libsdl_scene_clear(window);
    debug(DEBUG_INFO, "SDL", "using a %dx%d scene texture (format %s) for \"%s\"", w, h, SDL_GetPixelFormatName(window->scene_fmt), s ? s : "?");
  }
  if (0) {
#else
  if (s != NULL && sys_strcmp(s, "software")) {
#endif
    window->target = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_TARGET, w, h);
    if (window->target != NULL) {
      SDL_SetRenderTarget(window->renderer, window->target);
      SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
      SDL_RenderClear(window->renderer);
      debug(DEBUG_INFO, "SDL", "using persistent render target for \"%s\"", s);
    } else {
      debug(DEBUG_ERROR, "SDL", "SDL_CreateTexture (target) failed: %s", SDL_GetError());
    }
  }

  for (i = 0; keymap[i].from; i++) {
    index = (keymap[i].mods << 8) | keymap[i].from;
    window->keymap[index] = keymap[i];
  }

  return 0;
}

static int libsdl_video_close(libsdl_window_t *window) {
#ifdef LIBSDL_STICK_POINTER
  if (pointer_texture) {
    SDL_DestroyTexture(pointer_texture);
    pointer_texture = NULL;
  }
#endif
  if (window->target) {
    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_DestroyTexture(window->target);
  }
#ifdef LIBSDL_SOFT_SCENE
  if (window->scene_tex) SDL_DestroyTexture(window->scene_tex);
  if (window->bgraw) sys_free(window->bgraw);
#endif
  if (window->background) SDL_DestroyTexture(window->background);
  if (window->renderer) SDL_DestroyRenderer(window->renderer);
  if (window->window) SDL_DestroyWindow(window->window);
  sys_free(window);

  return 0;
}

static window_t *libsdl_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libsdl_window_t *window;
  SDL_Rect rect;
  uint32_t format, spixel;

  switch (encoding) {
/*
    case ENC_I420:
      format = SDL_PIXELFORMAT_IYUV;
      spixel = sizeof(uint16_t); // XXX ?
      break;
*/
    case ENC_UYVY:
      format = SDL_PIXELFORMAT_UYVY;
      spixel = sizeof(uint16_t);
      break;
    case ENC_YUYV:
      format = SDL_PIXELFORMAT_YUY2;
      spixel = sizeof(uint16_t);
      break;
    case ENC_RGB565:
      format = SDL_PIXELFORMAT_RGB565;
      spixel = sizeof(uint16_t);
      break;
    case ENC_RGB:
      format = SDL_PIXELFORMAT_BGR24;
      spixel = 3*sizeof(uint8_t);
      break;
    case ENC_RGBA:
      format = SDL_PIXELFORMAT_ARGB8888;
      spixel = sizeof(uint32_t);
      break;
    default:
      debug(DEBUG_ERROR, "SDL", "invalid encoding %s", video_encoding_name(encoding));
      return NULL;
  }

  if (*width == 0 || *height == 0) {
    if (fullscreen) {
      SDL_GetDisplayBounds(0, &rect);
    } else {
      SDL_GetDisplayUsableBounds(0, &rect);
    }
    *width = rect.w;
    *height = rect.h;
    debug(DEBUG_INFO, "SDL", "using whole display %dx%d", *width, *height);
  }

  if ((window = sys_calloc(1, sizeof(libsdl_window_t))) != NULL) {
    window->width = *width;
    window->height = *height;
    window->xfactor = xfactor;
    window->yfactor = yfactor;
    window->fullscreen = fullscreen;
    window->software = software;
    window->format = format;
    window->spixel = spixel;
    window->rotate = rotate;
#ifdef LIBSDL_STICK_POINTER
    window->px = window->width / 2;
    window->py = window->height / 2;
    window->x = (int)window->px;
    window->y = (int)window->py;
    window->pvisible = 1;   // shown from the start, at the screen center
#endif

#if defined(ESP32) || defined(ANDROID)
    sys_strncpy(window->driver, "software", MAX_DRIVER);
#else
    sys_strncpy(window->driver, driver, MAX_DRIVER);
#endif

    if (libsdl_video_setup(window) == 0) {
      debug(DEBUG_INFO, "SDL", "window created");

    } else {
      sys_free(window);
      window = NULL;
    }
  }

  return (window_t *)window;
}

#ifdef SDL_OPENGL
static void presentBackBuffer(libsdl_window_t *window, texture_t *t, GLuint programId) {
  GLint oldProgramId, numUniforms, index, size, loc;
  GLenum type;
  GLsizei length;
  GLfloat minx, miny, maxx, maxy;
  GLfloat minu, maxu, minv, maxv;
  GLfloat value;
  char var[256];
  PFNGLUSEPROGRAMPROC glUseProgram;

  glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");

  SDL_SetRenderTarget(window->renderer, NULL);
  SDL_RenderClear(window->renderer);

  if (programId != 0) {
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    glUseProgram(programId);
  }

  if (window->getvar) {
    glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (index = 0; index < numUniforms; index++) {
      glGetActiveUniform(programId, index, sizeof(var), &length, &size, &type, var);
      if (type == GL_FLOAT && sys_strcmp(var, "u_texture") != 0) {
        value = window->getvar(var, window->data);
        debug(DEBUG_TRACE, "SDL", "shader var \"%s\" = %f", var, value);
        glUniform1f(index, value);
      }
    }
  }

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  SDL_GL_BindTexture(t->t, NULL, NULL);
  loc = glGetUniformLocation(programId, "u_texture");
  glUniform1i(loc, 0);

  minx = -1.0f;
  maxx = 1.0f;
  miny = 1.0f;
  maxy = -1.0f;

  minu = 0.0f;
  maxu = 1.0f;
  minv = 0.0f;
  maxv = 1.0f;

  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(minu, minv);
  glVertex2f(minx, miny);
  glTexCoord2f(maxu, minv);
  glVertex2f(maxx, miny);
  glTexCoord2f(minu, maxv);
  glVertex2f(minx, maxy);
  glTexCoord2f(maxu, maxv);
  glVertex2f(maxx, maxy);
  glEnd();

  SDL_GL_SwapWindow(window->window);

  if (programId != 0) {
    glUseProgram(oldProgramId);
  }
}
#endif

#ifdef LIBSDL_SOFT_SCENE
// Scene texture (PlayStation 2): the textures PumpkinOS draws are kept in
// memory (texture->buf, window pixel format) and blitted into one streaming
// texture of a format the renderer takes as is (RGB565 -> ABGR1555, ARGB8888
// -> ABGR8888), through SDL_LockTexture(), which on the PS2 renderer is the
// texture memory itself; the texture is drawn whole at every present (the
// renderer flips between two back buffers). Only a 1:1 window (xfactor =
// yfactor = 1) is supported; pixels are copied opaque (no alpha blending
// between textures).
static void libsdl_scene_convert(libsdl_window_t *window, uint8_t *dst, uint8_t *src, int w) {
  uint16_t *s16, *d16, p;
  uint32_t *s32, *d32, q;
  int i;

  if (window->spixel == 2) {
    s16 = (uint16_t *)src;
    d16 = (uint16_t *)dst;
    for (i = 0; i < w; i++) {
      p = s16[i];  // RGB565
      d16[i] = 0x8000 | ((p & 0x1F) << 10) | (((p >> 6) & 0x1F) << 5) | (p >> 11);
    }
  } else {
    s32 = (uint32_t *)src;
    d32 = (uint32_t *)dst;
    for (i = 0; i < w; i++) {
      q = s32[i];  // ARGB8888
      d32[i] = (q & 0xFF00FF00) | ((q & 0xFF) << 16) | ((q >> 16) & 0xFF);
    }
  }
}

// copies the w x h rectangle at (sx,sy) of a sw x sh pixels source buffer
// (window pixel format) to (x,y) of the scene, clipped to both
static int libsdl_scene_blit(libsdl_window_t *window, uint8_t *src, int sw, int sh, int sx, int sy, int w, int h, int x, int y) {
  SDL_Rect rect;
  void *pixels;
  uint8_t *s, *d;
  int pitch, i;

  if (window->scene_tex == NULL || src == NULL) return -1;

  if (sx < 0) { w += sx; x -= sx; sx = 0; }
  if (sy < 0) { h += sy; y -= sy; sy = 0; }
  if (sx + w > sw) w = sw - sx;
  if (sy + h > sh) h = sh - sy;
  if (x < 0) { w += x; sx -= x; x = 0; }
  if (y < 0) { h += y; sy -= y; y = 0; }
  if (x + w > window->scene_w) w = window->scene_w - x;
  if (y + h > window->scene_h) h = window->scene_h - y;
  if (w <= 0 || h <= 0) return 0;

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  if (!SDL_LockTexture(window->scene_tex, &rect, &pixels, &pitch)) {
    debug(DEBUG_ERROR, "SDL", "SDL_LockTexture (scene) failed: %s", SDL_GetError());
    return -1;
  }
  s = &src[(sy * sw + sx) * window->spixel];
  d = (uint8_t *)pixels;
  for (i = 0; i < h; i++) {
    libsdl_scene_convert(window, d, s, w);
    s += sw * window->spixel;
    d += pitch;
  }
  SDL_UnlockTexture(window->scene_tex);

  return 0;
}

// black
static void libsdl_scene_clear(libsdl_window_t *window) {
  void *pixels;
  int pitch;

  if (window->scene_tex && SDL_LockTexture(window->scene_tex, NULL, &pixels, &pitch)) {
    sys_memset(pixels, 0, pitch * window->scene_h);
    SDL_UnlockTexture(window->scene_tex);
  }
}

static int libsdl_scene_present(libsdl_window_t *window) {
  SDL_RenderClear(window->renderer);
  SDL_RenderTexture(window->renderer, window->scene_tex, NULL, NULL);
#ifdef LIBSDL_STICK_POINTER
  libsdl_pointer_draw(window);
#endif
  SDL_RenderPresent(window->renderer);

  return 0;
}
#endif

// shows the scene: the persistent render target (plus the stick pointer,
// when there is one) is copied to the back buffer and presented
static int libsdl_present(libsdl_window_t *window) {
#ifdef LIBSDL_SOFT_SCENE
  if (window->scene_tex != NULL) {
    return libsdl_scene_present(window);
  }
#endif
  if (window->target != NULL) {
    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_RenderClear(window->renderer);
    SDL_RenderTexture(window->renderer, window->target, NULL, NULL);
#ifdef LIBSDL_STICK_POINTER
    libsdl_pointer_draw(window);
#endif
    SDL_RenderPresent(window->renderer);
    SDL_SetRenderTarget(window->renderer, window->target);
  } else {
    SDL_RenderPresent(window->renderer);
  }

  return 0;
}

static int libsdl_window_render(window_t *_window) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  int r = -1;

  if (window) {
#ifdef SDL_OPENGL
    if (window->opengl && window->texture) {
      presentBackBuffer(window, window->texture, window->programId[0]);
    } else {
#endif
      libsdl_present(window);
#ifdef SDL_OPENGL
    }
#endif
    r = 0;
  }

  return r;
}

static int libsdl_window_erase(window_t *_window, uint32_t bg) {
  libsdl_window_t *window;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window) {
#ifdef LIBSDL_SOFT_SCENE
    if (window->scene_tex) {
      libsdl_scene_clear(window);
      if (window->bgraw) {
        libsdl_scene_blit(window, window->bgraw, window->bg_w, window->bg_h, 0, 0, window->bg_w, window->bg_h, 0, 0);
      }
      return 0;
    }
#endif
    SDL_RenderClear(window->renderer);

    if (window->background) {
      SDL_RenderTexture(window->renderer, window->background, NULL, NULL);
    }

    r = 0;
  }

  return r;
}

static texture_t *libsdl_window_create_texture(window_t *_window, int width, int height) {
  libsdl_window_t *window;
  texture_t *texture = NULL;

  window = (libsdl_window_t *)_window;

  if (window) {
    if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
#ifdef LIBSDL_SOFT_SCENE
      // no SDL texture: the pixels stay in memory and are blitted into the scene
      if ((texture->buf = sys_calloc(1, width * height * window->spixel)) != NULL) {
        texture->width = width;
        texture->height = height;
      } else {
        debug(DEBUG_ERROR, "SDL", "texture allocation (%dx%d) failed", width, height);
        sys_free(texture);
        texture = NULL;
      }
#else
      if ((texture->t = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height)) != NULL) {
        SDL_SetTextureBlendMode(texture->t, SDL_BLENDMODE_BLEND);
        texture->width = width;
        texture->height = height;
        texture->blur = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture->blur, SDL_BLENDMODE_BLEND);
        texture->buf = sys_calloc(1, width * height * 4);
      } else {
        debug(DEBUG_ERROR, "SDL", "SDL_CreateTexture format 0x%08X failed: %s", window->format, SDL_GetError());
        sys_free(texture);
        texture = NULL;
      }
#endif
    }
  }

  return texture;
}

static int libsdl_window_destroy_texture(window_t *_window, texture_t *texture) {
  libsdl_window_t *window;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture) {
    if (texture->t) SDL_DestroyTexture(texture->t);
    if (texture->blur) SDL_DestroyTexture(texture->blur);
    if (texture->buf) sys_free(texture->buf);
    sys_free(texture);
  }

  return r;
}

static int libsdl_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  libsdl_window_t *window;
  SDL_Rect rect;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && raw) {
#ifdef LIBSDL_SOFT_SCENE
    if (window->bgraw) sys_free(window->bgraw);
    if ((window->bgraw = sys_calloc(1, width * height * window->spixel)) != NULL) {
      sys_memcpy(window->bgraw, raw, width * height * window->spixel);
      window->bg_w = width;
      window->bg_h = height;
      r = 0;
    }
    if (0) {
#else
    if ((window->background = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height)) != NULL) {
#endif
      rect.x = 0;
      rect.y = 0;
      rect.w = width;
      rect.h = height;
      SDL_UpdateTexture(window->background, &rect, (uint8_t *)raw, width * window->spixel);
      r = 0;
    }
  }

  return r;
}

static int libsdl_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  libsdl_window_t *window;
  SDL_Rect rect;
  void *pixels;
  uint8_t *dst;
  int pitch, spitch, len, i, r = -1;

  window = (libsdl_window_t *)_window;

  rect.x = tx;
  rect.y = ty;
  rect.w = w;
  rect.h = h;

#ifdef LIBSDL_SOFT_SCENE
  if (texture && texture->buf && w > 0 && h > 0) {
    spitch = texture->width * window->spixel;
    src = &src[(ty * texture->width + tx) * window->spixel];
    dst = (uint8_t *)texture->buf + (ty * texture->width + tx) * window->spixel;
    len = w * window->spixel;
    for (i = 0; i < h; i++) {
      sys_memcpy(dst, src, len);
      src += spitch;
      dst += spitch;
    }
    r = 0;
  }
  (void)pixels; (void)pitch;
  if (0) {
#elif defined(GAMECUBE) || defined(WII)
  // the GameCube renderer's SDL_LockTexture() mallocs a staging buffer of
  // the locked size (600KB for the whole screen) and reports success even
  // when that fails, handing back a NULL pointer; SDL_UpdateTexture()
  // converts straight from the source into the texture, no allocation
  (void)pixels; (void)pitch; (void)dst; (void)len; (void)i;
  if (texture && texture->t && w > 0 && h > 0) {
    spitch = texture->width * window->spixel;
    if (SDL_UpdateTexture(texture->t, &rect, &src[(ty * texture->width + tx) * window->spixel], spitch)) {
      r = 0;
    } else {
      debug(DEBUG_ERROR, "SDL", "SDL_UpdateTexture failed: %s", SDL_GetError());
    }
  }
  if (0) {
#else
  if (SDL_LockTexture(texture->t, &rect, &pixels, &pitch)) {
    if (pixels == NULL) {
      // (a renderer whose staging allocation failed)
      debug(DEBUG_ERROR, "SDL", "SDL_LockTexture returned no pixels");
      SDL_UnlockTexture(texture->t);
      return -1;
    }
#endif
    src = &src[(ty * texture->width + tx) * window->spixel];
    dst = (uint8_t *)pixels;
    len = w * window->spixel;
    spitch = texture->width * window->spixel;
    for (i = 0; i < rect.h; i++) {
      sys_memcpy(dst, src, len);
      src += spitch;
      dst += pitch;
    }
    r = 0;
    SDL_UnlockTexture(texture->t);
  }

  return r;
}

static int libsdl_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  return libsdl_window_update_texture_rect(_window, texture, src, 0, 0, texture->width, texture->height);
}

static int libsdl_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libsdl_window_t *window;
  SDL_FRect src, dst;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture && w > 0 && h > 0) {
#ifdef SDL_OPENGL
    if (window->opengl) {
      window->texture = texture;
      return 0;
    }
#endif
#ifdef LIBSDL_SOFT_SCENE
    if (window->scene_tex) {
      return libsdl_scene_blit(window, (uint8_t *)texture->buf, texture->width, texture->height, tx, ty, w, h, x, y);
    }
#endif

    src.x = tx;
    src.y = ty;
    src.w = w;
    src.h = h;

    dst.x = x * window->xfactor;
    dst.y = y * window->yfactor;
    dst.w = w * window->xfactor;
    dst.h = h * window->yfactor;

    if (!SDL_RenderTexture(window->renderer, texture->t, &src, &dst)) {
      debug(DEBUG_ERROR, "SDL", "SDL_RenderTexture failed");
      r = -1;
    }
  }

  return r;
}

static int libsdl_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  libsdl_window_t *window;
  SDL_FRect dst;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture) {
#ifdef LIBSDL_SOFT_SCENE
    if (window->scene_tex) {
      return libsdl_scene_blit(window, (uint8_t *)texture->buf, texture->width, texture->height, 0, 0, texture->width, texture->height, x, y);
    }
#endif
    dst.x = x * window->xfactor;
    dst.y = y * window->yfactor;
    dst.w = texture->width  * window->xfactor;
    dst.h = texture->height * window->yfactor;

    if (!SDL_RenderTexture(window->renderer, texture->t, NULL, &dst)) {
      debug(DEBUG_ERROR, "SDL", "SDL_RenderTexture failed");
      r = -1;
    }
  }

  return r;
}

#ifndef LIBSDL_NOAUDIO
static void audio_destructor(void *p) {
  sdl_audio_t *audio = (sdl_audio_t *)p;

  if (audio) {
    sys_free(audio->buffer);
    sys_free(audio);
  }
}

static audio_t libsdl_audio_create(int pcm, int channels, int rate, void *data) {
  sdl_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_INFO, "SDL", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(sdl_audio_t))) != NULL) {
      switch (pcm) {
        case PCM_U8:
          audio->format = SDL_AUDIO_U8;
          audio->bsize = 1;
          break;
        case PCM_S16:
          audio->format = SDL_AUDIO_S16;
          audio->bsize = 2;
          break;
        case PCM_S32:
          audio->format = SDL_AUDIO_S32;
          audio->bsize = 4;
          break;
      }
      audio->rate = rate;
      audio->channels = channels;
      debug(DEBUG_INFO, "SDL", "sample size %d", audio->bsize);
      audio->bsize *= rate;
      if (audio->bsize > 4096) {
        audio->bsize = 4096;
      }
      audio->buffer = sys_calloc(1, audio->bsize * audio->channels);
      debug(DEBUG_INFO, "SDL", "buffer size %d", audio->bsize);

      audio->tag = TAG_AUDIO;
      if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
        sys_free(audio->buffer);
        sys_free(audio);
      }
    }
  }

  return ptr;
}

#ifndef LIBSDL_AUDIO_PUMP
static int libsdl_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  sdl_audio_t *audio;
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
#endif

static int libsdl_audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static int libsdl_mixer_init(void) {
  int r = -1;
#ifdef SDL_MIXER
  int i, n;
  char *s;

  if (MIX_Init() != false) {
    n = MIX_GetNumAudioDecoders();
    for (i = 0; i < n; i++) {
      s = (char *)MIX_GetAudioDecoder(i);
      if (s) debug(DEBUG_INFO, "SDL", "mixer audio decoder %s", s);
    }
    r = 0;
    debug(DEBUG_INFO, "SDL", "mixer init ok");
  } else {
    debug(DEBUG_ERROR, "SDL", "mixer init failed: %s", SDL_GetError());
  }
#endif

  return r;
}

#ifdef SDL_MIXER
// The FluidSynth MIDI decoder in SDL3_mixer needs an instrument bank (a
// SoundFont). It only auto-searches a single hardcoded path and there is no
// SDL_SOUNDFONT fallback in this build, so we locate one ourselves and hand
// it to the decoder through MIX_LoadAudioWithProperties(). Resolution order:
// the SDL_SOUNDFONT environment variable, then a soundfont bundled with
// PumpkinOS (looked up relative to the current working directory), then the
// FluidSynth default location.
static const char *libsdl_find_soundfont(void) {
  static const char *const candidates[] = {
    "soundfont.sf2",
    "./soundfont.sf2",
    "../soundfont.sf2", // PumpkinOS tree layout, running from bin/
#ifdef DOS
    "soundfon.sf2",     // 8.3 name
    "../soundfon.sf2",
#endif
#ifdef VITA
    "ux0:data/pumpkinos/soundfont.sf2",
    "app0:soundfont.sf2",
#endif
#ifdef SWITCH
    "sdmc:/switch/pumpkinos/soundfont.sf2",
    "romfs:/soundfont.sf2",
#endif
#ifdef PSP
    "ms0:/PSP/GAME/PumpkinOS/soundfont.sf2",
    "ef0:/PSP/GAME/PumpkinOS/soundfont.sf2",
#endif
    "/usr/local/share/soundfonts/default.sf2",
    NULL
  };
  sys_stat_t st;
  char *env;
  int i;

  if ((env = sys_getenv("SDL_SOUNDFONT")) != NULL && env[0]) {
    return env;
  }

  for (i = 0; candidates[i]; i++) {
    if (sys_stat(candidates[i], &st) == 0) {
      return candidates[i];
    }
  }

  return NULL;
}
#endif

static int libsdl_mixer_play(uint8_t *buf, uint32_t len, int volume) {
#ifdef SDL_MIXER
  MIX_Track *track;
  MIX_Audio *audio;
  SDL_PropertiesID props;
  SDL_IOStream *io;
  const char *soundfont;

  debug(DEBUG_INFO, "SDL", "mixer play begin");
  if (mixer != NULL) {
    if ((track = MIX_CreateTrack(mixer)) != NULL) {
      debug(DEBUG_INFO, "SDL", "mixer create track done");
      if ((io = SDL_IOFromConstMem(buf, len)) != NULL) {
        if ((props = SDL_CreateProperties()) != 0) {
          SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
          SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
          if ((soundfont = libsdl_find_soundfont()) != NULL) {
            SDL_SetStringProperty(props, "SDL_mixer.decoder.fluidsynth.soundfont_path", soundfont);
            debug(DEBUG_INFO, "SDL", "mixer using soundfont %s", soundfont);
          } else {
            debug(DEBUG_ERROR, "SDL", "no soundfont found; MIDI playback will be silent");
          }

          audio = MIX_LoadAudioWithProperties(props);
          SDL_DestroyProperties(props); // does not close the IO stream
          if (audio != NULL) {
            debug(DEBUG_INFO, "SDL", "mixer load audio done");
            if (MIX_SetTrackAudio(track, audio) != false) {
              if (volume >= 0) {
                MIX_SetTrackGain(track, ((float)volume) / 128.0f); // volume 0-128 -> gain 0.0-1.0
              }
              if (MIX_PlayTrack(track, 0) != false) {
                debug(DEBUG_INFO, "SDL", "mixer play track done");
                for (; MIX_TrackPlaying(track) && !thread_must_end();) {
                  sys_usleep(1000);
                }
                debug(DEBUG_INFO, "SDL", "mixer play track loop done");
              }
              MIX_SetTrackAudio(track, NULL); // detach before destroying the audio
            }
            MIX_DestroyAudio(audio);
          } else {
            debug(DEBUG_ERROR, "SDL", "mixer load audio failed: %s", SDL_GetError());
          }
        } else {
          SDL_CloseIO(io);
        }
      }
      MIX_DestroyTrack(track);
      debug(DEBUG_INFO, "SDL", "mixer free track done");
    }
  }
#endif

  return 0;
}

static int libsdl_mixer_stop(void) {
#ifdef SDL_MIXER
  debug(DEBUG_INFO, "SDL", "mixer stop");
  MIX_StopAllTracks(mixer, 0);
#endif
  return 0;
}

void libsdl_midi_finish(void) {
#ifdef SDL_MIXER
  if (mixer != NULL) {
    MIX_DestroyMixer(mixer);
  }
  MIX_Quit();
#endif
}
#endif

static void libsdl_window_status(window_t *window, int *x, int *y, int *buttons) {
  libsdl_status((libsdl_window_t *)window, x, y, buttons);
}

static void libsdl_window_title(window_t *window, char *title) {
  libsdl_title((libsdl_window_t *)window, title);
}

static char *libsdl_window_clipboard(window_t *window, char *clipboard, int len) {
  return libsdl_clipboard((libsdl_window_t *)window, clipboard, len);
}

static int libsdl_window_event(window_t *window, int wait, int remove, int *key, int *mods, int *buttons) {
  return libsdl_event((libsdl_window_t *)window, wait, remove, key, mods, buttons);
}

static int libsdl_window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  return libsdl_event2((libsdl_window_t *)window, wait, arg1, arg2);
}

static int libsdl_window_update(window_t *_window, int x, int y, int width, int height) {
  return 0;
}

#ifdef SDL_OPENGL
static GLuint compileShader(const char *source, GLint len, GLuint shaderType) {
  GLuint result;
  GLint shaderCompiled, logLength;
  GLchar *log;
  char *type;
  PFNGLCREATESHADERPROC glCreateShader;
  PFNGLSHADERSOURCEPROC glShaderSource;
  PFNGLCOMPILESHADERPROC glCompileShader;
  PFNGLGETSHADERIVPROC glGetShaderiv;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
  PFNGLDELETESHADERPROC glDeleteShader;

  glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
  result = 0;

  if (glCreateShader && glShaderSource && glCompileShader && glGetShaderiv && glGetShaderInfoLog && glDeleteShader) {
    type = shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment";
    result = glCreateShader(shaderType);
    glShaderSource(result, 1, &source, &len);
    glCompileShader(result);

    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      log = (GLchar *)sys_malloc(logLength);
      glGetShaderInfoLog(result, logLength, &logLength, log);
      debug(DEBUG_INFO, "SDL", "%s shader compilation log: %s", type, log);
      sys_free(log);
    }

    shaderCompiled = GL_FALSE;
    glGetShaderiv(result, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled) {
      debug(DEBUG_ERROR, "SDL", "error compiling %s shader: %d", type, result);
      glDeleteShader(result);
      result = 0;
    } else {
      debug(DEBUG_INFO, "SDL", "%s shader compiled successfully", type);
    }
  } else {
    debug(DEBUG_ERROR, "SDL", "compileShader: one or more OpenGL functions are not available");
  }

  return result;
}

static int libsdl_window_shader(window_t *_window, int i, char *vertex, int vlen, char *fragment, int flen, float (*getvar)(char *name, void *data), void *data) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  GLint logLen, linkStatus, validateStatus;
  GLuint vtxShaderId, fragShaderId;
  char *log;
  int r = -1;
  PFNGLCREATEPROGRAMPROC glCreateProgram;
  PFNGLLINKPROGRAMPROC glLinkProgram;
  PFNGLVALIDATEPROGRAMPROC glValidateProgram;
  PFNGLGETPROGRAMIVPROC glGetProgramiv;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  PFNGLATTACHSHADERPROC glAttachShader;
  PFNGLDELETESHADERPROC glDeleteShader;

  glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
  glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
  glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
  glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");

  if (glCreateProgram && glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog && glAttachShader && glDeleteShader) {
    window->programId[i] = glCreateProgram();
    vtxShaderId = compileShader(vertex, vlen, GL_VERTEX_SHADER);
    fragShaderId = compileShader(fragment, flen, GL_FRAGMENT_SHADER);

    if (vtxShaderId && fragShaderId) {
      glAttachShader(window->programId[i], vtxShaderId);
      glAttachShader(window->programId[i], fragShaderId);
      glLinkProgram(window->programId[i]);
      glGetProgramiv(window->programId[i], GL_LINK_STATUS, &linkStatus);
      glValidateProgram(window->programId[i]);
      glGetProgramiv(window->programId[i], GL_VALIDATE_STATUS, &validateStatus);
      window->getvar = getvar;
      window->data = data;
      debug(DEBUG_INFO, "SDL", "program link status %d, validate status %d", linkStatus, validateStatus);
      window->opengl = linkStatus && validateStatus;
      r = window->opengl ? 0 : -1;

      glGetProgramiv(window->programId[i], GL_INFO_LOG_LENGTH, &logLen);
      if (logLen > 0) {
        log = (char *)sys_malloc(logLen * sizeof(char));
        glGetProgramInfoLog(window->programId[i], logLen, &logLen, log);
        debug(DEBUG_INFO, "SDL", "shader program log: %s", log);
        sys_free(log);
      }
    }

    if (vtxShaderId) {
      glDeleteShader(vtxShaderId);
    }

    if (fragShaderId) {
      glDeleteShader(fragShaderId);
    }
  } else {
    debug(DEBUG_ERROR, "SDL", "libsdl_window_shader: one or more OpenGL functions are not available");
  }

  return r;
}
#endif

static int libsdl_window_drop_file(window_t *_window, void (*callback)(char *filename, void *data), void *data) {
  libsdl_window_t *window = (libsdl_window_t *)_window;

  if (window) {
    window->drop_callback = callback;
    window->drop_data = data;
  }

  return 0;
}

static int libsdl_window_destroy(window_t *window) {
  return libsdl_video_close((libsdl_window_t *)window);
}

static int libsdl_window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&window_provider, window, x, y, ms);
}

static int libsdl_calib(int pe) {
  window_provider.average = libsdl_window_average;
  return 0;
}

static int set_video_driver(char *video_driver) {
#ifdef SDL_HINT_VIDEODRIVER
  if (video_driver) debug(DEBUG_INFO, "SDL", "hint video driver \"%s\"", video_driver);
  SDL_SetHint(SDL_HINT_VIDEODRIVER, video_driver);
#else
  if (video_driver) debug(DEBUG_INFO, "SDL", "setenv video driver \"%s\"", video_driver);
  SDL_setenv_unsafe("SDL_VIDEODRIVER", video_driver, 1);
#endif
  return 1;
}

#if !defined(LIBSDL_NOAUDIO) && !defined(LIBSDL_AUDIO_PUMP)
static int audio_action(void *_arg) {
  sdl_audio_arg_t *arg = (sdl_audio_arg_t *)_arg;
  SDL_AudioDeviceID dev = 0;
  SDL_AudioStream *audioStream = NULL;
  SDL_AudioSpec desired, obtained;
  sdl_audio_t *audio;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr;
  int len1, len2, frame, r;

  debug(DEBUG_INFO, "SDL", "audio thread starting");

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(2000, &msg, &msglen)) == -1) {
      break;
    }

    if (r == 1) {
      if (dev == 0) {
        switch (arg->pcm) {
          case PCM_U8:  desired.format = SDL_AUDIO_U8;  break;
          case PCM_S16: desired.format = SDL_AUDIO_S16; break;
          case PCM_S32: desired.format = SDL_AUDIO_S32; break;
        }
        desired.freq = arg->rate; // DSP frequency -- samples per second
        desired.channels = arg->channels; // Number of channels: 1 mono, 2 stereo

        audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired, NULL, NULL);
        if (audioStream == NULL) {
          debug(DEBUG_ERROR, "SDL", "SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
          break;
        }

        if ((dev = SDL_GetAudioStreamDevice(audioStream)) == 0) {
          debug(DEBUG_ERROR, "SDL", "SDL_GetAudioStreamDevice failed: %s", SDL_GetError());
          break;
        }

        SDL_GetAudioStreamFormat(audioStream, &desired, &obtained);
        SDL_ResumeAudioStreamDevice(audioStream);
        SDL_ResumeAudioDevice(dev);

        debug(DEBUG_INFO, "SDL", "open device %d", dev);

#ifdef SDL_MIXER
      if ((mixer = MIX_CreateMixerDevice(dev, &obtained)) == NULL) {
        debug(DEBUG_ERROR, "SDL", "create mixer failed: %s", SDL_GetError());
      } else {
        debug(DEBUG_INFO, "SDL", "mixer inited");
      }
#endif
      }

      if (msg) {
        if (msglen == 4) {
          ptr = *((uint32_t *)msg);
          debug(DEBUG_INFO, "SDL", "received ptr %d", ptr);
          if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
            // If this source's format differs from the stream's current input
            // format, retarget the stream input; SDL converts and resamples
            // to the device format internally.
            if (audio->format != desired.format ||
                audio->channels != desired.channels ||
                audio->rate != desired.freq) {
              SDL_AudioSpec inspec;
              inspec.format = audio->format;
              inspec.channels = audio->channels;
              inspec.freq = audio->rate;
              debug(DEBUG_INFO, "SDL", "set stream input format=0x%04X channels=%d rate=%d", inspec.format, inspec.channels, inspec.freq);
              if (SDL_SetAudioStreamFormat(audioStream, &inspec, NULL) == false) {
                debug(DEBUG_ERROR, "SDL", "SDL_SetAudioStreamFormat failed: %s", SDL_GetError());
              } else {
                desired = inspec;
              }
            }
            frame = SDL_AUDIO_BYTESIZE(audio->format) * audio->channels;
            uint64_t pace_start = sys_get_clock();
            uint64_t pace_frames = 0;
            for (; !thread_must_end();) {
              // SDL_PutAudioStreamData never blocks, so without pacing this
              // loop would drain the app's audio callback as fast as the CPU
              // allows and audio would play far ahead of (and faster than)
              // real time. Feed the stream at the source sample rate, keeping
              // at most ~100 ms of lead over the wall clock.
              uint64_t ahead = (pace_frames * 1000000ULL) / audio->rate;
              uint64_t elapsed = sys_get_clock() - pace_start;
              if (ahead > elapsed + 100000) {
                sys_usleep(ahead - elapsed - 100000);
              }
              len1 = audio->bsize * audio->channels;
              len2 = audio->getaudio(audio->buffer, len1, audio->data);
              debug(DEBUG_TRACE, "SDL", "get audio len=%d bytes", len2);
              if (len2 <= 0) break;
              len2 -= len2 % frame; // SDL rejects partial sample frames
              if (SDL_PutAudioStreamData(audioStream, audio->buffer, len2) == false) {
                debug(DEBUG_ERROR, "SDL", "SDL_PutAudioStreamData failed: %s", SDL_GetError());
              }
              pace_frames += len2 / frame;
              if (len2 != len1) break;
            }
            ptr_unlock(ptr, TAG_AUDIO);
          }
          ptr_free(ptr, TAG_AUDIO);
          debug(DEBUG_INFO, "SDL", "handled ptr %d", ptr);
        }
        sys_free(msg);
      }
    }
  }

  if (dev > 0) {
    debug(DEBUG_INFO, "SDL", "close device %d", dev);
    SDL_CloseAudioDevice(dev);
  }

  if (audioStream != NULL) {
    debug(DEBUG_INFO, "SDL", "close stream %p", audioStream);
    SDL_DestroyAudioStream(audioStream);
  }

  debug(DEBUG_INFO, "SDL", "audio thread exiting");

  return 0;
}

static int libsdl_audio_init(int pcm, int channels, int rate) {
  sdl_audio_arg_t *arg = sys_calloc(1, sizeof(sdl_audio_arg_t));
  arg->pcm = pcm;
  arg->channels = channels;
  arg->rate = rate;
  return thread_begin("AUDIO", audio_action, arg);
}

static int libsdl_audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}
#endif

#ifdef LIBSDL_AUDIO_PUMP
// Single threaded audio provider (MS-DOS), see the comment at the top.
// The sources (SndStreams) started by the applications are queued and
// played one after the other, like the audio thread does.

#define LIBSDL_PUMP_MAX_SOURCES 16

typedef struct {
  SDL_AudioDeviceID dev;
  SDL_AudioStream *stream;
  SDL_AudioSpec spec;     // current input format of the stream
  uint32_t ptrs[LIBSDL_PUMP_MAX_SOURCES]; // queue of audio sources
  int iptr, nptr;
} sdl_audio_pump_t;

static sdl_audio_pump_t pump;
static uint64_t pump_stat_t0;  // periodic statistics (bytes moved per interval)
static uint32_t pump_stat_bytes;

static int libsdl_audio_init(int pcm, int channels, int rate) {
  SDL_AudioSpec desired, obtained;

  if (pump.stream != NULL) return 1;

  sys_memset(&desired, 0, sizeof(desired));
  switch (pcm) {
    case PCM_U8:  desired.format = SDL_AUDIO_U8;  break;
    case PCM_S32: desired.format = SDL_AUDIO_S32; break;
    default:      desired.format = SDL_AUDIO_S16; break;
  }
  desired.freq = rate;
  desired.channels = channels;

  if ((pump.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired, NULL, NULL)) == NULL) {
    debug(DEBUG_ERROR, "SDL", "SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
    return -1;
  }

  if ((pump.dev = SDL_GetAudioStreamDevice(pump.stream)) == 0) {
    debug(DEBUG_ERROR, "SDL", "SDL_GetAudioStreamDevice failed: %s", SDL_GetError());
    SDL_DestroyAudioStream(pump.stream);
    pump.stream = NULL;
    return -1;
  }

  SDL_GetAudioStreamFormat(pump.stream, &pump.spec, &obtained);
  SDL_ResumeAudioStreamDevice(pump.stream);
  SDL_ResumeAudioDevice(pump.dev);
  pump.iptr = pump.nptr = 0;
  debug(DEBUG_INFO, "SDL", "open device %d (%d Hz, %d channel(s), format 0x%04X)", pump.dev, obtained.freq, obtained.channels, obtained.format);

#ifdef SDL_MIXER
  if ((mixer = MIX_CreateMixerDevice(pump.dev, &obtained)) == NULL) {
    debug(DEBUG_ERROR, "SDL", "create mixer failed: %s", SDL_GetError());
  } else {
    debug(DEBUG_INFO, "SDL", "mixer inited");
  }
#endif

  // the handle is only tested for > 0 by the callers
  return 1;
}

static int libsdl_audio_finish(int handle) {
  for (; pump.nptr > 0; pump.nptr--) {
    ptr_free(pump.ptrs[pump.iptr], TAG_AUDIO);
    pump.iptr = (pump.iptr + 1) % LIBSDL_PUMP_MAX_SOURCES;
  }

  if (pump.dev > 0) {
    debug(DEBUG_INFO, "SDL", "close device %d", pump.dev);
    SDL_CloseAudioDevice(pump.dev);
    pump.dev = 0;
  }

  if (pump.stream != NULL) {
    SDL_DestroyAudioStream(pump.stream);
    pump.stream = NULL;
  }

  return 0;
}

static int libsdl_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  sdl_audio_t *audio;
  int r = -1;

  if (handle > 0 && pump.stream != NULL && (audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    audio->getaudio = getaudio;
    audio->data = data;
    ptr_unlock(_audio, TAG_AUDIO);

    if (pump.nptr < LIBSDL_PUMP_MAX_SOURCES) {
      pump.ptrs[(pump.iptr + pump.nptr) % LIBSDL_PUMP_MAX_SOURCES] = _audio;
      pump.nptr++;
      debug(DEBUG_INFO, "SDL", "audio source %d queued (%d queued)", _audio, pump.nptr);
      libsdl_audio_pump();
      r = 0;
    } else {
      debug(DEBUG_ERROR, "SDL", "audio source queue full");
    }
  }

  return r;
}

// Pull from the current source until the stream holds LIBSDL_PUMP_LEAD_MS
// of audio. Returns 1 if a source is (still) playing, 0 otherwise.
static int libsdl_audio_pump(void) {
  sdl_audio_t *audio;
  SDL_AudioSpec inspec;
  uint32_t ptr;
  int len1, len2, frame, lead, done, n;

  if (pump.stream == NULL) return 0;

  for (n = 0; pump.nptr > 0 && n < 4; n++) {
    ptr = pump.ptrs[pump.iptr];
    done = 1;

    if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
      done = 0;
      if (audio->format != pump.spec.format || audio->channels != pump.spec.channels || audio->rate != pump.spec.freq) {
        inspec.format = audio->format;
        inspec.channels = audio->channels;
        inspec.freq = audio->rate;
        debug(DEBUG_INFO, "SDL", "set stream input format=0x%04X channels=%d rate=%d", inspec.format, inspec.channels, inspec.freq);
        if (SDL_SetAudioStreamFormat(pump.stream, &inspec, NULL) == false) {
          debug(DEBUG_ERROR, "SDL", "SDL_SetAudioStreamFormat failed: %s", SDL_GetError());
        } else {
          pump.spec = inspec;
        }
      }
      frame = SDL_AUDIO_BYTESIZE(audio->format) * audio->channels;
      lead = (audio->rate * frame * LIBSDL_PUMP_LEAD_MS) / 1000;

      while (SDL_GetAudioStreamQueued(pump.stream) < lead) {
        len1 = audio->bsize * audio->channels;
        len2 = audio->getaudio(audio->buffer, len1, audio->data);
        debug(DEBUG_TRACE, "SDL", "get audio len=%d bytes", len2);
        if (len2 <= 0) {
          done = 1;
          break;
        }
        len2 -= len2 % frame; // SDL rejects partial sample frames
        if (SDL_PutAudioStreamData(pump.stream, audio->buffer, len2) == false) {
          debug(DEBUG_ERROR, "SDL", "SDL_PutAudioStreamData failed: %s", SDL_GetError());
        }
        pump_stat_bytes += len2;
        if (len2 != len1) {
          // a short buffer ends the source, like in the audio thread
          done = 1;
          break;
        }
      }
      ptr_unlock(ptr, TAG_AUDIO);

      // once every 5 seconds: bytes pushed since the last report and bytes
      // still queued (steady pushing with a bounded queue means the device
      // is consuming the audio)
      uint64_t now = sys_get_clock();
      if (pump_stat_t0 == 0) pump_stat_t0 = now;
      if (now - pump_stat_t0 >= 5000000) {
        debug(DEBUG_INFO, "SDL", "audio pump: %u bytes pushed in %u ms, %d queued", pump_stat_bytes, (uint32_t)((now - pump_stat_t0) / 1000), SDL_GetAudioStreamQueued(pump.stream));
        pump_stat_t0 = now;
        pump_stat_bytes = 0;
      }
    }

    if (!done) break;

    // this source is finished: free it (like the audio thread) and go on with the next one
    ptr_free(ptr, TAG_AUDIO);
    debug(DEBUG_INFO, "SDL", "audio source %d finished", ptr);
    pump.iptr = (pump.iptr + 1) % LIBSDL_PUMP_MAX_SOURCES;
    pump.nptr--;
  }

  return pump.nptr > 0;
}
#endif

int liblsdl3_load(void) {
  int version;
  int set = 0;

  version = SDL_GetVersion();
  debug(DEBUG_INFO, "SDL", "version %d.%d.%d", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));

  if (!SDL_getenv("SDL_VIDEODRIVER") && SDL_getenv("WAYLAND_DISPLAY") != NULL) {
    set = set_video_driver("wayland");
  }

#ifndef LIBSDL_NOAUDIO
  libsdl_init_audio();
#endif

  if (libsdl_init_video() != 0) {
    SDL_Quit();
    return -1;
  }

  SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, true);

  if (set) {
    set_video_driver(NULL);
  }

  sys_memset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libsdl_window_create;
  window_provider.event = libsdl_window_event;
  window_provider.destroy = libsdl_window_destroy;
  window_provider.erase = libsdl_window_erase;
  window_provider.render = libsdl_window_render;
  window_provider.background = libsdl_window_background;
  window_provider.create_texture = libsdl_window_create_texture;
  window_provider.destroy_texture = libsdl_window_destroy_texture;
  window_provider.update_texture = libsdl_window_update_texture;
  window_provider.draw_texture = libsdl_window_draw_texture;
  window_provider.status = libsdl_window_status;
  window_provider.title = libsdl_window_title;
  window_provider.clipboard = libsdl_window_clipboard;
  window_provider.event2 = libsdl_window_event2;
  window_provider.update = libsdl_window_update;
  window_provider.draw_texture_rect = libsdl_window_draw_texture_rect;
  window_provider.update_texture_rect = libsdl_window_update_texture_rect;
  window_provider.show_cursor = libsdl_window_show_cursor;
#ifdef SDL_OPENGL
  window_provider.shader = libsdl_window_shader;
#endif
  window_provider.drop_file = libsdl_window_drop_file;

#ifndef LIBSDL_NOAUDIO
  sys_memset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.init = libsdl_audio_init;
  audio_provider.finish = libsdl_audio_finish;
  audio_provider.create = libsdl_audio_create;
  audio_provider.start = libsdl_audio_start;
  audio_provider.destroy = libsdl_audio_destroy;
  audio_provider.mixer_init = libsdl_mixer_init;
  audio_provider.mixer_play = libsdl_mixer_play;
  audio_provider.mixer_stop = libsdl_mixer_stop;
#endif

  return 0;
}

int liblsdl3_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "SDL", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

#ifndef LIBSDL_NOAUDIO
  debug(DEBUG_INFO, "SDL", "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);
#endif

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
  script_add_iconst(pe, obj, "hdepth", 16);

  script_add_function(pe, obj, "calib", libsdl_calib);

  return 0;
}

int liblsdl3_unload(void) {
  SDL_Quit();
  return 0;
}
