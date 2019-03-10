/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MeshBase& MeshBase::addVtx(C Vec &pos)
{
   exclude(VTX_DUP);

                    Realloc1    (vtx._pos     , vtxs());
   if(vtx._nrm     )ReallocZero1(vtx._nrm     , vtxs());
   if(vtx._tan     )ReallocZero1(vtx._tan     , vtxs());
   if(vtx._bin     )ReallocZero1(vtx._bin     , vtxs());
   if(vtx._hlp     )ReallocZero1(vtx._hlp     , vtxs());
   if(vtx._tex0    )ReallocZero1(vtx._tex0    , vtxs());
   if(vtx._tex1    )ReallocZero1(vtx._tex1    , vtxs());
   if(vtx._tex2    )ReallocZero1(vtx._tex2    , vtxs());
   if(vtx._matrix  )ReallocZero1(vtx._matrix  , vtxs());
   if(vtx._blend   )ReallocZero1(vtx._blend   , vtxs());
   if(vtx._size    )ReallocZero1(vtx._size    , vtxs());
   if(vtx._material)ReallocZero1(vtx._material, vtxs());
   if(vtx._color   )ReallocZero1(vtx._color   , vtxs());
   if(vtx._flag    )ReallocZero1(vtx._flag    , vtxs());

   Int t=vtxs(); vtx._elms++; vtx.pos(t)=pos;

   return T;
}
MeshBase& MeshBase::addEdge(C VecI2 &ind)
{
   exclude(ADJ_ALL);

                 Realloc1    (edge._ind , edges());
   if(edge._nrm )ReallocZero1(edge._nrm , edges());
   if(edge._flag)ReallocZero1(edge._flag, edges());
   if(edge._id  )ReallocZero1(edge._id  , edges());

   Int t=edges(); edge._elms++;
                  edge.ind (t)=ind;
   if(edge.flag())edge.flag(t)=ETQ_R;

   return T;
}
MeshBase& MeshBase::addTri(C VecI &ind)
{
   exclude(ADJ_ALL);

                Realloc1    (tri._ind , tris());
   if(tri._nrm )ReallocZero1(tri._nrm , tris());
   if(tri._flag)ReallocZero1(tri._flag, tris());
   if(tri._id  )ReallocZero1(tri._id  , tris());

   Int t=tris(); tri._elms++; tri.ind(t)=ind;

   return T;
}
MeshBase& MeshBase::addQuad(C VecI4 &ind)
{
   exclude(ADJ_ALL);

                 Realloc1    (quad._ind , quads());
   if(quad._nrm )ReallocZero1(quad._nrm , quads());
   if(quad._flag)ReallocZero1(quad._flag, quads());
   if(quad._id  )ReallocZero1(quad._id  , quads());

   Int t=quads(); quad._elms++; quad.ind(t)=ind;

   return T;
}
MeshBase& MeshBase::add(C MeshBase &src, UInt flag_and)
{
   if(!src.is())return T;
 C MeshBase *meshes[]={this, &src};
   return create(meshes, Elms(meshes), flag_and);
}
MeshBase& MeshBase::add(C MeshRender &src, UInt flag_and) {MeshBase mb(src, flag_and); return add(mb);}
MeshBase& MeshBase::add(C MeshPart   &src, UInt flag_and) {MeshBase mb(src, flag_and); return add(mb);}
/******************************************************************************
MeshBase& addEdge(C Vec2       &pos, Int edge        ); // add edge     by splitting 'edge' edge into two edges at 'pos' position
MeshBase& MeshBase::addEdge(C Vec2 &pos, Int e)
{
   exclude((VTX_ALL|ADJ_ALL|EDGE_ALL)^(VTX_POS|EDGE_IND|EDGE_ID|EDGE_FLAG));
   Clamp(e, -1, edges()-1);

               ReallocZero1(vtx ._pos , vtxs ());
               ReallocZero1(edge._ind , edges());
               ReallocZero1(edge._flag, edges());
   if(edge._id)ReallocZero1(edge._id  , edges());

   Int  vtx_last=vtxs (); vtx ._elms++;
   Int edge_last=edges(); edge._elms++;

   vtx.pos(vtx_last).set(pos, 0);
   if(e==-1)
   {
      edge.flag(edge_last)=ETQ_R;
      edge.ind (edge_last).set(vtx_last);
   }else
   {
      Bool    sd= ((edge.flag(e)&ETQ_LR)!=ETQ_L);
      VecI2 &ind=   edge.ind (e);
                    vtx .pos ( vtx_last).z=Avg(vtx.pos(ind.x).z, vtx.pos(ind.y).z);
                    edge.ind (edge_last).set(vtx_last, ind.c[sd]); ind.c[sd]=vtx_last;
                    edge.flag(edge_last)=edge.flag(e); if(!sd)edge.flag(edge_last)=EtqFlagSwap(edge.flag(edge_last));
      if(edge.id()){edge.id  (edge_last)=edge.id  (e); if(!sd)edge.id  (edge_last).reverse();}
   }
   return T;
}

MeshBase& subVtx    (Int vtx ); // subtract vertex and try to re-connect 2D edges
MeshBase& MeshBase::subVtx(Int v)
{
   if(InRange(v, vtx))
   {
      Int  edge_ind[2], edge_num=0;
      REPA(edge)
      {
         if(edge.ind(i).c[0]==v
         || edge.ind(i).c[1]==v)if(edge_num<2)edge_ind[edge_num++]=i;
      }
      if(edge_num==2)
      {
         VecI2 id0, id1;
         Vec   nrm;
         UInt  flag;
         Int   i0, i1, e;
         Int  *p;

         e=edge_ind[0]; p=edge.ind(e).c;
         if(p[1]==v){i0=p[0]; if(edge.nrm())nrm = edge.nrm(e); if(edge.flag())flag =            edge.flag(e) ; if(edge.id()) id0=edge.id(e);}
         if(p[0]==v){i0=p[1]; if(edge.nrm())nrm =-edge.nrm(e); if(edge.flag())flag =EtqFlagSwap(edge.flag(e)); if(edge.id()){id0=edge.id(e); id0.reverse();}}
         e=edge_ind[1]; p=edge.ind(e).c;
         if(p[0]==v){i1=p[1]; if(edge.nrm())nrm+= edge.nrm(e); if(edge.flag())flag|=            edge.flag(e) ; if(edge.id()) id1=edge.id(e);}
         if(p[1]==v){i1=p[0]; if(edge.nrm())nrm-= edge.nrm(e); if(edge.flag())flag|=EtqFlagSwap(edge.flag(e)); if(edge.id()){id1=edge.id(e); id1.reverse();}}

         REPA(edge)
         {
            if((edge.ind(i).x==i0 && edge.ind(i).y==i1)
            || (edge.ind(i).x==i1 && edge.ind(i).y==i0))goto is;
         }
         addEdge(VecI2(i0,i1));
         if(edge.nrm ())edge.nrm (edges()-1)=!nrm;
         if(edge.flag())edge.flag(edges()-1)=flag;
         if(edge.id  ())edge.id  (edges()-1)=Max(id0, id1);
      is:;
      }
      removeVtx(v);
   }
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::removeVtx (Int v) {if(InRange(v, vtx )){Memt<Bool> is; is.setNum(vtxs ()); FREPA(vtx )is[i]=(i!=v); keepVtxs (is);} return T;}
MeshBase& MeshBase::removeEdge(Int e) {if(InRange(e, edge)){Memt<Bool> is; is.setNum(edges()); FREPA(edge)is[i]=(i!=e); keepEdges(is);} return T;}
MeshBase& MeshBase::removeTri (Int t) {if(InRange(t, tri )){Memt<Bool> is; is.setNum(tris ()); FREPA(tri )is[i]=(i!=t); keepTris (is);} return T;}
MeshBase& MeshBase::removeQuad(Int q) {if(InRange(q, quad)){Memt<Bool> is; is.setNum(quads()); FREPA(quad)is[i]=(i!=q); keepQuads(is);} return T;}
MeshBase& MeshBase::removeFace(Int f)
{
   if(f&SIGN_BIT)removeQuad(f^SIGN_BIT);
   else          removeTri (f         );
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::keepVtxs(C MemPtr<Bool> &is)
{
   exclude(VTX_DUP|ADJ_ALL);

   Int  *p;
   Memt<Bool> face_is;
   Memt<Int > vtx_remap; SetRemap(vtx_remap, is, vtxs());

   // vtx
   MeshBase temp(CountIs(is), 0, 0, 0, flag());
   temp.copyVtxs(T, is);

   // edge
   face_is.setNum(edges()); FREPA(edge){p=edge.ind(i).c; face_is[i]=(ElmIs(is, p[0]) && ElmIs(is, p[1]));}
   temp.edge._elms=CountIs(face_is); temp.include(flag()&EDGE_ALL);
   temp.copyEdges(T, face_is);
   IndRemap(vtx_remap, temp.edge.ind(), temp.edges());

   // tri
   face_is.setNum(tris()); FREPA(tri){p=tri.ind(i).c; face_is[i]=(ElmIs(is, p[0]) && ElmIs(is, p[1]) && ElmIs(is, p[2]));}
   temp.tri._elms=CountIs(face_is); temp.include(flag()&TRI_ALL);
   temp.copyTris(T, face_is);
   IndRemap(vtx_remap, temp.tri.ind(), temp.tris());

   // quad
   face_is.setNum(quads()); FREPA(quad){p=quad.ind(i).c; face_is[i]=(ElmIs(is, p[0]) && ElmIs(is, p[1]) && ElmIs(is, p[2]) && ElmIs(is, p[3]));}
   temp.quad._elms=CountIs(face_is); temp.include(flag()&QUAD_ALL);
   temp.copyQuads(T, face_is);
   IndRemap(vtx_remap, temp.quad.ind(), temp.quads());

   // end
   Swap(T, temp); return T;
}
MeshBase& MeshBase::keepEdges(C MemPtr<Bool> &is)
{
   Int edges =CountIs(is);
   if( edges!=T.edges())
   {
      exclude(ADJ_ALL); MeshBase temp(0, edges, 0, 0, flag()); temp.copyEdges(T, is);
      Swap(edge, temp.edge);
   }
   return T;
}
MeshBase& MeshBase::keepTris(C MemPtr<Bool> &is)
{
   Int tris =CountIs(is);
   if( tris!=T.tris())
   {
      exclude(ADJ_ALL); MeshBase temp(0, 0, tris, 0, flag()); temp.copyTris(T, is);
      Swap(tri, temp.tri);
   }
   return T;
}
MeshBase& MeshBase::keepQuads(C MemPtr<Bool> &is)
{
   Int quads =CountIs(is);
   if( quads!=T.quads())
   {
      exclude(ADJ_ALL); MeshBase temp(0, 0, 0, quads, flag()); temp.copyQuads(T, is);
      Swap(quad, temp.quad);
   }
   return T;
}
MeshBase& MeshBase::removeVtxs(C MemPtr<Int> &vtxs)
{
   if(vtxs.elms()){Memt<Bool> nis; CreateIsNot(nis, vtxs, T.vtxs()); keepVtxs(nis);}
   return T;
}
MeshBase& MeshBase::removeEdges(C MemPtr<Int> &edges, Bool remove_unused_vtxs)
{
   if(edges.elms()){Memt<Bool> nis; CreateIsNot(nis, edges, T.edges()); keepEdges(nis); if(remove_unused_vtxs)removeUnusedVtxs();}
   return T;
}
MeshBase& MeshBase::removeTris(C MemPtr<Int> &tris, Bool remove_unused_vtxs)
{
   if(tris.elms()){Memt<Bool> nis; CreateIsNot(nis, tris, T.tris()); keepTris(nis); if(remove_unused_vtxs)removeUnusedVtxs();}
   return T;
}
MeshBase& MeshBase::removeQuads(C MemPtr<Int> &quads, Bool remove_unused_vtxs)
{
   if(quads.elms()){Memt<Bool> nis; CreateIsNot(nis, quads, T.quads()); keepQuads(nis); if(remove_unused_vtxs)removeUnusedVtxs();}
   return T;
}
MeshBase& MeshBase::removeFaces(C MemPtr<Int> &faces, Bool remove_unused_vtxs)
{
   if(faces.elms())
   {
      Memt<Bool> tri_is, quad_is; CreateFaceIsNot(tri_is, quad_is, faces, tris(), quads());
      keepTris ( tri_is);
      keepQuads(quad_is);
      if(remove_unused_vtxs)removeUnusedVtxs();
   }
   return T;
}
/******************************************************************************/
static Int SameVtxs(C Int *a, Int a_elms, C Int *b, Int b_elms)
{
   Int same=0; REP(a_elms)
   {
      Int x=a[i];
      REP(b_elms)if(b[i]==x){same++; break;}
   }
   return same;
}
static Bool Continuous(C Int *a, Int a_elms, C Int *b, Int b_elms)
{
   REP(a_elms)
   {
      Int x=a[i], y=a[(i+1)%a_elms];
      REP(b_elms)
      {
         if(y==b[i] && x==b[(i+1)%b_elms])return true; // we need to check reversed order, because they are continuous if one face has the same vtxs but in reversed order
      }
   }
   return false;
}

enum RDSF
{
   RDSF_REMOVE =1<<0, // this face is marked to be removed
   RDSF_LIST   =1<<1, // this face is already listed in the queue
   RDSF_CHECKED=1<<2, // this face is already processed
};

MeshBase& MeshBase::removeDoubleSideFaces(Bool remove_unused_vtxs)
{
   Memt<Byte> face_flag; face_flag.setNumZero(faces());
   {
      Index     face_face; linkFaceFace(face_face);
      Memt<Int> face_list;
      REP(faces()) // iterate all faces
      {
         Int f=i;
      check:
         Byte &ff=face_flag[f];
         if( !(ff&(RDSF_REMOVE|RDSF_CHECKED))) // if this face was not yet removed or checked
         {
            ff|=RDSF_CHECKED; // mark as checked

            VecI4 face_vtx; Int face_vtxs;
            if(InRange(f, tri))
            {
               VecI &tri_vtx=face_vtx.xyz; tri_vtx=tri.ind(f); tri_vtx.remap(vtx.dup()); face_vtxs=3;
            }else
            {
               Int quad_index=f-tris(); face_vtx=quad.ind(quad_index); face_vtx.remap(vtx.dup()); face_vtxs=4;
            }

          C IndexGroup &ig=face_face.group[f]; REPA(ig) // iterate all faces connected to this face
            {
               VecI4 test_face_vtx; Int test_face_vtxs;
               UInt  test_face=ig[i], test_face2;
               if(   test_face&SIGN_BIT) // quad
               {
                  test_face^=SIGN_BIT; test_face2=tris()+test_face; test_face_vtx=quad.ind(test_face); test_face_vtx.remap(vtx.dup()); test_face_vtxs=4;
               }else
               {
                  test_face2=test_face; VecI &tri_vtx=test_face_vtx.xyz; tri_vtx=tri.ind(test_face); tri_vtx.remap(vtx.dup()); test_face_vtxs=3;
               }
               Byte &test_ff=face_flag[test_face2];
               if( !(test_ff&(RDSF_REMOVE|RDSF_CHECKED))) // if this face was not yet removed or checked
               {
                  if(SameVtxs(face_vtx.c, face_vtxs, test_face_vtx.c, test_face_vtxs)>=3) // if this is an overlapping face
                  {
                     test_ff|=RDSF_REMOVE; // remove overlapping face
                  }else
                  if(!(test_ff&RDSF_LIST) // not yet listed
                  && Continuous(face_vtx.c, face_vtxs, test_face_vtx.c, test_face_vtxs))
                  {
                     face_list.add(test_face2); test_ff|=RDSF_LIST; // mark as already listed
                  }
               }
            }

            if(face_list.elms()){f=face_list.pop(); goto check;} // process listed elements first
         }
      }
   }
   {
      Memt<Bool> is; is.reserve(Max(tris(), quads()));
      Byte *quad_flag=face_flag.data()+tris(); is.setNum(quads()); REPAO(is)=!(quad_flag[i]&RDSF_REMOVE); keepQuads(is); // !! delete quads first, because removing tris below changes the number of tris, which is needed to get 'quad_flag' !!
      Byte * tri_flag=face_flag.data()       ; is.setNum( tris()); REPAO(is)=!( tri_flag[i]&RDSF_REMOVE); keepTris (is); // !! delete tris  next !!
   }

   if(remove_unused_vtxs)removeUnusedVtxs();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::mergeFaces(Int a, Int b)
{
   if(a!=b)
   {
      Bool a_quad=FlagTest(a, SIGN_BIT), a_in_range; Int ai=a; if(a_quad){ai^=SIGN_BIT; a_in_range=InRange(ai, quad);}else a_in_range=InRange(ai, tri);
      if(a_in_range)
      {
         Bool b_quad=FlagTest(b, SIGN_BIT), b_in_range; Int bi=b; if(b_quad){bi^=SIGN_BIT; b_in_range=InRange(bi, quad);}else b_in_range=InRange(bi, tri);
         if(b_in_range)
         {
            VecI4 a_ind , b_ind;
            Int   a_inds, b_inds;
            if(a_quad){a_ind=quad.ind(ai); a_inds=4;}else{a_ind.xyz=tri.ind(ai); a_inds=3;}
            if(b_quad){b_ind=quad.ind(bi); b_inds=4;}else{b_ind.xyz=tri.ind(bi); b_inds=3;}
            REPD(a_ofs, a_inds)
            {
               Int ai=a_ind.c[a_ofs];
               REPD(b_ofs, b_inds)if(b_ind.c[b_ofs]==ai)
               {
                  Int a_next=a_ind.c[(a_ofs+1       )%a_inds];
                  Int b_prev=b_ind.c[(b_ofs-1+b_inds)%b_inds];
                  if(a_next==b_prev)
                  {
                     // first remove faces
                     Int faces[]={a, b}; removeFaces(faces, false);

                     // add new faces
                     if(!a_quad && !b_quad) // 2 triangles
                     {
                        VecI4 q(a_ind.c[(a_ofs+2)%a_inds], ai, b_ind.c[(b_ofs+1)%b_inds], a_next);
                        if(vtx.pos())REP(4) // check if any of the points is inline
                        {
                         C Vec &p0=vtx.pos(q.c[i]), &p1=vtx.pos(q.c[(i+1)%4]), &p2=vtx.pos(q.c[(i+2)%4]);
                           if(DistPointStr(p1, p0, !(p2-p0))<=EPS)
                           {
                              addTri(VecI(q.c[i], q.c[(i+2)%4], q.c[(i+3)%4]));
                              goto added;
                           }
                        }
                        addQuad(q);
                        added:;
                     }else
                     if(a_quad && b_quad)addQuad(VecI4(a_ind.c[(a_ofs+2)%a_inds], a_ind.c[(a_ofs+3)%a_inds], b_ind.c[(b_ofs+1)%b_inds], b_ind.c[(b_ofs+2)%b_inds]));else // 2 quads
                     { // one triangle and one quad
                        if(b_quad) // put quad to 'a'
                        {
                           Swap(a_ofs , b_ofs );
                           Swap(a_ind , b_ind );
                           Swap(a_inds, b_inds);
                           a_ofs=(a_ofs-1+a_inds)%a_inds;
                           b_ofs=(b_ofs+1       )%b_inds;
                        }
                        Int   b=b_ind.c[(b_ofs+1)%b_inds]; // set 'b' triangle outer vtx
                        VecI4 face(a_ind.c[(a_ofs+2)%a_inds], a_ind.c[(a_ofs+3)%a_inds],  // 'a' quad outer vtxs
                                   a_ind.c[ a_ofs          ], a_ind.c[(a_ofs+1)%a_inds]); // 'a' quad inner vtxs
                        if(vtx.pos()) // there are 3 possible quads (original, or with one corner replaced with point from the triangle), pick one with the biggest area
                        {
                           VecI4 f1=face; f1.w=b;
                           VecI4 f2=face; f2.z=b;
                           Flt area=(GetNormalU(vtx.pos(face.x), vtx.pos(face.y), vtx.pos(face.w))+GetNormalU(vtx.pos(face.z), vtx.pos(face.w), vtx.pos(face.y))).length2(),
                               a1  =(GetNormalU(vtx.pos(f1  .x), vtx.pos(f1  .y), vtx.pos(f1  .w))+GetNormalU(vtx.pos(f1  .z), vtx.pos(f1  .w), vtx.pos(f1  .y))).length2(),
                               a2  =(GetNormalU(vtx.pos(f2  .x), vtx.pos(f2  .y), vtx.pos(f2  .w))+GetNormalU(vtx.pos(f2  .z), vtx.pos(f2  .w), vtx.pos(f2  .y))).length2();
                           if(a1>area){face=f1; area=a1;}
                           if(a2>area){face=f2; area=a2;}
                        }
                        addQuad(face);
                     }

                     // remove unused vtxs
                     removeUnusedVtxs();
                     return T;
                  }
               }
            }
         }
      }
   }
   return T;
}
/******************************************************************************/
// OPTIMIZE
/******************************************************************************/
Bool MeshBase::removeUnusedVtxs(Bool include_edge_references)
{
 C Int *p;
   Memt<Bool> is; is.setNumZero(vtxs());
   if(include_edge_references)REPA(edge){p=edge.ind(i).c; REPD(j, 2)is[p[j]]=true;}
                              REPA(tri ){p=tri .ind(i).c; REPD(j, 3)is[p[j]]=true;}
                              REPA(quad){p=quad.ind(i).c; REPD(j, 4)is[p[j]]=true;}

   Int vtxs =CountIs(is);
   if( vtxs!=T.vtxs())
   {
      if(!include_edge_references)exclude(EDGE_ALL);
      MeshBase  temp(vtxs, 0, 0, 0, flag()&(~VTX_DUP)); temp.copyVtxs(T, is);
      Memt<Int> remap;
      SetRemap(remap, is        , T.vtxs ());
      IndRemap(remap, edge.ind(), T.edges());
      IndRemap(remap, tri .ind(), T.tris ());
      IndRemap(remap, quad.ind(), T.quads());
      Swap(vtx, temp.vtx);
      return true;
   }
   return false;
}
/******************************************************************************/
MeshBase& MeshBase::removeDoubleEdges()
{
   if(edge.flag())
   {
      Memt<Bool> is; is.setNum(edges()); REPAO(is)=((edge.flag(i)&ETQ_LR)!=ETQ_LR);
      keepEdges       (is  );
      removeUnusedVtxs(true);
   }
   return T;
}
/******************************************************************************/
struct TriFromQuad
{
   Int  quad_index;
   VecI  vtx_index;

   void from(Int quad_index, Int p0, Int p1, Int p2)
   {
      T.quad_index=quad_index;
         vtx_index.set(p0, p1, p2);
   }
};
MeshBase& MeshBase::removeDegenerateFaces(Flt eps)
{
   Memt<Bool> tri_is, quad_is;
    tri_is.setNum(tris ());
   quad_is.setNum(quads());

   // triangles
   REPA(tri)if(tri_is[i]=tri.ind(i).allDifferent())
   {
    C Int *p=tri.ind(i).c;
      if(!Tri(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[2])).valid(eps))tri_is[i]=false;
   }

   // quads
   Memc<TriFromQuad> tri_from_quad;

   REPA(quad)
   {
    C Int *p=quad.ind(i).c;
      if(Quad(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[2]), vtx.pos(p[3])).valid(eps))
      {
         quad_is[i]=true;
      }else
      {
         quad_is[i]=false;
         Int p0=p[0], p1=p[1], p2=p[2], p3=p[3];
       C Vec &vp0=vtx.pos(p0), &vp1=vtx.pos(p1), &vp2=vtx.pos(p2), &vp3=vtx.pos(p3);
         if(Tri(vp0, vp1, vp3).valid(eps)){tri_from_quad.New().from(i, p0, p1, p3); if(Tri(vp1, vp2, vp3).valid(eps))tri_from_quad.New().from(i, p1, p2, p3);}else // second tri can also be valid, this can happen for <--| case when 2 tris are valid but quad is not
         if(Tri(vp0, vp1, vp2).valid(eps)){tri_from_quad.New().from(i, p0, p1, p2); if(Tri(vp2, vp3, vp0).valid(eps))tri_from_quad.New().from(i, p2, p3, p0);}else //                                                    \ |
         if(Tri(vp1, vp2, vp3).valid(eps)) tri_from_quad.New().from(i, p1, p2, p3);else // no need to check for 013 because it was checked at start                                                                    \|
         if(Tri(vp2, vp3, vp0).valid(eps)) tri_from_quad.New().from(i, p2, p3, p0);     // no need to check for 012 because it was checked at start
      }
   }

   Int src_tris =CountIs( tri_is),
       src_quads=CountIs(quad_is);
   if( src_tris!=tris() || src_quads!=quads())
   {
      exclude(ADJ_ALL);

      // set flag
      UInt flag=T.flag();
      if(tri_from_quad.elms())
      {
         if(quad.flag())flag|=TRI_FLAG;
         if(quad.id  ())flag|=TRI_ID  ;
         if(quad.nrm ())flag|=TRI_NRM ;
      }

      // create temp mesh
      MeshBase temp(0, 0, src_tris+tri_from_quad.elms(), src_quads, flag);

      // copy original data
      temp.copyTris (T,  tri_is);
      temp.copyQuads(T, quad_is);

      // set missing data
      if(temp.tri.flag() && !tri.flag())ZeroN(temp.tri.flag(), src_tris);
      if(temp.tri.id  () && !tri.id  ())ZeroN(temp.tri.id  (), src_tris);
      if(temp.tri.nrm () && !tri.nrm ())REP(src_tris)
      {
       C VecI &ind=temp.tri.ind(i);
         temp.tri.nrm(i)=GetNormal(vtx.pos(ind.x), vtx.pos(ind.y), vtx.pos(ind.z));
      }

      // copy additional triangles from quads
      REPA(tri_from_quad)
      {
       C TriFromQuad &tfq=tri_from_quad[i];
                            temp.tri.ind (src_tris+i)=tfq.vtx_index;
         if(temp.tri.flag())temp.tri.flag(src_tris+i)=(quad.flag() ? quad.flag(tfq.quad_index) : 0);
         if(temp.tri.id  ())temp.tri.id  (src_tris+i)=(quad.id  () ? quad.id  (tfq.quad_index) : 0);
         if(temp.tri.nrm ())temp.tri.nrm (src_tris+i)=(quad.nrm () ? quad.nrm (tfq.quad_index) : GetNormal(vtx.pos(tfq.vtx_index.x), vtx.pos(tfq.vtx_index.y), vtx.pos(tfq.vtx_index.z)));
      }

      // swap new data with old one
      Swap(temp.tri , tri );
      Swap(temp.quad, quad);
   }
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::removeSingleFaces(Flt fraction)
{
   setAdjacencies(true, false);

   Flt              f=0.25f;
   Memt<Bool>  tri_is;  tri_is.setNum( tris()); REPA(tri ){C VecI  &a=tri .adjFace(i); if(a.x!=-1 || a.y!=-1 || a.z!=-1           ) tri_is[i]=true;else{f+=fraction; if( tri_is[i]=(f>=1))f--;}} // compare to -1 and not >=0 because it can have SIGN_BIT
   Memt<Bool> quad_is; quad_is.setNum(quads()); REPA(quad){C VecI4 &a=quad.adjFace(i); if(a.x!=-1 || a.y!=-1 || a.z!=-1 || a.w!=-1)quad_is[i]=true;else{f+=fraction; if(quad_is[i]=(f>=1))f--;}} // compare to -1 and not >=0 because it can have SIGN_BIT

   keepTris ( tri_is);
   keepQuads(quad_is);

   return T;
}
/******************************************************************************/
struct VtxEdgeLink
{
   Int edges;
   Int vtx[2], edge[2];
};
MeshBase& MeshBase::weldInlineEdges(Flt cos_edge, Flt cos_vtx, Bool z_test)
{
   CHS(cos_edge);

   Int         *p;
   Memt<Bool>   edge_is; edge_is.setNum    (edges());
   Memt<VtxEdgeLink> vi; vi     .setNumZero(vtxs ());

   // link vtx->vtx, edge
   FREPA(edge)
   {
      edge_is[i]=true;
      p=edge.ind(i).c;
      FREPD(j, 2)
      {
         VtxEdgeLink &v=vi[p[j]];
         if(v.edges<2)
         {
            v.vtx [v.edges]=p[!j];
            v.edge[v.edges]=i;
         }
         v.edges++;
      }
   }

   // edge is
   FREPA(vtx)if(vi[i].edges==2)
   if(CosBetween(vtx.pos(vi[i].vtx[0]).xy, vtx.pos(i).xy,
                 vtx.pos(vi[i].vtx[1]).xy)<=cos_edge)
   {
      if(cos_vtx>-1 && vtx.nrm() && Dot(vtx.nrm(vi[i].vtx[0]).xy, vtx.nrm(vi[i].vtx[1]).xy)<cos_vtx)continue;
      if(z_test)
      {
         Flt s=    LerpR(vtx.pos(vi[i].vtx[0]).xy, vtx.pos(vi[i].vtx[1]).xy, vtx.pos(i).xy);
         if(!Equal(Lerp (vtx.pos(vi[i].vtx[0]).z , vtx.pos(vi[i].vtx[1]).z, s), vtx.pos(i).z))continue;
      }

      VtxEdgeLink &v=vi[i];
      Int l_vtx =v.vtx [0],
          r_vtx =v.vtx [1],
          l_edge=v.edge[0],
          r_edge=v.edge[1];
        edge_is[r_edge]=false;
      p=edge.ind(l_edge).c   ; p[(p[0]!=i     )]=r_vtx;
      p=      vi[l_vtx ].vtx ; p[(p[0]!=i     )]=r_vtx;
      p=      vi[r_vtx ].vtx ; p[(p[0]!=i     )]=l_vtx;
      p=      vi[r_vtx ].edge; p[(p[0]!=r_edge)]=l_edge;
   }

   // create mesh
   Int edges=CountIs(edge_is); if(edges!=T.edges())
   {
      exclude(VTX_DUP|FACE_ALL|ADJ_ALL);
      MeshBase temp(0, edges, 0, 0, flag()); temp.copyEdges(T, edge_is);
      Swap(edge, temp.edge);
      removeUnusedVtxs();
   }
   return T;
}
/******************************************************************************/
static Bool VertexValueTest(MeshBase &mshb, Int mid, Int pa, Int pb) // return true if shouldn't be welded
{
   Flt length=Dist(mshb.vtx.pos(pa), mshb.vtx.pos(pb));
   if( length>EPS)
   {
      Flt step=Dist(mshb.vtx.pos(mid), mshb.vtx.pos(pa))/length;
      if( mshb.vtx.material())
      {
         VecB4 m =Lerp(mshb.vtx.material(pa ), mshb.vtx.material(pb), step),
               mm=     mshb.vtx.material(mid);
                const Int mtrl_eps=5;
         if(Abs(m.x-mm.x)>mtrl_eps
         || Abs(m.y-mm.y)>mtrl_eps
         || Abs(m.z-mm.z)>mtrl_eps
         || Abs(m.w-mm.w)>mtrl_eps)return true;
      }
      if(mshb.vtx.color())
      {
         Color c =Lerp(mshb.vtx.color(pa ), mshb.vtx.color(pb), step),
               cm=     mshb.vtx.color(mid);
                const Int col_eps=3;
         if(Abs(c.a-cm.a)>col_eps
         || Abs(c.r-cm.r)>col_eps
         || Abs(c.g-cm.g)>col_eps
         || Abs(c.b-cm.b)>col_eps)return true;
      }
      const Flt tex_eps2=Sqr(0.01f);
      if(mshb.vtx.tex0() && Dist2(Lerp(mshb.vtx.tex0(pa), mshb.vtx.tex0(pb), step), mshb.vtx.tex0(mid))>tex_eps2)return true;
      if(mshb.vtx.tex1() && Dist2(Lerp(mshb.vtx.tex1(pa), mshb.vtx.tex1(pb), step), mshb.vtx.tex1(mid))>tex_eps2)return true;
      if(mshb.vtx.tex2() && Dist2(Lerp(mshb.vtx.tex2(pa), mshb.vtx.tex2(pb), step), mshb.vtx.tex2(mid))>tex_eps2)return true;
   }
   return false;
}
MeshBase& MeshBase::weldCoplanarFaces(Flt cos_face, Flt cos_vtx, Bool safe, Flt max_face_length)
{
   setAdjacencies(true, false);
   setFaceNormals();
   exclude       (EDGE_ADJ_FACE|FACE_ADJ_EDGE);
   Bool  vtx_value_test=(vtx.tex0() || vtx.tex1() || vtx.tex2() || vtx.material() || vtx.color()),
       face_length_test=(max_face_length>=0); max_face_length*=max_face_length; // it's now squared
 C Vec *pos=vtx.pos();
   Memt<Bool>  tri_is;  tri_is.setNum(tris ()); SetMemN( tri_is.data(), 1,  tri_is.elms());
   Memt<Bool> quad_is; quad_is.setNum(quads()); SetMemN(quad_is.data(), 1, quad_is.elms());

   // Find these triangles : "/|\"
   for(Int i=0; i<tris(); )if(!tri_is[i])i++;else // for each tri
   {
      Bool  found=false;
    C Vec  &nrm  =tri.nrm    (i);
      Int  *af   =tri.adjFace(i).c;
      REPD(vi, 3) // for each tris adjacent tri
      {
         Int face =af[vi];
         if( face!=-1 && !(face&SIGN_BIT)) // compare to -1 and not >=0 because it can have SIGN_BIT
         {
            // co-planar test
            if(Dot(tri.nrm(face), nrm)<cos_face)continue;

            // same vertex test
            Int *af2=tri.adjFace(face).c,
                *p  =tri.ind    (i   ).c,
                *p2 =tri.ind    (face).c;
            Int vi2; for(vi2=3; --vi2>=0; )if(af2[vi2]==i)break; if(vi2<0)continue;
            if(p[vi]!=p2[(vi2+1)%3] || p[(vi+1)%3]!=p2[vi2])continue;

            // vertex normal test
            if(cos_vtx>-1 && vtx.nrm() && Dot(vtx.nrm(p[(vi+2)%3]), vtx.nrm(p2[(vi2+2)%3]))<cos_vtx)continue;

            // safety test (faces without other neighbors)
            if(safe)if(af[(vi+1)%3]!=-1 || af2[(vi2+2)%3]!=-1)continue; // compare to -1 and not >=0 because it can have SIGN_BIT

            // edges inline test
            if(DistPointEdge(pos[p[(vi+1)%3]], pos[p[(vi+2)%3]], pos[p2[(vi2+2)%3]])>EPS)continue;

            // face length test
            if(face_length_test)
               if(Dist2(pos[p[ vi     ]], pos[p2[(vi2+2)%3]])>max_face_length
               || Dist2(pos[p[(vi+2)%3]], pos[p2[(vi2+2)%3]])>max_face_length)continue;

            // vertex values test
            if(vtx_value_test)if(VertexValueTest(T, p[(vi+1)%3], p[(vi+2)%3], p2[(vi2+2)%3]))continue;

            // remap points & adj
             p[(vi+1)%3]= p2[(vi2+2)%3];
            af[ vi     ]=af2[(vi2+1)%3];

            // fix adjacency
            REPD(a, 3)if(af2[a]!=-1) // compare to -1 and not >=0 because it can have SIGN_BIT
            {
               Int face3=af2[a];
               if( face3&SIGN_BIT){REPD(j, 4)if(quad.adjFace(face3^SIGN_BIT).c[j]==face)quad.adjFace(face3^SIGN_BIT).c[j]=i;}
               else               {REPD(j, 3)if(tri .adjFace(face3         ).c[j]==face)tri .adjFace(face3         ).c[j]=i;}
            }

            // remove face
            tri_is[face]=0;
            found=true;
            break;
         }
      }
      if(!found)i++;
   }

   // quads
   for(Int i=0; i<quads(); )if(!quad_is[i])i++;else // for each quad
   {
    C Vec  &nrm  =quad.nrm(i);
      Bool  found=false;
      Int  *af   =quad.adjFace(i).c;
      REPD(vi, 4) // for each quads adjacent face
      {
         Int face =af[vi];
         if( face!=-1) // compare to -1 and not >=0 because it can have SIGN_BIT
         {
            // co-planar test
            Int vtxs;
            if(face&SIGN_BIT){if(Dot(quad.nrm(face^SIGN_BIT), nrm)<cos_face)continue; vtxs=4;}
            else             {if(Dot(tri .nrm(face         ), nrm)<cos_face)continue; vtxs=3;}

            // same vertex test
            Int *af2, *p2,   *p =quad.ind(i            ).c;
            if(face&SIGN_BIT){p2=quad.ind(face^SIGN_BIT).c; af2=quad.adjFace(face^SIGN_BIT).c;}
            else             {p2=tri .ind(face         ).c; af2=tri .adjFace(face         ).c;}
            Int vi2; for(vi2=vtxs; --vi2>=0; )if(af2[vi2]==(i^SIGN_BIT))break; if(vi2<0)continue;
            if(p[vi]!=p2[(vi2+1)%vtxs] || p[(vi+1)%4]!=p2[vi2])continue;

            // vertex normal test
            if(cos_vtx>-1 && vtx.nrm())
            {
             C Vec &n_2 =vtx.nrm(p [(vi +2)%   4]),
                   &n_3 =vtx.nrm(p [(vi +3)%   4]),
                   &n2_2=vtx.nrm(p2[(vi2+2)%vtxs]);
               if(Dot(n_2, n2_2)<cos_vtx
               || Dot(n_3, n2_2)<cos_vtx)continue;
               if(face&SIGN_BIT)
               {
                C Vec &n2_3=vtx.nrm(p2[(vi2+3)%4]);
                  if(Dot(n_2, n2_3)<cos_vtx
                  || Dot(n_3, n2_3)<cos_vtx)continue;
               }
            }

            // safety test (faces without other neighbors)
            if(safe)
            {
                                if(af [(vi +1)%4]!=-1 || af [(vi +3)%4]!=-1)continue; // compare to -1 and not >=0 because it can have SIGN_BIT
               if(face&SIGN_BIT)if(af2[(vi2+1)%4]!=-1 || af2[(vi2+3)%4]!=-1)continue; // compare to -1 and not >=0 because it can have SIGN_BIT
            }

            // face length test
            if(face_length_test)
            {
               if(Dist2(pos[p[(vi+2)%4]], pos[p2[(vi2+2)%vtxs]])>max_face_length
               || Dist2(pos[p[(vi+3)%4]], pos[p2[(vi2+2)%vtxs]])>max_face_length)continue;
               if(face&SIGN_BIT)
                  if(Dist2(pos[p[(vi+2)%4]], pos[p2[(vi2+3)%4]])>max_face_length
                  || Dist2(pos[p[(vi+3)%4]], pos[p2[(vi2+3)%4]])>max_face_length)continue;
            }

            // edges inline test
            if(face&SIGN_BIT)
            {
               if(DistPointEdge(pos[p [vi ]], pos[p[(vi+3)%4]], pos[p2[(vi2+2)%4]])>EPS)continue;
               if(DistPointEdge(pos[p2[vi2]], pos[p[(vi+2)%4]], pos[p2[(vi2+3)%4]])>EPS)continue;

               // vertex values test
               if(vtx_value_test)
               {
                  if(VertexValueTest(T, p [vi ], p[(vi+3)%4], p2[(vi2+2)%4])
                  || VertexValueTest(T, p2[vi2], p[(vi+2)%4], p2[(vi2+3)%4]))continue;
               }

               // remap points & adj
                p[ vi     ]= p2[(vi2+2)%4];
                p[(vi+1)%4]= p2[(vi2+3)%4];
               af[ vi     ]=af2[(vi2+2)%4];
            }else
            {
               if(DistPointEdge (pos[p[vi]], pos[p[(vi+3)%4]], pos[p2[(vi2+2)%3]])<=EPS
               && DistPointPlane(pos[p2[(vi2+2)%3]], pos[p2[vi2]], CrossN(nrm, pos[p2[vi2]]-pos[p[(vi+2)%4]]))<=EPS)
               {
                  if(safe)if(af2[(vi2+1)%3]!=-1)continue; // compare to -1 and not >=0 because it can have SIGN_BIT

                  // remap points & adj
                   p[vi]= p2[(vi2+2)%3];
                  af[vi]=af2[(vi2+2)%3];
               }else
               if(DistPointEdge (pos[p2[vi2]], pos[p[(vi+2)%4]], pos[p2[(vi2+2)%3]])<=EPS
               && DistPointPlane(pos[p2[(vi2+2)%3]], pos[p[vi]], CrossN(nrm, pos[p[(vi+3)%4]]-pos[p[vi]]))<=EPS)
               {
                  if(safe)if(af2[(vi2+2)%3]!=-1)continue; // compare to -1 and not >=0 because it can have SIGN_BIT

                  // remap points & adj
                   p[(vi+1)%4]= p2[(vi2+2)%3];
                  af[ vi     ]=af2[(vi2+1)%3];
               }else continue;
            }

            // fix adjacency
            REPD(a, vtxs)
            {
               Int face3 =af2[a];
               if( face3!=-1) // compare to -1 and not >=0 because it can have SIGN_BIT
               {
                  if( face3&SIGN_BIT){REPD(j, 4)if(quad.adjFace(face3^SIGN_BIT).c[j]==face)quad.adjFace(face3^SIGN_BIT).c[j]=(i^SIGN_BIT);}
                  else               {REPD(j, 3)if(tri .adjFace(face3         ).c[j]==face)tri .adjFace(face3         ).c[j]=(i^SIGN_BIT);}
               }
            }

            // remove face
            if(face&SIGN_BIT)quad_is[face^SIGN_BIT]=0;
            else              tri_is[face         ]=0;
            found=true;
            break;
         }
      }
      if(!found)i++;
   }

   exclude         (EDGE_ALL|ADJ_ALL);
   keepTris        ( tri_is);
   keepQuads       (quad_is);
   removeUnusedVtxs(       );
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
