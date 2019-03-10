/******************************************************************************/
class PanelEditor : PropWin
{
   class Change : Edit._Undo.Change
   {
      EditPanel data;

      virtual void create(ptr user)override
      {
         data=PanelEdit.edit;
         PanelEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         PanelEdit.edit.undo(data);
         PanelEdit.setChanged();
         PanelEdit.toGui();
         PanelEdit.undoVis();
      }
   }
   class GuiPanel : GuiCustom
   {
      flt clip_x=0;

      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}

      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            GuiPC gpc2=gpc;
            MAX(gpc2.clip.min.x, clip_x+parent().screenPos().x);
            D.clip(gpc2.clip);
            Rect r=rect()+gpc.offset;
            if(PanelEdit.game)
            {
               PanelEdit.game->draw(r);
               if(PanelEdit.show_lines()){PanelEdit.game->drawLines(RED, r); r.draw(BLACK, false);}
            }
         }
      }
   }

   UID               elm_id=UIDZero;
   Elm              *elm=null;
   bool              changed=false;
   EditPanel         edit;
       PanelPtr      game;
    GuiPanel         gui_panel;
   Button            undo, redo, locate, show_lines,
                     set_corner_size,
                     set_top_size,
                     set_bottom_size,
                     set_side_size;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void PreChanged(C Property &prop) {PanelEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {PanelEdit.setChanged();}

   static void ParamsCenterStretch(EditPanel &e, C Str &t) {e.center_stretch=TextBool(t); e.center_stretch_time.getUTC();}
   static void ParamsSideStretch  (EditPanel &e, C Str &t) {e.  side_stretch=TextBool(t); e.  side_stretch_time.getUTC();}

   static void ParamsCenterColor(EditPanel &e, C Str &t) {e.center_color=TextVec4(t); e.center_color_time.getUTC();}
   static void ParamsBorderColor(EditPanel &e, C Str &t) {e.border_color=TextVec4(t); e.border_color_time.getUTC();}
   static void ParamsSideColor  (EditPanel &e, C Str &t) {e.  side_color=TextVec4(t); e.  side_color_time.getUTC();}
   static void ParamsBlurColor  (EditPanel &e, C Str &t) {e.  blur_color=TextVec4(t); e.  blur_color_time.getUTC();}

   static void ParamsShadowOpacity(EditPanel &e, C Str &t) {e.shadow_opacity=TextInt (t); e.shadow_opacity_time.getUTC();}
   static void ParamsShadowRadius (EditPanel &e, C Str &t) {e.shadow_radius =TextFlt (t); e. shadow_radius_time.getUTC();}
   static void ParamsShadowOffset (EditPanel &e, C Str &t) {e.shadow_offset =TextFlt (t); e. shadow_offset_time.getUTC();}
   static void ParamsShadowCenter (EditPanel &e, C Str &t) {e.center_shadow =TextBool(t); e. center_shadow_time.getUTC();}

   static void ParamsBorderSize        (EditPanel &e, C Str &t) {e.       border_size  =TextFlt (t); e.         border_size_time.getUTC();}
   static void ParamsCenterScale       (EditPanel &e, C Str &t) {e.       center_scale =TextFlt (t); e.        center_scale_time.getUTC();}
   static void ParamsTopSize           (EditPanel &e, C Str &t) {e.          top_size  =TextFlt (t); e.            top_size_time.getUTC();}
   static void ParamsTopOffset         (EditPanel &e, C Str &t) {e.          top_offset=TextFlt (t); e.          top_offset_time.getUTC();}
   static void ParamsBottomSize        (EditPanel &e, C Str &t) {e.       bottom_size  =TextFlt (t); e.         bottom_size_time.getUTC();}
   static void ParamsBottomOffset      (EditPanel &e, C Str &t) {e.       bottom_offset=TextFlt (t); e.       bottom_offset_time.getUTC();}
   static void ParamsLeftRightSize     (EditPanel &e, C Str &t) {e.   left_right_size  =TextFlt (t); e.     left_right_size_time.getUTC();}
   static void ParamsLeftRightOffset   (EditPanel &e, C Str &t) {e.   left_right_offset=TextVec2(t); e.   left_right_offset_time.getUTC();}
   static void ParamsTopCornerSize     (EditPanel &e, C Str &t) {e.   top_corner_size  =TextFlt (t); e.     top_corner_size_time.getUTC();}
   static void ParamsTopCornerOffset   (EditPanel &e, C Str &t) {e.   top_corner_offset=TextVec2(t); e.   top_corner_offset_time.getUTC();}
   static void ParamsBottomCornerSize  (EditPanel &e, C Str &t) {e.bottom_corner_size  =TextFlt (t); e.  bottom_corner_size_time.getUTC();}
   static void ParamsBottomCornerOffset(EditPanel &e, C Str &t) {e.bottom_corner_offset=TextVec2(t); e.bottom_corner_offset_time.getUTC();}

   static Str ParamsCenterImage      (C EditPanel &e) {return Proj.elmFullName(e.       center_image);}
   static Str ParamsBorderImage      (C EditPanel &e) {return Proj.elmFullName(e.       border_image);}
   static Str ParamsTopImage         (C EditPanel &e) {return Proj.elmFullName(e.          top_image);}
   static Str ParamsBottomImage      (C EditPanel &e) {return Proj.elmFullName(e.       bottom_image);}
   static Str ParamsLeftRightImage   (C EditPanel &e) {return Proj.elmFullName(e.   left_right_image);}
   static Str ParamsTopCornerImage   (C EditPanel &e) {return Proj.elmFullName(e.   top_corner_image);}
   static Str ParamsBottomCornerImage(C EditPanel &e) {return Proj.elmFullName(e.bottom_corner_image);}
   static Str ParamsPanelImage       (C EditPanel &e) {return Proj.elmFullName(e.        panel_image);}

   static void ParamsCenterImage      (EditPanel &e, C Str &t) {e.       center_image=Proj.findElmImageID(t                 ); e.       center_image_time.getUTC();}
   static void ParamsBorderImage      (EditPanel &e, C Str &t) {e.       border_image=Proj.findElmImageID(t                 ); e.       border_image_time.getUTC();}
   static void ParamsTopImage         (EditPanel &e, C Str &t) {e.          top_image=Proj.findElmImageID(t                 ); e.          top_image_time.getUTC();}
   static void ParamsBottomImage      (EditPanel &e, C Str &t) {e.       bottom_image=Proj.findElmImageID(t                 ); e.       bottom_image_time.getUTC();}
   static void ParamsLeftRightImage   (EditPanel &e, C Str &t) {e.   left_right_image=Proj.findElmImageID(t                 ); e.   left_right_image_time.getUTC();}
   static void ParamsTopCornerImage   (EditPanel &e, C Str &t) {e.   top_corner_image=Proj.findElmImageID(t                 ); e.   top_corner_image_time.getUTC();}
   static void ParamsBottomCornerImage(EditPanel &e, C Str &t) {e.bottom_corner_image=Proj.findElmImageID(t                 ); e.bottom_corner_image_time.getUTC();}
   static void ParamsPanelImage       (EditPanel &e, C Str &t) {e.        panel_image=Proj.findElmID     (t, ELM_PANEL_IMAGE); e.        panel_image_time.getUTC();}

   static void SetCornerSize(PanelEditor &editor)
   {
      if(editor.game)
      {
         if(editor.game->border_image && editor.game->top_corner_image)
         {
            editor.undos.set("corner");
            editor.edit.top_corner_size=Abs(editor.game->top_corner_image->h()*editor.game->border_size/editor.game->border_image->h());
            editor.edit.top_corner_size_time.getUTC(); editor.setChanged(); editor.toGui();
         }
         if(editor.game->border_image && editor.game->bottom_corner_image)
         {
            editor.undos.set("corner");
            editor.edit.bottom_corner_size=Abs(editor.game->bottom_corner_image->h()*editor.game->border_size/editor.game->border_image->h());
            editor.edit.bottom_corner_size_time.getUTC(); editor.setChanged(); editor.toGui();
         }
      }
   }
   static void SetTopSize(PanelEditor &editor)
   {
      if(editor.game && editor.game->border_image && editor.game->top_image)
      {
         editor.undos.set("top");
         editor.edit.top_size=Abs(editor.game->top_image->h()*editor.game->border_size/editor.game->border_image->h());
         editor.edit.top_size_time.getUTC(); editor.setChanged(); editor.toGui();
      }
   }
   static void SetBottomSize(PanelEditor &editor)
   {
      if(editor.game && editor.game->border_image && editor.game->bottom_image)
      {
         editor.undos.set("bottom");
         editor.edit.bottom_size=Abs(editor.game->bottom_image->h()*editor.game->border_size/editor.game->border_image->h());
         editor.edit.bottom_size_time.getUTC(); editor.setChanged(); editor.toGui();
      }
   }
   static void SetSideSize(PanelEditor &editor)
   {
      if(editor.game && editor.game->border_image && editor.game->left_right_image)
      {
         editor.undos.set("side");
         editor.edit.left_right_size=Abs(editor.game->left_right_image->h()*editor.game->border_size/editor.game->border_image->h());
         editor.edit.left_right_size_time.getUTC(); editor.setChanged(); editor.toGui();
      }
   }

   static void Undo  (PanelEditor &editor) {editor.undos.undo();}
   static void Redo  (PanelEditor &editor) {editor.undos.redo();}
   static void Locate(PanelEditor &editor) {Proj.elmLocate(editor.elm_id);}

   void create()
   {
      add("Shadow Opacity"           , MemberDesc(MEMBER(EditPanel, shadow_opacity)).setTextToDataFunc(ParamsShadowOpacity));
      add("Shadow Radius"            , MemberDesc(MEMBER(EditPanel, shadow_radius )).setTextToDataFunc(ParamsShadowRadius)).mouseEditSpeed(0.05).min(0);
      add("Shadow Offset"            , MemberDesc(MEMBER(EditPanel, shadow_offset )).setTextToDataFunc(ParamsShadowOffset)).mouseEditSpeed(0.05);
      add("Shadow At Center (Slower)", MemberDesc(MEMBER(EditPanel, center_shadow )).setTextToDataFunc(ParamsShadowCenter));

      add("Center Color"  , MemberDesc(MEMBER(EditPanel, center_color  )).setTextToDataFunc(ParamsCenterColor  )).setColor().desc("Affects Center Image and Panel Image");
      add("Center Stretch", MemberDesc(MEMBER(EditPanel, center_stretch)).setTextToDataFunc(ParamsCenterStretch)).desc("If stretch center image to fit the whole rectangle");
      add("Center Scale"  , MemberDesc(MEMBER(EditPanel, center_scale  )).setTextToDataFunc(ParamsCenterScale  )).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).range(-64, 64);
      add("Center Image"  , MemberDesc(DATA_STR                         ).setFunc          (ParamsCenterImage, ParamsCenterImage)).elmType(ELM_IMAGE);
      add("Panel Image"   , MemberDesc(DATA_STR                         ).setFunc          (ParamsPanelImage , ParamsPanelImage )).elmType(ELM_PANEL_IMAGE);

      add("Border Color", MemberDesc(MEMBER(EditPanel, border_color)).setTextToDataFunc(ParamsBorderColor)).setColor().desc("Affects Border Image");
      add("Border Size" , MemberDesc(MEMBER(EditPanel, border_size )).setTextToDataFunc(ParamsBorderSize )).mouseEditSpeed(0.05);
      add("Border Image", MemberDesc(DATA_STR                       ).setFunc          (ParamsBorderImage, ParamsBorderImage      )).elmType(ELM_IMAGE);
      add();
      add("Side Color"          , MemberDesc(MEMBER(EditPanel,           side_color)).setTextToDataFunc(ParamsSideColor         )).setColor().desc("Affects Top Image, Bottom Image, Left/Right Image, Top Corner Image and Bottom Corner Image");
      add("Side Stretch"        , MemberDesc(MEMBER(EditPanel,         side_stretch)).setTextToDataFunc(ParamsSideStretch       )).desc("If stretch side images to fit the whole side\nAffects Top Image, Bottom Image, Left/Right Image, Top Corner Image and Bottom Corner Image");
      add("Top Image"           , MemberDesc(DATA_STR                               ).setFunc          (ParamsTopImage         , ParamsTopImage         )).elmType(ELM_IMAGE);
      add("Top Size"            , MemberDesc(MEMBER(EditPanel,           top_size  )).setTextToDataFunc(ParamsTopSize           )).mouseEditSpeed(0.05);
      add("Top Offset"          , MemberDesc(MEMBER(EditPanel,           top_offset)).setTextToDataFunc(ParamsTopOffset         )).mouseEditSpeed(0.05);
      add("Bottom Image"        , MemberDesc(DATA_STR                               ).setFunc          (ParamsBottomImage      , ParamsBottomImage      )).elmType(ELM_IMAGE);
      add("Bottom Size"         , MemberDesc(MEMBER(EditPanel,        bottom_size  )).setTextToDataFunc(ParamsBottomSize        )).mouseEditSpeed(0.05);
      add("Bottom Offset"       , MemberDesc(MEMBER(EditPanel,        bottom_offset)).setTextToDataFunc(ParamsBottomOffset      )).mouseEditSpeed(0.05);
      add("Left/Right Image"    , MemberDesc(DATA_STR                               ).setFunc          (ParamsLeftRightImage   , ParamsLeftRightImage   )).elmType(ELM_IMAGE);
      add("Left/Right Size"     , MemberDesc(MEMBER(EditPanel,    left_right_size  )).setTextToDataFunc(ParamsLeftRightSize     )).mouseEditSpeed(0.05);
      add("Left/Right Offset"   , MemberDesc(MEMBER(EditPanel,    left_right_offset)).setTextToDataFunc(ParamsLeftRightOffset   )).mouseEditSpeed(0.05);
      add("Top Corner Image"    , MemberDesc(DATA_STR                               ).setFunc          (ParamsTopCornerImage   , ParamsTopCornerImage   )).elmType(ELM_IMAGE);
      add("Top Corner Size"     , MemberDesc(MEMBER(EditPanel,    top_corner_size  )).setTextToDataFunc(ParamsTopCornerSize     )).mouseEditSpeed(0.05);
      add("Top Corner Offset"   , MemberDesc(MEMBER(EditPanel,    top_corner_offset)).setTextToDataFunc(ParamsTopCornerOffset   )).mouseEditSpeed(0.05);
      add("Bottom Corner Image" , MemberDesc(DATA_STR                               ).setFunc          (ParamsBottomCornerImage, ParamsBottomCornerImage)).elmType(ELM_IMAGE);
      add("Bottom Corner Size"  , MemberDesc(MEMBER(EditPanel, bottom_corner_size  )).setTextToDataFunc(ParamsBottomCornerSize  )).mouseEditSpeed(0.05);
      add("Bottom Corner Offset", MemberDesc(MEMBER(EditPanel, bottom_corner_offset)).setTextToDataFunc(ParamsBottomCornerOffset)).mouseEditSpeed(0.05);

      add("Blur Color", MemberDesc(MEMBER(EditPanel, blur_color)).setTextToDataFunc(ParamsBlurColor)).setColor();

      autoData(&edit);

      Rect r=super .create("Panel Editor", Vec2(0.02, -0.07), 0.036, 0.043, PropElmNameWidth); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE; super.changed(Changed, PreChanged);
      T+=undo      .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo      .create(Rect_LU(undo  .rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate    .create(Rect_LU(redo  .rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=show_lines.create(Rect_LU(locate.rect().ru()+Vec2(0.01, 0), 0.22, 0.05), "Show Lines").focusable(false); show_lines.mode=BUTTON_TOGGLE;
      flt y=r.min.y-0.01, h=0.045;
      T+=set_corner_size.create(Rect_LU(0.02, y, 0.69, 0.043), "Set Corner Size to match Border Pixel Ratio").func(SetCornerSize, T); y-=h;
      T+=set_top_size   .create(Rect_LU(0.02, y, 0.69, 0.043), "Set Top Size to match Border Pixel Ratio"   ).func(SetTopSize   , T); y-=h;
      T+=set_bottom_size.create(Rect_LU(0.02, y, 0.69, 0.043), "Set Bottom Size to match Border Pixel Ratio").func(SetBottomSize, T); y-=h;
      T+=set_side_size  .create(Rect_LU(0.02, y, 0.69, 0.043), "Set Side Size to match Border Pixel Ratio"  ).func(SetSideSize  , T); y-=h;
      T+=gui_panel.create();
      Vec2 padd=defaultInnerPaddingSize(); rect(Rect_C(0, 0, Min(1.7+padd.x, D.w()*2), Min(-y+padd.y+0.01, D.h()*2)));
   }
   void toGame() {if(game)edit.copyTo(*game, Proj);}
   void toGui () {super.toGui(); toGame();}

   virtual PanelEditor& hide     (            )  override {set(null); super.hide(); return T;}
   virtual Rect         sizeLimit(            )C override {Rect r=super.sizeLimit(); r.min.set(1.0, 0.5); return r;}
   virtual PanelEditor& rect     (C Rect &rect)  override
   {
      super.rect(rect);
      flt  x=set_top_size.rect().max.x; if(props.elms())MAX(x, props[0].button.rect().max.x);
      Rect r(x, -clientHeight(), clientWidth(), 0); r.extend(-0.08);
      r.setC(r.center(), Min(r.w(), 1.3), Min(r.h(), 1.09));
      gui_panel.clip_x=x;
      gui_panel.rect(r);
      return T;
   }

   void flush()
   {
      if(elm && game && changed)
      {
         if(ElmPanel *data=elm.panelData()){data.newVer(); data.from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm.id);
         Proj.panelChanged(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmPanel *data=elm.panelData()){data.newVer(); data.from(edit);}
         toGame();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_PANEL)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=Proj.gamePath(elm_id);}else{edit.reset(); game.clear();}
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         EditPanel temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj) && elms.elms())
      {
      }
   }
}
PanelEditor PanelEdit;
/******************************************************************************/
