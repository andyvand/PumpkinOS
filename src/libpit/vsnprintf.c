#include <stdarg.h>
#include <limits.h>
#include <float.h>

#include "sys.h"
#include "vsnprintf.h"

#include "ldshape.h"

#define NL_ARGMAX 64

typedef struct {
  char *buf;
  sys_size_t pos;
  sys_size_t size;
} printf_buf_t;

/* Some useful macros */

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Convenient bit representation for modifier flags, which all fall
 * within 31 codepoints of the space character. */

#define ALT_FORM   (1U<<('#'-' '))
#define ZERO_PAD   (1U<<('0'-' '))
#define LEFT_ADJ   (1U<<('-'-' '))
#define PAD_POS    (1U<<(' '-' '))
#define MARK_POS   (1U<<('+'-' '))
#define GROUPED    (1U<<('\''-' '))

#define FLAGMASK (ALT_FORM|ZERO_PAD|LEFT_ADJ|PAD_POS|MARK_POS|GROUPED)

/* State machine to accept length modifiers + conversion specifiers.
 * Result is 0 on failure, or an argument type to pop on success. */


enum {
	_BARE, _LPRE, L_LPRE, _HPRE, _HHPRE, _BIGLPRE,
	_ZTPRE, _JPRE,
	STOP,
	_PTR, _INT, _UINT, _ULLONG,
	_LONG, _ULONG,
	_SHORT, _USHORT, _CHAR, _UCHAR,
	_LLONG, _SIZET, _IMAX, _UMAX, _PDIFF, _UIPTR,
	_DBL, _LDBL,
	_NOARG,
	_MAXSTATE
};

#define S(x) [(x)-'A']

#ifdef _MSC_VER
static const unsigned char states[8][0x3A] = {
	{
		0x19,
		0x0,
		0xb,
		0x0,
		0x19,
		0x19,
		0x19,
		0x0,
		0x0,
		0x0,
		0x0,
		0x5,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x0,
		0x0,
		0x0,
		0x0,
		0xb,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x19,
		0x0,
		0xa,
		0xa,
		0x19,
		0x19,
		0x19,
		0x3,
		0xa,
		0x7,
		0x0,
		0x1,
		0x1b,
		0x9,
		0xb,
		0x18,
		0x0,
		0x0,
		0x9,
		0x6,
		0xb,
		0x0,
		0x0,
		0xb,
		0x0,
		0x6
	},
	{
		0x19,
		0x0,
		0x0,
		0x0,
		0x19,
		0x19,
		0x19,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0xe,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x19,
		0x0,
		0xb,
		0xd,
		0x19,
		0x19,
		0x19,
		0x0,
		0xd,
		0x0,
		0x0,
		0x2,
		0x0,
		0x9,
		0xe,
		0x0,
		0x0,
		0x0,
		0x9,
		0x0,
		0xe,
		0x0,
		0x0,
		0xe,
		0x0,
		0x0
	},
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0xc,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x13,
		0x0,
		0x0,
		0x0,
		0x0,
		0x13,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0xc,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0xc,
		0x0,
		0x0,
		0xc,
		0x0,
		0x0
	},
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x10,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0xf,
		0x0,
		0x0,
		0x0,
		0x4,
		0xf,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x10,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x10,
		0x0,
		0x0,
		0x10,
		0x0,
		0x0
	},
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x12,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x11,
		0x0,
		0x0,
		0x0,
		0x0,
		0x11,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x12,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x12,
		0x0,
		0x0,
		0x12,
		0x0,
		0x0
	},
	{
		0x1a,
		0x0,
		0x0,
		0x0,
		0x1a,
		0x1a,
		0x1a,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x1a,
		0x0,
		0x0,
		0x0,
		0x1a,
		0x1a,
		0x1a,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0
	},
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x14,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x17,
		0x0,
		0x0,
		0x0,
		0x0,
		0x17,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x14,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x14,
		0x0,
		0x0,
		0x14,
		0x0,
		0x0
	},
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x16,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x15,
		0x0,
		0x0,
		0x0,
		0x0,
		0x15,
		0x0,
		0x0,
		0x0,
		0x0,
		0x9,
		0x16,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x16,
		0x0,
		0x0,
		0x16,
		0x0,
		0x0
	}
};
#else
static const unsigned char states[8][0x3A] = {
	{ /* 0: _BARE types */
		S('d') = _INT, S('i') = _INT,
		S('o') = _UINT, S('u') = _UINT, S('x') = _UINT, S('X') = _UINT,
		S('e') = _DBL, S('f') = _DBL, S('g') = _DBL, S('a') = _DBL,
		S('E') = _DBL, S('F') = _DBL, S('G') = _DBL, S('A') = _DBL,
		S('c') = _INT, S('C') = _UINT,
		S('s') = _PTR, S('S') = _PTR, S('p') = _UIPTR, S('n') = _PTR,
		S('m') = _NOARG,
		S('l') = _LPRE, S('h') = _HPRE, S('L') = _BIGLPRE,
		S('z') = _ZTPRE, S('j') = _JPRE, S('t') = _ZTPRE,
	}, { /* 1: l-prefixed */
		S('d') = _LONG, S('i') = _LONG,
		S('o') = _ULONG, S('u') = _ULONG, S('x') = _ULONG, S('X') = _ULONG,
		S('e') = _DBL, S('f') = _DBL, S('g') = _DBL, S('a') = _DBL,
		S('E') = _DBL, S('F') = _DBL, S('G') = _DBL, S('A') = _DBL,
		S('c') = _UINT, S('s') = _PTR, S('n') = _PTR,
		S('l') = L_LPRE,
	}, { /* 2: ll-prefixed */
		S('d') = _LLONG, S('i') = _LLONG,
		S('o') = _ULLONG, S('u') = _ULLONG,
		S('x') = _ULLONG, S('X') = _ULLONG,
		S('n') = _PTR,
	}, { /* 3: h-prefixed */
		S('d') = _SHORT, S('i') = _SHORT,
		S('o') = _USHORT, S('u') = _USHORT,
		S('x') = _USHORT, S('X') = _USHORT,
		S('n') = _PTR,
		S('h') = _HHPRE,
	}, { /* 4: hh-prefixed */
		S('d') = _CHAR, S('i') = _CHAR,
		S('o') = _UCHAR, S('u') = _UCHAR,
		S('x') = _UCHAR, S('X') = _UCHAR,
		S('n') = _PTR,
	}, { /* 5: L-prefixed */
		S('e') = _LDBL, S('f') = _LDBL, S('g') = _LDBL, S('a') = _LDBL,
		S('E') = _LDBL, S('F') = _LDBL, S('G') = _LDBL, S('A') = _LDBL,
		S('n') = _PTR,
	}, { /* 6: z- or t-prefixed (assumed to be same size) */
		S('d') = _PDIFF, S('i') = _PDIFF,
		S('o') = _SIZET, S('u') = _SIZET,
		S('x') = _SIZET, S('X') = _SIZET,
		S('n') = _PTR,
	}, { /* 7: j-prefixed */
		S('d') = _IMAX, S('i') = _IMAX,
		S('o') = _UMAX, S('u') = _UMAX,
		S('x') = _UMAX, S('X') = _UMAX,
		S('n') = _PTR,
	}
};
#endif

#define OOB(x) ((unsigned)(x)-'A' > 'z'-'A')

union arg {
	uintmax_t i;
	long double f;
	void *p;
};

static void pop_arg(union arg *arg, int type, sys_va_list *ap) {
	switch (type) {
	       case _PTR:	arg->p = sys_va_arg(*ap, void *);
	break; case _INT:	arg->i = sys_va_arg(*ap, int);
	break; case _UINT:	arg->i = sys_va_arg(*ap, unsigned int);
	break; case _LONG:	arg->i = sys_va_arg(*ap, long);
	break; case _ULONG:	arg->i = sys_va_arg(*ap, unsigned long);
	break; case _ULLONG:	arg->i = sys_va_arg(*ap, unsigned long long);
	break; case _SHORT:	arg->i = (short)sys_va_arg(*ap, int);
	break; case _USHORT:	arg->i = (unsigned short)sys_va_arg(*ap, int);
	break; case _CHAR:	arg->i = (signed char)sys_va_arg(*ap, int);
	break; case _UCHAR:	arg->i = (unsigned char)sys_va_arg(*ap, int);
	break; case _LLONG:	arg->i = sys_va_arg(*ap, long long);
	break; case _SIZET:	arg->i = sys_va_arg(*ap, sys_size_t);
	break; case _IMAX:	arg->i = sys_va_arg(*ap, intmax_t);
	break; case _UMAX:	arg->i = sys_va_arg(*ap, uintmax_t);
	break; case _PDIFF:	arg->i = sys_va_arg(*ap, sys_ptrdiff_t);
	break; case _UIPTR:	arg->i = (uintptr_t)sys_va_arg(*ap, void *);
	break; case _DBL:	arg->f = sys_va_arg(*ap, double);
	break; case _LDBL:	arg->f = sys_va_arg(*ap, long double);
	}
}

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
static double frexp(double x, int *e) {
  union { double d; uint64_t i; } y = { x };
  int ee = y.i>>52 & 0x7ff;

  if (!ee) {
    if (x) {
	  x = frexp(x*18446744073709551616.000000, e);
      *e -= 64;
    } else *e = 0;
    return x;
  } else if (ee == 0x7ff) {
    return x;
  }

  *e = ee - 0x3fe;
  y.i &= 0x800fffffffffffffull;
  y.i |= 0x3fe0000000000000ull;
  return y.d;
}

static long double frexpl(long double x, int *e) {
  return frexp(x, e);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
static long double frexpl(long double x, int *e) {
  union ldshape u = {x};
  int ee = u.i.se & 0x7fff;

  if (!ee) {
    if (x) {
      x = frexpl(x*0x1p120, e);
      *e -= 120;
    } else *e = 0;
    return x;
  } else if (ee == 0x7fff) {
    return x;
  }

  *e = ee - 0x3ffe;
  u.i.se &= 0x8000;
  u.i.se |= 0x3ffe;
  return u.f;
}
#endif

static void out(printf_buf_t *f, const char *s, sys_size_t l) {
  sys_size_t i;

  for (i = 0; i < l && f->pos < f->size; i++) {
    f->buf[f->pos++] = s[i];
  }
}

static void pad(printf_buf_t *f, char c, int w, int l, int fl) {
	char pad[256];
	if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w) return;
	l = w - l;
	sys_memset(pad, c, l>sizeof pad ? sizeof pad : l);
	for (; l >= sizeof pad; l -= sizeof pad)
		out(f, pad, sizeof pad);
	out(f, pad, l);
}

static const char xdigits[16] = {
	"0123456789ABCDEF"
};

static char *fmt_x(uintmax_t x, char *s, int lower) {
	for (; x; x>>=4) *--s = xdigits[(x&15)]|lower;
	return s;
}

static char *fmt_o(uintmax_t x, char *s) {
	for (; x; x>>=3) *--s = '0' + (x&7);
	return s;
}

static char *fmt_u(uintmax_t x, char *s) {
	unsigned long y;
	for (   ; x>ULONG_MAX; x/=10) *--s = '0' + x%10;
	for (y=(unsigned long)x;           y; y/=10) *--s = '0' + y%10;
	return s;
}

/* Do not override this check. The floating point printing code below
 * depends on the float.h constants being right. If they are wrong, it
 * may overflow the stack. */
#if LDBL_MANT_DIG == 53
typedef char compiler_defines_long_double_incorrectly[9-(int)sizeof(long double)];
#endif

static int fmt_fp(printf_buf_t *f, long double y, int w, int p, int fl, int t) {
	uint32_t big[(LDBL_MANT_DIG+28)/29 + 1          // mantissa expansion
		+ (LDBL_MAX_EXP+LDBL_MANT_DIG+28+8)/9]; // exponent expansion
	uint32_t *a, *d, *r, *z;
	int e2=0, e, i, j, l;
	char buf[9+LDBL_MANT_DIG/4], *s;
	const char *prefix="-0X+0X 0X-0x+0x 0x";
	int pl;
	char ebuf0[3*sizeof(int)], *ebuf=&ebuf0[3*sizeof(int)], *estr;

	pl=1;
	if (sys_signbit(y)) {
		y=-y;
	} else if (fl & MARK_POS) {
		prefix+=3;
	} else if (fl & PAD_POS) {
		prefix+=6;
	} else prefix++, pl=0;

#ifdef _WIN32_WCE
	if (!_finite(y)) {
#else
	if (!sys_isfinite(y)) {
#endif
		char *s = (t&32)?"inf":"INF";
		if (y!=y) s=(t&32)?"nan":"NAN";
		pad(f, ' ', w, 3+pl, fl&~ZERO_PAD);
		out(f, prefix, pl);
		out(f, s, 3);
		pad(f, ' ', w, 3+pl, fl^LEFT_ADJ);
		return MAX(w, 3+pl);
	}

	y = frexpl(y, &e2) * 2;
	if (y) e2--;

	if ((t|32)=='a') {
		long double round = 8.0;
		int re;

		if (t&32) prefix += 9;
		pl += 2;

		if (p<0 || p>=LDBL_MANT_DIG/4-1) re=0;
		else re=LDBL_MANT_DIG/4-1-p;

		if (re) {
			round *= 1<<(LDBL_MANT_DIG%4);
			while (re--) round*=16;
			if (*prefix=='-') {
				y=-y;
				y-=round;
				y+=round;
				y=-y;
			} else {
				y+=round;
				y-=round;
			}
		}

		estr=fmt_u(e2<0 ? -e2 : e2, ebuf);
		if (estr==ebuf) *--estr='0';
		*--estr = (e2<0 ? '-' : '+');
		*--estr = t+('p'-'a');

		s=buf;
		do {
			int x=(int)y;
			*s++=xdigits[x]|(t&32);
			y=16*(y-x);
			if (s-buf==1 && (y||p>0||(fl&ALT_FORM))) *s++='.';
		} while (y);

		if (p > INT_MAX-2-(ebuf-estr)-pl)
			return -1;
		if (p && s-buf-2 < p)
			l = (p+2) + (ebuf-estr);
		else
			l = (s-buf) + (ebuf-estr);

		pad(f, ' ', w, pl+l, fl);
		out(f, prefix, pl);
		pad(f, '0', w, pl+l, fl^ZERO_PAD);
		out(f, buf, s-buf);
		pad(f, '0', l-(ebuf-estr)-(s-buf), 0, 0);
		out(f, estr, ebuf-estr);
		pad(f, ' ', w, pl+l, fl^LEFT_ADJ);
		return MAX(w, pl+l);
	}
	if (p<0) p=6;

	if (y) y *= 0x10000000, e2 -= 28;

	if (e2<0) a=r=z=big;
	else a=r=z=big+sizeof(big)/sizeof(*big) - LDBL_MANT_DIG - 1;

	do {
		*z = (uint32_t)y;
		y = 1000000000*(y-*z++);
	} while (y);

	while (e2>0) {
		uint32_t carry=0;
		int sh=MIN(29,e2);
		for (d=z-1; d>=a; d--) {
			uint64_t x = ((uint64_t)*d<<sh)+carry;
			*d = x % 1000000000;
			carry = (uint32_t)(x / 1000000000);
		}
		if (carry) *--a = carry;
		while (z>a && !z[-1]) z--;
		e2-=sh;
	}
	while (e2<0) {
		uint32_t carry=0, *b;
		int sh=MIN(9,-e2), need=1+(p+LDBL_MANT_DIG/3U+8)/9;
		for (d=a; d<z; d++) {
			uint32_t rm = *d & ((1<<sh)-1);
			*d = (*d>>sh) + carry;
			carry = (1000000000>>sh) * rm;
		}
		if (!*a) a++;
		if (carry) *z++ = carry;
		/* Avoid (slow!) computation past requested precision */
		b = (t|32)=='f' ? r : a;
		if (z-b > need) z = b+need;
		e2+=sh;
	}

	if (a<z) for (i=10, e=(int)(9*(r-a)); *a>=(uint32_t)i; i*=10, e++);
	else e=0;

	/* Perform rounding: j is precision after the radix (possibly neg) */
	j = p - ((t|32)!='f')*e - ((t|32)=='g' && p);
	if (j < 9*(z-r-1)) {
		uint32_t x;
		/* We avoid C's broken division of negative numbers */
		d = r + 1 + ((j+9*LDBL_MAX_EXP)/9 - LDBL_MAX_EXP);
		j += 9*LDBL_MAX_EXP;
		j %= 9;
		for (i=10, j++; j<9; i*=10, j++);
		x = *d % i;
		/* Are there any significant digits past j? */
		if (x || d+1!=z) {
			long double round = 2/LDBL_EPSILON;
			long double small;
			if ((*d/i & 1) || (i==1000000000 && d>a && (d[-1]&1)))
				round += 2;
			if (x<((double)i) / 2) small = 0.500000;
			else if (x == i / 2 && d + 1 == z) small = 1.000000;
			else small = 1.500000;
			if (pl && *prefix=='-') round*=-1, small*=-1;
			*d -= x;
			/* Decide whether to round by probing round+small */
			if (round+small != round) {
				*d = *d + i;
				while (*d > 999999999) {
					*d--=0;
					if (d<a) *--a=0;
					(*d)++;
				}
				for (i=10, e=(int)(9*(r-a)); *a>=(uint32_t)i; i*=10, e++);
			}
		}
		if (z>d+1) z=d+1;
	}
	for (; z>a && !z[-1]; z--);
	
	if ((t|32)=='g') {
		if (!p) p++;
		if (p>e && e>=-4) {
			t--;
			p-=e+1;
		} else {
			t-=2;
			p--;
		}
		if (!(fl&ALT_FORM)) {
			/* Count trailing zeros in last place */
			if (z>a && z[-1]) for (i=10, j=0; z[-1]%i==0; i*=10, j++);
			else j=9;
			if ((t|32)=='f')
				p = MIN(p,MAX(0,9*(z-r-1)-j));
			else
				p = MIN(p,MAX(0,9*(z-r-1)+e-j));
		}
	}
	if (p > INT_MAX-1-(p || (fl&ALT_FORM)))
		return -1;
	l = 1 + p + (p || (fl&ALT_FORM));
	if ((t|32)=='f') {
		if (e > INT_MAX-l) return -1;
		if (e>0) l+=e;
	} else {
		estr=fmt_u(e<0 ? -e : e, ebuf);
		while(ebuf-estr<2) *--estr='0';
		*--estr = (e<0 ? '-' : '+');
		*--estr = t;
		if (ebuf-estr > INT_MAX-l) return -1;
		l += ebuf-estr;
	}

	if (l > INT_MAX-pl) return -1;
	pad(f, ' ', w, pl+l, fl);
	out(f, prefix, pl);
	pad(f, '0', w, pl+l, fl^ZERO_PAD);

	if ((t|32)=='f') {
		if (a>r) a=r;
		for (d=a; d<=r; d++) {
			char *s = fmt_u(*d, buf+9);
			if (d!=a) while (s>buf) *--s='0';
			else if (s==buf+9) *--s='0';
			out(f, s, buf+9-s);
		}
		if (p || (fl&ALT_FORM)) out(f, ".", 1);
		for (; d<z && p>0; d++, p-=9) {
			char *s = fmt_u(*d, buf+9);
			while (s>buf) *--s='0';
			out(f, s, MIN(9,p));
		}
		pad(f, '0', p+9, 9, 0);
	} else {
		if (z<=a) z=a+1;
		for (d=a; d<z && p>=0; d++) {
			char *s = fmt_u(*d, buf+9);
			if (s==buf+9) *--s='0';
			if (d!=a) while (s>buf) *--s='0';
			else {
				out(f, s++, 1);
				if (p>0||(fl&ALT_FORM)) out(f, ".", 1);
			}
			out(f, s, MIN(buf+9-s, p));
			p -= buf+9-s;
		}
		pad(f, '0', p+18, 18, 0);
		out(f, estr, ebuf-estr);
	}

	pad(f, ' ', w, pl+l, fl^LEFT_ADJ);

	return MAX(w, pl+l);
}

static int getint(char **s) {
	int i;
	for (i=0; sys_isdigit(**s); (*s)++) {
		if (i > INT_MAX/10U || **s-'0' > INT_MAX-10*i) i = -1;
		else i = 10*i + (**s-'0');
	}
	return i;
}

static int printf_core(printf_buf_t *f, const char *fmt, sys_va_list *ap, union arg *nl_arg, int *nl_type) {
	char *a, *z, *s=(char *)fmt;
	unsigned l10n=0, fl;
	int w, p, xp;
	union arg arg;
	int argpos;
	unsigned st, ps;
	int cnt=0, l=0;
	sys_size_t i;
	char buf[sizeof(uintmax_t)*3];
	const char *prefix;
	int t, pl;
	uint16_t wc[2];

	for (;;) {
		/* This error is only specified for snprintf, but since it's
		 * unspecified for other forms, do the same. Stop immediately
		 * on overflow; otherwise %n could produce wrong results. */
		if (l > INT_MAX - cnt) goto overflow;

		/* Update output count, end loop when fmt is exhausted */
		cnt += l;
		if (!*s) break;

		/* Handle literal text and %% format specifiers */
		for (a=s; *s && *s!='%'; s++);
		for (z=s; s[0]=='%' && s[1]=='%'; z++, s+=2);
		if (z-a > INT_MAX-cnt) goto overflow;
		l = z-a;
		if (f) out(f, a, l);
		if (l) continue;

		if (sys_isdigit(s[1]) && s[2]=='$') {
			l10n=1;
			argpos = s[1]-'0';
			s+=3;
		} else {
			argpos = -1;
			s++;
		}

		/* Read modifier flags */
		for (fl=0; (unsigned)*s-' '<32 && (FLAGMASK&(1U<<(*s-' '))); s++)
			fl |= 1U<<(*s-' ');

		/* Read field width */
		if (*s=='*') {
			if (sys_isdigit(s[1]) && s[2]=='$') {
				l10n=1;
				if (!f) nl_type[s[1]-'0'] = _INT, w = 0;
				else w = (int)nl_arg[s[1]-'0'].i;
				s+=3;
			} else if (!l10n) {
				w = f ? sys_va_arg(*ap, int) : 0;
				s++;
			} else goto inval;
			if (w<0) fl|=LEFT_ADJ, w=-w;
		} else if ((w=getint(&s))<0) goto overflow;

		/* Read precision */
		if (*s=='.' && s[1]=='*') {
			if (sys_isdigit(s[2]) && s[3]=='$') {
				if (!f) nl_type[s[2]-'0'] = _INT, p = 0;
				else p = (int)nl_arg[s[2]-'0'].i;
				s+=4;
			} else if (!l10n) {
				p = f ? sys_va_arg(*ap, int) : 0;
				s+=2;
			} else goto inval;
			xp = (p>=0);
		} else if (*s=='.') {
			s++;
			p = getint(&s);
			xp = 1;
		} else {
			p = -1;
			xp = 0;
		}

		/* Format specifier state machine */
		st=0;
		do {
			if (OOB(*s)) goto inval;
			ps=st;
			st=(unsigned)states[st]S(*s++);
		} while (st-1<STOP);
		if (!st) goto inval;

		/* Check validity of argument type (nl/normal) */
		if (st==_NOARG) {
			if (argpos>=0) goto inval;
		} else {
			if (argpos>=0) {
				if (!f) nl_type[argpos]=st;
				else arg=nl_arg[argpos];
			} else if (f) pop_arg(&arg, st, ap);
			else return 0;
		}

		if (!f) continue;

		z = buf + sizeof(buf);
		prefix = "-+   0X0x";
		pl = 0;
		t = s[-1];

		/* Transform ls,lc -> S,C */
		if (ps && (t&15)==3) t&=~32;

		/* - and 0 flags are mutually exclusive */
		if (fl & LEFT_ADJ) fl &= ~ZERO_PAD;

		switch(t) {
		case 'n':
			switch(ps) {
			case _BARE: *(int *)arg.p = cnt; break;
			case _LPRE: *(long *)arg.p = cnt; break;
			case L_LPRE: *(long long *)arg.p = cnt; break;
			case _HPRE: *(unsigned short *)arg.p = cnt; break;
			case _HHPRE: *(unsigned char *)arg.p = cnt; break;
			case _ZTPRE: *(sys_size_t *)arg.p = cnt; break;
			case _JPRE: *(uintmax_t *)arg.p = cnt; break;
			}
			continue;
		case 'p':
			p = MAX(p, 2*sizeof(void*));
			t = 'x';
			fl |= ALT_FORM;
		case 'x': case 'X':
			a = fmt_x(arg.i, z, t&32);
			if (arg.i && (fl & ALT_FORM)) prefix+=(t>>4), pl=2;
			if (0) {
		case 'o':
			a = fmt_o(arg.i, z);
			if ((fl&ALT_FORM) && p<z-a+1) p=z-a+1;
			} if (0) {
		case 'd': case 'i':
			pl=1;
			if (arg.i>INTMAX_MAX) {
				arg.i = (uintmax_t)-((intmax_t)arg.i);
			} else if (fl & MARK_POS) {
				prefix++;
			} else if (fl & PAD_POS) {
				prefix+=2;
			} else pl=0;
		case 'u':
			a = fmt_u(arg.i, z);
			}
			if (xp && p<0) goto overflow;
			if (xp) fl &= ~ZERO_PAD;
			if (!arg.i && !p) {
				a=z;
				break;
			}
			p = MAX(p, z-a + !arg.i);
			break;
		narrow_c:
		case 'c':
			*(a=z-(p=1))=(char)arg.i;
			fl &= ~ZERO_PAD;
			break;
#if 0
		case 'm':
			if (1) a = strerror(errno); else
#endif
		case 's':
			a = arg.p ? arg.p : "(null)";
			z = a + sys_strnlen(a, p<0 ? INT_MAX : p);
			if (p<0 && *z) goto overflow;
			p = z-a;
			fl &= ~ZERO_PAD;
			break;
		case 'C':
			if (!arg.i) goto narrow_c;
			wc[0] = (uint16_t)arg.i;
			wc[1] = 0;
			arg.p = wc;
			p = -1;
#if 0
		case 'S':
			ws = arg.p;
			for (i=l=0; i<p && *ws && (l=wctomb(mb, *ws++))>=0 && l<=p-i; i+=l);
			if (l<0) return -1;
			if (i > INT_MAX) goto overflow;
			p = i;
			pad(f, ' ', w, p, fl);
			ws = arg.p;
			for (i=0; i<0U+p && *ws && i+(l=wctomb(mb, *ws++))<=p; i+=l)
				out(f, mb, l);
			pad(f, ' ', w, p, fl^LEFT_ADJ);
			l = w>p ? w : p;
#endif
			continue;
		case 'e': case 'f': case 'g': case 'a':
		case 'E': case 'F': case 'G': case 'A':
			if (xp && p<0) goto overflow;
			l = fmt_fp(f, arg.f, w, p, fl, t);
			if (l<0) goto overflow;
			continue;
		}

		if (p < z-a) p = z-a;
		if (p > INT_MAX-pl) goto overflow;
		if (w < pl+p) w = pl+p;
		if (w > INT_MAX-cnt) goto overflow;

		pad(f, ' ', w, pl+p, fl);
		out(f, prefix, pl);
		pad(f, '0', w, pl+p, fl^ZERO_PAD);
		pad(f, '0', p, z-a, 0);
		out(f, a, z-a);
		pad(f, ' ', w, pl+p, fl^LEFT_ADJ);

		l = w;
	}

	if (f) return cnt;
	if (!l10n) return 0;

	for (i=1; i<=NL_ARGMAX && nl_type[i]; i++)
		pop_arg(nl_arg+i, nl_type[i], ap);
	for (; i<=NL_ARGMAX && !nl_type[i]; i++);
	if (i<=NL_ARGMAX) goto inval;
	return 1;

inval:
	return -1;
overflow:
	return -1;
}

int my_vsnprintf(char *str, sys_size_t size, const char *fmt, sys_va_list ap) {
  printf_buf_t f;
  sys_va_list ap2 = { 0 };
	int nl_type[NL_ARGMAX+1];
	union arg nl_arg[NL_ARGMAX+1];
	int i, ret;

  if (str == NULL || fmt == NULL || size <= 0) return -1;

  for (i = 0; i <= NL_ARGMAX; i++) {
	  nl_type[i] = 0;
  }

	/* the copy allows passing va_list* even if va_list is an array */
	sys_va_copy(ap2, ap);
	if (printf_core(0, fmt, &ap2, nl_arg, nl_type) < 0) {
		sys_va_end(ap2);
		return -1;
	}

  f.buf = str;
  f.pos = 0;
  f.size = size - 1;
	ret = printf_core(&f, fmt, &ap2, nl_arg, nl_type);
	sys_va_end(ap2);
  f.buf[f.pos] = 0;

	return ret;
}
