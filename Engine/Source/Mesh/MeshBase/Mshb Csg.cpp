/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// if it won't work then probably 'edgeToTri' should be converted to Dbl version
/******************************************************************************/
// CSG
/******************************************************************************/
#if DEBUG
Int  F=-1,
     P=-1,
     E=-1,
     O=-1;
Bool DrawEdges,
     DrawPolys,
     DrawTris,
     DrawVtxs,
     DrawInside;
#endif
/******************************************************************************/
enum // SOLID_FLAG
{
   SOLID_CUT=1<<31,
};
/******************************************************************************/
UInt SolidSwap(UInt flag)
{
   UInt out=(flag&~SOLID_ALL);
   if(flag&SOLID_AL )out|=SOLID_AR;
   if(flag&SOLID_AR )out|=SOLID_AL;
   if(flag&SOLID_BL )out|=SOLID_BR;
   if(flag&SOLID_BR )out|=SOLID_BL;
   if(flag&SOLID_NAL)out|=SOLID_NAR;
   if(flag&SOLID_NAR)out|=SOLID_NAL;
   if(flag&SOLID_NBL)out|=SOLID_NBR;
   if(flag&SOLID_NBR)out|=SOLID_NBL;
   return out;
}
/******************************************************************************/
struct CSG // Constructive Solid Geometry
{
   STRUCT(Vtx , VtxFull) // vertex with enhanced position precision
   //{
      VecD posd; // position (double precision)

      void from(C MeshBase &mshb, Int i)
      {
         super::from(mshb, i);
         posd=pos;
      }
      void to(MeshBase &mshb, Int i)C
      {
         super::to(mshb, i);
         if(InRange(i, mshb.vtx) && mshb.vtx.pos())mshb.vtx.pos(i)=posd;
      }
   };
   struct Face
   {
      VecI ind ; // triangle vertex indexes
      Int  id  ;
      UInt flag; // SOLID_FLAG
      BoxI boxi;
      Int  last_tested_with;

      void set(C VecI &ind, Int id, UInt flag) {T.ind=ind; T.id=id; T.flag=flag;}

      Face()
      {
         last_tested_with=-1;
      }
   };
   struct FaceEdge
   {
      UInt     flag   ;
      EdgeD2_I edge_2d;
      EdgeD    edge   ;

      FaceEdge& set(C EdgeD &edge, C MatrixD &face_matrix)
      {
         T.edge=edge;
         T.edge_2d.set(face_matrix.convert(edge, true));
         return T;
      }
      FaceEdge(){flag=0;}
   };
   struct FacePos
   {
      VecD2 pos_2d;
      VecD  pos   ;
   };
   struct FaceEdgeI
   {
      VecI2 index; // 'face_pos' index
      UInt  flag ; // ETQ_FLAG

      void set(Int index0, Int index1, UInt flag) {T.index.set(index0, index1); T.flag=flag;}
   };

   Memb<Vtx > vtx      ; // list of vertexes
   Memb<Face> face     , // list of faces
              face_over, // list of possibly overlapped faces
              face_done; // list of processed faces
   Box        box_ab   ; // Box(a) & Box(b), operation box
   Boxes      boxes_ab ;
   Memc<Int> *box_face ;
   CutsCache  cuts_cache_a,
              cuts_cache_b;

   // per face
   Memc<FaceEdge > face_edge ;
   Memc<FacePos  > face_pos  ;
   Memc<FaceEdgeI> face_edgei;

   void addTri(C VecI &ind, Int id, UInt flag)
   {
      Box box =vtx[ind.x].pos;
          box|=vtx[ind.y].pos;
          box|=vtx[ind.z].pos;
      if(CutsEps(box, box_ab))face     .New().set(ind, id, flag); // add for processing
      else                    face_done.New().set(ind, id, flag); // add as  processed
   }
   void addMesh(C MeshBase &mshb, UInt flag)
   {
      Int vtxs=vtx.elms();
      FREPA(mshb.vtx )T.vtx.New().from(mshb, i);
      FREPA(mshb.tri )addTri(mshb.tri.ind(i)+vtxs, mshb.tri.id() ? mshb.tri.id(i) : -1, flag);
      FREPA(mshb.quad)
      {
         addTri(mshb.quad.ind(i).tri0()+vtxs, mshb.quad.id() ? mshb.quad.id(i) : -1, flag);
         addTri(mshb.quad.ind(i).tri1()+vtxs, mshb.quad.id() ? mshb.quad.id(i) : -1, flag);
      }
   }
   void setFaceBox(Face &face)
   {
      VecI &ind =face.ind;
      Box   box =vtx [ind.x].pos;
            box|=vtx [ind.y].pos;
            box|=vtx [ind.z].pos;

      face.boxi=boxes_ab.coords(box.extend(EPS));
   }
   void assignFacesToBoxes()
   {
      REPA(face)
      {
         setFaceBox(face[i]);
         BoxI &boxi=face[i].boxi;
         for(Int z=boxi.min.z; z<=boxi.max.z; z++)
         for(Int y=boxi.min.y; y<=boxi.max.y; y++)
         for(Int x=boxi.min.x; x<=boxi.max.x; x++)box_face[boxes_ab.index(VecI(x, y, z))].New()=i;
      }
   }
   Int getPos(C VecD &pos, C MatrixD &face_matrix)
   {
      // find existing
      REPA(face_pos)if(Equal(face_pos[i].pos, pos))return i;

      // add new
      Int      i =face_pos.elms();
      FacePos &fp=face_pos.New ();
      fp.pos   =pos;
      fp.pos_2d=face_matrix.convert(pos, true);
      return i;
   }
   void triFromEdges(Face &face, TriD &face_tri, MatrixD &face_matrix, Memb<Face> &faces)
   {
      // add base edges
      face_edge.New().set(face_tri.edge0(), face_matrix).flag=ETQ_R;
      face_edge.New().set(face_tri.edge1(), face_matrix).flag=ETQ_R;
      face_edge.New().set(face_tri.edge2(), face_matrix).flag=ETQ_R;

      // cut edges
      Memc<Dbl> steps;
      REPAD(i, face_edge)
      {
         FaceEdge &face_edge_i=face_edge[i];
         REPAD(j,  face_edge)if(i!=j)
         {
            EdgeD2 cuts;
            REPD(c, CutsEdgeEdge(face_edge_i.edge_2d, face_edge[j].edge_2d, &cuts))
            {
               Dbl step=DistPointPlane(cuts.p[c], face_edge_i.edge_2d.p[0], face_edge_i.edge_2d.dir);
               if( step>EPSD && step<face_edge_i.edge_2d.length-EPSD)steps.New()=step/face_edge_i.edge_2d.length;
            }
         }
         Sort(steps.data(), steps.elms());
         Int p0=getPos(face_edge_i.edge.p[0], face_matrix), p1;
         FREPA(steps)
         {
            p1=getPos(face_edge_i.edge.lerp(steps[i]), face_matrix);
            if(p1!=p0)
            {
               face_edgei.New().set(p0, p1, face_edge_i.flag);
               p0=p1;
            }
         }
         p1=getPos(face_edge_i.edge.p[1], face_matrix); if(p1!=p0)face_edgei.New().set(p0, p1, face_edge_i.flag);

         // finish
         steps.clear();
      }

      // join edges
      FREPAD(i, face_edgei)
      {
         FaceEdgeI &face_edgei_i=face_edgei[i]; Int pi0=face_edgei_i.index.x, pi1=face_edgei_i.index.y;
         for(Int j=i+1; j<face_edgei.elms(); )
         {
            FaceEdgeI &face_edgei_j=face_edgei[j]; Int pj0=face_edgei_j.index.x, pj1=face_edgei_j.index.y;
            if((pi0==pj0 && pi1==pj1)  // equal in the same order
            || (pi1==pj0 && pi0==pj1)) // equal in reversed order
            {
               face_edgei_i.flag|=((pi0==pj0) ? face_edgei_j.flag : EtqFlagSwap(face_edgei_j.flag)); // if equal in reversed order then swap the solid flag
               face_edgei.remove(j);
            }else j++;
         }

         // adjust flags
         if(!face_edgei_i.flag                                 )face_edgei_i.flag|= ETQ_LR;else // if there's  no side , then add both of them
         if( face_edgei_i.flag&ETQ_L && face_edgei_i.flag&ETQ_R)face_edgei_i.flag&=~ETQ_LR;     // if it has both sides, then remove them
      }

      // create triangles
      {
         // codes assume that 'mshb' will not change the order of vertexes, but only triangles will be created from edges (edge->tri)
         MeshBase mshb(face_pos.elms(), face_edgei.elms(), 0, 0, EDGE_FLAG);
         REPA(mshb.vtx)
         {
            mshb.vtx.pos(i).set(face_pos[i].pos_2d, 0);
         }
         REPA(mshb.edge)
         {
            FaceEdgeI    &src=face_edgei[i];
            mshb.edge.ind (i)=src.index;
            mshb.edge.flag(i)=src.flag ;
         }

      #if DEBUG
         // draw edges
         if(DrawEdges)REPA(face_edgei)if(E<0 || E==i)
         {
            EdgeD edge(face_pos[face_edgei[i].index.x].pos, face_pos[face_edgei[i].index.y].pos);
            edge.draw(RED);
            if(E>=0)
            {
               edge.p[0].draw(RED);
               edge.p[1].draw(RED);
            }
         }

         // draw polys
         if(DrawPolys)
         {
            SetMatrix((Matrix)face_matrix);
            Meml<Poly> polys; mshb.edgeToPoly(polys);
            REPA(polys)if(P<0 || P==i)polys[i].draw3D(GREEN);
            if(InRange(P, polys))
            {
               REPA(polys[P].vtx)
               {
                  Vec  pos=mshb.vtx.pos(polys[P].vtx[i].index);
                  Vec2 screen; if(PosToScreenM(pos, screen))D.text(screen, S+i);                  
               }
            }
            SetMatrix();
         }
      #endif

         mshb.edgeToTri(false);

         // convert mshb vtxs/triangles to csg vtxs/faces
         Int vtxs=vtx.elms();
         FREPA(mshb.vtx)
         {
            Vtx &v=vtx.New();
            v.posd=face_pos[i].pos;
            v.lerp(vtx[face.ind.x], vtx[face.ind.y], vtx[face.ind.z], (Vec)TriBlend(v.posd, face_tri, true));
         }
         FREPA(mshb.tri)
         {
            faces.New().set(mshb.tri.ind(i)+vtxs, face.id, face.flag|SOLID_CUT);
         }

      #if DEBUG
         // draw tris
         if(DrawTris)REPA(mshb.tri)
         {
            VecI &ind=faces[faces.elms()-1-i].ind;
            TriD(vtx[ind.x].posd, vtx[ind.y].posd, vtx[ind.z].posd).draw(ColorI(i), true);
         }

         // draw vtxs
         if(DrawVtxs)if(F>=0)REPA(face_pos)
         {
            Vec  p=face_pos[i].pos;
            Vec2 s; if(PosToScreen(p, s))D.text(s, S+i);
         }
      #endif
      }

      // finish
      face_edgei.clear();
      face_pos  .clear();
   }
   void cutFaces(Bool detect_self_intersections)
   {
      REPA(face) // for each face
      {
         Face   &face=T.face[i];
         BoxI   &boxi=  face.boxi;
         TriD    face_tri  (vtx[face.ind.x].posd, vtx[face.ind.y].posd, vtx[face.ind.z].posd);
         PlaneD  face_plane(face_tri.p[0], face_tri.n);
         MatrixD face_matrix; face_matrix.setPosDir(face_tri.p[0], -face_tri.n);
         PlaneD  face_tri_planes[3]=
                 {
                    PlaneD(face_tri.p[0], CrossN(face_tri.n, face_tri.p[0]-face_tri.p[1])),
                    PlaneD(face_tri.p[1], CrossN(face_tri.n, face_tri.p[1]-face_tri.p[2])),
                    PlaneD(face_tri.p[2], CrossN(face_tri.n, face_tri.p[2]-face_tri.p[0])),
                 };
       /*TriD2   face_tri_2d( face_matrix.convert(face_tri.p[0], true), face_matrix.convert(face_tri.p[1], true), face_matrix.convert(face_tri.p[2], true));
         PlaneD2 face_tri_planes[3]=
                 {
                    PlaneD2(face_tri_2d.p[0], PerpN(face_tri_2d.p[0]-face_tri_2d.p[1])),
                    PlaneD2(face_tri_2d.p[1], PerpN(face_tri_2d.p[1]-face_tri_2d.p[2])),
                    PlaneD2(face_tri_2d.p[2], PerpN(face_tri_2d.p[2]-face_tri_2d.p[0])),
                 };*/

         Bool overlapped=false;
         for(Int z=boxi.min.z; z<=boxi.max.z; z++) // for every box where face lies
         for(Int y=boxi.min.y; y<=boxi.max.y; y++)
         for(Int x=boxi.min.x; x<=boxi.max.x; x++)
         {
            Memc<Int> &box_face=T.box_face[boxes_ab.index(VecI(x, y, z))];
            REPAD(t, box_face) // for each face in that box
            {
               Int testi=box_face[t]; if(testi!=i) // if the face isn't current
               {
                  Face &test=T.face[testi];
                  UInt  flag=(face.flag|test.flag); if(detect_self_intersections ? true : (flag&SOLID_A && flag&SOLID_B))
                  if(test.last_tested_with!=i) // if we haven't yet tested the pair
                  {
                     test.last_tested_with=i;

                     TriD test_tri(vtx[test.ind.x].posd, vtx[test.ind.y].posd, vtx[test.ind.z].posd);
                     // TODO: optimize
                  /*#if 0
                     Bool p0_equal=(face_tri.p[0]==test_tri.p[0] || face_tri.p[0]==test_tri.p[1] || face_tri.p[0]==test_tri.p[2]),
                          p1_equal=(face_tri.p[1]==test_tri.p[0] || face_tri.p[1]==test_tri.p[1] || face_tri.p[1]==test_tri.p[2]),
                          p2_equal=(face_tri.p[2]==test_tri.p[0] || face_tri.p[2]==test_tri.p[1] || face_tri.p[2]==test_tri.p[2]);
                  #else
                     Bool p0_equal=(Equal(face_tri.p[0], test_tri.p[0]) || Equal(face_tri.p[0], test_tri.p[1]) || Equal(face_tri.p[0], test_tri.p[2])),
                          p1_equal=(Equal(face_tri.p[1], test_tri.p[0]) || Equal(face_tri.p[1], test_tri.p[1]) || Equal(face_tri.p[1], test_tri.p[2])),
                          p2_equal=(Equal(face_tri.p[2], test_tri.p[0]) || Equal(face_tri.p[2], test_tri.p[1]) || Equal(face_tri.p[2], test_tri.p[2]));
                  #endif
                   //if(p0_equal+p1_equal+p2_equal<=1) // we can easily skip thise triangles which share 2,3 vertexes, no we can't because they can overlap*/
                     {
                        EdgeD edge;
                        Int   c=CutsTriPlaneEps(test_tri, face_plane, edge);
                        if(c==-1) // coplanar
                        {
                           overlapped=true;
                           REP(3) // all triangle edges
                           {
                                         edge.set(test_tri.p[i], test_tri.p[(i+1)%3]);
                              if(ClipEps(edge, face_tri_planes[0])) // Eps necessary
                              if(ClipEps(edge, face_tri_planes[1]))
                              if(ClipEps(edge, face_tri_planes[2]))
                              {
                                 face_edge.New().set(edge, face_matrix);
                              }
                           }
                        }else
                        if(c==2)
                        {
                           if(ClipEps(edge, face_tri_planes[0])) // Eps necessary
                           if(ClipEps(edge, face_tri_planes[1]))
                           if(ClipEps(edge, face_tri_planes[2]))
                           {
                              face_edge.New().set(edge, face_matrix);
                           }
                        }
                     }
                  }
               }
            }
         }

      #if DEBUG
         if(F<0 || i==F)
      #endif
         {
         #if DEBUG
            if(DrawEdges && !face_edge.elms())face_tri.draw(RED);
         #endif
            Memb<Face> &faces=(overlapped ? face_over : face_done);
            if(face_edge.elms())triFromEdges(face, face_tri, face_matrix, faces); //    intersection
            else                faces.New()=face                                ; // no intersection
         }
         face_edge.clear();
      }
   }
   Bool overlap(TriD &tri_a, TriD &tri_b)
   {
   /* we need to use more comples cases, because triangles are created from polygons
         simple algorithm is not enough:
      return
         (Equal(tri_a.p[0], tri_b.p[0]) || Equal(tri_a.p[0], tri_b.p[1]) || Equal(tri_a.p[0], tri_b.p[2]))
      && (Equal(tri_a.p[1], tri_b.p[0]) || Equal(tri_a.p[1], tri_b.p[1]) || Equal(tri_a.p[1], tri_b.p[2]))
      && (Equal(tri_a.p[2], tri_b.p[0]) || Equal(tri_a.p[2], tri_b.p[1]) || Equal(tri_a.p[2], tri_b.p[2]));
   */
   #if DEBUG
      if(Kb.b(KB_O))return false;
   #endif
      Bool  max =(tri_a.area()>tri_b.area());
      TriD &tri0=(max ? tri_a : tri_b), // bigger
           &tri1=(max ? tri_b : tri_a); // smaller

      if(tri1.coplanar(tri0))
      {
         VecD tri_n[3]=
         {
            CrossN(tri0.n, tri0.p[0]-tri0.p[1]),
            CrossN(tri0.n, tri0.p[1]-tri0.p[2]),
            CrossN(tri0.n, tri0.p[2]-tri0.p[0]),
         };
         EdgeD_I ei0[3]=
         {
            EdgeD_I(tri0.p[0], tri0.p[1]),
            EdgeD_I(tri0.p[1], tri0.p[2]),
            EdgeD_I(tri0.p[2], tri0.p[0]),
         };
         REPD(e1, 3)
         {
            EdgeD_I ei1(tri1.p[e1], tri1.p[(e1+1)%3]);
            EdgeD   cuts;
            Bool    touch=false;

            REPD(e0, 3)if(Int c=CutsEdgeEdge(ei1, ei0[e0], &cuts))
            {
               if(c==2)return DistPointPlane(tri1.p[(e1+2)%3], tri0.p[e0], tri_n[e0])<0;

               Int dup0;
               if(Equal(cuts.p[0], ei0[e0].p[0]))dup0=0;else
               if(Equal(cuts.p[0], ei0[e0].p[1]))dup0=1;else dup0=-1;

               Int dup1;
               if(Equal(cuts.p[0], ei1.p[0]))dup1=0;else
               if(Equal(cuts.p[0], ei1.p[1]))dup1=1;else dup1=-1;

               if(dup0<0)
               {
                  if(dup1<0)return true;
                  if(DistPointPlane(ei1.p[!dup1], tri0.p[e0], tri_n[e0])<0)return true;
               }else
               {
                  if(dup1!=0)if(DistPointPlane(ei1.p[0], tri0.p[(e0+dup0  )%3], tri_n[(e0+dup0  )%3])<-EPSD
                             && DistPointPlane(ei1.p[0], tri0.p[(e0+dup0+2)%3], tri_n[(e0+dup0+2)%3])<-EPSD)return true;
                  if(dup1!=1)if(DistPointPlane(ei1.p[1], tri0.p[(e0+dup0  )%3], tri_n[(e0+dup0  )%3])<-EPSD
                             && DistPointPlane(ei1.p[1], tri0.p[(e0+dup0+2)%3], tri_n[(e0+dup0+2)%3])<-EPSD)return true;
               }
               touch=true;
            }
            if(!touch && DistPointPlane(ei1.p[0], tri0.p[0], tri_n[0])<0
                      && DistPointPlane(ei1.p[0], tri0.p[1], tri_n[1])<0
                      && DistPointPlane(ei1.p[0], tri0.p[2], tri_n[2])<0)return true;
         }
      }
      return false;
   }
   // TODO: analyze if 'removeOverlapped' works ok (order, enabling B<->A, removing)
   void removeOverlapped()
   {
      // set boxes
      REPA(face_over)setFaceBox(face_over[i]);

      // detect overlaps
      REPAD(i, face_over)
      {
         Face &face_i=face_over[i]; TriD tri_i(vtx[face_i.ind.x].posd, vtx[face_i.ind.y].posd, vtx[face_i.ind.z].posd);
         Bool  face_i_has_a=((face_i.flag&SOLID_A)!=0);
         REPD(j, i)
         {
            Face &face_j=face_over[j];
            Bool  face_j_has_a=((face_j.flag&SOLID_A)!=0);

            if(face_i_has_a!=face_j_has_a && Cuts(face_i.boxi, face_j.boxi))
            {
               TriD tri_j(vtx[face_j.ind.x].posd, vtx[face_j.ind.y].posd, vtx[face_j.ind.z].posd);
               if(overlap(tri_i, tri_j))
               {
/*#if DEBUG
   tri_i.draw(RED  , true);
   tri_j.draw(GREEN, true);
#endif*/
                  face_j.flag|=((Dot(tri_i.n, tri_j.n)>0) ? face_i.flag : SolidSwap(face_i.flag)); // if equal in reversed order the swap the solid flag
                  face_j.boxi|=face_i.boxi;
                  goto overlapped;
               }
            }
         }
         face_done.New()=face_i;
      overlapped:;
      }
   }
   void setCutFacesSolid(C MeshBase &a, C MeshBase &b)
   {
      // set solid for 'cut' faces
      REPA(face_done)
      {
         Face &face=face_done[i];
         UInt &flag=face.flag;

         if(flag&SOLID_CUT)
         {
            flag^=SOLID_CUT;
            if(!(flag&SOLID_A) || !(flag&SOLID_B))
            {
               VecI &ind=face.ind;
               Vec   p  =Avg(vtx[ind.x].pos, vtx[ind.y].pos, vtx[ind.z].pos);

               if(!(flag&SOLID_A)) // if there's no info about SOLID_A
               {
                  if(CutsPointMesh(p, a, null, &cuts_cache_a))flag|=SOLID_AR |SOLID_AL ;
                  else                                        flag|=SOLID_NAR|SOLID_NAL;
               #if DEBUG
                  if(DrawInside)TriD(vtx[ind.x].posd, vtx[ind.y].posd, vtx[ind.z].posd).draw((flag&SOLID_AR) ? ORANGE : PURPLE, true);
               #endif
               }
               if(!(flag&SOLID_B)) // if there's no info about SOLID_B
               {
                  if(CutsPointMesh(p, b, null, &cuts_cache_b))flag|=SOLID_BR |SOLID_BL ;
                  else                                        flag|=SOLID_NBR|SOLID_NBL;
               #if DEBUG
                  if(DrawInside)TriD(vtx[ind.x].posd, vtx[ind.y].posd, vtx[ind.z].posd).draw((flag&SOLID_BR) ? ORANGE : PURPLE, true);
               #endif
               }
            }
         }
      }
   }
   void build(MeshBase &mshb, UInt flag)
   {
      mshb.create(face_done.elms()*3, 0, face_done.elms(), 0, flag);
      FREPA(face_done)
      {
         Face &face=face_done[i];
         Int   p0  =i*3+0,
               p1  =i*3+1,
               p2  =i*3+2;
         vtx[face.ind.x].to(mshb, p0);
         vtx[face.ind.y].to(mshb, p1);
         vtx[face.ind.z].to(mshb, p2);
                          mshb.tri.ind (i).set(p0,p1,p2);
                          mshb.tri.flag(i)=face.flag;
         if(mshb.tri.id())mshb.tri.id  (i)=face.id  ;
      }
   }
   void distributeSolid(MeshBase &mshb, Int tri, UInt flag, UInt solid) // distribute the solid info for neighbor faces, 'solid' here is equal to SOLID_A or SOLID_B
   {
      Memb<Int> tris; tris.New()=tri; // list of triangles for checking
      for(; tris.elms(); )
      {
         Int tri=tris.pop(); // get the triangle and remove it from the list
         if(!(mshb.tri.flag(tri)&solid)) // if there's no info about solid
         {
                          mshb.tri.flag   (tri)|=flag; // add info
            VecI adj_face=mshb.tri.adjFace(tri)      ; // list of neighbors
         #if DEBUG
            if(!Kb.b(KB_D))
         #endif
            REPA(adj_face) // for each neighbor
            {
               Int adj =adj_face.c[i];
               if( adj!=-1)tris.New()=adj; // if it exists then add it to the list, compare to -1 and not >=0 because it can have SIGN_BIT
            }
         }
      }
   }
   void setSolid(MeshBase &mshb, C MeshBase &a, C MeshBase &b)
   {
      mshb.setVtxDup().setAdjacencies(true, false);

      // distribute info about solid which we already have
   #if DEBUG
      if(!Kb.b(KB_D))
   #endif
      REPD(s, 2) // distribute 2 solids
      {
         UInt solid=(s ? SOLID_B : SOLID_A); // SOLID_A and SOLID_B
         REPA(mshb.tri) // for each triangle
         {
            if(UInt f=(mshb.tri.flag(i)&solid)) // if it has info about solid
            {
               VecI af=mshb.tri.adjFace(i); // list of neighbors
               REPA(af) // for each neighbor
               {
                  Int adj =af.c[i];
                  if( adj!=-1 && !(mshb.tri.flag(adj)&solid))distributeSolid(mshb, adj, f, solid); // if the neighbor exists and it doesn't have the info about solid which we can provide, compare to -1 and not >=0 because it can have SIGN_BIT
               }
            }
         }
      }

      // test missing solid's
      REPD(s, 2)
      {
         UInt solid=(s ? SOLID_B : SOLID_A); // SOLID_A and SOLID_B
         REPA(mshb.tri) // for each triangle
         {
            if(!(mshb.tri.flag(i)&solid)) // if doesn't have info about solid
            {
               Int *p     =    mshb.tri.ind(i).c;
               Vec  pos   =Avg(mshb.vtx.pos(p[0]), mshb.vtx.pos(p[1]), mshb.vtx.pos(p[2]));
               Bool inside=CutsPointMesh(pos, s ? b : a, null, s ? &cuts_cache_b : &cuts_cache_a); // test solid info
               UInt flag  =(inside ? (s?(SOLID_BL|SOLID_BR):(SOLID_AL|SOLID_AR)) : (s?(SOLID_NBL|SOLID_NBR):(SOLID_NAL|SOLID_NAR)));
               distributeSolid(mshb, i, flag, solid); // distribute solid info
            }
         }
      }
   }
   Bool faceIs(UInt sel, UInt f, Bool &reverse)
   {
      reverse=false;
      switch(sel)
      {
         case SEL_A: // a sub b
         {
            if(f&SOLID_NBL && f&SOLID_NBR) // outside B
            {
               if(f&SOLID_AR && f&SOLID_NAL)return         true;
               if(f&SOLID_AL && f&SOLID_NAR)return reverse=true;
            }else
            if(f&SOLID_AR && f&SOLID_AL) // inside A
            {
               if(f&SOLID_BR && f&SOLID_NBL)return reverse=true;
               if(f&SOLID_BL && f&SOLID_NBR)return         true;
            }
         }break;

         case SEL_B: // b sub a
         {
            if(f&SOLID_NAL && f&SOLID_NAR) // outside A
            {
               if(f&SOLID_BR && f&SOLID_NBL)return         true;
               if(f&SOLID_BL && f&SOLID_NBR)return reverse=true;
            }else
            if(f&SOLID_BR && f&SOLID_BL) // inside B
            {
               if(f&SOLID_AR && f&SOLID_NAL)return reverse=true;
               if(f&SOLID_AL && f&SOLID_NAR)return         true;
            }
         }break;

         case SEL_A|SEL_B       : // xor
         case SEL_A|SEL_B|SEL_AB: // add,or
         {
            if(f&SOLID_NBL && f&SOLID_NBR) // outside B
            {
               if(f&SOLID_AR && f&SOLID_NAL)return         true;
               if(f&SOLID_AL && f&SOLID_NAR)return reverse=true;
            }else
            if(f&SOLID_NAL && f&SOLID_NAR) // outside A
            {
               if(f&SOLID_BR && f&SOLID_NBL)return         true;
               if(f&SOLID_BL && f&SOLID_NBR)return reverse=true;
            }else // covers
            {
               if(f&SOLID_AR && f&SOLID_NAL && f&SOLID_BR && f&SOLID_NBL)return         true;
               if(f&SOLID_AL && f&SOLID_NAR && f&SOLID_BL && f&SOLID_NBR)return reverse=true;
            }
         }break;

         case SEL_AB: // and,mul
         {
            if(f&SOLID_BL && f&SOLID_BR) // inside B
            {
               if(f&SOLID_AR && f&SOLID_NAL)return         true;
               if(f&SOLID_AL && f&SOLID_NAR)return reverse=true;
            }else
            if(f&SOLID_AL && f&SOLID_AR) // inside A
            {
               if(f&SOLID_BR && f&SOLID_NBL)return         true;
               if(f&SOLID_BL && f&SOLID_NBR)return reverse=true;
            }else // covers
            {
               if(f&SOLID_AR && f&SOLID_NAL && f&SOLID_BR && f&SOLID_NBL)return         true;
               if(f&SOLID_AL && f&SOLID_NAR && f&SOLID_BL && f&SOLID_NBR)return reverse=true;
            }
         }break;
      }
      return false;
   }
   void removeAndFlipFaces(MeshBase &mshb, UInt sel)
   {
      Memt<Bool> is; is.setNum(mshb.tris());
      REPA(mshb.tri)
      {
         Bool reverse;
         if(is[i]=faceIs(sel, mshb.tri.flag(i), reverse))if(reverse)
         {
            VecI ind=mshb.tri.ind(i).reverse();
            if(mshb.vtx.nrm())
            {
               mshb.vtx.nrm(ind.x).chs();
               mshb.vtx.nrm(ind.y).chs();
               mshb.vtx.nrm(ind.z).chs();
            }
            if(mshb.vtx.tan() && !mshb.vtx.bin())
            {
               mshb.vtx.tan(ind.x).chs();
               mshb.vtx.tan(ind.y).chs();
               mshb.vtx.tan(ind.z).chs();
               if(mshb.vtx.tex0())
               {
                  CHS(mshb.vtx.tex0(ind.x).x);
                  CHS(mshb.vtx.tex0(ind.y).x);
                  CHS(mshb.vtx.tex0(ind.z).x);
               }
               if(mshb.vtx.tex1())
               {
                  CHS(mshb.vtx.tex1(ind.x).x);
                  CHS(mshb.vtx.tex1(ind.y).x);
                  CHS(mshb.vtx.tex1(ind.z).x);
               }
               if(mshb.vtx.tex2())
               {
                  CHS(mshb.vtx.tex2(ind.x).x);
                  CHS(mshb.vtx.tex2(ind.y).x);
                  CHS(mshb.vtx.tex2(ind.z).x);
               }
            }
         }
      }
      mshb.keepTris(is);
   }
   void optimize(MeshBase &mshb, Flt weld_pos_eps)
   {
      mshb.removeUnusedVtxs(); mshb.weldVtxValues(VTX_POS, weld_pos_eps, EPS_COL_COS, -1).weldVtx(VTX_ALL, 0);
   }

   CSG(C MeshBase &a, C MeshBase &b, UInt sel, MeshBase &dest, Bool detect_self_intersections, Flt weld_pos_eps) : cuts_cache_a(a), cuts_cache_b(b)
   {
   #if DEBUG
      DrawEdges ^=Kb.bp(KB_E);
      DrawPolys ^=Kb.bp(KB_P);
      DrawTris  ^=Kb.bp(KB_T);
      DrawVtxs  ^=Kb.bp(KB_V);
      DrawInside^=Kb.bp(KB_I);
   #endif
      // load data
      box_ab=(cuts_cache_a.box&cuts_cache_b.box);
      addMesh(a, SOLID_AR|SOLID_NAL);
      addMesh(b, SOLID_BR|SOLID_NBL);
      boxes_ab.set(box_ab, face.elms());
      New(box_face, boxes_ab.num());

      // process data
      assignFacesToBoxes();
      cutFaces          (detect_self_intersections);
      removeOverlapped  ();
      setCutFacesSolid  (a, b);

      // build data
      Bool               id=(((a.flag()|b.flag())&FACE_ID)!=0);
      MeshBase           temp;
      build             (temp, ((a.flag()|b.flag())&VTX_ALL) | TRI_FLAG | (id ? FACE_ID : 0));
      setSolid          (temp, a, b);
   #if DEBUG
      if(!Kb.b(KB_R))
   #endif
      removeAndFlipFaces(temp, sel);
      optimize          (temp, weld_pos_eps);
      Swap              (dest, temp);
   }
  ~CSG()
   {
      DeleteN(box_face);
   }
};
/******************************************************************************/
struct Material4
{
   MaterialPtr m[4];
   
   Material4(                ) {}
   Material4(C MeshPart &part)
   {
      m[0]=part.multiMaterial(0);
      m[1]=part.multiMaterial(1);
      m[2]=part.multiMaterial(2);
      m[3]=part.multiMaterial(3);
   }
};
void Csg(MeshBase &a, C MeshBase &b, UInt sel, MeshBase *dest, Bool detect_self_intersections, Flt weld_pos_eps)
{
   CSG csg(a, b, sel, dest ? *dest : a, detect_self_intersections, weld_pos_eps);
}
void Csg(MeshLod &a, C MeshLod &b, UInt sel, MeshLod *dest, Bool detect_self_intersections, Flt weld_pos_eps)
{
   Memc<Material4> materials;
   FREPA(a)materials.add(a.parts[i]);
   FREPA(b)materials.add(b.parts[i]);

   MeshBase a_base; a_base.create(a, ~0, true);
   MeshBase b_base; b_base.create(b, ~0, true);

   REPA(b_base.edge)b_base.edge.id(i)+=a.parts.elms();
   REPA(b_base.tri )b_base.tri .id(i)+=a.parts.elms();
   REPA(b_base.quad)b_base.quad.id(i)+=a.parts.elms();

   Csg(a_base, b_base, sel, null, detect_self_intersections, weld_pos_eps);
   
   if(!dest)dest=&a;
   a_base.copyId(*dest);

   FREPA(*dest)dest->parts[i].multiMaterial(materials[i].m[0], materials[i].m[1], materials[i].m[2], materials[i].m[3]);
}
/******************************************************************************/
// CLIP MESH
/******************************************************************************/
static void ClipMeshSimple(C MeshBase &src, C Matrix *matrix, MeshBase &dest, C Plane *clip_plane, Int clip_planes, Flt weld_pos_eps) // processes only vertex positions and face (triangle and quad) indexes
{
   MeshBase temp;

   // box test
   Vec  corner[8]; OBox(Box(src), matrix ? *matrix : MatrixIdentity).toCorners(corner);
   Bool all_inside=true;
   REPD(p, clip_planes)
   {
      Bool  inside=false,
           outside=false;
      REPAD(c, corner)
      {
         if(Dist(corner[c], clip_plane[p])<=0)inside=true;else outside=true;
      }
      if(!inside)goto finished; // if no vertexes are inside the plane then cancel
      if(outside)all_inside=false;
   }

   // create dest
   if(all_inside)
   {
      temp.create(src, VTX_POS|FACE_ALL);
      if(matrix)temp.transform(*matrix);
   }else
   {
            Bool        set_flags=(src.tri.flag() || src.quad.flag());
      Memc< Byte      > poly_flags;
      Memc< Memc<Vec> > polys;
            Memc<Vec>   poly[2];
            Bool        poly_cur=0;

      // triangles
      REPA(src.tri)
      {
         VecI ind=src.tri.ind(i);
         poly[poly_cur].add(src.vtx.pos(ind.x));
         poly[poly_cur].add(src.vtx.pos(ind.y));
         poly[poly_cur].add(src.vtx.pos(ind.z));
         if(matrix)REPAO(poly[poly_cur])*=*matrix;
         REP(clip_planes)
         {
            ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
            poly_cur^=1; if(!poly[poly_cur].elms())goto no_tri;
         }
         Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.tri.flag() ? src.tri.flag(i) : 0);
      no_tri:;
         poly[poly_cur^1].clear();
      }

      // quads
      REPA(src.quad)
      {
         VecI4 ind=src.quad.ind(i);
         poly[poly_cur].add(src.vtx.pos(ind.x));
         poly[poly_cur].add(src.vtx.pos(ind.y));
         poly[poly_cur].add(src.vtx.pos(ind.z));
         poly[poly_cur].add(src.vtx.pos(ind.w));
         if(matrix)REPAO(poly[poly_cur])*=*matrix;
         REP(clip_planes)
         {
            ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
            poly_cur^=1; if(!poly[poly_cur].elms())goto no_quad;
         }
         Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.quad.flag() ? src.quad.flag(i) : 0);
      no_quad:;
         poly[poly_cur^1].clear();
      }

      // polys to dest
      Triangulate(polys, temp, weld_pos_eps, true, poly_flags.data());
   }

finished:
   Swap(dest, temp);
}
/******************************************************************************/
void ClipMesh(C MeshBase &src, C Matrix *matrix, MeshBase &dest, C Plane *clip_plane, Int clip_planes, UInt flag_and, Flt weld_pos_eps)
{
   flag_and&=src.flag();
   if(!(flag_and&VTX_ALL&~(VTX_POS|VTX_FLAG|VTX_DUP))) // if simple is enough
   {
      ClipMeshSimple(src, matrix, dest, clip_plane, clip_planes, weld_pos_eps);
   }else
   {
      MeshBase temp;

      // box test
      Vec  corner[8]; OBox(Box(src), matrix ? *matrix : MatrixIdentity).toCorners(corner);
      Bool all_inside=true;
      REPD(p, clip_planes)
      {
         Bool  inside=false,
              outside=false;
         REPAD(c, corner)
         {
            if(Dist(corner[c], clip_plane[p])<=0)inside=true;else outside=true;
         }
         if(!inside)goto finished; // if no vertexes are inside the plane then cancel
         if(outside)all_inside=false;
      }

      // create dest
      if(all_inside)
      {
         temp.create(src, flag_and);
         if(matrix)temp.transform(*matrix);
      }else
      {
         Matrix3 matrix3; if(matrix){matrix3=*matrix; matrix3.normalize();}

               Bool             set_flags=FlagTest(flag_and, FACE_FLAG);
         Memc< Byte          > poly_flags;
         Memc< Memc<VtxFull> > polys;
               Memc<VtxFull>   poly[2];
               Bool            poly_cur=0;

         // triangles
         REPA(src.tri)
         {
            VecI ind=src.tri.ind(i);
            poly[poly_cur].New().from(src, ind.x);
            poly[poly_cur].New().from(src, ind.y);
            poly[poly_cur].New().from(src, ind.z);
            if(matrix)REPAO(poly[poly_cur]).mul(*matrix, matrix3);
            REP(clip_planes)
            {
               ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
               poly_cur^=1; if(!poly[poly_cur].elms())goto no_tri;
            }
            Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.tri.flag() ? src.tri.flag(i) : 0);
         no_tri:;
            poly[poly_cur^1].clear();
         }

         // quads
         REPA(src.quad)
         {
            VecI4 ind=src.quad.ind(i);
            poly[poly_cur].New().from(src, ind.x);
            poly[poly_cur].New().from(src, ind.y);
            poly[poly_cur].New().from(src, ind.z);
            poly[poly_cur].New().from(src, ind.w);
            if(matrix)REPAO(poly[poly_cur]).mul(*matrix, matrix3);
            REP(clip_planes)
            {
               ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
               poly_cur^=1; if(!poly[poly_cur].elms())goto no_quad;
            }
            Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.quad.flag() ? src.quad.flag(i) : 0);
         no_quad:;
            poly[poly_cur^1].clear();
         }

         // polys to dest
         Triangulate(polys, temp, flag_and, weld_pos_eps, true, poly_flags.data());
      }

   finished:
      Swap(dest, temp);
   }
}
/******************************************************************************/
void ClipMesh(C Mesh &src, C Matrix *matrix, Mesh &dest, C Plane *clip_plane, Int clip_planes, UInt flag_and, Flt weld_pos_eps)
{
   Mesh temp;

   // box test
   Vec  corner[8]; src.ext.toCorners(corner); if(matrix)Transform(corner, *matrix, Elms(corner));
   Bool all_inside=true;
   REPD(p, clip_planes)
   {
      Bool  inside=false,
           outside=false;
      REPAD(c, corner)
      {
         if(Dist(corner[c], clip_plane[p])<=0)inside=true;else outside=true;
      }
      if(!inside)goto finished; // if no vertexes are inside the plane then cancel
      if(outside)all_inside=false;
   }

   // create dest
   if(all_inside)
   {
      temp.create(src, flag_and);
      if(matrix)
      {
         temp.transform(*matrix).setBox();
         temp.lod_center=src.lod_center*(*matrix); // set manual lod center after setting box
      }
   }else
   {
      Flt     scale=(matrix ? matrix->maxScale() : 1);
      Matrix3 matrix3; if(matrix){matrix3=*matrix; matrix3.normalize();}

      Memc< Byte          > poly_flags;
      Memc< Memc<VtxFull> > polys;
            Memc<VtxFull>   poly[2];
            Bool            poly_cur=0;

      temp.setLods(src.lods()); temp.copyParams(src);
      REPD(l, src.lods()) // order is important
      {
       C MeshLod & src_lod=src .lod(l);
         MeshLod &dest_lod=temp.lod(l);
         FREPA(src_lod)
         {
          C MeshPart &src_part = src_lod .parts[i]; MeshBase temp; if(!src_part.base.is() && src_part.render.is())temp.create(src_part.render);
          C MeshBase &src      =(src_part.base.is() ? src_part.base : temp);
            Bool      set_flags=FlagTest(src.flag()&flag_and, FACE_FLAG);

            // triangles
            REPA(src.tri)
            {
               VecI ind=src.tri.ind(i);
               poly[poly_cur].New().from(src, ind.x);
               poly[poly_cur].New().from(src, ind.y);
               poly[poly_cur].New().from(src, ind.z);
               if(matrix)REPAO(poly[poly_cur]).mul(*matrix, matrix3);
               REP(clip_planes)
               {
                  ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
                  poly_cur^=1; if(!poly[poly_cur].elms())goto no_tri;
               }
               Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.tri.flag() ? src.tri.flag(i) : 0);
            no_tri:;
               poly[poly_cur^1].clear();
            }

            // quads
            REPA(src.quad)
            {
               VecI4 ind=src.quad.ind(i);
               poly[poly_cur].New().from(src, ind.x);
               poly[poly_cur].New().from(src, ind.y);
               poly[poly_cur].New().from(src, ind.z);
               poly[poly_cur].New().from(src, ind.w);
               if(matrix)REPAO(poly[poly_cur]).mul(*matrix, matrix3);
               REP(clip_planes)
               {
                  ClipPoly(poly[poly_cur], clip_plane[i], poly[poly_cur^1]);
                  poly_cur^=1; if(!poly[poly_cur].elms())goto no_quad;
               }
               Swap(polys.New(), poly[poly_cur]); if(set_flags)poly_flags.add(src.quad.flag() ? src.quad.flag(i) : 0);
            no_quad:;
               poly[poly_cur^1].clear();
            }

            // polys to dest
            if(polys.elms())
            {
               MeshPart &dest_part=dest_lod.parts.New(); dest_part.copyParams(src_part); dest_part.scaleParams(scale);
               Triangulate(polys, dest_part.base, src.flag()&flag_and, weld_pos_eps, true, poly_flags.data());
               polys.clear(); poly_flags.clear();
            }
         }
         if(!dest_lod.parts.elms())temp.removeLod(l);else
         {
            dest_lod. copyParams(src_lod);
            dest_lod.scaleParams(scale);
         }
      }
      temp.setBox();
      temp.lod_center=src.lod_center; if(matrix)temp.lod_center*=*matrix; // set manual lod center after setting box
   }

finished:
   Swap(dest, temp);
}
/******************************************************************************/
void SplitMesh(C Mesh &src, C Matrix *matrix, Mesh &dest_positive, Mesh &dest_negative, C Plane &clip_plane, UInt flag_and, Flt weld_pos_eps)
{
   Bool same=(&dest_positive==&dest_negative);
   Mesh temp_positive, temp_negative2, &temp_negative=(same ? temp_positive : temp_negative2); // if we're splitting to the same mesh, then just store everything in 'temp_positive'

   // box test
   Vec  corner[8]; src.ext.toCorners(corner); if(matrix)Transform(corner, *matrix, Elms(corner));
   Bool inside=false,
       outside=false;
   REPAD(c, corner)
   {
      if(Dist(corner[c], clip_plane)<=0)inside=true;else outside=true;
   }

   if(!inside) // if no vertexes are inside the plane
   {
      temp_positive.create(src, flag_and);
      if(matrix)
      {
         temp_positive.transform(*matrix).setBox();
         temp_positive.lod_center=src.lod_center*(*matrix); // set manual lod center after setting box
      }
   }else
   if(!outside) // if no vertexes are outside the plane
   {
      temp_negative.create(src, flag_and);
      if(matrix)
      {
         temp_negative.transform(*matrix).setBox();
         temp_negative.lod_center=src.lod_center*(*matrix); // set manual lod center after setting box
      }
   }
   else // vertexes are from both sides so we need to cut
   {
      Flt     scale=(matrix ? matrix->maxScale() : 1);
      Matrix3 matrix3; if(matrix){matrix3=*matrix; matrix3.normalize();}

      Memc< Memc<VtxFull> > polys_pos, polys_neg;
            Memc<VtxFull>   poly_src, poly_pos, poly_neg;

                temp_positive.setLods(src.lods()); temp_positive.copyParams(src);
      if(!same){temp_negative.setLods(src.lods()); temp_negative.copyParams(src);}
      REPD(l, src.lods()) // order is important
      {
       C MeshLod & src_lod  =src          .lod(l);
         MeshLod &dest_lod_p=temp_positive.lod(l),
                 &dest_lod_n=temp_negative.lod(l);
         FREPA(src_lod)
         {
          C MeshPart &src_part= src_lod .parts[i]; MeshBase temp; if(!src_part.base.is() && src_part.render.is())temp.create(src_part.render);
          C MeshBase &src     =(src_part.base.is() ? src_part.base : temp);

            // triangles
            REPA(src.tri)
            {
               VecI ind=src.tri.ind(i);
               poly_src.New().from(src, ind.x);
               poly_src.New().from(src, ind.y);
               poly_src.New().from(src, ind.z);
               if(matrix)REPAO(poly_src).mul(*matrix, matrix3);
               SplitPoly(poly_src, clip_plane, poly_pos, poly_neg);
               if(poly_pos.elms())Swap(polys_pos.New(), poly_pos);
               if(poly_neg.elms())Swap(polys_neg.New(), poly_neg);
                  poly_src.clear();
            }

            // quads
            REPA(src.quad)
            {
               VecI4 ind=src.quad.ind(i);
               poly_src.New().from(src, ind.x);
               poly_src.New().from(src, ind.y);
               poly_src.New().from(src, ind.z);
               poly_src.New().from(src, ind.w);
               if(matrix)REPAO(poly_src).mul(*matrix, matrix3);
               SplitPoly(poly_src, clip_plane, poly_pos, poly_neg);
               if(poly_pos.elms())Swap(polys_pos.New(), poly_pos);
               if(poly_neg.elms())Swap(polys_neg.New(), poly_neg);
                  poly_src.clear();
            }

            // polys to dest
            if(polys_pos.elms())
            {
               MeshPart &dest_part=dest_lod_p.parts.New(); dest_part.copyParams(src_part); dest_part.scaleParams(scale);
               Triangulate(polys_pos, dest_part.base, src.flag()&flag_and, weld_pos_eps, true);
               polys_pos.clear();
            }
            if(polys_neg.elms())
            {
               MeshPart &dest_part=dest_lod_n.parts.New(); dest_part.copyParams(src_part); dest_part.scaleParams(scale);
               Triangulate(polys_neg, dest_part.base, src.flag()&flag_and, weld_pos_eps, true);
               polys_neg.clear();
            }
         }

         // copy lod settings (or remove them if no parts exist)
         if(!dest_lod_p.parts.elms())temp_positive.removeLod(l);else
         {
            dest_lod_p. copyParams(src_lod);
            dest_lod_p.scaleParams(scale);
         }
         if(!same && !dest_lod_n.parts.elms())temp_negative.removeLod(l);else
         {
            dest_lod_n. copyParams(src_lod);
            dest_lod_n.scaleParams(scale);
         }
      }
               temp_positive.setBox();
      if(!same)temp_negative.setBox();

      // set manual lod center after setting box
      temp_positive.lod_center=
      temp_negative.lod_center=src.lod_center;
      if(matrix)
      {
         temp_positive.lod_center*=*matrix;
         temp_negative.lod_center*=*matrix;
      }
   }

            Swap(dest_positive, temp_positive);
   if(!same)Swap(dest_negative, temp_negative);
}
/******************************************************************************/
struct MiddlePoint
{
   Flt prev_dist, next_dist;
   Vec pos;

   void set(Flt prev_dist, Flt next_dist, C Vec &pos) {T.prev_dist=prev_dist; T.next_dist=next_dist; T.pos=pos;}
};
static void SplitPoly(C Memc<VtxFull> &poly, C Plane &plane, Memc<VtxFull> &output_positive, Memc<VtxFull> &output_negative, Memc<Edge> &edges)
{
   output_positive.clear();
   output_negative.clear();

   switch(poly.elms())
   {
      case 0: break;

      case 1:
         if(Dist(poly[0].pos, plane)<0)output_negative.add(poly[0]);
         else                          output_positive.add(poly[0]);
      break;

      default: // >=2
      {
         Memc<MiddlePoint> middles;

       C VtxFull *prev     =&poly.last();
         Flt      prev_dist=Dist(prev->pos, plane);
         FREPA(poly) // preserve order
         {
            Bool     have_mid =false;
          C VtxFull &next     =poly[i]; VtxFull mid;
            Flt      next_dist=Dist(next.pos, plane);

            // negative
            if(prev_dist<0)
            {
               output_negative.add(*prev);
               if(next_dist>=0) // change of side
               {
                  if(!have_mid){have_mid=true; mid.lerp(*prev, next, prev_dist/(prev_dist-next_dist));} // -prev_dist/(next_dist-prev_dist)
                  output_negative.add(mid);
               }
            }
            else // prev_dist>=0
            {
               if(next_dist<0) // change of side
               {
                  if(!have_mid){have_mid=true; mid.lerp(*prev, next, prev_dist/(prev_dist-next_dist));}
                  output_negative.add(mid);
               }
            }

            // positive
            if(prev_dist>0)
            {
               output_positive.add(*prev);
               if(next_dist<=0) // change of side
               {
                  if(!have_mid){have_mid=true; mid.lerp(*prev, next, prev_dist/(prev_dist-next_dist));} // -prev_dist/(next_dist-prev_dist)
                  output_positive.add(mid);
               }
            }
            else // prev_dist<=0
            {
               if(next_dist>0) // change of side
               {
                  if(!have_mid){have_mid=true; mid.lerp(*prev, next, prev_dist/(prev_dist-next_dist));}
                  output_positive.add(mid);
               }
            }

            if(have_mid)middles.New().set(prev_dist, next_dist, mid.pos);

            prev     =&next;
            prev_dist= next_dist;
         }

         // rotate the middle vertexes until they're sorted
         Vec nrm  =0; REPA(poly)nrm+=GetNormalEdge(poly[i].pos, poly[(i+1)%poly.elms()].pos); // get average face normal
         Vec cross=Cross(plane.normal, nrm);
         for(; middles.elms()>=2; )// && Dot(middles.last().pos, cross)>Dot(middles.first().pos, cross); )
         {
            Flt a=Dot(middles.first().pos, cross),
                b=Dot(middles.last ().pos, cross); if(a>=b)break; // must be stored in Flt variables insted of comparing values directly due to imprecisions of FPU
            middles.rotateOrder(1);
         }

         // create clip edges
         FREPA(middles)if(InRange(i+1, middles))
         {
            MiddlePoint &m =middles[i  ],
                        &m1=middles[i+1];
            
            if(m .prev_dist<=0 && m .next_dist>=0
            && m1.prev_dist>=0 && m1.next_dist<=0)edges.New().set(m.pos, m1.pos);
         }
      }break;
   }
}
/******************************************************************************/
void SplitMeshSolid(C Mesh &src, C Matrix *matrix, Mesh &dest_positive, Mesh &dest_negative, C Plane &clip_plane, C MaterialPtr &material, Flt tex_scale, UInt flag_and, Flt weld_pos_eps)
{
   Mesh temp_positive, temp_negative;

   // box test
   Vec  corner[8]; src.ext.toCorners(corner); if(matrix)Transform(corner, *matrix, Elms(corner));
   Bool inside=false,
       outside=false;
   REPAD(c, corner)
   {
      if(Dist(corner[c], clip_plane)<=0)inside=true;else outside=true;
   }

   if(!inside) // if no vertexes are inside the plane
   {
      temp_positive.create(src, flag_and);
      if(matrix)
      {
         temp_positive.transform(*matrix).setBox();
         temp_positive.lod_center=src.lod_center*(*matrix); // set manual lod center after setting box
      }
   }else
   if(!outside) // if no vertexes are outside the plane
   {
      temp_negative.create(src, flag_and);
      if(matrix)
      {
         temp_negative.transform(*matrix).setBox();
         temp_negative.lod_center=src.lod_center*(*matrix); // set manual lod center after setting box
      }
   }else // vertexes are from both sides so we need to cut
   {
      Flt     scale=(matrix ? matrix->maxScale() : 1);
      Matrix3 matrix3; if(matrix){matrix3=*matrix; matrix3.normalize();}

      Memc< Memc<VtxFull> > polys_pos, polys_neg;
            Memc<VtxFull>   poly_src, poly_pos, poly_neg;

      temp_positive.setLods(src.lods()); temp_positive.copyParams(src);
      temp_negative.setLods(src.lods()); temp_negative.copyParams(src);
      REPD(l, src.lods()) // order is important
      {
         Memc<Edge> edges;
       C MeshLod & src_lod  =src          .lod(l);
         MeshLod &dest_lod_p=temp_positive.lod(l),
                 &dest_lod_n=temp_negative.lod(l);

         FREPA(src_lod)
         {
          C MeshPart  &src_part= src_lod .parts[i]; MeshBase temp; if(!src_part.base.is() && src_part.render.is())temp.create(src_part.render);
          C MeshBase  &src     =(src_part.base.is() ? src_part.base : temp);

            // triangles
            REPA(src.tri)
            {
               VecI ind=src.tri.ind(i);
               poly_src.New().from(src, ind.x);
               poly_src.New().from(src, ind.y);
               poly_src.New().from(src, ind.z);
               if(matrix)REPAO(poly_src).mul(*matrix, matrix3);
               SplitPoly(poly_src, clip_plane, poly_pos, poly_neg, edges);
               if(poly_pos.elms())Swap(polys_pos.New(), poly_pos);
               if(poly_neg.elms())Swap(polys_neg.New(), poly_neg);
                  poly_src.clear();
            }

            // quads
            REPA(src.quad)
            {
               VecI4 ind=src.quad.ind(i);
               poly_src.New().from(src, ind.x);
               poly_src.New().from(src, ind.y);
               poly_src.New().from(src, ind.z);
               poly_src.New().from(src, ind.w);
               if(matrix)REPAO(poly_src).mul(*matrix, matrix3);
               SplitPoly(poly_src, clip_plane, poly_pos, poly_neg, edges);
               if(poly_pos.elms())Swap(polys_pos.New(), poly_pos);
               if(poly_neg.elms())Swap(polys_neg.New(), poly_neg);
                  poly_src.clear();
            }

            // polys to dest
            if(polys_pos.elms())
            {
               MeshPart &dest_part=dest_lod_p.parts.New(); dest_part.copyParams(src_part); dest_part.scaleParams(scale);
               Triangulate(polys_pos, dest_part.base, src.flag()&flag_and, weld_pos_eps, true);
               polys_pos.clear();
            }
            if(polys_neg.elms())
            {
               MeshPart &dest_part=dest_lod_n.parts.New(); dest_part.copyParams(src_part); dest_part.scaleParams(scale);
               Triangulate(polys_neg, dest_part.base, src.flag()&flag_and, weld_pos_eps, true);
               polys_neg.clear();
            }
         }

         // create insides
         if(edges.elms())
         {
            MeshBase temp(edges.elms()*2, edges.elms(), 0, 0, EDGE_FLAG);
            REPA(edges)
            {
               temp.vtx .pos (i*2+0)=edges[i].p[0];
               temp.vtx .pos (i*2+1)=edges[i].p[1];
               temp.edge.ind (i    ).set(i*2+0, i*2+1);
               temp.edge.flag(i    )=ETQ_R;
            }
            temp.weldVtx().removeDoubleEdges();
            MeshBase temp2D(temp); temp2D.transform(~Matrix3().setDir(clip_plane.normal)).edgeToTri(); Swap(temp2D.tri, temp.tri);
            if(temp.faces())
            {
               temp.texMap(clip_plane).texScale(Vec2(tex_scale)).include(VTX_NRM); REPA(temp.vtx)temp.vtx.nrm(i)=-clip_plane.normal;
               MeshPart &solid_p=dest_lod_p.parts.New(); solid_p.base=temp          ; solid_p.material(material, l);
               MeshPart &solid_n=dest_lod_n.parts.New(); solid_n.base=temp.reverse(); solid_n.material(material, l);
            }
         }

         // copy lod settings (or remove them if no parts exist)
         if(!dest_lod_p.parts.elms())temp_positive.removeLod(l);else
         {
            dest_lod_p. copyParams(src_lod);
            dest_lod_p.scaleParams(scale);
         }

         if(!dest_lod_n.parts.elms())temp_negative.removeLod(l);else
         {
            dest_lod_n. copyParams(src_lod);
            dest_lod_n.scaleParams(scale);
         }

         //dest_lod_p.weldVtxValues(VTX_POS, EPS);
         //dest_lod_n.weldVtxValues(VTX_POS, EPS);
      }
      temp_positive.setBox();
      temp_negative.setBox();
      
      // set manual lod center after setting box
      temp_positive.lod_center=
      temp_negative.lod_center=src.lod_center;
      if(matrix)
      {
         temp_positive.lod_center*=*matrix;
         temp_negative.lod_center*=*matrix;
      }
   }

   Swap(dest_positive, temp_positive);
   Swap(dest_negative, temp_negative);
}
/******************************************************************************/
}
/******************************************************************************/
