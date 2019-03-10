/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
#define TMPL_B     u"<T"  // begin template operator
#define TMPL_E     u"T>"  // end   template operator
#define ELLIPSIS   u"(…)" // character known as "Ellipsis" surrounded by brackets
#define UNIQUE_NAME '@'   // used for distinguishing files of same names, also for special headers
#define DIV         '`'   // used instead of '/'
#define SEP         '\\'
#define SEP_FILE    '.'
#define SEP_LINE    "/******************************************************************************/"
#define EE_PATH     "Esenthel Engine"
/******************************************************************************/
enum VAR_TYPE : Byte // native variable type
{
   VAR_NONE  ,
   VAR_VOID  ,
   VAR_BOOL  ,
   VAR_BYTE  ,
   VAR_SBYTE ,
   VAR_SHORT ,
   VAR_USHORT,
   VAR_INT   ,
   VAR_UINT  ,
   VAR_LONG  ,
   VAR_ULONG ,
   VAR_FLT   ,
   VAR_DBL   ,
   VAR_CHAR8 ,
   VAR_CHAR16,
   VAR_ENUM  ,
   VAR_NULL  , // null_t

   VAR_FROM=VAR_BOOL  , // first type on which can be performed operations
   VAR_TO  =VAR_CHAR16, // last  type on which can be performed operations
};
struct Keyword
{
   Str      name;
   Bool     use_for_suggestions, follow_by_space, data_type, is_modifier, cpp;
   VAR_TYPE var_type;
};
/******************************************************************************/
struct RecTest // Recurrence Tester
{
   Bool crossed;
   Int  recurrence;

        operator Bool(   ) {if(recurrence>=128)crossed=true; return crossed;}
   void operator ++  (int) {recurrence++;}

   RecTest() {crossed=false; recurrence=0;}
};
struct IntLock // Int Increase Lock
{
   Int &value;

  ~IntLock(           )                        {value--;}
   IntLock(Int  &value) : value(        value) {value++;}
   IntLock(RecTest &rt) : value(rt.recurrence) {value++;}
   NO_COPY_CONSTRUCTOR(IntLock);
};
/******************************************************************************/
extern Bool      CppMode;
extern Int       PtrSize, EnumSize, TabLength;
extern CChar8   *AutoSource;
extern Keyword    NativeKeywords[];
extern Str       PreprocKeywords[];
extern const Int PreprocKeywordsElms, NativeKeywordsElms;
/******************************************************************************/
inline Bool  IntType(VAR_TYPE type) {return type==VAR_BYTE || type==VAR_SBYTE || type==VAR_SHORT || type==VAR_USHORT || type==VAR_INT || type==VAR_UINT || type==VAR_LONG || type==VAR_ULONG;}
inline Bool RealType(VAR_TYPE type) {return type==VAR_FLT  || type==VAR_DBL;}

inline Bool   SignedType(VAR_TYPE type) {return                   type==VAR_SBYTE || type==VAR_SHORT  || type==VAR_INT  || type==VAR_LONG  || type==VAR_FLT   || type==VAR_DBL   || type==VAR_ENUM;} // if type can   have negative values
inline Bool UnsignedType(VAR_TYPE type) {return type==VAR_BOOL || type==VAR_BYTE  || type==VAR_USHORT || type==VAR_UINT || type==VAR_ULONG || type==VAR_CHAR8 || type==VAR_CHAR16                 ;} // if type can't have negative values

inline Bool OffsetType(VAR_TYPE type) {return IntType(type) || type==VAR_BOOL || type==VAR_CHAR8 || type==VAR_CHAR16 || type==VAR_ENUM;}

Int     TypeSize  (VAR_TYPE type);
Str     TypeName  (VAR_TYPE type);
Symbol* TypeSymbol(VAR_TYPE type);

VAR_TYPE SameSizeType        (VAR_TYPE type);
VAR_TYPE SameSizeSignType    (VAR_TYPE type);
VAR_TYPE SameSizeSignBoolType(VAR_TYPE type);

VAR_TYPE ComplementResult(VAR_TYPE type);
VAR_TYPE   NegativeResult(VAR_TYPE type);
VAR_TYPE        MulResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE        DivResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE        ModResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE        AddResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE        SubResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE  ShiftLeftResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE ShiftRightResult(VAR_TYPE a, VAR_TYPE b);
VAR_TYPE        BitResult(VAR_TYPE a, VAR_TYPE b);

Bool LostConst(UInt src_const_level, UInt dest_const_level);

Int AlignAddress(Int address, Int size);

inline CHAR_TYPE CodeCharType(Char c) {return (c=='$') ? CHART_CHAR : CharType(c);}

inline Int AlignToTab(Int x) {return (x/TabLength)*TabLength;}

inline Bool AllUpCase(C Str &s) {REPA(s)if(s[i]!=CaseUp(s[i]))return false; return true;}

inline Str RandomName() {UID id; id.randomize(); return TextHex(id.i[0], 8);}

inline void ReplacePath(Str &path, Str src, Str dest) {if(StartsPath(path, src))path=dest.tailSlash(true)+SkipStartPath(path, src);}

Bool  SourceExt(C Str &ext);
Bool    TextExt(C Str &ext);
Bool AllowedExt(C Str &ext);

Memc<Str> GetFiles(C Str &files);
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int CompareCS(C Edit::Keyword &a, C Edit::Keyword &b) {return CompareCS(a.name, b.name);}
inline Int CompareCS(C Edit::Keyword &a, C       BStr    &b) {return CompareCS(a.name, b     );}
/******************************************************************************/
#endif
/******************************************************************************/
