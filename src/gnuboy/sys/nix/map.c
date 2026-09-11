

/*
 * this code is not yet used. eventually we want to support using mmap
 * to map rom and sram into memory, so we don't waste virtual memory.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "defs.h"

#define DOTDIR ".gnuboy"

static char *home, *saves;

byte *map_rom()
{
    return NULL;
}

int map_checkdirs()
{
	home = malloc(strlen(getenv("HOME")) + strlen(DOTDIR) + 2);
	snprintf(home, strlen(getenv("HOME")) + strlen(DOTDIR) + 2, "%s/" DOTDIR, getenv("HOME"));
	saves = malloc(strlen(home) + 6);
	snprintf(saves, strlen(home) + 6, "%s/saves", home);
	
	if (access(saves, X_OK|W_OK))
	{
		if (access(home, X_OK|W_OK))
		{
			if (!access(home, F_OK))
				die("cannot access %s (%s)\n", home, strerror(errno));
			if (mkdir(home, 0777))
				die("cannot create %s (%s)\n", home, strerror(errno));
		}
		if (!access(saves, F_OK))
			die("cannot access %s (%s)\n", home, strerror(errno));
		if (mkdir(saves, 0777))
			die("cannot create %s (%s)\n", saves, strerror(errno));
	}
	return 0;
}










