/******************************************************************************/
struct GUI // Graphical User Interface
{
   Bool allow_window_fade    , // if allow Window fading (smooth transparency), default=true
        window_fade          ; // if use fade when closing windows or displaying message boxes, default=false
   Flt  window_fade_in_speed , // speed of Window fading in     , 0..Inf, default=9
        window_fade_out_speed, // speed of Window fading out    , 0..Inf, default=6
        window_fade_scale    , // scale of Window when faded out, 0..2  , default=0.85
        dialog_padd          , // Dialog Window        padding  , 0..Inf, default=0.03
        dialog_button_height , // Dialog Window Button height   , 0..Inf, default=0.06
        dialog_button_padd   , // Dialog Window Button padding  , 0..Inf, default=dialog_button_height*2
        dialog_button_margin , // Dialog Window Button margin   , 0..Inf, default=dialog_button_height
        resize_radius        , // radius used for detection of resizing windows, 0..Inf, default=0.022
        desc_delay           ; // time after which gui object descriptions should be displayed, default=0.3
   UID  click_sound_id       ; // click sound id, default=UIDZero, if specified then it will be always played when a Button, CheckBox or a Tab is clicked

   ImagePtr image_shadow   , // shadow            image, default=ImagePtr().get("Gui/shadow.img"   )
            image_drag     , // drag              image, default=ImagePtr().get("Gui/drag.img"     )
            image_resize_x , // resize horizontal image, default=ImagePtr().get("Gui/resize_x.img" )
            image_resize_y , // resize vertical   image, default=ImagePtr().get("Gui/resize_y.img" )
            image_resize_ld, // resize left  down image, default=ImagePtr().get("Gui/resize_ru.img")
            image_resize_lu, // resize left  up   image, default=ImagePtr().get("Gui/resize_rd.img")
            image_resize_ru, // resize right up   image, default=ImagePtr().get("Gui/resize_ru.img")
            image_resize_rd; // resize right down image, default=ImagePtr().get("Gui/resize_rd.img")

   GuiSkinPtr skin        ; // active Gui Skin
   UID        default_skin; // ID of the Gui Skin to be loaded during engine initialization, this can be modified in 'InitPre'

   void (*draw_keyboard_highlight)(GuiObj *obj, C Rect &rect, C GuiSkin *skin       ); // pointer to custom function responsible for drawing keyboard highlight    , 'obj'=pointer to gui object for the highlight , 'rect'=screen rectangle of the object, 'skin'=Gui Skin of the object, default='DrawKeyboardHighlight'
   void (*draw_description       )(GuiObj *obj, C Vec2 &pos, CChar *text, Bool mouse); // pointer to custom function responsible for drawing gui object description, 'obj'=pointer to gui object of the desctiption, 'pos' =screen position where the description should be drawn, 'text'=text of the description, 'mouse'=if description drawing was triggered by mouse, default='DrawDescription'
   void (*draw_imm               )(GuiObj *obj                                      ); // pointer to custom function responsible for drawing the IMM data          , 'obj'=pointer to gui object responsible for IMM text, default='DrawIMM'

   // get
   GuiObj * kb     ()C {return _kb     ;} // current object with  keyboard    focus
   GuiObj * ms     ()C {return _ms     ;} // current object with  mouse       focus (this is not always the object under mouse cursor, for example if you push the mouse button on a gui object, and move the cursor away without releasing the button, this method will still return the object on which the button was pushed, however as soon as the object gets deactivated, this will get cleared to null)
   GuiObj * msSrc  ()C {return _ms_src ;} // current object with  mouse       focus (this is not always the object under mouse cursor, for example if you push the mouse button on a gui object, and move the cursor away without releasing the button, this method will still return the object on which the button was pushed, however as soon as the object gets deleted    , this will get cleared to null)
   GuiObj * msLit  ()C {return _ms_lit ;} // current object under mouse cursor      (this is     always the object under mouse cursor)
   GuiObj * wheel  ()C {return _wheel  ;} // current object with  mouse wheel focus
   Desktop* desktop()C {return _desktop;} // current desktop
   Window * window ()C {return _window ;} // current window with focus
   Menu   * menu   ()C {return _menu   ;} // current menu
#if EE_PRIVATE
   Window * windowLit()C {return _window_lit;} // current window under mouse cursor
#endif

   GuiObj* objAtPos(C Vec2 &pos)C; // get gui object at 'pos' screen position

   Flt updateTime()C {return _update_time;} // get CPU time needed to perform last 'update' method

   Color backgroundColor()C; // get background color of the current skin, this is 'Gui.skin.background_color'
   Color borderColor    ()C; // get border     color of the current skin, this is 'Gui.skin.border_color'

   // operations
   T1(TYPE) void operator+=(TYPE &child) {if(_desktop)*_desktop+=child;} // add    gui object to   active desktop
   T1(TYPE) void operator-=(TYPE &child) {if(_desktop)*_desktop-=child;} // remove gui object from active desktop

   void msgBox(C Str &title, C Str &text, C TextStylePtr &text_style=null); // create and display a gui based message box, 'title'=window title, 'text'=text message, 'text_style'=style of 'text'

   Dialog&  getMsgBox(CPtr id); // get      message box by its 'id', if 'id' is null then this method always creates a new message box, otherwise it first tries to find an existing message box with a matching id, if none was found then a new one is created
   Dialog* findMsgBox(CPtr id); // find     message box by its 'id', if 'id' is null then this method does nothing, returns null when not found
   void     delMsgBox(CPtr id); // delete   message box by its 'id', if 'id' is null then this method does nothing
   void fadeOutMsgBox(CPtr id); // fade out message box by its 'id', if 'id' is null then this method does nothing
   void   closeMsgBox(CPtr id); // close    message box by its 'id', if 'id' is null then this method does nothing, closing means that either 'fadeOut' or 'del' will be called depending on 'Gui.msg_box_fade'

   GUI& passwordChar(Char c); // set character used for TextLine's when in password mode (default='*')

   GUI& windowButtonsRight(Bool right);   Bool windowButtonsRight()C {return _window_buttons_right;} // set/get if Window Buttons (minimise, maximise, close) should appear on the right side of the Window, default=false for Mac and true otherwise

   void playClickSound()C; // play click sound based on 'click_sound_*' members

   // dragging
   Bool   dragging  ()C {return _dragging;} // if currently dragging an element
   Vec2   dragPos   ()C;                    // get screen position of the input that triggered dragging (this is valid if 'dragging' returns true)
   void (*dragFinish()C)(Ptr user, GuiObj *obj, C Vec2 &screen_pos) {return _drag_finish;} // get function that was specified as 'finish' parameter for 'drag' method

   void dragCancel(); // cancel any current dragging

            void drag(C Str &name                                                              , Touch *touch=null                                                           );                                                                                                         // start dragging 'name' element,                                                                          'touch'=touch used to initiate the dragging (null for mouse)
            void drag(void finish(Ptr   user, GuiObj *obj, C Vec2 &screen_pos), Ptr   user=null, Touch *touch=null, void start(Ptr   user)=null, void cancel(Ptr   user)=null);                                                                                                         // start dragging and call 'finish' function when finished, 'start' when started, 'cancel' when canceled, 'touch'=touch used to initiate the dragging (null for mouse)
   T1(TYPE) void drag(void finish(TYPE *user, GuiObj *obj, C Vec2 &screen_pos), TYPE *user     , Touch *touch=null, void start(TYPE *user)=null, void cancel(TYPE *user)=null) {drag((void(*)(Ptr, GuiObj*, C Vec2&))finish,  user, touch, (void(*)(Ptr))start, (void(*)(Ptr))cancel);} // start dragging and call 'finish' function when finished, 'start' when started, 'cancel' when canceled, 'touch'=touch used to initiate the dragging (null for mouse)
   T1(TYPE) void drag(void finish(TYPE &user, GuiObj *obj, C Vec2 &screen_pos), TYPE &user     , Touch *touch=null, void start(TYPE &user)=null, void cancel(TYPE &user)=null) {drag((void(*)(Ptr, GuiObj*, C Vec2&))finish, &user, touch, (void(*)(Ptr))start, (void(*)(Ptr))cancel);} // start dragging and call 'finish' function when finished, 'start' when started, 'cancel' when canceled, 'touch'=touch used to initiate the dragging (null for mouse)
#if EE_PRIVATE
   Bool dragWant()C {return _drag_want;} // if dragging is possible
   void dragDraw();
#endif

   // function callbacks
            void addFuncCall(void func(          )            );                                          // add custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
            void addFuncCall(void func(Ptr   user), Ptr   user);                                          // add custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
   T1(TYPE) void addFuncCall(void func(TYPE *user), TYPE *user) {addFuncCall((void(*)(Ptr))func,  user);} // add custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
   T1(TYPE) void addFuncCall(void func(TYPE &user), TYPE &user) {addFuncCall((void(*)(Ptr))func, &user);} // add custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'

   // update / draw
   void update(); // update gui
   void draw  (); // draw   gui

#if !EE_PRIVATE
private:
#endif
   Bool          _drag_want, _dragging, _window_buttons_right;
   Char          _pass_char;
   GuiObj       *_kb, *_ms, *_ms_src, *_ms_lit, *_wheel, *_desc, *_touch_desc, *_overlay_textline;
   Menu         *_menu;
   Window       *_window, *_window_lit;
   Desktop      *_desktop;
   Flt           _update_time, _time_d_fade_out;
   Dbl           _desc_time, _touch_desc_time;
   UInt          _drag_touch_id;
   Vec2          _drag_pos, _overlay_textline_offset;
   Str           _drag_name, _pass_temp;
   Ptr           _drag_user;
   void        (*_drag_start )(Ptr user),
               (*_drag_cancel)(Ptr user),
               (*_drag_finish)(Ptr user, GuiObj *obj, C Vec2 &screen_pos);
   Callbacks     _callbacks;
   SyncLock      _lock;
   Memx<Desktop> _desktops;

#if EE_PRIVATE
   void del   ();
   void create();

   void screenChanged(Flt old_width, Flt old_height);
   Bool Switch       (                             );
   void kbLit        (GuiObj *obj, C Rect &rect, C GuiSkin *skin) {if(draw_keyboard_highlight)draw_keyboard_highlight(obj, rect, skin);}
   void setText      ();

 C Str& passTemp(Int length); // Warning: this is not thread-safe

   TextLine* overlayTextLine(Vec2 &offset);

   friend struct Display;
#endif
   GUI();
}extern
   Gui; // Main GUI
/******************************************************************************/
void DrawKeyboardHighlight(GuiObj *obj, C Rect &rect, C GuiSkin *skin       ); // default Keyboard Highlight                 drawing function
void DrawDescription      (GuiObj *obj, C Vec2 &pos, CChar *text, Bool mouse); // default Gui Object Description             drawing function
void DrawIMM              (GuiObj *obj                                      ); // default IMM (Windows Input Method Manager) drawing function
/******************************************************************************/
