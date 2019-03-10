/******************************************************************************/
class GuiView : Region
{
   static const flt   MinSize=0.02;
   static       Color TextRectColor(0, 0, 0, 0);
   static       void  ViewportDraw(Viewport &viewport) {D.clearCol(BLACK);}
   static       bool  IgnoreSelected=false;

   class Button2    : GuiObjs.ObjName<Button   > {~Button2   () {GuiEdit.removed(this);}   Button2   () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class CheckBox2  : GuiObjs.ObjName<CheckBox > {~CheckBox2 () {GuiEdit.removed(this);}   CheckBox2 () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void update(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.update(temp);} };
   class ComboBox2  : GuiObjs.ObjName<ComboBox > {~ComboBox2 () {GuiEdit.removed(this);}   ComboBox2 () {create(); size(Vec2(0.3, 0.05));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void update(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.update(temp);} };
   class GuiCustom2 : GuiObjs.ObjName<GuiCustom> {~GuiCustom2() {GuiEdit.removed(this);}   GuiCustom2() {create(); size(Vec2(0.3));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void draw(C GuiPC &gpc)override {if(gpc.visible && visible()){D.clip(gpc.clip); (rect()+gpc.offset).draw(BLACK, false);}} };
   class Desktop2   : GuiObjs.ObjName<Desktop  > {~Desktop2  () {GuiEdit.removed(this);}   Desktop2  () {create();}   /*virtual GuiObj* test(C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(pos, mouse_wheel);}*/ };
   class GuiImage2  : GuiObjs.ObjName<GuiImage > {~GuiImage2 () {GuiEdit.removed(this);}   GuiImage2 () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class List2      : GuiObjs.ObjName<_List    > {~List2     () {GuiEdit.removed(this);}   List2     () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class Menu2      : GuiObjs.ObjName<Menu     > {~Menu2     () {GuiEdit.removed(this);}   Menu2     () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class MenuBar2   : GuiObjs.ObjName<MenuBar  > {~MenuBar2  () {GuiEdit.removed(this);}   MenuBar2  () {         }   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}};
   class Progress2  : GuiObjs.ObjName<Progress > {~Progress2 () {GuiEdit.removed(this);}   Progress2 () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class Region2    : GuiObjs.ObjName<Region   > {~Region2   () {GuiEdit.removed(this);}   Region2   () {create(); size(Vec2(0.3));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class SlideBar2  : GuiObjs.ObjName<SlideBar > {~SlideBar2 () {GuiEdit.removed(this);}   SlideBar2 () {create(); size(Vec2(0.3, 0.05));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class Slider2    : GuiObjs.ObjName<Slider   > {~Slider2   () {GuiEdit.removed(this);}   Slider2   () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class Tabs2      : GuiObjs.ObjName<Tabs     > {~Tabs2     () {GuiEdit.removed(this);}   Tabs2     () {cchar8 *t[]={"Tab 0", "Tab 1", "Tab 2"}; create(t, Elms(t)); size(Vec2(0.6, 0.06));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);} };
   class Text2      : GuiObjs.ObjName<Text     > {~Text2     () {GuiEdit.removed(this);}   Text2     () {create("Text"); size(Vec2(0.04));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void draw(C GuiPC &gpc)override {super.draw(gpc); if(gpc.visible && visible() && TextRectColor.a){D.clip(gpc.clip); (rect()+gpc.offset).draw(TextRectColor, false);}} };
   class TextBox2   : GuiObjs.ObjName<TextBox  > {~TextBox2  () {GuiEdit.removed(this);}   TextBox2  () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void update(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.update(temp);}   virtual void draw(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.draw(temp);} };
   class TextLine2  : GuiObjs.ObjName<TextLine > {~TextLine2 () {GuiEdit.removed(this);}   TextLine2 () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void update(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.update(temp);}   virtual void draw(C GuiPC &gpc)override {GuiPC temp=gpc; temp.enabled=false; super.draw(temp);} };
   class Viewport2  : GuiObjs.ObjName<Viewport > {~Viewport2 () {GuiEdit.removed(this);}   Viewport2 () {create();}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void draw(C GuiPC &gpc)override {draw_func=ViewportDraw; super.draw(gpc);} };
   class Window2    : GuiObjs.ObjName<Window   > {~Window2   () {GuiEdit.removed(this);}   Window2   () {create(); size(Vec2(0.3));}   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(gpc, pos, mouse_wheel);}   virtual void update(C GuiPC &gpc)override {uint flag=T.flag; FlagDisable(T.flag, WIN_MOVABLE|WIN_RESIZABLE); super.update(gpc); T.flag=flag;} };

   class Objects
   {
      GuiObj *parent=null;

      Memx<Button2   > button  ;
      Memx<CheckBox2 > checkbox;
      Memx<ComboBox2 > combobox;
      Memx<GuiCustom2> custom  ;
      Memx<Desktop2  > desktop ;
      Memx<GuiImage2 > image   ;
      Memx<List2     > list    ;
      Memx<Menu2     > menu    ;
      Memx<MenuBar2  > menubar ;
      Memx<Progress2 > progress;
      Memx<Region2   > region  ;
      Memx<SlideBar2 > slidebar;
      Memx<Slider2   > slider  ;
      Memx<Tabs2     > tabs    ;
      Memx<Text2     > text    ;
      Memx<TextBox2  > textbox ;
      Memx<TextLine2 > textline;
      Memx<Viewport2 > viewport;
      Memx<Window2   > window  ;

      // get
      Memx<GuiObj>* container(GUI_OBJ_TYPE type)
      {
         switch(type)
         {
            default         : return null;
            case GO_BUTTON  : return &SCAST(Memx<GuiObj>, button  );
            case GO_CHECKBOX: return &SCAST(Memx<GuiObj>, checkbox);
            case GO_COMBOBOX: return &SCAST(Memx<GuiObj>, combobox);
            case GO_CUSTOM  : return &SCAST(Memx<GuiObj>, custom  );
            case GO_DESKTOP : return &SCAST(Memx<GuiObj>, desktop );
            case GO_IMAGE   : return &SCAST(Memx<GuiObj>, image   );
            case GO_LIST    : return &SCAST(Memx<GuiObj>, list    );
            case GO_MENU    : return &SCAST(Memx<GuiObj>, menu    );
            case GO_MENU_BAR: return &SCAST(Memx<GuiObj>, menubar );
            case GO_PROGRESS: return &SCAST(Memx<GuiObj>, progress);
            case GO_REGION  : return &SCAST(Memx<GuiObj>, region  );
            case GO_SLIDEBAR: return &SCAST(Memx<GuiObj>, slidebar);
            case GO_SLIDER  : return &SCAST(Memx<GuiObj>, slider  );
            case GO_TABS    : return &SCAST(Memx<GuiObj>, tabs    );
            case GO_TEXT    : return &SCAST(Memx<GuiObj>, text    );
            case GO_TEXTBOX : return &SCAST(Memx<GuiObj>, textbox );
            case GO_TEXTLINE: return &SCAST(Memx<GuiObj>, textline);
            case GO_VIEWPORT: return &SCAST(Memx<GuiObj>, viewport);
            case GO_WINDOW  : return &SCAST(Memx<GuiObj>, window  );
         }
      }
      bool containsMain(GuiObj *go) {return contains(MainObject(go));} // if 'go' is stored in this objects
      bool contains    (GuiObj *go) // if 'go' is stored in this objects
      {
         if(go)REP(GO_NUM)if(Memx<GuiObj> *container=T.container(GUI_OBJ_TYPE(i)))if(container.contains(go))return true;
         return false;
      }

      // manage
      void del()
      {
         REP(GO_NUM)if(Memx<GuiObj> *memx=container(GUI_OBJ_TYPE(i)))memx.del();
      }
      void create(C GuiObjs &objs) // create from
      {
         del();
         objs.copyTo(button, checkbox, combobox, custom, desktop, image, list, menu, menubar, progress, region, slidebar, slider, tabs, text, textbox, textline, viewport, window, parent);
      }
      void copyTo(GuiObjs &objs)C
      {
         objs.create(button, checkbox, combobox, custom, desktop, image, list, menu, menubar, progress, region, slidebar, slider, tabs, text, textbox, textline, viewport, window);
      }

      // operations
      GuiObj* New(GUI_OBJ_TYPE type)
      {
         if(Memx<GuiObj> *container=T.container(type))return &container.New();
         return null;
      }
      void remove(GuiObj *go)
      {
         // delete all children first, so all objects are removed from containers
         if(go && contains(go))
         {
            REP(go.childNum())remove(go.child(i)); // go from end
            if(Memx<GuiObj> *container=T.container(go.type()))container.removeData(go);
         }
      }

      // io
      bool save(File &f, cchar *path)C
      {
         GuiObjs objs; copyTo(objs); return objs.save(f, path);
      }
      bool load(File &f, cchar *path)
      {
         GuiObjs objs; if(objs.load(f, path)){create(objs); return true;}
         del(); return false;
      }
      bool save(C Str &name)C
      {
         File f; if(save(f.writeMem(), GetPath(name))){f.pos(0); return SafeOverwrite(f, name);}
         return false;
      }
      bool load(C Str &name)
      {
         File f; if(f.readTry(name))return load(f, GetPath(name));
         del(); return false;
      }
   }

   static void PosX (  GuiObj &go, C Str &t) {       go.pos(Vec2(TextFlt(t), go.pos().y));}
   static Str  PosX (C GuiObj &go          ) {return go.pos().x;}
   static void PosY (  GuiObj &go, C Str &t) {       go.pos(Vec2(go.pos().x, TextFlt(t)));}
   static Str  PosY (C GuiObj &go          ) {return go.pos().y;}
   static void SizeX(  GuiObj &go, C Str &t) {       go.size(Vec2(TextFlt(t), go.size().y));}
   static Str  SizeX(C GuiObj &go          ) {return go.size().x;}
   static void SizeY(  GuiObj &go, C Str &t) {       go.size(Vec2(go.size().x, TextFlt(t)));}
   static Str  SizeY(C GuiObj &go          ) {return go.size().y;}
   static void Desc (  GuiObj &go, C Str &t) {               go.desc(Replace(t, "\\n", "\n" ));}
   static Str  Desc (C GuiObj &go          ) {return Replace(go.desc(), "\n" , "\\n") ;}

   static void CheckBoxOn   (  CheckBox &checkbox, C Str &t) {       checkbox.set(TextBool(t));}
   static Str  CheckBoxOn   (C CheckBox &checkbox          ) {return checkbox();}
   static void ProgressValue(  Progress &progress, C Str &t) {       progress.set(TextFlt(t));}
   static Str  ProgressValue(C Progress &progress          ) {return progress();}
   static void RegionSbc    (  Region   &region  , C Str &t) {       region.slidebar[0].sbc=region.slidebar[1].sbc=SLIDEBAR_BACK_CLICK_MODE(TextInt(t));}
   static Str  RegionSbc    (C Region   &region            ) {return region.slidebar[0].sbc;}

   static void SlideBarLength     (  SlideBar &slidebar, C Str &t) {       slidebar.setLengths(TextFlt(t), slidebar.lengthTotal());}
   static Str  SlideBarLength     (C SlideBar &slidebar          ) {return slidebar.length();}
   static void SlideBarLengthTotal(  SlideBar &slidebar, C Str &t) {       slidebar.setLengths(slidebar.length(), TextFlt(t));}
   static Str  SlideBarLengthTotal(C SlideBar &slidebar          ) {return slidebar.lengthTotal();}
   static void SlideBarFrac       (  SlideBar &slidebar, C Str &t) {       slidebar.set(TextFlt(t));}
   static Str  SlideBarFrac       (C SlideBar &slidebar          ) {return slidebar();}

   static void SliderValue(  Slider &slider, C Str &t) {       slider.set(TextFlt(t));}
   static Str  SliderValue(C Slider &slider          ) {return slider();}

                     static Str  TabsValid   (C Tabs &tabs          ) {return tabs.valid();}
                     static void TabsValid   (  Tabs &tabs, C Str &t) {       tabs.valid(TextBool(t));}
                     static Str  TabsLayout  (C Tabs &tabs          ) {return tabs.layout();}
                     static void TabsLayout  (  Tabs &tabs, C Str &t) {       tabs.layout((TABS_LAYOUT)TextInt(t));}
                     static Str  TabsSpace   (C Tabs &tabs          ) {return tabs.space();}
                     static void TabsSpace   (  Tabs &tabs, C Str &t) {       tabs.rect(tabs.rect(), TextFlt(t), tabs.autoSize());}
                     static Str  TabsAutoSize(C Tabs &tabs          ) {return tabs.autoSize();}
                     static void TabsAutoSize(  Tabs &tabs, C Str &t) {       tabs.rect(tabs.rect(), tabs.space(), TextBool(t));}
   template<int i>   static Str  TabsTabText (C Tabs &tabs          ) {return InRange(i, tabs) ? tabs.tab(i).text( ) : S;}
   template<int i>   static void TabsTabText (  Tabs &tabs, C Str &t) {    if(InRange(i, tabs))  tabs.tab(i).text(t);    }

   static void TextCodeFunc(  Text &text, C Str &t) {               text.code(Replace(t, "\\n", "\n" ));}
   static Str  TextCodeFunc(C Text &text          ) {return Replace(text.code(),         "\n" , "\\n") ;}

   static Str  TextLineText     (C TextLine &textline          ) {return textline();}
   static void TextLineText     (  TextLine &textline, C Str &t) {       textline.set(t);}
   static Str  TextLinePassword (C TextLine &textline          ) {return textline.password();}
   static void TextLinePassword (  TextLine &textline, C Str &t) {       textline.password(TextBool(t));}
   static Str  TextLineMaxLength(C TextLine &textline          ) {return textline.maxLength();}
   static void TextLineMaxLength(  TextLine &textline, C Str &t) {       textline.maxLength(TextInt(t));}

   static Str  TextBoxText     (C TextBox &textbox          ) {return textbox();}
   static void TextBoxText     (  TextBox &textbox, C Str &t) {       textbox.set(t);}
   static Str  TextBoxMaxLength(C TextBox &textbox          ) {return textbox.maxLength();}
   static void TextBoxMaxLength(  TextBox &textbox, C Str &t) {       textbox.maxLength(TextInt(t));}

   static void WindowLevel     (  Window &window, C Str &t) {       window.level(TextInt(t));}
   static Str  WindowLevel     (C Window &window          ) {return window.level();}
   static void WindowMovable   (  Window &window, C Str &t) {       FlagSet (window.flag, WIN_MOVABLE, TextBool(t));}
   static Str  WindowMovable   (C Window &window          ) {return FlagTest(window.flag, WIN_MOVABLE);}
   static void WindowResizable (  Window &window, C Str &t) {       FlagSet (window.flag, WIN_RESIZABLE, TextBool(t));}
   static Str  WindowResizable (C Window &window          ) {return FlagTest(window.flag, WIN_RESIZABLE);}
   static void WindowBarVisible(  Window &window, C Str &t) {       window.barVisible(TextBool(t));}
   static Str  WindowBarVisible(C Window &window          ) {return window.barVisible();}
 //static void WindowBarHeight (  Window &window, C Str &t) {       window.barHeight (TextFlt (t));}
 //static Str  WindowBarHeight (C Window &window          ) {return window.barHeight ();}

   static void ButtonImage  (Button   &button  , C Str &t) {button  .image=Proj.gamePath(Proj.findElmImageID(t));}
   static void ButtonSkin   (Button   &button  , C Str &t) {button  .skin =Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN));}
   static void CheckBoxSkin (CheckBox &checkbox, C Str &t) {checkbox.skin =Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN));}
   static void ComboBoxSkin (ComboBox &combobox, C Str &t) {combobox.skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void ImageImage   (GuiImage &image   , C Str &t) {image   .image=Proj.gamePath(Proj.findElmImageID(t));}
   static void ProgressSkin (Progress &progress, C Str &t) {progress.skin =Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN));}
   static void SlideBarSkin (SlideBar &slidebar, C Str &t) {slidebar.skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void SliderSkin   (Slider   &slider  , C Str &t) {slider  .skin =Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN));}
   static void RegionSkin   (Region   &region  , C Str &t) {region  .skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void TabsSkin     (Tabs     &tabs    , C Str &t) {tabs    .skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void TextSkin     (Text     &text    , C Str &t) {text    .skin =Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN));}
   static void TextLineSkin (TextLine &textline, C Str &t) {textline.skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void TextBoxSkin  (TextBox  &textbox , C Str &t) {textbox .skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void WindowSkin   (Window   &window  , C Str &t) {window  .skin (Proj.gamePath(Proj.findElmID     (t, ELM_GUI_SKIN)));}
   static void TextTextStyle(Text     &text    , C Str &t) {text    .text_style=Proj.gamePath(Proj.findElmID(t, ELM_TEXT_STYLE));}

   static Str ButtonImage  (C Button   &button  ) {return Proj.elmFullName(button  .image .id());}
   static Str ButtonSkin   (C Button   &button  ) {return Proj.elmFullName(button  .skin  .id());}
   static Str CheckBoxSkin (C CheckBox &checkbox) {return Proj.elmFullName(checkbox.skin  .id());}
   static Str ComboBoxSkin (C ComboBox &combobox) {return Proj.elmFullName(combobox.skin().id());}
   static Str ImageImage   (C GuiImage &image   ) {return Proj.elmFullName(image   .image .id());}
   static Str ProgressSkin (C Progress &progress) {return Proj.elmFullName(progress.skin  .id());}
   static Str SlideBarSkin (C SlideBar &slidebar) {return Proj.elmFullName(slidebar.skin().id());}
   static Str SliderSkin   (C Slider   &slider  ) {return Proj.elmFullName(slider  .skin  .id());}
   static Str RegionSkin   (C Region   &region  ) {return Proj.elmFullName(region  .skin().id());}
   static Str TabsSkin     (C Tabs     &tabs    ) {return Proj.elmFullName(tabs    .skin().id());}
   static Str TextSkin     (C Text     &text    ) {return Proj.elmFullName(text    .skin  .id());}
   static Str TextLineSkin (C TextLine &textline) {return Proj.elmFullName(textline.skin().id());}
   static Str TextBoxSkin  (C TextBox  &textbox ) {return Proj.elmFullName(textbox .skin().id());}
   static Str WindowSkin   (C Window   &window  ) {return Proj.elmFullName(window  .skin().id());}
   static Str TextTextStyle(C Text     &text    ) {return Proj.elmFullName(text    .text_style.id());}

   class ObjEdit : PropWin
   {
      static GuiSkinPtr Skin;

      static void PreChanged(C Property &prop) {Skin=Gui.skin; if(GuiSkinPtr app_skin=Proj.appGuiSkin())Gui.skin=app_skin; GuiEdit.undo.set(&prop);} // set skin because that may affect object properties (for example slidebar button visibility, window client rectangle)
      static void    Changed(C Property &prop) {GuiEdit.setChanged(); Gui.skin=Skin; Skin=null;}

      Rect create(GUI_OBJ_TYPE type, int level)
      {
         if(props.elms())
         {
            props.NewAt(0).create("Pos X" , MemberDesc(DATA_REAL).setFunc( PosX,  PosX)).mouseEditSpeed(0.1);
            props.NewAt(1).create("Pos Y" , MemberDesc(DATA_REAL).setFunc( PosY,  PosY)).mouseEditSpeed(0.1);
            props.NewAt(2).create("Width" , MemberDesc(DATA_REAL).setFunc(SizeX, SizeX)).min(MinSize).mouseEditSpeed(0.1);
            props.NewAt(3).create("Height", MemberDesc(DATA_REAL).setFunc(SizeY, SizeY)).min(MinSize).mouseEditSpeed(0.1);
            Rect rect=super.create(S+GuiObjTypeName(type)+" Properties", Vec2(0.02, -0.02), 0.036, 0.043, PropElmNameWidth); changed(Changed, PreChanged).level(level);
            Mode.tab(MODE_GUI)+=T; // move to gui tab so it will be hidden in other modes
            button[2].func(null); // disable auto-closing
            return rect;
         }
         return Rect(Vec2(0));
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(gpc.visible && visible())
         {
            GuiSkinPtr temp=Gui.skin; Gui.skin=Mode.Gui_skin; // restore Editor's Gui.skin before drawing this Window
            super.draw(gpc);
            Gui.skin=temp;
         }
      }
   }

   static GuiObj* MainObject(GuiObj *go)
   {
      if(go)
      {
         if(go.parent())switch(go.type())
         {
            case GO_BUTTON: if(go.parent().type()==GO_SLIDEBAR)go=go.parent(); break; // one of slidebar buttons
            case GO_TAB   : go=go.parent(); break;
         }
         if(go.parent() && go.parent().type()==GO_REGION)
         {
            Region &region=go.parent().asRegion();
            if(go==&region.slidebar[0]
            || go==&region.slidebar[1]
            || go==&region.view       )go=&region;
         }
      }
      return go;
   }
   static GuiObj* MainParent(GuiObj *parent, bool allow_tabs)
   {
      if(!allow_tabs && parent && parent.type()==GO_TAB)if(GuiObj *tabs=parent.parent())if(tabs.type()==GO_TABS)return MainParent(tabs.parent(), allow_tabs);
      return (parent==&Mode.tab(MODE_GUI)) ? Gui.desktop() : parent;
   }
   static GuiObj* FirstContainer(GuiObj *go, bool allow_tabs)
   {
      for(; go; go=go.parent())switch(go.type())
      {
         case GO_MENU   :
         case GO_DESKTOP:
         case GO_REGION :
         case GO_WINDOW : return go;
         case GO_TAB    : return allow_tabs ? go : FirstContainer(go.parent(), allow_tabs); break;
      }
      return null;
   }

   enum OP
   {
      OP_NEW     ,
      OP_DEL     ,
      OP_MOVE    ,
      OP_SIZE    ,
      OP_COPY    ,
      OP_SEPARATE,
      OP_SCALE   ,
      OP_PARENT  ,
      OP_NUM     ,
   }
   static GUI_OBJ_TYPE obj_new[]=
   {
      GO_BUTTON  , // Button
      GO_CHECKBOX, // CheckBox
    //GO_MENU    , // Menu
      GO_COMBOBOX, // ComboBox
      GO_CUSTOM  , // Custom
      GO_IMAGE   , // Image
    //GO_LIST    , // List
    //GO_MENU_BAR, // MenuBar
      GO_PROGRESS, // PogressBar
      GO_REGION  , // Region
      GO_SLIDEBAR, // SlideBar
      GO_SLIDER  , // Slider
      GO_TABS    , // Tabs
      GO_TEXT    , // Text
      GO_TEXTBOX , // TextBox
      GO_TEXTLINE, // TextLine
      GO_VIEWPORT, // Viewport
      GO_WINDOW  , // Window
   }; static int obj_new_elms=Elms(obj_new);

   class Panel : Window
   {
      flt            grid_size=0.010;
      Button         draw_proportions, grid_align, undo, redo, locate, align_sel, aspect, move_up, move_down;
      Tabs           op, obj_type;
      Memx<Property> props;

      static void AlignSel(Panel &panel) {GuiEdit.alignSel   ();}
      static void MoveUp  (Panel &panel) {GuiEdit.moveUpSel  ();}
      static void MoveDown(Panel &panel) {GuiEdit.moveDownSel();}
      static void Aspect  (Panel &panel) {GuiEdit.resetAspect();}
      static void Undo    (Panel &panel) {GuiEdit.undo.undo  ();}
      static void Redo    (Panel &panel) {GuiEdit.undo.redo  ();}
      static void Locate  (Panel &panel) {Proj.elmLocate(GuiEdit.elm_id);}

      GUI_OBJ_TYPE objType()C
      {
         if(InRange(obj_type(), obj_new))return obj_new[obj_type()];
         return GO_NONE;
      }
      void align(Vec2 &pos, bool force=false)
      {
         if((grid_align() || force) && grid_size>EPS)pos.set(AlignRound(pos.x, grid_size), AlignRound(pos.y, grid_size));
      }
      void alignTrunc(Vec2 &pos)
      {
         if(grid_align() && grid_size>EPS)pos.set(AlignTrunc(pos.x, grid_size), AlignTrunc(pos.y, grid_size));
      }
      Panel& create()
      {
         super.create(Rect_RU(D.w(), D.h(), 0.31, 1.13)).level(/*LicenseCheck.level()*/256-2).barVisible(false);
         flt h=0.05, b=0.03, p=0.01;
         T+=draw_proportions.create(Rect_LU(b, -b, h, h)).focusable(false).desc("Draw screen proportions:\nPurple : X Range    -1 .. 1   , Aspect Ratio  1:1\nBlue   : X Range -1.25 .. 1.25, Aspect Ratio 5:4 (1280x1024)\nGreen : X Range -1.33 .. 1.33, Aspect Ratio 4:3 (1024x768)\nRed   : X Range -1.60 .. 1.60, Aspect Ratio 16:10 (1680x1050)\nCyan  : X Range -1.77 .. 1.77, Aspect Ratio 16:9 (1920x1080)"); draw_proportions.mode=BUTTON_TOGGLE; draw_proportions.image="Gui/Misc/grid_small.img";
         T+=grid_align.create(Rect_LU(draw_proportions.rect().ru(), h, h)).focusable(false).desc("Align objects to grid"); grid_align.mode=BUTTON_TOGGLE; grid_align.set(true); grid_align.image="Gui/Misc/grid.img";
         T+=op.create(Rect_LU(draw_proportions.rect().ld()+Vec2(0, -p), h*5, h*2), 0, (cchar**)null, OP_NUM); REPA(op)op.tab(i).size(Vec2(h, h)).pos(op.rect().lu());
         op.tab(OP_NEW     ).setText ("+1"                    ).move(Vec2(h*0,  0)).desc(S+"Create new object\nSelect with LeftClick\nInsert with RightClick\n\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Insert");
         op.tab(OP_COPY    ).setText ("Copy"                  ).move(Vec2(h*0, -h)).desc(S+"Copy selected objects\nSelect with LeftClick\nCopy with RightClick\n\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+D").resize(Vec2(h, 0));
         op.tab(OP_DEL     ).setText ("-1"                    ).move(Vec2(h*1,  0)).desc(S+"Delete object\nDelete with RightClick\n\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Del");
         op.tab(OP_MOVE    ).setImage("Gui/Misc/move.img"     ).move(Vec2(h*2,  0)).desc(S+"Move object\nSelect with LeftClick\nMove with RightClick");
         op.tab(OP_SEPARATE).setImage("Gui/Misc/separate.img" ).move(Vec2(h*2, -h)).desc(S+"Separate group of objects\nSelect with LeftClick\nSeparate with RightClick\nShortcut: Shift + Right Mouse Button");
         op.tab(OP_SIZE    ).setImage("Gui/Misc/scale.img"    ).move(Vec2(h*3,  0)).desc(S+"Resize object\nSelect with LeftClick\nResize with RightClick\nShortcut: "+Kb.ctrlCmdName()+" + Right Mouse Button");
         op.tab(OP_SCALE   ).setImage("Gui/Misc/scale_all.img").move(Vec2(h*3, -h)).desc(S+"Move and resize a group of objects\nSelect with LeftClick\nMove+Resize with RightClick\nShortcut: "+Kb.ctrlCmdName()+" + Shift + Right Mouse Button");
         op.tab(OP_PARENT  ).setImage("Gui/Misc/target.img"   ).move(Vec2(h*4,  0)).desc(S+"Move object and change its parent to object under mouse cursor\nSelect with LeftClick\nMove with RightClick\nShortcut: Alt + Right Mouse Button\n\nIn this mode you don't need to actually move the object to change its parent,\nyou can just select an object, move the mouse over to desired parent, and RightClick.");
         T+=undo     .create(Rect_LU(b, op.rect().min.y-p,  h, h)).func(Undo    , T).focusable(false).desc(S+"Undo\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Z"                                  ); undo     .image="Gui/Misc/undo.img";
         T+=redo     .create(Rect_LU(undo     .rect().ru(), h, h)).func(Redo    , T).focusable(false).desc(S+"Redo\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Y"                                  ); redo     .image="Gui/Misc/redo.img";
         T+=locate   .create(Rect_LU(redo.rect().ru(), 0.15, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
         T+=align_sel.create(Rect_LU(undo     .rect().ld(), h, h)).func(AlignSel, T).focusable(false).desc(S+"Align selected objects to grid\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+G"        ); align_sel.text ="G";
         T+=aspect   .create(Rect_LU(align_sel.rect().ru(), h, h)).func(Aspect  , T).focusable(false).desc(S+"Reset aspect ratio of selected objects\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+R"); aspect   .text ="R";
         T+=move_up  .create(Rect_LU(aspect   .rect().ru(), h, h)).func(MoveUp  , T).focusable(false).desc(S+"Move selected objects up\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+PageUp"         ); move_up  .image="Gui/arrow_up.img";
         T+=move_down.create(Rect_LU(move_up  .rect().ru(), h, h)).func(MoveDown, T).focusable(false).desc(S+"Move selected objects down\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+PageDown"     ); move_down.image="Gui/arrow_down.img";
         T+=obj_type .create(Rect_LU(b, move_down.rect().min.y-p, 0.25, 0.053*obj_new_elms), 0, (cchar**)null, obj_new_elms).valid(true).set(0);
         FREPA(obj_type)obj_type.tab(i).text(GuiObjTypeName(obj_new[i]));
         props.New().create(S, MEMBER(Panel, grid_size)).range(0.001, 0.1).mouseEditSpeed(0.01).desc("Grid size");
         AddProperties(props, T, grid_align.rect().ru(), h, 0.14); REPAO(props).autoData(this);
         return T;
      }
   }

   class Change : Edit._Undo.Change
   {
      GuiObjs objs;

      virtual void create(ptr user)override
      {
         GuiEdit.objs.copyTo(objs);
         GuiEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         if(GuiEdit.elm)
         {
            GuiEdit.objs.create(objs);
            GuiEdit.setChanged();
            GuiEdit.undoVis();
         }
      }
   }

   bool              changed=false, selecting=false, disable_selection=false;
   Vec2              sel_pos=0, helper=0;
   UID               elm_id=UIDZero;
   Elm              *elm=null;
   Objects           objs;
   Panel             panel;
   ObjEdit           obj_edit[GO_NUM];
   Button            new_tab;
   int               op=-1;
   GuiObj           *sel_parent=null, *last=null;
   Memc<GuiObj*>     sel, lit, temp;
   Edit.Undo<Change> undo(true);   void undoVis() {SetUndo(undo, panel.undo, panel.redo);}

   bool selected()C {return Mode()==MODE_GUI;}
   void selectedChanged()
   {
      panel.visible(selected()).enabled(selected());
   }
   void flush()
   {
      if(elm && changed)
      {
         if(ElmGui *data=elm.guiData())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         objs.save(Proj.gamePath(*elm));
         Server.setElmLong(elm.id);
         Preview.elmChanged(elm.id);
      }
      changed=false;
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_GUI)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undo.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm)
         {
            GuiSkinPtr temp=Gui.skin; if(GuiSkinPtr skin=Proj.appGuiSkin())Gui.skin=skin; // set app skin before loading objects because that may affect some of their properties (such as slidebar side button visibility)
            objs.load(Proj.gamePath(*elm));
            Gui.skin=temp;
         }else objs.del();
         Proj.refresh(false, false);
         Mode.tabAvailable(MODE_GUI, elm!=null);
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm){Mode.set(MODE_GUI); HideBig();}}
   void toggle  (Elm *elm)
   {
      if(elm==T.elm && selected())elm=null;
      activate(elm);
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}

   static void DelTab(ptr user)
   {
      if(Tabs *tabs=CAST(Tabs, GuiEdit.cur()))
      {
         int i=(uintptr)user;
         if(InRange(i, *tabs))
         {
            GuiEdit.undo.set("delTab", true);
            Tab &tab=tabs.tab(i);
            if(GuiObj *parent=tabs.parent())REP(tab.childNum())*parent+=*tab.child(i); // move tab children to tabs parent
            tabs.remove(i);
            if(tabs.tabs())GuiEdit.createPropsTabs(tabs).toGui();else GuiEdit.objs.remove(tabs); // if there are no tabs left then remove the object
            GuiEdit.setChanged();
         }
      }
   }
   static void NewTab(GuiView &gv)
   {
      if(Tabs *tabs=CAST(Tabs, gv.cur()))
      {
         gv.undo.set("newTab", true);
         tabs.New(S);
         gv.createPropsTabs(tabs).toGui().setChanged();
      }
   }
   GuiView& createPropsTabs(Tabs *tabs)
   {
      ObjEdit &oe=obj_edit[GO_TABS];
      int      t =(tabs ? tabs.tabs() : 0);

      static cchar8 *tabs_layout[]=
      {
         "AUTO"      , // 0
         "HORIZONTAL", // 1
         "VERTICAL"  , // 2
      }; ASSERT(TABS_AUTO==0 && TABS_HORIZONTAL==1 && TABS_VERTICAL==2 && TABS_NUM==3);

                          oe.props.clear();
                          oe.add("Name"         , MemberDesc(MEMBER(Tabs2, name)));
                          oe.add("Desc"         , MemberDesc(DATA_STR ).setFunc(Desc           , Desc           ));
                          oe.add("Valid"        , MemberDesc(DATA_BOOL).setFunc(TabsValid      , TabsValid      )).desc("If at least one tab must be selected");
                          oe.add("Layout"       , MemberDesc(         ).setFunc(TabsLayout     , TabsLayout     )).setEnum(tabs_layout, Elms(tabs_layout));
                          oe.add("Space"        , MemberDesc(DATA_REAL).setFunc(TabsSpace      , TabsSpace      )).min(0).mouseEditSpeed(0.2f).desc("Space between tabs");
                          oe.add("Auto Size"    , MemberDesc(DATA_BOOL).setFunc(TabsAutoSize   , TabsAutoSize   )).desc("If set tab sizes according to their text length");
                          oe.add("Skin Override", MemberDesc(DATA_STR ).setFunc(TabsSkin       , TabsSkin       )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");
      if(t>= 1){Button &b=oe.add("Tab 1"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<0> , TabsTabText<0> )).button; b.create().func(DelTab, (Ptr) 0).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 2){Button &b=oe.add("Tab 2"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<1> , TabsTabText<1> )).button; b.create().func(DelTab, (Ptr) 1).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 3){Button &b=oe.add("Tab 3"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<2> , TabsTabText<2> )).button; b.create().func(DelTab, (Ptr) 2).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 4){Button &b=oe.add("Tab 4"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<3> , TabsTabText<3> )).button; b.create().func(DelTab, (Ptr) 3).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 5){Button &b=oe.add("Tab 5"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<4> , TabsTabText<4> )).button; b.create().func(DelTab, (Ptr) 4).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 6){Button &b=oe.add("Tab 6"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<5> , TabsTabText<5> )).button; b.create().func(DelTab, (Ptr) 5).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 7){Button &b=oe.add("Tab 7"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<6> , TabsTabText<6> )).button; b.create().func(DelTab, (Ptr) 6).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 8){Button &b=oe.add("Tab 8"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<7> , TabsTabText<7> )).button; b.create().func(DelTab, (Ptr) 7).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>= 9){Button &b=oe.add("Tab 9"        , MemberDesc(DATA_STR ).setFunc(TabsTabText<8> , TabsTabText<8> )).button; b.create().func(DelTab, (Ptr) 8).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=10){Button &b=oe.add("Tab 10"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<9> , TabsTabText<9> )).button; b.create().func(DelTab, (Ptr) 9).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=11){Button &b=oe.add("Tab 11"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<10>, TabsTabText<10>)).button; b.create().func(DelTab, (Ptr)10).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=12){Button &b=oe.add("Tab 12"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<11>, TabsTabText<11>)).button; b.create().func(DelTab, (Ptr)11).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=13){Button &b=oe.add("Tab 13"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<12>, TabsTabText<12>)).button; b.create().func(DelTab, (Ptr)11).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=14){Button &b=oe.add("Tab 14"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<13>, TabsTabText<13>)).button; b.create().func(DelTab, (Ptr)11).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=15){Button &b=oe.add("Tab 15"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<14>, TabsTabText<14>)).button; b.create().func(DelTab, (Ptr)11).desc("Remove this tab"); b.image="Gui/close.img";}
      if(t>=16){Button &b=oe.add("Tab 16"       , MemberDesc(DATA_STR ).setFunc(TabsTabText<15>, TabsTabText<15>)).button; b.create().func(DelTab, (Ptr)11).desc("Remove this tab"); b.image="Gui/close.img";}

      bool visible=(oe.is() && oe.visible());
      Rect r=oe.create(GO_TABS, panel.level());
      oe+=new_tab.create(Rect_U(oe.clientWidth()/2, -r.h()-0.03, 0.2, 0.05), "New Tab").func(NewTab, T);
      oe.rect(Rect_RD(oe.rect().rd(), oe.rect().w(), oe.rect().h()+0.06));
      oe.autoData(tabs).visible(visible); // keep visibility
      return T;
   }
   void createProps()
   {
      obj_edit[GO_BUTTON].add("Name"         , MemberDesc(MEMBER(Button2, name       )));
      obj_edit[GO_BUTTON].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc) );
      obj_edit[GO_BUTTON].add("Text"         , MemberDesc(MEMBER(Button2, text       )));
      obj_edit[GO_BUTTON].add("Text Align"   , MemberDesc(MEMBER(Button2, text_align ))).range(-1, 1);
      obj_edit[GO_BUTTON].add("Text Size"    , MemberDesc(MEMBER(Button2, text_size  ))).range(0.01, 2.0);
      obj_edit[GO_BUTTON].add("Image"        , MemberDesc(MEMBER(Button2, image      )).setFunc(ButtonImage, ButtonImage)).elmType(ELM_IMAGE);
      obj_edit[GO_BUTTON].add("Image Color"  , MemberDesc(MEMBER(Button2, image_color))).setColor();
      obj_edit[GO_BUTTON].add("Skin Override", MemberDesc(DATA_STR).setFunc(ButtonSkin, ButtonSkin)).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_CHECKBOX].add("Name"         , MemberDesc(MEMBER(CheckBox2, name)));
      obj_edit[GO_CHECKBOX].add("Desc"         , MemberDesc(DATA_STR ).setFunc(Desc, Desc));
      obj_edit[GO_CHECKBOX].add("On"           , MemberDesc(DATA_BOOL).setFunc(CheckBoxOn, CheckBoxOn));
      obj_edit[GO_CHECKBOX].add("Skin Override", MemberDesc(DATA_STR ).setFunc(CheckBoxSkin, CheckBoxSkin)).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_COMBOBOX].add("Name"         , MemberDesc(MEMBER(ComboBox2, name)));
      obj_edit[GO_COMBOBOX].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_COMBOBOX].add("Skin Override", MemberDesc(DATA_STR).setFunc(ComboBoxSkin, ComboBoxSkin)).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_CUSTOM].add("Name", MemberDesc(MEMBER(GuiCustom2, name)));
      obj_edit[GO_CUSTOM].add("Desc", MemberDesc(DATA_STR).setFunc(Desc, Desc));

      obj_edit[GO_IMAGE].add("Name"      , MemberDesc(MEMBER(GuiImage2, name     )));
      obj_edit[GO_IMAGE].add("Desc"      , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_IMAGE].add("Fit"       , MemberDesc(MEMBER(GuiImage2, fit       )));
      obj_edit[GO_IMAGE].add("Rect Color", MemberDesc(MEMBER(GuiImage2, rect_color))).setColor();
      obj_edit[GO_IMAGE].add("Color"     , MemberDesc(MEMBER(GuiImage2, color     ))).setColor();
      obj_edit[GO_IMAGE].add("Color Add" , MemberDesc(MEMBER(GuiImage2, color_add ))).setColor();
      obj_edit[GO_IMAGE].add("Image"     , MemberDesc(MEMBER(GuiImage2, image     )).setFunc(ImageImage, ImageImage)).elmType(ELM_IMAGE);

      static cchar8 *progress_text[]=
      {
         "NONE"   , // 0
         "PERCENT", // 1
         "REAL"   , // 2
      }; ASSERT(PROGRESS_NONE==0 && PROGRESS_PERCENT==1 && PROGRESS_REAL==2);

      obj_edit[GO_PROGRESS].add("Name"         , MemberDesc(MEMBER(Progress2, name      )));
      obj_edit[GO_PROGRESS].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_PROGRESS].add("Text Mode"    , MemberDesc(MEMBER(Progress2, mode      ))).setEnum(progress_text, Elms(progress_text));
      obj_edit[GO_PROGRESS].add("Progress"     , MemberDesc(DATA_REAL).setFunc(ProgressValue, ProgressValue)).range(0, 1).mouseEditSpeed(0.1);
      obj_edit[GO_PROGRESS].add("Skin Override", MemberDesc(DATA_STR ).setFunc(ProgressSkin , ProgressSkin )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      static cchar8 *sbc_text[]=
      {
         "OFF"    , // 0
         "STEP"   , // 1
         "SMOOTH" , // 2
         "SET_POS", // 3
      }; ASSERT(SBC_OFF==0 && SBC_STEP==1 && SBC_SMOOTH==2 && SBC_SET_POS==3);

      obj_edit[GO_REGION].add("Name"         , MemberDesc(MEMBER(Region2, name)));
      obj_edit[GO_REGION].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_REGION].add("Back Click"   , MemberDesc().setFunc(RegionSbc, RegionSbc)).setEnum(sbc_text, Elms(sbc_text));
      obj_edit[GO_REGION].add("Skin Override", MemberDesc(DATA_STR).setFunc(RegionSkin, RegionSkin)).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_SLIDEBAR].add("Name"         , MemberDesc(MEMBER(SlideBar2, name         )));
      obj_edit[GO_SLIDEBAR].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_SLIDEBAR].add("Back Click"   , MemberDesc(MEMBER(SlideBar2, sbc          ))).setEnum(sbc_text, Elms(sbc_text));
      obj_edit[GO_SLIDEBAR].add("Length"       , MemberDesc(DATA_REAL).setFunc(SlideBarLength     , SlideBarLength     )).min(0);
      obj_edit[GO_SLIDEBAR].add("Length Total" , MemberDesc(DATA_REAL).setFunc(SlideBarLengthTotal, SlideBarLengthTotal)).min(0);
      obj_edit[GO_SLIDEBAR].add("Fraction"     , MemberDesc(DATA_REAL).setFunc(SlideBarFrac       , SlideBarFrac       )).range(0, 1).mouseEditSpeed(0.1);
      obj_edit[GO_SLIDEBAR].add("Skin Override", MemberDesc(DATA_STR ).setFunc(SlideBarSkin       , SlideBarSkin       )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_SLIDER].add("Name"         , MemberDesc(MEMBER(Slider2, name)));
      obj_edit[GO_SLIDER].add("Desc"         , MemberDesc(DATA_STR ).setFunc(Desc, Desc));
      obj_edit[GO_SLIDER].add("Value"        , MemberDesc(DATA_REAL).setFunc(SliderValue, SliderValue)).range(0, 1).mouseEditSpeed(0.1);
      obj_edit[GO_SLIDER].add("Skin Override", MemberDesc(DATA_STR ).setFunc(SliderSkin , SliderSkin )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      static cchar8 *auto_line[]=
      {
         "NONE"       , // 0
         "SPACE"      , // 1
         "SPACE_SPLIT", // 2
         "SPLIT"      , // 3
      }; ASSERT(AUTO_LINE_NONE==0 && AUTO_LINE_SPACE==1 && AUTO_LINE_SPACE_SPLIT==2 && AUTO_LINE_SPLIT==3 && AUTO_LINE_NUM==4);

      obj_edit[GO_TEXT].add("Name", MemberDesc(MEMBER(Text2, name)));
      obj_edit[GO_TEXT].add("Desc", MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_TEXT].add("Code", MemberDesc(DATA_STR).setFunc(TextCodeFunc, TextCodeFunc)).desc(
         "1. code format accepts following keywords:   in following formats:\n"
         "   col, color   -   RGB, RGBA, RRGGBB, RRGGBBAA (hexadecimal format)\n"
         "   shadow   -   X, XX (hexadecimal format)\n"
         "\n"
         "2. codes should be surrounded by '[' ']' signs\n"
         "\n"
         "3. removing the effect of a code should be handled by '/' sign followed by code name\n"
         "\n"
         "4. sample codes:\n"
         "      \"Text without code. [color=F00]Text with code[/color]\"   -   will force red color on \"Text with code\"\n"
         "      \"[shadow=0]No Shadow[/shadow] [shadow=F]Full Shadow[/shadow]\"   -   will force no shadow on \"No Shadow\" and full shadow on \"Full Shadow\"");
      obj_edit[GO_TEXT].add("Auto Line", MemberDesc(MEMBER(Text2, auto_line))).setEnum(auto_line, Elms(auto_line)).desc(
         "NONE            - only manual new lines '\\n' are used\n"
         "SPACE          - new lines can be  calculated on spaces, words are not split\n"
         "SPACE_SPLIT - new lines can be  calculated on spaces, words can be  split if their length exceeds available width\n"
         "SPLIT            - new lines are not calculated on spaces, words can be  split if their length exceeds available width");

      obj_edit[GO_TEXT].add("Text Style"   , MemberDesc(DATA_STR).setFunc(TextTextStyle, TextTextStyle)).elmType(ELM_TEXT_STYLE);
      obj_edit[GO_TEXT].add("Skin Override", MemberDesc(DATA_STR).setFunc(TextSkin     , TextSkin     )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_TEXTBOX].add("Name"         , MemberDesc(MEMBER(TextBox2, name          )));
      obj_edit[GO_TEXTBOX].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc   ));
      obj_edit[GO_TEXTBOX].add("Text"         , MemberDesc(DATA_STR).setFunc(TextBoxText, TextBoxText));
      obj_edit[GO_TEXTBOX].add("Hint"         , MemberDesc(MEMBER(TextBox2, hint          )));
      obj_edit[GO_TEXTBOX].add("Max Length"   , MemberDesc(DATA_INT ).setFunc(TextBoxMaxLength, TextBoxMaxLength)).min(-1).desc("Maximum allowed text length, -1 = no limit");
      obj_edit[GO_TEXTBOX].add("Skin Override", MemberDesc(DATA_STR ).setFunc(TextBoxSkin     , TextBoxSkin     )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_TEXTLINE].add("Name"         , MemberDesc(MEMBER(TextLine2, name          )));
      obj_edit[GO_TEXTLINE].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc   ));
      obj_edit[GO_TEXTLINE].add("Text"         , MemberDesc(DATA_STR).setFunc(TextLineText, TextLineText));
      obj_edit[GO_TEXTLINE].add("Hint"         , MemberDesc(MEMBER(TextLine2, hint          )));
      obj_edit[GO_TEXTLINE].add("Show Find"    , MemberDesc(MEMBER(TextLine2, show_find     )));
      obj_edit[GO_TEXTLINE].add("Password"     , MemberDesc(DATA_BOOL).setFunc(TextLinePassword , TextLinePassword ));
      obj_edit[GO_TEXTLINE].add("Max Length"   , MemberDesc(DATA_INT ).setFunc(TextLineMaxLength, TextLineMaxLength)).min(-1).desc("Maximum allowed text length, -1 = no limit");
      obj_edit[GO_TEXTLINE].add("Skin Override", MemberDesc(DATA_STR ).setFunc(TextLineSkin     , TextLineSkin     )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      obj_edit[GO_VIEWPORT].add("Name"      , MemberDesc(MEMBER(Viewport2, name      )));
      obj_edit[GO_VIEWPORT].add("Desc"      , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_VIEWPORT].add("Rect Color", MemberDesc(MEMBER(Viewport2, rect_color))).setColor();

      obj_edit[GO_WINDOW].add("Name"         , MemberDesc(MEMBER(Window2, name       )));
      obj_edit[GO_WINDOW].add("Desc"         , MemberDesc(DATA_STR).setFunc(Desc, Desc));
      obj_edit[GO_WINDOW].add("Title"        , MemberDesc(MEMBER(Window2, title      )));
      obj_edit[GO_WINDOW].add("Level"        , MemberDesc(DATA_INT ).setFunc(WindowLevel     , WindowLevel     )).mouseEditSpeed(4);
      obj_edit[GO_WINDOW].add("Movable"      , MemberDesc(DATA_BOOL).setFunc(WindowMovable   , WindowMovable   ));
      obj_edit[GO_WINDOW].add("Resizable"    , MemberDesc(DATA_BOOL).setFunc(WindowResizable , WindowResizable ));
      obj_edit[GO_WINDOW].add("Bar Visible"  , MemberDesc(DATA_BOOL).setFunc(WindowBarVisible, WindowBarVisible));
    //obj_edit[GO_WINDOW].add("Bar Height"   , MemberDesc(DATA_REAL).setFunc(WindowBarHeight , WindowBarHeight )).min(0).mouseEditSpeed(0.05);
      obj_edit[GO_WINDOW].add("Skin Override", MemberDesc(DATA_STR ).setFunc(WindowSkin      , WindowSkin      )).elmType(ELM_GUI_SKIN).desc("Override Skin, if no skin is specified then default will be used");

      REPAO(obj_edit).create(GUI_OBJ_TYPE(i), panel.level());
      createPropsTabs(null);
   }
   GuiView& create(GuiObj &parent)
   {
      parent+=super.create();
      objs.parent=&parent;
      Gui+=panel.create().hide().disabled(true);
      undoVis();
      createProps();
      removeSlideBars();
      return T;
   }
   void resize() {rect(EditRect());}
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undo.set(null, true);
         objs.load(Proj.gamePath(*elm));
      }
   }
   void setChanged(bool to_gui=false)
   {
      if(elm)
      {
         if(ElmGui *data=elm.guiData()){data.newVer(); data.file_time.getUTC();}
         if(to_gui)toGui();
         changed=true;
      }
   }

   // get
   GuiObj*          cur(                 ) {return (sel.elms()==1) ? sel[0] : null;}
   GuiObj*          cur(GUI_OBJ_TYPE type) {if(GuiObj *obj=cur())if(obj.type()==type)return obj; return null;}
   bool selectedParents(GuiObj *obj) {for(; obj; obj=obj.parent())if(sel.has(obj))return true; return false;} // if object or its parents are selected
   bool        editable(GuiObj *obj) {return obj==Gui.desktop() || obj==this || objs.containsMain(obj);}
   Rect         selRect(           ) {return Rect().from(sel_pos, Ms.pos());}
   bool      hasKbFocus(           )
   {
      if(editable(Gui.kb()))return true;
      if(panel.contains(Gui.kb())
      || Gui.kb()==&Mode)if(Gui.kb().type()!=GO_TEXTLINE && Gui.kb().type()!=GO_TEXTBOX)return true;
      return false;
   }

   // operations
   void removed(GuiObj *obj)
   {
      if(obj){sel.exclude(obj, true); lit.exclude(obj, true); if(obj.contains(sel_parent))sel_parent=null;}
      REPA(obj_edit)if(GuiObj *data=(GuiObj*)obj_edit[i].autoData())if(obj.contains(data))obj_edit[i].autoData(null).hide();
   }
   void sort() {sel.sort(CompareOrder);}   static int CompareOrder(GuiObj*C &a, GuiObj*C &b) {return a.compareLevel(*b);}
   void alignSel()
   {
      undo.set("align");
      if(sel.elms())
      {
         REPA(sel){GuiObj &obj=*sel[i]; Vec2 pos=obj.pos(); panel.align(pos, true); obj.pos(pos);}
         setChanged(true);
      }
   }
   void moveUpSel  () {undo.set("moveUpDown", true); if(sel.elms()){sort();  REPA(sel)sel[i].moveUp  (); for(int i=0; i<sel.elms()-1; i++)sel[i+1].moveAbove(*sel[i]); setChanged();}} // process from end  , but keep selection order
   void moveDownSel() {undo.set("moveUpDown", true); if(sel.elms()){sort(); FREPA(sel)sel[i].moveDown(); for(int i=0; i<sel.elms()-1; i++)sel[i+1].moveAbove(*sel[i]); setChanged();}} // process from start, but keep selection order
   void resetAspect()
   {
      undo.set("aspect");
      if(sel.elms())
      {
         REPA(sel)
         {
            GuiObj &obj=*sel[i]; Vec2 size=obj.size();
            if(obj.type()==GO_BUTTON  )            if(C ImagePtr      &image=obj.asButton  ().image)if(image->      is())size.y=size.x/image->      aspect();
            if(obj.type()==GO_COMBOBOX)            if(C ImagePtr      &image=obj.asComboBox().image)if(image->      is())size.y=size.x/image->      aspect();
            if(obj.type()==GO_CHECKBOX)if(Gui.skin)if(C PanelImagePtr &image=Gui.skin->checkbox.off)if(image->image.is())size.y=size.x/image->image.aspect();
            if(obj.type()==GO_IMAGE   )            if(C ImagePtr      &image=obj.asImage   ().image)if(image->      is())size.y=size.x/image->      aspect();
            MAX(size.x, MinSize); MAX(size.y, MinSize);
            obj.size(size);
         }
         setChanged();
      }
   }
   void processSel()
   {
      SelectionClass.SEL_MODE mode=((Kb.ctrlCmd() && Kb.shift()) ? SelectionClass.SM_KEEP : Kb.ctrlCmd() ? SelectionClass.SM_TOGGLE : Kb.shift() ? SelectionClass.SM_INCLUDE : Kb.alt() ? SelectionClass.SM_EXCLUDE : SelectionClass.SM_SET);
      if(mode==SelectionClass.SM_KEEP)
      {
         REPA(sel)if(!lit.has(sel[i]))sel.remove(i);
      }else
      {
         if(mode==SelectionClass.SM_SET)sel.clear();
         REPA(lit)switch(mode)
         {
            case SelectionClass.SM_INCLUDE:
            case SelectionClass.SM_SET    : sel.include(lit[i]); break;
            case SelectionClass.SM_TOGGLE : sel.toggle (lit[i]); break;
            case SelectionClass.SM_EXCLUDE: sel.exclude(lit[i]); break;
         }
      }
      sort();
      lit.clear();
      undo.forceCreateNextUndo();
   }
   GuiView& toGui()
   {
      if(GuiObj *obj=cur())if(InRange(obj.type(), obj_edit))obj_edit[obj.type()].toGui(obj); return T;
   }
   GuiObj* copy(GuiObj &obj, GuiObj *parent, C Vec2 *screen_pos, C Vec2 *offset)
   {
      if(parent)
         if(!obj.contains(parent))
            if(GuiObj *dest=objs.New(obj.type()))
      {
        *parent+=dest.create(obj);
         if(screen_pos){Vec2 screen=dest.pos(Vec2(0)).screenPos(), pos=*screen_pos-screen; panel.align(pos); dest.pos(pos);}
         if(offset    )dest.move(*offset);

         // copy children
         if(Tabs *src_tabs=CAST(Tabs, &obj)) // 'child' will return Tab for Tabs
         {
            if(Tabs *dest_tabs=CAST(Tabs, dest))FREP(Min(src_tabs.tabs(), dest_tabs.tabs()))
            {
               Tab & src_tab= src_tabs.tab(i),
                   &dest_tab=dest_tabs.tab(i);
               FREP(src_tab.childNum())copy(*src_tab.child(i), &dest_tab, null, null);
            }
         }else FREP(obj.childNum())copy(*obj.child(i), dest, null, null);

         return dest;
      }
      return null;
   }
   void clean() // clean all references which point to invalid files
   {
      if(elm)
      {
         undo.set("clean");
         REP(GO_NUM)if(Memx<GuiObj> *container=objs.container(GUI_OBJ_TYPE(i)))REPA(*container)Clean((*container)[i]);
         setChanged();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      lit.clear();
      if(visible() && gpc.visible)
      {
         GuiSkinPtr temp_skin=Gui.skin; if(GuiSkinPtr skin=Proj.appGuiSkin())Gui.skin=skin; // set skin because that may affect object properties (for example slidebar button visibility, window client rectangle)

         // get highlighted objects (only from containers, do not include desktop)
         if(selecting) // highlight by selection
         {
            Rect sel_rect=selRect();
            REP(GO_NUM)if(Memx<GuiObj> *container=objs.container(GUI_OBJ_TYPE(i)))REPA(*container)
            {
               GuiObj &go=(*container)[i];
               if(MainParent(go.parent(), false)==sel_parent && go.visibleFull())
                  if(Cuts(sel_rect, go.screenRect()))lit.include(&go);
            }
         }else // highlight by mouse
         if(GuiObj *obj=MainObject(Gui.msLit()))
            if(objs.contains(obj))
               if(!Ms.b(1) && !disable_selection) // don't highlight when editing or when cancelled selection
                  lit.include(obj);

         // enable rectangle selection
         if(!selecting)
         {
            if(!disable_selection)if(GuiObj *go=MainObject(Gui.ms()))if(editable(go))
               if(go.type()!=GO_SLIDER && go.type()!=GO_SLIDEBAR && go.type()!=GO_CHECKBOX && go.type()!=GO_TEXTLINE && go.type()!=GO_TEXTBOX) // disallow selection when starting with these elements because they support mouse action
            {
               if(Ms.bp(0))
               {
                  sel_pos   =Ms.pos();
                  sel_parent=FirstContainer(go, false);
               }
               if(Ms.selecting())selecting=true;
            }
         }

         // end rectangle selection
         if(selecting)
         {
            if(Ms.bp(1)){selecting=false; disable_selection=true;}else // cancel selection, 'disable_selection' so releasing later any button currently pressed will not force selection
            if(Ms.br(0))
            {
               selecting=false; disable_selection=true; processSel();
            }else 
            if(!Ms.b(0))
            {
               selecting=false;
            }
         }
         // select objs on tap
         if(!disable_selection && !selecting)
            if(!Ms.selecting() && (Ms.br(0) || Ms.bd(0)))
               if(editable(Gui.ms()))
         {
            disable_selection|=Ms.bd(0); processSel();
         }
         if(!((Ms.b(0) || Ms.br(0)) && editable(Gui.ms())))disable_selection=false; // if there are no presses and releases then re-enable selection

         op=panel.op();
         if(Kb.ctrlCmd() && Kb.shift())op=OP_SCALE   ;else
         if(                Kb.shift())op=OP_SEPARATE;else
         if(Kb.ctrlCmd()              )op=OP_SIZE    ;else
         if(Kb.alt    ()              )op=OP_PARENT  ;

         // add new object
         if(!selecting)
            if(Ms.bp(1) && op==OP_NEW
            || Kb.kf(KB_INS) && Kb.k.ctrlCmd() && hasKbFocus())
               if(GuiObj *parent=FirstContainer(Gui.ms(), false))
                  if(editable(parent))
         {
            if(parent==Gui.desktop())parent=T.parent(); // don't add to the desktop but to the Mode Gui Tab
            if(parent)
            {
               undo.set("new", true);
               if(GuiObj *obj=objs.New(panel.objType()))
               {
                  sel.clear();
                 *parent+=*obj;
                  Vec2 pos=Ms.pos()-obj.screenPos();
                  panel.align(pos);
                  obj.pos(pos);
                  sel.add(obj);
                  disable_selection=true;
                  setChanged();
                  Kb.eat(KB_INS);
               }
            }
         }

         // copy selection
         if(!selecting && sel.elms())
            if(Ms.bp(1) && op==OP_COPY
            || Kb.kf(KB_D) && Kb.k.ctrlCmd() && hasKbFocus())
               if(GuiObj *parent=FirstContainer(Gui.ms(), true)) // allow tabs so we can copy a group at once to target tab
                  if(editable(parent))
         {
            for(; selectedParents(parent); ) // don't add to selection
               parent=FirstContainer(parent.parent(), true); // allow tabs so we can copy a group at once to target tab
            if(parent==Gui.desktop())parent=T.parent(); // don't add to the desktop but to the Mode Gui Tab
            if(parent)
            {
               undo.set("copy", true);
               sort();
               temp.clear(); FREPA(sel)if(!selectedParents(sel[i].parent()))temp.add(sel[i]); // add root selected objects to temp (keep order)
               sel .clear();
               for(; temp.elms(); )
               {
                  GuiObj *temp_parent=temp.first().parent();
                  Vec2    pos        =temp.first().pos   ();
                  REPA(temp)if(temp[i].parent()==temp_parent){Vec2 p=temp[i].pos(); MIN(pos.x, p.x); MAX(pos.y, p.y);} // get top-left corner
                  for(int i=0; i<temp.elms(); )if(temp[i].parent()==temp_parent) // keep order
                  {
                     if(GuiObj *obj=copy(*temp[i], parent, &Ms.pos(), &(temp[i].pos()-pos)))sel.add(obj);
                     temp.remove(i, true); // keep order
                  }else i++;
               }
               setChanged();
               Kb.eat(KB_D);
            }
         }

         // delete
         if(!selecting && Ms.bp(1) && op==OP_DEL) // remove highlighted on RMB
         {
            undo.set("del", true);
            objs.remove(MainObject(Gui.ms()));
            disable_selection=true;
            setChanged();
         }
         if(hasKbFocus() && Kb.kf(KB_DEL) && Kb.k.ctrlCmd() && sel.elms()) // remove selected on Ctrl+Del
         {
            undo.set("del", true);
            temp=sel; REPA(temp)objs.remove(temp[i]); // copy to 'temp' container because removing changes 'sel'
            Kb.eatKey();
            setChanged();
         }

         // undo
         if(hasKbFocus() && Kb.k.ctrlCmd() && Kb.k(KB_Z)){undo.undo(); Kb.eatKey();}
         if(hasKbFocus() && Kb.k.ctrlCmd() && Kb.k(KB_Y)){undo.redo(); Kb.eatKey();}

         // edit
         if(sel.elms())
         {
            int op=T.op;
            if(op<      0)op=OP_MOVE;
            if(op==OP_NEW)op=OP_SIZE;

            if(editable(Gui.ms()) && Ms.b(1) && op==OP_MOVE) // move objects
            {
               undo.set("move");
               helper+=Ms.dc(); Vec2 d=helper; panel.alignTrunc(d); helper-=d;
               REPA(sel)if(!selectedParents(sel[i].parent()))sel[i].move(d);
               setChanged(true);
            }else
            if(editable(Gui.ms()) && Ms.b(1) && op==OP_SIZE) // resize objects
            {
               helper.x+=Ms.dc().x; helper.y-=Ms.dc().y; Vec2 d=helper; panel.alignTrunc(d); helper-=d;
               cchar8 *type="size"; if(undo.lastChangeType()==type || d.any()) // this is to prevent creating a dummy Undo level right away after creating new gui objects
               {
                  undo.set(type);
                  REPA(sel)
                  {
                     GuiObj &go=*sel[i]; Vec2 size=go.size()+d; MAX(size.x, MinSize); MAX(size.y, MinSize); go.size(size);
                  }
                  setChanged(true);
               }
            }else
            if(editable(Gui.ms()) && Ms.b(1) && op==OP_SEPARATE) // separate objects
            {
               undo.set("move");
               helper.x+=Ms.dc().x; helper.y-=Ms.dc().y; Vec2 d=helper; panel.alignTrunc(d); helper-=d;
               for(temp=sel; temp.elms(); )
               {
                  GuiObj *parent=temp.first().parent(); // get parent of first object
                  Rect    rect  =temp.first().rect  (); // get rect   of first object
                  REPA(temp)if(temp[i].parent()==parent)rect|=temp[i].rect(); // get rect covering all objects for this parent
                  Vec2 scale=1; if(rect.size().all()){scale=(rect.size()+d)/rect.size(); MAX(scale.x, 0); MAX(scale.y, 0);}
                  REPA(temp)if(temp[i].parent()==parent) // go from back because we're removing elements
                  {
                     GuiObj &obj=*temp[i]; temp.remove(i); // this object was processed so remove it
                     obj.pos((obj.pos()-rect.lu())*scale+rect.lu());
                  }
               }
               setChanged(true);
            }else
            if(editable(Gui.ms()) && Ms.b(1) && op==OP_SCALE) // scale objects
            {
               undo.set("scale");
               helper.x+=Ms.dc().x; helper.y-=Ms.dc().y; Vec2 d=helper; panel.alignTrunc(d); helper-=d;
               for(temp=sel; temp.elms(); )
               {
                  GuiObj *parent=temp.first().parent(); // get parent of first object
                  Rect    rect  =temp.first().rect  (); // get rect   of first object
                  REPA(temp)if(temp[i].parent()==parent)rect|=temp[i].rect(); // get rect covering all objects for this parent
                  Vec2 scale=1; if(rect.size().all()){scale=(rect.size()+d)/rect.size(); MAX(scale.x, 0); MAX(scale.y, 0);}
                  REPA(temp)if(temp[i].parent()==parent) // go from back because we're removing elements
                  {
                     GuiObj &obj=*temp[i]; temp.remove(i); // this object was processed so remove it
                     Vec2    size=obj.size()*scale; MAX(size.x, MinSize); MAX(size.y, MinSize);
                     obj.pos((obj.pos()-rect.lu())*scale+rect.lu()).size(size);
                  }
               }
               setChanged(true);
            }else
            if(editable(Gui.ms()) && Ms.b(1) && op==OP_PARENT) // parent objects
            {
               undo.set("move");
               IgnoreSelected=true ; GuiObj *target=Gui.objAtPos(Ms.pos()); // don't put objects into selected ones
               IgnoreSelected=false;
               target=MainParent(FirstContainer(target, true), true); if(!editable(target))target=null; if(target==Gui.desktop())target=T.parent(); // don't add to the desktop but to the Mode Gui Tab
               lit.clear(); if(target)lit.add(MainParent(target, true)); // highlight target for silky smooth user experience
               helper+=Ms.dc(); Vec2 d=helper; panel.alignTrunc(d); helper-=d;
               sort();
               FREPA(sel)if(!selectedParents(sel[i].parent())) // keep order
               {
                  sel[i].move(d);
                  if(target && sel[i].parent()!=target && !sel[i].contains(target)) // move to new parent
                  {
                     Vec2 pos=sel[i].screenPos();
                    *target+=*sel[i];
                     sel[i].move(pos-sel[i].screenPos());
                  }
               }
               setChanged(true);
            }else helper.zero();

            // keyboard edit
            if(hasKbFocus() && Kb.k.k)
            {
               if(Kb.k.ctrlCmd())
               {
                  if(Kb.k (KB_PGUP)){moveUpSel  (); Kb.eatKey();}else
                  if(Kb.k (KB_PGDN)){moveDownSel(); Kb.eatKey();}else
                  if(Kb.kf(KB_R   )){resetAspect(); Kb.eatKey();}else
                  if(Kb.kf(KB_G   )){alignSel   (); Kb.eatKey();}
               }
               Vec2 d=D.pixelToScreenSize(); if(panel.grid_align())d=panel.grid_size;
               Vec2 m(0, 0);
               if(Kb.k(KB_LEFT ))m.x-=d.x;else
               if(Kb.k(KB_RIGHT))m.x+=d.x;else
               if(Kb.k(KB_DOWN ))m.y-=d.y;else
               if(Kb.k(KB_UP   ))m.y+=d.y;
               if(op==OP_SIZE)
               {
                  if(m.any())
                  {
                     undo.set("size");
                     REPA(sel)
                     {
                        GuiObj &obj=*sel[i]; Vec2 size=obj.size(); MAX(size.x+=m.x, MinSize); MAX(size.y-=m.y, MinSize);
                        obj.size(size);
                     }
                     setChanged(true);
                  }
               }else
               {
                  if(m.any())
                  {
                     undo.set("move");
                     REPAO(sel).move(m);
                     setChanged(true);
                  }
               }
            }
         }

         // update gui
         GuiObj *cur=T.cur();
         if(last!=cur)
         {
            last=cur;
            if(cur && cur.type()==GO_TABS)createPropsTabs(&cur.asTabs());
            REPAO(obj_edit).autoData((cur && cur.type()==i) ? cur : null).visible(cur && cur.type()==i);
         }

         Gui.skin=temp_skin;
      }
   }
   static void Highlight(GuiObj &go, C Color &color, flt a)
   {
      Rect r=go.screenRect(); if(&go==Gui.desktop())r&=EditRect();
      r.drawShaded(ColorAlpha(color, 0), ColorAlpha(color, a), 0.013);
   }
   void drawPreview(GuiObj &obj, C GuiPC &gpc, GuiObj &parent)
   {
      parent+=obj;
      Vec2 screen=obj.screenPos(), pos=Ms.pos()-screen; panel.align(pos); obj.pos(pos);
      parent-=obj;
      obj.move(screen);
      obj.draw(gpc);
   }
   virtual void draw(C GuiPC &gpc)override
   {
   }
   void drawPre()
   {
      if(GuiSkinPtr temp=Proj.appGuiSkin())D.clear(temp->background_color);else D.clear(BackgroundColor());
   }
   void draw()
   {
      if(selected())
      {
         // highlight objects
         REPA(lit)Highlight(*lit[i], Color(128, 255, 255), (op==OP_PARENT || op==OP_COPY || op==OP_NEW) ? 1 : 0.6);
         REPA(sel)Highlight(*sel[i], Color(255, 255,   0),                                                    0.6);

         // draw preview of object "to-be created"
         GuiPC gpc; gpc.clip=gpc.client_rect=Gui.desktop().rect(); gpc.visible=gpc.enabled=true; gpc.offset.zero();
         if(op==OP_NEW && !Ms.b(1))if(GuiObj *parent=FirstContainer(Gui.ms(), false))if(editable(parent))
         {
            GuiSkinPtr temp=Gui.skin; if(GuiSkinPtr skin=Proj.appGuiSkin())Gui.skin=skin; // set current skin so object is previewed with correct skin
            switch(panel.objType())
            {
               case GO_BUTTON  : {Button2    obj; drawPreview(obj, gpc, *parent);} break;
               case GO_CHECKBOX: {CheckBox2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_COMBOBOX: {ComboBox2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_CUSTOM  : {GuiCustom2 obj; drawPreview(obj, gpc, *parent);} break;
             //case GO_DESKTOP : {Desktop2   obj;} break;
               case GO_IMAGE   : {GuiImage2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_LIST    : {List2      obj; drawPreview(obj, gpc, *parent);} break;
               case GO_MENU    : {Menu2      obj; drawPreview(obj, gpc, *parent);} break;
               case GO_MENU_BAR: {MenuBar2   obj; drawPreview(obj, gpc, *parent);} break;
               case GO_PROGRESS: {Progress2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_REGION  : {Region2    obj; drawPreview(obj, gpc, *parent);} break;
               case GO_SLIDEBAR: {SlideBar2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_SLIDER  : {Slider2    obj; drawPreview(obj, gpc, *parent);} break;
               case GO_TABS    : {Tabs2      obj; drawPreview(obj, gpc, *parent);} break;
               case GO_TEXT    : {Text2      obj; drawPreview(obj, gpc, *parent);} break;
               case GO_TEXTBOX : {TextBox2   obj; drawPreview(obj, gpc, *parent);} break;
               case GO_TEXTLINE: {TextLine2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_VIEWPORT: {Viewport2  obj; drawPreview(obj, gpc, *parent);} break;
               case GO_WINDOW  : {Window2    obj; drawPreview(obj, gpc, *parent);} break;
            }
            Gui.skin=temp;
            D.clip();
         }

         // draw screen proportions
         if(panel.draw_proportions())
         {
            flt  y=1,
                 a=16.0/ 9; Rect(-a, -y, a, y).draw(CYAN  , false);
                 a=16.0/10; Rect(-a, -y, a, y).draw(RED   , false);
                 a= 4.0/ 3; Rect(-a, -y, a, y).draw(GREEN , false);
                 a= 5.0/ 4; Rect(-a, -y, a, y).draw(BLUE  , false);
                 a= 1.0/ 1; Rect(-a, -y, a, y).draw(PURPLE, false);
            D.lineY(PURPLE, 0, -1, 1);
            D.lineX(PURPLE, 0, -1, 1);
         }

         // draw selection rectangle
         if(selecting)
         {
            Rect r=selRect();
            r.draw(ColorAlpha(ColorBrightness(LitColor, 0.25), 0.1), true );
            r.draw(ColorAlpha(ColorBrightness(LitColor, 0.80), 0.6), false);
         }
      }
   }

   // drag
   void drag(Memc<UID> &elms, GuiObj* &obj, C Vec2 &screen_pos)
   {
      if(elms.elms())
      {
         // check for drop onto gui object
         if(GuiObj *main=MainObject(obj))
            if(objs.contains(main))
         {
            Memx<PropEx> &props=obj_edit[main.type()].props; // list of all properties for that kind of object
            FREPA(props) // order important (set first property of elm type in case there are many)
            {
               PropEx &prop=props[i];
               if(prop.elm_type) // if this property supports drag drop
                  REPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(prop.compatible(elm.type)) // if dropping element of type that matches property type
               {
                  undo.set(null, true);
                  ptr auto_data=prop.autoData(); // remember old auto data
                  prop.autoData(main).set(elm.id.asHex()); // set auto data to target object, then set value to element ID
                  prop.autoData(auto_data); // restore old auto data
                  toGui(); // refresh gui because we've set ID in hex format (to replace with elm path)
                  obj=null; // clear in case got deleted
                  return;
               }
            }
         }
      }
   }
}
GuiView GuiEdit;
/******************************************************************************/
