/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
static Bool Overwrite;
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Line::setRect(Int i)
{
   Flt lh=CE.ts.lineHeight();
   super::rect(Rect_LU(0, -lh*i + CE.fontSpaceOffset(), CE.ts.textWidth(T), lh));
}
void Line::setGui(Int i, GuiObj &parent)
{
   text_valid=false;
   if(T.parent()!=&parent)parent+=create(S, &CE.ts).visible(!CE.view_mode());
   setRect(i);
}
/******************************************************************************/
void Source::setOffset    () {lc_offset.set(slidebar[0].wantedOffset()/CE.ts.colWidth(), slidebar[1].wantedOffset()/CE.ts.lineHeight());}
void Source::setRegionSize()
{
   Vec2 size=0; REP(childNum())if(GuiObj *c=child(i))if(c->visible() && c->type()==GO_TEXT){MAX(size.x, GuiMaxX(c->rect())); MAX(size.y, GuiMaxY(c->rect()));}
   size.x+=CE.ts.colWidth()+CE.lineNumberSize(); size.y+=clientHeight()/2;
   virtualSize(&size);
   slidebar[0].offset(lc_offset.x*CE.ts. colWidth ());
   slidebar[1].offset(lc_offset.y*CE.ts.lineHeight());
}
void Source::resize()
{
   REPAO(     lines).setRect(i);
   REPAO(view_lines).setRect(i);
   rect(CE.sourceRect());
   setRegionSize();
   setScroll();

   suggestions_list.elmHeight(CE.ts.lineHeight()).textSize(CE.ts.size.y).columnWidth(0, CE.ts.lineHeight());
   suggestionsSetRect();
}
void Source::themeChanged()
{
   REPAO(     lines).text_valid=false;
   REPAO(view_lines).text_valid=false;
}
void Source::setGui()
{
   REPAO(lines).setGui(i, T);
   setRegionSize();
   suggestions_region  .moveToTop();
   suggestions_textline.moveToTop();
}
/******************************************************************************/
void Source::prepareForDraw()
{
   resize      ();
   validateView();
}
void Source::validateView()
{
   Flt const_view_y=(hasMsFocus() ? posCur(Ms.pos()).y-lc_offset.y : 0), // cursor position relative to zero which should remain constant
       const_full_y=lc_offset.y+const_view_y;
   Int const_line=Trunc(const_full_y); Flt frac=const_full_y-const_line;
   if(view_lines.elms() && view_lines[0].visible())const_line=view_lines[Mid(const_line, 0, view_lines.elms()-1)].line(); // top visible line

   if(CE.view_mode())
      if((lines.elms() && !view_lines.elms()) || view_comments!=CE.view_comments || view_funcs!=CE.view_funcs || view_func_bodies!=CE.view_func_bodies || view_private_members!=CE.view_private_members)setView(CE.view_comments, CE.view_funcs, CE.view_func_bodies, CE.view_private_members);

   REPAO(     lines).visible(!CE.view_mode());
   REPAO(view_lines).visible( CE.view_mode());

   if(CE.view_mode())const_line=realToView(const_line);
   lc_offset.y=const_line-const_view_y+frac;

   setRegionSize();
}
void Source::setView(Bool comments, Bool funcs, Bool func_bodies, Bool private_members)
{
   view_lines.del();
   view_comments       =comments;
   view_funcs          =funcs;
   view_func_bodies    =func_bodies;
   view_private_members=private_members;

   // copy to 'view_lines' hiding function bodies if necessary
   Int  func_level=0;
   Bool last_was_bracket=false; // if last encountered char was ')'
   Int  bracket_line=-1, bracket_col=-1;
   FREPAD(y, lines)
   {
      Line &src=lines[y];
      if(!(func_level && !src.length())) // don't copy if we're inside function and the line is empty
      {
         ViewLine &dest=view_lines.New(); dest.includeLine(y); dest.source=this;
         FREPA(src)
         {
            if(!func_bodies && !func_level && src.Type(i)==TOKEN_OPERATOR && src[i]==')'){last_was_bracket=true; bracket_line=y; bracket_col=i;}
            if((func_level || last_was_bracket) && src.Type(i)==TOKEN_OPERATOR && src[i]=='{')
            {
               if(!func_level)
                  if(CodeLine *cl=FindLine(view_lines, bracket_line))
               {
                  if(lines[bracket_line][bracket_col+1]==' ' && bracket_line==y)cl->remove(bracket_col+1);
                                                                                cl->insert(bracket_col+1, ";", TOKEN_OPERATOR);
               }
               func_level++;
            }
            if(!func_level)dest.cols.New().set(src[i], i, y, -1, src.Type(i));else dest.cols.New().remove();
            if( func_level && src.Type(i)==TOKEN_OPERATOR && src[i]=='}')func_level--;
            if(TokenType(src.Type(i)) && src[i]!=')')
            {
               if((src[i]=='C' && src.Type(i)==TOKEN_CODE && src.Type(i+1)!=TOKEN_CODE && src.Type(i-1)!=TOKEN_CODE)  // C macro for const
               || (src.Type(i)==TOKEN_KEYWORD                                                                      )) // const keyword (for simplicity just check if it's a keyword)
               {
                  bracket_line=y; bracket_col=i;
               }else
               {
                  last_was_bracket=false;
               }
            }
         }
      }
   }

   // remove "private" until "}" or "public" or "protected" found
   if(!private_members)
   {
      Int private_level=0;
      FREPA(view_lines)
      {
         ViewLine &line=view_lines[i];
         if(private_level && !line.cols.elms()) // if it's an empty line and we're inside 'private_level' then remove fully
         {
            view_lines.remove(i--, true); // decrease 'i' because it will get increased in FREPA
         }else
         FREPA(line)
         {
            if(!private_level && !lines[line.line()].preproc && line.isKeyword(i, "private") && line[i+7]==':')private_level=1; // check for ':' to overcome "struct A : private B" issues
            if( private_level)
            {
               if(!lines[line.line()].preproc)
               {
                  if(line.CodeLine::type(i)==TOKEN_OPERATOR)if(line[i]=='{')private_level++;else if(line[i]=='}')private_level--;
                  if(private_level==1 && (line.isKeyword(i, "public") || line.isKeyword(i, "protected")))private_level=0;
               }
               if(private_level)line.cols[i].remove();else
               {  // if we stopped removing private
                  REPD(j, i)if(ValidType(line.cols[j].type))goto at_least_one_valid; // check if there's at least one valid character in current line before current position
                  REPD(j, i)             line.cols[j].type=TOKEN_NONE;               // if not, then force all previous characters as spaces
               at_least_one_valid:;
               }
            }
         }
      }
   }

   if(!comments)
   {
      REPA(view_lines)
      {
         ViewLine &line=view_lines[i];
         FREPA(line.cols)if(line.cols[i].type==TOKEN_COMMENT)if(line.starts(i, SEP_LINE))i+=Length(SEP_LINE)-1;else line.cols[i].remove();
      }
   }

   // clean
   if(!comments || !funcs || !func_bodies || !private_members)Clean(view_lines);

   // set gui text
   FREPA(view_lines){ViewLine &line=view_lines[i]; T+=line.create(S, &CE.ts); line.setRect(i);}
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
GuiObj* Source::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   GuiObj *prev=mouse_wheel, *go=super::test(gpc, pos, mouse_wheel);
   if(go && Kb.ctrlCmd() && prev==mouse_wheel)mouse_wheel=this; // if didn't change mouse wheel focus and has general focus then set wheel focus as well (needed for zooming)
   return go;
}
void Source::update(C GuiPC &gpc)
{
   if(Kb.ctrlCmd() && contains(Gui.wheel())){CE.zoom(Ms.wheelI()); Ms.eatWheel();}

   super::update(gpc);

   if(slidebar[1].button[1]()
   || slidebar[1].button[2]())
      if(!isCurVisible())
   {
      forceCreateNextUndo(); clearSuggestions(); sel=-1;
      Clamp(cur.y, viewBeginPos(), viewEndPos());
   }

   highlight_time-=Time.ad();

   if(hasKbFocus())
   {
      Kb.requestTextInput();

      if(Kb.ctrlCmd() && !Kb.alt() && Kb.b(KB_UP  ))slidebar[1].button[1].push(); // scroll up
      if(Kb.ctrlCmd() && !Kb.alt() && Kb.b(KB_DOWN))slidebar[1].button[2].push(); // scroll down

      for(; Kb.k.any(); Kb.nextKey())
      {
         if(Kb.k(KB_BACK )                  && !Kb.k.alt()){if(Kb.k.ctrlCmd())delWordBack   ();else delBack   ();}else
         if(Kb.k(KB_DEL  ) && !Kb.k.shift() && !Kb.k.alt()){if(Kb.k.ctrlCmd())delWordForward();else delForward();}else
         if(Kb.k(KB_LEFT ) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curPrevWord   ();else curLeft   ();}else
         if(Kb.k(KB_RIGHT) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curNextWord   ();else curRight  ();}else
         if(Kb.k(KB_HOME ) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curViewBegin  ();else /*if(suggestions_region.visible())setSuggestion(0                      );else */curLineBegin();}else
         if(Kb.k(KB_END  ) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curViewEnd    ();else /*if(suggestions_region.visible())setSuggestion(suggestions_list.elms());else */curLineEnd  ();}else
         if(Kb.k(KB_PGUP ) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curDocBegin   ();else   if(suggestions_region.visible())setSuggestion(suggestions_list.cur-Trunc(suggestions_region.slidebar[1].length()/CE.ts.lineHeight()));else curPageUp  ();}else
         if(Kb.k(KB_PGDN ) &&                  !Kb.k.alt()){if(Kb.k.ctrlCmd())curDocEnd     ();else   if(suggestions_region.visible())setSuggestion(suggestions_list.cur+Trunc(suggestions_region.slidebar[1].length()/CE.ts.lineHeight()));else curPageDown();}else

         if( Kb.k.shift()   &&                Kb.k(KB_DEL ))cut  ();else
         if( Kb.k.shift()   &&                Kb.k(KB_INS ))paste(null, !Kb.k.ctrlCmd());else
         if( Kb.k.ctrlCmd() &&                Kb.k(KB_INS ))copy ();else
         if( Kb.k.ctrlCmd() &&  Kb.k.alt() && Kb.k(KB_UP  ))curPrevLevelBracket();else
         if( Kb.k.ctrlCmd() &&  Kb.k.alt() && Kb.k(KB_DOWN))curNextLevelBracket();else
         if(!Kb.k.ctrlCmd() &&  Kb.k.alt() && Kb.k(KB_UP  ))curPrevBracket     ();else
         if(!Kb.k.ctrlCmd() &&  Kb.k.alt() && Kb.k(KB_DOWN))curNextBracket     ();else
         if(!Kb.k.ctrlCmd() && !Kb.k.alt() && Kb.k(KB_UP  )){if(suggestions_region.visible())setSuggestion(suggestions_list.cur-1);else curUp  ();}else
         if(!Kb.k.ctrlCmd() && !Kb.k.alt() && Kb.k(KB_DOWN)){if(suggestions_region.visible())setSuggestion(suggestions_list.cur+1);else curDown();}else

         // overwrite
         if(Kb.kf(KB_INS) && !Kb.k.ctrlCmd() && !Kb.k.shift() && !Kb.k.alt())Overwrite^=1;else

         // escape
         if(KbSc(KB_ESC).pd() || Kb.k(KB_NAV_BACK))
         {
            if(suggestions_region.visible())clearSuggestions();else // hide suggestions
            if(CE.visibleOutput() || CE.visibleAndroidDevLog())
            {
               CE.visibleOutput       (false);
               CE.visibleAndroidDevLog(false);
            }else break; // don't eat the key when not processed
         }else

         // enter
         if(Kb.k(KB_ENTER) && Kb.k.ctrlCmd() && !Kb.k.shift())
         {
            if(Kb.k.alt())findAllReferences();
            else          jumpToCur();
         }else
         if(Kb.k(KB_ENTER) && !Const)
         {
            if(Kb.k.ctrlCmd() && Kb.k.shift())separator();else
            {
               delSel(true, false); // sets undo
               if(cur.y>=lines.elms())lines.setNum(cur.y+1);

               if(!Kb.k.ctrlCmd() &&  Kb.k.alt() && Kb.k.shift()){curNextBracket(); curRight();} // on Shift+Alt+Enter, used for writing new methods
               if( Kb.k.ctrlCmd() && !Kb.k.alt()                ){cur.x=lines[cur.y].length(); clearSuggestions();} // Ctrl+Enter = move to end of line and press enter

               Int   pos;
               Line &prev=lines[cur.y];
               if(suggestions_list()) // use suggestion
               {
                  autoComplete(true, false);
               }else
               if(CE.options.guided() &&
                  (
                     (prev.chrNext  (cur.x, &pos)=='{' && prev.chrBefore(pos)) // detect if we're before '{' and there's something before '{'
                    ||prev.chrBefore(cur.x, &pos)=='{'                         // or               after '{'
                  )
               )
               {
                  if(prev.chrBefore(pos)) // if there's something before '{'
                  {
                     // move '{' to next line
                     Line &next =lines.NewAt(cur.y+1); REP(AlignToTab(prev.start())          )next +=' '; next+='{'; // set "{"
                     Line &next2=lines.NewAt(cur.y+2); REP(AlignToTab(prev.start())+TabLength)next2+=' '; Int pos2; if(prev.chrNext(pos+1, &pos2))for(Int i=pos2; i<prev.length(); i++)next2.append(prev[i], prev.Type(i)); // set "   remains"
                                                     cur.x=AlignToTab(prev.start())+TabLength; // set cursor to "   |remains"
                     Int level=0; FREPAD(end, next2) // find any loose '}' in remainings, then move it to next line
                     {
                        if(next2[end]=='{' && next2.Type(end)==TOKEN_OPERATOR)level++;
                        if(next2[end]=='}' && next2.Type(end)==TOKEN_OPERATOR)level--;
                        if(level<0)
                        {
                           Line &next3=lines.NewAt(cur.y+3); REP(AlignToTab(prev.start()))next3+=' '; for(Int i=end; i<next2.length(); i++)next3+=next2[i]; // set "}"
                           next2.clip(end);
                           exist(0, cur.y+4); // make sure that there is at least 1 line after newly added '}'
                           break;
                        }
                     }
                     cur.y+=2;
                     prev.clip(pos); prev.clipSpaces();
                  }else
                  {
                     Line &next=lines.NewAt(cur.y+1); REP(AlignToTab(prev.start())+TabLength)next+=' '; Int pos2; if(prev.chrNext(pos+1, &pos2))for(Int i=pos2; i<prev.length(); i++)next.append(prev[i], prev.Type(i)); // set "   remains"
                                                    cur.x=AlignToTab(prev.start())+TabLength; // set cursor to "   |remains"
                     Int level=0; FREPAD(end, next) // find any loose '}' in remainings, then move it to next line
                     {
                        if(next[end]=='{' && next.Type(end)==TOKEN_OPERATOR)level++;
                        if(next[end]=='}' && next.Type(end)==TOKEN_OPERATOR)level--;
                        if(level<0)
                        {
                           Line &next2=lines.NewAt(cur.y+2); REP(AlignToTab(prev.start()))next2+=' '; for(Int i=end; i<next.length(); i++)next2+=next[i]; // set "}"
                           next.clip(end);
                           exist(0, cur.y+3); // make sure that there is at least 1 line after newly added '}'
                           break;
                        }
                     }
                     cur.y++;
                     prev.clip(pos+1); prev.clipSpaces();
                  }
                  changed(cur.y-4, 5);
               }else
               {
                  Line &next=lines.NewAt(cur.y+1);
                  Int  start=(prev.empty() ? prev.length() : prev.start()); MIN(start, cur.x);
                  FREP(start)next+=' '; for(Int i=cur.x; i<prev.length(); i++)next+=prev[i];
                  prev.clip(cur.x);
                  cur.x=start;
                  cur.y++;
                  changed(cur.y-1, 2);
               }
               clearSuggestions();
               makeCurVisible();
            }
         }else

         // tab
         if(Kb.k(KB_TAB) && !Kb.k.ctrlCmd())
         {
            if(!Const)
            if(!CE.options.ac_on_enter() && suggestions_list())autoComplete();else
            {
               setUndo();
               if(sel.y>=0) // move whole selection
               {
                  VecI2 min, max; curSel(min, max);
                  for(Int y=min.y; y<Min(max.y+(max.x!=0), lines.elms()); y++)
                  {
                     if(Kb.k.shift())
                     {
                        Int processed=0; REP(TabLength)if(lines[y].first()!=' ')break;else{lines[y].remove(0); processed++;}
                        if(sel.y==y)MAX(sel.x-=processed, 0);
                        if(cur.y==y)MAX(cur.x-=processed, 0);
                     }else
                     {
                        if(lines[y].length())REP(TabLength)lines[y].insert(0, ' ');
                     }
                  }
                  if(!Kb.k.shift())
                  {
                     if(sel.x)sel.x+=TabLength;
                     if(cur.x)cur.x+=TabLength;
                  }
                  if(sel==cur)sel=-1;
                  changed(min.y, max.y-min.y+1);
                  clearSuggestions();
               }else // in single line only (at cursor)
               {
                  if(Kb.k.shift()) // backwards
                  {
                     if(!InRange(cur.y, lines)     )cur.x=0;else
                     if(cur.x>lines[cur.y].length())cur.x=lines[cur.y].length();else
                     {
                        Int dest_x=Max(0, (cur.x-1)/TabLength*TabLength);
                        for(; cur.x>dest_x; cur.x--)
                        {
                           if(lines[cur.y][cur.x-1]!=' ')break;
                           lines[cur.y].remove(cur.x-1);
                        }
                     }
                  }else // forward
                  {
                     exist(cur.x, cur.y);
                     Int dest_x=(cur.x+TabLength)/TabLength*TabLength,
                         num   =dest_x-cur.x;
                     if (Overwrite)lines[cur.y].remove(cur.x, num);
                     REP(num      )lines[cur.y].insert(cur.x, ' ');
                     cur.x=dest_x;
                  }
                  changed(cur.y);
                  clearSuggestions();
                  makeCurVisible();
               }
            }
         }else

         // insert single character
         if(Kb.k.c && !Kb.k.ctrlCmd() && !Kb.k.lalt() && !Const)
         {
            CE.markCurPos();

            if(Suggestion *sugg=suggestions_list())
               if(CodeCharType(Kb.k.c)!=CHART_CHAR)
            {
               Bool ac=!CE.options.ac_on_enter(); // if want to auto-complete
               if(  ac)
               {
                  // check if we typed a label (for which auto-complete should be disabled)
                  if(Kb.k(':'))
                  {
                     Int i; if(Token *token=findPrevToken(cur, i))if(token->type==TOKEN_CODE && !(token->parent && token->parent->type==Symbol::CLASS) && InRange(i-1, tokens)) // labels can't be declared inside class (there private/protected/public can be used)
                     {
                        Token &token=*tokens[i-1];
                        if(token==';' || token=='{' || token=='}')ac=false;
                     }
                  }
               }
               if(ac)
               {
                  autoComplete(false, true, !Kb.k('('), false, Kb.k('.') || Kb.k('?'));
               }else
               {
                  if(InRange(suggestions_pos.y, lines))
                  {
                     Str &src=lines[suggestions_pos.y];
                     if(InRange(suggestions_pos.x, src))
                     {
                        Str s=src()+suggestions_pos.x; s.clip(cur.x-suggestions_pos.x);
                        if(Equal(s, sugg->text, true))SuggestionsUsed(s);
                     }
                  }
               }
            }

            Bool space=false;
         char_loop:
            {
               space|=Kb.k(' ');

               Bool has_sel=(sel.x>=0);

               if(has_sel)delSel(); // already calls undo
               else       setUndo((Kb.k('{') || Kb.k('(') || Kb.k('[') || Kb.k(';') || Kb.k(',') || Kb.k('"') || Kb.k('\'') || Kb.k('?') || Kb.k(':')) ? INS_CHRS : INS_CHR);

               exist(cur.x, cur.y);
               Line &prev=lines[cur.y];

               // add to suggestions a word we typed before
               if(CodeCharType(Kb.k.c)!=CHART_CHAR && !Overwrite)
               {
                  if(CodeCharType(prev[cur.x-1])==CHART_CHAR)
                  {
                     Str w; Int s=prev.wordStart(cur.x-1); for(; s<cur.x; s++)w+=prev[s];
                     SuggestionsUsed(w);
                  }
               }

               if(CE.options.guided())
               {
                  if(Kb.k('{')) // automatically place cursor at the end of the line when adding new bracket
                  {
                     if(!prev.empty() && !TextType(prev.Type(cur.x)))
                     {
                        if(prev.chrBefore(cur.x)==')' && prev.chrNext(cur.x)!=')') // we're in "if(x)|y=2;" situation -> change to "if(x){|y=2;}", support also "if(x)|if(x)x;else x;" -> "if(x){|if(x)x;else x;}"
                        {
                           for(Int i=cur.x, brackets=0, level=0; i<prev.length(); i++)//if(prev[i]==';' && prev.Type(i)==TOKEN_OPERATOR) // find first ';'
                           {
                              TOKEN_TYPE type=prev.Type(i); if(type!=TOKEN_COMMENT && !TextType(type))
                              {
                                 Char c=prev[i];
                                 if(c=='(')brackets++;else
                                 if(c==')')brackets--;else
                                 if(!brackets)
                                 {
                                    if(c=='{')level++;else
                                    if(c=='}')level--;
                                    if(!level)
                                    {
                                       if(c=='}' || c==';')
                                       {
                                          Bool next_else=false; // check if we're followed by else
                                          for(Int j=i+1; j<prev.length(); j++)
                                          {
                                             TOKEN_TYPE type=prev.Type(j); if(type!=TOKEN_COMMENT && type!=TOKEN_NONE)
                                             {
                                                if(type==TOKEN_KEYWORD && prev[j]=='e' && prev[j+1]=='l' && prev[j+2]=='s' && prev[j+3]=='e')next_else=true;
                                                break;
                                             }
                                          }
                                          if(!next_else)
                                          {
                                             prev.insert(i+1, '}', TOKEN_OPERATOR); // insert } after ; -> ";}"
                                             break;
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }else // we're in "void func(masndas|)" situation -> move to "void func(masndas)|"
                        if(insideRSTBrackets(cur))
                        {
                           cur.x=prev.end()+1;
                           exist(cur.x, cur.y);
                        }
                     }
                  }else
                  if(Kb.k(';')) // automatically place cursor at the end of params "|..)" -> "..)|"
                  {
                     Int bottom_level=0, level=0;
                     if(!TextType(prev.Type(cur.x)))
                        if(!Contains(prev, "for", true, true)) // 'for' is the only keyword that allows ';' inside "()" brackets
                           for(Int i=cur.x; i<prev.length(); i++)
                     {
                        if((prev[i]=='(' || prev[i]=='[') && prev.Type(i)==TOKEN_OPERATOR)level++;
                        if((prev[i]==')' || prev[i]==']') && prev.Type(i)==TOKEN_OPERATOR)level--;
                        if(level<bottom_level)
                        {
                           bottom_level=level;
                           cur.x=i+1;
                        }
                     }
                  }
               }

               if(Overwrite && !has_sel)prev.remove(cur.x); // don't use overwrite if we have selection
                                        prev.insert(cur.x, Kb.k.c);

               if(CE.options.guided() && !Overwrite)
               {
                  prev.resetType(); // this will help detect the TOKEN_TYPE of inserted character

                  if(!Kb.k(':') && !Kb.k(' ') && !Kb.k(';') && prev[cur.x-1]==':' && prev.Type(cur.x-1)==TOKEN_OPERATOR && prev[cur.x-2]!=':') // convert X:|X -> X : |X   (';' because of labels "label:;")
                  {
                     if(prev[cur.x-2]!=' ')prev.insert(cur.x++ -1, ' ');
                     if(prev[cur.x+1]!=' ')prev.insert(cur.x++   , ' ');
                  }

                  if(Kb.k('{'))
                  {
                     if(prev.start()==cur.x && prev.end()==cur.x+1) // if line consists only of '{'
                     {
                        if(!(InRange(cur.y+1, lines) && (lines[cur.y+1][cur.x]=='}' || lines[cur.y+1].start()>cur.x)))
                        {
                           lines.NewAt( cur.y+1);
                           lines.NewAt( cur.y+2);
                           exist(cur.x, cur.y+2);
                           lines[cur.y+2].insert(cur.x, '}');
                           exist(0, cur.y+3); // make sure that there is at least 1 line after newly added '}'
                           cur.y++;
                           cur.x+=TabLength-1;
                           REP(cur.x+1)lines[cur.y].insert(0, ' ');
                        }
                     }else
                     {
                        if(prev.end()==cur.x+1)prev.append('}');
                     }
                  }else
                  if(Kb.k('}'))
                  {
                     // align to previous '{'
                     Int level=0;
                     for(VecI2 p=cur-VecI2(1, 0); lineValid(p); dec(p))
                     {
                        if(T[p]=='{' && Type(p)==TOKEN_OPERATOR)level++;
                        if(T[p]=='}' && Type(p)==TOKEN_OPERATOR)level--;
                        if(level>0)
                        {
                           for(; cur.x>p.x && prev[cur.x-1]==' '; )prev.remove(--cur.x);
                           // overwrite any existing '}' that shouldn't be there
                           if(prev[cur.x+1]=='}')prev.remove(cur.x);
                           break;
                        }
                     }
                  }else
                  if(Kb.k('('))
                  {
                     prev.insert(cur.x+1, ')');
                     //if(prev.Type(cur.x+1)==TOKEN_NONE || prev[cur.x+1]==';' || prev[cur.x+1]==',')prev.insert(cur.x+1, ')');
                  }else
                  if(Kb.k(')'))
                  {
                     if(prev[cur.x+1]==')') // if there's another ')' after the one we just wrote to, then check if we can remove it
                     {
                        Int left =0; for(VecI2 p=cur            ; lineValid(p); dec(p))if(Type(p)==TOKEN_OPERATOR){Char c=T[p]; if(c=='{' || c=='}' || c==';')break; if(c=='(')left ++; if(c==')')left --;}
                        Int right=0; for(VecI2 p=cur+VecI2(1, 0); lineValid(p); inc(p))if(Type(p)==TOKEN_OPERATOR){Char c=T[p]; if(c=='{' || c=='}' || c==';')break; if(c=='(')right++; if(c==')')right--;}
                        if(left<=-right)prev.remove(cur.x); // right side already has enough brackets, we can remove this one
                     }
                  }else
                  if(Kb.k('['))
                  {
                     prev.insert(cur.x+1, ']');
                  }else
                  if(Kb.k(']'))
                  {
                     if(prev[cur.x+1]==']') // if there's another ']' after the one we just wrote to, then check if we can remove it
                     {
                        Int left =0; for(VecI2 p=cur            ; lineValid(p); dec(p))if(Type(p)==TOKEN_OPERATOR){Char c=T[p]; if(c=='{' || c=='}' || c==';')break; if(c=='[')left ++; if(c==']')left --;}
                        Int right=0; for(VecI2 p=cur+VecI2(1, 0); lineValid(p); inc(p))if(Type(p)==TOKEN_OPERATOR){Char c=T[p]; if(c=='{' || c=='}' || c==';')break; if(c=='[')right++; if(c==']')right--;}
                        if(left<=-right)prev.remove(cur.x); // right side already has enough brackets, we can remove this one
                     }
                  }else
                  if(Kb.k('"'))
                  {
                     if(prev[cur.x+1]=='"')prev.remove(cur.x);else // if we're already followed by '"'
                     {TOKEN_TYPE t=prev.Type(cur.x-1); if(!TextType(t))prev.insert(cur.x+1, '"');}
                  }else
                  if(Kb.k('\''))
                  {
                     if(prev[cur.x+1]=='\'')prev.remove(cur.x);else // if we're already followed by '\''
                     {TOKEN_TYPE t=prev.Type(cur.x-1); if(!TextType(t) && t!=TOKEN_COMMENT)prev.insert(cur.x+1, '\'');}
                  }else
                  if(Kb.k('.'))
                  {
                     // overwrite any existing '.' that shouldn't be there
                     TOKEN_TYPE t=prev.Type(cur.x-1); if(!TextType(t))if(prev[cur.x+1]=='.')prev.remove(cur.x);
                  }else
                  if(Kb.k(','))
                  {
                     if(prev.Type(cur.x)==TOKEN_OPERATOR) // if this became an operator
                     {
                        if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x+1, ' '); cur.x++; // after ','
                     }
                  }else
                  if(Kb.k('?'))
                  {
                     if(prev.Type(cur.x)==TOKEN_OPERATOR) // if this became an operator
                     {
                        if(prev.Type(cur.x-1)!=TOKEN_NONE)prev.insert(cur.x++, ' ');          // before '?'
                        if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x+1, ' '); cur.x++; // after  '?'
                     }
                  }else
                  if(Kb.k(':')) // "case X:", "label:", "a ? b : c"
                  {
                     if(prev.Type(cur.x)==TOKEN_OPERATOR) // if this became an operator
                        for(VecI2 p=cur-VecI2(1, 0); lineValid(p); dec(p))
                     {
                        if(Type(p)==TOKEN_OPERATOR)
                        {
                           Char c=T[p];
                           if(c=='?')
                           {
                              if(prev.Type(cur.x-1)!=TOKEN_NONE)prev.insert(cur.x++, ' ');          // before ':'
                              if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x+1, ' '); cur.x++; // after  ':'
                              break;
                           }
                           if(c=='{' || c=='}' || c==';')break;
                        }else
                        if(Type(p)==TOKEN_KEYWORD)
                        {
                           CChar *s=lines[p.y]()+p.x;
                           if(Starts(s, "case"   , true, true)
                           || Starts(s, "default", true, true))
                           {
                              if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x+1, ' '); cur.x++; // after  ':'
                              break;
                           }
                        }
                     }
                  }else
                  if(Kb.k('&')) // check for '&&'
                  {
                     if(prev[cur.x-1]=='&' && prev.Type(cur.x-1)==TOKEN_OPERATOR)
                     {
                        if(prev.Type(cur.x-2)!=TOKEN_NONE)prev.insert(cur.x++ -1, ' ');          // before "&&"
                        if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x   +1, ' '); cur.x++; // after  "&&"
                     }
                  }else
                  if(Kb.k('|')) // check for '||'
                  {
                     if(prev[cur.x-1]=='|' && prev.Type(cur.x-1)==TOKEN_OPERATOR)
                     {
                        if(prev.Type(cur.x-2)!=TOKEN_NONE)prev.insert(cur.x++ -1, ' ');          // before "||"
                        if(prev.Type(cur.x+1)!=TOKEN_NONE)prev.insert(cur.x   +1, ' '); cur.x++; // after  "||"
                     }
                  }
               }

               cur.x++;
               CE.markCurPos();

               if(CE.options.guided())
               {
                  if(Kb.k(';')          && prev.Type(cur.x  )!=TOKEN_NONE && prev[cur.x]!='}' && prev.Type(cur.x-1)==TOKEN_OPERATOR)prev.insert(cur.x++   , ' ');else // if we typed ';' which is followed by something then add space
                  if(prev[cur.x-2]==',' && prev.Type(cur.x-1)!=TOKEN_NONE                     && prev.Type(cur.x-2)==TOKEN_OPERATOR)prev.insert(cur.x++ -1, ' ');else // if we typed something after ',' then insert space between that
                  if(prev[cur.x-2]==';' && !Kb.k(' ') && !Kb.k('}')                                                                )prev.insert(cur.x++ -1, ' ');     // if we typed something after ';' then insert space between that

                  if(Kb.k(':')) // if we typed ':' after private,protected,public then convert "   private:|" to "private:\n   |"
                  {
                     Int    start=prev  .start();
                     CChar *s    =prev()+start  ;
                     if(Starts(s, "public"   , true, true)
                     || Starts(s, "protected", true, true)
                     || Starts(s, "private"  , true, true))
                     {
                        if(prev[cur.x-2]==' '){prev.remove(cur.x-2); cur.x--;} // remove any space between keyword and ':' ("private :|" -> "private:|")
                        Int rem=Min(start, TabLength); prev.remove(0, rem); start-=rem; start+=TabLength;
                        Line &l=lines.NewAt(cur.y+1); REP(start)l.insert(0, ' ');
                        cur.y++; cur.x=start;
                     }
                  }
                  if( Kb.k('/') &&               prev[cur.x-2]=='/' && prev[cur.x-3]!='/' && prev.Type(cur.x-3)!=TOKEN_NONE && !TextType(prev.Type(cur.x-2))){prev.insert(cur.x-2, ' '); cur.x++;} // if we started typing a     comment then make sure it's separated with at least one space
                  if(!Kb.k('/') && !Kb.k(' ') && prev[cur.x-2]=='/' && prev[cur.x-3]=='/'                                   && !TextType(prev.Type(cur.x-2))){prev.insert(cur.x-1, ' '); cur.x++;} // if we started typing after comment then make sure it's separated with at least one space
               }

               Keyboard::Key *k=Kb.nextKeyPtr(); if(k && k->c && !k->ctrlCmd() && !k->lalt()){Kb.nextKey(); goto char_loop;}
            }

            changed(cur.y-1, 3);
            makeCurVisible();
            listSuggestions(space ? -1 : 0);
            CE.markCurPos();
         }else

         break; // stop and don't eat using 'nextKey' in the 'for' loop
      }
   }else
   if(Gui.kb()==&suggestions_textline)
   {
      if(Kb.ctrlCmd() && Kb.b(KB_UP  ))slidebar[1].button[1].push();else // scroll up
      if(Kb.ctrlCmd() && Kb.b(KB_DOWN))slidebar[1].button[2].push();else // scroll down
      if(Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)){clearSuggestions(); Kb.eatKey();}else // hide suggestions
      if(Kb.k(KB_ENTER)){autoComplete(); Kb.eatKey();}else
      if(Kb.k(KB_PGUP))setSuggestion(suggestions_list.cur-Trunc(suggestions_region.slidebar[1].length()/CE.ts.lineHeight()));else
      if(Kb.k(KB_PGDN))setSuggestion(suggestions_list.cur+Trunc(suggestions_region.slidebar[1].length()/CE.ts.lineHeight()));else
      if(Kb.k(KB_UP  ))setSuggestion(suggestions_list.cur-1);else
      if(Kb.k(KB_DOWN))setSuggestion(suggestions_list.cur+1);
   }

   REPA(MT)if(hasFocus(MT.guiObj(i)))
   {
      if(MT.bp(i, 1))if(Kb.ctrlCmd())CE.nextCurPos();else CE.prevCurPos();
      if(CE.view_mode())
      {
         if(MT.bp(i, 0) || MT.bp(i, 2))
         {
            VecI2 view=Trunc(posCur(MT.pos(i))), real;
            if(viewToReal(view, real))
            {
               SymbolPtr symbol;
               Macro    *macro=null;
               UID       id;
               if(getSymbolMacroID(real, symbol, macro, id))
               {
                  cur=real;
                  if(Kb.ctrlCmd())
                  {
                     if(symbol    )CE.findAllReferences(symbol()   );else
                     if(macro     )CE.findAllReferences(macro->name);else
                     if(id.valid())CE.findAllReferences(id         );
                  }else
                  {
                     if(symbol    )CE.jumpTo(symbol());else
                     if(macro     )CE.jumpTo(macro   );else
                     if(id.valid())CE.cei().elmOpen(id);
                  }
               }
            }
         }
      }else
      {
         Vec2 c=posCur(MT.pos(i)); MAX(c.x, 0); MAX(c.y, 0);
         if(MT.bp(i, 2) || (Kb.ctrlCmd() && MT.bp(i, 0)))
         {
            sel=sel_temp=-1;
            cur.set(Trunc(c.x), Trunc(c.y));
            if(Kb.ctrlCmd() && MT.bp(i, 2))findAllReferences();
            else                           jumpToCur();
         }else
         if(MT.bd(i, 0))
         {
            sel_temp=-1;
            cur.set(Trunc(c.x), Trunc(c.y));
            curClip();
            selWord();
         }else
         if(MT.bp(i, 0))
         {
            forceCreateNextUndo(); clearSuggestions();
            if(Kb.shift() && (sel.x<0 || sel.y<0))sel=cur;
            cur.set(Round(c.x), Trunc(c.y));
            curClip();
            if(!Kb.shift()){sel=-1; sel_temp=cur;}
            makeCurVisible();
         }else
         if(MT.b(i, 0) && sel_temp.x>=0)
         {
            if(MT.pos(i).y>=_crect.max.y)slidebar[1].button[1].push();else
            if(MT.pos(i).y<=_crect.min.y)slidebar[1].button[2].push();
            cur.set(Round(c.x), Trunc(c.y));
            curClip();
            sel=sel_temp;
            if(sel==cur)sel=-1;
         }else
         if(MT.bp(i, 4))delBack();
      }
   }
   if(Gui.ms()==&suggestions_list)
   {
      if(Ms.bp(0))
      {
         if(suggestions_list.lit>=0)suggestions_list.cur=suggestions_list.lit;
         autoComplete();
      }else
      if(Ms.b(1))
      {
         if(suggestions_list.lit>=0)suggestions_list.cur=suggestions_list.lit;
      }else
      if(Ms.bp(2))
      {
         clearSuggestions();
      }else
      if(Ms.bp(4))
      {
         delBack();
      }
   }
   REPA(Touches)if(Touches[i].guiObj()==&suggestions_list && Touches[i].rs())
   {
      autoComplete();
   }

   if(CE.view_mode() && contains(Gui.kb()))
   {
      if(Kb.b(KB_LEFT ))slidebar[0].button[1].push();
      if(Kb.b(KB_RIGHT))slidebar[0].button[2].push();
      if(Kb.b(KB_UP   ))slidebar[1].button[1].push();
      if(Kb.b(KB_DOWN ))slidebar[1].button[2].push();
      if(Kb.k(KB_PGUP ))scrollY  (-slidebar[1].length());
      if(Kb.k(KB_PGDN ))scrollY  ( slidebar[1].length());
      if(Kb.k(KB_HOME ))scrollToY (0);
      if(Kb.k(KB_END  ))scrollEndY( );
   }

   setOffset();
}
/******************************************************************************/
// DRAW
/******************************************************************************/
static void HighlightFind(C Str &str, C Rect &rect, C GuiPC &gpc)
{
   for(Int offset=0; ; )
   {
      Int match_length, i=TextPosSkipSpaceI(str()+offset, CE.find.text(), match_length, CE.find.case_sensitive(), CE.find.whole_words()); if(i<0)break; offset+=i;
      Rect_LU(rect.lu()+gpc.offset+Vec2(offset*CE.ts.colWidth(), 0), match_length*CE.ts.colWidth(), CE.ts.lineHeight()).draw(ColorAlpha(YELLOW, 0.5f)); offset+=match_length;
   }
}
static void ShowElmName(C UID &id, C Rect &rect, C GuiPC &gpc, C VecI2 &range)
{
   Bool valid;
   Str  name=CE.cei().idToText(id, &valid);
   if(  name.is())
   {
      TextStyleParams ts=CE.ts_small; ts.align=0; ts.color=Theme.colors[TOKEN_ELM_NAME];
      Rect_LU r(rect.lu()+gpc.offset, Max((name.length()+0.5f)*ts.colWidth(), (range.y-range.x+1)*CE.ts.colWidth()), CE.ts.lineHeight()); r+=Vec2(range.x*CE.ts.colWidth(), 0);
      r.draw(valid ? Theme.colors[TOKEN_ELM_BACKGROUND] : Color(148, 0, 0, 233));
      D.text(ts, r.center(), name);
   }
}
static void ShowElmNames(C Str &str, C Rect &rect, C GuiPC &gpc)
{
   UID id; VecI2 range; FREPA(str)if(TextToIDAt(str, i, id, range))ShowElmName(id, rect, gpc, range);
}
/******************************************************************************/
void Line::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(rect().max.y+gpc.offset.y>=gpc.clip.min.y
      && rect().min.y+gpc.offset.y<=gpc.clip.max.y)
   {
      if(!text_valid){text_valid=true; code(textCode());}
      GuiPC gpc2=gpc ; gpc2.offset.x+=CE. lineNumberSize();
         // if spacing between elements is a fixed number of pixels then flickering can occur if positions will be at 0.5 pixels (0.5, 1.5, 2.5, ..), to prevent that from happening align the vertical position
         gpc2.offset+=D.alignScreenToPixelOffset(Vec2(gpc2.offset.x, CE.fontSpaceOffset()+CE.ts.posY(gpc2.offset.y)));
      GuiPC gpc3=gpc2; gpc3.offset.y-=CE.fontSpaceOffset();

      // highlight symbol
      if(source)if(Symbol *lit=source->lit_symbol)
      {
         Memc<Token> &tokens=Tokens();
         REPA(tokens)
         {
            Token &token=tokens[i];
            if(token.symbol==lit)goto highlight;
            if(token==source->lit_symbol_cpp_name)
            {
               if(!token.symbol){source->parseFunc(token); if(!token.symbol)token.symbol=source->finalSymbol(token.source_index);}
               if( token.symbol && token.symbol->type==Symbol::FUNC_LIST)if(Symbol *token_symbol=source->finalSymbol(token.source_index))token.symbol=token_symbol;
               if( token.symbol)if(lit->sameSymbol(*token.symbol))
               {
               highlight:;
                  D.clip(gpc3.clip);
                  Rect_LU(rect().lu()+gpc3.offset+Vec2(tokens[i].col*CE.ts.colWidth(), 0), tokens[i].length()*CE.ts.colWidth(), CE.ts.lineHeight()).draw(Theme.colors[TOKEN_SYMBOL_HIGHLIGHT]);
               }
            }
         }
      }

      super::draw(gpc2);
      if(CE.find.visible() && CE.find.text().is())HighlightFind(T, rect(), gpc3);
      if(CE.view_elm_names                       )ShowElmNames (T, rect(), gpc3);
   }
}
void Source::ViewLine::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(rect().max.y+gpc.offset.y>=gpc.clip.min.y
      && rect().min.y+gpc.offset.y<=gpc.clip.max.y)
   {
      if(!text_valid){text_valid=true; code(textCode());}
      GuiPC gpc2=gpc;    gpc2.offset.x+=CE. lineNumberSize();
         // if spacing between elements is a fixed number of pixels then flickering can occur if positions will be at 0.5 pixels (0.5, 1.5, 2.5, ..), to prevent that from happening align the vertical position
         gpc2.offset+=D.alignScreenToPixelOffset(Vec2(gpc2.offset.x, CE.fontSpaceOffset()+CE.ts.posY(gpc2.offset.y)));
      super::draw(gpc2); gpc2.offset.y-=CE.fontSpaceOffset();
      if(CE.find.visible() && CE.find.text().is())HighlightFind(asStr(), rect(), gpc2);
      if(CE.view_elm_names                       )ShowElmNames (asStr(), rect(), gpc2);
   }
}
/******************************************************************************/
void Source::drawSelection(C Color &color, Int y, Int min_x, Int max_x)
{
   Vec2 pos=rect().lu()+offset(); D.alignScreenToPixel(pos);
   pos+=D.alignScreenToPixelOffset(Vec2(pos.x, CE.fontSpaceOffset()+CE.ts.posY(pos.y)));
   pos+=posVisual(VecI2(min_x, y));
   Rect_LU(pos, (max_x>=min_x) ? CE.ts.colWidth()*(max_x-min_x) : D.w()-pos.x, CE.ts.lineHeight()).draw(color);
}
/******************************************************************************/
void Source::drawSelection(C Color &color, C VecI2 &a, C VecI2 &b, Bool including)
{
   VecI2 min=a, max=b;
   if(max.y<min.y || (max.y==min.y && max.x<min.x))Swap(min, max);

   if(min.y==max.y)drawSelection(color, min.y, min.x, max.x+including);else
   {
                                      drawSelection(color, min.y, min.x,    -1);
      for(Int y=min.y+1; y<max.y; y++)drawSelection(color,     y,     0,    -1);
                                      drawSelection(color, max.y,     0, max.x+including);
   }
}
/******************************************************************************/
void Source::draw(C GuiPC &gpc)
{
   if(gpc.visible && visible())
   {
      D.clip(_crect);

      Vec2 offset=rect().lu()+T.offset(); D.alignScreenToPixel(offset);
      offset+=D.alignScreenToPixelOffset(Vec2(offset.x, CE.fontSpaceOffset()+CE.ts.posY(offset.y)));

      if(!CE.view_mode())
      {
         // draw selection
         if(sel.x>=0 && sel.y>=0)drawSelection(Theme.colors[TOKEN_SELECT], sel, cur);

         // draw braces highlight
         if(InRange(cur.y, lines))
         {
            VecI2 test=cur; Char c=lines[test.y][test.x];
            if(c!='{' && c!='}' && c!='(' && c!=')' && c!='[' && c!=']')c=lines[test.y][--test.x]; // try the previous char
            if(c=='{' || c=='}' || c=='(' || c==')' || c=='[' || c==']')
            {
               TOKEN_TYPE type=MainType(lines[test.y].Type(test.x));
               VecI2      pos =test; Int level=0;
               if(c=='{'){for(inc(pos); posValid(pos); inc(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='{')level++;else if(c=='}')level--; if(level<0)break;}}else
               if(c=='}'){for(dec(pos); posValid(pos); dec(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='{')level++;else if(c=='}')level--; if(level>0)break;}}else
               if(c=='('){for(inc(pos); posValid(pos); inc(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='(')level++;else if(c==')')level--; if(level<0)break;}}else
               if(c==')'){for(dec(pos); posValid(pos); dec(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='(')level++;else if(c==')')level--; if(level>0)break;}}else
               if(c=='['){for(inc(pos); posValid(pos); inc(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='[')level++;else if(c==']')level--; if(level<0)break;}}else
               if(c==']'){for(dec(pos); posValid(pos); dec(pos)){Char c=lines[pos.y][pos.x]; if(type==MainType(lines[pos.y].Type(pos.x)))if(c=='[')level++;else if(c==']')level--; if(level>0)break;}}

                                              Rect_LU(offset+posVisual(test), CE.ts.colWidth(), CE.ts.lineHeight()).draw(Theme.colors[TOKEN_BRACE_HIGHLIGHT]);
               if(pos!=test && posValid(pos)){Rect_LU(offset+posVisual(pos ), CE.ts.colWidth(), CE.ts.lineHeight()).draw(Theme.colors[TOKEN_BRACE_HIGHLIGHT]); drawSelection(Theme.colors[TOKEN_BRACE_HIGHLIGHT], pos, test, true);}
            }
         }

         // draw current line highlight
         if(Theme.colors[TOKEN_LINE_HIGHLIGHT].a)
         {
            Int  line=cur.y; if(CE.view_mode())line=realToView(line);
            Vec2 pos =offset+posVisual(VecI2(0, line));
            D.lineX(Theme.colors[TOKEN_LINE_HIGHLIGHT], pos.y                   , _crect.min.x, _crect.max.x);
            D.lineX(Theme.colors[TOKEN_LINE_HIGHLIGHT], pos.y-CE.ts.lineHeight(), _crect.min.x, _crect.max.x);
         }
      }

      if(InRange(highlight_line, lines))
      {
         Int  line=highlight_line; if(CE.view_mode())line=realToView(line);
         Vec2 pos =offset+posVisual(VecI2(0, line));
         Rect(_crect.min.x, pos.y-CE.ts.lineHeight(), _crect.max.x, pos.y).draw(ColorAlpha(CYAN, highlight_time*0.5f));
      }

      // highlight token definition
      Source *lit_token_source=null;
      Int     lit_token_line  =-1;
      UID     lit_elm_id=UIDZero;
      if(CE.view_mode() && hasMsFocus())
      {
         VecI2 view=Trunc(posCur(Ms.pos())), real;
         if(viewToReal(view, real))
         {
            SymbolPtr symbol;
            Macro    *macro=null;
            VecI2     x_range;
            if(getSymbolMacroID(real, symbol, macro, lit_elm_id, &x_range))
            {
               // highlight line of the definition
               if(macro  && macro ->source)                                                                          {lit_token_source=macro ->source; lit_token_line=                               macro->line;}
               if(symbol && symbol->source){Int token=symbol->token_index; if(InRange(token, symbol->source->tokens)){lit_token_source=symbol->source; lit_token_line=symbol->source->tokens[token]->lineIndex();}}
               if(lit_token_source==this && lit_token_line>=0)
               {
                  Vec2 pos=offset+posVisual(VecI2(0, realToView(lit_token_line)));
                  Rect lit_rect(_crect.min.x, pos.y-CE.ts.lineHeight(), _crect.max.x, pos.y);
                       lit_rect.draw(ColorAlpha(CYAN, 0.2f));
                  Rect screen_crect=(_crect-_crect.lu() + screenPos() + _crect.lu()-rect().lu()).extend(CE.ts.lineHeight()*-1.5f);
                  if(Cuts(lit_rect, screen_crect))lit_token_source=null; // if definition is on the screen then don't draw its preview
               }

               // highlight token
               x_range.x=view_lines[view.y].findCol(x_range.x);
               x_range.y=view_lines[view.y].findCol(x_range.y);
               if(x_range.x>=0 && x_range.y>=0)drawSelection(Theme.colors[TOKEN_SELECT], view.y, x_range.x, x_range.y+1);
            }
         }
      }

      // highlight all occurences of token under keyboard cursor
      lit_symbol=null;
      if(!CE.view_mode() && hasKbFocus() && !CE.find.visible()) // don't display when finding something
      {
         SymbolPtr symbol;
         Macro    *macro=null;
         UID       id=UIDZero;
         if(getSymbolMacroID(cur, symbol, macro, id))lit_symbol=symbol();
      }
      lit_symbol_cpp_name=(lit_symbol ? lit_symbol->shortName() : S);

      // draw text lines and children
      super::draw(gpc);
      D.clip(_crect);

      // draw cursor
      if(hasKbFocus() && App.active() && !Kb._hidden)
      {
         Vec2 pos=offset+posVisual(cur); pos+=D.pixelToScreenSize(VecI2(1, 0));
         if(Overwrite && sel.x<0)DrawKeyboardCursorOverwrite(pos, CE.ts.lineHeight(), CE.ts, '\0'); // don't draw overwrite if we have selection
         else                    DrawKeyboardCursor         (pos, CE.ts.lineHeight());
      }

      // draw line numbers
      if(CE.options.line_numbers())
      {
         Rect r=_crect; /*r.min.x+=D.pixelToScreenSize().x*0.5f;*/ r.max.x=r.min.x+CE.lineNumberSize();
         Flt  offset  =slidebar[1].offset(), line_offset=offset/CE.ts.lineHeight();
         Int  line_add=Trunc(line_offset);
         r.draw(Theme.colors[TOKEN_LINE_NUM_BACKGROUND]);
         TextStyleParams ts=CE.ts_small; ts.color=Theme.colors[TOKEN_LINE_NUM]; ts.align.set(1, -1); ts.space.y=CE.ts.lineHeight()/ts.size.y;
         Char8 text[1024], temp[256]; text[0]=0; Int lines=Ceil(r.h()/CE.ts.lineHeight())+1; Clamp(lines, 0, 1024); FREP(lines){Int li=line_add+i; Append(text, TextInt(CE.view_mode() ? viewToReal(li) : li, temp)); Append(text, "\n");}
         r+=Vec2(0, (line_offset-line_add)*CE.ts.lineHeight() + (CE.ts.lineHeight()-CE.ts_small.lineHeight())*-0.2f);
         D.text(ts, r, text);
      }

      // draw function information
      if(hasKbFocus() && !Kb.b(KB_RCTRL)) // Right Control can be used to hide it
      {
         Int c; if(findPrevToken(cur, c))for(Int i=c, level=0; i>=0; i--)
         {
            Int    func_i=i;
            Token &func  =*tokens[func_i];
            if(func=='(')level++;else
            if(func==')')level--;
            if(func=='{' || func=='}' || func==';')break;
            if(func.type==TOKEN_CODE && level>0)
            {
               if(InRange(func_i+1, tokens))
               {
                  if(CE.view_elm_names && func=="UID" && func.line) // don't display info for this function if we're inside project element UID(..), instead continue to previous function "func(|UID(..))"
                  {
                     UID id; VecI2 range; if(TextToIDAt(*func.line, func.col, id, range))if(CE.cei().idToText(id).is())continue; // only if it's a text ID
                  }
                  Token &op=*tokens[func_i+1];
                  if(op=='(' || op=='<' || op==TMPL_B) // if func name is followed by '<' or '(' : "func(..)", "func<template>()", ..
                  {
                     parseCurFunc(); // parse func to detect templates and vars
                     if(Symbol *symbol=finalSymbol(func_i, true))
                        if(symbol->type==Symbol::FUNC || symbol->type==Symbol::FUNC_LIST || symbol->isVar() || symbol->type==Symbol::CLASS) // function, operator, constructor (through class "Vec2(0, 0)" or default value "Vec2 x(0, 0)")
                           if(!(symbol->type==Symbol::FUNC && func.def_decl && (symbol->params.elms() || (symbol->modifiers&Symbol::MODIF_FUNC_BODY)))) // don't use when defining function (and typing params)
                     {
                        Bool force_ctor=(symbol->type==Symbol::FUNC && func.def_decl && !symbol->params.elms() && !(symbol->modifiers&Symbol::MODIF_FUNC_BODY));
                        if(symbol->type==Symbol::CLASS)
                        {
                           if(func==*symbol && func.parent==symbol)symbol=null; // defining constructor/destructor, don't list parameters
                           if(symbol && InRange(func_i-2, tokens)) // X.X  X.~X // defining constructor/destructor, don't list parameters
                           {
                              Token &token=*tokens[func_i-1],
                                    &prev =*tokens[func_i-2];
                              if(token=='.' && prev==func
                              || token=='~' && prev=='.' && (InRange(func_i-3, tokens) && *tokens[func_i]==func))symbol=null;
                           }
                           if(symbol)symbol=SymbolPtr().find(symbol->full_name+SEP+*symbol)(); // set symbol to constructor function list
                        }
                        if(symbol)for(; i<tokens.elms(); )
                        {
                           Token &op=*tokens[i++];
                           if(op=='(')
                           {
                              Memc<Message> msgs;
                              Compiler      compiler(msgs, tokens, this, null); compiler.relax();

                              // detect parameters
                              Int        cur_param=0;
                              Memc<Expr> params;
                              if(InRange(i, tokens))for(Int level=0, param_start=i; ; i++)
                              {
                                 Token &token=*tokens[i];
                                 if(token=='(' || token=='[' || token==TMPL_B)level++;else
                                 if(token==')' || token==']' || token==TMPL_E)level--;
                                 Bool end         =(token==';' || level< 0 || token=='{' || token=='}' || i>=tokens.elms()-1),
                                      finish_param=(token==',' && level==0);
                                 if(end || finish_param)
                                 {
                                    Int param_end=i-1;
                                    if( param_end>=param_start // valid range
                                    ||  finish_param           // after ','
                                    || (end && params.elms())) // encountered end without valid range but with previous elements present
                                    {
                                       if(c+1>=param_start)cur_param=params.elms();
                                       compiler.compileTokens(param_start, param_end, params.New());
                                    }
                                    param_start=i+1;
                                 }
                                 if(end)break;
                              }

                              Memc<FuncMatch    > matches;
                              Memc<Symbol::Modif> templates;
                              if(!force_ctor && symbol->type==Symbol::FUNC && symbol->parent && symbol->parent->type==Symbol::FUNC_LIST)symbol=symbol->parent(); // display all functions
                              if(!force_ctor && symbol->type==Symbol::FUNC     )AddMatch  (matches, symbol       , true, params.data(), params.elms(), templates, compiler);else
                              if(!force_ctor && symbol->type==Symbol::FUNC_LIST)AddMatches(matches, symbol->funcs, true, params.data(), params.elms(), templates, compiler);else
                              {
                                 // variable
                                 if(symbol=GetFinalSymbol(symbol))if(symbol->type==Symbol::CLASS)
                                 {
                                    if(func.def_decl) // being defined, so it's a constructor
                                    {
                                       if(symbol=SymbolPtr().find(symbol->full_name+SEP+*symbol)()) // set symbol to constructor function list
                                          AddMatches(matches, symbol->funcs, true, params.data(), params.elms(), templates, compiler);
                                    }else // was already defined, so it's an operator
                                    {
                                       if(symbol=FindChild("operator()", symbol)()) // get class () operators
                                          AddMatches(matches, symbol->funcs, true, params.data(), params.elms(), templates, compiler);
                                    }
                                 }
                              }

                              // remove double functions (definition/declaration)
                              REPA(matches){Symbol *a=matches[i].func; REPD(j, i)if(a->sameFunc(*matches[j].func)){matches.remove((a->modifiers&Symbol::MODIF_FUNC_BODY) ? i : j, true); break;}} // remove the one with function body (because it doesn't have default parameters listed, and full class path)

                              matches.sort(CompareAverage);

                              if(matches.elms())
                              {
                                 Memc<Str> t;
                               //Int       match=matches[0].average_match;
                                 Int       match=matches.last().lowest_match; REPA(matches)MAX(match, matches[i].lowest_match);
                                 FREPA(matches){FuncMatch &fm=matches[i]; t.New()=S+((fm.lowest_match==match) ? "[col=000F]" : "[col=0008]")+fm.func->funcDefinition(cur_param)+fm.func->commentsCode()+"[/col]";}

                                 Str code; FREPA(t){if(i)code+='\n'; code+=t[i];}
                                 Str text; Memt<TextCodeData> codes; SetTextCode(code, text, codes);
                                 Flt w=0; t=Split(text, '\n'); REPA(t)MAX(w, CE.ts_small.textWidth(t[i]));
                                 MIN(w, clientWidth()*0.9f);

                                 Vec2 fp  =offset+Vec2(0, CE.fontSpaceOffset())+posVisual(VecI2(func.col, cur.y)), // use cursor.y so information is displayed below cursor (needed for multi-line functions)
                                      size=suggestions_region.size(),
                                      pos =suggestions_region.screenPos();

                                 Flt     ext=0.01f;
                                 Rect_LU r(fp.x, suggestions_region.visible() ? pos.y-size.y-ext : fp.y-CE.ts.lineHeight()-ext, w, 0); if(r.max.x>=_crect.max.x)r-=Vec2(r.max.x-_crect.max.x, 0);
                                 Int     lines=CE.ts_small.textLines(text, r.w(), AUTO_LINE_SPACE_SPLIT); r.min.y=r.max.y-lines*CE.ts_small.lineHeight();
                                 Rect(r).extend(ext).draw(WHITE);
                                 Rect(r).extend(ext).draw(Color(0, 0, 0, 112), false);
                                 CE.ts_small.drawCode(r, text, AUTO_LINE_SPACE_SPLIT, codes.data(), codes.elms());
                              }
                              break;
                           }
                        }
                     }
                  }
               }
               break;
            }
         }
      }

      // draw suggestion information
      if(Suggestion *sugg=suggestions_list())
      {
         Symbol *symbol=sugg->symbol();
         if(symbol || sugg->is_macro)
         {
            Memc<Str> t;
            if(symbol)
            {
               // get list of functions
               Memc<Symbol*> funcs; FREPA(symbol->funcs)funcs.add(symbol->funcs[i]());

               // remove double functions (definition/declaration)
               REPA(funcs){Symbol *a=funcs[i]; REPD(j, i)if(a->sameFunc(*funcs[j])){funcs.remove((a->modifiers&Symbol::MODIF_FUNC_BODY) ? i : j, true); break;}} // remove the one with function body (because it doesn't have default parameters listed, and full class path)

               // use function if only one is present
               if(symbol->type==Symbol::FUNC_LIST && funcs.elms()==1)symbol=funcs[0];

               t.New()="Type:";
               t.New()=S+"   "+symbol->typeName();
               if(symbol->type!=Symbol::FUNC_LIST)
               {
                  if(t.elms())t.New();
                  t.New()="Definition:";
                  t.New()=S+"   "+symbol->definition();
               }
               if(symbol->type!=Symbol::KEYWORD && symbol->type!=Symbol::PREPROC)
               {
                  if(t.elms())t.New();
                  t.New()="Full name:";
                  t.New()=S+"   "+symbol->fullCppName();
               }
               Str comments=symbol->comments();
               if( comments.is())
               {
                  Memc<Str> c=Split(S+"   "+comments, '\n');
                  if(t.elms())t.New();
                  t.New()="Comments:";
                  FREPA(c)Swap(t.New(), c[i]);
               }
               if(symbol->type==Symbol::FUNC_LIST)
               {
                  if(t.elms())t.New();
                  t.New()="Functions:";
                  FREPA(funcs)t.New()=S+"   "+funcs[i]->funcDefinition(-1)+funcs[i]->commentsCode();
               }
            }else
            {
               t.New()="Type:";
               t.New()="   Macro";
               t.New();
               t.New()="Definition:";
               t.New()=S+"   "+sugg->macro_def;
            }

            Str code; FREPA(t){if(i)code+='\n'; code+=t[i];}
            Str text; Memt<TextCodeData> codes; SetTextCode(code, text, codes);
            Flt w=0; t=Split(text, '\n'); REPA(t)MAX(w, CE.ts_small.textWidth(t[i]));
            MIN(w, D.pixelToScreenSize().x*520);

            Vec2 size=suggestions_region.size(),
                 pos =suggestions_region.screenPos();

            Flt     space=D.pixelToScreenSize().x*24;
            Rect_LU r(pos+Vec2(size.x+space, 0), w, 0); if(r.max.x>=D.w())r-=Vec2(r.max.x-pos.x+space, 0); if(r.min.x<rect().min.x)r+=Vec2(pos.x+size.x+space-r.min.x, 0);
            Int     lines=CE.ts_small.textLines(text, r.w(), AUTO_LINE_SPACE_SPLIT); r.min.y=r.max.y-lines*CE.ts_small.lineHeight();

            if(Gui.skin)
            {
               Rect rect=r; rect.extend(0.01f);
               if(Gui.skin->region.normal)Gui.skin->region.normal->draw(Gui.skin->region.normal_color, rect);else
               if(Gui.skin->region.normal_color.a)            rect.draw(Gui.skin->region.normal_color);
            }
            CE.ts_small.drawCode(r, text, AUTO_LINE_SPACE_SPLIT, codes.data(), codes.elms());
         }
         if(sugg->elm_id.valid()) // project element
         {
            Rect    rect =suggestions_region.screenRect()|suggestions_textline.screenRect();
            Flt     space=D.pixelToScreenSize().x*24;
            Rect_LU r(rect.lu()+Vec2(rect.w()+space, 0), rect.size()); if(r.max.x>=D.w())r-=Vec2(r.max.x-rect.min.x+space, 0); if(r.min.x<rect.min.x)r+=Vec2(rect.max.x+space-r.min.x, 0);
            CE.cei().elmPreview(sugg->elm_id, r.lu(), false, r);
         }
      }

      // draw too long lines
      if(CE.view_mode() && hasMsFocus())
      {
         Vec2 cur=posCur(Ms.pos());
         Int  l  =Trunc(cur.y); if(InRange(l, view_lines))
         {
            ViewLine &view_line=view_lines[l];
            if(view_line.rect().w()>=clientWidth()*0.95f)
            {
               ViewLine cl=view_line;
               for(; cl[0]==' '; )cl.remove(0); // skip start spaces
               // remove spaces and insert lines
               REPA(cl)if(cl[i]==' ')
               {
                  if(cl[i-1]==' ' // "  " -> " "
                  || cl[i-1]=='(' // "( " -> "("
                  || cl[i+1]==')' // " )" -> ")"
                  || cl[i+1]==',' // " ," -> ","
                  || cl[i+1]==';' // " ;" -> ";"
                  || cl[i-1]=='=' // "= " -> "="
                  || cl[i-2]=='(' && cl[i-1]=='*' // "(* " -> "(*"
                  || cl[i-2]==' ' && cl[i-1]=='&' // " & " -> " &"
                  || cl[i+1]=='(' && cl.CodeLine::type(i)==TOKEN_NONE // " (" -> "("
                  )cl.remove(i);else
                  if(cl[i-1]==';'                 // "; "  -> ";\n"
                  || cl[i+1]=='/' && cl[i+2]=='/' // " //" -> "\n//"
                  )cl.cols[i].c='\n';
               }
                                                                          
               // align 1st and 2nd lines
               // "void set(Int x);" -> "void set(Int x);"
               // "Int get();      "    "Int  get();     "
               FREPA(cl)
               {
                  if(cl[i]=='\n')break;
                  if(cl[i]!=' ' && cl[i-1]==' ') // find first text after space
                  {
                     for(Int l=i+1; l<Elms(cl); l++)if(cl[l]=='\n') // find next line
                     {
                        l++;
                        if(cl[l]!='/') // 2nd line does not start from comment
                           for(Int j=l; j<Elms(cl); j++)
                        {
                           if(cl[j]=='\n')break;
                           if(cl[j]!=' ' && cl[j-1]==' ') // find first text after space
                           {
                              Int ai=i, aj=j-l; // distance from start of the line to texts
                              Str spaces; REPD(s, Abs(ai-aj))spaces+=' ';
                              cl.insert((ai<aj) ? i : j, spaces, TOKEN_NONE);
                              break;
                           }
                        }
                        break;
                     }
                     break;
                  }
               }
               Str  code =cl.textCode(), text; Memt<TextCodeData> codes; SetTextCode(code, text, codes);
               Vec2 pos  =screenPos();
               Flt  min_x=pos.x+              0.1f,
                    max_x=pos.x+clientWidth()-0.1f,
                    max_y=Ms.pos().y-0.15f;

               Int  lines=CE.ts.textLines(text, max_x-min_x, AUTO_LINE_SPACE_SPLIT);
               Flt  min_y=max_y-lines*CE.ts.lineHeight();
               Rect rect (min_x, min_y, max_x, max_y);
               if(rect.min.y-0.05f<=pos.y-clientHeight())rect+=Vec2(0, rect.h() + 0.15f*2);
               Rect rect_e=rect; rect_e.extend(0.02f);
               D.drawShadow(190, rect_e, 0.0875f);
               rect_e.draw (Theme.colors[TOKEN_NONE]);
               CE.ts.drawCode(rect, text, AUTO_LINE_SPACE_SPLIT, codes.data(), codes.elms());
            }
         }
      }

      // draw source preview
      if(lit_token_source && lit_token_line>=0)
      {
         if(lit_token_source!=this)lit_token_source->prepareForDraw();

         GuiPC gpc;
         gpc.visible    =true;
         gpc.enabled    =true;
         gpc.clip       =(_crect-_crect.lu() + screenPos() + _crect.lu()-rect().lu()).extend(-0.05f); gpc.clip.min.x=gpc.clip.lerpX(0.6f);
         gpc.client_rect=gpc.clip;
         gpc.offset     =gpc.clip.left();
         gpc.offset.y  +=(lit_token_source->realToView(lit_token_line)+0.5f)*CE.ts.lineHeight();

         Rect rect_e=gpc.clip; rect_e.extend(0.01f);
         D.drawShadow(190, rect_e, 0.0875f);
         rect_e.draw (Theme.colors[TOKEN_NONE]);
         Rect_L(gpc.clip.left(), gpc.clip.w(), CE.ts.lineHeight()).draw(ColorAlpha(CYAN, 0.2f));
         REPAO(lit_token_source->     lines).draw(gpc);
         REPAO(lit_token_source->view_lines).draw(gpc);
         D.clip(null);
      }

      // draw element preview
      CE.cei().elmPreview(lit_elm_id, Ms.pos(), true, _crect);
   }
}
/******************************************************************************/
}}
/******************************************************************************/
