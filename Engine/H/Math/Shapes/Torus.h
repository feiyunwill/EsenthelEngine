/******************************************************************************

   Use 'Torus' to handle torus shapes.

/******************************************************************************/
struct Torus // Torus Shape
{
   Flt R  , // main radius
       r  ; // tube radius
   Vec pos, // center position
       up ; // up     direction

   Torus& set(Flt R, Flt r, C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {T.R=R; T.r=r; T.pos=pos; T.up=up; return T;}

   // transform
   Torus& operator+=(C Vec     &v) {pos+=v; return T;}
   Torus& operator-=(C Vec     &v) {pos-=v; return T;}
   Torus& operator*=(  Flt      f);
   Torus& operator/=(  Flt      f);
   Torus& operator*=(C Matrix3 &m);
   Torus& operator*=(C Matrix  &m);

   friend Torus operator+ (C Torus &torus, C Vec     &v) {return Torus(torus)+=v;}
   friend Torus operator- (C Torus &torus, C Vec     &v) {return Torus(torus)-=v;}
   friend Torus operator* (C Torus &torus,   Flt      f) {return Torus(torus)*=f;}
   friend Torus operator/ (C Torus &torus,   Flt      f) {return Torus(torus)/=f;}
   friend Torus operator* (C Torus &torus, C Matrix3 &m) {return Torus(torus)*=m;}
   friend Torus operator* (C Torus &torus, C Matrix  &m) {return Torus(torus)*=m;}

   // get
   Flt area  ()C {return (4*PI*PI)*R*r  ;} // get surface area
   Flt volume()C {return (2*PI*PI)*R*r*r;} // get volume

   Str asText()C {return S+"MainRadius: "+R+", TubeRadius: "+r+", Pos: "+pos+", Up: "+up;} // get text description

   // operations
   Torus& extend(Flt e) {r+=e; return T;} // extend

   // draw
   void draw(C Color &color=WHITE, Bool fill=false, VecI2 resolution=VecI2(-1))C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Torus() {}
   Torus(Flt R, Flt r, C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {set(R, r, pos, up);}
};
/******************************************************************************/
// distance
Flt Dist(C Vec &point, C Torus &torus); // distance between point and a torus
/******************************************************************************/
