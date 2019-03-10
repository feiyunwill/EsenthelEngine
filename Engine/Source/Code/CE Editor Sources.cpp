/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
void CodeEditor::cur(Source *cur)
{
   if(T._cur!=cur && parent)
   {
      if(T._cur)*parent-=*T._cur; //         hide old
      if(T._cur=cur)              // set and show new
      {
        *parent+=*cur;
         cur->moveToBottom().moveAbove(menu); // move to bottom below 'browse_mode' buttons, but above 'menu' so alt is processed properly (Menu Alt detection relies on 'Kb.k' detection, which can be eaten by this gui object, so make sure 'menu' is processed first)
         cur->prepareForDraw();
         cur->activate();
         if(!cur->opened)
         {
            cur->opened=true;
          //sources.moveToEnd(curI()); this won't work with 'find previous/next' and can make engine headers be between custom sources
         }
         if(cur->used())init(); // if activated a used source then make sure that Code Editor is initialized (do this after operating on 'cur' in case 'init' will somehow change/delete it)
      }
      b_close.visible(T._cur!=null && menu.visible()); // close button is visible on top of menu (so display only if that's visible too)
      cei().sourceChanged(true);
   }
}
/******************************************************************************/
Source& CodeEditor::New()
{
   Source &src=sources.New();
   cur(&src);
   return src;
}
/******************************************************************************/
void CodeEditor::sourceRemoved(Source &src)
{
   REPA(curpos)
   {
      if(curpos[i].source==&src){curpos.remove(i, true); if(i<curposi)curposi--;}
   }

   // adjust symbols
   REPA(Symbols)
   {
      Symbol &symbol=Symbols.lockedData(i);
      if(symbol.source==&src)symbol.source=null;
   }

   // exclude from opened
   src.opened=false;

   // make sure it's not current
   if(cur()==&src)nextFile();
}
/******************************************************************************/
void CodeEditor::closeDo()
{
   if(Source *s=cur())
   {
      s->opened=false; // close
      prevFile(); // activate previous opened source
      if(s->used())s->reload();else sources.removeData(s, true); // if 's' is used in the project then reload to maintain last saved position (in case there were some changes applied, but the user decided not to save them), if not then remove from 'sources'
      cei().sourceChanged();
   }
}
void CodeEditor::closeAll()
{
   for(; cur(); )closeDo();
   removeUselessSources();
}
/******************************************************************************/
static Bool  SaveSource(Edit::SaveChanges::Elm &elm) {REPA(CE.sources)if(elm.user==&CE.sources[i])return CE.sources[i].overwrite(); return true;}
static void CloseSource(Edit::SaveChanges::Elm &elm) {if(CE.cur() && elm.user==CE.cur())CE.closeDo();}
void CodeEditor::close()
{
   if(cur())
   {
      if(cur()->modified())
      {
         save_changes.set(Edit::SaveChanges::Elm().set(cur()->loc.file_name, S+"Source - \""+cur()->loc.asText()+'"', cur()->loc.id, cur()).save(SaveSource).close(CloseSource));
      }else
      {
         closeDo();
      }
   }
}
void CodeEditor::saveChanges()
{
   REPA(sources)
   {
      Source &src=sources[i];
      if(src.modified())src.overwrite();
   }
}
void CodeEditor::saveChanges(Memc<Edit::SaveChanges::Elm> &elms)
{
   REPA(sources)
   {
      Source &src=sources[i];
      if(src.modified())elms.New().set(src.loc.file_name, S+"Source - \""+src.loc.asText()+'"', src.loc.id, &src).save(SaveSource);
   }
}
/******************************************************************************/
void CodeEditor::removeUselessSources()
{
   REPA(sources){Source &src=sources[i]; if(!src.opened && !src.used())sources.removeValid(i, true);}
}
/******************************************************************************/
CodeEditor::CurPos CodeEditor::curPos() {if(Source *source=cur())return CurPos(source->cur.x, InRange(source->cur.y, source->lines) ? source->lines[source->cur.y].id : UIDZero, source); return CurPos(0, UIDZero, null);}
/******************************************************************************/
void CodeEditor::markCurPos()
{
   if(Source *source=cur())
   {
      CurPos cp=curPos();
      if(InRange(curposi-1, curpos))
      {
         CurPos &prev=curpos[curposi-1];
         if(prev==cp)return;
         if(prev.source==cp.source && prev.line==cp.line && Abs(prev.col-cp.col)<=3){prev.col=cp.col; return;}
      }
      if(InRange(curposi, curpos))
      {
         CurPos &next=curpos[curposi];
         if(next==cp){curposi++; return;}
      }
      curpos.NewAt(curposi++)=cp;
      if(curposi              >128){curpos.remove(0, true); curposi--;}else
      if(curpos.elms()-curposi>128){curpos.removeLast(   );           }
   }
}
/******************************************************************************/
void CodeEditor::prevCurPos() {if(InRange(curposi-1, curpos)){markCurPos(); CurPos cp=curPos(); for(; InRange(curposi-1, curpos); ){jumpTo(curpos[--curposi  ]); if(cp!=curPos())break;}}}
void CodeEditor::nextCurPos() {if(InRange(curposi  , curpos)){markCurPos(); CurPos cp=curPos(); for(; InRange(curposi  , curpos); ){jumpTo(curpos[  curposi++]); if(cp!=curPos())break;}}}
/******************************************************************************/
void CodeEditor::prevIssue() {CurPos cp=curPos(); FREP(build_list.visibleElms()){Int v=Mod(       build_list.highlight_line -i, build_list.visibleElms()); if(BuildResult *br=build_list.visToData(v))if(br->jumpTo() && (cp!=curPos() || build_list.highlight_line<0)){build_list.highlight_line=v; build_list.highlight_time=Time.appTime(); build_list.scrollTo(v, false, 0.5f); break;}}}
void CodeEditor::nextIssue() {CurPos cp=curPos(); FREP(build_list.visibleElms()){Int v=Mod(Max(0, build_list.highlight_line)+i, build_list.visibleElms()); if(BuildResult *br=build_list.visToData(v))if(br->jumpTo() && (cp!=curPos() || build_list.highlight_line<0)){build_list.highlight_line=v; build_list.highlight_time=Time.appTime(); build_list.scrollTo(v, false, 0.5f); break;}}}
/******************************************************************************/
void CodeEditor::jumpTo(CurPos &cp)
{
   Bool immediate=(cur()!=cp.source);
   cur(cp.source);
   cp.source->sel=-1;
   cp.source->cur= 0;
   Int l=cp.source->findLine(cp.line); if(l>=0)cp.source->cur.set(cp.col, l);
   if(!view_mode())cp.source->makeCurVisible(true, immediate);else
   if( l>=0       )cp.source->highlight     (l   , immediate);
}
/******************************************************************************/
void CodeEditor::jumpTo(Source *source, Int line)
{
   if(source)
   {
      markCurPos();
      Source *old=cur();
      cur(source);
      source->sel=-1;
      source->cur= 0;
      source->highlight(line, old!=cur());
   }
}
/******************************************************************************/
void CodeEditor::jumpTo(Macro *macro)
{
   if(macro)jumpTo(macro->source, macro->line);
}
/******************************************************************************/
void CodeEditor::jumpTo(Symbol *symbol)
{
   if(symbol)
   {
      if(Source *source=symbol->source)
      {
         markCurPos();
         Source *old=cur();
         cur(source);
         if(InRange(symbol->token_index, source->tokens))
         {
            Token &token=*source->tokens[symbol->token_index];
            source->highlight(token.lineIndex(), old!=cur());
            source->sel=-1;
            source->cur.set(token.col, token.lineIndex());
            markCurPos();
         }
      }else
      if(symbol->type==Symbol::KEYWORD)
      {
         Str comments=symbol->comments();
         if( comments.is())Gui.msgBox(S+"Keyword \""+*symbol+"\"", comments);
      }
   }
}
/******************************************************************************/
void CodeEditor::findAllReferences(C Str &text)
{
   buildClear();
   FREPA(sources)if(Find::FilterScope(&sources[i], Find::ALL))
   {
      Source &source=sources[i];
      Bool    found =false;
      FREPA(source.lines)
      {
         Line &line=source.lines[i];
         if(Contains(line, text, true, true))
         {
            if(!found){found=true; buildNew().set(S+"\""+source.loc.asText()+"\":", &source);}
            buildNew().set(S+"   "+line, &line);
         }
      }
   }
   buildUpdate();
}
/******************************************************************************/
void CodeEditor::findAllReferences(Symbol *symbol)
{
   if(symbol)
   {
      Str symbol_name=symbol->shortName();
      buildClear();
      Line *last_line=null;
      FREPA(sources)if(Find::FilterScope(&sources[i], Find::ALL))
      {
         Source &source=sources[i];
         Bool    found =false;
         FREPA(source.tokens)
         {
            Token &token=*source.tokens[i];
            if(token.line!=last_line && token==symbol_name)
            {
               source.parseFunc(token);
               if(Symbol *token_symbol=source.finalSymbol(i)) // always calculate because 'token.symbol' can be equal to FUNC_LIST or data type (not var)
                  if(token_symbol->sameSymbol(*symbol)) // if the same symbol
               {
                  if(!found){found=true; buildNew().set(S+"\""+source.loc.asText()+"\":", &source);}
                  buildNew().set(S+"   "+*token.line, token.line);
                  last_line=token.line;
               }
            }
         }
      }
      buildUpdate();
   }
}
/******************************************************************************/
void CodeEditor::findAllReferences(C UID &id)
{
   // TODO: support this
}
/******************************************************************************/
}}
/******************************************************************************/
