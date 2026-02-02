/*
** $Id: lprefix.h,v 1.2.1.1 2017/04/19 17:20:42 roberto Exp $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#ifndef lprefix_h
#define lprefix_h

#ifdef __MINGW32__
#define _MMINTRIN_H_INCLUDED 1
#define _XMMINTRIN_H_INCLUDED 1
#define _EMMINTRIN_H_INCLUDED 1
#define _TMMINTRIN_H_INCLUDED 1
#define _WMMINTRIN_H_INCLUDED 1
#define _SMMINTRIN_H_INCLUDED 1
#define _PMMINTRIN_H_INCLUDED 1
#define _IMMINTRIN_H_INCLUDED 1
#define _X86INTRIN_H_INCLUDED 1
#endif

/*
** Allows POSIX/XSI stuff
*/
#if !defined(LUA_USE_C89)	/* { */

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE           600
#elif _XOPEN_SOURCE == 0
#undef _XOPEN_SOURCE  /* use -D_XOPEN_SOURCE=0 to undefine it */
#endif

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#if !defined(LUA_32BITS) && !defined(_FILE_OFFSET_BITS)
#define _LARGEFILE_SOURCE       1
#define _FILE_OFFSET_BITS       64
#endif

#endif				/* } */


/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#endif

#endif			/* } */

#endif

