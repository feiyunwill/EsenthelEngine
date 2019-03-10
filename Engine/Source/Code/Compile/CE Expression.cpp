/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
// TODO: 
// parse #pragma pack and setup Symbol::class_pack
// call ctors/def_values for global vars and dtors at the end
// a && b && c conditional execute support (add destructors to conditional destructor list)
// a || b || c conditional execute support (add destructors to conditional destructor list)
// a ?  b :  c conditional execute support (add destructors to conditional destructor list)
// reuse stack size, when destroying locals, reuse their stack memory (use 'chunks' struct Chunk{Bool used; Int offset, length;}), only for release mode (so debug will keep variable values for preview)
// test 'goto' for "can't jump to block after ctor/dtor var"
// enum value calculation (into 'Symbol::raw_offset' ?)
// support auto-cast for calculate "CChar *s=Str();"

// TODO: 
// compile errors for parsing data types, read var funcs, etc.
// do not allow custom operators that operate on basic types only
// do not allow class static operators
// do not allow default parameters for operators
// () [] operators can be only class non-static operators
// verify correct amount of parameters for operators (remember that class non-static require 1 less)
// verify uniqueness of symbol names
// pointer to reference is illegal
// array of references are illegal
// if one func param has default value specified, then all following params also must have a default value
// do not allow 2 same bases for a class (allow if different in templates: "class Ext : Str, Str {}" BAD, "class Ext : Memc<Int>, Memc<Str> {}" GOOD)
// do not allow virtual for global or class static funcs

// TODO: 
// do following optimizations (add MODIF_ARITHMETIC to functions? to allow following optimizations also to classes):
// "a+b" -> "a+=b" if 'a' is a temporary
// "a+=1" -> "++a" if 'a' is a basic type (pointer only if size==1)
// "a=b+c" -> don't create temporary "$temp=b+c" and store it "a=$temp" but already do "a=b+c" (only if 'a' and '$temp' are of the same type, and basic type? remember that operator+ uses $temp::ctor(b+c))
// "x*0" -> if 'x' is basic type then set whole expr as known
// skip "1*x", "x*1", "x/1", "0+x", "x+0", "x-0", "x<<0", "x>>0", .. (same for *= /= += -= <<= >>=) if 'x' is basic type (however keep */+- if operation changes type int->flt/dbl, flt->dbl)
// set known for 'const' expressions of basic type
// "a+-b" -> "a-b" if 'a' and 'b' are basic type
// "!!x" -> "x" (only if 'x' is bool or result is used for 'if/for/while', and '!' is not overloaded operator)
// "- - a" (not "--a"), "~~a" -> "a" if 'a' is basic type
// "x++" "x--" if result is unused then replace with "++x" "--x"
// "x*=2" -> "x<<=1" .. (watch out for negative values - only for unsigned ints?)
// "x/=2" -> "x>>=1" .. (watch out for negative values - only for unsigned ints?)
// "1 && x" -> "x", "0 || x" -> "x" (only if 'x' is bool or result is used for 'if/for/while')

// TODO: 
// do not allow compiling when class belongs to one of its base classes
// do not allow compiling when class belongs to one of class types of its member
// error for const vars that don't have default value specified (ignore case when there is a default constructor, like "const Str S;")
// error for references that don't have default value specified
// do not allow modifying of values or calling non-const methods, or passing this to func/operator params of non const pointer/ref, for const methods "void func()const"
// where to set src_template_i to -1 ?
// mutable, explicit support
// when detecting cast currently templates will always return ok/conv/ctor match, but the case of "func(TYPE a, TYPE b)" and "func(Image(), Str())" is ignored, when passing different types to the same template, so when assuming to test cast match, make local copy of the TYPE value/type, so all remaining parameters will have the same value/type
// allocate class static vars in global heap, call ctors/dtors                                                              , (watch out for template classes   <TYPE> class X      {static TYPE obj;})
// allocate func  static vars in global heap, call ctors (on first func/space/block enter), dtors if initialized on app exit, (watch out for template functions <TYPE> void  func() {static TYPE obj;})
// do some global variable initialization order sorting (basing on dependencies)
// initializing variables with default values (globals, statics, members), include arrays "type x[]={..};"
// operator new delete
// friend (classes, funcs, etc)
// dynamic cast, rtti, virtual functions
// pointers to class members and methods
// pointers to non-static functions (as variables)
/******************************************************************************/
// CAST MATCH
/******************************************************************************/
static CAST_MATCH CastMatch(VAR_TYPE src, VAR_TYPE dest) // treat CHAR,ENUM as both signed/unsigned
{
   if(src==dest && src!=VAR_ENUM && src!=VAR_NONE)return CAST_MAX;
   switch(dest)
   {
      case VAR_BOOL: 
         if(src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_LONG || src==VAR_ULONG || src==VAR_CHAR8 || src==VAR_CHAR16 || src==VAR_FLT || src==VAR_DBL || src==VAR_ENUM)return CAST_CONV;
      break;

      case VAR_BYTE:
         if(src==VAR_BOOL  || src==VAR_CHAR8)return CAST_CONV;
         if(src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_LONG || src==VAR_ULONG || src==VAR_CHAR16 || src==VAR_FLT || src==VAR_DBL || src==VAR_ENUM)return CAST_CONV;
      break;
      case VAR_SBYTE:
         if(src==VAR_BOOL || src==VAR_CHAR8)return CAST_CONV;
         if(src==VAR_BYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_LONG || src==VAR_ULONG || src==VAR_CHAR16 || src==VAR_FLT || src==VAR_DBL || src==VAR_ENUM)return CAST_CONV;
      break;

      case VAR_SHORT:
         if(src==VAR_BOOL   || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_CHAR8 || src==VAR_CHAR16)return CAST_CONV;
         if(src==VAR_USHORT || src==VAR_INT  || src==VAR_UINT  || src==VAR_LONG  || src==VAR_ULONG || src==VAR_FLT || src==VAR_DBL || src==VAR_ENUM)return CAST_CONV;
      break;
      case VAR_USHORT:
         if(src==VAR_BOOL  || src==VAR_BYTE  || src==VAR_CHAR8 || src==VAR_CHAR16)return CAST_CONV;
         if(src==VAR_SBYTE || src==VAR_SHORT || src==VAR_INT   || src==VAR_UINT  || src==VAR_LONG || src==VAR_ULONG || src==VAR_FLT || src==VAR_DBL || src==VAR_ENUM)return CAST_CONV;
      break;

      case VAR_INT:
         if(src==VAR_BOOL || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_CHAR8 || src==VAR_CHAR16 || src==VAR_ENUM)return CAST_CONV;
         if(src==VAR_UINT || src==VAR_LONG || src==VAR_ULONG || src==VAR_FLT   || src==VAR_DBL)return CAST_CONV;
      break;

      case VAR_UINT:
         if(src==VAR_BOOL  || src==VAR_BYTE  || src==VAR_USHORT || src==VAR_CHAR8 || src==VAR_CHAR16 || src==VAR_ENUM)return CAST_CONV;
         if(src==VAR_SBYTE || src==VAR_SHORT || src==VAR_INT    || src==VAR_LONG  || src==VAR_ULONG  || src==VAR_FLT || src==VAR_DBL)return CAST_CONV;
      break;

      case VAR_LONG:
         if(src==VAR_BOOL  || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_CHAR8 || src==VAR_CHAR16 || src==VAR_ENUM)return CAST_CONV;
         if(src==VAR_ULONG || src==VAR_FLT  || src==VAR_DBL)return CAST_CONV;
      break;
      case VAR_ULONG:
         if(src==VAR_BOOL  || src==VAR_BYTE  || src==VAR_USHORT || src==VAR_UINT || src==VAR_CHAR8 || src==VAR_CHAR16 || src==VAR_ENUM)return CAST_CONV;
         if(src==VAR_SBYTE || src==VAR_SHORT || src==VAR_INT    || src==VAR_LONG || src==VAR_FLT   || src==VAR_DBL)return CAST_CONV;
      break;

      case VAR_CHAR8:
         if(src==VAR_BYTE  || src==VAR_SBYTE                                                                                        )return CAST_CONV;
         if(src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_LONG || src==VAR_ULONG || src==VAR_CHAR16)return CAST_CONV;
      break;
      case VAR_CHAR16:
         if(src==VAR_CHAR8 || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT)return CAST_CONV;
         if(src==VAR_INT   || src==VAR_UINT || src==VAR_LONG  || src==VAR_ULONG                   )return CAST_CONV;
      break;

      case VAR_FLT:
         if(src==VAR_BOOL || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT)return CAST_CONV;
         if(src==VAR_INT  || src==VAR_UINT || src==VAR_LONG  || src==VAR_ULONG || src==VAR_DBL   )return CAST_CONV;
      break;
      case VAR_DBL:
         if(src==VAR_BOOL || src==VAR_BYTE || src==VAR_SBYTE || src==VAR_SHORT || src==VAR_USHORT || src==VAR_INT || src==VAR_UINT || src==VAR_FLT)return CAST_CONV;
         if(src==VAR_LONG || src==VAR_ULONG)return CAST_CONV;
      break;
   }
   return CAST_NONE;
}
static CAST_MATCH CastMatchConst(CAST_MATCH cast)
{
   switch(cast)
   {
      default       : return cast;
      case CAST_CTOR: return CAST_CTOR_CONST;
      case CAST_CONV: return CAST_CONV_CONST;
      case CAST_MAX : return CAST_MAX_CONST ;
   }
}
/******************************************************************************/
// EXPRESSION
/******************************************************************************/
static void ConvertPtrToUIntPtr(Symbol::Modif &symbol) {symbol.clear(true); symbol=TypeSymbol((PtrSize==4) ? VAR_UINT : VAR_ULONG);} // treat pointers as unsigned's - this is needed for comparing pointers

void Expr::calculateKnown(Expr &op, Expr &a, Compiler &compiler)
{
   if(known() && symbol)
   {
      if(op=='!')setBorrowed(a.asBool(compiler) ? u"false" : u"true");else
      if(op=='~')switch(symbol->var_type)
      {
         case VAR_INT  : setCustom(S + ~a.asInt  (compiler)); break;
         case VAR_UINT : setCustom(S + ~a.asUInt (compiler)); break;
         case VAR_LONG : setCustom(S + ~a.asLong (compiler)); break;
         case VAR_ULONG: setCustom(S + ~a.asULong(compiler)); break;
      }else
      if(op=='-')switch(symbol->var_type)
      {
         case VAR_INT : setCustom(S + -a.asInt (compiler)); break;
         case VAR_LONG: setCustom(S + -a.asLong(compiler)); break;
         case VAR_FLT : setCustom(S + -a.asDbl (compiler)); break; // use Dbl to increase precision
         case VAR_DBL : setCustom(S + -a.asDbl (compiler)); break;
      }else
      compiler.msgs.New().error("Uknown operator", op.origin);
   }
}
void Expr::calculateKnown(Expr &op, Expr &a, Expr &b, Compiler &compiler)
{
   if(known() && symbol && a.symbol && b.symbol)
   {
      if(op=="&&")setBorrowed(((a.known() && !a.asBool(compiler) || b.known() && !b.asBool(compiler)) ? false : (a.asBool(compiler) && b.asBool(compiler))) ? u"true" : u"false");else // if at least one is false then the result is false
      if(op=="||")setBorrowed(((a.known() &&  a.asBool(compiler) || b.known() &&  b.asBool(compiler)) ? true  : (a.asBool(compiler) || b.asBool(compiler))) ? u"true" : u"false");else // if at least one is true  then the result is true
      if(op=="^^")setBorrowed((a.asBool(compiler) != b.asBool(compiler)) ? u"true" : u"false");else
      if(op=="==")
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) == b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) == b.asULong(compiler));else r=(a.asULong(compiler) == b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) == b.asULong(compiler));else r=(a.asLong (compiler) == b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op=="!=")
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) != b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) != b.asULong(compiler));else r=(a.asULong(compiler) != b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) != b.asULong(compiler));else r=(a.asLong (compiler) != b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op=="<=")
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) <= b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) <= b.asULong(compiler));else r=(a.asULong(compiler) <= b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) <= b.asULong(compiler));else r=(a.asLong (compiler) <= b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op==">=")
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) >= b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) >= b.asULong(compiler));else r=(a.asULong(compiler) >= b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) >= b.asULong(compiler));else r=(a.asLong (compiler) >= b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op=='<')
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) < b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) < b.asULong(compiler));else r=(a.asULong(compiler) < b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) < b.asULong(compiler));else r=(a.asLong (compiler) < b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op=='>')
      {
         Bool r=false;
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )r=(a.asDbl  (compiler) > b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)r=(a.asULong(compiler) > b.asULong(compiler));else r=(a.asULong(compiler) > b.asLong(compiler));}else
                                          {if(b.symbol->var_type==VAR_ULONG)r=(a.asLong (compiler) > b.asULong(compiler));else r=(a.asLong (compiler) > b.asLong(compiler));}
         setBorrowed(r ? u"true" : u"false");
      }else
      if(op=='*')
      {
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )setCustom(S+(a.asDbl  (compiler) * b.asDbl  (compiler)));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) * b.asULong(compiler)));else setCustom(S+(a.asULong(compiler) * b.asLong(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) * b.asULong(compiler)));else setCustom(S+(a.asLong (compiler) * b.asLong(compiler)));}
      }else
      if(op=='/')
      {
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )setCustom(S+(a.asDbl(compiler) / b.asDbl(compiler)));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG){ULong v=b.asULong(compiler); if(v)setCustom(S+(a.asULong(compiler) / v));else compiler.msgs.New().error("Division by zero", b.origin);}else {Long v=b.asLong(compiler); if(v)setCustom(S+(a.asULong(compiler) / v));else compiler.msgs.New().error("Division by zero", b.origin);}}else
                                          {if(b.symbol->var_type==VAR_ULONG){ULong v=b.asULong(compiler); if(v)setCustom(S+(a.asLong (compiler) / v));else compiler.msgs.New().error("Division by zero", b.origin);}else {Long v=b.asLong(compiler); if(v)setCustom(S+(a.asLong (compiler) / v));else compiler.msgs.New().error("Division by zero", b.origin);}}
      }else
      if(op=='%')
      {
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG){ULong v=b.asULong(compiler); if(v)setCustom(S+(a.asULong(compiler) % v));else compiler.msgs.New().error("Modulo by zero", b.origin);}else {Long v=b.asLong(compiler); if(v)setCustom(S+(a.asULong(compiler) % v));else compiler.msgs.New().error("Modulo by zero", b.origin);}}else
                                          {if(b.symbol->var_type==VAR_ULONG){ULong v=b.asULong(compiler); if(v)setCustom(S+(a.asLong (compiler) % v));else compiler.msgs.New().error("Modulo by zero", b.origin);}else {Long v=b.asLong(compiler); if(v)setCustom(S+(a.asLong (compiler) % v));else compiler.msgs.New().error("Modulo by zero", b.origin);}}
      }else
      if(op=='+')
      {
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )setCustom(S+(a.asDbl  (compiler) + b.asDbl  (compiler)));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) + b.asULong(compiler)));else setCustom(S+(a.asULong(compiler) + b.asLong(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) + b.asULong(compiler)));else setCustom(S+(a.asLong (compiler) + b.asLong(compiler)));}
      }else
      if(op=='-')
      {
         if(RealType(a.symbol->var_type) || RealType(b.symbol->var_type)   )setCustom(S+(a.asDbl  (compiler) - b.asDbl  (compiler)));else
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) - b.asULong(compiler)));else setCustom(S+(a.asULong(compiler) - b.asLong(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) - b.asULong(compiler)));else setCustom(S+(a.asLong (compiler) - b.asLong(compiler)));}
      }else
      if(op=="<<")
      {
         if(a.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) << b.asInt(compiler)));
         else                             setCustom(S+(a.asLong (compiler) << b.asInt(compiler)));
      }else
      if(op==">>")
      {
         if(a.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) >> b.asInt(compiler)));
         else                             setCustom(S+(a.asLong (compiler) >> b.asInt(compiler)));
      }else
      if(op=='&')
      {
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) & b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asULong(compiler) & b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asULong(compiler) & b.asUInt(compiler)));else setCustom(S+(a.asULong(compiler) & b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_LONG ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) & b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asLong (compiler) & b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asLong (compiler) & b.asUInt(compiler)));else setCustom(S+(a.asLong (compiler) & b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_UINT ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asUInt (compiler) & b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asUInt (compiler) & b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asUInt (compiler) & b.asUInt(compiler)));else setCustom(S+(a.asUInt (compiler) & b.asInt(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asInt  (compiler) & b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asInt  (compiler) & b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asInt  (compiler) & b.asUInt(compiler)));else setCustom(S+(a.asInt  (compiler) & b.asInt(compiler)));}
      }else
      if(op=='^')
      {
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) ^ b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asULong(compiler) ^ b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asULong(compiler) ^ b.asUInt(compiler)));else setCustom(S+(a.asULong(compiler) ^ b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_LONG ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) ^ b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asLong (compiler) ^ b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asLong (compiler) ^ b.asUInt(compiler)));else setCustom(S+(a.asLong (compiler) ^ b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_UINT ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asUInt (compiler) ^ b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asUInt (compiler) ^ b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asUInt (compiler) ^ b.asUInt(compiler)));else setCustom(S+(a.asUInt (compiler) ^ b.asInt(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asInt  (compiler) ^ b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asInt  (compiler) ^ b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asInt  (compiler) ^ b.asUInt(compiler)));else setCustom(S+(a.asInt  (compiler) ^ b.asInt(compiler)));}
      }else
      if(op=='|')
      {
         if(a.symbol->var_type==VAR_ULONG){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asULong(compiler) | b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asULong(compiler) | b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asULong(compiler) | b.asUInt(compiler)));else setCustom(S+(a.asULong(compiler) | b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_LONG ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asLong (compiler) | b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asLong (compiler) | b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asLong (compiler) | b.asUInt(compiler)));else setCustom(S+(a.asLong (compiler) | b.asInt(compiler)));}else
         if(a.symbol->var_type==VAR_UINT ){if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asUInt (compiler) | b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asUInt (compiler) | b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asUInt (compiler) | b.asUInt(compiler)));else setCustom(S+(a.asUInt (compiler) | b.asInt(compiler)));}else
                                          {if(b.symbol->var_type==VAR_ULONG)setCustom(S+(a.asInt  (compiler) | b.asULong(compiler)));else if(b.symbol->var_type==VAR_LONG)setCustom(S+(a.asInt  (compiler) | b.asLong(compiler)));else if(b.symbol->var_type==VAR_UINT)setCustom(S+(a.asInt  (compiler) | b.asUInt(compiler)));else setCustom(S+(a.asInt  (compiler) | b.asInt(compiler)));}
      }else
      if(op=='=')
      {
         if(a.symbol.isPtr()){if(b=="null")setBorrowed(u"null");else setCustom(S+b.asULong(compiler));}else
         if(RealType(a.symbol->var_type) )setCustom(S+b.asDbl  (compiler));else
         if(a.symbol->var_type==VAR_ULONG)setCustom(S+b.asULong(compiler));else
                                          setCustom(S+b.asLong (compiler));
      }else
      /*if(op=="*=" || op=="/=" || op=="%=" || op=="+=" || op=="-=" || op=="<<=" || op==">>=" || op=="&=" || op=="|=" || op=="^=")
      {
      }else*/
      compiler.msgs.New().error("Uknown operator", op.origin);
   }
}
CAST_MATCH Expr::calculate(Compiler &compiler)
{
   if(func.elms()==1)
   {
      Expr &func=T.func[0];
      origin  =func.origin;
      final  |=func.final;
      instance=true; // functions/operators/constructors always return an instance

      // check for built-in operators functions
      if(func=="sizeof")
      {
         setBasic(VAR_INT);
         if(compiler.strict && func.params.elms()!=1){compiler.msgs.New().error(S+"Invalid number of parameters to '"+func+'\'', origin); return CAST_NONE;}
         if(func.params.elms()>=1){Expr &a=func.params[0]; mem.setKnown(); setCustom(S+a.symbol.rawSize(false));}
         return CAST_MAX;
      }else
      if(func=="typeid")
      {
         setPtr(); return CAST_MAX;
      }else
      if(func=="dynamic_cast" || func=="static_cast" || func=="const_cast" || func=="reinterpret_cast")
      {
         if(compiler.strict)
         {
            if(func.symbol.templates.elms()!=1){compiler.msgs.New().error(S+"Invalid number of template parameters to '"+func+'\'', origin); return CAST_NONE;}
            if(func.params.          elms()!=1){compiler.msgs.New().error(S+"Invalid number of parameters to '"         +func+'\'', origin); return CAST_NONE;}
            Expr          &src =func.params[0];
            Symbol::Modif &dest=func.symbol.templates[0]; if(!dest){compiler.msgs.New().error(S+"Unknown template parameter", origin); return CAST_NONE;}
            if(func=="dynamic_cast")
            {
               if(dest.ptr_level==1 && !(dest.modifiers&Symbol::MODIF_REF) && !dest.array_dims.elms()  // C++ allows 'dynamic_cast' only when target is TYPE* or TYPE&
               || dest.ptr_level==0 &&  (dest.modifiers&Symbol::MODIF_REF) && !dest.array_dims.elms())
               {
                  if(src.symbol.ptr_level==dest.ptr_level && !src.symbol.array_dims.elms())
                  {
                     temporary=(dest.ptr_level ? true : src.temporary); symbol=dest; if(dest.ptr_level)symbol.setConst(); return CAST_MAX; // yes it is a temporary - C++ compiling failed on "struct A{virtual ~A(){}}; struct B:A{virtual ~B(){}}; A *a=null; &CAST(B,a);"
                  }
                  compiler.msgs.New().error(S+"Can't perform 'dynamic_cast' from: "+src.symbol.modifName(true, true, false)+", to: "+dest.modifName(true, true, false), origin); return CAST_NONE;
               }
               compiler.msgs.New().error(S+"Target for 'dynamic_cast' must be a pointer or a reference", origin); return CAST_NONE;
            }else
            {
               temporary=true; symbol=dest; symbol.setConst(); return CAST_MAX;
            }
         }else
         {
            if(func.symbol.templates.elms()>=1) // set 'dest'
            {
               Symbol::Modif &dest=func.symbol.templates[0];
               if(dest){symbol=dest; return CAST_MAX;}
            }
            if(func.params.elms()>=1) // set 'src'
            {
               Expr &src=func.params[0];
               symbol=src.symbol; return CAST_MAX;
            }
            return CAST_NONE;
         }
      }else
      if(func._operator && func!="()") // there is no built-in "operator()"
      {
         if(func.params.elms()==1) // unary [] *x, &x, !x, ~x, +x, -x, ++x, --x, x++, x--
         {
            if(func=="[]") // parent[a], C++ supports only non-static [] operators
            {
               Expr &a=func.params[0];
               if(func.parent.elms()!=1){compiler.msgs.New().error("Invalid object for the [] operator", origin); return CAST_NONE;}
               if(compiler.strict)if(!a.symbol.isObj() || !a.symbol || !OffsetType(a.symbol->var_type)){compiler.msgs.New().error("Invalid index for the [] operator", a.origin); return CAST_NONE;}
               Expr &parent=func.parent[0];

               if(!parent.symbol.isObj())
               {
                  if(!parent.indirection(compiler))return CAST_NONE; // do "TYPE a[]; *a;"
                  temporary=parent.temporary; symbol=parent.symbol;
                  if(a.known())
                  {
                     memFrom(parent.mem, compiler);
                     memOffset(a.asLong(compiler)*parent.symbol.rawSize(false));
                     func.func_call=GetIgnoreCall();
                  }else
                  {
                     symbol.modifiers|=Symbol::MODIF_REF;
                     memNew(false, compiler);
                     func.func_call_mask=FC_PARENT|FC_RESULT;
                     func.func_call     =GetOffsetCall(a.symbol);
                     if(!func.func_call){if(!compiler.quiet)compiler.msgs.New().error("No function/operator found able to process command", origin); return CAST_NONE;} // if didn't found the function then it means this call is impossible
                     // create extra param specifying size of the object (do this after all codes accessing 'a', because this will change memory address of 'a')
                     Expr &raw_size=func.params.New(); raw_size.setBasic(VAR_INT); raw_size.mem.setKnown(); raw_size.setCustom(S+parent.symbol.rawSize(false));
                  }
                  return CAST_MAX;
               }else
               if(parent.basicType()){compiler.msgs.New().error("Invalid object for the [] operator", origin); return CAST_NONE;}
            }else
            if(func.params[0].basicType())
            {
            indirection_address_of:
               Expr &a=func.params[0];
               if(func=='*') // verified Obj, Ptr, Array
               {
                  if(a.symbol.isObj() && compiler.strict){compiler.msgs.New().error("Invalid * operator", origin); return CAST_NONE;} // in C++ "int a[2]; *a;" is correct!
                  temporary=false; symbol=a.symbol; symbol.setConst(false); // remove const from pointer, 'temporary' IS false because this "int a[1]; &(*a);" compiles on C++
                  if(a.symbol.isArray())
                  {
                     if(symbol.array_dims.elms())symbol.array_dims.removeLast();else if(symbol.ptr_level>0)symbol.ptr_level--;
                     memFrom(a.mem, compiler);
                     func.func_call=GetIgnoreCall();
                  }else // use REF for arrays too ? (consider REF array and non-REF array)
                  {
                     if(a.func.elms()) // "*(&x)" -> "x"
                     {
                        Expr &a_func=a.func[0];
                        if(   a_func.func_call==GetAddressOfCall() && a_func.params.elms()==1)
                        {
                           Expr temp; Swap(temp, a_func.params[0]); Swap(temp, T); return CAST_MAX;
                        }
                     }
                     FlagEnable(symbol.modifiers, Symbol::MODIF_REF); // treat this expression as reference (because this was a pointer, and we still operate on Ptr address)
                     if(symbol.array_dims.elms())symbol.array_dims.removeLast();else if(symbol.ptr_level>0)symbol.ptr_level--;
                     if(a.symbol.modifiers&Symbol::MODIF_REF) // if param was a reference too, then we need to create a temporary local variable
                     {
                        memNew(false, compiler);
                        func.func_call_mask=FC_RESULT;
                        func.func_call     =GetIndirectionCall();
                     }else // we have direct access to the param Ptr, so reuse it
                     {
                        memFrom(a.mem, compiler);
                        if(parent.elms()!=0){compiler.msgs.New().error("Parent should be empty in indirection", origin); return CAST_NONE;}
                        Swap(parent.New(), a); // move the original pointer to parent so func is not needed
                        T.func.del(); // delete the function
                     }
                  }
                  return CAST_MAX;
               }else
               if(func=='&') // verified Obj, Ptr, Array
               {
                  if(a.temporary && a.basicType() && compiler.strict){compiler.msgs.New().error("Can't take address of a temporary", origin); return CAST_NONE;}
                  if(a.func.elms()) // "&(*x)" -> "x"
                  {
                     Expr &a_func=a.func[0];
                     if(   a_func.func_call==GetIndirectionCall() && a_func.params.elms()==1)
                     {
                        Expr temp; Swap(temp, a_func.params[0]); Swap(temp, T); return CAST_MAX;
                     }
                  }
                  temporary=true; symbol=a.symbol; // make the address a 'temporary'
                  FlagDisable(symbol.modifiers, Symbol::MODIF_REF); // clear any reference modifier
                  if(symbol.array_dims.elms())symbol.array_dims.add(Symbol::DIM_PTR);else symbol.ptr_level++;
                  symbol.setConst(); // set const to the pointer
                  memNew(false, compiler);
                  func.func_call_mask=FC_RESULT;
                  func.func_call     =GetAddressOfCall();
                  return CAST_MAX;
               }else
               if(func=='!') // verified Obj, Ptr, Array
               {
                  setBasic(VAR_BOOL); memNew(a.known(), compiler); calculateKnown(func, a, compiler);
                  Symbol::Modif src=a.symbol;
                  if(src.isPtr())ConvertPtrToUIntPtr(src); // treat ptr as int
                  if(src.isObj() && src)src=TypeSymbol(SameSizeType(src->var_type));
                  func.func_call_mask=FC_RESULT;
                  func.func_call     =FindFuncCall(NativeOperatorName(func, symbol, src));
                  return CAST_MAX;
               }else
               if(func=='~') // verified Obj, Ptr, Array
               {
                  setBasic(ComplementResult(a.symbol ? a.symbol->varType() : VAR_NONE)); memNew(a.known(), compiler); calculateKnown(func, a, compiler);
                  Symbol::Modif src=a.symbol; if(src.isObj() && src)src=TypeSymbol(SameSizeSignType(src->var_type));
                  func.func_call_mask=FC_RESULT;
                  func.func_call     =FindFuncCall(NativeOperatorName(func, symbol, src));
                  return CAST_MAX;
               }else
               if(func=='+') // verified Obj, Ptr, Array
               {
                  Expr temp; // use temporary because 'a' belongs to 'T'
                  Swap(temp, a);
                  Swap(temp, T);
                  return CAST_MAX;
               }else
               if(func=='-') // verified Obj, Ptr, Array
               {
                  setBasic(NegativeResult(a.symbol ? a.symbol->varType() : VAR_NONE)); memNew(a.known(), compiler); calculateKnown(func, a, compiler);
                  Symbol::Modif src=a.symbol; if(src.isObj() && src)src=TypeSymbol(SameSizeSignType(src->var_type));
                  func.func_call_mask=FC_RESULT;
                  func.func_call     =FindFuncCall(NativeOperatorName(func, symbol, src));
                  return CAST_MAX;
               }else
               if(func=="++" || func=="--") // verified Obj, Ptr, Array
               {
                  if(a.symbol.isConst() && compiler.strict){compiler.msgs.New().error("Can't modify a const value", origin); return CAST_NONE;}
                  if(func.l_to_r) // x++, x--
                  {
                     // TODO: add support
                     temporary=true; symbol=a.symbol; symbol.setConst();
                     if(compiler.strict){compiler.msgs.New().error("This operator is not yet supported, try using ++x or --x", origin); return CAST_NONE;}
                  }else // ++x, --x
                  {
                     temporary=a.temporary; symbol=a.symbol; memFrom(a.mem, compiler); // use the same memory layout as 'a' because these operators do not return any other address
                     Symbol::Modif src=a.symbol;
                     func.func_call_mask=0;
                     if(src.isPtr())
                     {
                        if(src.array_dims.elms())src.array_dims.removeLast();else src.ptr_level--; // remove the ptr
                        Int size=src.rawSize(false); // get the size of under the pointer "byte *x" would be 1, while "byte **x" would be PtrSize
                        if( size<=0 && compiler.strict){compiler.msgs.New().error("Unknown pointer size", origin); return CAST_NONE;}
                        if( size> 0)switch(size)
                        {
                           case 1: ConvertPtrToUIntPtr(src); goto inc_dec_1;

                           case 2:
                           case 4:
                           case 8: func.func_call=FindFuncCall(S+((func=="++") ? "inc ptr " : "dec ptr ")+size); break;

                           default:
                           {
                              Expr &by=func.params.New(); by.origin=origin; by.setBasic(VAR_INT); by.mem.setKnown(); by.setCustom(S+size);
                              func.func_call=FindFuncCall((func=="++") ? "inc ptr n" : "dec ptr n");
                           }break;
                        }
                     }else
                     {
                     inc_dec_1:
                        if(src.isObj() && src)src=TypeSymbol(SameSizeType(src->var_type));
                        Symbol::Modif result; result=TypeSymbol(VAR_VOID);
                        func.func_call=FindFuncCall(NativeOperatorName(func, result, src));
                     }
                  }
                  return CAST_MAX;
               }else
               {
                  compiler.msgs.New().error("Invalid unary operator", origin); return CAST_NONE;
               }
            }
         }else
         if(func.params.elms()==2) // binary * / % + - << >> < > <= >= == != === !!= & ^ | && ^^ || = *= /= %= += -= <<= >>= &= |= ^= ,
         {
            Expr &a=func.params[0], &b=func.params[1];

            // cast to conditionals
            if(func=="&&" || func=="||" || func=="^^")
            {
               if(!a.castToConditional() && compiler.strict){compiler.msgs.New().error("Can't cast to bool", a.origin); return CAST_NONE;}
               if(!b.castToConditional() && compiler.strict){compiler.msgs.New().error("Can't cast to bool", b.origin); return CAST_NONE;}
            }

            if(func==',') // a, b
            {
               temporary=b.temporary; symbol=b.symbol; memFrom(b.mem, compiler); set(b); return CAST_MAX;
            }else
            if(a.basicType() && b.basicType())
            {
               if(func=="==" || func=="===" || func=="!=" || func=="!!=" || func=='<' || func=='>' || func=="<=" || func==">=" || func=="&&" || func=="^^" || func=="||") // verified Obj, Ptr, Array
               {
                  setBasic(VAR_BOOL);
                  if(func=="&&")memNew(a.known() && !a.asBool(compiler) || b.known() && !b.asBool(compiler), compiler);else // if at least one is false
                  if(func=="||")memNew(a.known() &&  a.asBool(compiler) || b.known() &&  b.asBool(compiler), compiler);else // if at least one is true
                                memNew(a.known()                        && b.known()                       , compiler);
                  calculateKnown(func, a, b, compiler);
                  Symbol::Modif pa=a.symbol, pb=b.symbol;

                  // adjust pointers
                  if(func=="&&" || func=="^^" || func=="||")
                  {
                     if(pa.isPtr())ConvertPtrToUIntPtr(pa);
                     if(pb.isPtr())ConvertPtrToUIntPtr(pb);
                  }else // "==", "===", "!=", "!!=", "<", ">", "<=", ">="
                  if(!pa.isObj() && !pb.isObj()) // ptr or array
                  {
                     Bool a_is_void_ptr=a.symbol.isVoidPtr(),
                          b_is_void_ptr=b.symbol.isVoidPtr();
                     if(pa.isPtr() && pb.isPtr() && (pa.same(pb, false, false) || pa=="null" || pb=="null" || a_is_void_ptr || b_is_void_ptr))
                     {
                        ConvertPtrToUIntPtr(pa);
                        ConvertPtrToUIntPtr(pb);
                     }else
                     {
                        // TODO: test for "class Base {} class Ext : Image, Base, Sound {}  {Base *b; Ext *e; b==e;}" - WORKS OK in C++, Watch out for different offsets !! (don't forget about possible vfunc_table)
                        // if(pa && pa->hasBase(pb..) || pb && pb->hasBase(pa..)) and compare if base's are the 'same' use 'same' func 'Base<int>' vs 'Ext' which has 'Base<float>'
                        /*if(pa pb .isPtr())
                        {
                        }else // array
                        {
                        }*/
                        if(compiler.strict){compiler.msgs.New().error("Can't compare pointers of different types", origin); return CAST_NONE;}
                     }
                  }

                  if(func=="&&" || func=="^^" || func=="||")
                  {
                     if(pa.isObj() && pa)pa=TypeSymbol(SameSizeType(pa->var_type));
                     if(pb.isObj() && pb)pb=TypeSymbol(SameSizeType(pb->var_type));
                  }else
                  {
                     if(pa.isObj() && pa)pa=TypeSymbol(SameSizeSignType(pa->var_type));
                     if(pb.isObj() && pb)pb=TypeSymbol(SameSizeSignType(pb->var_type));
                  }

                  func.func_call_mask=FC_RESULT;
                  func.func_call     =FindFuncCall(NativeOperatorName(func, symbol, pa, pb));
                  return CAST_MAX;
               }else
               if(func=='=' || func=="*=" || func=="/=" || func=="%=" || func=="+=" || func=="-=" || func=="<<=" || func==">>=" || func=="&=" || func=="|=" || func=="^=") // verified Obj, Ptr, Array
               {
                  if(a.symbol.isConst() && compiler.strict){if(!compiler.quiet)compiler.msgs.New().error("Can't modify a const value", origin); return CAST_NONE;}
                  Bool array_to_ptr=false;
                  if(a.symbol.isPtr() && b.symbol.isArray()) // convert "TYPE b[]" -> "TYPE *b"
                  {
                     if(!b.addressOf(compiler))return CAST_NONE; if(b.symbol.array_dims.elms()<2)return CAST_NONE;
                     // 'b' is now pointer to array "TYPE (*b)[]", which needs to be converted to "TYPE *b" (ptr_level, array_dims[0..last-2]=*, array_dims[last-1]!=PTR, array_dims.last=PTR) -> (ptr_level, array_dims[0..last-2]=*, array_dims.last=PTR)
                     UInt start=(b.symbol.const_level & ((1<<(b.symbol.ptr_level+b.symbol.array_dims.elms()-1))-1)); // get  const level of (ptr_level, array_dims[0..last-2])
                                                                                                                     // skip const level of (           array_dims[   last-1])
                     Bool end  =b.symbol.isConst();                                                                  // get  const level of (           array_dims[   last  ])
                     if(b.symbol.array_dims.elms()!=2)b.symbol.array_dims.remove(b.symbol.array_dims.elms()-2, true); // remove element before last
                     {  // in case there are 2 array_dims then they are {"!=PTR, PTR"}, removing first would result in {"PTR"}, which should be represented as ptr_level and not array_dims
                        b.symbol.ptr_level++;
                        b.symbol.array_dims.clear();
                     }
                     b.symbol.const_level=start; b.symbol.setConst(end); // set new const levels
                     array_to_ptr=true;
                  }
                  Bool       same=a.symbol.same(b.symbol, false, false); // this is needed to test for the same enum's and pointers (enum's are not included in the 'CastMatch' function)
                  CAST_MATCH cast=(same ? CAST_MAX : CastMatch(a.symbol ? a.symbol->varType() : VAR_NONE, b.symbol ? b.symbol->varType() : VAR_NONE));
                  temporary=a.temporary; symbol=a.symbol;
                  if(a.known() && b.known() && a.basicType() && func=='=')mem.setKnown();else memFrom(a.mem, compiler); // allow setting known only if both 'a' and 'b' are known (this is used in casting 'CastTo' when converting constants, however do not set known if 'a' is not a known constant, because it would cause skipping of setting the value in the compiler) otherwise use the same memory layout as 'a' because these operators do not return any other address
                  calculateKnown(func, a, b, compiler);
                  Symbol::Modif result, pa=a.symbol, pb=b.symbol; result=TypeSymbol(VAR_VOID);
                  if(pa.isPtr())
                  {
                     if(func=="+=" || func=="-=")
                     {
                        if(compiler.strict)if(!pb.isObj() || !pb || !OffsetType(pb->var_type)){compiler.msgs.New().error(S+"Invalid offset for the "+func+" operator", b.origin); return CAST_NONE;}
                        if(pa.array_dims.elms())pa.array_dims.removeLast();else pa.ptr_level--; // remove the ptr
                        Int size=pa.rawSize(false); // get the size of under the pointer "byte *x" would be 1, while "byte **x" would be PtrSize
                        if( size<=0 && compiler.strict){compiler.msgs.New().error("Unknown pointer size", origin); return CAST_NONE;}
                        if( size==1) // "byte *p;" (can use integer += -=)
                        {
                           ConvertPtrToUIntPtr(pa);
                        }else
                        if(b.known()) // "TYPE *p; p+=const;" (can use integer += -= by "const*SIZE(TYPE)")
                        {
                           ConvertPtrToUIntPtr(pa);
                           Long total=b.asLong(compiler)*size;
                           b.setBasic((Int(total)!=total) ? VAR_LONG : VAR_INT); b.setCustom(S+total);
                        }else
                        {
                           func.func_call_mask=0;
                           func.func_call     =GetAddPtrCall(pb);
                           if(!func.func_call){if(!compiler.quiet)compiler.msgs.New().error("No function/operator found able to process command", origin); return CAST_NONE;} // if didn't found the function then it means this call is impossible
                           // create extra param specifying size of the object (do this after all codes accessing func.params, because this will change memory address of func.params)
                           Expr &raw_size=func.params.New(); raw_size.setBasic(VAR_INT); raw_size.mem.setKnown(); raw_size.setCustom(S+((func=="+=") ? size : -size));
                           return CAST_MAX;
                        }
                     }else
                     if(func=='=' && pb.isPtr())
                     {
                        Bool a_is_void_ptr=pa.isVoidPtr();
                        if(  a_is_void_ptr || same || pb=="null")
                        {
                           if(pb!="null" && compiler.strict)
                           {
                              pb.setConst(false); // disable last const, since "void *a; void *const b; a=b;" is OK
                              if(a_is_void_ptr)pb.const_level=(pb.const_level!=0); // 'a' can be Ptr or CPtr, in that case set const_level to 'b' only to 1st bit
                              if(LostConst(pb.const_level, pa.const_level) && compiler.strict){if(compiler.quiet)return CAST_NONE; compiler.msgs.New().error("Lost const", origin);}
                           }
                           ConvertPtrToUIntPtr(pa);
                           ConvertPtrToUIntPtr(pb);
                           if(!same)cast=CAST_CONV; // if the pointers are not the same, but we still can convert them, then set conversion priority (otherwise this could be CAST_NONE through 'CastMatch')
                           if(array_to_ptr && compiler.unmappedLocal(b.mem)) // if 'b' pointer was created using 'addressOf'
                           {
                              b.mem=a.mem; // set target of 'addressOf' to 'a' pointer
                              func.func_call=GetIgnoreCall();
                              return cast;
                           }
                        }else
                        {
                           // TODO: do the same base checks as above in "==" operators
                           // include LostConst(pb.const_level, pa.const_level))
                           // include "cast=.."
                           if(compiler.strict){if(!compiler.quiet)compiler.msgs.New().error("Can't set pointer to a different type", origin); return CAST_NONE;}
                        }
                     }
                  }
                  if(pa.isObj() && pa)pa=TypeSymbol(                                              SameSizeSignBoolType(pa->var_type));
                  if(pb.isObj() && pb)pb=TypeSymbol((pa && pa->var_type==VAR_BOOL && func=='=') ? SameSizeSignBoolType(pb->var_type) : SameSizeSignType(pb->var_type));
                  func.func_call_mask=0;
                  func.func_call     =FindFuncCall(NativeOperatorName(func, result, pa, pb));
                  if(!func.func_call){if(!compiler.quiet)compiler.msgs.New().error("No function/operator found able to process command", origin); return CAST_NONE;} // if didn't found the function then it means this call is impossible
                  return cast;
               }else
               if(func=='*' || func=='/' || func=='%' || func=='+' || func=='-' || func=="<<" || func==">>" || func=='&' || func=='^' || func=='|') // verified Obj, Ptr, Array
               {
                  if(a.symbol.isPtr())
                  {
                     if(func=='-' && b.symbol.isPtr()) // ptr-ptr (pointer difference)
                     {
                        if(!a.symbol.same(b.symbol, false, false) && compiler.strict){compiler.msgs.New().error("Can't perform pointer difference on different type of pointers", origin); return CAST_NONE;}
                        Symbol::Modif pa=a.symbol; if(pa.array_dims.elms())pa.array_dims.removeLast();else if(pa.ptr_level>0)pa.ptr_level--;
                        Int size=pa.rawSize(false); if(size<=0 && compiler.strict){compiler.msgs.New().error("Can't perform pointer difference on zero sized pointers", origin); return CAST_NONE;}
                        setBasic((PtrSize==4) ? VAR_INT : VAR_LONG); // result must be UIntPtr
                        memNew(false, compiler);
                        func.func_call_mask=FC_RESULT;
                        func.func_call     =GetPtrDiffCall();
                        // create extra param specifying size of the object (do this after all codes accessing func.params, because this will change memory address of func.params)
                        Expr &raw_size=func.params.New(); raw_size.setBasic(VAR_INT); raw_size.mem.setKnown(); raw_size.setCustom(S+size);
                        return CAST_MAX;
                     }else
                     if(func=='+' || func=='-') // ptr+int, ptr-int (pointer offset)
                     {
                        if(compiler.strict)if(!b.symbol.isObj() || !b.symbol || !OffsetType(b.symbol->var_type)){compiler.msgs.New().error(S+"Invalid offset for the "+func+" operator", b.origin); return CAST_NONE;}
                        Symbol::Modif pa=a.symbol; if(pa.array_dims.elms())pa.array_dims.removeLast();else if(pa.ptr_level>0)pa.ptr_level--;
                        Int size=pa.rawSize(false); if(size<=0 && compiler.strict){compiler.msgs.New().error("Unknown pointer size", origin); return CAST_NONE;}
                        symbol=a.symbol; symbol.setConst(true); temporary=true; memNew(false, compiler);
                        func.func_call_mask=FC_RESULT;
                        func.func_call     =GetPtrAddCall(b.symbol);
                        if(!func.func_call){if(!compiler.quiet)compiler.msgs.New().error("No function/operator found able to process command", origin); return CAST_NONE;} // if didn't found the function then it means this call is impossible
                        // create extra param specifying size of the object (do this after all codes accessing func.params, because this will change memory address of func.params)
                        Expr &raw_size=func.params.New(); raw_size.setBasic(VAR_INT); raw_size.mem.setKnown(); raw_size.setCustom(S+((func=='+') ? size : -size));
                        return CAST_MAX;
                     }
                  }else
                  if(a.symbol && a.symbol.isObj() && b.symbol && b.symbol.isObj())
                  {
                     Symbol::Modif pa, pb;
                     pa=TypeSymbol(SameSizeSignType(a.symbol->var_type));
                     pb=TypeSymbol(SameSizeSignType(b.symbol->var_type));
                     VAR_TYPE type=VAR_NONE;
                     if(func=='*'                          )type=       MulResult(pa->var_type, pb->var_type);else
                     if(func=='/'                          )type=       DivResult(pa->var_type, pb->var_type);else
                     if(func=='%'                          )type=       ModResult(pa->var_type, pb->var_type);else
                     if(func=='+'                          )type=       AddResult(pa->var_type, pb->var_type);else
                     if(func=='-'                          )type=       SubResult(pa->var_type, pb->var_type);else
                     if(func=="<<"                         )type= ShiftLeftResult(pa->var_type, pb->var_type);else
                     if(func==">>"                         )type=ShiftRightResult(pa->var_type, pb->var_type);else
                     if(func=='&' || func=='^' || func=='|')type=       BitResult(pa->var_type, pb->var_type);
                     setBasic(type); memNew(a.known() && b.known(), compiler); calculateKnown(func, a, b, compiler);
                     func.func_call_mask=FC_RESULT;
                     func.func_call     =FindFuncCall(NativeOperatorName(func, symbol, pa, pb));
                     return CAST_MAX;
                  }
               }else
               {
                  compiler.msgs.New().error("Invalid binary operator", origin); return CAST_NONE;
               }
            }
         }else
         if(func.params.elms()==3) // ternary
         {
            Expr &a=func.params[0], &b=func.params[1], &c=func.params[2];
            if(func=='?') // a ? b : c
            {
               if(!a.known()){b.setConditional(); c.setConditional();}
               // TODO:
               // a must be of bool type (allow casting)
               // b must be of similar type to c (allow converting/casting)
               //temporary=b.temporary;
               symbol=b.symbol;
               //known=(a.known ? (a.asBool() ? b.known : c.known) : false);
               return CAST_MAX;
            }else
            {
               compiler.msgs.New().error("Invalid ternary operator", origin); return CAST_NONE;
            }
         }else
         {
            compiler.msgs.New().error("Invalid parameter number for operator", origin); return CAST_NONE;
         }
      }

      // TODO: check for function template specification "func<Image>(..)" and include it for AddMatches
      // TODO: check if calling non-const function for const parent/object
      // TODO: check for constructors / functions / custom operators
      Bool            ignore_first   =false, // used for detecting templates from function parameters "func(TYPE x)" func(Image()) -> TYPE=Image, ignore first param when having "a+b" operator when defined as class operator "class A{operator+(A b);}, in such case list of params is "a,b" but func/operator has "b" and we should ignore the 'a' param
                      allow_mem_reuse=false; // if allow func.mem reuse for the result if available, this is used if we're calling class constructor and we already know the destination memory for the result
      Memc<FuncMatch> matches;
      if(func.symbol && (func.symbol->modifiers&Symbol::MODIF_DATA_TYPE)) // constructor/cast
      {
         if(!func.instance)
         {
            if(func.symbol.modifiers&Symbol::MODIF_REF) // reinterpret cast "(TYPE&)x"
            {
               symbol=func.symbol;
               if(func.params.elms()!=1 && compiler.strict){compiler.msgs.New().error(S+"Invalid number of parameters for cast to "+func, origin); return CAST_NONE;}
               if(func.params.elms()>=1)
               {
                  Expr &a=func.params[0];
                  if(a.known() && compiler.strict){compiler.msgs.New().error(S+"Can't perform cast on a known constant", origin); return CAST_NONE;}
                  temporary=a.temporary; memFrom(a.mem, compiler); FlagSet(symbol.modifiers, Symbol::MODIF_REF, FlagTest(a.symbol.modifiers, Symbol::MODIF_REF)); // discard reference from the func, and keep it from the original 'a' parameter
               }
               return CAST_MAX;
            }else
            if(func.basicType()) // cast
            {
               if(func.params.elms()!=1 && compiler.strict){compiler.msgs.New().error(S+"Invalid number of parameters for cast to "+func, origin); return CAST_NONE;}
               // TODO: add support
               if(func.params.elms()>=1){Expr &a=func.params[0];}
               temporary=true; symbol=func.symbol; symbol.setConst(); return CAST_MAX;
            }else
            if(func.symbol->type==Symbol::CLASS) // class constructor
            {
               allow_mem_reuse=true;
               if(Symbol *ctor=FindChild(*func.symbol, func.symbol(), null, false)()) // get ctor func list
                  if(ctor->type==Symbol::FUNC_LIST)
                     AddMatches(matches, ctor->funcs, func.l_to_r, func.params.data(), func.params.elms(), func.symbol.templates, compiler); // add matches for all ctors
            }
         }
      }else
      if(func.symbol && func.symbol->type==Symbol::FUNC_LIST) // function (global func, class static method, class non-static method)
      {
         AddMatches(matches, func.symbol->funcs, func.l_to_r, func.params.data(), func.params.elms(), func.symbol.templates, compiler);
      }else
      if(func.symbol && func.symbol->type==Symbol::FUNC) // if a ready function was already provided (global func, class static method, class non-static method)
      {
         AddMatch(matches, func.symbol(), func.l_to_r, func.params.data(), func.params.elms(), func.symbol.templates, compiler);
      }else
      if(func._operator) // custom operator ("parent[param]", "parent(param)" - both can be class non-static method only) (global "operator(obj, param)", or class non-static "Class::operator(param)")
      {
         if(func=="()" || func=="[]") // "parent[param]", "parent(param)"
         {
            if(func.parent.elms()!=1){compiler.msgs.New().error(S+"Invalid object for the "+func+" operator", origin); return CAST_NONE;}
            Expr &parent=func.parent[0];
            if(!parent.instance || !parent.symbol || parent.symbol->type!=Symbol::CLASS){compiler.msgs.New().error(S+"Invalid object for the "+func+" operator", origin); return CAST_NONE;}

            Str func_name=SymbolToPath(S+"operator"+func);
            if(Symbol *op=FindChild(func_name, parent.symbol())())if(op->type==Symbol::FUNC_LIST)AddMatches(matches, op->funcs, func.l_to_r, func.params.data(), func.params.elms(), func.symbol.templates, compiler); // add class non-static operators "parent[params]", "parent(params)"
         }else
         if(func.params.elms()>=1) // global "operator(obj, param)" or class non-static "Class::operator(param)"
         {
            Str func_name=SymbolToPath(S+"operator"+func);
            ignore_first=true; // allow for potential ignoring of the first parameter
                                                                                               if(Symbol *global_op=FindChild(func_name, null                   )())if(global_op->type==Symbol::FUNC_LIST)AddMatches(matches, global_op->funcs, func.l_to_r, func.params.data()  , func.params.elms()  , func.symbol.templates, compiler      ); // add global           operators "       op(obj, params)"
            if(func.params.elms()>=1 && func.params[0].symbol && func.params[0].symbol.isObj())if(Symbol * class_op=FindChild(func_name, func.params[0].symbol())())if( class_op->type==Symbol::FUNC_LIST)AddMatches(matches,  class_op->funcs, func.l_to_r, func.params.data()+1, func.params.elms()-1, func.symbol.templates, compiler, true); // add class non-static operators "Class::op(     params)", add extra match, because 'this' already fits perfectly, and unlike matching line above it is not included in param list so it doesn't benefit from full match and we must add this manually

            if(!matches.elms() && func.params.elms()==1) // if there are no matches
            {
               // continue with built-in operators
               if(func=='*' || func=='&')goto indirection_address_of;
            }
         }
      }else
      {
         compiler.msgs.New().error("Invalid token", origin); return CAST_NONE;
      }

      if(matches.elms())
      {
         // remove double functions (definition/declaration)
         REPA(matches){Symbol *a=matches[i].func; REPD(j, i)if(a->sameFunc(*matches[j].func)){matches.remove((a->modifiers&Symbol::MODIF_FUNC_BODY) ? i : j, true); break;}} // remove the one with function body (because it doesn't have default parameters listed, and full class path)

         Symbol *f=matches.sort(CompareLowest).first().func;
         func.symbol=func.symbol_origin=f; // change function symbol from FUNC_LIST to the actual FUNC (do this before calling 'castParentToSymbolClass')
         if(f && f->source && f->source->ee_header) // check if this is EE function
            if(func.func_call=FindFuncCall(FuncName(*f))) // if this function was found on the list
               func.func_call_mask=((f->hasResult() ? FC_RESULT : 0) | (f->isClassNonStaticFunc() ? FC_PARENT : 0)); // setup call mask (use 'f' when checking for hasResult and not T.symbol.value!=void because 'f' was used during creation of the func list, and all functions there, never have templates)

         if(matches.elms()>=2 && matches[0].lowest_match==matches[1].lowest_match && compiler.strict) // check if there are multiple functions with same match
         {
            if(!compiler.quiet)
            {
               Str s=S+"Multiple function candidates found for "+f->fullCppName()+'(';
               FREPA(func.params)
               {
                  if(i)s+=", ";
                  s+=func.params[i].symbol.modifName(true, true, false);
               }
               s+=')';
               Message &msg=compiler.msgs.New().error(s, origin);
               FREPA(matches)if(matches[i].lowest_match==matches[0].lowest_match)if(Symbol *func=matches[i].func)
               {
                  msg.children.New().set(func->definition(), func);
               }
            }
            return CAST_NONE;
         }
         if(matches[0].lowest_match<=CAST_NONE && compiler.strict) // check if the call is do-able
         {
            if(!compiler.quiet)compiler.msgs.New().error(S+"Invalid parameters for function call "+f->definition(), origin);
            return CAST_NONE;
         }

         // check if this is class non-static operator Class::op(params) which needs to move the 1st param as the parent
         if(ignore_first && f->isClassNonStaticFunc())
         {
            if(func.parent.elms()!=0 || func.params.elms()<=0){compiler.msgs.New().error(S+"Can't adjust class non-static operator", origin); return CAST_NONE;}
            Swap(func.parent.New(), func.params[0]); func.params.remove(0, true); // move the 1st param as the parent and remove from params list
         }

         // add default parameters
         if(compiler.strict)FREP(f->realParams())if(!InRange(i, func.params)) // order is important
         {
            Symbol *defaul=f->params[i]();
            if(defaul->modifiers&Symbol::MODIF_DEF_VALUE) // add param from default value
            {
               if(!defaul->source || compiler.compileTokens(defaul->def_val_range.x, defaul->def_val_range.y, func.params.New(), &defaul->source->tokens)!=COMPILE_FULL)
                  {if(!compiler.quiet)compiler.msgs.New().error(S+"Error compiling expression for default value", defaul); return CAST_NONE;}
            }else
            {
               if(!compiler.quiet)compiler.msgs.New().error(S+"Missing parameters for function call", origin); return CAST_NONE;
               break; // stop on first which does not have default value
            }
         }

         // verify number of parameters
         if(f->realParams()!=func.params.elms() && compiler.strict){compiler.msgs.New().error(S+"Invalid number of parameters for function call", origin); return CAST_NONE;}

         // detect unknown template typenames from parameters, for example "TYPE func(TYPE x)" check every parameter if it's a TYPENAME and then set it according to parameter type, "func(Str())" would set 'TYPE' to 'Str' and "func(&Str())" would set 'TYPE' to 'Str*'
         FREP(f->realParams())if(f->params[i]->value && f->params[i]->value->type==Symbol::TYPENAME && InRange(i, func.params))
         {
            Expr &src=func.params[i];
            if(src.symbol)
            {
               Symbol::Modif &t=func.symbol.templates.New();
               t=src.symbol;
               t.src_template=f->params[i]->value;
               // TODO: what about const_level, array_dims, templates ?
               t.ptr_level  -=f->params[i]->value.ptr_level; // yes it should be '-' because for "TYPE func(TYPE *x);   func(&TYPE());" : func.param[0].ptr_level==1, param[0].ptr_level==1
            }
         }

         // detect unknown template typenames from function specification "func<TYPE>();"
         // TODO: check if this is not in conflict with above (template setting, however this should be more important)
         FREPA(func.symbol.templates)
         {
            Symbol::Modif &tmplt=func.symbol.templates[i]; // get all acquired template infos
            if(InRange(tmplt.src_template_i, f->templates.elms())) // if we've set 'src_template_i' lately, check if it should belong to obtained function
               tmplt.src_template=f->templates[tmplt.src_template_i]; // set that template
            tmplt.src_template_i=-1;
         }

         // verify if func is non-static and we have a good object/parent for it (do this after we have gathered all templates)
         // TODO: if there is any non-instance in the parent then disable virtual function calls (do this earlier as at this stage non-instance parents could have been removed in earlier call to 'castParentToSymbolClass')
         if(compiler.strict && !func.castParentToSymbolClass(compiler))return CAST_NONE; // do this after setting 'func.symbol_origin'

         // cast parameters if needed (do this after getting all templates)
         if(compiler.strict)FREP(f->realParams())if(InRange(i, func.params))if(!func.params[i].castTo(f->params[i](), func.symbol.templates, compiler)){compiler.msgs.New().error("Can't cast", func.params[i].origin); return CAST_NONE;}

         symbol.templates=func.symbol.templates; // copy templates obtained from the function to the result
         toSymbol(f, true);
         if(!(symbol.modifiers&Symbol::MODIF_REF)) // for functions that don't return references
         {
            temporary=true;
            if(basicType())symbol.setConst(); // this is C++ behavior (can't modify returned basic types)
         }
         if(allow_mem_reuse && func.mem.type)memFrom(func.mem, compiler);else memNew(false, compiler); // set memory layout for the result

         // once we have the result we need to setup the parent/object/this for the constructors to be the same as the result
         if(compiler.strict && (f->modifiers&Symbol::MODIF_CTOR))
         {
            if(func.parent.elms()!=0){compiler.msgs.New().error("Constructor parent should be empty", origin); return CAST_NONE;}
            copyThis(func.parent.New());
         }
         return CAST_MAX;
      }else
      {
         if(!compiler.quiet)compiler.msgs.New().error("No function/operator found able to process command", origin); return CAST_NONE;
      }
   }
   compiler.msgs.New().error("Invalid token", origin); return CAST_NONE;
}
Expr& Func(Memc<Expr> &expr, Int func_i, Int params, Compiler &compiler) // calculate function call, which starts at 'expr[func_i]' (which specifies function or operator) followed by list of 'params' parameters, result will be put into expr[func_i] while previous func/operator and params will be moved from the 'expr' array into the result (as members Expr::func,params)
{
   if(!InRange(func_i, expr) || params<0 || params>=expr.elms()-func_i)Exit("Func out of range");

   Expr temp, // create element that will store the result
       &temp_func=temp.func.New(); // create func
   Swap(temp_func, expr[func_i]); // move 'func' from 'expr' into 'temp.func'
   temp_func.params.setNum(params); FREP(params)Swap(temp_func.params[i], expr[func_i+1+i]); // move 'params' from 'expr' into 'temp.func.params'
   CAST_MATCH cast=temp.calculate(compiler);

   // delete members because they will no longer be needed, this is to release memory and also avoid stack overflow, because the calculation tree can get very big for very long expressions such as "int x[]={0,1,2,3,4,..};"
   temp.parent.del();
   temp.func  .del();
   temp.params.del();

   expr.removeNum(func_i+1, params, true); // remove params from 'expr' !! this call may change memory addresses !!
   Expr &result=expr[func_i]; // set reference after removing params
   Swap(temp, result);
   return result;
}
/******************************************************************************/
Expr& Template(Memc<Expr> &expr, Int templ_i, Int params)
{
   if(!InRange(templ_i, expr) || params<0 || params>=expr.elms()-templ_i)Exit("Template out of range");

   Expr &templ=expr[templ_i];
   FREP(params)
   {
      Expr          &param=expr[templ_i+1+i];
      Symbol::Modif &dest =templ.symbol.templates.New();
      dest=param.symbol;
      if(templ.symbol)
      {
         if(templ.symbol->type==Symbol::FUNC_LIST)dest.src_template_i=i;else // we operate on FUNC_LIST so we don't know which exactly function we're going to call (as there can be "<TYPE> func(int a, TYPE b)", "<TYPE> func(float a, TYPE b)" , so instead of storing 'src_template' store its index 'src_template_i'
         if(InRange(i, templ.symbol->templates)  )dest.src_template  =templ.symbol->templates[i];
      }
   }

   REP(params)expr.remove(templ_i+1, true); // remove 'params' from 'expr' container !! after this call 'templ' can no longer be accessed !! (because memory addresses may changed)
   return expr[templ_i]; // don't use old reference (because memory addresses may changed)
}
/******************************************************************************/
Bool  Expr::possibleInstanceOrDataTypeOrNativeFunc(                     ) {return _operator==false;} // this function is used to disallow (prefix operator "this ++x") or ("this (x)" to "this x") - disallow: "int ++x" but allow: "return ++x" (currently 'return' and other similar keywords are assumed to be not set in Expr, so just ignore them)
Bool  Expr::knownFinal                            (Bool allow_func_lists) {return final && symbol && (allow_func_lists ? true : symbol->type!=Symbol::FUNC_LIST);}
Bool  Expr::instanceChild                         (                     ) {if(parent.elms())return parent[0].firstInstance()!=null; return false;} // if any of parents is an instance then this expr is a child and not full object
Bool  Expr::fullTemporary                         (                     ) {return temporary && !instanceChild();}
Expr* Expr::firstInstance                         (                     ) {for(Expr *expr=this; expr; expr=expr->parent.data())if(expr->instance)return expr; return null;}
/******************************************************************************/
Bool Expr::asBool(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return false;
      if(T=="false")return false;
      if(T=="true" )return true ;

      // pointers
      if(symbol.isPtr())return TextULong(asStr())!=0;

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return asChar(compiler)!=0;

         case VAR_FLT:
         case VAR_DBL: return TextDbl(asStr())!=0;

         case VAR_NULL: return false;

         default: return TextULong(asStr())!=0;
      }
   }
   return false;
}
Int Expr::asInt(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0;
      if(T=="false")return 0;
      if(T=="true" )return 1;

      // pointers
      if(symbol.isPtr())return TextInt(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextInt(asStr());

         case VAR_NULL: return 0;

         default: return TextInt(asStr());
      }
   }
   return 0;
}
UInt Expr::asUInt(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0;
      if(T=="false")return 0;
      if(T=="true" )return 1;

      // pointers
      if(symbol.isPtr())return TextUInt(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextUInt(asStr());

         case VAR_NULL: return 0;

         default: return TextUInt(asStr());
      }
   }
   return 0;
}
Long Expr::asLong(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0;
      if(T=="false")return 0;
      if(T=="true" )return 1;

      // pointers
      if(symbol.isPtr())return TextLong(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextLong(asStr());

         case VAR_NULL: return 0;

         default: return TextLong(asStr());
      }
   }
   return 0;
}
ULong Expr::asULong(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0;
      if(T=="false")return 0;
      if(T=="true" )return 1;

      // pointers
      if(symbol.isPtr())return TextULong(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextULong(asStr());

         case VAR_NULL: return 0;

         default: return TextULong(asStr());
      }
   }
   return 0;
}
Flt Expr::asFlt(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0.0f;
      if(T=="false")return 0.0f;
      if(T=="true" )return 1.0f;

      // pointers
      if(symbol.isPtr())return TextFlt(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextFlt(asStr());

         case VAR_NULL: return 0.0f;

         default: return TextFlt(asStr());
      }
   }
   return 0.0f;
}
Dbl Expr::asDbl(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return 0.0;
      if(T=="false")return 0.0;
      if(T=="true" )return 1.0;

      // pointers
      if(symbol.isPtr())return TextDbl(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16: return (UShort)asChar(compiler);

         case VAR_FLT:
         case VAR_DBL: return TextDbl(asStr());

         case VAR_NULL: return 0.0;

         default: return TextDbl(asStr());
      }
   }
   return 0.0;
}
Char CppChar(C Str &s, Int &i, Token *token, Compiler &compiler)
{
   if(s[i]=='\\')switch(s[(++i)++])
   {
      default  : compiler.msgs.New().error(S+"Unrecognized '"+s[i-1]+"' character escape sequence", token); return '\0';
      case 'n' : return '\n';
      case 't' : return '\t';
      case 'v' : return '\v';
      case 'b' : return '\b';
      case 'r' : return '\r';
      case 'f' : return '\f';
      case 'a' : return '\a';
      case '\\': return '\\';
      case '?' : return '\?';
      case '\'': return '\'';
      case '"' : return '\"';
      case '0' : return '\0';
      case 'x' :
      {
         UInt u=0; for(; i<s.length(); i++)
         {
            Int v=CharInt(s[i]);
            if( v>=0)u=u*16+v;else break;
         }
         return (Char)u;
      }break;
   }
   return s[i++];
}
Char Expr::asChar(Compiler &compiler)
{
   if(known())
   {
      // built-in constants
      if(T=="null" )return '\0';
      if(T=="false")return '\0';
      if(T=="true" )return '\1';

      // pointers
      if(symbol.isPtr())return (Char)TextInt(asStr());

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_CHAR8 :
         case VAR_CHAR16:
         {
            Str s=T;
            if((s[0]=='8' || s[0]=='u' || s[0]=='U' || s[0]=='L') && s[1]=='\'' && s.last()=='\'')s.remove(0); // eat 8,u,U,L from 8"..", u"..", U"..", L".."
            if(s[0]=='\'' && s.last()=='\'')s.removeLast().remove(0); // eat '' from '..'
            Int i=0; return CppChar(s, i, origin, compiler);
         }break;

         case VAR_FLT:
         case VAR_DBL: return (Char)TextInt(asStr());

         case VAR_NULL: return '\0';

         default: return (Char)TextInt(asStr());
      }
   }
   return '\0';
}
Memc<Char> Expr::asText(Compiler &compiler)
{
   Str s=T;
   if((s[0]=='8' || s[0]=='u' || s[0]=='U' || s[0]=='L') && s[1]=='"' && s.last()=='"')s.remove(0); // eat 8,u,U,L from 8"..", u"..", U"..", L".."
   if(s[0]=='"' && s.last()=='"')s.removeLast().remove(0); // eat "" from ".."
   Memc<Char> out; for(Int i=0; i<s.length(); )out.add(CppChar(s, i, origin, compiler)); out.add('\0');
   return     out;
}
U64 Expr::asRaw(Compiler &compiler)
{
   if(known() && is())
   {
      // pointers
      if(symbol.isPtr())return asLong(compiler);

      // values
      if(symbol.isObj() && symbol)switch(symbol->var_type)
      {
         case VAR_BOOL  : return asBool(compiler);

         case VAR_BYTE  :
         case VAR_SBYTE :
         case VAR_SHORT :
         case VAR_USHORT:
         case VAR_INT   :
         case VAR_UINT  :
         case VAR_ENUM  : return asInt(compiler);

         case VAR_LONG  :
         case VAR_ULONG : return asLong(compiler);

         case VAR_CHAR8 : return (Byte  )asChar8(compiler);
         case VAR_CHAR16: return (UShort)asChar (compiler);

         case VAR_FLT: {Flt f=asFlt(compiler); return (U32&)f;}
         case VAR_DBL: {Dbl d=asDbl(compiler); return (U64&)d;}
      }
   }
   compiler.msgs.New().error("Expression value is not known and can't be converted to raw memory"); return 0;
}
/******************************************************************************/
Expr& Expr::setOperator(CChar *op)
{
   symbol.clear(true);
   setBorrowed(op);
  _operator=true;
   instance=false;
   setPriority();
   return T;
}
Expr& Expr::setBasic(VAR_TYPE type)
{
   symbol.clear(true); toSymbol(TypeSymbol(type));
   symbol.const_level=1;
  _operator =false;
   instance =true;
   temporary=true;
   return T;
}
Expr& Expr::setPtr()
{
   symbol.clear(true); toSymbol(Symbols("ptr"));
   symbol.const_level=1|2;
  _operator =false;
   instance =true;
   temporary=true;
   return T;
}
/******************************************************************************/
Expr& Expr::toSymbol(Symbol *symbol, Bool func_call) // this function moves from current symbol (usually a parent) to 'symbol' (usually a child), keeping templates (templates are kept because in "<TYPE> class X {TYPE x;}" if we're moving from parent "X<int> obj; obj.x" then 'obj' (parent) has the template information and proceeding to 'x' symbol (child) we will still know its template and thus its type)
{
   T.symbol.clear(false); // keep templates
   T.symbol       =symbol;
   T.symbol_origin=symbol;

   if(!final)T.symbol.proceedToFinal(&T.symbol.templates, func_call); // for final we are interested only in the root 'symbol' (not its value) for example when we want to use jump-to-definition on function we want to calculate which FUNC is that

   if(T.symbol)AddBaseTemplates(T.symbol.templates, *T.symbol); // after getting the final value we need to add base templates for its class (new templates may override previous values)

   //if(T.symbol)BStr::setBorrowed(*T.symbol); adjust the name for debug purpose - DON'T DO THIS because it will destroy constants: "abc" 'a' 123 etc. !

   if(symbol_origin) // check 'T.symbol_origin' and not 'T.symbol', because for example 'T.symbol_origin' operates on root (like variable) while 'T.symbol' is already set to target type (such as type/class of variable)
   {
      if(symbol_origin->type==Symbol::VAR                                            // variable, for example 'x' in "int x"
      || symbol_origin->type==Symbol::FUNC || symbol_origin->type==Symbol::FUNC_LIST // functions are also instances, because in C++ they can be casted to pointers for example, this is correct!
      || symbol_origin->type==Symbol::ENUM_ELM)T.instance=true;                      // enum element is also an instance (just like constant integer '1')

      if(symbol_origin->type==Symbol::ENUM_ELM){T.temporary=true; T.symbol.const_level|=1; mem.setKnown();}
   }
   return T;
}
/******************************************************************************/
void Expr::setPriority()
{
   l_to_r=true;
   if(_operator)
   {
   #define UNARY_PRIORITY 17
      if(T=='.'  || T=="->" || T=="::")priority=19;else
      if(T=='('  || T==')' || T=='[' || T==']' || T==TMPL_B || T==TMPL_E || T=="++" || T=="--")priority=18;else
      if(T=='~'  || T=='!'/* || T=='*' || T=='&' || T=="+" || T=="-"*/){priority=UNARY_PRIORITY; l_to_r=false;}else
      if(T==".*" || T=="->*")priority=16;else
      if(T=='*'  || T=='/' || T=='%')priority=15;else
      if(T=='+'  || T=='-'  )priority=14;else
      if(T=="<<" || T==">>" )priority=13;else
      if(T=='<'  || T=='>'  || T=="<="  || T==">=" )priority=12;else
      if(T=="==" || T=="!=" || T=="===" || T=="!!=")priority=11;else
      if(T=='&' )priority=10;else
      if(T=='^' )priority= 9;else
      if(T=='|' )priority= 8;else
      if(T=="&&")priority= 7;else
      if(T=="^^")priority= 6;else
      if(T=="||")priority= 5;else
      if(T=='?' || T==':'){priority=4; l_to_r=false;}else
      if(T=='=' || T=="*=" || T=="/=" || T=="%=" || T=="+=" || T=="-=" || T=="<<=" || T==">>=" || T=="&=" || T=="|=" || T=="^="){priority=3; l_to_r=false;}else
      if(T==',' )priority= 2;else
         priority=1;
   }else priority=0;
}
/******************************************************************************/
void Expr::setConditional()
{
   conditional=true;
   REPAO(parent).setConditional();
   REPAO(func  ).setConditional();
   REPAO(params).setConditional();
}
void Expr::setBlockScope(Compiler &compiler, Bool force_heap)
{
   if(mem.type==Memory::LOCAL && InRange(mem.index, compiler.locals))
   {
      Local &local=compiler.locals[mem.index];
      local.block_scope=true;
      if(force_heap && !local.force_heap)
      {
         local.force_heap  =true;
         local.stack_offset=-1; // unuse the stack
      }
   }
}
/******************************************************************************/
void Expr::memAuto(Compiler &compiler)
{
   if(symbol_origin && symbol_origin->isVar())
   {
      if(mem.type)compiler.msgs.New().error("Expression already had memory layout set", origin);

      if(symbol_origin->modifiers&Symbol::MODIF_FUNC_PARAM) // function parameter
      {
         mem.setParam(*symbol_origin);
      }else
      if(symbol_origin->func()) // symbol belongs to a function - it's a local variable in a function
      {
         if(symbol_origin->modifiers&Symbol::MODIF_STATIC) // static variable inside function body
         {
            // TODO: support function static variables
         }else // non-static variable inside function body
         {
            if(!InRange(symbol_origin->raw_offset, compiler.locals)) // if the symbol was not yet initialized in the 'locals'
               symbol_origin->raw_offset=compiler.newLocal(symbol, true, origin); // set local variable type to the final symbol of this 'Expr' and NOT 'symbol_origin' (and set the returned index of the local variable)

            mem.setLocal(*symbol_origin);
         }
      }else
      if(symbol_origin->isGlobalOrStatic()) // global or class static variable
      {
         mem.setGlobal(*symbol_origin);
      }else
      {
         // class non-static members (this.member, obj.member), don't set this here because we don't know the class object for the member
      }
   }
}
void Expr::memNew(Bool known, Compiler &compiler)
{
   if(mem.type)compiler.msgs.New().error("Expression already had memory layout set", origin);
   if(known)mem.setKnown();else mem.setLocal(compiler.newLocal(symbol, false, origin));
}
void Expr::memFrom(Memory &mem, Compiler &compiler)
{
   if(T.mem.type)compiler.msgs.New().error("Expression already had memory layout set", origin);
   T.mem=mem;
}
void Expr::memOffset(Long offset)
{
   mem.addOffset(offset, FlagTest(symbol.modifiers, Symbol::MODIF_REF));
}
/******************************************************************************/
static void CastTo(Expr &expr, Symbol::Modif &dest, CAST_MATCH &max_cast, Expr &max_result, Compiler &compiler, RecTest &rt=ConstCast(RecTest())) // function which recursively updates the cast
{
   IntLock lock(rt); if(rt)return;

   Symbol::Modif &src=expr.symbol;
   if(src && dest)
   {
      //Bool is_null=(src=="null"), good_null=(is_null && dest.isPtr());
      UInt  src_const_level=src .const_level; FlagDisable( src_const_level, 1<<(src .ptr_level+src .array_dims.elms())); // disable top const
      UInt dest_const_level=dest.const_level; FlagDisable(dest_const_level, 1<<(dest.ptr_level+dest.array_dims.elms())); // disable top const
      Bool             same=src .same(dest, false, false);

      // check for reference able to modify source
      if((dest.modifiers&Symbol::MODIF_REF) && !dest.isConst()) // if expecting non-const reference - "TYPE&" or "TYPE *const *const * &" then proceed only if the 'src' is the same
      {
         if(same && !LostConst(src.const_level, dest.const_level))
         {
            max_cast  =((src.const_level==dest.const_level) ? CAST_MAX : CAST_MAX_CONST);
            max_result=expr;
            return; // we won't get any better cast
         }
         goto try_base; // do not try operator=, constructor and auto-cast, but go directly to base testing
      }

      // check for same type
      if(same)
         if(!LostConst(src.const_level, dest.const_level)                     // identical
         || !LostConst(src_const_level, dest_const_level) && src.basicType()) // allow "const int -> int" and "int *const-> int *"
      {
         max_cast  =((src.const_level==dest.const_level) ? CAST_MAX : CAST_MAX_CONST);
         max_result=expr;
         return; // we won't get any better cast
      }

      // allow conversion

      // try calling 'dest' constructor
      if(dest.basicType()) // try operator = (this will automatically calculate known value)
      {
         // =(expr_dest, expr_src) -> expr_dest=expr_src
         Expr result, &func=result.func.New(); func.origin=expr.origin; func.setOperator(u"="); func.params.setNum(2); // allocate parameters up-front so New will not change address of references
         Expr &expr_dest=func.params[0], &expr_src=func.params[1];

         // setup 'expr_src'
         expr_src=expr; // expr_src is the original (so copy the original 'expr' into 'expr_src')

         // setup 'expr_dest'
         expr_dest.origin   =expr_src.origin;
         expr_dest.instance =expr_src.instance;
         expr_dest.temporary=expr_src.temporary;
         expr_dest.symbol   =dest; expr_dest.symbol.setConst(false); FlagDisable(expr_dest.symbol.modifiers, Symbol::MODIF_REF); // disable main const to enable setting the value, and disable reference because we're still setting to a temporary local variable (or known const) which then later will be passed to the reference
         expr_dest.memNew(expr_src.known(), compiler); // if source is known then dest will be known as well, if not then we need to store it in locals

         // compile the 'operator='
         Bool quiet=compiler.quiet, allow_cast=compiler.allow_cast; compiler.quiet=true; compiler.allow_cast=false; // make sure error messages are not generated
         CAST_MATCH cast=result.calculate(compiler);
         compiler.quiet=quiet; compiler.allow_cast=allow_cast; // restore quiet and allow cast
         if(src.const_level!=dest.const_level)cast=CastMatchConst(cast); // adjust const if needed (do this here and not inside 'calculate' because there the 'expr_dest' has const disabled in order to allow setting the value)
         if(cast>max_cast){max_cast=cast; Swap(max_result, result); if(max_cast>=CAST_MAX_CONST)return;} // 'Swap' can be used because 'result' is local variable
      }else // try class constructor
      if(dest.isObj() && compiler.allow_cast)
      {
         // class Test { Test(Str str); Test(int x); }; void func(Test t); - func(0.0f) is GOOD, but func("") is BAD (here only direct match, or basic type conversions are allowed)
         Expr result, &func=result.func.New(); func.origin=expr.origin;
         func.symbol=dest;
         func.symbol.setConst(false); // disable const so it can be modified for further purpose
         FlagDisable(func.symbol.modifiers, Symbol::MODIF_REF); // disable reference (we're calling ctor)
         func.params.New()=expr; // this is the original expression

         Bool quiet=compiler.quiet, allow_cast=compiler.allow_cast; compiler.quiet=true; compiler.allow_cast=false; // make sure error messages are not generated
         CAST_MATCH cast=result.calculate(compiler); MIN(cast, CAST_CTOR);
         compiler.quiet=quiet; compiler.allow_cast=allow_cast; // restore quiet and allow cast
         if(cast>max_cast){max_cast=cast; Swap(max_result, result);} // 'Swap' can be used because 'result' is local variable
      }
         //expr.copyCtor
         /*if(Symbol *ctor=FindChild(*dest, dest(), null, false)())
            if(ctor->type==Symbol::FUNC_LIST)
               REPA(ctor->funcs)
         {
            Symbol *func=ctor->funcs[i]();
            if(func->params.elms()==1 || (func->params.elms()>=2 && (func->params[1]->modifiers&Symbol::MODIF_DEF_VALUE))) // if can be called by using only 1 parameter only (can accept 2 or more but all must have default values specified)
            {
               if(func->modifiers&Symbol::MODIF_FUNC_BODY)REPAD(j, ctor->funcs)if(i!=j && func->sameFunc(*ctor->funcs[j]))goto skip_func; // skip same functions with bodies (they have different templates and no default parameters)
               {
                  Symbol::Modif param; param=func->params[0]; param.proceedToFinal(&dest.templates);
                  
                  //if(param.same(src, false, false
                  
                  //Symbol::Modif param; param=func->params[0]; param.proceedToFinal(&templates); // dest.templates?

                  // CAST_CTOR

                  //if(param.same(dest, false, false) // if has constructor for source
                  //|| (param && param->type==Symbol::TYPENAME) // if this is unknown template
                  //  )return CAST_OK;

                  /*Bool identical=(!LostConst(src.const_level, param.const_level) && src.same(param, false, false));
                  if(  identical)return CAST_MAX;
                  if((param.modifiers&Symbol::MODIF_REF) && !param.isConst()) // if expecting non-const reference - "TYPE&" or "TYPE *const *const * &" then proceed only if the 'src' is the same
                  {
                     return identical ? CAST_MAX : CAST_NONE;
                  }*
               }
               skip_func:;
            }
         }*/

      // try calling auto-cast
      if(src.isObj() && !src.basicType() && compiler.allow_cast)
      {
         if(Symbol *auto_cast=FindChild("operator cast", src(), null, false)())
            if(auto_cast->type==Symbol::FUNC_LIST)
               REPA(auto_cast->funcs)
         {
            Symbol *func=auto_cast->funcs[i]();
            if(func->modifiers&Symbol::MODIF_FUNC_BODY)REPAD(j, auto_cast->funcs)if(i!=j && func->sameFunc(*auto_cast->funcs[j]))goto skip_auto_cast; // skip same functions with bodies (they have different templates and no default parameters)

            {
               //Symbol::Modif auto_cast; auto_cast=func; auto_cast.proceedToFinal(&templates); // src.templates?
               //if(auto_cast)
               {
                  //MAX(cast, CastMatch(auto_cast->var_type, auto_cast.ptr_level, to, templates    )-1);
                  //MAX(cast, CastMatch(auto_cast                               , to, templates, rt)-1);
               }
            }
            skip_auto_cast:;
         }
      }

      // check if any base class has better casting to the target
      // TODO: check if bases can be accessed (private, protected ..)
      // TODO: what if src is a pointer, can this still be used? perhaps only first base can be used, and only if it's at zero memory offset (watch out for 1st base non-virtual, but some other base or parent being virtual, in such case base would be offsetted by vfunc ptr)
   try_base:
      if(max_cast<CAST_MAX_CONST)REPA(src->base)
      {
         Expr base=expr; base.proceedToBase(i); // operate on temporary value so that 'expr' remains unmodified
         CastTo(base, dest, max_cast, max_result, compiler, rt); if(max_cast>=CAST_MAX_CONST)return;
      }
   }
}
static Bool CastToBase(Expr &expr, Symbol::Modif &dest, Expr &out, RecTest &rt=ConstCast(RecTest()))
{
   IntLock lock(rt); if(rt)return false;

   Symbol::Modif &src=expr.symbol;
   if(src.same(dest, false, false)){if(&out!=&expr)out=expr; return true;}
   // TODO: what if src is a pointer, can this still be used? perhaps only first base can be used, and only if it's at zero memory offset (watch out for 1st base non-virtual, but some other base or parent being virtual, in such case base would be offsetted by vfunc ptr)
   if(src)REPA(src->base)
   {
      // TODO: check if bases can be accessed (private, protected ..)
      Expr base=expr; base.proceedToBase(i); // operate on temporary value so that 'expr' remains unmodified
      if(CastToBase(base, dest, out, rt))return true;
   }
   return false;
}
Bool Expr::castToBase(Symbol::Modif &dest)
{
   return CastToBase(T, dest, T);
}
Bool Expr::castTo(Symbol::Modif &dest, Compiler &compiler) // try to convert 'this' to 'dest' by ('dest' ctor from 'this', or use 'this' auto-cast operator)
{
   if(!instance)return false; // can't convert from non-instance
   if(!(dest.modifiers&Symbol::MODIF_REF) && dest.isPtr() && T=="null")return true; // no conversion needed (test this in case "operator=" may not set "null" value in the result)

/*TODO: link templates? template_src.. (what about T.symbol.templates and dest.templates), template_src is already used in 'same' function
Memc<Symbol::Modif> linked_templates=templates; // if target accepts templates, and we know those templates (they are listed in 'templates', then add target template as known)
// for example target is "Memc<Int>" or "<TYPE> Memc<TYPE>" or "<TYPE> TYPE" ?
// TODO: should this be "dest->value.templates" or "to_final.templates" ?
FREPA(to_final.templates)
{
   Symbol::Modif &dest=to_final.templates[i];
   REPA(linked_templates)
   {
      Symbol::Modif &src=linked_templates[i];
      if(dest==src.src_template)
      {
         src.src_template=dest.src_template;
         break;
      }
   }
}*/

   CAST_MATCH cast=CAST_NONE; Expr result;
   CastTo(T, dest, cast, result, compiler);
   if(cast>CAST_NONE){Swap(T, result); return true;}
   return false;
}
Bool Expr::castTo(Symbol *dest, Memc<Symbol::Modif> &templates, Compiler &compiler)
{
   Symbol::Modif final; final=dest; final.templates=templates; final.proceedToFinal(&final.templates); return castTo(final, compiler); // copy templates in the modif and do proceedToFinal so "TYPE<TYPE2>" can be replaced with "Memc<Str>" (meaning, template class can be replaced with the target, and its templates can be as well, in other words keep the templates in modif.templates so we can pass "Memc<Str>" with "Str" in the templates)
}
CAST_MATCH Expr::canBeCastedTo(Symbol *dest, Memc<Symbol::Modif> &templates, Compiler &compiler)
{
   Symbol::Modif final; final=dest; final.templates=templates; final.proceedToFinal(&final.templates);
   if(!(final.modifiers&Symbol::MODIF_REF) && final.isPtr() && T=="null")return CAST_MAX;
   if(!compiler.strict && final && final->type==Symbol::TYPENAME)return CAST_CONV; // if dest is template then it can always be used
   CAST_MATCH cast=CAST_NONE; Expr result; CastTo(T, final, cast, result, compiler); return cast;
}
Bool Expr::castToConditional()
{
   if(symbol.isPtr())return true;
   if(symbol.isObj() && symbol)
   {
      switch(symbol->var_type)
      {
         case VAR_VOID  : return false; // can't be casted

         case VAR_BOOL  : // all of these types have their 'goto' commands defined in the func list
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
         case VAR_ENUM  : return true;

         case VAR_NONE  : // possible object
         {
            // TODO: add support for casting to bool (use some universal function used in other parts too)
         }break;
      }
   }
   return false;
}
Bool Expr::castParentToSymbolClass(Compiler &compiler)
{
   if(symbol_origin && symbol_origin->isClassNonStaticMember()) // expects 'this'
   {
      if(symbol_origin->modifiers&Symbol::MODIF_CTOR){parent.del(); return true;} // constructors can be called without specifying the parent

      // move all non-instance parents to temporary, sample data: "obj.Base::this" will move "obj.Base" to temp
      Expr parent_path;
      if(parent.elms() && !parent[0].instance)
      {
         Swap(parent[0], parent_path); parent.clear();  // move all parents to the temp
         if(Expr *instance=parent_path.firstInstance()) // get first instance in temp parent path
            Swap(parent.New(), *instance);              // put the instance into this.parent, this may leave the top parent_path as empty expression
      }

      // get the object
      Expr *parent_instance=parent.data();

      // if no instance is specified then use 'this'
      if(!parent_instance)
         if(origin && origin->parent)
            if(Symbol *caller_func=origin->parent->func())
               if(caller_func->isClassNonStaticFunc())
                  if(Symbol *caller_class=caller_func->Class())
      {
         parent_instance=&parent.New();
         if(!parent_instance->setThis(caller_func, origin, compiler))return false;
      }

      // verify the obtained instance
      if(!parent_instance || !parent_instance->instance){compiler.msgs.New().error(S+"Can't access class non-static member without specifying the class object", origin); return false;}
      if( parent_instance->symbol.array_dims.elms()    ){compiler.msgs.New().error(S+"Can't access class non-static member from an array"                      , origin); return false;}
      if( parent_instance->symbol.ptr_level>1          ){compiler.msgs.New().error(S+"Can't access class non-static member from deep pointer"                  , origin); return false;}
      if( parent_instance->symbol.ptr_level>0          )if(!parent_instance->indirection(compiler))return false;

      // follow the parent path (this is needed in case of "Class::Base1::Str" and "Class::Base2::Str"
      Memc<Symbol::Modif*> parents; for(Expr *cur=&parent_path; cur; cur=cur->parent.data())if(cur->symbol && cur->symbol->type==Symbol::CLASS)parents.add(&cur->symbol); // parents are now listed in C++ reversed order "empty_instance::base::class::namespace"
      REPA(parents)parent_instance->castToBase(*parents[i]); // process in reversed order to process in normal C++ order (first namespace, then class, then bases)

      // now we have the object, so verify that the 'symbol_origin' can be called from it
      Symbol *parent_symbol=parent_instance->symbol();
      Bool    allow_self   =true;
      if(parent_symbol && parent_symbol->type==Symbol::KEYWORD && (*parent_symbol=="super" || *parent_symbol=="__super"))
      {
         allow_self=false;
         if(parent_instance->parent.elms())
         {
            parent_symbol=parent_instance->parent[0].symbol();
         }else
         {
            parent_symbol=((origin && origin->parent) ? origin->parent->Class() : null);
         }
      }
      if(!symbol_origin->isMemberOf(parent_symbol, symbol.templates, origin ? origin->parent : null, true, false, allow_self, true)){compiler.msgs.New().error(S+"Can't access class non-static member from specified object", origin); return false;}

      // cast parent to class made from "symbol_origin->Class" with templates extracted from "this.templates"
      Symbol::Modif cast; cast=symbol_origin->Class(); cast.templates=symbol.templates;
      if(!parent_instance->castToBase(cast)){compiler.msgs.New().error("Can't cast object from its class to class of the member", parent_instance->origin ? parent_instance->origin : origin); return false;}

      // setup memory layout for the member
      if(symbol_origin->isVar())
      {
         if(!parent_instance || !parent_instance->symbol){compiler.msgs.New().error(S+"Something went wrong during member object calculation", origin); return COMPILE_FAILED;}

         Bool found; Int member_offset=parent_instance->symbol->memberOffset(symbol_origin, &found, &symbol.templates);
         if( !found){compiler.msgs.New().error(S+"Member not found in class", origin); return COMPILE_FAILED;}

         mem=parent_instance->mem;
         mem.addOffset(member_offset, FlagTest(parent_instance->symbol.modifiers, Symbol::MODIF_REF)); // here 'this.ref' should be ignored for 'memOffset' and only 'parent.ref' should be checked

         if(parent_instance->symbol.modifiers&Symbol::MODIF_REF)
         {
            if(symbol.modifiers&Symbol::MODIF_REF)
            {
               // if child is reference and parent is reference then create new local variable that will hold the address of the target child
               // TODO: 
               compiler.msgs.New().error("ref to ref not yet supported", origin);
            }else
            {
               symbol.modifiers|=Symbol::MODIF_REF; // parent is reference so it means we're still accessing the member through reference
            }
         }
      }
   }
   return true; // success
}
/******************************************************************************/
Bool Expr::addressOf(Compiler &compiler) // this function performs address-of on self
{
   Expr result, &func=result.func.New(); func.origin=origin; func.setOperator(u"&"); Swap(T, func.params.New());
   Swap(result, T);
   return calculate(compiler)>CAST_NONE;
}
Bool Expr::indirection(Compiler &compiler) // this function performs indirection on self
{
   Expr result, &func=result.func.New(); func.origin=origin; func.setOperator(u"*"); Swap(T, func.params.New());
   Swap(result, T);
   return calculate(compiler)>CAST_NONE;
}
Bool Expr::copyCtor(Compiler &compiler)
{
   Expr result, &func=result.func.New(); func.origin=origin;
   if(symbol.basicType() && !symbol.isArray()) // operator=
   {
      func.setOperator(u"=");
      func.params.setNum(2); // create params up-front so their memory is already allocated
      Expr &a=func.params[0], &b=func.params[1]; // "a=b"
      Swap(T, b);
      b.copyThis(a);
      a.temporary=true;
      a.symbol.setConst(false); // disable const so it can be modified for further purpose
      FlagDisable(a.symbol.modifiers, Symbol::MODIF_REF); // disable reference (we're copying to value and not reference)
      a.mem.clear();
      a.memNew(b.known(), compiler);
   }else // class ctor call
   if(!symbol.basicType() && symbol.isObj())
   {
      func.symbol=symbol;
      func.symbol.setConst(false); // disable const so it can be modified for further purpose
      FlagDisable(func.symbol.modifiers, Symbol::MODIF_REF); // disable reference (we're calling ctor)
      Expr &a=func.params.New();
      Swap(T, a);
      func.memNew(false, compiler);
   }else
   {
      compiler.msgs.New().error("Can't copy object", origin); return false;
   }
   Swap(result, T);
   return calculate(compiler)>CAST_NONE;
}
void Expr::proceedToBase(Int base_index) // this function moves self to T.parent, and sets self as the child
{
   if(symbol && InRange(base_index, symbol->base))
   {
      Symbol::Modif base=symbol->base[base_index]; base.proceedToFinal(&symbol.templates);
      base.modifiers  =symbol.  modifiers; // keep modifiers (especially MODIF_REF) so that if parent is reference then the base is a reference too
      base.  ptr_level=symbol.  ptr_level; // normally ptr_level should be zero, however keeping original value helps with auto-complete
      base.const_level=symbol.const_level; // normally ptr_level should be zero, however keeping original value helps with auto-complete
      base.array_dims =symbol.array_dims ; // normally ptr_level should be zero, however keeping original value helps with auto-complete

      Expr child; copyThis(child); // copy all params to child (except parent, func, params)
      Swap(child.parent.New(), T); // swap this with child.parent
      Swap(child             , T); // swap this with child

      // now we operate on the child
      BStr::clear(); // clear its value as we don't know it
      memOffset(symbol->baseOffset(base_index));
      symbol=base; // set child symbol
   }
}
/******************************************************************************/
Bool Expr::setThis(Symbol *caller, Token *origin, Compiler &compiler)
{
   BStr::setBorrowed(u"this");
   T.symbol.clear(true);
   T.origin=origin;
   if(caller)
      if(Symbol *func=caller->func())
         if(func->isClassNonStaticFunc())
   {
      toSymbol(func->Class());
      instance=true; symbol.const_level=FlagTest(func->modifiers, Symbol::MODIF_FUNC_CONST); // if the function is const "void func()const" then mark the data as const
      mem.setThis();
   }
   if(!symbol)compiler.msgs.New().error(S+"'this' can be used only in non-static class methods", origin);
   return symbol!=null;
}
void Expr::create(Token &src, Compiler &compiler, Bool final)
{
   BStr &bs=T; bs=src;
   T.symbol.clear(true);
   T.final = final;
   T.origin=&src;

   switch(src.type)
   {
      case TOKEN_CHAR8 : setBasic(VAR_CHAR8 ); mem.setKnown(); break;
      case TOKEN_CHAR16: setBasic(VAR_CHAR16); mem.setKnown(); break;
      case TOKEN_TEXT8 : setBasic(VAR_CHAR8 ); temporary=false; symbol.const_level=1|2; symbol.array_dims.add(Symbol::DIM_UNKNOWN); break; // convert "" text8 to 'char8[]' (include null-terminating character, make the characters and the array pointer itself both constants "1|2")
      case TOKEN_TEXT16: setBasic(VAR_CHAR16); temporary=false; symbol.const_level=1|2; symbol.array_dims.add(Symbol::DIM_UNKNOWN); break; // convert "" text  to 'char []' (include null-terminating character, make the characters and the array pointer itself both constants "1|2")
      case TOKEN_NUMBER:
      {
         VAR_TYPE type=VAR_NONE; REPA(bs)if(bs[i]=='.'){type=((bs.last()=='d') ? VAR_DBL : VAR_FLT); break;} // detect if real type
         if(!type) // detect int type
         {
            ULong u=TextULong(asStr()); // the text never has any '-' so read as ulong
            if(   u<= INT_MAX             )type=VAR_INT  ;else
            if(   u<=UINT_MAX             )type=VAR_UINT ;else
            if(   u<=0x7FFFFFFFFFFFFFFFull)type=VAR_LONG ;else // ULL = UNSIGNED LONG LONG
                                           type=VAR_ULONG;
         }
         setBasic(type); mem.setKnown();
      }break;

      case TOKEN_OPERATOR: _operator=true; separator=(src=='.' || src=="->" || src=="::"); setPriority(); break;

      default:
      {
         // check special symbols
         if(src=="this" ){if(setThis(src.parent, &src, compiler))addressOf(compiler);}else
         if(src=="true" ){setBasic(VAR_BOOL); mem.setKnown();}else
         if(src=="false"){setBasic(VAR_BOOL); mem.setKnown();}else
         if(src=="null" ){setBasic(VAR_NULL); mem.setKnown(); symbol.const_level=2; symbol.ptr_level=1;}else // make the pointer as constant (not the data to which it points to), always fit as KNOWN because 'null' is always zero
         {
            // TODO: detect enum value (set to BStr as Int)
            toSymbol(src.symbol());
            memAuto (compiler    ); // set mem after setting the symbol
         }
      }break;
   }
}
/******************************************************************************/
void Expr::copyThis(Expr &dest)
{
   dest.set(T);
   dest.instance=T.instance;
   dest.final=T.final;
   dest._operator=T._operator;
   dest.separator=T.separator;
   dest.l_to_r=T.l_to_r;
   dest.temporary=T.temporary;
   dest.conditional=T.conditional;
   dest.func_call_mask=T.func_call_mask;
   dest.priority=T.priority;
   dest.symbol=T.symbol;
   dest.symbol_origin=T.symbol_origin;
   dest.origin=T.origin;
   dest.func_call=T.func_call;
   dest.mem=T.mem;
}
/******************************************************************************/
// FUNCTION MATCH
/******************************************************************************/
void FuncMatch::create(Symbol *func, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match)
{
   T.func=func;
   average_match=(extra_match ? CAST_MAX+1 : CAST_NONE);
    lowest_match=CAST_MAX;
   FREP(Max(func->realParams(), params))
   {
      if(InRange(i, func->realParams()))
      {
         if(InRange(i, params)) // check match for param
         {
            Int param_match=param[i].canBeCastedTo(func->params[i](), templates, compiler);
            // TODO: for lowest_match disallow casting to non-const "TYPE &" if src is not compatible
               average_match+=param_match+1;
            MIN(lowest_match, param_match);
         }else
         if(func->params[i]->modifiers&Symbol::MODIF_DEF_VALUE) // custom param is not specified but there is default value in the function param
         {
            // don't modify
         }else // we haven't specified required param
         {
            average_match-=CAST_MAX+1;
             lowest_match =CAST_NONE;
         }
      }else // we've specified a parameter too much (doesn't fit in func param list)
      {
         average_match-=CAST_MAX+1;
          lowest_match =CAST_NONE;
      }
   }
}
void AddMatch(Memc<FuncMatch> &matches, Symbol *func, Bool l_to_r, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match)
{
   if(func && func->valid)
   {
      if(Symbol *parent=func->parent()) // func FUNC has "29386481" name, so check the FUNC_LIST parent
      {
         if(*parent=="operator*"
         || *parent=="operator&"
         || *parent=="operator+"
         || *parent=="operator-")
         {
            if(func->params.elms()!=params)return; // above funcs/operators can be both unary/binary, so make additional check for the number of parameters (this is to avoid situations of for example "Rect rect; &rect" when wanting to get the address of the object using unary '&', but there is custom binary operator '&' of Rect&Rect format)
         }else
         if(*parent=="operator++"
         || *parent=="operator--")
         {
            // OP  | l_to_r | params (dummy "int" param)
            // x++ |   1    |   1
            // x-- |   1    |   1
            // ++x |   0    |   0
            // --x |   0    |   0
            if(func->params.elms()!=Int(l_to_r))return;
         }
      }

      matches.New().create(func, param, params, templates, compiler, extra_match);
   }
}
void AddMatches(Memc<FuncMatch> &matches, Memc<SymbolPtr> &funcs, Bool l_to_r, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match)
{
   FREPA(funcs)AddMatch(matches, funcs[i](), l_to_r, param, params, templates, compiler, extra_match);
}
/******************************************************************************/
// COMPILER
/******************************************************************************
SAMPLE EXPRESSIONS:
   S.
   (S+S).
   Map<Image,Data>.
   Mems< Map<Image,Data> >.
   Image x[20];
   x[0].
   func<temp>().
   D.mode(320, 240).hdr(true).

TEMPLATE:
   x.
   x[].
   x().
   x<..>.
   x<..>().
   x[]().
   (..).

OPERATORS:
   UNARY  : ~x !x x++ x-- ++x --x
            *x &x +x -x (must be checked if token before *&+- operators isn't a value)
            x(..) x[..] type(x)
            (type)x
   BINARY : a+b a.b a->b a::b a,b ..
   TERNARY: a ? b : c

   SORTED BY PRIORITY:
      . -> ::
      () [] <T T> x++ x--
      ~ ! * & + - ++x --x (here '*' and '&' are indirection and address-of, and '+' and '-' are unary operators like "+x" or "-x")
      (type)              (cast)
      .* ->*
      * / %
      + -
      << >>
      < > <= >=
      == != === !!=
      &
      ^
      |
      &&
      ^^
      ||
      "a ? b : c"
      = *= /= %= += -= <<= >>= &= |= ^=
      ,

   TERNARY DESCRIPTION:
      "a ? b ? c : d : e" == "a ? (b ? c : d) : e"
      "a ? b : c ? d : e" == "a ? b : (c ? d : e)"


   Expression templates are setup as follow:
      1. all root expressions ("ROOT.", ".ROOT.", "ROOT.NOT_ROOT") have all 'class_base_templates' from current class "space->Class()" (NOT_ROOT cannot have 'class_base_templates' because later when merging symbols "parent.child" child 'class_base_templates' would have overriden parent templates (which are in form 1.class_base_templates, 2.value base templates), and it would be 1.class_base_templates, 2.value base templates, 3.class_base_templates)
      2. then all expressions have their templates overriden by value class base templates

/******************************************************************************/
COMPILE_RESULT Compiler::compileExpr(Memc<Expr> &expr, Symbol *space, Expr &out)
{
   IntLock lock(recursive); if(recursive>16){msgs.New().error("Compiler recursive loop", expr.elms() ? expr.first().origin : null); return COMPILE_FAILED;} // this can happen if calculating "int func(int par=func())" or "int f(int par=g()); int g(int par=f());"

   Str     temp;
   Symbol *Class=(space ? space->Class() : null);

   // merge < < and > > into << and >> (this can be done because at this stage all < > templates should be converted to TMPL_B and TMPL_E)
   FREPA(expr)if(InRange(i+1, expr))if(expr[i]=='<' && expr[i+1]=='<'
                                    || expr[i]=='>' && expr[i+1]=='>')
                                        if(expr[i]()+1==expr[i+1]()) // if text pointers are after each other
   {
      expr[i].final|=expr[i+1].final;
      expr[i].extend();
      expr[i].setPriority();
      expr.remove(i+1, true);
   }

   Memc<Symbol::Modif> class_base_templates;
   if(Class)AddBaseTemplates(class_base_templates, *Class);

   // setup templates and verify that already detected symbols indeed can be accessed
   FREPA(expr)
   {
      Expr &e=expr[i], *prev=(InRange(i-1, expr) ? &expr[i-1] : null);
      if(!(prev && prev->separator)) // if not preceeded by separator (check only root symbols, sub-symbols will be checked in the '.' merging)
      {
         FREPA(class_base_templates)e.symbol.templates.NewAt(i)=class_base_templates[i]; // set only for expressions not preceeded by separator (later below needs to be called for ".ROOT" too), add in this order because 'class_base_templates' need to be placed at start, to be least important
         if(strict && e.symbol_origin && !e.symbol_origin->canBeAccessedFrom(space, space, false, ProjectUsings)){msgs.New().error("Symbol can't be accessed from current space", e.origin); return COMPILE_FAILED;} // this needs to be checked here because later access is checked only on detecting of unknown yet symbols
         if(strict && !e.castParentToSymbolClass(T))return COMPILE_FAILED; // this will make sure that "member_var" have "this." parent set
      }
   }

   // processing loop
   for(; expr.elms(); )
   {
      Bool changed=false;

      if(final>=0)REPA(expr)if(expr[i].knownFinal(allow_func_lists)){Swap(out, expr[i]); return COMPILE_FULL;}

      // detect symbols
      FREPA(expr)
      {
         Expr &e=expr[i];
         if(  !e.symbol) // not yet detected
         {
            if(!e._operator) // starts with name
            {
               Expr *prev=(InRange(i-1, expr) ? &expr[i-1] : null);
               if( !(prev && prev->separator)) // if not preceeded by separator
               {
                  if(Symbol *s=FindSymbol(temp=e, space, ProjectUsings)())
                  {
                     changed=true; e.toSymbol(s); if(e.knownFinal(allow_func_lists)){Swap(out, e); return COMPILE_FULL;}
                     if(strict && !e.symbol_origin->canBeAccessedFrom(space, space, false, ProjectUsings)){msgs.New().error("Symbol can't be accessed from current space", e.origin); return COMPILE_FAILED;}
                     e.memAuto(T); // set mem after setting the symbol
                  }
               }
            }else
            if(e.separator) // encountered separator (this can be symbol with parent "obj.member" or global namespace "::EE::SomeFunc", this can also be object with base class specification "obj.Base<Str>::func")
            {
               Expr *prev =(InRange(i-1, expr) ? &expr[i-1] : null),
                    *prev2=(InRange(i-2, expr) ? &expr[i-2] : null);
               if( !(prev  && (*prev==']' || *prev==TMPL_E || (!prev->_operator && !prev->symbol)))) // don't process if it's ). ]. >. ?. (? = unknown symbol), however allow ")." if it's a cast "(TYPE).EE.SomeFunc" - perform additional checks below
               if( !(prev2 && prev2->separator)) // if not preceeded by operator
               {
                  if(prev && *prev==')') // test if we're preceeded by cast "(TYPE).x"
                  {
                     Expr *prev3=(InRange(i-3, expr) ? &expr[i-3] : null); // (
                     if(!(prev3 && *prev3=='(' && prev2 && prev2->symbol && (prev2->symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !prev2->instance))continue; // if it's not a cast then continue the loop
                  }

                  if(prev && prev->instance && prev->symbol && prev->symbol.isObj() && e=="->") // check for overloaded -> operator
                  {
                     // TODO: 
                     if(Symbol *s=FindChild("operator->", prev->symbol(), &prev->symbol.templates)())if(s->type==Symbol::FUNC_LIST && s->funcs.elms())
                     {
                        prev->final|=e.final;
                        prev->toSymbol(s->funcs[0](), true); MAX(--prev->symbol.ptr_level, 0); if(prev->knownFinal(allow_func_lists)){Swap(out, *prev); return COMPILE_FULL;}
                        e.setOperator(u"."); // replace "obj->" with "obj."
                        changed=true;
                        continue;
                     }
                  }

                  if(InRange(i+1, expr))
                  {
                     Expr   &next=expr[i+1];
                     Symbol *s   =next.symbol_origin;
                     Bool    prev_is_parent=(prev && prev->symbol);
                     if(!s)
                     {
                        Bool    allow_self=true;
                        Symbol *parent    =(prev_is_parent ? prev->symbol() : null);
                        if(prev && (*prev=="super" || *prev=="__super")) // we're referring to an element of a base class (when checking against "super" name, use Expr::BStr and not the symbol, because symbol may point to the target class already)
                        {
                           Expr *prev_parent=prev->parent.data();
                           parent=(prev_parent ? prev_parent->symbol() : Class); // this can be "obj.super::method()", "super::method"
                           allow_self=false;
                        }
                        s=FindChild(temp=next, parent, prev_is_parent ? &prev->symbol.templates : null, true, allow_self)();
                        if(!s) // if not found
                           if(prev_is_parent && prev->symbol->type==Symbol::CLASS && *prev->symbol==temp())s=prev->symbol();else // check "X::X"
                           if(Equal(temp, "super", true) || Equal(temp, "__super", true)                  )s=Symbols("super");
                     }
                     if(s)
                     {
                        if(prev_is_parent)
                        {
                           {FREPA(prev->symbol.templates)next.symbol.templates.NewAt(i)=prev->symbol.templates[i];} // inherit all template informations from the parent (in correct order)
                           Swap(next.parent.New(), expr[i-1]); // store 'parent' into 'child' "parent.child"
                        }
                        next.final|=e.final; next.toSymbol(s); if(next.knownFinal(allow_func_lists)){Swap(out, next); return COMPILE_FULL;}
                        if(!prev_is_parent)FREPA(class_base_templates)next.symbol.templates.NewAt(i)=class_base_templates[i]; // we need to call this here for ".ROOT" cases, check above similar usage of 'class_base_templates'

                        // setup memory layout (don't setup mem for class specification ".Class" or "obj.Base<Str>::member" because as in example 'next' "Base" may be followed by template "Str", and since there can be multiple bases "Base<Str>, Base<Int>" then don't proceed to child yet)
                        if(strict)
                        {
                           if( next=="this"){msgs.New().error("Keyword can't be used as a child", next.origin); return COMPILE_FAILED;}
                           if(!prev_is_parent && !next.symbol_origin->canBeAccessedFrom(null, space, true, ProjectUsings)){msgs.New().error("Symbol can't be accessed from current space", next.origin); return COMPILE_FAILED;}
                           if(!next.castParentToSymbolClass(T))return COMPILE_FAILED;
                           if(!next.symbol_origin->isClassNonStaticMember() && !next.mem.type)next.memAuto(T); // global, class static, memAuto can be used
                        }

                        // remove expressions
                                          expr.remove(i  , true); // remove "."
                        if(prev_is_parent)expr.remove(i-1, true); // remove "parent" / 'prev'
                        changed=true;
                     }
                  }
               }
            }
         }
      }

      // unary postfix operators
      FREPA(expr)
      {
         Expr &op=expr[i];
         if(op._operator)
            if(op=="++" || op=="--") // x++ x--
               if(InRange(i-1, expr))
         {
            Expr &value=expr[i-1];
            if(   value.instance)
            {
               Expr *prev=(InRange(i-2, expr) ? &expr[i-2] : null);
               if( !(prev && prev->separator)) // if not preceeded by separator
               {
                  Swap(op, value); // put operator as first
                  Expr &result=Func(expr, i-1, 1, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
                  changed=true;
               }
            }
         }
      }

      // unary prefix operators
      FREPA(expr)
      {
         Expr &op=expr[i];
         if(op._operator)
            if(op=='~' || op=='!' || op=='*' || op=='&' || op=='+' || op=='-' || op=="++" || op=="--") // ~x, !x, *x, &x, +x, -x, ++x, --x
               if(InRange(i+1, expr))
         {
            // use custom priority and associativity because some operators are placed in different groups (*&+- and ++ --)
            Int   priority=UNARY_PRIORITY;
            Bool  l_to_r  =false         ;
            Expr &value   =                       expr[i+1],
                 *next_op =(InRange(i+2, expr) ? &expr[i+2] : null),
                 *prev_op =(InRange(i-1, expr) ? &expr[i-1] : null);
            if(value.instance && !(prev_op && prev_op->possibleInstanceOrDataTypeOrNativeFunc())) // can't be preceeded by an instance (disallow: "int ++x" but allow: "return ++x")
               if(!prev_op || ((l_to_r ? prev_op->priority< priority : prev_op->priority<=priority) || *prev_op=='{' || *prev_op=='(' || *prev_op=='[' || *prev_op==TMPL_B || *prev_op==')')) // allow special case of ')' which is checked later
               if(!next_op || ((l_to_r ? next_op->priority<=priority : next_op->priority< priority) || *next_op=='}' || *next_op==')' || *next_op==']' || *next_op==TMPL_E))
            {
               if(prev_op && *prev_op==')') // test if we're preceeded by cast "(TYPE)&x"
               {
                  Expr *type    =(InRange(i-2, expr) ? &expr[i-2] : null), // TYPE
                       *prev_op2=(InRange(i-3, expr) ? &expr[i-3] : null); // (
                  if(!(prev_op2 && *prev_op2=='(' && type && type->symbol && (type->symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !type->instance))continue; // if it's not a cast then continue the loop
               }
               // adjust operator parameters
               op.l_to_r  =l_to_r  ;
               op.priority=priority;
               Expr &result=Func(expr, i, 1, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
               changed=true;
            }
         }
      }

      // binary operators
      FREPA(expr)
      {
         Expr &op=expr[i];
         if(op._operator)
            if(op=='*' || op=='/' || op=='%' || op=='+' || op=='-' || op=="<<" || op==">>" || op=='<' || op=='>' || op=="<=" || op==">=" || op=="==" || op=="!=" || op=="===" || op=="!!="
            || op=='&' || op=='^' || op=='|' || op=="&&" || op=="^^" || op=="||"
            || op=='=' || op=="*=" || op=="/=" || op=="%=" || op=="+=" || op=="-=" || op=="<<=" || op==">>=" || op=="&=" || op=="|=" || op=="^=")
               if(InRange(i-1, expr)
               && InRange(i+1, expr))
         {
            Expr &a      =                       expr[i-1],
                 &b      =                       expr[i+1],
                 *next_op=(InRange(i+2, expr) ? &expr[i+2] : null),
                 *prev_op=(InRange(i-2, expr) ? &expr[i-2] : null);
            if(a.instance && b.instance)
               if(!prev_op || ((op.l_to_r ? prev_op->priority< op.priority : prev_op->priority<=op.priority) || *prev_op=='{' || *prev_op=='(' || *prev_op=='[' || *prev_op==TMPL_B))
               if(!next_op || ((op.l_to_r ? next_op->priority<=op.priority : next_op->priority< op.priority) || *next_op=='}' || *next_op==')' || *next_op==']' || *next_op==TMPL_E))
            {
               Swap(op, a); // put operator as first
               Expr &result=Func(expr, i-1, 2, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
               changed=true;
            }
         }
      }

      // ternary operators "a ? b : c"
      FREPA(expr)
      {
         Expr &op=expr[i];
         if(   op=='?' && InRange(i-1, expr) && InRange(i+3, expr))
         {
            if(expr[i-1].instance && // a
                                     // ?
               expr[i+1].instance && // b
               expr[i+2]==':'     && // :
               expr[i+3].instance)   // c
            {
               Expr *prev_op=(InRange(i-2, expr) ? &expr[i-2] : null),
                    *next_op=(InRange(i+4, expr) ? &expr[i+4] : null);
               if(!prev_op || ((op.l_to_r ? prev_op->priority< op.priority : prev_op->priority<=op.priority) || *prev_op=='{' || *prev_op=='(' || *prev_op=='[' || *prev_op==TMPL_B))
               if(!next_op || ((op.l_to_r ? next_op->priority<=op.priority : next_op->priority< op.priority) || *next_op=='}' || *next_op==')' || *next_op==']' || *next_op==TMPL_E))
               {
                  Swap(expr[i], expr[i-1]); // put operator as first, now it's "? a b : c"
                  expr.remove(i+2, true  ); // remove ':' operatior , now it's "? a b c"
                  Expr &result=Func(expr, i-1, 3, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
                  changed=true;
               }
            }
         }
      }

      // convert "(x)" to "x", only if not preceeded by TOKEN_CODE|TOKEN_KEYWORD, like int(x) || f(x) || f<T>(x) || f()() || f[]()
      FREPA(expr)
      {
         if(expr[i  ]=='(' && InRange(i+2, expr)
         && expr[i+2]==')')
         {
            Expr *prev=(InRange(i-1, expr) ? &expr[i-1] : null);
            if( !(prev && (prev->possibleInstanceOrDataTypeOrNativeFunc() || *prev==')' || *prev==']' || *prev==TMPL_E)))
            {
               Expr &type=expr[i+1], *next=(InRange(i+3, expr) ? &expr[i+3] : null);
               if(type.symbol && (type.symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !type.instance && next) // if it's actually a cast "(TYPE)x"
               {
                  Expr *next2=(InRange(i+4, expr) ? &expr[i+4] : null);
                  if(next->instance)
                  {
                     if(!(next2 && (next2->separator || *next2=='(' || *next2=='[' || *next2==TMPL_B))) // don't cast yet when it's "(TYPE)x.", "(TYPE)x(", "(TYPE)x[", "(TYPE)x<"
                     {
                        type.instance=true;
                        expr.remove(i  , true); // remove '('
                        expr.remove(i+1, true); // remove ')'
                        expr.remove(i+1, true); // remove src
                        changed=true;
                     }
                  }else
                  if(!strict && *next=='.' && !next2) // if not using strict T and we have "(TYPE)." and nothing afterwards (next2==null) then most probably we're just evaluating the symbol for '.' dot adjustment, so just remove the "(TYPE)" part to allow proper detection of the '.' as the "::" for the global namespace in the 'adjustDot' function (see more info there)
                  {
                     expr.remove(i, true); // remove '('
                     expr.remove(i, true); // remove TYPE
                     expr.remove(i, true); // remove ')'
                     changed=true;
                  }
               }else
               {
                  expr.remove(i  , true); // remove '('
                  expr.remove(i+1, true); // remove ')'
                  changed=true;
               }
            }
         }
      }

      // [] operators, like x[i]
      FREPA(expr)
      {
         if(expr[i  ]=='[' && InRange(i+2, expr)
         && expr[i+2]==']' && InRange(i-1, expr))
         {
            Expr &value=expr[i-1];
            if(   value.instance)
            {
               Expr *prev=(InRange(i-2, expr) ? &expr[i-2] : null);
               if( !(prev && prev->separator)) // if not preceeded by separator
               {
                  expr[i].symbol.templates=expr[i-1].symbol.templates; // let "[]" operator have all 'x' object templates
                  Swap(expr[i-1], expr[i].parent.New()); // move   the 'x' into [.parent
                  expr.remove(i-1, true);                // remove the empty 'x', now it's "[ i ]"
                  expr[i-1].final|=expr[i+1].final;      // ].final to [.final
                  expr[i-1].setBorrowed(u"[]");          // set [ into []
                  expr.remove(i+1, true);                // remove ], now it's "[] i"
                  Expr &result=Func(expr, i-1, 1, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
                  changed=true;
               }
            }
         }
      }

      // cast modifiers: (TYPE*), (TYPE&), (TYPE*&), (TYPE *const), (const TYPE), (const TYPE *const), (const TYPE<A,B>.B), (const .TYPE.A), ..
      FREPA(expr)
      {
         Expr &type=expr[i];
         if(type.symbol && (type.symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !type.instance)
         {
            Expr *prev=(InRange(i-1, expr) ? &expr[i-1] : null);
            if( !(prev && prev->separator)) // if not preceeded by separator
            {
               if(InRange(i+1, expr)) // check for "TYPE*", "TYPE&", "TYPE const"
               {
                  Expr &modif=expr[i+1];
                  if(modif=='&'    ){type.symbol.  modifiers|=Symbol::MODIF_REF;          expr.remove(i+1, true); changed=true;}else
                  if(modif=='*'    ){type.symbol.  ptr_level++;                           expr.remove(i+1, true); changed=true;}else
                  if(modif=="const"){type.symbol.const_level|=(1<<type.symbol.ptr_level); expr.remove(i+1, true); changed=true;}
               }
               if(prev && *prev=="const") // check for "const TYPE"
               {
                  Expr *next=(InRange(i+1, expr) ? &expr[i+1] : null);
                  if( !(next && (next->separator || *next==TMPL_B))) // if not followed by separator or template
                  {
                     type.symbol.const_level|=1; expr.remove(i-1, true); changed=true; // add const to TYPE, remove 'const' expr
                  }
               }
            }
         }
      }

      // templates like "A<X>", "A<X,Y>" (valid parameters are non-operators)
      FREPA(expr)
      {
         if(expr[i]==TMPL_B && InRange(i-1, expr)) // 'i' points to '<'
         {
            Expr &templ=expr[i-1];
            if(   templ.symbol) // can be symbol of data type or function or native function
            {
               Expr *prev=(InRange(i-2, expr) ? &expr[i-2] : null);
               if( !(prev && prev->separator)) // if not preceeded by separator
               {
                  for(Int j=i+1, last_elm=-1; j<expr.elms(); j++) // last_elm: -1=start, 0=parameter, 1=comma(,)
                  {
                     Expr &param=expr[j];
                     if(   param==',') // comma
                     {
                        if(last_elm==1) // if last element was comma
                        {
                           if(strict){msgs.New().error("Unexpected ','", param.origin); return COMPILE_FAILED;}
                           expr.NewAt(j++); // insert empty expression between last and current comma - ',', ' ', ',' and increase the 'j' index so we don't process the same comma again in next step
                        }else last_elm=1; // set last element to comma
                     }else
                     if(param==TMPL_E) // end
                     {
                        if(last_elm==1) // if last element was comma
                        {
                           if(strict){msgs.New().error("Unexpected ','", param.origin); return COMPILE_FAILED;}
                           expr.NewAt(j++); // insert empty expression between last comma and current '>' - ',', ' ', '>' and increase the 'j' index so it will still point to the '>'
                        }
                        // 'j' points to '>'
                        Expr &templ=expr[i-1]; // set 'templ' reference again because 'expr' addresses may have changed
                                                                templ.final|=expr[i].final; expr.remove(i, true); j--;  // remove '<'
                                                                templ.final|=expr[j].final; expr.remove(j, true); j--;  // remove '>'
                        for(Int k=j; k>=i; k--)if(expr[k]==','){templ.final|=expr[k].final; expr.remove(k, true); j--;} // remove ','

                        Int   params=j-i+1;
                        Expr &result=Template(expr, i-1, params); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
                        changed=true;
                        break;
                     }else
                     if(!param._operator) // parameter
                     {
                        if(last_elm==0)break; // if last element was parameter then it means both parameters haven't been merged yet
                        last_elm=0; // set last element to parameter
                     }else
                     {
                        break; // invalid parameter
                     }
                  }
               }
            }
         }
      }

      // function calls, constructors/casting, operator(), native-functions, like: f(x), f(a,b,c), int(x), Vec2(0,1), D.staticFunc(0), object(0), sizeof(..) (valid parameters are instances)
      FREPA(expr)
      {
         if(expr[i]=='(' && InRange(i-1, expr)) // 'i' points to '('
         {
            Expr &func=expr[i-1];
            if(func.instance                                                     // function or object for operator()
            || (func.symbol && ((func.symbol->modifiers&Symbol::MODIF_DATA_TYPE) // constructor
                              || func.symbol->isNativeFunc())))                  // native func
            {
               Bool  size_of=(func=="sizeof");
               Expr *prev=(InRange(i-2, expr) ? &expr[i-2] : null);
               if( !(prev && prev->separator)) // if not preceeded by separator, needed because "D.mode(320,240)" could result into "D.D" if "mode(320,240)" would be computed before "D.mode"
               {
                  for(Int j=i+1, last_elm=-1; j<expr.elms(); j++) // last_elm: -1=start, 0=parameter, 1=comma(,)
                  {
                     Expr &param=expr[j];
                     if(   param==',') // comma
                     {
                        if(last_elm==1) // if last element was comma
                        {
                           if(strict){msgs.New().error("Unexpected ','", param.origin); return COMPILE_FAILED;}
                           expr.NewAt(j++); // insert empty expression between last and current comma - ',', ' ', ',' and increase the 'j' index so we don't process the same comma again in next step
                        }else last_elm=1; // set last element to comma
                     }else
                     if(param==')') // end
                     {
                        if(last_elm==1) // if last element was comma
                        {
                           if(strict){msgs.New().error("Unexpected ','", param.origin); return COMPILE_FAILED;}
                           expr.NewAt(j++); // insert empty expression between last comma and current ')' - ',', ' ', ')' and increase the 'j' index so it will still point to the ')'
                        }
                        // 'j' points to ')'
                        Expr &func=expr[i-1]; // set 'func' reference again because 'expr' addresses may have changed
                                                                func.final|=expr[i].final; expr.remove(i, true); j--;  // remove '('
                                                                func.final|=expr[j].final; expr.remove(j, true); j--;  // remove ')'
                        for(Int k=j; k>=i; k--)if(expr[k]==','){func.final|=expr[k].final; expr.remove(k, true); j--;} // remove ','

                        // check for operator()
                        if(func.instance && func.symbol && func.symbol->type!=Symbol::FUNC && func.symbol->type!=Symbol::FUNC_LIST && !func.symbol->isNativeFunc()) // if 'x' in "x(..)" is an object, then this is operator() call
                        {
                           Expr op; op.setOperator(u"()"); op.symbol.templates=func.symbol.templates; // let "()" operator have all 'x' object templates
                           Swap(func, op.parent.New()); // move 'func' (object  ) into "op.parent"
                           Swap(func, op             ); // move 'op'   (operator) into 'expr' array at 'func' position
                        }

                        Int   params=j-i+1;
                        Expr &result=Func(expr, i-1, params, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
                        changed=true;
                        break;
                     }else
                     if(param.instance || (size_of && param.symbol && (param.symbol->modifiers&Symbol::MODIF_DATA_TYPE))) // parameter (must be instance, however sizeof can accept data types as well)
                     {
                        if(last_elm==0)break; // if last element was parameter then it means both parameters haven't been merged yet
                        last_elm=0; // set last element to parameter
                     }else
                     {
                        break; // invalid parameter
                     }
                  }
               }
            }
         }
      }

      // process binary comma ',' operators as last to avoid confusions with commas for function/template parameters "func(a,b)", "<a,b>" which would change into "func(b)", "<b>"
      if(!changed)
         FREPA(expr)
      {
         Expr &op=expr[i];
         if(op._operator)
            if(op==',')
               if(InRange(i-1, expr)
               && InRange(i+1, expr))
         {
            Expr &a      =                       expr[i-1],
                 &b      =                       expr[i+1],
                 *next_op=(InRange(i+2, expr) ? &expr[i+2] : null),
                 *prev_op=(InRange(i-2, expr) ? &expr[i-2] : null);
            if(a.instance && b.instance)
               if(!prev_op || ((op.l_to_r ? prev_op->priority< op.priority : prev_op->priority<=op.priority) || *prev_op=='{' || *prev_op=='(' || *prev_op=='[' /*|| *prev_op==TMPL_B*/)) // don't allow inside templates "<a,b>"
               if(!next_op || ((op.l_to_r ? next_op->priority<=op.priority : next_op->priority< op.priority) || *next_op=='}' || *next_op==')' || *next_op==']' /*|| *next_op==TMPL_E*/))
            {
               if(prev_op && *prev_op=='(' && InRange(i-3, expr))
               {
                  Expr &func=expr[i-3];
                  if(func.possibleInstanceOrDataTypeOrNativeFunc() || func==')' || func==']' || func==TMPL_E)continue; // disallow "func(a,b", "x()(a,b", "x[i](a,b", "func<t>(a,b"
               }
               // C++ comma operator "a,b" results in "return" of 2nd expression "b"
               Swap(expr[i-1], expr[i]); // put operator as first, now it's ", a b"
               Expr &result=Func(expr, i-1, 2, T); if(result.knownFinal(allow_func_lists)){Swap(out, result); return COMPILE_FULL;}
               changed=true;
            }
         }
      }

      // end
      if(!changed)break;
   }

   // return
   if(expr.elms() && expr.last().separator && !strict) // if we were calculating "something x." then remove last separator (expressions which include the last separator are needed in case of "(TYPE*)obj." expressions, see comments in 'adjustDot' functions)
   {
      if(expr.elms()>=2)expr[expr.elms()-2].final|=expr.last().final; // merge final from last separator to expr before
      expr.removeLast(); // remove last separator
   }
   if(final>=0) // if final was requested
   {
      REPA(expr)if(expr[i].final)
      {
         Swap(out, expr[i]);
         return out.knownFinal(allow_func_lists) ? COMPILE_FULL : COMPILE_PARTIAL;
      }
      return COMPILE_FAILED;
   }
   if(expr.elms())
   {
      Swap(out, expr.last());
      return (expr.elms()==1) ? COMPILE_FULL : COMPILE_PARTIAL;
   }
   return COMPILE_FAILED;
}
COMPILE_RESULT Compiler::compileTokens(Int from, Int to, Expr &out, Memc<Token*> *tokens)
{
   Memc<Expr  > expr;
   Memc<Token*> &t=(tokens ? *tokens : T.tokens);
   return appendTokens(expr, from, to, tokens) ? compileExpr(expr, t[from]->parent, out) : COMPILE_FAILED;
}
Bool Compiler::appendTokens(Memc<Expr> &expr, Int from, Int to, Memc<Token*> *tokens)
{
   Memc<Token*> &t=(tokens ? *tokens : T.tokens);
   if(InRange(from, t)
   && InRange(to  , t))
   {
      for(Int i=from; i<=to; i++)expr.New().create(*t[i], T, i==final);
      return true;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
