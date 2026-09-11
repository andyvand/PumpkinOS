/*
 * sdl2.c
 * sdl2 interfaces -- based on sdl.c
 *
 * (C) 2001 Damian Gryski <dgryski@uwaterloo.ca> (SDL 1 version)
 * (C) 2021 rofl0r (SDL2 port)
 *
 * Licensed under the GPLv2, or later.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "fb.h"
#include "input.h"
#include "rc.h"

extern void sdljoy_process_event(SDL_Event *event);

struct fb fb;

static int fullscreen = 0;
static int use_altenter = 1;
static int vsync;

static int integer_scale = 1;
static SDL_Window *win;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static int vmode[3] = { 0, 0, 32 };

rcvar_t vid_exports[] =
{
	RCV_BOOL("vsync", &vsync, "enforce vsync (slow)"),
	RCV_VECTOR("vmode", &vmode, 3, "video mode: w h bpp"),
	RCV_BOOL("fullscreen", &fullscreen, "start in fullscreen mode"),
	RCV_BOOL("altenter", &use_altenter, "alt-enter can toggle fullscreen"),
	RCV_END
};

/* keymap - mappings of the form { scancode, localcode } - from sdl/keymap.c */
extern int keymap[][2];

static int mapscancode(SDL_Keycode sym)
{
	/* this could be faster:  */
	/*  build keymap as int keymap[256], then ``return keymap[sym]'' */

	int i;
	for (i = 0; keymap[i][0]; i++)
		if (keymap[i][0] == sym)
			return keymap[i][1];
	if (sym >= '0' && sym <= '9')
		return sym;
	if (sym >= 'a' && sym <= 'z')
		return sym;
	return 0;
}



void vid_init()
{
	int flags = 0;
	int scale = rc_getint("scale");
	int pitch = 0;
	int fmt = 0;
	void *pixels = 0;
	SDL_PixelFormat *format;
	SDL_RendererInfo info;

	if (!vmode[0] || !vmode[1])
	{
		if (scale < 1) scale = /*1*/2;
		vmode[0] = 160 * scale;
		vmode[1] = 144 * scale;
	}
	if (vmode[2] == 32)
		fmt = SDL_PIXELFORMAT_BGRA32;
	else if(vmode[2] == 16)
		fmt = SDL_PIXELFORMAT_BGR565;
	else
		die("vmode pixel format not supported, choose 16 or 32");

#if SDL_HWACCEL_OPENGL
	flags = SDL_WINDOW_OPENGL;
#endif

	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
    else
        flags |= SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_VIDEO))
		die("SDL: Couldn't initialize SDL: %s\n", SDL_GetError());

#if SDL_HWACCEL_OPENGL
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#elif defined(HW_ACCEL_SOFTWARE)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
#endif

	if (!(win = SDL_CreateWindow("gnuboy", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, vmode[0], vmode[1], flags)))
		die("SDL: can't set video mode: %s\n", SDL_GetError());

	/* for SDL2, which uses OpenGL, we internally use scale 1 and
	   render everything into a 32bit high color buffer, and let the
	   hardware do the scaling; thus "fb.delegate_scaling" */

	/* warning: using vsync causes much higher CPU usage in the XServer
	   you may want to turn it off using by setting the environment
	   variable SDL_RENDER_VSYNC to "0" or "false", which activates
	   SDL_HINT_RENDER_VSYNC (yes, the env var lacks "HINT") */
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|vsync*SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		fprintf(stderr, "warning: fallback to software renderer\n");
		renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE|vsync*SDL_RENDERER_PRESENTVSYNC);
	}
	if (!renderer) die("SDL2: can't create renderer: %s\n", SDL_GetError());

	SDL_GetRendererInfo(renderer, &info);
	fprintf(stdout, "using renderer %s\n", info.name);

	SDL_RenderSetScale(renderer, scale, scale);

	texture = SDL_CreateTexture(renderer, fmt,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);

    SDL_RenderSetLogicalSize(renderer, vmode[0], vmode[1]);
    integer_scale = rc_getint("integer_scale");
    if (integer_scale)
    {
        int window_width, window_height, render_width, render_height;
        SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
        SDL_RenderGetLogicalSize(renderer, &render_width, &render_height);
        SDL_bool makes_sense = (window_width >= render_width && window_height >= render_height);
        SDL_RenderSetIntegerScale(renderer, makes_sense);
    }

	SDL_ShowCursor(0);

	format = SDL_AllocFormat(fmt);
    const SDL_Rect dst_rect = { 0, 0, fb.w, fb.h };
	SDL_LockTexture(texture, &dst_rect, &pixels, &pitch);

	fb.delegate_scaling = 1;
	fb.w = 160;
	fb.h = 144;
	fb.pelsize = vmode[2]/8;
	fb.pitch = pitch;
	fb.indexed = 0;
	fb.ptr = pixels;

	fb.cc[0].r = format->Rloss;
	fb.cc[0].l = format->Rshift;
	fb.cc[1].r = format->Gloss;
	fb.cc[1].l = format->Gshift;
	fb.cc[2].r = format->Bloss;
	fb.cc[2].l = format->Bshift;

	SDL_UnlockTexture(texture);
	SDL_FreeFormat(format);

	fb.enabled = 1;
	fb.dirty = 0;
}

void ev_poll(int wait)
{
	event_t ev;
	SDL_Event event;

	if(wait && SDL_WaitEvent(&event)) goto process_evt;

	while (SDL_PollEvent(&event))
	{
	process_evt:;
		switch(event.type)
		{
		case SDL_WINDOWEVENT:
			/* https://wiki.libsdl.org/SDL_WindowEvent */
			switch(event.window.event) {
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_HIDDEN:
				fb.enabled = 0; break;
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_RESTORED:
				fb.enabled = 1; break;
			}
			break;
		case SDL_KEYDOWN:
			if ((event.key.keysym.sym == SDLK_RETURN) && (event.key.keysym.mod & KMOD_ALT)) {
				SDL_SetWindowFullscreen(win, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
				fullscreen = !fullscreen;
			}
			ev.type = EV_PRESS;
			ev.code = mapscancode(event.key.keysym.sym);
			ev_postevent(&ev);
			break;
		case SDL_KEYUP:
			ev.type = EV_RELEASE;
			ev.code = mapscancode(event.key.keysym.sym);
			ev_postevent(&ev);
			break;
		case SDL_JOYHATMOTION:
		case SDL_CONTROLLERAXISMOTION:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			sdljoy_process_event(&event);
			break;
		case SDL_QUIT:
			exit(1);
			break;
		default:
			break;
		}
	}
}

void vid_setpal(int i, int r, int g, int b)
{
	/* not supposed to be called */
}

void vid_preinit()
{
}

void vid_close()
{
	SDL_UnlockTexture(texture);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	fb.enabled = 0;
}

void vid_settitle(char *title)
{
	SDL_SetWindowTitle(win, title);
}

void vid_begin()
{
    const SDL_Rect dst_rect = { 0, 0, fb.w, fb.h };
	void *p = 0;
	int pitch;
	SDL_LockTexture(texture, &dst_rect, &p, &pitch);
	fb.ptr = p;
    fb.pitch = pitch;
}

void vid_end()
{
    SDL_Rect dst_rect = { 0, 0, 0, 0 };

	SDL_UnlockTexture(texture);

	if (fb.enabled) {
        dst_rect.w = fb.w;
        dst_rect.h = fb.h;

        SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
		SDL_RenderPresent(renderer);
	}
}
