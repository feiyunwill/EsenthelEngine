/******************************************************************************/
Window window; // gui window
Text   text  ; // gui text
Button button; // gui button
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   Gui   +=window.create(Rect(-0.5 , -0.3 , 0.5 ,  0.2), "Window Title"); // create window and add it to Gui desktop
   window+=text  .create(Vec2( 0.5 , -0.15            ), "Text"        ); // create text   and add it to 'window', coordinates are relative to parent (its top left corner)
   window+=button.create(Rect( 0.35, -0.37, 0.65, -0.3), "OK"          ); // create button and add it to 'window', coordinates are relative to parent (its top left corner)
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

   Gui.update(); // update GUI

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(WHITE);
   Gui.draw (); // draw GUI
}
/******************************************************************************/
