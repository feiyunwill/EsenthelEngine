/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Int MaterialPalette::getMaterialIndex0(C MaterialPtr &material) {return Max(0, getMaterialIndex(material));}
Int MaterialPalette::getMaterialIndex (C MaterialPtr &material)
{
   if(InRange(_last, T) && T[_last]==material)return _last;
   if(!material && elms())return 0; // 0 is reserved to be null material (however return it only if it was already added to the palette, so 'operator[]' will not crash)

   Int find=T.find(material);
   if( find<0 && elms()<255) // limit to 255 because we're saving later number of elements as byte
   {
      if( !elms()){New(); if(!material)return 0;} // 0 is reserved to be null material
      find=elms(); add(material);
   }
   if(find>0)_last=find; // don't adjust '_last' for null because that one is easy to find
   return find;
}
/******************************************************************************/
Int IDPalette::getIDIndex0(C UID &id) {return Max(0, getIDIndex(id));}
Int IDPalette::getIDIndex (C UID &id)
{
   if(InRange(_last, T) && T[_last]==id)return _last;
   if(!id.valid() && elms())return 0; // 0 is reserved to be 'UIDZero' id (return it only if it was added to the palette)

   Int find=T.find(id);
   if( find<0 && elms()<255) // limit to 255 because we're saving later number of elements as byte
   {
      if( !elms()){New(); if(!id.valid())return 0;} // 0 is reserved to be 'UIDZero' id
      find=elms(); add(id);
   }
   if(find>0)_last=find; // don't adjust '_last' for null because that one is easy to find
   return find;
}
/******************************************************************************/
void MaterialPalette::remove(Int i)
{
   if(i)super::remove(i, true);
}
/******************************************************************************/
Bool MaterialPalette::clean(Bool is[256], Byte remap[256])
{
   for(Int i=1; i<elms(); i++)if(!is[i])
   {
      remap[0]=0;
      for(Int i=1, next=1; i<elms(); i++)if( is[i])remap[i]=next++;else remap[i]=0;
      for(Int i=elms();  --i>=1    ;    )if(!is[i])super::remove(i, true); // need to go from the end
      return true;
   }
   return false;
}
/******************************************************************************/
Bool MaterialPalette::save(File &f, CChar *path)C
{
   f.putMulti(Byte(1), Byte(elms())); // version
   FREPA(T)f.putAsset(T[i].id());
   return f.ok();
}
Bool MaterialPalette::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 1:
      {
         setNum(f.getByte());
         FREPA(T)super::operator[](i).require(f.getAssetID(), path);
         if(f.ok())return true;
      }break;

      case 0:
      {
         setNum(f.getByte());
         FREPA(T)super::operator[](i).require(f._getStr(), path);
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
Bool MaterialPalette::loadOld(File &f, CChar *path)
{
   setNum(f.getInt());
   FREPA(T)super::operator[](i).require(f._getStr(), path);
   if(f.ok())return true;
   del(); return false;
}
/******************************************************************************/
Bool IDPalette::save(File &f)C
{
   f.cmpUIntV(0); // version
   if(saveRaw(f))
      return f.ok();
   return false;
}
Bool IDPalette::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         if(loadRaw(f))
            if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
