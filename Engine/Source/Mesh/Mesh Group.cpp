/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MeshGroup& MeshGroup::include (UInt flag) {REPAO(meshes).include (flag); return T;}
MeshGroup& MeshGroup::exclude (UInt flag) {REPAO(meshes).exclude (flag); return T;}
MeshGroup& MeshGroup::keepOnly(UInt flag) {REPAO(meshes).keepOnly(flag); return T;}
/******************************************************************************/
void MeshGroup::zero     () {ext.zero();}
     MeshGroup::MeshGroup() {zero();}

MeshGroup& MeshGroup::delBase  () {REPAO(meshes).delBase  (); return T;}
MeshGroup& MeshGroup::delRender() {REPAO(meshes).delRender(); return T;}
MeshGroup& MeshGroup::del      ()
{
   meshes.del();
   zero(); return T;
}
/******************************************************************************/
MeshGroup& MeshGroup::create(C Mesh &src, C Boxes &boxes)
{
   struct MeshPartEx : MeshPart
   {
      Int lod_index, box_index;
   };
   Memt<MeshPartEx>    mesh_parts;
   Memt<MeshBaseIndex> mesh_splits;
   MemtN<Int, 256>     box_parts;

   // split 'src'
   FREPD(l, src.lods()) // add in order
   {
    C MeshLod &lod=src.lod(l); FREPAD(p, lod) // add in order
      {
       C MeshPart &part=lod.parts[p]; part.base.split(mesh_splits, boxes); FREPAD(s, mesh_splits) // add in order
         {
            MeshBaseIndex &mesh_split=mesh_splits[s];
            MeshPartEx    &mesh_part =mesh_parts.New();
                 Swap(mesh_part.base, SCAST(MeshBase, mesh_split));
                      mesh_part.copyParams(part, true);
                      mesh_part.lod_index=l;
            box_parts(mesh_part.box_index=mesh_split.index)=1; // set that this box part has some data
         }
      }
   }

   // check which box_parts have anything
   Int bps=0; FREPA(box_parts) // process in order
   {
      Int &box_part=box_parts[i]; if(box_part)box_part=bps++; // if box part is valid, then set its target index as current number of bps, and inc that by 1
   }

   // create mesh group
   MeshGroup temp, &dest=(T.meshes.contains(&src) ? temp : T); // if this contains 'src' then operate on 'temp' first
   dest.meshes.setNum(bps); REPAD(s, dest.meshes)
   {
      Mesh &mesh=dest.meshes[s].setLods(src.lods()); mesh.copyParams(src); // create split meshes to always have the same amount of LOD's
      REPD(l, mesh.lods()) // process all LOD's in this split mesh
      {
         MeshLod &lod=mesh.lod(l); lod.copyParams(src.lod(l));
         Int parts=0; // how many parts in this LOD
         REPA(mesh_parts)
         {
            MeshPartEx &mesh_part=mesh_parts[i]; if(mesh_part.lod_index==l && box_parts[mesh_part.box_index]==s)parts++; // if mesh_part should be assigned to this lod
         }
         lod.parts.setNum(parts); // always set LOD parts in case we're operating on MeshGroup that already had some data, and this will clear it
         if(parts)
         {
            parts=0;
            FREPA(mesh_parts) // add in order
            {
               MeshPartEx &mesh_part=mesh_parts[i]; if(mesh_part.lod_index==l && box_parts[mesh_part.box_index]==s)Swap(SCAST(MeshPart, mesh_part), lod.parts[parts++]);
            }
         }
      }
   }
   if(&dest==&temp)Swap(temp, T);
   setBox(true); return T;
}
MeshGroup& MeshGroup::create(C Mesh      &src, C VecI &cells   ) {return create(src, Boxes(src.ext, cells));}
MeshGroup& MeshGroup::create(C MeshGroup &src,   UInt  flag_and)
{
   if(this==&src)keepOnly(flag_and);else
   {
      meshes.setNum(src.meshes.elms()); FREPA(T)meshes[i].create(src.meshes[i], flag_and);
      copyParams(src);
   }
   return T;
}
void MeshGroup::copyParams(C MeshGroup &src)
{
   if(this!=&src)
   {
      ext=src.ext;
   }
}
/******************************************************************************/
// GET
/******************************************************************************/
UInt MeshGroup::flag     ()C {UInt flag=0; REPA(T)flag|=meshes[i].flag     (); return flag;}
Int  MeshGroup::vtxs     ()C {Int  n   =0; REPA(T)n   +=meshes[i].vtxs     (); return n   ;}
Int  MeshGroup::edges    ()C {Int  n   =0; REPA(T)n   +=meshes[i].edges    (); return n   ;}
Int  MeshGroup::tris     ()C {Int  n   =0; REPA(T)n   +=meshes[i].tris     (); return n   ;}
Int  MeshGroup::quads    ()C {Int  n   =0; REPA(T)n   +=meshes[i].quads    (); return n   ;}
Int  MeshGroup::faces    ()C {Int  n   =0; REPA(T)n   +=meshes[i].faces    (); return n   ;}
Int  MeshGroup::trisTotal()C {Int  n   =0; REPA(T)n   +=meshes[i].trisTotal(); return n   ;}
/******************************************************************************/
// JOIN
/******************************************************************************/
MeshGroup& MeshGroup::join(Int i0, Int i1)
{
   if(i0!=i1 && InRange(i0, T) && InRange(i1, T))
   {
      if(i1<i0)Swap(i0, i1);
      meshes[i0]+=meshes[i1];
      meshes.remove(i1, true);
   }
   return T;
}
MeshGroup& MeshGroup::joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag, Flt weld_pos_eps) {REPAO(meshes).joinAll(test_material, test_draw_group, test_name, test_vtx_flag, weld_pos_eps); return T;}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
MeshGroup& MeshGroup::texMap   (  Flt     scale , Byte tex_index) {REPAO(meshes).texMap   (scale , tex_index); return T;}
MeshGroup& MeshGroup::texMap   (C Matrix &matrix, Byte tex_index) {REPAO(meshes).texMap   (matrix, tex_index); return T;}
MeshGroup& MeshGroup::texMap   (C Plane  &plane , Byte tex_index) {REPAO(meshes).texMap   (plane , tex_index); return T;}
MeshGroup& MeshGroup::texMap   (C Ball   &ball  , Byte tex_index) {REPAO(meshes).texMap   (ball  , tex_index); return T;}
MeshGroup& MeshGroup::texMap   (C Tube   &tube  , Byte tex_index) {REPAO(meshes).texMap   (tube  , tex_index); return T;}
MeshGroup& MeshGroup::texMove  (C Vec2   &move  , Byte tex_index) {REPAO(meshes).texMove  (move  , tex_index); return T;}
MeshGroup& MeshGroup::texScale (C Vec2   &scale , Byte tex_index) {REPAO(meshes).texScale (scale , tex_index); return T;}
MeshGroup& MeshGroup::texRotate(  Flt     angle , Byte tex_index) {REPAO(meshes).texRotate(angle , tex_index); return T;}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
MeshGroup& MeshGroup::move     (              C Vec &move) {            ext+=move; REPAO(meshes).move     (       move); return T;}
MeshGroup& MeshGroup::scale    (C Vec &scale             ) {ext*=scale;            REPAO(meshes).scale    (scale      ); return T;}
MeshGroup& MeshGroup::scaleMove(C Vec &scale, C Vec &move) {ext*=scale; ext+=move; REPAO(meshes).scaleMove(scale, move); return T;}
/******************************************************************************/
// SET
/******************************************************************************/
MeshGroup& MeshGroup::setFaceNormals(                                   ) {REPAO(meshes).setFaceNormals(                      ); return T;}
MeshGroup& MeshGroup::setNormals    (                                   ) {REPAO(meshes).setNormals    (                      ); return T;}
MeshGroup& MeshGroup::setVtxDup     (UInt flag, Flt pos_eps, Flt nrm_cos) {REPAO(meshes).setVtxDup     (flag, pos_eps, nrm_cos); return T;}
MeshGroup& MeshGroup::setRender     (                                   ) {REPAO(meshes).setRender     (                      ); return T;}
MeshGroup& MeshGroup::setShader     (                                   ) {REPAO(meshes).setShader     (                      ); return T;}
MeshGroup& MeshGroup::material      (C MaterialPtr &material            ) {REPAO(meshes).material      (material              ); return T;}
Bool       MeshGroup::setBox        (Bool           set_mesh_boxes      )
{
   if(set_mesh_boxes)REPAO(meshes).setBox();
   Bool found=false; Box box; REPA(T)
   {
    C Mesh &mesh=T.meshes[i]; if(mesh.vtxs()){if(found)box|=mesh.ext;else{found=true; box=mesh.ext;}}
   }
   if(found)T.ext=box;else T.ext.zero(); return found;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
MeshGroup& MeshGroup::weldVtx2D    (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REPAO(meshes).weldVtx2D(flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
MeshGroup& MeshGroup::weldVtx      (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REPAO(meshes).weldVtx  (flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
MeshGroup& MeshGroup::weldVtxValues(UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps)
{
   struct VtxDupIndex : VtxDupNrm
   {
      VecI index;
      Int  count;
   };
   flag&=T.flag();

   // create vertex array
   Int vtx_num=0; REPAD(m, T){Mesh &mesh=T.meshes[m]; REPAD(p, mesh.parts)vtx_num+=mesh.parts[p].base.vtxs();} // do not use T.vtxs() as it includes MeshRender
   Memc<VtxDupIndex> vtxs; vtxs.setNum(vtx_num); vtx_num=0;
   REPAD(m, T)
   {
      Mesh &mesh=T.meshes[m];
      REPAD(p, mesh.parts)
      {
         MeshBase &mshb=mesh.parts[p].base;
       C Vec      *nrm =mshb.vtx.nrm();
         REPA(mshb.vtx)
         {
            VtxDupIndex &vtx=vtxs[vtx_num++];
            vtx.pos  =mshb.vtx.pos(i);
            vtx.nrm  =(nrm ? nrm[i] : VecZero);
            vtx.index.set(i, p, m);
            vtx.count=0;
         }
      }
   }

   // get vtx dup
   SetVtxDup(SCAST(Memc<VtxDupNrm>, vtxs), ext, pos_eps, nrm_cos);

   // weld
   REPA(vtxs)
   {
      VtxDupIndex &vn=vtxs[i]; vtxs[vn.dup].count++; if(vn.dup!=i)
      {
         VtxDupIndex &vd=vtxs[vn.dup];
         if(flag&VTX_POS )meshes[vn.index.z].parts[vn.index.y].base.vtx.pos (vn.index.x) =meshes[vd.index.z].parts[vd.index.y].base.vtx.pos (vd.index.x);
         if(flag&VTX_NRM )meshes[vd.index.z].parts[vd.index.y].base.vtx.nrm (vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm (vn.index.x);
         if(flag&VTX_TAN )meshes[vd.index.z].parts[vd.index.y].base.vtx.tan (vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.tan (vn.index.x);
         if(flag&VTX_BIN )meshes[vd.index.z].parts[vd.index.y].base.vtx.bin (vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.bin (vn.index.x);
         if(flag&VTX_HLP )meshes[vd.index.z].parts[vd.index.y].base.vtx.hlp (vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp (vn.index.x);
         if(flag&VTX_TEX0)meshes[vd.index.z].parts[vd.index.y].base.vtx.tex0(vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x);
         if(flag&VTX_TEX1)meshes[vd.index.z].parts[vd.index.y].base.vtx.tex1(vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x);
         if(flag&VTX_TEX2)meshes[vd.index.z].parts[vd.index.y].base.vtx.tex2(vd.index.x)+=meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x);
      }
   }
   if(flag&VTX_NRM ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm (vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.nrm (vd.index.x);}}
   if(flag&VTX_TAN ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.tan (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.tan (vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.tan (vd.index.x);}}
   if(flag&VTX_BIN ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.bin (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.bin (vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.bin (vd.index.x);}}
   if(flag&VTX_HLP ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp (vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.hlp (vd.index.x);}}
   if(flag&VTX_TEX0){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.tex0(vd.index.x);}}
   if(flag&VTX_TEX1){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.tex1(vd.index.x);}}
   if(flag&VTX_TEX2){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x)=meshes[vd.index.z].parts[vd.index.y].base.vtx.tex2(vd.index.x);}}

   // remove degenerate faces
   if(flag&VTX_POS && remove_degenerate_faces_eps>=0)removeDegenerateFaces(remove_degenerate_faces_eps);

   return T;
}
MeshGroup& MeshGroup::freeOpenGLESData() {REPAO(meshes).freeOpenGLESData(); return T;}
/******************************************************************************/
// CONVERT
/******************************************************************************/
MeshGroup& MeshGroup::triToQuad(Flt cos) {REPAO(meshes).triToQuad(cos); return T;}
MeshGroup& MeshGroup::quadToTri(Flt cos) {REPAO(meshes).quadToTri(cos); return T;}
/******************************************************************************/
// FIX
/******************************************************************************/
MeshGroup& MeshGroup::fixTexWrapping(Byte tex_index) {REPAO(meshes).fixTexWrapping(tex_index); return T;}
MeshGroup& MeshGroup::fixTexOffset  (Byte tex_index) {REPAO(meshes).fixTexOffset  (tex_index); return T;}
/******************************************************************************/
// ADD/REMOVE
/******************************************************************************/
MeshGroup& MeshGroup::remove(Int i, Bool set_box)
{
   if(InRange(i, T))
   {
     meshes.remove(i, true);
     if(set_box)setBox(false);
   }
   return T;
}
/******************************************************************************/
// OPTIMIZE
/******************************************************************************/
MeshGroup& MeshGroup::removeDegenerateFaces(Flt eps                                                  ) {REPAO(meshes).removeDegenerateFaces(eps                                     ); return T;}
MeshGroup& MeshGroup::weldCoplanarFaces    (Flt cos_face, Flt cos_vtx, Bool safe, Flt max_face_length) {REPAO(meshes).weldCoplanarFaces    (cos_face, cos_vtx, safe, max_face_length); return T;}
MeshGroup& MeshGroup::sortByMaterials      (                                                         ) {REPAO(meshes).sortByMaterials      (                                        ); return T;}

MeshGroup& MeshGroup::simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps, MeshGroup *dest, Bool *stop)
{
   if(!dest)dest=this;
   if( dest!=this)
   {
      dest->copyParams(T);
      dest->meshes.setNum(meshes.elms());
   }
   REPAO(meshes).simplify(intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, keep_border, mode, pos_eps, &dest->meshes[i], stop);
   return *dest;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void MeshGroup::draw(C MatrixM &matrix)C
{
   switch(meshes.elms())
   {
      case 0 : return;                          // if           no  meshes then return
      case 1 : break ;                          // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
      default: if(!Frustum(ext, matrix))return; // if more than one meshes then check         the MeshGroup.ext
   }
   FREPA(meshes)
   {
    C Mesh &mesh=meshes[i]; if(Frustum(mesh, matrix))mesh.draw(matrix);
   }
}
void MeshGroup::drawShadow(C MatrixM &matrix)C
{
   switch(meshes.elms())
   {
      case 0 : return;                          // if           no  meshes then return
      case 1 : break ;                          // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
      default: if(!Frustum(ext, matrix))return; // if more than one meshes then check         the MeshGroup.ext
   }
   FREPA(meshes)
   {
    C Mesh &mesh=meshes[i]; if(Frustum(mesh, matrix))mesh.drawShadow(matrix);
   }
}
/******************************************************************************/
}
/******************************************************************************/
