/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
void CodeEditor::parseHeaderEx(Str h, C Str &parent_path, Bool try_system, Memc<Token> &dest, Memc<Macro> &macros, Memc<SymbolPtr> &usings, Bool ee, Memc<PrepCond> &prep_if)
{
   Source *s=null;
   if(FullPath(h))s=getSource(h);else
   {
      if(try_system)
         // TODO: temporarily don't use system headers (too slow, not fully supported, and pollute the global namespace)
         if(0)
      {
         Str sdk_path=GetRegStr(RKG_LOCAL_MACHINE, "Software/Microsoft/Microsoft SDKs/Windows/v6.0A/InstallationFolder"); if(sdk_path.is())sdk_path.tailSlash(true)+="Include";
         Str  vc_path=vs_path; if(vc_path.is())vc_path.tailSlash(true)+="VC\\include";
         if(!s && sdk_path.is()){Str t=sdk_path+'\\'+h; if(s=getSource(t))h=t;}
         if(!s &&  vc_path.is()){Str t= vc_path+'\\'+h; if(s=getSource(t))h=t;}
      }
      if(!s)
      {
         if(parent_path.is()){Str t=NormalizePath(parent_path+'\\'+h); if(s=getSource(t))h=t;}else // try relative to parent
         {
            // try relative to build
            Str build_path, build_project_name; if(getBuildPath(build_path, build_project_name)){Str t=NormalizePath(build_path+h); if(s=getSource(t))h=t;}
         }
      }
   }
   if(s)
   {
      if(s->recursive>16)
      {
         // TODO: display error for Header Recursiveness
         DEBUG_ASSERT(false, "Header Recursive too much");
      }else
      {
         s->recursive++;
         s->header=true;
         if(ee)
         {
            s->ee_header=s->Const=true;

            // replace STRUCT, STRUCT_PRIVATE
            FREPAD(l, s->lines)
            {
               Line &line=s->lines[l];
               if(!line.preproc)
               {
                  Int base;
                  if((base=TextPosI(line, "STRUCT(", true, true))>=0)
                  {
                     line.remove(base, 7).insert(base, "struct ");
                     Int i=TextPosI(line()+base, ','); if(i>=0){i+=base; line.remove(i).insert(i, ":", TOKEN_OPERATOR);}
                         i=TextPosI(line()+base, ')'); if(i>=0){i+=base; line.remove(i);}
                     line.setType(line.starts_with_comment, line.starts_with_preproc);
                     if(InRange(l+1, s->lines)){Line &line=s->lines[l+1]; line=Replace(line, "//{", "{"); line.setType(line.starts_with_comment, line.starts_with_preproc);}
                  }else
                  if((base=TextPosI(line, "STRUCT_PRIVATE(", true, true))>=0)
                  {
                     line.remove(base, 15).insert(base, "struct ");
                     Int i=TextPosI(line()+base, ','); if(i>=0){i+=base; line.remove(i).insert(i, ":", TOKEN_OPERATOR).insert(i+1, " ", TOKEN_NONE).insert(i+2, "private", TOKEN_KEYWORD);}
                         i=TextPosI(line()+base, ')'); if(i>=0){i+=base; line.remove(i);}
                     line.setType(line.starts_with_comment, line.starts_with_preproc);
                     if(InRange(l+1, s->lines)){Line &line=s->lines[l+1]; line=Replace(line, "//{", "{"); line.setType(line.starts_with_comment, line.starts_with_preproc);}
                  }
               }else
               {
                  if(Starts(_SkipWhiteChars(line), "#define const_mem_addr", true, true)) // for generation of the EE header we need to remove definition of 'const_mem_addr' as a macro, because in Code Editor it is used as a keyword
                  {
                     s->lines.removeValid(l, true);
                     if(InRange(l, s->lines) && !Is(_SkipWhiteChars(s->lines[l])))s->lines.removeValid(l, true); // if the next line is empty then remove it
                     l--;
                  }
               }
            }
         }

         Int token_index=0; // will be invalid if source is included more than 1 time, but it's only for some extra auto-complete info, so not so important
         Memc<Token*> temp;
         FREPA(s->lines)
         {
            Int start=i; s->preprocess(macros, i, temp, true, prep_if);
            for(Int j=start; j<=i; j++)
            {
               Line        &line=s->lines[j];
               Memc<Token> &src =line.Tokens(); FREPA(src){Token &token=dest.New(); token=src[i]; token.source_index=token_index++;}
               Memc<Token> &org =line.tokens  ;
               if(line.preproc && !line.starts_with_preproc && !line.starts_with_macro_param && org.elms()>=3)
                  if(!prep_if.elms() || prep_if.last().currently_valid)
                     if(org[0]=='#' && org[1]=="include")
               {
                  if(org[2]=='<')
                  {
                     Str file; for(Int i=3; i<org.elms() && org[i]!='>'; i++)file+=org[i];
                     parseHeaderEx(file, GetPath(h), true , dest, macros, usings, ee, prep_if);
                  }else
                  {
                     Str file=org[2]; file.removeLast().remove(0);
                     parseHeaderEx(file, GetPath(h), false, dest, macros, usings, ee, prep_if);
                  }
               }
            }
         }
         // after possible recursive modifying of the same file is finished
         s->tokens.clear();
         FREPA(s->lines)
         {
            Memc<Token> &src=s->lines[i].Tokens();
            FREPA(src)s->tokens.add(&src[i]);
         }
         s->recursive--;
      }
      s->parse_count++;
   }
}
/******************************************************************************/
void CodeEditor::parseHeader(C Str &h, Memc<Macro> &macros, Memc<SymbolPtr> &usings, Bool ee)
{
   CppMode=true;

   Memc<PrepCond  > prep_if   ;
   Memc<Token     > tokens    ; parseHeaderEx(h, S, ee ? false : true, tokens, macros, usings, ee, prep_if);
   Memc<Token*    > token_ptrs; FREPA(tokens)token_ptrs.add(&tokens[i]);
   Memc<SymbolDef > symbols   ; 
   Memc<SymbolDecl> decls     ; DetectDataTypes(symbols, decls, token_ptrs);
                                  LinkDataTypes(symbols,        token_ptrs);
                                 DetectVarFuncs(symbols,        token_ptrs);

   // adjust symbol token indexes (convert from 'tokens' list of all source tokens, to per-symbol tokens, this will work ok only if source was included only one time)
   REPA (symbols)FlagDisable(symbols[i]->helper, Symbol::HELPER_ADJUSTED_INDEXES);
   REPA (decls  )FlagDisable(decls  [i]->helper, Symbol::HELPER_ADJUSTED_INDEXES);
   REPAO(symbols)->adjustIndexes(tokens);
   REPAO(decls  )->adjustIndexes(tokens);

   // adjust source tokens, since they were modified inside 'DetectDataTypes', 'LinkDataTypes', 'DetectVarFuncs'
   REPA(tokens)
   {
      Token     &token =tokens[i];
      if(Source *source=token.line->source)
         if(InRange(token.source_index, source->tokens))Swap(*source->tokens[token.source_index], token);
   }

   // put SymbolDef's into proper sources
   REPA(symbols){SymbolDef  &symbol=symbols[i]; if(symbol && symbol->source)Swap(symbol->source->symbols.New(), symbol);}
   REPA(decls  ){SymbolDecl &decl  =decls  [i]; if(decl   && decl  ->source)Swap(decl  ->source->decls  .New(), decl  );}

   CppMode=false;
}
/******************************************************************************/
}}
/******************************************************************************/
