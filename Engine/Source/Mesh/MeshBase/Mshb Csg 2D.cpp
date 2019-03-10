/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
struct MC_POINT
{
   Vec2 pos;
   Int  sec_next;
};
struct MC_EDGE
{
   VecI2 ind;
   Byte  flag, solid;
   Rect  rect;
};
struct SECT : Rects
{
   Int *first;

   void create(C Rect &rect, Int vtx_num)
   {
      set(rect, vtx_num*4);
      Alloc(first, num());
      SetMemN(first, 0xFF, num());
   }
   void del()
   {
      Free(first);
      Zero(T);
   }
};
static struct CSG2D
{
   SECT           sec;
   Memc<MC_POINT> point;
   Memc<MC_EDGE > edge1,
                  edge2,
                  edgec;

   void create(Int vtx_num, C Rect &rect)
   {
      sec.create(rect, vtx_num);
   }
   void del()
   {
      edgec.del();
      edge2.del();
      edge1.del();
      point.del();
      sec  .del();
   }
}Csg2D;
/******************************************************************************/
// DRAW
/******************************************************************************
#if DEBUG
static void draw_side(C Color &color, Vec2 &v1, Vec2 &v2, Bool hlt=false)
{
   Flt  length=Dist(v1, v2);
   Vec2 dir   =!(v2-v1), nrm=Perp(dir);
   Flt  teeth =0.03f;
   if(hlt)teeth=Sin(Time.appTime()*6)*0.015f+0.020f;
   for(Flt step=0; step<=length; step+=0.02f/D.draw_scale.x)
   {
      Vec2 pos=v1+dir*step;
      D.line(color, pos, pos+teeth*(dir+nrm)/D.draw_scale.x);
   }
}
static void draw_edge(C Color &color, MC_EDGE &edge)
{
   Vec2 &v1=Csg2D.point[edge.ind.c[0]].pos,
        &v2=Csg2D.point[edge.ind.c[1]].pos;
   D.line(color, v1, v2);
   D.dot (color, v1, 0.005f/D.draw_scale.x);
   D.dot (color, v2, 0.005f/D.draw_scale.x);
   color=0; if(edge.solid&SOLID_AR)color|=GREEN; if(edge.solid&SOLID_BR)color|=BLUE; if(color)draw_side(color, v1, v2);
   color=0; if(edge.solid&SOLID_AL)color|=GREEN; if(edge.solid&SOLID_BL)color|=BLUE; if(color)draw_side(color, v2, v1);
}
static void disp(Memc<MC_EDGE> &medge)
{
   FREPD(s,2)
   do{
      InpGet();
      DISPClr();
      Csg2D.sec.rect.draw(0xFF660066);
      Flt fx=Csg2D.sec.rect.min.x; REP(Csg2D.sec.cells.x+1){D.line(0xFF880088, fx, Csg2D.sec.rect.min.y, fx, Csg2D.sec.rect.max.y    ); fx+=Csg2D.sec.size.x;}
      Flt fy=Csg2D.sec.rect.min.y; REP(Csg2D.sec.cells.y+1){D.line(0xFF880088,     Csg2D.sec.rect.min.x, fy, Csg2D.sec.rect.max.x, fy); fy+=Csg2D.sec.size.y;}
      REPA(medge)draw_edge(TURQ, medge[i]);
      D.text(0, 0, StrF("xs:%d ys:%d", Csg2D.sec.cells.x, Csg2D.sec.cells.y));
      if(s>0)
      {
         REPD(x, Csg2D.sec.cells.x)
         REPD(y, Csg2D.sec.cells.y)
         {
            Flt fx=Csg2D.sec.rect.min.x+x*Csg2D.sec.size.x,
                fy=Csg2D.sec.rect.min.y+y*Csg2D.sec.size.y;
            for(Int i=Csg2D.sec.first[x+Csg2D.sec.cells.x*y]; i>=0; )
            {
               MC_POINT &point=Csg2D.point[i];
               D.dot(WHITE,point.pos,0.006f/D.draw_scale.x);
               i=point.sec_next;
            }
         }
      }
      DISPFlip();
   }while(!Kb.bp(KB_SPACE));
}
#endif
/******************************************************************************/
// ADD ELEMENTS
/******************************************************************************/
static UInt GetPoint(C Vec2 &pos)
{
   // get sec
   VecI2 crd=Csg2D.sec.coords(pos);
   Int   r  =Csg2D.sec.index (crd);

   // find duplicate
   for(Int sy=-1; sy<=1; sy++)if(InRange(crd.y+sy, Csg2D.sec.cells.y))
   for(Int sx=-1; sx<=1; sx++)if(InRange(crd.x+sx, Csg2D.sec.cells.x))
   for(Int i=Csg2D.sec.first[r+sx+Csg2D.sec.cells.x*sy]; i>=0; )
   {
      MC_POINT &point=Csg2D.point[i];
      if(Equal(point.pos, pos))return i;
      i=point.sec_next;
   }

   // add new
   Int       i    =Csg2D.point.elms();
   MC_POINT &point=Csg2D.point.New();
   point.pos=pos;
   point.sec_next=Csg2D.sec.first[r];
   return         Csg2D.sec.first[r]=i;
}
/******************************************************************************/
static UInt MCEdgeSolidSwap(UInt solid)
{
   UInt s=(solid&(~SOLID_ALL));
   if(solid&SOLID_AL )s|=SOLID_AR ;
   if(solid&SOLID_AR )s|=SOLID_AL ;
   if(solid&SOLID_BL )s|=SOLID_BR ;
   if(solid&SOLID_BR )s|=SOLID_BL ;
   if(solid&SOLID_NAL)s|=SOLID_NAR;
   if(solid&SOLID_NAR)s|=SOLID_NAL;
   if(solid&SOLID_NBL)s|=SOLID_NBR;
   if(solid&SOLID_NBR)s|=SOLID_NBL;
   return s;
}
static void AddEdge(UInt p0, UInt p1, UInt flag, UInt solid, Memc<MC_EDGE> &medge)
{
   if(p0!=p1)
   {
      if(p1<p0)
      {
         Swap(p0, p1);
         flag =    EtqFlagSwap(flag );
         solid=MCEdgeSolidSwap(solid);
      }
      MC_EDGE &edge=medge.New();
      edge.flag =flag;
      edge.solid=solid;
      edge.ind .set (p0, p1);
      edge.rect.from(Csg2D.point[p0].pos, Csg2D.point[p1].pos).extend(EPS);
   }
}
/******************************************************************************/
static void AddMesh(C MeshBase &mshb, Memc<MC_EDGE> &medge, Bool type)
{
   FREPA(mshb.edge)
   {
      C Int *p=mshb.edge.ind(i).c;
      UInt p0=GetPoint(mshb.vtx.pos(p[0]).xy),
           p1=GetPoint(mshb.vtx.pos(p[1]).xy);
      if(p0!=p1)
      {
         UInt flag =0,
              solid=0;
         if(mshb.edge.flag())
         {
            UInt mf=mshb.edge.flag(i);
                              flag |=( mf&~ETQ_LR);
            if(type==TYPE_A)  solid|=((mf& ETQ_L)?SOLID_AL:SOLID_NAL)|((mf&ETQ_R)?SOLID_AR:SOLID_NAR);else
                              solid|=((mf& ETQ_L)?SOLID_BL:SOLID_NBL)|((mf&ETQ_R)?SOLID_BR:SOLID_NBR);
         }
         AddEdge(p0, p1, flag, solid, medge);
      }
   }
}
/******************************************************************************/
// CUT
/******************************************************************************/
static void CutEdgeEdge(Memc<MC_EDGE> &edge0, Memc<MC_EDGE> &edge1, Memc<MC_EDGE> &edgec)
{
   Flt *step_array=Alloc<Flt>(2*edge1.elms());
   FREPA(edge0)
   {
      MC_EDGE  &e0=edge0[i];
      MC_POINT &p1=Csg2D.point[e0.ind.c[0]],
               &p2=Csg2D.point[e0.ind.c[1]];
      Edge2_I   ei(p1.pos, p2.pos);
      Edge2     cuts;
      Int       steps=0;

      FREPAD(j, edge1)
      {
         MC_EDGE &e1=edge1[j];
         if(Cuts(e0.rect, e1.rect))REPD(c, CutsEdgeEdge(ei, Edge2_I(Csg2D.point[e1.ind.c[0]].pos, Csg2D.point[e1.ind.c[1]].pos), &cuts))
         {
            Flt s=DistPointPlane(cuts.p[c], ei.p[0], ei.dir);
            if(s>EPS && s<ei.length-EPS)step_array[steps++]=s/ei.length;
         }
      }
      Int prev=e0.ind.c[0],
          next;
      Sort(step_array, steps); FREPD(j, steps)
      {
         next=GetPoint(Lerp(p1.pos, p2.pos, step_array[j])); AddEdge(prev, next, e0.flag, e0.solid, edgec);
         prev=next;
      }  next=e0.ind.c[1];
      AddEdge(prev, next, e0.flag, e0.solid, edgec);
   }
   Free(step_array);
}
static void CutEdgePoint(Memc<MC_EDGE> &edge, Memc<MC_EDGE> &edgec)
{
   Flt *step_array=Alloc<Flt>(Csg2D.point.elms());
   FREPA(edge)
   {
      MC_EDGE  &e =edge[i];
      MC_POINT &p1=Csg2D.point[e.ind.c[0]],
               &p2=Csg2D.point[e.ind.c[1]];
      Edge2_I   ei(p1.pos, p2.pos);
      Int   steps=0;
      RectI secr;
      secr.from(Csg2D.sec.coords(p1.pos), Csg2D.sec.coords(p2.pos)).extend(1);
      Clamp(secr.min.x, 0, Csg2D.sec.cells.x-1); Clamp(secr.max.x, 0, Csg2D.sec.cells.x-1);
      Clamp(secr.min.y, 0, Csg2D.sec.cells.y-1); Clamp(secr.max.y, 0, Csg2D.sec.cells.y-1);

      for(Int y=secr.min.y; y<=secr.max.y; y++)
      for(Int x=secr.min.x; x<=secr.max.x; x++)
      for(Int tnum=Csg2D.sec.first[x+Csg2D.sec.cells.x*y]; tnum>=0; )
      {
         MC_POINT &tpoint=Csg2D.point[tnum];
         if(tnum!=e.ind.c[0] && tnum!=e.ind.c[1] && CutsPointEdge(tpoint.pos, ei))
         {
            Flt s=DistPointPlane(tpoint.pos, ei.p[0], ei.dir);
            if(s>EPS && s<ei.length-EPS)step_array[steps++]=s/ei.length;
         }
         tnum=tpoint.sec_next;
      }
      Int prev=e.ind.c[0],
          next;
      Sort(step_array, steps); FREPD(j, steps)
      {
         next=GetPoint(Lerp(p1.pos, p2.pos, step_array[j])); AddEdge(prev, next, e.flag, e.solid, edgec);
         prev=next;
      }  next=e.ind.c[1];
      AddEdge(prev, next, e.flag, e.solid, edgec);
   }
   Free(step_array);
}
/******************************************************************************/
// JOIN EDGES
/******************************************************************************/
static Int CompareEdge(C MC_EDGE &e0, C MC_EDGE &e1)
{
   if(e0.ind.c[0]<e1.ind.c[0])return -1;
   if(e0.ind.c[0]>e1.ind.c[0])return +1;
   if(e0.ind.c[1]<e1.ind.c[1])return -1;
   if(e0.ind.c[1]>e1.ind.c[1])return +1;
   return 0;
}
static void JoinEdge(Memc<MC_EDGE> &medge, Bool grid)
{
   Int      edges    =medge.elms();
   Bool    *edge_is  =Alloc<Bool   >(edges); SetMem(edge_is, 1, edges);
   MC_EDGE *edge_sort=Alloc<MC_EDGE>(edges);
   medge.copyTo(edge_sort); Sort(edge_sort, edges, CompareEdge);
   medge.clear ();
   FREP(edges)if(edge_is[i])
   {
      MC_EDGE &edge  =edge_sort[i];
      UInt     flag  =edge.flag;
      UInt     solid =edge.solid;
      Int      point0=edge.ind.c[0];
      for(Int j=i+1; j<edges; j++)if(edge_is[j])
      {
         MC_EDGE &tedge=edge_sort[j];
         if(tedge.ind.c[0]!=point0)break;
         if(tedge.ind.c[1]==edge.ind.c[1])
         {
            flag |=tedge.flag;
            solid|=tedge.solid;
            edge_is[j]=false;
         }
      }
      if(grid)solid|=SOLID_BL|SOLID_BR;
      if(solid&SOLID_AL)solid&=~SOLID_NAL;
      if(solid&SOLID_AR)solid&=~SOLID_NAR;
      if(solid&SOLID_BL)solid&=~SOLID_NBL;
      if(solid&SOLID_BR)solid&=~SOLID_NBR;
      MC_EDGE &edge_new=medge.New();
      edge_new.flag =flag;
      edge_new.solid=solid;
      edge_new.ind  =edge.ind;
   }
   Free(edge_sort);
   Free(edge_is);
}
/******************************************************************************/
// SET SOLID
/******************************************************************************/
static void LinkPointEdge(Memc<MC_EDGE> &medge, Index &point_edge)
{
   Int *p;

   // link point->edges
   point_edge.create(Csg2D.point.elms());
   FREPA(medge){p=medge[i].ind.c; point_edge.incGroup(p[0]   ); point_edge.incGroup(p[1]   );} point_edge.set();
   FREPA(medge){p=medge[i].ind.c; point_edge.addElm  (p[0], i); point_edge.addElm  (p[1], i);}

   // sort point->edges depending on angles
   FloatIndex *fi=Alloc<FloatIndex>(point_edge.group_elms_max);
   FREPA(Csg2D.point)
   {
      Vec2       &center=Csg2D.point[i].pos;
      IndexGroup &ig    =point_edge.group[i];
      FREPAD(e, ig)
      {
         Int      edge_i=ig[e];
         MC_EDGE &edge  =medge[edge_i];
         Int      pi    =edge.ind.c[0]; if(pi==i)pi=edge.ind.c[1];
         fi[e].f=AngleFast(Csg2D.point[pi].pos-center);
         fi[e].i=edge_i;
      }
      Sort(fi, ig.num);
      FREPAD(e, ig)ig[e]=fi[e].i;
   }
   Free(fi);
}
static UInt SolidNext(MC_EDGE &edge, Int point_ind, Bool type)
{
   Bool solid, left=(edge.ind.c[0]==point_ind);
   if(type==TYPE_A)
   {
      if(left)solid=FlagTest(edge.solid, SOLID_AL);
      else    solid=FlagTest(edge.solid, SOLID_AR);
      return  solid ? (SOLID_AL|SOLID_AR) : (SOLID_NAL|SOLID_NAR);
   }else
   {
      if(left)solid=FlagTest(edge.solid, SOLID_BL);
      else    solid=FlagTest(edge.solid, SOLID_BR);
      return  solid ? (SOLID_BL|SOLID_BR) : (SOLID_NBL|SOLID_NBR);
   }
}
static struct SETSOLID
{
   Bool type;
   UInt solid;
   Memc<MC_EDGE> *medge;
   Index         *point_edge;
}SS;
static void SetSolid(MC_EDGE &e)
{
   e.solid|=SS.solid;
   REPD(p, 2)
   {
      IndexGroup &ig=SS.point_edge->group[e.ind.c[p]];
      REPA(ig)
      {
         MC_EDGE &et=(*SS.medge)[ig[i]];
         if(!SolidT(et.solid, SS.type))SetSolid(et);
      }
   }
}
struct SETSOLIDNODE // point linking to owning edges
{
   Int point_ind, edge_ofs, edge_cur, solid;
};
static void SetSolid(Memc<MC_EDGE> &medge, Bool grid, C MeshBase *mshb_a, C MeshBase *mshb_b)
{
   // link point->edge
   Index point_edge;
   LinkPointEdge(medge, point_edge);

   // set solid
   Memb<SETSOLIDNODE>  nodes(1024);
   SS.     medge=&     medge;
   SS.point_edge=&point_edge;
   FREPD(type, grid ? 1 : 2)
   {
      SS.type=(type!=0);
      FREPA(Csg2D.point) // for each vtx
      {
         IndexGroup &ig=point_edge.group[i];
         REPAD(j, ig)
         {
            MC_EDGE &edge=medge[ig[j]];
            if(SolidT(edge.solid, SS.type)) // find edge that has solid info
            {
               SETSOLIDNODE *node=&nodes.New();
               node->point_ind=i;
               node->edge_ofs =j;
               node->edge_cur =1;
               node->solid    =SolidNext(edge, i, SS.type);
               for(; node; )
               {
                  IndexGroup &ig=point_edge.group[node->point_ind];
                  Bool recursive=false;
                  for(; node->edge_cur<ig.num; ) // counter clockwising update solid
                  {
                     Int edge_num=ig[(node->edge_cur+node->edge_ofs)%ig.num]; node->edge_cur++;
                     MC_EDGE &e=medge[edge_num];
                     if(SolidT(e.solid, SS.type)) // update if solid already set
                     {
                        node->solid=SolidNext(e, node->point_ind, SS.type);
                     }else // if not, do the recursive stepping to the next edges-nodes and filling solid info
                     {
                        e.solid|=node->solid;
                        SETSOLIDNODE *prev=node;
                        if(ig.num>2)node=&nodes.New();
                        node->point_ind=((e.ind.c[0]==prev->point_ind) ? e.ind.c[1] : e.ind.c[0]);
                        node->edge_cur =1;
                        node->solid    =prev->solid;
                        IndexGroup  &ig=point_edge.group[node->point_ind]; REPA(ig)if(edge_num==ig[i]){node->edge_ofs=i; break;}
                        recursive      =true; break;
                     }
                  }
                  if(!recursive)
                  {
                     nodes.removeLast();
                     node=(nodes.elms() ? &nodes.last() : null);
                  }
               }
               break;
            }
         }
      }
      if(!grid && mshb_a && mshb_b)FREPA(medge)
      {
         MC_EDGE &e=medge[i];
         if(!SolidT(e.solid, SS.type))
         {
            Vec2 test_point=Avg(Csg2D.point[e.ind.c[0]].pos, Csg2D.point[e.ind.c[1]].pos);
            SS.solid=SolidTI(SS.type, CutsPointMesh(test_point, (SS.type==TYPE_A) ? *mshb_a : *mshb_b));
            SetSolid(e);
         }
      }
   }

   // free
   nodes.del();
}
/******************************************************************************/
// CREATE MESH
/******************************************************************************/
static void CreateMesh(MeshBase &mshb, Memc<MC_EDGE> &medge, UInt sel)
{
   Memt<Bool> is;
   is.setNum(medge.elms());
   FREPA(medge)
   {
      MC_EDGE &e=medge[i];
      UInt s=0;
      if((e.solid&SOLID_AL && e.solid&SOLID_BL ) || (e.solid&SOLID_AR && e.solid&SOLID_BR ))s|=SEL_AB;
      if((e.solid&SOLID_AL && e.solid&SOLID_NBL) || (e.solid&SOLID_AR && e.solid&SOLID_NBR))s|=SEL_A ;
      if((e.solid&SOLID_BL && e.solid&SOLID_NAL) || (e.solid&SOLID_BR && e.solid&SOLID_NAR))s|=SEL_B ;
      is[i]=((s&sel)!=0);
   }
   mshb.create(0, CountIs(is), 0, 0, EDGE_FLAG);
   Int n=0;
   FREPA(medge)if(is[i])
   {
      MC_EDGE &e=medge[i];
      UInt     l=0, r=0;
      if(e.solid&SOLID_AL && e.solid&SOLID_BL )l|=SEL_AB; if(e.solid&SOLID_AR && e.solid&SOLID_BR )r|=SEL_AB;
      if(e.solid&SOLID_AL && e.solid&SOLID_NBL)l|=SEL_A ; if(e.solid&SOLID_AR && e.solid&SOLID_NBR)r|=SEL_A ;
      if(e.solid&SOLID_BL && e.solid&SOLID_NAL)l|=SEL_B ; if(e.solid&SOLID_BR && e.solid&SOLID_NAR)r|=SEL_B ;
      UInt flag=e.flag;
      if(l&sel)flag|=ETQ_L;
      if(r&sel)flag|=ETQ_R;
      mshb.edge.ind (n)=e.ind;
      mshb.edge.flag(n)=flag;
      n++;
   }
   is.clear().setNumZero(Csg2D.point.elms());
   FREPA(mshb.edge)
   {
      Int *p=mshb.edge.ind(i).c;
      is[p[0]]=1;
      is[p[1]]=1;
   }
   Memt<Int> point_remap;
   SetRemap( point_remap, is, Csg2D.point.elms());
   IndRemap( point_remap, mshb.edge.ind(), mshb.edges());

   mshb.vtx._elms=CountIs(is); mshb.include(VTX_POS);
   n=0; FREPA(Csg2D.point)if(is[i])mshb.vtx.pos(n++).set(Csg2D.point[i].pos, 0);
}
/******************************************************************************/
// CUT & GRID & CSG
/******************************************************************************/
MeshBase& MeshBase::cut2D()
{
   // create
   Csg2D.create(vtxs(), Rect(T));

   // add
   AddMesh(T, Csg2D.edge1, TYPE_A);

   // cut
   CutEdgeEdge(Csg2D.edge1, Csg2D.edge1, Csg2D.edgec);

   // join edges
   JoinEdge(Csg2D.edgec, true);

   // create
   del(); CreateMesh(T, Csg2D.edgec, SEL_A|SEL_AB);

   // free
   Csg2D.del();
   return T;
}
/******************************************************************************/
static void CreateGrid(MeshBase &mesh, MeshBase &src, C Vec2 &scale, C Vec2 &offset, Bool fast)
{
   Vec2 s=Abs(scale);
   MAX(s.x, EPS);
   MAX(s.y, EPS);
   Rect rect=Rect(src)-offset;
   rect.min.x=AlignFloor(rect.min.x, s.x); rect.min.y=AlignFloor(rect.min.y, s.y);
   rect.max.x=AlignCeil (rect.max.x, s.x); rect.max.y=AlignCeil (rect.max.y, s.y);
   Int xs=1+Round(rect.w()/s.x),
       ys=1+Round(rect.h()/s.y);
   mesh.createGrid(xs, ys, fast).scaleMove(Vec(rect.size(), 0), Vec(rect.min+offset, 0));
}
MeshBase& MeshBase::grid2D(C Vec2 &scale, C Vec2 &offset)
{
   // create fast grid
   MeshBase temp; CreateGrid(temp, T, scale, offset, true);

   // create
   Csg2D.create((vtxs()+temp.vtxs())/2, Rect(temp));

   // add
   AddMesh(T   , Csg2D.edge1, TYPE_A);
   AddMesh(temp, Csg2D.edge2, TYPE_B);

   // cut
   CutEdgeEdge(Csg2D.edge1, Csg2D.edge2, Csg2D.edgec);

   // grid up
   Csg2D.edge2.clear(); CreateGrid(temp, T, scale, offset, false);
   AddMesh     (temp, Csg2D.edge2, TYPE_B);
   CutEdgePoint(      Csg2D.edge2, Csg2D.edgec);

   // join edges
   JoinEdge(Csg2D.edgec, true);

   // set solid
   SetSolid(Csg2D.edgec, true, null, null);
 //disp(Csg2D.edgec);

   // create
   del(); CreateMesh(T, Csg2D.edgec, SEL_A|SEL_AB);

   // free
   Csg2D.del();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::csg2D(C MeshBase &mshb, UInt sel)
{
   // create
   Csg2D.create((vtxs()+mshb.vtxs())/2, Rect(T)|Rect(mshb));

   // add
   AddMesh(T   , Csg2D.edge1, TYPE_A);
   AddMesh(mshb, Csg2D.edge2, TYPE_B);

   // cut
   CutEdgeEdge(Csg2D.edge1, Csg2D.edge2, Csg2D.edgec);
   CutEdgeEdge(Csg2D.edge2, Csg2D.edge1, Csg2D.edgec);

   // join edges
   JoinEdge(Csg2D.edgec, false);

   // set solid
   SetSolid(Csg2D.edgec, false, &T, &mshb);
 //disp(Csg2D.edgec);

   // create
   del(); CreateMesh(T, Csg2D.edgec, sel);

   // free
   Csg2D.del();
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
