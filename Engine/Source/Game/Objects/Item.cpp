/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Item::~Item()
{
   if(_grabber)_grabber->grabStop();
}
Item::Item()
{
   scale=0;
   mesh_variation=0;
  _vel    .zero();
  _ang_vel.zero();
  _grabber=null;
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Item::setUnsavedParams()
{
   mesh=(base ? base->mesh() : MeshPtr());
   if(base && base->phys())actor.create(*base->phys(), 1, scale).obj(this);
}
void Item::create(Object &obj)
{
   scale         =obj.scale             ();
   base          =obj.firstStored       ();
   mesh_variation=obj.meshVariationIndex();
   setUnsavedParams();

   actor.matrix  (obj.matrixFinal().normalize())
        .adamping(5   )
        .ccd     (true); // enable ccd for all items, because most item actors are very small and can easily fall through ground or other obstacles
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    Item::pos         (                ) {return actor.pos   (      );                }
void   Item::pos         (C Vec    &pos   ) {       actor.pos   (pos   );                }
Matrix Item::matrix      (                ) {return actor.matrix(      );                }
Matrix Item::matrixScaled(                ) {return actor.matrix(      ).scaleOrn(scale);}
void   Item::matrix      (C Matrix &matrix) {       actor.matrix(matrix);                }
/******************************************************************************/
void Item::setDrawingVelocities(C Vec &vel, C Vec &ang_vel)
{
   T._vel    =vel    ;
   T._ang_vel=ang_vel;
}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Item::memoryAddressChanged()
{
   actor.obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Item::update()
{
   setDrawingVelocities(actor.vel(), actor.angVel());
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Item::drawPrepare()
{
   if(mesh && actor.is())
   {
      Matrix matrix=matrixScaled();
      if(Frustum(*mesh, matrix))
      {
         SetVariation(mesh_variation); mesh->draw(matrix, _vel, _ang_vel);
         SetVariation();
      }
   }
   return 0; // no additional render modes required
}
/******************************************************************************/
void Item::drawShadow()
{
   if(mesh && actor.is())
   {
      Matrix matrix=matrixScaled();
      if(Frustum(*mesh, matrix))
      {
         SetVariation(mesh_variation); mesh->drawShadow(matrix);
         SetVariation();
      }
   }
}
/******************************************************************************/
// ENABLE / DISABLE
/******************************************************************************/
void Item::disable()
{
   super::disable();

   actor.kinematic(true);
}
void Item::enable()
{
   super::enable();

   actor.kinematic(false);
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Item::willBeMovedFromWorldToStorage()
{
   super::willBeMovedFromWorldToStorage();

   actor.active(false);

   if(_grabber)_grabber->grabStop();
}
void Item::willBeMovedFromStorageToWorld()
{
   super::willBeMovedFromStorageToWorld();

   actor.active(true);

   // when item is dropped down, then add random angular velocity
   Vec ang_vel=Random(Ball(1));
       ang_vel.setLength(Random.f(30, 50));
   actor.angVel(ang_vel); // set random angular velocity
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Item::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f<<scale;
      f.putAsset(base.id());
      f.putUInt (mesh ? mesh->variationID(mesh_variation) : 0);
      if(!actor.saveState(f))return false;
      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Item::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>scale;
         base=f.getAssetID();
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
