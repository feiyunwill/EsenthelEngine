/******************************************************************************

   Use 'Quad2' to handle 2D quads, Flt type
   Use 'Quad'  to handle 3D quads, Flt type

   Use 'QuadD2' to handle 2D quads, Dbl type
   Use 'QuadD'  to handle 3D quads, Dbl type

/******************************************************************************/
struct Quad2 // Quadrilateral 2D
{
   Vec2 p[4]; // points

   Quad2& set(C Vec2 &p0, C Vec2 &p1, C Vec2 &p2, C Vec2 &p3) {p[0]=p0; p[1]=p1; p[2]=p2; p[3]=p3; return T;}

   // get
   Vec2 center   (           )C {return Avg(p[0], p[1], p[2], p[3]);} // get quad center
   Flt  area     (           )C; // get surface area
   Bool convex   (           )C; // if quad is convex
   Bool valid    (Flt eps=EPS)C; // if quad is valid (points aren't inline)
   Bool clockwise(           )C; // if points are in clockwise order

   Tri2 tri013()C {return Tri2(p[0], p[1], p[3]);}
   Tri2 tri123()C {return Tri2(p[1], p[2], p[3]);}

   Tri2 tri012()C {return Tri2(p[0], p[1], p[2]);}
   Tri2 tri230()C {return Tri2(p[2], p[3], p[0]);}

   // draw
   void draw(C Color &color=WHITE, Bool fill=true)C;

   Quad2& operator+=(C Vec2 &v);
   Quad2& operator-=(C Vec2 &v);
   Quad2& operator*=(  Flt   r);
   Quad2& operator/=(  Flt   r);

   friend Quad2 operator+ (C Quad2 &quad, C Vec2 &v) {return Quad2(quad)+=v;}
   friend Quad2 operator- (C Quad2 &quad, C Vec2 &v) {return Quad2(quad)-=v;}
   friend Quad2 operator* (C Quad2 &quad,   Flt   r) {return Quad2(quad)*=r;}
   friend Quad2 operator/ (C Quad2 &quad,   Flt   r) {return Quad2(quad)/=r;}

   Quad2() {}
   Quad2(C Vec2 &p0, C Vec2 &p1, C Vec2 &p2, C Vec2 &p3) {set(p0, p1, p2, p3);}
};
/******************************************************************************/
struct QuadD2 // Quadrilateral 2D (double precision)
{
   VecD2 p[4]; // points

   QuadD2& set(C VecD2 &p0, C VecD2 &p1, C VecD2 &p2, C VecD2 &p3) {p[0]=p0; p[1]=p1; p[2]=p2; p[3]=p3; return T;}

   // get
   VecD2 center   ()C {return Avg(p[0], p[1], p[2], p[3]);} // get quad center
   Dbl   area     ()C; // get triangle surface area
   Bool  clockwise()C; // if points are in clockwise order

   TriD2 tri013()C {return TriD2(p[0], p[1], p[3]);}
   TriD2 tri123()C {return TriD2(p[1], p[2], p[3]);}

   TriD2 tri012()C {return TriD2(p[0], p[1], p[2]);}
   TriD2 tri230()C {return TriD2(p[2], p[3], p[0]);}

   QuadD2& operator+=(C VecD2 &v);
   QuadD2& operator-=(C VecD2 &v);
   QuadD2& operator*=(  Dbl    r);
   QuadD2& operator/=(  Dbl    r);

   friend QuadD2 operator+ (C QuadD2 &quad, C VecD2 &v) {return QuadD2(quad)+=v;}
   friend QuadD2 operator- (C QuadD2 &quad, C VecD2 &v) {return QuadD2(quad)-=v;}
   friend QuadD2 operator* (C QuadD2 &quad,   Dbl    r) {return QuadD2(quad)*=r;}
   friend QuadD2 operator/ (C QuadD2 &quad,   Dbl    r) {return QuadD2(quad)/=r;}

   QuadD2() {}
   QuadD2(C VecD2 &p0, C VecD2 &p1, C VecD2 &p2, C VecD2 &p3) {set(p0, p1, p2, p3);}
};
/******************************************************************************/
struct Quad // Quadrilateral 3D
{
   Vec p[4], // points
       n   ; // normal

   Quad& set      (C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3, C Vec *normal=null);
   Quad& setNormal(                                                              ) {n=GetNormal(p[0], p[1], p[3]); return T;} // recalculate normal

   // get
   Vec  center  (            )C {return Avg(p[0], p[1], p[2], p[3]);} // get quad center
   Flt  area    (            )C; // get quad surface area
   Bool convex  (            )C; // if quad is convex
   Bool valid   (Flt eps=EPS )C; // if quad is valid (points aren't inline)
   Bool coplanar(C Quad &quad)C; // if quads are coplanar

   Tri tri013()C {return Tri(p[0], p[1], p[3]);}
   Tri tri123()C {return Tri(p[1], p[2], p[3]);}

   Tri tri012()C {return Tri(p[0], p[1], p[2]);}
   Tri tri230()C {return Tri(p[2], p[3], p[0]);}

   // draw
   void draw(C Color &color=WHITE, Bool fill=false)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Quad& operator+=(C Vec &v);
   Quad& operator-=(C Vec &v);
   Quad& operator*=(  Flt  r);
   Quad& operator/=(  Flt  r);

   friend Quad operator+ (C Quad &quad, C Vec &v) {return Quad(quad)+=v;}
   friend Quad operator- (C Quad &quad, C Vec &v) {return Quad(quad)-=v;}
   friend Quad operator* (C Quad &quad,   Flt  r) {return Quad(quad)*=r;}
   friend Quad operator/ (C Quad &quad,   Flt  r) {return Quad(quad)/=r;}

   Quad() {}
   Quad(C Vec   &p0, C Vec &p1, C Vec &p2, C Vec &p3, C Vec *normal=null) {set(p0, p1, p2, p3, normal);}
   Quad(C QuadD &quad);
};
/******************************************************************************/
struct QuadD // Quadrilateral 3D (double precision)
{
   VecD p[4], // points
        n   ; // normal

   QuadD& set      (C VecD &p0, C VecD &p1, C VecD &p2, C VecD &p3, C VecD *normal=null);
   QuadD& setNormal(                                                                   ) {n=GetNormal(p[0], p[1], p[3]); return T;} // recalculate normal

   // get
   VecD center()C {return Avg(p[0], p[1], p[2], p[3]);} // get quad center
   Dbl  area  ()C; // get quad surface area

   TriD tri013()C {return TriD(p[0], p[1], p[3]);}
   TriD tri123()C {return TriD(p[1], p[2], p[3]);}

   TriD tri012()C {return TriD(p[0], p[1], p[2]);}
   TriD tri230()C {return TriD(p[2], p[3], p[0]);}

   QuadD& operator+=(C VecD &v);
   QuadD& operator-=(C VecD &v);
   QuadD& operator*=(  Dbl   r);
   QuadD& operator/=(  Dbl   r);

   friend QuadD operator+ (C QuadD &quad, C VecD &v) {return QuadD(quad)+=v;}
   friend QuadD operator- (C QuadD &quad, C VecD &v) {return QuadD(quad)-=v;}
   friend QuadD operator* (C QuadD &quad,   Dbl   r) {return QuadD(quad)*=r;}
   friend QuadD operator/ (C QuadD &quad,   Dbl   r) {return QuadD(quad)/=r;}

   QuadD() {}
   QuadD(C VecD &p0, C VecD &p1, C VecD &p2, C VecD &p3, C VecD *normal=null) {set(p0, p1, p2, p3, normal);}
   QuadD(C Quad &quad);
};
/******************************************************************************/
// distance between point and a quad, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Flt Dist(C Vec2 &point, C Quad2 &quad, DIST_TYPE *type=null);
Flt Dist(C Vec  &point, C Quad  &quad, DIST_TYPE *type=null, Bool test_quads_as_2_tris=true);

// distance between point and a plane (from quad)
inline Flt DistPointPlane(C Vec  &point, C Quad  &quad) {return DistPointPlane(point, quad.p[0], quad.n);}
inline Dbl DistPointPlane(C VecD &point, C QuadD &quad) {return DistPointPlane(point, quad.p[0], quad.n);}

// if point cuts quad assuming they're coplanar (epsilon=0), if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool Cuts(C Vec2 &point, C Quad2 &quad                                );
Bool Cuts(C Vec  &point, C Quad  &quad, Bool test_quads_as_2_tris=true);

// if point cuts quad assuming they're coplanar (epsilon=EPS), if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool CutsEps(C Vec2 &point, C Quad2 &quad                                );
Bool CutsEps(C Vec  &point, C Quad  &quad, Bool test_quads_as_2_tris=true);

// if moving point cuts through static quad (epsilon=0)
Bool SweepPointQuad(C Vec &point, C Vec &move, C Quad &quad, Flt *hit_frac=null, Vec *hit_pos=null, Bool test_quads_as_2_tris=true, Bool two_sided=false); // if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost

// if moving point cuts through static quad (epsilon=EPS)
Bool SweepPointQuadEps(C Vec &point, C Vec &move, C Quad &quad, Flt *hit_frac=null, Vec *hit_pos=null, Bool test_quads_as_2_tris=true, Bool two_sided=false); // if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
/******************************************************************************/
