/******************************************************************************/
struct GuiObjs // Set of Gui Objects
{
   // get
   GuiObj* find(C Str &name, GUI_OBJ_TYPE type); // find Gui Object of 'name' and 'type', null on fail
   GuiObj& get (C Str &name, GUI_OBJ_TYPE type); //  get Gui Object of 'name' and 'type', Exit on fail

   Button   * findButton  (C Str &name) {return ( Button   *)find(name, GO_BUTTON  );} // find Gui Object, null on fail
   CheckBox * findCheckBox(C Str &name) {return ( CheckBox *)find(name, GO_CHECKBOX);} // find Gui Object, null on fail
   ComboBox * findComboBox(C Str &name) {return ( ComboBox *)find(name, GO_COMBOBOX);} // find Gui Object, null on fail
   GuiCustom* findCustom  (C Str &name) {return ( GuiCustom*)find(name, GO_CUSTOM  );} // find Gui Object, null on fail
   Desktop  * findDesktop (C Str &name) {return ( Desktop  *)find(name, GO_DESKTOP );} // find Gui Object, null on fail
   GuiImage * findImage   (C Str &name) {return ( GuiImage *)find(name, GO_IMAGE   );} // find Gui Object, null on fail
  _List     * findList    (C Str &name) {return (_List     *)find(name, GO_LIST    );} // find Gui Object, null on fail
   Menu     * findMenu    (C Str &name) {return ( Menu     *)find(name, GO_MENU    );} // find Gui Object, null on fail
   MenuBar  * findMenuBar (C Str &name) {return ( MenuBar  *)find(name, GO_MENU_BAR);} // find Gui Object, null on fail
   Progress * findProgress(C Str &name) {return ( Progress *)find(name, GO_PROGRESS);} // find Gui Object, null on fail
   Region   * findRegion  (C Str &name) {return ( Region   *)find(name, GO_REGION  );} // find Gui Object, null on fail
   SlideBar * findSlideBar(C Str &name) {return ( SlideBar *)find(name, GO_SLIDEBAR);} // find Gui Object, null on fail
   Slider   * findSlider  (C Str &name) {return ( Slider   *)find(name, GO_SLIDER  );} // find Gui Object, null on fail
   Tabs     * findTabs    (C Str &name) {return ( Tabs     *)find(name, GO_TABS    );} // find Gui Object, null on fail
   Text     * findText    (C Str &name) {return ( Text     *)find(name, GO_TEXT    );} // find Gui Object, null on fail
   TextBox  * findTextBox (C Str &name) {return ( TextBox  *)find(name, GO_TEXTBOX );} // find Gui Object, null on fail
   TextLine * findTextLine(C Str &name) {return ( TextLine *)find(name, GO_TEXTLINE);} // find Gui Object, null on fail
   Viewport * findViewport(C Str &name) {return ( Viewport *)find(name, GO_VIEWPORT);} // find Gui Object, null on fail
   Window   * findWindow  (C Str &name) {return ( Window   *)find(name, GO_WINDOW  );} // find Gui Object, null on fail

   Button   & getButton  (C Str &name) {return ( Button   &)get(name, GO_BUTTON  );} // get Gui Object, Exit on fail
   CheckBox & getCheckBox(C Str &name) {return ( CheckBox &)get(name, GO_CHECKBOX);} // get Gui Object, Exit on fail
   ComboBox & getComboBox(C Str &name) {return ( ComboBox &)get(name, GO_COMBOBOX);} // get Gui Object, Exit on fail
   GuiCustom& getCustom  (C Str &name) {return ( GuiCustom&)get(name, GO_CUSTOM  );} // get Gui Object, Exit on fail
   Desktop  & getDesktop (C Str &name) {return ( Desktop  &)get(name, GO_DESKTOP );} // get Gui Object, Exit on fail
   GuiImage & getImage   (C Str &name) {return ( GuiImage &)get(name, GO_IMAGE   );} // get Gui Object, Exit on fail
  _List     & getList    (C Str &name) {return (_List     &)get(name, GO_LIST    );} // get Gui Object, Exit on fail
   Menu     & getMenu    (C Str &name) {return ( Menu     &)get(name, GO_MENU    );} // get Gui Object, Exit on fail
   MenuBar  & getMenuBar (C Str &name) {return ( MenuBar  &)get(name, GO_MENU_BAR);} // get Gui Object, Exit on fail
   Progress & getProgress(C Str &name) {return ( Progress &)get(name, GO_PROGRESS);} // get Gui Object, Exit on fail
   Region   & getRegion  (C Str &name) {return ( Region   &)get(name, GO_REGION  );} // get Gui Object, Exit on fail
   SlideBar & getSlideBar(C Str &name) {return ( SlideBar &)get(name, GO_SLIDEBAR);} // get Gui Object, Exit on fail
   Slider   & getSlider  (C Str &name) {return ( Slider   &)get(name, GO_SLIDER  );} // get Gui Object, Exit on fail
   Tabs     & getTabs    (C Str &name) {return ( Tabs     &)get(name, GO_TABS    );} // get Gui Object, Exit on fail
   Text     & getText    (C Str &name) {return ( Text     &)get(name, GO_TEXT    );} // get Gui Object, Exit on fail
   TextBox  & getTextBox (C Str &name) {return ( TextBox  &)get(name, GO_TEXTBOX );} // get Gui Object, Exit on fail
   TextLine & getTextLine(C Str &name) {return ( TextLine &)get(name, GO_TEXTLINE);} // get Gui Object, Exit on fail
   Viewport & getViewport(C Str &name) {return ( Viewport &)get(name, GO_VIEWPORT);} // get Gui Object, Exit on fail
   Window   & getWindow  (C Str &name) {return ( Window   &)get(name, GO_WINDOW  );} // get Gui Object, Exit on fail

 C MembConst< GuiObj   >* objects (GUI_OBJ_TYPE type)C;   // get object    container according to selected 'type', null on fail
 C MembConst< Button   >& button  ()C {return _button  ;} // get Button    container
 C MembConst< CheckBox >& checkbox()C {return _checkbox;} // get CheckBox  container
 C MembConst< ComboBox >& combobox()C {return _combobox;} // get ComboBox  container
 C MembConst< GuiCustom>& custom  ()C {return _custom  ;} // get GuiCustom container
 C MembConst< Desktop  >& desktop ()C {return _desktop ;} // get Desktop   container
 C MembConst< GuiImage >& image   ()C {return _image   ;} // get GuiImage  container
 C MembConst<_List     >& list    ()C {return _list    ;} // get List      container
 C MembConst< Menu     >& menu    ()C {return _menu    ;} // get Menu      container
 C MembConst< MenuBar  >& menubar ()C {return _menubar ;} // get MenuBar   container
 C MembConst< Progress >& progress()C {return _progress;} // get Progress  container
 C MembConst< Region   >& region  ()C {return _region  ;} // get Region    container
 C MembConst< SlideBar >& slidebar()C {return _slidebar;} // get SlideBar  container
 C MembConst< Slider   >& slider  ()C {return _slider  ;} // get Slider    container
 C MembConst< Tabs     >& tabs    ()C {return _tabs    ;} // get Tabs      container
 C MembConst< Text     >& text    ()C {return _text    ;} // get Text      container
 C MembConst< TextBox  >& textbox ()C {return _textbox ;} // get TextBox   container
 C MembConst< TextLine >& textline()C {return _textline;} // get TextLine  container
 C MembConst< Viewport >& viewport()C {return _viewport;} // get Viewport  container
 C MembConst< Window   >& window  ()C {return _window  ;} // get Window    container

   // replace base gui object classes with extended versions, if used then it should be called before 'load' method
   T1(TYPE) GuiObjs& replaceButton  () {_button  .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceCheckBox() {_checkbox.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceComboBox() {_combobox.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceCustom  () {_custom  .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceDesktop () {_desktop .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceImage   () {_image   .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceList    () {_list    .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceMenu    () {_menu    .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceMenuBar () {_menubar .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceProgress() {_progress.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceRegion  () {_region  .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceSlideBar() {_slidebar.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceSlider  () {_slider  .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceTabs    () {_tabs    .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceText    () {_text    .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceTextBox () {_textbox .replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceTextLine() {_textline.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceViewport() {_viewport.replaceClass<TYPE>(); return T;}
   T1(TYPE) GuiObjs& replaceWindow  () {_window  .replaceClass<TYPE>(); return T;}

   // operations
   GuiObjs& hide    (); // hide     all root objects
   GuiObjs& show    (); // show     all root objects
   GuiObjs& activate(); // show     all root objects and activate the first of them
   GuiObjs& fadeIn  (); // fade in  all root objects
   GuiObjs& fadeOut (); // fade out all root objects

   // manage
   T1(TYPE) struct ObjName : TYPE // Gui Object with its name
   {
      Str name;
   };

   GuiObjs& del(); // delete manually

   GuiObjs& create(C Memx< ObjName< Button   > > &button  , // create from objects
                   C Memx< ObjName< CheckBox > > &checkbox,
                   C Memx< ObjName< ComboBox > > &combobox,
                   C Memx< ObjName< GuiCustom> > &custom  ,
                   C Memx< ObjName< Desktop  > > &desktop ,
                   C Memx< ObjName< GuiImage > > &image   ,
                   C Memx< ObjName<_List     > > &list    ,
                   C Memx< ObjName< Menu     > > &menu    ,
                   C Memx< ObjName< MenuBar  > > &menubar ,
                   C Memx< ObjName< Progress > > &progress,
                   C Memx< ObjName< Region   > > &region  ,
                   C Memx< ObjName< SlideBar > > &slidebar,
                   C Memx< ObjName< Slider   > > &slider  ,
                   C Memx< ObjName< Tabs     > > &tab     ,
                   C Memx< ObjName< Text     > > &text    ,
                   C Memx< ObjName< TextBox  > > &textbox ,
                   C Memx< ObjName< TextLine > > &textline,
                   C Memx< ObjName< Viewport > > &viewport,
                   C Memx< ObjName< Window   > > &window  );

   void     copyTo(  Memx< ObjName< Button   > > &button  , // copy self to object containers
                     Memx< ObjName< CheckBox > > &checkbox,
                     Memx< ObjName< ComboBox > > &combobox,
                     Memx< ObjName< GuiCustom> > &custom  ,
                     Memx< ObjName< Desktop  > > &desktop ,
                     Memx< ObjName< GuiImage > > &image   ,
                     Memx< ObjName<_List     > > &list    ,
                     Memx< ObjName< Menu     > > &menu    ,
                     Memx< ObjName< MenuBar  > > &menubar ,
                     Memx< ObjName< Progress > > &progress,
                     Memx< ObjName< Region   > > &region  ,
                     Memx< ObjName< SlideBar > > &slidebar,
                     Memx< ObjName< Slider   > > &slider  ,
                     Memx< ObjName< Tabs     > > &tabs    ,
                     Memx< ObjName< Text     > > &text    ,
                     Memx< ObjName< TextBox  > > &textbox ,
                     Memx< ObjName< TextLine > > &textline,
                     Memx< ObjName< Viewport > > &viewport,
                     Memx< ObjName< Window   > > &window  ,
                     GuiObj                      *parent=null)C; // 'parent'=gui object to which root objects should be attached to

   // io
   void operator=(C Str &name) ; // load from file name   , Exit  on fail
   void operator=(C UID &id  ) ; // load from file name ID, Exit  on fail
   Bool load     (C Str &name) ; // load from file name   , false on fail
   Bool load     (C UID &id  ) ; // load from file name ID, false on fail
   Bool save     (C Str &name)C; // save to   file name   , false on fail

   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

#if !EE_PRIVATE
private:
#endif
   struct Obj
   {
      GUI_OBJ_TYPE type , parent_type;
      Int          index, parent_index, parent_sub, name_offset;
   };
   Mems     < Char     > _names   ;
   Mems     < Obj      > _objs    ;
   MembConst< Button   > _button  ;
   MembConst< CheckBox > _checkbox;
   MembConst< ComboBox > _combobox;
   MembConst< GuiCustom> _custom  ;
   MembConst< Desktop  > _desktop ;
   MembConst< GuiImage > _image   ;
   MembConst<_List     > _list    ;
   MembConst< Menu     > _menu    ;
   MembConst< MenuBar  > _menubar ;
   MembConst< Progress > _progress;
   MembConst< Region   > _region  ;
   MembConst< SlideBar > _slidebar;
   MembConst< Slider   > _slider  ;
   MembConst< Tabs     > _tabs    ;
   MembConst< Text     > _text    ;
   MembConst< TextBox  > _textbox ;
   MembConst< TextLine > _textline;
   MembConst< Viewport > _viewport;
   MembConst< Window   > _window  ;
#if EE_PRIVATE
   MembConst<GuiObj>* objs(GUI_OBJ_TYPE type                       );
             GuiObj * go  (GUI_OBJ_TYPE type, Int index, Int sub=-1);
#endif
};
/******************************************************************************/
