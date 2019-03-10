/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
Rotator              Rot;
Edit::Viewport4::DPad  DPad;
Edit::Viewport4::DPadY DPadY;
Edit::Viewport4::Zoom  Zoom;
Button               ExitButton;
/******************************************************************************/
void InitGameButtons()
{
   if(SupportedTouches())
   {
      Gui+=ExitButton.create(Rect_LU(-D.w(), D.h(), 0.15f, 0.07f), "Exit").func(ExitGame).focusable(false);
      flt y=Lerp(-D.h(), D.h(), 0.4f);
      Gui+=DPadY     .create(null, null).rect(Rect_L(-D.w()     , y, 0.08f, 0.23f));
      Gui+=DPad      .create(null, null).rect(Rect_L(-D.w()+0.09f, y, 0.23f, 0.23f));
      Gui+=Rot       .create(          ).rect(Rect_R( D.w()-0.09f, y, 0.23f, 0.23f));
      Gui+=Zoom      .create(null, null).rect(Rect_R( D.w()     , y, 0.08f, 0.23f));
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
   
   flt  y=Lerp(-D.h(), D.h(), 0.4f);
   Vec2 d=Vec2(-D.w(), y)-DPadY.rect().left();
   DPadY.move(d);
   DPad .move(d);

   d=Vec2(D.w(), y)-Zoom.rect().right();
   Zoom.move(d);
   Rot .move(d);
}
/******************************************************************************/

/******************************************************************************/
   GuiObj* Rotator::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
      return Cuts(pos, Circle(rect().h()*0.5f, rect().center()+gpc.offset)) ? ::EE::GuiObj::test(gpc, pos, mouse_wheel) : null;
   }
   void Rotator::update(C GuiPC &gpc)
{
      ::EE::GuiObj::update(gpc);
      delta=0; if(Gui.ms()==this && Ms.b(0)){delta+=Ms.d(); Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())delta+=Touches[i].ad()*3;
   }
   void Rotator::draw(C GuiPC &gpc)
{
      if(visible() && gpc.visible)
      {
         D.clip(gpc.clip);
         Circle circle(rect().h()*0.5f, rect().center()+gpc.offset);
         circle.draw(Color(192, 200, 232, 64));
         circle.draw(Color(0, 0, 0, 128), false);
      }
   }
Rotator::Rotator() : delta(0) {}

/******************************************************************************/
