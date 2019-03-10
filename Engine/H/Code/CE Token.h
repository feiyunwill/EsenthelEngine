/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
enum TOKEN_TYPE : Byte
{
   TOKEN_NONE    , // space or empty (end of line / out of range)
   TOKEN_REMOVE  , // removed and to be cleaned
   TOKEN_OPERATOR,
   TOKEN_KEYWORD ,
   TOKEN_CODE    ,
   TOKEN_COMMENT ,
   TOKEN_TEXT8   ,
   TOKEN_TEXT16  ,
   TOKEN_CHAR8   ,
   TOKEN_CHAR16  ,
   TOKEN_NUMBER  ,
   TOKEN_PREPROC , // preprocessor

   TOKEN_MACRO              , // used only in ColorTheme (and not in source)
   TOKEN_ENUM_TYPE          , // used only in ColorTheme (and not in source)
   TOKEN_ENUM_ELM           , // used only in ColorTheme (and not in source)
   TOKEN_FUNC               , // used only in ColorTheme (and not in source)
   TOKEN_SELECT             , // used only in ColorTheme (and not in source)
   TOKEN_LINE_HIGHLIGHT     , // used only in ColorTheme (and not in source)
   TOKEN_SYMBOL_HIGHLIGHT   , // used only in ColorTheme (and not in source)
   TOKEN_BRACE_HIGHLIGHT    , // used only in ColorTheme (and not in source)
   TOKEN_PREPROC_DISABLED   , // used only in ColorTheme (and not in source)
   TOKEN_ELM_BACKGROUND     , // used only in ColorTheme (and not in source)
   TOKEN_ELM_NAME           , // used only in ColorTheme (and not in source)
   TOKEN_LINE_NUM_BACKGROUND, // used only in ColorTheme (and not in source)
   TOKEN_LINE_NUM           , // used only in ColorTheme (and not in source)
   TOKEN_TYPES              ,
};
/******************************************************************************/
STRUCT(Token , BStr)
//{
   Bool       def_decl, ctor_initializer, macro;
   TOKEN_TYPE type;
   Int        col, // original position (column) in source where the token starts
              source_index, // index of the token in source
              macro_depth , // helper used in preprocessing
              macro_col   ; // if this token was created by macro, then this member specifies column in source of that macro
   Line      *macro_line  ; // if this token was created by macro, then this member specifies line   in source of that macro
   SymbolPtr  symbol; // must be Ptr because can refer to a Symbol in different file
   Symbol    *parent;
   Line      *line;

   Int   lineIndex()C;                                  // original position (line) in source where the token starts
   VecI2 pos      ()C {return VecI2(col, lineIndex());} // original position        in source where the token starts
   Bool  sameMacro(C Token &token)C;

   Token& set        (C BStr &s,             Int col, Line &line, TOKEN_TYPE type);
   Token& setCustom  (C Str  &s,             Int col, Line &line, TOKEN_TYPE type);
   Token& setBorrowed(CChar  *d, Int length, Int col, Line &line, TOKEN_TYPE type);

   Token& macroPos  (Int col, Line &line) {T.macro_col=col; T.macro_line=&line; return T;}
   Token& macroDepth(Int depth          ) {T.macro_depth=depth; return T;}

   void asText(Str &str)C;

   void reset() {symbol=null; parent=null; def_decl=false; ctor_initializer=false;}

   Bool save(File &f, StrLibrary &sl, C Str &text)C;
   Bool load(File &f, StrLibrary &sl, C Str &text, Line &line, Str &temp);

   Token() {reset(); col=0; line=null; source_index=-1; type=TOKEN_NONE; macro=false; macro_col=-1; macro_line=null; macro_depth=0;}
};
/******************************************************************************/
inline Bool      ValidType(TOKEN_TYPE type) {return type!=TOKEN_NONE && type!=TOKEN_REMOVE;}
inline Bool      TokenType(TOKEN_TYPE type) {return type!=TOKEN_NONE && type!=TOKEN_REMOVE && type!=TOKEN_COMMENT;}
inline Bool       TextType(TOKEN_TYPE type) {return type==TOKEN_TEXT8 || type==TOKEN_TEXT16 || type==TOKEN_CHAR8 || type==TOKEN_CHAR16;}
inline Bool      ConstType(TOKEN_TYPE type) {return type==TOKEN_TEXT8 || type==TOKEN_TEXT16 || type==TOKEN_CHAR8 || type==TOKEN_CHAR16 || type==TOKEN_NUMBER;}
inline TOKEN_TYPE MainType(TOKEN_TYPE type)
{
   switch(type)
   {
      case TOKEN_CHAR8:
      case TOKEN_TEXT8: return TOKEN_TEXT8;

      case TOKEN_CHAR16:
      case TOKEN_TEXT16: return TOKEN_TEXT16;

      case TOKEN_COMMENT: return TOKEN_COMMENT;

      default: return TOKEN_CODE;
   }
}
inline Bool MustSeparate(TOKEN_TYPE a, TOKEN_TYPE b)
{
   return (a==TOKEN_CODE || a==TOKEN_KEYWORD)
       && (b==TOKEN_CODE || b==TOKEN_KEYWORD);
}
Bool TextToIDAt    (C Str &text, Int pos, UID &id, VecI2 &range); // !! this function is not multi-threaded safe !!
Bool TextToIDInside(C Str &text, Int pos, UID &id, VecI2 &range); // !! this function is not multi-threaded safe !!
/******************************************************************************/
} // namespace
#endif
/******************************************************************************/
