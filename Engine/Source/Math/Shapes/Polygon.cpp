/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// POLY
/******************************************************************************/
Poly::Poly()
{
  _infinite=angle=id=0;
   pos=null;
}
Poly& Poly::del()
{
  _infinite=angle=id=0;
   pos  =null;
   vtx.del();
   return T;
}
Poly& Poly::create(Vec *pos, Int id)
{
   del();

   T.pos=pos;
   T.id =id;

   return T;
}
/******************************************************************************/
Bool Poly::infinite()
{
   if(!_infinite)
   {
      if(!vtx.elms())return false;
      if(!angle     )setAngle();
      Flt angle=0;
      Int num  =0; MFREP(vtx)
      {
         Flt a=vtx[i].angle;
         if( a>EPS && a<PI2-EPS) // vertexes with angle ~0 should be ignored because it's unknown whether they should be set to 0 or PI2
         {
            angle+=a;
            num++;
         }
      }
     _infinite=1+(angle>num*PI);
   }
   return _infinite==2;
}
/******************************************************************************/
Flt Poly::length2D()C
{
   if(vtx.elms()>=2)
   {
      Vec2 *prev  =&pos[vtx[vtx.last()].index].xy;
      Flt   length=0; MFREP(vtx)
      {
         Vec2 *next=&pos[vtx[i].index].xy;
         length+=Dist(*prev, *next); prev=next;
      }
      return length;
   }
   return 0;
}
Flt Poly::length3D()C
{
   if(vtx.elms()>=2)
   {
      Vec *prev  =&pos[vtx[vtx.last()].index];
      Flt  length=0; MFREP(vtx)
      {
         Vec *next=&pos[vtx[i].index];
         length+=Dist(*prev, *next); prev=next;
      }
      return length;
   }
   return 0;
}
/******************************************************************************/
void Poly::setAngle()
{
   if(vtx.elms())
   {
      MemlNode *prev =vtx.first(),
               *cur  =vtx.loopNext(prev);
      Vec2     *prevv=&pos[vtx[prev].index].xy,
               *curv =&pos[vtx[cur ].index].xy;
      REPA(vtx)
      {
         MemlNode  *next =vtx.loopNext(cur);
         Vec2      *nextv=&pos[vtx[next].index].xy;
         vtx[cur] . angle=AngleFull(AngleBetween(*prevv, *curv, *nextv));
         prev=cur ; prevv=curv ;
         cur =next; curv =nextv;
      }
      angle=true;
   }
}
/******************************************************************************/
void Poly::removeVtx(MemlNode *v)
{
   MemlNode *prev=vtx.loopPrev(v),
            *next=vtx.loopNext(v);
   vtx.remove(v);
   if(!vtx.elms())
   {
     _infinite=0;
      angle   =0;
   }else
   if(angle)
   {
      Vtx  &P =    vtx[             prev ],
           &N =    vtx[             next ];
      Vec2 &pp=pos[vtx[vtx.loopPrev(prev)].index].xy,
           &nn=pos[vtx[vtx.loopNext(next)].index].xy,
           &p =pos[                      P.index].xy,
           &n =pos[                      N.index].xy;
      P.angle=AngleFull(AngleBetween(pp, p, n));
      N.angle=AngleFull(AngleBetween(p, n, nn));
   }
}
/******************************************************************************/
void Poly::link(Poly &poly, MemlNode *i, MemlNode *pi)
{
   // T0..i, pi..p1, p0..pi, i..T1
   Poly temp; temp.create(pos, id);
   MemlNode *c;
   for(c=   T.vtx.first(); c; c=c->next()){temp.addVtx(   T.vtx[c].index); if(c== i)break;}
   for(c=pi              ; c; c=c->next()){temp.addVtx(poly.vtx[c].index);                }
   for(c=poly.vtx.first(); c; c=c->next()){temp.addVtx(poly.vtx[c].index); if(c==pi)break;}
   for(c= i              ; c; c=c->next()){temp.addVtx(   T.vtx[c].index);                }

   if(angle || poly.angle)temp.setAngle();
   Swap(T, temp);
}
/******************************************************************************/
void Poly::draw2D(C Color &color)
{
   MemlNode *cur=vtx.first();
   REPA(vtx)
   {
      MemlNode *next=vtx.loopNext(cur);
      D.line(color, pos[vtx[cur].index].xy, pos[vtx[next].index].xy);
      D.dot (color, pos[vtx[cur].index].xy);
      cur=next;
   }
}
void Poly::draw3D(C Color &color)
{
   MemlNode *cur=vtx.first();
   REPA(vtx)
   {
      MemlNode *next=vtx.loopNext(cur);
      D.line(color, pos[vtx[cur].index], pos[vtx[next].index]);
      D.dot (color, pos[vtx[cur].index]);
      cur=next;
   }
}
/******************************************************************************/
// MAIN
/******************************************************************************/
static Int CompareAngle(C Vec4 &a, C Vec4 &b)
{
   if(Int c=Compare(b.z, a.z))return c; // compare 'b' against 'a' so generated poly will be in clockwise order
   return   Compare(a.w, b.w);
}
static Int CompareAngle(C VecD4 &a, C VecD4 &b)
{
   if(Int c=Compare(b.z, a.z))return c; // compare 'b' against 'a' so generated poly will be in clockwise order
   return   Compare(a.w, b.w);
}
static Flt Dist(C Vec2 &p1, C Vec2 &p2, C Vec2 &p3)
{
   return (p2.x-p1.x)*(p3.y-p1.y) - (p2.y-p1.y)*(p3.x-p1.x); // return DistPointPlane(p2, p1, Perp(p3-p1));
}
static Dbl Dist(C VecD2 &p1, C VecD2 &p2, C VecD2 &p3)
{
   return (p2.x-p1.x)*(p3.y-p1.y) - (p2.y-p1.y)*(p3.x-p1.x); // return DistPointPlane(p2, p1, Perp(p3-p1));
}
/******************************************************************************/
void CreateConvex2D(MemPtr<Vec2> poly, C Vec2 *point, Int points)
{
   poly.clear();
   if(points>=1)
   {
      if(points==1)poly.add(point[0]);else
      {
         // find the most to the left bottom
         Int  min_index=points-1;
         Vec2 min_value=point[min_index];
         REP(points-1){C Vec2 &p=point[i]; if(p.x<min_value.x || p.x==min_value.x && p.y<min_value.y){min_value=p; min_index=i;}} // if same X then compare Y to avoid having last point on the same line as #0 #1 that is useless

         // sort by angle
         Memt<Vec4> temp; temp.setNum(points);
       C Vec2 &start=point[min_index];
         REPA(temp)
         {
            // encode angle in 'z'
          C Vec2 &p=point[i];
            Vec2  d=p-start;
            temp[i].set(p, d.any() ? d.y/d.x : 1/d.x, Abs(d.y)+Abs(d.x)); // Warning: here we assume that +-Inf is obtained when dividing by d.x==0, first sort by 'z' (angle), then sort by 'w' (distance), if "!d.any()" which means both XY are zero and this is the starting point, then use 1/x to get INF instead of NAN which would break or even crash (stack overflow when sorting), use 'any' instead of comparing 'min_index' because there could be other points with the same coordinates
         }
         temp.sort(CompareAngle);

         // create poly
         poly.add(temp[0].xy);
         poly.add(temp[1].xy);
         for(Int i=2; i<temp.elms(); i++)
         {
          C Vec2 &p=temp[i].xy;
            for(; poly.elms()>=2 && Dist(p, poly.last(), poly[poly.elms()-2])<=0; )poly.removeLast();
            poly.add(p);
         }
      }
   }
}
void CreateConvex2D(MemPtr<VecD2> poly, C VecD2 *point, Int points)
{
   poly.clear();
   if(points>=1)
   {
      if(points==1)poly.add(point[0]);else
      {
         // find the most to the left bottom
         Int   min_index=points-1;
         VecD2 min_value=point[min_index];
         REP(points-1){C VecD2 &p=point[i]; if(p.x<min_value.x || p.x==min_value.x && p.y<min_value.y){min_value=p; min_index=i;}} // if same X then compare Y to avoid having last point on the same line as #0 #1 that is useless

         // sort by angle
         Memt<VecD4> temp; temp.setNum(points);
       C VecD2 &start=point[min_index];
         REPA(temp)
         {
            // encode angle in 'z'
          C VecD2 &p=point[i];
            VecD2  d=p-start;
            temp[i].set(p, d.any() ? d.y/d.x : 1/d.x, Abs(d.y)+Abs(d.x)); // Warning: here we assume that +-Inf is obtained when dividing by d.x==0, first sort by 'z' (angle), then sort by 'w' (distance), if "!d.any()" which means both XY are zero and this is the starting point, then use 1/x to get INF instead of NAN which would break or even crash (stack overflow when sorting), use 'any' instead of comparing 'min_index' because there could be other points with the same coordinates
         }
         temp.sort(CompareAngle);

         // create poly
         poly.add(temp[0].xy);
         poly.add(temp[1].xy);
         for(Int i=2; i<temp.elms(); i++)
         {
          C VecD2 &p=temp[i].xy;
            for(; poly.elms()>=2 && Dist(p, poly.last(), poly[poly.elms()-2])<=0; )poly.removeLast();
            poly.add(p);
         }
      }
   }
}
/******************************************************************************/
void CreateConvex2Dxz(MemPtr<Vec2> poly, C Vec *point, Int points)
{
   poly.clear();
   if(points>=1)
   {
      if(points==1)poly.add(point[0].xz());else
      {
         // find the most to the left bottom
         Int  min_index=points-1;
         Vec2 min_value=point[min_index].xz();
         REP(points-1){C Vec &p=point[i]; if(p.x<min_value.x || p.x==min_value.x && p.z<min_value.y){min_value=p.xz(); min_index=i;}} // if same X then compare Y to avoid having last point on the same line as #0 #1 that is useless

         // sort by angle
         Memt<Vec4> temp; temp.setNum(points);
         Vec2 start=point[min_index].xz();
         REPA(temp)
         {
            // encode angle in 'z'
            Vec2 p=point[i].xz(), d=p-start;
            temp[i].set(p, d.any() ? d.y/d.x : 1/d.x, Abs(d.y)+Abs(d.x)); // Warning: here we assume that +-Inf is obtained when dividing by d.x==0, first sort by 'z' (angle), then sort by 'w' (distance), if "!d.any()" which means both XY are zero and this is the starting point, then use 1/x to get INF instead of NAN which would break or even crash (stack overflow when sorting), use 'any' instead of comparing 'min_index' because there could be other points with the same coordinates
         }
         temp.sort(CompareAngle);

         // create poly
         poly.add(temp[0].xy);
         poly.add(temp[1].xy);
         for(Int i=2; i<temp.elms(); i++)
         {
          C Vec2 &p=temp[i].xy;
            for(; poly.elms()>=2 && Dist(p, poly.last(), poly[poly.elms()-2])<=0; )poly.removeLast();
            poly.add(p);
         }
      }
   }
}
void CreateConvex2Dxz(MemPtr<VecD2> poly, C VecD *point, Int points)
{
   poly.clear();
   if(points>=1)
   {
      if(points==1)poly.add(point[0].xz());else
      {
         // find the most to the left bottom
         Int   min_index=points-1;
         VecD2 min_value=point[min_index].xz();
         REP(points-1){C VecD &p=point[i]; if(p.x<min_value.x || p.x==min_value.x && p.z<min_value.y){min_value=p.xz(); min_index=i;}} // if same X then compare Y to avoid having last point on the same line as #0 #1 that is useless

         // sort by angle
         Memt<VecD4> temp; temp.setNum(points);
         VecD2 start=point[min_index].xz();
         REPA(temp)
         {
            // encode angle in 'z'
            VecD2 p=point[i].xz(), d=p-start;
            temp[i].set(p, d.any() ? d.y/d.x : 1/d.x, Abs(d.y)+Abs(d.x)); // Warning: here we assume that +-Inf is obtained when dividing by d.x==0, first sort by 'z' (angle), then sort by 'w' (distance), if "!d.any()" which means both XY are zero and this is the starting point, then use 1/x to get INF instead of NAN which would break or even crash (stack overflow when sorting), use 'any' instead of comparing 'min_index' because there could be other points with the same coordinates
         }
         temp.sort(CompareAngle);

         // create poly
         poly.add(temp[0].xy);
         poly.add(temp[1].xy);
         for(Int i=2; i<temp.elms(); i++)
         {
          C VecD2 &p=temp[i].xy;
            for(; poly.elms()>=2 && Dist(p, poly.last(), poly[poly.elms()-2])<=0; )poly.removeLast();
            poly.add(p);
         }
      }
   }
}
/******************************************************************************/
void Triangulate(C MemPtr<Vec> &poly, MeshBase &mesh, Bool convex)
{
   if(poly.elms()>=3)
   {
      mesh.create(poly.elms(), 0, poly.elms()-2, 0);
      Vec *pos=mesh.vtx.pos();

      if(poly.elms()==3)
      {
         pos[0]=poly[0];
         pos[1]=poly[1];
         pos[2]=poly[2];
         mesh.tri.ind(0).set(0, 1, 2);
      }else
      {
         // triangulate
         Memt<Int> temp; temp.setNum(poly.elms());
         REPA(poly)
         {
            temp[i]=i;
            pos [i]=poly[i];
         }

         if(convex)
         {
            REP(temp.elms()-2)
            {
               mesh.tri.ind(i).set(temp[i], temp[i+1], temp[i+2]);
               temp.remove(i+1, true);
            }
         }else
         {
            // validate polygon normal
            Vec nrm, *normal=null;
            if(!normal)
            {
               nrm.zero(); REPA(poly)nrm+=GetNormalEdge(poly[i], poly[(i+1)%poly.elms()]); // get average face normal
               normal=&nrm;
            }

            for(mesh.tri._elms=0; temp.elms()>=3; )
            {
               Bool added=false;
               REP(temp.elms()-2)
               {
                  Tri tri(pos[temp[i]], pos[temp[i+1]], pos[temp[i+2]]);
                  if(Dot(*normal, tri.n)>0)
                  {
                     Vec cross[3]={Cross(tri.n, tri.p[0]-tri.p[1]), Cross(tri.n, tri.p[1]-tri.p[2]), Cross(tri.n, tri.p[2]-tri.p[0])};
                     REPAD(t, temp)if(t<i || t>i+2)if(Cuts(pos[temp[t]], tri, cross))goto cuts;
                     {
                        Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]);
                        temp.remove(i+1, true);
                        added=true;
                     }
                  cuts:;
                  }
               }
               if(!added)break;
            }
         }
      }
   }else
   {
      mesh.del();
   }
}
void Triangulate(C MemPtr<VtxFull> &poly, MeshBase &mesh, UInt flag_and, Bool convex)
{
   if(poly.elms()>=3)
   {
      mesh.create(poly.elms(), 0, poly.elms()-2, 0, flag_and&~(VTX_FLAG|VTX_DUP));
      Vec *pos=mesh.vtx.pos();

      if(poly.elms()==3)
      {
         poly[0].to(mesh, 0);
         poly[1].to(mesh, 1);
         poly[2].to(mesh, 2);
         mesh.tri.ind(0).set(0, 1, 2);
      }else
      {
         // triangulate
         Memt<Int> temp; temp.setNum(poly.elms()); 
         REPA(poly)
         {
            temp[i]=i;
            poly[i].to(mesh, i);
         }

         if(convex)
         {
            REP(temp.elms()-2)
            {
               mesh.tri.ind(i).set(temp[i], temp[i+1], temp[i+2]);
               temp.remove(i+1, true);
            }
         }else
         {
            // validate polygon normal
            Vec nrm, *normal=null;
            if(!normal)
            {
               nrm.zero(); REPA(poly)nrm+=GetNormalEdge(poly[i].pos, poly[(i+1)%poly.elms()].pos); // get average face normal
               normal=&nrm;
            }

            for(mesh.tri._elms=0; temp.elms()>=3; )
            {
               Bool added=false;
               REP(temp.elms()-2)
               {
                  Tri tri(pos[temp[i]], pos[temp[i+1]], pos[temp[i+2]]);
                  if(Dot(*normal, tri.n)>0)
                  {
                     Vec cross[3]={Cross(tri.n, tri.p[0]-tri.p[1]), Cross(tri.n, tri.p[1]-tri.p[2]), Cross(tri.n, tri.p[2]-tri.p[0])};
                     REPAD(t, temp)if(t<i || t>i+2)if(Cuts(pos[temp[t]], tri, cross))goto cuts;
                     {
                        Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]);
                        temp.remove(i+1, true);
                        added=true;
                     }
                  cuts:;
                  }
               }
               if(!added)break;
            }
         }
      }
   }else mesh.del();
}
void Triangulate(C MemPtr< Memc<Vec> > &polys, MeshBase &mesh, Flt weld_pos_eps, Bool convex, C Byte *poly_flags)
{
   // count number of vertexes and triangles
   Int vtxs=0, max_vtxs=0,
       tris=0;
   REPA(polys)
   {
    C Memc<Vec> &poly=polys[i];
      if(poly.elms()>=3)
      {
         MAX(max_vtxs, poly.elms());
         vtxs+=poly.elms();
         tris+=poly.elms()-2;
      }
   }

   // build mesh
   Memt<Int> temp;
   mesh.create(vtxs, 0, tris, 0, poly_flags ? FACE_FLAG : 0); mesh.tri._elms=0; Vec *pos=mesh.vtx.pos();
   vtxs=0;
   FREPA(polys)
   {
    C Memc<Vec> &poly=polys[i];
      if(poly.elms()>=3)
      {
         Byte flag=(poly_flags ? poly_flags[i] : 0);
         if(poly.elms()==3)
         {
            pos[vtxs+0]=poly[0];
            pos[vtxs+1]=poly[1];
            pos[vtxs+2]=poly[2];
            Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(vtxs, vtxs+1, vtxs+2); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
            vtxs+=3;
         }else
         if(convex)
         {
            FREP(poly.elms()  )pos[temp.New()=vtxs+i]=poly[i]; // order is important
             REP(temp.elms()-2)
            {
               Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
               temp.remove(i+1, true);
            }
            temp.clear();
            vtxs+=poly.elms();
         }else
         {
            Vec nrm(0);
            FREPA(poly) // order is important
            {
               nrm+=GetNormalEdge(poly[i], poly[(i+1)%poly.elms()]); // get average face normal
               pos[temp.New()=vtxs+i]=poly[i];
            }
            for(; temp.elms()>=3; )
            {
               Bool added=false;
               REP(temp.elms()-2)
               {
                  Tri tri(pos[temp[i]], pos[temp[i+1]], pos[temp[i+2]]);
                  if(Dot(nrm, tri.n)>0)
                  {
                     Vec cross[3]={Cross(tri.n, tri.p[0]-tri.p[1]), Cross(tri.n, tri.p[1]-tri.p[2]), Cross(tri.n, tri.p[2]-tri.p[0])};
                     REPAD(t, temp)if(t<i || t>i+2)if(Cuts(pos[temp[t]], tri, cross))goto cuts;
                     {
                        Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
                        temp.remove(i+1, true);
                        added=true;
                     }
                  cuts:;
                  }
               }
               if(!added)break;
            }
            temp.clear();
            vtxs+=poly.elms();
         }
      }
   }
   mesh.weldVtx(VTX_ALL, weld_pos_eps);
}
void Triangulate(C MemPtr< Memc<VtxFull> > &polys, MeshBase &mesh, UInt flag_and, Flt weld_pos_eps, Bool convex, C Byte *poly_flags)
{
   // count number of vertexes and triangles
   Int vtxs=0, max_vtxs=0,
       tris=0;
   REPA(polys)
   {
    C Memc<VtxFull> &poly=polys[i];
      if(poly.elms()>=3)
      {
         MAX(max_vtxs, poly.elms());
         vtxs+=poly.elms();
         tris+=poly.elms()-2;
      }
   }

   // build mesh
   Memt<Int> temp;
   mesh.create(vtxs, 0, tris, 0, flag_and & ((VTX_ALL&~(VTX_FLAG|VTX_DUP)) | (poly_flags ? FACE_FLAG : 0))); mesh.tri._elms=0; Vec *pos=mesh.vtx.pos();
   vtxs=0;
   FREPA(polys)
   {
    C Memc<VtxFull> &poly=polys[i];
      if(poly.elms()>=3)
      {
         Byte flag=(poly_flags ? poly_flags[i] : 0);
         if(poly.elms()==3)
         {
            poly[0].to(mesh, vtxs+0);
            poly[1].to(mesh, vtxs+1);
            poly[2].to(mesh, vtxs+2);
            Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(vtxs, vtxs+1, vtxs+2); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
            vtxs+=3;
         }else
         if(convex)
         {
            FREP(poly.elms()  )poly[i].to(mesh, temp.New()=vtxs+i); // order is important
             REP(temp.elms()-2)
            {
               Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
               temp.remove(i+1, true);
            }
            temp.clear();
            vtxs+=poly.elms();
         }else
         {
            Vec nrm(0);
            FREPA(poly) // order is important
            {
               nrm+=GetNormalEdge(poly[i].pos, poly[(i+1)%poly.elms()].pos); // get average face normal
               poly[i].to(mesh, temp.New()=vtxs+i);
            }
            for(; temp.elms()>=3; )
            {
               Bool added=false;
               REP(temp.elms()-2)
               {
                  Tri tri(pos[temp[i]], pos[temp[i+1]], pos[temp[i+2]]);
                  if(Dot(nrm, tri.n)>0)
                  {
                     Vec cross[3]={Cross(tri.n, tri.p[0]-tri.p[1]), Cross(tri.n, tri.p[1]-tri.p[2]), Cross(tri.n, tri.p[2]-tri.p[0])};
                     REPAD(t, temp)if(t<i || t>i+2)if(Cuts(pos[temp[t]], tri, cross))goto cuts;
                     {
                        Int t=mesh.tri.elms(); mesh.tri._elms++; mesh.tri.ind(t).set(temp[i], temp[i+1], temp[i+2]); if(mesh.tri.flag())mesh.tri.flag(t)=flag;
                        temp.remove(i+1, true);
                        added=true;
                     }
                  cuts:;
                  }
               }
               if(!added)break;
            }
            temp.clear();
            vtxs+=poly.elms();
         }
      }
   }
   mesh.weldVtx(VTX_ALL, weld_pos_eps);
}
/******************************************************************************/
void ClipPoly(C MemPtr<Vec> &poly, C Plane &plane, MemPtr<Vec> output)
{
   output.clear();

   switch(poly.elms())
   {
      case 0: break;

      case 1:
         if(Dist(poly[0], plane)<0)output.add(poly[0]);
      break;

      default: // >=2
      {
         Vec prev     =poly.last();
         Flt prev_dist=Dist(prev, plane);
         FREPA(poly) // preserve order
         {
          C Vec &next     =poly[i];
            Flt  next_dist=Dist(next, plane);
            if(    prev_dist<0)
            {
               output.add(prev);
               if(next_dist>=0) // change of side
               {
                  output.add(Lerp(prev, next, prev_dist/(prev_dist-next_dist))); // -prev_dist/(next_dist-prev_dist)
               }
            }
            else // prev_dist>=0
            {
               if(next_dist<0) // change of side
               {
                  output.add(Lerp(prev, next, prev_dist/(prev_dist-next_dist)));
               }
            }
            prev     =next;
            prev_dist=next_dist;
         }
      }break;
   }
}
void ClipPoly(C MemPtr<VtxFull> &poly, C Plane &plane, MemPtr<VtxFull> output)
{
   output.clear();

   switch(poly.elms())
   {
      case 0: break;

      case 1:
         if(Dist(poly[0].pos, plane)<0)output.add(poly[0]);
      break;

      default: // >=2
      {
       C VtxFull *prev     =&poly.last();
         Flt      prev_dist=Dist(prev->pos, plane);
         FREPA(poly) // preserve order
         {
          C VtxFull &next     =poly[i];
            Flt      next_dist=Dist(next.pos, plane);
            if(      prev_dist<0)
            {
               output.add(*prev);
               if(next_dist>=0) // change of side
               {
                  output.New().lerp(*prev, next, prev_dist/(prev_dist-next_dist)); // -prev_dist/(next_dist-prev_dist)
               }
            }
            else // prev_dist>=0
            {
               if(next_dist<0) // change of side
               {
                  output.New().lerp(*prev, next, prev_dist/(prev_dist-next_dist));
               }
            }
            prev     =&next;
            prev_dist= next_dist;
         }
      }break;
   }
}
/******************************************************************************/
void SplitPoly(C MemPtr<Vec> &poly, C Plane &plane, MemPtr<Vec> output_positive, MemPtr<Vec> output_negative)
{
   output_positive.clear();
   output_negative.clear();

   switch(poly.elms())
   {
      case 0: break;

      case 1:
         if(Dist(poly[0], plane)<0)output_negative.add(poly[0]);
         else                      output_positive.add(poly[0]);
      break;

      default: // >=2
      {
         Vec prev     =poly.last();
         Flt prev_dist=Dist(prev, plane);
         FREPA(poly) // preserve order
         {
            Bool have_mid =false;
          C Vec &next     =poly[i]; Vec mid;
            Flt  next_dist=Dist(next, plane);

            // negative
            if(prev_dist<0)
            {
               output_negative.add(prev);
               if(next_dist>=0) // change of side
               {
                  if(!have_mid){have_mid=true; mid=Lerp(prev, next, prev_dist/(prev_dist-next_dist));} // -prev_dist/(next_dist-prev_dist)
                  output_negative.add(mid);
               }
            }
            else // prev_dist>=0
            {
               if(next_dist<0) // change of side
               {
                  if(!have_mid){have_mid=true; mid=Lerp(prev, next, prev_dist/(prev_dist-next_dist));}
                  output_negative.add(mid);
               }
            }

            // positive
            if(prev_dist>0)
            {
               output_positive.add(prev);
               if(next_dist<=0) // change of side
               {
                  if(!have_mid){have_mid=true; mid=Lerp(prev, next, prev_dist/(prev_dist-next_dist));} // -prev_dist/(next_dist-prev_dist)
                  output_positive.add(mid);
               }
            }
            else // prev_dist<=0
            {
               if(next_dist>0) // change of side
               {
                  if(!have_mid){have_mid=true; mid=Lerp(prev, next, prev_dist/(prev_dist-next_dist));}
                  output_positive.add(mid);
               }
            }

            prev     =next;
            prev_dist=next_dist;
         }
      }break;
   }
}
void SplitPoly(C MemPtr<VtxFull> &poly, C Plane &plane, MemPtr<VtxFull> output_positive, MemPtr<VtxFull> output_negative)
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

            prev     =&next;
            prev_dist= next_dist;
         }
      }break;
   }
}
/******************************************************************************/
void DrawPoly2D(C MemPtr<Vec> &poly, C Color &edge_color, C Color &vtx_color)
{
   if(poly.elms())
   {
      if(edge_color.a)
      {
         VI.color(edge_color);
         Vec2 prev=poly.first().xy;
         REPA(poly)
         {
          C Vec2 &next=poly[i].xy; VI.line(prev, next);
                  prev=next;
         }
         VI.end();
      }
      if(vtx_color.a)
      {
                   VI.color(vtx_color);
         REPA(poly)VI.dot  (poly[i].xy);
                   VI.end  ();
      }
   }
}
void DrawPoly(C MemPtr<Vec> &poly, C Color &edge_color, C Color &vtx_color)
{
   if(poly.elms())
   {
      if(edge_color.a)
      {
         VI.color(edge_color);
         Vec prev=poly.first();
         REPA(poly)
         {
          C Vec &next=poly[i]; VI.line(prev, next);
                 prev=next;
         }
         VI.end();
      }
      if(vtx_color.a)
      {
                   VI.color(vtx_color);
         REPA(poly)VI.dot  (poly[i]);
                   VI.end  ();
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
