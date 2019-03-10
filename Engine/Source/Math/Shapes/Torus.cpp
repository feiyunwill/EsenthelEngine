/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Torus& Torus::operator*=(  Flt      f) {pos*=f; R*=f; r*=f; return T;}
Torus& Torus::operator/=(  Flt      f) {pos/=f; R/=f; r/=f; return T;}
Torus& Torus::operator*=(C Matrix3 &m)
{
   pos*=m;
   up *=m; up.normalize(); Flt l=m.maxScale();
   R  *=l;
   r  *=l;
   return T;
}
Torus& Torus::operator*=(C Matrix &m)
{
   pos*=m;
   up *=m.orn(); up.normalize(); Flt l=m.maxScale();
   R  *=l;
   r  *=l;
   return T;
}
/******************************************************************************/
void Torus::draw(C Color &color, Bool fill, VecI2 resolution)C
{
   if(resolution.x<0)resolution.x=12;else if(resolution.x<3)resolution.x=3;
   if(resolution.y<0)resolution.y=12;else if(resolution.y<3)resolution.y=3;
   Matrix3 m; m.setUp(up);
   Vec2    cs;
   Vec     d0=m.x;

   VI.color(color);
   REP(resolution.x)
   {
      CosSin(cs.x, cs.y, i*PI2/resolution.x);
      Vec d1=cs.x*m.x + cs.y*m.z,
          l0=pos + d0*(R+r),
          r0=pos + d1*(R+r);
      REPD(n2, resolution.y)
      {
         CosSin(cs.x, cs.y, n2*PI2/resolution.y);
         Vec l1=pos + d0*R + (cs.x*d0 + cs.y*m.y)*r,
             r1=pos + d1*R + (cs.x*d1 + cs.y*m.y)*r;
         if(fill)
         {
            VI.quad(l0, l1, r1, r0);
         }else
         {
            VI.line(l0, l1);
            VI.line(l0, r0);
         }
         l0=l1;
         r0=r1;
      }
      d0=d1;
   }
   VI.end();
}
/******************************************************************************/
Flt Dist(C Vec &point, C Torus &torus)
{
   Vec delta=point-torus.pos;
   Flt dist_point_plane=DistPointPlane(delta, torus.up);
   Vec nearest=delta-torus.up*dist_point_plane; // PointOnPlane(delta, torus.up);
   Flt dist;
   if(nearest.setLength(torus.R))dist=Dist(delta, nearest);
   else                          dist=Dist(torus.R, dist_point_plane); // point is located exactly on the axis of rotation (pos=torus.pos, dir=torus.up)
   return Max(dist-torus.r, 0);
}
/******************************************************************************/
}
/******************************************************************************/
