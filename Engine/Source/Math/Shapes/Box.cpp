/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Extent::Extent(C Box &box) {ext=box.size()*0.5f; pos=box.min+ext;}

Box ::Box (C Rect    &rect   ) {min.set(rect.min           , 0); max.set(rect.max           , 0);}
Box ::Box (C Circle  &circle ) {min.set(circle.pos-circle.r, 0); max.set(circle.pos+circle.r, 0);}
Box ::Box (C Edge    &edge   ) {from(edge.p, Elms(edge.p));}
BoxD::BoxD(C EdgeD   &edge   ) {from(edge.p, Elms(edge.p));}
Box ::Box (C Tri     &tri    ) {from(tri .p, Elms(tri .p));}
BoxD::BoxD(C TriD    &tri    ) {from(tri .p, Elms(tri .p));}
Box ::Box (C Quad    &quad   ) {from(quad.p, Elms(quad.p));}
BoxD::BoxD(C QuadD   &quad   ) {from(quad.p, Elms(quad.p));}
Box ::Box (C BoxD    &box    ) {min=box.min; max=box.max;}
BoxD::BoxD(C Box     &box    ) {min=box.min; max=box.max;}
Box ::Box (C BoxI    &box    ) {min=box.min; max=box.max;}
Box ::Box (C Extent  &ext    ) {min=ext.min(); max=ext.max();}
Box ::Box (C Ball    &ball   ) {set (ball.r, ball.pos);}
BoxD::BoxD(C BallM   &ball   ) {set (ball.r, ball.pos);}
Box ::Box (C Capsule &capsule) {from(capsule.pointU(), capsule.pointD()); extend (capsule.r);}
Box ::Box (C Tube    &tube   ) {from(tube   .pointU(), tube   .pointD()); extendX(CosSin(tube .up.x)*tube.r         ).extendY(CosSin(tube .up.y)*tube.r         ).extendZ(CosSin(tube .up.z)*tube.r         );}
Box ::Box (C Torus   &torus  ) {T=torus.pos;                              extendX(CosSin(torus.up.x)*torus.R+torus.r).extendY(CosSin(torus.up.y)*torus.R+torus.r).extendZ(CosSin(torus.up.z)*torus.R+torus.r);}
Box ::Box (C Pyramid &pyramid) {Vec points[5]; pyramid.toCorners(points); from(points, Elms(points));}
Box ::Box (C OBox    &obox   )
{
   Vec size=obox.box.size();
    min=max=obox.box.min*obox.matrix;
 
   if(obox.matrix.x.x>0)max.x+=obox.matrix.x.x*size.x;else min.x+=obox.matrix.x.x*size.x;
   if(obox.matrix.x.y>0)max.y+=obox.matrix.x.y*size.x;else min.y+=obox.matrix.x.y*size.x;
   if(obox.matrix.x.z>0)max.z+=obox.matrix.x.z*size.x;else min.z+=obox.matrix.x.z*size.x;
 
   if(obox.matrix.y.x>0)max.x+=obox.matrix.y.x*size.y;else min.x+=obox.matrix.y.x*size.y;
   if(obox.matrix.y.y>0)max.y+=obox.matrix.y.y*size.y;else min.y+=obox.matrix.y.y*size.y;
   if(obox.matrix.y.z>0)max.z+=obox.matrix.y.z*size.y;else min.z+=obox.matrix.y.z*size.y;
 
   if(obox.matrix.z.x>0)max.x+=obox.matrix.z.x*size.z;else min.x+=obox.matrix.z.x*size.z;
   if(obox.matrix.z.y>0)max.y+=obox.matrix.z.y*size.z;else min.y+=obox.matrix.z.y*size.z;
   if(obox.matrix.z.z>0)max.z+=obox.matrix.z.z*size.z;else min.z+=obox.matrix.z.z*size.z;
}
Box::Box(C Cone &cone)
{
         T=cone.pos               ; // bottom position
   Box top=cone.pos+cone.up*cone.h; // top    position
   Flt csx=CosSin(cone.up.x), csy=CosSin(cone.up.y), csz=CosSin(cone.up.z);
       extendX(csx*cone.r_low ).extendY(csy*cone.r_low ).extendZ(csz*cone.r_low );
   top.extendX(csx*cone.r_high).extendY(csy*cone.r_high).extendZ(csz*cone.r_high);
   T|=top;
}
Box::Box(C Shape &shape)
{
   switch(shape.type)
   {
      default           : zero()         ; break;
      case SHAPE_POINT  : T=shape.point  ; break;
      case SHAPE_EDGE   : T=shape.edge   ; break;
      case SHAPE_RECT   : T=shape.rect   ; break;
      case SHAPE_BOX    : T=shape.box    ; break;
      case SHAPE_OBOX   : T=shape.obox   ; break;
      case SHAPE_CIRCLE : T=shape.circle ; break;
      case SHAPE_BALL   : T=shape.ball   ; break;
      case SHAPE_CAPSULE: T=shape.capsule; break;
      case SHAPE_TUBE   : T=shape.tube   ; break;
      case SHAPE_TORUS  : T=shape.torus  ; break;
      case SHAPE_CONE   : T=shape.cone   ; break;
      case SHAPE_PYRAMID: T=shape.pyramid; break;
   }
}
Box::Box(C MeshBase &mesh    ) {from(mesh.vtx.pos(), mesh.vtxs());}
Box::Box(C MeshPart &mesh    ) {mesh.getBox(T);}
Box::Box(C Skeleton &skeleton)
{
   if(!skeleton.bones.elms())zero();else
   {
      T=skeleton.bones.last().pos;
      REP(skeleton.bones.elms()-1)include(skeleton.bones[i].pos);
   }
}
OBox::OBox(C Torus &torus)
{
   box   .set     (Vec(-torus.R-torus.r, -torus.r, -torus.R-torus.r), Vec(torus.R+torus.r, torus.r, torus.R+torus.r));
   matrix.setPosUp(torus.pos, torus.up);
}
/******************************************************************************/
Box& Box::operator&=(C Box &b)
{
   if(b.min.x>min.x)min.x=b.min.x; if(b.max.x<max.x)max.x=b.max.x;
   if(b.min.y>min.y)min.y=b.min.y; if(b.max.y<max.y)max.y=b.max.y;
   if(b.min.z>min.z)min.z=b.min.z; if(b.max.z<max.z)max.z=b.max.z;
   return T;
}
BoxD& BoxD::operator&=(C BoxD &b)
{
   if(b.min.x>min.x)min.x=b.min.x; if(b.max.x<max.x)max.x=b.max.x;
   if(b.min.y>min.y)min.y=b.min.y; if(b.max.y<max.y)max.y=b.max.y;
   if(b.min.z>min.z)min.z=b.min.z; if(b.max.z<max.z)max.z=b.max.z;
   return T;
}
BoxI& BoxI::operator&=(C BoxI &b)
{
   if(b.min.x>min.x)min.x=b.min.x; if(b.max.x<max.x)max.x=b.max.x;
   if(b.min.y>min.y)min.y=b.min.y; if(b.max.y<max.y)max.y=b.max.y;
   if(b.min.z>min.z)min.z=b.min.z; if(b.max.z<max.z)max.z=b.max.z;
   return T;
}
OBox& OBox::operator*=(C Flt f) {box*=f; matrix.pos*=f; return T;}
OBox& OBox::operator/=(C Flt f) {box/=f; matrix.pos/=f; return T;}
/******************************************************************************/
OBox& OBox::operator*=(C Vec &v)
{
   Flt d=(matrix.x*v).length(); box.min.x*=d; box.max.x*=d;
       d=(matrix.y*v).length(); box.min.y*=d; box.max.y*=d;
       d=(matrix.z*v).length(); box.min.z*=d; box.max.z*=d;
       matrix.pos*=v;
   return T;
}
OBox& OBox::operator/=(C Vec &v)
{
   Flt d=(matrix.x/v).length(); box.min.x*=d; box.max.x*=d;
       d=(matrix.y/v).length(); box.min.y*=d; box.max.y*=d;
       d=(matrix.z/v).length(); box.min.z*=d; box.max.z*=d;
       matrix.pos/=v;
   return T;
}
/******************************************************************************/
Box& Box::operator*=(C Matrix3 &matrix)
{
   Vec v[8]; toCorners(v);
   Transform(v, matrix, Elms(v));
   from     (v,         Elms(v));
   return T;
}
Box& Box::operator*=(C Matrix &matrix)
{
   Vec v[8]; toCorners(v);
   Transform(v, matrix, Elms(v));
   from     (v,         Elms(v));
   return T;
}
Extent& Extent::operator*=(C Matrix3 &matrix)
{
   Vec v[8]; toCorners(v);
   Transform(v, matrix, Elms(v));
   from     (v,         Elms(v));
   return T;
}
Extent& Extent::operator*=(C Matrix &matrix)
{
   Vec v[8]; toCorners(v);
   Transform(v, matrix, Elms(v));
   from     (v,         Elms(v));
   return T;
}
OBox& OBox::mul(C Matrix3 &matrix, Bool normalized)
{
   T.matrix*=matrix;
   if(!normalized)
   {
      Flt x=T.matrix.x.normalize(); box.min.x*=x; box.max.x*=x;
      Flt y=T.matrix.y.normalize(); box.min.y*=y; box.max.y*=y;
      Flt z=T.matrix.z.normalize(); box.min.z*=z; box.max.z*=z;
   }
   return T;
}
OBox& OBox::mul(C Matrix &matrix, Bool normalized)
{
   T.matrix*=matrix;
   if(!normalized)
   {
      Flt x=T.matrix.x.normalize(); box.min.x*=x; box.max.x*=x;
      Flt y=T.matrix.y.normalize(); box.min.y*=y; box.max.y*=y;
      Flt z=T.matrix.z.normalize(); box.min.z*=z; box.max.z*=z;
   }
   return T;
}
OBox& OBox::div(C Matrix3 &matrix, Bool normalized)
{
   if(normalized)T.matrix.divNormalized(matrix);else
   {
      T.matrix.div(matrix);
      Flt x=T.matrix.x.normalize(); box.min.x*=x; box.max.x*=x;
      Flt y=T.matrix.y.normalize(); box.min.y*=y; box.max.y*=y;
      Flt z=T.matrix.z.normalize(); box.min.z*=z; box.max.z*=z;
   }
   return T;
}
OBox& OBox::div(C Matrix &matrix, Bool normalized)
{
   if(normalized)T.matrix.divNormalized(matrix);else
   {
      T.matrix.div(matrix);
      Flt x=T.matrix.x.normalize(); box.min.x*=x; box.max.x*=x;
      Flt y=T.matrix.y.normalize(); box.min.y*=y; box.max.y*=y;
      Flt z=T.matrix.z.normalize(); box.min.z*=z; box.max.z*=z;
   }
   return T;
}
/******************************************************************************/
Flt Box::area()C
{
   Flt w=T.w(),
       h=T.h(),
       d=T.d();
   return 2*(w*h + h*d + w*d);
}
Dbl BoxD::area()C
{
   Dbl w=T.w(),
       h=T.h(),
       d=T.d();
   return 2*(w*h + h*d + w*d);
}
/******************************************************************************/
Box & Box ::extendX(  Flt   e) {min.x-=e; max.x+=e; return T;}
BoxD& BoxD::extendX(  Dbl   e) {min.x-=e; max.x+=e; return T;}
BoxI& BoxI::extendX(  Int   e) {min.x-=e; max.x+=e; return T;}
Box & Box ::extendY(  Flt   e) {min.y-=e; max.y+=e; return T;}
BoxD& BoxD::extendY(  Dbl   e) {min.y-=e; max.y+=e; return T;}
BoxI& BoxI::extendY(  Int   e) {min.y-=e; max.y+=e; return T;}
Box & Box ::extendZ(  Flt   e) {min.z-=e; max.z+=e; return T;}
BoxD& BoxD::extendZ(  Dbl   e) {min.z-=e; max.z+=e; return T;}
BoxI& BoxI::extendZ(  Int   e) {min.z-=e; max.z+=e; return T;}
Box & Box ::extend (  Flt   e) {min  -=e; max  +=e; return T;}
BoxD& BoxD::extend (  Dbl   e) {min  -=e; max  +=e; return T;}
BoxI& BoxI::extend (  Int   e) {min  -=e; max  +=e; return T;}
Box & Box ::extend (C Vec  &e) {min  -=e; max  +=e; return T;}
BoxD& BoxD::extend (C VecD &e) {min  -=e; max  +=e; return T;}
BoxI& BoxI::extend (C VecI &e) {min  -=e; max  +=e; return T;}
OBox& OBox::extend (  Flt   e) {box.extend(e);      return T;}
/******************************************************************************/
Box & Box ::includeX(Flt x) {if(x<min.x)min.x=x;else if(x>max.x)max.x=x; return T;}
BoxD& BoxD::includeX(Dbl x) {if(x<min.x)min.x=x;else if(x>max.x)max.x=x; return T;}
BoxI& BoxI::includeX(Int x) {if(x<min.x)min.x=x;else if(x>max.x)max.x=x; return T;}
Box & Box ::includeY(Flt y) {if(y<min.y)min.y=y;else if(y>max.y)max.y=y; return T;}
BoxD& BoxD::includeY(Dbl y) {if(y<min.y)min.y=y;else if(y>max.y)max.y=y; return T;}
BoxI& BoxI::includeY(Int y) {if(y<min.y)min.y=y;else if(y>max.y)max.y=y; return T;}
Box & Box ::includeZ(Flt z) {if(z<min.z)min.z=z;else if(z>max.z)max.z=z; return T;}
BoxD& BoxD::includeZ(Dbl z) {if(z<min.z)min.z=z;else if(z>max.z)max.z=z; return T;}
BoxI& BoxI::includeZ(Int z) {if(z<min.z)min.z=z;else if(z>max.z)max.z=z; return T;}
/******************************************************************************/
Box& Box::include(C Vec &v)
{
   Flt x=v.x, y=v.y, z=v.z;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   if(z<min.z)min.z=z;else if(z>max.z)max.z=z;
   return T;
}
BoxD& BoxD::include(C VecD &v)
{
   Dbl x=v.x, y=v.y, z=v.z;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   if(z<min.z)min.z=z;else if(z>max.z)max.z=z;
   return T;
}
BoxI& BoxI::include(C VecI &v)
{
   Int x=v.x, y=v.y, z=v.z;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   if(z<min.z)min.z=z;else if(z>max.z)max.z=z;
   return T;
}
/******************************************************************************/
Box& Box::include(C Box &b)
{
   if(b.min.x<min.x)min.x=b.min.x; if(b.max.x>max.x)max.x=b.max.x;
   if(b.min.y<min.y)min.y=b.min.y; if(b.max.y>max.y)max.y=b.max.y;
   if(b.min.z<min.z)min.z=b.min.z; if(b.max.z>max.z)max.z=b.max.z;
   return T;
}
BoxD& BoxD::include(C BoxD &b)
{
   if(b.min.x<min.x)min.x=b.min.x; if(b.max.x>max.x)max.x=b.max.x;
   if(b.min.y<min.y)min.y=b.min.y; if(b.max.y>max.y)max.y=b.max.y;
   if(b.min.z<min.z)min.z=b.min.z; if(b.max.z>max.z)max.z=b.max.z;
   return T;
}
BoxI& BoxI::include(C BoxI &b)
{
   if(b.min.x<min.x)min.x=b.min.x; if(b.max.x>max.x)max.x=b.max.x;
   if(b.min.y<min.y)min.y=b.min.y; if(b.max.y>max.y)max.y=b.max.y;
   if(b.min.z<min.z)min.z=b.min.z; if(b.max.z>max.z)max.z=b.max.z;
   return T;
}
Extent& Extent::includeX(  Flt  x) {T=Box(T).includeX(x); return T;}
Extent& Extent::includeY(  Flt  y) {T=Box(T).includeY(y); return T;}
Extent& Extent::includeZ(  Flt  z) {T=Box(T).includeZ(z); return T;}
Extent& Extent::include (C Vec &v) {T=Box(T).include (v); return T;}
Extent& Extent::include (C Box &b) {T=Box(T).include (b); return T;}
/******************************************************************************/
Box& Box::from(C Vec &a, C Vec &b)
{
   MinMax(a.x, b.x, min.x, max.x);
   MinMax(a.y, b.y, min.y, max.y);
   MinMax(a.z, b.z, min.z, max.z);
   return T;
}
BoxD& BoxD::from(C VecD &a, C VecD &b)
{
   MinMax(a.x, b.x, min.x, max.x);
   MinMax(a.y, b.y, min.y, max.y);
   MinMax(a.z, b.z, min.z, max.z);
   return T;
}

Bool Box::from(C Vec *point, Int points)
{
   if(MinMax(point, points, min, max))return true; zero(); return false;
}
Bool BoxD::from(C VecD *point, Int points)
{
   if(MinMax(point, points, min, max))return true; zero(); return false;
}
Bool Extent::from(C Vec *point, Int points)
{
   Box box; if(MinMax(point, points, box.min, box.max)){T=box; return true;} zero(); return false;
}

Bool Box::from(C Vec *point, Int points, C Matrix &matrix)
{
   if(points<=0 || !point){zero(); return false;}
   for(min=max=(*point++)*matrix, points--; --points>=0; )
   {
      Vec v=(*point++)*matrix;
      if(v.x<min.x)min.x=v.x;else if(v.x>max.x)max.x=v.x;
      if(v.y<min.y)min.y=v.y;else if(v.y>max.y)max.y=v.y;
      if(v.z<min.z)min.z=v.z;else if(v.z>max.z)max.z=v.z;
   }
   return true;
}

void Box::toCorners(Vec (&v)[8])C
{
   FREP(8)v[i].set((i&1) ? max.x : min.x,
                   (i&2) ? max.y : min.y,
                   (i&4) ? max.z : min.z);
}
void Extent::toCorners(Vec (&v)[8])C
{
   FREP(8)v[i].set((i&1) ? pos.x+ext.x : pos.x-ext.x,
                   (i&2) ? pos.y+ext.y : pos.y-ext.y,
                   (i&4) ? pos.z+ext.z : pos.z-ext.z);
}
void OBox::toCorners(Vec (&v)[8])C
{
#if 0
   box.toCorners(v);
   Transform(v, matrix, Elms(v));
#else // optimized
   Vec x=matrix.x*box.w(),
       y=matrix.y*box.h(),
       z=matrix.z*box.d();

   v[0|0|0]=box.min; v[0]*=matrix;
   v[1|0|0]=v[0]; v[1]+=x; // +x
   v[0|2|0]=v[0]; v[2]+=y; // +y
   v[0|0|4]=v[0]; v[4]+=z; // +z

   v[1|2|0]=v[1]; v[3]+=y; // +x+y
   v[1|0|4]=v[1]; v[5]+=z; // +x+z
   v[0|2|4]=v[2]; v[6]+=z; // +y+z
   v[1|2|4]=v[3]; v[7]+=z; // +x+y+z
#endif
}

Box& Box::mirrorX() {setX(-max.x, -min.x); return T;}
Box& Box::mirrorY() {setY(-max.y, -min.y); return T;}
Box& Box::mirrorZ() {setZ(-max.z, -min.z); return T;}

OBox& OBox::mirrorX() {CHS(matrix.pos.x); CHS(matrix.x.y); CHS(matrix.x.z); CHS(matrix.y.x); CHS(matrix.z.x); CHS(box.min.x); CHS(box.max.x); Swap(box.min.x, box.max.x); return T;}
OBox& OBox::mirrorY() {CHS(matrix.pos.y); CHS(matrix.y.x); CHS(matrix.y.z); CHS(matrix.x.y); CHS(matrix.z.y); CHS(box.min.y); CHS(box.max.y); Swap(box.min.y, box.max.y); return T;}
OBox& OBox::mirrorZ() {CHS(matrix.pos.z); CHS(matrix.z.x); CHS(matrix.z.y); CHS(matrix.x.z); CHS(matrix.y.z); CHS(box.min.z); CHS(box.max.z); Swap(box.min.z, box.max.z); return T;}
/******************************************************************************/
void Box   ::rightToLeft() {min.rightToLeft(); max.rightToLeft(); Swap(min.x, max.x); Swap(min.z, max.z);}
void Extent::rightToLeft() {ext.swapYZ(); pos.rightToLeft();}
/******************************************************************************/
// DRAW
/******************************************************************************/
void Box::draw(C Color &color, Bool fill)C
{
   VI.color(color);
   if(fill)
   {
      VI.quad(cornerRUF(), cornerLUF(), cornerLDF(), cornerRDF()); // forward
      VI.quad(cornerLUB(), cornerRUB(), cornerRDB(), cornerLDB()); // back
      VI.quad(cornerRUB(), cornerRUF(), cornerRDF(), cornerRDB()); // right
      VI.quad(cornerLUF(), cornerLUB(), cornerLDB(), cornerLDF()); // left
      VI.quad(cornerLUF(), cornerRUF(), cornerRUB(), cornerLUB()); // up
      VI.quad(cornerLDB(), cornerRDB(), cornerRDF(), cornerLDF()); // down
   }else
   {
      Vec a, b;
      a.x=min.x; b.x=max.x; REP(4){a.y=b.y=((i&1) ? min.y : max.y); a.z=b.z=((i&2) ? min.z : max.z); VI.line(a, b);}
      a.y=min.y; b.y=max.y; REP(4){a.x=b.x=((i&1) ? min.x : max.x); a.z=b.z=((i&2) ? min.z : max.z); VI.line(a, b);}
      a.z=min.z; b.z=max.z; REP(4){a.x=b.x=((i&1) ? min.x : max.x); a.y=b.y=((i&2) ? min.y : max.y); VI.line(a, b);}
   }
   VI.end();
}
/******************************************************************************/
void OBox::draw(C Color &color, Bool fill)C
{
   Vec v[8]; toCorners(v);
   VI.color(color);
   if(fill)
   {
      VI.quad(v[1|2|4], v[0|2|4], v[0|0|4], v[1|0|4]); // forward
      VI.quad(v[0|2|0], v[1|2|0], v[1|0|0], v[0|0|0]); // back
      VI.quad(v[1|2|0], v[1|2|4], v[1|0|4], v[1|0|0]); // right
      VI.quad(v[0|2|4], v[0|2|0], v[0|0|0], v[0|0|4]); // left
      VI.quad(v[0|2|4], v[1|2|4], v[1|2|0], v[0|2|0]); // up
      VI.quad(v[0|0|0], v[1|0|0], v[1|0|4], v[0|0|4]); // down
   }else
   {
      REP(4)VI.line(v[0| i                ], v[4| i                ]);
      REP(4)VI.line(v[0|(i<<1)            ], v[1|(i<<1)            ]);
      REP(4)VI.line(v[0|(i &1)|((i&2)?4:0)], v[2|(i &1)|((i&2)?4:0)]);
   }
   VI.end();
}
/******************************************************************************/
void Extent::draw(C Color &color, Bool fill)C {Box(T).draw(color, fill);}
/******************************************************************************/
// BOXES
/******************************************************************************/
void Boxes::set(C Box &box, C VecI &cells)
{
   T.box    =box;
   T.cells.x=Max(cells.x, 1);
   T.cells.y=Max(cells.y, 1);
   T.cells.z=Max(cells.z, 1);
   T.size   =box.size()/T.cells;
}
void Boxes::set(C Box &box, Int elms)
{
   VecI cells;
   Vec  size=box.size();
   Int  i0=size.minI(), i1=(i0+1)%3, i2=(i0+2)%3; if(size.c[i2]<size.c[i1])Swap(i1, i2); // 'size' indexes sorted from smallest to biggest
   Flt  volume=size.mul(),
        mul   =Cbrt(elms/volume); // = 1/avg_cell_size

   Int smallest=cells.c[i0]=Max(1, Round(size.c[i0]*mul)); // set number of cells in the smallest dimension

   Flt area=size.c[i1]*size.c[i2]; // calculate area of remaining 2 dimensions (middle and biggest)
       mul =Sqrt(elms/(smallest*area)); // = 1/avg_cell_size

   Int middle=cells.c[i1]=Max(1, Round(size.c[i1]*mul)); // set number of cells in the middle dimension

   cells.c[i2]=Max(1, DivRound(elms, smallest*middle)); // set number of cells in the biggest dimension

   set(box, cells);
}
VecI Boxes::coords(C Vec &pos)C
{
   VecI O=Trunc((pos-box.min)/size); // use 'Trunc' because it's faster and we're not interested in negative indexes
   Clamp(O.x, 0, cells.x-1);
   Clamp(O.y, 0, cells.y-1);
   Clamp(O.z, 0, cells.z-1);
   return O;
}
BoxI Boxes::coords(C Box &box)C
{
   return BoxI(coords(box.min),
               coords(box.max));
}
Box Boxes::getBox(C VecI &cell)C
{
   Box O;
   O.min=T.box.min+size*cell;
   O.max=    O.min+size;
   return O;
}
void Boxes::draw(C Color &color)C
{
   if(color.a)
   {
      VI.color(color);
      REPD(z, cells.z+1)
      {
         Flt fz=box.min.z+z*size.z;
         REPD(y, cells.y+1)
         {
            Flt fy=box.min.y+y*size.y;
            VI.line(Vec(box.min.x, fy, fz), Vec(box.max.x, fy, fz));
         }
         REPD(x, cells.x+1)
         {
            Flt fx=box.min.x+x*size.x;
            VI.line(Vec(fx, box.min.y, fz), Vec(fx, box.max.y, fz));
         }
      }
      REPD(y, cells.y+1)
      {
         Flt fy=box.min.y+y*size.y;
         REPD(x, cells.x+1)
         {
            Flt fx=box.min.x+x*size.x;
            VI.line(Vec(fx, fy, box.min.z), Vec(fx, fy, box.max.z));
         }
      }
      VI.end();
   }
}
/******************************************************************************/
Box Avg(C Box &a, C Box &b) {return Box(Avg(a.min, b.min), Avg(a.max, b.max));}
/******************************************************************************/
Flt Dist(C Vec &point, C Box &box)
{
   #define S box.min
   #define L box.max
   if(point.x>L.x) // right
   {
      if(point.y>L.y) // up
      {
         if(point.z>L.z)return Dist(point, box.cornerRUF()); // ruf corner
         if(point.z<S.z)return Dist(point, box.cornerRUB()); // rub corner
                        return Dist(Vec2(point.x, point.y), Vec2(L.x, L.y)); // ru edge
      }
      if(point.y<S.y) // down
      {
         if(point.z>L.z)return Dist(point, box.cornerRDF()); // rdf corner
         if(point.z<S.z)return Dist(point, box.cornerRDB()); // rdb corner
                        return Dist(Vec2(point.x, point.y), Vec2(L.x, S.y)); // rd edge
      }
      if(point.z>L.z)return Dist(Vec2(point.x, point.z), Vec2(L.x, L.z)); // rf edge
      if(point.z<S.z)return Dist(Vec2(point.x, point.z), Vec2(L.x, S.z)); // rb edge
                     return point.x-L.x; // r side
   }
   if(point.x<S.x) // left
   {
      if(point.y>L.y) // up
      {
         if(point.z>L.z)return Dist(point, box.cornerLUF()); // luf corner
         if(point.z<S.z)return Dist(point, box.cornerLUB()); // lub corner
                        return Dist(Vec2(point.x, point.y), Vec2(S.x, L.y)); // lu edge
      }
      if(point.y<S.y) // down
      {
         if(point.z>L.z)return Dist(point, box.cornerLDF()); // ldf corner
         if(point.z<S.z)return Dist(point, box.cornerLDB()); // ldb corner
                        return Dist(Vec2(point.x, point.y), Vec2(S.x, S.y)); // ld edge
      }
      if(point.z>L.z)return Dist(Vec2(point.x, point.z), Vec2(S.x, L.z)); // lf edge
      if(point.z<S.z)return Dist(Vec2(point.x, point.z), Vec2(S.x, S.z)); // lb edge
                     return S.x-point.x; // l side
   }
   if(point.y>L.y) // up
   {
      if(point.z>L.z)return Dist(Vec2(point.y, point.z), Vec2(L.y, L.z)); // uf edge
      if(point.z<S.z)return Dist(Vec2(point.y, point.z), Vec2(L.y, S.z)); // ub edge
                     return point.y-L.y; // u side
   }
   if(point.y<S.y) // down
   {
      if(point.z>L.z)return Dist(Vec2(point.y, point.z), Vec2(S.y, L.z)); // df edge
      if(point.z<S.z)return Dist(Vec2(point.y, point.z), Vec2(S.y, S.z)); // db edge
                     return S.y-point.y; // d side
   }
   if(point.z>L.z)return point.z-L.z; // f side
   if(point.z<S.z)return S.z-point.z; // b side
   return 0; // inside
   #undef S
   #undef L
}
/******************************************************************************/
Flt Dist2(C Vec &point, C Box &box)
{
   #define S box.min
   #define L box.max
   if(point.x>L.x) // right
   {
      if(point.y>L.y) // up
      {
         if(point.z>L.z)return Dist2(point, box.cornerRUF()); // ruf corner
         if(point.z<S.z)return Dist2(point, box.cornerRUB()); // rub corner
                        return Dist2(Vec2(point.x, point.y), Vec2(L.x, L.y)); // ru edge
      }
      if(point.y<S.y) // down
      {
         if(point.z>L.z)return Dist2(point, box.cornerRDF()); // rdf corner
         if(point.z<S.z)return Dist2(point, box.cornerRDB()); // rdb corner
                        return Dist2(Vec2(point.x, point.y), Vec2(L.x, S.y)); // rd edge
      }
      if(point.z>L.z)return Dist2(Vec2(point.x, point.z), Vec2(L.x, L.z)); // rf edge
      if(point.z<S.z)return Dist2(Vec2(point.x, point.z), Vec2(L.x, S.z)); // rb edge
                     return Sqr(point.x-L.x); // r side
   }
   if(point.x<S.x) // left
   {
      if(point.y>L.y) // up
      {
         if(point.z>L.z)return Dist2(point, box.cornerLUF()); // luf corner
         if(point.z<S.z)return Dist2(point, box.cornerLUB()); // lub corner
                        return Dist2(Vec2(point.x, point.y), Vec2(S.x, L.y)); // lu edge
      }
      if(point.y<S.y) // down
      {
         if(point.z>L.z)return Dist2(point, box.cornerLDF()); // ldf corner
         if(point.z<S.z)return Dist2(point, box.cornerLDB()); // ldb corner
                        return Dist2(Vec2(point.x, point.y), Vec2(S.x, S.y)); // ld edge
      }
      if(point.z>L.z)return Dist2(Vec2(point.x, point.z), Vec2(S.x, L.z)); // lf edge
      if(point.z<S.z)return Dist2(Vec2(point.x, point.z), Vec2(S.x, S.z)); // lb edge
                     return Sqr(S.x-point.x); // l side
   }
   if(point.y>L.y) // up
   {
      if(point.z>L.z)return Dist2(Vec2(point.y, point.z), Vec2(L.y, L.z)); // uf edge
      if(point.z<S.z)return Dist2(Vec2(point.y, point.z), Vec2(L.y, S.z)); // ub edge
                     return Sqr(point.y-L.y); // u side
   }
   if(point.y<S.y) // down
   {
      if(point.z>L.z)return Dist2(Vec2(point.y, point.z), Vec2(S.y, L.z)); // df edge
      if(point.z<S.z)return Dist2(Vec2(point.y, point.z), Vec2(S.y, S.z)); // db edge
                     return Sqr(S.y-point.y); // d side
   }
   if(point.z>L.z)return Sqr(point.z-L.z); // f side
   if(point.z<S.z)return Sqr(S.z-point.z); // b side
   return 0; // inside
   #undef S
   #undef L
}
/******************************************************************************/
Flt Dist(C Vec &point, C Extent &ext)
{
   return Dist(Max(0, Abs(point.x-ext.pos.x)-ext.ext.x),
               Max(0, Abs(point.y-ext.pos.y)-ext.ext.y),
               Max(0, Abs(point.z-ext.pos.z)-ext.ext.z));
}
Flt Dist2(C Vec &point, C Extent &ext)
{
   return Dist2(Max(0, Abs(point.x-ext.pos.x)-ext.ext.x),
                Max(0, Abs(point.y-ext.pos.y)-ext.ext.y),
                Max(0, Abs(point.z-ext.pos.z)-ext.ext.z));
}
/******************************************************************************/
Flt Dist(C Edge &edge, C Box &box)
{
   Edge edge_list[2][3*3*3];
   Int  edges=0, list_cur=0;

   // put edge to the list
   edge_list[list_cur][edges++]=edge-box.center();

   FREPD(d, 3) // 3 dimensions (x, y, z)
   {
      Int new_edges=0;
      Flt r        =(box.max.c[d]-box.min.c[d])*0.5f; // d-dimension radius (d=0 -> width/2, d=1 -> height/2, d=2 -> depth/2)

      REP(edges) // try to split all edges by 2 axes: -r, +r
      {
         Edge &edge=edge_list[list_cur][i];
         if(edge.p[0].c[d]>edge.p[1].c[d])edge.reverse(); // sort so they are in "growing" order

         Bool left_cut=false, right_cut=false;
         Vec  left, right;

         if(edge.p[0].c[d]<-r) // cut left outside piece
         {
            if(edge.p[1].c[d]<=-r)edge_list[list_cur^1][new_edges++]=edge;else // fully fits
            {
               left_cut=true;
               Flt f=(-r-edge.p[0].c[d])/(edge.p[1].c[d]-edge.p[0].c[d]);
               edge_list[list_cur^1][new_edges++].set(edge.p[0], left=edge.lerp(f));
            }
         }
         if(edge.p[1].c[d]>r) // cut right outside piece
         {
            if(edge.p[0].c[d]>=r)edge_list[list_cur^1][new_edges++]=edge;else // fully fits
            {
               right_cut=true;
               Flt f=(r-edge.p[0].c[d])/(edge.p[1].c[d]-edge.p[0].c[d]);
               edge_list[list_cur^1][new_edges++].set(right=edge.lerp(f), edge.p[1]);
            }
         }
         if(edge.p[0].c[d]<=r && edge.p[1].c[d]>=-r)
            edge_list[list_cur^1][new_edges++].set(left_cut ? left : edge.p[0], right_cut ? right : edge.p[1]);
      }

      // swap lists
      list_cur^=1;
      edges=new_edges;

      // warp
      REP(edges)
      {
         Edge &edge=edge_list[list_cur][i];
         ABS(edge.p[0].c[d]); MAX(edge.p[0].c[d]-=r, 0.0f);
         ABS(edge.p[1].c[d]); MAX(edge.p[1].c[d]-=r, 0.0f);
      }
   }
   
   Flt           dist=FLT_MAX;
   REP(edges)MIN(dist, Dist(VecZero, edge_list[list_cur][i]));
   return        dist;
}
/******************************************************************************/
Flt Dist(C Box &a, C Box &b)
{
   return Dist(Max(0, Abs(a.centerX()-b.centerX())-(a.w()+b.w())*0.5f),
               Max(0, Abs(a.centerY()-b.centerY())-(a.h()+b.h())*0.5f),
               Max(0, Abs(a.centerZ()-b.centerZ())-(a.d()+b.d())*0.5f));
}
/******************************************************************************/
Flt Dist(C Box &box, C Plane &plane)
{
   Vec test((plane.normal.x<=0) ? box.max.x : box.min.x,
            (plane.normal.y<=0) ? box.max.y : box.min.y,
            (plane.normal.z<=0) ? box.max.z : box.min.z);
   return Dist(test, plane);
}
Flt Dist(C OBox &obox, C Plane &plane)
{
   Flt n, d=Dist(obox.box.min*obox.matrix, plane);
   n=Dot(obox.matrix.x, plane.normal); if(n<0)d+=n*obox.box.w();
   n=Dot(obox.matrix.y, plane.normal); if(n<0)d+=n*obox.box.h();
   n=Dot(obox.matrix.z, plane.normal); if(n<0)d+=n*obox.box.d();
   return d;
}
/******************************************************************************/
Bool Cuts(C Edge &edge, C Box &box)
{
   if(Cuts(edge.p[0], box)
   || Cuts(edge.p[1], box))return true;

   Flt frac;
   Vec point=edge.p   [0],
       move =edge.delta();

   if(point.x<box.min.x && edge.p[1].x>=box.min.x){frac=(box.min.x-point.x)/move.x; if(                                      box.includesY(point.y+move.y*frac) && box.includesZ(point.z+move.z*frac))return true;}else
   if(point.x>box.max.x && edge.p[1].x<=box.max.x){frac=(box.max.x-point.x)/move.x; if(                                      box.includesY(point.y+move.y*frac) && box.includesZ(point.z+move.z*frac))return true;}
   if(point.y<box.min.y && edge.p[1].y>=box.min.y){frac=(box.min.y-point.y)/move.y; if(box.includesX(point.x+move.x*frac) &&                                       box.includesZ(point.z+move.z*frac))return true;}else
   if(point.y>box.max.y && edge.p[1].y<=box.max.y){frac=(box.max.y-point.y)/move.y; if(box.includesX(point.x+move.x*frac) &&                                       box.includesZ(point.z+move.z*frac))return true;}
   if(point.z<box.min.z && edge.p[1].z>=box.min.z){frac=(box.min.z-point.z)/move.z; if(box.includesX(point.x+move.x*frac) && box.includesY(point.y+move.y*frac)                                      )return true;}else
   if(point.z>box.max.z && edge.p[1].z<=box.max.z){frac=(box.max.z-point.z)/move.z; if(box.includesX(point.x+move.x*frac) && box.includesY(point.y+move.y*frac)                                      )return true;}

   return false;
}
/******************************************************************************/
Bool Cuts(C Rect &rect, C Box &box)
{
   return rect.min.x<=box.max.x && rect.max.x>=box.min.x
       && rect.min.y<=box.max.y && rect.max.y>=box.min.y;
}
Bool Cuts(C Box &a, C Box &b)
{
   return a.min.x<=b.max.x && a.max.x>=b.min.x
       && a.min.y<=b.max.y && a.max.y>=b.min.y
       && a.min.z<=b.max.z && a.max.z>=b.min.z;
}
Bool Cuts(C BoxD &a, C BoxD &b)
{
   return a.min.x<=b.max.x && a.max.x>=b.min.x
       && a.min.y<=b.max.y && a.max.y>=b.min.y
       && a.min.z<=b.max.z && a.max.z>=b.min.z;
}
Bool Cuts(C BoxI &a, C BoxI &b)
{
   return a.min.x<=b.max.x && a.max.x>=b.min.x
       && a.min.y<=b.max.y && a.max.y>=b.min.y
       && a.min.z<=b.max.z && a.max.z>=b.min.z;
}
/******************************************************************************/
Bool Cuts(C Extent &a, C Extent &b)
{
   return Abs(a.pos.x-b.pos.x)<=(a.ext.x+b.ext.x)
       && Abs(a.pos.y-b.pos.y)<=(a.ext.y+b.ext.y)
       && Abs(a.pos.z-b.pos.z)<=(a.ext.z+b.ext.z);
}
/******************************************************************************/
Bool Cuts(C Box &box, C OBox &obox)
{
   Vec ae=     box.size()*0.5f,       // a extents
       be=obox.box.size()*0.5f,       // b extents
       bo=obox.center()-box.center(); // b offset (b center relative to a center)

   Matrix3 m=obox.matrix;
   m.x.abs();
   m.y.abs();
   m.z.abs();

   if(Abs(bo.x) > ae.x + be.x*m.x.x + be.y*m.y.x + be.z*m.z.x)return false; // a.x axis
   if(Abs(bo.y) > ae.y + be.x*m.x.y + be.y*m.y.y + be.z*m.z.y)return false; // a.y axis
   if(Abs(bo.z) > ae.z + be.x*m.x.z + be.y*m.y.z + be.z*m.z.z)return false; // a.z axis

   if(Abs(bo.x*obox.matrix.x.x + bo.y*obox.matrix.x.y + bo.z*obox.matrix.x.z) > be.x + ae.x*m.x.x + ae.y*m.x.y + ae.z*m.x.z)return false; // b.x axis
   if(Abs(bo.x*obox.matrix.y.x + bo.y*obox.matrix.y.y + bo.z*obox.matrix.y.z) > be.y + ae.x*m.y.x + ae.y*m.y.y + ae.z*m.y.z)return false; // b.x axis
   if(Abs(bo.x*obox.matrix.z.x + bo.y*obox.matrix.z.y + bo.z*obox.matrix.z.z) > be.z + ae.x*m.z.x + ae.y*m.z.y + ae.z*m.z.z)return false; // b.x axis

   if(Abs(bo.z*obox.matrix.x.y - bo.y*obox.matrix.x.z) > ae.y*m.x.z + ae.z*m.x.y + be.y*m.z.x + be.z*m.y.x)return false; // a.x X b.x axis
   if(Abs(bo.z*obox.matrix.y.y - bo.y*obox.matrix.y.z) > ae.y*m.y.z + ae.z*m.y.y + be.x*m.z.x + be.z*m.x.x)return false; // a.x X b.y axis
   if(Abs(bo.z*obox.matrix.z.y - bo.y*obox.matrix.z.z) > ae.y*m.z.z + ae.z*m.z.y + be.x*m.y.x + be.y*m.x.x)return false; // a.x X b.z axis
   if(Abs(bo.x*obox.matrix.x.z - bo.z*obox.matrix.x.x) > ae.x*m.x.z + ae.z*m.x.x + be.y*m.z.y + be.z*m.y.y)return false; // a.y X b.x axis
   if(Abs(bo.x*obox.matrix.y.z - bo.z*obox.matrix.y.x) > ae.x*m.y.z + ae.z*m.y.x + be.x*m.z.y + be.z*m.x.y)return false; // a.y X b.y axis
   if(Abs(bo.x*obox.matrix.z.z - bo.z*obox.matrix.z.x) > ae.x*m.z.z + ae.z*m.z.x + be.x*m.y.y + be.y*m.x.y)return false; // a.y X b.z axis
   if(Abs(bo.y*obox.matrix.x.x - bo.x*obox.matrix.x.y) > ae.x*m.x.y + ae.y*m.x.x + be.y*m.z.z + be.z*m.y.z)return false; // a.z X b.x axis
   if(Abs(bo.y*obox.matrix.y.x - bo.x*obox.matrix.y.y) > ae.x*m.y.y + ae.y*m.y.x + be.x*m.z.z + be.z*m.x.z)return false; // a.z X b.y axis
   if(Abs(bo.y*obox.matrix.z.x - bo.x*obox.matrix.z.y) > ae.x*m.z.y + ae.y*m.z.x + be.x*m.y.z + be.y*m.x.z)return false; // a.z X b.z axis

   return true;
}
/******************************************************************************/
Bool CutsEps(C Box &a, C Box &b)
{
   return a.min.x<=b.max.x+EPS && a.max.x>=b.min.x-EPS
       && a.min.y<=b.max.y+EPS && a.max.y>=b.min.y-EPS
       && a.min.z<=b.max.z+EPS && a.max.z>=b.min.z-EPS;
}
Bool CutsEps(C BoxD &a, C BoxD &b)
{
   return a.min.x<=b.max.x+EPSD && a.max.x>=b.min.x-EPSD
       && a.min.y<=b.max.y+EPSD && a.max.y>=b.min.y-EPSD
       && a.min.z<=b.max.z+EPSD && a.max.z>=b.min.z-EPSD;
}
/******************************************************************************/
Bool Inside(C Box &a, C Box &b)
{
   return a.min.x>=b.min.x && a.max.x<=b.max.x
       && a.min.y>=b.min.y && a.max.y<=b.max.y
       && a.min.z>=b.min.z && a.max.z<=b.max.z;
}
Bool Inside(C BoxD &a, C BoxD &b)
{
   return a.min.x>=b.min.x && a.max.x<=b.max.x
       && a.min.y>=b.min.y && a.max.y<=b.max.y
       && a.min.z>=b.min.z && a.max.z<=b.max.z;
}
Bool Inside(C BoxI &a, C BoxI &b)
{
   return a.min.x>=b.min.x && a.max.x<=b.max.x
       && a.min.y>=b.min.y && a.max.y<=b.max.y
       && a.min.z>=b.min.z && a.max.z<=b.max.z;
}
Bool InsideEps(C Box &a, C Box &b)
{
   return a.min.x>=b.min.x-EPS && a.max.x<=b.max.x+EPS
       && a.min.y>=b.min.y-EPS && a.max.y<=b.max.y+EPS
       && a.min.z>=b.min.z-EPS && a.max.z<=b.max.z+EPS;
}
/******************************************************************************/
Bool SweepPointBox(C Vec &point, C Vec &move, C Box &box, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos)
{
   if(Cuts(point, box)){if(hit_frac)*hit_frac=0; if(hit_normal)hit_normal->zero(); if(hit_pos)*hit_pos=point; return true;}
   Flt frac;
   Vec test;
   if(point.x<box.min.x && point.x+move.x>=box.min.x){frac=(box.min.x-point.x)/move.x; test.set(box.min.x, point.y+move.y*frac, point.z+move.z*frac); if(                         box.includesY(test.y) && box.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(-1, 0, 0); if(hit_pos)*hit_pos=test; return true;}}else
   if(point.x>box.max.x && point.x+move.x<=box.max.x){frac=(box.max.x-point.x)/move.x; test.set(box.max.x, point.y+move.y*frac, point.z+move.z*frac); if(                         box.includesY(test.y) && box.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set( 1, 0, 0); if(hit_pos)*hit_pos=test; return true;}}
   if(point.y<box.min.y && point.y+move.y>=box.min.y){frac=(box.min.y-point.y)/move.y; test.set(point.x+move.x*frac, box.min.y, point.z+move.z*frac); if(box.includesX(test.x) &&                          box.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, -1, 0); if(hit_pos)*hit_pos=test; return true;}}else
   if(point.y>box.max.y && point.y+move.y<=box.max.y){frac=(box.max.y-point.y)/move.y; test.set(point.x+move.x*frac, box.max.y, point.z+move.z*frac); if(box.includesX(test.x) &&                          box.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0,  1, 0); if(hit_pos)*hit_pos=test; return true;}}
   if(point.z<box.min.z && point.z+move.z>=box.min.z){frac=(box.min.z-point.z)/move.z; test.set(point.x+move.x*frac, point.y+move.y*frac, box.min.z); if(box.includesX(test.x) && box.includesY(test.y)                         ){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, 0, -1); if(hit_pos)*hit_pos=test; return true;}}else
   if(point.z>box.max.z && point.z+move.z<=box.max.z){frac=(box.max.z-point.z)/move.z; test.set(point.x+move.x*frac, point.y+move.y*frac, box.max.z); if(box.includesX(test.x) && box.includesY(test.y)                         ){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, 0,  1); if(hit_pos)*hit_pos=test; return true;}}
   return false;
}
Bool SweepPointBox(C Vec &point, C Vec &move, C Extent &ext, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos)
{
   if(Cuts(point, ext)){if(hit_frac)*hit_frac=0; if(hit_normal)hit_normal->zero(); if(hit_pos)*hit_pos=point; return true;}
   Flt frac, border;
   Vec test;
        border=ext.minX(); if(point.x<border && point.x+move.x>=border){frac=(border-point.x)/move.x; test.set(border, point.y+move.y*frac, point.z+move.z*frac); if(                         ext.includesY(test.y) && ext.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(-1, 0, 0); if(hit_pos)*hit_pos=test; return true;}}
   else{border=ext.maxX(); if(point.x>border && point.x+move.x<=border){frac=(border-point.x)/move.x; test.set(border, point.y+move.y*frac, point.z+move.z*frac); if(                         ext.includesY(test.y) && ext.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set( 1, 0, 0); if(hit_pos)*hit_pos=test; return true;}}}
        border=ext.minY(); if(point.y<border && point.y+move.y>=border){frac=(border-point.y)/move.y; test.set(point.x+move.x*frac, border, point.z+move.z*frac); if(ext.includesX(test.x) &&                          ext.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, -1, 0); if(hit_pos)*hit_pos=test; return true;}}
   else{border=ext.maxY(); if(point.y>border && point.y+move.y<=border){frac=(border-point.y)/move.y; test.set(point.x+move.x*frac, border, point.z+move.z*frac); if(ext.includesX(test.x) &&                          ext.includesZ(test.z)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0,  1, 0); if(hit_pos)*hit_pos=test; return true;}}}
        border=ext.minZ(); if(point.z<border && point.z+move.z>=border){frac=(border-point.z)/move.z; test.set(point.x+move.x*frac, point.y+move.y*frac, border); if(ext.includesX(test.x) && ext.includesY(test.y)                         ){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, 0, -1); if(hit_pos)*hit_pos=test; return true;}}
   else{border=ext.maxZ(); if(point.z>border && point.z+move.z<=border){frac=(border-point.z)/move.z; test.set(point.x+move.x*frac, point.y+move.y*frac, border); if(ext.includesX(test.x) && ext.includesY(test.y)                         ){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, 0,  1); if(hit_pos)*hit_pos=test; return true;}}}
   return false;
}
Bool SweepPointBox(C Vec &point, C Vec &move, C OBox &obox, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos)
{
   Vec om_point; om_point.fromDivNormalized(point, obox.matrix      );
   Vec om_move ; om_move .fromDivNormalized(move , obox.matrix.orn());
   Flt frac;
   if(SweepPointBox(om_point, om_move, obox.box, &frac, hit_normal))
   {
      if(hit_frac  )*hit_frac   =frac;
      if(hit_pos   )*hit_pos    =point+move*frac;
      if(hit_normal)*hit_normal*=obox.matrix.orn();
      return true;
   }
   return false;
}
/******************************************************************************/
Flt  Dist (C Vec  &point, C OBox &obox) {return Dist (Vec ().fromDivNormalized(point, obox.matrix), obox.box);}
Flt  Dist2(C Vec  &point, C OBox &obox) {return Dist2(Vec ().fromDivNormalized(point, obox.matrix), obox.box);}
Bool Cuts (C Vec  &point, C OBox &obox) {return Cuts (Vec ().fromDivNormalized(point, obox.matrix), obox.box);}
Flt  Dist (C Edge &edge , C OBox &obox) {return Dist (Edge(edge).divNormalized(       obox.matrix), obox.box);}
Bool Cuts (C Edge &edge , C OBox &obox) {return Cuts (Edge(edge).divNormalized(       obox.matrix), obox.box);}
Bool Cuts (C OBox &a    , C OBox &b   ) {return Cuts (a.box, OBox(b).div(a.matrix, true));}
/******************************************************************************/
}
/******************************************************************************/
