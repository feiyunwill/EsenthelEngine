/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Macro& Macro::set(Token &token) {T.name=token; all_up_case=AllUpCase(name); source=token.line->source; line=token.lineIndex(); return T;}
Macro& Macro::set(C Str &name ) {T.name=name ; all_up_case=AllUpCase(name); source=null              ; line=               -1; return T;}

enum
{
   ALL_UP_CASE        =1<<0,
   USE_FOR_SUGGESTIONS=1<<1,
};

Bool Macro::save(File &f, StrLibrary &sl)C
{
   f.putMulti(Byte((all_up_case ? ALL_UP_CASE : 0)|(use_for_suggestions ? USE_FOR_SUGGESTIONS : 0)), params, line).putStr(name).putStr(def);
   if(parts.save(f))
   {
      sl.putStr(f, source ? source->loc.file_name : S);
      return f.ok();
   }
   return false;
}
Bool Macro::load(File &f, StrLibrary &sl, Str &temp)
{
   Byte flag;
   f.getMulti(flag, params, line).getStr(name).getStr(def);
   all_up_case        =FlagTest(flag, ALL_UP_CASE);
   use_for_suggestions=FlagTest(flag, USE_FOR_SUGGESTIONS);
   if(parts.load(f))
   {
      sl.getStr(f, temp); source=CE.findSource(temp);
      if(f.ok())return true;
   }
   /*del();*/ return false;
}
/******************************************************************************/
}}
/******************************************************************************/
