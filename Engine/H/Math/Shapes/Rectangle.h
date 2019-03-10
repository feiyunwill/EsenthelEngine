/******************************************************************************

   Use 'Rect'  to handle rectangle shape, Flt type
   Use 'RectI' to handle rectangle shape, Int type

/******************************************************************************/
enum FIT_MODE : Byte // Fit Mode, these modes specify fitting while preserving original proportions
{
   FIT_FULL  , // fit width and height to fit  the entire source
   FIT_FILL  , // fit width and height to fill the entire destination
   FIT_WIDTH , // fit width
   FIT_HEIGHT, // fit height
};
/******************************************************************************/
struct Rect // Rectangle Shape
{
   Vec2 min, max;

   Rect& zero(                                              ) {  min.zero();   max.zero();                   return T;} // zero rectangle coordinates
   Rect& set (C Vec2 &min, C Vec2 &max                      ) {T.min=min   ; T.max=max   ;                   return T;} // set  rectangle from minimum maximum coordinates
   Rect& set (  Flt   min_x, Flt min_y, Flt max_x, Flt max_y) {min.set(min_x, min_y); max.set(max_x, max_y); return T;} // set  rectangle from minimum maximum coordinates

   Rect& setX(Flt x           ) {T.min.x=x  ; T.max.x=  x; return T;}
   Rect& setY(Flt y           ) {T.min.y=y  ; T.max.y=  y; return T;}
   Rect& setX(Flt min, Flt max) {T.min.x=min; T.max.x=max; return T;}
   Rect& setY(Flt min, Flt max) {T.min.y=min; T.max.y=max; return T;}

   Rect& setRU(C Vec2 &ru, C Vec2 &size) {min.set(ru.x-size.x     , ru.y-size.y     ); max.set(ru.x            , ru.y            ); return T;} // set rectangle by specifying its "Right Up"   position
   Rect& setLU(C Vec2 &lu, C Vec2 &size) {min.set(lu.x            , lu.y-size.y     ); max.set(lu.x+size.x     , lu.y            ); return T;} // set rectangle by specifying its "Left  Up"   position
   Rect& setRD(C Vec2 &rd, C Vec2 &size) {min.set(rd.x-size.x     , rd.y            ); max.set(rd.x            , rd.y+size.y     ); return T;} // set rectangle by specifying its "Right Down" position
   Rect& setLD(C Vec2 &ld, C Vec2 &size) {min.set(ld.x            , ld.y            ); max.set(ld.x+size.x     , ld.y+size.y     ); return T;} // set rectangle by specifying its "Left  Down" position
   Rect& setR (C Vec2 &r , C Vec2 &size) {min.set( r.x-size.x     ,  r.y-size.y*0.5f); max.set( r.x            ,  r.y+size.y*0.5f); return T;} // set rectangle by specifying its "Right"      position
   Rect& setL (C Vec2 &l , C Vec2 &size) {min.set( l.x            ,  l.y-size.y*0.5f); max.set( l.x+size.x     ,  l.y+size.y*0.5f); return T;} // set rectangle by specifying its "Left"       position
   Rect& setU (C Vec2 &u , C Vec2 &size) {min.set( u.x-size.x*0.5f,  u.y-size.y     ); max.set( u.x+size.x*0.5f,  u.y            ); return T;} // set rectangle by specifying its "Up"         position
   Rect& setD (C Vec2 &d , C Vec2 &size) {min.set( d.x-size.x*0.5f,  d.y            ); max.set( d.x+size.x*0.5f,  d.y+size.y     ); return T;} // set rectangle by specifying its "Down"       position
   Rect& setC (C Vec2 &c , C Vec2 &size) {min.set( c.x-size.x*0.5f,  c.y-size.y*0.5f); max.set( c.x+size.x*0.5f,  c.y+size.y*0.5f); return T;} // set rectangle by specifying its "Center"     position

   Rect& setRU(C Vec2 &ru, Flt w, Flt h) {min.set(ru.x-w     , ru.y-h     ); max.set(ru.x       , ru.y       ); return T;} // set rectangle by specifying its "Right Up"   position
   Rect& setLU(C Vec2 &lu, Flt w, Flt h) {min.set(lu.x       , lu.y-h     ); max.set(lu.x+w     , lu.y       ); return T;} // set rectangle by specifying its "Left  Up"   position
   Rect& setRD(C Vec2 &rd, Flt w, Flt h) {min.set(rd.x-w     , rd.y       ); max.set(rd.x       , rd.y+h     ); return T;} // set rectangle by specifying its "Right Down" position
   Rect& setLD(C Vec2 &ld, Flt w, Flt h) {min.set(ld.x       , ld.y       ); max.set(ld.x+w     , ld.y+h     ); return T;} // set rectangle by specifying its "Left  Down" position
   Rect& setR (C Vec2 &r , Flt w, Flt h) {min.set( r.x-w     ,  r.y-h*0.5f); max.set( r.x       ,  r.y+h*0.5f); return T;} // set rectangle by specifying its "Right"      position
   Rect& setL (C Vec2 &l , Flt w, Flt h) {min.set( l.x       ,  l.y-h*0.5f); max.set( l.x+w     ,  l.y+h*0.5f); return T;} // set rectangle by specifying its "Left"       position
   Rect& setU (C Vec2 &u , Flt w, Flt h) {min.set( u.x-w*0.5f,  u.y-h     ); max.set( u.x+w*0.5f,  u.y       ); return T;} // set rectangle by specifying its "Up"         position
   Rect& setD (C Vec2 &d , Flt w, Flt h) {min.set( d.x-w*0.5f,  d.y       ); max.set( d.x+w*0.5f,  d.y+h     ); return T;} // set rectangle by specifying its "Down"       position
   Rect& setC (C Vec2 &c , Flt w, Flt h) {min.set( c.x-w*0.5f,  c.y-h*0.5f); max.set( c.x+w*0.5f,  c.y+h*0.5f); return T;} // set rectangle by specifying its "Center"     position
   Rect& setC (C Vec2 &c , Flt r       ) {min.set( c.x-r     ,  c.y-r     ); max.set( c.x+r     ,  c.y+r     ); return T;} // set rectangle by specifying its "Center"     position

   Rect& setRU(Flt x, Flt y, Flt w, Flt h) {min.set(x-w     , y-h     ); max.set(x       , y       ); return T;} // set rectangle by specifying its "Right Up"   position
   Rect& setLU(Flt x, Flt y, Flt w, Flt h) {min.set(x       , y-h     ); max.set(x+w     , y       ); return T;} // set rectangle by specifying its "Left  Up"   position
   Rect& setRD(Flt x, Flt y, Flt w, Flt h) {min.set(x-w     , y       ); max.set(x       , y+h     ); return T;} // set rectangle by specifying its "Right Down" position
   Rect& setLD(Flt x, Flt y, Flt w, Flt h) {min.set(x       , y       ); max.set(x+w     , y+h     ); return T;} // set rectangle by specifying its "Left  Down" position
   Rect& setR (Flt x, Flt y, Flt w, Flt h) {min.set(x-w     , y-h*0.5f); max.set(x       , y+h*0.5f); return T;} // set rectangle by specifying its "Right"      position
   Rect& setL (Flt x, Flt y, Flt w, Flt h) {min.set(x       , y-h*0.5f); max.set(x+w     , y+h*0.5f); return T;} // set rectangle by specifying its "Left"       position
   Rect& setU (Flt x, Flt y, Flt w, Flt h) {min.set(x-w*0.5f, y-h     ); max.set(x+w*0.5f, y       ); return T;} // set rectangle by specifying its "Up"         position
   Rect& setD (Flt x, Flt y, Flt w, Flt h) {min.set(x-w*0.5f, y       ); max.set(x+w*0.5f, y+h     ); return T;} // set rectangle by specifying its "Down"       position
   Rect& setC (Flt x, Flt y, Flt w, Flt h) {min.set(x-w*0.5f, y-h*0.5f); max.set(x+w*0.5f, y+h*0.5f); return T;} // set rectangle by specifying its "Center"     position
   Rect& setC (Flt x, Flt y, Flt r       ) {min.set(x-r     , y-r     ); max.set(x+r     , y+r     ); return T;} // set rectangle by specifying its "Center"     position

   Rect& operator+=(C Vec2  &v) {min+=v   ; max+=v   ; return T;}
   Rect& operator-=(C Vec2  &v) {min-=v   ; max-=v   ; return T;}
   Rect& operator+=(C Vec   &v) {min+=v.xy; max+=v.xy; return T;}
   Rect& operator-=(C Vec   &v) {min-=v.xy; max-=v.xy; return T;}
   Rect& operator*=(  Flt    r) {min*=r   ; max*=r   ; return T;}
   Rect& operator/=(  Flt    r) {min/=r   ; max/=r   ; return T;}
   Rect& operator*=(C Vec2  &v) {min*=v   ; max*=v   ; return T;}
   Rect& operator/=(C Vec2  &v) {min/=v   ; max/=v   ; return T;}
   Rect& operator|=(C Vec2  &v) {return include(v);}
   Rect& operator|=(C Rect  &r) {return include(r);}
   Rect& operator&=(C Rect  &r); // intersect
   Rect& operator&=(C RectI &r); // intersect
   Bool  operator==(C Rect  &r)C {return min==r.min && max==r.max;}
   Bool  operator!=(C Rect  &r)C {return min!=r.min || max!=r.max;}

   friend Rect operator+ (C Rect &rect, C Vec2  &v) {return Rect(rect)+=v;}
   friend Rect operator- (C Rect &rect, C Vec2  &v) {return Rect(rect)-=v;}
   friend Rect operator* (C Rect &rect,   Flt    r) {return Rect(rect)*=r;}
   friend Rect operator/ (C Rect &rect,   Flt    r) {return Rect(rect)/=r;}
   friend Rect operator* (C Rect &rect, C Vec2  &v) {return Rect(rect)*=v;}
   friend Rect operator/ (C Rect &rect, C Vec2  &v) {return Rect(rect)/=v;}
   friend Rect operator| (C Rect &rect, C Vec2  &v) {return Rect(rect)|=v;} // union
   friend Rect operator| (C Rect &rect, C Rect  &r) {return Rect(rect)|=r;} // union
   friend Rect operator& (C Rect &rect, C Rect  &r) {return Rect(rect)&=r;} // intersection
   friend Rect operator& (C Rect &rect, C RectI &r) {return Rect(rect)&=r;} // intersection

   // get
   Flt  w        (            )C {return        max.x   - min.x          ;} // get width
   Flt  h        (            )C {return        max.y   - min.y          ;} // get height
   Vec2 size     (            )C {return        max     - min            ;} // get size
   Vec2 ld       (            )C {return        min                      ;} // get Left  Down corner
   Vec2 ru       (            )C {return        max                      ;} // get Right Up   corner
   Vec2 lu       (            )C {return Vec2(min.x     , max.y     )    ;} // get Left  Up   corner
   Vec2 rd       (            )C {return Vec2(max.x     , min.y     )    ;} // get Right Down corner
   Vec2 left     (            )C {return Vec2(min.x     , centerY() )    ;} // get Left       point
   Vec2 right    (            )C {return Vec2(max.x     , centerY() )    ;} // get Right      point
   Vec2 down     (            )C {return Vec2(centerX() , min.y     )    ;} // get Down       point
   Vec2 up       (            )C {return Vec2(centerX() , max.y     )    ;} // get Up         point
   Flt  centerX  (            )C {return  Avg(min.x     , max.x     )    ;} // get center X
   Flt  centerY  (            )C {return  Avg(min.y     , max.y     )    ;} // get center Y
   Vec2 center   (            )C {return Vec2(centerX() , centerY() )    ;} // get center
   Vec2 xs       (            )C {return Vec2(min.x     , max.x     )    ;} // get min max X's as Vec2
   Vec2 ys       (            )C {return Vec2(min.y     , max.y     )    ;} // get min max Y's as Vec2
   Flt  lerpX    (Flt s       )C {return Lerp(min.x     , max.x, s  )    ;} // lerp between min.x and max.x
   Flt  lerpY    (Flt s       )C {return Lerp(min.y     , max.y, s  )    ;} // lerp between min.y and max.y
   Vec2 lerp     (Flt x, Flt y)C {return Vec2(lerpX(  x), lerpY(  y))    ;} // lerp between min   and max
   Vec2 lerp     (C Vec2 &s   )C {return Vec2(lerpX(s.x), lerpY(s.y))    ;} // lerp between min   and max
   Bool includesX(  Flt   x   )C {return       x>=min.x && x<=max.x      ;} // if rectangle includes 'x'
   Bool includesY(  Flt   y   )C {return       y>=min.y && y<=max.y      ;} // if rectangle includes 'y'
   Bool includes (C Vec2 &v   )C {return includesX(v.x) && includesY(v.y);} // if rectangle includes vector
   Bool validX   (            )C {return           min.x<=max.x          ;} // if has valid X's
   Bool validY   (            )C {return           min.y<=max.y          ;} // if has valid Y's
   Bool valid    (            )C {return       validX() && validY()      ;} // if is  valid
   Bool vertical (            )C {return             h()>w()             ;} // if rectangle is vertical (height greater than width)
   Flt  area     (            )C {return             w()*h()             ;} // get surface area
   Flt  aspect   (            )C {return             w()/h()             ;} // get aspect  ratio

   Str asText()C {return S+"Min: "+min+", Max: "+max;} // get text description

   // operations
   Rect& extendX   (  Flt   e           ); // extend rectangle X's
   Rect& extendY   (  Flt   e           ); // extend rectangle Y's
   Rect& extend    (  Flt   e           ); // extend rectangle
   Rect& extend    (C Vec2 &v           ); // extend rectangle
   Rect& extend    (  Flt   x  , Flt y  ); // extend rectangle
   Rect& includeX  (  Flt   x           ); // extend rectangle to include 'x'
   Rect& includeY  (  Flt   y           ); // extend rectangle to include 'y'
   Rect& includeX  (  Flt   min, Flt max); // extend rectangle to include min..max range
   Rect& includeY  (  Flt   min, Flt max); // extend rectangle to include min..max range
   Rect& include   (C Vec2 &v           ); // extend rectangle to include vector
   Rect& include   (C Rect &r           ); // extend rectangle to include rectangle
   Rect& moveX     (  Flt   dx          ); // move   rectangle X's by 'dx'
   Rect& moveY     (  Flt   dy          ); // move   rectangle Y's by 'dy'
   Rect& from      (C Vec2 &a, C Vec2 &b); // create from 2 points
   Rect& swapX     (                    ); // swap 'min.x' with 'max.x'
   Rect& swapY     (                    ); // swap 'min.y' with 'max.y'
   Rect& rotatePI_2(  Int rotations     ); // rotate rectangle by "rotations*PI_2" angle

   // draw
   void draw            (C Color &color , Bool fill=true                 )C; // draw
   void drawBorder      (C Color &color ,                    Flt   border)C; // draw border
   void drawBorder      (C Color &color ,                  C Vec2 &border)C; // draw border
   void drawShadedX     (C Color &color0, C Color &color1                )C; // draw        shaded horizontally
   void drawShadedY     (C Color &color0, C Color &color1                )C; // draw        shaded   vertically
   void drawShaded      (C Color &color0, C Color &color1,   Flt   border)C; // draw        shaded inside<->outside
   void drawBorderShaded(C Color &color0, C Color &color1, C Rect &border)C; // draw border shaded inside<->outside

   Rect() {}
   Rect(C Vec2     &vec                                       ) {set(vec, vec);}
   Rect(C Vec2     &min  , C Vec2 &max                        ) {set(min, max);}
   Rect(  Flt       min_x,   Flt   min_y, Flt max_x, Flt max_y) {set(min_x, min_y, max_x, max_y);}
   Rect(C RectI    &rect  );
   Rect(C RectD    &rect  );
   Rect(C Box      &box   );
   Rect(C Extent   &ext   );
   Rect(C Circle   &circle);
   Rect(C Edge2    &edge  );
   Rect(C Tri2     &tri   );
   Rect(C Quad2    &quad  );
   Rect(C MeshBase &mshb  );
};
/******************************************************************************/
// rectangles with helper constructors
struct Rect_RU : Rect {  Rect_RU(C Vec2 &ru, C Vec2 &size) {setRU(ru, size);}   Rect_RU(C Vec2 &ru, Flt w, Flt h) {setRU(ru, w, h);}   Rect_RU(Flt x, Flt y, Flt w, Flt h) {setRU(x, y, w, h);}   Rect_RU(C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Right Up  " position
struct Rect_LU : Rect {  Rect_LU(C Vec2 &lu, C Vec2 &size) {setLU(lu, size);}   Rect_LU(C Vec2 &lu, Flt w, Flt h) {setLU(lu, w, h);}   Rect_LU(Flt x, Flt y, Flt w, Flt h) {setLU(x, y, w, h);}   Rect_LU(C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Left  Up  " position
struct Rect_RD : Rect {  Rect_RD(C Vec2 &rd, C Vec2 &size) {setRD(rd, size);}   Rect_RD(C Vec2 &rd, Flt w, Flt h) {setRD(rd, w, h);}   Rect_RD(Flt x, Flt y, Flt w, Flt h) {setRD(x, y, w, h);}   Rect_RD(C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Right Down" position
struct Rect_LD : Rect {  Rect_LD(C Vec2 &ld, C Vec2 &size) {setLD(ld, size);}   Rect_LD(C Vec2 &ld, Flt w, Flt h) {setLD(ld, w, h);}   Rect_LD(Flt x, Flt y, Flt w, Flt h) {setLD(x, y, w, h);}   Rect_LD(C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Left  Down" position
struct Rect_R  : Rect {  Rect_R (C Vec2 &r , C Vec2 &size) {setR (r , size);}   Rect_R (C Vec2 &r , Flt w, Flt h) {setR (r , w, h);}   Rect_R (Flt x, Flt y, Flt w, Flt h) {setR (x, y, w, h);}   Rect_R (C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Right     " position
struct Rect_L  : Rect {  Rect_L (C Vec2 &l , C Vec2 &size) {setL (l , size);}   Rect_L (C Vec2 &l , Flt w, Flt h) {setL (l , w, h);}   Rect_L (Flt x, Flt y, Flt w, Flt h) {setL (x, y, w, h);}   Rect_L (C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Left      " position
struct Rect_U  : Rect {  Rect_U (C Vec2 &u , C Vec2 &size) {setU (u , size);}   Rect_U (C Vec2 &u , Flt w, Flt h) {setU (u , w, h);}   Rect_U (Flt x, Flt y, Flt w, Flt h) {setU (x, y, w, h);}   Rect_U (C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Up        " position
struct Rect_D  : Rect {  Rect_D (C Vec2 &d , C Vec2 &size) {setD (d , size);}   Rect_D (C Vec2 &d , Flt w, Flt h) {setD (d , w, h);}   Rect_D (Flt x, Flt y, Flt w, Flt h) {setD (x, y, w, h);}   Rect_D (C Rect &rect) {SCAST(Rect, T)=rect;}  }; // create rectangle by specifying its "Down      " position
struct Rect_C  : Rect {  Rect_C (C Vec2 &c , C Vec2 &size) {setC (c , size);}   Rect_C (C Vec2 &c , Flt w, Flt h) {setC (c,  w, h);}   Rect_C (Flt x, Flt y, Flt w, Flt h) {setC (x, y, w, h);}   Rect_C (C Rect &rect) {SCAST(Rect, T)=rect;}   Rect_C(C Vec2 &c, Flt r) {setC(c, r);}   Rect_C(Flt x, Flt y, Flt r) {setC(x, y, r);}  }; // create rectangle by specifying its "Center" position
/******************************************************************************/
struct RectD // Rectangle Shape (double precision)
{
   VecD2 min, max;

   RectD& zero(                                          ) {  min.zero();   max.zero();                   return T;} // zero rectangle coordinates
   RectD& set (C VecD2 &min, C VecD2 &max                ) {T.min=min   ; T.max=max   ;                   return T;} // set  rectangle from minimum maximum coordinates
   RectD& set (Dbl min_x, Dbl min_y, Dbl max_x, Dbl max_y) {min.set(min_x, min_y); max.set(max_x, max_y); return T;} // set  rectangle from minimum maximum coordinates

   RectD& setX(Dbl x           ) {T.min.x=x  ; T.max.x=  x; return T;}
   RectD& setY(Dbl y           ) {T.min.y=y  ; T.max.y=  y; return T;}
   RectD& setX(Dbl min, Dbl max) {T.min.x=min; T.max.x=max; return T;}
   RectD& setY(Dbl min, Dbl max) {T.min.y=min; T.max.y=max; return T;}

   RectD& operator+=(C VecD2 &v) {min+=v   ; max+=v   ; return T;}
   RectD& operator-=(C VecD2 &v) {min-=v   ; max-=v   ; return T;}
   RectD& operator+=(C VecD  &v) {min+=v.xy; max+=v.xy; return T;}
   RectD& operator-=(C VecD  &v) {min-=v.xy; max-=v.xy; return T;}
   RectD& operator*=(  Dbl    r) {min*=r   ; max*=r   ; return T;}
   RectD& operator/=(  Dbl    r) {min/=r   ; max/=r   ; return T;}
   RectD& operator*=(C VecD2 &v) {min*=v   ; max*=v   ; return T;}
   RectD& operator/=(C VecD2 &v) {min/=v   ; max/=v   ; return T;}
   RectD& operator|=(C VecD2 &v) {return include(v);}
   RectD& operator|=(C RectD &r) {return include(r);}
   RectD& operator&=(C RectD &r); // intersect
   RectD& operator&=(C RectI &r); // intersect
   Bool   operator==(C RectD &r)C {return min==r.min && max==r.max;}
   Bool   operator!=(C RectD &r)C {return min!=r.min || max!=r.max;}

   friend RectD operator+ (C RectD &rect, C VecD2 &v) {return RectD(rect)+=v;}
   friend RectD operator- (C RectD &rect, C VecD2 &v) {return RectD(rect)-=v;}
   friend RectD operator* (C RectD &rect,   Dbl    r) {return RectD(rect)*=r;}
   friend RectD operator/ (C RectD &rect,   Dbl    r) {return RectD(rect)/=r;}
   friend RectD operator* (C RectD &rect, C VecD2 &v) {return RectD(rect)*=v;}
   friend RectD operator/ (C RectD &rect, C VecD2 &v) {return RectD(rect)/=v;}
   friend RectD operator| (C RectD &rect, C VecD2 &v) {return RectD(rect)|=v;} // union
   friend RectD operator| (C RectD &rect, C RectD &r) {return RectD(rect)|=r;} // union
   friend RectD operator& (C RectD &rect, C RectD &r) {return RectD(rect)&=r;} // intersection
   friend RectD operator& (C RectD &rect, C RectI &r) {return RectD(rect)&=r;} // intersection

   // get
   Dbl   w        (            )C {return       max.x     - min.x         ;} // get width
   Dbl   h        (            )C {return       max.y     - min.y         ;} // get height
   VecD2 size     (            )C {return       max       - min           ;} // get size
   VecD2 ld       (            )C {return       min                       ;} // get Left  Down corner
   VecD2 ru       (            )C {return       max                       ;} // get Right Up   corner
   VecD2 lu       (            )C {return VecD2(min.x     , max.y     )   ;} // get Left  Up   corner
   VecD2 rd       (            )C {return VecD2(max.x     , min.y     )   ;} // get Right Down corner
   VecD2 left     (            )C {return VecD2(min.x     , centerY() )   ;} // get Left       point
   VecD2 right    (            )C {return VecD2(max.x     , centerY() )   ;} // get Right      point
   VecD2 down     (            )C {return VecD2(centerX() , min.y     )   ;} // get Down       point
   VecD2 up       (            )C {return VecD2(centerX() , max.y     )   ;} // get Up         point
   Dbl   centerX  (            )C {return   Avg(min.x     , max.x     )   ;} // get center X
   Dbl   centerY  (            )C {return   Avg(min.y     , max.y     )   ;} // get center Y
   VecD2 center   (            )C {return VecD2(centerX() , centerY() )   ;} // get center
   VecD2 xs       (            )C {return VecD2(min.x     , max.x     )   ;} // get min max X's as VecD2
   VecD2 ys       (            )C {return VecD2(min.y     , max.y     )   ;} // get min max Y's as VecD2
   Dbl   lerpX    (Dbl s       )C {return  Lerp(min.x     , max.x, s  )   ;} // lerp between min.x and max.x
   Dbl   lerpY    (Dbl s       )C {return  Lerp(min.y     , max.y, s  )   ;} // lerp between min.y and max.y
   VecD2 lerp     (Dbl x, Dbl y)C {return VecD2(lerpX(  x), lerpY(  y))   ;} // lerp between min   and max
   VecD2 lerp     (C VecD2 &s  )C {return VecD2(lerpX(s.x), lerpY(s.y))   ;} // lerp between min   and max
   Bool  includesX(  Dbl    x  )C {return       x>=min.x && x<=max.x      ;} // if rectangle includes 'x'
   Bool  includesY(  Dbl    y  )C {return       y>=min.y && y<=max.y      ;} // if rectangle includes 'y'
   Bool  includes (C VecD2 &v  )C {return includesX(v.x) && includesY(v.y);} // if rectangle includes vector
   Bool  validX   (            )C {return       min.x    <= max.x         ;} // if has valid X's
   Bool  validY   (            )C {return       min.y    <= max.y         ;} // if has valid Y's
   Bool  valid    (            )C {return       validX() && validY()      ;} // if is  valid
   Bool  vertical (            )C {return             h()>w()             ;} // if rectangle is vertical (height greater than width)
   Dbl   area     (            )C {return             w()*h()             ;} // get surface area
   Dbl   aspect   (            )C {return             w()/h()             ;} // get aspect  ratio

   Str asText()C {return S+"Min: "+min+", Max: "+max;} // get text description

   // operations
   RectD& extendX   (  Dbl    e            ); // extend rectangle X's
   RectD& extendY   (  Dbl    e            ); // extend rectangle Y's
   RectD& extend    (  Dbl    e            ); // extend rectangle
   RectD& extend    (C VecD2 &v            ); // extend rectangle
   RectD& extend    (  Dbl    x  , Dbl y   ); // extend rectangle
   RectD& includeX  (  Dbl    x            ); // extend rectangle to include x
   RectD& includeY  (  Dbl    y            ); // extend rectangle to include y
   RectD& includeX  (  Dbl    min, Dbl max ); // extend rectangle to include min..max range
   RectD& includeY  (  Dbl    min, Dbl max ); // extend rectangle to include min..max range
   RectD& include   (C VecD2 &v            ); // extend rectangle to include vector
   RectD& include   (C RectD &r            ); // extend rectangle to include rectangle
   RectD& moveX     (  Dbl    dx           ); // move   rectangle X's by 'dx'
   RectD& moveY     (  Dbl    dy           ); // move   rectangle Y's by 'dy'
   RectD& from      (C VecD2 &a, C VecD2 &b); // create from 2 points
   RectD& swapX     (                      ); // swap 'min.x' with 'max.x'
   RectD& swapY     (                      ); // swap 'min.y' with 'max.y'
   RectD& rotatePI_2(  Int rotations       ); // rotate rectangle by "rotations*PI_2" angle

   RectD() {}
   RectD(C VecD2 &vec                              ) {set(vec, vec);}
   RectD(C VecD2 &min, C VecD2 &max                ) {set(min, max);}
   RectD(Dbl min_x, Dbl min_y, Dbl max_x, Dbl max_y) {set(min_x, min_y, max_x, max_y);}
};
/******************************************************************************/
struct RectI // Rectangle Shape (integer)
{
   VecI2 min, max;

   RectI& zero(                                                    ) {  min.zero();   max.zero();                   return T;} // zero rectangle coordinates
   RectI& set (C VecI2 &min  , C VecI2 &max                        ) {T.min=min   ; T.max=max   ;                   return T;} // set  rectangle from minimum maximum coordinates
   RectI& set (  Int    min_x,   Int    min_y, Int max_x, Int max_y) {min.set(min_x, min_y); max.set(max_x, max_y); return T;} // set  rectangle from minimum maximum coordinates

   RectI& setRU(C VecI2 &ru, C VecI2 &size) {min.set(ru.x-size.x, ru.y-size.y); max.set(ru.x       , ru.y       ); return T;} // set rectangle by specifying its "Right Up"   position
   RectI& setLU(C VecI2 &lu, C VecI2 &size) {min.set(lu.x       , lu.y-size.y); max.set(lu.x+size.x, lu.y       ); return T;} // set rectangle by specifying its "Left  Up"   position
   RectI& setRD(C VecI2 &rd, C VecI2 &size) {min.set(rd.x-size.x, rd.y       ); max.set(rd.x       , rd.y+size.y); return T;} // set rectangle by specifying its "Right Down" position
   RectI& setLD(C VecI2 &ld, C VecI2 &size) {min.set(ld.x       , ld.y       ); max.set(ld.x+size.x, ld.y+size.y); return T;} // set rectangle by specifying its "Left  Down" position

   RectI& setRU(C VecI2 &ru, Int w, Int h) {min.set(ru.x-w, ru.y-h); max.set(ru.x  , ru.y  ); return T;} // set rectangle by specifying its "Right Up"   position
   RectI& setLU(C VecI2 &lu, Int w, Int h) {min.set(lu.x  , lu.y-h); max.set(lu.x+w, lu.y  ); return T;} // set rectangle by specifying its "Left  Up"   position
   RectI& setRD(C VecI2 &rd, Int w, Int h) {min.set(rd.x-w, rd.y  ); max.set(rd.x  , rd.y+h); return T;} // set rectangle by specifying its "Right Down" position
   RectI& setLD(C VecI2 &ld, Int w, Int h) {min.set(ld.x  , ld.y  ); max.set(ld.x+w, ld.y+h); return T;} // set rectangle by specifying its "Left  Down" position

   RectI& setRU(Int x, Int y, Int w, Int h) {min.set(x-w, y-h); max.set(x  , y  ); return T;} // set rectangle by specifying its "Right Up"   position
   RectI& setLU(Int x, Int y, Int w, Int h) {min.set(x  , y-h); max.set(x+w, y  ); return T;} // set rectangle by specifying its "Left  Up"   position
   RectI& setRD(Int x, Int y, Int w, Int h) {min.set(x-w, y  ); max.set(x  , y+h); return T;} // set rectangle by specifying its "Right Down" position
   RectI& setLD(Int x, Int y, Int w, Int h) {min.set(x  , y  ); max.set(x+w, y+h); return T;} // set rectangle by specifying its "Left  Down" position

   RectI& operator +=(C VecI2 &v)  {min +=v; max +=v; return T;}
   RectI& operator -=(C VecI2 &v)  {min -=v; max -=v; return T;}
   RectI& operator *=(  Int    i)  {min *=i; max *=i; return T;}
   RectI& operator /=(  Int    i)  {min /=i; max /=i; return T;}
   RectI& operator>>=(  Int    i)  {min>>=i; max>>=i; return T;}
   RectI& operator<<=(  Int    i)  {min<<=i; max<<=i; return T;}
   RectI& operator |=(C VecI2 &v)  {return include(v);}
   RectI& operator |=(C RectI &r)  {return include(r);}
   RectI& operator &=(C RectI &r);
   Bool   operator== (C RectI &r)C {return min==r.min && max==r.max;}
   Bool   operator!= (C RectI &r)C {return min!=r.min || max!=r.max;}

   friend RectI operator + (C RectI &rect, C VecI2 &v) {return RectI(rect) +=v;}
   friend RectI operator - (C RectI &rect, C VecI2 &v) {return RectI(rect) -=v;}
   friend RectI operator * (C RectI &rect,   Int    i) {return RectI(rect) *=i;}
   friend RectI operator / (C RectI &rect,   Int    i) {return RectI(rect) /=i;}
   friend Rect  operator * (C RectI &rect,   Flt    f) {return Rect (rect) *=f;}
   friend Rect  operator / (C RectI &rect,   Flt    f) {return Rect (rect) /=f;}
   friend RectI operator>> (C RectI &rect,   Int    i) {return RectI(rect)>>=i;}
   friend RectI operator<< (C RectI &rect,   Int    i) {return RectI(rect)<<=i;}
   friend RectI operator | (C RectI &rect, C VecI2 &v) {return RectI(rect) |=v;} // union
   friend RectI operator | (C RectI &rect, C RectI &r) {return RectI(rect) |=r;} // union
   friend RectI operator & (C RectI &rect, C RectI &r) {return RectI(rect) &=r;} // intersection

   // get
   Int   w       (            )C {return       max.x     - min.x      ;} // get width
   Int   h       (            )C {return       max.y     - min.y      ;} // get height
   VecI2 size    (            )C {return       max       - min        ;} // get size
   VecI2 ld      (            )C {return       min                    ;} // get Left  Down corner
   VecI2 ru      (            )C {return       max                    ;} // get Right Up   corner
   VecI2 lu      (            )C {return VecI2(min.x     , max.y     );} // get Left  Up   corner
   VecI2 rd      (            )C {return VecI2(max.x     , min.y     );} // get Right Down corner
   Int   centerXI(            )C {return AvgI (min.x     , max.x     );} // get center X (Int)
   Flt   centerXF(            )C {return AvgF (min.x     , max.x     );} // get center X (Flt)
   Int   centerYI(            )C {return AvgI (min.y     , max.y     );} // get center Y (Int)
   Flt   centerYF(            )C {return AvgF (min.y     , max.y     );} // get center Y (Flt)
   VecI2 centerI (            )C {return VecI2(centerXI(), centerYI());} // get center   (VecI2)
   Vec2  centerF (            )C {return Vec2 (centerXF(), centerYF());} // get center   (Vec2 )
   VecI2 xs      (            )C {return VecI2(min.x     , max.x     );} // get min max X's as VecI2
   VecI2 ys      (            )C {return VecI2(min.y     , max.y     );} // get min max Y's as VecI2
   Flt   lerpX   (Flt s       )C {return Lerp (min.x     , max.x, s  );} // lerp between min.x and max.x
   Flt   lerpY   (Flt s       )C {return Lerp (min.y     , max.y, s  );} // lerp between min.y and max.y
   Vec2  lerp    (Flt x, Flt y)C {return Vec2 (lerpX(  x), lerpY(  y));} // lerp between min   and max
   Vec2  lerp    (C Vec2 &s   )C {return Vec2 (lerpX(s.x), lerpY(s.y));} // lerp between min   and max
   Bool  validX  (            )C {return       min.x    <= max.x      ;} // if has valid X's
   Bool  validY  (            )C {return       min.y    <= max.y      ;} // if has valid Y's
   Bool  valid   (            )C {return       validX() && validY()   ;} // if is  valid
   Int   area    (            )C {return            w() *h()          ;} // get surface area
   Flt   aspect  (            )C {return        Flt(w())/h()          ;} // get aspect  ratio

   Bool includesX(  Int    x)C {return       x>=min.x && x<=max.x      ;} // if rectangle includes 'x'
   Bool includesY(  Int    y)C {return       y>=min.y && y<=max.y      ;} // if rectangle includes 'y'
   Bool includes (C VecI2 &v)C {return includesX(v.x) && includesY(v.y);} // if rectangle includes vector

   Str asText()C {return S+"Min: "+min+", Max: "+max;} // get text description

   // set
   RectI& setX(Int x           ) {T.min.x=x  ; T.max.x=  x; return T;}
   RectI& setY(Int y           ) {T.min.y=y  ; T.max.y=  y; return T;}
   RectI& setX(Int min, Int max) {T.min.x=min; T.max.x=max; return T;}
   RectI& setY(Int min, Int max) {T.min.y=min; T.max.y=max; return T;}

   // operations
   RectI& extendX   (  Int    x            ); // extend X's by 'x'
   RectI& extendY   (  Int    y            ); // extend Y's by 'y'
   RectI& extend    (  Int    e            ); // extend     by 'e'
   RectI& extend    (C VecI2 &v            ); // extend     by 'v'
   RectI& extend    (  Int    x  , Int y   ); // extend     by 'x, y'
   RectI& includeX  (  Int    x            ); // extend rectangle to include 'x'
   RectI& includeY  (  Int    y            ); // extend rectangle to include 'y'
   RectI& includeX  (  Int    min, Int max ); // extend rectangle to include min..max range
   RectI& includeY  (  Int    min, Int max ); // extend rectangle to include min..max range
   RectI& include   (C VecI2 &v            ); // extend rectangle to include vector
   RectI& include   (C RectI &r            ); // extend rectangle to include rectangle
   RectI& from      (C VecI2 &a, C VecI2 &b); // create from 2 points
   RectI& rotatePI_2(  Int rotations       ); // rotate rectangle by "rotations*PI_2" angle

   RectI() {}
   RectI(C VecI2 &vec                                        ) {set(vec, vec);}
   RectI(C VecI2 &min  , C VecI2 &max                        ) {set(min, max);}
   RectI(  Int    min_x,   Int    min_y, Int max_x, Int max_y) {set(min_x, min_y, max_x, max_y);}
};
/******************************************************************************/
struct Rects // Rectangles, allows space partitioning divided into "cells.x * cells.y" cells (sub-rectangles)
{
   VecI2 cells; // number of cells in each dimension
   Vec2  size ; // size of a single cell
   Rect  rect ; // rectangle covering all cells

   // set
   void set(C Rect &rect, C VecI2 &cells); // set partitioning according to 'rect' space divided into "cells.x * cells.y"  cells
   void set(C Rect &rect,   Int    elms ); // set partitioning according to 'rect' space divided into approximately "elms" cells, 'cells' will be calculated automatically

   // get
   VecI2 coords (C Vec2  &pos )C; // get cell coordinates of 'pos'  world position , coordinates are automatically clamped to 0..cells-1 range
   RectI coords (C Rect  &rect)C; // get cell coordinates of 'rect' world rectangle, coordinates are automatically clamped to 0..cells-1 range
   Rect  getRect(C VecI2 &cell)C; // get world rectangle from 'cell' coordinates
   Int   index  (C VecI2 &pos )C {return pos.x + cells.x*pos.y;} // get cell index of cell  position
   Int   index  (C Vec2  &pos )C {return index(coords(pos))   ;} // get cell index of world position
   Int   num    (             )C {return cells.x*cells.y      ;} // get total number of cells

   // draw
   void draw(C Color &grid_color, C Color &back_color=TRANSPARENT)C;
#if EE_PRIVATE
   void draw(C Color &grid_color, C Color &field_color, Index *rect_edge)C;
#endif

   Rects() {}
   Rects(C Rect &rect, C VecI2 &cells) {set(rect, cells);}
   Rects(C Rect &rect,   Int    elms ) {set(rect, elms );}
};
/******************************************************************************/
inline RectI Round(C Rect &r) {return RectI(Round(r.min.x), Round(r.min.y), Round(r.max.x), Round(r.max.y));}
inline RectI Floor(C Rect &r) {return RectI(Floor(r.min.x), Floor(r.min.y), Floor(r.max.x), Floor(r.max.y));}
#if EE_PRIVATE
inline RectI RoundGPU(C Rect &r) {return RectI(RoundGPU(r.min.x), RoundGPU(r.min.y), RoundGPU(r.max.x), RoundGPU(r.max.y));}
#endif

// distance
Flt Dist (C Vec2 &point, C Rect  &rect); //         distance between point     and a rectangle
Flt Dist (C Vec2 &point, C RectI &rect); //         distance between point     and a rectangle
Flt Dist2(C Vec2 &point, C Rect  &rect); // squared distance between point     and a rectangle
Flt Dist2(C Vec2 &point, C RectI &rect); // squared distance between point     and a rectangle
Flt Dist (C Rect &a    , C Rect  &b   ); //         distance between rectangle and a rectangle

Flt Dist2PointSquare(C Vec2 &pos, C Vec2  &square_center, Flt square_radius);
Flt Dist2PointSquare(C Vec2 &pos, C VecI2 &square_center, Flt square_radius);

// cuts
inline Bool Cuts   (C Vec2  &point, C Rect  &rect) {return rect.includes(point);} // if point     cuts rectangle
inline Bool Cuts   (C VecD2 &point, C RectD &rect) {return rect.includes(point);} // if point     cuts rectangle
inline Bool Cuts   (C VecI2 &point, C RectI &rect) {return rect.includes(point);} // if point     cuts rectangle
       Bool Cuts   (C Edge2 &edge , C Rect  &rect);                               // if edge      cuts rectangle
       Bool Cuts   (C Rect  &a    , C Rect  &b   );                               // if rectangle cuts rectangle
       Bool Cuts   (C RectI &a    , C RectI &b   );                               // if rectangle cuts rectangle
       Bool CutsEps(C Vec2  &point, C Rect  &rect);                               // if point     cuts rectangle, with EPS epsilon tolerance

Bool Inside   (C Rect  &a, C Rect  &b); // if 'a' is fully inside 'b'
Bool Inside   (C RectI &a, C RectI &b); // if 'a' is fully inside 'b'
Bool InsideEps(C Rect  &a, C Rect  &b); // if 'a' is fully inside 'b', with EPS epsilon tolerance

// fit
Rect Fit(Flt src_aspect, C Rect &dest_rect, FIT_MODE fit=FIT_FULL); // get a rectangle that will fit in the 'dest_rect' while preserving the 'src_aspect' source aspect ratio using the 'fit' mode

// sweep
Bool SweepPointRect(C Vec2 &point, C Vec2 &move, C Rect &rect, Flt *hit_frac=null, Vec2 *hit_normal=null, Vec2 *hit_pos=null); // if moving point cuts through a static rectangle

// pack rectangles
struct RectSizeAnchor
{
   VecI2 size  , // size of the rectangle
         anchor; // anchor position (used when 'align_for_compression' is enabled)

   RectSizeAnchor() {anchor.zero();}
};
struct RectIndex : RectI
{
   Int index;
};
Bool PackRectsKnownLimit  (C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectI    > rects,      C VecI2 &limit , Bool allow_rotate=true, Int border=0, Bool align_for_compression=false, Bool compact_arrangement=false                                            ); // pack rectangles defined by their 'sizes' into 'rects', 'limit' =specified  size  which should be able to contain the rectangles, 'allow_rotate'=if allow rectangles to be rotated, 'border'=spacing between rectangles, 'align_for_compression'=if align rectangles on a 4-pixel boundary to allow for equal compression on all images (enabled 'allow_rotate' may prevent this option from best effect), 'compact_arrangement'=if try to make the arrangement of rectangles as compact as possible (this affects only arrangement and does not affect being able to fit in the limit), false on fail (if rectangles don't fit)
Bool PackRectsUnknownLimit(C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectI    > rects,        VecI2 &limit , Bool allow_rotate=true, Int border=0, Bool align_for_compression=false, Bool compact_arrangement=false, Bool only_square=false, Int max_size=65536); // pack rectangles defined by their 'sizes' into 'rects', 'limit' =calculated size  which should be able to contain the rectangles, 'allow_rotate'=if allow rectangles to be rotated, 'border'=spacing between rectangles, 'align_for_compression'=if align rectangles on a 4-pixel boundary to allow for equal compression on all images (enabled 'allow_rotate' may prevent this option from best effect), 'compact_arrangement'=if try to make the arrangement of rectangles as compact as possible (this affects only arrangement and does not affect being able to fit in the limit), 'only_square'=if test only squares (otherwise rectangles are tested too), 'max_size'=max size to test, false on fail (if rectangles don't fit)
Bool PackRectsMultiLimit  (C MemPtr<RectSizeAnchor> &sizes, MemPtr<RectIndex> rects, MemPtr<VecI2> limits, Bool allow_rotate=true, Int border=0, Bool align_for_compression=false, Bool compact_arrangement=false, Bool only_square=false, Int max_size=65536); // pack rectangles defined by their 'sizes' into 'rects', 'limits'=calculated sizes which should be able to contain the rectangles, 'allow_rotate'=if allow rectangles to be rotated, 'border'=spacing between rectangles, 'align_for_compression'=if align rectangles on a 4-pixel boundary to allow for equal compression on all images (enabled 'allow_rotate' may prevent this option from best effect), 'compact_arrangement'=if try to make the arrangement of rectangles as compact as possible (this affects only arrangement and does not affect being able to fit in the limit), 'only_square'=if test only squares (otherwise rectangles are tested too), 'max_size'=max size to test, 'index' member of 'rects' specifies the index in 'limits' array that rectangle got placed, false on fail (if rectangles don't fit)

Bool BestFit(Vec2  *point, Int points, Vec2  &axis); // find best-fit rectangle axis direction to store the points, false on fail
Bool BestFit(VecD2 *point, Int points, VecD2 &axis); // find best-fit rectangle axis direction to store the points, false on fail
/******************************************************************************/
