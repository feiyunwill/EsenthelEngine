/******************************************************************************

   In following functions 'hit_face' can indicate a triangle or a quad index
   If it's a quad then 'hit_face' has XOR'ed SIGN_BIT into its value
   Determining triangle or quad looks like this:

   if(hit_face&SIGN_BIT)quad_index=hit_face^SIGN_BIT; // it's a quad
   else                  tri_index=hit_face         ; // it's a triangle

/******************************************************************************/
// if moving 3D point cuts through a static mesh, if mesh is transformed pass its matrix to 'mesh_matrix', 'hit_frac'=0..1, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool Sweep(C Vec  &point, C Vec  &move, C MeshBase   &mshb, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null                                        , Bool test_quads_as_2_tris=true, Bool two_sided=false                     ); //                                                                 'two_sided'=if mesh faces are two-sided
Bool Sweep(C Vec  &point, C Vec  &move, C MeshRender &mshr, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null                                                                        , Bool two_sided=false                     ); //                                                                 'two_sided'=if mesh faces are two-sided
Bool Sweep(C Vec  &point, C Vec  &move, C MeshPart   &part, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null                                        , Bool test_quads_as_2_tris=true, Int  two_sided=-1, Bool only_visible=true); //                                                               , 'two_sided'=if mesh faces are two-sided (use -1 to set according to material cull), 'only_visible'=if process only visible mesh parts
Bool Sweep(C Vec  &point, C Vec  &move, C MeshLod    &mesh, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true, Int  two_sided=-1, Bool only_visible=true); // 'hit_part'=index of hit MeshPart                              , 'two_sided'=if mesh faces are two-sided (use -1 to set according to material cull), 'only_visible'=if process only visible mesh parts
Bool Sweep(C Vec  &point, C Vec  &move, C Mesh       &mesh, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true, Int  two_sided=-1, Bool only_visible=true); // 'hit_part'=index of hit MeshPart                              , 'two_sided'=if mesh faces are two-sided (use -1 to set according to material cull), 'only_visible'=if process only visible mesh parts
Bool Sweep(C VecD &point, C VecD &move, C Mesh       &mesh, C MatrixM *mesh_matrix=null, Flt *hit_frac=null, VecD *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true, Int  two_sided=-1, Bool only_visible=true); // 'hit_part'=index of hit MeshPart                              , 'two_sided'=if mesh faces are two-sided (use -1 to set according to material cull), 'only_visible'=if process only visible mesh parts
Bool Sweep(C Vec  &point, C Vec  &move, C MeshGroup  &mshg, C Matrix  *mesh_matrix=null, Flt *hit_frac=null, Vec  *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true, Int  two_sided=-1, Bool only_visible=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh, 'two_sided'=if mesh faces are two-sided (use -1 to set according to material cull), 'only_visible'=if process only visible mesh parts
/******************************************************************************/
// raycast left from(*, point.y, point.x), return if hit, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool PosPointMeshXL(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos=null, Int *hit_face=null,                        C Box *box=null, Bool test_quads_as_2_tris=true); // 'box'=pass 'mshb' bounding box if you know it to improve performance
Bool PosPointMeshXL(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart
Bool PosPointMeshXL(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh

// raycast right from(*, point.y, point.x), return if hit, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool PosPointMeshXR(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos=null, Int *hit_face=null,                        C Box *box=null, Bool test_quads_as_2_tris=true); // 'box'=pass 'mshb' bounding box if you know it to improve performance
Bool PosPointMeshXR(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart
Bool PosPointMeshXR(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh

// raycast downward from(point.x, *, point.y), return if hit, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool PosPointMeshY(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos=null, Int *hit_face=null,                        C Box *box=null, Bool test_quads_as_2_tris=true); // 'box'=pass 'mshb' bounding box if you know it to improve performance
Bool PosPointMeshY(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart
Bool PosPointMeshY(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh

// raycast forward from(point.x, point.y, *), return if hit, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool PosPointMeshZF(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos=null, Int *hit_face=null,                        C Box *box=null, Bool test_quads_as_2_tris=true); // 'box'=pass 'mshb' bounding box if you know it to improve performance
Bool PosPointMeshZF(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart
Bool PosPointMeshZF(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh

// raycast backwards from(point.x, point.y, *), return if hit, 'hit_pos'=hit position, 'hit_face'=index of hit face, if you're sure that quad's are fully valid (their triangles are coplanar) set 'test_quads_as_2_tris'=false for performance boost
Bool PosPointMeshZB(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos=null, Int *hit_face=null,                        C Box *box=null, Bool test_quads_as_2_tris=true); // 'box'=pass 'mshb' bounding box if you know it to improve performance
Bool PosPointMeshZB(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null                    , Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart
Bool PosPointMeshZB(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos=null, Int *hit_face=null, Int *hit_part=null, Int *hit_mesh=null, Bool test_quads_as_2_tris=true); // 'hit_part'=index of hit MeshPart, 'hit_mesh'=index of hit Mesh
/******************************************************************************/
// get mesh rest matrix as if it has fallen on the ground and lying there, 'initial_matrix'=initial matrix of the mesh (use null for no custom matrix), 'mass_center'=mesh center of mass (use null for auto-detect), 'min_dist'=minimum distance between vertexes to consider them as contact points, 'rest_box'=if specified then it will be set to the precise box of the mesh after being transformed by the rest matrix, 'max_steps'=maximum number of steps to perform the calculation (<0 = unlimited), 'only_visible'=if process only visible mesh parts, 'only_phys'=if process only mesh parts without 'MSHP_NO_PHYS_BODY'
Matrix GetRestMatrix(C MeshBase            &mesh  , C Matrix *initial_matrix=null, C Vec *mass_center=null, Flt min_dist=0.01f, Box *rest_box=null, Int max_steps=-1);
Matrix GetRestMatrix(C MeshLod             &mesh  , C Matrix *initial_matrix=null, C Vec *mass_center=null, Flt min_dist=0.01f, Box *rest_box=null, Int max_steps=-1, Bool only_visible=true, Bool only_phys=true);
Matrix GetRestMatrix(C Mesh                &mesh  , C Matrix *initial_matrix=null, C Vec *mass_center=null, Flt min_dist=0.01f, Box *rest_box=null, Int max_steps=-1, Bool only_visible=true, Bool only_phys=true);
Matrix GetRestMatrix(C MemPtr<C MeshPart*> &meshes, C Matrix *initial_matrix=null, C Vec *mass_center=null, Flt min_dist=0.01f, Box *rest_box=null, Int max_steps=-1, Bool only_visible=true, Bool only_phys=true);
Matrix GetRestMatrix(C MemPtr<C MeshLod *> &meshes, C Matrix *initial_matrix=null, C Vec *mass_center=null, Flt min_dist=0.01f, Box *rest_box=null, Int max_steps=-1, Bool only_visible=true, Bool only_phys=true);
/******************************************************************************/
#if EE_PRIVATE
/******************************************************************************/
enum TEST_FLAG // Test Flags
{
   TEST_NO_PHYS    =0x1, // skip testing if ETQ_NO_PHYS set
 //TEST_DOUBLE_SIDE=0x2, // skip testing if double sided
};
/******************************************************************************/
struct CutsCache
{
   Box         box     ;
   Boxes       boxes   ;
   Memc<UInt> *box_face;

           ~CutsCache();
   explicit CutsCache(C MeshBase &mshb);
   NO_COPY_CONSTRUCTOR(CutsCache);
};
/******************************************************************************/
DIST_TYPE DistPointMesh(C Vec2 &point, C MeshBase  &mshb, UInt flag, Flt *dist=null, Int *index=null, UInt test_flag=0); // 'flag' specifies what to test(can be combination of VTX_POS,EDGE_IND,TRI_IND,QUAD_IND), 'dist'=pointer to distance, 'index'=pointer to nearest element(vtx/edge/tri/quad), 'test_flag'=TEST_FLAG
DIST_TYPE DistPointMesh(C Vec  &point, C MeshBase  &mshb, UInt flag, Flt *dist=null, Int *index=null                  ); // 'flag' specifies what to test(can be combination of VTX_POS,EDGE_IND,TRI_IND,QUAD_IND), 'dist'=pointer to distance, 'index'=pointer to nearest element(vtx/edge/tri/quad)

Bool      CutsPointMesh(C Vec2 &point, C MeshBase  &mshb,            Flt *dist=null,                  UInt test_flag=0); // if point 2D is inside mshb, 'dist'=distance to mesh, 'test_flag'=TEST_FLAG
Bool      CutsPointMesh(C Vec  &point, C MeshBase  &mshb,            Flt *dist=null,             CutsCache *cache=null); // if point 3D is inside mshb, 'dist'=distance to mesh, 'cache    '=helper object created on the base of 'mshb' for function improved performance

Bool      CutsPointMesh(C Vec2 &point, C MeshLod   &mesh,            Flt *dist=null,                  UInt test_flag=0); // if point 2D is inside mesh, 'dist'=distance to mesh, 'test_flag'=TEST_FLAG
Bool      CutsPointMesh(C Vec  &point, C MeshLod   &mesh,            Flt *dist=null                                   ); // if point 3D is inside mesh, 'dist'=distance to mesh

Bool      CutsPointMesh(C Vec2 &point, C Mesh      &mesh,            Flt *dist=null,                  UInt test_flag=0); // if point 2D is inside mesh, 'dist'=distance to mesh, 'test_flag'=TEST_FLAG
Bool      CutsPointMesh(C Vec  &point, C Mesh      &mesh,            Flt *dist=null                                   ); // if point 3D is inside mesh, 'dist'=distance to mesh

Bool      CutsPointMesh(C Vec2 &point, C MeshGroup &mshg,                                             UInt test_flag=0); // if point 2D is inside mshg
Bool      CutsPointMesh(C Vec  &point, C MeshGroup &mshg                                                              ); // if point 3D is inside mshg
/******************************************************************************/
// if moving 2D point cuts through a static mesh, 'hit_frac'=0..1, 'hit_pos'=hit position, 'hit_edge'=index of hit edge
Bool Sweep(C Vec2 &point, C Vec2 &move, C MeshBase &mshb,                                   Flt *hit_frac=null, Vec2 *hit_pos=null, Int *hit_edge=null);
Bool Sweep(C Vec2 &point, C Vec2 &move, C MeshBase &mshb, C Rects &rects, Index &rect_edge, Flt *hit_frac=null, Vec2 *hit_pos=null, Int *hit_edge=null);
/******************************************************************************/
#endif
/******************************************************************************/
