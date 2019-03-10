// TODO: 
// process system headers button

// C++ HEADERS PARSING:
// using namespace;
// EE basic data types to ES types (only in EE headers) ?
// EE headers hide private stuff (after calculating symbols/tokens ?)

// add comments for all builtin keywords (sample usage..)

// C++ generation for Visual Studio
// 0b0011   -> 0x003
// 0x1.1    -> 0...
// 0b0011.1 -> 3.5

// support for custom #define's (make them only global, fix 'preprocess')

// replace
// refactoring

// generation of C++ headers: define global typedefs for nested classes (after "class.h" headers in UNIQUE_NAME+UNIQUE_NAME+"headers.h") "typedef A.AA X;"

// template specialization? (similar mechanism to function overloading? "func_name/817293")
// <TYPE> class X      { X(){} }
// <    > class X<int> { X(){} } // parsing CTOR will result in error, because creation of X will create X_18273912 (to be unique) and encountering X() will not detect it as the constructor
/******************************************************************************

   Font "Lucida Console" should be Sub-Pixel and should contain:
      -Ansi, German, French, Polish, Russian, Ellipsis

   operators are stored as following names in symbols:
      operator=       -> "operator="
      operator==      -> "operator=="
      operator+=      -> "operator+="
      operator()      -> "operator()"
      operator TYPE   -> "operator cast"
      operator TYPE2  -> "operator cast"
      operator new    -> "operator new"
      operator delete -> "operator delete"

      in short it's:
         auto-cast operators as "operator cast"
         other     operators as "operator?" (? = symbols)

   Functions are stored as following names in symbols:
                       => "func"         (function list)
      void func(     ) .. "func/8917283" (function)
      void func(int a) .. "func/7628349" (function)

   Labels are stored as:
      label: -> "@label" ('@' is added to avoid confusion with other symbols, labels are stored as child of FUNC, and not SPACE)

   Symbols with duplicate names are stored as:
      "name"                // original
      "Unique name 1293879" // Unique + name + random value using 'MakeUnique' function

   Nameless classes are stored as:
      "Nameless 0"
      "Nameless 1"
      ..

   Members of MODIF_TRANSPARENT classes:
      class X
      {
         class // this is the transparent class
         {
            int x;
         }
      }
      x.full_name -> "X.x"        (path/name is set to first non-transparent class, to allow for 'FindChild' to work)
      x.parent    -> "X.nameless" (parent is set to the transparent class)

   All symbols are stored in 'Symbols' counted cache.
   
   'SymbolDef' is used to store "creator" reference to a 'Symbol' in 'Symbols' (creator increases/decreases 'Symbol::valid')
      Symbol::valid indicates if symbol is actually defined (if it's set to zero then it means it's "zombie" symbol, no longer valid)

   Handling symbol "pointer array ref" is done in following order:
      1. pointer, 2. array, 3. ref - example - symbol can be a pointer, symbol can be array of pointers, symbol can be reference to array of pointers

   Pointers to arrays are processed in following method:
      1. ptr_level, 2. array_dims (those can be both array and pointer) - 1 pointer to array of 3 pointer-pointers "int** (*)x[3]" is handled like this - 'ptr_level'=2 (describes "**"), 'array_dims'={3 (describes "[3]"), DIM_PTR (describes "(*)")}

/******************************************************************************/
#include "stdafx.h"
#if WINDOWS_OLD
   #include "../../../ThirdPartyLibs/begin.h"

   #define MAXUINT ((UINT)~((UINT)0))
   #include "../../../ThirdPartyLibs/VS/Setup.Configuration.h"

   #include "../../../ThirdPartyLibs/end.h"

   // Use smart pointers (without ATL) to release objects when they fall out of scope.
   _COM_SMARTPTR_TYPEDEF(ISetupInstance, __uuidof(ISetupInstance));
   _COM_SMARTPTR_TYPEDEF(ISetupInstance2, __uuidof(ISetupInstance2));
   _COM_SMARTPTR_TYPEDEF(IEnumSetupInstances, __uuidof(IEnumSetupInstances));
   _COM_SMARTPTR_TYPEDEF(ISetupConfiguration, __uuidof(ISetupConfiguration));
   _COM_SMARTPTR_TYPEDEF(ISetupConfiguration2, __uuidof(ISetupConfiguration2));
   _COM_SMARTPTR_TYPEDEF(ISetupHelper, __uuidof(ISetupHelper));
   _COM_SMARTPTR_TYPEDEF(ISetupPackageReference, __uuidof(ISetupPackageReference));
#endif
namespace EE{
namespace Edit{
/******************************************************************************/
#define LOAD_SYMBOLS 1
#define LOG_LINES 16384
/******************************************************************************/
Int TabLength=3;
/******************************************************************************/
Memc<Macro>    SystemMacros, // base macros (__cplusplus, _WIN32, ..)
                   EEMacros, // base macros after including EE  libraries
                  LibMacros, // base macros after including C++ libraries (EE, 3rd party libs)
              ProjectMacros; // lib  macros with  custom project macros
Memc<SymbolPtr>    EEUsings, // base using namespaces after including EE  libraries
                  LibUsings, // base using namespaces after including C++ libraries (EE, 3rd party libs)
              ProjectUsings; // lib  using namespaces with custom project usings
/******************************************************************************/
Keyword NativeKeywords[]=
{
   {"using"    , true , true , false, false, true, VAR_NONE},
   {"namespace", true , true , false, false, true, VAR_NONE},
   {"union"    , true , true , false, false, true, VAR_NONE},
   {"struct"   , false, true , false, false, true, VAR_NONE},
   {"class"    , true , true , false, false, true, VAR_NONE},
   {"enum"     , true , true , false, false, true, VAR_NONE},
   {"typedef"  , true , true , false, false, true, VAR_NONE},
   {"operator" , true , false, false, false, true, VAR_NONE},

   {"public"        , true , true , false, true, true, VAR_NONE},
   {"private"       , true , true , false, true, true, VAR_NONE},
   {"protected"     , true , true , false, true, true, VAR_NONE},
   {"static"        , true , true , false, true, true, VAR_NONE},
   {"extern"        , false, true , false, true, true, VAR_NONE},
   {"const"         , true , true , false, true, true, VAR_NONE},
   {"constexpr"     , true , true , false, true, true, VAR_NONE},
   {"mutable"       , true , true , false, true, true, VAR_NONE},
   {"register"      , false, true , false, true, true, VAR_NONE},
   {"explicit"      , true , true , false, true, true, VAR_NONE},
   {"virtual"       , true , true , false, true, true, VAR_NONE},
   {"final"         , true , false, false, true, true, VAR_NONE},
   {"override"      , true , false, false, true, true, VAR_NONE},
   {"volatile"      , false, true , false, true, true, VAR_NONE},
   {"inline"        , true , true , false, true, true, VAR_NONE},
   {"__forceinline" , false, true , false, true, true, VAR_NONE},
   {"friend"        , true , true , false, true, true, VAR_NONE},
   {"const_mem_addr", true , true , false, true, true, VAR_NONE}, // cpp too because it's used in EE headers

   {"switch"  , true , false, false, false, true, VAR_NONE},
   {"case"    , true , true , false, false, true, VAR_NONE},
   {"default" , true , false, false, false, true, VAR_NONE},
   {"return"  , true , true , false, false, true, VAR_NONE},
   {"break"   , true , false, false, false, true, VAR_NONE},
   {"continue", true , false, false, false, true, VAR_NONE},
   {"goto"    , true , true , false, false, true, VAR_NONE},
   {"if"      , true , false, false, false, true, VAR_NONE},
   {"else"    , true , true , false, false, true, VAR_NONE},
   {"for"     , true , false, false, false, true, VAR_NONE},
   {"while"   , true , false, false, false, true, VAR_NONE},
   {"do"      , true , false, false, false, true, VAR_NONE},
   {"throw"   , false, false, false, false, true, VAR_NONE},
   {"try"     , false, false, false, false, true, VAR_NONE},
   {"catch"   , false, false, false, false, true, VAR_NONE},

   {"auto"  , true, false, true, false, true, VAR_NONE  }, // for now just use as typedef
   {"void"  , true, false, true, false, true, VAR_VOID  },
   {"bool"  , true, false, true, false, true, VAR_BOOL  },
   {"char"  , true, false, true, false, true, VAR_CHAR16},
   {"short" , true, false, true, false, true, VAR_SHORT },
   {"int"   , true, false, true, false, true, VAR_INT   },
   {"long"  , true, false, true, false, true, VAR_LONG  },
   {"float" , true, false, true, false, true, VAR_NONE  }, // this is typedef
   {"double", true, false, true, false, true, VAR_NONE  }, // this is typedef

   {   "size_t", false, false, true, false, true, VAR_NONE}, // this is typedef
   {  "ssize_t", false, false, true, false, true, VAR_NONE}, // this is typedef
   { "intptr_t", false, false, true, false, true, VAR_NONE}, // this is typedef
   {"uintptr_t", false, false, true, false, true, VAR_NONE}, // this is typedef
   {"char16_t" , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"char32_t" , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"wchar_t"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"__int8"   , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"__int16"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"__int32"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"__int64"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"int8_t"   , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"uint8_t"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"int16_t"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"uint16_t" , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"int32_t"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"uint32_t" , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"int64_t"  , false, false, true, false, true, VAR_NONE}, // this is typedef
   {"uint64_t" , false, false, true, false, true, VAR_NONE}, // this is typedef

   {"signed"  , false, true, true, false, true, VAR_NONE}, // this is typedef
   {"unsigned", false, true, true, false, true, VAR_NONE}, // this is typedef

   {"template", true, false, false, false, true, VAR_NONE},
   {"typename", true, true , false, false, true, VAR_NONE},

   {"sizeof", true, false, false, false, true, VAR_NONE},
   {"typeid", true, false, false, false, true, VAR_NONE},

   {"dynamic_cast"    , true, false, false, false, true, VAR_NONE},
   {"static_cast"     , true, false, false, false, true, VAR_NONE},
   {"const_cast"      , true, false, false, false, true, VAR_NONE},
   {"reinterpret_cast", true, false, false, false, true, VAR_NONE},

   {"__super", false, false, false, false, true, VAR_NONE},
   {"this"   , true , false, false, false, true, VAR_NONE},

   {"true" , true, false, false, false, true, VAR_BOOL},
   {"false", true, false, false, false, true, VAR_BOOL},

   {"new"   , true, true, false, false, true, VAR_NONE},
   {"delete", true, true, false, false, true, VAR_NONE},

   {"__cdecl"   , false, true, false, true, true, VAR_NONE},
   {"__stdcall" , false, true, false, true, true, VAR_NONE},
   {"__fastcall", false, true, false, true, true, VAR_NONE},
   {"__thiscall", false, true, false, true, true, VAR_NONE},

   {"__declspec", false, true, false, false, true, VAR_NONE},

   // c# / es
 //{"base" , true, false, false, false, false, VAR_NONE},
   {"super", true, false, false, false, false, VAR_NONE},

 //{"str"   , true, false, true, false, false, VAR_NONE},
 //{"string", true, false, true, false, false, VAR_NONE},

   { "byte"  , true, false, true, false, false, VAR_BYTE  },
   {"sbyte"  , true, false, true, false, false, VAR_SBYTE },
   {"ushort" , true, false, true, false, false, VAR_USHORT},
   {"uint"   , true, false, true, false, false, VAR_UINT  },
   { "intptr", true, false, true, false, false, VAR_NONE  }, // this is a typedef
   {"uintptr", true, false, true, false, false, VAR_NONE  }, // this is a typedef
   {"ulong"  , true, false, true, false, false, VAR_ULONG },
   {"flt"    , true, false, true, false, false, VAR_FLT   },
   {"dbl"    , true, false, true, false, false, VAR_DBL   },
   { "ptr"   , true, false, true, false, false, VAR_NONE  }, // this is a typedef
   {"cptr"   , true, false, true, false, false, VAR_NONE  }, // this is a typedef
   { "char8" , true, false, true, false, false, VAR_CHAR8 },
   {"cchar8" , true, false, true, false, false, VAR_NONE  }, // this is a typedef
   {"cchar"  , true, false, true, false, false, VAR_NONE  }, // this is a typedef

 //{"until", true, false, false, false, false, VAR_NONE},

   {"null_t", false, false, true , false, false, VAR_NULL},
   {"null"  , true , false, false, false, false, VAR_NULL},
};
const Int NativeKeywordsElms=Elms(NativeKeywords);

Str PreprocKeywords[]=
{
   "include",
   "if",
   "ifdef",
   "ifndef",
   "elif",
   "else",
   "endif",
   "undef",
   "define",
   "defined",
   "pack",
   "pragma",
   "error",
};
const Int PreprocKeywordsElms=Elms(PreprocKeywords);

static Memc<SymbolDef> SymbolDefs; // built-in keywords
       Bool            CppMode;
       CodeEditor      CE;
       
static CChar8 *CodeEditorDat="Bin/Code Editor.dat";
       CChar8 *AutoSource   ="Auto.h";
/******************************************************************************/
// GLOBAL
/******************************************************************************/
Int TypeSize(VAR_TYPE type)
{
   switch(type) // now check if it has raw value
   {
      default        : return 0; // VAR_NONE
      case VAR_VOID  : return 0;
      case VAR_BOOL  : return 1;
      case VAR_BYTE  : return 1;
      case VAR_SBYTE : return 1;
      case VAR_SHORT : return 2;
      case VAR_USHORT: return 2;
      case VAR_INT   : return 4;
      case VAR_UINT  : return 4;
      case VAR_LONG  : return 8;
      case VAR_ULONG : return 8;
      case VAR_FLT   : return 4;
      case VAR_DBL   : return 8;
      case VAR_CHAR8 : return 1;
      case VAR_CHAR16: return 2;
      case VAR_ENUM  : return EnumSize;
      case VAR_NULL  : return  PtrSize;
   }
}
Str TypeName(VAR_TYPE type)
{
   switch(type)
   {
      default        : return "none"; // VAR_NONE
      case VAR_VOID  : return "void";
      case VAR_BOOL  : return "bool";
      case VAR_BYTE  : return "byte";
      case VAR_SBYTE : return "sbyte";
      case VAR_SHORT : return "short";
      case VAR_USHORT: return "ushort";
      case VAR_INT   : return "int";
      case VAR_UINT  : return "uint";
      case VAR_LONG  : return "long";
      case VAR_ULONG : return "ulong";
      case VAR_FLT   : return "flt";
      case VAR_DBL   : return "dbl";
      case VAR_CHAR8 : return "char8";
      case VAR_CHAR16: return "char";
      case VAR_ENUM  : return "enum";
      case VAR_NULL  : return "null";
   }
}
Symbol* TypeSymbol(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_VOID  :
      case VAR_BOOL  :
      case VAR_BYTE  :
      case VAR_SBYTE :
      case VAR_SHORT :
      case VAR_USHORT:
      case VAR_INT   :
      case VAR_UINT  :
      case VAR_LONG  :
      case VAR_ULONG :
      case VAR_FLT   :
      case VAR_DBL   :
      case VAR_CHAR8 :
      case VAR_CHAR16:
      case VAR_NULL  : return Symbols(TypeName(type));
      default        : return null;
   }
}
VAR_TYPE SameSizeType(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_BOOL :
      case VAR_CHAR8:
      case VAR_SBYTE: return VAR_BYTE;

      case VAR_CHAR16:
      case VAR_USHORT: return VAR_SHORT;

      case VAR_ENUM:
      case VAR_UINT: return VAR_INT;

      case VAR_ULONG: return VAR_LONG;

      default: return type;
   }
}
VAR_TYPE SameSizeSignType(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_BOOL :
      case VAR_CHAR8: return VAR_BYTE;

      case VAR_CHAR16: return VAR_USHORT;

      case VAR_ENUM: return VAR_INT;

      default: return type;
   }
}
VAR_TYPE SameSizeSignBoolType(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_CHAR8: return VAR_BYTE;

      case VAR_CHAR16: return VAR_USHORT;

      case VAR_ENUM: return VAR_INT;

      default: return type;
   }
}
VAR_TYPE ComplementResult(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_CHAR8 :
      case VAR_CHAR16:
      case VAR_BOOL  :
      case VAR_BYTE  :
      case VAR_SBYTE :
      case VAR_SHORT :
      case VAR_USHORT:
      case VAR_INT   :
      case VAR_ENUM  : return VAR_INT;

      case VAR_UINT  : return VAR_UINT;

      case VAR_LONG  : return VAR_LONG;

      case VAR_ULONG : return VAR_ULONG;
      
      default: return VAR_NONE;
   }
}
VAR_TYPE NegativeResult(VAR_TYPE type)
{
   switch(type)
   {
      case VAR_CHAR8 :
      case VAR_CHAR16:
      case VAR_BOOL  :
      case VAR_BYTE  :
      case VAR_SBYTE :
      case VAR_SHORT :
      case VAR_USHORT:
      case VAR_INT   :
      case VAR_UINT  :
      case VAR_ENUM  : return VAR_INT;

      case VAR_LONG  :
      case VAR_ULONG : return VAR_LONG;

      case VAR_FLT: return VAR_FLT;

      case VAR_DBL: return VAR_DBL;

      default: return VAR_NONE;
   }
}
// in C++ (int op int)->int, (uint op int)->uint, (int op uint)->uint, (uint op uint)->uint (uint has higher preference, int is choosed only if both are int)
VAR_TYPE MulResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL)return VAR_NONE;
   if(a==VAR_DBL  || b==VAR_DBL )return VAR_DBL;
   if(a==VAR_FLT  || b==VAR_FLT )return VAR_FLT;
   if(a==VAR_BOOL && b==VAR_BOOL)return VAR_BOOL;
   if(a==VAR_LONG || a==VAR_ULONG || b==VAR_LONG || b==VAR_ULONG)return (SignedType(a) && SignedType(b)) ? VAR_LONG : VAR_ULONG;
                                                                 return (SignedType(a) && SignedType(b)) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE DivResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL)return VAR_NONE;
   if(a==VAR_DBL  || b==VAR_DBL  )return VAR_DBL;
   if(a==VAR_FLT  || b==VAR_FLT  )return VAR_FLT;
   if(a==VAR_LONG || a==VAR_ULONG)return (SignedType(a) && SignedType(b)) ? VAR_LONG : VAR_ULONG;
                                  return (SignedType(a) && SignedType(b)) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE ModResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL || RealType(a) || RealType(b))return VAR_NONE;
   if(a==VAR_LONG || a==VAR_ULONG)return (SignedType(a) && SignedType(b)) ? VAR_LONG : VAR_ULONG;
                                  return (SignedType(a) && SignedType(b)) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE AddResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL)return VAR_NONE;
   if(a==VAR_DBL  || b==VAR_DBL)return VAR_DBL;
   if(a==VAR_FLT  || b==VAR_FLT)return VAR_FLT;
   if(a==VAR_LONG || a==VAR_ULONG || b==VAR_LONG || b==VAR_ULONG)return (SignedType(a) && SignedType(b)) ? VAR_LONG : VAR_ULONG;
                                                                 return (SignedType(a) && SignedType(b)) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE SubResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL)return VAR_NONE;
   if(a==VAR_DBL  || b==VAR_DBL)return VAR_DBL;
   if(a==VAR_FLT  || b==VAR_FLT)return VAR_FLT;
   if(a==VAR_LONG || a==VAR_ULONG || b==VAR_LONG || b==VAR_ULONG)return VAR_LONG;
                                                                 return VAR_INT ;
}
VAR_TYPE ShiftLeftResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL || RealType(a) || RealType(b))return VAR_NONE;
   if(a==VAR_LONG || a==VAR_ULONG)return SignedType(a) ? VAR_LONG : VAR_ULONG; // in C++ "Int<<Int64" is still an Int
                                  return SignedType(a) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE ShiftRightResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL || RealType(a) || RealType(b))return VAR_NONE;
   if(a==VAR_BOOL && b==VAR_BOOL )return VAR_BOOL;
   if(a==VAR_LONG || a==VAR_ULONG)return SignedType(a) ? VAR_LONG : VAR_ULONG;
                                  return SignedType(a) ? VAR_INT  : VAR_UINT ;
}
VAR_TYPE BitResult(VAR_TYPE a, VAR_TYPE b)
{
   if(a==VAR_NONE || a==VAR_VOID || a==VAR_NULL || b==VAR_NONE || b==VAR_VOID || b==VAR_NULL || RealType(a) || RealType(b))return VAR_NONE;
   if(a==VAR_LONG || a==VAR_ULONG || b==VAR_LONG || b==VAR_ULONG)return (SignedType(a) && SignedType(b)) ? VAR_LONG : VAR_ULONG;
                                                                 return (SignedType(a) && SignedType(b)) ? VAR_INT  : VAR_UINT ;
}
/******************************************************************************/
Bool LostConst(UInt src_const_level, UInt dest_const_level)
{
   // src dest lost
   //  0   0    0
   //  0   1    0
   //  1   0    1
   //  1   1    0
   UInt   lost=(src_const_level & ~dest_const_level);
   return lost!=0;
}
/******************************************************************************/
Int AlignAddress(Int address, Int size)
{
   if(size>=2)switch(size)
   {
      case  2:
      case  3: return Ceil2(address);

      case  4:
      case  5:
      case  6:
      case  7: return Ceil4(address);

      default: return Ceil8(address);
   }
   return address;
}
/******************************************************************************/
Bool SourceExt(C Str &ext)
{
   return ext=="h"
      ||  ext=="hpp"
      ||  ext=="cpp"
      ||  ext=="c"
      ||  ext=="cc"
      ||  ext=="cxx"
      ||  ext=="inc"  // include c file
      ||  ext=="m"    // Apple Objective-C
      ||  ext=="mm";  // Apple Objective-C++
}
Bool TextExt(C Str &ext)
{
   return SourceExt(ext)
      ||  ext=="cs"   // C#
      ||  ext=="java"
      ||  ext=="txt"
      ||  ext=="xml"
      ||  ext=="htm"
      ||  ext=="html"
      ||  ext=="php"
      ||  ext=="mk";
}
Bool AllowedExt(C Str &ext)
{
   return TextExt(ext)
   || ext=="lib"
   || ext=="ico"
   || ext=="pak";
}
/******************************************************************************/
Memc<Str> GetFiles(C Str &files)
{
   Memc<Str> elms; Split(elms, files, '|'); REPA(elms)
   {
      Str &s=elms[i];
      s.removeOuterWhiteChars().replace('\\', '/');
      if(!s.is())elms.remove(i, true);
   }
   return elms;
}
/******************************************************************************/
// MENU
/******************************************************************************/
static void MenuNew      (   ) {CE.New      ();}
static void MenuOverwrite(   ) {CE.overwrite();}
static void MenuSave     (   ) {CE.save     ();}
static void MenuLoad     (   ) {CE.load     ();}
static void MenuClose    (Ptr) {CE.close    ();}
static void MenuLocate   (   ) {CE.cei().elmLocate(CE.cei().sourceCurId());}

static void EditUndo        (   ) {                   CE.undo      ();}
static void EditRedo        (   ) {                   CE.redo      ();}
static void EditCut         (   ) {if(!CE.view_mode())CE.cut       ();}
static void EditCopy        (   ) {if(!CE.view_mode())CE.copy      ();}
static void EditPaste       (   ) {if(!CE.view_mode())CE.paste     ();}
static void EditPaste2      (   ) {if(!CE.view_mode())CE.paste     (false);}
static void EditSeparator   (   ) {if(!CE.view_mode())CE.separator ();}
static void EditSelectAll   (   ) {if(!CE.view_mode())CE.selectAll ();}
static void EditSelectWord  (   ) {if(!CE.view_mode())CE.selectWord();}
static void EditMakeLower   (   ) {if(!CE.view_mode())CE.makeCase  (false);}
static void EditMakeUpper   (   ) {if(!CE.view_mode())CE.makeCase  (true );}
static void EditFind        (Ptr) {CE.find   .toggle();}
static void EditReplace     (Ptr) {CE.replace.visibleToggleActivate();}
static void EditGotoLine    (   ) {CE.goto_line_window.visibleToggleActivate();}
static void EditGotoDef     (   ) {if(CE.cur())CE.cur()->jumpToCur        ();}
static void EditFindRef     (   ) {if(CE.cur())CE.cur()->findAllReferences();}
static void EditPrevIssue   (   ) {CE.prevIssue();}
static void EditNextIssue   (   ) {CE.nextIssue();}
static void EditPrevCurPos  (   ) {CE.prevCurPos();}
static void EditNextCurPos  (   ) {CE.nextCurPos();}
static void EditAutoComplete(   ) {if(CE.cur())CE.cur()->listSuggestions( 1);}
static void EditAutoCompleteElm() {if(CE.cur())CE.cur()->listSuggestions(-2);}

static void EditNextFile() {CE.nextFile();}
static void EditPrevFile() {CE.prevFile();}

static void ViewMode() {CE.view_mode.push();}
static void ElmNames() {CE.view_what.menu("Element Names"  , !CE.view_what.menu("Element Names"  ));}
static void FuncBody() {CE.view_what.menu("Function Bodies", !CE.view_what.menu("Function Bodies"));}
/******************************************************************************/
// CODE EDITOR
/******************************************************************************/
static CChar8 *ExcludeHeaders[]=
{
   "begin.h",
   "end.h",
   "Structs.h",
   "Vertex Index Buffer Ex.h",
   "_Cache.h",
   "_Map.h",
   "_Memc.h",
   "_Memc Thread Safe.h",
   "_Memb.h",
   "_Memx.h",
   "_Meml.h",
   "_Grid.h",
   "Templates.h",
   "Index.h",
   "Display Viewport.h",
   "Fur.h",
   "Renderer Instancing.h",
   "Shader Main.h",
   "Sound Buffer.h",
   "Aggregate.h",
   "Android.h",
   "String Borrowed.h",
   "Esenthel Config.h",
   "MP4.h",
};
/*static CChar8 *ReplaceHeaders[][2]=
{
   "animation keys.h", "_/Animation",
   "quaternion.h", "_/Math",
   "grid.h", "_/Memory",
   "mem list.h", "_/Memory",
   "Mem Continuous Thread Safe.h", "_/Memory",
   "mesh group.h", "_/Mesh",
   "mesh import.h", "_/Mesh",
   "mesh render.h", "_/Mesh",
 //"string borrowed.h", "_/Misc",
   "thread.h", "_/Misc",
   "windows.h", "_/Misc",
   "socket.h", "_/Net",

   "Editor Interface.h", "_/Edit",
   "Undo.h", "_/Edit",
   "Version.h", "_/Edit",
   "Viewport4.h", "_/Edit",
};*/
static FILE_LIST_MODE AddEEHeader(C FileFind &ff, CodeEditor &ce)
{
   if(ff.type==FSTD_FILE)
   {
      Str relative_path=SkipStartPath(ff.pathName(), ce.ee_h_path),
            target_path=relative_path,
                   base=ff.name;
      REPA(ExcludeHeaders)if(base==ExcludeHeaders[i]   )return FILE_LIST_CONTINUE;
    //REPA(ReplaceHeaders)if(base==ReplaceHeaders[i][0]){target_path=S+ReplaceHeaders[i][1]+'\\'+base; break;}
      if(StartsPath(target_path, "Code"))return FILE_LIST_CONTINUE; // skip all from "Code" folders
      GetFile(ce.items, GetExtNot(target_path), ff.pathName()); // add folder & file to items
   }
   return FILE_LIST_CONTINUE;
}
/******************************************************************************/
void CodeEditor::setMenu(Node<MenuElm> &menu)
{
   {
      {
         Node<MenuElm> &f=menu+="File";
       /*f.New().create("New"      , MenuNew      ).kbsc(KbSc(KB_N, KBSC_CTRL_CMD));
         f++;*/
         f.New().create("Save"     , MenuOverwrite).kbsc(KbSc(KB_F2               )).kbsc2(KbSc(KB_S, KBSC_CTRL_CMD           ));
       /*f.New().create("Save"     , MenuSave     ).kbsc(KbSc(KB_F2, KBSC_CTRL_CMD)).kbsc2(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_SHIFT));
         f++;
         f.New().create("Load"     , MenuLoad     ).kbsc(KbSc(KB_F3                      )).kbsc2(KbSc(KB_O, KBSC_CTRL_CMD));*/
         f.New().create("Close"    , MenuClose    ).kbsc(KbSc(KB_F3, KBSC_ALT|KBSC_REPEAT)).kbsc2(KbSc('q' , KBSC_CTRL_CMD|KBSC_REPEAT));
         f.New().create("Locate"   , MenuLocate   ).kbsc(KbSc(KB_L , KBSC_CTRL_CMD       )).desc("Locate this file in the Project");
      }
      {
         Node<MenuElm> &e=menu+="Edit";
         e.New().create("Undo" , EditUndo).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Redo" , EditRedo).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc2(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT));
         e.New().create("Undo2", EditUndo).kbsc(KbSc(KB_BACK, KBSC_ALT           |KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space (besides on Windows Notepad they also work and are not listed)
         e.New().create("Redo2", EditRedo).kbsc(KbSc(KB_BACK, KBSC_ALT|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space (besides on Windows Notepad they also work and are not listed)
         e++;
         e.New().create("Cut"                    , EditCut   ).kbsc(KbSc('x', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Copy"                   , EditCopy  ).kbsc(KbSc('c', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Paste"                  , EditPaste ).kbsc(KbSc('v', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Paste Don't Move Cursor", EditPaste2).kbsc(KbSc('V', KBSC_CTRL_CMD|KBSC_REPEAT)).desc("This option performs Paste without moving the cursor");
         e++;
         e.New().create("Insert Separator", EditSeparator).kbsc(KbSc(KB_ENTER, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT));
         e++;
         e.New().create("Select All" , EditSelectAll ).kbsc(KbSc('a', KBSC_CTRL_CMD));
         e.New().create("Select Word", EditSelectWord).kbsc(KbSc('w', KBSC_CTRL_CMD));
         e++;
         e.New().create("Find"         , EditFind      ).kbsc(KbSc('f', KBSC_CTRL_CMD));
         e.New().create("Find Next"    , FindNext, find).kbsc(KbSc('d', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Find Previous", FindPrev, find).kbsc(KbSc('D', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Replace"      , EditReplace   ).kbsc(KbSc('r', KBSC_CTRL_CMD));
         e++;
         e.New().create("Next Opened File"    , EditNextFile).kbsc(KbSc(KB_F6,            KBSC_REPEAT)).kbsc2(KbSc(KB_TAB, KBSC_CTRL_CMD           |KBSC_REPEAT));
         e.New().create("Previous Opened File", EditPrevFile).kbsc(KbSc(KB_F6, KBSC_SHIFT|KBSC_REPEAT)).kbsc2(KbSc(KB_TAB, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT));
         e++;
         e.New().create("Next Issue"    , EditNextIssue).kbsc(KbSc('e', KBSC_CTRL_CMD|KBSC_REPEAT)).desc("Go to the next compiler issue");
         e.New().create("Previous Issue", EditPrevIssue).kbsc(KbSc('E', KBSC_CTRL_CMD|KBSC_REPEAT)).desc("Go to the previous compiler issue");
         e++;
         e.New().create("Navigate Forward"  , EditNextCurPos).kbsc(KbSc(KB_RIGHT, KBSC_ALT|KBSC_REPEAT)).kbsc2(KbSc(KB_RBR, KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Navigate Backward" , EditPrevCurPos).kbsc(KbSc(KB_LEFT , KBSC_ALT|KBSC_REPEAT)).kbsc2(KbSc(KB_LBR, KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Navigate Forward2" , EditNextCurPos).kbsc(KbSc(KB_NAV_FORWARD, KBSC_REPEAT)).flag(MENU_HIDDEN);
         e.New().create("Navigate Backward2", EditPrevCurPos).kbsc(KbSc(KB_NAV_BACK   , KBSC_REPEAT)).flag(MENU_HIDDEN);
         e++;
         e.New().create("Auto Complete (for Code Symbols)"    , EditAutoComplete   ).kbsc(KbSc(KB_SPACE, KBSC_CTRL_CMD));
         e.New().create("Auto Complete (for Project Elements)", EditAutoCompleteElm).kbsc(KbSc(KB_SPACE, KBSC_CTRL_CMD|KBSC_SHIFT));
         e++;
         e.New().create("Go To Line", EditGotoLine).kbsc(KbSc('g', KBSC_CTRL_CMD));

         e.New().create("Go To Definition"   , EditGotoDef).kbsc(KbSc(KB_ENTER, KBSC_CTRL_CMD         )).desc("Go to definition of the symbol located at cursor position.\nAlternative shortcut for this option is Middle Mouse Button or Ctrl + Left Mouse Button.");
         e.New().create("Find All References", EditFindRef).kbsc(KbSc(KB_ENTER, KBSC_CTRL_CMD|KBSC_ALT)).desc("Find all references of the symbol located at cursor position.\nAlternative shortcut for this option is Ctrl + Middle Mouse Button.");

         e++;
         e.New().create("Make Lowercase", EditMakeLower).kbsc(KbSc('u', KBSC_CTRL_CMD|KBSC_REPEAT));
         e.New().create("Make Uppercase", EditMakeUpper).kbsc(KbSc('U', KBSC_CTRL_CMD|KBSC_REPEAT));

         e.New().create("Elm Names"  , ElmNames).kbsc(KbSc(KB_E, KBSC_ALT)).flag(MENU_HIDDEN);
         e.New().create("View Mode"  , ViewMode).kbsc(KbSc(KB_V, KBSC_ALT)).flag(MENU_HIDDEN);
         e.New().create("Func Bodies", FuncBody).kbsc(KbSc(KB_B, KBSC_ALT)).flag(MENU_HIDDEN);
      }
   }
}
/******************************************************************************/
static void BuildClose        (CodeEditor &ce) {ce.visibleOutput       (false);}
static void DevlogClose       (CodeEditor &ce) {ce.visibleAndroidDevLog(false);}
static void OutputExport      (CodeEditor &ce) {ce. build_io.save();}
static void DevlogExport      (CodeEditor &ce) {ce.devlog_io.save();}
static void DevlogClear       (CodeEditor &ce) {ce.devlog_data.clear(); ce.devlog_list.clear();}
static void RefreshSourceLines(CodeEditor &ce)
{
   //ce.func_bodies    .visible(ce.view_mode());
   //ce.private_members.visible(ce.view_mode());
   if(ce.cur())ce.cur()->validateView();
}
static void OutputCopy(CodeEditor &ce)
{
   Str     text; FREPA(ce.build_list)if(CodeEditor::BuildResult *line=ce.build_list.visToData(i)){text+=line->text; text+='\n';}
   ClipSet(text);
}
static void ViewElmNames      () {CE.view_elm_names      =CE.view_what.menu("Element Names"  );}
static void ViewComments      () {CE.view_comments       =CE.view_what.menu("Comments"       ); RefreshSourceLines(CE);}
static void ViewFuncs         () {CE.view_funcs          =CE.view_what.menu("Functions"      ); RefreshSourceLines(CE);}
static void ViewFuncBodies    () {CE.view_func_bodies    =CE.view_what.menu("Function Bodies"); RefreshSourceLines(CE);}
static void ViewPrivateMembers() {CE.view_private_members=CE.view_what.menu("Private Members"); RefreshSourceLines(CE);}

static void OutputExport(C Str &name, CodeEditor &ce)
{
   FileText ft; if(ft.write(name))FREPA(ce.build_list)if(CodeEditor::BuildResult *line=ce.build_list.visToData(i))ft.putLine(line->text);
}
static void DevlogExport(C Str &name, CodeEditor &ce)
{
   FileText ft; if(ft.write(name))FREPA(ce.devlog_list)if(CodeEditor::DeviceLog *dl=ce.devlog_list.visToData(i))ft.putLine(dl->asText());
}
static void DevlogFilter(CodeEditor &ce)
{
   Memt  <Bool> visible;
   MemPtr<Bool> v;
   if(ce.devlog_filter())
   {
      v.point(visible.setNum(ce.devlog_data.elms()));
      REPAO(visible)=(ce.devlog_data[i].mode==CodeEditor::DeviceLog::APP);
   }
   ce.devlog_list.setData(ce.devlog_data, v, true);
}
static void DevlogFilter2(CodeEditor &ce) {DevlogFilter(ce); ce.devlog_region.scrollEndY(true);}
static Str  DeviceLogOrder(C CodeEditor::DeviceLog &dl) {return TextInt(CE.devlog_data.index(&dl), 5);}

static void BuildError(C Str &msg, Bool at_end)
{
   CE.buildNew().set(msg).setError();
   CE.buildUpdate(false);
   if(at_end)CE.build_region.scrollEndY(); // auto-scroll to the end
}

static Int CompareItem(C Item &a, C Item &b)
{
   if(Int c=Compare(a.type     , b.type     ))return c;
   if(Int c=Compare(a.base_name, b.base_name))return c;
   return 0;
}
static void SortItems(Memx<Item> &items)
{
   items.sort(CompareItem); REPA(items)SortItems(items[i].children);
}
/******************************************************************************/
CodeEditor::CodeEditor()
{
  _cur=null;
   view_elm_names=view_comments=view_funcs=true; view_func_bodies=view_private_members=false;
   devenv_version=-1; devenv_express=devenv_com=build_msbuild=false;
   build_mode=BUILD_BUILD; build_exe_type=EXE_EXE; build_debug=true; build_windows_code_sign=false; build_phase=build_phases=build_step=build_steps=0; build_refresh=0; build_project_id.zero();
   curposi=0;
   symbols_loaded=false;
   menu_on_top=false;
   config_debug=true; config_dx9=false; config_32_bit=false;
   cei(cei_temp);
   parent=null;
#if MAC
   config_exe=EXE_MAC;
#elif LINUX
   config_exe=EXE_LINUX;
#else
   config_exe=EXE_EXE;
#endif
}
void CodeEditor::del()
{
   sources   .del(); // delete before 'Symbols' container
   SymbolDefs.del(); // delete before 'Symbols' container
   items     .del();
}
/******************************************************************************/
void CodeEditor::replacePath(C Str &src, C Str &dest)
{
   REPAO(items  ).replacePath(src, dest);
   REPAO(sources).replacePath(src, dest);
}
/******************************************************************************/
void CodeEditor::create(GuiObj *parent, Bool menu_on_top)
{
   T.parent     =parent;
   T.menu_on_top=menu_on_top;

   Symbols.caseSensitive(true);

   // setup keywords
   Sort( NativeKeywords, Elms( NativeKeywords), CompareCS);
   Sort(PreprocKeywords, Elms(PreprocKeywords), CompareCS);

   FREPA(NativeKeywords)
   {
      Keyword   &k=NativeKeywords[i];
      SymbolDef &s=SymbolDefs.New().require(k.name);
      s->type     =Symbol::KEYWORD;
      s->var_type =k.var_type;
      FlagSet(s->modifiers, Symbol::MODIF_DATA_TYPE       ,  k.data_type          );
      FlagSet(s->modifiers, Symbol::MODIF_ES_ONLY         , !k.cpp                );
      FlagSet(s->modifiers, Symbol::MODIF_CPP_MODIFIER    ,  k.is_modifier        );
      FlagSet(s->modifiers, Symbol::MODIF_FOLLOW_BY_SPACE ,  k.follow_by_space    );
      FlagSet(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS, !k.use_for_suggestions);
   }
   Symbol::TYPE typedef_type=Symbol::KEYWORD; // keep as keywords so "Find" displays them as KEYWORD's and 'isMemberOf' and 'canBeAccessedFrom' treat them as keywords
   {Symbol &s=*Symbols("auto"     ); s.type=typedef_type; s.value="void"   ;}
   {Symbol &s=*Symbols( "ptr"     ); s.type=typedef_type; s.value="void"   ; s.value.const_level=0; s.value.ptr_level=1;}
   {Symbol &s=*Symbols("cptr"     ); s.type=typedef_type; s.value="void"   ; s.value.const_level=1; s.value.ptr_level=1;}
   {Symbol &s=*Symbols("cchar8"   ); s.type=typedef_type; s.value="char8"  ; s.value.const_level=1; s.value.ptr_level=0;}
   {Symbol &s=*Symbols("cchar"    ); s.type=typedef_type; s.value="char"   ; s.value.const_level=1; s.value.ptr_level=0;}
   {Symbol &s=*Symbols("float"    ); s.type=typedef_type; s.value="flt"    ;}
   {Symbol &s=*Symbols("double"   ); s.type=typedef_type; s.value="dbl"    ;}
#if X64
   {Symbol &s=*Symbols( "intptr"  ); s.type=typedef_type; s.value= "long"  ;}
   {Symbol &s=*Symbols("uintptr"  ); s.type=typedef_type; s.value="ulong"  ;}
#else
   {Symbol &s=*Symbols( "intptr"  ); s.type=typedef_type; s.value= "int"   ;}
   {Symbol &s=*Symbols("uintptr"  ); s.type=typedef_type; s.value="uint"   ;}
#endif
   {Symbol &s=*Symbols(   "size_t"); s.type=typedef_type; s.value= "intptr";}
   {Symbol &s=*Symbols(  "ssize_t"); s.type=typedef_type; s.value="uintptr";}
   {Symbol &s=*Symbols( "intptr_t"); s.type=typedef_type; s.value= "intptr";}
   {Symbol &s=*Symbols("uintptr_t"); s.type=typedef_type; s.value="uintptr";}
   {Symbol &s=*Symbols("wchar_t"  ); s.type=typedef_type; s.value="char"   ;}
   {Symbol &s=*Symbols("char16_t" ); s.type=typedef_type; s.value="char"   ;}
   {Symbol &s=*Symbols("char32_t" ); s.type=typedef_type; s.value="uint"   ;}
   {Symbol &s=*Symbols("__int8"   ); s.type=typedef_type; s.value="sbyte"  ;}
   {Symbol &s=*Symbols("__int16"  ); s.type=typedef_type; s.value="short"  ;}
   {Symbol &s=*Symbols("__int32"  ); s.type=typedef_type; s.value="int"    ;}
   {Symbol &s=*Symbols("__int64"  ); s.type=typedef_type; s.value="long"   ;}
   {Symbol &s=*Symbols("int8_t"   ); s.type=typedef_type; s.value="sbyte"  ;}
   {Symbol &s=*Symbols("uint8_t"  ); s.type=typedef_type; s.value="byte"   ;}
   {Symbol &s=*Symbols("int16_t"  ); s.type=typedef_type; s.value="short"  ;}
   {Symbol &s=*Symbols("uint16_t" ); s.type=typedef_type; s.value="ushort" ;}
   {Symbol &s=*Symbols("int32_t"  ); s.type=typedef_type; s.value="int"    ;}
   {Symbol &s=*Symbols("uint32_t" ); s.type=typedef_type; s.value="uint"   ;}
   {Symbol &s=*Symbols("int64_t"  ); s.type=typedef_type; s.value="long"   ;}
   {Symbol &s=*Symbols("uint64_t" ); s.type=typedef_type; s.value="ulong"  ;}
   {Symbol &s=*Symbols("signed"   ); s.type=typedef_type; s.value="int"    ;}
   {Symbol &s=*Symbols("unsigned" ); s.type=typedef_type; s.value="uint"   ;}

   FREPA(PreprocKeywords)
   {
      SymbolDef &s=SymbolDefs.New().require(S+'#'+PreprocKeywords[i]);
      s->type=Symbol::PREPROC;
      FlagSet(s->modifiers, Symbol::MODIF_FOLLOW_BY_SPACE, *s!="#else" && *s!="#endif");
   }

   SystemMacros.New().set("__cplusplus");
   SystemMacros.New().set("_WIN32"); // for constant behavior across platforms, define _WIN32 everywhere
   SystemMacros.sort(CompareCS); // macros need to be sorted

   // setup gui
   if(T.parent) // only if we're adding it to parent, so we can create Code Editor in Esenthel Builder without using GUI at all
   {
      ts.reset();
      ts.align.set(1, -1);
      ts.shade  =255;
      ts.spacing=SPACING_CONST;

      ts_small=ts;

      skinChanged();

      Node<MenuElm> menu_elms;
      setMenu(menu_elms);
     *parent+=menu.create(menu_elms).disabled(true);

     *parent+=b_close.create().func(MenuClose).hide().desc("Close current file"); b_close.image="Gui/close.img"; b_close.skin=&EmptyGuiSkin;

      Node<MenuElm> view_what_elms;
      view_what_elms.New().create("Element Names"  , ViewElmNames      ).flag(MENU_TOGGLABLE            ).setOn(view_elm_names      ).desc("Display project element names on top of their ID in the codes\nKeyboard Shortcut: Alt+E");
      view_what_elms++;
      view_what_elms.New().create("Comments"       , ViewComments      ).flag(MENU_TOGGLABLE            ).setOn(view_comments       ).desc("Show Comments");
      view_what_elms.New().create("Functions"      , ViewFuncs         ).flag(MENU_TOGGLABLE|MENU_HIDDEN).setOn(view_funcs          ).desc("Show Functions");
      view_what_elms.New().create("Function Bodies", ViewFuncBodies    ).flag(MENU_TOGGLABLE            ).setOn(view_func_bodies    ).desc("Show Function Bodies\nKeyboard Shortcut: Alt+B");
      view_what_elms.New().create("Private Members", ViewPrivateMembers).flag(MENU_TOGGLABLE            ).setOn(view_private_members).desc("Show Class Private Members");

     *parent+=view_mode.create("View Mode").func(RefreshSourceLines, T).focusable(false).desc("Keyboard Shortcut: Alt+V");
     *parent+=view_what.create().setData(view_what_elms); view_what.menu_align=-1; view_what.flag|=COMBOBOX_CONST_TEXT;
      view_mode.mode=b_find.mode=BUTTON_TOGGLE;

      goto_line_window   .create(T);
      options            .create(T);
      android_certificate.create(T);
      {
         Gui+=build_progress.create();
         Gui+=build_region.create(); build_region.kb_lit=false;
         ListColumn lc[]=
         {
            ListColumn(MEMBER(BuildResult, text), LCW_MAX_DATA_PARENT, "Output"),
         };
         build_region+=build_list  .create(lc, Elms(lc)).skin(&cjk_skin); build_list.column(0).disabled(true); build_list.cur_mode=LCM_MOUSE; build_list.setElmTextColor(MEMBER(BuildResult, color));
         Gui         +=build_close .create().func(BuildClose, T); build_close.image="Gui/close.img"; build_close.skin=&EmptyGuiSkin;
         Gui         +=build_export.create("Export").func(OutputExport, T);
         Gui         +=build_copy  .create("Copy"  ).func(OutputCopy  , T);
         Gui         +=build_io    .create("txt", S, SystemPath(SP_DESKTOP), OutputExport, OutputExport, T); build_io.textline.set("Output.txt");
      }
      {
         Gui+=devlog_region.create(); devlog_region.kb_lit=false;
         ListColumn lc[]=
         {
            ListColumn(MEMBER(DeviceLog, time   ), 0.20f, "Time"       ),
            ListColumn(MEMBER(DeviceLog, app    ), 0.25f, "Application"),
            ListColumn(MEMBER(DeviceLog, message), LCW_MAX_DATA_PARENT, "Message"),
         };
         lc[0].sort=&devlog_time_sort; devlog_time_sort.setDataToTextFunc(DeviceLogOrder); // when sorting by time, then actually sort by order in the container
         devlog_region+=devlog_list  .create(lc, Elms(lc)); devlog_list.cur_mode=LCM_MOUSE; devlog_list.setElmTextColor(MEMBER(DeviceLog, color));
         Gui          +=devlog_close .create(          ).func(DevlogClose  , T); devlog_close.image="Gui/close.img"; devlog_close.skin=&EmptyGuiSkin;
         Gui          +=devlog_filter.create("App Only").func(DevlogFilter2, T).desc("Display messages only from your application"); devlog_filter.mode=BUTTON_TOGGLE;
         Gui          +=devlog_export.create("Export"  ).func(DevlogExport , T);
         Gui          +=devlog_clear .create("Clear"   ).func(DevlogClear  , T);
         Gui          +=devlog_io    .create("txt", S, SystemPath(SP_DESKTOP), DevlogExport, DevlogExport, T); devlog_io.textline.set("Android Device Log.txt");
      }
      find   .create();
      replace.create();
      save_changes.create();

      visibleOutput(false);
      visibleOpenedFiles(false);
      visibleAndroidDevLog(false);
      resize();
   }

   if(!loadSymbols(CodeEditorDat, false)){}
}
void CodeEditor::genSymbols(C Str &ee_editor_bin)
{
   ee_h_path=Str(ee_editor_bin).tailSlash(true)+"EsenthelEngine";
   FList(ee_h_path, AddEEHeader, T); // add all headers from folder to 'items'

   EEUsings.clear();
   EEMacros=SystemMacros;
   parseHeader(ee_h_path+"\\EsenthelEngine.h", EEMacros, EEUsings, true);
   // TODO: remove this after adding support for detection of "using namespace .." in sources
   EEUsings.New().find("EE");

   replacePath(ee_h_path, EE_PATH);
   SortItems(items);

   // remove macros
   CChar8 *remove_macros[]=
   {
      "super"         , // use keyword instead of macro
      "null"          , // use keyword instead of macro
      "null_t"        , // use keyword instead of macro
      "NULL"          , // use 'null' keyword
      "STRUCT"        , // use "class A : B"
      "STRUCT_PRIVATE", // use "class A : private B"
   };
#if 1
   REPAD(d, remove_macros)REPA(EEMacros)if(Equal(EEMacros[i].name, remove_macros[d], true))EEMacros.remove(i, true);
#else
   REPAD(d, disable_macros)REPA(EEMacros)if(Equal(EEMacros[i].name, disable_macros[d], true))EEMacros[i].use_for_suggestions=false;
#endif

   // disable EE basic data types suggestions
   SymbolPtr s;
   if(s.find("Bool"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find( "Char8" )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("CChar8" )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find( "Char"  )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("CChar"  )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("I8"     )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("U8"     )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("I16"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("U16"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("I32"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("U32"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("I64"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("U64"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("SByte"  )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("Byte"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("Short"  )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("UShort" )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("Int"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("UInt"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("Long"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("ULong"  )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find( "IntPtr")){FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS); s->value= "intptr";} // replace typedef with  "intptr" because that one is changed for 32/64 compilation
   if(s.find("UIntPtr")){FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS); s->value="uintptr";} // replace typedef with "uintptr" because that one is changed for 32/64 compilation
   if(s.find("Flt"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("Dbl"    )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find( "Ptr"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);
   if(s.find("CPtr"   )) FlagEnable(s->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);

   // disable suggestions for all symbols starting with a '_', because they're meant as hidden/private, but not all can be hidden in EE_PRIVATE (such as _Sort, _GameAnalytics, etc.)
   Symbols.  lock(); REPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol[0]=='_')FlagEnable(symbol.modifiers, Symbol::MODIF_SKIP_SUGGESTIONS);}
   Symbols.unlock();

   saveSymbols(Str(ee_editor_bin).tailSlash(true)+"Code Editor.dat");

 //createFuncList();
}
/******************************************************************************/
void CodeEditor::init()
{
   if(!symbols_loaded)
   {
      symbols_loaded=true;

      // setup defaults
      EEMacros=SystemMacros;

      // load
   #if LOAD_SYMBOLS
      if(!loadSymbols(CodeEditorDat))Exit("Error loading \"Code Editor.dat\".");
   #endif

      // setup project basing on libs
      ProjectMacros=LibMacros=EEMacros;
      ProjectUsings=LibUsings=EEUsings;

      // rebuild symbols of all sources that were loaded before the init
      rebuildSymbols(true);
   }
}
/******************************************************************************/
Flt  CodeEditor:: lineNumberSize()C {return options.line_numbers() ? ts.lineHeight()*2.3f : 0;}
Flt  CodeEditor::fontSpaceOffset()C {return (1-CE.ts.space.y)*CE.ts.size.y*0.5f;}
Rect CodeEditor::sourceRect     ()
{
   Rect   r(-D.w(), build_region.visible() ? build_region.rect().max.y : -D.h(), D.w(), D.h());
   if(menu.visibleFull())if(menu_on_top)MIN(r.max.y, menu.rect().min.y);else MAX(r.min.y, menu.rect().max.y); // check 'visibleFull' in case 'menu' is attached to a Tabs::Tab
   return r&cei().sourceRect();
}
Source* CodeEditor::findSource(C SourceLoc &loc) {if(loc.is())REPA(sources)if(sources[i].loc==loc)return &sources[i]; return null;}
Source* CodeEditor:: getSource(C SourceLoc &loc, ERROR_TYPE *error)
{
   Source *s=findSource(loc);
   if(    !s && loc.is())
   {
      s=&sources.New();
      ERROR_TYPE e=s->load(loc); if(error)*error=e;
      if(e!=EE_ERR_NONE){sources.removeData(s, true); s=null;}
   }
   return s;
}

Int     CodeEditor::curI() {return sources.validIndex(_cur);}
Source* CodeEditor::cur () {return                    _cur ;}

void CodeEditor::nextFile() {Source *src=null; Int offset=curI()+1; FREPA(sources){Source &s=sources[Mod(offset+i, sources.elms())]; if(s.opened){src=&s; break;}} cur(src);} // activate next opened source
void CodeEditor::prevFile() {Source *src=null; Int offset=curI()-1; FREPA(sources){Source &s=sources[Mod(offset-i, sources.elms())]; if(s.opened){src=&s; break;}} cur(src);} // activate prev opened source

Str CodeEditor::title()
{
   if(cur())
   {
      Str path=cur()->loc.asText();
      if( path.is())return S+'"'+path+(cur()->modified() ? '*' : '\0')+'"'+(cur()->Const ? " (Read Only)" : null);
   }
   return S;
}

Str CodeEditor::adbPath()C
{
   if(android_sdk.is())
   {
      CChar8 *name=PLATFORM("adb.exe", "adb");
      Str sdk=Str(android_sdk).tailSlash(true);
      Str p=sdk+"platform-tools/"+name; if(FExistSystem(p))return p;
   }
   return S;
}
Str CodeEditor::zipalignPath()C
{
   if(android_sdk.is())
   {
      CChar8 *name=PLATFORM("zipalign.exe", "zipalign");
      Str sdk=Str(android_sdk).tailSlash(true);
      Str p=sdk+"tools/"+name; if(FExistSystem(p))return p;
      for(FileFind ff(sdk+"build-tools"); ff(); )p=ff.pathName()+'/'+name; if(FExistSystem(p))return p;
   }
   return S;
}
Str CodeEditor::ndkBuildPath()C
{
   if(android_ndk.is())
   {
      CChar8 *name=PLATFORM("ndk-build.cmd", "ndk-build");
      Str ndk=Str(android_ndk).tailSlash(true);
      Str p=ndk+name; if(FExistSystem(p))return p;
   }
   return S;
}
/******************************************************************************/
static Bool EmbedAppResource(C Str &dest, File &src, UInt type) // used only for Linux apps
{
   const UInt CC4_CHNK=CC4('C', 'H', 'N', 'K');
   File f; if(f.appendTry(dest))
   {
      Long pos=f.pos();
      f.putUInt(CC4_CHNK);
      f.putUInt(4+src.left()); // type + src
      f.putUInt(type); // type
      if(src.copy(f)) // src
      {
         UInt skip=f.pos()-pos+4*2; // we're writing 2 UInt's:
         f.putUInt(skip    );
         f.putUInt(CC4_CHNK);
         return f.flushOK();
      }
   }
   return false;
}
static Bool EmbedAppResource(C Str &dest, C Str &src, UInt type)
{
   File f; if(f.readStdTry(src))return EmbedAppResource(dest, f, type);
   return false;
}
void CodeEditor::BuildResult::setWarning() {mode=1; setColor();}
void CodeEditor::BuildResult::setError  () {mode=2; setColor();}
void CodeEditor::BuildResult::setColor  ()
{
   switch(mode)
   {
      case  1: color.set(128, 128, 0); break;
      case  2: color=RED; break;
      default: if(GuiSkin *skin=Gui.skin())if(TextStyle *ts=skin->text.text_style()){color=ts->color; break;} color=BLACK; break;
   }
}
void CodeEditor::DeviceLog::setWarning() {mode=WARNING; setColor();}
void CodeEditor::DeviceLog::setError  () {mode=ERROR  ; setColor();}
void CodeEditor::DeviceLog::setApp    () {mode=APP    ; setColor();}
void CodeEditor::DeviceLog::setColor  ()
{
   switch(mode)
   {
      case WARNING: color.set(128, 128, 0); break;
      case ERROR  : color=RED; break;
      case APP    : color.set(0, 128, 0); break;
      default     : if(GuiSkin *skin=Gui.skin())if(TextStyle *ts=skin->text.text_style()){color=ts->color; break;} color=BLACK; break;
   }
}
static Str FindPath(C Str &registry, C Str &sub_path)
{
   Str path=GetRegStr(RKG_LOCAL_MACHINE, registry); if(path.is())
   {
      path.tailSlash(true)+=sub_path;
      if(FExistSystem(path))return path;
   }
   return S;
}
void CodeEditor::update(Bool active)
{
   if(active)
   {
      if(Gui.kb()==&build_list)if(cur())cur()->activate();

      if(Ms.bp(2) && build_region .contains(Gui.ms()))visibleOutput       (false);
      if(Ms.bp(2) && devlog_region.contains(Gui.ms()))visibleAndroidDevLog(false);

      if(cur())cur()->suggestionsSetRect();

      // highlight element containing token under mouse cursor
      SourceLoc elm_highlight;
      if(cur() && cur()->hasMsFocus() && CE.view_mode())
      {
         VecI2 view=Trunc(cur()->posCur(Ms.pos())), real;
         if(cur()->viewToReal(view, real))
         {
            SymbolPtr symbol;
            Macro    *macro=null;
            UID       id;
            if(cur()->getSymbolMacroID(real, symbol, macro, id))
            {
               if(symbol && symbol->source)elm_highlight=symbol->source->loc;else
               if(macro  && macro ->source)elm_highlight=macro ->source->loc;else
               if(id.valid()              )elm_highlight=id;
            }
         }
      }
      cei().elmHighlight(elm_highlight.id, elm_highlight.file_name);

      // auto-hide menu
      if(options.auto_hide_menu())setMenuBarVisibility();

   #if 0 // display 'view_what.menu' when hovering over the 'view_mode' button
      if(!view_what.button()())
      {
         Bool make_vis=(view_mode.contains(Gui.ms()) || view_what.menu.contains(Gui.ms()));
         if(view_what.menu.visible()!=make_vis)
         {
            if(make_vis)view_what.menu.posAround(view_what.screenRect(), view_what.menu_align);
            view_what.menu.visibleToggleActivate();
         }
      }
   #endif
   }

      build_progress.visible(build_process.created());
   if(build_process.created())
   {
      build_progress.set(build_steps ? build_step+1 : build_phase+1, build_steps ? build_steps+1 : build_phases+1);
      Bool active =build_process.active(), // get information about active before reading output
           refresh=false;
      if(build_log.is()) // read from file
      {
         if((build_refresh-=Time.rd())<=0 || !active)
         {
            build_refresh=0.25f;
            refresh=true;
         }
      }else // read from process
      {
         Str output=build_process.get();
         if( output.is())
         {
            FREPA(output)
            {
               Char c=output[i];
               if(c=='\t')build_output+="   ";else
               if(c==L'‘')build_output+='\'' ;else // occurs on Linux
               if(c==L'’')build_output+='\'' ;else // occurs on Linux
                          build_output+=c;
            }
            refresh=true;
         }
      }
      Bool at_end=build_region.slidebar[1].wantedAtEnd(0.02f);
      if(refresh)
      {
         Memc<Str> lines;
         if(build_log.is()) // read from file
         {
            FileText f; if(f.read(build_log))for(; !f.end(); )f.fullLine(lines.New());
         }else // read from process
         {
            lines=Split(build_output, '\n');
         }
         buildClear();
         if(build_exe_type==EXE_EXE || build_exe_type==EXE_DLL || build_exe_type==EXE_LIB || build_exe_type==EXE_NEW || build_exe_type==EXE_MAC || build_exe_type==EXE_IOS || build_exe_type==EXE_LINUX || build_exe_type==EXE_WEB)build_step=0; // we're going to count compilation progress for these platforms
         FREPA(lines)
         {
          C Str &line=lines[i];
            if(SkipWhiteChars(line).is())
               if(build_exe_type!=EXE_APK || // hide some Android compiler/linker warnings
                  !Contains(line, "uses 2-byte wchar_t yet the output is to use 4-byte wchar_t; use of wchar_t values across objects may fail")
               && !Contains(line, "uses 4-byte wchar_t yet the output is to use 2-byte wchar_t; use of wchar_t values across objects may fail")
               )
               if(!Contains(line, "manifest authoring warning 81010002:"))
            {
               BuildResult &br=buildNew().set(line);
               if(Contains(line,   "warning:", false, true)
               || Contains(line, ": warning" , false, true))br.setWarning();else
               if(Contains(line,   "error:"  , false, true)
               || Contains(line, ": error "  , false, true))br.setError  ();

               // count sources compiled
               if(build_exe_type==EXE_EXE || build_exe_type==EXE_DLL || build_exe_type==EXE_LIB || build_exe_type==EXE_NEW || build_exe_type==EXE_WEB)
               {
                  if(build_msbuild)
                  {
                     if(line[0]==' ' && line[1]==' ')
                     {
                        if(Ends(line, ".cpp"))
                        {
                           CChar *l=_SkipWhiteChars(line()+2);
                           if(CleanFileName(l)==l)build_step++;
                        }else
                        if(line=="  Generating Code...")build_step++;
                     }
                  }else
                  FREPA(line)
                  {
                     if(CharFlag(line[i])&CHARF_DIG){}else
                     if(line[i]=='>')
                     {
                        Str l=SkipWhiteChars(line()+i+1);
                        if(Ends(l, ".cpp") && CleanFileName(l)==l || Equal(l, "linking..."))build_step++;
                     }else break;
                  }
               }else
               if(build_exe_type==EXE_MAC || build_exe_type==EXE_IOS)
               {
                  if(Starts(line, "CompileC", true, true)
                  || Starts(line, "Ld"      , true, true))build_step++;
                  if(Starts(line, "Undefined symbols", true, true)
                  || Starts(line, "ld: framework not found"      ))br.setError();
               }else
               if(build_exe_type==EXE_LINUX)
               {
                  if(Starts(line, "g++"                     , true, true)
                  || Starts(line, "clang++"                 , true, true))build_step++;
                  if(Starts(line, "/usr/bin/ld: cannot find", true, true))br.setError();
               }
            }
         }
         buildUpdate(false);
         if(at_end)build_region.scrollEndY(); // auto-scroll to the end
      }
      if(!active) // process finished building
      {
         Int exit_code=build_process.exitCode();
         build_process.del();
         Bool was_log;
         if(was_log=build_log.is()){FDelFile(build_log); build_log.clear();}
         if(build_exe_type==EXE_EXE || build_exe_type==EXE_DLL || build_exe_type==EXE_LIB || build_exe_type==EXE_NEW || build_exe_type==EXE_WEB)
         {
            Bool ok=false;
            if(build_phase==0)
            {
               if(exit_code>=0)ok=(exit_code==0);else // exit code known
               if(build_data.elms())                  // detect success
               {
                C Str &last=build_data.last().text;
                  if(Starts(last, "==="))
                  {
                     // only on success
                     ok=(((Contains(last,  "1 succeeded"  ) || Contains(last,  "1 up-to-date"    )) && Contains(last,  "0 failed"      ))   // English (========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========)
                      || ((Contains(last,  "1 erfolgreich") || Contains(last,  "1 aktuell"       )) && Contains(last,  "Fehler bei 0"  ))   // German  (========== Build: 1 erfolgreich, Fehler bei 0, 0 aktuell, 0 übersprungen ==========)
                      || ((Contains(last, u"1 a réussi"   ) || Contains(last, u"1 mis à jour"    )) && Contains(last, u"0 a échoué"    ))   // French  (========== Génération : 1 a réussi, 0 a échoué, 0 mis à jour, 0 a été ignoré ==========)
                      || ((Contains(last,  "1 completate" ) || Contains(last,  "1 aggiornate"    )) && Contains(last,  "0 non riuscite"))   // Italian (========== Compilazione: 1 completate, 0 non riuscite, 0 aggiornate, 0 ignorate ==========)
                      || ((Contains(last, u"успешно: 1"   ) || Contains(last, u"без изменений: 1")) && Contains(last, u"с ошибками: 0" ))); // Russian (========== Построение: успешно: 1, с ошибками: 0, без изменений: 0, пропущено: 0 ==========)
                  }
               }
               if(1 && ok) // store hash, !! have to do this before Code Signing, because we modify the EXE file, otherwise it would make the signature invalid !!
                  if(!build_debug) // skip for DEBUG because it interferes with incremental linking
                  if(build_exe_type==EXE_EXE || build_exe_type==EXE_DLL)
               {
                  File f; if(f.readStdTry(build_exe))
                  {
                     Memc<ExeSection> sections; if(ParseExe(f, sections))
                     {
                        xxHash64 file_hash;
                      C ExeSection *section_hash_ptr=null;
                        FREPA(sections) // process in order
                        {
                         C ExeSection &section=sections[i]; switch(section.type)
                           {
                              case ExeSection::VARIABLE: break; // don't calc hash because this section data can be outside of file range
                              case ExeSection::HASH    : section_hash_ptr=&section; break; // remember HASH section for use later
                              default:
                              {
                                 if(!f.pos(section.offset))goto hash_error;
                                 ULong section_hash=f.xxHash64(section.size); if(!f.ok())goto hash_error; // get hash of this section
                                 file_hash.update(&section_hash, SIZE(section_hash)); // update file hash based on section hash, use this method so in the app we can just once calculate hash per section just like here, and combine section hashes for final hash
                              }break;
                           }
                        }
                        if(section_hash_ptr && (build_debug ? section_hash_ptr->size>=SIZE(ULong) : section_hash_ptr->size==SIZE(ULong))) // when building in DEBUG mode, some extra data may be allocated
                        {
                           ULong hash=file_hash();
                           if(Cipher *cipher=CE.cei().appEmbedCipher())cipher->encrypt(&hash, &hash, SIZE(hash), 0); // encrypt with project cipher
                           if(!f.pos(section_hash_ptr->offset))goto hash_error;
                           if( f.getULong()==hash)goto hash_ok; // file already has correct hash, check this in case we are building for 2nd time and file was already adjusted, perhaps that EXE is already running so we can't modify it
                           if( f.appendTry(build_exe) && f.pos(section_hash_ptr->offset)) // modify hash
                           {
                              f.putULong(hash);
                              if(f.flushOK())goto hash_ok;
                           }
                        }
                     }
                  }
               hash_error: BuildError("Error: Can't set EXE hash", at_end); //ok=false; ignore clearing 'ok' to allow proceeding in case hash is not needed by the user
               hash_ok   :;
               }
               if(ok)
               {
                  build_phase++;
                  if(build_windows_code_sign) // !! have to do this after storing Hash !! CodeSigning will not change any ExeSection offset, size or data
                  {
                     Str               signtool=FindPath("Software/Microsoft/Windows App Certification Kit/InstallLocation"  , "signtool.exe");
                     if(!signtool.is())signtool=FindPath("Software/Microsoft/Microsoft SDKs/Windows/v10.0/InstallationFolder", "App Certification Kit\\signtool.exe");
                     if(!signtool.is())signtool=FindPath("Software/Microsoft/Windows Kits/Installed Roots/KitsRoot10"        , "App Certification Kit\\signtool.exe");
                     if(!build_process.create(signtool, S+"sign /a \""+build_exe+'"'))
                     {
                        BuildError("Error: Can't do Windows Code Sign - SignTool (signtool.exe) was not found. Please download it from the internet", at_end);
                     }else // copy current output so it will be displayed for next phase as well
                     {
                        if(was_log)FREPA(build_data)build_output.line()+=build_data[i].text;
                        build_output.line();
                     }
                  }
               }
            }else
            if(build_phase==(build_windows_code_sign ? 1 : -2))
            {
               ok=(exit_code==0);
               if(ok)build_phase++;
            }
            if(ok && build_phase==(build_windows_code_sign ? 2 : 1))switch(build_mode)
            {
               case BUILD_PLAY:
               {
                  switch(build_exe_type)
                  {
                     case EXE_EXE: Run(build_exe); break;
                     case EXE_WEB: goto publish; // when playing for WEB we will create PAK's after compilation, so call publish success to do so
                  }
               }break;
             //case BUILD_DEBUG  : if(build_exe_type==EXE_EXE)VSRun(build_project_file); break; // no need to call this because building was done by launching VS and it will automatically run the app
               case BUILD_PUBLISH: publish: cei().publishSuccess(build_exe, build_exe_type, build_mode, build_project_id); break;
            }
         }else
         if(build_exe_type==EXE_LINUX)
         {
            Bool ok=(exit_code==0); // exit code is available on Linux
            if(  ok)
            {
               // on Linux we had to use temp exe name because 'make' fails for some characters
               Str new_exe=GetPath(build_exe).tailSlash(true)+build_project_name; // set new name
               if(!FRename(build_exe, new_exe))Gui.msgBox(S, S+"Can't rename file from: "+build_exe+"\nto: "+new_exe);else
               {
                  build_exe=new_exe;

                  // embed app resources
                  FREPA(build_embed)
                  {
                     BuildEmbed &be=build_embed[i];
                     if(!EmbedAppResource(build_exe, be.path, be.type)){Gui.msgBox(S, "Can't embed app resource"); ok=false; break;}
                  }
                  build_embed.clear();

                  if(ok)switch(build_mode)
                  {
                     case BUILD_PLAY   :
                     case BUILD_DEBUG  : Run(build_exe); break;
                     case BUILD_PUBLISH: cei().publishSuccess(build_exe, build_exe_type, build_mode, build_project_id); break;
                  }
               }
            }
         }else
         if(build_exe_type==EXE_MAC)
         {
            Bool ok=false;
          //if(exit_code>=0)ok=(exit_code==0);else                                     // exit code known
            if(build_data.elms())ok=(build_data.last().text=="** BUILD SUCCEEDED **"); // detect success
            if(ok)switch(build_mode)
            {
               case BUILD_PLAY   :
               case BUILD_DEBUG  : Run(build_exe); break;
               case BUILD_PUBLISH: cei().publishSuccess(build_exe, build_exe_type, build_mode, build_project_id); break;
            }
         }else
         if(build_exe_type==EXE_APK)
         {
            if(build_phase==0) // link with ant
            {
               // can't use 'exit_code' because it's always zero, #AndroidArchitecture
             //if(!FExistSystem(build_path+"Android/libs/armeabi/libProject.so"    ))BuildError("Failed to build armeabi shared library"    , at_end);else
               if(!FExistSystem(build_path+"Android/libs/armeabi-v7a/libProject.so"))BuildError("Failed to build armeabi-v7a shared library", at_end);else
               if(!FExistSystem(build_path+"Android/libs/arm64-v8a/libProject.so"  ))BuildError("Failed to build arm64-v8a shared library"  , at_end);else
             //if(!FExistSystem(build_path+"Android/libs/x86/libProject.so"        ))BuildError("Failed to build x86 shared library"        , at_end);else
               {
                  if(build_exe.is()){build_exe+=(build_debug ? "-debug.apk" : "-release-unsigned.apk"); FDelFile(build_exe);} // delete it so we can know that the build was ok if it reappears
                  build_phase++;
                  build_process.create(GetPath(App.exe())+PLATFORM("/Bin/Android/Ant/bin/ant.bat", "/Bin/Android/Ant/bin/ant"), S+(build_debug ? "debug" : "release")+" -f \""+build_path+"Android\\build.xml\""+(build_log.is() ? S+" > \""+build_log+"\"" : S)); // Mac script "Android/Ant/bin/ant" must have changed "JAVA_HOME=/System/Library/Frameworks/JavaVM.framework/Home" to "JAVA_HOME=$(/usr/libexec/java_home)" !! (without this building Android apps won't work on Mac)
               }
            }else
            if(build_phase==(build_debug ? -2 : 1)) // sign with jarsigner
            {
               Bool ok=FExistSystem(build_exe); // can't use 'exit_code' because it's always zero
               if(  ok) // build succeeded
               {
                  build_phase++;
                  build_process.create(PLATFORM(jdk_path.tailSlash(true)+"bin/jarsigner.exe", "jarsigner"), S+"-verbose -sigalg MD5withRSA -digestalg SHA1 -storepass \""+cert_pass+"\" -keypass \""+cert_pass+"\" -keystore \""+cert_file+"\" \""+build_exe+"\" \"key\"");
               }
            }else
            if(build_phase==(build_debug ? -2 : 2)) // align with zipalign
            {
               if(exit_code==0) // sign succeeded
               {
                  // force stop previous installation
                  Str src=build_exe, dest=SkipEnd(src, "-release-unsigned.apk")+".apk"; FDelFile(dest); build_exe=dest; // delete it so we can know that the align was ok if it reappears
                  build_phase++;
                  build_process.create(zipalignPath(), S+"-v 4 \""+src+"\" \""+dest+"\"");
               }else
               {
                  FDelFile(build_exe);
               }
            }else
            if(build_phase==(build_debug ? 1 : 3)) // force stop
            {
               if(build_debug) // rename "*-debug.apk" ?
               {
                  
               }else // remove "*-release-unsigned.apk"
               {
                  FDelFile(SkipEnd(build_exe, ".apk")+"-release-unsigned.apk");
               }
               Bool ok=FExistSystem(build_exe); // can't use 'exit_code' because it's always zero
               if(  ok)switch(build_mode) // build succeeded
               {
                  case BUILD_PLAY :
                  case BUILD_DEBUG:
                  {
                     // force stop previous installation
                     build_phase++;
                     build_process.create(adbPath(), S+"shell am force-stop \""+build_package+"\"");
                  }break;

                  case BUILD_PUBLISH: cei().publishSuccess(build_exe, build_exe_type, build_mode, build_project_id); break;
               }
            }else
            if(build_phase==(build_debug ? 2 : 4)) // install on the device
            {
               build_phase++;
            install:
               build_process.create(adbPath(), S+"-d install -r \""+build_exe+"\""); // -r does reinstall if already exists
            }else
            if(build_phase==(build_debug ? 3 : 5)) // start on the device
            {
               Bool ok=false; // can't use 'exit_code' because it's always zero
               FREP(Min(build_data.elms(), 2)) // check 2 last messages
               {
                C Str &s=build_data[build_data.elms()-1-i].text;
                  if(Contains(s, "Success")){ok=true; break;}
               }
               if(!ok && build_data.elms() && build_data.last().text=="- waiting for device -")goto install; // if just connected to a device, then try again
               if(ok)
               {
                  // start on the device without -n parameter
                  // the command has optional -n parameter which it seems that forces resume of an application (and its last memory state) from the AndroidMain (start) without any closing/resetting
                  // without this parameter Android 2.3 has trouble to start the app (intent not detected)
                  // that's why in 1st attempt start is tried without the parameter, if it will fail, then it's started with the parameter
                  // in order to avoid app weird behaviour with the -n parameter, engine detects at startup (in AndroidMain) if Jni is null
                  // if yes then it means that app memory state is ok or it was shut down properly, if not then it calls Exit immediattely
                  // the "shell am start" command works that if the app exited immediattely then it tries to launch it again, so even in case of Exit the app will be restarted
                  // side note: when trying to solve the memory issue by clearing java values to null helped, however when trying to activate the opengl context it seemed to be ok (in 'androidOpen'), however when using it in the app loop, the open gl context seemed null
                  build_phase++;
                  build_process.create(adbPath(), S+"shell am start \""+build_package+"/.LoaderActivity\"");
               }
            }else
            if(build_phase==(build_debug ? 4 : 6)) // activate logcat
            {
               visibleAndroidDevLog(true);
               if(build_data.elms())
               {
                  if(Contains(build_data.last().text, "Activity not started, unable to resolve Intent"))
                  {
                     // start on the device thout -n parameter
                     build_phase++;
                     build_process.create(adbPath(), S+"shell am start -n \""+build_package+"/.LoaderActivity\""); // start with -n this time
                  }
               }
            }
         }
      }
   }
   if(devlog_process.created())
   {
      Str log=devlog_process.get();
      if( log.is())
      {
         devlog_text+=log;
         Memc<Str> lines=Split(devlog_text, '\n');
         if(lines.elms()>=2)
         {
            Bool at_end=devlog_region.slidebar[1].wantedAtEnd(0.02f);
            FREP(lines.elms()-1)
            {
               Str &l=lines[i]; // "mm-dd hh:mm:ss.123 D/name( pid): msg
               if(l[0]!='-')if(CChar *time=TextPos(l, ' ')) // skip date "mm-dd"
               {
                  time++;
                  if(CChar *debug=TextPos(time, ' '))
                  {
                     debug++;
                     DeviceLog &dl=devlog_data.New();
                     Char t[9]; Set(t, time); dl.time=t;
                     if(CChar *app=TextPos(debug, '/'))
                     {
                        app++;
                        if(CChar *pid=TextPos(app, '('))
                        {
                           for(; app<pid; )dl.app+=*app++; for(;dl.app.last()==' ';)dl.app.removeLast();
                           if(CChar *msg=TextPos(pid, ':'))
                           {
                              msg++; if(*msg==' ')msg++;
                              dl.message=msg;
                           }
                        }
                     }
                     if(Equal(dl.app, "Esenthel") )dl.setApp    ();else
                     if(*debug=='D'               )dl.setWarning();else
                     if(*debug=='E' || *debug=='F')dl.setError  ();
                     // filter out unwanted messages
                     if(Equal(dl.app, "AlarmManager")
                     || Equal(dl.app, "Lights") // on HTC control screen brightness
                     || Equal(dl.app, "Sensors") // gyroscope, accelerometer, etc
                   //|| Equal(dl.app, "Ethernet")
                   //|| Equal(dl.app, "WifiService")
                     || Equal(dl.app, "BatteryService")
                     || Equal(dl.app, "BATT_ALG")
                     || Equal(dl.app, "TabletStatusBar")
                     || Equal(dl.app, "CameraService")
                     || Equal(dl.app, "NvOmxCamera")
                     || Equal(dl.app, "NvOmxCameraBuffers")
                     || Equal(dl.app, "NvOmxCameraCallbacks")
                     || Equal(dl.app, "NvOmxCameraSettings")
                     || Equal(dl.app, "NvOmxCameraSettingsParser")
                     || Equal(dl.app, "PowerManagerService")
                     || Equal(dl.app, "DownloadManager")
                     || Equal(dl.app, "[WeatherService]")
                     || Equal(dl.app, "[WeatherService1]")
                     || Equal(dl.app, "[WeatherReceiver]")
                     || Equal(dl.app, "[WeatherSettingsAdapter]")
                     || Equal(dl.app, "Unity")
                     ||(Equal(dl.app, "DalvikVM") && (Starts(dl.message, "GC_EXPLICIT") || Starts(dl.message, "GC_CONCURRENT")))
                     ||(Equal(dl.app, "LocationManagerService") && Starts(dl.message, "getLastLocation: Request"))
                     )devlog_data.removeLast();
                  }
               }
            }
            devlog_text=lines.last();
            Int remove=Max(0, devlog_data.elms()-LOG_LINES), remove_vis=remove; // keep last LOG_LINES lines
            if(devlog_filter()){remove_vis=0; FREP(remove)remove_vis+=(devlog_data[i].mode==DeviceLog::APP);}
            devlog_data.removeNum(0, remove, true);
            if(devlog_list.lit>=0)devlog_list.lit-=remove_vis;
            if(devlog_list.cur>=0)devlog_list.cur-=remove_vis;
            if(!at_end)devlog_region.scrollY(-remove_vis*devlog_list.elmHeight(), true); // keep current position, call before 'DevlogFilter' so it can use current position to detect 'lit/cur'
            DevlogFilter(T);
            if(at_end)devlog_region.scrollEndY(); // auto-scroll to the end
         }
      }
   }
   if(adb_server.created() && !adb_server.active()) // ADB server finished starting
   {
      adb_server.del();
      if(!devlog_process.active() && adb_path.is())
      {
         devlog_data.New().message="Android Debug Bridge Started, Starting LogCat..";
         devlog_list.setData(devlog_data);
         devlog_process.create(adb_path, "logcat -v time");
      }
   }
}
/******************************************************************************/
void CodeEditor::draw()
{
   D.clearCol(cur() ? Theme.colors[TOKEN_NONE] : GREY);
}
/******************************************************************************/
void CodeEditor::rebuild3rdPartyHeaders()
{
   // reset headers (except EE headers)
   REPA(sources)
   {
      Source &src=sources[i];
      if(src.header && !src.ee_header)
      {
         src.header=false; // first clear 'header' before resetting symbols
         src.resetSymbols();
      }
      src.parse_count=0;
   }

   // parse all 3rd party headers
   LibUsings.clear();
   LibMacros=SystemMacros;
   Memc<Str> headers; // TODO: this should be set based on selected target platform
#if WINDOWS
   headers=GetFiles(cei().appHeadersWindows());
#elif MAC
   headers=GetFiles(cei().appHeadersMac());
#elif LINUX
   headers=GetFiles(cei().appHeadersLinux());
#endif
   FREPA(headers)parseHeader(headers[i], LibMacros, LibUsings, false);

   // remove system macros incompatible with EE
   REPA(LibMacros)if(LibMacros[i]=="min" || LibMacros[i]=="max")LibMacros.remove(i, true);

   // add stuff from EE
   REPA(EEUsings)LibUsings.include(EEUsings[i]); // include all usings defined by EE
   REPA(EEMacros)LibMacros.exclude(EEMacros[i]); // remove  all macros defined by EE (to avoid duplicates, and replace old ones with new ones)
   REPA(EEMacros)LibMacros.add    (EEMacros[i]); // add     all macros defined by EE

   LibMacros.sort(CompareCS); // macros need to be sorted

   ProjectMacros=LibMacros;
   ProjectUsings=LibUsings;
}
/******************************************************************************/
void CodeEditor::rebuildSymbols(Bool rebuild_3rd_party_headers)
{
   REPAO(sources).resetSymbols   ();

   if(rebuild_3rd_party_headers)rebuild3rdPartyHeaders();
   // setup custom macros
   Macro *macro; Int index;
   CChar8 *name="STEAM"  ; if(ProjectMacros.binarySearch(name, index, CompareCS))macro=&ProjectMacros[index];else macro=&ProjectMacros.NewAt(index).set(name); macro->def=cei().appPublishSteamDll (); macro->parts.setNum(1)[0].set(TOKEN_NUMBER, -1, &BStr().setBorrowed(macro->def));
           name="OPEN_VR"; if(ProjectMacros.binarySearch(name, index, CompareCS))macro=&ProjectMacros[index];else macro=&ProjectMacros.NewAt(index).set(name); macro->def=cei().appPublishOpenVRDll(); macro->parts.setNum(1)[0].set(TOKEN_NUMBER, -1, &BStr().setBorrowed(macro->def));

   REPAO(sources).detectDefines  ();
   REPAO(sources).preprocess     ();
   REPAO(sources).detectDataTypes();
   REPAO(sources).linkDataTypes  ();
   REPAO(sources).detectVarFuncs ();
}
/******************************************************************************/
void CodeEditor::validateActiveSources(Bool rebuild_3rd_party_headers) {cei().validateActiveSources();} // ignore 'rebuild_3rd_party_headers' for simplicity
/******************************************************************************/
static void CloseAll(Bool all_saved=true, Ptr user=null) {CE.closeAll();}

void CodeEditorInterface::del        () {CE.del();}
void CodeEditorInterface::clear      () {CE.closeAll(); CE.sources.removeData(CE.findSource(Str(AutoSource)), true); clearActiveSources(); activateApp(true);}
void CodeEditorInterface::create     (GuiObj &parent, Bool menu_on_top) {       CE.cei(T); CE.create(&parent, menu_on_top);}
void CodeEditorInterface::update     (Bool    active                  ) {       CE.update(active);}
void CodeEditorInterface::draw       (                                ) {       CE.draw  (      );}
void CodeEditorInterface::resize     (                                ) {       CE.resize(      );}
void CodeEditorInterface::skinChanged(                                ) {       CE.skinChanged ();}
Bool CodeEditorInterface::initialized(                                ) {return CE.symbols_loaded;}
Str  CodeEditorInterface::title      (                                ) {return CE.title (      );}
Str  CodeEditorInterface::appPath    (C Str &app_name                 ) {Str build_path, build_project_name; return CE.getBuildPath(build_path, build_project_name, &app_name) ? build_path : S;}
Str  CodeEditorInterface::androidProjectPakPath(                      ) {Str build_path, build_project_name; return CE.getBuildPath(build_path, build_project_name           ) ? build_path+"Android/assets/Project.pak" : S;}
Str  CodeEditorInterface::    iOSProjectPakPath(                      ) {Str build_path, build_project_name; return CE.getBuildPath(build_path, build_project_name           ) ? build_path+"Assets/Project.pak"         : S;}
Str  CodeEditorInterface::windowsProjectPakPath(                      ) {Str build_path, build_project_name; return CE.getBuildPath(build_path, build_project_name           ) ? build_path+"Project.pak"                : S;} // if we would set "Assets/Project.pak" then the file would be included inside the EXE including the "Assets" too
void CodeEditorInterface::saveChanges(                                                                                                ) {CE.saveChanges();}
void CodeEditorInterface::saveChanges(Memc<Edit::SaveChanges::Elm> &elms                                                              ) {CE.saveChanges(elms);}
void CodeEditorInterface::saveChanges(Memc<Edit::SaveChanges::Elm> &elms, void (*after_save_close)(Bool all_saved, Ptr user), Ptr user) {CE.save_changes.set(elms, after_save_close, user);}
void CodeEditorInterface::sourceRename     (C UID &id  ) {SourceLoc loc=id; if(Source *source=CE.findSource(loc))source->loc=loc;}
Bool CodeEditorInterface::sourceCur        (C Str &name) {CE.init(); return CE.load(name);} // 'init' in case we're loading source from data
Bool CodeEditorInterface::sourceCur        (C UID &id  ) {           return CE.load(id  );}
Bool CodeEditorInterface::sourceCurIs      (           ) {return CE.cur()!=null;}
Str  CodeEditorInterface::sourceCurName    (           ) {return CE.cur() ? CE.cur()->loc.file_name : S      ;}
UID  CodeEditorInterface::sourceCurId      (           ) {return CE.cur() ? CE.cur()->loc.id        : UIDZero;}
Bool CodeEditorInterface::sourceCurModified(           ) {return CE.cur() ? CE.cur()->modified()    : false  ;}
Bool CodeEditorInterface::sourceCurConst   (           ) {return CE.cur() ? CE.cur()->Const         : false  ;}
void CodeEditorInterface::projectsBuildPath(C Str &path) {CE.projects_build_path=path;} // !! 'projects_build_path' may get deleted in 'cleanAll' !!
void CodeEditorInterface::menuEnabled         (Bool on) {CE.menu.enabled(on);}
void CodeEditorInterface::hideAll             (       ) {CE.hideAll();}
Bool CodeEditorInterface::visibleOptions      (       ) {return CE.options.visible();}
void CodeEditorInterface::visibleOptions      (Bool on) {       CE.options.visibleActivate(on);}
Bool CodeEditorInterface::visibleOpenedFiles  (       ) {return CE.visibleOpenedFiles  (  );}
void CodeEditorInterface::visibleOpenedFiles  (Bool on) {       CE.visibleOpenedFiles  (on);}
Bool CodeEditorInterface::visibleOutput       (       ) {return CE.visibleOutput       (  );}
void CodeEditorInterface::visibleOutput       (Bool on) {       CE.visibleOutput       (on);}
Bool CodeEditorInterface::visibleAndroidDevLog(       ) {return CE.visibleAndroidDevLog(  );}
void CodeEditorInterface::visibleAndroidDevLog(Bool on) {       CE.visibleAndroidDevLog(on);}

void CodeEditorInterface::paste(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
{
   if(Source *src=CE.cur())
      if(!src->Const && src->contains(obj) && elms.elms())
   {
      Str text;
      Vec2 c=src->posCur(screen_pos); MAX(c.x, 0); MAX(c.y, 0);
      src->sel=src->sel_temp=-1;
      src->cur.set(Round(c.x), Trunc(c.y)); if(CE.view_mode())src->viewToReal(src->cur, src->cur);
      src->curClip();
      FREPA(elms)
      {
         if(i)
         {
            if(Kb.ctrlCmd())REP(src->cur.x)text+=' ';else text+=", ";
         }
         text+=elms[i].asCString();
         if(Kb.ctrlCmd())text+=",\n";
      }
      src->paste(&text);
   }
}
void CodeEditorInterface::paste(C Str &text, GuiObj *obj, C Vec2 &screen_pos)
{
   if(Source *src=CE.cur())
      if(!src->Const && src->contains(obj) && text.is())
   {
      Vec2 c=src->posCur(screen_pos); MAX(c.x, 0); MAX(c.y, 0);
      src->sel=src->sel_temp=-1;
      src->cur.set(Round(c.x), Trunc(c.y)); if(CE.view_mode())src->viewToReal(src->cur, src->cur);
      src->curClip();
      src->paste(&text);
   }
}
void CodeEditorInterface::paste(C Str &text)
{
   if(Source *src=CE.cur())
      if(!src->Const && text.is())src->paste(&text);
}

C Memx<Item>& CodeEditorInterface::items() {return CE.items;}

Bool CodeEditorInterface::importPaths       ()C {return CE.options.import_path_mode     ()>0;}
Bool CodeEditorInterface::importImageMipMaps()C {return CE.options.import_image_mip_maps()  ;}

void CodeEditorInterface::          play(                ) {CE.play();}
void CodeEditorInterface::         debug(                ) {CE.debug();}
void CodeEditorInterface::   runToCursor(                ) {CE.runToCursor();}
void CodeEditorInterface::       publish(                ) {CE.build(BUILD_PUBLISH);}
void CodeEditorInterface::         build(                ) {CE.build();}
void CodeEditorInterface::       rebuild(                ) {CE.rebuild();}
void CodeEditorInterface::rebuildSymbols(                ) {CE.rebuildSymbols(false);}
void CodeEditorInterface::         clean(                ) {CE.clean();}
void CodeEditorInterface::      cleanAll(                ) {CE.cleanAll();}
void CodeEditorInterface::          stop(                ) {CE.killBuild();}
void CodeEditorInterface::       openIDE(                ) {CE.openIDE();}
void CodeEditorInterface::   exportPaths(Bool    relative) {CE.options.export_path_mode.set(relative);}
Bool CodeEditorInterface::        Export(EXPORT_MODE mode) {if(CE.Export(mode, BUILD_EXPORT)){Explore(CE.build_path); return true;} return false;}

void     CodeEditorInterface::configDebug(Bool     debug) {       CE.configDebug(debug);}
Bool     CodeEditorInterface::configDebug(              ) {return CE.config_debug      ;}
void     CodeEditorInterface::config32Bit(Bool     bit32) {       CE.config32Bit(bit32);}
Bool     CodeEditorInterface::config32Bit(              ) {return CE.config_32_bit     ;}
void     CodeEditorInterface::configDX9  (Bool     dx9  ) {       CE.configDX9  (dx9  );}
Bool     CodeEditorInterface::configDX9  (              ) {return CE.config_dx9        ;}
void     CodeEditorInterface::configEXE  (EXE_TYPE exe  ) {       CE.configEXE  (exe  );}
EXE_TYPE CodeEditorInterface::configEXE  (              ) {return CE.config_exe        ;}

static void DrawPreview(C SourceLoc &loc)
{
   if(Source *source=CE.getSource(loc))
   {
      source->prepareForDraw();
      D.clip();

      GuiPC gpc;
      gpc.visible    =true;
      gpc.enabled    =true;
      gpc.clip       =CE.sourceRect();
      gpc.client_rect=gpc.clip;
      gpc.offset     =gpc.clip.lu();
      gpc.clip.draw(Theme.colors[TOKEN_NONE]);
      gpc.clip.draw(Color(0, 0, 0, 112), false);
      FREPAO(source->     lines).draw(gpc);
      FREPAO(source->view_lines).draw(gpc);
      D.clip();
   }
}
void CodeEditorInterface::sourceDrawPreview(C UID &id  ) {           DrawPreview(id  );}
void CodeEditorInterface::sourceDrawPreview(C Str &name) {CE.init(); DrawPreview(name);} // 'init' in case we're viewing source from data

void CodeEditorInterface::sourceAuto(C Str &data)
{
   Source *source=CE.findSource(Str(AutoSource)); if(!source){source=&CE.sources.New(); source->loc.setFile(AutoSource);}
   source->cpp=false;
   source->Const=true;
   source->active=true;
   source->fromText(data);
}
void CodeEditorInterface::sourceRemove   (C UID &id) {CE.sources.removeData(CE.findSource(id), true);}
void CodeEditorInterface::sourceOverwrite(C UID &id) {if(Source *source=CE.findSource(id))if(source->modified())source->overwrite();}

Bool CodeEditorInterface::sourceDataGet(C UID &id,   Str &data) {if(Source *source=CE.findSource(id)){              data=source->  asText(    );                                                                                   return true;} data.clear(); return false;}
Bool CodeEditorInterface::sourceDataSet(C UID &id, C Str &data) {if(Source *source=CE.findSource(id)){source->setUndo(); source->fromText(data); source->undo_original_state=source->undos.undos(); source->forceCreateNextUndo(); return true;}               return false;}

void CodeEditorInterface::kbSet() {if(CE.cur())CE.cur()->kbSet();}

void CodeEditorInterface::close   () {CE.close();}
void CodeEditorInterface::closeAll()
{
   Memc<SaveChanges::Elm> elms;
   saveChanges(elms);
   saveChanges(elms, CloseAll);
}

void CodeEditorInterface::saveSettings(  TextNode &code) {CE.saveSettings(code);}
void CodeEditorInterface::loadSettings(C TextNode &code) {CE.loadSettings(code);}

void CodeEditorInterface::clearActiveSources()
{
   REPA(CE.sources){Source &src=CE.sources[i]; src.was_active=src.active; src.active=false;}
}
void CodeEditorInterface::activateSource(C UID &id  ) {if(Source *source=CE.getSource(id  ))source->active=true;}
void CodeEditorInterface::activateSource(C Str &name) {if(Source *source=CE.getSource(name))source->active=true;}
void CodeEditorInterface::activateApp   (Bool rebuild_3rd_party_headers)
{
   // always make sure auto generated header is active
   if(Source *source=CE.findSource(Str(AutoSource)))source->active=true;

   // check if not initialized and opened source became used
   if(!CE.symbols_loaded && CE.cur() && CE.cur()->used())
   {
      CE.init(); // this will rebuild
   }else
   {
      // detect if at least 1 source has different active state
      Bool rebuild=false; REPA(CE.sources){Source &src=CE.sources[i]; if(src.was_active!=src.active){rebuild=true; break;}}

      // rebuild
      if(rebuild || rebuild_3rd_party_headers)CE.rebuildSymbols(rebuild_3rd_party_headers);
   }

   // remove useless sources
   CE.removeUselessSources();
}
/******************************************************************************/
static CChar8 *SupportedVS="Following versions are supported:\nVisual Studio 2015\nVisual Studio 2017 (at least version 15.5)";
Bool CheckVisualStudio(C VecI4 &vs_ver, Str *message, Bool check_minor)
{
   if(vs_ver.x<=0){if(message)*message=S+"Visual Studio was not found.\n"+SupportedVS; return false;}
   if(vs_ver.x<14){if(message)*message=S+"Visual Studio version "+vs_ver.asTextDots()+" is no longer supported.\n"+SupportedVS; return false;}
   if(vs_ver.x>15){if(message)*message=S+"Visual Studio version "+vs_ver.asTextDots()+" is not yet supported.\n"+SupportedVS; return false;}
   if(check_minor)
   {
      if(vs_ver.x==15 && vs_ver.y>=0 && vs_ver.y<5){if(message)*message=S+"Visual Studio version "+vs_ver.asTextDots()+" is not supported due to a bug in compiler.\n"+SupportedVS; return false;} // disable VS 2017 ver 15.0 .. 15.4
   }
   if(message)message->clear(); return true;
}
static Int Compare(C VisualStudioInstallation &a, C VisualStudioInstallation &b)
{
   if(Int c=Compare          (a.ver , b.ver ))return c;
   if(Int c=CompareNumber    (a.name, b.name))return c;
   if(Int c=ComparePathNumber(a.path, b.path))return c;
   return 0;
}
Bool GetVisualStudioInstallations(MemPtr<VisualStudioInstallation> installs)
{
   Bool ok=false;
   installs.clear();
#if WINDOWS_OLD
   // !! requires 'CoInitialize' !!
   ISetupConfigurationPtr query;
   if(OK(query.CreateInstance(__uuidof(SetupConfiguration))))
   {
      ISetupConfiguration2Ptr query2(query);
      IEnumSetupInstancesPtr e;
      if(OK(query2->EnumAllInstances(&e)))
      {
         ok=true;
         ISetupHelperPtr helper(query);
         ISetupInstance *pInstances[1]={};
         for(; e->Next(1, pInstances, null)==S_OK; )
         {
            ISetupInstance2Ptr instance(pInstances[0]);
            ISetupPackageReferencePtr product;
            InstanceState state;
            bstr_t instance_id, product_id, install_version, display_name, path;
            if(OK(instance->GetInstanceId(instance_id.GetAddress())))
            if(OK(instance->GetState(&state)))
            if(state==eComplete && (eLocal&state)==eLocal && (eRegistered&state)==eRegistered)
            if(OK(instance->GetInstallationVersion(install_version.GetAddress())))
            if(OK(instance->GetInstallationPath(path.GetAddress())))
            if(OK(instance->GetProduct(&product)))
            if(OK(product->GetId(product_id.GetAddress())))
            if(Starts(WChar(product_id.operator const wchar_t*()), "Microsoft.VisualStudio.Product.", true, true))
            {
            #if 0
               bstr_t desc, engine, install_name, product_path, branch, type, version;
               OK(instance->GetDescription(0, desc.GetAddress()));
               OK(instance->GetEnginePath(engine.GetAddress()));
               OK(instance->GetInstallationName(install_name.GetAddress()));
               OK(instance->GetProductPath(product_path.GetAddress()));
               OK(product->GetBranch(branch.GetAddress()));
               OK(product->GetType(type.GetAddress()));
               OK(product->GetVersion(version.GetAddress()));
            #endif
               Char8 temp[256]; Set(temp, WChar(install_version.operator const wchar_t*())); ReplaceSelf(temp, '.', ',');
               VisualStudioInstallation &install=installs.New();
               install.path=path.operator const wchar_t*();
               install.ver =TextVecI4(temp);
               if(OK(instance->GetDisplayName(0, display_name.GetAddress())))install.name=display_name.operator const wchar_t*();
            }
         }
      }
   }
#if 0
   // older versions were obtained through registry:
   for(Int version=9; version<=..; version++)
   for(Int express=0; express<=1; express++)
   {
      Str path=GetRegStr(RKG_LOCAL_MACHINE, S+"Software/Microsoft/"+(express ? "VCExpress" : "VisualStudio")+'/'+version+".0/Setup/VS/ProductDir");
      if(path.is())
      {
         VisualStudioInstallation &install=installs.New();
         install.path=path;
         install.ver.set(version, 0, 0, 0);
         install.name=S+"Visual Studio "..+(express ? " Express" : null);
      }
   }
#endif
   installs.sort(Compare);
#endif
   return ok;
}
Str MSBuildPath(C Str &vs_path, C VecI4 &vs_ver)
{
   if(vs_path.is())
   {
      Str ms_build_path=vs_path; ms_build_path.tailSlash(true)+="MSBuild\\";
      if(vs_ver.x>0)
      {
         Str ms_build=ms_build_path+vs_ver.x+".0\\Bin\\MSBuild.exe";
       //if(FExistSystem(ms_build))
            return ms_build;
      }
      for(FileFind ff(ms_build_path); ff(); )if(ff.type==FSTD_DIR)
      {
         VecI4 ver=TextVecI4(ff.name); if(ver.x>0)return ms_build_path+ver.x+".0\\Bin\\MSBuild.exe";
      }
   }
   Memc<VisualStudioInstallation> installs; if(GetVisualStudioInstallations(installs))REPA(installs) // go from the end to try the latest version first
   {
      VisualStudioInstallation &install=installs[i]; if(CheckVisualStudio(install.ver))
      {
         return install.path.tailSlash(true)+"MSBuild\\"+install.ver.x+".0\\Bin\\MSBuild.exe";
      }
   }
#if 0 // VS 2015 and older
   {
      Str path=GetRegStr(RKG_LOCAL_MACHINE, S+"Software/Microsoft/MSBuild/ToolsVersions/"+ver+".0/MSBuildToolsPath");
      if( path.is())return path.tailSlash(true)+"MSBuild.exe";
   }
#endif
   return S;
}
Str MSBuildParams(C Str &project, C Str &config, C Str &platform)
{
   // optional params: /target:clean /target:rebuild /FileLogger /nologo /Verbosity:(quiet, minimal, normal, detailed, or diagnostic) /p:AppxPackageSigningEnabled=false
   return S+'"'+NormalizePath(MakeFullPath(project))+"\" /Verbosity:minimal /nologo"+(config.is() ? S+" /p:Configuration=\""+config+'"' : S)+(platform.is() ? S+" /p:Platform=\""+platform+'"' : S);
}
Str VSBuildParams(C Str &project, C Str &config, C Str &platform, C Str &log)
{
   return S+"/Build \""+config+(platform.is() ? S+'|'+platform : S)+"\" \""+NormalizePath(MakeFullPath(project))+'"'+(log.is() ? S+" /Out \""+NormalizePath(MakeFullPath(log))+'"' : S);
}
Str XcodeBuildParams(C Str &project, C Str &config, C Str &platform, C Str &sdk)
{
   return S+"-project \""+UnixPath(project)+"\" -target \""+platform+"\" -configuration \""+config+'"'+(sdk.is() ? S+" -sdk \""+sdk+"\"" : S);
}
Str XcodeBuildCleanParams(C Str &project, C Str &config, C Str &platform, C Str &sdk)
{
   return S+"clean "+XcodeBuildParams(project, config, platform, sdk);
}
Str LinuxBuildParams(C Str &project, C Str &config, Int build_threads)
{
   return S+(config.is() ? S+"CONF=\""+config+"\" " : S)+"-j"+build_threads+" -C \""+UnixPath(project)+'"';
}
/******************************************************************************/
}}
/******************************************************************************/
