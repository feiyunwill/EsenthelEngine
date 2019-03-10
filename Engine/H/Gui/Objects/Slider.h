/******************************************************************************/
const_mem_addr STRUCT(Slider , GuiObj) // Gui Slider !! must be stored in constant memory address !!
//{
   GuiSkinPtr skin; // skin override, default=null (if set to null then current value of 'Gui.skin' is used)

   // manage
   Slider& del   (                         );                                            // delete
   Slider& create(                         );                                            // create
   Slider& create(C Rect   &rect           ) {create()           .rect(rect); return T;} // create and set initial values
   Slider& create(C Rect   &rect, Flt value) {create().set(value).rect(rect); return T;} // create and set initial values
   Slider& create(C Slider &src            );                                            // create from 'src'

   // get / set
   Flt      operator()(                                    )C {return _value;} // get value (0..1)
   Slider & set       (Flt value, SET_MODE mode=SET_DEFAULT);                  // set value (0..1)
   GuiSkin* getSkin   (                                    )C {return skin ? skin() : Gui.skin();} // get actual skin

            Slider& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=true);                                                       // set function called when value has changed, with 'user' as its parameter
   T1(TYPE) Slider& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=true) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when value has changed, with 'user' as its parameter
   T1(TYPE) Slider& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=true) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when value has changed, with 'user' as its parameter

   Slider& focusable(Bool on);   Bool focusable()C {return _focusable;} // set/get if can catch keyboard focus, default=false

   // operations
   virtual Slider& rect(C Rect &rect);   C Rect& rect()C {return super::rect();} // set/get rectangle

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void zero();
   void call();
#endif

  ~Slider() {del();}
   Slider();

#if !EE_PRIVATE
private:
#endif
   Bool   _vertical, _focusable, _func_immediate;
   Flt    _value, _lit;
   Ptr    _func_user;
   void (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;
};
/******************************************************************************/
