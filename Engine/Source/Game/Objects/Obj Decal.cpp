/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
ObjDecal::~ObjDecal()
{
}
ObjDecal::ObjDecal()
{
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ObjDecal::create(Object &obj)
{
   decal.terrain_only=true;
   if(C Param *material=obj.findParam("material"))decal.material(material->asID());
   decal.matrix=obj.matrixFinal(); decal.matrix.scaleOrn(0.5f); Swap(decal.matrix.y, decal.matrix.z); decal.matrix.y.chs();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    ObjDecal::pos   (                ) {return decal.matrix.pos       ;}
Matrix ObjDecal::matrix(                ) {return decal.matrix           ;}
void   ObjDecal::pos   (C Vec    &pos   ) {       decal.matrix.pos=pos   ;}
void   ObjDecal::matrix(C Matrix &matrix) {       decal.matrix    =matrix;}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool ObjDecal::update()
{
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt ObjDecal::drawPrepare()
{
   return IndexToFlag(RM_OVERLAY); // decal requires RM_OVERLAY render mode
}
void ObjDecal::drawOverlay() {decal.drawStatic();}
/******************************************************************************/
// IO
/******************************************************************************/
Bool ObjDecal::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      if(decal.save(f))
         return f.ok();
   }
   return false;
}
Bool ObjDecal::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         if(decal.load(f))
            if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
