/******************************************************************************

   Instead of creating all Gui Objects manually in the code,
   you can create them in the Gui Editor tool, and then just load them.

/******************************************************************************/
GuiObjs   gui_objs;
TextLine *name=null;
Button   *ok  =null;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void ButtonFunction(ptr) // custom function called when button pressed
{
   if(name)name.clear(); // if we have accessed the 'name' TextLine then simply clear it
}
/******************************************************************************/
bool Init()
{
   // load prepared Gui Objects in Gui Editor
   if(gui_objs.load(UID(2399284535, 1267562369, 1694492846, 1613024587)))
   {
      // if loaded succesfully 'gui_objs' now contains all the buttons, windows and other objects, stored in containers
      // first we'll add all of them to the Gui (more specifically to the active desktop) :
      Gui+=gui_objs;

      // now all objects are placed onto the desktop

      // the next step is to access the objects, accessing is done by requesting objects by name
      // each objects name has to be manually set in the Gui Editor
      name=&gui_objs.getTextLine("name"); // get the player name TextLine, which has its name set to "name" (it has been set in the Gui Editor)
      ok  =&gui_objs.getButton  ("ok"  ); // get the ok          Button  , which has its name set to "ok"   (it has been set in the Gui Editor)

      ok.func(ButtonFunction); // once we have the button we can manually assign a function for it, which will be called when the button is pressed

      return true;
   }
   return false;
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
   D  .clear();
   Gui.draw ();

   if(name)D.text(0, 0.9, (*name)()); // draw accessed TextLine value
}
/******************************************************************************/
