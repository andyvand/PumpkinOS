#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef ALT_PATH_SEP
#define SEP ';'
#else
#define SEP ':'
#endif

#ifdef _MSC_VER
#define strdup _strdup
#endif

#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#else
#define PATH_MAX 512
#endif
#endif

#if defined(_MSC_VER) && __STDC_WANT_SECURE_LIB__
#define strncpy(a,b,c) strncpy_s(a,c,b,c)
#define strncat(a,b,c) strncat_s(a,c,b,c)
#endif

char *path_search(char *name, char *mode, char *path)
{
	FILE *f;
	static char *buf;
	char *p, *n;
	int l;

/*#ifndef _WIN32
	if (buf != NULL)
		free(buf);
#endif*/

	buf = NULL;
	if (!path || !*path || *name == '/')
		return (buf = strdup(name));

	for (p = path; *p; p += l)
	{
		buf = malloc(PATH_MAX);
		if (*p == SEP) p++;
		n = strchr(p, SEP);
		if (n) l = (int)(n - p);
		else l = (int)strlen(p);
		strncpy(buf, p, PATH_MAX);
		strncat(buf, "/", PATH_MAX - strlen(buf));
		strncat(buf, name, PATH_MAX - strlen(buf) - 1);
#if defined(_MSC_VER) && __STDC_WANT_SECURE_LIB__
		fopen_s(&f, buf, mode);
		if (f != NULL)
#else
		if ((f = fopen(buf, mode)))
#endif
		{
			fclose(f);
			return buf;
		}

		if (buf != NULL)
		{
			free(buf);
		}
	}

	return name;
}







