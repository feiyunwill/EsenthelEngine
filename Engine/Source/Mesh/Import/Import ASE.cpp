/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace ASE{ // so local structures are unique
/******************************************************************************/
// ASE stores animation keyframes relative to previous frames
/******************************************************************************/
struct FaceNrm
{
   Vec nrm[3];
};
struct Face
{
   VecI ind;
   Int  sub_material;

   Face() {ind.zero(); sub_material=0;}
};
struct Mtrl : XMaterial
{
   Int        global_index;
   Memc<Mtrl> sub_mtrl;

   void import(FileTextEx &f, C Str &path)
   {
      for(; f.level(); )
      {
         if(f.cur("*MATERIAL_TWOSIDED"    ))cull     =  false      ;else
         if(f.cur("*MATERIAL_NAME"        ))name     =  f.getName();else
       //if(f.cur("*MATERIAL_AMBIENT"     ))ambient  =  f.getVec ();else
         if(f.cur("*MATERIAL_DIFFUSE"     ))color.xyz=  f.getVec ();else
       //if(f.cur("*MATERIAL_SPECULAR"    ))specular =  f.getVec ();else
         if(f.cur("*MATERIAL_SHINE"       ))specular =  f.getFlt ();else
         if(f.cur("*MATERIAL_TRANSPARENCY"))color.w  =1-f.getFlt ();else
         if(f.cur("*MAP_GENERIC") && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("*BITMAP") && !color_map.is())color_map=f.getName();
            }
         }else
         if(f.cur("*MAP_DIFFUSE") && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("*BITMAP"))color_map=f.getName();
            }
         }else
         if(f.cur("*MAP_SHINE") && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("*BITMAP"))specular_map=f.getName();
            }
         }else
         if(f.cur("*MAP_OPACITY") && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("*BITMAP"))alpha_map=f.getName();
            }
         }else
         if(f.cur("*MAP_BUMP") && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("*BITMAP"))normal_map=f.getName();else
               if(f.cur("*MAP_GENERIC") && f.getIn())
               {
                  for(; f.level(); )
                  {
                     if(f.cur("*BITMAP") && !normal_map.is())normal_map=f.getName();
                  }
               }
            }
         }else
         if(f.cur("*SUBMATERIAL") && f.getIn())sub_mtrl.New().import(f, path);
      }
      fixPath(path);
   }
   void copyTo(MemPtr<XMaterial> materials) {global_index=materials.elms(); materials.New()=T; FREPAO(sub_mtrl).copyTo(materials);}
};
struct Node
{
   Int           material;
   Str           name, parent;
   Matrix        matrix;
   Memc<Vec    > pos;
   Memc<FaceNrm> nrm;
   Memc<Vec2   > tex;
   Memc<Face   > tri;
   Memc<VecI   > ttex;

   Node() {matrix.identity(); material=-1;}
};
struct Ase
{
   Memc<Mtrl> mtrl;
   Memc<Node> node;
};
/******************************************************************************/
static void ImportASE(FileTextEx &f, Ase &ase, C Str &path)
{
   for(; f.level(); )
   {
      if(f.cur("*MATERIAL_LIST") && f.getIn())
      {
         for(; f.level(); )
         {
            if(f.cur("*MATERIAL") && f.getIn())ase.mtrl.New().import(f, path);
         }
      }else
      if(f.cur("*GROUP"     ) && f.getIn())ImportASE(f, ase, path);else
      if(f.cur("*GEOMOBJECT") && f.getIn())
      {
         Node &node=ase.node.New();
         for(; f.level(); )
         {
            if(f.cur("*NODE_NAME"  ))node.name  =f.getName();else
            if(f.cur("*NODE_PARENT"))node.parent=f.getName();else
            if(f.cur("*NODE_TM"    ) && f.getIn())
            {
               for(; f.level(); )
               {
                  if(f.cur("*TM_ROW0"))f.get(node.matrix.x  );else
                  if(f.cur("*TM_ROW1"))f.get(node.matrix.y  );else
                  if(f.cur("*TM_ROW2"))f.get(node.matrix.z  );else
                  if(f.cur("*TM_ROW3"))f.get(node.matrix.pos);
               }
            }else
            if(f.cur("*MESH") && f.getIn())
            {
               for(; f.level(); )
               {
                  if(f.cur("*MESH_VERTEX_LIST") && f.getIn())
                  {for(; f.level(); ){
                     if(f.cur("*MESH_VERTEX"))
                     {
                        f.getWord();
                        f.get(node.pos.New());
                     }
                  }}else
                  if(f.cur("*MESH_FACE_LIST") && f.getIn())
                  {for(; f.level(); ){
                     if(f.cur("*MESH_FACE"))
                     {
                        VecI &tri=node.tri.New().ind; // range checks are done later
                        f.getWord();
                        f.getWord(); tri.x=f.getInt();
                        f.getWord(); tri.y=f.getInt();
                        f.getWord(); tri.z=f.getInt();
                     }else
                     if(f.cur("*MESH_MTLID"))
                     {
                        if(node.tri.elms())node.tri.last().sub_material=f.getInt();
                     }
                  }}else
                  if(f.cur("*MESH_TVERTLIST") && f.getIn())
                  {for(; f.level(); ){
                     if(f.cur("*MESH_TVERT"))
                     {
                        f.getWord();
                        Vec2 &tex=node.tex.New(); f.get(tex); tex.y=1-tex.y;
                     }
                  }}else
                  if(f.cur("*MESH_TFACELIST") && f.getIn())
                  {for(; f.level(); ){
                     if(f.cur("*MESH_TFACE"))
                     {
                        f.getWord();
                        f.get(node.ttex.New());
                     }
                  }}else
                  if(f.cur("*MESH_NORMALS") && f.getIn())
                  {
                     node.nrm.setNum(node.tri.elms());
                     for(Int face=-1, vtx; f.level(); )
                     {
                        if(f.cur("*MESH_FACENORMAL"  )){face=f.getInt(); vtx=0;}else
                        if(f.cur("*MESH_VERTEXNORMAL") && InRange(face, node.tri.elms()) && vtx<3){f.getWord(); f.get(node.nrm[face].nrm[vtx++]);}
                     }
                  }
               }
            }else
            if(f.cur("*MATERIAL_REF"))node.material=f.getInt();
         }
      }
   }
}
/******************************************************************************/
} // namespace ASE
/******************************************************************************/
Bool ImportASE(C Str &name, Mesh *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   using namespace ASE;

   if(mesh)mesh->del();
   materials          .clear();
   part_material_index.clear();

   FileTextEx f; if(f.read(name))
   {
      Ase ase; ImportASE(f, ase, GetPath(name));

      // materials
      if(materials)FREPA(ase.mtrl)ase.mtrl[i].copyTo(materials);

      // mesh
      if(mesh)
      {
         FREPA(ase.node)
         {
            MeshPart  part;
            MeshBase &base=part.base;
       ASE::Node     &src =ase .node[i];

            Set(part.name, src.name);
            base.create(src.tri.elms()*3, 0, src.tri.elms(), 0);
            Int j=0; FREPA(src.tri)
            {
             C VecI &ind=src.tri[i].ind; REPA(ind)if(!InRange(ind.c[i], src.pos))return false;
               base.tri.ind(i).set(j, j+1, j+2);
               base.vtx.pos(j++)=src.pos[ind.c[0]];
               base.vtx.pos(j++)=src.pos[ind.c[1]];
               base.vtx.pos(j++)=src.pos[ind.c[2]];
            }
            if(src.nrm.elms())
            {
               base.include(VTX_NRM);
               Int j=0; FREPA(src.nrm)
               {
                C FaceNrm &nrm=src.nrm[i];
                  base.vtx.nrm(j++)=nrm.nrm[0];
                  base.vtx.nrm(j++)=nrm.nrm[1];
                  base.vtx.nrm(j++)=nrm.nrm[2];
               }
               Transform(base.vtx.nrm(), src.matrix.orn(), base.vtxs());
               Normalize(base.vtx.nrm(),                   base.vtxs());
            }
            if(src.ttex.elms())
            {
               base.include(VTX_TEX0);
               j=0; FREPA(src.ttex)
               {
                C VecI &ttex=src.ttex[i]; REPA(ttex)if(!InRange(ttex.c[i], src.tex))return false;
                  base.vtx.tex0(j++)=src.tex[ttex.c[0]];
                  base.vtx.tex0(j++)=src.tex[ttex.c[1]];
                  base.vtx.tex0(j++)=src.tex[ttex.c[2]];
               }
            }
            if(InRange(src.material, ase.mtrl) && ase.mtrl[src.material].sub_mtrl.elms()) // if selected material has sub-materials
            {
               base.include(TRI_ID); FREPA(base.tri)base.tri.id(i)=src.tri[i].sub_material;
            }
            base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down

            if(base.tri.id()) // we need to split into parts
            {
               Mesh  temp; base.copyId(temp, ~TRI_ID);
               FREPA(temp)if(temp.parts[i].is())
               {
                  Swap(mesh->parts.New(), temp.parts[i]);
                  if(part_material_index)
                  {
                     Mtrl &mtrl=ase.mtrl[src.material]; // this will always be in range, because this is executed for tri.id, and that is created only if 'src.material' is in range
                     part_material_index.add(InRange(i, mtrl.sub_mtrl) ? mtrl.sub_mtrl[i].global_index : mtrl.global_index);
                  }
               }
            }else
            {
               if(part.is())
               {
                  Swap(mesh->parts.New(), part);
                  if(part_material_index)part_material_index.add(InRange(src.material, ase.mtrl) ? ase.mtrl[src.material].global_index : -1);
               }
            }
         }
         mesh->rightToLeft().setBox();
         REPA(*mesh)if(!mesh->parts[i].base.vtx.nrm())mesh->parts[i].base.setNormals();
         CleanMesh(*mesh);
      }
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
