/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
GuiPC::GuiPC(Desktop &desktop)
{
   visible=(Gui.desktop()==&desktop);
   enabled=(Gui.desktop()==&desktop && desktop.enabled());
   offset.zero();
   client_rect=clip=desktop.rect();
}
/******************************************************************************/
void Desktop::zero()
{
}
Desktop::Desktop() {zero(); create();}
Desktop& Desktop::del()
{
  _children.del();
   super::del(); zero(); return T;
}
Desktop& Desktop::create()
{
   del();
 
  _type   =GO_DESKTOP;
  _visible=true;
  _rect   =D.rect();
   return T;
}
Desktop& Desktop::create(C Desktop &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type=GO_DESKTOP;
      }
   }
   return T;
}
/******************************************************************************/
void Desktop::addChild(GuiObj &child)
{
  _children.add(child, T);
}
void Desktop::removeChild(GuiObj &child)
{
  _children.remove(child);
}
/******************************************************************************/
GuiObj* Desktop::test(C Vec2 &pos, GuiObj* &mouse_wheel)
{
   GuiPC gpc(T);

   mouse_wheel=this;

   // test overlay textline
   Vec2 offset; if(Gui.desktop()==this)if(TextLine *tl=Gui.overlayTextLine(offset))
   {
      GuiPC gpc2=gpc; gpc2.offset=offset; if(GuiObj *go=tl->test(gpc2, pos, mouse_wheel))return go;
      if(Cuts(pos, Rect(-D.w(), tl->rect().min.y+offset.y, D.w(), tl->rect().max.y+offset.y)))return tl;
   }

   // test children
   if(GuiObj *go=_children.test(gpc, pos, mouse_wheel))return go;

   // test self
   if(CutsEps(pos, rect()))return this; // use EPS version to make sure that for example Mouse Position will cut the Desktop despite numerical precision issues

   return null;
}
void Desktop::update()
{
   GuiPC gpc(T); if(gpc.enabled)_children.update(gpc);
}
void Desktop::draw()
{
   if(visible())
   {
      // draw children
      GuiPC gpc(T); _children.draw(gpc);

      // draw overlay textline
      Vec2 offset; if(Gui.desktop()==this)if(TextLine *tl=Gui.overlayTextLine(offset)) // don't use "Gui._overlay_textline" because offset can change during textline's window resize
      {
         D.clip(gpc.clip); Rect(-D.w(), tl->rect().min.y+offset.y, D.w(), tl->rect().max.y+offset.y).extend(0.01f).drawShaded(Color(0, 0, 0, 128), TRANSPARENT, 0.01f);
         GuiPC gpc2=gpc; gpc2.offset=offset; tl->draw(gpc2);
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
