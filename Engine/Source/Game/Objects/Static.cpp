/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Static::~Static()
{
}
Static::Static()
{
   scale=0;
   mesh_variation=0;
  _matrix.zero();
  _matrix_scaled.zero();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Static::setUnsavedParams()
{
   mesh=(base ? base->mesh() : MeshPtr());
   if(base && base->phys())actor.create(*base->phys(), 0, scale)
                                .matrix(_matrix)
                                .obj   (this)
                                .group (AG_TERRAIN);
}
void Static::create(Object &obj)
{
   base  =obj.firstStored();
   scale =obj.scale3     ();
  _matrix=obj.matrixFinal().normalize(); _matrix_scaled=_matrix; _matrix_scaled.scaleOrnL(scale);
   mesh_variation=obj.meshVariationIndex();

   setUnsavedParams();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    Static::pos         (                ) {return _matrix.pos   ;}
Matrix Static::matrix      (                ) {return _matrix       ;}
Matrix Static::matrixScaled(                ) {return _matrix_scaled;}
void   Static::pos         (C Vec    &pos   ) {actor.pos   (pos   ); T._matrix_scaled.pos=T._matrix.pos=pos   ;}
void   Static::matrix      (C Matrix &matrix) {actor.matrix(matrix); T._matrix_scaled    =T._matrix    =matrix; T._matrix_scaled.scaleOrnL(scale);}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Static::memoryAddressChanged()
{
   actor.obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Static::update()
{
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Static::drawPrepare()
{
   if(mesh)if(Frustum(*mesh, _matrix_scaled))
   {
      SetVariation(mesh_variation); mesh->draw(_matrix_scaled);
      SetVariation();
   }
   return 0; // no additional render modes required
}
/******************************************************************************/
void Static::drawShadow()
{
   if(mesh)if(Frustum(*mesh, _matrix_scaled))
   {
      SetVariation(mesh_variation); mesh->drawShadow(_matrix_scaled);
      SetVariation();
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Static::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f.putAsset(base.id())<<scale<<_matrix;
      f.putUInt (mesh ? mesh->variationID(mesh_variation) : 0);
      if(!actor.saveState(f))return false;
      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Static::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         base=f.getAssetID();
         f>>scale>>_matrix; _matrix_scaled=_matrix; _matrix_scaled.scaleOrnL(scale);
         setUnsavedParams();
         UInt mesh_variation_id=f.getUInt(); mesh_variation=(mesh ? mesh->variationFind(mesh_variation_id) : 0);
         if(!actor.loadState(f))return false;
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
