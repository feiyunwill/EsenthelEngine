/******************************************************************************/
Window   window; // gui window
Tabs     tabs  ; // gui tabs
Text     text  ; // gui text
Button   button; // gui button
GuiImage image ; // gui image
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   CChar *tab[]=
   {
      "First",
      "Second",
      "Third",
   };

   Gui   +=window.create(Rect   (-0.5, -0.4, 0.5, 0.4), "Window with tabs");
   window+=tabs  .create(Rect_LU(0, 0, window.clientWidth(), 0.08f), 0, tab, Elms(tab)); // create tabs

   // add some elements to tabs
   {
      tabs.tab(0)+=text  .create(Vec2  (window.clientWidth()/2, -window.clientHeight()/2          ), "Text"                                          ); // create and add text   to 0th tab
      tabs.tab(1)+=button.create(Rect_C(window.clientWidth()/2, -window.clientHeight()/2, 0.4, 0.1), "Button"                                        ); // create and add button to 1st tab
      tabs.tab(2)+=image .create(Rect_C(window.clientWidth()/2, -window.clientHeight()/2, 0.6, 0.3), UID(1119600675, 1212460399, 80010661, 526665178)); // create and add image  to 2nd tab
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
   D  .clear(TURQ);
   Gui.draw ();
}
/******************************************************************************/
