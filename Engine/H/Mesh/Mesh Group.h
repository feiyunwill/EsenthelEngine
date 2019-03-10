/******************************************************************************

   Use 'MeshGroup' to store a group of 'Mesh' objects.

/******************************************************************************/
struct MeshGroup // Mesh Group (array of Meshes)
{
   Extent     ext   ; // bounding box
   Mems<Mesh> meshes; // meshes

   // manage
   MeshGroup& del   (                                  ); // delete
   MeshGroup& create(C MeshGroup &src, UInt flag_and=~0); // create from 'src', 'flag_and'=MESH_BASE_FLAG

   MeshGroup& create(C Mesh &src, C VecI &cells); // create from 'src' partitioned into "cells.x * cells.y * cells.z" meshes

#if EE_PRIVATE
   MeshGroup& create(C Mesh &src, C Boxes &boxes); // create from 'src' and boxes

   void copyParams(C MeshGroup &src); // copy only parameters without meshes

   MeshGroup& include (UInt flag); // include   elements specified with 'flag' MESH_BASE_FLAG
#endif
   MeshGroup& exclude (UInt flag); // exclude   elements specified with 'flag' MESH_BASE_FLAG
   MeshGroup& keepOnly(UInt flag); // keep only elements specified with 'flag' MESH_BASE_FLAG

   // get
   Bool is       ()C {return meshes.elms()>0;} // if has any meshes
   UInt flag     ()C; // get MESH_BASE_FLAG
   Int  vtxs     ()C; // get total number of vertexes
   Int  edges    ()C; // get total number of edges
   Int  tris     ()C; // get total number of triangles
   Int  quads    ()C; // get total number of quads
   Int  faces    ()C; // get total number of faces                    , faces    =(triangles + quads  )
   Int  trisTotal()C; // get total number of triangles including quads, trisTotal=(triangles + quads*2)

   // join
   MeshGroup& join   (Int i0, Int i1                                                                                      ); // join i0-th and i1-th Mesh together
   MeshGroup& joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag=0, Flt weld_pos_eps=EPS); // join all MeshParts, 'test_material'=join only those MeshParts which have the same material, 'test_draw_group'=join only those MeshParts which have the same draw group, 'test_name'=join only those MeshParts which have the same name, 'test_vtx_flag'=join only those MeshParts which have same vertex flag, 'weld_pos_eps'=epsilon used for welding vertexes after joining (use <0 to disable welding), this does not join 'meshes' together, this only joins MeshParts within the 'meshes'

   // transform
   MeshGroup& move     (              C Vec &move); //           move MeshGroup
   MeshGroup& scale    (C Vec &scale             ); // scale          MeshGroup
   MeshGroup& scaleMove(C Vec &scale, C Vec &move); // scale and move MeshGroup

   // texture transform
   MeshGroup& texMove  (C Vec2 &move , Byte tex_index=0); // move   texture UV's
   MeshGroup& texScale (C Vec2 &scale, Byte tex_index=0); // scale  texture UV's
   MeshGroup& texRotate(  Flt   angle, Byte tex_index=0); // rotate texture UV's

#if EE_PRIVATE
   // texturize
   MeshGroup& texMap(  Flt     scale=1, Byte tex_index=0); // map texture UV's according to vertex XY position and scale
   MeshGroup& texMap(C Matrix &matrix , Byte tex_index=0); // map texture UV's according to matrix
   MeshGroup& texMap(C Plane  &plane  , Byte tex_index=0); // map texture UV's according to plane
   MeshGroup& texMap(C Ball   &ball   , Byte tex_index=0); // map texture UV's according to ball
   MeshGroup& texMap(C Tube   &tube   , Byte tex_index=0); // map texture UV's according to tube
#endif

   // set
   MeshGroup& delBase       (                                                     ); // delete all software meshes (MeshBase  ) in this mesh
   MeshGroup& delRender     (                                                     ); // delete all hardware meshes (MeshRender) in this mesh
   MeshGroup& setRender     (                                                     ); // set rendering version, convert all MeshBase's to MeshRender's
   MeshGroup& setVtxDup     (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex duplicates (vtx.dup)
   MeshGroup& setNormals    (                                                     ); // recalculate vertex            3D normals
   MeshGroup& setFaceNormals(                                                     ); // recalculate triangle and quad 3D normals
   MeshGroup& setShader     (                                                     ); // reset shader
   MeshGroup& material      (C MaterialPtr &material                              ); // set material, 'material' must point to object in constant memory address (mesh will store only the pointer to the material and later use it if needed)
   Bool       setBox        (Bool           set_mesh_boxes                        ); // recalculate bounding box

   // operations
#if EE_PRIVATE
   MeshGroup& weldVtx2D    (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 2D vertexes     , this function will weld vertexes together if they share the same position (ignoring Z), 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
#endif
   MeshGroup& weldVtx      (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 3D vertexes     , this function will weld vertexes together if they share the same position             , 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
   MeshGroup& weldVtxValues(UInt flag  , Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld    vertex values, this function will weld values of vertexes which  share the same position             , 'flag'=                                 elements to weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)

   MeshGroup& freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // convert
   MeshGroup& triToQuad(Flt cos=EPS_COL_COS); // triangles to quads    , 'cos'=minimum cosine between 2      triangle normals to weld  them into 1 quad (0..1)
   MeshGroup& quadToTri(Flt cos=2          ); // quads     to triangles, 'cos'=minimum cosine between 2 quad triangle normals to leave them as   1 quad (0..1, value >1 converts all quads into triangles)

#if EE_PRIVATE
   // fix
   MeshGroup& fixTexWrapping(Byte tex_index=0); // fix texture wrapping, fixes texture coordinates created by spherical/tube mapping
   MeshGroup& fixTexOffset  (Byte tex_index=0); // fix texture offset  , tries to reduce big texture coordinates to small ones
#endif

   // add / remove
   MeshGroup& remove(Int i, Bool set_box=true); // remove i-th mesh, 'set_box'=if recalculate bounding box

   // optimize
#if EE_PRIVATE
   MeshGroup& sortByMaterials  (); // sort MeshParts according to their materials
#endif
   MeshGroup& removeDegenerateFaces(Flt eps=EPS);

   MeshGroup& simplify(Flt intensity, Flt max_distance=1.0f, Flt max_uv=1.0f, Flt max_color=0.02f, Flt max_material=0.02f, Flt max_skin=1, Flt max_normal=PI, Bool keep_border=false, MESH_SIMPLIFY mode=SIMPLIFY_QUADRIC, Flt pos_eps=EPS, MeshGroup *dest=null, Bool *stop=null); // simplify mesh by removing vertexes/faces, 'intensity'=how much to simplify (0..1, 0=no simplification, 1=full simplification), 'max_distance'=max distance between elements to merge them (0..Inf), 'max_uv'=max allowed vertex texture UV deviations (0..1), 'max_color'=max allowed vertex color deviations (0..1), 'max_material'=max allowed vertex material deviations (0..1), 'max_skin'=max allowed vertex skin deviations (0..1), 'max_normal'=max allowed vertex normal angle deviations (0..PI), 'keep_border'=if always keep border edges (edges that have faces only on one side), 'pos_eps'=vertex position epsilon, 'dest'=destination MeshBase (if set to null then the mesh will simplify itself), 'stop'=set to 'true' on secondary thread to stop this method, returns dest

   MeshGroup& weldCoplanarFaces(Flt cos_face=EPS_COL_COS, Flt cos_vtx=-1, Bool safe=true, Flt max_face_length=-1); // weld coplanar faces, 'cos_face'=minimum cosine between face normals, 'cos_vtx'=minimum cosine between vertex normals, 'safe'=if process only faces without neighbors, 'max_face_length'=max allowed face length (-1=no limit)

   // draw
   #if EE_PRIVATE
      // helper drawing
      void draw2D       (C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r=0.04f, Flt side_width=0.01f                                             )C; // draw 2D
      void drawNormals2D(  Flt    length   , C Color &edge_color, C Color &vtx_color=TRANSPARENT                                                                         )C; // draw 2D normals
      void drawNormals  (  Flt    length   , C Color &face_color, C Color &vtx_color=TRANSPARENT, C Color &tangent_color=TRANSPARENT, C Color &binormal_color=TRANSPARENT)C; // draw 3D normals
   #endif
      // default drawing, automatically uses Frustum Culling, this doesn't draw the mesh immediately, instead it adds the mesh to a draw list
      void draw(C MatrixM &matrix)C; // add mesh to draw list using 'matrix' matrix and no velocities, this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND

      void drawShadow(C MatrixM &matrix)C; // add mesh to shadow draw list using 'matrix' matrix, this should be called only in RM_SHADOW

   // io
   void operator=(C Str &name) ; // load, Exit  on fail
   Bool save     (C Str &name)C; // save, false on fail
   Bool load     (C Str &name) ; // load, false on fail

   Bool save   (File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load   (File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadAdd(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail, this method adds the data from file to self (not replaces it)
#if EE_PRIVATE
   Bool saveData(File     &f, CChar *path=null)C; // save binary, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadData(File     &f, CChar *path=null) ; // load binary, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool saveTxt (FileText &f, CChar *path=null)C; // save text  , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadTxt (FileText &f, CChar *path=null) ; // load text  , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool saveTxt (C Str    &name               )C; // save text  , false on fail
   Bool loadTxt (C Str    &name               ) ; // load text  , false on fail

   void zero();
#endif

   MeshGroup();
};
/******************************************************************************/
inline Int Elms(C MeshGroup &mshg) {return mshg.meshes.elms();}
/******************************************************************************/
#if EE_PRIVATE
void ShutMesh();
#endif
/******************************************************************************/
