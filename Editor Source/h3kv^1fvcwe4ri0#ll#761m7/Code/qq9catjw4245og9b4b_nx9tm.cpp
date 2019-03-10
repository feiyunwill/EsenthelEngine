/******************************************************************************/
class GridPlaneLevel : ClosableWindow
{
   Tabs           mode;
   Memx<Property> props;
   TextBlack      ts;

   void create()
   {
      cchar8 *mode_t[]=
      {
         "Custom"       ,
         "Camera Target",
         "Selection"    ,
      };
      ts.reset().size=0.05;
      Gui+=super .create(Rect_C(0, 0, 0.85, 0.25), "Grid Plane Level").hide(); button[2].show();
      T  +=mode  .create(Rect_C(clientWidth()*0.5, -0.05, 0.8, 0.055), 0, mode_t, Elms(mode_t), true).valid(true).set(0); mode.tab(0).desc("Set Grid Plane Level from value specified below"); mode.tab(1).desc("Set Grid Plane Level from Camera Target (doesn't work in FPP mode)"); mode.tab(2).desc("Set Grid Plane Level from current selection (this can be selected objects, waypoints, etc.)");
      props.New().create("Level ", MEMBER(WorldView, grid_plane_level)).mouseEditSpeed(5).toGui(&WorldEdit);
      AddProperties(props, mode.tab(0), Vec2(0.06, -0.11), 0.05, 0.5, &ts);
   }
   virtual Window& show()override
   {
      if(hidden() && props.elms())props[0].textline.selectAll();
      return super.show();
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); hide();}
   }
   void set()
   {
      switch(mode())
      {
         case 0: REPAO(props).fromGui(&WorldEdit); break;
         case 1: if(!WorldEdit.v4.fpp())WorldEdit.grid_plane_level=ActiveCam.at.y; break;
         case 2: {Vec center; if(WorldEdit.selectionCenter(center))WorldEdit.grid_plane_level=center.y;} break;
      }
   }
}
/******************************************************************************/
class GoToArea : ClosableWindow
{
   Text     tx, tz;
   TextLine  x,  z;
   Button   go;

   static void Go(GoToArea &go_to)
   {
      HideEditAct(go_to);
      WorldEdit.v4.moveTo(Vec((TextFlt(go_to.x())+0.5)*WorldEdit.areaSize(), ActiveCam.at.y,
                              (TextFlt(go_to.z())+0.5)*WorldEdit.areaSize()));
   }

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.5, 0.4), "Go To Area").hide(); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      T  +=tx   .create(Vec2(0.1, -0.07), "X:"); T+=x.create(Rect_L(0.2, -0.07, 0.2, 0.06));
      T  +=tz   .create(Vec2(0.1, -0.14), "Z:"); T+=z.create(Rect_L(0.2, -0.14, 0.2, 0.06));
      T  +=go   .create(Rect_C(clientWidth()/2, -0.25, 0.3, 0.07), MLT("GO", PL,"Idź", RU,"Двигаться")).func(Go, T).focusable(false);
   }
   virtual GoToArea& show()override
   {
      if(hidden())
      {
         x.set(S+Floor(ActiveCam.at.x/WorldEdit.areaSize()));
         z.set(S+Floor(ActiveCam.at.z/WorldEdit.areaSize()));
         x.selectAll().activate();
      }
      super.show(); return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); Go(T);}
   }
}
/******************************************************************************/
class PathProps : Region
{
   Memx<Property> props;
   Text           tcell, vcell, tctrl_r, vctrl_r                    ; // t-text, v-value
   Button         dcell, icell, dctrl_r, ictrl_r, apply, copy, paste; // d-dec , i-inc
   TextWhite      ts, tsr;

   static ElmWorld* WS() {return WorldEdit.elm ? WorldEdit.elm.worldData() : null;} // world settings

   static void Changed(C Property &prop) {WorldEdit.changed_settings=true;}

   static void CellSize  (PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws.cellSize  (         TextFlt(text) ); pp.toGui();}}   static Str CellSize  (C PathProps &pp) {if(ElmWorld *ws=WS())return          ws.cell_size ; return S;}
   static void CtrlRadius(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws.ctrlRadius(         TextFlt(text) ); pp.toGui();}}   static Str CtrlRadius(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws.ctrl_r    ; return S;}
   static void CtrlHeight(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws.ctrlHeight(         TextFlt(text) );            }}   static Str CtrlHeight(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws.ctrl_h    ; return S;}
   static void MaxSlope  (PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws.maxSlope  (DegToRad(TextFlt(text)));            }}   static Str MaxSlope  (C PathProps &pp) {if(ElmWorld *ws=WS())return RadToDeg(ws.max_slope); return S;}
   static void MaxClimb  (PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws.maxClimb  (         TextFlt(text) );            }}   static Str MaxClimb  (C PathProps &pp) {if(ElmWorld *ws=WS())return          ws.max_climb ; return S;}

   static void IncCellSize(PathProps &pp) {if(ElmWorld *ws=WS()){flt cs=ws.cell_size; cs=1.0/cs; cs=         AlignFloor(cs+EPS, 0.5)+0.5 ; cs=1.0/cs; ws.cellSize(cs); WorldEdit.changed_settings=true; pp.toGui();}}
   static void DecCellSize(PathProps &pp) {if(ElmWorld *ws=WS()){flt cs=ws.cell_size; cs=1.0/cs; cs=Max(0.5, AlignCeil (cs-EPS, 0.5)-0.5); cs=1.0/cs; ws.cellSize(cs); WorldEdit.changed_settings=true; pp.toGui();}}

   static void IncCtrlRadius(PathProps &pp) {if(ElmWorld *ws=WS()){int rc=       Floor(ws.ctrl_r/ws.cell_size+EPS)+1 ; ws.ctrlRadius(rc*ws.cell_size); WorldEdit.changed_settings=true; pp.toGui();}}
   static void DecCtrlRadius(PathProps &pp) {if(ElmWorld *ws=WS()){int rc=Max(0, Ceil (ws.ctrl_r/ws.cell_size-EPS)-1); ws.ctrlRadius(rc*ws.cell_size); WorldEdit.changed_settings=true; pp.toGui();}}

   static void Apply(PathProps &pp) {WorldEdit.flushSettings();}

   static void Copy (PathProps &pp) {TextData data; FileText f; SaveProperties(pp.props, data.nodes); data.save(f.writeMem()); ClipSet(f.rewind().getAll());}
   static void Paste(PathProps &pp) {TextData data; FileText f; f.writeMem().putText(ClipGet()); data.load(f.rewind()); LoadProperties(pp.props, data.nodes);}

   PathProps& create()
   {
      super.create().skin(&TransparentSkin, false); kb_lit=false;
      props.New().create("Cell Size"        , MemberDesc(DATA_REAL).setFunc(CellSize  , CellSize  )).min(0.01).mouseEditSpeed(0.2).desc("Size of single path cell (in meters)\nDefault = 0.333");
      flt cs=props.elms()+0.5; props.New();
      props.New().create("Controller Radius", MemberDesc(DATA_REAL).setFunc(CtrlRadius, CtrlRadius)).min(0).mouseEditSpeed(0.2).desc("Controller Radius (in meters)\nwill be rounded according to Cell Size\nDefault = 0.33");
      flt cr=props.elms()+0.5; props.New();
      props.New().create("Controller Height", MemberDesc(DATA_REAL).setFunc(CtrlHeight, CtrlHeight)).min(0).mouseEditSpeed(0.2).desc("Controller Height (in meters)\nDefault = 2.0");
      props.New().create("Max Slope Angle"  , MemberDesc(DATA_INT ).setFunc(MaxSlope  , MaxSlope  )).range(0, 90)                    .desc("Maximum slope angle the controller can cross (in degrees)\nDefault = 45");
      props.New().create("Max Climb Height" , MemberDesc(DATA_REAL).setFunc(MaxClimb  , MaxClimb  )).min  (0    ).mouseEditSpeed(0.2).desc("Maximum height the controller can climb (in meters)\nDefault = 0.7");
      ts.reset(); ts.size=0.04; ts.align.x=1; tsr=ts; tsr.size*=0.8; tsr.align.x=-1;
      Rect r=AddProperties(props, T, Vec2(0.02,-0.02), 0.043, 0.15, &ts);
      flt y;
      y=r.lerpY(1-cs/props.elms()); T+=tcell  .create(Vec2(0.02, y), "Cells per Meter", &ts); T+=icell  .create(Rect_R(r.max.x, y, 0.045, 0.045), ">").func(IncCellSize  , T).focusable(false); T+=dcell  .create(Rect_R(icell  .rect().min.x, y, 0.045, 0.045), "<").func(DecCellSize  , T).focusable(false); T+=vcell  .create(Vec2(dcell  .rect().min.x-0.015, y), S, &tsr);
      y=r.lerpY(1-cr/props.elms()); T+=tctrl_r.create(Vec2(0.02, y), "Radius Cells"   , &ts); T+=ictrl_r.create(Rect_R(r.max.x, y, 0.045, 0.045), ">").func(IncCtrlRadius, T).focusable(false); T+=dctrl_r.create(Rect_R(ictrl_r.rect().min.x, y, 0.045, 0.045), "<").func(DecCtrlRadius, T).focusable(false); T+=vctrl_r.create(Vec2(dctrl_r.rect().min.x-0.015, y), S, &tsr);
      T+=copy .create(Rect_C(r.down()+Vec2(-0.15, -0.040), 0.10, 0.045), "Copy" ).func(Copy , T).func(Copy, T).desc("Copy settings into system clipboard memory");
      T+=apply.create(Rect_C(r.down()+Vec2(    0, -0.040), 0.16, 0.05 ), "Apply").func(Apply, T);
      T+=paste.create(Rect_C(r.down()+Vec2( 0.15, -0.040), 0.10, 0.045), "Paste").func(Copy , T).func(Paste, T).desc("Paste settings from system clipboard memory");
      rect(Rect_LU(0, 0, r.max.x+0.02, -r.min.y+0.02+0.065));

      REPAO(props).autoData(this).changed(Changed);
      return T;
   }
   void toGui()
   {
      if(ElmWorld *ws=WS())
      {
         REPAO(props).toGui();
         vcell  .set(S+TextReal(      1.0/ws.cell_size, 2));
         vctrl_r.set(S+Ceil    (ws.ctrl_r/ws.cell_size   ));
      }
   }
}
/******************************************************************************/
