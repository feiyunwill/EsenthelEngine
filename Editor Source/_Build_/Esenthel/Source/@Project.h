/******************************************************************************/
// Project Data
/******************************************************************************/
class Project
{
   bool              text_data, synchronize;
   byte              cipher_key[256], compress_level;
   CIPHER_TYPE       cipher;
   COMPRESS_TYPE     compress_type;
   MATERIAL_SIMPLIFY material_simplify;
   TimeStamp         cipher_time, cipher_key_time, compress_type_time, compress_level_time, material_simplify_time;
   UID               id, app_id, hm_mtrl_id, water_mtrl_id, mtrl_brush_id[MtrlBrushSlots];
   Str               name, path, code_path, code_base_path, edit_path, game_path, temp_path, tex_path, temp_tex_path, temp_tex_dynamic_path;
   Memx<Elm>         elms;
   Memc<UID>         texs, texs_update;
   Memc<UID>         world_paths, mini_map_paths;
   Cache<WorldVer>   world_vers;
   Cache<MiniMapVer> mini_map_vers;

   Project();
   
   virtual Project& del();

   static int CompareID(C Elm &elm, C UID &id);

   // get
   bool valid()C;     
   bool needUpdate()C;

   bool  hasElm (C UID &id)C;
   int  findElmI(C UID &id)C;
   Elm* findElm (C UID &id); 
 C Elm* findElm (C UID &id)C;
   Elm&  getElm (C UID &id);  // get element of specified 'id', if it doesn't exist yet then create new one
   Elm&  newElm (         ); // create new element with random id
   Elm& newElm(C Str &name, C UID &parent_id, ELM_TYPE type); // use custom times for 'setName' and 'setParent' so those times don't need to be sent to the server in the new element command, instead they will always be assumed 'NewElmTime' (check 'decompressNew')

   virtual Elm* findElmByPath(C Str &path);
   Elm* findElm(C Str &text);
   UID  findElmID(C Str &text);

   Elm* findElm  (C UID &id  , ELM_TYPE type); 
 C Elm* findElm  (C UID &id  , ELM_TYPE type)C;
   Elm* findElm  (C Str &text, ELM_TYPE type); 
   UID  findElmID(C UID &id  , ELM_TYPE type); 
   UID  findElmID(C Str &text, ELM_TYPE type); 

   Elm* findElmImage  (C Str &text);
   UID  findElmImageID(C Str &text);

   Elm* findElmByTexture(C UID &tex_id);

           Str elmFullName(C UID &elm_id, int max_elms=-1)C;
   virtual Str elmFullName(C Elm *elm   , int max_elms=-1)C;

   Str basePath       (C Elm &elm   )C;                             
   Str codeBasePath   (C UID &elm_id)C;                             
   Str codePath       (C UID &elm_id)C;                             
   Str editPath       (C UID &elm_id)C;                             
   Str gamePath       (C UID &elm_id)C;                             
   Str codeBasePath   (C Elm &elm   )C;                             
   Str codePath       (C Elm &elm   )C;                             
   Str editPath       (C Elm &elm   )C;                             
   Str gamePath       (C Elm &elm   )C;                             
   Str  texPath       (C UID &tex_id)C;                             
   Str  texDynamicPath(C UID &tex_id)C;                             
   Str  texFormatPath (C UID &tex_id, cchar8*format, int downsize)C;
   Str     formatPath (C UID &elm_id, cchar8*suffix              )C;

   Str gameAreaPath(C UID &world_id, C VecI2 &area_xy)C;
   Str editAreaPath(C UID &world_id, C VecI2 &area_xy)C;

   Str gameWaypointPath(C UID &world_id, C UID &waypoint_id)C;
   Str editWaypointPath(C UID &world_id, C UID &waypoint_id)C;

   bool waypointExists(C UID &world_id, C UID &waypoint_id)C; // waypoint game file can be deleted when it was removed so we need to check for file existence

   static Str EditLakePath (C Str &edit_path, C UID &world_id, C UID & lake_id); 
   static Str EditRiverPath(C Str &edit_path, C UID &world_id, C UID &river_id); 
          Str editLakePath (                  C UID &world_id, C UID & lake_id)C;
          Str editRiverPath(                  C UID &world_id, C UID &river_id)C;

   Str       worldVerPath   (C UID &world_id)C;
   WorldVer* worldVerFind   (C UID &world_id); 
   WorldVer* worldVerGet    (C UID &world_id); 
   WorldVer* worldVerRequire(C UID &world_id); 

   Str         miniMapVerPath   (C UID &mini_map_id)C;
   MiniMapVer* miniMapVerFind   (C UID &mini_map_id); 
   MiniMapVer* miniMapVerGet    (C UID &mini_map_id); 
   MiniMapVer* miniMapVerRequire(C UID &mini_map_id); 

   UID physToMesh(C Elm *phys);   
   UID animToSkel(C Elm *anim);   
   UID skelToMesh(C Elm *skel);   
   UID meshToSkel(C Elm *mesh);   
   UID meshToObj (C Elm *mesh);   
   UID  objToMesh(C Elm *obj );   
   UID animToMesh(C Elm *anim);   
   UID animToObj (C Elm *anim);   
   UID skelToObj (C Elm *skel);   
   UID physToObj (C Elm *phys);   
   UID  objToSkel(C Elm *obj );   
   UID animToSkel(C UID &anim_id);
   UID animToMesh(C UID &anim_id);
   UID animToObj (C UID &anim_id);
   UID skelToMesh(C UID &skel_id);
   UID skelToObj (C UID &skel_id);
   UID meshToSkel(C UID &mesh_id);
   UID meshToObj (C UID &mesh_id);
   UID physToObj (C UID &phys_id);
   UID  objToMesh(C UID & obj_id);
   UID  objToSkel(C UID & obj_id);

   Elm* objToMeshElm(C Elm * obj   );
   Elm* objToMeshElm(C UID & obj_id);
   Elm* skelToObjElm(C Elm *skel   );
   Elm* skelToObjElm(C UID &skel_id);
   Elm* meshToObjElm(C Elm *mesh   );
   Elm* meshToObjElm(C UID &mesh_id);
   Elm* animToObjElm(C Elm *anim   );
   Elm* animToObjElm(C UID &anim_id);
   Elm* physToObjElm(C Elm *phys   );
   Elm* physToObjElm(C UID &phys_id);

   Elm* mtrlToMeshElm(C UID &mtrl_id);
   UID mtrlToObj(C UID &mtrl_id);

   void getTextures(MemPtr<UID> texs, bool existing_elms=false)C; // 'existing_elms'=if get only from elements that exist (are not removed)
   void getUsedMaterials(MemPtr<UID> used, bool publish)C; // !! this needs to have hierarchy set !! 'publish'=if true then get materials used by publishable elements, if false then get materials used by existing elements
   Enum* getEnum(C UID &enum_id);
   void getMeshSkels(ElmMesh *mesh_data, UID *mesh_skel, UID *body_skel); // get skeletons for specified mesh
   void getMeshSkels(ElmMesh *mesh_data, Skeleton* *mesh_skel, Skeleton* *body_skel); // get skeletons for specified mesh
   bool getObjBox(C UID &elm_id, Box &box); // get box of ELM_OBJ (mesh.box|phys.box)
   bool getObjTerrain(C UID &elm_id); // get if ELM_OBJ has OBJ_ACCESS_TERRAIN
   OBJ_PATH getObjPath(C UID &elm_id); // get OBJ_PATH of ELM_OBJ
   RectI getWorldAreas(C UID &world_id, bool water=true);
   int worldAreasToRebuild(C UID *world_id=null);

   bool materialSimplify(Edit::EXE_TYPE type)C;

   bool isBasedOnObjs(C Elm &elm, C Memt<UID> &objs)C; // check if 'elm' is based on 'objs' (assumes that 'objs' is sorted)
   void getExtendedObjs(C Memt<UID> &objs, Memt<UID> &exts)C; // get list of all objects that are based on 'objs' (assumes that 'objs' is sorted)

   bool idToValid(C UID &id); // if target is valid (not removed)
   enum ID_MODE
   {
      ID_ALWAYS,
      ID_SKIP_ZERO,
      ID_SKIP_UNKNOWN,
   };
   Str idToText(C UID &id, bool *valid=null, ID_MODE id_mode=ID_ALWAYS); // 'valid'=if target is valid (not removed)

   bool invalidSrc(C Str &src, Str *invalid=null)C; // if specified and is not present
   bool invalidTexSrc(C Str &src, Str *invalid=null)C; // if specified and is not present
   bool invalidTex(C UID &tex_id                     )C; // is specified and is not present
   bool invalidRef(C UID &elm_id, bool optional=false)C; // is specified and is not present
   bool invalidRefs(Elm &elm); // check if this element has invalid references

   // operations
   Elm& getFolder(C Str &name, C UID &parent_id, bool &added, bool ignore_removed=true);
   UID getPathID(C Str &path, C UID &parent_id=UIDZero, bool ignore_removed=true); // create all project element folders from 'path' and return last element's id
   bool getWorldPaths(C UID &world_id, Str &world_edit_path, Str &world_game_path);
   void createWorldPaths(C UID &world_id);
   void createMiniMapPaths(C UID &mini_map_id);

   static int ChannelIndex(char c);
   static IMAGE_TYPE HighPrecType(IMAGE_TYPE type);
   static void MakeHighPrec(Image &image);
   static void ContrastLum(Image &image, flt contrast, flt avg_lum, C BoxI &box);
   static void AvgContrastLum(Image &image, flt contrast, dbl avg_lum, C BoxI &box);
   static void AddHue(Image &image, flt hue, C BoxI &box);
   static void MulRGBH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box);
   static void MulRGBHS(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box);
   static void MulSatH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, bool sat, bool photo, C BoxI &box);
   static flt HueDelta(flt a, flt b); // returns -0.5 .. 0.5
   static Vec2  LerpToMad(flt from, flt to);
   static Vec2 ILerpToMad(flt from, flt to);
   static flt   FloatSelf(flt x);         
   static flt   PowMax   (flt x, flt y);  
   static void IncludeAlpha(Image &image);

   static void TransformImage(Image &image, C MemPtr<TextParam> &params, bool clamp);
   bool loadImage(Image &image, C Edit::FileParams &fp, bool clamp=false, C Image *color=null, C Image *spec=null, C Image *bump=null)C;
   enum APPLY_MODE
   {
      APPLY_SET,
      APPLY_BLEND,
      APPLY_BLEND_PREMUL,
      APPLY_MUL,
      APPLY_DIV,
      APPLY_ADD,
      APPLY_SUB,
      APPLY_MAX,
   };
   bool loadImages(Image &image, C Str &src, bool clamp=false, C Color &background=TRANSPARENT, C Image *color=null, C Image *spec=null, C Image *bump=null)C;

   void savedGame(Elm &elm, C Str &name);
   void savedGame(Elm &elm             );

   void makeGameVer(Elm &elm, File *file=null);
   void removeOrphanedElms();
   virtual void eraseElm(C UID &elm_id);
   bool eraseElms(C MemPtr<UID> &elm_ids);
   void eraseTexFormats(C UID &tex_id);
   virtual bool eraseTex(C UID &_tex_id);
   bool eraseTexs();
   virtual void eraseWorldAreaObjs(C UID &world_id, C VecI2 &area_xy);
   bool eraseWorldObjs();

   void quickUpdateVersion(int ver); // this is called inside 'load', it occurs when opening projects and loading from EsenthelProject file, we can't modify files here !!
   void updateVersion(int ver, bool this_project=true, C MemPtr<UID> &elm_ids=null); // if 'elm_ids' is null, then all elements are processed, this is called inside 'open' or after copying elements to project

   void textData(bool on);

   // process dependencies
   virtual void meshSetAutoTanBin(Elm &elm, C MaterialPtr &material);
   void mtrlSetAutoTanBin(C UID &mtrl_id);
   virtual void animTransformChanged(Elm &elm_anim);
   void setAnimTransform(Elm &elm_anim);
   void setPhysParams(Elm &phys_elm);
   virtual void skelTransformChanged(C UID &skel_id);
   void skelChanged(Elm &skel_elm);
   void setSkelTransform(Elm &skel_elm);
   bool setPhysTransform(Elm &phys_elm);
   void meshTransformChanged(Elm &mesh_elm, bool body_changed=false);                     
   void meshTransformChanged(Elm &mesh_elm, bool body_changed,     Memt<UID>&processed); // can't do default param "=Memt<UID>()" on Mac
   void rebuildEmbedForObj (C    UID  &obj );                                         
   void rebuildEmbedForObjs(Memt<UID> &objs);
   void rebuildPathsForObj (C    UID  &obj , bool only_not_ovr);
   void rebuildPathsForObjs(Memt<UID> &objs, bool only_not_ovr); // 'only_not_ovr'=if rebuild paths only for objects not overriding the path mode
   void verifyPathsForObjClass(C UID &obj_class);
   void physChanged(Elm &phys);
   virtual void objChanged(Elm &obj);
   virtual void meshChanged(Elm &mesh); // objects rely on mesh information (physical body "obj -> mesh -> phys"), so rebuild objects
   void rebuildWorldAreas(Elm &elm, uint flag=AREA_REBUILD_PATH);

   virtual void hmDel(C UID &world_id, C VecI2 &area_xy, C TimeStamp *time=null);
   virtual Heightmap* hmGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp);
   virtual uint hmUpdate(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm);
   virtual void objGet(C UID &world_id, C VecI2 &area_xy, C Memc<UID> &obj_ids, Memc<ObjData> &objs); // assumes that 'obj_ids' is sorted
   Heightmap* hmObjGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp, Memc<ObjData> &objs, bool get_hm, bool get_objs);

   // sync
   bool syncElm(Elm &elm, Elm &src, File &src_data, File &src_extra, bool sync_long, bool &elm_newer_src, bool &src_newer_elm);
   virtual uint syncArea(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm, Memc<ObjData> &src_objs, Memc<UID> *local_objs_newer=null);
   class AreaSyncObjData
   {
      Project  &proj;
      UID       world_id;
      WorldVer &world_ver;

      AreaSyncObjData(Project &proj, C UID &world_id, WorldVer &world_ver);
   };
   class AreaSyncObj
   {
      bool          changed;
      VecI2         xy;
      AreaVer      *area_ver;
      WorldVer     *world_ver;
      Project      *project;
      Str           chunk_edit_path, chunk_game_path, game_path, edit_path;
      Chunks        chunks;
      Memc<ObjData> objs;

     ~AreaSyncObj();

      static bool Create(AreaSyncObj &area, C VecI2 &area_xy, ptr asod_ptr);

public:
   AreaSyncObj();
   };
   void rebuildEmbedObj(C UID &world_obj_instance_id, C VecI2 &area_xy, WorldVer &world_ver, bool rebuild_game_area_objs); // this must be called after 'changedObj'
   void rebuildWater(Lake *lake, River *river, C UID &water_id, WorldVer &world_ver);
   virtual bool syncObj(C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, Map<VecI2, Memc<ObjData> > *obj_modified=null, Memc<UID> *local_newer=null);
   virtual bool syncWaypoint(C UID &world_id, C UID &waypoint_id, Version &src_ver, EditWaypoint &src); // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that
   virtual bool syncLake(C UID &world_id, C UID &lake_id, Version &src_ver, Lake &src); // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that
   virtual bool syncRiver(C UID &world_id, C UID &river_id, Version &src_ver, River &src); // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that

   virtual bool syncMiniMapSettings(C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time);
   virtual bool syncMiniMapImage(C UID &mini_map_id, C VecI2 &image_xy, C TimeStamp &image_time, File &image_data);

   bool newerSettings(C Project &src)C;
   bool oldSettings(C TimeStamp &now=TimeStamp().getUTC())C;
   bool syncSettings(C Project &src);
   void initSettings(C Project &src); // this is called when finished copying elements to an empty project (for example after importing *.EsenthelProject file)

   // io
   virtual void flush(SAVE_MODE save_mode=SAVE_DEFAULT);
   bool loadOldSettings(File &f);
   bool loadOldSettings (C Str &name); 
   bool loadOldSettings2(C Str &name); 

   enum SAVE_DATA
   {
      SAVE_ID_NAME , // id, name
      SAVE_SETTINGS, // id, name, settings
      SAVE_ALL     , // id, name, settings, data
   };
   bool save(File &f, bool network=false, SAVE_DATA mode=SAVE_ALL)C;
   LOAD_RESULT load(File &f, int &ver, bool network=false, SAVE_DATA mode=SAVE_ALL);

   void save(MemPtr<TextNode> nodes)C;
   LOAD_RESULT load(C MemPtr<TextNode> &nodes, int &ver, Str &error); // !! this assumes that binary was already loaded and 'ver' already set !!

   LOAD_RESULT load(C Str &name, int &ver, SAVE_DATA mode=SAVE_ALL); 
   bool        save(C Str &name                                   )C;

   LOAD_RESULT load2(C Str &name, int &ver, SAVE_DATA mode=SAVE_ALL); 
   bool        save2(C Str &name                                   )C;

   bool saveTxt(C Str &name)C;
   LOAD_RESULT loadTxt(C Str &name, int &ver, Str &error); // !! this assumes that binary was already loaded and 'ver' already set !!

   LOAD_RESULT load3(Str path, int &ver, Str &error, SAVE_DATA mode=SAVE_ALL); // this loads "Data" and "Data.txt" (and old "Settings" file)
   bool isProject(C FileFind &ff); // this will only set 'id' and 'name', but not 'path'

   void setIDPath(C UID &id, C Str &path);
   virtual LOAD_RESULT open(C UID &id, C Str &name, C Str &path, Str &error, bool ignore_lock=false);
   virtual bool save(SAVE_MODE save_mode=SAVE_DEFAULT);
   virtual void close();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
