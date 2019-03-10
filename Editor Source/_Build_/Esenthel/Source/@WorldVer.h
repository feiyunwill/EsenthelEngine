/******************************************************************************/
class WorldVer
{
   static bool CreateRebuild (byte  &area_rebuild_flag, C VecI2 &area_xy, ptr); // set empty   initially
   static bool CreateObjEmbed(RectI &obj_area         , C UID   &id     , ptr); // set invalid initially

   bool                           changed; // if any member was changed, and the 'WorldVer' needs to be flushed to disk
   Str                            path;
   UID                            world_id;
   ThreadSafeMap<VecI2, AreaVer > areas    ; // make this thread-safe because Area.getVer uses this and needs to be thread-safe
   ThreadSafeMap<VecI2, byte    > rebuild  ; // AREA_REBUILD_FLAG areas that need to be rebuilt, make this thread-safe just in case
   ThreadSafeMap<UID  , ObjVer  > obj      ; // !! can be modified only after locking !!       , make this thread-safe just in case
   ThreadSafeMap<UID  , RectI   > obj_embed; // map of objects that are currently embedded !! can be modified only after locking !!, make this thread-safe just in case
   ThreadSafeMap<UID  , Version > waypoints; // make this thread-safe just in case
   ThreadSafeMap<UID  , WaterVer> lakes    ; // make this thread-safe just in case
   ThreadSafeMap<UID  , WaterVer> rivers   ; // make this thread-safe just in case

  ~WorldVer();          
   WorldVer& setChanged();        
   void operator=(C WorldVer&src);

   // get
   bool embedded(C ObjData &obj); // !! this function must be multi-threaded SAFE !!

   bool hasHm(C VecI2 &area_xy);

   RectI getTerrainAreas();
   RectI getObjAreas();
   RectI getObjEmbedAreas();
   RectI getLakeAreas();
   RectI getRiverAreas();

   // rebuild
   void rebuildAreaNeighbor(C VecI2 &area_xy, uint flag, uint neighbor_flag_if_exists=0); // !! 'neighbor_flag_if_exists' is processed only if that neighbor area already exists !!
   void rebuildArea(C VecI2 &area_xy, uint sync_flag, bool skip_mesh=false);
   void rebuildPaths(C RectI &area);
   void rebuildPaths(C UID &obj_id, C VecI2 &obj_area_xy);
   void rebuildEmbedObj(C RectI &area);
   void rebuildGameAreaObjs(C VecI2 &area_xy);
   void rebuildWater(C RectI &area);

   // changed
   void changedObj(C ObjData &obj, C VecI2 &area_xy);
   void changedWaypoint(C UID &waypoint_id);       
   void changedLake    (C UID &    lake_id);       
   void changedRiver   (C UID &   river_id);       

   // io
   bool save(File &f, bool network=false);
   bool load(File &f, bool network=false);
   bool save(C Str &name);
   bool load(C Str &name); // name is "<WorldID>\\Data"

   void flush();

public:
   WorldVer();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
