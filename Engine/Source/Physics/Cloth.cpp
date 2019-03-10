/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "PhysX Stream.h"
/******************************************************************************/
#define CC4_CLMS CC4('C','L','M','S')
/******************************************************************************/
Cache<ClothMesh> ClothMeshes("Cloth Mesh"); // ClothMesh Cache
/******************************************************************************/
// CLOTH MESH
/******************************************************************************/
#if PHYSX
static PxClothFabric* CreateClothMesh(MeshBase &mesh, Flt scale) // assumes that 'mesh' doesn't contain quads
{
   if(Physx.physics && mesh.vtxs() && mesh.tris())
   {
      PxClothMeshDesc desc;

      desc.points   .data  =mesh.vtx.pos();
      desc.points   .count =mesh.vtxs();
      desc.points   .stride=SIZE(Vec);
      desc.triangles.data  =mesh.tri.ind();
      desc.triangles.count =mesh.tris();
      desc.triangles.stride=SIZE(VecI);

      // scale
      Vec *pos_scaled=null;
      if(!Equal(scale, 1))
      {
         desc.points.data=Alloc(pos_scaled, mesh.vtxs()); REPA(mesh.vtx)pos_scaled[i]=mesh.vtx.pos(i)*scale;
      }

      // cook cloth mesh
      OutputStream buf;
      PxClothFabricCooker cook(desc, Physx.vec(Vec(0,-1,0)), true); cook.save(buf, false);
      Free(pos_scaled);

      if(!buf.data || !buf.pos)Exit("ClothMesh.create");

      WriteLock lock(Physics._rws);
      InputStream input(buf.data, buf.pos);
      return Physx.physics->createClothFabric(input);
   }
   return null;
}
PxClothFabric* ClothMesh::scaledMesh(Flt scale)
{
   // find existing
   REPA(_scales)if(Equal(_scales[i].scale, scale))return _scales[i].mesh;

   // create new one
   if(PxClothFabric *mesh=CreateClothMesh(_phys, scale))
   {
      Scale &s=_scales.New();
             s.scale=scale;
      return s.mesh =mesh ;
   }
   return null;
}
#endif
/******************************************************************************/
ClothMesh& ClothMesh::del()
{
#if PHYSX
   if(_scales.elms())
   {
      SafeWriteLock lock(Physics._rws);
      if(Physx.physics)REPA(_scales)_scales[i].mesh->release();
   }
#endif
  _scales  .del();
  _phys    .del();
  _skin    .del();
  _ind_buf .del();
  _bone_map.del();
  _material=null;
  _skeleton=null;
   return T;
}
ClothMesh::ClothMesh()
{
   REPAO(_skin_shader)=null;
   REPAO(_phys_shader)=null;
  _skin_frst=null;
  _phys_frst=null;
  _skeleton =null;
}
/******************************************************************************/
ClothMesh& ClothMesh::create(C MeshBase &mesh, C MaterialPtr &material, Skeleton *skeleton)
{
   del();

   if(!mesh.vtx.flag() || !mesh.vtx.matrix() || !mesh.vtx.blend()) // if we're putting the whole thing to phys
   {
     _phys.create(mesh, VTX_TEX0|VTX_FLAG).quadToTri();
   }else
   {
   /* Rules:

      Vtx:
         All vtx with VTX_FLAG_CLOTH go to Cloth
         Neighbor vtx of VTX_FLAG_CLOTH have unified skin and go to Cloth and MeshRender
         Rest goes only to MeshRender

      Tri:
         At least one vtx with VTX_FLAG_CLOTH -> Cloth
         No                    VTX_FLAG_CLOTH -> MeshRender
   */

      MeshBase temp;
      temp.create(mesh, VTX_NRM|VTX_TEX0|VTX_SKIN|VTX_FLAG).quadToTri();

      // unify skin
      Memt<Bool> cloth_neighbor; cloth_neighbor.setNumZero(temp.vtxs()); // cloth or neighbor
      Memt<Bool> tri_as_cloth; tri_as_cloth.setNum(temp.tris());
      Int        tris_cloth=0,
                 tris_skin =0;
      REPA(temp.tri)
      {
         Int *p=temp.tri.ind(i).c, p0=p[0], p1=p[1], p2=p[2];
         if(tri_as_cloth[i]=FlagTest(temp.vtx.flag(p0)|temp.vtx.flag(p1)|temp.vtx.flag(p2), VTX_FLAG_CLOTH)) // if at least one vtx is meant for phys cloth
         {
            // set all to cloth and neighbor
            cloth_neighbor[p0]=true;
            cloth_neighbor[p1]=true;
            cloth_neighbor[p2]=true;
            tris_cloth++;
         }else
         {
            tris_skin++;
         }
      }
      if(tris_cloth && tris_skin<=8)REPA(temp.tri) // if only 8 tris left in skinning version, then put them also to cloth (this will probably be faster instead of additional overhead for separate MeshRender drawing)
      {
         if(!tri_as_cloth[i]) // if not yet added to cloth
         {
            Int *p=temp.tri.ind(i).c, p0=p[0], p1=p[1], p2=p[2];
            tri_as_cloth  [ i]=true;
            cloth_neighbor[p0]=true;
            cloth_neighbor[p1]=true;
            cloth_neighbor[p2]=true;
         }
      }
      REPA(temp.vtx)if(cloth_neighbor[i]) // is neighbor or cloth
      {
         // unify skinning
         VecB4 &blend=temp.vtx.blend (i),
               &bone =temp.vtx.matrix(i);
         Swap(bone.c[0], bone.c[blend.maxI()]); // put most important bone to first slot
         blend.set(255, 0, 0, 0); // set max weight for first bone
      }

      // create mesh
      temp.splitFaces(_phys, null, tri_as_cloth, null, VTX_TEX0|VTX_MATRIX|VTX_FLAG); // move selected triangles cloth/neighbors to cloth keeping flags and bone indexes
     _skin.create    (temp                                                         ); // create remaining skinned part as MeshRender

      T.skeleton(skeleton);
   }

   T._material=material;
   if(_ind_buf.create(_phys.tris()*3, _phys.vtxs()<=0x10000))if(Ptr dest=_ind_buf.lock(LOCK_WRITE))
   {
      if(_ind_buf.bit16())Copy32To16(dest, _phys.tri.ind(), _ind_buf._ind_num);
      else                Copy32To32(dest, _phys.tri.ind(), _ind_buf._ind_num);
     _ind_buf.unlock();
   }
   setShader();

   return T;
}
/******************************************************************************/
ClothMesh& ClothMesh::skeleton(Skeleton *skeleton)
{
   T._skeleton=skeleton;
   if(skeleton && skeleton->is() && !_bone_map.same(*skeleton)) // remap only if new skeleton exists (in case it failed to load, then it could be null or have no bones, or the parameter could just have been specifically set to null in order to clear the skeleton link) and if the existing bone map is different than of the new skeleton
   {
      if(_bone_map.is()) // remap vertexes only if the mesh already had a bone map (otherwise the vertex bone could be set, but the bone map doesn't exist yet, and in that case the vertex bone data could be lost if we've performed remapping)
      {
         Memt<Byte, 256> old_to_new;
        _bone_map.setRemap(*skeleton, old_to_new);
         boneRemap(old_to_new, false); // set false to not waste time on adjusting the '_bone_map' itself, because we're going to recreate it anyway below
      }

      // setup new mesh bone info
     _bone_map.create(*skeleton);
   }
   return T;
}
/******************************************************************************/
//Bool boneRename(C Str8 &src, C Str8 &dest                             ); // rename 'src' bone to 'dest' bone, returns true if a bone was renamed
//Bool ClothMesh::boneRename(C Str8 &src, C Str8 &dest) {return _bone_map.rename(src, dest);}

void ClothMesh::boneRemap (C MemPtr<Byte, 256> &old_to_new, Bool remap_names)
{
                                       _phys.boneRemap(old_to_new);
   if(_skin.is()){MeshBase temp(_skin); temp.boneRemap(old_to_new); _skin.create(temp);}
   if(remap_names)_bone_map.remap(old_to_new);
}
void ClothMesh::setShader()
{
   DefaultShaders(_material(), _skin.flag()            , 0, false).set(_skin_shader, &_skin_frst, null);
   DefaultShaders(_material(), VTX_POS|VTX_NRM|VTX_TEX0, 0, false).set(_phys_shader, &_phys_frst, null);
}
/******************************************************************************/
Bool ClothMesh::saveData(File &f, CChar *path)C
{
   f.cmpUIntV(2); // version
   f._putStr(_material.name(path));
   if(_phys    .saveData(f))
   if(_skin    .saveData(f))
   if(_ind_buf .save    (f))
   if(_bone_map.saveOld (f))
   {
      f._putStr(Skeletons.name(skeleton(), path));
      // physx cooked data isn't saved, because later physx versions can't read older cloth versions (at least that's how it was in the past)
      return f.ok();
   }
   return false;
}
Bool ClothMesh::loadData(File &f, CChar *path)
{
   del();

   switch(f.decUIntV()) // version
   {
      case 2:
      {
        _material.require(f._getStr(), path);
         if(_phys    .loadData(f))
         if(_skin    .loadData(f))
         if(_ind_buf .load    (f))
         if(_bone_map.loadOld (f))
         {
            skeleton(Skeletons(f._getStr(), path));
            setShader();
            if(f.ok())return true;
         }
      }break;

      case 1:
      {
        _material.require(f._getStr(), path);
         if(!_phys.loadData(f))goto error;
         if(!_skin.loadData(f))goto error;
         if( _ind_buf.create(_phys.tris()*3, _phys.vtxs()<=0x10000))if(Ptr dest=_ind_buf.lock(LOCK_WRITE))
         {
            if(_ind_buf.bit16())Copy32To16(dest, _phys.tri.ind(), _ind_buf._ind_num);
            else                Copy32To32(dest, _phys.tri.ind(), _ind_buf._ind_num);
           _ind_buf.unlock();
         }
         if(!_bone_map.loadOld(f))goto error;
         skeleton(Skeletons(f._getStr(), path));

         setShader();
         if(f.ok())return true;
      }break;

      case 0:
      {
        _material.require(f._getStr(), path);
         if(!_phys.loadData(f))goto error;
         if(!_skin.loadData(f))goto error;
         if( _ind_buf.create(_phys.tris()*3, _phys.vtxs()<=0x10000))if(Ptr dest=_ind_buf.lock(LOCK_WRITE))
         {
            if(_ind_buf.bit16())Copy32To16(dest, _phys.tri.ind(), _ind_buf._ind_num);
            else                Copy32To32(dest, _phys.tri.ind(), _ind_buf._ind_num);
           _ind_buf.unlock();
         }
         setShader();
         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}

Bool ClothMesh::save(File &f, CChar *path)C
{
   f.putUInt(CC4_CLMS);
   return saveData(f, path);
}
Bool ClothMesh::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_CLMS)return loadData(f, path);
   del(); return false;
}

Bool ClothMesh::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool ClothMesh::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
// CLOTH
/******************************************************************************/
Cloth::Cloth() {_update_count=0; _scale=1; _cloth_mesh=null; _vtxs=null; _vtx=null; _cloth=null; _lock=null;}
Cloth& Cloth::del()
{
   unlock();
#if PHYSX
   if(_cloth)
   {
      SafeWriteLock lock(Physics._rws);
      if(_cloth)
      {
         if(Physx.physics)_cloth->release();
        _cloth=null;
      }
   }
#else
   // TODO: Bullet
#endif
  _vtx_buf.del();
  _update_count=Physics._update_count-1; _scale=1; _cloth_mesh=null;
   return T;
}
Bool Cloth::create(ClothMesh &cloth_mesh, C Matrix &matrix)
{
   del();

#if PHYSX
   if(Physx.physics && Physx.world)
   {
      Matrix m=matrix;
        _scale=m.x.normalize();
               m.y.normalize();
               m.z.normalize();

      if(PxClothFabric *cm=cloth_mesh.scaledMesh(_scale))
      {
         Memt<PxClothParticle> particles;
         particles.setNum(cloth_mesh._phys.vtxs());
         REPA(particles)
         {
            particles[i].pos      =Physx.vec(cloth_mesh._phys.vtx.pos(i)*matrix);
            particles[i].invWeight=1.0f/1;
         }
         WriteLock lock(Physics._rws);
         if(_cloth=Physx.physics->createCloth(PxTransform(PxIdentity), *cm, particles.data(), PxClothFlags()))
            if(_vtx_buf.createNum(SIZE(Vtx), cloth_mesh._phys.vtxs(), true))
         {
            T._cloth_mesh=&cloth_mesh;
           _cloth->setSolverFrequency(Physics.precision());
            Physx.world->addActor(*_cloth);
            return true;
         }
      }
      del();
   }
#endif
   return false;
}
/******************************************************************************/
#if PHYSX
C Cloth::Particle* Cloth::lockRead()
{
#if !APPLE // fails to compile
   ASSERT(SIZE  (*_lock->particles         )==SIZE  (Particle              ));
   ASSERT(OFFSET(PxClothParticle, pos      )==OFFSET(Particle, pos         ));
   ASSERT(OFFSET(PxClothParticle, invWeight)==OFFSET(Particle, inverse_mass));
#endif
   if(_cloth && !_lock)_lock=_cloth->lockParticleData(PxDataAccessFlag::eREADABLE);
   return _lock ? (Particle*)_lock->particles : null;
}
Cloth& Cloth::unlock()
{
   if(_lock)
   {
      if(Physx.world)_lock->unlock(); // in case we're unlocking after deleting physics
     _lock=null;
   }
   return T;
}
Cloth& Cloth::set(C Particle *particle, Int particles)
{
   if(_cloth && particle && particles>=_cloth->getNbParticles())_cloth->setParticles((C PxClothParticle*)particle, null);
   return T;
}
Cloth& Cloth::setCollisionBalls(C MemPtr<Ball> &balls)
{
   if(_cloth)
   {
      PxClothCollisionSphere sphere[32]; Int elms=Min(Elms(sphere), balls.elms());
      REP(elms)
      {
         sphere[i].radius=          balls[i].r   ;
         sphere[i].pos   =Physx.vec(balls[i].pos);
      }
     _cloth->setCollisionSpheres(sphere, elms);
   }
   return T;
}
Cloth& Cloth::setCollisionCapsules(C MemPtr<VecI2> &capsules)
{
   if(_cloth)
   {
      // remove all
      REP(_cloth->getNbCollisionCapsules())_cloth->removeCollisionCapsule(i);

      // add
      Int  elms=Min(capsules.elms(), 32);
      FREP(elms)_cloth->addCollisionCapsule(capsules[i].x, capsules[i].y);
   }
   return T;
}
#else
C Cloth::Particle* Cloth::  lockRead() {return null;}
Cloth&             Cloth::unlock    () {return T;}
Cloth&             Cloth::set                 (C Particle      *particle, Int particles) {return T;}
Cloth&             Cloth::setCollisionBalls   (C MemPtr<Ball > &balls   ) {return T;}
Cloth&             Cloth::setCollisionCapsules(C MemPtr<VecI2> &capsules) {return T;}
#endif
/******************************************************************************/
// GET / SET
/******************************************************************************/
#if PHYSX
Int    Cloth::vtxs (            )C {return  _cloth_mesh ? _cloth_mesh->_phys.vtxs() : 0;}
Ptr    Cloth::obj  (            )C {return  _cloth ? (Ptr)_cloth->getName() :  null;}
Cloth& Cloth::obj  (Ptr    obj  )  {if     (_cloth)       _cloth->setName((char*)obj); return T;}
Bool   Cloth::sleep(            )C {return  _cloth ? _cloth->isSleeping() : false;}
Cloth& Cloth::sleep(Bool   sleep)  {if     (_cloth)if(sleep)_cloth->putToSleep();else _cloth->wakeUp(); return T;}
Vec    Cloth::wind (            )C {return  _cloth ? Physx.vec(_cloth->getExternalAcceleration()) : 0;}
Cloth& Cloth::wind (C Vec &accel)  {if     (_cloth)            _cloth->setExternalAcceleration(Physx.vec(accel)); return T;}
Box    Cloth::box  (            )C {return  _cloth ? Physx.box(_cloth->getWorldBounds()) : Box(0);}
Bool   Cloth::ccd  (            )C {return  _cloth ? FlagTest((UInt)_cloth->getClothFlags(), PxClothFlag::eSWEPT_CONTACT) : false;}
Cloth& Cloth::ccd  (Bool      on)  {if     (_cloth)                 _cloth->setClothFlag (   PxClothFlag::eSWEPT_CONTACT, on); return T;}
Bool   Cloth::gpu  (            )C {return  _cloth ? FlagTest((UInt)_cloth->getClothFlags(), PxClothFlag::eGPU          ) : false;}
Cloth& Cloth::gpu  (Bool      on)  {if     (_cloth)                 _cloth->setClothFlag (   PxClothFlag::eGPU          , on); return T;}
#else
Int    Cloth::vtxs (            )C {return  0    ;}
Ptr    Cloth::obj  (            )C {return  null ;}
Cloth& Cloth::obj  (Ptr    obj  )  {return  T    ;}
Bool   Cloth::sleep(            )C {return  false;}
Cloth& Cloth::sleep(Bool   sleep)  {return  T    ;}
Vec    Cloth::wind (            )C {return  0    ;}
Cloth& Cloth::wind (C Vec &accel)  {return  T    ;}
Bool   Cloth::ccd  (            )C {return  false;}
Cloth& Cloth::ccd  (Bool      on)  {return  T    ;}
Bool   Cloth::gpu  (            )C {return  false;}
Cloth& Cloth::gpu  (Bool      on)  {return  T    ;}
Box    Cloth::box  (            )C {return  0    ;}
#endif
/******************************************************************************/
// PARAMETERS
/******************************************************************************/
#if PHYSX
Flt Cloth::damping        ()C {return _cloth ? _cloth->getDampingCoefficient    ().x : 0;}
Flt Cloth::friction       ()C {return _cloth ? _cloth->getFrictionCoefficient   ()   : 0;}
Flt Cloth::   drag        ()C {return _cloth ? _cloth->getLinearDragCoefficient ().x : 0;}
Flt Cloth::angDrag        ()C {return _cloth ? _cloth->getAngularDragCoefficient().x : 0;}
Flt Cloth::   inertiaScale()C {return _cloth ? _cloth->getLinearInertiaScale    ().x : 0;}
Flt Cloth::angInertiaScale()C {return _cloth ? _cloth->getAngularInertiaScale   ().x : 0;}
Flt Cloth::bending        ()C {return _cloth ? 1-_cloth->getStretchConfig(PxClothFabricPhaseType::eBENDING).stiffness : 0;}

Cloth& Cloth::damping        (Flt damping ) {if(_cloth)_cloth->setDampingCoefficient    (Physx.vec(Sat(damping ))); return T;}
Cloth& Cloth::friction       (Flt friction) {if(_cloth)_cloth->setFrictionCoefficient   (          Sat(friction) ); return T;}
Cloth& Cloth::   drag        (Flt drag    ) {if(_cloth)_cloth->setLinearDragCoefficient (Physx.vec(Sat(drag    ))); return T;}
Cloth& Cloth::angDrag        (Flt drag    ) {if(_cloth)_cloth->setAngularDragCoefficient(Physx.vec(Sat(drag    ))); return T;}
Cloth& Cloth::   inertiaScale(Flt scale   ) {if(_cloth)_cloth->setLinearInertiaScale    (Physx.vec(Sat(scale   ))); return T;}
Cloth& Cloth::angInertiaScale(Flt scale   ) {if(_cloth)_cloth->setAngularInertiaScale   (Physx.vec(Sat(scale   ))); return T;}
Cloth& Cloth::bending        (Flt bending ) {if(_cloth){PxClothStretchConfig config; config.stiffness=1-bending; _cloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, config);} return T;}
#else
Flt Cloth::damping        ()C {return 0;}
Flt Cloth::friction       ()C {return 0;}
Flt Cloth::   drag        ()C {return 0;}
Flt Cloth::angDrag        ()C {return 0;}
Flt Cloth::   inertiaScale()C {return 0;}
Flt Cloth::angInertiaScale()C {return 0;}
Flt Cloth::bending        ()C {return 0;}

Cloth& Cloth::damping        (Flt damping ) {return T;}
Cloth& Cloth::friction       (Flt friction) {return T;}
Cloth& Cloth::   drag        (Flt drag    ) {return T;}
Cloth& Cloth::angDrag        (Flt drag    ) {return T;}
Cloth& Cloth::   inertiaScale(Flt scale   ) {return T;}
Cloth& Cloth::angInertiaScale(Flt scale   ) {return T;}
Cloth& Cloth::bending        (Flt bending ) {return T;}
#endif
/******************************************************************************/
// DRAW
/******************************************************************************/
void Cloth::update()
{
	if(_update_count!=Physics._update_count)
	{
	  _update_count=Physics._update_count;
	#if PHYSX
	   if(C Particle *particles=lockRead())
	   {
	      if(Vtx *vtx=(Vtx*)_vtx_buf.lock(LOCK_WRITE))
	      {
	       C Vec2 *tex=_cloth_mesh->_phys.vtx.tex0();
	         Vtx  *v  =vtx;
	         REPA(_cloth_mesh->_phys.vtx)
	         {
	            v->nrm.zero();
	            v->pos=(particles++)->pos;
	            if(tex)v->tex=*tex++;else v->tex.zero();
	            v++;
	         }
	       C VecI *ind=_cloth_mesh->_phys.tri.ind();
	         REPA(_cloth_mesh->_phys.tri)
	         {
	            Vec nrm=GetNormalU(vtx[ind->x].pos, vtx[ind->y].pos, vtx[ind->z].pos);
	            vtx[ind->x].nrm+=nrm;
	            vtx[ind->y].nrm+=nrm;
	            vtx[ind->z].nrm+=nrm;
	            ind++;
	         }
	         REPA(_cloth_mesh->_phys.vtx)vtx[i].nrm.normalize();
	        _vtx_buf.unlock();
	      }
	      unlock();
	   }
	#endif
	}
}
/******************************************************************************/
void Cloth::_drawPhysical()C
{
#if DX9
  _vtx_buf.set();
  _cloth_mesh->_ind_buf.set();
   D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _vtx_buf.vtxs(), 0, _cloth_mesh->_phys.tris());
#elif DX11
  _vtx_buf.set();
  _cloth_mesh->_ind_buf.set();
   D3DC->DrawIndexed(_cloth_mesh->_ind_buf._ind_num, 0, 0);
#elif GL
   SetDefaultVAO(); _vtx_buf.set(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
  _cloth_mesh->_ind_buf.set();
   glDrawElements(GL_TRIANGLES, _cloth_mesh->_ind_buf._ind_num, _cloth_mesh->_ind_buf.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null);
#endif
}
/******************************************************************************/
void Cloth::drawSkinnedOutline(C AnimatedSkeleton &anim_skel, C Color &color)C
{
   if(_cloth_mesh)if(Shader *shader=_cloth_mesh->_skin_shader[RM_OUTLINE])
   {
      anim_skel.setMatrix();
    C Material &mtrl=GetMaterial(_cloth_mesh->material()());
      Renderer.setOutline(color);
      D.depth(true);
      mtrl.setOutline(); D.cull(mtrl.cull);
      shader->begin(); _cloth_mesh->_skin.set().draw();
      ShaderEnd();
   }
}
INLINE void ClothInstances::add(C Cloth &cloth, Shader &shader, C Material &material)
{
   ClothInstance &ci=New();
   ci.cloth    =&cloth;
   ci.shader   =&shader;
   ci.material =&material; material.incUsage();
}
INLINE void ClothInstances::add(C Cloth &cloth, Shader &shader, C Material &material, C Vec &vel)
{
   ClothInstance &ci=New();
   ci.cloth    =&cloth;
   ci.shader   =&shader;
   ci.material =&material; material.incUsage();
   ci.vel      = vel;
   ci.highlight= Renderer._mesh_highlight;
}
INLINE void ClothInstances::add(C Cloth &cloth, FRST &frst, C Material &material)
{
   ClothInstance &ci=New();
   ci.cloth    =&cloth;
   ci.shader   =&frst;
   ci.material =&material; material.incUsage();
   ci.highlight= Renderer._mesh_highlight;
}
void Cloth::drawPhysical(C Vec &vel)C
{
   if(_cloth && Frustum(box()))
   {
      ConstCast(T).update();
    C Material &material=GetMaterial(_cloth_mesh->material()());
      switch(Renderer._cur_type)
      {
         case RT_DEFERRED:
         case RT_SIMPLE  : if(Shader *shader=_cloth_mesh->_phys_shader[Renderer._solid_mode_index])
         {
            SolidClothInstances.add(T, *shader, material, vel);
         }break;

         case RT_FORWARD: if(FRST *frst=_cloth_mesh->_phys_frst)if(Renderer.firstPass() || frst->all_passes)//if(Shader *shader=frst->getShader())
         {
            SolidClothInstances.add(T, *frst, material); // velocities not needed in forward renderer
         }break;
      }
   }
}
void Cloth::drawPhysicalShadow()C
{
   if(_cloth && Frustum(box()))
      if(Shader *shader=_cloth_mesh->_phys_shader[RM_SHADOW])
   {
      ConstCast(T).update();
      ShadowClothInstances.add(T, *shader, GetMaterial(_cloth_mesh->material()()));
   }
}
void Cloth::drawPhysicalOutline(C Color &color)C
{
   if(_cloth && Frustum(box()))
      if(Shader *shader=_cloth_mesh->_phys_shader[RM_OUTLINE])
   {
      ConstCast(T).update();
    C Material &mtrl=GetMaterial(_cloth_mesh->material()());
      Renderer.setOutline(color);
      SetOneMatrix();
      D.depth(true);
      SetDefaultVAO(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
      D.cull (mtrl.cull); mtrl.setOutline();

      SetVtxNrmMulAdd(false);
      shader->begin(); _drawPhysical();
      ShaderEnd();
      SetVtxNrmMulAdd(true);
   }
}
/******************************************************************************/
}
/******************************************************************************/
