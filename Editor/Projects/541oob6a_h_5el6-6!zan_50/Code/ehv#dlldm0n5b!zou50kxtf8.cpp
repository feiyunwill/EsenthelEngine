/******************************************************************************/
class MenuNew : Window // new-game menu
{
   Text   text;
   Button close;
   
   static void Close(MenuNew &menu_new) {menu_new.fadeOut();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.9, 0.5)).barVisible(false).hide();
      T  +=text .create(Vec2  (clientWidth()/2, -0.15), "New Game");
      T  +=close.create(Rect_C(clientWidth()/2, -clientHeight()/2-0.1, 0.4, 0.08), "Close").func(Close, T);
   }
}
MenuNew MN;
/******************************************************************************/
class MenuLoad : Window // load-game menu
{
   Text   text;
   Button close;
   
   static void Close(MenuLoad &menu_load) {menu_load.fadeOut();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.9, 0.5)).barVisible(false).hide();
      T  +=text .create(Vec2  (clientWidth()/2, -0.15), "Load Game");
      T  +=close.create(Rect_C(clientWidth()/2, -clientHeight()/2-0.1, 0.4, 0.08), "Close").func(Close, T);
   }
}
MenuLoad ML;
/******************************************************************************/
class MenuOptions : Window // options menu
{
   Text   text;
   Button close;
   
   static void Close(MenuOptions &menu_options) {menu_options.fadeOut();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.9, 0.5)).barVisible(false).hide();
      T  +=text .create(Vec2  (clientWidth()/2, -0.15),"Options");
      T  +=close.create(Rect_C(clientWidth()/2, -clientHeight()/2-0.1, 0.4, 0.08), "Close").func(Close, T);
   }
}
MenuOptions MO;
/******************************************************************************/
class MenuMain : Window // main menu
{
   Button b_new, b_load, b_options, b_exit;

   static void New    (ptr) {MN.fadeIn();}
   static void Load   (ptr) {ML.fadeIn();}
   static void Options(ptr) {MO.fadeIn();}
   static void Exit   (ptr) {StateExit.set();}

   void create()
   {
      Gui+=super    .create(Rect_C(0, -0.1, 0.6, 0.6)).barVisible(false);
      T  +=b_new    .create(Rect_C(clientWidth()/2, -0.15, 0.4, 0.08), "New"    ).func(New    );
      T  +=b_load   .create(Rect_C(clientWidth()/2, -0.25, 0.4, 0.08), "Load"   ).func(Load   );
      T  +=b_options.create(Rect_C(clientWidth()/2, -0.35, 0.4, 0.08), "Options").func(Options);
      T  +=b_exit   .create(Rect_C(clientWidth()/2, -0.45, 0.4, 0.08), "Exit"   ).func(Exit   );
      flag&=~WIN_MOVABLE; // disable moving of this window
      level(-1)         ; // set lower level so this window will be always below the other created in this tutorial
   }
}
MenuMain MM;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   MN.create();
   ML.create();
   MO.create();
   MM.create();
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
   D  .clear();
   Gui.draw ();
}
/******************************************************************************/
