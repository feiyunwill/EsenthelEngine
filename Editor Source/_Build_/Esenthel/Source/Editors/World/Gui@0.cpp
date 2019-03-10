/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   void GridPlaneLevel::create()
   {
      cchar8 *mode_t[]=
      {
         "Custom"       ,
         "Camera Target",
         "Selection"    ,
      };
      ts.reset().size=0.05f;
      Gui+=::EE::Window::create(Rect_C(0, 0, 0.85f, 0.25f), "Grid Plane Level").hide(); button[2].show();
      T  +=mode  .create(Rect_C(clientWidth()*0.5f, -0.05f, 0.8f, 0.055f), 0, mode_t, Elms(mode_t), true).valid(true).set(0); mode.tab(0).desc("Set Grid Plane Level from value specified below"); mode.tab(1).desc("Set Grid Plane Level from Camera Target (doesn't work in FPP mode)"); mode.tab(2).desc("Set Grid Plane Level from current selection (this can be selected objects, waypoints, etc.)");
      props.New().create("Level ", MEMBER(WorldView, grid_plane_level)).mouseEditSpeed(5).toGui(&WorldEdit);
      AddProperties(props, mode.tab(0), Vec2(0.06f, -0.11f), 0.05f, 0.5f, &ts);
   }
   Window& GridPlaneLevel::show()
{
      if(hidden() && props.elms())props[0].textline.selectAll();
      return ::EE::Window::show();
   }
   void GridPlaneLevel::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); hide();}
   }
   void GridPlaneLevel::set()
   {
      switch(mode())
      {
         case 0: REPAO(props).fromGui(&WorldEdit); break;
         case 1: if(!WorldEdit.v4.fpp())WorldEdit.grid_plane_level=ActiveCam.at.y; break;
         case 2: {Vec center; if(WorldEdit.selectionCenter(center))WorldEdit.grid_plane_level=center.y;} break;
      }
   }
   void GoToArea::Go(GoToArea &go_to)
   {
      HideEditAct(go_to);
      WorldEdit.v4.moveTo(Vec((TextFlt(go_to.x())+0.5f)*WorldEdit.areaSize(), ActiveCam.at.y,
                              (TextFlt(go_to.z())+0.5f)*WorldEdit.areaSize()));
   }
   void GoToArea::create()
   {
      Gui+=::EE::Window::create(Rect_C(0, 0, 0.5f, 0.4f), "Go To Area").hide(); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      T  +=tx   .create(Vec2(0.1f, -0.07f), "X:"); T+=x.create(Rect_L(0.2f, -0.07f, 0.2f, 0.06f));
      T  +=tz   .create(Vec2(0.1f, -0.14f), "Z:"); T+=z.create(Rect_L(0.2f, -0.14f, 0.2f, 0.06f));
      T  +=go   .create(Rect_C(clientWidth()/2, -0.25f, 0.3f, 0.07f), MLT("GO", PL,u"Idź", RU,u"Двигаться")).func(Go, T).focusable(false);
   }
   GoToArea& GoToArea::show()
{
      if(hidden())
      {
         x.set(S+Floor(ActiveCam.at.x/WorldEdit.areaSize()));
         z.set(S+Floor(ActiveCam.at.z/WorldEdit.areaSize()));
         x.selectAll().activate();
      }
      ::EE::Window::show(); return T;
   }
   void GoToArea::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); Go(T);}
   }
   ElmWorld* PathProps::WS() {return WorldEdit.elm ? WorldEdit.elm->worldData() : null;}
   void PathProps::Changed(C Property &prop) {WorldEdit.changed_settings=true;}
   void PathProps::CellSize(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws->cellSize  (         TextFlt(text) ); pp.toGui();}}
   Str PathProps::CellSize(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws->cell_size ; return S;}
   void PathProps::CtrlRadius(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws->ctrlRadius(         TextFlt(text) ); pp.toGui();}}
   Str PathProps::CtrlRadius(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws->ctrl_r    ; return S;}
   void PathProps::CtrlHeight(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws->ctrlHeight(         TextFlt(text) );            }}
   Str PathProps::CtrlHeight(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws->ctrl_h    ; return S;}
   void PathProps::MaxSlope(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws->maxSlope  (DegToRad(TextFlt(text)));            }}
   Str PathProps::MaxSlope(C PathProps &pp) {if(ElmWorld *ws=WS())return RadToDeg(ws->max_slope); return S;}
   void PathProps::MaxClimb(PathProps &pp, C Str &text) {if(ElmWorld *ws=WS()){ws->maxClimb  (         TextFlt(text) );            }}
   Str PathProps::MaxClimb(C PathProps &pp) {if(ElmWorld *ws=WS())return          ws->max_climb ; return S;}
   void PathProps::IncCellSize(PathProps &pp) {if(ElmWorld *ws=WS()){flt cs=ws->cell_size; cs=1.0f/cs; cs=         AlignFloor(cs+EPS, 0.5f)+0.5f ; cs=1.0f/cs; ws->cellSize(cs); WorldEdit.changed_settings=true; pp.toGui();}}
   void PathProps::DecCellSize(PathProps &pp) {if(ElmWorld *ws=WS()){flt cs=ws->cell_size; cs=1.0f/cs; cs=Max(0.5f, AlignCeil (cs-EPS, 0.5f)-0.5f); cs=1.0f/cs; ws->cellSize(cs); WorldEdit.changed_settings=true; pp.toGui();}}
   void PathProps::IncCtrlRadius(PathProps &pp) {if(ElmWorld *ws=WS()){int rc=       Floor(ws->ctrl_r/ws->cell_size+EPS)+1 ; ws->ctrlRadius(rc*ws->cell_size); WorldEdit.changed_settings=true; pp.toGui();}}
   void PathProps::DecCtrlRadius(PathProps &pp) {if(ElmWorld *ws=WS()){int rc=Max(0, Ceil (ws->ctrl_r/ws->cell_size-EPS)-1); ws->ctrlRadius(rc*ws->cell_size); WorldEdit.changed_settings=true; pp.toGui();}}
   void PathProps::Apply(PathProps &pp) {WorldEdit.flushSettings();}
   void PathProps::Copy(PathProps &pp) {TextData data; FileText f; SaveProperties(pp.props, data.nodes); data.save(f.writeMem()); ClipSet(f.rewind().getAll());}
   void PathProps::Paste(PathProps &pp) {TextData data; FileText f; f.writeMem().putText(ClipGet()); data.load(f.rewind()); LoadProperties(pp.props, data.nodes);}
   PathProps& PathProps::create()
   {
      ::EE::Region::create().skin(&TransparentSkin, false); kb_lit=false;
      props.New().create("Cell Size"        , MemberDesc(DATA_REAL).setFunc(CellSize  , CellSize  )).min(0.01f).mouseEditSpeed(0.2f).desc("Size of single path cell (in meters)\nDefault = 0.333");
      flt cs=props.elms()+0.5f; props.New();
      props.New().create("Controller Radius", MemberDesc(DATA_REAL).setFunc(CtrlRadius, CtrlRadius)).min(0).mouseEditSpeed(0.2f).desc("Controller Radius (in meters)\nwill be rounded according to Cell Size\nDefault = 0.33");
      flt cr=props.elms()+0.5f; props.New();
      props.New().create("Controller Height", MemberDesc(DATA_REAL).setFunc(CtrlHeight, CtrlHeight)).min(0).mouseEditSpeed(0.2f).desc("Controller Height (in meters)\nDefault = 2.0");
      props.New().create("Max Slope Angle"  , MemberDesc(DATA_INT ).setFunc(MaxSlope  , MaxSlope  )).range(0, 90)                    .desc("Maximum slope angle the controller can cross (in degrees)\nDefault = 45");
      props.New().create("Max Climb Height" , MemberDesc(DATA_REAL).setFunc(MaxClimb  , MaxClimb  )).min  (0    ).mouseEditSpeed(0.2f).desc("Maximum height the controller can climb (in meters)\nDefault = 0.7");
      ts.reset(); ts.size=0.04f; ts.align.x=1; tsr=ts; tsr.size*=0.8f; tsr.align.x=-1;
      Rect r=AddProperties(props, T, Vec2(0.02f,-0.02f), 0.043f, 0.15f, &ts);
      flt y;
      y=r.lerpY(1-cs/props.elms()); T+=tcell  .create(Vec2(0.02f, y), "Cells per Meter", &ts); T+=icell  .create(Rect_R(r.max.x, y, 0.045f, 0.045f), ">").func(IncCellSize  , T).focusable(false); T+=dcell  .create(Rect_R(icell  .rect().min.x, y, 0.045f, 0.045f), "<").func(DecCellSize  , T).focusable(false); T+=vcell  .create(Vec2(dcell  .rect().min.x-0.015f, y), S, &tsr);
      y=r.lerpY(1-cr/props.elms()); T+=tctrl_r.create(Vec2(0.02f, y), "Radius Cells"   , &ts); T+=ictrl_r.create(Rect_R(r.max.x, y, 0.045f, 0.045f), ">").func(IncCtrlRadius, T).focusable(false); T+=dctrl_r.create(Rect_R(ictrl_r.rect().min.x, y, 0.045f, 0.045f), "<").func(DecCtrlRadius, T).focusable(false); T+=vctrl_r.create(Vec2(dctrl_r.rect().min.x-0.015f, y), S, &tsr);
      T+=copy .create(Rect_C(r.down()+Vec2(-0.15f, -0.040f), 0.10f, 0.045f), "Copy" ).func(Copy , T).func(Copy, T).desc("Copy settings into system clipboard memory");
      T+=apply.create(Rect_C(r.down()+Vec2(    0, -0.040f), 0.16f, 0.05f ), "Apply").func(Apply, T);
      T+=paste.create(Rect_C(r.down()+Vec2( 0.15f, -0.040f), 0.10f, 0.045f), "Paste").func(Copy , T).func(Paste, T).desc("Paste settings from system clipboard memory");
      rect(Rect_LU(0, 0, r.max.x+0.02f, -r.min.y+0.02f+0.065f));

      REPAO(props).autoData(this).changed(Changed);
      return T;
   }
   void PathProps::toGui()
   {
      if(ElmWorld *ws=WS())
      {
         REPAO(props).toGui();
         vcell  .set(S+TextReal(      1.0f/ws->cell_size, 2));
         vctrl_r.set(S+Ceil    (ws->ctrl_r/ws->cell_size   ));
      }
   }
/******************************************************************************/
