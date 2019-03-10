/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MenuBar::Elm::Elm()
{
   hidden=0;
   x=0;
   w=0;
}
MenuBar::Elm& MenuBar::Elm::create(C Elm &src)
{
           name=src.        name;
   display_name=src.display_name;
         hidden=src.      hidden;
              x=src.           x;
              w=src.           w;
   menu.create(src.menu);
   return T;
}
/******************************************************************************/
void MenuBar::zero()
{
  _alt=false;
  _lit=_push=_menu_prev=-1;
}
MenuBar::MenuBar() {zero();}
MenuBar& MenuBar::del()
{
  _elms.del  ();
  _skin.clear();
   super::del(); zero(); return T;
}
MenuBar& MenuBar::create(C Node<MenuElm> &node)
{
   del();

  _type   =GO_MENU_BAR;
  _visible=true;

  _elms.setNum(node.children.elms());
   FREP(elms())
   {
    C Node<MenuElm> &src=node.children[i];
      Elm &elm=T.elm(i);
           elm.name        =src.name;
           elm.display_name=src.display_name;
           elm.menu.create(src)._owner=this;
      Gui+=elm.menu; // add after setting '_owner' because it may affect level in children array
   }
   return T;
}
MenuBar& MenuBar::create(C MenuBar &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type     =GO_MENU_BAR;
        _skin     =src._skin;
        _alt      =src._alt;
        _lit      =src._lit;
        _push     =src._push;
        _menu_prev=src._menu_prev;
        _elms.setNum(src.elms()); FREP(elms())
         {
            elm(i).create(src.elm(i));
            elm(i).menu._owner=this;
         }
      }
   }
   return T;
}
/******************************************************************************/
void MenuBar::setElms()
{
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->menubar.text_style())
   {
      Flt x=0, h=rect().h(), padd=skin->menubar.text_padd*h;
      TextStyleParams ts=*text_style; ts.size=skin->menubar.text_size*h;
      FREP(elms())
      {
         Flt w=ts.textWidth(elm(i).display_name)+padd;
         elm(i).x=x;
         elm(i).w=w;
         if(!elm(i).hidden)x+=w;
      }
   }
}
MenuBar& MenuBar::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      super::rect(rect);
      setElms();
   }
   return T;
}
Rect MenuBar::elmsRect()C
{
   Rect   r=rect(); r.max.x=r.min.x; if(elms())r.max.x+=_elms.last().x1();
   return r;
}
MenuBar& MenuBar::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   T._skin=skin;
   if(sub_objects)REPAO(_elms).menu.skin(skin, true);
   return T;
}
void MenuBar::parentClientRectChanged(C Rect *old_client, C Rect *new_client)
{
   if(new_client)
      if(GuiSkin *skin=getSkin())
         rect(Rect_LU(new_client->min.x, new_client->max.y, new_client->w(), skin->menubar.bar_height));
}
/******************************************************************************/
void MenuBar::operator()(C Str &command, Bool on, SET_MODE mode)
{
   CChar *start=_GetStart(command);
   if( Is(start))REP(elms())if(Equal(elm(i).name, start)){elm(i).menu(_GetStartNot(command), on, mode); break;}
}
Bool MenuBar::operator()(C Str &command)C
{
   CChar *start=_GetStart(command);
   if( Is(start))REP(elms())if(Equal(elm(i).name, start))return elm(i).menu(_GetStartNot(command));
   return false;
}
Bool MenuBar::exists(C Str &command)C
{
   CChar *start=_GetStart(command);
   if( Is(start))REP(elms())if(Equal(elm(i).name, start))
   {
            start=_GetStartNot(command);
      if(Is(start))return elm(i).menu.exists(start);
                   return true;
   }
   return false;
}
/******************************************************************************/
MenuBar& MenuBar::setCommand(C Str &command, Bool visible, Bool enabled)
{
   CChar *start=_GetStart(command);
   if( Is(start))REP(elms())if(Equal(elm(i).name, start))
   {
             start=_GetStartNot(command);
      if(!Is(start))
      {
         if(enabled==elm(i).menu.disabled())elm(i).menu.disabled(!elm(i).menu.disabled());
         if(visible==elm(i).hidden)
         {
               elm(i).hidden^=1;
            if(elm(i).menu.contains(Gui.menu()))activate();
            setElms();
         }
      }
      break;
   }
   return T;
}
/******************************************************************************/
static void Activate(MenuBar &menu, MenuBar::Elm &elm, C GuiPC &gpc)
{
   Vec2 pos(menu.rect().min.x+elm.x+gpc.offset.x-elm.menu.paddingL(), menu.rect().min.y+gpc.offset.y);
   Rect screen_rect(0, -D.h(), 0, D.h());
   Flt  h_below=(Rect_LU(0, pos.y                , 0, elm.menu.rect().h())&screen_rect).h(), // visible height when below the MenuBar
        h_above=(Rect_LD(0, pos.y+menu.rect().h(), 0, elm.menu.rect().h())&screen_rect).h(); // visible height when above the MenuBar
   elm.menu.pos((h_above>h_below+EPS) ? pos+Vec2(0, menu.rect().h()+elm.menu.rect().h()) : pos).activate(); // select position according to which visible height is bigger, use EPS to more often place the Menu below
}
void MenuBar::update(C GuiPC &gpc)
{
   if(enabled() && gpc.enabled)
   {
      // activate
      if((Kb.bp(KB_LALT) || Kb.bp(KB_MENU)) // if menu key pressed this frame
      && visible() && gpc.visible // this MenuBar is visible
      && first(GO_WINDOW)==Gui.window() // it belongs to the focused window (this also works OK if there's no focused window and this MenuBar doesn't belong to any window)
      )_alt=true; // mark that alt is being pressed
      if(_alt) // if alt is being pressed
      {
         if(Kb.k.c
         ||(Kb.k.k && Kb.k.k!=KB_ALT && Kb.k.k!=KB_LALT && Kb.k.k!=KB_RALT && Kb.k.k!=KB_MENU)
         || Kb.ctrl() || Kb.shift() || Kb.win()
         || Kb.b(KB_FIND)
         || Ms._button[0] || Ms._button[1] || Ms._button[2]
         || !App.active() // if lost focus due to Alt-tab
         )_alt=false;else // if any action happened, then cancel activating
         if(Kb.br(KB_LALT) || Kb.br(KB_MENU)) // if menu key released
         {
           _alt=false; // finished
            if(_menu_prev>=0)deactivate();else // if already activated, then deactivate
            FREP(elms())if(!elm(i).hidden){_push=i; break;} // find first visible and push it
         }
      }

      // get highlight
      if(_push>=0)_lit=_push;else
      if(Gui.ms()!=this || (!Ms._action && _menu_prev>=0))_lit=_menu_prev;else
      {
        _lit=-1;
         Flt l=rect().min.x+gpc.offset.x,
             b=rect().min.y+gpc.offset.y,
             t=rect().max.y+gpc.offset.y;
         REP(elms())
         {
            Elm &e=elm(i);
            if( !e.hidden && !e.menu.disabled() && Ms.test(Rect(l+e.x, b, l+e.x1(), t))){_lit=i; goto found;}
         }
         if(Gui.ms()==this)_lit=_menu_prev;
         found:;
      }

      // activate menu
      if(_lit!=_menu_prev && _lit>=0)
         if(Ms.bp(0) || _menu_prev>=0 || _push>=0)
      {
         Elm &e=elm(_lit);
         if(Ms.bp(0))
         {
            e.menu.list.cur=-1;
            Ms.eat(0);
         }else
         {
            e.menu.list.cur=(e.menu.list.elms() ? 0 : -1);
         }
         Activate(T, e, gpc);
      }
      REPA(Touches)
      {
         Touch &touch=Touches[i];
         if(touch.guiObj()==this)
         {
            Flt l=rect().min.x+gpc.offset.x,
                b=rect().min.y+gpc.offset.y,
                t=rect().max.y+gpc.offset.y;
            REP(elms())
            {
               Elm &e=elm(i);
               if( !e.hidden && !e.menu.disabled() && Cuts(touch.pos(), Rect(l+e.x, b, l+e.x1(), t)))
               {
                  if(touch.pd() && _menu_prev==i) // deselect
                  {
                     activate();
                  }else
                  if(touch.pd() || _menu_prev>=0) // if we just pushed or there was already selected
                  {
                     e.menu.list.cur=-1;
                     Activate(T, e, gpc);
                  }
                  break;
               }
            }
         }
      }

     _push=-1;
     _menu_prev=-1;
      if(Gui.menu())REP(elms())if(elm(i).menu.contains(Gui.menu())){_menu_prev=i; break;}
   }
}
/******************************************************************************/
void MenuBar::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(GuiSkin *skin=getSkin())
   {
      D.clip(gpc.clip);

      Rect rect=T.rect()+gpc.offset;

      if(skin->menubar.background        )skin->menubar.background->draw(skin->menubar.background_color, TRANSPARENT, rect);else
      if(skin->menubar.background_color.a)                     rect.draw(skin->menubar.background_color);

      if(TextStyle *text_style=skin->menubar.text_style())
      {
         TextStyleParams ts=*text_style; ts.align.set(0, 0); ts.size=rect.h()*skin->menubar.text_size; Flt y=rect.centerY();
         FREP(elms())if(!elm(i).hidden)
         {
            Elm &e=elm(i);
            Flt  x=rect.min.x+e.x;
            if(_lit==i)
            {
               Rect lit_rect(x, rect.min.y, x+e.w, rect.max.y);
               if(skin->menubar.highlight        )skin->menubar.highlight->draw(skin->menubar.highlight_color, TRANSPARENT, lit_rect);else
               if(skin->menubar.highlight_color.a)                lit_rect.draw(skin->menubar.highlight_color);
            }
            D.text(ts, x+e.w*0.5f, y, e.display_name);
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
