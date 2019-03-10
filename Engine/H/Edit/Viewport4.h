/******************************************************************************/
namespace Edit{
/******************************************************************************/
const_mem_addr struct Viewport4
{
   enum VIEW_TYPE : Byte // view type
   {
      VIEW_TOP     ,
      VIEW_DIAGONAL,
      VIEW_FRONT   ,
      VIEW_LEFT    ,
      VIEW_NUM     ,
   };

   enum ORIENT_MODE : Byte // orientation mode
   {
      ORIENT_NONE    , // disabled
      ORIENT_TOP_LEFT, // visible in top left corner
      ORIENT_RIGHT   , // visible in right side
   };

   enum MOVE_MODE : Byte // movement mode
   {
      MOVE_NONE  ,
      MOVE_DPAD  ,
      MOVE_APAD  ,
      MOVE_DRAG  ,
      MOVE_ARROWS,
   };

   STRUCT(ViewportEx , Viewport)
   //{
      virtual void draw(C GuiPC &gpc);
   };

   struct View
   {
      ViewportEx viewport;
      Camera     camera  ;

      void setViewportCamera(); // manually activate the viewport and camera of this view
   }view[VIEW_NUM];

   STRUCT(DPad , GuiCustom)
   //{
      Bool       touched;
      Flt        angle;
      VecI2      axis;
      Viewport4 *v4;
      View      *view;

      DPad() {v4=null; view=null; touched=false; angle=0; axis=0;}

      DPad& create(Viewport4 *v4, View *view);

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
      virtual void    update(C GuiPC &gpc);
      virtual void    draw  (C GuiPC &gpc);
   };

   STRUCT(DPadY , GuiCustom)
   //{
      Int        dir;
      Viewport4 *v4;
      View      *view;

      DPadY() {v4=null; view=null; dir=0;}

      DPadY& create(Viewport4 *v4, View *view);

      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };

   STRUCT(Zoom , GuiCustom)
   //{
      Flt        zoom;
      Viewport4 *v4;
      View      *view;

      Zoom() {v4=null; view=null; zoom=0;}

      Zoom& create(Viewport4 *v4, View *view);

      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };

   // manage
   Viewport4& create(void (*draw)(Viewport&), Flt default_pitch=0, Flt default_yaw=0, Flt default_dist=1, Flt from=0.01f, Flt range=100); // create and use 'draw' function for viewport drawing

   // get / set
   Flt         minDist    ()C;                                                                    // get minimum camera distance
   Flt         maxDist    ()C;                                                                    // get maximum camera distance
   Bool        visible    ()C;                         Viewport4& visible    (Bool        on   ); // get/set viewport visibility
 C Rect&       rect       ()C {return _rect       ;}   Viewport4& rect       (C Rect     &rect ); // get/set viewport rectangle
   Bool        fpp        ()C {return _fpp        ;}   Viewport4& fpp        (Bool        on   ); // get/set if camera in   fpp      mode
   Bool        lock       ()C {return _lock       ;}   Viewport4& lock       (Bool        on   ); // get/set if camera in   lock     mode
   Bool        perspective()C {return _perspective;}   Viewport4& perspective(Bool        on   ); // get/set if perspective view     mode
   Bool        horizontal ()C {return _horizontal ;}   Viewport4& horizontal (Bool        on   ); // get/set if horizontal  movement mode
   Bool        keyboard   ()C {return _keyboard   ;}   Viewport4& keyboard   (Bool        on   ); // get/set if keyboard    movement enabled
   Int         sel        ()C {return _sel        ;}   Viewport4& sel        (Int         view ); // get/set selected view (VIEW_TYPE), -1=all
   Flt         fppSpeed   ()C {return _fpp_speed  ;}   Viewport4& fppSpeed   (Flt         speed); // get/set FPP camera speed, default=5
   Flt         perspFov   ()C {return _persp_fov  ;}   Viewport4& perspFov   (Flt         fov  ); // get/set perspective FOV , default=DegToRad(70)
   Bool        drawZoom   ()C {return _draw_zoom  ;}   Viewport4&   drawZoom (Bool        on   ); // get/set camera zoom        visibility
   ORIENT_MODE orientMode ()C {return _orn_mode   ;}   Viewport4& orientMode (ORIENT_MODE mode ); // get/set camera orientation mode
     MOVE_MODE   moveMode ()C {return _move_mode  ;}   Viewport4&   moveMode (  MOVE_MODE mode ); // get/set camera movement    mode
   View       *last       ()C {return _last       ;}                                              // get     view that was last active, null if none
   View       *focus      ()C {return _focus      ;}                                              // get     view with mouse focus    , null if none
   View       *getView    (GuiObj *go);                                                           // get     view from gui object     , null if none, this method tests if 'go' belongs to any of the views, if so then it returns view associated with that object
#if EE_PRIVATE
   View       *getViewCtrl(GuiObj *go);                                                           // get     view from gui object     , null if none, this method tests if 'go' belongs to any of the views, if so then it returns view associated with that object
   Flt         moveScale  (View &view, Bool time=true)C; // get keyboard movement scale for 'view'
   void        setRect    ();
   void        setCubeRect();
#endif

   // operations
   Viewport4& show             (                ) {return visible(true );}
   Viewport4& hide             (                ) {return visible(false);}
   Viewport4& resetPosOrn      (                ); // reset position and orientation
   Viewport4& axisAlign        (                ); // align the cameras to axis
   Viewport4& moveTo           (C VecD     &pos ); // move camera focus to 'pos'
   Viewport4& dist             (  Flt       dist); // set  camera distance
   Viewport4& toggleView       (  Int       view); // toggle selected    view
   Viewport4& toggleOrientMode (ORIENT_MODE mode); // toggle orientation mode
   Viewport4& toggleMoveMode   (  MOVE_MODE mode); // toggle movement    mode
   Viewport4& toggleZoom       (                ); // toggle drawing     zoom pad
   Viewport4& togglePerspective(                ); // toggle perspective view
   Viewport4& toggleHorizontal (                ); // toggle horizontal  movement
   Viewport4& toggleKeyboard   (                ); // toggle keyboard    movement
   Viewport4& toggleFpp        (                ); // toggle fpp         view
   Viewport4& toggleLock       (                ); // toggle cameras     lock
   Viewport4& setViewportCamera(                ); // manually activate the viewport and camera of last active view

   // update
   void update();

#if !EE_PRIVATE
private:
#endif
   STRUCT(Cube , GuiCustom)
   //{
      Int        part;
      MeshPtr    mesh;
      Viewport4 *v4;
      View      *view;

      Cube() {part=-1; v4=null; view=null;}

#if EE_PRIVATE
      Cube&    create(Viewport4 &v4, View &view);
      void    setView(C GuiPC &gpc, Display::ViewportSettings &view,   Camera &camera, MatrixM &matrix);
      void  resetView(            C Display::ViewportSettings &view, C Camera &camera);
#endif

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
      virtual void    update(C GuiPC &gpc);
      virtual void    draw  (C GuiPC &gpc);
   };

   STRUCT(APad , GuiCustom)
   //{
      Bool       touched;
      Vec2       start, cur;
      Viewport4 *v4;
      View      *view;

      APad() {v4=null; view=null; touched=false;}

      APad& create(Viewport4 &v4, View &view);

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
      virtual void    update(C GuiPC &gpc);
      virtual void    draw  (C GuiPC &gpc);
   };

   STRUCT(APadY , GuiCustom)
   //{
      Bool       touched;
      Flt        start, cur;
      Viewport4 *v4;
      View      *view;

      APadY() {v4=null; view=null; touched=false;}

      APadY& create(Viewport4 &v4, View &view);

      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };

   STRUCT(Drag , GuiCustom)
   //{
      Viewport4 *v4;
      View      *view;

      Drag() {v4=null; view=null;}

      Drag& create(Viewport4 &v4, View &view);

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
      virtual void    update(C GuiPC &gpc);
      virtual void    draw  (C GuiPC &gpc);
   };

   STRUCT(DragY , GuiCustom)
   //{
      Viewport4 *v4;
      View      *view;

      DragY() {v4=null; view=null;}

      DragY& create(Viewport4 &v4, View &view);

      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };

   STRUCT(Arrows , GuiCustom)
   //{
      Bool       arrow_pushed[DIR_NUM];
      Rect       arrow_rect  [DIR_NUM];
      Viewport4 *v4;
      View      *view;

      Arrows() {v4=null; view=null; Zero(arrow_pushed);}

      Arrows& create(Viewport4 &v4, View &view);

      virtual GuiObj& rect  (C Rect  &rect);
      virtual void    update(C GuiPC &gpc );
      virtual void    draw  (C GuiPC &gpc );
   };

   Bool        _perspective, _horizontal, _keyboard, _fpp, _lock, _draw_zoom;
   ORIENT_MODE _orn_mode;
   MOVE_MODE   _move_mode;
   Int         _sel;
   Flt         _fpp_speed,
               _persp_fov,
               _default_pitch,
               _default_yaw  ,
               _default_dist ,
               _rotate_time  ;
   Vec2        _rotate_yaw   ,
               _rotate_pitch ;
   Rect        _rect;
   View       *_focus, *_last, *_rotate_view;
   Cube        _cube  [VIEW_NUM];
   Zoom        _zoom  [VIEW_NUM];
   DPad        _dpad  [VIEW_NUM];
   DPadY       _dpady [VIEW_NUM];
   APad        _apad  [VIEW_NUM];
   APadY       _apady [VIEW_NUM];
   Drag        _drag  [VIEW_NUM];
   DragY       _dragy [VIEW_NUM];
   Arrows      _arrows[VIEW_NUM];
};
/******************************************************************************/
} // namespace
/******************************************************************************/
void operator+=(GuiObj &gui_obj, Edit::Viewport4 &child); // add    child
void operator-=(GuiObj &gui_obj, Edit::Viewport4 &child); // remove child
/******************************************************************************/
