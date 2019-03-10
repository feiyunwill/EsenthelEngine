/******************************************************************************/
class AreaBuild
{
   class PhysObj
   {
      OBJ_PATH    path;
      PhysBodyPtr phys;
      Matrix      matrix;
      
      void set(C Obj &src); // this is called on main thread
      void set(C Object &src); // this is called on secondary thread
      void set(C ObjData &src, Memt<ObjectPtr> &obj_cache); // this is called on secondary thread

public:
   PhysObj();
   };

   VecI2         xy;
   uint       build, // AREA_REBUILD_FLAG
               load; // AREA_LOAD_FLAG
   Heightmap       hm;
   Image           hm_height_map,
                   hm_mtrl_map;
   Memc<UID>       hm_mtrls;
   Mesh            hm_mesh;
   PhysPart        hm_phys; 
   MeshGroup       obj_mesh;
   PhysBody        obj_phys;
   Memc<WaterMesh> waters;
   MeshBase        path_src;
   PathMesh        path;
   Memc<PhysObj>   objs;
   AreaVer         ver;
   Str             edit_path, game_path;
   AreaBuild      *l, *r, *b, *f, *lb, *lf, *rb, *rf;
   WorldVer       *world_ver;

   static bool Create(AreaBuild &area, C VecI2 &xy, ptr world_ver_ptr);
   static void SetPathType(MeshBase &mesh, PATH_MESH_TYPE path_mode);
   static void SetDefaultPathType(Memt<MeshBase> &meshes);

   void process(BuilderClass &builder);
   void processPath(BuilderClass &builder);

public:
   AreaBuild();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
