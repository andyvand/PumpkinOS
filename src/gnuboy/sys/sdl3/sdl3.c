/*
 * sdl3.c
 * sdl3 interfaces -- based on sdl.c
 *
 * (C) 2001 Damian Gryski <dgryski@uwaterloo.ca> (SDL 1 version)
 * (C) 2021 rofl0r (SDL2 port)
 * (C) 2025 Andy Vandijck (SDL3 port)
 *
 * Licensed under the GPLv2, or later.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "../../fb.h"
#include "../../input.h"
#include "../../rc.h"

//#define _DEBUG 1

extern void sdljoy_process_event(SDL_Event *event);

struct fb fb;

static int fullscreen = 0;
static int use_altenter = 1;
static int vsync;

static SDL_Window *win = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

#ifdef VID_MODE_16BIT
static int vmode[3] = { 0, 0, 16 };
#else /* VID_MODE_32BIT */
static int vmode[3] = { 0, 0, 32 };
#endif

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
	int flags = SDL_WINDOW_HIDDEN;
	int scale = rc_getint("scale");
	int pitch = 0;
	int fmt = 0;
	void *pixels = 0;
    const SDL_PixelFormatDetails *format = NULL;

	if (!vmode[0] || !vmode[1])
	{
		if (scale < 1) scale = 2;
		vmode[0] = 160 * scale;
		vmode[1] = 144 * scale;
	}

	if (vmode[2] == 32)
		fmt = SDL_PIXELFORMAT_BGRX8888;
	else if(vmode[2] == 16)
		fmt = SDL_PIXELFORMAT_BGR565;
	else
		die("vmode pixel format not supported, choose 16 or 32");

#ifdef HW_ACCEL_OPENGL
	flags = SDL_WINDOW_OPENGL;
#elif defined(HW_ACCEL_VULKAN)
    flags |= SDL_WINDOW_VULKAN;
#elif defined(HW_ACCEL_METAL)
    flags |= SDL_WINDOW_METAL;
#endif

	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
    else
        flags |= SDL_WINDOW_RESIZABLE;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == false)
    {
        die("SDL: Couldn't initialize SDL subsystem: %s\n", SDL_GetError());
    }

	if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        die("SDL: Couldn't initialize SDL: %s\n", SDL_GetError());
    }

#ifdef HW_ACCEL_OPENGL
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#elif defined(HW_ACCEL_VULKAN)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");
#elif defined(HW_ACCEL_METAL)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
#elif defined(HW_ACCEL_SOFTWARE)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
#endif

#ifdef CONFIG_IDF_TARGET
#if CONFIG_BSP_DISPLAY_ROTATION_SWAP_XY
    if ((win = SDL_CreateWindow("gnuboy", CONFIG_BSP_DISPLAY_HEIGHT, CONFIG_BSP_DISPLAY_WIDTH, flags)) == NULL)
#else
    if ((win = SDL_CreateWindow("gnuboy", CONFIG_BSP_DISPLAY_WIDTH, CONFIG_BSP_DISPLAY_HEIGHT, flags)) == NULL)
#endif
#else
	if ((win = SDL_CreateWindow("gnuboy", vmode[0], vmode[1], flags)) == NULL)
#endif
		die("SDL: can't set video mode: %s\n", SDL_GetError());

	/* for SDL3, which uses OpenGL, we internally use scale 1 and
	   render everything into a 32bit high color buffer, and let the
	   hardware do the scaling; thus "fb.delegate_scaling" */

	/* warning: using vsync causes much higher CPU usage in the XServer
	   you may want to turn it off using by setting the environment
	   variable SDL_RENDER_VSYNC to "0" or "false", which activates
	   SDL_HINT_RENDER_VSYNC (yes, the env var lacks "HINT") */
	renderer = SDL_CreateRenderer(win, SDL_GetHint(SDL_HINT_RENDER_DRIVER));

	if (renderer == NULL) {
		fprintf(stderr, "warning: fallback to software renderer\n");
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		renderer = SDL_CreateRenderer(win, "software");
	}
	if (renderer == NULL) die("SDL3: can't create renderer: %s\n", SDL_GetError());

    if (vsync)
    {
        SDL_SetRenderVSync(renderer, 1);
    }

	fprintf(stdout, "using renderer %s\n", SDL_GetRendererName(renderer));

    SDL_SetRenderScale(renderer, (float)scale, (float)scale);

	texture = SDL_CreateTexture(renderer, fmt, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    if (texture == NULL)
    {
        die("SDL3: can't create texture: %s\n", SDL_GetError());
    }

    SDL_ShowWindow(win);

    SDL_SetRenderLogicalPresentation(renderer, vmode[0], vmode[1], SDL_LOGICAL_PRESENTATION_DISABLED);

    if (scale)
    {
        int window_width, window_height, render_width, render_height;
        bool makes_sense = false;

        SDL_RendererLogicalPresentation representation;
        SDL_GetCurrentRenderOutputSize(renderer, &window_width, &window_height);
        SDL_GetRenderLogicalPresentation(renderer, &render_width, &render_height, &representation);

        makes_sense = (window_width >= render_width && window_height >= render_height);

        if (makes_sense == true)
        {
            SDL_SetRenderLogicalPresentation(renderer, vmode[0], vmode[1], SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
        } else {
            SDL_SetRenderLogicalPresentation(renderer, vmode[0], vmode[1], SDL_LOGICAL_PRESENTATION_DISABLED);
        }

        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    }

	SDL_HideCursor();

	format = SDL_GetPixelFormatDetails(fmt);
    const SDL_Rect dst_rect = { 0, 0, 160, 144 };

    if (SDL_LockTexture(texture, &dst_rect, &pixels, &pitch) == false)
    {
        die("Couldn't lock texture (%dx%d): %p (pitch %d)\n", dst_rect.w, dst_rect.h, pixels, pitch);
    }

	fb.delegate_scaling = 1;
	fb.w = 160;
	fb.h = 144;
	fb.pelsize = vmode[2]/8;
	fb.pitch = pitch;
	fb.indexed = 0;
	fb.ptr = pixels;

    fb.cc[0].r = 8 - format->Rbits;
	fb.cc[0].l = format->Rshift;
    fb.cc[1].r = 8 - format->Gbits;
	fb.cc[1].l = format->Gshift;
    fb.cc[2].r = 8 - format->Bbits;
	fb.cc[2].l = format->Bshift;

    SDL_UnlockTexture(texture);

	format = NULL;

	fb.enabled = 1;
	fb.dirty = 0;

#ifdef _DEBUG
    fprintf(stdout, "Scale: %d\n", scale);
#endif
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
        /* https://wiki.libsdl.org/SDL_WindowEvent */
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_HIDDEN:
#ifdef _DEBUG
            fprintf(stdout, "Framebuffer disabled\n");
#endif
            fb.enabled = 0;
            break;

        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_RESTORED:
#ifdef _DEBUG
            fprintf(stdout, "Framebuffer enabled\n");
#endif
            fb.enabled = 1;
            break;

		case SDL_EVENT_KEY_DOWN:
#ifdef _DEBUG
            fprintf(stdout, "Key down: %d\n", event.key.key);
#endif
			if ((event.key.key == SDLK_RETURN) && (event.key.mod & SDL_KMOD_ALT)) {
#ifdef _DEBUG
                fprintf(stdout, "Set fullscreen: %s\n", fullscreen ? "Yes" : "No");
#endif
				SDL_SetWindowFullscreen(win, fullscreen ? true : false);
				fullscreen = !fullscreen;
			}
			ev.type = EV_PRESS;
			ev.code = mapscancode(event.key.key);
			ev_postevent(&ev);
			break;

		case SDL_EVENT_KEY_UP:
#ifdef _DEBUG
            fprintf(stdout, "Key up: %d\n", event.key.key);
#endif
			ev.type = EV_RELEASE;
			ev.code = mapscancode(event.key.key);
			ev_postevent(&ev);
			break;

		case SDL_EVENT_JOYSTICK_HAT_MOTION:
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
#ifdef _DEBUG
            fprintf(stdout, "Joystick motion\n");
#endif
			sdljoy_process_event(&event);
			break;

		case SDL_EVENT_QUIT:
#ifdef _DEBUG
            fprintf(stdout, "Quit application\n");
#endif
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
#ifdef _DEBUG
    fprintf(stdout, "Set window title: %s\n", title);
#endif

	SDL_SetWindowTitle(win, title);
}

void vid_begin()
{
    const SDL_Rect dst_rect = { 0, 0, fb.w, fb.h };
    void *pixels = NULL;
	int pitch = 0;

    if (SDL_LockTexture(texture, &dst_rect, &pixels, &pitch) == false)
    {
        die("Couldn't lock texture (%dx%d): %p (pitch %d)\n", fb.w, fb.h, pixels, pitch);
    }

	fb.ptr = pixels;
    fb.pitch = pitch;

#ifdef _DEBUG
    fprintf(stdout, "Video begin (%dx%d): %p, pitch: %d\n", fb.w, fb.h, pixels, pitch);
#endif
}

void vid_end()
{
    SDL_FRect dst_rect = { 0, 0, 0, 0 };

#ifdef _DEBUG
    fprintf(stdout, "Window size: %dx%d\n", fb.w, fb.h);
    fprintf(stdout, "Framebuffer enabled: %s\n", fb.enabled ? "Yes" : "No");
    fprintf(stdout, "Framebuffer pointer: %p\n", fb.ptr);
    fprintf(stdout, "Renderer: %p\n", renderer);
    fprintf(stdout, "Texture: %p\n", texture);
#endif

    SDL_UnlockTexture(texture);

	if (fb.enabled) {
        dst_rect.w = (float)fb.w;
        dst_rect.h = (float)fb.h;

        if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) == false)
        {
            fprintf(stderr, "warning: failed to set renderer black draw color %p (%dx%d): %s\n", renderer, fb.w, fb.h, SDL_GetError());
        }

        if (SDL_RenderClear(renderer) == false)
        {
            fprintf(stderr, "warning: failed to clear renderer %p (%dx%d): %s\n", renderer, fb.w, fb.h, SDL_GetError());
        }

        if (SDL_RenderTexture(renderer, texture, NULL, &dst_rect) == false)
        {
            fprintf(stderr, "warning: failed to render texture %p (%dx%d): %s\n", texture, fb.w, fb.h, SDL_GetError());
        }

		if (SDL_RenderPresent(renderer) == false)
        {
            fprintf(stderr, "warning: failed to present renderer %p (%dx%d): %s\n", renderer, fb.w, fb.h, SDL_GetError());
        }
	}
}
