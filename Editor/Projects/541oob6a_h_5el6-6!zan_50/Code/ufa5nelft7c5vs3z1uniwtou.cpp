/******************************************************************************/
// Define extended version of default 'Button'
class Button2 : Button // Create a new class extending the default Gui Button
{
   virtual void update(C GuiPC &gpc) // extend updating object
   {
      super.update(gpc); // call default method
   }
   virtual void draw(C GuiPC &gpc) // extend drawing object
   {
      if(gpc.visible && visible()) // if parents are visible and this object is visible too
      {
         D.clip(gpc.clip); // clip display drawing to given clipping rectangle, this is needed for example when object is inside a window, and is partially hidden because the parent's rectangle doesn't fully cover this object

         Rect screen_rect=rect()+gpc.offset; // calculate screen rectangle (this is object's rectangle repositioned by parents offset)

         Color color=(T() ? BLACK : Lerp(ORANGE, RED, lit())); // set different color depending if the button is pushed and highlighted
         screen_rect.draw(color);

         D.text(screen_rect.center(), text); // draw button's text, at center of rectangle
      }
   }
}
/******************************************************************************/
GuiObjs gui_objs;
Button2 button;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // replace default 'Button' class with extended 'Button2'
   gui_objs.replaceButton<Button2>();

   // load the gui objects
   if(gui_objs.load(UID(2399284535, 1267562369, 1694492846, 1613024587)))
   {
      Gui+=gui_objs;
      return true;
   }
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Gui.update();
   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(BLACK);
   Gui.draw ();
}
/******************************************************************************/
