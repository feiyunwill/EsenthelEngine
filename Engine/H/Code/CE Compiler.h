/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
enum CMD_TYPE : Byte
{
   CMD_NONE       ,
   CMD_INSTRUCT   , // raw instruction, sample: "int x=0;", "x+=15;", "obj.func(123);" - 'raw_range' specifies instruction range of tokens (excluding semicolon)
   CMD_GROUP      , // group of commands, sample; "{ .. }" - 'raw_range' specifies range of { .. } excluding brackets, 'cmds' specifies all commands inside
   CMD_IF         , // if(cond)true;else false; - 'cond_range' specifies range of tokens (excluding brackets), 'cmds' specifies all commands for true, 'cmds_false' specifies all commands for false
   CMD_FOR        , // for(init; cond; step)cmds; - 'init_range' specifies range of tokens (excluding semicolon), 'cond_range' specifies range of tokens excluding semicolon, 'step_range' specifies range of tokens, 'cmds' specifies all commands inside loop
   CMD_WHILE      , // while(cond)cmds; - 'cond_range' specifies range of tokens (excluding brackets), 'cmds' specifies all commands inside loop
   CMD_DO         , // do cmds while(cond); - 'cmds' specifies all commands inside loop, 'cond_range' specifies range of tokens (excluding brackets)
   CMD_SWITCH     , // switch(expr){cmds} - 'cmds' specifies all commands inside switch, 'raw_range' specifies range of tokens (excluding brackets)
   CMD_RETURN     , // return expr; - 'raw_range' specifies range of tokens after return (excluding semicolon)
   CMD_BREAK      , // break; - 'raw_range' specifies range of tokens after break (excluding semicolon)
   CMD_CONTINUE   , // continue; - 'raw_range' specifies range of tokens after continue (excluding semicolon)
   CMD_GOTO       , // goto label; - 'raw_range' specifies range of tokens after goto (excluding semicolon), 'label_index'=index of the label to jump to (-1 if unknown)
   CMD_GOTO_COND  , // if( cond)goto label; - 'cond_range' specifies range of tokens (excluding brackets), 'label_index'=index of the label to jump to (-1 if unknown)
   CMD_GOTO_COND_N, // if(!cond)goto label; - 'cond_range' specifies range of tokens (excluding brackets), 'label_index'=index of the label to jump to (-1 if unknown)
   CMD_LABEL      , // label: - 'raw_range' specifies range of tokens for label (excluding colon), 'label_index'=index of the label (-1 if unknown)
};
struct Command
{
   struct For
   {
      VecI2 cond_range, init_range, step_range;

      void reset() {cond_range.set(0,-1); init_range.set(0,-1); step_range.set(0,-1);}
   };

   CMD_TYPE      type;
   Memc<Command> cmds, cmds_false; // 'cmds_false' is used for "if(..).. else cmds_false;"
   union // all ranges may be invalid "max<min", ranges are described as "VecI2.x=min, VecI2.y=max"
   {
      VecI2 raw_range;
      For  _for      ;
   };
   Int label_index, // label index
       scope_label, // label index which specifies end of the block scope
       code_pos   ; // byte code raw position of the start of the command

   Int startTokenIndex(); // return the token index of the start of the command

   Command() {type=CMD_NONE; raw_range.set(0,-1); _for.reset(); label_index=scope_label=-1; code_pos=0;}
};
/******************************************************************************/
struct Message
{
   enum TYPE : Byte
   {
      NONE   ,
      WARNING,
      ERROR  ,
   };
   TYPE          type;
   Str           text;
   Token        *token;
   Source       *source;
   Memc<Message> children;

   Message& set(C Str &text,                            TYPE type=NONE) {T.type=type; T.text=text; T.token=                           null          ; T.source=                                                null ; return T;}
   Message& set(C Str &text, Token  *token ,            TYPE type=NONE) {T.type=type; T.text=text; T.token=                           token         ; T.source=((token && token->line) ? token->line->source : null); return T;}
   Message& set(C Str &text, Source *source, Int token, TYPE type=NONE) {T.type=type; T.text=text; T.token=(source ? source->getToken(token) : null); T.source=                                       source        ; return T;}
   Message& set(C Str &text, Symbol *symbol,            TYPE type=NONE) {return set(text, symbol ? symbol->source : null, symbol ? symbol->token_index : -1, type);}

   Message& error  (C Str &text                           ) {return set(text,                ERROR  );}
   Message& error  (C Str &text, Token  *token            ) {return set(text, token ,        ERROR  );}
   Message& error  (C Str &text, Source *source, Int token) {return set(text, source, token, ERROR  );}
   Message& error  (C Str &text, Symbol *symbol           ) {return set(text, symbol,        ERROR  );}
   Message& warning(C Str &text                           ) {return set(text,                WARNING);}
   Message& warning(C Str &text, Token  *token            ) {return set(text, token ,        WARNING);}
   Message& warning(C Str &text, Source *source, Int token) {return set(text, source, token, WARNING);}
   Message& warning(C Str &text, Symbol *symbol           ) {return set(text, symbol,        WARNING);}

   Message() {type=NONE; token=null; source=null;}
};
/******************************************************************************/
struct CompilerContext
{
   struct Constant
   {
      Int const_offset, // offset in the 'const_data'
           heap_offset, // offset in the 'heap'
                  size; // size of the data
   };
   Bool             store_known_global_var_on_heap;
   Memc<Constant>   constants, var_constants;
   Memc<Byte    >   const_data;
   CodeEnvironment &env;

   Int     heapConstant(CPtr data, Int size, Int align      ); // create new constant on the heap and return its raw offset in the heap
   void varHeapConstant(Int heap_offset, CPtr data, Int size); // set constant in the specified position of the heap

   explicit CompilerContext(CodeEnvironment &env) : env(env) {store_known_global_var_on_heap=false;}
};
/******************************************************************************/
enum COMPILE_RESULT
{
   COMPILE_FAILED , // no expression was returned
   COMPILE_PARTIAL, //    expression was returned, however there still remained others which weren't processed
   COMPILE_FULL   , //    expression was returned, all expressions were procesed and the returned is the only one that's left
};
struct Compiler // Function Compiler
{
   Bool               strict, // if strict compilation
                       quiet, // if silence all compilation messages
                  allow_cast, // if allow casting
            allow_func_lists; // if allow FUNC_LIST when calculating final
   Int                 final, // token index we want to calculate as final
                      labels, // current number of labels in a function
                   cmd_index, // current command index (increased during compilation)
                 scope_label, // index of the label at which current scope ends (changed during compilation)
                  stack_size, // current stack size (increased during compilation by local variables)
                   recursive; // recursive function depth during compiling
   Source            *source; // source file of the tokens to compile
   Memc<Local  >      locals; // all detected local variables (named and temporaries)
   Memc<Int    > live_locals; // index of locals that are live and need to be destroyed
   Memc<Message>       &msgs; // compilation messages
   Memc<Token* >     &tokens; // source tokens
   CompilerContext      *ctx;

   Compiler(Memc<Message> &msgs, Memc<Token*> &tokens, Source *source, CompilerContext *ctx) : msgs(msgs), tokens(tokens), source(source), ctx(ctx) {strict=true; quiet=false; allow_cast=true; allow_func_lists=false; final=-1; labels=0; cmd_index=0; scope_label=-1; stack_size=0; recursive=0; source=null;}

   Compiler& relax   (                                      ) {strict=false; return T;}
   Compiler& setFinal(Int token_index, Bool allow_func_lists) {T.final=token_index; T.allow_func_lists=allow_func_lists; return T;}

   void expand (Memc<Command> &src, Memc<Command> &dest, Int label_break, Int label_continue, Int label_return, Int label_scope);
   void compile(Memc<Command> &cmds, Mems<Byte> &code, Symbol &func, Symbol::Modif *result_value);

   Int     heapConstant(CPtr data, Int size, Int align      ); // create new constant on the heap and return its raw offset in the heap
   void varHeapConstant(Int heap_offset, CPtr data, Int size); // set constant in the specified position of the heap
   Int  newLocal       (Symbol::Modif &type, Bool block_scope, Token *token); // create new local variable of the 'type' type and return its index in the 'locals' container, 'token'=token at which the variable gets created
   void mapLocalInStack(Expr::Memory   &mem, Bool error_on_fail=true       ); // map the local variable in fixed stack position
   void mapVar         (Expr::Memory   &mem                                ); // map the variable if it's a local
   Bool unmappedLocal  (Expr::Memory   &mem                                );

   COMPILE_RESULT compileExpr  (Memc<Expr> &expr, Symbol *space, Expr &out); // compile series of expressions, 'space'=where are the expressions located, store the result in 'out'
   COMPILE_RESULT compileTokens(Int from, Int to,                Expr &out, Memc<Token*> *tokens=null);

   Bool appendTokens(Memc<Expr> &expr, Int from, Int to, Memc<Token*> *tokens=null); // add tokens in the range of 'from..to' to the 'expr' container, false on fail
};
/******************************************************************************/
//inline Bool ValueFitsInInstruction(Int size) {return size<=PtrSize && size<=SIZE(Ptr)*/;}  // check pointer size of target configuration (so it can fit in target variables) and current configuration (so it can fit in temporary variables used in compilation process)
  inline Bool ValueFitsInInstruction(Int size) {return size<=MEMBER_SIZE(Call::Param, raw);} // currently U64 is always used for 'raw' so it can always fit that kind of data

void ReadCommands(Source &source, Int &token_index, Symbol &set_parent, Memc<Command> &cmds, Memc<Message> &msgs); // recursive space assign + organize into commands

void CompileCommands(Source &source, Memc<Command> &cmds, Memc<Message> &msgs);

CChar8*    FindFuncCall(Call::Func  func);
Call::Func FindFuncCall(C Str &func_name);
Call::Func  GetFuncCall(C Str &func_name);
CPtr      FindGlobalVar(C Str & var_name);

Call::Func GetIgnoreCall       ();
Call::Func GetCallFunc         (Bool has_this, Bool has_result);
Call::Func GetIndirectionCall  ();
Call::Func GetAddressOfCall    ();
Call::Func GetOffsetCall       (Symbol::Modif &offset);
Call::Func GetAddPtrCall       (Symbol::Modif &offset);
Call::Func GetPtrAddCall       (Symbol::Modif &offset);
Call::Func GetPtrDiffCall      ();
Call::Func GetGotoCall         ();
Call::Func GetGotoCondCall     (Symbol::Modif &cond);
Call::Func GetGotoCondNCall    (Symbol::Modif &cond);
Call::Func GetSetConstCall     (Int size, Bool heap);
Call::Func GetSetAddrHeapCall  ();
Call::Func GetSetAddrStackCall ();
Call::Func GetSetAddrResultCall();

Str           FuncName(Symbol &func );
Str           DtorName(Symbol &Class);
Str NativeOperatorName(C Str &op, Symbol::Modif &result, Symbol::Modif &a                  ); // this function skips the 'const and '&' REF symbol for result and parameters
Str NativeOperatorName(C Str &op, Symbol::Modif &result, Symbol::Modif &a, Symbol::Modif &b); // this function skips the 'const and '&' REF symbol for result and parameters
/******************************************************************************/
} // namespace
/******************************************************************************/
Int Compare(C Edit::Message &a, C Edit::Message &b);
/******************************************************************************/
#endif
/******************************************************************************/
