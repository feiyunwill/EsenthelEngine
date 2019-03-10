/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Chr::~Chr()
{
   grabStop();
}
Chr::Chr()
{
   action      =ACTION_NONE;
   move_walking=false;
   speed       = 4.1f;
   turn_speed  = 3.5f;
   flying_speed=25.0f;
   angle.zero();

   input_move.zero();
   input_turn.zero();

   dodging=0;
   dodge_step=0;
   dodge_availability=0;

   move_dir.zero();

   mesh_variation=0;

   ragdoll_mode=RAGDOLL_NONE;
   ragdoll_time=0;

   Zero(anim); anim.speed=speed/6; anim.unique=Random.f();
   Zero(sac );
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Chr::setUnsavedParams()
{
   mesh=(base ? base->mesh() : MeshPtr());
   if(!mesh)Exit("Attempting to create a character without Mesh");   
}
void Chr::create(Object &obj)
{
   Matrix matrix=obj.matrixFinal();
   Flt    scale =obj.scale      ();

   base          =obj.firstStored(); // copy pointer to object stored in project data
   mesh_variation=obj.meshVariationIndex();
   setUnsavedParams();

   // setup controller parameters
   Flt ctrl_height=mesh->ext.h()*scale,
       ctrl_radius=Max(0.4f, ctrl_height*0.25f);
   Vec ctrl_pos   =mesh->ext.pos*matrix;
   if(C PhysBodyPtr &phys=obj.phys()) // if 'Object' has 'PhysBody' specified, then take radius and height from the PhysBody capsule shape
      if(phys->is())
   {
      if(phys->parts.elms()==1 && phys->parts[0].type()==PHYS_SHAPE && phys->parts[0].shape.type==SHAPE_CAPSULE)
      {
       C Capsule &capsule=phys->parts[0].shape.capsule;
         ctrl_radius=capsule.r  *scale ;
         ctrl_height=capsule.h  *scale ;
         ctrl_pos   =capsule.pos*matrix;
      }else Exit("Character Controller can be created only from PhysBody Capsule");
   }

   // create character controller
   ctrl.createCapsule(ctrl_radius, ctrl_height, ctrl_pos, &matrix.pos)
       .actor.obj(this);

   // create skeleton
   skel.create(mesh->skeleton(), scale, matrix);
   sac.set(skel, base); // set default animations

   //
   angle.x=Angle(matrix.z.xz())-PI_2; // set initial horizontal angle
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Flt Chr::desiredSpeed() // get desired speed
{
   Flt    speed =(ctrl.flying() ? T.flying_speed : T.speed);
          speed*=Lerp(0.33f, 1.0f, anim.walk_run); // slow down when walking
   return speed;
}
/******************************************************************************/
Vec    Chr::pos   (          ) {return skel.pos   ();}
Matrix Chr::matrix(          ) {return skel.matrix();}
void   Chr::pos   (C Vec &pos)
{
   Vec delta=pos-T.pos(); // delta which moves from old position to new position

   skel.move(delta);                       // move skeleton
   ctrl.actor.pos(ctrl.actor.pos()+delta); // move controller
   ragdoll   .pos(ragdoll   .pos()+delta); // move ragdoll
}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Chr::memoryAddressChanged()
{
   ctrl.actor.obj(this);
   ragdoll   .obj(this);
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Chr::update()
{
   if(ragdoll_mode==RAGDOLL_FULL) // the character has ragdoll fully enabled, which most likely means that the character is dead, so update only skeleton basing on the ragdoll pose
   {
      ragdoll.toSkel(skel);
   }else
   {
      updateAction    (); // update automatic actions to set the input
      updateAnimation (); // update all animation parameters
      updateController(); // update character controller
   }

   skel.updateVelocities(); // update skeleton velocities

   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Chr::drawPrepare()
{
   if(mesh)if(Frustum(Ball().setAnimated(mesh->ext, skel)))
   {
      SetVariation(mesh_variation); mesh->draw(skel);
      SetVariation();
   }
   return 0; // no additional render modes required
}
void Chr::drawShadow()
{
   if(mesh)if(Frustum(Ball().setAnimated(mesh->ext, skel)))
   {
      SetVariation(mesh_variation); mesh->drawShadow(skel);
      SetVariation();
   }
}
/******************************************************************************/
// ENABLE / DISABLE
/******************************************************************************/
void Chr::disable()
{
   super::disable();

                ctrl.actor.kinematic(true);
   if(ragdoll_mode)ragdoll.kinematic(true);
}
void Chr::enable()
{
   super::enable();

                ctrl.actor.kinematic((ragdoll_mode==RAGDOLL_FULL) ? true : false); // if character is in full ragdoll mode, then controller should be set as kinematic
   if(ragdoll_mode)ragdoll.kinematic(false);
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Chr::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f<<move_walking<<speed<<turn_speed<<flying_speed<<angle;

      if(ctrl.save(f))
      if(skel.save(f))
      {
         f.putAsset(base.id());
         f.putUInt (mesh ? mesh->variationID(mesh_variation) : 0);
         f.putByte (ragdoll_mode);
         if(        ragdoll_mode)
         {
            f<<ragdoll_time;
            if(!ragdoll.saveState(f))return false;
         }

         f<<dodging; if(dodging)f<<dodge_step;
         f<<move_dir;
         f<<anim;

         f.putByte(action);
         if(action==ACTION_MOVE_TO)f<<path_target;
         return f.ok();
      }
   }
   return false;
}
/******************************************************************************/
Bool Chr::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>move_walking>>speed>>turn_speed>>flying_speed>>angle;

         if(ctrl.load(f))
         if(skel.load(f))
         {
            ctrl.actor.obj(this);

            base=f.getAssetID();
            setUnsavedParams();
            UInt mesh_variation_id=f.getUInt(); mesh_variation=(mesh ? mesh->variationFind(mesh_variation_id) : 0);
            sac.set(skel, base);
            if(ragdoll_mode=RAGDOLL_MODE(f.getByte()))
            {
                f>>ragdoll_time;
                   ragdollValidate  ();
               if(!ragdoll.loadState(f))return false; ragdoll.obj(this);
            }

            f>>dodging; if(dodging)f>>dodge_step;
            f>>move_dir;
            f>>anim;

               action =ACTION_TYPE(f.getByte());
            if(action==ACTION_MOVE_TO){Vec target; f>>target; actionMoveTo(target);} // again set the action to reset the paths

            if(f.ok())return true;
         }
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
