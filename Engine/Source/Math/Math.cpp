/******************************************************************************

   Floating Point Performance, Tested on Intel i7-3632QM 2.2 Ghz

      Operations Flt x Flt, have the same performance as Dbl x Dbl.
      However Flt x Dbl (mixed precision) are slower.
      Casting to the same types restores full performance: Flt x Flt(Dbl), without any cost for casting.

   Floating Point Precision:

      After 1 month of running an app:
      Flt time=60*60*24*30; (60s*60m*24h*30d = 2592000 seconds)
      At this value, 'time' + average frame time delta (1.0f/60) is the same as 'time'
      time+=1.0f/60; // does not modify 'time'

/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
const Half HalfZero=false, HalfOne=true;
const Int PrimeNumbers[16]={2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53};
/******************************************************************************/
Bool Special(C Flt &r) {UInt *d=(UInt*)&r; return (d[0]&0x7F800000)==0x7F800000         ;} // all exponent bits are on
Bool Special(C Dbl &r) {UInt *d=(UInt*)&r; return (d[1]&0x7FF00000)==0x7FF00000         ;} // all exponent bits are on
Bool NaN    (C Flt &r) {UInt *d=(UInt*)&r; return Special(r) &&  (d[0]&0x7FFFFF        );} // significand!=0
Bool Inf    (C Flt &r) {UInt *d=(UInt*)&r; return Special(r) && !(d[0]&0x7FFFFF        );} // significand==0
Bool NaN    (C Dbl &r) {UInt *d=(UInt*)&r; return Special(r) &&  (d[1]&0x0FFFFF || d[0]);} // significand!=0
Bool Inf    (C Dbl &r) {UInt *d=(UInt*)&r; return Special(r) && !(d[1]&0x0FFFFF || d[0]);} // significand==0
/******************************************************************************/
Vec2  ScaleFactor(C Vec2  &vec) {return Vec2 (ScaleFactor(vec.x), ScaleFactor(vec.y));}
VecD2 ScaleFactor(C VecD2 &vec) {return VecD2(ScaleFactor(vec.x), ScaleFactor(vec.y));}
Vec   ScaleFactor(C Vec   &vec) {return Vec  (ScaleFactor(vec.x), ScaleFactor(vec.y), ScaleFactor(vec.z));}
VecD  ScaleFactor(C VecD  &vec) {return VecD (ScaleFactor(vec.x), ScaleFactor(vec.y), ScaleFactor(vec.z));}
Vec4  ScaleFactor(C Vec4  &vec) {return Vec4 (ScaleFactor(vec.x), ScaleFactor(vec.y), ScaleFactor(vec.z), ScaleFactor(vec.w));}
VecD4 ScaleFactor(C VecD4 &vec) {return VecD4(ScaleFactor(vec.x), ScaleFactor(vec.y), ScaleFactor(vec.z), ScaleFactor(vec.w));}

Vec2  ScaleFactorR(C Vec2  &vec) {return Vec2 (ScaleFactorR(vec.x), ScaleFactorR(vec.y));}
VecD2 ScaleFactorR(C VecD2 &vec) {return VecD2(ScaleFactorR(vec.x), ScaleFactorR(vec.y));}
Vec   ScaleFactorR(C Vec   &vec) {return Vec  (ScaleFactorR(vec.x), ScaleFactorR(vec.y), ScaleFactorR(vec.z));}
VecD  ScaleFactorR(C VecD  &vec) {return VecD (ScaleFactorR(vec.x), ScaleFactorR(vec.y), ScaleFactorR(vec.z));}
Vec4  ScaleFactorR(C Vec4  &vec) {return Vec4 (ScaleFactorR(vec.x), ScaleFactorR(vec.y), ScaleFactorR(vec.z), ScaleFactorR(vec.w));}
VecD4 ScaleFactorR(C VecD4 &vec) {return VecD4(ScaleFactorR(vec.x), ScaleFactorR(vec.y), ScaleFactorR(vec.z), ScaleFactorR(vec.w));}
/******************************************************************************/
void MinMax(C Flt *f, Int elms, Flt &min, Flt &max)
{
   if(elms)for(min=max=*f++; --elms; )
   {
      Flt r=*f++;
      if(r<min)min=r;else
      if(r>max)max=r;
   }
}
void MinMax(C Dbl *f, Int elms, Dbl &min, Dbl &max)
{
   if(elms)for(min=max=*f++; --elms; )
   {
      Dbl r=*f++;
      if(r<min)min=r;else
      if(r>max)max=r;
   }
}
void MinMaxI(C Flt *f, Int elms, Int &min, Int &max)
{
   if(elms)
   {
      Flt _min, _max;
     _min=_max=f[0];
      min= max=  0 ;
      for(Int i=1; i<elms; i++)
      {
         Flt r=f[i];
         if(r<_min){_min=r; min=i;}else
         if(r>_max){_max=r; max=i;}
      }
   }
}
void MinMaxI(C Dbl *f, Int elms, Int &min, Int &max)
{
   if(elms)
   {
      Dbl _min, _max;
     _min=_max=f[0];
      min= max=  0 ;
      for(Int i=1; i<elms; i++)
      {
         Dbl r=f[i];
         if(r<_min){_min=r; min=i;}else
         if(r>_max){_max=r; max=i;}
      }
   }
}
/******************************************************************************/
UInt SqrtI(UInt x, Int max_steps)
{
   if(x<=1)return x;
   UInt y=1<<(BitHi(x)>>1); // approximate the sqrt
   REP(max_steps)
   {
      UInt prev=y;
      y=(y + x/y + 1)/2;
      if(prev==y)break; // if no change was made then stop
   }
   return y;
}
UInt SqrtI(UInt x)
{
   UInt res=0, one=1<<30;
   for(; one>x; one>>=2);
   for(; one; )
   {
     if(x>=res+one)
     {
         x  -=res+one;
         res+=one<<1;
     }
     res>>=1;
     one>>=2;
   }
   if(x>res)res++; // use rounding
   return res;
}
UInt SqrtI(ULong x)
{
   ULong res=0, one=1ll<<62;
   for(; one>x; one>>=2);
   for(; one; )
   {
     if(x>=res+one)
     {
         x  -=res+one;
         res+=one<<1;
     }
     res>>=1;
     one>>=2;
   }
   if(x>res)res++; // use rounding
   return res;
}
Int SqrtI(Int  x) {return (x>0) ? SqrtI(UInt (x)) : 0;}
Int SqrtI(Long x) {return (x>0) ? SqrtI(ULong(x)) : 0;}

Flt Log(Flt x, Flt base) {return logf(x)/logf(base);}
Dbl Log(Dbl x, Dbl base) {return log (x)/log (base);}

Flt Pinch(Flt x, Flt pinch) {return x*pinch/(1+x*(pinch-1));}
/******************************************************************************/
// ROUNDING
/******************************************************************************/
Dbl TruncD(Dbl x)
{
   Dbl    trunc; modf(x, &trunc);
   return trunc;
}
/******************************************************************************/
Flt SplitAlpha(Flt alpha, Int steps)
{
/* For 2 steps:
   apply regular alpha blending:
      alpha*src + (1-alpha)*dest = final
   apply regular alpha blending but two times in a row (now using unknown 'beta' instead of 'alpha'):
      beta*src + (1-beta)*(beta*src + (1-beta)*dest) = final

   now we want 'final' result to be the same in both cases:
      alpha*src + (1-alpha)*dest = beta*src + (1-beta)*(beta*src + (1-beta)*dest)

   alpha*src + dest - dest*alpha = beta*src + (1-beta)*(beta*src + dest - dest*beta)
   alpha*src + dest - dest*alpha = beta*src + beta*src + dest - dest*beta - beta*beta*src - dest*beta + dest*beta*beta
   alpha*src - dest*alpha = beta*src + beta*src - dest*beta - beta*beta*src - dest*beta + dest*beta*beta
   alpha*src - dest*alpha = beta*beta*(dest - src) + beta*2*(src - dest) 
   alpha*src - dest*alpha = beta*beta*(dest - src) - beta*2*(dest - src) 
   (alpha*src - dest*alpha)/(dest-src) = beta*beta - beta*2
   -alpha = beta*beta - beta*2 -> assumed dest=1, src=0
   beta*beta + (-2)*beta + alpha = 0

   delta = -2*-2 - 4*alpha*1
   return 1-Sqrt(4-4*alpha)*0.5
   return 1-2*Sqrt(1-alpha)*0.5;

   gives final result of: return 1-Sqrt(1-alpha);
   it was tested that for 3 steps: return 1-Cbrt(1-alpha); gives positive results

   So the final formula is: */
   return (steps>1) ? 1-Pow(1-alpha, 1.0f/steps) : alpha;
}
Flt     VisibleOpacity(Flt density, Flt range) {return   Pow(1-density, range);}
Flt AccumulatedDensity(Flt density, Flt range) {return 1-Pow(1-density, range);}
/******************************************************************************/
// ANGLES
/******************************************************************************/
Vec2 Tan(C Vec2 &angle) {return Vec2(Tan(angle.x), Tan(angle.y));}

void CosSin(Flt &cos, Flt &sin, Flt angle)
{
#if APPLE
 __sincosf(angle, &sin, &cos);
#elif !WINDOWS
   sincosf(angle, &sin, &cos);
#elif X64 || ARM
   sin=Sin(angle);
   cos=Cos(angle);
#else
  _asm
   {
      mov edx, cos
      mov eax, sin
      fld angle
      fsincos
      fstp dword ptr[edx]
      fstp dword ptr[eax]
   }
#endif
}
void CosSin(Dbl &cos, Dbl &sin, Dbl angle)
{
#if APPLE
 __sincos(angle, &sin, &cos);
#elif !WINDOWS
   sincos(angle, &sin, &cos);
#elif X64 || ARM
   sin=Sin(angle);
   cos=Cos(angle);
#else
  _asm
   {
      mov edx, cos
      mov eax, sin
      fld angle
      fsincos
      fstp qword ptr[edx]
      fstp qword ptr[eax]
   }
#endif
}
Flt Acos(Flt cos)
{
   if(cos>= 1)return  0;
   if(cos<=-1)return PI;
              return acosf(cos);
}
Dbl Acos(Dbl cos)
{
   if(cos>= 1)return   0;
   if(cos<=-1)return PID;
              return acos(cos);
}
Flt Asin(Flt sin)
{
   if(sin>= 1)return  PI_2;
   if(sin<=-1)return -PI_2;
              return  asinf(sin);
}
Dbl Asin(Dbl sin)
{
   if(sin>= 1)return  PID_2;
   if(sin<=-1)return -PID_2;
              return  asin(sin);
}

Flt ACosSin(Flt cos, Flt sin) // assumes "sin>=0"
{ // use Acos for angles 45..135 deg
   return (sin>=SQRT2_2) ?            Acos(cos)
                         : (cos>=0) ? Asin(sin) : PI-Asin(sin);
}
Dbl ACosSin(Dbl cos, Dbl sin) // assumes "sin>=0"
{ // use Acos for angles 45..135 deg
   return (sin>=SQRT2_2) ?            Acos(cos)
                         : (cos>=0) ? Asin(sin) : PID-Asin(sin);
}

Vec2 Atan(C Vec2 &tan) {return Vec2(Atan(tan.x), Atan(tan.y));}

Flt AngleFast(Flt x, Flt y)
{
   Flt r=Abs(x)+Abs(y);
   if(!r)return 0;
   return (y>=0) ? (1-x/r)*PI_2 : (x/r-1)*PI_2;
}
Flt AngleFast(C Vec2 &v)
{
   Flt r=Abs(v.x)+Abs(v.y);
   if(!r)return 0;
   return (v.y>=0) ? (1-v.x/r)*PI_2 : (v.x/r-1)*PI_2;
}

Flt AngleFast(C Vec &v, C Vec &x, C Vec &y) {return AngleFast(Dot(v, x), Dot(v, y));}
Flt Angle    (C Vec &v, C Vec &x, C Vec &y) {return Angle    (Dot(v, x), Dot(v, y));}
/******************************************************************************/
Flt AngleNormalize(Flt angle) {angle=AngleFull(angle); return (angle>PI ) ? angle-PI2  : angle;}
Dbl AngleNormalize(Dbl angle) {angle=AngleFull(angle); return (angle>PID) ? angle-PID2 : angle;}
/******************************************************************************/
Flt CosBetween(C Vec2  &a, C Vec2  &b) {if(Flt l2=a.length2()*b.length2())return Dot(a, b)/SqrtFast(l2); return 0;}
Dbl CosBetween(C VecD2 &a, C VecD2 &b) {if(Dbl l2=a.length2()*b.length2())return Dot(a, b)/SqrtFast(l2); return 0;}
Flt CosBetween(C Vec   &a, C Vec   &b) {if(Flt l2=a.length2()*b.length2())return Dot(a, b)/SqrtFast(l2); return 0;}
Dbl CosBetween(C VecD  &a, C VecD  &b) {if(Dbl l2=a.length2()*b.length2())return Dot(a, b)/SqrtFast(l2); return 0;}

Flt    SinBetween(C Vec2  &a, C Vec2  &b) {if(Flt l2=a.length2()*b.length2())return          Cross(a, b)/ SqrtFast(l2); return 0;}
Dbl    SinBetween(C VecD2 &a, C VecD2 &b) {if(Dbl l2=a.length2()*b.length2())return          Cross(a, b)/ SqrtFast(l2); return 0;}
Flt AbsSinBetween(C Vec   &a, C Vec   &b) {if(Flt l2=a.length2()*b.length2())return SqrtFast(Cross(a, b).length2()/l2); return 0;}
Dbl AbsSinBetween(C VecD  &a, C VecD  &b) {if(Dbl l2=a.length2()*b.length2())return SqrtFast(Cross(a, b).length2()/l2); return 0;}

Flt    AngleBetween (C Vec2  &a, C Vec2  &b) {return AngleDelta(Angle(a), Angle(b));}
Dbl    AngleBetween (C VecD2 &a, C VecD2 &b) {return AngleDelta(Angle(a), Angle(b));}
Flt AbsAngleBetween (C Vec   &a, C Vec   &b) {return Angle  (CosBetweenN(a, b), AbsSinBetweenN(a, b));} // we don't need to do any scaling because 'CosBetweenN' and 'AbsSinBetweenN' will have proportional lengths, and we use 'Angle' which doesn't require lengthts to be normalized
Dbl AbsAngleBetween (C VecD  &a, C VecD  &b) {return Angle  (CosBetweenN(a, b), AbsSinBetweenN(a, b));} // we don't need to do any scaling because 'CosBetweenN' and 'AbsSinBetweenN' will have proportional lengths, and we use 'Angle' which doesn't require lengthts to be normalized
Flt AbsAngleBetweenN(C Vec   &a, C Vec   &b) {return ACosSin(CosBetweenN(a, b), AbsSinBetweenN(a, b));}
Dbl AbsAngleBetweenN(C VecD  &a, C VecD  &b) {return ACosSin(CosBetweenN(a, b), AbsSinBetweenN(a, b));}

Flt AngleBetween(C Vec  &a, C Vec  &b, C Vec  &z) {Flt angle=AbsAngleBetween(a, b); if(Dot(Cross(a, b), z)<0)CHS(angle); return angle;}
Dbl AngleBetween(C VecD &a, C VecD &b, C VecD &z) {Dbl angle=AbsAngleBetween(a, b); if(Dot(Cross(a, b), z)<0)CHS(angle); return angle;}
/******************************************************************************/
Vec DequantizeNormal(C Vec &n)
{
   Flt z=CalcZ(n.xy);

   Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, n.xy.length())); // error 5232, errorNoNormalize 1583174
 //Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, CosSin(z))); // error 5232, errorNoNormalize 1583103

 //Vec dn(n.x, n.y, Lerp(n.z, (n.z>=0) ? z : -z, Sqr(z))); // error 5425, errorNoNormalize 1410261
 //Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, 1-Sqr(z))); // error 5425, errorNoNormalize 1410261

 //Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, n.xy.length2())); // error 5426, errorNoNormalize 1410402

 //Vec dn(n.x, n.y, Lerp(n.z, (n.z>=0) ? z : -z, z)); // error 7035, errorNoNormalize 1101598
 //Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, 1-z)); // error 7035, errorNoNormalize 1101598

 //Vec dn(n.x, n.y, Lerp((n.z>=0) ? z : -z, n.z, Sqr(1-z))); // error 10519, errorNoNormalize 793122

 //Vec dn(n.x, n.y, (n.z>=0) ? z : -z); // error 120477, errorNoNormalize 144597

   dn.normalize();
   return dn;
}
/******************************************************************************/
// INTERPOLATION
/******************************************************************************/
#define TANGENT_FULL 0.817538f
#define TANGENT      0.5f
/******************************************************************************/
Flt  GetTangent   (  Flt   prev,   Flt   next) {return (next-prev)*TANGENT;}
Vec2 GetTangent   (C Vec2 &prev, C Vec2 &next) {return (next-prev)*TANGENT;}
Vec  GetTangent   (C Vec  &prev, C Vec  &next) {return (next-prev)*TANGENT;}
Vec4 GetTangent   (C Vec4 &prev, C Vec4 &next) {return (next-prev)*TANGENT;}
Vec2 GetTangentDir(C Vec2 &prev, C Vec2 &next) {return (next-prev)*Lerp(TANGENT, TANGENT_FULL, Sqr(AbsAngleBetween(prev, next)/PI));}
Vec  GetTangentDir(C Vec  &prev, C Vec  &next) {return (next-prev)*Lerp(TANGENT, TANGENT_FULL, Sqr(AbsAngleBetween(prev, next)/PI));}

Flt  GetTangent(  Flt   prev,   Flt   cur,   Flt   next) {return GetTangent   (prev    , next    );}
Vec2 GetTangent(C Vec2 &prev, C Vec2 &cur, C Vec2 &next) {return GetTangentDir(prev-cur, next-cur);}
Vec  GetTangent(C Vec  &prev, C Vec  &cur, C Vec  &next) {return GetTangentDir(prev-cur, next-cur);}
Vec4 GetTangent(C Vec4 &prev, C Vec4 &cur, C Vec4 &next) {return GetTangent   (prev    , next    );}
/******************************************************************************/
VecB4 Lerp(C VecB4 &from, C VecB4 &to, Flt step)
{
   Flt step1=1-step;
   return VecB4(Mid(RoundPos(from.x*step1 + to.x*step), 0, 255),
                Mid(RoundPos(from.y*step1 + to.y*step), 0, 255),
                Mid(RoundPos(from.z*step1 + to.z*step), 0, 255),
                Mid(RoundPos(from.w*step1 + to.w*step), 0, 255));
}
VecB4 Lerp(C VecB4 &a, C VecB4 &b, C VecB4 &c, C Vec &blend)
{
   return VecB4(Mid(RoundPos(a.x*blend.x + b.x*blend.y + c.x*blend.z), 0, 255),
                Mid(RoundPos(a.y*blend.x + b.y*blend.y + c.y*blend.z), 0, 255),
                Mid(RoundPos(a.z*blend.x + b.z*blend.y + c.z*blend.z), 0, 255),
                Mid(RoundPos(a.w*blend.x + b.w*blend.y + c.w*blend.z), 0, 255));
}
/******************************************************************************/
Flt LerpR(C Vec2  &from, C Vec2  &to, C Vec2  &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
Dbl LerpR(C VecD2 &from, C VecD2 &to, C VecD2 &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
Flt LerpR(C Vec   &from, C Vec   &to, C Vec   &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
Dbl LerpR(C VecD  &from, C VecD  &to, C VecD  &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
Flt LerpR(C Vec4  &from, C Vec4  &to, C Vec4  &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
Dbl LerpR(C VecD4 &from, C VecD4 &to, C VecD4 &value) {Int i=Abs(to-from).maxI(); return LerpR(from.c[i], to.c[i], value.c[i]);}
/******************************************************************************/
Flt LerpAngle(Flt from, Flt to, Flt step) {return step*AngleDelta(from, to)+from;}
/******************************************************************************/
void Lerp4Weights(Vec4 &weights, Flt step)
{
   Flt s =step,
       s2=step*step,
       s3=step*step*step;
   weights.x=((  -TANGENT)*(s3+s ) + (2*TANGENT  )*s2                );
   weights.y=(( 2-TANGENT)* s3     + (  TANGENT-3)*s2             + 1);
 //weights.z=((-2+TANGENT)* s3     - (2*TANGENT-3)*s2 + TANGENT*s    ); don't calculate it manually, use the fact that the sum is always equal to 1, using Z component was the fastest version tested
   weights.w=(    TANGENT *(s3-s2)                                   );
   weights.z=1-weights.x-weights.y-weights.w;
}
/******************************************************************************/
Flt Lerp4(Flt v0, Flt v1, Flt v2, Flt v3, Flt step)
{
   Vec4 weights; Lerp4Weights(weights, step); return v0*weights.x + v1*weights.y + v2*weights.z + v3*weights.w;
}
Vec2 Lerp4(C Vec2 &v0, C Vec2 &v1, C Vec2 &v2, C Vec2 &v3, Flt step)
{
   Vec4 weights; Lerp4Weights(weights, step); return v0*weights.x + v1*weights.y + v2*weights.z + v3*weights.w;
}
Vec Lerp4(C Vec &v0, C Vec &v1, C Vec &v2, C Vec &v3, Flt step)
{
   Vec4 weights; Lerp4Weights(weights, step); return v0*weights.x + v1*weights.y + v2*weights.z + v3*weights.w;
}
Vec4 Lerp4(C Vec4 &v0, C Vec4 &v1, C Vec4 &v2, C Vec4 &v3, Flt step)
{
   Vec4 weights; Lerp4Weights(weights, step); return v0*weights.x + v1*weights.y + v2*weights.z + v3*weights.w;
}
/******************************************************************************/
Flt LerpTan(Flt from, Flt to, Flt step, Flt tan0, Flt tan1)
{
   return step*step*step * ((from-to)* 2 +   tan0 + tan1)
        + step*step      * ((from-to)*-3 - 2*tan0 - tan1)
        + step           * (                 tan0       )
        + from;
}
Vec2 LerpTan(C Vec2 &from, C Vec2 &to, Flt step, C Vec2 &tan0, C Vec2 &tan1)
{
   Flt s2=step*step,
       s3=step*step*step;
   return from
       + (from-to) * (2*s3 - 3*s2       )
       +  tan0     * (  s3 - 2*s2 + step)
       +  tan1     * (  s3 -   s2       );
}
Vec LerpTan(C Vec &from, C Vec &to, Flt step, C Vec &tan0, C Vec &tan1)
{
   Flt s2=step*step,
       s3=step*step*step;
   return from
       + (from-to) * (2*s3 - 3*s2       )
       +  tan0     * (  s3 - 2*s2 + step)
       +  tan1     * (  s3 -   s2       );
}
Vec4 LerpTan(C Vec4 &from, C Vec4 &to, Flt step, C Vec4 &tan0, C Vec4 &tan1)
{
   Flt s2=step*step,
       s3=step*step*step;
   return from
       + (from-to) * (2*s3 - 3*s2       )
       +  tan0     * (  s3 - 2*s2 + step)
       +  tan1     * (  s3 -   s2       );
}
/******************************************************************************/
// ADJUST VALUE
/******************************************************************************/
void AdjustValDir(Flt &value, Int dir, Flt dv)
{
   if(dir) // adjust by direction
   {
      value+=dir*dv;
      Clamp(value, -1, 1);
   }else
   if(Int s=Sign(value)) // move towards zero
   {
      value-=s*dv;
      if(Sign(value)!=s)value=0;
   }
}
void AdjustValDir(Flt &value, Int dir, Flt change, Flt reset)
{
   if(dir) // adjust by direction
   {
      value+=dir*change;
      Clamp(value, -1, 1);
   }else
   if(Int s=Sign(value)) // move towards zero
   {
      value-=s*reset;
      if(Sign(value)!=s)value=0;
   }
}
/******************************************************************************/
void AdjustValBool(Flt &value, Bool on, Flt dv)
{
   if(on)value+=dv; // increase
   else  value-=dv; // decrease
   SAT(value);      // saturate
}
void AdjustValBool(Flt &value, Bool on, Flt inc, Flt dec)
{
   if(on)value+=inc; // increase
   else  value-=dec; // decrease
   SAT(value);       // saturate
}
/******************************************************************************/
void AdjustValBoolSet(Flt &value, Bool on, Bool set, Flt dv          ) {if(set)value=on;else AdjustValBool(value, on, dv      );} // immediately set or smoothly adjust
void AdjustValBoolSet(Flt &value, Bool on, Bool set, Flt inc, Flt dec) {if(set)value=on;else AdjustValBool(value, on, inc, dec);} // immediately set or smoothly adjust
/******************************************************************************/
void AdjustValTime(Flt  &value,   Flt   target, Flt exponent, Flt dt) {value=Pow(exponent, dt)*(value-target)+target;} // smoothly adjust value to 'target' according to 'exponent' and 'time delta'
void AdjustValTime(Dbl  &value,   Dbl   target, Flt exponent, Flt dt) {value=Pow(exponent, dt)*(value-target)+target;} // smoothly adjust value to 'target' according to 'exponent' and 'time delta'
void AdjustValTime(Vec2 &value, C Vec2 &target, Flt exponent, Flt dt) {value=Pow(exponent, dt)*(value-target)+target;} // smoothly adjust value to 'target' according to 'exponent' and 'time delta'
void AdjustValTime(Vec  &value, C Vec  &target, Flt exponent, Flt dt) {value=Pow(exponent, dt)*(value-target)+target;} // smoothly adjust value to 'target' according to 'exponent' and 'time delta'
void AdjustValTime(VecD &value, C VecD &target, Flt exponent, Flt dt) {value=Pow(exponent, dt)*(value-target)+target;} // smoothly adjust value to 'target' according to 'exponent' and 'time delta'
/******************************************************************************/
void AdjustValTarget(Flt &value, Flt target, Flt dv)
{
   if(value>target){value-=dv; if(value<target)value=target;}else // move towards 'target' by increasing 'value' by 'dv'
   if(value<target){value+=dv; if(value>target)value=target;}     // move towards 'target' by decreasing 'value' by 'dv'
}
void AdjustValTarget(Dbl &value, Dbl target, Dbl dv)
{
   if(value>target){value-=dv; if(value<target)value=target;}else // move towards 'target' by increasing 'value' by 'dv'
   if(value<target){value+=dv; if(value>target)value=target;}     // move towards 'target' by decreasing 'value' by 'dv'
}
void AdjustValTarget(Flt &value, Flt target, Flt inc, Flt dec)
{
   if(value>target){value-=dec; if(value<target)value=target;}else // move towards 'target' by increasing 'value' by 'inc'
   if(value<target){value+=inc; if(value>target)value=target;}     // move towards 'target' by decreasing 'value' by 'dec'
}
void AdjustValTarget(Vec2 &value, C Vec2 &target, Flt dv)
{
   AdjustValTarget(value.x, target.x, dv);
   AdjustValTarget(value.y, target.y, dv);
}
void AdjustValTarget(Vec &value, C Vec &target, Flt dv)
{
   AdjustValTarget(value.x, target.x, dv);
   AdjustValTarget(value.y, target.y, dv);
   AdjustValTarget(value.z, target.z, dv);
}
void AdjustValTarget(VecD &value, C VecD &target, Dbl dv)
{
   AdjustValTarget(value.x, target.x, dv);
   AdjustValTarget(value.y, target.y, dv);
   AdjustValTarget(value.z, target.z, dv);
}
/******************************************************************************/
// SMOOTH OFFSET
/******************************************************************************/
Flt SmoothOffset(Flt &offset, Flt max_length)
{
   Flt ofs=offset; if(Flt length=Abs(ofs))
   {
      ofs=Sign(ofs);
      const Flt factor=1.0f/PI_2; // mul by this because we're operating on 'Sin' below, and for low values "Sin(x*PI_2)" is equal to "x*PI_2/PI_2" (tested via "Functions" tool)
      length*=factor;
      if(length>max_length){offset=ofs*(max_length/factor); length=max_length;}
      Flt frac=length/max_length;
      frac=Sin(frac*PI_2);
      ofs*=max_length*frac;
   }
   return ofs;
}
Vec2 SmoothOffset(Vec2 &offset, Flt max_length)
{
   Vec2 ofs=offset; if(Flt length=ofs.normalize())
   {
      const Flt factor=1.0f/PI_2; // mul by this because we're operating on 'Sin' below, and for low values "Sin(x*PI_2)" is equal to "x*PI_2/PI_2" (tested via "Functions" tool)
      length*=factor;
      if(length>max_length){offset=ofs*(max_length/factor); length=max_length;}
      Flt frac=length/max_length;
      frac=Sin(frac*PI_2);
      ofs*=max_length*frac;
   }
   return ofs;
}
Vec SmoothOffset(Vec &offset, Flt max_length)
{
   Vec ofs=offset; if(Flt length=ofs.normalize())
   {
      const Flt factor=1.0f/PI_2; // mul by this because we're operating on 'Sin' below, and for low values "Sin(x*PI_2)" is equal to "x*PI_2/PI_2" (tested via "Functions" tool)
      length*=factor;
      if(length>max_length){offset=ofs*(max_length/factor); length=max_length;}
      Flt frac=length/max_length;
      frac=Sin(frac*PI_2);
      ofs*=max_length*frac;
   }
   return ofs;
}
/******************************************************************************/
// SMOOTH CURVES
/******************************************************************************/
Flt SmoothSqr(Flt x)
{
   if(x<=0   )return 0;
   if(x>=1   )return 1;
   if(x<=0.5f)return   2*Sqr(  x);
              return 1-2*Sqr(1-x); // 1-2*Sqr(1-x) -> 1-2*(1+x*x-2*x) -> 1 - 2 - 2*x*x + 4*x -> -2*x*x + 4*x - 1 (however this formula has more operations)
}
Flt SmoothCube(Flt x)
{
   if(x<=0)return 0;
   if(x>=1)return 1;
           return _SmoothCube(x);
}
Flt SmoothCubeInv(Flt y)
{
   if(y<=0)return 0;
   if(y>=1)return 1;
   return 0.5f-Sin(asinf(1-2*y)/3);
}
Flt SmoothCube2(Flt x)
{
   if(x<=0   )return 0;
   if(x>=1   )return 1;
   if(x<=0.5f)return   4*Cube(  x);
              return 1-4*Cube(1-x);
}
Flt _SmoothQuintic(Flt x)
{
   return x*x*x*(x*(x*6-15)+10);
}
Flt SmoothSextic(Flt x)
{
   if(x<=0)return 0;
   if(x>=1)return 1;
   x*=x;
   return (4.0f/9)*x*x*x - (17.0f/9)*x*x + (22.0f/9)*x;
}
Flt SmoothSin(Flt x)
{
   if(x<=0)return 0;
   if(x>=1)return 1;
           return 0.5f-0.5f*Cos(x*PI);
}
Flt SmoothPow(Flt x, Flt pow)
{
   if(x<=0   )return 0;
   if(x>=1   )return 1;
   if(x<=0.5f)return   0.5f*Pow(  2*x, pow);
              return 1-0.5f*Pow(2-2*x, pow);
}
Flt SmoothPinch(Flt x, Flt pinch)
{
   if(x<=0   )return 0;
   if(x>=1   )return 1;
   if(x<=0.5f)return   0.5f*Pinch(  2*x, pinch);
              return 1-0.5f*Pinch(2-2*x, pinch);
}
/******************************************************************************/
// SIGMOID
/******************************************************************************/
Flt gd(Flt x) {return Atan(sinhf(x));}
Dbl gd(Dbl x) {return Atan(sinh (x));}

Flt SigmoidExp    (Flt x) {return 2/(1+expf(-x))-1;}
Flt SigmoidDiv    (Flt x) {return x/(1+x);}
Flt SigmoidAtan   (Flt x) {return Atan(PI_2*x)*2/PI;}
Flt SigmoidSqrt   (Flt x) {return x/SqrtFast(1+x*x);}
Flt SigmoidSqrtInv(Flt y) {return y/SqrtFast(1-y*y);}
Flt SigmoidGd     (Flt x) {return (2/PI)*gd(PI_2*x);}
Flt SigmoidTanh   (Flt x) {return tanhf(x);}
Flt SigmoidErf    (Flt x) {return erff((SqrtFast(PI)/2)*x);}
/******************************************************************************/
// BLENDING
/******************************************************************************/
Flt BlendSqr(Flt x)
{
   x*=x;
   return (x<1) ? 1-x : 0;
}
Flt BlendSmoothSqr(Flt x)
{
   ABS(x);
   return (x<=0.5f) ? 1-2*Sqr(  x)
        : (x< 1   ) ?   2*Sqr(1-x)
        :                        0;
}
Flt BlendSmoothCube(Flt x) // !! avoid changing this formula, but if it's changed, then adjust 'BlendSmoothCubeSum' and 'BlendSmoothCubeSumHalf' !!
{
   ABS(x);
   return (x<1) ? 1-(3-2*x)*x*x : 0;
}
Flt BlendSmoothSextic(Flt x)
{
   x*=x;
   return (x<1) ? (-4.0f/9)*x*x*x + (17.0f/9)*x*x - (22.0f/9)*x + 1 : 0;
}
Flt BlendSmoothSin(Flt x)
{
   ABS(x);
   return (x<1) ? Cos(x*PI)*0.5f+0.5f : 0;
}
/******************************************************************************/
Flt Gaussian(Flt x)
{
    return expf(-x*x);
}
/******************************************************************************/
// EQUATIONS
/******************************************************************************/
Int Polynominal1(Flt a, Flt b,        Flt &x          ) {if(!a)return Polynominal0(b); x=-b/a; return 1;}
Int Polynominal1(Dbl a, Dbl b,        Dbl &x          ) {if(!a)return Polynominal0(b); x=-b/a; return 1;}
Int Polynominal2(Flt a, Flt b, Flt c, Flt &x0, Flt &x1)
{
   if(!a)return Polynominal1(b, c, x0);
   Flt delta=b*b-4*a*c;
   if( delta<0)return 0; a+=a;
   if(!delta){x0= -b       /a; return 1;} delta=Sqrt(delta);
              x0=(-b-delta)/a;
              x1=(-b+delta)/a; return 2;
}
Int Polynominal2(Dbl a, Dbl b, Dbl c, Dbl &x0, Dbl &x1)
{
   if(!a)return Polynominal1(b, c, x0);
   Dbl delta=b*b-4*a*c;
   if( delta<0)return 0; a+=a;
   if(!delta){x0= -b       /a; return 1;} delta=Sqrt(delta);
              x0=(-b-delta)/a;
              x1=(-b+delta)/a; return 2;
}
Int Polynominal3(Flt a, Flt b, Flt c, Flt d, Flt &x0, Flt &x1, Flt &x2)
{
   if(!a)return Polynominal2(b, c, d, x0, x1);

   Flt f=c/a - b*b/(a*a*3),
       g=2*b*b*b/(a*a*a*27) - b*c/(a*a*3) + d/a,
       h=g*g/4 + f*f*f/27;
   if(h>0)
   {
      g/=-2;
      h =Sqrt(h);
      x0=Cbrt(g+h)+Cbrt(g-h) - b/(3*a);
      return 1;
   }else
   if(!f && !g && !h)
   {
      x0=x1=x2=-Cbrt(d/a);
      return 3;
   }else
   if(h<=0)
   {
      Flt i= g*g/4 - h,
          p=-b/(3*a),
          m, n; CosSin(m, n, Acos(-0.5f*g/Sqrt(i))/3);
      i=Pow(i, 1.0f/6); n*=SQRT3;
      x0=2*i* m   +p;
      x1= -i*(m+n)+p;
      x2= -i*(m-n)+p;
      return 3;
   }
   return 0;
}
Int Polynominal3(Dbl a, Dbl b, Dbl c, Dbl d, Dbl &x0, Dbl &x1, Dbl &x2)
{
   if(!a)return Polynominal2(b, c, d, x0, x1);

   Dbl f=c/a - b*b/(a*a*3),
       g=2*b*b*b/(a*a*a*27) - b*c/(a*a*3) + d/a,
       h=g*g/4 + f*f*f/27;
   if(h>0)
   {
      g/=-2;
      h =Sqrt(h);
      x0=Cbrt(g+h)+Cbrt(g-h) - b/(3*a);
      return 1;
   }else
   if(!f && !g && !h)
   {
      x0=x1=x2=-Cbrt(d/a);
      return 3;
   }else
   if(h<=0)
   {
      Dbl i= g*g/4 - h,
          p=-b/(3*a),
          m, n; CosSin(m, n, Acos(-0.5*g/Sqrt(i))/3);
      i=Pow(i, 1.0/6); n*=SQRT3;
      x0=2*i* m   +p;
      x1= -i*(m+n)+p;
      x2= -i*(m-n)+p;
      return 3;
   }
   return 0;
}
/******************************************************************************/
Int Solve(Flt a1, Flt a2, Flt b1, Flt b2, Flt c1, Flt c2, Flt &x, Flt &y)
{
   // x*a1 + y*b1 = c1
   // x*a2 + y*b2 = c2

   // W = | a1 b1 | = a1*b2 - b1*a2,   Wx = | c1 b1 | = c1*b2 - c2*b1,   Wy = | a1 c1 | = a1*c2 - a2*c1
   //     | a2 b2 |                         | c2 b2 |                         | a2 c2 |

   if(Flt W=a1*b2 - b1*a2)
   {
      Flt Wx=c1*b2 - c2*b1,
          Wy=a1*c2 - a2*c1;
      if(!Wx && !Wy)return -1;
      x=Wx/W;
      y=Wy/W;
      return 1;
   }
   return 0;
}
Int Solve(Dbl a1, Dbl a2, Dbl b1, Dbl b2, Dbl c1, Dbl c2, Dbl &x, Dbl &y)
{
   // x*a1 + y*b1 = c1
   // x*a2 + y*b2 = c2

   // W = | a1 b1 | = a1*b2 - b1*a2,   Wx = | c1 b1 | = c1*b2 - c2*b1,   Wy = | a1 c1 | = a1*c2 - a2*c1
   //     | a2 b2 |                         | c2 b2 |                         | a2 c2 |

   if(Dbl W=a1*b2 - b1*a2)
   {
      Dbl Wx=c1*b2 - c2*b1,
          Wy=a1*c2 - a2*c1;
      if(!Wx && !Wy)return -1;
      x=Wx/W;
      y=Wy/W;
      return 1;
   }
   return 0;
}
/******************************************************************************

   64 BIT FLOAT - S EEEEEEEEEEE FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF (S-1 bit Sign, E-11 bit Exponent, F-52 bit Fraction)
   32 BIT FLOAT - S EEEEEEEE    FFFFFFFFFFFFFFFFFFFFFFF                              (S-1 bit Sign, E- 8 bit Exponent, F-23 bit Fraction)
   16 BIT FLOAT - S EEEEE       FFFFFFFFFF                                           (S-1 bit Sign, E- 5 bit Exponent, F-10 bit Fraction)
   
   32:
      Exponent | Fraction Zero | Fraction Non-Zero | Equation
      0        |     +0 -0     | subnormal numbers | Pow(-1, sign) * Pow(2,         -126) * 0.Fraction
      1..254   |     value     |        value      | Pow(-1, sign) * Pow(2, exponent-127) * 1.Fraction
         255   |   +Inf -Inf   |        NaN        |

   16:
      Exponent | Fraction Zero | Fraction Non-Zero | Equation
      0        |     +0 -0     | subnormal numbers | Pow(-1, sign) * Pow(2,         -14) * 0.Fraction
      1..30    |     value     |        value      | Pow(-1, sign) * Pow(2, exponent-15) * 1.Fraction
         31    |   +Inf -Inf   |        NaN        |

/******************************************************************************/
Half::operator Flt()C
{
   UInt f;
   Int  e=(data&0x7C00);
   if(e==     0)f=((data&0x8000)<<16);else
   if(e==0x7C00)f=((data&0x8000)<<16) | (0xFF       <<23) | ((data&0x03FF)<<13);else
                f=((data&0x8000)<<16) | ((e+0x1C000)<<13) | ((data&0x03FF)<<13); // (127-15)<<10 = 0x1C000
   return (Flt&)f;
}
/*Half::Half(Bool b) {data=(b ? 15360 : 0);}
Half::Half(Int  i) : Half(Flt(i)) {}
Half::Half(UInt i) : Half(Flt(i)) {}*/
Half::Half(Flt  f)
{
   UInt u=(U32&)f;
// Int  e=((u>>23)&0xFF)+(15-127);
// if(e<= 0)data= (u>>16)&0x8000;else
// if(e> 31)data=((u>>16)&0x8000) | (31<<10) | ((u>>13)&0x03FF);else
//          data=((u>>16)&0x8000) | ( e<<10) | ((u>>13)&0x03FF);
   Int e =(u&0x7F800000);
   if( e<=   0x38000000)data= (u>>16)&0x8000;else
   if( e>=   0x47800000)data=((u>>16)&0x8000) | (           31 <<10) | ((u>>13)&0x03FF);else
                        data=((u>>16)&0x8000) | ((e-0x38000000)>>13) | ((u>>13)&0x03FF);
}
/******************************************************************************/
void DecRealByBit(Flt &r)
{
   UInt u   =(U32&)r,
        f   =(u&0x7FFFFF),
        exp =((u>>23)&0xFF);
   Bool sign=(u>>31);
   if( !sign)
   {
      f--;
      f&=0x7FFFFF;
      switch(f)
      {
         case 0x7FFFFF: if(exp)exp--;else{sign=true; /*exp=0;*/ f=1;} break; // -eps
      }
   }else
   if(exp<0xFF)
   {
      f++;
      f&=0x7FFFFF;
      if(!f)exp++;
   }
   (U32&)r=(sign<<31)|(exp<<23)|f;
}
void IncRealByBit(Flt &r)
{
   UInt u   =(U32&)r,
        f   =(u&0x7FFFFF),
        exp =((u>>23)&0xFF);
   Bool sign=(u>>31);
   if(  sign)
   {
      f--;
      f&=0x7FFFFF;
      switch(f)
      {
         case 0x7FFFFF: if(exp)exp--;else{sign=false; /*exp=0;*/ f=1;} break; // eps
         case 0x000000:          if(!exp){sign=false; /*exp=0;*/ f=0;} break; // fix -0 -> 0
      }
   }else
   if(exp<0xFF)
   {
      f++;
      f&=0x7FFFFF;
      if(!f)exp++;
   }
   (U32&)r=(sign<<31)|(exp<<23)|f;
}
void DecRealByBit(Dbl &r)
{
   ULong u   =(U64&)r,
         f   =(u&0xFFFFFFFFFFFFF),
         exp =((u>>52)&0x7FF),
         sign=(u>>63);
   if(  !sign)
   {
      f--;
      f&=0xFFFFFFFFFFFFF;
      switch(f)
      {
         case 0xFFFFFFFFFFFFF: if(exp)exp--;else{sign=true; /*exp=0;*/ f=1;} break; // -eps
      }
   }else
   if(exp<0x7FF)
   {
      f++;
      f&=0xFFFFFFFFFFFFF;
      if(!f)exp++;
   }
   (U64&)r=(sign<<63)|(exp<<52)|f;
}
void IncRealByBit(Dbl &r)
{
   ULong u   =(U64&)r,
         f   =(u&0xFFFFFFFFFFFFF),
         exp =((u>>52)&0x7FF),
         sign=(u>>63);
   if(   sign)
   {
      f--;
      f&=0xFFFFFFFFFFFFF;
      switch(f)
      {
         case 0xFFFFFFFFFFFFF: if(exp)exp--;else{sign=false; /*exp=0;*/ f=1;} break; // eps
         case 0x0000000000000:          if(!exp){sign=false; /*exp=0;*/ f=0;} break; // fix -0 -> 0
      }
   }else
   if(exp<0x7FF)
   {
      f++;
      f&=0xFFFFFFFFFFFFF;
      if(!f)exp++;
   }
   (U64&)r=(sign<<63)|(exp<<52)|f;
}
/******************************************************************************/
// Standard Deviational Ellipse
// http://resources.esri.com/help/9.3/arcgisengine/java/gp_toolref/spatial_statistics_tools/how_directional_distribution_colon_standard_deviational_ellipse_spatial_statistics_works.htm
void AvgDirU(Vec2 &dir, C MemPtr<Vec2> &points)
{
   Flt x2=0, y2=0, xy=0;
   REPA(points)
   {
    C Vec2 &p=points[i];
      x2+=Sqr(p.x);
      y2+=Sqr(p.y);
      xy+=p.x*p.y;
   }
   Flt a=x2-y2,
       c=2*xy,
       b=Dist(a, c);
   dir.set(a+b, c); // if we would normalize, then dir.x=cos, dir.y=sin
}
/*
void AvgDirU(Vec  &dir, C MemPtr<Vec > &points); // get average direction from array of points, points should be located around center Vec (0,0,0), direction is not normalized
void AvgDirU(Vec &dir, C MemPtr<Vec> &points)
{
   this is wrong
   Flt x2=0, y2=0, z2=0, xy=0, xz=0;
   REPA(points)
   {
    C Vec &p=points[i];
      x2+=Sqr(p.x);
      y2+=Sqr(p.y);
      z2+=Sqr(p.z);
      xy+=p.x*p.y;
      xz+=p.x*p.z;
   }
   Flt a=x2-y2,
       c=2*xy,
       b=Dist(a, c);
   dir.x=a+b; dir.y=c;
   dir.z=c;
}*/
/******************************************************************************
void MathAssert()
{
   DEBUG_ASSERT(FloorSpecial(0)==-1 && FloorSpecial(0.01f)==0 && FloorSpecial(0.5f)==0 && FloorSpecial(0.99f)==0 && FloorSpecial(1.0f)==0, "FloorSpecial");
   DEBUG_ASSERT( CeilSpecial(0)== 1 &&  CeilSpecial(0.01f)==1 &&  CeilSpecial(0.5f)==1 &&  CeilSpecial(0.99f)==1 &&  CeilSpecial(1.0f)==2,  "CeilSpecial");
}
/******************************************************************************/
}
/******************************************************************************/
