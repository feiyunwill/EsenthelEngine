/******************************************************************************/
enum MENU_FLAG // Menu Element Flags
{
   MENU_TOGGLABLE     =1<<0, // if togglable
   MENU_NOT_SELECTABLE=1<<1, // if can't be selected
   MENU_HIDDEN        =1<<2, // if hidden
   MENU_NO_CLOSE      =1<<3, // if selecting will not close menu
};
enum MENU_COLUMN // List columns available when creating 'Menu' from 'Node<MenuElm>'
{
   MENU_COLUMN_CHECK, // enabled/checked state
   MENU_COLUMN_NAME , // display name
   MENU_COLUMN_KBSC , // keyboard shortcut
   MENU_COLUMN_KBSC2, // keyboard shortcut #2
   MENU_COLUMN_SUB  , // sub-elements (children)
   MENU_COLUMN_NUM  , // number of menu list columns
};
/******************************************************************************/
struct MenuElm // Menu Element
{
   Str  name        , //         name (used for code commands)
        display_name; // display name
   Bool on          , // if on       (checked   ), default=false
        disabled    ; // if disabled (greyed out), default=false

            MenuElm& del   (                                                                            ); // delete
            MenuElm& create(C Str &name, void (*func)(          )=null            , Bool immediate=false); // create
            MenuElm& create(C Str &name, void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false); // create
   T1(TYPE) MenuElm& create(C Str &name, void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return create(name, (void (*)(Ptr))func,  user, immediate);} // create
   T1(TYPE) MenuElm& create(C Str &name, void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return create(name, (void (*)(Ptr))func, &user, immediate);} // create
#if EE_PRIVATE
            MenuElm& create(C MenuElm &src, Menu *parent); // create from 'src'
#endif

   MenuElm& flag   (  Byte  flag   );   Byte  flag ()C {return _flag ;} // set/get MENU_FLAG
   MenuElm& kbsc   (C KbSc &kbsc   );   KbSc  kbsc ()C {return _kbsc ;} // set/get keyboard shortcut
   MenuElm& kbsc2  (C KbSc &kbsc   );   KbSc  kbsc2()C {return _kbsc2;} // set/get keyboard shortcut (alternative)
   MenuElm& desc   (C Str  &desc   ); C Str&  desc ()C {return _desc ;} // set/get description
   MenuElm& setOn  (  Bool  on=true);                                   // set     if on
   MenuElm& display(C Str  &name   );                                   // set     display name, this method makes effect only if 'name' is not empty
                                        Menu* menu ()C {return _menu ;} //     get child menu

            MenuElm& func(void (*func)(          ),                  Bool immediate=false);                                                       // set function called when 'MenuElm' is selected, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
            MenuElm& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when 'MenuElm' is selected, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) MenuElm& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when 'MenuElm' is selected, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) MenuElm& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when 'MenuElm' is selected, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

#if EE_PRIVATE
   Bool pushable();
   void push    ();
   void zero    ();
   void call    ();
#endif

  ~MenuElm() {del();}
   MenuElm();

#if !EE_PRIVATE
private:
#endif
   Bool   _func_immediate;
   Byte   _flag;
   Ptr    _func_user;
   void (*_func )(        );
   void (*_func2)(Ptr user);
   Str    _desc;
   KbSc   _kbsc, _kbsc2;
   Menu  *_menu;

   NO_COPY_CONSTRUCTOR(MenuElm);
};
/******************************************************************************/
const_mem_addr STRUCT(Menu , GuiObj) // Gui Menu !! must be stored in constant memory address !!
//{
  _List list; // list

   // manage
   Menu& del   (                     );                                 // delete
   Menu& create(                     );                                 // create empty
   Menu& create(C Node<MenuElm> &node) {return create().setData(node);} // create from node of MenuElm's
   Menu& create(C Menu &src          );                                 // create from 'src'

   // get / set
   void operator()(C Str &command, Bool on, SET_MODE mode=SET_DEFAULT) ; // set 'command' 'on' state (checked), sample usage:       ("View/Wireframe", true)
   Bool operator()(C Str &command                                    )C; // if  'command' is on      (checked), sample usage:       ("View/Wireframe")
   Bool exists    (C Str &command                                    )C; // if  'command' exists in menu      , sample usage: exists("File/Exit")

            Menu& func(void (*func)(C Str &path, Ptr   user), Ptr   user=null);                                                    // set function called when any sub-element is pushed, with 'user' as its parameter
   T1(TYPE) Menu& func(void (*func)(C Str &path, TYPE *user), TYPE *user     ) {return T.func((void(*)(C Str&, Ptr))func,  user);} // set function called when any sub-element is pushed, with 'user' as its parameter
   T1(TYPE) Menu& func(void (*func)(C Str &path, TYPE &user), TYPE &user     ) {return T.func((void(*)(C Str&, Ptr))func, &user);} // set function called when any sub-element is pushed, with 'user' as its parameter

   Menu& setData(CChar8 *data[], Int elms, C MemPtr<Bool> &visible=null, Bool keep_cur=false); // set columns and data from text array
   Menu& setData(CChar  *data[], Int elms, C MemPtr<Bool> &visible=null, Bool keep_cur=false); // set columns and data from text array
   Menu& setData(C Node<MenuElm> &node                                                      ); // set columns and data from node of menu elements

            Menu& setColumns(ListColumn  *column, Int columns, Bool columns_hidden);                                                                                       // set list columns
   T1(TYPE) Menu& setData   (      TYPE  *data  , Int elms   , C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(data, elms, visible, keep_cur); return T;} // set data from continuous memory
   T1(TYPE) Menu& setData   ( Mems<TYPE> &mems  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(mems,       visible, keep_cur); return T;} // set data from Mems
            Menu& setData   (_Memc       &memc  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(memc,       visible, keep_cur); return T;} // set data from Memc
            Menu& setData   (_Memb       &memb  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(memb,       visible, keep_cur); return T;} // set data from Memb
            Menu& setData   (_Memx       &memx  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(memx,       visible, keep_cur); return T;} // set data from Memx
            Menu& setData   (_Meml       &meml  ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(meml,       visible, keep_cur); return T;} // set data from Meml
            Menu& setData   (_Map        &map   ,              C MemPtr<Bool> &visible=null, Bool keep_cur=false) {list.setData(map ,       visible, keep_cur); return T;} // set data from Map

   Menu& clearElmSelectable(                ); // clear per element selectable
   Menu&   setElmSelectable(Bool &selectable); // set   per element selectable, from list data elements member, for 'selectable' parameter 'MEMBER' macro usage is required

   Int      elms(     )C {return _elms.elms();} // get number of menu elements
   MenuElm& elm (Int i)  {return _elms[i]    ;} // get i-th      menu element
 C MenuElm& elm (Int i)C {return _elms[i]    ;} // get i-th      menu element

   virtual Menu& rect     (C Rect &rect ); C Rect& rect ()C {return super::rect ();} // set/get rectangle
   virtual Menu& pos      (C Vec2 &pos  );   Vec2  pos  ()C {return super::pos  ();} // set/get top    left  position
   virtual Menu& posRU    (C Vec2 &pos  );   Vec2  posRU()C {return super::posRU();} // set/get top    right position
   virtual Menu& posLD    (C Vec2 &pos  );   Vec2  posLD()C {return super::posLD();} // set/get bottom left  position
   virtual Menu& posL     (C Vec2 &pos  );   Vec2  posL ()C {return super::posL ();} // set/get left         position
   virtual Menu& posR     (C Vec2 &pos  );   Vec2  posR ()C {return super::posR ();} // set/get right        position
   virtual Menu& posD     (C Vec2 &pos  );   Vec2  posD ()C {return super::posD ();} // set/get bottom       position
   virtual Menu& posU     (C Vec2 &pos  );   Vec2  posU ()C {return super::posU ();} // set/get top          position
   virtual Menu& posC     (C Vec2 &pos  );   Vec2  posC ()C {return super::posC ();} // set/get center       position
   virtual Menu& move     (C Vec2 &delta);                                           // move by delta
   virtual Menu& moveClamp(C Vec2 &delta);                                           // move by delta and clamp to desktop area
   virtual Menu& show     (             );                                           // show

   virtual Menu& posAround(C Rect &rect, Flt align=1); // set menu position around the 'rect' screen rectangle while trying to avoid occluding it, 'align'=horizontal alignment (-1 .. 1) specifying on which side (left or right) the menu should be located

   Menu& skin(C GuiSkinPtr &skin, Bool sub_objects=true); C GuiSkinPtr& skin()C {return _skin                       ;} // set/get skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of the 'list' and sub-menus
                                                            GuiSkin* getSkin()C {return _skin ? _skin() : Gui.skin();} //     get actual skin

   Menu& setSize(Bool touch=false); // set element height and text size, 'touch'=if set for touch, making size bigger

   void checkKeyboardShortcuts(); // manually iterate all elements to check for their keyboard shortcuts and process them if pushed

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   Flt         paddingL  ()C {return _crect.min.x- _rect.min.x;}
   Flt         paddingR  ()C {return  _rect.max.x-_crect.max.x;}
   Flt         paddingT  ()C {return  _rect.max.y-_crect.max.y;}
   Flt         paddingB  ()C {return _crect.min.y- _rect.min.y;}
 C Rect&       clientRect()C {return _crect;}
   Vec2        clientSize()C {return _crect.size();}
   void        clientSize(C Vec2 &size);
   void        zero      ();
   GuiObj*     Owner     ()C {return _owner ? _owner : _parent;}
   ListColumn* listColumn();
   void        push      (C Str &elm);
   void        hideAll   ();
#endif

  ~Menu() {del();}
   Menu();

#if !EE_PRIVATE
private:
#endif
   Bool          _no_child_draw;
   Int           _selectable_offset;
   Rect          _crect;
   GuiObj       *_kb, *_owner;
   Mems<MenuElm> _elms;
   GuiSkinPtr    _skin;
   Ptr           _func_user;
   void        (*_func)(C Str &path, Ptr user);

protected:
   virtual GuiObj* owner()C;

   NO_COPY_CONSTRUCTOR(Menu);
};
/******************************************************************************/
inline Node<MenuElm>& operator+=(Node<MenuElm> &node, C Str &name) {Node<MenuElm> &child=node.New(); child.create(name); return child;}

inline Int Elms(C Menu &menu) {return Elms(menu.list);}
/******************************************************************************/
