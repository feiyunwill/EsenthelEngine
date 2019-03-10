/******************************************************************************/
enum COMBOBOX_FLAG // Combobox Flag
{
   COMBOBOX_MOUSE_WHEEL=0x1, // if mouse wheel can change selection
   COMBOBOX_CONST_TEXT =0x2, // if don't change text value
   COMBOBOX_SET_CLAMP  =0x4, // if calling 'set' with a value bigger than allowed, then clamp it to last value
};
/******************************************************************************/
const_mem_addr STRUCT(ComboBox , Button) // Gui ComboBox !! must be stored in constant memory address !!
//{
   Byte flag      ; // COMBOBOX_FLAG, default=COMBOBOX_MOUSE_WHEEL
   Flt  menu_align; // menu horizontal alignment (-1 .. 1), default=1, this value specifies on which side (left or right) the menu should be activated
   Menu menu      ; // menu

   // manage
   ComboBox& del   (                                      );                                                  // delete
   ComboBox& create(                                      );                                                  // create empty
   ComboBox& create(C Rect &rect                          ) {return create().                    rect(rect);} // create empty and set rectangle
   ComboBox& create(              CChar8 *data[], Int elms) {return create().setData(data, elms)           ;} // create from text array
   ComboBox& create(C Rect &rect, CChar8 *data[], Int elms) {return create().setData(data, elms).rect(rect);} // create from text array and set rectangle
   ComboBox& create(              CChar  *data[], Int elms) {return create().setData(data, elms)           ;} // create from text array
   ComboBox& create(C Rect &rect, CChar  *data[], Int elms) {return create().setData(data, elms).rect(rect);} // create from text array and set rectangle
   ComboBox& create(              Node<MenuElm>      &node) {return create().setData(node      )           ;} // create from node of menu elements
   ComboBox& create(C Rect &rect, Node<MenuElm>      &node) {return create().setData(node      ).rect(rect);} // create from node of menu elements and set rectangle
   ComboBox& create(C ComboBox &src                       );                                                  // create from 'src'

   // data
            ComboBox& clear(); // clear data

            ComboBox& setData(CChar8 *data[], Int elms, C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData(data, elms, visible, keep_cur); return T;} // set columns and data from text array
            ComboBox& setData(CChar  *data[], Int elms, C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData(data, elms, visible, keep_cur); return T;} // set columns and data from text array
            ComboBox& setData(C Node<MenuElm> &node                                                      ) {menu.setData(node                         ); return T;} // set columns and data from node of menu elements

            ComboBox& setColumns(ListColumn  *column, Int columns, Bool columns_hidden=true                         ) {menu.setColumns(column, columns, columns_hidden); return T;} // set list columns
   T1(TYPE) ComboBox& setData   (      TYPE  *data  , Int elms   , C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (data, elms, visible, keep_cur  ); return T;} // set data from continuous memory
   T1(TYPE) ComboBox& setData   ( Mems<TYPE> &mems  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (mems,       visible, keep_cur  ); return T;} // set data from Mems
            ComboBox& setData   (_Memc       &memc  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (memc,       visible, keep_cur  ); return T;} // set data from Memc
            ComboBox& setData   (_Memb       &memb  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (memb,       visible, keep_cur  ); return T;} // set data from Memb
            ComboBox& setData   (_Memx       &memx  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (memx,       visible, keep_cur  ); return T;} // set data from Memx
            ComboBox& setData   (_Meml       &meml  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (meml,       visible, keep_cur  ); return T;} // set data from Meml
            ComboBox& setData   (_Map        &map   ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {menu.setData   (map ,       visible, keep_cur  ); return T;} // set data from Map

   // set / get
                                                                                       Int       visSel()C {return menu.list.absToVis(_abs_sel);} //     get active visible  selection
                                                                                       Int       absSel()C {return                    _abs_sel ;} //     get active absolute selection
           ComboBox&   set    (  Int   abs_sel         , SET_MODE mode=SET_DEFAULT);   Int   operator()()C {return                    _abs_sel ;} // set/get active absolute selection, -1=none 
           ComboBox&   setText(C Str  &text, Bool force, SET_MODE mode=SET_DEFAULT);                                                              // set     active          selection from text, if 'text' not found : -1 is set and text is set depending on 'force' (false: set(""), true: set(text))
           ComboBox& resetText(                                                   );                                                              // reset   text from active selection
#if EE_PRIVATE
           ComboBox&   setPath(C Str &start, C Str &end                           );
#endif
   virtual ComboBox& rect     (C Rect &rect                                       ); C Rect&       rect()C {return super::rect();} // set/get rectangle
           ComboBox& skin     (C GuiSkinPtr &skin, Bool sub_objects=true          ); C GuiSkinPtr& skin()C {return super::skin  ;} // set/get skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of sub-menus

            ComboBox& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when selection has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ComboBox& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when selection has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ComboBox& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when selection has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object

#if EE_PRIVATE
   void zero     ();
   void call     ();
   void setMenu  ();
   void setParams();
#endif

  ~ComboBox() {del();}
   ComboBox();

#if !EE_PRIVATE
private:
#endif
   Bool   _func_immediate;
   Int    _abs_sel;
   Ptr    _func_user;
   void (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;
};
/******************************************************************************/
inline Int Elms(C ComboBox &combobox) {return Elms(combobox.menu);}
/******************************************************************************/
