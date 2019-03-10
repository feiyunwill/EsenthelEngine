/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// MANAGE
/******************************************************************************/
void MeshLod::zero() {dist2=0;}
MeshLod::MeshLod() {zero();}

MeshLod& MeshLod::delBase  () {REPAO(parts).delBase  (); return T;}
MeshLod& MeshLod::delRender() {REPAO(parts).delRender(); return T;}
MeshLod& MeshLod::del      ()
{
   parts.del();
   zero(); return T;
}
MeshLod& MeshLod::create(Int num)
{
   del();
   parts.setNum(num);
   return T;
}
MeshLod& MeshLod::create(C MeshLod &src, UInt flag_and)
{
   if(this==&src)keepOnly(flag_and);else
   {
      create(src.parts.elms()); copyParams(src); FREPAO(parts).create(src.parts[i], flag_and);
   }
   return T;
}
void MeshLod::copyParams(C MeshLod &src)
{
   if(this!=&src)
   {
      dist2=src.dist2;
   }
}
void MeshLod::scaleParams(Flt scale)
{
   dist2*=Sqr(scale);
}
MeshLod& MeshLod::include (UInt flag) {REPAO(parts).include (flag); return T;}
MeshLod& MeshLod::exclude (UInt flag) {REPAO(parts).exclude (flag); return T;}
MeshLod& MeshLod::keepOnly(UInt flag) {REPAO(parts).keepOnly(flag); return T;}
/******************************************************************************/
// GET
/******************************************************************************/
UInt MeshLod::flag     ()C {UInt flag =0; REPA(T)flag |=parts[i].flag     (); return flag ;}
UInt MeshLod::memUsage ()C {UInt size =0; REPA(T)size +=parts[i].memUsage (); return size ;}
Int  MeshLod::vtxs     ()C {Int  vtxs =0; REPA(T)vtxs +=parts[i].vtxs     (); return vtxs ;}
Int  MeshLod::edges    ()C {Int  edges=0; REPA(T)edges+=parts[i].edges    (); return edges;}
Int  MeshLod::tris     ()C {Int  tris =0; REPA(T)tris +=parts[i].tris     (); return tris ;}
Int  MeshLod::quads    ()C {Int  quads=0; REPA(T)quads+=parts[i].quads    (); return quads;}
Int  MeshLod::faces    ()C {Int  faces=0; REPA(T)faces+=parts[i].faces    (); return faces;}
Int  MeshLod::trisTotal()C {Int  tris =0; REPA(T)tris +=parts[i].trisTotal(); return tris ;}
Flt  MeshLod::area     (Vec *center)C
{
   if(center)center->zero();
   Flt area=0;
   REPA(T)
   {
      Vec c;
      Flt a=parts[i].area(center ? &c : null);
                 area  +=a;
      if(center)*center+=a*c;
   }
   if(center && area)*center/=area;
   return area;
}
Bool MeshLod::getBox(Box &box, Bool skip_hidden_parts)C
{
   Bool found=false; Box temp; REPA(T)
   {
    C MeshPart &part=parts[i];
      if(!skip_hidden_parts || !(part.part_flag&MSHP_HIDDEN))
         if(part.getBox(temp))
      {
         if(!found){found=true; box=temp;}else box|=temp;
      }
   }
   if(!found)box.zero(); return found;
}
/******************************************************************************/
// have to use SB functions for special case of -0
Flt MeshLod::dist()C
{
   Flt    dist; if(NegativeSB(dist2)){dist=SqrtFast(-dist2); CHSSB(dist);}else dist=SqrtFast(dist2);
   return dist;
}
MeshLod& MeshLod::dist(Flt dist)
{
   dist2=Sqr(dist); if(NegativeSB(dist))CHSSB(dist2);
   return T;
}
/******************************************************************************/
Bool MeshLod::hasDrawGroup    ( Int draw_group_index)C {REPA(parts)if(    parts[i].drawGroup()==draw_group_index)return true; return false;}
Bool MeshLod::hasDrawGroupMask(UInt draw_group_mask )C {REPA(parts)if((1<<parts[i].drawGroup())&draw_group_mask )return true; return false;}
/******************************************************************************/
// SET
/******************************************************************************/
MeshLod& MeshLod::setEdgeNormals(Bool flag                          ) {REPAO(parts).base.setEdgeNormals(flag                  ); return T;}
MeshLod& MeshLod::setFaceNormals(                                   ) {REPAO(parts).base.setFaceNormals(                      ); return T;}
MeshLod& MeshLod::setNormals2D  (Bool flag                          ) {REPAO(parts).base.setNormals2D  (flag                  ); return T;}
MeshLod& MeshLod::setNormals    (                                   ) {REPAO(parts).base.setNormals    (                      ); return T;}
MeshLod& MeshLod::setTangents   (                                   ) {REPAO(parts).base.setTangents   (                      ); return T;}
MeshLod& MeshLod::setBinormals  (                                   ) {REPAO(parts).base.setBinormals  (                      ); return T;}
MeshLod& MeshLod::setAutoTanBin (                                   ) {REPAO(parts).     setAutoTanBin (                      ); return T;}
MeshLod& MeshLod::setVtxDup2D   (UInt flag, Flt pos_eps, Flt nrm_cos) {REPAO(parts).base.setVtxDup2D   (flag, pos_eps, nrm_cos); return T;}
MeshLod& MeshLod::setVtxDup     (UInt flag, Flt pos_eps, Flt nrm_cos) {REPAO(parts).base.setVtxDup     (flag, pos_eps, nrm_cos); return T;}
MeshLod& MeshLod::setAdjacencies(Bool faces, Bool edges             ) {REPAO(parts).base.setAdjacencies(faces, edges          ); return T;}

MeshLod& MeshLod::setVtxColorAlphaAsTesselationIntensity(Bool tesselate_edges)
{
   Box box; if(!getBox(box))return T;

   struct VtxDupIndex : VtxDup
   {
      VecI2 index;
      Bool  freeze;
   };

   // create vertex array
   Int vtx_num=0; REPA(parts)vtx_num+=parts[i].base.vtxs(); // do not use T.vtxs() as it includes 'MeshRender'
   Memc<VtxDupIndex> vtxs; vtxs.setNum(vtx_num); vtx_num=0;
   REPAD(p, T)
   {
      MeshBase &mshb=parts[p].base;
      REPA(mshb.vtx)
      {
         VtxDupIndex &vtx=vtxs[vtx_num++];
         vtx.pos=mshb.vtx.pos(i);
         vtx.index.set(i, p);
         vtx.freeze=false;
      }
   }

   // get vtx dup
   SetVtxDup(SCAST(Memc<VtxDup>, vtxs), box, EPSD);

   // set freeze
   REPA(vtxs)
   {
      VtxDupIndex &vi=vtxs[i]; if(vi.dup!=i)
      {
         VtxDupIndex &vd=vtxs[vi.dup];
         if(!parts[vi.index.y].base.vtx.nrm()
         || !parts[vd.index.y].base.vtx.nrm())vtxs[vi.dup].freeze=true;else
         {
          C Vec &nrm1=parts[vi.index.y].base.vtx.nrm(vi.index.x),
                &nrm2=parts[vd.index.y].base.vtx.nrm(vd.index.x);
            if(Dot(nrm1, nrm2)<0.995f)vtxs[vi.dup].freeze=true;
         }
      }
   }

   // set color
   REPA(vtxs)
   {
      VtxDupIndex &vi=vtxs[i]; if(vtxs[vi.dup].freeze) // if the main should be frozen, then all duplicates also should be frozen
      {
         MeshBase &base=parts[vi.index.y].base;
         if(base.vtx.nrm())
         {
            if(!base.vtx.color()){base.include(VTX_COLOR); REPA(base.vtx)base.vtx.color(i)=WHITE;} // create vtx colors
            base.vtx.color(vi.index.x).a=0; // set alpha to zero
         }
      }
   }
   return T;
}
/******************************************************************************/
MeshLod& MeshLod::setRenderSS(                                      ) {REPAO(parts).setRenderEx(false   , false, 0 ); return T;}
MeshLod& MeshLod::setBase    (Bool only_if_empty                    ) {REPAO(parts).setBase    (only_if_empty      ); return T;}
MeshLod& MeshLod::setRender  (Bool optimize          , Int lod_index) {REPAO(parts).setRender  (optimize, lod_index); return T;}
MeshLod& MeshLod::setShader  (                         Int lod_index) {REPAO(parts).setShader  (          lod_index); return T;}
MeshLod& MeshLod::material   (C MaterialPtr &material, Int lod_index) {REPAO(parts).material   (material, lod_index); return T;}
/******************************************************************************/
// JOIN
/******************************************************************************/
MeshLod& MeshLod::join(Int i0, Int i1, Flt weld_pos_eps)
{
   if(i0!=i1 && InRange(i0, T) && InRange(i1, T))
   {
      MeshPart &part=parts[i0];
      part+=parts[i1];
      if(weld_pos_eps>=0)
      {
         Bool base=part.base.is();
         part.setBase().base.weldVtx(VTX_ALL, weld_pos_eps, EPS_COL_COS, -1); // there are no degenerate faces
         if(part.render.is())part.setRender();
         if(           !base)part.delBase();
      }
      parts.remove(i1, true);
   }
   return T;
}
MeshLod& MeshLod::joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag, Flt weld_pos_eps)
{
   if(parts.elms()>1) // if has some parts to merge
   {
      Bool base_is=false, render_is=false; REPA(T){if(parts[i].base.is())base_is=true; if(parts[i].render.is())render_is=true;} // detect what is present in the mesh
      if(test_material || test_draw_group || test_name || test_vtx_flag)
      {
         MemtN<  Int      , 256> similar;
         Memb <  MeshBase      > temp; // use 'Memb' because we're storing pointers to elms
         MemtN<C MeshBase*, 256> mesh_ptr;
         MemtN<  MeshPart ,  16> joined;

         FREPA(T) // iterate through all parts, preserve order
         {
            MeshPart &part=parts[i]; if(part.is())
            {
               Int     draw_group=part.drawGroup();
               UInt part_vtx_flag=(part.flag()&test_vtx_flag);

               similar.add(i); // add self before others, so order is preserved
               for(Int j=i+1; j<parts.elms(); j++) // find matching, preserve order
               {
                C MeshPart &test=parts[j]; if(test.is())
                  {
                     if(test_material   && !test.sameMaterials(part)    )continue;
                     if(test_draw_group &&  test.drawGroup()!=draw_group)continue;
                     if(test_name       && !Equal(test.name, part.name) )continue;
                     if(test_vtx_flag   && (test.flag()&test_vtx_flag)!=part_vtx_flag)continue;
                     similar.add(j);
                  }
               }
               if(similar.elms()>1) // if detected any similar parts
               {
                  // list similar parts
                  FREPA(similar) // preserve order
                  {
                   C MeshPart &test=parts[similar[i]];
                     if(!(test.part_flag&MSHP_HIDDEN))FlagDisable(part.part_flag, MSHP_HIDDEN); // if at least one part is visible then set all as visible
                     if(test.render.is() && !test.base.is())mesh_ptr.add(&temp.New().create(test.render)); // MeshBase needs to be created
                     else                                   mesh_ptr.add(&                  test.base   ); // point to original MeshBase
                  }

                  part.base.create(mesh_ptr.data(), mesh_ptr.elms());
                  if(weld_pos_eps>=0)part.base.weldVtx(VTX_ALL, weld_pos_eps, EPS_COL_COS, -1); // there are no degenerate faces
                  if(render_is      )part.setRender();
                  if( !base_is      )part.base.del ();

                      temp.clear();
                  mesh_ptr.clear();
               }
               Swap(joined.New(), part);
               REPA(similar)parts[similar[i]].del(); // delete joined parts, so they won't be processed again
               similar.clear();
            }
         }
         parts.setNum(joined.elms()); REPA(parts)Swap(parts[i], joined[i]);
      }else
      {
         MeshPart &first=parts[0];
         REPA(T)if(!(parts[i].part_flag&MSHP_HIDDEN)){FlagDisable(first.part_flag, MSHP_HIDDEN); break;} // if at least one part is visible then set all as visible
         first.base.create(T); // create from all
         parts.setNum(1); // set only one part
         if(weld_pos_eps>=0)weldVtx(VTX_ALL, weld_pos_eps, EPS_COL_COS, -1); // there are no degenerate faces
         if(render_is      )setRender();
         if( !base_is      )delBase  ();
      }
   }
   return T;
}
/******************************************************************************/
MeshPart* MeshLod::splitVtxs(Int i, C MemPtr<Bool> &vtx_is)
{
   MeshPart *part=null; if(InRange(i, T) && vtx_is.elms())
   {
      MeshBase &base=parts[i].base; // !! don't use this later as creating New Part invalidates this reference !!
      Int       vtxs=CountIs(vtx_is);
      if(vtxs && vtxs!=base.vtxs())
      {
         part=&parts.NewAt(i+1);
         MeshBase &old=parts[i].base; old.splitVtxs(part->base, vtx_is);
         if(old.is())
         {
            part->copyParams(parts[i]);
         }else // 'old' may be empty for 'splitVtxs' because we had to select neighbor vertexes which could've resulted in entire mesh being copied
         {
            Swap(old, part->base); // bring back to the old
            parts.removeData(part, true); part=null; // remove the newly created part
         }
      }
   }
   return part;
}
MeshPart* MeshLod::splitFaces(Int i, C MemPtr<Bool> &edge_is, C MemPtr<Bool> &tri_is, C MemPtr<Bool> &quad_is)
{
   MeshPart *part=null; if(InRange(i, T) && (edge_is.elms() || tri_is.elms() || quad_is.elms()))
   {
      MeshBase &base =parts[i].base; // !! don't use this later as creating New Part invalidates this reference !!
      Int       edges=CountIs(edge_is),
                 tris=CountIs( tri_is),
                quads=CountIs(quad_is);
      if((edges!=           0 || tris!=          0 || quads!=0           )  // if want to copy something
      && (edges!=base.edges() || tris!=base.tris() || quads!=base.quads())) // and not everything
      {
         part=&parts.NewAt(i+1); parts[i].base.splitFaces(part->base, edge_is, tri_is, quad_is); // !! can't access 'base' anymore because 'New' could've changed its mem address !!
         part->copyParams(parts[i]);
      }
   }
   return part;
}
MeshPart* MeshLod::splitBone(Int i, Int bone, C Skeleton *skeleton)
{
   MeshPart *part=null; if(InRange(i, T) && parts[i].base.vtx.matrix())
   {
      part=&parts.NewAt(i+1); parts[i].base.splitBone(part->base, bone);
      if(part->is())
      {
         part->copyParams(parts[i]); if(skeleton && InRange(bone, skeleton->bones))Set(part->name, skeleton->bones[bone].name);
      }else
      {
         parts.removeData(part, true); part=null;
      }
   }
   return part;
}
MeshPart* MeshLod::splitVtxs(Int i, C MemPtr<Int> &vtxs)
{
   if(vtxs.elms())if(MeshPart *part=parts.addr(i))
   {
      Memt<Bool> vtx_is; CreateIs(vtx_is, vtxs, part->base.vtxs()); return splitVtxs(i, vtx_is);
   }
   return null;
}
MeshPart* MeshLod::splitFaces(Int i, C MemPtr<Int> &faces)
{
   if(faces.elms())if(MeshPart *part=parts.addr(i))
   {
      Memt<Bool> tri_is, quad_is; CreateFaceIs(tri_is, quad_is, faces, part->base.tris(), part->base.quads());
      return splitFaces(i, null, tri_is, quad_is);
   }
   return null;
}
MeshPart* MeshLod::splitFaces(Int i, C MemPtr<Int> &edges, C MemPtr<Int> &tris, C MemPtr<Int> &quads)
{
   if(edges.elms() || tris.elms() || quads.elms())if(MeshPart *part=parts.addr(i))
   {
      Memt<Bool> edge_is; CreateIs(edge_is, edges, part->base.edges());
      Memt<Bool>  tri_is; CreateIs( tri_is,  tris, part->base. tris());
      Memt<Bool> quad_is; CreateIs(quad_is, quads, part->base.quads());
      return splitFaces(i, edge_is, tri_is, quad_is);
   }
   return null;
}
/*
MeshLod& splitFloor(Flt y, Int i=-1                                  ); // split i-th(-1=all) part faces if face.normal.y>=y
MeshLod& MeshLod::splitFloor(Flt y, Int i)
{
   REPD(m, parts())if((i==-1 || i==m) && base(m).is()) // order is important
   {
      MeshBase &mshb=base(m);
      if((mshb.tris () && !mshb.tri .nrm())
      || (mshb.quads() && !mshb.quad.nrm()))mshb.setFaceNormals();
      Vec  *nrm;
      Bool * tri_is=Alloc<Bool>(mshb.tris ()); Int  tris=0; nrm=mshb.tri .nrm(); REPA(mshb.tri )if( tri_is[i]=(nrm[i].y>=y)) tris++;
      Bool *quad_is=Alloc<Bool>(mshb.quads()); Int quads=0; nrm=mshb.quad.nrm(); REPA(mshb.quad)if(quad_is[i]=(nrm[i].y>=y))quads++;
      if(tris || quads)
      {
         if(tris==mshb.tris() && quads==mshb.quads())part(m).part_flag|=MSHP_FLOOR;
         else                   split(m,null,tri_is,quad_is).part_flag|=MSHP_FLOOR;
      }
      Free( tri_is);
      Free(quad_is);
   }
   return T;
}
/******************************************************************************/
// TEXTURE TRANSFORM
/******************************************************************************/
MeshLod& MeshLod::texMove  (C Vec2 &move , Byte tex_index) {REPAO(parts).texMove  (move , tex_index); return T;}
MeshLod& MeshLod::texScale (C Vec2 &scale, Byte tex_index) {REPAO(parts).texScale (scale, tex_index); return T;}
MeshLod& MeshLod::texRotate(  Flt   angle, Byte tex_index) {REPAO(parts).texRotate(angle, tex_index); return T;}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
MeshLod& MeshLod::texMap(  Flt     scale , Byte tex_index) {REPAO(parts).base.texMap(scale , tex_index); return T;}
MeshLod& MeshLod::texMap(C Matrix &matrix, Byte tex_index) {REPAO(parts).base.texMap(matrix, tex_index); return T;}
MeshLod& MeshLod::texMap(C Plane  &plane , Byte tex_index) {REPAO(parts).base.texMap(plane , tex_index); return T;}
MeshLod& MeshLod::texMap(C Ball   &ball  , Byte tex_index) {REPAO(parts).base.texMap(ball  , tex_index); return T;}
MeshLod& MeshLod::texMap(C Tube   &tube  , Byte tex_index) {REPAO(parts).base.texMap(tube  , tex_index); return T;}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
MeshLod& MeshLod::move         (              C Vec &move       ) {                    REPAO(parts).     move         (       move);                                            return T;}
MeshLod& MeshLod::scale        (C Vec &scale                    ) {                    REPAO(parts).     scale        (scale      ); scaleParams(      Abs(scale).max     ());  return T;}
MeshLod& MeshLod::scaleMove    (C Vec &scale, C Vec &move       ) {                    REPAO(parts).     scaleMove    (scale, move); scaleParams(      Abs(scale).max     ());  return T;}
MeshLod& MeshLod::scaleMoveBase(C Vec &scale, C Vec &move       ) {                    REPAO(parts).     scaleMoveBase(scale, move); scaleParams(      Abs(scale).max     ());  return T;}
MeshLod& MeshLod::transform    (C Matrix3              &matrix  ) {                    REPAO(parts).     transform    (matrix     ); scaleParams(     matrix     .maxScale());  return T;}
MeshLod& MeshLod::transform    (C Matrix               &matrix  ) {                    REPAO(parts).     transform    (matrix     ); scaleParams(     matrix     .maxScale());  return T;}
MeshLod& MeshLod::animate      (C MemPtrN<Matrix, 256> &matrixes) {if(matrixes.elms()){REPAO(parts).     animate      (matrixes   ); scaleParams(     matrixes[0].maxScale());} return T;}
MeshLod& MeshLod::animate      (C AnimatedSkeleton     &skel    ) {                    REPAO(parts).     animate      (skel       ); scaleParams(skel.matrix  ( ).maxScale());  return T;}
MeshLod& MeshLod::mirrorX      (                                ) {                    REPAO(parts).base.mirrorX      (           );                                            return T;}
MeshLod& MeshLod::mirrorY      (                                ) {                    REPAO(parts).base.mirrorY      (           );                                            return T;}
MeshLod& MeshLod::mirrorZ      (                                ) {                    REPAO(parts).base.mirrorZ      (           );                                            return T;}
MeshLod& MeshLod::rightToLeft  (                                ) {                    REPAO(parts).base.rightToLeft  (           );                                            return T;}
MeshLod& MeshLod::reverse      (                                ) {                    REPAO(parts).base.reverse      (           );                                            return T;}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
MeshLod& MeshLod::weldVtx2D    (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REPA(T)parts[i].base.weldVtx2D(flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
MeshLod& MeshLod::weldVtx      (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REPA(T)parts[i].base.weldVtx  (flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
MeshLod& MeshLod::weldVtxValues(UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps)
{
   flag&=T.flag();
   if(flag&(VTX_POS|VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_COLOR|VTX_MATERIAL|VTX_SKIN|VTX_SIZE))
   {
      struct VtxDupIndex : VtxDupNrm
      {
         VecI2 index;
         Int   count;
         VecI4 color;
      };

      Box box; if(!getBox(box))return T;

      // create vertex array
      Int vtx_num=0; REPA(parts)vtx_num+=parts[i].base.vtxs(); // do not use T.vtxs() as it includes MeshRender
      Memc<VtxDupIndex> vtxs; vtxs.setNum(vtx_num); vtx_num=0;
      REPAD(p, T)
      {
         MeshBase &mshb=parts[p].base;
       C Vec      *nrm =mshb.vtx.nrm();
         REPA(mshb.vtx)
         {
            VtxDupIndex &vtx=vtxs[vtx_num++];
            vtx.pos=mshb.vtx.pos(i);
            vtx.nrm=(nrm ? nrm[i] : VecZero);
            vtx.index.set(i, p);
            vtx.count=0;
            vtx.color.zero();
         }
      }

      // get vtx dup
      SetVtxDup(SCAST(Memc<VtxDupNrm>, vtxs), box, pos_eps, nrm_cos);

      // weldVtx
      REPA(vtxs)
      {
         VtxDupIndex &vn=vtxs[i]; vtxs[vn.dup].count++; if(vn.dup!=i)
         {
            VtxDupIndex &vd=vtxs[vn.dup];
            if(flag&VTX_POS     )parts[vn.index.y].base.vtx.pos     (vn.index.x) =parts[vd.index.y].base.vtx.pos     (vd.index.x);
            if(flag&VTX_MATERIAL)parts[vn.index.y].base.vtx.material(vn.index.x) =parts[vd.index.y].base.vtx.material(vd.index.x); // !! sum must be equal to 255 !!
            if(flag&VTX_MATRIX  )parts[vn.index.y].base.vtx.matrix  (vn.index.x) =parts[vd.index.y].base.vtx.matrix  (vd.index.x);
            if(flag&VTX_BLEND   )parts[vn.index.y].base.vtx.blend   (vn.index.x) =parts[vd.index.y].base.vtx.blend   (vd.index.x); // !! sum must be equal to 255 !!
            if(flag&VTX_NRM     )parts[vd.index.y].base.vtx.nrm     (vd.index.x)+=parts[vn.index.y].base.vtx.nrm     (vn.index.x);
            if(flag&VTX_TAN     )parts[vd.index.y].base.vtx.tan     (vd.index.x)+=parts[vn.index.y].base.vtx.tan     (vn.index.x);
            if(flag&VTX_BIN     )parts[vd.index.y].base.vtx.bin     (vd.index.x)+=parts[vn.index.y].base.vtx.bin     (vn.index.x);
            if(flag&VTX_HLP     )parts[vd.index.y].base.vtx.hlp     (vd.index.x)+=parts[vn.index.y].base.vtx.hlp     (vn.index.x);
            if(flag&VTX_TEX0    )parts[vd.index.y].base.vtx.tex0    (vd.index.x)+=parts[vn.index.y].base.vtx.tex0    (vn.index.x);
            if(flag&VTX_TEX1    )parts[vd.index.y].base.vtx.tex1    (vd.index.x)+=parts[vn.index.y].base.vtx.tex1    (vn.index.x);
            if(flag&VTX_TEX2    )parts[vd.index.y].base.vtx.tex2    (vd.index.x)+=parts[vn.index.y].base.vtx.tex2    (vn.index.x);
            if(flag&VTX_SIZE    )parts[vd.index.y].base.vtx.size    (vd.index.x)+=parts[vn.index.y].base.vtx.size    (vn.index.x);
         }
         if(flag&VTX_COLOR)vtxs[vn.dup].color+=parts[vn.index.y].base.vtx.color(vn.index.x).v4;
      }
      // first calculate the average values, then set those values
      if(flag&VTX_NRM  ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.nrm  (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.nrm (vn.index.x)=parts[vd.index.y].base.vtx.nrm (vd.index.x);}}
      if(flag&VTX_TAN  ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.tan  (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.tan (vn.index.x)=parts[vd.index.y].base.vtx.tan (vd.index.x);}}
      if(flag&VTX_BIN  ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.bin  (vn.index.x).normalize();} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.bin (vn.index.x)=parts[vd.index.y].base.vtx.bin (vd.index.x);}}
      if(flag&VTX_HLP  ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.hlp  (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.hlp (vn.index.x)=parts[vd.index.y].base.vtx.hlp (vd.index.x);}}
      if(flag&VTX_TEX0 ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.tex0 (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.tex0(vn.index.x)=parts[vd.index.y].base.vtx.tex0(vd.index.x);}}
      if(flag&VTX_TEX1 ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.tex1 (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.tex1(vn.index.x)=parts[vd.index.y].base.vtx.tex1(vd.index.x);}}
      if(flag&VTX_TEX2 ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.tex2 (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.tex2(vn.index.x)=parts[vd.index.y].base.vtx.tex2(vd.index.x);}}
      if(flag&VTX_SIZE ){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.size (vn.index.x)/=vn.count  ;} REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.size(vn.index.x)=parts[vd.index.y].base.vtx.size(vd.index.x);}}
      if(flag&VTX_COLOR){REPA(vtxs){VtxDupIndex &vn=vtxs[i]; if(vn.dup==i)parts[vn.index.y].base.vtx.color(vn.index.x).set(DivRound(vn.color.x, vn.count), DivRound(vn.color.y, vn.count), DivRound(vn.color.z, vn.count), DivRound(vn.color.w, vn.count));}
         REPA(vtxs){VtxDupIndex &vn=vtxs[i], &vd=vtxs[vn.dup]; parts[vn.index.y].base.vtx.color(vn.index.x)=parts[vd.index.y].base.vtx.color(vd.index.x);}
      }

      // remove degenerate faces
      if((flag&VTX_POS) && remove_degenerate_faces_eps>=0)removeDegenerateFaces(remove_degenerate_faces_eps);
   }
   return T;
}

MeshLod& MeshLod::weldEdge () {REPAO(parts).base.weldEdge (); return T;}
MeshLod& MeshLod::tesselate() {REPAO(parts).base.tesselate(); return T;}
MeshLod& MeshLod::subdivide() {REPAO(parts).base.subdivide(); return T;}

MeshLod& MeshLod::boneRemap(C MemPtr<Byte, 256> &old_to_new)  {REPAO(parts).boneRemap  (old_to_new); return T;}
void     MeshLod::includeUsedBones(Bool (&bones)[256]      )C {REPAO(parts).includeUsedBones(bones);}
void     MeshLod::    setUsedBones(Bool (&bones)[256]      )C {Zero(bones); includeUsedBones(bones);}

MeshLod& MeshLod::freeOpenGLESData() {REPAO(parts).freeOpenGLESData(); return T;}
/******************************************************************************/
// FIX
/******************************************************************************/
MeshLod& MeshLod::fixTexOffset  (Byte tex_index) {REPA(T)parts[i].base.fixTexOffset  (tex_index); return T;}
MeshLod& MeshLod::fixTexWrapping(Byte tex_index) {REPA(T)parts[i].base.fixTexWrapping(tex_index); return T;}
/******************************************************************************/
// CONVERT
/******************************************************************************/
MeshLod& MeshLod::edgeToDepth(Bool tex_align) {REPA(T)parts[i].base.edgeToDepth(tex_align); return T;}
MeshLod& MeshLod::edgeToTri  (Bool set_id   ) {REPA(T)parts[i].base.edgeToTri  (set_id   ); return T;}
MeshLod& MeshLod::triToQuad  (Flt  cos      ) {REPA(T)parts[i].base.triToQuad  (cos      ); return T;}
MeshLod& MeshLod::quadToTri  (Flt  cos      ) {REPA(T)parts[i].base.quadToTri  (cos      ); return T;}
/******************************************************************************/
// ADD / REMOVE
/******************************************************************************/
MeshLod& MeshLod::add(C MeshBase &src)
{
   MeshBase temp; temp.create(src); // create in 'temp' in case 'src' belongs to this (and 'New' would change its memory address)
   Swap(parts.New().base, temp);
   return T;
}
MeshLod& MeshLod::add(C MeshPart &src)
{
   MeshPart temp; temp.create(src); // create in 'temp' in case 'src' belongs to this (and 'New' would change its memory address)
   Swap(parts.New(), temp);
   return T;
}
MeshLod& MeshLod::add(C MeshLod &src, C Mesh *src_mesh, C Mesh *this_mesh)
{
   if(&src==this)
   {
      Int  num=parts.elms();
      FREP(num)
      {
         MeshPart &dest=parts.New(); dest.create(parts[i]); // don't put New into create(..) (because of memory address issues)
      }
   }else
   {
      if(!is())copyParams(src); // if this is empty, then copy params from src
      FREPA(src)
      {
         MeshPart &dest=parts.New().create(src.parts[i]);
         if(src_mesh && this_mesh)dest.variationRemap(*src_mesh, *this_mesh);
      }
   }
   return T;
}
/******************************************************************************/
// OPTIMIZE
/******************************************************************************/
MeshLod& MeshLod::removeDoubleEdges    (                                                          ) {                    REPA(T)         parts[i].base.removeDoubleEdges    (                                        ); return       T;}
MeshLod& MeshLod::removeDegenerateFaces(Flt  eps                                                  ) {                    REPA(T)         parts[i].base.removeDegenerateFaces(eps                                     ); return       T;}
Bool     MeshLod::removeUnusedVtxs     (Bool include_edge_references                              ) {Bool changed=false; REPA(T)changed|=parts[i].base.removeUnusedVtxs     (include_edge_references                 ); return changed;}
MeshLod& MeshLod::removeSingleFaces    (Flt  fraction                                             ) {                    REPA(T)         parts[i].base.removeSingleFaces    (fraction                                ); return       T;}
MeshLod& MeshLod::weldInlineEdges      (Flt  cos_edge, Flt cos_vtx, Bool z_test                   ) {                    REPA(T)         parts[i].base.weldInlineEdges      (cos_edge, cos_vtx, z_test               ); return       T;}
MeshLod& MeshLod::weldCoplanarFaces    (Flt  cos_face, Flt cos_vtx, Bool safe, Flt max_face_length) {                    REPA(T)         parts[i].base.weldCoplanarFaces    (cos_face, cos_vtx, safe, max_face_length); return       T;}

static Bool HasMaterial(MeshPart &part, Material *mtrl)
{
   return part.multiMaterial(0)==mtrl
       || part.multiMaterial(1)==mtrl
       || part.multiMaterial(2)==mtrl
       || part.multiMaterial(3)==mtrl;
}
struct MaterialIndex
{
   CChar *name;
   Byte   index;

   static Int Compare(C MaterialIndex &a, C MaterialIndex &b)
   {
      return ComparePath(b.name, a.name); // the order is swapped intentionally so null materials are at the end
   }

   MaterialIndex(C MaterialPtr &material, Byte index) {T.name=material.name(); T.index=index;}
};
MeshLod& MeshLod::sortByMaterials()
{
   // first sort multi materials by universal comparison (best are paths to file)
   FREPA(T)
   {
      MeshPart &p=parts[i];
      if(p.multiMaterial(1) || p.multiMaterial(2) || p.multiMaterial(3))
      {
         MaterialIndex m[4]=
         {
            MaterialIndex(p.multiMaterial(0), 0),
            MaterialIndex(p.multiMaterial(1), 1),
            MaterialIndex(p.multiMaterial(2), 2),
            MaterialIndex(p.multiMaterial(3), 3),
         };
         Sort(m, Elms(m), MaterialIndex::Compare);
         Byte new_index[4]={m[0].index, m[1].index, m[2].index, m[3].index};
         p.remapMaterials(new_index);
      }
   }

   // now sort parts, so the same materials would be close to each other
   FREPA(T)
   {
      MeshPart &p=parts[i];
      Int       best_match=-1, matching_materials=0;
      for(Int j=i+1; j<parts.elms(); j++)
      {
         MeshPart &t    =parts[j];
         Int       mtrls=0;
         if(p.multiMaterial(0) && HasMaterial(t, p.multiMaterial(0)()))mtrls++;
         if(p.multiMaterial(1) && HasMaterial(t, p.multiMaterial(1)()))mtrls++;
         if(p.multiMaterial(2) && HasMaterial(t, p.multiMaterial(2)()))mtrls++;
         if(p.multiMaterial(3) && HasMaterial(t, p.multiMaterial(3)()))mtrls++;

         if(mtrls>matching_materials){best_match=j; matching_materials=mtrls;}
      }
      if(best_match>=0 && best_match!=i+1)Swap(parts[i+1], parts[best_match]);
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
