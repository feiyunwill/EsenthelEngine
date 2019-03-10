/******************************************************************************/
const_mem_addr STRUCT(TextLine , GuiObj) // Gui TextLine !! must be stored in constant memory address !!
//{
   Bool   kb_lit   , // if highlight when has keyboard focus           , default=true
          show_find; // if show find image when there's no text entered, default=false
   Str    hint     ; // hint displayed     when there's no text entered, default=""
   Button reset    ; // reset/clear button                             , default=created but hidden

   // manage
   TextLine& del   (                               );                                     // delete
   TextLine& create(                  C Str &text=S);                                     // create
   TextLine& create(C Rect     &rect, C Str &text=S) {create(text).rect(rect); return T;} // create
   TextLine& create(C TextLine &src                );                                     // create from 'src'

   // get / set
   Bool  password  ()C;                        TextLine& password (  Bool on                             );    // get/set password mode, in password mode text characters will be displayed as '*' (the character can be changed using 'Gui.passwordChar' method), password mode additionally prevents copying text to the clipboard
   Int   maxLength ()C {return _max_length;}   TextLine& maxLength(  Int  max_length                     );    // get/set maximum allowed text length (-1=no limit), default=-1
   Int   cursor    ()C {return _edit.cur  ;}   TextLine& cursor   (  Int  position                       );    // get/set cursor position
 C Str&  operator()()C {return _text      ;}   TextLine& set      (C Str &text, SET_MODE mode=SET_DEFAULT);    // get/set text
                                               TextLine& clear    (             SET_MODE mode=SET_DEFAULT);    // clear   text
   Flt   offset    ()C {return _offset    ;}                                                                   // get     horizontal offset currently used for displaying text
 C GuiSkinPtr& skin()C {return _skin      ;}   TextLine& skin     (C GuiSkinPtr &skin, Bool sub_objects=true); // get/set skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of 'reset' button
   GuiSkin* getSkin()C {return _skin ? _skin() : Gui.skin();}                                                  // get     actual skin

            TextLine& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextLine& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextLine& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

   virtual TextLine& rect(C Rect &rect );   C Rect& rect()C {return super::rect();} // set/get rectangle
   virtual TextLine& move(C Vec2 &delta);                                           // move by delta

   // operations
   TextLine& selectNone  (); // select no  text
   TextLine& selectAll   (); // select all text
   TextLine& selectExtNot(); // select all but extension

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void  adjustOffset() ;
   Bool     showClear()C;
   Flt    clientWidth()C;
 C Str&   displayText()C; // returns "***" when in password mode, Warning: this is not thread-safe
   void          zero() ;
   void          call() ;
   void   createReset() ;
   Bool    setChanged(C Str &text, SET_MODE mode=SET_DEFAULT);
   Bool cursorChanged(Int position);
   void  setTextInput()C;
#endif

  ~TextLine() {del();}
   TextLine();

#if !EE_PRIVATE
private:
#endif
   Bool       _can_select, _func_immediate;
   Int        _max_length;
   Flt        _offset;
   Str        _text;
   TextEdit   _edit;
   GuiSkinPtr _skin;
   Ptr        _func_user;
   void     (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;

   NO_COPY_CONSTRUCTOR(TextLine);
#if EE_PRIVATE
   friend struct ComboBox;
#endif
};
/******************************************************************************/
