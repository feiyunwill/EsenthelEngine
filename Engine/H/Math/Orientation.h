/******************************************************************************

   Use 'Orient' to handle objects orientation.

   This is similar to 'Matrix', except the 'Orient' stores only 2 vectors of the orientation:
      'z-forward' and 'y-up' (the 'x-right' can always be computed from 'z' and 'y')

/******************************************************************************/
struct Orient // Orientation
{
   Vec dir ,                               // direction                  (z - forward)
       perp;                               // perpendicular to direction (y - up     )
   Vec cross()C {return Cross(perp, dir);} // cross                      (x - right  )

   Orient& operator+=(C Orient  &o);
   Orient& operator*=(  Flt      f);
   Orient& operator*=(C Vec     &v);
   Orient& operator*=(C Matrix3 &m) {return mul(m);}
   Orient& operator/=(C Matrix3 &m) {return div(m);}
   Bool    operator==(C Orient  &o)C;
   Bool    operator!=(C Orient  &o)C;

   friend Orient operator* (C Orient  &o,   Flt      f) {return Orient(o)*=f;}
   friend Orient operator* (  Flt      f, C Orient  &o) {return Orient(o)*=f;}
   friend Orient operator* (C Orient  &o, C Matrix3 &m) {return Orient(o)*=m;}

   // set
   Orient& zero    (          ); // set all vectors to zero
   Orient& identity(          ); // set identity
   Orient& setDir  (C Vec &dir); // set from 'dir' and calculate correct 'perp' to match it, 'dir' must be normalized
   Orient& fixPerp (          ); // fix perpendicular, use when 'dir' or 'perp' has changed, this method aligns 'perp' so it's perpendicular to 'dir' and normalized
   Bool    fix     (          ); // normalize and fix perpendicular, false on fail

   // transform
   Orient& mirrorX(); // mirror in X axis
#if EE_PRIVATE
   Orient& mirrorY(); // mirror in Y axis
   Orient& mirrorZ(); // mirror in Z axis

   Orient& chs        (); // change sign of all vectors
   Orient& rightToLeft(); // convert right to left hand coordinate system
#endif

   Orient& rotateDir  (Flt angle); // rotate along 'dir'   vector, this is equal to "mul(Matrix3().setRotate(dir    , angle), true)" but faster
   Orient& rotatePerp (Flt angle); // rotate along 'perp'  vector, this is equal to "mul(Matrix3().setRotate(perp   , angle), true)" but faster
   Orient& rotateCross(Flt angle); // rotate along 'cross' vector, this is equal to "mul(Matrix3().setRotate(cross(), angle), true)" but faster

   Orient& rotateX(Flt angle); // rotate along X axis, this is equal to "mul(Matrix3().setRotateX(angle), true)" but faster
   Orient& rotateY(Flt angle); // rotate along Y axis, this is equal to "mul(Matrix3().setRotateY(angle), true)" but faster
   Orient& rotateZ(Flt angle); // rotate along Z axis, this is equal to "mul(Matrix3().setRotateZ(angle), true)" but faster

   Orient& mul(C Matrix3 &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   Orient& div(C Matrix3 &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance

   Orient& rotateToDir(C Vec &dir           ); // rotate current orientation to 'dir' vector, 'dir' must be normalized
   Orient& rotateToDir(C Vec &dir, Flt blend); // rotate current orientation to 'dir' vector, 'dir' must be normalized, 'blend'=how much to rotate (0=no rotation, 0.5=half rotation, 1.0=full rotation)

   // operations
   Orient& inverse(              ) ; // inverse orientation
   void    inverse(Orient   &dest)C; // inverse orientation and store it in 'dest'
   void    inverse(Matrix3  &dest)C; // inverse orientation and store it in 'dest' matrix
   void    inverse(MatrixD3 &dest)C; // inverse orientation and store it in 'dest' matrix

   // io
   void save(MemPtr<TextNode> nodes)C; // save as text

   Orient() {}
   Orient(C Vec        &dir, C Vec &perp) {T.dir=dir; T.perp=perp;}
   Orient(C OrientD    &o);
   Orient(C Matrix3    &m);
   Orient(C MatrixD3   &m);
   Orient(C Quaternion &q);
};
/******************************************************************************/
struct OrientD // Orientation (double precision)
{
   VecD dir ,                               // direction                  (z - forward)
        perp;                               // perpendicular to direction (y - up     )
   VecD cross()C {return Cross(perp, dir);} // cross                      (x - right  )

   OrientD& operator*=(  Dbl       f);
   OrientD& operator*=(C VecD     &v);
   OrientD& operator*=(C MatrixD3 &m) {return mul(m);}
   OrientD& operator/=(C MatrixD3 &m) {return div(m);}
   Bool     operator==(C OrientD  &o)C;
   Bool     operator!=(C OrientD  &o)C;

   friend OrientD operator* (C OrientD  &o,   Dbl       f) {return OrientD(o)*=f;}
   friend OrientD operator* (  Dbl       f, C OrientD  &o) {return OrientD(o)*=f;}
   friend OrientD operator* (C OrientD  &o, C MatrixD3 &m) {return OrientD(o)*=m;}

   // set
   OrientD& zero    (           ); // set all vectors to zero
   OrientD& identity(           ); // set identity
   OrientD& setDir  (C VecD &dir); // set from 'dir' and calculate correct 'perp' to match it, 'dir' must be normalized
   OrientD& fixPerp (           ); // fix perpendicular, use when 'dir' or 'perp' has changed, this method aligns 'perp' so it's perpendicular to 'dir' and normalized
   Bool     fix     (           ); // normalize and fix perpendicular, false on fail

   // transform
   OrientD& mirrorX(); // mirror in X axis
#if EE_PRIVATE
   OrientD& mirrorY(); // mirror in Y axis
   OrientD& mirrorZ(); // mirror in Z axis

   OrientD& chs(); // change sign of all vectors
#endif
   OrientD& mul(C MatrixD3 &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientD& div(C MatrixD3 &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance

   // operations
   OrientD& inverse(              ) ; // inverse orientation
   void     inverse(OrientD  &dest)C; // inverse orientation and store it in 'dest'
   void     inverse(MatrixD3 &dest)C; // inverse orientation and store it in 'dest' matrix

   // io
   void save(MemPtr<TextNode> nodes)C; // save as text

   OrientD() {}
   OrientD(C VecD     &dir, C VecD &perp) {T.dir=dir; T.perp=perp;}
   OrientD(C Orient   &o);
   OrientD(C Matrix3  &m);
   OrientD(C MatrixD3 &m);
};
/******************************************************************************/
STRUCT(OrientP , Orient) // Positioned Orientation
//{
   Vec pos; // position

   OrientP& operator+=(C Vec     &v) {pos+=v; return T;}
   OrientP& operator-=(C Vec     &v) {pos-=v; return T;}
   OrientP& operator+=(C OrientP &o);
   OrientP& operator*=(  Flt      f);
   OrientP& operator*=(C Vec     &v);
   OrientP& operator*=(C Matrix3 &m) {return mul(m);}
   OrientP& operator*=(C Matrix  &m) {return mul(m);}
   OrientP& operator*=(C MatrixM &m) {return mul(m);}
   OrientP& operator/=(C Matrix3 &m) {return div(m);}
   OrientP& operator/=(C Matrix  &m) {return div(m);}
   OrientP& operator/=(C MatrixM &m) {return div(m);}
   Bool     operator==(C OrientP &o)C;
   Bool     operator!=(C OrientP &o)C;

   friend OrientP operator* (C OrientP &o,   Flt      f) {return OrientP(o)*=f;}
   friend OrientP operator* (  Flt      f, C OrientP &o) {return OrientP(o)*=f;}
   friend OrientP operator* (C OrientP &o, C Matrix3 &m) {return OrientP(o)*=m;}
   friend OrientP operator* (C OrientP &o, C Matrix  &m) {return OrientP(o)*=m;}
   friend OrientP operator* (C OrientP &o, C MatrixM &m) {return OrientP(o)*=m;}

   // set
   OrientP& zero     (                      ); // set all vectors to zero
   OrientP& identity (                      ); // set identity
   OrientP& setPosDir(C Vec &pos, C Vec &dir); // set from 'pos dir' and calculate correct 'perp' to match 'dir', 'dir' must be normalized

   // transform
   OrientP& mirrorX(); // mirror in X axis
#if EE_PRIVATE
   OrientP& mirrorY(); // mirror in Y axis
   OrientP& mirrorZ(); // mirror in Z axis

   OrientP& rightToLeft(); // convert right to left hand coordinate system
#endif
   OrientP& mul(C Matrix3 &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientP& mul(C Matrix  &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientP& mul(C MatrixM &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientP& div(C Matrix3 &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientP& div(C Matrix  &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientP& div(C MatrixM &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance

   // draw
   void draw(C Color &color, Flt size=0.2f)C; // this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   // io
   void save(MemPtr<TextNode> nodes)C; // save as text

   OrientP() {}
   OrientP(C Vec        &pos, C Vec &dir, C Vec &perp) {T.pos=pos; T.dir=dir; T.perp=perp;}
   OrientP(C Orient     &o);
   OrientP(C Matrix3    &m);
   OrientP(C Matrix     &m);
   OrientP(C MatrixM    &m);
   OrientP(C Quaternion &q);
};
/******************************************************************************/
STRUCT(OrientM , Orient) // Positioned Orientation (mixed precision)
//{
   VecD pos; // position

   OrientM& operator+=(C VecD    &v) {pos+=v; return T;}
   OrientM& operator-=(C VecD    &v) {pos-=v; return T;}
   OrientM& operator*=(  Flt      f);
   OrientM& operator*=(C Vec     &v);
   OrientM& operator*=(C Matrix3 &m) {return mul(m);}
   OrientM& operator*=(C Matrix  &m) {return mul(m);}
   OrientM& operator*=(C MatrixM &m) {return mul(m);}
   OrientM& operator/=(C Matrix3 &m) {return div(m);}
   OrientM& operator/=(C Matrix  &m) {return div(m);}
   OrientM& operator/=(C MatrixM &m) {return div(m);}
   Bool     operator==(C OrientM &o)C;
   Bool     operator!=(C OrientM &o)C;

   friend OrientM operator* (C OrientM &o,   Flt      f) {return OrientM(o)*=f;}
   friend OrientM operator* (  Flt      f, C OrientM &o) {return OrientM(o)*=f;}
   friend OrientM operator* (C OrientM &o, C Matrix3 &m) {return OrientM(o)*=m;}
   friend OrientM operator* (C OrientM &o, C Matrix  &m) {return OrientM(o)*=m;}
   friend OrientM operator* (C OrientM &o, C MatrixM &m) {return OrientM(o)*=m;}

   // set
   OrientM& zero     (                       ); // set all vectors to zero
   OrientM& identity (                       ); // set identity
   OrientM& setPosDir(C VecD &pos, C Vec &dir); // set from 'pos dir' and calculate correct 'perp' to match 'dir', 'dir' must be normalized

   // transform
   OrientM& mirrorX(); // mirror in X axis
#if EE_PRIVATE
   OrientM& mirrorY(); // mirror in Y axis
   OrientM& mirrorZ(); // mirror in Z axis
#endif
   OrientM& mul(C Matrix3 &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientM& mul(C Matrix  &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientM& mul(C MatrixM &matrix, Bool normalized=false); // transform by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientM& div(C Matrix3 &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientM& div(C Matrix  &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance
   OrientM& div(C MatrixM &matrix, Bool normalized=false); // divide    by matrix, if 'matrix' is normalized set 'normalized' to true for more performance

   // draw
   void draw(C Color &color, Flt size=0.2f)C; // this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   // io
   void save(MemPtr<TextNode> nodes)C; // save as text

   OrientM() {}
   OrientM(C VecD       &pos, C Vec &dir, C Vec &perp) {T.pos=pos; T.dir=dir; T.perp=perp;}
   OrientM(C Orient     &o);
   OrientM(C Matrix3    &m);
   OrientM(C Matrix     &m);
   OrientM(C MatrixM    &m);
   OrientM(C Quaternion &q);
};
/******************************************************************************/
struct AxisRoll // Axis+Roll based rotation
{
   Vec axis; // direction of this vector specifies the rotation axis, and the length of this vector specifies the rotation angle
   Flt roll; // roll angle, rotation along the original direction

   Vec4& v4()  {return (Vec4&)T;} // return this in Vec4 format, this returns reference to self and not a new object!
 C Vec4& v4()C {return (Vec4&)T;} // return this in Vec4 format, this returns reference to self and not a new object!

   // get / set
   Bool      any (                                 )C {return v4().any();}
   AxisRoll& zero(                                 )  {axis.zero(); roll=0; return T;} // clear to zero
   AxisRoll& from(C Orient &start, C Orient &result); // set axis roll which transforms 'start' into 'result'

   // transform
   AxisRoll& mul       (C Matrix3 &matrix);                              // transform by matrix
   AxisRoll& operator*=(C Matrix3 &matrix) {return mul(matrix);}         // transform by matrix
   AxisRoll& operator*=(  Flt      f     ) {axis*=f; roll*=f; return T;} // scale by 'f'
};
/******************************************************************************/
Orient Lerp(C Orient &a, C Orient &b, Flt step); // linear interpolation, 'step'=0..1
/******************************************************************************/
Bool Equal(C Orient  &a, C Orient  &b, Flt eps=EPS                   );
Bool Equal(C OrientD &a, C OrientD &b, Dbl eps=EPSD                  );
Bool Equal(C OrientP &a, C OrientP &b, Flt eps=EPS , Flt pos_eps=EPS );
Bool Equal(C OrientM &a, C OrientM &b, Flt eps=EPS , Dbl pos_eps=EPSD);
/******************************************************************************/
Orient GetAnimOrient(C Orient  &orn ,                                             C Orient *parent=null                               ); // get animation orientation     'orn'                               to be displayed when having 'parent'                               , which is " orn               /  parent               "
Orient GetAnimOrient(C Orient  &bone, C Matrix3 &bone_matrix,                     C Orient *parent=null, C Matrix3 *parent_matrix=null); // get animation orientation for 'bone' transformed by 'bone_matrix' to be displayed when having 'parent' transformed by 'parent_matrix', which is "(bone*bone_matrix) / (parent*parent_matrix)"
Vec    GetAnimPos   (C OrientP &bone, C Vec     &bone_pos   , C Orient &bone_orn, C Orient *parent=null, C Matrix  *parent_matrix=null); // get animation position    for 'bone' currently set with 'bone_orn' orientation and having 'bone_pos' offset, with 'parent' transformed by 'parent_matrix'
/******************************************************************************/
