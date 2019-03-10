/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
struct Macro
{
   struct Part
   {
      TOKEN_TYPE type;
      Int        param; // -1 if not param, >=0 if should be replaced with 'param' param
      BStr       text;

      void set(TOKEN_TYPE type, Int param, BStr *text) {T.type=type; T.param=param; if(text)T.text=*text;}

      Bool save(File &f)C
      {
         f.putMulti(type, param).putStr(text.asStr());
         return true;
      }
      Bool load(File &f)
      {
         f.getMulti(type, param); text.setCustom(f.getStr());
         return true;
      }

      Part() {type=TOKEN_NONE; param=-1;}
   };

   Bool       all_up_case, use_for_suggestions;
   Int        params, // -1 if macro definition is not followed by (), 0 if is followed by (), 1 if (x), 2 if (x, y), ..
              line;
   Str        name, def;
   Memc<Part> parts;
   Source    *source;

   Bool operator==(CChar8  *name )C {return Equal(T.name, name, true);}
   Bool operator==(CChar   *name )C {return Equal(T.name, name, true);}
   Bool operator==(C Macro &macro)C {return T==macro.name;}

   Macro& set(Token &token);
   Macro& set(C Str &name );

   Macro& reset() {params=-1; line=-1; parts.clear(); source=null; return T;}

   Bool save(File &f, StrLibrary &sl)C;
   Bool load(File &f, StrLibrary &sl, Str &temp);

   Macro() {all_up_case=false; use_for_suggestions=true; params=-1; line=-1; source=null;}
};
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int CompareCS(C Edit::Macro &a, C Edit::Macro &b) {return CompareCS(a.name, b.name);}
inline Int CompareCS(C Edit::Macro &a, C Str         &b) {return CompareCS(a.name, b     );}
inline Int CompareCS(C Edit::Macro &a, C BStr        &b) {return CompareCS(a.name, b     );}
inline Int CompareCS(C Edit::Macro &a, CChar8*C      &b) {return CompareCS(a.name, b     );}
/******************************************************************************/
#endif
/******************************************************************************/
