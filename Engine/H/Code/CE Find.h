/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
STRUCT(Find , Region)
//{
   enum MODE
   {
      EXACT  ,
      NEAREST,
      FILES  ,
   };
   enum SCOPE
   {
      CUR_FILE=1<<0,
      ACT_APP =1<<1,
      ESENTHEL=1<<2,
      ALL     =CUR_FILE|ACT_APP|ESENTHEL,
   };

   STRUCT(ResultRegion , Region)
   //{
      struct Result
      {
         Bool         opened;
         Str          text;
         SourceLoc    source_loc;
         SymbolPtr    symbol;
         Int          priority, line;
         Memx<Result> children;

         Result() {opened=false; priority=0; line=-1;}

         Result& set    (Bool skip_ee, C Str &t, C SymbolPtr &symbol    , Int priority=0) {T.text=PathToSymbol(skip_ee ? SkipStartPath(t, "EE") : t); T.symbol=symbol; T.priority=priority; return T;}
         Result& setText(              C Str &t, C SourceLoc &source_loc, Int line   =-1) {T.text=                                                t ; T.source_loc=source_loc; T.line=line; return T;}
      };

      Memx<Result> data;
      List<Result> list;

              void create ();
              void resize ();
              void find   (C Str &t, Bool case_sensitive, Bool whole_words, MODE mode, UInt scope, Bool skip_ee);
              void setData();
      virtual void update (C GuiPC &gpc);
   };

   static Bool FilterScope(Source *source, UInt scope);

   Int          history_pos;
   Memc<Str>    history;
   TextLine     text;
   Button       prev, next, close, case_sensitive, whole_words, cur_file, active_app, engine, skip_ee;
   Tabs         mode;
   ResultRegion result;

   UInt scope()C {return (cur_file() ? CUR_FILE : 0)|(active_app() ? ACT_APP : 0)|(engine() ? ESENTHEL : 0);}

           void  create    ();
           void  resize    ();
           void  find      ();
           void  findPrev  ();
           void  findNext  ();
           void  toggle    ();
           void  historyAdd(C Str &text);
           void  historySet(Int delta);
   virtual Find& hide      ();
   virtual Find& show      ();
   virtual Find& activate  ();
   virtual Find& moveToTop () {result.moveToTop(); super::moveToTop(); return T;}
   virtual void  update    (C GuiPC &gpc);
};
/******************************************************************************/
STRUCT(ReplaceText , ClosableWindow)
//{
   Text   t_src, t_dest, t_scope;
   TextLine src,   dest;
   ComboBox scope;
   Button   case_sensitive, whole_words, replace, cancel;
   Menu     menu;

           void         create ();
           void         process();
   virtual ReplaceText& show   ();
};
/******************************************************************************/
void FindPrev(Find &find);
void FindNext(Find &find);
/******************************************************************************/
} // namespace
#endif
/******************************************************************************/
