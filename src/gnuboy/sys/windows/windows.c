/*
 * MinGW32 system file
 * based on nix.c and dos.c
 * req's SDL
 * -Dave Kiddell
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SDL2_SDL_H
#include <SDL2/SDL.h>
#else
#ifdef HAVE_SDL_H
#include <SDL/SDL.h>
#else
#include <SDL3/SDL.h>
#endif
#endif

#ifdef _MSC_VER
#define strdup _strdup
#endif

#include "../../rc.h"

void *sys_timer()
{
	Uint32 *tv;
	
	tv = malloc(sizeof *tv);
	*tv = (Uint32)(SDL_GetTicks() * 1000);
	return tv;
}

int sys_elapsed(Uint32 *cl)
{
	Uint32 now;
	Uint32 usecs;

	now = (Uint32)(SDL_GetTicks() * 1000);
	usecs = now - *cl;
	*cl = now;
	return usecs;
}

void sys_sleep(int us)
{
	/* dbk: for some reason 2000 works..
	   maybe its just compensation for the time it takes for SDL_Delay to
	   execute, or maybe sys_timer is too slow */
	SDL_Delay(us/1000);
}

void sys_sanitize(char *s)
{
	int i;
	for (i = 0; s[i]; i++)
		if (s[i] == '\\') s[i] = '/';
}

void sys_initpath(char *exe)
{
	char *buf, *home, *p;

	home = strdup(exe);
	if (home == NULL)
	{
		home = ".";
	}
	sys_sanitize(home);
	p = strrchr(home, '/');
	if (p) *p = 0;
	else
	{
		buf = ".";
		rc_setvar("rcpath", 1, &buf);
		rc_setvar("savedir", 1, &buf);
		return;
	}
	buf = malloc(strlen(home) + 8);
	snprintf(buf, strlen(home) + 8, ".;%s/", home);
	rc_setvar("rcpath", 1, &buf);
	snprintf(buf, strlen(home) + 8, ".");
	rc_setvar("savedir", 1, &buf);
	free(buf);
}

void sys_checkdir(char *path, int wr)
{
}


