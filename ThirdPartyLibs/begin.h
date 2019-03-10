// this file should be included before including external headers, after those headers were included, "end.h" should be included

#undef C
#undef T
#undef null

#if DEBUG
   #define SET_DEBUG 1
#else
   #define SET_DEBUG 0
   #undef DEBUG
#endif

#if WINDOWS
   #define SET_WINDOWS 1
#else
   #define SET_WINDOWS 0
   #undef WINDOWS
#endif

#if MAC
   #define SET_MAC 1
#else
   #define SET_MAC 0
   #undef MAC
#endif

#if LINUX
   #define SET_LINUX 1
#else
   #define SET_LINUX 0
   #undef LINUX
#endif

#if ANDROID
   #define SET_ANDROID 1
#else
   #define SET_ANDROID 0
   #undef ANDROID
#endif

#if IOS
   #define SET_IOS 1
#else
   #define SET_IOS 0
   #undef IOS
#endif

#if WEB
   #define SET_WEB 1
#else
   #define SET_WEB 0
   #undef WEB
#endif

#undef  LONG_MIN
#define LONG_MIN (-2147483647L - 1) // minimum (signed) long value
