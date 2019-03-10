/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
enum CAST_MATCH
{
   CAST_NONE      , // can't be casted
   CAST_CTOR_CONST, // after converting basic data using constructor parameter with different const
   CAST_CTOR      , // after converting basic data using constructor parameter
   CAST_CONV_CONST, // after conversion (possibly losing precision/data) with different const
   CAST_CONV      , // after conversion (possibly losing precision/data)
   CAST_MAX_CONST , // direct with different const
   CAST_MAX       , // direct
};
/******************************************************************************/
struct Local // function local variable
{
   Symbol::Modif       type; // final type of the variable
   Bool         block_scope, // this variable has block scope
                 force_heap; // force creation on the heap
   Int         stack_offset, // offset of the variable in function stack (in bytes, -1=not yet initialized) (or offset in the heap for 'force_heap'==true)
                scope_start, // index of command where the local is being created
                scope_label; // index of label where scope of this variable ends
   Token             *token; // token at which the variable gets created

   Local() {block_scope=false; force_heap=false; stack_offset=-1; scope_start=scope_label=-1; token=null;}
};
/******************************************************************************/
STRUCT(Expr , BStr)
//{
   enum FUNC_CALL_MASK
   {
      FC_RESULT=0x1,
      FC_PARENT=0x2,
   };

   struct Memory // use 64-bit values to support 64-bit compilation
   {
      enum TYPE : Byte
      {
         NONE  , // memory address is not known
         GLOBAL, // global or static symbol
         LOCAL , // function local non-static variable
         PARAM , // function parameter
         THIS  , // function 'this' caller/parent/object
         RESULT, // function result
         KNOWN , // the value is known at compile-time and doesn't need to be stored in the memory
      };

      TYPE    type   ;
      Symbol *symbol ; // pointer to the symbol (if it's not a temporary)
      Long    index  , // GLOBAL: raw offset in the heap, LOCAL: index of the variable in the 'locals' memory container, PARAM: raw offset in the parameter stack, THIS: unused, RESULT: unused, KNOWN: unused
              offset , // offset from the start of the symbol (applied for all types)
              offset2; // offset applied after getting the reference

      void clear() {type=NONE; symbol=null; index=-1; offset=offset2=0;}

      void setGlobal(Symbol &symbol ) {T.type=GLOBAL; T.symbol=&symbol; T.index=symbol.raw_offset;}
      void setParam (Symbol &symbol ) {T.type=PARAM ; T.symbol=&symbol; T.index=symbol.raw_offset;}
      void setLocal (Symbol &symbol ) {T.type=LOCAL ; T.symbol=&symbol; T.index=symbol.raw_offset;} // 'symbol' is already assumed to have its 'raw_offset' set to locals
      void setLocal (Int local_index) {T.type=LOCAL ; T.symbol= null  ; T.index=local_index      ;} // set layout for 'local_index' local variable
      void setThis  (               ) {T.type=THIS  ;}
      void setResult(               ) {T.type=RESULT;}
      void setKnown (               ) {T.type=KNOWN ;}

      void addOffset(Long offset, Bool ref) {if(ref)T.offset2+=offset;else T.offset+=offset;}

      Memory() {clear();}
   };

   Bool          instance      , // if the expression contains an actual value (this can be constant bool 'false', number '2', character 'a', variable/object 'x', function pointer 'func' YES in C++ function can be casted to pointers and this is correct), this is FALSE for classes, namespaces, etc.
                 final         , // if from the whole array of expressions this is the only one that we care about calculating
                 _operator     , // if expression is an operator
                 separator     , // if expression operator is a separator '.', '->', '::'
                 l_to_r        , // operator associativity (true="left to right", false="right to left")
                 temporary     , // if the expression is a temporary, this can be constant bool 'false', number '2', character 'a', text "abc", any kind of pointer, or object returned by func/operator/ctor, this currently prevents address-of & operator only
                 conditional   ; // if this expression is not always executed (for example "a ? b : c" - 'b' and 'c' are conditionals, "a && b" - 'b' is conditional, "a || b" - 'b' is conditional)
   Byte          func_call_mask,
                 priority      ; // operator priority
   Symbol::Modif symbol        ;
   Symbol       *symbol_origin ;
   Mems<Expr>    parent, func, params;
   Token        *origin        ;
   Call::Func    func_call     ;
   Memory        mem           ;

   Bool  possibleInstanceOrDataTypeOrNativeFunc();
   Bool  known        () {return mem.type==Memory::KNOWN;}
   Bool  knownFinal   (Bool allow_func_lists);
   Bool  anyInstance  () {return instance || instanceChild();} // if is instance or child of an instance
   Bool  instanceChild(); // if is a child of other variable (has 'instance' parent)
   Bool  fullTemporary(); // if is a temporary and not a child of other variable (not instanceChild)
   Bool  basicType    () {return symbol.basicType();}
   Expr* firstInstance();

   Bool       asBool (Compiler &compiler);
   Int        asInt  (Compiler &compiler);
   UInt       asUInt (Compiler &compiler);
   Long       asLong (Compiler &compiler);
   ULong      asULong(Compiler &compiler);
   Flt        asFlt  (Compiler &compiler);
   Dbl        asDbl  (Compiler &compiler);
   Char       asChar (Compiler &compiler);
   Char8      asChar8(Compiler &compiler) {return Char16To8Fast(asChar(compiler));} // we can assume that Str was already initialized
   Memc<Char> asText (Compiler &compiler); // don't operate on Str to allow '\0' characters inside
   U64        asRaw  (Compiler &compiler); // use U64 to fit all types of variables

   Expr& setOperator(CChar   *op    ); // will be set as 'borrowed'
   Expr& setBasic   (VAR_TYPE type  );
   Expr& setPtr     (               );
   Expr&  toSymbol  (Symbol  *symbol, Bool func_call=false);

   Bool setThis(Symbol *caller, Token *origin, Compiler &compiler);

   void setPriority   ();
   void setConditional();
   void setBlockScope (Compiler &compiler, Bool force_heap=false);

   void memAuto  (                Compiler &compiler); // try to set memory layout from existing symbol
   void memNew   (Bool    known , Compiler &compiler); // create new memory layout on stack or as a known constant
   void memFrom  (Memory &mem   , Compiler &compiler); // set from 'mem'
   void memOffset(Long    offset                    ); // apply memory offset to 'mem'

   Bool       castToBase       (Symbol::Modif &dest);
   Bool       castTo           (Symbol::Modif &dest,                                 Compiler &compiler);
   Bool       castTo           (Symbol        *dest, Memc<Symbol::Modif> &templates, Compiler &compiler);
   CAST_MATCH canBeCastedTo    (Symbol        *dest, Memc<Symbol::Modif> &templates, Compiler &compiler);
   Bool       castToConditional();
   Bool       castParentToSymbolClass(Compiler &compiler);

   Bool addressOf    (Compiler &compiler);
   Bool indirection  (Compiler &compiler);
   Bool copyCtor     (Compiler &compiler);
   void proceedToBase(Int base_index);

   void create  (Token &src, Compiler &compiler, Bool final=false);
   void copyThis(Expr &dest); // copy all members except 'parent', 'func', 'params'

   void calculateKnown(Expr &op, Expr &a         , Compiler &compiler);
   void calculateKnown(Expr &op, Expr &a, Expr &b, Compiler &compiler);

   CAST_MATCH calculate(Compiler &compiler);

   Expr() {instance=final=_operator=separator=l_to_r=temporary=conditional=false; func_call_mask=priority=0; symbol_origin=null; origin=null; func_call=null;}
};
/******************************************************************************/
struct FuncMatch
{
   Symbol *func;
   Int     average_match,
            lowest_match;

   void create(Symbol *func, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match=false);

   FuncMatch() {func=null; average_match=lowest_match=0;}
};
/******************************************************************************/
void AddMatch  (Memc<FuncMatch> &matches,      Symbol     *func , Bool l_to_r, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match=false);
void AddMatches(Memc<FuncMatch> &matches, Memc<SymbolPtr> &funcs, Bool l_to_r, Expr *param, Int params, Memc<Symbol::Modif> &templates, Compiler &compiler, Bool extra_match=false);
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int CompareAverage(C Edit::FuncMatch &a, C Edit::FuncMatch &b) {  return Compare(b.average_match, a.average_match);                                      } // compare in reversed order so functions with greatest match are listed first (this is the approximate compare used for visual suggestion which function should be best match)
inline Int CompareLowest (C Edit::FuncMatch &a, C Edit::FuncMatch &b) {if(Int c=Compare(b. lowest_match, a. lowest_match))return c; return CompareAverage(a, b);} // compare in reversed order so functions with greatest match are listed first (this is the exact       compare used for compilers), use average comparison as second alternative in case there are multiple choices for lowest match (useful for auto-complete suggestions)
/******************************************************************************/
#endif
/******************************************************************************/
