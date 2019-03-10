/******************************************************************************/
const_mem_addr STRUCT(TextBox , GuiObj) // Gui TextBox !! must be stored in constant memory address !!
//{
   Bool     kb_lit     ; // if highlight when has keyboard focus       , default=true
   Str      hint       ; // hint displayed when there's no text entered, default=""
   Button   view       ; // view button
   SlideBar slidebar[2]; // 2 SlideBars (0=horizontal, 1=vertical)

   // manage
   TextBox& del   (                              ); // delete
   TextBox& create(                 C Str &text=S);                                     // create
   TextBox& create(C Rect    &rect, C Str &text=S) {create(text).rect(rect); return T;} // create
   TextBox& create(C TextBox &src                );                                     // create from 'src'

   // get / set
   Int   maxLength ()C {return _max_length   ;}   TextBox& maxLength   (  Int  max_length                        ); // get/set maximum allowed text length (-1=no limit), default=-1
   Int   cursor    ()C {return _edit.cur     ;}   TextBox& cursor      (  Int  position                          ); // get/set cursor position
   Bool  wordWrap  ()C {return _word_wrap    ;}   TextBox& wordWrap    (  Bool wrap                              ); // get/set word wrapping, default=true
 C Str&  operator()()C {return _text         ;}   TextBox& set         (C Str &text, SET_MODE mode=SET_DEFAULT   ); // get/set text
                                                  TextBox& clear       (             SET_MODE mode=SET_DEFAULT   ); // clear   text
   Flt slidebarSize()C {return _slidebar_size;}   TextBox& slidebarSize(  Flt  size                              ); // set/get slidebar size, default=0.05
 C GuiSkinPtr& skin()C {return _skin         ;}   TextBox& skin        (C GuiSkinPtr &skin, Bool sub_objects=true); // get/set skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of slidebars and view button
   GuiSkin* getSkin()C {return _skin ? _skin() : Gui.skin();}                                                       // get     actual skin

            TextBox& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextBox& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextBox& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

   virtual TextBox& rect(C Rect &rect );   C Rect& rect()C {return super::rect();} // set/get rectangle
   virtual TextBox& move(C Vec2 &delta);                                           // move by delta

   // operations
   TextBox& selectNone(); // select no  text
   TextBox& selectAll (); // select all text

   TextBox& scrollX   (Flt delta       , Bool immediate=false) {slidebar[0].scroll   (delta   , immediate); return T;} // horizontal scroll by delta
   TextBox& scrollToX (Flt pos         , Bool immediate=false) {slidebar[0].scrollTo (pos     , immediate); return T;} // horizontal scroll to pos
   TextBox& scrollFitX(Flt min, Flt max, Bool immediate=false) {slidebar[0].scrollFit(min, max, immediate); return T;} // horizontal scroll to fit min..max range

   TextBox& scrollY   (Flt delta       , Bool immediate=false) {slidebar[1].scroll   (delta   , immediate); return T;} // vertical scroll by delta
   TextBox& scrollToY (Flt pos         , Bool immediate=false) {slidebar[1].scrollTo (pos     , immediate); return T;} // vertical scroll to pos
   TextBox& scrollFitY(Flt min, Flt max, Bool immediate=false) {slidebar[1].scrollFit(min, max, immediate); return T;} // vertical scroll to fit min..max range

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void           zero();
   void           call();
   void     setButtons();
   void      setParent();
   void      setParams();
   Bool     setChanged(C Str &text, SET_MODE mode=SET_DEFAULT);
   Bool  cursorChanged(Int position);
   void     moveCursor(Int lines, Int pages);
   void setVirtualSize();
   void setTextInput  ()C;

   Flt    clientWidth ()C {return      _crect.w          ()            ;} // get client  width
   Flt    clientHeight()C {return      _crect.h          ()            ;} // get client  height
   Vec2   clientSize  ()C {return      _crect.size       ()            ;} // get client  size
 C Rect&  clientRect  ()C {return      _crect                          ;} // get client  rectangle
   Flt   virtualWidth ()C {return slidebar[0].lengthTotal()            ;} // get virtual width
   Flt   virtualHeight()C {return slidebar[1].lengthTotal()            ;} // get virtual height
   Vec2  virtualSize  ()C {return Vec2(virtualWidth(), virtualHeight());} // get virtual size
#endif

  ~TextBox() {del();}
   TextBox();

#if !EE_PRIVATE
private:
#endif
   Bool       _word_wrap, _can_select, _func_immediate;
   Int        _max_length;
   Flt        _slidebar_size;
   Str        _text;
   TextEdit   _edit;
   Ptr        _func_user;
   void     (*_func)(Ptr user);
   Rect       _crect;
   GuiSkinPtr _skin;

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;

   NO_COPY_CONSTRUCTOR(TextBox);
};
/******************************************************************************/
