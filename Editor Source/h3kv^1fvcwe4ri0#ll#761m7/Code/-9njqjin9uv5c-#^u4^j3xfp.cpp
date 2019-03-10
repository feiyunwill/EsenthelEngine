/******************************************************************************

   CacheLock is used on:
      Object
      Mesh
      PhysBody
   because it's used for reading on BuilderThread and can be modified on MainThread

   Please remember that:
      Reading on BuilderThread + writing on MainThread = fail
   // TODO: verify all codes for that
   When doing File.read any SafeOverwrite will fail

   When rebuilding paths, can't use Game.Area.ObjectPhys because it doesn't have information about OBJ_PATH

   Slowdowns may occur when flushing built areas to the disk,
      however this is needed to be done on the main thread, so new data will be synchronized with times.

/******************************************************************************/
const bool FastTerrainPhys=true;
/******************************************************************************/
class EmbedObjBase
{
   UID      elm_obj_id; // ID of project element ELM_OBJ
   Matrix   matrix;
   bool     ovr_path=false, ovr_mesh_variation_id=false;
   uint     mesh_variation_id=0;
   OBJ_PATH path=OBJ_PATH_CREATE;

   void operator=(ObjVer &obj)
   {
      elm_obj_id=obj.elm_obj_id; obj.matrix.set(matrix);
      if(ovr_path             =obj.ovrPath   ())path             =obj.pathSelf       (); // set only if is custom value, otherwise we'll take path from 'Object' later !! can't access Proj.getObjPath because it's not thread safe !!
      if(ovr_mesh_variation_id=obj.meshVarOvr())mesh_variation_id=obj.mesh_variation_id; // set only if is custom value, otherwise we'll take path from 'Object' later
   }
}
class EmbedObj
{
   Mesh        mesh;
   MeshBase    phys_path , // phys for path  (has phys mesh/convex/shapes)
               phys_actor; // phys for actor (has phys mesh/convex       ), shapes will be taken from phys_shapes
   Rect        mesh_rect, phys_path_rect, phys_actor_rect;
   Memc<Shape> phys_shapes;
   OBJ_PATH    path=OBJ_PATH_CREATE;

   void set(C Matrix &matrix, C Object &src, bool set_mesh, bool set_phys, OBJ_PATH *ovr_path, uint *ovr_mesh_variation_id) // this is called on secondary thread
   {
      MeshPtr     mesh_ptr;
      PhysBodyPtr phys_ptr;
      uint        mesh_variation_id=0; 
      {
         CacheLock cl(Objects); // use lock so 'Object' members will not be changed on secondary thread
                     path             =(ovr_path              ? *ovr_path              : src.path           ());
                     mesh_variation_id=(ovr_mesh_variation_id ? *ovr_mesh_variation_id : src.meshVariationID());
         if(set_mesh)mesh_ptr=src.mesh();
         if(set_phys)phys_ptr=src.phys();
      }

      // copy mesh
      if(mesh_ptr)
      {
         CacheLock cl(Meshes); // use lock so Mesh members will not be changed on secondary thread
         mesh.create(*mesh_ptr);
      }

      // copy phys
      if(phys_ptr)
      {
         CacheLock cl(PhysBodies); // use lock so PhysBody members will not be changed on secondary thread
         FREPA(*phys_ptr)
         {
            PhysPart &part=phys_ptr->parts[i];
            MeshBase  temp; temp.create(part);
            if(part.type()==PHYS_SHAPE)phys_shapes.add(part.shape);
            else                       phys_actor .add(temp);
            if(path!=OBJ_PATH_IGNORE  )phys_path  .add(temp); // don't copy if we're going to ignore it
         }
      }

      Box box;
      mesh.setBase().delRender().exclude(VTX_SKIN).transform(matrix); mesh      .getBox(box); mesh_rect      =box.xz();
                                      phys_path   .transform(matrix); phys_path .getBox(box); phys_path_rect =box.xz();
                                      phys_actor  .transform(matrix); phys_actor.getBox(box); phys_actor_rect=box.xz();
                                REPAO(phys_shapes).transform(matrix);
      mesh.variationKeep(mesh.variationFind(mesh_variation_id)); // keep only desired variation
   }
}
/******************************************************************************/
enum AREA_LOAD_FLAG
{
   AREA_LOAD_HEIGHTMAP     =1<<0,
   AREA_LOAD_HM_MESH       =1<<1,
   AREA_LOAD_HM_PHYS       =1<<2,
   AREA_LOAD_LOCAL_OBJ_PHYS=1<<3, // if load physical bodies of area objects (that are not embedded)
   AREA_LOAD_LOCAL_WATER   =1<<4, // if load water meshes of an area
   AREA_MAKE_PATH_SRC      =1<<5, // if create 'path_src' member which is used for PathMesh generation in later step (don't put this into 'build' because it should not be stored in save, it's only a temporary helper value)
}
class AreaBuild
{
   class PhysObj
   {
      OBJ_PATH    path=OBJ_PATH_IGNORE;
      PhysBodyPtr phys;
      Matrix      matrix(1);
      
      void set(C Obj &src) // this is called on main thread
      {
            path =(src.removed ? OBJ_PATH_IGNORE : src.params.path);
         if(path!=OBJ_PATH_IGNORE)
         {
            matrix=src.matrix;
            phys  =src.phys;
         }
      }
      void set(C Object &src) // this is called on secondary thread
      {
         CacheLock cl(Objects); // use lock so 'Object' members will not be changed on secondary thread
            path =src.path();
         if(path!=OBJ_PATH_IGNORE)
         {
            phys=src.phys();
            {CacheLock clm(Meshes); CacheLock clp(PhysBodies); matrix=src.matrixFinal();} // lock because 'matrixFinal' accesses mesh and phys (lock first mesh in case in the future mesh would load physics like skeleton, in that case it would use phys lock inside mesh lock)
         }
      }
      void set(C ObjData &src, Memt<ObjectPtr> &obj_cache) // this is called on secondary thread
      {
            path =(src.removed ? OBJ_PATH_IGNORE : src.params.path);
         if(path!=OBJ_PATH_IGNORE)
         {
            matrix=src.matrix;
            CacheLock cl(Objects); ObjectPtr obj=EditToGamePath(src.params.base.name()); phys=obj->phys(); Swap(obj_cache.New(), obj); // use lock so 'Object' members will not be changed on a secondary thread
         }
      }
   }

   VecI2         xy=0;
   uint       build=0, // AREA_REBUILD_FLAG
               load=0; // AREA_LOAD_FLAG
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
   AreaBuild      *l=null, *r=null, *b=null, *f=null, *lb=null, *lf=null, *rb=null, *rf=null;
   WorldVer       *world_ver=null;

   static bool Create(AreaBuild &area, C VecI2 &xy, ptr world_ver_ptr)
   {
      if(WorldVer *world_ver=(WorldVer*)world_ver_ptr)
      if( AreaVer * area_ver=world_ver.areas.find(xy))
      {
         area.ver      =*area_ver;
         area.xy       =xy;
         area.world_ver=world_ver;
         area.edit_path=Proj.editAreaPath(world_ver.world_id, xy);
         area.game_path=Proj.gameAreaPath(world_ver.world_id, xy);
         return true;
      }
      return false;
   }
   static void SetPathType(MeshBase &mesh, PATH_MESH_TYPE path_mode)
   {
      mesh.include( TRI_FLAG); REPA(mesh.tri )mesh.tri .flag(i)=path_mode;
      mesh.include(QUAD_FLAG); REPA(mesh.quad)mesh.quad.flag(i)=path_mode;
   }
   static void SetDefaultPathType(Memt<MeshBase> &meshes)
   {
      REPA(meshes)if(meshes[i].tri.flag() || meshes[i].quad.flag()) // if at least one mesh has flag specified, then set all that don't have
      {
         REPA(meshes) // set PM_GROUND to all face flags
         {
            MeshBase &base=meshes[i];
            if(!base.tri .flag()){base.include( TRI_FLAG); REPA(base.tri )base.tri .flag(i)=PM_GROUND;}
            if(!base.quad.flag()){base.include(QUAD_FLAG); REPA(base.quad)base.quad.flag(i)=PM_GROUND;}
         }
         break;
      }
   }

   void process(BuilderClass &builder)
   {
      flt   area_size=builder.path_settings.areaSize();
      Rect  area_rect(xy, xy+1); area_rect*=area_size;
      Plane area_clip_planes[4]=
      {
         Plane(area_rect.min.x0y(), Vec(-1,0,0)), // left
         Plane(area_rect.min.x0y(), Vec(0,0,-1)), // back
         Plane(area_rect.max.x0y(), Vec( 1,0,0)), // right
         Plane(area_rect.max.x0y(), Vec(0,0, 1)), // forward
      };

      if(build&(AREA_REBUILD_HM|AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS|AREA_REBUILD_EMBED_OBJ|AREA_REBUILD_WATER))ThreadMayUseGPUData();

      if(build&AREA_REBUILD_HM)
      {
         if(builder.io_thread.wantStop())return;

         if(hm.is())
         {
            hm.cleanMaterials(); // to remove any materials no longer used

            hm_height_map.createSoft(hm.resolution(), hm.resolution(), 1, IMAGE_F32);
            REPD(y, hm_height_map.h())
            REPD(x, hm_height_map.w())hm_height_map.pixelF(x, y, builder.path_settings.areaSize()*hm.height(x, y));

            hm_mtrl_map.createSoft(hm.resolution(), hm.resolution(), 1, IMAGE_I8);
            REPD(y, hm_mtrl_map.h())
            REPD(x, hm_mtrl_map.w())
            {
               VecB4 index, blend;
               hm.getMaterial   (x, y, index, blend);
               hm_mtrl_map.pixel(x, y, index.c[blend.maxI()]);
            }
            hm_mtrls.setNum(hm.materials()); REPAO(hm_mtrls)=hm.material(i).id();
         }else
         {
            hm_height_map.del();
              hm_mtrl_map.del();
                 hm_mtrls.del();
         }
      }

      if(build&AREA_REBUILD_HM_MESH)
      {
         if(builder.io_thread.wantStop())return;
         Build(hm, hm_mesh, builder.path_settings.areaSize(), xy, l ? &l.hm : null, r ? &r.hm : null, b ? &b.hm : null, f ? &f.hm : null, lb ? &lb.hm : null, lf ? &lf.hm : null, rb ? &rb.hm : null, rf ? &rf.hm : null);
      }
      if(build&AREA_REBUILD_HM_PHYS)if(!FastTerrainPhys) // creating terrain phys from original mesh is much slower
      {
         if(builder.io_thread.wantStop())return;
         MeshBase temp; temp.createPhys   (hm_mesh).weldVtx(0, 0.01, EPS_COL_COS, -1).simplify(1, 0.10, 1, 1, 1, 1, PI, true, SIMPLIFY_PLANES);
                     hm_phys.createMeshTry(   temp);
      }
      if(build&AREA_REBUILD_HM_MESH_SIMPLIFY)
      {
         if(builder.io_thread.wantStop())return;
         Simplify(hm_mesh);
      }
      if(build&AREA_REBUILD_HM_PHYS)if(FastTerrainPhys) // creating terrain phys from a simplified mesh is much faster, but we need to use different 'max_distance'
      {
         if(builder.io_thread.wantStop())return;
         MeshBase temp; temp.createPhys   (hm_mesh).weldVtx(0, 0.01, EPS_COL_COS, -1).simplify(1, 0.05, 1, 1, 1, 1, PI, true, SIMPLIFY_PLANES);
                     hm_phys.createMeshTry(   temp);
      }
      if(build&AREA_REBUILD_EMBED_OBJ)
      {
         if(builder.io_thread.wantStop())return;
         Memt<MeshBase> phys_meshes;
         FREPA(builder.embed_objs)
         {
          C EmbedObj &obj=builder.embed_objs[i];
            // add meshes
            if(obj.mesh.is() && Cuts(area_rect, obj.mesh_rect))
            {
               Mesh clipped; ClipMesh(obj.mesh, null, clipped, area_clip_planes, Elms(area_clip_planes));
               if(clipped.is())
               {
                  clipped.fixTexOffset(); // this is important for Mobile platforms (for meshes with high tex coords, like roads)
                  clipped.joinAll(true, false, false, MeshJoinAllTestVtxFlag);
                  Swap(obj_mesh.meshes.New(), clipped);
               }
            }

            // add physical meshes
            if(obj.phys_actor.is() && Cuts(area_rect, obj.phys_actor_rect))
            {
               MeshBase clipped; ClipMesh(obj.phys_actor, null, clipped, area_clip_planes, Elms(area_clip_planes));
               if(      clipped.is())Swap(phys_meshes.New(), clipped);
            }

            // add physical shapes
            if(Cuts(area_rect, obj.phys_path_rect))REPA(obj.phys_shapes)
            {
             C Shape &shape=obj.phys_shapes[i];
               REPA(area_clip_planes)if(Dist(shape, Shape(area_clip_planes[i]))>0)goto skip_shape; // if the shape is completely outside of at least 1 plane, then skip the shape
               obj_phys.parts.New().create(shape);
            skip_shape:;
            }
         }

         // split big and complex meshes (important for local light shadows)
         REPD(cut_steps, MeshSplitMaxSteps) // try few steps
            REPA(obj_mesh) // go from end so split meshes aren't processed again
         {
            Mesh &mesh=obj_mesh.meshes[i];
            if(mesh.vtxs()>=MeshSplitMinVtxs) // has enough vertexes
            {
               Vec size=mesh.ext.size(); int max_size=size.maxI();
               if( size.c[max_size]>=MeshSplitMinSize) // is big enough
               {
                  Plane plane; plane.pos=mesh.ext.center; // split in half
                  switch(max_size)
                  {
                     default: plane.normal.set(1, 0, 0); break; // the biggest is X dimension so split in half of the X
                     case  1: plane.normal.set(0, 1, 0); break; // the biggest is Y dimension so split in half of the Y
                     case  2: plane.normal.set(0, 0, 1); break; // the biggest is Z dimension so split in half of the Z
                  }
                  Mesh temp; SplitMesh(mesh, null, mesh, temp, plane); Swap(obj_mesh.meshes.New(), temp); // !! creating new element makes 'mesh' reference invalid !!
               }
            }
         }

         // finalize mesh and phys
         obj_mesh.setBox(false);
         obj_mesh.setRender().delBase();
         if(phys_meshes.elms())
         {
            MeshBase temp; temp.create(phys_meshes.data(), phys_meshes.elms());
            obj_phys.parts.New().createMeshTry(temp);
         }
         obj_phys.setBox();
      }
      if(build&AREA_REBUILD_WATER)
      {
         if(builder.io_thread.wantStop())return;
         FREPA(builder.waters)
         {
            C WaterMesh &water=builder.waters[i];
            if(water.is() && Cuts(water.box().xz(), area_rect))
            {
               MeshBase clipped; ClipMesh(water.mesh(), null, clipped, area_clip_planes, Elms(area_clip_planes));
               if(      clipped.is())waters.New().create(clipped, water.lake(), water.depth, water.material());
            }
         }
      }
      if(load&AREA_MAKE_PATH_SRC) // create 'path_src' MeshBase from heightmap+local_obj+embed_obj+water phys
      {
         if(builder.io_thread.wantStop())return;
         Memt<MeshBase> meshes;

         // heightmap
         if(hm_phys.is())meshes.New().create(hm_phys);

         // local objects
         FREPA(objs)
         {
            PhysObj &obj=objs[i];
            if(obj.path!=OBJ_PATH_IGNORE)if(obj.phys)
            {
               int meshes_elms=meshes.elms();
               {CacheLock cl(PhysBodies); REPA(*obj.phys)meshes.New().create(obj.phys->parts[i]);} // use lock because PhysBody can be changed on a different thread
               for(; meshes_elms<meshes.elms(); meshes_elms++) // after unlock, process added meshes
               {
                  MeshBase &mesh=meshes[meshes_elms].transform(obj.matrix);
                  if(obj.path==OBJ_PATH_BLOCK)SetPathType(mesh, PM_OBSTACLE);
               }
            }
         }

         // embedded objects
         FREPA(builder.embed_objs)
         {
            EmbedObj &obj=builder.embed_objs[i];
            if(obj.phys_path.is() && Cuts(area_rect, obj.phys_path_rect))
            {
               MeshBase clipped; ClipMesh(obj.phys_path, null, clipped, area_clip_planes, Elms(area_clip_planes)); if(obj.path==OBJ_PATH_BLOCK)SetPathType(clipped, PM_OBSTACLE);
               if(      clipped.is())Swap(meshes.New(), clipped);
            }
         }

         // water
         FREPA(waters)
         {
            MeshBase &water=meshes.New().create(waters[i].mesh(), VTX_POS|TRI_IND|QUAD_IND);
            SetPathType(water, PM_WATER);
         }
         /* TODO: 
         // ocean
         if(Water.draw) // add ocean surface if enabled
         {
            flt       size =World.areaSize();
            MeshBase &ocean=path_meshes.New();
            ocean.createPlane(2, 2, VTX_POS|TRI_IND|QUAD_IND).transform(Matrix().setPosOrient(Vec(0), DIR_DOWN).scale(size).move(Vec(cell.x()*size, Water.plane.pos.y, cell.y()*size)));
            SetPathType(ocean, PM_WATER);
         }*/

         SetDefaultPathType(meshes);
         path_src.create(meshes.data(), meshes.elms());
      }
   }
   void processPath(BuilderClass &builder)
   {
      if(build&AREA_REBUILD_PATH)
      {
         if(builder.io_thread.wantStop())return;
         // gather all nearby meshes
         Memt<MeshBase> meshes;
         if(         path_src.is())meshes.New().create(   path_src);
         if(l  && l .path_src.is())meshes.New().create(l .path_src);
         if(r  && r .path_src.is())meshes.New().create(r .path_src);
         if(b  && b .path_src.is())meshes.New().create(b .path_src);
         if(f  && f .path_src.is())meshes.New().create(f .path_src);
         if(lb && lb.path_src.is())meshes.New().create(lb.path_src);
         if(lf && lf.path_src.is())meshes.New().create(lf.path_src);
         if(rb && rb.path_src.is())meshes.New().create(rb.path_src);
         if(rf && rf.path_src.is())meshes.New().create(rf.path_src);
         if(meshes.elms())
         {
            SetDefaultPathType(meshes);
            MeshBase mesh; mesh.create(meshes.data(), meshes.elms());
            path.create(mesh, xy, builder.path_settings);
            path.preSave(); // prepare data so there won't be any delays during saving
         }
      }
   }
}
/******************************************************************************/
class BuilderClass
{
   static void Process    (AreaBuild &area, BuilderClass &builder, int thread_index) {area.process    (builder);}
   static void ProcessPath(AreaBuild &area, BuilderClass &builder, int thread_index) {area.processPath(builder);}

   static bool IOFunc(Thread &thread) {return ((BuilderClass*)thread.user).ioFunc();}
          bool ioFunc(              )
   {
      io_wait.wait(); // wait until we have data to process
      if(!io_thread.wantStop())
      {
         SyncLocker locker(lock);
         // prepare
         Memt<ObjectPtr> obj_cache; // use cache so objects don't need to be loaded everytime
         uint area_build=0;
         REPA(areas)
         {
            if(io_thread.wantStop())return false;
            // load data for areas
            AreaBuild &area=areas.lockedData(i);
            area_build|=area.build;
            if(area.load) // if want to load something
            {
               // load objects from Edit (ObjData) and not from 'EE.Object' so Mesh is not loaded
               if(area.load&(AREA_LOAD_HEIGHTMAP|AREA_LOAD_HM_MESH|AREA_LOAD_LOCAL_WATER))ThreadMayUseGPUData(); // heightmaps use Materials and Textures, water uses textures and shaders
               if(area.load&(AREA_LOAD_HEIGHTMAP|AREA_LOAD_LOCAL_OBJ_PHYS               ))
               {
                  Memc<ObjData> objs;
                  LoadEdit(area.edit_path, (area.load&AREA_LOAD_HEIGHTMAP) ? &area.hm : null, (area.load&AREA_LOAD_LOCAL_OBJ_PHYS) ? &objs : null, game_path, edit_path);
                  if(area.world_ver)FREPA(objs)if(objs[i].params.path!=OBJ_PATH_IGNORE && !objs[i].removed && !area.world_ver.embedded(objs[i]))area.objs.New().set(objs[i], obj_cache);
                  FlagDisable(area.load, AREA_LOAD_HEIGHTMAP|AREA_LOAD_LOCAL_OBJ_PHYS);
               }
               if(area.load&(AREA_LOAD_HM_MESH|AREA_LOAD_HM_PHYS|AREA_LOAD_LOCAL_WATER))
               {
                  LoadGame(area.game_path, (area.load&AREA_LOAD_HM_MESH) ? &area.hm_mesh : null, (area.load&AREA_LOAD_HM_PHYS) ? &area.hm_phys : null, null, null, null, (area.load&AREA_LOAD_LOCAL_WATER) ? &area.waters : null, game_path);
                  FlagDisable(area.load, AREA_LOAD_HM_MESH|AREA_LOAD_HM_PHYS|AREA_LOAD_LOCAL_WATER);
               }
            }
            // set neighbors
            area.l =areas.find(area.xy+VecI2(-1, 0));
            area.r =areas.find(area.xy+VecI2( 1, 0));
            area.b =areas.find(area.xy+VecI2( 0,-1));
            area.f =areas.find(area.xy+VecI2( 0, 1));
            area.lb=areas.find(area.xy+VecI2(-1,-1));
            area.lf=areas.find(area.xy+VecI2(-1, 1));
            area.rb=areas.find(area.xy+VecI2( 1,-1));
            area.rf=areas.find(area.xy+VecI2( 1, 1));
         }

         // load embedded
         bool load_obj_mesh=FlagTest(area_build, AREA_REBUILD_EMBED_OBJ),
              load_obj_phys=FlagTest(area_build, AREA_REBUILD_EMBED_OBJ|AREA_REBUILD_PATH);
         if(embed_objs_rect.valid() && world_ver && (load_obj_mesh || load_obj_phys))
         {
            ThreadMayUseGPUData(); // objects load meshes
            Memt<UID         > world_objs;                           {MapLock ml(world_ver.obj_embed); REPA(world_ver.obj_embed)if(Cuts(world_ver.obj_embed.lockedData(i), embed_objs_rect))world_objs.add(world_ver.obj_embed.lockedKey(i));} // use lock because 'obj_embed' can be modified on MainThread, use lock inside brackets so it will be released after we've gathered ID's
            Memt<EmbedObjBase> embed_objs_base; if(world_objs.elms()){MapLock ml(world_ver.obj      ); REPA(world_objs         )if(ObjVer *obj_ver=world_ver.obj.find(world_objs[i]))embed_objs_base.New()=*obj_ver;} // use lock because 'obj' can be modified on MainThread, use lock inside brackets so it will be released after we've gathered ID's
            FREPA(embed_objs_base)
            {
               EmbedObjBase &src=embed_objs_base[i];
               if(src.elm_obj_id.valid())
                  if(ObjectPtr obj=game_path+EncodeFileName(src.elm_obj_id))embed_objs.New().set(src.matrix, *obj, load_obj_mesh, load_obj_phys, src.ovr_path ? &src.path : null, src.ovr_mesh_variation_id ? &src.mesh_variation_id : null);
            }
         }

         // load waters
         if(water_rect.valid())
         {
            ThreadMayUseGPUData(); // waters use textures, shaders, meshes
            Memt<UID> lakes ; {MapLock ml(world_ver.lakes ); REPA(world_ver.lakes )if(Cuts(world_ver.lakes .lockedData(i).areas, water_rect))lakes .add(world_ver.lakes .lockedKey(i));} // use lock because 'lakes ' can be modified on MainThread
            Memt<UID> rivers; {MapLock ml(world_ver.rivers); REPA(world_ver.rivers)if(Cuts(world_ver.rivers.lockedData(i).areas, water_rect))rivers.add(world_ver.rivers.lockedKey(i));} // use lock because 'rivers' can be modified on MainThread
            Lake lake; REPA(lakes)
            {
               bool ok; {ReadLock rl(WorldAreaSync); ok=lake.load(Project.EditLakePath(edit_path, world_ver.world_id, lakes[i]));}
               if(  ok && !lake.removed)Swap(waters.New(), lake.checkMesh(Proj).water_mesh);
            }
            River river; REPA(rivers)
            {
               bool ok; {ReadLock rl(WorldAreaSync); ok=river.load(Project.EditRiverPath(edit_path, world_ver.world_id, rivers[i]));}
               if(  ok && !river.removed)Swap(waters.New(), river.checkMesh(Proj).water_mesh);
            }
         }

         ThreadFinishedUsingGPUData(); // release this context so build threads can use all available
         Memt<AreaBuild*> areas_build;

         // build areas
         if(io_thread.wantStop())return false;
         areas_build.clear(); REPA(areas){AreaBuild &area=areas.lockedData(i); if((area.build&~AREA_REBUILD_PATH) || (area.load&AREA_MAKE_PATH_SRC))areas_build.add(&area);} // build all except paths
         BuilderThreads.process1(areas_build, Process, T, background ? Max(1, Cpu.threads()-1) : Cpu.threads()); // leave one thread empty for main thread

         // build paths
         if(io_thread.wantStop())return false;
         areas_build.clear(); REPA(areas){AreaBuild &area=areas.lockedData(i); if(area.build&AREA_REBUILD_PATH)areas_build.add(&area);} // select only paths
         BuilderThreads.process1(areas_build, ProcessPath, T, background ? Max(1, Cpu.threads()-1) : Cpu.threads()); // leave one thread empty for main thread

         // finish
         finished_processing=true;
      }
      return true;
   }

   bool                            finished_processing=false, background=false;
   PathSettings                    path_settings;
   Str                             game_path, edit_path;
   SyncLock                        lock; // we need to operate on separate 'SyncLock' (and not 'areas.lock') because we use 'Swap' for 'areas' and thus we would operate on 2 'SyncLock's
   ThreadSafeMap<VecI2, AreaBuild> areas(Compare, AreaBuild.Create); // areas currently being processed, make thread-safe just in case
   Memc<EmbedObj>                  embed_objs; // embedded objects loaded for desired areas
   Memc<WaterMesh>                 waters; // waters loaded for desired areas
   RectI                           embed_objs_rect(0, -1), water_rect(0, -1);
   WorldVer                       *world_ver=null;
   Thread                          io_thread;
   SyncEvent                       io_wait;

  ~BuilderClass() {io_thread.stop(); io_wait.on(); io_thread.del();} // delete thread before other members

   // get
   bool processing(                    ) {return areas.elms()>0;} // if we're currently processing some areas
   bool finished  (C UID *world_id=null) {return !processing() && !Proj.worldAreasToRebuild(world_id);} // if builded all areas for rebuilt

   // operations
   void stop()
   {
      io_thread.stop(); io_wait.on(); // request the thread to stop
      io_thread.del ();               // wait for it to finish
      SyncLocker locker(lock);
      finished_processing=false;
      REPA(areas){AreaBuild &area=areas.lockedData(i); if(area.build && area.world_ver)area.world_ver.rebuildAreaNeighbor(area.xy, area.build);} // put back to world rebuild
      areas.del();
      embed_objs.del(); embed_objs_rect.set(0, -1);
      waters    .del();      water_rect.set(0, -1);
      world_ver=null;
   }
   bool process(WorldVer &world_ver, C ElmWorld &world_data, VecI2 area_xy)
   {
      TimeStamp now=CurTime; // this needs to be 'CurTime' instead of UTC (because curtime is always older) to prevent issues when area is ordered for rebuild, it is being built and in the same time new change got made with the same date
      if(AreaVer *area_ver=world_ver.areas.find(area_xy)) // get 'AreaVer' of that area
      if(!background || area_ver.old(now)) // process it only if it's old
      {
         ThreadSafeMap<VecI2, AreaBuild> areas(Compare, AreaBuild.Create, &world_ver); // specify 'world_ver' in the user member of 'areas' Map
         RectI                           embed_objs_rect(0, -1), water_rect(0, -1);
         Memc<WaterMesh>                 waters;

         // process in aligned blocks, need to cast to long because if area_xy==INT_MIN then int-based DivFloor will move the index too far so original will not be included
         area_xy.x=DivFloor((long)area_xy.x, (long)RebuildBlockSize)*RebuildBlockSize;
         area_xy.y=DivFloor((long)area_xy.y, (long)RebuildBlockSize)*RebuildBlockSize;

         // setup elements to build/load
         for(int y=-1; y<RebuildBlockSize+1; y++) // process with 1 extra border (so we have mesh and phys built on that borders)
         for(int x=-1; x<RebuildBlockSize+1; x++) // process with 1 extra border (so we have mesh and phys built on that borders)
         {
            bool  build_path=(InRange(x, RebuildBlockSize) && InRange(y, RebuildBlockSize)); // build PathMeshes only on 0..RebuildBlockSize range (not on borders)
            VecI2 xy=area_xy+VecI2(x, y);
            if(byte *area_build=world_ver.rebuild.find(xy))
            {
               byte want_area_build=*area_build; if(!build_path)FlagDisable(want_area_build, AREA_REBUILD_PATH); // skip AREA_REBUILD_PATH if we don't want to build it
               if(  want_area_build)
               {
                  if(background)if(AreaVer *area_ver=world_ver.areas.find(xy))if(!area_ver.old(now))continue; // don't process this area if it's not old
                  if(AreaBuild *area=areas.get(xy))
                  {
                     area.build=want_area_build;
                     uint neighbor_load=0;

                     if(area.build&AREA_REBUILD_HM)area.load|=AREA_LOAD_HEIGHTMAP;
                     if(area.build&AREA_REBUILD_HM_MESH)
                     {
                            area.load|=AREA_LOAD_HEIGHTMAP;
                        neighbor_load|=AREA_LOAD_HEIGHTMAP;
                     }
                     if(area.build&(AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS))area.load|=AREA_LOAD_HM_MESH;
                     if(area.build&AREA_REBUILD_PATH) // in order to build paths we need to have physics and water
                     {
                            area.load|=AREA_LOAD_HM_PHYS|AREA_LOAD_LOCAL_OBJ_PHYS|AREA_LOAD_LOCAL_WATER|AREA_MAKE_PATH_SRC;
                        neighbor_load|=AREA_LOAD_HM_PHYS|AREA_LOAD_LOCAL_OBJ_PHYS|AREA_LOAD_LOCAL_WATER|AREA_MAKE_PATH_SRC;
                     }

                     // check for embedded
                     if(area.build&(AREA_REBUILD_EMBED_OBJ|AREA_REBUILD_PATH)) // rebuilding paths also needs to get embedded objects, in order to get their phys bodies and path settings
                     {
                        if(!embed_objs_rect.valid())embed_objs_rect=xy;else embed_objs_rect|=xy;
                        if(area.build&AREA_REBUILD_PATH)embed_objs_rect|=RectI(xy).extend(1); // get from neighbors as well
                     }

                     // check for water
                     if(area.build&AREA_REBUILD_WATER)if(!water_rect.valid())water_rect=xy;else water_rect|=xy;

                     // apply load to neighbors
                     if(neighbor_load)
                        for(int y=-1; y<=1; y++)
                        for(int x=-1; x<=1; x++)if(x || y)if(AreaBuild *area=areas.get(VecI2(xy.x+x, xy.y+y)))area.load|=neighbor_load;
                  }
                  // try to remove if now empty
                  {
                     world_ver.rebuild.lock();
                     FlagDisable(*area_build, want_area_build); if(!*area_build){world_ver.rebuild.removeKey(xy); world_ver.changed=true;}
                     world_ver.rebuild.unlock();
                  }
               }
            }
         }

         if(areas.elms()) // if gathered areas to process
         {
            // clear loading if rebuilding is set
            REPA(areas)
            {
               AreaBuild &area_build=areas.lockedData(i);
               if(area_build.build&AREA_REBUILD_HM_MESH)FlagDisable(area_build.load, AREA_LOAD_HM_MESH    ); // don't load mesh  if it needs to be rebuilt
               if(area_build.build&AREA_REBUILD_HM_PHYS)FlagDisable(area_build.load, AREA_LOAD_HM_PHYS    ); // don't load phys  if it needs to be rebuilt
               if(area_build.build&AREA_REBUILD_WATER  )FlagDisable(area_build.load, AREA_LOAD_LOCAL_WATER); // don't load water if it needs to be rebuilt
            }

            // try to load data from current world
            if(&world_ver==WorldEdit.ver) // if we're processing currently edited world, then try to load data from areas that are already loaded
            {
               // area data
               REPA(areas)
               {
                  AreaBuild &area_build=areas.lockedData(i);
                  if(area_build.load) // want to load something
                     if(Cell<Area> *cell=WorldEdit.grid.find(area_build.xy))
                  {
                     Area &area=*cell.data();
                     if(area.loaded) // if area is loaded and we can take data from it
                     {
                        if(area_build.load&AREA_LOAD_HEIGHTMAP     ){FlagDisable(area_build.load, AREA_LOAD_HEIGHTMAP     ); if(area.hm)area_build.hm     .create(*area.hm     );}
                        if(area_build.load&AREA_LOAD_HM_MESH       ){FlagDisable(area_build.load, AREA_LOAD_HM_MESH       ); if(area.hm)area_build.hm_mesh.create( area.hm.mesh);}
                        if(area_build.load&AREA_LOAD_HM_PHYS       ){FlagDisable(area_build.load, AREA_LOAD_HM_PHYS       ); if(area.hm)area_build.hm_phys=        area.hm.phys ;}
                        if(area_build.load&AREA_LOAD_LOCAL_OBJ_PHYS){FlagDisable(area_build.load, AREA_LOAD_LOCAL_OBJ_PHYS); REPA(area.objs)if(Obj *obj=area.objs[i])if(obj.params.path!=OBJ_PATH_IGNORE && obj.phys && !obj.removed && !obj.embedded())area_build.objs.New().set(*obj);}
                        if(area_build.load&AREA_LOAD_LOCAL_WATER   ){FlagDisable(area_build.load, AREA_LOAD_LOCAL_WATER   ); area_build.waters=area.waters;}
                     }
                  }
               }

               // waters
               if(water_rect.valid())
               {
                  REPA(WorldEdit.ver.lakes )if(Cuts(water_rect, WorldEdit.ver.lakes .lockedData(i).areas))if(Lake  *lake =WorldEdit.lakes .find(WorldEdit.ver.lakes .lockedKey(i)))if(!lake .removed)waters.add(lake .checkMesh(Proj).water_mesh);
                  REPA(WorldEdit.ver.rivers)if(Cuts(water_rect, WorldEdit.ver.rivers.lockedData(i).areas))if(River *river=WorldEdit.rivers.find(WorldEdit.ver.rivers.lockedKey(i)))if(!river.removed)waters.add(river.checkMesh(Proj).water_mesh);
                  water_rect.set(0, -1); // set as invalid so waters will not try to be loaded again
               }
            }

            // setup thread
            {SyncLocker locker(lock); finished_processing=false; world_data.copyTo(path_settings); game_path=Proj.game_path; edit_path=Proj.edit_path; Swap(T.areas, areas); T.embed_objs.del(); T.embed_objs_rect=embed_objs_rect; Swap(T.waters, waters); T.water_rect=water_rect; T.world_ver=&world_ver;} // these elements should be changed only under critical section
            io_wait.on(); if(!io_thread.active())io_thread.create(IOFunc, this);
            return true;
         }
      }
      return false;
   }
   void update(bool background, C UID *priority_world_id=null)
   {
      T.background=background;

      // queue data for processing
      if(!processing()) // process only 1 world at a time
      {
         // first check for priority world
         if(priority_world_id && priority_world_id.valid())
            if(WorldVer *world_ver=Proj.worldVerFind(*priority_world_id))
            if(world_ver.rebuild.elms()) // if has any area to rebuild
            if(Elm      *world     =Proj.findElm(world_ver.world_id))
            if(ElmWorld *world_data=world.worldData())
            if(world_data.valid()) // if we have known information about the world
               REPA(world_ver.rebuild) // iterate all areas to rebuild
                  if(process(*world_ver, *world_data, world_ver.rebuild.lockedKey(i)))goto queued; // don't check other world/areas

         // then try to rebuild edited world's loaded areas
         if(WorldEdit.ver && WorldEdit.ver.rebuild.elms())
         if(WorldEdit.elm)
         if(ElmWorld *world_data=WorldEdit.elm.worldData())
            for(int y=WorldEdit.visible_area.min.y; y<=WorldEdit.visible_area.max.y; y++)
            for(int x=WorldEdit.visible_area.min.x; x<=WorldEdit.visible_area.max.x; x++)
               if(WorldEdit.ver.rebuild.find(VecI2(x, y)))
                  if(process(*WorldEdit.ver, *world_data, VecI2(x, y)))goto queued; // don't check other world/areas

         // process any world
         REPA(Proj.world_vers)
         {
            WorldVer &world_ver=Proj.world_vers.lockedData(i);
            if(world_ver.rebuild.elms()) // if has any area to rebuild
            if(Elm      *world     =Proj.findElm(world_ver.world_id))
            if(ElmWorld *world_data=world.worldData())
            if(world_data.valid()) // if we have known information about the world
               REPA(world_ver.rebuild) // iterate all areas to rebuild
                  if(process(world_ver, *world_data, world_ver.rebuild.lockedKey(i)))goto queued; // don't check other world/areas
         }
      queued:;
      }

      // check if finished processing
      if(finished_processing)
      {
         const uint time=Time.curTimeMs(), delay=(background ? 5 : 100); // 1000ms/60fps=16ms (use smaller value because these slowdowns can be noticable)
         SyncLocker locker(lock);
         // flush area data
         for(; areas.elms(); )
         {
            AreaBuild &area_build=areas.lockedData(areas.elms()-1); // get last, it will be removed once it's processed
            if(area_build.build) // if had something to build
               if(area_build.world_ver) // if has valid world
                  if(AreaVer *area_ver=area_build.world_ver.areas.find(area_build.xy)) // if exists in world
            {
               if(area_ver.newerHm(area_build.ver)) // if world area has at least one element newer than processed area then it means we can't set it
                  FlagDisable(area_build.build, AREA_REBUILD_HM|AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS); // remove heightmap stuff

               if(area_build.build) // if still has something
               {
                  // try putting into opened world
                  if(area_build.world_ver==WorldEdit.ver) // if area belongs to opened world
                     if(Cell<Area> *cell=WorldEdit.grid.find(area_build.xy))
                  {
                     Area &area=*cell.data();
                     if(area.loaded) // if area is loaded
                     {
                        if(area_build.build&AREA_REBUILD_HM) // has heightmap changed
                        {
                           if(area.hm)
                           {
                              Swap(SCAST(Heightmap, *area.hm), area_build.hm); // swap heightmaps too because 'cleanMaterials' was called, which is needed for validateRefs called below
                              Swap(area.hm.height_map, area_build.hm_height_map);
                              Swap(area.hm.  mtrl_map, area_build.hm_mtrl_map  );
                              Swap(area.hm.  mtrls   , area_build.hm_mtrls     ); area.setChanged();
                           }
                           area.delayedValidateRefs();
                        }
                        if(area_build.build&(AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY))if(area.hm) // has mesh changed
                        {
                           Swap(area.hm.mesh, area_build.hm_mesh); area.setShader(); area.setChanged();
                        }
                        if(area_build.build&AREA_REBUILD_HM_PHYS)if(area.hm) // has phys changed
                        {
                           Swap(area.hm.phys, area_build.hm_phys); area.setChanged();
                        }
                        if(area_build.build&AREA_REBUILD_EMBED_OBJ) // has embedded object changed
                        {
                           Swap(area.obj_mesh, area_build.obj_mesh); Swap(area.obj_phys, area_build.obj_phys); area.setChanged();
                        }
                        if(area_build.build&AREA_REBUILD_WATER) // has water changed
                        {
                           Swap(area.waters, area_build.waters); area.setChanged();
                        }
                        if(area_build.build&AREA_REBUILD_PATH) // has path changed
                        {
                           // unlink first, set new, and link to the PathWorld
                           WorldEdit.path_world.set(      null, area.xy); Swap(area.path, area_build.path); area.setChanged();
                           WorldEdit.path_world.set(&area.path, area.xy);
                        }
                        goto processed; // continue to next area
                     }
                  }

                  // put into file
                  Chunks chunks; chunks.load(area_build.game_path, WorldAreaSync); // load previous chunks
                  if(area_build.build&AREA_REBUILD_HM)
                  {
                     if(!area_build.hm_height_map.is())chunks.delChunk("Heightmap");else
                     {
                        File f; f.writeMem();
                        area_build.hm_height_map.save(f);
                        area_build.  hm_mtrl_map.save(f);
                        f.putInt(area_build.hm_mtrls.elms()); REPA(area_build.hm_mtrls)PutStr(f, area_build.hm_mtrls[i].valid() ? EncodeFileName(area_build.hm_mtrls[i]) : S); // in future version replace this with 'putAsset' and save in order
                        f.pos(0); chunks.setChunk("Heightmap", 0, f);
                     }
                  }
                  if(area_build.build&(AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY))
                  {
                     if(!area_build.hm_mesh.is())chunks.delChunk("HeightmapMesh");else
                     {
                        File f; area_build.hm_mesh.save(f.writeMem(), Proj.game_path); f.pos(0); chunks.setChunk("HeightmapMesh", 0, f);
                     }
                  }
                  if(area_build.build&AREA_REBUILD_HM_PHYS)
                  {
                     if(!area_build.hm_phys.is())chunks.delChunk("HeightmapPhys");else
                     {
                        File f; area_build.hm_phys.save(f.writeMem()); f.pos(0); chunks.setChunk("HeightmapPhys", 0, f);
                     }
                  }
                  if(area_build.build&AREA_REBUILD_EMBED_OBJ)
                  {
                     // mesh
                     {
                        if(!area_build.obj_mesh.is())chunks.delChunk("ObjectMesh");else
                        {
                           File f; area_build.obj_mesh.save(f.writeMem(), Proj.game_path); f.pos(0); chunks.setChunk("ObjectMesh", 0, f);
                        }
                     }
                     // phys
                     {
                        if(!area_build.obj_phys.is())chunks.delChunk("ObjectPhys");else
                        {
                           File f; area_build.obj_phys.save(f.writeMem()); f.pos(0); chunks.setChunk("ObjectPhys", 0, f);
                        }
                     }
                     if(area_build.build&AREA_REBUILD_GAME_AREA_OBJS)
                     {
                        Memc<ObjData> objs;
                        LoadEditObject(area_build.edit_path, objs, Proj.edit_path             ); // load current state of objects from edit ver
                        SaveGameObject(chunks              , objs, Proj, *area_build.world_ver); // save objects to game ver
                     }
                  }
                  if(area_build.build&AREA_REBUILD_PATH)
                  {
                     if(!area_build.path.is())chunks.delChunk("PathMesh");else
                     {
                        File f; area_build.path.save(f.writeMem()); f.pos(0); chunks.setChunk("PathMesh", 0, f);
                     }
                  }
                  if(area_build.build&AREA_REBUILD_WATER)
                  {
                     if(!area_build.waters.elms())chunks.delChunk("Water");else
                     {
                        File f; Save(f.writeMem(), area_build.waters, Proj.game_path); f.pos(0); chunks.setChunk("Water", 0, f);
                     }
                  }
                  chunks.save(area_build.game_path, WorldAreaSync); // warning: just saving a file can cause slowdowns of up to 40ms in rare cases (tested on Asus Zenbook Prime 2xSSD), but typically this should be fast
               }
            }
         processed:
            areas.remove(areas.elms()-1); // remove last (the one that we've just processed)
            if(Time.curTimeMs()-time>=delay)break; // stop if it's taking too long, process remaining areas in the next frame
         }
         if(!areas.elms())
         {
            embed_objs.del(); embed_objs_rect.set(0, -1);
            waters    .del();      water_rect.set(0, -1);
            world_ver=null;
            finished_processing=false;
         }
      }
   }
}
BuilderClass Builder;
/******************************************************************************/
void Build(Heightmap &hm, Mesh &mesh, flt area_size, C VecI2 &area_xy, Heightmap *l, Heightmap *r, Heightmap *b, Heightmap *f, Heightmap *lb, Heightmap *lf, Heightmap *rb, Heightmap *rf)
{
   // detect if there are gaps between neighbors
   int res=hm.resolution(), res1=res-1;
   if(l  && res==l.resolution())REP(res)if(!Equal(l.height(res1, i), hm.height(0   , i))){l=null; break;}
   if(r  && res==r.resolution())REP(res)if(!Equal(r.height(   0, i), hm.height(res1, i))){r=null; break;}
   if(b  && res==b.resolution())REP(res)if(!Equal(b.height(i, res1), hm.height(i, 0   ))){b=null; break;}
   if(f  && res==f.resolution())REP(res)if(!Equal(f.height(i,    0), hm.height(i, res1))){f=null; break;}
   if(lb && !Equal(lb.height(res1, res1), hm.height(   0,    0)))lb=null;
   if(lf && !Equal(lf.height(res1,    0), hm.height(   0, res1)))lf=null;
   if(rb && !Equal(rb.height(   0, res1), hm.height(res1,    0)))rb=null;
   if(rf && !Equal(rf.height(   0,    0), hm.height(res1, res1)))rf=null;

   // build
   hm.build(mesh, 0, HeightmapTexScale, HM_AO, l, r, b, f, lb, lf, rb, rf);
   mesh.scaleMove(area_size, (area_xy*area_size).x0y());
}
void Simplify(Mesh &mesh)
{
   mesh.setBase().simplify(1, 0.05, 1, 0.02, 0.02, 1, PI, true, SIMPLIFY_PLANES);

   // remove unused lods
   REPD(l, mesh.lods()) // go from the end
   {
      MeshLod &lod=mesh.lod(l);
      if(!lod.is()
      || (l>0 && lod.faces()>=mesh.lod(l-1).faces())) // if the LOD actually uses more faces than the base version
      {
         if(l==1) // create basing on the first LOD, in order to use the second LOD for lower shaders quality
         {
            flt dist2=lod.dist2; lod.create(mesh.lod(0));
                      lod.dist2=dist2;
         }else
         {
            mesh.removeLod(l); // remove MeshLod
         }
      }
   }

   mesh.setRender().delBase();
}
/******************************************************************************/
