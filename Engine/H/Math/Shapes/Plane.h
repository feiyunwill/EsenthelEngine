/******************************************************************************

   Use 'Plane2' to handle 2D planes, Flt type
   Use 'Plane'  to handle 3D planes, Flt type

   Use 'PlaneD2' to handle 2D planes, Dbl type
   Use 'PlaneD'  to handle 3D planes, Dbl type

/******************************************************************************/
struct Plane2 // Plane 2D
{
   Vec2 pos   , // plane position
        normal; // plane normal

   Plane2& set(C Vec2 &pos, C Vec2 &normal) {T.pos=pos; T.normal=normal; return T;}

   Plane2() {}
   Plane2(C Vec2 &pos, C Vec2 &normal) {set(pos, normal);}
};
/******************************************************************************/
struct PlaneD2 // Plane 2D (double precision)
{
   VecD2 pos   , // plane position
         normal; // plane normal

   PlaneD2& set(C VecD2 &pos, C VecD2 &normal) {T.pos=pos; T.normal=normal; return T;}

   PlaneD2() {}
   PlaneD2(C VecD2 &pos, C VecD2 &normal) {set(pos, normal);}
};
/******************************************************************************/
struct Plane // Plane 3D
{
   Vec pos   , // plane position
       normal; // plane normal

   Plane& set(C Vec &pos, C Vec &normal) {T.pos=pos; T.normal=normal; return T;}

   Plane& operator+=(C Vec &d) {T.pos+=d; return T;}
   Plane& operator-=(C Vec &d) {T.pos-=d; return T;}

   friend Plane operator+ (C Plane &plane, C Vec &d) {return Plane(plane)+=d;}
   friend Plane operator- (C Plane &plane, C Vec &d) {return Plane(plane)-=d;}

   // get
   Str asText()C {return S+"Pos: "+pos+", Normal: "+normal;} // get text description

   // draw
   void drawLocal               (C Color &color, Flt      size, Bool fill=false, Int resolution=-1)C; // this relies on active object matrix which can be set using 'SetMatrix' function
   void drawInfiniteBySize      (C Color &color, Flt cell_size                                    )C; // this relies on active object matrix which can be set using 'SetMatrix' function
   void drawInfiniteByResolution(C Color &color,                                 Int resolution=-1)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Plane() {}
   Plane(C Vec &pos,C Vec &normal) {set(pos, normal);}
};
/******************************************************************************/
struct PlaneM // Plane 3D (mixed precision)
{
   VecD pos   ; // plane position
   Vec  normal; // plane normal

   PlaneM& set(C VecD &pos, C Vec &normal) {T.pos=pos; T.normal=normal; return T;}

   PlaneM() {}
   PlaneM(C VecD &pos, C Vec &normal) {set(pos, normal);}
};
/******************************************************************************/
struct PlaneD // Plane 3D (double precision)
{
   VecD pos   , // plane position
        normal; // plane normal

   PlaneD& set(C VecD &pos, C VecD &normal) {T.pos=pos; T.normal=normal; return T;}

   PlaneD() {}
   PlaneD(C VecD &pos, C VecD &normal) {set(pos, normal);}
};
/******************************************************************************/
// distance between point and a plane ('plane_normal' must be normalized)
inline Flt DistPointPlane(C Vec2  &point,                     C Vec2  &plane_normal) {return Dot(point, plane_normal);} // this function assumes that plane position is Vec2 (0,0)
inline Dbl DistPointPlane(C VecD2 &point,                     C VecD2 &plane_normal) {return Dot(point, plane_normal);} // this function assumes that plane position is VecD2(0,0)
inline Flt DistPointPlane(C Vec   &point,                     C Vec   &plane_normal) {return Dot(point, plane_normal);} // this function assumes that plane position is Vec  (0,0,0)
inline Dbl DistPointPlane(C VecD  &point,                     C Vec   &plane_normal) {return Dot(point, plane_normal);} // this function assumes that plane position is VecD (0,0,0)
inline Dbl DistPointPlane(C VecD  &point,                     C VecD  &plane_normal) {return Dot(point, plane_normal);} // this function assumes that plane position is VecD (0,0,0)
       Flt DistPointPlane(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal);
       Dbl DistPointPlane(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal);
       Flt DistPointPlane(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal);
       Dbl DistPointPlane(C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal);
       Dbl DistPointPlane(C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal);
       Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal);
       Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal);

// distance between point and a plane using 'ray' movement directions ('plane_normal' and 'ray' must be normalized)
Flt DistPointPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray); // this function assumes that plane position is Vec2 (0,0)
Dbl DistPointPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray); // this function assumes that plane position is VecD2(0,0)
Flt DistPointPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray); // this function assumes that plane position is Vec  (0,0,0)
Dbl DistPointPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray); // this function assumes that plane position is VecD (0,0,0)
Dbl DistPointPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray); // this function assumes that plane position is VecD (0,0,0)
Flt DistPointPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray);
Dbl DistPointPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray);
Flt DistPointPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray);
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray);
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray);

// distance between point and a plane using Y axis movement vector ('plane_normal' must be normalized)
Flt DistPointPlaneY(C Vec  &point,                    C Vec  &plane_normal); // this function assumes that plane position is Vec (0,0,0)
Dbl DistPointPlaneY(C VecD &point,                    C Vec  &plane_normal); // this function assumes that plane position is VecD(0,0,0)
Dbl DistPointPlaneY(C VecD &point,                    C VecD &plane_normal); // this function assumes that plane position is VecD(0,0,0)
Flt DistPointPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal);
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal);
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal);

// distance between point and a plane
inline Flt Dist(C Vec2  &point, C Plane2  &plane) {return DistPointPlane(point, plane.pos, plane.normal);}
inline Dbl Dist(C VecD2 &point, C PlaneD2 &plane) {return DistPointPlane(point, plane.pos, plane.normal);}
inline Flt Dist(C Vec   &point, C Plane   &plane) {return DistPointPlane(point, plane.pos, plane.normal);}
inline Dbl Dist(C Vec   &point, C PlaneM  &plane) {return DistPointPlane(point, plane.pos, plane.normal);}
inline Dbl Dist(C VecD  &point, C PlaneM  &plane) {return DistPointPlane(point, plane.pos, plane.normal);}
inline Dbl Dist(C VecD  &point, C PlaneD  &plane) {return DistPointPlane(point, plane.pos, plane.normal);}

// if point cuts a plane
inline Bool Cuts(C Vec2  &point, C Plane2  &plane) {return Dist(point, plane)<=0;}
inline Bool Cuts(C VecD2 &point, C PlaneD2 &plane) {return Dist(point, plane)<=0;}
inline Bool Cuts(C Vec   &point, C Plane   &plane) {return Dist(point, plane)<=0;}
inline Bool Cuts(C VecD  &point, C PlaneD  &plane) {return Dist(point, plane)<=0;}

// return point casted on the plane along the 'plane_normal' ('plane_normal' must be normalized)
Vec2  PointOnPlane(C Vec2  &point,                     C Vec2  &plane_normal); // this function assumes that plane position is Vec2 (0,0)
VecD2 PointOnPlane(C VecD2 &point,                     C VecD2 &plane_normal); // this function assumes that plane position is VecD2(0,0)
Vec   PointOnPlane(C Vec   &point,                     C Vec   &plane_normal); // this function assumes that plane position is Vec  (0,0,0)
VecD  PointOnPlane(C VecD  &point,                     C VecD  &plane_normal); // this function assumes that plane position is VecD (0,0,0)
Vec2  PointOnPlane(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal);
VecD2 PointOnPlane(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal);
Vec   PointOnPlane(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal);
VecD  PointOnPlane(C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal);
VecD  PointOnPlane(C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal);
VecD  PointOnPlane(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal);
VecD  PointOnPlane(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal);

// return point casted on the plane along the 'ray' movement vector ('plane_normal' and 'ray' must be normalized)
Vec2  PointOnPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray); // this function assumes that plane position is Vec2 (0,0,0)
VecD2 PointOnPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray); // this function assumes that plane position is VecD2(0,0,0)
Vec   PointOnPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray); // this function assumes that plane position is Vec  (0,0,0)
VecD  PointOnPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray); // this function assumes that plane position is VecD (0,0,0)
VecD  PointOnPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray); // this function assumes that plane position is VecD (0,0,0)
Vec2  PointOnPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray);
VecD2 PointOnPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray);
Vec   PointOnPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray);
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray);
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray);

// return point casted on the plane along the Y axis movement vector ('plane_normal' must be normalized)
Vec  PointOnPlaneY(C Vec  &point,                    C Vec  &plane_normal); // this function assumes that plane position is Vec (0,0,0)
VecD PointOnPlaneY(C VecD &point,                    C Vec  &plane_normal); // this function assumes that plane position is VecD(0,0,0)
VecD PointOnPlaneY(C VecD &point,                    C VecD &plane_normal); // this function assumes that plane position is VecD(0,0,0)
Vec  PointOnPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal);
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal);
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal);

// return step (0..1) used for interpolating between 'a' and 'b' points "Lerp(a, b, step)" in order to obtain a point casted on a plane, 'dist_a' and 'dist_b' are the point distances from the plane
inline Flt PointOnPlaneStep(Flt dist_a, Flt dist_b) {return dist_a/(dist_a-dist_b);}
inline Dbl PointOnPlaneStep(Dbl dist_a, Dbl dist_b) {return dist_a/(dist_a-dist_b);}

// return point casted on a plane, by known 2 points 'a' and 'b', and their distance from plane 'dist_a' and 'dist_b' (returned point and 'a' 'b' points will always be on the same straight line)
Flt   PointOnPlane(  Flt    a,   Flt    b, Flt dist_a, Flt dist_b);
Dbl   PointOnPlane(  Dbl    a,   Dbl    b, Dbl dist_a, Dbl dist_b);
Vec2  PointOnPlane(C Vec2  &a, C Vec2  &b, Flt dist_a, Flt dist_b);
VecD2 PointOnPlane(C VecD2 &a, C VecD2 &b, Dbl dist_a, Dbl dist_b);
Vec   PointOnPlane(C Vec   &a, C Vec   &b, Flt dist_a, Flt dist_b);
VecD  PointOnPlane(C VecD  &a, C VecD  &b, Dbl dist_a, Dbl dist_b);

// if moving point cuts through static plane
Bool SweepPointPlane(C Vec2  &point, C Vec2  &move, C Plane2  &plane, Flt *hit_frac=null, Vec2  *hit_normal=null, Vec2  *hit_pos=null, Bool two_sided=false);
Bool SweepPointPlane(C VecD2 &point, C VecD2 &move, C PlaneD2 &plane, Dbl *hit_frac=null, VecD2 *hit_normal=null, VecD2 *hit_pos=null, Bool two_sided=false);
Bool SweepPointPlane(C Vec   &point, C Vec   &move, C Plane   &plane, Flt *hit_frac=null, Vec   *hit_normal=null, Vec   *hit_pos=null, Bool two_sided=false);
Bool SweepPointPlane(C VecD  &point, C VecD  &move, C PlaneD  &plane, Dbl *hit_frac=null, VecD  *hit_normal=null, VecD  *hit_pos=null, Bool two_sided=false);

// if moving plane cuts through static point
inline Bool SweepPlanePoint(C Plane2  &plane, C Vec2  &move, C Vec2  &point, Flt *hit_frac=null) {return SweepPointPlane(point, -move, plane, hit_frac);}
inline Bool SweepPlanePoint(C PlaneD2 &plane, C VecD2 &move, C VecD2 &point, Dbl *hit_frac=null) {return SweepPointPlane(point, -move, plane, hit_frac);}
inline Bool SweepPlanePoint(C Plane   &plane, C Vec   &move, C Vec   &point, Flt *hit_frac=null) {return SweepPointPlane(point, -move, plane, hit_frac);}
inline Bool SweepPlanePoint(C PlaneD  &plane, C VecD  &move, C VecD  &point, Dbl *hit_frac=null) {return SweepPointPlane(point, -move, plane, hit_frac);}

// slide 'move' movement by colliding normals, this function adjusts desired 'move' vector according to colliding planes with their normals, so that 'move' vector will slide along the planes if needed, and not move through them
void SlideMovement(Vec2 &move, C Vec2 *normal, Int normals);
void SlideMovement(Vec  &move, C Vec  *normal, Int normals);
/******************************************************************************/
