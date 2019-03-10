/******************************************************************************/
typedef                       bool               Bool       ; // Boolean Value (8-bit) false/true

typedef                       char               Char8      ; // Character ( 8-bit)
typedef                       char16_t           Char       ; // Character (16-bit)

typedef   PLATFORM(  signed __int8 ,   int8_t)   I8 ,  SByte; //   Signed Int ( 8-bit)                        -128 .. 127
typedef   PLATFORM(unsigned __int8 ,  uint8_t)   U8 ,   Byte; // Unsigned Int ( 8-bit)                           0 .. 255
typedef   PLATFORM(  signed __int16,  int16_t)   I16,  Short; //   Signed Int (16-bit)                     -32 768 .. 32 767
typedef   PLATFORM(unsigned __int16, uint16_t)   U16, UShort; // Unsigned Int (16-bit)                           0 .. 65 535
typedef   PLATFORM(  signed __int32,  int32_t)   I32,    Int; //   Signed Int (32-bit)              -2 147 483 648 .. 2 147 483 647
typedef   PLATFORM(unsigned __int32, uint32_t)   U32,   UInt; // Unsigned Int (32-bit)                           0 .. 4 294 967 295
typedef   PLATFORM(  signed __int64,  int64_t)   I64,   Long; //   Signed Int (64-bit)  -9 223 372 036 854 775 808 ..  9 223 372 036 854 775 807
typedef   PLATFORM(unsigned __int64, uint64_t)   U64,  ULong; // Unsigned Int (64-bit)                           0 .. 18 446 744 073 709 551 615

typedef                       float              F32, Flt   ; //  Float (32-bit)  1-bit sign +  8-bit exponent + 23-bit fraction
typedef                       double             F64, Dbl   ; // Double (64-bit)  1-bit sign + 11-bit exponent + 52-bit fraction

typedef                       void              *Ptr        ; // General Pointer               (32-bit or 64-bit)
typedef                 const void              *CPtr       ; // General Pointer to const data (32-bit or 64-bit)
/******************************************************************************/
typedef   const Char8   CChar8; // Const Char8
typedef   const Char    CChar ; // Const Char
/******************************************************************************/
#if X64
   typedef I64  IntPtr; //   Signed Int capable of storing full memory address (needs to be 64-bit on 64-bit platforms)
   typedef U64 UIntPtr; // Unsigned Int capable of storing full memory address (needs to be 64-bit on 64-bit platforms)
#else
   typedef I32  IntPtr; //   Signed Int capable of storing full memory address (needs to be 32-bit on 32-bit platforms)
   typedef U32 UIntPtr; // Unsigned Int capable of storing full memory address (needs to be 32-bit on 32-bit platforms)
#endif
/******************************************************************************/
#undef   SHORT_MIN
#define  SHORT_MIN (-0x7FFF-1)             // Minimum possible value of 16-bit   signed int ( Short)
#undef   SHORT_MAX
#define  SHORT_MAX   0x7FFF                // Maximum possible value of 16-bit   signed int ( Short)
#undef  USHORT_MAX
#define USHORT_MAX   0xFFFFu               // Maximum possible value of 16-bit unsigned int (UShort)
#undef     INT_MIN
#define    INT_MIN (-0x7FFFFFFF-1)         // Minimum possible value of 32-bit   signed int ( Int  )
#undef     INT_MAX
#define    INT_MAX   0x7FFFFFFF            // Maximum possible value of 32-bit   signed int ( Int  )
#undef    UINT_MAX
#define   UINT_MAX   0xFFFFFFFFu           // Maximum possible value of 32-bit unsigned int (UInt  )
#undef   INT24_MIN
#define  INT24_MIN (-0x7FFFFF-1)           // Minimum possible value of 24-bit   signed int ( Int24)
#undef   INT24_MAX
#define  INT24_MAX   0x7FFFFF              // Maximum possible value of 24-bit   signed int ( Int24)
#undef  UINT24_MAX
#define UINT24_MAX   0xFFFFFFu             // Maximum possible value of 24-bit unsigned int (UInt24)
#undef    LONG_MIN
#define   LONG_MIN (-0x7FFFFFFFFFFFFFFF-1) // Minimum possible value of 64-bit   signed int ( Long )
#undef    LONG_MAX
#define   LONG_MAX   0x7FFFFFFFFFFFFFFF    // Maximum possible value of 64-bit   signed int ( Long )
#undef   ULONG_MAX
#define  ULONG_MAX   0xFFFFFFFFFFFFFFFFu   // Maximum possible value of 64-bit unsigned int (ULong )
#undef    HALF_MIN
#define   HALF_MIN 0.00006103515625f       // Minimum positive value of 16-bit real         (Half )
#undef    HALF_MAX
#define   HALF_MAX 65504                   // Maximum possible value of 16-bit real         (Half )
#undef    HALF_EPS
#define   HALF_EPS 0.0009765625f           // smallest such that 1.0h+HALF_EPS != 1.0h
#undef     FLT_MAX
#define    FLT_MAX 3.402823466e+38f        // Maximum possible value of 32-bit real         ( Flt )
#undef     FLT_EPS
#define    FLT_EPS 1.192092896e-7f         // smallest such that 1.0f+FLT_EPS != 1.0f
#undef     DBL_MAX
#define    DBL_MAX 1.7976931348623158e+308 // Maximum possible value of 64-bit real         ( Dbl )
/******************************************************************************/
