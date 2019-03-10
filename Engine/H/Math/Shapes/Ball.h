/******************************************************************************

   Use 'Ball'  to handle ball shapes, Flt type
   Use 'BallD' to handle ball shapes, Dbl type

/******************************************************************************/
struct Ball // Ball Shape
{
   Flt r  ; // radius
   Vec pos; // center position

   Ball& zero(                         ) {T.r=0; T.pos.zero(); return T;}
   Ball& set (Flt r, C Vec &pos=VecZero) {T.r=r; T.pos=pos   ; return T;}

#if EE_PRIVATE
   Ball& setAnimated(C Extent &ext, C Matrix           &matrix   ); // set ball from 'ext' animated by 'matrix'
#endif
   Ball& setAnimated(C Extent &ext, C AnimatedSkeleton &anim_skel); // set approximate ball encapsulating 'ext' animated by 'anim_skel' skeleton

   Ball& operator+=(C Vec     &v) {pos+=v; return T;}
   Ball& operator-=(C Vec     &v) {pos-=v; return T;}
   Ball& operator*=(  Flt      f);
   Ball& operator/=(  Flt      f);
   Ball& operator*=(C Matrix3 &m);
   Ball& operator/=(C Matrix3 &m);
   Ball& operator*=(C Matrix  &m);
   Ball& operator/=(C Matrix  &m);

   friend Ball operator+ (C Ball &ball, C Vec     &v) {return Ball(ball)+=v;}
   friend Ball operator- (C Ball &ball, C Vec     &v) {return Ball(ball)-=v;}
   friend Ball operator* (C Ball &ball,   Flt      f) {return Ball(ball)*=f;}
   friend Ball operator/ (C Ball &ball,   Flt      f) {return Ball(ball)/=f;}
   friend Ball operator* (C Ball &ball, C Matrix3 &m) {return Ball(ball)*=m;}
   friend Ball operator/ (C Ball &ball, C Matrix3 &m) {return Ball(ball)/=m;}
   friend Ball operator* (C Ball &ball, C Matrix  &m) {return Ball(ball)*=m;}
   friend Ball operator/ (C Ball &ball, C Matrix  &m) {return Ball(ball)/=m;}

   // get
   Flt area  ()C {return (PI*4  )*r*r  ;} // get surface area
   Flt volume()C {return (PI*4/3)*r*r*r;} // get volume

   Vec nearest(C Vec &normal)C; // get nearest point on ball towards normal

   Str asText()C {return S+"Radius: "+r+", Pos: "+pos;} // get text description

   // operations
   Ball& extend(Flt e) {r+=e; return T;} // extend
   Bool  from  (C Vec *point, Int points                  ); // set ball from an array of points                        , false on fail (if there are no points)
   Bool  from  (C Vec *point, Int points, C Matrix &matrix); // set ball from an array of points transformed by 'matrix', false on fail (if there are no points)

   // draw
#if EE_PRIVATE
   void drawVI   (                                                   Bool fill=false, C VecI2 &resolution=VecI2(-1))C; // draw, this relies on active object matrix which can be set using 'SetMatrix' function
#endif
   void draw     (C Color &color=WHITE,                              Bool fill=false, C VecI2 &resolution=VecI2(-1))C; // draw                  , this relies on active object matrix which can be set using 'SetMatrix' function
   void drawAngle(C Color &color      , Flt from, Flt to, C Vec &up, Bool fill=false, C VecI2 &resolution=VecI2(-1))C; // draw with angle ranges, this relies on active object matrix which can be set using 'SetMatrix' function
   void draw2    (C Color &color=WHITE,                              Bool fill=false,   Int    resolution=      -1 )C; // draw box based        , this relies on active object matrix which can be set using 'SetMatrix' function

   Ball() {}
   Ball(Flt r, C Vec &pos=VecZero) {set(r, pos);}
   Ball(C Box      &box    );
   Ball(C OBox     &obox   );
   Ball(C Extent   &ext    );
   Ball(C Capsule  &capsule);
   Ball(C Shape    &shape  );
   Ball(C MeshBase &mesh   );
};
/******************************************************************************/
struct BallM // Ball Shape (mixed precision)
{
   Flt  r  ; // radius
   VecD pos; // center position

   BallM& set(Flt r, C VecD &pos) {T.r=r; T.pos=pos; return T;}

   BallM& extend(Flt e) {r+=e; return T;} // extend

   BallM() {}
   BallM(Flt r, C VecD &pos=0) {set(r, pos);}
};
/******************************************************************************/
struct BallD // Ball Shape (double precision)
{
   Dbl  r  ; // radius
   VecD pos; // center position

   BallD& set(Dbl r, C VecD &pos) {T.r=r; T.pos=pos; return T;}

   BallD& extend(Dbl e) {r+=e; return T;} // extend

   BallD() {}
   BallD(Dbl r, C VecD &pos=0) {set(r, pos);}
};
/******************************************************************************/
Ball Avg(C Ball &a, C Ball &b);

// distance
       Flt Dist         (C Vec   &point, C Ball   &ball                ); // distance between point    and a ball
       Flt Dist         (C Edge  &edge , C Ball   &ball                ); // distance between edge     and a ball
       Flt Dist         (C Tri   &tri  , C Ball   &ball                ); // distance between triangle and a ball
       Flt Dist         (C Box   &box  , C Ball   &ball                ); // distance between box      and a ball
       Flt Dist         (C OBox  &obox , C Ball   &ball                ); // distance between box      and a ball
       Flt Dist         (C Ball  &a    , C Ball   &b                   ); // distance between ball     and a ball
       Flt DistBallPlane(C Ball  &ball , C Vec    &plane, C Vec &normal); // distance between ball     and a plane
       Dbl DistBallPlane(C Ball  &ball , C VecD   &plane, C Vec &normal); // distance between ball     and a plane
       Dbl DistBallPlane(C BallM &ball , C VecD   &plane, C Vec &normal); // distance between ball     and a plane
inline Flt Dist         (C Ball  &ball , C Plane  &plane               ) {return DistBallPlane(ball, plane.pos, plane.normal);} // distance between ball and a plane
inline Dbl Dist         (C Ball  &ball , C PlaneM &plane               ) {return DistBallPlane(ball, plane.pos, plane.normal);} // distance between ball and a plane
inline Dbl Dist         (C BallM &ball , C PlaneM &plane               ) {return DistBallPlane(ball, plane.pos, plane.normal);} // distance between ball and a plane

// cuts
Bool Cuts(C Vec    &point, C Ball  &ball); // if point    cuts a ball
Bool Cuts(C VecD   &point, C Ball  &ball); // if point    cuts a ball
Bool Cuts(C VecD   &point, C BallM &ball); // if point    cuts a ball
Bool Cuts(C Edge   &edge , C Ball  &ball); // if edge     cuts a ball
Bool Cuts(C Tri    &tri  , C Ball  &ball); // if triangle cuts a ball
Bool Cuts(C Box    &box  , C Ball  &ball); // if box      cuts a ball
Bool Cuts(C OBox   &obox , C Ball  &ball); // if box      cuts a ball
Bool Cuts(C Extent &ext  , C Ball  &ball); // if extent   cuts a ball
Bool Cuts(C Ball   &a    , C Ball  &b   ); // if ball     cuts a ball

Int CutsStrBall (C Vec & str_pos  , C Vec & str_dir, C Ball &ball, Vec *contact_a=null, Vec *contact_b=null); // if straight infinite line cuts a ball, return number of contacts, 'str_dir'=straight line direction (must be normalized)
Int CutsEdgeBall(C Vec &edge_start, C Vec &edge_end, C Ball &ball, Vec *contact_a=null, Vec *contact_b=null); // if edge                   cuts a ball, return number of contacts

Bool Inside(C Box    &a, C Ball &b); // if 'a' is fully inside 'b'
Bool Inside(C Extent &a, C Ball &b); // if 'a' is fully inside 'b'

// sweep
Bool SweepPointBall(C Vec  &point, C Vec  &move, C Ball  &ball , Flt *hit_frac=null, Vec  *hit_normal=null); // if moving point cuts through a static ball
Bool SweepPointBall(C VecD &point, C VecD &move, C BallD &ball , Dbl *hit_frac=null, VecD *hit_normal=null); // if moving point cuts through a static ball

Bool SweepBallPoint(C Ball  &ball, C Vec  &move, C Vec   &point, Flt *hit_frac=null, Vec  *hit_normal=null); // if moving ball cuts through a static point
Bool SweepBallPoint(C BallD &ball, C VecD &move, C VecD  &point, Dbl *hit_frac=null, VecD *hit_normal=null); // if moving ball cuts through a static point
Bool SweepBallEdge (C Ball  &ball, C Vec  &move, C Edge  &edge , Flt *hit_frac=null, Vec  *hit_normal=null); // if moving ball cuts through a static edge
Bool SweepBallEdge (C BallD &ball, C VecD &move, C EdgeD &edge , Dbl *hit_frac=null, VecD *hit_normal=null); // if moving ball cuts through a static edge
Bool SweepBallBall (C Ball  &ball, C Vec  &move, C Ball  &ball2, Flt *hit_frac=null, Vec  *hit_normal=null); // if moving ball cuts through a static ball
Bool SweepBallBall (C BallD &ball, C VecD &move, C BallD &ball2, Dbl *hit_frac=null, VecD *hit_normal=null); // if moving ball cuts through a static ball
/******************************************************************************/
