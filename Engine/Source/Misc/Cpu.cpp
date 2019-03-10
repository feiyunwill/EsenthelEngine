/******************************************************************************/
#include "stdafx.h"
/*#if ANDROID
   #include <thread>
   #include <cpu-features.h>
#endif*/
namespace EE{
/******************************************************************************/
CPU Cpu;
/******************************************************************************/
void CPU::set()
{
   // disable denormals (multiplying denormals on Intel i7-3632QM 2.2 Ghz is 16x slower compared to normal values)
#if !(WINDOWS && ARM) && !IOS && !ANDROID && !WEB
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
}
#if MAC
void __cpuid(int regs[4], int cpuid_leaf)
{
	int eax, ebx, ecx, edx;
	asm(
#if !X64
   "pushl %%ebx;\n\t"
#endif
   "movl %4, %%eax;\n\t"
   "cpuid;\n\t"
   "movl %%eax, %0;\n\t"
   "movl %%ebx, %1;\n\t"
   "movl %%ecx, %2;\n\t"
   "movl %%edx, %3;\n\t"
#if !X64
   "popl %%ebx;\n\t"
#endif
   :"=m" (eax), "=m" (ebx), "=m" (ecx), "=m" (edx)
   :"r" (cpuid_leaf)
   :"%eax",
#if X64
   "%ebx",
#endif
   "%ecx", "%edx");

   regs[0]=eax;
   regs[1]=ebx;
   regs[2]=ecx;
   regs[3]=edx;
}
#elif LINUX
   #undef   __cpuid
static void __cpuid(int regs[4], int cpuid_leaf)
{
   __get_cpuid(cpuid_leaf, (unsigned int*)&regs[0], (unsigned int*)&regs[1], (unsigned int*)&regs[2], (unsigned int*)&regs[3]);
}
#elif ANDROID
static ULong GetBits(CChar8 *text) // sample: "0,1-3"
{
   ULong   out=0;
   CalcValue val;
   for(; text; )
   {
      text=TextValue(text, val); if(!val.type)break;
      Int a=val.asInt();
      if(text && *text=='-') // range
      {
         text=TextValue(text+1, val); if(!val.type)break; // skip '-'
         Int b=val.asInt();
         for(Int i=a; i<=b; i++)out|=(ULong(1)<<i);
      }else // single value
      {
         out|=(ULong(1)<<a);
      }
      if(text && *text==',')text++; // skip comma and proceed to next value
   }
   return out;
}
#endif
CPU::CPU()
{
   if(LogInit)LogN("CPU.create");
#if WINDOWS
  _flag|=(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE   ) ? CPU_MMX   : 0);
  _flag|=(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE ) ? CPU_3DNOW : 0);
  _flag|=(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE  ) ? CPU_SSE   : 0);
  _flag|=(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) ? CPU_SSE2  : 0);
  _flag|=(IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE  ) ? CPU_SSE3  : 0);

   #if WINDOWS_OLD
      DWORD_PTR process, system; GetProcessAffinityMask(GetCurrentProcess(), &process, &system);
      REP(64)if(process&(ULong(1)<<i))_threads++;
   #elif WINDOWS_NEW
      SYSTEM_INFO sys_info; GetSystemInfo(&sys_info); _threads=sys_info.dwNumberOfProcessors;
   #endif
#elif LINUX
  _threads=sysconf(_SC_NPROCESSORS_CONF);
#elif APPLE
   #if 1
     _threads=sysconf(_SC_NPROCESSORS_CONF);
   #else
      size_t len=SIZE(ncpus); sysctl((int[2]){CTL_HW, HW_NCPU}, 2, &_threads, &len, null, 0);
   #endif
#endif

#if (WINDOWS && !ARM) || MAC || LINUX
   Int CPUInfo[4];

   // constants taken from cpuid.h
   __cpuid(CPUInfo, 1);
   if(CPUInfo[3]&(1<<23))_flag|=CPU_MMX;
   if(CPUInfo[3]&(1<<25))_flag|=CPU_SSE;
   if(CPUInfo[3]&(1<<26))_flag|=CPU_SSE2;
   if(CPUInfo[2]&(1<< 0))_flag|=CPU_SSE3;
   if(CPUInfo[2]&(1<<19))_flag|=CPU_SSE4_1;
   if(CPUInfo[2]&(1<<20))_flag|=CPU_SSE4_2;
   if(CPUInfo[2]&(1<<28))_flag|=CPU_AVX;
   if(CPUInfo[2]&(1<<25))_flag|=CPU_AES;

   __cpuid(CPUInfo, 7);
   if(CPUInfo[1]&(1<<5))_flag|=CPU_AVX2;

   __cpuid(CPUInfo, 0x80000001);
   if(CPUInfo[3]&(1<<31))_flag|=CPU_3DNOW;
   
   char string[64]; Zero(string);
   __cpuid(CPUInfo, 0x80000000); UInt ids=CPUInfo[0];
   for(UInt i=0x80000002; i<=Min(ids, 0x80000004); i++)
   {
      __cpuid(CPUInfo, i);
      if(i==0x80000002)CopyFast(string   , CPUInfo, SIZE(CPUInfo));else
      if(i==0x80000003)CopyFast(string+16, CPUInfo, SIZE(CPUInfo));else
      if(i==0x80000004)CopyFast(string+32, CPUInfo, SIZE(CPUInfo));
   }
  _name=_SkipWhiteChars(string);
#elif ANDROID
   Char8 data[65536];

/* Sample output for Galaxy Note 4
      processor	: 0
      model name	: ARMv7 Processor rev 1 (v7l)
      BogoMIPS	: 52.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x0
      CPU part	: 0xd03
      CPU revision	: 1
      processor	: 1
      model name	: ARMv7 Processor rev 1 (v7l)
      BogoMIPS	: 52.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x0
      CPU part	: 0xd03
      CPU revision	: 1
      processor	: 2
      model name	: ARMv7 Processor rev 1 (v7l)
      BogoMIPS	: 52.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x0
      CPU part	: 0xd03
      CPU revision	: 1
      processor	: 3
      model name	: ARMv7 Processor rev 1 (v7l)
      BogoMIPS	: 52.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x0
      CPU part	: 0xd03
      CPU revision	: 1
      processor	: 4
      model name	: ARMv7 Processor rev 0 (v7l)
      BogoMIPS	: 76.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x1
      CPU part	: 0xd07
      CPU revision	: 0
      processor	: 5
      model name	: ARMv7 Processor rev 0 (v7l)
      BogoMIPS	: 76.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x1
      CPU part	: 0xd07
      CPU revision	: 0
      processor	: 6
      model name	: ARMv7 Processor rev 0 (v7l)
      BogoMIPS	: 76.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x1
      CPU part	: 0xd07
      CPU revision	: 0
      processor	: 7
      model name	: ARMv7 Processor rev 0 (v7l)
      BogoMIPS	: 76.00
      Features	: swp half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
      CPU implementer	: 0x41
      CPU architecture: 7
      CPU variant	: 0x1
      CPU part	: 0xd07
      CPU revision	: 0
      Hardware	: Samsung EXYNOS5433
      Revision	: 000d
      Serial		: 5b8165e4c0680041
*/
   // get name
   if(UnixReadFile("/proc/cpuinfo", data, SIZE(data)))
   {
    //LogN(data);
      CChar8 *t;
      if(t=TextPos(data, "Hardware" , false, true))t+=8;else // Length("Hardware" ) -> 8
      if(t=TextPos(data, "Processor", false, true))t+=9;     // Length("Processor") -> 9, if "Hardware" not available, then try using "Processor", because in the past sample output was: "Processor: ARMv7 Processor rev 9 (v7l)"
      if(t)
      {
         for(; *t==' '  || *t==':' || *t=='\t'; )        t++; // skip spaces
         for(; *t!='\0' && *t!='\n'           ; )_name+=*t++;
      }
   }

   // get threads - do not rely on '_SC_NPROCESSORS_CONF' or '_SC_NPROCESSORS_ONLN' as they both may return not all threads (if some are switched off due to power saving), 'sysctl' is not available on Android
   UnixReadFile("/sys/devices/system/cpu/present" , data, SIZE(data)); ULong cpu_present =GetBits(data); // sample output: "0-3", "0,1-2" (no spaces, values separated with commas, ranges or single values, ranges inclusive)
   UnixReadFile("/sys/devices/system/cpu/possible", data, SIZE(data)); ULong cpu_possible=GetBits(data);
   ULong mask=(cpu_present&cpu_possible); REP(64)if(mask&(ULong(1)<<i))_threads++;
   MAX(_threads, sysconf(_SC_NPROCESSORS_CONF)); // just in case code above was unsuccessful
 //LogN(S+"cpu_present:"+TextBin(cpu_present)+" cpu_possible:"+TextBin(cpu_possible)+" mask:"+TextBin(mask)+", sysconf(_SC_NPROCESSORS_CONF):"+(Int)sysconf(_SC_NPROCESSORS_CONF)+", android_getCpuCount:"+(Int)android_getCpuCount()+", std::thread::hardware_concurrency:"+(Int)std::thread::hardware_concurrency());
#endif

   MAX(_threads, 1);
   set();
}
/******************************************************************************/
}
/******************************************************************************/
