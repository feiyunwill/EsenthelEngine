/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Kinematic::~Kinematic()
{
}
Kinematic::Kinematic()
{
   scale=0;
   mesh_variation=0;
  _matrix.zero();
  _matrix_scaled.zero();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Kinematic::setUnsavedParams()
{
   mesh=(base ? base->mesh() : MeshPtr());
   if(base && base->phys())actor.create(*base->phys(), 1, scale, true)
                                .matrix(_matrix)
                                .obj   (this)
                                .group (AG_TERRAIN);
}
void Kinematic::create(Object &obj)
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
Vec    Kinematic::pos            (                ) {return _matrix.pos   ;}
Matrix Kinematic::matrix         (                ) {return _matrix       ;}
Matrix Kinematic::matrixScaled   (                ) {return _matrix_scaled;}
void   Kinematic::pos            (C Vec    &pos   ) {actor.         pos   (pos   ); T._matrix_scaled.pos=T._matrix.pos=pos   ;}
void   Kinematic::kinematicMoveTo(C Vec    &pos   ) {actor.kinematicMoveTo(pos   ); T._matrix_scaled.pos=T._matrix.pos=pos   ;}
void   Kinematic::matrix         (C Matrix &matrix) {actor.         matrix(matrix); T._matrix_scaled    =T._matrix    =matrix; T._matrix_scaled.scaleOrnL(scale);}
void   Kinematic::kinematicMoveTo(C Matrix &matrix) {actor.kinematicMoveTo(matrix); T._matrix_scaled    =T._matrix    =matrix; T._matrix_scaled.scaleOrnL(scale);}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Kinematic::memoryAddressChanged()
{
   actor.obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Kinematic::update()
{
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Kinematic::drawPrepare()
{
   if(mesh)if(Frustum(*mesh, _matrix_scaled))
   {
      SetVariation(mesh_variation); mesh->draw(_matrix_scaled);
      SetVariation();
   }
   return 0; // no additional render modes required
}
/******************************************************************************/
void Kinematic::drawShadow()
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
Bool Kinematic::save(File &f)
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
Bool Kinematic::load(File &f)
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
