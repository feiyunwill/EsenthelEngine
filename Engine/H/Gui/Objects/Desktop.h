/******************************************************************************/
const_mem_addr STRUCT(Desktop , GuiObj) // Gui Desktop !! must be stored in constant memory address !!
//{
   // manage
   Desktop& del   (              ); // manually delete
   Desktop& create(              ); // create
   Desktop& create(C Desktop &src); // create from 'src'

   // main
   GuiObj* test  (C Vec2 &pos, GuiObj* &mouse_wheel); // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   void    update(); // update object
   void    draw  (); // draw   object

#if EE_PRIVATE
   void        zero();
   void    addChild(GuiObj &child);
   void removeChild(GuiObj &child);
#endif

  ~Desktop() {del();}
   Desktop();

#if !EE_PRIVATE
private:
#endif
   GuiObjChildren _children;
#if EE_PRIVATE
   friend struct GUI;
#endif
};
/******************************************************************************/
