/*
 * pumpkin.c
 * PumpkinOS (PalmOS re-implementation) frontend for gnuboy.
 *
 * Provides the sys/ backend (video, input, timing, paths), the PalmOS
 * application entry point (PilotMain), a ROM chooser form and the main
 * emulator form with on-screen Game Boy buttons:
 *
 *   +-----------------------------------------------+
 *   | title bar                                     |
 *   |   D-pad   |     Game Boy screen     |  A  B   |
 *   |  (3x3)    |   (integer scaled)      | START   |
 *   |           |                         | SELECT  |
 *   +-----------------------------------------------+
 *
 * The buttons are driven by pen down/move/up events (sliding from one
 * button to another releases the old one and presses the new one) and
 * post K_JOY* events into the gnuboy event queue, so they go through the
 * normal "bind" mechanism. The keyboard is polled with pumpkin_status().
 *
 * Video: gnuboy renders a 160x144 RGB565 frame into a private buffer;
 * vid_end() pixel-scales it into the window bitmap and flushes the
 * rectangle with pumpkin_screen_dirty(), like the Doom and VBA-M ports.
 *
 * Licensed under the GPLv2, or later.
 */

#include <PalmOS.h>
#include <VFSMgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <setjmp.h>

#ifdef ESP_PLATFORM
#include "../../libpit/sys.h"
#include "../../libpit/thread.h"
#include "../../libpumpkin/pumpkin.h"
#include "../../libpit/debug.h"

#include "defs.h"
#include "fb.h"
#include "input.h"
#include "rc.h"
#include "rckeys.h"'
#include "emu.h"
#include "loader.h"
#include "sys.h"
#else
#include "sys.h"
#include "thread.h"
#include "pumpkin.h"
#include "debug.h"

#include "../../defs.h"
#include "../../fb.h"
#include "../../input.h"
#include "../../rc.h"
#include "../../rckeys.h"
#include "../../emu.h"
#include "../../loader.h"
#include "../../sys.h"
#endif

#include "pumpkin-glue.h"
#include "resource.h"

#define TAG "GNUBOY"

#define ROM_DIR   "/PALM/Programs/GnuBoy"
#define SAVE_DIR  ROM_DIR "/saves"
#define MAX_ROMS  256
#define MAX_PATH  512

#define GB_W 160
#define GB_H 144

/* minimum width of the side panels holding the controls (pixels) */
#define MIN_SIDE 64
/* minimum height of the bottom control area in the portrait layout */
#define MIN_BOTTOM 96

extern void init_exports(void);

/* ------------------------------------------------------------------ */
/* state                                                              */
/* ------------------------------------------------------------------ */

struct fb fb;

rcvar_t vid_exports[] = { RCV_END };
rcvar_t joy_exports[] = { RCV_END };

static UInt16 volref;
static UInt32 screenWidth, screenHeight;   /* window size in pixels */
static int coordMul;                        /* pixels per standard coordinate */
static int titleH;                          /* title bar height in pixels */

static char romDirHost[MAX_PATH];
static char saveDirHost[MAX_PATH];
static char *romItems[MAX_ROMS];
static int numRoms;
static int romIndex;

static Boolean ready;           /* MainForm is open and may be drawn into */
static Boolean mainFormOpen;
static Boolean quitApp;
static Boolean appStopped;
static Boolean backToChooser;
static Boolean doReset, doSaveState, doLoadState;

static char title[64];

static jmp_buf dieJmp;
static int dieArmed;

/* video */
static uint16_t *gbfb;          /* 160x144 native RGB565 */
static int scale;               /* integer blit scale */
static int screenX0, screenY0;  /* position of the Game Boy screen (pixels) */
static int fmtProbed;
static int swap16;              /* 16 bpp window bitmap stores big-endian pixels */
static int idxR, idxG, idxB, idxA; /* byte offsets within a 32 bpp pixel */

/* keyboard */
static uint32_t prevKeyMask;
static uint64_t prevExtKeys[2];

/* on-screen buttons */
enum { BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_START, BTN_SELECT, NUM_BTNS };

typedef struct {
	int x, y, w, h;
	int round;          /* corner diameter for WinPaintRectangle */
	const char *label;
	int code;           /* gnuboy key code posted for this button */
} button_t;

static button_t buttons[NUM_BTNS] = {
	{ 0, 0, 0, 0, 0, NULL,     K_JOYUP    },
	{ 0, 0, 0, 0, 0, NULL,     K_JOYDOWN  },
	{ 0, 0, 0, 0, 0, NULL,     K_JOYLEFT  },
	{ 0, 0, 0, 0, 0, NULL,     K_JOYRIGHT },
	{ 0, 0, 0, 0, 0, "A",      K_JOY1     },
	{ 0, 0, 0, 0, 0, "B",      K_JOY0     },
	{ 0, 0, 0, 0, 0, "START",  K_JOY3     },
	{ 0, 0, 0, 0, 0, "SELECT", K_JOY2     },
};

static struct { int x, y, cell; } dpad;
static uint8_t btnDown[NUM_BTNS];
static uint32_t penMask;
static Boolean penActive;

static const RGBColorType black   = { 0, 0x00, 0x00, 0x00 };
static const RGBColorType white   = { 0, 0xFF, 0xFF, 0xFF };
static const RGBColorType dpadUp  = { 0, 0x50, 0x50, 0x58 };
static const RGBColorType dpadDn  = { 0, 0x90, 0x90, 0xA0 };
static const RGBColorType abUp    = { 0, 0xA0, 0x20, 0x60 };
static const RGBColorType abDn    = { 0, 0xE0, 0x60, 0xA0 };
static const RGBColorType pillUp  = { 0, 0x70, 0x70, 0x78 };
static const RGBColorType pillDn  = { 0, 0xB0, 0xB0, 0xB8 };

/* ------------------------------------------------------------------ */
/* helpers                                                            */
/* ------------------------------------------------------------------ */

static int imin(int a, int b) { return a < b ? a : b; }
static int imax(int a, int b) { return a > b ? a : b; }

static void post_key(int code, int press)
{
	event_t ev;

	if (!code) return;
	ev.type = press ? EV_PRESS : EV_RELEASE;
	ev.code = code;
	ev.dx = ev.dy = ev.x = ev.y = 0;
	ev_postevent(&ev);
}

static Boolean validwindow(void)
{
	FormType *frm = FrmGetActiveForm();
	return frm && FrmGetWindowHandle(frm) == WinGetActiveWindow();
}

static UInt16 draw_begin(void)
{
	UInt16 prev = WinGetCoordinateSystem();
	if (coordMul == 2) WinSetCoordinateSystem(kCoordinatesDouble);
	return prev;
}

static void draw_end(UInt16 prev)
{
	WinSetCoordinateSystem(prev);
}

static void show_error(const char *msg)
{
	debug(DEBUG_ERROR, TAG, "%s", msg);
	FrmCustomAlert(ErrorAlert, msg, "", "");
}

/* ------------------------------------------------------------------ */
/* layout and drawing of the on-screen controls                       */
/* ------------------------------------------------------------------ */

static void set_rect(int i, int x, int y, int w, int h, int round)
{
	buttons[i].x = x;
	buttons[i].y = y;
	buttons[i].w = w;
	buttons[i].h = h;
	buttons[i].round = round;
}

static void layout_dpad(int x, int y, int cell)
{
	dpad.x = x;
	dpad.y = y;
	dpad.cell = cell;
	set_rect(BTN_UP,    x + cell,     y,            cell, cell, 0);
	set_rect(BTN_DOWN,  x + cell,     y + 2 * cell, cell, cell, 0);
	set_rect(BTN_LEFT,  x,            y + cell,     cell, cell, 0);
	set_rect(BTN_RIGHT, x + 2 * cell, y + cell,     cell, cell, 0);
}

static void layout(void)
{
	int w = (int)screenWidth, h = (int)screenHeight - titleH;
	int s, side, cell, d, pw, ph, block, y0, rx, by0, bh;

	/* largest integer scale that still leaves room for side panels */
	for (s = 8; s > 1; s--)
		if (GB_W * s + 2 * MIN_SIDE <= w && GB_H * s <= h) break;
	scale = s;
	side = (w - GB_W * scale) / 2;

	if (side >= MIN_SIDE) {
		/* landscape: controls left and right of the screen */
		screenX0 = side;
		screenY0 = titleH + (h - GB_H * scale) / 2;

		cell = imin(imin(side / 3, h / 3), 28 * coordMul);
		layout_dpad((side - 3 * cell) / 2, titleH + (h - 3 * cell) / 2, cell);

		rx = w - side;
		ph = imin(14 * coordMul, h / 8);
		pw = imin(side - 8 * coordMul, 55 * coordMul);
		d = imin(side / 2 - 4 * coordMul, (h - 2 * ph - 12 * coordMul) / 2);
		d = imin(d, 36 * coordMul);
		d = imax(d, 12 * coordMul);
		block = d + d / 2 + 6 * coordMul + 2 * ph + 4 * coordMul;
		y0 = titleH + imax((h - block) / 2, 0);

		set_rect(BTN_A, rx + side / 2 + coordMul, y0, d, d, d);
		set_rect(BTN_B, rx + side / 2 - d - coordMul, y0 + d / 2, d, d, d);
		set_rect(BTN_START, rx + (side - pw) / 2, y0 + d + d / 2 + 6 * coordMul, pw, ph, ph);
		set_rect(BTN_SELECT, rx + (side - pw) / 2, buttons[BTN_START].y + ph + 4 * coordMul, pw, ph, ph);
	} else {
		/* portrait / narrow: screen on top, controls below */
		for (s = 8; s > 1; s--)
			if (GB_W * s <= w && GB_H * s + MIN_BOTTOM <= h) break;
		scale = s;
		screenX0 = (w - GB_W * scale) / 2;
		screenY0 = titleH;
		by0 = titleH + GB_H * scale;
		bh = h - GB_H * scale;

		ph = imin(12 * coordMul, bh / 5);
		cell = imin(imin(w / 6, (bh - ph - 6 * coordMul) / 3), 28 * coordMul);
		cell = imax(cell, 8 * coordMul);
		layout_dpad(2 * coordMul, by0 + 2 * coordMul, cell);

		d = imin(imin(w / 4 - 4 * coordMul, (bh - ph - 8 * coordMul) / 2), 36 * coordMul);
		d = imax(d, 12 * coordMul);
		set_rect(BTN_A, w - d - 3 * coordMul, by0 + 2 * coordMul, d, d, d);
		set_rect(BTN_B, w - 2 * d - 5 * coordMul, by0 + 2 * coordMul + d / 2, d, d, d);

		pw = imin(50 * coordMul, w / 2 - 6 * coordMul);
		set_rect(BTN_SELECT, w / 2 - pw - 2 * coordMul, by0 + bh - ph - 2 * coordMul, pw, ph, ph);
		set_rect(BTN_START, w / 2 + 2 * coordMul, by0 + bh - ph - 2 * coordMul, pw, ph, ph);
	}

	fmtProbed = 0;
}

static void paint_triangle(int i, int cx, int cy, int s)
{
	switch (i) {
	case BTN_UP:
		WinPaintLine(cx, cy - s, cx - s, cy + s / 2);
		WinPaintLine(cx - s, cy + s / 2, cx + s, cy + s / 2);
		WinPaintLine(cx + s, cy + s / 2, cx, cy - s);
		break;
	case BTN_DOWN:
		WinPaintLine(cx, cy + s, cx - s, cy - s / 2);
		WinPaintLine(cx - s, cy - s / 2, cx + s, cy - s / 2);
		WinPaintLine(cx + s, cy - s / 2, cx, cy + s);
		break;
	case BTN_LEFT:
		WinPaintLine(cx - s, cy, cx + s / 2, cy - s);
		WinPaintLine(cx + s / 2, cy - s, cx + s / 2, cy + s);
		WinPaintLine(cx + s / 2, cy + s, cx - s, cy);
		break;
	case BTN_RIGHT:
		WinPaintLine(cx + s, cy, cx - s / 2, cy - s);
		WinPaintLine(cx - s / 2, cy - s, cx - s / 2, cy + s);
		WinPaintLine(cx - s / 2, cy + s, cx + s, cy);
		break;
	}
}

/* draw one control; assumes the double coordinate system is active */
static void paint_button(int i)
{
	button_t *b = &buttons[i];
	RectangleType r;
	const RGBColorType *fill;
	int tw, th, len, inset;

	if (b->w <= 0 || b->h <= 0) return;

	if (i <= BTN_RIGHT) {
		fill = btnDown[i] ? &dpadDn : &dpadUp;
		inset = coordMul;
		RctSetRectangle(&r, b->x + inset, b->y + inset, b->w - 2 * inset, b->h - 2 * inset);
		WinSetForeColorRGB(fill, NULL);
		WinPaintRectangle(&r, 0);
		WinSetForeColorRGB(&white, NULL);
		paint_triangle(i, b->x + b->w / 2, b->y + b->h / 2, imax(b->w / 5, 2));
		return;
	}

	if (i == BTN_A || i == BTN_B) fill = btnDown[i] ? &abDn : &abUp;
	else fill = btnDown[i] ? &pillDn : &pillUp;

	RctSetRectangle(&r, b->x, b->y, b->w, b->h);
	WinSetForeColorRGB(fill, NULL);
	WinPaintRectangle(&r, b->round);
	WinSetForeColorRGB(&black, NULL);
	WinPaintRoundedRectangleFrame(&r, coordMul, b->round / 2, 0);

	if (b->label) {
		FntSetFont((i == BTN_A || i == BTN_B) ? largeBoldFont : boldFont);
		len = StrLen(b->label);
		tw = FntCharsWidth(b->label, len);
		th = FntCharHeight();
		if (tw <= b->w - 2 * coordMul) {
			WinSetTextColorRGB(&white, NULL);
			WinSetBackColorRGB(fill, NULL);
			WinPaintChars(b->label, len, b->x + (b->w - tw) / 2, b->y + (b->h - th) / 2);
		}
	}
}

static void draw_button(int i)
{
	UInt16 prev;

	if (!ready || !validwindow()) return;
	prev = draw_begin();
	paint_button(i);
	draw_end(prev);
}

static void draw_controls(void)
{
	RectangleType r;
	UInt16 prev;
	int i;

	if (!ready) return;
	prev = draw_begin();

	/* d-pad centre */
	RctSetRectangle(&r, dpad.x + dpad.cell + coordMul, dpad.y + dpad.cell + coordMul,
			dpad.cell - 2 * coordMul, dpad.cell - 2 * coordMul);
	WinSetForeColorRGB(&dpadUp, NULL);
	WinPaintRectangle(&r, 0);

	for (i = 0; i < NUM_BTNS; i++) paint_button(i);

	/* frame around the Game Boy screen */
	RctSetRectangle(&r, screenX0 - coordMul, screenY0 - coordMul,
			GB_W * scale + 2 * coordMul, GB_H * scale + 2 * coordMul);
	WinSetForeColorRGB(&black, NULL);
	WinPaintRectangleFrame(simpleFrame, &r);

	draw_end(prev);
}

/* ------------------------------------------------------------------ */
/* pen input                                                          */
/* ------------------------------------------------------------------ */

static uint32_t hit_mask(int x, int y)
{
	uint32_t m = 0;
	int i, col, row;
	button_t *b;

	if (dpad.cell > 0 && x >= dpad.x && x < dpad.x + 3 * dpad.cell &&
	    y >= dpad.y && y < dpad.y + 3 * dpad.cell) {
		col = (x - dpad.x) / dpad.cell;
		row = (y - dpad.y) / dpad.cell;
		if (row == 0) m |= 1u << BTN_UP;
		else if (row == 2) m |= 1u << BTN_DOWN;
		if (col == 0) m |= 1u << BTN_LEFT;
		else if (col == 2) m |= 1u << BTN_RIGHT;
		return m;
	}

	for (i = BTN_A; i < NUM_BTNS; i++) {
		b = &buttons[i];
		if (x >= b->x && x < b->x + b->w && y >= b->y && y < b->y + b->h)
			return 1u << i;
	}

	return 0;
}

static void apply_pen_mask(uint32_t m)
{
	uint32_t diff = penMask ^ m;
	int i;

	for (i = 0; i < NUM_BTNS; i++) {
		if (diff & (1u << i)) {
			btnDown[i] = (m >> i) & 1;
			post_key(buttons[i].code, btnDown[i]);
			draw_button(i);
		}
	}
	penMask = m;
}

static void release_all(void)
{
	int i;

	if (penMask) apply_pen_mask(0);
	penActive = false;

	for (i = 0; i < 4; i++) {
		static const int codes[4] = { K_UP, K_DOWN, K_LEFT, K_RIGHT };
		static const uint32_t bits[4] = { keyBitPageUp, keyBitPageDown, keyBitLeft, keyBitRight };
		if (prevKeyMask & bits[i]) post_key(codes[i], 0);
	}
	prevKeyMask = 0;
	prevExtKeys[0] = prevExtKeys[1] = 0;

	/* nothing consumes gnuboy events while no ROM is running */
	{
		event_t ev;
		while (ev_getevent(&ev));
	}
}

static Boolean handle_pen(EventType *event)
{
	uint32_t m;
	int x, y;

	if (!mainFormOpen) return false;

	x = event->screenX * coordMul;
	y = event->screenY * coordMul;

	switch (event->eType) {
	case penDownEvent:
		m = hit_mask(x, y);
		if (!m) return false;
		penActive = true;
		apply_pen_mask(m);
		return true;
	case penMoveEvent:
		if (!penActive) return false;
		apply_pen_mask(hit_mask(x, y));
		return true;
	case penUpEvent:
		if (!penActive) return false;
		penActive = false;
		apply_pen_mask(0);
		return true;
	default:
		return false;
	}
}

/* ------------------------------------------------------------------ */
/* keyboard input (polled)                                            */
/* ------------------------------------------------------------------ */

static int map_ascii(int c)
{
	if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
	if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) return c;

	switch (c) {
	case 13: return K_ENTER;
	case 27: return K_ESC;
	case 32: return K_SPACE;
	case 9:  return K_TAB;
	case 8:  return K_BS;
	case 127: return K_DEL;
	case '-': return K_MINUS;
	case '=': return K_EQUALS;
	default: return 0;
	}
}

static void poll_ext_keys(uint64_t old, uint64_t new, int offset)
{
	uint64_t diff = old ^ new;
	int i;

	for (i = 0; diff && i < 64; i++, diff >>= 1, new >>= 1) {
		if (diff & 1) post_key(map_ascii(offset + i), (int)(new & 1));
	}
}

static void poll_keys(void)
{
	uint32_t keyMask, modMask, diff;
	uint64_t ext[2];

	if (!mainFormOpen) return;

	pumpkin_status(NULL, NULL, &keyMask, &modMask, NULL, ext);

	diff = prevKeyMask ^ keyMask;
	if (diff & keyBitPageUp)   post_key(K_UP,    (keyMask & keyBitPageUp) != 0);
	if (diff & keyBitPageDown) post_key(K_DOWN,  (keyMask & keyBitPageDown) != 0);
	if (diff & keyBitLeft)     post_key(K_LEFT,  (keyMask & keyBitLeft) != 0);
	if (diff & keyBitRight)    post_key(K_RIGHT, (keyMask & keyBitRight) != 0);
	prevKeyMask = keyMask;

	poll_ext_keys(prevExtKeys[0], ext[0], 0);
	poll_ext_keys(prevExtKeys[1], ext[1], 64);
	prevExtKeys[0] = ext[0];
	prevExtKeys[1] = ext[1];
}

/* ------------------------------------------------------------------ */
/* forms                                                              */
/* ------------------------------------------------------------------ */

static void resize(FormType *frm)
{
	UInt32 sw, sh;
	RectangleType rect;

	WinScreenMode(winScreenModeGet, &sw, &sh, NULL, NULL);
	RctSetRectangle(&rect, 0, 0, sw, sh);
	WinSetBounds(FrmGetWindowHandle(frm), &rect);
}

static void menu_event(UInt16 id)
{
	switch (id) {
	case menuOpen:
		backToChooser = true;
		emu_pause(1);
		break;
	case menuReset:
		doReset = true;
		break;
	case menuSaveState:
		doSaveState = true;
		break;
	case menuLoadState:
		doLoadState = true;
		break;
	case menuAbout:
		AbtShowAboutPumpkin(pumpkin_get_app_creator());
		break;
	case menuQuit:
		gnuboy_request_quit();
		break;
	}
}

static Boolean MainFormHandleEvent(EventType *event)
{
	FormType *frm;
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		frm = FrmGetActiveForm();
		resize(frm);
		FrmSetTitle(frm, title);
		FrmDrawForm(frm);
		layout();
		mainFormOpen = true;
		ready = true;
		draw_controls();
		handled = true;
		break;
	case frmUpdateEvent:
		frm = FrmGetActiveForm();
		FrmDrawForm(frm);
		draw_controls();
		handled = true;
		break;
	case frmCloseEvent:
		mainFormOpen = false;
		ready = false;
		break;
	case penDownEvent:
	case penMoveEvent:
	case penUpEvent:
		handled = handle_pen(event);
		break;
	case menuEvent:
		menu_event(event->data.menu.itemID);
		handled = true;
		break;
	default:
		break;
	}

	return handled;
}

static Boolean ChooseFormHandleEvent(EventType *event)
{
	FormType *frm;
	ListType *lst;
	RectangleType rect, brect;
	UInt32 sw, sh;
	UInt16 index, bindex;
	Int16 n, lh;
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		frm = FrmGetActiveForm();
		resize(frm);
		WinScreenMode(winScreenModeGet, &sw, &sh, NULL, NULL);

		/* stretch the list to the window and put the button below it */
		index = FrmGetObjectIndex(frm, romList);
		lst = (ListType *)FrmGetObjectPtr(frm, index);
		FntSetFont(stdFont);
		lh = FntLineHeight();
		FrmGetObjectBounds(frm, index, &rect);
		rect.extent.x = (Coord)sw - 16;
		n = ((Coord)sh - rect.topLeft.y - 14 - 8) / lh;
		if (n < 1) n = 1;
		if (n > 20) n = 20;
		rect.extent.y = n * lh;
		FrmSetObjectBounds(frm, index, &rect);
		LstSetHeight(lst, n);

		bindex = FrmGetObjectIndex(frm, runBtn);
		FrmGetObjectBounds(frm, bindex, &brect);
		brect.topLeft.y = rect.topLeft.y + rect.extent.y + 4;
		FrmSetObjectBounds(frm, bindex, &brect);

		bindex = FrmGetObjectIndex(frm, hintLbl);
		FrmGetObjectBounds(frm, bindex, &brect);
		brect.topLeft.y = rect.topLeft.y + rect.extent.y + 6;
		FrmSetObjectBounds(frm, bindex, &brect);

		if (numRoms > 0) {
			LstSetListChoices(lst, romItems, numRoms);
			LstSetSelection(lst, 0);
		} else {
			LstSetListChoices(lst, NULL, 0);
		}
		FrmDrawForm(frm);
		handled = true;
		break;
	case ctlSelectEvent:
		if (event->data.ctlSelect.controlID == runBtn && numRoms > 0) {
			frm = FrmGetActiveForm();
			index = FrmGetObjectIndex(frm, romList);
			lst = (ListType *)FrmGetObjectPtr(frm, index);
			romIndex = LstGetSelection(lst);
			handled = true;
		}
		break;
	case menuEvent:
		menu_event(event->data.menu.itemID);
		handled = true;
		break;
	default:
		break;
	}

	return handled;
}

static Boolean ApplicationHandleEvent(EventType *event)
{
	FormType *frm;
	UInt16 form;
	Boolean handled = false;

	if (event->eType == frmLoadEvent) {
		form = event->data.frmLoad.formID;
		frm = FrmInitForm(form);
		FrmSetActiveForm(frm);
		switch (form) {
		case MainForm:
			FrmSetEventHandler(frm, MainFormHandleEvent);
			break;
		case ChooseForm:
			FrmSetEventHandler(frm, ChooseFormHandleEvent);
			break;
		}
		handled = true;
	}

	return handled;
}

/* pump one event and drain the task mailbox; us is the maximum wait */
int gnuboy_pump_events(uint32_t us)
{
	EventType event;
	Err err;
	unsigned char *buf;
	unsigned int len;

	if (thread_must_end()) {
		appStopped = true;
		emu_pause(1);
		return 0;
	}

	EvtGetEventUs(&event, us);
	if (!SysHandleEvent(&event)) {
		if (!MenuHandleEvent(NULL, &event, &err)) {
			if (!ApplicationHandleEvent(&event)) {
				FrmDispatchEvent(&event);
			}
		}
	}

	if (thread_server_read_timeout(0, &buf, &len) == -1) {
		appStopped = true;
	} else if (buf) {
		sys_free(buf);
	}

	if (event.eType == appStopEvent) appStopped = true;

	if (quitApp || appStopped || backToChooser) {
		emu_pause(1);
		return 0;
	}

	return 1;
}

void gnuboy_request_quit(void)
{
	quitApp = true;
	emu_pause(1);
}

/* ------------------------------------------------------------------ */
/* gnuboy sys/ backend: events                                        */
/* ------------------------------------------------------------------ */

void ev_poll(int wait)
{
	gnuboy_pump_events(wait ? 20000 : 0);
	poll_keys();

	/* menu actions are applied here, at a frame boundary */
	if (doReset) {
		doReset = false;
		rc_command("reset");
	}
	if (doSaveState) {
		doSaveState = false;
		rc_command("savestate");
	}
	if (doLoadState) {
		doLoadState = false;
		rc_command("loadstate");
	}
}

void doevents(void)
{
	event_t ev;

	ev_poll(0);
	while (ev_getevent(&ev)) {
		if (ev.type != EV_PRESS && ev.type != EV_RELEASE) continue;
		rc_dokey(ev.code, ev.type != EV_RELEASE);
	}
}

void joy_init(void) {}
void joy_poll(void) {}
void joy_close(void) {}
void kb_init(void) {}
void kb_poll(void) {}
void kb_close(void) {}

/* ------------------------------------------------------------------ */
/* gnuboy sys/ backend: video                                         */
/* ------------------------------------------------------------------ */

/* paint one pixel of the given colour at the screen origin and look at
   the raw bytes to learn the byte order of the window bitmap */
static void probe_pixel(const RGBColorType *c, uint8_t *bits, UInt16 rowBytes, int bpp, uint8_t out[4])
{
	RGBColorType old;
	UInt16 prev;
	uint8_t *p;

	prev = draw_begin();
	WinSetForeColorRGB(c, &old);
	WinPaintPixel(screenX0, screenY0);
	WinSetForeColorRGB(&old, NULL);
	draw_end(prev);

	p = bits + screenY0 * rowBytes + screenX0 * bpp;
	memcpy(out, p, bpp);
}

static void probe_format(uint8_t *bits, UInt16 rowBytes, int depth)
{
	static const RGBColorType red   = { 0, 0xFF, 0x00, 0x00 };
	static const RGBColorType green = { 0, 0x00, 0xFF, 0x00 };
	static const RGBColorType blue  = { 0, 0x00, 0x00, 0xFF };
	uint8_t px[4];
	int i;

	swap16 = 0;
	idxA = 0; idxR = 1; idxG = 2; idxB = 3;

	if (depth == 16) {
		probe_pixel(&red, bits, rowBytes, 2, px);
		/* native little-endian 0xF800 is stored as 00 F8 */
		swap16 = !(px[0] == 0x00 && px[1] == 0xF8);
	} else if (depth == 32) {
		probe_pixel(&red, bits, rowBytes, 4, px);
		for (i = 0; i < 4; i++) if (px[i] == 0xFF) { idxR = i; break; }
		probe_pixel(&green, bits, rowBytes, 4, px);
		for (i = 0; i < 4; i++) if (px[i] == 0xFF) { idxG = i; break; }
		probe_pixel(&blue, bits, rowBytes, 4, px);
		for (i = 0; i < 4; i++) if (px[i] == 0xFF) { idxB = i; break; }
		for (i = 0; i < 4; i++) if (i != idxR && i != idxG && i != idxB) idxA = i;
	}

	debug(DEBUG_INFO, TAG, "window bitmap depth %d swap16 %d rgba idx %d %d %d %d",
		depth, swap16, idxR, idxG, idxB, idxA);
	fmtProbed = 1;
}

/* once per second: frames drawn and audio ring level, for tuning */
static void perf_tick(void)
{
	static int64_t t0;
	static int frames;
	int64_t now = sys_get_clock();

	frames++;
	if (t0 == 0) t0 = now;
	if (now - t0 >= 1000000) {
		debug(DEBUG_INFO, TAG, "perf: fps=%d scale=%d audio queued=%d", frames, scale, gnuboy_audio_queued());
		frames = 0;
		t0 = now;
	}
}

static void blit(void)
{
	WinHandle wh;
	BitmapType *bmp;
	Coord bw, bh;
	UInt16 rowBytes;
	UInt8 depth;
	uint8_t *bits;
	const uint16_t *src;
	uint16_t c, *dst16;
	uint8_t *dst8;
	int x, y, s, k, sw, sh;

	if (!ready || !mainFormOpen || !gbfb || !validwindow()) return;

	wh = WinGetDisplayWindow();
	bmp = WinGetBitmap(wh);
	if (!bmp) return;

	BmpGetDimensions(bmp, &bw, &bh, &rowBytes);
	depth = BmpGetBitDepth(bmp);
	bits = (uint8_t *)BmpGetBits(bmp);
	if (!bits) return;

	sw = GB_W * scale;
	sh = GB_H * scale;
	if (screenX0 + sw > bw || screenY0 + sh > bh) return;

	if (!fmtProbed) probe_format(bits, rowBytes, depth);

	if (depth == 16) {
		for (y = 0; y < GB_H; y++) {
			src = gbfb + y * GB_W;
			for (s = 0; s < scale; s++) {
				dst16 = (uint16_t *)(bits + (screenY0 + y * scale + s) * rowBytes) + screenX0;
				if (scale == 1 && !swap16) {
					memcpy(dst16, src, GB_W * 2);
				} else {
					for (x = 0; x < GB_W; x++) {
						c = src[x];
						if (swap16) c = (uint16_t)((c << 8) | (c >> 8));
						for (k = 0; k < scale; k++) *dst16++ = c;
					}
				}
			}
		}
	} else if (depth == 32) {
		for (y = 0; y < GB_H; y++) {
			src = gbfb + y * GB_W;
			for (s = 0; s < scale; s++) {
				dst8 = bits + (screenY0 + y * scale + s) * rowBytes + screenX0 * 4;
				for (x = 0; x < GB_W; x++) {
					c = src[x];
					for (k = 0; k < scale; k++, dst8 += 4) {
						dst8[idxA] = 0xFF;
						dst8[idxR] = (uint8_t)(((c >> 11) & 0x1F) << 3);
						dst8[idxG] = (uint8_t)(((c >> 5) & 0x3F) << 2);
						dst8[idxB] = (uint8_t)((c & 0x1F) << 3);
					}
				}
			}
		}
	} else {
		return;
	}

	pumpkin_dirty_region_mode(dirtyRegionBegin);
	pumpkin_screen_dirty(wh, screenX0, screenY0, sw, sh);
	pumpkin_dirty_region_mode(dirtyRegionEnd);

	perf_tick();
}

void vid_preinit(void)
{
}

void vid_init(void)
{
	if (!gbfb) gbfb = calloc(GB_W * GB_H, sizeof(uint16_t));

	/* gnuboy renders 1x RGB565 into our buffer; scaling happens in blit() */
	fb.w = GB_W;
	fb.h = GB_H;
	fb.pelsize = 2;
	fb.pitch = GB_W * 2;
	fb.ptr = (byte *)gbfb;
	fb.indexed = 0;
	fb.delegate_scaling = 0;
	fb.yuv = 0;
	fb.cc[0].r = 3; fb.cc[0].l = 11;
	fb.cc[1].r = 2; fb.cc[1].l = 5;
	fb.cc[2].r = 3; fb.cc[2].l = 0;
	fb.cc[3].r = 0; fb.cc[3].l = 0;
	fb.enabled = 1;
	fb.dirty = 0;

	layout();
}

void vid_close(void)
{
	fb.enabled = 0;
	fb.ptr = NULL;
	if (gbfb) {
		free(gbfb);
		gbfb = NULL;
	}
}

void vid_setpal(int i, int r, int g, int b)
{
	/* not indexed */
}

void vid_settitle(char *t)
{
	FormType *frm;

	if (!t) return;
	strncpy(title, t, sizeof(title) - 1);
	title[sizeof(title) - 1] = 0;

	if (mainFormOpen && (frm = FrmGetActiveForm()) != NULL && FrmGetFormId(frm) == MainForm)
		FrmSetTitle(frm, title);
}

void vid_begin(void)
{
}

void vid_end(void)
{
	blit();
}

/* ------------------------------------------------------------------ */
/* gnuboy sys/ backend: timing and paths                              */
/* ------------------------------------------------------------------ */

void *sys_timer(void)
{
	int64_t *t = malloc(sizeof(int64_t));
	*t = sys_get_clock();
	return t;
}

int sys_elapsed(struct timeval *prev)
{
	int64_t *t = (int64_t *)prev;
	int64_t now = sys_get_clock();
	int64_t d = now - *t;

	*t = now;
	if (d > 1000000) d = 1000000;
	return (int)d;
}

/* sleep while keeping the PalmOS UI responsive */
void sys_sleep(int us)
{
	int64_t deadline, now;

	if (us <= 0) return;
	deadline = sys_get_clock() + us;
	for (;;) {
		now = sys_get_clock();
		if (now >= deadline) break;
		if (!gnuboy_pump_events((uint32_t)(deadline - now))) break;
	}
}

void sys_checkdir(char *path, int wr)
{
	/* directories are created through the VFS in ensure_dirs() */
}

void sys_initpath(void)
{
}

void sys_sanitize(char *s)
{
}

/* ------------------------------------------------------------------ */
/* fatal errors                                                       */
/* ------------------------------------------------------------------ */

void die(char *fmt, ...)
{
	char msg[256];
	va_list ap;
	size_t n;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	n = strlen(msg);
	while (n > 0 && (msg[n - 1] == '\n' || msg[n - 1] == '\r')) msg[--n] = 0;

	show_error(msg);

	if (dieArmed) longjmp(dieJmp, 1);

	/* nowhere to return to: stop the emulator */
	quitApp = true;
	emu_pause(1);
}

/* ------------------------------------------------------------------ */
/* ROM handling                                                       */
/* ------------------------------------------------------------------ */

static int allowed_ext(const char *fn)
{
	static const char *exttab[] = { ".gb", ".gbc", ".sgb", ".cgb", ".dmg", ".xz", ".gz", ".zip", NULL };
	const char *e = strrchr(fn, '.');
	int i;

	if (!e) return 0;
	for (i = 0; exttab[i]; i++)
		if (!StrCaselessCompare(exttab[i], e)) return 1;
	return 0;
}

static void free_rom_items(void)
{
	int i;

	for (i = 0; i < numRoms; i++) {
		if (romItems[i]) MemPtrFree(romItems[i]);
		romItems[i] = NULL;
	}
	numRoms = 0;
}

static int compare_items(const void *a, const void *b)
{
	return StrCaselessCompare(*(char *const *)a, *(char *const *)b);
}

static void scan_roms(void)
{
	FileRef fr;
	FileInfoType info;
	UInt32 iterator;
	char name[256];

	free_rom_items();

	if (VFSFileOpen(volref, ROM_DIR, vfsModeRead, &fr) != errNone) return;

	iterator = vfsIteratorStart;
	for (;;) {
		info.nameP = name;
		info.nameBufLen = sizeof(name);
		if (VFSDirEntryEnumerate(fr, &iterator, &info) != errNone) break;
		if (info.attributes & vfsFileAttrDirectory) continue;
		if (!allowed_ext(name)) continue;
		if (numRoms >= MAX_ROMS) break;

		romItems[numRoms] = MemPtrNew(StrLen(name) + 1);
		StrCopy(romItems[numRoms], name);
		numRoms++;
	}
	VFSFileClose(fr);

	qsort(romItems, numRoms, sizeof(char *), compare_items);
}

static void ensure_dir(const char *path)
{
	FileRef fr;

	if (VFSFileOpen(volref, path, vfsModeRead, &fr) == errNone) {
		VFSFileClose(fr);
	} else {
		VFSDirCreate(volref, path);
	}
}

static char volDir[64];

static void find_volume_dir(const char *mount, int n)
{
	static const char *candidates[] = { "/app_card", "", "/card", NULL };
	static const char *marker = ROM_DIR "/.gnuboy-probe";
	FileRef fr;
	FILE *f;
	char path[MAX_PATH];
	int i;

	strcpy(volDir, "/app_card");
	VFSFileCreate(volref, marker); /* may already exist */
	if (VFSFileOpen(volref, marker, vfsModeRead, &fr) != errNone) return;
	VFSFileClose(fr);

	for (i = 0; candidates[i]; i++) {
		snprintf(path, sizeof(path), "%.*s%s%s", n, mount, candidates[i], marker);
		if ((f = fopen(path, "rb")) != NULL) {
			fclose(f);
			strncpy(volDir, candidates[i], sizeof(volDir) - 1);
			break;
		}
	}
	VFSFileDelete(volref, marker);
}

static void ensure_dirs(void)
{
	char *mount;
	size_t n;

	ensure_dir(ROM_DIR);
	ensure_dir(SAVE_DIR);

	/* the gnuboy core uses stdio, so it needs host paths. VFSGetMount()
	   returns the host directory of the whole PumpkinOS VFS; the PalmOS
	   volume lives in a subdirectory of it (normally /app_card). Find it
	   by creating a marker file through the VFS and looking for it. */
	mount = VFSGetMount(volref);
	if (!mount) mount = "";
	n = strlen(mount);
	if (n > 0 && mount[n - 1] == '/') n--;
	find_volume_dir(mount, (int)n);
	snprintf(romDirHost, sizeof(romDirHost), "%.*s%s%s", (int)n, mount, volDir, ROM_DIR);
	snprintf(saveDirHost, sizeof(saveDirHost), "%.*s%s%s", (int)n, mount, volDir, SAVE_DIR);
	debug(DEBUG_INFO, TAG, "rom dir %s", romDirHost);
}

/* replaces the function of the same name in main.c */
int load_rom_and_rc(char *rom)
{
	char *rc, *base, *dot;
	size_t len;

	/* per-ROM rc file: <rom without extension>.rc */
	len = strlen(rom) + 4;
	rc = malloc(len);
	strcpy(rc, rom);
	base = strrchr(rc, '/');
	base = base ? base + 1 : rc;
	dot = strrchr(base, '.');
	if (dot) *dot = 0;
	strcat(rc, ".rc");
	rc_sourcefile(rc);
	free(rc);

	rom = strdup(rom); /* owned (and freed) by the loader */
	if (loader_init(rom)) return -1;
	emu_reset();
	return 0;
}

static const char *defaultconfig[] = {
	"bootrom_dmg \"\"",
	"bootrom_gbc \"\"",
	"bind esc menu",
	"bind up +up",
	"bind down +down",
	"bind left +left",
	"bind right +right",
	"bind x +a",
	"bind z +b",
	"bind d +a",
	"bind s +b",
	"bind enter +start",
	"bind space +select",
	"bind tab +select",
	"bind bs +select",
	"bind joyup +up",
	"bind joydown +down",
	"bind joyleft +left",
	"bind joyright +right",
	"bind joy0 +b",
	"bind joy1 +a",
	"bind joy2 +select",
	"bind joy3 +start",
	"bind 1 \"set saveslot 1\"",
	"bind 2 \"set saveslot 2\"",
	"bind 3 \"set saveslot 3\"",
	"bind 4 \"set saveslot 4\"",
	"bind 5 \"set saveslot 5\"",
	"bind 6 \"set saveslot 6\"",
	"bind 7 \"set saveslot 7\"",
	"bind 8 \"set saveslot 8\"",
	"bind 9 \"set saveslot 9\"",
	"bind 0 \"set saveslot 0\"",
	"bind ins savestate",
	"bind del loadstate",
	"set scale 1",
	"set sound 1",
	"set stereo 1",
	"set samplerate 44100",
	NULL
};

static void init_config(void)
{
	char *v;
	int i;

	init_exports();
	for (i = 0; defaultconfig[i]; i++) rc_command((char *)defaultconfig[i]);

	/* paths may contain spaces, so bypass the rc tokenizer */
	v = romDirHost;
	rc_setvar("romdir", 1, &v);
	rc_setvar("rcpath", 1, &v);
	v = saveDirHost;
	rc_setvar("savedir", 1, &v);

	rc_sourcefile("gnuboy.rc");
}

static Boolean chooser_loop(void)
{
	for (;;) {
		if (!gnuboy_pump_events(20000)) return false;
		if (romIndex >= 0) return true;
	}
}

/* ------------------------------------------------------------------ */
/* PilotMain                                                          */
/* ------------------------------------------------------------------ */

#ifdef ESP32
UInt32 GnuBoyPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#else
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#endif
{
	UInt32 iterator, density;
	char path[MAX_PATH];
	Boolean autoran = false;

	(void)cmdPBP;
	(void)launchFlags;

	if (cmd != sysAppLaunchCmdNormalLaunch) return 0;

	iterator = vfsIteratorStart;
	VFSVolumeEnumerate(&volref, &iterator);

	WinScreenGetAttribute(winScreenWidth, &screenWidth);
	WinScreenGetAttribute(winScreenHeight, &screenHeight);
	WinScreenGetAttribute(winScreenDensity, &density);
	coordMul = (density == kDensityDouble) ? 2 : 1;
	titleH = 15 * coordMul;
	FrmCenterDialogs(true);

	StrCopy(title, "GNUBoy");
	quitApp = false;
	appStopped = false;

	ensure_dirs();
	init_config();
	vid_init();
	pcm_init();
	pcm_pause(1);

	for (;;) {
		scan_roms();
		romIndex = -1;
		ready = false;
		mainFormOpen = false;
		backToChooser = false;
		doReset = doSaveState = doLoadState = false;

		if (numRoms == 1 && !autoran) {
			/* exactly one ROM: skip the chooser */
			romIndex = 0;
		} else {
			FrmGotoForm(ChooseForm);
			if (!chooser_loop()) break;
		}
		autoran = true;
		if (romIndex < 0 || romIndex >= numRoms) break;

		snprintf(path, sizeof(path), "%s/%s", romDirHost, romItems[romIndex]);
		vid_settitle(romItems[romIndex]);

		ready = false;
		FrmGotoForm(MainForm);
		while (!ready) {
			if (!gnuboy_pump_events(10000)) break;
		}
		if (quitApp || appStopped) break;

		dieArmed = 1;
		if (setjmp(dieJmp) == 0) {
			if (load_rom_and_rc(path) == 0) {
				emu_pause(0);
				pcm_pause(0);
				emu_run();
			} else {
				snprintf(path, sizeof(path), "Cannot load %s: %s", romItems[romIndex],
					loader_get_error() ? loader_get_error() : "unknown error");
				show_error(path);
			}
		}
		dieArmed = 0;

		pcm_pause(1);
		loader_unload();
		release_all();

		if (quitApp || appStopped) break;
	}

	pcm_close();
	vid_close();
	free_rom_items();
	FrmCloseAllForms();

	return 0;
}
