/******************************************************************************/
class Rotator : GuiCustom
{
   Vec2 delta=0;

   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override
   {
      return Cuts(pos, Circle(rect().h()*0.5, rect().center()+gpc.offset)) ? super.test(gpc, pos, mouse_wheel) : null;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      delta=0; if(Gui.ms()==this && Ms.b(0)){delta+=Ms.d(); Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())delta+=Touches[i].ad()*3;
   }
   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         D.clip(gpc.clip);
         Circle circle(rect().h()*0.5, rect().center()+gpc.offset);
         circle.draw(Color(192, 200, 232, 64));
         circle.draw(Color(0, 0, 0, 128), false);
      }
   }
}
/******************************************************************************/
Rotator              Rot;
Edit.Viewport4.DPad  DPad;
Edit.Viewport4.DPadY DPadY;
Edit.Viewport4.Zoom  Zoom;
Button               ExitButton;
/******************************************************************************/
void InitGameButtons()
{
   if(SupportedTouches())
   {
      Gui+=ExitButton.create(Rect_LU(-D.w(), D.h(), 0.15, 0.07), "Exit").func(ExitGame).focusable(false);
      flt y=Lerp(-D.h(), D.h(), 0.4);
      Gui+=DPadY     .create(null, null).rect(Rect_L(-D.w()     , y, 0.08, 0.23));
      Gui+=DPad      .create(null, null).rect(Rect_L(-D.w()+0.09, y, 0.23, 0.23));
      Gui+=Rot       .create(          ).rect(Rect_R( D.w()-0.09, y, 0.23, 0.23));
      Gui+=Zoom      .create(null, null).rect(Rect_R( D.w()     , y, 0.08, 0.23));
   }
}
/******************************************************************************/
void ShutGameButtons()
{
   ExitButton.del();
   DPad      .del();
   DPadY     .del();
   Rot       .del();
   Zoom      .del();
}
/******************************************************************************/
void GameScreenChanged()
{
   ExitButton.pos(Vec2(-D.w(), D.h()));
   
   flt  y=Lerp(-D.h(), D.h(), 0.4);
   Vec2 d=Vec2(-D.w(), y)-DPadY.rect().left();
   DPadY.move(d);
   DPad .move(d);

   d=Vec2(D.w(), y)-Zoom.rect().right();
   Zoom.move(d);
   Rot .move(d);
}
/******************************************************************************/
