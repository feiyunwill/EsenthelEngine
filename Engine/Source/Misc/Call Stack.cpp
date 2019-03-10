/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   StackWalker can't support getting the context, and extracting the stack at a later time,
      because the stack is not included in the context (this was tested)

/******************************************************************************/
#if WINDOWS_OLD
namespace StackWalker
{
   #define SW_PROFILE 0
   #if     SW_PROFILE
      Dbl d[16];
      struct Temp{~Temp(){Str s; FREPA(d)s.line()+=S+i+':'+d[i]; Exit(s);}}temp;
      #define SWP_START  Dbl t=Time.curTime();
      #define SWP(x)    {Dbl c=Time.curTime(); d[x]+=c-t; t=c;}
   #else
      #define SWP_START
      #define SWP(x)
   #endif
   #include "../ThirdPartyLibs/StackWalker/StackWalker.cpp"
   struct StackWalkerEx : StackWalker
   {
      Int  i;
      Str *stack;

      virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
      {
         if(++i>2 // skip first 2 entries ("StackWalkerEx::ShowCallStack", "GetCallStack", "check" is inlined in release)
         // skip Windows stuff
         && !Equal(entry.name, "RtlUnicodeStringToInteger", true)
         && !Equal(entry.name, "RtlUserThreadStart"       , true)
         && !Equal(entry.name, "BaseThreadInitThunk"      , true)
         && !Equal(entry.name, "__scrt_common_main_seh"   , true)
         && !Equal(entry.name, X64 ? "thread_start<unsigned int (__cdecl*)(void * __ptr64)>" : "thread_start<unsigned int (__stdcall*)(void *)>", true)
         )
         {
            SWP_START
            stack->line()+=entry.name;
            SWP(15)
         }
      }

      bool check(Str &stack)
      {
         i=0;
         T.stack=&stack;
         return ShowCallstack()!=0;
      }
   };
} // namespace
#endif
/******************************************************************************/
Bool GetCallStack(Str &stack)
{
   stack.clear();
#if WINDOWS_OLD
   StackWalker::StackWalkerEx sw; return sw.check(stack);
#elif WEB
   int size=emscripten_get_callstack(EM_LOG_DEMANGLE|EM_LOG_NO_PATHS|EM_LOG_FUNC_PARAMS, null, 0)-1; // remove null-terminated char
   if( size>0)
   {
      Memt<Char8> temp; temp.setNum(size+64); // +64 for safety because of different line numbers than before
      Int got=emscripten_get_callstack(EM_LOG_DEMANGLE|EM_LOG_NO_PATHS|EM_LOG_FUNC_PARAMS, temp.data(), temp.elms())-1; // remove null-terminated char
      if( got>0)
      {
         stack.reserve(got); FREP(got)stack+=temp[i];
         return true;
      }
   }
#endif
   return false;
}
Bool GetCallStackFast(Str &stack) // !! this function is not thread-safe !!
{
   stack.clear();
#if WINDOWS_OLD
   static StackWalker::StackWalkerEx *SW; if(!SW){New(SW); SW->fast=true;} return SW->check(stack);
#elif WEB
   return GetCallStack(stack);
#endif
   return false;
}
/******************************************************************************/
} // namespace
/******************************************************************************/
