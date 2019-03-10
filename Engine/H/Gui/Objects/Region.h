/******************************************************************************/
const_mem_addr STRUCT(Region , GuiObj) // Gui Region !! must be stored in constant memory address !!
//{
   Bool     kb_lit     ; // if highlight when has keyboard focus, default=true
   Button   view       ; // view button
   SlideBar slidebar[2]; // 2 SlideBars (0=horizontal, 1=vertical)

   // manage
   Region& del   (              );                                 // delete
   Region& create(              );                                 // create
   Region& create(C Rect   &rect) {create().rect(rect); return T;} // create
   Region& create(C Region &src );                                 // create from 'src'

   // operations
   Region& scrollX   (Flt delta       , Bool immediate=false) {slidebar[0].scroll   (delta   , immediate); return T;} // horizontal scroll by delta
   Region& scrollToX (Flt pos         , Bool immediate=false) {slidebar[0].scrollTo (pos     , immediate); return T;} // horizontal scroll to pos
   Region& scrollFitX(Flt min, Flt max, Bool immediate=false) {slidebar[0].scrollFit(min, max, immediate); return T;} // horizontal scroll to fit min..max range

   Region& scrollY   (Flt delta       , Bool immediate=false) {slidebar[1].scroll   (delta   , immediate); return T;} // vertical scroll by delta
   Region& scrollToY (Flt pos         , Bool immediate=false) {slidebar[1].scrollTo (pos     , immediate); return T;} // vertical scroll to pos
   Region& scrollFitY(Flt min, Flt max, Bool immediate=false) {slidebar[1].scrollFit(min, max, immediate); return T;} // vertical scroll to fit min..max range
   Region& scrollEndY(                  Bool immediate=false) {slidebar[1].scrollEnd(          immediate); return T;} // vertical scroll to end

   Region& removeSlideBars(); // remove slide bars, this will completely remove the SlideBars (they can't be used unless the Region is recreated, effectively disabling any scrolling)
   Region& alwaysHideHorizontalSlideBar(Bool hide);   Bool alwaysHideHorizontalSlideBar()C; // set/get if horizontal SlideBar should be always hidden

   // set / get
   virtual Region& rect        (C Rect &rect                             ); C Rect&  rect        ()C {return super::rect()                        ;} // set/get rectangle
   virtual Region& move        (C Vec2 &delta                            );                                                                          // move by delta
           Region& slidebarSize(  Flt   size                             );   Flt    slidebarSize()C {return      _slidebar_size                  ;} // set/get slidebar size, default=0.05
                                                                              Flt    clientWidth ()C {return      _crect.w          ()            ;} //     get client   width
                                                                              Flt    clientHeight()C {return      _crect.h          ()            ;} //     get client   height
                                                                              Vec2   clientSize  ()C {return      _crect.size       ()            ;} //     get client   size
                                                                            C Rect&  clientRect  ()C {return      _crect                          ;} //     get client   rectangle
                                                                              Flt   virtualWidth ()C {return slidebar[0].lengthTotal()            ;} //     get virtual  width
                                                                              Flt   virtualHeight()C {return slidebar[1].lengthTotal()            ;} //     get virtual  height
           Region& virtualSize (C Vec2       *size                       );   Vec2  virtualSize  ()C {return Vec2(virtualWidth(), virtualHeight());} // set/get virtual  size, pass null to use 'childrenSize'
                                                                              Vec2 childrenSize  ()C;                                                //     get virtual  size needed to cover all children
           Region& skin        (C GuiSkinPtr &skin, Bool sub_objects=true); C GuiSkinPtr&    skin()C {return _skin                                ;} // set/get skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of slidebars and view button
                                                                              GuiSkin*    getSkin()C {return _skin ? _skin() : Gui.skin()         ;} //     get actual   skin

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc); // update object
   virtual void    draw  (C GuiPC &gpc); // draw   object

#if EE_PRIVATE
   void        zero();
   void    addChild(GuiObj &child);
   void removeChild(GuiObj &child);
   void  setButtons();
   void  setParams ();
   void  setParent (Bool on=true);

   enum
   {
      ALWAYS_HIDE_HORIZONTAL_SLIDEBAR=1<<0,
   };
#endif

  ~Region() {del();}
   Region();

#if !EE_PRIVATE
private:
#endif
   Byte           _flag;
   Flt            _slidebar_size;
   Rect           _crect;
   GuiSkinPtr     _skin;
   GuiObjChildren _children;

protected:
   virtual void childRectChanged(C Rect *old_rect, C Rect *new_rect, GuiObj &child);

   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;

   NO_COPY_CONSTRUCTOR(Region);
};
/******************************************************************************/
