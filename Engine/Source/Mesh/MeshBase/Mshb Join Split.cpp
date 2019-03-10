/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************
void MeshBase::corridor(MeshBase &mshb, Int e0, Int e1) // add 'mshb' and create en edged corridor between 'e0' (from self) and 'e1' (from 'mshb') edges
{
   if(edge.flag() && mshb.edge.flag())
   {
      if(this==&mshb)
      {
         if(e0==e1)return;
      }else
      {
         e1+=edges();
         add(mshb);
      }
      VecI2 i0=edge.ind (e0),
            i1=edge.ind (e1);
      UInt  f0=edge.flag(e0); edge.flag(e0)|=ETQ_LR;
      UInt  f1=edge.flag(e1); edge.flag(e1)|=ETQ_LR;
      Int   id0, id1, id_value, *p0, *p1;
      if((f0&ETQ_LR)==ETQ_L){i0.reverse(); if(edge.id()){id0=edge.id(e0).y; p0=&edge.id(e0).x;}}else if(edge.id()){id0=edge.id(e0).x; p0=&edge.id(e0).y;}
      if((f1&ETQ_LR)==ETQ_L){i1.reverse(); if(edge.id()){id1=edge.id(e1).y; p1=&edge.id(e1).x;}}else if(edge.id()){id1=edge.id(e1).x; p1=&edge.id(e1).y;}
      if(edge.ind())
      {
          id_value=id0;
         *p0=id_value;
         *p1=id_value;
      }
      addEdge(VecI2(i0.c[0], i1.c[1])); edge.flag(edges()-1)=ETQ_R; if(edge.id())edge.id(edges()-1).set(id_value, -1);
      addEdge(VecI2(i1.c[0], i0.c[1])); edge.flag(edges()-1)=ETQ_R; if(edge.id())edge.id(edges()-1).set(id_value, -1);
   }
}
/******************************************************************************/
void MeshBase::split(MemPtr<MeshBaseIndex> meshes, C Boxes &boxes, UInt flag_and)C
{
 C Int *p;

   flag_and&=((VTX_ALL&~VTX_DUP)|EDGE_NRM|EDGE_FLAG|EDGE_ID|TRI_NRM|TRI_FLAG|TRI_ID|QUAD_NRM|QUAD_FLAG|QUAD_ID)&flag();

   // link box->edge,tri,quad
   Index box_edge(boxes.num());
   Index box_tri (boxes.num());
   Index box_quad(boxes.num());
   {
      Int *    _box= Alloc<Int>(Max(edges(), tris(), quads()));
      Int *edge_box=_box; FREPA(edge){p=edge.ind(i).c; box_edge.incGroup(edge_box[i]=boxes.index(Avg(vtx.pos(p[0]), vtx.pos(p[1])                              )));} box_edge.set(edge_box);
      Int * tri_box=_box; FREPA(tri ){p=tri .ind(i).c; box_tri .incGroup( tri_box[i]=boxes.index(Avg(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[2])               )));} box_tri .set( tri_box);
      Int *quad_box=_box; FREPA(quad){p=quad.ind(i).c; box_quad.incGroup(quad_box[i]=boxes.index(Avg(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[2]), vtx.pos(p[3]))));} box_quad.set(quad_box);
      Free(_box);
   }

   // create boxes
   meshes.clear();
   Memt<Bool> vtx_is; vtx_is.setNum(vtxs());
   Memt<Int > vtx_remap;
   FREP(boxes.num())
   {
      Int *_edge=box_edge.group[i].elm, edges=box_edge.group[i].num,
          *_tri =box_tri .group[i].elm, tris =box_tri .group[i].num,
          *_quad=box_quad.group[i].elm, quads=box_quad.group[i].num;

      if(edges || tris || quads)
      {
         MeshBaseIndex &mesh=meshes.New();
         mesh.index=i;
         mesh.create(0, edges, tris, quads, flag_and);

         CopyList(mesh.edge.ind (), edge.ind (), MemPtr<Int>(_edge, edges));
         CopyList(mesh.edge.nrm (), edge.nrm (), MemPtr<Int>(_edge, edges));
         CopyList(mesh.edge.flag(), edge.flag(), MemPtr<Int>(_edge, edges));
         CopyList(mesh.edge.id  (), edge.id  (), MemPtr<Int>(_edge, edges));

         CopyList(mesh.tri.ind (), tri.ind (), MemPtr<Int>(_tri, tris));
         CopyList(mesh.tri.nrm (), tri.nrm (), MemPtr<Int>(_tri, tris));
         CopyList(mesh.tri.flag(), tri.flag(), MemPtr<Int>(_tri, tris));
         CopyList(mesh.tri.id  (), tri.id  (), MemPtr<Int>(_tri, tris));

         CopyList(mesh.quad.ind (), quad.ind (), MemPtr<Int>(_quad, quads));
         CopyList(mesh.quad.nrm (), quad.nrm (), MemPtr<Int>(_quad, quads));
         CopyList(mesh.quad.flag(), quad.flag(), MemPtr<Int>(_quad, quads));
         CopyList(mesh.quad.id  (), quad.id  (), MemPtr<Int>(_quad, quads));

         Zero(vtx_is.data(), vtx_is.elms());

         REPAD(j, mesh.edge){p=mesh.edge.ind(j).c; REPD(k, 2)vtx_is[p[k]]=true;}
         REPAD(j, mesh.tri ){p=mesh.tri .ind(j).c; REPD(k, 3)vtx_is[p[k]]=true;}
         REPAD(j, mesh.quad){p=mesh.quad.ind(j).c; REPD(k, 4)vtx_is[p[k]]=true;}

         SetRemap(vtx_remap, vtx_is         ,      vtxs ());
         IndRemap(vtx_remap, mesh.edge.ind(), mesh.edges());
         IndRemap(vtx_remap, mesh.tri .ind(), mesh.tris ());
         IndRemap(vtx_remap, mesh.quad.ind(), mesh.quads());

         mesh.vtx._elms=CountIs(vtx_is);
         mesh.include(flag_and);
         mesh.copyVtxs(T, vtx_is);
      }
   }
}
void MeshBase::split(MemPtr<MeshBaseIndex> meshes, C VecI &cells, UInt flag_and)C
{
   split(meshes, Boxes(Box(T), cells), flag_and);
}
/******************************************************************************/
void MeshBase::splitVtxs(MeshBase &dest, C MemPtr<Bool> &vtx_is, UInt flag_and)
{
   if(this!=&dest)
   {
      if(!vtx_is.elms())dest.del();else
      {
         Memt<Bool> is, is2; is.setNum(vtx.elms());

         Int i=0, elms=Min(is.elms(), vtx.elms());
         for(; i<   elms  ; i++)is[i]=vtx_is[i];
         for(; i<is.elms(); i++)is[i]=false;

         // copy to 'dest' and select neighbors as well
         dest.del();
         is2.setNum(edges()); REPA(edge){C VecI2 &e=edge.ind(i); if(is2[i]=(ElmIs(vtx_is, e.x) || ElmIs(vtx_is, e.y)                                            ))is[e.x]=is[e.y]=                true;} dest.edge._elms=CountIs(is2); dest.include(flag()&flag_and&EDGE_ALL); dest.copyEdges(T, is2);
         is2.setNum(tris ()); REPA(tri ){C VecI  &t=tri .ind(i); if(is2[i]=(ElmIs(vtx_is, t.x) || ElmIs(vtx_is, t.y) || ElmIs(vtx_is, t.z)                      ))is[t.x]=is[t.y]=is[t.z]=        true;} dest.tri ._elms=CountIs(is2); dest.include(flag()&flag_and& TRI_ALL); dest.copyTris (T, is2);
         is2.setNum(quads()); REPA(quad){C VecI4 &q=quad.ind(i); if(is2[i]=(ElmIs(vtx_is, q.x) || ElmIs(vtx_is, q.y) || ElmIs(vtx_is, q.z) || ElmIs(vtx_is, q.w)))is[q.x]=is[q.y]=is[q.z]=is[q.w]=true;} dest.quad._elms=CountIs(is2); dest.include(flag()&flag_and&QUAD_ALL); dest.copyQuads(T, is2);
         dest.vtx._elms=CountIs(is); dest.include(flag()&flag_and&(VTX_ALL&~VTX_DUP)); dest.copyVtxs(T, is);
         Memt<Int> vtx_remap;
         SetRemap(vtx_remap, is, vtxs());
         IndRemap(vtx_remap, dest.edge.ind(), dest.edges());
         IndRemap(vtx_remap, dest. tri.ind(), dest. tris());
         IndRemap(vtx_remap, dest.quad.ind(), dest.quads());

         // remove from self
         is2.setNum(vtx.elms()); // select vertexes that are left
         for(i=0; i<    elms  ; i++)is2[i]=!vtx_is[i];
         for(   ; i<is2.elms(); i++)is2[i]=true;
         REPAO(is)^=1; // invert selection, this now removes the neighbors too, so we need to check which remaining faces still use them
         REPA(edge){C VecI2 &e=edge.ind(i); if(is2[e.x] && is2[e.y]                        )is[e.x]=is[e.y]=                true;}
         REPA(tri ){C VecI  &t=tri .ind(i); if(is2[t.x] && is2[t.y] && is2[t.z]            )is[t.x]=is[t.y]=is[t.z]=        true;}
         REPA(quad){C VecI4 &q=quad.ind(i); if(is2[q.x] && is2[q.y] && is2[q.z] && is2[q.w])is[q.x]=is[q.y]=is[q.z]=is[q.w]=true;}
         keepVtxs(is);
      }
   }
}
void MeshBase::splitFaces(MeshBase &dest, C MemPtr<Bool> &edge_is, C MemPtr<Bool> &tri_is, C MemPtr<Bool> &quad_is, UInt flag_and)
{
   if(this!=&dest)
   {
      if(!edge_is.elms() && !tri_is.elms() && !quad_is.elms())dest.del();else
      {
         // copy
         copyFace(dest, edge_is, tri_is, quad_is, flag_and);

         // remove from original
         Memt<Bool> edge_nis; edge_nis.setNum(edges()); if(!edge_is)SetMem(edge_nis.data(), 1, edge_nis.elms());else{Int i=0, elms=Min(edge_is.elms(), edge_nis.elms()); for(; i<elms; i++)edge_nis[i]=edge_is[i]^1; for(; i<edge_nis.elms(); i++)edge_nis[i]=true;}
         Memt<Bool>  tri_nis;  tri_nis.setNum(tris ()); if(! tri_is)SetMem( tri_nis.data(), 1,  tri_nis.elms());else{Int i=0, elms=Min( tri_is.elms(),  tri_nis.elms()); for(; i<elms; i++) tri_nis[i]= tri_is[i]^1; for(; i< tri_nis.elms(); i++) tri_nis[i]=true;}
         Memt<Bool> quad_nis; quad_nis.setNum(quads()); if(!quad_is)SetMem(quad_nis.data(), 1, quad_nis.elms());else{Int i=0, elms=Min(quad_is.elms(), quad_nis.elms()); for(; i<elms; i++)quad_nis[i]=quad_is[i]^1; for(; i<quad_nis.elms(); i++)quad_nis[i]=true;}

         keepEdges       (edge_nis);
         keepTris        ( tri_nis);
         keepQuads       (quad_nis);
         removeUnusedVtxs(        );
      }
   }
}
void MeshBase::splitBone(MeshBase &dest, Int bone, UInt flag_and)
{
   if(this!=&dest)
   {
      if(C VecB4 *matrix=vtx.matrix())
      {
       C Int *p;
         Bool is;
         Memt<Bool> vtx_bone, tri_is, quad_is;
          vtx_bone.setNum(vtxs ());
          tri_is  .setNum(tris ());
         quad_is  .setNum(quads());

         // create copy
         REPA(vtx )vtx_bone[i]=(matrix[i].c[0]-1==bone);
         REPA(tri ){is=false; p=tri .ind(i).c; REPD(j, 3)if(vtx_bone[p[j]]){is=true; break;}  tri_is[i]=is;}
         REPA(quad){is=false; p=quad.ind(i).c; REPD(j, 4)if(vtx_bone[p[j]]){is=true; break;} quad_is[i]=is;}

         // split
         splitFaces(dest, null, tri_is, quad_is, flag_and);
      }else
      {
         dest.del();
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
