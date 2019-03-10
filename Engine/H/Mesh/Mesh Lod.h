/******************************************************************************

   'MeshLod' represents a single level of detail, by containing an array of 'MeshPart's.

/******************************************************************************/
struct MeshLod // Level of Detail, array of Mesh Part's
{
   Flt            dist2; // squared distance at which this level of detail should be rendered when using 90 deg FOV, negative version means that this LOD should be removed
   Mems<MeshPart> parts; // mesh parts

   // manage
   MeshLod& del   (                                ); // delete
   MeshLod& create(  Int      num                  ); // create with 'num' empty MeshParts
   MeshLod& create(C MeshLod &src, UInt flag_and=~0); // create from 'src', 'flag_and'=MESH_BASE_FLAG

   void copyParams(C MeshLod &src); // copy only parameters without meshes

#if EE_PRIVATE
   void zero();

   void scaleParams(Flt scale);

   MeshLod& include (UInt flag); // include   elements specified with 'flag' MESH_BASE_FLAG
#endif
   MeshLod& exclude (UInt flag); // exclude   elements specified with 'flag' MESH_BASE_FLAG
   MeshLod& keepOnly(UInt flag); // keep only elements specified with 'flag' MESH_BASE_FLAG

   // get
   Bool is       (                                     )C {return parts.elms()>0;}    // if  has any parts
   UInt flag     (                                     )C;                            // get MESH_BASE_FLAG
   UInt memUsage (                                     )C;                            // get memory usage
   Int  vtxs     (                                     )C;                            // get total number of vertexes
   Int  edges    (                                     )C;                            // get total number of edges
   Int  tris     (                                     )C;                            // get total number of triangles
   Int  quads    (                                     )C;                            // get total number of quads
   Int  faces    (                                     )C;                            // get total number of faces                    , faces    =(triangles + quads  )
   Int  trisTotal(                                     )C;                            // get total number of triangles including quads, trisTotal=(triangles + quads*2)
   Bool getBox   (Box &box, Bool skip_hidden_parts=true)C;                            // get box   encapsulating the MeshLod, 'skip_hidden_parts'=if MeshParts with MSHP_HIDDEN should not be included in the box, returns false on fail (if no vertexes are present)
   Flt  area     (Vec *center=null                     )C;                            // get surface area of all mesh faces, 'center'=if specified then it will be calculated as the average surface center
   Flt  dist     (                                     )C;   MeshLod& dist(Flt dist); // get/set LOD distance

   Bool hasDrawGroup    ( Int draw_group_index)C; // check if at least one MeshPart has specified draw group enum index
   Bool hasDrawGroupMask(UInt draw_group_mask )C; // check if at least one MeshPart has specified draw group enum mask

   // set
#if EE_PRIVATE
   MeshLod& setEdgeNormals(Bool flag=false); // recalculate edge 2D normals, 'flag'=if include ETQ_FLAG behavior
   MeshLod& setNormals2D  (Bool flag=false); // recalculate edge and vertex 2D normals, 'flag'=if include ETQ_FLAG behavior
#endif
   MeshLod& setNormals    (); // recalculate vertex            3D normals
   MeshLod& setTangents   (); // recalculate vertex            3D tangents
   MeshLod& setBinormals  (); // recalculate vertex            3D binormals
   MeshLod& setFaceNormals(); // recalculate triangle and quad 3D normals
   MeshLod& setAutoTanBin (); // automatically calculate vertex tangents and binormals if needed, if they're not needed then they will be removed

#if EE_PRIVATE
   MeshLod& setVtxColorAlphaAsTesselationIntensity(Bool tesselate_edges                                 ); // set vertex color alpha   (vtx.color.a) as tesselation intensity, 'tesselate_edges'=if tesselate non continuous edges
   MeshLod& setVtxDup2D                           (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 2D duplicates (vtx.dup)
#endif
   MeshLod& setVtxDup     (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 3D duplicates (vtx.dup)
   MeshLod& setAdjacencies(Bool faces=true, Bool edges=false                    ); // set adjacencies, 'faces'=if set face adjacencies ('tri.adjFace', 'quad.adjFace'), 'edges'=if set edges ('edge') and edge adjacencies ('tri.adjEdge', 'quad.adjEdge', 'edge.adjFace')

   MeshLod& delBase  (                                        ); // delete all software meshes (MeshBase  ) in this mesh
   MeshLod& delRender(                                        ); // delete all hardware meshes (MeshRender) in this mesh
#if EE_PRIVATE
   MeshLod& setRenderSS(                                      ); // set rendering version of stencil shadow
#endif
   MeshLod& setBase  (Bool only_if_empty=true                 ); // set software  version, convert 'MeshRender' to 'MeshBase', 'only_if_empty'=perform conversion only if the MeshBase is empty (if set to false then conversion is always performed)
   MeshLod& setRender(Bool optimize     =true, Int lod_index=0); // set rendering version, convert 'MeshBase'   to 'MeshRender', 'optimize'=if optimize the mesh by re-ordering the vertexes/triangles for optimal processing on the GPU, 'lod_index'=index of the LOD in the mesh (used to determine quality of the shader)
   MeshLod& setShader(                         Int lod_index=0); // reset shader, 'lod_index'=index of the LOD in the mesh (used to determine quality of the shader)
   MeshLod& material (C MaterialPtr &material, Int lod_index=0); // set material, 'lod_index'=index of the LOD in the mesh (used to determine quality of the shader, if it's <0 then shader will not be reset), 'material' must point to object in constant memory address (mesh will store only the pointer to the material and later use it if needed)

   // transform
   MeshLod& move         (              C Vec &move        ); //           move
   MeshLod& scale        (C Vec &scale                     ); // scale
   MeshLod& scaleMove    (C Vec &scale, C Vec &move        ); // scale and move
   MeshLod& scaleMoveBase(C Vec &scale, C Vec &move        ); // scale and move (including the 'MeshBase' but without 'MeshRender')
   MeshLod& transform    (C Matrix3              &matrix   ); // transform by matrix
   MeshLod& transform    (C Matrix               &matrix   ); // transform by matrix
   MeshLod& animate      (C MemPtrN<Matrix, 256> &matrixes ); // animate   by matrixes
   MeshLod& animate      (C AnimatedSkeleton     &anim_skel); // animate   by skeleton
   MeshLod& mirrorX      (                                 ); // mirror in X axis
   MeshLod& mirrorY      (                                 ); // mirror in Y axis
   MeshLod& mirrorZ      (                                 ); // mirror in Z axis
   MeshLod& reverse      (                                 ); // reverse faces
#if EE_PRIVATE
   MeshLod& rightToLeft  (                                 ); // convert from right hand to left hand coordinate system
#endif

#if EE_PRIVATE
   // texturize
   MeshLod& texMap(  Flt     scale=1, Byte tex_index=0); // map texture UV's according to vertex XY position and scale
   MeshLod& texMap(C Matrix &matrix , Byte tex_index=0); // map texture UV's according to matrix
   MeshLod& texMap(C Plane  &plane  , Byte tex_index=0); // map texture UV's according to plane
   MeshLod& texMap(C Ball   &ball   , Byte tex_index=0); // map texture UV's according to ball
   MeshLod& texMap(C Tube   &tube   , Byte tex_index=0); // map texture UV's according to tube
#endif

   // texture transform
   MeshLod& texMove  (C Vec2 &move , Byte tex_index=0); // move   texture UV's
   MeshLod& texScale (C Vec2 &scale, Byte tex_index=0); // scale  texture UV's
   MeshLod& texRotate(  Flt   angle, Byte tex_index=0); // rotate texture UV's

   // join / split
   MeshLod& join   (Int i0, Int i1                                                                , Flt weld_pos_eps=EPS); // join i0-th and i1-th parts together, 'weld_pos_eps'=epsilon used for welding vertexes after joining (use <0 to disable welding)
   MeshLod& joinAll(Bool test_material, Bool test_draw_group, Bool test_name, UInt test_vtx_flag=0, Flt weld_pos_eps=EPS); // join all parts, 'test_material'=join only those MeshParts which have the same material, 'test_draw_group'=join only those MeshParts which have the same draw group, 'test_name'=join only those MeshParts which have the same name, 'test_vtx_flag'=join only those MeshParts which have same vertex flag, 'weld_pos_eps'=epsilon used for welding vertexes after joining (use <0 to disable welding)

#if EE_PRIVATE
   MeshPart* splitVtxs (Int i, C MemPtr<Bool> &vtx_is                                                  ); // split i-th part by given 'is' array of vertexes to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created
   MeshPart* splitFaces(Int i, C MemPtr<Bool> &edge_is, C MemPtr<Bool> &tri_is, C MemPtr<Bool> &quad_is); // split i-th part by given 'is' array of faces    to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created
   MeshPart* splitBone (Int i, Int bone, C Skeleton *skeleton=null); // split i-th part by bone blend index to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created
#endif
   MeshPart* splitVtxs (Int i, C MemPtr<Int> &vtxs                                            ); // split i-th part by given array of vertexes to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created
   MeshPart* splitFaces(Int i, C MemPtr<Int> &faces                                           ); // split i-th part by given array of faces    to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created, here 'faces' indexes can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"
   MeshPart* splitFaces(Int i, C MemPtr<Int> &edges, C MemPtr<Int> &tris, C MemPtr<Int> &quads); // split i-th part by given array of faces    to a new MeshPart, pointer to that MeshPart is returned or null if it wasn't created

   // operations
#if EE_PRIVATE
   MeshLod& weldEdge     (); // weld edges
   MeshLod& weldVtx2D    (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 2D vertexes     , this function will weld vertexes together if they share the same position (ignoring Z), 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
#endif
   MeshLod& weldVtx      (UInt flag=0, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 3D vertexes     , this function will weld vertexes together if they share the same position             , 'flag'=if selected elements aren't equal then don't weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
   MeshLod& weldVtxValues(UInt flag  , Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld    vertex values, this function will weld values of vertexes which  share the same position             , 'flag'=                                 elements to weld (MESH_BASE_FLAG), 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)

   MeshLod& tesselate(); // smooth subdivide faces, preserving original vertexes
   MeshLod& subdivide(); // smooth subdivide faces,  smoothing original vertexes

   MeshLod& boneRemap(C MemPtr<Byte, 256> &old_to_new); // remap vertex bone/matrix indexes according to bone 'old_to_new' remap
   void         setUsedBones(Bool (&bones)[256])C;
   void     includeUsedBones(Bool (&bones)[256])C;

   MeshLod& setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps=EPS, Int rays=1024, MESH_AO_FUNC func=MAF_FULL, Threads *threads=null); // calculate per-vertex ambient occlusion in vertex colors, 'strength'=0..1 AO strength, 'bias'=0..1, 'max'=AO limit 0..1, 'ray_length'=max ray distance to test, 'rays'=number of rays to use for AO calculation, 'func'=falloff function

   MeshLod& freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // fix
   MeshLod& fixTexOffset  (Byte tex_index=0); // fix texture offset  , this reduces big texture coordinates to small ones increasing texturing quality on low precision video cards
   MeshLod& fixTexWrapping(Byte tex_index=0); // fix texture wrapping, fixes texture coordinates created by spherical/tube mapping (this can add new vertexes to the mesh)

   // convert
   MeshLod& edgeToDepth(Bool tex_align =true       ); // edges to depth (extrude 2D edges to 3D faces)
   MeshLod& edgeToTri  (Bool set_id    =false      ); // edges to triangles (triangulation)
   MeshLod& triToQuad  (Flt  cos       =EPS_COL_COS); // triangles to quads    , 'cos'=minimum cosine between 2      triangle normals to weld  them into 1 quad (0..1)
   MeshLod& quadToTri  (Flt  cos       =2          ); // quads     to triangles, 'cos'=minimum cosine between 2 quad triangle normals to leave them as   1 quad (0..1, value >1 converts all quads into triangles)

   // add / remove
   MeshLod& add(C MeshBase &src                                               ); // add MeshBase to self
   MeshLod& add(C MeshPart &src                                               ); // add MeshPart to self
   MeshLod& add(C MeshLod  &src, C Mesh *src_mesh=null, C Mesh *this_mesh=null); // add MeshLod  to self, 'src_mesh'=Mesh that 'src' belongs to, 'this_mesh'=Mesh that this belongs to, settings 'src_mesh' and 'this_mesh' is optional, it is used for remapping mesh part variations

   // optimize
#if EE_PRIVATE
   MeshLod& sortByMaterials  (); // sort MeshParts according to their materials
   MeshLod& removeDoubleEdges();
   MeshLod& removeSingleFaces(Flt fraction                                              ); // remove fraction of single faces (single triangles or quads not linked to any other face), 'fraction'=0..1
   MeshLod& weldInlineEdges  (Flt cos_edge=EPS_COL_COS, Flt cos_vtx=-1, Bool z_test=true); // weld   inline edge vertexes, 'cos_edge'=minimum cosine between edge normals, 'cos_vtx'=minimum cosine between vertex normals, 'z_test'=if perform tests for inline 'z' vertex component
#endif
   MeshLod& removeDegenerateFaces(Flt eps=EPS);
   Bool     removeUnusedVtxs     (Bool include_edge_references=true); // remove vertexes which aren't used by any face or edge, if 'include_edge_references' is set to false then only face references are tested (without the edges), returns true if any vertex was removed

   MeshLod& simplify(Flt intensity, Flt max_distance=1.0f, Flt max_uv=1.0f, Flt max_color=0.02f, Flt max_material=0.02f, Flt max_skin=1, Flt max_normal=PI, Bool keep_border=false, MESH_SIMPLIFY mode=SIMPLIFY_QUADRIC, Flt pos_eps=EPS, MeshLod *dest=null, Bool *stop=null); // simplify mesh by removing vertexes/faces, 'intensity'=how much to simplify (0..1, 0=no simplification, 1=full simplification), 'max_distance'=max distance between elements to merge them (0..Inf), 'max_uv'=max allowed vertex texture UV deviations (0..1), 'max_color'=max allowed vertex color deviations (0..1), 'max_material'=max allowed vertex material deviations (0..1), 'max_skin'=max allowed vertex skin deviations (0..1), 'max_normal'=max allowed vertex normal angle deviations (0..PI), 'keep_border'=if always keep border edges (edges that have faces only on one side), 'pos_eps'=vertex position epsilon, 'dest'=destination MeshBase (if set to null then the mesh will simplify itself), 'stop'=set to 'true' on secondary thread to stop this method, returns dest

   MeshLod& weldCoplanarFaces(Flt cos_face=EPS_COL_COS, Flt cos_vtx=-1, Bool safe=true, Flt max_face_length=-1); // weld coplanar faces, 'cos_face'=minimum cosine between face normals, 'cos_vtx'=minimum cosine between vertex normals, 'safe'=if process only faces without neighbors, 'max_face_length'=max allowed face length (-1=no limit)

   // draw
   #if EE_PRIVATE
      // helper drawing
      void draw2D       (C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r=0.04f, Flt side_width=0.01f                                             )C; // draw 2D
      void drawNormals2D(  Flt    length   , C Color &edge_color, C Color &vtx_color=TRANSPARENT                                                                         )C; // draw 2D normals
      void drawNormals  (  Flt    length   , C Color &face_color, C Color &vtx_color=TRANSPARENT, C Color &tangent_color=TRANSPARENT, C Color &binormal_color=TRANSPARENT)C; // draw 3D normals
   #endif
      // default drawing, doesn't use automatic Frustum Culling, this doesn't draw the mesh immediately, instead it adds the mesh to a draw list
      void draw(C MatrixM          &matrix, C Vec &vel, C Vec &ang_vel=VecZero)C; // add mesh to draw list using 'matrix'    matrix, 'vel'   velocity and 'ang_vel' angular velocity, this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C MatrixM          &matrix                                    )C; // add mesh to draw list using 'matrix'    matrix   and no velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed: RM_EARLY_Z RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND
      void draw(C AnimatedSkeleton &anim_skel                                 )C; // add mesh to draw list using 'anim_skel' matrixes and    velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed:            RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering)
      void draw(C AnimatedSkeleton &anim_skel, C Material &material           )C; // add mesh to draw list using 'anim_skel' matrixes and    velocities                             , this should be called only in RM_PREPARE, when used it will automatically draw meshes in following modes when needed:            RM_SIMPLE RM_SOLID RM_SOLID_M RM_AMBIENT RM_BLEND, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering), 'material'=material used for rendering which overrides the default material, however for performance reasons, the default shader is used, which means that the 'material' should be similar to the default material, and if it's too different then some artifacts can occur

      void drawShadow(C MatrixM          &matrix                         )C; // add mesh to shadow draw list using 'matrix'    matrix  , this should be called only in RM_SHADOW
      void drawShadow(C AnimatedSkeleton &anim_skel                      )C; // add mesh to shadow draw list using 'anim_skel' skeleton, this should be called only in RM_SHADOW, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering)
      void drawShadow(C AnimatedSkeleton &anim_skel, C Material &material)C; // add mesh to shadow draw list using 'anim_skel' skeleton, this should be called only in RM_SHADOW, 'anim_skel' must point to constant memory address (the pointer is stored through which the object can be accessed later during frame rendering), 'material'=material used for rendering which overrides the default material, however for performance reasons, the default shader is used, which means that the 'material' should be similar to the default material, and if it's too different then some artifacts can occur

      // draw blended, this is an alternative to default 'draw' (typically 'draw' draws blended meshes automatically for materials with technique in blend mode), this method however draws the mesh immediately (which allows to set custom shader parameters per draw call), it always uses blend shaders regardless if the material has technique set in blend mode, and provides additional control over material color, this can be called only in RM_BLEND rendering mode, doesn't use automatic Frustum culling
      void drawBlend(C Vec4 *color=null)C; // draw with current matrix, 'color'=pointer to optional Material color multiplication

      // draw mesh outline, this can be optionally called in RM_OUTLINE in order to outline the mesh, doesn't use automatic Frustum culling
      void drawOutline(C Color &color)C; // draw with current matrix

      // draw using the "behind" effect, this can be optionally called in RM_BEHIND, doesn't use automatic Frustum culling
      void drawBehind(C Color &color_perp, C Color &color_parallel)C; // draw with current matrix, 'color_perp'=color to be used for normals perpendicular to camera, 'color_parallel'=color to be used for normals parallel to camera

#if EE_PRIVATE
   // io
   Bool save    (File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load    (File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool saveData(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadData(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadOld (File &f, CChar *path=null) ;
#endif

   void operator*=(C Matrix3 &m) {transform(m);} // transform by matrix
   void operator*=(C Matrix  &m) {transform(m);} // transform by matrix

   MeshLod();
};
/******************************************************************************/
inline Int Elms(C MeshLod &mshl) {return mshl.parts.elms();}
/******************************************************************************/
