/******************************************************************************/
/******************************************************************************/
class NewWorldClass : ClosableWindow
{
   TextBlack   ts;
   TextNoTest tname, tarea_size, theightmap_res, tdensity, density;
   TextLine    name;
   ComboBox           area_size,  heightmap_res;
   Button     ok, cancel;
   UID        parent_id;

   static void OK(NewWorldClass &nw);

   int      areaSize();
   int heightmapRes ();

   void create();
   void display();
   void update(C GuiPC &gpc);

public:
   NewWorldClass();
};
/******************************************************************************/
/******************************************************************************/
extern NewWorldClass NewWorld;
/******************************************************************************/
