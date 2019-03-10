/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
// TODO: stack variables reuse memory of destroyed vars
/******************************************************************************/
static void SetMemUInt(Ptr data, UInt value, UInt size)
{
   VecB4 v; v.u=value;
   UInt *u=(UInt*)data; REP(size/SIZE(UInt))*u++ =v.u;
   Byte *b=(Byte*)   u; REP(size%SIZE(UInt)) b[i]=v.c[i&3];
}
/******************************************************************************/
void CodeEnvironment::Thread::del   () {stack.del(); levels.del(); zero();}
void CodeEnvironment::Thread::create(CodeEnvironment &env)
{
   del();
   stack.setNum(CE_STACK_SIZE);
   heap =env.heap.data();
   debug=env.debug;
   if(debug)SetMemUInt(stack.data(), 0xBAADF00D, stack.elms()); // in debug mode fill stack with 0xBAADF00D
}
#if WINDOWS
Bool CodeEnvironment::Thread::test()
{
   __try
   {
      for(; func_code_pos<func_code_end; )
      {
         Call &call=*(Call*)func_code_pos;
       //func_code_pos+=Call::Size(call.params); // adjust the code position BEFORE the call, to allow modifying position INSIDE the call (this is currently done inside func calls because it is faster that way)
         call.func(call, T);
      }
      return true;
   }
   __except(EXCEPTION_EXECUTE_HANDLER) {return false;} // unhandled exception encountered
}
#endif
Bool CodeEnvironment::Thread::start(FuncBody &func)
{
   if(func.stack_size>stack.elms())Exit("Function requires too big stack size");

   T.func         =&func;
   T.func_stack   = stack.data();
   T.func_code_pos= func.code.data();
   T.func_code_end= func.code.data()+func.code.elms();

   for(;;) // levels
   {
   #if WINDOWS
      if(debug)
      {
         if(!test())return false;
      }else
   #endif
      for(; func_code_pos<func_code_end; ) // function
      {
         Call &call=*(Call*)func_code_pos;
       //func_code_pos+=Call::Size(call.params); // adjust the code position BEFORE the call, to allow modifying position INSIDE the call (this is currently done inside func calls because it is faster that way)
         call.func(call, T);
      }

      if(!levels.elms())break; // no more levels left

      // go back to last function
      Level &level=levels.last(); // get last function
    T.func            =level.func            ; // set new function
      func_code_pos   =level.func_code_pos   ; // restore last code position
      func_code_end   =T.func->code.data()+T.func->code.elms(); // adjust code position end
      func_stack     -=T.func->stack_size    ; // decrease stack by NEW function
      func_param_stack=level.func_param_stack; // restore last param stack
      func_this       =level.func_this       ; // restore last this
      func_result     =level.func_result     ; // restore last result
      levels.removeLast();
   }
   return true;
}
void CodeEnvironment::Thread::call(FuncBody &func, Int param_stack_offset, Ptr func_this, Ptr result)
{
   // save current level
   Level &level=levels.New();
   level.func            =T.func;
   level.func_code_pos   =T.func_code_pos;
   level.func_param_stack=T.func_param_stack;
   level.func_this       =T.func_this;
   level.func_result     =T.func_result;

   // set new func
   T.func_param_stack=T.func_stack+param_stack_offset; // set new param stack from current stack offsetted by 'param_stack_offset'
   T.func_stack     +=T.func->stack_size; // adjust stack position by PREVIOUS function
   Int stack_required =func.stack_size,
       stack_available=stack.data() - T.func_stack + stack.elms();
   if( stack_required > stack_available)
   {
      Str msg=S+"Not enough stack memory available to call function.\nFunction call depth: "+levels.elms()+"\nTotal stack size: "+stack.elms()+"\nStack memory required: "+stack_required+"\nStack memory available: "+stack_available;
      if(debug)
      {
         Gui.msgBox("Error", msg);
         throw(0);
      }else
      {
         msg+="\nCall stack:";
         Int num=0;
                      msg+=S+'\n'+          func. name;
         REPA(levels){msg+=S+'\n'+levels[i].func->name; if(++num>=16){msg+="\n.."; break;}}
         Exit(msg);
      }
   }
   T.func         =&func;
   T.func_code_pos= func.code.data();
   T.func_code_end= func.code.data()+func.code.elms();
   T.func_this    = (Byte*)func_this;
   T.func_result  = (Byte*)result;
}
/******************************************************************************/
static Int CompareName(Symbol*C &a, Symbol*C &b) {return ComparePath(a->fileName(), b->fileName());}
Bool CodeEnvironment::VerifySymbols(Memc<Message> &msgs, Memc<Symbol*> &sorted_classes)
{
   // helpers, set in root namespace to avoid unnecessary memory allocations
   Str temp; Memc<Symbol::Modif> templates;

   // get symbols from current build files
   Memc<Symbol*> active_symbols;
   FREPA(Symbols)
   {
      Symbol &symbol=Symbols.lockedData(i);
      if(symbol.source && symbol.source->active && symbol.valid)active_symbols.add(&symbol);
   }

   // various tests
   FREPA(active_symbols)
   {
      Symbol &symbol=*active_symbols[i];

      // static global variables functions
      if(symbol.isGlobalAndStatic())msgs.New().error("Invalid \"static\" keyword for global variable/function.", &symbol);

      // transparent classes that don't have non-transparent class
      if(symbol.type==Symbol::CLASS && (symbol.modifiers&Symbol::MODIF_TRANSPARENT))
      {
         Symbol *p=symbol.Parent();
         if(!p || p->type!=Symbol::CLASS)msgs.New().error("Transparent class must belong to non-transparent class.", &symbol);
      }
      
      // const_mem_addr used for non-const_mem_addr
      if(symbol.source && !symbol.source->header) // only in custom sources
         if(!symbol.value.constMemAddrOK())
            msgs.New().warning("Specified class requires to be stored in a constant memory address (const_mem_addr), however the target does not support it. If you're sure that this is not an error, then please specify the class with 'const_mem_addr' modifier, as <const_mem_addr TYPE>", &symbol);
   }

   // clear dependencies and setup processed
   REPA(Symbols)
   {
      Symbol &symbol   =Symbols.lockedData(i); symbol.dependencies.del(); // clear dependencies
      Bool    processed=false;
      if( symbol.type==Symbol::KEYWORD           )processed=true; // keywords are already processed
      if(!symbol.source || !symbol.source->active)processed=true; // symbols which aren't in current build files (for example EE headers) set as processed
      FlagSet(symbol.helper, Symbol::HELPER_PROCESSED|Symbol::HELPER_PROCESSED_FULL, processed);
   }

   // setup dependencies
   REPA(Symbols)
   {
      Symbol &symbol=Symbols.lockedData(i);
      Bool    active=(symbol.source && symbol.source->active && symbol.valid);
      if(     active // process only active symbols (user defined, not from the engine)
      || symbol.type==Symbol::VAR && symbol.value && symbol.value->type==Symbol::TYPENAME // or engine defined if they're a template, this is needed because:
      )
      /*
         <TYPE> class FixedArray // this Engine Defined
         {
            TYPE data[..];
         }
         // following is user defined:
         class Base // base class
         {
         }
         class Use // usage of base using engine's 'FixedArray'
         {
            FixedArray<Base> array;
         }

         In the above example, we're making sure that 'data' is set as a dependency to 'FixedArray',
            thanks to which allows to detect 'Base' as dependency to 'Use'.
      */
      {
         REPA(symbol.base)symbol.addDependency(symbol.base[i]); // add base classes
         if(symbol.type==Symbol::VAR && symbol.Parent() && symbol.Parent()->type==Symbol::CLASS)symbol.Parent()->addDependency(symbol.value); // add member types to their classes

         // add all nested classes because they couldn't be forward declared earlier, for example "class A { class AA {} }   class B { A.AA *x; }" make sure that "B" depends on root of "AA" (which is "A") this will guarantee the "A" header declared before "B" header (so "AA" is known)
         if(symbol.type==Symbol::VAR || symbol.type==Symbol::FUNC || symbol.type==Symbol::TYPEDEF)
         {
            if(symbol.Parent() && symbol.Parent()->type==Symbol::CLASS)
            {
               symbol.Parent()->addDependencyIfNested(symbol.value()); // include value of the varaible
               REPA(symbol.params)if(Symbol *param=symbol.params[i]()) // iterate all params of a function
               {
                  symbol.Parent()->addDependencyIfNested(param->value()); // include value of all parameters of the function to handle "void func(A.AA *x)"
                  if(param->modifiers&Symbol::MODIF_DEF_VALUE) // include default value for function parameters to handle "class A{}; A global; void func(A p=global);"
                     for(Int i=param->def_val_range.x; i<=param->def_val_range.y; )
                  {
                     Int start=i;
                     if(Symbol *s=GetFullSymbol(param->source->tokens, i, temp, &symbol, templates))
                     {
                        Symbol::Modif modif; modif=s; symbol.Parent()->addDependency(modif);
                     }
                     MAX(i, start+1);
                  }
               }
            }
         }
      }
   }

   // now that we know what typenames are used by what classes "<TYPE> class A { class Sub : TYPE { class Sub2 {} }}" (Sub has TYPE in its 'dependencies', Sub2 doesn't, however it should check for all parents)
   REPA(active_symbols)
   {
      Symbol &symbol=*active_symbols[i];
      if(symbol.Parent() && symbol.Parent()->type==Symbol::CLASS) // element that belongs to class
      {
         Symbol::Modif &value=symbol.value; // get its value "Memc<int> x" -> "Memc<int>"
         REPA(value.templates)
         {
            Symbol::Modif &templat=value.templates[i];
            if(Symbol *src_template=templat.src_template()) // iterate all its templates "Memc.TYPE"
            {
               if(templat && templat->type==Symbol::ENUM) // enums can't be forward declared (they can in VS but not in GCC)
               {
                  symbol.Parent()->addDependency(templat); // add dependency to "enum"
               }else
               for(Symbol *cur=value(); cur; cur=cur->parent()) // check if that template is used by class or any of its parents
                  if(cur->type==Symbol::CLASS)
                     if(cur->dependencies.has(src_template))
               {
                  symbol.Parent()->addDependency(templat); // add dependency to "int"
                  break;
               }
            }
         }
      }
   }

   // add dependencies from nested classes to their root
   REPA(active_symbols)
   {
      Symbol &symbol=*active_symbols[i];
      if(symbol.type==Symbol::CLASS)
         if(Symbol *root_class=symbol.rootClass())
            if(root_class!=&symbol)
               REPA(symbol.dependencies)
      {
         Symbol *dep=symbol.dependencies[i];
         if(dep!=root_class)root_class->dependencies.include(dep);
      }
   }

   // create list of global classes
   Memc<Symbol*> classes;
   REPA(active_symbols)
   {
      Symbol &symbol=*active_symbols[i];
      if(symbol.type==Symbol::CLASS && symbol.isGlobal())classes.add(&symbol); // global classes only
   }
   classes.sort(CompareName); // sort classes by name, to try generating them always in the same order
   for(;;)
   {
      Bool removed=false;

      // list classes ready to process
      REPA(classes)
      {
         Symbol &symbol=*classes[i];
         REPAD(d, symbol.dependencies)if(!(symbol.dependencies[d]->helper&Symbol::HELPER_PROCESSED) && symbol.dependencies[d]->type!=Symbol::TYPENAME)goto not_yet; // if at least one dependency not yet available then skip (here ignore typename dependencies)
         symbol.helper|=Symbol::HELPER_PROCESSED; // we list classes sequentially one-by-one (in 'to_process' container and later when writing it) so we can set HELPER_PROCESSED here already
         sorted_classes.add(&symbol); classes.remove(i); removed=true;
      not_yet:;
      }

      if(!removed)break; // if no class was processed in this step, then break the loop
   }
   FREPA(classes) // report all un-processed classes as errors
   {
      Symbol  &symbol=*classes[i];
      Message &msg   = msgs.New().error(S+"class \""+symbol.fullCppName()+"\" is dependent on following classes:", &symbol);
      FREPA(symbol.dependencies)
      {
         Symbol &dependency=*symbol.dependencies[i];
         if(   !(dependency.helper&Symbol::HELPER_PROCESSED) && dependency.type!=Symbol::TYPENAME)msg.children.New().set(dependency.fullCppName(), &dependency); // here ignore typename dependencies
      }
   }
   return !msgs.elms();
}
static Int CompareTokenIndex(Symbol*C &a, Symbol*C &b) {return Compare(a->token_index, b->token_index);}
void CodeEnvironment::SetupChildren()
{
   // delete children
   REPA(Symbols){Symbol &symbol=Symbols.lockedData(i); symbol.children.clear();}

   // setup children
   FREPA(Symbols)
   {
      Symbol &symbol=Symbols.lockedData(i);
      if(symbol.valid && symbol.parent && symbol.parent->type==Symbol::CLASS) // use direct parent (without skipping transparent classes), skip function params, templates and other useless stuff
         if((symbol.isVar() && !(symbol.modifiers&Symbol::MODIF_STATIC))                 // non-static variables
         || (symbol.type==Symbol::CLASS && (symbol.modifiers&Symbol::MODIF_TRANSPARENT)) // transparent classes
         || (symbol.type==Symbol::FUNC_LIST))                                            // func lists
            symbol.parent->children.add(&symbol);
   }

   // sort the children by the token index (to make sure that they are in the correct order)
   FREPA(Symbols)Symbols.lockedData(i).children.sort(CompareTokenIndex);
}
/******************************************************************************/
// TODO: call this for 32/64
static void SetArchitecture(Bool config_32_bit)
{
   PtrSize=(config_32_bit ? 4 : 8);
   Symbols( "intptr")->value=(config_32_bit ?  "int" :  "long");
   Symbols("uintptr")->value=(config_32_bit ? "uint" : "ulong");
}
/******************************************************************************/
void CodeEnvironment::del()
{
   main_thread.del();
   func_bodies.del();
   heap       .del(); heap_size=0;

   // set all token.parent of functions to the function (to remove any references for symbols from inside the function), set func.parsed to false
   FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.source && symbol.source->active && symbol.valid)symbol.clearBody(); symbol.raw_offset=-1;}

   // delete all symbols from functions from active sources (except function params "void func(int param) {}" and function templates "<TYPE> void func() {}")
   FREPA(CE.sources) // iterate all sources
   {
      Source &source=CE.sources[i];
      if(source.active)REPA(source.symbols) // iterate all symbols from active source
      {
         Symbol &symbol=*source.symbols[i];
         if(!(symbol.modifiers&Symbol::MODIF_FUNC_PARAM)) // skip function parameters
            if(Symbol *root_func=symbol.rootFunc()) // get root function of the symbol
               if(root_func!=&symbol) // if we're not processing the function itself
                  if(symbol.token_index > root_func->token_index) // if symbol is defined after the function definition (this will skip the function templates "<TYPE> void func() {}")
                     source.symbols.remove(i); // remove that symbol
      }
   }
}
Bool CodeEnvironment::create(Memc<Message> &msgs)
{
   del();
   //SetArchitecture(CE.config_32_bit);
   SetupChildren();

   Memc<Symbol*> sorted_classes;
   if(VerifySymbols(msgs, sorted_classes))
   {
      CompilerContext ctx(T);
      ctx.store_known_global_var_on_heap=true;

      // initialize the heap
      FREPA(Symbols)
      {
         Symbol &symbol=Symbols.lockedData(i);
         if(symbol.source && symbol.source->active && symbol.valid && symbol.isVar() && symbol.isGlobalOrStatic())
         {
            Int size=symbol.rawSize(true);
            symbol.raw_offset=AlignAddress(heap_size, symbol.firstMemberSize());
            heap_size=symbol.raw_offset+size;
         }
      }

      // allocate room for 2 functions (global var init and shut) so they are first on the list
      Int func_global_vars=func_bodies.addNum(2);

      // compile function bodies
      FREPA(Symbols)
      {
         Symbol &symbol=Symbols.lockedData(i);
         if(symbol.source && symbol.source->active && symbol.valid && symbol.isFunc() && (symbol.modifiers&Symbol::MODIF_FUNC_BODY))
         {
            symbol.raw_offset=func_bodies.elms();
            func_bodies.New().create(symbol, msgs, ctx);
         }
      }

      // initialize global variables
      createGlobalVarSetup(func_global_vars, msgs, ctx);

      // link function bodies
      REPAO(func_bodies).link(msgs, T);

      // check for errors
      Bool ok=true; REPA(msgs)if(msgs[i].type==Message::ERROR){ok=false; break;}
      if( !ok)
      {
         func_bodies.del();
         return false;
      }

      // create the heap
      heap.setNumZero(heap_size);

      // copy constants
      REPA(ctx.    constants){CompilerContext::Constant &c=ctx.    constants[i]; Copy(heap.data()+c.heap_offset, ctx.const_data.data()+c.const_offset, c.size);}
      REPA(ctx.var_constants){CompilerContext::Constant &c=ctx.var_constants[i]; Copy(heap.data()+c.heap_offset, ctx.const_data.data()+c.const_offset, c.size);}

      // optimize function bodies
      REPAO(func_bodies).optimize(msgs, T);

      // create the main thread
      main_thread.create(T);

      return true;
   }
   return false;
}
/******************************************************************************/
void CodeEditor::playEsenthelCompiler()
{
   Bool debug=CE.config_debug;

   CE.stopBuild ();
   CE.buildClear();
   CE.buildNew  ().set(S+"Compiling Project (using Esenthel compiler, "+(debug ? "Debug" : "Release")+" configuration)");
   CE.buildNew  ().set("-------------");

   Memc<Message>   msgs;
   CodeEnvironment CEnv; CEnv.debug=debug;
   Dbl compile_time=Time.curTime(); Bool ok=(CE.verifyBuildFiles(msgs) && CEnv.create(msgs));
       compile_time=Time.curTime()-compile_time;
   CE.buildNew(msgs.sort(Compare));
   CE.buildNew().set(S+"Codes compiled in "+Flt(compile_time)+"s");

   // TODO:
   if(ok)
      if(SymbolPtr main=FindSymbol("Main", null))if(main->type==Symbol::FUNC_LIST)
         REPA(main->funcs)if(Symbol *m=main->funcs[i]())
            if(!m->hasResult() && !m->params.elms() && InRange(m->raw_offset, CEnv.func_bodies))
   {
      Bool safe=true;
      Dbl  run_time=Time.curTime();
      if(safe && InRange(0, CEnv.func_bodies))safe=CEnv.main_thread.start(CEnv.func_bodies[            0]); // init global vars
      if(safe                                )safe=CEnv.main_thread.start(CEnv.func_bodies[m->raw_offset]); // func
      if(safe && InRange(1, CEnv.func_bodies))safe=CEnv.main_thread.start(CEnv.func_bodies[            1]); // shut global vars
      run_time=Time.curTime()-run_time;
      if(!safe)CE.buildNew().set(S+"Program triggered unhandled exception!");
      CE.buildNew().set(S+"Codes executed in "+Flt(run_time)+"s");
      break;
   }

   Int errors=0, warnings=0;
   REPA(msgs)switch(msgs[i].type)
   {
      case Message::ERROR  : errors  ++; break;
      case Message::WARNING: warnings++; break;
   }

   if(msgs.elms())CE.buildNew   ().set("-------------");
                  CE.buildNew   ().set(S+(ok ? "Success" : "Failed")+", errors: "+errors+", warnings: "+warnings);
                  CE.buildUpdate();
}
/******************************************************************************/
}}
/******************************************************************************/
