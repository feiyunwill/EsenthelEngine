#ifdef _WIN32
   typedef   unsigned __int64   U64;
   typedef     signed __int64   I64;
   typedef     signed __int32   I32;
#else
   #include <stdint.h>
   typedef   uint64_t           U64;
   typedef    int64_t           I64;
   typedef    int32_t           I32;
#endif

#if defined _WIN64 || defined __LP64__
   typedef I64 IntPtr; // Signed Int capable of storing full memory address (needs to be 64-bit on 64-bit platforms)
#else
   typedef I32 IntPtr; // Signed Int capable of storing full memory address (needs to be 32-bit on 32-bit platforms)
#endif

typedef void *Ptr;

namespace EE
{
   extern Ptr  Alloc(IntPtr size);
   extern void Free (Ptr    &ptr);
}
