#define C    const
#define T    (*this)
#define null nullptr

#undef  DEBUG
#define DEBUG   SET_DEBUG
#undef  WINDOWS
#define WINDOWS SET_WINDOWS
#undef  MAC
#define MAC     SET_MAC
#undef  LINUX
#define LINUX   SET_LINUX
#undef  ANDROID
#define ANDROID SET_ANDROID
#undef  IOS
#define IOS     SET_IOS
#undef  WEB
#define WEB     SET_WEB
#undef  LONG_MIN
#define LONG_MIN (-0x7FFFFFFFFFFFFFFF-1) // Minimum possible value of 64-bit   signed int ( Long )
