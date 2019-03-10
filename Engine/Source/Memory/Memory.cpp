/******************************************************************************/
#include "stdafx.h"

#define MEM_SINGLE_MAX                0  // measure highest size for a single allocation
#define MEM_LEAK_FULL                 0  // record all allocations
#define MEM_GUARD       (DEBUG ? 16 : 0) // number of bytes to be used as a guard, use 0 to disable
#define MEM_PEAK                      0  // measure peak allocation of all memory
#define MEM_COUNT_USAGE (0 || MEM_PEAK || MEM_GUARD) // when using MEM_PEAK or MEM_GUARD then MEM_COUNT_USAGE must be always enabled
#define MEM_CUSTOM      (MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD)

#if (MEM_SINGLE_MAX || MEM_LEAK_FULL || MEM_GUARD || MEM_PEAK || MEM_COUNT_USAGE) && !DEBUG
   #pragma message("!! Warning: Use this only for debugging !!")
#endif
/******************************************************************************/
namespace EE{
/******************************************************************************/
Bool MemStats::get()
{
#if WINDOWS_OLD
   MEMORYSTATUSEX ms;
   ms.dwLength=SIZE(ms);
   if(GlobalMemoryStatusEx(&ms))
   {
      usage     =ms.dwMemoryLoad;
      avail_phys=ms.ullAvailPhys;
      total_phys=ms.ullTotalPhys;
      avail_page=ms.ullAvailPageFile;
      total_page=ms.ullTotalPageFile;
      avail_virt=ms.ullAvailVirtual;
      total_virt=ms.ullTotalVirtual;
      return true;
   }
   Zero(T); return false;
#elif WINDOWS_NEW
   total_phys=total_page=total_virt=           Windows::System::MemoryManager::AppMemoryUsageLimit;
   avail_phys=avail_page=avail_virt=total_phys-Windows::System::MemoryManager::AppMemoryUsage;
   if(total_phys)usage=(total_phys-avail_phys)*100/total_phys;
   return true;
#elif APPLE
   Zero(T);
   
   int    mib[]={CTL_HW, HW_MEMSIZE};
   Long   physical_memory;
   size_t length=SIZE(physical_memory);
   sysctl(mib, 2, &physical_memory, &length, null, 0);

   total_phys=physical_memory;

   mach_port_t            host_priv_port=mach_host_self();
   vm_size_t              page_size; host_page_size(host_priv_port, &page_size);
   vm_statistics_data_t   vm_stat;
   mach_msg_type_number_t host_count=HOST_VM_INFO_COUNT;
   if(host_statistics(host_priv_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_count)==KERN_SUCCESS)
   {
      avail_phys=avail_virt=avail_page=page_size* vm_stat.free_count;
                 total_virt=total_page=page_size*(vm_stat.free_count+vm_stat.active_count+vm_stat.inactive_count+vm_stat.wire_count);

      if(total_virt)usage=(total_virt-avail_virt)*100/total_virt;
      return true;
   }
   return false;
#else
   Char8 data[2048]; if(UnixReadFile("/proc/meminfo", data, SIZE(data))) // sample output: "MemTotal: 1024 kB\nMemFree: 512 kB\nCached: 256 kB"
   {
      if(CChar8 *total =TextPos(data, "MemTotal", false, true))
      if(CChar8 *free  =TextPos(data, "MemFree" , false, true))
      if(CChar8 *cached=TextPos(data, "Cached"  , false, true))
      {
         total +=8; // Length("MemTotal") -> 8
         free  +=7; // Length("MemFree" ) -> 7
         cached+=6; // Length("Cached"  ) -> 6
         for(; *total ==' ' || *total ==':' || *total =='\t'; )total ++;
         for(; *free  ==' ' || *free  ==':' || *free  =='\t'; )free  ++;
         for(; *cached==' ' || *cached==':' || *cached=='\t'; )cached++;
         CalcValue value;
         total =_SkipWhiteChars(TextValue(total , value)); Long t=value.asLong(); if(Starts(total , "tb"))t<<=40;else if(Starts(total , "gb"))t<<=30;else if(Starts(total , "mb"))t<<=20;else if(Starts(total , "kb"))t<<=10;
         free  =_SkipWhiteChars(TextValue(free  , value)); Long f=value.asLong(); if(Starts(free  , "tb"))f<<=40;else if(Starts(free  , "gb"))f<<=30;else if(Starts(free  , "mb"))f<<=20;else if(Starts(free  , "kb"))f<<=10;
         cached=_SkipWhiteChars(TextValue(cached, value)); Long c=value.asLong(); if(Starts(cached, "tb"))c<<=40;else if(Starts(cached, "gb"))c<<=30;else if(Starts(cached, "mb"))c<<=20;else if(Starts(cached, "kb"))c<<=10;
         total_phys=t;
         avail_phys=f+c;
         usage=(total_phys ? (total_phys-avail_phys)*100/total_phys : 0);
         avail_virt=avail_page=avail_phys;
         total_virt=total_page=total_phys;
         return true;
      }
   }
   Zero(T); return false;
#endif
}
/******************************************************************************/
static Char8 Base64Chars[]={'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                            'w', 'x', 'y', 'z', '0', '1', '2', '3',
                            '4', '5', '6', '7', '8', '9', '+', '/'};
Str8 Base64(CPtr data, Int size)
{
   Str8 temp; if(size>0)
   {
      temp.reserve((size+2)/3*4);
      Byte *d=(Byte*)data;
      Int mod=size%3;
      REP((size-mod)/3)
      {
         UInt a=*d++;
         UInt b=*d++;
         UInt c=*d++;
         UInt triple=(a<<0x10)+(b<<0x08)+c;
         temp+=Base64Chars[(triple>>3*6)&0x3F];
         temp+=Base64Chars[(triple>>2*6)&0x3F];
         temp+=Base64Chars[(triple>>1*6)&0x3F];
         temp+=Base64Chars[(triple>>0*6)&0x3F];
      }
      if(mod)
      {
         UInt a=            *d++;      // always
         UInt b=((mod==2) ? *d++ : 0); // conditional
         UInt triple=(a<<0x10)+(b<<0x08);
         temp+=                  Base64Chars[(triple>>3*6)&0x3F];
         temp+=                  Base64Chars[(triple>>2*6)&0x3F];
         temp+=((mod==1) ? '=' : Base64Chars[(triple>>1*6)&0x3F]);
         temp+=            '=';
      }
   }
   return temp;
}
/******************************************************************************/
// MEM
/******************************************************************************/
#if MEM_LEAK_FULL
struct AllocData
{
   Ptr data;
   Str call_stack;

   AllocData() {GetCallStackFast(call_stack);}
};
static SyncLock         MemInsideLock;
static Bool             MemInside;
static Memx<AllocData> *MemLog; // set this as a pointer so its dtor won't get automatically called
#endif

#if MEM_COUNT_USAGE
Long _MemUsage; // current memory usage
#endif

#if MEM_PEAK
Long _MemPeakUsage; // peak memory usage
#endif

#if MEM_SINGLE_MAX
IntPtr MaxSize;
#endif

Long MemUsage()
{
#if MEM_COUNT_USAGE
   return _MemUsage;
#else
   return 0;
#endif
}
Long MemPeakUsage()
{
#if MEM_PEAK
   return _MemPeakUsage;
#else
   return 0;
#endif
}
void ClearMemPeakUsage()
{
#if MEM_PEAK
   AtomicSet(_MemPeakUsage, AtomicGet(_MemUsage));
#endif
}

void ListMemLeaks()
{
#if MEM_LEAK_FULL
   SafeSyncLocker locker(MemInsideLock);
   MemInside=true;
   if(MemLog)FREPA(*MemLog)
   {
      LogN(S+"Leak #"+i);
      LogN((*MemLog)[i].call_stack);
      LogN();
   }
   MemInside=false;
#endif
}
static void AllocError(IntPtr size)
{
   Char8   temp[256];
   Char   error[512];
   Set   (error,  "Can't allocate "); Append(error, TextInt(size, temp, -1, 3));
   Append(error, " bytes of memory.\nApplication Type: "
   #if X64
      "64-bit"
   #else
      "32-bit"
   #endif
   );
   MemStats mem; if(mem.get())
   {
       //Append(error, "\nMemory Usage: "      ); Append(error, TextInt(mem.usage     , temp)); Append(error, "%");
         Append(error, "\nPhysical Available: "); Append(error, TextInt(mem.avail_phys, temp, -1, 3)); Append(error, " / "); Append(error, TextInt(mem.total_phys, temp, -1, 3));
         Append(error,  "\nVirtual Available: "); Append(error, TextInt(mem.avail_virt, temp, -1, 3)); Append(error, " / "); Append(error, TextInt(mem.total_virt, temp, -1, 3));
      if(mem.avail_page!=mem.avail_virt || mem.total_page!=mem.total_virt)
      {  Append(error,    "\nPaged Available: "); Append(error, TextInt(mem.avail_page, temp, -1, 3)); Append(error, " / "); Append(error, TextInt(mem.total_page, temp, -1, 3));
      }
   }
   ExitEx(error);
}
static INLINE void AllocInc() {AtomicInc(App._mem_leaks);}
static INLINE void AllocDec() {AtomicDec(App._mem_leaks);}
/******************************************************************************/
// FREE
/******************************************************************************/
static INLINE Byte MemGuard(Int offset) {return Byte(0xEE + offset);}
void Free(Ptr &data)
{
   if(data)
   {
   #if MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD
      UInt before=0; // extra size needed before alloc
   #endif

   #if MEM_GUARD
      Byte *guard=(Byte*)data-MEM_GUARD; data=guard;
   #endif

   #if MEM_COUNT_USAGE
      IntPtr data_size; {IntPtr *size=(IntPtr*)data-1; data=size; data_size=*size; AtomicSub(_MemUsage, data_size);}
      before+=SIZE(data_size); // make room for size
   #endif

   #if MEM_GUARD
      before+=MEM_GUARD; // make room for guard
      FREP(MEM_GUARD)if(*guard++!=MemGuard(i))Exit("Mem Guard was modified"); guard+=data_size;
      FREP(MEM_GUARD)if(*guard++!=MemGuard(i))Exit("Mem Guard was modified");
   #endif

   #if MEM_LEAK_FULL
      before+=SIZE(Int); // make room for leak data
      {
         Int *index=(Int*)data-1; data=index;
         SafeSyncLocker locker(MemInsideLock);
         if(!MemInside)
         {
            MemInside=true;
            App._mem_leaks--;
            { // braces to make sure locals are removed inside the block
               AllocData &ad=MemLog->absElm(*index);
               if(ad.data!=index)Exit("Invalid mem link");
               MemLog->removeAbs(*index);
            }
            MemInside=false;
         }
      }
   #else
      AllocDec();
   #endif

   #if MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD
      data=(Byte*)data+before-Ceil16(before); // we have processed only 'before' but we need to process 'Ceil16(before)', so go forward to what was processed, and go back by Ceil
   #endif

      free(data);
           data=null;
   }
}
/******************************************************************************/
// ALLOCATE
/******************************************************************************/
Ptr Alloc(IntPtr size)
{
   if(size>0)
   {
   #if MEM_SINGLE_MAX
   again:
      Long max=AtomicGet(MaxSize);
      if(size>max && !AtomicCAS(MaxSize, max, size))goto again;
   #endif

   #if MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD
      UInt before=0, after=0; // extra size needed before and after alloc
   #endif

   #if MEM_LEAK_FULL
      before+=SIZE(Int); // make room for leak data
   #endif

   #if MEM_COUNT_USAGE
      IntPtr data_size=size; // remember this before increasing the size
      before+=SIZE(data_size); // make room for size
   #endif

   #if MEM_GUARD
      before+=MEM_GUARD; after+=MEM_GUARD; // make room for guard
   #endif

   #if MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD
      before=Ceil16(before); // !! need to align 'before' because some codes (custom or in the system) make assumptions to malloc being aligned, for example without this, Apple platforms can crash !!
      size+=before+after;
   #endif

      if(Ptr data=malloc(size))
      {
      #if MEM_LEAK_FULL || MEM_COUNT_USAGE || MEM_GUARD
         data=(Byte*)data+before; // data that will be returned to the user
         Ptr debug=data; // start from 'data' and go back and write debug info
      #endif

      #if MEM_GUARD // needs to be processed first - written next to 'data'
         {Byte *guard=(Byte*)debug-MEM_GUARD; debug=guard; FREP(MEM_GUARD)*guard++=MemGuard(i); guard=(Byte*)data+data_size; FREP(MEM_GUARD)*guard++=MemGuard(i);}
      #endif

      #if MEM_COUNT_USAGE
         {
            IntPtr *size=(IntPtr*)debug-1; debug=size; *size=data_size; Long old_usage=AtomicAdd(_MemUsage, data_size);
         #if MEM_PEAK
            for(Long new_usage=old_usage+data_size; ; )
            {
               Long peak=AtomicGet(_MemPeakUsage);
               if(new_usage<=peak
               || AtomicCAS(_MemPeakUsage, peak, new_usage))break;
            }
         #endif
         }
      #endif

      #if MEM_LEAK_FULL
         {
            Int *index=(Int*)debug-1; debug=index; *index=-1;
            SafeSyncLocker locker(MemInsideLock);
            if(!MemInside)
            {
               App._mem_leaks++;
               MemInside=true;
               { // braces to make sure locals are removed inside the block
                  if(!MemLog)New(MemLog);
                  AllocData &ad=MemLog->New();
                  *index=MemLog->absIndex(&ad);
                  ad.data=index;
               }
               MemInside=false;
            }
         }
      #else
         AllocInc();
      #endif

         // test alignment to verify that EE codes haven't messed up required alignment by the system (in case some codes or the system make assumption about alignment)
      #if X64 || APPLE // 64-bit platforms and all Apple have 16-byte alignment
         DEBUG_ASSERT((UIntPtr(data)&15)==0, "Memory is not 16-byte aligned");
      #else // other platforms should have at least 8-byte alignment
         // https://msdn.microsoft.com/en-us/library/ycsb6wwf.aspx
         // http://www.gnu.org/software/libc/manual/html_node/Aligned-Memory-Blocks.html
         DEBUG_ASSERT((UIntPtr(data)& 7)==0, "Memory is not 8-byte aligned");
      #endif

         return data;
      }
      AllocError(size);
   }
   return null;
}
Ptr AllocZero(IntPtr size)
{
   Ptr    data=Alloc(size); Zero(data, size);
   return data;
}
/******************************************************************************/
// REALLOCATE
/******************************************************************************/
void _Realloc(Ptr &data, IntPtr size_new, IntPtr size_old)
{
   MAX(size_new, 0);
   MAX(size_old, 0);
   if(size_new!=size_old)
   {
      Ptr p=Alloc(size_new);
      Copy(p, data, Min(size_new, size_old)); // copy old data
      Free(data); data=p;
   }
}
void _ReallocZero(Ptr &data, IntPtr size_new, IntPtr size_old)
{
   MAX(size_new, 0);
   MAX(size_old, 0);
   if(size_new!=size_old)
   {
      Ptr p=Alloc(size_new);
      Copy(       p, data, Min(size_new, size_old)); // copy old data
      Zero((Byte*)p+size_old,  size_new- size_old ); // zero new data
      Free(data); data=p;
   }
}
/******************************************************************************/
// ALIGNED
/******************************************************************************/
#if WINDOWS && !X64 && !MEM_CUSTOM // Win 32-bit has only 8-byte alignment
Ptr AlignedAlloc(IntPtr size)
{
   if(size>0)
   {
      if(Ptr data=_aligned_malloc(size, 16))
      {
         AllocInc();
         return data;
      }
      AllocError(size);
   }
   return null;
}
void AlignedFree(Ptr &data)
{
   if(data)
   {
      AllocDec    ();
     _aligned_free(data);
                   data=null;
   }
}
#elif (LINUX || ANDROID) && !MEM_CUSTOM
Ptr AlignedAlloc(IntPtr size)
{
   if(size>0)
   {
      if(Ptr data=memalign(16, size))
      {
         AllocInc();
         return data;
      }
      AllocError(size);
   }
   return null;
}
void AlignedFree(Ptr &data)
{
   if(data)
   {
      AllocDec();
      free    (data);
               data=null;
   }
}
#elif (defined WII) || MEM_CUSTOM
typedef Byte AAOffs; // Byte is enough to store the 0..16 offset
Ptr AlignedAlloc(IntPtr size)
{
   if(size>0)
   {
      Int padd=15+SIZE(AAOffs); // 15 for alignment and AAOffs for offset to the original pointer
      if(Byte *original=Alloc<Byte>(size+padd))
      {
         Byte     *aligned=(Byte*)(UIntPtr(original+padd)&~15);
         ((AAOffs*)aligned)[-1]=aligned-original; // store offset to the original pointer
         return    aligned;
      }
   }
   return null;
}
void AlignedFree(Ptr &data)
{
   if(data)
   {
      Ptr  original=((Byte*)data)-((AAOffs*)data)[-1];
      Free(original);
      data=null;
   }
}
#else // PS3, XBox, Win64, Apple have 16-byte alignment
Ptr  AlignedAlloc(IntPtr size) {Ptr  data=Alloc(size); DEBUG_ASSERT((UIntPtr(data)&15)==0, "Memory is not 16-byte aligned"); return data;}
void AlignedFree (Ptr   &data) {Free(data);}
#endif
/******************************************************************************/
// ZERO
/******************************************************************************/
void Zero(Ptr data, IntPtr size)
{
   if(data && size>0)memset(data, 0, size);
}
/******************************************************************************/
// SET
/******************************************************************************/
void SetMem(Ptr data, Byte value, IntPtr size)
{
   if(data && size>0)memset(data, value, size);
}
/******************************************************************************/
// COPY
/******************************************************************************/
void Copy(Ptr dest, CPtr src, IntPtr size)
{
   if(dest && size>0)
   {
      if(src)memmove(dest, src, size);
      else   memset (dest,   0, size);
   }
}
void Copy(Ptr dest, CPtr src, IntPtr dest_size, IntPtr src_size)
{
   if(dest && dest_size>0)
   {
      if(!src || src_size<0)src_size=0;
      if(dest_size>src_size)
      {
         memmove(       dest         , src,           src_size); // copy what we have
         memset ((Byte*)dest+src_size,   0, dest_size-src_size); // zero what's left
      }else
      {
         memmove(       dest         , src, dest_size         ); // copy what we have
      }
   }
}
/******************************************************************************/
void _CopyIs(Ptr dest, CPtr src, C MemPtr<Bool> &is, UInt elm_size)
{
   if(dest && src)switch(elm_size)
   {
      case 1:
      {
         Byte *d=(Byte*)dest,
              *s=(Byte*)src ;
         FREPA(is){if(is[i])*d++=*s; s++;}
      }break;

      case 4:
      {
         UInt *d=(UInt*)dest,
              *s=(UInt*)src ;
         FREPA(is){if(is[i])*d++=*s; s++;}
      }break;

      default:
      {
         Byte *d=(Byte*)dest,
              *s=(Byte*)src ;
         FREPA(is){if(is[i]){CopyFast(d, s, elm_size); d+=elm_size;} s+=elm_size;}
      }break;
   }
}
/******************************************************************************/
void _CopyList(Ptr dest, CPtr src, C MemPtr<Int> &list, UInt elm_size)
{
   if(dest && src)switch(elm_size)
   {
      case 1:
      {
         Byte *d=(Byte*)dest,
              *s=(Byte*)src ;
         FREPA(list)*d++=s[list[i]];
      }break;

      case 4:
      {
         UInt *d=(UInt*)dest,
              *s=(UInt*)src ;
         FREPA(list)*d++=s[list[i]];
      }break;

      default:
      {
         Byte *d=(Byte*)dest,
              *s=(Byte*)src ;
         FREPA(list){CopyFast(d, s+list[i]*elm_size, elm_size); d+=elm_size;}
      }break;
   }
}
/******************************************************************************/
void Copy8To16(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U16 *d=(U16*)dest)
   if(U8  *s=(U8 *)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
      xor eax, eax
   start:
      lodsb
      stosw
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy8To32(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U32 *d=(U32*)dest)
   if(U8  *s=(U8 *)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
      xor eax, eax
   start:
      lodsb
      stosd
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy16To8(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U8  *d=(U8 *)dest)
   if(U16 *s=(U16*)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
   start:
      lodsw
      stosb
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy16To32(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U32 *d=(U32*)dest)
   if(U16 *s=(U16*)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
      xor eax, eax
   start:
      lodsw
      stosd
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy24To32(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U8 *d=(U8*)dest)
   if(U8 *s=(U8*)src )if(elms)
   {
      REP(elms-1)
      {
         *(U32*)d=*(U32*)s;
         d[3]=0;
         d+=4;
         s+=3;
      }
      *(U16*)d   =*(U16*)s   ;
             d[2]=       s[2];
             d[3]=          0;
   }
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
      dec ecx
      jz  last
   start:
      movsd
      mov byte ptr[edi-1], 0
      dec esi
      dec ecx
      jnz start
   last:
      movsw
      movsb
      mov byte ptr[edi], 0
   end:
   }
#endif
}
void Copy32To8(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U8  *d=(U8 *)dest)
   if(U32 *s=(U32*)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
   start:
      lodsd
      stosb
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy32To16(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U16 *d=(U16*)dest)
   if(U32 *s=(U32*)src )REP(elms)*d++=*s++;
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
   start:
      lodsd
      stosw
      dec ecx
      jnz start
   end:
   }
#endif
}
void Copy32To24(Ptr dest, CPtr src, Int elms)
{
#if X64 || ARM || !WINDOWS
   if(U8 *d=(U8*)dest)
   if(U8 *s=(U8*)src )if(elms)
   {
      REP(elms-1)
      {
         *(U32*)d=*(U32*)s;
         d+=3;
         s+=4;
      }
      *(U16*)d   =*(U16*)s   ;
             d[2]=       s[2];
   }
#else
  _asm
   {
      mov esi, src
      or  esi, esi
      jz  end
      mov edi, dest
      or  edi, edi
      jz  end
      mov ecx, elms
      or  ecx, ecx
      jle end // signed<=
      dec ecx
      jz  last
   start:
      movsd
      dec edi
      dec ecx
      jnz start
   last:
      movsw
      movsb
   end:
   }
#endif
}
/******************************************************************************/
// SWAP
/******************************************************************************/
void SwapFast(Ptr a, Ptr b, IntPtr size) // !! this will crash if "!a || !b || size<0" !!
{
   U32 *i1=(U32*) a, *i2=(U32*) b; REPP  (size>>2)Swap(*i1++, *i2++); // size>>2 is faster than Int size/4
   U8  *j1=(U8 *)i1, *j2=(U8 *)i2; switch(size& 3)                    // size& 3 is faster than Int size%4 but doesn't work with negative numbers
   {
      case 3: Swap(*j1++, *j2++); // !! no break on purpose !!
      case 2: Swap(*j1++, *j2++); // !! no break on purpose !!
      case 1: Swap(*j1++, *j2++); // !! no break on purpose !!
   }
}
void Swap(Ptr a, Ptr b, IntPtr size)
{
   if(a && b && a!=b && size>0)
   {
      U32 *i1=(U32*) a, *i2=(U32*) b; REPP  (size>>2)Swap(*i1++, *i2++);
      U8  *j1=(U8 *)i1, *j2=(U8 *)i2; switch(size& 3)
      {
         case 3: Swap(*j1++, *j2++); // !! no break on purpose !!
         case 2: Swap(*j1++, *j2++); // !! no break on purpose !!
         case 1: Swap(*j1++, *j2++); // !! no break on purpose !!
      }
   }
}
void _ReverseOrder(Ptr data, Int elms, UInt elm_size)
{
   if(elms>0)
   {
      Ptr end=(Byte*)data+(elms-1)*elm_size;
      elms>>=1;
      switch(elm_size)
      {
         case 1:
            REP(elms)
            {
               Swap(*(U8*)data, *(U8*)end);
               data=(U8*)data+1;
               end =(U8*)end -1;
            }
         break;

         case 4:
            REP(elms)
            {
               Swap(*(U32*)data, *(U32*)end);
               data=(U32*)data+1;
               end =(U32*)end -1;
            }
         break;

         case 8:
            REP(elms)
            {
               Swap(*(U64*)data, *(U64*)end);
               data=(U64*)data+1;
               end =(U64*)end -1;
            }
         break;

         default:
            REP(elms)
            {
               Swap(data, end, elm_size);
               data=(Byte*)data+elm_size;
               end =(Byte*)end -elm_size;
            }
         break;
      }
   }
}
void _RandomizeOrder(Ptr data, Int elms, UInt elm_size, Randomizer &random)
{
   FREP(elms-1)Swap((Byte*)data+i*elm_size, (Byte*)data+random(i, elms-1)*elm_size, elm_size);
}
void _RotateOrder(Ptr data, Int elms, UInt elm_size, Int offset)
{
   if(elms>1)
   {
         offset%=elms;
      if(offset)
      {
         if(offset<0)offset+=elms;

         Int        offset_reversed=elms-offset;
         UInt       required_size  =Min(offset, offset_reversed)*elm_size;
         Memt<Byte> buf; buf.setNum(required_size); Ptr temp=buf.data();
         Byte      *dest=(Byte*)data;

         if(offset<=offset_reversed) // ROR
         {
            // offset=2, offset_reversed=8
            // 0123456789 - sample data
            // 89         - temp
            //   01234567 - copy
            CopyFast(temp                , dest+offset_reversed*elm_size, offset         *elm_size); // copy second part into temp (here "89"      )
            MoveFast(dest+offset*elm_size, dest                         , offset_reversed*elm_size); // move first  part right     (here "01234567")
            CopyFast(dest                , temp                         , offset         *elm_size); // copy temp   to   start     (here "89"      )
         }
         else // ROL
         {
            // offset=8, offset_reversed=2
            // 0123456789 - sample data
            //         01 - temp
            // 23456789   - copy
            CopyFast(temp                , dest                         , offset_reversed*elm_size); // copy first  part into temp (here "01"      )
            MoveFast(dest                , dest+offset_reversed*elm_size, offset         *elm_size); // move second part left      (here "01234567")
            CopyFast(dest+offset*elm_size, temp                         , offset_reversed*elm_size); // copy temp   to   end       (here "89"      )
         }
      }
   }
}
void _MoveElm(Ptr data, Int elms, UInt elm_size, Int elm, Int new_index) // move 'elm' element and preserve order of other elements, 01X2 -> X012
{
   if(InRange(elm, elms))
   {
      Clamp(new_index, 0, elms-1); if(new_index!=elm)
      {
         Memt<Byte> buf; buf.setNum(elm_size); Ptr temp=buf.data();
         Byte      *d=(Byte*)data;
         CopyFast(temp, d+elm*elm_size, elm_size); // copy element from data to temp memory
                           //                                                                                       E N        E N
         if(elm<new_index) // element is on the left, and we're moving it to the right, move the data to the left "0X123" -> "012X3"
         {
            MoveFast(d+elm*elm_size, d+(elm+1)*elm_size, (new_index-elm)*elm_size);
         }    //                                                                                        N E        N E
         else // element is on the right, and we're moving it to the left, move the data to the right "012X3" -> "0X123"
         {
            MoveFast(d+(new_index+1)*elm_size, d+new_index*elm_size, (elm-new_index)*elm_size);
         }
         CopyFast(d+new_index*elm_size, temp, elm_size); // copy element from temp memory back to the data
      }
   }
}
void _MoveElmLeftUnsafe(Ptr data, UInt elm_size, Int elm, Int new_index, Ptr temp) // !! assumes indexes are in range, "elm>=new_index", 'temp' can fit 'elm_size' !!
{
   if(new_index!=elm)
   {
      Byte *d=(Byte*)data;
      CopyFast(temp, d+elm*elm_size, elm_size); // copy element from data to temp memory
   #if 0 // not needed since we're always moving left in this function
                        //                                                                                       E N        E N
      if(elm<new_index) // element is on the left, and we're moving it to the right, move the data to the left "0X123" -> "012X3"
      {
         MoveFast(d+elm*elm_size, d+(elm+1)*elm_size, (new_index-elm)*elm_size);
      }    //                                                                                        N E        N E
      else // element is on the right, and we're moving it to the left, move the data to the right "012X3" -> "0X123"
   #endif
      {
         MoveFast(d+(new_index+1)*elm_size, d+new_index*elm_size, (elm-new_index)*elm_size);
      }
      CopyFast(d+new_index*elm_size, temp, elm_size); // copy element from temp memory back to the data
   }
}
/******************************************************************************/
// COMPARE
/******************************************************************************/
Bool EqualMem(CPtr a, CPtr b, IntPtr size)
{
   if(size>0 && a!=b)return (a && b) ? !memcmp(a, b, size) : false;
   return true;
}
/******************************************************************************/
}
/******************************************************************************/
Ptr  operator new   (size_t size)         {return Alloc(size);}
void operator delete(Ptr    data)noexcept {       Free (data);}
/******************************************************************************/
