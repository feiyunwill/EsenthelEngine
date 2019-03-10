/******************************************************************************/
#include "stdafx.h"
#if !PHYSX // Bullet
   #define CONVEX_RAY_TEST_SUB_SIMPLEX 0 // precision: low , performance: fastest
   #define CONVEX_RAY_TEST_GJK         1 // precision: high, performance: medium
   #define CONVEX_RAY_TEST_CCC         2 // precision: high, performance: slowest
   #define CONVEX_RAY_TEST CONVEX_RAY_TEST_GJK

   #include "../../../ThirdPartyLibs/begin.h"
   #include "../../../ThirdPartyLibs/Bullet/lib/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#if   CONVEX_RAY_TEST==CONVEX_RAY_TEST_SUB_SIMPLEX
   #include "../../../ThirdPartyLibs/Bullet/lib/src/BulletCollision/NarrowPhaseCollision/btSubsimplexConvexCast.h"
#elif CONVEX_RAY_TEST==CONVEX_RAY_TEST_GJK
   #include "../../../ThirdPartyLibs/Bullet/lib/src/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.h"
#elif CONVEX_RAY_TEST==CONVEX_RAY_TEST_CCC
   #include "../../../ThirdPartyLibs/Bullet/lib/src/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.h"
   #include "../../../ThirdPartyLibs/Bullet/lib/src/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#endif
   #include "../../../ThirdPartyLibs/end.h"
#endif
namespace EE{
/******************************************************************************

   PxTransform * operator uses reverse order:
   Matrix      A, B;
   PxTransform a, b;   A*B == b*a

/******************************************************************************/
Bool PhysPart::ray(C Vec &pos, C Vec &move, C Matrix *body_matrix, PhysHitBasic *phys_hit, Bool two_sided)C
{
   if(type()==PHYS_SHAPE)
   {
      Vec p=pos, m=move;
      if(body_matrix)
      {
         p.div(*body_matrix       );
         m.div( body_matrix->orn());
      }
      Flt frac; Vec normal;
      if(SweepPointShape(p, m, shape, phys_hit ? &frac : null, phys_hit ? &normal : null))
      {
         if(phys_hit)
         {
            if(body_matrix){normal*=body_matrix->orn(); normal.normalize();}
            phys_hit->face        =-1;
            phys_hit->frac        =frac;
            phys_hit->dist        =    frac*move.length();
            phys_hit->plane.pos   =pos+frac*move;
            phys_hit->plane.normal=phys_hit->face_nrm=normal;
         }
         return true;
      }
   }else
   if(_pm)
   {
   #if PHYSX
   #if PHYSX_DLL_ACTUAL
      #define PxRaycast (decltype(&PxGeometryQuery::raycast)(Physx.raycast))
      if(!PxRaycast)Exit("PhysPart.ray\nPhysics hasn't been created");
   #else
      #define PxRaycast PxGeometryQuery::raycast
   #endif
      Vec dir=move; Flt length=dir.normalize();
      PxTransform      transform=Physx.matrix(body_matrix ? *body_matrix : MatrixIdentity);
      Vec              scale    =            (body_matrix ?  body_matrix->scale() : Vec(1));
      PxVec3           px_pos   =Physx.vec(pos),
                       px_dir   =Physx.vec(dir);
      PxMeshScale      mesh_scale(Physx.vec(scale), PxQuat(PxIdentity));
      PxGeometryHolder geom;
      switch(_pm->_type)
      {
         case PHYS_MESH  : geom.storeAny(PxTriangleMeshGeometry(_pm->_mesh  , mesh_scale)); break;
         case PHYS_CONVEX: geom.storeAny(  PxConvexMeshGeometry(_pm->_convex, mesh_scale)); break;
      }
      PxRaycastHit hits[1];
      PxHitFlag::Enum flag=(two_sided ? PxHitFlag::eMESH_BOTH_SIDES : PxHitFlag::Enum(0));
      if(phys_hit)
      {
         if(PxRaycast(px_pos, px_dir, geom.any(), transform, length, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | flag, Elms(hits), hits)>0)
         {
            phys_hit->set(hits[0], length);
            if(phys_hit->face>=0 && _pm->_type==PHYS_MESH)
            {
               Matrix3 orn; if(body_matrix) // normalize
               {
                  orn.x=body_matrix->x/scale.x;
                  orn.y=body_matrix->y/scale.y;
                  orn.z=body_matrix->z/scale.z;
               }
               phys_hit->updateFace(geom.triangleMesh(), body_matrix ? &orn : null);
            }
            return true;
         }
      }else return PxRaycast(px_pos, px_dir, geom.any(), transform, length, PxHitFlag::eMESH_ANY | flag, Elms(hits), hits)>0; // if we don't pass 'hits' then this function won't do anything
   #else
      switch(_pm->_type)
      {
         case PHYS_CONVEX:
         {
            // code taken from "btCollisionWorld::rayTestSingleInternal"
            btSphereShape pointShape(0); pointShape.setMargin(0);
            btVoronoiSimplexSolver simplexSolver;

         #if   CONVEX_RAY_TEST==CONVEX_RAY_TEST_SUB_SIMPLEX
		      btSubsimplexConvexCast caster(&pointShape, _pm->_convex, &simplexSolver);
         #elif CONVEX_RAY_TEST==CONVEX_RAY_TEST_GJK
		      btGjkConvexCast caster(&pointShape, _pm->_convex, &simplexSolver);
         #elif CONVEX_RAY_TEST==CONVEX_RAY_TEST_CCC
		      btGjkEpaPenetrationDepthSolver gjkEpaPenetrationSolver; btContinuousConvexCollision caster(&pointShape, _pm->_convex, &simplexSolver, &gjkEpaPenetrationSolver);
         #endif

            btTransform rayFromTrans; rayFromTrans.setIdentity(); rayFromTrans.setOrigin(Bullet.vec(pos));
            btTransform rayToTrans  ; rayToTrans  .setIdentity(); rayToTrans  .setOrigin(Bullet.vec(pos+move));
            btTransform colObjWorldTransform; if(body_matrix)colObjWorldTransform=Bullet.matrix(*body_matrix);else colObjWorldTransform.setIdentity();
            btConvexCast::CastResult castResult;
            if(caster.calcTimeOfImpact(rayFromTrans, rayToTrans, colObjWorldTransform, colObjWorldTransform, castResult))
            {
               if(phys_hit)
               {
                  phys_hit->face=-1;
                  phys_hit->frac=castResult.m_fraction;
                  phys_hit->dist=phys_hit->frac*move.length();
                  phys_hit->face_nrm=Bullet.vec(castResult.m_normal);
                  phys_hit->plane.set(Bullet.vec(castResult.m_hitPoint), phys_hit->face_nrm);
               }
               return true;
            }
         }break;

         case PHYS_MESH:
         {
            Vec p=pos, m=move;
            if(body_matrix)
            {
               p.div(*body_matrix       );
               m.div( body_matrix->orn());
            }
            struct Callback : btTriangleRaycastCallback
            {
               PhysHitBasic *phys_hit;
               Bool          hit;

	            virtual btScalar reportHit(const btVector3& hitNormalLocal, btScalar hitFraction, int partId, int triangleIndex)
               {
                  hit=true;
                  if(phys_hit)
                  {
                     phys_hit->face    =triangleIndex;
                     phys_hit->frac    =hitFraction;
                     phys_hit->face_nrm=Bullet.vec(hitNormalLocal);
                  }
                  return hitFraction;
               }
	            Callback(const btVector3& from, const btVector3& to, unsigned int flags, PhysHitBasic *phys_hit) : btTriangleRaycastCallback(from, to, flags), phys_hit(phys_hit), hit(false) {}
            };
            Callback callback(Bullet.vec(p), Bullet.vec(p+m), btTriangleRaycastCallback::kF_KeepUnflippedNormal | (two_sided ? 0 : btTriangleRaycastCallback::kF_FilterBackfaces), phys_hit);
           _pm->_mesh->performRaycast(&callback, callback.m_from, callback.m_to);
            if(callback.hit && phys_hit)
            {
               if(body_matrix)phys_hit->face_nrm.mul(body_matrix->orn()).normalize();
               phys_hit->dist=phys_hit->frac*move.length();
               phys_hit->plane.set(pos+move*phys_hit->frac, phys_hit->face_nrm);
            }
            return callback.hit;
         }break;
      }
   #endif
   }
   return false;
}
/******************************************************************************/
#if PHYSX
/******************************************************************************/
#define INITIAL_OVERLAP_ON  PxHitFlags()
#define INITIAL_OVERLAP_OFF PxHitFlag::eASSUME_NO_INITIAL_OVERLAP

#define INITIAL_OVERLAP INITIAL_OVERLAP_OFF // this is used only for sweeps

void ActorInfo::set(PxShape *shape)
{
   if(T._actor=(shape ? shape->getActor() : null))
   {
      PxRigidDynamic *rigid_dynamic=_actor->is<PxRigidDynamic>();

      collision=                   FlagTest((UInt)        shape->getFlags         (),     PxShapeFlag::eSIMULATION_SHAPE) ;
      dynamic  =(rigid_dynamic && !FlagTest((UInt)rigid_dynamic->getRigidBodyFlags(), PxRigidBodyFlag::eKINEMATIC       ));
      group    =      shape->getSimulationFilterData().word0;
      user     =     _actor->userData;
      obj      =(Ptr)_actor->getName();
   }else
   {
      collision=false;
      dynamic  =false;
      group    =0;
      user     =null;
      obj      =null;
   }
}
/******************************************************************************/
void PhysHitBasic::set(C PxLocationHit &hit, Flt move_length)
{
   face=hit.faceIndex;
   dist=hit.distance;
   frac=(move_length ? dist/move_length : 0);
            plane.pos   =Physx.vec(hit.position);
   face_nrm=plane.normal=Physx.vec(hit.normal  );
}
void PhysHitBasic::updateFace(C PxTriangleMeshGeometry &mesh, C Matrix3 *orn)
{
   if(mesh.triangleMesh && InRange(face, mesh.triangleMesh->getNbTriangles()))
   {
    C Vec *pos  =(Vec*)mesh.triangleMesh->getVertices();
      Vec  scale=Physx.vec(mesh.scale.scale);
      VecI ind;
      if(mesh.triangleMesh->getTriangleMeshFlags()&PxTriangleMeshFlag::e16_BIT_INDICES)ind=((VecUS*)mesh.triangleMesh->getTriangles())[face];
      else                                                                             ind=((VecI *)mesh.triangleMesh->getTriangles())[face];
             face_nrm =GetNormal(pos[ind.x]*scale, pos[ind.y]*scale, pos[ind.z]*scale);
      if(orn)face_nrm*=*orn;

      if(C PxU32 *remap=mesh.triangleMesh->getTrianglesRemap())face=remap[face]; // remap to original index if available, do this at the end because other operations operate on phys mesh index
   }
}
void PhysHit::set(C PxLocationHit &hit, Flt move_length)
{
      ActorInfo::set(hit.shape); // set this first, because we need to set '_actor'
   PhysHitBasic::set(hit, move_length);
   if(face>=0 && _actor /*&& hit.shape*/ && hit.shape->getGeometryType()==PxGeometryType::eTRIANGLEMESH) // no need to check for 'hit.shape' because we already check for '_actor' and it will be set only if shape is
   {
      PxTriangleMeshGeometry mesh;
      if(hit.shape->getTriangleMeshGeometry(mesh))updateFace(mesh, &Physx.orn(_actor->getGlobalPose().q));
   }
}
/******************************************************************************/
// CUTS
/******************************************************************************
struct CutsCallback : PxQueryFilterCallback
{
   virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, PxShape* shape, PxQueryFlags &filterFlags)
   {
      return ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
   }
   virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
   {
      return PxQueryHitType::eTOUCH;
   }
};
/******************************************************************************/
Bool PhysicsClass::cuts(C Box &box, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock lock(Physics._rws);
      return Physx.world->overlap(PxBoxGeometry(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f), Physx.matrix(box.center()), buf, qfd);
   }
   return false;
}
/******************************************************************************/
Bool PhysicsClass::cuts(C OBox &obox, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock lock(Physics._rws);
      return Physx.world->overlap(PxBoxGeometry(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f), Physx.matrix(Matrix(obox.center(), obox.matrix.orn())), buf, qfd);
   }
   return false;
}
/******************************************************************************/
Bool PhysicsClass::cuts(C Ball &ball, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock lock(Physics._rws);
      return Physx.world->overlap(PxSphereGeometry(ball.r), Physx.matrix(ball.pos), buf, qfd);
   }
   return false;
}
/******************************************************************************/
Bool PhysicsClass::cuts(C Capsule &capsule, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock lock(Physics._rws);
      return Physx.world->overlap(PxCapsuleGeometry(capsule.r, capsule.h*0.5f-capsule.r), Physx.matrix(Matrix().setPosRight(capsule.pos, capsule.up)), buf, qfd);
   }
   return false;
}
/******************************************************************************/
// CUTS CALLBACK
/******************************************************************************/
struct CutsCallbackAll : PxQueryFilterCallback
{
   PhysCutsCallback   *callback;
   Memt<C PxRigidActor*> actors;

   CutsCallbackAll(PhysCutsCallback &callback) : callback(&callback) {}

   virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &filterFlags)
   {
      return (callback && !actors.has(actor)) ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
   }
   virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
   {
      if(callback && hit.shape)
         if(PxRigidActor *actor=hit.shape->getActor())
            if(actors.include(actor))
      {
         ActorInfo actor; actor.set(hit.shape); if(!callback->hit(actor))callback=null;
      }
      return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
   }
};
/******************************************************************************/
void PhysicsClass::cuts(C Box &box, PhysCutsCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->overlap(PxBoxGeometry(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f), Physx.matrix(box.center()), buf, qfd, &CutsCallbackAll(callback));
   }
}
/******************************************************************************/
void PhysicsClass::cuts(C OBox &obox, PhysCutsCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->overlap(PxBoxGeometry(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f), Physx.matrix(Matrix(obox.center(), obox.matrix.orn())), buf, qfd, &CutsCallbackAll(callback));
   }
}
/******************************************************************************/
void PhysicsClass::cuts(C Ball &ball, PhysCutsCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->overlap(PxSphereGeometry(ball.r), Physx.matrix(ball.pos), buf, qfd, &CutsCallbackAll(callback));
   }
}
/******************************************************************************/
void PhysicsClass::cuts(C Capsule &capsule, PhysCutsCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->overlap(PxCapsuleGeometry(capsule.r, capsule.h*0.5f-capsule.r), Physx.matrix(Matrix().setPosRight(capsule.pos, capsule.up)), buf, qfd, &CutsCallbackAll(callback));
   }
}
/******************************************************************************/
// ACTOR CUTS
/******************************************************************************/
Bool Actor::cuts(UInt groups)C
{
   struct ActorCutsCallback : PxQueryFilterCallback
   {
    C PxRigidActor *actor;

      ActorCutsCallback(C PxRigidActor *actor) {T.actor=actor;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (actor!=T.actor) ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
      }
	   virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit) // this isn't used but must be defined due to abstract class
	   {
	      return PxQueryHitType::eTOUCH;
	   }
   };

   if(_actor && Physx.world && groups)
   {
      ActorCutsCallback acc(_actor); PxShape *shape;
      PxOverlapBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      REP(_actor->getNbShapes())if(_actor->getShapes(&shape, 1, i))switch(shape->getGeometryType())
      {
         case PxGeometryType::eBOX       : {PxBoxGeometry        box    ; if(shape->getBoxGeometry       (box    ))if(Physx.world->overlap(box    , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc))return true;} break;
         case PxGeometryType::eSPHERE    : {PxSphereGeometry     sphere ; if(shape->getSphereGeometry    (sphere ))if(Physx.world->overlap(sphere , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc))return true;} break;
         case PxGeometryType::eCAPSULE   : {PxCapsuleGeometry    capsule; if(shape->getCapsuleGeometry   (capsule))if(Physx.world->overlap(capsule, _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc))return true;} break;
         case PxGeometryType::eCONVEXMESH: {PxConvexMeshGeometry convex ; if(shape->getConvexMeshGeometry(convex ))if(Physx.world->overlap(convex , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc))return true;} break;
      }
   }
   return false;
}
/******************************************************************************/
void Actor::cuts(PhysCutsCallback &callback, UInt groups)C
{
   struct ActorCutsCallbackAll : PxQueryFilterCallback
   {
      Memt<C PxRigidActor*> actors;
      PhysCutsCallback     *callback;

      ActorCutsCallbackAll(PhysCutsCallback &callback, C PxRigidActor &actor) : callback(&callback) {T.actors.add(&actor);}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (callback && !actors.has(actor)) ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         if(callback)
            if(PxRigidActor *actor=hit.shape->getActor())
               if(actors.include(actor))
         {
            ActorInfo actor; actor.set(hit.shape); if(!callback->hit(actor))callback=null;
         }
         return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
      }
   };

   if(_actor && Physx.world && groups)
   {
      ActorCutsCallbackAll acc(callback, *_actor); PxShape *shape;
      PxOverlapBuffer      buf;
      PxQueryFilterData    qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eNO_BLOCK); qfd.data.word0=groups;
      ReadLock             lock(Physics._rws);
      REP(_actor->getNbShapes())if(_actor->getShapes(&shape, 1, i))switch(shape->getGeometryType())
      {
         case PxGeometryType::eBOX       : {PxBoxGeometry        box    ; if(shape->getBoxGeometry       (box    ))Physx.world->overlap(box    , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc);} break;
         case PxGeometryType::eSPHERE    : {PxSphereGeometry     sphere ; if(shape->getSphereGeometry    (sphere ))Physx.world->overlap(sphere , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc);} break;
         case PxGeometryType::eCAPSULE   : {PxCapsuleGeometry    capsule; if(shape->getCapsuleGeometry   (capsule))Physx.world->overlap(capsule, _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc);} break;
         case PxGeometryType::eCONVEXMESH: {PxConvexMeshGeometry convex ; if(shape->getConvexMeshGeometry(convex ))Physx.world->overlap(convex , _actor->getGlobalPose()*shape->getLocalPose(), buf, qfd, &acc);} break;
      }
   }
}
/******************************************************************************/
// RAY
/******************************************************************************/
Bool PhysicsClass::ray(C Vec &pos, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   /*struct RayCallback : PxQueryFilterCallback
   {
      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, PxShape *shape, PxQueryFlags &filterFlags)
      {
         return ? PxQueryHitType::eBLOCK : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         return PxQueryHitType::eBLOCK;
      }
   };*/

   if(Physx.world && groups)
   {
      Vec               dir   =move;
      Flt               length=dir.normalize();
      PxRaycastBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      if(phys_hit)
      {
         if(Physx.world->raycast(Physx.vec(pos), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX, qfd))
         {
            phys_hit->set(buf.block, length); return true;
         }
      }else
      {
         qfd.flags|=PxQueryFlag::eANY_HIT;
         return Physx.world->raycast(Physx.vec(pos), Physx.vec(dir), length, buf, PxHitFlags(), qfd);
      }
   }
   return false;
}
void PhysicsClass::ray(C Vec &pos, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   struct RayCallbackAll : PxQueryFilterCallback
   {
      Flt              move_length;
      PhysHitCallback *callback;

      RayCallbackAll(PhysHitCallback &callback, Flt move_length) : callback(&callback) {T.move_length=move_length;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return callback ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         if(callback)
         {
          C PxRaycastHit &ray_hit=(C PxRaycastHit&)hit;
            PhysHit       phys_hit; phys_hit.set(ray_hit, move_length); if(!callback->hit(phys_hit))callback=null;
         }
         return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
      }
   };

   if(Physx.world && groups)
   {
      Vec               dir   =move;
      Flt               length=dir.normalize();
      PxRaycastBuffer   buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->raycast(Physx.vec(pos), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::eMESH_MULTIPLE, qfd, &RayCallbackAll(callback, length));
   }
}
/******************************************************************************/
// SWEEP
/******************************************************************************
struct SweepCallback : PxQueryFilterCallback
{
   virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, PxShape *shape, PxQueryFlags &filterFlags)
   {
      return ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
   }
   virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
   {
      return PxQueryHitType::eTOUCH;
   }
};
/******************************************************************************/
Bool PhysicsClass::sweep(C Box &box, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize(); PxBoxGeometry geom(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f); PxTransform pose=Physx.matrix(box.center());
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      if(phys_hit)
      {
         if(Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, qfd)){phys_hit->set(buf.block, length); return true;}
      }else
      {
         qfd.flags|=PxQueryFlag::eANY_HIT;
         return Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_ANY | INITIAL_OVERLAP, qfd);
      }
   }
   return false;
}
Bool PhysicsClass::sweep(C OBox &obox, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize(); PxBoxGeometry geom(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f); PxTransform pose=Physx.matrix(Matrix(obox.center(), obox.matrix.orn()));
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      if(phys_hit)
      {
         if(Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, qfd)){phys_hit->set(buf.block, length); return true;}
      }else
      {
         qfd.flags|=PxQueryFlag::eANY_HIT;
         return Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_ANY | INITIAL_OVERLAP, qfd);
      }
   }
   return false;
}
Bool PhysicsClass::sweep(C Ball &ball, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize(); PxSphereGeometry geom(ball.r); PxTransform pose=Physx.matrix(ball.pos);
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      if(phys_hit)
      {
         if(Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, qfd)){phys_hit->set(buf.block, length); return true;}
      }else
      {
         qfd.flags|=PxQueryFlag::eANY_HIT;
         return Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_ANY | INITIAL_OVERLAP, qfd);
      }
   }
   return false;
}
Bool PhysicsClass::sweep(C Capsule &capsule, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize(); PxCapsuleGeometry geom(capsule.r, capsule.h*0.5f-capsule.r); PxTransform pose=Physx.matrix(Matrix().setPosRight(capsule.pos, capsule.up));
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      if(phys_hit)
      {
         if(Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, qfd)){phys_hit->set(buf.block, length); return true;}
      }else
      {
         qfd.flags|=PxQueryFlag::eANY_HIT;
         return Physx.world->sweep(geom, pose, Physx.vec(dir), length, buf, PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_ANY | INITIAL_OVERLAP, qfd);
      }
   }
   return false;
}
/******************************************************************************/
// SWEEP CALLBACK
/******************************************************************************/
struct SweepCallbackAll : PxQueryFilterCallback
{
   Flt              move_length;
   PhysHitCallback *callback;

   SweepCallbackAll(PhysHitCallback &callback, Flt move_length) : callback(&callback) {T.move_length=move_length;}

   virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
   {
      return callback ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
   }
   virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
   {
      if(callback)
      {
       C PxSweepHit &sweep_hit=(C PxSweepHit&)hit;
         PhysHit      phys_hit; phys_hit.set(sweep_hit, move_length); if(!callback->hit(phys_hit))callback=null;
      }
      return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
   }
};
/******************************************************************************/
void PhysicsClass::sweep(C Box &box, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize();
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->sweep(PxBoxGeometry(box.w()*0.5f, box.h()*0.5f, box.d()*0.5f), Physx.matrix(box.center()), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, qfd, &SweepCallbackAll(callback, length));
   }
}
void PhysicsClass::sweep(C OBox &obox, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize();
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->sweep(PxBoxGeometry(obox.box.w()*0.5f, obox.box.h()*0.5f, obox.box.d()*0.5f), Physx.matrix(Matrix(obox.center(), obox.matrix.orn())), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, qfd, &SweepCallbackAll(callback, length));
   }
}
void PhysicsClass::sweep(C Ball &ball, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize();
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->sweep(PxSphereGeometry(ball.r), Physx.matrix(ball.pos), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, qfd, &SweepCallbackAll(callback, length));
   }
}
void PhysicsClass::sweep(C Capsule &capsule, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize();
      PxSweepBuffer     buf;
      PxQueryFilterData qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=groups;
      ReadLock          lock(Physics._rws);
      Physx.world->sweep(PxCapsuleGeometry(capsule.r, capsule.h*0.5f-capsule.r), Physx.matrix(Matrix().setPosRight(capsule.pos, capsule.up)), Physx.vec(dir), length, buf, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, qfd, &SweepCallbackAll(callback, length));
   }
}
/******************************************************************************/
#if PX_PHYSICS_VERSION_MAJOR==3 && PX_PHYSICS_VERSION_MINOR==2 && PX_PHYSICS_VERSION_BUGFIX==2
#define MAX_ACTOR_SHAPES 32
/******************************************************************************/
static PxFilterData FilterData[MAX_ACTOR_SHAPES]; // use outside static because 'PxFilterData' has constructor (this can be global because it's never modified)

Bool Actor::sweep(C Vec &move, PhysHit *phys_hit)C
{
   struct ActorSweepCallback : PxQueryFilterCallback
   {
      UInt          groups;
      PxRigidActor *actor;

      ActorSweepCallback(PxRigidActor &actor, UInt groups) {T.actor=&actor; T.groups=groups;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (shape && actor!=shape->getActor() && (shape->getQueryFilterData().word0&groups)) ? PxQueryHitType::eBLOCK : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         return PxQueryHitType::eBLOCK; // return BLOCK so first result will be stored in the output buffer (touches would be ignored)
      }
   };

   if(_actor && Physx.world)
   {
      PxGeometryHolder  geom    [MAX_ACTOR_SHAPES];
    C PxGeometry       *geom_ptr[MAX_ACTOR_SHAPES];
      PxTransform       poses   [MAX_ACTOR_SHAPES];
      Int               shapes=_actor->getNbShapes(), geoms=0;
      PxShape          *shape;
      FREP(shapes)
      {
         if(!InRange(geoms, MAX_ACTOR_SHAPES))break;
         if(_actor->getShapes(&shape, 1, i))switch(shape->getGeometryType())
         {
            case PxGeometryType::eBOX       : if(shape->getBoxGeometry       (geom[geoms].box       ())){geom_ptr[geoms]=&geom[geoms].box       (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eSPHERE    : if(shape->getSphereGeometry    (geom[geoms].sphere    ())){geom_ptr[geoms]=&geom[geoms].sphere    (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eCAPSULE   : if(shape->getCapsuleGeometry   (geom[geoms].capsule   ())){geom_ptr[geoms]=&geom[geoms].capsule   (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eCONVEXMESH: if(shape->getConvexMeshGeometry(geom[geoms].convexMesh())){geom_ptr[geoms]=&geom[geoms].convexMesh(); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
         }
      }
      if(geoms)
      {
         Vec dir=move; Flt length=dir.normalize();
         ActorSweepCallback asc(*_actor, Physics.collisionGroups(group()));
         ReadLock           lock(Physics._rws);
         if(phys_hit)
         {
            PxSweepHit hit; if(Physx.world->sweepSingle(geom_ptr, poses, FilterData, geoms, Physx.vec(dir), length, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, hit, PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER, &asc)){phys_hit->set(hit, length); return true;}
         }else
         {
            PxQueryHit hit; return Physx.world->sweepAny(geom_ptr, poses, FilterData, geoms, Physx.vec(dir), length, PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, hit, PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER, &asc);
         }
      }
   }
   return false;
}
void Actor::sweep(C Vec &move, PhysHitCallback &callback)C
{
   struct ActorSweepCallbackAll : PxQueryFilterCallback
   {
      UInt             groups;
      Flt              move_length;
      PxRigidActor    *actor;
      PhysHitCallback *callback;

      ActorSweepCallbackAll(PhysHitCallback &callback, PxRigidActor &actor, UInt groups) : callback(&callback) {T.actor=&actor; T.groups=groups;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (callback && shape && actor!=shape->getActor() && (shape->getQueryFilterData().word0&groups)) ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         if(callback)
         {
          C PxSweepHit &sweep_hit=(C PxSweepHit&)hit;
            PhysHit      phys_hit; phys_hit.set(sweep_hit, move_length); if(!callback->hit(phys_hit))callback=null;
         }
         return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
      }
   };

   if(_actor && Physx.world)
   {
      PxGeometryHolder  geom    [MAX_ACTOR_SHAPES];
    C PxGeometry       *geom_ptr[MAX_ACTOR_SHAPES];
      PxTransform       poses   [MAX_ACTOR_SHAPES];
      Int               shapes=_actor->getNbShapes(), geoms=0;
      PxShape          *shape;
      FREP(shapes)
      {
         if(!InRange(geoms, MAX_ACTOR_SHAPES))break;
         if(_actor->getShapes(&shape, 1, i))switch(shape->getGeometryType())
         {
            case PxGeometryType::eBOX       : if(shape->getBoxGeometry       (geom[geoms].box       ())){geom_ptr[geoms]=&geom[geoms].box       (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eSPHERE    : if(shape->getSphereGeometry    (geom[geoms].sphere    ())){geom_ptr[geoms]=&geom[geoms].sphere    (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eCAPSULE   : if(shape->getCapsuleGeometry   (geom[geoms].capsule   ())){geom_ptr[geoms]=&geom[geoms].capsule   (); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
            case PxGeometryType::eCONVEXMESH: if(shape->getConvexMeshGeometry(geom[geoms].convexMesh())){geom_ptr[geoms]=&geom[geoms].convexMesh(); poses[geoms]=_actor->getGlobalPose()*shape->getLocalPose(); geoms++;} break;
         }
      }
      if(geoms)
      {
         Vec dir=move; Flt length=dir.normalize(); bool blocking;
         ActorSweepCallbackAll asc(callback, *_actor, Physics.collisionGroups(group()));
         ReadLock              lock(Physics._rws);
         Physx.world->sweepMultiple(geom_ptr, poses, FilterData, geoms, Physx.vec(dir), length, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, null, 0, blocking, PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER, &asc);
      }
   }
}
#else
// method taken from "PhysX SDK\Source\PhysXExtensions\src\ExtRigidBodyExt.cpp"
Bool Actor::sweep(C Vec &move, PhysHit *phys_hit, UInt groups)C
{
   struct ActorSweepCallback : PxQueryFilterCallback
   {
      UInt          groups;
      PxRigidActor *actor;

      ActorSweepCallback(PxRigidActor &actor, UInt groups) {T.actor=&actor; T.groups=groups;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (shape && actor!=T.actor && (shape->getQueryFilterData().word0&groups)) ? PxQueryHitType::eBLOCK : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         return PxQueryHitType::eBLOCK; // return BLOCK so first result will be stored in the output buffer (touches would be ignored)
      }
   };

   if(_actor && Physx.world && groups)
   {
      Vec                dir=move; Flt length=dir.normalize();
      Bool               have_hit=false; Flt dist_hit;
      PxSweepHit         closest_hit;
      ActorSweepCallback asc(*_actor, groups);
      PxQueryFilterData  qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER); qfd.data.word0=asc.groups; if(!phys_hit)qfd.flags|=PxQueryFlag::eANY_HIT;
      ReadLock           lock(Physics._rws);
      REP(_actor->getNbShapes())
      {
         PxShape *shape=null; _actor->getShapes(&shape, 1, i);
         if(shape)
         {
		      PxTransform   pose=_actor->getGlobalPose()*shape->getLocalPose();
		      PxSweepBuffer hit;
		      if(phys_hit)
		      {
		         if(Physx.world->sweep(shape->getGeometry().any(), pose, Physx.vec(dir), length, hit, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | INITIAL_OVERLAP, qfd, &asc))
		            if(!have_hit || hit.block.distance<dist_hit)
	            {
	               have_hit=true;
	               dist_hit=hit.block.distance;
	               Swap(closest_hit, hit.block);
	            }
		      }else
		      {
		         if(Physx.world->sweep(shape->getGeometry().any(), pose, Physx.vec(dir), length, hit, PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_ANY | INITIAL_OVERLAP, qfd, &asc))
		         {
		            return true;
		         }
		      }
         }
      }
      if(have_hit){phys_hit->set(closest_hit, length); return true;}
   }
   return false;
}
void Actor::sweep(C Vec &move, PhysHitCallback &callback, UInt groups)C
{
   struct ActorSweepCallbackAll : PxQueryFilterCallback
   {
      UInt             groups;
      Flt              move_length;
      PxRigidActor    *actor;
      PhysHitCallback *callback;

      ActorSweepCallbackAll(PhysHitCallback &callback, PxRigidActor &actor, UInt groups) : callback(&callback) {T.actor=&actor; T.groups=groups;}

      virtual PxQueryHitType::Enum preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
      {
         return (callback && shape && actor!=T.actor && (shape->getQueryFilterData().word0&groups)) ? PxQueryHitType::eTOUCH : PxQueryHitType::eNONE;
      }
      virtual PxQueryHitType::Enum postFilter(const PxFilterData &filterData, const PxQueryHit &hit)
      {
         if(callback)
         {
          C PxSweepHit &sweep_hit=(C PxSweepHit&)hit;
            PhysHit      phys_hit; phys_hit.set(sweep_hit, move_length); if(!callback->hit(phys_hit))callback=null;
         }
         return PxQueryHitType::eNONE; // return NONE so it won't need to be stored in the output buffer (which we don't specify)
      }
   };

   if(_actor && Physx.world && groups)
   {
      Vec dir=move; Flt length=dir.normalize();
      ActorSweepCallbackAll asc(callback, *_actor, groups);
      PxQueryFilterData     qfd(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER); qfd.data.word0=asc.groups;
      ReadLock              lock(Physics._rws);
      REP(_actor->getNbShapes())
      {
         PxShape *shape=null; _actor->getShapes(&shape, 1, i);
         if(shape)
         {
		      PxTransform   pose=_actor->getGlobalPose()*shape->getLocalPose();
		      PxSweepBuffer hit;
	         Physx.world->sweep(shape->getGeometry().any(), pose, Physx.vec(dir), length, hit, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eDISTANCE | PxHitFlag::eFACE_INDEX | PxHitFlag::ePRECISE_SWEEP | PxHitFlag::eMESH_MULTIPLE | INITIAL_OVERLAP, qfd, &asc);
         }
      }
   }
}
#endif
/******************************************************************************/
#else // BULLET
/******************************************************************************/
static Bool GetFaceNormal(C btBvhTriangleMeshShape &mesh, Int face, Int subpart, C Vec *scale, Vec &normal)
{
   Bool ok=false;
   if(C btStridingMeshInterface *smi=mesh.getMeshInterface())
   if(InRange(subpart, smi->getNumSubParts()))
   {
    C unsigned char *pos;
    C unsigned char *ind;
      int            vtxs, tris, vtx_stride, ind_stride;
      PHY_ScalarType vtx_type, ind_type;
      smi->getLockedReadOnlyVertexIndexBase((const unsigned char**)&pos, vtxs, vtx_type, vtx_stride, &ind, ind_stride, tris, ind_type, subpart);
      if(InRange(face, tris))
      {
         VecI v;
         switch(ind_type)
         {
            case PHY_SHORT  : v=((VecUS*)ind)[face]; break;
            case PHY_INTEGER: v=((VecI *)ind)[face]; break;
            default         : goto error;
         }
         switch(vtx_type)
         {
            case PHY_FLOAT:
            {
               Vec *p=(Vec*)pos;
               if(scale)normal=GetNormal(p[v.x]* *scale, p[v.y]* *scale, p[v.z]* *scale);
               else     normal=GetNormal(p[v.x]        , p[v.y]        , p[v.z]        );
               ok=true;
            }break;

            case PHY_DOUBLE:
            {
               VecD *p=(VecD*)pos;
               if(scale)normal=GetNormal(p[v.x]* *scale, p[v.y]* *scale, p[v.z]* *scale);
               else     normal=GetNormal(p[v.x]        , p[v.y]        , p[v.z]        );
               ok=true;
            }break;
         }
      }
   error:
      smi->unLockReadOnlyVertexBase(subpart);
   }
   return ok;
}
static Bool GetFaceNormal(C btScaledBvhTriangleMeshShape &scaled_mesh, Int face, Int subpart, Vec &normal)
{
   if(C btBvhTriangleMeshShape *mesh=scaled_mesh.getChildShape())
      return GetFaceNormal(*mesh, face, subpart, &Bullet.vec(scaled_mesh.getLocalScaling()), normal);
   return false;
}
static void GetFaceNormal(PhysHit &phys_hit, Int subpart)
{
   if(phys_hit.face>=0 && subpart>=0)
      if(C btCollisionShape *shape=phys_hit._actor->getCollisionShape())
         switch(shape->getShapeType())
   {
      case        TRIANGLE_MESH_SHAPE_PROXYTYPE: if(GetFaceNormal(*(      btBvhTriangleMeshShape*)shape, phys_hit.face, subpart, null, phys_hit.face_nrm)){phys_hit.face_nrm*=Bullet.matrix(phys_hit._actor->getWorldTransform().getBasis()); return;} break;
      case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE: if(GetFaceNormal(*(btScaledBvhTriangleMeshShape*)shape, phys_hit.face, subpart,       phys_hit.face_nrm)){phys_hit.face_nrm*=Bullet.matrix(phys_hit._actor->getWorldTransform().getBasis()); return;} break;
   }
   phys_hit.face_nrm=phys_hit.plane.normal;
}
/******************************************************************************/
void ActorInfo::set(RigidBody *actor)
{
   if(T._actor=actor)
   {
      collision=!FlagTest(actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE);
      dynamic  =!actor->isStaticOrKinematicObject();
      group    = actor->getBroadphaseProxy()->m_collisionFilterGroup;
      user     = actor->user;
      obj      = actor->obj;
   }else
   {
      collision=false;
      dynamic  =false;
      group    =0;
      user     =null;
      obj      =null;
   }
}
/******************************************************************************/
struct ContactResultCallback : btCollisionWorld::ContactResultCallback
{
   Bool hit;
   UInt groups;
   btCollisionObject *test;

	virtual bool needsCollision(btBroadphaseProxy *proxy0)C override
	{
	   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups);
	}
	virtual btScalar addSingleResult(btManifoldPoint &cp,	const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1)override
	{
    C btCollisionObject &colObj0=*colObj0Wrap->m_collisionObject,
                        &colObj1=*colObj1Wrap->m_collisionObject;
	   if((!test || &colObj0!=test) && !FlagTest(colObj0.getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)  // Actor.collision()==true
	   || (!test || &colObj1!=test) && !FlagTest(colObj1.getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
	   {
	      hit=true;
	   }
	   return cp.getDistance();
	}

	ContactResultCallback(UInt groups, btCollisionObject *test)
	{
	   T.hit   =false;
	   T.groups=groups;
	   T.test  =test;
	}
};
struct ContactResultCallbackAll : btCollisionWorld::ContactResultCallback
{
   Bool               hit;
   UInt               groups;
   PhysCutsCallback  *callback;
   btCollisionObject *test;

	virtual bool needsCollision(btBroadphaseProxy* proxy0)C override
	{
	   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups);
	}
	virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
	{
	   if(callback)
      {
       C btCollisionObject &colObj0=*colObj0Wrap->m_collisionObject,
                           &colObj1=*colObj1Wrap->m_collisionObject;
	      if(RigidBody *rb=(RigidBody*)((&colObj0!=test) ? colObj0 : colObj1).getUserPointer())
	         if(!FlagTest(rb->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
         {
            ActorInfo ai; ai.set(rb); if(!callback->hit(ai))callback=null;
         }
      }
	   return cp.getDistance();
	}

	ContactResultCallbackAll(UInt groups, PhysCutsCallback &callback, btCollisionObject &test)
	{
	   T.groups  = groups;
	   T.callback=&callback;
	   T.test    =&test;
	}
};
/******************************************************************************/
Bool PhysicsClass::cuts(C Box &box, UInt groups)
{
   if(Bullet.world)
   {
      ContactResultCallback crc(groups, null);
      btCollisionObject     object; object.setUserPointer(null);
      btBoxShape            shape(Bullet.vec(box.size()*0.5f));

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(box.center())));
      ReadLock lock(Physics._rws);
      Bullet.world->contactTest(&object, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::cuts(C OBox &obox, UInt groups)
{
   if(Bullet.world)
   {
      ContactResultCallback crc(groups, null);
      btCollisionObject     object; object.setUserPointer(null);
      btBoxShape            shape(Bullet.vec(obox.box.size()*0.5f));

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(obox.matrix.orn(), obox.center())));
      ReadLock lock(Physics._rws);
      Bullet.world->contactTest(&object, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::cuts(C Ball &ball, UInt groups)
{
   if(Bullet.world)
   {
      ContactResultCallback crc(groups, null);
      btCollisionObject     object; object.setUserPointer(null);
      btSphereShape         shape(ball.r);

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(ball.pos)));
      ReadLock lock(Physics._rws);
      Bullet.world->contactTest(&object, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::cuts(C Capsule &capsule, UInt groups)
{
   if(Bullet.world)
   {
      ContactResultCallback crc(groups, null);
      btCollisionObject     object; object.setUserPointer(null);
      btCapsuleShape        shape(capsule.r, capsule.h-capsule.r*2);

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix().setPosUp(capsule.pos, capsule.up)));
      ReadLock lock(Physics._rws);
      Bullet.world->contactTest(&object, crc);
      return crc.hit;
   }
   return false;
}
/******************************************************************************/
void PhysicsClass::cuts(C Box &box, PhysCutsCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btCollisionObject object; object.setUserPointer(null);
      btBoxShape        shape(Bullet.vec(box.size()*0.5f));

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(box.center())));
      ContactResultCallbackAll cb(groups, callback, object); ReadLock lock(Physics._rws); Bullet.world->contactTest(&object, cb);
   }
}
void PhysicsClass::cuts(C OBox &obox, PhysCutsCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btCollisionObject object; object.setUserPointer(null);
      btBoxShape        shape(Bullet.vec(obox.box.size()*0.5f));

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(obox.matrix.orn(), obox.center())));
      ContactResultCallbackAll cb(groups, callback, object); ReadLock lock(Physics._rws); Bullet.world->contactTest(&object, cb);
   }
}
void PhysicsClass::cuts(C Ball &ball, PhysCutsCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btCollisionObject object; object.setUserPointer(null);
      btSphereShape     shape(ball.r);

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix(ball.pos)));
      ContactResultCallbackAll cb(groups, callback, object); ReadLock lock(Physics._rws); Bullet.world->contactTest(&object, cb);
   }
}
void PhysicsClass::cuts(C Capsule &capsule, PhysCutsCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btCollisionObject object; object.setUserPointer(null);
      btCapsuleShape    shape(capsule.r, capsule.h-capsule.r*2);

      object.setCollisionShape(&shape);
      object.setWorldTransform(Bullet.matrix(Matrix().setPosUp(capsule.pos, capsule.up)));
      ContactResultCallbackAll cb(groups, callback, object); ReadLock lock(Physics._rws); Bullet.world->contactTest(&object, cb);
   }
}
/******************************************************************************/
Bool PhysicsClass::ray(C Vec &pos, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   struct RayResultCallback : btCollisionWorld::RayResultCallback
   {
      Bool     hit;
      UInt     groups;
      PhysHit *phys_hit;
      Vec      pos, move;
   
		virtual bool needsCollision(btBroadphaseProxy* proxy0) const
		{
		   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups) && proxy0->m_collisionFilterMask!=0; // Actor::ray()==true
		}
		virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
		{
		   if(RigidBody *actor=(RigidBody*)rayResult.m_collisionObject)
		   {
		      if(phys_hit)if(!hit || rayResult.m_hitFraction<phys_hit->frac)
	         {
	            phys_hit->set(actor);
	            phys_hit->face=(rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_triangleIndex : -1);
	            phys_hit->frac=rayResult.m_hitFraction;
	            phys_hit->dist=rayResult.m_hitFraction*move.length();
	            phys_hit->plane.set(pos+move*rayResult.m_hitFraction, Bullet.vec(normalInWorldSpace ? rayResult.m_hitNormalLocal : rayResult.m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal));
	            GetFaceNormal(*phys_hit, rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_shapePart : -1);
		      }
		      hit=true;
		   }
		   return rayResult.m_hitFraction;
		}
		
		RayResultCallback(PhysHit *phys_hit, C Vec &pos, C Vec &move, UInt groups)
		{
		   T.hit     =false;
		   T.groups  =groups;
		   T.phys_hit=phys_hit;
		   T.pos     =pos;
		   T.move    =move;
		}
   };

   if(Bullet.world)
   {
      RayResultCallback rrc(phys_hit, pos, move, groups);
      ReadLock          lock(Physics._rws);
      Bullet.world->rayTest(Bullet.vec(pos), Bullet.vec(pos+move), rrc);
      return rrc.hit;
   }
   return false;
}
void PhysicsClass::ray(C Vec &pos, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   struct RayResultCallbackAll : btCollisionWorld::RayResultCallback
   {
      PhysHit          phys_hit;
      UInt             groups;
      Vec              pos, move;
      PhysHitCallback *callback;
   
		virtual bool needsCollision(btBroadphaseProxy* proxy0) const
		{
		   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups) && proxy0->m_collisionFilterMask!=0; // Actor::ray()==true
		}
		virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
		{
		   if(callback)if(RigidBody *actor=(RigidBody*)rayResult.m_collisionObject)
		   {
	         phys_hit.set(actor);
            phys_hit.face=(rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_triangleIndex : -1);
            phys_hit.frac=rayResult.m_hitFraction;
            phys_hit.dist=rayResult.m_hitFraction*move.length();
            phys_hit.plane.set(pos+move*rayResult.m_hitFraction, Bullet.vec(normalInWorldSpace ? rayResult.m_hitNormalLocal : rayResult.m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal));
	         GetFaceNormal(phys_hit, rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_shapePart : -1);
		      if(!callback->hit(phys_hit))callback=null;
		   }
		   return rayResult.m_hitFraction;
		}

		RayResultCallbackAll(C Vec &pos, C Vec &move, PhysHitCallback &callback, UInt groups)
		{
		   T.groups  = groups;
		   T.pos     = pos;
		   T.move    = move;
		   T.callback=&callback;
		}
   };

   if(Bullet.world)
   {
      RayResultCallbackAll cb(pos, move, callback, groups);
      ReadLock             lock(Physics._rws);
      Bullet.world->rayTest(Bullet.vec(pos), Bullet.vec(pos+move), cb);
   }
}
/******************************************************************************/
struct ConvexResultCallback : btCollisionWorld::ConvexResultCallback
{
   Bool     hit;
   UInt     groups;
   Vec      move;
   PhysHit *phys_hit;

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
	{
	   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups);
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
	   if(RigidBody *actor=(RigidBody*)convexResult.m_hitCollisionObject)
	      if(!FlagTest(actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
	   {
	      if(phys_hit)if(!hit || convexResult.m_hitFraction<phys_hit->frac)
         {
	         phys_hit->set(actor);
            phys_hit->face=(convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_triangleIndex : -1);
            phys_hit->frac=convexResult.m_hitFraction;
            phys_hit->dist=convexResult.m_hitFraction*move.length();
            phys_hit->plane.set(Bullet.vec(convexResult.m_hitPointLocal), Bullet.vec(normalInWorldSpace ? convexResult.m_hitNormalLocal : convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal));
	         GetFaceNormal(*phys_hit, convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_shapePart : -1);
	      }
	      hit=true;
	   }
	   return convexResult.m_hitFraction;
	}
	
	ConvexResultCallback(C Vec &move, PhysHit *phys_hit, UInt groups)
	{
	   T.hit     =false;
	   T.move    =move;
	   T.phys_hit=phys_hit;
	   T.groups  =groups;
	}
};
Bool PhysicsClass::sweep(C Box &box, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(box.center())),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallback crc(move, phys_hit, groups);
      ReadLock             lock(Physics._rws);
      Bullet.world->convexSweepTest(&btBoxShape(Bullet.vec(box.size()*0.5f)), from, to, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::sweep(C OBox &obox, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(obox.matrix.orn(), obox.center())),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallback crc(move, phys_hit, groups);
      ReadLock             lock(Physics._rws);
      Bullet.world->convexSweepTest(&btBoxShape(Bullet.vec(obox.box.size()*0.5f)), from, to, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::sweep(C Ball &ball, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(ball.pos)),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallback crc(move, phys_hit, groups);
      ReadLock             lock(Physics._rws);
      Bullet.world->convexSweepTest(&btSphereShape(ball.r), from, to, crc);
      return crc.hit;
   }
   return false;
}
Bool PhysicsClass::sweep(C Capsule &capsule, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix().setPosUp(capsule.pos, capsule.up)),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallback crc(move, phys_hit, groups);
      ReadLock             lock(Physics._rws);
      Bullet.world->convexSweepTest(&btCapsuleShape(capsule.r, capsule.h-capsule.r*2), from, to, crc);
      return crc.hit;
   }
   return false;
}
/******************************************************************************/
struct ConvexResultCallbackAll : btCollisionWorld::ConvexResultCallback
{
   UInt             groups;
   Vec              move;
   PhysHit          phys_hit;
   PhysHitCallback *callback;

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
	{
	   return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups);
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
	   if(callback)
	      if(RigidBody *actor=(RigidBody*)convexResult.m_hitCollisionObject)
	         if(!FlagTest(actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
	   {
	      phys_hit.set(actor);
         phys_hit.face=(convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_triangleIndex : -1);
         phys_hit.frac=convexResult.m_hitFraction;
         phys_hit.dist=convexResult.m_hitFraction*move.length();
         phys_hit.plane.set(Bullet.vec(convexResult.m_hitPointLocal), Bullet.vec(normalInWorldSpace ? convexResult.m_hitNormalLocal : convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal));
         GetFaceNormal(phys_hit, convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_shapePart : -1);
         if(!callback->hit(phys_hit))callback=null;
	   }
	   return convexResult.m_hitFraction;
	}
	
	ConvexResultCallbackAll(C Vec &move, PhysHitCallback &callback, UInt groups)
	{
	   T.move    = move;
	   T.callback=&callback;
	   T.groups  = groups;
	}
};
void PhysicsClass::sweep(C Box &box, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(box.center())),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallbackAll cb(move, callback, groups); ReadLock lock(Physics._rws); Bullet.world->convexSweepTest(&btBoxShape(Bullet.vec(box.size()*0.5f)), from, to, cb);
   }
}
void PhysicsClass::sweep(C OBox &obox, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(obox.matrix.orn(), obox.center())),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallbackAll cb(move, callback, groups); ReadLock lock(Physics._rws); Bullet.world->convexSweepTest(&btBoxShape(Bullet.vec(obox.box.size()*0.5f)), from, to, cb);
   }
}
void PhysicsClass::sweep(C Ball &ball, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix(ball.pos)),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallbackAll cb(move, callback, groups); ReadLock lock(Physics._rws); Bullet.world->convexSweepTest(&btSphereShape(ball.r), from, to, cb);
   }
}
void PhysicsClass::sweep(C Capsule &capsule, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   if(Bullet.world)
   {
      btTransform from=Bullet.matrix(Matrix().setPosUp(capsule.pos, capsule.up)),
                  to  =from; to.getOrigin()+=Bullet.vec(move);
      ConvexResultCallbackAll cb(move, callback, groups); ReadLock lock(Physics._rws); Bullet.world->convexSweepTest(&btCapsuleShape(capsule.r, capsule.h-capsule.r*2), from, to, cb);
   }
}
/******************************************************************************/
Bool Actor::cuts(UInt groups)C
{
   if(_actor && Bullet.world)
   {
      ContactResultCallback crc(groups, _actor);
      ReadLock              lock(Physics._rws);
      Bullet.world->contactTest(_actor, crc);
      return crc.hit;
   }
   return false;
}
void Actor::cuts(PhysCutsCallback &callback, UInt groups)C
{
   if(_actor && Bullet.world)
   {
      ContactResultCallbackAll cb(groups, callback, *_actor); ReadLock lock(Physics._rws); Bullet.world->contactTest(_actor, cb);
   }
}
Bool Actor::sweep(C Vec &move, PhysHit *phys_hit, UInt groups)C
{
   struct ActorConvexResultCallback : btCollisionWorld::ConvexResultCallback
   {
      Bool     hit;
      UInt     groups;
    C Actor   *actor;
      PhysHit *phys_hit;
      Vec      move;

      virtual bool needsCollision(btBroadphaseProxy* proxy0) const
      {
         return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups) // collisions enabled between groups
             && actor->_actor->getBroadphaseProxy()!=proxy0;                 // this is not the same actor
      }

      virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
      {
         if(RigidBody *actor=(RigidBody*)convexResult.m_hitCollisionObject)
	         if(!FlagTest(actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
         {
            if(phys_hit)if(!hit || convexResult.m_hitFraction<phys_hit->frac)
            {
	            phys_hit->set(actor);
               phys_hit->face=(convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_triangleIndex : -1);
               phys_hit->frac=convexResult.m_hitFraction;
               phys_hit->dist=convexResult.m_hitFraction*move.length();
               phys_hit->plane.set(Bullet.vec(convexResult.m_hitPointLocal), Bullet.vec(normalInWorldSpace ? convexResult.m_hitNormalLocal : convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal));
               GetFaceNormal(*phys_hit, convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_shapePart : -1);
            }
            hit=true;
         }
         return convexResult.m_hitFraction;
      }
   	
      ActorConvexResultCallback(C Actor &actor, PhysHit *phys_hit, C Vec &move, UInt groups)
      {
         T.hit     = false   ;
         T.groups  = groups  ;
         T.actor   =&actor   ;
         T.phys_hit= phys_hit;
         T.move    = move    ;
      }
      
      Bool test(btConvexShape *convex, C Matrix &matrix)
      {
         btTransform from=Bullet.matrix(matrix),
                     to  =from; to.getOrigin()+=Bullet.vec(move);
         Bullet.world->convexSweepTest(convex, from, to, T);
         return hit;
      }
   };

   if(_actor && Bullet.world && groups)
   {
      ActorConvexResultCallback crc(T, phys_hit, move, groups);
      Matrix                    matrix=T.matrix();
      ReadLock                  lock(Physics._rws);

      if(btCompoundShape *compound=CAST(btCompoundShape, _actor->getCollisionShape())) // built from many shapes
      {
         REP(compound->getNumChildShapes()) // test all shapes
            if(btConvexShape *convex=CAST(btConvexShape, compound->getChildShape(i))) // if it's convex
               if(crc.test(convex, Bullet.matrix(compound->getChildTransform(i))*matrix)) // if collision has encountered
                  if(!phys_hit)return true; //  if we're not interested in precise 'phys_hit' info, then return on first collision
      }else
      if(btConvexShape *convex=CAST(btConvexShape, _actor->getCollisionShape())) // if it's convex
      {
         crc.test(convex, matrix);
      }

      return crc.hit;
   }
   return false;
}
void Actor::sweep(C Vec &move, PhysHitCallback &callback, UInt groups)C
{
   struct ActorConvexResultCallbackAll : btCollisionWorld::ConvexResultCallback
   {
      Vec              move;
      UInt             groups;
    C Actor           *actor;
      PhysHit          phys_hit;
      PhysHitCallback *callback;

      virtual bool needsCollision(btBroadphaseProxy* proxy0) const
      {
         return Physics.collidesMask(proxy0->m_collisionFilterGroup, groups) // collisions enabled between groups
             && actor->_actor->getBroadphaseProxy()!=proxy0;                 // this is not the same actor
      }

      virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
      {
         if(callback)
            if(RigidBody *actor=(RigidBody*)convexResult.m_hitCollisionObject)
	            if(!FlagTest(actor->getCollisionFlags(), btCollisionObject::CF_NO_CONTACT_RESPONSE)) // Actor.collision()==true
         {
	         phys_hit.set(actor);
            phys_hit.face=(convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_triangleIndex : -1);
            phys_hit.frac=convexResult.m_hitFraction;
            phys_hit.dist=convexResult.m_hitFraction*move.length();
            phys_hit.plane.set(Bullet.vec(convexResult.m_hitPointLocal), Bullet.vec(normalInWorldSpace ? convexResult.m_hitNormalLocal : convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal));
            GetFaceNormal(phys_hit, convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_shapePart : -1);
            if(!callback->hit(phys_hit))callback=null;
         }
         return convexResult.m_hitFraction;
      }

      ActorConvexResultCallbackAll(C Actor &actor, C Vec &move, UInt groups, PhysHitCallback &callback)
      {
         T.groups  = groups  ;
         T.actor   =&actor   ;
         T.move    = move    ;
         T.callback=&callback;
      }

      void test(btConvexShape *convex, C Matrix &matrix)
      {
         btTransform from=Bullet.matrix(matrix),
                     to  =from; to.getOrigin()+=Bullet.vec(move);
         Bullet.world->convexSweepTest(convex, from, to, T);
      }
   };

   if(_actor && Bullet.world && groups)
   {
      ActorConvexResultCallbackAll crc(T, move, groups, callback);
      Matrix                       matrix=T.matrix();
      ReadLock                     lock(Physics._rws);

      if(btCompoundShape *compound=CAST(btCompoundShape, _actor->getCollisionShape())) // built from many shapes
      {
         REP(compound->getNumChildShapes()) // test all shapes
            if(btConvexShape *convex=CAST(btConvexShape, compound->getChildShape(i))) // if it's convex
               crc.test(convex, Bullet.matrix(compound->getChildTransform(i))*matrix);
      }else
      if(btConvexShape *convex=CAST(btConvexShape, _actor->getCollisionShape())) // if it's convex
      {
         crc.test(convex, matrix);
      }
   }
}
/******************************************************************************/
#endif
/******************************************************************************/
Bool PhysicsClass::cuts(C Shape &shape, UInt groups)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : return cuts(shape.box    , groups);
      case SHAPE_OBOX   : return cuts(shape.obox   , groups);
      case SHAPE_BALL   : return cuts(shape.ball   , groups);
      case SHAPE_CAPSULE: return cuts(shape.capsule, groups);
      default           : return false;
   }
}
/******************************************************************************/
void PhysicsClass::cuts(C Shape &shape, PhysCutsCallback &callback, UInt groups)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : cuts(shape.box    , callback, groups); break;
      case SHAPE_OBOX   : cuts(shape.obox   , callback, groups); break;
      case SHAPE_BALL   : cuts(shape.ball   , callback, groups); break;
      case SHAPE_CAPSULE: cuts(shape.capsule, callback, groups); break;
   }
}
/******************************************************************************/
Bool PhysicsClass::sweep(C Shape &shape, C Vec &move, PhysHit *phys_hit, UInt groups)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : return sweep(shape.box    , move, phys_hit, groups);
      case SHAPE_OBOX   : return sweep(shape.obox   , move, phys_hit, groups);
      case SHAPE_BALL   : return sweep(shape.ball   , move, phys_hit, groups);
      case SHAPE_CAPSULE: return sweep(shape.capsule, move, phys_hit, groups);
      default           : return false;
   }
}
/******************************************************************************/
void PhysicsClass::sweep(C Shape &shape, C Vec &move, PhysHitCallback &callback, UInt groups)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : sweep(shape.box    , move, callback, groups); break;
      case SHAPE_OBOX   : sweep(shape.obox   , move, callback, groups); break;
      case SHAPE_BALL   : sweep(shape.ball   , move, callback, groups); break;
      case SHAPE_CAPSULE: sweep(shape.capsule, move, callback, groups); break;
   }
}
/******************************************************************************/
// MOVE
/******************************************************************************/
void PhysicsClass::move(Box     &box    , C Vec &move, UInt groups) {PhysHit hit; if(sweep(box    , move, &hit, groups))box    +=move*hit.frac;else box    +=move;}
void PhysicsClass::move(OBox    &obox   , C Vec &move, UInt groups) {PhysHit hit; if(sweep(obox   , move, &hit, groups))obox   +=move*hit.frac;else obox   +=move;}
void PhysicsClass::move(Ball    &ball   , C Vec &move, UInt groups) {PhysHit hit; if(sweep(ball   , move, &hit, groups))ball   +=move*hit.frac;else ball   +=move;}
void PhysicsClass::move(Capsule &capsule, C Vec &move, UInt groups) {PhysHit hit; if(sweep(capsule, move, &hit, groups))capsule+=move*hit.frac;else capsule+=move;}
void PhysicsClass::move(Shape   &shape  , C Vec &move, UInt groups) {PhysHit hit; if(sweep(shape  , move, &hit, groups))shape  +=move*hit.frac;else shape  +=move;}
/******************************************************************************/
}
/******************************************************************************/
