/******************************************************************************/
// DEFINITIONS
/******************************************************************************/
#undef  NULL
#define NULL   0
#define C      const
#define T      (*this)
#define null   nullptr
#define null_t std::nullptr_t
#if WINDOWS
   #define super __super
#endif
/******************************************************************************/
// TEMPLATE MACROS
/******************************************************************************/
#define T1(a      )   template<typename a                        > // 1 type  template
#define T2(a, b   )   template<typename a, typename b            > // 2 types template
#define T3(a, b, c)   template<typename a, typename b, typename c> // 3 types template
#if EE_PRIVATE
#define T4(a, b, c, d               ) template<typename a, typename b, typename c, typename d                                                            > // 4 types template
#define T5(a, b, c, d, e            ) template<typename a, typename b, typename c, typename d, typename e                                                > // 5 types template
#define T6(a, b, c, d, e, f         ) template<typename a, typename b, typename c, typename d, typename e, typename f                                    > // 6 types template
#define T7(a, b, c, d, e, f, g      ) template<typename a, typename b, typename c, typename d, typename e, typename f, typename g                        > // 7 types template
#define T8(a, b, c, d, e, f, g, h   ) template<typename a, typename b, typename c, typename d, typename e, typename f, typename g, typename h            > // 8 types template
#define T9(a, b, c, d, e, f, g, h, i) template<typename a, typename b, typename c, typename d, typename e, typename f, typename g, typename h, typename i> // 9 types template
#endif
/******************************************************************************/
// HELPER MACROS
/******************************************************************************/
#define SIZE                                    sizeof               // get raw size of C++ element in bytes
#define MEMBER(     Class, member)        (((Class*)null)-> member)  // null based Class::member, this macro is used to obtain member information by many other macros/functions
#define OFFSET(     Class, member)  UIntPtr(&MEMBER(Class,  member)) // get offset   of member in class
#define MEMBER_SIZE(Class, member)     SIZE( MEMBER(Class,  member)) // get size     of member in class
#define MEMBER_ELMS(Class, member)     Elms( MEMBER(Class,  member)) // get elements of member in class
#define  CAST(      Class, object)     dynamic_cast<Class*>(object)  // perform a dynamic cast of 'object' to 'Class' class
#define SCAST(      Class, object)      static_cast<Class&>(object)  // perform a  static cast of 'object' to 'Class' class
#define ELMS(       Array        )      (SIZE(Array)/SIZE(Array[0])) // get number of elements in array (this is the compile-time version, use 'Elms' instead of 'ELMS' whenever possible)

T1(TYPE) TYPE& ConstCast(C TYPE &x) {return const_cast<TYPE&>(x);} // remove the const modifier
T1(TYPE) TYPE* ConstCast(C TYPE *x) {return const_cast<TYPE*>(x);} // remove the const modifier

T1(TYPE)   TYPE& NoTemp(  TYPE &x) {return x;}
T1(TYPE) C TYPE& NoTemp(C TYPE &x) {return x;}

T1(TYPE) TYPE&& RValue(TYPE &type) {return (TYPE&&)type;}
/******************************************************************************/
// ITERATION MACROS
/******************************************************************************/
#define    REP(    n)  for(Int i=(n); --i>= 0 ;    ) //         repeat                : n-1 .. 0
#define    REPD(i, n)  for(Int i=(n); --i>= 0 ;    ) //         repeat with definition: n-1 .. 0
#define   FREP(    n)  for(Int i= 0 ;   i< (n); i++) // forward repeat                :   0 .. n-1
#define   FREPD(i, n)  for(Int i= 0 ;   i< (n); i++) // forward repeat with definition:   0 .. n-1

#define   REPA(    a)  for(Int i=Elms(a); --i>=     0 ;    ) //         repeat all                : Elms(a)-1 .. 0
#define   REPAD(i, a)  for(Int i=Elms(a); --i>=     0 ;    ) //         repeat all with definition: Elms(a)-1 .. 0
#define  FREPA(    a)  for(Int i=     0 ;   i< Elms(a); i++) // forward repeat all                :         0 .. Elms(a)-1
#define  FREPAD(i, a)  for(Int i=     0 ;   i< Elms(a); i++) // forward repeat all with definition:         0 .. Elms(a)-1

#define  REPAO(    a)  for(Int i=Elms(a); --i>=     0 ;    ) (a)[i] //         repeat all                 and operate: Elms(a)-1 .. 0
#define  REPAOD(i, a)  for(Int i=Elms(a); --i>=     0 ;    ) (a)[i] //         repeat all with definition and operate: Elms(a)-1 .. 0
#define FREPAO(    a)  for(Int i=     0 ;   i< Elms(a); i++) (a)[i] // forward repeat all                 and operate:         0 .. Elms(a)-1
#define FREPAOD(i, a)  for(Int i=     0 ;   i< Elms(a); i++) (a)[i] // forward repeat all with definition and operate:         0 .. Elms(a)-1

#define   REPS( i, n)  for((i)=    (n); --(i)>= 0 ;      ) //         repeat     with i specified:      n -1 .. 0
#define  FREPS( i, n)  for((i)=     0 ;   (i)< (n); (i)++) // forward repeat     with i specified:         0 .. n-1
#define   REPAS(i, a)  for((i)=Elms(a); --(i)>= 0 ;      ) //         repeat all with i specified: Elms(a)-1 .. 0

#if EE_PRIVATE
   #define REPP(n)  for(IntPtr i=(n); --i>=0; ) // repeat: n-1 .. 0
#endif
/******************************************************************************/
// ENUM MACROS
/******************************************************************************/
#define  ENABLE_IF_ENUM(ENUM, RESULT) typename std::enable_if< std::is_enum<ENUM>::value, RESULT>::type
#define DISABLE_IF_ENUM(ENUM, RESULT) typename std::enable_if<!std::is_enum<ENUM>::value, RESULT>::type

T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Int) operator+ (Bool a, TYPE b) {return a+Int(b);}
T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Int) operator+ (Int  a, TYPE b) {return a+Int(b);}
T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Flt) operator+ (Flt  a, TYPE b) {return a+Int(b);}
T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Int) operator+ (TYPE a, Bool b) {return Int(a)+b;}
T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Int) operator+ (TYPE a, Int  b) {return Int(a)+b;}
T1(TYPE) constexpr ENABLE_IF_ENUM(TYPE, Flt) operator+ (TYPE a, Flt  b) {return Int(a)+b;}

T2(ENUM0, ENUM1) constexpr typename std::enable_if< std::is_enum<ENUM0>::value && std::is_enum<ENUM1>::value, Int>::type operator+ (ENUM0 a, ENUM1 b) {return Int(a)+Int(b);} // ENUM0+ENUM1
/******************************************************************************/
// ASSERTIONS
/******************************************************************************/
#define ASSERT_CONCAT2(a, b) a##b                 // don't use this
#define ASSERT_CONCAT( a, b) ASSERT_CONCAT2(a, b) // don't use this

#define          ASSERT(value        )   typedef Int ASSERT_CONCAT(_AssertDummyName, __LINE__)[(value) ? 1 : -1]    // compile time assertion, alternative to static_assert(value, "assert failed"); which is more flexible on Clang/GCC
#define  DYNAMIC_ASSERT(value, error )   {if(!(value))Exit(S+(error)+"\nFile: \""+__FILE__+"\"\nLine: "+__LINE__);} // dynamic      assertion
#if DEBUG
   #define DEBUG_ASSERT(value, error )   DYNAMIC_ASSERT(value, error)                                               // debug        assertion   available only in debug   mode
#else
   #define DEBUG_ASSERT(value, error )   {}                                                                         // debug        assertion unavailable      in release mode
#endif
#define    RANGE_ASSERT(index, elms  )   DEBUG_ASSERT(InRange(index, elms), "Element out of range")                 // out of range assertion, asserts that 'index' is in range "0..elms-1"
#define    ALIGN_ASSERT(Class, member)   ASSERT(!(OFFSET(Class, member)&(SIZE(Ptr)-1)))                             // assert that class member has alignment native to the target platform

ASSERT(SIZE(Bool)==1); // size of Bool must be 1 byte
ASSERT(SIZE(Char)==2); // size of Char must be 2 bytes
/******************************************************************************/
// STRUCT DECLARATION
/******************************************************************************/
#define const_mem_addr // custom keyword specifying that the struct/class must be stored in constant memory address, if you see this keyword next to a struct/class declaration you must ensure that when defining objects of that struct/class you will store them in constant memory address (this can be either global namespace or inside 'Memx' 'Meml' containers)

#define STRUCT(        Extended, Base       )   struct Extended :         Base        { PLATFORM(, typedef Base super;) // macro for declaring an 'Extended' class from 'Base'            , and declaring 'super' keyword at the same time (this is needed for non Windows platforms which don't declare 'super' automatically)
#define STRUCT_PRIVATE(Extended, Base       )   struct Extended : private Base        { PLATFORM(, typedef Base super;) // macro for declaring an 'Extended' class from 'Base'            , and declaring 'super' keyword at the same time (this is needed for non Windows platforms which don't declare 'super' automatically)
#define STRUCT2(       Extended, Base, Base1)   struct Extended :         Base, Base1 { PLATFORM(, typedef Base super;) // macro for declaring an 'Extended' class from 'Base' and 'Base1', and declaring 'super' keyword at the same time (this is needed for non Windows platforms which don't declare 'super' automatically)

#define NO_COPY_CONSTRUCTOR(Class)       \
    void operator=(C Class &src)=delete; \
             Class(C Class &src)=delete; // when declared inside a class this macro disables the use of copy constructors

// alignment
#undef  ALIGN
#define ALIGN(    x) PLATFORM(__declspec(align(x)), )
#define ALIGN_END(x) PLATFORM(, __attribute__((aligned(x))))
/******************************************************************************/
// FUNCTION DECLARATION
/******************************************************************************/
#if WINDOWS
   #define INLINE         __forceinline  // force   inlining, this is stronger than 'inline'
   #define NOINLINE __declspec(noinline) // disable inlining
#else
   #define INLINE inline __attribute__((always_inline)) // force   inlining, this is stronger than 'inline'
   #define NOINLINE      __attribute__((     noinline)) // disable inlining
#endif
/******************************************************************************/
// CONFIGURATION
/******************************************************************************/
// Rendering
#define SUPPORT_EARLY_Z              (!MOBILE) // disable on Mobile because it's discouraged by PowerVR, Mali, ..
#define SUPPORT_MATERIAL_AMBIENT             1
#define COUNT_MATERIAL_USAGE                 0 // never use "DEBUG" here, because it affects Material class size/members, which needs to remain constant
#define CACHE_DX9_CONSTANTS                  0 // enabling didn't make any performance difference, so disable to reduce memory usage
#define SUPPORT_MATERIAL_CHANGE_IN_RENDERING 0
#define SUPPORT_MLAA                         0

// Compression
#define SUPPORT_RLE    (!WEB)
#define SUPPORT_SNAPPY (!WEB)
#define SUPPORT_LZ4    1
#define SUPPORT_ZLIB   (!WEB)
#define SUPPORT_ZSTD   1
#define SUPPORT_LZHAM  1
#define SUPPORT_LZMA   1

// Audio
#define SUPPORT_FLAC       1
#define SUPPORT_VORBIS     1
#define SUPPORT_VORBIS_ENC 1 // isn't going to be linked unless used
#define SUPPORT_OPUS       1
#define SUPPORT_OPUS_ENC   1
#define SUPPORT_MP3        1

// Image
#define SUPPORT_JPG  1
#define SUPPORT_PNG  1
#define SUPPORT_PSD  (!WEB)
#define SUPPORT_TIF  (!WEB)
#define SUPPORT_WEBP 1

// Video
#define SUPPORT_THEORA 1
#define SUPPORT_VP     (WINDOWS || MAC || LINUX || ANDROID || (IOS && !IOS_SIMULATOR))

// Database
#define SUPPORT_SQLITE 1 // isn't going to be linked unless used
#define SUPPORT_ODBC   (DESKTOP && !WINDOWS_NEW)
/******************************************************************************/
#if EE_PRIVATE
   #define MAX_LONG_PATH 1024
   #define MAX_UTF_PATH  2048

   #define SIZEI(x)  Int(SIZE(x)) // get size of element
   #define SIZEU(x) UInt(SIZE(x)) // get size of element

   #if LINUX
      #define FIND_ATOM(x) x=XInternAtom(XDisplay, #x, true ) // null on fail
      #define  GET_ATOM(x) x=XInternAtom(XDisplay, #x, false) // New  on fail
   #endif

   T1(TYPE ) TYPE& DTOR(TYPE &elm             ) {     elm.~TYPE(     ); return elm;} //  destructor
   T1(TYPE ) TYPE& CTOR(TYPE &elm             ) {new(&elm) TYPE       ; return elm;} // constructor
   T2(TA,TB) TA  & CTOR(TA   &elm,   TB &param) {new(&elm) TA  (param); return elm;} // constructor with a parameter
   T2(TA,TB) TA  & CTOR(TA   &elm, C TB &param) {new(&elm) TA  (param); return elm;} // constructor with a parameter

   T1(TYPE) Bool OK     (TYPE   x) {return x>=0;}
   T1(TYPE) void RELEASE(TYPE* &x) {if(x){x->Release(); x=null;}}

   #define IS_POW_2(x) (!( (x) & ((x)-1) ))
#endif
/******************************************************************************/
