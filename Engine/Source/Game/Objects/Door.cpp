/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Door::~Door()
{
}
Door::Door()
{
   scale=0;
   mesh_variation=0;
  _open =false;
  _state=0;
  _angle=PI;
  _actor_to_hinge_dist=0;
   Zero(_hinge_matrix);
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Door::setUnsavedParams()
{
   mesh=(base ? base->mesh() : MeshPtr());
   if(base && base->phys())actor.create(*base->phys(), 1, scale).obj(this);
}
void Door::create(Object &obj)
{
   scale         =obj.scale             ();
   base          =obj.firstStored       ();
   mesh_variation=obj.meshVariationIndex();
   setUnsavedParams();

  _hinge_matrix=obj.matrixFinal().normalize();
   if(C PhysBodyPtr &phys=obj.phys())
   {
      actor.matrix   (_hinge_matrix)
           .kinematic(true);

     _actor_to_hinge_dist=scale*phys->box.max.x;

     _hinge_matrix.pos+=_hinge_matrix.x*_actor_to_hinge_dist;

      joint.createHinge(actor, null, _hinge_matrix.pos, _hinge_matrix.y, -PI_2, PI_2);
   }
   setObstacle();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    Door::pos   () {return actor.pos   ();}
Matrix Door::matrix() {return actor.matrix();}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Door::memoryAddressChanged()
{
   actor.obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Door::update()
{
   if(_state==STATE_UNKNOWN)
   {
      Flt angle      =AngleNormalize(PI-AngleXZ(actor.pos(), _hinge_matrix)), // get angle of the actor position relative to the hinge position
          angle_delta=actor.angVel().y*Time.d();                              // current frame angle delta

      if(_open) // if want to open
      {
         if(angle+angle_delta<=-PI_2) // if angle exceeded opening limit value
         {
           _state=STATE_OPENED;
            Matrix temp=_hinge_matrix; temp.orn().rotateY(-PI_2); temp.pos-=temp.x*_actor_to_hinge_dist;
            actor.kinematic(true).kinematicMoveTo(temp); // freeze the actors and set its final opened position
            setObstacle();
         }else
         if(angle+angle_delta>=PI_2) // if angle exceeded opening limit value
         {
           _state=STATE_OPENED;
            Matrix temp=_hinge_matrix; temp.orn().rotateY(PI_2); temp.pos-=temp.x*_actor_to_hinge_dist;
            actor.kinematic(true).kinematicMoveTo(temp); // freeze the actors and set its final opened position
            setObstacle();
         }else
         {
            actor.addTorque(_hinge_matrix.y*SignBool(angle>0));
         }
      }else // want to close
      {
         if(Sign(T._angle)!=Sign(angle+angle_delta)) // angle=0 means door in closed position, so check if the new angle has a different sign than the old angle, which means that 0 was crossed
         {
           _state=STATE_CLOSED;
            actor.kinematic(true).kinematicMoveTo(Matrix(_hinge_matrix).move(_hinge_matrix.x*-_actor_to_hinge_dist));
            setObstacle();
         }else
         {
            actor.addTorque(_hinge_matrix.y*SignBool(angle<0));
         }
      }
      T._angle=angle; // store new angle
   }
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Door::drawPrepare()
{
   if(mesh && actor.is())
   {
      Matrix matrix=actor.matrix().scaleOrn(scale);
      if(Frustum(*mesh, matrix))
      {
         SetVariation(mesh_variation); mesh->draw(matrix, actor.vel(), actor.angVel());
         SetVariation();
      }
   }
   return 0; // no additional render modes required
}
void Door::drawShadow()
{
   if(mesh && actor.is())
   {
      Matrix matrix=actor.matrix().scaleOrn(scale);
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
void Door::disable() {super::disable(); actor.kinematic(true                 );}
void Door:: enable() {super:: enable(); actor.kinematic(_state!=STATE_UNKNOWN);}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Door::open()
{
   if(!_open)
   {
     _open =true;
     _state=STATE_UNKNOWN;
      actor.kinematic(false);
   }
}
/******************************************************************************/
void Door::close()
{
   if(_open)
   {
     _open =false;
     _state=STATE_UNKNOWN;
      actor.kinematic(false);
   }
}
/******************************************************************************/
void Door::toggle()
{
   if(_open)close();
   else     open ();
}
/******************************************************************************/
void Door::setObstacle()
{
   if(mesh && actor.is())
   {
      Matrix matrix=actor.matrix().scaleOrn(scale);
      obstacle.create(OBox(mesh->ext, matrix), World.path());
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Door::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f<<scale<<_open<<_state<<_angle<<_actor_to_hinge_dist<<_hinge_matrix;
      f.putAsset(base.id());
      f.putUInt (mesh ? mesh->variationID(mesh_variation) : 0);

      if(!actor.saveState(f))return false;
      if(!joint.save     (f))return false;

      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Door::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>scale>>_open>>_state>>_angle>>_actor_to_hinge_dist>>_hinge_matrix;
         base=f.getAssetID();
         setUnsavedParams();
         UInt mesh_variation_id=f.getUInt(); mesh_variation=(mesh ? mesh->variationFind(mesh_variation_id) : 0);

         if(!actor.loadState(f))return false;
         if(!joint.load     (f, actor, null))return false;

         setObstacle();
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
