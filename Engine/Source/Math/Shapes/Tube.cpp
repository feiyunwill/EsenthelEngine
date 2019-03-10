/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Tube& Tube::operator*=(  Flt  f) {pos*=f; r*=f; h*=f; return T;}
Tube& Tube::operator/=(  Flt  f) {pos/=f; r/=f; h/=f; return T;}
Tube& Tube::operator*=(C Vec &v)
{
   Matrix3 m; m.setUp(up)*=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos*=v;
   return T;
}
Tube& Tube::operator/=(C Vec &v)
{
   Matrix3 m; m.setUp(up)/=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos/=v;
   return T;
}
Tube& Tube::operator*=(C Matrix3 &m)
{
   pos*=m;
   up *=m;
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Tube& Tube::operator*=(C Matrix &m)
{
   pos*=m;
   up *=m .orn      ();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
/******************************************************************************/
void Tube::drawVI(Bool fill, Int resolution)C
{
   if(resolution<0)resolution=24;else if(resolution<3)resolution=3;
   Matrix3 matrix; matrix.setUp(up);
   matrix.x*=r; matrix.y*=h/2;
   matrix.z*=r;
   Vec prev=pos+matrix.x;
   REP(resolution)
   {
      Flt c, s; CosSin(c, s, (PI2*i)/resolution); Vec next=pos + c*matrix.x + s*matrix.z;
      if(fill)VI.quad(next+matrix.y, prev+matrix.y, prev-matrix.y, next-matrix.y);else
      {
         VI.line(prev-matrix.y, prev+matrix.y);
         VI.line(prev+matrix.y, next+matrix.y);
         VI.line(prev-matrix.y, next-matrix.y);
      }
      prev=next;
   }
}
void Tube::draw(C Color &color, Bool fill, Int resolution)C
{
   VI.color(color); drawVI(fill, resolution);
   VI.end  (     );
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Tube &tube)
{
   Flt dist_y=DistPointPlane(point, tube.pos, tube.up);
   if( dist_y>=-tube.h*0.5f
   &&  dist_y<= tube.h*0.5f)return DistPointStr(point, tube.pos, tube.up)<=tube.r;
   return false;
}
Bool Cuts(C Edge &edge, C Tube &tube)
{
   Flt h_2    =tube.h*0.5f,
       dist_y0=DistPointPlane(edge.p[0], tube.pos, tube.up),
       dist_y1=DistPointPlane(edge.p[1], tube.pos, tube.up);

   Edge temp=edge; if(dist_y0>dist_y1){temp.reverse(); Swap(dist_y0, dist_y1);}

   if(dist_y0<= h_2
   && dist_y1>=-h_2)
   {
      if(dist_y0!=dist_y1) // clamp 'temp' to -h_2 .. h_2 range
      {
         if(dist_y0<-h_2)
         {
            temp.p[0]=PointOnPlane(temp.p[0], temp.p[1], dist_y0-(-h_2), dist_y1-(-h_2));
            dist_y0=-h_2;
         }
         if(dist_y1>h_2)
         {
            temp.p[1]=PointOnPlane(temp.p[0], temp.p[1], dist_y0-h_2, dist_y1-h_2);
            dist_y1=h_2;
         }
      }
      return Dist(temp, tube.edge())<=tube.r;
   }

   return false;
}
/******************************************************************************/
Bool SweepPointTube(C Vec &point, C Vec &move, C Tube &tube, Flt *hit_frac, Vec *hit_normal)
{
   Vec pos; Plane plane;
   Flt h_2=tube.h*0.5f;
   plane.pos=tube.pos+h_2*tube.up; plane.normal= tube.up; if(SweepPointPlane(point, move, plane, hit_frac, hit_normal, &pos) && Dist(pos, plane.pos)<=tube.r)return true;
   plane.pos=tube.pos-h_2*tube.up; plane.normal=-tube.up; if(SweepPointPlane(point, move, plane, hit_frac, hit_normal, &pos) && Dist(pos, plane.pos)<=tube.r)return true;
   Matrix matrix; matrix.    setPosDir(tube.pos, tube.up);
   Vec2   point2= matrix.      convert(point   , true);
   Vec2   move2 = matrix.orn().convert(move    , true);
   Vec2   nrm2  ; Flt frac;
   if(SweepPointCircle(point2, move2, Circle(tube.r), &frac, &nrm2))
   {
      Flt y=DistPointPlane(point+move*frac, tube.pos, tube.up);
      if(Abs(y)<=h_2)
      {
         if(hit_frac  )*hit_frac  =frac;
         if(hit_normal)*hit_normal=matrix.orn().convert(nrm2);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
