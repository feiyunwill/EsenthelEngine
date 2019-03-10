/******************************************************************************/
enum AREA_SYNC_FLAG // these enums are not saved
{
   AREA_SYNC_HEIGHT =1<<0,
   AREA_SYNC_MTRL   =1<<1,
   AREA_SYNC_COLOR  =1<<2,
   AREA_SYNC_REMOVED=1<<3,
   AREA_SYNC_OBJ    =1<<4,
   AREA_SYNC_HM     =(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR|AREA_SYNC_REMOVED),
}
/******************************************************************************/
class AreaVer
{
   TimeStamp hm_removed_time, hm_height_time, hm_mtrl_time, hm_color_time,
             rebuild_time; // 'rebuild_time' doesn't need to be saved, it is only a helper member specifying when was the last rebuild ordered
   Version   obj_ver;

   static bool HasHm(C TimeStamp &removed_time, C TimeStamp &height_time) {return height_time>removed_time;}
          bool hasHm()C {return HasHm(hm_removed_time, hm_height_time);} // if has heightmap (not removed)

   // get
   bool newerHm(C AreaVer &ver)C
   {
      return hm_removed_time>ver.hm_removed_time || hm_height_time>ver.hm_height_time || hm_mtrl_time>ver.hm_mtrl_time || hm_color_time>ver.hm_color_time;
   }
   bool oldHm(C TimeStamp &now=TimeStamp().getUTC())C
   {
      return hm_removed_time.old(now) && hm_height_time.old(now) && hm_mtrl_time.old(now) && hm_color_time.old(now);
   }
   bool old(C TimeStamp &now=TimeStamp().getUTC())C
   {
      return oldHm(now) && rebuild_time.old(now);
   }
   uint compare(C AreaVer *dest)C // return which elements from 'this' should be sent to 'dest'
   {
      uint out=0;
      if(dest)
      {
         if(obj_ver!=dest.obj_ver)out|=AREA_SYNC_OBJ;
         if(hasHm() && dest.hasHm()) // both have heightmaps
         {
            out|=((hm_height_time>dest.hm_height_time) ? AREA_SYNC_HEIGHT : 0)
                |((  hm_mtrl_time>dest.  hm_mtrl_time) ? AREA_SYNC_MTRL   : 0)
                |(( hm_color_time>dest. hm_color_time) ? AREA_SYNC_COLOR  : 0);
         }else
         if(hasHm() && !dest.hasHm() && HasHm(dest.hm_removed_time, hm_height_time)) // this has heightmap, dest doesn't have, this will change dest state
         {
            out|=AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR;
         }else
         if(!hasHm() && dest.hasHm() && !HasHm(hm_removed_time, dest.hm_height_time)) // this has no heightmap, dest has heightmap, this will change dest state
         {
            out|=AREA_SYNC_REMOVED;
         }// else both_are_removed;
      }else
      {
         if(obj_ver)out|=AREA_SYNC_OBJ; // send only if have objects
         if(hasHm()) // send only if have heightmap
         {
            out|=AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR;
         }
      }
      return out;
   }

   // operations
   uint setHm(bool on) // make sure that 'hasHm' will return 'on'
   {
      if(on){if(!hasHm()){hm_height_time=hm_removed_time+1; return AREA_SYNC_HEIGHT ;}}
      else  {if( hasHm()){hm_removed_time=hm_height_time+1; return AREA_SYNC_REMOVED;}}
      return 0;
   }
   bool sync(C AreaVer &src)
   {
      bool changed=false;
      changed|=Sync(hm_removed_time, src.hm_removed_time);
      changed|=Sync(hm_height_time , src.hm_height_time );
      changed|=Sync(hm_mtrl_time   , src.hm_mtrl_time   );
      changed|=Sync(hm_color_time  , src.hm_color_time  );
      return changed;
   }
   uint sync(C AreaVer &src, Heightmap &hm, C Heightmap &src_hm, uint mask)
   {
      uint changed=0;
      int  res=src_hm.resolution(); // 'src' has newer data so use its resolution
      bool all=false;
      if(hm.is())hm.resize(res);else
      {
         hm.create(res, 0, null, false, null, null, null, null, null, null, null, null);
         if(src_hm.is())all=true; // if we're recreating heightmap completely then we need to set all values
      }

      if((mask&AREA_SYNC_HEIGHT) && (Sync(hm_height_time, src.hm_height_time) || all))
      {
         changed|=AREA_SYNC_HEIGHT;
         REPD(y, res)
         REPD(x, res)hm.height(x, y, src_hm.height(x, y));
      }
      if((mask&AREA_SYNC_MTRL) && (Sync(hm_mtrl_time, src.hm_mtrl_time) || all))
      {
         changed|=AREA_SYNC_MTRL;
         REPD(y, res)
         REPD(x, res)
         {
            MaterialPtr m[4]; VecB4 i;
            src_hm.getMaterial(x, y, m[0], m[1], m[2], m[3], i);
                hm.setMaterial(x, y, m[0], m[1], m[2], m[3], i);
         }
         hm.cleanMaterials();
      }
      if((mask&AREA_SYNC_COLOR) && (Sync(hm_color_time, src.hm_color_time) || all))
      {
         changed|=AREA_SYNC_COLOR;
         REPD(y, res)
         REPD(x, res)hm.color(x, y, src_hm.color(x, y));
         hm.cleanColor();
      }

      return changed;
   }
   uint undo(C AreaVer &src, Heightmap &hm, C Heightmap &src_hm)
   {
      uint changed=0;
      int  res=src_hm.resolution(); // 'src' has newer data so use its resolution
      bool all=false;
      if(hm.is())hm.resize(res);else
      {
         hm.create(res, 0, null, false, null, null, null, null, null, null, null, null);
         if(src_hm.is())all=true; // if we're recreating heightmap completely then we need to set all values
      }

      if(Undo(hm_height_time, src.hm_height_time) || all)
      {
         changed|=AREA_SYNC_HEIGHT;
         REPD(y, res)
         REPD(x, res)hm.height(x, y, src_hm.height(x, y));
      }
      if(Undo(hm_mtrl_time, src.hm_mtrl_time) || all)
      {
         changed|=AREA_SYNC_MTRL;
         REPD(y, res)
         REPD(x, res)
         {
            MaterialPtr m[4]; VecB4 i;
            src_hm.getMaterial(x, y, m[0], m[1], m[2], m[3], i);
                hm.setMaterial(x, y, m[0], m[1], m[2], m[3], i);
         }
         hm.cleanMaterials();
      }
      if(Undo(hm_color_time, src.hm_color_time) || all)
      {
         changed|=AREA_SYNC_COLOR;
         REPD(y, res)
         REPD(x, res)hm.color(x, y, src_hm.color(x, y));
      }
      return changed;
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(0);
      f<<hm_removed_time<<hm_height_time<<hm_mtrl_time<<hm_color_time<<obj_ver;
      return f.ok();
   }
   bool load(File &f)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>hm_removed_time>>hm_height_time>>hm_mtrl_time>>hm_color_time>>obj_ver;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
}
/******************************************************************************/
class ObjVer
{
   enum FLAG // !! these enums are saved !!
   {
      REMOVED           =1<<0,
      OVR_ACCESS        =1<<1,
      OVR_PATH          =1<<2,
      TERRAIN           =1<<3,
      PATH_SHIFT        =   4,
      PATH_MASK         = 1|2, // use 2 bits for mask to store up to 4 path modes
      OVR_MESH_VARIATION=1<<6,
   }
   VecI2       area_xy=0            ; // in which area the object is located                                (helper used to quickly locate the area and load object data)
   SmallMatrix matrix=MatrixIdentity; // object matrix                                                      (helper used to check if object needs to be stored as Terrain Mesh split into multiple areas)
   UID         elm_obj_id=UIDZero   ; // elm.id of the ELM_OBJ project element that this object is based on (helper used to check if object needs to be stored as Terrain Mesh split into multiple areas)
   uint        mesh_variation_id=0  ; // mesh variation id
   byte        flag=0               ; // FLAG

   // get / set
   bool     removed   (             )C {return FlagTest(flag, REMOVED);} // if object is removed
   bool     ovrPath   (             )C {return FlagTest(flag, OVR_PATH);}
   bool     meshVarOvr(             )C {return FlagTest(flag, OVR_MESH_VARIATION);}
   bool     terrain   (Project &proj)C {if(flag&OVR_ACCESS)return FlagTest(flag, TERRAIN); return proj.getObjTerrain(elm_obj_id);}
   OBJ_PATH path      (Project &proj)C {if(ovrPath()      )return pathSelf();              return proj.getObjPath   (elm_obj_id);}
   OBJ_PATH pathSelf  (             )C {return OBJ_PATH((flag>>PATH_SHIFT)&PATH_MASK);} // get path settings of self (don't check bases)

   bool set(C ObjData &obj, C VecI2 &area_xy) // return true if any member was changed
   {
      bool changed=false;
      uint flag   =0;
      FlagSet(flag, REMOVED           , obj.removed);
      FlagSet(flag, OVR_ACCESS        , FlagTest(obj.params.flag, EditObject.OVR_ACCESS        )); FlagSet(flag, TERRAIN, obj.params.access==OBJ_ACCESS_TERRAIN);
      FlagSet(flag, OVR_PATH          , FlagTest(obj.params.flag, EditObject.OVR_PATH          )); flag|=((obj.params.path&PATH_MASK)<<PATH_SHIFT);
      FlagSet(flag, OVR_MESH_VARIATION, FlagTest(obj.params.flag, EditObject.OVR_MESH_VARIATION));
                                                   if(T.flag             !=flag                        ){T.flag             =flag                        ; changed=true;}
                                                   if(T.area_xy          !=area_xy                     ){T.area_xy          =area_xy                     ; changed=true;}
                                                   if(T.mesh_variation_id!=obj.params.mesh_variation_id){T.mesh_variation_id=obj.params.mesh_variation_id; changed=true;}
      UID         elm_obj_id=obj.params.base.id(); if(T.elm_obj_id       !=elm_obj_id                  ){T.elm_obj_id       =elm_obj_id                  ; changed=true;}
      SmallMatrix matrix    =obj.matrix;           if(T.matrix           !=matrix                      ){T.matrix           =matrix                      ; changed=true;}
      return      changed;
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(2);
      f.cmpIntV(area_xy.x).cmpIntV(area_xy.y)<<flag<<elm_obj_id<<matrix<<mesh_variation_id;
      return f.ok();
   }
   bool load(File &f)
   {
      switch(f.decUIntV())
      {
         case 2:
         {
            f.decIntV(area_xy.x).decIntV(area_xy.y)>>flag>>elm_obj_id>>matrix>>mesh_variation_id;
            if(f.ok())return true;
         }break;

         case 1:
         {
            area_xy.x=DecIntV(f); area_xy.y=DecIntV(f); f>>flag>>elm_obj_id>>matrix>>mesh_variation_id;
            if(f.ok())return true;
         }break;

         case 0:
         {
            area_xy.x=DecIntV(f); area_xy.y=DecIntV(f); f>>flag>>elm_obj_id>>matrix; mesh_variation_id=0;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
}
/******************************************************************************/
class WaterVer
{
   Version ver;
   RectI   areas(0, -1); // set invalid at start

   bool removed()C {return !areas.valid();}

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(0);
      f<<ver<<areas;
      return f.ok();
   }
   bool load(File &f)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>ver>>areas;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
}
/******************************************************************************/
enum AREA_REBUILD_FLAG // !! these enums are saved !!
{
   AREA_REBUILD_HM_MESH         =1<<0,
   AREA_REBUILD_HM_MESH_SIMPLIFY=1<<1,
   AREA_REBUILD_HM_PHYS         =1<<2,
   AREA_REBUILD_EMBED_OBJ       =1<<3,
   AREA_REBUILD_GAME_AREA_OBJS  =1<<4, // if list of game area objects needs to be rebuilt
   AREA_REBUILD_WATER           =1<<5,
   AREA_REBUILD_PATH            =1<<6,
   AREA_REBUILD_HM              =1<<7, // heightmap lookup images (height, mtrl_map)
// newer flags require changing from byte to U16 (watch out for save/load)
}
class WorldVer
{
   static bool CreateRebuild (byte  &area_rebuild_flag, C VecI2 &area_xy, ptr) {area_rebuild_flag=0; return true;} // set empty   initially
   static bool CreateObjEmbed(RectI &obj_area         , C UID   &id     , ptr) {obj_area.set(0, -1); return true;} // set invalid initially

   bool                           changed=false; // if any member was changed, and the 'WorldVer' needs to be flushed to disk
   Str                            path;
   UID                            world_id=UIDZero;
   ThreadSafeMap<VecI2, AreaVer > areas    (Compare                ); // make this thread-safe because Area.getVer uses this and needs to be thread-safe
   ThreadSafeMap<VecI2, byte    > rebuild  (Compare, CreateRebuild ); // AREA_REBUILD_FLAG areas that need to be rebuilt, make this thread-safe just in case
   ThreadSafeMap<UID  , ObjVer  > obj      (Compare                ); // !! can be modified only after locking !!       , make this thread-safe just in case
   ThreadSafeMap<UID  , RectI   > obj_embed(Compare, CreateObjEmbed); // map of objects that are currently embedded !! can be modified only after locking !!, make this thread-safe just in case
   ThreadSafeMap<UID  , Version > waypoints(Compare                ); // make this thread-safe just in case
   ThreadSafeMap<UID  , WaterVer> lakes    (Compare                ); // make this thread-safe just in case
   ThreadSafeMap<UID  , WaterVer> rivers   (Compare                ); // make this thread-safe just in case

  ~WorldVer() {flush();}
   WorldVer& setChanged() {changed=true; return T;}
   void operator=(C WorldVer &src)
   {
      setChanged();
      // don't set path, to avoid accidental resaving from a temporary variable
      // don't set world_id
      areas=src.areas;
      rebuild=src.rebuild;
      obj=src.obj;
      obj_embed=src.obj_embed;
      waypoints=src.waypoints;
      lakes=src.lakes;
      rivers=src.rivers;
   }

   // get
   bool embedded(C ObjData &obj) {return obj_embed.find(obj.id)!=null;} // !! this function must be multi-threaded SAFE !!

   bool hasHm(C VecI2 &area_xy) {if(AreaVer *area_ver=areas.find(area_xy))return area_ver.hasHm(); return false;}

   RectI getTerrainAreas()
   {
      RectI  rect(0, -1); MapLock ml(areas); REPA(areas)if(areas.lockedData(i).hasHm())Include(rect, areas.lockedKey (i));
      return rect;
   }
   RectI getObjAreas()
   {
      RectI  rect(0, -1); MapLock ml(obj); REPA(obj)if(!obj.lockedData(i).removed())Include(rect, obj.lockedData(i).area_xy);
      return rect;
   }
   RectI getObjEmbedAreas()
   {
      RectI  rect(0, -1); MapLock ml(obj_embed); REPA(obj_embed)Include(rect, obj_embed.lockedData(i));
      return rect;
   }
   RectI getLakeAreas()
   {
      RectI  rect(0, -1); MapLock ml(lakes); REPA(lakes)if(!lakes.lockedData(i).removed())Include(rect, lakes.lockedData(i).areas);
      return rect;
   }
   RectI getRiverAreas()
   {
      RectI  rect(0, -1); MapLock ml(rivers); REPA(rivers)if(!rivers.lockedData(i).removed())Include(rect, rivers.lockedData(i).areas);
      return rect;
   }

   // rebuild
   void rebuildAreaNeighbor(C VecI2 &area_xy, uint flag, uint neighbor_flag_if_exists=0) // !! 'neighbor_flag_if_exists' is processed only if that neighbor area already exists !!
   {
      if(!IsServer) // 'rebuild' doesn't need to be stored on the server
         if(flag || neighbor_flag_if_exists)
      {
         MapLock ml(rebuild); // to be multi-thread safe

         if(flag)
            if(AreaVer *area_ver=areas.get(area_xy))
               if(byte *f=rebuild.get(area_xy)){area_ver.rebuild_time=CurTime; if((*f&flag)!=flag){*f|=flag; setChanged();}}

         if(neighbor_flag_if_exists)
            for(int y=-1; y<=1; y++)
            for(int x=-1; x<=1; x++)if(x || y) // skip 'area_xy'
         {
            VecI2 xy=area_xy+VecI2(x, y);
            if(AreaVer *area_ver=areas.find(xy))
            {
               uint nf=neighbor_flag_if_exists; if(!area_ver.hasHm())FlagDisable(nf, AREA_REBUILD_HM|AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS); // if neighbor doesn't have heightmap then don't rebuild these elements
               if(  nf)if(byte *f=rebuild.get(xy)){area_ver.rebuild_time=CurTime; if((*f&nf)!=nf){*f|=nf; setChanged();}}
            }
         }
      }
   }
   void rebuildArea(C VecI2 &area_xy, uint sync_flag, bool skip_mesh=false)
   {
      uint rebuild_flag=0, rebuild_neighbor_flag=0;
      if(sync_flag& AREA_SYNC_COLOR                                   )rebuild_flag         |=                AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY;
      if(sync_flag&(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_REMOVED))rebuild_flag         |=AREA_REBUILD_HM|AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS|AREA_REBUILD_PATH; // check 'AREA_SYNC_MTRL' too because it can create holes
      if(sync_flag&(AREA_SYNC_HEIGHT|               AREA_SYNC_REMOVED))rebuild_neighbor_flag|=                AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY                     |AREA_REBUILD_PATH; // check for 'AREA_SYNC_HEIGHT' and 'AREA_SYNC_REMOVED' because terrain shape affects ambient occlusion and vtx normals of neighbor areas, height can affect neighbor pathmesh too
      if(skip_mesh)FlagDisable(rebuild_flag, AREA_REBUILD_HM_MESH); // do not adjust 'rebuild_neighbor_flag' too, because the optimizations that use 'skip_mesh' will rebuild HM_MESH only for that area, but neighbor rebuild is not guaranteed
      rebuildAreaNeighbor(area_xy, rebuild_flag, rebuild_neighbor_flag);
   }
   void rebuildPaths(C RectI &area)
   {
      RectI a=area; a.extend(1); // extra 1 border is needed for paths, even if we're rebuilding a single area due to object change, because its path can span across multiple areas (if the object is bigger than area, then it's set as embedded and 'area' rect will cover those areas, so we don't need to extend it more)
      for(int x=a.min.x; x<=a.max.x; x++)
      for(int y=a.min.y; y<=a.max.y; y++)rebuildAreaNeighbor(VecI2(x, y), AREA_REBUILD_PATH);
   }
   void rebuildPaths(C UID &obj_id, C VecI2 &obj_area_xy)
   {
      if(C RectI *embed_rect=obj_embed.find(obj_id))rebuildPaths(*embed_rect); // if is embedded then rebuild all covered areas
      else                                          rebuildPaths(obj_area_xy); // rebuild paths at object area
   }
   void rebuildEmbedObj(C RectI &area)
   {
      for(int x=area.min.x; x<=area.max.x; x++)
      for(int y=area.min.y; y<=area.max.y; y++)rebuildAreaNeighbor(VecI2(x, y), AREA_REBUILD_EMBED_OBJ);
      // we must also rebuild paths for that areas
      rebuildPaths(area);
   }
   void rebuildGameAreaObjs(C VecI2 &area_xy) {rebuildAreaNeighbor(area_xy, AREA_REBUILD_GAME_AREA_OBJS);}
   void rebuildWater(C RectI &area)
   {
      for(int x=area.min.x; x<=area.max.x; x++)
      for(int y=area.min.y; y<=area.max.y; y++)rebuildAreaNeighbor(VecI2(x, y), AREA_REBUILD_WATER);
      // we must also rebuild paths for that areas
      rebuildPaths(area);
   }

   // changed
   void changedObj(C ObjData &obj, C VecI2 &area_xy)
   {
      MapLock ml(T.obj); // lock before modifying
      if(ObjVer *obj_ver=T.obj.get(obj.id))if(obj_ver.set(obj, area_xy))setChanged();
   }
   void changedWaypoint(C UID &waypoint_id) {waypoints(waypoint_id).    randomize(); setChanged();}
   void changedLake    (C UID &    lake_id) {lakes    (    lake_id).ver.randomize(); setChanged();}
   void changedRiver   (C UID &   river_id) {rivers   (   river_id).ver.randomize(); setChanged();}

   // io
   bool save(File &f, bool network=false)
   {
      f.cmpUIntV(0);
                   f.cmpUIntV(areas    .elms()); FREPA(areas    ){f<<areas    .lockedKey(i);    areas    .lockedData(i).save(f);}
      if(!network){f.cmpUIntV(rebuild  .elms()); FREPA(rebuild  ){f<<rebuild  .lockedKey(i); f<<rebuild  .lockedData(i);}} // every client maintains the rebuild list locally so no need to send it over
      if(!network){f.cmpUIntV(obj      .elms()); FREPA(obj      ){f<<obj      .lockedKey(i);    obj      .lockedData(i).save(f);}} // every client maintains the object quick list according to their local data, so no need to send it over
      if(!network){f.cmpUIntV(obj_embed.elms()); FREPA(obj_embed){f<<obj_embed.lockedKey(i); f<<obj_embed.lockedData(i)        ;}} // every client maintains the object quick list according to their local data, so no need to send it over
                   f.cmpUIntV(waypoints.elms()); FREPA(waypoints){f<<waypoints.lockedKey(i); f<<waypoints.lockedData(i);}
                   f.cmpUIntV(lakes    .elms()); FREPA(lakes    ){f<<lakes    .lockedKey(i);    lakes    .lockedData(i).save(f);}
                   f.cmpUIntV(rivers   .elms()); FREPA(rivers   ){f<<rivers   .lockedKey(i);    rivers   .lockedData(i).save(f);}
      return f.ok();
   }
   bool load(File &f, bool network=false)
   {
      changed=false;
      areas    .del();
      rebuild  .del();
      obj      .del();
      obj_embed.del();
      waypoints.del();
      lakes    .del();
      rivers   .del();
      switch(f.decUIntV())
      {
         case 0:
         {
                        REP(f.decUIntV()){VecI2 xy; f>>xy; if(!areas    (xy).load(f))goto error;}
            if(!network)REP(f.decUIntV()){VecI2 xy; f>>xy; f>>*rebuild  (xy);}
            if(!network)REP(f.decUIntV()){UID   id; f>>id; if(!obj      (id).load(f))goto error;}
            if(!network)REP(f.decUIntV()){UID   id; f>>id; f>>*obj_embed(id);}
                        REP(f.decUIntV()){UID   id; f>>id; f>>*waypoints(id);}
                        REP(f.decUIntV()){UID   id; f>>id; if(!lakes    (id).load(f))goto error;}
                        REP(f.decUIntV()){UID   id; f>>id; if(!rivers   (id).load(f))goto error;}
            if(f.ok())return true;
         }break;
      }
   error:
      areas.del(); rebuild.del(); obj.del(); obj_embed.del(); waypoints.del(); lakes.del(); rivers.del();
      return false;
   }
   bool save(C Str &name)
   {
      File f; save(f.writeMem()); f.pos(0); return SafeOverwrite(f, name);
   }
   bool load(C Str &name) // name is "<WorldID>\\Data"
   {
      T.path=name;
      T.world_id=FileNameID(GetPath(name)); // ignore "Data" and grab the WorldID
      File f; if(f.readTry(name))return load(f);
      return false;
   }

   void flush()
   {
      if(changed)
      {
         changed=false;
         if(path.is())save(path);
      }
   }
}
/******************************************************************************/
class MiniMapVer
{
   bool                  changed=false;
   TimeStamp             time; // this is time of both settings and images, images will be received only if their TimeStamp matches this value
   Game.MiniMap.Settings settings;
   Str                   path;
   UID                   mini_map_id=UIDZero;
   Memc<VecI2>           images;

  ~MiniMapVer() {flush();}
   MiniMapVer& setChanged() {changed=true; return T;}
   void operator=(C MiniMapVer &src)
   {
      setChanged();
      // don't set path, to avoid accidental resaving from a temporary variable
      // don't set mini_map_id
      time=src.time;
      settings=src.settings;
      images=src.images;
   }

   // io
   bool save(File &f, bool network=false)C
   {
      f.cmpUIntV(1);
      f<<time;
      settings.save(f);
      f.cmpUIntV(images.elms()); FREPA(images)f.cmpIntV(images[i].x).cmpIntV(images[i].y);
      return f.ok();
   }
   bool load(File &f, bool network=false)
   {
      changed=false;
      switch(f.decUIntV())
      {
         case 1:
         {
            f>>time;
            if(!settings.load(f))break;
            images.setNum(f.decUIntV()); FREPA(images)f.decIntV(images[i].x).decIntV(images[i].y);
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>time;
            if(!settings.load(f))break;
            images.setNum(f.decUIntV()); FREPA(images){images[i].x=DecIntV(f); images[i].y=DecIntV(f);}
            if(f.ok())return true;
         }break;
      }
      time   .zero();
      settings.del();
      images  .del();
      return false;
   }
   bool save(C Str &name)C
   {
      File f; save(f.writeMem()); f.pos(0); return SafeOverwrite(f, name);
   }
   bool load(C Str &name)
   {
      T.path=name;
      T.mini_map_id=FileNameID(name);
      File f; if(f.readTry(name))return load(f);
      return false;
   }

   void flush()
   {
      if(changed)
      {
         changed=false;
         if(path.is())save(path);
      }
   }
}
/******************************************************************************/
bool LoadEditHeightmap(C Str &name, Heightmap &hm, C Str &game_path)
{
   ReadLock rl(WorldAreaSync);
   File f; if(f.readTry(name))for(ChunkReader cr(f); File *f=cr(); )
   {
      if(EqualPath(cr.name(), "Heightmap"))switch(cr.ver())
      {
         case 0: return hm.load(*f, game_path);
      }
   }
   return false;
}
void SaveEditObject(Chunks &chunks, Memc<ObjData> &objs, C Str &edit_path)
{
   if(!objs.elms())chunks.delChunk("Object");else
   {
      File f; f.writeMem(); objs.save(f, edit_path); f.pos(0); chunks.setChunk("Object", 0, f);
   }
}
void SaveGameObject(Chunks &chunks, Memc<ObjData> &objs, C Project &proj, WorldVer &world_ver)
{
   REPA(objs)if(!objs[i].removed && !world_ver.embedded(objs[i])) // check if there's at least one existing object
   {
      File f; f.writeMem();
      Memt<Game.Area.Data.AreaObj> area_objs; FREPA(objs)if(!objs[i].removed && !world_ver.embedded(objs[i]))objs[i].copyTo(area_objs.New(), proj);
      area_objs.sort(CompareObj); FREPAO(area_objs).save(f, proj.game_path);
      f.pos(0); chunks.setChunk("Object", 1, f);
      return;
   }
   chunks.delChunk("Object");
}
bool LoadEditObject(int ver, File &f, Memc<ObjData> &objs, C Str &edit_path)
{
   switch(ver)
   {
      case 0: return objs.load(f, edit_path);
   }
   return false;
}
bool LoadEditObject(C Str &name, Memc<ObjData> &objs, C Str &edit_path)
{
   ReadLock rl(WorldAreaSync);
   File f; if(f.readTry(name))for(ChunkReader cr(f); File *f=cr(); )
   {
      if(EqualPath(cr.name(), "Object"))return LoadEditObject(cr.ver(), *f, objs, edit_path);
   }
   return false;
}
/******************************************************************************/
bool LoadEdit(C Str &name, Heightmap *hm, Memc<ObjData> *objs, C Str &game_path, C Str &edit_path)
{
   ReadLock rl(WorldAreaSync);
   File f; if(f.readTry(name))
   {
      for(ChunkReader cr(f); File *f=cr(); )
      {
         if(hm && EqualPath(cr.name(), "Heightmap"))switch(cr.ver())
         {
            case 0: hm.load(*f, game_path); break;
         }else
         if(objs && EqualPath(cr.name(), "Object"))LoadEditObject(cr.ver(), *f, *objs, edit_path);
      }
      return true;
   }
   return false;
}
/******************************************************************************/
bool LoadGame(C Str &name, Mesh *hm_mesh, PhysPart *hm_phys, Memc<Game.Area.Data.AreaObj> *objs, MeshGroup *obj_mesh, PhysBody *obj_phys, Memc<WaterMesh> *waters, C Str &game_path)
{
   ReadLock rl(WorldAreaSync);
   File f; if(f.readTry(name))
   {
      for(ChunkReader cr(f); File *f=cr(); )
      {
         if(hm_mesh && EqualPath(cr.name(), "HeightmapMesh"))switch(cr.ver())
         {
            case 0: hm_mesh.load(*f, game_path); break;
         }else
         if(hm_phys && EqualPath(cr.name(), "HeightmapPhys"))switch(cr.ver())
         {
            case 0: hm_phys.load(*f); break;
         }else
         if(obj_mesh && EqualPath(cr.name(), "ObjectMesh"))switch(cr.ver())
         {
            case 0: obj_mesh.load(*f, game_path); break;
         }else
         if(obj_phys && EqualPath(cr.name(), "ObjectPhys"))switch(cr.ver())
         {
            case 0: obj_phys.load(*f, game_path); break;
         }else
         if(objs && EqualPath(cr.name(), "Object"))switch(cr.ver())
         {
            case 1: for(; !f.end(); )if(!objs.New().load(*f, game_path)){objs.removeLast(); break;} break;
         }else
         if(waters && EqualPath(cr.name(), "Water"))switch(cr.ver())
         {
            case 0: Load(*f, *waters, game_path); break;
         }
      }
      return true;
   }
   return false;
}
/******************************************************************************/
