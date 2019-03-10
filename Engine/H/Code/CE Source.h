/******************************************************************************/
namespace Edit{
/******************************************************************************/
enum ERROR_TYPE
{
   EE_ERR_NONE              ,
   EE_ERR_FILE_NOT_FOUND    ,
   EE_ERR_FILE_INVALID      ,
   EE_ERR_FILE_READ_ERROR   ,
   EE_ERR_ELM_NOT_FOUND     ,
   EE_ERR_ELM_NOT_CODE      ,
   EE_ERR_ELM_NOT_DOWNLOADED,
};
#if EE_PRIVATE

#define CODE_EXT "es|c|cpp|h|cs|cc|cxx|m|mm|java|txt|xml|htm|html|php|mk"
/******************************************************************************/
struct LineMode // Line information, which can start with being inside comment or not (2 modes)
{
   Bool             starts_with_comment, ends_with_comment, preproc, starts_with_preproc, ends_with_preproc, starts_with_macro_param;
   Memc<TOKEN_TYPE> type; // type of character
   Memc<Token     > tokens;

   void resetTokens() {REPAO(tokens).reset();}

   void       clear(     )  {type.clear(); tokens.clear();}
   TOKEN_TYPE Type (Int i)C {return InRange(i, type) ? TOKEN_TYPE(type[i]) : TOKEN_NONE;}

   void setTokens(Line &line);

   void resetType(Line &line);
   void   setType(Line &line, Bool starts_with_comment, Bool starts_with_preproc);

   Bool save(File &f, StrLibrary &sl, C Str &text)C;
   Bool load(File &f, StrLibrary &sl, C Str &text, Line &line, Str &temp);

   LineMode() {starts_with_comment=ends_with_comment=preproc=starts_with_preproc=ends_with_preproc=starts_with_macro_param=false;}
};
/******************************************************************************/
struct Line : Str, Text, LineMode
{
#if !WINDOWS
   typedef Text super;
#endif

   Bool        changed, text_valid, tokens_preproc_use, tokens_preproc_condition_unavailable;
   Int         line; // original line index
   UID         id;
   Source     *source;
   LineMode    comment_mode;
   Memc<Token> tokens_preproc;

   Memc<Token>& Tokens()  {return tokens_preproc_use ? tokens_preproc : tokens;}
 C Memc<Token>& Tokens()C {return tokens_preproc_use ? tokens_preproc : tokens;}

  ~Line();
   Line() {changed=true; text_valid=false; tokens_preproc_use=tokens_preproc_condition_unavailable=false; line=-1; id.randomize(); source=null;}

   void resetTokens (                       ) {LineMode::resetTokens(); comment_mode.resetTokens(); REPAO(tokens_preproc).reset();}
   void resetPreproc(Bool inside_macro_param) {text_valid=false; starts_with_macro_param=inside_macro_param; tokens_preproc_use=inside_macro_param; tokens_preproc.clear(); REPAO(tokens).macro=false;}

   void preprocChanged();

   void clear    (          ) {Str::clear(); LineMode::clear(); comment_mode.clear(); tokens_preproc.clear(); changed=true;}
   void operator=(C Str &src) {clear(); Str &s=T; s=src;}

   Char   first     ()C {return Str::first();}
   Char   last      ()C {return Str::last ();}
   CChar* operator()()C {return Str::operator()();}

   Line& clip      (Int length      ) {Str::clip(length);                changed=true; return T;}
   Line& clipSpaces(                ) {for(; last()==' '; )removeLast(); changed=true; return T;}
   Line& removeLast(                ) {return remove(length()-1);}
   Line& remove    (Int i, Int num=1)
   {
      if(num>0)
      {
                       Str::remove   (i, num);
             LineMode::type.removeNum(i, num, true);
         comment_mode. type.removeNum(i, num, true);
         changed=true;
      }
      return T;
   }
   Line& insert(Int i, C Str &text, TOKEN_TYPE type=TOKEN_NONE)
   {
      if(text.is())
      {
         Str::insert(i, text);
         REPAD(n, text)
         {
                LineMode::type.NewAt(i)=type;
            comment_mode. type.NewAt(i)=type;
         }
         changed=true;
      }
      return T;
   }
   Line& append(C Str &text, TOKEN_TYPE type=TOKEN_NONE) {return insert(length(), text, type);}
   Line& operator+=(Char8   c) {return append(c);}
   Line& operator+=(Char    c) {return append(c);}
   Line& operator+=(CChar8 *t) {return append(t);}
   Line& operator+=(CChar  *t) {return append(t);}

   Line& setChar(Int i, Char c, TOKEN_TYPE type=TOKEN_NONE)
   {
      if(InRange(i, T) && c)
      {
               Str::setChar(i, c);
             LineMode::type(i)=type;
         comment_mode. type(i)=type;
         changed=true;
      }
      return T;
   }

   void resetType();
   void   setType(Bool starts_with_comment, Bool starts_with_preproc);

   void setRect(Int i                );
   void setGui (Int i, GuiObj &parent);

   Int  end      (                    ) { REPA(                     T)if(T[i]!=' ')                return   i+1;                  return    0;}
   Int  start    (                    ) {FREPA(                     T)if(T[i]!=' ')                return     i;                  return    0;}
   Bool empty    (                    ) { REPA(                     T)if(T[i]!=' ')                return false;                  return true;}
   Char chrBefore(Int x, Int *pos=null) { REP (Min(length(), x)      )if(T[i]!=' '){if(pos)*pos=i; return  T[i];} if(pos)*pos=-1; return    0;}
   Char chrNext  (Int x, Int *pos=null) {for(Int i=x; i<length(); i++)if(T[i]!=' '){if(pos)*pos=i; return  T[i];} if(pos)*pos=-1; return    0;}
   Char chrFirst (       Int *pos=null) {return chrNext(0, pos);}
   Int  wordStart(Int x               );
   Int  wordEnd  (Int x               );
   Int  wordBegin(Int x               );

   Str textTokens()C;
   Str textCode  () ;

   virtual void draw(C GuiPC &gpc);

   Bool saveData(File &f)C;
   Bool loadData(File &f);

   Bool save(File &f, StrLibrary &sl)C;
   Bool load(File &f, StrLibrary &sl, Int line, Source &source, Str &temp);
};
/******************************************************************************/
struct PrepCond // Preprocess Conditional (#if, #ifdef, #ifndef, #elif, #else, #endif)
{
   Bool can_enter      , // if can enter a "#elif" or "#else" condition
        currently_valid; // if currently is inside a valid condition

   void set(Bool can_enter, Bool currently_valid) {T.can_enter=can_enter; T.currently_valid=currently_valid;}
};
/******************************************************************************/
const_mem_addr STRUCT(Source , Region)
//{
   enum UNDO_TYPE
   {
      DEFAULT ,
      INS_CHR ,
      DEL_CHR ,
      INS_CHRS,
   };

   struct UndoChange : _Undo::Change
   {
      File     data;
      VecI2    cur, sel;
      DateTime modify_time;

      virtual void create(Ptr source);
      virtual void apply (Ptr source);
   };

   struct Suggestion
   {
              Bool      is_macro;
              Byte      macro_params; // 0-no brackets "X", 1-has brackets but no params "X()", 2-has brackets and params "X(..)"
              Int       priority;
              Str       text, display, macro_def;
              SymbolPtr symbol;
              UID       elm_id;
              ImagePtr  icon;
      mutable Int      _order;

      Int order()C;

      void set(Int priority, C Str &text, Symbol &symbol             );
      void set(Int priority, C Macro &macro                          );
      void set(Int priority, C Str &text, C UID &id, C ImagePtr &icon); // project element
   };

   struct ViewLine : CodeLine, Text
   {
   #if !WINDOWS
      typedef Text super;
   #endif

      Bool    text_valid;
      Source *source;

      Int line   (       )C {return lines.y;} // get original line index (use Y because it can be -1)
      Int findCol(Int col)C {return findPos(VecI2(col, line()));}

      ViewLine(                ) {text_valid=false; source=null;}
      ViewLine(C ViewLine &line)
      {
         SCAST(CodeLine, T)=SCAST(C CodeLine, line);
         text_valid=line.text_valid;
         source    =line.source;
      }

      void setRect(Int i);

      Str textCode();

      virtual void draw(C GuiPC &gpc);
   };

   SourceLoc        loc; // how this source is stored
   Bool             active, was_active, header, ee_header, // if source is included in current build and symbols generation
                    cpp, // if source is C++ style file (not .es)
                    opened, Const;
   Int              highlight_line, recursive, parse_count, preproc_line_changed;
   Flt              highlight_time;
   Str              cur_text;
   VecI2            cur, sel, sel_temp;
   Vec2             lc_offset; // line column offset
   Undo<UndoChange> undos;
   Int              undo_original_state; // index of UndoChange that is currently saved on the disk
   VecI2            suggestions_pos;
   Region           suggestions_region;
   List<Suggestion> suggestions_list;
   Memc<Suggestion> suggestions;
   TextLine         suggestions_textline; // for project elements
   DateTime         modify_time;
   Bool             view_comments, view_funcs, view_func_bodies, view_private_members;
   Memc<ViewLine  > view_lines; // 
   Memx<Line      > lines;      // Memx because of Gui Text inside which requires const_mem_addr
   Memc<SymbolDef > symbols;    // symbol         definitions
   Memc<SymbolDecl> decls;      // symbol forward declarations
   Memc<Token*    > tokens;
   WindowIO        *win_io_save; // WinIO used only for saving
   Symbol          *lit_symbol; // highlight occurences of this symbol when drawing
   Str              lit_symbol_cpp_name;

  ~Source();
   Source();

   // io
   Bool save(File &f, StrLibrary &sl)C;
   Bool load(File &f, StrLibrary &sl, Str &temp);

   ERROR_TYPE load   ();
   void     reload   ();
   ERROR_TYPE load   (C SourceLoc &loc );
   Bool       save   (C SourceLoc &loc );
   Bool       saveTxt(C Str       &name);
   void       save   ();
   Bool     overwrite();

   // get
   Bool hasUnicode  ()C;
   Bool used        ()C;
   Bool modified    ()C;
   Bool hasFocus    (GuiObj *go)C;
   Bool hasMsFocus  ()C {return hasFocus(Gui.ms());}
   Bool hasKbFocus  ()C;
   Bool isCurVisible()C;

   Token*     getToken         (Int token_index)  {return InRange(token_index, tokens) ? tokens[token_index] : null;}
   Char       operator[]       (C VecI2 &pos   )C {return InRange(pos.y, lines) ? lines[pos.y]     [pos.x] :          0;}
   TOKEN_TYPE Type             (C VecI2 &pos   )C {return InRange(pos.y, lines) ? lines[pos.y].Type(pos.x) : TOKEN_NONE;}
   Bool      lineValid         (C VecI2 &pos   )C {return InRange(pos.y, lines);}
   Bool       posValid         (C VecI2 &pos   )C {return InRange(pos.y, lines) && InRange(pos.x, lines[pos.y]);}
   Vec2       posVisual        (C VecI2 &pos   )C;
   Bool       insideRSTBrackets(C VecI2 &pos   ) ; // if inside Round() Square[] Template<> Brackets
   Vec2       posCur           (C Vec2  &pos   )C; // convert screen position to cursor
   Vec2       offset           (               )C; // get offset applied to code text
   Int        findLine         (C UID   &id    )C {REPA(lines)if(lines[i].id==id)return i; return -1;}

   Bool  viewToReal(C VecI2 &view,   VecI2 &real)C; // conversion is precise
   Int   viewToReal(  Int    view               )C; // conversion is precise
   Int   realToView(                 Int    real)C; // conversion is approximate
   VecI2 realToView(               C VecI2 &real)C; // conversion is approximate

   Str textTokens()C;

   // operations
   void replacePath(C Str &src, C Str &dest);

   void setScroll();
   void setHideSlideBar();

   void curSel(VecI2 &min, VecI2 &max);

   VecI2& dec(VecI2 &p);
   VecI2& inc(VecI2 &p);

   // edit
   void startSel();

   void highlight(Int line, Bool immediate);

   void makeCurVisible(Bool center=false, Bool immediate=true);

   void curLeft     ();
   void curRight    ();
   void curUp       ();
   void curDown     ();
   void curLineBegin();
   void curLineEnd  ();
   void selAll      ();
   void selWord     ();

   void curPrevWord();
   void curNextWord();

   void curPrevBracket();
   void curNextBracket();

   void curPrevLevelBracket();
   void curNextLevelBracket();

   void curPageUp  ();
   void curPageDown();

   Int viewBeginPos();
   Int viewEndPos  ();

   void curViewBegin();
   void curViewEnd  ();

   void curDocBegin();
   void curDocEnd  ();

   void curClip();

   void removeLine(Int i);

   void delSel(Bool set_undo=true, Bool clear_suggestions=true);
   void cut   ();
   void copy  ();
   void paste (C Str *text=null, Bool move_cur=true);
   void separator();

   void delForward();
   void delBack();

   void delWordForward();
   void delWordBack   ();

   void makeCase(Bool upper);

   void forceCreateNextUndo();
   void delUndo();
   void setUndo(UNDO_TYPE undo_type=DEFAULT);
   void undoAsChange();
   void undo();
   void redo();

   Str    asText()C;
   void fromText(C Str &data);

   // preprocessor
 /*Memc<Macro>& macrosForLine(Int y)
   {
      for(y--; InRange(y, lines); y--)if(lines[y].has_macros)return lines[y].macros; // find first previous line which has macro definitions
                                                             return   ProjectMacros; // if not found then use global macros
   }*/
   Token* previewNextToken(Int  line_index, Int  line_token_index);
   Token*     getNextToken(Int &line_index, Int &line_token_index, Bool only_cur_line=false);

   struct TokenSource
   {
      Memc<Token> *tokens;
      Int          token_index; // index in 'tokens'
      Source      *source;
      Int         *line_index,
                  *line_token_index; // index in 'Line.tokens'

      Token* previewNext   ();
      Token* next          (Bool only_cur_line=false);
      void   removeLastRead();

      TokenSource() {tokens=null; token_index=0; source=null; line_index=null; line_token_index=null;}
   };

   Int findMacro         (Memc<Macro> &macros, BStr &macro,                  Mems<Bool> *macro_used=null);
   Int findMacroToReplace(Memc<Macro> &macros, BStr &macro, TokenSource &ts, Mems<Bool> *macro_used=null);
   Int findMacroToReplace(Memc<Macro> &macros, BStr &macro, Int line_index, Int line_token_index);

   struct MacroParam
   {
      Memc<Token> tokens;
   };

   void replaceMacro(Int macro_index, Memc<Macro> &macros, TokenSource &ts, Mems<Bool> &macro_used, Int depth, Int col, Line &line);

   Bool getConditionalValue(Memc<Token> &tokens, Memc<Macro> &macros, Line &line);

   void preprocess(Memc<Macro> &macros, Int &line_index, Memc<Token*> &temp, Bool allow_defines, Memc<PrepCond> &prep_if); // check for special preprocessor tokens (#define, #undef, #if, #ifdef, #ifndef, #else, #elif, #endif)

   void detectDefines();
   void preprocess(Int from=0, Int num=-1);

   //
   void setTokenPtrs(); // set continuous tokens for the whole file
   void changed     (Int from, Int num=1);
   void exist       (Int x, Int y);

   Token* findPrevToken(C VecI2 &pos, Int &i);
   Token* findToken    (C VecI2 &pos, Int &i);

   // expression
   Bool    evaluateSymbol(Int start, Expr &out, Int final=-1, Bool allow_func_lists=false); // returns true if 'out' expression was returned
   Symbol*    finalSymbol(Int final,                          Bool allow_func_lists=false);

   // suggestions
   void   clearSuggestions       ();
   void    listSuggestions       (Int force=0); // -2=project element, -1=false, 0=autodetect, 1=true
   void refreshSuggestions       ();
   void        suggestionsSetRect();
   void     setSuggestion        (Int x);
   void      autoComplete        (Bool auto_space=true, Bool set_undo=true, Bool auto_brace=true, Bool call_changed=true, Bool ignore_params=false);

   // detect
   Bool getSymbolMacroID (C VecI2 &cur, SymbolPtr &symbol_ptr, Macro* &macro_ptr, UID &id, VecI2 *x_range=null, Bool prefer_definition=false);
   void jumpToCur        ();
   void findAllReferences();

   // gui
   void visible       (Bool visible) {visibleActivate(visible);}
   Bool visible       (            ) {return  super::visible();}
   void setOffset     ();
   void setRegionSize ();
   void resize        ();
   void themeChanged  ();
   void setGui        ();
   void prepareForDraw();
   void validateView  ();
   void setView       (Bool comments, Bool funcs, Bool func_bodies, Bool private_members);

   virtual GuiObj& hide  (            ) {clearSuggestions(); return super::hide();}
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
   virtual void    update(C GuiPC &gpc);

   // draw
           void drawSelection(C Color &color, Int y, Int min_x, Int max_x);
           void drawSelection(C Color &color, C VecI2 &a, C VecI2 &b, Bool including=false);
   virtual void draw         (C GuiPC &gpc);

   // parse
   Symbol* getFullSymbol(Int &i, Str &temp, Symbol *set_parent, Memc<Symbol::Modif> &templates, Symbol* *symbol_parent=null, Int *symbol_index=null) {return GetFullSymbol(tokens, i, temp, set_parent, templates, symbol_parent, symbol_index);}

   Symbol* createSpace(Symbol &parent, Int token_index) {return symbols.New().require(parent.full_name+SEP+NamelessName(&parent)).set(&parent, Symbol::SPACE, token_index, parent.source)();}

   void   delSymbols();
   void resetSymbols() {if(!header)delSymbols();}

   void detectDataTypes(SymbolPtr parent=null) {if(active)DetectDataTypes(symbols, decls, tokens, parent);}
   void   linkDataTypes(                     ) {if(active)  LinkDataTypes(symbols,        tokens        );}
   void  detectVarFuncs(                     ) {if(active) DetectVarFuncs(symbols,        tokens        );}

   Bool isDeclaration(Symbol *symbol, Int i) {return IsDeclaration(tokens, symbol, i);}

   void parseFunc   (Symbol  &func, Memc<Command> &cmds, Memc<Message> &msgs); // parse specified 'func' function and setup 'cmds' and 'msgs'
   void parseFunc   (Symbol  &func ); // parse specified 'func' function
   void parseFunc   (Token   &token); // parse function at 'token' token
   void parseFunc   (C VecI2 &cur  ); // parse function at 'cur' cursor position
   void parseCurFunc(              ); // parse function at cursor position

   // C++
   void  srcTokenRange(                        Int t_start, Int t_end, VecI2 &start, VecI2 &end);
   Bool destTokenRange(Memc<CodeLine> &clines, Int t_start, Int t_end, VecI2 &start, VecI2 &end);

   Str getText(Int start, Int end);

   void writeAll(  Str       &out   , C VecI2 &start, C VecI2 &end);

   void write(     Str       &out   , C VecI2 &start, C VecI2 &end);
   void write(Memc<CodeLine> &clines, C VecI2 &start, C VecI2 &end, VecI2 *clines_start=null, Int *line_i=null);
   void write(Memc<CodeLine> &clines,   Int    start,   Int    end, VecI2 *clines_start=null, Int *line_i=null);

   void writeTokens(CodeLine &cline, Int start, Int end, Bool gcc);

   void writeSymbolDecl(Memc<CodeLine> &clines, Symbol &symbol, Bool gcc);

   void remove      (Memc<CodeLine> &clines, Int start, Int end, Bool definite);
   void removeDefVal(Memc<CodeLine> &clines, Symbol &symbol);

   Int getSymbolStart(Int i);
   Int getListEnd    (Int i);
   Int getBodyStart  (Int i);
   Int getBodyEnd    (Int i);

   Bool isFirstVar(Symbol &symbol);

   CChar8* adjustDot  (                            Int i          ); // replace '.' with "->" or "::", returns string only if change is needed
   void    adjustToken(Memc<CodeLine> &code_lines, Int i, Bool gcc); // adjust token from Esenthel Script to C++, 'gcc'=if should be compatible with GCC

   void writeClassPath(CodeLine &line, Int col, Symbol *parent, Symbol *cur_namespace, Bool global, Memc<Symbol::Modif> *templates=null, bool start_separator=true);

   void expandName        (     CodeLine  &line, Int col, Symbol &symbol, Symbol *parent, Symbol *cur_namespace);
   void expandName        (Memc<CodeLine> &code_lines, Symbol &symbol, Symbol *cur_namespace);
   void expandRetVal      (Memc<CodeLine> &code_lines, Symbol &symbol);
   Bool expandableTypename(                            Symbol &symbol);
   Bool expandableTemplate(Int token_index);
   void expandTypename    (Memc<CodeLine> &code_lines, Symbol  &symbol, Int start_line=0);
   void expandTemplate    (Memc<CodeLine> &code_lines, Int token_index, Int start_line=0);
   void expandTemplates   (Memc<CodeLine> &code_lines);

   void writeClassTemplates(Memc<CodeLine> &clines, Symbol *Class);
   void writeCtorInit      (Memc<CodeLine> &clines, Symbol &ci   , Int &line_i, Bool first, Bool gcc);
   void writeCtorInits     (Memc<CodeLine> &clines, Symbol &func , Int  body_start, Bool gcc);
   void writeForcedCtor    (Memc<CodeLine> &clines, Symbol &Class, Symbol* &Namespace, Bool gcc);

   void detectDefaultCtors();
   Bool writeClass     (Memc<CodeLine> &clines, Symbol &symbol, Bool gcc);
   Bool writeVarFuncs  (Memc<CodeLine> &clines,                 Bool gcc);
   Bool writeInline    (Memc<CodeLine> &clines,                 Bool gcc);
   Bool writeFunc      (Memc<CodeLine> &clines, Bool gcc, Symbol &symbol, Symbol* &Namespace);
   Bool writeStaticVars(Memc<CodeLine> &clines, Bool gcc, Symbol* &Namespace, Bool templates);

   void makeCPP(C Str &path, C Str &file, Bool gcc, Bool include_headers);
};
#endif
/******************************************************************************/
} // namespace
/******************************************************************************/
