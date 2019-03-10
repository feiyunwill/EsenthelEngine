/******************************************************************************/
class Data // custom class
{
   flt  f;
   int  i;
   byte b;
}
Data data[10]= // sample data array
{
   {1.0f, 15, 4},
   {2.0f, 25, 3},
   {3.0f, 35, 2},
   {4.0f, 45, 1},
};
/******************************************************************************/
Window     window; // gui window
Region     region; // gui region
List<Data> list  ; // gui list
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   ListColumn list_column[]= // gui list column (stores information about class format)
   {
      ListColumn(MEMBER(Data, f), 0.3, "Float"  ), // column describing 'f' member in 'Data' class, width of column=0.3, name="Float"
      ListColumn(MEMBER(Data, i), 0.3, "Integer"), // column describing 'i' member in 'Data' class, width of column=0.3, name="Integer"
      ListColumn(MEMBER(Data, b), 0.2, "Byte"   ), // column describing 'b' member in 'Data' class, width of column=0.2, name="Byte"
   };

   Gui   +=window.create(Rect(-0.5 , -0.4, 0.5 ,  0.4),"Window with list");
   window+=region.create(Rect( 0.05, -0.6, 0.95, -0.1));                            // create region
   region+=list  .create(list_column, Elms(list_column)).setData(data, Elms(data)); // create list with 'list_column' columns and 'data' data

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

   if(Data *cur=list())D.text(0, -0.7, S+"current element 'Integer' value: "+cur.i);
                       D.text(0, -0.8, S+"current element visible index (on the list): " +list.cur);
                       D.text(0, -0.9, S+"current element absolute index (in the data): " +list.visToAbs(list.cur));
}
/******************************************************************************/
