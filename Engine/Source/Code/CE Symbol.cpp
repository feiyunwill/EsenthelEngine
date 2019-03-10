/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Int EnumSize=4, // 4 bytes
#if X64
     PtrSize=8; // 8 bytes (64-bits)
#else
     PtrSize=4; // 4 bytes (32-bits)
#endif
/******************************************************************************/
DEFINE_CACHE_EX(Symbol, Symbols, SymbolPtr, "Symbol", 512); // use high block size to improve performance because there are lot of symbols
/******************************************************************************/
Str SymbolToPath  (C Str &s) {return Replace(Replace(s, '.', SEP), '/', DIV);}
Str   PathToSymbol(C Str &s) {return Replace(Replace(s, SEP, '.'), DIV, '/');}
Str SymbolGetBase (C Str &s) {return PathToSymbol(GetBase(SymbolToPath(s)));}
Str SymbolGetPath (C Str &s) {return PathToSymbol(GetPath(SymbolToPath(s)));}

Str NamelessName(Symbol *parent) {return S+"Nameless "+(parent ? S+(parent->nameless_children++) : RandomName());}
/******************************************************************************/
Symbol* GetFinalSymbol(Symbol *symbol, Memc<Symbol::Modif> *templates) // simplified version of Symbol::Modif::proceedToFinal (skips modifiers, const_level, ptr_level, array_dims)
{
   for(RecTest rt; symbol; )
   {
      rt++; if(rt)break;

      if(symbol->type==Symbol::TYPENAME)
      {
         Symbol *temp=symbol;
         if(templates)REPA(*templates)
         {
            Symbol::Modif &templat=(*templates)[i];
            if(templat.src_template==symbol)
            {
               if(templat)symbol=templat();
               break;
            }
         }
         if(temp==symbol)break;
      }else
      if(symbol->isFunc())break;else // we can't proceed to the value of the function without calling it
      if(symbol->value)symbol=symbol->value();else
         break;
   }
   return symbol;
}
SymbolPtr FindChild(C Str &name, Symbol *parent, Memc<Symbol::Modif> *parent_templates, Bool allow_base, Bool allow_self, RecTest &rt)
{
   IntLock lock(rt); if(rt)return null;
   SymbolPtr s; if(allow_self && s.find(parent ? parent->full_name+SEP+name : name))if((s->valid || s->valid_decl) && (CppMode ? !(s->modifiers&Symbol::MODIF_ES_ONLY) : true))return s; // when in CppMode then skip Esenthel Script only keywords
   if(allow_base && parent && parent->type==Symbol::CLASS)FREPA(parent->base)
   {
      Symbol::Modif base=parent->base[i]; base.proceedToFinal(parent_templates);
      if(*base==name())return base; // check base itself
      if(s=FindChild(name, base(), &base.templates, allow_base, true, rt))return s; // check base members
   }
   return null;
}
SymbolPtr FindSymbol(C Str &name, Symbol *parent)
{
   if(SymbolPtr s=FindChild(name, parent))if(!(s->modifiers&Symbol::MODIF_CTOR_DTOR))return s;
   if(parent)return FindSymbol(name, parent->Parent());
   return null;
}
SymbolPtr FindSymbol(C Str &name, Symbol *parent, Memc<SymbolPtr> &usings)
{
   if(SymbolPtr s=FindSymbol(name, parent))return s;
   REPA(usings)if(SymbolPtr s=FindChild(name, usings[i]()))if(!(s->modifiers&Symbol::MODIF_CTOR_DTOR))return s;
   return null;
}
/******************************************************************************/
void AddBaseTemplates(Memc<Symbol::Modif> &templates, Symbol &Class, RecTest &rt)
{
   IntLock lock(rt); if(rt)return;

   REPA(Class.base) // iterate all bases in that class (process in reversed order, so first is most important)
      if(Symbol::Modif &base=Class.base[i])
   {
      AddBaseTemplates(templates, *base, rt); // add bases from that base first, so their templates are listed before the base templates
      REPA(base.templates)templates.add(base.templates[i]); // add all templates from that base (process in reversed order, so first is most important)
   }
}
/******************************************************************************/
// SYMBOL::MODIF CLASS METHODS
/******************************************************************************/
void Symbol::Modif::clear(Bool clear_templates)
{
   const_level=0; ptr_level=0; src_template_i=-1; modifiers=0; array_dims.clear(); src_template.clear();
   if(clear_templates)templates.clear();else REPAO(templates).src_template_i=-1;
   super::clear();
}
/******************************************************************************/
Bool Symbol::Modif:: isConst(       ) {return (const_level&(1<<(ptr_level+array_dims.elms())))!=0;}
void Symbol::Modif::setConst(Bool on) {FlagSet(const_level, 1<<(ptr_level+array_dims.elms()), on);}
/******************************************************************************/
Bool Symbol::Modif:: isObj   () {return !ptr_level && !array_dims.elms();}
Bool Symbol::Modif:: isPtr   () {return array_dims.elms() ? array_dims.last()==DIM_PTR : ptr_level>0;}
Bool Symbol::Modif:: isArray () {return array_dims.elms() && array_dims.last()!=DIM_PTR;}
Bool Symbol::Modif::anyPtr   () {if(ptr_level>0)return true; REPA(array_dims)if(array_dims[i]==DIM_PTR)return true; return false;}
Bool Symbol::Modif::isVoidPtr() {return ptr_level==1 && !array_dims.elms() && T && T->var_type==VAR_VOID;}
Bool Symbol::Modif::basicType() {return (T && T->var_type!=VAR_NONE) || ptr_level>0 || array_dims.elms();}
/******************************************************************************/
Bool Symbol::Modif::same(Modif &modif, Bool test_const, Bool test_ref)
{
   if(SCAST(SymbolPtr, T)==SCAST(SymbolPtr, modif)
   && ptr_level==modif.ptr_level
   && array_dims.elms()==modif.array_dims.elms())
   {
      REPA(array_dims)if(array_dims[i]!=modif.array_dims[i])return false;
      if(test_const &&  const_level         != modif.const_level         )return false;
      if(test_ref   && (modifiers&MODIF_REF)!=(modif.modifiers&MODIF_REF))return false;

      for(Symbol *cur=T(); cur; cur=cur->parent()) // check target templates of the symbol and its parents, for example "Memc<Byte> == Memc<Int>" (also parents because they also can have templates "<TYPE> class Main { <TYPE2> class Sub {}}" ) (bases don't need to be searched because they can depend only on the 'cur' temples only, so when checking cur we don't care about the base templates, example: "T1(TYPE) struct B : A<TYPE>")
         REPA(cur->templates)if(Symbol *t=cur->templates[i]()) // get 'TYPE' from "Memc<TYPE>"
      {
         // find that 'TYPE' in:
         Modif *a=null; REPA(      templates)if(      templates[i].src_template==t){a=&      templates[i]; break;} // 'T'     templates
         Modif *b=null; REPA(modif.templates)if(modif.templates[i].src_template==t){b=&modif.templates[i]; break;} // 'modif' templates
         if(a || b) // at least one is present
         {
            if(!a || !b)return false; // at least one is missing
            if(!a->same(*b, true, true))return false; // compare both templates, here test const and ref
         }
      }
      return true;
   }
   return false;
}
Bool Symbol::Modif::sameFuncParamValue(Modif &modif)
{
   if(!T() && !modif())return true ; // both are null
   if(!T() || !modif())return false; // one  is  null

   if(T()==modif() || (T->type==TYPENAME && modif->type==TYPENAME))
      if(const_level==modif.const_level && ptr_level==modif.ptr_level && array_dims.elms()==modif.array_dims.elms() && templates.elms()==modif.templates.elms() && ((modifiers&MODIF_SAME_FUNC_PARAM)==(modif.modifiers&MODIF_SAME_FUNC_PARAM))/* && src_template==modif.src_template*/)
   {
      REPA(array_dims)if( array_dims[i]                        != modif.array_dims[i]        )return false;
      REPA(templates )if(!templates [i]->value.sameFuncParamValue(modif.templates [i]->value))return false; // we need to check templates values (and not templates by themself, because they are only stored as TYPENAME or VAR, value has the actual type or null for TYPENAME, remember that TYPENAME point to different symbols because each template is created in the function location)
      return true;
   }
   return false;
}
Int Symbol::Modif::rawSize(Bool ref_as_ptr_size, RecTest &rt)
{
   // check reference at start
   if(ref_as_ptr_size && (modifiers&MODIF_REF))return PtrSize; // ignore arrays because "arrays of references are illegal in C++"

   // calculate total array dimensions
   Int dims=1; REPA(array_dims) // must go from the end to start
   {
      Int d=array_dims[i];
      if( d==DIM_PTR)return dims*PtrSize; // if encountered a pointer {1,2,PTR,4,5} then the size is 4*5*PtrSize
      if( d>=1)dims*=d;else
      {
         // TODO: change to compile error
         DEBUG_ASSERT(false, "Unknown array dimensions");
      }
   }

   if(ptr_level)return dims*PtrSize;
   if(T        )return dims*T->rawSize(ref_as_ptr_size, &templates, rt);

   // TODO: compile error
   return 0;
}
Int Symbol::Modif::firstMemberSize(RecTest &rt)
{
   if(modifiers&MODIF_REF)return PtrSize; // ignore arrays because "arrays of references are illegal in C++"
   if(anyPtr()           )return PtrSize;
   if(T                  )return T->firstMemberSize(&templates, rt);
   return 0;
}
Bool Symbol::Modif::hasConstructor(RecTest &rt)
{
   return !(modifiers&MODIF_REF) && !anyPtr() && T && T->hasConstructor(&templates, rt);
}
Bool Symbol::Modif::hasDestructor(RecTest &rt)
{
   return !(modifiers&MODIF_REF) && !anyPtr() && T && T->hasDestructor(&templates, rt);
}
Bool Symbol::Modif::constMemAddrOK()
{
   REPA(templates)
   {
      Modif &templat=templates[i];
      if(templat.src_template && (templat.src_template->modifiers&MODIF_CONST_MEM_ADDR)) // "<const_mem_addr TYPE> class Memc {..}" if source template has 'const_mem_addr' specified, then it means it will not preserve constant memory address
         if(templat && (templat->modifiers&MODIF_CONST_MEM_ADDR)) // if this element requires constant memory address "const_mem_addr class Thread"
            if(!(templat.modifiers&MODIF_CONST_MEM_ADDR)) // and not marked to ignore "Memc<const_mem_addr Thread>"
               if(!templat.anyPtr()) // "Memc<Thread*>" is ok, so ignore pointers
                  return false;
      if(!templat.constMemAddrOK())return false; // if any sub-templates have issue then fail too, "Memx< Memc<const_mem_addr Thread> >" the Memx is OK but the sub Memc should fail
   }
   return true;
}
Str Symbol::Modif::modifName(Bool spaces, Bool include_const, Bool include_ref, Bool invalid_array_as_1, Bool template_space)
{
   Str s;
   if((const_level&1) && include_const)s="const ";
   s+=(T ? T->fullCppName() : S+"unknown");
   if(T && T->templates.elms()) // Memc<>
   {
      s+='<';
      FREPA(T->templates)
      {
         if(i){if(spaces)s.space(); s+=',';}
         Symbol *src=T->templates[i]();
         Modif  *target=null;
         REPA(templates)if(templates[i].src_template==src){target=&templates[i]; break;}
         if(target)s+=target->modifName(spaces);else s+=*src; // here include const and ref
      }
      if(template_space && s.last()=='>')s+=' ';
      s+='>';
   }

   FREP(ptr_level)
   {
      if(spaces)s.space(); s+='*';
      if(include_const){Int ci=i-1; if(ci>=0 && (const_level&(1<<ci)))s+="const";}
   }
   REPA(array_dims) // process in reversed order
   {
      Int d=array_dims[i];
      if( d==DIM_PTR)
      {
         if(spaces)s.space(); s+='*';
         if(include_const){Int ci=i-1-ptr_level; if(ci>=0 && (const_level&(1<<ci)))s+="const";}
      }else
      {
         s+='['; if(d>=1)s+=d;else if(invalid_array_as_1)s+='1';
         s+=']';
      }
   }
   if((modifiers&MODIF_REF) && include_ref)
   {
      if(spaces)s.space(); s+='&';
   }
   return s;
}
void Symbol::Modif::proceedToFinal(Memc<Modif> *templates, Bool func_call, RecTest &rt) // 'templates'=container of all known templates !! this may be T.templates !!
{
   IntLock lock(rt); if(rt)return;

   for(RecTest lrt; T; )
   {
      lrt++; if(lrt)break;

      if(T->isFunc()){if(func_call)func_call=false;else break;} // we can't proceed to the value of the function without calling it (we can use only one call at a time)

      if(T->type==Symbol::TYPENAME)
      {
         Symbol *temp=T();
         if(templates)
            REPA(*templates) // go from end (from most recent templates, in case there are many of the same 'src_template')
               if((*templates)[i].src_template==T)
         {
            Modif temp=(*templates)[i]; // we need to store local copy, because 'templates' may belong to this (T.templates), in which case "T.proceedTo(T.templates[i])" may cause errors
            proceedTo(temp, rt);
            break;
         }
         if(temp==T())break;
      }else
      if(T->value)proceedTo(T->value, rt);else
         break;
   }
   if(templates)REPAO(T.templates).proceedToFinal(templates, false, rt);
}
void Symbol::Modif::proceedTo(Modif &src, RecTest &rt) // new order of const,ptr should be : src, T (since 'src' is the more low-level)
{
   IntLock lock(rt); if(rt)return;

   T           =SCAST(SymbolPtr, src);
 //src_template=src.src_template; // don't modify 'src_template' because we need to preserve the original so when searching for original we find it but with new target symbol (T, modifiers, ptr_level, ..)
   modifiers  |=src.modifiers   ;
   const_level =src.const_level|(T.const_level<<(src.ptr_level+src.array_dims.elms())); // set const keeping 'src.const' as more low-level and 'T.const' as high-level
   // set 'ptr_level' and 'array_dims' (order: src.ptr_level, src.array_dims, T.ptr_level, T.array_dims)
   if(src.array_dims.elms())
   {
      Mems<Int> array_dims; // setup new 'array_dims'
      array_dims.setNum(src.array_dims.elms() + T.ptr_level + T.array_dims.elms()); // total number of elements = src.array + T.ptr + T.array
      FREPA(src.array_dims)array_dims[                                  i]=src.array_dims[i]; // set src.array
      FREP (T  .ptr_level )array_dims[src.array_dims.elms()+            i]=          DIM_PTR; // set T  .ptr
      FREPA(T  .array_dims)array_dims[src.array_dims.elms()+T.ptr_level+i]=T  .array_dims[i]; // set T  .array
      T.ptr_level=src.ptr_level;                                                              // set src.ptr
      Swap(T.array_dims, array_dims);
   }else
   {
      ptr_level+=src.ptr_level; // increase 'ptr_level', and keep 'array_dims' as it is
   }

   // we need to add all templates from 'src' to T
   FREPA(src.templates) // order is important
   {
      Modif templat=src.templates[i]; // first create as temporary variable
      templat.proceedToFinal(&templates, false, rt); // 'templat' may be a TYPENAME so use 'proceedToFinal' with all known templates
      Swap(templates.New(), templat); // after we have processed it, we can add it to the known templates
   }
}
/******************************************************************************/
// SYMBOL CLASS METHODS
/******************************************************************************/
Str Symbol::typeName()
{
   switch(type)
   {
      case KEYWORD  : return "Keyword";
      case PREPROC  : return "Preprocessor Keyword";
      case ENUM     : return "Enum";
      case ENUM_ELM : return "Enum Element";
      case TYPEDEF  : return "Typedef";
      case NAMESPACE: return "Namespace";
      case CLASS    : return (modifiers&MODIF_UNION) ? "Union" : "Class";
      case TYPENAME : return "Typename";
      case FUNC_LIST: return "Function List";
      case FUNC     : return "Function";
      case VAR      : return "Variable";
      case SPACE    : return "Local Space";
      default       : return S;
   }
}
Str Symbol::fileName() {if(modifiers&MODIF_NAMELESS)return S+"nameless-"+nameless_index; return Replace(full_name, SEP, SEP_FILE);}
Str Symbol:: cppName() // returns base name only (return NAMELESS names because 'fullCppName' requires it)
{
   if(isFunc()   )return parent ? PathToSymbol(*parent) : S; // sample FUNC name "Namespace/Func/B123FE", so return parent FUNC_LIST name
   if(type==SPACE)return S;
   return T;
}
Str Symbol::fullCppName()
{
   Str out;
   for(Symbol *symbol=this; symbol; symbol=symbol->parent())
      if(!symbol->isFunc() && !(symbol->type==CLASS && (symbol->modifiers&MODIF_TRANSPARENT))) // skip FUNC "8761283" and transparent classes (still process nameless classes because of case "class A { class AA {int x;}a,b; int x; };" - there is A.x and A.nameless.x (accessed through A.a.x and A.b.x)
   {
      Str s=symbol->cppName(); if(s.is()){if(out.is())out=s+'.'+out;else out=s;}
   }
   return out;
}
Str Symbol::shortName()
{
   Str    name=cppName(); if(Starts(name, "operator", true, true))name=_SkipStart(name, "operator");
   return name;
}
/******************************************************************************/
Str Symbol::definition()
{
   if(source && InRange(token_index, source->tokens))
   {
      if(isVar() || type==FUNC || type==ENUM || type==ENUM_ELM || type==NAMESPACE || type==CLASS || type==TYPEDEF)
      {
         return source->getText(type_range.x, type_range.y)+' '+source->getText(def_range.x, def_range.y);
      }
   }
   return cppName();
}
Str Symbol::funcDefinition(Int highlight)
{
   if(source && InRange(token_index, source->tokens) && type==FUNC)
   {
      Str s=source->getText(def_range.x, def_range.y);
      Int level=0; REPA(s)if(s[i]==')')level--;else if(s[i]=='(')if(!++level){s.clip(i); break;} // remove param list
      s=source->getText(type_range.x, type_range.y)+' '+s;
      s+='(';
      FREPA(params)
      {
         Symbol &param=*params[i];
         if(i)s+=", ";
         if(i==highlight)s+="[color=F00]";
         s+=source->getText(param.type_range.x, param.def_range.y);
         if(i==highlight)s+="[/color]";
      }
      s+=')';
      return s;
   }
   return cppName();
}
Str Symbol::comments()
{
   if(type==KEYWORD) // some keywords are a typedef (like 'ptr')
   {
      if(T=="continue")return "proceed to the next step of current loop";
      if(T=="break"   )return "break current loop or case statement";
      if(T=="return"  )return "return from function";
      if(T=="if"      )return "Perform conditional code.\n\nSample usage:\n\n   int x;\n   if(x>0)..; // do codes if x is greater than 0";
      if(T=="goto"    )return "Jump to specified label (code position).\n\nSample usage:\n\n   for(int x=0; x<10; x++)\n   {\n      for(int y=0; y<10; y++)\n      {\n         if(..)goto break_both_loops;\n      }\n   }\nbreak_both_loops: // specify label";
      if(T=="sizeof"  )return "Obtain size of element in bytes.\n\nSample usage:\n\n   byte b;\n   sizeof(b   ); // 1\n   sizeof(byte); // 1\n\n   int i;\n   sizeof(i  ); // 4\n   sizeof(int); // 4";
      if(T=="this"    )return "Get reference to self.\n\nSample usage:\n\n   class Class\n   {\n      int value;\n\n      void set(int value) {this.value=value;}\n   }";
      if(T=="super"  || T=="__super")return "Access member/method of a base class.\n\nSample usage:\n\n   class BaseClass\n   {\n      void method() // base method\n      {\n      }\n   }\n   class ExtendedClass : BaseClass\n   {\n      void method() // extended method\n      {\n         super.method(); // call base method\n      }\n   }";
      if(T=="switch" || T=="case" || T=="default")return "keyword used to perform different codes basing on a value of parameter.\n\nSample usage:\n\nint x;\nswitch(x)\n{\n   case 0: .. break; // do codes for 0 value\n\n   case 1: .. break; // do codes for 1 value\n\n   case 2:\n   case 3: .. break; // do codes for 2,3 values\n\n   default: .. break; // do codes for all other values\n}";
      if(T=="const_mem_addr")return "Element must be stored in constant memory address.\nIf you see this keyword next to a class declaration,\nyou must ensure that when defining objects of that class\nyou will store them in constant memory address,\nthis can be either global namespace or inside 'Memx' 'Meml' containers.";

      if(T==  "bool"  || T=="Bool"                 )return "Boolean value 'true' or 'false' (8-bit size).";
      if(T==  "char"  || T== "Char"                )return       "Unicode Character (16-bit size).";
      if(T== "cchar"  || T=="CChar"                )return "Const Unicode Character (16-bit size).";
      if(T==  "char8" || T== "Char8"               )return          "Ansi Character (8-bit size).";
      if(T== "cchar8" || T=="CChar8"               )return    "Const Ansi Character (8-bit size).";
      if(T== "sbyte"  || T=="SByte"  || T=="I8"    )return   "Signed Integer -128 .. 127 (8-bit size).";
      if(T==  "byte"  || T=="Byte"   || T=="U8"    )return "Unsigned Integer 0 .. 255 (8-bit size).";
      if(T==  "short" || T=="Short"  || T=="I16"   )return   "Signed Integer -32 768 .. 32 767 (16-bit size).";
      if(T== "ushort" || T=="UShort" || T=="U16"   )return "Unsigned Integer 0 .. 65 535 (16-bit size).";
      if(T==  "int"   || T=="Int"    || T=="I32"   )return   "Signed Integer -2 147 483 648 .. 2 147 483 647 (32-bit size).";
      if(T== "uint"   || T=="UInt"   || T=="U32"   )return "Unsigned Integer 0 .. 4 294 967 295 (32-bit size).";
      if(T==  "long"  || T=="Long"   || T=="I64"   )return   "Signed Integer -9 223 372 036 854 775 808 ..  9 223 372 036 854 775 807 (64-bit size).";
      if(T== "ulong"  || T=="ULong"  || T=="U64"   )return "Unsigned Integer 0 .. 18 446 744 073 709 551 615 (64-bit size).";
      if(T==  "flt"   || T=="Flt"    || T=="float" )return "Floating Point 1-bit sign + 8-bit exponent + 23-bit fraction (32-bit size).";
      if(T==  "dbl"   || T=="Dbl"    || T=="double")return "Floating Point 1-bit sign + 11-bit exponent + 52-bit fraction (64-bit size).";
      if(T==  "ptr"   || T== "Ptr"                 )return "General Pointer (32-bit or 64-bit).";
      if(T== "cptr"   || T=="CPtr"                 )return "General Pointer to const data (32-bit or 64-bit).";
      if(T== "intptr" || T== "IntPtr"              )return   "Signed Integer capable of storing full memory address (32-bit or 64-bit).";
      if(T=="uintptr" || T=="UIntPtr"              )return "Unsigned Integer capable of storing full memory address (32-bit or 64-bit).";
   }
   if(source && InRange(token_index, source->tokens) && type!=NAMESPACE && type!=FUNC_LIST)
   {
      Token &token=*source->tokens[token_index];
      if(Line *line=token.line)
      {
         Str com;
         for(Int i=token.col; i<line->length(); i++)if(line->Type(i)==TOKEN_COMMENT)
         {
            Char c=(*line)[i];
            if(!(com.last()==' ' && c==' '))com+=c;
         }
         if(Starts(com, "//"))com=SkipStart(com, "//");
         com=SkipWhiteChars(com);
         return com;
      }
   }
   return S;
}
Str Symbol::commentsCode()
{
   Str    comm=comments(); if(comm.is())comm=S+" [color=080]// "+comm+"[/color]";
   return comm;
}
/******************************************************************************/
Symbol* Symbol::Parent()
{
   Symbol *p=T.parent(); for(; p && (p->modifiers&MODIF_TRANSPARENT); )p=p->parent();
   return  p;
}
Symbol* Symbol::rootClass()
{
   Symbol *root=null; for(Symbol *symbol=this; symbol; symbol=symbol->parent())if(symbol->type==CLASS)root=symbol;
   return  root;
}
Symbol* Symbol::Class()
{
   for(Symbol *symbol=this; symbol; symbol=symbol->parent())if(symbol->type==CLASS)return symbol;
   return null;
}
Symbol* Symbol::Namespace()
{
   for(Symbol *symbol=this; symbol; symbol=symbol->parent())if(symbol->type==NAMESPACE)return symbol;
   return null;
}
Symbol* Symbol::func()
{
   for(Symbol *symbol=this; symbol; symbol=symbol->parent())
   {
      if(symbol->type==FUNC                            )return symbol;
      if(symbol->type==CLASS || symbol->type==NAMESPACE)return null; // "void func() { class A { int a; void met() {} } }" // let "int a" tokens being child of "class A" don't return 'func'
   }
   return null;
}
Symbol* Symbol::rootFunc()
{
   Symbol *root=null; for(Symbol *symbol=this; symbol; symbol=symbol->parent())if(symbol->type==FUNC)root=symbol;
   return  root;
}
Symbol* Symbol::firstNonTransparent()
{
   for(Symbol *symbol=this; symbol; symbol=symbol->parent())if(!(symbol->modifiers&MODIF_TRANSPARENT))return symbol;
   return null;
}
Bool Symbol::contains(Symbol *symbol)
{
   for(; symbol; symbol=symbol->parent())if(this==symbol)return true;
   return false;
}
/******************************************************************************/
Token* Symbol::getToken() {return source ? source->getToken(token_index) : null;}
/******************************************************************************/
Bool Symbol::insideFunc()
{
   for(Symbol *cur=Parent(); cur; cur=cur->Parent())if(cur->type==FUNC || cur->type==FUNC_LIST)return true; // use Parent because when testing FUNC its parent is FUNC_LIST which would result in FUNC being inside func
   return false;
}
Bool Symbol::insideClass()
{
   if(source)
   {
      Int start=source->getSymbolStart(token_index); // symbol start must be used for functions "template<typename X> void Class::func(X param)" token.parent of 'func' could have been changed to detect templates
      if(InRange(start, source->tokens))return source->tokens[start]->parent==Parent();
   }
   return false;
}
/******************************************************************************/
Symbol::ACCESS_LEVEL Symbol::highestAccess()
{
   if(type==FUNC_LIST)
   {
      ACCESS_LEVEL level=ACCESS_PRIVATE; REPA(funcs)if(!(funcs[i]->modifiers&MODIF_OUTSIDE_METHOD))MAX(level, ModifToAccessLevel(funcs[i]->modifiers)); // ignore class members defined outside of class because they are not aware of access modifiers "class X{private: void method();} void X::method() {}" - while parsing 2nd method access is unknown
      return       level;
   }
   return ModifToAccessLevel(modifiers);
}
Bool Symbol::fullyStatic()
{
   if(type==FUNC_LIST)
   {
      REPA(funcs)if(!(funcs[i]->modifiers&MODIF_STATIC))return false; // if at least one is not static
      return true;
   }
   return FlagTest(modifiers, MODIF_STATIC);
}
Bool Symbol::partiallyStatic()
{
   if(type==FUNC_LIST)
   {
      REPA(funcs)if(funcs[i]->modifiers&MODIF_STATIC)return true; // if at least one is static
      return false;
   }
   return FlagTest(modifiers, MODIF_STATIC);
}
/******************************************************************************/
Bool Symbol::isNativeFunc          () {return type==KEYWORD && (T=="sizeof" || T=="typeid" || T=="dynamic_cast" || T=="static_cast" || T=="const_cast" || T=="reinterpret_cast");}
Bool Symbol::isGlobal              () {return !Parent() || Parent()->type==NAMESPACE;}
Bool Symbol::isVar                 () {return type==VAR || (type==FUNC &&  (func_ptr_level || (modifiers&MODIF_REF)));}
Bool Symbol::isFunc                () {return               type==FUNC && !(func_ptr_level || (modifiers&MODIF_REF)) ;}
Bool Symbol::isClassNonStaticMember() {return (Parent() && Parent()->type==CLASS) && !(modifiers&MODIF_STATIC);}
Bool Symbol::isClassNonStaticFunc  () {return isFunc()                 && isClassNonStaticMember();}
Bool Symbol::isGlobalFunc          () {return isFunc()                 && isGlobal();}
Bool Symbol::isGlobalAndStatic     () {return (modifiers&MODIF_STATIC) && isGlobal();}
Bool Symbol::isGlobalOrStatic      () {return (modifiers&MODIF_STATIC) || isGlobal();}
Bool Symbol::isTemplateClass       ()
{
   for(Symbol *cur=this; cur && cur->type==CLASS; cur=cur->parent())if(cur->templates.elms())return true; // if any of class parents has templates
   return false;
}
Bool Symbol::insideTemplateClass()
{
   if(Symbol *parent=Parent())return parent->isTemplateClass();
   return false;
}
Bool Symbol::isTemplateFunc()
{
   if(isFunc())
   {
      if(templates.elms()                   // func is "template func" if it has templates
      || insideTemplateClass())return true; // or any of its class parents   has templates
   }
   return false;
}
Bool Symbol::isInlineFunc()
{
   if(isFunc())
   {
      if(modifiers&MODIF_INLINE             // inline, below check for templates, because template functions are also inline
      || templates.elms()                   // func is "template func" if it has templates
      || insideTemplateClass())return true; // or any of its class parents   has templates
   }
   return false;
}
Bool Symbol::isAbstractClass(Memc<Modif> *templates, RecTest &rt)
{
   // TODO: this doesn't check class bases
   IntLock lock(rt); if(rt)return false;
   REPA(children)if(Symbol *func_list=children[i])if(func_list->type==FUNC_LIST)
   {
      REPA(func_list->funcs)if((func_list->funcs[i]->modifiers&MODIF_VIRTUAL) && (func_list->funcs[i]->modifiers&MODIF_DEF_VALUE))return true; // check all functions for 'virtual' and "=NULL"
   }
   return false;
}
Bool Symbol::hasVirtualDestructor(Memc<Modif> *templates, RecTest &rt) // assumes 'T.children' is set
{
   IntLock lock(rt); if(rt)return false;
   REPA(children)if(Symbol *func_list=children[i])if(func_list->modifiers&MODIF_DTOR) // if we've encountered destructor
   {
      REPA(func_list->funcs)if(func_list->funcs[i]->modifiers&MODIF_VIRTUAL)return true; // check all functions
      break; // don't check other function lists
   }
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base->hasVirtualDestructor(&base.templates, rt))return true;
   }
   return false;
}
Bool Symbol::hasVirtualFunc(Symbol &func, Memc<Modif> *templates, RecTest &rt) // assumes 'T.children' is set
{
   IntLock lock(rt); if(rt)return false;
   C Str &func_name=((func.parent && func.parent->type==FUNC_LIST) ? *func.parent : func); // get the name from the function list if possible
   REPA(children)if(Symbol *func_list=children[i])if(func_list->type==FUNC_LIST && *func_list==func_name()) // found function list with identical name
   {
      REPA(func_list->funcs)if(Symbol *f=func_list->funcs[i]())if((f->modifiers&MODIF_VIRTUAL) && func.sameFunc(*f))return true; // continue checking other functions because there can be multiple same (with body, or without, or multiple declarations)
      break;
   }
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base->hasVirtualFunc(func, &base.templates, rt))return true;
   }
   return false;
}
Bool Symbol::isVirtualClass(Memc<Modif> *templates, RecTest &rt) // assumes 'T.children' is set
{
   IntLock lock(rt); if(rt)return false;
   REPA(children)if(Symbol *func_list=children[i])if(func_list->type==FUNC_LIST) // if we've encountered function list
   {
      REPA(func_list->funcs)if(func_list->funcs[i]->modifiers&MODIF_VIRTUAL)return true; // check all functions
   }
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base->isVirtualClass(&base.templates, rt))return true;
   }
   return false;
}
Bool Symbol::isVirtualFunc(Memc<Modif> *templates) // assumes 'T.children' is set
{
   if(modifiers&MODIF_VIRTUAL)return true;
   if(modifiers&MODIF_DTOR   ) // destructor needs to be checked separately (there are no virtual constructors, so skip them)
   {
      if(Symbol *Class=T.Class())return Class->hasVirtualDestructor(templates);
   }else
   if(isClassNonStaticFunc()) // if at least one identical function in base classes is virtual then this is virtual too
   {
      if(Symbol *Class=T.Class())return Class->hasVirtualFunc(T, templates);
   }
   return false;
}
Bool Symbol::hasConstructor(Memc<Modif> *templates, RecTest &rt) // assumes 'T.children' is set
{
   IntLock lock(rt); if(rt)return false;
   REPA(children)
      if(Symbol *child=children[i])
         if(!(child->modifiers&MODIF_STATIC)) // non-static child
   {
      if(child->modifiers&MODIF_CTOR)return true; // we have found constructor
      if(child->isVar() && (child->modifiers&MODIF_DEF_VALUE))return true; // if is a variable (include function pointers) and has default value specified
      REPA(child->funcs)if(child->funcs[i]->modifiers&MODIF_VIRTUAL)return true; // if at least one function is virtual, then this class needs constructor
      if( child->type==VAR                                             // check variables (skip function pointers)
      || (child->type==CLASS && (child->modifiers&MODIF_TRANSPARENT))) // transparent class
      {
         Modif value=child->value; value.proceedToFinal(templates); if(value.hasConstructor(rt))return true; // if any type of variable has constructor, then this class needs constructor
      }
   }
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base.hasConstructor(rt))return true;
   }
   return false;
}
Bool Symbol::hasDestructor(Memc<Modif> *templates, RecTest &rt) // assumes 'T.children' is set
{
   IntLock lock(rt); if(rt)return false;
   REPA(children)
      if(Symbol *child=children[i])
         if(!(child->modifiers&MODIF_STATIC)) // non-static child
   {
      if(child->modifiers&MODIF_DTOR)return true; // we have found destructor
      REPA(child->funcs)if(child->funcs[i]->modifiers&MODIF_VIRTUAL)return true; // if at least one function is virtual, then this class needs destructor
      if( child->type==VAR                                             // check variables (skip function pointers)
      || (child->type==CLASS && (child->modifiers&MODIF_TRANSPARENT))) // transparent class
      {
         Modif value=child->value; value.proceedToFinal(templates); if(value.hasDestructor(rt))return true; // if any type of variable has destructor, then this class needs destructor
      }
   }
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base.hasDestructor(rt))return true;
   }
   return false;
}
Bool Symbol::hasResult(Memc<Modif> *templates, Modif *result_value)
{
   if(type==FUNC && !(modifiers&(MODIF_CTOR|MODIF_DTOR))) // constructors don't use return, destructors are always void
   {
      Modif value=T.value; value.proceedToFinal(templates);
      DEBUG_ASSERT(value && value->type!=TYPENAME, "Symbol::hasResult");
      if(!(value.isObj() && value && value->var_type==VAR_VOID)) // false for "void", true for everything else (including "void*")
      {
         if(result_value)Swap(*result_value, value);
         return true;
      }
   }
   return false;
}
Bool Symbol::fullyPublic()
{
   for(Symbol *cur=this; cur; cur=cur->Parent())if(ModifToAccessLevel(cur->modifiers)!=ACCESS_PUBLIC)return false;
   return true;
}
Int Symbol::templateClasses()
{
   Int    tc=0; for(Symbol *cur=Parent(); cur && cur->type==CLASS; cur=cur->Parent())if(cur->templates.elms())tc++; // if class has templates, can use Parent to skip transparent classes because those can't have templates, use Parent also to skip FUNC_LIST
   return tc;
}
Int Symbol::baseOffset(Int base_index, Memc<Modif> *templates, RecTest &rt)
{
   Int offset=0;

   // check virtual func table offset
   Bool base0_virtual=false; if(base.elms()){Modif base0=base[0]; base0.proceedToFinal(templates); if(base0 && base0->isVirtualClass(&base0.templates))base0_virtual=true;}
   if( !base0_virtual && isVirtualClass(templates))offset=PtrSize; // if this is virtual and 1st base is not, then we need to add virtual

   MIN( base_index, base.elms());
   FREP(base_index) // iterate the first bases
   {
      Symbol::Modif b=base[i]; b.proceedToFinal(templates);
      Int base_size=b.rawSize(true, rt),
          base_pos =AlignAddress(offset, Min(class_pack, b.firstMemberSize(rt)));
      offset=base_pos+base_size;
   }
   return offset;
}
Int Symbol::memberOffset(Symbol *member, Bool *found, Memc<Modif> *templates, RecTest &rt)
{
   if(found)*found=false;

   Int pos=baseOffset(base.elms(), templates, rt), total_size=pos;

   // iterate members
   FREPA(children)
      if(Symbol *child=children[i])
         if((child->isVar()     && !(child->modifiers&MODIF_STATIC     ))  // non-static variables
         || (child->type==CLASS &&  (child->modifiers&MODIF_TRANSPARENT))) // transparent classes
   {
      Int child_size=child->rawSize(true, templates, rt),
          child_pos =AlignAddress(pos, Min(class_pack, child->firstMemberSize(templates, rt)));
      if( child==member){if(found)*found=true; return child_pos;}
      if(modifiers&MODIF_UNION)MAX(total_size, child_pos+child_size); // if this 'class' is an 'union' then use max size of all members
      else                     pos=total_size= child_pos+child_size;
   }

   // make sure that first element is aligned at the end as well : "class A{int a; byte b;}" -> SIZE(A)==8 (4 for 'a', 1 for 'b', 3 padd so that next 'a' is aligned)
   return AlignAddress(total_size, Min(class_pack, firstMemberSize(templates, rt)));
}
Int Symbol::rawSize(Bool ref_as_ptr_size, Memc<Modif> *templates, RecTest &rt)
{
   IntLock lock(rt); if(rt)return 0;

   switch(type)
   {
      case FUNC: if(isVar())return PtrSize; break; // pointer to function

      case VAR: {Modif modif=value; modif.proceedToFinal(templates); return modif.rawSize(ref_as_ptr_size, rt);}

      case TYPEDEF: // check typedefs and keywords in the same group (because some keywords may be typedefs and vice-versa)
      case KEYWORD:
         if(value){Modif modif=value; modif.proceedToFinal(templates); return modif.rawSize(ref_as_ptr_size, rt);} // first check if it's a typedef
         return TypeSize(var_type);
      break;

      case ENUM: return EnumSize;

      case CLASS: return memberOffset(null, null, templates, rt);
   }
   return 0;
}
Int Symbol::firstMemberSize(Memc<Modif> *templates, RecTest &rt)
{
   IntLock lock(rt); if(rt)return 0;

   switch(type)
   {
      case FUNC: if(isVar())return PtrSize; break; // pointer to function

      case VAR: {Modif modif=value; modif.proceedToFinal(templates); return modif.firstMemberSize(rt);}

      case TYPEDEF: // check typedefs and keywords in the same group (because some keywords may be typedefs and vice-versa)
      case KEYWORD:
         if(value){Modif modif=value; modif.proceedToFinal(templates); return modif.firstMemberSize(rt);} // first check if it's a typedef
         return TypeSize(var_type);
      break;

      case ENUM: return EnumSize;

      case CLASS:
      {
         if(isVirtualClass(templates, rt))return Min(class_pack, PtrSize);
         FREPA(base){Symbol::Modif b=base[i]; b.proceedToFinal(templates); if(Int size=b.firstMemberSize(rt))return Min(class_pack, size);}

         Int max_size=0;
         FREPA(children)
            if(Symbol *child=children[i])
               if((child->isVar()     && !(child->modifiers&MODIF_STATIC     ))  // non-static variables
               || (child->type==CLASS &&  (child->modifiers&MODIF_TRANSPARENT))) // transparent classes
                  if(Int size=child->firstMemberSize(templates, rt))
                     if(modifiers&MODIF_UNION)MAX(max_size, size);          // from union we must pick the biggest element
                     else                     return Min(class_pack, size);
         return Min(class_pack, max_size);
      }break;
   }
   return 0;
}
Int Symbol::realParams()
{
   if(parent) // func FUNC has "29386481" name, so check the FUNC_LIST parent
      if(*parent=="operator++"
      || *parent=="operator--")
         return 0; // ignore dummy "(int)" param

   return params.elms();
}
/******************************************************************************/
Bool Symbol::sameFunc(Symbol &f)
{
   if(value.sameFuncParamValue(f.value))
   {
      if((modifiers&MODIF_SAME_FUNC)!=(f.modifiers&MODIF_SAME_FUNC))return false;
      if(params.elms()!=f.params.elms())return false;
      REPA(params)if(!(params[i]->value.sameFuncParamValue(f.params[i]->value)))return false;
      return true;
   }
   return false;
}
Bool Symbol::sameSymbol(Symbol &s)
{
   return this==&s // if exactly the same
       || (parent==s.parent && type==Symbol::FUNC && s.type==Symbol::FUNC && sameFunc(s)); // or 2 FUNC that share the same parent (FUNC_LIST) and are the same
}
/******************************************************************************/
Bool Symbol::constDefineInHeader() // if define const in header
{
   Symbol::Modif value; value=this; value.proceedToFinal(null);
   if(value)return value.const_level==1 && !value.anyPtr() && IntType(value->var_type); // !anyPtr: accept OBJ (int x=5) and ARRAY (int x[]={1, 2})
   return false;
}
Bool Symbol::fromPartialMacro()
{
   if(Token *token=getToken()) // found token
      if(token->macro_col>=0) // token was created by a macro
   {
      if(Token *prev=source->getToken(source->getSymbolStart(token_index)-1)) // if there's a token before the declaration of this symbol
         if(prev->sameMacro(*token))return true; // if that token was created by the same macro
      if(Token *next=source->getToken(source->getListEnd    (token_index)+1)) // if there's a token after  the declaration of this symbol
         if(next->sameMacro(*token))return true; // if that token was created by the same macro
   }
   return false;
}
/******************************************************************************/
Bool Symbol::hasBase(Symbol *Class, Memc<Modif> *templates, RecTest &rt)
{
   if(this==Class)return true;
   IntLock lock(rt); if(rt)return false;
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates); if(base->hasBase(Class, &base.templates, rt))return true;
   }
   return false;
}
Bool Symbol::hasNonPrivateBase(Symbol *Class, Memc<Modif> *templates, Bool allow_self, Bool test_private, RecTest &rt)
{
   if(allow_self && this==Class)return true;
   IntLock lock(rt); if(rt)return false;
   REPA(base)
   {
      Modif &b=base[i];
      if(test_private ? !FlagTest(b.modifiers, MODIF_PRIVATE) : true)
      {
         Modif base=b; base.proceedToFinal(templates); if(base->hasNonPrivateBase(Class, &base.templates, true, true, rt))return true;
      }
   }
   return false;
}
Bool Symbol::hasPrivateBase(Symbol *Class, Memc<Modif> *templates, Bool test_private, RecTest &rt)
{
   IntLock lock(rt); if(rt)return false;
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates);
      if(test_private ? FlagTest(base.modifiers, MODIF_PRIVATE) : false)
      {
         if(base->hasBase(Class, &base.templates, rt))return true;
      }else
      {
         if(base->hasPrivateBase(Class, &base.templates, true, rt))return true;
      }
   }
   return false;
}
Bool Symbol::hasNonPublicBase(Symbol *Class , Memc<Modif> *templates, Bool test_access, RecTest &rt)
{
   IntLock lock(rt); if(rt)return false;
   REPA(base)
   {
      Modif base=T.base[i]; base.proceedToFinal(templates);
      if(test_access ? FlagTest(base.modifiers, MODIF_PRIVATE|MODIF_PROTECTED) : false)
      {
         if(base->hasBase(Class, &base.templates, rt))return true;
      }else
      {
         if(base->hasNonPublicBase(Class, &base.templates, true, rt))return true;
      }
   }
   return false;
}
Bool Symbol::isMemberOf(Symbol *symbol, Memc<Symbol::Modif> &symbol_templates, Symbol *caller, Bool instance, Bool ctor_init, Bool allow_self, Bool allow_bases) // this checks only direct members of 'symbol' and if they can be called from 'caller' space (ignores parents of 'symbol', but includes bases of 'symbol'), 'instance'=if 'symbol' is an instance, 'ctor_init'=if this is a ctor init ("class A{ A():HERE(){}}")
{
   if(type==KEYWORD || type==PREPROC || type==TYPENAME)return false;

   Bool         caller_is_namespace=(!caller || caller->type==NAMESPACE);
   Symbol      *caller_class       =(caller ? caller->Class() : null),
               *Parent             =T.Parent(); // skip FUNC_LIST and transparent classes
   ACCESS_LEVEL access             =highestAccess();

   if((modifiers&MODIF_CTOR_DTOR) && !caller_is_namespace)return false; // constructors can be listed only in namespaces

   symbol=GetFinalSymbol(symbol);

   // if we're specifying something with class/namespace address then it's possible to use instance and non-instance symbols (if the class is caller or base of it), example "class X { int x; void met() {X.|} }"
   Bool ignore_instance=false;
   if(!instance)
      if(caller_class)
         if(Symbol *caller_func=caller->func()) // allow access only from functions
            if(caller_func->isClassNonStaticFunc()) // allow access only from non-static functions
               if(caller_class->hasNonPrivateBase(symbol))
                  ignore_instance=true;

   if(!ignore_instance)
   {
      if(instance)
      {
         if((type==CLASS && !allow_bases) || type==NAMESPACE || type==ENUM || type==ENUM_ELM || type==TYPEDEF/* || fullyStatic()*/)return false; // don't check for 'fullyStatic' because for example "EE.D.clear()" where 'clear' is static method but can be accessed
         if(access==ACCESS_PROTECTED && !caller_is_namespace)if(!symbol || !symbol->hasNonPrivateBase(caller_class, &symbol_templates))return false; // formula obtained using hand-made tests (works!)
      }else
      if(type==VAR || type==FUNC || type==FUNC_LIST) // if it's a member
         if(Parent && Parent->type==CLASS)
      {
         Bool allow_func=(symbol && symbol->type==Symbol::CLASS && caller_is_namespace); // if we're defining function body outside of class space (in some namespace) then allow listing functions as members "void Class::method"

         if(!( type==FUNC_LIST          && allow_func)
         && !((type==VAR || type==FUNC) && ctor_init ))
            if(!partiallyStatic())return false; // can't access non-static members from non-instance
      }
   }

   if(!Parent && !symbol                              )return true; // if both are global namespace
   if(ctor_init                                       )return symbol==Parent; // constructor initializers can be used only from the same class
   if(access==ACCESS_PRIVATE   && !caller_is_namespace)if(Parent!=caller_class                                     )return false; // private   members can be used only from the same class
   if(access==ACCESS_PROTECTED && !caller_is_namespace)if(!caller_class || !caller_class->hasNonPrivateBase(Parent))return false; // protected members can be used only from extended classes
   if(caller_class && caller_class->hasPrivateBase(Parent))return false;                                                                   // we're trying to access from extended class -> base     class                "class Base{int x;}   class Middle : private Base{}   class Ext : Middle{|}"
   if(symbol && symbol->hasNonPublicBase(Parent, &symbol_templates, true))if(!caller_class || !caller_class->hasBase(symbol))return false; // we're trying to access from base     class -> extended class -> base class) "class Base{void method(){Middle m; m.|}}   class Middle : private Base{}
   Symbol *base=Parent; if(allow_bases && type==CLASS)base=this;
   return symbol ? symbol->hasNonPrivateBase(base, &symbol_templates, allow_self) : false;
}
Bool Symbol::canBeAccessedFrom(Symbol *path, Symbol *caller, Bool precise_parent, Memc<SymbolPtr> &usings) // check if all symbols from 'path' and above (parents and bases are checked, however without children) can be accessed from 'caller' space
{
   if(type==KEYWORD && precise_parent)return false;
   if(modifiers&MODIF_CTOR_DTOR      )return false;

   Symbol *Parent=T.Parent(); // skip FUNC_LIST and transparent classes

   if(type==VAR || type==FUNC || type==FUNC_LIST)
   {
      Symbol *caller_class=(caller ? caller->Class() : null);
      if(highestAccess()==ACCESS_PRIVATE)if(Parent!=caller_class)return false; // private members can be used only from the same class

      if(Parent && Parent->type==CLASS && !partiallyStatic()) // if 'this' is a non-static member/method of a class
      {
         Bool accessible=false;
         if(caller_class)
            if(Symbol *caller_func=caller->func()) // allow access only from functions
               if(caller_func->isClassNonStaticFunc()) // allow access only from non-static functions
                  accessible=caller_class->hasNonPrivateBase(Parent);
         if(!accessible)return false;
      }
   }

   // check used namespaces
   REPA(usings)if(usings[i]==Parent)return true;

   // first parent of this must be included in 'path' or its parents list (this includes the case when parent==null) or their base list
   for(;;)
   {
      if(Parent==path)return true;
      if(       !path)return false;
      if(path->hasBase(Parent))return true;
      path=path->parent();
   }
}
/******************************************************************************/
void Symbol::adjustIndex(Memc<Token> &tokens, Int &i)
{
   if(InRange(i, tokens))i=tokens[i].source_index;
}
void Symbol::adjustIndexes(Memc<Token> &tokens)
{
   if(!(helper&HELPER_ADJUSTED_INDEXES)) // needed in case there are many symbol defs pointing to the same symbol
   {
      helper|=HELPER_ADJUSTED_INDEXES;
      adjustIndex(tokens,   token_index  );
      adjustIndex(tokens,    type_range.x);
      adjustIndex(tokens,    type_range.y);
      adjustIndex(tokens,     def_range.x);
      adjustIndex(tokens,     def_range.y);
      adjustIndex(tokens, def_val_range.x);
      adjustIndex(tokens, def_val_range.y);
   }
}
/******************************************************************************/
void Symbol::addDependency(Modif &symbol)
{
   Modif final=symbol;
   for(Int i=0; final && i<64; i++) // find first value which has 'rootClass'
   {
      if(Symbol *root=final->rootClass()) // we need to be actually dependent on the root class of symbol (because only root classes are stored as headers)
      {
         if(final->valid && !(final.modifiers&MODIF_REF) && !final.anyPtr()) // ignore references and pointers
         {
            if(final->type==TYPENAME)dependencies.include(final()); // if this is a TYPENAME, then add dependency to it and only to class using it "<TYPE> class A { TYPE t; }" A depends on TYPE, "<TYPE> class A { class Sub {TYPE t;} }" Sub depends on TYPE
            if(root!=this && !(root->helper&HELPER_PROCESSED))dependencies.include(root); // ignore self-dependencies
         }
         break;
      }
      final.proceedTo(final->value); // proceed to next value
   }
}
void Symbol::addDependencyIfNested(Symbol *symbol) // if 'symbol' is nested (defined inside some class) then add dependency to that "root class" to this (this ignores references and pointers because we are interested in only declaration of the symbol - its existence)
{
   if(symbol && symbol->type!=TYPENAME) // ignore unresolved templates
      if(Symbol *root=symbol->rootClass()) // get the root of symbol
         if(root!=symbol // if 'symbol' is nested
         && root!=this)  // if 'root' is not this
            if(!(root->helper&HELPER_PROCESSED))dependencies.include(root);
}
/******************************************************************************/
void Symbol::clearBody()
{
   if(isFunc() && (modifiers&MODIF_FUNC_BODY))
   {
      FlagDisable(helper, HELPER_FUNC_PARSED);
      nameless_children=0;
      ctor_inits.clear();
      if(source)for(Int i=source->getBodyStart(token_index), level=0; InRange(i, source->tokens); i++)
      {
         Token &token=*source->tokens[i];
         token.parent=this;
         if(token=='{')   ++level;else
         if(token=='}')if(--level<=0)break;
      }
   }
}
/******************************************************************************/
}}
/******************************************************************************/
