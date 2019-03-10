/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
// BORROWED STRING
/******************************************************************************/
enum BS_CODES
{
   BSC_TMPL_B  ,
   BSC_TMPL_E  ,
   BSC_CUSTOM  ,
   BSC_BORROWED,
};
static void BStrSave(C BStr &t, File &f, C Str &text)
{
   if(t==TMPL_B)f.cmpUIntV(BSC_TMPL_B);else
   if(t==TMPL_E)f.cmpUIntV(BSC_TMPL_E);else
   {
      UIntPtr offset =t()-text();
      Bool    invalid=(t.is() && (offset>=text.length() || t.length()>text.length()));
      if(t.custom() && t.length() || invalid)f.cmpUIntV(BSC_CUSTOM).putStr(t.asStr());else
      {
         f.cmpUIntV(BSC_BORROWED+t.length()); if(t.is())f.cmpUIntV(offset);
      }
   }
}
static void BStrLoad(BStr &t, File &f, C Str &text, Str &temp)
{
   t.del();
   Int code=f.decUIntV();
   if(code==BSC_TMPL_B  )                 t.setBorrowed(TMPL_B); else
   if(code==BSC_TMPL_E  )                 t.setBorrowed(TMPL_E); else
   if(code==BSC_CUSTOM  ){f.getStr(temp); t.setCustom  (temp  );}else
   if(code>=BSC_BORROWED)
   {
      Int length=code-BSC_BORROWED;
      if( length && length<=text.length())t.setBorrowed(text()+f.decUIntV(), length);
   }
}
/******************************************************************************/
// TOKEN
/******************************************************************************/
Int Token::lineIndex()C {return line ? line->line : -1;}

Bool Token::sameMacro(C Token &token)C {return macro_line==token.macro_line && macro_col==token.macro_col;}

Token& Token::set        (C BStr &s,             Int col, Line &line, TOKEN_TYPE type) {super::set        (s        ); T.col=col; T.line=&line; T.type=type; return T;}
Token& Token::setCustom  (C Str  &s,             Int col, Line &line, TOKEN_TYPE type) {super::setCustom  (s        ); T.col=col; T.line=&line; T.type=type; return T;}
Token& Token::setBorrowed(CChar  *d, Int length, Int col, Line &line, TOKEN_TYPE type) {super::setBorrowed(d, length); T.col=col; T.line=&line; T.type=type; return T;}
/******************************************************************************/
void Token::asText(Str &str)C
{
   if(T==TMPL_B)str="<";else
   if(T==TMPL_E)str=">";else
                str=T;
}
enum
{
   DEF_DECL=1<<0,
   CTOR    =1<<1,
   MACRO   =1<<2,
};
Bool Token::save(File &f, StrLibrary &sl, C Str &text)C
{
   BStrSave(T, f, text);
   f.putMulti(Byte((def_decl ? DEF_DECL : 0) | (ctor_initializer ? CTOR : 0) | (macro ? MACRO : 0)), type);
   f.cmpUIntV(col); sl.putStr(f, symbol.name()); sl.putStr(f, Symbols.name(parent));
   return f.ok();
}
Bool Token::load(File &f, StrLibrary &sl, C Str &text, Line &line, Str &temp) // load this after loading source symbols because of 'parent', and line text (Str) because of 'BStr'
{
   BStrLoad(T, f, text, temp);
   Byte flag; f.getMulti(flag, type); def_decl=FlagTest(flag, DEF_DECL); ctor_initializer=FlagTest(flag, CTOR); macro=FlagTest(flag, MACRO);
   col=f.decUIntV(); sl.getStr(f, temp); symbol=temp; sl.getStr(f, temp); parent=SymbolPtr().get(temp)(); T.line=&line; macro_col=-1; macro_line=null; macro_depth=0;
   if(f.ok())return true;
   del(); return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
static Str TempStr;
Bool TextToIDAt(C Str &text, Int pos, UID &id, VecI2 &range)
{
   id.zero();
   range=pos;
   if(text[pos]=='U' && text[pos+1]=='I' && text[pos+2]=='D' && CodeCharType(text[pos-1])!=CHART_CHAR) // UID(..)
   {
      CalcValue cv[4];
      if(CChar *next=_SkipWhiteChars(text()+pos+3          ))              if(*next++=='(')
      if(       next=_SkipWhiteChars(TextValue(next, cv[0])))if(cv[0].type)if(*next++==',')
      if(       next=_SkipWhiteChars(TextValue(next, cv[1])))if(cv[1].type)if(*next++==',')
      if(       next=_SkipWhiteChars(TextValue(next, cv[2])))if(cv[2].type)if(*next++==',')
      if(       next=_SkipWhiteChars(TextValue(next, cv[3])))if(cv[3].type)if(*next  ==')')
      {
         REPAO(id.i)=cv[i].asUInt();
         range.set(pos, next-text());
         return true;
      }
   }else
   if(text[pos]=='"' && text[pos+24+1]=='"') // ".." 24 char ID
   {
      TempStr.clear(); for(Int c=pos+1, j=c; j<c+24; j++)TempStr+=text[j]; // operate on 'TempStr' so it doesn't require memory allocation all the time
      if(DecodeFileName(TempStr, id))
      {
         range.set(pos, pos+24+1);
         return true;
      }
   }
   return false;
}
Bool TextToIDInside(C Str &text, Int pos, UID &id, VecI2 &range)
{
   REP(pos+1)if(TextToIDAt(text, i, id, range)) // start from pos and go back
   {
      if(pos<=range.y && CE.cei().idToText(id).is())return true; // if cursor within range and it's a text ID
      break; // pause on first ID found
   }
   id.zero(); return false;
}
/******************************************************************************/
}}
/******************************************************************************/
