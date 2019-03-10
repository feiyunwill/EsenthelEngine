/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Rect::Rect(C RectI    &rect  ) {set (rect.min   , rect.max   );}
Rect::Rect(C RectD    &rect  ) {set (rect.min   , rect.max   );}
Rect::Rect(C Box      &box   ) {set (box .min.xy, box .max.xy);}
Rect::Rect(C Extent   &ext   ) {set (ext .minX(), ext .minY(), ext.maxX(), ext.maxY());}
Rect::Rect(C Circle   &circle) {setC(circle.pos , circle.r   );}
Rect::Rect(C Edge2    &edge  ) {from(edge.p[0]  , edge.p[1]  );}
Rect::Rect(C Tri2     &tri   ) {MinMax(tri .p, 3, min, max);}
Rect::Rect(C Quad2    &quad  ) {MinMax(quad.p, 4, min, max);}
Rect::Rect(C MeshBase &mshb  ) {mshb.getRect(T);}
/******************************************************************************/
Rect& Rect::operator&=(C Rect &r)
{
   if(r.min.x>min.x)min.x=r.min.x; if(r.max.x<max.x)max.x=r.max.x;
   if(r.min.y>min.y)min.y=r.min.y; if(r.max.y<max.y)max.y=r.max.y;
   return T;
}
Rect& Rect::operator&=(C RectI &r)
{
   if(r.min.x>min.x)min.x=r.min.x; if(r.max.x<max.x)max.x=r.max.x;
   if(r.min.y>min.y)min.y=r.min.y; if(r.max.y<max.y)max.y=r.max.y;
   return T;
}
RectD& RectD::operator&=(C RectD &r)
{
   if(r.min.x>min.x)min.x=r.min.x; if(r.max.x<max.x)max.x=r.max.x;
   if(r.min.y>min.y)min.y=r.min.y; if(r.max.y<max.y)max.y=r.max.y;
   return T;
}
RectD& RectD::operator&=(C RectI &r)
{
   if(r.min.x>min.x)min.x=r.min.x; if(r.max.x<max.x)max.x=r.max.x;
   if(r.min.y>min.y)min.y=r.min.y; if(r.max.y<max.y)max.y=r.max.y;
   return T;
}
RectI& RectI::operator&=(C RectI &r)
{
   if(r.min.x>min.x)min.x=r.min.x; if(r.max.x<max.x)max.x=r.max.x;
   if(r.min.y>min.y)min.y=r.min.y; if(r.max.y<max.y)max.y=r.max.y;
   return T;
}
/******************************************************************************/
Rect& Rect::extendX(  Flt   e) {min.x-=e; max.x+=e; return T;}
Rect& Rect::extendY(  Flt   e) {min.y-=e; max.y+=e; return T;}
Rect& Rect::extend (  Flt   e) {min  -=e; max  +=e; return T;}
Rect& Rect::extend (C Vec2 &v) {min  -=v; max  +=v; return T;}

RectD& RectD::extendX(  Dbl    e) {min.x-=e; max.x+=e; return T;}
RectD& RectD::extendY(  Dbl    e) {min.y-=e; max.y+=e; return T;}
RectD& RectD::extend (  Dbl    e) {min  -=e; max  +=e; return T;}
RectD& RectD::extend (C VecD2 &v) {min  -=v; max  +=v; return T;}

RectI& RectI::extendX(  Int    e) {min.x-=e; max.x+=e; return T;}
RectI& RectI::extendY(  Int    e) {min.y-=e; max.y+=e; return T;}
RectI& RectI::extend (  Int    e) {min  -=e; max  +=e; return T;}
RectI& RectI::extend (C VecI2 &v) {min  -=v; max  +=v; return T;}

Rect& Rect::extend(Flt x, Flt y)
{
   min.x-=x; max.x+=x;
   min.y-=y; max.y+=y;
   return T;
}
RectD& RectD::extend(Dbl x, Dbl y)
{
   min.x-=x; max.x+=x;
   min.y-=y; max.y+=y;
   return T;
}
RectI& RectI::extend(Int x, Int y)
{
   min.x-=x; max.x+=x;
   min.y-=y; max.y+=y;
   return T;
}
/******************************************************************************/
Rect & Rect ::includeX(Flt x           ) {if(  x<  min.x)  min.x=x;else if(  x>  max.x)  max.x=x  ; return T;}
RectD& RectD::includeX(Dbl x           ) {if(  x<  min.x)  min.x=x;else if(  x>  max.x)  max.x=x  ; return T;}
RectI& RectI::includeX(Int x           ) {if(  x<  min.x)  min.x=x;else if(  x>  max.x)  max.x=x  ; return T;}
Rect & Rect ::includeY(Flt y           ) {if(  y<  min.y)  min.y=y;else if(  y>  max.y)  max.y=y  ; return T;}
RectD& RectD::includeY(Dbl y           ) {if(  y<  min.y)  min.y=y;else if(  y>  max.y)  max.y=y  ; return T;}
RectI& RectI::includeY(Int y           ) {if(  y<  min.y)  min.y=y;else if(  y>  max.y)  max.y=y  ; return T;}
Rect & Rect ::includeX(Flt min, Flt max) {if(min<T.min.x)T.min.x=min;   if(max>T.max.x)T.max.x=max; return T;}
RectD& RectD::includeX(Dbl min, Dbl max) {if(min<T.min.x)T.min.x=min;   if(max>T.max.x)T.max.x=max; return T;}
RectI& RectI::includeX(Int min, Int max) {if(min<T.min.x)T.min.x=min;   if(max>T.max.x)T.max.x=max; return T;}
Rect & Rect ::includeY(Flt min, Flt max) {if(min<T.min.y)T.min.y=min;   if(max>T.max.y)T.max.y=max; return T;}
RectD& RectD::includeY(Dbl min, Dbl max) {if(min<T.min.y)T.min.y=min;   if(max>T.max.y)T.max.y=max; return T;}
RectI& RectI::includeY(Int min, Int max) {if(min<T.min.y)T.min.y=min;   if(max>T.max.y)T.max.y=max; return T;}

Rect& Rect::include(C Vec2 &v)
{
   Flt x=v.x, y=v.y;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   return T;
}
RectD& RectD::include(C VecD2 &v)
{
   Dbl x=v.x, y=v.y;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   return T;
}
RectI& RectI::include(C VecI2 &v)
{
   Int x=v.x, y=v.y;
   if(x<min.x)min.x=x;else if(x>max.x)max.x=x;
   if(y<min.y)min.y=y;else if(y>max.y)max.y=y;
   return T;
}

Rect& Rect::include(C Rect &r)
{
   if(r.min.x<min.x)min.x=r.min.x; if(r.max.x>max.x)max.x=r.max.x;
   if(r.min.y<min.y)min.y=r.min.y; if(r.max.y>max.y)max.y=r.max.y;
   return T;
}
RectD& RectD::include(C RectD &r)
{
   if(r.min.x<min.x)min.x=r.min.x; if(r.max.x>max.x)max.x=r.max.x;
   if(r.min.y<min.y)min.y=r.min.y; if(r.max.y>max.y)max.y=r.max.y;
   return T;
}
RectI& RectI::include(C RectI &r)
{
   if(r.min.x<min.x)min.x=r.min.x; if(r.max.x>max.x)max.x=r.max.x;
   if(r.min.y<min.y)min.y=r.min.y; if(r.max.y>max.y)max.y=r.max.y;
   return T;
}
/******************************************************************************/
Rect& Rect::moveX(Flt dx) {min.x+=dx; max.x+=dx; return T;}
Rect& Rect::moveY(Flt dy) {min.y+=dy; max.y+=dy; return T;}

RectD& RectD::moveX(Dbl dx) {min.x+=dx; max.x+=dx; return T;}
RectD& RectD::moveY(Dbl dy) {min.y+=dy; max.y+=dy; return T;}
/******************************************************************************/
Rect& Rect::swapX() {Swap(min.x, max.x); return T;}
Rect& Rect::swapY() {Swap(min.y, max.y); return T;}

RectD& RectD::swapX() {Swap(min.x, max.x); return T;}
RectD& RectD::swapY() {Swap(min.y, max.y); return T;}
/******************************************************************************/
Rect& Rect::from(C Vec2 &a, C Vec2 &b)
{
   MinMax(a.x, b.x, min.x, max.x);
   MinMax(a.y, b.y, min.y, max.y);
   return T;
}
RectD& RectD::from(C VecD2 &a, C VecD2 &b)
{
   MinMax(a.x, b.x, min.x, max.x);
   MinMax(a.y, b.y, min.y, max.y);
   return T;
}
RectI& RectI::from(C VecI2 &a, C VecI2 &b)
{
   MinMax(a.x, b.x, min.x, max.x);
   MinMax(a.y, b.y, min.y, max.y);
   return T;
}
/******************************************************************************/
Rect& Rect::rotatePI_2(Int rotations)
{
   switch(rotations&3)
   {
      case 1: set(-max.y,  min.x, -min.y,  max.x); break;
      case 2: set(-max.x, -max.y, -min.x, -min.y); break;
      case 3: set( min.y, -max.x,  max.y, -min.x); break;
   }
   return T;
}
RectD& RectD::rotatePI_2(Int rotations)
{
   switch(rotations&3)
   {
      case 1: set(-max.y,  min.x, -min.y,  max.x); break;
      case 2: set(-max.x, -max.y, -min.x, -min.y); break;
      case 3: set( min.y, -max.x,  max.y, -min.x); break;
   }
   return T;
}
RectI& RectI::rotatePI_2(Int rotations)
{
   switch(rotations&3)
   {
      case 1: set(-max.y,  min.x, -min.y,  max.x); break;
      case 2: set(-max.x, -max.y, -min.x, -min.y); break;
      case 3: set( min.y, -max.x,  max.y, -min.x); break;
   }
   return T;
}
/******************************************************************************/
void Rect::draw(C Color &color, Bool fill)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, fill ? VI_STRIP : VI_LINE|VI_STRIP);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(fill ? 4 : 5))
   {
      if(fill)
      {
         v[0].pos.set(min.x, max.y);
         v[1].pos.set(max.x, max.y);
         v[2].pos.set(min.x, min.y);
         v[3].pos.set(max.x, min.y);
      }else
      {
         // drawing lines needs adjustments
         Rect r(min.x+D._pixel_size_2.x, min.y+D._pixel_size_2.y,
                max.x-D._pixel_size_2.x, max.y-D._pixel_size_2.y);
         v[0].pos.set(r.min.x, r.min.y);
         v[1].pos.set(r.min.x, r.max.y);
         v[2].pos.set(r.max.x, r.max.y);
         v[3].pos.set(r.max.x, r.min.y);
         v[4].pos.set(r.min.x, r.min.y);
      }
   }
   VI.end();
}
void Rect::drawBorder(C Color &color, Flt border)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(8))
   {
      Flt x0=min.x, y0=min.y, x1=x0+border, y1=y0+border,
          x3=max.x, y3=max.y, x2=x3-border, y2=y3-border;
      v[0].pos.set(x0, y0);
      v[1].pos.set(x0, y3);
      v[2].pos.set(x3, y3);
      v[3].pos.set(x3, y0);
      v[4].pos.set(x1, y1);
      v[5].pos.set(x1, y2);
      v[6].pos.set(x2, y2);
      v[7].pos.set(x2, y1);

      VI.flushIndexed(IndBufRectBorder, 4*2*3);
   }
   VI.clear();
}
void Rect::drawBorder(C Color &color, C Vec2 &border)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(8))
   {
      Flt x0=min.x, y0=min.y, x1=x0+border.x, y1=y0+border.y,
          x3=max.x, y3=max.y, x2=x3-border.x, y2=y3-border.y;
      v[0].pos.set(x0, y0);
      v[1].pos.set(x0, y3);
      v[2].pos.set(x3, y3);
      v[3].pos.set(x3, y0);
      v[4].pos.set(x1, y1);
      v[5].pos.set(x1, y2);
      v[6].pos.set(x2, y2);
      v[7].pos.set(x2, y1);

      VI.flushIndexed(IndBufRectBorder, 4*2*3);
   }
   VI.clear();
}
void Rect::drawShadedX(C Color &c0, C Color &c1)C
{
   VI.setType(VI_2D_COL, VI_STRIP);
   if(Vtx2DCol *v=(Vtx2DCol*)VI.addVtx(4))
   {
      v[0].pos.set(min.x, max.y);
      v[1].pos.set(max.x, max.y);
      v[2].pos.set(min.x, min.y);
      v[3].pos.set(max.x, min.y);
      v[0].color=v[2].color=c0;
      v[1].color=v[3].color=c1;
   }
   VI.end();
}
void Rect::drawShadedY(C Color &c0, C Color &c1)C
{
   VI.setType(VI_2D_COL, VI_STRIP);
   if(Vtx2DCol *v=(Vtx2DCol*)VI.addVtx(4))
   {
      v[0].pos.set(min.x, max.y);
      v[1].pos.set(max.x, max.y);
      v[2].pos.set(min.x, min.y);
      v[3].pos.set(max.x, min.y);
      v[0].color=v[1].color=c0;
      v[2].color=v[3].color=c1;
   }
   VI.end();
}
void Rect::drawShaded(C Color &c0, C Color &c1, Flt border)C
{
   VI.setType(VI_2D_COL);
   if(Vtx2DCol *v=(Vtx2DCol*)VI.addVtx(8))
   {
      Flt x0=min.x, y0=min.y, x1=x0+border, y1=y0+border,
          x3=max.x, y3=max.y, x2=x3-border, y2=y3-border;
      v[0].pos.set(x0, y0);
      v[1].pos.set(x0, y3);
      v[2].pos.set(x3, y3);
      v[3].pos.set(x3, y0);
      v[4].pos.set(x1, y1);
      v[5].pos.set(x1, y2);
      v[6].pos.set(x2, y2);
      v[7].pos.set(x2, y1);
      v[5].color=v[6].color=v[4].color=v[7].color=c0;
      v[0].color=v[1].color=v[2].color=v[3].color=c1;

      VI.flushIndexed(IndBufRectShaded, 5*2*3);
   }
   VI.clear();
}
void Rect::drawBorderShaded(C Color &c0, C Color &c1, C Rect &border)C
{
   VI.setType(VI_2D_COL);
   if(Vtx2DCol *v=(Vtx2DCol*)VI.addVtx(8))
   {
      Flt x0=min.x, y0=min.y, x1=x0+border.min.x, y1=y0+border.min.y,
          x3=max.x, y3=max.y, x2=x3-border.max.x, y2=y3-border.max.y;
      v[0].pos.set(x0, y0);
      v[1].pos.set(x0, y3);
      v[2].pos.set(x3, y3);
      v[3].pos.set(x3, y0);
      v[4].pos.set(x1, y1);
      v[5].pos.set(x1, y2);
      v[6].pos.set(x2, y2);
      v[7].pos.set(x2, y1);
      v[5].color=v[6].color=v[4].color=v[7].color=c0;
      v[0].color=v[1].color=v[2].color=v[3].color=c1;

      VI.flushIndexed(IndBufRectBorder, 4*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void Rects::set(C Rect &rect, C VecI2 &cells)
{
   T.rect   =rect;
   T.cells.x=Max(cells.x, 1);
   T.cells.y=Max(cells.y, 1);
   T.size   =rect.size()/T.cells;
}
void Rects::set(C Rect &rect, Int elms)
{
   VecI2 cells;
   Vec2  size=rect.size();
   Flt   area=size.mul(),
         mul =Sqrt(elms/area); // = 1/avg_cell_size

   if(size.x<size.y)
   {
      cells.x=Max(1,    Round(size.x*mul));
      cells.y=Max(1, DivRound(elms, cells.x));
   }else
   {
      cells.y=Max(1,    Round(size.y*mul));
      cells.x=Max(1, DivRound(elms, cells.y));
   }
   set(rect, cells);
}
VecI2 Rects::coords(C Vec2 &pos)C
{
   VecI2 O=Trunc((pos-rect.min)/size); // use 'Trunc' because it's faster and we're not interested in negative indexes
   Clamp(O.x, 0, cells.x-1);
   Clamp(O.y, 0, cells.y-1);
   return O;
}
RectI Rects::coords(C Rect &rect)C
{
   return RectI(coords(rect.min),
                coords(rect.max));
}
Rect Rects::getRect(C VecI2 &cell)C
{
   Rect O;
   O.min=T.rect.min+size*cell;
   O.max=     O.min+size;
   return O;
}
void Rects::draw(C Color &grid_color, C Color &back_color)C
{
   if(back_color.a)rect.draw(back_color, true);
   if(grid_color.a)
   {
      VI.color(grid_color);
      REP(cells.x+1)
      {
         Flt x=rect.min.x+i*size.x;
         VI.line(Vec2(x, rect.min.y), Vec2(x, rect.max.y));
      }
      REP(cells.y+1)
      {
         Flt y=rect.min.y+i*size.y;
         VI.line(Vec2(rect.min.x, y), Vec2(rect.max.x, y));
      }
      VI.end();
   }
}
void Rects::draw(C Color &grid_color, C Color &field_color, Index *rect_edge)C
{
   if(field_color.a && rect_edge)
   {
      VI.color(field_color);
      FREPD(y, cells.y)
      FREPD(x, cells.x)if(rect_edge->group[x+y*cells.x].num)VI.rect(getRect(VecI2(x, y)));
      VI.end();
   }
   draw(grid_color);
}
/******************************************************************************/
Flt Dist(C Vec2 &point, C Rect &rect)
{
   if(point.x>rect.max.x) // right
   {
      if(point.y<rect.min.y)return Dist(point, rect.rd()); // rd corner
      if(point.y>rect.max.y)return Dist(point, rect.max ); // ru corner
                            return point.x-rect.max.x;     // r  side
   }
   if(point.x<rect.min.x) // left
   {
      if(point.y>rect.max.y)return Dist(point, rect.lu()); // lu corner
      if(point.y<rect.min.y)return Dist(point, rect.min ); // ld corner
                            return rect.min.x-point.x;     // l  side
   }
   if(point.y>rect.max.y)return point.y-rect.max.y; // u side
   if(point.y<rect.min.y)return rect.min.y-point.y; // d side
   return 0;
}
Flt Dist(C Vec2 &point, C RectI &rect)
{
   if(point.x>rect.max.x) // right
   {
      if(point.y<rect.min.y)return Dist(point, rect.rd()); // rd corner
      if(point.y>rect.max.y)return Dist(point, rect.max ); // ru corner
                            return point.x-rect.max.x;     // r  side
   }
   if(point.x<rect.min.x) // left
   {
      if(point.y>rect.max.y)return Dist(point, rect.lu()); // lu corner
      if(point.y<rect.min.y)return Dist(point, rect.min ); // ld corner
                            return rect.min.x-point.x;     // l  side
   }
   if(point.y>rect.max.y)return point.y-rect.max.y; // u side
   if(point.y<rect.min.y)return rect.min.y-point.y; // d side
   return 0;
}
Flt Dist2(C Vec2 &point, C Rect &rect)
{
   if(point.x>rect.max.x) // right
   {
      if(point.y<rect.min.y)return Dist2(point, rect.rd()); // rd corner
      if(point.y>rect.max.y)return Dist2(point, rect.max ); // ru corner
                            return Sqr(point.x-rect.max.x); // r  side
   }
   if(point.x<rect.min.x) // left
   {
      if(point.y>rect.max.y)return Dist2(point, rect.lu()); // lu corner
      if(point.y<rect.min.y)return Dist2(point, rect.min ); // ld corner
                            return Sqr(rect.min.x-point.x); // l  side
   }
   if(point.y>rect.max.y)return Sqr(point.y-rect.max.y); // u side
   if(point.y<rect.min.y)return Sqr(rect.min.y-point.y); // d side
   return 0;
}
Flt Dist2(C Vec2 &point, C RectI &rect)
{
   if(point.x>rect.max.x) // right
   {
      if(point.y<rect.min.y)return Dist2(point, rect.rd()); // rd corner
      if(point.y>rect.max.y)return Dist2(point, rect.max ); // ru corner
                            return Sqr(point.x-rect.max.x); // r  side
   }
   if(point.x<rect.min.x) // left
   {
      if(point.y>rect.max.y)return Dist2(point, rect.lu()); // lu corner
      if(point.y<rect.min.y)return Dist2(point, rect.min ); // ld corner
                            return Sqr(rect.min.x-point.x); // l  side
   }
   if(point.y>rect.max.y)return Sqr(point.y-rect.max.y); // u side
   if(point.y<rect.min.y)return Sqr(rect.min.y-point.y); // d side
   return 0;
}
Flt Dist(C Rect &a, C Rect &b)
{
   return Dist(Max(0, Abs(a.centerX()-b.centerX())-(a.w()+b.w())*0.5f),
               Max(0, Abs(a.centerY()-b.centerY())-(a.h()+b.h())*0.5f));
}
/******************************************************************************/
Flt Dist2PointSquare(C Vec2 &pos, C Vec2 &square_center, Flt square_radius)
{
   return Sqr(Max(Abs(pos.x-square_center.x)-square_radius, 0))
         +Sqr(Max(Abs(pos.y-square_center.y)-square_radius, 0));
}
Flt Dist2PointSquare(C Vec2 &pos, C VecI2 &square_center, Flt square_radius)
{
   return Sqr(Max(Abs(pos.x-square_center.x)-square_radius, 0))
         +Sqr(Max(Abs(pos.y-square_center.y)-square_radius, 0));
}
/******************************************************************************/
Bool Cuts(C Rect &a, C Rect &b)
{
   return b.max.x>=a.min.x && b.min.x<=a.max.x
       && b.max.y>=a.min.y && b.min.y<=a.max.y;
}
Bool Cuts(C RectI &a, C RectI &b)
{
   return b.max.x>=a.min.x && b.min.x<=a.max.x
       && b.max.y>=a.min.y && b.min.y<=a.max.y;
}
Bool Cuts(C Edge2 &edge, C Rect &rect)
{
   if(Cuts(edge.p[0], rect)
   || Cuts(edge.p[1], rect))return true;

   if(edge.p[0].x<rect.min.x && edge.p[1].x>=rect.min.x){if(rect.includesY(PointOnPlane(edge.p[0].y, edge.p[1].y, edge.p[0].x-rect.min.x, edge.p[1].x-rect.min.x)))return true;}else
   if(edge.p[0].x>rect.max.x && edge.p[1].x<=rect.max.x){if(rect.includesY(PointOnPlane(edge.p[0].y, edge.p[1].y, edge.p[0].x-rect.max.x, edge.p[1].x-rect.max.x)))return true;}
   if(edge.p[0].y<rect.min.y && edge.p[1].y>=rect.min.y){if(rect.includesX(PointOnPlane(edge.p[0].x, edge.p[1].x, edge.p[0].y-rect.min.y, edge.p[1].y-rect.min.y)))return true;}else
   if(edge.p[0].y>rect.max.y && edge.p[1].y<=rect.max.y){if(rect.includesX(PointOnPlane(edge.p[0].x, edge.p[1].x, edge.p[0].y-rect.max.y, edge.p[1].y-rect.max.y)))return true;}

   return false;
}
Bool CutsEps(C Vec2 &point, C Rect &rect)
{
   return point.x>=rect.min.x-EPS && point.x<=rect.max.x+EPS
       && point.y>=rect.min.y-EPS && point.y<=rect.max.y+EPS;
}
/******************************************************************************/
Bool Inside(C Rect &a, C Rect &b)
{
   return a.min.x>=b.min.x && a.max.x<=b.max.x
       && a.min.y>=b.min.y && a.max.y<=b.max.y;
}
Bool Inside(C RectI &a, C RectI &b)
{
   return a.min.x>=b.min.x && a.max.x<=b.max.x
       && a.min.y>=b.min.y && a.max.y<=b.max.y;
}
Bool InsideEps(C Rect &a, C Rect &b)
{
   return a.min.x>=b.min.x-EPS && a.max.x<=b.max.x+EPS
       && a.min.y>=b.min.y-EPS && a.max.y<=b.max.y+EPS;
}
/******************************************************************************/
Rect Fit(Flt src_aspect, C Rect &dest_rect, FIT_MODE fit)
{
   Rect r=dest_rect; Bool mx=(r.min.x>r.max.x); if(mx)Swap(r.min.x, r.max.x); Flt w=r.w();
                     Bool my=(r.min.y>r.max.y); if(my)Swap(r.min.y, r.max.y); Flt h=r.h(), dest_aspect=w/h;

   if(fit==FIT_FULL)fit=((src_aspect>dest_aspect) ? FIT_WIDTH : FIT_HEIGHT);else
   if(fit==FIT_FILL)fit=((src_aspect<dest_aspect) ? FIT_WIDTH : FIT_HEIGHT);

   if(fit==FIT_WIDTH){Flt size=w/src_aspect, d=(size-h)*0.5f; r.extendY(d);}
   else              {Flt size=h*src_aspect, d=(size-w)*0.5f; r.extendX(d);}

   if(mx)Swap(r.min.x, r.max.x);
   if(my)Swap(r.min.y, r.max.y);
   
   return r;
}
/******************************************************************************/
Bool SweepPointRect(C Vec2 &point, C Vec2 &move, C Rect &rect, Flt *hit_frac, Vec2 *hit_normal, Vec2 *hit_pos)
{
   if(Cuts(point, rect)){if(hit_frac)*hit_frac=0; if(hit_normal)hit_normal->zero(); if(hit_pos)*hit_pos=point; return true;}
   Flt  frac;
   Vec2 test;
   if(point.x<rect.min.x && point.x+move.x>=rect.min.x){frac=(rect.min.x-point.x)/move.x; test.set(rect.min.x, point.y+move.y*frac); if(rect.includesY(test.y)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(-1, 0); if(hit_pos)*hit_pos=test; return true;}}else
   if(point.x>rect.max.x && point.x+move.x<=rect.max.x){frac=(rect.max.x-point.x)/move.x; test.set(rect.max.x, point.y+move.y*frac); if(rect.includesY(test.y)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set( 1, 0); if(hit_pos)*hit_pos=test; return true;}}
   if(point.y<rect.min.y && point.y+move.y>=rect.min.y){frac=(rect.min.y-point.y)/move.y; test.set(point.x+move.x*frac, rect.min.y); if(rect.includesX(test.x)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0, -1); if(hit_pos)*hit_pos=test; return true;}}else
   if(point.y>rect.max.y && point.y+move.y<=rect.max.y){frac=(rect.max.y-point.y)/move.y; test.set(point.x+move.x*frac, rect.max.y); if(rect.includesX(test.x)){if(hit_frac)*hit_frac=frac; if(hit_normal)hit_normal->set(0,  1); if(hit_pos)*hit_pos=test; return true;}}
   return false;
}
/******************************************************************************/
namespace RP
{
   #include "Rect Packer/Rect.cpp"
   #include "Rect Packer/ShelfBinPack.cpp"
   #include "Rect Packer/MaxRectsBinPack.cpp"
   #include "Rect Packer/GuillotineBinPack.cpp"
 //#include "Rect Packer/SkylineBinPack.cpp"
}
/******************************************************************************/
Bool PackRectsKnownLimit(C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectI> rects, C VecI2 &limit, Bool allow_rotate, Int border, Bool align_for_compression, Bool compact_arrangement)
{
   MAX(border, 0); if(align_for_compression)border+=3; // when using 'align_for_compression' we need to have at least 3 extra pixels to align the target

   Memc<RP::RectSizeIndex> rp_src;
   Memc<RP::RectIndex    > rp_dest;
   rects.setNum(sizes.elms());

   if(!compact_arrangement)
   { // try "shelf" first, because it's fastest
      RP::ShelfBinPack pack(limit.x, limit.y, false, allow_rotate);
      FREPA(sizes)
      {
       C RectSizeAnchor &rsa=sizes[i];
         if(rsa.size.x>0 && rsa.size.y>0)
         {
            RP::Rect rect=pack.Insert(rsa.size.x+border, rsa.size.y+border, RP::ShelfBinPack::ShelfNextFit); if(!rect.width)goto max_rects;
            Bool     flipped=((rect.width>rect.height)!=(rsa.size.x>rsa.size.y));
            if(align_for_compression)
            {
               VecI2 anchor=rsa.anchor; anchor.x+=rect.x; anchor.y+=rect.y; // final anchor position
               rect.x+=Ceil4(anchor.x)-anchor.x; rect.y+=Ceil4(anchor.y)-anchor.y; // offset rectangle so the anchor is 4-pixel aligned
            }
            rects[i].setLD(rect.x, rect.y, rsa.size.c[flipped], rsa.size.c[!flipped]);
         }else
         if(!rsa.size.x && !rsa.size.y)rects[i].zero();else return false;
      }
      return true;
   }

max_rects:
   { // try "max rects" next, because it has best occupancy rate
      RP::MaxRectsBinPack pack(limit.x, limit.y, allow_rotate);
      rp_src.setNum(sizes.elms()); REPA(rp_src)
      {
       C RectSizeAnchor &rsa=sizes[i];
         if(rsa.size.x>0 && rsa.size.y>0)
         {
            RP::RectSizeIndex &rp_s=rp_src[i];
            rp_s.width =rsa.size.x+border;
            rp_s.height=rsa.size.y+border;
            rp_s.index =i;
         }else
         if(!rsa.size.x && !rsa.size.y){rects[i].zero(); rp_src.remove(i);}else return false;
      }
      pack.Insert(rp_src, rp_dest, RP::MaxRectsBinPack::RectBestShortSideFit);
      if(!rp_src.elms()) // all were processed
      {
         REPA(rp_dest)
         {
            RP::RectIndex &rect=rp_dest[i];
            RectI         &dest=rects  [rect.index];
          C RectSizeAnchor &rsa=sizes  [rect.index];
            Bool        flipped=((rect.width>rect.height)!=(rsa.size.x>rsa.size.y));
            if(align_for_compression)
            {
               VecI2 anchor=rsa.anchor; anchor.x+=rect.x; anchor.y+=rect.y; // final anchor position
               rect.x+=Ceil4(anchor.x)-anchor.x; rect.y+=Ceil4(anchor.y)-anchor.y; // offset rectangle so the anchor is 4-pixel aligned
            }
            dest.setLD(rect.x, rect.y, rsa.size.c[flipped], rsa.size.c[!flipped]);
         }
         return true;
      }
   }

   return false;
}
/******************************************************************************/
Bool PackRectsUnknownLimit(C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectI> rects, VecI2 &limit, Bool allow_rotate, Int border, Bool align_for_compression, Bool compact_arrangement, Bool only_square, Int max_size)
{
   ULong area=0; REPA(sizes)
   {
    C RectSizeAnchor &rsa=sizes[i];
      area+=ULong(rsa.size.x)*ULong(rsa.size.y);
   }
   for(Int dim   =FloorPow2(SqrtI(area)); dim   <=max_size; dim <<=1)
   for(Int aspect=(only_square ? 4 : 0) ; aspect<        5; aspect++)
   {
      limit=dim; switch(aspect)
      {
         case 0: limit.x>>=2; break; // at 1st attempt try making (limit/4, limit  ), as textures should preferrably be taller than wider
         case 1: limit.y>>=2; break; // at 2nd attempt try making (limit  , limit/4)
         case 2: limit.x>>=1; break; // at 3rd attempt try making (limit/2, limit  ), as textures should preferrably be taller than wider
         case 3: limit.y>>=1; break; // at 4th attempt try making (limit  , limit/2)
                                     // at 5th attempt try making (limit  , limit  )
      }
      if(PackRectsKnownLimit(sizes, rects, limit, allow_rotate, border, align_for_compression, compact_arrangement))return true;
   }
   return false;
}
/******************************************************************************/
Bool PackRectsMultiLimit(C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectIndex> rects, MemPtr<VecI2> limits, Bool allow_rotate, Int border, Bool align_for_compression, Bool compact_arrangement, Bool only_square, Int max_size)
{
   MAX(border, 0); if(align_for_compression)border+=3; // when using 'align_for_compression' we need to have at least 3 extra pixels to align the target

   rects .setNum(sizes.elms());
   limits.clear();

   Memc<RP::RectSizeIndex> left_best, left, left_all;
   Memc<RectIndex>       placed_best, placed;
   Memc<RP::RectIndex>   placed_rp;
   left_all.setNum(sizes.elms()); REPA(left_all)
   {
    C RectSizeAnchor &rsa=sizes[i];
      if(rsa.size.x>0 && rsa.size.y>0)
      {
         RP::RectSizeIndex &l=left_all[i];
         l.width =rsa.size.x+border;
         l.height=rsa.size.y+border;
         l.index =i;
      }else
      if(!rsa.size.x && !rsa.size.y){RectIndex &r=rects[i]; r.zero(); r.index=0; left_all.remove(i);}else return false;
   }

   RP::ShelfBinPack    shelf;
   RP::MaxRectsBinPack max_rects;

   for(; left_all.elms(); )
   {
        left_best.clear();
      placed_best.clear();
      ULong  area_best=0, area=0; REPA(left_all){C RP::RectSizeIndex &l=left_all[i]; area+=ULong(l.width)*ULong(l.height);}
      VecI2 limit_best=0;
      for(Int dim   =Min(FloorPow2(SqrtI(area)), max_size); dim   <=max_size; dim <<=1)
      for(Int aspect=(only_square ? 2 : 0)                ; aspect<        3; aspect++)
      {
         VecI2 limit=dim; switch(aspect)
         {
            case 0: limit.x>>=1; break; // at 1st attempt try making (limit/2, limit  ), as textures should preferrably be taller than wider
            case 1: limit.y>>=1; break; // at 2nd attempt try making (limit  , limit/2), in case this fits the parts but above method doesn't
                                        // at 3rd attempt try making (limit  , limit  )
         }

         if(!compact_arrangement)
         {
            // try "shelf"
            left=left_all;
            placed.clear();
            shelf.Init(limit.x, limit.y, false, allow_rotate);
            REPA(left)
            {
             C RP::RectSizeIndex &l=left[i];
               RP::Rect        rect=shelf.Insert(l.width, l.height, RP::ShelfBinPack::ShelfNextFit);
               if(rect.width) // it fits
               {
                C RectSizeAnchor &rsa =sizes[l.index];
                  RectIndex      &dest=placed.New();
                  Bool         flipped=((rect.width>rect.height)!=(rsa.size.x>rsa.size.y));
                  if(align_for_compression)
                  {
                     VecI2 anchor=rsa.anchor; anchor.x+=rect.x; anchor.y+=rect.y; // final anchor position
                     rect.x+=Ceil4(anchor.x)-anchor.x; rect.y+=Ceil4(anchor.y)-anchor.y; // offset rectangle so the anchor is 4-pixel aligned
                  }
                  dest.index=l.index; // this should be index in 'limits' but for now we're using it as 'sizes' index
                  dest.setLD(rect.x, rect.y, rsa.size.c[flipped], rsa.size.c[!flipped]);
                  left.remove(i);
               }
            }
            if(!left.elms()){limit_best=limit; left_all.clear(); Swap(placed, placed_best); goto done;} // finished
            area=0; REPA(placed){C RectIndex &r=placed[i]; area+=ULong(r.w())*ULong(r.h());} // check if it's best (covers most area)
            if(area>area_best)
            {
                area_best=area;
               limit_best=limit;
               Swap(left  ,   left_best);
               Swap(placed, placed_best);
            }
         }

         // try "max_rects"
         {
            left=left_all;
            placed   .clear();
            placed_rp.clear();
            max_rects.Init  (limit.x, limit.y, allow_rotate);
            max_rects.Insert(left, placed_rp, RP::MaxRectsBinPack::RectBestShortSideFit);
            REPA(placed_rp)
            {
           RP::RectIndex      &rect=placed_rp[i];
               RectIndex      &dest=placed.New();
             C RectSizeAnchor &rsa =sizes[rect.index];
               Bool         flipped=((rect.width>rect.height)!=(rsa.size.x>rsa.size.y));
               if(align_for_compression)
               {
                  VecI2 anchor=rsa.anchor; anchor.x+=rect.x; anchor.y+=rect.y; // final anchor position
                  rect.x+=Ceil4(anchor.x)-anchor.x; rect.y+=Ceil4(anchor.y)-anchor.y; // offset rectangle so the anchor is 4-pixel aligned
               }
               dest.index=rect.index; // this should be index in 'limits' but for now we're using it as 'sizes' index
               dest.setLD(rect.x, rect.y, rsa.size.c[flipped], rsa.size.c[!flipped]);
            }
            if(!left.elms()){limit_best=limit; left_all.clear(); Swap(placed, placed_best); goto done;} // finished
            area=0; REPA(placed){C RectIndex &r=placed[i]; area+=ULong(r.w())*ULong(r.h());} // check if it's best (covers most area)
            if(area>area_best)
            {
                area_best=area;
               limit_best=limit;
               Swap(left  ,   left_best);
               Swap(placed, placed_best);
            }
         }
      }
      if(!placed_best.elms())return false; // if no elements were placed in this step then fail
      Swap(left_all, left_best);

   done:
      REPA(placed_best)
      {
         RectIndex &src=placed_best[i], &dest=rects[src.index];
         dest=src;
         dest.index=limits.elms(); // set index as index in 'limits'
      }
      limits.add(limit_best);
   }
   return true;
}
/******************************************************************************/
Bool BestFit(Vec2 *point, Int points, Vec2 &axis)
{
   Memt<Vec2> convex_points; CreateConvex2D(convex_points, point, points);
   if(convex_points.elms()>1)
   {
      Flt   best_area=FLT_MAX;
    C Vec2 &last=convex_points.last();
      REPA(convex_points)
      {
       C Vec2 &p0=convex_points[i], &p1=convex_points[(i+1)%convex_points.elms()];
         Vec2 dir=p1-p0; if(dir.normalize())
         {
            Vec2 perp=Perp(dir);
            Rect r; r.setX(Dot(last, dir)).setY(Dot(last, perp));
            REP(convex_points.elms()-1)
            {
             C Vec2 &p=convex_points[i];
               r.includeX(Dot(p, dir)).includeY(Dot(p, perp));
            }
            Flt area=r.area(); if(area<best_area){best_area=area; axis=dir;}
         }
      }
      if(best_area<FLT_MAX)return true;
   }
   return false;
}
Bool BestFit(VecD2 *point, Int points, VecD2 &axis)
{
   Memt<VecD2> convex_points; CreateConvex2D(convex_points, point, points);
   if(convex_points.elms()>1)
   {
      Dbl    best_area=DBL_MAX;
    C VecD2 &last=convex_points.last();
      REPA(convex_points)
      {
       C VecD2 &p0=convex_points[i], &p1=convex_points[(i+1)%convex_points.elms()];
         VecD2 dir=p1-p0; if(dir.normalize())
         {
            VecD2 perp=Perp(dir);
            RectD r; r.setX(Dot(last, dir)).setY(Dot(last, perp));
            REP(convex_points.elms()-1)
            {
             C VecD2 &p=convex_points[i];
               r.includeX(Dot(p, dir)).includeY(Dot(p, perp));
            }
            Dbl area=r.area(); if(area<best_area){best_area=area; axis=dir;}
         }
      }
      if(best_area<DBL_MAX)return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
