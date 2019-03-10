/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CHUNK3DS_MAIN3DS           0x4D4D
#define CHUNK3DS_EDIT3DS           0x3D3D
#define CHUNK3DS_EDIT_OBJECT       0x4000
#define CHUNK3DS_OBJ_TRIMESH       0x4100
#define CHUNK3DS_TRI_VERTEXLIST    0x4110
#define CHUNK3DS_TRI_FACELIST      0x4120
#define CHUNK3DS_TRI_MTRLLIST      0x4130
#define CHUNK3DS_TRI_TEXCOORD      0x4140
#define CHUNK3DS_TRI_SMOOTH_GROUPS 0x4150
#define CHUNK3DS_TRI_MATRIX        0x4160
#define CHUNK3DS_MTRL              0xAFFF
#define CHUNK3DS_MTRL_NAME         0xA000
#define CHUNK3DS_MTRL_COLOR        0xA020
#define CHUNK3DS_MTRL_COLOR_MAP    0xA200
#define CHUNK3DS_MTRL_SPEC_MAP     0xA204
#define CHUNK3DS_MTRL_ALPHA_MAP    0xA210
#define CHUNK3DS_MTRL_REFL_MAP     0xA220
#define CHUNK3DS_MTRL_BUMP_MAP     0xA230
#define CHUNK3DS_MTRL_TEX_NAME     0xA300
#define CHUNK3DS_MTRL_2_SIDED      0xA081
#define CHUNK3DS_KFDATA            0xB000
#define CHUNK3DS_OBJECT_NODE_TAG   0xB002
#define CHUNK3DS_NODE_HEADER       0xB010
#define CHUNK3DS_PIVOT             0xB013
#define CHUNK3DS_SCALE             0x0100

#define CHUNK3DS_COLOR_RGB_BYTE    0x0011
/******************************************************************************/
Bool Import3DS(C Str &name, Mesh *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh)mesh->del();
   materials          .clear();
   part_material_index.clear();

   #pragma pack(push, 1)
   struct Chunk
   {
      U16 id;
      U32 length;
   };
   #pragma pack(pop)
   struct Mesh : MeshBase
   {
      Memc<UInt> tri_smooth_groups;
      Matrix     matrix;
      Str8       name;

      Mesh() {matrix.identity();}
   };
   struct Node
   {
      Str8 name;
      Vec  pivot;

      Node() {pivot.zero();}
   };

   File f; if(f.readTry(name))
   {
      UInt       last_tex=0;
      Str8       last_name;
      Memc<Mesh> meshes;
      Memc<Node> nodes;
      Flt        scale=1;

      for(; !f.end(); )
      {
         Chunk chunk; f.get(&chunk, SIZE(chunk)); Long end=f.pos()+Unaligned(chunk.length)-SIZE(chunk);
         switch(Unaligned(chunk.id))
         {
            case CHUNK3DS_MAIN3DS        :
            case CHUNK3DS_EDIT3DS        :
            case CHUNK3DS_MTRL           :
            case CHUNK3DS_KFDATA         :
            case CHUNK3DS_OBJECT_NODE_TAG:
            case CHUNK3DS_MTRL_COLOR     : break;

            case CHUNK3DS_SCALE: f>>scale; break;

            case CHUNK3DS_MTRL_COLOR_MAP:
            case CHUNK3DS_MTRL_SPEC_MAP :
            case CHUNK3DS_MTRL_ALPHA_MAP:
            case CHUNK3DS_MTRL_REFL_MAP :
            case CHUNK3DS_MTRL_BUMP_MAP : last_tex=Unaligned(chunk.id); break;

            case CHUNK3DS_EDIT_OBJECT:
            {
               last_name.clear(); for(Char8 c; c=f.getByte(); )last_name+=c;
            }break;

               case CHUNK3DS_OBJ_TRIMESH:
               {
                  Mesh &mesh=meshes.New();
                  mesh.name=last_name;
               }break;

                  case CHUNK3DS_NODE_HEADER:
                  {
                     Node &node=nodes.New(); for(Char8 c; c=f.getByte(); )node.name+=c;
                  }goto skip;

                  case CHUNK3DS_PIVOT: if(Node *node=nodes.addrLast())
                  {
                     f>>node->pivot;
                  }goto skip;

                  case CHUNK3DS_TRI_VERTEXLIST: if(Mesh *mesh=meshes.addrLast())if(!mesh->vtxs())
                  {
                     mesh->vtx._elms=f.getUShort(); mesh->include(VTX_POS); f.getN(mesh->vtx.pos(), mesh->vtxs());
                  }goto skip;

                  case CHUNK3DS_TRI_TEXCOORD: if(Mesh *mesh=meshes.addrLast())if(!mesh->vtx.tex0())
                  {
                     Int vtxs=f.getUShort(); if(vtxs==mesh->vtxs()){mesh->include(VTX_TEX0); f.getN(mesh->vtx.tex0(), mesh->vtxs());}
                  }goto skip;

                  case CHUNK3DS_TRI_FACELIST:
                  {
                     Int tris=f.getUShort(); end=f.pos()+tris*4*2;
                     if(Mesh *mesh=meshes.addrLast())if(!mesh->tris())
                     {
                        mesh->tri._elms=tris; mesh->include(TRI_IND); FREPA(mesh->tri)
                        {
                           U16 ind[4]; f>>ind; mesh->tri.ind(i).set(ind[0], ind[1], ind[2]);
                        }
                     }
                  }goto skip;

                  case CHUNK3DS_TRI_MTRLLIST: if(Mesh *mesh=meshes.addrLast())
                  {
                     Str material_name    ; for(Char8 c; c=f.getByte(); )material_name+=c;
                     Int material_index=-1; if(materials){REPA(materials)if(Equal(materials[i].name, material_name)){material_index=i; break;} if(material_index<0){material_index=materials.elms(); materials.New().name=material_name;}}
                     Int tris          =f.getUShort();
                     if(!mesh->tri.id()){mesh->include(TRI_ID); REPA(mesh->tri)mesh->tri.id(i)=-1;}
                     REP(tris)
                     {
                        Int tri=f.getUShort(); if(InRange(tri, mesh->tri))mesh->tri.id(tri)=material_index;
                     }
                  }goto skip;

                  case CHUNK3DS_TRI_SMOOTH_GROUPS: if(Mesh *mesh=meshes.addrLast()) // a list of UInt's for each triangle, smooth group is a 32-bit UInt value, each bit specifies a smoothing group, if 2 tris share a smoothing group, then the vertex normals should be smoothened
                  {
                     if(!mesh->tri_smooth_groups.elms())
                     {
                        mesh->tri_smooth_groups.setNum(mesh->tris());
                        f.getN(mesh->tri_smooth_groups.data(), mesh->tri_smooth_groups.elms());
                     }
                  }goto skip;

                  case CHUNK3DS_TRI_MATRIX: if(Mesh *mesh=meshes.addrLast())
                  {
                     f>>mesh->matrix.x;
                     f>>mesh->matrix.y;
                     f>>mesh->matrix.z;
                     f>>mesh->matrix.pos;
                  }goto skip;

            case CHUNK3DS_MTRL_NAME:
            {
               Str material_name    ; for(Char8 c; c=f.getByte(); )material_name+=c;
               Int material_index=-1; if(materials){REPA(materials)if(Equal(materials[i].name, material_name)){material_index=i; break;} if(material_index<0){material_index=materials.elms(); materials.New().name=material_name;}}
            }break;

            // not confirmed:
            /*case CHUNK3DS_MTRL_2_SIDED: if(materials.notNull() && materials.elms())materials.last().cull=false; break;
            */

            case CHUNK3DS_MTRL_TEX_NAME:
            {
               Str tex_name; for(Char8 c; c=f.getByte(); )tex_name+=c;
               if(materials.elms())switch(last_tex)
               {
                  case CHUNK3DS_MTRL_COLOR_MAP: materials.last().color_map     =tex_name; break;
                  case CHUNK3DS_MTRL_SPEC_MAP : materials.last().specular_map  =tex_name; break;
                  case CHUNK3DS_MTRL_ALPHA_MAP: materials.last().alpha_map     =tex_name; break;
                  case CHUNK3DS_MTRL_REFL_MAP : materials.last().reflection_map=tex_name; break;
                  case CHUNK3DS_MTRL_BUMP_MAP : materials.last().bump_map      =tex_name; break;
               }
            }break;

            case CHUNK3DS_COLOR_RGB_BYTE:
            {
               Color c; f>>c.r>>c.g>>c.b; c.a=255;
               if(materials.elms())materials.last().color=c.asVec4();
            }break;

            default: skip: f.pos(end); break;
         }
         if(!f.ok())return false;
      }

      // update materials
      REPAO(materials).fixPath(GetPath(name));

      // create mesh
      if(mesh)
      {
         // process smoothing groups
         REPA(meshes)
         {
            Mesh &mesh=meshes[i];
            if(mesh.matrix.mirrored()) // flip X coordinate of vertices if mesh matrix has negative determinant
            {
               Matrix mirrored=mesh.matrix; mirrored.x.chs(); Matrix m=~mesh.matrix*mirrored;
               mesh.transform(m).reverse();
            }

            if(mesh.tri_smooth_groups.elms())
            {
               mesh.explodeVtxs(); // explode so each face has its unique vertexes, this is needed because in 3DS, 2 faces sharing the exact same vertex, can have different smoothing groups
               if(C VecI *tri=mesh.tri.ind())
               {
                  mesh.include(VTX_MATERIAL); // for simplification, we're applying smoothing groups onto vertexes, no need to initially clear the vtx material because we've used 'explodeVtxs', making all tris have their own unique vertexes
                  REPA(mesh.tri) // iterate all faces
                  {
                     UInt group=mesh.tri_smooth_groups[i]; // get smoothing groups of that face
                   C VecI &t=tri[i]; REPA(t)mesh.vtx.material(t.c[i]).u=group; // apply smoothing groups onto triangle vertexes
                  }
                  mesh.setVtxDupEx(0, EPSD, EPS_COL_COS, true); // generate vertex duplicates based on smoothing groups, use small epsilon in case mesh is scaled down
                  mesh.exclude(VTX_MATERIAL); // remove no longer needed vertex material
               }
            }
            mesh.setNormals();
            if(mesh.vtx.dup())
            {
               mesh.exclude(VTX_DUP);
               mesh.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, having duplicates means that we called explode vertexes, which now we need to weld, do not remove degenerate faces because they're not needed because we're doing this only because of 'explodeVtxs'
            }
         }

         Memc<MeshPart> parts; // parts after splitting materials

         FREPA(nodes) // iterate all nodes
         {
            Node &node=nodes[i]; REPA(meshes) // find mesh for this node
            {
             C Mesh &mesh=meshes[i]; if(Equal(mesh.name, node.name, true)) // if they have the same name
               {
                  Matrix m=~mesh.matrix;
                  m.moveBack(node.pivot);
                  m*=mesh.matrix;
                  Int max_id=mesh.maxId();
                  if( max_id<0) // if mesh doesn't have ID at all
                  {  // copy the complete mesh
                     MeshPart &part=parts.New();
                     Set(part.name, mesh.name);
                     part.base.create(mesh);
                     part.base.transform(m);
                     if(part_material_index)part_material_index.add(-1); // set material as -1
                  }else // if it has ID's
                  for(Int i=-1; i<=max_id; i++)if(mesh.hasId(i)) // iterate through all id's and copy only those which have faces
                  {
                     MeshPart &part=parts.New();
                     Set(part.name, mesh.name);
                     mesh.copyId(part.base, i);
                     part.base.transform(m);
                     if(part_material_index)part_material_index.add(i); // set i-th material
                  }
                  break;
               }
            }
         }

         mesh->create (parts.elms()); REPA(*mesh)Swap(mesh->parts[i], parts[i]);
         mesh->exclude(FACE_ID).rightToLeft().texScale(Vec2(1, -1)).setBox();
         CleanMesh(*mesh);
      }
      
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
