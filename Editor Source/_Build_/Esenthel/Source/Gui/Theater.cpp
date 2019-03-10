/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
TheaterClass Theater;
/******************************************************************************/

/******************************************************************************/
   cchar8 *TheaterClass::t_show[]=
   {
      "All",
      "Publishable",
      "Non-Publishable",
      "Invalid",
   };
   ELM_TYPE TheaterClass::modes[]=
   {
      ELM_OBJ, ELM_MTRL, ELM_IMAGE,
   };
   cchar8 *TheaterClass::t_obj_mode[]=
   {
      "Rotate",
      "Scale",
   };
/******************************************************************************/
      void TheaterClass::Options::Horizontal(  Options &options, C Str &text) {                               options.horizontal=     TextBool(text) ; Theater.refreshSize();}
      void TheaterClass::Options::Rows(  Options &options, C Str &text) {Theater.old_rows=options.rows; options.rows      =     TextInt (text) ; Theater.refreshSize();}
      void TheaterClass::Options::Size(  Options &options, C Str &text) {                               options.item_size =     TextFlt (text) ; Theater.refreshSize();}
      void TheaterClass::Options::Show(  Options &options, C Str &text) {                               options.show      =SHOW(TextInt (text)); Theater.refreshData();}
      void TheaterClass::Options::FOV(  Options &options, C Str &text) {Theater.viewport.fov=DegToRad(TextFlt(text));}
      Str  TheaterClass::Options::FOV(C Options &options             ) {return Round(RadToDeg(Theater.viewport.fov));}
      void TheaterClass::Options::Angle(  Options &options, C Str &text) {options.angle=DegToRad(TextVec2(text));}
      Str  TheaterClass::Options::Angle(C Options &options             ) {return Round(RadToDeg(options.angle));}
      void TheaterClass::Options::Env(  Options &options, C Str &text) {options.env_id=Proj.findElmID(text, ELM_ENV);}
      Str  TheaterClass::Options::Env(C Options &options             ) {return Proj.elmFullName(options.env_id);}
      void TheaterClass::Options::EditMode(  Options &options, C Str &text) {Theater.obj_mode.visible(TextBool(text));}
      Str  TheaterClass::Options::EditMode(C Options &options             ) {return Theater.obj_mode.visible();}
      ::TheaterClass::Options& TheaterClass::Options::create()
      {
         add("Horizontal"             , MemberDesc(MEMBER(Options, horizontal   )).setTextToDataFunc(Horizontal));
 p_rows=&add("Rows"                   , MemberDesc(MEMBER(Options, rows         )).setTextToDataFunc(Rows)).range(1, 10).mouseEditSpeed(2).desc("Change with: Ctrl+MouseWheel");
 p_size=&add("Item Size"              , MemberDesc(MEMBER(Options, item_size    )).setTextToDataFunc(Size)).range(0.1f, 1.5f).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).desc("Change with: Alt+MouseWheel");
p_angle=&add("Item Angle"             , MemberDesc(DATA_VECI2).setFunc(Angle, Angle));
p_scale=&add("Item 3D Scale"          , MemberDesc(MEMBER(Options, item_3d_scale))).range(0.01f, 20).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).desc("Change with: Shift+MouseWheel");
         add("Scale to Fit"           , MemberDesc(MEMBER(Options, scale_fit    ))).desc("Keyboard Shortcut: Alt+S");
         add("Draw at Center"         , MemberDesc(MEMBER(Options, center       ))).desc("Keyboard Shortcut: Alt+C");
         add("Draw Box"               , MemberDesc(MEMBER(Options, draw_box     ))).desc("Keyboard Shortcut: Alt+B");
         add("Draw Axis"              , MemberDesc(MEMBER(Options, draw_axis    ))).desc("Keyboard Shortcut: Alt+A");
         add("Draw Axis on Top"       , MemberDesc(MEMBER(Options, axis_on_top  ))).desc("Keyboard Shortcut: Shift+Alt+A");
         add("Axis Size"              , MemberDesc(MEMBER(Options, axis_size    ))).range(0.01f, 100).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(0.5f);
         add("Rotate"                 , MemberDesc(MEMBER(Options, rotate       )));
         add("Rotation Speed"         , MemberDesc(MEMBER(Options, rot_speed    ))).range(-4, 4);
         add("Variation Cycle Speed"  , MemberDesc(MEMBER(Options, var_speed    ))).range(0, 5).desc("How fast are Object Mesh Material variations cycled through");
         add("Auto Align Flat Objects", MemberDesc(MEMBER(Options, auto_rot_flat_objects)));
         add("Show Elements"          , MemberDesc(MEMBER(Options, show         )).setTextToDataFunc(Show)).setEnum(t_show, Elms(t_show));
         add("Light Direction"        , MemberDesc(MEMBER(Options, light_dir    ))).desc("Keyboard Shortcut: Alt+L");
         add("Environment"            , MemberDesc(MEMBER(Options, env_id       )).setFunc(Env, Env)).elmType(ELM_ENV);
         add("Field of View"          , MemberDesc(DATA_INT ).setFunc(FOV, FOV)).range(5, 75);
         add("Allow Edit Mode"        , MemberDesc(DATA_BOOL).setFunc(EditMode, EditMode)).desc("Edit Mode allows Transforming Objects directly in the Theater");
         autoData(this);

         Rect r=::PropWin::create("Options", Vec2(0.02f, -0.02f), 0.04f, 0.045f, 0.26f); barVisible(false).show(); FlagDisable(flag, WIN_MOVABLE); button[2].func(null);
         size(Vec2(r.max.x, -r.min.y)+0.02f+defaultInnerPaddingSize());
         return T;
      }
      void  TheaterClass::Options::rowsDelta(int delta ) {if(p_rows )p_rows ->textline.set(S+p_rows ->clamp(rows+delta));}
      void  TheaterClass::Options::sizeFactor(flt factor) {if(p_size )p_size ->textline.set(S+p_size ->clamp(item_size    *ScaleFactor(factor)));}
      void TheaterClass::Options::scaleFactor(flt factor) {if(p_scale)p_scale->textline.set(S+p_scale->clamp(item_3d_scale*ScaleFactor(factor)));}
      int TheaterClass::List2::screenToVis(C Vec2 &screen, C GuiPC *gpc)C 
{
         Theater.viewport.setDisplayView(); Camera temp=ActiveCam, cam; cam.set(MatrixIdentity).set();
         Vec pos, dir; ScreenToPosDir(screen, pos, dir); dir*=D.viewRange();
         int vis=-1; flt frac;
         VecI2 vis_range=visibleElmsOnScreen();
         MAX(vis_range.x-=Theater.options.rows,        0);
         MIN(vis_range.y+=Theater.options.rows, elms()-1);
         for(int i=vis_range.x; i<=vis_range.y; i++)
         {
            if(Elm *elm=Proj.findElm(visToData(i)->id))switch(elm->type)
            {
               case ELM_OBJ: if(ObjectPtr obj=Proj.gamePath(elm->id))if(C MeshPtr &mesh=obj->mesh())
               {
                  Rect rect=visToScreenRect(i);
                  Matrix m=Theater.getMatrix(UIDZero, mesh->ext, rect); // use 'UIDZero' so highlight will not change while transforming an object
                  flt f; if(Sweep(pos, dir, *mesh, &m, &f))if(vis<0 || f<frac){vis=i; frac=f;}
               }
            }
         }
         temp.set();
         if(vis>=0)return vis;
         return ::EE::_List::screenToVis(screen, gpc);
      }
   void TheaterClass::ModeChanged(TheaterClass &theater) {theater.refreshData(); theater.refreshNow();}
   void TheaterClass::Mode0(TheaterClass &theater) {theater.mode.set(0);}
   void TheaterClass::Mode1(TheaterClass &theater) {theater.mode.set(1);}
   void TheaterClass::Mode2(TheaterClass &theater) {theater.mode.set(2);}
   void TheaterClass::ModeS0(TheaterClass &theater) {theater.obj_mode.toggle(0);}
   void TheaterClass::ModeS1(TheaterClass &theater) {theater.obj_mode.toggle(1);}
   void TheaterClass::ToggleAxis(TheaterClass &theater) {theater.options.draw_axis  ^=1; theater.options.toGui();}
   void TheaterClass::ToggleAxisTop(TheaterClass &theater) {theater.options.axis_on_top^=1; theater.options.toGui();}
   void TheaterClass::ToggleBox(TheaterClass &theater) {theater.options.draw_box   ^=1; theater.options.toGui();}
   void TheaterClass::ToggleCenter(TheaterClass &theater) {theater.options.center     ^=1; theater.options.toGui();}
   void TheaterClass::ToggleScale(TheaterClass &theater) {theater.options.scale_fit  ^=1; theater.options.toGui();}
   void TheaterClass::ToggleLight(TheaterClass &theater) {theater.options.light_dir  ^=1; theater.options.toGui();}
   void TheaterClass::ToggleOptions(TheaterClass &theater) {theater.show_options.toggle(0);}
   void TheaterClass::DragElmsStart(ProjectEx &proj)
   {
      // get elements from theater list
      proj.drag_list_sel.clear();
      if(ListElm *list_elm=Theater.list())proj.drag_list_sel.add(list_elm->id);
    //REPA(Theater.list.sel)if(ListElm *list_elm=Theater.list.absToData(Theater.list.sel[i]))proj.drag_list_sel.include(list_elm.id);

      // start drag
      proj.dragElmsStart();
   }
   flt TheaterClass::depth()C {return D.viewRange()/10;}
   Matrix TheaterClass::getMatrix(C UID &elm_id, Box box, C Rect &rect)
   {
      bool small=(options.auto_rot_flat_objects && box.size().y<box.size().max()*0.3f);
      flt  depth=T.depth();
      Vec  pos  =ScreenToPos (rect.center(), depth);
      Vec2 size (ScreenToPosD(Vec2(rect.w(), 0), depth).length(),
                 ScreenToPosD(Vec2(0, rect.h()), depth).length());

      Matrix m(1); if(transform_ids.binaryHas(elm_id, Compare))m=transform_matrix;
      if(options.center)
      {
         m.move(-box.center()); box-=box.center(); // move so it's at the center
      }
      flt scale=1; if(options.scale_fit)
      {
         box|=box*Matrix3().setRotateY(PI_4); // include the box when it's rotated at 45 deg
         scale=Min(size.x/box.xz().size().max(), size.y/box.h()); // scale to visible screen size
      }else
      {
         scale*=2*Tan(D.viewFovY()/2);
      }
      scale*=options.item_3d_scale;
      m.scale(scale);
      m.rotateY(options.time_angle-options.angle.x+PI); // rotate Y, use -angle.x to match mouse rotation in all other editors, add PI to front-face objects by default
      m.rotateX(options.angle.y);
      if(small)m.rotateX(-PI_4); // if it's short, then rotate to look from above
      m*=ActiveCam.matrix.orn(); // adjust to camera orientation
      m.move(pos); // move to screen position
      return m;
   }
   void TheaterClass::draw(C UID &elm_id, C Mesh &mesh, C Rect &rect)
   {
      mesh.MeshLod::draw(getMatrix(elm_id, mesh.ext, rect)); // use best LOD
   }
   bool TheaterClass::litSel()C {return obj_mode.visible() && obj_mode()>=0;}
   bool TheaterClass::highlighted(int i)C {return list.lit==i;}
   bool TheaterClass::selected(int i)C {return list.sel.has(list.visToAbs(i));}
      void        TheaterClass::MeshMtrls::clean()  {REP(variations)parts[0].variation(i, null, -1); variations=0;}
      ::TheaterClass::MeshMtrls& TheaterClass::MeshMtrls::create()  {::EE::Mesh::create(1).parts[0].variations(32).base.create(Ball(0.42f), VTX_NRM|VTX_TAN|VTX_TEX0, 16); setRender().setBox(); delBase(); return T;}
      bool    TheaterClass::MeshMtrls::available()C {return InRange(variations, parts[0].variations());}
      Mesh&         TheaterClass::MeshMtrls::set(C MaterialPtr &mtrl, int &variation) {parts[0].variation(variations, mtrl); variation=variations++; return T;}
   void TheaterClass::cleanMeshMaterial() {REPAO(mesh_mtrls).clean();}
   Mesh&  TheaterClass::getMeshMaterial(C MaterialPtr &mtrl, int &variation)
   {
      MeshMtrls *mm=null; FREPA(mesh_mtrls){MeshMtrls &mesh_mtrl=mesh_mtrls[i]; if(mesh_mtrl.available()){mm=&mesh_mtrl; goto found;}}
      mm=&mesh_mtrls.New().create();
   found:
      return mm->set(mtrl, variation);
   }
   void TheaterClass::Render() {Theater.render();}
          void TheaterClass::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            LightDir(options.light_dir ? !(ActiveCam.matrix.z*2+ActiveCam.matrix.x-ActiveCam.matrix.y) : ActiveCam.matrix.z, 1-D.ambientColor()).add(false);

            VecI2 vis_range=list.visibleElmsOnScreen();
            MAX(vis_range.x-=options.rows,             0);
            MIN(vis_range.y+=options.rows, data.elms()-1);
            for(int i=vis_range.x; i<=vis_range.y; i++)
               if(Elm *elm=Proj.findElm(data[i].id))
            {
               Rect rect=list.visToScreenRect(i);
               bool highlighted=(litSel() && T.highlighted(i)),
                    selected   =(litSel() && T.selected   (i));
               switch(elm->type)
               {
                  case ELM_OBJ: if(ObjectPtr obj=Proj.gamePath(elm->id))if(C MeshPtr &mesh=obj->mesh()) // use "obj->mesh" instead of "Proj.gamePath(obj_data.mesh_id)" in case the object inherits mesh from base
                  {
                     int variation=obj->meshVariationIndex();
                     if(!obj->customMeshVariationAny()) // if object doesn't specify a custom variation
                        if(int variations=mesh->variations()) // copy to temp var to avoid div by zero in case mesh for some reason gets modified on another thread (shouldn't happen)
                           variation=Trunc(Time.realTime()*options.var_speed)%variations; // cycle through all available variations
                     Color lit=ColorBrightness(GetLitSelCol(highlighted, selected, TRANSPARENT), 0.33f);
                     SetVariation(variation); SetHighlight(lit); draw(elm->id, *mesh, rect);
                     SetVariation(         ); SetHighlight(   );
                  }break;

                  case ELM_MTRL: if(MaterialPtr mtrl=Proj.gamePath(elm->id))
                  {
                  #if 0 // changing materials during rendering is now illegal
                     Mesh &mesh=MtrlEdit.preview_mesh[0];
                     mesh.material(mtrl);
                     draw(elm.id, mesh, rect);
                  #else
                     int variation; Mesh &mesh=getMeshMaterial(mtrl, variation);
                     SetVariation(variation); draw(elm->id, mesh, rect); SetVariation();
                  #endif
                  }break;
               }
            }
         }break;
      }
   }
   void TheaterClass::Draw(Viewport &viewport) {Theater.draw();}
          void TheaterClass::draw()
   {
      // environment
      EnvironmentPtr env=EnvEdit.cur(); if(!env)env=Proj.gamePath(options.env_id); if(!env)env=&DefaultEnvironment; env->set();

      bool       line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
      AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
      DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
      MOTION_MODE  motion   =D.   motionMode(); D.   motionMode( MOTION_NONE);
      bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
      bool         astros   =AstrosDraw       ; AstrosDraw     =false;
      bool         ocean    =Water.draw       ; Water.draw     =false;
      Camera       temp     =ActiveCam,    cam; cam  .set(MatrixIdentity).set();

      Renderer(Render); cleanMeshMaterial(); // clean after rendering finished

      if(options.draw_axis || options.draw_box)
      {
         if(options.axis_on_top)D.depthLock(false);else Renderer.setDepthForDebugDrawing();
         SetMatrix();
      }

      VecI2 vis_range=list.visibleElmsOnScreen();
      for(int i=vis_range.x; i<=vis_range.y; i++)
      {
         Rect rect=list.visToScreenRect(i);
         if(Elm *elm=Proj.findElm(data[i].id))switch(elm->type)
         {
            case ELM_OBJ: if(options.draw_axis || options.draw_box)if(ObjectPtr obj=Proj.gamePath(elm->id))if(C MeshPtr &mesh=obj->mesh())
            {
               Matrix m=getMatrix(UIDZero, mesh->ext, rect); // use 'UIDZero' to avoid scaling matrix for drawing
               if(options.draw_box)OBox(mesh->ext, m).draw();
               if(options.draw_axis)m.scaleOrn(options.axis_size).draw();
            }break;

            case ELM_IMAGE: if(ImagePtr image=Proj.gamePath(elm->id))
            {
               Rect image_rect=image->fit(rect);
               if(image->mode()==IMAGE_CUBE)image->drawCubeFace(WHITE, TRANSPARENT, image_rect, DIR_FORWARD);
               else                         image->drawFilter  (image_rect);
            }break;
         }
         if(litSel())
         {
            Color lit=GetLitSelCol(highlighted(i), selected(i), TRANSPARENT); if(lit.a)
            {
               lit.a/= 2; rect.draw(lit, false);
               lit.a/=12; rect.draw(lit, true );
            }
         }
      }

      if(options.draw_axis || options.draw_box)
         if(options.axis_on_top)D.depthUnlock();

      temp.set();
      D.      dofMode(dof        );
      D.   motionMode(motion     );
      D.  ambientMode(ambient    );
      D.eyeAdaptation(eye_adapt  );
      D.lineSmooth   (line_smooth);
      AstrosDraw     =astros;
      Water.draw     =ocean;
   }
   void TheaterClass::setVisibility(bool vis)
   {
      vis&=Proj.theater();
                   visible(vis);
      viewport    .visible(vis);
      mode        .visible(vis);
      show_options.visible(vis);
      menu        .enabled(vis);
      SetKbExclusive();
   }
   void TheaterClass::hideDo()
   {
      if(Proj.theater())Proj.theater.push();
   }
   void TheaterClass::create()
   {
      cchar8 *options_t[]={"Options"};
      Gui+=viewport    .create(Draw); viewport.fov=DegToRad(30);
      Gui+=::EE::Region::create().slidebarSize(0.06f).skin(&EmptyGuiSkin, false); kb_lit=false; REPAO(slidebar).skin(&HalfTransparentSkin);
      Gui+=mode        .create((cchar8**)null, ELMS(modes)).valid(true).set(0).func(ModeChanged, T); REPA(mode)mode.tab(i).setText(Plural(ElmTypeName[modes[i]])).desc(S+"Keyboard Shortcut: F"+(i+1));
      FREPA(modes)if(modes[i]==ELM_OBJ)
      {
         mode.tab(i)+=obj_mode.create(t_obj_mode, Elms(t_obj_mode)).hide();
         REPA(obj_mode)obj_mode.tab(i).desc(S+"Select with LMB\nTransform with RMB\nKeyboard Shortcut: Shift+F"+(i+1));
         break;
      }
      Gui+=show_options.create(options_t, Elms(options_t)); show_options.tab(0)+=options.create(); show_options.tab(0).desc("Keyboard Shortcut: F10");
      T  +=list.create().drawMode(LDM_RECTS).horizontal(true).skin(&EmptyGuiSkin); list.flag|=LIST_MULTI_SEL;

      Node<MenuElm> menu;
      menu.New().create("Mode0"  , Mode0 , T).kbsc(KbSc(KB_F1));
      menu.New().create("Mode1"  , Mode1 , T).kbsc(KbSc(KB_F2));
      menu.New().create("Mode2"  , Mode2 , T).kbsc(KbSc(KB_F3));
      menu.New().create("ModeS0" , ModeS0, T).kbsc(KbSc(KB_F1, KBSC_SHIFT));
      menu.New().create("ModeS1" , ModeS1, T).kbsc(KbSc(KB_F2, KBSC_SHIFT));
      menu.New().create("Options", ToggleOptions, T).kbsc(KbSc(KB_F10));
      menu.New().create("Box"    , ToggleBox    , T).kbsc(KbSc(KB_B, KBSC_ALT));
      menu.New().create("Axis"   , ToggleAxis   , T).kbsc(KbSc(KB_A, KBSC_ALT));
      menu.New().create("AxisTop", ToggleAxisTop, T).kbsc(KbSc(KB_A, KBSC_ALT|KBSC_SHIFT));
      menu.New().create("Center" , ToggleCenter , T).kbsc(KbSc(KB_C, KBSC_ALT));
      menu.New().create("Scale"  , ToggleScale  , T).kbsc(KbSc(KB_S, KBSC_ALT));
      menu.New().create("Light"  , ToggleLight  , T).kbsc(KbSc(KB_L, KBSC_ALT));
      Gui+=T.menu.create(menu);

      setVisibility(false);
   }
   void TheaterClass::moveAbove(GuiObj &go)
   {
      show_options.moveAbove(go);
      mode        .moveAbove(go);
      ::EE::GuiObj::moveAbove(go);
      viewport    .moveAbove(go);
   }
   void TheaterClass::listElms(ElmNode &node, bool parent_contains_name)
   {
      node.children.sort(ProjectEx::CompareChildren); // needs to be sorted in case it wasn't before (this can happen if these elements aren't visible in the list)
      ELM_TYPE elm_type=modes[mode()];
      bool     ignore_publish=(options.show!=PUBLISHABLE && options.show!=INVALID);
      FREPA(node.children)
      {
         int      child_i=node.children [i];
         ElmNode &child  =Proj.hierarchy[child_i];
         Elm     &elm    =Proj.elms     [child_i];
         if(elm.finalExists())
         if(ignore_publish || elm.finalPublish()) // we're going recursively down, from parents to their children, so before checking children, we can only check for modes that will affect all children in the same way, and this is only if element is going to be published
         {
            bool this_contains_name=true, child_contains_name=true; // always show when no filter
            if(Proj.filter().is())
            {
                this_contains_name=(FlagTest(child.flag, ELM_CONTAINS_NAME) || parent_contains_name); // for performance reasons, merge 'this_contains_name' with 'parent_contains_name'
               child_contains_name= FlagTest(child.flag, ELM_CONTAINS_NAME_CHILD);
            }
            if(elm.type==elm_type && this_contains_name)
            {
               switch(options.show)
               {
                  case NON_PUBLISHABLE: if(  elm.finalPublish()  )goto skip; break;
                  case INVALID        : if(!Proj.invalidRefs(elm))goto skip; break;
               }
               data.New().id=elm.id;
            }
         skip:
            listElms(child, this_contains_name); // list children
         }
      }
   }
   Vec2 TheaterClass::offset()C
   {
      Vec2 offset=0; if(flt size=list.imageSizeBase().c[!options.horizontal])
      {
         flt scale=(old_rows ? old_rows : options.rows)/size;
         REPA(offset)offset.c[i]=slidebar[i].offset()*scale;
      }
      return offset;
   }
   void TheaterClass::offset(C Vec2 &offset)
   {
      flt scale=list.imageSizeBase().c[!options.horizontal]/options.rows;
      REPA(offset)slidebar[i].offset(offset.c[i]*scale);
   }
   void TheaterClass::refreshSize() {refresh|=REFRESH_SIZE;}
   void TheaterClass::refreshData() {refresh|=REFRESH_DATA;}
   void TheaterClass::refreshNow()
   {
      Vec2 offset=T.offset();
      Vec2 size=rect().size()-slidebarSize();
      if(options.horizontal)
      {
         size=size.y/options.rows;
         size.x*=options.item_size;
      }else
      {
         size.x/=options.rows;
         MIN(size.y, size.x);
         size.y*=options.item_size;
      }

      if(refresh&REFRESH_DATA)
      {
         // save selection
         Memt<UID> sel; sel.reserve(list.sel.elms()); FREPA(list.sel)if(ListElm *elm=list.absToData(list.sel[i]))sel.binaryInclude(elm->id, Compare);

         data.clear();
         if(!Proj.filter_is_id)listElms(Proj.root);else
         if( Proj.findElm(Proj.filter_id))data.New().id=Proj.filter_id;
         list.setData(data);

         // restore selection
         list.sel.clear(); if(sel.elms())FREPA(data)if(sel.binaryHas(data[i].id, Compare))list.sel.add(i);
      }

      list.imageSize(size, 0).horizontal(options.horizontal);
      T.offset(offset);
      old_rows=0;
      Rect sr=slidebar[0].rect(); MAX(sr.min.x, Misc.rect().max.x); slidebar[0].rect(sr);

      refresh=0;
   }
   void TheaterClass::update(C GuiPC &gpc)
{
      // process mouse wheel before super.update so it's not handled by the region slidebars
      if(visible() && gpc.visible && Gui.ms()==&list && Ms.wheel())
         if(Kb.ctrl() || Kb.shift() || Kb.alt())
      {
            if(Kb.ctrl ())options. rowsDelta (-Ms.wheel()    );else
         {
            if(Kb.shift())options.scaleFactor( Ms.wheel()*0.1f); // allow processing both alt+shift at the same time
            if(Kb.alt  ())options. sizeFactor( Ms.wheel()*0.1f); // allow processing both alt+shift at the same time
         }
         Ms.eatWheel();
      }

      ::EE::Region::update(gpc);

      if(visible() && gpc.visible)
      {
         bool rotate=options.rotate, can_transform=false;
         if(Gui.ms()==&list)
         {
            if(litSel() && modes[mode()]==ELM_OBJ)
            {
               // transform
               if(Ms.bp(1)) // start transforming
               {
                  transform_ids.clear();
                  if(!list.sel.has(list.visToAbs(list.lit)))list.sel.clear(); // if highlited is not selected, then clear selection
                  FREPA(list.sel)if(ListElm *elm=list.absToData(list.sel[i]))transform_ids.binaryInclude(elm->id, Compare);
                  if(!transform_ids.elms())if(ListElm *elm=list.visToData(list.lit))transform_ids.add(elm->id);
                  transform_rotate_y=0;
                  transform_rotate_y_frac=0;
                  transform_scale=1;
               }else
               if(Ms.br(1) && transform_ids.elms()) // apply transform
               {
                  Proj.transformApply(transform_ids, transform_matrix);
                  transform_ids.clear();
               }
               if(transform_ids.elms() && Ms.b(1)) // adjust transform
               {
                  can_transform=true; rotate=false;
                  Ms.freeze();
                  switch(obj_mode())
                  {
                     case OBJ_ROT:
                     {
                        transform_rotate_y_frac+=Ms.d().sum()*-1.33f;
                        if(int t=Trunc(transform_rotate_y_frac))
                        {
                           transform_rotate_y+=t;
                           transform_rotate_y_frac-=t;
                        }
                     }break;

                     case OBJ_SCALE:
                     {
                        transform_scale*=ScaleFactor(Ms.d().sum()*(Kb.shift() ? 0.1f : 1.0f));
                     }break;
                  }
                  transform_matrix.setRotateY(transform_rotate_y*PI_2).scale(transform_scale);
               }
            }

            if(Ms.bp(0))
               if(ListElm *elm=list())
            {
               if(Ms.bd(0))Proj.elmToggle(elm->id); // call this first, because it may open folders (parents)

               Proj.elmLocate(elm->id, true); // call this next, once parents are opened
               Gui.drag(ProjectEx::DragElmsFinish, Proj, null, DragElmsStart, ProjectEx::DragElmsCancel);
            }
            if(Ms.wheel())
            {
               if(Kb.ctrl ()){options. rowsDelta (-Ms.wheel()    ); Ms.eatWheel();}else
               if(Kb.shift()){options.scaleFactor( Ms.wheel()*0.1f); Ms.eatWheel();}else
               if(Kb.alt  ()){options. sizeFactor( Ms.wheel()*0.1f); Ms.eatWheel();}
            }
            if(Ms.b(2) || Ms.b(4) || Kb.win())
            {
               Ms.freeze();
               if(!(!Kb.ctrlCmd() &&  Kb.alt()))options.angle.x+=Ms.d().x; // disable X rotation when only alt  pressed
               if(!( Kb.ctrlCmd() && !Kb.alt()))options.angle.y+=Ms.d().y; // disable Y rotation when only ctrl pressed
               options.toGui();
               rotate=false;
            }
         }
         if(Ms.bp(2) && Gui.window()==&options)show_options.toggle(0);

         if(refresh)refreshNow();

         if(options.p_angle && options.p_angle->button())rotate=false;
         if(rotate)options.time_angle+=options.rot_speed*Time.ad();

         if(!can_transform || Kb.kf(KB_ESC))transform_ids.clear(); // cancel transform
      }
   }
   void TheaterClass::resize()
   {
      if(visible())
      {
         Rect r=EditRect(false); if(rect()!=r)
         {
                         rect(r);
            viewport    .rect(r);
            mode        .rect(Rect_U(r.up(), 0.6f, 0.06f));
            show_options.rect(Rect_RU(r.ru()-Vec2(slidebarSize(), 0), 0.24f, 0.06f));
            obj_mode    .rect(Rect_U(Avg(mode.rect().max.x, show_options.rect().min.x), r.max.y, 0.28f, 0.06f), 0, true);
            options     .posRU(show_options.rect().rd()-Vec2(0, 0.02f));
            refreshNow();
         }
      }
   }
TheaterClass::TheaterClass() : refresh(0), old_rows(0) {}

TheaterClass::Options::Options() : horizontal(true), rotate(true), auto_rot_flat_objects(true), scale_fit(true), center(true), draw_axis(false), axis_on_top(true), draw_box(false), light_dir(true), rows(2), item_size(1), item_3d_scale(1), axis_size(1), time_angle(0), rot_speed(0.5f), var_speed(1), angle(0), env_id(UIDZero), show(PUBLISHABLE), p_rows(null), p_size(null), p_scale(null), p_angle(null) {}

TheaterClass::ListElm::ListElm() : id(UIDZero) {}

TheaterClass::MeshMtrls::MeshMtrls() : variations(0) {}

/******************************************************************************/
