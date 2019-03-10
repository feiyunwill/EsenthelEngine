/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static void DrawPlane(C Color &color, Flt cell_size, Int resolution, C Vec &center, C Matrix3 &matrix, Bool ball)
{
   Flt size=cell_size*resolution/2;
   Vec lf  =center+size*( matrix.z-matrix.x),
       lb  =center+size*(-matrix.z-matrix.x),
       rb  =center+size*( matrix.x-matrix.z);
   VI.color(color);
   if(ball)
   {          
      Ball ball(D.viewRange(), CamMatrix.pos/ObjMatrix);
      REP(resolution+1)
      {
         Flt step=i*cell_size;
         Vec dx  =step*matrix.x,
             dz  =step*matrix.z, a, b;
         if(CutsStrBall(lb+dx, matrix.z, ball, &a, &b)==2)VI.line(a, b);
         if(CutsStrBall(lb+dz, matrix.x, ball, &a, &b)==2)VI.line(a, b);
      }
   }else
   {
      REP(resolution+1)
      {
         Flt step=i*cell_size;
         Vec dx  =step*matrix.x,
             dz  =step*matrix.z;
         VI.line(lf+dx, lb+dx);
         VI.line(lb+dz, rb+dz);
      }
   }
   VI.end();
}
void Plane::drawLocal(C Color &color, Flt size, Bool fill, Int resolution)C
{
   Matrix3 matrix; matrix.setUp(normal);
   if(fill)
   {
      Quad(pos+size*(matrix.z-matrix.x), pos+size*(matrix.x+matrix.z), pos+size*(matrix.x-matrix.z), pos+size*(-matrix.x-matrix.z)).draw(color, true);
   }else
   {
      if(resolution<0)resolution=16;else if(resolution<2)resolution=2;
      DrawPlane(color, (size*2)/resolution, resolution, pos, matrix, false);
   }
}
void Plane::drawInfiniteBySize(C Color &color, Flt cell_size)C
{
   Matrix3 matrix; matrix.setUp(normal);
   Int     resolution=Ceil2(Ceil((D.viewRange()*2)/cell_size)); // must be multiple of 2
   Vec     cam_delta =CamMatrix.pos/ObjMatrix-pos,
           aligned   =pos+matrix.x*AlignRound(Dot(cam_delta, matrix.x), cell_size)
                         +matrix.z*AlignRound(Dot(cam_delta, matrix.z), cell_size);

   DrawPlane(color, cell_size, resolution, aligned, matrix, true);
}
void Plane::drawInfiniteByResolution(C Color &color, Int resolution)C
{
   if(resolution<0)resolution=32;else if(resolution<2)resolution=2;

   Matrix3 matrix; matrix.setUp(normal);
   Flt     cell_size=(D.viewRange()*2)/resolution;
   Vec     cam_delta=CamMatrix.pos/ObjMatrix-pos,
           aligned  =pos+matrix.x*AlignRound(Dot(cam_delta, matrix.x), cell_size)
                        +matrix.z*AlignRound(Dot(cam_delta, matrix.z), cell_size);

   DrawPlane(color, cell_size, resolution, aligned, matrix, true);
}
/******************************************************************************/
Flt DistPointPlane(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y);}
Dbl DistPointPlane(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y);}
Flt DistPointPlane(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}

// skip testing "Dot!=0" before division for performance reasons
Flt DistPointPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}

Flt DistPointPlaneY(C Vec  &point,                    C Vec  &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point,                    C Vec  &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point,                    C VecD &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Flt DistPointPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}

Vec2  PointOnPlane   (C Vec2  &point,                     C Vec2  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
VecD2 PointOnPlane   (C VecD2 &point,                     C VecD2 &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
Vec   PointOnPlane   (C Vec   &point,                     C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
VecD  PointOnPlane   (C VecD  &point,                     C VecD  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
Vec2  PointOnPlane   (C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD2 PointOnPlane   (C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
Vec   PointOnPlane   (C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
Vec2  PointOnPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD2 PointOnPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
Vec   PointOnPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
Vec2  PointOnPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD2 PointOnPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
Vec   PointOnPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}

Vec  PointOnPlaneY(C Vec  &point,                    C Vec  &plane_normal) {return Vec (point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point,                    C Vec  &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point,                    C VecD &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
Vec  PointOnPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal) {return Vec (point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}

Flt   PointOnPlane(  Flt    a,   Flt    b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Dbl   PointOnPlane(  Dbl    a,   Dbl    b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Vec2  PointOnPlane(C Vec2  &a, C Vec2  &b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
VecD2 PointOnPlane(C VecD2 &a, C VecD2 &b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Vec   PointOnPlane(C Vec   &a, C Vec   &b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
VecD  PointOnPlane(C VecD  &a, C VecD  &b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
/******************************************************************************/
Bool SweepPointPlane(C Vec2 &point, C Vec2 &move, C Plane2 &plane, Flt *hit_frac, Vec2 *hit_normal, Vec2 *hit_pos, Bool two_sided)
{
   Flt way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Flt dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos || hit_normal) skip testing this as it's faster this way
   {
      Flt frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
      if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   }
   return true;
}
Bool SweepPointPlane(C VecD2 &point, C VecD2 &move, C PlaneD2 &plane, Dbl *hit_frac, VecD2 *hit_normal, VecD2 *hit_pos, Bool two_sided)
{
   Dbl way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Dbl dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos || hit_normal) skip testing this as it's faster this way
   {
      Dbl frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
      if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   }
   return true;
}
Bool SweepPointPlane(C Vec &point, C Vec &move, C Plane &plane, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos, Bool two_sided)
{
   Flt way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Flt dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos || hit_normal) skip testing this as it's faster this way
   {
      Flt frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
      if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   }
   return true;
}
Bool SweepPointPlane(C VecD &point, C VecD &move, C PlaneD &plane, Dbl *hit_frac, VecD *hit_normal, VecD *hit_pos, Bool two_sided)
{
   Dbl way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Dbl dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos || hit_normal) skip testing this as it's faster this way
   {
      Dbl frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
      if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   }
   return true;
}
/******************************************************************************/
void SlideMovement(Vec2 &move, C Vec2 *normal, Int normals)
{
   Flt dist;
   Int find=-1; REP(normals)
   {
      Flt d=Dot(move, normal[i]);
      if( d<0)if(find<0 || d<dist){find=i; dist=d;}
   }
   if(find>=0)
   {
      move-=(dist-EPS)*normal[find];
      REP(normals)if(i!=find && Dot(move, normal[i])<0 && Dot(normal[i], normal[find])<1-EPS){move.zero(); return;}
   }
}
/******************************************************************************/
void SlideMovement(Vec &move, C Vec *normal, Int normals)
{
   Vec  slide;
   Bool hit =false,
        hit0=false;
   REPD(i0, normals)
   {
      Flt d=Dot(move, normal[i0]); if(d<0)
      {
              hit0  =true;
         Bool hit1  =false;
         Vec  slide0=move-(d-EPS)*normal[i0];
         REPD(i1, normals)if(i0!=i1 && Dot(slide0, normal[i1])<0 && Dot(normal[i0], normal[i1])<1-EPS)
         {
                 hit1   =true;
            Bool hit2   =false;
            Vec  slide1 =CrossN(normal[i0], normal[i1]);
                 slide1*=Dot   (move      , slide1    );
            REPD(i2, normals)if(i0!=i2 && i1!=i2 && Dot(slide1, normal[i2])<0 && Dot(normal[i0], normal[i2])<1-EPS && Dot(normal[i1], normal[i2])<1-EPS)
            {
               hit2=true;
               break;
            }
            if(!hit2)
            {
               if(!hit || slide1.length2()>slide.length2()){hit=true; slide=slide1;}
            }
         }
         if(!hit1)
         {
            if(!hit || slide0.length2()>slide.length2()){hit=true; slide=slide0;}
         }
      }
   }
   if(hit0)
   {
      if(hit)move=slide;else move.zero();
   }
}
/******************************************************************************/
}
/******************************************************************************/
