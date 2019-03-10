#ifndef _TIF_CONFIG_H_
#define _TIF_CONFIG_H_

#ifdef _WIN32 // Windows
   #define PLATFORM(windows, unix) windows
#else
   #define PLATFORM(windows, unix) unix
#endif

#if defined _WIN64 || defined __LP64__ // 64-bit platform
   #define X64 1
#else
   #define X64 0
#endif

#ifndef _WIN32
#include <stdint.h>
#endif

typedef   PLATFORM(  signed __int8 ,   int8_t)   I8 ,  SByte; //   Signed Int ( 8-bit)                        -128 .. 127
typedef   PLATFORM(unsigned __int8 ,  uint8_t)   U8 ,   Byte; // Unsigned Int ( 8-bit)                           0 .. 255
typedef   PLATFORM(  signed __int16,  int16_t)   I16,  Short; //   Signed Int (16-bit)                     -32 768 .. 32 767
typedef   PLATFORM(unsigned __int16, uint16_t)   U16, UShort; // Unsigned Int (16-bit)                           0 .. 65 535
typedef   PLATFORM(  signed __int32,  int32_t)   I32,    Int; //   Signed Int (32-bit)              -2 147 483 648 .. 2 147 483 647
typedef   PLATFORM(unsigned __int32, uint32_t)   U32,   UInt; // Unsigned Int (32-bit)                           0 .. 4 294 967 295
typedef   PLATFORM(  signed __int64,  int64_t)   I64,   Long; //   Signed Int (64-bit)  -9 223 372 036 854 775 808 ..  9 223 372 036 854 775 807
typedef   PLATFORM(unsigned __int64, uint64_t)   U64,  ULong; // Unsigned Int (64-bit)                           0 .. 18 446 744 073 709 551 615

#if X64
   typedef I64  IntPtr; //   Signed Int capable of storing full memory address (needs to be 64-bit on 64-bit platforms)
   typedef U64 UIntPtr; // Unsigned Int capable of storing full memory address (needs to be 64-bit on 64-bit platforms)
#else
   typedef I32  IntPtr; //   Signed Int capable of storing full memory address (needs to be 32-bit on 32-bit platforms)
   typedef U32 UIntPtr; // Unsigned Int capable of storing full memory address (needs to be 32-bit on 32-bit platforms)
#endif

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#define HAVE_IEEEFP 1

/* Define to 1 if you have the `jbg_newlen' function. */
#define HAVE_JBG_NEWLEN 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
//#define HAVE_IO_H 1 ESENTHEL CHANGED

/* Define to 1 if you have the <search.h> header file. */
#if !ANDROID               // ESENTHEL CHANGED
   #define HAVE_SEARCH_H 1 // ESENTHEL CHANGED
#endif

/* Define to 1 if you have the `setmode' function. */
#define HAVE_SETMODE 1

/* Define to 1 if you have the declaration of `optarg', and to 0 if you don't. */
#define HAVE_DECL_OPTARG 0

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT sizeof(int)

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG sizeof(long)

/* The size of `size_t', as computed by sizeof. */
#  define SIZEOF_SIZE_T sizeof(size_t)

/* Signed 64-bit type formatter */
#define TIFF_INT64_FORMAT "%I64d"

/* Signed 64-bit type */
#define TIFF_INT64_T I64

/* Unsigned 64-bit type formatter */
#define TIFF_UINT64_FORMAT "%I64u"

/* Unsigned 64-bit type */
#define TIFF_UINT64_T U64

/* Pointer difference type */
#  define TIFF_PTRDIFF_T IntPtr

/* Unsigned size type */
#  define TIFF_SIZE_T UIntPtr

/* Signed size type */
#  define TIFF_SSIZE_T IntPtr

#if X64
/* Size type formatter */
#  define TIFF_SIZE_FORMAT TIFF_INT64_FORMAT

/* Signed size type formatter */
#  define TIFF_SSIZE_FORMAT TIFF_INT64_FORMAT

#else
/* Size type formatter */
#  define TIFF_SIZE_FORMAT "%u"

/* Signed size type formatter */
#  define TIFF_SSIZE_FORMAT "%d"
#endif

// ESENTHEL CHANGED
#define HAVE_SNPRINTF
#define HAVE__SNPRINTF
#ifndef _WIN32
#define HAVE_UNISTD_H 1
#define HAVE_GETOPT 1
#endif

/* Set the native cpu bit order */
#define HOST_FILLORDER FILLORDER_LSB2MSB

/* Visual Studio 2015 / VC 14 / MSVC 19.00 finally has snprintf() */
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
# ifndef inline
#  define inline __inline
# endif
#endif

#define lfind _lfind

#pragma warning(disable : 4996) /* function deprecation warnings */

#endif /* _TIF_CONFIG_H_ */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
