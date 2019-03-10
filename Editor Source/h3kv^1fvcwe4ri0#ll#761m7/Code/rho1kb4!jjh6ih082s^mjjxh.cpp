/******************************************************************************/
class WorldData
{
   UID            elm_id=UIDZero;
   Elm           *elm=null;
   Grid<Area>     grid;
   Str            edit_path, game_path, edit_area_path, game_area_path, edit_waypoint_path, game_waypoint_path, lake_path, river_path;
   flt            area_size=64;
   int            hm_res=64;
   WorldVer      *ver=null;
   Memx<Obj>      objs;
   PathWorld      path_world;
   PathSettings   path_settings;
   EnvironmentPtr environment;

   WorldData() {grid.user(this);}

   flt areaSize()C {return area_size;}
   int   hmRes ()C {return hm_res;}

   Vec2   areaToWorld(C VecI2 &xz  )C {return                                      xz*areaSize() ;} // convert Area  Coordinates to World Position
   VecI2 worldToArea (C Vec2  &xz  )C {return                                Floor(xz/areaSize());} // convert World Position    to Area  Coordinates
   VecI2 worldToArea (C Vec   &pos )C {return                              worldToArea(pos .xz());} // convert World Position    to Area  Coordinates
   RectI worldToArea (C Rect  &rect)C {return RectI(worldToArea(rect.min), worldToArea(rect.max));} // convert World Position    to Area  Coordinates
   RectI worldToArea (C Box   &box )C {return RectI(worldToArea(box .min), worldToArea(box .max));} // convert World Position    to Area  Coordinates

   Area*  getArea      (C VecI2 &area_xy) {return              grid.get (area_xy).data();}
   Area* findArea      (C VecI2 &area_xy) {if(Cell<Area> *area=grid.find(area_xy))return area.data(); return null;}
   Area* findAreaLoaded(C VecI2 &area_xy) {if(     Area  *area=findArea (area_xy))if(area.loaded)return area; return null;}

   void load()
   {
      ver=null; 
      if(elm)ver=Proj.worldVerRequire(elm_id);
   }
   void flush()
   {
      if(elm)grid.func(Area.Flush);
      if(ver)ver.flush();
   }
   void unload()
   {
      flush();
      grid.del();
      path_world.del();
      path_settings.reset();
      environment=null;
      ver=null;
   }
}
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
   }
   static cchar8 *mode_t[]=
   {
      "Terrain",
      "Object",
      "Waypoint",
      "Water",
      "Path",
   };
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
   }
   enum WAYPOINT_OP
   {
      WPO_INS       ,
      WPO_DEL       ,
      WPO_MOVE      ,
      WPO_MOVE_ALL  ,
      WPO_MOVE_Y    ,
      WPO_MOVE_ALL_Y,
      WPO_ROT       ,
   }
   static cchar8 *waypoint_loop_t[]=
   {
      "Single"   , // 0
      "Loop"     , // 1
      "Ping-Pong", // 2
   }; ASSERT(Game.Waypoint.SINGLE==0 && Game.Waypoint.LOOP==1 && Game.Waypoint.PING_PONG==2);
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
   }

   Button               show_cur_pos, show_grid, undo, redo;
   Tabs                 mode, show_obj_grid;
   ComboBox             show_menu;
   Menu                *show_menu_objs=null;
   Memb<UID>            show_menu_objs_id; // use Memb because we store pointer to elements
   Button               set_grid_level, obj_hm_align, play, locate;
   Memx<Property>       view_props;
   TextWhite            ts;
   flt                  grid_plane_level=0;
   int                  visible_radius=4;
   bool                 show_world_center=true, show_objs=true, show_obj_access[OBJ_ACCESS_NUM], show_obj_box=false, show_obj_phys=false, show_world_path=false,
                        cur_collides_with_hm=true, cur_collides_with_obj=true,
                        obj_center_points=false, obj_matrix_points=false,
                        obj_random_angle=false,
                        hm_align=true, hm_align_nrm=false,
                        grid_align=false, grid_align_round=false;
   flt                  grid_align_size_xz=1, grid_align_size_y=1;
   RectI                visible_area(0, 0, 0, 0), visible_area_1(0, 0, 0, 0), valid_area(0, 0, 0, 0);
        Cursor          cur;
   Memc<Cursor>         cur_touch;
   bool                 changed_settings=false; // if changed world settings
   flt                  grass_range2=0, validate_refs_time=FLT_MAX;
   Memc<Edit.EditorInterface.Line> lines;
   WorldUndo            undos;   void undoVis() {SetUndo(undos, undo, redo);}

   // heightmap
   Button               hm_add_rem, hm_import;
   MaterialPtr          hm_mtrl;
   ImageSkin            hm_mtrl_img;
   Text                 hm_mtrl_text;
   flt                  hm_mtrl_highlight=-256;
   int                  hm_sel_size=0;
   bool                 hm_ao=true;
   Memx<Property>       hm_props;
   bool                 hm_use_shader=false;
   Shader              *hm_shader=null;
   int                  update_count=0;
   uint                 update_time=0;

   // object
   Tabs                 obj_op, obj_params;
   ComboBox             obj_menu;
   Button               obj_list, obj_paint;
   int                  obj_axis=-1;
   flt                  obj_edit_angle=0, obj_edit_speed=0;
   Vec                  obj_edit_vec=0;
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
     ~Waypoint() {WaypointList.removed(T);}
   }
   Tabs                             waypoint_op;
   ComboBox                         waypoint_menu;
   WaypointPos                      waypoint_pos;
   Region                           waypoint_props_region;
   Memx<Property>                   waypoint_props, waypoint_global_props;
   Button                           waypoint_list;
   flt                              waypoint_edit_speed=0;
   int                              waypoint_subdivide=1;
   ThreadSafeMap<UID, EditWaypoint> waypoints(Compare);
   Memc<EditWaypoint*>              visible_waypoints, changed_waypoints;
   EditWaypoint                    *sel_waypoint=null, *lit_waypoint=null;
   UID                              sel_waypoint_point=UIDZero, lit_waypoint_point=UIDZero;
   flt                              last_waypoint_flush_time=0;

   // water
   ComboBox                  water_menu;
   WaterMtrlPtr              water_mtrl;
   ImageSkin                 water_mtrl_img;
   Text                      water_mtrl_text;
   flt                       water_edit_speed=0, water_mtrl_highlight=-256;
   Tabs                      water_op, water_mode;
   Region                    water_props_region;
   Memx<Property>            water_props; 
   Property                 *water_mtrl_p=null, *water_tex_scale=null, *water_smooth=null;
   ThreadSafeMap<UID, Lake>   lakes(Compare);
   ThreadSafeMap<UID, River> rivers(Compare);
   Memc<Lake *>              visible_lakes , changed_lakes ;
   Memc<River*>              visible_rivers, changed_rivers;
   Lake                     *sel_lake =null, *lit_lake =null;
   River                    *sel_river=null, *lit_river=null;
   int                       sel_lake_poly=-1, sel_lake_point =-1, lit_lake_poly=-1, lit_lake_point =-1,
                                               sel_river_point=-1,                   lit_river_point=-1;
   flt                       last_water_flush_time=0;

   // path
   PathProps            path_props;

   WorldView()
   {
      REPAO(show_obj_access)=true; waypoints.replaceClass<Waypoint>();
      
      MemStats mem; mem.get(); long limit=(mem.total_phys ? mem.total_phys*25/100 : 256*1024*1024); // allocate 25% of RAM for Undo or 256 MB if RAM is unknown
   #if !X64
      MIN(limit, 400*1024*1024); // for 32-bit platforms limit to 400 MB
   #endif
      undos.maxMemUsage(limit);
   }

   static void Grid            (WorldView &world) {world.show_grid.push();}
   static void GridLevel       (WorldView &world) {world.grid_plane_level_win.visibleToggleActivate();}
   static void GotoArea        (WorldView &world) {world.goto_area.visibleToggleActivate();}
   static void HmAddRem        (WorldView &world) {SetHmOp(world.hm_add_rem() ? OP_HM_ADD_REM : OP_HM_NONE);}
   static void HmSelMtrl       (WorldView &world) {}
   static void HmImport        (WorldView &world) {ImportTerrain.visibleToggleActivate();}
   static void DecRadius       (WorldView &world) {world.visibleRadius(world.visibleRadius()-1);}
   static void IncRadius       (WorldView &world) {world.visibleRadius(world.visibleRadius()+1);}
   static void DecSel          (WorldView &world) {world.hmSelSize    (world.hmSelSize    ()-1);}
   static void IncSel          (WorldView &world) {world.hmSelSize    (world.hmSelSize    ()+1);}
   static void Mode1           (WorldView &world) {world.mode.toggle(0);}
   static void Mode2           (WorldView &world) {world.mode.toggle(1);}
   static void Mode3           (WorldView &world) {world.mode.toggle(2);}
   static void Mode4           (WorldView &world) {world.mode.toggle(3);}
   static void Mode5           (WorldView &world) {world.mode.toggle(4);}
   static void ModeS0          (WorldView &world) {world.modeS(0);}
   static void ModeS1          (WorldView &world) {world.modeS(1);}
   static void ModeS2          (WorldView &world) {world.modeS(2);}
   static void ModeS3          (WorldView &world) {world.modeS(3);}
   static void ModeS4          (WorldView &world) {world.modeS(4);}
   static void ModeS5          (WorldView &world) {world.modeS(5);}
   static void ModeS6          (WorldView &world) {world.modeS(6);}
   static void ModeS7          (WorldView &world) {world.modeS(7);}
   static void ModeS8          (WorldView &world) {world.modeS(8);}
   static void ModeS9          (WorldView &world) {world.modeS(9);}
   static void ModeS10         (WorldView &world) {world.modeS(10);}
   static void ModeS11         (WorldView &world) {world.modeS(11);}
   static void OpMove          (WorldView &world) {world.obj_op.toggle(OP_MOVE     );}
   static void OpRot           (WorldView &world) {world.obj_op.toggle(OP_ROT      );}
   static void OpScale         (WorldView &world) {world.obj_op.toggle(OP_SCALE    );}
   static void Op4             (WorldView &world) {world.obj_op.toggle(OP_SEPARATE );}
   static void Op5             (WorldView &world) {world.obj_op.toggle(OP_MOVE_Y   );}
   static void Op6             (WorldView &world) {world.obj_op.toggle(OP_ROT_Y    );}
   static void Op7             (WorldView &world) {world.obj_op.toggle(OP_ROT_GROUP);}
   static void AlignHm         (WorldView &world) {world.obj_hm_align.push();}
   static void AlignGrid       (WorldView &world) {world.show_obj_grid.toggle(0);}
   static void CurPos          (WorldView &world) {world.show_cur_pos.push();}
   static void ShowObjList     (WorldView &world) {world.obj_list .push();}
   static void ShowObjPaint    (WorldView &world) {world.obj_paint.push();}
   static void ShowObjPoint    (WorldView &world) {world.obj_matrix_points=world.show_menu.menu("Object Points");}
   static void ShowObjBox      (WorldView &world) {world.show_obj_box     =world.show_menu.menu("Object Boxes");}
   static void ShowObjPhys     (WorldView &world) {world.show_obj_phys    =world.show_menu.menu("Physical Bodies");}
   static void ShowWorldPath   (WorldView &world) {world.show_world_path  =world.show_menu.menu("World Path Mesh");}
   static void ShowWorldCenter (WorldView &world) {world.show_world_center=world.show_menu.menu("World Center");}
   static void ShowObjs        (WorldView &world) {world.show_objs                          =world.show_menu.menu("Objects/All"    ); world.setObjVisibility();}
   static void ShowObjTerrain  (WorldView &world) {world.show_obj_access[OBJ_ACCESS_TERRAIN]=world.show_menu.menu("Objects/Terrain"); world.setObjVisibility();}
   static void ShowObjGrass    (WorldView &world) {world.show_obj_access[OBJ_ACCESS_GRASS  ]=world.show_menu.menu("Objects/Grass"  ); world.setObjVisibility();}
   static void ShowObjCustom   (WorldView &world) {world.show_obj_access[OBJ_ACCESS_CUSTOM ]=world.show_menu.menu("Objects/Custom" ); world.setObjVisibility();}
   static void ShowObjClass    (UID       &id   ) {WorldEdit.hide_obj_classes.binaryToggle(id, Compare);                          WorldEdit.setObjVisibility();}
   static void ShowWaypointList(WorldView &world) {world.waypoint_list.push();}
   static void ModeChanged     (WorldView &world) {world.modeChanged();}
   static void ObjOpChanged    (WorldView &world) {world.obj_pos.apply();}
   static void WaterOpChanged  (WorldView &world) {world.setWaterVis();}
   static void Play            (WorldView &world) {if(world.elm)StartGame(*world.elm);}
   static void Undo            (WorldView &world) {world.undos.undo();}
   static void Redo            (WorldView &world) {world.undos.redo();}
   static void Locate          (WorldView &world) {Proj.elmLocate(world.elm_id);}

   void modeS(int i)
   {
      switch(mode())
      {
         case HEIGHTMAP: HeightBrush.mode.toggle(i); break;
         case OBJECT   :           obj_op.toggle(i); break;
         case WAYPOINT :      waypoint_op.toggle(i); break;
         case WATER    :         water_op.toggle(i); break;
      }
   }

   static void AreaDrawPrepare(Cell<Area> &cell, ptr) {cell().draw      ();}
   static void AreaDrawShadow (Cell<Area> &cell, ptr) {cell().drawShadow();}
   static void AreaDraw2D     (Cell<Area> &cell, ptr) {cell().draw2D    ();}

   static void Render() {WorldEdit.render();}
          void render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            grid.func(visibleArea(), AreaDrawPrepare);
            REPAO(obj_visible).draw();
         }break;

         case RM_SHADOW:
         {
            flt lod_s=D.lodFactorShadow(); D.lodFactorShadow(1);
            grid.func(visibleArea(), AreaDrawShadow);
            D.lodFactorShadow(lod_s);
            REPAO(obj_visible).drawShadow();
         }break;

         case RM_WATER:
         {
            REPAO(visible_lakes ).render();
            REPAO(visible_rivers).render();
         }break;

         case RM_BLEND   : REPAO(obj_blend     ).drawParticle(); obj_blend     .clear(); break;
         case RM_PALETTE : REPAO(obj_palette[0]).drawParticle(); obj_palette[0].clear(); break;
         case RM_PALETTE1: REPAO(obj_palette[1]).drawParticle(); obj_palette[1].clear(); break;
      }
   }

   static void Draw(Viewport &viewport) {if(Edit.Viewport4.View *view=WorldEdit.v4.getView(&viewport))WorldEdit.draw(*view);}
          void draw(Edit.Viewport4.View &view)
   {
      if(BigVisible())return;

      Environment *env=EnvEdit.cur(); if(!env)env=environment(); if(!env)env=&DefaultEnvironment; env.set();
      view.camera.set();

      // set shadowing distance
      Rect rect(areaToWorld(visible_area.min), areaToWorld(visible_area.max+1));
      flt  dist=Max(DistPointPlane(rect.lu().x0y(), ActiveCam.matrix.pos, ActiveCam.matrix.z),
                    DistPointPlane(rect.ru().x0y(), ActiveCam.matrix.pos, ActiveCam.matrix.z),
                    DistPointPlane(rect.ld().x0y(), ActiveCam.matrix.pos, ActiveCam.matrix.z),
                    DistPointPlane(rect.rd().x0y(), ActiveCam.matrix.pos, ActiveCam.matrix.z)); dist=AlignCeil(dist, 16); // align to make use of reduce shadow flicker
      flt frac=D.shadowFrac(); D.shadowFrac(frac*1.05*dist/D.viewRange());

      Renderer.wire=wire();
      SHADOW_MODE shd_mode=D.shadowMode(); if(Demo)D.shadowMode(SHADOW_NONE);
      grass_range2=Sqr(D.grassRange());
      obj_blend.clear(); REPAO(obj_palette).clear();
      Renderer(WorldView.Render);
      D.shadowMode(shd_mode);
      Renderer.wire=false;

      // restore settings
      D.shadowFrac(frac);

      // helpers using depth buffer
      Renderer.setDepthForDebugDrawing();
      if(show_world_center){SetMatrix(); Matrix(ActiveCam.dist*0.07*2*Tan(D.viewFovY()/2)).draw();}
      if(show_obj_box || show_obj_phys)REPAO(obj_visible).drawHelper(show_obj_box, show_obj_phys);
      if(show_world_path){D.depthLock(false); path_world.draw(); D.depthUnlock();}
      if(show_grid())drawGrid();
      if(cur.valid() && mode()==HEIGHTMAP && hm_add_rem())drawAreaSelection();
      if(ImportTerrain.visible())drawHeightmapImport();
      if(mode()==OBJECT  )Selection.drawSelLit();
      if(mode()==WAYPOINT)
      {
         SetMatrix();
         REPA(visible_waypoints){EditWaypoint *w=visible_waypoints[i]; if(!w.removed)w.draw((sel_waypoint==w) ? SelColor : (lit_waypoint==w) ? LitColor : DefColor, waypoint_subdivide);}
         if(sel_waypoint && !sel_waypoint.removed)if(EditWaypointPoint *p=sel_waypoint.find(sel_waypoint_point))if(!p.removed)Game.DrawWaypoint(p.pos, LitSelColor                                       , WaypointRadius);
         if(lit_waypoint && !lit_waypoint.removed)if(EditWaypointPoint *p=lit_waypoint.find(lit_waypoint_point))if(!p.removed)Game.DrawWaypoint(p.pos, (lit_waypoint==sel_waypoint) ? LitColor : SelColor, WaypointRadius);
         if(sel_waypoint && !sel_waypoint.removed)sel_waypoint.drawText(sel_waypoint_point, (lit_waypoint==sel_waypoint) ? lit_waypoint_point : UIDZero);
      }
      if(mode()==WATER)
      {
         SetMatrix(); D.depthLock(false);

         REPA(visible_lakes ){Lake  *w=visible_lakes [i]; if(!w.removed)w.draw((sel_lake ==w) ? SelColor : (lit_lake ==w) ? LitColor : DefColor, (sel_lake ==w) ? ColorAlpha(SelColor, 0.08) : TRANSPARENT);}
         REPA(visible_rivers){River *w=visible_rivers[i]; if(!w.removed)w.draw((sel_river==w) ? SelColor : (lit_river==w) ? LitColor : DefColor, (sel_river==w) ? ColorAlpha(SelColor, 0.08) : TRANSPARENT);}

         // lake selection
         if(sel_lake && InRange(sel_lake_poly, sel_lake.polys) && InRange(sel_lake_point, sel_lake.polys[sel_lake_poly]))
         {
            Vec &pos =sel_lake.polys[sel_lake_poly][sel_lake_point],
                &next=sel_lake.polys[sel_lake_poly][(sel_lake_point+1)%sel_lake.polys[sel_lake_poly].elms()];
            pos.draw(LitSelColor);
            D.line(LitSelColor, pos, next);
         }
         if(lit_lake && InRange(lit_lake_poly, lit_lake.polys) && InRange(lit_lake_point, lit_lake.polys[lit_lake_poly]))lit_lake.polys[lit_lake_poly][lit_lake_point].draw((lit_lake==sel_lake) ? LitColor : SelColor);

         // river selection
         if(lit_river && InRange(lit_river_point, lit_river.vtxs))lit_river.vtxs[lit_river_point].pos.draw((lit_river==sel_river) ? LitColor : SelColor);
         if(sel_river && InRange(sel_river_point, sel_river.vtxs))sel_river.vtxs[sel_river_point].pos.draw(LitSelColor);

         D.depthUnlock();
      }
      if(lines.elms())
      {
         SetMatrix();
         FREPA(lines)
         {
            Edit.EditorInterface.Line &line=lines[i];
            VI.line(line.color, line.edge.p[0], line.edge.p[1]);
         }
         VI.end();
      }

      // 2d stuff
      drawObjPoints();
      drawArea2D();
      Selection.draw(view);
   }
   void drawGrid()
   {
      Color color=ColorAlpha(0.15), color2=ColorAlpha(0.43);
      flt   unit=area_size, y=grid_plane_level;
      Rect  v=visibleArea(); v.max.x++; v.max.y++; v*=unit;
      SetMatrix();
      ALPHA_MODE alpha=D.alpha(ALPHA_BLEND_DEC); bool line_smooth=D.lineSmooth(false      ), depth_write=D.depthWrite(false      ); Plane(Vec(0, y, 0), Vec(0, 1, 0)).drawInfiniteBySize(color, unit); Quad(Vec(v.min.x, y, v.min.y), Vec(v.min.x, y, v.max.y), Vec(v.max.x, y, v.max.y), Vec(v.max.x, y, v.min.y)).draw(color2, false); // disable line smoothing because it can be very slow for lots of full-screen lines
                       D.alpha(alpha          );                  D.lineSmooth(line_smooth);             D.depthWrite(depth_write);
   }
   void drawAreaSelection()
   {
      SetMatrix(); ALPHA_MODE alpha=D.alpha(ALPHA_BLEND_DEC);
      flt x0=(cur.area().x-hm_sel_size)*areaSize(), x1=x0+areaSize()*(hm_sel_size*2+1),
          z0=(cur.area().y-hm_sel_size)*areaSize(), z1=z0+areaSize()*(hm_sel_size*2+1), y=grid_plane_level;
      Quad(Vec(x0,y,z0), Vec(x0,y,z1), Vec(x1,y,z1), Vec(x1,y,z0)).draw(Color(255, 255, 255, 128), false);
      D.alpha(alpha);
   }
   void drawHeightmapImport()
   {
      if(ImportTerrain.areaRectValid())
      {
         SetMatrix(); ALPHA_MODE alpha=D.alpha(ALPHA_BLEND_DEC);
         flt  y=grid_plane_level;
         Rect r=ImportTerrain.areaRect(); if(!r.validX())Swap(r.min.x, r.max.x); if(!r.validY())Swap(r.min.y, r.max.y); r.max.x++; r.max.y++; r*=area_size;
         Vec2 h=ImportTerrain.heightRange();
         Box  box(Vec(r.min.x, h.x, r.min.y), Vec(r.max.x, h.y, r.max.y)); bool dw=D.depthWrite(false); box.draw(ColorAlpha(RED, 0.3), true); D.depthWrite(dw); box.draw(ColorAlpha(RED, 0.5));
         Quad(Vec(r.min.x, y, r.min.y), Vec(r.min.x, y, r.max.y), Vec(r.max.x, y, r.max.y), Vec(r.max.x, y, r.min.y)).draw(RED);
         D.alpha(alpha);
      }
   }
   void drawObjPoints()
   {
      const flt r=0.005;
      if(cur.valid() && (mode()==OBJECT || mode()==WAYPOINT || mode()==WATER)){SetMatrix(); cur.pos().draw(Color(255, 255, 255, 112), r);}
      if(mode()==OBJECT)
      {
         SetMatrix();
         Rect tex(0, 1); Vec2 pos;
         Color lit_sel=ColorAlpha(LitSelColor, 0.75),
                   sel=ColorAlpha(   SelColor, 0.75),
               lit    =ColorAlpha(   LitColor, 0.75),
               normal =ColorAlpha(   DefColor, 0.75);
         REPA(obj_visible)
         {
            Obj &obj=*obj_visible[i]; if(obj.icon && PosToScreen(obj.pos(), pos))
            {
               VI.image(obj.icon());
               VI.color((obj.selected && obj.highlighted) ? lit_sel : obj.selected ? sel : obj.highlighted ? lit : normal);
               VI.imagePart(Rect_C(pos, 0.035), tex);
            }
         }
         VI.end();
      }
      if(obj_matrix_points)
      {
         SetMatrix();
         bool invalid=false;
                    REPA(obj_visible){Obj &obj=*obj_visible[i]; if(obj.invalid_refs){invalid=true; continue;} VI.dot(GetLitSelCol(obj.highlighted, obj.selected              ), obj.pos(), r);}
         if(invalid)REPA(obj_visible){Obj &obj=*obj_visible[i]; if(obj.invalid_refs)                          VI.dot(GetLitSelCol(obj.highlighted, obj.selected, InvalidColor), obj.pos(), r);} // draw invalid at the end, so they're at the top, to make sure they're always visible and not occluded
         VI.end();
      }
      if(ObjListClass.Elm *elm=ObjList.list.visToData(ObjList.list.lit))if(elm.obj){SetMatrix(); elm.obj.pos().draw(LitColor, r*2);}

      if(WaypointListClass.Elm *elm=WaypointList.list.visToData(WaypointList.list.lit))if(elm.waypoint){SetMatrix(); elm.waypoint.pos().draw(LitColor, r*2);}
   }
   void drawArea2D()
   {
      SetMatrix();
      grid.func(visibleArea(), AreaDraw2D);
   }

   void higlight()
   {
      flt a=1+(   hm_mtrl_highlight-Time.appTime())*0.8; if(a>0)Highlight(   hm_mtrl_img.screenRect(), a*0.75);
          a=1+(water_mtrl_highlight-Time.appTime())*0.8; if(a>0)Highlight(water_mtrl_img.screenRect(), a*0.75);
   }

   // get / set
   bool selected()C {return Mode()==MODE_WORLD;}

   Cursor& curAll  (int i)  {return InRange(i, cur_touch) ? cur_touch[i] : cur;} // get cursor from all cursors, including mouse and touches
   int     curTotal(     )C {return 1+cur_touch.elms()                        ;} // get number of   all cursors, including mouse and touches

   Cursor* findCur(Touch *touch) {if(!touch)return &cur; REPA(cur_touch)if(touch==cur_touch[i].touch())return &cur_touch[i]; return null;}

   class HeightmapY
   {
      Vec2  xz;
      VecI2 xzi;
      flt   dist;
      Area *find=null;
   }
   static void HeightNearestFunc(Cell<Area> &cell, HeightmapY &hy)
   {
      if(Heightmap2 *h=cell().hm)
      {
         if(cell.xy()==hy.xzi){hy.find=cell.data(); hy.dist=0;}else
         {
            Rect rect(cell.x(), cell.y(), cell.x()+1, cell.y()+1); rect*=WorldEdit.areaSize();
            flt  d=Dist(hy.xz, rect); if(!hy.find || d<hy.dist){hy.dist=d; hy.find=cell.data();}
         }
      }
   }

   flt hmHeight(C Vec  &pos, bool *found=null) {return hmHeight(pos.xz(), found);}
   flt hmHeight(C Vec2 &xz , bool *found=null)
   {
      if(Area *area=getArea(worldToArea(xz)))if(area.hm)
      {
         if(found)*found=true; return area.hmHeight(xz);
      }
      if(found)*found=false; return 0;
   }
   flt hmHeightNearest(C Vec2 &xz)
   {
      HeightmapY hy;
      hy.xz  =xz;
      hy.xzi =worldToArea(xz);
      hy.find=null;
      for(int i=0; i<=4; i+=2)
      {
         grid.func(RectI(hy.xzi).extend(i), HeightNearestFunc, hy); // first find the nearest heightmap
         if(hy.find)return hy.find.hmHeight(xz);
      }
      return grid_plane_level;
   }

   Vec hmNormalAvg(C Matrix &matrix, C Box &box) {if(Area *area=getArea(worldToArea(matrix.pos)))return area.hmNormalAvg(matrix, box); return Vec(0, 1, 0);}

   bool hmColor(C Vec  &pos, Vec &color) {return hmColor(pos.xz(), color);}
   bool hmColor(C Vec2 &xz , Vec &color) {if(Area *area=getArea(worldToArea(xz)))return area.hmColor(xz, color); color=1; return false;}

   MaterialPtr hmMtrl(C Vec  &pos, C MaterialPtr &cur_mtrl=null) {return hmMtrl(pos.xz(), cur_mtrl);}
   MaterialPtr hmMtrl(C Vec2 &xz , C MaterialPtr &cur_mtrl=null) {if(Area *area=getArea(worldToArea(xz)))return area.hmMtrl(xz, cur_mtrl); return null;}

   bool gridAlign     ()C {return grid_align        ;}
   bool gridAlignRound()C {return grid_align_round  ;}
   flt  gridAlignSize ()C {return grid_align_size_xz;}
   flt  gridAlignSizeY()C {return grid_align_size_y ;}

   RectI visibleArea  ()C {return visible_area  ;} // visible area rectangle
   int   visibleRadius()C {return visible_radius;} // visible area radius
   void  visibleRadius(int r) {Clamp(r, 0, MaxVisibleRadius); visible_radius=r; REPAO(view_props).toGui();}
   int   hmSelSize    ()C {return hm_sel_size   ;}
   void  hmSelSize    (int r) {Clamp(r, 0, MaxVisibleRadius); hm_sel_size=r; REPAO(hm_props).toGui();}

   bool selectionCenter(Vec &center)
   {
      switch(mode())
      {
         case OBJECT:
         {
            if(Selection.elms())
            {
               center=0; REPA(Selection)center+=Selection[i].center(); center/=Selection.elms(); return true;
            }
         }break;

         case WAYPOINT:
         {
            if(sel_waypoint)if(EditWaypointPoint *p=sel_waypoint.find(sel_waypoint_point))
            {
               center=p.pos; return true;
            }
         }break;

         case WATER:
         {
            if(sel_lake  && InRange(sel_lake_poly  , sel_lake .polys) && InRange(sel_lake_point, sel_lake.polys[sel_lake_poly])){center=sel_lake .polys[sel_lake_poly][sel_lake_point ]    ; return true;}
            if(sel_river && InRange(sel_river_point, sel_river.vtxs )                                                          ){center=sel_river.vtxs                [sel_river_point].pos; return true;}
         }break;
      }
      center=(cur.valid() ? cur.pos() : 0); return false;
   }
   bool selectionZoom(flt &dist)
   {
      flt size=-1;
      switch(mode())
      {
         case OBJECT  : if(Selection.elms()==1){Box box; if(Selection[0].getBox(box))size=box.size().avg();else size=1;} break;
         case WAYPOINT: if(sel_waypoint)size=1; break;
         case WATER   : if(sel_lake || sel_river)size=1; break;
      }
      if(size>0)
      {
         dist=4*size/Tan(v4.perspFov()/2);
         return true;
      }
      return false;
   }

 C UID* ID(C EditWaypoint *waypoint)C {return waypoints.dataToKey(waypoint);}
 C UID* ID(C Lake         *lake    )C {return lakes    .dataToKey(lake    );}
 C UID* ID(C River        *river   )C {return rivers   .dataToKey(river   );}

   // create
   void setMenu()
   {
      super.setMenu(selected());
      Proj.menu.setCommand(prefix+"Obj"     , selected() && mode()==OBJECT  );
      Proj.menu.setCommand(prefix+"Waypoint", selected() && mode()==WAYPOINT);
          show_menu.menu.enabled(selected()                    );
           obj_menu.menu.enabled(selected() && mode()==OBJECT  );
      waypoint_menu.menu.enabled(selected() && mode()==WAYPOINT);
         water_menu.menu.enabled(selected() && mode()==WATER   );
   }
   void setMenu(Node<MenuElm> &menu, C Str &prefix)
   {
      super.setMenu(menu, prefix);
      FREPA(menu.children)if(menu.children[i].name==prefix+"View")
      {
         Node<MenuElm> &v=menu.children[i];
         v.New().create("Dec Radius", DecRadius  , T).kbsc(KbSc(KB_LBR  , KBSC_REPEAT));
         v.New().create("Inc Radius", IncRadius  , T).kbsc(KbSc(KB_RBR  , KBSC_REPEAT));
         v.New().create("Dec Sel"   , DecSel     , T).kbsc(KbSc(KB_SUB  , KBSC_REPEAT));
         v.New().create("Inc Sel"   , IncSel     , T).kbsc(KbSc(KB_EQUAL, KBSC_REPEAT));
         v.New().create("Mode 1"    , Mode1      , T).kbsc(KbSc(KB_F1));
         v.New().create("Mode 2"    , Mode2      , T).kbsc(KbSc(KB_F2));
         v.New().create("Mode 3"    , Mode3      , T).kbsc(KbSc(KB_F3));
         v.New().create("Mode 4"    , Mode4      , T).kbsc(KbSc(KB_F4));
         v.New().create("Mode 5"    , Mode5      , T).kbsc(KbSc(KB_F5));
         v.New().create("ModeS 0"   , ModeS0     , T).kbsc(KbSc(KB_F1 , KBSC_SHIFT));
         v.New().create("ModeS 1"   , ModeS1     , T).kbsc(KbSc(KB_F2 , KBSC_SHIFT));
         v.New().create("ModeS 2"   , ModeS2     , T).kbsc(KbSc(KB_F3 , KBSC_SHIFT));
         v.New().create("ModeS 3"   , ModeS3     , T).kbsc(KbSc(KB_F4 , KBSC_SHIFT));
         v.New().create("ModeS 4"   , ModeS4     , T).kbsc(KbSc(KB_F5 , KBSC_SHIFT));
         v.New().create("ModeS 5"   , ModeS5     , T).kbsc(KbSc(KB_F6 , KBSC_SHIFT));
         v.New().create("ModeS 6"   , ModeS6     , T).kbsc(KbSc(KB_F7 , KBSC_SHIFT));
         v.New().create("ModeS 7"   , ModeS7     , T).kbsc(KbSc(KB_F8 , KBSC_SHIFT));
         v.New().create("ModeS 8"   , ModeS8     , T).kbsc(KbSc(KB_F9 , KBSC_SHIFT));
         v.New().create("ModeS 9"   , ModeS9     , T).kbsc(KbSc(KB_F10, KBSC_SHIFT));
         v.New().create("ModeS 10"  , ModeS10    , T).kbsc(KbSc(KB_F11, KBSC_SHIFT));
         v.New().create("ModeS 11"  , ModeS11    , T).kbsc(KbSc(KB_F12, KBSC_SHIFT));
         v.New().create("Grid"      , Grid       , T).kbsc(KbSc(KB_G  , KBSC_ALT));
         v.New().create("Cur Pos"   , CurPos     , T).kbsc(KbSc(KB_C  , KBSC_ALT));

         v.New().create("Undo" , Undo, T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_REPEAT));
         v.New().create("Redo" , Redo, T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc2(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT));
         v.New().create("Undo2", Undo, T).kbsc(KbSc(KB_BACK, KBSC_ALT           |KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space (besides on Windows Notepad they also work and are not listed)
         v.New().create("Redo2", Redo, T).kbsc(KbSc(KB_BACK, KBSC_ALT|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space (besides on Windows Notepad they also work and are not listed)

         break;
      }
      {
         Node<MenuElm> &o=menu+=prefix+"Obj";
         o.New().create("Op Move"   , OpMove      , T).kbsc(KbSc(KB_Z));
         o.New().create("Op Rotate" , OpRot       , T).kbsc(KbSc(KB_X));
         o.New().create("Op Scale"  , OpScale     , T).kbsc(KbSc(KB_C));
         o.New().create("Op 4"      , Op4         , T).kbsc(KbSc(KB_V));
         o.New().create("Op 5"      , Op5         , T).kbsc(KbSc(KB_B));
         o.New().create("Op 6"      , Op6         , T).kbsc(KbSc(KB_N));
         o.New().create("Op 7"      , Op7         , T).kbsc(KbSc(KB_M));
         o.New().create("Align Hm"  , AlignHm     , T).kbsc(KbSc(KB_H, KBSC_CTRL_CMD));
         o.New().create("Align Grid", AlignGrid   , T).kbsc(KbSc(KB_G, KBSC_CTRL_CMD));
         o.New().create("Obj List"  , ShowObjList , T).kbsc(KbSc(KB_L, KBSC_CTRL_CMD));
         o.New().create("Obj Paint" , ShowObjPaint, T).kbsc(KbSc(KB_P, KBSC_CTRL_CMD));
      }
      {
         Node<MenuElm> &o=menu+=prefix+"Waypoint";
         o.New().create("Waypoint List", ShowWaypointList, T).kbsc(KbSc(KB_L, KBSC_CTRL_CMD));
      }
   }
   void createTerrain()
   {
      flt h=0.05;
      mode.tab(HEIGHTMAP)+=hm_add_rem  .create(Rect_LU(mode.tab(HEIGHTMAP).rect().ld()+Vec2(-0.13, -0.01), 0.20, 0.055), "Del / Add").func(HmAddRem, T).focusable(false).desc("Remove existing heightmaps with LeftClick\nInsert new heightmaps with RightClick"); hm_add_rem.mode=BUTTON_TOGGLE;
      mode.tab(HEIGHTMAP)+=hm_mtrl_text.create(Vec2(hm_add_rem.rect().right()+Vec2(0.015, 0)), "Material", &ts);
      mode.tab(HEIGHTMAP)+=hm_mtrl_img .create(Rect_LU(hm_add_rem.rect().ru()+Vec2(0.140, 0), 0.055, 0.055)).desc("Material used when creating new heightmaps\nDrag and drop a material here"); hm_mtrl_img.alpha_mode=ALPHA_NONE;
      hm_props.New().create("Radius", MEMBER(WorldView, hm_sel_size)).range(0, MaxVisibleRadius).mouseEditSpeed(10).desc("Keyboard Shortcuts: - =");
      Rect hm_rect=AddProperties(hm_props, mode.tab(HEIGHTMAP), hm_mtrl_img.rect().ru()+Vec2(0.01, 0), h, 0.12, &ts); REPAO(hm_props).autoData(this);
      mode.tab(HEIGHTMAP)+=hm_import.create(Rect_LU(hm_rect.ru()+Vec2(h*1.5, 0), 0.165, 0.055), "Import").func(HmImport, T).focusable(false);

                                 Brush.create(mode.tab(HEIGHTMAP));
      mode.tab(HEIGHTMAP)+=HeightBrush.create();
      mode.tab(HEIGHTMAP)+= ColorBrush.create();
      mode.tab(HEIGHTMAP)+=  MtrlBrush.create();
   }
   void createObj()
   {
      flt h=0.05;
      mode.tab(OBJECT)+=obj_op.create(Rect_U(mode.tab(OBJECT).rect().down()+Vec2(0.13, -0.01), 0.055*8, 0.055), 0, (cchar**)null, 8).func(ObjOpChanged, T);
      obj_op.tab(OP_INS      ).setText ("+1"                        ).desc("Copy object\nSelect existing with LeftClick\nCopy with RightClick\n\nKeyboard Shortcut: Insert");
      obj_op.tab(OP_MOVE     ).setImage("Gui/Misc/move.img"         ).desc(S+    "Move object\nSelect with LeftClick\nMove with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Z");
      obj_op.tab(OP_ROT      ).setImage("Gui/Misc/rotate.img"       ).desc(S+  "Rotate object\nSelect with LeftClick\nRotate with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: X");
      obj_op.tab(OP_SCALE    ).setImage("Gui/Misc/scale.img"        ).desc(S+   "Scale object\nSelect with LeftClick\nScale with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: C");
      obj_op.tab(OP_SEPARATE ).setImage("Gui/Misc/separate.img"     ).desc(S+"Separate objects (this will move objects away from each other)\nSelect with LeftClick\nSeparate with RightClick\n\nKeyboard Shortcut: V (or "+Kb.ctrlCmdName()+"+Shift)");
      obj_op.tab(OP_MOVE_Y   ).setImage("Gui/Misc/move_vertical.img").desc(S+    "Move object vertically\nSelect with LeftClick\nMove with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: B (or "+Kb.ctrlCmdName()+")");
      obj_op.tab(OP_ROT_Y    ).setImage("Gui/Misc/rotate_y.img"     ).desc(S+  "Rotate objects by Y axis\nSelect with LeftClick\nRotate with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: N (or Alt)");
      obj_op.tab(OP_ROT_GROUP).setImage("Gui/Misc/rotate_group.img" ).desc(S+  "Rotate objects around their middle position\nSelect with LeftClick\nRotate with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: M (or "+Kb.ctrlCmdName()+"+Alt)");

      cchar8 *obj_grid_t[]={"Grid"};
      mode.tab(OBJECT)+=show_obj_grid.create(Rect_RU(obj_op.rect().lu()-Vec2(h, 0), 0.10, obj_op.rect().h()), 0, obj_grid_t, Elms(obj_grid_t)).desc(S+"Align objects to grid on movement\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+G");
      show_obj_grid.tab(0)+=obj_grid.create().move(show_obj_grid.rect().rd()-Vec2(0, 0.01)-obj_grid.rect().ru());
      mode.tab(OBJECT)+=obj_hm_align.create(Rect_RU(show_obj_grid.rect().lu(), 0.15, obj_op.rect().h()), "Ground").focusable(false).desc(S+"Align objects to heightmap on movement\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+H"); obj_hm_align.mode=BUTTON_TOGGLE; obj_hm_align.set(true);

      obj_op.tab(OP_MOVE )+=obj_pos  .create(obj_op.tab(OP_MOVE ).rect().down()-Vec2(0, 0.01));
      obj_op.tab(OP_ROT  )+=obj_rot  .create(obj_op.tab(OP_ROT  ).rect().down()-Vec2(0, 0.01));
      obj_op.tab(OP_SCALE)+=obj_scale.create(obj_op.tab(OP_SCALE).rect().down()-Vec2(0, 0.01));

      Node<MenuElm> n;
      n.New().create("Align to Terrain"       , ObjAlignTer ).kbsc(KbSc(KB_T, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Align to Terrain Normal", ObjAlignNrm ).kbsc(KbSc(KB_N, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Align to Grid"          , ObjAlignGrid).kbsc(KbSc(KB_G, KBSC_CTRL_CMD|KBSC_SHIFT));
      n++;
      n.New().create("Randomize Rotation", ObjRandomRot).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT));
      n.New().create("Reset Rotation"    , ObjResetRot ).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT));
      n++;
      n.New().create("Rotate X", ObjRotX).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT));
      n.New().create("Rotate Y", ObjRotY).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT));
      n.New().create("Rotate Z", ObjRotZ).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT));
      n++;
      n.New().create("Edit Object"           , EditObj          ).kbsc(KbSc(KB_E, KBSC_CTRL_CMD));
      n.New().create("Open Object Material"  , OpenObjMaterial  ).kbsc(KbSc(KB_M, KBSC_CTRL_CMD)).desc("Open selected Object's Material in the Material Editor.\nCalling this function many times will cycle through all materials of the object.");
      n.New().create("Copy Object InstanceID", CopyObjInstanceID).kbsc(KbSc(KB_D, KBSC_CTRL_CMD)).desc("Copy InstanceID of selected world object into clipboard.\n\nThis ID is not the ID of object or object class, it's the ID of specific instance of the object in the world.\nAs each instance of an object in the world has its own ID.");
      n++;
      n.New().create("Delete", DeleteObj).kbsc(KbSc(KB_DEL));
      n++;
      {
         Node<MenuElm> &extra=(n+="Extra");
         extra.New().create("Scale Original Object", ScaleOrgObj).kbsc(KbSc(KB_T, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT)).desc("This option will apply current World Object scale onto Project Element Object.\nWarning: this will affect scale of all instances of this object and this operation cannot be undone.");
      }
      mode.tab(OBJECT)+=obj_menu.create(Rect_LU(obj_op.rect().ru(), obj_op.rect().h()), n); obj_menu.flag|=COMBOBOX_CONST_TEXT;

      cchar8 *obj_params_t[]={"Params"};
      mode.tab(OBJECT)+=obj_params.create(Rect_LU(obj_menu  .rect().ru()+Vec2(h, 0), 0.19, obj_op.rect().h()), 0, obj_params_t, Elms(obj_params_t)).set(0).hide();
      mode.tab(OBJECT)+=obj_list  .create(Rect_LU(obj_params.rect().ru(), 0.14, obj_op.rect().h()), "List" ).focusable(false).desc(S+"Show object list\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+L"); obj_list.mode=BUTTON_TOGGLE;
      mode.tab(OBJECT)+=obj_paint .create(Rect_LU(obj_list  .rect().ru(), 0.15, obj_op.rect().h()), "Paint").focusable(false).desc(S+"Show object painter\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+P\n\nPaint objects with RightClick\nRemove objects with LeftClick"); obj_paint.mode=BUTTON_TOGGLE;
      param_edit.create(obj_params.tab(0), false, true);

      Gui+=ObjList .create();
      Gui+=ObjPaint.create();
   }
   void createWaypoint()
   {
      flt h=0.05;
      mode.tab(WAYPOINT)+=waypoint_op.create(Rect_U(mode.tab(WAYPOINT).rect().down()-Vec2(0, 0.01), 0.055*7, 0.055), 0, (cchar**)null, 7);
      waypoint_op.tab(WPO_INS       ).setText ("+1"                              ).desc("Insert new point in a waypoint\nSelect with LeftClick\nInsert with RightClick\n\nKeyboard Shortcut: Insert");
      waypoint_op.tab(WPO_DEL       ).setText ("-1"                              ).desc("Delete point in a waypoint\nDelete with RightClick\n\nKeyboard Shortcut: Delete");
      waypoint_op.tab(WPO_MOVE      ).setImage("Gui/Misc/move.img"               ).desc("Move single point of a waypoint\nSelect with LeftClick\nMove with RightClick");
      waypoint_op.tab(WPO_MOVE_ALL  ).setImage("Gui/Misc/separate.img"           ).desc("Move all points of a waypoint\nSelect with LeftClick\nMove with RightClick");
      waypoint_op.tab(WPO_MOVE_Y    ).setImage("Gui/Misc/move_vertical.img"      ).desc("Move vertically single point of a waypoint\nSelect with LeftClick\nMove with RightClick");
      waypoint_op.tab(WPO_MOVE_ALL_Y).setImage("Gui/Misc/move_group_vertical.img").desc("Move vertically all points of a waypoint\nSelect with LeftClick\nMove with RightClick");
      waypoint_op.tab(WPO_ROT       ).setImage("Gui/Misc/rotate.img"             ).desc("Rotate waypoint\nSelect with LeftClick\nRotate with RightClick");

      waypoint_op.tab(WPO_MOVE)+=waypoint_pos.create(waypoint_op.tab(WPO_MOVE).rect().down()-Vec2(0, 0.01));

      Node<MenuElm> n;
      n.New().create("Rotate Order Left" , WaypointRol).kbsc(KbSc(KB_SEMI, KBSC_REPEAT));
      n.New().create("Rotate Order Right", WaypointRor).kbsc(KbSc(KB_APO , KBSC_REPEAT));
      n.New().create("Reverse Order"     , WaypointRev).kbsc(KbSc(KB_R   , KBSC_CTRL_CMD));
      mode.tab(WAYPOINT)+=waypoint_menu.create(Rect_LU(waypoint_op.rect().ru(), waypoint_op.rect().h()), n); waypoint_menu.flag|=COMBOBOX_CONST_TEXT;
      mode.tab(WAYPOINT)+=waypoint_list.create(Rect_LU(waypoint_menu.rect().ru()+Vec2(h, 0), 0.14, waypoint_op.rect().h()), "List").focusable(false).desc(S+"Show waypoint list\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+L"); waypoint_list.mode=BUTTON_TOGGLE;
      mode.tab(WAYPOINT)+=waypoint_props_region.create().skin(&TransparentSkin, false); waypoint_props_region.kb_lit=false;
      Property &p=waypoint_props.New().create("ID"       , MemberDesc(DATA_STR).setFunc(WaypointID  , WaypointID  )); p.textline.disabled(true); p.button.create("Copy").func(WaypointIDCopy, T).desc("Copy Waypoint ID into clipboard");
                  waypoint_props.New().create("Name"     , MemberDesc(DATA_STR).setFunc(WaypointName, WaypointName));
                  waypoint_props.New().create("Loop Mode", MemberDesc(        ).setFunc(WaypointLoop, WaypointLoop)).setEnum(waypoint_loop_t, Elms(waypoint_loop_t));
      Rect r=AddProperties(waypoint_props, waypoint_props_region, Vec2(0.01, -0.01), h, 0.50, &ts); REPAO(waypoint_props).autoData(this).changed(WaypointChanged, WaypointPreChanged); p.button.resize(Vec2(h, 0));
      waypoint_props_region.rect(Rect_LU(waypoint_list.rect().ru()+Vec2(h, 0), r.size()+0.01*2+Vec2(h, 0)));

      waypoint_global_props.New().create("Line Smooth", MEMBER(WorldView, waypoint_subdivide)).range(1, 16).mouseEditSpeed(3);
      AddProperties(waypoint_global_props, mode.tab(WAYPOINT), waypoint_op.rect().lu()+Vec2(-0.34, 0), h, 0.12, &ts); REPAO(waypoint_global_props).autoData(this);

      Gui+=WaypointList.create();
   }
   void createWater()
   {
      flt h=0.05;
      mode.tab(WATER)+=water_op.create(Rect_U(mode.tab(WATER).rect().down()-Vec2(0, 0.01), 0.055*10*1.1, 0.055), 0, (cchar**)null, 10).func(WaterOpChanged, T); // make width slightly larger because "+S" occupies lot of space
      water_op.tab(WAO_NEW_POINT       ).setText ("+1"                                ).desc("Insert new point in water that is connected to selected point\nSelect with LeftClick\nInsert with RightClick\n\nKeyboard Shortcut: Insert");
      water_op.tab(WAO_NEW_WATER       ).setText ("+S"                                ).desc(S+"Insert new standalone point in water that won't be connected to selected point\nSelect with LeftClick\nInsert with RightClick\n\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Insert");
      water_op.tab(WAO_DEL_POINT       ).setText ("-1"                                ).desc("Delete point in water\nDelete with RightClick\n\nKeyboard Shortcut: Delete");
      water_op.tab(WAO_MOVE            ).setImage("Gui/Misc/move.img"                 ).desc("Move single point in water\nSelect with LeftClick\nMove with RightClick");
      water_op.tab(WAO_MOVE_ALL        ).setImage("Gui/Misc/separate.img"             ).desc("Move all points in water\nSelect with LeftClick\nMove with RightClick");
      water_op.tab(WAO_ROT             ).setImage("Gui/Misc/rotate.img"               ).desc("Rotate water\nSelect with LeftClick\nRotate with RightClick");
      water_op.tab(WAO_MOVE_ALL_Y      ).setImage("Gui/Misc/move_group_vertical.img"  ).desc("Move vertically all points in water\nSelect with LeftClick\nMove with RightClick");
      water_op.tab(WAO_MOVE_Y          ).setImage("Gui/Misc/move_vertical.img"        ).desc("Move vertically single point in water\nSelect with LeftClick\nMove with RightClick");
      water_op.tab(WAO_RIVER_POINT_SIZE).setImage("Gui/Misc/move_horizontal.img"      ).desc("Scale radius of single point in water\nSelect with LeftClick\nMove with RightClick");
      water_op.tab(WAO_RIVER_SIZE      ).setImage("Gui/Misc/move_group_horizontal.img").desc("Scale radius of all points in water\nSelect with LeftClick\nMove with RightClick");

      cchar8 *water_mode_t[]=
      {
         "Lake" ,
         "River",
      };
      mode.tab(WATER)+=water_mode.create(Rect_RU(water_op.rect().lu()-Vec2(0.05, 0), 0.28, 0.055), 0, water_mode_t, Elms(water_mode_t)).valid(true).set(0);
      mode.tab(WATER)+=water_mtrl_text.create(water_op.tab(WAO_LAKE_NUM-1).rect().right()+Vec2(0.05, 0), "Material", &ts);
      mode.tab(WATER)+=water_mtrl_img .create(Rect_L(water_mtrl_text.rect().right()+Vec2(0.12, 0), 0.055, 0.055)).desc("Material used when creating new water elements\nDrag and drop a material here"); water_mtrl_img.alpha_mode=ALPHA_NONE;
      mode.tab(WATER)+=water_props_region.create().skin(&TransparentSkin, false); water_props_region.kb_lit=false;
      water_mtrl_p   =&water_props.New().create("Material" , MemberDesc(DATA_STR ).setFunc(WaterMaterial, WaterMaterial)).desc("Drag and drop water material here"); water_mtrl_p.textline.disabled(true);
                       water_props.New().create("Depth"    , MemberDesc(DATA_REAL).setFunc(WaterDepth   , WaterDepth   )).min(0).mouseEditSpeed(1.5);
      water_tex_scale=&water_props.New().create("Tex Scale", MemberDesc(DATA_VEC2).setFunc(WaterTexScale, WaterTexScale));
      water_smooth   =&water_props.New().create("Smooth"   , MemberDesc(DATA_INT ).setFunc(WaterSmooth  , WaterSmooth  )).range(0, 4).mouseEditSpeed(2);
      Rect r=AddProperties(water_props, water_props_region, Vec2(0.01, -0.01), h, 0.30, &ts); REPAO(water_props).autoData(this).changed(WaterChanged, WaterPreChanged);
      water_props_region.rect(Rect_LU(water_op.rect().ru()+Vec2(0.01, 0), r.size()+0.01*2));
      setWaterVis();
   }
   void createPath()
   {
      mode.tab(PATH)+=path_props.create();
   }
   WorldView& create()
   {
      super.create(Draw, true, -0.4, 0, 64*3, 0.25, 3000);
      flt h=0.05;
      T+=show_cur_pos  .create(Rect_LU(ctrls       .rect().ld(), h)).focusable(false).desc("Display mouse cursor position information\nKeyboard Shortcut: Alt+C"); show_cur_pos.mode=BUTTON_TOGGLE; show_cur_pos.image="Gui/Misc/select_info.img";
      T+=show_grid     .create(Rect_LU(show_cur_pos.rect().ru(), h)).focusable(false).desc("Draw world grid\nKeyboard Shortcut: Alt+G"); show_grid.mode=BUTTON_TOGGLE; show_grid.set(true); show_grid.image="Gui/Misc/grid.img";
         wire          .pos   (        show_grid   .rect().ru());
      T+=set_grid_level.create(Rect_LU(show_grid   .rect().ld(), h, h*0.64), "SET").func(GridLevel, T).focusable(false).desc("Set world grid plane level\nThis affects cursor position (can be useful if operating on objects placed very high or very low).");
      grid_plane_level_win.create();
      goto_area           .create();
      Node<MenuElm> sn;
                                sn.New().create("Object Points"  , ShowObjPoint   , T).kbsc(KbSc(KB_V, KBSC_ALT           )).flag(MENU_TOGGLABLE).setOn(obj_matrix_points);
                                sn.New().create("Object Boxes"   , ShowObjBox     , T).kbsc(KbSc(KB_B, KBSC_ALT           )).flag(MENU_TOGGLABLE).setOn(show_obj_box     );
                                sn.New().create("Physical Bodies", ShowObjPhys    , T).kbsc(KbSc(KB_B, KBSC_ALT|KBSC_SHIFT)).flag(MENU_TOGGLABLE).setOn(show_obj_phys    );
                                sn.New().create("World Path Mesh", ShowWorldPath  , T).kbsc(KbSc(KB_P, KBSC_ALT|KBSC_SHIFT)).flag(MENU_TOGGLABLE).setOn(show_world_path  );
                                sn.New().create("World Center"   , ShowWorldCenter, T).kbsc(KbSc(KB_A, KBSC_ALT           )).flag(MENU_TOGGLABLE).setOn(show_world_center);
      Node<MenuElm> &objs=sn.New(); objs.create("Objects"                            ); objs.New(); // create child so "child column" will be created and "child menu"
      sn++;
      sn.New().create("Go to Area", GotoArea, T).kbsc(KbSc(KB_G, KBSC_ALT|KBSC_SHIFT));
      T+=show_menu.create(Rect_LU(wire.rect().ru(), h)).setData(sn); show_menu.flag|=COMBOBOX_CONST_TEXT;
      REP(show_menu.menu.elms())if(show_menu.menu.elm(i).name=="Objects"){show_menu_objs=show_menu.menu.elm(i).menu(); break;}

      T+=undo  .create(Rect_LU(ctrls.rect().ru()+Vec2(h, 0), h, h)     ).func(Undo  , T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU( undo.rect().ru()           , h, h)     ).func(Redo  , T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU( redo.rect().ru()           , h, h), "L").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      T+=mode.create(Rect_LU(locate.rect().ru()+Vec2(h, 0), 0.63, h), 0, mode_t, Elms(mode_t), true).func(ModeChanged, T, true); // set immediate func call so Brush visibility will be set instantly
      mode.tab(HEIGHTMAP).desc("Edit world heightmaps\nKeyboard Shortcut: F1");
      mode.tab(OBJECT   ).desc("Edit world objects\nKeyboard Shortcut: F2");
      mode.tab(WAYPOINT ).desc("Edit world waypoints\nKeyboard Shortcut: F3");
      mode.tab(WATER    ).desc("Edit world lakes and rivers\nKeyboard Shortcut: F4");
      mode.tab(PATH     ).desc("Edit world path settings\nKeyboard Shortcut: F5");

      ts.reset().size=0.038; ts.align.set(1, 0);
      view_props.New().create("Visible Radius", MEMBER(WorldView, visible_radius)).range(0, MaxVisibleRadius).mouseEditSpeed(4).desc("Keyboard Shortcuts: [ ]");
      Rect view_props_r=AddProperties(view_props, T, mode.rect().ru()+Vec2(h, 0), h, 0.12, &ts); REPAO(view_props).autoData(this);

      T+=play.create(Rect_LU(view_props_r.ru()+Vec2(h, 0), 0.12, h), "Play").func(Play, T).focusable(false);

      createTerrain ();
      createObj     ();
      createWaypoint();
      createWater   ();
      createPath    ();

      mode.moveToTop(); // because of sub-windows

      dummy_mtrl.reset().color.set(0.7, 0.7, 0.7, 0.5);
      dummy_mtrl.technique=MTECH_BLEND_LIGHT;
      dummy_mtrl.validate();
      dummy_mesh.create(1).parts[0].material(&dummy_mtrl).base.create(Ball(0.5), VTX_NRM, 3); dummy_mesh.setRender().setBox(); // keep base to speed up Cursor ray test
      return T;
   }

   // operations
   void modeChanged()
   {
      flushSettings(); flushWaypoints(); flushWater(); setMenu();
      if(mode()==HEIGHTMAP)
      {
         mode.tab(HEIGHTMAP)+=Brush;
         mode.tab(HEIGHTMAP)+=Brush.image_window;
         mode.tab(HEIGHTMAP)+=Brush.slope_window;
      }else
      if(mode()==OBJECT) // for Object Paint
      {
         mode.tab(OBJECT)+=Brush;
         mode.tab(OBJECT)+=Brush.image_window;
         mode.tab(OBJECT)+=Brush.slope_window;
      }
      Brush.setVisibility();
   }
   void selectedChanged()
   {
      setMenu();
      flushSettings ();
      flushWaypoints();
      flushWater    ();
   }
   void skinChanged()
   {
      MtrlBrush.setColors();
   }
   void camCenter(bool only_with_selection, bool zoom)
   {
      Vec center; flt dist;
      if(selectionCenter(center)            ){v4.moveTo(center   ); if(zoom && selectionZoom(dist))v4.dist(dist);}else
      if(cur.valid() && !only_with_selection) v4.moveTo(cur.pos());
   }
   virtual void camCenter(bool zoom)override {camCenter(false, zoom);}
   void flushSettings()
   {
      if(elm && changed_settings)if(ElmWorld *data=elm.worldData())
      {
         Proj.makeGameVer(*elm); // save settings to world game folder
         PathSettings temp; data.copyTo(temp); if(path_settings!=temp){path_settings=temp; Proj.rebuildWorldAreas(*elm);} // rebuild paths if needed
         Server.setElmShort(elm.id); // send new settings to the server
      }
      changed_settings=false;
   }
   void flushWaypoints();
   void flushWater    ();
   void flush()
   {
      super.flush   ();
      flushSettings ();
      flushWaypoints();
      flushWater    ();
   }
   void setVisibleWaypoints();
   void setVisibleWaters   ();

   void setChangedWaypoint(EditWaypoint *waypoint, bool refresh_gui=true) {if(C UID *waypoint_id=ID(waypoint)){changed_waypoints.include(waypoint); if(ver){ver.changedWaypoint(*waypoint_id);                                                                       if(refresh_gui && waypoint==sel_waypoint)waypoint_pos.toGui();}}} // if belongs to this world
   void setChangedLake    (Lake         *lake                           ) {if(C UID *    lake_id=ID(lake    )){changed_lakes    .include(lake    ); if(ver){ver.changedLake    (*    lake_id); lake .setMesh(Proj); Proj.rebuildWater(lake, null , * lake_id, *ver); if(               lake    ==sel_lake    )        waterToGui();}}} // if belongs to this world, call rebuild water after setting mesh so vertexes and area coverage is up to date
   void setChangedRiver   (River        *river                          ) {if(C UID *   river_id=ID(river   )){changed_rivers   .include(river   ); if(ver){ver.changedRiver   (*   river_id); river.setMesh(Proj); Proj.rebuildWater(null, river, *river_id, *ver); if(               river   ==sel_river   )        waterToGui();}}} // if belongs to this world, call rebuild water after setting mesh so vertexes and area coverage is up to date

   void selWaypoint(EditWaypoint *waypoint, C UID &point)
   {
      sel_waypoint=waypoint; sel_waypoint_point=point; waypoint_pos.toGui(); setVisibleWaypoints(); flushWaypoints(); WaypointList.setCur();
   }

   void waterToGui () {REPAO(water_props).toGui();}
   void setWaterVis()
   {
      bool vis=(/*(water_op()==WAO_NEW_POINT || water_op()==WAO_NEW_WATER) &&*/ !sel_lake && !sel_river); water_mode.visible(vis); water_mtrl_text.visible(vis); water_mtrl_img.visible(vis);
      water_props_region.visible(sel_lake || sel_river);
      if(water_tex_scale)water_tex_scale.visible(sel_river!=null);
      if(water_smooth   )water_smooth   .visible(sel_river!=null);
   }

   void reloadEnv()
   {
      UID env_id=UIDZero; if(elm)if(ElmWorld *data=elm.worldData())env_id=data.env_id;
      environment=Proj.gamePath(env_id);
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_WORLD)elm=null;
      if(elm)
         if(!(elm.data && elm.worldData() && elm.worldData().valid())){Gui.msgBox(S, "World settings not available.\nPerhaps they didn't finish downloading from the server."); elm=null;}
      if(T.elm!=elm)
      {
         HeightBrush.updateChanged();
         flushSettings ();
         flushWaypoints();
         flushWater    ();
         unload(); // unload after flushing

         waypoints.del(); visible_waypoints.del(); sel_waypoint=lit_waypoint=null; sel_waypoint_point.zero(); lit_waypoint_point.zero();
         lakes    .del(); visible_lakes    .del(); sel_lake    =lit_lake    =null; sel_lake_poly=sel_lake_point=lit_lake_poly=lit_lake_point-1;
         rivers   .del(); visible_rivers   .del(); sel_river   =lit_river   =null; sel_river_point=lit_river_point=-1;

         undos.del();

         T.elm=elm;
         T.elm_id=(elm ? elm.id : UIDZero); Proj.createWorldPaths(elm_id);
         if(elm)if(ElmWorld *data=elm.worldData())
         {
            edit_path=Proj.edit_path;
            Proj.getWorldPaths(elm_id, edit_path, game_path);
            edit_area_path=edit_path+"Area\\";
            game_area_path=game_path+"Area\\";
            edit_waypoint_path=edit_path+"Waypoint\\";
            game_waypoint_path=game_path+"Waypoint\\";
                     lake_path=edit_path+"Lake\\";
                    river_path=edit_path+"River\\";

            Game.WorldSettings ws; data.copyTo(ws, Proj); // get from 'WorldSettings' because values may get adjusted
            area_size=ws.areaSize();
              hm_res =ws.  hmRes ();
            path_world.create(area_size);
            data.copyTo(path_settings); // keep a copy of current path settings to detect if they got changed later
            path_props.toGui();
         }
         reloadEnv();
         WorldData.load();
         if(ver) // load all world waypoints and waters
         {
            REPA(ver.waypoints)waypoints(ver.waypoints.lockedKey(i)).load(edit_waypoint_path+EncodeFileName(ver.waypoints.lockedKey(i))); setVisibleWaypoints(); WaypointList.setChanged();
            REPA(ver.lakes    )lakes    (ver.lakes    .lockedKey(i)).load(         lake_path+EncodeFileName(ver.    lakes.lockedKey(i)));
            REPA(ver.rivers   )rivers   (ver.rivers   .lockedKey(i)).load(        river_path+EncodeFileName(ver.   rivers.lockedKey(i))); setVisibleWaters();
         }
         Proj.refresh(false, false);
         Mode.tabAvailable(MODE_WORLD, elm!=null);
         Server.getWorldVer(elm_id);
      }
   }
   void activate(Elm *elm)
   {
      set(elm); if(T.elm){Mode.set(MODE_WORLD); HideBig();}
   }
   void toggle(Elm *elm)
   {
      if(elm==T.elm && selected())elm=null;
      activate(elm);
   }

   class IDName
   {
      UID id;
      Str name;

      void set(C UID &id, C Str &name) {T.id=id; T.name=name;}

      static int CompareName(C IDName &a, C IDName &b) {return Compare(a.name, b.name);}
   }
   void enumChanged()
   {
      param_edit.enumChanged();
      if(show_menu_objs)
      {
         Node<MenuElm> elms;
         show_menu_objs_id.clear();
         elms.New().create("All", ShowObjs, T).flag(MENU_TOGGLABLE).setOn(show_objs);
         elms.New();
         elms.New().create("Terrain", ShowObjTerrain, T).flag(MENU_TOGGLABLE).setOn(show_obj_access[OBJ_ACCESS_TERRAIN]);
         elms.New().create("Grass"  , ShowObjGrass  , T).flag(MENU_TOGGLABLE).setOn(show_obj_access[OBJ_ACCESS_GRASS  ]);
         elms.New().create("Custom" , ShowObjCustom , T).flag(MENU_TOGGLABLE).setOn(show_obj_access[OBJ_ACCESS_CUSTOM ]);
         if(Proj.existing_obj_classes.elms())
         {
            elms.New();
            Memt<IDName> classes; FREPA(Proj.existing_obj_classes)if(Elm *obj_class=Proj.findElm(Proj.existing_obj_classes[i]))classes.New().set(obj_class.id, obj_class.name); classes.sort(IDName.CompareName);
            FREPA(classes)elms.New().create(classes[i].name, ShowObjClass, show_menu_objs_id.New()=classes[i].id).flag(MENU_TOGGLABLE).setOn(!hide_obj_classes.binaryHas(classes[i].id, Compare));
         }
         show_menu_objs.setData(elms);
      }
   }
   void meshVariationChanged() {param_edit.meshVariationChanged();}
   void elmChanged(C UID &elm_id)
   {
      if(elm_id==T.elm_id && elm)
      {
         path_props.toGui();
         reloadEnv();
      }
   }
   void erasing(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)set(null);
      if(Proj.   hm_mtrl_id==elm_id)setHmMtrl   (UIDZero);
      if(Proj.water_mtrl_id==elm_id)setWaterMtrl(UIDZero);
      MtrlBrush.erasing(elm_id);
       ObjPaint.erasing(elm_id);
   }
   void objTransChanged()
   {
      obj_pos  .reset();
      obj_scale.reset();
      obj_rot  .reset();
   }
   void objToGui()
   {
      ObjList.setChanged();
      objTransChanged();
      param_edit.toGui();
      param_edit.rename_window.hide();
      obj_pos   .visible(Selection.objs.elms()!=0);
      obj_scale .visible(Selection.objs.elms()!=0);
      obj_rot   .visible(Selection.objs.elms()!=0);
      obj_params.visible(Selection.objs.elms()!=0);
   }

   virtual void resize()override
   {
      super.resize();
      param_edit.move(Vec2(rect().w(), obj_params.rect().min.y-0.01)-param_edit.rect().ru());
      path_props.move(Vec2(rect().w(), mode      .rect().min.y-0.01)-path_props.rect().ru());
   }
   void setHmMtrl(C UID &id)
   {
      Elm *elm=Proj.findElm(id); if(elm && elm.type!=ELM_MTRL)elm=null;
      Proj.hm_mtrl_id=(elm ? elm.id : UIDZero);
      hm_mtrl=Proj.gamePath(Proj.hm_mtrl_id);
      hm_mtrl_img.set(MaterialImage(hm_mtrl));
      hm_mtrl_img.color=MaterialColor(hm_mtrl);
   }
   void setWaterMtrl(C UID &id)
   {
      Elm *elm=Proj.findElm(id); if(elm && elm.type!=ELM_WATER_MTRL)elm=null;
      Proj.water_mtrl_id=(elm ? elm.id : UIDZero);
      water_mtrl=Proj.gamePath(Proj.water_mtrl_id);
      water_mtrl_img.set(MaterialImage(water_mtrl));
      water_mtrl_img.color=MaterialColor(water_mtrl);
   }
   void setSelWaterMtrl(C UID &id)
   {
      if(Elm *elm=Proj.findElm(id, ELM_WATER_MTRL))
      {
         if(sel_lake ){undos.set(*sel_lake ); sel_lake .material=id; sel_lake .material_time.getUTC(); setChangedLake (sel_lake );}
         if(sel_river){undos.set(*sel_river); sel_river.material=id; sel_river.material_time.getUTC(); setChangedRiver(sel_river);}
      }
   }
   static bool SameNameExcludeIndex(Str a, Str b)
   {
      // ignore last character so "part_01a" is same as "part_01b"
      if(CharFlag(a.last())&(CHARF_ALPHA|CHARF_UNDER) && CharFlag(a[a.length()-2])&CHARF_DIG)a.removeLast();
      if(CharFlag(b.last())&(CHARF_ALPHA|CHARF_UNDER) && CharFlag(b[b.length()-2])&CHARF_DIG)b.removeLast();

      // remove last digits
      for(; CharFlag(a.last())&CHARF_DIG; )a.removeLast();
      for(; CharFlag(b.last())&CHARF_DIG; )b.removeLast();

      // remove outer spaces
      a.removeOuterWhiteChars();
      b.removeOuterWhiteChars();

      return a==b;
   }
   void drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos)
   {
      if(                focus_obj==&   hm_mtrl_img       )REPA(elms)setHmMtrl      (elms[i]);
      if(                focus_obj==&water_mtrl_img       )REPA(elms)setWaterMtrl   (elms[i]);
      if(water_mtrl_p && focus_obj==&water_mtrl_p.textline)REPA(elms)setSelWaterMtrl(elms[i]);
      if(Edit.Viewport4.View *view=v4.getView(focus_obj))
      {
         Selection.clearSelect();
         Cursor cur; cur.update(screen_pos, focus_obj, false, false, T);
         if(cur.onViewport())
         {
            Str last_name;
            UID last_parent=UIDZero;
            Vec start_pos=cur.pos(), pos=start_pos;
            int o=0, objs=0; REPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(elm.type==ELM_OBJ)objs++; int x=0, xs=Round(Sqrt(objs));
            FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))switch(elm.type)
            {
               case ELM_OBJ:
               {
                  if(Kb.ctrl() || Kb.shift())
                     if(o++)
                  {
                     x++;
                     if(Kb.ctrl () && x>=xs
                     || Kb.shift() && !(SameNameExcludeIndex(last_name, elm.name) && last_parent==elm.parent_id))
                        {pos.x=start_pos.x; pos.z++; x=0;}else pos.x++;
                  }
                  if(Obj *obj=NewObj(pos, *elm))Selection.select(*obj);
                  last_name  =elm.name;
                  last_parent=elm.parent_id;
               }break;

               case ELM_MTRL: if(Area *area=findAreaLoaded(cur.area()))if(area.hm)
               {
                  Heightmap2 &hm  =*area.hm;
                  int         res =hmRes();
                  MaterialPtr mtrl=Proj.gamePath(elm.id);
                  area.setChangedMtrl();

                  // edit pixels
                  REPD(y, res)
                  REPD(x, res)hm.setMaterial(x, y, mtrl, null, null, null, VecB4(255, 0, 0, 0));

                  // build
                  area.hmBuild();
               }break;
            }
         }
         if(elm)REPA(elms)if(Elm *env=Proj.findElm(elms[i], ELM_ENV))
         {
            elm.worldData().env(env.id); changed_settings=true;
            Proj.makeGameVer(*elm);
            reloadEnv();
            break;
         }
      }
      MtrlBrush .drag(elms, focus_obj, screen_pos);
      param_edit.drag(elms, focus_obj, screen_pos);
   }
   static void SetShader(Cell<Area> &cell, ptr user, int thread_index) {ThreadMayUseGPUData(); cell().setShader();}
          void setShader(                                            ) {grid.mtFunc(WorkerThreads, valid_area, SetShader);}

   // update
   void updateAligning()
   {
      if(mode()==OBJECT)
      {
        grid_align        =(show_obj_grid()>=0);
        grid_align_round  =true;
        grid_align_size_xz=(obj_grid.bxz() ? Max(0, TextFlt(obj_grid.xz())) : 0);
        grid_align_size_y =(obj_grid.by () ? Max(0, TextFlt(obj_grid.y ())) : 0);
      }else
      {
         grid_align        =false;
         grid_align_round  =false;
         grid_align_size_xz=areaSize();
         grid_align_size_y =0;
      }
   }
   static void UpdateVisibility(Cell<Area> &cell, WorldView &world) {world.updateVisibility(cell);}
          void updateVisibility(Cell<Area> &cell)
   {
      Area &area=cell();
      if(!Cuts(area.xy, valid_area))
      {
         if(!area.hasSelectedObj())area.unload();
      }else // remove
      {  // load
         if(update_count>=4 && Time.curTimeMs()-update_time>=90)return; AtomicInc(update_count); // skip if it takes too long
         area.load();
      }
   }
   void updateVisibility()
   {
      RectI old_valid=valid_area;

    C Camera &cam=v4.last().camera;
      visible_area=worldToArea(v4.fpp() ? cam.matrix.pos : cam.at);
      visible_area.extend(visible_radius);

      visible_area_1=visible_area;
      visible_area_1.extend(1);

      valid_area=visible_area;
      valid_area.extend(1);

      update_count=0;
      update_time =Time.curTimeMs();

      if(old_valid!=valid_area)
      {
         HeightBrush.updateChanged();
         setVisibleWaypoints();
         setVisibleWaters   ();
      }
      grid.func      (            UpdateVisibility, T);
      grid.funcCreate(valid_area, UpdateVisibility, T);
   }
   void updateCursors()
   {
      cur.update(Ms.pos(), Gui.ms(), Ms.b(0) || Ms.b(1), Ms.bd(0) || Ms.bd(1), T);
      REPA(cur_touch)if(!cur_touch[i].touch())cur_touch.remove(i); // delete cursors linked with missing touches
      REPA(Touches) // setup cursors according to touches
      {
         Touch  &touch=Touches[i];
         Cursor *cur  =null; REPA(cur_touch)if(cur_touch[i]._touch_id==touch.id()){cur=&cur_touch[i]; break;} // find existing cursor linked with the touch
         if(!cur){cur=&cur_touch.New(); cur._touch_id=touch.id();} // create new cursor and link it with the touch
         cur.update(touch.pos(), touch.guiObj(), touch.on(), touch.db(), T); // set cursor according to the touch
      }
   }
   void updateCamCenter()
   {
      REPA(MT)if(MT.guiObj(i)==&cam_center && MT.bd(i))
      {
         cam_center.mode=BUTTON_TOGGLE;
         cam_center.set(true);
      }
      if(cam_center.mode==BUTTON_TOGGLE)
      {
         if(!cam_center()){cam_center.set(true); cam_center.mode=BUTTON_DEFAULT;}
         camCenter(true, false);
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc); // update viewport4
      if(gpc.visible && visible())
      {
         grid_plane_level_win.set();
         Selection.update();
         updateAligning();
         updateVisibility();
         updateCursors(); // update cursor before anything else (like height brush for example, which is done in 'updateHm')
         updateHm();
         updateObj();
         updateWaypoint();
         updateWater();
         updateValidateRefs();

         if(Ms.bp(2))
         {
            if(param_edit.contains(Gui.ms()) || param_edit.param_window.contains(Gui.ms()))obj_params.toggle(0);
            if(path_props.contains(Gui.ms()))mode.set(-1);
         }
      }
   }
   static void HmCreate(Cell<Area> &cell, Memt<Area*> &created) {if(cell().hmCreate(true))created.add(cell.data());}
   static void HmDel   (Cell<Area> &cell, Memt<Area*> &created) {   cell().hmDel   (    );}
   static void HmBuild (     Area* &area, ptr user, int thread_index) {area.hmBuild();}
   void highlightHmMtrl   () {   hm_mtrl_highlight=Time.appTime();}
   void highlightWaterMtrl() {water_mtrl_highlight=Time.appTime();}
   void editHm(bool insert, Cursor &cur)
   {
      if(cur.onViewport() && cur.on())
      {
         if(insert && !hm_mtrl){highlightHmMtrl(); Gui.msgBox(S, "Please drag and drop a material into the terrain material slot in order to create new heightmaps");}else
         if(cur.valid())
         {
            Memt<Area*> created;
            for(PixelWalker walker(cur.edge().p[0]/areaSize(), cur.edge().p[1]/areaSize()); walker.active(); walker.step())
               grid.funcCreate(RectI(walker.pos()).extend(hm_sel_size)&visibleArea(), insert ? HmCreate : HmDel, created);
            WorkerThreads.process1(created, HmBuild);
         }
      }
   }
   static void DragHmMtrl(ptr, GuiObj *obj, C Vec2 &screen_pos)
   {
      UID mtrl_id=WorldEdit.hm_mtrl.id();
      if( mtrl_id.valid()){Memc<UID> elms; elms.add(mtrl_id); Drag(elms, obj, screen_pos);}
   }
   void updateHm()
   {
      HeightBrush.update();
        MtrlBrush.update();
       ColorBrush.update();

      if(mode()==HEIGHTMAP && hm_add_rem())
      {
                        editHm(Ms.b(1), cur);
       //REPA(cur_touch)editHm(, cur_touch[i]);
      }

      REPA(MT)if(MT.bp(i) && MT.guiObj(i)==&hm_mtrl_img && hm_mtrl){Gui.drag(DragHmMtrl, null, MT.touch(i)); if(MT.bd(i))MtrlEdit.set(hm_mtrl);}

      // set custom shader
      Shader *new_shader=null; bool use_shader=true;
      if(Brush.visible() && 
         (  mode()==OBJECT // ObjPaint
         || mode()==HEIGHTMAP && (OpHm==OP_HM_HEIGHT || OpHm==OP_HM_COLOR || OpHm==OP_HM_MTRL || OpHm==OP_HM_MTRL_SCALE || OpHm==OP_HM_MTRL_HOLE || OpHm==OP_HM_MTRL_MAX_1 || OpHm==OP_HM_MTRL_MAX_2)
         )
      )
      {
         if(Brush.shape()==BS_CIRCLE)new_shader=ShaderFiles("World Editor")->get("Circle");
         if(Brush.shape()==BS_SQUARE)new_shader=ShaderFiles("World Editor")->get("Square");

         Cursor &cur=(cur_touch.elms() ? cur_touch.last() : T.cur);
         if(cur.valid() && !(ObjPaint.available() && !ObjPaint.ready())) // don't draw brush when ObjPaint is in get-material mode
         {
            GetShaderImage("XZImage").set(Brush.image_hw);
            SPSet("XZImageUse"    , Brush.image() && Brush.image_hw.is());
            SPSet("XZPattern"     , Brush.pattern());
            SPSet("XZPatternScale", Brush.pattern_scale);
            SPSet("XZRange"       , Brush.size);
            SPSet("XZSoft"        , Brush.soft);
            SPSet("XZAngle"       , Brush.sangle()*PI2);
            SPSet("XZPos"         , cur.pos().xz());
            SPSet("XZCol"         , Vec(0.33, 0.66, 1.00)*0.45);
         }else
         {
            use_shader=false;
            SPSet("XZRange", EPS);
            SPSet("XZSoft" , EPS);
         }
      }else
      if((mode()==OBJECT && grid_align) || mode()==PATH)
      {
         new_shader=ShaderFiles("World Editor")->get("Grid");
         flt size=gridAlignSize(); if(size<=0)size=1;
         Vec col(0.33, 0.66, 1);
         switch(mode())
         {
            case OBJECT: col*=(grid_align ? 0.3 : 0.1); break;
            case PATH  : col*=0.2; if(elm)if(ElmWorld *ew=elm.worldData())size=ew.cell_size; break;
         }
         SPSet("XZRange", size);
         SPSet("XZSoft" , 0.08);
         SPSet("XZCol"  , col );
      }
      if(hm_shader!=new_shader){hm_shader=new_shader; setShader();}
      hm_use_shader=(hm_shader && use_shader);
   }
   void updateObj()
   {
      // update objects
      REPAO(obj_update).update();

      // get highlighted axis of selected object
      if(mode()!=OBJECT || (obj_op()!=OP_MOVE && obj_op()!=OP_SCALE && obj_op()!=OP_ROT) || Selection.elms()!=1)obj_axis=-1;else
      {
         Cursor *editing=((cur.onViewport() && Ms.b(1)) ? &cur : null); REPA(cur_touch)if(cur_touch[i].onViewport() && cur_touch[i].on()){editing=&cur_touch[i]; break;}
         bool first_push=true; REP(curTotal())if(curAll(i).notFirstEdit()){first_push=false; break;}
         if(editing ? first_push : true)
         {
            obj_axis=-1;
            Cursor &cur=(editing ? *editing : T.cur); if(cur.onViewport())
            {
               cur.view().setViewportCamera();
               obj_axis=MatrixAxis(cur.screenPos(), Selection[0].drawMatrix());
            }
         }
      }
      if(mode()==OBJECT)editObj();
   }

   static void ValidateRefs(Cell<Area> &cell, ptr) {cell().validateRefs();}
   void        validateRefs() {grid.func(ValidateRefs); validate_refs_time=FLT_MAX;}
   void delayedValidateRefs() {MIN(validate_refs_time, Time.appTime()+2);} // update after 2 seconds
   void  updateValidateRefs() {if(Time.appTime()>=validate_refs_time)validateRefs();}

   void updateWaypoint();
   void updateWater();
   void  lakeDel(Lake  *lake , int poly, int point);
   void riverDel(River *river, int vtx);
   void editObj();
   void removed(Obj &obj) {REP(curTotal())curAll(i).removed(obj); obj_visible.binaryExclude(&obj, Obj.ComparePtr); obj_update.binaryExclude(&obj, Obj.ComparePtr);}

   void hmDel(C VecI2 &area_xy, C TimeStamp *time=null)
   {
      if(elm)
      {
         Area &area=*getArea(area_xy);
         bool loaded=area.loaded;
         area.load();
         area.hmDel(time);
         if(!loaded)area.unload();
      }
   }
   void setObjVisibility()
   {
      REPAO(objs).setVisibility();
      ObjList.setChanged();
   }

   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         super.draw(gpc);
         Memt<Str> a;
         if(show_cur_pos())a.add(S+"Cursor: "+(cur.valid() ? S+cur.pos()+" (Area: "+worldToArea(cur.pos())+")" : S));
         if(a.elms())
         {
            flt x=rect().min.x+gpc.offset.x+0.01, y=rect().max.y+gpc.offset.y-0.25;
            TextStyleParams ts(false); ts.size=0.045; ts.align.set(1, -1);
            D.clip(); FREPA(a){D.text(ts, x, y, a[i]); y-=ts.size.y;}
         }
         if(Gui.dragging() && Gui.dragFinish()==(ptr)ProjectEx.DragElmsFinish && contains(Gui.msLit()))
         {
            TextStyleParams ts(false); ts.size=0.045; ts.align.set(1, -1);
            D.clip(); D.text(ts, Gui.dragPos(), "Hold Ctrl and/or Shift for alternative Object Placement");
         }
      }
   }
}
WorldView WorldEdit;
/******************************************************************************/
