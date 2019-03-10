/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// INFLATE EDGES
/******************************************************************************/
struct EdgeTex
{
   Edge  edge;
   Vec2  tex[2];
   Color col[2];

   void set(C Vec &p0, C Vec &p1, C Vec2 &t0, C Vec2 &t1, C Color &c0, C Color &c1) {edge.set(p0,p1); tex[0]=t0; tex[1]=t1; col[0]=c0; col[1]=c1;}
};
struct TriTex
{
   Tri   tri;
   Vec2  tex[3];
   Color col[3];

   void set(Vec &p0, Vec &p1, Vec &p2, Vec2 &t0, Vec2 &t1, Vec2 &t2, C Color &c0, C Color &c1, C Color &c2) {tri.set(p0, p1, p2); tex[0]=t0; tex[1]=t1; tex[2]=t2; col[0]=c0; col[1]=c1; col[2]=c2;}
};
struct QuadTex
{
   Quad  quad;
   Vec2  tex[4];
   Color col[4];

   void set(Vec &p0, Vec &p1, Vec &p2, Vec &p3, Vec2 &t0, Vec2 &t1, Vec2 &t2, Vec2 &t3, C Color &c0, C Color &c1, C Color &c2, C Color &c3) {quad.set(p0, p1, p2, p3); tex[0]=t0; tex[1]=t1; tex[2]=t2; tex[3]=t3; col[0]=c0; col[1]=c1; col[2]=c2; col[3]=c3;}
};
MeshBase& MeshBase::inflateEdges(TEX_MODE x_tex_coords, TEX_MODE y_tex_coords, Bool to_edges) // assumes that vertexes are welded, weldVtx
{
   Bool          adjust_length=true;
   Index         vtx_vtx, vtx_edge;
   Memc<EdgeTex> fat_edge;
   Memc<TriTex > fat_tri ;
   Memc<QuadTex> fat_quad;
   Bool         *tex_set=null;
   Flt          *tex_y  =null,
                *size   =vtx.size();
   Vec          *pos    =vtx.pos ();

   linkVtxVtxOnEdge(vtx_vtx, true);
   if(y_tex_coords!=TEX_NONE && y_tex_coords!=TEX_KEEP)
   {
      linkVtxEdge(vtx_edge, false);

      AllocZero(tex_set, vtxs());
      Alloc    (tex_y  , vtxs());

      Memc<Int> vtx_left;
      FREPA(vtx)if(!tex_set[i]) // iterate all not yet set vertexes (in case there are multiple sets not joined together)
      {
         vtx_left.New()=i;
         tex_set[i]=true;
         tex_y  [i]=0;
         for(; vtx_left.elms(); ) // fire the "recursive" loop
         {
            Int         v =vtx_left.pop  ( ); // take vertex
            IndexGroup &ig=vtx_edge.group[v];
            REPA(ig) // iterate all its edges, and find a vertex which hasn't been set yet
            {
               Int    e  =ig[i];
               VecI2 &ind=edge.ind(e);
               Int    v2 =-1, dir;
               if(!tex_set[ind.y]){v2=ind.y; dir= 1;}else
               if(!tex_set[ind.x]){v2=ind.x; dir=-1;}
               if(v2>=0)
               {
                  Flt dist=Dist(pos[v], pos[v2]);
                  if( dist)
                  {
                     Flt old_z=pos[v ].z,
                         new_z=pos[v2].z;
                     if( new_z<old_z) // only if flowing downwards (falling)
                     {
                        Flt dist_z=Abs(old_z-new_z), // if most of the distance is vertical (here described by Z) then increase the "speed of river" (by reducing the difference between texture coordinates)
                            frac  =dist_z/dist;
                        dist*=Lerp(0.1f, 1.0f, Sqr(1-frac));
                     }
                  }
                  tex_set[v2]=true;
                  tex_y  [v2]=tex_y[v]+dist*dir; // set tex coords as equal to previous vertex + distance between them
                  vtx_left.add(v2);
               }
            }
         }
      }
      if(y_tex_coords==TEX_UNIFORM)
      {
         Flt min, max; MinMax(tex_y, vtxs(), min, max);
         Flt scale=max-min; if(scale)scale=1.0f/scale; Flt add=-min*scale;
         REP(vtxs())tex_y[i]=tex_y[i]*scale+add;
      }
   }

   REPAD(e, edge)
   {
      VecI2 &ind=edge.ind(e);
      Int    p0 =ind.x,
             p1 =ind.y;

      // we're going from p0 to p1
      Flt    size0=(size ? size[p0] : 1),
             size1=(size ? size[p1] : 1);
      Vec    pos0 =pos[p0],
             pos1 =pos[p1];
      Vec2   right=PerpN(pos1.xy-pos0.xy);

      // find a pair of vertexes for p0
      Int p0_p=-1, p0_n=-1; // previous, next
      {
         IndexGroup &ig=vtx_vtx.group[p0];
         if(ig.num>1) // if vertex is connected with at least 2 others
            REPA(ig)if(ig.elm[i]==p1)
         {
            p0_p=ig.elm[(i+ig.num-1)%ig.num];
            p0_n=ig.elm[(i+       1)%ig.num];
            break;
         }
      }

      // find a pair of vertexes for p1
      Int p1_p=-1, p1_n=-1; // previous, next
      {
         IndexGroup &ig=vtx_vtx.group[p1];
         if(ig.num>1) // if vertex is connected with at least 2 others
            REPA(ig)if(ig.elm[i]==p0)
         {
            p1_p=ig.elm[(i+ig.num-1)%ig.num];
            p1_n=ig.elm[(i+       1)%ig.num];
            break;
         }
      }

      Vec    lb,  rb,  lf,  rf; // pos
      Vec2  tlb, trb, tlf, trf; // tex
      Color  cb,       cf     ; // col
      Flt    max_div=SQRT2_2;
      if(vtx.color())
      {
         cb=vtx.color(p0);
         cf=vtx.color(p1);
      }
      // lb
      {
         Vec2 n=-right;
         Flt  s= size0;
         if(p0_n>=0)
         {
            Vec2 nn   =PerpN(pos[p0_n].xy-pos0.xy);
            Flt  angle=AngleFull(PI-AngleBetween(n, nn));
            n+=nn;
            n.normalize();
            if(adjust_length)s/=Max(max_div, Sin(angle/2));
         }
         n*=s;
         lb=pos0+n.xy0();
         switch(x_tex_coords)
         {
            default         : tlb.x= 0; break;
            case TEX_KEEP   : tlb.x=(vtx.tex0() ? vtx.tex0(p0).x : 0); break;
            case TEX_UNIFORM: tlb.x= 0; break;
            case TEX_SCALED : tlb.x=-s; break;
         }
         switch(y_tex_coords)
         {
            default         : tlb.y=0; break;
            case TEX_KEEP   : tlb.y=(vtx.tex0() ? vtx.tex0(p0).y : 0); break;
            case TEX_UNIFORM:
            case TEX_SCALED : tlb.y=tex_y[p0]; break;
         }
      }
      // rb
      {
         Vec2 n=right;
         Flt  s=size0;
         if(p0_p>=0)
         {
            Vec2 nn   =PerpN(pos0.xy-pos[p0_p].xy);
            Flt  angle=AngleFull(PI-AngleBetween(n, nn));
            n+=nn;
            n.normalize();
            if(adjust_length)s/=Max(max_div, Sin(angle/2));
         }
         n*=s;
         rb=pos0+n.xy0();
         switch(x_tex_coords)
         {
            default         : trb.x=0; break;
            case TEX_KEEP   : trb.x=(vtx.tex0() ? vtx.tex0(p0).x : 0); break;
            case TEX_UNIFORM: trb.x=1; break;
            case TEX_SCALED : trb.x=s; break;
         }
         switch(y_tex_coords)
         {
            default         : trb.y=0; break;
            case TEX_KEEP   : trb.y=(vtx.tex0() ? vtx.tex0(p0).y : 0); break;
            case TEX_UNIFORM:
            case TEX_SCALED : trb.y=tex_y[p0]; break;
         }
      }
      // lf
      {
         Vec2 n=-right;
         Flt  s= size1;
         if(p1_p>=0)
         {
            Vec2 nn   =PerpN(pos1.xy-pos[p1_p].xy);
            Flt  angle=AngleFull(PI-AngleBetween(n, nn));
            n+=nn;
            n.normalize();
            if(adjust_length)s/=Max(max_div, Sin(angle/2));
         }
         n*=s;
         lf=pos1+n.xy0();
         switch(x_tex_coords)
         {
            default         : tlf.x= 0; break;
            case TEX_KEEP   : tlf.x=(vtx.tex0() ? vtx.tex0(p1).x : 0); break;
            case TEX_UNIFORM: tlf.x= 0; break;
            case TEX_SCALED : tlf.x=-s; break;
         }
         switch(y_tex_coords)
         {
            default         : tlf.y=0; break;
            case TEX_KEEP   : tlf.y=(vtx.tex0() ? vtx.tex0(p1).y : 0); break;
            case TEX_UNIFORM:
            case TEX_SCALED : tlf.y=tex_y[p1]; break;
         }
      }
      // rf
      {
         Vec2 n=right;
         Flt  s=size1;
         if(p1_n>=0)
         {
            Vec2 nn   =PerpN(pos[p1_n].xy-pos1.xy);
            Flt  angle=AngleFull(PI-AngleBetween(n, nn));
            n+=nn;
            n.normalize();
            if(adjust_length)s/=Max(max_div, Sin(angle/2));
         }
         n*=s;
         rf=pos1+n.xy0();
         switch(x_tex_coords)
         {
            default         : trf.x=0; break;
            case TEX_KEEP   : trf.x=(vtx.tex0() ? vtx.tex0(p1).x : 0); break;
            case TEX_UNIFORM: trf.x=1; break;
            case TEX_SCALED : trf.x=s; break;
         }
         switch(y_tex_coords)
         {
            default         : trf.y=0; break;
            case TEX_KEEP   : trf.y=(vtx.tex0() ? vtx.tex0(p1).y : 0); break;
            case TEX_UNIFORM:
            case TEX_SCALED : trf.y=tex_y[p1]; break;
         }
      }

      if(to_edges)
      {
                             fat_edge.New().set(lb, lf, tlb, tlf, cb, cf); // left  edge
                             fat_edge.New().set(rf, rb, trf, trb, cf, cb); // right edge
         if(p0_p<0 && p0_n<0)fat_edge.New().set(rb, lb, trb, tlb, cb, cb); // upper edge
         if(p1_p<0 && p1_n<0)fat_edge.New().set(lf, rf, tlf, trf, cf, cf); // lower edge
      }else
      {
         if(p0_p>=0 && p0_n>=0 && p0_p!=p0_n)
         {
            Vec2 tb=Avg(tlb, trb);
            if(y_tex_coords==TEX_SCALED)
            {
               Flt d=Dist(Avg(lb, rb), pos0);
               tb.y=tex_y[p0]-d;
            }
            fat_tri.New().set(lb, rb, pos0, tlb, trb, tb, cb, cb, cb);
         }
         if(p1_p>=0 && p1_n>=0 && p1_p!=p1_n)
         {
            Vec2 tf=Avg(tlf, trf);
            if(y_tex_coords==TEX_SCALED)
            {
               Flt d=Dist(Avg(lf, rf), pos1);
               tf.y=tex_y[p1]+d;
            }
            fat_tri.New().set(rf, lf, pos1, trf, tlf, tf, cf, cf, cf);
         }
         fat_quad.New().set(lb, lf, rf, rb, tlb, tlf, trf, trb, cb, cf, cf, cb);
      }
   }

   Bool set_tex_coords=(x_tex_coords==TEX_UNIFORM || x_tex_coords==TEX_SCALED
                    ||  y_tex_coords==TEX_UNIFORM || y_tex_coords==TEX_SCALED
                    || ((x_tex_coords==TEX_KEEP || y_tex_coords==TEX_KEEP) && vtx.tex0())
                       );
   if(to_edges) // create from edges
   {
      create(fat_edge.elms()*2, fat_edge.elms(), 0, 0, EDGE_FLAG | (set_tex_coords ? VTX_TEX0 : 0) | (vtx.color() ? VTX_COLOR : 0));
      REPA(fat_edge)
      {
         if(vtx.tex0())
         {
            vtx.tex0(i*2+0)=fat_edge[i].tex[0];
            vtx.tex0(i*2+1)=fat_edge[i].tex[1];
         }
         if(vtx.color())
         {
            vtx.color(i*2+0)=fat_edge[i].col[0];
            vtx.color(i*2+1)=fat_edge[i].col[1];
         }
         vtx .pos (i*2+0)=fat_edge[i].edge.p[0];
         vtx .pos (i*2+1)=fat_edge[i].edge.p[1];
         edge.ind (i).set(i*2+0, i*2+1);
         edge.flag(i)=ETQ_R;
      }
   }else // create from faces
   {
      create(fat_tri.elms()*3+fat_quad.elms()*4, 0, fat_tri.elms(), fat_quad.elms(), (set_tex_coords ? VTX_TEX0 : 0) | (vtx.color() ? VTX_COLOR : 0));
      Int ofs=0;
      FREPA(fat_tri)
      {
         TriTex &t=fat_tri[i];
         if(vtx.tex0())
         {
            vtx.tex0(ofs+0)=t.tex[0];
            vtx.tex0(ofs+1)=t.tex[1];
            vtx.tex0(ofs+2)=t.tex[2];
         }
         if(vtx.color())
         {
            vtx.color(ofs+0)=t.col[0];
            vtx.color(ofs+1)=t.col[1];
            vtx.color(ofs+2)=t.col[2];
         }
         vtx.pos(ofs+0)=t.tri.p[0];
         vtx.pos(ofs+1)=t.tri.p[1];
         vtx.pos(ofs+2)=t.tri.p[2];
         tri.ind(i).set(ofs+0, ofs+1, ofs+2);
         ofs+=3;
      }
      FREPA(fat_quad)
      {
         QuadTex &q=fat_quad[i];
         if(vtx.tex0())
         {
            vtx.tex0(ofs+0)=q.tex[0];
            vtx.tex0(ofs+1)=q.tex[1];
            vtx.tex0(ofs+2)=q.tex[2];
            vtx.tex0(ofs+3)=q.tex[3];
         }
         if(vtx.color())
         {
            vtx.color(ofs+0)=q.col[0];
            vtx.color(ofs+1)=q.col[1];
            vtx.color(ofs+2)=q.col[2];
            vtx.color(ofs+3)=q.col[3];
         }
         vtx .pos(ofs+0)=q.quad.p[0];
         vtx .pos(ofs+1)=q.quad.p[1];
         vtx .pos(ofs+2)=q.quad.p[2];
         vtx .pos(ofs+3)=q.quad.p[3];
         quad.ind(i).set(ofs+0, ofs+1, ofs+2, ofs+3);
         ofs+=4;
      }
      setVtxDup();
      
   }
   weldVtx(VTX_TEX_ALL);

   // free
   Free(tex_set);
   Free(tex_y  );

   return T;
}
/******************************************************************************/
// EDGE TO POLY
/******************************************************************************/
MeshBase& MeshBase::edgeToPoly(MemPtr<Poly> polys)
{
   polys.clear();
   if(vtxs() && edges() && edge.flag())
   {
      Byte *flag=Alloc<Byte>(edges()); CopyN(flag, edge.flag(), edges());

      // link vtx->edge
      Index vtx_edge; linkVtxEdge(vtx_edge);

      // get polys
      REPAD(e, edge)if(flag[e]&ETQ_LR)                 // for each solid edge
      REPD (side, 2)if(flag[e]&(side ? ETQ_R : ETQ_L)) // for each solid side
      {
         Poly &poly    =polys.New().create(vtx.pos(), edge.id() ? edge.id(e) : -1); // start creating new poly and optionally set its ID from edge ID
         Int   prev_vtx=edge .ind (e).c[!side], // setting 'prev' and 'cur' vertexes determines movement direction
                cur_vtx=edge .ind (e).c[ side], next_vtx;
                              flag[e]&=( side ? ~ETQ_R : ~ETQ_L); // eat solid

         for(Int edge_cur=e; ; ) // start movement loop
         {
            next_vtx=-1;                     // set next vertex as unknown
            poly.addVtx(prev_vtx, edge_cur); // add 'prev' vertex to the poly list
            IndexGroup &ig=vtx_edge.group[cur_vtx]; // get next edge linked with 'cur_vtx'
            REPA(ig) // iterate all edges linked with 'cur_vtx' to find the next valid edge (which has solid info)
            {
               Int edge_test=ig[i]; if(edge_test==edge_cur) // find 'i' position in which the edge is equal to the current
               {
                  Bool other_side=false; // work mode
                  for(Int j=1; j<=ig.num; j++) // start from the next edge (+1) but finish also on the starting (edge_cur) because there are cases that only it's the correct one
                  {
                     edge_test=ig[(i+j)%ig.num]; Int *p=edge.ind(edge_test).c; Byte &fl=flag[edge_test], f=ETQ_R, nf=ETQ_L; // on the end when j==ig.num, (i+ig.num)%ig.num==i, this means that at the end will be checked the starting edge (edge_cur)
                     next_vtx =p[1]; if(next_vtx==cur_vtx){next_vtx=p[0]; Swap(f, nf);} // make sure that the next vertex isn't set as the current one

                     if(!other_side) // normal work mode
                     {
                        if(fl&f){fl&=~f; break;}else // encountered edge has desired info about solid, so let's eat it and finish
                        {
                           next_vtx=-1;
                           if(fl&nf)other_side=true; // encountered edge has info about solid but from the other side, this means that now we'll have to skip all information about solid because we're not interested in them
                        }
                     }else // secondary work mode in which we skip information about encountered solid
                     {
                        if(fl&f && !(fl&nf))other_side=false; // we've encountered closing solid
                        next_vtx=-1;
                     }
                  }
                  edge_cur=edge_test;
                  break;
               }
            }
            if(next_vtx<0)break;
               prev_vtx= cur_vtx;
                cur_vtx=next_vtx;
         }
         if(poly.vtx.elms()<=2)polys.removeLast();
      }

      // free
      Free(flag);
   }
   return T;
}
/******************************************************************************/
// EDGE TO DEPTH
/******************************************************************************/
struct EdgeWall
{
   VecI4 ind;
   Int   id;
};
MeshBase& MeshBase::edgeToDepth(Bool tex_align)
{
   Memb<EdgeWall> wall(256);
   Memb<Poly    > polys    ; edgeToPoly(polys);

   Int      j=0, edges=0; FREPA(polys)edges+=polys[i].vtx.elms();
   MeshBase temp(2*(edges+polys.elms()), 0, 0, 0, VTX_TEX0);
   Vec2    *tex   =temp.vtx .tex0();
   Flt     *height=     vtx .size();
   Vec     *src   =     vtx .pos (),
           *dest  =temp.vtx .pos ();
   Byte    *flag  =     edge.flag();
   Int     *eid   =     edge.id  ();

   FREPA(polys)
   {
      Poly &poly=polys[i];
      Flt   t=0, tex_scale=1;
      if(tex_align)
      {
         Flt length=poly.length3D();
         tex_scale =Max(1, RoundPos(length))/length;
      }
      {
         Int i    =poly.vtx[poly.vtx.last()].index,
             e    =poly.vtx[poly.vtx.last()].edge ;
         Flt h    =(height ? height[i] : 1);
         Vec prev =src [i];
         dest[j  ]=dest[j+1]=prev; dest[j].z+=h;
         tex [j  ].set(0, 0);
         tex [j+1].set(0, h);

         MFREPD(vtx, poly.vtx)
         {
            if(!flag || !(flag[e]&ETQ_NO_PHYS))
            {
               EdgeWall &w=wall.New();
                      w.ind.set(j, j+2, j+3, j+1);
               if(eid)w.id =eid[e];
            }
            j+=2;
            i=poly.vtx[vtx].index;
            e=poly.vtx[vtx].edge ;
            h=(height ? height[i] : 1);
            Vec &next=src [i]; t+=Dist(prev, next)*tex_scale;
            dest[j  ]=dest[j+1]=next; dest[j].z+=h;
            tex [j  ].set(t, 0);
            tex [j+1].set(t, h);
            prev=next;
         }
         j+=2;
      }
   }

   temp.quad._elms=wall.elms();
   temp.include(QUAD_IND|(eid ? QUAD_ID : 0));
   REPA(wall)
   {
      EdgeWall &w=wall[i];
                        temp.quad.ind(i)=w.ind;
      if(temp.quad.id())temp.quad.id (i)=w.id ;
   }

   Swap(T, temp); return T;
}
/******************************************************************************/
// EDGE TO TRI
/******************************************************************************/
#define ETT_EPS 0.09f
struct ETT_TRI
{
   VecI ind;
   Int  id;
};
struct FloatPtr // Float + Pointer
{
   Flt f; // float
   Ptr p; // pointer

   static Int Compare(C FloatPtr &a, C FloatPtr &b) {return ::Compare(a.f, b.f);}
};
struct ETT
{
   Meml<Poly>    polys;
   Vec          *pos; Int vtxs;
   FloatPtr     *fi ; Int fis;

   Rects         rects;
   Index         rect_vtx;
   IndexPtr      vtx_poly;

   Memb<ETT_TRI> mtri ;
   MemlNode     *mpoly;
   Poly         * poly;

  ~ETT()
   {
      /*#if DEBUG
      extern Int I;
      if(polys.elms())
      {
         do{
            GetInput(); CAMHndl(0.00001f, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
            DISPClr(GREY);
            DRAWSet(Vec2(1)/CAM_CUR.dist, -CAM_CUR.at.xy);
            REPA(mtri)
            {
               TRIDraw(Tri2(pos[mtri[i].c[0]].xy,
                            pos[mtri[i].c[1]].xy,
                            pos[mtri[i].c[2]].xy), BLACK);
            }
            Int j=0; MFREP(polys)POLYDraw(memn->dColI(j++));
            DRAWSet(Vec2(1), Vec2(0));
            D.text(0, 0.7f, S+polys.elms()+" Poly(s) left on "+I);
            DISPFlip();
         }while(!Kb.bp(KB_SPACE));
         // Exit(S+polys.elms()+" Poly(s) left on "+I);
      }
      #endif*/
      Free(fi);
   }
   void vtxRem(MemlNode *vtx)
   {
      Int            index=poly->vtx[vtx].index;
      IndexPtrGroup &ig   =vtx_poly.group[index]; ig.subVal(mpoly); if(!ig.num)rect_vtx.group[rects.index(pos[index].xy)].subVal(index);
      poly->removeVtx(vtx);
   }
   Int testTri(MemlNode *vtx, Bool test) // returns 0-not ok try next vertex, 1-added triangle and repeat the test, 2-ok finish the test
   {
      Int  p0=poly->vtx[poly->vtx.loopPrev(vtx)].index,
           p1=poly->vtx[                   vtx ].index,
           p2=poly->vtx[poly->vtx.loopNext(vtx)].index;
      Vec &P0=pos[p0],
          &P1=pos[p1],
          &P2=pos[p2];
      Tri2 tri(P0.xy, P1.xy, P2.xy);
      if(test ? tri.valid() : true)
      {
         // test if triangle does not contain any points inside
         Plane plane(P0, GetNormal(P0, P1, P2));
         Int   vtx_cut=-1;
         Flt   dist;
         Rect  rect    =tri; rect.extend(EPS);
         VecI2 rect_min=rects.coords(rect.min),
               rect_max=rects.coords(rect.max);
         for(Int y=rect_min.y; y<=rect_max.y; y++)
         for(Int x=rect_min.x; x<=rect_max.x; x++)
         {
            IndexGroup &ig=rect_vtx.group[rects.index(VecI2(x, y))];
            REPAD(v, ig)
            {
               Int pt=ig[v];
               if( pt!=p0 && pt!=p1 && pt!=p2)
               {
                  Vec2 &vt=pos[pt].xy;
                  if(test ? CutsEps(vt, tri) : Cuts(vt, tri))
                  if(Abs(Dist(pos[pt], plane))<=ETT_EPS)
                  {
                     Flt d=Min(DistPointEdge(vt, tri.p[0], tri.p[1]), DistPointEdge(vt, tri.p[1], tri.p[2]));
                     if(vtx_cut<0 || d<dist){vtx_cut=pt; dist=d;}
                  }
               }
            }
         }

         if(vtx_cut<0) // no vtx inside
         {
            // create the tri basing on the source 3
            ETT_TRI &et=mtri.New();
            et.ind.set(p0, p1, p2);
            et.id =poly->id;

            vtxRem(vtx);
            if(poly->vtx.elms()<=2) // there won't be anything more out of the poly
            {
               for(; poly->vtx.elms(); )vtxRem(poly->vtx.first());
               MemlNode *next=mpoly->next();
               poly->del(); polys.remove(mpoly);
               mpoly=next;
               return 2;
            }
            return 1;
         }

         // vtx inside
         MemlNode      *mpoly_cut=null;
         IndexPtrGroup &ig       =vtx_poly.group[vtx_cut];
         REPAD(p, ig)
         {
            MemlNode  *tpoly=(MemlNode*)ig[p]; if(tpoly==mpoly){mpoly_cut=null; break;} // cuts self & skip
            if(((Poly*)tpoly->data())->infinite())mpoly_cut=tpoly;
         }
         if(mpoly_cut) // link two polys
         {
            Poly &poly2=*(Poly*)mpoly_cut->data();
            MemlNode *vtx2=null; MFREP(poly2.vtx)
            {
               Int index=poly2.vtx[i].index; if(index==vtx_cut)vtx2=i;
               vtx_poly.group[index].changeVal(mpoly_cut, mpoly);
            }
            poly->link(poly2, vtx, vtx2); poly2.del(); polys.remove(mpoly_cut);
            if(poly->vtx.elms()>fis)Realloc(fi, fis=poly->vtx.elms(), 0);
            return 1;
         }
      }
      return 0;
   }
};
MeshBase& MeshBase::edgeToTri(Bool set_id) // assumes: weldVtx, removeUnusedVtxs, this method is required not to modify vertexes (only faces should be changed)
{
   if(vtxs() && edges() && edge.flag())
   {
      ETT ett;

      // convert to polys
      ett.vtxs=vtxs();
      ett.pos =vtx.pos();
      edgeToPoly(ett.polys);

      // link rect->vtxs
      ett.rects   .set   (Rect(T), ett.vtxs);
      ett.rect_vtx.create(ett.rects.num(), ett.vtxs);
      FREP(ett.vtxs)ett.rect_vtx.elmGroup(ett.rects.index(ett.pos[i].xy), i); ett.rect_vtx.set();

      // link vtx->polys
      ett.vtx_poly.create(ett.vtxs);
      MFREP(ett.polys)MFREPD(vtx, ett.polys[i].vtx)ett.vtx_poly.incGroup(ett.polys[i].vtx[vtx].index   ); ett.vtx_poly.set();
      MFREP(ett.polys)MFREPD(vtx, ett.polys[i].vtx)ett.vtx_poly.addElm  (ett.polys[i].vtx[vtx].index, i);

      // create angle array
      ett.fis=0; MFREP(ett.polys)
      {
         Poly &poly=ett.polys[i]; poly.setAngle();
         ett.fis=Max(ett.fis, poly.vtx.elms());
      }
      Alloc(ett.fi, ett.fis);

      // convert polys to triangles
      REPD(test, 2)for(ett.mpoly=ett.polys.first(); ett.mpoly; )
      {
         ett.poly=&ett.polys[ett.mpoly];
         if(!ett.poly->infinite())
         {
         test:;
            FloatPtr *fi=ett.fi;
            MFREPD(vtx, ett.poly->vtx)
            {
               fi->p=vtx;
               fi->f=ett.poly->vtx[vtx].angle;
               fi++;
            }
            Sort (ett.fi, ett.poly->vtx.elms(), FloatPtr::Compare);
            FREPA(ett.poly->vtx)
            {
               Flt a =ett.fi[i].f;
               if( a>=PI-EPS)break;
               if( a>    EPS)
               {
                  Int c=ett.testTri((MemlNode*)ett.fi[i].p, test!=0);
                  if( c==1)goto test;
                  if( c==2)goto done;
               }
            }
         }
         ett.mpoly=ett.mpoly->next();
      done:;
      }

      // free & create
      exclude(ADJ_ALL|FACE_ALL); tri._elms=ett.mtri.elms();
      include(TRI_IND|(set_id ? TRI_ID : 0));
      FREPA(ett.mtri)
      {
         ETT_TRI &et=ett.mtri[i];
                     tri.ind(i)=et.ind;
         if(tri.id())tri.id (i)=et.id ;
      }
   }
   return T;
}
/******************************************************************************/
// TRI TO QUAD
/******************************************************************************/
struct TriLink
{
   Int tri[2];
   Flt length2;

   void set(Int t0, Int t1, Flt length2) {tri[0]=t0; tri[1]=t1; T.length2=length2;}

   static Int Compare(C TriLink &l0, C TriLink &l1)
   {
      if(l0.length2>l1.length2)return -1;
      if(l0.length2<l1.length2)return +1;
                               return  0;
   }
};
MeshBase& MeshBase::triToQuad(Flt cos)
{
   if(tris())
   {
      setAdjacencies(true, false);
      setFaceNormals();
      Memb<TriLink> link(64);
      FREPA(tri)REPD(j, 3)
      {
         Int af =tri.adjFace(i).c[j];
         if( af!=-1 && !(af&SIGN_BIT) && i<af && Dot(tri.nrm(i), tri.nrm(af))>=cos) // compare to -1 and not >=0 because it can have SIGN_BIT
         {
            Int *p0=tri.ind(i ).c,
                *p1=tri.ind(af).c;
            Int  i0=j;
            REPD(i1, 3)if(p0[i0]==p1[(i1+1)%3] && p0[(i0+1)%3]==p1[i1])
            {
               Vec &a=vtx.pos(p0[ i0     ]),
                   &b=vtx.pos(p1[(i1+2)%3]),
                   &c=vtx.pos(p0[(i0+1)%3]),
                   &d=vtx.pos(p0[(i0+2)%3]),
                  nrm=Cross(tri.nrm(i), b-d);
               if(DistPointPlane(c, d, nrm)> EPS
               && DistPointPlane(a, d, nrm)<-EPS)link.New().set(i, af, Dist2(a, c));
               break;
            }
         }
      }
      if(link.elms())
      {
         exclude(ADJ_ALL);
         link.sort(TriLink::Compare);
         Memt<Int> linked; linked.setNum(tris()); SetMemN(linked.data(), 0xFF, linked.elms());
         FREPA(link)
         {
            TriLink &l=link[i];
            if(linked[l.tri[0]]<0 && linked[l.tri[1]]<0)
            {
               linked[l.tri[0]]=l.tri[1];
               linked[l.tri[1]]=l.tri[0];
            }
         }

         Memt<Bool> is; is.setNum(T.tris()); FREPA(tri)is[i]=(linked[i]<0);
         Int      tris=CountIs(is);
         MeshBase temp(0, 0, tris, quads()+(T.tris()-tris)/2);
         CopyIs(temp.tri .ind(), tri .ind(), is       );
         CopyN (temp.quad.ind(), quad.ind(), T.quads());
         VecI4 *_quad=temp.quad.ind()+quads();
         FREPA(tri)
         {
            Int  n2=linked[i];
            if(i<n2)
            {
               Int *p0=tri.ind(i ).c,
                   *p1=tri.ind(n2).c;
               Int  i0, i1; for(i0=3; --i0>=0; )for(i1=3; --i1>=0; )if(p0[i0]==p1[(i1+1)%3] && p0[(i0+1)%3]==p1[i1])goto found; found:;
               (_quad++)->set(p1[(i1+2)%3], p0[(i0+1)%3], p0[(i0+2)%3], p0[i0]);
            }
         }
         Swap(tri , temp.tri );
         Swap(quad, temp.quad);
      }
   }
   return T;
}
/******************************************************************************/
// QUAD TO TRI
/******************************************************************************/
MeshBase& MeshBase::quadToTri(Flt cos)
{
   if(quads() && cos>-1) // if cos<=-1 then keep all quads
   if(!tris() || (!tri.ind())==(!quad.ind())) // if existence of vtx ID's match
   {
      if(tris() && !tri.nrm() && quad.nrm())setFaceNormals(); // if there are tris, and quads have normals but tris don't, then set normals for tris

      exclude(ADJ_ALL);

      UInt f=flag();
      if(f&QUAD_NRM )f|=TRI_NRM ;
      if(f&QUAD_FLAG)f|=TRI_FLAG;
      if(f&QUAD_ID  )f|=TRI_ID  ;

    C Vec *pos=vtx.pos();
      if(cos>1 || !quad.ind() || !pos) // convert all quads to tris
      {
         MeshBase temp(0, 0, tris()+quads()*2, 0, f);
         CopyN(temp.tri.ind (), tri.ind (), tris());
         CopyN(temp.tri.nrm (), tri.nrm (), tris());
         CopyN(temp.tri.flag(), tri.flag(), tris());
         CopyN(temp.tri.id  (), tri.id  (), tris());
         if(quad.ind())
         {
            VecI4 *s=    quad.ind();
            VecI  *d=temp.tri.ind()+tris();
            REPA(quad)
            {
               d[0].set(s->c[0], s->c[1], s->c[3]);
               d[1].set(s->c[3], s->c[1], s->c[2]); s++; d+=2;
            }
         }
         if(quad.nrm())
         {
            Vec *s=    quad.nrm(),
                *d=temp.tri.nrm()+tris();
            REPA(quad){d[0]=d[1]=*s++; d+=2;}
         }
         if(quad.flag())
         {
            Byte *s=    quad.flag(),
                 *d=temp.tri.flag()+tris();
            REPA(quad){d[0]=d[1]=*s++; d+=2;}
         }
         if(quad.id())
         {
            Int *s=    quad.id(),
                *d=temp.tri.id()+tris();
            REPA(quad){d[0]=d[1]=*s++; d+=2;}
         }
         Swap(tri , temp.tri );
         Swap(quad, temp.quad);
      }else
      {
         // check which quads should be left/separated
         Memt<Bool> keep; keep.setNum(quads());
         Int        keep_num=0, split_num=0;
         REPA(quad)
         {
          C VecI4 &p =quad.ind(i);
          C Vec   &p0=pos[p.x],
                  &p1=pos[p.y],
                  &p2=pos[p.z],
                  &p3=pos[p.w];
            if(keep[i]=(Dot(GetNormal(p0, p1, p3), GetNormal(p1, p2, p3))>=cos))keep_num++;else split_num++;
         }
         
         // separate
         MeshBase temp(0, 0, tris()+split_num*2, keep_num, f);
         CopyN(temp.tri.ind (), tri.ind (), tris());
         CopyN(temp.tri.nrm (), tri.nrm (), tris());
         CopyN(temp.tri.flag(), tri.flag(), tris());
         CopyN(temp.tri.id  (), tri.id  (), tris());
         if(quad.ind())
         {
            VecI4 *s =     quad.ind(),
                  *dq=temp.quad.ind();
            VecI  *dt=temp.tri .ind()+tris();
            FREPA(quad)
            {
               if(keep[i])
               {
                  *dq++=*s++;
               }else
               {
                  dt[0].set(s->c[0], s->c[1], s->c[3]);
                  dt[1].set(s->c[3], s->c[1], s->c[2]); dt+=2; s++;
               }
            }
         }
         if(quad.nrm())
         {
            Vec *s =     quad.nrm(),
                *dq=temp.quad.nrm(),
                *dt=temp.tri .nrm()+tris();
            FREPA(quad)
            {
               if(keep[i])
               {
                  *dq++=*s++;
               }else
               {
                  dt[0]=dt[1]=*s++; dt+=2;}
               }
         }
         if(quad.flag())
         {
            Byte *s =     quad.flag(),
                 *dq=temp.quad.flag(),
                 *dt=temp.tri .flag()+tris();
            FREPA(quad)
            {
               if(keep[i])
               {
                  *dq++=*s++;
               }else
               {
                  dt[0]=dt[1]=*s++; dt+=2;
               }
            }
         }
         if(quad.id())
         {
            Int *s =     quad.id(),
                *dq=temp.quad.id(),
                *dt=temp.tri .id()+tris();
            FREPA(quad)
            {
               if(keep[i])
               {
                  *dq++=*s++;
               }else
               {
                  dt[0]=dt[1]=*s++; dt+=2;
               }
            }
         }
         Swap(tri , temp.tri );
         Swap(quad, temp.quad);
      }
   }
   return T;
}
MeshBase& MeshBase::quadToTri(C MemPtr<Int> &quads)
{
   Memt<Int> actual_quads; FREPA(quads){Int q=quads[i]; if(InRange(q, T.quad))actual_quads.binaryInclude(q, Compare);}
   if(       actual_quads.elms())
   {
      if(tris() && !tri.nrm() && quad.nrm())setFaceNormals(); // if there are tris, and quads have normals but tris don't, then set normals for tris

      exclude(ADJ_ALL);

      UInt f=flag();
      if(f&QUAD_NRM )f|=TRI_NRM ;
      if(f&QUAD_FLAG)f|=TRI_FLAG;
      if(f&QUAD_ID  )f|=TRI_ID  ;

      MeshBase temp(0, 0, tris()+actual_quads.elms()*2, 0, f);
      CopyN(temp.tri.ind (), tri.ind (), tris());
      CopyN(temp.tri.nrm (), tri.nrm (), tris());
      CopyN(temp.tri.flag(), tri.flag(), tris());
      CopyN(temp.tri.id  (), tri.id  (), tris());
      if(quad.ind())
      {
         VecI *d=temp.tri.ind()+tris();
         FREPA(actual_quads)
         {
            VecI4 s=quad.ind(actual_quads[i]);
            d[0].set(s.c[0], s.c[1], s.c[3]);
            d[1].set(s.c[3], s.c[1], s.c[2]); d+=2;
         }
      }
      if(quad.nrm())
      {
         Vec *d=temp.tri.nrm()+tris();
         FREPA(actual_quads){d[0]=d[1]=quad.nrm(actual_quads[i]); d+=2;}
      }
      if(quad.flag())
      {
         Byte *d=temp.tri.flag()+tris();
         FREPA(actual_quads){d[0]=d[1]=quad.flag(actual_quads[i]); d+=2;}
      }
      if(quad.id())
      {
         Int *d=temp.tri.id()+tris();
         FREPA(actual_quads){d[0]=d[1]=quad.id(actual_quads[i]); d+=2;}
      }
      Swap(tri, temp.tri);

      removeQuads(actual_quads, false); // no need to remove vtxs because all of them are used by tris
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
