/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Token* Source::TokenSource::previewNext()
{
   if(tokens && InRange(token_index, *tokens))return &(*tokens)[token_index];                                  // first try custom token list
   if(source                                 )return source->previewNextToken(*line_index, *line_token_index); // then  try tokens from the source
   return null;
}
Token* Source::TokenSource::next(Bool only_cur_line)
{
   if(tokens && InRange(token_index, *tokens))return &(*tokens)[token_index++];                                           // first try custom token list
   if(source                                 )return source->getNextToken(*line_index, *line_token_index, only_cur_line); // then  try tokens from the source
   return null;
}
void Source::TokenSource::removeLastRead() {if(tokens && token_index>0)tokens->remove(--token_index, true);}
/******************************************************************************/
Token* Source::previewNextToken(Int line_index, Int line_token_index)
{
   if(InRange(line_index, lines))
   {
      Line &line=lines[line_index]; if(InRange(line_token_index, line.tokens))return &line.tokens[line_token_index];
      for(Int i=line_index+1; i<lines.elms(); i++)
      {
         Line &line=lines[i]; if(line.tokens.elms())return &line.tokens[0];
      }
   }
   return null;
}
Token* Source::getNextToken(Int &line_index, Int &line_token_index, Bool only_cur_line)
{
   for(;;)
   {
      if(!InRange(      line_index, lines      ))return null; Line &line=lines[line_index];
      if( InRange(line_token_index, line.tokens))return &line.tokens[line_token_index++]; // if we still have tokens in current line
      if( only_cur_line                         )return null;
      line_token_index=0; line_index++; if(InRange(line_index, lines))lines[line_index].resetPreproc(true); // need to proceed to next line, when doing so we need to initialize its macro state
   }
}
/******************************************************************************/
Int Source::findMacro(Memc<Macro> &macros, BStr &macro, Mems<Bool> *macro_used)
{
   Int index; Bool found=macros.binarySearch(macro, index, CompareCS);
   if(    found && macro_used && InRange(index, *macro_used) && (*macro_used)[index])found=false; // if macro was already used then skip
   return found ? index : ~index;
}
Int Source::findMacroToReplace(Memc<Macro> &macros, BStr &macro, TokenSource &ts, Mems<Bool> *macro_used)
{
   Int index=findMacro(macros, macro, macro_used);
   if(InRange(index, macros) && macros[index].params>=0) // if macro was defined with (..) then require source tokens to have (..) as well, otherwise don't replace (this is how C++ works)
   {
      Token *next=ts.previewNext();
      if(!(next && *next=='('))return ~index; // if next token is not '(' then don't detect the macro replacement
   }
   return index;
}
Int Source::findMacroToReplace(Memc<Macro> &macros, BStr &macro, Int line_index, Int line_token_index)
{
   Int index=findMacro(macros, macro);
   if(InRange(index, macros) && macros[index].params>=0) // if macro was defined with (..) then require source tokens to have (..) as well, otherwise don't replace (this is how C++ works)
   {
      Token *next=previewNextToken(line_index, line_token_index+1);
      if(!(next && *next=='('))return ~index; // if next token is not '(' then don't detect the macro replacement
   }
   return index;
}
/******************************************************************************/
void Source::replaceMacro(Int macro_index, Memc<Macro> &macros, TokenSource &ts, Mems<Bool> &macro_used, Int depth, Int col, Line &line)
{
   // read parameters of macro
   Memc<MacroParam> params;
   Macro &macro=macros[macro_index];
   if(macro.params>=0)
   {
      // store current position in token buffer
      Int prev_pos=ts.token_index;

      if(Token *token=ts.next())
         if(*token=='(')
            for(Int level=0; Token *token=ts.next(); )
      {
         switch((*token)[0])
         {
            case '(': level++; break;
            case ')': level--; break;
         }
         if( level<0)break; if(!params.elms())params.New();
         if(!level && *token==',')params.New ();
         else                     params.last().tokens.add(*token);
      }

      // remove read elements from token buffer
      REP(ts.token_index-prev_pos)ts.removeLastRead();

      // expand parameter macros
      /*Mems<Bool> macro_used; this shouldn't be done here, just rely on macro replacement coded below
      FREPA(params)
      {
         TokenSource ts;
         ts.tokens     =&params[i].tokens;
         ts.token_index=0;
         for(; InRange(ts.token_index, *ts.tokens); )
         {
            Token &token=(*ts.tokens)[ts.token_index];
            Int macro_index=findMacro(macros, token);
            if( macro_index>=0)
            {
               ts.tokens->remove(ts.token_index, true); // remove macro token
               replaceMacro(macro_index, macros, ts, macro_used, 1, col, line);
            }else
            {
               ts.token_index++;
            }
         }
      }*/
   }

   // write
   Int token_index=ts.token_index;
   FREPA(macro.parts) // iterate through all parts of the macro
   {
      Int         start=token_index;
      Macro::Part &part=macro.parts[i];
      if(InRange(part.param, params)) // if the part is an argument
      {
         MacroParam &mp=params[part.param];
         FREPA(mp.tokens)
         {
            Token &token=mp.tokens[i];
            ts.tokens->NewAt(token_index++).set(token, token.col, *token.line, token.type).macroPos(col, line).macroDepth(depth);
         }
      }else
      // TODO: can 'setCustom' be replaced with 'set' ? remember that Macro can be defined in one source, part.text BStr points to string data from that source, then other source uses this macro and its token BStr will point to string data from macro source
      if(part.text.is())ts.tokens->NewAt(token_index++).setCustom(part.text, col, line, part.type).macroPos(col, line).macroDepth(depth);

      for(; start<token_index; start++)if(Token *token=ts.tokens->addr(start)) // process all new tokens
      {
         if(*token=="__LINE__"){token->BStr::setCustom(S+line.line); token->type=TOKEN_NUMBER;} // replace '__LINE__' with code line
      }

      Int last =token_index-1,
          first=token_index-3;

      if(InRange(first, *ts.tokens)
      && InRange(last , *ts.tokens) && (*ts.tokens)[first+1]=="##") // if previous token was merger then merge the neighbours
      {
         Token &a=(*ts.tokens)[first],
               &b=(*ts.tokens)[last ];
         a.BStr::setCustom(a.asStr()+b.asStr());
         ts.tokens->remove(first+1, true).remove(first+1, true); token_index-=2;
      }
   }

   // verify result for other macros to be replaced (skipping those already processed)
   // macro.used=true; don't do here but only if another macro was detected below (this benefits in less often memory allocation requirement)
   for(; InRange(ts.token_index, *ts.tokens); )
   {
      Token &token=(*ts.tokens)[ts.token_index]; if(token.macro_depth<depth)break; // don't process parent tokens (process only from this or lower level)
      ts.next(); // increase token_index
      Int macro_index2=findMacroToReplace(macros, token, ts, &macro_used);
      if( macro_index2>=0 && macro_index2!=macro_index) // if we've detected another macro
      {
         if(!macro_used.elms())macro_used.setNumZero(macros.elms()); macro_used[macro_index]=true; // we must specify which macros were used
         ts.removeLastRead();
         replaceMacro(macro_index2, macros, ts, macro_used, depth+1, col, line);
      }
   }
   if(InRange(macro_index, macro_used))macro_used[macro_index]=false; // macro.used=false;
}
/******************************************************************************/
Bool Source::getConditionalValue(Memc<Token> &tokens, Memc<Macro> &macros, Line &line)
{
   Mems<Bool> macro_used;

   // replace "defined(x)" and "defined x" to "1" or "0"
   FREPA(tokens)if(tokens[i]=="defined")
   {
      tokens.remove(i, true); // remove 'defined'
      if(InRange(i, tokens) && tokens[i]=='(')tokens.remove(i, true).remove(i+1, true); // remove '(' ')' in "defined(x)"
      if(InRange(i, tokens))
      {
         Token &macro=tokens[i];
         macro.type=TOKEN_NUMBER;
         macro.BStr::setBorrowed((findMacro(macros, macro)>=0) ? u"1" : u"0"); // if macro exists, then set "1", if not then set "0"
      }
   }

   // replace macros
   FREPA(tokens)
   {
      Token &token=tokens[i];
      if(token.type==TOKEN_CODE || token.type==TOKEN_KEYWORD)
      {
         Int macro_index =findMacroToReplace(macros, token, line.line, i);
         if( macro_index>=0)
         {
            tokens.remove(i, true); // remove macro

            TokenSource ts;
            ts.tokens     =&tokens;
            ts.token_index=i;

            // replace macro
            REPAO(macro_used)=false; replaceMacro(macro_index, macros, ts, macro_used, 1, 0, line);

            // process all remaining tokens
            for(; Token *token=ts.next(); )
            {
                  macro_index =findMacroToReplace(macros, *token, ts);
               if(macro_index>=0)
               {
                  ts.removeLastRead();
                  REPAO(macro_used)=false; replaceMacro(macro_index, macros, ts, macro_used, 1, 0, line);
               }
            }
            break;
         }
      }
   }

   Str s; FREPA(tokens)
   {
      Token &token=tokens[i];
      s.space()+=((token.type==TOKEN_CODE || token.type==TOKEN_KEYWORD) ? "0" : token.asStr()); // replace all text with "0" value
   }
   return CalculateI(s)!=0;
}
/******************************************************************************/
void Source::preprocess(Memc<Macro> &macros, Int &line_index, Memc<Token*> &temp, Bool allow_defines, Memc<PrepCond> &prep_if)
{
   Bool             vis  =(recursive<=1 && parse_count==0); // if apply visuals
   Int start_line_index  =line_index;
   Line           &line  =lines[line_index]; line.resetPreproc(false); if(vis)line.tokens_preproc_condition_unavailable=false;
   Memc<Token>    &tokens=line.tokens;

   // preprocessor tokens - detect macro changes
   if(line.preproc)
   {
      if(!line.starts_with_preproc && !line.starts_with_macro_param)
      {
         Bool handled_unavailable=true;
         if(tokens.elms()>=2 && tokens[0]=='#')
         {
            if(tokens[1]=="define" && allow_defines) // add new macro
            {
               if(!prep_if.elms() || prep_if.last().currently_valid)
               {
                  if(Token *name=tokens.addr(2))
                     if(name->type==TOKEN_CODE || name->type==TOKEN_KEYWORD)
                  {
                     // line.macros=macros; line.has_macros=true;
                     Int    index=findMacro(macros, *name);
                     Macro &macro=((index>=0) ? macros[index].reset() : macros.NewAt(~index).set(*name)); macro.def=line;
                     Int    i=3; temp.clear();
                     if(InRange(i, tokens) && tokens[i]=='(' && tokens[i]()==(*name)()+name->length()) // if macro name followed directly by '('
                     {
                        // detect macro params
                        macro.params=0;
                        for(i++; i<tokens.elms(); ) // skip '('
                        {
                           Token &token=tokens[i++];
                           if(token==',')continue;
                           if(token.type==TOKEN_CODE)
                           {
                              macro.params++;
                              temp.add(&token);
                              continue;
                           }
                           break;
                        }
                     }
                     // get macro body
                     for(; Token *token=getNextToken(line_index, i, true); )
                     {
                        if(*token=='\\' && token->macro_col<0 && token->col==token->line->length()-1) // if this is \ character located at the end of the line
                        {
                           if(!InRange(line_index+1, lines))break; // if there is no next line then break
                           i=0; // start with first token in next line
                           Line &line=lines[++line_index]; line.resetPreproc(false); if(vis)line.tokens_preproc_condition_unavailable=false; // increase line counter and initialize the next line
                        }else
                        {
                           if(token->type==TOKEN_CODE)REPA(temp)if((*temp[i])==*token)
                           {
                              macro.parts.New().set(token->type, i, null);
                              goto added;
                           }
                           macro.parts.New().set(token->type, -1, token);
                        added:;
                        }
                     }
                  }
               }else if(vis)line.tokens_preproc_condition_unavailable=true;
            }else
            if(tokens[1]=="undef" && allow_defines) // remove macro
            {
               if(!prep_if.elms() || prep_if.last().currently_valid)
               {
                  // TODO: line.ends_with_preproc
                  if(Token *name=tokens.addr(2))
                     if(name->type==TOKEN_CODE || name->type==TOKEN_KEYWORD)
                  {
                     Int index=findMacro(macros, *name); // if(index>=0){line.has_macros=true; FREPA(macros)if(i!=index)line.macros.add(macros[i]);}
                     macros.remove(index, true);
                  }
               }else if(vis)line.tokens_preproc_condition_unavailable=true;
            }else
            if(tokens[1]=="if")
            {
               // TODO: line.ends_with_preproc
               if(!prep_if.elms() || prep_if.last().currently_valid)
               {
                  for(Int i=2; i<tokens.elms(); i++)line.tokens_preproc.New()=tokens[i];
                  Bool value=getConditionalValue(line.tokens_preproc, macros, line);
                  prep_if.New().set(!value, value);
               }else
               {
                  prep_if.New().set(false, false);
                  if(vis)line.tokens_preproc_condition_unavailable=true;
               }
            }else
            if(tokens[1]=="ifdef")
            {
               if(!prep_if.elms() || prep_if.last().currently_valid)
               {
                  line.tokens_preproc.New().setBorrowed(u"defined", -1, 0, line, TOKEN_PREPROC);
                  for(Int i=2; i<tokens.elms(); i++)line.tokens_preproc.New()=tokens[i];
                  Bool value=getConditionalValue(line.tokens_preproc, macros, line);
                  prep_if.New().set(!value, value);
               }else
               {
                  prep_if.New().set(false, false);
                  if(vis)line.tokens_preproc_condition_unavailable=true;
               }
            }else
            if(tokens[1]=="ifndef")
            {
               if(!prep_if.elms() || prep_if.last().currently_valid)
               {
                  line.tokens_preproc.New().setBorrowed(u"!"      , -1, 0, line, TOKEN_OPERATOR);
                  line.tokens_preproc.New().setBorrowed(u"defined", -1, 0, line, TOKEN_PREPROC );
                  for(Int i=2; i<tokens.elms(); i++)line.tokens_preproc.New()=tokens[i];
                  Bool value=getConditionalValue(line.tokens_preproc, macros, line);
                  prep_if.New().set(!value, value);
               }else
               {
                  prep_if.New().set(false, false);
                  if(vis)line.tokens_preproc_condition_unavailable=true;
               }
            }else
            if(tokens[1]=="elif")
            {
               // TODO: line.ends_with_preproc
               if(prep_if.elms() && prep_if.last().can_enter)
               {
                  for(Int i=2; i<tokens.elms(); i++)line.tokens_preproc.New()=tokens[i];
                  Bool value=getConditionalValue(line.tokens_preproc, macros, line);
                  prep_if.last().set(!value, value);
               }else
               {
                  if(!prep_if.elms())prep_if.New();
                  prep_if.last().set(false, false);
               }
               if(vis && prep_if.elms()>=2 && !prep_if[prep_if.elms()-2].currently_valid)line.tokens_preproc_condition_unavailable=true;
            }else
            if(tokens[1]=="else")
            {
               if(prep_if.elms())
               {
                  Bool value=prep_if.last().can_enter;
                  prep_if.last().set(false, value);
               }
               if(vis && prep_if.elms()>=2 && !prep_if[prep_if.elms()-2].currently_valid)line.tokens_preproc_condition_unavailable=true;
            }else
            if(tokens[1]=="endif")
            {
               prep_if.removeLast();
               if(vis && prep_if.elms() && !prep_if.last().currently_valid)line.tokens_preproc_condition_unavailable=true;
            }else
               handled_unavailable=false;
         }else
            handled_unavailable=false;

         if(!handled_unavailable)
            if(vis && prep_if.elms() && !prep_if.last().currently_valid)line.tokens_preproc_condition_unavailable=true;
      }

      // if this is a preprocess line, then no actual tokens should be used for the code compilation
      for(Int i=start_line_index; i<=line_index; i++) // call for each processed line
      {
         Line &line=lines[i];
         line.tokens_preproc_use=true;
         line.tokens_preproc.clear();
      }
   }else
   if(prep_if.elms() && !prep_if.last().currently_valid)
   {
      // if there was a preprocess condition which wasn't met, then no actual tokens should be used for the code compilation
      if(vis)line.tokens_preproc_condition_unavailable=true;
      line.tokens_preproc_use=true;
      line.tokens_preproc.clear();
   }else
   if(macros.elms()) // normal line, try to replace tokens with macros
   {
      Mems<Bool> macro_used;
      FREPA(tokens)
      {
         Token &token=tokens[i];
         if(token.type==TOKEN_CODE || token.type==TOKEN_KEYWORD)
         {
            Int macro_index =findMacroToReplace(macros, token, line_index, i);
            if( macro_index>=0) // we've encountered token which should be overriden by macro
            {
               // replace with macro
               line .tokens_preproc_use=true;
               token.macro             =true;
               for(Int j=0; j<i; j++)line.tokens_preproc.add(tokens[j]); // copy all tokens up to now into preprocessed token buffer

               i++; // skip macro token
               TokenSource ts;
               ts.tokens          =&line.tokens_preproc;
               ts.token_index     = line.tokens_preproc.elms();
               ts.source          =this;
               ts.line_index      =&line_index;
               ts.line_token_index=&i;

               // replace macro
               REPAO(macro_used)=false; replaceMacro(macro_index, macros, ts, macro_used, 1, token.col, *token.line);

               // process all remaining in this line
               ts.token_index=ts.tokens->elms(); // move at the end so next token will be from source
               for(; Token *token=ts.next(true); )
               {
                  Int macro_index=findMacroToReplace(macros, *token, ts);
                  if( macro_index>=0){token->macro=true; REPAO(macro_used)=false; replaceMacro(macro_index, macros, ts, macro_used, 1, token->col, *token->line);} // if it's a macro, then trigger macro replacement function
                  else               {ts.tokens->add(*token);                                                                                                    } // copy normally
                  ts.token_index=ts.tokens->elms(); // move at the end so next token will be from source
               }
               break;
            }
         }
      }
   }

 //return line.has_macros ? line.macros : macros;
}
/******************************************************************************/
void Source::detectDefines()
{
   // TODO: 
   if(active)
   {
   }
}
/******************************************************************************/
void Source::preprocess(Int from, Int num)
{
   tokens.clear();
   if(active)
   {
      // preprocess
      Int max=lines.elms()-1,
          to =((num<0) ? max : Min(from+num-1, max));
      MAX(from, 0);

      if(preproc_line_changed>=0) // if any of the preprocessor lines was changed, then update from that line till the end of the file
      {
         MIN(from, preproc_line_changed); preproc_line_changed=-1;
         from=0; // preprocess from the start, because currently 'prep_if' is created from current position only
         to=max;
      }

      if(from<=to)
      {
         // preprocess tokens
         if (        from>0         && (lines[from-1].starts_with_macro_param || lines[from-1].starts_with_preproc || lines[from-1].tokens_preproc_condition_unavailable)) from--;
         for(;       from>0         && (lines[from  ].starts_with_macro_param || lines[from  ].starts_with_preproc || lines[from  ].tokens_preproc_condition_unavailable); from--); // go to first line                which doesn't start with macro param or preprocess
         for(; InRange(to+1, lines) && (lines[  to+1].starts_with_macro_param || lines[  to+1].starts_with_preproc                                                      );   to++); // expand range to cover all lines which         start with macro param or preprocess

       //Memc<Macro   > *macros=&macrosForLine(from);
         Memc<Token*  > temp;
         Memc<PrepCond> prep_if;
         Int cur=from; for(; cur<=to; cur++)preprocess(ProjectMacros, cur, temp, false, prep_if); // preprocess lines
         FREPA(view_lines){ViewLine &line=view_lines[i]; if(line.line()>=from && line.line()<cur)line.text_valid=false;} // force text reset on all processed lines (use 'cur' because it may be bigger than 'to')
      }
   }
   setTokenPtrs();
}
/******************************************************************************/
}}
/******************************************************************************/
