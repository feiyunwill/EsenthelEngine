/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void MeshBase::linkVtxVtxOnFace(Index &vtx_vtx)C
{
 C Int   *p;
 C VecI  *_tri =tri .ind();
 C VecI4 *_quad=quad.ind();

   // prepare groups
   vtx_vtx.create(vtxs());
   if(!vtx.dup())
   {
      REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_vtx.incGroup(*p++, 2);}
      REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_vtx.incGroup(*p++, 3);}
   }else
   {
      REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_vtx.incGroup(vtx.dup(*p++), 2);}
      REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_vtx.incGroup(vtx.dup(*p++), 3);}
   }
   vtx_vtx.set();

   // add elms
  _tri=tri.ind();
   REPA(tri)
   {
      VecI f=*_tri++; f.remap(vtx.dup());
      vtx_vtx.addElm(f.c[0], f.c[1]); vtx_vtx.addElm(f.c[0], f.c[2]);
      vtx_vtx.addElm(f.c[1], f.c[2]); vtx_vtx.addElm(f.c[1], f.c[0]);
      vtx_vtx.addElm(f.c[2], f.c[0]); vtx_vtx.addElm(f.c[2], f.c[1]);
   }
  _quad=quad.ind();
   REPA(quad)
   {
      VecI4 f=*_quad++; f.remap(vtx.dup());
      vtx_vtx.addElm(f.c[0], f.c[1]); vtx_vtx.addElm(f.c[0], f.c[2]); vtx_vtx.addElm(f.c[0], f.c[3]);
      vtx_vtx.addElm(f.c[1], f.c[2]); vtx_vtx.addElm(f.c[1], f.c[3]); vtx_vtx.addElm(f.c[1], f.c[0]);
      vtx_vtx.addElm(f.c[2], f.c[3]); vtx_vtx.addElm(f.c[2], f.c[0]); vtx_vtx.addElm(f.c[2], f.c[1]);
      vtx_vtx.addElm(f.c[3], f.c[0]); vtx_vtx.addElm(f.c[3], f.c[1]); vtx_vtx.addElm(f.c[3], f.c[2]);
   }

   // remove duplicates
   FREPA(vtx)
   {
      IndexGroup &ig=vtx_vtx.group[i];
      FREPAD(j, ig)
      {
         Int elm=ig[j];
         for(Int k=j+1; k<ig.num; )if(ig[k]==elm)ig.subElm(k);else k++;
      }
   }
}
void MeshBase::linkVtxVtxOnEdge(Index &vtx_vtx, Bool sort)C
{
 C Int *p;
   vtx_vtx.create(vtxs());
   FREPA(edge){p=edge.ind(i).c; vtx_vtx.incGroup(p[0]      ); vtx_vtx.incGroup(p[1]      );} vtx_vtx.set();
   FREPA(edge){p=edge.ind(i).c; vtx_vtx.addElm  (p[0], p[1]); vtx_vtx.addElm  (p[1], p[0]);}

   if(sort)
   {
    C Vec        *pos=vtx.pos();
      FloatIndex *fi =Alloc<FloatIndex>(vtx_vtx.group_elms_max);
      FREP(vtx_vtx.groups)
      {
         IndexGroup &ig    =vtx_vtx.group[i];
       C Vec2       &center=pos[i].xy;
         FREPA(ig)
         {
            Int vtx_index=ig[i];
            fi[i].f=AngleFast(pos[vtx_index].xy-center);
            fi[i].i=vtx_index;
         }
         Sort(fi, ig.num);
         FREPA(ig)ig[i]=fi[i].i;
      }
      Free(fi);
   }
}
/******************************************************************************/
void MeshBase::linkVtxEdge(Index &vtx_edge, Bool sort)C
{
 C Int *p;
   vtx_edge.create(vtxs());
   FREPA(edge){p=edge.ind(i).c; vtx_edge.incGroup(p[0]   ); vtx_edge.incGroup(p[1]   );} vtx_edge.set();
   FREPA(edge){p=edge.ind(i).c; vtx_edge.addElm  (p[0], i); vtx_edge.addElm  (p[1], i);}

   if(sort)
   {
    C Vec        *pos=vtx.pos();
      FloatIndex *fi =Alloc<FloatIndex>(vtx_edge.group_elms_max);
      FREP(vtx_edge.groups)
      {
         IndexGroup &ig    =vtx_edge.group[i];
       C Vec2       &center=pos[i].xy;
         FREPAD(e, ig)
         {
            Int edge_index=ig[e]; p=edge.ind(edge_index).c; Int vtx_index=p[0]; if(vtx_index==i)vtx_index=p[1];
            fi[e].f=AngleFast(pos[vtx_index].xy-center);
            fi[e].i=edge_index;
         }
         Sort(fi, ig.num);
         FREPA(ig)ig[i]=fi[i].i;
      }
      Free(fi);
   }
}
void MeshBase::linkVtxFace(Index &vtx_face)C
{
 C Int *p;
   vtx_face.create(vtxs());
   if(!vtx.dup())
   {
    C VecI  *_tri =tri .ind();  REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.incGroup(*p++            );}
    C VecI4 *_quad=quad.ind();  REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.incGroup(*p++            );} vtx_face.set();
             _tri =tri .ind(); FREPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.addElm  (*p++, i         );}
             _quad=quad.ind(); FREPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.addElm  (*p++, i^SIGN_BIT);}
   }else
   {
    C VecI  *_tri =tri .ind();  REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.incGroup(vtx.dup(*p++)            );}
    C VecI4 *_quad=quad.ind();  REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.incGroup(vtx.dup(*p++)            );} vtx_face.set();
             _tri =tri .ind(); FREPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.addElm  (vtx.dup(*p++), i         );}
             _quad=quad.ind(); FREPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.addElm  (vtx.dup(*p++), i^SIGN_BIT);}
   }
}
void MeshBase::linkFaceFace(Index &face_face)C
{
   face_face.create(faces());
   Index vtx_face; linkVtxFace(vtx_face);
   Memt<Int> face;

   REPAD(f, tri) // iterate all tris
   {
      VecI a=tri.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all tri vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=f)face.include(test_face); // face different than triangle 'f'
         }
      }
      face_face.incGroup(f, face.elms()); face.clear();
   }
   REPAD(f, quad) // iterate all quads
   {
      UInt fs=f^SIGN_BIT;
      VecI4 a=quad.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all quad vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=fs)face.include(test_face); // face different than quad 'f'
         }
      }
      face_face.incGroup(tris()+f, face.elms()); face.clear(); // add 'tris()' because we're processing quads and tris were listed first
   }

   face_face.set();

   REPAD(f, tri) // iterate all tris
   {
      VecI a=tri.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all tri vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=f)face.include(test_face); // face different than triangle 'f'
         }
      }
      IndexGroup &ig=face_face.group[f]; REPA(face)ig.add(face[i]); face.clear();
   }
   REPAD(f, quad) // iterate all quads
   {
      UInt fs=f^SIGN_BIT;
      VecI4 a=quad.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all quad vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=fs)face.include(test_face); // face different than quad 'f'
         }
      }
      IndexGroup &ig=face_face.group[tris()+f]; REPA(face)ig.add(face[i]); face.clear(); // add 'tris()' because we're processing quads and tris were listed first
   }
}
/******************************************************************************/
void MeshBase::linkEdgeFace()
{
   Index vtx_face; linkVtxFace(vtx_face);
   include(EDGE_ADJ_FACE);
   FREPAD(e, edge)
   {
      Int         r =-1, l=-1;
      VecI2       ep=edge.ind(e); ep.remap(vtx.dup());
      IndexGroup &vf=vtx_face.group[ep.c[0]];
      REPAD(f, vf)
      {
         Int tf=vf[f], side;
         if(tf&SIGN_BIT)
         {
            VecI4 fp=quad.ind(tf^SIGN_BIT); fp.remap(vtx.dup());
            side=GetSide(ep, fp);
         }else
         {
            VecI fp=tri.ind(tf); fp.remap(vtx.dup());
            side=GetSide(ep, fp);
         }
         if(side==SIDE_R)r=tf;else
         if(side==SIDE_L)l=tf;
      }
      edge.adjFace(e).set(r, l);
   }
}
/******************************************************************************/
void MeshBase::linkRectEdge(Index &rect_edge, C Rects &rects)C
{
   rect_edge.create(rects.num());
   FREPD(step, 2)
   {
      FREPA(edge)
      {
       C Int *p=edge.ind(i).c;
         for(PixelWalkerMask walker(rects.coords(vtx.pos(p[0]).xy), rects.coords(vtx.pos(p[1]).xy), RectI(0, 0, rects.cells.x-1, rects.cells.y-1)); walker.active(); walker.step())
         {
            Int group=rects.index(walker.pos());
            if(!step)rect_edge.incGroup(group);
            else     rect_edge.addElm  (group, i);
         }
      }
      if(!step)rect_edge.set();
   }
}
/******************************************************************************/
static void FillVtx(C Index &vtx_vtx, Memt<Bool> &is, Int vtx)
{
   Memt<Int> vtxs; if(!is[vtx]){is[vtx]=true; vtxs.add(vtx);}
   for(; vtxs.elms(); )
   {
      Int vtx=vtxs.pop();
      IndexGroup &ig=vtx_vtx.group[vtx]; REPA(ig){Int vtx=ig[i]; if(!is[vtx]){is[vtx]=true; vtxs.add(vtx);}}
   }
}
void MeshBase::getVtxNeighbors(Int vtx, MemPtr<Int> vtxs)C
{
   vtxs.clear(); if(InRange(vtx, T.vtx))
   {
      Index vtx_vtx; linkVtxVtxOnFace(vtx_vtx);
      Memt<Bool> is; is.setNumZero(T.vtxs());
      FillVtx(vtx_vtx, is, T.vtx.dup() ? T.vtx.dup(vtx) : vtx);
      FREPA(T.vtx)
      {
         Int v=i; if(T.vtx.dup())v=T.vtx.dup(v);
         if(is[v])vtxs.add(i);
      }
   }
}
void MeshBase::getFaceNeighbors(Int face, MemPtr<Int> faces)C
{
   faces.clear(); if((face&SIGN_BIT) ? InRange(face^SIGN_BIT, quad) : InRange(face, tri))
   {
      Index vtx_vtx; linkVtxVtxOnFace(vtx_vtx);
      Memt<Bool> is; is.setNumZero(T.vtxs());
      if(face&SIGN_BIT){VecI4 f=quad.ind(face^SIGN_BIT); f.remap(vtx.dup()); REPA(f)FillVtx(vtx_vtx, is, f.c[i]);}
      else             {VecI  f=tri .ind(face         ); f.remap(vtx.dup()); REPA(f)FillVtx(vtx_vtx, is, f.c[i]);}
      REPA(tri ){VecI  f=tri .ind(i); f.remap(vtx.dup()); if(is[f.x] || is[f.y] || is[f.z]           )faces.add(i         );}
      REPA(quad){VecI4 f=quad.ind(i); f.remap(vtx.dup()); if(is[f.x] || is[f.y] || is[f.z] || is[f.w])faces.add(i^SIGN_BIT);}
   }
}
/******************************************************************************/
}
/******************************************************************************/
