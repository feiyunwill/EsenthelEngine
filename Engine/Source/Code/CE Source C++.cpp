/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
#define WRITE_SEPARATORS 0
/******************************************************************************
void write(CodeLine &cline, C VecI2 &start, C VecI2 &end);
void write(CodeLine &cline,   Int    start,   Int    end);

void Source::write(CodeLine &cline, C VecI2 &start, C VecI2 &end)
{
   for(Int y=start.y; y<=end.y; y++)if(InRange(y, lines))
   {
      Line      &l=lines[y]; cline.includeLine(y);
      Int  start_x=((y==start.y) ? start.x :            0),
             end_x=((y==  end.y) ?   end.x : l.length()-1);
      for(Int x=start_x; x<=end_x; x++)if(l.Type(x)!=TOKEN_COMMENT)cline.cols.New().set(l[x], x, y, -1, l.Type(x));
   }
}
void Source::write(CodeLine &cline, Int start, Int end)
{
   MAX(start,               0);
   MIN(end  , tokens.elms()-1);
   if(InRange(start, tokens)
   && InRange(end  , tokens))
   {
      for(Int i=start; start<=end; i++)
      {
         Token  &token=*tokens   [i];
         CChar8 *dot  = adjustDot(i);
         if(dot || i==end)
         {
            Int   to=(dot ? i-1 : i);
            VecI2 s, e; srcTokenRange(start, to, s, e); write(cline, s, e);
            if(dot)cline.append(dot, TOKEN_OPERATOR, i);
            start=i+1;
         }
      }
   }
}
/******************************************************************************/
void Source::srcTokenRange(Int t_start, Int t_end, VecI2 &start, VecI2 &end)
{
   Token //*p=(InRange(t_start-1, tokens) ? tokens[t_start-1] : null),
         &s=*tokens[t_start],
         &e=*tokens[t_end  ],
         *n=(InRange(t_end+1, tokens) ? tokens[t_end+1] : null);

   if(s.macro_col>=0)start.set(s.macro_col, s.macro_line->line);
   else              start.set(s.col      , s.lineIndex()     );

   if(n)end.set(n->col-1          , n->lineIndex()); // process from s to n-1
   else end.set(e.line->length()-1, e. lineIndex()); // process from s to end of line

   for(; lineValid(end); dec(end))
   {
      TOKEN_TYPE t=lines[end.y].Type(end.x);
      if(t!=TOKEN_NONE && t!=TOKEN_COMMENT)break; // don't include spaces or comments after the token
   }
}
Bool Source::destTokenRange(Memc<CodeLine> &clines, Int t_start, Int t_end, VecI2 &start, VecI2 &end)
{
   if(InRange(t_start, tokens)
   && InRange(t_end  , tokens))
   {
      // find by token indexes
      if(FindLineCol(clines, t_start, tokens[t_start]->lineIndex(), true , start))
      if(FindLineCol(clines, t_end  , tokens[t_end  ]->lineIndex(), false, end  ))
         return true;

      // find by source text positions
      srcTokenRange(t_start, t_end, start, end); // convert tokens to positions (this already handles macro issues)
      if(FindLineCol(clines, start, start))
      if(FindLineCol(clines, end  , end  ))
         return true;
   }
   return false;
}
Str Source::getText(Int start, Int end)
{
   Str out;
   MAX(start,               0);
   MIN(end  , tokens.elms()-1);
   if(InRange(start, tokens)
   && InRange(end  , tokens))
   {
      VecI2 s, e; srcTokenRange(start, end, s, e);
      write(out, s, e);
   }
   return out;
}
void Source::writeAll(Str &out, C VecI2 &start, C VecI2 &end)
{
   Int start_y=Max(0, start.y), end_y=Min(end.y, lines.elms()-1);
   for(Int y=start_y; y<=end_y; y++)
   {
      if(y!=start_y)out+='\n';
    C Line &line=lines[y];
      for(Int max_x=((y==end  .y) ? Min(end  .x, line.length()) : line.length()),
                  x=((y==start.y) ? Max(start.x,             0) : 0            ); x<max_x; x++)out+=line[x];
   }
}
void Source::write(Str &out, C VecI2 &start, C VecI2 &end)
{
   for(Int y=start.y; y<=end.y; y++)if(InRange(y, lines))
   {
    C Line      &l=lines[y];
      Int  start_x=((y==start.y) ? start.x :            0),
             end_x=((y==  end.y) ?   end.x : l.length()-1);
      for(Int x=start_x; x<=end_x; x++)if(l.Type(x)!=TOKEN_COMMENT)
      {
         Char c=l[x];
         if(!(c==' ' && (out.last()==' ' || !out.is())))out+=c;
      }
   }
   if(out.last()==' ')out.removeLast();
}
// this method appends text starting from line_i inclusive
void Source::write(Memc<CodeLine> &clines, C VecI2 &start, C VecI2 &end, VecI2 *clines_start, Int *line_i) // 'clines_start' position in 'clines' where first text was written
{
   if(clines_start)*clines_start=-1; // set invalid at start in case no text is written
   Int  li=(line_i ? *line_i : clines.elms());
   Bool wrote=false;
   for(Int y=start.y; y<=end.y; y++)if(InRange(y, lines))
   {
    C Line &l=lines[y];
      if(wrote)clines.NewAt(++li);else // if we're writing secondary line, then create new line
      {
         wrote=true;
         if(!InRange(li, clines))
         {
            clines.NewAt(li);
            Clamp(li, 0, clines.elms()-1);
         }
      }
      CodeLine &cl=clines[li].includeLine(y);
      Int  start_x=((y==start.y) ? start.x :            0),
             end_x=((y==  end.y) ?   end.x : l.length()-1);
      if(y==start.y)
      {
         if(!cl.cols.elms())FREP(start_x)if(l[i]==' ')cl.append(' ', TOKEN_NONE);else break;
         if(clines_start)clines_start->set(cl.cols.elms(), li);
      }
      for(Int x=start_x; x<=end_x; x++)cl.cols.New().set(l[x], x, y, -1, l.Type(x));
   }
   if(line_i)*line_i=li;
}
void Source::write(Memc<CodeLine> &clines, Int start, Int end, VecI2 *clines_start, Int *line_i)
{
   MAX(start,               0);
   MIN(end  , tokens.elms()-1);
   if(InRange(start, tokens)
   && InRange(end  , tokens))
   {
      VecI2 s, e; srcTokenRange(start, end, s, e);
      write(clines, s, e, clines_start, line_i);
   }else
   {
      if(clines_start)*clines_start=-1;
   }
}
void Source::writeTokens(CodeLine &cline, Int start, Int end, Bool gcc)
{
   MAX(start,               0);
   MIN(end  , tokens.elms()-1);
   if(InRange(start, tokens)
   && InRange(end  , tokens))
   {
      Str        temp;
      TOKEN_TYPE last_type=TOKEN_NONE;
      for(Int i=start; i<=end; i++)
      {
         Token &token=*tokens[i];
         Int    line = token.lineIndex();
         cline.includeLine(line);
         if(MustSeparate(last_type, token.type) || gcc && (token==TMPL_B || token==TMPL_E))cline.append(' ', TOKEN_NONE); // GCC does not handle << >> double templates
         last_type=token.type;
         token.asText(temp);
         FREPAD(t, temp)cline.cols.New().set(temp[t], token.col, line, i, token.type);
      }
   }
}
void Source::writeSymbolDecl(Memc<CodeLine> &clines, Symbol &symbol, Bool gcc)
{
   Bool   add_extern=(symbol.isVar() && !(symbol.modifiers&Symbol::MODIF_EXTERN));
   Int    start     =getSymbolStart(symbol.token_index),
          end       =getListEnd    (symbol.token_index);
   Token *token     =symbol.getToken();

   // write full variable list until first ';' encountered
   if(symbol.fromPartialMacro()) // if this symbol was created from a partial macro, then we need to expand it fully (because macro can declare/define multiple variables, like DEFINE_CACHE, and when copying just DEFINE_CACHE into the header, we cannot insert the "extern" keyword inside it)
   {
      CodeLine &cl=clines.New();
      if(add_extern)cl.append("extern", TOKEN_KEYWORD).append(' ', TOKEN_NONE); // prepend by "extern"
      writeTokens(cl, start, end, gcc);
   }else
   {
      VecI2 var_start;
      write(clines, start, end, &var_start);
      if(add_extern && InRange(var_start.y, clines))clines[var_start.y].insert(var_start.x, ' ', TOKEN_NONE).insert(var_start.x, "extern", TOKEN_KEYWORD); // prepend by "extern"
   }

   // adjust tokens
   for(Int i=start; i<=end; i++)adjustToken(clines, i, gcc);
}
void Source::remove(Memc<CodeLine> &clines, Int start, Int end, Bool definite)
{
   VecI2 s, e; if(destTokenRange(clines, start, end, s, e))Remove(clines, s, e, definite);
}
void Source::removeDefVal(Memc<CodeLine> &clines, Symbol &symbol)
{
   if(symbol.isVar()) // only variables
      if(symbol.modifiers&Symbol::MODIF_DEF_VALUE)
   {
      Int start=symbol.def_val_range.x,
          end  =symbol.def_val_range.y;
      if(InRange(start-1, tokens) && *tokens[start-1]=='=') // check if it's "=0" or "(0)" default value
      {
         start--;
         /*// we may have added 'f' suffix for float numbers "0.0 -> 0.0f" we need to catch it manually because 'remove' won't detect it (This was moved into 'Parse' because of issues with tokens generated from macros)
         if(InRange(end, tokens))
         {
            Token &token=*tokens[end];
            if(token.type==TOKEN_NUMBER)
            {
               Int col; if(CodeLine *cl=FindLineCol(clines, token.pos()+VecI2(token.length()-1, 0), col)) // get last character of token
                  if((*cl)[col+1]=='f')cl->remove(col+1);
            }
         }*/
      }else{start--; end++;}
      remove(clines, start, end, true);
   }
}

Int Source::getSymbolStart(Int i) {return GetSymbolStart(tokens, i);}
Int Source::getListEnd    (Int i) // get next ';' marking end of var declaration
{
   if(!InRange(i, tokens))return -1;
   for(; i<tokens.elms(); )if(*tokens[i++]==';')break;
   return i-1;
}
Int Source::getBodyStart(Int i)
{
   if(!InRange(i, tokens))return -1;
   for(; i<tokens.elms(); i++)
   {
      Token &c=*tokens[i];
      if(c=='{' || c==';')break;
   }
   return i;
}
Int Source::getBodyEnd(Int i)
{
   if(!InRange(i, tokens))return -1;
   for(Int level=0; i<tokens.elms(); i++)
   {
      Token &c=*tokens[i];
      if(c=='{')   ++level;else
      if(c=='}')if(--level<=0)break;
   }
   if(InRange(i+1, tokens) && (*tokens[i+1])==';')i++;
   MIN(i, tokens.elms()-1);
   return i;
}
Bool Source::isFirstVar(Symbol &symbol)
{
   if(!InRange(symbol.token_index, tokens))return false;
   for(Int   i=symbol.token_index-1; i>=0; i--)
   {
      Token &c=*tokens[i];
      if(c=='(' || c=='*' || c=='&' || c=="const")continue; // ignore "int *const&(x)"
      if(c==',')return false;
      break;
   }
   return true;
}
CChar8* Source::adjustDot(Int i)
{
   if(InRange(i, tokens))
   {
      Token &token=*tokens[i];
      if(token=='.')
      {
         Expr expr; Bool result=evaluateSymbol(i, expr); // we DO need to calculate from 'i' (and include the '.' separator) because expressions like "(TYPE*)obj." without the '.' separator would look like "(TYPE*)obj" and always return obj already casted to some type and be a pointer, which we don't want, we want the original "obj." first
         if(!result)return "::"; // if no expression was received then it means it was an empty "." command (which points to global namespace)
         // we have received an expression, so test it
         if( expr.instance && expr.symbol.isPtr())return "->"; // it's an instance and a pointer
         if(!expr.instance && expr.symbol        )return "::"; // we have received symbol which is not an instance (it's a namespace or class)
         if( expr=="super" || expr=="__super"    )return "::"; // super must be followed by "::" in C++
         // test for global namespace "5+.EE.SomeFunc()" or "(int).EE.SomeFunc()", in such cases we can receive operator '+' or ')' in both examples
         if(expr._operator && expr!=']' && expr!="++" && expr!="--" && expr!=')')return "::"; // x[]. is always '.', x<>. is always "::", x++. is always '.', x--. is always '.', x(). is usually a function call or operator() which use '.' or very frequently it's a cast "(int).EE.SomeFunc" which uses "::" (however because we've failed to fully compile, operator was received in the 'expr', we don't know for sure if it's a function call or cast, so we need to keep the '.' to allow usage of unknown symbols, so keep the original '.' and don't convert it into "::")
      }
   }
   return null;
}
void Source::adjustToken(Memc<CodeLine> &code_lines, Int i, Bool gcc)
{
   if(InRange(i, tokens))
   {
      Token &token=*tokens[i];
      if(token=='.') // replace '.' with "->" or "::"
      {
         if(CChar8 *dot=adjustDot(i))
         {
            Int col; if(CodeLine *cl=FindLineCol(code_lines, token.pos(), col))cl->remove(col).insert(col, dot, TOKEN_OPERATOR, i);
         }
      }else
      /*if(token.type==TOKEN_NUMBER) // convert 0.0f -> 0.0f, 0.0 -> 0.0f, 0.0d -> 0.0 (This was moved into 'Parse' because of issues with tokens generated from macros)
      {
         FREPA(token)if(token[i]=='.') // if floating point
         {
            Char c=token.last(); Int col;
            if(c=='F' || c=='f'){}else // do nothing (0.0f -> 0.0f)
            if(CodeLine *cl=FindLineCol(code_lines, token.lineIndex(), token.col+token.length()-1, col)) // find column of last char of token
            {
               if(c=='d' || c=='D')cl->remove(col);                      // remove double suffix (0.0d -> 0.0 )
               else                cl->insert(col+1, 'f', TOKEN_NUMBER); // add    float  suffix (0.0  -> 0.0f)
            }
            break;
         }
      }else*/
      if(gcc)
      {
         if(token==TMPL_E) // replace "Memc<Memc<TYPE>>" with "Memc<Memc<TYPE> >"
         {
            if(InRange(i+1, tokens))
            {
               Token &next=*tokens[i+1];
               if(next==TMPL_E && token.col+1==next.col && token.line==next.line)
               {
                  Int col; if(CodeLine *cl=FindLineCol(code_lines, token.pos(), col))cl->insert(col+1, ' ', TOKEN_NONE);
               }
            }
         }else
         if(token==TMPL_B) // replace "Memc<::Class>" with "Memc< ::Class>" (this fails on Xcode 5 Apple LLVM compiler)
         {
            if(InRange(i+1, tokens))
            {
               Token &next=*tokens[i+1];
               if((next=='.' || next=="::") && token.col+1==next.col && token.line==next.line)
               {
                  Int col; if(CodeLine *cl=FindLineCol(code_lines, token.pos(), col))cl->insert(col+1, ' ', TOKEN_NONE);
               }
            }
         }else
         if(token.type==TOKEN_KEYWORD && (token=="super" || token=="__super")) // replace 'super' with 'ClassName'
         {
            if(Symbol *symbol=finalSymbol(i+2)) // get 'symbol' from "super.symbol"
               if(Symbol *Class=symbol->Class())
            {
               Int col; if(CodeLine *cl=FindLineCol(code_lines, token.pos(), col))
               {
                  remove(code_lines, i, i+1, true); // remove "super."
                  Memc<Symbol::Modif> templates; if(token.parent)if(Symbol *Class=token.parent->Class())AddBaseTemplates(templates, *Class);
                  writeClassPath(*cl, col, Class, null, true, &templates); // full class path must be written, because if writing only base name of the parent, then "class Obj : Game:Obj {void func() {super.func();}}" would translate into "Obj::func()" because both this and parent classes have the same name, error would occur
               }
            }
         }else
         if(token.type==TOKEN_KEYWORD && token=="friend") // replace "friend X;" with "friend struct X;"
         {
            if(InRange(i+1, tokens))
               if(tokens[i+1]->type!=TOKEN_KEYWORD) // quick check for skipping already correct "friend struct"
                  for(Int j=i+1; j<tokens.elms(); j++)
            {
               Token &token=*tokens[j];
               if(token.type==TOKEN_OPERATOR)
               {
                  if(token=='(' || token==')')break; // this is a function -> don't add keyword
                  if(token==';' || token=='{' || token=='}') // encountered end of friend declaration
                  {
                     Token &token=*tokens[i+1];
                     Bool   is_class=false; // set false because 'class' is macro for 'struct' anyway in Esenthel Script
                     //j=i+1; Str temp; Memc<Symbol::Modif> templates; Symbol *symbol=GetFullSymbol(tokens, j, temp, , templates);
                     Int col; if(CodeLine *cl=FindLineCol(code_lines, token.pos(), col))cl->insert(col, ' ', TOKEN_NONE).insert(col, is_class ? "class" : "struct", TOKEN_KEYWORD);
                     break;
                  }
               }
            }
         }
      }
   }
}
void Source::writeClassPath(CodeLine &line, Int col, Symbol *parent, Symbol *cur_namespace, Bool global, Memc<Symbol::Modif> *templates, bool start_separator)
{
   for(Symbol *cur=parent; cur && cur!=cur_namespace && (cur->type==Symbol::CLASS || cur->type==Symbol::NAMESPACE); cur=cur->Parent())
   {
      if(start_separator)line.insert(col, "::", TOKEN_OPERATOR);else start_separator=true;
      if(cur->templates.elms()) // write all templates of this symbol
      {
         line.insert(col, '>', TOKEN_OPERATOR);
         REPA(cur->templates)
         {
            if(i!=cur->templates.elms()-1)line.insert(col, ',', TOKEN_OPERATOR);
            Symbol *tmplt=cur->templates[i]();
            if(templates)REPA(*templates)if(Symbol::Modif &t=(*templates)[i])if(t.src_template==tmplt) // find known template target
            {
               writeClassPath(line, col, t(), null, true, &t.templates, false); // write itself, its templates, and its class path
               goto next; // we've already written it so don't write it manually below
            }
            if(tmplt)line.insert(col, *tmplt, TOKEN_CODE);
         next:;
         }
         line.insert(col, '<', TOKEN_OPERATOR);
      }
      line.insert(col, *cur, TOKEN_CODE);
   }
   if(global)line.insert(col, "::", TOKEN_OPERATOR); // start from global namespace
}
void Source::expandName(CodeLine &line, Int col, Symbol &symbol, Symbol *parent, Symbol *cur_namespace)
{
   line.insert(col, symbol.cppName(), TOKEN_CODE); // name
   writeClassPath(line, col, parent, cur_namespace, false); // X<A,B>::Y::name
}
void Source::expandName(Memc<CodeLine> &code_lines, Symbol &symbol, Symbol *cur_namespace)
{
   if(InRange(symbol.token_index, tokens))
   {
      Token &c=*tokens[symbol.token_index];
      if(c.type==TOKEN_KEYWORD && c=="operator")
      {
         VecI2 pos; if(FindLineCol(code_lines, c.pos(), pos))
         {
            writeClassPath(code_lines[pos.y], pos.x, symbol.Parent(), cur_namespace, false);
         }
      }else
      for(Int i=symbol.token_index; i<tokens.elms(); i++) // get symbol name ending
      {
         Token &e=*tokens[i];
         if(e=='(' || e==')' || e==',' || e=='=' || e==';' || e=='[') // "void func|(){}", "void (func)|(){}", "int x|,", "int x|(0)", "int x|=", "int x|;", "int x|[]=", "operator=", "operator()", ..
         {
            VecI2 start, end;
            if(FindLineCol(code_lines, c.pos(), start))
            if(FindLineCol(code_lines, e.pos(),   end))
            {
               Remove(code_lines, start, end-VecI2(1, 0), true);
               expandName(code_lines[start.y], start.x, symbol, symbol.Parent(), cur_namespace);
            }
            break;
         }
      }
   }
}
void Source::expandRetVal(Memc<CodeLine> &code_lines, Symbol &symbol)
{
   // if return value is a symbol (class/enum) defined inside another class, then we need to write "class path" to it
   if(symbol.value && symbol.value->type!=Symbol::TYPENAME) // TYPE func() does not need to be written
      if(Symbol *value_parent=symbol.value->Parent()) // if value type has parent
         if(value_parent->type==Symbol::CLASS) // if that parent is a class
            for(Int i=symbol.type_range.x; i<tokens.elms(); i++) // start parsing the "|TYPE method()" func definition to get the first symbol in the path to value
   {
      Token &t=*tokens[i]; Symbol *middle=t.symbol();
      if(!middle)break; // "::" operators can be ignored because they use full path already "::TYPE method()"
      if(!(middle->modifiers&Symbol::MODIF_CPP_MODIFIER)) // encountered 'const', 'virtual', .. (skip it and continue parsing)
      {
         if(Symbol *middle_parent=middle->Parent()) // if middle has parent
         {
            if(middle_parent->type==Symbol::CLASS) // if parent is a class then we need to write path from namespace to middle->parent
            {
               VecI2 pos; if(FindLineCol(code_lines, t.pos(), pos))writeClassPath(code_lines[pos.y], pos.x, middle_parent, null, true);
            }
         }else
         if(middle->type==Symbol::KEYWORD && *middle=="template") // skip template<..>
         {
            for(Int level=0; ++i<tokens.elms(); )
            {
               Token &t=*tokens[i];
               if(t==TMPL_B          )   ++level;
               if(t==TMPL_E || t=='>')if(--level<=0)break;
            }
            continue;
         }
         break;
      }
   }
}

Bool Source::expandableTypename(Symbol &symbol)
{
   if(symbol.type==Symbol::TYPENAME && InRange(symbol.token_index-1, tokens) && InRange(symbol.token_index, tokens))
   {
      Token &token=*tokens[symbol.token_index-1];
      return token!="typename" && token!="class" && token!="struct";
   }
   return false;
}
Bool Source::expandableTemplate(Int token_index)
{
   if(InRange(token_index, tokens))
   {
      Token &token=*tokens[token_index];
      if(token==TMPL_B)
      {
         if(!InRange(token_index-1, tokens))return true;
         Token &prev=*tokens[token_index-1];
         if(prev=='{' || prev=='}' || prev==';' || prev=='>' || prev==TMPL_E)return true;
      }
   }
   return false;
}
void Source::expandTypename(Memc<CodeLine> &code_lines, Symbol &symbol, Int start_line)
{
   if(expandableTypename(symbol))
   {
      Token &token=*tokens[symbol.token_index]; VecI2 pos=token.pos();
      for(Int y=start_line; y<code_lines.elms(); y++) // all lines need to be checked for multiple occurences
         if(code_lines[y].hasLine(pos.y))
      {
         CodeLine &line=code_lines[y]; Int col=line.findPos(pos); if(col>=0)line.insert(col, ' ', TOKEN_NONE).insert(col, "typename", TOKEN_KEYWORD);
      }
   }
}
void Source::expandTemplate(Memc<CodeLine> &code_lines, Int token_index, Int start_line)
{
   if(expandableTemplate(token_index))
   {
      Token &token=*tokens[token_index]; VecI2 pos=token.pos();
      for(Int y=start_line; y<code_lines.elms(); y++) // all lines need to be checked for multiple occurences
         if(code_lines[y].hasLine(pos.y))
      {
         CodeLine &line=code_lines[y]; Int col=line.findPos(pos); if(col>=0)line.insert(col, "template", TOKEN_KEYWORD);
      }
   }
}
void Source::expandTemplates(Memc<CodeLine> &code_lines)
{
   REPA(symbols)expandTypename(code_lines, *symbols[i]);
   REPA(tokens )expandTemplate(code_lines,          i );
}

void Source::writeClassTemplates(Memc<CodeLine> &clines, Symbol *Class)
{
   Memc<Symbol*> template_classes; for(; Class && Class->type==Symbol::CLASS; Class=Class->Parent())if(Class->templates.elms())template_classes.add(Class);
   REPA(template_classes){Symbol *Class=template_classes[i]; write(clines, getSymbolStart(Class->token_index), Class->token_index-2);} // skip "struct name"
}
void Source::writeCtorInit(Memc<CodeLine> &clines, Symbol &ci, Int &line_i, Bool first, Bool gcc)
{
   CodeLine &line=clines[line_i];
   if(first)line.append(' ', TOKEN_NONE    ).append(':', TOKEN_OPERATOR).append(' ', TOKEN_NONE);
   else     line.append(',', TOKEN_OPERATOR).append(' ', TOKEN_NONE);

   // -1 to include ('=' or '(') in "x=5;", +1 to include (')' or ',' or ';') in "x=5;"
   // this extra space will also copy any preprocessor between the symbol, default value and the end, like this:
   // x=
   // #if 1
   //    5
   // #else
   //    0
   // #endif
   // ;
   // watch out for "void (*func)(ptr)=null;"
   write(clines, ci.token_index      , ci.token_index      , null, &line_i); // write 'symbol.name'
   write(clines, ci.def_val_range.x-1, ci.def_val_range.y+1, null, &line_i); // write default value

   if(Token *equals=getToken(ci.def_val_range.x-1)) // get '=' token
   {
      Int col; if(CodeLine *cl=FindLineCol(clines, equals->pos(), col))cl->cols[col].c='('; // replace it with '('
   }
   if(Token *end=getToken(ci.def_val_range.y+1)) // get ';' token
   {
      Int col; if(CodeLine *cl=FindLineCol(clines, end->pos(), col))cl->cols[col].c=')'; // replace it with ')'
   }

   for(Int i=ci.def_val_range.x; i<=ci.def_val_range.y; i++)adjustToken(clines, i, gcc);
}
void Source::writeCtorInits(Memc<CodeLine> &clines, Symbol &func, Int body_start, Bool gcc)
{
   if((func.modifiers&Symbol::MODIF_CTOR) && InRange(body_start-1, tokens)) // if is constructor
      if(Symbol *Class=func.Class())
         if(Class->ctor_inits.elms()) // don't try to skip if "func.ctor_inits.elms()==Class->ctor_inits.elms()" because these may be different members
   {
      Token &token=*tokens[body_start-1];
      VecI2  pos; if(FindLineCol(clines, token.pos(), pos))
      {
         pos.x++;
         CodeLine *cl=&clines[pos.y]; VecI2 start_lines=cl->lines;
         MemtN<CodeCol, 64> end; for(Int i=pos.x; i<cl->cols.elms(); i++)end.add(cl->cols[i]); cl->cols.setNum(pos.x); // remove all end of line to temporary container, because 'writeCtorInit' can operate only by appending
         Bool first=!func.ctor_inits.elms(); // if none were listed yet
         if(first) // there is a possibility that there's a ctor init for the base class "class Base{Base(Obj *){}}   class Ext : Base{Ext(Obj *obj):Base(obj){}}"
            for(Int i=body_start-1, level=0; i>=func.token_index; i--) // check all tokens from declaration start to end for existence of ':' token in the main level (to avoid something like "Ext(Obj *obj=0?null:null)" or anything else)
         {
            Token &token=*tokens[i];
            if(token=='(')level++;else
            if(token==')')level--;else
            if(token==':' && level==0){first=false; break;}
         }
         FREPA(Class->ctor_inits) // all required
         {
            SymbolPtr &ci=Class->ctor_inits[i]; if(!func.ctor_inits.has(ci)) // if not yet listed
               if(ci->source)
            {
               // we need to use the source where the class member is listed, and not this source where the function is listed
               // for example Source A: "class A{int x=0; A();}", Source B: "A::A(){}"
               // this is Source B with the function definition, however class member is listed in Source A
               ci->source->writeCtorInit(clines, *ci, pos.y, first, gcc); // write
               first=false;
            }
         }
         if(end.elms()) // if we need to insert columns
         {
            cl=&clines[pos.y]; // get current line, access it again because 'pos.y' could've changed
            if(!cl->hasAnyLines(start_lines))cl=&clines.NewAt(++pos.y).includeLines(start_lines); // create new line if needed
            FREPA(end)cl->cols.add(end[i]); // append previously removed columns
         }
      }
   }
}
void Source::writeForcedCtor(Memc<CodeLine> &clines, Symbol &Class, Symbol* &Namespace, Bool gcc)
{
   AdjustNameSymbol(clines, Namespace, Class.Namespace());
   writeClassTemplates(clines, &Class);
   {
      CodeLine &line=clines.New();
      REP(Namespace ? (Namespace->level+1)*TabLength : 0)line.append(' ', TOKEN_NONE);
      expandName(line, line.cols.elms(), Class, &Class, Namespace); // X::X
      line.append("()", TOKEN_OPERATOR); // X::X()
   } // braces so that we won't use 'line' after 'writeCtorInit' because that may change its address

   Int line_i=clines.elms()-1;
   FREPA(Class.ctor_inits)writeCtorInit(clines, *Class.ctor_inits[i], line_i, i==0, gcc); // X::X() x(..)

   clines.last().append(' ', TOKEN_NONE).append("{}", TOKEN_OPERATOR); // X::X() x(..) {}
   clines.New();
}
/******************************************************************************/
void Source::detectDefaultCtors()
{
   // detect if classes need default constructors
   FREPA(symbols){Symbol &symbol=*symbols[i]; if(symbol.type!=Symbol::FUNC)symbol.ctor_inits.clear(); FlagDisable(symbol.helper, Symbol::HELPER_FORCE_CTOR);}
   FREPA(symbols) // order important to add 'ctor_inits' as they are listed in the class
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isVar() && (symbol.modifiers&Symbol::MODIF_DEF_VALUE) && !(symbol.modifiers&Symbol::MODIF_STATIC) && symbol.Parent() && symbol.Parent()->type==Symbol::CLASS) // class non-static members with default values
      {
         Symbol &Class=*symbol.Parent();
         if(!Class.ctor_inits.elms()) // if we're adding first initializer, check if it needs forced constructor
         {
            if(Symbol *ctors=FindChild(Class, &Class, null, false)()) // check if has any ctors
               REPA(ctors->funcs)
            {
               Symbol &ctor=*ctors->funcs[i];
               if(!ctor.params.elms() || (ctor.params[0]->modifiers&Symbol::MODIF_DEF_VALUE))goto has_default_ctor;
            }
            Class.helper|=Symbol::HELPER_FORCE_CTOR; // if not found then force default constructor
         has_default_ctor:;
         }
         Class.ctor_inits.add(&symbol); // add to the list of required initializers (after checking for forced constructor)
      }
   }

   // check if we have to disable forced constructors (if some member is a reference without a default value)
   REPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isVar() && (symbol.value.modifiers&Symbol::MODIF_REF) && !(symbol.modifiers&Symbol::MODIF_DEF_VALUE) && !(symbol.modifiers&Symbol::MODIF_STATIC) && symbol.Parent()/* && symbol.Parent()->type==Symbol::CLASS check not needed because we just disable HELPER_FORCE_CTOR*/) // class non-static reference member without default value
      {
         Symbol &Class=*symbol.Parent();
         FlagDisable(Class.helper, Symbol::HELPER_FORCE_CTOR); // can't do forced constructor
      }
   }
}
/******************************************************************************/
Bool Source::writeClass(Memc<CodeLine> &clines, Symbol &symbol, Bool gcc)
{
   if(symbol.type==Symbol::CLASS && symbol.isGlobal()) // global classes only
   {
      symbol.helper|=Symbol::HELPER_PROCESSED|Symbol::HELPER_PROCESSED_FULL;
      Int start     =getSymbolStart(symbol.token_index),
          end       =getBodyEnd    (start); if(symbol.modifiers&Symbol::MODIF_TYPEDEF)end=getListEnd(end); // if class was defined with typedef "typedef class .. { } .. ;" then after '}' it is followed by typedefs until;
      if(InRange(start, tokens))
      {
         Memc<Symbol*> namespaces; for(Symbol *name=symbol.Parent(); name && name->type==Symbol::NAMESPACE; name=name->Parent())namespaces.NewAt(0)=name;

         // start namespaces
         clines.New().append(SEP_LINE, TOKEN_COMMENT);
         FREPA(namespaces)clines.New().append("namespace", TOKEN_KEYWORD).append(' ', TOKEN_NONE).append(*namespaces[i], TOKEN_CODE).append('{', TOKEN_OPERATOR);
         if(namespaces.elms())clines.New().append(SEP_LINE, TOKEN_COMMENT);

         // copy comments located before the class
         Token &start_token=*tokens[start];
         VecI2  start_com(-1), end_com(-1), pos_com(start_token.col-1, start_token.lineIndex());
         for(; lineValid(pos_com); dec(pos_com))
         {
            TOKEN_TYPE type=lines[pos_com.y].Type(pos_com.x);
            if(type==TOKEN_COMMENT)
            {
               if(end_com.x<0)end_com=pos_com;
                            start_com=pos_com;
            }else
            if(ValidType(type))break; // encountered char which is a token
         }
         if(start_com.x>=0)write(clines, start_com, end_com);

         // write class
         write(clines, start, end);

         // remove function bodies
         // insert ';' at the end of struct/class/enum declarations
         // convert '.' to "->" or "::" when needed
         for(Int i=start; i<=end; i++)
         {
            Token &c=*tokens[i]; adjustToken(clines, i, gcc);
            if(c=='}' && c.parent)if(c.parent->type==Symbol::CLASS || c.parent->type==Symbol::ENUM)
            {
               VecI2 pos; if(FindLineCol(clines, c.pos(), pos))
               {
                  CodeLine &cl=clines[pos.y];

                  // put ';'
                  if(!(c.parent->modifiers&Symbol::MODIF_TYPEDEF)) // don't write ';' if the class was defined with typedef "typedef class .. {} .. ;" because in that case the codes need to be followed by ';'
                     if(!InRange(i+1, tokens) || (*tokens[i+1])!=';') // only if it's not already there (this caused issues when the struct/class/enum was defined by a macro, like UNION)
                        cl.insert(pos.x+1, ';', TOKEN_OPERATOR); // apend '}' with ';'

                  if(c.parent->helper&Symbol::HELPER_FORCE_CTOR) // force default constructor
                  {
                     REP(pos.x-1)if(cl.cols[i].type!=TOKEN_NONE) // there is something before '}'
                     {
                        cl.insert(pos.x, "();", TOKEN_OPERATOR).insert(pos.x, *c.parent, TOKEN_CODE).insert(pos.x, ' ', TOKEN_NONE).insert(pos.x, ':', TOKEN_OPERATOR).insert(pos.x, "public", TOKEN_KEYWORD).insert(pos.x, ' ', TOKEN_NONE); // public: X();
                        goto added_ctor;
                     }
                     {
                        // TODO: align columns to level
                        CodeLine &cl=clines.NewAt(pos.y); REP(TabLength)cl.append(' ', TOKEN_NONE); cl.append(*c.parent, TOKEN_CODE).append("();", TOKEN_OPERATOR); // X();
                                     clines.NewAt(pos.y).append("public", TOKEN_KEYWORD).append(':', TOKEN_OPERATOR); // public:
                                     clines.NewAt(pos.y);
                     }
                  added_ctor:;
                  }
               }
            }
            if(c=='{' && c.parent && c.parent->type==Symbol::FUNC // function bodies
            || c==':' && c.ctor_initializer)                      // optionally started with constructor initializers
            {
               VecI2 func, from, to;
               if(InRange(i-1, tokens))
               if(FindLineCol(clines, tokens[i-1]->pos(), func))
               if(FindLineCol(clines, c           .pos(), from))
               for(Int level=0; i<=end; i++)
               {
                  Token &c=*tokens[i];
                  if(c=='{')   ++level;else
                  if(c=='}')if(--level<=0)
                  {
                     if(FindLineCol(clines, c.pos(), to))
                     {
                        Remove(clines, from, to, false); // remove function body
                        CodeLine &cl=clines[func.y];
                        for(; InRange(func.x, cl) && TokenType(cl.cols[func.x].type); func.x++); // skip last token before function body, typically ')', but can be "const final override .."
                        if (  InRange(func.x, cl) && cl.cols[func.x].c==' ' && cl.type(func.x+1)!=TOKEN_COMMENT)cl.cols[func.x].set(';', -1, -1, -1, TOKEN_OPERATOR);else cl.insert(func.x, ';', TOKEN_OPERATOR); // end func declaration with ';', replace existing ' ' but not if it's followed by a comment (to keep "); //")
                     }
                     break;
                  }
               }
            }
         }

         // close namespaces
         clines.New().append(SEP_LINE, TOKEN_COMMENT);
         FREPA(namespaces)clines.New().append('}', TOKEN_OPERATOR).append(' ', TOKEN_NONE).append(S+"// namespace "+*namespaces[i], TOKEN_COMMENT);
         if(namespaces.elms())clines.New().append(SEP_LINE, TOKEN_COMMENT);

         Symbol *Namespace=null;

         // write global typedefs for this class
      #if WRITE_SEPARATORS
         clines.New().append("// TYPEDEFS", TOKEN_COMMENT);
         clines.New().append(SEP_LINE     , TOKEN_COMMENT);
      #endif
         FREPA(Symbols)
         {
            Symbol &Typedef=Symbols.lockedData(i);

            if(!(Typedef.helper&Symbol::HELPER_PROCESSED))
               if(Typedef.type==Symbol::TYPEDEF && Typedef.source && Typedef.valid && Typedef.value && Typedef.value->valid && Typedef.isGlobal()) // process only global typedefs
                  if(Symbol *root=Typedef.value->rootClass())
                     if(root->helper&Symbol::HELPER_PROCESSED_FULL) // only if root class of target is processed
            {
               Typedef.helper|=Symbol::HELPER_PROCESSED|Symbol::HELPER_PROCESSED_FULL;
               Source &source=*Typedef.source;
               if(     source.isFirstVar(Typedef)) // process only first typedef
               {
                  // adjust namespaces to typedef namespace
                  AdjustNameSymbol(clines, Namespace, Typedef.Namespace());

                  source.writeSymbolDecl(clines, Typedef, gcc);
               }
            }
         }

         // adjust namespaces to global namespace
         AdjustNameSymbol(clines, Namespace, null);
         clines.New().append(SEP_LINE      , TOKEN_COMMENT);

         // write global variables of this class type
      #if WRITE_SEPARATORS
         clines.New().append("// VARIABLES", TOKEN_COMMENT);
         clines.New().append(SEP_LINE      , TOKEN_COMMENT);
      #endif
         FREPA(symbols)
         {
            Symbol &var=*symbols[i];
            if(var.isGlobal() && var.isVar() && isFirstVar(var) && !var.constDefineInHeader() && !(var.helper&Symbol::HELPER_PROCESSED) // process only global and first variables on the list
            && var.value && var.value->rootClass()==&symbol) // type of the value is the class being processed
            {
               var.helper|=Symbol::HELPER_PROCESSED;

               // adjust namespaces to variable namespace
               AdjustNameSymbol(clines, Namespace, var.Namespace());

               writeSymbolDecl(clines, var, gcc);
            }
         }

         // adjust namespaces to global namespace
         AdjustNameSymbol(clines, Namespace, null);
         clines.New().append(SEP_LINE      , TOKEN_COMMENT);

         // clean 'TOKEN_REMOVE'
         Clean(clines);

         // remove starting symbols
         REP(tokens[start]->col)
         {
            REPA (clines)if(clines[i].cols.elms() && ValidType(clines[i].cols[0].type))goto finished_start_spaces;
            REPAO(clines).remove(0);
         }
      finished_start_spaces:;

         // remove default variable values "x={..}" and "x(..)"
         FREPA(symbols)
         {
            Symbol &symbol=*symbols[i];
            if(symbol.Parent() && symbol.Parent()->type==Symbol::CLASS // only class vars
            || symbol.isGlobal()) // or global variables
               removeDefVal(clines, symbol);
         }

         // fix templates <TYPE> -> template<typename TYPE>
         FREPA(symbols)expandTypename(clines, *symbols[i]);
         FREPA(tokens )expandTemplate(clines,          i );

         Parse(clines);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Source::writeVarFuncs(Memc<CodeLine> &clines, Bool gcc)
{
   Bool    is=false; // if actually contains anything
   Symbol *Namespace=null;

   // write global variables forward declarations (skip default values)
   clines.New().append(SEP_LINE      , TOKEN_COMMENT);
#if WRITE_SEPARATORS
   clines.New().append("// VARIABLES", TOKEN_COMMENT);
   clines.New().append(SEP_LINE      , TOKEN_COMMENT);
#endif
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isGlobal() && symbol.isVar() && isFirstVar(symbol) && !symbol.constDefineInHeader() && !(symbol.helper&Symbol::HELPER_PROCESSED)) // process only global and first variables on the list
      {
         is=true;
         symbol.helper|=Symbol::HELPER_PROCESSED;

         // adjust namespaces to variable namespace
         AdjustNameSymbol(clines, Namespace, symbol.Namespace());

         writeSymbolDecl(clines, symbol, gcc);
      }
   }

   // remove default variable values "x={..}" and "x(..)"
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isGlobal()) // only global vars
         removeDefVal(clines, symbol);
   }

   // adjust namespaces to global namespace
   AdjustNameSymbol(clines, Namespace, null);
   clines.New().append(SEP_LINE      , TOKEN_COMMENT);
#if WRITE_SEPARATORS
   clines.New().append("// FUNCTIONS", TOKEN_COMMENT);
   clines.New().append(SEP_LINE      , TOKEN_COMMENT);
#endif

   // write global functions forward declarations (without bodies)
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isGlobalFunc() && InRange(symbol.token_index, tokens) && !(symbol.modifiers&Symbol::MODIF_FRIEND)) // friend functions should be listed only inside classes
      {
         is=true;

         // adjust namespaces to function namespace
         AdjustNameSymbol(clines, Namespace, symbol.Namespace());

         Int start=getSymbolStart(symbol.token_index)  , // get func declaration start
               end=getBodyStart  (symbol.token_index)-1; // get func declaration end
         write(clines, start, end);
         if(clines.elms())clines.last().append(';', TOKEN_OPERATOR);

         // adjust tokens
         for(Int i=start; i<=end; i++)adjustToken(clines, i, gcc);
      }
   }

   if(is)
   {
      // expand templates
      expandTemplates(clines);
      
      // adjust namespaces to global namespace
      AdjustNameSymbol(clines, Namespace, null);
      clines.New().append(SEP_LINE, TOKEN_COMMENT);

      // clean
      Clean(clines);
      Parse(clines);
   }
   return is;
}
/******************************************************************************/
Bool Source::writeFunc(Memc<CodeLine> &clines, Bool gcc, Symbol &symbol, Symbol* &Namespace)
{
   if((symbol.modifiers&Symbol::MODIF_FUNC_BODY) && !symbol.insideFunc()) // process only functions with bodies
   {
      Int start=getSymbolStart(symbol.token_index); if(InRange(start, tokens))
      {
         Int body_start=getBodyStart(symbol.token_index), // get func declaration end
                  end  =getBodyEnd  (body_start);

         // parse function to obtain info about local vars
         parseFunc(symbol);

         Bool is_inline=symbol.isInlineFunc(), write=true;

         if(!is_inline) // inline functions are written to headers, and always need to be written, as for non-inline, check that
         {
            Token &c=*tokens[start],
                  &e=*tokens[  end];

            VecI2 temp;
            if(FindLineCol(clines, c.pos(), temp)
            && FindLineCol(clines, e.pos(), temp))write=false; // if the body already exists, then don't write it
         }

         if(write)
         {
            AdjustNameSymbol(clines, Namespace, symbol.Namespace());

            // write class templates list
            if(symbol.insideClass()) // only if func body was declared inside class, and now we need to store the body outside of it (if it was already stored outside class then it has class templates listed already)
               writeClassTemplates(clines, symbol.Parent());

            // write the body
            T.write(clines, start, end);
               
            // remove function modifiers for body declarations (virtual, static, ..) before expanding names and return value (so that removing modifier along with the FullPath will not happen "virtual TYPE name" -> "virtual FullPath::TYPE name" -> "TYPE name")
            for(Int i=start; i<symbol.token_index; i++)
            {
               Token &mod=*tokens[i];
               if(mod.symbol && (mod.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER) && mod!="const")
               {
                  Token &next=*tokens[i+1];
                  VecI2  start, end;
                  if(FindLineCol(clines, mod .pos(), start))
                  if(FindLineCol(clines, next.pos(),   end))Remove(clines, start, end-VecI2(1, 0), true);
               }
            }
            // remove function modifiers for body declarations (final, override)
            for(Int i=body_start-1; i>symbol.token_index; i++)
            {
               Token &mod=*tokens[i]; if(mod.symbol && (mod.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER))
               {
                  if(mod!="const")
                  {
                     Token &next=*tokens[i+1];
                     VecI2  start, end;
                     if(FindLineCol(clines, mod .pos(), start))
                     if(FindLineCol(clines, next.pos(),   end))Remove(clines, start, end-VecI2(1, 0), true);
                  }
               }else break; // stop on func()|override
            }

            // replace name with full name
            expandName(clines, symbol, Namespace);

            // expand return value name
            expandRetVal(clines, symbol);
         }

         // remove default values in function params list
         REPA(symbol.params)removeDefVal(clines, *symbol.params[i]);

         // write ctor initializers
         writeCtorInits(clines, symbol, body_start, gcc);

         // adjust token
         for(Int i=start; i<end; i++)adjustToken(clines, i, gcc);

         return true;
      }
   }
   return false;
}
Bool Source::writeStaticVars(Memc<CodeLine> &clines, Bool gcc, Symbol* &Namespace, Bool templates)
{
   Bool is=false;
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isVar() && symbol.Parent() && symbol.Parent()->type==Symbol::CLASS && (symbol.modifiers&Symbol::MODIF_STATIC) && symbol.insideTemplateClass()==templates) // only static class vars
         if(InRange(symbol.token_index, tokens))
      {
         Bool first=true;
         for(Int i=symbol.token_index-1; i>=0; i--)
         {
            Token &c=*tokens[i];
            if(c=='(' || c=='*' || c=='&')continue; // ignore "int *&(x)"
            if(c==',')first=false;
            break;
         }
         if(first) // process only first variables on the list
         {
            is=true;

            Int start=getSymbolStart(symbol.token_index),
                end  =getListEnd    (symbol.token_index);

            // write full variable list until first ';' encountered
            Int var_start=clines.elms();
            AdjustNameSymbol(clines, Namespace, symbol.Namespace());
            writeClassTemplates(clines, symbol.Parent());
            write(clines, start, end);

            // remove var modifiers
            for(Int i=start; i<symbol.token_index; i++)
            {
               Token &mod=*tokens[i];
               if(    mod.symbol && (mod.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER) && mod=="static") // only static modifier can be removed
               {
                  Token &next=*tokens[i+1];
                  VecI2  start, end;
                  if(FindLineCol(clines, mod .pos(), start))
                  if(FindLineCol(clines, next.pos(),   end))Remove(clines, start, end-VecI2(1, 0), true);
               }
            }

            // expand value type name
            expandRetVal(clines, symbol);

            // adjust token
            for(Int i=start; i<end; i++)adjustToken(clines, i, gcc);
         }
      }
   }
   if(is)FREPA(symbols) // expand full names of written variables, do this after writing because statics could be written using lists (many at one time)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isVar() && symbol.Parent() && symbol.Parent()->type==Symbol::CLASS && (symbol.modifiers&Symbol::MODIF_STATIC) && symbol.insideTemplateClass()==templates) // only static class vars
         expandName(clines, symbol, symbol.Namespace());
   }
   return is;
}
/******************************************************************************/
Bool Source::writeInline(Memc<CodeLine> &clines, Bool gcc)
{
   Bool    is=false; // if actually contains anything
   Symbol *Namespace=null;

   clines.New().append(SEP_LINE, TOKEN_COMMENT);

   // insert static class vars
   is|=writeStaticVars(clines, gcc, Namespace, true);

   // insert class method bodies
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isInlineFunc())is|=writeFunc(clines, gcc, symbol, Namespace);
   }

   // write forced constructors
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if((symbol.helper&Symbol::HELPER_FORCE_CTOR) && symbol.isTemplateClass()){is=true; writeForcedCtor(clines, symbol, Namespace, gcc);}
   }
      
   if(is)
   {
      // expand templates
      expandTemplates(clines);

      // adjust namespaces to global namespace
      AdjustNameSymbol(clines, Namespace, null);
      clines.New().append(SEP_LINE, TOKEN_COMMENT);

      Parse(clines);
   }
   return is;
}
void Source::makeCPP(C Str &path, C Str &file, Bool gcc, Bool include_headers)
{
   Memc<CodeLine> clines;
                      clines.New().append(SEP_LINE  , TOKEN_COMMENT);
                      clines.New().append("#include", TOKEN_PREPROC).append(' ', TOKEN_NONE).append(                                                                           "\"stdafx.h\""     , TOKEN_TEXT8);
   if(include_headers)clines.New().append("#include", TOKEN_PREPROC).append(' ', TOKEN_NONE).append(S+"\""+UnixPath(GetRelativePath(GetPath(file), path+UNIQUE_NAME+UNIQUE_NAME+"headers.h"))+"\"", TOKEN_TEXT8);
                      clines.New().append(SEP_LINE  , TOKEN_COMMENT);

   if(lines.elms())write(clines, VecI2(0, 0), VecI2(lines.last().length()-1, lines.elms()-1));

   // remove stuff
   REPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.type==Symbol::CLASS && symbol.isGlobal() // all global classes
      || symbol.type==Symbol::ENUM  && symbol.isGlobal() // all global enums
      || symbol.isInlineFunc())                          // all inline functions
      {
         Int start=getSymbolStart(symbol.token_index),
             end  =getBodyEnd    (start);
         remove(clines, start, end, false);
      }else
      if(symbol.isGlobalFunc() && !symbol.templates.elms() && !(symbol.modifiers&Symbol::MODIF_FUNC_BODY) // all global function declarations
      || symbol.isVar() && ((symbol.modifiers&Symbol::MODIF_EXTERN) || symbol.isGlobal() && symbol.constDefineInHeader())) // all externs and global simple constants
      {
         Int start=getSymbolStart(symbol.token_index),
             end  =getListEnd    (start);
         remove(clines, start, end, false);
      }else
      if(symbol.type==Symbol::TYPEDEF && symbol.isGlobal()) // all global typedef lists
      {
         Int start=getSymbolStart(symbol.token_index),
             end  =getListEnd    (start);
         if(symbol.fromPartialMacro()) // if this symbol was created from a partial macro, then we have to keep it as the macro may also define other things, like 'DEFINE_CACHE' that also defines variables
         {
            // since we're keeping it then we need to adjust tokens
            for(Int i=start; i<end; i++)adjustToken(clines, i, gcc);
         }else // otherwise we can remove it as it will be stored in the headers
         {
            remove(clines, start, end, false);
         }
      }
   }

   // clean
   Clean(clines);
   clines.New().append(SEP_LINE, TOKEN_COMMENT);

   // adjust global variables
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isGlobal() && symbol.isVar() && isFirstVar(symbol)) // process only global and first variables on the list
      {
         Int start=getSymbolStart(symbol.token_index),
             end  =getListEnd    (symbol.token_index);

         // adjust token
         for(Int i=start; i<end; i++)adjustToken(clines, i, gcc);
      }
   }

   // adjust global functions
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if(symbol.isGlobalFunc() && !symbol.templates.elms() && (symbol.modifiers&Symbol::MODIF_FUNC_BODY)) // process only global functions
      {
         Int start=getSymbolStart(symbol.token_index),
             end  =getBodyEnd    (start);
         if(InRange(start, tokens))
         {
            // parse function to obtain info about local vars
            parseFunc(symbol);

            // remove default values in function params list
            REPA(symbol.params)removeDefVal(clines, *symbol.params[i]);

            // adjust token
            for(Int i=start; i<end; i++)adjustToken(clines, i, gcc);
         }
      }
   }

   clines.New().append(SEP_LINE                 , TOKEN_COMMENT);
#if WRITE_SEPARATORS
   clines.New().append("// CLASS STATIC MEMBERS", TOKEN_COMMENT);
   clines.New().append(SEP_LINE                 , TOKEN_COMMENT);
#endif
   Symbol *Namespace=null;

   // insert static class vars
   writeStaticVars(clines, gcc, Namespace, false);
   AdjustNameSymbol(clines, Namespace, null);
   clines.New().append(SEP_LINE          , TOKEN_COMMENT);
#if WRITE_SEPARATORS
   clines.New().append("// CLASS METHODS", TOKEN_COMMENT);
   clines.New().append(SEP_LINE          , TOKEN_COMMENT);
#endif

   // TODO: align start spaces to namespace level (make func for that, that removes/adds first columns)
   // insert class method bodies
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if((symbol.isFunc() && symbol.Parent() && symbol.Parent()->type==Symbol::CLASS) || (symbol.type==Symbol::FUNC && (symbol.modifiers&Symbol::MODIF_FRIEND))) // friend functions also need to be written
         if(!symbol.isInlineFunc())writeFunc(clines, gcc, symbol, Namespace);
   }

   // write forced constructors
   FREPA(symbols)
   {
      Symbol &symbol=*symbols[i];
      if((symbol.helper&Symbol::HELPER_FORCE_CTOR) && !symbol.isTemplateClass())writeForcedCtor(clines, symbol, Namespace, gcc);
   }
   AdjustNameSymbol(clines, Namespace, null);
   clines.New().append(SEP_LINE, TOKEN_COMMENT);

   // remove double separators
   REPA(clines)if(i && clines[i]==SEP_LINE && clines[i-1]==SEP_LINE)clines.remove(i, true);

   // write file
   FCreateDirs(GetPath(file));
   Parse(clines);
   if(OverwriteOnChangeLoud(clines, file))
      if(LineMap *lm=CE.build_line_maps.get(file))lm->add(loc, clines);
 //if(loc!=Str(AutoSource))FTimeUTC(file, modify_time); // don't adjust file modification time for auto header, just rely on overwrite on change, currently commented out, is there any point in this since above there's overwrite on change?
}
/******************************************************************************/
}}
/******************************************************************************/
