/******************************************************************************/
#include "stdafx.h"
#if !WINDOWS
#include <dlfcn.h>
#endif
namespace EE{
#if WINDOWS_OLD

#define MEMORY 1 // '2' didn't work on one computer when trying to run Ineisis Online

#if   MEMORY==1
   #include "../../../ThirdPartyLibs/MemoryModule/MemoryModule.c" // https://github.com/fancycode/MemoryModule
#elif MEMORY==2
   #include "../../../ThirdPartyLibs/LoadDLL/LoadDLL.c" // https://www.codeproject.com/Tips/430684/Loading-Win-DLLs-manually-without-LoadLibrary
#endif
/******************************************************************************/
void DLL::delForce()
{
   HMODULE module=(HMODULE)_dll_file;
   del();
   if(module)REP(16)if(!FreeLibrary(module)){SetLastError(0); break;} // clear any errors that 'FreeLibrary' might have generated
}
DLL& DLL::del()
{
#if MEMORY==1
   if(_dll_mem ){MemoryFreeLibrary(                _dll_mem ); _dll_mem =null;}
#elif MEMORY==2
   if(_dll_mem ){        UnloadDLL((LOAD_DLL_INFO*)_dll_mem ); _dll_mem =null;}
#endif
   if(_dll_file){      FreeLibrary((HMODULE       )_dll_file); _dll_file=null;}
   return T;
}
Bool DLL::createMem(CPtr data, Int size)
{
   del();
   if(data && size>0)
   {
   #if MEMORY==1
     _dll_mem=MemoryLoadLibrary(data, size);
   #elif MEMORY==2
	   LOAD_DLL_INFO *dll=Alloc<LOAD_DLL_INFO>(); if(LoadDLLFromMemory(data, size, 0, dll)==ELoadDLLResult_OK)_dll_mem=dll;else Free(dll);
   #endif
   }
   return _dll_mem!=null;
}
Bool DLL::createFile(C Str &file) {return createFile(file());}
Bool DLL::createFile(CChar *file)
{
   del();
  _dll_file=LoadLibrary(WChar(file));
   return _dll_file!=null;
}
Ptr DLL::getFunc(CChar8 *name)C
{
#if MEMORY==1
   if(_dll_mem )return         MemoryGetProcAddress(                _dll_mem , name);
#elif MEMORY==2
   if(_dll_mem )return myGetProcAddress_LoadDLLInfo((LOAD_DLL_INFO*)_dll_mem , name);
#endif
   if(_dll_file)return               GetProcAddress((HMODULE       )_dll_file, name);
                return null;
}
#elif WINDOWS_NEW
   void DLL::delForce  () {}
   DLL& DLL::del       () {return T;}
   Bool DLL::createMem (CPtr data, Int size) {return false;}
   Bool DLL::createFile(C Str  &file)  {return false;}
   Bool DLL::createFile(CChar  *file)  {return false;}
   Ptr  DLL::getFunc   (CChar8 *name)C {return null;}
#else
   void DLL::delForce()
   {
      del();
   }
   DLL& DLL::del()
   {
      if(_dll_file){dlclose(_dll_file); _dll_file=null;}
      return T;
   }
   Bool DLL::createMem(CPtr data, Int size)
   {
      del();
      return false;
   }
   Bool DLL::createFile(CChar *file) {return createFile(Str(file));}
   Bool DLL::createFile(C Str &file)
   {
      del();
     _dll_file=dlopen(UnixPathUTF8(file), RTLD_LAZY);
      return _dll_file!=null;
   }
   Ptr DLL::getFunc(CChar8 *name)C
   {
      if(_dll_file)return dlsym(_dll_file, name);
      return null;
   }
#endif

DLL::DLL() {_dll_mem=_dll_file=null;}

Bool DLL::is()C
{
   return _dll_mem || _dll_file;
}
/******************************************************************************/
} // namespace
/******************************************************************************/
