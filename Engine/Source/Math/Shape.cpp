/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Shape& Shape::operator+=(C Vec &v)
{
   switch(type)
   {
      case SHAPE_POINT  : point  +=v   ; break;
      case SHAPE_EDGE   : edge   +=v   ; break;
      case SHAPE_RECT   : rect   +=v   ; break;
      case SHAPE_BOX    : box    +=v   ; break;
      case SHAPE_OBOX   : obox   +=v   ; break;
      case SHAPE_CIRCLE : circle +=v.xy; break;
      case SHAPE_BALL   : ball   +=v   ; break;
      case SHAPE_CAPSULE: capsule+=v   ; break;
      case SHAPE_TUBE   : tube   +=v   ; break;
      case SHAPE_TORUS  : torus  +=v   ; break;
      case SHAPE_CONE   : cone   +=v   ; break;
      case SHAPE_PYRAMID: pyramid+=v   ; break;
   }
   return T;
}
Shape& Shape::operator-=(C Vec &v)
{
   switch(type)
   {
      case SHAPE_POINT  : point  -=v   ; break;
      case SHAPE_EDGE   : edge   -=v   ; break;
      case SHAPE_RECT   : rect   -=v   ; break;
      case SHAPE_BOX    : box    -=v   ; break;
      case SHAPE_OBOX   : obox   -=v   ; break;
      case SHAPE_CIRCLE : circle -=v.xy; break;
      case SHAPE_BALL   : ball   -=v   ; break;
      case SHAPE_CAPSULE: capsule-=v   ; break;
      case SHAPE_TUBE   : tube   -=v   ; break;
      case SHAPE_TORUS  : torus  -=v   ; break;
      case SHAPE_CONE   : cone   -=v   ; break;
      case SHAPE_PYRAMID: pyramid-=v   ; break;
   }
   return T;
}
Shape& Shape::operator*=(Flt f)
{
   switch(type)
   {
      case SHAPE_POINT  : point  *=f; break;
      case SHAPE_EDGE   : edge   *=f; break;
      case SHAPE_RECT   : rect   *=f; break;
      case SHAPE_BOX    : box    *=f; break;
      case SHAPE_OBOX   : obox   *=f; break;
      case SHAPE_CIRCLE : circle *=f; break;
      case SHAPE_BALL   : ball   *=f; break;
      case SHAPE_CAPSULE: capsule*=f; break;
      case SHAPE_TUBE   : tube   *=f; break;
      case SHAPE_TORUS  : torus  *=f; break;
      case SHAPE_CONE   : cone   *=f; break;
      case SHAPE_PYRAMID: pyramid*=f; break;
   }
   return T;
}
Shape& Shape::operator/=(Flt f)
{
   switch(type)
   {
      case SHAPE_POINT  : point  /=f; break;
      case SHAPE_EDGE   : edge   /=f; break;
      case SHAPE_RECT   : rect   /=f; break;
      case SHAPE_BOX    : box    /=f; break;
      case SHAPE_OBOX   : obox   /=f; break;
      case SHAPE_CIRCLE : circle /=f; break;
      case SHAPE_BALL   : ball   /=f; break;
      case SHAPE_CAPSULE: capsule/=f; break;
      case SHAPE_TUBE   : tube   /=f; break;
      case SHAPE_TORUS  : torus  /=f; break;
      case SHAPE_CONE   : cone   /=f; break;
      case SHAPE_PYRAMID: pyramid/=f; break;
   }
   return T;
}
Shape& Shape::operator*=(C Vec &v)
{
   switch(type)
   {
      case SHAPE_POINT  : point  *=v; break;
      case SHAPE_EDGE   : edge   *=v; break;
      case SHAPE_RECT   : rect   *=v.xy; break;
      case SHAPE_BOX    : box    *=v; break;
      case SHAPE_OBOX   : obox   *=v; break;
      case SHAPE_CIRCLE : circle *=v.avg(); break;
      case SHAPE_BALL   : ball   *=v.avg(); break;
      case SHAPE_CAPSULE: capsule*=v; break;
      case SHAPE_TUBE   : tube   *=v; break;
      case SHAPE_TORUS  : torus  *=v.avg(); break;
      case SHAPE_CONE   : cone   *=v.avg(); break;
      case SHAPE_PYRAMID: pyramid*=v.avg(); break;
   }
   return T;
}
Shape& Shape::operator/=(C Vec &v)
{
   switch(type)
   {
      case SHAPE_POINT  : point  /=v; break;
      case SHAPE_EDGE   : edge   /=v; break;
      case SHAPE_RECT   : rect   /=v.xy; break;
      case SHAPE_BOX    : box    /=v; break;
      case SHAPE_OBOX   : obox   /=v; break;
      case SHAPE_CIRCLE : circle /=v.avg(); break;
      case SHAPE_BALL   : ball   /=v.avg(); break;
      case SHAPE_CAPSULE: capsule/=v; break;
      case SHAPE_TUBE   : tube   /=v; break;
      case SHAPE_TORUS  : torus  /=v.avg(); break;
      case SHAPE_CONE   : cone   /=v.avg(); break;
      case SHAPE_PYRAMID: pyramid/=v.avg(); break;
   }
   return T;
}
Shape& Shape::operator*=(C Matrix3 &m)
{
   switch(type)
   {
      case SHAPE_POINT  : point  *=m; break;
      case SHAPE_EDGE   : edge   *=m; break;
      case SHAPE_BOX    : box    *=m; break;
      case SHAPE_OBOX   : obox   *=m; break;
      case SHAPE_BALL   : ball   *=m; break;
      case SHAPE_CAPSULE: capsule*=m; break;
      case SHAPE_TUBE   : tube   *=m; break;
      case SHAPE_TORUS  : torus  *=m; break;
      case SHAPE_CONE   : cone   *=m; break;
      case SHAPE_PYRAMID: pyramid*=m; break;
   }
   return T;
}
Shape& Shape::operator*=(C Matrix &m)
{
   switch(type)
   {
      case SHAPE_POINT  : point  *=m; break;
      case SHAPE_EDGE   : edge   *=m; break;
      case SHAPE_BOX    : box    *=m; break;
      case SHAPE_OBOX   : obox   *=m; break;
      case SHAPE_BALL   : ball   *=m; break;
      case SHAPE_CAPSULE: capsule*=m; break;
      case SHAPE_TUBE   : tube   *=m; break;
      case SHAPE_TORUS  : torus  *=m; break;
      case SHAPE_CONE   : cone   *=m; break;
      case SHAPE_PYRAMID: pyramid*=m; break;
   }
   return T;
}
Shape operator+(C Shape &shape, C Vec     &v) {return Shape(shape)+=v;}
Shape operator-(C Shape &shape, C Vec     &v) {return Shape(shape)-=v;}
Shape operator*(C Shape &shape,   Flt      f) {return Shape(shape)*=f;}
Shape operator/(C Shape &shape,   Flt      f) {return Shape(shape)/=f;}
Shape operator*(C Shape &shape, C Vec     &v) {return Shape(shape)*=v;}
Shape operator/(C Shape &shape, C Vec     &v) {return Shape(shape)/=v;}
Shape operator*(C Shape &shape, C Matrix3 &m) {return Shape(shape)*=m;}
Shape operator*(C Shape &shape, C Matrix  &m) {return Shape(shape)*=m;}
/******************************************************************************/
Flt Shape::area()C
{
   switch(type)
   {
      case SHAPE_RECT   : return rect   .area();
      case SHAPE_BOX    : return box    .area();
      case SHAPE_OBOX   : return obox   .area();
      case SHAPE_CIRCLE : return circle .area();
      case SHAPE_BALL   : return ball   .area();
      case SHAPE_CAPSULE: return capsule.area();
      case SHAPE_TUBE   : return tube   .area();
      case SHAPE_TORUS  : return torus  .area();
      case SHAPE_CONE   : return cone   .area();
      case SHAPE_PYRAMID: return pyramid.area();
      default           : return 0;
   }
}
Flt Shape::volume()C
{
   switch(type)
   {
      case SHAPE_BOX    : return box    .volume();
      case SHAPE_OBOX   : return obox   .volume();
      case SHAPE_BALL   : return ball   .volume();
      case SHAPE_CAPSULE: return capsule.volume();
      case SHAPE_TUBE   : return tube   .volume();
      case SHAPE_TORUS  : return torus  .volume();
      case SHAPE_CONE   : return cone   .volume();
      case SHAPE_PYRAMID: return pyramid.volume();
      default           : return 0;
   }
}
Vec Shape::pos()C
{
   switch(type)
   {
      case SHAPE_POINT  : return     point               ;
      case SHAPE_EDGE   : return     edge   .center()    ;
      case SHAPE_RECT   : return Vec(rect   .center(), 0);
      case SHAPE_BOX    : return     box    .center()    ;
      case SHAPE_OBOX   : return     obox   .center()    ;
      case SHAPE_CIRCLE : return Vec(circle .pos     , 0);
      case SHAPE_BALL   : return     ball   .pos         ;
      case SHAPE_CAPSULE: return     capsule.pos         ;
      case SHAPE_TUBE   : return     tube   .pos         ;
      case SHAPE_TORUS  : return     torus  .pos         ;
      case SHAPE_CONE   : return     cone   .pos         ;
      case SHAPE_PYRAMID: return     pyramid.pos         ;
      default           : return                       0 ;
   }
}
void Shape::pos(C Vec &pos)
{
   T+=pos-T.pos();
}
/******************************************************************************/
Str Shape::asText(Bool include_shape_type_name)C
{
   Str text;
   switch(type)
   {
      default           : return S;
      case SHAPE_POINT  : text=point           ; break;
      case SHAPE_EDGE   : text=edge   .asText(); break;
      case SHAPE_RECT   : text=rect   .asText(); break;
      case SHAPE_BOX    : text=box    .asText(); break;
      case SHAPE_OBOX   : text=obox   .asText(); break;
      case SHAPE_CIRCLE : text=circle .asText(); break;
      case SHAPE_BALL   : text=ball   .asText(); break;
      case SHAPE_CAPSULE: text=capsule.asText(); break;
      case SHAPE_TUBE   : text=tube   .asText(); break;
      case SHAPE_TORUS  : text=torus  .asText(); break;
      case SHAPE_CONE   : text=cone   .asText(); break;
      case SHAPE_PYRAMID: text=pyramid.asText(); break;
      case SHAPE_PLANE  : text=plane  .asText(); break;
   }
   return include_shape_type_name ? S+ShapeTypeName(type)+'('+text+')' : text;
}
Matrix Shape::asMatrix()C
{
   switch(type)
   {
      default           : return MatrixIdentity;
      case SHAPE_POINT  : return                    point;
      case SHAPE_EDGE   : return Matrix().setPosDir(edge.p[0], edge.dir());
      case SHAPE_RECT   : return                Vec(rect   .center(), 0);
      case SHAPE_BOX    : return                    box    .center();
      case SHAPE_OBOX   : return Matrix(            obox   .center(), obox.matrix.orn());
      case SHAPE_CIRCLE : return                Vec(circle .pos     , 0);
      case SHAPE_BALL   : return                    ball   .pos;
      case SHAPE_CAPSULE: return Matrix().setPosUp( capsule.pos, capsule.up);
      case SHAPE_TUBE   : return Matrix().setPosUp( tube   .pos, tube   .up);
      case SHAPE_TORUS  : return Matrix().setPosUp( torus  .pos, torus  .up);
      case SHAPE_CONE   : return Matrix().setPosUp( cone   .pos, cone   .up);
      case SHAPE_PYRAMID: return                    pyramid;
      case SHAPE_PLANE  : return Matrix().setPosUp( plane.pos  , plane  .normal);
   }
}
Matrix Shape::asMatrixScaled()C
{
   Matrix m;
   switch(type)
   {
      default           : return MatrixIdentity;
      case SHAPE_POINT  : return point;
      case SHAPE_EDGE   : m.setPosDir(edge.p[0], edge.dir()); m.z*=edge.length(); return m;
      case SHAPE_RECT   : m=Vec   (rect   .center(), 0);                 m.x*=    rect.w()*0.5f; m.y*=    rect.h()*0.5f; return m;
      case SHAPE_BOX    : m=       box    .center();                     m.x*=     box.w()*0.5f; m.y*=     box.h()*0.5f; m.z*=     box.d()*0.5f; return m;
      case SHAPE_OBOX   : m=Matrix(obox   .center(), obox.matrix.orn()); m.x*=obox.box.w()*0.5f; m.y*=obox.box.h()*0.5f; m.z*=obox.box.d()*0.5f; return m;
      case SHAPE_CIRCLE : return Matrix(circle.r, Vec(circle.pos, 0));
      case SHAPE_BALL   : return Matrix(ball  .r,     ball  .pos    );
      case SHAPE_CAPSULE: m.setPosUp(capsule.pos, capsule.up); m.x*=    capsule.r                  ; m.y*=capsule.h*0.5f; m.z*=    capsule.r                  ; return m;
      case SHAPE_TUBE   : m.setPosUp(tube   .pos, tube   .up); m.x*=    tube   .r                  ; m.y*=tube   .h*0.5f; m.z*=    tube   .r                  ; return m;
      case SHAPE_TORUS  : m.setPosUp(torus  .pos, torus  .up); m.x*=    torus  .R    +torus.r      ; m.y*=torus  .r     ; m.z*=    torus  .R    +torus.r      ; return m;
      case SHAPE_CONE   : m.setPosUp(cone   .pos, cone   .up); m.x*=Max(cone   .r_low, cone.r_high); m.y*=cone   .h     ; m.z*=Max(cone   .r_low, cone.r_high); return m;
      case SHAPE_PYRAMID: m=pyramid; m.x*=pyramid.scale*pyramid.h; m.y*=pyramid.scale*pyramid.h; m.z*=pyramid.h; return m;
      case SHAPE_PLANE  : return Matrix().setPosUp(plane.pos, plane.normal);
   }
}
/******************************************************************************/
Shape& Shape::extend(Flt e)
{
   switch(type)
   {
      case SHAPE_RECT   : rect   .extend(e); break;
      case SHAPE_BOX    : box    .extend(e); break;
      case SHAPE_OBOX   : obox   .extend(e); break;
      case SHAPE_CIRCLE : circle .extend(e); break;
      case SHAPE_BALL   : ball   .extend(e); break;
      case SHAPE_CAPSULE: capsule.extend(e); break;
      case SHAPE_TUBE   : tube   .extend(e); break;
      case SHAPE_TORUS  : torus  .extend(e); break;
   }
   return T;
}
/******************************************************************************/
Shape& Shape::mirrorX()
{
   switch(type)
   {
      case SHAPE_POINT  : CHS(point.x); break;
      case SHAPE_EDGE   : CHS(edge.p[0].x); CHS(edge.p[1].x); break;
      case SHAPE_RECT   : CHS(rect.min.x ); CHS(rect.max.x ); Swap(rect.min.x, rect.max.x); break;
      case SHAPE_BOX    : CHS(box .min.x ); CHS(box .max.x ); Swap(box .min.x, box .max.x); break;
      case SHAPE_OBOX   : obox.mirrorX(); break;
      case SHAPE_CIRCLE : CHS(circle .pos.x); break;
      case SHAPE_BALL   : CHS(ball   .pos.x); break;
      case SHAPE_CAPSULE: CHS(capsule.pos.x); CHS(capsule.up.x); break;
      case SHAPE_TUBE   : CHS(tube   .pos.x); CHS(tube   .up.x); break;
      case SHAPE_TORUS  : CHS(torus  .pos.x); CHS(torus  .up.x); break;
      case SHAPE_CONE   : CHS(cone   .pos.x); CHS(cone   .up.x); break;
      case SHAPE_PYRAMID: pyramid.mirrorX(); break;
      case SHAPE_PLANE  : CHS(plane.pos.x); CHS(plane.normal.x); break;
   }
   return T;
}
Shape& Shape::mirrorY()
{
   switch(type)
   {
      case SHAPE_POINT  : CHS(point.y); break;
      case SHAPE_EDGE   : CHS(edge.p[0].y); CHS(edge.p[1].y); break;
      case SHAPE_RECT   : CHS(rect.min.y ); CHS(rect.max.y ); Swap(rect.min.y, rect.max.y); break;
      case SHAPE_BOX    : CHS(box .min.y ); CHS(box .max.y ); Swap(box .min.y, box .max.y); break;
      case SHAPE_OBOX   : obox.mirrorY(); break;
      case SHAPE_CIRCLE : CHS(circle .pos.y); break;
      case SHAPE_BALL   : CHS(ball   .pos.y); break;
      case SHAPE_CAPSULE: CHS(capsule.pos.y); CHS(capsule.up.y); break;
      case SHAPE_TUBE   : CHS(tube   .pos.y); CHS(tube   .up.y); break;
      case SHAPE_TORUS  : CHS(torus  .pos.y); CHS(torus  .up.y); break;
      case SHAPE_CONE   : CHS(cone   .pos.y); CHS(cone   .up.y); break;
      case SHAPE_PYRAMID: pyramid.mirrorY(); break;
      case SHAPE_PLANE  : CHS(plane.pos.y); CHS(plane.normal.y); break;
   }
   return T;
}
Shape& Shape::mirrorZ()
{
   switch(type)
   {
      case SHAPE_POINT  : CHS(point.z); break;
      case SHAPE_EDGE   : CHS(edge.p[0].z); CHS(edge.p[1].z); break;
    //case SHAPE_RECT   : CHS(rect.min.z ); CHS(rect.max.z ); Swap(rect.min.z, rect.max.z); break; // Rect is 2D and doesn't have Z
      case SHAPE_BOX    : CHS(box .min.z ); CHS(box .max.z ); Swap(box .min.z, box .max.z); break;
      case SHAPE_OBOX   : obox.mirrorZ(); break;
    //case SHAPE_CIRCLE : CHS(circle .pos.z); break; // Circle is 2D and doesn't have Z
      case SHAPE_BALL   : CHS(ball   .pos.z); break;
      case SHAPE_CAPSULE: CHS(capsule.pos.z); CHS(capsule.up.z); break;
      case SHAPE_TUBE   : CHS(tube   .pos.z); CHS(tube   .up.z); break;
      case SHAPE_TORUS  : CHS(torus  .pos.z); CHS(torus  .up.z); break;
      case SHAPE_CONE   : CHS(cone   .pos.z); CHS(cone   .up.z); break;
      case SHAPE_PYRAMID: pyramid.mirrorZ(); break;
      case SHAPE_PLANE  : CHS(plane.pos.z); CHS(plane.normal.z); break;
   }
   return T;
}
/******************************************************************************/
void Shape::draw(C Color &color, Bool fill)C
{
   switch(type)
   {
      case SHAPE_POINT  : point  .draw(color      ); break;
      case SHAPE_EDGE   : edge   .draw(color      ); break;
      case SHAPE_RECT   : rect   .draw(color, fill); break;
      case SHAPE_BOX    : box    .draw(color, fill); break;
      case SHAPE_OBOX   : obox   .draw(color, fill); break;
      case SHAPE_CIRCLE : circle .draw(color, fill); break;
      case SHAPE_BALL   : ball   .draw(color, fill); break;
      case SHAPE_CAPSULE: capsule.draw(color, fill); break;
      case SHAPE_TUBE   : tube   .draw(color, fill); break;
      case SHAPE_TORUS  : torus  .draw(color, fill); break;
      case SHAPE_CONE   : cone   .draw(color, fill); break;
      case SHAPE_PYRAMID: pyramid.draw(color, fill); break;
      case SHAPE_PLANE  : plane  .drawLocal(color, 10, fill); break;
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Shape::save(File &f)C
{
   f.putMulti(Byte(0), type); // version
   switch(type)
   {
      case SHAPE_NONE   :             break;
      case SHAPE_POINT  : f<<point  ; break;
      case SHAPE_EDGE   : f<<edge   ; break;
      case SHAPE_RECT   : f<<rect   ; break;
      case SHAPE_BOX    : f<<box    ; break;
      case SHAPE_OBOX   : f<<obox   ; break;
      case SHAPE_CIRCLE : f<<circle ; break;
      case SHAPE_BALL   : f<<ball   ; break;
      case SHAPE_CAPSULE: f<<capsule; break;
      case SHAPE_TUBE   : f<<tube   ; break;
      case SHAPE_TORUS  : f<<torus  ; break;
      case SHAPE_CONE   : f<<cone   ; break;
      case SHAPE_PYRAMID: f<<pyramid; break;
      case SHAPE_PLANE  : f<<plane  ; break;
      default           : return false;
   }
   return f.ok();
}
/******************************************************************************/
Bool Shape::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>type; switch(type)
         {
            case SHAPE_NONE   :             break;
            case SHAPE_POINT  : f>>point  ; break;
            case SHAPE_EDGE   : f>>edge   ; break;
            case SHAPE_RECT   : f>>rect   ; break;
            case SHAPE_BOX    : f>>box    ; break;
            case SHAPE_OBOX   : f>>obox   ; break;
            case SHAPE_CIRCLE : f>>circle ; break;
            case SHAPE_BALL   : f>>ball   ; break;
            case SHAPE_CAPSULE: f>>capsule; break;
            case SHAPE_TUBE   : f>>tube   ; break;
            case SHAPE_TORUS  : f>>torus  ; break;
            case SHAPE_CONE   : f>>cone   ; break;
            case SHAPE_PYRAMID: f>>pyramid; break;
            case SHAPE_PLANE  : f>>plane  ; break;
            default           : goto error;
         }
         if(f.ok())return true;
      }break;
   }
error:
   type=SHAPE_NONE; return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool ShapeType2D(SHAPE_TYPE type)
{
   switch(type)
   {
      case SHAPE_RECT  :
      case SHAPE_CIRCLE: return true ;
      default          : return false;
   }
}
Bool ShapeTypeRound(SHAPE_TYPE type)
{
   switch(type)
   {
      case SHAPE_CIRCLE :
      case SHAPE_BALL   :
      case SHAPE_CAPSULE:
      case SHAPE_TUBE   :
      case SHAPE_TORUS  :
      case SHAPE_CONE   : return true ;
      default           : return false;
   }
}
CChar* ShapeTypeName(SHAPE_TYPE type)
{
   switch(type)
   {
      case SHAPE_POINT  : return u"Point";
      case SHAPE_EDGE   : return u"Edge";
      case SHAPE_RECT   : return u"Rectangle";
      case SHAPE_BOX    : return u"Box";
      case SHAPE_OBOX   : return u"OrientedBox";
      case SHAPE_CIRCLE : return u"Circle";
      case SHAPE_BALL   : return u"Ball";
      case SHAPE_CAPSULE: return u"Capsule";
      case SHAPE_TUBE   : return u"Tube";
      case SHAPE_TORUS  : return u"Torus";
      case SHAPE_CONE   : return u"Cone";
      case SHAPE_PYRAMID: return u"Pyramid";
      case SHAPE_PLANE  : return u"Plane";
      default           : return S;
   }
}
/******************************************************************************/
Bool SweepPointShape(C Vec &point, C Vec &move, C Shape &shape, Flt *hit_frac, Vec *hit_normal)
{
   switch(shape.type)
   {
    //case SHAPE_EDGE   : if(hit_normal)hit_normal->z=0; return SweepPointEdge   (point.xy, move.xy, shape.edge2  , hit_frac, &hit_normal->xy);
      case SHAPE_RECT   : if(hit_normal)hit_normal->z=0; return SweepPointRect   (point.xy, move.xy, shape.rect   , hit_frac, &hit_normal->xy);
      case SHAPE_BOX    :                                return SweepPointBox    (point   , move   , shape.box    , hit_frac,  hit_normal);
      case SHAPE_OBOX   :                                return SweepPointBox    (point   , move   , shape.obox   , hit_frac,  hit_normal);
      case SHAPE_CIRCLE : if(hit_normal)hit_normal->z=0; return SweepPointCircle (point.xy, move.xy, shape.circle , hit_frac, &hit_normal->xy);
      case SHAPE_BALL   :                                return SweepPointBall   (point   , move   , shape.ball   , hit_frac,  hit_normal);
      case SHAPE_CAPSULE:                                return SweepPointCapsule(point   , move   , shape.capsule, hit_frac,  hit_normal);
      case SHAPE_TUBE   :                                return SweepPointTube   (point   , move   , shape.tube   , hit_frac,  hit_normal);
    //case SHAPE_TORUS  :                                return SweepPointTorus  (point   , move   , shape.torus  , hit_frac,  hit_normal);
    //case SHAPE_CONE   :                                return SweepPointCone   (point   , move   , shape.cone   , hit_frac,  hit_normal);
    //case SHAPE_PYRAMID:                                return SweepPointPyramid(point   , move   , shape.pyramid, hit_frac,  hit_normal);
      case SHAPE_PLANE  :                                return SweepPointPlane  (point   , move   , shape.plane  , hit_frac,  hit_normal);
      default           : return false;
   }
}
/******************************************************************************/
Flt Dist(C Shape &a, C Shape &b)
{
   switch(a.type)
   {
      case SHAPE_POINT: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(a.point   , b.point  );
         case SHAPE_RECT   : return Dist(a.point.xy, b.rect   );
         case SHAPE_BOX    : return Dist(a.point   , b.box    );
         case SHAPE_OBOX   : return Dist(a.point   , b.obox   );
         case SHAPE_CIRCLE : return Dist(a.point.xy, b.circle );
         case SHAPE_BALL   : return Dist(a.point   , b.ball   );
         case SHAPE_CAPSULE: return Dist(a.point   , b.capsule);
         case SHAPE_TORUS  : return Dist(a.point   , b.torus  );
         case SHAPE_PLANE  : return Dist(a.point   , b.plane  );
      }break;

      case SHAPE_EDGE: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point, a.edge   ); // swapped
         case SHAPE_EDGE   : return Dist(a.edge , b.edge   );
         case SHAPE_BOX    : return Dist(a.edge , b.box    );
         case SHAPE_OBOX   : return Dist(a.edge , b.obox   );
         case SHAPE_BALL   : return Dist(a.edge , b.ball   );
         case SHAPE_CAPSULE: return Dist(a.edge , b.capsule);
         case SHAPE_PLANE  : return Dist(a.edge , b.plane  );
      }break;

      case SHAPE_RECT: switch(b.type)
      {
         case SHAPE_POINT : return Dist(b.point.xy, a.rect  ); // swapped
         case SHAPE_RECT  : return Dist(a.rect    , b.rect  );
         case SHAPE_CIRCLE: return Dist(a.rect    , b.circle);
      }break;

      case SHAPE_BOX: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point, a.box    ); // swapped
         case SHAPE_EDGE   : return Dist(b.edge , a.box    ); // swapped
         case SHAPE_BOX    : return Dist(a.box  , b.box    );
         case SHAPE_BALL   : return Dist(a.box  , b.ball   );
         case SHAPE_CAPSULE: return Dist(a.box  , b.capsule);
         case SHAPE_PLANE  : return Dist(a.box  , b.plane  );
      }break;

      case SHAPE_OBOX: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point, a.obox   ); // swapped
         case SHAPE_EDGE   : return Dist(b.edge , a.obox   ); // swapped
         case SHAPE_BALL   : return Dist(a.obox , b.ball   );
         case SHAPE_CAPSULE: return Dist(a.obox , b.capsule);
         case SHAPE_PLANE  : return Dist(a.obox , b.plane  );
      }break;

      case SHAPE_CIRCLE: switch(b.type)
      {
         case SHAPE_POINT : return Dist(b.point.xy, a.circle); // swapped
         case SHAPE_RECT  : return Dist(b.rect    , a.circle); // swapped
         case SHAPE_CIRCLE: return Dist(a.circle  , b.circle);
      }break;

      case SHAPE_BALL: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point, a.ball   ); // swapped
         case SHAPE_EDGE   : return Dist(b.edge , a.ball   ); // swapped
         case SHAPE_BOX    : return Dist(b.box  , a.ball   ); // swapped
         case SHAPE_OBOX   : return Dist(b.obox , a.ball   ); // swapped
         case SHAPE_BALL   : return Dist(a.ball , b.ball   );
         case SHAPE_CAPSULE: return Dist(a.ball , b.capsule);
         case SHAPE_PLANE  : return Dist(a.ball , b.plane  );
      }break;

      case SHAPE_CAPSULE: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point  , a.capsule); // swapped
         case SHAPE_EDGE   : return Dist(b.edge   , a.capsule); // swapped
         case SHAPE_BOX    : return Dist(b.box    , a.capsule); // swapped
         case SHAPE_OBOX   : return Dist(b.obox   , a.capsule); // swapped
         case SHAPE_BALL   : return Dist(b.ball   , a.capsule); // swapped
         case SHAPE_CAPSULE: return Dist(a.capsule, b.capsule);
         case SHAPE_PLANE  : return Dist(a.capsule, b.plane  );
      }break;

      case SHAPE_PLANE: switch(b.type)
      {
         case SHAPE_POINT  : return Dist(b.point  , a.plane); // swapped
         case SHAPE_EDGE   : return Dist(b.edge   , a.plane); // swapped
         case SHAPE_BOX    : return Dist(b.box    , a.plane); // swapped
         case SHAPE_OBOX   : return Dist(b.obox   , a.plane); // swapped
         case SHAPE_BALL   : return Dist(b.ball   , a.plane); // swapped
         case SHAPE_CAPSULE: return Dist(b.capsule, a.plane); // swapped
      }break;

      case SHAPE_TORUS: switch(b.type)
      {
         case SHAPE_POINT: return Dist(b.point, a.torus); // swapped
      }break;
   }
   return 0;
}
/******************************************************************************/
Bool Cuts(C Vec2 &p, C Shape &s)
{
   switch(s.type)
   {
      case SHAPE_RECT  : return Cuts(p, s.rect  );
      case SHAPE_CIRCLE: return Cuts(p, s.circle);
   }
   return false;
}
Bool Cuts(C Vec &p, C Shape &s)
{
   switch(s.type)
   {
      case SHAPE_RECT   : return Cuts(p.xy, s.rect   );
      case SHAPE_BOX    : return Cuts(p   , s.box    );
      case SHAPE_OBOX   : return Cuts(p   , s.obox   );
      case SHAPE_CIRCLE : return Cuts(p.xy, s.circle );
      case SHAPE_BALL   : return Cuts(p   , s.ball   );
      case SHAPE_CAPSULE: return Cuts(p   , s.capsule);
      case SHAPE_TUBE   : return Cuts(p   , s.tube   );
      case SHAPE_CONE   : return Cuts(p   , s.cone   );
      case SHAPE_PYRAMID: return Cuts(p   , s.pyramid);
      case SHAPE_PLANE  : return Cuts(p   , s.plane  );
   }
   return false;
}
/******************************************************************************/
Bool Cuts(C Shape &a, C Shape &b)
{
   switch(a.type)
   {
      case SHAPE_POINT: switch(b.type)
      {
         case SHAPE_RECT   : return Cuts(a.point.xy, b.rect   );
         case SHAPE_BOX    : return Cuts(a.point   , b.box    );
         case SHAPE_OBOX   : return Cuts(a.point   , b.obox   );
         case SHAPE_CIRCLE : return Cuts(a.point.xy, b.circle );
         case SHAPE_BALL   : return Cuts(a.point   , b.ball   );
         case SHAPE_CAPSULE: return Cuts(a.point   , b.capsule);
         case SHAPE_TUBE   : return Cuts(a.point   , b.tube   );
         case SHAPE_CONE   : return Cuts(a.point   , b.cone   );
         case SHAPE_PYRAMID: return Cuts(a.point   , b.pyramid);
         case SHAPE_PLANE  : return Cuts(a.point   , b.plane  );
      }break;

      case SHAPE_EDGE: switch(b.type)
      {
         case SHAPE_BOX    : return Cuts(a.edge, b.box    );
         case SHAPE_OBOX   : return Cuts(a.edge, b.obox   );
         case SHAPE_BALL   : return Cuts(a.edge, b.ball   );
         case SHAPE_CAPSULE: return Cuts(a.edge, b.capsule);
         case SHAPE_TUBE   : return Cuts(a.edge, b.tube   );
      }break;

      case SHAPE_RECT: switch(b.type)
      {
         case SHAPE_POINT : return Cuts(b.point.xy, a.rect  ); // swapped
         case SHAPE_RECT  : return Cuts(a.rect    , b.rect  );
         case SHAPE_CIRCLE: return Cuts(a.rect    , b.circle);
         case SHAPE_BOX   : return Cuts(a.rect    , b.box   );
      }break;

      case SHAPE_BOX: switch(b.type)
      {
         case SHAPE_POINT  : return Cuts(b.point, a.box    ); // swapped
         case SHAPE_EDGE   : return Cuts(b.edge , a.box    ); // swapped
         case SHAPE_RECT   : return Cuts(b.rect , a.box    ); // swapped
         case SHAPE_BOX    : return Cuts(a.box  , b.box    );
         case SHAPE_OBOX   : return Cuts(a.box  , b.obox   );
         case SHAPE_BALL   : return Cuts(a.box  , b.ball   );
         case SHAPE_CAPSULE: return Cuts(a.box  , b.capsule);
      }break;

      case SHAPE_OBOX: switch(b.type)
      {
         case SHAPE_POINT  : return Cuts(b.point, a.obox   ); // swapped
         case SHAPE_EDGE   : return Cuts(b.edge , a.obox   ); // swapped
         case SHAPE_BOX    : return Cuts(b.box  , a.obox   ); // swapped
         case SHAPE_OBOX   : return Cuts(a.obox , b.obox   );
         case SHAPE_BALL   : return Cuts(a.obox , b.ball   );
         case SHAPE_CAPSULE: return Cuts(a.obox , b.capsule);
      }break;

      case SHAPE_CIRCLE: switch(b.type)
      {
         case SHAPE_POINT : return Cuts(b.point.xy, a.circle); // swapped
         case SHAPE_RECT  : return Cuts(b.rect    , a.circle); // swapped
         case SHAPE_CIRCLE: return Cuts(a.circle  , b.circle);
      }break;

      case SHAPE_BALL: switch(b.type)
      {
         case SHAPE_POINT  : return Cuts(b.point, a.ball   ); // swapped
         case SHAPE_EDGE   : return Cuts(b.edge , a.ball   ); // swapped
         case SHAPE_BOX    : return Cuts(b.box  , a.ball   ); // swapped
         case SHAPE_OBOX   : return Cuts(b.obox , a.ball   ); // swapped
         case SHAPE_BALL   : return Cuts(a.ball , b.ball   );
         case SHAPE_CAPSULE: return Cuts(a.ball , b.capsule);
      }break;

      case SHAPE_CAPSULE: switch(b.type)
      {
         case SHAPE_POINT  : return Cuts(b.point  , a.capsule); // swapped
         case SHAPE_EDGE   : return Cuts(b.edge   , a.capsule); // swapped
         case SHAPE_BOX    : return Cuts(b.box    , a.capsule); // swapped
         case SHAPE_OBOX   : return Cuts(b.obox   , a.capsule); // swapped
         case SHAPE_BALL   : return Cuts(b.ball   , a.capsule); // swapped
         case SHAPE_CAPSULE: return Cuts(a.capsule, b.capsule);
      }break;
      
      case SHAPE_TUBE: switch(b.type)
      {
         case SHAPE_POINT: return Cuts(b.point, a.tube); // swapped
         case SHAPE_EDGE : return Cuts(b.edge , a.tube); // swapped
      }break;

      case SHAPE_CONE: switch(b.type)
      {
         case SHAPE_POINT: return Cuts(b.point, a.cone); // swapped
      }break;

      case SHAPE_PYRAMID: switch(b.type)
      {
         case SHAPE_POINT: return Cuts(b.point, a.pyramid); // swapped
      }break;

      case SHAPE_PLANE: switch(b.type)
      {
         case SHAPE_POINT: return Cuts(b.point, a.plane); // swapped
      }break;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
