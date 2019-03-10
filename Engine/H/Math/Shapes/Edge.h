/******************************************************************************

   Use 'Edge2' to handle 2D edges, Flt type
   Use 'Edge'  to handle 3D edges, Flt type

   Use 'EdgeD2' to handle 2D edges, Dbl type
   Use 'EdgeD'  to handle 3D edges, Dbl type

/******************************************************************************/
struct Edge2 // Edge 2D
{
   Vec2 p[2]; // points

   // set
   Edge2& set(C Vec2 &p0    , C Vec2 &p1    ) {p[0]=p0        ; p[1]=p1        ; return T;}
   Edge2& set(Flt x0, Flt y0, Flt x1, Flt y1) {p[0].set(x0,y0); p[1].set(x1,y1); return T;}

   // get
   Flt  centerX(     )C {return        Avg(p[0].x, p[1].x);} // center x
   Flt  centerY(     )C {return        Avg(p[0].y, p[1].y);} // center y
   Vec2 center (     )C {return Vec2(centerX(), centerY());} // center
   Vec2 delta  (     )C {return                  p[1]-p[0];} // delta
   Vec2 dir    (     )C {return                  !delta() ;} // direction
   Vec2 perp   (     )C {return             Perp (delta());} // perpendicular
   Vec2 perpN  (     )C {return             PerpN(delta());} // perpendicular normalized
   Flt  length (     )C {return    Dist(p[0]  , p[1]     );} // length
   Vec2 lerp   (Flt s)C {return    Lerp(p[0]  , p[1]  , s);} // lerp from p[0]   to p[1]
   Flt  lerpX  (Flt s)C {return    Lerp(p[0].x, p[1].x, s);} // lerp from p[0].x to p[1].x
   Flt  lerpY  (Flt s)C {return    Lerp(p[0].y, p[1].y, s);} // lerp from p[0].y to p[1].y

   Str asText()C {return S+"Point0: "+p[0]+", Point1: "+p[1];} // get text description

   // operations
   Edge2& reverse() {Swap(p[0], p[1]); return T;} // reverse the order of points

   // transform
   Edge2& operator+=(C Vec2    &v) {p[0]+=v; p[1]+=v; return T;}
   Edge2& operator-=(C Vec2    &v) {p[0]-=v; p[1]-=v; return T;}
   Edge2& operator*=(  Flt      f) {p[0]*=f; p[1]*=f; return T;}
   Edge2& operator/=(  Flt      f) {p[0]/=f; p[1]/=f; return T;}
   Edge2& operator*=(C Vec2    &v) {p[0]*=v; p[1]*=v; return T;}
   Edge2& operator/=(C Vec2    &v) {p[0]/=v; p[1]/=v; return T;}
   Edge2& operator*=(C Matrix3 &m) {p[0]*=m; p[1]*=m; return T;}
   Edge2& operator*=(C Matrix  &m) {p[0]*=m; p[1]*=m; return T;}

   friend Edge2 operator+ (C Edge2 &edge, C Vec2    &v) {return Edge2(edge)+=v;}
   friend Edge2 operator- (C Edge2 &edge, C Vec2    &v) {return Edge2(edge)-=v;}
   friend Edge2 operator* (C Edge2 &edge,   Flt      f) {return Edge2(edge)*=f;}
   friend Edge2 operator/ (C Edge2 &edge,   Flt      f) {return Edge2(edge)/=f;}
   friend Edge2 operator* (C Edge2 &edge, C Vec2    &v) {return Edge2(edge)*=v;}
   friend Edge2 operator/ (C Edge2 &edge, C Vec2    &v) {return Edge2(edge)/=v;}
   friend Edge2 operator* (C Edge2 &edge, C Matrix3 &m) {return Edge2(edge)*=m;}
   friend Edge2 operator* (C Edge2 &edge, C Matrix  &m) {return Edge2(edge)*=m;}

   // draw
   void draw(C Color &color=WHITE            )C; // draw
   void draw(C Color &color ,   Flt    width )C; // draw with width
   void draw(C Color &color0, C Color &color1)C; // draw with per-point color

   Edge2() {}
   Edge2(C Vec2 &p                             ) {set(p ,  p);}
   Edge2(C Vec2 &p0, C Vec2 &p1                ) {set(p0, p1);}
   Edge2(  Flt   x0,   Flt   y0, Flt x1, Flt y1) {set(x0, y0, x1, y1);}
   Edge2(C EdgeD2 &edge                        );
};
/******************************************************************************/
struct EdgeD2 // Edge 2D (double precision)
{
   VecD2 p[2]; // points

   // set
   EdgeD2& set(C VecD2 &p0   , C VecD2 &p1   ) {p[0]=p0        ; p[1]=p1        ; return T;}
   EdgeD2& set(Dbl x0, Dbl y0, Dbl x1, Dbl y1) {p[0].set(x0,y0); p[1].set(x1,y1); return T;}

   // get
   Dbl   centerX(     )C {return         Avg(p[0].x, p[1].x);} // center x
   Dbl   centerY(     )C {return         Avg(p[0].y, p[1].y);} // center y
   VecD2 center (     )C {return VecD2(centerX(), centerY());} // center
   VecD2 delta  (     )C {return                   p[1]-p[0];} // delta
   VecD2 dir    (     )C {return                   !delta() ;} // direction
   VecD2 perp   (     )C {return              Perp (delta());} // perpendicular
   VecD2 perpN  (     )C {return              PerpN(delta());} // perpendicular normalized
   Dbl   length (     )C {return    Dist(p[0]  , p[1]     );} // length
   VecD2 lerp   (Dbl s)C {return    Lerp(p[0]  , p[1]  , s);} // lerp from p[0]   to p[1]
   Dbl   lerpX  (Dbl s)C {return    Lerp(p[0].x, p[1].x, s);} // lerp from p[0].x to p[1].x
   Dbl   lerpY  (Dbl s)C {return    Lerp(p[0].y, p[1].y, s);} // lerp from p[0].y to p[1].y

   // draw
   void draw(C Color &color=WHITE            )C; // draw
   void draw(C Color &color ,   Flt    width )C; // draw with width
   void draw(C Color &color0, C Color &color1)C; // draw with per-point color

   // operations
   EdgeD2& reverse() {Swap(p[0], p[1]); return T;} // reverse the order of points

   // transform
   EdgeD2& operator+=(C VecD2   &v) {p[0]+=v; p[1]+=v; return T;}
   EdgeD2& operator-=(C VecD2   &v) {p[0]-=v; p[1]-=v; return T;}
   EdgeD2& operator*=(  Dbl      r) {p[0]*=r; p[1]*=r; return T;}
   EdgeD2& operator/=(  Dbl      r) {p[0]/=r; p[1]/=r; return T;}
   EdgeD2& operator*=(C MatrixD &m) {p[0]*=m; p[1]*=m; return T;}

   friend EdgeD2 operator+ (C EdgeD2 &edge, C VecD2   &v) {return EdgeD2(edge)+=v;}
   friend EdgeD2 operator- (C EdgeD2 &edge, C VecD2   &v) {return EdgeD2(edge)-=v;}
   friend EdgeD2 operator* (C EdgeD2 &edge,   Dbl      r) {return EdgeD2(edge)*=r;}
   friend EdgeD2 operator/ (C EdgeD2 &edge,   Dbl      r) {return EdgeD2(edge)/=r;}
   friend EdgeD2 operator* (C EdgeD2 &edge, C MatrixD &m) {return EdgeD2(edge)*=m;}

   EdgeD2() {}
   EdgeD2(C VecD2 &p                              ) {set(p , p );}
   EdgeD2(C VecD2 &p0, C VecD2 &p1                ) {set(p0, p1);}
   EdgeD2(  Dbl    x0,   Dbl    y0, Dbl x1, Dbl y1) {set(x0, y0, x1, y1);}
   EdgeD2(C Edge2 &edge);
};
/******************************************************************************/
struct Edge // Edge 3D
{
   Vec p[2]; // points

   // set
   Edge& set(C Vec   &p0, C Vec  &p1       ) {p[0]=p0               ; p[1]=p1               ; return T;}
   Edge& set(C Vec2  &p0, C Vec2 &p1, Flt z) {p[0].set(p0       , z); p[1].set(p1       , z); return T;}
   Edge& set(C Edge2 &edge          , Flt z) {p[0].set(edge.p[0], z); p[1].set(edge.p[1], z); return T;}

   // get
   Edge2 xy     (     )C {return          Edge2(p[0].xy  , p[1].xy  );} // get    XY components as Edge2
   Edge2 xz     (     )C {return          Edge2(p[0].xz(), p[1].xz());} // get    XZ components as Edge2
   Flt   centerX(     )C {return            Avg(p[0].x   , p[1].x   );} // center x
   Flt   centerY(     )C {return            Avg(p[0].y   , p[1].y   );} // center y
   Flt   centerZ(     )C {return            Avg(p[0].z   , p[1].z   );} // center z
   Vec   center (     )C {return Vec(centerX(), centerY(), centerZ());} // center
   Vec   delta  (     )C {return                            p[1]-p[0];} // delta
   Vec   dir    (     )C {return                             !delta();} // direction
   Flt   length (     )C {return              Dist(p[0]  , p[1]     );} // length
   Vec   lerp   (Flt s)C {return              Lerp(p[0]  , p[1]  , s);} // lerp from p[0]   to p[1]
   Flt   lerpX  (Flt s)C {return              Lerp(p[0].x, p[1].x, s);} // lerp from p[0].x to p[1].x
   Flt   lerpY  (Flt s)C {return              Lerp(p[0].y, p[1].y, s);} // lerp from p[0].y to p[1].y
   Flt   lerpZ  (Flt s)C {return              Lerp(p[0].z, p[1].z, s);} // lerp from p[0].z to p[1].z

   Str asText()C {return S+"Point0: "+p[0]+", Point1: "+p[1];} // get text description

   // operations
   Edge& reverse() {Swap(p[0], p[1]); return T;} // reverse the order of points

   // transform
   Edge& operator+=(C Vec     &v) {p[0]+=v; p[1]+=v; return T;}
   Edge& operator-=(C Vec     &v) {p[0]-=v; p[1]-=v; return T;}
   Edge& operator*=(  Flt      f) {p[0]*=f; p[1]*=f; return T;}
   Edge& operator/=(  Flt      f) {p[0]/=f; p[1]/=f; return T;}
   Edge& operator*=(C Vec     &v) {p[0]*=v; p[1]*=v; return T;}
   Edge& operator/=(C Vec     &v) {p[0]/=v; p[1]/=v; return T;}
   Edge& operator*=(C Matrix3 &m) {p[0]*=m; p[1]*=m; return T;}
   Edge& operator*=(C Matrix  &m) {p[0]*=m; p[1]*=m; return T;}
   Edge& operator/=(C Matrix3 &m) {return div(m);}
   Edge& operator/=(C Matrix  &m) {return div(m);}

   friend Edge operator+ (C Edge &edge, C Vec     &v) {return Edge(edge)+=v;}
   friend Edge operator- (C Edge &edge, C Vec     &v) {return Edge(edge)-=v;}
   friend Edge operator* (C Edge &edge,   Flt      f) {return Edge(edge)*=f;}
   friend Edge operator/ (C Edge &edge,   Flt      f) {return Edge(edge)/=f;}
   friend Edge operator* (C Edge &edge, C Vec     &v) {return Edge(edge)*=v;}
   friend Edge operator/ (C Edge &edge, C Vec     &v) {return Edge(edge)/=v;}
   friend Edge operator* (C Edge &edge, C Matrix3 &m) {return Edge(edge)*=m;}
   friend Edge operator/ (C Edge &edge, C Matrix3 &m) {return Edge(edge)/=m;}
   friend Edge operator* (C Edge &edge, C Matrix  &m) {return Edge(edge)*=m;}
   friend Edge operator/ (C Edge &edge, C Matrix  &m) {return Edge(edge)/=m;}

   Edge& div          (C Matrix3 &matrix); // transform by matrix inverse
   Edge& div          (C Matrix  &matrix); // transform by matrix inverse
   Edge& divNormalized(C Matrix3 &matrix); // transform by matrix inverse, this method is faster than 'div' however 'matrix' must be normalized
   Edge& divNormalized(C Matrix  &matrix); // transform by matrix inverse, this method is faster than 'div' however 'matrix' must be normalized

   // draw
   void draw(C Color &color=WHITE            )C; // draw                     , this relies on active object matrix which can be set using 'SetMatrix' function
   void draw(C Color &color ,   Flt    width )C; // draw with width          , this relies on active object matrix which can be set using 'SetMatrix' function
   void draw(C Color &color0, C Color &color1)C; // draw with per-point color, this relies on active object matrix which can be set using 'SetMatrix' function

   Edge() {}
   Edge(C Vec   &p                    ) {set(p , p    );}
   Edge(C Vec   &p0, C Vec  &p1       ) {set(p0, p1   );}
   Edge(C Vec2  &p0, C Vec2 &p1, Flt z) {set(p0, p1, z);}
   Edge(C Edge2 &edge          , Flt z) {set(edge  , z);}
   Edge(C EdgeD &edge                 );
};
/******************************************************************************/
struct EdgeD // Edge 3D (double precision)
{
   VecD p[2]; // points

   // set
   EdgeD& set(C VecD  &p0, C VecD  &p1       ) {p[0]=p0       ; p[1]=p1       ; return T;}
   EdgeD& set(C VecD2 &p0, C VecD2 &p1, Dbl z) {p[0].set(p0,z); p[1].set(p1,z); return T;}

   // get
   Dbl  centerX(     )C {return                   Avg(p[0].x, p[1].x);} // center x
   Dbl  centerY(     )C {return                   Avg(p[0].y, p[1].y);} // center y
   Dbl  centerZ(     )C {return                   Avg(p[0].z, p[1].z);} // center z
   VecD center (     )C {return VecD(centerX(), centerY(), centerZ());} // center
   VecD delta  (     )C {return                             p[1]-p[0];} // delta
   VecD dir    (     )C {return                              !delta();} // direction
   Dbl  length (     )C {return               Dist(p[0]  , p[1]     );} // length
   VecD lerp   (Dbl s)C {return               Lerp(p[0]  , p[1]  , s);} // lerp from p[0]   to p[1]
   Dbl  lerpX  (Dbl s)C {return               Lerp(p[0].x, p[1].x, s);} // lerp from p[0].x to p[1].x
   Dbl  lerpY  (Dbl s)C {return               Lerp(p[0].y, p[1].y, s);} // lerp from p[0].y to p[1].y
   Dbl  lerpZ  (Dbl s)C {return               Lerp(p[0].z, p[1].z, s);} // lerp from p[0].z to p[1].z

   // operations
   EdgeD& reverse() {Swap(p[0], p[1]); return T;} // reverse the order of points

   // transform
   EdgeD& operator+=(C VecD    &v) {p[0]+=v; p[1]+=v; return T;}
   EdgeD& operator-=(C VecD    &v) {p[0]-=v; p[1]-=v; return T;}
   EdgeD& operator*=(  Dbl      r) {p[0]*=r; p[1]*=r; return T;}
   EdgeD& operator/=(  Dbl      r) {p[0]/=r; p[1]/=r; return T;}
   EdgeD& operator*=(C MatrixD &m) {p[0]*=m; p[1]*=m; return T;}

   friend EdgeD operator+ (C EdgeD &edge, C VecD    &v) {return EdgeD(edge)+=v;}
   friend EdgeD operator- (C EdgeD &edge, C VecD    &v) {return EdgeD(edge)-=v;}
   friend EdgeD operator* (C EdgeD &edge,   Dbl      r) {return EdgeD(edge)*=r;}
   friend EdgeD operator/ (C EdgeD &edge,   Dbl      r) {return EdgeD(edge)/=r;}
   friend EdgeD operator* (C EdgeD &edge, C MatrixD &m) {return EdgeD(edge)*=m;}

   // draw
   void draw(C Color &color=WHITE            )C; // draw                     , this relies on active object matrix which can be set using 'SetMatrix' function
   void draw(C Color &color ,   Flt    width )C; // draw with width          , this relies on active object matrix which can be set using 'SetMatrix' function
   void draw(C Color &color0, C Color &color1)C; // draw with per-point color, this relies on active object matrix which can be set using 'SetMatrix' function

   EdgeD() {}
   EdgeD(C VecD &p             ) {set(p , p );}
   EdgeD(C VecD &p0, C VecD &p1) {set(p0, p1);}
   EdgeD(C Edge &edge          );
};
/******************************************************************************/
struct Edge2_I : Edge2 // Edge 2D + information
{
   Flt  length; // length
   Vec2 dir   , // direction
        normal; // normal

   // set
   void set(C Vec2  &a, C Vec2 &b, C Vec2 *normal=null);
   void set(C Edge2 &edge                             ) {set(edge.p[0], edge.p[1]);}

   Edge2_I() {}
   Edge2_I(C Vec2  &a, C Vec2 &b, C Vec2 *normal=null){set(a, b, normal);}
   Edge2_I(C Edge2 &edge                             ){set(edge        );}
};
/******************************************************************************/
struct EdgeD2_I : EdgeD2 // Edge 2D + information (double precision)
{
   Dbl   length; // length
   VecD2 dir   , // direction
         normal; // normal

   // set
   void set(C VecD2  &a, C VecD2 &b, C VecD2 *normal=null);
   void set(C EdgeD2 &edge                               ) {set(edge.p[0], edge.p[1]);}

   EdgeD2_I() {}
   EdgeD2_I(C VecD2  &a, C VecD2 &b, C VecD2 *normal=null) {set(a, b, normal);}
   EdgeD2_I(C EdgeD2 &edge                               ) {set(edge        );}
};
/******************************************************************************/
struct Edge_I : Edge // Edge 3D + information
{
   Flt length; // length
   Vec dir   ; // direction

   // set
   void set(C Vec  &a, C Vec &b);
   void set(C Edge &edge       ) {set(edge.p[0], edge.p[1]);}

   Edge_I() {}
   Edge_I(C Vec  &a, C Vec &b) {set(a, b);}
   Edge_I(C Edge &edge       ) {set(edge);}
};
/******************************************************************************/
struct EdgeD_I : EdgeD // Edge 3D + information (double precision)
{
   Dbl  length; // length
   VecD dir   ; // direction

   // set
   void set(C VecD  &a, C VecD &b);
   void set(C EdgeD &edge        ) {set(edge.p[0], edge.p[1]);}

   EdgeD_I() {}
   EdgeD_I(C VecD  &a, C VecD &b) {set(a, b);}
   EdgeD_I(C EdgeD &edge        ) {set(edge);}
};
/******************************************************************************/
struct PixelWalker // iterates through pixels of a rasterized edge
{
   // get
   Bool   active  ()C {return _active   ;} // if  walker still active
   Bool   sideStep()C {return _side_step;} // if  last move was a side step (not along the main axis)
 C VecI2& pos     ()C {return _pos_temp ;} // get current position

   // set
   void start(C VecI2 &start, C VecI2 &end); // start walking from 'start' to 'end'
   void start(C Vec2  &start, C Vec2  &end); // start walking from 'start' to 'end'

   // operations
   void step(); // make a single step

   PixelWalker(                            ) {_active=false;}
   PixelWalker(C VecI2 &start, C VecI2 &end) {_active=false; T.start(start, end);}
   PixelWalker(C Vec2  &start, C Vec2  &end) {_active=false; T.start(start, end);}

#if !EE_PRIVATE
private:
#endif
   Bool  _active, _axis, _side_step;
   Int   _steps , _main_sign;
   VecI2 _pos , _pos_temp;
   Vec2  _posr, _step;
};
STRUCT(PixelWalkerMask , PixelWalker) // iterates through pixels of a rasterized edge only within a specified mask
//{
   // set
   void start(C VecI2 &start, C VecI2 &end, C RectI &mask); // start walking from 'start' to 'end', 'mask'=process pixels only within this inclusive rectangle
   void start(C Vec2  &start, C Vec2  &end, C RectI &mask); // start walking from 'start' to 'end', 'mask'=process pixels only within this inclusive rectangle

   // operations
   void step(); // make a single step

   PixelWalkerMask(                                           ) {}
   PixelWalkerMask(C VecI2 &start, C VecI2 &end, C RectI &mask) {T.start(start, end, mask);}
   PixelWalkerMask(C Vec2  &start, C Vec2  &end, C RectI &mask) {T.start(start, end, mask);}

private:
   RectI _mask;
};
/******************************************************************************/
struct PixelWalkerEdge // iterates through pixels of a rasterized line and returns intersections with the edges
{
   // get
   Bool  active  ()C {return _active   ;} // if  walker still active
   Bool  sideStep()C {return _side_step;} // if  last move was a side step (not along the main axis)
 C Vec2& pos     ()C {return _posr     ;} // get current position

   // set
   void start(C Vec2 &start, C Vec2 &end); // start walking from 'start' to 'end'

   // operations
   void step(); // make a single step

   PixelWalkerEdge(                          ) {_active=false;}
   PixelWalkerEdge(C Vec2 &start, C Vec2 &end) {_active=false; T.start(start, end);}

#if !EE_PRIVATE
private:
#endif
   Bool _active, _axis, _side_step;
   Int  _steps, _pos;
   Vec2 _posr, _pos_next, _step, _pos_end;
};
STRUCT(PixelWalkerEdgeMask , PixelWalkerEdge) // iterates through pixels of a rasterized line and returns intersections with the edges only within a specified mask
//{
   // set
   void start(C Vec2 &start, C Vec2 &end, C RectI &mask); // start walking from 'start' to 'end', 'mask'=process pixels only within this inclusive rectangle

   // operations
   void step(); // make a single step

   PixelWalkerEdgeMask(                                         ) {}
   PixelWalkerEdgeMask(C Vec2 &start, C Vec2 &end, C RectI &mask) {T.start(start, end, mask);}

private:
   Rect _mask;
};
/******************************************************************************/
struct VoxelWalker // iterates through voxels of a rasterized edge
{
   // get
   Bool  active  ()C {return _active      ;} // if  walker still active
   Bool  sideStep()C {return _side_step!=0;} // if  last move was a side step (not along the main axis)
 C VecI& pos     ()C {return _pos_temp    ;} // get current position

   // set
   void start(C VecI &start, C VecI &end); // start walking from 'start' to 'end'
   void start(C Vec  &start, C Vec  &end); // start walking from 'start' to 'end'

   // operations
   void step(); // make a single step

   VoxelWalker(                          ) {_active=false;}
   VoxelWalker(C VecI &start, C VecI &end) {_active=false; T.start(start, end);}
   VoxelWalker(C Vec  &start, C Vec  &end) {_active=false; T.start(start, end);}

private:
   Bool _active;
   Byte _side_step;
   Int  _steps, _main_sign;
   VecI _axis; // 0, 1, 2 axis sorted by most important movement
   VecI _pos , _pos_temp;
   Vec  _posr, _step;
};
/******************************************************************************/
inline Edge2 ::Edge2 (C EdgeD2 &edge){p[0]=edge.p[0]; p[1]=edge.p[1];}
inline EdgeD2::EdgeD2(C Edge2  &edge){p[0]=edge.p[0]; p[1]=edge.p[1];}
inline Edge  ::Edge  (C EdgeD  &edge){p[0]=edge.p[0]; p[1]=edge.p[1];}
inline EdgeD ::EdgeD (C Edge   &edge){p[0]=edge.p[0]; p[1]=edge.p[1];}
/******************************************************************************/
// distance between point and a straight line
Flt DistPointStr(C Vec2  &point, C Vec2  &str, C Vec2  &dir);
Dbl DistPointStr(C VecD2 &point, C VecD2 &str, C VecD2 &dir);
Flt DistPointStr(C Vec   &point, C Vec   &str, C Vec   &dir);
Dbl DistPointStr(C VecD  &point, C Vec   &str, C Vec   &dir);
Dbl DistPointStr(C VecD  &point, C VecD  &str, C VecD  &dir);

// distance between 2 straight lines
Flt DistStrStr(C Vec &pos_a, C Vec &dir_a, C Vec &pos_b, C Vec &dir_b);

// distance between point and an edge
       Flt DistPointEdge(C Vec2  &point, C Vec2   &edge_a, C Vec2  &edge_b, DIST_TYPE *type=null);
       Dbl DistPointEdge(C VecD2 &point, C VecD2  &edge_a, C VecD2 &edge_b, DIST_TYPE *type=null);
       Flt DistPointEdge(C Vec   &point, C Vec    &edge_a, C Vec   &edge_b, DIST_TYPE *type=null);
       Dbl DistPointEdge(C VecD  &point, C Vec    &edge_a, C Vec   &edge_b, DIST_TYPE *type=null);
       Dbl DistPointEdge(C VecD  &point, C VecD   &edge_a, C VecD  &edge_b, DIST_TYPE *type=null);
inline Flt Dist         (C Vec2  &point, C Edge2  &edge  ,                  DIST_TYPE *type=null) {return DistPointEdge(point, edge.p[0], edge.p[1], type);}
inline Dbl Dist         (C VecD2 &point, C EdgeD2 &edge  ,                  DIST_TYPE *type=null) {return DistPointEdge(point, edge.p[0], edge.p[1], type);}
inline Flt Dist         (C Vec   &point, C Edge   &edge  ,                  DIST_TYPE *type=null) {return DistPointEdge(point, edge.p[0], edge.p[1], type);}
inline Dbl Dist         (C VecD  &point, C EdgeD  &edge  ,                  DIST_TYPE *type=null) {return DistPointEdge(point, edge.p[0], edge.p[1], type);}

// distance between 2 edges
Flt Dist(C Edge2 &a, C Edge2 &b);
Flt Dist(C Edge  &a, C Edge  &b);

// distance between edge and a plane
Flt Dist(C Edge &edge, C Plane &plane);

       Vec2 NearestPointOnEdge(C Vec2 &point, C Vec2  &edge_a, C Vec2 &edge_b);                                                          // nearest point on edge
       Vec  NearestPointOnEdge(C Vec  &point, C Vec   &edge_a, C Vec  &edge_b);                                                          // nearest point on edge
inline Vec2 NearestPointOnEdge(C Vec2 &point, C Edge2 &edge                  ) {return NearestPointOnEdge(point, edge.p[0], edge.p[1]);} // nearest point on edge
inline Vec  NearestPointOnEdge(C Vec  &point, C Edge  &edge                  ) {return NearestPointOnEdge(point, edge.p[0], edge.p[1]);} // nearest point on edge
       Vec  NearestPointOnStr (C Vec  &point, C Vec   &str  , C Vec &dir     );                                                          // nearest point on straight line
       Bool NearestPointOnStr (C Vec  &pos_a, C Vec   &dir_a, C Vec &pos_b, C Vec &dir_b, Edge &out);                                    // nearest points on straight lines, return true is succesfull, put points to out.p[]

// if points cuts edge (with epsilon)
       Bool CutsPointEdge(C Vec2  &point, C Edge2_I  &ei  , Vec2  *cuts=null);
       Bool CutsPointEdge(C VecD2 &point, C EdgeD2_I &ei  , VecD2 *cuts=null);
       Bool CutsPointEdge(C Vec   &point, C Edge_I   &ei  , Vec   *cuts=null);
       Bool CutsPointEdge(C VecD  &point, C EdgeD_I  &ei  , VecD  *cuts=null);
inline Bool Cuts         (C Vec2  &point, C Edge2    &edge, Vec2  *cuts=null) {return CutsPointEdge(point, Edge2_I (edge), cuts);}
inline Bool Cuts         (C VecD2 &point, C EdgeD2   &edge, VecD2 *cuts=null) {return CutsPointEdge(point, EdgeD2_I(edge), cuts);}
inline Bool Cuts         (C Vec   &point, C Edge     &edge, Vec   *cuts=null) {return CutsPointEdge(point, Edge_I  (edge), cuts);}
inline Bool Cuts         (C VecD  &point, C EdgeD    &edge, VecD  *cuts=null) {return CutsPointEdge(point, EdgeD_I (edge), cuts);}

// if edge cuts edge (with epsilon)
Int CutsEdgeEdge(C Edge2_I  &a, C Edge2_I  &b, Edge2  *cuts=null);
Int CutsEdgeEdge(C EdgeD2_I &a, C EdgeD2_I &b, EdgeD2 *cuts=null);
Int CutsEdgeEdge(C Edge_I   &a, C Edge_I   &b, Edge   *cuts=null);
Int CutsEdgeEdge(C EdgeD_I  &a, C EdgeD_I  &b, EdgeD  *cuts=null);

// if edge cuts plane (with epsilon)
Bool Cuts(C Edge  &edge, C Plane  &plane, Vec  *cuts=null);
Bool Cuts(C EdgeD &edge, C PlaneM &plane, VecD *cuts=null);

// clip edge to plane (no epsilon)
Bool Clip(Edge2  &edge, C Plane2  &plane);
Bool Clip(EdgeD2 &edge, C PlaneD2 &plane);
Bool Clip(Edge   &edge, C Plane   &plane);
Bool Clip(EdgeD  &edge, C PlaneD  &plane);

// clip edge to plane (with epsilon)
Bool ClipEps(Edge2  &edge, C Plane2  &plane);
Bool ClipEps(EdgeD2 &edge, C PlaneD2 &plane);
Bool ClipEps(Edge   &edge, C Plane   &plane);
Bool ClipEps(EdgeD  &edge, C PlaneD  &plane);

// clip edge to rectangle (no epsilon)
Bool Clip(Edge2  &edge, C Rect  &rect);
Bool Clip(EdgeD2 &edge, C RectD &rect);

// clip edge to box (no epsilon)
Bool Clip(Edge  &edge, C Box    &box);
Bool Clip(EdgeD &edge, C BoxD   &box);
Bool Clip(Edge  &edge, C Extent &ext);

// sweep
Bool SweepPointEdge(C Vec2  &point, C Vec2  &move, C Edge2_I  &ei, Flt *hit_frac=null, Vec2  *hit_normal=null, Vec2  *hit_pos=null); // if moving point cuts through a static edge
Bool SweepPointEdge(C VecD2 &point, C VecD2 &move, C EdgeD2_I &ei, Dbl *hit_frac=null, VecD2 *hit_normal=null, VecD2 *hit_pos=null); // if moving point cuts through a static edge

// draw
void DrawArrow (C Color &color, C Vec &start, C Vec &end,                  Flt tip_radius=0.1f, Flt tip_length=0.1f);
void DrawArrow2(C Color &color, C Vec &start, C Vec &end, Flt width=0.02f, Flt tip_radius=0.1f, Flt tip_length=0.1f);

// subdivide
void SubdivideEdges(C MemPtr<Vec    > &src, MemPtr<Vec    > dest); // subdivide continuous points from 'src' into 'dest'
void SubdivideEdges(C MemPtr<VtxFull> &src, MemPtr<VtxFull> dest); // subdivide continuous points from 'src' into 'dest'
/******************************************************************************/
