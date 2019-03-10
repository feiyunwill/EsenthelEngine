/******************************************************************************/
const_mem_addr STRUCT(ColorPicker , Window) // !! must be stored in constant memory address !!
//{
   MemberDesc md;

   // manage
   ColorPicker& create(C Str &name);

   // get / set
 C Vec4       & operator()(                                        )C {return _rgba;} // get current color in   RGBA format
   ColorPicker& set       (C Vec4 &color, SET_MODE mode=SET_DEFAULT);                 // set current color from RGBA values
   ColorPicker& setRGB    (C Vec  &rgb  , SET_MODE mode=SET_DEFAULT);                 // set current color from RGB  values
   ColorPicker& setHSB    (C Vec  &hsb  , SET_MODE mode=SET_DEFAULT);                 // set current color from HSB  values
   ColorPicker& setAlpha  (  Flt   alpha, SET_MODE mode=SET_DEFAULT);                 // set current alpha

            ColorPicker& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when color value has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ColorPicker& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when color value has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ColorPicker& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when color value has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

   // operations
   ColorPicker& show(         );
   ColorPicker& mode(Bool real); // change value display mode ('real'=0..1 range, or byte=0..255 range)

   virtual void update(C GuiPC &gpc);

   ColorPicker();

#if !EE_PRIVATE
private:
#endif
   STRUCT(SatLum , GuiCustom)
   //{
      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };
   STRUCT(Hue , GuiCustom)
   //{
      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };
   STRUCT(Colors , GuiCustom)
   //{
      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };
   Bool           _real, _func_immediate;
   Vec            _hsb;
   Vec4           _rgba, _old;
   Memx<Property> _props;
   SatLum         _sat_lum;
   Hue            _hue;
   Colors         _color;
   Text           _tnew, _told;
   Button         _mode;
   Ptr            _func_user;
   void         (*_func)(Ptr user);

#if EE_PRIVATE
   void  call    ();
   void  toGui   (Bool rgb=true, Bool alpha=true, Bool hsb=true, Bool rgba=true);
   void _set     (C Vec4 &color, SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setRGB  (C Vec  &rgb  , SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setHSB  (C Vec  &hsb  , SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setAlpha(  Flt   alpha, SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void  setOld  ();

   static void SetTextStyle();
#endif
};
/******************************************************************************/
