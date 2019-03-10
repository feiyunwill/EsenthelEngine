/******************************************************************************/
enum WIN_FLAG // Window Flags
{
   WIN_MOVABLE  =0x01, // movable
   WIN_RESIZABLE=0x02, // resizable
};
/******************************************************************************/
const_mem_addr STRUCT(Window , GuiObj) // Gui Window !! must be stored in constant memory address !!
//{
   Byte      flag       , // WIN_FLAG      , default=WIN_MOVABLE
             resize_mask; // specifies which edges of the window can be resized, default=DIRF_RIGHT|DIRF_LEFT|DIRF_DOWN|DIRF_UP (used only if 'flag' has WIN_RESIZABLE enabled)
   Str       title      ; // title bar text, default=""
   Button    button[3]  ; // buttons : 0-minimize, 1-maximize, 2-close
   RippleFx *ripple     ; // ripple effect , default=null

   // manage
   Window& del   (                              );                                      // delete
   Window& create(                C Str &title=S);                                      // create
   Window& create(C Rect   &rect, C Str &title=S) {create(title).rect(rect); return T;} // create and set rectangle
   Window& create(C Window &src                 );                                      // create from 'src'

   // set / get
           Window& setTitle  (C Str        &title  );                                                                        // set     title bar text
   virtual Window& move      (C Vec2       &delta  );                                                                        //         move by delta
   virtual Window& rect      (C Rect       &rect   ); C Rect&       rect          ()C {return super::rect()               ;} // set/get rectangle
           Window& maximize  (                     );   Bool        maximized     ()C;                                       // set/get maximized state, this method will check if the Window rectangle covers the entire screen
           Window& level     (  Int         level  );   Int         level         ()C {return _level                      ;} // set/get window level (windows with level 1 will always be drawn on top of level 0 windows), default=0
           Window& skin      (C GuiSkinPtr &skin   ); C GuiSkinPtr&    skin       ()C {return _skin                       ;} // set/get skin override, default=null (if set to null then current value of 'Gui.skin' is used), changing this value will automatically change the skin of the window buttons
                                                        GuiSkin*    getSkin       ()C {return _skin ? _skin() : Gui.skin();} //     get actual skin
           Window& barVisible(  Bool        visible);   Bool        barVisible    ()C {return _bar_visible                ;} // set/get title bar visibility, default=true
                                                        Flt         barHeight     ()C;                                       //     get title bar height
                                                        Flt         highlightHover()C {return _lit_hover                  ;} //     get current hover highlight (0..1)
                                                        Flt         highlightFocus()C {return _lit_focus                  ;} //     get current focus highlight (0..1)
           Window& alpha     (  Flt         alpha  );   Flt              alpha    ()C {return _alpha                      ;} // set/get custom  window opacity controlled by the user         (0..1)
                                                        Flt          fadeAlpha    ()C {return _fade_alpha                 ;} //     get current window opacity based on the fade effect       (0..1)
                                                        Flt         finalAlpha    ()C {return _final_alpha                ;} //     get final   window opacity based on custom and fade alpha (0..1), this is "alpha()*fadeAlpha()"
                                                        Flt         clientWidth   ()C {return _crect.w   ()               ;} //     get client width
                                                        Flt         clientHeight  ()C {return _crect.h   ()               ;} //     get client height
                                                        Vec2        clientSize    ()C {return _crect.size()               ;} //     get client size
           Window& clientRect(C Rect       &rect   ); C Rect&       clientRect    ()C {return _crect                      ;} // set/get client rect

   virtual Rect sizeLimit              (               )C {return Rect(0.045f, 0.045f, Max(4, D.w2()), Max(4, D.h2()));} // allowed size limits for the Window rectangle, you can override this method and return custom values, they will be used by 'rect' method
   virtual void extendedRect           (Rect &rect     )C; // get extended rectangle (including self and all child elements extending it visually)
           void defaultInnerPadding    (Rect &padding  )C; // get default inner padding
           void defaultInnerPaddingSize(Vec2 &padd_size)C; // get default inner padding size
           Vec2 defaultInnerPaddingSize(               )C; // get default inner padding size
           Flt  defaultBarHeight       (               )C; // get default bar height
           Flt  defaultBarTextWidth    (               )C; // get default bar text width
           Flt  defaultBarFullWidth    (               )C; // get default bar full width including text and visible buttons

   // operations
           Bool    showing   (       )C; // if visible and not fading out
           Bool    hiding    (       )C; // if hidden  or      fading out
   virtual Window& fadeIn    (       ) ; // fade in  window
   virtual Window& fadeOut   (       ) ; // fade out window
   virtual Window& fadeToggle(       ) ; // toggle fading
   virtual Window& fade      (Bool in) ; // set    fading
   virtual Window& show      (       ) ; // show
   virtual Window& hide      (       ) ; // hide

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   Panel* getNormalPanel()C;
   void      focusToggle(); // this method will activate the window if it's not at the top, otherwise it will hide it
   void             zero();
   void         addChild(GuiObj &child);
   void      removeChild(GuiObj &child);
   void        setParent();
   void        setParams();
   void       setButtons();
   void          setRect();
   void    setFinalAlpha();
   Bool           active()C {return Gui.window   ()==this && App.active();}
   Bool              lit()C {return Gui.windowLit()==this;}
#endif

  ~Window() {del();}
   Window();

#if !EE_PRIVATE
private:
#endif
   Bool           _bar_visible;
   Byte           _fade_type, _resize;
   Int            _level;
   Flt            _alpha, _fade_alpha, _final_alpha, _lit_hover, _lit_focus;
   Rect           _crect;
   GuiSkinPtr     _skin;
   GuiObjChildren _children;

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;
   virtual void parentClientRectChanged(C Rect *old_client, C Rect *new_client);

   NO_COPY_CONSTRUCTOR(Window);
};
/******************************************************************************/
STRUCT(ClosableWindow , Window) // Closable Window (automatically hides on Escape or Middle Mouse Button)
//{
   virtual void update(C GuiPC &gpc);
};
/******************************************************************************/
STRUCT(ModalWindow , Window) // Modal Window (draws half transparent black color on entire desktop below the window, after clicking the background the window fades out)
//{
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
   virtual void  update(C GuiPC &gpc);
   virtual void    draw(C GuiPC &gpc);

private:
   GuiObj _background;
};
/******************************************************************************/
STRUCT(Dialog , Window) // Dialog (has text and custom amount of buttons, creating it automatically sets the correct rectangles for the window and its children)
//{
   struct Text2 : Text
   {
      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel);
   };
   Text2        text;
   Memx<Button> buttons;

   Dialog& create  (C Str &title, C Str &text, C MemPtr<Str> &buttons, C TextStylePtr &text_style=null); // create with given parameters, this automatically calls 'set'
   Dialog& set     (C Str &title, C Str &text, C MemPtr<Str> &buttons, C TextStylePtr &text_style=null); // adjust an already created Dialog with given parameters, this automatically calls 'autoSize'
   Dialog& autoSize(); // set window, text and buttons rectangles based on their values
};
/******************************************************************************/
