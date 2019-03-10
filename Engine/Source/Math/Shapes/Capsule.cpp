/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Capsule& Capsule::operator*=(Flt f) {pos*=f; r*=f; h*=f; return T;}
Capsule& Capsule::operator/=(Flt f) {pos/=f; r/=f; h/=f; return T;}

CapsuleM& CapsuleM::operator*=(Flt f) {pos*=f; r*=f; h*=f; return T;}
CapsuleM& CapsuleM::operator/=(Flt f) {pos/=f; r/=f; h/=f; return T;}

Capsule& Capsule::operator*=(C Vec &v)
{
   Matrix3 m; m.setUp(up)*=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos*=v;
   return T;
}
Capsule& Capsule::operator/=(C Vec &v)
{
   Matrix3 m; m.setUp(up)/=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos/=v;
   return T;
}

CapsuleM& CapsuleM::operator*=(C Vec &v)
{
   Matrix3 m; m.setUp(up)*=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos*=v;
   return T;
}
CapsuleM& CapsuleM::operator/=(C Vec &v)
{
   Matrix3 m; m.setUp(up)/=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos/=v;
   return T;
}

Capsule& Capsule::operator*=(C Matrix3 &m)
{
   pos*=m;
   up *=m;
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Capsule& Capsule::operator/=(C Matrix3 &m)
{
   pos/=m;
   up /=m;
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
Capsule& Capsule::operator*=(C Matrix &m)
{
   pos*=m;
   up *=m .orn();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Capsule& Capsule::operator/=(C Matrix &m)
{
   pos/=m;
   up /=m .orn();
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
/******************************************************************************/
Capsule& Capsule::set(Flt r, C Vec &from, C Vec &to)
{
   T.r  =r;
   T.pos=Avg(from, to);
   T.up =to-from;
   T.h  =T.up.normalize()+r*2;
   return T;
}
Capsule& Capsule::setEdge(Flt r, C Vec &from, C Vec &to)
{
   T.r  =r;
   T.pos=Avg(from, to);
   T.up =to-from;
   T.h  =T.up.normalize();
   return T;
}
/******************************************************************************/
Vec Capsule::nearest(C Vec &normal)C
{
   return pos - normal*r - up*( Sign(Dot(up, normal))*(h*0.5f-r) );
}
VecD CapsuleM::nearest(C Vec &normal)C
{
   return pos - normal*r - up*( Sign(Dot(up, normal))*(h*0.5f-r) );
}
/******************************************************************************/
void Capsule::draw(C Color &color, Bool fill, Int resolution)C
{
   if(isBall())Ball(T).draw(color, fill, resolution);else
   {
      if(resolution<0)resolution=24;else if(resolution<3)resolution=3;
      Tube t(r, h-r*2, pos, up); t.draw(color, fill, resolution); t.up*=t.h*0.5f;
      Ball  (r, pos+t.up).drawAngle(color, 0,  PI_2, up, fill, VecI2(resolution, resolution/3));
      Ball  (r, pos-t.up).drawAngle(color, 0, -PI_2, up, fill, VecI2(resolution, resolution/3));
   }
}
/******************************************************************************/
Flt Dist(C Vec &point, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   Vec up=(capsule.h*0.5f-capsule.r)*capsule.up;
   return Max(0, DistPointEdge(point, capsule.pos-up, capsule.pos+up)-capsule.r); // 'DistPointEdge' is safe in case edge is zero length
}
Flt Dist(C Edge &edge, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Max(0, Dist(capsule.edge(), edge)-capsule.r);
}
Flt Dist(C Tri &tri, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Max(0, Dist(capsule.edge(), tri)-capsule.r);
}
Flt Dist(C Box &box, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Max(0, Dist(capsule.edge(), box)-capsule.r);
}
Flt Dist(C OBox &obox, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   Capsule temp=capsule; // temp = capsule in box local space
   temp.pos.divNormalized(obox.matrix      );
   temp.up .divNormalized(obox.matrix.orn());
   return Dist(obox.box, temp);
}
Flt Dist(C Ball &ball, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Max(0, Dist(ball.pos, capsule.edge())-capsule.r-ball.r);
}
Flt Dist(C Capsule &a, C Capsule &b) // safe in case "capsule.isBall()"
{
   return Max(0, Dist(a.edge(), b.edge())-a.r-b.r);
}
Flt DistCapsulePlane(C Capsule &capsule, C Vec &plane, C Vec &normal) // safe in case "capsule.isBall()"
{
   return DistPointPlane(capsule.nearest(normal), plane, normal);
}
Dbl DistCapsulePlane(C Capsule &capsule, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(capsule.nearest(normal), plane, normal);
}
Dbl DistCapsulePlane(C CapsuleM &capsule, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(capsule.nearest(normal), plane, normal);
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   Vec up=(capsule.h*0.5f-capsule.r)*capsule.up;
   return DistPointEdge(point, capsule.pos-up, capsule.pos+up)<=capsule.r;
}
Bool Cuts(C VecD &point, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   Vec up=(capsule.h*0.5f-capsule.r)*capsule.up;
   return DistPointEdge(point, capsule.pos-up, capsule.pos+up)<=capsule.r;
}
Bool Cuts(C VecD &point, C CapsuleM &capsule) // safe in case "capsule.isBall()"
{
   Vec up=(capsule.h*0.5f-capsule.r)*capsule.up;
   return DistPointEdge(point, capsule.pos-up, capsule.pos+up)<=capsule.r;
}
Bool Cuts(C Edge &edge, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Dist(capsule.edge(), edge)<=capsule.r;
}
Bool Cuts(C Tri &tri, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Dist(capsule.edge(), tri)<=capsule.r;
}
Bool Cuts(C Box &box, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Dist(capsule.edge(), box)<=capsule.r;
}
Bool Cuts(C OBox &obox, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   Capsule temp=capsule;
   temp.pos.divNormalized(obox.matrix      );
   temp.up .divNormalized(obox.matrix.orn());
   return Dist(temp.edge(), obox.box)<=temp.r;
}
Bool Cuts(C Ball &ball, C Capsule &capsule) // safe in case "capsule.isBall()"
{
   return Dist(ball.pos, capsule.edge())<=capsule.r+ball.r;
}
Bool Cuts(C Capsule &a, C Capsule &b) // safe in case "capsule.isBall()"
{
   return Dist(a.edge(), b.edge())<=a.r+b.r;
}
/******************************************************************************/
Bool SweepPointCapsule(C Vec &point, C Vec &move, C Capsule &capsule, Flt *hit_frac, Vec *hit_normal) // safe in case "capsule.isBall()"
{
   return SweepBallEdge(Ball(capsule.r, point), move, capsule.edge(), hit_frac, hit_normal);
}
Bool SweepBallCapsule(C Ball &ball, C Vec &move, C Capsule &capsule, Flt *hit_frac, Vec *hit_normal) // safe in case "capsule.isBall()"
{
   return SweepBallEdge(Ball(ball.r+capsule.r, ball.pos), move, capsule.edge(), hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepCapsuleEdge(C Capsule &capsule, C Vec &move, C Edge &edge, Flt *hit_frac, Vec *hit_normal)
{
   Byte check;
   Flt  tube_h_2=capsule.h*0.5f-capsule.r;

   Matrix matrix; matrix.    setPosDir(capsule.pos, capsule.up);
   Edge2  edge2  (matrix.      convert(edge.p[0]  , true), matrix.convert(edge.p[1], true));
   Vec2   move2  =matrix.orn().convert(move       , true);
   Vec2   edge2_d=edge2.p[1]-edge2.p[0];
   Bool   swap=false;
   if(Dot(move2, Perp(edge2_d))>0)
   {
      Swap(edge2.p[0], edge2.p[1]);
      edge2_d.chs();
      swap=true;
   }
   Flt frac; Vec2 normal2; if(!SweepCircleEdge(Circle(capsule.r), move2, edge2, &frac, &normal2))
   {
      check=(DistPointEdge(capsule.pos+capsule.up*tube_h_2, edge.p[0], edge.p[1])<DistPointEdge(capsule.pos-capsule.up*tube_h_2, edge.p[0], edge.p[1]));
   }else
   {
      Byte hitd =false;
      Flt  pos_h=frac*Dot      (move                  , capsule.up),
           e0_h =DistPointPlane(edge.p[0], capsule.pos, capsule.up),
           e1_h =DistPointPlane(edge.p[1], capsule.pos, capsule.up);
      if(swap)Swap(e0_h, e1_h);
      if(Equal(edge2.p[0], edge2.p[1]))
      {
         Flt min_h, max_h; MinMax(e0_h, e1_h, min_h, max_h);
         if(min_h>pos_h+tube_h_2)check=1;else // upper
         if(max_h<pos_h-tube_h_2)check=0;else // lower
            hitd=true;
      }else
      {
         Vec2  hit_point=frac*move2 - capsule.r*normal2;
         Flt  edge_step =Sat(DistPointPlane(hit_point, edge2.p[0], edge2_d)/DistPointPlane(edge2.p[1], edge2.p[0], edge2_d));
         Flt  edge_h    =e0_h + edge_step*(e1_h-e0_h);
         if(Abs(edge_h-pos_h)>tube_h_2)check=(edge_h>pos_h);else
            hitd=true;
      }
      if(hitd)
      {
         if(hit_frac  )*hit_frac  =frac;
         if(hit_normal)*hit_normal=matrix.orn().convert(normal2);
         return true;
      }
   }
   Ball ball(capsule.r, capsule.pos); ball.pos+=capsule.up*(check ? tube_h_2 : -tube_h_2);
   return SweepBallEdge(ball, move, edge, hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepCapsulePlane(C Capsule &capsule, C Vec &move, C Plane &plane, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos) // safe in case "capsule.isBall()"
{
   if(Dot(move, plane.normal)>=0)return false;
   return SweepPointPlane(capsule.nearest(plane.normal), move, plane, hit_frac, hit_normal, hit_pos);
}
Bool SweepCapsuleTri(C Capsule &capsule, C Vec &move, C Tri &tri, Flt *hit_frac, Vec *hit_normal)
{
   if(Dot(move, tri.n)>=0)return false;
   if(SweepPointTri(capsule.nearest(tri.n), move, tri, hit_frac))
   {
      if(hit_normal)*hit_normal=tri.n;
      return true;
   }

   Byte hit=false;
   Flt  f, frac;
   Vec  n, normal;
   REP(3)if(SweepCapsuleEdge(capsule, move, tri.edge(i), &f, &n))if(!hit || f<frac){hit=true; frac=f; normal=n;}
   if(hit)
   {
      if(hit_frac  )*hit_frac  =frac  ;
      if(hit_normal)*hit_normal=normal;
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
