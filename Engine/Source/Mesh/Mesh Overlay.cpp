/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_MSOV CC4('M','S','O','V')
/******************************************************************************/
static void SetBoxAndPlanes(Box &box, Memc<Plane> &planes, Matrix &matrix)
{
   box.set(1)*=matrix;

   planes.del();
   Vec n;

   // x
   n=matrix.x; n.normalize();
   planes.New().set(matrix.pos+matrix.x,  n);
   planes.New().set(matrix.pos-matrix.x, -n);

   // y
   n=matrix.y; n.normalize();
   planes.New().set(matrix.pos+matrix.y,  n);
   planes.New().set(matrix.pos-matrix.y, -n);

   // z
   n=matrix.z; n.normalize();
   planes.New().set(matrix.pos+matrix.z,  n);
   planes.New().set(matrix.pos-matrix.z, -n);
}
static void ClipMeshToPlanes(C Mesh &src, C Matrix *matrix, Mesh &dest, C Plane *clip_plane, Int clip_planes, C Box &box)
{
   Mesh temp;

   // box test
   Vec  corner[8]; src.ext.toCorners(corner); if(matrix)Transform(corner, *matrix, Elms(corner));
   Bool all_inside=true;
   REPD(p, clip_planes)
   {
      Bool  inside=false,
           outside=false;
      REPAD(c, corner)
      {
         if(Dist(corner[c], clip_plane[p])<=0)inside=true;else outside=true;
      }
      if(!inside)goto finished; // if no vertexes are inside the plane then cancel
      if(outside)all_inside=false;
   }

   // create dest
   /*if(all_inside) can't be used because it may happen that we're operating on MeshRender
   {
      temp.create(src, flag_and);
      if(matrix)
      {
         temp.transform(*matrix).setBox();
         temp.lod_center=src.lod_center*(*matrix);
      }
   }else*/
   {
      Bool              poly_cur=0;
      Flt               scale=(matrix ? matrix->maxScale() : 1);
      Memc< Vec>        poly[2];
      Memc< Memc<Vec> > polys;

      temp.copyParams(src);

      FREPD(l, src.lods()) // order is important
      {
       C MeshLod &src_lod=src.lod(l);
         REPA(src_lod)
         {
            // create 'polys' from MeshPart
          C MeshPart &part=src_lod.parts[i];
            
            Bool semi_transparent=false; if(part.material())if(part.material()->hasAlpha() || part.material()->technique==MTECH_FUR)semi_transparent=true;
            if( !semi_transparent) // don't apply overlays on semi-transparent parts (because it will look like floating in air)
            {
               if(part.base.is()) // MSHB
               {
                C Vec *pos=part.base.vtx.pos();
                  REPA(part.base.tri) // TRI
                  {
                     VecI t=part.base.tri.ind(i);
                     poly[poly_cur].New()=pos[t.x];
                     poly[poly_cur].New()=pos[t.y];
                     poly[poly_cur].New()=pos[t.z];
                     if(matrix)REPAO(poly[poly_cur])*=*matrix;
                     Box b; b.from(poly[poly_cur].data(), poly[poly_cur].elms());
                     if(Cuts(b, box))
                     {
                        REPD(p, clip_planes)
                        {
                           ClipPoly(poly[poly_cur], clip_plane[p], poly[poly_cur^1]); poly_cur^=1;
                           if(poly[poly_cur].elms()<=2)goto mshb_tri_done;
                        }
                        Swap(poly[poly_cur], polys.New());
                     }
                  mshb_tri_done:;
                     poly[0].clear();
                     poly[1].clear();
                  }
                  REPA(part.base.quad) // QUAD
                  {
                     VecI4 q=part.base.quad.ind(i);
                     poly[poly_cur].New()=pos[q.x];
                     poly[poly_cur].New()=pos[q.y];
                     poly[poly_cur].New()=pos[q.z];
                     poly[poly_cur].New()=pos[q.w];
                     if(matrix)REPAO(poly[poly_cur])*=*matrix;
                     Box b; b.from(poly[poly_cur].data(), poly[poly_cur].elms());
                     if(Cuts(b, box))
                     {
                        REPD(p, clip_planes)
                        {
                           ClipPoly(poly[poly_cur], clip_plane[p], poly[poly_cur^1]); poly_cur^=1;
                           if(poly[poly_cur].elms()<=2)goto mshb_quad_done;
                        }
                        Swap(poly[poly_cur], polys.New());
                     }
                  mshb_quad_done:;
                     poly[0].clear();
                     poly[1].clear();
                  }
               }else
               if(part.render.is()) // MSHR
               {
                  Int vtx_pos=part.render.vtxOfs(VTX_POS);
                  if( vtx_pos>=0)
                  if(C Byte *vtx=part.render.vtxLockRead())
                  {
                     vtx+=vtx_pos;
                     if(CPtr ind=part.render.indLockRead())
                     {
                        if(part.render._ib.bit16()) // 16 BIT
                        {
                           VecUS *tri=(VecUS*)ind;
                           REP(part.render.tris())
                           {
                              VecUS t=tri[i];
                              poly[poly_cur].New()=*(Vec*)(vtx+t.x*part.render.vtxSize());
                              poly[poly_cur].New()=*(Vec*)(vtx+t.y*part.render.vtxSize());
                              poly[poly_cur].New()=*(Vec*)(vtx+t.z*part.render.vtxSize());
                              if(matrix)REPAO(poly[poly_cur])*=*matrix;
                              Box b; b.from(poly[poly_cur].data(), poly[poly_cur].elms());
                              if(Cuts(b, box))
                              {
                                 REPD(p, clip_planes)
                                 {
                                    ClipPoly(poly[poly_cur], clip_plane[p], poly[poly_cur^1]); poly_cur^=1;
                                    if(poly[poly_cur].elms()<=2)goto mshr_tri16_done;
                                 }
                                 Swap(poly[poly_cur], polys.New());
                              }
                           mshr_tri16_done:;
                              poly[0].clear();
                              poly[1].clear();
                           }
                        }
                        else // 32 BIT
                        {
                           VecI *tri=(VecI*)ind;
                           REP(part.render.tris())
                           {
                              VecI t=tri[i];
                              poly[poly_cur].New()=*(Vec*)(vtx+t.x*part.render.vtxSize());
                              poly[poly_cur].New()=*(Vec*)(vtx+t.y*part.render.vtxSize());
                              poly[poly_cur].New()=*(Vec*)(vtx+t.z*part.render.vtxSize());
                              if(matrix)REPAO(poly[poly_cur])*=*matrix;
                              Box b; b.from(poly[poly_cur].data(), poly[poly_cur].elms());
                              if(Cuts(b, box))
                              {
                                 REPD(p, clip_planes)
                                 {
                                    ClipPoly(poly[poly_cur], clip_plane[p], poly[poly_cur^1]); poly_cur^=1;
                                    if(poly[poly_cur].elms()<=2)goto mshr_tri32_done;
                                 }
                                 Swap(poly[poly_cur], polys.New());
                              }
                           mshr_tri32_done:;
                              poly[0].clear();
                              poly[1].clear();
                           }
                        }
                        part.render.indUnlock();
                     }
                     part.render.vtxUnlock();
                  }
               }
            }
         }

         // create MeshBase from 'polys'
         if(polys.elms())
         {
            MeshBase mshb; Triangulate(polys, mshb, EPS, true);
            polys.clear();
            if(mshb.vtxs() && mshb.faces())
            {
               MeshLod &dest_lod=(temp.parts.elms() ? temp.newLod() : temp);
               dest_lod. copyParams(src_lod);
               dest_lod.scaleParams(scale  );
               Swap(mshb, dest_lod.parts.New().base);
            }
         }
      }
      temp.setBox();
      temp.lod_center=src.lod_center; if(matrix)temp.lod_center*=*matrix;
   }

finished:;
   Swap(dest, temp);
}
/******************************************************************************/
void MeshOverlay::zero()
{
  _lod_center.zero();
  _ext       .zero();
  _matrix    .identity();
  _shader=null;
}
MeshOverlay::MeshOverlay() {zero();}
MeshOverlay& MeshOverlay::del()
{
  _lods    .del  ();
  _material.clear();
   zero(); return T;
}
/******************************************************************************/
Bool MeshOverlay::createStatic(C Mesh &mesh, C MaterialPtr &material, C Matrix &overlay_matrix, C Matrix *mesh_matrix)
{
   del();

   if(mesh.is())
   {
     _matrix=overlay_matrix;
      Memc<Plane> planes;
      Box         box   ;  SetBoxAndPlanes(box, planes, _matrix);
      Mesh        temp  ; ClipMeshToPlanes(mesh, mesh_matrix, temp, planes.data(), planes.elms(), box);

      if(temp.is())
      {
         T._lod_center= temp.lod_center;
         T._ext       = temp.ext;
         T._lods.setNum(temp.lods());
         FREPD(l, temp.lods()) // order is important
         {
            MeshBase &mshb=temp.lod(l).parts[0].base;
         #if 0
            mshb.setNormals(); REPA(mshb.vtx)mshb.vtx.pos[i]+=mshb.vtx.nrm[i]*0.005f; // surface epsilon
         #endif
         #if 0
            mshb.include(VTX_TEX0); REPA(mshb.vtx)mshb.vtx.tex0[i]=_matrix.convert(mshb.vtx.pos[i]); // maybe it will require changing axis length when restored
         #endif
           _lods[l].dist2=temp.lod(l).dist2;
           _lods[l].mshr.create(mshb);
         }
         T.material(material);
         return true;
      }
   }
   return false;
}
Bool MeshOverlay::createAnimated(C Mesh &mesh, C MaterialPtr &material, C Matrix &overlay_matrix, C Matrix *mesh_matrix)
{
   del();

   if(mesh.is())
   {
      if(!mesh_matrix)_matrix=overlay_matrix;else
      {
         mesh_matrix->inverse(_matrix);
         overlay_matrix.mul(_matrix, _matrix);
      }
      Memc<Plane> planes;
      Box         box   ;  SetBoxAndPlanes(box, planes, _matrix);
      Mesh        temp  ; ClipMeshToPlanes(mesh, null, temp, planes.data(), planes.elms(), box);

      if(temp.is())
      {
         T._lod_center= temp.lod_center;
         T._ext       = temp.ext;
         T._lods.setNum(temp.lods());
         FREPD(l, temp.lods()) // order is important
         {
            MeshBase &mshb=temp.lod(l).parts[0].base;
         #if 0
            mshb.setNormals(); REPA(mshb.vtx)mshb.vtx.pos[i]+=mshb.vtx.nrm[i]*0.005f; // surface epsilon
         #endif
         #if 0
            mshb.include(VTX_TEX0); REPA(mshb.vtx)mshb.vtx.tex0[i]=_matrix.convert(mshb.vtx.pos[i]); // maybe it will require changing axis length when restored
         #endif
           _lods[l].dist2=temp.lod(l).dist2;
           _lods[l].mshr.create(mshb);
         }
         T.material(material);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
void MeshOverlay::setShader()
{
  _shader=null;
   if(_lods.elms() && _material && _material->base_0)_shader=ShaderFiles("Overlay")->get(TechNameOverlay(false, _material->base_1!=null));
}
MeshOverlay& MeshOverlay::material(C MaterialPtr &material)
{
   T._material=material;
   setShader();
   return T;
}
#pragma pack(push, 4)
struct GOverlayClass
{
   Vec4   param;
   Matrix mtrx ;
};
#pragma pack(pop)
void MeshOverlay::setParams(Flt alpha)C // this is called only inside 'draw' methods, so '_material' is assumed to be != null
{
   GOverlayClass overlay_params;
   overlay_params.param.set(0.8f, alpha, 0, 0);
   overlay_params.mtrx=_matrix;
   Sh.h_OverlayParams->set(overlay_params);
  _material          ->setBlend();
   D.depth(true);
   D.cull (true);
 //D.alpha(ALPHA_BLEND_FACTOR); not needed because ALPHA_BLEND_FACTOR is used everywhere in RM_OVERLAY
 /*D.depthWrite(false); not needed because false is used everywhere in RM_OVERLAY*/ Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
}
/******************************************************************************/
C MeshRender& MeshOverlay::getDrawLod(C Matrix &matrix)C
{
   if(_lods.elms()>1)
   {
      Flt dist2=GetLodDist2(_lod_center, matrix);
      REPA(_lods){C Lod &test=_lods[i]; if(dist2>=test.dist2)return test.mshr;}
   }
   return _lods[0].mshr;
}
C MeshRender& MeshOverlay::getDrawLod(C MatrixM &matrix)C
{
   if(_lods.elms()>1)
   {
      Flt dist2=GetLodDist2(_lod_center, matrix);
      REPA(_lods){C Lod &test=_lods[i]; if(dist2>=test.dist2)return test.mshr;}
   }
   return _lods[0].mshr;
}
/******************************************************************************/
void MeshOverlay::draw(Flt alpha)C
{
   if(_shader)
   {
      setParams(alpha);
     _shader->begin(); C MeshRender &mshr=getDrawLod(ObjMatrix); mshr.set().drawFull();
      ShaderEnd();
   }
}
void MeshOverlay::draw(C MatrixM &matrix, Flt alpha)C
{
   if(_shader)
   {
      SetOneMatrix(matrix);
      setParams(alpha);
     _shader->begin(); C MeshRender &mshr=getDrawLod(matrix); mshr.set().drawFull();
      ShaderEnd();
   }
}
void MeshOverlay::draw(C AnimatedSkeleton &anim_skel, Flt alpha)C
{
   if(_shader)
   {
      anim_skel.setMatrix();
      setParams(alpha);
     _shader->begin(); C MeshRender &mshr=getDrawLod(anim_skel.matrix()); mshr.set(); mshr.draw();
      ShaderEnd();
   }
}
/******************************************************************************/
Bool MeshOverlay::save(File &f, CChar *path)C
{
   f.putMulti(Byte(0), Int(_lods.elms())); // version
   if(_lods.elms())
   {
      f.putMulti(_lod_center, _ext, _matrix);
      f.putAsset(_material.id());
      FREPA(_lods)
      {
         f<<_lods[i].dist2;
        if(!_lods[i].mshr.saveData(f))return false;
      }
   }
   return f.ok();
}
Bool MeshOverlay::load(File &f, CChar *path)
{
   del(); switch(f.decUIntV()) // version
   {
      case 0:
      {
            _lods.setNum(f.getInt());
         if(_lods.elms())
         {
            f.getMulti(_lod_center, _ext, _matrix);
           _material.require(f.getAssetID(), path);
            FREPA(_lods)
            {
                f>>_lods[i].dist2;
               if(!_lods[i].mshr.loadData(f))goto error;
            }
            setShader();
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
