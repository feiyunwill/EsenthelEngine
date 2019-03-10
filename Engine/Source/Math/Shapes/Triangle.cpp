/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Tri2 ::Tri2 (C TriD2 &tri) {p[0]=tri.p[0]; p[1]=tri.p[1]; p[2]=tri.p[2];}
TriD2::TriD2(C Tri2  &tri) {p[0]=tri.p[0]; p[1]=tri.p[1]; p[2]=tri.p[2];}
Tri  ::Tri  (C TriD  &tri) {p[0]=tri.p[0]; p[1]=tri.p[1]; p[2]=tri.p[2]; n=tri.n    ;}
TriD ::TriD (C Tri   &tri) {p[0]=tri.p[0]; p[1]=tri.p[1]; p[2]=tri.p[2]; setNormal();}

Tri2 & Tri2 ::operator+=(C Vec2  &v) {p[0]+=v; p[1]+=v; p[2]+=v; return T;}
Tri2 & Tri2 ::operator-=(C Vec2  &v) {p[0]-=v; p[1]-=v; p[2]-=v; return T;}
TriD2& TriD2::operator+=(C VecD2 &v) {p[0]+=v; p[1]+=v; p[2]+=v; return T;}
TriD2& TriD2::operator-=(C VecD2 &v) {p[0]-=v; p[1]-=v; p[2]-=v; return T;}
Tri  & Tri  ::operator+=(C Vec   &v) {p[0]+=v; p[1]+=v; p[2]+=v; return T;}
Tri  & Tri  ::operator-=(C Vec   &v) {p[0]-=v; p[1]-=v; p[2]-=v; return T;}
TriD & TriD ::operator+=(C VecD  &v) {p[0]+=v; p[1]+=v; p[2]+=v; return T;}
TriD & TriD ::operator-=(C VecD  &v) {p[0]-=v; p[1]-=v; p[2]-=v; return T;}
Tri2 & Tri2 ::operator*=(  Flt    r) {p[0]*=r; p[1]*=r; p[2]*=r; return T;}
Tri2 & Tri2 ::operator/=(  Flt    r) {p[0]/=r; p[1]/=r; p[2]/=r; return T;}
TriD2& TriD2::operator*=(  Dbl    r) {p[0]*=r; p[1]*=r; p[2]*=r; return T;}
TriD2& TriD2::operator/=(  Dbl    r) {p[0]/=r; p[1]/=r; p[2]/=r; return T;}
Tri  & Tri  ::operator*=(  Flt    r) {p[0]*=r; p[1]*=r; p[2]*=r; return T;}
Tri  & Tri  ::operator/=(  Flt    r) {p[0]/=r; p[1]/=r; p[2]/=r; return T;}
TriD & TriD ::operator*=(  Dbl    r) {p[0]*=r; p[1]*=r; p[2]*=r; return T;}
TriD & TriD ::operator/=(  Dbl    r) {p[0]/=r; p[1]/=r; p[2]/=r; return T;}
/******************************************************************************/
Tri& Tri::set(C Vec &p0, C Vec &p1, C Vec &p2, C Vec *nrm)
{
   p[0]=p0;
   p[1]=p1;
   p[2]=p2;
   if(nrm)n=*nrm;else setNormal();
   return T;
}
TriD& TriD::set(C VecD &p0, C VecD &p1, C VecD &p2, C VecD *nrm)
{
   p[0]=p0;
   p[1]=p1;
   p[2]=p2;
   if(nrm)n=*nrm;else setNormal();
   return T;
}
/******************************************************************************/
Tri2& Tri2::setArrow(Flt direction, Flt angle)
{
   CosSin(p[0].x, p[0].y, direction); direction+=PI; angle*=0.5f;
   CosSin(p[1].x, p[1].y, direction+angle);
   CosSin(p[2].x, p[2].y, direction-angle);
   return T;
}
/******************************************************************************/
Flt Tri2 ::area()C {return 0.5f*Abs(Cross(p[1]-p[0], p[2]-p[0]));}
Dbl TriD2::area()C {return 0.5 *Abs(Cross(p[1]-p[0], p[2]-p[0]));}
Flt Tri  ::area()C {return 0.5f*    Cross(p[1]-p[0], p[2]-p[0]).length();}
Dbl TriD ::area()C {return 0.5 *    Cross(p[1]-p[0], p[2]-p[0]).length();}
/******************************************************************************/
Bool Tri2::valid(Flt eps)C
{
   return DistPointStr(p[0], p[1], !(p[2]-p[1]))>eps
       && DistPointStr(p[1], p[2], !(p[0]-p[2]))>eps
       && DistPointStr(p[2], p[0], !(p[1]-p[0]))>eps;
}
Bool Tri::valid(Flt eps)C
{
   return DistPointStr(p[0], p[1], !(p[2]-p[1]))>eps
       && DistPointStr(p[1], p[2], !(p[0]-p[2]))>eps
       && DistPointStr(p[2], p[0], !(p[1]-p[0]))>eps;
}
/******************************************************************************/
Bool Tri2 ::clockwise()C {return Cross(p[1]-p[0], p[2]-p[0])<0;}
Bool TriD2::clockwise()C {return Cross(p[1]-p[0], p[2]-p[0])<0;}
/******************************************************************************/
Bool Tri::coplanar(C Tri &tri)C
{
   return Abs(DistPointPlane(p[0], tri))<=EPS
       && Abs(DistPointPlane(p[1], tri))<=EPS
       && Abs(DistPointPlane(p[2], tri))<=EPS;
}
Bool TriD::coplanar(C TriD &tri)C
{
   return Abs(DistPointPlane(p[0], tri))<=EPSD
       && Abs(DistPointPlane(p[1], tri))<=EPSD
       && Abs(DistPointPlane(p[2], tri))<=EPSD;
}
/******************************************************************************/
void Tri2::circularLerp(Tri2 *tri, Int num)C
{
   if(num>0)
   {
      tri[0    ].p[1]=p[1];
      tri[num-1].p[2]=p[2];
      tri[num-1].p[0]=p[0];
      Flt d01=Dist(p[0], p[1]),
          d02=Dist(p[0], p[2]);
      REP(num-1)
      {
         Flt  s=Flt(i+1)/num;
         Vec2 t=          Lerp(p[1], p[2], s)    - p[0];
              t.setLength(Lerp(d01 , d02 , s)); t+=p[0];
         tri[i  ].p[0]=p[0];
         tri[i  ].p[2]=
         tri[i+1].p[1]=t;
      }
   }
}
void Tri::circularLerp(Tri *tri, Int num)C
{
   if(num>0)
   {
      tri[0    ].p[1]=p[1];
      tri[num-1].p[2]=p[2];
      tri[num-1].p[0]=p[0];
      tri[num-1].n   =n   ;
      Flt d01=Dist(p[0], p[1]),
          d02=Dist(p[0], p[2]);
      REP(num-1)
      {
         Flt s=Flt(i+1)/num;
         Vec t=          Lerp(p[1], p[2], s)    - p[0];
             t.setLength(Lerp(d01 , d02 , s)); t+=p[0];
         tri[i  ].p[0]=p[0];
         tri[i  ].p[2]=
         tri[i+1].p[1]=t;
         tri[i  ].n   =n;
      }
   }
}
/******************************************************************************/
void Tri2::draw(C Color &color, Bool fill)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, fill ? 0 : VI_LINE|VI_STRIP);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(fill ? 3 : 4))
   {
      if(fill)
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
      }else
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
         v[3].pos=p[0];
      }
   }
   VI.end();
}
void TriD2::draw(C Color &color, Bool fill)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, fill ? 0 : VI_LINE|VI_STRIP);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(fill ? 3 : 4))
   {
      if(fill)
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
      }else
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
         v[3].pos=p[0];
      }
   }
   VI.end();
}
void Tri::draw(C Color &color, Bool fill)C
{
   VI.color  (color);
   VI.setType(VI_3D_FLAT, fill ? 0 : VI_LINE|VI_STRIP);
   if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(fill ? 3 : 4))
   {
      if(fill)
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
      }else
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
         v[3].pos=p[0];
      }
   }
   VI.end();
}
void TriD::draw(C Color &color, Bool fill)C
{
   VI.color  (color);
   VI.setType(VI_3D_FLAT, fill ? 0 : VI_LINE|VI_STRIP);
   if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(fill ? 3 : 4))
   {
      if(fill)
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
      }else
      {
         v[0].pos=p[0];
         v[1].pos=p[1];
         v[2].pos=p[2];
         v[3].pos=p[0];
      }
   }
   VI.end();
}
/******************************************************************************/
Vec  GetNormal (C Vec  &p0, C Vec  &p1, C Vec  &p2) {return CrossN(p1-p0, p2-p0);}
VecD GetNormal (C VecD &p0, C VecD &p1, C VecD &p2) {return CrossN(p1-p0, p2-p0);}
Vec  GetNormalU(C Vec  &p0, C Vec  &p1, C Vec  &p2) {return Cross (p1-p0, p2-p0);}
VecD GetNormalU(C VecD &p0, C VecD &p1, C VecD &p2) {return Cross (p1-p0, p2-p0);}
Vec  GetNormalU(            C Vec  &p1, C Vec  &p2) {return Cross (p1   , p2   );}

Flt TriArea2(C Vec  &p0, C Vec  &p1, C Vec  &p2) {return Cross(p1-p0, p2-p0).length();}
Dbl TriArea2(C VecD &p0, C VecD &p1, C VecD &p2) {return Cross(p1-p0, p2-p0).length();}

Vec  GetNormalEdge(C Vec  &p0, C Vec  &p1) // this is called the "Newell" method
{
   return Vec((p0.y-p1.y)*(p0.z+p1.z),
              (p0.z-p1.z)*(p0.x+p1.x),
              (p0.x-p1.x)*(p0.y+p1.y));
}
/******************************************************************************/
Flt TriABAngle(Flt a_length, Flt b_length, Flt c_length)
{
   if(Flt div=2*a_length*b_length)return Acos((Sqr(a_length)+Sqr(b_length)-Sqr(c_length))/div);
   return 0;
}
/******************************************************************************/
Vec TriBlend(C Vec2 &p, C Tri2 &tri)
{
   // blend.x*(x0-x2) + blend.y*(x1-x2) = x-x2
   // blend.x*(y0-y2) + blend.y*(y1-y2) = y-y2

   Vec blend; if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].y-tri.p[2].y, tri.p[1].x-tri.p[2].x, tri.p[1].y-tri.p[2].y, p.x-tri.p[2].x, p.y-tri.p[2].y, blend.x, blend.y)!=1)blend.x=blend.y=0;
   blend.z=1-blend.x-blend.y;
   return blend;
}
VecD TriBlend(C VecD2 &p, C TriD2 &tri)
{
   // blend.x*(x0-x2) + blend.y*(x1-x2) = x-x2
   // blend.x*(y0-y2) + blend.y*(y1-y2) = y-y2

   VecD blend; if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].y-tri.p[2].y, tri.p[1].x-tri.p[2].x, tri.p[1].y-tri.p[2].y, p.x-tri.p[2].x, p.y-tri.p[2].y, blend.x, blend.y)!=1)blend.x=blend.y=0;
   blend.z=1-blend.x-blend.y;
   return blend;
}
Vec TriBlend(C Vec &p, C Tri &tri, Bool pos_on_tri_plane)
{
   if(!pos_on_tri_plane)return TriBlend(PointOnPlane(p, tri.p[0], tri.n), tri, true);

   Vec blend, size=Box(tri).size();
   if(size.z<=size.x && size.z<=size.y) // if Z dimension is smallest
   {
      // use XY plane
      if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].y-tri.p[2].y, tri.p[1].x-tri.p[2].x, tri.p[1].y-tri.p[2].y, p.x-tri.p[2].x, p.y-tri.p[2].y, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }else
   if(size.y<=size.x && size.y<=size.z) // if Y dimension is smallest
   {
      // use XZ plane
      if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].z-tri.p[2].z, tri.p[1].x-tri.p[2].x, tri.p[1].z-tri.p[2].z, p.x-tri.p[2].x, p.z-tri.p[2].z, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }else
   {
      // use YZ plane
      if(Solve(tri.p[0].y-tri.p[2].y, tri.p[0].z-tri.p[2].z, tri.p[1].y-tri.p[2].y, tri.p[1].z-tri.p[2].z, p.y-tri.p[2].y, p.z-tri.p[2].z, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }
   blend.z=1-blend.x-blend.y;
   return blend;
}
VecD TriBlend(C VecD &p, C TriD &tri, Bool pos_on_tri_plane)
{
   if(!pos_on_tri_plane)return TriBlend(PointOnPlane(p, tri.p[0], tri.n), tri, true);

   VecD blend, size=BoxD(tri).size();
   if(size.z<=size.x && size.z<=size.y) // if Z dimension is smallest
   {
      // use XY plane
      if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].y-tri.p[2].y, tri.p[1].x-tri.p[2].x, tri.p[1].y-tri.p[2].y, p.x-tri.p[2].x, p.y-tri.p[2].y, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }else
   if(size.y<=size.x && size.y<=size.z) // if Y dimension is smallest
   {
      // use XZ plane
      if(Solve(tri.p[0].x-tri.p[2].x, tri.p[0].z-tri.p[2].z, tri.p[1].x-tri.p[2].x, tri.p[1].z-tri.p[2].z, p.x-tri.p[2].x, p.z-tri.p[2].z, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }else
   {
      // use YZ plane
      if(Solve(tri.p[0].y-tri.p[2].y, tri.p[0].z-tri.p[2].z, tri.p[1].y-tri.p[2].y, tri.p[1].z-tri.p[2].z, p.y-tri.p[2].y, p.z-tri.p[2].z, blend.x, blend.y)!=1)blend.x=blend.y=0;
   }
   blend.z=1-blend.x-blend.y;
   return blend;
}
/******************************************************************************/
Vec4 TetraBlend(C Vec &p, C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3)
{
	Vec4 blend;
	Vec  q =p -p3,
	     q0=p0-p3,
	     q1=p1-p3,
	     q2=p2-p3;

	Matrix3 m;
	m.x=q0;
	m.y=q1;
	m.z=q2;

	           Flt det    =m.determinant();
	m.x=q;         blend.x=m.determinant();
	m.x=q0; m.y=q; blend.y=m.determinant();
	m.y=q1; m.z=q; blend.z=m.determinant();

	if(det)blend/=det; blend.w=1-blend.x-blend.y-blend.z;
   return blend;
}
Flt TetraVolume(C Vec &a, C Vec &b, C Vec &c, C Vec &d)
{
   return Abs(Dot(a-d, Cross(b-d, c-d)))/6;
}
/******************************************************************************/
static inline DIST_TYPE UpdateTypeT(DIST_TYPE type, DIST_TYPE p0, DIST_TYPE p1, DIST_TYPE edge)
{
   switch(type)
   {
      case DIST_POINT0: return p0  ;
      case DIST_POINT1: return p1  ;
      case DIST_EDGE  : return edge;
      default         : return type;
   }
}
Flt Dist(C Vec2 &point, C Tri2 &tri, DIST_TYPE *_type)
{
   DIST_TYPE t, type=DIST_NONE;
   Flt       d, dist=0;
   UInt         sign=(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign

   Vec2 p=point-tri.p[1]; if(Xor(DistPointPlane(p    ,           Perp(tri.p[0]-tri.p[1])), sign)>0){d=DistPointEdge(point, tri.p[0], tri.p[1], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT0, DIST_POINT1, DIST_EDGE0);}}
                          if(Xor(DistPointPlane(p    ,           Perp(tri.p[1]-tri.p[2])), sign)>0){d=DistPointEdge(point, tri.p[1], tri.p[2], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT1, DIST_POINT2, DIST_EDGE1);}}
                          if(Xor(DistPointPlane(point, tri.p[2], Perp(tri.p[2]-tri.p[0])), sign)>0){d=DistPointEdge(point, tri.p[2], tri.p[0], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT2, DIST_POINT0, DIST_EDGE2);}}
   if(!type)
   {
      type=DIST_PLANE;
      dist=0;
   }
   if(_type)*_type=type; return dist;
}
Flt Dist(C Vec &point, C Tri &tri, DIST_TYPE *_type)
{
   DIST_TYPE t, type=DIST_NONE;
   Flt       d, dist=0;

   Vec p=point-tri.p[1]; if(DistPointPlane(p    ,           Cross(tri.n, tri.p[0]-tri.p[1]))>0){d=DistPointEdge(point, tri.p[0], tri.p[1], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT0, DIST_POINT1, DIST_EDGE0);}}
                         if(DistPointPlane(p    ,           Cross(tri.n, tri.p[1]-tri.p[2]))>0){d=DistPointEdge(point, tri.p[1], tri.p[2], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT1, DIST_POINT2, DIST_EDGE1);}}
                         if(DistPointPlane(point, tri.p[2], Cross(tri.n, tri.p[2]-tri.p[0]))>0){d=DistPointEdge(point, tri.p[2], tri.p[0], &t); if(!type || d<dist){dist=d; type=UpdateTypeT(t, DIST_POINT2, DIST_POINT0, DIST_EDGE2);}}
   if(!type)
   {
      type=DIST_PLANE;
      dist=Abs(DistPointPlane(point, tri));
   }
   if(_type)*_type=type; return dist;
}
Flt Dist(C Edge &edge, C Tri &tri)
{
   // TODO: optimize
   if(Cuts(edge, tri))return 0;
   return Min(Min(Dist(edge     , tri.edge0()),
                  Dist(edge     , tri.edge1()),
                  Dist(edge     , tri.edge2())),
                  Dist(edge.p[0], tri        ),
                  Dist(edge.p[1], tri        ));
}
Flt Dist(C Tri &a, C Tri &b)
{
   // TODO: optimize
   return Min(Min(Dist(a.edge0(), b),
                  Dist(a.edge1(), b),
                  Dist(a.edge2(), b)),
              Min(Dist(b.edge0(), a),
                  Dist(b.edge1(), a),
                  Dist(b.edge2(), a)));
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Tri &tri, C Vec (&tri_cross)[3])
{
   if(DistPointPlane(point, tri.p[0], tri_cross[0])>0)return false;
   if(DistPointPlane(point, tri.p[1], tri_cross[1])>0)return false;
   if(DistPointPlane(point, tri.p[2], tri_cross[2])>0)return false;
   return true;
}
/******************************************************************************/
Bool Cuts(C Vec2 &point, C Tri2 &tri)
{
   UInt sign=(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   Vec2 p=point-tri.p[1]; if(Xor(DistPointPlane(p    ,           Perp(tri.p[0]-tri.p[1])), sign)>0)return false;
                          if(Xor(DistPointPlane(p    ,           Perp(tri.p[1]-tri.p[2])), sign)>0)return false;
                          if(Xor(DistPointPlane(point, tri.p[2], Perp(tri.p[2]-tri.p[0])), sign)>0)return false;
   return true;
}
Bool Cuts(C VecD2 &point, C TriD2 &tri)
{
   UInt  sign=(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   VecD2 p=point-tri.p[1]; if(Xor(DistPointPlane(p    ,           Perp(tri.p[0]-tri.p[1])), sign)>0)return false;
                           if(Xor(DistPointPlane(p    ,           Perp(tri.p[1]-tri.p[2])), sign)>0)return false;
                           if(Xor(DistPointPlane(point, tri.p[2], Perp(tri.p[2]-tri.p[0])), sign)>0)return false;
   return true;
}
Bool Cuts(C Vec &point, C Tri &tri)
{
   if(DistPointPlane(point, tri.p[0], Cross(tri.n, tri.p[0]-tri.p[1]))>0)return false;
   if(DistPointPlane(point, tri.p[1], Cross(tri.n, tri.p[1]-tri.p[2]))>0)return false;
   if(DistPointPlane(point, tri.p[2], Cross(tri.n, tri.p[2]-tri.p[0]))>0)return false;
   return true;
}
Bool Cuts(C VecD &point, C TriD &tri)
{
   if(DistPointPlane(point, tri.p[0], Cross(tri.n, tri.p[0]-tri.p[1]))>0)return false;
   if(DistPointPlane(point, tri.p[1], Cross(tri.n, tri.p[1]-tri.p[2]))>0)return false;
   if(DistPointPlane(point, tri.p[2], Cross(tri.n, tri.p[2]-tri.p[0]))>0)return false;
   return true;
}
/******************************************************************************/
Bool CutsEps(C Vec2 &point, C Tri2 &tri)
{
   UInt sign=(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   Vec2 d10=!(tri.p[0]-tri.p[1]), p0=point-tri.p[0]; if(Xor(DistPointPlane(p0,  Perp(d10)), sign)>EPS)return false;
   Vec2 d21=!(tri.p[1]-tri.p[2]), p1=point-tri.p[1]; if(Xor(DistPointPlane(p1,  Perp(d21)), sign)>EPS)return false;
   Vec2 d02=!(tri.p[2]-tri.p[0]), p2=point-tri.p[2]; if(Xor(DistPointPlane(p2,  Perp(d02)), sign)>EPS)return false;
                                                     if(    DistPointPlane(p0, !(d10-d02))       >EPS)return false;
                                                     if(    DistPointPlane(p1, !(d21-d10))       >EPS)return false;
                                                     if(    DistPointPlane(p2, !(d02-d21))       >EPS)return false;
   return true;
}
Bool CutsEps(C VecD2 &point, C TriD2 &tri)
{
   UInt sign=(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   VecD2 d10=!(tri.p[0]-tri.p[1]), p0=point-tri.p[0]; if(Xor(DistPointPlane(p0,  Perp(d10)), sign)>EPSD)return false;
   VecD2 d21=!(tri.p[1]-tri.p[2]), p1=point-tri.p[1]; if(Xor(DistPointPlane(p1,  Perp(d21)), sign)>EPSD)return false;
   VecD2 d02=!(tri.p[2]-tri.p[0]), p2=point-tri.p[2]; if(Xor(DistPointPlane(p2,  Perp(d02)), sign)>EPSD)return false;
                                                      if(    DistPointPlane(p0, !(d10-d02))       >EPSD)return false;
                                                      if(    DistPointPlane(p1, !(d21-d10))       >EPSD)return false;
                                                      if(    DistPointPlane(p2, !(d02-d21))       >EPSD)return false;
   return true;
}
Bool CutsEps(C Vec &point, C Tri &tri)
{
   Vec d10=!(tri.p[0]-tri.p[1]); if(DistPointPlane(point, tri.p[0], Cross(tri.n, d10))>EPS)return false;
   Vec d21=!(tri.p[1]-tri.p[2]); if(DistPointPlane(point, tri.p[1], Cross(tri.n, d21))>EPS)return false;
   Vec d02=!(tri.p[2]-tri.p[0]); if(DistPointPlane(point, tri.p[2], Cross(tri.n, d02))>EPS)return false;
   if(DistPointPlane(point, tri.p[0], !(d10-d02))>EPS)return false;
   if(DistPointPlane(point, tri.p[1], !(d21-d10))>EPS)return false;
   if(DistPointPlane(point, tri.p[2], !(d02-d21))>EPS)return false;
   return true;
}
Bool CutsEps(C VecD &point, C TriD &tri)
{
   VecD d10=!(tri.p[0]-tri.p[1]); if(DistPointPlane(point, tri.p[0], Cross(tri.n, d10))>EPSD)return false;
   VecD d21=!(tri.p[1]-tri.p[2]); if(DistPointPlane(point, tri.p[1], Cross(tri.n, d21))>EPSD)return false;
   VecD d02=!(tri.p[2]-tri.p[0]); if(DistPointPlane(point, tri.p[2], Cross(tri.n, d02))>EPSD)return false;
   if(DistPointPlane(point, tri.p[0], !(d10-d02))>EPSD)return false;
   if(DistPointPlane(point, tri.p[1], !(d21-d10))>EPSD)return false;
   if(DistPointPlane(point, tri.p[2], !(d02-d21))>EPSD)return false;
   return true;
}
/******************************************************************************/
Bool Cuts(C Edge &edge, C Tri &tri)
{
   Vec hitp;
   return Cuts(edge, tri.plane(), &hitp) && Cuts(hitp, tri);
}
/******************************************************************************/
Int CutsTriPlane(C Tri &tri, C Plane &plane, Edge &edge)
{
   Flt d0=Dist(tri.p[0], plane),
       d1=Dist(tri.p[1], plane),
       d2=Dist(tri.p[2], plane);
   Int s0=Sign(d0),
       s1=Sign(d1),
       s2=Sign(d2);
   if(!s0 && !s1 && !s2)return -1; // co-planar
   if( s0==s1 && s0==s2)return  0; // not cutting
   if(!s0 &&    !s1){edge.p[0]=tri.p[0]; edge.p[1]=tri.p[1]; return 2;}
   if(!s1 &&    !s2){edge.p[0]=tri.p[1]; edge.p[1]=tri.p[2]; return 2;}
   if(!s2 &&    !s0){edge.p[0]=tri.p[2]; edge.p[1]=tri.p[0]; return 2;}
   if(!s0 && s1==s2){edge.p[0]=tri.p[0];                     return 1;}
   if(!s1 && s2==s0){edge.p[0]=tri.p[1];                     return 1;}
   if(!s2 && s0==s1){edge.p[0]=tri.p[2];                     return 1;}
   Int i=0;
   if(!s0){edge.p[0]=tri.p[0]; i++;}else
   if(!s1){edge.p[0]=tri.p[1]; i++;}else
   if(!s2){edge.p[0]=tri.p[2]; i++;}
   if(!(s0+s1))edge.p[i++]=PointOnPlane(tri.p[0], tri.p[1], d0, d1);
   if(!(s1+s2))edge.p[i++]=PointOnPlane(tri.p[1], tri.p[2], d1, d2);
   if(!(s2+s0))edge.p[i++]=PointOnPlane(tri.p[2], tri.p[0], d2, d0);
   return 2;
}
Int CutsTriPlaneEps(C Tri &tri, C Plane &plane, Edge &edge)
{
   Flt d0=Dist(tri.p[0], plane),
       d1=Dist(tri.p[1], plane),
       d2=Dist(tri.p[2], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1),
       s2=SignEps(d2);
   if(!s0 && !s1 && !s2)return -1; // co-planar
   if( s0==s1 && s0==s2)return  0; // not cutting
   if(!s0 &&    !s1){edge.p[0]=tri.p[0]; edge.p[1]=tri.p[1]; return 2;}
   if(!s1 &&    !s2){edge.p[0]=tri.p[1]; edge.p[1]=tri.p[2]; return 2;}
   if(!s2 &&    !s0){edge.p[0]=tri.p[2]; edge.p[1]=tri.p[0]; return 2;}
   if(!s0 && s1==s2){edge.p[0]=tri.p[0];                     return 1;}
   if(!s1 && s2==s0){edge.p[0]=tri.p[1];                     return 1;}
   if(!s2 && s0==s1){edge.p[0]=tri.p[2];                     return 1;}
   Int i=0;
   if(!s0){edge.p[0]=tri.p[0]; i++;}else
   if(!s1){edge.p[0]=tri.p[1]; i++;}else
   if(!s2){edge.p[0]=tri.p[2]; i++;}
   if(!(s0+s1))edge.p[i++]=PointOnPlane(tri.p[0], tri.p[1], d0, d1);
   if(!(s1+s2))edge.p[i++]=PointOnPlane(tri.p[1], tri.p[2], d1, d2);
   if(!(s2+s0))edge.p[i++]=PointOnPlane(tri.p[2], tri.p[0], d2, d0);
   return 2;
}
Int CutsTriPlane(C TriD &tri, C PlaneD &plane, EdgeD &edge)
{
   Dbl d0=Dist(tri.p[0], plane),
       d1=Dist(tri.p[1], plane),
       d2=Dist(tri.p[2], plane);
   Int s0=Sign(d0),
       s1=Sign(d1),
       s2=Sign(d2);
   if(!s0 && !s1 && !s2)return -1; // co-planar
   if( s0==s1 && s0==s2)return  0; // not cutting
   if(!s0 &&    !s1){edge.p[0]=tri.p[0]; edge.p[1]=tri.p[1]; return 2;}
   if(!s1 &&    !s2){edge.p[0]=tri.p[1]; edge.p[1]=tri.p[2]; return 2;}
   if(!s2 &&    !s0){edge.p[0]=tri.p[2]; edge.p[1]=tri.p[0]; return 2;}
   if(!s0 && s1==s2){edge.p[0]=tri.p[0];                     return 1;}
   if(!s1 && s2==s0){edge.p[0]=tri.p[1];                     return 1;}
   if(!s2 && s0==s1){edge.p[0]=tri.p[2];                     return 1;}
   Int i=0;
   if(!s0){edge.p[0]=tri.p[0]; i++;}else
   if(!s1){edge.p[0]=tri.p[1]; i++;}else
   if(!s2){edge.p[0]=tri.p[2]; i++;}
   if(!(s0+s1))edge.p[i++]=PointOnPlane(tri.p[0], tri.p[1], d0, d1);
   if(!(s1+s2))edge.p[i++]=PointOnPlane(tri.p[1], tri.p[2], d1, d2);
   if(!(s2+s0))edge.p[i++]=PointOnPlane(tri.p[2], tri.p[0], d2, d0);
   return 2;
}
Int CutsTriPlaneEps(C TriD &tri, C PlaneD &plane, EdgeD &edge)
{
   Dbl d0=Dist(tri.p[0], plane),
       d1=Dist(tri.p[1], plane),
       d2=Dist(tri.p[2], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1),
       s2=SignEps(d2);
   if(!s0 && !s1 && !s2)return -1; // co-planar
   if( s0==s1 && s0==s2)return  0; // not cutting
   if(!s0 &&    !s1){edge.p[0]=tri.p[0]; edge.p[1]=tri.p[1]; return 2;}
   if(!s1 &&    !s2){edge.p[0]=tri.p[1]; edge.p[1]=tri.p[2]; return 2;}
   if(!s2 &&    !s0){edge.p[0]=tri.p[2]; edge.p[1]=tri.p[0]; return 2;}
   if(!s0 && s1==s2){edge.p[0]=tri.p[0];                     return 1;}
   if(!s1 && s2==s0){edge.p[0]=tri.p[1];                     return 1;}
   if(!s2 && s0==s1){edge.p[0]=tri.p[2];                     return 1;}
   Int i=0;
   if(!s0){edge.p[0]=tri.p[0]; i++;}else
   if(!s1){edge.p[0]=tri.p[1]; i++;}else
   if(!s2){edge.p[0]=tri.p[2]; i++;}
   if(!(s0+s1))edge.p[i++]=PointOnPlane(tri.p[0], tri.p[1], d0, d1);
   if(!(s1+s2))edge.p[i++]=PointOnPlane(tri.p[1], tri.p[2], d1, d2);
   if(!(s2+s0))edge.p[i++]=PointOnPlane(tri.p[2], tri.p[0], d2, d0);
   return 2;
}
/******************************************************************************/
Bool SweepPointTri(C Vec &point, C Vec &move, C Tri &tri, Flt *hit_frac, Vec *hit_pos, Bool two_sided)
{
   Vec hitp;
   if(SweepPointPlane(point, move, tri.plane(), hit_frac, null, &hitp, two_sided))
   if(Cuts(hitp, tri))
   {
      if(hit_pos)*hit_pos=hitp;
      return true;
   }
   return false;
}
Bool SweepPointTriEps(C Vec &point, C Vec &move, C Tri &tri, Flt *hit_frac, Vec *hit_pos, Bool two_sided)
{
   Vec hitp;
   if(SweepPointPlane(point, move, tri.plane(), hit_frac, null, &hitp, two_sided))
   if(CutsEps(hitp, tri))
   {
      if(hit_pos)*hit_pos=hitp;
      return true;
   }
   return false;
}
/******************************************************************************/
Int Clip(Edge2 &edge, C Tri2 &tri)
{
   UInt sign     =(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   Byte edge_n[2]={0, 0};
   Vec2  tri_n[3]=
   {
      PerpN(tri.p[0]-tri.p[1]),
      PerpN(tri.p[1]-tri.p[2]),
      PerpN(tri.p[2]-tri.p[0]),
   };

   REPD(e, 3)
   {
      Edge2 cuts; if(Int c=CutsEdgeEdge(Edge2_I(tri.p[e], tri.p[(e+1)%3]), Edge2_I(edge.p[0], edge.p[1]), &cuts))
      {
         if(c==2)
         {
            if(++edge_n[0]>2
            || ++edge_n[1]>2)return 0;
            Int i=Closer(edge.p[0], cuts.p[0], cuts.p[1]);
            edge.p[0]=cuts.p[ i];
            edge.p[1]=cuts.p[!i];
         }else
         {
            Int dup;
            if(Equal(cuts.p[0], edge.p[0]))dup=0;else
            if(Equal(cuts.p[0], edge.p[1]))dup=1;else dup=-1;
            if(dup!=0 && Xor(DistPointPlane(edge.p[0], tri.p[e], tri_n[e]), sign)<0)dup=1;else
            if(dup!=1 && Xor(DistPointPlane(edge.p[1], tri.p[e], tri_n[e]), sign)<0)dup=0;else
            {
               edge.p[0]=cuts.p[0];
               return 1;
            }
            if(++edge_n[dup]>2)return 0;
                 edge.p[dup]=cuts.p[0];
         }
      }
   }
   if(edge_n[0] || edge_n[1])return 2;
   return (Xor(DistPointPlane(edge.p[0], tri.p[0], tri_n[0]), sign)<0
        && Xor(DistPointPlane(edge.p[0], tri.p[1], tri_n[1]), sign)<0
        && Xor(DistPointPlane(edge.p[0], tri.p[2], tri_n[2]), sign)<0) ? 2 : 0;
}
Int Clip(EdgeD2 &edge, C TriD2 &tri)
{
   UInt sign     =(tri.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
   Byte edge_n[2]={0, 0};
   VecD2 tri_n[3]=
   {
      PerpN(tri.p[0]-tri.p[1]),
      PerpN(tri.p[1]-tri.p[2]),
      PerpN(tri.p[2]-tri.p[0]),
   };

   REPD(e, 3)
   {
      EdgeD2 cuts; if(Int c=CutsEdgeEdge(EdgeD2_I(tri.p[e], tri.p[(e+1)%3]), EdgeD2_I(edge.p[0], edge.p[1]), &cuts))
      {
         if(c==2)
         {
            if(++edge_n[0]>2
            || ++edge_n[1]>2)return 0;
            Int i=Closer(edge.p[0], cuts.p[0], cuts.p[1]);
            edge.p[0]=cuts.p[ i];
            edge.p[1]=cuts.p[!i];
         }else
         {
            Int dup;
            if(Equal(cuts.p[0], edge.p[0]))dup=0;else
            if(Equal(cuts.p[0], edge.p[1]))dup=1;else dup=-1;
            if(dup!=0 && Xor(DistPointPlane(edge.p[0], tri.p[e], tri_n[e]), sign)<0)dup=1;else
            if(dup!=1 && Xor(DistPointPlane(edge.p[1], tri.p[e], tri_n[e]), sign)<0)dup=0;else
            {
               edge.p[0]=cuts.p[0];
               return 1;
            }
            if(++edge_n[dup]>2)return 0;
                 edge.p[dup]=cuts.p[0];
         }
      }
   }
   if(edge_n[0] || edge_n[1])return 2;
   return (Xor(DistPointPlane(edge.p[0], tri.p[0], tri_n[0]), sign)<0
        && Xor(DistPointPlane(edge.p[0], tri.p[1], tri_n[1]), sign)<0
        && Xor(DistPointPlane(edge.p[0], tri.p[2], tri_n[2]), sign)<0) ? 2 : 0;
}
/******************************************************************************/
}
/******************************************************************************/
