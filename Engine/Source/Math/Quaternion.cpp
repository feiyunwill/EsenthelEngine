/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Quaternion& Quaternion::operator*=(Flt f)
{
   x*=f;
   y*=f;
   z*=f;
   w*=f;
   return T;
}
/******************************************************************************/
void Quaternion::mul(C Quaternion &q, Quaternion &dest)C
{
   dest.set(y*q.z - z*q.y + w*q.x + x*q.w,
            z*q.x - x*q.z + w*q.y + y*q.w,
            x*q.y - y*q.x + w*q.z + z*q.w,
            w*q.w - x*q.x - y*q.y - z*q.z);
}
void Quaternion::inverse(Quaternion &dest)C
{
   dest.set(-x, -y, -z, w);
   dest/=length2();
}
/******************************************************************************/
Quaternion& Quaternion::setRotateX(Flt angle          ) {CosSin(w, x, angle*-0.5f); y=z=0; return T;}
Quaternion& Quaternion::setRotateY(Flt angle          ) {CosSin(w, y, angle*-0.5f); x=z=0; return T;}
Quaternion& Quaternion::setRotateZ(Flt angle          ) {CosSin(w, z, angle*-0.5f); x=y=0; return T;}
Quaternion& Quaternion::setRotate (Flt x, Flt y, Flt z)
{
   Flt cos_x_2, sin_x_2; CosSin(cos_x_2, sin_x_2, x*-0.5f);
   Flt cos_y_2, sin_y_2; CosSin(cos_y_2, sin_y_2, y*-0.5f);
   Flt cos_z_2, sin_z_2; CosSin(cos_z_2, sin_z_2, z*-0.5f);

	T.x = cos_z_2*cos_y_2*sin_x_2 - sin_z_2*sin_y_2*cos_x_2;
	T.y = cos_z_2*sin_y_2*cos_x_2 + sin_z_2*cos_y_2*sin_x_2;
	T.z = sin_z_2*cos_y_2*cos_x_2 - cos_z_2*sin_y_2*sin_x_2;
	T.w = cos_z_2*cos_y_2*cos_x_2 + sin_z_2*sin_y_2*sin_x_2;

   return T;
}
Quaternion& Quaternion::setRotate(C Vec &axis, Flt angle)
{
   Flt sin; CosSin(w, sin, angle*-0.5f);
   xyz=axis*sin;
   return T;
}
/******************************************************************************/
Flt Quaternion::angle()C
{
   return Acos(w)*2;
}
Vec Quaternion::axis()C
{
   Vec O=-xyz; O.normalize(); return O;
}
/******************************************************************************/
Orient::Orient(C Quaternion &q)
{
   Flt xx=q.x*q.x,
       xy=q.x*q.y,
       xz=q.x*q.z,
       xw=q.x*q.w,
       yy=q.y*q.y,
       yz=q.y*q.z,
       yw=q.y*q.w,
       zz=q.z*q.z,
       zw=q.z*q.w;

   perp.x=  2*(xy+zw);
   perp.y=1-2*(xx+zz);
   perp.z=  2*(yz-xw);

   dir.x=  2*(xz-yw);
   dir.y=  2*(yz+xw);
   dir.z=1-2*(xx+yy);

   perp.normalize();
   dir .normalize();
}
Matrix3::Matrix3(C Quaternion &q)
{
   Flt xx=q.x*q.x,
       xy=q.x*q.y,
       xz=q.x*q.z,
       xw=q.x*q.w,
       yy=q.y*q.y,
       yz=q.y*q.z,
       yw=q.y*q.w,
       zz=q.z*q.z,
       zw=q.z*q.w;

   T.x.x=1-2*(yy+zz);
   T.x.y=  2*(xy-zw);
   T.x.z=  2*(xz+yw);

   T.y.x=  2*(xy+zw);
   T.y.y=1-2*(xx+zz);
   T.y.z=  2*(yz-xw);

   T.z.x=  2*(xz-yw);
   T.z.y=  2*(yz+xw);
   T.z.z=1-2*(xx+yy);

   normalize();
}
/******************************************************************************/
Quaternion::Quaternion(C Matrix3 &m)
{
   Flt f=m.x.x+m.y.y+m.z.z+1;
   if( f>0.4f)
   {
      w=0.5f *Sqrt(f);
      f=0.25f/w;
	   x=(m.z.y-m.y.z)*f;
	   y=(m.x.z-m.z.x)*f;
	   z=(m.y.x-m.x.y)*f;
   }else
   if(m.x.x>m.y.y && m.x.x>m.z.z)
   {
      x=-0.5f *Sqrt(1+m.x.x-m.y.y-m.z.z);
      f= 0.25f/x;
      y=(m.x.y+m.y.x)*f;
      z=(m.x.z+m.z.x)*f;
      w=(m.z.y-m.y.z)*f;
   }else
   if(m.y.y>m.z.z)
   {
      y=-0.5f *Sqrt(1+m.y.y-m.x.x-m.z.z);
      f= 0.25f/y;
      x=(m.x.y+m.y.x)*f;
      z=(m.y.z+m.z.y)*f;
      w=(m.x.z-m.z.x)*f;
   }else
   {
      z=-0.5f *Sqrt(1+m.z.z-m.x.x-m.y.y);
      f= 0.25f/z;
      x=(m.x.z+m.z.x)*f;
      y=(m.y.z+m.z.y)*f;
      w=(m.y.x-m.x.y)*f;
   }
}
/******************************************************************************/
static Quaternion Log(C Quaternion &q)
{
   Flt length=q.xyz.length();
   if( length<=EPS)return Vec4(q.xyz                   , 0);
   else            return Vec4(q.xyz*(Acos(q.w)/length), 0);
}
static Quaternion Exp(C Quaternion &q)
{
   Flt length=q.xyz.length();
   if( length<=EPS)return Vec4(q.xyz                     , Cos(length));
   else            return Vec4(q.xyz*(Sin(length)/length), Cos(length));
}
Quaternion GetTangent(C Quaternion &prev, C Quaternion &cur, C Quaternion &next)
{
   Quaternion a=cur; a.xyz.chs();
   Quaternion b=a  ;

   a=Log(a*=prev) 
    +Log(b*=next);
   a*=-0.25f;
   Quaternion O; cur.mul(Exp(a), O); return O;
}
/******************************************************************************/
static Quaternion SlerpNoInv(C Quaternion &a, C Quaternion &b, Flt step)
{
   Quaternion O;
   Flt dot=Dot(a, b);
   if(Abs(dot)>=0.99f)O=Lerp(a, b, step);else // if angle is small then use linear interpolation
   {
      Flt angle=Acos(dot  ),
          sin  =Sin (angle);
      O=a*(Sin(angle*(1-step))/sin) + b*(Sin(angle*step)/sin);
   }
   O.normalize();
   return O;
}
/******************************************************************************/
Quaternion Slerp(C Quaternion &a, C Quaternion &b, Flt step)
{
   Quaternion O, temp=b;
   Flt dot=Dot(a, b);
   if( dot<0) // other side
   {
      CHS(dot);
      temp.chs();
   }
   if(dot>=0.99f)O=Lerp(a, temp, step);else // if angle is small then use linear interpolation
   {
      Flt angle=Acos(dot  ),
          sin  =Sin (angle);
      O=a*(Sin(angle*(1-step))/sin) + temp*(Sin(angle*step)/sin);
   }
   O.normalize();
   return O;
}
/******************************************************************************/
Quaternion Squad(C Quaternion &from, C Quaternion &to, C Quaternion &tan0, C Quaternion &tan1, Flt step)
{
   return SlerpNoInv(SlerpNoInv(from, to  , step),
                     SlerpNoInv(tan0, tan1, step), 2*step*(1-step));
}
/******************************************************************************/
}
/******************************************************************************/
