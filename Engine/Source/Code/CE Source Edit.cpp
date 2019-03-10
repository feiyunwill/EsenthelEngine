/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
static Bool ClipFullLine;
static Str  ClipTemp;
/******************************************************************************/
void Source::startSel()
{
   if(Kb.k.shift())
   {
      if(sel.x<0)sel=cur;
   }else
   {
      sel=-1;
   }
}
/******************************************************************************/
void Source::highlight(Int line, Bool immediate)
{
   highlight_line=line;
   highlight_time=1;
   sel  =-1;
   cur.y=Mid(line, 0, lines.elms()-1);
   makeCurVisible(true, immediate);
}
/******************************************************************************/
void Source::makeCurVisible(Bool center, Bool immediate)
{
   VecI2 cur=T.cur; if(CE.view_mode())cur=realToView(cur);
   if(center)
   {
      scrollFitY(cur.y*CE.ts.lineHeight()-clientHeight()*0.5f, cur.y*CE.ts.lineHeight()+clientHeight()*0.5f, immediate);
   }else
   {
      if(cur.y  <Ceil ( slidebar[1].offset()                /CE.ts.lineHeight()))scrollToY( cur.y   *CE.ts.lineHeight(), immediate);else
      if(cur.y+1>Trunc((slidebar[1].offset()+clientHeight())/CE.ts.lineHeight()))scrollToY((cur.y+1)*CE.ts.lineHeight(), immediate);
   }
   if(cur.x  <Ceil ((slidebar[0].offset()+CE.lineNumberSize())/CE.ts.colWidth()))scrollToX( cur.x   *CE.ts.colWidth(), true);else
   if(cur.x+1>Trunc((slidebar[0].offset()+clientWidth()      )/CE.ts.colWidth()))scrollToX((cur.x+8)*CE.ts.colWidth(), true);

   setOffset();
}
/******************************************************************************/
void Source::curLeft     () {forceCreateNextUndo(); clearSuggestions(); if(sel.x>=0 && !Kb.k.shift()){if(sel.y<cur.y || (sel.y==cur.y && sel.x<cur.x))cur=sel; sel=-1;}else{startSel(); MAX(--cur.x, 0); if(0 && CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x  , id, range))cur.x=range.x  ;}} makeCurVisible();}
void Source::curRight    () {forceCreateNextUndo(); clearSuggestions(); if(sel.x>=0 && !Kb.k.shift()){if(sel.y>cur.y || (sel.y==cur.y && sel.x>cur.x))cur=sel; sel=-1;}else{startSel();     ++cur.x    ; if(0 && CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x-1, id, range))cur.x=range.y+1;}} makeCurVisible();}
void Source::curUp       () {forceCreateNextUndo(); clearSuggestions(); if(sel.x>=0 && !Kb.k.shift())MIN(cur.y, sel.y); startSel(); MAX(--cur.y,     0                 ); makeCurVisible();}
void Source::curDown     () {forceCreateNextUndo(); clearSuggestions(); if(sel.x>=0 && !Kb.k.shift())MAX(cur.y, sel.y); startSel(); MIN(++cur.y, Max(0, lines.elms()-1)); makeCurVisible();}
void Source::curLineBegin() {forceCreateNextUndo(); clearSuggestions(); startSel();                                           if(InRange(cur.y, lines)){Int start=lines[cur.y].start(); cur.x=((cur.x==start) ? 0 : start);}else cur.x=0; makeCurVisible();}
void Source::curLineEnd  () {forceCreateNextUndo(); clearSuggestions(); startSel();                                           if(InRange(cur.y, lines))                                 cur.x=lines[cur.y].length()       ; else cur.x=0; makeCurVisible();}
void Source::selAll      () {forceCreateNextUndo(); clearSuggestions(); CE.markCurPos(); sel=0; cur.y=Max(0, lines.elms()-1); if(InRange(cur.y, lines))                                 cur.x=lines[cur.y].length()       ; else cur.x=0; makeCurVisible();}
void Source::selWord     ()
{
   forceCreateNextUndo(); clearSuggestions();
   sel=cur;
   if(InRange(cur.y, lines))
   {
      Line &line=lines[cur.y];
      sel.x=line.wordStart(cur.x);
      cur.x=line.wordEnd  (sel.x);
   }
   if(sel==cur)sel=-1;
   makeCurVisible();
}
/******************************************************************************/
void Source::curPrevWord()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   if(cur.x)
   {
      cur.x--;
      if(InRange(cur.y, lines))for(CHAR_TYPE ct=CodeCharType(lines[cur.y][cur.x]); cur.x>0; cur.x--)
      {
         if(CE.view_elm_names){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x, id, range)){cur.x=range.x; break;}}
         CHAR_TYPE nt=CodeCharType(lines[cur.y][cur.x-1]);
         if(ct==CHART_SPACE || ct==CHART_NONE)ct=nt;
         if(ct!=nt || (ct==CHART_SIGN && (/*lines[cur.y].Type(cur.x-1)==TOKEN_OPERATOR || */lines[cur.y].Type(cur.x)==TOKEN_OPERATOR)))break;
      }
   }else
   {
      if(cur.y){cur.y--; cur.x=lines[cur.y].end();}
   }
   makeCurVisible();
}
void Source::curNextWord()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   if(InRange(cur.y, lines))
   {
      if(cur.x>=lines[cur.y].end()) // at the end of line
      {
         if(InRange(cur.y+1, lines))
         {
            cur.y++;
            for(cur.x=0; cur.x<lines[cur.y].length(); cur.x++)if(lines[cur.y][cur.x]!=' ')break;
         }
      }else
      {
         cur.x++;
         for(CHAR_TYPE ct=CodeCharType(lines[cur.y][cur.x-1]); cur.x<lines[cur.y].length(); cur.x++)
         {
            if(CE.view_elm_names){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x-1, id, range))cur.x=range.y+1;}
            CHAR_TYPE nt=CodeCharType(lines[cur.y][cur.x]);
            if(ct!=nt || (ct==CHART_SIGN && lines[cur.y].Type(cur.x)==TOKEN_OPERATOR)){for(; cur.x<lines[cur.y].length() && lines[cur.y][cur.x]==' '; cur.x++); break;}
         }
      }
   }
   makeCurVisible();
}
/******************************************************************************/
void Source::curPrevBracket() {for(VecI2 p=cur; lineValid(dec(p)); )if((lines[p.y][p.x]=='{' || lines[p.y][p.x]=='}') && lines[p.y].Type(p.x)==TOKEN_OPERATOR){cur=p; makeCurVisible(); break;}}
void Source::curNextBracket() {for(VecI2 p=cur; lineValid(inc(p)); )if((lines[p.y][p.x]=='{' || lines[p.y][p.x]=='}') && lines[p.y].Type(p.x)==TOKEN_OPERATOR){cur=p; makeCurVisible(); break;}}
/******************************************************************************/
void Source::curPrevLevelBracket()
{
   Int level=((posValid(cur) && lines[cur.y][cur.x]=='{') ? 2 : 0); // if we're on '{' then break on any next brace
   for(VecI2 p=cur; lineValid(dec(p)); )if(lines[p.y].Type(p.x)==TOKEN_OPERATOR){if(lines[p.y][p.x]=='{')level++;else if(lines[p.y][p.x]=='}')level--;else continue; if(level>0){cur=p; makeCurVisible(); break;}}
}
void Source::curNextLevelBracket()
{
   Int level=((posValid(cur) && lines[cur.y][cur.x]=='}') ? -2 : 0); // if we're on '}' then break on any next brace
   for(VecI2 p=cur; lineValid(inc(p)); )if(lines[p.y].Type(p.x)==TOKEN_OPERATOR){if(lines[p.y][p.x]=='{')level++;else if(lines[p.y][p.x]=='}')level--;else continue; if(level<0){cur=p; makeCurVisible(); break;}}
}
/******************************************************************************/
void Source::curPageUp()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   Bool visible=isCurVisible();
   Int  ys=Max(1, Trunc(clientHeight()/CE.ts.lineHeight()));
   MAX(cur.y-=ys, 0);
   if(visible)scrollY(-ys*CE.ts.lineHeight(), true);else makeCurVisible();
}
void Source::curPageDown()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   Bool visible=isCurVisible();
   Int  ys=Max(1, Trunc(clientHeight()/CE.ts.lineHeight()));
   MIN(cur.y+=ys, Max(0, lines.elms()-1));
   if(visible)scrollY(ys*CE.ts.lineHeight(), true);else makeCurVisible();
}
/******************************************************************************/
Int Source::viewBeginPos() {return     Max(0,                   Ceil((slidebar[1].offset()               )/CE.ts.lineHeight())  );}
Int Source::viewEndPos  () {return Min(Max(0, lines.elms()-1), Trunc((slidebar[1].offset()+clientHeight())/CE.ts.lineHeight())-1);}
/******************************************************************************/
void Source::curViewBegin()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   cur.y=viewBeginPos();
   makeCurVisible();
}
void Source::curViewEnd()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   cur.y=viewEndPos();
   makeCurVisible();
}
void Source::curDocBegin()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   cur=0;
   scrollToY(0, true);
}
void Source::curDocEnd()
{
   forceCreateNextUndo(); clearSuggestions(); startSel();
   cur.y=Max(0, lines.elms()-1);
   if(InRange(cur.y, lines))cur.x=lines[cur.y].length();else cur.x=0;
   makeCurVisible();
}
/******************************************************************************/
void Source::curClip()
{
   if(CE.options.eol_clip())
   {
      MIN(cur.y, Max(0, lines.elms()-1));
      if(InRange(cur.y, lines))MIN(cur.x, lines[cur.y].length());
      else                         cur.x=0;
   }
}
/******************************************************************************/
void Source::removeLine(Int i) {lines.removeValid(i, true);}
/******************************************************************************/
void Source::delSel(Bool set_undo, Bool clear_suggestions)
{
   if(Const)return;
   if(set_undo         )setUndo();
   if(clear_suggestions)clearSuggestions();
   if(sel.x>=0 && sel.y>=0)
   {
      VecI2 min, max; curSel(min, max);
      if(min.y==max.y)
      {
         if(InRange(min.y, lines))lines[min.y].remove(min.x, max.x-min.x);
      }else
      {
         if(InRange(min.y, lines)){lines[min.y].clip  (   min.x); if(0)REP(min.x-lines[min.y].length())lines[min.y]+=' '; min.x=lines[min.y].length();}
         if(InRange(max.y, lines)) lines[max.y].remove(0, max.x);
      }
      REP(max.y-min.y-1)removeLine(min.y+1); // remove lines between
      if(min.y!=max.y && InRange(min.y+1, lines)) // merge first and last lines
      {
         lines[min.y].append(lines[min.y+1]);
         removeLine(min.y+1);
      }
      sel=-1;
      cur=min;
      changed(cur.y);
      makeCurVisible();
   }
}
/******************************************************************************/
void Source::cut()
{
   if(Const)return;
   copy();
   if(sel.x>=0)delSel();else
   {
      setUndo();
      if(InRange(cur.y, lines))
      {
         removeLine(cur.y);
         changed   (cur.y);
      }
   }
}
/******************************************************************************/
void Source::copy()
{
   clearSuggestions();

   if(sel.x>=0 && sel.y>=0)
   {
      Str c; VecI2 min, max; curSel(min, max);
      writeAll(c, min, max);
      ClipFullLine=false;
      ClipSet(c);
   }else
   {
      if(InRange(cur.y, lines))
      {
         ClipFullLine=true;
         ClipSet(ClipTemp=(lines[cur.y]+'\n'));
      }else
      {
         ClipFullLine=false;
         ClipSet(S);
      }
   }
}
/******************************************************************************/
void Source::paste(C Str *text, Bool move_cur)
{
   if(Const)return;
   delSel(); // already calls undo
   Str c=(text ? *text : ClipGet());
   if( c.is())
   {
      VecI2 old=cur;
      CE.markCurPos();
      Str tab; REP(TabLength)tab+=' ';
      Int start=cur.y;
      if(!text && ClipFullLine && Equal(c, ClipTemp, true))
      {
         lines.NewAt(cur.y)=Replace(Replace(Replace(c, "\t", tab), '\r', '\0'), '\n', '\0');
         cur.y++;
      }else
      {
         Memc<Str> ln=Split(Replace(Replace(c, "\t", tab), '\r', '\0'), '\n');
         if(ln.elms())
         {
            exist(cur.x, cur.y);
          //sel=cur;
            FREPA(ln)
            {
               Str &l=ln[i];
               if(i)
               {
               	Str &src=lines[cur.y];
                  lines.NewAt(cur.y+1)=src()+cur.x;
               	lines[cur.y].clip(cur.x);
                  cur.y++;
                  cur.x=0;
               }
               FREPA(l)lines[cur.y].insert(cur.x++, l[i]);
            }
         }
      }
      changed(start, cur.y-start+1);
      if(!move_cur)cur=old;
      makeCurVisible();
   }
}
/******************************************************************************/
void Source::separator()
{
   if(Const)return;
   delSel(); // already calls undo
   CE.markCurPos();
   Int start=cur.y;
   if(cur.y>=lines.elms())lines.setNum(cur.y+1);
   lines.NewAt(cur.y)=SEP_LINE;
   cur.y++;
   changed(start, cur.y-start+1);
   makeCurVisible();
}
/******************************************************************************/
void Source::delForward()
{
   if(Const)return;
   if(sel.x>=0)delSel();else
   {
      if(CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x, id, range)){setUndo(); clearSuggestions(); lines[cur.y].remove(range.x, range.y-range.x+1); cur.x=range.x; changed(cur.y); makeCurVisible(); return;}}

      clearSuggestions();
      setUndo(DEL_CHR);
      if(InRange(cur.y, lines))
      {
         if(cur.x<lines[cur.y].length())lines[cur.y].remove(cur.x);else
         if(InRange(cur.y+1, lines))
         {
            exist(cur.x, cur.y);
            lines[cur.y].append(lines[cur.y+1]);
            removeLine(cur.y+1);
         }
         changed(cur.y);
      }
   }
}
/******************************************************************************/
void Source::delBack()
{
   if(Const)return;
   if(sel.x>=0)delSel();else
   {
      if(CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x-1, id, range)){setUndo(); clearSuggestions(); lines[cur.y].remove(range.x, range.y-range.x+1); cur.x=range.x; changed(cur.y); makeCurVisible(); return;}}

      clearSuggestions();
      if(undos.lastChangeTypeI()==INS_CHRS)undoAsChange();else
      {
         setUndo(DEL_CHR);
         if(cur.x==0)
         {
            if(InRange(cur.y-1, lines)
            && InRange(cur.y  , lines))
            {
               cur.x=lines[cur.y-1].length();
               lines[cur.y-1].append(lines[cur.y]);
               removeLine(cur.y);
               cur.y--;
               changed(cur.y);
               makeCurVisible();
            }else
            if(cur.y>=lines.elms())
            {
               cur.y=Max(0, lines.elms()-1);
               if(InRange(cur.y, lines))cur.x=lines[cur.y].length();
               makeCurVisible();
            }
         }else
         {
            if(InRange(cur.y, lines)){cur.x--; lines[cur.y].remove(cur.x); changed(cur.y); makeCurVisible(); listSuggestions(-1);}else cur.x=0;
         }
      }
   }
}
/******************************************************************************/
// set undo before changing selection so pressing undo later will preserve the selection
void Source::delWordForward()
{
   if(Const)return; setUndo();
   if(sel.x<0)
   {
      if(CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x, id, range))cur.x=range.x;} // move to the beginning of the word
      VecI2 temp=cur; curNextWord(); sel=temp;
   }
   delSel(false);
}
void Source::delWordBack()
{
   if(Const)return; setUndo();
   if(sel.x<0)
   {
      if(CE.view_elm_names && InRange(cur.y, lines)){UID id; VecI2 range; if(TextToIDInside(lines[cur.y], cur.x-1, id, range))cur.x=range.y+1;} // move to the end of the word
      VecI2 temp=cur; curPrevWord(); sel=temp;
   }
   delSel(false);
}
/******************************************************************************/
static Char  MakeCase(Char c, Bool upper) {return upper ? CaseUp(c) : CaseDown(c);}
void Source::makeCase(Bool upper)
{
   if(Const)return;
   if(sel.x>=0 && sel.y>=0)
   {
      setUndo();
      VecI2 min, max; curSel(min, max);
      for(Int y=min.y; y<=max.y; y++)if(InRange(y, lines))
      {
         Line &line=lines[y];
         for(Int max_x=((y==max.y) ? Min(max.x, line.length()) : line.length()),
                     x=((y==min.y) ? Max(min.x,             0) : 0            ); x<max_x; x++)line.setChar(x, MakeCase(line[x], upper));
      }
      changed(min.y, max.y-min.y+1);
   }else
   if(InRange(cur.y, lines))
   {
      Line &line=lines[cur.y]; if(InRange(cur.x, line))
      {
         setUndo();
         line.setChar(cur.x, MakeCase(line[cur.x], upper));
         cur.x++;
         changed(cur.y);
      }
   }
   clearSuggestions();
   makeCurVisible  ();
}
/******************************************************************************/
// UNDO
/******************************************************************************/
void Source::UndoChange::create(Ptr source)
{
   if(Source *src=(Source*)source)
   {
      File temp; temp.writeMem(); temp.putInt(src->lines.elms()); FREPA(src->lines)src->lines[i].saveData(temp); temp.pos(0); Compress(temp, data.writeMem(), COMPRESS_LZ4, 0, false);
      cur        =src->cur;
      sel        =src->sel;
      modify_time=src->modify_time;
   }
}
void Source::UndoChange::apply(Ptr source)
{
   if(Source *src=(Source*)source)
   {
      File temp; data.pos(0); Decompress(data, temp, true); temp.pos(0); src->lines.clear(); REP(temp.getInt())src->lines.New().loadData(temp);
      src->cur=cur;
      src->sel=sel;
      src->changed(0, -1);
      src->clearSuggestions();
      src->makeCurVisible();
    //src->modify_time=modify_time; // after changed // EDIT: do not restore modification time, because VS will not recompile .cpp file if its modification time is older than compiled .obj file (VS checks if date(.cpp)>date(.obj) and not date!=data, so we need to always set newest date)
   }
}
void Source::forceCreateNextUndo() {undos.forceCreateNextUndo();}
void Source::delUndo()
{
   undos.del();
   undo_original_state=0;
   modify_time.getUTC();
}
void Source::setUndo(UNDO_TYPE undo_type)
{
   Bool modified=T.modified();
   if(UndoChange *change=undos.set(undo_type, undo_type==DEFAULT || undo_type==INS_CHRS))
   {
      if(!InRange(undo_original_state, undos.changes()))undo_original_state=-1;
      if(CE.cur()==this && modified!=T.modified())CE.cei().sourceChanged();
   }
}
void Source::undoAsChange()
{
   clearSuggestions();
   Bool modified=T.modified();
   if(undos.undoAsChange())
      if(CE.cur()==this && modified!=T.modified())CE.cei().sourceChanged();
}
void Source::undo()
{
   clearSuggestions();
   Bool modified=T.modified();
   if(undos.undo())
      if(CE.cur()==this && modified!=T.modified())CE.cei().sourceChanged();
}
void Source::redo()
{
   clearSuggestions();
   Bool modified=T.modified();
   if(undos.redo())
      if(CE.cur()==this && modified!=T.modified())CE.cei().sourceChanged();
}
/******************************************************************************/
Str Source::asText()C
{
   Str    data; FREPA(lines){if(i)data+='\n'; data+=lines[i];}
   return data;
}
void Source::fromText(C Str &data)
{
   lines.clear();
   FREPA(data)
   {
      if(!lines.elms())lines.New();
      Char c=data[i];
      if(c==  9){REP(TabLength)lines.last()+=' ';}else // tab
      if(c==0xA)lines.New ();else                      // new line
      if(c>= 32)lines.last()+=c;                       // any valid char
   }
   changed(0, -1);
   clearSuggestions();
   makeCurVisible  ();
}
/******************************************************************************/
}}
/******************************************************************************/
