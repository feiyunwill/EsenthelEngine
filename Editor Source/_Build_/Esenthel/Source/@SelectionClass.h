/******************************************************************************/
/******************************************************************************/
class SelectionClass
{
   enum SEL_MODE // Selection Mode
   {
      SM_TOGGLE , // toggle  element in   selection
      SM_INCLUDE, // include element in   selection
      SM_SET    , // set     element as   selection
      SM_EXCLUDE, // exclude element from selection
      SM_KEEP   , // keep only highlighted elements in selection
      SM_NUM    ,
   };
   enum SEL_SCOPE // Selection Scope
   {
      SS_NONE     ,
      SS_HIGHLIGHT, // highlighted
      SS_SCREEN   , // visible on the screen
      SS_LOADED   , // all loaded
   };

   class TouchSelTabs : GuiCustom
   {
      int            sel , // currently selected mode
                     keep, // mode activated through tapping
                     keep_on_touch_start; // value of 'keep' at the moment of new touch start
      Mems<ImagePtr> images;
      Mems<Rect    > rects;

      // get / set
      SEL_MODE selMode   (           );
      int      operator()(           );
    C Rect&    tabRect   (int i      );
      int      tabIndex  (C Vec2 &pos);
      void image(int i, C ImagePtr &image);
      void clear();                      // clear 'keep' and store 'keep_on_touch_start' as 'sel' for the case of (button press->do quick selection->release button, doing everything so quickly that tap is triggered on release) making the tap not enable 'keep'

      // manage
      TouchSelTabs& create(int elms);
            C Rect&         rect()C;                  
      virtual TouchSelTabs& rect(C Rect&rect)override;
      virtual TouchSelTabs& move(C Vec2&delta)override;

      // update / draw
      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;

public:
   TouchSelTabs();
   };

   bool                 selecting, disable_selection, tsm_side, sel_changed;
   Vec                  sel_pos;
   Edit::Viewport4::View *sel_view;
   Memc<Obj*>           objs, highlighted;
   TouchSelTabs         tsm; // touch selection mode

   // get
   int  elms      (     )C;                  
   Obj& operator[](int i);                   
   bool has       (C UID &world_obj_instance_id)C;

   bool selRect(Rect &rect);

   // manage
   void create();
   void resize();

   // operations
   void   select      (Obj &obj);
   void deselect      (Obj &obj);
   void   selectToggle(Obj &obj);

   void   highlight(Obj &obj);
   void unhighlight(Obj &obj);

   SelectionClass& clearSelect   ();
   SelectionClass& clearHighlight();

   void highlightScreenObjs(Obj&base);
   void removed(Obj &obj);          

   void setUndo();

   // update
   void update();

   void draw(Edit::Viewport4::View &view);
   void drawSelLit();

  ~SelectionClass();

public:
   SelectionClass();
};
/******************************************************************************/
/******************************************************************************/
extern SelectionClass Selection;
/******************************************************************************/
