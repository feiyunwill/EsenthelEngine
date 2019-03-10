/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
DEFINE_CACHE(Mesh, Meshes, MeshPtr, "Mesh");
/******************************************************************************/
// VARIATIONS
/******************************************************************************/
     Mesh::Variations::Variations(C Variations &src) : Variations() {T=src;}
void Mesh::Variations::del  (                             ) {Free(_variation); _variations=0;}
Int  Mesh::Variations::alloc(Int variations, Int name_size)
{
   del();
   Int size=SIZE(Variation)*variations + name_size;
  _variations=variations;
  _variation =(Variation*)Alloc(size);
   return size;
}
   Int  Mesh::Variations::nameSize (     )C {return _variations ? _variation[_variations-1].name_offset + Length(name(_variations-1))+1 : 0;}
 Char8* Mesh::Variations::nameStart(     )C {return (Char8*)(_variation+_variations);}
CChar8* Mesh::Variations::name     (Int i)C {return InRange(i, _variations) ? nameStart()+_variation[i].name_offset : null;}
  UInt  Mesh::Variations::id       (Int i)C {return InRange(i, _variations) ?             _variation[i].id          : 0   ;}

void Mesh::Variations::operator=(C Variations &src)
{
   if(this!=&src)
   {
      Int size=alloc(src._variations, src.nameSize());
      CopyFast(_variation, src._variation, size);
   }
}

Bool Mesh::Variations::save(File &f)C
{
   f.cmpUIntV(_variations); if(_variations)
   {
      Int name_size=nameSize(); f.cmpUIntV(name_size);
      FREP(_variations){C Variation &variation=_variation[i]; f<<variation.id; f.cmpUIntV(variation.name_offset);}
      f.put(nameStart(), name_size);
   }
   return f.ok();
}
Bool Mesh::Variations::load(File &f)
{
  _variations=f.decUIntV(); if(_variations)
   {
      Int name_size=f.decUIntV();
      alloc(_variations, name_size);
      FREP(_variations){Variation &variation=_variation[i]; f>>variation.id; variation.name_offset=f.decUIntV();}
      f.get(nameStart(), name_size);
   }
   if(f.ok())return true;
   del(); return false;
}
/******************************************************************************/
static UInt RandomID() // avoid zero as it's reserved for #0 default variation
{
   for(;;)if(UInt id=Random())return id;
}
struct VariationEx
{
   UInt id;
   Str8 name;

   void randomizeID() {id=RandomID();}
};
static void GetVariations(Memt<VariationEx> &variations, C Mesh &mesh)
{
   variations.setNum(mesh.variations());
   REPA(variations)
   {
      VariationEx &dest=variations[i];
      dest.id  =mesh.variationID  (i);
      dest.name=mesh.variationName(i);
   }
}
static void SetVariations(Memt<VariationEx> &variations, Mesh &mesh)
{
   // we're not setting variation #0
   Int name_size=0; for(Int i=1; i<variations.elms(); i++)name_size+=variations[i].name.length()+1;
   mesh._variations.alloc(Max(0, variations.elms()-1), name_size);
   name_size=0;
   Char8 *variation_name=mesh._variations.nameStart();
   FREP(mesh._variations._variations)
   {
      Mesh::Variations::Variation   &dest=mesh._variations._variation [i  ];
                      C VariationEx &src =                  variations[i+1];
      Int length_1=src.name.length()+1;
      dest.name_offset=name_size;
      dest.id         =(src.id ? src.id : RandomID());
      Set(variation_name, src.name, length_1);
      variation_name+=length_1;
      name_size     +=length_1;
   }
}
CChar8* Mesh::variationName( Int i )C {return _variations.name(i-1);} // #0 is default and always null
  UInt  Mesh::variationID  ( Int i )C {return _variations.id  (i-1);} // #0 is default and always 0
   Int  Mesh::variationFind(UInt id)C
{
   if(!id)return 0; // #0 is default and always 0
   REP(_variations._variations)if(_variations._variation[i].id==id)return i+1;
   return -1;
}
Int Mesh::variationFind(CChar8 *name)C
{
   if(!Is(name))return 0; // #0 is default and always null
   REP(_variations._variations)if(Equal(_variations.name(i), name))return i+1;
   return -1;
}
Int   Mesh::variations(              )C {return _variations._variations+1;}
Mesh& Mesh::variations(Int variations)
{
   MAX(variations, 1);
   if(T.variations()!=variations)
   {
      Memt<VariationEx> variation_exs; GetVariations(variation_exs, T);
      variation_exs.setNum(variations);
      for(Int i=T.variations(); i<variation_exs.elms(); i++)variation_exs[i].randomizeID(); // randomize ID
      if(variation_exs.elms()<T.variations())REP(lods()) // minimize mesh part variations
      {
         MeshLod &lod=T.lod(i); REPA(lod)
         {
            MeshPart &part=lod.parts[i];
            if(part.variations()>variation_exs.elms())part.variations(variation_exs.elms()); // if part has more variations than what we want, then remove the extra
         }
      }
      SetVariations(variation_exs, T);
   }
   return T;
}
Mesh& Mesh::variationName(Int variation, C Str8 &name)
{
   if(variation>0 // can't set name to 0-th default variation
   && InRange(variation, variations()))
   {
      Memt<VariationEx> variations; GetVariations(variations, T);
      variations[variation].name=name;
      SetVariations(variations, T);
   }
   return T;
}
Mesh& Mesh::variationRemove(Int variation)
{
   if(InRange(variation, variations()) && variations()>1)
   {
      Int num=T.variations(); REP(lods()){MeshLod &lod=T.lod(i); REPA(lod)lod.parts[i].variations(num).variationRemove(variation);} // allocate all first to make sure others will be processed ok
      Memt<VariationEx> variations; GetVariations(variations, T);
      variations.remove(variation, true);
      SetVariations(variations, T);
   }
   return T;
}
Mesh& Mesh::variationKeep(Int variation)
{
   if(variations()>1)
   {
      REP(lods()){MeshLod &lod=T.lod(i); REPA(lod)lod.parts[i].variationKeep(variation);}
     _variations.del();
   }
   return T;
}
Mesh& Mesh::variationMove(Int variation, Int new_index)
{
   if(InRange(variation, variations()))
   {
      Clamp(new_index, 0, variations()-1); if(new_index!=variation)
      {
         Int num=T.variations(); REP(lods()){MeshLod &lod=T.lod(i); REPA(lod)lod.parts[i].variations(num).variationMove(variation, new_index);} // allocate all first to make sure others will be processed ok
         Memt<VariationEx> variations; GetVariations(variations, T);
         variations.moveElm(variation, new_index);
         SetVariations(variations, T);
      }
   }
   return T;
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Mesh::zero()
{
   ext       .zero();
   lod_center.zero();
  _skeleton       =null;
  _draw_group_enum=null;
}
Mesh::Mesh() {zero();}

Mesh& Mesh::delBase  () {REP(lods())lod(i).delBase  (); return T;}
Mesh& Mesh::delRender() {REP(lods())lod(i).delRender(); return T;}
Mesh& Mesh::del      ()
{
      _lods.del();
     super::del();
  _bone_map.del();
_variations.del();
   zero(); return T;
}
Mesh& Mesh::create(Int parts)
{
   del();
   super::create(parts);
   return T;
}
Mesh& Mesh::create(C Mesh &src, UInt flag_and)
{
   if(this==&src)keepOnly(flag_and);else
   {
     _lods.setNum(src._lods.elms()); FREP(lods())lod(i).create(src.lod(i), flag_and);
      copyParams(src);
   }
   return T;
}
Mesh& Mesh::create(C MeshGroup &src, UInt flag_and)
{
   return create(src.meshes.data(), src.meshes.elms(), flag_and);
}
Mesh& Mesh::create(C Mesh *mesh, Int meshes, UInt flag_and)
{
   if(!mesh || meshes<=0)del();else
   {
      Mesh temp, &dest=(InRange(IntPtr(this-mesh), IntPtr(meshes)) ? temp : T); // if 'mesh' array contains 'this' then operate on 'temp' first
      Int lods=0; REP(meshes)MAX(lods, mesh[i].lods()); // get max number of LOD's
      dest.setLods(lods).copyParams(*mesh); // copy params from first Mesh
      REPD(l, dest.lods())
      {
         MeshLod &lod=dest.lod(l);
         Int parts=0; REP(meshes){C Mesh &src_mesh=mesh[i]; if(InRange(l, src_mesh.lods()))parts+=src_mesh.lod(l).parts.elms();} // how many parts for this LOD
         lod.parts.setNum(parts); parts=0; // always set LOD parts in case we're operating on MeshGroup that already had some data, and this will clear it
         Bool params=true;
         FREP(meshes) // add in order
         {
          C Mesh &src_mesh=mesh[i]; if(InRange(l, src_mesh.lods()))
            {
             C MeshLod &src_lod=src_mesh.lod(l);
               if(params){lod.copyParams(src_lod); params=false;} // copy params from first encountered LOD
               FREPA(src_lod)lod.parts[parts++].create(src_lod.parts[i], flag_and);
            }
         }
      }
      // at this point 'dest.ext' and 'dest.lod_center' are already set because of earlier 'copyParams', so we can override them only if we want to
      Int valid=0; Box box; REP(meshes)
      {
       C Mesh &src_mesh=mesh[i]; if(src_mesh.vtxs()){if(valid)box|=src_mesh.ext;else {box=src_mesh.ext; dest.lod_center=src_mesh.lod_center;} valid++;} // copy 'lod_center' on first found valid mesh
      }
      if(valid) // if encountered any valid mesh vtxs (otherwise keep what was already set in 'copyParams')
      {
         dest.ext=box; if(valid>1)dest.lod_center=dest.ext.pos; // if more than 1 valid mesh, then use box center, if only 1 valid mesh then use its 'lod_center' that we've already set in the loop
      }
      if(&dest==&temp)Swap(temp, T);
   }
   return T;
}
void Mesh::copyParams(C Mesh &src)
{
   if(this!=&src)
   {
      ext            =src. ext       ;
      lod_center     =src. lod_center;
     _skeleton       =src._skeleton  ;
     _draw_group_enum=src._draw_group_enum;
     _bone_map       =src._bone_map;
     _variations     =src._variations;
   }
}
/******************************************************************************/
Mesh& Mesh::include (UInt flag) {REP(lods())lod(i).include (flag); return T;}
Mesh& Mesh::exclude (UInt flag) {REP(lods())lod(i).exclude (flag); return T;}
Mesh& Mesh::keepOnly(UInt flag) {REP(lods())lod(i).keepOnly(flag); return T;}
/******************************************************************************/
// GET
/******************************************************************************/
UInt Mesh::memUsage()C {UInt size=0; REP(lods())size+=lod(i).memUsage(); return size;}

C MeshLod& Mesh::getDrawLod(C Matrix &matrix)C
{
   if(_lods.elms())
   {
      Flt dist2=GetLodDist2(lod_center, matrix);
      REPA(_lods){C MeshLod &test=_lods[i]; if(dist2>=test.dist2)return test;}
   }
   return T;
}
C MeshLod& Mesh::getDrawLod(C MatrixM &matrix)C
{
   if(_lods.elms())
   {
      Flt dist2=GetLodDist2(lod_center, matrix);
      REPA(_lods){C MeshLod &test=_lods[i]; if(dist2>=test.dist2)return test;}
   }
   return T;
}
Int Mesh::getDrawLodI(C Matrix &matrix)C
{
   if(_lods.elms())
   {
      Flt dist2=GetLodDist2(lod_center, matrix);
      REPA(_lods)if(dist2>=_lods[i].dist2)return i+1;
   }
   return 0;
}
Int Mesh::getDrawLodI(C MatrixM &matrix)C
{
   if(_lods.elms())
   {
      Flt dist2=GetLodDist2(lod_center, matrix);
      REPA(_lods)if(dist2>=_lods[i].dist2)return i+1;
   }
   return 0;
}
C MeshLod& Mesh::getDrawLod (Flt dist2)C {REPA(_lods){C MeshLod &test=_lods[i]; if(dist2>= test   .dist2)return test;} return T;}
      Int  Mesh::getDrawLodI(Flt dist2)C {REPA(_lods)                           if(dist2>=_lods[i].dist2)return i+1 ;  return 0;}
/*Flt Mesh::lodQuality(Int i, Int base)C
{
   if(InRange(i, lods()) && InRange(base, lods()))
   {
    C MeshLod &b=lod(base),
              &l=lod(i   );
      Int      v=b.vtxs     (),
               f=b.trisTotal();
      return Avg(v ? Flt(l.vtxs     ())/v : 1,
                 f ? Flt(l.trisTotal())/f : 1);
   }
   return 0;
}
/******************************************************************************/
// SET
/******************************************************************************/
Mesh& Mesh::setEdgeNormals(Bool flag) {REP(lods())lod(i).setEdgeNormals(flag); return T;}
Mesh& Mesh::setFaceNormals(         ) {REP(lods())lod(i).setFaceNormals(    ); return T;}
Mesh& Mesh::setNormals2D  (Bool flag) {REP(lods())lod(i).setNormals2D  (flag); return T;}
Mesh& Mesh::setNormals    (         ) {REP(lods())lod(i).setNormals    (    ); return T;}
Mesh& Mesh::setTangents   (         ) {REP(lods())lod(i).setTangents   (    ); return T;}
Mesh& Mesh::setBinormals  (         ) {REP(lods())lod(i).setBinormals  (    ); return T;}
Mesh& Mesh::setAutoTanBin (         ) {REP(lods())lod(i).setAutoTanBin (    ); return T;}
Bool  Mesh::setBox        (Bool skip_hidden_parts)
{
   Box box; if(getBox(box, skip_hidden_parts))
   {
      ext       =box;
      lod_center=ext.pos;
      return true;
   }
   ext       .zero();
   lod_center.zero();
   return false;
}
Mesh& Mesh::setVtxDup2D                           (UInt flag, Flt pos_eps, Flt nrm_cos) {REP(lods())lod(i).setVtxDup2D                           (flag, pos_eps, nrm_cos); return T;}
Mesh& Mesh::setVtxDup                             (UInt flag, Flt pos_eps, Flt nrm_cos) {REP(lods())lod(i).setVtxDup                             (flag, pos_eps, nrm_cos); return T;}
Mesh& Mesh::setVtxColorAlphaAsTesselationIntensity(Bool tesselate_edges               ) {REP(lods())lod(i).setVtxColorAlphaAsTesselationIntensity(tesselate_edges       ); return T;}
Mesh& Mesh::setAdjacencies                        (Bool faces, Bool edges             ) {REP(lods())lod(i).setAdjacencies                        (faces, edges          ); return T;}

Mesh& Mesh::setBase  (Bool only_if_empty     ) {REP(lods())lod(i).setBase  (only_if_empty); return T;}
Mesh& Mesh::setRender(Bool optimize          ) {REP(lods())lod(i).setRender(optimize, i  ); return T;}
Mesh& Mesh::setShader(                       ) {REP(lods())lod(i).setShader(          i  ); return T;}
Mesh& Mesh::material (C MaterialPtr &material) {REP(lods())lod(i).material (material, i  ); return T;}
/******************************************************************************/
// JOIN / SPLIT
/******************************************************************************/
Mesh& Mesh::join   (Int i0, Int i1                                                              , Flt weld_pos_eps) {           super::join   (i0, i1                                                  , weld_pos_eps); return T;}
Mesh& Mesh::joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag, Flt weld_pos_eps) {REP(lods())lod(i).joinAll(test_material, test_draw_group, test_name, test_vtx_flag, weld_pos_eps); return T;}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
Mesh& Mesh::texMap   (  Flt     scale , Byte tex_index) {REP(lods())lod(i).texMap   (scale , tex_index); return T;}
Mesh& Mesh::texMap   (C Matrix &matrix, Byte tex_index) {REP(lods())lod(i).texMap   (matrix, tex_index); return T;}
Mesh& Mesh::texMap   (C Plane  &plane , Byte tex_index) {REP(lods())lod(i).texMap   (plane , tex_index); return T;}
Mesh& Mesh::texMap   (C Ball   &ball  , Byte tex_index) {REP(lods())lod(i).texMap   (ball  , tex_index); return T;}
Mesh& Mesh::texMap   (C Tube   &tube  , Byte tex_index) {REP(lods())lod(i).texMap   (tube  , tex_index); return T;}
Mesh& Mesh::texMove  (C Vec2   &move  , Byte tex_index) {REP(lods())lod(i).texMove  (move  , tex_index); return T;}
Mesh& Mesh::texScale (C Vec2   &scale , Byte tex_index) {REP(lods())lod(i).texScale (scale , tex_index); return T;}
Mesh& Mesh::texRotate(  Flt     angle , Byte tex_index) {REP(lods())lod(i).texRotate(angle , tex_index); return T;}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
Mesh& Mesh::move         (              C Vec &move       ) {REP(lods())lod(i).move         (       move);              ext+=move;                     lod_center+=move; return T;}
Mesh& Mesh::scale        (C Vec &scale                    ) {REP(lods())lod(i).scale        (scale      ); ext*=scale ;            lod_center*=scale ;                   return T;}
Mesh& Mesh::scaleMove    (C Vec &scale, C Vec &move       ) {REP(lods())lod(i).scaleMove    (scale, move); ext*=scale ; ext+=move; lod_center*=scale ; lod_center+=move; return T;}
Mesh& Mesh::scaleMoveBase(C Vec &scale, C Vec &move       ) {REP(lods())lod(i).scaleMoveBase(scale, move); ext*=scale ; ext+=move; lod_center*=scale ; lod_center+=move; return T;}
Mesh& Mesh::transform    (C Matrix3              &matrix  ) {REP(lods())lod(i).transform    (matrix     ); ext*=matrix;            lod_center*=matrix;                   return T;}
Mesh& Mesh::transform    (C Matrix               &matrix  ) {REP(lods())lod(i).transform    (matrix     ); ext*=matrix;            lod_center*=matrix;                   return T;}
Mesh& Mesh::animate      (C MemPtrN<Matrix, 256> &matrixes) {REP(lods())lod(i).animate      (matrixes   ); setBox();                                                     return T;}
Mesh& Mesh::animate      (C AnimatedSkeleton     &skel    ) {REP(lods())lod(i).animate      (skel       ); setBox();                                                     return T;}
Mesh& Mesh::setSize      (C Box &box                      )
{
   Vec size=T.ext.size(),
       scale((size.x>EPS) ? box.w()/size.x : 0,
             (size.y>EPS) ? box.h()/size.y : 0,
             (size.z>EPS) ? box.d()/size.z : 0),
       move=box.min-T.ext.min()*scale;
   scaleMove(scale, move);
   return T;
}
Mesh& Mesh::mirrorX    () {REP(lods())lod(i).mirrorX    (); ext.mirrorX    (); CHS(lod_center.x);        return T;}
Mesh& Mesh::mirrorY    () {REP(lods())lod(i).mirrorY    (); ext.mirrorY    (); CHS(lod_center.y);        return T;}
Mesh& Mesh::mirrorZ    () {REP(lods())lod(i).mirrorZ    (); ext.mirrorZ    (); CHS(lod_center.z);        return T;}
Mesh& Mesh::reverse    () {REP(lods())lod(i).reverse    ();                                              return T;}
Mesh& Mesh::rightToLeft() {REP(lods())lod(i).rightToLeft(); ext.rightToLeft(); lod_center.rightToLeft(); return T;}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
Mesh& Mesh::weldVtx2D    (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REP(lods())lod(i).weldVtx2D    (flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
Mesh& Mesh::weldVtx      (UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REP(lods())lod(i).weldVtx      (flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}
Mesh& Mesh::weldVtxValues(UInt flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {REP(lods())lod(i).weldVtxValues(flag, pos_eps, nrm_cos, remove_degenerate_faces_eps); return T;}

Mesh& Mesh::weldEdge () {REP(lods())lod(i).weldEdge (); return T;}
Mesh& Mesh::tesselate() {REP(lods())lod(i).tesselate(); return T;}
Mesh& Mesh::subdivide() {REP(lods())lod(i).subdivide(); return T;}

Int Mesh::boneFind(CChar8 *bone_name)C {return _bone_map.find(bone_name);}

//Bool boneRename(C Str8 &src, C Str8 &dest                             ) ; // rename 'src' bone to 'dest' bone, returns true if a bone was renamed
//Bool Mesh::boneRename(C Str8 &src, C Str8 &dest) {return _bone_map.rename(src, dest);}

Mesh& Mesh::boneRemap(C MemPtr<Byte, 256> &old_to_new, Bool remap_names)
{
   REP(lods())lod(i).boneRemap(old_to_new);
   if(remap_names)_bone_map.remap(old_to_new);
   return T;
}
void Mesh::includeUsedBones(Bool (&bones)[256])C {REP(lods())lod(i).includeUsedBones(bones);}
void Mesh::    setUsedBones(Bool (&bones)[256])C {Zero(bones); includeUsedBones(bones);}

Mesh& Mesh::setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func, Threads *threads) {REP(lods())lod(i).setVtxAO(strength, bias, max, ray_length, pos_eps, rays, func, threads); return T;}

Mesh& Mesh::freeOpenGLESData() {REP(lods())lod(i).freeOpenGLESData(); return T;}
/******************************************************************************/
// FIX
/******************************************************************************/
Mesh& Mesh::fixTexWrapping(Byte tex_index) {REP(lods())lod(i).fixTexWrapping(tex_index); return T;}
Mesh& Mesh::fixTexOffset  (Byte tex_index) {REP(lods())lod(i).fixTexOffset  (tex_index); return T;}
/******************************************************************************/
// CONVERT
/******************************************************************************/
Mesh& Mesh::edgeToDepth(Bool tex_align) {REP(lods())lod(i).edgeToDepth(tex_align); return T;}
Mesh& Mesh::edgeToTri  (Bool set_id   ) {REP(lods())lod(i).edgeToTri  (set_id   ); return T;}
Mesh& Mesh::triToQuad  (Flt  cos      ) {REP(lods())lod(i).triToQuad  (cos      ); return T;}
Mesh& Mesh::quadToTri  (Flt  cos      ) {REP(lods())lod(i).quadToTri  (cos      ); return T;}
/******************************************************************************/
// ADD / REMOVE
/******************************************************************************/
MeshLod& Mesh::newLod()
{
   MeshLod &last=_lods.New();
   MeshLod &prev= lod(lods()-2);
   last.copyParams(prev);
   return last;
}
MeshLod& Mesh::newLod(Int i)
{
   Clamp(i, 0, lods());
   MeshLod &lod=_lods.NewAt(i-1);
   if(!i)
   {
      MeshLod &prev=T;
      lod.copyParams(prev);
      Swap(prev, lod);
      return T;
   }else
   {
      MeshLod &prev=T.lod(i-1);
      lod.copyParams(prev);
      return lod;
   }
}
Mesh& Mesh::add(C MeshBase &src)
{
   Box box; Bool valid_box=src.getBox(box), this_box=(vtxs()>0);
   super::add(src);
   if(valid_box)
   {
      if(this_box)T.ext|=box;else T.ext=box; lod_center=T.ext.pos;
   }
   return T;
}
Mesh& Mesh::add(C MeshPart &src)
{
   Box box; Bool valid_box=src.getBox(box), this_box=(vtxs()>0);
   super::add(src);
   if(valid_box)
   {
      if(this_box)T.ext|=box;else T.ext=box; lod_center=T.ext.pos;
   }
   return T;
}
Mesh& Mesh::variationInclude(C Mesh &src)
{
   FREP(src.variations())if(variationFind(src.variationName(i))<0) // add in same order
   {
      variations   (variations()+1); // create new one
      variationName(variations()-1, src.variationName(i)); // set name
   }
   return T;
}
Mesh& Mesh::add(C Mesh &src)
{
   if(!is())copyParams(src);else
   {
      if(src.vtxs())
      {
         if(vtxs()){ext|=src.ext; lod_center=    ext.pos   ;}
         else      {ext =src.ext; lod_center=src.lod_center;}
      }
      variationInclude(src);
   }
   if(src.lods()>lods())setLods(src.lods()); // make room for LOD's
   FREP(src.lods())lod(i).add(src.lod(i), &src, this);
   return T;
}
Mesh& Mesh::remove(Int i, Bool set_box)
{
   if(InRange(i, T))
   {
      parts.remove(i, true);
      if(set_box)setBox();
   }
   return T;
}
Mesh& Mesh::removeLod(Int i)
{
   if(InRange(i, lods()))
   {
      if(i)_lods.remove(i-1, true);else
      if( !_lods.elms())del();else
      {
         Swap(SCAST(MeshLod, T), _lods[0]); // move #0 to T
       //dist2=0; // since it's now the main LOD, then clear its distance, this is to keep consistency about Main LOD being at distance 0
        _lods.remove(0, true);
      }
   }
   return T;
}
Mesh& Mesh::setLods(Int n)
{
   if(n<=0)del();else _lods.setNum(n-1);
   return T;
}
/******************************************************************************/
// OPTIMIZE
/******************************************************************************/
Mesh& Mesh::removeDoubleEdges    (                                                          ) {                    REP(lods())         lod(i).removeDoubleEdges    (                                        );                                 return       T;}
Mesh& Mesh::removeDegenerateFaces(Flt  eps                                                  ) {                    REP(lods())         lod(i).removeDegenerateFaces(eps                                     );                                 return       T;}
Bool  Mesh::removeUnusedVtxs     (Bool include_edge_references, Bool set_box                ) {Bool changed=false; REP(lods())changed|=lod(i).removeUnusedVtxs     (include_edge_references                 ); if(changed && set_box)setBox(); return changed;}
Mesh& Mesh::removeSingleFaces    (Flt  fraction                                             ) {                    REP(lods())         lod(i).removeSingleFaces    (fraction                                );                                 return       T;}
Mesh& Mesh::weldInlineEdges      (Flt  cos_edge, Flt cos_vtx, Bool z_test                   ) {                    REP(lods())         lod(i).weldInlineEdges      (cos_edge, cos_vtx, z_test               );                                 return       T;}
Mesh& Mesh::weldCoplanarFaces    (Flt  cos_face, Flt cos_vtx, Bool safe, Flt max_face_length) {                    REP(lods())         lod(i).weldCoplanarFaces    (cos_face, cos_vtx, safe, max_face_length);                                 return       T;}
Mesh& Mesh::sortByMaterials      (                                                          ) {                    REP(lods())         lod(i).sortByMaterials      (                                        );                                 return       T;}

Mesh& Mesh::simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps, Mesh *dest, Bool *stop)
{
   if(!dest)dest=this;
   if( dest!=this)
   {
      dest->copyParams(T);
      dest->setLods(lods());
   }
   REP(lods()) // go from the end so we can remove lod's if needed
   {
      MeshLod &dest_lod=dest->lod(i);
      lod(i).simplify(intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, keep_border, mode, pos_eps, &dest_lod, stop); if(stop && *stop)break;
      if(!dest_lod.is())dest->removeLod(i); // if became empty, then just remove it
   }
   return *dest;
}
/******************************************************************************/
//
/******************************************************************************/
Mesh& Mesh::skeleton(Skeleton *skeleton, Bool by_name)
{
   T._skeleton=skeleton;
   if(skeleton && skeleton->is() && !_bone_map.same(*skeleton)) // remap only if new skeleton exists (in case it failed to load, then it could be null or have no bones, or the parameter could just have been specifically set to null in order to clear the skeleton link) and if the existing bone map is different than of the new skeleton
   {
      if(_bone_map.is()) // remap vertexes only if the mesh already had a bone map (otherwise the vertex bone could be set, but the bone map doesn't exist yet, and in that case the vertex bone data could be lost if we've performed remapping)
      {
         Memt<Byte, 256> old_to_new;
        _bone_map.setRemap(*skeleton, old_to_new, by_name);
         boneRemap(old_to_new, false); // set false to not waste time on adjusting the '_bone_map' itself, because we're going to recreate it anyway below
      }

      // setup new mesh bone info
     _bone_map.create(*skeleton);
   }
   return T;
}
Mesh& Mesh::clearSkeleton()
{
  _skeleton=null;
  _bone_map.del();
   return T;
}
Bool  Mesh::hasDrawGroup    ( Int draw_group_index)C {REP(lods())if(lod(i).hasDrawGroup    (draw_group_index))return true; return false;}
Bool  Mesh::hasDrawGroupMask(UInt draw_group_mask )C {REP(lods())if(lod(i).hasDrawGroupMask(draw_group_mask ))return true; return false;}
Mesh& Mesh::   drawGroupEnum(Enum *e, Bool reset_when_not_found)
{
   T._draw_group_enum=e;
   REP(lods())
   {
      MeshLod &lod=T.lod(i); REPA(lod)
      {
         MeshPart &part=lod.parts[i];
         Int enum_index=(e ? e->findByIDUInt(part._draw_mask_enum_id) : -1); // find that enum
         if(InRange(enum_index, 32))part._draw_mask=IndexToFlag(enum_index);else // if enum found then adjust the draw mask
         if(reset_when_not_found   )part._draw_mask=IndexToFlag(0); // otherwise reset or keep
      }
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
