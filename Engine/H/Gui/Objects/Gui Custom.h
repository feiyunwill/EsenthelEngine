/******************************************************************************/
const_mem_addr STRUCT(GuiCustom , GuiObj) // Gui Custom Object !! must be stored in constant memory address !!
//{
   // manage
   GuiCustom& del   (                                );                                     // delete
   GuiCustom& create(                   Ptr user=null);                                     // create
   GuiCustom& create(C Rect      &rect, Ptr user=null) {create(user).rect(rect); return T;} // create
   GuiCustom& create(C GuiCustom &src                );                                     // create from 'src'

   // set / get
   GuiCustom& focusable(Bool on);   Bool focusable()C {return _focusable;} // set/get if can catch keyboard focus, default=false

#if EE_PRIVATE
   void zero();
#endif

  ~GuiCustom() {del();}
   GuiCustom();

#if !EE_PRIVATE
private:
#endif
   Bool _focusable;

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;
};
/******************************************************************************/
