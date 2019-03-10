/******************************************************************************

   Use 'Shape' to store custom kind of shape.

/******************************************************************************/
enum SHAPE_TYPE : Byte // Shape Type
{
   SHAPE_NONE   , // none
   SHAPE_POINT  , // point
   SHAPE_EDGE   , // edge
   SHAPE_RECT   , // rectangle
   SHAPE_BOX    , // box
   SHAPE_OBOX   , // oriented box
   SHAPE_CIRCLE , // circle
   SHAPE_BALL   , // ball
   SHAPE_CAPSULE, // capsule
   SHAPE_TUBE   , // tube
   SHAPE_TORUS  , // torus
   SHAPE_CONE   , // cone
   SHAPE_PYRAMID, // pyramid
   SHAPE_PLANE  , // plane
   SHAPE_NUM    , // number of shape types
};
/******************************************************************************/
struct Shape // General Shape 
{
   SHAPE_TYPE type; // shape type
   union
   {
      Vec     point  ;
      Edge    edge   ;
      Rect    rect   ;
      Box     box    ;
      OBox    obox   ;
      Circle  circle ;
      Ball    ball   ;
      Capsule capsule;
      Tube    tube   ;
      Torus   torus  ;
      Cone    cone   ;
      Pyramid pyramid;
      Plane   plane  ;
   };

   Shape& operator+=(C Vec     &v);
   Shape& operator-=(C Vec     &v);
   Shape& operator*=(  Flt      f);
   Shape& operator/=(  Flt      f);
   Shape& operator*=(C Vec     &v);
   Shape& operator/=(C Vec     &v);
   Shape& operator*=(C Matrix3 &m);
   Shape& operator*=(C Matrix  &m);

   friend Shape operator+ (C Shape &shape, C Vec     &v);
   friend Shape operator- (C Shape &shape, C Vec     &v);
   friend Shape operator* (C Shape &shape,   Flt      f);
   friend Shape operator/ (C Shape &shape,   Flt      f);
   friend Shape operator* (C Shape &shape, C Vec     &v);
   friend Shape operator/ (C Shape &shape, C Vec     &v);
   friend Shape operator* (C Shape &shape, C Matrix3 &m);
   friend Shape operator* (C Shape &shape, C Matrix  &m);

   // get / set
   Flt    area          (          )C; // get surface area
   Flt    volume        (          )C; // get volume
   Vec    pos           (          )C; // get position
   void   pos           (C Vec &pos) ; // set position
   Matrix asMatrix      (          )C; // try to return the shape as a matrix, where matrix position is the shape position, and matrix orientation is the shape orientation, matrix scale is identity
   Matrix asMatrixScaled(          )C; // try to return the shape as a matrix, where matrix position is the shape position, and matrix orientation is the shape orientation, matrix scale depends on shape size
   Str    asText        (Bool include_shape_type_name)C; // get shape text description

   // operations
   Shape& extend(Flt e); // extend

   Shape& move     (C Vec     &v) {return T+=v;}
   Shape& scale    (  Flt      f) {return T*=f;}
   Shape& scale    (C Vec     &v) {return T*=v;}
   Shape& transform(C Matrix3 &m) {return T*=m;}
   Shape& transform(C Matrix  &m) {return T*=m;}

   Shape& mirrorX(); // mirror in X axis
   Shape& mirrorY(); // mirror in Y axis
   Shape& mirrorZ(); // mirror in Z axis

   // draw
   void draw(C Color &color=WHITE, Bool fill=false)C; // this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   // io
   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

   Shape(                  ) {type=SHAPE_NONE   ;                   }
   Shape(C Vec     &point  ) {type=SHAPE_POINT  ; T.point  =point  ;}
   Shape(C Edge    &edge   ) {type=SHAPE_EDGE   ; T.edge   =edge   ;}
   Shape(C Rect    &rect   ) {type=SHAPE_RECT   ; T.rect   =rect   ;}
   Shape(C Box     &box    ) {type=SHAPE_BOX    ; T.box    =box    ;}
   Shape(C OBox    &obox   ) {type=SHAPE_OBOX   ; T.obox   =obox   ;}
   Shape(C Extent  &ext    ) {type=SHAPE_BOX    ; T.box    =ext    ;}
   Shape(C Circle  &circle ) {type=SHAPE_CIRCLE ; T.circle =circle ;}
   Shape(C Ball    &ball   ) {type=SHAPE_BALL   ; T.ball   =ball   ;}
   Shape(C Capsule &capsule) {type=SHAPE_CAPSULE; T.capsule=capsule;}
   Shape(C Tube    &tube   ) {type=SHAPE_TUBE   ; T.tube   =tube   ;}
   Shape(C Torus   &torus  ) {type=SHAPE_TORUS  ; T.torus  =torus  ;}
   Shape(C Cone    &cone   ) {type=SHAPE_CONE   ; T.cone   =cone   ;}
   Shape(C Pyramid &pyramid) {type=SHAPE_PYRAMID; T.pyramid=pyramid;}
   Shape(C Plane   &plane  ) {type=SHAPE_PLANE  ; T.plane  =plane  ;}
};
/******************************************************************************/
// shape type
#if EE_PRIVATE
Bool   ShapeType2D   (SHAPE_TYPE type); // if shape type is 2-dimensional
Bool   ShapeTypeRound(SHAPE_TYPE type); // if shape type has round elements
#endif
CChar* ShapeTypeName(SHAPE_TYPE type); // get shape type name from 'type'

// sweep
Bool SweepPointShape(C Vec &point, C Vec &move, C Shape &shape, Flt *hit_frac=null, Vec *hit_normal=null); // if moving point cuts through a static shape

// distance
Flt  Dist(C Shape &a, C Shape &b); // get the distance between 'a' and 'b' shape, warning: not all 'a' 'b' shape type combinations are supported in this function, to check if a certain combination is supported please check the headers for 'a' and 'b' shape types, and check if there is a separate 'Dist' function of 'a' type and 'b' type

// cuts
Bool Cuts(C Vec2  &p, C Shape &s); // if point 'p' cuts 's'
Bool Cuts(C Vec   &p, C Shape &s); // if point 'p' cuts 's'
Bool Cuts(C Shape &a, C Shape &b); // if shape 'a' cuts 'b', warning: not all 'a' 'b' shape type combinations are supported in this function, to check if a certain combination is supported please check the headers for 'a' and 'b' shape types, and check if there is a separate 'Cuts' function of 'a' type and 'b' type
/******************************************************************************/
