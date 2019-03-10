/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
DECLARE_CACHE(Symbol, Symbols, SymbolPtr); // 'Symbols' cache storing 'Symbol' objects which can be accessed by 'SymbolPtr' pointer
/******************************************************************************/
STRUCT(Symbol , Str) // C++ Symbol
//{
   enum TYPE : Byte
   {
      UNKNOWN  ,
      KEYWORD  , // built-in C++ keyword
      PREPROC  , // built-in C++ preprocessor keyword
      ENUM     , // enum X {..}
      ENUM_ELM , // enum .. {X}
      TYPEDEF  , // typedef .. X;
      NAMESPACE, // namespace X {..}
      CLASS    , // struct/class/union X {..}
      TYPENAME , // template<typename TYPE>
      FUNC_LIST, // list of polymorphic funcion symbols within the same symbol ('funcs' member is a list of them)
      FUNC     , // global function or class method or pointer to such
      VAR      , // member
      SPACE    , // braces {} used for separating local variables like: "Int x; {Int x;}"
      LABEL    , // label "label:" used together with 'goto'
   };
   enum MODIFIER // modifiers are designed to have all modifiers disabled by default, and they are only enabled if needed (m|=option), in some circumstances they are set FlagSet(m, option, on/off)
   {
      MODIF_REF             =1<< 0, //                TYPE& (this is set only to Symbol::value.modifiers, however this can be set to Symbol::modifiers in case of function references "void (&func)()")
      MODIF_DEF_VALUE       =1<< 1, //                TYPE= (if has default value)
      MODIF_STATIC          =1<< 2, //         static TYPE
      MODIF_INLINE          =1<< 3, //         inline TYPE
      MODIF_VIRTUAL         =1<< 4, //        virtual TYPE
      MODIF_FRIEND          =1<< 5, //         friend TYPE
      MODIF_MUTABLE         =1<< 6, //        mutable TYPE
      MODIF_EXPLICIT        =1<< 7, //       explicit TYPE
      MODIF_EXTERN          =1<< 8, //         extern TYPE
      MODIF_PRIVATE         =1<< 9, //        private TYPE
      MODIF_PROTECTED       =1<<10, //      protected TYPE
      MODIF_CONST_MEM_ADDR  =1<<11, // const_mem_addr TYPE

      MODIF_CTOR            =1<<12, // this function (FUNC_LIST and FUNC) is a constructor
      MODIF_DTOR            =1<<13, // this function (FUNC_LIST and FUNC) is a  destructor
      MODIF_FUNC_BODY       =1<<14, // this symbol is a function which contains a body (sometimes there can be 2 functions declaration/definition, only one of them has the body)
      MODIF_FUNC_CONST      =1<<15, // this symbol is a function which has 'const' modifier "void func()const {}" and cannot modify object members
      MODIF_FUNC_PARAM      =1<<16, // if this symbol is a function parameter "void func(int param)" <- 'param'
      MODIF_DATA_TYPE       =1<<17, // if is a data type (void, int, class, enum, typedef, template) this needs to be a member and not method because some KEYWORD's are a data type and some are not
      MODIF_UNION           =1<<18, // if class  is actually a union
      MODIF_NAMELESS        =1<<19, // if symbol is nameless
      MODIF_TRANSPARENT     =1<<20, // if symbol is nameless class and doesn't have any instances, for example "class {};" (doesn't have a name or any instances), following example is not transparent "class {} value;" (this is because class members can be accessed through "value."), this is also always set for FUNC_LIST symbols
      MODIF_SKIP_SUGGESTIONS=1<<21, // if skip this symbol from suggestions list
      MODIF_FOLLOW_BY_SPACE =1<<22, // if automatically follow by space when typing
      MODIF_OUTSIDE_METHOD  =1<<23, // this is a class method defined outside of class body (FUNC type) "class A {void func();}   void A::func() {}"
      MODIF_TYPEDEF         =1<<24, // for struct/class/union/enum: class A was defined with typedef before it "typedef class A {} B", for typedef: typedef B was defined directly after class definition "typedef class A {} B"
      MODIF_CPP_MODIFIER    =1<<25, // if the symbol is a C++ modifier (const, static, virtual, friend, mutable, explicit, ..)
      MODIF_ES_ONLY         =1<<26, // if the symbol is a keyword supported only in Esenthel Script (not pure C++)
      MODIF_ALL_UP_CASE     =1<<27, // if base name of the symbol is made completely out of upper case characters (used in suggestions)

      MODIF_SAME_FUNC      =MODIF_FUNC_CONST, // virtual should not be included
      MODIF_SAME_FUNC_PARAM=MODIF_REF,
      MODIF_ACCESS         =MODIF_PRIVATE|MODIF_PROTECTED,
      MODIF_CTOR_DTOR      =MODIF_CTOR|MODIF_DTOR,
      MODIF_VALUE          =MODIF_REF|MODIF_DEF_VALUE|MODIF_MUTABLE, // modifiers that can be used by Symbol::Modif::modifiers
   };
   enum HELPER // helpers used during processing only
   {
      HELPER_FORCE_CTOR      =1<<0, // force empty constructor for classes (with member initializers, used during creation of the headers)
      HELPER_PROCESSED       =1<<1, // used during C++ creation (declaration)
      HELPER_PROCESSED_FULL  =1<<2, // used during C++ creation (definition )
      HELPER_ADJUSTED_INDEXES=1<<3, // if symbol indexes have been already adjusted (used during header parsing)
      HELPER_FUNC_PARSED     =1<<4, // if function was already parsed
   };
   enum DIM_CODE // custom codes which can be used for 'array_dims'
   {
      DIM_MIN    =-2,
      DIM_PTR    =-2, // this specifies that this is actually a pointer, and not array dimension, this is used to specify pointers to arrays "int x[2]; &x" for "&x" symbol 'array_dims' would be set to {2, DIM_PTR}
      DIM_UNKNOWN=-1, // this specifies that dimension is not yet known, must be calculated from some formula like "const int x=5; x+2" or by counting number of default values "int x[]={1,2,3};"
      DIM_INVALID= 0, // this specifies that dimension is invalid      , for example "int x[0], y[-1], z[non_existing_+symbol];"
   };
   enum ACCESS_LEVEL : Byte
   {
      ACCESS_PRIVATE  ,
      ACCESS_PROTECTED,
      ACCESS_PUBLIC   ,
   };

   static ACCESS_LEVEL ModifToAccessLevel(UInt modifiers)
   {
      if(modifiers&MODIF_PRIVATE  )return ACCESS_PRIVATE  ;
      if(modifiers&MODIF_PROTECTED)return ACCESS_PROTECTED;
                                   return ACCESS_PUBLIC   ;
   }
   static UInt AccessLevelToModif(ACCESS_LEVEL level)
   {
      switch(level)
      {
         default              : return 0              ; // ACCESS_PUBLIC
         case ACCESS_PRIVATE  : return MODIF_PRIVATE  ;
         case ACCESS_PROTECTED: return MODIF_PROTECTED;
      }
   }

   static void SetPrivate  (UInt &modifiers) {modifiers&=~MODIF_ACCESS; modifiers|=MODIF_PRIVATE  ;}
   static void SetProtected(UInt &modifiers) {modifiers&=~MODIF_ACCESS; modifiers|=MODIF_PROTECTED;}
   static void SetPublic   (UInt &modifiers) {modifiers&=~MODIF_ACCESS;                            }

   STRUCT(Modif , SymbolPtr) // pointer to symbol with modifiers
   //{
      Byte        const_level   ; // bit mask where each bit specifies const for object or pointers (bit 0: "const TYPE obj", bit 1: "TYPE *const obj", bit 2: "TYPE * *const obj")
      SByte         ptr_level   , // amount of pointers "", "*", "**", ..
                  src_template_i;
      UInt        modifiers     ; // MODIFIER (when using this for expression calculation then don't include MODIFIER from the original Symbol but only its values, see more at MODIF_REF)
      Mems<Int>   array_dims    ; // array dimensions in reversed order, for example "int x[2][3];" would have array_dims={3,2}; (value can be also one of DIM_CODE codes) first element in 'array_dims' specifies the most low-level index in memory mapping, "int x[A:2][B:3]" stores elements in following linear mapping - A0B0, A0B1, A0B2,  A1B0, A1B1, A1B2
      Memc<Modif> templates     ; // contains a list of all known templates for this symbol, for example this which points to symbol 'D' of following formula "A<B,C>::D<E>" will have 3 elements: A.T0->B, A.T1->C, D.T0->E
      SymbolPtr   src_template  ; // pointer to symbol of TYPENAME type, for above example "A.T0->B" : 'src_template->A.T0' and 'this->B'

      void operator=(C SymbolPtr &sp     ) {SymbolPtr &t=T; t=sp;}
      void clear    (Bool clear_templates);

      Bool isConst  (); // test last pointer or array if it's const
      Bool isObj    (); // test if it's not pointer and not array
      Bool isPtr    (); // test if last pointer or array is pointer
      Bool isArray  (); // test if last array is not a pointer
      Bool anyPtr   (); // test if pointer or if any array has a pointer
      Bool isVoidPtr(); // test if this object is exactly a 'Ptr' or 'CPtr'
      Bool basicType(); // test if is a C++ native type or pointer of any kind

      Int  rawSize        (Bool ref_as_ptr_size, RecTest &rt=ConstCast(RecTest()));
      Int  firstMemberSize(                      RecTest &rt=ConstCast(RecTest())); // get size of first member in the class (or size of this if it's a var)
      Bool hasConstructor (                      RecTest &rt=ConstCast(RecTest()));
      Bool hasDestructor  (                      RecTest &rt=ConstCast(RecTest()));
      Bool constMemAddrOK (                                                      );
      Str  modifName      (Bool spaces=true, Bool include_const=true, Bool include_ref=true, Bool invalid_array_as_1=false, Bool template_space=false);

      Bool same              (Modif &modif, Bool test_const, Bool test_ref);
      Bool sameFuncParamValue(Modif &modif);

      void setConst      (Bool on=true);
      void proceedToFinal(Memc<Modif> *templates, Bool func_call=false, RecTest &rt=ConstCast(RecTest()));
      void proceedTo     (Modif &src            ,                       RecTest &rt=ConstCast(RecTest()));

      Bool save(File &f, StrLibrary &sl)
      {
         sl.putStr(f, name()); f.putMulti(const_level, ptr_level, modifiers);
         f.cmpUIntV(array_dims.elms()); FREPA (array_dims)f.cmpUIntV(((array_dims[i]>=DIM_MIN) ? array_dims[i] : DIM_INVALID)+DIM_MIN);
         f.cmpUIntV(templates .elms()); FREPAO(templates ).save(f, sl); sl.putStr(f, src_template.name());
         return true;
      }
      Bool load(File &f, StrLibrary &sl)
      {
         require(sl.getStr(f)); f.getMulti(const_level, ptr_level, modifiers);
         array_dims.setNum(f.decUIntV()); FREPA (array_dims)array_dims[i]=f.decUIntV()-DIM_MIN;
         templates .setNum(f.decUIntV()); FREPAO(templates ).load(f, sl); src_template=sl.getStr(f);
         return true;
      }

      Modif() {const_level=0; ptr_level=0; src_template_i=-1; modifiers=0;}
   };

   TYPE            type             ; // symbol type
   VAR_TYPE        var_type         ; // var    type
   ACCESS_LEVEL    access_level     ; // access level, used when parsing classes (struct will have ACCESS_PUBLIC, class will have ACCESS_PRIVATE), this may be changed when parsing class and encountering "private, protected, public" keywords inside
   Byte            level            , // amount of parents (symbol depth/level)
                   func_ptr_level   ,
                   class_pack       , // class member packing (1,2,4,8,16,..) setup with #pragma pack(..)
                   helper           ; // HELPER
   Short           valid            , // if main definition  still exists (number of main definitions )
                   valid_decl       ; // if main declaration still exists (number of main declarations)
   UShort          nameless_children; // amount of nameless children - {} spaces or nameless variables
   UInt            modifiers        ; // MODIFIER
   Str             full_name        ; // full name like "EE\Game\Chr" (not followed by separator)
   Memc<SymbolPtr> funcs            ; // if type==FUNC_LIST          , then 'funcs'      contains a list of polymorphic functions
   Memc<SymbolPtr> templates        ; // if type==FUNC || type==CLASS, then 'templates'  contains a list of template parameters for this class/func (of TYPE::TYPENAME type), for example: "template<typename TYPE> struct X{};" templates[0]=TYPE (this doesn't include templates of parents)
        Modif      value            ; // if type==FUNC || type==VAR  , then 'value'      is a type of the value (it's not a var by itself, Symbol points to data type), if type==TYPEDEF then 'value' points to the base type symbol
   Memc<SymbolPtr> params           ; // if type==FUNC               , then 'params'     contains a list of parameters for this func (of TYPE::VAR,FUNC type)
   Memc<SymbolPtr> ctor_inits       ; // if type==FUNC               , then 'ctor_inits' contains a list of specified constructor initializers, if type==CLASS then 'ctor_inits' contains a list of all members with default values
   Memc<Symbol*>   dependencies     , // used when generating list of all headers, this  contains a list of classes that current class depends on (all 'dependencies' must be included before this class in order to compile in C++) these are base classes, non-pointer non-ref members, and all the same for nested classes
                   children         ; // used during manual compilation,                 contains a list of all symbols which parent is 'this'
   Memc<Modif>     base             ; // base classes which this class extends, modifiers can be used to determine class access or template params
   SymbolPtr       parent           ; // parent of the symbol
   Int                  raw_offset  , // for global variables this is the memory position in the global heap, for local variables this is the offset in the local stack (relative to function entry), for function parameters this is the offset in the parameter stack, for functions this is the index of function body in the code environment, for labels this is the index of the label
                   nameless_index   ,
                      token_index   ; // index of token in source which defines the symbol
   VecI2               type_range   , // type describing the symbol start/end token indexes
                        def_range   , // symbol definition          start/end token indexes
                    def_val_range   ; // default value              start/end token indexes
   Source          *source          ; // source that this symbol belongs to

   Symbol()
   {
      type=UNKNOWN;
      modifiers=0;
      level=0;
      valid=valid_decl=0;
      token_index=0; source=null;
      clear();
   }

   void clear() // don't clear 'level', 'valid', 'valid_decl', 'token_index', 'source', 'type' (why 'type' not?)
   {
      var_type      =VAR_NONE;
        access_level=ACCESS_PUBLIC;
      func_ptr_level=0;
      class_pack    =4;

      nameless_children=0;

      helper=0;
      modifiers&=~MODIF_ALL_UP_CASE; // clear everything except 'MODIF_ALL_UP_CASE'

      funcs       .clear();
      templates   .clear();
      value       .clear(true);
      params      .clear();
      ctor_inits  .clear();
      dependencies.clear();
      children    .clear();
      base        .clear();
      parent      .clear();
      raw_offset    =-1;
      nameless_index= 0;
      type_range.set(0, -1); def_range.set(0, -1); def_val_range.set(0, -1);
   }

   // don't save/load ('valid', 'valid_decl' - managed by SymbolDef/SymbolDecl), ('name', 'full_name' - setup by load from name), ('access_level', 'nameless_children', 'raw_offset', 'nameless_index' - not needed)
   Bool save(File &f, StrLibrary &sl)
   {
      f.putMulti(type, var_type, level, func_ptr_level, class_pack, modifiers);

      f.cmpIntV(token_index).cmpIntV(type_range.x).cmpIntV(type_range.y).cmpIntV(def_range.x).cmpIntV(def_range.y); if(modifiers&MODIF_DEF_VALUE)f.cmpIntV(def_val_range.x).cmpIntV(def_val_range.y);

      value.save(f, sl);
      f.cmpUIntV(funcs     .elms()); FREPA(funcs     )sl.putStr(f, funcs     [i].name());
      f.cmpUIntV(templates .elms()); FREPA(templates )sl.putStr(f, templates [i].name());
      f.cmpUIntV(params    .elms()); FREPA(params    )sl.putStr(f, params    [i].name());
      f.cmpUIntV(ctor_inits.elms()); FREPA(ctor_inits)sl.putStr(f, ctor_inits[i].name());
      f.cmpUIntV(base      .elms()); FREPAO(base).save(f, sl);
      sl.putStr(f, parent.name());
      return true;
   }
   Bool load(File &f, StrLibrary &sl, Source &source)
   {
      f.getMulti(type, var_type, level, func_ptr_level, class_pack, modifiers);

      f.decIntV(token_index).decIntV(type_range.x).decIntV(type_range.y).decIntV(def_range.x).decIntV(def_range.y); if(modifiers&MODIF_DEF_VALUE)f.decIntV(def_val_range.x).decIntV(def_val_range.y);

      value.load(f, sl);
      funcs     .setNum(f.decUIntV()); FREPA(funcs     )funcs     [i]=sl.getStr(f);
      templates .setNum(f.decUIntV()); FREPA(templates )templates [i]=sl.getStr(f);
      params    .setNum(f.decUIntV()); FREPA(params    )params    [i]=sl.getStr(f);
      ctor_inits.setNum(f.decUIntV()); FREPA(ctor_inits)ctor_inits[i]=sl.getStr(f);
      base      .setNum(f.decUIntV()); FREPAO(base).load(f, sl);
      parent=sl.getStr(f);
      T.source=&source;
      return true;
   }
   Bool load(C Str &name) // this is always called at symbol creation in the cache
   {
      // setup params on symbol creation
      full_name=name;
      Str &s=T; s=GetBase(name);
      FlagSet(modifiers, MODIF_ALL_UP_CASE, AllUpCase(s));
      return true;
   }

   // get
   Bool operator==(CChar8 *name) {return Equal(T, name, true);} // force case-sensitive comparison
   Bool operator==(CChar  *name) {return Equal(T, name, true);} // force case-sensitive comparison

   VAR_TYPE         varType() {return    var_type ;}
   ACCESS_LEVEL accessLevel() {return access_level;}

   Str    typeName();
   Str    fileName(); // full name used for writing C++ files
   Str     cppName();
   Str fullCppName();
   Str   shortName(); // 'cppName' without the "operator" part for the operators

   Str funcDefinition  (Int highlight);
   Str     definition  ();
   Str     comments    ();
   Str     commentsCode();

   Symbol*    Parent          (); // return symbol's parent ignoring any transparent symbols (such as FUNC_LIST or TRANSPARENT class), which allows to return CLASS as parent for FUNC
   Symbol* rootClass          ();
   Symbol*     Class          ();
   Symbol* Namespace          ();
   Symbol*      func          ();
   Symbol*  rootFunc          ();
   Symbol* firstNonTransparent();
   Bool    contains           (Symbol *symbol);

   Token* getToken();

   Bool insideFunc         (); // if is inside another function, sample : void func() {   void inside1(){}   struct str{void inside2(){}}   }
   Bool insideClass        ();
   Bool insideTemplateClass();

   ACCESS_LEVEL highestAccess();
   Bool           fullyStatic();
   Bool       partiallyStatic();

   Bool isNativeFunc          ();
   Bool isGlobal              ();
   Bool isVar                 ();
   Bool isFunc                ();
   Bool isClassNonStaticMember();
   Bool isClassNonStaticFunc  ();
   Bool isGlobalFunc          ();
   Bool isGlobalAndStatic     ();
   Bool isGlobalOrStatic      ();
   Bool isTemplateClass       ();
   Bool isTemplateFunc        ();
   Bool isInlineFunc          ();
   Bool isAbstractClass       (              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class    is abstract
   Bool isVirtualClass        (              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class    is virtual
   Bool isVirtualFunc         (              Memc<Modif> *templates=null                                  ); // check if a function is virtual
   Bool hasVirtualDestructor  (              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class has a virtual destructor
   Bool hasVirtualFunc        (Symbol &func, Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class has a virtual function identical to 'func'
   Bool hasConstructor        (              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class has or should have constructor
   Bool hasDestructor         (              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // check if a class has or should have  destructor
   Bool hasResult             (              Memc<Modif> *templates=null, Modif   *value=null             ); // check if a function has result (return value) which is not 'void' (can be everything else, including 'void*')
   Bool fullyPublic           (); // if this symbol and all of its parents are public
   Int  templateClasses       ();
   Int  baseOffset            (Int     base_index     ,              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // get raw offset of the 'base_index' base class from the start of the object (this should support also the case when pointing after all bases, to return the offset of all bases)
   Int  memberOffset          (Symbol *member         , Bool *found, Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // get raw offset of the 'member'     member     from the start of the object (this should support alse the case when 'member' is not a child of the class, to return the size of the whole class)
   Int  rawSize               (Bool    ref_as_ptr_size,              Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest()));
   Int  firstMemberSize       (                                      Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest())); // get size of first member in the class (or size of this if it's a var)
   Int  realParams            (); // return 0 for operator++(Int), operator--(Int) and "params.elms()" for everything else

   Bool sameFunc  (Symbol &f); // compares return values and parameters only (ignores parents)
   Bool sameSymbol(Symbol &s); // if it's the same symbol (or a FUNC function identical to the 's' FUNC)

   Bool constDefineInHeader(); // if define const in header
   Bool fromPartialMacro   (); // if was created from a macro, and only from its part

   Bool hasBase          (Symbol *Class , Memc<Modif> *templates=null, RecTest &rt=ConstCast(RecTest()));
   Bool hasNonPrivateBase(Symbol *Class , Memc<Modif> *templates=null, Bool allow_self=true, Bool test_private=false, RecTest &rt=ConstCast(RecTest()));
   Bool hasPrivateBase   (Symbol *Class , Memc<Modif> *templates=null, Bool test_private=false, RecTest &rt=ConstCast(RecTest()));
   Bool hasNonPublicBase (Symbol *Class , Memc<Modif> *templates=null, Bool test_access =false, RecTest &rt=ConstCast(RecTest()));
   Bool isMemberOf       (Symbol *symbol, Memc<Symbol::Modif> &symbol_templates, Symbol *caller, Bool instance, Bool ctor_init, Bool allow_self, Bool allow_bases);
   Bool canBeAccessedFrom(Symbol *path  , Symbol *caller, Bool precise_parent, Memc<SymbolPtr> &usings);

   // operations
   void adjustIndex  (Memc<Token> &tokens, Int &i);
   void adjustIndexes(Memc<Token> &tokens        );

   void addDependency        (Modif  &symbol);
   void addDependencyIfNested(Symbol *symbol);
   
   void clearBody(); // set token parents of the wholy body of the function to the function itself (to remove any references from symbols created inside the function)
};
/******************************************************************************/
STRUCT(SymbolDecl , SymbolPtr) // symbol forward declaration (used only for classes)
//{
   SymbolDecl& set(C SymbolPtr &parent, Symbol::TYPE type, Int token_index, Source *source)
   {
      if(!T->valid) // allow modifying only once
      {
         T->clear();
         T->parent     =parent;
         T->type       =type  ;
         T->var_type   =((type==Symbol::ENUM || type==Symbol::ENUM_ELM) ? VAR_ENUM : VAR_NONE);
         T->level      =(parent ? parent->level+1 : 0);
         T->token_index=token_index;
         T->source     =source;
         FlagSet(T->modifiers, Symbol::MODIF_DATA_TYPE       , type==Symbol::CLASS || type==Symbol::ENUM || type==Symbol::TYPEDEF || type==Symbol::TYPENAME);
         FlagSet(T->modifiers, Symbol::MODIF_FOLLOW_BY_SPACE , type==Symbol::ENUM);
         FlagSet(T->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS, T->isFunc() || type==Symbol::SPACE || type==Symbol::LABEL);
         FlagSet(T->modifiers, Symbol::MODIF_TRANSPARENT     , type==Symbol::FUNC_LIST);
      }
      return T;
   }

   SymbolDecl& require(C Str &name)
   {
      super::require(name);
      T->valid_decl++;
      return T;
   }

  ~SymbolDecl()
   {
      if(T())
      {
         T->valid_decl--;
         if(!T->valid_decl && !T->valid)
         {
            T->clear();
         }
      }
   }
   SymbolDecl() {}
   NO_COPY_CONSTRUCTOR(SymbolDecl);
};
/******************************************************************************/
STRUCT(SymbolDef , SymbolPtr) // symbol definition (pointer to symbol which also acts as the creator of the symbol, by holding a 'valid' ref-count)
//{
   SymbolDef& set(C SymbolPtr &parent, Symbol::TYPE type, Int token_index, Source *source)
   {
      if(T->valid==1) // allow modifying only once
      {
         T->clear();
         T->parent     =parent;
         T->type       =type  ;
         T->var_type   =((type==Symbol::ENUM || type==Symbol::ENUM_ELM) ? VAR_ENUM : VAR_NONE);
         T->level      =(parent ? parent->level+1 : 0);
         T->token_index=token_index;
         T->source     =source;
         FlagSet(T->modifiers, Symbol::MODIF_DATA_TYPE       , type==Symbol::CLASS || type==Symbol::ENUM || type==Symbol::TYPEDEF || type==Symbol::TYPENAME);
         FlagSet(T->modifiers, Symbol::MODIF_FOLLOW_BY_SPACE , type==Symbol::ENUM);
         FlagSet(T->modifiers, Symbol::MODIF_SKIP_SUGGESTIONS, T->isFunc() || type==Symbol::SPACE || type==Symbol::LABEL);
         FlagSet(T->modifiers, Symbol::MODIF_TRANSPARENT     , type==Symbol::FUNC_LIST);
      }
      return T;
   }

   SymbolDef& require(C Str &name)
   {
      super::require(name);
      T->valid++;
      return T;
   }

  ~SymbolDef()
   {
      if(T())
      {
         T->valid--;
         if(!T->valid)
         {
            if(T->isFunc())
            {
               SymbolPtr func_list; if(func_list.find(GetPath(T->full_name)))REPA(func_list->funcs)if(func_list->funcs[i]==T())func_list->funcs.remove(i);
            }
            T->clear();
         }
      }
   }
   SymbolDef() {}
   NO_COPY_CONSTRUCTOR(SymbolDef);
};
/******************************************************************************/
Str SymbolToPath  (C Str &s);
Str   PathToSymbol(C Str &s);
Str SymbolGetBase (C Str &s);
Str SymbolGetPath (C Str &s);

Str NamelessName(Symbol *parent);

Symbol* GetFinalSymbol(Symbol *symbol, Memc<Symbol::Modif> *templates=null);

SymbolPtr FindChild (C Str &name, Symbol *parent, Memc<Symbol::Modif> *parent_templates=null, Bool allow_base=true, Bool allow_self=true, RecTest &rt=ConstCast(RecTest()));
SymbolPtr FindSymbol(C Str &name, Symbol *parent);
SymbolPtr FindSymbol(C Str &name, Symbol *parent, Memc<SymbolPtr> &usings);

void AddBaseTemplates(Memc<Symbol::Modif> &templates, Symbol &Class, RecTest &rt=ConstCast(RecTest()));
/******************************************************************************/
} // namespace
#endif
/******************************************************************************/
