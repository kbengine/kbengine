/* Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* There may be prolems include all of theese. Try to test in
   configure with ones are needed? */

/*  This is needed for the definitions of strchr... on solaris */

#ifndef _m_string_h
#define _m_string_h
#ifndef __USE_GNU
#define __USE_GNU				/* We want to use stpcpy */
#endif
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#if defined(HAVE_STRING_H)
#include <string.h>
#endif

/* need by my_vsnprintf */
#include <stdarg.h> 

/* Correct some things for UNIXWARE7 */
#ifdef HAVE_UNIXWARE7_THREADS
#undef HAVE_STRINGS_H
#undef HAVE_MEMORY_H
#define HAVE_MEMCPY
#ifndef HAVE_MEMMOVE
#define HAVE_MEMMOVE
#endif
#undef HAVE_BCMP
#undef bcopy
#undef bcmp
#undef bzero
#endif /* HAVE_UNIXWARE7_THREADS */
#ifdef _AIX
#undef HAVE_BCMP
#endif

/*  This is needed for the definitions of bzero... on solaris */
#if defined(HAVE_STRINGS_H) && !defined(HAVE_mit_thread)
#include <strings.h>
#endif

/*  This is needed for the definitions of memcpy... on solaris */
#if defined(HAVE_MEMORY_H) && !defined(__cplusplus)
#include <memory.h>
#endif

#if !defined(HAVE_MEMCPY) && !defined(HAVE_MEMMOVE)
# define memcpy(d, s, n)	bcopy ((s), (d), (n))
# define memset(A,C,B)		bfill((A),(B),(C))
# define memmove(d, s, n)	bmove ((d), (s), (n))
#elif defined(HAVE_MEMMOVE)
# define bmove(d, s, n)		memmove((d), (s), (n))
#else
# define memmove(d, s, n)	bmove((d), (s), (n)) /* our bmove */
#endif

/* Unixware 7 */
#if !defined(HAVE_BFILL)
# define bfill(A,B,C)           memset((A),(C),(B))
# define bmove_align(A,B,C)    memcpy((A),(B),(C))
#endif

#if !defined(HAVE_BCMP)
# define bcopy(s, d, n)		memcpy((d), (s), (n))
# define bcmp(A,B,C)		memcmp((A),(B),(C))
# define bzero(A,B)		memset((A),0,(B))
# define bmove_align(A,B,C)    memcpy((A),(B),(C))
#endif

#if defined(__cplusplus) && !defined(OS2)
extern "C" {
#endif

/*
  my_str_malloc() and my_str_free() are assigned to implementations in
  strings/alloc.c, but can be overridden in the calling program.
 */
extern void *(*my_str_malloc)(size_t);
extern void (*my_str_free)(void *);

#if defined(HAVE_STPCPY) && !defined(HAVE_mit_thread)
#define strmov(A,B) stpcpy((A),(B))
#ifndef stpcpy
extern char *stpcpy(char *, const char *);	/* For AIX with gcc 2.95.3 */
#endif
#endif

/* Declared in int2str() */
extern char NEAR _dig_vec_upper[];
extern char NEAR _dig_vec_lower[];

/* Defined in strtod.c */
extern const double log_10[309];

#ifndef strmov
#define strmov_overlapp(A,B) strmov(A,B)
#define strmake_overlapp(A,B,C) strmake(A,B,C)
#endif

#ifdef BAD_MEMCPY			/* Problem with gcc on Alpha */
#define memcpy_fixed(A,B,C) bmove((A),(B),(C))
#else
#define memcpy_fixed(A,B,C) memcpy((A),(B),(C))
#endif

#ifdef MSDOS
#undef bmove_align
#define bmove512(A,B,C) bmove_align(A,B,C)
extern	void bmove_align(gptr dst,const gptr src,uint len);
#endif

#if (!defined(USE_BMOVE512) || defined(HAVE_purify)) && !defined(bmove512)
#define bmove512(A,B,C) memcpy(A,B,C)
#endif

	/* Prototypes for string functions */

#if !defined(bfill) && !defined(HAVE_BFILL)
extern	void bfill(gptr dst,uint len,pchar fill);
#endif

#if !defined(bzero) && !defined(HAVE_BZERO)
extern	void bzero(gptr dst,uint len);
#endif

#if !defined(bcmp) && !defined(HAVE_BCMP)
extern	int bcmp(const char *s1,const char *s2,uint len);
#endif
#ifdef HAVE_purify
extern	int my_bcmp(const char *s1,const char *s2,uint len);
#undef bcmp
#define bcmp(A,B,C) my_bcmp((A),(B),(C))
#endif

#ifndef bmove512
extern	void bmove512(gptr dst,const gptr src,uint len);
#endif

#if !defined(HAVE_BMOVE) && !defined(bmove)
extern	void bmove(char *dst, const char *src,uint len);
#endif

extern	void bmove_upp(char *dst,const char *src,uint len);
extern	void bchange(char *dst,uint old_len,const char *src,
		     uint new_len,uint tot_len);
extern	void strappend(char *s,uint len,pchar fill);
extern	char *strend(const char *s);
extern  char *strcend(const char *, pchar);
extern	char *strfield(char *src,int fields,int chars,int blanks,
			   int tabch);
extern	char *strfill(my_string s,uint len,pchar fill);
extern	uint strinstr(const char *str,const char *search);
extern  uint r_strinstr(reg1 my_string str,int from, reg4 my_string search);
extern	char *strkey(char *dst,char *head,char *tail,char *flags);
extern	char *strmake(char *dst,const char *src,uint length);

#ifndef strmov
extern	char *strmov(char *dst,const char *src);
#else
extern	char *strmov_overlapp(char *dst,const char *src);
#endif
extern	char *strnmov(char *dst,const char *src,uint n);
extern	char *strsuff(const char *src,const char *suffix);
extern	char *strcont(const char *src,const char *set);
extern	char *strxcat _VARARGS((char *dst,const char *src, ...));
extern	char *strxmov _VARARGS((char *dst,const char *src, ...));
extern	char *strxcpy _VARARGS((char *dst,const char *src, ...));
extern	char *strxncat _VARARGS((char *dst,uint len, const char *src, ...));
extern	char *strxnmov _VARARGS((char *dst,uint len, const char *src, ...));
extern	char *strxncpy _VARARGS((char *dst,uint len, const char *src, ...));

/* Prototypes of normal stringfunctions (with may ours) */

#ifdef WANT_STRING_PROTOTYPES
extern char *strcat(char *, const char *);
extern char *strchr(const char *, pchar);
extern char *strrchr(const char *, pchar);
extern char *strcpy(char *, const char *);
extern int strcmp(const char *, const char *);
#ifndef __GNUC__
extern size_t strlen(const char *);
#endif
#endif
#ifndef HAVE_STRNLEN
extern uint strnlen(const char *s, uint n);
#endif

#if !defined(__cplusplus)
#ifndef HAVE_STRPBRK
extern char *strpbrk(const char *, const char *);
#endif
#ifndef HAVE_STRSTR
extern char *strstr(const char *, const char *);
#endif
#endif
extern int is_prefix(const char *, const char *);

/* Conversion routines */
double my_strtod(const char *str, char **end, int *error);
double my_atof(const char *nptr);

#ifndef NOT_FIXED_DEC
#define NOT_FIXED_DEC			31
#endif

/*
  Max length of a floating point number.
 */
#define FLOATING_POINT_BUFFER (311 + NOT_FIXED_DEC)

extern char *llstr(longlong value,char *buff);
extern char *ullstr(longlong value,char *buff);
#ifndef HAVE_STRTOUL
extern long strtol(const char *str, char **ptr, int base);
extern ulong strtoul(const char *str, char **ptr, int base);
#endif

extern char *int2str(long val, char *dst, int radix, int upcase);
extern char *int10_to_str(long val,char *dst,int radix);
extern char *str2int(const char *src,int radix,long lower,long upper,
			 long *val);
longlong my_strtoll10(const char *nptr, char **endptr, int *error);
#if SIZEOF_LONG == SIZEOF_LONG_LONG
#define longlong2str(A,B,C) int2str((A),(B),(C),1)
#define longlong10_to_str(A,B,C) int10_to_str((A),(B),(C))
#undef strtoll
#define strtoll(A,B,C) strtol((A),(B),(C))
#define strtoull(A,B,C) strtoul((A),(B),(C))
#ifndef HAVE_STRTOULL
#define HAVE_STRTOULL
#endif
#ifndef HAVE_STRTOLL
#define HAVE_STRTOLL
#endif
#else
#ifdef HAVE_LONG_LONG
extern char *longlong2str(longlong val,char *dst,int radix);
extern char *longlong10_to_str(longlong val,char *dst,int radix);
#if (!defined(HAVE_STRTOULL) || defined(HAVE_mit_thread)) || defined(NO_STRTOLL_PROTO)
extern longlong strtoll(const char *str, char **ptr, int base);
extern ulonglong strtoull(const char *str, char **ptr, int base);
#endif
#endif
#endif

/* my_vsnprintf.c */

extern int my_vsnprintf( char *str, size_t n,
                                const char *format, va_list ap );
extern int my_snprintf(char *to, size_t n, const char *fmt, ...)
  ATTRIBUTE_FORMAT(printf, 3, 4);

#if defined(__cplusplus) && !defined(OS2)
}
#endif
#endif
