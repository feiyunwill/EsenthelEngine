/*
  Configuration defines for installed libtiff.
  This file maintained for backward compatibility. Do not use definitions
  from this file in your programs.
*/

#ifndef _TIFFCONF_
#define _TIFFCONF_

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

/* Define to 1 if the system has the type `int16'. */
/* #undef HAVE_INT16 */

/* Define to 1 if the system has the type `int32'. */
/* #undef HAVE_INT32 */

/* Define to 1 if the system has the type `int8'. */
/* #undef HAVE_INT8 */

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT sizeof(int)

/* Signed 8-bit type */
#define TIFF_INT8_T I8

/* Unsigned 8-bit type */
#define TIFF_UINT8_T U8

/* Signed 16-bit type */
#define TIFF_INT16_T I16

/* Unsigned 16-bit type */
#define TIFF_UINT16_T U16

/* Signed 32-bit type formatter */
#define TIFF_INT32_FORMAT "%d"

/* Signed 32-bit type */
#define TIFF_INT32_T I32

/* Unsigned 32-bit type formatter */
#define TIFF_UINT32_FORMAT "%u"

/* Unsigned 32-bit type */
#define TIFF_UINT32_T U32

/* Signed 64-bit type formatter */
#define TIFF_INT64_FORMAT "%I64d"

/* Signed 64-bit type */
#define TIFF_INT64_T I64

/* Unsigned 64-bit type formatter */
#define TIFF_UINT64_FORMAT "%I64u"

/* Unsigned 64-bit type */
#define TIFF_UINT64_T U64

/* Signed size type */
#  define TIFF_SSIZE_T IntPtr

/* Compatibility stuff. */

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#define HAVE_IEEEFP 1

/* Set the native cpu bit order (FILLORDER_LSB2MSB or FILLORDER_MSB2LSB) */
#define HOST_FILLORDER FILLORDER_LSB2MSB

/* Native cpu byte order: 1 if big-endian (Motorola) or 0 if little-endian
   (Intel) */
#define HOST_BIGENDIAN 0

/* Support CCITT Group 3 & 4 algorithms */
#define CCITT_SUPPORT 1

/* Support JPEG compression (requires IJG JPEG library) */
/* #undef JPEG_SUPPORT */

/* Support JBIG compression (requires JBIG-KIT library) */
/* #undef JBIG_SUPPORT */

/* Support LogLuv high dynamic range encoding */
#define LOGLUV_SUPPORT 1

/* Support LZW algorithm */
#define LZW_SUPPORT 1

/* Support NeXT 2-bit RLE algorithm */
#define NEXT_SUPPORT 1

/* Support Old JPEG compresson (read contrib/ojpeg/README first! Compilation
   fails with unpatched IJG JPEG library) */
/* #undef OJPEG_SUPPORT */

/* Support Macintosh PackBits algorithm */
#define PACKBITS_SUPPORT 1

/* Support Pixar log-format algorithm (requires Zlib) */
/* #undef PIXARLOG_SUPPORT */

/* Support ThunderScan 4-bit RLE algorithm */
#define THUNDER_SUPPORT 1

/* Support Deflate compression */
#define ZIP_SUPPORT 1

/* Support strip chopping (whether or not to convert single-strip uncompressed
   images to mutiple strips of ~8Kb to reduce memory usage) */
#define STRIPCHOP_DEFAULT TIFF_STRIPCHOP

/* Enable SubIFD tag (330) support */
#define SUBIFD_SUPPORT 1

/* Treat extra sample as alpha (default enabled). The RGBA interface will
   treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many
   packages produce RGBA files but don't mark the alpha properly. */
#define DEFAULT_EXTRASAMPLE_AS_ALPHA 1

/* Pick up YCbCr subsampling info from the JPEG data stream to support files
   lacking the tag (default enabled). */
#define CHECK_JPEG_YCBCR_SUBSAMPLING 1

/* Support MS MDI magic number files as TIFF */
/* #undef MDI_SUPPORT */

/*
 * Feature support definitions.
 * XXX: These macros are obsoleted. Don't use them in your apps!
 * Macros stays here for backward compatibility and should be always defined.
 */
#define COLORIMETRY_SUPPORT
#define YCBCR_SUPPORT
#define CMYK_SUPPORT
#define ICC_SUPPORT
#define PHOTOSHOP_SUPPORT
#define IPTC_SUPPORT

#endif /* _TIFFCONF_ */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
