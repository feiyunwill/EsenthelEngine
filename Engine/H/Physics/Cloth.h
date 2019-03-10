/******************************************************************************

   Use 'Cloth' to simulate realistic cloth behavior.

   'ClothMesh' may not be deleted manually if there's still at least one 'Cloth' using the mesh.
      This means that the 'ClothMesh' may be deleted only after deleting all 'Cloth' objects using the mesh.

/******************************************************************************/
const_mem_addr struct ClothMesh // Physical Cloth Mesh, it is created from MeshBase and is used to create Cloth !! must be stored in constant memory address !!
{
   // manage
   ClothMesh& del   (                                                                  ); // delete manually
   ClothMesh& create(C MeshBase &mesh, C MaterialPtr &material, Skeleton *skeleton=null); // create from 'mesh' with 'material', set 'skeleton' for a Skeleton file to automatically adjust ClothMesh vertex bone indexes when loading to match the skeleton bone order

   // get
   Int       vtxs()C {return _phys.vtxs();} // get physical cloth vertex number
 C MeshBase& mesh()C {return _phys       ;} // get physical cloth mesh

   Skeleton*  skeleton(                  )C {return _skeleton;} // get Skeleton linked with this ClothMesh
   ClothMesh& skeleton(Skeleton *skeleton);                     // link ClothMesh with specified Skeleton file, avoid calling this realtime as it requires adjusting the vertex skinning information (bone indexes) and re-creating the hardware mesh version

 C MaterialPtr& material()C {return _material;} // get ClothMesh material

   // operations
   void boneRemap(C MemPtr<Byte, 256> &old_to_new, Bool remap_names=true); // remap vertex bone/matrix indexes according to bone 'old_to_new' remap, 'remap_names'=if remap the bone names as well

   // io
   Bool save(C Str &name)C; // save, false on fail
   Bool load(C Str &name) ; // load, false on fail
   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#if EE_PRIVATE
   Bool saveData(File &f, CChar *path=null)C; // save, false on fail
   Bool loadData(File &f, CChar *path=null) ; // load, false on fail
#endif

  ~ClothMesh() {del();}
   ClothMesh();

#if EE_PRIVATE
   void setShader();
#if PHYSX
   PxClothFabric* scaledMesh(Flt scale);
#endif
 C Material& getMaterial      (                  )C {return GetMaterial      (_material());}
 C Material& getShadowMaterial(Bool reuse_default)C {return GetShadowMaterial(_material(), reuse_default);}
#endif

#if !EE_PRIVATE
private:
#endif
   struct Scale
   {
      Flt scale;
   #if EE_PRIVATE
      PHYS_API(PxClothFabric, void) *mesh;
   #else
      Ptr mesh;
   #endif
   };
   MeshBase    _phys;
   MeshRender  _skin;
   Memc<Scale> _scales;
   MaterialPtr _material;
   IndBuf      _ind_buf;
   Shader     *_skin_shader[RM_SHADER_NUM],
              *_phys_shader[RM_SHADER_NUM];
#if EE_PRIVATE
   FRST       *_skin_frst, *_phys_frst;
#else
   Ptr         _skin_frst, _phys_frst;
#endif
   Skeleton   *_skeleton;
   BoneMap     _bone_map;

   NO_COPY_CONSTRUCTOR(ClothMesh);
};
/******************************************************************************/
struct Cloth // Physical Cloth
{
   // manage
   Cloth& del(); // delete manually
   Bool   create(ClothMesh &cloth_mesh, C Matrix &matrix); // create from 'cloth_mesh', 'matrix'=cloth matrix where its scale determines cloth scale, false on fail

   // get / set
   ClothMesh* clothMesh()C {return _cloth_mesh;}               // get     original 'ClothMesh' from which the 'Cloth' has been created
   Flt        scale    ()C {return _scale     ;}               // get     the scale which was used during the cloth creation
   Int        vtxs     ()C;                                    // get     number of vertexes
   Vec        wind     ()C;   Cloth& wind  (C Vec     &accel); // get/set wind acceleration
   Ptr        obj      ()C;   Cloth& obj   (Ptr        obj  ); // get/set pointer to object containing the cloth
   Bool       sleep    ()C;   Cloth& sleep (Bool       sleep); // get/set sleeping
   Box        box      ()C;                                    // get     world box containing the physical cloth

   Flt damping ()C;   Cloth& damping (Flt damping ); // get/set damping , 0..1
   Flt friction()C;   Cloth& friction(Flt friction); // get/set friction, 0..1
   Flt bending ()C;   Cloth& bending (Flt bending ); // get/set bending , 0..1

   Bool ccd()C;   Cloth& ccd(Bool on); // get/set continuous collision detection
   Bool gpu()C;   Cloth& gpu(Bool on); // get/set cloth processing on the GPU

   Flt    drag        ()C;   Cloth&    drag        (Flt drag ); // get/set         drag coefficient, 0..1, default=0.0
   Flt angDrag        ()C;   Cloth& angDrag        (Flt drag ); // get/set angular drag coefficient, 0..1, default=0.0
   Flt    inertiaScale()C;   Cloth&    inertiaScale(Flt scale); // get/set         inertia scale   , 0..1, default=1.0
   Flt angInertiaScale()C;   Cloth& angInertiaScale(Flt scale); // get/set angular inertia scale   , 0..1, default=1.0

   struct Particle
   {
      Vec pos         ; // position of a single particle
      Flt inverse_mass; // set 0 for static or 1 for dynamic particle (static will not move at all, dynamic will move according to physics simulation)
   };
 C Particle* lockRead(); // access cloth particles for reading, length of returned array is equal to 'vtxs', after calling this method you need to call 'unlock'
   Cloth&  unlock    (); // unlock read access, this must be called after 'lockRead'
   Cloth&    set     (C Particle *particle, Int particles); // set custom particles for the cloth, length of this array must be at least as long as 'vtxs'

   Cloth& setCollisionBalls   (C MemPtr<Ball > &balls   ); // set balls    that collide with this Cloth (up to 32 balls    are supported)
   Cloth& setCollisionCapsules(C MemPtr<VecI2> &capsules); // set capsules that collide with this Cloth (up to 32 capsules are supported), they are specified using indexes for both ends of capsule balls from the 'balls' specified using 'setCollisionBalls' (they must be set prior to calling 'setCollisionCapsules')

   // draw
   void drawSkinned        (C AnimatedSkeleton &anim_skel                      )C; // draw   normally skinned   cloth part using 'anim_skel', doesn't       use  Frustum culling, this can be called only in RM_PREPARE
   void drawSkinnedShadow  (C AnimatedSkeleton &anim_skel                      )C; // draw   normally skinned   cloth part using 'anim_skel', doesn't       use  Frustum culling, this can be called only in RM_SHADOW
   void drawSkinnedOutline (C AnimatedSkeleton &anim_skel, C Color &color      )C; // draw   normally skinned   cloth part using 'anim_skel', doesn't       use  Frustum culling, this can be called only in RM_OUTLINE in order to outline the mesh
   void drawPhysical       (                               C Vec   &vel=VecZero)C; // draw physically simulated cloth part                  , automatically uses Frustum culling, this can be called only in RM_PREPARE
   void drawPhysicalShadow (                                                   )C; // draw physically simulated cloth part                  , automatically uses Frustum culling, this can be called only in RM_SHADOW
   void drawPhysicalOutline(                               C Color &color      )C; // draw physically simulated cloth part                  , automatically uses Frustum culling, this can be called only in RM_OUTLINE in order to outline the mesh

  ~Cloth() {del();}
   Cloth();

#if !EE_PRIVATE
private:
#endif
   struct Vtx // Cloth Vertex
   {
      Vec  pos, // position
           nrm; // normal
      Vec2 tex; // texture coordinates
   };
   UInt       _update_count;
   Flt        _scale;
   ClothMesh *_cloth_mesh;
   UInt      *_vtxs;
   Vtx       *_vtx;
#if EE_PRIVATE
   PHYS_API(PxCloth            , void) *_cloth;
   PHYS_API(PxClothParticleData, void) *_lock;
#else
   Ptr        _cloth, _lock;
#endif
   VtxBuf     _vtx_buf;
#if EE_PRIVATE
   void  update      () ;
   void _drawPhysical()C;
#endif

   NO_COPY_CONSTRUCTOR(Cloth);
};
/******************************************************************************/
extern Cache<ClothMesh> ClothMeshes; // ClothMesh Cache
/******************************************************************************/
