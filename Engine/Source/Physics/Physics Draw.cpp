/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static void DrawTris(Vec *pos, VecI *tri, Int tris, C Color &color, Bool fill)
{
	if(pos && tri && tris)
	{
	   VI.color(color);
	   if(fill)
	   {
         for(; tris--; )
         {
            Int *p=(tri++)->c;
            VI.tri(pos[p[0]], pos[p[1]], pos[p[2]]);
         }
	   }else
	   {
         for(; tris--; )
         {
            Int *p=(tri++)->c, p0=p[0], p1=p[1], p2=p[2];
            VI.line(pos[p0], pos[p1]);
            VI.line(pos[p1], pos[p2]);
            VI.line(pos[p2], pos[p0]);
         }
	   }
	   VI.end();
	}
}
static void DrawTris16(Vec *pos, VecUS *tri, Int tris, C Color &color, Bool fill)
{
	if(pos && tri && tris)
	{
	   VI.color(color);
	   if(fill)
	   {
         for(; tris--; )
         {
            UShort *p=(tri++)->c;
            VI.tri(pos[p[0]], pos[p[1]], pos[p[2]]);
         }
	   }else
	   {
         for(; tris--; )
         {
            UShort *p=(tri++)->c, p0=p[0], p1=p[1], p2=p[2];
            VI.line(pos[p0], pos[p1]);
            VI.line(pos[p1], pos[p2]);
            VI.line(pos[p2], pos[p0]);
         }
	   }
	   VI.end();
	}
}
/******************************************************************************/
#if PHYSX
/******************************************************************************/
void DrawMesh(PxTriangleMesh &mesh, C Color &color, Bool fill)
{
   if(mesh.getTriangleMeshFlags()&PxTriangleMeshFlag::e16_BIT_INDICES)DrawTris16((Vec*)mesh.getVertices(), (VecUS*)mesh.getTriangles(), mesh.getNbTriangles(), color, fill);
   else                                                               DrawTris  ((Vec*)mesh.getVertices(), (VecI *)mesh.getTriangles(), mesh.getNbTriangles(), color, fill);
}
void DrawConvex(PxConvexMesh &convex, C Color &color, Bool fill)
{
   const PxU8 *ind=            convex.getIndexBuffer();
   const Vec  *vtx=(const Vec*)convex.getVertices   ();
   PxHullPolygon poly;
   VI.color(color);
   REP(convex.getNbPolygons())
      if(convex.getPolygonData(i, poly))
         REP(poly.mNbVerts)
   {
      VI.line(vtx[ind[poly.mIndexBase+i]], vtx[ind[poly.mIndexBase+(i+1)%poly.mNbVerts]]);
   }
   VI.end();
}
/******************************************************************************/
static void DrawShape(PxShape *shape, C Color &color, Bool fill, C Matrix &actor_matrix)
{
   if(shape)
   {
      Matrix matrix=Physx.matrix(shape->getLocalPose())*actor_matrix;
      switch(shape->getGeometryType())
      {
         case PxGeometryType::ePLANE       : {PxPlaneGeometry        plane  ; if(shape->getPlaneGeometry       (plane  )){SetMatrix(matrix); Plane  (VecZero, Vec(1,0,0)).drawInfiniteByResolution(color);}} break;
         case PxGeometryType::eSPHERE      : {PxSphereGeometry       sphere ; if(shape->getSphereGeometry      (sphere )){SetMatrix(matrix); Ball   (sphere.radius).draw(color, fill);}} break;
         case PxGeometryType::eCAPSULE     : {PxCapsuleGeometry      capsule; if(shape->getCapsuleGeometry     (capsule)){SetMatrix(matrix); Capsule(capsule.radius, (capsule.halfHeight+capsule.radius)*2, VecZero, Vec(1,0,0)).draw(color, fill);}} break;
         case PxGeometryType::eBOX         : {PxBoxGeometry          box    ; if(shape->getBoxGeometry         (box    )){SetMatrix(matrix); Box    (Vec(-box.halfExtents.x, -box.halfExtents.y, -box.halfExtents.z), Vec(box.halfExtents.x, box.halfExtents.y, box.halfExtents.z)).draw(color, fill);}} break;
         case PxGeometryType::eCONVEXMESH  : {PxConvexMeshGeometry   convex ; if(shape->getConvexMeshGeometry  (convex )){SetMatrix(Matrix().setScale(Physx.vec(convex.scale.scale))*matrix); DrawConvex(*convex.  convexMesh, color, fill);}} break;
         case PxGeometryType::eTRIANGLEMESH: {PxTriangleMeshGeometry mesh   ; if(shape->getTriangleMeshGeometry(mesh   )){SetMatrix(Matrix().setScale(Physx.vec(mesh  .scale.scale))*matrix); DrawMesh  (*mesh  .triangleMesh, color, fill);}} break;
         case PxGeometryType::eHEIGHTFIELD : break;
      }
   }
}
static void ActorDraw(PxActor *actor, C Color &color, Bool fill)
{
   if(actor)
      if(Frustum(Physx.box(actor->getWorldBounds())))
   {
      if(PxRigidActor *rigid=actor->is<PxRigidActor>())
      {
         Matrix matrix=Physx.matrix(rigid->getGlobalPose());
         Int    shapes=rigid->getNbShapes();
         REP(shapes)
         {
            PxShape *shape=null;
            if(rigid->getShapes(&shape, 1, i))DrawShape(shape, color, fill, matrix);
         }
      }
   }
}
void Actor       ::draw(C Color &color, Bool fill)C {ActorDraw(_actor, color, fill);}
void PhysicsClass::draw()
{
   ReadLock lock(_rws);
   if(Physx.world)
   {
      PxActorTypeFlags types=PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
      Int actors=Physx.world->getNbActors(types);
      REP(actors)
      {
         PxActor *actor=null;
         if(Physx.world->getActors(types, &actor, 1, i))ActorDraw(actor, WHITE, false);
      }
   }
}
/******************************************************************************/
#else // BULLET
/******************************************************************************/
void DrawConvex(btConvexHullShape &convex, C Color &color, Bool fill)
{
   VI.color(color);
   REP(convex.getNumEdges   ()){btVector3 a, b; convex.getEdge(i, a, b); VI.line(Bullet.vec(a), Bullet.vec(b));}
 //REP(convex.getNumVertices()){btVector3 v;  convex.getVertex(i, v   ); VI.dot (Bullet.vec(v));}
 //REP(convex.getNumPoints  ())VI.dot(Bullet.vec(convex.getUnscaledPoints()[i]));
   VI.end();
}
void DrawMesh(btBvhTriangleMeshShape &mesh, C Color &color, Bool fill)
{
   if(btStridingMeshInterface *smi=mesh.getMeshInterface())
   {
      Vec           *pos;
      VecI          *ind;
      int            vtxs, tris, vtx_stride, ind_stride;
      PHY_ScalarType vtx_type, ind_type;
      int            subpart=0;
      smi->getLockedReadOnlyVertexIndexBase((const unsigned char**)&pos, vtxs, vtx_type, vtx_stride, (const unsigned char**)&ind, ind_stride, tris, ind_type, subpart);
      if(vtx_type==PHY_FLOAT && ind_type==PHY_INTEGER)
      {
         VI.color(color);
         REP(tris)
         {
            VecI v=ind[i];
            if(fill)VI.tri(pos[v.x], pos[v.y], pos[v.z]);else
            {
               VI.line(pos[v.x], pos[v.y]);
               VI.line(pos[v.y], pos[v.z]);
               VI.line(pos[v.z], pos[v.x]);
            }
         }
         VI.end();
      }
      smi->unLockReadOnlyVertexBase(subpart);
   }
}
static void Draw(btCollisionShape *shape, C Color &color, Bool fill, C Matrix &matrix)
{
   if(shape)
   {
      if(btCompoundShape              *compound=CAST(btCompoundShape             , shape)){REP(compound->getNumChildShapes())Draw(compound->getChildShape(i), color, fill, Bullet.matrix(compound->getChildTransform(i))*matrix);}else
      if(btUniformScalingShape        *uni     =CAST(btUniformScalingShape       , shape))Draw(uni ->getChildShape(), color, fill, Matrix(matrix).scaleOrn (           uni ->getUniformScalingFactor() ));else
      if(btScaledBvhTriangleMeshShape *mesh    =CAST(btScaledBvhTriangleMeshShape, shape))Draw(mesh->getChildShape(), color, fill, Matrix(matrix).scaleOrnL(Bullet.vec(mesh->getLocalScaling        ())));else
      {
         SetMatrix(matrix);

         if(btBoxShape             *box    =CAST(btBoxShape            , shape)){Vec ext=Bullet.vec(box->getHalfExtentsWithMargin()); Box(-ext, ext).draw(color, fill);}else
         if(btSphereShape          *ball   =CAST(btSphereShape         , shape)){Ball(ball->getRadius()).draw(color, fill);}else
         if(btCapsuleShape         *capsule=CAST(btCapsuleShape        , shape)){Flt r=capsule->getRadius(); Capsule(r, (capsule->getHalfHeight()+r)*2).draw(color, fill);}else
         if(btCylinderShape        *tube   =CAST(btCylinderShape       , shape)){Tube(tube->getRadius(), tube->getHalfExtentsWithMargin().y()*2).draw(color, fill);}else
         if(btStaticPlaneShape     *plane  =CAST(btStaticPlaneShape    , shape)){Vec n=Bullet.vec(plane->getPlaneNormal()); Plane(plane->getPlaneConstant()*n, n).drawInfiniteByResolution(color);}else
         if(btConvexHullShape      *convex =CAST(btConvexHullShape     , shape))DrawConvex(*convex, color, fill);else
         if(btBvhTriangleMeshShape *mesh   =CAST(btBvhTriangleMeshShape, shape))DrawMesh  (*mesh  , color, fill);
      }
   }
}
void Actor       ::draw(C Color &color, Bool fill)C {if(_actor)Draw(_actor->getCollisionShape(), color, fill, massCenterMatrix());}
void PhysicsClass::draw(                         )
{
   ReadLock lock(_rws);

   if(Bullet.world)
      REP(Bullet.world->getNumCollisionObjects())
	      if(btCollisionObject *obj=Bullet.world->getCollisionObjectArray()[i])
	         Draw(obj->getCollisionShape(), WHITE, false, Bullet.matrix(obj->getWorldTransform()));
}
#endif
/******************************************************************************/
void DrawPhys(C MeshBase &base, C Color &color, Bool fill)
{
   if(C Vec *pos=base.vtx.pos())
   {
      VI.color(color);
      if(fill)
      {
         REPA(base.tri ){C VecI  &ind=base.tri .ind(i); VI.tri (pos[ind.x], pos[ind.y], pos[ind.z]);}
         REPA(base.quad){C VecI4 &ind=base.quad.ind(i); VI.quad(pos[ind.x], pos[ind.y], pos[ind.z], pos[ind.w]);}
      }else
      {
         REPA(base.tri ){C VecI  &ind=base.tri .ind(i); VI.line(pos[ind.x], pos[ind.y]); VI.line(pos[ind.y], pos[ind.z]); VI.line(pos[ind.z], pos[ind.x]);}
         REPA(base.quad){C VecI4 &ind=base.quad.ind(i); VI.line(pos[ind.x], pos[ind.y]); VI.line(pos[ind.y], pos[ind.z]); VI.line(pos[ind.z], pos[ind.w]); VI.line(pos[ind.w], pos[ind.x]);}
      }
      VI.end();
   }
}
/******************************************************************************/
}
/******************************************************************************/
