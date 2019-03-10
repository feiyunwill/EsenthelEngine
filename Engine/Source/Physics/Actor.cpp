/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Applying Flt damping; (0..1 range) each frame (correct formula) :

      actor.vel(actor.vel()*Pow(damping, Time.d()));
      OR
      actor.addVel(actor.vel()*(Pow(damping, Time.d())-1));


   Actor 'addVel at pos' and 'addAccel at pos' are not possible to perform.
   It's not the same as 'Actor.addForce*mass() at pos',
   Also remember that torque is calculated not based on mass but probably on inertia tensor.

/******************************************************************************/
#if PHYSX
/******************************************************************************

   SimulationFilterData(            group ,  ignore_id, PxPairFlag, 0) (also in 'Vehicle')
        QueryFilterData(IndexToFlag(group), vehicle_id,          0, 0) (also in 'Vehicle')

/******************************************************************************/
Flt     Actor::energy     (          )C {return _dynamic ? (inertia().avg()*angVel().length2() + mass()*vel().length2())*0.5f : 0;}
Flt     Actor:: damping   (          )C {return _dynamic ? _dynamic->getLinearDamping     () : 0;}
Flt     Actor::adamping   (          )C {return _dynamic ? _dynamic->getAngularDamping    () : 0;}
Flt     Actor::maxAngVel  (          )C {return _dynamic ? _dynamic->getMaxAngularVelocity() : 0;}
Flt     Actor::mass       (          )C {return _dynamic ? _dynamic->getMass              () : 0;}
Vec     Actor::massCenterL(          )C {return _dynamic ? Physx.vec   (_dynamic->getCMassLocalPose        ().p) : 0                   ;}
Vec     Actor::massCenterW(          )C {return massCenterL()*matrix();}
Vec     Actor::inertia    (          )C {return _dynamic ? Physx.vec   (_dynamic->getMassSpaceInertiaTensor()  ) : 0                   ;}
Vec     Actor::pos        (          )C {return _actor   ? Physx.vec   (_actor  ->getGlobalPose            ().p) : 0                   ;}
Matrix3 Actor::orn        (          )C {return _actor   ? Physx.orn   (_actor  ->getGlobalPose            ().q) : MatrixIdentity.orn();}
Matrix  Actor::matrix     (          )C {return _actor   ? Physx.matrix(_actor  ->getGlobalPose            ()  ) : MatrixIdentity      ;}
Vec     Actor::     vel   (          )C {return _dynamic ? Physx.vec   (_dynamic->getLinearVelocity        ()  ) : 0                   ;}
Vec     Actor::  angVel   (          )C {return _dynamic ? Physx.vec   (_dynamic->getAngularVelocity       ()  ) : 0                   ;}
Vec     Actor::pointVelL  (C Vec &pos)C {return vel() + Cross(angVel(), (pos-massCenterL())*orn());}
Vec     Actor::pointVelW  (C Vec &pos)C {return vel() + Cross(angVel(),  pos-massCenterW()       );}

Actor& Actor:: damping   (  Flt      damping) {if(_dynamic)_dynamic->setLinearDamping         (damping           ); return T;}
Actor& Actor::adamping   (  Flt      damping) {if(_dynamic)_dynamic->setAngularDamping        (damping           ); return T;}
Actor& Actor::maxAngVel  (  Flt      vel    ) {if(_dynamic)_dynamic->setMaxAngularVelocity    (vel               ); return T;}
Actor& Actor::mass       (  Flt      mass   ) {if(_dynamic)_dynamic->setMass                  (Max(EPS, mass    )); return T;} // may crash with zero
Actor& Actor::massCenterL(C Vec     &center ) {if(_dynamic){PxTransform t=_dynamic->getCMassLocalPose(); t.p=Physx.vec(center); _dynamic->setCMassLocalPose(t);} return T;}
Actor& Actor::massCenterW(C Vec     &center ) {return massCenterL(Vec(center).divNormalized(matrix()));}
Actor& Actor::inertia    (C Vec     &inertia) {if(_dynamic)_dynamic->setMassSpaceInertiaTensor(Physx.vec(Vec(Max(EPS, inertia.x), Max(EPS, inertia.y), Max(EPS, inertia.z)))); return T;} // may crash with zero
Actor& Actor::pos        (C Vec     &pos    ) {if(_actor  ){PxTransform t=_actor->getGlobalPose(); t.p=Physx.vec(pos); _actor->setGlobalPose(                   t);} return T;}
Actor& Actor::orn        (C Matrix3 &orn    ) {if(_actor  ){PxTransform t=_actor->getGlobalPose(); t.q=Physx.orn(orn); _actor->setGlobalPose(                   t);} return T;}
Actor& Actor::matrix     (C Matrix  &matrix ) {if(_actor  )                                                            _actor->setGlobalPose(Physx.matrix(matrix));  return T;}
Actor& Actor::   vel     (C Vec     &vel    ) {if(_dynamic && !kinematic())_dynamic->setLinearVelocity (Physx.vec(vel)); return T;} // PhysX will report an error if this is called for kinematic
Actor& Actor::angVel     (C Vec     &vel    ) {if(_dynamic && !kinematic())_dynamic->setAngularVelocity(Physx.vec(vel)); return T;} // PhysX will report an error if this is called for kinematic

Actor& Actor::kinematicMoveTo(C Vec     &pos   ) {if(_dynamic){PxTransform t; if(!_dynamic->getKinematicTarget(t))t=_dynamic->getGlobalPose(); t.p=Physx.vec(pos); _dynamic->setKinematicTarget(t                   );} return T;}
Actor& Actor::kinematicMoveTo(C Matrix3 &orn   ) {if(_dynamic){PxTransform t; if(!_dynamic->getKinematicTarget(t))t=_dynamic->getGlobalPose(); t.q=Physx.orn(orn); _dynamic->setKinematicTarget(t                   );} return T;}
Actor& Actor::kinematicMoveTo(C Matrix  &matrix) {if(_dynamic)                                                                                                     _dynamic->setKinematicTarget(Physx.matrix(matrix));  return T;}

// !! in the codes below require actor to be non-kinematic, because when applying forces to a dynamic object that was turned into kinematic, its mass will become infinite, and when disabling kinematic afterwards, it will be like a static actor !!
Actor& Actor::addAngVel (C Vec &ang_vel            ) {if(_dynamic && !kinematic())      _dynamic->addTorque    (           Physx.vec(ang_vel),                 PxForceMode::eVELOCITY_CHANGE); return T;}
Actor& Actor::addImpulse(C Vec &impulse            ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(impulse),                 PxForceMode::eIMPULSE        ); return T;}
Actor& Actor::addImpulse(C Vec &impulse, C Vec &pos) {if(_dynamic && !kinematic())PxRigidBodyExt::addForceAtPos(*_dynamic, Physx.vec(impulse), Physx.vec(pos), PxForceMode::eIMPULSE        ); return T;}
Actor& Actor::addVel    (C Vec &vel                ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(vel    ),                 PxForceMode::eVELOCITY_CHANGE); return T;}
#if 0 // we can't use 'eFORCE' because we manually manage simulation steps durations and need to make sure we specify correct time duration, also these methods can be called after each step completed AND after all steps completed, so we have to use 'Physics.time' which is set to 'stepTime' and 'updatedTime'
Actor& Actor::addTorque (C Vec &torque             ) {if(_dynamic && !kinematic())      _dynamic->addTorque    (           Physx.vec(torque ),                 PxForceMode::eFORCE          ); return T;}
Actor& Actor::addForce  (C Vec &force              ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(force  ),                 PxForceMode::eFORCE          ); return T;}
Actor& Actor::addForce  (C Vec &force  , C Vec &pos) {if(_dynamic && !kinematic())PxRigidBodyExt::addForceAtPos(*_dynamic, Physx.vec(force  ), Physx.vec(pos), PxForceMode::eFORCE          ); return T;}
Actor& Actor::addAccel  (C Vec &accel              ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(accel  ),                 PxForceMode::eACCELERATION   ); return T;}
#else
Actor& Actor::addTorque (C Vec &torque             ) {if(_dynamic && !kinematic())      _dynamic->addTorque    (           Physx.vec(torque*Physics.time()),                 PxForceMode::eIMPULSE        ); return T;}
Actor& Actor::addForce  (C Vec &force              ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(force *Physics.time()),                 PxForceMode::eIMPULSE        ); return T;}
Actor& Actor::addForce  (C Vec &force  , C Vec &pos) {if(_dynamic && !kinematic())PxRigidBodyExt::addForceAtPos(*_dynamic, Physx.vec(force *Physics.time()), Physx.vec(pos), PxForceMode::eIMPULSE        ); return T;}
Actor& Actor::addAccel  (C Vec &accel              ) {if(_dynamic && !kinematic())      _dynamic->addForce     (           Physx.vec(accel *Physics.time()),                 PxForceMode::eVELOCITY_CHANGE); return T;}
#endif

Actor& Actor::gravity  (Bool on)  {if(    _actor   && gravity  ()!=on){_actor  ->setActorFlag     (   PxActorFlag    ::eDISABLE_GRAVITY, !on); if( on)sleep(false);} return T;}
Actor& Actor::kinematic(Bool on)  {if(    _dynamic && kinematic()!=on){_dynamic->setRigidBodyFlag (   PxRigidBodyFlag::eKINEMATIC      ,  on); if(!on)sleep(false);} return T;}
Bool   Actor::kinematic(       )C {return _dynamic ?    FlagTest((UInt)_dynamic->getRigidBodyFlags(), PxRigidBodyFlag::eKINEMATIC           ) : false;}
Bool   Actor::gravity  (       )C {return _actor   ?   !FlagTest((UInt)_actor  ->getActorFlags    (), PxActorFlag    ::eDISABLE_GRAVITY     ) : true ;}
Bool   Actor::ccd      (       )C {return _dynamic ?    FlagTest((UInt)_dynamic->getRigidBodyFlags(), PxRigidBodyFlag::eENABLE_CCD          ) : false;}

Bool Actor::ray      ()C {if(_actor){PxShape *shape; if(_actor->getShapes(&shape, 1))return FlagTest((UInt)shape->getFlags(), PxShapeFlag::eSCENE_QUERY_SHAPE);} return false;}
Bool Actor::trigger  ()C {if(_actor){PxShape *shape; if(_actor->getShapes(&shape, 1))return FlagTest((UInt)shape->getFlags(), PxShapeFlag::eTRIGGER_SHAPE    );} return false;}
Bool Actor::collision()C {if(_actor){PxShape *shape; if(_actor->getShapes(&shape, 1))return FlagTest((UInt)shape->getFlags(), PxShapeFlag::eSIMULATION_SHAPE );} return false;}
Byte Actor::group    ()C {if(_actor){PxShape *shape; if(_actor->getShapes(&shape, 1))return                shape->getSimulationFilterData().word0             ;} return     0;}

Actor& Actor::ccd(Bool on)
{
   if(_dynamic)
   {
     _dynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, on);
      group(group()); // need to re-apply group because of 'eCCD_LINEAR' in 'SimulationFilterData'
   }
   return T;
}
Actor& Actor::ray(Bool on)
{
   if(_actor)for(Int offset=0, shapes=_actor->getNbShapes(); shapes>0; )
   {
      PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
      REP(_actor->getShapes(shape, s, offset))shape[i]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, on);
      shapes-=s;
      offset+=s;
   }
   return T;
}
Actor& Actor::trigger(Bool on)
{
   if(_actor)for(Int offset=0, shapes=_actor->getNbShapes(); shapes>0; )
   {
      PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
      REP(_actor->getShapes(shape, s, offset))
      {
         if(on)shape[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); // if enabling trigger, we need to first disable simulation, because PhysX will ignore setting trigger flag
               shape[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE   , on   );
      }
      shapes-=s;
      offset+=s;
   }
   return T;
}
Actor& Actor::collision(Bool on)
{
   if(_actor)for(Int offset=0, shapes=_actor->getNbShapes(); shapes>0; )
   {
      PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
      REP(_actor->getShapes(shape, s, offset))
      {
         if(on)shape[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE   , false); // if enabling collision, we need to first disable trigger, because PhysX will ignore setting collision flag
               shape[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, on   );
      }
      shapes-=s;
      offset+=s;
   }
   return T;
}
Actor& Actor::group(Byte group)
{
   if(_actor && InRange(group, AG_NUM)) // must always apply group even if applying the same one, because '_ignore_id' could be different (and 'qfd' needs to be set)
   {
      PxFilterData sfd(group, _ignore_id, ccd() ? PxPairFlag::eDETECT_CCD_CONTACT : 0, 0), qfd(IndexToFlag(group), 0, 0, 0);
      for(Int offset=0, shapes=_actor->getNbShapes(); shapes>0; )
      {
         PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
         REP(_actor->getShapes(shape, s, offset))
         {
            shape[i]->setSimulationFilterData(sfd);
            shape[i]->setQueryFilterData     (qfd);
         }
         shapes-=s;
         offset+=s;
      }
   }
   return T;
}

PhysMtrl* Actor::material()C
{
   if(_actor)
   {
      PxShape *shape; if(_actor->getShapes(&shape, 1))
      {
         PxMaterial *mtrl;
         if(shape->getMaterials(&mtrl, 1))if(mtrl)if(PhysMtrl *pm=(PhysMtrl*)mtrl->userData)if(pm!=&Physics.mtrl_default)return pm; // return default material always as null (we can use 'userData' because PhysMtrl's are stored in non-ref-counted cache and kept forever)
      }
   }
   return null;
}

Bool Actor::freezePosX()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_LINEAR_X) : false;}
Bool Actor::freezePosY()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_LINEAR_Y) : false;}
Bool Actor::freezePosZ()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_LINEAR_Z) : false;}
Bool Actor::freezePos ()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), UInt(PxRigidDynamicLockFlag::eLOCK_LINEAR_X | PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)) : false;}

Bool Actor::freezeRotX()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_ANGULAR_X) : false;}
Bool Actor::freezeRotY()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y) : false;}
Bool Actor::freezeRotZ()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z) : false;}
Bool Actor::freezeRot ()C {return _dynamic ? FlagTest((UInt)_dynamic->getRigidDynamicLockFlags(), UInt(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)) : false;}

Actor& Actor::freezePosX(Bool freeze)
{
   if(_dynamic)
   {
      PxRigidDynamicLockFlags flags=_dynamic->getRigidDynamicLockFlags(), new_flags=flags;
      FlagSet(new_flags, PxRigidDynamicLockFlag::eLOCK_LINEAR_X, freeze);
      if(flags!=new_flags)
      {
        _dynamic->setRigidDynamicLockFlags(new_flags); if(!freeze)sleep(false); // wake up on un-freeze
      }
   }
   return T;
}
Actor& Actor::freezePosY(Bool freeze)
{
   if(_dynamic)
   {
      PxRigidDynamicLockFlags flags=_dynamic->getRigidDynamicLockFlags(), new_flags=flags;
      FlagSet(new_flags, PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, freeze);
      if(flags!=new_flags)
      {
        _dynamic->setRigidDynamicLockFlags(new_flags); if(!freeze)sleep(false); // wake up on un-freeze
      }
   }
   return T;
}
Actor& Actor::freezePosZ(Bool freeze)
{
   if(_dynamic)
   {
      PxRigidDynamicLockFlags flags=_dynamic->getRigidDynamicLockFlags(), new_flags=flags;
      FlagSet(new_flags, PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freeze);
      if(flags!=new_flags)
      {
        _dynamic->setRigidDynamicLockFlags(new_flags); if(!freeze)sleep(false); // wake up on un-freeze
      }
   }
   return T;
}
Actor& Actor::freezePos(Bool freeze)
{
   if(_dynamic)
   {
      PxRigidDynamicLockFlags flags=_dynamic->getRigidDynamicLockFlags(), new_flags=flags;
      FlagSet(new_flags, PxRigidDynamicLockFlag::eLOCK_LINEAR_X | PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freeze);
      if(flags!=new_flags)
      {
        _dynamic->setRigidDynamicLockFlags(new_flags); if(!freeze)sleep(false); // wake up on un-freeze
      }
   }
   return T;
}

Actor& Actor::freezeRotX(Bool freeze) {if(_dynamic)_dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, freeze); return T;}
Actor& Actor::freezeRotY(Bool freeze) {if(_dynamic)_dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, freeze); return T;}
Actor& Actor::freezeRotZ(Bool freeze) {if(_dynamic)_dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freeze); return T;}
Actor& Actor::freezeRot (Bool freeze) {if(_dynamic)
{
   PxRigidDynamicLockFlags flags=_dynamic->getRigidDynamicLockFlags();
   FlagSet(flags, PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freeze);
  _dynamic->setRigidDynamicLockFlags(flags);
}return T;}

Ptr    Actor::user       ()C {return _actor ?      _actor->userData            : null ;}
Ptr    Actor::obj        ()C {return _actor ? (Ptr)_actor->getName          () : null ;}
Byte   Actor::dominance  ()C {return _actor ?      _actor->getDominanceGroup() : 0    ;}
Bool   Actor::sleep      ()C {return _dynamic ? _dynamic->isSleeping       () : false;}
Flt    Actor::sleepEnergy()C {return _dynamic ? _dynamic->getSleepThreshold() : 0    ;}
Actor& Actor::user       (Ptr    user     ) {if(_actor)_actor->userData         =      user ; return T;}
Actor& Actor::obj        (Ptr    obj      ) {if(_actor)_actor->setName          ((char*)obj); return T;}
Actor& Actor::dominance  (Byte   dominance) {if(_actor)_actor->setDominanceGroup( dominance); return T;}
Actor& Actor::sleep      (Bool   sleep    ) {if(_dynamic && !kinematic()){if(sleep)_dynamic->putToSleep();else _dynamic->wakeUp();} return T;} // PhysX will report an error if this is called for kinematic
Actor& Actor::sleepEnergy(Flt    energy   ) {if(_dynamic)_dynamic->setSleepThreshold(energy); return T;}
Actor& Actor::ignore     (Actor &actor, Bool ignore)
{
   if(T._actor && actor._actor && T._actor!=actor._actor)
   {
      if(ignore) // enable per-actor ignore
      {
         if(!_ignore_id || !actor._ignore_id) // if at least one doesn't have "ignore ID"
         {
            WriteLock lock(Physics._rws); // lock needed for 'ignore_id_gen'
            if(!      _ignore_id){UInt id=Physx.ignore_id_gen.New(); if(id<MAX_ACTOR_IGNORE){      _ignore_id=id;       group(      group());}else Physx.ignore_id_gen.Return(id);} // if ID is ok then apply it to member and filtering data (through 'group'), apply ONLY AFTER range verification in case filter func is running on secondary thread and could cause crash if it's invalid, if not ok then return it
            if(!actor._ignore_id){UInt id=Physx.ignore_id_gen.New(); if(id<MAX_ACTOR_IGNORE){actor._ignore_id=id; actor.group(actor.group());}else Physx.ignore_id_gen.Return(id);}
         }
      }
      if(_ignore_id && actor._ignore_id) // if both have "ignore ID"
      {
         FlagSet(Physx.ignoreMap(      _ignore_id, actor._ignore_id), 1<<(actor._ignore_id&7), ignore);
         FlagSet(Physx.ignoreMap(actor._ignore_id,       _ignore_id), 1<<(      _ignore_id&7), ignore);
      }
   }
   return T;
}

Box Actor::box()C {return _actor ? Physx.box(_actor->getWorldBounds()) : Box(0);}

Shape Actor::shape(Bool local)C
{
   Shape shape;
   if(_actor)
   {
      PxShape *s; if(_actor->getShapes(&s, 1))
      {
         switch(s->getGeometryType())
         {
            case PxGeometryType::ePLANE  : {PxPlaneGeometry   plane  ; if(s->getPlaneGeometry  (plane  )){shape.type=SHAPE_PLANE  ; shape.plane  .set(VecZero, Vec(1, 0, 0));}} break;
            case PxGeometryType::eSPHERE : {PxSphereGeometry  sphere ; if(s->getSphereGeometry (sphere )){shape.type=SHAPE_BALL   ; shape.ball   .set(sphere .radius);}} break;
            case PxGeometryType::eCAPSULE: {PxCapsuleGeometry capsule; if(s->getCapsuleGeometry(capsule)){shape.type=SHAPE_CAPSULE; shape.capsule.set(capsule.radius, (capsule.halfHeight+capsule.radius)*2, VecZero, Vec(1, 0, 0));}} break;
            case PxGeometryType::eBOX    : {PxBoxGeometry     box    ; if(s->getBoxGeometry    (box    )){shape.type=SHAPE_OBOX   ; shape.obox.box.set(Vec(-box.halfExtents.x, -box.halfExtents.y, -box.halfExtents.z), Vec(box.halfExtents.x, box.halfExtents.y, box.halfExtents.z)); shape.obox.matrix.identity();}} break;
         }
         if(shape.type)shape*=Physx.matrix(local ? s->getLocalPose() : _actor->getGlobalPose()*s->getLocalPose());
      }
   }
   return shape;
}
/******************************************************************************/
#else // BULLET
/******************************************************************************/
RigidBody::~RigidBody()
{
   REPA(ignore)if(RigidBody *rb=CAST(RigidBody, ignore[i]))rb->ignore.exclude(this); // remove self from other ignores
}
RigidBody::RigidBody(btRigidBody::btRigidBodyConstructionInfo &info) : btRigidBody(info)
{
   material=&Physics.mtrl_default; user=null; obj=null; offset_com.zero(); offset.identity(); setUserPointer(this);
}

Flt Actor::energy()C {return (inertia().avg()*angVel().length2() + mass()*vel().length2())*0.5f;}

Ptr    Actor::user(        )C {return _actor ? _actor->user : null        ;}
Actor& Actor::user(Ptr data)  {if(    _actor)  _actor->user=data; return T;}

Ptr    Actor::obj(       )C {return _actor ? _actor->obj : null       ;}
Actor& Actor::obj(Ptr obj)  {if(    _actor)  _actor->obj=obj; return T;}

Bool   Actor::gravity(       )C {return _actor ? !FlagTest(_actor->getFlags(), BT_DISABLE_WORLD_GRAVITY) : true;}
Actor& Actor::gravity(Bool on)
{
   if(_actor)
   {
      UInt flags=_actor->getFlags(); if(FlagTest(flags, BT_DISABLE_WORLD_GRAVITY)==on)
      {
         flags^=BT_DISABLE_WORLD_GRAVITY; _actor->setFlags(flags);
        _actor->setGravity(Bullet.vec(on ? Physics.gravity() : VecZero));
         if(on)sleep(false);
      }
   }
   return T;
}

Byte   Actor::group(          )C {return _actor ? _actor->getBroadphaseProxy()->m_collisionFilterGroup : 0;}
Actor& Actor::group(Byte group)  {if(_actor && InRange(group, AG_NUM))_actor->getBroadphaseProxy()->m_collisionFilterGroup=group; return T;}

Actor& Actor::ignore(Actor &actor, Bool ignore)
{
   if(T._actor && actor._actor && T._actor!=actor._actor)
   {
      if(ignore){T._actor->ignore.include(actor._actor); actor._actor->ignore.include(T._actor);}
      else      {T._actor->ignore.exclude(actor._actor); actor._actor->ignore.exclude(T._actor);}
   }
   return T;
}

Bool Actor::freezePosX()C {return _actor ?             _actor->getLinearFactor().x()==0 : false;}
Bool Actor::freezePosY()C {return _actor ?             _actor->getLinearFactor().y()==0 : false;}
Bool Actor::freezePosZ()C {return _actor ?             _actor->getLinearFactor().z()==0 : false;}
Bool Actor::freezePos ()C {return _actor ? !Bullet.vec(_actor->getLinearFactor()).all() : false;}

Bool Actor::freezeRotX()C {return _actor ?             _actor->getAngularFactor().x()==0 : false;}
Bool Actor::freezeRotY()C {return _actor ?             _actor->getAngularFactor().y()==0 : false;}
Bool Actor::freezeRotZ()C {return _actor ?             _actor->getAngularFactor().z()==0 : false;}
Bool Actor::freezeRot ()C {return _actor ? !Bullet.vec(_actor->getAngularFactor()).all() : false;}

Actor& Actor::freezePosX(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getLinearFactor(); if(Bool(factor.getX())==on) // if different
      {
         if(on)
         {
                               _actor->setLinearFactor  (btVector3(1, 0, 0));                   _actor->applyCentralForce(-_actor->getTotalForce()); // clear X force
            btVector3 velocity=_actor->getLinearVelocity(                  ); velocity.setX(0); _actor->setLinearVelocity(                velocity); // clear X velocity
         }else sleep(false); // wake up on un-freeze
         factor.setX(!on); _actor->setLinearFactor(factor);
      }
   }
   return T;
}
Actor& Actor::freezePosY(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getLinearFactor(); if(Bool(factor.getY())==on) // if different
      {
         if(on)
         {
                               _actor->setLinearFactor  (btVector3(0, 1, 0));                   _actor->applyCentralForce(-_actor->getTotalForce()); // clear Y force
            btVector3 velocity=_actor->getLinearVelocity(                  ); velocity.setY(0); _actor->setLinearVelocity(                velocity); // clear Y velocity
         }else sleep(false); // wake up on un-freeze
         factor.setY(!on); _actor->setLinearFactor(factor);
      }
   }
   return T;
}
Actor& Actor::freezePosZ(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getLinearFactor(); if(Bool(factor.getZ())==on) // if different
      {
         if(on)
         {
                               _actor->setLinearFactor  (btVector3(0, 0, 1));                   _actor->applyCentralForce(-_actor->getTotalForce()); // clear Z force
            btVector3 velocity=_actor->getLinearVelocity(                  ); velocity.setZ(0); _actor->setLinearVelocity(                velocity); // clear Z velocity
         }else sleep(false); // wake up on un-freeze
         factor.setZ(!on); _actor->setLinearFactor(factor);
      }
   }
   return T;
}
Actor& Actor::freezePos(Bool on)
{
   if(_actor)
   {
      btVector3 new_factor=(on ? btVector3(0, 0, 0) : btVector3(1, 1, 1)); if(_actor->getLinearFactor()!=new_factor) // if different
      {
         if(on)
         {
           _actor->setLinearFactor  (btVector3(1, 1, 1)); _actor->applyCentralForce(-_actor->getTotalForce()); // clear force
           _actor->setLinearVelocity(btVector3(0, 0, 0));                                                      // clear velocity
         }else sleep(false); // wake up on un-freeze
        _actor->setLinearFactor(new_factor);
      }
   }
   return T;
}

Actor& Actor::freezeRotX(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getAngularFactor();
      if(on)
      {
                            _actor->setAngularFactor  (btVector3(1, 0, 0));                   _actor->applyTorque       (-_actor->getTotalTorque()); // clear X torque
         btVector3 velocity=_actor->getAngularVelocity(                  ); velocity.setX(0); _actor->setAngularVelocity(                 velocity); // clear X velocity
      }
      factor.setX(!on); _actor->setAngularFactor(factor);
   }
   return T;
}
Actor& Actor::freezeRotY(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getAngularFactor();
      if(on)
      {
                            _actor->setAngularFactor  (btVector3(0, 1, 0));                   _actor->applyTorque       (-_actor->getTotalTorque()); // clear Y torque
         btVector3 velocity=_actor->getAngularVelocity(                  ); velocity.setY(0); _actor->setAngularVelocity(                 velocity); // clear Y velocity
      }
      factor.setY(!on); _actor->setAngularFactor(factor);
   }
   return T;
}
Actor& Actor::freezeRotZ(Bool on)
{
   if(_actor)
   {
      btVector3 factor=_actor->getAngularFactor();
      if(on)
      {
                            _actor->setAngularFactor  (btVector3(0, 0, 1));                   _actor->applyTorque       (-_actor->getTotalTorque()); // clear Z torque
         btVector3 velocity=_actor->getAngularVelocity(                  ); velocity.setZ(0); _actor->setAngularVelocity(                 velocity); // clear Z velocity
      }
      factor.setZ(!on); _actor->setAngularFactor(factor);
   }
   return T;
}
Actor& Actor::freezeRot(Bool on)
{
   if(_actor)
   {
      if(on)
      {
        _actor->setAngularFactor  (btVector3(1, 1, 1)); _actor->applyTorque(-_actor->getTotalTorque()); // clear torque
        _actor->setAngularVelocity(btVector3(0, 0, 0));                                                 // clear angular velocity
      }
     _actor->setAngularFactor(on ? btVector3(0, 0, 0) : btVector3(1, 1, 1));
   }
   return T;
}

Flt    Actor::     mass(           )C {if(_actor){Flt mass=_actor->getInvMass(); return mass ? 1/mass : 0;} return 0;}
Actor& Actor::     mass(Flt mass   )  {if(_actor)_actor->setMassProps(mass, Bullet.vec(inertia())); return T;}
Flt    Actor::  damping(           )C {return _actor ? _actor->getLinearDamping () : 0;}
Flt    Actor:: adamping(           )C {return _actor ? _actor->getAngularDamping() : 0;}
Flt    Actor::maxAngVel(           )C {return 0;}
Actor& Actor::  damping(Flt damping)  {if(_actor)_actor->setDamping(  damping  , T.adamping()); return T;}
Actor& Actor:: adamping(Flt damping)  {if(_actor)_actor->setDamping(T.damping(),    damping  ); return T;}
Actor& Actor::maxAngVel(Flt damping)  {return T;}
Bool   Actor::    sleep(           )C {return _actor ? !_actor->isActive() : false;}
Actor& Actor::    sleep(Bool on    )
{
   if(_actor)
   {
      if(!on)_actor->activate();else
      {
        _actor->setAngularVelocity(btVector3(0, 0, 0));
        _actor->setLinearVelocity (btVector3(0, 0, 0));
         // following code taken from "btDiscreteDynamicsWorld::updateActivationState"
         if(_actor->isStaticOrKinematicObject())
		   {
		      _actor->setActivationState(ISLAND_SLEEPING);
		   }else
		   {
		      if(_actor->getActivationState()==ACTIVE_TAG)_actor->setActivationState(WANTS_DEACTIVATION);
         }
      }
   }
   return T;
}
// TODO: check if the scale is OK vs PhysX, or maybe it needs to be dependent on mass?
#define BULLET_SLEEP_ENERGY_LINEAR  0.013f
#define BULLET_SLEEP_ENERGY_ANBULAR 0.02f
Flt    Actor::sleepEnergy(          )C {return _actor ? Sqr(_actor->getLinearSleepingThreshold())/BULLET_SLEEP_ENERGY_LINEAR : 0;} // to match PhysX scale
Actor& Actor::sleepEnergy(Flt energy)  {if(_actor)_actor->setSleepingThresholds(Sqrt(energy*BULLET_SLEEP_ENERGY_LINEAR), Sqrt(energy*BULLET_SLEEP_ENERGY_ANBULAR)); return T;} // to match PhysX scale
Bool   Actor::ccd        (          )C {return _actor ? _actor->getCcdMotionThreshold()!=0 : false;}
Actor& Actor::ccd        (Bool on   )
{
   if(_actor && on!=ccd())
   {
      if(on)
      {
        _actor->setCcdMotionThreshold  (0.01f);
        _actor->setCcdSweptSphereRadius(0.01f);
      }else
      {
        _actor->setCcdMotionThreshold  (0);
        _actor->setCcdSweptSphereRadius(0);
      }
   }
   return T;
}
Bool   Actor::kinematic(       )C {return _actor ? FlagTest(_actor->getCollisionFlags(), btCollisionObject::CF_KINEMATIC_OBJECT) : false;}
Actor& Actor::kinematic(Bool on)
{
   if(_actor)
   {
      UInt flags=_actor->getCollisionFlags(); if(FlagTest(flags, btCollisionObject::CF_KINEMATIC_OBJECT)!=on)
      {
         FlagToggle(flags, btCollisionObject::CF_KINEMATIC_OBJECT); _actor->setCollisionFlags(flags);
         if(on)
         { // clear velocities
               _actor->setAngularVelocity(btVector3(0, 0, 0));
               _actor->setLinearVelocity (btVector3(0, 0, 0));
               _actor->setActivationState  (DISABLE_DEACTIVATION);
         }else _actor->forceActivationState(ACTIVE_TAG);
      }
   }
   return T;
}
Bool   Actor::collision(       )C {return _actor ? !FlagTest(_actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE) : true;}
Actor& Actor::collision(Bool on)
{
   if(_actor)
   {
      UInt flags=_actor->getCollisionFlags();
      FlagSet(flags, btCollisionObject::CF_NO_CONTACT_RESPONSE, !on);
     _actor->setCollisionFlags(flags);
   }
   return T;
}
Bool   Actor::ray(       )C {return _actor ? _actor->getBroadphaseProxy()->m_collisionFilterMask!=0 : false;}
Actor& Actor::ray(Bool on)  {if(_actor)_actor->getBroadphaseProxy()->m_collisionFilterMask=on; return T;}

Bool Actor::trigger()C
{
   // TODO: Bullet
   return false;
}
Actor& Actor::trigger(Bool on)
{
   // TODO: Bullet
   return T;
}
Byte Actor::dominance()C
{
   // TODO: Bullet
   return 0;
}
Actor& Actor::dominance(Byte dominance)
{
   // TODO: Bullet
   return T;
}
PhysMtrl* Actor::material()C {return (_actor && _actor->material!=&Physics.mtrl_default) ? _actor->material : null;} // return default material always as null
void RigidBody::materialApply()
{
   setRestitution(material->bounciness());
   setFriction   (Avg(material->frictionStatic(), material->frictionDynamic()));
   setDamping    (material->damping(), material->adamping());
   // TODO: Bullet
 //setAnisotropicFriction
}

Box Actor::box()C
{
   if(_actor)
   {
      btVector3 min, max; _actor->getAabb(min, max);
      return Box(Bullet.vec(min), Bullet.vec(max));
   }
   return 0;
}
Shape Actor::shape(Bool local)C
{
   Shape shape;
   if(_actor)if(btCollisionShape *cs=_actor->getCollisionShape())
   {
      Bool   child_matrix_apply=false;
      Matrix child_matrix;
      if(btCompoundShape *compound=CAST(btCompoundShape, cs))
      {
         if(!compound->getNumChildShapes())cs=null;else
         {
            child_matrix_apply=true;
            child_matrix      =Bullet.matrix(compound->getChildTransform(0));
            cs                =              compound->getChildShape    (0) ;
         }
      }
      if(btBoxShape         *box    =CAST(btBoxShape        , cs)){Vec ext=Bullet.vec(box->getHalfExtentsWithMargin()); shape=Box(-ext, ext);}else
      if(btSphereShape      *ball   =CAST(btSphereShape     , cs)){shape=Ball(ball->getRadius());}else
      if(btCapsuleShape     *capsule=CAST(btCapsuleShape    , cs)){Flt r=capsule->getRadius(); shape=Capsule(r, (capsule->getHalfHeight()+r)*2);}else
      if(btCylinderShape    *tube   =CAST(btCylinderShape   , cs)){shape=Tube(tube->getRadius(), tube->getHalfExtentsWithMargin().y()*2);}else
      if(btStaticPlaneShape *plane  =CAST(btStaticPlaneShape, cs)){Vec n=Bullet.vec(plane->getPlaneNormal()); shape=Plane(plane->getPlaneConstant()*n, n);}
      if(shape.type)
      {
         if(child_matrix_apply)shape*=child_matrix;
         if(!local            )shape*=massCenterMatrix();else
         {
            Matrix temp; _actor->offset.inverse(temp, true); shape*=temp;
         }
      }
   }
   return shape;
}
Vec     Actor::pos   (          )C {return _actor ? _actor->offset.pos  *Bullet.matrix(_actor->getWorldTransform()           ) :                    0;}
Matrix3 Actor::orn   (          )C {return _actor ? _actor->offset.orn()*Bullet.matrix(_actor->getWorldTransform().getBasis()) : MatrixIdentity.orn();}
Matrix  Actor::matrix(          )C {return _actor ? _actor->offset      *Bullet.matrix(_actor->getWorldTransform()           ) : MatrixIdentity      ;}
Actor&  Actor::pos   (C Vec &pos)
{
   if(_actor)
   {
      // desired_matrix = offset * bullet_matrix
      // offset * bullet_matrix = desired_matrix
      // bullet_matrix = GetTransform(offset, desired_matrix) = ~offset * desired_matrix
      Matrix temp; _actor->offset.inverse(temp, true); temp.mul(orn()).move(pos);
     _actor->getWorldTransform().getOrigin()=Bullet.vec(temp.pos);
      if(!_actor->isActive() && Bullet.world)Bullet.world->updateSingleAabb(_actor);
      sleep(false); // needs to be woken up
      REP(_actor->getNumConstraintRefs())if(btTypedConstraint *joint=_actor->getConstraintRef(i)) // wake up all actors linked by joints
      {
         if(btRigidBody *rb=&joint->getRigidBodyA())if(rb!=_actor)rb->activate();
         if(btRigidBody *rb=&joint->getRigidBodyB())if(rb!=_actor)rb->activate();
      }
   }
   return T;
}
Actor& Actor::orn(C Matrix3 &orn)
{
   if(_actor)
   {
      // desired_matrix = offset * bullet_matrix
      // offset * bullet_matrix = desired_matrix
      // bullet_matrix = GetTransform(offset, desired_matrix) = ~offset * desired_matrix
      Matrix temp; _actor->offset.inverse(temp, true); temp.mul(orn).move(pos());
     _actor->getWorldTransform()=Bullet.matrix(temp);
      if(!_actor->isActive() && Bullet.world)Bullet.world->updateSingleAabb(_actor);
      sleep(false); // needs to be woken up
      REP(_actor->getNumConstraintRefs())if(btTypedConstraint *joint=_actor->getConstraintRef(i)) // wake up all actors linked by joints
      {
         if(btRigidBody *rb=&joint->getRigidBodyA())if(rb!=_actor)rb->activate();
         if(btRigidBody *rb=&joint->getRigidBodyB())if(rb!=_actor)rb->activate();
      }
   }
   return T;
}
Actor& Actor::matrix(C Matrix &matrix)
{
   if(_actor)
   {
      // desired_matrix = offset * bullet_matrix
      // offset * bullet_matrix = desired_matrix
      // bullet_matrix = GetTransform(offset, desired_matrix) = ~offset * desired_matrix
      Matrix temp; _actor->offset.inverse(temp, true); temp.mul(matrix);
     _actor->setWorldTransform(Bullet.matrix(temp));
      if(!_actor->isActive() && Bullet.world)Bullet.world->updateSingleAabb(_actor);
      sleep(false); // needs to be woken up
      REP(_actor->getNumConstraintRefs())if(btTypedConstraint *joint=_actor->getConstraintRef(i)) // wake up all actors linked by joints
      {
         if(btRigidBody *rb=&joint->getRigidBodyA())if(rb!=_actor)rb->activate();
         if(btRigidBody *rb=&joint->getRigidBodyB())if(rb!=_actor)rb->activate();
      }
   }
   return T;
}

Actor& Actor::kinematicMoveTo(C Vec     &pos   ) {return T.pos   (pos   );}
Actor& Actor::kinematicMoveTo(C Matrix3 &orn   ) {return T.orn   (orn   );}
Actor& Actor::kinematicMoveTo(C Matrix  &matrix) {return T.matrix(matrix);}

Vec    Actor::angVel(              )C {return _actor ? Bullet.vec(_actor->getAngularVelocity()) : 0;}
Vec    Actor::   vel(              )C {return _actor ? Bullet.vec(_actor->getLinearVelocity ()) : 0;}
Actor& Actor::angVel(C Vec &ang_vel)  {if(_actor){_actor->setAngularVelocity(_actor->getAngularFactor()*Bullet.vec(ang_vel)); if(sleep() && ang_vel.any())sleep(false);} return T;}
Actor& Actor::   vel(C Vec &    vel)  {if(_actor){_actor->setLinearVelocity (_actor->getLinearFactor ()*Bullet.vec(    vel)); if(sleep() &&     vel.any())sleep(false);} return T;}

Vec    Actor::pointVelL       (C Vec &pos)C {return _actor ? Bullet.vec(_actor->getVelocityInLocalPoint(Bullet.vec(pos*matrix()-massCenterW()))) : 0;}
Vec    Actor::pointVelW       (C Vec &pos)C {return _actor ? Bullet.vec(_actor->getVelocityInLocalPoint(Bullet.vec(pos         -massCenterW()))) : 0;}
Vec    Actor::massCenterW     (          )C {return _actor ? Bullet.vec(_actor->getCenterOfMassPosition())+_actor->offset_com : 0;}
Vec    Actor::massCenterL     (          )C {return massCenterW().divNormalized(matrix());}
Matrix Actor::massCenterMatrix(          )C {return _actor ? Bullet.matrix(_actor->getWorldTransform()) : MatrixIdentity;}

Vec Actor::inertia()C
{
   if(_actor)
   {
      Vec inertia=Bullet.vec(_actor->getInvInertiaDiagLocal());
      if( inertia.x)inertia.x=1/inertia.x;
      if( inertia.y)inertia.y=1/inertia.y;
      if( inertia.z)inertia.z=1/inertia.z;
      return inertia;
   }
   return 0;
}
Actor& Actor::inertia(C Vec &inertia) {if(_actor)_actor->setMassProps(mass(), Bullet.vec(inertia)); return T;}

Actor& Actor::massCenterL(C Vec &center) {return T;}
Actor& Actor::massCenterW(C Vec &center) {return T;}

Actor& Actor::addAngVel (C Vec &ang_vel            ) {if(_actor){_actor->setAngularVelocity (_actor->getAngularVelocity() + _actor->getAngularFactor()*Bullet.vec(ang_vel)); if(sleep() && ang_vel.any())sleep(false);} return T;}
Actor& Actor::addVel    (C Vec &vel                ) {if(_actor){_actor-> setLinearVelocity (_actor-> getLinearVelocity() + _actor-> getLinearFactor()*Bullet.vec(    vel)); if(sleep() &&     vel.any())sleep(false);} return T;}
Actor& Actor::addImpulse(C Vec &impulse            ) {if(_actor){_actor->applyCentralImpulse(Bullet.vec(impulse     )                                                     ); if(sleep() && impulse.any())sleep(false);} return T;}
Actor& Actor::addImpulse(C Vec &impulse, C Vec &pos) {if(_actor){_actor->applyImpulse       (Bullet.vec(impulse     ), Bullet.vec(pos-massCenterW())                      ); if(sleep() && impulse.any())sleep(false);} return T;}
#if 0 // we can't use Force because we manually manage simulation steps durations and need to make sure we specify correct time duration, also these methods can be called after each step completed AND after all steps completed, so we have to use 'Physics.time' which is set to 'stepTime' and 'updatedTime', also on Bullet forces get cleared at the end of simulation, so if we've added a force in the last step, it would be ignored
Actor& Actor::addTorque (C Vec &torque             ) {if(_actor){_actor->applyTorque        (Bullet.vec(torque      )                                                     ); if(sleep() && torque .any())sleep(false);} return T;}
Actor& Actor::addForce  (C Vec &force              ) {if(_actor){_actor->applyCentralForce  (Bullet.vec(force       )                                                     ); if(sleep() && force  .any())sleep(false);} return T;}
Actor& Actor::addForce  (C Vec &force  , C Vec &pos) {if(_actor){_actor->applyForce         (Bullet.vec(force       ), Bullet.vec(pos-massCenterW())                      ); if(sleep() && force  .any())sleep(false);} return T;}
Actor& Actor::addAccel  (C Vec &accel              ) {if(_actor){_actor->applyCentralForce  (Bullet.vec(accel*mass())                                                     ); if(sleep() && accel  .any())sleep(false);} return T;}
#else
Actor& Actor::addTorque (C Vec &torque             ) {if(_actor){_actor->applyTorqueImpulse (Bullet.vec(torque       *Physics.time())                                     ); if(sleep() && torque .any())sleep(false);} return T;}
Actor& Actor::addForce  (C Vec &force              ) {if(_actor){_actor->applyCentralImpulse(Bullet.vec(force        *Physics.time())                                     ); if(sleep() && force  .any())sleep(false);} return T;}
Actor& Actor::addForce  (C Vec &force  , C Vec &pos) {if(_actor){_actor->applyImpulse       (Bullet.vec(force        *Physics.time()), Bullet.vec(pos-massCenterW())      ); if(sleep() && force  .any())sleep(false);} return T;}
#if 0
Actor& Actor::addAccel  (C Vec &accel              ) {if(_actor){_actor->applyCentralImpulse(Bullet.vec(accel*(mass()*Physics.time()))                                    ); if(sleep() && accel  .any())sleep(false);} return T;}
#else
Actor& Actor::addAccel  (C Vec &accel              ) {return addVel(accel*Physics.time());}
#endif
#endif
/******************************************************************************/
#endif
/******************************************************************************/
Bool Actor::materialForce(PhysMtrl *material)
{
   if(_actor)
   {
      if(!material)material=&Physics.mtrl_default; // always set valid material
   #if PHYSX
      if(PxMaterial *m=material->_m)
      {
         for(Int offset=0, shapes=_actor->getNbShapes(); shapes>0; )
         {
            PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
            REP(_actor->getShapes(shape, s, offset))shape[i]->setMaterials(&m, 1);
            shapes-=s;
            offset+=s;
         }
         // leave opened brace
   #else
      {
        _actor->material=material;
        _actor->materialApply();
         // leave opened brace
   #endif
         damping(material->damping()).adamping(material->adamping());
         return true;
      }
   }
   return false;
}
Actor& Actor::material(PhysMtrl *material)
{
   if(_actor)
   {
                                       if(!material)material=&Physics.mtrl_default; // always set valid material
      PhysMtrl *old_mtrl=T.material(); if(!old_mtrl)old_mtrl=&Physics.mtrl_default; // always set valid material
      if(material!=old_mtrl && materialForce(material))
      {
         Flt old_density=old_mtrl->density(), new_density=material->density();
         if(!Equal(old_density, new_density) && old_density>EPS && new_density>EPS)
         {
            Flt mass=T.mass();
            if( mass>EPS)T.mass(mass*new_density/old_density);
         }
      }
   }
   return T;
}
/******************************************************************************/
Bool Actor::ignored(C Actor     &actor)C {return (T._actor && actor._actor) ? Physics.ignored(*T._actor, *actor._actor) : false;}
Bool Actor::ignored(C ActorInfo &actor)C {return (T._actor && actor._actor) ? Physics.ignored(*T._actor, *actor._actor) : false;}
/******************************************************************************/
#pragma pack(push, 1)
struct ActorDesc
{
   Byte   group, dominance;
   U16    flag;
   UInt   user;
 //Flt    mass, damping, adamping, sleep; these are ignored
   Vec    vel, ang_vel;
   Matrix matrix;
};
#pragma pack(pop)

Bool Actor::saveState(File &f)C
{
   f.cmpUIntV(0); // version

   ActorDesc desc;

   Unaligned(desc.group    , group    ());
   Unaligned(desc.dominance, dominance());
  _Unaligned(desc.user     , (UIntPtr)user());

 //Unaligned(desc.    mass,        mass());
 //Unaligned(desc. damping,     damping());
 //Unaligned(desc.adamping,    adamping());
 //Unaligned(desc.sleep   , sleepEnergy());

   Unaligned(desc.    vel,    vel());
   Unaligned(desc.ang_vel, angVel());
   Unaligned(desc. matrix, matrix());

   U16 flag=0;
   if(freezePosX())flag|=ACTOR_FREEZE_POS_X;
   if(freezePosY())flag|=ACTOR_FREEZE_POS_Y;
   if(freezePosZ())flag|=ACTOR_FREEZE_POS_Z;
   if(freezeRotX())flag|=ACTOR_FREEZE_ROT_X;
   if(freezeRotY())flag|=ACTOR_FREEZE_ROT_Y;
   if(freezeRotZ())flag|=ACTOR_FREEZE_ROT_Z;
   if(kinematic ())flag|=ACTOR_KINEMATIC;
   if(gravity   ())flag|=ACTOR_GRAVITY;
   if(ray       ())flag|=ACTOR_RAY;
   if(collision ())flag|=ACTOR_COLLISION;
   if(trigger   ())flag|=ACTOR_TRIGGER;
   if(sleep     ())flag|=ACTOR_SLEEP;
   if(ccd       ())flag|=ACTOR_CCD;
   Unaligned(desc.flag, flag);

   f.put(desc);
   return f.ok();
}
Bool Actor::loadState(File &f) // don't delete on fail, as here we're loading only state !! do not change 'obj' because that's a pointer, and game object classes expect that 'loadState' does not modify this parameter !!
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         ActorDesc desc; if(f.getFast(desc))
         {
            kinematic(FlagTest(Unaligned(desc.flag), ACTOR_KINEMATIC)); // !! set 'kinematic' as first, because it could disallow setting other members (such as 'vel, angVel, sleep') !!

            group    (     Unaligned(desc.group    ));
            dominance(     Unaligned(desc.dominance));
            user     ((Ptr)Unaligned(desc.user     ));

                 //mass(Unaligned(desc.    mass)); ignore these and take from 'PhysMtrl' and 'PhysBody'
              //damping(Unaligned(desc. damping));
             //adamping(Unaligned(desc.adamping));
          //sleepEnergy(Unaligned(desc.   sleep));

            matrix(Unaligned(desc. matrix));
               vel(Unaligned(desc.    vel));
            angVel(Unaligned(desc.ang_vel));

            freezePosX(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_POS_X));
            freezePosY(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_POS_Y));
            freezePosZ(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_POS_Z));
            freezeRotX(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_ROT_X));
            freezeRotY(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_ROT_Y));
            freezeRotZ(FlagTest(Unaligned(desc.flag), ACTOR_FREEZE_ROT_Z));
            gravity   (FlagTest(Unaligned(desc.flag), ACTOR_GRAVITY     ));
            ray       (FlagTest(Unaligned(desc.flag), ACTOR_RAY         ));
            collision (FlagTest(Unaligned(desc.flag), ACTOR_COLLISION   ));
            trigger   (FlagTest(Unaligned(desc.flag), ACTOR_TRIGGER     ));
            ccd       (FlagTest(Unaligned(desc.flag), ACTOR_CCD         ));

            sleep(FlagTest(Unaligned(desc.flag), ACTOR_SLEEP)); // !! set 'sleep' as last, because other methods may change it !!

            if(f.ok())return true;
         }
      }break;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
