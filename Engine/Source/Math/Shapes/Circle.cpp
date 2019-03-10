/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Circle::draw(C Color &color, Bool fill, Int resolution)C
{
   if(resolution<0)resolution=96;else if(resolution<3)resolution=3;
   Vec2 prev(pos.x+r, pos.y);

   VI.color(color);
   REP(resolution)
   {
      Flt c, s; CosSin(c, s, (PI2*i)/resolution); Vec2 next(pos.x+c*r, pos.y+s*r);
      if(fill)VI.tri (pos , next, prev);
      else    VI.line(prev, next);
      prev=next;
   }
   VI.end();
}
void Circle::drawPie(C Color &color, Flt r_start, Flt angle_start, Flt angle_range, Bool fill, Int resolution)C
{
   if(Abs(angle_range)>=PI2 && !fill) // draw inner and outer lines only
   {
      Circle(r_start, pos).draw(color, false, resolution); // draw inner lines
                           draw(color, false, resolution); // draw outer lines
   }else
   {
      if(resolution<0)resolution=96;else if(resolution<3)resolution=3;
      Flt c, s; CosSin(c, s, angle_start+angle_range); Vec2 prev(pos.x+c*r, pos.y+s*r), prev2(pos.x+c*r_start, pos.y+s*r_start);

      VI.color(color);
      if(!fill)VI.line(prev, prev2); // side line
      REP(resolution)
      {
         CosSin(c, s, angle_start+(angle_range*i)/resolution); Vec2 next(pos.x+c*r, pos.y+s*r), next2(pos.x+c*r_start, pos.y+s*r_start);
         if(fill)VI.quad(prev, next, next2, prev2);
         else   {VI.line(prev, next); VI.line(prev2, next2);}
         prev=next; prev2=next2;
      }
      if(!fill)VI.line(prev, prev2); // side line
      VI.end();
   }
}
/******************************************************************************/
Flt Dist(C Vec2 &point, C Circle &circle)
{
   return Max(0, Dist(point, circle.pos)-circle.r);
}
Flt Dist(C VecI2 &point, C Circle &circle)
{
   return Max(0, Dist(point, circle.pos)-circle.r);
}
Flt Dist(C Rect &rect, C Circle &circle)
{
   Flt dist2=0;

	if(circle.pos.x<rect.min.x)dist2+=Sqr(circle.pos.x-rect.min.x);else
	if(circle.pos.x>rect.max.x)dist2+=Sqr(circle.pos.x-rect.max.x);

	if(circle.pos.y<rect.min.y)dist2+=Sqr(circle.pos.y-rect.min.y);else
	if(circle.pos.y>rect.max.y)dist2+=Sqr(circle.pos.y-rect.max.y);

   return Max(0, SqrtFast(dist2)-circle.r);
}
Flt Dist(C RectI &rect, C Circle &circle)
{
   Flt dist2=0;

	if(circle.pos.x<rect.min.x)dist2+=Sqr(circle.pos.x-rect.min.x);else
	if(circle.pos.x>rect.max.x)dist2+=Sqr(circle.pos.x-rect.max.x);

	if(circle.pos.y<rect.min.y)dist2+=Sqr(circle.pos.y-rect.min.y);else
	if(circle.pos.y>rect.max.y)dist2+=Sqr(circle.pos.y-rect.max.y);

   return Max(0, SqrtFast(dist2)-circle.r);
}
Flt Dist(C Circle &a, C Circle &b)
{
   return Max(0, Dist(a.pos, b.pos)-a.r-b.r);
}
/******************************************************************************/
Bool Cuts(C Vec2 &point, C Circle &circle)
{
   return Dist2(point, circle.pos)<=Sqr(circle.r);
}
Bool Cuts(C VecI2 &point, C Circle &circle)
{
   return Dist2(point, circle.pos)<=Sqr(circle.r);
}
Bool Cuts(C Rect &rect, C Circle &circle)
{
   Flt dist2=0;

	if(circle.pos.x<rect.min.x)dist2+=Sqr(circle.pos.x-rect.min.x);else
	if(circle.pos.x>rect.max.x)dist2+=Sqr(circle.pos.x-rect.max.x);

	if(circle.pos.y<rect.min.y)dist2+=Sqr(circle.pos.y-rect.min.y);else
	if(circle.pos.y>rect.max.y)dist2+=Sqr(circle.pos.y-rect.max.y);

   return dist2<=Sqr(circle.r);
}
Bool Cuts(C RectI &rect, C Circle &circle)
{
   Flt dist2=0;

	if(circle.pos.x<rect.min.x)dist2+=Sqr(circle.pos.x-rect.min.x);else
	if(circle.pos.x>rect.max.x)dist2+=Sqr(circle.pos.x-rect.max.x);

	if(circle.pos.y<rect.min.y)dist2+=Sqr(circle.pos.y-rect.min.y);else
	if(circle.pos.y>rect.max.y)dist2+=Sqr(circle.pos.y-rect.max.y);

   return dist2<=Sqr(circle.r);
}
Bool Cuts(C Circle &a, C Circle &b)
{
   return Dist2(a.pos, b.pos)<=Sqr(a.r+b.r);
}
/******************************************************************************/
Int CutsStrCircle(C Vec2 &point, C Vec2 &normal, C Circle &circle, Vec2 *contact_a, Vec2 *contact_b, Flt *width)
{
   Flt  d=DistPointPlane(point, circle.pos, normal),
        y=Abs(d);
   Vec2 right(normal.y, -normal.x);
   if(!y)
   {
      right*=circle.r;
      if(contact_a   )*contact_a=circle.pos+right;
      if(contact_b   )*contact_b=circle.pos-right;
      if(width       )*width    =circle.r;
      return 2;
   }
   if(y==circle.r)
   {
      if(contact_a)*contact_a=circle.pos+d*normal;
      if(width    )*width    =0;
      return 1;
   }
   if(y>circle.r)return 0;
   Vec2 center=circle.pos+d*normal;
   Flt  x     =Sqrt(circle.r*circle.r - y*y);
        right*=x;
   if(contact_a)*contact_a=center+right;
   if(contact_b)*contact_b=center-right;
   if(width    )*width    =x;
   return 2;
}
/******************************************************************************/
Int CutsCircleCircle(C Circle &c1, C Circle &c2, Vec2 *contact_a, Vec2 *contact_b, Flt *width)
{
   Vec2 edge=c2.pos-c1.pos;
   Flt  Sr, Lr, d=edge.length();
   MinMax(c1.r, c2.r, Sr, Lr);

   // the same center
   if(!d)return (c1.r==c2.r) ? -1 : 0;

   // inside || outside separate
   if(d+Sr<Lr || d>c1.r+c2.r)return 0;

   // inside || outside together
   if(d+Sr==Lr || d==c1.r+c2.r){if(contact_a)*contact_a=c1.pos+(c1.r/d)*edge; return 1;}

   // 2 points, straight covering those points : ax + by = c
   Flt c =c1.r*c1.r - c2.r*c2.r;
   Flt x1=c1.pos.x, x2=c2.pos.x, a=2*(x2-x1); c+=x2*x2 - x1*x1;
   Flt y1=c1.pos.y, y2=c2.pos.y, b=2*(y2-y1); c+=y2*y2 - y1*y1;
   Flt l =Sqrt(a*a + b*b);
   a/=l;
   b/=l;
   c/=l;
   Vec2 normal(a, b), point(a*c, b*c);
   return CutsStrCircle(point, normal, c1, contact_a, contact_b, width);
}
/******************************************************************************/
Bool Inside(C Rect &a, C Circle &b)
{
#if 0 // this is slower
   return Dist2(Max(Abs(a.max.x-b.pos.x), Abs(a.min.x-b.pos.x)),
                Max(Abs(a.max.y-b.pos.y), Abs(a.min.y-b.pos.y)))<=Sqr(b.r);
#else // this is faster
   return Max(Sqr(a.max.x-b.pos.x), Sqr(a.min.x-b.pos.x))
         +Max(Sqr(a.max.y-b.pos.y), Sqr(a.min.y-b.pos.y))<=Sqr(b.r);
#endif
}
/******************************************************************************/
Bool SweepPointCircle(C Vec2 &point, C Vec2 &move, C Circle &circle, Flt *hit_frac, Vec2 *hit_normal)
{
   Vec2 dir   =move; Flt length=dir.normalize();
   Vec2 dir_n =Perp(dir);
   Flt  s     =DistPointPlane(circle.pos, point, dir_n)/circle.r; if(Abs(s)>1)return false;
   Vec2 normal=-s*dir_n-CosSin(s)*dir;
   Flt  d     =DistPointPlane(circle.pos+normal*circle.r, point, dir); if(d<0 || d>length)return false;
   if(hit_frac  )*hit_frac  =d/length;
   if(hit_normal)*hit_normal=normal;
   return true;
}
Bool SweepPointCircle(C VecD2 &point, C VecD2 &move, C CircleD &circle, Dbl *hit_frac, VecD2 *hit_normal)
{
   VecD2 dir   =move; Dbl length=dir.normalize();
   VecD2 dir_n =Perp(dir);
   Dbl   s     =DistPointPlane(circle.pos, point, dir_n)/circle.r; if(Abs(s)>1)return false;
   VecD2 normal=-s*dir_n-CosSin(s)*dir;
   Dbl   d     =DistPointPlane(circle.pos+normal*circle.r, point, dir); if(d<0 || d>length)return false;
   if(hit_frac  )*hit_frac  =d/length;
   if(hit_normal)*hit_normal=normal;
   return true;
}
/******************************************************************************/
Bool SweepEdgeCircle(C Edge2 &edge, C Vec2 &move, C Circle &circle, Flt *hit_frac, Vec2 *hit_normal)
{
   Byte point_test;
   Vec2 edge_dir   =edge.p[1]-edge.p[0];
   Vec2 edge_normal=PerpN(edge_dir);
   if(!Dot(move, edge_normal))point_test=(Dot(edge_dir, move)>0);else
   {
      Vec2 point=circle.pos - edge_normal*circle.r;
      Flt  frac; if(SweepPlanePoint(Plane2(edge.p[0], edge_normal), move, point, &frac))
      {
         point-=frac*move;
         if(DistPointPlane(point, edge.p[0], edge_dir)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], edge_dir)>0)point_test=1;else
         {
            if(hit_frac  )*hit_frac  = frac;
            if(hit_normal)*hit_normal=-edge_normal;
            return true;
         }
      }else point_test=Closer(circle.pos, edge.p[0], edge.p[1]);
   }
   return SweepPointCircle(edge.p[point_test], move, circle, hit_frac, hit_normal);
}
Bool SweepEdgeCircle(C EdgeD2 &edge, C VecD2 &move, C CircleD &circle, Dbl *hit_frac, VecD2 *hit_normal)
{
   Byte  point_test;
   VecD2 edge_dir   =edge.p[1]-edge.p[0];
   VecD2 edge_normal=PerpN(edge_dir);
   if(!Dot(move, edge_normal))point_test=(Dot(edge_dir, move)>0);else
   {
      VecD2 point=circle.pos - edge_normal*circle.r;
      Dbl   frac; if(SweepPlanePoint(PlaneD2(edge.p[0], edge_normal), move, point, &frac))
      {
         point-=frac*move;
         if(DistPointPlane(point, edge.p[0], edge_dir)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], edge_dir)>0)point_test=1;else
         {
            if(hit_frac  )*hit_frac  = frac;
            if(hit_normal)*hit_normal=-edge_normal;
            return true;
         }
      }else point_test=Closer(circle.pos, edge.p[0], edge.p[1]);
   }
   return SweepPointCircle(edge.p[point_test], move, circle, hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepCircleCircle(C Circle &circle, C Vec2 &move, C Circle &c2, Flt *hit_frac, Vec2 *hit_normal)
{
   Vec2 dir=move; Flt length=dir.normalize();
   Vec2 d  =c2.pos-circle.pos;
   Flt  b  =-2*Dot(dir, d),
        c  =d.length2()-Sqr(circle.r+c2.r),
        x1, x2;
   Int  xs=Polynominal2(1, b, c, x1, x2);
   if(xs>=1 && x1>=0 && x1<=length)
   {
      Flt frac=x1/length;
      if(hit_frac  )*hit_frac  =frac;
      if(hit_normal)*hit_normal=!((circle.pos+frac*move)-c2.pos);
      return true;
   }
   return false;
}
Bool SweepCircleCircle(C CircleD &circle, C VecD2 &move, C CircleD &c2, Dbl *hit_frac, VecD2 *hit_normal)
{
   VecD2 dir=move; Dbl length=dir.normalize();
   VecD2 d  =c2.pos-circle.pos;
   Dbl   b  =-2*Dot(dir, d),
         c  =d.length2()-Sqr(circle.r+c2.r),
         x1, x2;
   Int   xs=Polynominal2(1, b, c, x1, x2);
   if(xs>=1 && x1>=0 && x1<=length)
   {
      Dbl frac=x1/length;
      if(hit_frac  )*hit_frac  =frac;
      if(hit_normal)*hit_normal=!((circle.pos+frac*move)-c2.pos);
      return true;
   }
   return false;
}
/******************************************************************************/
Bool SweepCirclePoint(C Circle &circle, C Vec2 &move, C Vec2 &point, Flt *hit_frac, Vec2 *hit_normal)
{
   if(SweepPointCircle(point, -move, circle, hit_frac, hit_normal)){if(hit_normal)hit_normal->chs(); return true;}
   return false;
}
Bool SweepCirclePoint(C CircleD &circle, C VecD2 &move, C VecD2 &point, Dbl *hit_frac, VecD2 *hit_normal)
{
   if(SweepPointCircle(point, -move, circle, hit_frac, hit_normal)){if(hit_normal)hit_normal->chs(); return true;}
   return false;
}
/******************************************************************************/
Bool SweepCircleEdge(C Circle &circle, C Vec2 &move, C Edge2 &edge, Flt *hit_frac, Vec2 *hit_normal)
{
   Byte point_test;
   Vec2 edge_dir   =edge.p[1]-edge.p[0];
   Vec2 edge_normal=PerpN(edge_dir);
   if(!Dot(move, edge_normal))point_test=(Dot(edge_dir, move)<0);else
   {
      Vec2 point=circle.pos - edge_normal*circle.r;
      if(SweepPointPlane(point, move, Plane2(edge.p[0], edge_normal), hit_frac, hit_normal, &point))
      {
         if(DistPointPlane(point, edge.p[0], edge_dir)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], edge_dir)>0)point_test=1;else
            return true;
      }else point_test=Closer(circle.pos, edge.p[0], edge.p[1]);
   }
   return SweepCirclePoint(circle, move, edge.p[point_test], hit_frac, hit_normal);
}
Bool SweepCircleEdge(C CircleD &circle, C VecD2 &move, C EdgeD2 &edge, Dbl *hit_frac, VecD2 *hit_normal)
{
   Byte  point_test;
   VecD2 edge_dir   =edge.p[1]-edge.p[0];
   VecD2 edge_normal=PerpN(edge_dir);
   if(!Dot(move, edge_normal))point_test=(Dot(edge_dir, move)<0);else
   {
      VecD2 point=circle.pos - edge_normal*circle.r;
      if(SweepPointPlane(point, move, PlaneD2(edge.p[0], edge_normal), hit_frac, hit_normal, &point))
      {
         if(DistPointPlane(point, edge.p[0], edge_dir)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], edge_dir)>0)point_test=1;else
            return true;
      }else point_test=Closer(circle.pos, edge.p[0], edge.p[1]);
   }
   return SweepCirclePoint(circle, move, edge.p[point_test], hit_frac, hit_normal);
}
/******************************************************************************/
}
/******************************************************************************/
