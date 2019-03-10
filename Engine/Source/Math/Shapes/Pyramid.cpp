/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Flt Pyramid::area()C
{
   Flt a=side();
   return a*(a + 2*Dist(a*0.5f, h));
}
Flt Pyramid::volume()C
{
   return Sqr(side())*h/3;
}
Pyramid& Pyramid::operator*=(Flt f)
{
   pos*=f;
   h  *=f;
   return T;
}
Pyramid& Pyramid::operator/=(Flt f)
{
   pos/=f;
   h  /=f;
   return T;
}
Pyramid& Pyramid::operator*=(C Matrix3 &m)
{
   OrientP &ornp=T;
   Flt f=m.maxScale();
   ornp*=m; ornp.fix();
   h   *=f;
   return T;
}
Pyramid& Pyramid::operator*=(C Matrix &m)
{
   OrientP &ornp=T;
   Flt f=m.maxScale();
   ornp*=m; ornp.fix();
   h   *=f;
   return T;
}
/******************************************************************************/
void Pyramid::toCorners(Vec (&v)[5])C
{
   Vec x     =cross()*(scale*h),
       y     =perp   *(scale*h),
       z     =dir    * h,
       center=pos    + z;

   v[0]=pos;
   v[1]=center-x+y;
   v[2]=center+x+y;
   v[3]=center+x-y;
   v[4]=center-x-y;
}
/******************************************************************************/
void Pyramid::draw(Color color, Bool fill)C
{
   Vec v[5]; toCorners(v);
   VI.color(color);
   if(fill)
   {
      VI.tri(v[0], v[1], v[2]);
      VI.tri(v[0], v[2], v[3]);
      VI.tri(v[0], v[3], v[4]);
      VI.tri(v[0], v[4], v[1]);
      VI.tri(v[1], v[2], v[4]);
      VI.tri(v[4], v[2], v[3]);
   }else
   {
      VI.line(v[0], v[1]);
      VI.line(v[0], v[2]);
      VI.line(v[0], v[3]);
      VI.line(v[0], v[4]);
      VI.line(v[1], v[2]);
      VI.line(v[2], v[3]);
      VI.line(v[3], v[4]);
      VI.line(v[4], v[1]);
   }
   VI.end();
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Pyramid &pyramid)
{
   Vec d=point-pyramid.pos;
   Flt z=DistPointPlane(d, pyramid.dir);
   if( z>=0 && z<=pyramid.h)
   {
      z*=pyramid.scale;
      if(Abs(DistPointPlane(d, pyramid.perp   ))<=z // check 'perp' first because unlike 'cross', it doesn't require calculations
      && Abs(DistPointPlane(d, pyramid.cross()))<=z)return true;
   }
   return false;
}
Bool Cuts(C VecD &point, C Pyramid &pyramid)
{
   Vec d=point-pyramid.pos; // no need for 'VecD'
   Flt z=DistPointPlane(d, pyramid.dir);
   if( z>=0 && z<=pyramid.h)
   {
      z*=pyramid.scale;
      if(Abs(DistPointPlane(d, pyramid.perp   ))<=z // check 'perp' first because unlike 'cross', it doesn't require calculations
      && Abs(DistPointPlane(d, pyramid.cross()))<=z)return true;
   }
   return false;
}
Bool Cuts(C VecD &point, C PyramidM &pyramid)
{
   Vec d=point-pyramid.pos; // no need for 'VecD'
   Flt z=DistPointPlane(d, pyramid.dir);
   if( z>=0 && z<=pyramid.h)
   {
      z*=pyramid.scale;
      if(Abs(DistPointPlane(d, pyramid.perp   ))<=z // check 'perp' first because unlike 'cross', it doesn't require calculations
      && Abs(DistPointPlane(d, pyramid.cross()))<=z)return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
