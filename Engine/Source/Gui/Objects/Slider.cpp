/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Slider::zero()
{
  _vertical      =false;
  _focusable     =false;
  _value         =0;
  _lit           =0;
  _func_immediate=false;
  _func_user     =null;
  _func          =null;
}
Slider::Slider() {zero();}
Slider& Slider::del()
{
   skin.clear();
   super::del(); zero(); return T;
}
Slider& Slider::create()
{
   del();

  _type      =GO_SLIDER;
  _visible   =true;
  _rect.max.x= 0.30f;
  _rect.min.y=-0.05f;

   return T;
}
Slider& Slider::create(C Slider &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type          =GO_SLIDER;
         skin          =src. skin;
        _vertical      =src._vertical;
        _focusable     =src._focusable;
        _value         =src._value;
        _lit           =src._lit;
        _func_immediate=src._func_immediate;
        _func_user     =src._func_user;
        _func          =src._func;
      }
   }
   return T;
}
/******************************************************************************/
Slider& Slider::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void Slider::call()
{
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
Slider& Slider::focusable(Bool on) {if(_focusable!=on){_focusable=on; if(!on)kbClear();} return T;}
/******************************************************************************/
Slider& Slider::set(Flt value, SET_MODE mode)
{
   SAT(value);
   if(T._value!=value)
   {
      T._value=value;
      if(mode!=QUIET)call();
   }
   return T;
}
/******************************************************************************/
Slider& Slider::rect(C Rect &rect)
{
 //if(T.rect()!=rect) below looks fast so don't need this
   {
      super::rect(rect);
      T._vertical=T.rect().vertical();
   }
   return T;
}
/******************************************************************************/
GuiObj* Slider::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible() && gpc.visible)
   {
      Rect r=rect();
      if(GuiSkin *skin=getSkin())
         if(PanelImage *slider=skin->slider.slider())
      {
         if(_vertical)
         {
            Flt h=rect().w()*slider->image.aspect()*0.5f,
                y=rect().h()*_value+rect().min.y;
            r.includeY(y-h, y+h);
         }else
         {
            Flt w=rect().h()*slider->image.aspect()*0.5f,
                x=rect().w()*_value+rect().min.x;
            r.includeX(x-w, x+w);
         }
      }
      if(Cuts(pos, (r+gpc.offset)&gpc.clip))
      {
         if(!mouse_wheel || mouse_wheel->type()!=GO_SLIDEBAR)mouse_wheel=this; // don't grab wheel focus if it already belongs to a SlideBar (for example Slider located in a Region with SlideBars)
         return this;
      }
   }
   return null;
}
void Slider::update(C GuiPC &gpc)
{
   Bool enabled=(T.enabled() && gpc.enabled);
   if(  enabled)
   {
      if(Gui.wheel()==this && (Ms.wheel() || Ms.wheelX()))set(_value+(Ms.wheel()+Ms.wheelX())*0.1f);
      if(Gui.kb   ()==this)
      {
         if(_vertical)
         {
            if(Kb.b(KB_DOWN))set(_value-Time.d()*0.2f);
            if(Kb.b(KB_UP  ))set(_value+Time.d()*0.2f);
         }else
         {
            if(Kb.b(KB_LEFT ))set(_value-Time.d()*0.2f);
            if(Kb.b(KB_RIGHT))set(_value+Time.d()*0.2f);
         }
      }
    C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); if(!pos)REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())pos=&Touches[i].pos();
      if(   pos)set(_vertical ? (pos->y-(rect().min.y+gpc.offset.y))/rect().h() : (pos->x-(rect().min.x+gpc.offset.x))/rect().w());

    //AdjustValBool(_lit, Gui.ms()==this && enabled, Gui._time_d_fade_in, Gui._time_d_fade_out);
      if(Gui.ms()==this)_lit=1;else MAX(_lit-=Gui._time_d_fade_out, 0);
   }
}
void Slider::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(GuiSkin *skin=getSkin())
   {
      Rect rect=T.rect()+gpc.offset, ext_rect;
      if(skin->slider.background)            skin->slider.background->extendedRect(rect, ext_rect);else ext_rect =rect;
      if(skin->slider.progress  ){Rect r; if(skin->slider.progress  ->extendedRect(rect, r       ))     ext_rect|=r;}
   // for simplicity 'skin->slider.slider' is not included in 'ext_rect'
      if(Cuts(ext_rect, gpc.clip))
      {
         D.clip(gpc.clip);
         D.alignScreenToPixel(rect);

         Rect background_rect=rect;
         if(_vertical)
         {
            Flt shrink=skin->slider.background_shrink*rect.w();
            background_rect.min.x+=shrink;
            background_rect.max.x-=shrink;
         }else
         {
            Flt shrink=skin->slider.background_shrink*rect.h();
            background_rect.min.y+=shrink;
            background_rect.max.y-=shrink;
         }

         // background
         if(skin->slider.background)
         {
            if(_vertical)skin->slider.background->drawVertical(skin->slider.background_color, TRANSPARENT, background_rect);
            else         skin->slider.background->draw        (skin->slider.background_color, TRANSPARENT, background_rect);
         }else
         if(skin->slider.background_color.a)background_rect.draw(skin->slider.background_color);

         // progress
         if(skin->slider.progress)
         {
            if(skin->slider.draw_progress_partial)
            {
               if(_vertical)skin->slider.progress->drawVerticalFrac(skin->slider.progress_color, TRANSPARENT, background_rect, T(), false);
               else         skin->slider.progress->drawFrac        (skin->slider.progress_color, TRANSPARENT, background_rect, T(), false);
            }else
            {
               Rect prog_rect=background_rect; if(_vertical)prog_rect.max.y=background_rect.lerpY(T());else prog_rect.max.x=background_rect.lerpX(T());
               if(_vertical)skin->slider.progress->drawVertical(skin->slider.progress_color, TRANSPARENT, prog_rect);
               else         skin->slider.progress->draw        (skin->slider.progress_color, TRANSPARENT, prog_rect);
            }
         }else
         if(skin->slider.progress_color.a)
         {
            Rect prog_rect=background_rect; if(_vertical)prog_rect.max.y=background_rect.lerpY(T());else prog_rect.max.x=background_rect.lerpX(T());
            prog_rect.draw(skin->slider.progress_color);
         }

         // slider
         if(skin->slider.slider)
         {
            Color highlight=ColorMulZeroAlpha(skin->mouse_highlight_color, _lit);
            if(_vertical)
            {
               Rect_C r(rect.centerX(), rect.lerpY(T()), rect.w(), rect.w()*skin->slider.slider->image.aspect()); D.alignScreenToPixel(r);
               skin->slider.slider->drawVertical(skin->slider.slider_color, highlight, r);
            }else
            {
               Rect_C r(rect.lerpX(T()), rect.centerY(), rect.h()*skin->slider.slider->image.aspect(), rect.h()); D.alignScreenToPixel(r);
               skin->slider.slider->draw(skin->slider.slider_color, highlight, r);
            }
         }

         if(Gui.kb()==this)Gui.kbLit(this, rect, skin);
      }
   }
}
/******************************************************************************/
Bool Slider::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(2); // version

      f<<_focusable<<_value<<_lit;
      f._putAsset(skin.name(path));
      return f.ok();
   }
   return false;
}
Bool Slider::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 2:
      {
        _type=GO_SLIDER;

         f>>_focusable>>_value>>_lit; _vertical=rect().vertical();
         skin.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 1:
      {
        _type=GO_SLIDER;

         f>>_focusable>>_value>>_lit; _vertical=rect().vertical();
         f._getStr(); f._getStr();
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_SLIDER;

         f>>_focusable; f.skip(4); f>>_value>>_lit; _vertical=rect().vertical();
         f._getStr8();
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
