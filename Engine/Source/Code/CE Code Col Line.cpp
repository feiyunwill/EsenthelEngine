/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
// CODE LINE
/******************************************************************************/
Bool CodeLine::operator==(C Str &text)C
{
   if(cols.elms()!=text.length())return false;
   REPA(cols)if(!EqualCS(cols[i].c, text[i]))return false;
   return true;
}
/******************************************************************************/
Bool CodeLine::isKeyword(Int i, CChar8 *keyword)C
{
   if(type(i-1)!=TOKEN_KEYWORD && keyword)for(; ; i++)
   {
      Char8 k=*keyword++; if(!k)return type(i)!=TOKEN_KEYWORD;
      if(type(i)!=TOKEN_KEYWORD || T[i]!=k)break;
   }
   return false;
}
Bool CodeLine::starts(Int i, CChar8 *text)C
{
   if(InRange(i, cols) && text)for(;;)
   {
      Char c=*text++; if(!c)return true;
      if(cols[i++].c!=c)break;
   }
   return false;
}
/******************************************************************************/
// SOURCE LOC
/******************************************************************************/
Int SourceLoc::Compare(C SourceLoc &a, C SourceLoc &b)
{
   if(Int c=::Compare(a.file, b.file))return c;
   return a.file ? ::Compare(a.file_name, b.file_name) : ::Compare(a.id, b.id);
}
Str SourceLoc::asText()C
{
   return file ? file_name : CE.cei().elmFullName(id);
}
/******************************************************************************/
// LINE MAP
/******************************************************************************/
void LineMap::get(Int line, SourceLoc &src, Int &original_index, UID &original_id)C
{
   REPA(sections)
   {
    C Section &section     =sections[i];
      Int      section_line=line-section.offset;
      if(InRange(section_line, section.target_to_original))
      {
         src=section.src;
         original_index=section.target_to_original[section_line].line_index;
         original_id   =section.target_to_original[section_line].line_id;
         return;
      }
   }
   src.clear(); original_index=-1; original_id.zero();
}
void LineMap::operator++(int)
{
   Section *section=null;
   Int      offset =0;
   if(sections.elms())
   {
      Section &last=sections.last(); offset=last.offset+last.target_to_original.elms();
      if(!last.src.is())section=&last;
   }
   if(!section)
   {
      section=&sections.New();
      section->offset=offset;
   }
   Section::Map &map=section->target_to_original.New();
   map.line_index=-1;
   map.line_id.zero();
}
void LineMap::add(C SourceLoc &src, C Memc<CodeLine> &lines)
{
   Section *section=null;
   Int      offset =0;
   if(sections.elms())
   {
      Section &last=sections.last(); offset=last.offset+last.target_to_original.elms();
      if(last.src==src)section=&last;
   }
   if(!section)
   {
      section=&sections.New();
      section->src   =src;
      section->offset=offset;
   }
   Source *source=CE.findSource(src);
   FREPA(lines) // order important
   {
      Section::Map &map=section->target_to_original.New();
      map.line_index=lines[i].lines.y; // use Y because that one can be -1
      map.line_id.zero();
      if(source && InRange(map.line_index, source->lines))map.line_id=source->lines[map.line_index].id;
   }
}
/******************************************************************************/
// FUNCTIONS
/******************************************************************************/
Int       FindLineI(Memc<CodeLine> &code_lines, Int line) {REPA(code_lines)if(code_lines[i].hasLine(line))return i; return -1;}
CodeLine* FindLine (Memc<CodeLine> &code_lines, Int line) {Int i=FindLineI(code_lines, line); return (i>=0) ? &code_lines[i] : null;}

CodeLine* FindLineCol(Memc<CodeLine> &code_lines, C VecI2 &pos, Int   &cl_col) {VecI2 cl_pos; if(FindLineCol(code_lines, pos, cl_pos)){cl_col=cl_pos.x; return &code_lines[cl_pos.y];} cl_col=-1; return null;}
Bool      FindLineCol(Memc<CodeLine> &code_lines, C VecI2 &pos, VecI2 &cl_pos)
{
   if(pos.x>=0 && pos.y>=0)
      REPA(code_lines) // all need to be checked because they are not sorted, and there may be multiple code lines pointing to the same line
   {
      CodeLine &cl=code_lines[i]; if(cl.hasLine(pos.y))
      {
         Int cl_col=cl.findPos(pos); if(cl_col>=0){cl_pos.set(cl_col, i); return true;} // operate on temporary 'cl_col' in case 'cl_pos' is actually 'pos'
      }
   }
   return false;
}
Bool FindLineCol(Memc<CodeLine> &code_lines, Int token_index, Int token_line, Bool token_start, VecI2 &cl_pos)
{
   if(token_index>=0 && token_line>=0)
      REPA(code_lines) // all need to be checked because they are not sorted, and there may be multiple code lines pointing to the same line
   {
      CodeLine &cl=code_lines[i]; if(cl.hasLine(token_line))
      {
         cl_pos.x=cl.findToken(token_index, token_start); if(cl_pos.x>=0){cl_pos.y=i; return true;}
      }
   }
   return false;
}
/******************************************************************************/
void Remove(Memc<CodeLine> &lines, C VecI2 &start, C VecI2 &end, Bool definite)
{
   for(Int y=end.y; y>=start.y; y--)
   {
      if(y>start.y && y<end.y)lines.remove(y, true);else
      {
         CodeLine &cl=lines[y];
         Int  start_x=((y==start.y) ? start.x :                0),
                end_x=((y==  end.y) ?   end.x : cl.cols.elms()-1);
         for(Int x=start_x; x<=end_x; x++)if(definite)cl.remove(start_x);else cl.cols[x].remove();
      }
   }
}
/******************************************************************************/
void Clean(Memc<CodeLine> &lines)
{
   // remove empty lines
   REPA(lines)
   {
      CodeLine &line=lines[i];
      if(line.toRemove())
      {
         lines.remove(i, true);

         // check for empty lines between this removed line and previous line to be removed
         /* does not work well with "}\nprivate" because "\n" gets removed
         REPD(j, i) // find previous line to be removed
         {
            CodeLine &line=lines[j];
            if(!line.empty   ())break; // if encountered not empty line then break the process
            if( line.toRemove())       // if found another line to be removed
            {
               REPD(k, i-j-1)lines.remove(--i, true); // remove all between current (i) and previous (j) "j+1 .. i-1"
               break;
            }
         }*/

         // after removing removed line, check if previous and next lines are empty, in this case remove one of them
         if(InRange(i-1, lines)
         && InRange(i  , lines))
         {
            CodeLine &prev=lines[i-1]; Bool prev_can_be_removed=true;
            CodeLine &next=lines[i  ]; Bool next_can_be_removed=true;
             REPAD(c, prev)if(prev.cols[c].type!=TOKEN_NONE)if(prev.cols[c].c!='{')goto has_valid;else {prev_can_be_removed=false; break;} // allow '{'
            FREPAD(c, next)if(next.cols[c].type!=TOKEN_NONE)if(next.cols[c].c!='}')goto has_valid;else {next_can_be_removed=false; break;} // allow '}'
            if(next_can_be_removed)lines.remove(i  , true);else
            if(prev_can_be_removed)lines.remove(i-1, true);
         has_valid:;
         }
      }
   }

   FREPAD(y, lines)
   {
      CodeLine &l=lines[y];
      FREPA(l)if(l.type(i)==TOKEN_REMOVE)
      {
         Int  y0, y1;
         Bool obstacle=false;
         if( !obstacle)for(y0=y; InRange(y0-1, lines); ){if(!InRange(i, lines[y0-1]) || lines[y0-1].type(i)==TOKEN_COMMENT || lines[y0-1][i]==')')break; y0--; if(ValidType(lines[y0].type(i)) || lines[y0][i-1]==',' || lines[y0][i+1]=='{' || MustSeparate(lines[y0].type(i-1), lines[y0].type(i+1))){obstacle=true; break;}}
         if( !obstacle)for(y1=y; InRange(y1+1, lines); ){if(!InRange(i, lines[y1+1]) || lines[y1+1].type(i)==TOKEN_COMMENT                       )break; y1++; if(ValidType(lines[y1].type(i)) || lines[y1][i-1]==',' || lines[y1][i+1]=='{' || MustSeparate(lines[y1].type(i-1), lines[y1].type(i+1))){obstacle=true; break;}}
         if( !obstacle)
         {
            Bool removed=false;
            for(; y0<=y1; y0++)
            {
               CodeLine &l=lines[y0];
               if(!(ValidType(l.type(i-1)) && l.type(i+1)==TOKEN_COMMENT)){l.remove(i); removed=true;}
            }
            if(removed)i--; // try again only if we've actually removed something
         }
      }

      // convert " ;" to "; "
      FREPA(l)if(l.type(i)==TOKEN_REMOVE)if(l[i+1]==';')l.remove(i).insert(i+1, ' ', TOKEN_REMOVE);
   }
}
/******************************************************************************/
void Write(FileText &f, C Memc<CodeLine> &lines)
{
   FREPA(lines){C CodeLine &cl=lines[i]; FREPA(cl.cols)f.putChar(cl.cols[i].c); f.endLine();}
}
/******************************************************************************/
void AdjustNameSymbol(Memc<CodeLine> &lines, Symbol* &src, Symbol *dest)
{
   if(src!=dest)
   {
      // test if 'dest' is a child of 'src' : go from 'dest' to the root and check if 'src' is along the way
      if(dest)for(Symbol *cur=dest->Parent(), *child=dest; ; )
      {
         if(cur==src)
         {
            src=child;
            {CodeLine &cl=lines.New(); REP(TabLength*src->level)cl.append(' ', TOKEN_NONE); cl.append("namespace", TOKEN_KEYWORD).append(' ', TOKEN_NONE).append(*src, TOKEN_CODE);}
            {CodeLine &cl=lines.New(); REP(TabLength*src->level)cl.append(' ', TOKEN_NONE); cl.append('{', TOKEN_OPERATOR);}
            AdjustNameSymbol(lines, src, dest);
            return;
         }
         if(!cur)break;
         child=cur;
         cur  =cur->Parent();
      }

      // we must go back, because 'dest' is not a child of 'src'
      {CodeLine &cl=lines.New(); REP(TabLength*src->level)cl.append(' ', TOKEN_NONE); cl.append('}', TOKEN_OPERATOR).append(' ', TOKEN_NONE).append(S+"// namespace "+*src, TOKEN_COMMENT);}
      src=src->Parent();
      AdjustNameSymbol(lines, src, dest);
      return;
   }
}
/******************************************************************************/
void Parse(Memc<CodeLine> &lines)
{
   FREPA(lines)
   {
      // convert 8'' -> '', '' -> u''
      // convert 8"" -> "", "" -> u""
      // convert 0.0f -> 0.0f, 0.0 -> 0.0f, 0.0d -> 0.0
      // convert "<::" -> "< ::" (Apple Xcode 5 LLVM compiler fails)
      CodeLine &cl=lines[i];
      FREPA(cl)
      {
         TOKEN_TYPE type=cl.cols[i].type;
         if(type==TOKEN_CHAR16 && cl.type(i-1)!=TOKEN_CHAR16 && cl[i]=='\'')cl.insert(i, 'u', TOKEN_CHAR16);else // '' -> u''
         if(type==TOKEN_TEXT16 && cl.type(i-1)!=TOKEN_TEXT16 && cl[i]=='"' )                                     // "" -> u""
         {
            // don't parse for "include" and "extern"
            REPD(j, i)if(ValidType(cl.type(j)))
            {
               if(cl.type(j)==TOKEN_KEYWORD && cl[j]=='n' && cl[j-1]=='r' && cl[j-2]=='e' && cl[j-3]=='t' && cl[j-4]=='x' && cl[j-5]=='e'                )goto skip_parse; // extern
               if(cl.type(j)==TOKEN_PREPROC && cl[j]=='e' && cl[j-1]=='d' && cl[j-2]=='u' && cl[j-3]=='l' && cl[j-4]=='c' && cl[j-5]=='n' && cl[j-6]=='i')goto skip_parse; // include
               break;
            }
            cl.insert(i, 'u', TOKEN_TEXT16);
         skip_parse:;
         }else
         if(type==TOKEN_NUMBER) // 0.0f -> 0.0f, 0.0 -> 0.0f, 0.0d -> 0.0
         {
            for(; cl.type(i)==TOKEN_NUMBER; i++)if(cl[i]=='.') // if floating point
            {
               for(; cl.type(i+1)==TOKEN_NUMBER; i++); // move 'i' to the last character of the number token
               if(cl[i]=='f' || cl[i]=='F'){}else // do nothing (0.0f -> 0.0f)
               {
                  if(cl[i]=='d' || cl[i]=='D')cl.remove(  i                   ); // remove double suffix (0.0d -> 0.0 )
                  else                        cl.insert(++i, 'f', TOKEN_NUMBER); // add    float  suffix (0.0  -> 0.0f)
               }
               break;
            }
         }else
         if(type==TOKEN_OPERATOR)
         {
            if(cl[i]=='<' && cl[i+1]==':' && cl[i+2]==':')cl.insert(i+1, ' ', TOKEN_NONE);
         }
      }
   }
}
/******************************************************************************/
Bool OverwriteOnChange(File &src, C Str &dest, Bool *changed)
{
   File f; if(f.readStdTry(dest)){src.pos(0); if(src.equal(f)){if(changed)*changed=false; return true;}} if(changed)*changed=true;
   if(!f.writeTry(dest)){FCreateDirs(_GetPath(dest)); if(!f.writeTry(dest))return false;} src.pos(0); return src.copy(f) && f.flush();
}
Bool OverwriteOnChange(FileText &src, C Str &dest, Bool *changed)
{
#if 1 // binary mode
   return OverwriteOnChange(src._f, dest, changed);
#else // text mode
   FileText f; if(f.read(dest))
   {
      Str s, d;
      for(src.rewind(); !src.end() || !f.end(); )
      {
         src.fullLine(s);
         f  .fullLine(d);
         if(!Equal(s, d, true))goto different;
      }
      if(changed)*changed=false; return true;
   }
different:
   if(changed)*changed=true;
   if(!f.write(dest, src.encoding())){FCreateDirs(_GetPath(dest)); if(!f.write(dest, src.encoding()))return false;}
   for(src.rewind(); !src.end(); )f.putLine(src.fullLine()); return f.flushOK();
#endif
}
Bool OverwriteOnChange(XmlData &src, C Str &dest, Bool params_in_separate_lines, ENCODING encoding)
{
   FileText ft; ft.writeMem(encoding);
   src.save(ft, params_in_separate_lines);
   return OverwriteOnChange(ft, dest);
}
Bool OverwriteOnChange(Memc<CodeLine> &src, C Str &dest)
{
   FileText f; Write(f.writeMem(), src); return OverwriteOnChange(f, dest);
}
Bool OverwriteOnChangeLoud(File           &src, C Str &dest                                                  ) {if(!OverwriteOnChange(src, dest                                    ))return ErrorWrite(dest); return true;}
Bool OverwriteOnChangeLoud(FileText       &src, C Str &dest                                                  ) {if(!OverwriteOnChange(src, dest                                    ))return ErrorWrite(dest); return true;}
Bool OverwriteOnChangeLoud(XmlData        &src, C Str &dest, Bool params_in_separate_lines, ENCODING encoding) {if(!OverwriteOnChange(src, dest, params_in_separate_lines, encoding))return ErrorWrite(dest); return true;}
Bool OverwriteOnChangeLoud(Memc<CodeLine> &src, C Str &dest                                                  ) {if(!OverwriteOnChange(src, dest                                    ))return ErrorWrite(dest); return true;}
/******************************************************************************/
}}
/******************************************************************************/
