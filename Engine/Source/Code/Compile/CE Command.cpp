/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
// WRITE COMMANDS
/******************************************************************************/
static Str TextTokens(Memc<Token*> &tokens, Int from, Int to)
{
   Str text; TOKEN_TYPE last_type=TOKEN_NONE;
   for(Int i=from; i<=to; i++)
   {
      Token &token=*tokens[i];
      if(MustSeparate(last_type, token.type))text+=' ';
      text+=token;
      last_type=token.type;
   }
   return text;
}
/******************************************************************************
static void ListCommands(FileText &ft, Memc<Command> &cmds, Memc<Token*> &tokens)
{
   FREPA(cmds)
   {
      Str      line;
      Command &cmd=cmds[i];
      switch(  cmd.type)
      {
         case CMD_INSTRUCT   : line=S+          TextTokens(tokens, cmd. raw_range.x, cmd. raw_range.y)+";"; break;
         case CMD_IF         : line=S+"if("    +TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+")"; break;
         case CMD_FOR        : line=S+"for("   +TextTokens(tokens, cmd.init_range.x, cmd.init_range.y)+"; "+TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+"; "+TextTokens(tokens, cmd.step_range.x, cmd.step_range.y)+")"; break;
         case CMD_WHILE      : line=S+"while(" +TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+")"; break;
         case CMD_DO         : line=  "do"                                                                ; break;
         case CMD_SWITCH     : line=S+"switch("+TextTokens(tokens, cmd. raw_range.x, cmd. raw_range.y)+")"; break;
         case CMD_RETURN     : line=S+"return "+TextTokens(tokens, cmd. raw_range.x, cmd. raw_range.y)+";"; break;
         case CMD_BREAK      : line=S+"break;"                                                            ; break;
         case CMD_CONTINUE   : line=S+"continue;"                                                         ; break;
         case CMD_GOTO       : line=S+"goto "  +TextTokens(tokens, cmd. raw_range.x, cmd. raw_range.y)+";"; break;
         case CMD_GOTO_COND  : line=S+"if("    +TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+")goto @"+cmd.label_index+";"; break;
         case CMD_GOTO_COND_N: line=S+"if(!"   +TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+")goto @"+cmd.label_index+";"; break;
         case CMD_LABEL      : line=S+"@"      +cmd.label_index+":"; break;
      }
      if(line.is())ft.putLine(line);
      if(cmd.cmds.elms())
      {
         ft++;
         ListCommands(ft, cmd.cmds, tokens);
         ft--;
      }
      if(cmd.cmds_false.elms())
      {
         ft.putLine("else");
         ft++;
         ListCommands(ft, cmd.cmds_false, tokens);
         ft--;
      }
      if(cmd.type==CMD_DO)ft.putLine(S+"while("+TextTokens(tokens, cmd.cond_range.x, cmd.cond_range.y)+");");
   }
}
/******************************************************************************
static void ListFunction(FileText &ft, Symbol &func, Memc<Command> &cmds)
{
   if(func.source)
   {
      ft.putLine(SEP_LINE);
      ft.putLine(func.definition());
      ft++;
      ListCommands(ft, cmds, func.source->tokens);
      ft--;
   }
}
/******************************************************************************/
// READ COMMANDS
/******************************************************************************/
static void ReadInstruction(Source &source, Int &token_index, Symbol &set_parent, Memc<Command> &cmds, Memc<Message> &msgs, CMD_TYPE type) // read until ; encountered (eat the ';'), if { or } then don't set its parents set error and return
{
   Memc<Token*> &tokens=source.tokens;
   for(Int start=token_index; token_index<tokens.elms(); )
   {
      Token &token=*tokens[token_index];
      if((token.type==TOKEN_OPERATOR && (token=='{' || token=='}'))
      || (token.type==TOKEN_KEYWORD  && (token=="if" || token=="for" || token=="while" || token=="do" || token=="return" || token=="break" || token=="continue" || token=="goto" || token=="else"))){msgs.New().error(S+"Unexpected '"+token+"'", &token); return;}
      token.parent=&set_parent; token_index++;
      if(token==';')
      {
         Int end=token_index-2;
         if((type==CMD_INSTRUCT) ? end>=start : true) // don't process empty instruction ";" when processing CMD_INSTRUCT
         {
            Command &cmd=cmds.New();
            cmd.type=type;
            cmd.raw_range.set(start, end);
         }
         return;
      }
   }
}
/******************************************************************************/
static Bool ReadCommand(Source &source, Int &token_index, Symbol &set_parent, Memc<Command> &cmds, Memc<Message> &msgs)
{
   Memc<Token*> &tokens=source.tokens;
   if(InRange(token_index, tokens))
   {
      Token &token=*tokens[token_index++]; token.parent=&set_parent;
      if(token.type==TOKEN_KEYWORD)
      {
         if(token=="if") // spaces will be: "if(cond_space)true_space;else false_space;", 'true_space' and 'false_space' are child of 'cond_space'
         {
            Command &cmd=cmds.New();
            cmd.type=CMD_IF;
            if(InRange(token_index, tokens)) // check for "(condition)" after 'if'
            {
               Token &token=*tokens[token_index];
               if(token!='(')msgs.New().error(S+"'if' should be followed by '(' : \"if(..)\", while encountered '"+token+"'", &token);else
               {
                  token.parent=&set_parent; token_index++; // 'token_index' now points after '('
                  Symbol *cond_space=source.createSpace(set_parent, token_index);
                  cmd._for.cond_range.x=token_index;
                  Bool valid=false;
                  for(Int round_level=0, bracket_level=0, semicolons=0; token_index<tokens.elms(); )
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')if(!bracket_level--){msgs.New().error(S+"Unexpected '}' encountered after \"if(..\"", &token); break;}
                     if(token=='{')    bracket_level++;
                     if(token=='(')      round_level++;
                     if(token==')')if(!  round_level--){if(semicolons==0){valid=true; cmd._for.cond_range.y=token_index-1;}else msgs.New().error("Unexpected ';' inside \"if(..)\"", &token); token.parent=&set_parent; token_index++; break;}
                     if(token==';')       semicolons++;
                     token.parent=cond_space; token_index++;
                  }
                  if(valid && InRange(token_index, tokens)) // check for commands after "if(..)"
                  {
                     Token  &token     =*tokens[token_index];
                     Symbol *true_space=source.createSpace(*cond_space, token_index);
                     if(token=='}')msgs.New().error("No commands after \"if(..)\"", &token);else
                     if(token=='{'){token.parent=true_space; token_index++; ReadCommands(source, token_index, *true_space, cmd.cmds, msgs);}else
                                                                            ReadCommand (source, token_index, *true_space, cmd.cmds, msgs);

                     if(InRange(token_index, tokens)) // check for 'else'
                     {
                        Token &token=*tokens[token_index];
                        if(token=="else")
                        {
                           token.parent=&set_parent; token_index++;
                           if(InRange(token_index, tokens))
                           {
                              Token  &token      =*tokens[token_index];
                              Symbol *false_space=source.createSpace(*cond_space, token_index);
                              if(token=='}')msgs.New().error("No commands after \"if(..).. else\"", &token);else
                              if(token=='{'){token.parent=false_space; token_index++; ReadCommands(source, token_index, *false_space, cmd.cmds_false, msgs);}else
                                                                                      ReadCommand (source, token_index, *false_space, cmd.cmds_false, msgs);
                           }
                        }
                     }
                  }
               }
            }
            return true;
         }else
         if(token=="for") // spaces will be: "for(space; space; space_step)space;", 'space_step' is a child of 'space'
         {
            Command &cmd=cmds.New();
            cmd.type=CMD_FOR;
            if(InRange(token_index, tokens)) // check for "(init; condition; step)" after 'for'
            {
               Token &token=*tokens[token_index];
               if(token!='(')msgs.New().error(S+"'for' should be followed by '(' : \"for(..)\", while encountered '"+token+"'", &token);else
               {
                  token.parent=&set_parent; token_index++; // 'token_index' now points after '('
                  Symbol *space=source.createSpace(set_parent, token_index), *set_space=space;
                  cmd._for.init_range.x=token_index;
                  Bool valid=false;
                  for(Int round_level=0, bracket_level=0, semicolons=0; token_index<tokens.elms(); )
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')if(!bracket_level--){msgs.New().error(S+"Unexpected '}' encountered after \"for(..\"", &token); break;}
                     if(token=='{')    bracket_level++;
                     if(token=='(')      round_level++;
                     if(token==')')if(!  round_level--){if(semicolons==2){valid=true; cmd._for.step_range.y=token_index-1;}else msgs.New().error("'for' should be of following format: \"for(initialize; condition; step)\"", &token); token.parent=&set_parent; token_index++; break;}
                     if(token==';')
                     {
                        if(semicolons==0){cmd._for.init_range.y=token_index-1; cmd._for.cond_range.x=token_index+1;}else
                        if(semicolons==1){cmd._for.cond_range.y=token_index-1; cmd._for.step_range.x=token_index+1; set_space=source.createSpace(*set_space, token_index);} // create new space for the 'step'
                        semicolons++;
                     }
                     token.parent=set_space; token_index++;
                  }
                  if(valid && InRange(token_index, tokens))
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')msgs.New().error("No commands after \"for(..)\"", &token);else
                     if(token=='{'){token.parent=space; token_index++; ReadCommands(source, token_index, *space, cmd.cmds, msgs);}else
                                                                       ReadCommand (source, token_index, *space, cmd.cmds, msgs);
                  }
               }
            }
            return true;
         }else
         if(token=="while") // spaces will be: "while(space)space;"
         {
            Command &cmd=cmds.New();
            cmd.type=CMD_WHILE;
            if(InRange(token_index, tokens)) // check for "(condition)" after 'while'
            {
               Token &token=*tokens[token_index];
               if(token!='(')msgs.New().error(S+"'while' should be followed by '(' : \"while(..)\", while encountered '"+token+"'", &token);else
               {
                  token.parent=&set_parent; token_index++; // 'token_index' now points after '('
                  Symbol *cond_space=source.createSpace(set_parent, token_index);
                  cmd._for.cond_range.x=token_index;
                  Bool valid=false;
                  for(Int round_level=0, bracket_level=0, semicolons=0; token_index<tokens.elms(); )
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')if(!bracket_level--){msgs.New().error(S+"Unexpected '}' encountered after \"while(..\"", &token); break;}
                     if(token=='{')    bracket_level++;
                     if(token=='(')      round_level++;
                     if(token==')')if(!  round_level--){if(semicolons==0){valid=true; cmd._for.cond_range.y=token_index-1;}else msgs.New().error("Unexpected ';' inside \"while(..)\"", &token); token.parent=&set_parent; token_index++; break;}
                     if(token==';')       semicolons++;
                     token.parent=cond_space; token_index++;
                  }
                  if(valid && InRange(token_index, tokens))
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')msgs.New().error("No commands after \"while(..)\"", &token);else
                     if(token=='{'){token.parent=cond_space; token_index++; ReadCommands(source, token_index, *cond_space, cmd.cmds, msgs);}else
                                                                            ReadCommand (source, token_index, *cond_space, cmd.cmds, msgs);
                  }
               }
            }
            return true;
         }else
         if(token=="do") // spaces will be: "do cmds_space; while(cond_space);", 'cond_space' is NOT a child of 'cmds_space'
         {
            Command &cmd=cmds.New();
            cmd.type=CMD_DO;
            if(InRange(token_index, tokens)) // check for commands after 'do'
            {
               Token  &token     =*tokens[token_index];
               Symbol *cmds_space=source.createSpace(set_parent, token_index);
               if(token=='}')msgs.New().error("No commands after 'do'", &token);else
               if(token=='{'){token.parent=cmds_space; token_index++; ReadCommands(source, token_index, *cmds_space, cmd.cmds, msgs);}else
                                                                      ReadCommand (source, token_index, *cmds_space, cmd.cmds, msgs);

               if(InRange(token_index, tokens)) // check for 'while'
               {
                  Token &token=*tokens[token_index];
                  if(token!="while")msgs.New().error(S+"\"do ..\" should be followed by 'while' : \"do .. while\", while encountered '"+token+"'", &token);else
                  {
                     token.parent=&set_parent; token_index++;
                     if(InRange(token_index, tokens)) // check for "(condition)" after 'while'
                     {
                        Token &token=*tokens[token_index];
                        if(token!='(')msgs.New().error(S+"'while' should be followed by '(' : \"while(..)\", while encountered '"+token+"'", &token);else
                        {
                           token.parent=&set_parent; token_index++; // 'token_index' now points after '('
                           Symbol *cond_space=source.createSpace(set_parent, token_index);
                           cmd._for.cond_range.x=token_index;
                           Bool valid=false;
                           for(Int round_level=0, bracket_level=0, semicolons=0; token_index<tokens.elms(); )
                           {
                              Token &token=*tokens[token_index];
                              if(token=='}')if(!bracket_level--){msgs.New().error(S+"Unexpected '}' encountered after \"while(..\"", &token); break;}
                              if(token=='{')    bracket_level++;
                              if(token=='(')      round_level++;
                              if(token==')')if(!  round_level--){if(semicolons==0){valid=true; cmd._for.cond_range.y=token_index-1;}else msgs.New().error("Unexpected ';' inside \"while(..)\"", &token); token.parent=&set_parent; token_index++; break;}
                              if(token==';')       semicolons++;
                              token.parent=cond_space; token_index++;
                           }
                           if(valid && InRange(token_index, tokens)) // check for ';' after "while(..)"
                           {
                              Token &token=*tokens[token_index];
                              if(token!=';')msgs.New().error(S+"\"do .. while(..)\" should be followed by ';', while encountered '"+token+"'", &token);
                              else         {token.parent=&set_parent; token_index++;}
                           }
                        }
                     }
                  }
               }
            }
            return true;
         }else
         if(token=="switch") // spaces will be: "switch(space)space;"
         {
            Command &cmd=cmds.New();
            cmd.type=CMD_SWITCH;
            if(InRange(token_index, tokens)) // check for "(expression)" after 'switch'
            {
               Token &token=*tokens[token_index];
               if(token!='(')msgs.New().error(S+"'switch' should be followed by '(' : \"switch(..)\", while encountered '"+token+"'", &token);else
               {
                  token.parent=&set_parent; token_index++; // 'token_index' now points after '('
                  Symbol *switch_space=source.createSpace(set_parent, token_index);
                  cmd.raw_range.x=token_index;
                  Bool valid=false;
                  for(Int round_level=0, bracket_level=0, semicolons=0; token_index<tokens.elms(); )
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')if(!bracket_level--){msgs.New().error(S+"Unexpected '}' encountered after \"switch(..\"", &token); break;}
                     if(token=='{')    bracket_level++;
                     if(token=='(')      round_level++;
                     if(token==')')if(!  round_level--){if(semicolons==0){valid=true; cmd.raw_range.y=token_index-1;}else msgs.New().error("Unexpected ';' inside \"switch(..)\"", &token); token.parent=&set_parent; token_index++; break;}
                     if(token==';')       semicolons++;
                     token.parent=switch_space; token_index++;
                  }
                  if(valid && InRange(token_index, tokens))
                  {
                     Token &token=*tokens[token_index];
                     if(token=='}')msgs.New().error("No commands after \"switch(..)\"", &token);else
                     if(token=='{'){token.parent=switch_space; token_index++; ReadCommands(source, token_index, *switch_space, cmd.cmds, msgs);}else
                                                                              ReadCommand (source, token_index, *switch_space, cmd.cmds, msgs);
                  }
               }
            }
            return true;
         }else
         if(token=="return")
         {
            ReadInstruction(source, token_index, set_parent, cmds, msgs, CMD_RETURN);
            return true;
         }else
         if(token=="break")
         {
            ReadInstruction(source, token_index, set_parent, cmds, msgs, CMD_BREAK);
            return true;
         }else
         if(token=="continue")
         {
            ReadInstruction(source, token_index, set_parent, cmds, msgs, CMD_CONTINUE);
            return true;
         }else
         if(token=="goto")
         {
            ReadInstruction(source, token_index, set_parent, cmds, msgs, CMD_GOTO);
            return true;
         }else
         if(token=="else")
         {
            msgs.New().error(S+"Unexpected 'else'", &token);
            return true;
         }
      }else
      if(token=='{')
      {
         Command &cmd=cmds.New();
         cmd.type=CMD_GROUP;
         cmd.raw_range.x=token_index; // skip '{', at this moment 'token_index' is after '{'
         token.parent=source.createSpace(set_parent, token_index-1); // set '{' parent to point to newly created sub space
         ReadCommands(source, token_index, *token.parent, cmd.cmds, msgs);
         cmd.raw_range.y=token_index-2; // skip '}', at this moment 'token_index' is after '}', so -2 is used to go back to '}' and later before '}'
         return true;
      }else
      if(token=='}')
      {
         return false; // false to stop processing this level
      }else
      if(token.type==TOKEN_CODE && InRange(token_index, tokens) && *tokens[token_index]==':') // X: label
      {
         Symbol *func=set_parent.func();
         token.symbol=source.symbols.New().require((func ? func->full_name+SEP : S)+'@'+token).set(func, Symbol::LABEL, token_index-1, &source);
         if(token.symbol->valid>1)msgs.New().error("Label redefinition", &source, token_index-1); // name used more than once
         else                     func->children.add(token.symbol()); // add to children list
         Command &cmd=cmds.New();
         cmd.type=CMD_LABEL;
         cmd.raw_range.set(token_index-1, token_index-1); // label:
         tokens[token_index++]->parent=&set_parent;
         return true;
      }
      ReadInstruction(source, --token_index, set_parent, cmds, msgs, CMD_INSTRUCT);
      return true;
   }
   return false;
}
/******************************************************************************/
void ReadCommands(Source &source, Int &token_index, Symbol &set_parent, Memc<Command> &cmds, Memc<Message> &msgs)
{
   for(; ReadCommand(source, token_index, set_parent, cmds, msgs); );
}
/******************************************************************************/
Int Command::startTokenIndex()
{
   switch(type)
   {
      case CMD_INSTRUCT   : return  raw_range.x;
      case CMD_GROUP      : return  raw_range.x-1;
      case CMD_IF         : return _for.cond_range.x-2; // 'if'    , '('
      case CMD_FOR        : return _for.init_range.x-2; // 'for'   , '('
      case CMD_WHILE      : return _for.cond_range.x-2; // 'while' , '('
      case CMD_DO         : return cmds.elms() ? cmds[0].startTokenIndex()-1 : _for.cond_range.x-3; // x ? 'do' : 'do', 'while', '('
      case CMD_SWITCH     : return       raw_range.x-2; // 'switch', '('
      case CMD_RETURN     : return       raw_range.x-1; // 'return'
      case CMD_BREAK      : return       raw_range.x-1; // 'break'
      case CMD_CONTINUE   : return       raw_range.x-1; // 'continue'
      case CMD_GOTO       : return       raw_range.x-1; // 'goto'
      case CMD_GOTO_COND  : return _for.cond_range.x-2; // 'if'    , '('
      case CMD_GOTO_COND_N: return _for.cond_range.x-2; // 'if'    , '('
      case CMD_LABEL      : return       raw_range.x;
   }
   return -1;
}
/******************************************************************************/
}}
/******************************************************************************/
