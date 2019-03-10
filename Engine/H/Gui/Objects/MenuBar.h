/******************************************************************************/
const_mem_addr STRUCT(MenuBar , GuiObj) // Gui Menu Bar !! must be stored in constant memory address !!
//{
   struct Elm // Menu Element
   {
      Str  name        , //         name (used for code commands)
           display_name; // display name
      Menu menu        ;

      Elm();
   #if EE_PRIVATE
      Elm& create(C Elm &src);
      Flt  x1()C {return x+w;}
   #endif

   #if !EE_PRIVATE
   private:
   #endif
      Bool hidden;
      Flt  x, w;
   };

   // manage
   MenuBar& del   (                     ); // delete manually
   MenuBar& create(C Node<MenuElm> &node); // create, 'node'=menu node
   MenuBar& create(C MenuBar       &src ); // create from 'src'

   // get / set
   Int  elms(     )C {return _elms.elms();} // get number of menu elements
   Elm& elm (Int i)  {return _elms[i]    ;} // get i-th      menu element
 C Elm& elm (Int i)C {return _elms[i]    ;} // get i-th      menu element

   void operator()(C Str &command, Bool on, SET_MODE mode=SET_DEFAULT) ; // set 'command' 'on' state (enabled), sample usage:       ("View/Wireframe",true)
   Bool operator()(C Str &command                                    )C; // if  'command' is on      (enabled), sample usage:       ("View/Wireframe")
   Bool exists    (C Str &command                                    )C; // if  'command' exists in menu      , sample usage: exists("File/Exit")

   MenuBar& setCommand(C Str &command, Bool visible, Bool enabled);                                            // set 'command' visibility and if not disabled
   MenuBar& setCommand(C Str &command, Bool state                ) {return setCommand(command, state, state);} // set 'command' visibility and if not disabled

   virtual MenuBar& rect(C Rect &rect);   C Rect&    rect()C {return super::rect();} // set/get rectangle
                                            Rect elmsRect()C;                        //     get rectangle covering all Menu Elements

   MenuBar& skin(C GuiSkinPtr &skin, Bool sub_objects=true); C GuiSkinPtr& skin()C {return _skin                       ;} // set/get skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of sub-menus
                                                               GuiSkin* getSkin()C {return _skin ? _skin() : Gui.skin();} //     get actual skin

   // main
   virtual void update(C GuiPC &gpc); // update object
   virtual void draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void zero   ();
   void setElms();
#endif

  ~MenuBar() {del();}
   MenuBar();

#if !EE_PRIVATE
private:
#endif
   Bool       _alt;
   Int        _lit, _push, _menu_prev;
   GuiSkinPtr _skin;
   Mems<Elm>  _elms;

protected:
   virtual void parentClientRectChanged(C Rect *old_client, C Rect *new_client);
#if EE_PRIVATE
   friend struct GUI; friend struct GuiObjChildren; friend struct GuiObj;
#endif
};
/******************************************************************************/
inline Int Elms(C MenuBar &menu) {return menu.elms();}
/******************************************************************************/
