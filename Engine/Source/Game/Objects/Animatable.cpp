/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
static SkelAnim* GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim)
{
   if(obj)if(C Param *param=obj->findParam(anim))return anim_skel.findSkelAnim(param->asID());
   return null;
}
/******************************************************************************/
Animatable::~Animatable()
{
}
Animatable::Animatable()
{
   scale         =0;
   mesh_variation=0;
   skel_anim     =null;
  _matrix.zero();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Animatable::create(Object &obj)
{
   base          =obj.firstStored       ();
   mesh_variation=obj.meshVariationIndex();
   scale         =obj.scale             ();
  _matrix        =obj.matrixFinal       ().normalize();

   setUnsavedParams();

   if(skel.skeleton())skel_anim=GetAnim(skel, &obj, "animation");
}
void Animatable::setUnsavedParams()
{
         mesh=(base ? base->mesh    () : MeshPtr());
   skel.create(mesh ? mesh->skeleton() : null, scale, _matrix);

   if(base && base->phys())actor.create(*base->phys(), 0, scale)
                                .matrix(_matrix)
                                .obj   (this)
                                .group (AG_TERRAIN);
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    Animatable::pos   (                ) {return _matrix.pos;}
Matrix Animatable::matrix(                ) {return _matrix    ;}
void   Animatable::pos   (C Vec    &pos   ) {actor.pos   (pos   ); T._matrix.pos=pos   ;}
void   Animatable::matrix(C Matrix &matrix) {actor.matrix(matrix); T._matrix    =matrix;}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Animatable::memoryAddressChanged()
{
   actor.obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Animatable::update()
{
   skel.clear().animate(skel_anim, Time.time()).updateMatrix(_matrix).updateVelocities();
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Animatable::drawPrepare()
{
   if(mesh)if(Frustum(*mesh, skel.matrix()))
   {
      SetVariation(mesh_variation); mesh->draw(skel);
      SetVariation();
   }
   return 0; // no additional render modes required
}
/******************************************************************************/
void Animatable::drawShadow()
{
   if(mesh)if(Frustum(*mesh, skel.matrix()))
   {
      SetVariation(mesh_variation); mesh->drawShadow(skel);
      SetVariation();
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Animatable::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f.putAsset(base.id())<<scale<<_matrix;
      f.putAsset(skel_anim ? skel_anim->id() : UIDZero);
      f.putUInt (mesh ? mesh->variationID(mesh_variation) : 0);
      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Animatable::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         base=f.getAssetID();
         f>>scale>>_matrix;
         setUnsavedParams();
         UID            anim_id=f.getAssetID(); if(skel.skeleton())skel_anim=skel.getSkelAnim(anim_id);
         UInt mesh_variation_id=f.getUInt   (); mesh_variation=(mesh ? mesh->variationFind(mesh_variation_id) : 0);
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
