/******************************************************************************/
#include "stdafx.h"
//#define SUPPORT_SCRIPTS
namespace EE{
namespace Edit{
/******************************************************************************/
struct FuncCall
{
   CChar8    *name;
   Call::Func func;
};
struct GlobalVar
{
   CChar8 *name;
   CPtr    addr;
};
/******************************************************************************/
static Str SymbolToCppName(C Str &s)
{
   Str n='$';
   FREPA(s)
   {
      Char c=s[i];
      if(c>='a' && c<='z'
      || c>='A' && c<='Z'
      || c>='0' && c<='9'
      || c=='_')n+=c;else n+=S+'$'+TextHex(UInt(c), 2);
   }
   return n;
}
Str CppStyle(C Str &s) {return Replace(s, '.', "::");} // "EE.Str" -> "EE::Str"
Str ModifName(Symbol::Modif &m, Bool spaces=true, Bool include_const=true, Bool include_ref=true, Bool invalid_array_as_1=false, Bool template_space=false) // this function replaces basic types "Int" -> "int"
{
   //if(m && m->var_type!=VAR_NONE && m->var_type!=VAR_ENUM){Symbol::Modif temp=m; temp.proceedToFinal(&temp.templates); return temp.modifName(spaces, include_const, include_ref);}
   return m.modifName(spaces, include_const, include_ref, invalid_array_as_1, template_space);
}
Str FuncName(Symbol &func)
{
   if(!func.value)return S;
   Str n=ModifName(func.value, false)+' '+func.fullCppName()+'(';
   FREPA(func.params){if(i)n+=',';
   Symbol *param=func.params[i]();
   if(!param || !param->value)return S; n+=ModifName(param->value, false);}
   n+=')';
   if(func.modifiers&Symbol::MODIF_FUNC_CONST   )n        +="const";
 //if(func.modifiers&Symbol::MODIF_FUNC_FINAL   )n.space()+="final";
 //if(func.modifiers&Symbol::MODIF_FUNC_OVERRIDE)n.space()+="override";
   return n;
}
Str DtorName(Symbol &Class)
{
   return S+"void "+Class.fullCppName()+".~"+Class+"()";
}
Str NativeOperatorName(C Str &op, Symbol::Modif &result, Symbol::Modif &a)
{
   return result.modifName(false, false, false)+" operator"+op+'('+a.modifName(false, false, false)+')';
}
Str NativeOperatorName(C Str &op, Symbol::Modif &result, Symbol::Modif &a, Symbol::Modif &b)
{
   return result.modifName(false, false, false)+" operator"+op+'('+a.modifName(false, false, false)+','+b.modifName(false, false, false)+')';
}
/******************************************************************************/
static CChar8 *op_change []={"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "|=", "^="},
              *op_compare[]={"==", "===", "!=", "!!=", "<", ">", "<=", ">=", "&&", "^^", "||"},
              *op_binary []={"*", "/", "%", "+", "-", "<<", ">>", "&", "^", "|"},
              *op_inc_dec[]={"++", "--"};
static Mems<FuncCall*> FC2; // 'FC' sorted by FuncCall::func
/******************************************************************************/
void CodeEditor::createFuncList()
{
   FileText  ft; ft.writeMem(UTF_8); // xcode had problems with utf-16
   Memc<Str> funcs, vars;

   CodeEnvironment::SetupChildren(); // needed for some functions

   // create call's
   {
      Str name;
      name="nop"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { thread.func_code_pos+=Call::Size(0); }");
   }

   // create call's
   {
      Str name; // use 'getInstructP' and 'getInstructI' for param 0 because they're always stored in INSTRUCT (adjust code positions before making the call, so thread.call will store new positions and will return to them later)
      name="call func"         ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { thread.func_code_pos+=Call::Size(1); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(),                          null,                          null); }");
      name="call func result"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { thread.func_code_pos+=Call::Size(2); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(),                          null, call.param(1).address(thread)); }");
      name="call method"       ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { thread.func_code_pos+=Call::Size(2); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(), call.param(1).address(thread),                          null); }");
      name="call method result"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { thread.func_code_pos+=Call::Size(3); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(), call.param(1).address(thread), call.param(2).address(thread)); }");
   }

   // create set const's
   {
      Str name; // use 'getInstructI' and 'raw' for params 0 and 1 because they're always stored in INSTRUCT
      name="set const 1"     ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(byte *)(thread.func_stack+call.param(0).getInstructI()) = (byte &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }");
      name="set const 2"     ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(short*)(thread.func_stack+call.param(0).getInstructI()) = (short&)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }");
      name="set const 4"     ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(int  *)(thread.func_stack+call.param(0).getInstructI()) = (int  &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }");
      name="set const 8"     ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(long *)(thread.func_stack+call.param(0).getInstructI()) = (long &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }");
      name="set const heap 1"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(byte *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,byte ); thread.func_code_pos+=Call::Size(2); }");
      name="set const heap 2"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(short*)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,short); thread.func_code_pos+=Call::Size(2); }");
      name="set const heap 4"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(int  *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,int  ); thread.func_code_pos+=Call::Size(2); }");
      name="set const heap 8"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(long *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,long ); thread.func_code_pos+=Call::Size(2); }");
   }

   // create set addr
   {
      Str name; // use 'getInstructI' for param 0 because it's always stored in INSTRUCT
      name="set addr heap"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(ptr*)(thread.heap      +call.param(0).getInstructI()) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }");
      name="set addr stack" ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(ptr*)(thread.func_stack+call.param(0).getInstructI()) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }");
      name="set addr result"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(ptr*)(thread.func_result                            ) = call.param(0).address(thread); thread.func_code_pos+=Call::Size(1); }");
   }

   // create indirection + addressOf
   {
      Str name;
      name="indirection"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { *(ptr*)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,ptr); thread.func_code_pos+=Call::Size(2); }"); // we can't use "PARAM(0,ptr)" because it's always STACK_REF and by using it we would access the value of the address not set yet
      name="address of" ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }");
      name="offset 1s"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,sbyte ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 1u"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,byte  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 2s"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,short ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 2u"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,ushort) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 4s"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,int   ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 4u"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,uint  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 8s"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,long  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="offset 8u"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,ulong ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
   }

   // create inc/dec ptr's
   {
      Str name;
      name="inc ptr 2"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { ++PARAM(0,short*); thread.func_code_pos+=Call::Size(1); }");
      name="dec ptr 2"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { --PARAM(0,short*); thread.func_code_pos+=Call::Size(1); }");
      name="inc ptr 4"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { ++PARAM(0,int  *); thread.func_code_pos+=Call::Size(1); }");
      name="dec ptr 4"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { --PARAM(0,int  *); thread.func_code_pos+=Call::Size(1); }");
      name="inc ptr 8"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { ++PARAM(0,long *); thread.func_code_pos+=Call::Size(1); }");
      name="dec ptr 8"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { --PARAM(0,long *); thread.func_code_pos+=Call::Size(1); }");
      name="inc ptr n"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) {   PARAM(0,byte *) += call.param(1).getInstructI(); thread.func_code_pos+=Call::Size(2); }"); // use 'getInstructI' for param 1 because it's always stored in INSTRUCT
      name="dec ptr n"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) {   PARAM(0,byte *) -= call.param(1).getInstructI(); thread.func_code_pos+=Call::Size(2); }"); // use 'getInstructI' for param 1 because it's always stored in INSTRUCT

      name="add ptr 1s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,sbyte ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 1u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,byte  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 2s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,short ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 2u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,ushort) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 4s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,int   ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 4u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,uint  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 8s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,long  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");
      name="add ptr 8u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,byte*) += PARAM(1,ulong ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }");

      name="ptr add 1s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,sbyte ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 1u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,byte  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 2s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,short ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 2u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,ushort) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 4s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,int   ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 4u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,uint  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 8s"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,long  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
      name="ptr add 8u"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,ulong ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");

      name="ptr diff"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,uintptr) = (PARAM(1,byte*) - PARAM(2,byte*)) / call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }");
   }

   // create goto's
   {
      Str name; // use 'getInstructP' because goto's are always stored in INSTRUCT
      name="goto"         ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) {                     thread.func_code_pos = (byte*)call.param(0).getInstructP(); }");
      name="goto cond 1"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,byte )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond 2"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,short)) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond 4"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,int  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond 8"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,long )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond f"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,flt  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond d"  ; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if( PARAM(1,dbl  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n 1"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,byte )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n 2"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,short)) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n 4"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,int  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n 8"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,long )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n f"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,flt  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
      name="goto cond n d"; funcs.add(name); ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { if(!PARAM(1,dbl  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }");
   }

   ft.endLine();
   ft.putLine("#ifdef SUPPORT_SCRIPTS");
   ft.endLine();

   // create basic operators
      // binary change
      FREPAD(o, op_change)
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
      for(VAR_TYPE b=VAR_FROM; b<=VAR_TO; b=VAR_TYPE(b+1))
         if(!((op_change[o]=="%=" || op_change[o]=="<<=" || op_change[o]==">>=" || op_change[o]=="&=" || op_change[o]=="|=" || op_change[o]=="^=") && (RealType(a) || RealType(b)))) // disallow modulo and binary operators if at least one param is real
         if(SameSizeSignBoolType(a)==a && ((a==VAR_BOOL && op_change[o]=="=") ? SameSizeSignBoolType(b)==b : SameSizeSignType(b)==b)) // keep bool for left side because C++ forces the result to bool (for bool=bool keep bool on the right side as well)
      {
         Symbol::Modif result, pa, pb;
         result=TypeSymbol(VAR_VOID);
         pa    =TypeSymbol(a);
         pb    =TypeSymbol(b);
         Str name=NativeOperatorName(op_change[o], result, pa, pb); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*pa+") "+op_change[o]+" PARAM(1,"+*pb+"); thread.func_code_pos+=Call::Size(2); }"); // A OP B;
      }
      // binary compare
      FREPAD(o, op_compare)
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
      for(VAR_TYPE b=VAR_FROM; b<=VAR_TO; b=VAR_TYPE(b+1))
         if(op_compare[o]!="===" && op_compare[o]!="!!=" && op_compare[o]!="^^") // temporarily skip extra operators
         if((op_compare[o]=="&&" || op_compare[o]=="^^" || op_compare[o]=="||") ? (SameSizeType(a)==a && SameSizeType(b)==b) : (SameSizeSignType(a)==a && SameSizeSignType(b)==b))
      {
         Symbol::Modif result, pa, pb;
         result=TypeSymbol(VAR_BOOL);
         pa    =TypeSymbol(a);
         pb    =TypeSymbol(b);
         Str name=NativeOperatorName(op_compare[o], result, pa, pb); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*result+") = (PARAM(1,"+*pa+") "+op_compare[o]+" PARAM(2,"+*pb+")); thread.func_code_pos+=Call::Size(3); }"); // RESULT = (A OP B);
      }
      // binary
      FREPAD(o, op_binary)
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
      for(VAR_TYPE b=VAR_FROM; b<=VAR_TO; b=VAR_TYPE(b+1))
      {
         VAR_TYPE type=VAR_NONE, ta=SameSizeSignType(a), tb=SameSizeSignType(b);
         if(ta==a && tb==b)
         {
            if(op_binary[o]=="*"                                          )type=       MulResult(ta, tb);else
            if(op_binary[o]=="/"                                          )type=       DivResult(ta, tb);else
            if(op_binary[o]=="%"                                          )type=       ModResult(ta, tb);else
            if(op_binary[o]=="+"                                          )type=       AddResult(ta, tb);else
            if(op_binary[o]=="-"                                          )type=       SubResult(ta, tb);else
            if(op_binary[o]=="<<"                                         )type= ShiftLeftResult(ta, tb);else
            if(op_binary[o]==">>"                                         )type=ShiftRightResult(ta, tb);else
            if(op_binary[o]=="&" || op_binary[o]=="^" || op_binary[o]=="|")type=       BitResult(ta, tb);
            if(type)
            {
               Symbol::Modif result, pa, pb;
               result=TypeSymbol(type);
               pa    =TypeSymbol(ta);
               pb    =TypeSymbol(tb);
               Str name=NativeOperatorName(op_binary[o], result, pa, pb); funcs.add(name);
               ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*result+") = (PARAM(1,"+*pa+") "+op_binary[o]+" PARAM(2,"+*pb+")); thread.func_code_pos+=Call::Size(3); }"); // RESULT = (A OP B);
            }
         }
      }
      // unary !
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
         if(SameSizeType(a)==a)
      {
         Symbol::Modif result, pa;
         result=TypeSymbol(VAR_BOOL);
         pa    =TypeSymbol(a);
         Str name=NativeOperatorName("!", result, pa); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*result+") = !PARAM(1,"+*pa+"); thread.func_code_pos+=Call::Size(2); }"); // RESULT = !A;
      }
      // unary ~
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))if(!RealType(a))
         if(SameSizeSignType(a)==a)
      {
         Symbol::Modif result, pa;
         result=TypeSymbol(ComplementResult(a));
         pa    =TypeSymbol(a);
         Str name=NativeOperatorName("~", result, pa); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*result+") = ~PARAM(1,"+*pa+"); thread.func_code_pos+=Call::Size(2); }"); // RESULT = ~A;
      }
      // unary -
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
         if(SameSizeSignType(a)==a)
      {
         Symbol::Modif result, pa;
         result=TypeSymbol(NegativeResult(a));
         pa    =TypeSymbol(a);
         Str name=NativeOperatorName("-", result, pa); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { PARAM(0,"+*result+") = -PARAM(1,"+*pa+"); thread.func_code_pos+=Call::Size(2); }"); // RESULT = -A;
      }
      // unary ++ --
      FREPAD(o, op_inc_dec)
      for(VAR_TYPE a=VAR_FROM; a<=VAR_TO; a=VAR_TYPE(a+1))
         if(!(a==VAR_BOOL && op_inc_dec[o]=="--")) // C++ doesn't allow "--bool"
         if(SameSizeType(a)==a)
      {
         Symbol::Modif result, pa;
         result=TypeSymbol(VAR_VOID);
         pa    =TypeSymbol(a);
         Str name=NativeOperatorName(op_inc_dec[o], result, pa); funcs.add(name);
         ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { "+op_inc_dec[o]+"PARAM(0,"+*pa+"); thread.func_code_pos+=Call::Size(1); }"); // ++A;
      }

   // create EE functions (as first, so binarySearch can be used without sorting native operators, and faster because of fewer functions at this stage)
   // TODO: process virtuals twice (as virtual and as not)
   FREPA(Symbols)
   {
      Symbol &func=Symbols.lockedData(i);
      if(func.isFunc() // functions
      && !(func.modifiers&Symbol::MODIF_FUNC_PARAM) // which are not function parameters "Mems& Mems::sort(Int Compare(C TYPE &a, C TYPE &b));"
      && !func.isTemplateFunc() // skip templates
      && func.fullyPublic() // this and all parents are public, skip "class Parent { private class Child { void func(); } }"
      && func.source && func.source->ee_header) // only stuff from EE headers
      {
// TODO: skip functions which accept pointers to functions as parameters
Bool has_func_param=false; REPA(func.params)if(func.params[i]->type==Symbol::FUNC){has_func_param=true; break;} if(has_func_param)continue;

         Str short_name=func.fullCppName();
         if(Equal   (short_name, "InitPre", true) || Equal(short_name, "Init", true) || Equal(short_name, "Shut", true) || Equal(short_name, "Update", true) || Equal(short_name, "Draw", true) // skip user provided functions
         || Equal   (short_name, "SupportScripts", true) // skip enabling scripts since it's already a script
         || Contains(short_name, "PVRTC"         , true))continue; // skip anything PVRTC related so that the EXE is not bigger due to PVRTC compression libraries
         Int index;
         Str name=FuncName(func);
         if(!funcs.binarySearch(name, index, CompareCS))
         {
            Symbol::Modif &result   =func.value, result_final=result; result_final.proceedToFinal(null, true);
            Symbol        *Class    =func.Class();
            Str            func_name=func.cppName(), op_name;
            Bool has_result  =func.hasResult(),
                 has_this    =func.isClassNonStaticFunc(),
                  is_virtual =func.isVirtualFunc(),
                  is_operator=Starts(func_name, "operator", true, true); if(is_operator)op_name=SkipWhiteChars(SkipStart(func_name, "operator"));
            if(has_result  && !result
            || has_this    && !Class
            || is_operator && (op_name=="new" || op_name=="delete")
            || (func.modifiers&Symbol::MODIF_CTOR) && Class->isAbstractClass() // can't call constructor using placement new for abstract classes
            )continue; // quietly skip incorrect data

            funcs.NewAt(index)=name;
            Str body=S+"thread.func_code_pos+=Call::Size("+(has_result + has_this + func.realParams())+"); ";
            Int params=0;

            if(has_result)
               if(result.modifiers&Symbol::MODIF_REF)body+=S+     "PARAM_REF("+(params++)+                                                                  ") = &(";else                                                           //      PARAM_REF(0      )   = &(
               if(result_final.basicType()          )body+=S+     "PARAM("    +(params++)+", "+CppStyle(ModifName(result, false, true, false, false, true))+") = (" ;else                                                           //      PARAM    (0, type)   =  (
                                                     body+=S+"new(&PARAM("    +(params++)+", "+CppStyle(ModifName(result, false, true, false, false, true))+"))"+CppStyle(ModifName(result, false, false, false, false, true))+'('; // new(&PARAM    (0, type))Class(

            if(has_this)
            {
               if(func.modifiers&Symbol::MODIF_CTOR)body+="new(&";                  // new(&
               body+=S+"PARAM("+(params++)+", "+CppStyle(Class->fullCppName())+")"; // PARAM(1, type)
               if(func.modifiers&Symbol::MODIF_CTOR)body+=S+')'+CppStyle(ModifName(result, false, false, false, false, true));else // )Class
               if(func.modifiers&Symbol::MODIF_DTOR)body+=S+'.'+*Class+"::";else // ctors/dtors need to have class "XX::" added, sample: "Str::Str()"
                                                    body+=  '.';
            }

            if(!(func.modifiers&Symbol::MODIF_CTOR))
            {
               if(is_operator) // operator[], operator+, operator CChar*
               {
                  body+="operator ";
                  if(op_name=="cast")body+=CppStyle(ModifName(result, false, true, true));else body+=op_name;
               }else
               {
                  body+=(has_this ? func_name : CppStyle(func.fullCppName())); // func
               }
            }
            body+='(';
            FREPA(func.params)
            {
               if(i)body+=", ";
               if(InRange(i, func.realParams()))body+=S+"PARAM("+(params++)+", "+CppStyle(ModifName(func.params[i]->value, false, true, false, true, true))+')'; // skip references but keep const in case there are "func(const int x)" and "func(int x)"
               else                             body+='0'; // 0 for dummy operator++(int)
            }
            body+=')'; // func(|)
            if(has_result)body+=')'; // x = (|)
            body+=';';
            ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { "+body+" }");
//if(funcs.elms()>=128)break;
         }
      }
   }

   // setup default destructors
   FREPA(Symbols)
   {
      Symbol &Class=Symbols.lockedData(i);
      if( Class.type==Symbol::CLASS // classes
      && !Class.isTemplateClass() // skip templates
      && !(Class.modifiers&Symbol::MODIF_NAMELESS) // skip nameless classes
      &&  Class.fullyPublic() // this and all parents are public, skip "class Parent { private class Child { void func(); } }"
      &&  Class.source && Class.source->ee_header) // only stuff from EE headers
      {
         if(Class.hasDestructor() && !FindChild(S+'~'+Class, &Class, null, false)) // should have destructor but doesn't
         {
            Str name=DtorName(Class);
            if(funcs.binaryInclude(name, CompareCS))
            {
               Str body ="thread.func_code_pos+=Call::Size(1); ";
                   body+=S+"PARAM(0, "+CppStyle(Class.fullCppName())+").";
                   body+=Class+"::~"+Class+"();";
               ft.putLine(S+"static void "+SymbolToCppName(name)+"(PARAMS) { "+body+" }");
            }
         }
      }
   }
   ft.endLine();

   // put list of functions
   funcs.sort(CompareCS); // compare before writing to enable binary search later

   ft.putLine("#ifdef _MSC_VER");
   ft.putLine("   #pragma optimize(\"\", off) // because this file compiles so slow!");
   ft.putLine("#endif");
   ft.endLine();

#if 0
   ft.putLine("static FuncCall FC[]=");
   ft.putLine("{"); ft.depth++;
   FREPA(funcs)
   {
      Bool shader_compile=(Contains(funcs[i], "EE.ShaderCompile", true, true) || Contains(funcs[i], "EE.ShaderCompileTry", true, true));
      if(shader_compile){ft.depth--; ft.putLine("#if !MAC // disable this on Mac, because compiling shaders on Mac requires CG framework installed and thus all EE based applications would not run without CG"); ft.depth++;}
      ft.putLine(S+"{\""+funcs[i]+"\", "+SymbolToCppName(funcs[i])+"},");
      if(shader_compile){ft.depth--; ft.putLine("#endif"); ft.depth++;}
   }
   ft.depth--; ft.putLine("};");
   ft.putLine("static Int ElmsFC=Elms(FC);");
   ft.putLine("static void InitFC() {}");
#else
   ft.putLine(S+"static FuncCall FC["+funcs.elms()+"];");
   ft.putLine("static Int ElmsFC;");
   ft.putLine("static void InitFC()");
   ft.putLine("{"); ft.depth++;
   ft.putLine("if(ElmsFC)return;");
   #if 1 // makes .exe files smallest
      ft.putLine("static FuncCall FC2[]=");
      ft.putLine("{"); ft.depth++;
      FREPA(funcs)
      {
         Bool shader_compile=(Contains(funcs[i], "EE.ShaderCompile", true, true) || Contains(funcs[i], "EE.ShaderCompileTry", true, true));
         if(shader_compile){ft.depth--; ft.putLine("#if !MAC // disable this on Mac, because compiling shaders on Mac requires CG framework installed and thus all EE based applications would not run without CG"); ft.depth++;}
         ft.putLine(S+"{\""+funcs[i]+"\", "+SymbolToCppName(funcs[i])+"},");
         if(shader_compile){ft.depth--; ft.putLine("#endif"); ft.depth++;}
      }
      ft.depth--; ft.putLine("};");
      ft.putLine("CopyN(FC, FC2, ElmsFC=Elms(FC2));");
   #else
      ft.putLine("int i=0;");
      FREPA(funcs)
      {
         Bool shader_compile=(Contains(funcs[i], "EE.ShaderCompile", true, true) || Contains(funcs[i], "EE.ShaderCompileTry", true, true));
         if(shader_compile){ft.depth--; ft.putLine("#if !MAC // disable this on Mac, because compiling shaders on Mac requires CG framework installed and thus all EE based applications would not run without CG"); ft.depth++;}
         ft.putLine(S+"FC[i].name=\""+funcs[i]+"\"; FC[i].func="+SymbolToCppName(funcs[i])+"; i++;");
         if(shader_compile){ft.depth--; ft.putLine("#endif"); ft.depth++;}
      }
      ft.putLine("ElmsFC=i;");
   #endif
   ft.depth--; ft.putLine("}");
#endif

   // put list of variables
   FREPA(Symbols)
   {
      Symbol &var=Symbols.lockedData(i);
      if(var.isVar() // variables
      && var.isGlobalOrStatic()
      && var.fullyPublic()
      && var.source && var.source->ee_header) // only stuff from EE headers
      {
         Str short_name=var.fullCppName();
         if(Contains(short_name, "PVRTC", true))continue; // skip anything PVRTC related so that the EXE is not bigger due to PVRTC compression libraries
         vars.add(short_name);
      }
   }

   vars.sort(CompareCS); // compare before writing to enable binary search later
   ft.endLine();
   ft.putLine("static GlobalVar GV[]=");
   ft.putLine("{"); ft.depth++;
   FREPA(vars)ft.putLine(S+"{\""+vars[i]+"\", &"+CppStyle(vars[i])+"},");
   ft.depth--; ft.putLine("};");

   ft.endLine(); ft.depth++;
   ft.putLine("#ifdef _MSC_VER");
   ft.putLine("   #pragma optimize(\"\", on) // because this file compiles so slow!");
   ft.putLine("#endif"); ft.depth--;
   ft.putLine("#endif // SUPPORT_SCRIPTS");

   if(!OverwriteOnChange(ft, "C:\\Projects\\EsenthelEngine\\Src\\Code\\Compile\\CE Func List.h"))Exit("Can't create FuncList");
}
/******************************************************************************/
// FUNCTION LIST
/******************************************************************************/
void Call::Param::set(Expr &expr, Compiler &compiler, Bool auto_map)
{
   switch(expr.mem.type)
   {
      case Expr::Memory::GLOBAL:
      {
         if(expr.mem.index>=0)                                                         {setHeap  (expr.mem.index+expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF)); return;} // set successfully
         if(expr.mem.symbol)if(CPtr addr=FindGlobalVar(expr.mem.symbol->fullCppName())){setGlobal((Byte*)addr   +expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF)); return;} // set successfully
      }break;

      case Expr::Memory::LOCAL:
      {
         if(InRange(expr.mem.index, compiler.locals))
         {
            Local &local=compiler.locals[expr.mem.index];
            if(local.stack_offset< 0 && auto_map)compiler.mapLocalInStack(expr.mem); // auto map only if allowed
            if(local.stack_offset>=0)
            {
               if(local.force_heap)setHeap (local.stack_offset+expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF));
               else                setStack(local.stack_offset+expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF));
               return; // set successfully
            }
         }
      }break;

      case Expr::Memory::PARAM:
      {
         if(expr.mem.index>=0){setParamStack(expr.mem.index+expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF)); return;} // set successfully
      }break;

      case Expr::Memory::THIS:
      {
         setThis(expr.mem.offset, expr.mem.offset2, FlagTest(expr.symbol.modifiers, Symbol::MODIF_REF)); return; // set successfully
      }break;

      case Expr::Memory::RESULT:
      {
         if(expr.mem.offset==0 && expr.mem.offset2==0){setResult(); return;} // set successfully
      }break;

      case Expr::Memory::KNOWN:
      {
         if(expr.mem.offset==0 && expr.mem.offset2==0)
         {
            Int size=expr.symbol.rawSize(false);
            U64 raw =expr.asRaw(compiler);
            if(ValueFitsInInstruction(size) || expr.symbol.isPtr() && !raw)setInstruct(raw);else setHeap(compiler.heapConstant(&raw, size, expr.symbol.firstMemberSize()), 0, false);
            return;
         }
      }break;
      
      case Expr::Memory::NONE:
      {
         if(expr.instance && expr.symbol.isArray() && expr.symbol && (expr.symbol->var_type==VAR_CHAR8 || expr.symbol->var_type==VAR_CHAR16)) // "text"
            if(expr[0]=='"' || (expr[1]=='"' && (expr[0]=='8' || expr[0]=='u' || expr[0]=='U' || expr[0]=='L'))) // "", 8"", u"", U"", L""
         {
            Int        heap;
            Memc<Char> text=expr.asText(compiler);
            if(expr.symbol->var_type==VAR_CHAR16)heap=compiler.heapConstant(text.data(), text.elms()*SIZE(Char), 2);else
            {
               Mems<Char8> text8; text8.setNum(text.elms()); FREPAO(text8)=Char16To8Fast(text[i]); // convert Str to Str8, we can assume that Str was already initialized
               heap=compiler.heapConstant(text8.data(), text8.elms(), 1);
            }
            setHeap(heap, 0, false);
            return;
         }
      }break;
   }
   compiler.msgs.New().error("Unknown memory address of expression", expr.origin);
}
void Call::Param::setRef(Expr &expr, Compiler &compiler)
{
   set(expr, compiler);
   switch(type)
   {
      default             : return; // don't check after switch
      case ADDR_GLOBAL_REF: type=ADDR_GLOBAL; break;
      case ADDR_HEAP_REF  : type=ADDR_HEAP  ; break;
      case ADDR_STACK_REF : type=ADDR_STACK ; break;
      case ADDR_THIS_REF  : type=ADDR_THIS  ; break;
   }
   if(offset2!=0)compiler.msgs.New().error("Invalid reference address", expr.origin);
}
void Call::Param::setGlobal     (Ptr addr  , Int offset2, Bool ref) {type=(ref ? ADDR_GLOBAL_REF      : ADDR_GLOBAL     ); T.addr  =addr  ; T.offset2=offset2;}
void Call::Param::setHeap       (Int offset, Int offset2, Bool ref) {type=(ref ? ADDR_HEAP_REF        : ADDR_HEAP       ); T.offset=offset; T.offset2=offset2;}
void Call::Param::setStack      (Int offset, Int offset2, Bool ref) {type=(ref ? ADDR_STACK_REF       : ADDR_STACK      ); T.offset=offset; T.offset2=offset2;}
void Call::Param::setParamStack (Int offset, Int offset2, Bool ref) {type=(ref ? ADDR_PARAM_STACK_REF : ADDR_PARAM_STACK); T.offset=offset; T.offset2=offset2;}
void Call::Param::setThis       (Int offset, Int offset2, Bool ref) {type=(ref ? ADDR_THIS_REF        : ADDR_THIS       ); T.offset=offset; T.offset2=offset2;}
void Call::Param::setResult     (                                 ) {type=       ADDR_RESULT                             ; T.offset=     0; T.offset2=      0;}
void Call::Param::setInstruct   (U64 raw                          ) {type=       ADDR_INSTRUCT                           ; T.raw   =raw   ;                   }
void Call::Param::setInstructI  (Int raw                          ) {type=       ADDR_INSTRUCT                           ; T.offset=raw   ; T.offset2=      0;}
void Call::Param::setInstructP  (Ptr addr                         ) {type=       ADDR_INSTRUCT                           ; T.addr  =addr  ; T.offset2=      0;}
void Call::Param::setInstructPI2(Ptr addr  , Int i                ) {type=       ADDR_INSTRUCT                           ; T.addr  =addr  ; T.offset2=      i;}

#if WINDOWS
   #pragma warning(push)
   #pragma warning(disable:4715) // not all control paths return a value
#endif
#if 0
Ptr Call::Param::address(CodeEnvironment::Thread &thread)
{
   switch(type)
   {
      case ADDR_STACK          : return           (thread.func_stack      +offset);
      case ADDR_STACK_REF      : return (*(Byte**)(thread.func_stack      +offset)) + offset2;
      case ADDR_PARAM_STACK    : return           (thread.func_param_stack+offset);
      case ADDR_PARAM_STACK_REF: return (*(Byte**)(thread.func_param_stack+offset)) + offset2;
      case ADDR_THIS           : return           (thread.func_this       +offset);
      case ADDR_THIS_REF       : return (*(Byte**)(thread.func_this       +offset)) + offset2;
      case ADDR_RESULT         : return           (thread.func_result            );
      case ADDR_HEAP           : return           (thread.heap            +offset);
      case ADDR_HEAP_REF       : return (*(Byte**)(thread.heap            +offset)) + offset2;
      case ADDR_GLOBAL         : return           (                          addr);
      case ADDR_GLOBAL_REF     : return (*(Byte**)(                          addr)) + offset2;
      case ADDR_INSTRUCT       : return          &(                           raw);
   }
}
#else
// TODO: make all _REF with _REF_OFS and only there apply 'offset2'?
T1(TYPE) TYPE& Call::Param::value(CodeEnvironment::Thread &thread)
{
   switch(type)
   {
      case ADDR_STACK          : return     *(TYPE*)           (thread.func_stack      +offset);
      case ADDR_STACK_REF      : return     *(TYPE*)((*(Byte**)(thread.func_stack      +offset)) + offset2);
      case ADDR_PARAM_STACK    : return     *(TYPE*)           (thread.func_param_stack+offset);
      case ADDR_PARAM_STACK_REF: return     *(TYPE*)((*(Byte**)(thread.func_param_stack+offset)) + offset2);
      case ADDR_THIS           : return     *(TYPE*)           (thread.func_this       +offset);
      case ADDR_THIS_REF       : return     *(TYPE*)((*(Byte**)(thread.func_this       +offset)) + offset2);
      case ADDR_RESULT         : return     *(TYPE*)           (thread.func_result            );
      case ADDR_HEAP           : return     *(TYPE*)           (thread.heap            +offset);
      case ADDR_HEAP_REF       : return     *(TYPE*)((*(Byte**)(thread.heap            +offset)) + offset2);
      case ADDR_GLOBAL         : return     *(TYPE*)           (                          addr);
      case ADDR_GLOBAL_REF     : return     *(TYPE*)((*(Byte**)(                          addr)) + offset2);
      case ADDR_INSTRUCT       : return reinterpret_cast<TYPE&>(                           raw); // use 'reinterpret_cast' instead of (TYPE&) because 'TYPE' can be const and "(const TYPE&)(raw)" could create new temporary var
   }
}
INLINE Ptr Call::Param::address(CodeEnvironment::Thread &thread) {return &value<Byte>(thread);} // use Byte because void can't be used
#endif
CPtr& Call::Param::ref(CodeEnvironment::Thread &thread)
{
   switch(type)
   {
      case ADDR_STACK_REF      : return *(CPtr*)(thread.func_stack      +offset);
      case ADDR_PARAM_STACK_REF: return *(CPtr*)(thread.func_param_stack+offset);
      case ADDR_THIS_REF       : return *(CPtr*)(thread.func_this       +offset);
      case ADDR_HEAP_REF       : return *(CPtr*)(thread.heap            +offset);
      case ADDR_GLOBAL_REF     : return *(CPtr*)(                          addr);
   }
   return *(CPtr*)null; // on purpose return null based pointer so it will trigger exception
}
#if WINDOWS
   #pragma warning(pop)
#endif
/******************************************************************************/
#define PARAMS         Call &call, CodeEnvironment::Thread &thread
#define PARAM(i, type) call.param(i).value<type>(thread)
#define PARAM_REF(i  ) call.param(i).ref        (thread)

#define bool Bool // boolean value (8-bit)

#define char8 Char8 //  8-bit character
#define char  Char  // 16-bit character

#define sbyte  I8  //  8-bit   signed integer
#define  byte  U8  //  8-bit unsigned integer
#define  short I16 // 16-bit   signed integer
#define ushort U16 // 16-bit unsigned integer
#define  int   I32 // 32-bit   signed integer
#define uint   U32 // 32-bit unsigned integer
#define  long  I64 // 64-bit   signed integer
#define ulong  U64 // 64-bit unsigned integer

#define flt Flt // 32-bit floating point
#define dbl Dbl // 64-bit floating point

#define  ptr  Ptr // universal pointer
#define cptr CPtr // universal pointer to const data

#define cchar8 CChar8 // const Char8
#define cchar  CChar  // const Char16

#define  intptr  IntPtr //   signed integer capable of storing full memory address
#define uintptr UIntPtr // unsigned integer capable of storing full memory address

#if WINDOWS
   #pragma warning(push)
   #pragma warning(disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
   #pragma warning(disable:4804) // unsafe use of type 'bool' in operation
   #pragma warning(disable:4805) // unsafe mix of type A and type B in operation
   #pragma warning(disable:4146) // unary minus operator applied to unsigned type, result still unsigned
#endif

#include "CE Func List.h"

#if WINDOWS
   #pragma warning(pop)
#endif

#undef bool

#undef char8
#undef char

#undef sbyte
#undef  byte
#undef  short
#undef ushort
#undef  int
#undef uint
#undef  long
#undef ulong

#undef flt
#undef dbl

#undef  ptr
#undef cptr

#undef cchar8
#undef cchar

#undef  intptr
#undef uintptr

#undef PARAMS
#undef PARAM
#undef PARAM_REF
/******************************************************************************/
static Int CompareFC(C FuncCall   &fc, C Str &func_name  ) {return CompareCS(fc.name, func_name);}
static Int CompareGV(C GlobalVar  &gv, C Str & var_name  ) {return CompareCS(gv.name,  var_name);}
static Int CompareFC(  FuncCall*C &a , FuncCall*C &b     ) {if(a->func < b->func)return -1; if(a->func > b->func)return +1; return 0;}
static Int CompareFC(  FuncCall*C &a , C Call::Func &func) {if(a->func <    func)return -1; if(a->func >    func)return +1; return 0;}

#ifdef SUPPORT_SCRIPTS
CChar8* FindFuncCall(Call::Func func)
{
   if(!FC2.elms()){FC2.setNum(ElmsFC); FREPAO(FC2)=&FC[i]; FC2.sort(CompareFC);} // create array of functions sorted by their 'func' address
   Int index; return FC2.binarySearch(func, index, CompareFC) ? FC2[index]->name : null;
}
Call::Func FindFuncCall(C Str &func_name)
{
   Int index; return BinarySearch(FC, ElmsFC, func_name, index, CompareFC) ? FC[index].func : null;
}
CPtr FindGlobalVar(C Str &var_name)
{
   Int index; return BinarySearch(GV, Elms(GV), var_name, index, CompareGV) ? GV[index].addr : null;
}
#else
CChar8*    FindFuncCall (Call::Func  func) {return null;}
Call::Func FindFuncCall (C Str &func_name) {return null;}
CPtr       FindGlobalVar(C Str & var_name) {return null;}
#endif
Call::Func GetFuncCall(C Str &func_name)
{
   Call::Func func=FindFuncCall(func_name); if(!func)Exit(S+"Can't find \""+func_name+"\" function.");
   return     func;
}

Call::Func GetIgnoreCall() {return $nop;}

Call::Func GetCallFunc(Bool has_this, Bool has_result)
{
   if(has_this)return has_result ? $call$20method$20result : $call$20method;
               return has_result ? $call$20func$20result   : $call$20func;
}

Call::Func GetIndirectionCall() {return $indirection ;}
Call::Func GetAddressOfCall  () {return $address$20of;}
Call::Func GetOffsetCall     (Symbol::Modif &offset)
{
   if(offset)switch(offset.rawSize(false))
   {
      case 1: return SignedType(offset->var_type) ? $offset$201s : $offset$201u;
      case 2: return SignedType(offset->var_type) ? $offset$202s : $offset$202u;
      case 4: return SignedType(offset->var_type) ? $offset$204s : $offset$204u;
      case 8: return SignedType(offset->var_type) ? $offset$208s : $offset$208u;
   }
   return null;
}
Call::Func GetAddPtrCall(Symbol::Modif &offset)
{
   if(offset)switch(offset.rawSize(false))
   {
      case 1: return SignedType(offset->var_type) ? $add$20ptr$201s : $add$20ptr$201u;
      case 2: return SignedType(offset->var_type) ? $add$20ptr$202s : $add$20ptr$202u;
      case 4: return SignedType(offset->var_type) ? $add$20ptr$204s : $add$20ptr$204u;
      case 8: return SignedType(offset->var_type) ? $add$20ptr$208s : $add$20ptr$208u;
   }
   return null;
}
Call::Func GetPtrAddCall(Symbol::Modif &offset)
{
   if(offset)switch(offset.rawSize(false))
   {
      case 1: return SignedType(offset->var_type) ? $ptr$20add$201s : $ptr$20add$201u;
      case 2: return SignedType(offset->var_type) ? $ptr$20add$202s : $ptr$20add$202u;
      case 4: return SignedType(offset->var_type) ? $ptr$20add$204s : $ptr$20add$204u;
      case 8: return SignedType(offset->var_type) ? $ptr$20add$208s : $ptr$20add$208u;
   }
   return null;
}
Call::Func GetPtrDiffCall() {return $ptr$20diff;}

Call::Func GetGotoCall() {return $goto;}

Call::Func GetGotoCondCall(Symbol::Modif &cond)
{
   if(cond.isPtr())
   {
      if(PtrSize==4)return $goto$20cond$204;
      if(PtrSize==8)return $goto$20cond$208;
   }else
   if(cond.isObj() && cond)switch(cond->var_type)
   {
      case VAR_CHAR8 :
      case VAR_BOOL  :
      case VAR_BYTE  :
      case VAR_SBYTE : return $goto$20cond$201;

      case VAR_CHAR16:
      case VAR_SHORT :
      case VAR_USHORT: return $goto$20cond$202;

      case VAR_ENUM  :
      case VAR_INT   :
      case VAR_UINT  : return $goto$20cond$204;

      case VAR_LONG  :
      case VAR_ULONG : return $goto$20cond$208;

      case VAR_FLT   : return $goto$20cond$20f;

      case VAR_DBL   : return $goto$20cond$20d;
   }
   return null;
}
Call::Func GetGotoCondNCall(Symbol::Modif &cond)
{
   if(cond.isPtr())
   {
      if(PtrSize==4)return $goto$20cond$20n$204;
      if(PtrSize==8)return $goto$20cond$20n$208;
   }else
   if(cond.isObj() && cond)switch(cond->var_type)
   {
      case VAR_CHAR8 :
      case VAR_BOOL  :
      case VAR_BYTE  :
      case VAR_SBYTE : return $goto$20cond$20n$201;

      case VAR_CHAR16:
      case VAR_SHORT :
      case VAR_USHORT: return $goto$20cond$20n$202;

      case VAR_ENUM  :
      case VAR_INT   :
      case VAR_UINT  : return $goto$20cond$20n$204;

      case VAR_LONG  :
      case VAR_ULONG : return $goto$20cond$20n$208;

      case VAR_FLT   : return $goto$20cond$20n$20f;

      case VAR_DBL   : return $goto$20cond$20n$20d;
   }
   return null;
}
Call::Func GetSetConstCall(Int size, Bool heap)
{
   switch(size)
   {
      case  1: return heap ? $set$20const$20heap$201 : $set$20const$201;
      case  2: return heap ? $set$20const$20heap$202 : $set$20const$202;
      case  4: return heap ? $set$20const$20heap$204 : $set$20const$204;
      case  8: return heap ? $set$20const$20heap$208 : $set$20const$208;
      default: return null;
   }
}
Call::Func GetSetAddrHeapCall  () {return $set$20addr$20heap  ;}
Call::Func GetSetAddrStackCall () {return $set$20addr$20stack ;}
Call::Func GetSetAddrResultCall() {return $set$20addr$20result;}
/******************************************************************************/
} // namespace Edit
/******************************************************************************/
void SupportScripts()
{
#ifdef SUPPORT_SCRIPTS
   Edit::InitFC();
#endif
}
Bool ScriptsSupported()
{
#ifdef SUPPORT_SCRIPTS
   return Edit::ElmsFC>0;
#else
   return false;
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
