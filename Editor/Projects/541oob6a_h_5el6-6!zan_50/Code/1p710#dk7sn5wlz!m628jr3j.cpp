/******************************************************************************/
ImagePtr logo;
GuiImage image[4];
flt      screen_changed_time;
/******************************************************************************/
void ScreenChanged(flt old_width, flt old_height)
{
   screen_changed_time=1;
}
void InitPre()
{
   EE_INIT();
   App.flag=APP_RESIZABLE;
   App.x=0;
   App.y=0;
   D.screen_changed=ScreenChanged; // set callback function to be called when app screen size was changed
}
/******************************************************************************/
bool Init()
{
   // import logo image
   logo=UID(1119600675, 1212460399, 80010661, 526665178);

   // add gui image to 4 screen corners
   Gui+=image[0].create(Rect_LU(-D.w(),  D.h(), 0.6, 0.3), logo);
   Gui+=image[1].create(Rect_RU( D.w(),  D.h(), 0.6, 0.3), logo);
   Gui+=image[2].create(Rect_RD( D.w(), -D.h(), 0.6, 0.3), logo);
   Gui+=image[3].create(Rect_LD(-D.w(), -D.h(), 0.6, 0.3), logo);

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

   screen_changed_time-=Time.d();

   if(Ms.bp(0))D.mode(D.resW()-32, D.resH()); // decrease screen size on LMB
   if(Ms.bp(1))D.mode(D.resW()+32, D.resH()); // increase screen size on RMB

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   Gui.draw ();

   // draw a black cross on the screen center
   D.lineX(BLACK, 0, -0.1, 0.1);
   D.lineY(BLACK, 0, -0.1, 0.1);

   // draw "screen changed" info
   TextStyleParams ts; ts.color=ColorAlpha(ts.color, screen_changed_time);
   D.text(ts, 0, 0.15, "Screen Changed");

   // draw informations
   D.text(0,  0.9, S+"Press LMB/RMB to change screen size");
   D.text(0,  0.8, S+"Mouse Position: "+Ms.pos());
   D.text(0, -0.4, S+"You can see that horizontal range of the screen changes (currently "+D.w()+')');
   D.text(0, -0.5, S+"While vertical range of the screen remains constant (currently "    +D.h()+')');
   D.text(0, -0.6, S+"Gui Objects on the desktop are set relatively to the desktop center.");
}
/******************************************************************************/
