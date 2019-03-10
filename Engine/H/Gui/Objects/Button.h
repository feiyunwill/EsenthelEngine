/******************************************************************************/
enum BUTTON_MODE : Byte // Button Mode
{
   BUTTON_DEFAULT   , // callback function gets called when                  pointer is released on the button
   BUTTON_CONTINUOUS, // callback function gets called every frame while the pointer is enabled  on the button
   BUTTON_TOGGLE    , // callback function gets called when                  pointer is pushed   on the button which toggles its state (enabled <-> disabled)
   BUTTON_IMMEDIATE , // callback function gets called when                  pointer is pushed   on the button
};
enum BUTTON_TYPE : Byte // Button Type
{
   BUTTON_TYPE_DEFAULT,
   BUTTON_TYPE_COMBOBOX,
   BUTTON_TYPE_LIST_COLUMN,
   BUTTON_TYPE_PROPERTY_VALUE,
   BUTTON_TYPE_REGION_VIEW,
   BUTTON_TYPE_SLIDEBAR_LEFT,
   BUTTON_TYPE_SLIDEBAR_CENTER,
   BUTTON_TYPE_SLIDEBAR_RIGHT,
   BUTTON_TYPE_TAB_LEFT,
   BUTTON_TYPE_TAB_HORIZONTAL,
   BUTTON_TYPE_TAB_RIGHT,
   BUTTON_TYPE_TAB_TOP,
   BUTTON_TYPE_TAB_VERTICAL,
   BUTTON_TYPE_TAB_BOTTOM,
   BUTTON_TYPE_TAB_TOP_LEFT,
   BUTTON_TYPE_TAB_TOP_RIGHT,
   BUTTON_TYPE_TAB_BOTTOM_LEFT,
   BUTTON_TYPE_TAB_BOTTOM_RIGHT,
   BUTTON_TYPE_TEXTLINE_CLEAR,
   BUTTON_TYPE_WINDOW_MINIMIZE,
   BUTTON_TYPE_WINDOW_MAXIMIZE,
   BUTTON_TYPE_WINDOW_CLOSE,
#if EE_PRIVATE
   BUTTON_TYPE_TAB_CORNER_START=BUTTON_TYPE_TAB_TOP_LEFT,
   BUTTON_TYPE_TAB_CORNER_END  =BUTTON_TYPE_TAB_BOTTOM_RIGHT,
#endif
};
/******************************************************************************/
const_mem_addr STRUCT(Button , GuiObj) // Gui Button !! must be stored in constant memory address !!
//{
   BUTTON_MODE mode       ; // button mode     , default=BUTTON_DEFAULT
   Bool        sound      ; // play click sound, default=true, if enabled then 'Gui.playClickSound' will be called everytime this button is clicked
   Color       image_color; // image color     , default=WHITE (this is multiplied by 'GuiSkin.button.image_color')
   Flt         text_align , // text aligning   , default=0.0
               text_size  ; // text size       , default=1.0 (this is multiplied by button height and 'GuiSkin.button.text_size')
   Str         text       ; // text            , default=""
   ImagePtr    image      ; // image           , default=null
   GuiSkinPtr  skin       ; // skin override   , default=null (if set to null then current value of 'Gui.skin' is used)

   // manage
   Button& del   (                             );                                     // delete
   Button& create(                C Str &text=S);                                     // create
   Button& create(C Rect   &rect, C Str &text=S) {create(text).rect(rect); return T;} // create and set rectangle
   Button& create(C Button &src                );                                     // create from 'src'

   // get / set
           Bool    operator()(                                  )C {return _on ;} // get button state (if it's visually pushed)
           Flt     lit       (                                  )C {return _lit;} // get highlight factor (0..1)
           Button& set       (Bool on, SET_MODE mode=SET_DEFAULT);                // set button state, this method is valid only for BUTTON_TOGGLE mode
           Button& push      (                                  );                // push manually
           Button& setText   (C Str      &text                  );                // set text
           Button& setImage  (C ImagePtr &image                 );                // set image
           Button& subType   (BUTTON_TYPE type                  );   BUTTON_TYPE subType()C {return                  _sub_type;} // set/get button type, default=BUTTON_TYPE_DEFAULT
   virtual Button&  enabled  (Bool  enabled                     );   Bool        enabled()C {return          super:: enabled();} // set/get if  enabled
   virtual Button& disabled  (Bool disabled                     );   Bool       disabled()C {return          super::disabled();} // set/get if disabled
           Button& focusable (Bool on                           );   Bool      focusable()C {return                 _focusable;} // set/get if can catch keyboard focus, default=true
                                                                     GuiSkin*    getSkin()C {return skin ? skin() : Gui.skin();} //     get actual skin

   Flt        textWidth (                                C Flt *height=null)C; // calculate button text width     , 'height'=if calculate based on custom button height (if null then current button height is used)
   TextStyle* textParams(Flt &text_size, Flt &text_padd, C Flt *height=null)C; // calculate button text parameters, 'height'=if calculate based on custom button height (if null then current button height is used)

            Button& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when button state has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) Button& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when button state has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) Button& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when button state has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

            void  (*func    ()C) (Ptr user) {return _func     ;} // get                    function called when button state has changed, this returns a pointer to "void func(Ptr user)" function
            Ptr     funcUser()C             {return _func_user;} // get user parameter for function called when button state has changed

   // main
   virtual Button& hide  (            ); // hide
   virtual Button& show  (            ); // show
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void zero();
   void call(Bool sound);
   void setParams();
#endif

  ~Button() {del();}
   Button();

#if !EE_PRIVATE
private:
#endif
   Bool        _push_button, _on, _vertical, _focusable, _pixel_align, _func_immediate;
   BUTTON_TYPE _sub_type;
   Flt         _lit;
   Ptr         _func_user;
   void      (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;
#if EE_PRIVATE
   friend struct ComboBox;   friend struct _List;   friend struct Region;   friend struct SlideBar;   friend struct Tabs;   friend struct TextBox;   friend struct TextLine;   friend struct Window;
#endif
};
/******************************************************************************/
