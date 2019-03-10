/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
#define RESULT_SEPARATOR "-------------------------------------------"
/******************************************************************************/
Bool Find::FilterScope(Source *source, UInt scope)
{
   //Find::OPENED : return source->opened;
   if(source)return (scope&CUR_FILE) && CE.cur()==source
                 || (scope&ACT_APP ) && source->used() && !source->ee_header
                 || (scope&ESENTHEL) &&                    source->ee_header;
   return false;
}
/******************************************************************************/
static Bool FilterText(C Str &t, Memc<Str> &texts, Bool case_sensitive, Bool whole_words)
{
   REPA(texts)if(!TextPos(t, texts[i], case_sensitive, whole_words))return false;
   return true;
}
/******************************************************************************/
static void FindChanged(Find &find) {find.find    ();}
static void FindClose  (Find &find) {find.hide    ();}
       void FindPrev   (Find &find) {find.findPrev();}
       void FindNext   (Find &find) {find.findNext();}
/******************************************************************************/
void Find::find()
{
   result.find(visible() ? text() : S, case_sensitive(), whole_words(), (MODE)mode(), scope(), skip_ee());
}
/******************************************************************************/
void Find::findPrev()
{
   if(CE.cur())CE.cur()->clearSuggestions();
   Memt<VecI2> res;
   if(text().is())
   {
      Memc<Source*> sources; Int offset=CE.curI(); FREPA(CE.sources){Source &s=CE.sources[Mod(i+offset, CE.sources.elms())]; if(FilterScope(&s, scope()))sources.add(&s);} // list all possible sources
      if(sources.elms())
         for(Int s=sources.elms(); s>=0; s--)
      {
         Source &src      =*sources[Mod(s, sources.elms())];
         Bool    cur      =(CE.cur()==&src),
                 cur_start=(cur && s==sources.elms()), // if we're starting search
                 cur_end  =(cur && s==             0); // if we're ending   search
         Int       y_from =(cur_start ? src.cur.y : src.lines.elms()-1),
                   y_to   =(cur_end   ? src.cur.y :                  0);
         for(Int y=y_from; y>=y_to; y--)if(InRange(y, src.lines))
         {
            res.clear();
            CChar *line=src.lines[y]; for(Int offset=0; ; )
            {
               Int match_length, pos=TextPosSkipSpaceI(line+offset, text(), match_length, case_sensitive(), whole_words());
               if(pos<0)break; offset+=pos; res.add(VecI2(offset, match_length)); offset+=match_length; // add all results to container
            }
            REPA(res) // go from last one
            {
               VecI2 r=res[i];
               if(cur_start && y==src.cur.y) // if we're starting search
               {
                  Int cur_x=((src.sel.y==src.cur.y && src.sel.x>=0) ? src.sel.x : src.cur.x); // use 'sel' instead of 'cur' if possible
                  if(r.x>=cur_x)continue; // if result is completely after cursor then ignore
               }
               src.sel.set(r.x    , y);
               src.cur.set(r.x+r.y, y);
               if(!CE.view_mode())src.makeCurVisible(true, !cur);
               else               src.highlight     (y   , !cur);
               CE.cur(&src);
               return;
            }
         }
      }
   }
}
/******************************************************************************/
void Find::findNext()
{
   if(CE.cur())CE.cur()->clearSuggestions();
   if(text().is())
   {
      Memc<Source*> sources; Int offset=CE.curI(); FREPA(CE.sources){Source &s=CE.sources[Mod(i+offset, CE.sources.elms())]; if(FilterScope(&s, scope()))sources.add(&s);} // list all possible sources
      if(sources.elms())
         for(Int s=0; s<sources.elms()+1; s++)
      {
         Source &src      =*sources[Mod(s, sources.elms())];
         Bool    cur      =(CE.cur()==&src),
                 cur_start=(cur && s==             0), // if we're starting search
                 cur_end  =(cur && s==sources.elms()); // if we're ending   search
         Int       y_from =(cur_start ? src.cur.y :                  0),
                   y_to   =(cur_end   ? src.cur.y : src.lines.elms()-1);
         for(Int y=y_from; y<=y_to; y++)if(InRange(y, src.lines))
         {
            CChar *line=src.lines[y]; for(Int offset=0; ; )
            {
               Int match_length, pos=TextPosSkipSpaceI(line+offset, text(), match_length, case_sensitive(), whole_words());
               if(pos<0)break; offset+=pos;
               if(cur_start && y==src.cur.y) // if we're starting search
                  if(offset+match_length<=src.cur.x){offset+=match_length; continue;} // if result is completely before cursor then ignore
               src.sel.set(offset             , y);
               src.cur.set(offset+match_length, y);
               if(!CE.view_mode())src.makeCurVisible(true, !cur);
               else               src.highlight     (y   , !cur);
               CE.cur(&src);
               return;
            }
         }
      }
   }
}
/******************************************************************************/
void Find::toggle() {if(Gui.ms()==&CE.b_find)visibleToggleActivate();else if(Gui.kb()!=&text)activate();else hide();}
/******************************************************************************/
void Find::create()
{
   CChar8 *mode_t[]=
   {
      "Symbol (exact)",
      "Symbol (nearest)",
      "Text in Files",
   };
   Gui+=super::create().hide();
   T+=text          .create(   ).func(FindChanged, T).desc("Use Up/Down Arrow Keys to use previous searches");
   T+=prev          .create("<").func(FindPrev   , T).focusable(false).desc(S+"Previous Search Result\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Shift+D");
   T+=next          .create(">").func(FindNext   , T).focusable(false).desc(S+"Next Search Result\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+D"          );
   T+=close         .create(   ).func(FindClose  , T).focusable(false); close.image="Gui/close.img"; close.skin=&EmptyGuiSkin;
   T+=case_sensitive.create("case sensitive"       ).func(FindChanged, T).focusable(false).desc("Keyboard Shortcut: Alt+C"                                             ); case_sensitive.mode=BUTTON_TOGGLE;
   T+=whole_words   .create("whole words"          ).func(FindChanged, T).focusable(false).desc("Keyboard Shortcut: Alt+W"                                             ); whole_words   .mode=BUTTON_TOGGLE;
   T+=mode          .create(mode_t,    Elms(mode_t)).set(0).valid(true).func(FindChanged, T); mode.tab(0).desc("Keyboard Shortcut: Alt+S"); mode.tab(1).desc("Keyboard Shortcut: Alt+N"); mode.tab(2).desc("Keyboard Shortcut: Alt+T");
   T+=cur_file      .create("Current File"         ).func(FindChanged, T).focusable(false).desc("If perform the search in current file\nKeyboard Shortcut: Alt+F"      ); cur_file      .mode=BUTTON_TOGGLE; cur_file  .set(true, QUIET);
   T+=active_app    .create("Active Application"   ).func(FindChanged, T).focusable(false).desc("If perform the search in active application\nKeyboard Shortcut: Alt+A"); active_app    .mode=BUTTON_TOGGLE; active_app.set(true, QUIET);
   T+=engine        .create("Engine"               ).func(FindChanged, T).focusable(false).desc("If perform the search in the engine headers\nKeyboard Shortcut: Alt+E"); engine        .mode=BUTTON_TOGGLE; engine    .set(true, QUIET);
   T+=skip_ee       .create("Skip \"EE\" namespace").func(FindChanged, T).focusable(false); skip_ee.mode=BUTTON_TOGGLE; skip_ee.set(true);
   slidebar[0].del(); slidebar[1].del(); view.del();

   result.create();
   history_pos=0;
}
/******************************************************************************/
void Find::resize()
{
   Flt x=0.0025f, //D.pixelToScreenSize().x,
       y=0.0025f, //D.pixelToScreenSize().y,
       b=y*4 ,
       h=y*17;
                  rect(Rect_RU(Vec2(D.w()-0.12f, D.h()), x*270, h*5 + b*5 + b));
   close         .rect(Rect_RU(      rect().w()    , -b, h     , h));
   next          .rect(Rect_RU(close.rect().min.x-b, -b, h*0.9f, h));
   prev          .rect(Rect_RU(next .rect().min.x  , -b, h*0.9f, h));
   text          .rect(Rect   (b, -b-h, prev.rect().min.x-b, -b));
   case_sensitive.rect(Rect   (b, -b-h-b-h, rect().w()/2-b/2, -b-h-b));
   whole_words   .rect(Rect   (rect().w()/2+b/2, -b-h-b-h, rect().w()-b, -b-h-b));
   mode          .rect(Rect   (b, -b-h-b-h-b-h, rect().w()-b, -b-h-b-h-b), 0, true);
   cur_file      .rect(Rect   (b, -b-h-b-h-b-h-b-h, rect().w()*0.3f, -b-h-b-h-b-h-b));
   engine        .rect(Rect   (rect().w()*0.76f, -b-h-b-h-b-h-b-h, rect().w()-b, -b-h-b-h-b-h-b));
   active_app    .rect(Rect   (cur_file.rect().max.x, -b-h-b-h-b-h-b-h, engine.rect().min.x, -b-h-b-h-b-h-b));
   skip_ee       .rect(Rect   (rect().w()/2+b/2, -b-h-b-h-b-h-b-h-b-h, rect().w()-b, -b-h-b-h-b-h-b-h-b));

   result.resize();
}
/******************************************************************************/
Find& Find::hide()
{
   Bool visible=T.visible();
   super::hide();
   result.hide();
   if(visible)FindChanged(T); // this will clear results when hidden
   return T;
}
/******************************************************************************/
Find& Find::show()
{
   Bool hidden=T.hidden();
   super::show();
   if(hidden){text.selectAll().activate(); FindChanged(T);}
   return T;
}
/******************************************************************************/
static Bool SelectedText(Str &t, Bool &force)
{
   if(Source *s=CE.cur())
      if(InRange(s->cur.y, s->lines))
   {
      Line &line=s->lines[s->cur.y];
      Int   from, to;
      if(force=(s->sel.x>=0 && s->sel.y==s->cur.y))
      {
         from=Min(s->sel.x, s->cur.x);
         to  =Max(s->sel.x, s->cur.x);
      }else
      {
         from=line.wordBegin(s->cur.x);
         to  =line.wordEnd  (    from);
      }
      for(; from<to; from++)t+=line[from];
      return true;
   }
   return false;
}
Find& Find::activate()
{
   Str t; Bool force;
   // copy text from cursor position to find box
   if(hidden())
      if(SelectedText(t, force))
   {
      if(!force)
      {
         if(t=='.')t.clear(); // clear if the cursor is at '.' because searching for that means that all symbols will be listed which may take some time
         t=SkipWhiteChars(t); // if selection consists only of spaces then clear it
      }
      if(t.is())
      {
         text.set(t); // if there's some text specified then use it for search, otherwise keep the old search
         historyAdd(t);
      }
   }
   super::activate();
   text.selectAll().activate();
   return T;
}
/******************************************************************************/
void Find::historyAdd(C Str &text)
{
   if(history.elms()>=64)history.remove(0, true);
   REPA(history)if(Equal(history[i], text))history.remove(i, true);
   history.add(text);
   history_pos=history.elms()-1;
}
void Find::historySet(Int delta)
{
   Clamp(history_pos+=delta, 0, history.elms()-1);
   if(InRange(history_pos, history))text.set(history[history_pos]).selectAll();
}
void Find::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(visible())
   {
      if(contains(Gui.kb()))
      {
         if((Kb.k(KB_ESC  ) || Kb.k(KB_NAV_BACK)) && Kb.k.first()){hide(); Kb.eatKey();}else
         if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER )) && Kb.k.first())
         {
            CE.markCurPos();
            historyAdd(text());
            findNext();
            hide();
            Kb.eatKey();
         }else
         if(Kb.k(KB_UP  ))historySet(+1);else
         if(Kb.k(KB_DOWN))historySet(-1);
         if(Kb.k(KB_C) && Kb.k.alt()){case_sensitive.push(); Kb.eatKey();}
         if(Kb.k(KB_W) && Kb.k.alt()){whole_words   .push(); Kb.eatKey();}
         if(Kb.k(KB_S) && Kb.k.alt()){mode          .set(0); Kb.eatKey();}
         if(Kb.k(KB_N) && Kb.k.alt()){mode          .set(1); Kb.eatKey();}
         if(Kb.k(KB_T) && Kb.k.alt()){mode          .set(2); Kb.eatKey();}
         if(Kb.k(KB_F) && Kb.k.alt()){cur_file      .push(); Kb.eatKey();}
         if(Kb.k(KB_A) && Kb.k.alt()){active_app    .push(); Kb.eatKey();}
         if(Kb.k(KB_E) && Kb.k.alt()){engine        .push(); Kb.eatKey();}
      }else
      if(result.contains(Gui.kb()))
         if((Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)) && Kb.k.first()){hide(); Kb.eatKey();}

      Bool ms=(contains(Gui.ms()) || result.contains(Gui.ms())),
           kb=(contains(Gui.kb()) || result.contains(Gui.kb()));
      result.visible(ms || kb);
      if(ms && Ms.bp(2))hide();
   }
}
/******************************************************************************/
void Find::ResultRegion::update(C GuiPC &gpc)
{
   super::update(gpc);

   Bool sel=(Gui.ms()==&list && Ms.bp(0)); REPA(Touches)if(Touches[i].guiObj()==&list && Touches[i].rs())sel=true;
   if(  sel)
      if(Result *result=list())
   {
      if(result->symbol         )CE.jumpTo(result->symbol());else
      if(result->source_loc.is())
      {
         Source *cur=CE.cur();
         if(CE.load(result->source_loc))
            if(result->line>=0)
               CE.cur()->highlight(result->line, cur!=CE.cur());
      }else
      {
         result->opened^=1;
         setData();
      }
   }
}
/******************************************************************************/
void Find::ResultRegion::create()
{
   Gui+=super::create().hide();
   ListColumn lc[]=
   {
      ListColumn(MEMBER(Result, text), LCW_DATA, "text"),
   };
   T+=list.create(lc, Elms(lc), true).skin(&CE.find_result_skin); list.cur_mode=LCM_ALWAYS;
}
/******************************************************************************/
void Find::ResultRegion::resize()
{
   rect(Rect(D.w()*0.27f, -D.h()*0.85f, D.w()-0.07f, CE.find.rect().min.y));
   if(TextStyle *text_style=list.getTextStyle())list.elmHeight(text_style->lineHeight()).textSize(text_style->size.y);
}
/******************************************************************************/
static Int CompareSymbolPath(C Str &a, C Str &b)
{
   Str ap=SymbolToPath(a),
       bp=SymbolToPath(b);
   if(Int c=ComparePath(GetPath(ap), GetPath(bp)))return c;
   return   ComparePath(        ap ,         bp  );
}
/******************************************************************************/
static Int CompareResult        (C Find::ResultRegion::Result &a, C Find::ResultRegion::Result &b) {return CompareSymbolPath(a.text    , b.text    );}
static Int CompareResultPriority(C Find::ResultRegion::Result &a, C Find::ResultRegion::Result &b) {return Compare          (b.priority, a.priority);}
static Int CompareResultAlphabet(C Find::ResultRegion::Result &a, C Find::ResultRegion::Result &b) {return Compare          (a.text    , b.text    );}
/******************************************************************************/
static void SetVisible(Memt<Bool> &visible, Memx<Find::ResultRegion::Result> &results, Bool parent_visible=true)
{
   FREPA(results)
   {
      Find::ResultRegion::Result &r=results[i];
      visible.add(parent_visible);
      SetVisible(visible, r.children, parent_visible && r.opened);
   }
}
/******************************************************************************/
void Find::ResultRegion::setData()
{
   Memt<Bool> visible; SetVisible(visible, data);
   list.setDataNode(data, visible);
}
/******************************************************************************/
void Find::ResultRegion::find(C Str &text, Bool case_sensitive, Bool whole_words, MODE mode, UInt scope, Bool skip_ee)
{
   data.clear();
   Str  t        =SymbolToPath(Replace(Replace(text, "::", "."), DIV, '\0'));
   Bool full_name=Contains    (t, SEP);

   if(mode==EXACT)
   {
      Memc<Result> temp;
      Memc<Str   > texts;

         texts=Split(text, L' '); REPA(texts)if(!texts[i].is())texts.remove(i);
      if(texts.elms())
      {
         // files
         temp.clear(); FREPA(CE.sources)if(FilterScope(&CE.sources[i], scope) && FilterText(CE.sources[i].loc.base_name, texts, case_sensitive, whole_words))
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="FILES:"; data.New();}
            temp.New().setText(CE.sources[i].loc.asText(), CE.sources[i].loc);
         }
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(GetPath(temp[i].text), GetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);
      }

         texts=Split(t, L' '); REPA(texts)if(!texts[i].is())texts.remove(i);
      if(texts.elms())
      {
         // macros
         temp.clear(); FREPA(ProjectMacros)
         {
            Macro &macro=ProjectMacros[i];
            if(FilterScope(macro.source, scope) && FilterText(macro.name, texts, case_sensitive, whole_words))
            {
               if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="MACROS:"; data.New();}
               temp.New().setText(macro.name, macro.source ? macro.source->loc : SourceLoc(), macro.line);
            }
         }
         temp.sort(CompareResultAlphabet); FREPA(temp)Swap(data.New(), temp[i]);

         // typedefs
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::TYPEDEF && FilterScope(symbol.source, scope) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words) && !symbol.insideFunc())
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="TYPEDEFS:"; data.New();}
            temp.New().set(skip_ee, symbol.full_name, &symbol);
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // enums
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if((symbol.type==Symbol::ENUM || symbol.type==Symbol::ENUM_ELM) && !(symbol.modifiers&Symbol::MODIF_NAMELESS) && FilterScope(symbol.source, scope) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words) && !symbol.insideFunc())
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="ENUMS:"; data.New();}
            temp.New().set(skip_ee, symbol.full_name, &symbol);
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // namespaces
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::NAMESPACE && !(symbol.modifiers&Symbol::MODIF_NAMELESS) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words) && !symbol.insideFunc())
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="NAMESPACES:"; data.New();}
            temp.New().set(skip_ee, symbol.full_name, &symbol);
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // classes
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::CLASS && !(symbol.modifiers&Symbol::MODIF_NAMELESS) && FilterScope(symbol.source, scope) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words))
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="CLASSES:"; data.New();}
            temp.New().set(skip_ee, symbol.full_name, &symbol);
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // classes extending
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::CLASS && !(symbol.modifiers&Symbol::MODIF_NAMELESS) && FilterScope(symbol.source, scope))
         {
            FREPA(symbol.base)if(FilterText(full_name ? symbol.base[i]->full_name : *symbol.base[i], texts, case_sensitive, whole_words))
            {
               if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text=S+"CLASSES EXTENDING \""+text+"\":"; data.New();}
               temp.New().set(skip_ee, symbol.full_name, &symbol);
               break;
            }
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // functions
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::FUNC_LIST && !(symbol.modifiers&Symbol::MODIF_CTOR_DTOR) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words) && !symbol.insideFunc())
         {
            FREPA(symbol.funcs)if(FilterScope(symbol.funcs[i]->source, scope)) // if at least one function is in scope
            {
               if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="FUNCTIONS:"; data.New();}

               // add all functions within scope
               Memc<SymbolPtr> funcs; FREPA(symbol.funcs)if(FilterScope(symbol.funcs[i]->source, scope))funcs.add(symbol.funcs[i]);

               // remove double functions (definition/declaration)
               REPA(funcs){Symbol *a=funcs[i](); REPD(j, i)if(a->sameFunc(*funcs[j])){funcs.remove((a->modifiers&Symbol::MODIF_FUNC_BODY) ? i : j, true); break;}} // remove the one with function body (because it doesn't have default parameters listed, and full class path)

               if(funcs.elms()==1)temp.New().set(false, SkipWhiteChars(funcs[0]->definition()), funcs[0]);else // if only one function
               {
                  // add functions list and its children
                  Result &func_list=temp.New().set(skip_ee, symbol.full_name+ELLIPSIS, null);
                  FREPA(funcs)func_list.children.New().set(false, S+"   "+SkipWhiteChars(funcs[i]->definition()), funcs[i]);
               }

               break;
            }
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);

         // variables
         temp.clear(); FREPA(Symbols){Symbol &symbol=Symbols.lockedData(i); if(symbol.type==Symbol::VAR && FilterScope(symbol.source, scope) && FilterText(full_name ? symbol.full_name : symbol, texts, case_sensitive, whole_words) && !symbol.insideFunc())
         {
            if(!temp.elms()){if(data.elms())data.New(); data.New().text=RESULT_SEPARATOR; data.New(); data.New().text="VARIABLES:"; data.New();}
            temp.New().set(skip_ee, symbol.full_name, &symbol);
         }}
         temp.sort(CompareResult); REPA(temp)if(i)if(!Equal(SymbolGetPath(temp[i].text), SymbolGetPath(temp[i-1].text), true))temp.NewAt(i); FREPA(temp)Swap(data.New(), temp[i]);
      }
   }else
   if(mode==NEAREST)
   {
      if(t.is())
      {
         // symbols
         REPA(Symbols)
         {
            Symbol &symbol=Symbols.lockedData(i);
            if(!(symbol.modifiers&Symbol::MODIF_SKIP_SUGGESTIONS) && symbol.valid && symbol.type!=Symbol::PREPROC && symbol.type!=Symbol::TYPENAME && !symbol.insideFunc())
               if(Int p=SuggestionsPriority(full_name ? symbol.full_name : symbol, t, FlagTest(symbol.modifiers, Symbol::MODIF_ALL_UP_CASE)))
            {
               if(symbol.type==Symbol::FUNC_LIST)
               {
                  FREPA(symbol.funcs)if(FilterScope(symbol.funcs[i]->source, scope)) // if at least one function is in scope
                  {
                     // add all functions within scope
                     Memc<SymbolPtr> funcs; FREPA(symbol.funcs)if(FilterScope(symbol.funcs[i]->source, scope))funcs.add(symbol.funcs[i]);

                     // remove double functions (definition/declaration)
                     REPA(funcs){Symbol *a=funcs[i](); REPD(j, i)if(a->sameFunc(*funcs[j])){funcs.remove((a->modifiers&Symbol::MODIF_FUNC_BODY) ? i : j, true); break;}} // remove the one with function body (because it doesn't have default parameters listed, and full class path)

                     if(funcs.elms()==1)data.New().set(false, SkipWhiteChars(funcs[0]->definition()), funcs[0], p);else // if only one function
                     {
                        // add functions list and its children
                        Result &func_list=data.New().set(skip_ee, symbol.full_name+ELLIPSIS, null, p);
                        FREPA(funcs)func_list.children.New().set(false, S+"   "+SkipWhiteChars(funcs[i]->definition()), funcs[i]);
                     }

                     break;
                  }
               }else
               if(FilterScope(symbol.source, scope))
               {
                  data.New().set(skip_ee, symbol.full_name, &symbol, p);
               }
            }
         }

         // macros
         REPA(ProjectMacros)
         {
            Macro &macro=ProjectMacros[i];
            if(FilterScope(macro.source, scope))
               if(Int p=SuggestionsPriority(macro.name, t, macro.all_up_case))
                  data.New().setText(macro.name, macro.source ? macro.source->loc : SourceLoc(), macro.line).priority=p;
         }
      }
      data.sort(CompareResultPriority);
   }else
   if(mode==FILES)
   {
      Source *last_source=null;
      FREPA(CE.sources)
      {
         Source &s=CE.sources[i]; if(FilterScope(&s, scope))FREPA(s.lines)
         {
            if(Contains(s.lines[i], text, case_sensitive, whole_words))
            {
               if(last_source!=&s){last_source=&s; if(data.elms())data.New(); data.New().setText(S+'"'+s.loc.asText()+"\":", s.loc);} // add "FILE:"
               data.New().setText(S+"  "+SkipWhiteChars(s.lines[i]), s.loc, i);
            }
         }
      }
   }

   setData();
}
/******************************************************************************/
static CChar8 *scope_t[]=
{
   "Selection",
   "Current File",
};
enum REPLACE_SCOPE
{
   RS_SEL,
   RS_CUR_FILE,
};
void ReplaceProcess(ReplaceText &r) {r.process();}
void ReplaceText::process()
{
   if(Source *src=CE.cur())
   {
      if(src->Const){Gui.msgBox(S, "This file can't be changed"); return;}
      Str start, text, end;
      if(src->lines.elms())
      {
         Int   last_line_length=-1;
         VecI2 min, max;
         if(scope()==RS_SEL)
         {
            if(src->sel.x<0){Gui.msgBox(S, "Selection is empty"); return;}
            src->curSel(min, max);
            src->writeAll(start, VecI2(0), min);
            src->writeAll(text , min     , max);
            src->writeAll(end  , max     , VecI2(src->lines.last().length(), src->lines.elms()-1));
            if(InRange(max.y, src->lines))last_line_length=src->lines[max.y].length();
         }else text=src->asText();
         text=Replace(text, T.src(), T.dest(), case_sensitive(), whole_words());
         src->setUndo();
         src->fromText(start+text+end);

         // adjust cursor/selection
         if(last_line_length>=0 && InRange(max.y, src->lines))
         {
            Int delta=src->lines[max.y].length()-last_line_length;
            ((src->cur==max) ? src->cur : src->sel).x+=delta;
         }
      }
   }else Gui.msgBox(S, "No file opened");
}
static void CaseSensitive(ReplaceText &r) {r.case_sensitive.push();}
static void WholeWords   (ReplaceText &r) {r.whole_words   .push();}
void ReplaceText::create()
{
   Flt y=-0.04f, h=0.055f, s=h+0.01f;
   Gui+=super::create(Rect_C(0, 0, 1.05f, 0.47f), "Replace").hide(); button[2].func(CodeEditor::HideAndFocusCE, SCAST(GuiObj, T)).show();
   T+=t_src  .create(Vec2(0.09f, y), "Find"   ); T+=src  .create(Rect_L(0.18f, y, 0.82f, h)); y-=s;
   T+=t_dest .create(Vec2(0.09f, y), "Replace"); T+=dest .create(Rect_L(0.18f, y, 0.82f, h)); y-=s;
   T+=t_scope.create(Vec2(0.09f, y), "Scope"  ); T+=scope.create(Rect_L(0.18f, y, 0.82f, h), scope_t, Elms(scope_t)); y-=s;
   T+=case_sensitive.create(Rect_R(clientWidth()/2-0.03f, y, 0.3f, h), "case sensitive").desc("Keyboard Shortcut: Alt+C"); case_sensitive.mode=BUTTON_TOGGLE;
   T+=whole_words   .create(Rect_L(clientWidth()/2+0.03f, y, 0.3f, h), "whole words"   ).desc("Keyboard Shortcut: Alt+W"); whole_words   .mode=BUTTON_TOGGLE; y-=s;
   y-=s/2;
   T+=replace       .create(Rect_C(clientWidth()*1.0f/4, y, 0.3f, 0.06f), "Replace All").func(ReplaceProcess, T).desc("Keyboard Shortcut: Enter");
   T+=cancel        .create(Rect_C(clientWidth()*3.0f/4, y, 0.3f, 0.06f), "Cancel").func(button[2].func(), button[2].funcUser()); y-=s;
   Node<MenuElm> node;
   node.New().create("Case Sensitive", CaseSensitive , T).kbsc(KbSc(KB_C, KBSC_ALT));
   node.New().create("Whole Words"   , WholeWords    , T).kbsc(KbSc(KB_W, KBSC_ALT));
   node.New().create("Replace"       , ReplaceProcess, T).kbsc(KbSc(KB_ENTER));
   T+=menu.create(node);
}
ReplaceText& ReplaceText::show()
{
   if(hidden())
   {
      super::show();
      Str t; Bool force; if(!src().is() && SelectedText(t, force))src.set(t);
      src.selectAll().kbSet();
      Source *src=CE.cur();
      scope.set((src && src->sel.x>=0) ? RS_SEL : RS_CUR_FILE);
   }
   return T;
}
/******************************************************************************/
}}
/******************************************************************************/
