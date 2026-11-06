#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <PalmOS.h>

#include "main.h"
#include "drop.h"
#include "midi.h"
#include "thread.h"
#include "script.h"
#include "sys.h"
#include "pwindow.h"
#include "audio.h"
#include "media.h"
#include "ptr.h"
#include "average.h"
#include "pumpkin.h"
#include "debug.h"

#ifdef _WIN32_WCE
#include <timidity.h>
#endif

#define TAG_AUDIO  "audio"

#define MAX_ARGS   16
#define MAX_EVENTS 16

#if defined(_WIN32_WCE) && !defined(CE_EMU)
#ifndef LCS_GM_BUSINESS
#define LCS_GM_BUSINESS 0x00000001L
#endif

#ifndef LCS_WINDOWS_COLOR_SPACE
#define LCS_WINDOWS_COLOR_SPACE 'Win '  // Windows default color space
#endif

typedef struct _PAINTSTRUCT PAINTSTRUCT;
typedef long            FXPT2DOT30, FAR *LPFXPT2DOT30;

typedef struct tagCIEXYZ
{
	FXPT2DOT30 ciexyzX;
	FXPT2DOT30 ciexyzY;
	FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct tagICEXYZTRIPLE
{
	CIEXYZ  ciexyzRed;
	CIEXYZ  ciexyzGreen;
	CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct {
	DWORD        bV5Size;
	LONG         bV5Width;
	LONG         bV5Height;
	WORD         bV5Planes;
	WORD         bV5BitCount;
	DWORD        bV5Compression;
	DWORD        bV5SizeImage;
	LONG         bV5XPelsPerMeter;
	LONG         bV5YPelsPerMeter;
	DWORD        bV5ClrUsed;
	DWORD        bV5ClrImportant;
	DWORD        bV5RedMask;
	DWORD        bV5GreenMask;
	DWORD        bV5BlueMask;
	DWORD        bV5AlphaMask;
	DWORD        bV5CSType;
	CIEXYZTRIPLE bV5Endpoints;
	DWORD        bV5GammaRed;
	DWORD        bV5GammaGreen;
	DWORD        bV5GammaBlue;
	DWORD        bV5Intent;
	DWORD        bV5ProfileData;
	DWORD        bV5ProfileSize;
	DWORD        bV5Reserved;
} BITMAPV5HEADER, FAR *LPBITMAPV5HEADER, *PBITMAPV5HEADER;
#endif

typedef struct {
	int ev;
	int arg1;
	int arg2;
} win_event_t;

typedef struct {
  BITMAPV5HEADER bmiHeader;
  DWORD bmiColors[3];
} V5BMPINFO;

typedef struct {
  HWND hwnd;
  IDropTarget *dt;
  V5BMPINFO bmi;
  HBITMAP bitmap;
  HDC hdcBitmap;
  uint16_t *bitmapBuffer;
  int width, height, size;
  win_event_t events[MAX_EVENTS];
  int num_ev, ie, oe, down;
  int x, y, buttons;
  int last_key;
  uint64_t last_timestamp;
} win_window_t;

struct texture_t {
  int width, height, size;
  uint16_t *buffer;
};

typedef struct {
  char *tag;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  int pcm, channels, rate;
  uint8_t *buffer;
  uint32_t bsize;   // bytes per channel in one chunk; buffer holds bsize * channels bytes
} win_audio_t;

#define WAVE_NUM_BUFFERS 3

// state of the waveOut device owned by the audio thread
typedef struct {
  HWAVEOUT hwo;
  WAVEFORMATEX fmt;
  int pcm, channels, rate;
  WAVEHDR hdr[WAVE_NUM_BUFFERS];
  uint8_t *obuf[WAVE_NUM_BUFFERS];
  uint32_t osize[WAVE_NUM_BUFFERS];
  int queued[WAVE_NUM_BUFFERS];
  int next;
} win_wave_t;

typedef struct {
  int pcm, channels, rate;
} win_audio_arg_t;

static const wchar_t CLASS_NAME[]  = L"PumpkinOS";
static const wchar_t WINDOW_PROP[] = L"window";

static window_provider_t wp;
static audio_provider_t ap;

static void putEvent(win_window_t *window, win_event_t *ev) {
	if (window->num_ev < MAX_EVENTS) {
		sys_memcpy(&window->events[window->ie], ev, sizeof(win_event_t));
		window->ie++;
		if (window->ie >= MAX_EVENTS) window->ie = 0;
		window->num_ev++;
	}
}

static int getEvent(win_window_t *window, win_event_t *ev) {
	if (window->num_ev > 0) {
		sys_memcpy(ev, &window->events[window->oe], sizeof(win_event_t));
		window->oe++;
		if (window->oe >= MAX_EVENTS) window->oe = 0;
		window->num_ev--;
		return 1;
	}

	return 0;
}

static int mapKey(int code) {
  switch (code) {
    case VK_BACK:   code = 8;  break;
    case VK_TAB:    code = 9;  break;
    case VK_RETURN: code = 13;  break;
    case VK_HOME:   code = WINDOW_KEY_HOME;  break;
    case VK_LEFT:   code = WINDOW_KEY_LEFT;  break;
    case VK_UP:     code = WINDOW_KEY_UP;    break;
    case VK_RIGHT:  code = WINDOW_KEY_RIGHT; break;
    case VK_DOWN:   code = WINDOW_KEY_DOWN;  break;
    case VK_F1:     code = WINDOW_KEY_F1;    break;
    case VK_F2:     code = WINDOW_KEY_F2;    break;
    case VK_F3:     code = WINDOW_KEY_F3;    break;
    case VK_F4:     code = WINDOW_KEY_F4;    break;
    case VK_F5:     code = WINDOW_KEY_F5;    break;
    case VK_F6:     code = WINDOW_KEY_F6;    break;
    case VK_F7:     code = WINDOW_KEY_F7;    break;
    case VK_F8:     code = WINDOW_KEY_F8;    break;
    case VK_F9:     code = WINDOW_KEY_F9;    break;
    case VK_F10:    code = WINDOW_KEY_F10;   break;
    case VK_F11:    code = WINDOW_KEY_F11;   break;
    case VK_F12:    code = WINDOW_KEY_F12;   break;
    default: code = 0;
  }

  return code;
}

static int mapChar(WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32_WCE
  // 1. Windows CE 8/WEC2013: Map the virtual key to a character.
  // MAPVK_VK_TO_CHAR (2) returns the unshifted Unicode character.
  int code = MapVirtualKey(wParam, 2);

  // 2. Manual modifier check since GetKeyboardState is missing.
  // Use GetKeyState to check if SHIFT is pressed.
  SHORT shiftState = GetKeyState(VK_SHIFT);
  BOOL isShifted = (shiftState & 0x8000) != 0;

  // 3. Optional: Handle basic Shift logic manually for A-Z.
  // For complex layouts, it is better to handle the WM_CHAR message directly.
  if (isShifted && code >= 'a' && code <= 'z') {
    code = code - 'a' + 'A';
  }

  return code;
#else
  UINT scandCode;
  WCHAR lBuffer[16]; 
  BYTE State[256];    
  int r, code = 0; 

  scandCode = (lParam >> 8) & 0xFFFFFF00; 
  GetKeyboardState(State);
  if ((r = ToUnicode((UINT)wParam, scandCode, State, lBuffer, 16, 0)) == 1) {
    code = lBuffer[0];
  } else if (r < 0) {
    // dead key, "maybe" there is something in the buffer
    code = lBuffer[0];
    debug(DEBUG_TRACE, "Windows", "dead key %d", code);
  }

  return code;
#endif
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  win_window_t *window;
  win_event_t ev;
  PAINTSTRUCT ps;
  HDC hdc;
  int code;
  uint64_t timestamp;

  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      pumpkin_set_finish(1);
      return 0;

    // XXX some dead keys are not mapped by this

    case WM_KEYDOWN:
      debug(DEBUG_TRACE, "Windows", "keyDown %u 0x%08X", (uint32_t)wParam, (uint32_t)lParam);
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_KEYDOWN: window property is NULL!");
		  return 0;
	  }

      code = mapKey((int)wParam);
      debug(DEBUG_TRACE, "Windows", "keyDown mapKey %d", code);
      if (code == 0) {
        code = mapChar(wParam, lParam);
        debug(DEBUG_TRACE, "Windows", "keyDown mapChar %d", code);
      }
      if (code) {
        debug(DEBUG_TRACE, "Windows", "keyDown event");
        // try to avoid multiple key press events when holding down a key
        timestamp = sys_get_clock();
        if (code != window->last_key || timestamp - window->last_timestamp > 200000) {
          window->last_timestamp = timestamp;
          window->last_key = code;
          ev.ev = WINDOW_KEYDOWN;
          ev.arg1 = code;
          ev.arg2 = 0;
          putEvent(window, &ev);
        }
      }
      return 0;

    case WM_KEYUP:
      debug(DEBUG_TRACE, "Windows", "keyUp %u 0x%08X", (uint32_t)wParam, (uint32_t)lParam);
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_KEYUP: window property is NULL!");
		  return 0;
	  }

      code = mapKey((int)wParam);
      debug(DEBUG_TRACE, "Windows", "keyUp mapKey %d", code);
      if (code == 0) {
        code = mapChar(wParam, lParam);
        debug(DEBUG_TRACE, "Windows", "keyUp mapChar %d", code);
      }
      if (code) {
        debug(DEBUG_TRACE, "Windows", "keyUp event");
        ev.ev = WINDOW_KEYUP;
        ev.arg1 = code;
        ev.arg2 = 0;
        putEvent(window, &ev);
      }
      return 0;

    case WM_MOUSEMOVE:
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_MOUSEMOVE: window property is NULL!");
		  return 0;
	  }

      ev.ev = WINDOW_MOTION;
      ev.arg1 = GET_X_LPARAM(lParam);
      ev.arg2 = GET_Y_LPARAM(lParam);
      window->x = ev.arg1;
      window->y = ev.arg2;
      putEvent(window, &ev);
#ifdef _DEBUG
	  debug(DEBUG_ERROR, "Windows", "WM_MOUSEMOVE: x - %d, y - %d", window->x, window->y);
#endif
      return 0;

    case WM_LBUTTONDOWN:
      debug(DEBUG_TRACE, "Windows", "button 1 down %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_LBUTTONDOWN: window property is NULL!");
		  return 0;
	  }

      window->down = 1;
      ev.ev = WINDOW_MOTION;
      ev.arg1 = GET_X_LPARAM(lParam);
      ev.arg2 = GET_Y_LPARAM(lParam);
      window->x = ev.arg1;
      window->y = ev.arg2;
      window->buttons = 1;
      putEvent(window, &ev);
      ev.ev = WINDOW_BUTTONDOWN;
      ev.arg1 = 1;
      ev.arg2 = 0;
      putEvent(window, &ev);
#ifdef _DEBUG
	  debug(DEBUG_ERROR, "Windows", "WM_LBUTTONDOWN: x - %d, y - %d", window->x, window->y);
#endif
	  return 0;


    case WM_LBUTTONUP:
      debug(DEBUG_TRACE, "Windows", "button up %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_LBUTTONUP: window property is NULL!");
		  return 0;
	  }

      window->down = 0;
      ev.ev = WINDOW_BUTTONUP;
      ev.arg1 = 1;
      ev.arg2 = 0;
      window->buttons = 0;
      putEvent(window, &ev);
#ifdef _DEBUG
	  debug(DEBUG_ERROR, "Windows", "WM_LBUTTONUP: x - %d, y - %d", window->x, window->y);
#endif
      return 0;

    case WM_RBUTTONDOWN:
      debug(DEBUG_TRACE, "Windows", "button 2 down %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_MOUSEMOVE: window property is NULL!");
		  return 0;
	  }

      window->down = 1;
      ev.ev = WINDOW_MOTION;
      ev.arg1 = GET_X_LPARAM(lParam);
      ev.arg2 = GET_Y_LPARAM(lParam);
      window->x = ev.arg1;
      window->y = ev.arg2;
      putEvent(window, &ev);
      ev.ev = WINDOW_BUTTONDOWN;
      ev.arg1 = 2;
      ev.arg2 = 0;
      putEvent(window, &ev);
	  debug(DEBUG_ERROR, "Windows", "WM_RBUTTONDOWN: x - %d, y - %d", window->x, window->y);
      return 0;

    case WM_RBUTTONUP:
      debug(DEBUG_TRACE, "Windows", "button 2 up %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);

	  if (window == NULL) {
		  // Dit mag niet gebeuren als SetProp correct is aangeroepen
		  debug(DEBUG_ERROR, "Windows", "WM_RBUTTONUP: window property is NULL!");
		  return 0;
	  }

      window->down = 0;
      ev.ev = WINDOW_BUTTONUP;
      ev.arg1 = 2;
      ev.arg2 = 0;
      putEvent(window, &ev);
#ifdef _DEBUG
	  debug(DEBUG_ERROR, "Windows", "WM_RBUTTONUP: x - %d, y - %d", window->x, window->y);
#endif
      return 0;

    case WM_PAINT:
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      hdc = BeginPaint(hwnd, &ps);
      debug(DEBUG_TRACE, "Windows", "paint %d %d %d %d", (int32_t)ps.rcPaint.left, (int32_t)ps.rcPaint.right, (int32_t)ps.rcPaint.top, (int32_t)ps.rcPaint.bottom);
	  if (window && window->bitmapBuffer) {
#ifdef _WIN32_WCE
	    SetDIBitsToDevice(
		  hdc,                             // Destination Device Context
		  0, 0,                            // Destination X and Y coordinates
		  window->width,                   // Width of the source rectangle
		  window->height,                  // Height of the source rectangle
		  0, 0,                            // Source X and Y coordinates (usually 0, 0)
		  0,                               // First scan line in the array (usually 0)
		  window->height,                  // Number of scan lines in the array
		  window->bitmapBuffer,            // Pointer to your raw pixel data
		  (BITMAPINFO *)&window->bmi,      // Pointer to BITMAPINFO structure
		  DIB_RGB_COLORS                   // Color format (DIB_RGB_COLORS or DIB_PAL_COLORS)
		  ); 
#else
        if (window->bitmap) {
          SetDIBits(hdc, window->bitmap, 0, window->height, window->bitmapBuffer, (BITMAPINFO *)&window->bmi, DIB_RGB_COLORS);
		}
#endif
        BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right-ps.rcPaint.left+1, ps.rcPaint.bottom-ps.rcPaint.top+1, window->hdcBitmap, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	  }
      EndPaint(hwnd, &ps);
      return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#ifndef WS_OVERLAPPEDWINDOW
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#endif

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  win_window_t *window;
  HINSTANCE hInstance;
  BITMAPV5HEADER bmh;
  RECT rect;
  HDC hdc;

  debug(DEBUG_TRACE, "Windows", "window_create(%d,%d,%d,%d,%d,%d,%d,%d)", encoding, *width, *height, xfactor, yfactor, rotate, fullscreen, software);

  if ((window = sys_malloc(sizeof(win_window_t))) != NULL) {
    hInstance = (HINSTANCE)data;

    rect.left = rect.top = 0;
    rect.right = *width;
    rect.bottom = *height;
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	window->ie = 0;
	window->oe = 0;
	window->num_ev = 0;
	window->x = 0;
	window->y = 0;
	window->down = 0;
	window->buttons = 0;

	sys_memset(window->events, 0, sizeof(window->events));

    window->hwnd = CreateWindowEx(
      0,                              // Optional window styles.
      CLASS_NAME,                     // Window class
      L"PumpkinOS",                   // Window text
      WS_OVERLAPPEDWINDOW,            // Window style
      CW_USEDEFAULT, CW_USEDEFAULT,
      rect.right - rect.left, rect.bottom - rect.top,
      NULL,       // Parent window
      NULL,       // Menu
      hInstance,  // Instance handle
      NULL        // Additional application data
    );
    SetProp(window->hwnd, WINDOW_PROP, window);

    window->width = *width;
    window->height = *height;

    window->size = window->width * window->height * sizeof(uint16_t);

    hdc = GetDC(window->hwnd);
    window->hdcBitmap = CreateCompatibleDC(hdc);
	window->bitmapBuffer = NULL;

    sys_memset(&window->bmi, 0, sizeof(V5BMPINFO));
    sys_memset(&bmh, 0, sizeof(BITMAPV5HEADER));
    bmh.bV5Size = sizeof(BITMAPV5HEADER);
    bmh.bV5Width = window->width;
    bmh.bV5Height = window->height;
    bmh.bV5Planes = 1;
    bmh.bV5BitCount = 16;
    bmh.bV5Compression = BI_BITFIELDS;
    bmh.bV5AlphaMask = 0x0000;
    bmh.bV5RedMask   = 0xF800;
    bmh.bV5GreenMask = 0x07E0;
    bmh.bV5BlueMask  = 0x001F;
    bmh.bV5CSType = LCS_WINDOWS_COLOR_SPACE;
    bmh.bV5Intent = LCS_GM_BUSINESS;
    window->bmi.bmiHeader = bmh;
    window->bmi.bmiColors[0] = 0x001F;
    window->bmi.bmiColors[1] = 0x07E0;
    window->bmi.bmiColors[2] = 0xF800;
    window->bitmap = CreateDIBSection(hdc, (BITMAPINFO *)&window->bmi, DIB_RGB_COLORS, (void **)&window->bitmapBuffer, 0, 0);

    SelectObject(window->hdcBitmap, window->bitmap);
    ReleaseDC(window->hwnd, hdc);

    ShowWindow(window->hwnd, SW_SHOWNORMAL);
  }

  return (window_t *)window;
}

static int window_destroy(window_t *_window) {
  win_window_t *window = (win_window_t *)_window;

  debug(DEBUG_TRACE, "Windows", "window_destroy()");

  if (window) {
    if (window->hwnd) DestroyWindow(window->hwnd);
    sys_free(window);
  }

  return 0;
}

static int window_render(window_t *_window) {
  win_window_t *window = (win_window_t *)_window;

  debug(DEBUG_TRACE, "Windows", "window_render()");

  if (window) {
    //debug(DEBUG_TRACE, "Windows", "render");
    //InvalidateRect(window->hwnd, NULL, false);
  }

  return 0;
}

static texture_t *window_create_texture(window_t *_window, int width, int height) {
  texture_t *texture;

  debug(DEBUG_TRACE, "Windows", "window_create_texture(%d,%d)", width, height);

  if ((texture = sys_malloc(sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->size = width * height * sizeof(uint16_t);
    texture->buffer = sys_malloc(texture->size);
  }

  debug(DEBUG_TRACE, "Windows", "window_create_texture(%d,%d): %p", width, height, texture);
  return texture;
}

static int window_destroy_texture(window_t *window, texture_t *texture) {
  debug(DEBUG_TRACE, "Windows", "window_destroy_texture");

  if (texture) {
    if (texture->buffer) {
      sys_free(texture->buffer);
    }
    sys_free(texture);
  }

  return 0;
}

static int window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  int i, j, tpitch, tindex;
  uint16_t *buffer;

  debug(DEBUG_TRACE, "Windows", "window_update_texture_rect(%d,%d,%d,%d)", tx, ty, w, h);

  if (texture && src) {
    buffer = (uint16_t *)src;
    tpitch = texture->width;
    tindex = ty * tpitch + tx;
    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        texture->buffer[tindex + j] = buffer[tindex + j];
      }
      tindex += tpitch;
    }
  }

  return 0;
}

static int window_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  debug(DEBUG_TRACE, "Windows", "window_update_texture");

  if (texture && texture->buffer && raw) {
    sys_memcpy(texture->buffer, raw, texture->size);
  }

  return 0;
}

static int window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  win_window_t *window = (win_window_t *)_window;
  int i, j, d, tpitch, wpitch, tindex, windex;
  RECT rect;

  debug(DEBUG_TRACE, "Windows", "window_draw_texture_rect(%d,%d,%d,%d,%d,%d)", tx, ty, w, h, x, y);

  if (texture && window->bitmapBuffer) {
    if (x < 0) {
      tx -= x;
      w += x;
      x = 0;
    }
    if (x+w >= window->width) {
      d = x+w - window->width;
      w -= d;
    }
    if (y < 0) {
      ty -= y;
      h += y;
      y = 0;
    }
    if (y+h >= window->height) {
      d = y+h - window->height;
      h -= d;
    }
    if (w > 0 && h > 0) {
      tpitch = texture->width;
      wpitch = window->width;
      tindex = ty * tpitch + tx;
      windex = (window->height - y - 1) * wpitch + x;
#ifndef _WIN32_WCE
	  GdiFlush();
#endif
      for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
          window->bitmapBuffer[windex + j] = texture->buffer[tindex + j];
        }
        tindex += tpitch;
        windex -= wpitch;
      }
      rect.left = x;
      rect.right = x + w;
      rect.top = y;
      rect.bottom = y + h;
      debug(DEBUG_TRACE, "Windows", "invalidate %d %d %d %d", (int32_t)rect.left, (int32_t)rect.right, (int32_t)rect.top, (int32_t)rect.bottom);
      InvalidateRect(window->hwnd, &rect, false);
    }
  }

  debug(DEBUG_TRACE, "Windows", "window_draw_texture_rect(%d,%d,%d,%d,%d,%d) end", tx, ty, w, h, x, y);

  return 0;
}

static int window_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  debug(DEBUG_TRACE, "Windows", "window_draw_texture(%d,%d)", x, y);

  if (texture) {
    window_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return 0;
}

#ifdef _MSC_VER
int __RTC_CheckEsp(void *arg) {
	return 0;
}
#endif

static int window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
	win_window_t *window = (win_window_t *)_window;
	win_event_t event;
	MSG msg = { NULL, 0, 0, 0, 0, { 0, 0 } };
	int64_t t, dt, endt;
	uint32_t waitUs, us;
	int ev = 0;

	if (wait == 0) {
		while (PeekMessage(&msg, NULL, 0, 0, 0)) {
			if (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	} else if (wait < 0) {
		us = 100000;
		for (; !thread_must_end();) {
			while (PeekMessage(&msg, NULL, 0, 0, 0)) {
				if (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if (window->num_ev) break;
			sys_usleep(us);
		}
	} else {
		waitUs = wait * 1000;
		us = waitUs;
		if (us > 1000) us = 1000;
		endt = sys_get_clock() + waitUs;
		for (;;) {
			while (PeekMessage(&msg, NULL, 0, 0, 0)) {
				if (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if (window->num_ev) break;
			t = sys_get_clock();
			if (t >= endt) break;
			dt = endt - t;
			sys_usleep(dt < us ? (uint32_t)dt : us);
		}
	}

	if (getEvent(window, &event)) {
		ev = event.ev;
		*arg1 = event.arg1;
		*arg2 = event.arg2;
	}

	return ev;
}

static void window_status(window_t *_window, int *x, int *y, int *buttons) {
  win_window_t *window = (win_window_t *)_window;
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static int window_show_cursor(window_t *window, int show) {
  ShowCursor(show ? TRUE : FALSE);
  return 0;
}

static int window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&wp, window, x, y, ms);
}

static int window_drop_file(window_t *_window, void (*callback)(char *filename, void *data), void *data) {
  win_window_t *window = (win_window_t *)_window;

  RegisterDropWindow(window->hwnd, &window->dt, callback, data);

  return 0;
}


static int audio_mixer_init(void) {
  return 0;
}

static int audio_mixer_play(uint8_t *buf, uint32_t len, int volume) {
  char name[256 + L_tmpnam + 1];
  int fd, r = -1;

  if (buf) {
    if (sys_tmpname(name, sizeof(name)) == 0) {
      if ((fd = sys_create(name, SYS_WRITE, 0644)) != -1) {
        sys_write(fd, buf, len);
        sys_close(fd);
        r = playMIDIFile(NULL, name) == 0;
        sys_unlink(name);
      }
    }
  }

  return r;
}

static int audio_mixer_stop(void) {
  return 0;
}

static void audio_destructor(void *p) {
  win_audio_t *audio = (win_audio_t *)p;

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
  win_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_TRACE, "Windows", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(win_audio_t))) != NULL) {
      audio->pcm = pcm;
      audio->channels = channels;
      audio->rate = rate;

      // bsize is the number of bytes per channel in one chunk (same convention as the SDL provider);
      // the buffer holds bsize * channels bytes
      audio->bsize = audio_sample_size(pcm) * rate;
      if (audio->bsize > 4096) {
        audio->bsize = 4096;
      }
      // keep whole frames
      audio->bsize -= audio->bsize % audio_sample_size(pcm);
      audio->buffer = sys_calloc(1, audio->bsize * audio->channels);
      debug(DEBUG_TRACE, "Windows", "buffer size %d", audio->bsize * audio->channels);

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
  win_audio_t *audio;
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
  int32_t s = 0;

  switch (pcm) {
    case PCM_U8:
      s = ((int32_t)buffer[i] - 128) << 24;
      break;
    case PCM_S16:
      s = ((int32_t)((int16_t *)buffer)[i]) << 16;
      break;
    case PCM_S32:
      s = ((int32_t *)buffer)[i];
      break;
  }

  return s;
}

static void audio_put_sample(uint8_t *buffer, int32_t s, int i, int pcm) {
  switch (pcm) {
    case PCM_U8:
      buffer[i] = (uint8_t)((s >> 24) + 128);
      break;
    case PCM_S16:
      ((int16_t *)buffer)[i] = (int16_t)(s >> 16);
      break;
    case PCM_S32:
      ((int32_t *)buffer)[i] = s;
      break;
  }
}

static void audio_get_frame(uint8_t *buffer, int frame, int pcm, int channels, int32_t *left, int32_t *right) {
  if (channels == 2) {
    *left = audio_get_sample(buffer, frame * 2, pcm);
    *right = audio_get_sample(buffer, frame * 2 + 1, pcm);
  } else {
    *left = *right = audio_get_sample(buffer, frame, pcm);
  }
}

static void audio_put_frame(uint8_t *buffer, int frame, int pcm, int channels, int32_t left, int32_t right) {
  if (channels == 2) {
    audio_put_sample(buffer, left, frame * 2, pcm);
    audio_put_sample(buffer, right, frame * 2 + 1, pcm);
  } else {
    audio_put_sample(buffer, (int32_t)(((int64_t)left + (int64_t)right) / 2), frame, pcm);
  }
}

// Converts len bytes of audio in the stream format (audio->pcm, audio->channels, audio->rate)
// to the device format (wave->pcm, wave->channels, wave->rate), writing into wave->obuf[i].
// The output buffer is grown as needed. Returns the number of output bytes.

static uint32_t audio_convert(win_wave_t *wave, int i, win_audio_t *audio, uint32_t len) {
  int32_t l0, r0, l1, r1, l, r;
  uint32_t nin, nout, n, k, k1, size;
  uint32_t inFrameSize, outFrameSize;
  double pos, step, frac;
  uint8_t *obuf;

  inFrameSize = audio_sample_size(audio->pcm) * audio->channels;
  outFrameSize = audio_sample_size(wave->pcm) * wave->channels;
  if (inFrameSize == 0 || outFrameSize == 0) return 0;

  nin = len / inFrameSize;
  if (nin == 0) return 0;

  if (audio->rate == wave->rate) {
    nout = nin;
  } else {
    nout = (uint32_t)(((uint64_t)nin * (uint64_t)wave->rate) / (uint64_t)audio->rate);
    if (nout == 0) nout = 1;
  }

  size = nout * outFrameSize;
  if (wave->obuf[i] == NULL || wave->osize[i] < size) {
    if (wave->obuf[i]) sys_free(wave->obuf[i]);
    wave->obuf[i] = sys_calloc(1, size);
    wave->osize[i] = wave->obuf[i] ? size : 0;
    if (wave->obuf[i] == NULL) return 0;
  }
  obuf = wave->obuf[i];

  if (audio->rate == wave->rate) {
    if (audio->pcm == wave->pcm && audio->channels == wave->channels) {
      sys_memcpy(obuf, audio->buffer, size);
    } else {
      for (n = 0; n < nout; n++) {
        audio_get_frame(audio->buffer, n, audio->pcm, audio->channels, &l, &r);
        audio_put_frame(obuf, n, wave->pcm, wave->channels, l, r);
      }
    }
  } else {
    // linear interpolation resampler
    step = (double)audio->rate / (double)wave->rate;
    for (n = 0; n < nout; n++) {
      pos = n * step;
      k = (uint32_t)pos;
      if (k >= nin) k = nin - 1;
      k1 = (k + 1 < nin) ? k + 1 : k;
      frac = pos - (double)k;
      audio_get_frame(audio->buffer, k, audio->pcm, audio->channels, &l0, &r0);
      audio_get_frame(audio->buffer, k1, audio->pcm, audio->channels, &l1, &r1);
      l = (int32_t)(l0 + (l1 - l0) * frac);
      r = (int32_t)(r0 + (r1 - r0) * frac);
      audio_put_frame(obuf, n, wave->pcm, wave->channels, l, r);
    }
  }

  return size;
}

static int audio_wave_open(win_wave_t *wave) {
  MMRESULT res;

  sys_memset(&wave->fmt, 0, sizeof(WAVEFORMATEX));
  wave->fmt.wFormatTag = WAVE_FORMAT_PCM;
  wave->fmt.nChannels = wave->channels;
  wave->fmt.nSamplesPerSec = wave->rate;
  wave->fmt.wBitsPerSample = audio_sample_size(wave->pcm) * 8;
  wave->fmt.nBlockAlign = (wave->fmt.nChannels * wave->fmt.wBitsPerSample) / 8;
  wave->fmt.nAvgBytesPerSec = wave->fmt.nSamplesPerSec * wave->fmt.nBlockAlign;
  wave->fmt.cbSize = 0;

  if ((res = waveOutOpen(&wave->hwo, WAVE_MAPPER, &wave->fmt, 0, 0, CALLBACK_NULL)) != MMSYSERR_NOERROR) {
    debug(DEBUG_ERROR, "Windows", "waveOutOpen failed: %d", res);
    wave->hwo = NULL;
    return -1;
  }

  debug(DEBUG_INFO, "Windows", "open audio %d,%d,%d", wave->pcm, wave->channels, wave->rate);
  return 0;
}

// waits until the header i (if queued) has finished playing and unprepares it

static int audio_wave_wait(win_wave_t *wave, int i) {
  if (wave->queued[i]) {
    while (!(wave->hdr[i].dwFlags & WHDR_DONE)) {
      if (thread_must_end()) return -1;
      sys_usleep(1000);
    }
    waveOutUnprepareHeader(wave->hwo, &wave->hdr[i], sizeof(WAVEHDR));
    wave->queued[i] = 0;
  }

  return 0;
}

static int audio_wave_write(win_wave_t *wave, int i, uint32_t len) {
  MMRESULT res;

  sys_memset(&wave->hdr[i], 0, sizeof(WAVEHDR));
  wave->hdr[i].lpData = (LPSTR)wave->obuf[i];
  wave->hdr[i].dwBufferLength = len;

  if ((res = waveOutPrepareHeader(wave->hwo, &wave->hdr[i], sizeof(WAVEHDR))) != MMSYSERR_NOERROR) {
    debug(DEBUG_ERROR, "Windows", "waveOutPrepareHeader failed (%d)", res);
    return -1;
  }

  if ((res = waveOutWrite(wave->hwo, &wave->hdr[i], sizeof(WAVEHDR))) != MMSYSERR_NOERROR) {
    debug(DEBUG_ERROR, "Windows", "waveOutWrite failed (%d)", res);
    waveOutUnprepareHeader(wave->hwo, &wave->hdr[i], sizeof(WAVEHDR));
    return -1;
  }

  wave->queued[i] = 1;
  return 0;
}

static void audio_wave_close(win_wave_t *wave) {
  int i;

  if (wave->hwo) {
    debug(DEBUG_INFO, "Windows", "close audio");
    waveOutReset(wave->hwo);
    for (i = 0; i < WAVE_NUM_BUFFERS; i++) {
      if (wave->queued[i]) {
        waveOutUnprepareHeader(wave->hwo, &wave->hdr[i], sizeof(WAVEHDR));
        wave->queued[i] = 0;
      }
    }
    waveOutClose(wave->hwo);
    wave->hwo = NULL;
  }

  for (i = 0; i < WAVE_NUM_BUFFERS; i++) {
    if (wave->obuf[i]) {
      sys_free(wave->obuf[i]);
      wave->obuf[i] = NULL;
      wave->osize[i] = 0;
    }
  }
}

static int audio_action(void *_arg) {
  win_audio_arg_t *arg = (win_audio_arg_t *)_arg;
  win_audio_t *audio;
  win_wave_t wave;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr, wait, olen;
  int len1, len2, r, i;

  debug(DEBUG_INFO, "Windows", "audio thread starting");
  sys_memset(&wave, 0, sizeof(wave));
  wave.pcm = arg->pcm;
  wave.channels = arg->channels;
  wave.rate = arg->rate;
  wait = 2000;
  ptr = 0;

  for (; !thread_must_end();) {
    debug(DEBUG_TRACE, "Windows", "receiving msg wait=%u ...", wait);
    if ((r = thread_server_read_timeout(wait, &msg, &msglen)) == -1) {
      debug(DEBUG_ERROR, "Windows", "error receiving msg");
      break;
    }

    if (r == 1) {
      if (msg) {
        wait = 2000;
        if (msglen == 4) {
          ptr = *((uint32_t *)msg);
          debug(DEBUG_TRACE, "Windows", "received ptr %d", ptr);
          wait = 0;
        } else {
          debug(DEBUG_ERROR, "Windows", "received invalid length %u", msglen);
        }
        sys_free(msg);
      } else {
        debug(DEBUG_ERROR, "Windows", "received null msg");
      }
      continue;
    }

    if (ptr == 0) {
      // nothing is playing; the device stays open until the thread exits
      wait = 2000;
      continue;
    }

    if ((audio = ptr_lock(ptr, TAG_AUDIO)) == NULL) {
      debug(DEBUG_TRACE, "Windows", "ptr %d deleted", ptr);
      ptr = 0;
      wait = 2000;
      continue;
    }

    if (wave.hwo == NULL && audio_wave_open(&wave) == -1) {
      ptr_unlock(ptr, TAG_AUDIO);
      ptr = 0;
      wait = 2000;
      continue;
    }

    len1 = audio->bsize * audio->channels;
    len2 = audio->getaudio ? audio->getaudio(audio->buffer, len1, audio->data) : -1;
    if (len2 > len1) len2 = len1;

    if (len2 > 0) {
      debug(DEBUG_TRACE, "Windows", "got audio len=%d bytes", len2);
      i = wave.next;

      // wait for the oldest buffer to finish playing before reusing it
      if (audio_wave_wait(&wave, i) == -1) {
        ptr_unlock(ptr, TAG_AUDIO);
        break;
      }

      if ((olen = audio_convert(&wave, i, audio, (uint32_t)len2)) > 0) {
        debug(DEBUG_TRACE, "Windows", "waveOutWrite sending %u bytes", olen);
        if (audio_wave_write(&wave, i, olen) == 0) {
          wave.next = (i + 1) % WAVE_NUM_BUFFERS;
        }
      }
      wait = 0;
    }
    ptr_unlock(ptr, TAG_AUDIO);

    if (len2 <= 0) {
      debug(DEBUG_TRACE, "Windows", "ptr %d ended", ptr);
      ptr = 0;
      wait = 2000;
    }
  }

  audio_wave_close(&wave);

  sys_free(arg);
  debug(DEBUG_INFO, "Windows", "audio thread exiting");

  return 0;
}

static int audio_init(int pcm, int channels, int rate) {
  win_audio_arg_t *arg;

  if (audio_sample_size(pcm) == 0 || (channels != 1 && channels != 2) || rate <= 0) {
    debug(DEBUG_ERROR, "Windows", "audio_init invalid format %d,%d,%d", pcm, channels, rate);
    return -1;
  }

  if ((arg = sys_calloc(1, sizeof(win_audio_arg_t))) == NULL) return -1;
  arg->pcm = pcm;
  arg->channels = channels;
  arg->rate = rate;
  return thread_begin("AUDIO", audio_action, arg);
}

static int audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

static int custom_load(int pe) {
  script_ref_t obj;
  int r = -1;

  if ((obj = script_create_object(pe)) != -1) {
    debug(DEBUG_INFO, "Windows", "registering provider %s", WINDOW_PROVIDER);
    script_set_pointer(pe, WINDOW_PROVIDER, &wp);
  
    debug(DEBUG_INFO, "Windows", "registering provider %s", AUDIO_PROVIDER);
    script_set_pointer(pe, AUDIO_PROVIDER, &ap);

    script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
    script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
    script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
    script_add_iconst(pe, obj, "hdepth", 16);
  
    r = script_push_object(pe, obj);
    script_remove_ref(pe, obj);
  }
    
  return r;
}       
  
static void pit_callback(int pe, void *data) {
  script_arg_t value;

  value.type = SCRIPT_ARG_FUNCTION;
  value.value.r = script_create_function(pe, custom_load);
  script_global_set(pe, "custom_load", &value);
} 

#ifdef _WIN32_WCE
#ifdef CE_EMU
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpWCmdLine, int nShowCmd) {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpWCmdLine, int nShowCmd) {
#endif
  char lpCmdLine[MAX_PATH];
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif
  WNDCLASS wc;
  char *argv[MAX_ARGS];
  char *cmdline;
  int i, s, argc;

#ifdef _WIN32_WCE
#if __STDC_WANT_SECURE_LIB__
  size_t cmdlen = 0;

  wcstombs_s(&cmdlen, lpCmdLine, MAX_PATH, lpWCmdLine, wcslen(lpWCmdLine));
#else
  wcstombs(lpCmdLine, lpWCmdLine, MAX_PATH);
#endif

  mid_init(NULL);
#else
  OleInitialize(0);
#endif

  sys_memset(&wc, 0, sizeof(WNDCLASS));
  wc.lpfnWndProc = (WNDPROC)WindowProc;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);

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
  wp.average = window_average;
  wp.drop_file = window_drop_file;
  wp.data = hInstance;

  sys_memset(&ap, 0, sizeof(audio_provider_t));
  ap.init = audio_init;
  ap.finish = audio_finish;
  ap.create = audio_create;
  ap.start = audio_start;
  ap.destroy = audio_destroy;
  ap.mixer_init = audio_mixer_init;
  ap.mixer_play = audio_mixer_play;
  ap.mixer_stop = audio_mixer_stop;
  ap.data = hInstance;

  cmdline = sys_strdup(lpCmdLine);
  argc = 0;
  argv[argc++] = "pit";
  s = 0;
  for (i = 0; cmdline[i] && argc < MAX_ARGS; i++) {
    switch (s) {
      case 0:
        if (cmdline[i] != ' ') {
          argv[argc++] = &cmdline[i];
          s = 1;
        }
        break;
      case 1:
        if (cmdline[i] == ' ') {
          cmdline[i] = 0;
          s = 0;
        }
        break;
    }
  }

  pit_main(argc, argv, pit_callback, NULL);
  sys_free(cmdline);

#ifndef _WIN32_WCE
  OleUninitialize();
#endif

  return 0;
}
