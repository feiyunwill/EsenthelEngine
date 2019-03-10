/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Viewport::zero()
{
   rect_color.zero();
   draw_func=null;
   fov_mode=FOV_Y;
   fov=0;
   from=0;
   range=0;
}
void Viewport::reset()
{
   fov_mode=D.viewFovMode();
   fov     =D.viewFov    ();
   from    =D.viewFrom   ();
   range   =D.viewRange  ();
}
Viewport::Viewport() {zero();}
Viewport& Viewport::del()
{
   super::del(); zero(); return T;
}
Viewport& Viewport::create(void (*draw)(Viewport&), Ptr user)
{
   del();

  _type      =GO_VIEWPORT;
  _visible   =true;
 T.user      =user;
 T.draw_func =draw;
  _rect.max.x= 0.3f;
  _rect.min.y=-0.3f;
   rect_color=Gui.borderColor();

   reset();
   return T;
}
Viewport& Viewport::create(C Viewport &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type      =GO_VIEWPORT;
         rect_color=src.rect_color;
         fov_mode  =src.fov_mode;
         fov       =src.fov;
         from      =src.from;
         range     =src.range;
         draw_func =src.draw_func;
      }
   }
   return T;
}
/******************************************************************************/
void Viewport::setDisplayView()C
{
   D.view(screenRect(), from, range, fov, fov_mode);
}
/******************************************************************************/
GuiObj* Viewport::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      mouse_wheel=this;
      return go;
   }
   return null;
}
void Viewport::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      Rect r=rect()+gpc.offset;
      if(draw_func && (r&Rect(gpc.clip).extend(-D.pixelToScreenSize().x, -D.pixelToScreenSize().y)).valid())
      {
         Display::ViewportSettings temp_view; temp_view.get();
         Camera                    temp_cam=ActiveCam;
         D.view(r, from, range, fov, fov_mode).clip(gpc.clip); draw_func(T);
         temp_view.set(); temp_cam.set();
      }
      if(rect_color.a)
      {
         D.clip(gpc.clip);
         r.draw(rect_color, false);
      }
   }
}
/******************************************************************************/
Bool Viewport::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(1); // version
      f<<rect_color;
      return f.ok();
   }
   return false;
}
Bool Viewport::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 1:
      {
        _type=GO_VIEWPORT;
         f>>rect_color;
         if(f.ok()){reset(); return true;}
      }break;

      case 0:
      {
        _type=GO_VIEWPORT;
         f>>rect_color; Swap(rect_color.r, rect_color.b);
         if(f.ok()){reset(); return true;}
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
