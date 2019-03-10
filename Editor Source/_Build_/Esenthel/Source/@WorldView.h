/******************************************************************************/
/******************************************************************************/
class WorldView : Viewport4Region, WorldData
{
   enum MODE
   {
      HEIGHTMAP,
      OBJECT   ,
      WAYPOINT ,
      WATER    ,
      PATH     ,
   };
   static cchar8 *mode_t[]
;
   enum OBJ_OP
   {
      OP_INS      ,
      OP_MOVE     ,
      OP_ROT      ,
      OP_SCALE    ,
      OP_SEPARATE ,
      OP_MOVE_Y   ,
      OP_ROT_Y    ,
      OP_ROT_GROUP,
      OP_SEL      ,
   };
   enum WAYPOINT_OP
   {
      WPO_INS       ,
      WPO_DEL       ,
      WPO_MOVE      ,
      WPO_MOVE_ALL  ,
      WPO_MOVE_Y    ,
      WPO_MOVE_ALL_Y,
      WPO_ROT       ,
   };
   static cchar8 *waypoint_loop_t[]
; ASSERT(Game::Waypoint::SINGLE==0 && Game::Waypoint::LOOP==1 && Game::Waypoint::PING_PONG==2);
   enum WATER_OP
   {
      WAO_NEW_POINT       ,
      WAO_NEW_WATER       ,
      WAO_DEL_POINT       ,
      WAO_MOVE            ,
      WAO_MOVE_ALL        ,
      WAO_ROT             ,
      WAO_MOVE_ALL_Y      ,
      WAO_MOVE_Y          ,
      WAO_RIVER_POINT_SIZE, // river point radius
      WAO_RIVER_SIZE      , // river       radius

      WAO_LAKE_NUM =WAO_MOVE_Y      ,
      WAO_RIVER_NUM=WAO_RIVER_SIZE+1,
   };

   Button               show_cur_pos, show_grid, undo, redo;
   Tabs                 mode, show_obj_grid;
   ComboBox             show_menu;
   Menu                *show_menu_objs;
   Memb<UID>            show_menu_objs_id; // use Memb because we store pointer to elements
   Button               set_grid_level, obj_hm_align, play, locate;
   Memx<Property>       view_props;
   TextWhite            ts;
   flt                  grid_plane_level;
   int                  visible_radius;
   bool                 show_world_center, show_objs, show_obj_access[OBJ_ACCESS_NUM], show_obj_box, show_obj_phys, show_world_path,
                        cur_collides_with_hm, cur_collides_with_obj,
                        obj_center_points, obj_matrix_points,
                        obj_random_angle,
                        hm_align, hm_align_nrm,
                        grid_align, grid_align_round;
   flt                  grid_align_size_xz, grid_align_size_y;
   RectI                visible_area, visible_area_1, valid_area;
        Cursor          cur;
   Memc<Cursor>         cur_touch;
   bool                 changed_settings; // if changed world settings
   flt                  grass_range2, validate_refs_time;
   Memc<Edit::EditorInterface::Line> lines;
   WorldUndo            undos;   void undoVis();

   // heightmap
   Button               hm_add_rem, hm_import;
   MaterialPtr          hm_mtrl;
   ImageSkin            hm_mtrl_img;
   Text                 hm_mtrl_text;
   flt                  hm_mtrl_highlight;
   int                  hm_sel_size;
   bool                 hm_ao;
   Memx<Property>       hm_props;
   bool                 hm_use_shader;
   Shader              *hm_shader;
   int                  update_count;
   uint                 update_time;

   // object
   Tabs                 obj_op, obj_params;
   ComboBox             obj_menu;
   Button               obj_list, obj_paint;
   int                  obj_axis;
   flt                  obj_edit_angle, obj_edit_speed;
   Vec                  obj_edit_vec;
   ParamEditor          param_edit;
   ObjGrid              obj_grid;
   ObjPos               obj_pos;
   ObjScale             obj_scale;
   ObjRot               obj_rot;
   GridPlaneLevel       grid_plane_level_win;
   GoToArea             goto_area;
   Memc<Obj*>           obj_update, obj_visible, obj_blend, obj_palette[2];
   Mesh                 dummy_mesh;
   Material             dummy_mtrl;
   Memc<UID>            hide_obj_classes; // id's of ELM_OBJ_CLASS that want to be hidden in the world

   // waypoint
   class Waypoint : EditWaypoint
   {
     ~Waypoint();
   };
   Tabs                             waypoint_op;
   ComboBox                         waypoint_menu;
   WaypointPos                      waypoint_pos;
   Region                           waypoint_props_region;
   Memx<Property>                   waypoint_props, waypoint_global_props;
   Button                           waypoint_list;
   flt                              waypoint_edit_speed;
   int                              waypoint_subdivide;
   ThreadSafeMap<UID, EditWaypoint> waypoints;
   Memc<EditWaypoint*>              visible_waypoints, changed_waypoints;
   EditWaypoint                    *sel_waypoint, *lit_waypoint;
   UID                              sel_waypoint_point, lit_waypoint_point;
   flt                              last_waypoint_flush_time;

   // water
   ComboBox                  water_menu;
   WaterMtrlPtr              water_mtrl;
   ImageSkin                 water_mtrl_img;
   Text                      water_mtrl_text;
   flt                       water_edit_speed, water_mtrl_highlight;
   Tabs                      water_op, water_mode;
   Region                    water_props_region;
   Memx<Property>            water_props; 
   Property                 *water_mtrl_p, *water_tex_scale, *water_smooth;
   ThreadSafeMap<UID, Lake>   lakes;
   ThreadSafeMap<UID, River> rivers;
   Memc<Lake *>              visible_lakes , changed_lakes ;
   Memc<River*>              visible_rivers, changed_rivers;
   Lake                     *sel_lake , *lit_lake ;
   River                    *sel_river, *lit_river;
   int                       sel_lake_poly, sel_lake_point , lit_lake_poly, lit_lake_point ,
                                               sel_river_point,                   lit_river_point;
   flt                       last_water_flush_time;

   // path
   PathProps            path_props;

   WorldView();

   static void Grid            (WorldView &world);
   static void GridLevel       (WorldView &world);
   static void GotoArea        (WorldView &world);
   static void HmAddRem        (WorldView &world);
   static void HmSelMtrl       (WorldView &world);
   static void HmImport        (WorldView &world);
   static void DecRadius       (WorldView &world);
   static void IncRadius       (WorldView &world);
   static void DecSel          (WorldView &world);
   static void IncSel          (WorldView &world);
   static void Mode1           (WorldView &world);
   static void Mode2           (WorldView &world);
   static void Mode3           (WorldView &world);
   static void Mode4           (WorldView &world);
   static void Mode5           (WorldView &world);
   static void ModeS0          (WorldView &world);
   static void ModeS1          (WorldView &world);
   static void ModeS2          (WorldView &world);
   static void ModeS3          (WorldView &world);
   static void ModeS4          (WorldView &world);
   static void ModeS5          (WorldView &world);
   static void ModeS6          (WorldView &world);
   static void ModeS7          (WorldView &world);
   static void ModeS8          (WorldView &world);
   static void ModeS9          (WorldView &world);
   static void ModeS10         (WorldView &world);
   static void ModeS11         (WorldView &world);
   static void OpMove          (WorldView &world);
   static void OpRot           (WorldView &world);
   static void OpScale         (WorldView &world);
   static void Op4             (WorldView &world);
   static void Op5             (WorldView &world);
   static void Op6             (WorldView &world);
   static void Op7             (WorldView &world);
   static void AlignHm         (WorldView &world);
   static void AlignGrid       (WorldView &world);
   static void CurPos          (WorldView &world);
   static void ShowObjList     (WorldView &world);
   static void ShowObjPaint    (WorldView &world);
   static void ShowObjPoint    (WorldView &world);
   static void ShowObjBox      (WorldView &world);
   static void ShowObjPhys     (WorldView &world);
   static void ShowWorldPath   (WorldView &world);
   static void ShowWorldCenter (WorldView &world);
   static void ShowObjs        (WorldView &world);
   static void ShowObjTerrain  (WorldView &world);
   static void ShowObjGrass    (WorldView &world);
   static void ShowObjCustom   (WorldView &world);
   static void ShowObjClass    (UID       &id   );
   static void ShowWaypointList(WorldView &world);
   static void ModeChanged     (WorldView &world);
   static void ObjOpChanged    (WorldView &world);
   static void WaterOpChanged  (WorldView &world);
   static void Play            (WorldView &world);
   static void Undo            (WorldView &world);
   static void Redo            (WorldView &world);
   static void Locate          (WorldView &world);

   void modeS(int i);

   static void AreaDrawPrepare(Cell<Area> &cell, ptr);
   static void AreaDrawShadow (Cell<Area> &cell, ptr);
   static void AreaDraw2D     (Cell<Area> &cell, ptr);

   static void Render();
          void render();

   static void Draw(Viewport &viewport);        
          void draw(Edit::Viewport4::View&view);
   void drawGrid();
   void drawAreaSelection();
   void drawHeightmapImport();
   void drawObjPoints();
   void drawArea2D();

   void higlight();

   // get / set
   bool selected()C;

   Cursor& curAll  (int i);  // get cursor from all cursors, including mouse and touches
   int     curTotal(     )C; // get number of   all cursors, including mouse and touches

   Cursor* findCur(Touch *touch);

   class HeightmapY
   {
      Vec2  xz;
      VecI2 xzi;
      flt   dist;
      Area *find;

public:
   HeightmapY();
   };
   static void HeightNearestFunc(Cell<Area> &cell, HeightmapY &hy);

   flt hmHeight(C Vec  &pos, bool *found=null);
   flt hmHeight(C Vec2 &xz , bool *found=null);
   flt hmHeightNearest(C Vec2 &xz);

   Vec hmNormalAvg(C Matrix &matrix, C Box &box);

   bool hmColor(C Vec  &pos, Vec &color);
   bool hmColor(C Vec2 &xz , Vec &color);

   MaterialPtr hmMtrl(C Vec  &pos, C MaterialPtr &cur_mtrl=null);
   MaterialPtr hmMtrl(C Vec2 &xz , C MaterialPtr &cur_mtrl=null);

   bool gridAlign     ()C;
   bool gridAlignRound()C;
   flt  gridAlignSize ()C;
   flt  gridAlignSizeY()C;

   RectI visibleArea  ()C;    // visible area rectangle
   int   visibleRadius()C;    // visible area radius
   void  visibleRadius(int r);
   int   hmSelSize    ()C;    
   void  hmSelSize    (int r);

   bool selectionCenter(Vec &center);
   bool selectionZoom(flt &dist);

 C UID* ID(C EditWaypoint *waypoint)C;
 C UID* ID(C Lake         *lake    )C;
 C UID* ID(C River        *river   )C;

   // create
   void setMenu();
   void setMenu(Node<MenuElm> &menu, C Str &prefix);
   void createTerrain();
   void createObj();
   void createWaypoint();
   void createWater();
   void createPath();
   WorldView& create();

   // operations
   void modeChanged();
   void selectedChanged();
   void skinChanged();
   void camCenter(bool only_with_selection, bool zoom);
   virtual void camCenter(bool zoom)override;        
   void flushSettings();
   void flushWaypoints();
   void flushWater    ();
   void flush();
   void setVisibleWaypoints();
   void setVisibleWaters   ();

   void setChangedWaypoint(EditWaypoint *waypoint, bool refresh_gui=true); // if belongs to this world
   void setChangedLake    (Lake         *lake                           ); // if belongs to this world, call rebuild water after setting mesh so vertexes and area coverage is up to date
   void setChangedRiver   (River        *river                          ); // if belongs to this world, call rebuild water after setting mesh so vertexes and area coverage is up to date

   void selWaypoint(EditWaypoint *waypoint, C UID &point);

   void waterToGui ();
   void setWaterVis();

   void reloadEnv();
   void set(Elm *elm);
   void activate(Elm *elm);
   void toggle(Elm *elm);

   class IDName
   {
      UID id;
      Str name;

      void set(C UID &id, C Str &name);

      static int CompareName(C IDName &a, C IDName &b);
   };
   void enumChanged();
   void meshVariationChanged();   
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id);
   void objTransChanged();
   void objToGui();

   virtual void resize()override;
   void setHmMtrl(C UID &id);
   void setWaterMtrl(C UID &id);
   void setSelWaterMtrl(C UID &id);
   static bool SameNameExcludeIndex(Str a, Str b);
   void drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos);
   static void SetShader(Cell<Area> &cell, ptr user, int thread_index);
          void setShader(                                            );

   // update
   void updateAligning();
   static void UpdateVisibility(Cell<Area> &cell, WorldView &world);
          void updateVisibility(Cell<Area> &cell);
   void updateVisibility();
   void updateCursors();
   void updateCamCenter();
   virtual void update(C GuiPC &gpc)override;
   static void HmCreate(Cell<Area> &cell, Memt<Area*> &created);      
   static void HmDel   (Cell<Area> &cell, Memt<Area*> &created);      
   static void HmBuild (     Area* &area, ptr user, int thread_index);
   void highlightHmMtrl   ();                                      
   void highlightWaterMtrl();                                      
   void editHm(bool insert, Cursor &cur);
   static void DragHmMtrl(ptr, GuiObj *obj, C Vec2 &screen_pos);
   void updateHm();
   void updateObj();

   static void ValidateRefs(Cell<Area>&cell, ptr);
   void        validateRefs();                  
   void delayedValidateRefs();                  // update after 2 seconds
   void  updateValidateRefs();                  

   void updateWaypoint();
   void updateWater();
   void  lakeDel(Lake  *lake , int poly, int point);
   void riverDel(River *river, int vtx);
   void editObj();
   void removed(Obj &obj);

   void hmDel(C VecI2 &area_xy, C TimeStamp *time=null);
   void setObjVisibility();

   virtual void draw(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern WorldView WorldEdit;
/******************************************************************************/
