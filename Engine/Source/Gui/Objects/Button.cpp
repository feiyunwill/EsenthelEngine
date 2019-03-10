/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static void WindowButtonCheck(Button &button)
{
   if(GuiObj *parent=button.parent())
      if(parent->type()==GO_WINDOW)
   {
      Window &window= parent->asWindow();
      IntPtr  index =&button-window.button;
      if(InRange(index, window.button))window.setButtons();
   }
}
/******************************************************************************/
void Button::zero()
{
   mode          =BUTTON_DEFAULT;
   sound         =true;
   image_color   =WHITE;
   text_align    =0;
   text_size     =1;
  _push_button   =false;
  _on            =false;
  _vertical      =false;
  _focusable     =true;
  _pixel_align   =true;
  _sub_type      =BUTTON_TYPE_DEFAULT;
  _func_immediate=false;
  _func_user     =null;
  _func          =null;
  _lit           =0;
}
Button::Button() {zero();}
Button& Button::del()
{
   text .clear();
   image.clear();
   skin .clear();
   super::del(); zero(); return T;
}
void Button::setParams()
{
  _type    =GO_BUTTON;
  _sub_type=BUTTON_TYPE_DEFAULT;
}
Button& Button::create(C Str &text)
{
   del();

   setParams();
   T._visible    = true;
   T._rect.max.x = 0.27f;
   T._rect.min.y =-0.06f;
   T. text       = text;
   return T;
}
Button& Button::create(C Button &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type          =GO_BUTTON;
         mode          =src. mode;
         sound         =src. sound;
         image_color   =src. image_color;
         text_align    =src. text_align;
         text_size     =src. text_size;
         text          =src. text;
         image         =src. image;
         skin          =src. skin;
        _push_button   =src._push_button;
        _on            =src._on;
        _vertical      =src._vertical;
        _focusable     =src._focusable;
        _pixel_align   =src._pixel_align;
        _sub_type      =src._sub_type;
        _func          =src._func;
        _func_user     =src._func_user;
        _func_immediate=src._func_immediate;
        _lit           =src._lit;
      }
   }
   return T;
}
/******************************************************************************/
Button& Button::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void Button::call(Bool sound)
{
   if(sound && T.sound && mode!=BUTTON_CONTINUOUS && _sub_type==BUTTON_TYPE_DEFAULT)Gui.playClickSound(); // don't play sounds for 'Tab' because they're played by 'Tabs'
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
TextStyle* Button::textParams(Flt &text_size, Flt &text_padd, C Flt *height)C
{
   if(GuiSkin *skin=getSkin())
   {
      TextStyle *text_style;
      switch(_sub_type)
      {
         default                     : text_style=skin->button     .text_style(); text_size=skin->button     .text_size; text_padd=skin->button     .text_padd; break; // BUTTON_TYPE_DEFAULT
         case BUTTON_TYPE_COMBOBOX   : text_style=skin->combobox   .text_style(); text_size=skin->combobox   .text_size; text_padd=skin->combobox   .text_padd; break;
         case BUTTON_TYPE_LIST_COLUMN: text_style=skin->list.column.text_style(); text_size=skin->list.column.text_size; text_padd=skin->list.column.text_padd; break;

         case BUTTON_TYPE_TAB_LEFT        :
         case BUTTON_TYPE_TAB_HORIZONTAL  :
         case BUTTON_TYPE_TAB_RIGHT       :
         case BUTTON_TYPE_TAB_TOP         :
         case BUTTON_TYPE_TAB_VERTICAL    :
         case BUTTON_TYPE_TAB_BOTTOM      :
         case BUTTON_TYPE_TAB_TOP_LEFT    :
         case BUTTON_TYPE_TAB_TOP_RIGHT   :
         case BUTTON_TYPE_TAB_BOTTOM_LEFT :
         case BUTTON_TYPE_TAB_BOTTOM_RIGHT:
            text_style=skin->tab.horizontal.text_style(); text_size=skin->tab.horizontal.text_size; text_padd=skin->tab.horizontal.text_padd; // for simplicity return the same tab as they should be all the same
         break;
         // others don't use text
      }
      Flt h=(height ? *height : rect().h());
      text_size*=h*T.text_size;
      text_padd*=h;
      return text_style;
   }
   return null;
}
Flt Button::textWidth(C Flt *height)C
{
   if(text.is())
   {
      Flt text_size, text_padd; if(TextStyle *text_style=textParams(text_size, text_padd, height))
      {
         TextStyleParams ts=*text_style; ts.size=text_size;
      #if DEFAULT_FONT_FROM_CUSTOM_SKIN
         if(!ts.font())if(GuiSkin *skin=getSkin())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
      #endif
         return ts.textWidth(text);
      }
   }
   return 0;
}
/******************************************************************************/
Button& Button::set(Bool on, SET_MODE mode)
{
   if(T._on!=on && T.mode==BUTTON_TOGGLE)
   {
      T._on=on;
      if(mode!=QUIET)call(mode!=NO_SOUND);
   }
   return T;
}
Button& Button::push    (                 ) {T._push_button=true ; return T;}
Button& Button::setText (C Str      &text ) {T. text       =text ; return T;}
Button& Button::setImage(C ImagePtr &image) {T. image      =image; return T;}
Button& Button::subType (BUTTON_TYPE type ) {T._sub_type   =type ; return T;}
/******************************************************************************/
Button& Button:: enabled (Bool  enabled) {if(T. enabled()!= enabled){if(!enabled && mode!=BUTTON_TOGGLE)_on=false; super:: enabled( enabled);} return T;}
Button& Button::disabled (Bool disabled) {if(T.disabled()!=disabled){if(disabled && mode!=BUTTON_TOGGLE)_on=false; super::disabled(disabled);} return T;}
Button& Button::focusable(Bool on      ) {if(T._focusable!=on      ){_focusable=on; if(!on)kbClear();} return T;}
/******************************************************************************/
Button& Button::hide()
{
   Bool visible =T.visible(); super::hide();
   if(  visible!=T.visible())WindowButtonCheck(T);
   return T;
}
Button& Button::show()
{
   Bool visible =T.visible(); super::show();
   if(  visible!=T.visible())WindowButtonCheck(T);
   return T;
}
/******************************************************************************/
void Button::update(C GuiPC &gpc)
{
   Bool manual_push=_push_button; _push_button=false;
   Bool enabled=(T.enabled() && gpc.enabled);
   if(  enabled)
   {
      Bool call_func=false, lit=false;
      Byte state=0;
      if(Gui.ms()==this)
      {
         lit   =true;
         state|=((Ms.b (0) && (mode!=BUTTON_DEFAULT || Gui.msLit()==this)) ? BS_ON       : 0) // mode==BUTTON_DEFAULT requires Gui.msLit()==this while other modes don't
              | ((Ms.bp(0)                                               ) ? BS_PUSHED   : 0)
              | ((Ms.br(0) &&                          Gui.msLit()==this ) ? BS_RELEASED : 0);
      }
      if(Gui.kb()==this)
      {
         if(mode==BUTTON_CONTINUOUS)state|=((Kb.b(KB_ENTER) || Kb.b(KB_NPENTER)) ? BS_ON : 0);
         if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first()){Kb.eatKey(); state|=(BS_PUSHED|BS_RELEASED|BS_ON);} // 'BS_RELEASED' to work for 'BUTTON_DEFAULT', this is because there's no way to cancel keyboard activation (mouse cursor for example can be moved away before releasing button, while keyboard can't)
      }
      REPA(Touches)
      {
         Touch &t=Touches[i]; if(t.guiObj()==this)
         {
            lit|=t.stylus();
            if(t.scrolling()) // if touch is used for scrolling, then we need to process the button differently
            {
               state|=((t.on() && (mode!=BUTTON_DEFAULT || Gui.objAtPos(t.pos())==this)) ? BS_ON                 : 0)  // mode==BUTTON_DEFAULT requires Gui.objAtPos(t.pos())==this while other modes don't
                  //| ((t.pd()                                                         ) ? BS_PUSHED             : 0)  // ignore the first push
                    | ((t.rs() &&                          Gui.objAtPos(t.pos())==this ) ? BS_RELEASED|BS_PUSHED : 0); // process touch release as both BS_RELEASED|BS_PUSHED so it can be used instead of touch pushes
            }else
            {
               state|=((t.on() && (mode!=BUTTON_DEFAULT || Gui.objAtPos(t.pos())==this)) ? BS_ON       : 0) // mode==BUTTON_DEFAULT requires Gui.objAtPos(t.pos())==this while other modes don't
                    | ((t.pd()                                                         ) ? BS_PUSHED   : 0)
                    | ((t.rs() &&                          Gui.objAtPos(t.pos())==this ) ? BS_RELEASED : 0);
            }
         }
      }
      switch(mode)
      {
         case BUTTON_DEFAULT:
         {
            if(ButtonRs(state))manual_push=true;
           _on       =ButtonOn(state);
            call_func=manual_push;
         }break;

         case BUTTON_CONTINUOUS:
         {
            if(ButtonPd(state))manual_push=true;
           _on       =(manual_push || ButtonOn(state));
            call_func=_on;
         }break;

         case BUTTON_TOGGLE:
         {
            if(ButtonPd(state))manual_push=true;
            if(manual_push)_on^=1;
            call_func=manual_push;
         }break;

         case BUTTON_IMMEDIATE:
         {
            if(ButtonPd(state))manual_push=true;
           _on       =(manual_push || ButtonOn(state));
            call_func= manual_push;
         }break;
      }
      if(call_func)call(true);

    //AdjustValBool(_lit, lit, Gui._time_d_fade_in, Gui._time_d_fade_out);
      if(lit)_lit=1;else MAX(_lit-=Gui._time_d_fade_out, 0);
   }
}
/******************************************************************************/
void Button::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(GuiSkin *skin=getSkin())
   {
      Rect                  r=rect()+gpc.offset;
    C GuiSkin::Button      *button_skin ; // never  null
    C GuiSkin::ButtonImage *button_image; // may be null
      Int                   image_x     ; // valid only if "button_image!=null"
      switch(_sub_type)
      {
         default                          : button_skin=&skin->button          ; button_image= null                 ;            break; // BUTTON_TYPE_DEFAULT
         case BUTTON_TYPE_COMBOBOX        : button_skin=                         button_image=&skin->combobox       ; image_x=1; break;
         case BUTTON_TYPE_LIST_COLUMN     : button_skin=&skin->list.column     ; button_image= null                 ;            break;
         case BUTTON_TYPE_PROPERTY_VALUE  : button_skin=                         button_image=&skin->property.value ; image_x=0; break;
         case BUTTON_TYPE_REGION_VIEW     : button_skin=                         button_image=&skin->region  .view  ; image_x=0; break;
         case BUTTON_TYPE_SLIDEBAR_LEFT   : button_skin=                         button_image=&skin->slidebar.left  ; image_x=0; break;
         case BUTTON_TYPE_SLIDEBAR_CENTER : button_skin=                         button_image=&skin->slidebar.center; image_x=0; break;
         case BUTTON_TYPE_SLIDEBAR_RIGHT  : button_skin=                         button_image=&skin->slidebar.right ; image_x=0; break;
         case BUTTON_TYPE_TAB_LEFT        : button_skin=&skin->tab.left        ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_HORIZONTAL  : button_skin=&skin->tab.horizontal  ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_RIGHT       : button_skin=&skin->tab.right       ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_TOP         : button_skin=&skin->tab.top         ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_VERTICAL    : button_skin=&skin->tab.vertical    ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_BOTTOM      : button_skin=&skin->tab.bottom      ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_TOP_LEFT    : button_skin=&skin->tab.top_left    ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_TOP_RIGHT   : button_skin=&skin->tab.top_right   ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_BOTTOM_LEFT : button_skin=&skin->tab.bottom_left ; button_image= null                 ;            break;
         case BUTTON_TYPE_TAB_BOTTOM_RIGHT: button_skin=&skin->tab.bottom_right; button_image= null                 ;            break;
         case BUTTON_TYPE_TEXTLINE_CLEAR  : button_skin=                         button_image=&skin->textline.clear ; image_x=0; break;
         case BUTTON_TYPE_WINDOW_MINIMIZE : button_skin=                         button_image=&skin->window.minimize; image_x=0; break;
         case BUTTON_TYPE_WINDOW_MAXIMIZE : button_skin=                         button_image=&skin->window.maximize; image_x=0; break;
         case BUTTON_TYPE_WINDOW_CLOSE    : button_skin=                         button_image=&skin->window.close   ; image_x=0; break;
      }
    C PanelImage *panel_image ; // may be null
    C Color      *pushed_color; // never  null
      Bool        enabled=(gpc.enabled && T.enabled());
      if(enabled)
      {
         if(T()){panel_image=button_skin->pushed(); pushed_color=&button_skin->pushed_color;}
         else   {panel_image=button_skin->normal(); pushed_color=&button_skin->normal_color;}
      }else
      {
         if(T()){panel_image=button_skin->pushed_disabled(); pushed_color=&button_skin->pushed_disabled_color;}
         else   {panel_image=button_skin->       disabled(); pushed_color=&button_skin->       disabled_color;}
      }
      Rect    ext_rect; if(panel_image)panel_image->extendedRect(r, ext_rect);else ext_rect=r;
      if(Cuts(ext_rect, gpc.clip))
      {
         D.clip(gpc.clip);
         if(_pixel_align)D.alignScreenToPixel(r);

         Color      highlight=ColorMulZeroAlpha(skin->mouse_highlight_color, lit());
         Flt        text_size, text_padd;
         Rect       text_rect;
         TextStyle *text_style=null;

         if(text.is())
         {
            text_rect=r;
            if(text_style=textParams(text_size, text_padd)){text_rect.min.x+=text_padd; text_rect.max.x-=text_padd;}
         }

         // draw panel image
         if(panel_image)
         {
            if(_vertical)panel_image->drawVertical(*pushed_color, highlight, r);
            else         panel_image->draw        (*pushed_color, highlight, r);
         }//else .. there's no Rect draw because to avoid it we would need to set 'pushed_color' to TRANSPARENT, however it also affects the 'text' and images as well

         // draw custom image
         if(image)
         {
            Color col=ColorMul(image_color, *pushed_color); // 'image_color' is user modification, 'pushed_color' is push/disabled modification
            if(!panel_image)image->draw   (col, highlight, r);else // if there's no 'panel_image' then there's no way to determine the button boundaries, so let's stretch the custom image to entire rect
            if( text.is()  )image->draw   (col, highlight, Rect(text_rect.min.x, text_rect.min.y, text_rect.min.x+image->aspect()*text_rect.h(), text_rect.max.y));else // draw on the left if there's text available
                            image->drawFit(col, highlight, r);
         }

         // draw text
         if(text.is() && text_style)
         {
            TextStyleParams ts=*text_style; ts.size=text_size; ts.align.x=text_align; ts.color=ColorMul(ts.color, *pushed_color); // modify text color based on pushed/disabled
         #if DEFAULT_FONT_FROM_CUSTOM_SKIN
            if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
         #endif
            D.text(ts, text_rect, text);
         }

         // draw special image (draw after text in case of combobox arrows)
         if(button_image)
            if(C Image *special_image=button_image->image())
         {
            Color col    =ColorMul         (button_image->image_color    , *pushed_color); // modify image color based on pushed/disabled
                //col_add=ColorMulZeroAlpha(button_image->image_color_add, *pushed_color);
            if(_vertical)
            {
               if(!image_x)special_image->drawFitVertical(col, TRANSPARENT, r);
               else        special_image->drawVertical   (col, TRANSPARENT, Rect(r.min.x, r.max.y-special_image->aspect()*r.w(), r.max.x, r.max.y));
            }else
            {
               if(!image_x)special_image->drawFit(col, TRANSPARENT, r);else
               {
                  Rect rect; rect.setY(r.min.y, r.max.y);
                  Flt w=special_image->aspect()*r.h();
                  if(r.w()>=w)rect.setX(r.max.x-w, r.max.x); // if image can fit in rectangle
                  else        {Flt c=r.centerX(); w*=0.5f; rect.setX(c-w, c+w);} // if image width is bigger than rect width, then draw at the center of the rectangle
                  special_image->draw(col, TRANSPARENT, rect);
               }
            }
         }

         if(Gui.kb()==this)Gui.kbLit(this, r, skin);
      }
   }
}
/******************************************************************************/
Bool Button::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.putMulti(Byte(7), mode, sound, image_color, text_align, text_size, _on, _focusable)<<text; // version
      f.putAsset(image.id());
      f.putAsset(skin .id());
      // '_pixel_align' doesn't need to be saved as it's currently being used only by Tab      and there it's always set manually
      // '_vertical'    doesn't need to be saved as it's currently being used only by SlideBar and there it's always set manually
      // '_sub_type'    doesn't need to be saved as it's assigned by parents
      return f.ok();
   }
   return false;
}
Bool Button::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 7:
      {
         setParams();
         f.getMulti(mode, sound, image_color, text_align, text_size, _on, _focusable)>>text;
         image.require(f.getAssetID(), path);
         skin .require(f.getAssetID(), path);
         if(f.ok())return true;
      }break;

      case 6:
      {
         setParams();
         f.getMulti(mode, sound, image_color, text_align, text_size, _on, _focusable)._getStr2(text);
         image.require(f.getAssetID(), path);
         skin .require(f.getAssetID(), path);
         if(f.ok())return true;
      }break;

      case 5:
      {
         setParams();
         f>>mode>>image_color>>text_align>>text_size>>_on>>_focusable; f._getStr2(text);
         image.require(f._getAsset(), path);
         skin .require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 4:
      {
         setParams();
         f>>mode; f.skip(1); f>>_focusable; f.skip(1); f>>image_color>>text_align>>text_size; f.skip(4); f>>_on; f._getStr(text); f._getStr(); f._getStr();
         if(f.ok())return true;
      }break;

      case 3:
      {
         setParams();
         f>>mode; f.skip(1); f>>_focusable; f.skip(1); f>>image_color>>text_align; f.skip(4); f>>_on; text_size=1; f._getStr(text); f._getStr(); f._getStr();
         if(f.ok())return true;
      }break;

      case 2:
      {
         setParams();
         f>>mode; f.skip(1); f>>_focusable; f.skip(1); f>>image_color>>text_align; f.skip(4); f>>_on; text_size=1; f._getStr(text); f._getStr();
         if(f.ok())return true;
      }break;

      case 1:
      {
         setParams();
         f>>mode; f.skip(1); f>>_focusable; f.skip(1); f>>image_color>>text_align; f.skip(4); f>>_on; Swap(image_color.r, image_color.b); text_size=1; text=f._getStr16(); f._getStr16();
         if(f.ok())return true;
      }break;

      case 0:
      {
         setParams();
         Char8 text[32]; f>>text;
         f>>mode; f.skip(1); f>>_focusable; f.skip(1); f>>image_color>>text_align; f.skip(4); f>>_on; Swap(image_color.r, image_color.b); T.text=text; text_size=1; f._getStr8();
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
