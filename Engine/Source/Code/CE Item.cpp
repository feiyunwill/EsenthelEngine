/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Item& GetFolder(Memx<Item> &items, C Str &name)
{
   REPA(items)if(items[i].base_name==name)return items[i];
   return items.New().set(Item::FOLDER, name);
}
Item& GetFile(Memx<Item> &items, C Str &name, C Str &full_name)
{
   Str start=GetStart(name);
   if( start.length()==name.length())return items.New().set(Item::FILE, name, full_name);
   return GetFile(GetFolder(items, start).children, GetStartNot(name), full_name);
}
/******************************************************************************/
void Item::replacePath(C Str &src, C Str &dest)
{
   ReplacePath(full_name, src, dest);
   REPAO(children).replacePath(src, dest);
}
/******************************************************************************/
Bool Item::save(File &f, StrLibrary &sl)C
{
   f<<type;
   sl.putStr(f, base_name);
   sl.putStr(f, full_name);
   f.cmpUIntV(children.elms()); FREPA(children)if(!children[i].save(f, sl))return false;
   return f.ok();
}
Bool Item::load(File &f, StrLibrary &sl)
{
   f>>type;
   sl.getStr(f, base_name); // base_name is not GetBase(full_name) because it is display name, while full_name is file name (folders have it empty)
   sl.getStr(f, full_name);
   children.setNum(f.decUIntV()); FREPA(children)if(!children[i].load(f, sl))goto error;
   if(f.ok())return true;
error:
   /*del();*/ return false;
}
/******************************************************************************/
}}
/******************************************************************************/
