/******************************************************************************/
#define JPEG_LIB_VERSION 80
#define LIBJPEG_TURBO_VERSION 1.4.1
#define C_ARITH_CODING_SUPPORTED
#define D_ARITH_CODING_SUPPORTED
#define MEM_SRCDST_SUPPORTED

/*
 * Define BITS_IN_JSAMPLE as either
 *   8   for 8-bit sample values (the usual setting)
 *   12  for 12-bit sample values
 * Only 8 and 12 are legal data precisions for lossy JPEG according to the
 * JPEG standard, and the IJG code does not support anything else!
 * We do not support run-time selection of data precision, sorry.
 */
#define BITS_IN_JSAMPLE  8      /* use 8 or 12 */

/* The size of `size_t', as computed by sizeof. */
#if defined _WIN64 || defined __LP64__
   #define SIZEOF_SIZE_T 8
#else
   #define SIZEOF_SIZE_T 4
#endif

/* Use accelerated SIMD routines. */
#ifndef WITH_SIMD
   #ifdef _WIN32
      #define WITH_SIMD 1
   #elif defined __APPLE__
      #include <TargetConditionals.h>
      #if !TARGET_IPHONE_SIMULATOR // use SIMD everywhere except iOS Simulator
         #define WITH_SIMD 1
      #endif
   #elif defined __linux__ && !defined ANDROID
      #define WITH_SIMD 1
   #elif defined ANDROID
      #ifdef __arm__ // use only on ARM
         #define WITH_SIMD 1
      #endif
   #elif defined EMSCRIPTEN
   #else
      #error unknown platform
   #endif
#endif
/******************************************************************************/
#ifdef _WIN32
/******************************************************************************/
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
/* #define void char */
/* #define const */
#undef __CHAR_UNSIGNED__
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS	/* we presume a 32-bit flat memory model */
#undef INCOMPLETE_TYPES_BROKEN

/* Define "boolean" as unsigned char, not int, per Windows custom */
#ifndef __RPCNDR_H__		/* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#define HAVE_BOOLEAN		/* prevent jmorecfg.h from redefining it */

/* Define "INT32" as int, not long, per Windows custom */
#if !(defined(_BASETSD_H_) || defined(_BASETSD_H))   /* don't conflict if basetsd.h already read */
typedef short INT16;
typedef signed int INT32;
#endif
#define XMD_H                   /* prevent jmorecfg.h from redefining it */

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED

#endif /* JPEG_INTERNALS */
/******************************************************************************/
#else
/******************************************************************************/
/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if the system has the type `unsigned char'. */
#define HAVE_UNSIGNED_CHAR 1

/* Define to 1 if the system has the type `unsigned short'. */
#define HAVE_UNSIGNED_SHORT 1

/* Compiler does not support pointers to undefined structures. */
/* #undef INCOMPLETE_TYPES_BROKEN */

/* Define if you have BSD-like bzero and bcopy in <strings.h> rather than
   memset/memcpy in <string.h>. */
/* #undef NEED_BSD_STRINGS */

/* Define if you need to include <sys/types.h> to get size_t. */
#define NEED_SYS_TYPES_H 1

/* Define if your (broken) compiler shifts signed values as if they were
   unsigned. */
/* #undef RIGHT_SHIFT_IS_UNSIGNED */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
/******************************************************************************/
#endif
/******************************************************************************/
