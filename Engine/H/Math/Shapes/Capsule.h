/******************************************************************************

   Use 'Capsule' to handle capsule shapes.

/******************************************************************************/
struct Capsule // Capsule Shape
{
   Flt r  , // radius
       h  ; // total  height
   Vec pos, // center position
       up ; // up     direction

   // set
   Capsule& set    (Flt r,   Flt  h   , C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {T.r=r; T.h=h; T.pos=pos; T.up=up; return T;}
   Capsule& set    (Flt r, C Vec &from, C Vec &to); // 'from' and 'to' are the bottom and top points of capsule
   Capsule& setEdge(Flt r, C Vec &from, C Vec &to); // 'from' and 'to' are the bottom and top points of capsule inner edge

   // get
   Flt area  ()C {return PI2*r*   h            ;} // get surface area
   Flt volume()C {return PI *r*r*(h-r*(2.0f/3));} // get volume

   Vec pointD()C {return pos-up*(h*0.5f-r);       } // get lower ball center point
   Vec pointU()C {return pos+up*(h*0.5f-r);       } // get upper ball center point
   Ball ballD()C {return Ball(r, pointD());       } // get lower ball
   Ball ballU()C {return Ball(r, pointU());       } // get upper ball
   Edge edge ()C {return Edge(pointD(), pointU());} // get edge between lower and upper ball center points

   Vec nearest(C Vec &normal)C; // get nearest point on capsule towards normal

   Bool isBall()C {return h<=r*2+EPS;} // if this capsule is actually a ball (total height is smaller than 2*radius)

   Str asText()C {return S+"Radius: "+r+", Height: "+h+", Pos: "+pos+", Up: "+up;} // get text description

   // transform
   Capsule& operator+=(C Vec     &v) {pos+=v; return T;}
   Capsule& operator-=(C Vec     &v) {pos-=v; return T;}
   Capsule& operator*=(  Flt      f);
   Capsule& operator/=(  Flt      f);
   Capsule& operator*=(C Vec     &v);
   Capsule& operator/=(C Vec     &v);
   Capsule& operator*=(C Matrix3 &m);
   Capsule& operator/=(C Matrix3 &m);
   Capsule& operator*=(C Matrix  &m);
   Capsule& operator/=(C Matrix  &m);

   friend Capsule operator+ (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)+=v;}
   friend Capsule operator- (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)-=v;}
   friend Capsule operator* (C Capsule &capsule,   Flt      f) {return Capsule(capsule)*=f;}
   friend Capsule operator/ (C Capsule &capsule,   Flt      f) {return Capsule(capsule)/=f;}
   friend Capsule operator* (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)*=v;}
   friend Capsule operator/ (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)/=v;}
   friend Capsule operator* (C Capsule &capsule, C Matrix3 &m) {return Capsule(capsule)*=m;}
   friend Capsule operator/ (C Capsule &capsule, C Matrix3 &m) {return Capsule(capsule)/=m;}
   friend Capsule operator* (C Capsule &capsule, C Matrix  &m) {return Capsule(capsule)*=m;}
   friend Capsule operator/ (C Capsule &capsule, C Matrix  &m) {return Capsule(capsule)/=m;}

   // operations
   Capsule& extend(Flt e) {r+=e; h+=e+e; return T;} // extend

   // draw
   void draw(C Color &color=WHITE, Bool fill=false, Int resolution=-1)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Capsule() {}
   Capsule(Flt r,   Flt  h   , C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {set(r, h, pos , up);}
   Capsule(Flt r, C Vec &from, C Vec &to                               ) {set(r,    from, to);}
};
/******************************************************************************/
struct CapsuleM // Capsule Shape (mixed precision)
{
   Flt  r  , // radius
        h  ; // total  height
   VecD pos; // center position
   Vec  up ; // up     direction

   // set
   CapsuleM& set(Flt r, Flt h, C VecD &pos=0, C Vec &up=Vec(0,1,0)) {T.r=r; T.h=h; T.pos=pos; T.up=up; return T;}

   // get
   Flt area  ()C {return PI2*r*   h            ;} // get surface area
   Flt volume()C {return PI *r*r*(h-r*(2.0f/3));} // get volume

   VecD pointD()C {return pos-up*(h*0.5f-r);        } // get lower ball center point
   VecD pointU()C {return pos+up*(h*0.5f-r);        } // get upper ball center point
   BallM ballD()C {return BallM(r, pointD());       } // get lower ball
   BallM ballU()C {return BallM(r, pointU());       } // get upper ball
   EdgeD edge ()C {return EdgeD(pointD(), pointU());} // get edge between lower and upper ball center points

   VecD nearest(C Vec &normal)C; // get nearest point on capsule towards normal

   Bool isBall()C {return h<=r*2+EPS;} // if this capsule is actually a ball (total height is smaller than 2*radius)

   Str asText()C {return S+"Radius: "+r+", Height: "+h+", Pos: "+pos+", Up: "+up;} // get text description

   // transform
   CapsuleM& operator+=(C VecD    &v) {pos+=v; return T;}
   CapsuleM& operator-=(C VecD    &v) {pos-=v; return T;}
   CapsuleM& operator*=(  Flt      f);
   CapsuleM& operator/=(  Flt      f);
   CapsuleM& operator*=(C Vec     &v);
   CapsuleM& operator/=(C Vec     &v);

   friend CapsuleM operator+ (C CapsuleM &capsule, C VecD    &v) {return CapsuleM(capsule)+=v;}
   friend CapsuleM operator- (C CapsuleM &capsule, C VecD    &v) {return CapsuleM(capsule)-=v;}
   friend CapsuleM operator* (C CapsuleM &capsule,   Flt      f) {return CapsuleM(capsule)*=f;}
   friend CapsuleM operator/ (C CapsuleM &capsule,   Flt      f) {return CapsuleM(capsule)/=f;}
   friend CapsuleM operator* (C CapsuleM &capsule, C Vec     &v) {return CapsuleM(capsule)*=v;}
   friend CapsuleM operator/ (C CapsuleM &capsule, C Vec     &v) {return CapsuleM(capsule)/=v;}

   CapsuleM() {}
   CapsuleM(Flt r, Flt h, C VecD &pos=0, C Vec &up=Vec(0,1,0)) {set(r, h, pos , up);}
};
/******************************************************************************/
// distance
       Flt Dist            (C Vec      &point  , C Capsule &capsule             ); // distance between point    and a capsule
       Flt Dist            (C Edge     &edge   , C Capsule &capsule             ); // distance between edge     and a capsule
       Flt Dist            (C Tri      &tri    , C Capsule &capsule             ); // distance between triangle and a capsule
       Flt Dist            (C Box      &box    , C Capsule &capsule             ); // distance between box      and a capsule
       Flt Dist            (C OBox     &obox   , C Capsule &capsule             ); // distance between box      and a capsule
       Flt Dist            (C Ball     &ball   , C Capsule &capsule             ); // distance between ball     and a capsule
       Flt Dist            (C Capsule  &a      , C Capsule &b                   ); // distance between capsule  and a capsule
       Flt DistCapsulePlane(C Capsule  &capsule, C Vec     &plane, C Vec &normal); // distance between capsule  and a plane
       Dbl DistCapsulePlane(C Capsule  &capsule, C VecD    &plane, C Vec &normal); // distance between capsule  and a plane
       Dbl DistCapsulePlane(C CapsuleM &capsule, C VecD    &plane, C Vec &normal); // distance between capsule  and a plane
inline Flt Dist            (C Capsule  &capsule, C Plane   &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane
inline Dbl Dist            (C Capsule  &capsule, C PlaneM  &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane
inline Dbl Dist            (C CapsuleM &capsule, C PlaneM  &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane

// cuts
Bool Cuts(C Vec     &point, C Capsule  &capsule); // if point    cuts a capsule
Bool Cuts(C VecD    &point, C Capsule  &capsule); // if point    cuts a capsule
Bool Cuts(C VecD    &point, C CapsuleM &capsule); // if point    cuts a capsule
Bool Cuts(C Edge    &edge , C Capsule  &capsule); // if edge     cuts a capsule
Bool Cuts(C Tri     &tri  , C Capsule  &capsule); // if triangle cuts a capsule
Bool Cuts(C Box     &box  , C Capsule  &capsule); // if box      cuts a capsule
Bool Cuts(C OBox    &obox , C Capsule  &capsule); // if box      cuts a capsule
Bool Cuts(C Ball    &ball , C Capsule  &capsule); // if ball     cuts a capsule
Bool Cuts(C Capsule &a    , C Capsule  &b      ); // if capsule  cuts a capsule

// sweep
Bool SweepPointCapsule(C Vec  &point, C Vec &move, C Capsule &capsule, Flt *hit_frac=null, Vec *hit_normal=null); // if moving point cuts through a static capsule
Bool SweepBallCapsule (C Ball &ball , C Vec &move, C Capsule &capsule, Flt *hit_frac=null, Vec *hit_normal=null); // if moving ball  cuts through a static capsule

Bool SweepCapsuleEdge (C Capsule &capsule, C Vec &move, C Edge  &edge , Flt *hit_frac=null, Vec *hit_normal=null                   ); // if moving capsule cuts through a static edge
Bool SweepCapsulePlane(C Capsule &capsule, C Vec &move, C Plane &plane, Flt *hit_frac=null, Vec *hit_normal=null, Vec *hit_pos=null); // if moving capsule cuts through a static plane
Bool SweepCapsuleTri  (C Capsule &capsule, C Vec &move, C Tri   &tri  , Flt *hit_frac=null, Vec *hit_normal=null                   ); // if moving capsule cuts through a static triangle
/******************************************************************************/
