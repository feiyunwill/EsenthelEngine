/******************************************************************************

   Use 'Cpu' to check your cpu capabilities.

/******************************************************************************/
enum CPU_FLAG // CPU Flags
{
   CPU_MMX   =0x001, // if MMX     is supported
   CPU_3DNOW =0x002, // if 3DNow   is supported
   CPU_SSE   =0x004, // if SSE     is supported
   CPU_SSE2  =0x008, // if SSE 2   is supported
   CPU_SSE3  =0x010, // if SSE 3   is supported
   CPU_SSE4_1=0x020, // if SSE 4.1 is supported
   CPU_SSE4_2=0x040, // if SSE 4.2 is supported
   CPU_AVX   =0x080, // if AVX     is supported
   CPU_AVX2  =0x100, // if AVX 2   is supported
   CPU_AES   =0x200, // if AES     is supported
};
/******************************************************************************/
struct CPU // Central Processing Unit
{
   // get
   Int   threads()C {return _threads;} // number of hardware threads
   UInt  flag   ()C {return _flag   ;} // get CPU_FLAG
 C Str8& name   ()C {return _name   ;} // get cpu name

#if EE_PRIVATE
   static void set();
#endif

#if !EE_PRIVATE
private:
#endif
   Int  _threads;
   UInt _flag;
   Str8 _name;

   CPU();
}extern
   Cpu;
/******************************************************************************/
inline Int Elms(C CPU &cpu) {return cpu.threads();}
/******************************************************************************/
