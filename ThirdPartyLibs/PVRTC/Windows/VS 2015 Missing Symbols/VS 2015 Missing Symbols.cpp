/******************************************************************************/
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <corecrt_wstdio.h>
extern "C" FILE* __iob_func(unsigned i) {return __acrt_iob_func(i);}
int _vsnprintf_l(char* const _Buffer, size_t const _BufferCount, char const* const _Format, _locale_t const _Locale, va_list _ArgList)
{  
    int const _Result = __stdio_common_vsprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION, _Buffer, _BufferCount, _Format, _Locale, _ArgList);
    return _Result < 0 ? -1 : _Result;
}
int _vscprintf_l(char const* const _Format, _locale_t   const _Locale, va_list           _ArgList)
{
    int const _Result = __stdio_common_vsprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR, NULL, 0, _Format, _Locale, _ArgList);
    return _Result < 0 ? -1 : _Result;
}
int _vsprintf_p_l(char*       const _Buffer, size_t      const _BufferCount, char const* const _Format, _locale_t   const _Locale, va_list           _ArgList)
{
    int const _Result = __stdio_common_vsprintf_p(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, _Buffer, _BufferCount, _Format, _Locale, _ArgList);
    return _Result < 0 ? -1 : _Result;
}
int _vscprintf_p_l(char const* const _Format, _locale_t   const _Locale, va_list           _ArgList)
{
    int const _Result = __stdio_common_vsprintf_p(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR, NULL, 0, _Format, _Locale, _ArgList);
    return _Result < 0 ? -1 : _Result;
}
int printf(char const* const _Format, ...)
{
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vfprintf_l(stdout, _Format, NULL, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}
int _vsnprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, va_list _ArgList) {return _vsnprintf_l(_Buffer, _BufferCount, _Format, NULL, _ArgList);}
int _vscprintf(char const* const _Format, va_list _ArgList) {return _vscprintf_l(_Format, NULL, _ArgList);}
int _vsprintf_p(char*       const _Buffer, size_t      const _BufferCount, char const* const _Format, va_list _ArgList) {return _vsprintf_p_l(_Buffer, _BufferCount, _Format, NULL, _ArgList);}
int _vscprintf_p(char const* const _Format, va_list           _ArgList) {return _vscprintf_p_l(_Format, NULL, _ArgList);}
int vsprintf(char*       const _Buffer, char const* const _Format, va_list           _ArgList) {return _vsnprintf_l(_Buffer, (size_t)-1, _Format, NULL, _ArgList);}
int vfprintf(FILE*       const _Stream, char const* const _Format, va_list           _ArgList) {return _vfprintf_l(_Stream, _Format, NULL, _ArgList);}
int vprintf(char const* const _Format, va_list           _ArgList) {return _vfprintf_l(stdout, _Format, NULL, _ArgList);}
/******************************************************************************/
