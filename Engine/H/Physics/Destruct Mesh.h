/******************************************************************************

   Use 'DestructMesh' to pre-generate destructible meshes and their physical representation.

/******************************************************************************/
struct DestructMesh
{
   struct Part
   {
      Mesh     mesh;
      PhysPart phys;

   #if EE_PRIVATE
      void del   ();
      Bool create(Mesh &src, Int vtx_limit);
      Bool save  (File &f)C;
      Bool load  (File &f) ;
   #endif
   };
   struct Joint
   {
      Int a, b; // part indexes
   #if EE_PRIVATE
      void set(Int a, Int b) {T.a=a; T.b=b;}
   #endif
   };

   // get
   Bool   is    (     )C {return _parts .elms()>0;} // if  has any data
   Part&  part  (Int i)  {return _parts [i]      ;} // get i-th part
   Int    parts (     )C {return _parts .elms()  ;} // get number of parts
 C Joint& joint (Int i)C {return _joints[i]      ;} // get i-th joint
   Int    joints(     )C {return _joints.elms()  ;} // get number of joints

   // manage
   DestructMesh& del   (); // manually delete
   DestructMesh& create(Mesh &mesh, Int cuts, C MaterialPtr &material, Flt tex_scale=1, Int phys_vtx_limit=-1, Bool cut_affects_biggest_part=true, Bool simple_cuts=false); // create destructible object from 'mesh', use 'cuts' number of plane cuts to split the mesh, use 'material' for creating the solid inside part with 'tex_scale' texture scaling, 'phys_vtx_limit'=limit for number of vertexes in each physical body part (used only when >0), 'cut_affects_biggest_part'=if apply cutting to the biggest part only or the whole mesh, 'simple_cuts'=if use simplified mesh splitting algorithm that does not create the solid inside faces (if true then 'material' will be ignored)

   // operations
   Bool          adjustStorage (Bool universal, Bool physx, Bool bullet, Bool *changed=null); // adjust the type of storage for the physical body, 'universal'=can be used for both PhysX and Bullet, 'physx'=can be used for PhysX (and when used there it is faster than 'universal'), 'bullet'=can be used for Bullet (and when used there it is faster than 'universal'), each storage uses additional space, for PhysX only games it is suggested to set 'physx' to true and others to false, for Bullet only games it is suggested to set 'bullet' to true and others to false, please note if you call this method under engine compiled with Bullet library, you won't be able to use any PhysX information (which means converting from or to PhysX storage), 'changed'=pointer to custom bool which will be set to true if any change was performed on the physical body (false otherwise), false on fail
   DestructMesh& freeHelperData(                                                           ); // this free's up the helper data of the physical body, which increases available memory, however it disables saving the body to file, or converting it to 'MeshBase'
#if EE_PRIVATE
   void setShader();

   // draw
   void drawMesh(Int highlight_part=-1)C; // call in RM_PREPARE
   void drawPhys(                     )C; // call outside of Render
#endif

   // io
   Bool save(C Str &name); // save, false on fail
   Bool load(C Str &name); // load, false on fail

private:
   Mems<Part > _parts ;
   Mems<Joint> _joints;
};
/******************************************************************************/
extern Cache<DestructMesh> DestructMeshes;
/******************************************************************************/
