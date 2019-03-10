/******************************************************************************

   Use 'Cone' to handle cone shapes.

/******************************************************************************/
struct Cone // Cone Shape
{
   Flt  r_low , // lower  radius
        r_high; // higher radius
   Flt  h     ; // height
   Vec  pos   , // bottom position
        up    ; // up     direction

   // set
   Cone& set(Flt r_low, Flt r_high, Flt h, C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {T.r_low=r_low; T.r_high=r_high; T.h=h; T.pos=pos; T.up=up; return T;}

   // get
   Flt area  ()C; // get surface area
   Flt volume()C; // get volume

   Str asText()C {return S+"LowerRadius: "+r_low+", HigherRadius: "+r_high+", Height: "+h+", Pos: "+pos+", Up: "+up;} // get text description

   // transform
   Cone& operator+=(C Vec     &v) {pos+=v; return T;}
   Cone& operator-=(C Vec     &v) {pos-=v; return T;}
   Cone& operator*=(  Flt      f);
   Cone& operator/=(  Flt      f);
   Cone& operator*=(C Matrix3 &m);
   Cone& operator*=(C Matrix  &m);

   friend Cone operator+ (C Cone &cone, C Vec     &v){return Cone(cone)+=v;}
   friend Cone operator- (C Cone &cone, C Vec     &v){return Cone(cone)-=v;}
   friend Cone operator* (C Cone &cone,   Flt      f){return Cone(cone)*=f;}
   friend Cone operator/ (C Cone &cone,   Flt      f){return Cone(cone)/=f;}
   friend Cone operator* (C Cone &cone, C Matrix3 &m){return Cone(cone)*=m;}
   friend Cone operator* (C Cone &cone, C Matrix  &m){return Cone(cone)*=m;}

   // draw
   void draw(C Color &color=WHITE, Bool fill=false, Int resolution=-1)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Cone() {}
   Cone(Flt r_low, Flt r_high, Flt h, C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {set(r_low, r_high, h, pos, up);}
};
/******************************************************************************/
Bool Cuts(C Vec &point, C Cone &cone); // if point cuts cone
/******************************************************************************/
