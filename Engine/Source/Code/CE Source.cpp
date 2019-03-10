/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
void LineMode::setTokens(Line &line)
{
   C Str &s=line;
   tokens.clear();
   FREPA(s)
   {
      TOKEN_TYPE t=Type(i);
      if(TokenType(t))
      {
         Token &token=tokens.New(); token.setBorrowed(s()+i, 1, i, line, t);
         if(t==TOKEN_OPERATOR)
         {
            Char c=s[i+1];
            switch(s[i])
            {
               case '<': if(c=='='){token.extend(); i++;}else if(c=='<' && s[i+2]=='='){token.extend(2); i+=2;                                                         }  break; // <= <<= (<< is not processed here due to possible template problems, processed later)
               case '>': if(c=='='){token.extend(); i++;}else if(c=='>' && s[i+2]=='='){token.extend(2); i+=2;                                                         }  break; // >= >>= (>> is not processed here due to possible template problems, processed later)
               case '!': if(c=='='){token.extend(); i++;}else if(c=='!' && s[i+2]=='='){token.extend(2); i+=2;                                                         }  break; // == !!=
               case '=': if(c=='='){token.extend(); i++; if(s[i+1]=='='){token.extend(); i++;                                                                          }} break; // == ===
               case '+': if(c=='='){token.extend(); i++;}else if(c=='+'){token.extend(); i++;}                                                                            break; // += ++
               case '-': if(c=='='){token.extend(); i++;}else if(c=='-'){token.extend(); i++;}else if(c=='>'){token.extend(); i++; if(s[i+1]=='*'){token.extend(); i++;}} break; // -= -- -> ->*
               case '*': if(c=='='){token.extend(); i++;}                                                                                                                 break; // *=
               case '/': if(c=='='){token.extend(); i++;}                                                                                                                 break; // /=
               case '%': if(c=='='){token.extend(); i++;}                                                                                                                 break; // %=
               case '&': if(c=='='){token.extend(); i++;}else if(c=='&'){token.extend(); i++;}                                                                            break; // &= &&
               case '|': if(c=='='){token.extend(); i++;}else if(c=='|'){token.extend(); i++;}                                                                            break; // |= ||
               case '^': if(c=='='){token.extend(); i++;}else if(c=='^'){token.extend(); i++;}                                                                            break; // ^= ^^
               case ':': if(c==':'){token.extend(); i++;}                                                                                                                 break; // ::
               case '.': if(c=='*'){token.extend(); i++;}else if(c=='.' && s[i+2]=='.'){token.extend(2); i+=2;}                                                           break; // .* ... (can occur in "catch(...)")
               case '#': if(c=='#'){token.extend(); i++;}                                                                                                                 break; // ##
            }
         }else
         {
            for(; Type(i+1)==t; i++)token.extend();

            if(t==TOKEN_CODE)
            {
               BStr &bs=token;

               // #force_preproc, normal keyword first then preproc keyword
               // 2nd token in preproc is always preproc
               Keyword *keyword;
               if(preproc && !starts_with_preproc && tokens.elms()==2)goto preproc_only; // if we're processing the "#TOKEN"

               if(keyword=BinaryFind(NativeKeywords, NativeKeywordsElms, bs, CompareCS))
               {
                  if(keyword->cpp
                  ?  true              // cpp keywords are supported everywhere
                  : !line.source->cpp) // .es keywords are supported only in .es files
                     t=TOKEN_KEYWORD;
               }else
               if(preproc)
               {
               preproc_only:;
                  if(BinaryHas(PreprocKeywords, PreprocKeywordsElms, bs, CompareCS))t=TOKEN_PREPROC;
               }
               if(t!=token.type)
               {
                  token.type=t; REP(token.length())type[token.col+i]=t;
               }
            }
         }
      }
   }
}
/******************************************************************************/
static Bool _HasUnicode(CChar *t, Char stop)
{
   for(;;)
   {
      Char c=*t++;
      if(c=='\\') // check for escape characters
      {
         c=*t++; if(!c)break;
         if(c=='x' || c=='u' || c=='U')return true; // Warning: for simplicity we assume that the next char is a wide char
         continue;
      }
      if(!c || c==stop)break;
      if(HasUnicode(c))return true;
   }
   return false;
}
void LineMode::resetType(Line &line) // this just resets the 'type' without changing the tokens
{
 C Str &s=line;
   type.clear();

   TOKEN_TYPE t=(starts_with_comment ? TOKEN_COMMENT : TOKEN_NONE);
   Bool       ignore_rest=false, ignore_char=false, real=false;
   FREPA(s)
   {
      TOKEN_TYPE p=t;
      if(ignore_char)
      {
         ignore_char=false;
         type.add(t);
      }else
      {
         Char c;
         if(!ignore_rest)
         {
            c=s[i];
            if(TextType(t)      && c=='\\'     )ignore_char=true;else
            if(t!=TOKEN_COMMENT && !TextType(t))
            {
               if(c=='/' && s[i+1]=='*'      ){t=TOKEN_COMMENT; type.add(t); i++;}else // process 2 chars at a time
               if(c=='/' && s[i+1]=='/'      ){t=TOKEN_COMMENT; ignore_rest=true;}else
               if(c==' '                     ) t=TOKEN_NONE   ;else
               if(c=='"'                     ){Char p=s[i-1]; if(p=='L' || p=='u' || p=='U')type.last()=t=TOKEN_TEXT16;else t=(line.source->cpp ? TOKEN_TEXT8 : _HasUnicode(s()+i+1, c) ? TOKEN_TEXT16 : TOKEN_TEXT8);}else
               if(c=='\''                    ){Char p=s[i-1]; if(p=='L' || p=='u' || p=='U')type.last()=t=TOKEN_CHAR16;else t=(line.source->cpp ? TOKEN_CHAR8 : _HasUnicode(s()+i+1, c) ? TOKEN_CHAR16 : TOKEN_CHAR8);}else
               if(CodeCharType(c)==CHART_SIGN)
               {
                  if(c=='.' && (t==TOKEN_NUMBER || CharFlag(s[i+1])&CHARF_DIG)){t=TOKEN_NUMBER; real=true;}else // was already a number or the next char is a digit
                  if((c=='-' || c=='+') && t==TOKEN_NUMBER && real && s[i-1]=='e')t=TOKEN_NUMBER;else // 1.2e-3, 1.2e+3
                  t=((c=='#' && s[i+1]!='#' && s[i-1]!='#') ? TOKEN_PREPROC : TOKEN_OPERATOR);
               }else
               if(t!=TOKEN_CODE && t!=TOKEN_KEYWORD && CharFlag(c)&CHARF_DIG)t=TOKEN_NUMBER;else
               if(t!=TOKEN_CODE && t!=TOKEN_KEYWORD && t!=TOKEN_NUMBER && CodeCharType(s[i-1])!=CHART_CHAR)t=TOKEN_CODE;
            }
         }

         type.add(t);
         if(t==TOKEN_PREPROC)preproc=true;
         if(t!=TOKEN_NUMBER )real   =false;

         if(!ignore_rest && !ignore_char)
         {
            if( t==TOKEN_COMMENT                   && c=='*' && s[i+1]=='/'){type.add(t); i++; t=TOKEN_NONE;}else // process 2 chars at a time
            if((p==TOKEN_TEXT8 || p==TOKEN_TEXT16) && c=='"' )t=TOKEN_NONE;else
            if((p==TOKEN_CHAR8 || p==TOKEN_CHAR16) && c=='\'')t=TOKEN_NONE;else
            if( t==TOKEN_OPERATOR)t=TOKEN_NONE;
         }
      }
   }
   if(t!=TOKEN_COMMENT || ignore_rest)t=TOKEN_NONE;

   ends_with_comment=(t==TOKEN_COMMENT);
   ends_with_preproc=(preproc && s.last()=='\\');
}
void LineMode::setType(Line &line, Bool starts_with_comment, Bool starts_with_preproc)
{
             T.starts_with_comment=starts_with_comment;
   T.preproc=T.starts_with_preproc=starts_with_preproc;
   resetType(line);
   setTokens(line);
}
/******************************************************************************/
enum
{
   SWC=1<<0,
   EWC=1<<1,
   PRP=1<<2,
   SWP=1<<3,
   EWP=1<<4,
   SWM=1<<5,
};
Bool LineMode::save(File &f, StrLibrary &sl, C Str &text)C
{
   f.putByte((starts_with_comment ? SWC : 0) | (ends_with_comment ? EWC : 0) | (preproc ? PRP : 0) | (starts_with_preproc ? SWP : 0) | (ends_with_preproc ? EWP : 0) | (starts_with_macro_param ? SWM : 0));
   if(type.saveRaw(f))
   {
      f.cmpUIntV(tokens.elms()); FREPA(tokens)if(!tokens[i].save(f, sl, text))return false;
      return f.ok();
   }
   return false;
}
Bool LineMode::load(File &f, StrLibrary &sl, C Str &text, Line &line, Str &temp)
{
   Byte flag; f>>flag; starts_with_comment=FlagTest(flag, SWC); ends_with_comment=FlagTest(flag, EWC); preproc=FlagTest(flag, PRP); starts_with_preproc=FlagTest(flag, SWP); ends_with_preproc=FlagTest(flag, EWP); starts_with_macro_param=FlagTest(flag, SWM);
   if(type.loadRaw(f))
   {
      tokens.setNum(f.decUIntV()); FREPA(tokens)if(!tokens[i].load(f, sl, text, line, temp))goto error;
      if(f.ok())return true;
   }
error:
   /*del();*/ return false;
}
/******************************************************************************/
Line::~Line()
{
   if(preproc)preprocChanged();
}
void Line::preprocChanged()
{
   if(source)
      if(source->preproc_line_changed<0 || line<source->preproc_line_changed)
         source->preproc_line_changed=line;
}
void Line::resetType()
{
   LineMode::resetType(T);
}
void Line::setType(Bool starts_with_comment, Bool starts_with_preproc)
{
   if(changed)
   {
      Bool was_preproc=preproc;

          LineMode::setType(T,  starts_with_comment, starts_with_preproc);
      comment_mode. setType(T, !starts_with_comment, starts_with_preproc);
      changed=false;

      if(was_preproc || preproc)preprocChanged();
   }else
   {
      if(starts_with_comment!=T.starts_with_comment)
      {
         LineMode &cur=T;
         Swap(cur, comment_mode);
      }
   }
}
/******************************************************************************/
static Bool LineFileName(C Str &text, Int x, VecI2 &range) // detect if position is inside 24 char ID text "........."
{
   if(text[x]!='"')
   {
      Int a=x-1; for(; a>=0           ; a--)if(text[a]=='"')break; a++;
      Int b=x+1; for(; b<text.length(); b++)if(text[b]=='"')break;
      if(b-a==24)
      {
         Str s; for(Int i=a; i<b; i++)s+=text[i]; UID id; if(DecodeFileName(s, id)){range.set(a, b); return true;}
      }
   }
   return false;
}
static Bool Individual(Char c)
{
   switch(c)
   {
      case '{': case '(': case '[': case ';':
      case '}': case ')': case ']': case ',': return true;
      default : return false;
   }
}
Int Line::wordStart(Int x)
{
   Clamp(x, 0, length()-1); CHAR_TYPE type=CodeCharType(T[x]);
   for(; x>0 && CodeCharType(T[x-1])==type && !Individual(T[x-1]); x--);
   VecI2 range; if(CE.view_elm_names){UID id; if(TextToIDInside(T, x, id, range))return range.x;} if(LineFileName(T, x, range))return range.x;
   return x;
}
Int Line::wordEnd(Int x)
{
   Clamp(x, 0, length()); CHAR_TYPE type=CodeCharType(T[x]); if(x<length())x++;
   for(; x<length() && CodeCharType(T[x])==type && !Individual(T[x]); x++);
   VecI2 range; if(CE.view_elm_names){UID id; if(TextToIDInside(T, x, id, range))return range.y+1;} if(LineFileName(T, x, range))return range.y;
   return x;
}
Int Line::wordBegin(Int x) // skips all spaces until first word encountered
{
   Clamp(x, 0, length()-1); for(; x>0 && T[x]==' '; x--); return wordStart(x);
}
/******************************************************************************/
Bool Line::saveData(File &f)C
{
   f<<id;
   f.putStr(T);
   return f.ok();
}
Bool Line::loadData(File &f)
{
   f>>id;
   f.getStr(T);
   if(f.ok())return true;
   /*del();*/ return false;
}

enum
{
   TOKENS_PREPROC_USE                  =1<<0,
   TOKENS_PREPROC_CONDITION_UNAVAILABLE=1<<1,
};

Bool Line::save(File &f, StrLibrary &sl)C
{
   f.putStr(T).putByte((tokens_preproc_use ? TOKENS_PREPROC_USE : 0)|(tokens_preproc_condition_unavailable ? TOKENS_PREPROC_CONDITION_UNAVAILABLE : 0));
   if(LineMode   ::save(f, sl, T))
 //if(comment_mode.save(f, sl, T))
   {
      if(tokens_preproc_use){f.cmpUIntV(tokens_preproc.elms()); FREPA(tokens_preproc)if(!tokens_preproc[i].save(f, sl, T))return false;}
      return f.ok();
   }
   return false;
}
Bool Line::load(File &f, StrLibrary &sl, Int line, Source &source, Str &temp)
{
   tokens_preproc.clear();

   T.line  = line  ;
   T.source=&source;
   Byte flag;
   f.getStr(T)>>flag;
   tokens_preproc_use                  =FlagTest(flag, TOKENS_PREPROC_USE);
   tokens_preproc_condition_unavailable=FlagTest(flag, TOKENS_PREPROC_CONDITION_UNAVAILABLE);
   if(LineMode   ::load(f, sl, T, T, temp))
 //if(comment_mode.load(f, sl, T, T, temp))
   {
      if(tokens_preproc_use){tokens_preproc.setNum(f.decUIntV()); FREPA(tokens_preproc)if(!tokens_preproc[i].load(f, sl, T, T, temp))goto error;}
      if(f.ok())return true;
   }
error:
   /*del();*/ return false;
}
/******************************************************************************/
Str Line::textTokens()C
{
   Str text; TOKEN_TYPE last_type=TOKEN_NONE; C Memc<Token> &tokens=T.Tokens();
   FREPA(tokens)
   {
    C Token &token=tokens[i];
      if(MustSeparate(last_type, token.type))text+=' ';
      text+=token;
      last_type=token.type;
   }
   return text;
}
Str Line::textCode() // must be in sync with "Source::ViewLine::textCode"
{
   Str code;
   if(tokens_preproc_condition_unavailable)
   {
      code ="[color=";
      code+=Theme.colors[TOKEN_PREPROC_DISABLED].asHex();
      code+="][nocode]";
      code+=T;
   }else
   {
      // parse functions to detect symbols and their colorization
      {
         Memc<Token> &tokens=Tokens(); FREPA(tokens)if(Symbol *symbol=tokens[i].parent)if(Symbol *func=symbol->func())if(func->source)func->source->parseFunc(*func);
      }

      TOKEN_TYPE last_type =TOKEN_NONE;
      Color      last_color=TRANSPARENT;
      Token     *token_org_cur, *token_org_end, *token_prc_cur, *token_prc_end;
      if(tokens  .elms()){token_org_cur=tokens  .data(); token_org_end=token_org_cur+tokens  .elms();}else token_org_cur=null; // set null when there are no elements
      if(Tokens().elms()){token_prc_cur=Tokens().data(); token_prc_end=token_prc_cur+Tokens().elms();}else token_prc_cur=null; // set null when there are no elements

      FREPA(T)
      {
         TOKEN_TYPE type=Type(i);
         if(last_type!=type)
         {
            last_type=type;

            // check if data type or namespace
            if(token_prc_cur)
            {
               for(; token_prc_cur->col<i; )
               {
                  token_prc_cur++;
                  if(token_prc_cur==token_prc_end){token_prc_cur=null; break;}
               }
               if(token_prc_cur && token_prc_cur->col==i)
                  if(token_prc_cur->macro)type=TOKEN_KEYWORD;else
                  if(Symbol *symbol=token_prc_cur->symbol())
               {
                  if(symbol->type==Symbol::ENUM                                                                   )type=TOKEN_ENUM_TYPE;else
                  if(symbol->type==Symbol::ENUM_ELM                                                               )type=TOKEN_ENUM_ELM ;else
                  if((symbol->type==Symbol::FUNC_LIST || symbol->type==Symbol::FUNC) && symbol->isGlobalOrStatic())type=TOKEN_FUNC     ;else
                  if((symbol->modifiers&Symbol::MODIF_DATA_TYPE) || symbol->type==Symbol::NAMESPACE               )type=TOKEN_KEYWORD;
               }
            }

            // check if macro
            if(token_org_cur)
            {
               for(; token_org_cur->col<i; )
               {
                  token_org_cur++;
                  if(token_org_cur==token_org_end){token_org_cur=null; break;}
               }
               if(token_org_cur && token_org_cur->col==i && token_org_cur->macro)type=TOKEN_MACRO;
            }

            if(ValidType(type))
            {
               Color c=Theme.colors[type];
               if(last_color!=c)
               {
                  last_color=c;
                  if(code.is())code+="[/nocode][/color]";
                  code+="[color=";
                  code+=c.asHex();
                  code+="][nocode]";
               }
            }
         }
         code+=T[i];
      }
   }
   return code;
}
Str Source::ViewLine::textCode() // must be in sync with "Line::textCode"
{
   Str code;
   if(Line *line=((source && InRange(T.line(), source->lines)) ? &source->lines[T.line()] : null))
      if(line->tokens_preproc_condition_unavailable)
      {
         code ="[color=";
         code+=Theme.colors[TOKEN_PREPROC_DISABLED].asHex();
         code+="][nocode]";
         code+=asStr();
      }else
      {
         // parse functions to detect symbols and their colorization
         {
            Memc<Token> &tokens=line->Tokens(); FREPA(tokens)if(Symbol *symbol=tokens[i].parent)if(Symbol *func=symbol->func())if(func->source)func->source->parseFunc(*func);
         }

         TOKEN_TYPE last_type =TOKEN_NONE;
         Color      last_color=TRANSPARENT;
         Token     *token_org_cur, *token_org_end, *token_prc_cur, *token_prc_end;
         if(line->tokens  .elms()){token_org_cur=line->tokens  .data(); token_org_end=token_org_cur+line->tokens  .elms();}else token_org_cur=null; // set null when there are no elements
         if(line->Tokens().elms()){token_prc_cur=line->Tokens().data(); token_prc_end=token_prc_cur+line->Tokens().elms();}else token_prc_cur=null; // set null when there are no elements

         FREPA(T)
         {
            TOKEN_TYPE type=CodeLine::type(i);
            Int        col =          cols[i].pos.x;
            if(last_type!=type)
            {
               last_type=type;

               // check if data type or namespace
               if(token_prc_cur)
               {
                  for(; token_prc_cur->col<col; )
                  {
                     token_prc_cur++;
                     if(token_prc_cur==token_prc_end){token_prc_cur=null; break;}
                  }
                  if(token_prc_cur && token_prc_cur->col==col)
                     if(token_prc_cur->macro)type=TOKEN_KEYWORD;else
                     if(Symbol *symbol=token_prc_cur->symbol())
                  {
                     if(symbol->type==Symbol::ENUM                                                                   )type=TOKEN_ENUM_TYPE;else
                     if(symbol->type==Symbol::ENUM_ELM                                                               )type=TOKEN_ENUM_ELM ;else
                     if((symbol->type==Symbol::FUNC_LIST || symbol->type==Symbol::FUNC) && symbol->isGlobalOrStatic())type=TOKEN_FUNC     ;else
                     if((symbol->modifiers&Symbol::MODIF_DATA_TYPE) || symbol->type==Symbol::NAMESPACE               )type=TOKEN_KEYWORD;
                  }
               }

               // check if macro
               if(token_org_cur)
               {
                  for(; token_org_cur->col<col; )
                  {
                     token_org_cur++;
                     if(token_org_cur==token_org_end){token_org_cur=null; break;}
                  }
                  if(token_org_cur && token_org_cur->col==col && token_org_cur->macro)type=TOKEN_MACRO;
               }

               if(ValidType(type))
               {
                  Color c=Theme.colors[type];
                  if(last_color!=c)
                  {
                     last_color=c;
                     if(code.is())code+="[/nocode][/color]";
                     code+="[color=";
                     code+=c.asHex();
                     code+="][nocode]";
                  }
               }
            }
            code+=T[i];
         }
      }
   return code;
}
void Source::ViewLine::setRect(Int i)
{
   Flt lh=CE.ts.lineHeight();
   super::rect(Rect_LU(0, -lh*i + CE.fontSpaceOffset(), CE.ts.textWidth(asStr()), lh));
}
/******************************************************************************/
// DETECT
/******************************************************************************/
Bool Source::getSymbolMacroID(C VecI2 &cur, SymbolPtr &symbol_ptr, Macro* &macro_ptr, UID &id, VecI2 *x_range, Bool prefer_definition)
{
    macro_ptr=null;
   symbol_ptr=null;
   id.zero();

   if(InRange(cur.y, lines))
   {
      Line &line=lines[cur.y];

      // check for ID
      VecI2 range; if(CE.view_elm_names)if(TextToIDInside(line, cur.x, id, x_range ? *x_range : range))return true;

      // check for macro
      Int x=line.wordStart(cur.x);
      FREPA(line.tokens)
      {
         Token &token=line.tokens[i];
         if(token.macro && token.col==x)
         {
            REPA(ProjectMacros)
            {
               Macro &macro=ProjectMacros[i];
               if(token==macro.name)
               {
                  macro_ptr=&macro;
                  if(x_range)x_range->set(x, x+macro.name.length()-1);
                  return true;
               }
            }
            break;
         }
      }
   }

   // check for symbol
   Int token; if(Token *t=findToken(cur, token))
   {
      parseFunc(cur); // parse func to detect templates and vars
      if(Symbol *symbol=finalSymbol(token))
      {
         if(symbol->isFunc())
         {
            Memt<SymbolPtr> funcs; // definition + declaration
            SymbolPtr       func_list; if(func_list.find(GetPath(symbol->full_name)))REPA(func_list->funcs)if(symbol->sameFunc(*func_list->funcs[i]))funcs.add(func_list->funcs[i]); // gather same funcs into container
            Symbol         *preference=null; REPA(funcs)if(FlagTest(funcs[i]->modifiers, Symbol::MODIF_FUNC_BODY)==prefer_definition){preference=funcs[i](); break;} // find preference
            if(preference && preference->source==this && InRange(preference->token_index, tokens) && tokens[preference->token_index]->lineIndex()==cur.y)preference=null; // if we're already at the preference and trying to jump to it again, then cancel jumping to preference
            if(preference)symbol=preference;else REPA(funcs)if(funcs[i]==symbol){symbol=funcs[(i+1)%funcs.elms()](); break;} // if preference is available then jump to it, if not then select next function in the list
         }
         symbol_ptr=symbol;
         if(x_range)x_range->set(t->col, t->col+t->length()-1);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
void Source::jumpToCur()
{
   SymbolPtr symbol;
   Macro    *macro=null;
   UID       id;
   if(getSymbolMacroID(cur, symbol, macro, id))
   {
      forceCreateNextUndo(); clearSuggestions();
      if(symbol    )CE.jumpTo(symbol());else
      if(macro     )CE.jumpTo(macro   );else
      if(id.valid())CE.cei().elmOpen(id);
   }
}
/******************************************************************************/
void Source::findAllReferences()
{
   SymbolPtr symbol;
   Macro    *macro=null;
   UID       id;
   if(getSymbolMacroID(cur, symbol, macro, id))
   {
      forceCreateNextUndo(); clearSuggestions();
      if(symbol    )CE.findAllReferences(symbol()   );else
      if(macro     )CE.findAllReferences(macro->name);else
      if(id.valid())CE.findAllReferences(id         );
   }
}
/******************************************************************************/
// PARSE
/******************************************************************************/
void Source::delSymbols()
{
   REPAO(lines).resetTokens();
   symbols.clear();
   decls  .clear();
}
void Source::parseFunc(Symbol &func, Memc<Command> &cmds, Memc<Message> &msgs)
{
   /**
      exceptions of possible var/func declarations are:
      ;TYPE ..
      {TYPE ..
      }TYPE ..
      if(TYPE ..)TYPE ..
      else TYPE ..
      for(TYPE ..; TYPE ..; )TYPE ..
      do TYPE ..
      while(TYPE ..)TYPE ..
      switch(TYPE ..)
      ;label:TYPE
      {label:TYPE
      }label:TYPE
      case X:
   /**/
   if(used() && (func.modifiers&Symbol::MODIF_FUNC_BODY) && !(func.helper&Symbol::HELPER_FUNC_PARSED)) // if function has body and was not yet parsed
   {
      func.helper|=Symbol::HELPER_FUNC_PARSED; // set as parsed
      Int body_start=getBodyStart(func.token_index); // get body start ('{' token)
      if(InRange(body_start, tokens))
      {
         body_start++; // proceed to first token after '{'

         Str                 temp;
         Memc<Symbol::Modif> templates;
         Memc<Int          > new_templates;

         // set ctor initializers
         func.ctor_inits.clear();
         for(Int i=func.token_index; i<body_start && i<tokens.elms(); i++)
         {
            Token &token=*tokens[i];
            if(token.ctor_initializer && token.type==TOKEN_CODE)
               if(token.symbol=FindChild(temp=token, func.Parent(), null, false))
                  func.ctor_inits.add(token.symbol);
         }

         // set sub spaces
         Int body_end=body_start; // use copy because variable passed to ReadCommands is passed as reference
         ReadCommands(T, body_end, func, cmds, msgs);

         // detect vars and sub-spaces
         for(Int i=body_start; i<body_end && i<tokens.elms(); )
         {
            Int    start=i;
            Token &token=*tokens[i];

            if(token=='>')
            {
               ParseFuncTemplate(tokens, i);
            }else
            if(token=="goto")
            {
               if(InRange(i+1, tokens))
               {
                  Token &label=*tokens[++i]; i++;
                  label.symbol.find(func.full_name+SEP+'@'+label);
               }
            }else
            {
               Bool detect=true;
               if(token=='.' || token=="->" || token=="::")
                  if(InRange(i-1, tokens))
               {
                  Token &c=*tokens[i-1];
                  if(c==')' || c==']' || c.type==TOKEN_KEYWORD || c.type==TOKEN_CODE || c==TMPL_E)detect=false; // don't detect if we're preceeded by some possible symbol
               }

               // detect var definition
               Symbol *symbol;
               if(detect && IsVarFuncDefinition(tokens, i, temp, symbol, templates, token.parent))
                  if(symbol && (symbol->modifiers&Symbol::MODIF_DATA_TYPE) && isDeclaration(symbol, start))
                     ReadVarFunc(symbol, symbols, tokens, i, start, SPACE_NORMAL, temp, templates, new_templates);
            }
            MAX(i, start+1);
         }
      }
   }
}
void Source::parseFunc(Symbol &func)
{
   Memc<Command> cmds;
   Memc<Message> msgs;
   parseFunc(func, cmds, msgs);
}
void Source::parseFunc(Token &token)
{
   if(token.parent)if(Symbol *func=token.parent->func())parseFunc(*func);
}
void Source::parseFunc(C VecI2 &cur)
{
   if(!active && !header)return;
   Int i; if(Token *token=findPrevToken(cur, i))parseFunc(*token);
}
void Source::parseCurFunc() {parseFunc(cur);}
/******************************************************************************/
// GET
/******************************************************************************/
Bool Source::hasUnicode(          )C {REPA(lines)if(HasUnicode(lines[i]))return true; return false;}
Bool Source::used      (          )C {return active || header;}
Bool Source::modified  (          )C {return undos.undos()!=undo_original_state;}
Bool Source::hasKbFocus(          )C {if(CE.view_mode() || Gui.kb()==&suggestions_textline)return false; return contains(Gui.kb()) || ((!Gui.kb() || Gui.kb()==Gui.desktop()) && visibleFull() && parent());}
Bool Source::hasFocus  (GuiObj *go)C {return App.active() && contains(go) && !slidebar[0].contains(go) && !slidebar[1].contains(go) && !view.contains(go) && !suggestions_region.contains(go) && go!=&suggestions_textline;}

Bool Source::isCurVisible()C
{
   return cur.y  >=Ceil ( slidebar[1].offset()                     /CE.ts.lineHeight())
       && cur.y+1<=Trunc((slidebar[1].offset()+clientHeight()     )/CE.ts.lineHeight())
       && cur.x  >=Ceil ((slidebar[0].offset()+CE.lineNumberSize())/CE.ts. colWidth ())
       && cur.x+1<=Trunc((slidebar[0].offset()+clientWidth()      )/CE.ts. colWidth ());
}

Bool Source::insideRSTBrackets(C VecI2 &pos) // if inside Round Square Template brackets
{
   Int level=0, i; if(findPrevToken(pos, i))for(; i>=0; )
   {
      Token &token=*tokens[i--];
      if(token=='(' || token=='[' || token=='<' || token==TMPL_B)level++;else
      if(token==')' || token==']' || token=='>' || token==TMPL_E)level--;
      if(token=='{' || token=='}')break;
   }
   return level>0;
}

Vec2 Source::posVisual(C VecI2 &pos)C {return pos*Vec2(CE.ts.colWidth(), -CE.ts.lineHeight());}
Vec2 Source::posCur   (C Vec2  &pos)C {Vec2 c=pos-T.pos(); c-=offset(); c/=Vec2(CE.ts.colWidth(), -CE.ts.lineHeight()); return c;}
Vec2 Source::offset   (            )C {return Vec2(-slidebar[0].offset()+CE.lineNumberSize(), slidebar[1].offset());}

Int Source::viewToReal(Int view)C
{
   if(InRange(view, view_lines))return view_lines[view].line();
   if(view_lines.elms())return view_lines.last().line()+view-view_lines.elms()+1;
   return view;
}
Bool Source::viewToReal(C VecI2 &view, VecI2 &real)C
{
   if(InRange(view.y, view_lines))
   {
    C ViewLine &view_line=view_lines[view.y];
      if(InRange(view_line.line(), lines))
      {
       C Line &line=lines[view_line.line()];
         if(InRange(view.x, view_line))
         {
            real.set(view_line.cols[view.x].pos.x, view_line.line()); return InRange(real.x, line);
         }
         real.set(line.length(), view_line.line()); return false; // set 'real' as close as possible
      }
   }
   real.set(0, lines.elms()); return false; // set 'real' as close as possible
}
Int Source::realToView(Int real)C
{
   REPAD(y, view_lines)if(view_lines[y].line()<=real)return y;
   return 0;
}
VecI2 Source::realToView(C VecI2 &real)C
{
   Int y=realToView(real.y);
   if(InRange(y, view_lines))
   {
    C ViewLine &view_line=view_lines[y];
      if(view_line.line()==real.y)return VecI2(Max(0, view_line.findCol(real.x)), y);
      return VecI2(0, y);
   }
   return VecI2(0, 0);
}
/******************************************************************************/
Str Source::textTokens()C
{
   Str    out; FREPA(lines)out.line()+=lines[i].textTokens();
   return out;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Source::replacePath(C Str &src, C Str &dest) {loc.replacePath(src, dest);}
void Source::setScroll()
{
   Bool immediate=CE.options.imm_scroll();
   Flt  relative =(immediate ? 0                    : 0.5f),
        base     =(immediate ? CE.ts.lineHeight()*7 : 0   );
   slidebar[0].scrollOptions(relative, base, immediate);
   slidebar[1].scrollOptions(relative, base, immediate);
}
void Source::setHideSlideBar()
{
   alwaysHideHorizontalSlideBar(CE.options.hide_horizontal_slidebar());
}
void Source::curSel(VecI2 &min, VecI2 &max)
{
   min=sel;
   max=cur;
   if(max.y<min.y || (max.y==min.y && max.x<min.x))Swap(min, max);
}
VecI2& Source::dec(VecI2 &p)
{
   if(p.x>=0)p.x--;
   for(; p.x<0 && p.y>=0; )
   {
      p.y--;
      if(InRange(p.y, lines))p.x=lines[p.y].length()-1;
   }
   return p;
}
VecI2& Source::inc(VecI2 &p)
{
   if(InRange(p.y, lines))
   {
      if(p.x>=lines[p.y].length()-1)
      {
         p.x=0;
         do p.y++; while(InRange(p.y, lines) && !lines[p.y].length());
      }else p.x++;
   }
   return p;
}
void Source::setTokenPtrs()
{
   // set continuous tokens for the whole file
   Int token_index=0;
   tokens.clear(); FREPA(lines)
   {
      Memc<Token> &tokens=lines[i].Tokens(); FREPA(tokens)
      {
         Token &token=tokens[i]; T.tokens.add(&token); token.source_index=token_index++;
      }
   }
}
void Source::changed(Int from, Int num)
{
   view_lines.del();

   // validate types and line indexes of all lines (this is fast because types are recalculated only on change, and also line indexes/sources are required to setup everytime for all lines)
   Line *prev=null;
   FREPA(lines)
   {
      Line &l=lines[i]; l.line=i; l.source=this;
      l.setType(prev ? prev->ends_with_comment : false, prev ? prev->ends_with_preproc : false);
      prev=&l;
   }

   // process
   delSymbols     ();
   detectDefines  ();
   preprocess     (from, num);
   detectDataTypes();
   linkDataTypes  ();
   detectVarFuncs ();
   parseCurFunc   ();

   if(CE.view_mode() && CE.cur()==this)validateView();

   // set gui
   setGui();

   // set modification time
   modify_time.getUTC();
}
void Source::exist(Int x, Int y)
{
   if(y>=lines.elms())lines.setNum(y+1);
   REP(x-lines[y].length())lines[y].append(' ');
}
/******************************************************************************/
static Int CompareToken(Token* C &token, C VecI2 &pos)
{
   if(Int c=Compare(token->lineIndex(), pos.y))return c;
   return   Compare(token->col        , pos.x);
}
static Int CompareTokenOverlap(Token* C &token, C VecI2 &pos)
{
   if(Int c=Compare(token->lineIndex(), pos.y))return c;
   if(token->col+token->length()-1<pos.x)return -1;
   if(token->col                  >pos.x)return +1;
                                         return  0;
}
Token* Source::findPrevToken(C VecI2 &pos, Int &i)
{
   VecI2 p(pos.x-1, pos.y);
   if(tokens.binarySearch(p, i, CompareToken))return tokens[i];
   if(InRange(--i, tokens))
   {
      Token &token=*tokens[i]; if(token.macro_col<0)return &token; // this is not a token made by macro

      // go left to first token that is created by that macro
      for(; ; i--)
      {
         if(InRange(i-1, tokens))
         {
            Token &prev=*tokens[i-1];
            if(prev.sameMacro(token))continue;
         }
         break;
      }

      // look by going right
      Token *nearest=null;
      Int    nearest_i;
      for(; i<tokens.elms(); i++)
      {
         Token *cur=tokens[i]; switch(CompareTokenOverlap(cur, p))
         {
            case  0: return cur;
            case  1: goto too_far;
            case -1: nearest=cur; nearest_i=i; break;
         }
      }
   too_far:
      if(nearest){i=nearest_i; return nearest;}
   }
   i=-1; return null;
}
Token* Source::findToken(C VecI2 &pos, Int &i)
{
   VecI2 p=pos;
   if(tokens.binarySearch(p, i, CompareToken))return tokens[i];
   if(InRange(--i, tokens))
   {
      Token &token=*tokens[i]; if(token.lineIndex()==pos.y && token.col+token.length()>pos.x)return &token;
      if(token.macro_col>=0)
      {
         // go left to first token that is created by that macro
         for(; ; i--)
         {
            if(InRange(i-1, tokens))
            {
               Token &prev=*tokens[i-1];
               if(prev.sameMacro(token))continue;
            }
            break;
         }

         // look by going right
         for(; i<tokens.elms(); i++)
         {
            Token *cur=tokens[i]; if(!cur->sameMacro(token))break;
            if(cur->lineIndex()==pos.y && cur->col+cur->length()>pos.x)return cur;
         }
      }
   }
   i=-1; return null;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Source::~Source()
{
   CE.sourceRemoved(T);
   if(win_io_save)Delete(win_io_save);
}
/******************************************************************************/
// IO
/******************************************************************************/
enum
{
   HEADER   =1<<0,
   EE_HEADER=1<<1,
   CPP      =1<<2,
   SRC_CONST=1<<3,
};
Bool Source::save(File &f, StrLibrary &sl)C
{
   f.putByte((header ? HEADER : 0)|(ee_header ? EE_HEADER : 0)|(cpp ? CPP : 0)|(Const ? SRC_CONST : 0));
   sl.putStr(f, loc.file_name);
   if(!modify_time.save(f))return false;
   f.cmpUIntV(symbols.elms()); FREPA(symbols){C SymbolDef  &symbol=symbols[i]; sl.putStr(f, symbol.name()); if(!symbol->save(f, sl))return false;} // write before 'lines'
   f.cmpUIntV(decls  .elms()); FREPA(decls  ){C SymbolDecl &decl  =  decls[i]; sl.putStr(f, decl  .name()); if(!decl  ->save(f, sl))return false;} // write before 'lines'
   f.cmpUIntV(lines  .elms()); FREPA(lines  ){C Line       &line  =  lines[i];                              if(!line   .save(f, sl))return false;}
   return f.ok();
}
#define SOURCE_DEBUG_SPEED_SIZE 0
#if SOURCE_DEBUG_SPEED_SIZE
   static Dbl F0, F1, F2;
   static Int S0, S1, S2;
   struct XX{~XX(){Exit(S+F0+"   "+F1+"   "+F2+"\n"+S0+"   "+S1+"   "+S2);}}x;
#endif
Bool Source::load(File &f, StrLibrary &sl, Str &temp)
{
   Byte flag; f>>flag;
   header   =FlagTest(flag, HEADER   );
   ee_header=FlagTest(flag, EE_HEADER);
   cpp      =FlagTest(flag, CPP      );
   Const    =FlagTest(flag, SRC_CONST);
   sl.getStr(f, loc.file_name); loc.setFile(loc.file_name);
   if(!modify_time.load(f))goto error;
#if SOURCE_DEBUG_SPEED_SIZE
Dbl t=Time.curTime(); Int s=f.pos();
#endif
   symbols.setNum(f.decUIntV()); FREPA(symbols){sl.getStr(f, temp); if(!symbols[i].require(temp)->load(f, sl, T))goto error;}
#if SOURCE_DEBUG_SPEED_SIZE
F0+=Time.curTime()-t; t=Time.curTime(); S0+=f.pos()-s; s=f.pos();
#endif
   decls  .setNum(f.decUIntV()); FREPA(decls){sl.getStr(f, temp); if(!decls[i].require(temp)->load(f, sl, T))goto error;}
#if SOURCE_DEBUG_SPEED_SIZE
F1+=Time.curTime()-t; t=Time.curTime(); S1+=f.pos()-s; s=f.pos();
#endif
   lines  .setNum(f.decUIntV()); FREPA(lines)if(!lines[i].load(f, sl, i, T, temp))goto error;
#if SOURCE_DEBUG_SPEED_SIZE
F2+=Time.curTime()-t; t=Time.curTime(); S2+=f.pos()-s; s=f.pos();
#endif
   setTokenPtrs();
   setGui();
   if(f.ok())return true;
error:
   del(); return false;
}
/******************************************************************************/
ERROR_TYPE Source::load()
{
   ERROR_TYPE error;
   cpp=false;
   Str data;
   if(!loc.is())error=EE_ERR_ELM_NOT_FOUND;else
   {
      if(loc.file)
      {
         cpp=true;
         FileText f; if(!f.read(loc.file_name))error=EE_ERR_FILE_NOT_FOUND;else{error=EE_ERR_NONE; f.getAll(data);}
      }else
      {
         error=CE.cei().sourceDataLoad(loc.id, data);
      }
   }
   sel=-1; cur=0;
   fromText(data);
   if(error==EE_ERR_NONE && loc.file){FileInfo fi; if(fi.getSystem(loc.file_name)){modify_time=fi.modify_time_utc; Const=FlagTest(fi.attrib, FATTRIB_READ_ONLY);}} // adjust modification time after 'changed'
   return error;
}
void Source::reload()
{
   Bool modified=T.modified(); delUndo();
   if(  modified)load();
}
ERROR_TYPE Source::load(C SourceLoc &loc)
{
   if(lines.elms())setUndo();
   T.loc=loc;
   undo_original_state=undos.undos();
   return load();
}
Bool Source::saveTxt(C Str &name)
{
   FileText f; if(f.write(name, hasUnicode() ? UTF_8 : ANSI))
   {
      FREPA(lines){if(i)f.endLine(); f.putText(lines[i]);}
      if(f.flushOK())return true;
    //f.del(); FDelFile(name); no need to delete partially written text files
   }
   return false;
}
Bool Source::save(C SourceLoc &loc)
{
   Bool ok=false;
   if(!loc.is())
   {
      Gui.msgBox(S, S+"Invalid save location");
   }else
   if(loc.file)
   {
      ok=saveTxt(loc.file_name);
      if(!ok)Gui.msgBox(S, S+"Error saving to \""+loc.file_name+'"');
   }else
   {
      ok=CE.cei().sourceDataSave(loc.id, asText());
      if(!ok)Gui.msgBox(S, S+"Error saving source code");
   }
   if(ok)
   {
      T.loc=loc;
      undo_original_state=undos.undos();
      modify_time.getUTC(); // when saving, adjust new modification time of the file, this is needed in case of: start build, made change during build, end build, build again (during first build .obj file could get generated with date newer than change applied during build, adjusting date during save would guarantee newer version of the file)
      forceCreateNextUndo();
      if(loc.file)FTimeUTC(loc.file_name, modify_time);
      if(header)CE.rebuildSymbols(true);
      if(CE.cur()==this)CE.cei().sourceChanged();
   }
   return ok;
}
static void SaveEs(C Str &name, Source &source) {CE.save(&source, name);}
void Source::save()
{
   if(!win_io_save)New(win_io_save)->create(CODE_EXT, S, GetPath(GetPath(DataPath())), SaveEs, SaveEs, T);
   win_io_save->ext(GetExt(loc.file_name)+'|'+CODE_EXT); // use already existing extension as the 1st and most important
   win_io_save->save();
}
Bool Source::overwrite() // returns true if saved successfully (immediately)
{
   if(loc.is())return save(loc);
   save();
   return false;
}
/******************************************************************************/
static void SuggestionChanged(Source &src) {src.refreshSuggestions();}
Source::Source() : undos(true, this, 15)
{
   lit_symbol=null;
   win_io_save=null;
   active=was_active=header=ee_header=false; opened=false; Const=false; view_comments=view_funcs=true; view_func_bodies=view_private_members=false;
   recursive=parse_count=0; preproc_line_changed=highlight_line=-1; highlight_time=0; cur=0; sel=sel_temp=-1; lc_offset.zero(); delUndo();
   create().skin(&CE.source_skin, false); kb_lit=false; setScroll(); setHideSlideBar();
   T+=suggestions_region  .create().hide();
   T+=suggestions_textline.create().func(SuggestionChanged, T).hide();
   ListColumn lc[]=
   {
      ListColumn(MEMBER(Suggestion, icon   ), 0.05f   , "icon"),
      ListColumn(MEMBER(Suggestion, display), LCW_DATA, "text"),
   };
   suggestions_region+=suggestions_list.create(lc, Elms(lc), true).skin(&CE.suggestions_skin);
   FlagDisable(suggestions_list.flag, LIST_SEARCHABLE|LIST_SORTABLE);
   suggestions_list.cur_mode=LCM_ALWAYS;
   resize();
}
/******************************************************************************/
}}
/******************************************************************************/
