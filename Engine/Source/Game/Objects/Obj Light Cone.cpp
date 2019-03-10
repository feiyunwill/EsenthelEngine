/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
ObjLightCone::~ObjLightCone()
{
}
ObjLightCone::ObjLightCone()
{
   cast_shadows=true;
   range     =0;
   volumetric=0;
   angle     =1;
   falloff   =0.5f;
   position .zero();
   direction.set(0, 0, 1);
   color    .set(1, 1, 1);
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ObjLightCone::create(Object &obj)
{
   range    = obj.scale();
   position = obj.matrixFinal().pos;
   direction=!obj.matrix.z;
   if(Param *param=obj.findParam("cast shadows"))cast_shadows=    param->asBool();
   if(Param *param=obj.findParam("volumetric"  ))volumetric  =Max(param->asFlt (), 0.0f);
   if(Param *param=obj.findParam("angle"       ))angle       =Max(param->asFlt (), 0.0f);
   if(Param *param=obj.findParam("falloff"     ))falloff     =Sat(param->asFlt ());
   if(Param *param=obj.findParam("color"       ))color       =    param->asVec ();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    ObjLightCone::pos   (                ) {return position           ;}
Matrix ObjLightCone::matrix(                ) {return position           ;}
void   ObjLightCone::pos   (C Vec    &pos   ) {       position=pos       ;}
void   ObjLightCone::matrix(C Matrix &matrix) {       position=matrix.pos; direction=matrix.z;}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool ObjLightCone::update()
{
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void ObjLightCone::drawPrepare(C Matrix &matrix)
{
   LightCone lc(range, position*matrix, !(direction*matrix.orn()), color, volumetric);
   lc.pyramid.scale=angle;
   lc.falloff      =falloff;
   lc.add(cast_shadows, this);
}
UInt ObjLightCone::drawPrepare()
{
   LightCone lc(range, position, direction, color, volumetric);
   lc.pyramid.scale=angle;
   lc.falloff      =falloff;
   lc.add(cast_shadows, this);
   return 0; // no additional render modes required
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool ObjLightCone::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f<<cast_shadows<<range<<volumetric<<angle<<falloff<<position<<direction<<color;
      return f.ok();
   }
   return false;
}
Bool ObjLightCone::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>cast_shadows>>range>>volumetric>>angle>>falloff>>position>>direction>>color;
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
