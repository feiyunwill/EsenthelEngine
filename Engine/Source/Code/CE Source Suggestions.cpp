/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Memc<Str> Suggestions;
const Int OrderUnknown=-2; // to be smaller than 'SuggestionsOrder' result for unknown text
/******************************************************************************/
static Int SuggestionsOrder(C Str &t)
{
   REPA(Suggestions)if(Equal(Suggestions[i], t, true))return i;
   return -1;
}
void SuggestionsUsed(C Str &t)
{
   Suggestions.remove(SuggestionsOrder(t), true);
   Suggestions.add(t);
   if(Suggestions.elms()>16)Suggestions.remove(0, true);
}
/******************************************************************************/
// Factors:
// -word length (prefer those that are shorter)
// -last usage
// -if shift used the add bonus when matches
// -if string part match starts with case up in suggesion part
// -if string part match corresponds to the same index
// -character equality (use distance in buttons on keyboard?)
static Int EqualChars(CChar *t, CChar *s, Int &p, Bool start_case) // assumes that 't' and 's' != null
{
        p=0; // priority
   Int  e=0; // equal characters
   Char start=*s;
   for(;;)
   {
      Char ct=*t++,
           cs=*s++;
      if(Compare(ct, cs))break;
      if(!ct)break;
      e++;
      if(CaseUp(ct)==ct && ct==cs)p++; // if upper case and matches exactly then increase priority
   }
   p+=e*2;
   if(e && start_case && CaseUp(start)==start)p++; // if found any match and 's' starts with upper case, then increase priority
   return e;
}
Int SuggestionsPriority(C Str &s, C Str &t, Bool all_up_case) // 's'=suggestion, 't'=text
{
   Int priority=0;
   for(Int ti=0; ti<t.length(); ) // check every part of 't' text
   {
      Int max_p=0, final_ec=0;
      for(Int si=0; si<s.length(); si++) // find max number of continuous chars equal in self starting from 't+ti'
      {
         Int p, ec=EqualChars(t()+ti, s()+si, p, ti!=0 && si!=0 && !all_up_case); // don't test case at start of 't' and 's'
         if( p)
         {
            p=p*p*(20+(si==ti)+(ti==0)*3); // boost priority when at the same place, or starting
            if(p>max_p){max_p=p; final_ec=ec;}
         }
      }
      priority+=max_p;
      ti+=Max(1, final_ec);
      if(final_ec==s.length() && s.length()==t.length() && Equal(s, t, true))priority++; // if this is an exact match then boost priority
   }
   return priority;
}
/******************************************************************************/
static Int CompareAlphabetical(C Source::Suggestion &a, C Source::Suggestion &b)
{
   Int as=0; FREPA(a.text)if(a.text[i]=='_')as++;else break; // level of '_' signs
   Int bs=0; FREPA(b.text)if(b.text[i]=='_')bs++;else break; // level of '_' signs
   return (as==bs) ? Compare(a.text, b.text, false) : (as<bs) ? -1 : +1;
}
static Int CompareAlphabeticalCS(C Source::Suggestion &a, C Source::Suggestion &b) {return Compare(a.text, b.text, true);}
static Int ComparePriority      (C Source::Suggestion &a, C Source::Suggestion &b)
{
   if(Int c=Compare(a.priority     , b.priority     ))return -c;
   if(Int c=Compare(a.order()      , b.order()      ))return -c;
   if(Int c=Compare(a.text.length(), b.text.length()))return  c; // prefer shorter first
   return  -Compare(a.text         , b.text         , true    ); // use negative so small case will be on top of upper case
}
static Int CompareSuggPath(C Source::Suggestion &a, C Source::Suggestion &b) {return ComparePath(a.text, b.text);}
/******************************************************************************/
// SOURCE SUGGESTION
/******************************************************************************/
Int  Source::Suggestion::order()C {if(_order==OrderUnknown)_order=SuggestionsOrder(text); return _order;}

void Source::Suggestion::set(Int priority, C Str &text, Symbol &symbol             ) {T.priority=priority; T._order=OrderUnknown; T.text=text      ; T.display=text; T.symbol=&symbol; T.elm_id.zero(); T.is_macro=false; T.macro_params=                    0    ; if(symbol.type==Symbol::FUNC || symbol.type==Symbol::FUNC_LIST)T.display+=ELLIPSIS; T.icon=null;}
void Source::Suggestion::set(Int priority, C Macro &macro                          ) {T.priority=priority; T._order=OrderUnknown; T.text=macro.name; T.display=text; T.symbol= null  ; T.elm_id.zero(); T.is_macro=true ; T.macro_params=Mid(macro.params+1, 0, 2); macro_def=macro.def; T.icon=null;}
void Source::Suggestion::set(Int priority, C Str &text, C UID &id, C ImagePtr &icon) {T.priority=priority; T._order=OrderUnknown; T.text=text      ; T.display=text; T.symbol= null  ; T.elm_id=id    ; T.is_macro=false; T.macro_params=0; T.icon=icon;}
/******************************************************************************/
// SOURCE
/******************************************************************************/
void Source::clearSuggestions()
{
   cur_text            .clear();
   suggestions         .clear();
   suggestions_list    .clear().columnVisible(0, false);
   suggestions_region  .hide ();
   suggestions_textline.set(S, QUIET).hide(); // clear the textline so next time it is opened it will be empty
}
/******************************************************************************/
void Source::refreshSuggestions()
{
   Memc<CodeEditorInterface::ElmLink> elms; CE.cei().getProjPublishElms(elms);
   C Str &t=Replace(suggestions_textline(), '/', '\\'); // use back-slash everywhere because it's used for elm full names
   suggestions     .clear();
   suggestions_list.clear().columnVisible(0, true);
   if(t.is()) // if we have some text typed
   {
      FREPA(elms)
      {
         CodeEditorInterface::ElmLink &elm=elms[i];
         if(Int p=SuggestionsPriority(elm.full_name, t, false))suggestions.New().set(p, elm.full_name, elm.id, elm.icon);
      }
      suggestions.sort(ComparePriority); // sort by priority
      if(suggestions.elms()>64)suggestions.setNum(64); // limit to max 64 suggestions
   }else
   {
      FREPA(elms)suggestions.New().set(0, elms[i].full_name, elms[i].id, elms[i].icon);
      suggestions.sort(CompareSuggPath); // sort by path
   }
   suggestions_list.setData(suggestions);
   suggestions_list.cur=(suggestions.elms() ? 0 : -1);
}
void Source::listSuggestions(Int force)
{
   if(Const)return;
   clearSuggestions();

   suggestions_pos=cur;
   if(force==-2) // project elements
   {
      refreshSuggestions();
      suggestions_textline.activate();
      suggestions_region.show();
      suggestionsSetRect();
      return;
   }

   if(!active)return;
   if(InRange(cur.y, lines))
   {
      Line &l=lines[cur.y];
      if(l.tokens_preproc_condition_unavailable)return; // if inside inactive block then disable

      if(l.Type(cur.x-1)==TOKEN_CODE || l.Type(cur.x-1)==TOKEN_KEYWORD || l.Type(cur.x-1)==TOKEN_PREPROC)
      {
              suggestions_pos.x=l.wordStart(cur.x-1);
         if(l[suggestions_pos.x-1]=='#')suggestions_pos.x--;
         for(Int i=suggestions_pos.x; i<cur.x; i++)cur_text+=l[i];
      }else
      if(l.Type(cur.x-1)==TOKEN_COMMENT)return;else // don't list suggestions if in comment
      {
         Int i; if(Token *sep=findPrevToken(cur, i))
         {
            if(*sep=='.'
            || *sep=="->"
            || *sep=="::")
            {
               suggestions_pos=cur;
               if((force>=0) ? true : (sep->lineIndex()==cur.y && sep->col+sep->length()==cur.x))force=1;
            }
         }
      }
   }

   if(cur_text.is() || force>0)
   {
      parseCurFunc();

      // detect token at suggestions_pos
      Int t; Token *token=findToken(suggestions_pos, t); Symbol *caller=(token ? token->parent : null);
      if( token && token->def_decl && force<=0)return; // if token is definition/declaration then skip suggestions
      if(!token)
         if(token=findPrevToken(suggestions_pos, t))
            if(!caller)
               if(caller=token->parent)
                  if(*token=='}')
                     caller=caller->Parent(); // prev token needs to use caller->parent if '}' is used

      // if we're in a preprocess line, and after first token, then disable suggestions
      if(InRange(suggestions_pos.y, lines))
      {
         Line &l=lines[suggestions_pos.y];
         if(l.preproc && l.tokens.elms() && suggestions_pos.x>=l.tokens[0].col+l.tokens[0].length()
         || l.starts_with_preproc)return;
      }

      Bool precise_parent=false, ctor_init=false, allow_self=true;
      Expr parent; parent.symbol=caller; // set initial parent as the 'caller'
      if(token)
      {
         if(token->ctor_initializer)
         {
            precise_parent=true;
            ctor_init     =true;
         }else
         {
                     if(!(*token=='.' || *token=="->" || *token=="::"))token=(InRange(t-1, tokens) ? tokens[--t] : null);
            if(token)if(  *token=='.' || *token=="->" || *token=="::" ) // 't' now points to the separator
            {
               precise_parent=true;
               parent.symbol.clear(true); evaluateSymbol(t, parent); // calculate correct parent, we DO need to calculate from 'i' (and include the '.' separator) because expressions like "(TYPE*)obj." without the '.' separator would look like "(TYPE*)obj" and always return obj already casted to some type and be a pointer, which we don't want, we want the original "obj." first
               if(parent=="super" || parent=="__super")
               {
                  allow_self=false;
                  if(parent.parent.elms())
                  {
                     Expr temp;
                     Swap(temp, parent.parent[0]);
                     Swap(temp, parent);
                  }else
                  {
                     parent.symbol=(caller ? caller->Class() : null);
                  }
               }
            }
         }
      }
      
      Bool parent_instance=parent.anyInstance();
      if(cur_text[0]!='#' && !precise_parent){Memc<Macro> &macros=ProjectMacros/*macrosForLine(suggestions_pos.y)*/; REPA(macros){Macro &m=macros[i]; if(m.use_for_suggestions)if(Int p=(cur_text.is() ? SuggestionsPriority(m.name, cur_text, m.all_up_case) : 1))suggestions.New().set(p, m);}} // list macros
      if(cur_text[0]=='#' && !precise_parent){REPA(Symbols){Symbol &s=Symbols.lockedData(i); if(!(s.modifiers&Symbol::MODIF_SKIP_SUGGESTIONS) && (s.valid || s.valid_decl) && s.type==Symbol::PREPROC && ((precise_parent && parent.symbol) ? s.isMemberOf(parent.symbol(), parent.symbol.templates, caller, parent_instance, ctor_init, allow_self, false) : s.canBeAccessedFrom(parent.symbol(), caller, precise_parent, ProjectUsings)))if(Int p=SuggestionsPriority(s, cur_text, FlagTest(s.modifiers, Symbol::MODIF_ALL_UP_CASE)))suggestions.New().set(p, s, s);}}else // list preproc  keywords
      if(cur_text.is()                      ){REPA(Symbols){Symbol &s=Symbols.lockedData(i); if(!(s.modifiers&Symbol::MODIF_SKIP_SUGGESTIONS) && (s.valid || s.valid_decl) && s.type!=Symbol::PREPROC && ((precise_parent && parent.symbol) ? s.isMemberOf(parent.symbol(), parent.symbol.templates, caller, parent_instance, ctor_init, allow_self, false) : s.canBeAccessedFrom(parent.symbol(), caller, precise_parent, ProjectUsings)))if(Int p=SuggestionsPriority(s, cur_text, FlagTest(s.modifiers, Symbol::MODIF_ALL_UP_CASE)))suggestions.New().set(p, s, s);}}else // list matching symbols
      if(force                              ){REPA(Symbols){Symbol &s=Symbols.lockedData(i); if(!(s.modifiers&Symbol::MODIF_SKIP_SUGGESTIONS) && (s.valid || s.valid_decl) && s.type!=Symbol::PREPROC && ((precise_parent && parent.symbol) ? s.isMemberOf(parent.symbol(), parent.symbol.templates, caller, parent_instance, ctor_init, allow_self, false) : s.canBeAccessedFrom(parent.symbol(), caller, precise_parent, ProjectUsings)))                                                                                            suggestions.New().set(1, s, s);}}     // list all      symbols

      if(suggestions.elms())
      {
         suggestions.sort(CompareAlphabeticalCS); // first sort alphabetically, to remove suggestions with the same base names (but from different symbols)
         REPA(suggestions)if(i)
         {
            Suggestion &a=suggestions[i], &b=suggestions[i-1];
            if(Equal(a.text, b.text, true)) // if have the same names, use only suggestion which symbol is close to current level
            {
               Int a_level, b_level;
               if(a.symbol && b.symbol) // it's possible we're accessing overloaded members from base and extended classes "class A {void met();}   class B {void met() {met|}}" both 'met' symbols can be accessed, however only one is correct
               {
                  Symbol *a_class=a.symbol->Class(), // get classes of both members
                         *b_class=b.symbol->Class();
                  if(a_class && b_class) // if both detected
                  {
                     if(a_class==b_class         ){a_level=a.symbol->level; b_level=b.symbol->level;}else // if this is the same class then it's possible we're accessing local variable "class A {int x; void met() {int x;}}"
                     if(b_class->hasBase(a_class)){a_level=0; b_level=1;}else // "class A {}   class B : A {}" remove A and keep B
                     if(a_class->hasBase(b_class)){a_level=1; b_level=0;}else // "class B {}   class A : B {}" remove B and keep A
                                                  {a_level=0; b_level=0;}
                  }else // maybe one does not belong to class "namespace N {int x;}   class X {int x; void method() {x|}}" in such case most probably we're accessing class member, since this suggestion was returned it means we have permission to do so (most likely we're in the class)
                  {
                     a_level=(a_class!=null);
                     b_level=(b_class!=null);
                  }
               }else // macro
               {
                  a_level=(a.symbol==null);
                  b_level=(b.symbol==null);
               }
               suggestions.remove((a_level < b_level) ? i : i-1, true); // remove the suggestion which is not deeper (has smaller level) - this is needed if we're referencing 2 symbols from different levels "{int x; {int x; | }}" both symbols can be accessed, however only one is correct
            }
         }
         if(cur_text.is()) // if we have some text typed, then sort again, this time by priority
         {
            suggestions.sort(ComparePriority);
            if(suggestions.elms()>64)suggestions.setNum(64); // limit to max 64 suggestions
         }else
         {
            suggestions.sort(CompareAlphabetical);
         }
         suggestions_list  .setData(suggestions);
         suggestions_list  .cur=(suggestions.elms() ? 0 : -1);
         suggestions_region.show();
         suggestionsSetRect();
      }
   }
}
/******************************************************************************/
void Source::suggestionsSetRect()
{
   if(suggestions_region.visible())
   {
      Flt w=suggestions_region.slidebarSize(); REP(suggestions_list.columns())if(suggestions_list.columnVisible(i))w+=suggestions_list.columnWidth(i);
      suggestions_textline.rect(Rect_L (posVisual(suggestions_pos)+Vec2(CE.lineNumberSize(), -0.39f*CE.ts.lineHeight()), w, CE.ts.lineHeight()*1.4f));
      suggestions_region  .rect(Rect_LU(suggestions_textline.visible() ? suggestions_textline.rect().ld() : posVisual(suggestions_pos+VecI2(0, 1))+Vec2(CE.lineNumberSize()-0.01f, 0), w, D.h()*0.5f));

      Flt d=suggestions_region.rect().h()+(suggestions_textline.visible() ? suggestions_textline.rect().h() : CE.ts.lineHeight());
      if(suggestions_region.rect().min.y  +slidebar[1].offset() < -rect().h()+slidebarSize()  // if suggestions rect is below the screen
      && suggestions_region.rect().max.y+d+slidebar[1].offset() < 0                         ) // moved rect will not be above the top of the source rect
         suggestions_region.move(Vec2(0, d));
   }
}
/******************************************************************************/
void Source::setSuggestion(Int x)
{
   if(Gui.kb()!=&suggestions_list)
   {
      Clamp(x, 0, suggestions_list.elms()-1);
      suggestions_list.cur=x;
      suggestions_list.scrollTo(x, true);
   }
}
/******************************************************************************/
void Source::autoComplete(Bool auto_space, Bool set_undo, Bool auto_brace, Bool call_changed, Bool ignore_params)
{
   if(set_undo)setUndo();
   if(sel.y>=0)
   {
      delSel(false, false);
      suggestions_pos=cur;
   }
   if(Suggestion *sugg=suggestions_list())
   {
      SuggestionsUsed(sugg->text);
      if(!Const)
      {
         Str text=sugg->text; if(sugg->elm_id.valid())text=sugg->elm_id.asCString();
         exist(suggestions_pos.x, suggestions_pos.y);
         Line &line=lines[suggestions_pos.y];
         if(Equal(text, "else", true) && line[suggestions_pos.x-1]==' ' && line[suggestions_pos.x-2]==';')suggestions_pos.x--; // if we've typed "; else" then remove the space before 'else'
         line.remove(suggestions_pos.x, cur.x-suggestions_pos.x).insert(suggestions_pos.x, text);
         cur.x=suggestions_pos.x+text.length();
         cur.y=suggestions_pos.y;
         Bool changed_called=false;
         if(auto_space)
         {
            auto_space=false;
            if(sugg->symbol && (sugg->symbol->modifiers&Symbol::MODIF_FOLLOW_BY_SPACE))auto_space=true;else
            {
               changed(cur.y); changed_called=true; // needed for 'isDeclaration'
               Int i; if(Token *token=findPrevToken(cur, i))if(token->symbol && (token->symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !token->symbol->templates.elms() && isDeclaration(token->symbol(), i))auto_space=true; // don't insert space if we're using class with templates (like "Memc<>")
            }
            if(auto_space)
            {
               if(Equal(text, "else", true) && line.Type(cur.x)==TOKEN_NONE)cur.x++;else // if we've typed 'else' which is followed by (space or nothing), then just increase cursor position without inserting space, this is needed in case we're typing else at the end of line
               if(auto_space)
               {
                  line.insert(cur.x++, ' '); changed_called=false;
               }
            }
         }
         if(auto_brace && (Equal(text, "if", true) || Equal(text, "for", true) || Equal(text, "while", true) || Equal(text, "switch", true) || sugg->symbol && (sugg->symbol->type==Symbol::FUNC || sugg->symbol->type==Symbol::FUNC_LIST) || sugg->is_macro && sugg->macro_params))
         {
            if(line[cur.x]!='(')
            {
               line.insert(cur.x++, "()", TOKEN_OPERATOR); changed_called=false; // set cursor inside the brackets
               // if the function does not have parameters, then set the cursor after brackets
               if(sugg->is_macro)
               {
                  if(sugg->macro_params==1)cur.x++;
               }else
               if(sugg->symbol)
               {
                  if(sugg->symbol->type==Symbol::FUNC && (ignore_params || sugg->symbol->params.elms()==0))cur.x++;else
                  if(sugg->symbol->type==Symbol::FUNC_LIST)
                  {
                     Bool all_no_param=true; if(!ignore_params)REPA(sugg->symbol->funcs)if(sugg->symbol->funcs[i]->params.elms()){all_no_param=false; break;}
                     if(  all_no_param)cur.x++;
                  }
               }
            }
         }
         if(call_changed && !changed_called)changed(cur.y);
      }
      makeCurVisible();
      clearSuggestions();
   }
}
/******************************************************************************/
Bool Source::evaluateSymbol(Int start, Expr &out, Int final, Bool allow_func_lists)
{
   if(InRange(start, tokens))
   {
    /*if(parse_templates)
      {
         Int from=start+1;
         for(Int level=0, i=start; i>=0; i--)
         {
            Token &token=*tokens[i];
            switch(token[0])
            {
               case '(': case '[':           level++; break;
               case ')': case ']':           level--; break;
               case '{': case '}': case ';': goto found_start;
            }
            if(level>0)break;
            from=i;
         }
      found_start:;
         for(; from<=start; ){Int s=from; ParseTemplates(tokens, from, temp, expr_parent); MAX(from, s+1);}
      }*/
      
      // detect starting position required to obtain desired symbol "return (a+b).x|" -> "return |(a+b).x|"
      Symbol *start_parent=tokens[start]->parent;
      Int     from =start+1;
      for(Int level=0, i=start; i>=0; i--)
      {
         Token &token=*tokens[i];
         switch(token[0])
         {
            case '(': case '[':           level++; break;
            case ')': case ']':           level--; break;
            case '{': case '}': case ';': goto found;
            default : if(token==TMPL_B)level++;else if(token==TMPL_E)level--; break;
         }
         if(!(!start_parent || start_parent->contains(token.parent)))break; // break if the token.parent is not a child of start_parent (basically break if we've reached some parent of the start_parent)
         if(level>0)break;
         if(level<0)from=i;else
         {
            if(token.type==TOKEN_KEYWORD && (token=="return" || token=="do" || token=="else"))break; // return (a+b).x; - break on encountering command keyword

            if(token.type==TOKEN_KEYWORD || token.type==TOKEN_CODE
            ||(token.type==TOKEN_OPERATOR && (token=='(' || token==')' || token=='[' || token==']' || token==TMPL_B || token==TMPL_E || token=='.' || token=="->" || token=="::"))
            || final>=0) // for 'final' we need full formula
               from=i;
            else break;
         }
      }
   found:;
      Memc<Message> msgs;
      Compiler      compiler(msgs, tokens, this, null);
      return        compiler.relax().setFinal(final, allow_func_lists).compileTokens(from, start, out)!=COMPILE_FAILED;
   }
   return false;
}
/******************************************************************************/
Symbol* Source::finalSymbol(Int final, Bool allow_func_lists)
{
   if(InRange(final, tokens))
   {
      Int start=final;
      if(!allow_func_lists) // move cursor from "func|()" to "func()|" to include parameters for 'func' in order to detect correct polymorphic function
      {
         Token &start_token=*tokens[start];
         for(Int level=(start_token=='(' || start_token=='[' || start_token==TMPL_B); start+1<tokens.elms(); )
         {
            Token &token=*tokens[start+1];
            if(token=='(' || token=='[' || token==TMPL_B)level++;else
            if(token==')' || token==']' || token==TMPL_E)level--;else
            if(token=='{' || token=='}' || token==';')break;
            if((token==',' || token==':') && !level)break; // skip ':' for  ctor initializers, and for "x ? y : z", but not for "func(x ? y : z)" in case we're calculating 'func'

            if(level<0)break;
            start++;
         }
      }
      Expr expr;
      if(evaluateSymbol(start, expr, final, allow_func_lists))return expr.symbol();
   }
   return null;
}
/******************************************************************************/
}}
/******************************************************************************/
