/******************************************************************************/
#include "stdafx.h"
#define MAX_CONTACTS 32
namespace EE{
#include "PhysX Stream.h"
/******************************************************************************/
PhysicsClass Physics;
#if PHYSX
PhysxClass Physx;
#else
BulletClass Bullet;
#endif
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
static Flt WheelFriction(C PhysMtrl &ground_material, C PhysMtrl &wheel_material, C ActorInfo &vehicle, WHEEL_TYPE wheel)
{
   return 1.0f;
}
/******************************************************************************/
#if PHYSX
/******************************************************************************/
PxU32 OutputStream::write(const void* src, PxU32 count)
{
   if(pos+count>max)Realloc(data, max=pos+count+8192, pos);
   CopyFast(data+pos, src, count); pos+=count;
   return count;
}
PxU32 InputStream::read(void* dest, PxU32 count)
{
   Int read=Min(count, size-pos);
   CopyFast(dest, data+pos, read); pos+=read;
   return read;
}
/******************************************************************************/
static struct ErrorCallbackClass : PxErrorCallback
{
   virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)override
   {
      if(code&PxErrorCode::eABORT)Exit(S+"Physx("+code+"): "+message);
   #if DEBUG
      if(!Equal(message, "RigidBody::setRigidBodyFlag: kinematic bodies with CCD enabled are not supported! CCD will be ignored.")
      )LogN(S+"Physx("+code+"): "+message);
   #endif
   }
}ErrorCallback;

Ptr PhysxClass::AllocatorCallback::allocate(size_t size, const char *typeName, const char *filename, int line)
{
	Ptr    ptr=super::allocate(size, typeName, filename, line);
   if(    ptr)AtomicInc(Physx._mem_leaks);
   return ptr;
}
void PhysxClass::AllocatorCallback::deallocate(void *ptr)
{
   if(ptr)
   {
      AtomicDec(Physx._mem_leaks);
      super::deallocate(ptr);
   }
}

#if !USE_DEFAULT_CPU_DISPATCHER
static struct CpuDispatcherClass : PxCpuDispatcher // !!!!!!!!!!!!! if PhysX crashes here, then most likely invalid data was put into Actors (0, NaN, +-Inf, etc.) or other physics classes !!!!!!!!
{
#if HAS_THREADS
   Threads threads;
#endif

  ~CpuDispatcherClass() {del();}
   void                  del()
   {
   #if HAS_THREADS
      threads.wait(); // make sure all are processed because we need to release the 'PxBaseTask'
   #endif
   }
   CpuDispatcherClass& create()
   {
      del();
   #if HAS_THREADS
      threads.create(false, Cpu.threads()-1, 0, "EE.PhysX #"); // leave 1 thread for the main update, TODO: we could reuse any other 'Threads'
   #endif
      return T;
   }
	virtual PxU32 getWorkerCount()C override
   {
   #if HAS_THREADS
      return threads.threads();
   #else
      return 0;
   #endif
   }
    static void ProcessTask(PxBaseTask &task, Ptr user, Int thread_index) {task.run(); task.release();}
   virtual void  submitTask(PxBaseTask &task)override
   {
   #if HAS_THREADS
      threads.queue(task, ProcessTask);
   #else
      task.run(); task.release();
   #endif
   }
}CpuDispatcher;
#else
static PxDefaultCpuDispatcher *CpuDispatcher;
#endif

static PxFilterFlags FilterFunc(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags &pairFlags, const void *constantBlock, PxU32 constantBlockSize)
{
	// check triggers
	if((attributes0 | attributes1) & PxFilterObjectFlag::eTRIGGER)
	{
		pairFlags=(PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST);
		return PxFilterFlags();
	}

	// per-group filtering
	if(!Physics.collides(filterData0.word0, filterData1.word0))return PxFilterFlag::eSUPPRESS;
	// per-actor filtering
	if(filterData0.word1 && filterData1.word1 && (Physx.ignoreMap(filterData0.word1, filterData1.word1)&(1<<(filterData1.word1&7))))return PxFilterFlag::eSUPPRESS;

	// set default values
	pairFlags=PxPairFlags(PxPairFlag::eCONTACT_DEFAULT | filterData0.word2 | filterData1.word2); // word2 has encoded PxPairFlags (for example 'ccd')

	// check for collision reports
	if(Physics.reports(filterData0.word0, filterData1.word0))pairFlags|=(PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_CONTACT_POINTS);

	return PxFilterFlags();
}

static PxQueryHitType::Enum VehicleWheelRaycastFilterFunc(PxFilterData vehicle_susp, PxFilterData shape_fd, const void* constantBlock, PxU32 constantBlockSize, PxHitFlags& filterFlags)
{
   return (vehicle_susp.word1==shape_fd.word1) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK; // ignore actors of same actor_id
}

struct EventCallbackClass : PxSimulationEventCallback
{
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)override {}
	virtual void onWake(PxActor** actors, PxU32 count)override {}
	virtual void onSleep(PxActor** actors, PxU32 count)override {}
	virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)override {}
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)override
	{
      void (*func)(ActorInfo &actor_a, ActorInfo &actor_b, C PhysContact *contact, Int contacts)=Physics._report_contact; // assign to temporary variable in order to avoid multi-threading issues
      if(func)
	      if(!(pairHeader.flags&(PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1))) // if both still exist
            if(PxRigidActor *actor0=pairHeader.actors[0]->is<PxRigidActor>())
            if(PxRigidActor *actor1=pairHeader.actors[1]->is<PxRigidActor>())
	   {
         PxShape *shape0, *shape1;
         if(actor0->getShapes(&shape0, 1))
         if(actor1->getShapes(&shape1, 1))
         {
            ActorInfo   actor_a, actor_b; actor_a.set(shape0); actor_b.set(shape1);
            PhysContact contact[MAX_CONTACTS]; Int contacts=0;
	         REP(nbPairs)
	         {
	          C PxContactPair &pair=pairs[i];
	            if(pair.flags&PxContactPairFlag::eINTERNAL_HAS_IMPULSES) // we can ignore 'eREMOVED_SHAPE_0' and 'eREMOVED_SHAPE_1' because we're just collecting pos/normal/force
	            {
	               PxContactPairPoint px_contact[MAX_CONTACTS]; Int px_contacts=pair.extractContacts(px_contact, Elms(px_contact));
	               REP(px_contacts)
	               {
	                  if(InRange(contacts, contact))
	                  {
	                     contact[contacts].pos   =Physx.vec(px_contact[i].position);
	                     contact[contacts].normal=Physx.vec(px_contact[i].normal  );
	                     contact[contacts].force =Physx.vec(px_contact[i].impulse ); if(Physics.stepTime())contact[contacts].force/=Physics.stepTime(); // according to PhysX doc, impulse should be divided by simulation step to get force
	                     contacts++;
	                  }
	               }
	            }
	         }
            func(actor_a, actor_b, contact, contacts);
         }
	   }
	}
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)override
	{
      void (*func)(ActorInfo &trigger, ActorInfo &actor, PHYS_CONTACT contact)=Physics._report_trigger; // assign to temporary variable in order to avoid multi-threading issues
      if(func)FREP(count)
      {
       C PxTriggerPair &pair=pairs[i];
         if(!(pair.flags&(PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))) // if both still exist
         {
            PHYS_CONTACT contact;
            if(pair.status&PxPairFlag::eNOTIFY_TOUCH_LOST )contact=CONTACT_LOST;else
            if(pair.status&PxPairFlag::eNOTIFY_TOUCH_FOUND)contact=CONTACT_NEW ;else
                                                           contact=CONTACT_KEEP;
            ActorInfo trigger, actor; trigger.set(pair.triggerShape); actor.set(pair.otherShape); func(trigger, actor, contact);
         }
      }
	}
}EventCallback;
/******************************************************************************/
#else // BULLET
/******************************************************************************/
static struct OverlapFilterCallbackClass : btOverlapFilterCallback
{
   virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)const override
   {
      return Physics.collides(proxy0->m_collisionFilterGroup, proxy1->m_collisionFilterGroup);
   }
}OverlapFilterCallback;

STRUCT(CollisionDispatcher , btCollisionDispatcher)
//{
   bool needsCollision(btCollisionObject* body0, btCollisionObject* body1)
   {
      if(super::needsCollision(body0, body1))
      {
         // test pair<->pair
         if(RigidBody *rb=(RigidBody*)body0->getUserPointer())if(rb->ignore.has(body1))return false;
         return true;
      }
      return false;
   }

	CollisionDispatcher(btCollisionConfiguration* collisionConfiguration) : btCollisionDispatcher(collisionConfiguration) {}
};
/******************************************************************************/
#endif
/******************************************************************************/
// PHYSICS
/******************************************************************************/
#if PHYSX
ASSERT(SIZE(Quaternion)==SIZE(PxQuat));
ASSERT(SIZE(Matrix3   )==SIZE(PxMat33));
Matrix3 PhysxClass::orn(C PxQuat &quat)
{
   Quaternion q; q.set(quat.x, quat.y, quat.z, -quat.w); return q;
}
Matrix3 PhysxClass::orn(C PxMat33 &mat)
{
   return (Matrix3&)mat;
}
PxQuat PhysxClass::orn(C Matrix3 &matrix)
{
   PxQuat q; (Quaternion&)q=matrix;
   CHS(q.w);
   return q;
}
Matrix PhysxClass::matrix(C PxTransform &t)
{
   Matrix     m; m.pos.set(t.p.x, t.p.y, t.p.z);
   Quaternion q; q.    set(t.q.x, t.q.y, t.q.z, -t.q.w); m.orn()=q;
   return     m;
}
PxTransform PhysxClass::matrix(C Matrix &matrix)
{
   PxTransform t;
   t.p.x=matrix.pos.x;
   t.p.y=matrix.pos.y;
   t.p.z=matrix.pos.z;
   (Quaternion&)t.q=matrix;
   CHS(t.q.w);
   return t;
}
#else // BULLET
Matrix3 BulletClass::matrix(C btMatrix3x3 &t)
{
   Matrix3 m;
   m.x=vec(t.getColumn(0));
   m.y=vec(t.getColumn(1));
   m.z=vec(t.getColumn(2));
   return m;
}
Matrix BulletClass::matrix(C btTransform &t)
{
   Matrix m;
   m.pos=vec(t.getOrigin());
   m.x  =vec(t.getBasis ().getColumn(0));
   m.y  =vec(t.getBasis ().getColumn(1));
   m.z  =vec(t.getBasis ().getColumn(2));
   return m;
}
btMatrix3x3 BulletClass::matrix(C Matrix3 &m)
{
   btMatrix3x3 t;
   t.setValue(m.x.x, m.y.x, m.z.x,
              m.x.y, m.y.y, m.z.y,
              m.x.z, m.y.z, m.z.z);
   return t;
}
btTransform BulletClass::matrix(C Matrix &m)
{
   btTransform t;
   t.getOrigin()=vec(m.pos);
   t.getBasis ().setValue(m.x.x, m.y.x, m.z.x,
                          m.x.y, m.y.y, m.z.y,
                          m.x.z, m.y.z, m.z.z);
   return t;
}
#endif
/******************************************************************************/
/*PhysicsClass& PhysicsClass::filter(Bool on)
{
   if(Physx.world)
   {
      Physx.world->setFilterOps (NX_FILTEROP_OR, NX_FILTEROP_OR, NX_FILTEROP_AND);
      Physx.world->setFilterBool(true);
      NxGroupsMask mask; mask.bits0=mask.bits1=mask.bits2=mask.bits3=!on;
      Physx.world->setFilterConstant0(mask);
      Physx.world->setFilterConstant1(mask);
   }
   return T;
}*/
UInt PhysicsClass::collisionGroups(Byte group)C
{
   return InRange(group, AG_NUM) ? _collision_array[group] : 0;
}
PhysicsClass& PhysicsClass::ignore(Int group_a, Int group_b, Bool ignore)
{
   if(created() && InRange(group_a, AG_NUM) && InRange(group_b, AG_NUM))
   {
      FlagSet(_collision_array[group_a], 1<<group_b, !ignore);
      FlagSet(_collision_array[group_b], 1<<group_a, !ignore);
   }
   return T;
}
PhysicsClass& PhysicsClass::reportContact(Int group_a, Int group_b, Bool report)
{
#if PHYSX
   if(created() && InRange(group_a, AG_NUM) && InRange(group_b, AG_NUM))
   {
     _contact_report[group_a][group_b]=report;
     _contact_report[group_b][group_a]=report;
   }
#endif
   return T;
}
PhysicsClass& PhysicsClass::reportContact(void (*ReportContact)(ActorInfo &actor_a, ActorInfo &actor_b, C PhysContact *contact, Int contacts))
{
   T._report_contact=ReportContact;
   return T;
}
PhysicsClass& PhysicsClass::reportTrigger(void (*ReportTrigger)(ActorInfo &trigger, ActorInfo &actor, PHYS_CONTACT contact))
{
   T._report_trigger=ReportTrigger;
   return T;
}
PhysicsClass& PhysicsClass::wheelFriction(Flt (*WheelFriction)(C PhysMtrl &ground_material, C PhysMtrl &wheel_material, C ActorInfo &vehicle, WHEEL_TYPE wheel))
{
   T._wheel_friction=(WheelFriction ? WheelFriction : ::WheelFriction);
   return T;
}
PhysicsClass& PhysicsClass::dominance(Int dominance_group_a, Int dominance_group_b, Bool a_factor, Bool b_factor)
{
#if PHYSX
   if(Physx.world)
   {
	   PxDominanceGroupPair cd(a_factor, b_factor);
   	Physx.world->setDominanceGroupPair(dominance_group_a, dominance_group_b, cd);
   }
#endif
   return T;
}
/******************************************************************************/
#if !PHYSX
static Flt joint_impulse_scale;
#endif

PhysicsClass& PhysicsClass::startSimulation(Flt dt)
{
   if(!_simulated && created())
   {
     _simulated        =true;
     _new_updated      =false;
     _new_updated_time =0;
     _step_time        =0;
     _new_cpu_time     =0;
     _accumulated_time+=Max(0, dt); if(_accumulated_time>EPS)
      {
   const Int max_steps=16;
         Dbl time=Time.curTime();

         WriteLock lock(_rws);
      #if PHYSX
         if(timestep()==PHYS_TIMESTEP_ROUND)
         {
            if(_step_left=Trunc(_accumulated_time/_prec_time))
            {
               if(_step_left<=max_steps)
               {
                 _step_time        =_prec_time;
                 _new_updated_time =_prec_time*_step_left;
                 _accumulated_time-=_new_updated_time;
               }else
               {
                 _new_updated_time=_accumulated_time;
                 _step_left       = max_steps;
                 _step_time       =_new_updated_time/_step_left;
                 _accumulated_time=0;
               }
            }
         }else // PHYS_TIMESTEP_VARIABLE
         {
           _step_left       = Mid(RoundPos(_accumulated_time/_prec_time), 1, max_steps);
           _step_time       =_accumulated_time/_step_left;
           _new_updated_time=_accumulated_time;
           _accumulated_time=0;
         }
         if(_step_left){_step_left--; _new_updated=true; Physx.world->simulate(_step_time);}
	   #else
         // remember that calling 'stepSimulation' will internally call '*StepCompleted' functions
	      if(timestep()==PHYS_TIMESTEP_ROUND)
	      {
           _step_time       =_prec_time; // have to set before 'stepSimulation' because it will use 'stepCompleted' callback inside which this is going to be accessed
            Int steps=Bullet.world->stepSimulation(_accumulated_time, max_steps, _prec_time); // number of steps processed in this update
           _new_updated     =(steps>0);         // if the number of steps is >0 then it means we've updated physics in this update
           _new_updated_time= steps*_prec_time; // actual physics update time is the number of steps * step duration
         }else // PHYS_TIMESTEP_VARIABLE
         {
            Int steps=Mid(RoundPos(_accumulated_time/_prec_time), 1, max_steps); // number of steps in which we want to update physics
           _step_time       =_accumulated_time/steps; // have to set before 'stepSimulation' because it will use 'stepCompleted' callback inside which this is going to be accessed
            Bullet.world->stepSimulation(_accumulated_time, steps, _accumulated_time/(steps+EPSD)); // +EPSD is to make sure Bullet will calculate 'steps' steps when dividing 'dt' by "dt/(steps+EPSD)"
           _new_updated     = true            ; // in variable mode we always update physics
           _new_updated_time=_accumulated_time; // always by the total of 'dt' time
         }
         joint_impulse_scale=Sqrt(_step_time/_accumulated_time)/PI; // Sqrt(fixedTimeStep/timeStep), div by PI to match PhysX scale
	     _accumulated_time=0; // everything was pushed to Bullet counter
	   #endif
        _new_cpu_time=Time.curTime()-time;
      }
   }
   return T;
}
void PhysicsClass::stepCompleted()
{
   // set current 'time' to 'stepTime' as we may access it in vehicles update or custom step callback
  _time=stepTime();

   // first update vehicles
#if SUPPORT_PHYSX_VEHICLE
   Physx.updateVehicles();
#endif
   REPAO(_vehicles)->update();

   if(void (*step)()=simulation_step_completed)step(); // call custom callback if specified, copy to temp var to avoid multi-threaded issues

   // revert the time to the last update time, we have to do it, because on PhysX we may call 'stepCompleted' multiple times across the entire frame (update and draw), and when outside 'stepCompleted' we always want to refer to the last update total time
  _time=_last_updated_time;
}
void PhysicsClass::step()
{
#if PHYSX
   // TODO: alternatively "SnippetStepper" tutorial from PhysX SDK could be used, however it's possible it would execute codes on secondary threads
   if(_step_left>0)
   {
      WriteLock lock(_rws);
      if(_step_left>0 && Physx.world)
         if(Physx.world->checkResults(false)) // check if results are ready, don't block
      {
         Dbl time=Time.curTime();
         Physx.world->fetchResults(true);
         stepCompleted();
         Physx.world->simulate(_step_time);
        _step_left--;
        _new_cpu_time+=Time.curTime()-time;
      }
   }
#endif
}
PhysicsClass& PhysicsClass::stopSimulation()
{
   if(_simulated)
   {
      if(_new_updated)
      {
         Dbl time=Time.curTime();
         WriteLock lock(_rws);
      #if PHYSX
         for(; _step_left>0; _step_left--) // process all steps that are left
         {
            Physx.world->fetchResults(true);
            stepCompleted();
            Physx.world->simulate(_step_time);
         }
         Physx.world->fetchResults(true);
         stepCompleted();
      #else
         REPA(Bullet.breakables)if(btTypedConstraint *joint=Bullet.breakables[i])
         {
            // Warning: 'getAppliedImpulse' sometimes returns values too big TODO: investigate if it depends on steps/times or is it a bug in Bullet, or should this be called in 'stepCompleted'?
            Int force  =joint->getUserConstraintId();
            Flt impulse=joint->getAppliedImpulse()*joint_impulse_scale; joint->internalSetAppliedImpulse(0);
            if(btRigidBody *rb0=&joint->getRigidBodyA())impulse/=rb0->getInvMass();
            if(Abs(impulse)>=(Flt&)force)
            {
               joint->setUserConstraintType(1); // Joint.broken
               joint->enableFeedback(false);
               Bullet.breakables.remove(i);
               Bullet.world->removeConstraint(joint);
            }
         }
      #endif
        _new_cpu_time+=Time.curTime()-time;
        _update_count++;
      }

     _last_updated     =_updated;
     _last_updated_time=_updated_time;

                _updated     =_new_updated;
          _time=_updated_time=_new_updated_time;
      _cpu_time=_new_cpu_time;
     _simulated=false;
   }
   return T;
}
/******************************************************************************/
Bool PhysicsClass::createMaterials()
{
   WriteLock lock(_rws);

   mtrl_default  .create().bounciness(0.3f).frictionStatic(1.2f).frictionDynamic(1.0f);
 //mtrl_grass    .create().bounciness(0.0f).frictionStatic(0.0f).frictionDynamic(0.0f);
 //mtrl_ice      .create().bounciness(1.0f).frictionStatic(0.0f).frictionDynamic(0.0f);
 //mtrl_mud      .create().bounciness(0.0f).frictionStatic(3.0f).frictionDynamic(1.0f);
   mtrl_ctrl     .create().bounciness(0).bouncinessMode(PhysMtrl::MODE_MUL).anisotropicDir(Vec(0, 1, 0));
   mtrl_ctrl_stop.create().bounciness(0).bouncinessMode(PhysMtrl::MODE_MUL).frictionMode(PhysMtrl::MODE_MAX);

#if PHYSX
   if(_css==CSS_MATERIALS)
   {
      mtrl_ctrl.frictionStatic(0.0f).frictionDynamic(0.0f).frictionMode(PhysMtrl::MODE_MUL).frictionStaticA(2.5f).frictionDynamicA(0.0f).anisotropic(true);
   }else
   {
      mtrl_ctrl.frictionStatic(2.5f).frictionDynamic(2.5f).frictionStaticA(2.5f).frictionDynamicA(0.0f).anisotropic(true);
   }
   mtrl_ctrl_stop.frictionStatic(12).frictionDynamic(12);

   return mtrl_default._m!=null; // PhysX requires to have default material created (because of actor creation)
#else
   if(_css==CSS_MATERIALS)
   {
      mtrl_ctrl.frictionStatic(0.0f).frictionDynamic(0.0f).frictionMode(PhysMtrl::MODE_MUL).frictionStaticA(2.5f).frictionDynamicA(0.0f).anisotropic(true);
   }else
   {
      mtrl_ctrl.frictionStatic(0.2f).frictionDynamic(0.2f).frictionStaticA(2.5f).frictionDynamicA(0.0f).anisotropic(true);
   }
   mtrl_ctrl_stop.frictionStatic(1.0f).frictionDynamic(1.0f);

   return true;
#endif
}
/******************************************************************************/
Bool PhysicsClass::ignored(PHYS_API(PxRigidActor, RigidBody) &a, PHYS_API(PxRigidActor, RigidBody) &b)
{
   if(&a==&b)return true; // collisions are always ignored between the same actor (this is needed for Controller tests)
#if PHYSX
   PxShape *as, *bs;
   if(a.getShapes(&as, 1))if(UInt ai=as->getSimulationFilterData().word1)
   if(b.getShapes(&bs, 1))if(UInt bi=bs->getSimulationFilterData().word1)return FlagTest(Physx.ignoreMap(ai, bi), 1<<(bi&7));
   return false;
#else
   return a.ignore.has(&b);
#endif
}
/******************************************************************************/
PHYS_ENGINE PhysicsClass::engine()C
{
#if PHYSX
   return PHYS_ENGINE_PHYSX;
#else
   return PHYS_ENGINE_BULLET;
#endif
}
PhysicsClass::PhysicsClass()
{
   simulation_step_completed=null;
  _hw=_updated=_new_updated=_last_updated=false; Zero(_contact_report);
  _simulated=0;
  _precision=_actual_precision=_step_left=0;
  _update_count=0; Zero(_collision_array);
  _skin=_cpu_time=_new_cpu_time=_updated_time=_new_updated_time=_last_updated_time=_prec_time=_accumulated_time=_step_time=_time=0;
  _gravity.zero();
  _css=CSS_NONE;
  _timestep=PHYS_TIMESTEP_ROUND;
  _report_contact=null;
  _report_trigger=null;
  _wheel_friction=null;
}
Bool PhysicsClass::created()C {return PHYS_API(Physx, Bullet).world!=null;}
void PhysicsClass::del    ()
{
   SafeWriteLock lock(_rws);

   stopSimulation();

#if PHYSX
   Physx.del();
#else
   Bullet.del();
#endif

   PhysBodies.del();
   PhysMtrls .del(); mtrl_default.del(); mtrl_ctrl.del(); mtrl_ctrl_stop.del();

  _gravity .zero();
  _vehicles.del();
}
Bool PhysicsClass::createTry(C Str &physx_dll_path, CONTROLLER_SLOPE_SLIDING_MODE css, Bool hardware)
{
   if(created())return true;

   WriteLock lock(_rws);

   T._css           =css;
   T._timestep      =PHYS_TIMESTEP_ROUND;
   T._precision     =T._actual_precision=60; T._prec_time=1.0f/_actual_precision;
   T._hw            =false;
   T._wheel_friction=WheelFriction;

   T.ctrl_ground_group_force    =0;
   T.ctrl_ground_group_allow    =~IndexToFlag(AG_CONTROLLER);
   T.ctrl_slide_group_horizontal= IndexToFlag(AG_CONTROLLER);
   T.ctrl_ground_slope          = 0.65f;

#if PHYSX
   if(Physx.create(physx_dll_path, hardware))
#else
   if(Bullet.create())
#endif
   {
      skin(0.005f);
      gravity(Vec(0, -9.8f, 0));

      REPAO(_collision_array)=~0;
      Zero (_contact_report );

      if(createMaterials())return true;
   }
   del(); return false;
}
PhysicsClass& PhysicsClass::create(C Str &physx_dll_path, CONTROLLER_SLOPE_SLIDING_MODE css, Bool hardware)
{
   if(!createTry(physx_dll_path, css, hardware))
      Exit("Can't initialize physics."
         #if WINDOWS
           "\nPlease make sure you have PhysX DLL files in the path specified in 'Physics.create' method."
         #elif LINUX
           "\nPlease make sure you have PhysX SO files in the application's \"Bin\" folder."
         #endif
          );
   return T;
}
/******************************************************************************/
#if PHYSX
/******************************************************************************/
void PhysxClass::del()
{
#if SUPPORT_PHYSX_VEHICLE
   if(world        )PxCloseVehicleSDK();
   if(batch_query_4){batch_query_4->release(); batch_query_4=null;}
   vehicles.del();
#endif

   if(world        ){world        ->release(); world        =null;}
#if USE_DEFAULT_CPU_DISPATCHER
   if(CpuDispatcher){CpuDispatcher->release(); CpuDispatcher=null;}
#else
                     CpuDispatcher. del    ();
#endif
   if(cook[1]      ){cook[1]      ->release(); cook[1]      =null;}
   if(cook[0]      ){cook[0]      ->release(); cook[0]      =null;}
   if(physics      ){physics      ->release(); physics      =null;}
   if(foundation   ){foundation   ->release(); foundation   =null;}
   ignore_map     .del();
#if PHYSX_DLL_ACTUAL
   raycast=null;
   PhysXCooking   .del();
   PhysX3         .del();
   PhysXCommon    .delForce(); // PhysX has some bug, that it doesn't release the common DLL after interface creation, however we do need to release it in case we're deleting/updating DLL files (for example installer/patcher) TODO: test if this is still needed
   PhysXFoundation.del();
#endif
}
Bool PhysxClass::create(Str dll_path, Bool hardware)
{
   Bool ok=false;
#if PHYSX_DLL_ACTUAL
   #if WINDOWS
      dll_path.tailSlash(true);
      #if X64
         if(PhysXFoundation.createFile(dll_path+"PxFoundation_x64.dll" ))
         if(PhysXCommon    .createFile(dll_path+"PhysX3Common_x64.dll" ))
         if(PhysX3         .createFile(dll_path+"PhysX3_x64.dll"       ))
         if(PhysXCooking   .createFile(dll_path+"PhysX3Cooking_x64.dll"))
         if(raycast=PhysXCommon.getFunc("?raycast@PxGeometryQuery@physx@@SAIAEBVPxVec3@2@0AEBVPxGeometry@2@AEBVPxTransform@2@MV?$PxFlags@W4Enum@PxHitFlag@physx@@G@2@IPEIAUPxRaycastHit@2@@Z"))
      #else
         if(PhysXFoundation.createFile(dll_path+"PxFoundation_x86.dll" ))
         if(PhysXCommon    .createFile(dll_path+"PhysX3Common_x86.dll" ))
         if(PhysX3         .createFile(dll_path+"PhysX3_x86.dll"       ))
         if(PhysXCooking   .createFile(dll_path+"PhysX3Cooking_x86.dll"))
         if(raycast=PhysXCommon.getFunc("?raycast@PxGeometryQuery@physx@@SAIABVPxVec3@2@0ABVPxGeometry@2@ABVPxTransform@2@MV?$PxFlags@W4Enum@PxHitFlag@physx@@G@2@IPIAUPxRaycastHit@2@@Z"))
      #endif
   #elif LINUX // Warning: because of Linux poor DLL search path mechanism, the paths are hardcoded as "Bin" in project settings, therefore we must ignore the 'dll_path' path here
      #if X64
         if(PhysXFoundation.createFile("libPxFoundation_x64.so" ))
         if(PhysXCommon    .createFile("libPhysX3Common_x64.so" ))
         if(PhysX3         .createFile("libPhysX3_x64.so"       )) // warning: this file requires "libPhysX3Common_x64.so" and it will fail, unless we specify "Runtime Search Directories" in NetBeans Project Settings
         if(PhysXCooking   .createFile("libPhysX3Cooking_x64.so")) // warning: this file requires "libPhysX3Common_x64.so" and it will fail, unless we specify "Runtime Search Directories" in NetBeans Project Settings
         if(raycast=PhysXCommon.getFunc(fix me "_ZN5physx15PxGeometryQuery7raycastERKNS_6PxVec3ES3_RKNS_10PxGeometryERKNS_11PxTransformEfNS_7PxFlagsINS_9PxHitFlag4EnumEtEEjPNS_12PxRaycastHitEb")) // name extracted using Total Commander Hex Viewer (look for 'PxGeometryQuery' and copy everything from ' ' to ' ')
      #else
         if(PhysXFoundation.createFile("libPxFoundation_x86.so" ))
         if(PhysXCommon    .createFile("libPhysX3Common_x86.so" ))
         if(PhysX3         .createFile("libPhysX3_x86.so"       )) // warning: this file requires "libPhysX3Common_x86.so" and it will fail, unless we specify "Runtime Search Directories" in NetBeans Project Settings
         if(PhysXCooking   .createFile("libPhysX3Cooking_x86.so")) // warning: this file requires "libPhysX3Common_x86.so" and it will fail, unless we specify "Runtime Search Directories" in NetBeans Project Settings
         if(raycast=PhysXCommon.getFunc(fix me))
      #endif
   #endif
      if(auto _PxCreateFoundation =(decltype(&PxCreateFoundation ))PhysXFoundation.getFunc("PxCreateFoundation"))
      #define  PxCreateFoundation _PxCreateFoundation
      if(auto _PxCreateBasePhysics=(decltype(&PxCreateBasePhysics))PhysX3         .getFunc("PxCreateBasePhysics"))
      #define  PxCreateBasePhysics _PxCreateBasePhysics
      if(auto _PxCreateCooking    =(decltype(&PxCreateCooking    ))PhysXCooking   .getFunc("PxCreateCooking"))
      #define  PxCreateCooking _PxCreateCooking
      if(auto _PxRegisterCloth    =(decltype(&PxRegisterCloth    ))PhysX3         .getFunc("PxRegisterCloth"))
      #define  PxRegisterCloth _PxRegisterCloth
      if(auto _PxRegisterParticles=(decltype(&PxRegisterParticles))PhysX3         .getFunc("PxRegisterParticles"))
      #define  PxRegisterParticles _PxRegisterParticles
#endif
      if(foundation=PxCreateFoundation (PX_FOUNDATION_VERSION, allocator, ErrorCallback))
      if(physics   =PxCreateBasePhysics(PX_PHYSICS_VERSION   , *foundation, PxTolerancesScale(), false, null))
   {
      PxCookingParams cook_params(physics->getTolerancesScale());
      cook_params.suppressTriangleMeshRemapTable=true; // disable remap
      if(cook[0]=PxCreateCooking(PX_PHYSICS_VERSION, *foundation, cook_params))
      {
         cook_params.suppressTriangleMeshRemapTable=false; // enable remap
         if(cook[1]=PxCreateCooking(PX_PHYSICS_VERSION, *foundation, cook_params))
         {
            PxRegisterCloth    (*physics);
            PxRegisterParticles(*physics);

            PxSceneDesc scene_desc(physics->getTolerancesScale());
            scene_desc.flags|=PxSceneFlag::eENABLE_CCD;

         #if USE_DEFAULT_CPU_DISPATCHER
            if(!scene_desc.cpuDispatcher)scene_desc.cpuDispatcher=CpuDispatcher=PxDefaultCpuDispatcherCreate(Max(0, Cpu.threads()-1)); // leave 1 thread for the main update
         #else
            if(!scene_desc.cpuDispatcher)scene_desc.cpuDispatcher=&CpuDispatcher.create();
         #endif
            scene_desc.filterShader=FilterFunc;
            scene_desc.simulationEventCallback=&EventCallback;

            if(scene_desc.isValid())
            if(world=physics->createScene(scene_desc))
            {
               ignore_map.setNumZero(MAX_ACTOR_IGNORE*Ceil8(MAX_ACTOR_IGNORE)/8); // y's are bit-packed (8-bits per byte)
               ignore_id_gen.New(); // create 0 at start because "0" is a special case for turned off
            #if SUPPORT_PHYSX_VEHICLE
               vehicle_id_gen.New(); // create 0 at start because "0" is a special case for empty
               PxInitVehicleSDK(*physics);
               MeshBase mesh; mesh.create(Tube(1.0f, 1.0f, VecZero, Vec(1,0,0)), 0, 8); if(wheel_mesh.createConvexTry(mesh))
               {
	               PxBatchQueryDesc desc(Elms(raycast_query_result), 0, 0);
	               desc.queryMemory.userRaycastResultBuffer=raycast_query_result;
	               desc.queryMemory.userRaycastTouchBuffer =raycast_hit;
	               desc.queryMemory.raycastTouchBufferSize =Elms(raycast_hit);
	               desc.preFilterShader=VehicleWheelRaycastFilterFunc;
                  if(batch_query_4=world->createBatchQuery(desc))
                  {
                     // ok
                  }
               }
            #endif
               ok=true;
            }
         }
      }
   }
   if(!ok)del(); return ok;
}
/******************************************************************************/
#else // BULLET
/******************************************************************************/
static Ptr  BulletAlloc(size_t size) {return Alloc(size);}
static void BulletFree (Ptr    data) {return Free (data);}

void BulletClass::del()
{
	if(world)
	{
	   REP(world->getNumConstraints     ())world->removeConstraint(world->getConstraint(i));
	   REP(world->getNumCollisionObjects())
	   {
	      if(btCollisionObject *obj=world->getCollisionObjectArray()[i])
	      {
	         if(btRigidBody *body=CAST(btRigidBody, obj))if(btMotionState *motion=body->getMotionState())Delete(motion);

	         world->removeCollisionObject(obj);
	      }
	   }
	}

	Delete(world);
	Delete(solver);
	Delete(broadphase);
	Delete(dispatcher);
	Delete(collision_config);
}
static bool MaterialCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
{
   if(RigidBody *rb0=(RigidBody*)colObj0Wrap->m_collisionObject->getUserPointer())
   if(RigidBody *rb1=(RigidBody*)colObj1Wrap->m_collisionObject->getUserPointer())
   {
      /*// set 'rb0' as dynamic object with biggest mass (most significant dynamic object)
      if(!rb1->isStaticOrKinematicObject())
         if(rb0->isStaticOrKinematicObject() || rb1->getInvMass()<rb0->getInvMass)
            Swap(rb0, rb1);*/

      switch(rb0->material->bouncinessMode())
      {
         case PhysMtrl::MODE_AVG: cp.m_combinedRestitution=Avg(rb0->getRestitution(), rb1->getRestitution()); break;
         case PhysMtrl::MODE_MUL: cp.m_combinedRestitution=   (rb0->getRestitution()* rb1->getRestitution()); break;
         case PhysMtrl::MODE_MIN: cp.m_combinedRestitution=Min(rb0->getRestitution(), rb1->getRestitution()); break;
         case PhysMtrl::MODE_MAX: cp.m_combinedRestitution=Max(rb0->getRestitution(), rb1->getRestitution()); break;
      }
      switch(rb0->material->frictionMode())
      {
         case PhysMtrl::MODE_AVG: cp.m_combinedFriction=2*Avg(rb0->getFriction(), rb1->getFriction()); break; // 2* to match PhysX scale
         case PhysMtrl::MODE_MUL: cp.m_combinedFriction=2*   (rb0->getFriction()* rb1->getFriction()); break; // 2* to match PhysX scale
         case PhysMtrl::MODE_MIN: cp.m_combinedFriction=2*Min(rb0->getFriction(), rb1->getFriction()); break; // 2* to match PhysX scale
         case PhysMtrl::MODE_MAX: cp.m_combinedFriction=2*Max(rb0->getFriction(), rb1->getFriction()); break; // 2* to match PhysX scale
      }
   }
   return true;
}
static void BulletStepCompleted(btDynamicsWorld *world, btScalar timeStep) {Physics.stepCompleted();}
Bool BulletClass::create()
{
   btAlignedAllocSetCustom(BulletAlloc, BulletFree);

   collision_config=new btDefaultCollisionConfiguration();
   dispatcher      =new CollisionDispatcher(collision_config);
   broadphase      =new btDbvtBroadphase();
   solver          =new btSequentialImpulseConstraintSolver;
   world           =new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collision_config);

   gDeactivationTime=1;
   gContactAddedCallback=MaterialCallback;

   world->getPairCache()->setOverlapFilterCallback(&OverlapFilterCallback);
   world->setForceUpdateAllAabbs(false); // because of this call, changing pos/orn/matrix of inactive actors must be followed by 'updateSingleAabb'
   world->setInternalTickCallback(BulletStepCompleted);

   return true;
}
#endif
/******************************************************************************/
#if PHYSX
PhysicsClass& PhysicsClass::skin   (  Flt  skin   ) {if(                       Physx.world){T._skin   =Max(0, skin);                                                } return T;}
PhysicsClass& PhysicsClass::gravity(C Vec &gravity) {if(T._gravity!=gravity && Physx.world){T._gravity=gravity     ; Physx.world->setGravity(Physx.vec(T._gravity));} return T;}
#else
PhysicsClass& PhysicsClass::skin   (  Flt  skin   ) {if(                       Bullet.world){T._skin   =Max(0, skin);                                                  } return T;}
PhysicsClass& PhysicsClass::gravity(C Vec &gravity) {if(T._gravity!=gravity && Bullet.world){T._gravity=gravity     ; Bullet.world->setGravity(Bullet.vec(T._gravity));} return T;}
#endif
/******************************************************************************/
PhysicsClass& PhysicsClass::timestep(PHYS_TIMESTEP_MODE mode)
{
   Clamp(mode, PHYS_TIMESTEP_MODE(0), PHYS_TIMESTEP_MODE(PHYS_TIMESTEP_NUM-1));
   if(created())_timestep=mode;
   return T;
}
/******************************************************************************/
PhysicsClass& PhysicsClass::precision(Int steps_ps)
{
   if(created())
   {
      MAX(steps_ps, 0); T._precision=steps_ps;
      if(!steps_ps)
      {
             steps_ps=D.freq();
         if(!steps_ps)steps_ps=60;
      }
      if(steps_ps!=T._actual_precision)
      {
         T._actual_precision=steps_ps;
         T._prec_time       =1.0f/_actual_precision;
      }
   }
   return T;
}
Int PhysicsClass::actorShapes()C
{
#if PHYSX
   if(Physx.world)return Physx.world->getNbActors(PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC);
#else
   if(Bullet.world)return Bullet.world->getNumCollisionObjects();
#endif
   return 0;
}
/******************************************************************************/
}
/******************************************************************************/
