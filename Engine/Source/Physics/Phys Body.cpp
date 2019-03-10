/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "PhysX Stream.h"
/******************************************************************************

   'PhysPart.base' muse not have quads (only triangles),
      because Bullet doesn't copy memory and uses only provided data.

   '_bullet_cooked_data' must be aligned to 16 bytes.

   PhysMesh may not be deleted while there are still actors using that mesh,
      that's why it uses 'used_by' reference count, and it will get deleted
      only when there are no references left.

/******************************************************************************/
#define CC4_PHSB CC4('P','H','S','B') // PhysBase (unused anymore)
#define CC4_PHSP CC4('P','H','S','P') // PhysPart
#define CC4_PHYS CC4('P','H','Y','S') // PhysBody
#define CC4_PHSG CC4('P','H','S','G') // PhysGroup
/******************************************************************************/
DEFINE_CACHE(PhysBody, PhysBodies, PhysBodyPtr, "PhysBody");
static Memx<PhysMesh>  PhysMeshes;
static SyncLock        PhysMeshesLock;
/******************************************************************************/
void IncRef(PhysMesh* &pm) {if(pm)AtomicInc(pm->_used_by);}
void DecRef(PhysMesh* &pm)
{
   if(pm)
   {
      if(PhysMeshes.elms()) // if 'PhysMeshes' destructor was not yet called, and there are elements ('pm' is not pointing to already released memory)
      {
         AtomicDec(pm->_used_by);
         if(pm->_used_by< 0)Exit("PhysMesh has <0 references");
         if(pm->_used_by==0) // no longer used by any 'PhysBody' or 'Actor'
         {
            pm->del(); // delete it first, so it will not use 'Physics._lock' in destructor (deadlock could occur)
            {
               SafeSyncLocker locker(PhysMeshesLock);
               PhysMeshes.removeData(pm);
            }
         }
      }
      pm=null;
   }
}
void IncRef(btCollisionShape *shape)
{
   if(shape)
   {
      SafeWriteLock lock(Physics._rws);
      IntPtr refs=(IntPtr)shape->getUserPointer();
      shape->setUserPointer(Ptr(refs+1));
   }
}
void DecRef(btCollisionShape *shape)
{
   if(shape)
   {
      SafeWriteLock lock(Physics._rws);
      IntPtr refs=(IntPtr)shape->getUserPointer();
      if(    refs< 0)Exit("btCollisionShape has <0 references");
      if(    refs==0)Delete(shape);else shape->setUserPointer(Ptr(refs-1));
   }
}
static void Set(PhysMesh &pm, PhysMesh* &pm_ptr)
{
   pm._used_by=1;
   PhysMeshesLock.on (); PhysMesh &stored_pm=PhysMeshes.New();
   PhysMeshesLock.off();
   Swap(stored_pm, pm);
   pm_ptr=&stored_pm;
}
/******************************************************************************/
// PHYSICAL MESH
/******************************************************************************/
void PhysMesh::zero()
{
  _type=PHYS_NONE;
  _box.zero();
  _base=null;
  _used_by=0;
  _bullet_cooked_data     =null;
  _bullet_cooked_data_size=0;
  _convex=null;
  _mesh  =null;
  _tvia  =null;
}
void PhysMesh::del()
{
   if(_convex || _mesh || _tvia) // delete API specific data first (because they may be dependent on data listed below)
   {
      SafeWriteLock lock(Physics._rws);
   #if PHYSX
      if(_convex && Physx.physics)_convex->release();
      if(_mesh   && Physx.physics)_mesh  ->release();
   #else
      DecRef(_convex); _convex=null;
      DecRef(_mesh  ); _mesh  =null;
      Delete(_tvia  );
   #endif
   }
   Delete(_base);
                _physx_cooked_data.del();
   AlignedFree(_bullet_cooked_data);
   zero();
}
/******************************************************************************/
void PhysMesh::freeHelperData()
{
#if PHYSX
   AlignedFree(_bullet_cooked_data); _bullet_cooked_data_size=0;
   if(_convex || _mesh         ){Delete(_base); _physx_cooked_data.del();}else // delete helpers only if we've created data from them
   if(_physx_cooked_data.elms()){Delete(_base);}
#else
  _physx_cooked_data.del();
   // don't delete '_base' and '_bullet_cooked_data' since Bullet Phys Meshes uses them (bullet mesh vtx/tri is directly mapped to '_base', while '_bullet_cooked_data' contains some extra precomputed data)
#endif
}
/******************************************************************************/
Bool PhysMesh::adjustStorage(Bool universal, Bool physx, Bool bullet, Bool *changed)
{
   if(changed)*changed=false;
   if(_type==PHYS_CONVEX || _type==PHYS_MESH)
   {
      if(                         bullet)universal=true; // 'universal' is always needed when using 'bullet'
      if(!universal && !physx && !bullet)universal=true; // if no storage is specified then keep 'universal' only

      // create universal
      if(universal && !_base)
      {
      #if PHYSX
         setPhysMesh(); // create PhysX data if not yet available
         if(_convex){if(New(_base)->create(*_convex)){_base->quadToTri(); if(changed)*changed=true;}else Delete(_base);}else
         if(_mesh  ){if(New(_base)->create(*_mesh  )){_base->quadToTri(); if(changed)*changed=true;}else Delete(_base);}
      #endif
      }

      // create physx
      if(physx && !_physx_cooked_data.elms() && _base)switch(_type)
      {
         case PHYS_CONVEX: if(cookConvex(_base, true))if(changed)*changed=true; break;
         case PHYS_MESH  : if(cookMesh  (_base      ))if(changed)*changed=true; break;
      }

      // create bullet
      if(bullet && _type==PHYS_MESH && !_bullet_cooked_data)
      {
         btOptimizedBvh             *temp_bvh =null; // does not need to be released (as it's part of the mesh)
         btBvhTriangleMeshShape     *temp_mesh=null;
         btTriangleIndexVertexArray *temp_tvia=null;
         if(!temp_bvh)
         {
         #if !PHYSX
            if(_mesh)temp_bvh=_mesh->getOptimizedBvh(); // check if bullet mesh has already pre-computed data
         #endif
            if(!temp_bvh && _base)
            {
               // create temp mesh when not available
               temp_tvia=new btTriangleIndexVertexArray(_base->tris(), (int*)_base->tri.ind(), SIZE(VecI), _base->vtxs(), (Flt*)_base->vtx.pos(), SIZE(Vec));
               temp_mesh=new btBvhTriangleMeshShape    (temp_tvia, true, true);
               temp_bvh =temp_mesh->getOptimizedBvh();
            }
         }
         if(temp_bvh)
         {
            if(changed)*changed=true;
            if(_bullet_cooked_data_size=temp_bvh->calculateSerializeBufferSize())
            {
              _bullet_cooked_data=AlignedAlloc(_bullet_cooked_data_size);
               if( !temp_bvh->serializeInPlace(_bullet_cooked_data, _bullet_cooked_data_size, false))
               {
                  AlignedFree(_bullet_cooked_data);
                              _bullet_cooked_data_size=0;
               }
            }
         }
         Delete(temp_mesh);
         Delete(temp_tvia);
      }

      // then delete all unwanted
      if(!universal && _physx_cooked_data.elms() &&  _base                    ){Delete(_base);                                                if(changed)*changed=true;} // delete only if there's physx     left (don't delete if there's no physx    )
      if(!physx     && _base                     &&  _physx_cooked_data.elms()){             _physx_cooked_data.del();                        if(changed)*changed=true;} // delete only if there's universal left (don't delete if there's no universal)
      if(!bullet                                 && _bullet_cooked_data       ){AlignedFree(_bullet_cooked_data); _bullet_cooked_data_size=0; if(changed)*changed=true;}

      // check for failure
                          if((!universal)!=(! _base                    ))return false;
                          if((!physx    )!=(! _physx_cooked_data.elms()))return false;
      if(_type==PHYS_MESH)if((!bullet   )!=(!_bullet_cooked_data       ))return false; // only PHYS_MESH uses 'bullet' data
   }
   return true;
}
/******************************************************************************/
Bool PhysMesh::cookConvex(MeshBase *src, Bool mesh_is_already_convex) // cook data only if the body doesn't have yet any cooked data
{
#if PHYSX
   if(!_physx_cooked_data.elms() && src && src->vtxs())
   {
      PxConvexMeshDesc desc;
      Memt<PxHullPolygon, 32768> polys; // must be outside braces
      Memt<Byte         , 32768> ind  ; // must be outside braces
      desc.points.data  =src->vtx.pos();
      desc.points.count =src->vtxs();
      desc.points.stride=SIZE(Vec);
      if(mesh_is_already_convex && src->faces())
      {
         Bool bit16=(src->vtxs()<=0x10000);
         desc.indices.data  =src->tri.ind();
         desc.indices.count =src->tris()*3 + src->quads()*4;
         desc.indices.stride=(bit16 ? SIZE(U16) : SIZE(U32));
         if(bit16 || src->quads())
         {
            desc.indices.data=ind.setNum(desc.indices.count*desc.indices.stride).data();
            if(bit16)
            {
               Copy32To16((Byte*)desc.indices.data                          , src->tri .ind(), src->tris ()*3);
               Copy32To16((Byte*)desc.indices.data+src->tris()*(3*SIZE(U16)), src->quad.ind(), src->quads()*4);
            }else
            {
               Copy32To32((Byte*)desc.indices.data                          , src->tri .ind(), src->tris ()*3);
               Copy32To32((Byte*)desc.indices.data+src->tris()*(3*SIZE(U32)), src->quad.ind(), src->quads()*4);
            }
         }
         polys.setNum(src->faces());
       C Vec *pos=src->vtx.pos();
         Int  offset=0;
         FREPA(polys)
         {
            PxHullPolygon &poly=polys[i];
            VecI p; // we can access quads as VecI too
            if(bit16)p=*(VecUS*)((U16*)desc.indices.data+offset);
            else     p=*(VecI *)((U32*)desc.indices.data+offset);
          C Vec &p0=pos[p.x], &p1=pos[p.y], &p2=pos[p.z], n=GetNormal(p0, p1, p2);
            poly.mPlane[0]=n.x;
            poly.mPlane[1]=n.y;
            poly.mPlane[2]=n.z;
            poly.mPlane[3]=-Dot(p0, n);
            poly.mNbVerts  =(InRange(i, src->tri) ? 3 : 4);
            poly.mIndexBase=offset; offset+=poly.mNbVerts;
         }
         desc.polygons.data  =polys.data();
         desc.polygons.count =polys.elms();
         desc.polygons.stride=polys.elmSize();
         if(bit16)desc.flags|=PxConvexFlag::e16_BIT_INDICES;
      }else
      {
         desc.flags=PxConvexFlag::eCOMPUTE_CONVEX; // recompute the hull
      }

      OutputStream buf;
      if(!Physx.cook[0])Exit("PhysPart.createConvex\nPhysics hasn't been created");
          Physx.cook[0]->cookConvexMesh(desc, buf);
      if(!buf.data && !(desc.flags&PxConvexFlag::eCOMPUTE_CONVEX)) // if failed, then try to recompute the convex hull
      {
         FlagDisable(desc.flags, PxConvexFlag::e16_BIT_INDICES);
         FlagEnable (desc.flags, PxConvexFlag::eCOMPUTE_CONVEX);
         desc.polygons.data  =null;
         desc.polygons.count =0;
         desc.polygons.stride=0;
         desc.indices .data  =null;
         desc.indices .count =0;
         desc.indices .stride=0;
         Physx.cook[0]->cookConvexMesh(desc, buf);
      }
     _physx_cooked_data.setFrom(buf.data, buf.pos);
   }
#endif
   return _physx_cooked_data.elms()!=0;
}
/******************************************************************************/
Bool PhysMesh::cookMesh(MeshBase *src, Bool keep_face_indexes) // cook data only if the body doesn't have yet any cooked data
{
#if PHYSX
   if(!_physx_cooked_data.elms() && src && src->vtxs() && src->faces())
   {
      PxTriangleMeshDesc desc;
      Memt<Byte> ind; // must be outside braces
      Bool bit16=(src->vtxs()<=0x10000);
      desc.triangles.stride=(bit16 ? SIZE(U16)*3 : SIZE(U32)*3);
      desc.triangles.count =src->trisTotal();
      desc.triangles.data  =src->tri.ind();
      if(bit16 || src->quads())
      {
         desc.triangles.data=ind.setNum(desc.triangles.count*desc.triangles.stride).data();
         SetFaceIndex((Ptr)desc.triangles.data, src->tri.ind(), src->tris(), src->quad.ind(), src->quads(), bit16);
      }
      desc.points.data  =src->vtx.pos();
      desc.points.count =src->vtxs();
      desc.points.stride=SIZE(Vec);
      if(bit16)desc.flags|=PxMeshFlag::e16_BIT_INDICES;

      OutputStream buf;
      if(!Physx.cook[keep_face_indexes])Exit("PhysPart.createMesh\nPhysics hasn't been created");
          Physx.cook[keep_face_indexes]->cookTriangleMesh(desc, buf);
     _physx_cooked_data.setFrom(buf.data, buf.pos);
   }
#endif
   return _physx_cooked_data.elms()!=0;
}
/******************************************************************************
Bool PhysMesh::cookHeightfield(MeshBase *src, Flt extent)
{
#if PHYSX
#endif
   return _physx_cooked_data.elms()!=0;
}
/******************************************************************************/
void PhysMesh::setPhysMesh()
{
#if PHYSX
   switch(_type)
   {
      case PHYS_CONVEX: if(!_convex){WriteLock lock(Physics._rws); if(!_convex && Physx.physics && cookConvex(_base, true)){InputStream is(_physx_cooked_data.data(), _physx_cooked_data.elms()); _convex=Physx.physics->createConvexMesh  (is);}} break;
      case PHYS_MESH  : if(!_mesh  ){WriteLock lock(Physics._rws); if(!_mesh   && Physx.physics && cookMesh  (_base      )){InputStream is(_physx_cooked_data.data(), _physx_cooked_data.elms()); _mesh  =Physx.physics->createTriangleMesh(is);}} break;
   }
#else
   if(_base)switch(_type)
   {
      case PHYS_CONVEX: if(!_convex){WriteLock lock(Physics._rws); if(!_convex)if(_convex=new btConvexHullShape((Flt*)_base->vtx.pos(), _base->vtxs(), SIZE(Vec)))_convex->setMargin(Physics.skin());} break;
      case PHYS_MESH  : if(!_mesh  ){WriteLock lock(Physics._rws); if(!_mesh  )
         {
           _tvia=new btTriangleIndexVertexArray(_base->tris(), (Int*)_base->tri.ind(), SIZE(VecI), _base->vtxs(), (Flt*)_base->vtx.pos(), SIZE(Vec));
           _mesh=new btBvhTriangleMeshShape    (_tvia, true, false);
            if( _bullet_cooked_data     )_mesh->  setOptimizedBvh(btOptimizedBvh::deSerializeInPlace(_bullet_cooked_data, _bullet_cooked_data_size, false));
            if(!_mesh->getOptimizedBvh())_mesh->buildOptimizedBvh();
                                         _mesh->setMargin(Physics.skin());
         }}
      break;
   }
#endif
   if(App.flag&APP_AUTO_FREE_PHYS_BODY_HELPER_DATA)freeHelperData(); // after creating phys mesh we can try releasing helper data
}
/******************************************************************************/
Bool PhysMesh::createConvexTry(MeshBase &mshb, Bool mesh_is_already_convex)
{
   del();

   if(mshb.vtxs())
   {
   #if PHYSX
      if(!cookConvex(&mshb, mesh_is_already_convex))return false;
   #else
      New(_base);
      if(mesh_is_already_convex)_base->create      (mshb          ,           0).quadToTri();
      else                      _base->createConvex(mshb.vtx.pos(), mshb.vtxs()).quadToTri();
   #endif

      T._type=PHYS_CONVEX;
      mshb.getBox(T._box);
      setPhysMesh();
   }
   return true;
}
/******************************************************************************/
Bool PhysMesh::createMeshTry(MeshBase &mshb, Bool keep_face_indexes)
{
   del();

   if(mshb.vtxs() && mshb.faces())
   {
   #if PHYSX
      if(!cookMesh(&mshb, keep_face_indexes))return false;
   #else
      New(_base)->create(mshb, 0).quadToTri();
   #endif

      T._type=PHYS_MESH;
      mshb.getBox(T._box);
      setPhysMesh();
   }
   return true;
}
/******************************************************************************/
#if !PHYSX
Flt PhysMesh::volume()C
{
   if(_base)return _base->convexVolume();
   if(_convex) // approximate with a Ball
   {
      btVector3 center; btScalar radius; _convex->getBoundingSphere(center, radius);
      return Ball(radius).volume();
   }
   return 0;
}
#endif
Bool PhysMesh::setBox()
{
   switch(_type)
   {
      case PHYS_CONVEX:
      {
         if(_base)return _base->getBox(_box);
      #if PHYSX
         if(_convex){_box=Physx.box(_convex->getLocalBounds()); return true;}
      #else
         if(_convex){btVector3 min, max; _convex->getAabb(btTransform::getIdentity(), min, max); _box.set(Bullet.vec(min), Bullet.vec(max)); return true;}
      #endif
      }break;

      case PHYS_MESH:
      {
         if(_base)return _base->getBox(_box);
      #if PHYSX
         if(_mesh){_box=Physx.box(_mesh->getLocalBounds()); return true;}
      #else
         if(_mesh){btVector3 min, max; _mesh->getAabb(btTransform::getIdentity(), min, max); _box.set(Bullet.vec(min), Bullet.vec(max)); return true;}
      #endif
      }break;
   }
  _box.zero(); return false;
}
/******************************************************************************/
void PhysMesh::draw(C Color &color)C
{
   switch(_type)
   {
      case PHYS_CONVEX: if(_base)DrawPhys(*_base, color);else if(_convex)DrawConvex(*_convex, color); break;
      case PHYS_MESH  : if(_base)DrawPhys(*_base, color);else if(_mesh  )DrawMesh  (*_mesh  , color); break;
   }
}
PhysMesh& PhysMesh::operator=(C PhysMesh &src)
{
   if(this!=&src)
   {
      del();

     _type=src._type;
     _box =src._box;
      if(src._base)New(_base)->create(*src._base);
     _physx_cooked_data=src._physx_cooked_data;

      if(src._bullet_cooked_data)
      {
        _bullet_cooked_data_size=         src._bullet_cooked_data_size;
        _bullet_cooked_data     =AlignedAlloc(_bullet_cooked_data_size);
         CopyFast(_bullet_cooked_data, src._bullet_cooked_data, _bullet_cooked_data_size);
      }

   #if PHYSX
      if(!_base && !_physx_cooked_data.elms()) // if we don't have any phys, but src has phys mesh (this can happen if 'freeHelperData' was called)
      {
         if(src._convex){New(_base)->create(*src._convex); setPhysMesh(); Delete(_base); if(_convex)_physx_cooked_data.del();} // try creating from phys mesh
         if(src._mesh  ){New(_base)->create(*src._mesh  ); setPhysMesh(); Delete(_base); if(_mesh  )_physx_cooked_data.del();} // try creating from phys mesh
      }
   #endif

      setPhysMesh();
   }
   return T;
}
/******************************************************************************/
// PHYSICAL BODY PART
/******************************************************************************/
void PhysPart::zero()
{
   density=0;
  _type   =PHYS_NONE;
  _pm     =null;
}

PhysPart::PhysPart(                               ) {zero();                          }
PhysPart::PhysPart(C Box     &box    , Flt density) {zero(); create(box    , density);}
PhysPart::PhysPart(C OBox    &obox   , Flt density) {zero(); create(obox   , density);}
PhysPart::PhysPart(C Extent  &ext    , Flt density) {zero(); create(ext    , density);}
PhysPart::PhysPart(C Ball    &ball   , Flt density) {zero(); create(ball   , density);}
PhysPart::PhysPart(C Capsule &capsule, Flt density) {zero(); create(capsule, density);}
PhysPart::PhysPart(C Tube    &tube   , Flt density) {zero(); create(tube   , density);}
PhysPart::PhysPart(C Shape   &shape  , Flt density) {zero(); create(shape  , density);}

PhysPart& PhysPart::del()
{
   DecRef(_pm);
   shape.type=SHAPE_NONE;
   zero(); return T;
}
PhysPart& PhysPart::create(C Shape &shape, Flt density)
{
   del();

   T._type   =PHYS_SHAPE;
   T. shape  =shape;
   T. density=density;

   return T;
}
/******************************************************************************/
Bool PhysPart::createConvexTry(MeshBase &mshb, Flt density, Bool mesh_is_already_convex)
{
   del();

   if(mshb.vtxs())
   {
      PhysMesh pm; if(!pm.createConvexTry(mshb, mesh_is_already_convex))return false;
      Set(pm, T._pm);
      T._type   =PHYS_CONVEX;
      T. density=density;
   }
   return true;
}
Bool PhysPart::createMeshTry(MeshBase &mshb, Bool keep_face_indexes)
{
   del();

   if(mshb.vtxs() && mshb.faces())
   {
      PhysMesh pm; if(!pm.createMeshTry(mshb, keep_face_indexes))return false;
      Set(pm, T._pm);
      T._type   =PHYS_MESH;
      T. density=1;
   }
   return true;
}
/******************************************************************************/
Bool PhysPart::createConvexTry(MeshLod &mshl, Flt density, Bool mesh_is_already_convex, Bool skip_hidden_parts)
{
   MeshBase mshb; mshb.createPhys(mshl, VTX_POS|FACE_IND, false, skip_hidden_parts);
   return createConvexTry(mshb, density, mesh_is_already_convex);
}
Bool PhysPart::createMeshTry(MeshLod &mshl, Bool keep_face_indexes, Bool skip_hidden_parts)
{
   MeshBase mshb; mshb.createPhys(mshl, VTX_POS|FACE_IND, false, skip_hidden_parts);
   return createMeshTry(mshb, keep_face_indexes);
}
/******************************************************************************/
PhysPart& PhysPart::createConvex(MeshBase &mshb, Flt density, Bool mesh_is_already_convex                        ) {if(!createConvexTry(mshb, density, mesh_is_already_convex                   ))Exit("PhysPart::createConvex"); return T;}
PhysPart& PhysPart::createConvex(MeshLod  &mshl, Flt density, Bool mesh_is_already_convex, Bool skip_hidden_parts) {if(!createConvexTry(mshl, density, mesh_is_already_convex, skip_hidden_parts))Exit("PhysPart::createConvex"); return T;}

PhysPart& PhysPart::createMesh(MeshBase &mshb, Bool keep_face_indexes                        ) {if(!createMeshTry(mshb, keep_face_indexes                   ))Exit("PhysPart.createMesh"); return T;}
PhysPart& PhysPart::createMesh(MeshLod  &mshl, Bool keep_face_indexes, Bool skip_hidden_parts) {if(!createMeshTry(mshl, keep_face_indexes, skip_hidden_parts))Exit("PhysPart.createMesh"); return T;}
/******************************************************************************/
PhysPart& PhysPart::operator=(C PhysPart &src)
{
   if(this!=&src)
   {
      del();

      density=src. density;
      shape  =src. shape;
     _type   =src._type;
      if(src._pm)
      {
         PhysMesh pm; pm=*src._pm;
         Set(pm, T._pm);
      }
   }
   return T;
}
/******************************************************************************/
Flt  PhysPart::area  (        )C {return (_type==PHYS_SHAPE) ? shape.area  () : 0;}
Flt  PhysPart::volume(        )C {return (_type==PHYS_SHAPE) ? shape.volume() : 0;}
Bool PhysPart::getBox(Box &box)C
{
   switch(_type)
   {
      case PHYS_SHAPE: box=shape; return true;

      case PHYS_CONVEX:
      case PHYS_MESH  : if(_pm){box=_pm->_box; return true;} break;
   }
   box.zero(); return false;
}
/******************************************************************************/
PhysPart& PhysPart::transform(C Matrix3 &matrix)
{
   switch(_type)
   {
      case PHYS_SHAPE : shape*=matrix; break;
      case PHYS_CONVEX: {MeshBase temp; temp.create(T); temp.transform(matrix); createConvex(temp, density, true);} break;
      case PHYS_MESH  : {MeshBase temp; temp.create(T); temp.transform(matrix); createMesh  (temp               );} break;
   }
   return T;
}
PhysPart& PhysPart::transform(C Matrix &matrix)
{
   switch(_type)
   {
      case PHYS_SHAPE : shape*=matrix; break;
      case PHYS_CONVEX: {MeshBase temp; temp.create(T); temp.transform(matrix); createConvex(temp, density, true);} break;
      case PHYS_MESH  : {MeshBase temp; temp.create(T); temp.transform(matrix); createMesh  (temp               );} break;
   }
   return T;
}
/******************************************************************************/
PhysPart& PhysPart::mirrorX()
{
   switch(_type)
   {
      case PHYS_SHAPE : shape.mirrorX(); break;
      case PHYS_CONVEX: {MeshBase temp; temp.create(T); temp.mirrorX(); createConvexTry(temp, density, true);} break;
      case PHYS_MESH  : {MeshBase temp; temp.create(T); temp.mirrorX(); createMeshTry  (temp               );} break;
   }
   return T;
}
PhysPart& PhysPart::mirrorY()
{
   switch(_type)
   {
      case PHYS_SHAPE : shape.mirrorY(); break;
      case PHYS_CONVEX: {MeshBase temp; temp.create(T); temp.mirrorY(); createConvexTry(temp, density, true);} break;
      case PHYS_MESH  : {MeshBase temp; temp.create(T); temp.mirrorY(); createMeshTry  (temp               );} break;
   }
   return T;
}
PhysPart& PhysPart::mirrorZ()
{
   switch(_type)
   {
      case PHYS_SHAPE : shape.mirrorZ(); break;
      case PHYS_CONVEX: {MeshBase temp; temp.create(T); temp.mirrorZ(); createConvexTry(temp, density, true);} break;
      case PHYS_MESH  : {MeshBase temp; temp.create(T); temp.mirrorZ(); createMeshTry  (temp               );} break;
   }
   return T;
}
/******************************************************************************/
void PhysPart::setPhysMesh() {if(_pm)_pm->setPhysMesh();}
/******************************************************************************/
Bool PhysPart::adjustStorage(Bool universal, Bool physx, Bool bullet, Bool *changed)
{
   if(_pm)
   {
      PhysPart temp; temp=T; Bool ok=temp._pm->adjustStorage(universal, physx, bullet, changed); Swap(temp, T); return ok; // adjust storage on separate 'pm' so any current actors using the mesh, will not get invalid
   }
   if(changed)*changed=false;
   return true;
}
/******************************************************************************/
PhysPart& PhysPart::freeHelperData() {if(_pm)_pm->freeHelperData(); return T;}
/******************************************************************************/
void PhysPart::draw(C Color &color)C
{
   switch(_type)
   {
      case PHYS_SHAPE: shape.draw(color); break;

      case PHYS_CONVEX:
      case PHYS_MESH  : if(_pm)_pm->draw(color); break;
   }
}
/******************************************************************************/
static Bool VerifyPhysX(File &f) // this function checks if provided PhysX cooked data is of the same version as currently used PhysX SDK
{
   if(f.getUInt()==CC4('N', 'X', 'S', 1))switch(f.getUInt()) // type
   {
      case CC4('M','E','S','H'): // tri mesh
      {
         return f.getInt()==PHYS_API(PX_MESH_VERSION, -1); // this must be equal to the current version that PhysX SDK supports
      }break;

      case CC4('C','V','X','M'): // convex mesh
      {
         return f.getInt()==PHYS_API(PX_CONVEX_VERSION, -1); // this must be equal to the current version that PhysX SDK supports
      }break;
   }
   return false;
}
static Bool ImportPhysX(MeshBase &mesh, File &f) // import MeshBase from PhysX cooked data
{
   if(f.getUInt()==CC4('N','X','S',1))switch(f.getUInt()) // type
   {
      case CC4('M','E','S','H'): // triangle mesh
      {
         switch(f.getInt()) // version
         {
            case 1: // PhysX 2.8.4
            {
               UInt flags=f.getUInt(); f.skip(4*3);
               Int  vtxs =f.getInt ();
               Int  tris =f.getInt ();
               if(vtxs>0 && tris>0)
               {
                  mesh.create(vtxs, 0, tris, 0);
                  f.getN(mesh.vtx.pos(), mesh.vtxs());
                  if(flags& 8)FREPA(mesh.tri){VecB  t; f>>t; mesh.tri.ind(i)=t;}else //  8-bit indexes
                  if(flags&16)FREPA(mesh.tri){VecUS t; f>>t; mesh.tri.ind(i)=t;}else // 16-bit indexes
                             f.getN(mesh.tri.ind(), mesh.tris());                    // 32-bit indexes
                  return true;
               }
            }break;

            case  9: // PhysX 3.2.2
            case 12: // PhysX 3.3.0
            {
               UInt flags;
               Flt  unused;
               Int  vtxs, tris;
               f.getMulti(flags, unused, vtxs, tris);
               if(vtxs>0 && tris>0)
               {
                  mesh.create(vtxs, 0, tris, 0);
                  f.getN(mesh.vtx.pos(), mesh.vtxs());
                  if(flags&4)FREPA(mesh.tri){VecB  t; f>>t; mesh.tri.ind(i)=t;}else //  8-bit indexes
                  if(flags&8)FREPA(mesh.tri){VecUS t; f>>t; mesh.tri.ind(i)=t;}else // 16-bit indexes
                            f.getN(mesh.tri.ind(), mesh.tris());                    // 32-bit indexes
                  return true;
               }
            }break;

            case 15: // PhysX 3.4.1
            {
               UInt midphaseID, flags;
               Int  vtxs, tris;
               f.getMulti(midphaseID, flags, vtxs, tris);
               if(vtxs>0 && tris>0)
               {
                  mesh.create(vtxs, 0, tris, 0);
                  f.getN(mesh.vtx.pos(), mesh.vtxs());
                  if(flags&4)FREPA(mesh.tri){VecB  t; f>>t; mesh.tri.ind(i)=t;}else //  8-bit indexes
                  if(flags&8)FREPA(mesh.tri){VecUS t; f>>t; mesh.tri.ind(i)=t;}else // 16-bit indexes
                            f.getN(mesh.tri.ind(), mesh.tris());                    // 32-bit indexes
                  return true;
               }
            }break;

         #if PHYSX
            ASSERT(PX_MESH_VERSION==15); // if you get a compile error here, it means that support for the new format must be added above
         #endif
         }
      }break;

      case CC4('C','V','X','M'): // convex mesh
      {
         switch(f.getInt()) // version
         {
            case 3: // PhysX 2.8.4
            {
               f.getUInt();
               if(f.getUInt()==CC4('I','C','E', 1 ))
               if(f.getUInt()==CC4('C','L','H','L'))
               if(f.getInt ()==0)
               if(f.getUInt()==CC4('I','C','E', 1 ))
               if(f.getUInt()==CC4('C','V','H','L'))
               if(f.getInt ()==5)
               {
                  Int vtxs=f.getInt();
                  Int tris=f.getInt();
                  if(vtxs>0 && vtxs<=256 && tris>0)
                  {
                     Int a=f.getInt();
                     Int b=f.getInt();
                     Int c=f.getInt();
                     Int d=f.getInt();
                   //if(c==2*a && c==d)
                     {
                        mesh.create(vtxs, 0, tris, 0);
                        f.getN(mesh.vtx.pos(), mesh.vtxs());
                        Int e=f.getInt();
                        FREPA(mesh.tri){VecI &t=mesh.tri.ind(i); t.x=f.getByte(); t.y=f.getByte(); t.z=f.getByte();} //  8-bit indexes
                        return true;
                     }
                  }
               }
            }break;

            case 13: // PhysX 3.2.2, 3.3.0, 3.4.1
            {
                  f.getUInt();
               if(f.getUInt()==CC4('I','C','E', 1 ))
               if(f.getUInt()==CC4('C','L','H','L'))
               {
                  Memt<VecI2> poly_data; // x=offset, y=vtxs
                  Memt<Byte > ind;
                  Int v=f.getInt();
                  if( v==6)
                  {
                     if(f.getUInt()==CC4('I','C','E', 1 ))
                     if(f.getUInt()==CC4('C','V','H','L'))
                     if(f.getInt ()==6)
                     {
                        Int vtxs =f.getInt();
                        Int a    =f.getInt(); // this is NOT tris! (because it's different than what's calculated below)
                        Int polys=f.getInt();
                        Int inds =f.getInt();
                        if(vtxs>0 && vtxs<=256 && polys>0 && inds>0)
                        {
                           mesh.create(vtxs, 0, 0, 0);
                           f.getN(mesh.vtx.pos(), mesh.vtxs());
                           UShort b=f.getUShort();
                           Int tris=0;
                           FREP(polys)
                           {
                              Vec4   v; f>>v;
                              UShort offset=f.getUShort();
                              Byte   vtxs  =f.getByte  ();
                              Byte   a     =f.getByte  ();
                              poly_data.New().set(offset, vtxs);
                              tris+=Max(0, vtxs-2);
                           }
                           mesh.tri._elms=tris; mesh.include(TRI_IND);
                           ind.setNum(inds); f.getN(ind.data(), ind.elms());
                           tris=0;
                           FREPA(poly_data)
                           {
                            C VecI2 &p=poly_data[i];
                              Int offset=p.x, vtxs=p.y;
                              FREP(vtxs-2)
                                 if(InRange(tris, mesh.tri))
                                    mesh.tri.ind(tris++).set(ind[offset+0], ind[offset+i+1], ind[offset+i+2]);
                           }
                           return true;
                        }
                     }
                  }else
                  if(v==7 || v==8) // 8=PhysX 3.4.1
                  {
                     if(f.getUInt()==CC4('I','C','E', 1 ))
                     if(f.getUInt()==CC4('C','V','H','L'))
                     if(f.getInt ()==v)
                     {
                        Int vtxs =f.getInt();
                        Int a    =f.getInt(); // this is NOT tris! (because it's different than what's calculated below)
                        Int polys=f.getInt();
                        Int inds =f.getInt();
                        if(vtxs>0 && vtxs<=256 && polys>0 && inds>0)
                        {
                           mesh.create(vtxs, 0, 0, 0);
                           f.getN(mesh.vtx.pos(), mesh.vtxs());
                           Int tris=0;
                           FREP(polys)
                           {
                              Vec4   v;
                              UShort offset;
                              Byte   vtxs, a;
                              f.getMulti(v, offset, vtxs, a);
                              poly_data.New().set(offset, vtxs);
                              tris+=Max(0, vtxs-2);
                           }
                           mesh.tri._elms=tris; mesh.include(TRI_IND);
                           ind.setNum(inds); f.getN(ind.data(), ind.elms());
                           tris=0;
                           FREPA(poly_data)
                           {
                            C VecI2 &p=poly_data[i];
                              Int offset=p.x, vtxs=p.y;
                              FREP(vtxs-2)
                                 if(InRange(tris, mesh.tri))
                                    mesh.tri.ind(tris++).set(ind[offset+0], ind[offset+i+1], ind[offset+i+2]);
                           }
                           return true;
                        }
                     }
                  }
               }
            }break;

         #if PHYSX
            ASSERT(PX_CONVEX_VERSION==13); // if you get a compile error here, it means that support for the new format must be added above
         #endif
         }
      }break;
   }
   return false;
}
static void Loaded(PhysMesh &pm)
{
   if(pm._physx_cooked_data.elms())
   {
   #if PHYSX
      File f; f.readMem(pm._physx_cooked_data.data(), pm._physx_cooked_data.elms());
      if(!VerifyPhysX(f)) // if cooked version doesn't match with current PhysX SDK version
      {
         if(pm._base)pm._physx_cooked_data.del();else // if we have universal then just delete the incompatible physx cooked, and universal will be used
         {
            MeshBase temp;
            f.pos(0); if(ImportPhysX(temp, f)) // if import success
            {
               Swap(*New(pm._base), temp); // swap universal with temp
               pm._physx_cooked_data.del(); // delete cooked so universal will be used
               pm.setPhysMesh(); if(pm._mesh || pm._convex)Delete(pm._base); // if cooked and created PhysMesh successfully, then delete universal because we didn't want it originally
            }
         }
      }
   #else // bullet
      if(!pm._base) // try to import physx only if we don't have universal mesh
      {
         MeshBase temp;
         if(ImportPhysX(temp, File().readMem(pm._physx_cooked_data.data(), pm._physx_cooked_data.elms()))) // if import success
            Swap(*New(pm._base), temp); // swap universal with temp (keep physx cooked because it will not be used anyway, and we want to keep it in case of saving file)
      }
   #endif
   }
   pm.setPhysMesh();
}
Bool PhysPart::saveData(File &f)C
{
   f.putMulti(Byte(4), _type, density); // version
   switch(_type)
   {
      case PHYS_NONE: return f.ok();

      case PHYS_SHAPE: return shape.save(f) && f.ok();

      case PHYS_CONVEX:
      case PHYS_MESH  :
      {
         f<<_pm->_box;
         if(!_pm->_base)f.putBool(false);else{f.putBool(true); if(!_pm->_base->saveData(f))return false;}
         if(_pm->_physx_cooked_data._saveRaw(f))
         {
            if(!_pm->_bullet_cooked_data_size)f.cmpUIntV(0);else // bullet cooked data version, used in case future bullet cooked data will not be compatible, 0=no data, 1=current version
            {
               f.cmpUIntV(1); // version
                  f<<_pm->_bullet_cooked_data_size;
               f.put(_pm->_bullet_cooked_data, _pm->_bullet_cooked_data_size);
            }
            return f.ok();
         }
      }break;
   }
   return false;
}
Bool PhysPart::loadData(File &f)
{
   del(); switch(f.decUIntV()) // version
   {
      case 4:
      {
         f.getMulti(_type, density);
         switch(_type)
         {
            case PHYS_NONE: if(f.ok())return true; break;

            case PHYS_SHAPE: if(shape.load(f) && f.ok())return true; break;

            case PHYS_CONVEX:
            case PHYS_MESH  :
            {
               PhysMesh pm; pm._type=_type;
               f>>pm._box;
               if(f.getBool())if(!New(pm._base)->loadData(f))goto error;
               if(pm._physx_cooked_data._loadRaw(f))
               {
                  if(UInt bullet_ver=f.decUIntV())
                  {
                     if(bullet_ver!=1)f.skip(f.getUInt());else // other versions aren't supported
                     {
                        f>>pm._bullet_cooked_data_size;
                              pm._bullet_cooked_data=AlignedAlloc(pm._bullet_cooked_data_size);
                        f.get(pm._bullet_cooked_data,             pm._bullet_cooked_data_size);
                     }
                  }
                  if(f.ok())
                  {
                     Loaded(pm);
                     Set(pm, T._pm);
                     return true;
                  }
               }
            }break;
         }
      }break;

      case 3:
      {
         f.getMulti(_type, density);
         switch(_type)
         {
            case PHYS_NONE: if(f.ok())return true; break;

            case PHYS_SHAPE: if(shape.load(f) && f.ok())return true; break;

            case PHYS_CONVEX:
            case PHYS_MESH  :
            {
               PhysMesh pm; pm._type=_type;
               if(f.getBool())if(!New(pm._base)->loadData(f))goto error;
               if(pm._physx_cooked_data._loadRaw(f))
               {
                  if(UInt bullet_ver=f.decUIntV())
                  {
                     if(bullet_ver!=1)f.skip(f.getUInt());else // other versions aren't supported
                     {
                        f>>pm._bullet_cooked_data_size;
                              pm._bullet_cooked_data=AlignedAlloc(pm._bullet_cooked_data_size);
                        f.get(pm._bullet_cooked_data,             pm._bullet_cooked_data_size);
                     }
                  }
                  if(f.ok())
                  {
                     Loaded(pm);
                     pm.setBox();
                     Set(pm, T._pm);
                     return true;
                  }
               }
            }break;
         }
      }break;

      case 2:
      {
         f.getMulti(_type, density);
         switch(_type)
         {
            case PHYS_NONE: if(f.ok())return true; break;

            case PHYS_SHAPE: if(shape.load(f) && f.ok())return true; break;

            case PHYS_CONVEX:
            case PHYS_MESH  :
            {
               PhysMesh pm; pm._type=_type;
               if(f.getBool())if(!New(pm._base)->loadData(f))goto error;
               if(pm._physx_cooked_data._loadRaw(f))
               if(f.ok())
               {
                  Loaded(pm);
                  pm.setBox();
                  Set(pm, T._pm);
                  return true;
               }
            }break;
         }
      }break;

      case 1:
      {
         f.getMulti(_type, density);
         switch(_type)
         {
            case PHYS_NONE: if(f.ok())return true; break;

            case PHYS_SHAPE: if(shape.load(f) && f.ok())return true; break;

            case PHYS_CONVEX:
            case PHYS_MESH  :
            {
               PhysMesh pm; pm._type=_type;
               if(pm._physx_cooked_data._loadRaw(f))
               if(f.ok())
               {
                  Loaded(pm);
                  pm.setBox();
                  Set(pm, T._pm);
                  return true;
               }
            }break;
         }
      }break;

      case 0:
      {
         Byte unused; f.getMulti(unused, _type, density);
         switch(_type)
         {
            case PHYS_NONE: if(f.ok())return true; break;

            case PHYS_SHAPE:
            {
					shape.type=SHAPE_TYPE(Byte(f.getUInt())); // needed, because in the past it was saved as raw memory
               Long pos=f.pos();
            	switch(shape.type)
               {
                  default           : goto error;
                  case SHAPE_NONE   : break;
                  case SHAPE_POINT  : f>>shape.point  ; break;
                  case SHAPE_EDGE   : f>>shape.edge   ; break;
                  case SHAPE_RECT   : f>>shape.rect   ; break;
                  case SHAPE_BOX    : f>>shape.box    ; break;
                  case SHAPE_OBOX   : f>>shape.obox   ; break;
                  case SHAPE_CIRCLE : f>>shape.circle ; break;
                  case SHAPE_BALL   : f>>shape.ball   ; break;
                  case SHAPE_CAPSULE: f>>shape.capsule; break;
                  case SHAPE_TUBE   : f>>shape.tube   ; break;
                  case SHAPE_TORUS  : f>>shape.torus  ; break;
                  case SHAPE_CONE   : f>>shape.cone   ; break;
                  case SHAPE_PYRAMID: f>>shape.pyramid; break;
                  case SHAPE_PLANE  : f>>shape.plane  ; break;
               }
               if(f.pos(pos+72) && f.ok())return true; // 72 is the old size of raw memory for the shape data
            }break;

            case PHYS_CONVEX:
            case PHYS_MESH  :
            {
               PhysMesh pm; pm._type=_type;
               if(pm._physx_cooked_data._loadRaw(f))
               if(f.ok())
               {
                  Loaded(pm);
                  pm.setBox();
                  Set(pm, T._pm);
                  return true;
               }
            }break;
         }
      }break;
   }
error:
   del(); return false;
}

Bool PhysPart::save(File &f)C
{
   f.putUInt(CC4_PHSP);
   return saveData(f);
}
Bool PhysPart::load(File &f)
{
   switch(f.getUInt())
   {
      case CC4_PHSB: // compatibility reasons (this was used in the past)
      case CC4_PHSP: return loadData(f);

      case CC4_PHYS:
      {
         PhysBody phys; if(phys.loadData(f))
         {
            if(phys.parts.elms())Swap(T, phys.parts[0]);else del();
            return true;
         }
      }break;
   }
   del(); return false;
}

Bool PhysPart::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool PhysPart::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
// PHYSICAL BODY
/******************************************************************************/
void PhysBody::zero() {density=1; box.zero(); material=null;}

PhysBody::PhysBody() {zero();}

PhysBody& PhysBody::del()
{
   parts.del(); zero();
   return T;
}
/******************************************************************************/
Flt  PhysBody::area        ()C {Flt area  =0; REPA(T)area  +=parts[i].area  (); return area  ;}
Flt  PhysBody::volume      ()C {Flt volume=0; REPA(T)volume+=parts[i].volume(); return volume;}
Flt  PhysBody::mass        ()C {Flt mass  =0; REPA(T)mass  +=parts[i].mass  (); return mass*finalDensity();}
Flt  PhysBody::finalDensity()C {return material ? material->density()*density : density;}
Bool PhysBody::setBox      ()
{
   Bool found=false;
   Box  temp;
   REPA(T)if(parts[i].getBox(temp))
   {
      if(!found){found=true; box=temp;}else box|=temp;
   }
   if(!found)box.zero(); return found;
}
/******************************************************************************/
PhysBody& PhysBody::transform(C Matrix3 &matrix) {REPAO(parts)*=matrix  ; setBox(); return T;}
PhysBody& PhysBody::transform(C Matrix  &matrix) {REPAO(parts)*=matrix  ; setBox(); return T;}
PhysBody& PhysBody::mirrorX  (                 ) {REPAO(parts).mirrorX(); setBox(); return T;}
PhysBody& PhysBody::mirrorY  (                 ) {REPAO(parts).mirrorY(); setBox(); return T;}
PhysBody& PhysBody::mirrorZ  (                 ) {REPAO(parts).mirrorZ(); setBox(); return T;}
/******************************************************************************/
Bool PhysBody::adjustStorage(Bool universal, Bool physx, Bool bullet, Bool *changed)
{
   if(changed)*changed=false;
   Bool   ok=true, c; REPA(T){ok&=parts[i].adjustStorage(universal, physx, bullet, &c); if(changed)*changed|=c;}
   return ok;
}
PhysBody& PhysBody::freeHelperData() {REPAO(parts).freeHelperData(); return T;}
/******************************************************************************/
void PhysBody::draw(C Color &color)C
{
   REPAO(parts).draw(color);
}
/******************************************************************************/
Bool PhysBody::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(1), density, box); // version
   f._putStr(PhysMtrls.name(material, path));
   f.cmpUIntV(parts.elms()); FREPA(T)if(!parts[i].saveData(f))return false;
   return f.ok();
}
Bool PhysBody::loadData(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 1:
      {
         f.getMulti(density, box);
         material=PhysMtrls(f._getStr(), path);
         parts.setNum(f.decUIntV()); FREPA(T)if(!parts[i].loadData(f))return false;
         if(f.ok())return true;
      }break;

      case 0:
      {
         f.skip(1); // old version byte
         parts.setNum(f.getInt());
         f.skip(1); // flag
         f>>density;
         material=null;
         FREPA(T)if(!parts[i].loadData(f))return false;
         setBox(); // set after loading all data
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}

Bool PhysBody::save(File &f, CChar *path)C
{
   f.putUInt(CC4_PHYS);
   return saveData(f, path);
}
Bool PhysBody::load(File &f, CChar *path)
{
   switch(f.getUInt())
   {
      case CC4_PHYS: return loadData(f, path);

      case CC4_PHSB: // compatibility reasons (this was used in the past)
      case CC4_PHSP:
      {
         if(del().parts.New().loadData(f)){setBox(); return true;}
      }break;
   }
   del(); return false;
}

Bool PhysBody::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool PhysBody::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
Bool PhysBody::loadAdd(File &f, CChar *path)
{
   if(!parts.elms())return load(f, path);

   PhysBody phys;
   if(phys.load(f, path))
   {
      if(phys.parts.elms())
      {
         density=Avg(density, phys.density);
         box   |=phys.box;
         Int start=parts.addNum(phys.parts.elms());
         FREPA(phys)Swap(parts[start+i], phys.parts[i]);
      }
      return true;
   }
   return false;
}
/******************************************************************************/
void PhysPart ::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load Physical Body \""+name+"\"");}
void PhysBody ::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load Physical Body \""+name+"\"");}
/******************************************************************************/
// PHYSICAL BODY GROUP
/******************************************************************************
struct PhysGroup // Physical Body Group (array of PhysBodies)
{
   Mems<PhysBody> bodies; // physical bodies

   // manage
   PhysGroup& del   (               ); // delete
   PhysGroup& create(MeshLod   &mshl); // create from MeshLod
   PhysGroup& create(MeshGroup &mshg); // create from MeshGroup

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f   )C; // save, false on fail
   Bool load     (  File &f   ) ; // load, false on fail
#if EE_PRIVATE
   Bool saveData(File &f)C; // save, false on fail
   Bool loadData(File &f) ; // load, false on fail
#endif
};
inline Int Elms(C PhysGroup &phsg) {return phsg.bodies.elms();}
/******************************************************************************
PhysGroup& PhysGroup::del()
{
   bodies.del(); return T;
}
/******************************************************************************
PhysGroup& PhysGroup::create(MeshLod &mshl)
{
   del();
   MeshBase mesh; mesh.createPhys(mshl);
   if(mesh.is())
   {
      PhysBody &phys=bodies.New();
      phys.density=0;
      phys.parts.New().createMesh(mesh);
      phys.setBox();
   }
   return T;
}
PhysGroup& PhysGroup::create(MeshGroup &mshg)
{
   del();
   MeshBase mesh;
   FREPA(mshg)
   {
      mesh.createPhys(mshg.meshes[i]);
      if(mesh.is())
      {
         PhysBody &phys=bodies.New();
       //phys.flag   =0;
         phys.density=0;
         phys.parts.New().createMesh(mesh);
         phys.setBox();
      }
   }
   return T;
}
/******************************************************************************
Bool PhysGroup::saveData(File &f)C
{
   f.cmpUIntV(0); // version
   f.cmpUIntV(bodies.elms()); FREPA(T)if(!bodies[i].saveData(f))return false;
   return f.ok();
}
Bool PhysGroup::loadData(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         bodies.setNum(f.decUIntV()); FREPA(T)if(!bodies[i].loadData(f))return false;
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}

Bool PhysGroup::save(File &f)C
{
   f.putUInt(CC4_PHSG);
   return saveData(f);
}
Bool PhysGroup::load(File &f)
{
   switch(f.getUInt())
   {
      case CC4_PHSG: return loadData(f);
   }
   del(); return false;
}

Bool PhysGroup::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool PhysGroup::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}

void PhysGroup::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load Physical Body \""+name+"\"");}
/******************************************************************************/
}
/******************************************************************************/
