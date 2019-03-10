/******************************************************************************

   Because the main methods of Gui Objects are virtual, you can extend them.

/******************************************************************************/
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
Button2 button; // create a sample button
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   Gui+=button.create(Rect_C(0, 0, 0.5, 0.1), "New Button"); // add the button to the active desktop
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
   D  .clear(WHITE);
   Gui.draw ();
}
/******************************************************************************/
