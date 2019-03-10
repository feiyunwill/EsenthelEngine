/******************************************************************************

   Use 'PhysBody' as a physical body description, to create 'Actors' from it.

/******************************************************************************/
enum PHYS_TYPE : Byte // Physical Body Type
{
   PHYS_NONE  , // none
   PHYS_SHAPE , // shape
   PHYS_CONVEX, // convex
   PHYS_MESH  , // mesh, actor created from this type can be only static
};
/******************************************************************************/
#if EE_PRIVATE
struct PhysMesh
{
   PHYS_TYPE   _type;
   Int         _used_by;
   Box         _box;
   MeshBase   *_base;
   Mems<Byte>  _physx_cooked_data;
   Ptr        _bullet_cooked_data; UInt _bullet_cooked_data_size;
   PHYS_API(PxConvexMesh  , btConvexHullShape         ) *_convex;
   PHYS_API(PxTriangleMesh, btBvhTriangleMeshShape    ) *_mesh  ;
   PHYS_API(void          , btTriangleIndexVertexArray) *_tvia  ;

#if !PHYSX
   Flt volume()C;
#endif

   void zero();
   void del ();

   void freeHelperData();
   Bool adjustStorage (Bool universal, Bool physx, Bool bullet, Bool *changed=null); // adjust the type of storage for the physical body, 'universal'=can be used for both PhysX and Bullet, 'physx'=can be used for PhysX (and when used there it is faster than 'universal'), 'bullet'=can be used for Bullet (and when used there it is faster than 'universal'), each storage uses additional space, for PhysX only games it is suggested to set 'physx' to true and others to false, for Bullet only games it is suggested to set 'bullet' to true and others to false, please note if you call this method under engine compiled with Bullet library, you won't be able to use any PhysX information (which means converting from or to PhysX storage), 'changed'=pointer to custom bool which will be set to true if any change was performed on the physical body (false otherwise), false on fail

   Bool cookConvex(MeshBase *src, Bool mesh_is_already_convex ); // cook 'src' into 'physx_cooked_data'
   Bool cookMesh  (MeshBase *src, Bool keep_face_indexes=false); // cook 'src' into 'physx_cooked_data'

   void setPhysMesh();
   Bool      setBox();

   Bool createConvexTry(MeshBase &mshb, Bool mesh_is_already_convex ); // create as convex body, false on fail, if 'mesh_is_already_convex' is set to true then the mesh is assumed to be already convex which will make the creation faster, this method can sometimes fail if the mesh is too complex, in that case you can create a temporary mesh using 'MeshBase.createConvex', specify vertex limit and use that mesh for phys body creation
   Bool createMeshTry  (MeshBase &mshb, Bool keep_face_indexes=false); // create as static mesh, false on fail

   void draw(C Color &color)C;

             ~PhysMesh() {del ();}
              PhysMesh() {zero();}
              PhysMesh(C PhysMesh &src)=delete;
   PhysMesh& operator=(C PhysMesh &src); // create from 'src'
};
#endif
/******************************************************************************/
struct PhysPart // Physical Body Part, contains a single Shape/Convex/Mesh, can be used to create actors from it
{
   Flt   density; // mass density multiplier, 0..Inf
   Shape shape  ; // geometric shape

   // manage
   PhysPart& del   (                                 );                                          // delete
   PhysPart& create(C Box     &box    , Flt density=1) {return create(Shape(box    ), density);} // create as box     with 'density'
   PhysPart& create(C OBox    &obox   , Flt density=1) {return create(Shape(obox   ), density);} // create as obox    with 'density'
   PhysPart& create(C Extent  &ext    , Flt density=1) {return create(Shape(ext    ), density);} // create as ext     with 'density'
   PhysPart& create(C Ball    &ball   , Flt density=1) {return create(Shape(ball   ), density);} // create as ball    with 'density'
   PhysPart& create(C Capsule &capsule, Flt density=1) {return create(Shape(capsule), density);} // create as capsule with 'density'
   PhysPart& create(C Tube    &tube   , Flt density=1) {return create(Shape(tube   ), density);} // create as tube    with 'density'
   PhysPart& create(C Shape   &shape  , Flt density=1);                                          // create as shape   with 'density'

   Bool      createConvexTry(MeshBase &mshb, Flt density=1, Bool mesh_is_already_convex=false                              ); // create as convex body with 'density', false on fail, if 'mesh_is_already_convex' is set to true then the mesh is assumed to be already convex which will make the creation faster, this method can sometimes fail if the mesh is too complex, in that case you can create a temporary mesh using 'MeshBase.createConvex', specify vertex limit and use that mesh for phys body creation
   Bool      createConvexTry(MeshLod  &mshl, Flt density=1, Bool mesh_is_already_convex=false, Bool skip_hidden_parts=false); // create as convex body with 'density', false on fail, if 'mesh_is_already_convex' is set to true then the mesh is assumed to be already convex which will make the creation faster, this method can sometimes fail if the mesh is too complex, in that case you can create a temporary mesh using 'MeshBase.createConvex', specify vertex limit and use that mesh for phys body creation, 'skip_hidden_parts'=if ignore MeshPart's with MSHP_HIDDEN flag
   PhysPart& createConvex   (MeshBase &mshb, Flt density=1, Bool mesh_is_already_convex=false                              ); // create as convex body with 'density', Exit  on fail, if 'mesh_is_already_convex' is set to true then the mesh is assumed to be already convex which will make the creation faster, this method can sometimes fail if the mesh is too complex, in that case you can create a temporary mesh using 'MeshBase.createConvex', specify vertex limit and use that mesh for phys body creation
   PhysPart& createConvex   (MeshLod  &mshl, Flt density=1, Bool mesh_is_already_convex=false, Bool skip_hidden_parts=false); // create as convex body with 'density', Exit  on fail, if 'mesh_is_already_convex' is set to true then the mesh is assumed to be already convex which will make the creation faster, this method can sometimes fail if the mesh is too complex, in that case you can create a temporary mesh using 'MeshBase.createConvex', specify vertex limit and use that mesh for phys body creation, 'skip_hidden_parts'=if ignore MeshPart's with MSHP_HIDDEN flag

   Bool      createMeshTry(MeshBase &mshb, Bool keep_face_indexes=false                              ); // create as static mesh, 'keep_face_indexes'=if preserve original face indexes in the 'PhysHit.face' (when enabled this will use additional memory, when disabled the face index will point to the internal face of the physical body but not the original face of the source mesh), false on fail
   Bool      createMeshTry(MeshLod  &mshl, Bool keep_face_indexes=false, Bool skip_hidden_parts=false); // create as static mesh, 'keep_face_indexes'=if preserve original face indexes in the 'PhysHit.face' (when enabled this will use additional memory, when disabled the face index will point to the internal face of the physical body but not the original face of the source mesh), false on fail, 'skip_hidden_parts'=if ignore MeshPart's with MSHP_HIDDEN flag
   PhysPart& createMesh   (MeshBase &mshb, Bool keep_face_indexes=false                              ); // create as static mesh, 'keep_face_indexes'=if preserve original face indexes in the 'PhysHit.face' (when enabled this will use additional memory, when disabled the face index will point to the internal face of the physical body but not the original face of the source mesh), Exit  on fail
   PhysPart& createMesh   (MeshLod  &mshl, Bool keep_face_indexes=false, Bool skip_hidden_parts=false); // create as static mesh, 'keep_face_indexes'=if preserve original face indexes in the 'PhysHit.face' (when enabled this will use additional memory, when disabled the face index will point to the internal face of the physical body but not the original face of the source mesh), Exit  on fail, 'skip_hidden_parts'=if ignore MeshPart's with MSHP_HIDDEN flag

   // get
   Bool      is    (        )C {return _type!=PHYS_NONE;} // if  part body has any data
   PHYS_TYPE type  (        )C {return _type           ;} // get part body type
   Flt       area  (        )C;                           // get surface area, this method works only for PHYS_SHAPE types
   Flt       volume(        )C;                           // get volume      , this method works only for PHYS_SHAPE types
   Flt       mass  (        )C {return density*volume();} // get mass        , this method works only for PHYS_SHAPE types
   Bool      getBox(Box &box)C;                           // get bounding box surrounding the body, false on fail

   // operations
   Bool      adjustStorage (Bool universal, Bool physx, Bool bullet, Bool *changed=null); // adjust the type of storage for the physical body, 'universal'=can be used for both PhysX and Bullet, 'physx'=can be used for PhysX (and when used there it is faster than 'universal'), 'bullet'=can be used for Bullet (and when used there it is faster than 'universal'), each storage uses additional space, for PhysX only games it is suggested to set 'physx' to true and others to false, for Bullet only games it is suggested to set 'bullet' to true and others to false, please note if you call this method under engine compiled with Bullet library, you won't be able to create PhysX data, 'changed'=pointer to custom bool which will be set to true if any change was performed on the physical body (false otherwise), false on fail
   PhysPart& freeHelperData(                                                           ); // this free's up the helper data of the physical body, which increases available memory, however it disables saving the body to file, or converting it to 'MeshBase'

   PhysPart& transform (C Matrix3 &matrix);                            // transform by matrix
   PhysPart& transform (C Matrix  &matrix);                            // transform by matrix
   PhysPart& operator*=(C Matrix3 &matrix) {return transform(matrix);} // transform by matrix
   PhysPart& operator*=(C Matrix  &matrix) {return transform(matrix);} // transform by matrix

#if EE_PRIVATE
   PhysPart& mirrorX(); // mirror in X axis
   PhysPart& mirrorY(); // mirror in Y axis
   PhysPart& mirrorZ(); // mirror in Z axis

   void setPhysMesh();
#endif

   Bool ray(C Vec &pos, C Vec &move, C Matrix *body_matrix, PhysHitBasic *phys_hit, Bool two_sided=false)C; // if ray cuts with body, 'pos'=ray start position, 'move'=ray movement vector, 'body_matrix'=optional matrix of the physical body, 'phys_hit'=optionally pass pointer to 'PhysHitBasic' class to receive additional data about the nearest contact, 'two_sided'=if mesh faces are two sided

   // draw
   void draw(C Color &color)C; // this can be optionally called outside of Render function

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f   )C; // save, false on fail
   Bool load     (  File &f   ) ; // load, false on fail
#if EE_PRIVATE
   Bool saveData(File &f)C; // save, false on fail
   Bool loadData(File &f) ; // load, false on fail
#endif

            PhysPart& operator=(C PhysPart &src); // create from 'src'
           ~PhysPart() {del();}
            PhysPart();
   explicit PhysPart(C Box     &box    , Flt density=1); // create box
   explicit PhysPart(C OBox    &obox   , Flt density=1); // create obox
   explicit PhysPart(C Extent  &ext    , Flt density=1); // create ext
   explicit PhysPart(C Ball    &ball   , Flt density=1); // create ball
   explicit PhysPart(C Capsule &capsule, Flt density=1); // create capsule
   explicit PhysPart(C Tube    &tube   , Flt density=1); // create tube
   explicit PhysPart(C Shape   &shape  , Flt density=1); // create shape

#if EE_PRIVATE
   void zero();
#endif

#if !EE_PRIVATE
private:
#endif
   PHYS_TYPE _type;
#if EE_PRIVATE
   PhysMesh *_pm;
#else
   Ptr       _pm;
#endif
   PhysPart(C PhysPart &src)=delete;
};
/******************************************************************************/
struct PhysBody // Physical Body (array of PhysPart's), can be used to create actors from it
{
   Flt            density ; // mass density multiplier, 0..Inf, default=1
   Box            box     ; // bounding box surrounding the body
   PhysMtrl      *material; // physical material              , default=null
   Mems<PhysPart> parts   ; // physical body parts

   // manage
   PhysBody& del(); // delete

   // get
   Bool is          ()C {return parts.elms()>0;} // if has any data
   Flt  area        ()C; // get surface area of all parts, this method works only for PHYS_SHAPE types
   Flt  volume      ()C; // get volume       of all parts, this method works only for PHYS_SHAPE types
   Flt  mass        ()C; // get mass         of all parts, this method works only for PHYS_SHAPE types
   Flt  finalDensity()C; // get density of PhysBody including material -> "density * material.density"

   // operations
   Bool      adjustStorage (Bool universal, Bool physx, Bool bullet, Bool *changed=null); // adjust the type of storage for the physical body, 'universal'=can be used for both PhysX and Bullet, 'physx'=can be used for PhysX (and when used there it is faster than 'universal'), 'bullet'=can be used for Bullet (and when used there it is faster than 'universal'), each storage uses additional space, for PhysX only games it is suggested to set 'physx' to true and others to false, for Bullet only games it is suggested to set 'bullet' to true and others to false, please note if you call this method under engine compiled with Bullet library, you won't be able to use any PhysX information (which means converting from or to PhysX storage), 'changed'=pointer to custom bool which will be set to true if any change was performed on the physical body (false otherwise), false on fail
   Bool      setBox        (                                                           ); // recalculate bounding box, false on fail
   PhysBody& freeHelperData(                                                           ); // this free's up the helper data of the physical body, which increases available memory, however it disables saving the body to file, or converting it to 'MeshBase'

   PhysBody& transform (C Matrix3 &matrix);                            // transform by matrix
   PhysBody& transform (C Matrix  &matrix);                            // transform by matrix
   PhysBody& operator*=(C Matrix3 &matrix) {return transform(matrix);} // transform by matrix
   PhysBody& operator*=(C Matrix  &matrix) {return transform(matrix);} // transform by matrix

#if EE_PRIVATE
   PhysBody& mirrorX(); // mirror in X axis
   PhysBody& mirrorY(); // mirror in Y axis
   PhysBody& mirrorZ(); // mirror in Z axis
#endif

   // draw
   void draw(C Color &color)C; // this can be optionally called outside of Render function

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail

   Bool save     (File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load     (File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadAdd  (File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail, this method adds the data from file to self (not replaces it)
#if EE_PRIVATE
   Bool saveData(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadData(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   void zero();
#endif

   PhysBody();
};
/******************************************************************************/
DECLARE_CACHE(PhysBody, PhysBodies, PhysBodyPtr); // 'PhysBodies' cache storing 'PhysBody' objects which can be accessed by 'PhysBodyPtr' pointer

inline Int Elms(C PhysBody &phys) {return phys.parts.elms();}

#if EE_PRIVATE
void IncRef(PhysMesh* &ptr);
void DecRef(PhysMesh* &ptr);
#if !PHYSX
void IncRef(btCollisionShape* shape);
void DecRef(btCollisionShape* shape);
#endif
#endif
/******************************************************************************/
