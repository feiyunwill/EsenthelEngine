/******************************************************************************/
enum GUI_OBJ_TYPE : Byte // Gui Object Type
{
   GO_NONE    = 0, // none
   GO_BUTTON  = 1, // Button
   GO_CHECKBOX= 2, // CheckBox
   GO_COMBOBOX= 3, // ComboBox
   GO_CUSTOM  = 5, // Custom
   GO_DESKTOP = 6, // Desktop
   GO_IMAGE   = 7, // Image
   GO_LIST    = 8, // List
   GO_MENU    = 4, // Menu
   GO_MENU_BAR= 9, // MenuBar
   GO_PROGRESS=10, // ProgressBar
   GO_REGION  =11, // Region
   GO_SLIDEBAR=12, // SlideBar
   GO_SLIDER  =13, // Slider
   GO_TAB     =20, // Tab
   GO_TABS    =14, // Tabs
   GO_TEXT    =15, // Text
   GO_TEXTBOX =16, // TextBox
   GO_TEXTLINE=17, // TextLine
   GO_VIEWPORT=18, // Viewport
   GO_WINDOW  =19, // Window
   GO_NUM     =21, // number of Gui Object Types
};
enum GUI_BASE_LEVEL
{
   GBL_DEFAULT,
   GBL_WINDOW ,
   GBL_MENU   ,
};
enum SET_MODE
{
   SET_DEFAULT, // default  set mode, callback is     called, sound is     played
   QUIET      , // quiet    set mode, callback is NOT called, sound is NOT played
   NO_SOUND   , // no sound set mode, callback is     called, sound is NOT played
};
/******************************************************************************/
struct GuiPC // Gui Parent->Child Relation
{
   Bool visible    , // if parent is visible
        enabled    ; // if parent is enabled
   Vec2 offset     ; //        screen offset    which should be applied when drawing
   Rect clip       , //      clipping rectangle which should be used for D.clip method before any drawing
        client_rect; // parent client rectangle in screen space

   Bool hidden  ()C {return !visible;} // if parent is hidden
   Bool disabled()C {return !enabled;} // if parent is disabled

   GuiPC() {}
   GuiPC(              Desktop &desktop);
   GuiPC(C GuiPC &old, Region  &region );
   GuiPC(C GuiPC &old, Window  &window );
#if EE_PRIVATE
   GuiPC(C GuiPC &old,  Bool    visible, Bool enabled);
   GuiPC(C GuiPC &old,  Menu   &menu   );
   GuiPC(C GuiPC &old, _List   &list   );
#endif
};
/******************************************************************************/
const_mem_addr struct GuiObj // Gui Object interface inherited by all Gui Object classes (Button, CheckBox, Window, ..) !! must be stored in constant memory address !!
{
   Ptr user; // user data pointer

   // manage
   virtual GuiObj& del   (); // manually delete
           GuiObj& create(C GuiObj &src); // create from 'src', this method will succeed only if objects are of the same type

   // children
   void    operator+=(GuiObj  &child   ); // add           child
   void    operator-=(GuiObj  &child   ); // remove        child
   void    operator+=(GuiObjs &children); // add           children
   void    operator-=(GuiObjs &children); // remove        children
   GuiObj* child     (Int i            ); // get i-th      child
   Int     childNum  (                 ); // get number of children

   // operations
   Bool    moveUp      (             )C; // move object up          in parent's children hierarchy, returns true if object was moved and false it not
   Bool    moveDown    (             )C; // move object down        in parent's children hierarchy, returns true if object was moved and false it not
   GuiObj& moveToTop   (             ) ; // move object to top      in parent's children hierarchy
   GuiObj& moveToBottom(             ) ; // move object to bottom   in parent's children hierarchy
   GuiObj& moveAbove   (C GuiObj &obj) ; // move object above 'obj' in parent's children hierarchy, this call will be ignored if objects have different parents
   GuiObj& moveBelow   (C GuiObj &obj) ; // move object below 'obj' in parent's children hierarchy, this call will be ignored if objects have different parents
#if EE_PRIVATE
   GuiObj&              windowsToTop() ; // move all parent windows to top (including self)
   GuiObj&             validateLevel() ; // place    object in correct order in the parents child list
   Bool partiallyOccludedInSameLevel()C; // if       object is at least partially occluded by any other object in the same hierarchy level
   Bool                  enabledFull()C; // if       object and all of its parents are  enabled
   Bool                 disabledFull()C; // if       object or  any of its parents are disabled
#endif

           GuiObj& kbSet                (       ); //   set   keyborard focus
           GuiObj& kbClear              (       ); //   clear keyborard focus
   virtual GuiObj& hide                 (       ); //   hide     object
   virtual GuiObj& show                 (       ); //   show     object
   virtual GuiObj&   activate           (       ); //   activate object (show & set focus), this does not activate any hidden parents of the object
   virtual GuiObj& deactivate           (       ); // deactivate object (    remove focus)
           GuiObj& visible              (Bool on) {if(on      )                 show(); else hide(); return T;} // set    visibility
           GuiObj& visibleActivate      (Bool on) {if(on      ){if(hidden())activate();}else hide(); return T;} // set    visibility and activate if was hidden
           GuiObj& visibleToggle        (       ) {if(hidden())                 show(); else hide(); return T;} // toggle visibility
           GuiObj& visibleToggleActivate(       ) {if(hidden())             activate(); else hide(); return T;} // toggle visibility and activate if was hidden
   virtual GuiObj& setText              (       );                                                              // set text, this is called automatically when application language changes due to 'App.lang', you can override this method and set new text for this object (or its children) based on new 'App.lang', don't forget to call 'super.setText'

   // set / get
                                                 Bool         is         ()C {return  _type!=GO_NONE;} //     get if created
                                                 GUI_OBJ_TYPE type       ()C {return  _type         ;} //     get object type
                                                 CChar *      typeName   ()C;                          //     get object type name
                                                 GuiObj*      parent     ()C {return  _parent       ;} //     get object parent
                                                 Bool         visibleFull()C;                          //     get if visible and all parents are also visible
                                                 Bool         visible    ()C {return  _visible      ;} //     get if visible
           GuiObj& hidden  (  Bool  hidden  );   Bool         hidden     ()C {return !_visible      ;} // set/get if hidden
   virtual GuiObj&  enabled(  Bool   enabled);   Bool          enabled   ()C {return !_disabled     ;} // set/get if  enabled
   virtual GuiObj& disabled(  Bool  disabled);   Bool         disabled   ()C {return  _disabled     ;} // set/get if disabled
   virtual GuiObj& desc    (C Str  &desc    ); C Str&         desc       ()C {return  _desc         ;} // set/get description
   virtual GuiObj& rect    (C Rect &rect    ); C Rect&        rect       ()C {return  _rect         ;} // set/get rectangle
   virtual GuiObj& size    (C Vec2 &size    );   Vec2         size       ()C {return  _rect.size  ();} // set/get size
   virtual GuiObj& pos     (C Vec2 &pos     );   Vec2         pos        ()C {return  _rect.lu    ();} // set/get top    left  position
   virtual GuiObj& posRU   (C Vec2 &pos     );   Vec2         posRU      ()C {return  _rect.ru    ();} // set/get top    right position
   virtual GuiObj& posLD   (C Vec2 &pos     );   Vec2         posLD      ()C {return  _rect.ld    ();} // set/get bottom left  position
   virtual GuiObj& posL    (C Vec2 &pos     );   Vec2         posL       ()C {return  _rect.left  ();} // set/get left         position
   virtual GuiObj& posR    (C Vec2 &pos     );   Vec2         posR       ()C {return  _rect.right ();} // set/get right        position
   virtual GuiObj& posD    (C Vec2 &pos     );   Vec2         posD       ()C {return  _rect.down  ();} // set/get bottom       position
   virtual GuiObj& posU    (C Vec2 &pos     );   Vec2         posU       ()C {return  _rect.up    ();} // set/get top          position
   virtual GuiObj& posC    (C Vec2 &pos     );   Vec2         posC       ()C {return  _rect.center();} // set/get center       position
                                                 Vec2         screenPos  ()C;                          //     get absolute     position  on the screen
                                                 Rect         screenRect ()C;                          //     get absolute     rectangle on the screen
   virtual GuiObj& move    (C Vec2 &delta   );                                                         //         move   by delta
           GuiObj& resize  (C Vec2 &delta   );                                                         //         resize by delta
           GuiObj& baseLevel( Int   level   );   Int          baseLevel  ()C {return _base_level    ;} // set/get base level

   // helpers
   Bool contains    (C GuiObj *child)C; // if object contains 'child'
   Int  compareLevel(C GuiObj &obj  )C; // compare level between this object and 'obj' in parent's children hierarchy, <0 value is returned if this object is below 'obj', 0 value is returned if they share the same level or have different parents, >0 value is returned if this object is above 'obj'
   Bool above       (C GuiObj &obj  )C {return compareLevel(obj)>0;} // if this object is above 'obj'
   Bool below       (C GuiObj &obj  )C {return compareLevel(obj)<0;} // if this object is below 'obj'

   // convert
   Button   & asButton  () {return ( Button   &)T;}   C Button   & asButton  ()C {return ( Button   &)T;} // return as Button   (you may use this only if type()==GO_BUTTON  )
   CheckBox & asCheckBox() {return ( CheckBox &)T;}   C CheckBox & asCheckBox()C {return ( CheckBox &)T;} // return as CheckBox (you may use this only if type()==GO_CHECKBOX)
   ComboBox & asComboBox() {return ( ComboBox &)T;}   C ComboBox & asComboBox()C {return ( ComboBox &)T;} // return as ComboBox (you may use this only if type()==GO_COMBOBOX)
   GuiCustom& asCustom  () {return ( GuiCustom&)T;}   C GuiCustom& asCustom  ()C {return ( GuiCustom&)T;} // return as Custom   (you may use this only if type()==GO_CUSTOM  )
   Desktop  & asDesktop () {return ( Desktop  &)T;}   C Desktop  & asDesktop ()C {return ( Desktop  &)T;} // return as Desktop  (you may use this only if type()==GO_DESKTOP )
   GuiImage & asImage   () {return ( GuiImage &)T;}   C GuiImage & asImage   ()C {return ( GuiImage &)T;} // return as Image    (you may use this only if type()==GO_IMAGE   )
  _List     & asList    () {return (_List     &)T;}   C _List    & asList    ()C {return (_List     &)T;} // return as List     (you may use this only if type()==GO_LIST    )
   Menu     & asMenu    () {return ( Menu     &)T;}   C Menu     & asMenu    ()C {return ( Menu     &)T;} // return as Menu     (you may use this only if type()==GO_MENU    )
   MenuBar  & asMenuBar () {return ( MenuBar  &)T;}   C MenuBar  & asMenuBar ()C {return ( MenuBar  &)T;} // return as MenuBar  (you may use this only if type()==GO_MENU_BAR)
   Progress & asProgress() {return ( Progress &)T;}   C Progress & asProgress()C {return ( Progress &)T;} // return as Progress (you may use this only if type()==GO_PROGRESS)
   Region   & asRegion  () {return ( Region   &)T;}   C Region   & asRegion  ()C {return ( Region   &)T;} // return as Region   (you may use this only if type()==GO_REGION  )
   SlideBar & asSlideBar() {return ( SlideBar &)T;}   C SlideBar & asSlideBar()C {return ( SlideBar &)T;} // return as SlideBar (you may use this only if type()==GO_SLIDEBAR)
   Slider   & asSlider  () {return ( Slider   &)T;}   C Slider   & asSlider  ()C {return ( Slider   &)T;} // return as Slider   (you may use this only if type()==GO_SLIDER  )
   Tab      & asTab     () {return ( Tab      &)T;}   C Tab      & asTab     ()C {return ( Tab      &)T;} // return as Tab      (you may use this only if type()==GO_TAB     )
   Tabs     & asTabs    () {return ( Tabs     &)T;}   C Tabs     & asTabs    ()C {return ( Tabs     &)T;} // return as Tabs     (you may use this only if type()==GO_TABS    )
   Text     & asText    () {return ( Text     &)T;}   C Text     & asText    ()C {return ( Text     &)T;} // return as Text     (you may use this only if type()==GO_TEXT    )
   TextBox  & asTextBox () {return ( TextBox  &)T;}   C TextBox  & asTextBox ()C {return ( TextBox  &)T;} // return as TextBox  (you may use this only if type()==GO_TEXTBOX )
   TextLine & asTextLine() {return ( TextLine &)T;}   C TextLine & asTextLine()C {return ( TextLine &)T;} // return as TextLine (you may use this only if type()==GO_TEXTLINE)
   Viewport & asViewport() {return ( Viewport &)T;}   C Viewport & asViewport()C {return ( Viewport &)T;} // return as Viewport (you may use this only if type()==GO_VIEWPORT)
   Window   & asWindow  () {return ( Window   &)T;}   C Window   & asWindow  ()C {return ( Window   &)T;} // return as Window   (you may use this only if type()==GO_WINDOW  )

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc) {} // update object
   virtual void    draw  (C GuiPC &gpc) {} // draw   object

   // IO
   virtual Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   virtual Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

#if EE_PRIVATE
   void zero();

   void copyParams(C GuiObj &src); // copy common parameters

   Bool    is                   (GUI_OBJ_TYPE type)C; // if  object is of 'type' type, this method is "null safe"
   GuiObj* last                 (GUI_OBJ_TYPE type) ; // get last  parent of     'type'
   GuiObj* first                (GUI_OBJ_TYPE type) ; // get first parent of     'type'
   GuiObj* firstNon             (GUI_OBJ_TYPE type) ; // get first parent of non 'type'
   GuiObj* firstContainer       (                 ) ; // get first parent which is a container
   GuiObj* firstKbParent        (                 ) ; // get first parent which is a kb storage
   Region* firstScrollableRegion(                 ) ; // get first parent which is a scrollable region
   Int     parents              (                 )C; // get how many parents this object belongs to

   Vec2       clientOffset()C; // get client offset (from position to client position)
   Vec2       clientSize  ()C; // get client size
   Rect  localClientRect  ()C; // get client rectangle in local  space - this is with having Vec2(0, 0) as top left corner, except Desktop which has Vec2(-D.w(), D.h())
   Vec2 screenClientPos   ()C; // get client position  in screen space
   Rect screenClientRect  ()C; // get client rectangle in screen space

   Bool kbCatch()C; // if object can catch keyboard focus

   GuiObjChildren* children(); // get children container (null if none)
   void      notifyChildrenOfClientRectChange(C Rect *old_client, C Rect *new_client);
   void      notifyParentOfRectChange        (C Rect &old_rect  ,   Bool  old_visible);
#endif

   virtual ~GuiObj() {del();} // set virtual destructor so 'Delete' can be used together with extended classes
            GuiObj();

#if !EE_PRIVATE
private:
#endif
   Bool         _visible, _disabled, _updated;
   SByte        _base_level;
   GUI_OBJ_TYPE _type;
   GuiObj      *_parent;
   Rect         _rect;
   Str          _desc;

   virtual void parentClientRectChanged(C Rect *old_client, C Rect *new_client               ) {} // called when client rectangle of the parent has changed, 'old_client'=old client rectangle (can be null if old parent is null), 'new_client'=new client rectangle (can be null if new parent is null)
   virtual void        childRectChanged(C Rect *old_rect  , C Rect *new_rect  , GuiObj &child) {} // called when        rectangle of a child    has changed

   virtual GuiObj* owner()C {return _parent;}

   NO_COPY_CONSTRUCTOR(GuiObj);
};
/******************************************************************************/
struct GuiObjChildren
{
   struct Child
   {
      GuiObj *go;
   };
   Bool        changed;
   GuiObj     *kb;
   Memc<Child> children;

   void del();

#if EE_PRIVATE
   GuiObj* operator[](Int i)  {return children[i].go;}
 C GuiObj* operator[](Int i)C {return children[i].go;}

   Bool                 find        (C GuiObj &child, Int &index)C;
   Bool                 find        (C GuiObj &child_a, C GuiObj &child_b, Int &index_a, Int &index_b)C;
   Bool                 remove      (  GuiObj &child);
   Child*               add         (  GuiObj &child, GuiObj &parent);
   void                validateLevel(C GuiObj &child);
   Int                  compareLevel(C GuiObj &child_a, C GuiObj &child_b)C;
   Bool partiallyOccludedInSameLevel(C GuiObj &child);
   Bool                 moveUp      (C GuiObj &child);
   Bool                 moveDown    (C GuiObj &child);
   void                 moveToTop   (C GuiObj &child);
   void                 moveToBottom(C GuiObj &child);
   void                 moveAbove   (C GuiObj &child_a, C GuiObj &child_b);
   void                 moveBelow   (C GuiObj &child_a, C GuiObj &child_b);
   Bool                 Switch      (C GuiObj &go     , Bool next=true);

   GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
   void    update(C GuiPC &gpc);
   void    draw  (C GuiPC &gpc);
#endif

  ~GuiObjChildren() {del();}
   GuiObjChildren() {changed=false; kb=null;}

   NO_COPY_CONSTRUCTOR(GuiObjChildren);
};
inline Int Elms(C GuiObjChildren &children) {return children.children.elms();}
/******************************************************************************/
CChar* GuiObjTypeName(GUI_OBJ_TYPE type); // get Gui Object Type Name from 'type' GUI_OBJ_TYPE
#if EE_PRIVATE
inline Flt GuiMaxX(C Rect &rect) {return  rect.max.x;} // max horizontal extents
inline Flt GuiMaxY(C Rect &rect) {return -rect.min.y;} // max vertical   extents
#endif
/******************************************************************************/
