/******************************************************************************

   Use 'Tri2' to handle 2D triangles, Flt type
   Use 'Tri'  to handle 3D triangles, Flt type

   Use 'TriD2' to handle 2D triangles, Dbl type
   Use 'TriD'  to handle 3D triangles, Dbl type

   Use 'GetNormal' to calculate surface normal from 3 surface points.

/******************************************************************************/
// calculate normal from 3 triangle points
Vec  GetNormal (C Vec  &p0, C Vec  &p1, C Vec  &p2);
VecD GetNormal (C VecD &p0, C VecD &p1, C VecD &p2);
Vec  GetNormalU(C Vec  &p0, C Vec  &p1, C Vec  &p2); // calculate un-normalized normal vector from 3 triangle points (length of the vector is equal to the area of the triangle * 2)
VecD GetNormalU(C VecD &p0, C VecD &p1, C VecD &p2); // calculate un-normalized normal vector from 3 triangle points (length of the vector is equal to the area of the triangle * 2)
#if EE_PRIVATE
Vec  GetNormalU   (           C Vec &p1, C Vec &p2); // this is 'GetNormalU' with "p0==VecZero"
Vec  GetNormalEdge(C Vec &p0, C Vec &p1           ); // calculate partial normal vector from 2 edge points, this needs to be called for every edge in a polygon and summed together
#endif

// calculate triangle area
Flt TriArea2(C Vec  &p0, C Vec  &p1, C Vec  &p2); // calculate "area of the triangle * 2", this is the same as "GetNormalU(p0, p1, p2).length()"
Dbl TriArea2(C VecD &p0, C VecD &p1, C VecD &p2); // calculate "area of the triangle * 2", this is the same as "GetNormalU(p0, p1, p2).length()"
/******************************************************************************/
struct Tri2 // Triangle 2D
{
   Vec2 p[3]; // points

   Tri2& set     (C Vec2 &p0, C Vec2 &p1, C Vec2 &p2) {p[0]=p0; p[1]=p1; p[2]=p2; return T;}
   Tri2& setArrow(Flt direction, Flt angle=PI2/3);

   // get
   Vec2  center   (           )C {return Avg(p[0], p[1], p[2]);} // get center
   Edge2 edge0    (           )C {return     Edge2(p[0], p[1]);} // get 0-1  edge
   Edge2 edge1    (           )C {return     Edge2(p[1], p[2]);} // get 1-2  edge
   Edge2 edge2    (           )C {return     Edge2(p[2], p[0]);} // get 2-0  edge
   Flt   area     (           )C;                                // get surface area
   Bool  valid    (Flt eps=EPS)C;                                // if valid (points aren't inline)
   Bool  clockwise(           )C;                                // if points are in clockwise order

   // operations
   void circularLerp(Tri2 *tri, Int num)C; // set 'tri' 'num' number of triangles as a circular interpolation of the current triangle, interpolation occurs between p[1] and p[2] points, p[0] stays the same as in current triangle

   // draw
   void draw(C Color &color=WHITE, Bool fill=true)C;

   Tri2& operator+=(C Vec2 &v);
   Tri2& operator-=(C Vec2 &v);
   Tri2& operator*=(  Flt   r);
   Tri2& operator/=(  Flt   r);

   friend Tri2 operator+ (C Tri2 &tri, C Vec2 &v) {return Tri2(tri)+=v;}
   friend Tri2 operator- (C Tri2 &tri, C Vec2 &v) {return Tri2(tri)-=v;}
   friend Tri2 operator* (C Tri2 &tri,   Flt   r) {return Tri2(tri)*=r;}
   friend Tri2 operator/ (C Tri2 &tri,   Flt   r) {return Tri2(tri)/=r;}

   Tri2() {}
   Tri2(C Vec2  &p0, C Vec2 &p1, C Vec2 &p2) {set(p0, p1, p2);}
   Tri2(C TriD2 &tri);
};
/******************************************************************************/
struct TriD2 // Triangle 2D (double precision)
{
   VecD2 p[3]; // points

   TriD2& set(C VecD2 &p0, C VecD2 &p1, C VecD2 &p2) {p[0]=p0; p[1]=p1; p[2]=p2; return T;}

   // get
   VecD2  center   ()C {return Avg(p[0], p[1], p[2]);} // get center
   EdgeD2 edge0    ()C {return    EdgeD2(p[0], p[1]);} // get 0-1  edge
   EdgeD2 edge1    ()C {return    EdgeD2(p[1], p[2]);} // get 1-2  edge
   EdgeD2 edge2    ()C {return    EdgeD2(p[2], p[0]);} // get 2-0  edge
   Dbl    area     ()C;                                // get surface area
   Bool   clockwise()C;                                // if points are in clockwise order

   // draw
   void draw(C Color &color=WHITE, Bool fill=true)C;

   TriD2& operator+=(C VecD2 &v);
   TriD2& operator-=(C VecD2 &v);
   TriD2& operator*=(  Dbl    r);
   TriD2& operator/=(  Dbl    r);

   friend TriD2 operator+ (C TriD2 &tri, C VecD2 &v) {return TriD2(tri)+=v;}
   friend TriD2 operator- (C TriD2 &tri, C VecD2 &v) {return TriD2(tri)-=v;}
   friend TriD2 operator* (C TriD2 &tri,   Dbl    r) {return TriD2(tri)*=r;}
   friend TriD2 operator/ (C TriD2 &tri,   Dbl    r) {return TriD2(tri)/=r;}

   TriD2() {}
   TriD2(C VecD2 &p0, C VecD2 &p1, C VecD2 &p2) {set(p0, p1, p2);}
   TriD2(C Tri2  &tri);
};
/******************************************************************************/
struct Tri // Triangle 3D
{
   Vec p[3], // points
       n   ; // normal

   Tri& set      (C Vec &p0, C Vec &p1, C Vec &p2, C Vec *normal=null);
   Tri& setNormal() {n=getNormal(); return T;} // recalculate normal

   // get
   Vec   center    (           )C {return Avg  (p[0], p[1], p[2]          );} // get center
   Plane plane     (           )C {return Plane(p[0], n                   );} // return as Plane
   Edge  edge      (Int n      )C {return Edge (p[Mod(n,3)], p[Mod(n+1,3)]);} // get n-th edge
   Edge  edge0     (           )C {return Edge (p[      0 ], p[        1 ]);} // get 0-1  edge
   Edge  edge1     (           )C {return Edge (p[      1 ], p[        2 ]);} // get 1-2  edge
   Edge  edge2     (           )C {return Edge (p[      2 ], p[        0 ]);} // get 2-0  edge
   Vec   getNormal (           )C {return      GetNormal (p[0], p[1], p[2]);} // calculate    normalized normal vector from triangle points
   Vec   getNormalU(           )C {return      GetNormalU(p[0], p[1], p[2]);} // calculate un-normalized normal vector from triangle points (length of the vector is proportional to the area of the triangle)
   Flt   area      (           )C; // get surface area
   Bool  valid     (Flt eps=EPS)C; // if valid (points aren't inline)
   Bool  coplanar  (C Tri &tri )C; // if coplanar

   // operations
   void circularLerp(Tri *tri, Int num)C; // set 'tri' 'num' number of triangles as a circular interpolation of the current triangle, interpolation occurs between p[1] and p[2] points, p[0] stays the same as in current triangle

   // draw
   void draw(C Color &color=WHITE, Bool fill=false)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Tri& operator+=(C Vec &v);
   Tri& operator-=(C Vec &v);
   Tri& operator*=(  Flt  r);
   Tri& operator/=(  Flt  r);

   friend Tri operator+ (C Tri &tri, C Vec &v) {return Tri(tri)+=v;}
   friend Tri operator- (C Tri &tri, C Vec &v) {return Tri(tri)-=v;}
   friend Tri operator* (C Tri &tri,   Flt  r) {return Tri(tri)*=r;}
   friend Tri operator/ (C Tri &tri,   Flt  r) {return Tri(tri)/=r;}

   Tri() {}
   Tri(C Vec  &p0, C Vec &p1, C Vec &p2, C Vec *normal=null) {set(p0, p1, p2, normal);}
   Tri(C TriD &tri);
};
/******************************************************************************/
struct TriD // Triangle 3D (double precision)
{
   VecD p[3], // points
        n   ; // normal

   TriD& set      (C VecD &p0, C VecD &p1, C VecD &p2, C VecD *normal=null);
   TriD& setNormal() {n=GetNormal(p[0], p[1], p[2]); return T;} // recalculate normal

   // get
   VecD   center  (           )C {return Avg   (p[0], p[1], p[2]);} // get center
   PlaneD plane   (           )C {return PlaneD(p[0], n         );} // return as PlaneD
   EdgeD  edge0   (           )C {return EdgeD (p[0], p[1]      );} // get 0-1 edge
   EdgeD  edge1   (           )C {return EdgeD (p[1], p[2]      );} // get 1-2 edge
   EdgeD  edge2   (           )C {return EdgeD (p[2], p[0]      );} // get 2-0 edge
   Dbl    area    (           )C;                                   // get surface area
   Bool   coplanar(C TriD &tri)C;                                   // if coplanar

   // draw
   void draw(C Color &color=WHITE, Bool fill=false)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   TriD& operator+=(C VecD &v);
   TriD& operator-=(C VecD &v);
   TriD& operator*=(  Dbl   r);
   TriD& operator/=(  Dbl   r);

   friend TriD operator+ (C TriD &tri, C VecD &v) {return TriD(tri)+=v;}
   friend TriD operator- (C TriD &tri, C VecD &v) {return TriD(tri)-=v;}
   friend TriD operator* (C TriD &tri,   Dbl   r) {return TriD(tri)*=r;}
   friend TriD operator/ (C TriD &tri,   Dbl   r) {return TriD(tri)/=r;}

   TriD() {}
   TriD(C VecD &p0, C VecD &p1, C VecD &p2, C VecD *normal=null) {set(p0, p1, p2, normal);}
   TriD(C Tri  &tri);
};
/******************************************************************************/
// angle
Flt TriABAngle(Flt a_length, Flt b_length, Flt c_length); // calculate the angle between 'a' 'b' sides of a triangle made from 'a', 'b', 'c' sides when length of the sides are known

// return blending factors 'blend' that (blend.x*tri.p[0] + blend.y*tri.p[1] + blend.z*tri.p[2] == p), these are also known as "barycentric coordinates"
Vec  TriBlend(C Vec2  &p, C Tri2  &tri);
VecD TriBlend(C VecD2 &p, C TriD2 &tri);
Vec  TriBlend(C Vec   &p, C Tri   &tri, Bool pos_on_tri_plane); // 'pos_on_tri_plane'=if 'p' position lies on 'tri' triangle plane (if you're not sure, then set false)
VecD TriBlend(C VecD  &p, C TriD  &tri, Bool pos_on_tri_plane); // 'pos_on_tri_plane'=if 'p' position lies on 'tri' triangle plane (if you're not sure, then set false)

// return blending factors 'blend' that (blend.x*p0 + blend.y*p1 + blend.z*p2 + blend.w*p3 == p), these are also known as "barycentric coordinates"
Vec4 TetraBlend(C Vec &p, C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3);

// get volume of a Tetrahedron shape
Flt TetraVolume(C Vec &a, C Vec &b, C Vec &c, C Vec &d);

// distance
Flt Dist(C Vec2 &point, C Tri2 &tri, DIST_TYPE *type=null); // distance between point and a triangle
Flt Dist(C Vec  &point, C Tri  &tri, DIST_TYPE *type=null); // distance between point and a triangle
Flt Dist(C Edge &edge , C Tri  &tri); // distance between edge     and a triangle
Flt Dist(C Tri  &a    , C Tri  &b  ); // distance between triangle and a triangle

// distance between point and plane (from triangle)
inline Flt DistPointPlane(C Vec  &point, C Tri  &tri) {return DistPointPlane(point, tri.p[0], tri.n);}
inline Dbl DistPointPlane(C VecD &point, C TriD &tri) {return DistPointPlane(point, tri.p[0], tri.n);}

// if points cuts triangle assuming they're coplanar (epsilon=0)
Bool Cuts(C Vec2  &point, C Tri2  &tri);
Bool Cuts(C VecD2 &point, C TriD2 &tri);
Bool Cuts(C Vec   &point, C Tri   &tri);
Bool Cuts(C VecD  &point, C TriD  &tri);
#if EE_PRIVATE
Bool Cuts(C Vec &point, C Tri &tri, C Vec (&tri_cross)[3]);
#endif

// if points cuts triangle assuming they're coplanar (epsilon=EPS)
Bool CutsEps(C Vec2  &point, C Tri2  &tri);
Bool CutsEps(C VecD2 &point, C TriD2 &tri);
Bool CutsEps(C Vec   &point, C Tri   &tri);
Bool CutsEps(C VecD  &point, C TriD  &tri);

// if edge cuts triangle
Bool Cuts(C Edge &edge, C Tri &tri);

// if triangle cuts plane, return number of contacts (-1=co-planar, 0=zero, 1=one, 2=two), and set them as edge.p[]
Int CutsTriPlane   (C Tri  &tri, C Plane  &plane, Edge  &edge);
Int CutsTriPlane   (C TriD &tri, C PlaneD &plane, EdgeD &edge);
Int CutsTriPlaneEps(C Tri  &tri, C Plane  &plane, Edge  &edge);
Int CutsTriPlaneEps(C TriD &tri, C PlaneD &plane, EdgeD &edge);

// if moving point cuts static triangle (epsilon=0)
Bool SweepPointTri(C Vec &point, C Vec &move, C Tri &tri, Flt *hit_frac=null, Vec *hit_pos=null, Bool two_sided=false);

// if moving point cuts static triangle (epsilon=EPS)
Bool SweepPointTriEps(C Vec &point, C Vec &move, C Tri &tri, Flt *hit_frac=null, Vec *hit_pos=null, Bool two_sided=false);

// clip edge inside triangle
Int Clip(Edge2  &edge, C Tri2  &tri);
Int Clip(EdgeD2 &edge, C TriD2 &tri);
/******************************************************************************/
