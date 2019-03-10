/******************************************************************************

   Use 'Pyramid' to handle pyramid shapes.

/******************************************************************************/
STRUCT(Pyramid , OrientP) // Pyramid Shape
//{
   Flt scale, // proportional scale of side to height (0..Inf), default=1
       h    ; // height

   // set
   Pyramid& set(Flt scale, Flt h, C Vec &pos=VecZero, C Vec &dir=Vec(0,1,0)) {T.scale=scale; T.h=h; setPosDir(pos, dir); return T;} // 'dir' must be normalized

   // transform
   Pyramid& operator*=(  Flt      f);
   Pyramid& operator/=(  Flt      f);
   Pyramid& operator*=(C Matrix3 &m);
   Pyramid& operator*=(C Matrix  &m);

   friend Pyramid operator* (C Pyramid &pyramid,   Flt      f) {return Pyramid(pyramid)*=f;}
   friend Pyramid operator/ (C Pyramid &pyramid,   Flt      f) {return Pyramid(pyramid)/=f;}
   friend Pyramid operator* (C Pyramid &pyramid, C Matrix3 &m) {return Pyramid(pyramid)*=m;}
   friend Pyramid operator* (C Pyramid &pyramid, C Matrix  &m) {return Pyramid(pyramid)*=m;}

   // get
   Flt side  ()C {return scale*h*2;} // get pyramid side size
   Flt area  ()C;                    // get surface area
   Flt volume()C;                    // get volume

   Str asText()C {return S+"Scale: "+scale+", Height: "+h+", Pos: "+pos+", Dir: "+dir;} // get text description

   void toCorners(Vec (&v)[5])C; // convert to 5 corner points, where v[0] is the pyramid tip, and v[1..4] are the pyramid base

   // draw
   void draw(Color color=WHITE, Bool fill=false)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Pyramid() {}
   Pyramid(Flt scale, Flt h, C Vec &pos=VecZero, C Vec &dir=Vec(0,1,0)) {set(scale, h, pos, dir);} // 'dir' must be normalized
};
/******************************************************************************/
STRUCT(PyramidM , OrientM) // Pyramid Shape (mixed precision)
//{
   Flt scale, // proportional scale of side to height (0..Inf), default=1
       h    ; // height

   // set
   PyramidM& set(Flt scale, Flt h, C VecD &pos=0, C Vec &dir=Vec(0,1,0)) {T.scale=scale; T.h=h; setPosDir(pos, dir); return T;} // 'dir' must be normalized

   PyramidM() {}
   PyramidM(Flt scale, Flt h, C VecD &pos=0, C Vec &dir=Vec(0,1,0)) {set(scale, h, pos, dir);} // 'dir' must be normalized
};
/******************************************************************************/
Bool Cuts(C Vec  &point, C Pyramid  &pyramid); // if point cuts a pyramid
Bool Cuts(C VecD &point, C Pyramid  &pyramid); // if point cuts a pyramid
Bool Cuts(C VecD &point, C PyramidM &pyramid); // if point cuts a pyramid
/******************************************************************************/
