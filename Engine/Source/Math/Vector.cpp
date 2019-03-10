/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   EPS_COS and EPSD_COS were calculated using:
      Matrix3 mf; MatrixD3 md; Dbl df=2, dd=2; REP(100000000)
      {
         mf.setRotateZ(Random.f(PI2)).rotateXY(Random.f(PI2), Random.f(PI2));
         MIN(df, mf.x.length2()); // same as Dot(mf.x, mf.x)
         MIN(df, mf.y.length2());
         MIN(df, mf.z.length2());

         md.setRotateZ(Random.f(PI2)).rotateXY(Random.f(PI2), Random.f(PI2));
         MIN(dd, md.x.length2()); // same as Dot(md.x, md.x)
         MIN(dd, md.y.length2());
         MIN(dd, md.z.length2());
      }
      Exit(S+df+' '+dd);
      df=0.999999642;
      dd=0.99999999999999933; (obtained with VS debugger and not EE)

/******************************************************************************/
const Vec VecDir[DIR_NUM]=
{
   Vec( 1, 0, 0),
   Vec(-1, 0, 0),
   Vec( 0, 1, 0),
   Vec( 0,-1, 0),
   Vec( 0, 0, 1),
   Vec( 0, 0,-1),
};
const Vec2 Vec2Zero(0);
const Vec  VecZero (0), VecOne(1);
const Vec4 Vec4Zero(0);
/******************************************************************************/
Int MinI(Int x, Int y              ) {return y<x;}
Int MinI(Flt x, Flt y              ) {return y<x;}
Int MinI(Dbl x, Dbl y              ) {return y<x;}
Int MaxI(Int x, Int y              ) {return y>x;}
Int MaxI(Flt x, Flt y              ) {return y>x;}
Int MaxI(Dbl x, Dbl y              ) {return y>x;}
Int MinI(Int x, Int y, Int z       ) {return (x<=z) ? ((x<=y)?0:1) : ((y<=z)?1:2);}
Int MinI(Flt x, Flt y, Flt z       ) {return (x<=z) ? ((x<=y)?0:1) : ((y<=z)?1:2);}
Int MinI(Dbl x, Dbl y, Dbl z       ) {return (x<=z) ? ((x<=y)?0:1) : ((y<=z)?1:2);}
Int MaxI(Int x, Int y, Int z       ) {return (x>=z) ? ((x>=y)?0:1) : ((y>=z)?1:2);}
Int MaxI(Flt x, Flt y, Flt z       ) {return (x>=z) ? ((x>=y)?0:1) : ((y>=z)?1:2);}
Int MaxI(Dbl x, Dbl y, Dbl z       ) {return (x>=z) ? ((x>=y)?0:1) : ((y>=z)?1:2);}
Int MinI(Int x, Int y, Int z, Int w) {return (x<=w) ? ((x<=z)?((x<=y)?0:1):((y<=z)?1:2)) : ((y<=w)?((y<=z)?1:2):((z<=w)?2:3));}
Int MinI(Flt x, Flt y, Flt z, Flt w) {return (x<=w) ? ((x<=z)?((x<=y)?0:1):((y<=z)?1:2)) : ((y<=w)?((y<=z)?1:2):((z<=w)?2:3));}
Int MinI(Dbl x, Dbl y, Dbl z, Dbl w) {return (x<=w) ? ((x<=z)?((x<=y)?0:1):((y<=z)?1:2)) : ((y<=w)?((y<=z)?1:2):((z<=w)?2:3));}
Int MaxI(Int x, Int y, Int z, Int w) {return (x>=w) ? ((x>=z)?((x>=y)?0:1):((y>=z)?1:2)) : ((y>=w)?((y>=z)?1:2):((z>=w)?2:3));}
Int MaxI(Flt x, Flt y, Flt z, Flt w) {return (x>=w) ? ((x>=z)?((x>=y)?0:1):((y>=z)?1:2)) : ((y>=w)?((y>=z)?1:2):((z>=w)?2:3));}
Int MaxI(Dbl x, Dbl y, Dbl z, Dbl w) {return (x>=w) ? ((x>=z)?((x>=y)?0:1):((y>=z)?1:2)) : ((y>=w)?((y>=z)?1:2):((z>=w)?2:3));}
/******************************************************************************/
VecB4 AvgI(C VecB4 &a, C VecB4 &b)
{
   return VecB4(AvgI(a.x, b.x),
                AvgI(a.y, b.y),
                AvgI(a.z, b.z),
                AvgI(a.w, b.w));
}
VecB4 AvgI(C VecB4 &a, C VecB4 &b, C VecB4 &c)
{
   return VecB4(AvgI(a.x, b.x, c.x),
                AvgI(a.y, b.y, c.y),
                AvgI(a.z, b.z, c.z),
                AvgI(a.w, b.w, c.w));
}
VecB4 AvgI(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d)
{
   return VecB4(AvgI(a.x, b.x, c.x, d.x),
                AvgI(a.y, b.y, c.y, d.y),
                AvgI(a.z, b.z, c.z, d.z),
                AvgI(a.w, b.w, c.w, d.w));
}
/******************************************************************************/
VecI2 AvgI(C VecI2 &a, C VecI2 &b)
{
   return VecI2(AvgI(a.x, b.x),
                AvgI(a.y, b.y));
}
VecI AvgI(C VecI &a, C VecI &b)
{
   return VecI(AvgI(a.x, b.x),
               AvgI(a.y, b.y),
               AvgI(a.z, b.z));
}
VecI4 AvgI(C VecI4 &a, C VecI4 &b)
{
   return VecI4(AvgI(a.x, b.x),
                AvgI(a.y, b.y),
                AvgI(a.z, b.z),
                AvgI(a.w, b.w));
}
/******************************************************************************/
Vec2 AvgF(C VecI2 &a, C VecI2 &b) {return Avg(Vec2(a), Vec2(b));}
Vec  AvgF(C VecI  &a, C VecI  &b) {return Avg(Vec (a), Vec (b));}
Vec4 AvgF(C VecI4 &a, C VecI4 &b) {return Avg(Vec4(a), Vec4(b));}
/******************************************************************************/
Flt Dist (C Vec2  &a, C Vec2  &b) {return Dist (a.x-b.x, a.y-b.y);}
Flt Dist (C VecI2 &a, C Vec2  &b) {return Dist (a.x-b.x, a.y-b.y);}
Flt Dist (C Vec2  &a, C VecI2 &b) {return Dist (a.x-b.x, a.y-b.y);}
Dbl Dist (C VecD2 &a, C VecD2 &b) {return Dist (a.x-b.x, a.y-b.y);}
Flt Dist (C VecI2 &a, C VecI2 &b) {return Dist (a.x-b.x, a.y-b.y);}
Flt Dist2(C Vec2  &a, C Vec2  &b) {return Dist2(a.x-b.x, a.y-b.y);}
Flt Dist2(C VecI2 &a, C Vec2  &b) {return Dist2(a.x-b.x, a.y-b.y);}
Flt Dist2(C Vec2  &a, C VecI2 &b) {return Dist2(a.x-b.x, a.y-b.y);}
Dbl Dist2(C VecD2 &a, C VecD2 &b) {return Dist2(a.x-b.x, a.y-b.y);}
Int Dist2(C VecI2 &a, C VecI2 &b) {return Dist2(a.x-b.x, a.y-b.y);}
Flt Dist (C Vec   &a, C Vec   &b) {return Dist (a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist (C VecD  &a, C Vec   &b) {return Dist (a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist (C Vec   &a, C VecD  &b) {return Dist (a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist (C VecD  &a, C VecD  &b) {return Dist (a.x-b.x, a.y-b.y, a.z-b.z);}
Flt Dist (C VecI  &a, C VecI  &b) {return Dist (a.x-b.x, a.y-b.y, a.z-b.z);}
Flt Dist2(C Vec   &a, C Vec   &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist2(C VecD  &a, C Vec   &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist2(C Vec   &a, C VecD  &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z);}
Dbl Dist2(C VecD  &a, C VecD  &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z);}
Int Dist2(C VecI  &a, C VecI  &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z);}
Flt Dist2(C Vec4  &a, C Vec4  &b) {return Dist2(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}

Flt Cross(C Vec2  &a, C Vec2  &b) {return a.x*b.y - a.y*b.x;}
Dbl Cross(C VecD2 &a, C VecD2 &b) {return a.x*b.y - a.y*b.x;}

Vec Cross(C Vec &a, C Vec &b)
{
   return Vec(a.y*b.z - a.z*b.y,
              a.z*b.x - a.x*b.z,
              a.x*b.y - a.y*b.x);
}
VecD Cross(C VecD &a, C VecD &b)
{
   return VecD(a.y*b.z - a.z*b.y,
               a.z*b.x - a.x*b.z,
               a.x*b.y - a.y*b.x);
}
Vec  CrossN(C Vec  &a, C Vec  &b) {Vec  O=Cross(a, b); O.normalize(); return O;}
VecD CrossN(C VecD &a, C VecD &b) {VecD O=Cross(a, b); O.normalize(); return O;}
/******************************************************************************/
// Perp functions have many possibilities, however they were designed to provide some sensible results

// Perp 2D always returns a vector rotated by 90 deg clockwise
Vec2  Perp(C Vec2  &v) {return Vec2 (v.y, -v.x);}
VecD2 Perp(C VecD2 &v) {return VecD2(v.y, -v.x);}
VecI2 Perp(C VecI2 &v) {return VecI2(v.y, -v.x);}

// Perp 3D treat the 'v' vector as if it was 'z' forward look direction, and the 'Perp' is 'y' up direction, so:
// when looking forward v(0,0,1), perp(0,1,0) is returned
// when looking right   v(1,0,0), perp(0,1,0) is returned
// we're comparing 'x' and 'z', which is abs bigger (gives more precision) and use that component instead of the other, additionally we always use the 'y' in case both 'x' and 'z' are close to zero
// currently when looking back v(0,0,-1) or left v(-1,0,0) then we get perp(0,-1,0), that could be changed for perp(0,1,0) however it would require more if's, which we want to avoid for performance reasons
Vec Perp(C Vec &v)
{
   if(Abs(v.x)<Abs(v.z))return Vec(0, v.z, -v.y); // Cross(v, Vec(1, 0,  0));
   else                 return Vec(-v.y, v.x, 0); // Cross(v, Vec(0, 0, -1));
}
VecD Perp(C VecD &v)
{
   if(Abs(v.x)<Abs(v.z))return VecD(0, v.z, -v.y); // Cross(v, VecD(1, 0,  0));
   else                 return VecD(-v.y, v.x, 0); // Cross(v, VecD(0, 0, -1));
}

Vec2  PerpN(C Vec2  &v) {Vec2  O=Perp(v); O.normalize(); return O;}
VecD2 PerpN(C VecD2 &v) {VecD2 O=Perp(v); O.normalize(); return O;}
Vec   PerpN(C Vec   &v) {Vec   O=Perp(v); O.normalize(); return O;}
VecD  PerpN(C VecD  &v) {VecD  O=Perp(v); O.normalize(); return O;}
/******************************************************************************/
Vec2  Reflect(C Vec2  &vec,                       C Vec2  &plane_nrm) {return plane_nrm*(2*Dot           (vec,              plane_nrm))-vec;}
VecD2 Reflect(C VecD2 &vec,                       C VecD2 &plane_nrm) {return plane_nrm*(2*Dot           (vec,              plane_nrm))-vec;}
Vec   Reflect(C Vec   &vec,                       C Vec   &plane_nrm) {return plane_nrm*(2*Dot           (vec,              plane_nrm))-vec;}
VecD  Reflect(C VecD  &vec,                       C VecD  &plane_nrm) {return plane_nrm*(2*Dot           (vec,              plane_nrm))-vec;}
Vec2  Reflect(C Vec2  &vec, C Vec2  &reflect_pos, C Vec2  &plane_nrm) {return plane_nrm*(2*DistPointPlane(vec, reflect_pos, plane_nrm))-vec+reflect_pos*2;}
VecD2 Reflect(C VecD2 &vec, C VecD2 &reflect_pos, C VecD2 &plane_nrm) {return plane_nrm*(2*DistPointPlane(vec, reflect_pos, plane_nrm))-vec+reflect_pos*2;}
Vec   Reflect(C Vec   &vec, C Vec   &reflect_pos, C Vec   &plane_nrm) {return plane_nrm*(2*DistPointPlane(vec, reflect_pos, plane_nrm))-vec+reflect_pos*2;}
VecD  Reflect(C VecD  &vec, C VecD  &reflect_pos, C VecD  &plane_nrm) {return plane_nrm*(2*DistPointPlane(vec, reflect_pos, plane_nrm))-vec+reflect_pos*2;}

Vec2  Mirror(C Vec2  &vec,                     C Vec2  &plane_nrm) {return vec-plane_nrm*(2*Dot           (vec,            plane_nrm));}
VecD2 Mirror(C VecD2 &vec,                     C VecD2 &plane_nrm) {return vec-plane_nrm*(2*Dot           (vec,            plane_nrm));}
Vec   Mirror(C Vec   &vec,                     C Vec   &plane_nrm) {return vec-plane_nrm*(2*Dot           (vec,            plane_nrm));}
VecD  Mirror(C VecD  &vec,                     C VecD  &plane_nrm) {return vec-plane_nrm*(2*Dot           (vec,            plane_nrm));}
Vec2  Mirror(C Vec2  &vec, C Vec2  &plane_pos, C Vec2  &plane_nrm) {return vec-plane_nrm*(2*DistPointPlane(vec, plane_pos, plane_nrm));}
VecD2 Mirror(C VecD2 &vec, C VecD2 &plane_pos, C VecD2 &plane_nrm) {return vec-plane_nrm*(2*DistPointPlane(vec, plane_pos, plane_nrm));}
Vec   Mirror(C Vec   &vec, C Vec   &plane_pos, C Vec   &plane_nrm) {return vec-plane_nrm*(2*DistPointPlane(vec, plane_pos, plane_nrm));}
VecD  Mirror(C VecD  &vec, C VecD  &plane_pos, C Vec   &plane_nrm) {return vec-plane_nrm*(2*DistPointPlane(vec, plane_pos, plane_nrm));}
VecD  Mirror(C VecD  &vec, C VecD  &plane_pos, C VecD  &plane_nrm) {return vec-plane_nrm*(2*DistPointPlane(vec, plane_pos, plane_nrm));}

Int Closer(C Vec2  &p, C Vec2  &p0, C Vec2  &p1) {return Dist2(p1, p)<Dist2(p0, p);}
Int Closer(C VecD2 &p, C VecD2 &p0, C VecD2 &p1) {return Dist2(p1, p)<Dist2(p0, p);}
Int Closer(C Vec   &p, C Vec   &p0, C Vec   &p1) {return Dist2(p1, p)<Dist2(p0, p);}
Int Closer(C VecD  &p, C VecD  &p0, C VecD  &p1) {return Dist2(p1, p)<Dist2(p0, p);}
/******************************************************************************/
INLINE Bool _Equal(Flt r0, Flt r1         ) {return Abs(r0-r1)<=EPS ;}
INLINE Bool _Equal(Flt r0, Flt r1, Flt eps) {return Abs(r0-r1)<=eps ;}
INLINE Bool _Equal(Dbl r0, Dbl r1         ) {return Abs(r0-r1)<=EPSD;}
INLINE Bool _Equal(Dbl r0, Dbl r1, Dbl eps) {return Abs(r0-r1)<=eps ;}

INLINE Bool _EqualWrap(Flt r0, Flt r1) {Flt d=Frac(r0-r1); return d<=EPS || d>=1-EPS;}
/******************************************************************************/
Bool Equal(Flt r0, Flt r1)
{
   return _Equal(r0, r1);
}
Bool Equal(C Vec2 &v0, C Vec2 &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y);
}
Bool Equal(C Vec &v0, C Vec &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y)
       && _Equal(v0.z, v1.z);
}
Bool Equal(C Vec4 &v0, C Vec4 &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y)
       && _Equal(v0.z, v1.z)
       && _Equal(v0.w, v1.w);
}
/******************************************************************************/
Bool Equal(Dbl r0, Dbl r1)
{
   return _Equal(r0, r1);
}
Bool Equal(C VecD2 &v0, C VecD2 &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y);
}
Bool Equal(C VecD &v0, C VecD &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y)
       && _Equal(v0.z, v1.z);
}
Bool Equal(C VecD4 &v0, C VecD4 &v1)
{
   return _Equal(v0.x, v1.x)
       && _Equal(v0.y, v1.y)
       && _Equal(v0.z, v1.z)
       && _Equal(v0.w, v1.w);
}
/******************************************************************************/
Bool Equal(Flt r0, Flt r1, Flt epsilon)
{
   return _Equal(r0, r1, epsilon);
}
Bool Equal(Dbl r0, Dbl r1, Dbl epsilon)
{
   return _Equal(r0, r1, epsilon);
}
Bool Equal(C Vec2 &v0, C Vec2 &v1, Flt epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon);
}
Bool Equal(C VecD2 &v0, C VecD2 &v1, Dbl epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon);
}
Bool Equal(C Vec &v0, C Vec &v1, Flt epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon)
       && _Equal(v0.z, v1.z, epsilon);
}
Bool Equal(C VecD &v0, C VecD &v1, Dbl epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon)
       && _Equal(v0.z, v1.z, epsilon);
}
Bool Equal(C Vec4 &v0, C Vec4 &v1, Flt epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon)
       && _Equal(v0.z, v1.z, epsilon)
       && _Equal(v0.w, v1.w, epsilon);
}
Bool Equal(C VecD4 &v0, C VecD4 &v1, Dbl epsilon)
{
   return _Equal(v0.x, v1.x, epsilon)
       && _Equal(v0.y, v1.y, epsilon)
       && _Equal(v0.z, v1.z, epsilon)
       && _Equal(v0.w, v1.w, epsilon);
}
/******************************************************************************/
Bool EqualWrap(Flt r0, Flt r1)
{
   return _EqualWrap(r0, r1);
}
Bool EqualWrap(C Vec2 &v0, C Vec2 &v1)
{
   return _EqualWrap(v0.x, v1.x)
       && _EqualWrap(v0.y, v1.y);
}
Bool EqualWrap(C Vec &v0, C Vec &v1)
{
   return _EqualWrap(v0.x, v1.x)
       && _EqualWrap(v0.y, v1.y)
       && _EqualWrap(v0.z, v1.z);
}
/******************************************************************************/
Int Compare(C VecH2 &v0, C VecH2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}
Int Compare(C Vec2 &v0, C Vec2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}
Int Compare(C VecD2 &v0, C VecD2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}

Int Compare(C VecH &v0, C VecH &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}
Int Compare(C Vec &v0, C Vec &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}
Int Compare(C VecD &v0, C VecD &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}

Int Compare(C VecH4 &v0, C VecH4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}
Int Compare(C Vec4 &v0, C Vec4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}
Int Compare(C VecD4 &v0, C VecD4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}

Int Compare(C VecI2 &v0, C VecI2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}
Int Compare(C VecB2 &v0, C VecB2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}
Int Compare(C VecSB2 &v0, C VecSB2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}
Int Compare(C VecUS2 &v0, C VecUS2 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   return 0;
}

Int Compare(C VecI &v0, C VecI &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}
Int Compare(C VecB &v0, C VecB &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}
Int Compare(C VecSB &v0, C VecSB &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}
Int Compare(C VecUS &v0, C VecUS &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   return 0;
}

Int Compare(C VecI4 &v0, C VecI4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}
Int Compare(C VecB4 &v0, C VecB4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}
Int Compare(C VecSB4 &v0, C VecSB4 &v1)
{
   if(Int c=Compare(v0.x, v1.x))return c;
   if(Int c=Compare(v0.y, v1.y))return c;
   if(Int c=Compare(v0.z, v1.z))return c;
   if(Int c=Compare(v0.w, v1.w))return c;
   return 0;
}
Int Compare(C Color &c0, C Color &c1)
{
   if(Int c=Compare(c0.r, c1.r))return c;
   if(Int c=Compare(c0.g, c1.g))return c;
   if(Int c=Compare(c0.b, c1.b))return c;
   if(Int c=Compare(c0.a, c1.a))return c;
   return 0;
}
/******************************************************************************/
Flt Sqrt(Int x) {return (x<=0) ? 0 : SqrtFast(x);}
Flt Sqrt(Flt x) {return (x<=0) ? 0 : SqrtFast(x);}
Dbl Sqrt(Dbl x) {return (x<=0) ? 0 : SqrtFast(x);}

Flt SqrtS(Int x) {return (x>=0) ? SqrtFast(x) : -SqrtFast(-x);}
Flt SqrtS(Flt x) {return (x>=0) ? SqrtFast(x) : -SqrtFast(-x);}
Dbl SqrtS(Dbl x) {return (x>=0) ? SqrtFast(x) : -SqrtFast(-x);}

Vec2 Sqrt(C Vec2 &x) {return Vec2(Sqrt(x.x), Sqrt(x.y));}
Vec  Sqrt(C Vec  &x) {return Vec (Sqrt(x.x), Sqrt(x.y), Sqrt(x.z));}
Vec4 Sqrt(C Vec4 &x) {return Vec4(Sqrt(x.x), Sqrt(x.y), Sqrt(x.z), Sqrt(x.w));}

Flt Dist(Int x, Int y       ) {return SqrtFast(Dist2(x, y   ));}
Flt Dist(Flt x, Flt y       ) {return SqrtFast(Dist2(x, y   ));}
Dbl Dist(Dbl x, Dbl y       ) {return SqrtFast(Dist2(x, y   ));}
Flt Dist(Int x, Int y, Int z) {return SqrtFast(Dist2(x, y, z));}
Flt Dist(Flt x, Flt y, Flt z) {return SqrtFast(Dist2(x, y, z));}
Dbl Dist(Dbl x, Dbl y, Dbl z) {return SqrtFast(Dist2(x, y, z));}
/******************************************************************************/
Flt Vec2 ::length()C {return SqrtFast(length2());}
Flt VecI2::length()C {return SqrtFast(length2());}
Dbl VecD2::length()C {return SqrtFast(length2());}
Flt Vec  ::length()C {return SqrtFast(length2());}
Flt VecI ::length()C {return SqrtFast(length2());}
Dbl VecD ::length()C {return SqrtFast(length2());}
Flt Vec4 ::length()C {return SqrtFast(length2());}
Flt VecI4::length()C {return SqrtFast(length2());}
Dbl VecD4::length()C {return SqrtFast(length2());}
/******************************************************************************/
Vec2 & Vec2 ::satLength() {Flt l=length2(); if(l>1)T/=SqrtFast(l); return T;}
VecD2& VecD2::satLength() {Dbl l=length2(); if(l>1)T/=SqrtFast(l); return T;}
Vec  & Vec  ::satLength() {Flt l=length2(); if(l>1)T/=SqrtFast(l); return T;}
VecD & VecD ::satLength() {Dbl l=length2(); if(l>1)T/=SqrtFast(l); return T;}
/******************************************************************************/
Flt Vec2 ::setLength(Flt length) {if(Flt l=T.length2()){l=SqrtFast(l); T*=length/l; return l;} return 0;}
Dbl VecD2::setLength(Dbl length) {if(Dbl l=T.length2()){l=SqrtFast(l); T*=length/l; return l;} return 0;}
Flt Vec  ::setLength(Flt length) {if(Flt l=T.length2()){l=SqrtFast(l); T*=length/l; return l;} return 0;}
Dbl VecD ::setLength(Dbl length) {if(Dbl l=T.length2()){l=SqrtFast(l); T*=length/l; return l;} return 0;}
/******************************************************************************/
Vec2 & Vec2 ::clipLength(Flt max_length) {Flt l=length2(); if(l>Sqr(max_length))T*=max_length/SqrtFast(l); return T;}
VecD2& VecD2::clipLength(Dbl max_length) {Dbl l=length2(); if(l>Sqr(max_length))T*=max_length/SqrtFast(l); return T;}
Vec  & Vec  ::clipLength(Flt max_length) {Flt l=length2(); if(l>Sqr(max_length))T*=max_length/SqrtFast(l); return T;}
VecD & VecD ::clipLength(Dbl max_length) {Dbl l=length2(); if(l>Sqr(max_length))T*=max_length/SqrtFast(l); return T;}
/******************************************************************************/
Flt Vec2 ::normalize() {if(Flt l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
Dbl VecD2::normalize() {if(Dbl l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
Flt Vec  ::normalize() {if(Flt l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
Dbl VecD ::normalize() {if(Dbl l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
Flt Vec4 ::normalize() {if(Flt l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
Dbl VecD4::normalize() {if(Dbl l=length2()){l=SqrtFast(l); T/=l; return l;} return 0;}
/******************************************************************************/
void Normalize(Vec2  *v, Int num) {if(v)for(; --num>=0; )(v++)->normalize();}
void Normalize(VecD2 *v, Int num) {if(v)for(; --num>=0; )(v++)->normalize();}
void Normalize(Vec   *v, Int num) {if(v)for(; --num>=0; )(v++)->normalize();}
void Normalize(VecD  *v, Int num) {if(v)for(; --num>=0; )(v++)->normalize();}
/******************************************************************************/
Vec2 & Vec2 ::chs() {CHS(x); CHS(y); return T;}
VecD2& VecD2::chs() {CHS(x); CHS(y); return T;}
VecI2& VecI2::chs() {CHS(x); CHS(y); return T;}
Vec  & Vec  ::chs() {CHS(x); CHS(y); CHS(z); return T;}
VecD & VecD ::chs() {CHS(x); CHS(y); CHS(z); return T;}
VecI & VecI ::chs() {CHS(x); CHS(y); CHS(z); return T;}
Vec4 & Vec4 ::chs() {CHS(x); CHS(y); CHS(z); CHS(w); return T;}
VecD4& VecD4::chs() {CHS(x); CHS(y); CHS(z); CHS(w); return T;}
VecI4& VecI4::chs() {CHS(x); CHS(y); CHS(z); CHS(w); return T;}

Vec2 & Vec2 ::abs() {ABS(x); ABS(y); return T;}
VecD2& VecD2::abs() {ABS(x); ABS(y); return T;}
VecI2& VecI2::abs() {ABS(x); ABS(y); return T;}
Vec  & Vec  ::abs() {ABS(x); ABS(y); ABS(z); return T;}
VecD & VecD ::abs() {ABS(x); ABS(y); ABS(z); return T;}
VecI & VecI ::abs() {ABS(x); ABS(y); ABS(z); return T;}
Vec4 & Vec4 ::abs() {ABS(x); ABS(y); ABS(z); ABS(w); return T;}
VecD4& VecD4::abs() {ABS(x); ABS(y); ABS(z); ABS(w); return T;}
VecI4& VecI4::abs() {ABS(x); ABS(y); ABS(z); ABS(w); return T;}

Vec2 & Vec2 ::sat() {SAT(x); SAT(y); return T;}
VecD2& VecD2::sat() {SAT(x); SAT(y); return T;}
VecI2& VecI2::sat() {SAT(x); SAT(y); return T;}
Vec  & Vec  ::sat() {SAT(x); SAT(y); SAT(z); return T;}
VecD & VecD ::sat() {SAT(x); SAT(y); SAT(z); return T;}
VecI & VecI ::sat() {SAT(x); SAT(y); SAT(z); return T;}
Vec4 & Vec4 ::sat() {SAT(x); SAT(y); SAT(z); SAT(w); return T;}
VecD4& VecD4::sat() {SAT(x); SAT(y); SAT(z); SAT(w); return T;}
VecI4& VecI4::sat() {SAT(x); SAT(y); SAT(z); SAT(w); return T;}
/******************************************************************************/
void Chs (Vec2  *v, Int num) {if(UInt *d=(UInt*)v){      REP(num*2){*d^=SIGN_BIT; d++ ;}}}
void Chs (VecD2 *v, Int num) {if(UInt *d=(UInt*)v){d++ ; REP(num*2){*d^=SIGN_BIT; d+=2;}}}
void Chs (Vec   *v, Int num) {if(UInt *d=(UInt*)v){      REP(num*3){*d^=SIGN_BIT; d++ ;}}}
void Chs (VecD  *v, Int num) {if(UInt *d=(UInt*)v){d++ ; REP(num*3){*d^=SIGN_BIT; d+=2;}}}

void ChsX(Vec2  *v, Int num) {if(UInt *d=(UInt*)v){      REP(num  ){*d^=SIGN_BIT; d+=2;}}}
void ChsX(Vec   *v, Int num) {if(UInt *d=(UInt*)v){      REP(num  ){*d^=SIGN_BIT; d+=3;}}}

void ChsY(Vec2  *v, Int num) {if(UInt *d=(UInt*)v){d++ ; REP(num  ){*d^=SIGN_BIT; d+=2;}}}
void ChsY(Vec   *v, Int num) {if(UInt *d=(UInt*)v){d++ ; REP(num  ){*d^=SIGN_BIT; d+=3;}}}

void ChsZ(Vec   *v, Int num) {if(UInt *d=(UInt*)v){d+=2; REP(num  ){*d^=SIGN_BIT; d+=3;}}}

void Reverse(VecI2 *v, Int num) {if(v)for(; --num>=0; )(v++)->reverse();}
void Reverse(VecI  *v, Int num) {if(v)for(; --num>=0; )(v++)->reverse();}
void Reverse(VecI4 *v, Int num) {if(v)for(; --num>=0; )(v++)->reverse();}

void SwapXZ(VecI  *v, Int num) {if(v)for(; --num>=0; )(v++)->swapXZ();}
void SwapXZ(VecI4 *v, Int num) {if(v)for(; --num>=0; )(v++)->swapXZ();}
/******************************************************************************/
Int VecI2::find(Int value)C {if(c[0]==value)return 0; if(c[1]==value)return 1; return -1;}
Int VecI ::find(Int value)C {if(c[0]==value)return 0; if(c[1]==value)return 1; if(c[2]==value)return 2; return -1;}
Int VecI4::find(Int value)C {if(c[0]==value)return 0; if(c[1]==value)return 1; if(c[2]==value)return 2; if(c[3]==value)return 3; return -1;}
Int VecB ::find(Int value)C {if(c[0]==value)return 0; if(c[1]==value)return 1; if(c[2]==value)return 2; return -1;}
Int VecB4::find(Int value)C {if(c[0]==value)return 0; if(c[1]==value)return 1; if(c[2]==value)return 2; if(c[3]==value)return 3; return -1;}
/******************************************************************************/
VecI & VecI ::rotateOrder() {Int t=z;      z=y; y=x; x=t; return T;}
VecI4& VecI4::rotateOrder() {Int t=w; w=z; z=y; y=x; x=t; return T;}
/******************************************************************************/
Bool MinMax(C Vec2 *v, Int num, Vec2 &min, Vec2 &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Flt r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
   }
   return true;
}
Bool MinMax(C VecD2 *v, Int num, VecD2 &min, VecD2 &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Dbl r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
   }
   return true;
}
Bool MinMax(C Vec *v, Int num, Vec &min, Vec &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Flt r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
      r=v->z; if(r<min.z)min.z=r;else if(r>max.z)max.z=r;
   }
   return true;
}
Bool MinMax(C VecD *v, Int num, VecD &min, VecD &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Dbl r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
      r=v->z; if(r<min.z)min.z=r;else if(r>max.z)max.z=r;
   }
   return true;
}
Bool MinMax(C Vec4 *v, Int num, Vec4 &min, Vec4 &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Flt r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
      r=v->z; if(r<min.z)min.z=r;else if(r>max.z)max.z=r;
      r=v->w; if(r<min.w)min.w=r;else if(r>max.w)max.w=r;
   }
   return true;
}
Bool MinMax(C VecD4 *v, Int num, VecD4 &min, VecD4 &max)
{
   if(num<=0 || !v)return false;
   for(min=max=*v++, num--; --num>=0; v++)
   {
      Dbl r;
      r=v->x; if(r<min.x)min.x=r;else if(r>max.x)max.x=r;
      r=v->y; if(r<min.y)min.y=r;else if(r>max.y)max.y=r;
      r=v->z; if(r<min.z)min.z=r;else if(r>max.z)max.z=r;
      r=v->w; if(r<min.w)min.w=r;else if(r>max.w)max.w=r;
   }
   return true;
}
VecB4 Min(C VecB4 &a, C VecB4 &b) {return VecB4(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z), Min(a.w, b.w));}
VecB4 Max(C VecB4 &a, C VecB4 &b) {return VecB4(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z), Max(a.w, b.w));}

VecB4 Min(C VecB4 &a, C VecB4 &b, C VecB4 &c) {return VecB4(Min(a.x, b.x, c.x), Min(a.y, b.y, c.y), Min(a.z, b.z, c.z), Min(a.w, b.w, c.w));}
VecB4 Max(C VecB4 &a, C VecB4 &b, C VecB4 &c) {return VecB4(Max(a.x, b.x, c.x), Max(a.y, b.y, c.y), Max(a.z, b.z, c.z), Max(a.w, b.w, c.w));}

VecB4 Min(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d) {return VecB4(Min(a.x, b.x, c.x, d.x), Min(a.y, b.y, c.y, d.y), Min(a.z, b.z, c.z, d.z), Min(a.w, b.w, c.w, d.w));}
VecB4 Max(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d) {return VecB4(Max(a.x, b.x, c.x, d.x), Max(a.y, b.y, c.y, d.y), Max(a.z, b.z, c.z, d.z), Max(a.w, b.w, c.w, d.w));}
/******************************************************************************/
void Transform(Vec2 *v, C Matrix3 &m, Int num)
{
   if(v)for(; --num>=0; v++)
   {
      Flt x=v->x, y=v->y;
      v->x=x*m.x.x + y*m.y.x;
      v->y=x*m.x.y + y*m.y.y;
   }
}
void Transform(Vec2 *v, C Matrix &m, Int num)
{
   if(v)for(; --num>=0; v++)
   {
      Flt x=v->x, y=v->y;
      v->x=x*m.x.x + y*m.y.x + m.pos.x;
      v->y=x*m.x.y + y*m.y.y + m.pos.y;
   }
}
void Transform(Vec *v, C Matrix3 &m, Int num)
{
   if(v)for(; --num>=0; v++)
   {
      Flt x=v->x, y=v->y, z=v->z;
      v->x=x*m.x.x + y*m.y.x + z*m.z.x;
      v->y=x*m.x.y + y*m.y.y + z*m.z.y;
      v->z=x*m.x.z + y*m.y.z + z*m.z.z;
   }
}
void Transform(Vec *v, C Matrix &m, Int num)
{
   if(v)for(; --num>=0; v++)
   {
      Flt x=v->x, y=v->y, z=v->z;
      v->x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
      v->y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
      v->z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   }
}
void Transform(VecD *v, C MatrixM &m, Int num)
{
   if(v)for(; --num>=0; v++)
   {
      Dbl x=v->x, y=v->y, z=v->z;
      v->x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
      v->y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
      v->z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   }
}
/******************************************************************************/
void Vec::rightToLeft(                 ) {swapYZ(); CHS(x); CHS(z);}
void      RightToLeft(Vec *vec, Int num) {if(vec)REP(num)vec[i].rightToLeft();}
/******************************************************************************/
Vec2& Vec2::mul(C Matrix3 &m)
{
   Flt _x=x, _y=y;
   x=_x*m.x.x + _y*m.y.x;
   y=_x*m.x.y + _y*m.y.y;
   return T;
}
VecD2& VecD2::mul(C MatrixD3 &m)
{
   Dbl _x=x, _y=y;
   x=_x*m.x.x + _y*m.y.x;
   y=_x*m.x.y + _y*m.y.y;
   return T;
}
Vec2& Vec2::mul(C Matrix &m)
{
   Flt _x=x, _y=y;
   x=_x*m.x.x + _y*m.y.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + m.pos.y;
   return T;
}
VecD2& VecD2::mul(C MatrixD &m)
{
   Dbl _x=x, _y=y;
   x=_x*m.x.x + _y*m.y.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + m.pos.y;
   return T;
}
/******************************************************************************/
Vec& Vec::mul(C Orient &o)
{
   Flt _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z;
   return T;
}
Vec& Vec::mul(C OrientD &o)
{
   Flt _x=x, _y=y, _z=z; VecD cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z;
   return T;
}
Vec& Vec::mul(C OrientP &o)
{
   Flt _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x + o.pos.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y + o.pos.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z + o.pos.z;
   return T;
}
Vec& Vec::mul(C OrientM &o)
{
   Flt _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x + o.pos.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y + o.pos.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z + o.pos.z;
   return T;
}
/******************************************************************************/
VecD& VecD::mul(C Orient &o)
{
   Dbl _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z;
   return T;
}
VecD& VecD::mul(C OrientD &o)
{
   Dbl _x=x, _y=y, _z=z; VecD cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z;
   return T;
}
VecD& VecD::mul(C OrientP &o)
{
   Dbl _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x + o.pos.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y + o.pos.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z + o.pos.z;
   return T;
}
VecD& VecD::mul(C OrientM &o)
{
   Dbl _x=x, _y=y, _z=z; Vec cross=o.cross();
   x=_x*cross.x + _y*o.perp.x + _z*o.dir.x + o.pos.x;
   y=_x*cross.y + _y*o.perp.y + _z*o.dir.y + o.pos.y;
   z=_x*cross.z + _y*o.perp.z + _z*o.dir.z + o.pos.z;
   return T;
}
/******************************************************************************/
Vec& Vec::mul(C Matrix3 &m)
{
   Flt _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z;
   return T;
}
Vec& Vec::mul(C MatrixD3 &m)
{
   Flt _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z;
   return T;
}
VecD& VecD::mul(C Matrix3 &m)
{
   Dbl _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z;
   return T;
}
VecD& VecD::mul(C MatrixD3 &m)
{
   Dbl _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z;
   return T;
}
Vec& Vec::mul(C Matrix &m)
{
   Flt _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
Vec& Vec::mul(C MatrixM &m)
{
   Flt _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
Vec& Vec::mul(C MatrixD &m)
{
   Flt _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
VecD& VecD::mul(C Matrix &m)
{
   Dbl _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
VecD& VecD::mul(C MatrixM &m)
{
   Dbl _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
VecD& VecD::mul(C MatrixD &m)
{
   Dbl _x=x, _y=y, _z=z;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + m.pos.z;
   return T;
}
/******************************************************************************/
Vec4& Vec4::mul(C Matrix4 &m)
{
   Flt _x=x, _y=y, _z=z, _w=w;
   x=_x*m.x.x + _y*m.y.x + _z*m.z.x + _w*m.pos.x;
   y=_x*m.x.y + _y*m.y.y + _z*m.z.y + _w*m.pos.y;
   z=_x*m.x.z + _y*m.y.z + _z*m.z.z + _w*m.pos.z;
   w=_x*m.x.w + _y*m.y.w + _z*m.z.w + _w*m.pos.w;
   return T;
}
/******************************************************************************/
Vec2& Vec2::div(C Matrix3 &m)
{
   Vec t(T, 0);
   set(Dot(t, m.x)/m.x.length2(),
       Dot(t, m.y)/m.y.length2());
   return T;
}
VecD2& VecD2::div(C MatrixD3 &m)
{
   VecD t(T, 0);
   set(Dot(t, m.x)/m.x.length2(),
       Dot(t, m.y)/m.y.length2());
   return T;
}
Vec2& Vec2::div(C Matrix &m)
{
   Vec t(T, 0); t-=m.pos;
   set(Dot(t, m.x)/m.x.length2(),
       Dot(t, m.y)/m.y.length2());
   return T;
}
VecD2& VecD2::div(C MatrixD &m)
{
   VecD t(T, 0); t-=m.pos;
   set(Dot(t, m.x)/m.x.length2(),
       Dot(t, m.y)/m.y.length2());
   return T;
}
/******************************************************************************/
Vec2& Vec2::divNormalized(C Matrix3 &m)
{
   Vec t(T, 0);
   set(Dot(t, m.x),
       Dot(t, m.y));
   return T;
}
VecD2& VecD2::divNormalized(C MatrixD3 &m)
{
   VecD t(T, 0);
   set(Dot(t, m.x),
       Dot(t, m.y));
   return T;
}
Vec2& Vec2::divNormalized(C Matrix &m)
{
   Vec t(T, 0); t-=m.pos;
   set(Dot(t, m.x),
       Dot(t, m.y));
   return T;
}
VecD2& VecD2::divNormalized(C MatrixD &m)
{
   VecD t(T, 0); t-=m.pos;
   set(Dot(t, m.x),
       Dot(t, m.y));
   return T;
}
/******************************************************************************/
Vec& Vec::div(C Matrix3 &m)
{
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
Vec& Vec::div(C MatrixD3 &m)
{
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
VecD& VecD::div(C Matrix3 &m)
{
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
VecD& VecD::div(C MatrixD3 &m)
{
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
Vec& Vec::div(C Matrix &m)
{
   T-=m.pos;
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
Vec& Vec::div(C MatrixM &m)
{
   VecD d=T-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
Vec& Vec::div(C MatrixD &m)
{
   VecD d=T-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
VecD& VecD::div(C Matrix &m)
{
   T-=m.pos;
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
VecD& VecD::div(C MatrixM &m)
{
   T-=m.pos;
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
VecD& VecD::div(C MatrixD &m)
{
   T-=m.pos;
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
/******************************************************************************/
Vec& Vec::divNormalized(C Matrix3 &m)
{
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
Vec& Vec::divNormalized(C MatrixD3 &m)
{
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
VecD& VecD::divNormalized(C Matrix3 &m)
{
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
VecD& VecD::divNormalized(C MatrixD3 &m)
{
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
Vec& Vec::divNormalized(C Matrix &m)
{
   T-=m.pos;
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
Vec& Vec::divNormalized(C MatrixM &m)
{
   VecD d=T-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
Vec& Vec::divNormalized(C MatrixD &m)
{
   VecD d=T-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
VecD& VecD::divNormalized(C Matrix &m)
{
   T-=m.pos;
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
VecD& VecD::divNormalized(C MatrixM &m)
{
   T-=m.pos;
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
VecD& VecD::divNormalized(C MatrixD &m)
{
   T-=m.pos;
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
/******************************************************************************/
Vec& Vec::fromDiv(C Vec &v, C Matrix3 &m)
{
   set(Dot(v, m.x)/m.x.length2(),
       Dot(v, m.y)/m.y.length2(),
       Dot(v, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C Vec &v, C MatrixD3 &m)
{
   set(Dot(v, m.x)/m.x.length2(),
       Dot(v, m.y)/m.y.length2(),
       Dot(v, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C Vec &v, C Matrix &m)
{
   T=v-m.pos;
   set(Dot(T, m.x)/m.x.length2(),
       Dot(T, m.y)/m.y.length2(),
       Dot(T, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C Vec &v, C MatrixM &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C Vec &v, C MatrixD &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
/******************************************************************************/
Vec& Vec::fromDiv(C VecD &v, C Matrix3 &m)
{
   set(Dot(v, m.x)/m.x.length2(),
       Dot(v, m.y)/m.y.length2(),
       Dot(v, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C VecD &v, C MatrixD3 &m)
{
   set(Dot(v, m.x)/m.x.length2(),
       Dot(v, m.y)/m.y.length2(),
       Dot(v, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C VecD &v, C Matrix &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C VecD &v, C MatrixM &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
Vec& Vec::fromDiv(C VecD &v, C MatrixD &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x)/m.x.length2(),
       Dot(d, m.y)/m.y.length2(),
       Dot(d, m.z)/m.z.length2());
   return T;
}
/******************************************************************************/
Vec& Vec::fromDivNormalized(C Vec &v, C Matrix3 &m)
{
   set(Dot(v, m.x),
       Dot(v, m.y),
       Dot(v, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C MatrixD3 &m)
{
   set(Dot(v, m.x),
       Dot(v, m.y),
       Dot(v, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C Matrix &m)
{
   T=v-m.pos;
   set(Dot(T, m.x),
       Dot(T, m.y),
       Dot(T, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C MatrixM &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C MatrixD &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
/******************************************************************************/
Vec& Vec::fromDivNormalized(C VecD &v, C Matrix3 &m)
{
   set(Dot(v, m.x),
       Dot(v, m.y),
       Dot(v, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C VecD &v, C MatrixD3 &m)
{
   set(Dot(v, m.x),
       Dot(v, m.y),
       Dot(v, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C VecD &v, C Matrix &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C VecD &v, C MatrixM &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
Vec& Vec::fromDivNormalized(C VecD &v, C MatrixD &m)
{
   VecD d=v-m.pos;
   set(Dot(d, m.x),
       Dot(d, m.y),
       Dot(d, m.z));
   return T;
}
/******************************************************************************/
Vec& Vec::fromDivNormalized(C Vec &v, C Orient &o)
{
   set(Dot(v, o.cross()),
       Dot(v, o.perp   ),
       Dot(v, o.dir    ));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C OrientD &o)
{
   set(Dot(v, o.cross()),
       Dot(v, o.perp   ),
       Dot(v, o.dir    ));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C OrientP &o)
{
   T=v-o.pos;
   set(Dot(T, o.cross()),
       Dot(T, o.perp   ),
       Dot(T, o.dir    ));
   return T;
}
Vec& Vec::fromDivNormalized(C Vec &v, C OrientM &o)
{
   VecD d=v-o.pos;
   set(Dot(d, o.cross()),
       Dot(d, o.perp   ),
       Dot(d, o.dir    ));
   return T;
}
/******************************************************************************/
Vec2& Vec2::rotate(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      set(x*cos - y*sin,
          x*sin + y*cos);
   }
   return T;
}
Vec2& Vec2::rotateCosSin(Flt cos, Flt sin)
{
   return set(x*cos - y*sin,
              x*sin + y*cos);
}
/******************************************************************************/
Vec& Vec::rotateX(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Flt y=T.y, z=T.z;
      T.y=y*cos - z*sin;
      T.z=y*sin + z*cos;
   }
   return T;
}
Vec& Vec::rotateY(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Flt x=T.x, z=T.z;
      T.x=x*cos + z*sin;
      T.z=z*cos - x*sin;
   }
   return T;
}
Vec& Vec::rotateZ(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Flt x=T.x, y=T.y;
      T.x=x*cos - y*sin;
      T.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Vec2& Vec2::operator&=(C Rect &r)
{
   if(x<r.min.x)x=r.min.x;else if(x>r.max.x)x=r.max.x;
   if(y<r.min.y)y=r.min.y;else if(y>r.max.y)y=r.max.y;
   return T;
}
VecI2& VecI2::operator&=(C RectI &r)
{
   if(x<r.min.x)x=r.min.x;else if(x>r.max.x)x=r.max.x;
   if(y<r.min.y)y=r.min.y;else if(y>r.max.y)y=r.max.y;
   return T;
}
Vec& Vec::operator&=(C Box &b)
{
   if(x<b.min.x)x=b.min.x;else if(x>b.max.x)x=b.max.x;
   if(y<b.min.y)y=b.min.y;else if(y>b.max.y)y=b.max.y;
   if(z<b.min.z)z=b.min.z;else if(z>b.max.z)z=b.max.z;
   return T;
}
Vec& Vec::operator&=(C OBox &b)
{
   Vec r=T-b.matrix.pos;
   Flt d=Dot(r, b.matrix.x); if(d<b.box.min.x)T+=b.matrix.x*(b.box.min.x-d);else if(d>b.box.max.x)T-=b.matrix.x*(d-b.box.max.x);
       d=Dot(r, b.matrix.y); if(d<b.box.min.y)T+=b.matrix.y*(b.box.min.y-d);else if(d>b.box.max.y)T-=b.matrix.y*(d-b.box.max.y);
       d=Dot(r, b.matrix.z); if(d<b.box.min.z)T+=b.matrix.z*(b.box.min.z-d);else if(d>b.box.max.z)T-=b.matrix.z*(d-b.box.max.z);
   return T;
}
VecI& VecI::operator&=(C BoxI &b)
{
   if(x<b.min.x)x=b.min.x;else if(x>b.max.x)x=b.max.x;
   if(y<b.min.y)y=b.min.y;else if(y>b.max.y)y=b.max.y;
   if(z<b.min.z)z=b.min.z;else if(z>b.max.z)z=b.max.z;
   return T;
}

Bool Vec2 ::operator==(  Flt    r)C {return x==r   && y==r;}
Bool Vec2 ::operator!=(  Flt    r)C {return x!=r   || y!=r;}
Bool Vec2 ::operator==(C Vec2  &v)C {return x==v.x && y==v.y;}
Bool Vec2 ::operator!=(C Vec2  &v)C {return x!=v.x || y!=v.y;}
Bool VecD2::operator==(C VecD2 &v)C {return x==v.x && y==v.y;}
Bool VecD2::operator!=(C VecD2 &v)C {return x!=v.x || y!=v.y;}
Bool Vec  ::operator==(  Flt    r)C {return x==r   && y==r   && z==r;}
Bool Vec  ::operator!=(  Flt    r)C {return x!=r   || y!=r   || z!=r;}
Bool Vec  ::operator==(C Vec   &v)C {return x==v.x && y==v.y && z==v.z;}
Bool Vec  ::operator!=(C Vec   &v)C {return x!=v.x || y!=v.y || z!=v.z;}
Bool VecD ::operator==(C VecD  &v)C {return x==v.x && y==v.y && z==v.z;}
Bool VecD ::operator!=(C VecD  &v)C {return x!=v.x || y!=v.y || z!=v.z;}
Bool Vec4 ::operator==(  Flt    r)C {return x==r   && y==r   && z==r   && w==r;}
Bool Vec4 ::operator!=(  Flt    r)C {return x!=r   || y!=r   || z!=r   || w!=r;}
Bool Vec4 ::operator==(C Vec4  &v)C {return x==v.x && y==v.y && z==v.z && w==v.w;}
Bool Vec4 ::operator!=(C Vec4  &v)C {return x!=v.x || y!=v.y || z!=v.z || w!=v.w;}
Bool VecD4::operator==(C VecD4 &v)C {return x==v.x && y==v.y && z==v.z && w==v.w;}
Bool VecD4::operator!=(C VecD4 &v)C {return x!=v.x || y!=v.y || z!=v.z || w!=v.w;}
/******************************************************************************/
Str VecH2::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_HALF; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); return s;}
Str Vec2 ::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_FLT ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); return s;}
Str VecD2::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_DBL ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); return s;}
Str VecH ::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_HALF; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); return s;}
Str Vec  ::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_FLT ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); return s;}
Str VecD ::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_DBL ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); return s;}
Str VecH4::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_HALF; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); s+=", "; s+=TextReal(w, temp, precision); return s;}
Str Vec4 ::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_FLT ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); s+=", "; s+=TextReal(w, temp, precision); return s;}
Str VecD4::asText(Int precision)C {if(precision==INT_MAX)precision=PRECISION_DBL ; Char8 temp[256]; Str s=TextReal(x, temp, precision); s+=", "; s+=TextReal(y, temp, precision); s+=", "; s+=TextReal(z, temp, precision); s+=", "; s+=TextReal(w, temp, precision); return s;}

#if 0
Str VecB4::asTextDots()C {Char8 text[256], temp[256]; Set(text, TextInt(x, temp)); Append(text, "."); Append(text, TextInt(y, temp)); Append(text, "."); Append(text, TextInt(z, temp)); Append(text, "."); Append(text, TextInt(w, temp)); return text;}
Str VecI4::asTextDots()C {Char8 text[256], temp[256]; Set(text, TextInt(x, temp)); Append(text, "."); Append(text, TextInt(y, temp)); Append(text, "."); Append(text, TextInt(z, temp)); Append(text, "."); Append(text, TextInt(w, temp)); return text;}
#else
Str VecB4::asTextDots()C {Char8 temp[256]; Str s; s+=TextInt(x, temp); s+='.'; s+=TextInt(y, temp); s+="."; s+=TextInt(z, temp); s+="."; s+=TextInt(w, temp); return s;}
Str VecI4::asTextDots()C {Char8 temp[256]; Str s; s+=TextInt(x, temp); s+='.'; s+=TextInt(y, temp); s+="."; s+=TextInt(z, temp); s+="."; s+=TextInt(w, temp); return s;}
#endif
/******************************************************************************/
void Vec2::draw(C Color &color, Flt r)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_STRIP);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(4))
   {
      v[0].pos.set(x-r, y+r);
      v[1].pos.set(x+r, y+r);
      v[2].pos.set(x-r, y-r);
      v[3].pos.set(x+r, y-r);
   }
   VI.end();
}
void Vec::draw (C Color &color, Flt r)C {Vec2 screen; if(PosToScreenM(T, screen))screen.draw(color, r);}
void Vec::drawP(C Color &color, Flt r)C {Vec2 screen; if(PosToScreenM(T, screen))screen.draw(color, r/((T*ObjMatrix)*CamMatrixInv).z);}

void VecD2::draw (C Color &color, Flt r)C {return Vec2(T).draw (color, r);}
void VecD ::draw (C Color &color, Flt r)C {return Vec (T).draw (color, r);}
void VecD ::drawP(C Color &color, Flt r)C {return Vec (T).drawP(color, r);}
/******************************************************************************/
CChar8* DirToText(DIR_ENUM dir)
{
   switch(dir)
   {
      case DIR_RIGHT  : return "Right";
      case DIR_LEFT   : return "Left";
      case DIR_UP     : return "Up";
      case DIR_DOWN   : return "Down";
      case DIR_FORWARD: return "Forward";
      case DIR_BACK   : return "Back";
      default         : return null;
   }
}
AXIS_TYPE DirToAxis(DIR_ENUM dir)
{
   switch(dir)
   {
      default         :
      case DIR_RIGHT  :
      case DIR_LEFT   : return AXIS_X;
      case DIR_UP     :
      case DIR_DOWN   : return AXIS_Y;
      case DIR_FORWARD:
      case DIR_BACK   : return AXIS_Z;
   }
}
/******************************************************************************/
}
/******************************************************************************/
