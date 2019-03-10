/******************************************************************************/
class Elm // list element
{
   bool      opened=true; // if element is opened
   flt       offset=0; // horizontal offset in the list
   ImagePtr  arrow; // arrow displayed for elements which have children
   Str       text; // text of the element
   Memx<Elm> children; // children of the element

   Elm& set(C Str &text) {T.text=text; return T;}
}
/******************************************************************************/
Region    region; // gui region
List<Elm> list  ; // gui list
Memx<Elm> elms  ; // elements
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void SetVisible(Memt<bool> &elms_visible, Memx<Elm> &elms, bool visible, flt offset)
{
   flt children_offset=offset+0.05; // children will have bigger offset
   FREPA(elms) // process in order (important)
   {
      Elm &elm=elms[i];
      elm.offset=offset;
      elm.arrow =(elm.children.elms() ? elm.opened ? Gui.skin->combobox.image : Gui.skin->menu.sub_menu : ImagePtr()); // set element arrow
      elms_visible.add(visible); // add visibility of this element to the list
      bool children_visible=(visible && elm.opened); // its children will be visible only if the element is visible and it's opened
      SetVisible(elms_visible, elm.children, children_visible, children_offset); // process recursively its children
   }
}
void SetListData()
{
   Memt<bool> elms_visible; // specifies which elements should be visible (array of bool's for all of the elements, including their children as well, recursively)
   SetVisible(elms_visible, elms, true, 0); // set true because root elements are visible by default, and with zero offset
   list.setDataNode(elms, elms_visible);
}
/******************************************************************************/
bool Init()
{
   // setup sample elements
   Elm &a=elms.New().set("Root");
      Elm &b=a.children.New().set("Child" );
      Elm &c=a.children.New().set("Second");
         Elm &d=c.children.New().set("Inside");

   Elm &e=elms.New().set("Other");
      Elm &f=e.children.New().set("xxx");

   // create gui objects
   ListColumn list_column[]= // gui list column (stores information about class format)
   {
      ListColumn(MEMBER(Elm, arrow), 0.05, "Arrow"), // column displaying arrow
      ListColumn(MEMBER(Elm, text ), 0.40, "Text" ), // column displaying text
   };

   Gui   +=region.create(Rect_C(0, 0, 0.5, 0.5));               // create region
   region+=list  .create(list_column, Elms(list_column), true); // create list with 'list_column' columns but hidden
   
   list.setElmOffset(MEMBER(Elm, offset)); // set offset of each element to be taken from 'offset' member
   SetListData();

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

   // toggle list element on click
   if(Ms.bp(0))
      if(Gui.ms()==&list)
         if(Elm *elm=list())
   {
      elm.opened^=1; // toggle if opened
      SetListData(); // after changing data, set the list data again
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   Gui.draw ();
}
/******************************************************************************/
