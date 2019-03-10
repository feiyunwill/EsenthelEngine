/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define TUBE_RES         4
#define TUBE_MASS_FACTOR (1.5708f/TUBE_RES) // because we operate on boxes we need to adjust for tube/box volume factor, calculated comparing Boxed Actor mass to desired Tube volume
static Flt Density(Flt density, Bool kinematic) {return (kinematic && density<=0) ? 1 : Max(0, density);}
/******************************************************************************/
#if PHYSX
/******************************************************************************/
static Bool Add(PxRigidActor &actor, C Plane &plane)
{
   if(PxShape *shape=actor.createShape(PxPlaneGeometry(), *Physics.mtrl_default._m))
   {
      shape->setLocalPose(Physx.matrix(Matrix().setPosRight(plane.pos, plane.normal)));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, C Box &box, C Vec *local_pos)
{
   if(PxShape *shape=actor.createShape(PxBoxGeometry(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f), *Physics.mtrl_default._m))
   {
      Matrix local_matrix; if(local_pos)local_matrix.pos=*local_pos;else local_matrix.pos.zero(); local_matrix.orn().identity();
      shape->setLocalPose(Physx.matrix(local_matrix));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, C OBox &obox, C Vec *local_pos)
{
   if(PxShape *shape=actor.createShape(PxBoxGeometry(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f), *Physics.mtrl_default._m))
   {
      Matrix local_matrix; if(local_pos)local_matrix.pos=*local_pos;else local_matrix.pos.zero(); local_matrix.orn()=obox.matrix.orn();
      shape->setLocalPose(Physx.matrix(local_matrix));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, C Extent &ext, C Vec *local_pos)
{
   if(PxShape *shape=actor.createShape(PxBoxGeometry(Physx.vec(ext.ext)), *Physics.mtrl_default._m))
   {
      Matrix local_matrix; if(local_pos)local_matrix.pos=*local_pos;else local_matrix.pos.zero(); local_matrix.orn().identity();
      shape->setLocalPose(Physx.matrix(local_matrix));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, C Ball &ball, C Vec *local_pos)
{
   if(PxShape *shape=actor.createShape(PxSphereGeometry(ball.r), *Physics.mtrl_default._m))
   {
      Matrix local_matrix; if(local_pos)local_matrix.pos=*local_pos;else local_matrix.pos.zero(); local_matrix.orn().identity();
      shape->setLocalPose(Physx.matrix(local_matrix));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, C Capsule &capsule, C Vec *local_pos)
{
   if(PxShape *shape=actor.createShape(PxCapsuleGeometry(capsule.r, capsule.h*0.5f-capsule.r), *Physics.mtrl_default._m))
   {
      Matrix local_matrix; if(local_pos)local_matrix.pos=*local_pos;else local_matrix.pos.zero(); local_matrix.orn().setRight(capsule.up);
      shape->setLocalPose(Physx.matrix(local_matrix));
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, PxConvexMesh &mesh, C Vec &scale)
{
   if(PxShape *shape=actor.createShape(PxConvexMeshGeometry(&mesh, PxMeshScale(Physx.vec(scale), PxQuat(PxIdentity))), *Physics.mtrl_default._m))
   {
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static Bool Add(PxRigidActor &actor, PxTriangleMesh &mesh, C Vec &scale)
{
   if(PxShape *shape=actor.createShape(PxTriangleMeshGeometry(&mesh, PxMeshScale(Physx.vec(scale), PxQuat(PxIdentity))), *Physics.mtrl_default._m))
   {
      shape->setContactOffset(Physics.skin());
      return true;
   }
   return false;
}
static inline Bool Create(Actor &actor, Bool dynamic, Bool kinematic, C Vec &pos=VecZero)
{
   if(dynamic || kinematic)actor._actor=actor._dynamic=Physx.physics->createRigidDynamic(Physx.matrix(pos));
   else                    actor._actor=               Physx.physics->createRigidStatic (Physx.matrix(pos));
   if(           kinematic)actor.kinematic(true);
   return actor._actor!=null;
}
/******************************************************************************/
ActorShapes& ActorShapes::add(C Plane   &plane               ) {_as->shape.New().set(0      , plane  ); return T;}
ActorShapes& ActorShapes::add(C Box     &box    , Flt density) {_as->shape.New().set(density, box    ); return T;}
ActorShapes& ActorShapes::add(C OBox    &obox   , Flt density) {_as->shape.New().set(density, obox   ); return T;}
ActorShapes& ActorShapes::add(C Extent  &ext    , Flt density) {_as->shape.New().set(density, ext    ); return T;}
ActorShapes& ActorShapes::add(C Ball    &ball   , Flt density) {_as->shape.New().set(density, ball   ); return T;}
ActorShapes& ActorShapes::add(C Capsule &capsule, Flt density) {_as->shape.New().set(density, capsule); return T;}
ActorShapes& ActorShapes::add(C Tube    &tube   , Flt density)
{
   density*=TUBE_MASS_FACTOR;
   Matrix matrix; matrix.setPosUp(tube.pos, tube.up);
   OBox   obox(Box(tube.r*SQRT2, tube.h, tube.r*SQRT2), matrix);
   matrix.orn().setRotate(tube.up, PI_2/TUBE_RES); REP(TUBE_RES)
   {
      add(obox, density);
      if(i)obox.matrix.orn()*=matrix.orn();
   }
   return T;
}
ActorShapes& ActorShapes::add(C PhysPart &const_part, Flt density, C Vec &scale)
{
   PhysPart &part=ConstCast(const_part);
   density=Density(part.density, false)*Density(density, false);
   switch(part.type())
   {
      case PHYS_SHAPE : add(Equal(scale, VecOne) ? part.shape : part.shape*scale, density); break;
      case PHYS_CONVEX: part.setPhysMesh(); if(part._pm && part._pm->_convex){_as->convex.New().set(density, scale, part._pm->_convex); IncRef(_as->pm.New()=part._pm);} break;
      case PHYS_MESH  : part.setPhysMesh(); if(part._pm && part._pm->_mesh  ){_as->mesh  .New().set(density, scale, part._pm->_mesh  ); IncRef(_as->pm.New()=part._pm);} break;
   }
   return T;
}
/******************************************************************************/
Actor& Actor::del()
{
   if(_actor)
   {
      SafeWriteLock lock(Physics._rws);
      if(_actor)
      {
         if(Physx.physics)_actor->release();
        _actor=_dynamic=null;
         if(_ignore_id)
         {
            if(Physx.ignore_map.elms())REP(MAX_ACTOR_IGNORE) // disable all possible ignores (check for 'ignore_map' existence in case it was already deleted)
            {
               FlagDisable(Physx.ignoreMap(i, _ignore_id), 1<<(_ignore_id&7));
               FlagDisable(Physx.ignoreMap(_ignore_id, i), 1<<(         i&7));
            }
            Physx.ignore_id_gen.Return(_ignore_id); _ignore_id=0;
         }
      }
   }
   REPA(_pm)DecRef(_pm[i]); _pm.del();
   return T;
}
Bool Actor::createTry(C ActorShapes &shapes, Flt density, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
  _ActorShapes &as=*shapes._as;
   if(!as.shape.elms() && !as.convex.elms() && !as.mesh.elms())return true;
   if(Physx.world)
   {
      Bool dynamic=(density>0 && !as.mesh.elms()); // must be false for triangle meshes
      if(  dynamic) // if at least one shape is static, then we can't create dynamic
      {
         REPA(as.shape )if(as.shape [i].density<=0){dynamic=false; break;}
         REPA(as.convex)if(as.convex[i].density<=0){dynamic=false; break;}
         REPA(as.mesh  )if(as.mesh  [i].density<=0){dynamic=false; break;}
      }
      if(Create(T, dynamic, kinematic))
      {
         Memt<Flt> densities;
         density=Density(density, kinematic);
         FREPA(as.shape)
         {
           _ActorShapes::Shape &s=as.shape[i];
            switch(s.shape.type)
            {
               case SHAPE_PLANE  : if(!Add(*_actor, s.shape.plane                             ))goto error; densities.add(Density(s.density, kinematic)*density); break;
               case SHAPE_BOX    : if(!Add(*_actor, s.shape.box    , &s.shape.box    .center()))goto error; densities.add(Density(s.density, kinematic)*density); break;
               case SHAPE_OBOX   : if(!Add(*_actor, s.shape.obox   , &s.shape.obox   .center()))goto error; densities.add(Density(s.density, kinematic)*density); break;
               case SHAPE_BALL   : if(!Add(*_actor, s.shape.ball   , &s.shape.ball   .pos     ))goto error; densities.add(Density(s.density, kinematic)*density); break;
               case SHAPE_CAPSULE: if(!Add(*_actor, s.shape.capsule, &s.shape.capsule.pos     ))goto error; densities.add(Density(s.density, kinematic)*density); break;
            }
         }
         FREPA(as.convex){_ActorShapes::Convex &convex=as.convex[i]; if(!Add(*_actor, *convex.convex, convex.scale))goto error; densities.add(Density(convex.density, kinematic)*density);}
         FREPA(as.mesh  ){_ActorShapes::Mesh   &mesh  =as.mesh  [i]; if(!Add(*_actor, *mesh  .mesh  , mesh  .scale))goto error; densities.add(Density(mesh  .density, kinematic)*density);}

        _pm=as.pm; REPA(_pm)IncRef(_pm[i]);
         if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, densities.data(), densities.elms(), null);
         group(0).materialForce(shapes._mtrl);
         Physx.world->addActor(*_actor);
         return true;
      }
   }
error:
   del(); return false;
}
Bool Actor::createTry(C Plane &plane)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
      if(Create(T, false, false))
   {
      if(Add(*_actor, plane))
      {
         //if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
         group(0).materialForce(null);
         Physx.world->addActor(*_actor);
         return true;
      }
      del();
   }
   return false;
}
Bool Actor::createTry(C Box &box, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
   {
      Vec pos=box.center();
      if(Create(T, density>0, kinematic, anchor ? *anchor : pos))
      {
         if(Add(*_actor, box, anchor ? &(pos-*anchor) : null))
         {
            if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
            group(0).materialForce(null);
            Physx.world->addActor(*_actor);
            return true;
         }
         del();
      }
   }
   return false;
}
Bool Actor::createTry(C OBox &obox, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
   {
      Vec pos=obox.center();
      if(Create(T, density>0, kinematic, anchor ? *anchor : pos))
      {
         if(Add(*_actor, obox, anchor ? &(pos-*anchor) : null))
         {
            if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
            group(0).materialForce(null);
            Physx.world->addActor(*_actor);
            return true;
         }
         del();
      }
   }
   return false;
}
Bool Actor::createTry(C Extent &ext, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
   {
      if(Create(T, density>0, kinematic, anchor ? *anchor : ext.pos))
      {
         if(Add(*_actor, ext, anchor ? &(ext.pos-*anchor) : null))
         {
            if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
            group(0).materialForce(null);
            Physx.world->addActor(*_actor);
            return true;
         }
         del();
      }
   }
   return false;
}
Bool Actor::createTry(C Ball &ball, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
   {
      if(Create(T, density>0, kinematic, anchor ? *anchor : ball.pos))
      {
         if(Add(*_actor, ball, anchor ? &(ball.pos-*anchor) : null))
         {
            if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
            group(0).materialForce(null);
            Physx.world->addActor(*_actor);
            return true;
         }
         del();
      }
   }
   return false;
}
Bool Actor::createTry(C Capsule &capsule, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.world)
   {
      if(Create(T, density>0, kinematic, anchor ? *anchor : capsule.pos))
      {
         if(Add(*_actor, capsule, anchor ? &(capsule.pos-*anchor) : null))
         {
            if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, Density(density, kinematic), null);
            group(0).materialForce(null);
            Physx.world->addActor(*_actor);
            return true;
         }
         del();
      }
   }
   return false;
}
Bool Actor::createTry(C Tube &tube, Flt density, C Vec *anchor, Bool kinematic)
{
   Vec dest=(anchor ? *anchor : tube.pos);
   if(createTry(ActorShapes().add(tube-dest), density, kinematic))
   {
      pos(dest);
      return true;
   }
   return false;
}
Bool Actor::createTry(C PhysPart &const_part, Flt density, C Vec &scale, Bool kinematic)
{
   PhysPart &part=ConstCast(const_part);
   density=Density(part.density, kinematic)*Density(density, kinematic);
   switch(part.type())
   {
      case PHYS_SHAPE: return createTry(Equal(scale, VecOne) ? part.shape : part.shape*scale, density, &VecZero, kinematic);

      case PHYS_CONVEX:
      {
         WriteLock lock(Physics._rws);
         del();
         part.setPhysMesh(); // creating in background thread sometimes doesn't succeed, so let's try again
         if(Physx.world && part._pm && part._pm->_convex)
            if(Create(T, density>0, kinematic))
         {
            if(Add(*_actor, *part._pm->_convex, scale))
            {
               IncRef(_pm.New()=part._pm);
               if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, density, null);
               group(0).materialForce(null);
               Physx.world->addActor(*_actor);
               return true;
            }
            del();
         }
      }return false;

      case PHYS_MESH:
      {
         WriteLock lock(Physics._rws);
         del();
         part.setPhysMesh(); // creating in background thread sometimes doesn't succeed, so let's try again
         if(Physx.world && part._pm && part._pm->_mesh)
            if(Create(T, false, kinematic)) // must be false for triangle meshes
         {
            if(Add(*_actor, *part._pm->_mesh, scale))
            {
               IncRef(_pm.New()=part._pm);
               if(_dynamic)PxRigidBodyExt::updateMassAndInertia(*_dynamic, density, null);
               group(0).materialForce(null);
               Physx.world->addActor(*_actor);
               return true;
            }
            del();
         }
      }return false;

      default: del(); return true;
   }
}
/******************************************************************************/
#else // BULLET
/******************************************************************************/
static void DelActorShapes(btCollisionShape *shape)
{
   if(shape)
   {
      if(btCompoundShape *compound=CAST(btCompoundShape, shape))
      {
         REP(compound->getNumChildShapes())DelActorShapes(compound->getChildShape(i)); // delete children
      }else
      if(btScaledBvhTriangleMeshShape *mesh=CAST(btScaledBvhTriangleMeshShape, shape))
      {
         DelActorShapes(mesh->getChildShape());
      }else
      if(btUniformScalingShape *uni=CAST(btUniformScalingShape, shape))
      {
         DelActorShapes(uni->getChildShape());
      }

      DecRef(shape);
   }
}
/******************************************************************************/
btStaticPlaneShape* NewShape(C Plane   &plane                            ) {                                                       btStaticPlaneShape *shape=new btStaticPlaneShape          (Bullet.vec(plane.normal), Dot(plane.pos, plane.normal)            ); shape->setMargin(Physics.skin()); return shape;}
btBoxShape        * NewShape(C Box     &box                              ) {                                                       btBoxShape         *shape=new btBoxShape                  (btVector3(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f)               ); shape->setMargin(Physics.skin()); return shape;}
btBoxShape        * NewShape(C OBox    &obox                             ) {                                                       btBoxShape         *shape=new btBoxShape                  (btVector3(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f)); shape->setMargin(Physics.skin()); return shape;}
btBoxShape        * NewShape(C Extent  &ext                              ) {                                                       btBoxShape         *shape=new btBoxShape                  (Bullet.vec(ext.ext)                                               ); shape->setMargin(Physics.skin()); return shape;}
btSphereShape     * NewShape(C Ball    &ball                             ) {                                                       btSphereShape      *shape=new btSphereShape               (ball.r                                                            ); shape->setMargin(Physics.skin()); return shape;}
btCapsuleShape    * NewShape(C Capsule &capsule                          ) {                                                       btCapsuleShape     *shape=new btCapsuleShape              (capsule.r, capsule.h-capsule.r*2                                  ); shape->setMargin(Physics.skin()); return shape;}
btCylinderShape   * NewShape(C Tube    &tube                             ) {                                                       btCylinderShape    *shape=new btCylinderShape             (btVector3(tube.r, tube.h*0.5f, tube.r)                            ); shape->setMargin(Physics.skin()); return shape;}
btCollisionShape  * NewShape(btConvexHullShape      *convex, C Vec &scale) {IncRef(convex); if(Equal(scale, VecOne))return convex; btCollisionShape   *shape=new btUniformScalingShape       (convex,            scale.max()                                    ); shape->setMargin(Physics.skin()); return shape;} // always IncRef 'convex' because either it's returned or it's used by 'btUniformScalingShape'
btCollisionShape  * NewShape(btBvhTriangleMeshShape *mesh  , C Vec &scale) {IncRef(mesh  ); if(Equal(scale, VecOne))return mesh  ; btCollisionShape   *shape=new btScaledBvhTriangleMeshShape(mesh  , Bullet.vec(scale)                                         ); shape->setMargin(Physics.skin()); return shape;} // always IncRef 'mesh  ' because either it's returned or it's used by 'btScaledBvhTriangleMeshShape'
btCompoundShape   * NewShape(                                            ) {                                                       btCompoundShape    *shape=new btCompoundShape                                                                                 ; shape->setMargin(Physics.skin()); return shape;}
/******************************************************************************/
ActorShapes& ActorShapes::add(C Plane    &plane                  ) {_as->shape.New().set(0      , plane  ); return T;}
ActorShapes& ActorShapes::add(C Box      &box       , Flt density) {_as->shape.New().set(density, box    ); return T;}
ActorShapes& ActorShapes::add(C OBox     &obox      , Flt density) {_as->shape.New().set(density, obox   ); return T;}
ActorShapes& ActorShapes::add(C Extent   &ext       , Flt density) {_as->shape.New().set(density, ext    ); return T;}
ActorShapes& ActorShapes::add(C Ball     &ball      , Flt density) {_as->shape.New().set(density, ball   ); return T;}
ActorShapes& ActorShapes::add(C Capsule  &capsule   , Flt density) {_as->shape.New().set(density, capsule); return T;}
ActorShapes& ActorShapes::add(C Tube     &tube      , Flt density) {_as->shape.New().set(density, tube   ); return T;}
ActorShapes& ActorShapes::add(C PhysPart &const_part, Flt density, C Vec &scale)
{
   PhysPart &part=ConstCast(const_part);
   density=Density(part.density, false)*Density(density, false);
   switch(part.type())
   {
      case PHYS_SHAPE : add(Equal(scale, VecOne) ? part.shape : part.shape*scale, density); break;
      case PHYS_CONVEX: part.setPhysMesh(); if(part._pm && part._pm->_convex){_as->convex.New().set(density, part._pm->volume(), scale, part._pm->_convex); IncRef(_as->pm.New()=part._pm);} break; // here 'IncRef' for '_convex' is not needed because there's an 'IncRef' for '_pm' which already holds references to that shape
      case PHYS_MESH  : part.setPhysMesh(); if(part._pm && part._pm->_mesh  ){_as->mesh  .New().set(density,                     scale, part._pm->_mesh  ); IncRef(_as->pm.New()=part._pm);} break; // here 'IncRef' for '_mesh  ' is not needed because there's an 'IncRef' for '_pm' which already holds references to that shape
   }
   return T;
}
/******************************************************************************/
Actor& Actor::del()
{
   if(_actor)
   {
      SafeWriteLock lock(Physics._rws);
      if(_actor)
      {
         if(Bullet.world)
         {
            REP(_actor->getNumConstraintRefs())Bullet.world->removeConstraint(_actor->getConstraintRef(i)); // remove all joints linked with this actor
            Bullet.world->removeRigidBody(_actor); // remove from world
         }
         DelActorShapes(_actor->getCollisionShape()); // delete shapes
         if(btMotionState *motion_state=_actor->getMotionState())Delete(motion_state); // delete motion state
         Delete(_actor);
      }
   }
   REPA(_pm)DecRef(_pm[i]); _pm.del();
   return T;
}
Bool Actor::init(btRigidBody::btRigidBodyConstructionInfo &info, C Vec *anchor, Bool kinematic, PhysMesh *pm, PhysMtrl *material)
{
  _actor=new RigidBody(info);
	Bullet.world->addRigidBody(_actor);

   // desired_matrix = offset * bullet_matrix
   // offset = desired_matrix / bullet_matrix
   Matrix bullet_matrix=massCenterMatrix(),
         desired_matrix(anchor ? *anchor : bullet_matrix.pos);
   desired_matrix.divNormalized(bullet_matrix, _actor->offset);

	group(0).ray(true).kinematic(kinematic).materialForce(material);

  _actor->setCollisionFlags(_actor->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

   if(pm)IncRef(T._pm.New()=pm);
   return true;
}
/******************************************************************************/
static Matrix GetTransform(C Box     &box    ) {return Matrix(                    box.center()  );}
static Matrix GetTransform(C OBox    &obox   ) {return Matrix(obox.matrix.orn(), obox.center()  );}
static Matrix GetTransform(C Extent  &ext    ) {return Matrix(                    ext.center    );}
static Matrix GetTransform(C Ball    &ball   ) {return Matrix(           ball   .pos            );}
static Matrix GetTransform(C Capsule &capsule) {return Matrix().setPosUp(capsule.pos, capsule.up);}
static Matrix GetTransform(C Tube    &tube   ) {return Matrix().setPosUp(tube   .pos, tube   .up);}
/******************************************************************************/
Bool Actor::createTry(C ActorShapes &shapes, Flt density, Bool kinematic)
{
  _ActorShapes &as=*shapes._as;

   if(as.shape.elms()==0 && as.convex.elms()==0 && as.mesh.elms()==0){del(); return true;}
   if(as.shape.elms()==1 && as.convex.elms()==0 && as.mesh.elms()==0){   if(createTry(as.shape .first().shape ,                          as.shape .first().density*density, &VecZero, kinematic)){materialForce(shapes._mtrl); return true;} return false;}
   if(as.shape.elms()==0 && as.convex.elms()==1 && as.mesh.elms()==0)return createTry(as.convex.first().convex, as.convex.first().scale, as.convex.first().density*density,           kinematic, (as.pm.elms()==1) ? as.pm[0] : null, shapes._mtrl); // here 'IncRef' is not needed because it will be called inside that method if needed
   if(as.shape.elms()==0 && as.convex.elms()==0 && as.mesh.elms()==1)return createTry(as.mesh  .first().mesh  , as.mesh  .first().scale                                                        , (as.pm.elms()==1) ? as.pm[0] : null, shapes._mtrl); // here 'IncRef' is not needed because it will be called inside that method if needed

   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      density=Density(density, kinematic);
      Flt mass  =0;
      Vec center=0;
      REPA(as.shape ){_ActorShapes::Shape  &shape =as.shape [i];                                                                                        Flt m=shape.shape.volume()                                           *Density( shape.density, kinematic)*density; mass+=m; center+=m*shape.shape.pos();}
      REPA(as.convex){_ActorShapes::Convex &convex=as.convex[i]; btVector3 bcenter; btScalar radius; convex.convex->getBoundingSphere(bcenter, radius); Flt m=     convex.volume*convex.scale.x*convex.scale.y*convex.scale.z*Density(convex.density, kinematic)*density; mass+=m; center+=m*Bullet.vec(bcenter)*convex.scale;}
      if(mass)center/=mass;

      btCompoundShape *compound=NewShape();
      FREPA(as.shape)
      {
        _ActorShapes::Shape &s=as.shape[i];
         switch(s.shape.type)
         {
            case SHAPE_PLANE  : compound->addChildShape(Bullet.matrix(                               Matrix(-center)), NewShape(s.shape.plane  )); break;
            case SHAPE_BOX    : compound->addChildShape(Bullet.matrix(GetTransform(s.shape.box    ).moveBack(center)), NewShape(s.shape.box    )); break;
            case SHAPE_OBOX   : compound->addChildShape(Bullet.matrix(GetTransform(s.shape.obox   ).moveBack(center)), NewShape(s.shape.obox   )); break;
            case SHAPE_BALL   : compound->addChildShape(Bullet.matrix(GetTransform(s.shape.ball   ).moveBack(center)), NewShape(s.shape.ball   )); break;
            case SHAPE_CAPSULE: compound->addChildShape(Bullet.matrix(GetTransform(s.shape.capsule).moveBack(center)), NewShape(s.shape.capsule)); break;
            case SHAPE_TUBE   : compound->addChildShape(Bullet.matrix(GetTransform(s.shape.tube   ).moveBack(center)), NewShape(s.shape.tube   )); break;
         }
      }
      FREPA(as.convex){_ActorShapes::Convex &convex=as.convex[i]; compound->addChildShape(Bullet.matrix(Matrix(-center)), NewShape(convex.convex, convex.scale));} // 'NewShape' will call 'IncRef' if needed
      FREPA(as.mesh  ){_ActorShapes::Mesh   &mesh  =as.mesh  [i]; compound->addChildShape(Bullet.matrix(Matrix(-center)), NewShape(mesh  .mesh  , mesh  .scale));} // 'NewShape' will call 'IncRef' if needed

      btVector3 local_inertia(0, 0, 0); if(mass)compound->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, compound, local_inertia);
      if(init(info, &(-center), kinematic, null, shapes._mtrl))
      {
        _pm=as.pm; REPA(_pm)IncRef(_pm[i]);
         return true;
      }
   }
   return false;
}
Bool Actor::createTry(C Plane &plane)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btStaticPlaneShape *shape=NewShape(plane);
      btRigidBody::btRigidBodyConstructionInfo info(0, null, shape);
      return init(info, null, false, null, null);
	}
   return false;
}
Bool Actor::createTry(C Box &box, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btBoxShape *shape=NewShape(box);
      Flt         mass =box.volume()*Density(density, kinematic);
      btVector3   local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(box)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
   return false;
}
Bool Actor::createTry(C OBox &obox, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btBoxShape *shape=NewShape(obox);
      Flt         mass =obox.volume()*Density(density, kinematic);
      btVector3   local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(obox)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
   return false;
}
Bool Actor::createTry(C Extent &ext, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btBoxShape *shape=NewShape(ext);
      Flt         mass =ext.volume()*Density(density, kinematic);
      btVector3   local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(ext)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
   return false;
}
Bool Actor::createTry(C Ball &ball, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btSphereShape *shape=NewShape(ball);
      Flt            mass =ball.volume()*Density(density, kinematic);
      btVector3      local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(ball)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
	return false;
}
Bool Actor::createTry(C Capsule &capsule, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btCapsuleShape *shape=NewShape(capsule);
      Flt             mass =capsule.volume()*Density(density, kinematic);
      btVector3       local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(capsule)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
   return false;
}
Bool Actor::createTry(C Tube &tube, Flt density, C Vec *anchor, Bool kinematic)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world)
   {
      btCylinderShape *shape=NewShape(tube);
      Flt              mass =tube.volume()*Density(density, kinematic);
      btVector3        local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
      btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
      info.m_startWorldTransform=Bullet.matrix(GetTransform(tube)); // info.m_motionState=new btDefaultMotionState(info.m_startWorldTransform);
      return init(info, anchor, kinematic, null, null);
	}
   return false;
}
Bool Actor::createTry(btConvexHullShape *convex, C Vec &scale, Flt density, Bool kinematic, PhysMesh *pm, PhysMtrl *material)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world && convex)
   {
                        density=Density(density, kinematic);
      btVector3         bcenter; btScalar radius; convex->getBoundingSphere(bcenter, radius); Vec center=Bullet.vec(bcenter)*scale;
      Flt               mass  =(pm ? pm->volume() : Ball(radius).volume())*scale.x*scale.y*scale.z*density; // if mesh is unavailable then approximate with a Ball
      btCollisionShape *shape =NewShape(convex, scale); // 'NewShape' will call 'IncRef' if needed
      if(mass && center.length2()>0.01f*0.01f) // if non-static actor and convex center is not at (0, 0, 0) then offset must be used
      {
         btCompoundShape *compound=NewShape();
         compound->addChildShape(Bullet.matrix(Matrix(-center)), shape);

         btVector3 local_inertia(0, 0, 0); if(mass)compound->calculateLocalInertia(mass, local_inertia);
         btRigidBody::btRigidBodyConstructionInfo info(mass, null, compound, local_inertia);
         return init(info, &(-center), kinematic, pm, material);
      }else
      {
         btVector3 local_inertia(0, 0, 0); if(mass)shape->calculateLocalInertia(mass, local_inertia);
         btRigidBody::btRigidBodyConstructionInfo info(mass, null, shape, local_inertia);
         return init(info, null, kinematic, pm, material);
      }
   }
   return false;
}
Bool Actor::createTry(btBvhTriangleMeshShape *mesh, C Vec &scale, PhysMesh *pm, PhysMtrl *material)
{
   WriteLock lock(Physics._rws);
   del();
   if(Bullet.world && mesh)
   {
      btRigidBody::btRigidBodyConstructionInfo info(0, null, NewShape(mesh, scale)); // mass must be 0 to indicate static object, 'NewShape' will call 'IncRef' if needed
      return init(info, null, false, pm, material);
   }
   return false;
}
Bool Actor::createTry(C PhysPart &const_part, Flt density, C Vec &scale, Bool kinematic)
{
   PhysPart &part=ConstCast(const_part);
   density=Density(part.density, kinematic)*Density(density, kinematic);
   switch(part.type())
   {
      case PHYS_SHAPE :                     return createTry(Equal(scale, VecOne) ? part.shape : part.shape*scale, density, &VecZero, kinematic                );
      case PHYS_CONVEX: part.setPhysMesh(); return createTry(part._pm ? part._pm->_convex : null,           scale, density,           kinematic, part._pm, null); // here 'IncRef' for '_convex' is not needed because if needed then it's called inside that method
      case PHYS_MESH  : part.setPhysMesh(); return createTry(part._pm ? part._pm->_mesh   : null,           scale                              , part._pm, null); // here 'IncRef' for '_mesh  ' is not needed because if needed then it's called inside that method
   }
   del(); return true;
}
/******************************************************************************/
#endif
/******************************************************************************/
_ActorShapes::~_ActorShapes() {REPA(pm)DecRef(pm[i]);}
 ActorShapes::~ ActorShapes() {Delete(_as);}
 ActorShapes::  ActorShapes() {New   (_as); _mtrl=null;}

ActorShapes& ActorShapes::add(C Shape &shape, Flt density)
{
   switch(shape.type)
   {
      case SHAPE_PLANE  : add(shape.plane           ); break;
      case SHAPE_BOX    : add(shape.box    , density); break;
      case SHAPE_OBOX   : add(shape.obox   , density); break;
      case SHAPE_BALL   : add(shape.ball   , density); break;
      case SHAPE_CAPSULE: add(shape.capsule, density); break;
      case SHAPE_TUBE   : add(shape.tube   , density); break;
   }
   return T;
}
ActorShapes& ActorShapes::add(C PhysPart &part, C Vec &scale)
{
   return add(part, 1, scale);
}
ActorShapes& ActorShapes::add(C PhysBody &phys, C Vec &scale)
{
   if(!_mtrl && phys.material)_mtrl=phys.material;
   Flt density=phys.finalDensity();
   FREPA(phys)add(phys.parts[i], density, scale);
   return T;
}
/******************************************************************************/
Bool Actor::createTry(C Shape &shape, Flt density, C Vec *anchor, Bool kinematic)
{
   switch(shape.type)
   {
      default           : del(); return false;
      case SHAPE_NONE   : del(); return true ;
      case SHAPE_PLANE  :        return createTry(shape.plane                              );
      case SHAPE_BOX    :        return createTry(shape.box    , density, anchor, kinematic);
      case SHAPE_OBOX   :        return createTry(shape.obox   , density, anchor, kinematic);
      case SHAPE_BALL   :        return createTry(shape.ball   , density, anchor, kinematic);
      case SHAPE_CAPSULE:        return createTry(shape.capsule, density, anchor, kinematic);
      case SHAPE_TUBE   :        return createTry(shape.tube   , density, anchor, kinematic);
   }
}
Bool Actor::createTry(C PhysBody &phys, Flt density, C Vec &scale, Bool kinematic)
{
   switch(phys.parts.elms())
   {
      case 0: del(); return true;
      case 1: if(createTry(phys.parts[0], Density(phys.finalDensity(), kinematic)*Density(density, kinematic), scale, kinematic))
      {
         materialForce(phys.material);
         return true;
      }return false;

      default:
      {
         ActorShapes shapes; shapes.add(phys, scale); // 'phys.material' and 'phys.density' is passed into 'shapes'
         return createTry(shapes, density, kinematic);
      }
   }
}
/******************************************************************************/
static void CheckPhysics(                                        ) {if(!Physics.created())Exit("Attempting to create an actor without creating Physics first.");}
static void CheckMesh   (C PhysPart &part, C PhysBody *owner=null)
{
   switch(part.type())
   {
      case PHYS_CONVEX:
      case PHYS_MESH  :
      {
         Str file=PhysBodies.name(owner);
         Exit(S+"Can't create actor from "+((part.type()==PHYS_CONVEX) ? "PHYS_CONVEX" : "PHYS_MESH")+" physical body.\n"
         "Physics Engine Used: "
      #if PHYSX
         "PhysX\n"
      #else
         "Bullet\n"
      #endif
         "Universal Data Available: "+((part._pm && part._pm-> _base                    ) ? "Yes\n" : "No\n")+
         "PhysX Data Available: "    +((part._pm && part._pm-> _physx_cooked_data.elms()) ? "Yes\n" : "No\n")+
         "Bullet Data Available: "   +((part._pm && part._pm->_bullet_cooked_data       ) ? "Yes\n" : "No\n")+
      #if PHYSX
         "PhysX Mesh Available: "    +((part._pm && ((part.type()==PHYS_CONVEX) ? part._pm->_convex!=null : part._pm->_mesh!=null)) ? "Yes\n" : "No\n")+
      #else
         "Bullet Mesh Available: "   +((part._pm && ((part.type()==PHYS_CONVEX) ? part._pm->_convex!=null : part._pm->_mesh!=null)) ? "Yes\n" : "No\n")+
      #endif
         "PhysBody file: \""+file+"\""
      #if !PHYSX
         +((part._pm && part._pm->_base) ? "" : "\nPlease recreate the PhysBody file.")
      #endif
         );
      }break;
   }
}
static void CheckMesh(C PhysBody &phys) {REPA(phys)CheckMesh(phys.parts[i], &phys);}
/******************************************************************************/
Actor& Actor::create(C ActorShapes &shapes , Flt density,                Bool kinematic) {if(!createTry(shapes , density,         kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C ActorShapes &shapes, Flt density, Bool kinematic);\ndensity = "                                                           +density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Plane       &plane                                              ) {if(!createTry(plane                              )){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Plane &plane);\nplane = "                                                             +plane  .asText(    )                                                                                            );} return T;}
Actor& Actor::create(C Box         &box    , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(box    , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Box &box, Flt density, C Vec *anchor, Bool kinematic);\nbox = "                       +box    .asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C OBox        &obox   , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(obox   , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C OBox &obox, Flt density, C Vec *anchor, Bool kinematic);\nobox = "                    +obox   .asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Extent      &ext    , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(ext    , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Extent &ext, Flt density, C Vec *anchor, Bool kinematic);\next = "                    +ext    .asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Ball        &ball   , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(ball   , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Ball &ball, Flt density, C Vec *anchor, Bool kinematic);\nball = "                    +ball   .asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Capsule     &capsule, Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(capsule, density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Capsule &capsule, Flt density, C Vec *anchor, Bool kinematic);\ncapsule = "           +capsule.asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Tube        &tube   , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(tube   , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Tube &tube, Flt density, C Vec *anchor, Bool kinematic);\ntube = "                    +tube   .asText(    )+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C Shape       &shape  , Flt density, C Vec *anchor, Bool kinematic) {if(!createTry(shape  , density, anchor, kinematic)){CheckPhysics();                  Exit(S+"Can't create actor:\nActor.create(C Shape &shape, Flt density, C Vec *anchor, Bool kinematic);\nshape = "                 +shape  .asText(true)+"\ndensity = "+density+"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C PhysPart    &part   , Flt density, C Vec &scale , Bool kinematic) {if(!createTry(part   , density, scale , kinematic)){CheckPhysics(); CheckMesh(part); Exit(S+"Can't create actor:\nActor.create(C PhysPart &part, Flt density, C Vec &scale, C Vec *anchor,Bool kinematic);\ndensity = "+density               +"\nscale = "+scale  +"\nkinematic = "+kinematic                                          );} return T;}
Actor& Actor::create(C PhysBody    &phys   , Flt density, C Vec &scale , Bool kinematic) {if(!createTry(phys   , density, scale , kinematic)){CheckPhysics(); CheckMesh(phys); Exit(S+"Can't create actor:\nActor.create(C PhysBody &phys, Flt density, C Vec &scale, Bool kinematic);\ndensity = "              +density               +"\nscale = "+scale  +"\nkinematic = "+kinematic+"\nphys = \""+PhysBodies.name(&phys)+"\"");} return T;}
/******************************************************************************/
}
/******************************************************************************/
