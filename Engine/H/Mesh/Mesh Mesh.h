/******************************************************************************

   'Mesh' is a typical object mesh,
      it is an array of multiple level of details - 'MeshLods',
      it has a one bounding box.

   Every 'Mesh' object can be linked with custom 'Skeleton' file.
      Once a Mesh is being loaded, it automatically loads the linked Skeleton file,
      and adjusts vertex bone indexes to match the Skeleton bone indexes order (this is done according to bone names),
      making it possible to draw the mesh with Skeleton based matrixes.

   Use 'Meshes' cache for easy mesh accessing from files.

/******************************************************************************/
STRUCT(Mesh , MeshLod) // Mesh (array of Mesh Lod's)
//{
   Extent ext       ; // bounding box
   Vec    lod_center; // mesh center position used for calculating lod level, usually equal to "ext.pos"

   Int      lods(     )C {return     _lods.elms()+1                  ;} // get number of Level of Details including self
   MeshLod& lod (Int i)  {return i ? _lods[i-1] :                  T ;} // return  i-th  Level of Detail
 C MeshLod& lod (Int i)C {return i ? _lods[i-1] : SCAST(C MeshLod, T);} // return  i-th  Level of Detail

   Skeleton*  skeleton(                                      )C {return _skeleton;} // get Skeleton linked with this Mesh
   Mesh&      skeleton(Skeleton *skeleton, Bool by_name=false);                     // link Mesh with specified Skeleton file, avoid calling this realtime as it requires adjusting the vertex skinning information (bone indexes) and re-creating the hardware mesh version, 'by_name'=if remap by bone name only and ignore type/indexes
   Mesh& clearSkeleton(                                      );                     // clear Mesh link to skeleton and remove all information related to vertex bone mapping

   Enum*   drawGroupEnum(                                       )C {return _draw_group_enum;} // get enum used for specifying draw groups in all parts for this Mesh, you should pass this value to 'MeshPart.drawGroup' method
   Mesh&   drawGroupEnum(Enum *e, Bool reset_when_not_found=true);                            // set enum used for specifying draw groups in all parts for this Mesh, 'reset_when_not_found'=if existing mesh part draw groups are not found, then their draw masks will be reset for "reset_when_not_found==true" and kept for "reset_when_not_found==false"
   Bool hasDrawGroup    ( Int draw_group_index                  )C;                           // check if at least one MeshPart has specified draw group enum index
   Bool hasDrawGroupMask(UInt draw_group_mask                   )C;                           // check if at least one MeshPart has specified draw group enum mask

   // manage
   Mesh& del   (                                          ); // delete manually
   Mesh& create(  Int        parts                        ); // create with 'parts' empty MeshParts
   Mesh& create(C Mesh      &src        , UInt flag_and=~0); // create from 'src', 'flag_and'=MESH_BASE_FLAG
   Mesh& create(C MeshGroup &src        , UInt flag_and=~0); // create from 'src', 'flag_and'=MESH_BASE_FLAG
   Mesh& create(C Mesh *mesh, Int meshes, UInt flag_and=~0); // create from mesh array, 'flag_and'=MESH_BASE_FLAG

   void copyParams(C Mesh &src); // copy only parameters without meshes

#if EE_PRIVATE
   void zero();

   Mesh& include (UInt flag); // include   elements specified with 'flag' MESH_BASE_FLAG
#endif
   Mesh& exclude (UInt flag); // exclude   elements specified with 'flag' MESH_BASE_FLAG
   Mesh& keepOnly(UInt flag); // keep only elements specified with 'flag' MESH_BASE_FLAG

   // get
   UInt memUsage()C; // get memory usage

 C MeshLod& getDrawLod (C Matrix  &matrix)C; // get          Level of Detail which should be used for drawing with current camera and given object 'matrix'
 C MeshLod& getDrawLod (C MatrixM &matrix)C; // get          Level of Detail which should be used for drawing with current camera and given object 'matrix'
 C MeshLod& getDrawLod (  Flt      dist2 )C; // get          Level of Detail which should be used for drawing with current camera and given object 'dist2' squared distance to camera which can be calculated using 'GetLodDist2' function
   Int      getDrawLodI(C Matrix  &matrix)C; // get index of Level of Detail which should be used for drawing with current camera and given object 'matrix'
   Int      getDrawLodI(C MatrixM &matrix)C; // get index of Level of Detail which should be used for drawing with current camera and given object 'matrix'
   Int      getDrawLodI(  Flt      dist2 )C; // get index of Level of Detail which should be used for drawing with current camera and given object 'dist2' squared distance to camera which can be calculated using 'GetLodDist2' function
#if EE_PRIVATE
   Flt lodQuality(Int i, Int base=0)C; // get quality of Level of Detail comparing i-th to base level, returning value in range of 0..1 where 1=full quality, 0=zero quality
#endif

   // set
#if EE_PRIVATE
   Mesh& setEdgeNormals(Bool flag=false); // recalculate edge 2D normals, 'flag'=if include ETQ_FLAG behavior
   Mesh& setNormals2D  (Bool flag=false); // recalculate edge and vertex 2D normals, 'flag'=if include ETQ_FLAG behavior
#endif
   Mesh& setNormals    (); // recalculate vertex            3D normals
   Mesh& setTangents   (); // recalculate vertex            3D tangents
   Mesh& setBinormals  (); // recalculate vertex            3D binormals
   Mesh& setFaceNormals(); // recalculate triangle and quad 3D normals
   Mesh& setAutoTanBin (); // automatically calculate vertex tangents and binormals if needed, if they're not needed then they will be removed
   Bool  setBox        (Bool skip_hidden_parts=true); // recalculate bounding box, 'skip_hidden_parts'=if MeshParts with MSHP_HIDDEN should not be included in the box, returns false on fail

#if EE_PRIVATE
   Mesh& setVtxColorAlphaAsTesselationIntensity(Bool tesselate_edges                                 ); // set vertex color alpha   (vtx.color.a) as tesselation intensity, 'tesselate_edges'=if tesselate non continuous edges
   Mesh& setVtxDup2D                           (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 2D duplicates (vtx.dup)
#endif
   Mesh& setVtxDup     (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 3D duplicates (vtx.dup)
   Mesh& setAdjacencies(Bool faces=true, Bool edges=false                    ); // set adjacencies, 'faces'=if set face adjacencies ('tri.adjFace', 'quad.adjFace'), 'edges'=if set edges ('edge') and edge adjacencies ('tri.adjEdge', 'quad.adjEdge', 'edge.adjFace')

   Mesh& delBase  (                       ); // delete all software meshes (MeshBase  ) in this mesh
   Mesh& delRender(                       ); // delete all hardware meshes (MeshRender) in this mesh
   Mesh& setBase  (Bool only_if_empty=true); // set software  version, convert 'MeshRender' to 'MeshBase', 'only_if_empty'=perform conversion only if the MeshBase is empty (if set to false then conversion is always performed)
   Mesh& setRender(Bool optimize     =true); // set rendering version, convert 'MeshBase'   to 'MeshRender', 'optimize'=if optimize the mesh by re-ordering the vertexes/triangles for optimal processing on the GPU
   Mesh& setShader(                       ); // reset shader
   Mesh& material (C MaterialPtr &material); // set material, 'material' must point to object in constant memory address (mesh will store only the pointer to the material and later use it if needed), avoid changing materials real-time during rendering, instead consider using material variations (set them once, and later select which one to use with 'SetVariation' function)

   // join / split
   Mesh& join   (Int i0, Int i1                                                                , Flt weld_pos_eps=EPS); // join i0-th and i1-th parts together, 'weld_pos_eps'=epsilon used for welding vertexes after joining (use <0 to disable welding)
   Mesh& joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag=0, Flt weld_pos_eps=EPS); // join all parts, 'test_material'=join only those MeshParts which have the same material, 'test_draw_group'=join only those MeshParts which have the same draw group, 'test_name'=join only those MeshParts which have the same name, 'test_vtx_flag'=join only those MeshParts which have same vertex flag, 'weld_pos_eps'=epsilon used for welding vertexes after joining (use <0 to disable welding)

   // transform
   Mesh& move         (              C Vec &move        ); //           move
   Mesh& scale        (C Vec &scale                     ); // scale
   Mesh& scaleMove    (C Vec &scale, C Vec &move        ); // scale and move
   Mesh& scaleMoveBase(C Vec &scale, C Vec &move        ); // scale and move (including the 'MeshBase' but without 'MeshRender')
   Mesh& setSize      (C Box &box                       ); // scale and move to fit box
   Mesh& transform    (C Matrix3              &matrix   ); // transform by matrix
   Mesh& transform    (C Matrix               &matrix   ); // transform by matrix
   Mesh& animate      (C MemPtrN<Matrix, 256> &matrixes ); // animate   by matrixes
   Mesh& animate      (C AnimatedSkeleton     &anim_skel); // animate   by skeleton
   Mesh& mirrorX      (                                 ); // mirror in X axis
   Mesh& mirrorY      (                                 ); // mirror in Y axis
   Mesh& mirrorZ      (                                 ); // mirror in Z axis
   Mesh& reverse      (                                 ); // reverse faces
#if EE_PRIVATE
   Mesh& rightToLeft  (                                 ); // convert from right hand to left hand coordinate system
#endif

   // texture transform
   Mesh& texMove  (C Vec2 &move , Byte tex_index=0); // move   texture UV's
   Mesh& texScale (C Vec2 &scale, Byte tex_index=0); // scale  texture UV's
   Mesh& texRotate(  Flt   angle, Byte tex_index=0); // rotate texture UV's

#if EE_PRIVATE
   // texturize
   Mesh& texMap(  Flt     scale=1, Byte tex_index=0); // map texture UV's according to vertex XY position and scale
   Mesh& texMap(C Matrix &matrix , Byte tex_index=0); // map texture UV's according to matrix
   Mesh& texMap(C Plane  &plane  , Byte tex_index=0); // map texture UV's according to plane
   Mesh& texMap(C Ball   &ball   , Byte tex_index=0); // map texture UV's according to ball
   Mesh& texMap(C Tube   &tube   , Byte tex_index=0); // map texture UV's according to tube
#endif

   // operations
#if EE_PRIVATE
   Mesh& weldEdge     (); // weld edges
   Mesh& weldVtx2D    (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 2D vertexes     , this function will weld vertexes together if they share the same position (ignoring Z), 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
#endif
   Mesh& weldVtx      (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 3D vertexes     , this function will weld vertexes together if they share the same position             , 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
   Mesh& weldVtxValues(UInt flag  , Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld    vertex values, this function will weld values of vertexes which  share the same position             , 'flag'=                                 elements to weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)

   Mesh& tesselate(); // smooth subdivide faces, preserving original vertexes
   Mesh& subdivide(); // smooth subdivide faces,  smoothing original vertexes

   Int   boneFind (CChar8 *bone_name                                     )C; // find bone by its name and return its index, -1 on fail
   Mesh& boneRemap(C MemPtr<Byte, 256> &old_to_new, Bool remap_names=true) ; // remap vertex bone/matrix indexes according to bone 'old_to_new' remap, 'remap_names'=if remap the bone names as well
   void      setUsedBones(Bool (&bones)[256])C;
   void  includeUsedBones(Bool (&bones)[256])C;

   Mesh& setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps=EPS, Int rays=1024, MESH_AO_FUNC func=MAF_FULL, Threads *threads=null); // calculate per-vertex ambient occlusion in vertex colors, 'strength'=0..1 AO strength, 'bias'=0..1, 'max'=AO limit 0..1, 'ray_length'=max ray distance to test, 'rays'=number of rays to use for AO calculation, 'func'=falloff function

   Mesh& freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // variations
   Mesh& variations      (Int variations              );   Int     variations   (            )C; // set/get number of material variations (meshes always have at least 1 material variation)
   Mesh& variationName   (Int variation, C Str8 &name );   CChar8* variationName(Int     i   )C; // set/get           material variation name (first material variation always has name = null and it can't be changed)
                                                           UInt    variationID  (Int     i   )C; //     get           material variation ID   (first material variation always has ID   = 0    and it can't be changed, other variation ID's are always assigned to a random number when the variation is being created, so that each variation has a unique ID)
                                                           Int     variationFind(CChar8 *name)C; //    find index  of material variation which name is equal to 'name', -1 is returned if variation was not found
                                                           Int     variationFind(UInt    id  )C; //    find index  of material variation which ID   is equal to 'id'  , -1 is returned if variation was not found
   Mesh& variationKeep   (Int variation               );                                         // keep   specified 'variation' and remove all others
   Mesh& variationRemove (Int variation               );                                         // remove specified 'variation'
   Mesh& variationMove   (Int variation, Int new_index);                                         //   move specified 'variation' to a 'new_index'
   Mesh& variationInclude(C Mesh &src                 );                                         // include variation names from 'src' if they don't exist in this mesh yet, this does not setup existing mesh part variation materials

   // fix
   Mesh& fixTexOffset  (Byte tex_index=0); // fix texture offset  , this reduces big texture coordinates to small ones increasing texturing quality on low precision video cards
   Mesh& fixTexWrapping(Byte tex_index=0); // fix texture wrapping, fixes texture coordinates created by spherical/tube mapping (this can add new vertexes to the mesh)

   // convert
   Mesh& edgeToDepth(Bool tex_align =true       ); // edges to depth (extrude 2D edges to 3D faces)
   Mesh& edgeToTri  (Bool set_id    =false      ); // edges to triangles (triangulation)
   Mesh& triToQuad  (Flt  cos       =EPS_COL_COS); // triangles to quads    , 'cos'=minimum cosine between 2      triangle normals to weld  them into 1 quad (0..1)
   Mesh& quadToTri  (Flt  cos       =2          ); // quads     to triangles, 'cos'=minimum cosine between 2 quad triangle normals to leave them as   1 quad (0..1, value >1 converts all quads into triangles)

   // add / remove
   Mesh& add   (C MeshBase &src         ); // add MeshBase to self
   Mesh& add   (C MeshPart &src         ); // add MeshPart to self
   Mesh& add   (C Mesh     &src         ); // add Mesh     to self
   Mesh& remove(Int i, Bool set_box=true); // remove i-th MeshPart, 'set_box'=if recalculate bounding box

   MeshLod&    newLod(     ); // add empty   MeshLod
   MeshLod&    newLod(Int i); // add empty   MeshLod at i lod index
   Mesh   & removeLod(Int i); // remove i-th MeshLod
   Mesh   &   setLods(Int n); // set n levels of MeshLod's, new lods will be empty

   // optimize
#if EE_PRIVATE
   Mesh& sortByMaterials  (); // sort MeshParts according to their materials
   Mesh& removeDoubleEdges();
   Mesh& removeSingleFaces(Flt fraction                                              ); // remove fraction of single faces (single triangles or quads not linked to any other face), 'fraction'=0..1
   Mesh& weldInlineEdges  (Flt cos_edge=EPS_COL_COS, Flt cos_vtx=-1, Bool z_test=true); // weld inline edge vertexes, 'cos_edge'=minimum cosine between edge normals, 'cos_vtx'=minimum cosine between vertex normals, 'z_test'=if perform tests for inline 'z' vertex component
#endif
   Mesh& removeDegenerateFaces(Flt eps=EPS);
   Bool  removeUnusedVtxs     (Bool include_edge_references=true, Bool set_box=true); // remove vertexes which aren't used by any face or edge, if 'include_edge_references' is set to false then only face references are tested (without the edges), 'set_box'=if recalculate bounding box upon vertex removal, returns true if any vertex was removed

   Mesh& simplify(Flt intensity, Flt max_distance=1.0f, Flt max_uv=1.0f, Flt max_color=0.02f, Flt max_material=0.02f, Flt max_skin=1, Flt max_normal=PI, Bool keep_border=false, MESH_SIMPLIFY mode=SIMPLIFY_QUADRIC, Flt pos_eps=EPS, Mesh *dest=null, Bool *stop=null); // simplify mesh by removing vertexes/faces, 'intensity'=how much to simplify (0..1, 0=no simplification, 1=full simplification), 'max_distance'=max distance between elements to merge them (0..Inf), 'max_uv'=max allowed vertex texture UV deviations (0..1), 'max_color'=max allowed vertex color deviations (0..1), 'max_material'=max allowed vertex material deviations (0..1), 'max_skin'=max allowed vertex skin deviations (0..1), 'max_normal'=max allowed vertex normal angle deviations (0..PI), 'keep_border'=if always keep border edges (edges that have faces only on one side), 'pos_eps'=vertex position epsilon, 'dest'=destination MeshBase (if set to null then the mesh will simplify itself), 'stop'=set to 'true' on secondary thread to stop this method, returns dest

   Mesh& weldCoplanarFaces(Flt cos_face=EPS_COL_COS, Flt cos_vtx=-1, Bool safe=true, Flt max_face_length=-1); // weld coplanar faces, 'cos_face'=minimum cosine between face normals, 'cos_vtx'=minimum cosine between vertex normals, 'safe'=if process only faces without neighbors, 'max_face_length'=max allowed face length (-1=no limit)

   // draw
      // default drawing, doesn't use automatic Frustum Culling, this doesn't draw the mesh immediately, instead it adds the mesh to a draw list
      void draw(C Matrix           &matrix, C Vec &vel, C Vec &ang_vel=VecZero)C; // add mesh to draw list using 'matrix'    matrix, 'vel'   velocity and 'ang_vel' angular velocity, this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C MatrixM          &matrix, C Vec &vel, C Vec &ang_vel=VecZero)C; // add mesh to draw list using 'matrix'    matrix, 'vel'   velocity and 'ang_vel' angular velocity, this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C Matrix           &matrix                                    )C; // add mesh to draw list using 'matrix'    matrix   and no velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C MatrixM          &matrix                                    )C; // add mesh to draw list using 'matrix'    matrix   and no velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C AnimatedSkeleton &anim_skel                                 )C; // add mesh to draw list using 'anim_skel' matrixes and    velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed:            RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering)
      void draw(C AnimatedSkeleton &anim_skel, C Material &material           )C; // add mesh to draw list using 'anim_skel' matrixes and    velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed:            RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering), 'material'=material used for rendering which overrides the default material, however for performance reasons, the default shader is used, which means that the 'material' should be similar to the default material, and if it's too different then some artifacts can occur

      void drawShadow(C Matrix           &matrix                         )C; // add mesh to shadow draw list using 'matrix'    matrix  , this should be called only in RM_SHADOW
      void drawShadow(C MatrixM          &matrix                         )C; // add mesh to shadow draw list using 'matrix'    matrix  , this should be called only in RM_SHADOW
      void drawShadow(C AnimatedSkeleton &anim_skel                      )C; // add mesh to shadow draw list using 'anim_skel' skeleton, this should be called only in RM_SHADOW, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering)
      void drawShadow(C AnimatedSkeleton &anim_skel, C Material &material)C; // add mesh to shadow draw list using 'anim_skel' skeleton, this should be called only in RM_SHADOW, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering), 'material'=material used for rendering which overrides the default material, however for performance reasons, the default shader is used, which means that the 'material' should be similar to the default material, and if it's too different then some artifacts can occur

      // draw blended, this is an alternative to default 'draw' (typically 'draw' draws blended meshes automatically for materials with technique in blend mode), this method however draws the mesh immediately (which allows to set custom shader parameters per draw call), it always uses blend shaders regardless if the material has technique set in blend mode, and provides additional control over material color, this can be called only in RM_BLEND rendering mode, doesn't use automatic Frustum culling
      void drawBlend(                                                                        C Vec4 *color=null)C; // draw with  current    matrix  ,                                                'color'=pointer to optional Material color multiplication
      void drawBlend(C MatrixM          &matrix, C Vec &vel=VecZero, C Vec &ang_vel=VecZero, C Vec4 *color=null)C; // draw with 'matrix'    matrix  , 'vel' velocity and 'ang_vel' angular velocity, 'color'=pointer to optional Material color multiplication
      void drawBlend(C AnimatedSkeleton &anim_skel,                                          C Vec4 *color=null)C; // draw with 'anim_skel' matrixes,                                                'color'=pointer to optional Material color multiplication

      // draw mesh outline, this can be optionally called in RM_OUTLINE in order to outline the mesh, doesn't use automatic Frustum culling
      void drawOutline(C Color &color                               )C; // draw with  current    matrix
      void drawOutline(C Color &color, C MatrixM          &matrix   )C; // draw with 'matrix'    matrix
      void drawOutline(C Color &color, C AnimatedSkeleton &anim_skel)C; // draw with 'anim_skel' matrixes

      // draw using the "behind" effect, this can be optionally called in RM_BEHIND, doesn't use automatic Frustum culling, 'color_perp'=color to be used for normals perpendicular to camera, 'color_parallel'=color to be used for normals parallel to camera
      void drawBehind(C Color &color_perp, C Color &color_parallel                               )C; // draw with  current    matrix
      void drawBehind(C Color &color_perp, C Color &color_parallel, C MatrixM          &matrix   )C; // draw with 'matrix'    matrix
      void drawBehind(C Color &color_perp, C Color &color_parallel, C AnimatedSkeleton &anim_skel)C; // draw with 'anim_skel' matrixes

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load     (  File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#if EE_PRIVATE
   Bool saveTxt (C Str    &name               )C; // save text  , false on fail
   Bool loadTxt (C Str    &name               ) ; // load text  , false on fail
   Bool saveTxt (FileText &f, CChar *path=null)C; // save text  , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadTxt (FileText &f, CChar *path=null) ; // load text  , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool saveData(File     &f, CChar *path=null)C; // save binary, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadData(File     &f, CChar *path=null) ; // load binary, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#endif

   void operator*=(C Matrix3  &m   ) {transform(m   );} // transform by matrix
   void operator*=(C Matrix   &m   ) {transform(m   );} // transform by matrix
   void operator+=(C MeshBase &mshb) {add      (mshb);} // add MeshBase
   void operator+=(C MeshPart &part) {add      (part);} // add MeshPart
   void operator+=(C Mesh     &mesh) {add      (mesh);} // add Mesh

  ~Mesh() {del();}
   Mesh();

#if !EE_PRIVATE
private:
#endif
   struct Variations
   {
   #if EE_PRIVATE
      Bool is()C {return _variations>0;} // if has any data

      Int alloc(Int variations, Int name_size);

      Int    nameSize (     )C; // get size needed for variation names
      Char8* nameStart(     )C; // get start of the variation name memory
     CChar8* name     (Int i)C; // get i-th variation name
      UInt   id       (Int i)C; // get i-th variation id

      Bool save(File &f)C;
      Bool load(File &f) ;

      struct Variation
      {
         UInt id;
         Int  name_offset;
      };
      Variation *_variation; // right after '_variation' array, array of variation names is allocated (in Char8 mode)
   #else
      Ptr _variation;
   #endif
      Int _variations;

          void          del();
         ~Variations() {del();}
          Variations() {_variation=null; _variations=0;}
          Variations(C Variations &src);
      void operator=(C Variations &src);
   };

   Skeleton     *_skeleton;
   Enum         *_draw_group_enum;
   Mems<MeshLod> _lods;
   BoneMap       _bone_map;
   Variations    _variations;
};
/******************************************************************************/
DECLARE_CACHE(Mesh, Meshes, MeshPtr); // 'Meshes' cache storing 'Mesh' objects which can be accessed by 'MeshPtr' pointer

inline Int Elms(C Mesh &mesh) {return mesh.parts.elms();}

#if EE_PRIVATE
void InitMesh();
#endif
/******************************************************************************/
