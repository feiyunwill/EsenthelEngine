/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define LEAF_RANDOM_BEND_RANGE 1024
/******************************************************************************/
static void SetLeafAttachment(MeshBase &mesh, C Vec2 &tex, Memc<Int> &face)
{
   // find face which has tex coords nearest 'tex'
   Flt dist;
   Int found=-1;
   REPA(face)
   {
      Int f=face[i];
      if( f&SIGN_BIT)
      {
         VecI4 ind =mesh.quad.ind(f^SIGN_BIT);
         Quad2 quad(mesh.vtx.tex0(ind.x), mesh.vtx.tex0(ind.y), mesh.vtx.tex0(ind.z), mesh.vtx.tex0(ind.w));
         Flt   d   =Dist(tex, quad);
         if(found==-1 || d<dist){found=f; dist=d;} // compare to -1 and not <0 because 'found' can have SIGN_BIT
      }else
      {
         VecI ind=mesh.tri.ind(f);
         Tri2 tri(mesh.vtx.tex0(ind.x), mesh.vtx.tex0(ind.y), mesh.vtx.tex0(ind.z));
         Flt  d  =Dist(tex, tri);
         if(found==-1 || d<dist){found=f; dist=d;} // compare to -1 and not <0 because 'found' can have SIGN_BIT
      }
   }

   // calculate UV coordinates of face
   Tri2 tex_tri;
   Tri  pos_tri;
   if(found&SIGN_BIT)
   {
      VecI4 ind=mesh.quad.ind(found^SIGN_BIT);
            tex_tri.set(mesh.vtx.tex0(ind.x), mesh.vtx.tex0(ind.y), mesh.vtx.tex0(ind.w));
            pos_tri.set(mesh.vtx.pos (ind.x), mesh.vtx.pos (ind.y), mesh.vtx.pos (ind.w));
   }else
   {
      VecI ind=mesh.tri.ind(found);
           tex_tri.set(mesh.vtx.tex0(ind.x), mesh.vtx.tex0(ind.y), mesh.vtx.tex0(ind.z));
           pos_tri.set(mesh.vtx.pos (ind.x), mesh.vtx.pos (ind.y), mesh.vtx.pos (ind.z));
   }

   Vec2 base =tex_tri.p[0]     ,
        u_dir=tex_tri.p[1]-base,
        v_dir=tex_tri.p[2]-base;
   /* tex=base + u*u_dir + v*v_dir
      
      tex.x = base.x + u*u_dir.x + v*v_dir.x
      tex.y = base.y + u*u_dir.y + v*v_dir.y

      u_dir.x*u + v_dir.x*v = tex.x - base.x
      u_dir.y*u + v_dir.y*v = tex.y - base.y
   */
   Vec pos;
   Flt u, v;
   if(Solve(u_dir.x, u_dir.y, v_dir.x, v_dir.y, tex.x-base.x, tex.y-base.y, u, v)!=1)pos=pos_tri.center();
   else                                                                              pos=pos_tri.p[0] + u*(pos_tri.p[1]-pos_tri.p[0]) + v*(pos_tri.p[2]-pos_tri.p[0]);

   REPA(face)
   {
      Int f=face[i];
      if( f&SIGN_BIT){VecI4 ind=mesh.quad.ind(f^SIGN_BIT); REPA(ind)mesh.vtx.hlp(ind.c[i])=pos;}
      else           {VecI  ind=mesh.tri .ind(f         ); REPA(ind)mesh.vtx.hlp(ind.c[i])=pos;}
   }
}
static void SetLeafAttachment(MeshBase &mesh, C Vec2 &tex)
{
   if(mesh.vtx.tex0())
   {
      mesh.setVtxDup().setAdjacencies(true, false).include(VTX_HLP);

      Memc<Int> face;
      Byte * tri_done=AllocZero<Byte>(mesh. tris()),
           *quad_done=AllocZero<Byte>(mesh.quads());

      REPA(mesh.tri)if(!tri_done[i])
      {
         face.add(i); tri_done[i]=true;
         for(Int last=0; last!=face.elms(); )
         {
            Int new_last=face.elms();
            for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
            {
               VecI4 a;
               Int   f=face[i];
               if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
               else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

               REPA(a) // iterate through all adjacent faces
               {
                  Int af =a.c[i];
                  if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
                  {
                     if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                     else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
                  }
               }
            }
            last=new_last;
         }
         SetLeafAttachment(mesh, tex, face);
         face.clear();
      }
      REPA(mesh.quad)if(!quad_done[i])
      {
         face.add(i^SIGN_BIT); quad_done[i]=true;
         for(Int last=0; last!=face.elms(); )
         {
            Int new_last=face.elms();
            for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
            {
               VecI4 a;
               Int   f=face[i];
               if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
               else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

               REPA(a) // iterate through all adjacent faces
               {
                  Int af =a.c[i];
                  if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
                  {
                     if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                     else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
                  }
               }
            }
            last=new_last;
         }
         SetLeafAttachment(mesh, tex, face);
         face.clear();
      }

      Free( tri_done);
      Free(quad_done);
   }
}
/******************************************************************************/
static void SetRandomColor(MeshBase &mesh, Memc<Int> &face, C Color &color)
{
   REPA(face)
   {
      Int f=face[i];
      if( f&SIGN_BIT){VecI4 q=mesh.quad.ind(f^SIGN_BIT); REPA(q)mesh.vtx.color(q.c[i])=color;}
      else           {VecI  t=mesh.tri .ind(f         ); REPA(t)mesh.vtx.color(t.c[i])=color;}
   }
}
static void SetRandomColor(MeshBase &mesh, Flt variation)
{
   const Byte random=Mid(255-RoundPos(255*variation), 0, 255);

   mesh.setVtxDup().setAdjacencies(true, false).include(VTX_COLOR);

   Memc<Int>       face;
   Mems<Bool>  tri_done;  tri_done.setNumZero(mesh. tris());
   Mems<Bool> quad_done; quad_done.setNumZero(mesh.quads());

   REPA(mesh.tri)if(!tri_done[i])
   {
      face.add(i); tri_done[i]=true;
      for(Int last=0; last!=face.elms(); )
      {
         Int new_last=face.elms();
         for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
         {
            VecI4 a;
            Int   f=face[i];
            if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
            else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

            REPA(a) // iterate through all adjacent faces
            {
               Int af =a.c[i];
               if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
               {
                  if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                  else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
               }
            }
         }
         last=new_last;
      }
      SetRandomColor(mesh, face, Color(Random(random, 255), Random(random, 255), Random(random, 255)));
      face.clear();
   }
   REPA(mesh.quad)if(!quad_done[i])
   {
      face.add(i^SIGN_BIT); quad_done[i]=true;
      for(Int last=0; last!=face.elms(); )
      {
         Int new_last=face.elms();
         for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
         {
            VecI4 a;
            Int   f=face[i];
            if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
            else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

            REPA(a) // iterate through all adjacent faces
            {
               Int af =a.c[i];
               if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
               {
                  if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                  else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
               }
            }
         }
         last=new_last;
      }
      SetRandomColor(mesh, face, Color(Random(random, 255), Random(random, 255), Random(random, 255)));
      face.clear();
   }
}
/******************************************************************************/
static void SetRandomBend(MeshBase &mesh, Memc<Int> &face, Flt value)
{
   REPA(face)
   {
      Int f=face[i];
      if( f&SIGN_BIT){VecI4 q=mesh.quad.ind(f^SIGN_BIT); REPA(q)mesh.vtx.size(q.c[i])=value;}
      else           {VecI  t=mesh.tri .ind(f         ); REPA(t)mesh.vtx.size(t.c[i])=value;}
   }
}
static void SetRandomBend(MeshBase &mesh)
{
   mesh.setVtxDup().setAdjacencies(true, false).include(VTX_SIZE);

   Memc<Int>       face;
   Mems<Bool>  tri_done;  tri_done.setNumZero(mesh. tris());
   Mems<Bool> quad_done; quad_done.setNumZero(mesh.quads());

   REPA(mesh.tri)if(!tri_done[i])
   {
      face.add(i); tri_done[i]=true;
      for(Int last=0; last!=face.elms(); )
      {
         Int new_last=face.elms();
         for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
         {
            VecI4 a;
            Int   f=face[i];
            if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
            else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

            REPA(a) // iterate through all adjacent faces
            {
               Int af =a.c[i];
               if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
               {
                  if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                  else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
               }
            }
         }
         last=new_last;
      }
      SetRandomBend(mesh, face, Random.f(LEAF_RANDOM_BEND_RANGE));
      face.clear();
   }
   REPA(mesh.quad)if(!quad_done[i])
   {
      face.add(i^SIGN_BIT); quad_done[i]=true;
      for(Int last=0; last!=face.elms(); )
      {
         Int new_last=face.elms();
         for(Int i=face.elms(); --i>=last; ) // search recently added, order is important because face will receive new elements continuously
         {
            VecI4 a;
            Int   f=face[i];
            if(   f&SIGN_BIT)a=    mesh.quad.adjFace(f^SIGN_BIT)     ; // get all adjacent faces
            else             a.set(mesh. tri.adjFace(f         ), -1); // get all adjacent faces

            REPA(a) // iterate through all adjacent faces
            {
               Int af =a.c[i];
               if( af!=-1) // if adjacent face exists, compare to -1 and not >=0 because it can have SIGN_BIT
               {
                  if(af&SIGN_BIT){if(!quad_done[af^SIGN_BIT]){quad_done[af^SIGN_BIT]=true; face.add(af);}}
                  else           {if(! tri_done[af         ]){ tri_done[af         ]=true; face.add(af);}}
               }
            }
         }
         last=new_last;
      }
      SetRandomBend(mesh, face, Random.f(LEAF_RANDOM_BEND_RANGE));
      face.clear();
   }
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
     MeshPart::Variation::Variation() {zero();}
void MeshPart::Variation::zero     () {last_solid_instance=last_shadow_instance=-1; REPAO(shader)=null; frst=null; blst=null;}
void MeshPart::Variation::del      () {material.clear(); zero();}
Bool MeshPart::Variation::save     (File &f, CChar *path)C {f.putAsset(material.id());              return f.ok();}
Bool MeshPart::Variation::load     (File &f, CChar *path)  {material.require(f.getAssetID(), path); return f.ok();}
/******************************************************************************/
     MeshPart::MeshPart() {zero();}
void MeshPart::zero    ()
{
   name[0]=0;
   part_flag=0;
  _vtx_heightmap=0;
  _draw_mask=IndexToFlag(0);
  _draw_mask_enum_id=0;
  _umm=null;
}
MeshPart& MeshPart::del()
{
          base      .del  ();
          render    .del  ();
         _variation .del  ();
         _variations.del  ();
   REPAO(_materials).clear();
   zero(); return T;
}
MeshPart& MeshPart::create(C MeshPart &src, UInt flag_and)
{
   if(this==&src)base.keepOnly(flag_and);else
   {
      del();

      copyParams(src, true);
      base.create(src.base, flag_and);

      // if creating from 'render' fails (for example we're on OpenGL ES and we can't read from GPU data) then try creating from MeshBase using 'setRender'
      flag_and|=VTX_POS;
      UInt src_render_flag=(src.render.flag()&VTX_MSHR),
           new_render_flag=(src_render_flag  &flag_and);
      if(  src_render_flag==new_render_flag)
      {
         if(!render.create(src.render))setRender();
      }else
      {
       C MeshRender *src_render[]={&src.render};
         if(render.create(src_render, Elms(src_render), flag_and))setShader(0);else setRender();
      }
   }
   return T;
}
void MeshPart::copyParams(C MeshPart &src, Bool copy_shaders)
{
   if(this!=&src)
   {
      Set( name        ,src. name);
     _draw_mask        =src._draw_mask;
     _draw_mask_enum_id=src._draw_mask_enum_id;
      part_flag        =src. part_flag;
     _vtx_heightmap    =src._vtx_heightmap;
     _umm              =src._umm;
      REPAO(_materials)=src._materials[i];
     _variation        =src._variation ;       _variation  .unlink();
     _variations       =src._variations; REPAO(_variations).unlink();
      if(!copy_shaders) // clear shaders
      {
               _variation  .zero();
         REPAO(_variations).zero();
      }
   }
}
void MeshPart::scaleParams(Flt scale)
{
   if(scale)_vtx_heightmap/=scale;
}
MeshPart& MeshPart::include (UInt flag) {                                     Bool base_is=base.is(); if(!base_is && render.is())base.create(render); base.include(flag);                                                     return T;} // don't create 'render' from uninitialized data
MeshPart& MeshPart::exclude (UInt flag) {if((base.flag()|render.flag())&flag){Bool base_is=base.is(); if(!base_is && render.is())base.create(render); base.exclude(flag); if(render.is())setRender(); if(!base_is)delBase();} return T;}
MeshPart& MeshPart::keepOnly(UInt flag) {return exclude(~flag);}
/******************************************************************************/
// GET
/******************************************************************************/
UInt MeshPart::flag     (           )C {return base.is() ? base.flag     () : render.flag();}
UInt MeshPart::memUsage (           )C {return base.memUsage()              + render.memUsage();}
Int  MeshPart::vtxs     (           )C {return base.is() ? base.vtxs     () : render.vtxs();}
Int  MeshPart::edges    (           )C {return             base.edges    ()                ;}
Int  MeshPart::tris     (           )C {return base.is() ? base.tris     () : render.tris();}
Int  MeshPart::quads    (           )C {return             base.quads    ()                ;}
Int  MeshPart::faces    (           )C {return base.is() ? base.faces    () : render.tris();}
Int  MeshPart::trisTotal(           )C {return base.is() ? base.trisTotal() : render.tris();}
Bool MeshPart::getBox   (Box &box   )C {return base.getBox(box) || render.getBox(box);}
Flt  MeshPart::area     (Vec *center)C {return base.is() ? base.area(center) : render.area(center);}
Int  MeshPart::drawGroup(           )C {return _draw_mask ? BitHi(_draw_mask) : -1;}

C MaterialPtr& MeshPart::multiMaterial(Int i)C
{
   if(!i)return _variation.material;
   if(InRange(--i, _materials))return _materials[i];
   return MaterialNull;
}
Bool MeshPart::sameMaterials(C MeshPart &part)C
{
   if( material ( )==part. material ( )
   && _materials[0]==part._materials[0]
   && _materials[1]==part._materials[1]
   && _materials[2]==part._materials[2])
   {
      Int shared_vars=Min(_variations.elms(), part._variations.elms());
      REP(shared_vars)                                      if(     _variations[i].material!=part._variations[i].material  )return false; // check variations which both parts have
      for(Int i=shared_vars; i<     _variations.elms(); i++)if(     _variations[i].material!=                    material())return false; // check extra variations in this  part, if it's not default, then fail
      for(Int i=shared_vars; i<part._variations.elms(); i++)if(part._variations[i].material!=                    material())return false; // check extra variations in other part, if it's not default, then fail
      return true;
   }
   return false;
}
/******************************************************************************/
// SET
/******************************************************************************/
MeshPart& MeshPart::remapMaterials(Byte new_index[4])
{
   if(new_index[0]!=0
   || new_index[1]!=1
   || new_index[2]!=2
   || new_index[3]!=3)
   {
      // materials
      MaterialPtr temp[4]={multiMaterial(0), multiMaterial(1), multiMaterial(2), multiMaterial(3)};
     _variation.material    =temp[new_index[0]];
               _materials[0]=temp[new_index[1]];
               _materials[1]=temp[new_index[2]];
               _materials[2]=temp[new_index[3]];

      // base
      if(base.vtx.material())REPA(base.vtx)
      {
         VecB4 &material=base.vtx.material(i), temp=material;
         material.c[0]=temp.c[new_index[0]];
         material.c[1]=temp.c[new_index[1]];
         material.c[2]=temp.c[new_index[2]];
         material.c[3]=temp.c[new_index[3]];
      }

      // render
      Int material_ofs =render.vtxOfs(VTX_MATERIAL);
      if( material_ofs>=0)if(Byte *vtx=render.vtxLock())
      {
         vtx+=material_ofs;
         REP(render.vtxs())
         {
            VecB4 &material=*(VecB4*)vtx, temp=material;
            material.c[0]=temp.c[new_index[0]];
            material.c[1]=temp.c[new_index[1]];
            material.c[2]=temp.c[new_index[2]];
            material.c[3]=temp.c[new_index[3]];
            vtx+=render.vtxSize();
         }
         render.vtxUnlock();
      }
      setUMM();
   }
   return T;
}
MeshPart& MeshPart::setUMM()
{
  _umm=(multiMaterial(1) ? UniqueMultiMaterialMap(UniqueMultiMaterialKey(multiMaterial(0)(), multiMaterial(1)(), multiMaterial(2)(), multiMaterial(3)())) : null);
   return T;
}
MeshPart& MeshPart::setRenderEx(Bool optimize, Bool compress, Int lod_index)
{
   render.create(base, ~0, optimize, compress);
   setShader(lod_index);
   return T;
}
MeshPart& MeshPart::delBase  (                            ) {base  .del(); return T;}
MeshPart& MeshPart::delRender(                            ) {render.del(); return T;}
MeshPart& MeshPart::setBase  (Bool only_if_empty          ) {if(only_if_empty ? !base.is() : true)base.create(render); return T;}
MeshPart& MeshPart::setRender(Bool optimize, Int lod_index) {return setRenderEx(optimize, true, lod_index);}

void MeshPart::setShaderMulti(Int lod_index)
{
 C Material *m[]=
   {
      multiMaterial(0)(),
      multiMaterial(1)(),
      multiMaterial(2)(),
      multiMaterial(3)(),
   };
   DefaultShaders(m, render.flag(), lod_index, heightmap()).set(_variation.shader, &_variation.frst, &_variation.blst);
}
void MeshPart::setShader(Int lod_index, Variation &variation)
{
   DefaultShaders(variation.material(), render.flag(), lod_index, heightmap()).set(variation.shader, &variation.frst, &variation.blst);
}
MeshPart& MeshPart::setShader(Int lod_index)
{
   if(!D._set_shader_material
   || multiMaterial(0)==D._set_shader_material || multiMaterial(1)==D._set_shader_material
   || multiMaterial(2)==D._set_shader_material || multiMaterial(3)==D._set_shader_material) // update only for changed material
      setShaderMulti(lod_index);
   REPA(_variations)
   {
      Variation &variation=_variations[i];
      if(!D._set_shader_material || variation.material==D._set_shader_material)setShader(lod_index, variation);
   }
   return T;
}
MeshPart& MeshPart::setShader(RENDER_MODE mode, Shader *shader, Int variation)
{
   if(InRange(mode, MEMBER_ELMS(Variation, shader)))
   {
      Variation *var;
      if(         !variation              )var=&_variation            ;else
      if(InRange(--variation, _variations))var=&_variations[variation];else goto error;
      var->shader[mode]=shader;
   }
error:
   return T;
}
MeshPart& MeshPart::material(C MaterialPtr &material, Int lod_index)
{
   if(_variation.material    !=material
   ||           _materials[0]!=null
   ||           _materials[1]!=null
   ||           _materials[2]!=null)
   {
   #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
     _variation.unlink();
   #else
      if(_variation.drawn())Exit("Changing Mesh Material after it was already requested to be drawn with a different Material is not supported");
   #endif
     _variation.material    =material;
               _materials[0]=null;
               _materials[1]=null;
               _materials[2]=null;
      setUMM();
      if(lod_index>=0)setShaderMulti(lod_index); // set shader only if valid lod specified (negative can be used for special case when we don't want to set shaders yet, for example because they may require locking D._lock)
   }
   return T;
}
MeshPart& MeshPart::multiMaterial(C MaterialPtr &m0, C MaterialPtr &m1, C MaterialPtr &m2, C MaterialPtr &m3, Int lod_index)
{
   if(_variation.material    !=m0
   ||           _materials[0]!=m1
   ||           _materials[1]!=m2
   ||           _materials[2]!=m3)
   {
   #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
     _variation.unlink();
   #else
      if(_variation.drawn())Exit("Changing Mesh Material after it was already requested to be drawn with a different Material is not supported");
   #endif
     _variation.material    =m0;
               _materials[0]=m1;
               _materials[1]=m2;
               _materials[2]=m3;
      setUMM();
      if(lod_index>=0)setShaderMulti(lod_index); // set shader only if valid lod specified (negative can be used for special case when we don't want to set shaders yet, for example because they may require locking D._lock)
   }
   return T;
}
MeshPart& MeshPart::setAutoTanBin()
{
   if(!heightmap()) // heightmaps generate tan/bin from normal in the shader
   REP(variations())
      if(C MaterialPtr &material=variation(i)) // if any of the variation materials need tan/bin
         if(material->wantTanBin())
   {
      Bool base_is=base.is();
      if( !base_is)base.create(render);

      base.setAutoTanBin();

      if(render.is())setRender();
      if( !base_is  )delBase();

      return T;
   }

   exclude(VTX_TAN_BIN); // not wanted
   return T;
}
MeshPart& MeshPart::drawGroup(Int group, Enum *draw_group_enum)
{
   if(InRange(group, 32))
   {
     _draw_mask        =IndexToFlag(group);
     _draw_mask_enum_id=(draw_group_enum ? draw_group_enum->elmIDUInt(group) : 0);
   }else // invalid group, force zero mask, never draw
   {
     _draw_mask=0;
     _draw_mask_enum_id=0;
   }
   return T;
}
/******************************************************************************/
// VARIATIONS
/******************************************************************************/
Int       MeshPart::variations(              )C {return _variations.elms()+1;}
MeshPart& MeshPart::variations(Int variations)
{
   variations=Max(0, variations-1); // 1 variation is stored in MeshPart, others are stored in '_variations'
   Int old=_variations.elms(); _variations.setNum(variations);
   for(Int i=old; i<_variations.elms(); i++)variation(i+1, material()); // set default material for newly added variations
   return T;
}
MeshPart& MeshPart::variation(Int variation, C MaterialPtr &material, Int lod_index)
{
   if(!variation)T.material(material, lod_index);else // 0-th is stored in MeshPart
   if(InRange(--variation, _variations))
   {
      Variation &var=_variations[variation]; if(var.material!=material)
      {
      #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
         var.unlink();
      #else
         if(var.drawn())Exit("Changing Mesh Material after it was already requested to be drawn with a different Material is not supported");
      #endif
         var.material=material;
         if(lod_index>=0)setShader(lod_index, var); // set shader only if valid lod specified (negative can be used for special case when we don't want to set shaders yet, for example because they may require locking D._lock)
      }
   }
   return T;
}
C MaterialPtr& MeshPart::variationNull(Int variation)C
{
   if(!variation)return material();
   if(InRange(--variation, _variations))return _variations[variation].material;
   return MaterialNull;
}
C MaterialPtr& MeshPart::variation(Int variation)C
{
   if(InRange(--variation, _variations))return _variations[variation].material;
   return material();
}
C MeshPart::Variation& MeshPart::getVariation()C
{
   Int i=Renderer._mesh_variation_1; // this is "variation-1"
   return InRange(i, _variations) ? _variations[i] : _variation;
}
C MeshPart::Variation& MeshPart::getVariation1(Int variation_1)C // this is "variation-1"
{
   return InRange(variation_1, _variations) ? _variations[variation_1] : _variation;
}
void MeshPart::variationRemove(Int variation)
{
   if(!variation)
   {
      if(_variations.elms())Swap(_variation, _variations[0]);
     _variations.remove(0, true); // #0 was placed in '_variation'
   }else
   {
     _variations.remove(variation-1, true);
   }
}
void MeshPart::variationKeep(Int variation)
{
   if(InRange(--variation, _variations))Swap(_variation, _variations[variation]);
  _variations.del();
}
void MeshPart::variationMove(Int variation, Int new_index)
{
   MemtN<Variation, 16> variations; variations.setNum(T.variations());
   Swap(variations[0], _variation); REPA(_variations)Swap(variations[i+1], _variations[i]);
   variations.moveElm(variation, new_index);
   Swap(variations[0], _variation); REPA(_variations)Swap(variations[i+1], _variations[i]);
}
void MeshPart::variationRemap(C Mesh &src, C Mesh &dest)
{
   if(&src!=&dest)
   {
      MemtN<Variation, 16> variations; variations.setNum(T.variations());
      Swap(variations[0], _variation); REPA(_variations)Swap(variations[i+1], _variations[i]);
     _variations.setNum(dest.variations()-1);
      REP(dest.variations())
      {
         Int         src_var_i=src.variationFind(dest.variationName(i));
       C Variation & src_var  =(InRange(src_var_i, variations) ? variations[src_var_i] : variations[0]);
         Variation &dest_var  =(i ? _variations[i-1] : _variation);
         dest_var=src_var;
      }
   }
}
/******************************************************************************/
// HEIGHTMAP
/******************************************************************************/
MeshPart& MeshPart::heightmap(Flt tex_scale, Int lod_index)
{
   MAX(tex_scale, 0); if(tex_scale!=_vtx_heightmap)
   {
     _vtx_heightmap=tex_scale;
      if(heightmap())exclude(VTX_TEX_ALL|VTX_TAN_BIN); // if enabled heightmap, then delete tex/tan/bin as they're no longer needed
      setShader(lod_index);
   }
   return T;
}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
MeshPart& MeshPart::texMove  (C Vec2 &move , Byte tex_index) {base.texMove  (move , tex_index); render.texMove  (move , tex_index); return T;}
MeshPart& MeshPart::texScale (C Vec2 &scale, Byte tex_index) {base.texScale (scale, tex_index); render.texScale (scale, tex_index); return T;}
MeshPart& MeshPart::texRotate(  Flt   angle, Byte tex_index) {base.texRotate(angle, tex_index); render.texRotate(angle, tex_index); return T;}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
MeshPart& MeshPart::move         (              C Vec &move) {base.     move(       move); render.scaleMove(VecOne, move);                                return T;}
MeshPart& MeshPart::scale        (C Vec &scale             ) {base.scale    (scale      ); render.scaleMove(scale       ); scaleParams(Abs(scale).max()); return T;}
MeshPart& MeshPart::scaleMove    (C Vec &scale, C Vec &move) {base.scaleMove(scale, move); render.scaleMove(scale , move); scaleParams(Abs(scale).max()); return T;}
MeshPart& MeshPart::scaleMoveBase(C Vec &scale, C Vec &move) {base.scaleMove(scale, move);                                 scaleParams(Abs(scale).max()); return T;}
MeshPart& MeshPart::transform    (C Matrix3 &matrix        )
{
   Bool base_is=base.is();
   if(! base_is)base.create(render);

   base.transform(matrix);
   scaleParams   (matrix.maxScale());

   if(render.is())render.create(base);
   if(! base_is  )delBase();
   return T;
}
MeshPart& MeshPart::transform(C Matrix &matrix)
{
   Bool base_is=base.is();
   if(! base_is)base.create(render);

   base.transform(matrix);
   scaleParams   (matrix.maxScale());

   if(render.is())render.create(base);
   if(! base_is  )delBase();
   return T;
}
MeshPart& MeshPart::animate(C MemPtrN<Matrix, 256> &matrixes)
{
   if(matrixes.elms())
   {
      Bool base_is=base.is();
      if(! base_is)base.create(render);

      base.animate(matrixes);
      scaleParams (matrixes[0].maxScale());

      if(render.is())render.create(base);
      if(! base_is  )delBase();
   }
   return T;
}
MeshPart& MeshPart::animate(C AnimatedSkeleton &skel)
{
   Bool base_is=base.is();
   if(! base_is)base.create(render);

   base.animate(skel);
   scaleParams (skel.matrix().maxScale());

   if(render.is())render.create(base);
   if(! base_is  )delBase();
   return T;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
MeshPart& MeshPart::boneRemap(C MemPtr<Byte, 256> &old_to_new)
{
   Bool base_is=base.is();
   if( !base_is)base.create(render);

   base.boneRemap(old_to_new);

   if(render.is())render.create(base);
   if( !base_is  )delBase();
   return T;
}
void MeshPart::    setUsedBones(Bool (&bones)[256])C {Zero(bones); includeUsedBones(bones);}
void MeshPart::includeUsedBones(Bool (&bones)[256])C
{
   if(base.is())base.includeUsedBones(bones);else render.includeUsedBones(bones);
}
MeshPart& MeshPart::freeOpenGLESData() {render.freeOpenGLESData(); return T;}
/******************************************************************************/
MeshPart& MeshPart::setLeafAttachment(C Vec2 &tex)
{
   Bool base_is=base.is();
   if( !base_is && render.is())setBase();

   SetLeafAttachment(base, tex);

   if(render.is())setRender();
   if(!base_is   )delBase();
   return T;
}
MeshPart& MeshPart::setLeafAttachment(C Vec &pos)
{
   Bool base_is=base.is();
   if( !base_is && render.is())setBase();

   base.include(VTX_HLP); REPA(base.vtx)base.vtx.hlp(i)=pos;

   if(render.is())setRender();
   if(!base_is   )delBase();
   return T;
}
MeshPart& MeshPart::setRandomLeafColor(Flt variation)
{
   Bool base_is=base.is();
   if( !base_is && render.is())setBase();

   SetRandomColor(base, variation);

   if(render.is())setRender();
   if(!base_is   )delBase();
   return T;
}
MeshPart& MeshPart::setRandomLeafBending()
{
   Bool base_is=base.is();
   if( !base_is && render.is())setBase();

   SetRandomBend(base);

   if(render.is())setRender();
   if(!base_is   )delBase();
   return T;
}
MeshPart& MeshPart::setRandomLeafBending(Flt random_value)
{
   Bool base_is=base.is();
   if( !base_is && render.is())setBase();

   base.include(VTX_SIZE); REPA(base.vtx)base.vtx.size(i)=random_value;

   if(render.is())setRender();
   if(!base_is   )delBase();
   return T;
}
MeshPart& MeshPart::delRandomLeafBending() {exclude(VTX_SIZE); return T;}
/******************************************************************************/
MeshPart& MeshPart::operator+=(C MeshPart &src)
{
   base  +=src.base  ;
   render+=src.render;
   if(!(src.part_flag&MSHP_HIDDEN))FlagDisable(part_flag, MSHP_HIDDEN); // unhide part if 'src' is visible
   for(Int i=_variations.elms(); i<src._variations.elms(); i++)_variations.add(src._variations[i]); // add variations available in 'src' but not in 'this'
   return T;
}
/******************************************************************************/
void MeshPart::unlinkSolid ()C {_variation.unlinkSolid (); REPAO(_variations).unlinkSolid ();}
void MeshPart::unlinkShadow()C {_variation.unlinkShadow(); REPAO(_variations).unlinkShadow();}
void MeshPart::unlink      ()C {_variation.unlink      (); REPAO(_variations).unlink      ();}
/******************************************************************************/
}
/******************************************************************************/
