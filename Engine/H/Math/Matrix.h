/******************************************************************************

   Use 'Matrix3' to represent objects orientation and scale.

   Use 'Matrix' to represent objects orientation, scale and position.

   Use 'GetVel' to calculate objects velocities according to its previous and current matrix.

   Use 'SetMatrix' to set mesh rendering matrix (use before manual drawing only).

/******************************************************************************/
struct Matrix3 // Matrix 3x3 (orientation + scale)
{
   Vec x, // right   vector
       y, // up      vector
       z; // forward vector

   // transform
   Matrix3& operator*=(     Flt      f);
   Matrix3& operator/=(     Flt      f);
   Matrix3& operator*=(C    Vec     &v);
   Matrix3& operator/=(C    Vec     &v);
   Matrix3& operator+=(C    Matrix3 &m);
   Matrix3& operator-=(C    Matrix3 &m);
   Matrix3& operator*=(C    Matrix3 &m) {return mul(m);}
   Matrix3& operator/=(C    Matrix3 &m) {return div(m);}
   Matrix3& operator*=(C RevMatrix3 &m) {return mul(m);}
   Bool     operator==(C    Matrix3 &m)C;
   Bool     operator!=(C    Matrix3 &m)C;
 
   friend Matrix3 operator* (C Matrix3 &a, C    Matrix3 &b) {Matrix3 temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix3 operator* (C Matrix3 &a, C RevMatrix3 &b) {Matrix3 temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix3 operator/ (C Matrix3 &a, C    Matrix3 &b) {Matrix3 temp; a.div    (b, temp); return temp;} // get a/b
   friend Matrix3 operator~ (C Matrix3 &m                 ) {Matrix3 temp; m.inverse(   temp); return temp;} // get inversed 'm'

   void     mul(C    Matrix3 &matrix, Matrix3 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void     mul(C RevMatrix3 &matrix, Matrix3 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   Matrix3& mul(C    Matrix3 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   Matrix3& mul(C RevMatrix3 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   void     div(C Matrix3 &matrix, Matrix3 &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   Matrix3& div(C Matrix3 &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

   void     divNormalized(C Matrix3 &matrix, Matrix3 &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   Matrix3& divNormalized(C Matrix3 &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

   void     inverse(Matrix3 &dest, Bool normalized=false)C;                                   // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
   Matrix3& inverse(               Bool normalized=false) {inverse(T, normalized); return T;} // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

   void     inverseNonOrthogonal(Matrix3 &dest)C;                                    // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
   Matrix3& inverseNonOrthogonal(             ) {inverseNonOrthogonal(T); return T;} // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

   Matrix3& inverseScale(); // inverse scale

   Matrix3& normalize(            ); // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
   Matrix3& normalize(  Flt  scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
   Matrix3& normalize(C Vec &scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

   Matrix3& scale     (            Flt  scale) {T*=scale; return T;} // scale
   Matrix3& scale     (          C Vec &scale) {T*=scale; return T;} // scale
   Matrix3& scaleL    (          C Vec &scale);                      // scale in local space
   Matrix3& scale     (C Vec &dir, Flt  scale);                      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
   Matrix3& scalePlane(C Vec &nrm, Flt  scale);                      // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

   Matrix3& rotateX (             Flt angle); // rotate by x axis
   Matrix3& rotateY (             Flt angle); // rotate by y axis
   Matrix3& rotateZ (             Flt angle); // rotate by z axis
   Matrix3& rotateXY(  Flt  x   , Flt y    ); // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
   Matrix3& rotate  (C Vec &axis, Flt angle); // rotate by vector, 'axis' must be normalized
   Matrix3& rotateXL(             Flt angle); // rotate matrix by its x vector (x-axis rotation in local space)
   Matrix3& rotateYL(             Flt angle); // rotate matrix by its y vector (y-axis rotation in local space)
   Matrix3& rotateZL(             Flt angle); // rotate matrix by its z vector (z-axis rotation in local space)

   Matrix3& rotateXLOrthoNormalized(Flt angle); // rotate matrix by its x vector (x-axis rotation in local space), this method is faster than 'rotateXL' however matrix must be orthogonal and normalized
   Matrix3& rotateYLOrthoNormalized(Flt angle); // rotate matrix by its y vector (y-axis rotation in local space), this method is faster than 'rotateYL' however matrix must be orthogonal and normalized
   Matrix3& rotateZLOrthoNormalized(Flt angle); // rotate matrix by its z vector (z-axis rotation in local space), this method is faster than 'rotateZL' however matrix must be orthogonal and normalized

   Matrix3& rotateXLOrthoNormalized(Flt cos, Flt sin); // rotate matrix by its x vector (x-axis rotation in local space), this method works like 'rotateXLOrthoNormalized(Flt angle)' however it accepts 'Cos' and 'Sin' of 'angle'

   Matrix3& mirrorX(             ); // mirror matrix in X axis
   Matrix3& mirrorY(             ); // mirror matrix in Y axis
   Matrix3& mirrorZ(             ); // mirror matrix in Z axis
   Matrix3& mirror (C Vec &normal); // mirror matrix by   plane normal

   Matrix3& swapYZ(); // swap Y and Z components of every vector

   // set (set methods reset the full matrix)
   Matrix3& identity(         ); // set as identity
   Matrix3& identity(Flt blend); // set as identity, this method is similar to 'identity()' however it does not perform full reset of the matrix. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)
   Matrix3& zero    (         ); // set all vectors to zero

   Matrix3& setScale(  Flt  scale); // set as scaled identity
   Matrix3& setScale(C Vec &scale); // set as scaled identity

   Matrix3& setRotateX (             Flt angle); // set as   x-rotated identity
   Matrix3& setRotateY (             Flt angle); // set as   y-rotated identity
   Matrix3& setRotateZ (             Flt angle); // set as   z-rotated identity
   Matrix3& setRotateXY(  Flt  x   , Flt y    ); // set as x-y-rotated identity, works the same as 'setRotateX(x).rotateY(y)' but faster
   Matrix3& setRotate  (C Vec &axis, Flt angle); // set as     rotated by vector identity, 'axis' must be normalized
#if EE_PRIVATE
   Matrix3& setRotateCosSin(C Vec &axis, Flt cos, Flt sin); // set as rotated by vector identity, 'axis' must be normalized
#endif

   Matrix3& setOrient  (DIR_ENUM dir                       ); // set as identity orientation from DIR_ENUM
   Matrix3& setRight   (C Vec &right                       ); // set as x='right'       and calculate correct y,z, 'right'        must be normalized
   Matrix3& setUp      (C Vec &up                          ); // set as y='up'          and calculate correct x,z, 'up'           must be normalized
   Matrix3& setDir     (C Vec &dir                         ); // set as z='dir'         and calculate correct x,y, 'dir'          must be normalized
   Matrix3& setDir     (C Vec &dir, C Vec &up              ); // set as z='dir', y='up' and calculate correct x  , 'dir up'       must be normalized
   Matrix3& setDir     (C Vec &dir, C Vec &up, C Vec &right); // set as z='dir', y='up', x='right'               , 'dir up right' must be normalized
   Matrix3& setRotation(C Vec &dir_from, C Vec &dir_to                     ); // set as matrix which rotates 'dir_from' to 'dir_to',                                              'dir_from dir_to' must be normalized
   Matrix3& setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend          ); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value                          , 'dir_from dir_to' must be normalized
   Matrix3& setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend, Flt roll); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value and additional roll angle, 'dir_from dir_to' must be normalized

   // get
   Flt  determinant()C;
   Bool mirrored   ()C {return determinant()<0;} // if matrix is mirrored ('x' axis is on the other/left side)

   Vec    scale ()C; // get each    axis scale
   Vec    scale2()C; // get each    axis scale squared
   Flt avgScale ()C; // get average axis scale
   Flt maxScale ()C; // get maximum axis scale

   Vec angles   (                                )C; // get rotation angles, this allows to reconstruct the matrix using "setRotateZ(angle.z).rotateX(angle.x).rotateY(angle.y)" or faster by using "setRotateZ(angle.z).rotateXY(angle.x, angle.y)"
   Vec axis     (           Bool normalized=false)C; // get rotation axis                , if you know that the matrix is normalized then set 'normalized=true' for more performance
   Flt angle    (           Bool normalized=false)C; // get rotation angle               , if you know that the matrix is normalized then set 'normalized=true' for more performance
   Flt axisAngle(Vec &axis, Bool normalized=false)C; // get rotation axis and       angle, if you know that the matrix is normalized then set 'normalized=true' for more performance
   Vec axisAngle(           Bool normalized=false)C; // get rotation axis scaled by angle, if you know that the matrix is normalized then set 'normalized=true' for more performance
   Flt angleY   (           Bool normalized=false)C; // get rotation angle along Y axis  , if you know that the matrix is normalized then set 'normalized=true' for more performance, this is the same as "axisAngle(normalized).y" but faster

   Str asText(Int precision=INT_MAX)C {return S+"X: "+x.asText(precision)+", Y: "+y.asText(precision)+", Z:"+z.asText(precision);} // get text description

#if EE_PRIVATE
   // operations
   Vec2  convert(C Vec   &src, Bool normalized=false)C; // return converted 3D 'src' to 2D vector according to matrix x,y axes
   Vec   convert(C Vec2  &src                       )C; // return converted 2D 'src' to 3D vector according to matrix x,y axes
   Edge2 convert(C Edge  &src, Bool normalized=false)C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes
   Edge  convert(C Edge2 &src                       )C; // return converted 2D 'src' to 3D edge   according to matrix x,y axes
#endif

   // draw
   void draw(C Vec &pos, C Color &x_color=RED, C Color &y_color=GREEN, C Color &z_color=BLUE, Bool arrow=true)C; // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   Matrix3() {}
   Matrix3(  Flt         scale) {setScale(scale);}
   Matrix3(C Vec        &scale) {setScale(scale);}
   Matrix3(C Vec        &x, C Vec &y, C Vec &z) {T.x=x; T.y=y; T.z=z;}
   Matrix3(C MatrixD3   &m);
   Matrix3(C Matrix     &m);
   Matrix3(C Matrix4    &m);
   Matrix3(C Orient     &o);
   Matrix3(C Quaternion &q);
};
/******************************************************************************/
struct MatrixD3 // Matrix 3x3 (orientation + scale, double precision)
{
   VecD x, // right   vector
        y, // up      vector
        z; // forward vector

   // transform
   MatrixD3& operator*=(  Dbl       f);
   MatrixD3& operator/=(  Dbl       f);
   MatrixD3& operator*=(C VecD     &v);
   MatrixD3& operator/=(C VecD     &v);
   MatrixD3& operator+=(C MatrixD3 &m);
   MatrixD3& operator-=(C MatrixD3 &m);
   MatrixD3& operator*=(C Matrix3  &m) {return mul(m);}
   MatrixD3& operator*=(C MatrixD3 &m) {return mul(m);}
   MatrixD3& operator/=(C Matrix3  &m) {return div(m);}
   MatrixD3& operator/=(C MatrixD3 &m) {return div(m);}
   Bool      operator==(C MatrixD3 &m)C;
   Bool      operator!=(C MatrixD3 &m)C;
 
   friend MatrixD3 operator* (C MatrixD3 &a, C MatrixD3 &b) {MatrixD3 temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixD3 operator/ (C MatrixD3 &a, C MatrixD3 &b) {MatrixD3 temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixD3 operator~ (C MatrixD3 &m               ) {MatrixD3 temp; m.inverse(   temp); return temp;} // get inversed 'm'

   void      mul(C Matrix3  &matrix, MatrixD3 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void      mul(C MatrixD3 &matrix, MatrixD3 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   MatrixD3& mul(C Matrix3  &matrix                ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   MatrixD3& mul(C MatrixD3 &matrix                ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   void      div(C Matrix3  &matrix, MatrixD3 &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void      div(C MatrixD3 &matrix, MatrixD3 &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   MatrixD3& div(C Matrix3  &matrix                ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   MatrixD3& div(C MatrixD3 &matrix                ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

   void      divNormalized(C Matrix3  &matrix, MatrixD3 &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void      divNormalized(C MatrixD3 &matrix, MatrixD3 &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   MatrixD3& divNormalized(C Matrix3  &matrix                ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   MatrixD3& divNormalized(C MatrixD3 &matrix                ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

   void      inverse(MatrixD3 &dest, Bool normalized=false)C;                                   // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
   MatrixD3& inverse(                Bool normalized=false) {inverse(T, normalized); return T;} // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

   void      inverseNonOrthogonal(MatrixD3 &dest)C;                                    // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
   MatrixD3& inverseNonOrthogonal(              ) {inverseNonOrthogonal(T); return T;} // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

   MatrixD3& inverseScale(); // inverse scale

   MatrixD3& normalize(             ); // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
   MatrixD3& normalize(  Dbl   scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
   MatrixD3& normalize(C VecD &scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

   MatrixD3& scale     (             Dbl   scale) {T*=scale; return T;} // scale
   MatrixD3& scale     (           C VecD &scale) {T*=scale; return T;} // scale
   MatrixD3& scale     (C VecD &dir, Dbl   scale);                      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
   MatrixD3& scalePlane(C VecD &nrm, Dbl   scale);                      // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

   MatrixD3& rotateX (              Dbl angle); // rotate by x axis
   MatrixD3& rotateY (              Dbl angle); // rotate by y axis
   MatrixD3& rotateZ (              Dbl angle); // rotate by z axis
   MatrixD3& rotateXY(  Dbl   x   , Dbl y    ); // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
   MatrixD3& rotate  (C VecD &axis, Dbl angle); // rotate by vector, 'axis' must be normalized
   MatrixD3& rotateXL(              Dbl angle); // rotate matrix by its x vector (x-axis rotation in local space)
   MatrixD3& rotateYL(              Dbl angle); // rotate matrix by its y vector (y-axis rotation in local space)
   MatrixD3& rotateZL(              Dbl angle); // rotate matrix by its z vector (z-axis rotation in local space)

   MatrixD3& rotateXLOrthoNormalized(Dbl angle); // rotate matrix by its x vector (x-axis rotation in local space), this method is faster than 'rotateXL' however matrix must be orthogonal and normalized
   MatrixD3& rotateYLOrthoNormalized(Dbl angle); // rotate matrix by its y vector (y-axis rotation in local space), this method is faster than 'rotateYL' however matrix must be orthogonal and normalized
   MatrixD3& rotateZLOrthoNormalized(Dbl angle); // rotate matrix by its z vector (z-axis rotation in local space), this method is faster than 'rotateZL' however matrix must be orthogonal and normalized

   MatrixD3& mirrorX(              ); // mirror matrix in X axis
   MatrixD3& mirrorY(              ); // mirror matrix in Y axis
   MatrixD3& mirrorZ(              ); // mirror matrix in Z axis
   MatrixD3& mirror (C VecD &normal); // mirror matrix by   plane normal

   // set (set methods reset the full matrix)
   MatrixD3& identity(); // set as identity
   MatrixD3& zero    (); // set all vectors to zero

   MatrixD3& setScale(  Dbl   scale); // set as scaled identity
   MatrixD3& setScale(C VecD &scale); // set as scaled identity

   MatrixD3& setRotateX (              Dbl angle); // set as   x-rotated identity
   MatrixD3& setRotateY (              Dbl angle); // set as   y-rotated identity
   MatrixD3& setRotateZ (              Dbl angle); // set as   z-rotated identity
   MatrixD3& setRotateXY(  Dbl   x   , Dbl y    ); // set as x-y-rotated identity, works the same as 'setRotateX(x).rotateY(y)' but faster
   MatrixD3& setRotate  (C VecD &axis, Dbl angle); // set as     rotated by vector identity, 'axis' must be normalized
#if EE_PRIVATE
   MatrixD3& setRotateCosSin(C VecD &axis, Dbl cos, Dbl sin); // set as rotated by vector identity, 'axis' must be normalized
#endif

   MatrixD3& setOrient  (DIR_ENUM dir                          ); // set as identity orientation from DIR_ENUM
   MatrixD3& setRight   (C VecD &right                         ); // set as x='right'       and calculate correct y,z, 'right'        must be normalized
   MatrixD3& setUp      (C VecD &up                            ); // set as y='up'          and calculate correct x,z, 'up'           must be normalized
   MatrixD3& setDir     (C VecD &dir                           ); // set as z='dir'         and calculate correct x,y, 'dir'          must be normalized
   MatrixD3& setDir     (C VecD &dir, C VecD &up               ); // set as z='dir', y='up' and calculate correct x  , 'dir up'       must be normalized
   MatrixD3& setDir     (C VecD &dir, C VecD &up, C VecD &right); // set as z='dir', y='up', x='right'               , 'dir up right' must be normalized
   MatrixD3& setRotation(C VecD &dir_from, C VecD &dir_to                     ); // set as matrix which rotates 'dir_from' to 'dir_to',                                              'dir_from dir_to' must be normalized
   MatrixD3& setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend          ); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value                          , 'dir_from dir_to' must be normalized
   MatrixD3& setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend, Dbl roll); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value and additional roll angle, 'dir_from dir_to' must be normalized

   // get
   Dbl  determinant()C;
   Bool mirrored   ()C {return determinant()<0;} // if matrix is mirrored ('x' axis is on the other/left side)

   VecD   scale ()C; // get each    axis scale
   VecD   scale2()C; // get each    axis scale squared
   Dbl avgScale ()C; // get average axis scale
   Dbl maxScale ()C; // get maximum axis scale

   VecD angles   (                                 )C; // get rotation angles, this allows to reconstruct the matrix using "setRotateZ(angle.z).rotateX(angle.x).rotateY(angle.y)" or faster by using "setRotateZ(angle.z).rotateXY(angle.x, angle.y)"
   VecD axis     (            Bool normalized=false)C; // get rotation axis                , if you know that the matrix is normalized then set 'normalized=true' for more performance
   Dbl  angle    (            Bool normalized=false)C; // get rotation angle               , if you know that the matrix is normalized then set 'normalized=true' for more performance
   Dbl  axisAngle(VecD &axis, Bool normalized=false)C; // get rotation axis and angle      , if you know that the matrix is normalized then set 'normalized=true' for more performance
   VecD axisAngle(            Bool normalized=false)C; // get rotation axis scaled by angle, if you know that the matrix is normalized then set 'normalized=true' for more performance

   Str asText(Int precision=INT_MAX)C {return S+"X: "+x.asText(precision)+", Y: "+y.asText(precision)+", Z:"+z.asText(precision);} // get text description

#if EE_PRIVATE
   // operations
   VecD2  convert(C VecD   &src, Bool normalized=false)C; // return converted 3D 'src' to 2D vector according to matrix x,y axes
   VecD   convert(C VecD2  &src                       )C; // retrun converted 2D 'src' to 3D vector according to matrix x,y axes
   EdgeD2 convert(C EdgeD  &src, Bool normalized=false)C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes
   EdgeD  convert(C EdgeD2 &src                       )C; // retrun converted 2D 'src' to 3D edge   according to matrix x,y axes
#endif

   // draw
   void draw(C VecD &pos, C Color &x_color=RED, C Color &y_color=GREEN, C Color &z_color=BLUE, Bool arrow=true)C; // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   MatrixD3() {}
   MatrixD3(  Dbl      scale) {setScale(scale);}
   MatrixD3(C VecD    &scale) {setScale(scale);}
   MatrixD3(C VecD    &x, C VecD &y, C VecD &z) {T.x=x; T.y=y; T.z=z;}
   MatrixD3(C Matrix3 &m);
   MatrixD3(C MatrixD &m);
   MatrixD3(C Orient  &o);
   MatrixD3(C OrientD &o);
};
/******************************************************************************/
STRUCT(Matrix , Matrix3) // Matrix 4x3 (orientation + scale + position)
//{
   Vec pos; // position

   Matrix3& orn()  {return T;} // get reference to self as       'Matrix3'
 C Matrix3& orn()C {return T;} // get reference to self as const 'Matrix3'

   // transform
   Matrix& operator*=(     Flt      f);
   Matrix& operator/=(     Flt      f);
   Matrix& operator*=(C    Vec     &v);
   Matrix& operator/=(C    Vec     &v);
   Matrix& operator+=(C    Vec     &v) {pos+=v; return T;}
   Matrix& operator-=(C    Vec     &v) {pos-=v; return T;}
   Matrix& operator+=(C    Matrix  &m);
   Matrix& operator-=(C    Matrix  &m);
   Matrix& operator*=(C    Matrix3 &m) {return mul(m);}
   Matrix& operator*=(C    Matrix  &m) {return mul(m);}
   Matrix& operator*=(C    MatrixM &m) {return mul(m);}
   Matrix& operator/=(C    Matrix3 &m) {return div(m);}
   Matrix& operator/=(C    Matrix  &m) {return div(m);}
   Matrix& operator/=(C    MatrixM &m) {return div(m);}
   Matrix& operator*=(C RevMatrix  &m) {return mul(m);}
   Bool    operator==(C    Matrix  &m)C;
   Bool    operator!=(C    Matrix  &m)C;

   friend Matrix operator+ (C Matrix  &m, C    Vec     &v) {return Matrix(m)+=v;                         } // get m+v
   friend Matrix operator- (C Matrix  &m, C    Vec     &v) {return Matrix(m)-=v;                         } // get m-v
   friend Matrix operator* (C Matrix  &a, C    Matrix3 &b) {Matrix temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix operator* (C Matrix  &a, C    Matrix  &b) {Matrix temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix operator* (C Matrix  &a, C    MatrixM &b) {Matrix temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix operator* (C Matrix  &a, C RevMatrix  &b) {Matrix temp; a.mul    (b, temp); return temp;} // get a*b
   friend Matrix operator/ (C Matrix  &a, C    Matrix3 &b) {Matrix temp; a.div    (b, temp); return temp;} // get a/b
   friend Matrix operator/ (C Matrix  &a, C    Matrix  &b) {Matrix temp; a.div    (b, temp); return temp;} // get a/b
   friend Matrix operator/ (C Matrix  &a, C    MatrixM &b) {Matrix temp; a.div    (b, temp); return temp;} // get a/b
   friend Matrix operator~ (C Matrix  &m                 ) {Matrix temp; m.inverse(   temp); return temp;} // get inversed 'm'
   friend Matrix operator* (C Matrix3 &a, C    Matrix  &b) {return Matrix(a)*=b;} // get a*b
   friend Matrix operator/ (C Matrix3 &a, C    Matrix  &b) {return Matrix(a)/=b;} // get a/b

   void    mul(C    Matrix3 &matrix, Matrix  &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void    mul(C    Matrix  &matrix, Matrix  &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void    mul(C    MatrixM &matrix, Matrix  &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void    mul(C    Matrix  &matrix, Matrix4 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void    mul(C    Matrix4 &matrix, Matrix4 &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void    mul(C RevMatrix  &matrix, Matrix  &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   Matrix& mul(C    Matrix3 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   Matrix& mul(C    Matrix  &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   Matrix& mul(C    MatrixM &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   Matrix& mul(C RevMatrix  &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   void    mulTimes(Int n, C    Matrix &matrix, Matrix &dest)C;                                   // multiply self by 'matrix' 'n'-times and store result in 'dest'
   void    mulTimes(Int n, C RevMatrix &matrix, Matrix &dest)C;                                   // multiply self by 'matrix' 'n'-times and store result in 'dest'
   Matrix& mulTimes(Int n, C    Matrix &matrix              ) {mulTimes(n, matrix, T); return T;} // multiply self by 'matrix' 'n'-times
   Matrix& mulTimes(Int n, C RevMatrix &matrix              ) {mulTimes(n, matrix, T); return T;} // multiply self by 'matrix' 'n'-times

   void    div(C Matrix3 &matrix, Matrix &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void    div(C Matrix  &matrix, Matrix &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void    div(C MatrixM &matrix, Matrix &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   Matrix& div(C Matrix3 &matrix              ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   Matrix& div(C Matrix  &matrix              ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   Matrix& div(C MatrixM &matrix              ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

   void    divNormalized(C Matrix3 &matrix, Matrix &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void    divNormalized(C Matrix  &matrix, Matrix &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void    divNormalized(C MatrixM &matrix, Matrix &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   Matrix& divNormalized(C Matrix3 &matrix              ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   Matrix& divNormalized(C Matrix  &matrix              ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   Matrix& divNormalized(C MatrixM &matrix              ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

   void    inverse(Matrix &dest, Bool normalized=false)C;                                   // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
   Matrix& inverse(              Bool normalized=false) {inverse(T, normalized); return T;} // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

   void    inverseNonOrthogonal(Matrix &dest)C;                                    // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
   Matrix& inverseNonOrthogonal(            ) {inverseNonOrthogonal(T); return T;} // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

   Matrix& normalize(            ) {super::normalize(     ); return T;} // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 'scale'
   Matrix& normalize(  Flt  scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
   Matrix& normalize(C Vec &scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

   Matrix& move    (Flt x, Flt y, Flt z) {pos   +=Vec(x, y, z); return T;} // move
   Matrix& move    (C Vec2 &move       ) {pos.xy+=move        ; return T;} // move
   Matrix& move    (C Vec  &move       ) {pos   +=move        ; return T;} // move
   Matrix& moveBack(C Vec  &move       ) {pos   -=move        ; return T;} // move back

   Matrix& anchor(C Vec &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

   Matrix& scale     (            Flt  scale) {           T*=scale ; return T;} // scale
   Matrix& scale     (          C Vec &scale) {           T*=scale ; return T;} // scale
   Matrix& scaleL    (          C Vec &scale);                                  // scale in local space
   Matrix& scaleOrn  (            Flt  scale) {super::scale (scale); return T;} // scale orientation only
   Matrix& scaleOrn  (          C Vec &scale) {super::scale (scale); return T;} // scale orientation only
   Matrix& scaleOrnL (          C Vec &scale) {super::scaleL(scale); return T;} // scale orientation only in local space
   Matrix& scale     (C Vec &dir, Flt  scale);                                  // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
   Matrix& scalePlane(C Vec &nrm, Flt  scale);                                  // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

   Matrix& rotateX (             Flt angle);                                    // rotate by x axis
   Matrix& rotateY (             Flt angle);                                    // rotate by y axis
   Matrix& rotateZ (             Flt angle);                                    // rotate by z axis
   Matrix& rotateXY(  Flt  x   , Flt y    );                                    // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
   Matrix& rotate  (C Vec &axis, Flt angle);                                    // rotate by vector, 'axis' must be normalized
   Matrix& rotateXL(             Flt angle) {super::rotateXL(angle); return T;} // rotate matrix by its x vector (x-axis rotation in local space)
   Matrix& rotateYL(             Flt angle) {super::rotateYL(angle); return T;} // rotate matrix by its y vector (y-axis rotation in local space)
   Matrix& rotateZL(             Flt angle) {super::rotateZL(angle); return T;} // rotate matrix by its z vector (z-axis rotation in local space)

   Matrix& mirrorX(              ); // mirror matrix in X axis
   Matrix& mirrorY(              ); // mirror matrix in Y axis
   Matrix& mirrorZ(              ); // mirror matrix in Z axis
   Matrix& mirror (C Plane &plane); // mirror matrix by   plane

   Matrix& swapYZ(); // swap Y and Z components of every vector

   // set (set methods reset the full matrix)
   Matrix& identity(         ); // set as identity
   Matrix& identity(Flt blend); // set as identity, this method is similar to 'identity()' however it does not perform full reset of the matrix. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)
   Matrix& zero    (         ); // set all vectors to zero

   Matrix& setPos     (Flt x, Flt y, Flt z        ); // set as positioned identity
   Matrix& setPos     (C Vec2 &pos                ); // set as positioned identity
   Matrix& setPos     (C Vec  &pos                ); // set as positioned identity
   Matrix& setScale   (  Flt   scale              ); // set as scaled     identity
   Matrix& setScale   (C Vec  &scale              ); // set as scaled     identity
   Matrix& setPosScale(C Vec  &pos  ,   Flt  scale); // set as positioned & scaled identity
   Matrix& setPosScale(C Vec  &pos  , C Vec &scale); // set as positioned & scaled identity
   Matrix& setScalePos(  Flt   scale, C Vec &pos  ); // set as scaled & positioned identity
   Matrix& setScalePos(C Vec  &scale, C Vec &pos  ); // set as scaled & positioned identity

   Matrix& setRotateX (             Flt angle); // set as   x-rotated identity
   Matrix& setRotateY (             Flt angle); // set as   y-rotated identity
   Matrix& setRotateZ (             Flt angle); // set as   z-rotated identity
   Matrix& setRotateXY(  Flt  x   , Flt y    ); // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
   Matrix& setRotate  (C Vec &axis, Flt angle); // set as     rotated by vector identity, 'axis' must be normalized
   Matrix& setRotation(C Vec &pos, C Vec &dir_from, C Vec &dir_to, Flt blend=1); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value, 'dir_from dir_to' must be normalized

   Matrix& setPosOrient(C Vec &pos,   DIR_ENUM dir                     ); // set as positioned orientation from DIR_ENUM
   Matrix& setPosRight (C Vec &pos, C Vec &right                       ); // set as pos='pos', x='right'       and calculate correct y,z, 'right'        must be normalized
   Matrix& setPosUp    (C Vec &pos, C Vec &up                          ); // set as pos='pos', y='up'          and calculate correct x,z, 'up'           must be normalized
   Matrix& setPosDir   (C Vec &pos, C Vec &dir                         ); // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'          must be normalized
   Matrix& setPosDir   (C Vec &pos, C Vec &dir, C Vec &up              ); // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir up'       must be normalized
   Matrix& setPosDir   (C Vec &pos, C Vec &dir, C Vec &up, C Vec &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir up right' must be normalized

   Matrix& set          (C Box &src, C Box &dest); // set as matrix that transforms 'src' to 'dest' (src*m=dest)
   Matrix& setNormalizeX(C Box &box             ); // set as matrix that (box*m).w()         =1
   Matrix& setNormalizeY(C Box &box             ); // set as matrix that (box*m).h()         =1
   Matrix& setNormalizeZ(C Box &box             ); // set as matrix that (box*m).d()         =1
   Matrix& setNormalize (C Box &box             ); // set as matrix that (box*m).size().max()=1

   // get
   Vec scale()C {return super::scale();} // get each axis scale

   Str asText(Int precision=INT_MAX)C {return super::asText(precision)+", Pos: "+pos.asText(precision);} // get text description

   // operations
#if EE_PRIVATE
   Vec2  convert(C Vec   &src, Bool normalized=false)C; // return converted 3D 'src' to 2D vector according to matrix x,y axes and position
   Vec   convert(C Vec2  &src                       )C; // return converted 2D 'src' to 3D vector according to matrix x,y axes and position
   Edge2 convert(C Edge  &src, Bool normalized=false)C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes and position
   Edge  convert(C Edge2 &src                       )C; // return converted 2D 'src' to 3D edge   according to matrix x,y axes and position
#endif

   Matrix& setTransformAtPos(C Vec &pos, C Matrix3 &matrix); // set as transformation at position
   Matrix& setTransformAtPos(C Vec &pos, C Matrix  &matrix); // set as transformation at position
   Matrix&    transformAtPos(C Vec &pos, C Matrix3 &matrix); //        transform      at position
   Matrix&    transformAtPos(C Vec &pos, C Matrix  &matrix); //        transform      at position

   // draw
   void draw(C Color &x_color=RED, C Color &y_color=GREEN, C Color &z_color=BLUE, Bool arrow=true)C {super::draw(pos, x_color, y_color, z_color, arrow);} // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   Matrix() {}
   Matrix(  Flt       scale                  ) {setScale   (scale       );}
   Matrix(C Vec      &pos                    ) {setPos     (pos         );}
   Matrix(C Vec      &pos  ,   Flt      scale) {setPosScale(pos  , scale);}
   Matrix(  Flt       scale, C Vec     &pos  ) {setScalePos(scale, pos  );}
   Matrix(C Vec      &scale, C Vec     &pos  ) {setScalePos(scale, pos  );}
   Matrix(C Matrix3  &orn  , C Vec     &pos  ) {T.orn()=orn;    T.pos=pos;}
   Matrix(C Vec      &pos  , C Matrix3 &orn  ) {T.orn()=orn;    T.pos=pos;}
   Matrix(C Matrix3  &m);
   Matrix(C MatrixD3 &m);
   Matrix(C MatrixM  &m);
   Matrix(C MatrixD  &m);
   Matrix(C Matrix4  &m);
   Matrix(C OrientP  &o);
};extern Matrix
   const MatrixIdentity; // identity
#if EE_PRIVATE

#define MAX_MATRIX_DX9   60 // max available Object Matrixes in DirectX  9  Hardware GPU ( 60 Matrix * 3 Vec4 = 180 Vec4's)
#define MAX_MATRIX_DX10 256 // max available Object Matrixes in DirectX 10+ Hardware GPU (256 Matrix * 3 Vec4 = 768 Vec4's)
#define MAX_MATRIX_SW   256 // max available Object Matrixes in             Software CPU (256 Matrix * 3 Vec4 = 768 Vec4's)
#define MAX_MATRIX_HWMIN 60 // max available Object Matrixes in worst       Hardware GPU ( 60 Matrix * 3 Vec4 = 180 Vec4's)

#define MAY_NEED_BONE_SPLITS (DX9 || GL)

extern    MatrixM ObjMatrix              , // object matrix
                  CamMatrix              , // camera, this is always set, even when drawing shadows
                  CamMatrixInv           , // camera inversed = ~CamMatrix
                  EyeMatrix[2]           ; // 'ActiveCam.matrix' adjusted for both eyes 0=left, 1=right
extern    Matrix3 CamMatrixInvMotionScale; // 'ActiveCam.matrix' inversed and scaled by 'D.motionScale'
extern GpuMatrix *ViewMatrix             ;
#endif
/******************************************************************************/
STRUCT(MatrixM , Matrix3) // Matrix 4x3 (orientation + scale + position, mixed precision, uses Flt for orientation+scale and Dbl for position)
//{
   VecD pos; // position

   Matrix3& orn()  {return T;} // get reference to self as       'Matrix3'
 C Matrix3& orn()C {return T;} // get reference to self as const 'Matrix3'

   // transform
   MatrixM& operator*=(  Flt      f);
   MatrixM& operator/=(  Flt      f);
   MatrixM& operator*=(C Vec     &v);
   MatrixM& operator/=(C Vec     &v);
   MatrixM& operator+=(C Vec     &v) {pos+=v; return T;}
   MatrixM& operator-=(C Vec     &v) {pos-=v; return T;}
   MatrixM& operator+=(C VecD    &v) {pos+=v; return T;}
   MatrixM& operator-=(C VecD    &v) {pos-=v; return T;}
   MatrixM& operator+=(C MatrixM &m);
   MatrixM& operator-=(C MatrixM &m);
   MatrixM& operator*=(C Matrix3 &m) {return mul(m);}
   MatrixM& operator*=(C Matrix  &m) {return mul(m);}
   MatrixM& operator*=(C MatrixM &m) {return mul(m);}
   MatrixM& operator/=(C Matrix3 &m) {return div(m);}
   MatrixM& operator/=(C Matrix  &m) {return div(m);}
   MatrixM& operator/=(C MatrixM &m) {return div(m);}
   Bool     operator==(C MatrixM &m)C;
   Bool     operator!=(C MatrixM &m)C;

   friend MatrixM operator+ (C MatrixM &m, C Vec     &v) {return MatrixM(m)+=v;                         } // get m+v
   friend MatrixM operator- (C MatrixM &m, C Vec     &v) {return MatrixM(m)-=v;                         } // get m-v
   friend MatrixM operator+ (C MatrixM &m, C VecD    &v) {return MatrixM(m)+=v;                         } // get m+v
   friend MatrixM operator- (C MatrixM &m, C VecD    &v) {return MatrixM(m)-=v;                         } // get m-v
   friend MatrixM operator* (C MatrixM &a, C Matrix3 &b) {MatrixM temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixM operator* (C MatrixM &a, C Matrix  &b) {MatrixM temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixM operator* (C MatrixM &a, C MatrixM &b) {MatrixM temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixM operator/ (C MatrixM &a, C Matrix3 &b) {MatrixM temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixM operator/ (C MatrixM &a, C Matrix  &b) {MatrixM temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixM operator/ (C MatrixM &a, C MatrixM &b) {MatrixM temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixM operator~ (C MatrixM &m              ) {MatrixM temp; m.inverse(   temp); return temp;} // get inversed 'm'
   friend MatrixM operator* (C Matrix3 &a, C MatrixM &b) {return MatrixM(a)*=b;} // get a*b
   friend MatrixM operator/ (C Matrix3 &a, C MatrixM &b) {return MatrixM(a)/=b;} // get a/b

   void     mul(C MatrixM &matrix, Matrix  &dest)C; // multiply self by 'matrix' and store result in 'dest'
   void     mul(C Matrix3 &matrix, MatrixM &dest)C; // multiply self by 'matrix' and store result in 'dest'
   void     mul(C Matrix  &matrix, MatrixM &dest)C; // multiply self by 'matrix' and store result in 'dest'
   void     mul(C MatrixM &matrix, MatrixM &dest)C; // multiply self by 'matrix' and store result in 'dest'
   MatrixM& mul(C Matrix3 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   MatrixM& mul(C Matrix  &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   MatrixM& mul(C MatrixM &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   void     div(C MatrixM &matrix, Matrix  &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void     div(C Matrix3 &matrix, MatrixM &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void     div(C Matrix  &matrix, MatrixM &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void     div(C MatrixM &matrix, MatrixM &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   MatrixM& div(C Matrix3 &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   MatrixM& div(C Matrix  &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   MatrixM& div(C MatrixM &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

   void     divNormalized(C Matrix3 &matrix, MatrixM &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void     divNormalized(C MatrixM &matrix, Matrix  &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void     divNormalized(C Matrix  &matrix, MatrixM &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void     divNormalized(C MatrixM &matrix, MatrixM &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   MatrixM& divNormalized(C Matrix3 &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   MatrixM& divNormalized(C Matrix  &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   MatrixM& divNormalized(C MatrixM &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

   void     inverse(MatrixM &dest, Bool normalized=false)C;                                   // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
   MatrixM& inverse(               Bool normalized=false) {inverse(T, normalized); return T;} // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

   void     inverseNonOrthogonal(MatrixM &dest)C;                                    // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
   MatrixM& inverseNonOrthogonal(             ) {inverseNonOrthogonal(T); return T;} // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

   MatrixM& normalize(            ) {super::normalize(     ); return T;} // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 'scale'
   MatrixM& normalize(  Flt  scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
   MatrixM& normalize(C Vec &scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

   MatrixM& move    (Dbl x, Dbl y, Dbl z) {pos   +=VecD(x, y, z); return T;} // move
   MatrixM& move    (C VecD2 &move      ) {pos.xy+=move         ; return T;} // move
   MatrixM& move    (C VecD  &move      ) {pos   +=move         ; return T;} // move
   MatrixM& moveBack(C VecD  &move      ) {pos   -=move         ; return T;} // move back

   MatrixM& anchor(C VecD &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

   MatrixM& scale   (  Flt  scale) {          T*=scale ; return T;} // scale
   MatrixM& scale   (C Vec &scale) {          T*=scale ; return T;} // scale
   MatrixM& scaleOrn(  Flt  scale) {super::scale(scale); return T;} // scale orientation only
   MatrixM& scaleOrn(C Vec &scale) {super::scale(scale); return T;} // scale orientation only

   MatrixM& rotateX (             Flt angle);                                    // rotate by x axis
   MatrixM& rotateY (             Flt angle);                                    // rotate by y axis
   MatrixM& rotateZ (             Flt angle);                                    // rotate by z axis
   MatrixM& rotateXY(  Flt  x   , Flt y    );                                    // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
   MatrixM& rotate  (C Vec &axis, Flt angle);                                    // rotate by vector, 'axis' must be normalized
   MatrixM& rotateXL(             Flt angle) {super::rotateXL(angle); return T;} // rotate matrix by its x vector (x-axis rotation in local space)
   MatrixM& rotateYL(             Flt angle) {super::rotateYL(angle); return T;} // rotate matrix by its y vector (y-axis rotation in local space)
   MatrixM& rotateZL(             Flt angle) {super::rotateZL(angle); return T;} // rotate matrix by its z vector (z-axis rotation in local space)

   MatrixM& mirror(C PlaneM &plane); // mirror matrix by plane

   // get
   Vec scale()C {return super::scale();} // get each axis scale

   Str asText(Int precision=INT_MAX)C {return super::asText(precision)+", Pos: "+pos.asText(precision);} // get text description

   // set (set methods reset the full matrix)
   MatrixM& identity(); // set as identity
   MatrixM& zero    (); // set all vectors to zero

   MatrixM& setPos     (Dbl x, Dbl y, Dbl z          ); // set as positioned identity
   MatrixM& setPos     (C VecD2 &pos                 ); // set as positioned identity
   MatrixM& setPos     (C VecD  &pos                 ); // set as positioned identity
   MatrixM& setScale   (  Flt    scale               ); // set as scaled     identity
   MatrixM& setScale   (C Vec   &scale               ); // set as scaled     identity
   MatrixM& setPosScale(C VecD  &pos  ,   Flt   scale); // set as positioned & scaled identity
   MatrixM& setPosScale(C VecD  &pos  , C Vec  &scale); // set as positioned & scaled identity
   MatrixM& setScalePos(  Flt    scale, C VecD &pos  ); // set as scaled & positioned identity
   MatrixM& setScalePos(C Vec   &scale, C VecD &pos  ); // set as scaled & positioned identity

   MatrixM& setRotateX (              Flt angle); // set as   x-rotated identity
   MatrixM& setRotateY (              Flt angle); // set as   y-rotated identity
   MatrixM& setRotateZ (              Flt angle); // set as   z-rotated identity
   MatrixM& setRotateXY(  Flt   x   , Flt y    ); // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
   MatrixM& setRotate  (C Vec  &axis, Flt angle); // set as     rotated by vector identity, 'axis' must be normalized

   MatrixM& setPosOrient(C VecD &pos,   DIR_ENUM dir                     ); // set as positioned orientation from DIR_ENUM
   MatrixM& setPosRight (C VecD &pos, C Vec &right                       ); // set as pos='pos', x='right'       and calculate correct y,z, 'right'        must be normalized
   MatrixM& setPosUp    (C VecD &pos, C Vec &up                          ); // set as pos='pos', y='up'          and calculate correct x,z, 'up'           must be normalized
   MatrixM& setPosDir   (C VecD &pos, C Vec &dir                         ); // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'          must be normalized
   MatrixM& setPosDir   (C VecD &pos, C Vec &dir, C Vec &up              ); // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir up'       must be normalized
   MatrixM& setPosDir   (C VecD &pos, C Vec &dir, C Vec &up, C Vec &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir up right' must be normalized

   MatrixM() {}
   MatrixM(  Flt      scale                  ) {setScale   (scale       );}
   MatrixM(C Vec     &pos                    ) {setPos     (pos         );}
   MatrixM(C VecD    &pos                    ) {setPos     (pos         );}
   MatrixM(C VecD    &pos  ,   Flt      scale) {setPosScale(pos  , scale);}
   MatrixM(  Flt      scale, C VecD    &pos  ) {setScalePos(scale, pos  );}
   MatrixM(C Vec     &scale, C VecD    &pos  ) {setScalePos(scale, pos  );}
   MatrixM(C Matrix3 &orn  , C VecD    &pos  ) {T.orn()=orn;    T.pos=pos;}
   MatrixM(C VecD    &pos  , C Matrix3 &orn  ) {T.orn()=orn;    T.pos=pos;}
   MatrixM(C Matrix3 &m);
   MatrixM(C Matrix  &m);
   MatrixM(C OrientM &o);
};extern MatrixM
   const MatrixMIdentity; // identity
/******************************************************************************/
STRUCT(MatrixD , MatrixD3) // Matrix 4x3 (orientation + scale + position, double precision)
//{
   VecD pos; // position

   MatrixD3& orn()  {return T;} // get reference to self as       'MatrixD3'
 C MatrixD3& orn()C {return T;} // get reference to self as const 'MatrixD3'

   // transform
   MatrixD& operator*=(  Dbl       f);
   MatrixD& operator/=(  Dbl       f);
   MatrixD& operator*=(C VecD     &v);
   MatrixD& operator/=(C VecD     &v);
   MatrixD& operator+=(C VecD     &v) {pos+=v; return T;}
   MatrixD& operator-=(C VecD     &v) {pos-=v; return T;}
   MatrixD& operator+=(C MatrixD  &m);
   MatrixD& operator-=(C MatrixD  &m);
   MatrixD& operator*=(C MatrixD3 &m) {return mul(m);}
   MatrixD& operator*=(C MatrixD  &m) {return mul(m);}
   MatrixD& operator/=(C MatrixD3 &m) {return div(m);}
   MatrixD& operator/=(C MatrixD  &m) {return div(m);}
   Bool     operator==(C MatrixD  &m)C;
   Bool     operator!=(C MatrixD  &m)C;

   friend MatrixD operator* (C MatrixD  &a, C MatrixD3 &b) {MatrixD temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixD operator* (C MatrixD  &a, C MatrixD  &b) {MatrixD temp; a.mul    (b, temp); return temp;} // get a*b
   friend MatrixD operator/ (C MatrixD  &a, C MatrixD3 &b) {MatrixD temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixD operator/ (C MatrixD  &a, C MatrixD  &b) {MatrixD temp; a.div    (b, temp); return temp;} // get a/b
   friend MatrixD operator~ (C MatrixD  &m               ) {MatrixD temp; m.inverse(   temp); return temp;} // get inversed 'm'
   friend MatrixD operator* (C MatrixD3 &a, C MatrixD  &b) {return MatrixD(a)*=b;} // get a*b
   friend MatrixD operator/ (C MatrixD3 &a, C MatrixD  &b) {return MatrixD(a)/=b;} // get a/b

   void     mul(C MatrixD3 &matrix, MatrixD &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   void     mul(C MatrixD  &matrix, MatrixD &dest)C;                           // multiply self by 'matrix' and store result in 'dest'
   MatrixD& mul(C MatrixD  &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'
   MatrixD& mul(C MatrixD3 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   void     div(C MatrixD3 &matrix, MatrixD &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   void     div(C MatrixD  &matrix, MatrixD &dest)C;                           // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
   MatrixD& div(C MatrixD3 &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
   MatrixD& div(C MatrixD  &matrix               ) {div(matrix, T); return T;} // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

   void     divNormalized(C MatrixD3 &matrix, MatrixD &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   void     divNormalized(C MatrixD  &matrix, MatrixD &dest)C;                                     // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
   MatrixD& divNormalized(C MatrixD3 &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
   MatrixD& divNormalized(C MatrixD  &matrix               ) {divNormalized(matrix, T); return T;} // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

   void     inverse(MatrixD &dest, Bool normalized=false)C;                                   // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
   MatrixD& inverse(               Bool normalized=false) {inverse(T, normalized); return T;} // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

   void     inverseNonOrthogonal(MatrixD &dest)C;                                    // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
   MatrixD& inverseNonOrthogonal(             ) {inverseNonOrthogonal(T); return T;} // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

   MatrixD& normalize(             ) {super::normalize(     ); return T;} // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 'scale'
   MatrixD& normalize(  Dbl   scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
   MatrixD& normalize(C VecD &scale) {super::normalize(scale); return T;} // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

   MatrixD& move    (Dbl x, Dbl y, Dbl z) {pos   +=VecD(x, y, z); return T;} // move
   MatrixD& move    (C VecD2 &move      ) {pos.xy+=move         ; return T;} // move
   MatrixD& move    (C VecD  &move      ) {pos   +=move         ; return T;} // move
   MatrixD& moveBack(C VecD  &move      ) {pos   -=move         ; return T;} // move back

   MatrixD& anchor(C VecD &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

   MatrixD& scale     (             Dbl   scale) {          T*=scale ; return T;} // scale
   MatrixD& scale     (           C VecD &scale) {          T*=scale ; return T;} // scale
   MatrixD& scaleOrn  (             Dbl   scale) {super::scale(scale); return T;} // scale orientation only
   MatrixD& scaleOrn  (           C VecD &scale) {super::scale(scale); return T;} // scale orientation only
   MatrixD& scale     (C VecD &dir, Dbl   scale);                                 // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
   MatrixD& scalePlane(C VecD &nrm, Dbl   scale);                                 // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

   MatrixD& rotateX (              Dbl angle);                                    // rotate by x axis
   MatrixD& rotateY (              Dbl angle);                                    // rotate by y axis
   MatrixD& rotateZ (              Dbl angle);                                    // rotate by z axis
   MatrixD& rotateXY(  Dbl   x   , Dbl y    );                                    // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
   MatrixD& rotate  (C VecD &axis, Dbl angle);                                    // rotate by vector, 'axis' must be normalized
   MatrixD& rotateXL(              Dbl angle) {super::rotateXL(angle); return T;} // rotate matrix by its x vector (x-axis rotation in local space)
   MatrixD& rotateYL(              Dbl angle) {super::rotateYL(angle); return T;} // rotate matrix by its y vector (y-axis rotation in local space)
   MatrixD& rotateZL(              Dbl angle) {super::rotateZL(angle); return T;} // rotate matrix by its z vector (z-axis rotation in local space)

   MatrixD& mirrorX(               ); // mirror matrix in X axis
   MatrixD& mirrorY(               ); // mirror matrix in Y axis
   MatrixD& mirrorZ(               ); // mirror matrix in Z axis
   MatrixD& mirror (C PlaneD &plane); // mirror matrix by   plane

   // set (set methods reset the full matrix)
   MatrixD& identity(); // set as identity
   MatrixD& zero    (); // set all vectors to zero

   MatrixD& setPos     (Dbl x, Dbl y, Dbl z          ); // set as positioned identity
   MatrixD& setPos     (C VecD2 &pos                 ); // set as positioned identity
   MatrixD& setPos     (C VecD  &pos                 ); // set as positioned identity
   MatrixD& setScale   (  Dbl    scale               ); // set as scaled     identity
   MatrixD& setScale   (C VecD  &scale               ); // set as scaled     identity
   MatrixD& setPosScale(C VecD  &pos  ,   Dbl   scale); // set as positioned & scaled identity
   MatrixD& setPosScale(C VecD  &pos  , C VecD &scale); // set as positioned & scaled identity
   MatrixD& setScalePos(  Dbl    scale, C VecD &pos  ); // set as scaled & positioned identity
   MatrixD& setScalePos(C VecD  &scale, C VecD &pos  ); // set as scaled & positioned identity

   MatrixD& setRotateX (              Dbl angle); // set as   x-rotated identity
   MatrixD& setRotateY (              Dbl angle); // set as   y-rotated identity
   MatrixD& setRotateZ (              Dbl angle); // set as   z-rotated identity
   MatrixD& setRotateXY(  Dbl   x   , Dbl y    ); // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
   MatrixD& setRotate  (C VecD &axis, Dbl angle); // set as     rotated by vector identity, 'axis' must be normalized

   MatrixD& setPosOrient(C VecD &pos,   DIR_ENUM dir                        ); // set as positioned orientation from DIR_ENUM
   MatrixD& setPosRight (C VecD &pos, C VecD &right                         ); // set as pos='pos', x='right'       and calculate correct y,z, 'right'        must be normalized
   MatrixD& setPosUp    (C VecD &pos, C VecD &up                            ); // set as pos='pos', y='up'          and calculate correct x,z, 'up'           must be normalized
   MatrixD& setPosDir   (C VecD &pos, C VecD &dir                           ); // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'          must be normalized
   MatrixD& setPosDir   (C VecD &pos, C VecD &dir, C VecD &up               ); // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir up'       must be normalized
   MatrixD& setPosDir   (C VecD &pos, C VecD &dir, C VecD &up, C VecD &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir up right' must be normalized

   // get
   VecD scale()C {return super::scale();} // get each axis scale

   Str asText(Int precision=INT_MAX)C {return super::asText(precision)+", Pos: "+pos.asText(precision);} // get text description

   // operations
#if EE_PRIVATE
   VecD2  convert(C VecD   &src, Bool normalized=false)C; // return converted 3D 'src' to 2D vector according to matrix x,y axes and position
   VecD   convert(C VecD2  &src                       )C; // return converted 2D 'src' to 3D vector according to matrix x,y axes and position
   EdgeD2 convert(C EdgeD  &src, Bool normalized=false)C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes and position
   EdgeD  convert(C EdgeD2 &src                       )C; // return converted 2D 'src' to 3D edge   according to matrix x,y axes and position
#endif

   MatrixD& setTransformAtPos(C VecD &pos, C MatrixD3 &matrix); // set as transformation at position
   MatrixD& setTransformAtPos(C VecD &pos, C MatrixD  &matrix); // set as transformation at position
   MatrixD&    transformAtPos(C VecD &pos, C MatrixD3 &matrix); //        transform      at position
   MatrixD&    transformAtPos(C VecD &pos, C MatrixD  &matrix); //        transform      at position

   // draw
   void draw(C Color &x_color=RED, C Color &y_color=GREEN, C Color &z_color=BLUE, Bool arrow=true)C {super::draw(pos, x_color, y_color, z_color, arrow);} // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   MatrixD() {}
   MatrixD(  Dbl       scale                   ) {setScale   (scale       );}
   MatrixD(C VecD     &pos                     ) {setPos     (pos         );}
   MatrixD(C VecD     &pos  ,   Dbl       scale) {setPosScale(pos  , scale);}
   MatrixD(  Dbl       scale, C VecD     &pos  ) {setScalePos(scale, pos  );}
   MatrixD(C VecD     &scale, C VecD     &pos  ) {setScalePos(scale, pos  );}
   MatrixD(C MatrixD3 &orn  , C VecD     &pos  ) {T.orn()=orn;    T.pos=pos;}
   MatrixD(C VecD     &pos  , C MatrixD3 &orn  ) {T.orn()=orn;    T.pos=pos;}
   MatrixD(C MatrixD3 &m);
   MatrixD(C Matrix   &m);
   MatrixD(C OrientP  &o);
};
/******************************************************************************/
struct Matrix4 // Matrix 4x4
{
   Vec4 x, y, z, pos;

#if EE_PRIVATE
   Flt determinant()C;
#endif

   // transform
   Matrix4& operator*=(C Matrix4 &m) {return mul(m);}

   friend Matrix4 operator* (C Matrix4 &a, C Matrix4 &b) {Matrix4 temp; a.mul(b, temp); return temp;} // get a*b
   friend Matrix4 operator* (C Matrix  &a, C Matrix4 &b) {Matrix4 temp; a.mul(b, temp); return temp;} // get a*b

   Matrix4& inverse  (); // inverse   self
   Matrix4& transpose(); // transpose self

   void     mul(C Matrix4 &matrix, Matrix4 &dest)C;                           // multiply self by 'matrix' and store result in 'dest' 
   Matrix4& mul(C Matrix4 &matrix               ) {mul(matrix, T); return T;} // multiply self by 'matrix'

   // set (set methods reset the full matrix)
   Matrix4& identity(); // set as identity
   Matrix4& zero    (); // set all vectors to zero
   
   Matrix4() {}
   Matrix4(C Matrix3 &m);
   Matrix4(C Matrix  &m);
};
#if EE_PRIVATE
extern Matrix4 ProjMatrix; // Projection Matrix
extern Flt     ProjMatrixEyeOffset[2];
extern Vec2    PixelOffset;
#endif
/******************************************************************************/
#if EE_PRIVATE
struct GpuMatrix // GPU Matrix (transposed Matrix)
{
   Flt xx, yx, zx, _x,
       xy, yy, zy, _y,
       xz, yz, zz, _z;

   GpuMatrix& fromMul(C Matrix  &a, C Matrix  &b); // set from "a*b"
   GpuMatrix& fromMul(C Matrix  &a, C MatrixM &b); // set from "a*b"
   GpuMatrix& fromMul(C MatrixM &a, C MatrixM &b); // set from "a*b"

   GpuMatrix() {}
   GpuMatrix(C Matrix  &m);
   GpuMatrix(C MatrixM &m);
};
#endif
struct RevMatrix3 : Matrix3 // Reverse Matrix
{
};
struct RevMatrix : Matrix // Reverse Matrix
{
   RevMatrix3& orn()  {return (RevMatrix3&)T;} // get reference to self as       'RevMatrix3'
 C RevMatrix3& orn()C {return (RevMatrix3&)T;} // get reference to self as const 'RevMatrix3'
};
/******************************************************************************/
#if EE_PRIVATE
extern Int Matrixes; // number of active matrixes
#endif
/******************************************************************************/
Bool Equal(C Matrix3  &a, C Matrix3  &b, Flt eps=EPS                   );
Bool Equal(C MatrixD3 &a, C MatrixD3 &b, Dbl eps=EPSD                  );
Bool Equal(C Matrix   &a, C Matrix   &b, Flt eps=EPS , Flt pos_eps=EPS );
Bool Equal(C MatrixM  &a, C MatrixM  &b, Flt eps=EPS , Dbl pos_eps=EPSD);
Bool Equal(C MatrixD  &a, C MatrixD  &b, Dbl eps=EPSD, Dbl pos_eps=EPSD);
Bool Equal(C Matrix4  &a, C Matrix4  &b, Flt eps=EPS                   );

void     GetTransform(Matrix3  &transform, C Orient   &start, C Orient   &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void     GetTransform(Matrix3  &transform, C Matrix3  &start, C Matrix3  &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void     GetTransform(MatrixD3 &transform, C MatrixD3 &start, C MatrixD3 &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void     GetTransform(Matrix   &transform, C Matrix   &start, C Matrix   &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void     GetTransform(MatrixD  &transform, C MatrixD  &start, C MatrixD  &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix3  GetTransform(                     C Orient   &start, C Orient   &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix3  GetTransform(                     C Matrix3  &start, C Matrix3  &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
MatrixD3 GetTransform(                     C MatrixD3 &start, C MatrixD3 &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix   GetTransform(                     C Matrix   &start, C Matrix   &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
MatrixD  GetTransform(                     C MatrixD  &start, C MatrixD  &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"

inline void GetTransform          (RevMatrix &transform, C Matrix &start, C Matrix &result) {result.div          (start, transform);} // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
inline void GetTransformNormalized(RevMatrix &transform, C Matrix &start, C Matrix &result) {result.divNormalized(start, transform);} // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result", this function assumes that 'start' and 'result' are normalized

void GetDelta(Vec &pos, Vec &angle, C Matrix  &from, C Matrix  &to); // get position and angle axis delta from 'from' and 'to' matrixes !! matrixes must be normalized !!
void GetDelta(Vec &pos, Vec &angle, C MatrixM &from, C MatrixM &to); // get position and angle axis delta from 'from' and 'to' matrixes !! matrixes must be normalized !!

void GetVel(Vec &vel, Vec &ang_vel, C Matrix  &from, C Matrix  &to, Flt dt=Time.d()); // get linear velocity and angular velocity from 'from' and 'to' matrixes using 'dt' time delta !! matrixes must be normalized !!
void GetVel(Vec &vel, Vec &ang_vel, C MatrixM &from, C MatrixM &to, Flt dt=Time.d()); // get linear velocity and angular velocity from 'from' and 'to' matrixes using 'dt' time delta !! matrixes must be normalized !!

Flt GetLodDist2(C Vec &lod_center, C Matrix  &matrix); // calculate squared distance from 'lod_center' transformed by 'matrix' to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods
Flt GetLodDist2(C Vec &lod_center, C MatrixM &matrix); // calculate squared distance from 'lod_center' transformed by 'matrix' to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods

#if EE_PRIVATE
void InitMatrix();

void SetMatrixCount(Int num=1); // set number of used matrixes and velocities
void SetFurVelCount(Int num=1); // set number of used fur velocities

void SetFastViewMatrix(C Matrix  &view_matrix); // set view   matrix
void SetFastMatrix    (                      ); // set object matrix to 'MatrixIdentity'
void SetFastMatrix    (C Matrix  &matrix     ); // set object matrix
void SetFastMatrix    (C MatrixM &matrix     ); // set object matrix

void SetFastVel   (Byte i, C Vec &vel           ); // set i-th object         velocity
void SetFastVel   (        C Vec &vel           ); // set      object         velocity
void SetFastVel   (                             ); // set      object         velocity to 'VecZero'
void SetFastAngVel(        C Vec &ang_vel_shader); // set      object angular velocity
void SetFastAngVel(                             ); // set      object angular velocity to 'VecZero'

void SetOneMatrix(                 ); // set object matrix to 'MatrixIdentity' and number of used matrixes to 1
void SetOneMatrix(C Matrix  &matrix); // set object matrix                     and number of used matrixes to 1
void SetOneMatrix(C MatrixM &matrix); // set object matrix                     and number of used matrixes to 1

void SetVelFur(C Matrix3 &view_matrix, C Vec &vel); // set velocity for fur effect

void SetProjMatrix();
void SetProjMatrix(Flt proj_offset);

void SetMatrixVelRestore();
void SetMatrixVelSplit  (Byte *matrix, Int matrixes);

void SetMatrixFurVelRestore();
void SetMatrixFurVelSplit  (Byte *matrix, Int matrixes);

void SetAngVelShader(Vec &ang_vel_shader, C Vec &ang_vel, C Matrix3 &matrix);
#endif

void SetMatrix(C Matrix  &matrix=MatrixIdentity                     ); // set active object rendering matrix and clear velocities to zero
void SetMatrix(C MatrixM &matrix                                    ); // set active object rendering matrix and clear velocities to zero
void SetMatrix(C Matrix  &matrix, C Vec &vel, C Vec &ang_vel=VecZero); // set active object rendering matrix and       velocities
void SetMatrix(C MatrixM &matrix, C Vec &vel, C Vec &ang_vel=VecZero); // set active object rendering matrix and       velocities
/******************************************************************************/
