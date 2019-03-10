/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Esenthel Script functions use following memory diagram: (engine codes which rely on this diagram have "RESULT PARENT PARAMS" near in the comments, use the string to search the engine)

      Stack:
         0 - Memory for local variables and temporaries

      Param Stack: (all parameters must be stored continuously)
         0 - Continuous memory for function parameters

      Nvidia Tegra CPU (Android) requires that FPU operations (on Flt and Dbl) use memories aligned to 4 bytes (non-float types alignment is not required)
      Performing FPU operation on unaligned memory results in instant crash.

/******************************************************************************

   Each "map local in stack" should check if that variable requires calling destructor.

   That destructor should be called in one of following scenario:
      -1 instruction after usage of the variable (only for temporaries), however remember that in C++ they can be held longer "C Str &var=Str("123")" (until end of 'var' life)
      -during goto which relocates code position outside of variable scope
      -at the end of variable scope

   Example:

      {
      start:
         C Str &s=Str("123"); // Str("123") is held by reference so make its life as long until next '}'
         goto start; // here call dtor for Str("123") since it jumps outside the scope of the var

         Str("abc");
         // here call dtor for Str("abc") since it's a temporary not held by any reference

         C Str &s2=(cond ? Str("def") : s); // Str("def") is created conditionally, so upon its creation add it to 'destructor_list' (with scope of 1 block or 1 instruction depending if value is set into reference)

         goto end; // here call dtor for Str("123"), since there was one conditional with block scope then check for 'destructor_list'

      } // here call dtor for Str("123"), since there was one conditional with block scope then check for 'destructor_list'
   end:

   There can be following types of destructors:
      Bool block_scope; (false for instruction scope)
      Bool conditional; (false for permanent)

   Goto's in some cases can't jump from outside the scope of a variable to inside the scope of a variable (don't do any special checks for references as they are treated the same as non-references)
      C++ behavior for:
         {
            TYPE var;
         label:
         }
         goto label;

      TYPE = basic type or pointer (initialization means "var=..")
         initialization | compile result
               0              success
               1              success

      TYPE = class object/array (initialization means if class has anykind of ctor, even empty or default)
         initialization | has dtor | compile result
               0             0           success
               1             0           warning
               0             1           success
               1             1            error

/******************************************************************************/
Int Compare(C Edit::Message &a, C Edit::Message &b)
{
   if(a.source && b.source)
   {
      if(a.source!=b.source)if(Int c=ComparePath(a.source->loc.asText(), b.source->loc.asText()))return c;

      if(a.token && b.token && a.token->line && b.token->line)
      {
         return Compare(a.token->lineIndex(), b.token->lineIndex());
      }
      return Compare(a.token!=null, b.token!=null);
   }
   return Compare(a.source!=null, b.source!=null);
}
/******************************************************************************/
namespace Edit{
/******************************************************************************/
static Int CompareConstantData(Byte *a, Int size_a, Byte *b, Int size_b)
{
   REP(Min(size_a, size_b))if(Int c=Compare(*a++, *b++))return c;
   return Compare(size_a, size_b);
}
Int CompilerContext::heapConstant(CPtr data, Int size, Int align)
{
   if(data && size>0)
   {
      Int l=0, r=constants.elms(); for(; l<r; )
      {
         Int mid    =UInt(l+r)/2; Constant &c=constants[mid];
         Int compare=CompareConstantData(const_data.data()+c.const_offset, c.size, (Byte*)data, size);
         if( compare<0)l=mid+1;else
         if( compare>0)r=mid  ;else return c.heap_offset; // found
      }
      Constant &c=constants.NewAt(l);
      c.size=size;
      c.heap_offset=AlignAddress(env.heap_size, align);
      env.heap_size=c.heap_offset+size;
      c.const_offset=const_data.addNum(size);
      Copy(const_data.data()+c.const_offset, data, size);
      return c.heap_offset;
   }
   return 0;
}
void CompilerContext::varHeapConstant(Int heap_offset, CPtr data, Int size)
{
   if(data && size>0)
   {
      Constant &c=var_constants.New();
      c.size=size;
      c. heap_offset=heap_offset;
      c.const_offset=const_data.addNum(size);
      Copy(const_data.data()+c.const_offset, data, size);
   }
}
Int  Compiler::   heapConstant(                 CPtr data, Int size, Int align) {if(!ctx)msgs.New().error("CompilerContext not set", source, -1); return ctx ? ctx->   heapConstant(             data, size, align) : 0;}
void Compiler::varHeapConstant(Int heap_offset, CPtr data, Int size           ) {if(!ctx)msgs.New().error("CompilerContext not set", source, -1);     if(ctx)  ctx->varHeapConstant(heap_offset, data, size       )    ;}

Int Compiler::newLocal(Symbol::Modif &type, Bool block_scope, Token *token)
{
   Int    index=locals.elms(); // get index of newly created element
   Local &local=locals.New (); // create the new local
   local.type       =type       ; // set its type
   local.block_scope=block_scope; // set its scope
   local.token      =token      ; // set its token
   return index; // return index of the local variable
}
void Compiler::mapLocalInStack(Expr::Memory &mem, Bool error_on_fail)
{
   if(mem.type!=Expr::Memory::LOCAL)msgs.New().error("Can't map variable in the stack because it's not a local variable");else
   if(!InRange(mem.index, locals)  )msgs.New().error("Specified local variable index is invalid");else
   if(!ctx                         )msgs.New().error("null compiler context");else
   {
      Local &local=locals[mem.index];
      if(local.stack_offset>=0)
      {
         if(error_on_fail)msgs.New().error("Can't map local variable to stack because it's already mapped");
      }else
      {
         Int size =local.type.rawSize(true),
             align=local.type.firstMemberSize();
         if(local.force_heap) // if temporary is forced to exist on the heap
         {
            local.stack_offset=AlignAddress(ctx->env.heap_size, align); // ALIGN
            ctx->env.heap_size=local.stack_offset+size; // adjust new heap size
         }else
         {
            local.stack_offset=AlignAddress(stack_size, align); // ALIGN
            stack_size        =local.stack_offset+size; // adjust new stack size
         }
         if(local.type.hasDestructor())live_locals.add(mem.index); // add to just mapped container
         local.scope_start=cmd_index; // set creation position to the currently compiled command
         local.scope_label=scope_label; // set scope label index at which variable should be destroyed
      }
   }
}
void Compiler::mapVar(Expr::Memory &mem)
{
   if(mem.type==Expr::Memory::LOCAL)mapLocalInStack(mem, false);
}
Bool Compiler::unmappedLocal(Expr::Memory &mem)
{
   if(mem.type==Expr::Memory::LOCAL && InRange(mem.index, locals))return locals[mem.index].stack_offset<0;
   return false;
}
/******************************************************************************/
void Compiler::expand(Memc<Command> &src, Memc<Command> &dest, Int label_break, Int label_continue, Int label_return, Int label_scope)
{
   FREPA(src)
   {
      Command &cmd=src[i]; cmd.scope_label=label_scope;
      switch(  cmd.type)
      {
         default: dest.New()=cmd; break;

         case CMD_RETURN  : {Command &d=dest.New(); d=cmd; d.label_index=label_return  ;} break;
         case CMD_BREAK   : {Command &d=dest.New(); d=cmd; d.label_index=label_break   ;} break;
         case CMD_CONTINUE: {Command &d=dest.New(); d=cmd; d.label_index=label_continue;} break;

         case CMD_GOTO :
         case CMD_LABEL: {Command &d=dest.New(); d=cmd; if(InRange(cmd.raw_range.x, tokens)){Token &token=*tokens[cmd.raw_range.x]; if(token.symbol && token.symbol->type==Symbol::LABEL)d.label_index=token.symbol->raw_offset;}} break;

         case CMD_GROUP:
         {
          /*{
               cmds;
            }
            
            cmds; #0
         scope:   #1 - end of the group scope*/
         
            Int label_scope=labels++;
            expand(cmd.cmds, dest, label_break, label_continue, label_return, label_scope); // 0
            {Command &d=dest.New(); d.type=CMD_LABEL; d.label_index=label_scope;} // 1
         }break;

         case CMD_IF:
         {
          /*if(cond)true;else false;

            if(!cond)goto false; #0
            true;                #1
         true_scope:             #2
            goto end;            #3
         false:                  #4
            false;               #5
         false_scope:            #6
         end:                    #7*/

            Bool has_else         =(cmd.cmds_false.elms()!=0);
            Int  label_true_scope =labels++,
                 label_false      =labels++,
                 label_false_scope=labels++,
                 label_end        =labels++;
                        {Command &d=dest.New(); d.type=CMD_GOTO_COND_N; d._for.cond_range=cmd._for.cond_range; d.label_index=label_false; d.scope_label=label_end;} // 0
                        expand(cmd.cmds, dest, label_break, label_continue, label_return, label_true_scope); // 1
                        {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_true_scope ;} // 2
            if(has_else){Command &d=dest.New(); d.type=CMD_GOTO       ; d.label_index=label_end        ;} // 3
                        {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_false      ;} // 4
            if(has_else)expand(cmd.cmds_false, dest, label_break, label_continue, label_return, label_false_scope); // 5
            if(has_else){Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_false_scope;} // 6
                        {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_end        ;} // 7
         }break;

         case CMD_FOR:
         {
          /*for(init; cond; step)cmds;

            init;                                            #0 (this can be empty if 'init_range' is empty)
         loop:                                               #1
            if(!cond)goto break;                             #2 (this can be empty if 'cond_range' is empty, in that case always proceed to step 3)
            cmds;                                            #3
         cmds_scope:                                         #4 (this label destroys all vars from 'cmds' scope)
         continue: // any "continue;" command will jump here #5
            step;                                            #6 (this can be empty if 'step_range' is empty)
         step_scope:                                         #7
            goto loop;                                       #8
         break: // any "break;" command will jump here       #9*/

            Int label_loop      =labels++,
                label_cmds_scope=labels++,
                label_continue  =labels++,
                label_step_scope=labels++,
                label_break     =labels++;
            if(cmd._for.init_range.x <= cmd._for.init_range.y){Command &d=dest.New(); d.type=CMD_INSTRUCT   ; d.raw_range=cmd._for.init_range; d.scope_label=label_break;} // 0 (add step only if 'init_range' is specified)
                                                              {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_loop;} // 1
            if(cmd._for.cond_range.x <= cmd._for.cond_range.y){Command &d=dest.New(); d.type=CMD_GOTO_COND_N; d._for.cond_range=cmd._for.cond_range; d.label_index=label_break; d.scope_label=label_break;} // 2 (add step only if 'cond_range' is specified) (this should point to 'label_break' and not 'step_scope' because there is no destroying of block scoped variables in conditional goto)
                                                               expand(cmd.cmds, dest, label_break, label_continue, label_return, label_cmds_scope); // 3
                                                              {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_cmds_scope;} // 4
                                                              {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_continue;} // 5
            if(cmd._for.step_range.x <= cmd._for.step_range.y){Command &d=dest.New(); d.type=CMD_INSTRUCT   ; d.raw_range=cmd._for.step_range; d.scope_label=label_step_scope;} // 6 (add step only if 'step_range' is specified)
            if(cmd._for.step_range.x <= cmd._for.step_range.y){Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_step_scope;} // 7 (add step only if 'step_range' is specified)
                                                              {Command &d=dest.New(); d.type=CMD_GOTO       ; d.label_index=label_loop;} // 8
                                                              {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_break;} // 9
         }break;

         case CMD_WHILE:
         {
          /*while(cond)cmds;

         continue: // any "continue;" command will jump here #0
            if(!cond)goto break;                             #1
            cmds;                                            #2
         cmds_scope:                                         #3
            goto continue;                                   #4
         break: // any "break;" command will jump here       #5*/

            Int label_continue  =labels++,
                label_cmds_scope=labels++,
                label_break     =labels++;
            {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_continue;} // 0
            {Command &d=dest.New(); d.type=CMD_GOTO_COND_N; d._for.cond_range=cmd._for.cond_range; d.label_index=label_break; d.scope_label=label_break;} // 1
            expand(cmd.cmds, dest, label_break, label_continue, label_return, label_cmds_scope); // 2
            {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_cmds_scope;} // 3
            {Command &d=dest.New(); d.type=CMD_GOTO       ; d.label_index=label_continue;} // 4
            {Command &d=dest.New(); d.type=CMD_LABEL      ; d.label_index=label_break;} // 5
         }break;

         case CMD_DO:
         {
          /*do cmds while(cond);

         loop:                                               #0
            cmds;                                            #1
         cmds_scope:                                         #2
         continue: // any "continue;" command will jump here #3
            if(cond)goto loop;                               #4
         break: // any "break;" command will jump here       #5*/

            Int label_loop      =labels++,
                label_cmds_scope=labels++,
                label_continue  =labels++,
                label_break     =labels++;
            {Command &d=dest.New(); d.type=CMD_LABEL    ; d.label_index=label_loop;} // 0
            expand(cmd.cmds, dest, label_break, label_continue, label_return, label_cmds_scope); // 1
            {Command &d=dest.New(); d.type=CMD_LABEL    ; d.label_index=label_cmds_scope;} // 2
            {Command &d=dest.New(); d.type=CMD_LABEL    ; d.label_index=label_continue;} // 3
            {Command &d=dest.New(); d.type=CMD_GOTO_COND; d._for.cond_range=cmd._for.cond_range; d.label_index=label_loop; d.scope_label=label_break;} // 4
            {Command &d=dest.New(); d.type=CMD_LABEL    ; d.label_index=label_break;} // 5
         }break;

       /*case CMD_SWITCH:
         {
          /*switch(expr)
            {
               case 0: break;

               case 1:
               case 2: break;

               default: break;
            }

            if(expr==0)goto case 0;
            if(expr==1)goto case 1;
            if(expr==2)goto case 2;
            if(default_exists)goto default;else goto break;

         case 0:
            goto break;

         case 1:
         case 2:
            goto break;

         default:
            goto break;

         break;*/

         /*}break;*/
      }
   }
}
/******************************************************************************/
static void CompileSetConst(Expr &expr, Compiler &compiler, Memc<Byte> &code)
{
   Int size=expr.symbol.rawSize(false);
   if( size>0) // size can be 0 for empty classes
   {
      Int   params=2, // stack_offset + raw value
            pos   =code.addNum(Call::Size(params)),
            align =expr.symbol.firstMemberSize(),
            offset=AlignAddress(compiler.stack_size, align); compiler.stack_size=offset+size; // ALIGN
      U64   raw   =expr.asRaw(compiler);
      Bool  heap  =!(ValueFitsInInstruction(size) || expr.symbol.isPtr() && !raw); // store in the instruction only if it fits, or allow special case of 'null' ptr value because in both 32/64 it will be just 0
      Call &call  =*(Call*)(&code[pos]);
      call.params =params;
      call.func   =GetSetConstCall(size, heap); if(!call.func){compiler.msgs.New().error("CompileSetConst invalid expression size"); return;}
              call.param(0).setInstructI(offset); // set as offset from stack to the value
      if(heap)call.param(1).setHeap     (compiler.heapConstant(&raw, size, align), 0, false); // create constant on the heap
      else    call.param(1).setInstruct (expr.asRaw(compiler));
   }
}
static void CompileSetAddrStackHeap(Expr &expr, Compiler &compiler, Bool stack, Int offset, Memc<Byte> &code, Bool auto_map=true) // this function puts address of 'expr' variable into specified position offset at the stack/heap
{
   Int   params=2, // target offset + src variable
         pos   =code.addNum(Call::Size(params));
   Call &call  =*(Call*)(&code[pos]);
   call.params=params;
   call.func  =(stack ? GetSetAddrStackCall() : GetSetAddrHeapCall()); if(!call.func){compiler.msgs.New().error("CompileSetAddrStackHeap unknown func call"); return;}
   call.param(0).setInstructI(offset); // set as offset from stack/heap to the value
   call.param(1).set         (expr, compiler, auto_map);
}
static void CompileSetAddrResult(Expr &expr, Compiler &compiler, Memc<Byte> &code) // this function puts address of 'expr' variable into the result stack
{
   Int   params=1, // src variable
         pos   =code.addNum(Call::Size(params));
   Call &call  =*(Call*)(&code[pos]);
   call.params=params;
   call.func  =GetSetAddrResultCall(); if(!call.func){compiler.msgs.New().error("CompileSetAddrResult unknown func call"); return;}
   call.param(0).set(expr, compiler);
}
enum PASS_TYPE // how the parameter is passed
{
   PASS_NONE ,
   PASS_MAP  , // map local in stack
   PASS_CONST, // set constant
   PASS_ADDR , // set address
};
static void CompileExpr(Expr &expr, Compiler &compiler, Memc<Byte> &code)
{
   // always compile parents of this expression as first
   FREPA(expr.parent)CompileExpr(expr.parent[i], compiler, code);

   if(!expr.known())
   {
      if(expr.func.elms()==1)
      {
         Expr &func=expr.func[0];
         if(func._operator && func==',') // for "a,b" operator process all parameters in their order
         {
            FREPA(func.params)CompileExpr(func.params[i], compiler, code);
         }else
         if(func.func_call)
         {
            // compile func parent and params first
            FREPA(func.parent)CompileExpr(func.parent[i], compiler, code);
            FREPA(func.params)CompileExpr(func.params[i], compiler, code);

            if(func.func_call!=GetIgnoreCall())
            {
               Int   params=(FlagTest(func.func_call_mask, Expr::FC_RESULT) + FlagTest(func.func_call_mask, Expr::FC_PARENT) + func.params.elms()),
                     pos   =code.addNum(Call::Size(params));
               Call &call  =*(Call*)(&code[pos]);
               call.params=params;
               call.func  =func.func_call;
               params=0;
               if(func.func_call_mask&Expr::FC_RESULT)                  {                                     call.param(params++).set(expr          , compiler);}
               if(func.func_call_mask&Expr::FC_PARENT)                  {if(func.parent.elms()!=1)goto error; call.param(params++).set(func.parent[0], compiler);}
                                                      FREPA(func.params){                                     call.param(params++).set(func.params[i], compiler);}
            }
            return; // success
         }else
         if(func.symbol && func.symbol->type==Symbol::FUNC && func.symbol->source && func.symbol->source->active) // Esenthel Script function, RESULT PARENT PARAMS
         {
            if(func.params.elms()!=func.symbol->realParams())compiler.msgs.New().error("Invalid number of function parameters", expr.origin);else
            {
               // compile func parent first
               FREPA(func.parent)CompileExpr(func.parent[i], compiler, code);

               // get func info
               Bool has_result =func.symbol->hasResult(null),
                    has_this   =func.symbol->isClassNonStaticFunc(),
                     is_virtual=func.symbol->isVirtualFunc(null);

               if(is_virtual)
               {
                  // TODO: remember to checkup virtual disabling when specyfing class "obj.Class::method()" (this is done in "Expr.cpp")
                  compiler.msgs.New().error("Calling virtual script methods is not yet supported", expr.origin);
               }

               // TODO: array_dims
               Mems<PASS_TYPE> param_pass; param_pass.setNumZero(func.params.elms());
               FREPA(func.params) // order is important as parameters must be in continuous memory
               {
                  Expr          &src=func.params[i];
                  Symbol::Modif dest=func.symbol->params[i]->value; dest.proceedToFinal(&expr.symbol.templates, true);
                  /* func.params | func.symbol->params
                        SRC           DEST
                        int  ->       int   GOOD (can use SRC, if can't then use COPY CTOR)
                        int  ->       int&  GOOD (                      must use REF  CTOR)
                        int& ->       int   GOOD (                      must use COPY CTOR)
                        int& ->       int&  GOOD (can use SRC, if can't then use REF  CTOR)
                        int  -> const int   GOOD (can use SRC, if can't then use COPY CTOR)
                        int  -> const int&  GOOD (                      must use REF  CTOR)
                        int& -> const int   GOOD (                      must use COPY CTOR)
                        int& -> const int&  GOOD (can use SRC, if can't then use REF  CTOR)
                  const int  ->       int   GOOD (                      must use COPY CTOR) (SRC may be allowed for basicType, read below)
                  const int  ->       int&  BAD
                  const int& ->       int   GOOD (                      must use COPY CTOR)
                  const int& ->       int&  BAD
                  const int  -> const int   GOOD (can use SRC, if can't then use COPY CTOR)
                  const int  -> const int&  GOOD (                      must use REF  CTOR)
                  const int& -> const int   GOOD (                      must use COPY CTOR)
                  const int& -> const int&  GOOD (can use SRC, if can't then use REF  CTOR)*/
                  Bool is_null=(src=="null"), good_null=(is_null && dest.isPtr());
                  UInt  src_const_level=src.symbol.const_level; FlagDisable( src_const_level, 1<<(src.symbol.ptr_level+src.symbol.array_dims.elms())); // disable top const
                  UInt dest_const_level=dest      .const_level; FlagDisable(dest_const_level, 1<<(dest      .ptr_level+dest      .array_dims.elms())); // disable top const
                  if((!src.symbol.same(dest, false, false) || LostConst(src_const_level, dest_const_level)) && !good_null) // if symbol is not the same (test underlying consts but not the main ones)
                  {
                     compiler.msgs.New().error(S+"Can't convert parameter #"+(i+1)+" from '"+src.symbol.modifName(true, true, false)+"' to '"+dest.modifName(true, true, false)+"'", src.origin);
                  }else
                  if((is_null || LostConst(src.symbol.const_level, dest.const_level)) && FlagTest(dest.modifiers, Symbol::MODIF_REF)) // const int/int& -> int&  BAD
                  {
                     compiler.msgs.New().error("Can't convert const parameter to non-const reference", src.origin);
                  }else
                  if(
                      (!LostConst(src.symbol.const_level, dest.const_level)           // only if not losing const
                    || (src.basicType() && !(src.symbol.modifiers&Symbol::MODIF_REF)) //      if     losing const then allow optimization only if we're operating on non-ref temporary basic type like "const int" (because if it's a LOCAL variable then we can modify it and don't care, if it's a KNOWN constant then it will be copied using CompileSetConst to the stack either way and we will be operating on the copy)
                    || good_null                                                      // when passing null the const doesn't matter
                      )
                  && src.fullTemporary() && (src.symbol.modifiers&Symbol::MODIF_REF)==(dest.modifiers&Symbol::MODIF_REF) && src.mem.offset==0) // use SRC
                  {
                     if(src.mem.type==Expr::Memory::LOCAL)param_pass[i]=PASS_MAP  ;else
                     if(src.mem.type==Expr::Memory::KNOWN)param_pass[i]=PASS_CONST;else // since we're copying the constant to the stack, then this allows above optimization when losing const but operating on basic type
                                                          compiler.msgs.New().error("Invalid expression memory type for src_temporary + dest_parameter merging optimization", src.origin);
                  }else
                  if(dest.modifiers&Symbol::MODIF_REF) // use REF CTOR, compile code which sets Ptr address at specified location of a custom variable
                  {
                     param_pass[i]=PASS_ADDR;
                     compiler.mapVar(src.mem); // make sure that param is mapped before func param stack
                  }else
                  {
                     src.copyCtor(compiler); // perform copy constructor
                     param_pass[i]=PASS_MAP; // created temporary should be mapped directly to the param stack
                  }
               }

               // ALIGN
               // set memory map for params in local stack (must be CONTINUOUS) do this before compiling params so the result is already set, RESULT PARENT PARAMS
               if(   param_pass.elms() )compiler.stack_size=Ceil8(compiler.stack_size); // must be aligned to max possible var size, because when compiling functions, each of their parameter's offset is calculated from zero
               Int   param_stack_offset=compiler.stack_size;
               FREPA(param_pass)
               {
                  Expr &src=func.params[i];
                  switch(param_pass[i])
                  {
                     case PASS_MAP  : compiler.mapLocalInStack(src.mem); break;
                     case PASS_CONST: CompileSetConst         (src, compiler, code); break;
                     case PASS_ADDR : {Int offset=AlignAddress(compiler.stack_size, PtrSize); compiler.stack_size=offset+PtrSize; CompileSetAddrStackHeap(src, compiler, true, offset, code, false);} break;
                  }
               }

               // compile params
               FREPA(func.params)CompileExpr(func.params[i], compiler, code);

               Int   params=1+has_this+has_result, // (func_body + param_stack)(2in1) + this + result
                     pos   =code.addNum(Call::Size(params));
               Call &call  =*(Call*)(&code[pos]);
               call.params=params;
               call.func  =GetCallFunc(has_this, has_result);
               params=0;
               call.param(params++).setInstructPI2(func.symbol(), // set pointer to func symbol, because that function may not yet be compiled into FuncBody, REMAP needed
                                             param_stack_offset); // set as offset from stack to first element in param stack
               if(has_this)
               {
                  Call::Param &call_parent=call.param(params++);
                  if(func.parent.elms()==1)
                  {
                     Expr &expr_parent=func.parent[0]; if(!expr_parent.instance || !expr_parent.symbol.isObj())compiler.msgs.New().error("Invalid object for method call", expr.origin);
                     call_parent.set(expr_parent, compiler);
                  }else
                  {
                     compiler.msgs.New().error("No object specified for method call", expr.origin);
                  }
               }
               if(has_result)call.param(params++).setRef(expr, compiler); // if result is a reference then we want to get the address of the reference and not the value that it points to
            }
            return; // don't process any more messages
         }
      error:
         compiler.msgs.New().error("Can't compile expression", expr.origin);
      }
   }
}
static void DestroyVar(Symbol::Modif &type, Token *token, Compiler &compiler, Memc<Byte> &code, Bool local, Int local_index, Symbol *global_symbol)
{
   if(type.isArray())
   {
      compiler.msgs.New().error(S+"Destructor for arrays is not yet supported", token);
   }else
   {
      Expr result, &func=result.func.New(), &parent=func.parent.New();
      func.instance=true;
      func.symbol  =FindChild(S+'~'+type, type(), null, false); // find destructor function
      if(!func.symbol)
      {
         func.func_call     =FindFuncCall(DtorName(*type));
         func.func_call_mask=Expr::FC_PARENT;
      }
      if(!func.symbol && !func.func_call)compiler.msgs.New().error(S+"Destructor not found for "+type.modifName(false, false, false), token);else
      {
         parent.instance=true;
         parent.symbol  =type;
         if(local)parent.mem.setLocal(local_index);else parent.mem.setGlobal(*global_symbol);
         if(func.func_call || result.calculate(compiler)>CAST_NONE)CompileExpr(result, compiler, code); // there is no need to calculate if we already know the 'func_call'
      }
   }
}
static void DestroyLiveLocal(Int i, Bool remove, Compiler &compiler, Memc<Byte> &code)
{
   if(InRange(i, compiler.live_locals))
   {
      Int    local_index=compiler.live_locals[i]; if(remove)compiler.live_locals.remove(i, true); // keep order
      Local &local      =compiler.locals[local_index];
      DestroyVar(local.type, local.token, compiler, code, true, local_index, null);
   }
}
static void DestroyInstructScopes(Compiler &compiler, Memc<Byte> &code)
{
   REPA(compiler.live_locals) // process in reversed order (ORDER IS IMPORTANT)
   {
      Local &local=compiler.locals[compiler.live_locals[i]];
      if(!local.block_scope)DestroyLiveLocal(i, true, compiler, code); // only instruction scope
   }
}
static void DestroyBlockScopes(Int label_index, Compiler &compiler, Memc<Byte> &code)
{
   REPA(compiler.live_locals) // process in reversed order (ORDER IS IMPORTANT)
   {
      Local &local=compiler.locals[compiler.live_locals[i]];
      if(local.block_scope && local.scope_label==label_index)DestroyLiveLocal(i, true, compiler, code);
   }
}
static Int LabelCommandIndex(Int label, Memc<Command> &cmds, Compiler &compiler, Int token_index) // get command index of specified label (always calculate in case commands container will get changed dynamically during compilation)
{
   if(InRange(label, compiler.labels))REPA(cmds)if(cmds[i].type==CMD_LABEL && cmds[i].label_index==label)return i;
   compiler.msgs.New().error("Invalid label index", compiler.source, token_index);
   return -1;
}
static void DestroyGoto(Int start_command, Int goto_label_index, Memc<Command> &cmds, Compiler &compiler, Memc<Byte> &code, Int token_index)
{
   if(compiler.live_locals.elms()) // only if there are live locals
   {
      Int goto_command_index=LabelCommandIndex(goto_label_index, cmds, compiler, token_index);
      REPA(compiler.live_locals) // process in reversed order (ORDER IS IMPORTANT)
      {
         Local &local=compiler.locals[compiler.live_locals[i]];
         if(local.block_scope) // only block scope
         {
            Int var_start=                  local.scope_start,                               // command index of variable creation
                var_end  =LabelCommandIndex(local.scope_label, cmds, compiler, token_index); // command index of variable destruction
            if(goto_command_index<=var_start  //       include start, so in case of jumping we will delete the variable, because it will be created in that command again , example: "0) Str s="123"; 1) goto 0;"
            || goto_command_index> var_end  ) // don't include end  , because at end, the variable gets destroyed, and we don't want to destroy it twice (here and at end), example: "0) Str s; 1) goto 2; 2) s.Str::~Str();"
               DestroyLiveLocal(i, false, compiler, code); // don't remove from live locals, as goto may be optional, example: "{Str s; if(x)goto end; s.clear();} end:"
         }
      }
   }
}
static void CreateVar(Expr &var, Token &token, Symbol &symbol, Compiler &compiler, Memc<Byte> &code)
{
   var.create(token, compiler); // create the variable 'x' expression from its token (don't map it in the stack here, so it will be mapped after any parameters/default values)

   if(var.symbol.modifiers&Symbol::MODIF_REF) // reference - "TYPE &x="
   {
      if(var.symbol.isArray()                       )compiler.msgs.New().error("Arrays of references are not supported", &token);else
      if(!(symbol.modifiers&Symbol::MODIF_DEF_VALUE))compiler.msgs.New().error("References must be initialized"        , &token);else
      {
         Expr expr;
         if(!symbol.source || compiler.compileTokens(symbol.def_val_range.x, symbol.def_val_range.y, expr, &symbol.source->tokens)!=COMPILE_FULL)compiler.msgs.New().error("Can't compile tokens", symbol.source, symbol.def_val_range.x);else
         if(!expr.castTo(var.symbol, compiler))compiler.msgs.New().error(S+"Can't initialize '"+var.symbol.modifName(true, true, true)+"' from '"+expr.symbol.modifName(true, true, false), &token);else
         {
            expr.setBlockScope(compiler, var.mem.type==Expr::Memory::GLOBAL); // the value will be held by reference so set it as block scope (do this before code compilation), if we're initializing global reference then it means we're compiling "global var init" and block scope temporaries should be held in heap memory
            CompileExpr(expr, compiler, code); // compile the source expression so later we can get its address
            compiler.mapVar(var.mem); // make sure that var is mapped
            if(var.mem.type==Expr::Memory::LOCAL)
            {
               if(!InRange(var.mem.index, compiler.locals))compiler.msgs.New().error("Local variable index is invalid", &token);else
               {
                  Local &local=compiler.locals[var.mem.index];
                  if(local.stack_offset<0)compiler.msgs.New().error("Local variable stack offset is invalid", &token);
                  else CompileSetAddrStackHeap(expr, compiler, !local.force_heap, local.stack_offset, code); // add instruction that sets the 'expr' address into the 'var' reference
               }
            }else
            if(var.mem.type==Expr::Memory::GLOBAL)
            {
               if(var.mem.index<0)compiler.msgs.New().error("Global variable heap offset is invalid", &token);
               else CompileSetAddrStackHeap(expr, compiler, false, var.mem.index, code); // add instruction that sets the 'expr' address into the 'var' reference
            }else
            {
               compiler.msgs.New().error("Invalid variable memory type for CreateVar", &token);
            }
         }
      }
   }else
   if(var.symbol.anyPtr() || (var.symbol && var.symbol->var_type!=VAR_NONE)) // basic type obj or pointer - "int x", "Obj *x"
   {
      if(symbol.modifiers&Symbol::MODIF_DEF_VALUE)
      {
         if(var.symbol.isArray())compiler.msgs.New().error("Variable initialization as array is not yet supported", &token);else
         {
            var.symbol.setConst(false); // disable const to allow setting the value
            Memc<Expr> expr; Swap(expr.New(), var); expr.New().setOperator(u"=").origin=&token; expr.New().setOperator(u"("); Bool ok=(symbol.source && compiler.appendTokens(expr, symbol.def_val_range.x, symbol.def_val_range.y, &symbol.source->tokens)); expr.New().setOperator(u")"); // create the "var=(..)" expressions
            Expr result;
            if(!ok || compiler.compileExpr(expr, token.parent, result)!=COMPILE_FULL)compiler.msgs.New().error("Can't compile tokens", symbol.source, symbol.def_val_range.x);else
            {
               if(compiler.ctx && compiler.ctx->store_known_global_var_on_heap
               && result.mem.type==Expr::Memory::GLOBAL && result.mem.index>=0                                                         // if we're initializing global var
               && result.func.elms()==1 && result.func[0]=='=' && result.func[0].params.elms()==2 && result.func[0].params[1].known()) // and the result is known then store that value on the heap instead of as codes (check known for the 'b' in "a=b" because 'a' known may be disabled)
               {
                  // we must first convert to target type in case of "int=flt"
                  Expr temp, &func=result.func[0];
                  temp.mem.setKnown();
                  temp.symbol=result.symbol;
                  temp.calculateKnown(func, func.params[0], func.params[1], compiler);
                  U64 raw=temp.asRaw(compiler); compiler.varHeapConstant(result.mem.index+result.mem.offset, &raw, result.symbol.rawSize(true));
               }else
               {
                  CompileExpr(result, compiler, code);
               }
               Swap(result, var);
            }
         }
      }
   }else // class object or array - "Vec x", "Vec x(..)", "Str x(..)", "Vec v[2];", "Vec v[]={..}"
   {
      if(var.symbol.isArray())compiler.msgs.New().error("Variable initialization as array is not yet supported", &token);else
      {
         var.instance=false; // disable instance to allow constructor call
         Memc<Expr> expr; Swap(expr.New(), var); expr.New().setOperator(u"("); // create the "var(..)" expressions
         Bool ok=((symbol.modifiers&Symbol::MODIF_DEF_VALUE) ? symbol.source && compiler.appendTokens(expr, symbol.def_val_range.x, symbol.def_val_range.y, &symbol.source->tokens) : true);
         expr.New().setOperator(u")");
         Expr result;
         if(!ok || compiler.compileExpr(expr, token.parent, result)!=COMPILE_FULL)compiler.msgs.New().error("Can't compile tokens", symbol.source, symbol.def_val_range.x);else
         {
            CompileExpr(result, compiler, code); Swap(result, var);
         }
      }
   }
   DestroyInstructScopes(compiler, code);
}
/******************************************************************************/
// TODO: optimizations:
// replace "goto label; ..; label: goto label2;" with "goto label2; ..; label: goto label2;" (replace jump->jump with jump)
// skip "goto label; label:" jumps to labels right after instruction
#if 1
static Bool FollowedByLabel(Memc<Command> &cmds, Int i, Int label_index) // this checks following command if it's a label (version below checks all labels, however this is no longer used since from now, labels can also generate codes - calling destructors)
{
   if(InRange(i+1, cmds)){Command &cmd=cmds[i+1]; return cmd.type==CMD_LABEL && cmd.label_index==label_index;}
   return false;
}
#else
static Bool FollowedByLabel(Memc<Command> &cmds, Int i, Int label_index) // check all following labels if one of them is 'label_index' (break on any other command)
{
   for(; ++i<cmds.elms(); ) // start from 1 after 'i'
   {
      Command &cmd=cmds[i];
      if(cmd.type!=CMD_LABEL)break;
      if(cmd.label_index==label_index)return true;
   }
   return false;
}
#endif
void Compiler::compile(Memc<Command> &cmds, Mems<Byte> &code, Symbol &func, Symbol::Modif *result_value)
{
   Memc<Int > label_code_pos; label_code_pos.setNum(labels); // code position for labels
   Memc<Int > gotos; // code position for goto commands
   Memc<Byte> temp ; // use temporary Memc for faster creation of the codes

   Bool has_return=false;

   FREPA(cmds)
   {
      Command &cmd=cmds[i];
      cmd.code_pos=temp.elms(); // set code position of current command
      cmd_index   =i; // set index of currently compiled command
      scope_label =cmd.scope_label; // set index of scope label
      switch(cmd.type)
      {
         default: msgs.New().error("Unsupported CMD_TYPE", source, cmd.startTokenIndex()); break;

         case CMD_LABEL:
         {
            if(!InRange(cmd.label_index, labels))msgs.New().error("Unknown label", source, cmd.startTokenIndex());else
            {
               label_code_pos(cmd.label_index)=cmd.code_pos;
               DestroyBlockScopes(cmd.label_index, T, temp);
            }
         }break;

         case CMD_INSTRUCT:
         {
            Bool has_vars=false;
            for(Int i=cmd.raw_range.x; i<=cmd.raw_range.y && InRange(i, tokens); i++) // check if in the token range there is a variable definition "int x;", "int a=0, b=a;", "Vec v(0,0,0);" (order is important)
            {
               Token &token=*tokens[i]; if(token.def_decl)if(Symbol *symbol=token.symbol()) // TYPE x; TYPE x=..; TYPE x(..);
               {
                  has_vars=true;
                  if(symbol->isVar()) // ignore functions, classes or some other declarations
                  {
                     Expr var; CreateVar(var, token, *symbol, T, temp);
                  }
               }
            }
            if(!has_vars) // if there is no local variable definition then just process the commands
            {
               Expr expr;
               if(compileTokens(cmd.raw_range.x, cmd.raw_range.y, expr)!=COMPILE_FULL)msgs.New().error("Can't compile tokens", source, cmd.startTokenIndex());else
               {
                  CompileExpr(expr, T, temp);
                  DestroyInstructScopes(T, temp);
               }
            }
         }break;
         
         case CMD_RETURN: // RESULT PARENT PARAMS
         {
            has_return=true;
            if(cmd.raw_range.x <= cmd.raw_range.y) // "return x;"
            {
               if(!result_value)msgs.New().error("Function can't return a value", source, cmd.startTokenIndex());else
               {
                  Expr result, value;
                  result.instance=true;
                  result.symbol  =*result_value;
                  result.mem.setResult();

                  if(result.symbol.modifiers&Symbol::MODIF_REF)
                  {
                     if(compileTokens(cmd.raw_range.x, cmd.raw_range.y, value)!=COMPILE_FULL)msgs.New().error("Can't compile tokens", source, cmd.raw_range.x);else
                     if(!value.castTo(result.symbol, T))msgs.New().error(S+"Can't initialize '"+result.symbol.modifName(true, true, true)+"' from '"+value.symbol.modifName(true, true, false), source, cmd.raw_range.x);else
                     {
                        CompileExpr         (value, T, temp); // compile the source expression so later we can get its address
                        CompileSetAddrResult(value, T, temp); // add instruction that sets the 'value' address into the 'result' reference
                     }
                  }else
                  if(result.basicType() && !result.symbol.isArray()) // basic type obj or pointer - "int x", "Obj *x"
                  {
                     result.symbol.setConst(false); // disable const to allow setting the value
                     Memc<Expr> expr; Swap(expr.New(), result); expr.New().setOperator(u"="); expr.New().setOperator(u"("); Bool ok=appendTokens(expr, cmd.raw_range.x, cmd.raw_range.y); expr.New().setOperator(u")"); // create the "result=(..)" expressions
                     if(!ok || compileExpr(expr, InRange(cmd.raw_range.x, tokens) ? tokens[cmd.raw_range.x]->parent : null, value)!=COMPILE_FULL)msgs.New().error("Can't compile tokens", source, cmd.raw_range.x);
                     else      CompileExpr(value, T, temp);
                  }else
                  if(!result.basicType() && result.symbol.isObj()) // class object - "Vec x", "Vec x(..)", "Str x(..)"
                  {
                     if(compileTokens(cmd.raw_range.x, cmd.raw_range.y, value)!=COMPILE_FULL)msgs.New().error("Can't compile tokens", source, cmd.raw_range.x);else
                     {
                        // check if we're returning temporary object of the same type, in that case we can map it to the return value already
                        if(value.fullTemporary() && value.symbol.same(result.symbol, false, true) && !LostConst(value.symbol.const_level, result.symbol.const_level) && unmappedLocal(value.mem))
                        {
                           value.mem=result.mem;
                           if(value.func.elms()) // ctors need to have parent changed too
                           {
                              Expr &func=value.func[0];
                              if(func.symbol && (func.symbol->modifiers&Symbol::MODIF_CTOR))
                              {
                                 DEBUG_ASSERT(func.parent.elms(), "no parent for ctor");
                                 if(func.parent.elms())func.parent[0].mem=value.mem;
                              }
                           }
                           CompileExpr(value, T, temp);
                        }else // convert result to class constructor, and convert "return x" -> "result(x)" (as in ctor call)
                        {
                           result.instance=false; // disable instance to allow constructor call
                           Expr ctor, &func=ctor.func.New(); Swap(func, result); Swap(func.params.New(), value);
                           if(ctor.calculate(T)>CAST_NONE)CompileExpr(ctor, T, temp);
                        }
                     }
                  }else
                  {
                     // TODO:
                     msgs.New().error("Returning value not yet supported", source, cmd.startTokenIndex());
                  }
               }
            }else // "return;"
            {
               if(result_value)msgs.New().error("Function must return a value", source, cmd.startTokenIndex());
            }
            DestroyInstructScopes(T, temp);
            goto write_goto; // jump to end of the function ('label_index' was set earlier)
         }break;

         case CMD_BREAK   : if(InRange(cmd.label_index, labels))goto write_goto; msgs.New().error("Invalid 'break'"   ); break;
         case CMD_CONTINUE: if(InRange(cmd.label_index, labels))goto write_goto; msgs.New().error("Invalid 'continue'"); break;

         case CMD_GOTO:
            write_goto:
         {
            if(!InRange(cmd.label_index, labels))msgs.New().error("Unknown label for 'goto' command", source, cmd.startTokenIndex());else
            if(!FollowedByLabel(cmds, i, cmd.label_index)) // check if we're not using goto to jump to command after us
            {
               // destroy all block scopes that are out of range
               DestroyGoto(cmd_index, cmd.label_index, cmds, T, temp, cmd.startTokenIndex());

               // write jump code
               Int   pos =temp.addNum(Call::Size(1)); // 1 param (label index)
               Call &call=*(Call*)(&temp[pos]);
               call.params=1;
               call.func  =GetGotoCall(); if(!call.func)msgs.New().error("Unknown 'goto' func", source, cmd.startTokenIndex());
               call.param(0).setInstructI(cmd.label_index); // REMAP needed
               gotos.add(pos); // add to goto container
            }
         }break;

         case CMD_GOTO_COND  : // if( cond)goto label; -> if(cond!=0)goto label;
         case CMD_GOTO_COND_N: // if(!cond)goto label; -> if(cond==0)goto label;
         {
            Expr cond;
            if(!InRange(cmd.label_index, labels))msgs.New().error("Unknown label for 'goto conditional' command", source, cmd.startTokenIndex());else
            {
               Bool has_var=false;
               for(Int i=cmd._for.cond_range.x; i<=cmd._for.cond_range.y && InRange(i, tokens); i++) // check if in the token range there is a variable definition "int x;", "int a=0, b=a;", "Vec v(0,0,0);" (order is important)
               {
                  Token &token=*tokens[i]; if(token.def_decl)if(Symbol *symbol=token.symbol()) // TYPE x; TYPE x=..; TYPE x(..);
                  {
                     if(has_var){msgs.New().error("Multiple variable definition inside condition is invalid", source, cmd.startTokenIndex()); break;}
                     has_var=true;
                     if(symbol->isVar())CreateVar(cond, token, *symbol, T, temp);else msgs.New().error("Invalid symbol declaration inside condition", source, cmd.startTokenIndex());
                  }
               }
               if(!has_var)
               {
                  if(compileTokens(cmd._for.cond_range.x, cmd._for.cond_range.y, cond)!=COMPILE_FULL){msgs.New().error("Can't compile tokens", source, cmd.startTokenIndex()); break;}
               }

               if(cond.known())
               {
                  if(cond.asBool(T)==(cmd.type==CMD_GOTO_COND))goto write_goto; // for GOTO_COND type add goto only if "cond!=0", for GOTO_COND_N type add goto only if "cond==0" (in other case don't do anything)
               }else
               {
                  if(has_var) // compilation of calculation of 'cond' was already performed in 'CreateLocal', now we need to make sure that it can be casted to bool
                  {
                     // leave only raw data (memory layout) without calculation
                     cond.parent.del(); // delete parent calculation
                     cond.func  .del(); // delete func   calculation
                     cond.params.del(); // delete params calculation
                  }
                  if(!cond.castToConditional())msgs.New().error("Can't convert to bool", source, cmd.startTokenIndex());else
                  {
                     // check if the condition result needs to be copied into temporary conditional before destroying it
                     if(!has_var) // don't need to test if it's a named variable
                     {
                        if(cond.symbol.modifiers&Symbol::MODIF_REF) // returned value is a reference to something (it may point to some member of the object about to be destroyed) "class X {int x=1; ~X(){x=0;} int& ref(){return x;}} void test() {if(X().ref());}"
                        {
                           REPA(live_locals)if(!locals[live_locals[i]].block_scope) // if there is at least one variable that needs to be destroyed
                           {
                              cond.copyCtor(T);
                              break;
                           }
                        }else
                        if(cond.mem.type==Expr::Memory::LOCAL && InRange(cond.mem.index, locals)) // check if the condition result is a local variable
                           REPA(live_locals)if(live_locals[i]==cond.mem.index) // check if that local variable is live and needs to be destroyed
                        {
                           if(!locals[cond.mem.index].block_scope)cond.copyCtor(T); // if not block scope then copy it to temporary
                           break;
                        }
                     }

                     // first compile expression
                     CompileExpr(cond, T, temp);
                     DestroyInstructScopes(T, temp); // call destructors before doing goto

                     if(!FollowedByLabel(cmds, i, cmd.label_index)) // check if we're not using goto to jump to command after us
                     {
                        // this should NOT perform destroying block scopes that are out of range

                        // write conditional jump code
                        Int   pos =temp.addNum(Call::Size(2)); // 2 params (label index, condition)
                        Call &call=*(Call*)(&temp[pos]);
                        call.params=2;
                        call.func  =((cmd.type==CMD_GOTO_COND) ? GetGotoCondCall(cond.symbol) : GetGotoCondNCall(cond.symbol)); if(!call.func)msgs.New().error("Unknown 'goto conditional' func", source, cmd.startTokenIndex());
                        call.param(0).setInstructI(cmd.label_index); // REMAP needed
                        call.param(1).set         (cond, T);
                        gotos.add(pos); // add to goto container
                     }
                  }
               }
            }
         }break;
      }
   }

   // copy into final memory
   code=temp;

   // adjust goto's (replace 'label_index' with 'label_code_pos[label_index]') (do this after having final memory of codes)
   REPA(gotos)
   {
      Call &call       =*(Call*)(&code[gotos[i]]);
      Int   label_index=call.param(0).getInstructI();
      call.param(0).addr=code.data()+label_code_pos[label_index];
   }

   // check if function does not return anything
   if(result_value && !has_return)msgs.New().error("Function does not return a value", &func);
}
void CodeEnvironment::FuncBody::del()
{
   zero();
   code.del();
   name.del();
}
void CodeEnvironment::FuncBody::create(Symbol &func, Memc<Message> &msgs, CompilerContext &ctx)
{
   del();

   // RESULT PARENT PARAMS
   Symbol::Modif result;
   Bool          has_result      =func.hasResult(null, &result);
   Int           param_stack_size=0;
   REPAO(func.children    )->raw_offset=-1; // make sure that each local doesn't have 'raw_offset' set
   REPAO(func.params      )->raw_offset=-1;
   FREP (func.realParams()) // order important
   {
      Symbol::Modif value=func.params[i]->value; value.proceedToFinal(null);
      if(!value || value->type==Symbol::TYPENAME)msgs.New().error("Uknown parameter type", func.params[i]());
      Int size =value.rawSize(true),
          align=value.firstMemberSize();
      func.params[i]->raw_offset=AlignAddress(param_stack_size, align); // ALIGN
      param_stack_size=func.params[i]->raw_offset+size;
   }

   if(Source *source=func.source)
   {
      Memc<Command> cmds, exp; source->parseFunc(func, cmds, msgs);
    //FileText ft; ft.append("c:/funcs.txt"); ListFunction(ft, func, cmds);
      Compiler compiler(msgs, source->tokens, source, &ctx);

      // set label indexes
      REPA(func.children)if(func.children[i]->type==Symbol::LABEL)func.children[i]->raw_offset=compiler.labels++;

      Int label_body_scope=compiler.labels++,
          label_return    =compiler.labels++;
      compiler.expand(cmds, exp, -1, -1, label_return, label_body_scope);
      {Command &d=exp.New(); d.type=CMD_LABEL; d.label_index=label_body_scope;} // place body scope label at the end of the codes (this deletes all variables)
      {Command &d=exp.New(); d.type=CMD_LABEL; d.label_index=label_return    ;} // place return     label at the end of the codes

      compiler.compile(exp, code, func, has_result ? &result : null);

      name=FuncName(func);
      stack_size=Ceil8(compiler.stack_size); // ALIGN, use Ceil8 to make sure that newer variables will have 8 byte alignment
   }
}
// TODO: statics in functions "void func() {Str s; static Str ss=s;}"
// TODO: vars as known, setup into heap constants instead of instructions
struct GlobalVarInit
{
};
void CodeEnvironment::createGlobalVarSetup(Int func_index, Memc<Message> &msgs, CompilerContext &ctx)
{
   if(InRange(func_index  , func_bodies)
   && InRange(func_index+1, func_bodies))
   {
      Str                 temp;
      Memc<Symbol*      > vars, sorted;
      Memc<Symbol::Modif> templates;

      // get global vars and clear dependencies for all symbols
      FREPA(Symbols)
      {
         Symbol &symbol=Symbols.lockedData(i);
         symbol.dependencies.del();
         FlagSet(symbol.helper, Symbol::HELPER_PROCESSED, true);
         if(symbol.source && symbol.source->active && symbol.valid && symbol.isVar() && symbol.isGlobalOrStatic())
         {
            FlagSet(symbol.helper, Symbol::HELPER_PROCESSED, false);
            vars.add(&symbol);
         }
      }

      // setup dependencies
      REPA(vars)
      {
         Symbol &symbol=*vars[i];
         if(symbol.modifiers&Symbol::MODIF_DEF_VALUE)
         {
            for(Int i=symbol.def_val_range.x; i<=symbol.def_val_range.y; )
            {
               Int start=i;
               if(Symbol *s=GetFullSymbol(symbol.source->tokens, i, temp, symbol.parent(), templates))
               {
                  if(!(s->helper&Symbol::HELPER_PROCESSED))symbol.dependencies.include(s);
               }
               MAX(i, start+1);
            }
         }
      }

      // process all that haven't got left any dependencies "int a=1, b=a+1, c=func(), d=func()+a" (process 'a', and then 'b', but not 'c' 'd')
      REPA(vars)
      {
         Symbol &symbol=*vars[i];
         REPA(symbol.dependencies) // check if any dependency got processed
         {
            Symbol &dep=*symbol.dependencies[i];
            if(dep.helper&Symbol::HELPER_PROCESSED)symbol.dependencies.remove(i); // if dependency was processed then remove it
         }
         if(!symbol.dependencies.elms()) // if no dependencies got left
         {
            FlagEnable(symbol.helper, Symbol::HELPER_PROCESSED); // set as processed
            sorted.add(&symbol); // add to processed
            vars.remove(i); // remove from to process
         }
      }

      // process all that haven't got left any dependencies on vars "int a, c=func(), d=func()+a" (process 'c' but not 'd')
      REPA(vars)
      {
         Symbol &symbol=*vars[i];
         REPA(symbol.dependencies) // check if any dependency got processed
         {
            Symbol &dep=*symbol.dependencies[i];
            if((dep.helper&Symbol::HELPER_PROCESSED) // if processed
            || !(dep.source && dep.source->active && dep.valid && dep.isVar() && dep.isGlobalOrStatic())) // or not a global var
               symbol.dependencies.remove(i); // remove dependency
         }
         if(!symbol.dependencies.elms()) // if no dependencies got left
         {
            FlagEnable(symbol.helper, Symbol::HELPER_PROCESSED); // set as processed
            sorted.add(&symbol); // add to processed
            vars.remove(i); // remove from to process
         }
      }

      // add all remaining vars
      FREPA(vars)sorted.add(vars[i]);

      // process variables
      Memc<Token*> tokens; // use temporary tokens so we can pass it as dummy reference to compiler construct to avoid changing 'tokens' to pointer with null checks
      Compiler     compiler(msgs, tokens, null, &ctx);
      Int          label_body_scope=compiler.labels++;

      Memc<Byte   > temp_init, temp_shut;
      Memc<Symbol*> dtors;

      // initialize variables
      compiler.scope_label=label_body_scope;
      FREPA(sorted) // in forward order
      {
         Symbol &symbol=*sorted[i];
         if(Token *token=symbol.getToken())
         {
            Expr var; CreateVar(var, *token, symbol, compiler, temp_init);
            if(var.symbol.hasDestructor())dtors.add(&symbol); // upon creation add in same order to destructor list
         }else
         {
            msgs.New().error("Unknown token for variable", &symbol);
         }
      }

      // destroy variables
      REPA(dtors) // first destroy named variables (in reversed order)
      {
         Symbol       &symbol=*dtors[i];
         Symbol::Modif final; final=&symbol; final.proceedToFinal(null);
         DestroyVar(final, symbol.getToken(), compiler, temp_shut, false, -1, &symbol);
      }
      DestroyBlockScopes(label_body_scope, compiler, temp_shut); // then destroy temporaries

      // setup function bodies
      FuncBody &init=func_bodies[func_index  ],
               &shut=func_bodies[func_index+1];
      init.del(); init.name="global var init"; init.code=temp_init; init.stack_size=Ceil8(compiler.stack_size); // ALIGN, use Ceil8 to make sure that newer variables will have 8 byte alignment
      shut.del(); shut.name="global var shut"; shut.code=temp_shut; shut.stack_size=Ceil8(compiler.stack_size);
   }
}
void CodeEnvironment::FuncBody::link(Memc<Message> &msgs, CodeEnvironment &ce)
{
   Call::Func c[]={GetCallFunc(false, false), GetCallFunc(false, true), GetCallFunc(true, false), GetCallFunc(true, true)};
   for(Int pos=0; pos<code.elms(); )
   {
      Call &call=*(Call*)(&code[pos]);
      REPA(c)if(call.func==c[i]) // if any of the 'call' functions
      {
         if(Symbol *func=(Symbol*)call.param(0).getInstructP())
         {
            if(InRange(func->raw_offset, ce.func_bodies))call.param(0).addr=&ce.func_bodies[func->raw_offset]; // REMAP from Symbol* to FuncBody*
            else msgs.New().error("Invalid func call 'func body'");
         }else   msgs.New().error("Invalid func call 'func' pointer");
         break;
      }
      pos+=Call::Size(call.params);
   }
}
void CodeEnvironment::FuncBody::optimize(Memc<Message> &msgs, CodeEnvironment &ce)
{
   for(Int pos=0; pos<code.elms(); )
   {
      Call &call=*(Call*)(&code[pos]);
      REP(call.params)
      {
         Call::Param &param=call.param(i);
         if(param.type==Call::ADDR_HEAP    ){param.type=Call::ADDR_GLOBAL    ; param.addr=ce.heap.data()+param.offset;}else
         if(param.type==Call::ADDR_HEAP_REF){param.type=Call::ADDR_GLOBAL_REF; param.addr=ce.heap.data()+param.offset;}
      }
      pos+=Call::Size(call.params);
   }
}
/******************************************************************************/
}}
/******************************************************************************/
