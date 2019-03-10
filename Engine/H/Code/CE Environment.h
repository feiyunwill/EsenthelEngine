/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
#define CE_STACK_SIZE 1024*1024 // 1 MB
/******************************************************************************/
struct CodeEnvironment
{
   struct FuncBody
   {
      Int       stack_size; // size of stack that is required for this function
      Mems<Byte>      code; // function body codes
      Str8            name; // function name

      void zero    () {stack_size=0;}
      void del     ();
      void create  (Symbol &func, Memc<Message> &msgs, CompilerContext &ctx);
      void compile (Symbol &func, Memc<Message> &msgs, Memc<Command> &cmds, Memc<Byte> &code);
      void link    (              Memc<Message> &msgs, CodeEnvironment &ce);
      void optimize(              Memc<Message> &msgs, CodeEnvironment &ce);

      FuncBody() {zero();}
   };

   struct Thread
   {
      struct Level
      {
         FuncBody *func;
         Byte     *func_code_pos,
                  *func_param_stack,
                  *func_this,
                  *func_result;
      };

      Byte       *func_code_pos   , // code position
                 *func_code_end   , // code position end
                 *func_stack      , //           stack of the function
                 *func_param_stack, // parameter stack of the function
                 *func_this       , // this     object of the function
                 *func_result     , // result  address of the function
                 *heap            ; // environment heap
      FuncBody   *func            ; // in which function we're currently located
      Bool        debug           ; // if run in debug mode
      Mems<Byte>  stack           ;
      Memc<Level> levels          ; // call-stack

      void zero  () {func_code_pos=func_code_end=func_stack=func_param_stack=func_this=func_result=heap=null; func=null; debug=false;}
      void del   ();
      void create(CodeEnvironment &env);
      Bool test  ();
      Bool start (FuncBody &func);
      void call  (FuncBody &func, Int param_stack_offset, Ptr func_this, Ptr result);

      Thread() {zero();}
   };

   static Bool VerifySymbols(Memc<Message> &msgs, Memc<Symbol*> &sorted_classes);
   static void SetupChildren();

   Bool           debug;
   UInt           heap_size;
   Mems<Byte    > heap;
   Memc<FuncBody> func_bodies;
   Thread         main_thread;

   void createGlobalVarSetup(Int func_index, Memc<Message> &msgs, CompilerContext &ctx);

   void del   ();
   Bool create(Memc<Message> &msgs);

   CodeEnvironment() {debug=false; heap_size=0;}

   NO_COPY_CONSTRUCTOR(CodeEnvironment);
};
/******************************************************************************/
struct Call
{
   enum ADDR_TYPE : Byte
   {
      ADDR_STACK          , // address is relative to CodeEnvironment function           stack
      ADDR_STACK_REF      , // address is relative to CodeEnvironment function           stack (in that address there is Ptr that points to the actual element)
      ADDR_PARAM_STACK    , // address is relative to CodeEnvironment function parameter stack
      ADDR_PARAM_STACK_REF, // address is relative to CodeEnvironment function parameter stack (in that address there is Ptr that points to the actual element)
      ADDR_THIS           , // address is relative to CodeEnvironment function           this
      ADDR_THIS_REF       , // address is relative to CodeEnvironment function           this  (in that address there is Ptr that points to the actual element)
      ADDR_RESULT         , // address is relative to CodeEnvironment function           result
      ADDR_HEAP           , // address is relative to CodeEnvironment heap (used only during compilation, upon loading is replaced with ADDR_GLOBAL    )
      ADDR_HEAP_REF       , // address is relative to CodeEnvironment heap (used only during compilation, upon loading is replaced with ADDR_GLOBAL_REF) (in that address there is Ptr that points to the actual element)
      ADDR_GLOBAL         , // address is in global C++ space
      ADDR_GLOBAL_REF     , // address is in global C++ space (in that address there is Ptr that points to the actual element)
      ADDR_INSTRUCT       , // if constant value is encoded in the instruction itself, in the 'addr' member (can be used only for constants! which size is <= SIZE(Ptr))
   };

   struct Param
   {
      ADDR_TYPE type;
      union
      {
         struct
         {
            union
            {
               Ptr addr;
               Int offset;
            };
            Int offset2; // offset applied after getting the reference
         };
         U64 raw;
      };

      void set           (Expr &expr, Compiler &compiler, Bool auto_map=true);
      void setRef        (Expr &expr, Compiler &compiler);
      void setGlobal     (Ptr addr  , Int offset2, Bool ref);
      void setHeap       (Int offset, Int offset2, Bool ref);
      void setStack      (Int offset, Int offset2, Bool ref);
      void setParamStack (Int offset, Int offset2, Bool ref);
      void setThis       (Int offset, Int offset2, Bool ref);
      void setResult     (         );
      void setInstruct   (U64  raw );
      void setInstructI  (Int  raw );
      void setInstructP  (Ptr  addr);
      void setInstructPI2(Ptr  addr, Int i);
      Int  getInstructI  (         ) {return offset ;}
      Int  getInstructI2 (         ) {return offset2;}
      Ptr  getInstructP  (         ) {return addr   ;}

   #if 0 // this is slightly slower (4.768s vs 4.653s on big loop)
                      Ptr   address(CodeEnvironment::Thread &thread);
      T1(TYPE) INLINE TYPE& value  (CodeEnvironment::Thread &thread) {return *(TYPE*)address(thread);}
   #else // slightly faster
      T1(TYPE) TYPE& value  (CodeEnvironment::Thread &thread);
               Ptr   address(CodeEnvironment::Thread &thread);
   #endif
              CPtr&  ref    (CodeEnvironment::Thread &thread);
   };

   typedef void (*Func)(Call &call, CodeEnvironment::Thread &thread);

   static INLINE Int Size(Int params) {return SIZE(Call) + SIZE(Param)*params;}

          Func   func;
          Int    params;
   INLINE Param& param(Int i) {RANGE_ASSERT(i, params); return ((Param*)(this+1))[i];}
};
/******************************************************************************/
} // namespace
/******************************************************************************/
#endif
/******************************************************************************/
