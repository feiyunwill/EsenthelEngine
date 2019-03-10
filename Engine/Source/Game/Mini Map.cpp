/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
void MiniMap::Settings::del()
{
   image_size     =0;
   areas_per_image=1; // use 1 instead of 0 to avoid div by 0 which could cause crash on Web
   area_size      =1; // use 1 instead of 0 to avoid div by 0 which could cause crash on Web
}
Bool MiniMap::Settings::save(File &f)C
{
   f.cmpUIntV(0); // version
   f<<areas_per_image<<image_size<<area_size;
   return f.ok();
}
Bool MiniMap::Settings::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>areas_per_image>>image_size>>area_size;
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
Bool MiniMap::Settings::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool MiniMap::Settings::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
Bool MiniMap::Create(Image &image, C VecI2 &key, Ptr user)
{
   MiniMap &mm=*(MiniMap*)user;
   if(mm.name().is())image.load(mm.name()+'/'+key);
   return true;
}
/******************************************************************************/
MiniMap::MiniMap() : _map(Compare, MiniMap::Create, this)
{
  _id.zero();
}
void MiniMap::del()
{
  _map     .del();
  _settings.del();
  _name    .del();
  _id      .zero();
}
/******************************************************************************/
Bool MiniMap::load(C Str &name)
{
   del();
   if(name.is())
   {
      if(!DecodeFileName(name, _id))_id.zero();
     _name=name; _name.tailSlash(false);
      if(!_settings.load(_name+"/Settings"))goto error;
   }
   return true;
error:
   del(); return false;
}
void MiniMap::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load Mini Map \""+name+"\"");}
Bool MiniMap::load     (C UID &id  ) {return load(id.valid() ? _EncodeFileName(id) : null);}
void MiniMap::operator=(C UID &id  ) {         T=(id.valid() ? _EncodeFileName(id) : null);}
/******************************************************************************/
void MiniMap::clear(C RectI *leave)
{
   if(!leave)_map.del();else
   {
      REPA(_map) // check all elements (images), order is important
         if(!Cuts(_map.key(i), *leave)) // if it shouldn't be left (coordinates aren't in the 'leave' rectangle)
           _map.remove(i); // remove this element
   }
}
/******************************************************************************/
}}
/******************************************************************************/
