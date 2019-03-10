/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum JOINT_TYPE : Byte
{
   JOINT_NONE     ,
   JOINT_FIXED    ,
   JOINT_HINGE    ,
   JOINT_SPHERICAL,
   JOINT_SLIDER   ,
   JOINT_DISTANCE ,
};
/******************************************************************************/
#if PHYSX
   #define BOUNCE  0.1f
   #define DAMPING 0.5f*0
   #define SPRING  1.0f*0

/*struct SphericalJoint : PxJoint, PxConstraintConnector
{
	static const PxU32 TYPE_ID = PxConcreteType::eFIRST_USER_EXTENSION;

   // PxJoint
	virtual void				setActors(PxRigidActor* actor0, PxRigidActor* actor1)override {}
	virtual void				getActors(PxRigidActor*& actor0, PxRigidActor*& actor1)const override {}
	virtual void				setLocalPose(PxJointActorIndex::Enum actor, const PxTransform& localPose)override {}
	virtual PxTransform		getLocalPose(PxJointActorIndex::Enum actor)const override {}
	virtual PxTransform		getRelativeTransform()const override {}
	virtual PxVec3				getRelativeLinearVelocity()const	override {}
	virtual PxVec3				getRelativeAngularVelocity()const override {}
	virtual void				setBreakForce(PxReal force, PxReal torque)override {}
	virtual void				getBreakForce(PxReal& force, PxReal& torque)const override {}
	virtual void				setConstraintFlags(PxConstraintFlags flags)override {}
	virtual void				setConstraintFlag(PxConstraintFlag::Enum flag, bool value)override {}
	virtual PxConstraintFlags	getConstraintFlags()const override {}
	virtual void				setInvMassScale0(PxReal invMassScale)override {}
	virtual PxReal				getInvMassScale0()const override {}
	virtual void				setInvInertiaScale0(PxReal invInertiaScale) override {}
	virtual PxReal				getInvInertiaScale0()const override {}
	virtual void				setInvMassScale1(PxReal invMassScale)override {}
	virtual PxReal				getInvMassScale1()const override {}
	virtual void				setInvInertiaScale1(PxReal invInertiaScale)override {}
	virtual PxReal				getInvInertiaScale1()const override {}
	virtual PxConstraint*	getConstraint()const	override {}
	virtual void				setName(const char* name)override {}
	virtual const char*		getName()const override {}
	virtual void				release()override {}
	virtual PxScene*			getScene()const override {}

   // PxConstraintConnector
	virtual void*			prepareData()override {}
	virtual bool			updatePvdProperties(physx::pvdsdk::PvdDataStream& pvdConnection, const PxConstraint* c, PxPvdUpdateType::Enum updateType)const override {}
	virtual void			onConstraintRelease()override {}
	virtual void			onComShift(PxU32 actor)override {}
	virtual void			onOriginShift(const PxVec3& shift)override {}
	virtual void*			getExternalReference(PxU32& typeID)override {}
	virtual PxBase* getSerializable()override {}
	virtual PxConstraintSolverPrep getPrep()const override {}
	virtual const void* getConstantBlock()const override {}
};*/
#else
struct btDistanceConstraint : btPoint2PointConstraint
{
   Bool spring;
   Flt  min_dist, max_dist, spring_spring, spring_damper;

   btDistanceConstraint(btRigidBody& rbA, const btVector3& pivotInA, Flt min_dist, Flt max_dist, Bool spring, Flt spring_spring, Flt spring_damper) : btPoint2PointConstraint(rbA, pivotInA)
   {
      T.min_dist     =min_dist     ;
      T.max_dist     =max_dist     ;
      T.spring       =spring       ;
      T.spring_spring=spring_spring;
      T.spring_damper=spring_damper;
   }

   btDistanceConstraint(btRigidBody& rbA, btRigidBody& rbB, const btVector3& pivotInA, const btVector3& pivotInB, Flt min_dist, Flt max_dist, Bool spring, Flt spring_spring, Flt spring_damper) : btPoint2PointConstraint(rbA, rbB, pivotInA, pivotInB)
   {
      T.min_dist     =min_dist     ;
      T.max_dist     =max_dist     ;
      T.spring       =spring       ;
      T.spring_spring=spring_spring;
      T.spring_damper=spring_damper;
   }

   virtual void getInfo1(btConstraintInfo1 *info)
   {
      info->m_numConstraintRows=1;
      info->nub=5;
   }
   virtual void getInfo2(btConstraintInfo2 *info)
   {
      btVector3 relA = m_rbA.getCenterOfMassTransform().getBasis() * getPivotInA();
      btVector3 relB = m_rbB.getCenterOfMassTransform().getBasis() * getPivotInB();
      btVector3 posA = m_rbA.getCenterOfMassTransform().getOrigin() + relA;
      btVector3 posB = m_rbB.getCenterOfMassTransform().getOrigin() + relB;
      btVector3 del = posB - posA;
      btScalar currDist = btSqrt(del.dot(del));
      btVector3 ortho = (currDist ? del / currDist : btVector3(0, 0, 0));
      info->m_J1linearAxis[0] = ortho[0];
      info->m_J1linearAxis[1] = ortho[1];
      info->m_J1linearAxis[2] = ortho[2];
      btVector3 p, q;
      p = relA.cross(ortho);
      q = relB.cross(ortho);
      info->m_J1angularAxis[0] = p[0];
      info->m_J1angularAxis[1] = p[1];
      info->m_J1angularAxis[2] = p[2];
      info->m_J2angularAxis[0] = -q[0];
      info->m_J2angularAxis[1] = -q[1];
      info->m_J2angularAxis[2] = -q[2];
      btScalar rhs = (currDist - 0/*m_distance*/) * info->fps * info->erp * (spring ? spring_spring*0.01f : 1);
      info->m_constraintError[0] = rhs;
      info->cfm[0] = btScalar(0.f);
      info->m_lowerLimit[0] = -SIMD_INFINITY;
      info->m_upperLimit[0] = SIMD_INFINITY;
   }
};
#endif
/******************************************************************************/
Joint& Joint::del()
{
   if(_joint)
   {
      SafeWriteLock lock(Physics._rws);
      if(_joint)
      {
      #if PHYSX
         if(Physx.world)_joint->release(); _joint=null;
      #else
         if(_joint->needsFeedback())Bullet.breakables.exclude(_joint); // remove from breakable list
         if(Bullet.world)Bullet.world->removeConstraint(_joint); // remove from world
         Delete(_joint);
      #endif
      }
   }
   return T;
}
/******************************************************************************/
static Bool ValidActors(Actor &a0, Actor *a1) // PhysX will still create joint for null actors, and creating for static only will even create a crash
{
   return a0.is() && (a1 ? a1->is() : true) // make sure that both exist
   #if PHYSX
      && (a0.isDynamic() || (a1 && a1->isDynamic())) // for PhysX make sure that at least one is dynamic
   #endif
   ;
}
/******************************************************************************/
Joint& Joint::create(Actor &a0, Actor *a1)
{
   WriteLock lock(Physics._rws);

   del();

#if PHYSX
   if(Physx.world && ValidActors(a0, a1))
      if(PxFixedJoint *fixed=PxFixedJointCreate(*Physx.physics, a0._actor, PxTransform(PxIdentity), a1 ? a1->_actor : null, Physx.matrix(a1 ? a0.matrix().divNormalized(a1->matrix()) : a0.matrix()))) // when specifying world matrix use current a0 world position, when specifying local frame for a1 use current a0 position in a1 space
   {
      fixed->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
     _joint=fixed;
   }
#else
   if(a0._actor)
   {
      btGeneric6DofConstraint *dof=null;
      if(a1 && a1->_actor)
      {
         dof=new btGeneric6DofConstraint(*a0._actor, *a1->_actor, btTransform::getIdentity(), Bullet.matrix(a0.massCenterMatrix()/a1->massCenterMatrix()), false);
      }else
      {
         dof=new btGeneric6DofConstraint(*a0._actor, btTransform::getIdentity(), false);
      }
      dof->setAngularLowerLimit(btVector3(0, 0, 0));
      dof->setAngularUpperLimit(btVector3(0, 0, 0));
      dof->setLinearLowerLimit (btVector3(0, 0, 0));
      dof->setLinearUpperLimit (btVector3(0, 0, 0));
      if(_joint=dof)
      {
         Flt f=FLT_MAX;
        _joint->setUserConstraintId((Int&)f);
        _joint->setUserConstraintType(0);
         if(Bullet.world)Bullet.world->addConstraint(_joint, true);
      }
   }
#endif
   return T;
}
/******************************************************************************/
static void CreateHinge(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], Bool limit_angle, Bool body, Flt angle_min, Flt angle_max, Bool collision)
{
#if PHYSX
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxRevoluteJoint *hinge=PxRevoluteJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      hinge->setConstraintFlag   (PxConstraintFlag   ::eCOLLISION_ENABLED, collision  );
      hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED    , limit_angle);
      PxJointAngularLimitPair limit(-angle_max, -angle_min); if(body){limit.restitution=BOUNCE; limit.stiffness=SPRING; limit.damping=DAMPING;} hinge->setLimit(limit);
      joint._joint=hinge;
   }
#else
   if(a0._actor)
   {
      Matrix m0; m0.pos=local_anchor[0]; m0.z=local_axis[0]; m0.x=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
      Matrix m1; m1.pos=local_anchor[1]; m1.z=local_axis[1]; m1.x=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
      btHingeConstraint *hinge=null;
      if(a1 && a1->_actor)
      {
         hinge=new btHingeConstraint(*a0._actor, *a1->_actor, Bullet.matrix(m0), Bullet.matrix(m1));
      }else
      {
         hinge=new btHingeConstraint(*a0._actor, Bullet.matrix(m0)); hinge->getBFrame()=Bullet.matrix(m1);
      }
      if(joint._joint=hinge)
      {
         Flt f=FLT_MAX;
         hinge->setUserConstraintId((Int&)f);
         hinge->setUserConstraintType(0);
         if(limit_angle)hinge->setLimit(angle_min, angle_max);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(hinge, !collision);
      }
   }
#endif
}
/******************************************************************************/
static void CreateSpherical(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], Bool collision, Bool limit_swing, Bool limit_twist, Bool body, Flt swing, Flt twist)
{
#if PHYSX
#if 0 // PxSphericalJoint
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxSphericalJoint *spherical=PxSphericalJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      spherical->setConstraintFlag    (PxConstraintFlag    ::eCOLLISION_ENABLED  , collision  );
      spherical->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED      , limit_swing);
      spherical->setSphericalJointFlag(PxSphericalJointFlag::eTWIST_LIMIT_ENABLED, limit_twist);
      Clamp(swing, EPS, PI-EPS);
      Clamp(twist, EPS, PI-EPS);
      PxJointLimitCone limit(swing, swing); limit.twist=twist; if(body){limit.restitution=BOUNCE; limit.stiffness=SPRING; limit.damping=DAMPING;}
      spherical->setLimitCone(limit);
      joint._joint=spherical;
   }
#else // PxD6Joint
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxD6Joint *spherical=PxD6JointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      spherical->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED  , collision  );
	   spherical->setMotion        (PxD6Axis::eSWING1, limit_swing ? PxD6Motion::eLIMITED : PxD6Motion::eFREE);
	   spherical->setMotion        (PxD6Axis::eSWING2, limit_swing ? PxD6Motion::eLIMITED : PxD6Motion::eFREE);
	   spherical->setMotion        (PxD6Axis::eTWIST , limit_twist ? PxD6Motion::eLIMITED : PxD6Motion::eFREE);
      if(limit_swing)
      {
         Clamp(swing, EPS, PI-EPS);
         PxJointLimitCone limit(swing, swing); if(body){limit.restitution=BOUNCE; limit.stiffness=SPRING; limit.damping=DAMPING;}
         spherical->setSwingLimit(limit);
      }
      if(limit_twist)
      {
         Clamp(twist, 0, PI);
         PxJointAngularLimitPair limit(-twist, twist); if(body){limit.restitution=BOUNCE; limit.stiffness=SPRING; limit.damping=DAMPING;}
         spherical->setTwistLimit(limit);
      }
      joint._joint=spherical;
   }
#endif
#else
   if(a0._actor)
   {
      if(!limit_swing && !limit_twist)
      {
         btPoint2PointConstraint *p2p=null;
         if(a1 && a1->_actor)
         {
            p2p=new btPoint2PointConstraint(*a0._actor, *a1->_actor, Bullet.vec(local_anchor[0]*a0._actor->offset), Bullet.vec(local_anchor[1]*a1->_actor->offset));
         }else
         {
            p2p=new btPoint2PointConstraint(*a0._actor, Bullet.vec(local_anchor[0]*a0._actor->offset)); p2p->setPivotB(Bullet.vec(local_anchor[1]));
         }
         if(joint._joint=p2p)
         {
            Flt f=FLT_MAX;
            p2p->setUserConstraintId((Int&)f);
            p2p->setUserConstraintType(0);

            WriteLock lock(Physics._rws);
            if(Bullet.world)Bullet.world->addConstraint(p2p, !collision);
         }
      }else
      {
         Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.z=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
         Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.z=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
         btConeTwistConstraint *cone=null;
         if(a1 && a1->_actor)
         {
            cone=new btConeTwistConstraint(*a0._actor, *a1->_actor, Bullet.matrix(m0), Bullet.matrix(m1));
         }else
         {
            cone=new btConeTwistConstraint(*a0._actor, Bullet.matrix(m0)); cone->getBFrame()=Bullet.matrix(m1);
         }
         if(joint._joint=cone)
         {
            Flt f=FLT_MAX;
            cone->setUserConstraintId((Int&)f);
            cone->setUserConstraintType(0);
            cone->setLimit(limit_swing ? swing : FLT_MAX, limit_swing ? swing : FLT_MAX, limit_twist ? twist : FLT_MAX);

            WriteLock lock(Physics._rws);
            if(Bullet.world)Bullet.world->addConstraint(cone, !collision);
         }
      }
   }
#endif
}
/******************************************************************************/
static void CreateSlider(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], Flt min, Flt max, Bool collision)
{
#if PHYSX
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxPrismaticJoint *slider=PxPrismaticJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      slider->setConstraintFlag    (PxConstraintFlag    ::eCOLLISION_ENABLED, collision);
      slider->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED    , true     );
      PxJointLinearLimitPair limit(Physx.physics->getTolerancesScale(), -max, -min); slider->setLimit(limit);
      joint._joint=slider;
   }
#else
   if(a0._actor)
   {
      Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.z=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
      Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.z=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
      btSliderConstraint *slider=null;
      if(a1 && a1->_actor)
      {
         slider=new btSliderConstraint(*a1->_actor, *a0._actor, Bullet.matrix(m1), Bullet.matrix(m0), true); // btSliderConstraint(bodyA, bodyB), because constructor below accepts B, to match order we need to swap here actor order
      }else
      {
         slider=new btSliderConstraint(*a0._actor, Bullet.matrix(m0), true); // btSliderConstraint(bodyB), this accepts B only
         slider->setFrames(Bullet.matrix(m1), Bullet.matrix(m0));
      }
      if(joint._joint=slider)
      {
         Flt f=FLT_MAX;
         slider->setUserConstraintId((Int&)f);
         slider->setUserConstraintType(0);
         slider->setLowerLinLimit(min);
         slider->setUpperLinLimit(max);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(slider, !collision);
      }
   }
#endif
}
/******************************************************************************/
static void CreateDistance(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], Flt min, Flt max, Bool collision, Bool spring, Flt spring_spring, Flt spring_damper)
{
#if PHYSX
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxDistanceJoint *distance=PxDistanceJointCreate(*Physx.physics, a0._actor, Physx.matrix(local_anchor[0]), a1 ? a1->_actor : null, Physx.matrix(local_anchor[1])))
   {
      distance->setConstraintFlag   (PxConstraintFlag::eCOLLISION_ENABLED, collision);
      distance->setMinDistance      (min);
      distance->setMaxDistance      (max);
      distance->setStiffness        (spring_spring);
      distance->setDamping          (spring_damper);
      distance->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true  );
      distance->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true  );
      distance->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED      , spring);
      joint._joint=distance;
   }
#else
   if(a0._actor)
   {
      btDistanceConstraint *dist=null;
      if(a1 && a1->_actor)
      {
         dist=new btDistanceConstraint(*a0._actor, *a1->_actor, Bullet.vec(local_anchor[0]*a0._actor->offset), Bullet.vec(local_anchor[1]*a1->_actor->offset), min, max, spring, spring_spring, spring_damper);
      }else
      {
         dist=new btDistanceConstraint(*a0._actor, Bullet.vec(local_anchor[0]*a0._actor->offset), min, max, spring, spring_spring, spring_damper); dist->setPivotB(Bullet.vec(local_anchor[1]));
      }
      if(joint._joint=dist)
      {
         Flt f=FLT_MAX;
         dist->setUserConstraintId((Int&)f);
         dist->setUserConstraintType(0);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(dist, !collision);
      }
   }
#endif
}
/******************************************************************************/
Joint& Joint::createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Bool collision)
{
   del();
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(axis  ).divNormalized(m0.orn()), Vec(axis  ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateHinge(T, a0, a1, local_anchor, local_axis, local_normal, false, false, 0, 0, collision);
   return T;
}
Joint& Joint::createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Flt min_angle, Flt max_angle, Bool collision)
{
   del();
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(axis  ).divNormalized(m0.orn()), Vec(axis  ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateHinge(T, a0, a1, local_anchor, local_axis, local_normal, true, false, min_angle, max_angle, collision);
   return T;
}
Joint& Joint::createSpherical(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Flt *swing, Flt *twist, Bool collision)
{
   del();
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(axis  ).divNormalized(m0.orn()), Vec(axis  ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateSpherical(T, a0, a1, local_anchor, local_axis, local_normal, collision, swing!=null, twist!=null, false, swing ? *swing : 0, twist ? *twist : 0);
   return T;
}
Joint& Joint::createSliding(Actor &a0, Actor *a1, C Vec &anchor, C Vec &dir, Flt min, Flt max, Bool collision)
{
   del();
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(dir),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(dir   ).divNormalized(m0.orn()), Vec(dir   ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateSlider(T, a0, a1, local_anchor, local_axis, local_normal, min, max, collision);
   return T;
}
Joint& Joint::createDist(Actor &a0, Actor *a1, C Vec &anchor0, C Vec &anchor1, Flt min, Flt max, Spring *spring, Bool collision)
{
   del();
   Vec local_anchor[]={anchor0, anchor1};
   CreateDistance(T, a0, a1, local_anchor, min, max, collision, spring!=null, spring ? spring->spring : 0, spring ? spring->damping : 0);
   return T;
}
Joint& Joint::createBodyHinge(Actor &bone, Actor &parent, C Vec &anchor, C Vec &axis, Flt min_angle, Flt max_angle)
{
   del();
   Matrix m0            =bone  .matrix(),
          m1            =parent.matrix();
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(axis  ).divNormalized(m0.orn()), Vec(axis  ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateHinge(T, bone, &parent, local_anchor, local_axis, local_normal, true, true, min_angle, max_angle, false);
   return T;
}
Joint& Joint::createBodySpherical(Actor &bone, Actor &parent, C Vec &anchor, C Vec &axis, Flt swing, Flt twist)
{
   del();
   Matrix m0            =bone  .matrix(),
          m1            =parent.matrix();
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec(anchor).divNormalized(m0      ), Vec(anchor).divNormalized(m1      )},
          local_axis  []={Vec(axis  ).divNormalized(m0.orn()), Vec(axis  ).divNormalized(m1.orn())},
          local_normal[]={Vec(normal).divNormalized(m0.orn()), Vec(normal).divNormalized(m1.orn())};
   CreateSpherical(T, bone, &parent, local_anchor, local_axis, local_normal, false, true, true, true, swing, twist);
   return T;
}
/******************************************************************************/
Matrix Joint::localAnchor(Bool index)C
{
#if PHYSX
   if(_joint)return Physx.matrix(_joint->getLocalPose(index ? PxJointActorIndex::eACTOR1 : PxJointActorIndex::eACTOR0));
#endif
   return MatrixIdentity;
}
void Joint::changed()
{
#if PHYSX
   if(_joint)if(PxConstraint *constraint=_joint->getConstraint())constraint->markDirty();
#endif
}
/******************************************************************************/
// BREAKABLE
/******************************************************************************/
Bool Joint::broken()C
{
#if PHYSX
   return _joint ? FlagTest((UInt)_joint->getConstraintFlags(), PxConstraintFlag::eBROKEN) : true;
#else
   return _joint ? _joint->getUserConstraintType()!=0 : true;
#endif
}
/******************************************************************************/
Joint& Joint::breakable(Flt max_force, Flt max_torque)
{
   if(_joint)
   {
   #if PHYSX
     _joint->setBreakForce((max_force <0) ? PX_MAX_REAL : max_force ,
                           (max_torque<0) ? PX_MAX_REAL : max_torque);
   #else
      Flt f;
      if(max_force >=FLT_MAX)max_force =-1; // need to adjust to -1 because depending on <0 we include/exclude in breakable list
      if(max_torque>=FLT_MAX)max_torque=-1; // need to adjust to -1 because depending on <0 we include/exclude in breakable list
      if(max_force>=0 || max_torque>=0)
      {
         Bullet.breakables.include(_joint); // add to breakable list
        _joint->enableFeedback(true);
         if(max_force>=0 && max_torque>=0)f=Avg(max_force, max_torque);else
         if(max_force>=0                 )f=    max_force             ;else
         if(                max_torque>=0)f=    max_torque            ;
      }else
      {
         Bullet.breakables.exclude(_joint); // remove from breakable list
        _joint->enableFeedback(false);
         f=FLT_MAX;
      }
     _joint->setUserConstraintId((Int&)f);
   #endif
   }
   return T;
}
/******************************************************************************/
// HINGE
/******************************************************************************/
Flt Joint::hingeAngle()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getAngle();
#endif
   return 0;
}
Flt Joint::hingeVel()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getVelocity();
#endif
   return 0;
}
Bool Joint::hingeDriveEnabled()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return FlagTest((UInt)hinge->getRevoluteJointFlags(), PxRevoluteJointFlag::eDRIVE_ENABLED);
#endif
   return false;
}
Joint& Joint::hingeDriveEnabled(Bool on)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, on);
#endif
   return T;
}
Bool Joint::hingeDriveFreeSpin()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return FlagTest((UInt)hinge->getRevoluteJointFlags(), PxRevoluteJointFlag::eDRIVE_FREESPIN);
#endif
   return false;
}
Joint& Joint::hingeDriveFreeSpin(Bool on)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, on);
#endif
   return T;
}
Flt Joint::hingeDriveVel()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveVelocity();
#endif
   return 0;
}
Joint& Joint::hingeDriveVel(Flt vel)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveVelocity(vel);
#endif
   return T;
}
Flt Joint::hingeDriveForceLimit()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveForceLimit();
#endif
   return 0;
}
Joint& Joint::hingeDriveForceLimit(Flt limit)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveForceLimit(limit);
#endif
   return T;
}
Flt Joint::hingeDriveGearRatio()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveGearRatio();
#endif
   return 0;
}
Joint& Joint::hingeDriveGearRatio(Flt ratio)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveGearRatio(ratio);
#endif
   return T;
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Joint::save(File &f)C
{
   f.cmpUIntV(0); // version

   JOINT_TYPE type=JOINT_NONE;
   
   if(_joint && !broken())
   {
   #if PHYSX
      switch(_joint->getConcreteType())
      {
         case PxJointConcreteType::eFIXED    : type=JOINT_FIXED    ; break;
         case PxJointConcreteType::eREVOLUTE : type=JOINT_HINGE    ; break;
         case PxJointConcreteType::ePRISMATIC: type=JOINT_SLIDER   ; break;
         case PxJointConcreteType::eDISTANCE : type=JOINT_DISTANCE ; break;

         case PxJointConcreteType::eD6       :
         case PxJointConcreteType::eSPHERICAL: type=JOINT_SPHERICAL; break;
      }
   #else
      if(CAST(btDistanceConstraint   , _joint))type=JOINT_DISTANCE ;else // this must be checked as first, because 'btDistanceConstraint' extends 'btPoint2PointConstraint' checked below
      if(CAST(btGeneric6DofConstraint, _joint))type=JOINT_FIXED    ;else
      if(CAST(btHingeConstraint      , _joint))type=JOINT_HINGE    ;else
      if(CAST(btPoint2PointConstraint, _joint))type=JOINT_SPHERICAL;else
      if(CAST(btConeTwistConstraint  , _joint))type=JOINT_SPHERICAL;else
      if(CAST(btSliderConstraint     , _joint))type=JOINT_SLIDER   ;
   #endif
   }

   // save joint type
   f.putByte(type);

   // save joint data
   switch(type)
   {
      case JOINT_HINGE:
      {
         Vec  anchor[2], axis[2], normal[2];
         Bool collision, limit_angle, body;
         Flt  angle_min, angle_max;
      #if PHYSX
         PxRevoluteJoint &hinge=*_joint->is<PxRevoluteJoint>();
         Matrix m0=Physx.matrix(hinge.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(hinge.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointAngularLimitPair limit=hinge.getLimit();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision  =FlagTest((UInt)hinge.getConstraintFlags   (), PxConstraintFlag   ::eCOLLISION_ENABLED);
         limit_angle=FlagTest((UInt)hinge.getRevoluteJointFlags(), PxRevoluteJointFlag::eLIMIT_ENABLED    );
         body       =(limit.restitution>0 || limit.stiffness>0);
         angle_min  =-limit.upper;
         angle_max  =-limit.lower;
      #else
         btHingeConstraint *hinge=CAST(btHingeConstraint, _joint);
         Matrix m=Bullet.matrix(hinge->getAFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.z; normal[0]=m.y;
                m=Bullet.matrix(hinge->getBFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.z; normal[1]=m.y;
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
         body     =false;
         angle_min=hinge->getLowerLimit();
         angle_max=hinge->getUpperLimit();
         limit_angle=(angle_min<=angle_max);
      #endif
         f<<anchor<<axis<<normal<<collision<<limit_angle<<body;
         if(limit_angle)f<<angle_min<<angle_max;
         f<<hingeDriveEnabled()<<hingeDriveFreeSpin()<<hingeDriveVel()<<hingeDriveForceLimit()<<hingeDriveGearRatio();
      }break;

      case JOINT_SPHERICAL:
      {
         Vec  anchor[2], axis[2], normal[2];
         Bool collision, limit_swing, limit_twist, body;
         Flt  swing, twist;
      #if PHYSX
      #if 0
         PxSphericalJoint &spherical=*_joint->is<PxSphericalJoint>();
         Matrix m0=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointLimitCone limit=spherical.getLimitCone();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision  =FlagTest((UInt)spherical.getConstraintFlags    (), PxConstraintFlag    ::eCOLLISION_ENABLED  );
         limit_swing=FlagTest((UInt)spherical.getSphericalJointFlags(), PxSphericalJointFlag::eLIMIT_ENABLED      );
         limit_twist=FlagTest((UInt)spherical.getSphericalJointFlags(), PxSphericalJointFlag::eTWIST_LIMIT_ENABLED);
         body       =(limit.restitution>0 || limit.stiffness>0);
         swing      =limit.yAngle;
         twist      =limit.twistAngle;
      #else
         PxD6Joint &spherical=*_joint->is<PxD6Joint>();
	      PxJointLimitCone        swing_limit=spherical.getSwingLimit();
         PxJointAngularLimitPair twist_limit=spherical.getTwistLimit();
         Matrix m0=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR1));
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision  =FlagTest((UInt)spherical.getConstraintFlags(), PxConstraintFlag::eCOLLISION_ENABLED);
         limit_swing=(spherical.getMotion(PxD6Axis::eSWING1)!=PxD6Motion::eFREE);
         limit_twist=(spherical.getMotion(PxD6Axis::eTWIST )!=PxD6Motion::eFREE);
         body       =(swing_limit.restitution>0 || swing_limit.stiffness>0);
         swing      = swing_limit.yAngle;
         twist      = twist_limit.upper;
      #endif
      #else
         if(btPoint2PointConstraint *p2p=CAST(btPoint2PointConstraint, _joint))
         {
            anchor[0]=Bullet.vec(p2p->getPivotInA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[0].divNormalized(rb->offset); axis[0].set(0, 0, 1); normal[0].set(1, 0, 0);
            anchor[1]=Bullet.vec(p2p->getPivotInB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))anchor[1].divNormalized(rb->offset); axis[1].set(0, 0, 1); normal[1].set(1, 0, 0);
            limit_swing=limit_twist=false;
         }else
         {
            btConeTwistConstraint *cone=CAST(btConeTwistConstraint, _joint);
            Matrix m=Bullet.matrix(cone->getAFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.x; normal[0]=m.z;
                   m=Bullet.matrix(cone->getBFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.x; normal[1]=m.z;
            swing=cone->getSwingSpan1(); limit_swing=(swing!=FLT_MAX);
            twist=cone->getTwistSpan (); limit_twist=(twist!=FLT_MAX);
         }
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
         body     =false;
      #endif
         f<<anchor<<axis<<normal<<collision<<limit_swing<<limit_twist<<body;
         if(limit_swing)f<<swing;
         if(limit_twist)f<<twist;
      }break;

      case JOINT_SLIDER:
      {
         Vec  anchor[2], axis[2], normal[2];
         Flt  min, max;
         Bool collision;
      #if PHYSX
         PxPrismaticJoint &slider=*_joint->is<PxPrismaticJoint>();
         Matrix m0=Physx.matrix(slider.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(slider.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointLinearLimitPair limit=slider.getLimit();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision=FlagTest((UInt)slider.getConstraintFlags(), PxConstraintFlag::eCOLLISION_ENABLED);
         min      =-limit.upper;
         max      =-limit.lower;
      #else
         btSliderConstraint *slider=CAST(btSliderConstraint, _joint);
         Matrix m=Bullet.matrix(slider->getFrameOffsetA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.x; normal[1]=m.z;
                m=Bullet.matrix(slider->getFrameOffsetB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.x; normal[0]=m.z;
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
         min=slider->getLowerLinLimit();
         max=slider->getUpperLinLimit();
      #endif
         f<<anchor<<axis<<normal<<min<<max<<collision;
      }break;

      case JOINT_DISTANCE:
      {
         Vec  anchor[2];
         Flt  min, max;
         Bool collision, spring;
         Flt  spring_spring, spring_damper;
      #if PHYSX
         PxDistanceJoint &distance=*_joint->is<PxDistanceJoint>();
         anchor[0]    =Physx.vec(distance.getLocalPose(PxJointActorIndex::eACTOR0).p);
         anchor[1]    =Physx.vec(distance.getLocalPose(PxJointActorIndex::eACTOR1).p);
         collision    =FlagTest((UInt)distance.getConstraintFlags   (), PxConstraintFlag   ::eCOLLISION_ENABLED);
         spring       =FlagTest((UInt)distance.getDistanceJointFlags(), PxDistanceJointFlag::eSPRING_ENABLED   );
         min          =distance.getMinDistance();
         max          =distance.getMaxDistance();
         spring_spring=distance.getStiffness();
         spring_damper=distance.getDamping  ();
	   #else
         btDistanceConstraint *dist=CAST(btDistanceConstraint, _joint);
         anchor[0]    =Bullet.vec(dist->getPivotInA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[0].divNormalized(rb->offset); 
         anchor[1]    =Bullet.vec(dist->getPivotInB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[1].divNormalized(rb->offset); 
         min          =dist->min_dist;
         max          =dist->max_dist;
         spring       =dist->spring;
         spring_spring=dist->spring_spring;
         spring_damper=dist->spring_damper;
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
	   #endif
	      f<<anchor<<min<<max<<collision<<spring;
	      if(spring)f<<spring_spring<<spring_damper;
      }break;
   }

   // save breakable data
   if(type)
   {
      Flt max_force, max_torque;
   #if PHYSX
     _joint->getBreakForce(max_force, max_torque);
   #else
      Int force=_joint->getUserConstraintId(); max_force=max_torque=(Flt&)force;
   #endif
      f<<max_force<<max_torque;
   }
   return f.ok();
}
/******************************************************************************/
Bool Joint::load(File &f, Actor &a0, Actor *a1)
{
   del();

   switch(f.decUIntV()) // version
   {
      case 0:
      {
         JOINT_TYPE type=JOINT_TYPE(f.getByte());
         switch(    type)
         {
            default         : goto error; // unknown joint type
            case JOINT_NONE :                 break;
            case JOINT_FIXED: create(a0, a1); break;

            case JOINT_HINGE:
            {
               Vec  anchor[2], axis[2], normal[2];
               Bool collision, limit_angle, body;
               Flt  angle_min, angle_max;
               f>>anchor>>axis>>normal>>collision>>limit_angle>>body;
               if(limit_angle)f>>angle_min>>angle_max;

               CreateHinge(T, a0, a1, anchor, axis, normal, limit_angle, body, angle_min, angle_max, collision);
               hingeDriveEnabled(f.getBool()); hingeDriveFreeSpin(f.getBool()); hingeDriveVel(f.getFlt()); hingeDriveForceLimit(f.getFlt()); hingeDriveGearRatio(f.getFlt());
            }break;

            case JOINT_SPHERICAL:
            {
               Vec  anchor[2], axis[2], normal[2];
               Bool collision, limit_swing, limit_twist, body;
               Flt  swing, twist;
               f>>anchor>>axis>>normal>>collision>>limit_swing>>limit_twist>>body;
               if(limit_swing)f>>swing;
               if(limit_twist)f>>twist;

               CreateSpherical(T, a0, a1, anchor, axis, normal, collision, limit_swing, limit_twist, body, swing, twist);
            }break;

            case JOINT_SLIDER:
            {
               Vec  anchor[2], axis[2], normal[2];
               Flt  min, max;
               Bool collision;
	            f>>anchor>>axis>>normal>>min>>max>>collision;

               CreateSlider(T, a0, a1, anchor, axis, normal, min, max, collision);
            }break;

            case JOINT_DISTANCE:
            {
               Vec  anchor[2];
               Flt  min, max;
               Bool collision, spring;
               Flt  spring_spring, spring_damper;
	            f>>anchor>>min>>max>>collision>>spring;
	            if(spring)f>>spring_spring>>spring_damper;

               CreateDistance(T, a0, a1, anchor, min, max, collision, spring, spring_spring, spring_damper);
            }break;
         }

         // load breakable data
         if(type)
         {
            Flt max_force, max_torque; f>>max_force>>max_torque; breakable(max_force, max_torque);
         }

         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
