/******************************************************************************/
class WorldChange : Edit._Undo.Change
{
   class Terrain : Heightmap
   {
      AreaVer ver;

      void set(C Area &area)
      {
         if(C AreaVer *v=area.getVer())ver=*v;
         if(area.hm)create(*area.hm);
      }
   }

   // need to be thread-safe
   ThreadSafeMap<VecI2, Terrain     >   terrain(Compare);
   ThreadSafeMap<UID  , ObjData     >      objs(Compare);
   ThreadSafeMap<UID  , EditWaypoint> waypoints(Compare);
   ThreadSafeMap<UID  ,  LakeBase   >     lakes(Compare);
   ThreadSafeMap<UID  , RiverBase   >    rivers(Compare);

   virtual uint memUsage()C override
   {
      uint size=0;
      REPA(  terrain)size+=  terrain.lockedData(i).memUsage();
      REPA(     objs)size+=     objs.lockedData(i).memUsage();
      REPA(waypoints)size+=waypoints.lockedData(i).memUsage();
      REPA(    lakes)size+=    lakes.lockedData(i).memUsage();
      REPA(   rivers)size+=   rivers.lockedData(i).memUsage();
      return size;
   }
   virtual void swap(ptr user)override
   {
      // terrain
      Memt<Area*> rebuild;
      REPA(terrain)if(Area *area=WorldEdit.getArea(terrain.lockedKey(i)))
      {
         bool   loaded=area.loaded; area.load();
         Terrain &undo=terrain.lockedData(i), temp;
         temp.set(*area); area.undo(undo, loaded); Swap(undo, temp); // if the area is loaded then skip rebuilding mesh through the Builder because we will do it here
         if(loaded)rebuild.add(area); // want to keep in memory so rebuild immediatelly
         else         area.unload( );
      }
      WorkerThreads.process1(rebuild, WorldView.HmBuild); rebuild.clear();

      // objects
      if(objs.elms() && WorldEdit.ver) // 'rebuild' is treated as 'unload_areas'
      {
         REPA(objs)
         {
            ObjData &undo=objs.lockedData(i), temp;
            if(C ObjVer *obj_ver=WorldEdit.ver.obj.find(undo.id))
               if(Area *world_area=WorldEdit.getArea(obj_ver.area_xy))
            {
               if(!world_area.loaded)rebuild.include(world_area); world_area.load();
               if(Obj *world=world_area.findobj(undo.id))
               if(Area *undo_area=WorldEdit.getArea(WorldEdit.worldToArea(undo.matrix.pos))) // get undo area
               {
                  if(!undo_area.loaded)rebuild.include(undo_area); // if the 'undo_area' isn't loaded before 'undo' then remember it for possible unload
                  temp=*world;

                  // perform undo
                  TerrainObj old_terrain          =world.terrainObj();
                  PhysPath   old_phys             =world.physPath();
                  TimeStamp  old_matrix_time      =world.matrix_time; // remember old matrix time before undo
                  uint       old_mesh_variation_id=world.params.mesh_variation_id;
                  if(world.undo(undo, Proj.edit_path))
                  {
                     bool changed_matrix        =(world.matrix_time             !=old_matrix_time),
                          changed_mesh_variation=(world.params.mesh_variation_id!=old_mesh_variation_id),
                          changed_embed         =(world.terrainObj()!=old_terrain ||  changed_matrix || changed_mesh_variation), // if changed terrain, matrix or mesh variation
                          changed_phys_path     =(world.physPath  ()!=old_phys    || (changed_matrix && (old_phys || world.physPath()))); // if changed phys, or if changed matrix and have phys

                     world_area.setChangedObj(*world); // call for old area before 'setChangedEmbed'
                     if(changed_embed)world.setChangedEmbed(&undo_area.xy); // call after 'changedObj' and before 'setChangedPaths' (use new area coordinates)
                     if(changed_phys_path)world.setChangedPaths(); // call 'setChangedPaths' after 'setChangedEmbed' for old area
                     world.attach(WorldEdit, undo_area); // attach to new area
                     if(changed_phys_path)world.setChangedPaths(); // call 'setChangedPaths' after 'setChangedEmbed' for new area
                     undo_area.setChangedObj(*world); // call for new area at the end
                  }

                  Swap(undo, temp);
               }
            }
         }
         WorldEdit.objToGui();
         REPA(rebuild)if(!rebuild[i].hasSelectedObj())rebuild[i].unload(); rebuild.clear();
      }

      // waypoints
      if(waypoints.elms())
      {
         REPA(waypoints)
            if(EditWaypoint *world=WorldEdit.waypoints.find(waypoints.lockedKey(i)))
         {
            EditWaypoint &undo=waypoints.lockedData(i), temp=*world; world.undo(undo); Swap(undo, temp); WorldEdit.setChangedWaypoint(world);
         }
         WorldEdit.setVisibleWaypoints();
         WaypointList.setChanged();
      }
      
      // lakes and rivers
      if(lakes.elms() || rivers.elms())
      {
         REPA(lakes)
            if(Lake *world=WorldEdit.lakes.find(lakes.lockedKey(i)))
         {
            LakeBase &undo=lakes.lockedData(i), temp=*world; world.undo(undo); Swap(undo, temp); WorldEdit.setChangedLake(world);
         }
         REPA(rivers)
            if(River *world=WorldEdit.rivers.find(rivers.lockedKey(i)))
         {
            RiverBase &undo=rivers.lockedData(i), temp=*world; world.undo(undo); Swap(undo, temp); WorldEdit.setChangedRiver(world);
         }
         WorldEdit.setVisibleWaters();
      }
      WorldEdit.undoVis();
   }
}
/******************************************************************************/
class WorldUndo : Edit.Undo<WorldChange>
{
   static cchar8 * TerrainType="Terrain" ,
                 *     ObjType="Obj"     ,
                 *  NewObjType="ObjNew"  ,
                 *WaypointType="Waypoint";

   SyncLock lock;

   WorldUndo() : Edit.Undo<WorldChange>(false) {}

   void del() {super.del(); WorldEdit.undoVis();}

   WorldChange* set(cptr type, bool force_create=false) // !! this may get called on multiple threads when processing different areas !!
   {
      SyncLocker locker(lock);
      WorldChange *change=super.set(type, force_create);
      WorldEdit.undoVis();
      return change;
   }
   void setTerrain(C Area &area) // !! this may get called on multiple threads when processing different areas !!
   {
      if(WorldChange *change=set(TerrainType))if(!change.terrain.find(area.xy))change.terrain(area.xy).set(area);
   }
   void set(C ObjData &obj, cptr type, bool as_new)
   {
      if(WorldChange *change=set(type))if(!change.objs.find(obj.id))
      {
         ObjData &undo=*change.objs(obj.id); undo=obj; if(as_new)undo.removed=true; // if we've just created this object, then set that 'undo' was originally removed (object didn't exist yet), so when this undo is applied, then object will be removed
      }
   }
   void set(C EditWaypoint &waypoint, cptr type=WaypointType)
   {
      if(C UID *waypoint_id=WorldEdit.ID(&waypoint))if(WorldChange *change=set(type))if(!change.waypoints.find(*waypoint_id))*change.waypoints(*waypoint_id)=waypoint;
   }
   void set(C Lake &lake)
   {
      if(C UID *lake_id=WorldEdit.ID(&lake))if(WorldChange *change=set("lake"))if(!change.lakes.find(*lake_id))*change.lakes(*lake_id)=lake;
   }
   void set(C River &river)
   {
      if(C UID *river_id=WorldEdit.ID(&river))if(WorldChange *change=set("river"))if(!change.rivers.find(*river_id))*change.rivers(*river_id)=river;
   }
}
/******************************************************************************/
