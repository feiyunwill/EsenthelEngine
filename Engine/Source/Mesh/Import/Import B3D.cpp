/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace B3D{ // so local structures are unique
/******************************************************************************/
#define BLENDER 1 // 1 for gandaldf's exporter, 0 for milkshape
#if DEBUG
   #define LOG  0
#else
   #define LOG  0
#endif
/******************************************************************************/
struct NODE;
/******************************************************************************/
#pragma pack(push, 1)
struct Chunk
{
   Char8 c[4];
   UInt  size;
   
   Bool equal(CChar8 *name)C {return c[0]==name[0] && c[1]==name[1] && c[2]==name[2] && c[3]==name[3];}
   Str8 asStr(            )C {Str8 s; FREPA(c)s+=c[i]; return s;}
};
struct BONE
{
   Int vertex_id;
   Flt weight;
};
struct FRAME
{
   Int frame;
   Vec pos, scale;
   Quaternion quat;

   FRAME() {frame=0; pos.zero(); scale=1; quat.identity();}
};
struct ANIM
{
   Int flags, frames;
   Flt fps;

   ANIM() {flags=0; frames=0; fps=60;}
};
struct F_TEXS
{
   UInt flags, blend;
   Vec2 pos  , scale;
   Flt  rotate;
};
#pragma pack(pop)
struct TEXS
{
   Bool has_alpha, second_tex;
   Str8 file;
   Vec2 pos, scale;
   Flt  rotate;
}; 
struct BRUS
{
   Bool      cull, full_bright;
   Str8      name;
   Vec4      color;
   Flt       specular;
   Memc<Int> tex_id;
};
struct VRT
{
   Vec               pos, nrm;
   Vec4              color;
   Vec2              tex[8];
   Memc<IndexWeight> skin;
};
struct TRIS
{
   Int        brush;
   Memc<VecI> ind;
};
struct MESH_BONE
{
   Byte  parent;
   NODE *node;

   MESH_BONE() {parent=0xFF; node=null;}
};
struct MESH
{
   Byte            has_nrm, has_color, tex_coords;
   Int             brush;
   Memc<VRT      > vrt  ;
   Memc<TRIS     > tris ;
   Memc<MESH_BONE> bones;

   MESH() {has_nrm=has_color=false; tex_coords=0;}
};
struct KEYS
{
   Bool has_pos, has_scale, has_rot;
   Memc<FRAME> frames;

   KEYS() {has_pos=has_scale=has_rot=false;}
};
struct NODE
{
   Str        name;
   Bool       bone;
   Byte       bone_index;
   Matrix     local_matrix, global_matrix;
   NODE      *parent;
   Memc<MESH> meshes;
   Memc<BONE> bones ;
   Memc<KEYS> keys  ;
   Memc<ANIM> anims ;
   Memx<NODE> nodes ; // use 'Memx' because nodes are referencing each other using 'parent'

   NODE() {bone=false; bone_index=0xFF; local_matrix.identity(); global_matrix.identity(); parent=null;}
};
/******************************************************************************/
static Str8 GetStr(File &f)
{
   for(Str8 s; ; )
   {
      Char8 c=f.getByte(); if(!c)return s;
      s+=c;
   }
}
/******************************************************************************/
static void ImportB3DNode(File &f, Chunk &c, Memx<NODE> &nodes, NODE *parent, TextData &log)
{
   c.size+=f.pos();

   NODE &node =nodes.New();
   node.name  =GetStr(f);
   node.parent=parent;
   Vec        pos, scale;
   Quaternion quat;
   f>>pos>>scale>>quat.w>>quat.xyz;
   node. local_matrix.orn()=Matrix3(quat).scale(scale);
   node. local_matrix.pos  =pos;
   node.global_matrix=node.local_matrix; if(parent)node.global_matrix*=parent->global_matrix;

   if(LOG)
   {
      /*log.putLine(S+"name=\""+node.name+"\", local_m.x="+node.local_matrix.x
                                         +", local_m.y="+node.local_matrix.y
                                         +", local_m.z="+node.local_matrix.z);*/
   }
   for(; !f.end() && c.size>f.pos(); )
   {
      Chunk c; f>>c; //if(LOG)log.putLine(c.asStr());
      if(c.equal("NODE"))
      {
         ImportB3DNode(f, c, node.nodes, &node, log);
      }else
      if(c.equal("MESH"))
      {
         c.size+=f.pos();
         MESH &mesh=node.meshes.New(); f>>mesh.brush;
         for(; !f.end() && c.size>f.pos(); )
         {
            Chunk c; f>>c;
            if(c.equal("VRTS"))
            {
               c.size+=f.pos();
               Int flags, tex_coord_sets, tex_coord_set_size; f>>flags>>tex_coord_sets>>tex_coord_set_size;
               mesh.has_nrm   =FlagTest(flags, 1);
               mesh.has_color =FlagTest(flags, 2);
               mesh.tex_coords=tex_coord_sets;
               for(; !f.end() && c.size>f.pos(); )
               {
                  VRT &v=mesh.vrt.New();
                                    f>>v.pos  ;
                  if(mesh.has_nrm  )f>>v.nrm  ;
                  if(mesh.has_color)f>>v.color;
                  FREPD(i, tex_coord_sets    )
                  FREPD(j, tex_coord_set_size)
                  {
                     Flt t; f>>t;
                     if(i<Elms(v.tex) && j<2)v.tex[i].c[j]=t;
                  }
               }
            }else
            if(c.equal("TRIS"))
            {
               c.size+=f.pos();
               TRIS &tris=mesh.tris.New(); f>>tris.brush;
               for(; !f.end() && c.size>f.pos(); )f>>tris.ind.New(); // range checks are done later
            }else
            {
               f.skip(c.size); // skip chunk
            }
         }
      }else
      if(c.equal("BONE"))
      {
         node.bone=true;
         c.size+=f.pos();
         for(; !f.end() && c.size>f.pos(); )f>>node.bones.New();
      }else
      if(c.equal("KEYS"))
      {
         c.size+=f.pos();
         KEYS &key=node.keys.New();
         Int flags; f>>flags;
         key.has_pos  =FlagTest(flags, 1);
         key.has_scale=FlagTest(flags, 2);
         key.has_rot  =FlagTest(flags, 4);
         for(; !f.end() && c.size>f.pos(); )
         {
            FRAME &frame=key.frames.New();
            f>>frame.frame;
            if(key.has_pos  )f>>frame.pos  ;
            if(key.has_scale)f>>frame.scale;
            if(key.has_rot  )f>>frame.quat.w>>frame.quat.xyz;
         }
      }else
      if(c.equal("ANIM"))
      {
         f>>node.anims.New();
      }else
      {
         f.skip(c.size); // skip chunk
      }
   }
}
/******************************************************************************/
static void ProcessBones(Memx<NODE> &nodes, Byte parent_index, MESH *last_mesh)
{
   FREPA(nodes)
   {
      NODE &node=nodes[i];
      if(node.meshes.elms())last_mesh=&node.meshes.first();
      node.bone_index=parent_index;
      if(last_mesh && node.bone)
      {
         node.bone_index=last_mesh->bones.elms();
         MESH_BONE   &mb=last_mesh->bones.New ();
         mb.parent= parent_index;
         mb.node  =&node;
      }
      ProcessBones(node.nodes, node.bone_index, last_mesh);
   }
}
/******************************************************************************/
static void ProcessNodes(Memx<NODE> &nodes, Memc<MeshPart> &parts, MemPtr<Int> part_material_index, Memc<TEXS> &texs, Memc<BRUS> &brus, Skeleton &skeleton, MemtN<Byte, 256> &old_to_new)
{
   FREPA(nodes)
   {
      NODE &node=nodes[i]; Matrix3 matrix3=node.global_matrix; matrix3.normalize();
      FREPA(node.meshes)
      {
         MESH &mesh=node.meshes[i];
         if(mesh.vrt.elms())FREPA(mesh.tris)
         {
            TRIS &tris=mesh.tris[i]; if(tris.ind.elms())
            {
               MeshPart &part=parts.New(); Set(part.name, node.name); if(part_material_index)part_material_index.add(tris.brush);
               MeshBase &mshb=part .base;
               mshb.create(mesh.vrt.elms(), 0, tris.ind.elms(), 0, (mesh.has_nrm?VTX_NRM:0)|((mesh.tex_coords>=1)?VTX_TEX0:0)|((mesh.tex_coords>=2)?VTX_TEX1:0)|(mesh.bones.elms()?VTX_SKIN:0));

               // vertexes
                                     REPA(mesh.vrt)mshb.vtx.pos (i)=mesh.vrt[i].pos*node.global_matrix ;
               if(mesh.has_nrm      )REPA(mesh.vrt)mshb.vtx.nrm (i)=mesh.vrt[i].nrm*            matrix3;
               if(mesh.tex_coords>=1)REPA(mesh.vrt)mshb.vtx.tex0(i)=mesh.vrt[i].tex[0];
               if(mesh.tex_coords>=2)REPA(mesh.vrt)mshb.vtx.tex1(i)=mesh.vrt[i].tex[1];
               if(mesh.bones.elms() )
               {
                  REPA(mesh.bones)
                  {
                     MESH_BONE &mb=mesh.bones[i];
                     NODE      &node=*mb.node;
                     if(InRange(node.bone_index, old_to_new))
                     {
                        Int matrix=old_to_new[node.bone_index];
                        FREPA(node.bones)
                        {
                         C BONE &bone=node.bones[i];
                           if(InRange(bone.vertex_id, mesh.vrt))mesh.vrt[bone.vertex_id].skin.New().set(matrix, bone.weight);
                        }
                     }
                  }
                  REPA(mesh.vrt)SetSkin(mesh.vrt[i].skin, mshb.vtx.matrix(i), mshb.vtx.blend(i), &skeleton);
               }

               // triangles
               Bool invalid=false;
               FREPA(tris.ind)
               {
                  VecI &ind=mshb.tri.ind(i); ind=tris.ind[i]; REPA(ind)if(!InRange(ind.c[i], mshb.vtx)){ind.zero(); invalid=true; break;}
               }
               if(invalid)mshb.removeDegenerateFaces(0);

               mshb.removeUnusedVtxs();
               if(!mshb.vtx.nrm())mshb.setNormals();

               if(Memc<Int> *tex_id=(InRange(tris.brush, brus) ? &brus[tris.brush].tex_id : null))
               {
                  TEXS *tx[2]=
                  {
                     (tex_id->elms()>=1) ? &texs[(*tex_id)[0]] : null,
                     (tex_id->elms()>=2) ? &texs[(*tex_id)[1]] : null,
                  };
                  switch(tex_id->elms())
                  {
                     case 1: if(tx[0]->second_tex && mshb.vtx.tex1()   )Swap(mshb.vtx._tex0, mshb.vtx._tex1); break;
                     case 2: if(tx[0]->second_tex && !tx[1]->second_tex)Swap(mshb.vtx._tex0, mshb.vtx._tex1); break;
                  }
               }
            }
         }
      }
      ProcessNodes(node.nodes, parts, part_material_index, texs, brus, skeleton, old_to_new);
   }
}
/******************************************************************************/
static void CreateAnimation(C Memc<MESH_BONE> &mesh_bone, Skeleton &skeleton, XAnimation &animation)
{
   Animation &anim=animation.anim;
   anim.bones.setNum(mesh_bone.elms());
   FREPA(mesh_bone)
   {
    C NODE     &node=   *mesh_bone [i].node;
      AnimBone &abon=    anim.bones[i];
      SkelBone &sbon=skeleton.bones[i];
      
      Bool    parent=(sbon.parent!=0xFF);
      Matrix3 parent_matrix_inv; if(parent)skeleton.bones[sbon.parent].inverse(parent_matrix_inv);

      abon.set(sbon.name);

      REPA(node.keys)if(node.keys[i].has_rot)
      {
       C Memc<FRAME> &frames=node.keys[i].frames;
         abon.orns.setNumZero(frames.elms());

         FREPA(frames)
         {
          C FRAME         &frame=frames   [i];
            AnimBone::Orn &orn  =abon.orns[i];

            // set time
            orn.time=frame.frame;
            anim.length(Max(anim.length(), orn.time), false);

            // set rotation
            orn.orn=sbon;

            // animate
            {
               if(node.parent)orn.orn*=~node.parent->global_matrix.orn()                      ; // transform bone from global space to local space relative to parent
                              orn.orn*=~node.         local_matrix.orn() * Matrix3(frame.quat); // perform transformation from source to target matrix, looks like 'frame' is the target matrix
               if(node.parent)orn.orn*= node.parent->global_matrix.orn()                      ; // transform bone from local space to global space
            }

            //
            if(parent)orn.orn*=parent_matrix_inv;
         }

         break;
      }

      FREPA(node.keys)if(node.keys[i].has_pos)
      {
       C Memc<FRAME> &frames=node.keys[i].frames;
         abon.poss.setNumZero(frames.elms());

         FREPA(frames)
         {
          C FRAME         &frame=frames   [i];
            AnimBone::Pos &pos  =abon.poss[i];

            // set time
            pos.time=frame.frame;
            anim.length(Max(anim.length(), pos.time), false);

            // set position
            pos.pos=frame.pos-node.local_matrix.pos;
            if(node.parent)pos.pos*=node.parent->global_matrix.orn();
            if(parent     )pos.pos*=parent_matrix_inv;
         }

         break;
      }

      abon.sortFrames();
   }

   // scale length
   if(mesh_bone.elms())for(NODE *node=mesh_bone[0].node; node; node=node->parent)
   {
      if(node->anims.elms())
      {
         ANIM &src=node->anims.first();
         anim.length(Max(anim.length(), (Flt)src.frames)   , false);
         anim.length(anim.length()/(src.fps ? src.fps : 60), true );
         animation.fps=(src.fps ? src.fps : 60);
         break;
      }
   }

   // process
   anim.setTangents().removeUnused().setRootMatrix();
}
/******************************************************************************/
static Bool CreateSkeleton(C Memx<NODE> &nodes, Skeleton &skeleton, XAnimation *animation)
{
   FREPA(nodes)
   {
    C NODE &node=nodes[i]; FREPA(node.meshes)
      {
       C MESH &mesh=node.meshes[i]; if(mesh.bones.elms())
         {
            skeleton.bones.setNum(mesh.bones.elms());
            FREPA(mesh.bones)
            {
             C MESH_BONE &src =mesh    .bones[i];
               SkelBone  &dest=skeleton.bones[i];

               Set(dest.name, src.node->name);
                   dest.pos = src.node->global_matrix.pos;
            #if BLENDER
                   dest.dir = !src.node->global_matrix.y;
                   dest.perp=-!src.node->global_matrix.z;
            #else
                   dest.dir =!src.node->global_matrix.x;
                   dest.perp=!src.node->global_matrix.y;
            #endif
                   dest.parent= src.parent;
            }
            skeleton.setBoneLengths();
            if(animation)CreateAnimation(mesh.bones, skeleton, *animation);
            return true;
         }
      }
      if(CreateSkeleton(node.nodes, skeleton, animation))return true;
   }
   return false;
}
/******************************************************************************/
} // namespace B3D
/******************************************************************************/
Bool ImportB3D(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh     )mesh     ->del();
   if(skeleton )skeleton ->del();
   if(animation)animation->del();
   materials          .clear();
   part_material_index.clear();

#if !WEB // structs are unaligned, would need using 'Unaligned' func
   using namespace B3D;

   Memc<TEXS> texs;
   Memc<BRUS> brus;
   Memx<NODE> node;

   File f; if(f.readTry(name))
   {
      Chunk c; f>>c; if(c.equal("BB3D"))
      {
         TextData log; if(LOG)log.save(name+".txt");
         UInt ver; f>>ver;
         for(; !f.end(); )
         {
            f>>c; //if(LOG)log.putLine(c.asStr());
            if(c.equal("TEXS"))
            {
               c.size+=f.pos();
               for(; !f.end() && c.size>f.pos(); )
               {
                  F_TEXS f_t;
                  TEXS  &t=texs.New(); t.file=GetStr(f); f>>f_t; t.pos=f_t.pos; t.scale=f_t.scale; t.rotate=f_t.rotate;
                  t.has_alpha =FlagTest(f_t.flags,     2);
                  t.second_tex=FlagTest(f_t.flags, 65536);
                //LogN(S+"T "+f_t.flags+' '+f_t.blend+' '+t.file);
               }
            }else
            if(c.equal("BRUS"))
            {
               c.size+=f.pos();
               Int n_texs=f.getInt();
               for(; !f.end() && c.size>f.pos(); )
               {
                  BRUS &b=brus.New(); b.name=GetStr(f); Int blend, fx; f>>b.color>>b.specular>>blend>>fx; REP(n_texs){Int tex_id; f>>tex_id; if(InRange(tex_id, texs))b.tex_id.add(tex_id);}
                  b.cull       =!FlagTest(fx, 16);
                  b.full_bright= FlagTest(fx,  1);
                //Log(S+"B "+blend+' '+fx+' '+b.name); FREPA(b.tex_id)Log(S+' '+texs[b.tex_id[i]].file); LogN();
               }
            }else
            if(c.equal("NODE"))
            {
               ImportB3DNode(f, c, node, null, log);
            }else
            {
               f.skip(c.size); // skip chunk
            }
            if(!f.ok())return false;
         }

         // finalize
         Skeleton temp, *skel=(skeleton ? skeleton : (mesh || animation) ? &temp : null); // if skel not specified, but we want mesh or animation, then we have to process it
         if(skel)
         {
            MemtN<Byte, 256> old_to_new;
            ProcessBones  (node, 0xFF, null);
            CreateSkeleton(node, *skel, animation);
            skel->sortBones(old_to_new).setBoneTypes(); if(VIRTUAL_ROOT_BONE)REPAO(old_to_new)++;
            if(animation)animation->anim.setBoneTypeIndexesFromSkeleton(*skel);
            if(mesh)
            {
               Memc<MeshPart> parts;
               ProcessNodes(node, parts, part_material_index, texs, brus, *skel, old_to_new);
               mesh->create(parts.elms()); FREPA(*mesh)Swap(mesh->parts[i], parts[i]);
               mesh->skeleton(skel).skeleton(null).setBox(); CleanMesh(*mesh);
            }
            if(skeleton){skel->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}
         }

         // materials
         if(materials)FREPA(brus)
         {
            BRUS      &b       =brus[i];
            XMaterial &material=materials.New();
            material.name    =b.name;
            material.cull    =b.cull;
            material.color   =b.color;
            material.specular=b.specular;
            material.ambient =b.full_bright;
            if(b.tex_id.elms()>=1)material.color_map=texs[b.tex_id[0]].file;
            if(b.tex_id.elms()>=2)material.light_map=texs[b.tex_id[1]].file;
            if(b.tex_id.elms()>=1 && texs[b.tex_id[0]].has_alpha)
            {
               material.technique=MTECH_ALPHA_TEST;
               material.color.w  =0.5f;
            }
            material.fixPath(GetPath(name));
         }
         return true;
      }
   }
#endif
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
