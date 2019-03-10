/******************************************************************************/
Window  window; // gui window
MenuBar menu  ; // gui menu bar
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
// here are defined functions which are going to be called by menu commands when activated
void MenuFileNew () {}
void MenuFileOpen() {}
void MenuFileSave() {}
void MenuFileExit() {StateExit.set();}
/******************************************************************************/

/******************************************************************************/
Bool Init()
{
   Gui+=window.create(Rect(-0.5, -0.4, 0.5, 0.4),"Window with menu");
   {
      Node<MenuElm> n; // let's start with main node
      {
         Node<MenuElm> &f=(n+="File"); // add "File" element to main node, and store it's reference in 'f'
         f.New().create("New" , MenuFileNew ).kbsc(KbSc('n', KBSC_CTRL)); // add "New"  command to 'f', giving it's procedure and keyboard shortcut (ctrl+n)
         f.New().create("Open", MenuFileOpen).kbsc(KbSc('o', KBSC_CTRL)); // add "Open" command to 'f', giving it's procedure and keyboard shortcut (ctrl+o)
         f.New().create("Save", MenuFileSave).kbsc(KbSc('s', KBSC_CTRL)); // add "Save" command to 'f', giving it's procedure and keyboard shortcut (ctrl+s)
         f++; // add empty line
         f.New().create("Exit", MenuFileExit).kbsc(KbSc('x', KBSC_CTRL)); // add "Exit" command to 'f', giving it's procedure and keyboard shortcut (ctrl+x)
      }
      {
         Node<MenuElm> &e=(n+="Edit");
         e.New().create("Option").flag(MENU_TOGGLABLE); // giving 'MENU_TOGGLABLE' flag means that we can toggle this element
         {
            Node<MenuElm> &c=(e+="Children");
            c.New().create("child 1");
            c.New().create("child 2").desc("This is child 2");      // this element has a description which will be shown when mouse cursor stops on the element
            c.New().create("child 3").flag(MENU_TOGGLABLE).setOn(); // this element can be toggled and is checked by default
         }
      }
      window+=menu.create(n); // create menu from nodes and add it to window
   }
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
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
   D  .text (0, -0.7, S+"'Option' is "+menu("edit/option")); // draw state of "option" located in "edit" using operator() method
}
/******************************************************************************/
