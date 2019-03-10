/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void CheckBox::zero()
{
  _focusable     =false;
  _on            =false;
  _multi         =false;
  _func_immediate=false;
  _func_user     =null;
  _func          =null;
  _lit           =0;
}
CheckBox::CheckBox() {zero();}
CheckBox& CheckBox::del()
{
   skin.clear();
   super::del(); zero(); return T;
}
CheckBox& CheckBox::create(Bool on)
{
   del();

   T._type      =GO_CHECKBOX;
   T._visible   =true;
   T._on        =on;
   T._rect.max.x= 0.04f;
   T._rect.min.y=-0.04f;

   return T;
}
CheckBox& CheckBox::create(C CheckBox &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type          =GO_CHECKBOX;
         skin          =src.skin;
        _focusable     =src._focusable;
        _on            =src._on;
        _multi         =src._multi;
        _lit           =src._lit;
        _func          =src._func;
        _func_user     =src._func_user;
        _func_immediate=src._func_immediate;
      }   
   }
   return T;
}
CheckBox& CheckBox::toggle(SET_MODE mode)
{
  _on^=1; _multi=false;
   if(mode!=QUIET)call(mode!=NO_SOUND);
   return T;
}
CheckBox& CheckBox::set(Bool on, SET_MODE mode)
{
   if(T._on!=on || _multi)
   {
      T._on=on; _multi=false;
      if(mode!=QUIET)call(mode!=NO_SOUND);
   }
   return T;
}
CheckBox& CheckBox::setMulti()
{
  _multi=true;
   return T;
}
/******************************************************************************/
CheckBox& CheckBox::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void CheckBox::call(Bool sound)
{
   if(sound)Gui.playClickSound();
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
CheckBox& CheckBox::focusable(Bool on) {if(_focusable!=on){_focusable=on; if(!on)kbClear();} return T;}
/******************************************************************************/
void CheckBox::update(C GuiPC &gpc)
{
   Bool enabled=(T.enabled() && gpc.enabled);
   if(  enabled)
   {
      if((Gui.kb()==this && Kb.k(' '))
      || (Gui.ms()==this && Ms.bp(0) ))toggle();else // don't check touches if we already toggled it with keyboard/mouse (this is to avoid double presses when using 'SimulateTouches')
         REPA(Touches)
      {
         Touch &touch=Touches[i];
         if(touch.guiObj()==this)
            if(touch.scrolling() ? touch.rs() : touch.pd())toggle();
      }
    //AdjustValBool(_lit, Gui.ms()==this && enabled, Gui._time_d_fade_in, Gui._time_d_fade_out);
      if(Gui.ms()==this)_lit=1;else MAX(_lit-=Gui._time_d_fade_out, 0);
   }
}
void CheckBox::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(GuiSkin *skin=getSkin())
        if(PanelImage *panel_image=(_multi ? skin->checkbox.multi() : T() ? skin->checkbox.on() : skin->checkbox.off()))
   {
      Rect rect=T.rect()+gpc.offset, ext_rect;
      panel_image->extendedRect(rect, ext_rect);
      if(Cuts(ext_rect, gpc.clip))
      {
         D.clip(gpc.clip);
         D.alignScreenToPixel(rect);
         Bool  enabled  =(T.enabled() && gpc.enabled);
         Color highlight=ColorMulZeroAlpha(skin->mouse_highlight_color, _lit);
         panel_image->draw(enabled ? skin->checkbox.normal_color : skin->checkbox.disabled_color, highlight, rect);
         if(Gui.kb()==this)Gui.kbLit(this, rect, skin);
      }
   }
}
/******************************************************************************/
Bool CheckBox::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(1); // version

      f<<_focusable<<_on<<_multi;
      f._putAsset(skin.name(path));
      return f.ok();
   }
   return false;
}
Bool CheckBox::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 1:
      {
        _type=GO_CHECKBOX;

         f>>_focusable>>_on>>_multi;
         skin.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_CHECKBOX;

         f>>_focusable>>_on;
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
