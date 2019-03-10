/******************************************************************************/
#include "stdafx.h"
#define CP_PROPS_RGB   0
#define CP_PROPS_ALPHA 3
#define CP_PROPS_RGBA  4
#define CP_PROPS_HSB   5
namespace EE{
/******************************************************************************/
static TextStyle ColorPickerTS;
/******************************************************************************/
void ColorPicker::SetTextStyle()
{
   ColorPickerTS.reset(true).size=0.04f; ColorPickerTS.align.set(1, 0);
}
/******************************************************************************/
ColorPicker::ColorPicker()
{
  _real          =false;
  _func_immediate=false;
  _func_user     =null;
  _func          =null;
  _rgba.set(1, 0, 0, 1);
  _hsb .set(0, 1, 1   );
}
/******************************************************************************/
ColorPicker& ColorPicker::show()
{
   if(hidden())setOld();
   super::show();
   return T;
}
ColorPicker& ColorPicker::mode(Bool real)
{
   if(T._real!=real)
   {
      T._real=real;

      DATA_TYPE dt   =(_real ? DATA_REAL : DATA_INT);
      Flt       range=(_real ?      1.0f :      255),
                speed=(_real ?      0.4f :       40);

      REPA(_props)if(i!=CP_PROPS_RGBA)_props[i].range(0, range).mouseEditSpeed(speed).md.type=dt;

      toGui(true, true, true, false);
   }
   return T;
}
void ColorPicker::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(Gui.window()==this)
   {
      if((Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)) && Kb.k.first() || Ms.bp(2)){Kb.eatKey(); Ms.eat(2); hide();}
   }
}
/******************************************************************************/
void ColorPicker::toGui(Bool rgb, Bool alpha, Bool hsb, Bool rgba)
{
   if(rgb  )REP(3)_props[CP_PROPS_RGB+i].toGui();
   if(alpha)      _props[CP_PROPS_ALPHA].toGui();
   if(hsb  )REP(3)_props[CP_PROPS_HSB+i].toGui();
   if(rgba )      _props[CP_PROPS_RGBA ].toGui();
}
void ColorPicker::setOld()
{
  _old=_rgba;
}
void ColorPicker::_set(C Vec4 &color, SET_MODE mode)
{
   if(T._rgba!=color)
   {
      T._rgba=color;
      T._hsb =RgbToHsb(color.xyz);
      if(mode!=QUIET)call();
   }
}
void ColorPicker::_setRGB(C Vec &rgb, SET_MODE mode)
{
   if(T._rgba.xyz!=rgb)
   {
      T._rgba.xyz=rgb;
      T._hsb     =RgbToHsb(rgb);
      if(mode!=QUIET)call();
   }
}
void ColorPicker::_setHSB(C Vec &hsb, SET_MODE mode)
{
   if(T._hsb!=hsb)
   {
      T._hsb     =hsb;
      T._rgba.xyz=HsbToRgb(hsb);
      if(mode!=QUIET)call();
   }
}
void ColorPicker::_setAlpha(Flt alpha, SET_MODE mode)
{
   if(T._rgba.w!=alpha)
   {
      T._rgba.w=alpha;
      if(mode!=QUIET)call();
   }
}
ColorPicker& ColorPicker::set     (C Vec4 &color, SET_MODE mode) {_set     (color, mode); toGui(                   ); return T;}
ColorPicker& ColorPicker::setRGB  (C Vec  &rgb  , SET_MODE mode) {_setRGB  (rgb  , mode); toGui(true , false, true ); return T;}
ColorPicker& ColorPicker::setHSB  (C Vec  &hsb  , SET_MODE mode) {_setHSB  (hsb  , mode); toGui(true , false, true ); return T;}
ColorPicker& ColorPicker::setAlpha(  Flt   alpha, SET_MODE mode) {_setAlpha(alpha, mode); toGui(false, true , false); return T;}
/******************************************************************************/
ColorPicker& ColorPicker::func(void (*func)(Ptr user), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void ColorPicker::call()
{
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
void ColorPicker::SatLum::update(C GuiPC &gpc)
{
   if(gpc.visible && gpc.enabled && visible() && enabled())
   {
    C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); if(!pos)REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())pos=&Touches[i].pos();
      if(   pos)
      {
         ColorPicker &cp  =*(ColorPicker*)user;
         Rect         rect=T.rect()+gpc.offset;
         cp.setHSB(Vec(cp._hsb.x, LerpRS(rect.min.x, rect.max.x, pos->x), LerpRS(rect.min.y, rect.max.y, pos->y)));
      }
   }
}
void ColorPicker::SatLum::draw(C GuiPC &gpc)
{
   if(gpc.visible && visible())
   {
      D.clip(gpc.clip);
      ColorPicker &cp  =*(ColorPicker*)user;
      Rect         rect=T.rect()+gpc.offset;

      Int steps=8;
      FREPD(y, steps)
      FREPD(x, steps)
      {
         Flt sx=Flt(x)/steps, sx1=Flt(x+1)/steps,
             sy=Flt(y)/steps, sy1=Flt(y+1)/steps;

         Color clu=ColorHSB(cp._hsb.x, sx , sy1),
               cru=ColorHSB(cp._hsb.x, sx1, sy1),
               crd=ColorHSB(cp._hsb.x, sx1, sy ),
               cld=ColorHSB(cp._hsb.x, sx , sy );

         Flt l=rect.lerpX(sx ),
             r=rect.lerpX(sx1),
             d=rect.lerpY(sy ),
             u=rect.lerpY(sy1);

         VI.tri(clu, cru, cld, Vec2(l, u), Vec2(r, u), Vec2(l, d));
         VI.tri(cru, crd, cld, Vec2(r, u), Vec2(r, d), Vec2(l, d));
      }
      VI.end();
      
      rect.draw(Gui.borderColor(), false);

      ALPHA_MODE alpha=D.alpha(ALPHA_INVERT); Circle(0.01f, rect.lerp(cp._hsb.y, cp._hsb.z)).draw(WHITE, false);
                       D.alpha(alpha       );
   }
}
void ColorPicker::Hue::update(C GuiPC &gpc)
{
   if(gpc.visible && gpc.enabled && visible() && enabled())
   {
    C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); if(!pos)REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())pos=&Touches[i].pos();
      if(   pos)
      {
         ColorPicker &cp  =*(ColorPicker*)user;
         Rect         rect=T.rect()+gpc.offset;
         cp.setHSB(Vec(LerpRS(rect.min.y, rect.max.y, pos->y), cp._hsb.y, cp._hsb.z));
      }
   }
}
void ColorPicker::Hue::draw(C GuiPC &gpc)
{
   if(gpc.visible && visible())
   {
      D.clip(gpc.clip);
      ColorPicker &cp  =*(ColorPicker*)user;
      Rect         rect=T.rect()+gpc.offset;

      Int   steps=6;
      Flt   y    =rect.min.y;
      Color prev =RED;
      FREP(steps)
      {
         Flt   s   =Flt(i+1)/steps, yn=rect.lerpY(s);
         Color next=ColorHue(s);
         
         Vec2 lu(rect.min.x, yn), ru(rect.max.x, yn),
              ld(rect.min.x, y ), rd(rect.max.x, y );

         VI.tri(next, next, prev, lu, ru, ld);
         VI.tri(next, prev, prev, ru, rd, ld);

         y=yn;
         prev=next;
      }
      VI.end();

      rect.draw(Gui.borderColor(), false);

      ALPHA_MODE alpha=D.alpha(ALPHA_INVERT); Rect(rect.lerp(0.5f, cp._hsb.x)).extend(rect.w()/2+0.01f, 0.008f).draw(WHITE, false);
                       D.alpha(alpha       );
   }
}
void ColorPicker::Colors::update(C GuiPC &gpc)
{
   if(gpc.visible && gpc.enabled && visible() && enabled())
   {
    C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); if(!pos)REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())pos=&Touches[i].pos();
      if(   pos)
      {
         ColorPicker &cp  =*(ColorPicker*)user;
         Rect         rect=T.rect()+gpc.offset;
         if(Cuts(*pos, Rect(rect.min.x, rect.min.y, rect.max.x, rect.centerY())))cp.set(cp._old);
      }
   }
}
void ColorPicker::Colors::draw(C GuiPC &gpc)
{
   if(gpc.visible && visible())
   {
      D.clip(gpc.clip);
      ColorPicker &cp  =*(ColorPicker*)user;
      Rect         rect=T.rect()+gpc.offset;
      Rect(rect.min.x, rect.centerY(), rect.max.x, rect.max.y    ).draw(Color(cp._rgba.xyz));
      Rect(rect.min.x, rect.min.y    , rect.max.x, rect.centerY()).draw(Color(cp._old .xyz));
      rect.draw(Gui.borderColor(), false);
   }
}
/******************************************************************************/
static void ColorRed  (ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setRGB  (Vec(val       , cp._rgba.y, cp._rgba.z)); cp.toGui(false, false, true        );} // update HSB, RGBA
static void ColorGreen(ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setRGB  (Vec(cp._rgba.x, val       , cp._rgba.z)); cp.toGui(false, false, true        );} // update HSB, RGBA
static void ColorBlue (ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setRGB  (Vec(cp._rgba.x, cp._rgba.y, val       )); cp.toGui(false, false, true        );} // update HSB, RGBA
static void ColorAlpha(ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setAlpha(    val                                ); cp.toGui(false, false, false       );} // udpate      RGBA
static void ColorHue  (ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setHSB  (Vec(val       , cp._hsb.y , cp._hsb.z )); cp.toGui(true , false, false       );} // update RGB, RGBA
static void ColorSat  (ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setHSB  (Vec(cp._hsb.x , val       , cp._hsb.z )); cp.toGui(true , false, false       );} // update RGB, RGBA
static void ColorLum  (ColorPicker &cp, C Str &text) {Flt val=TextFlt(text); if(!cp._real)val/=255; cp._setHSB  (Vec(cp._hsb.x , cp._hsb.y , val       )); cp.toGui(true , false, false       );} // update RGB, RGBA
static void ColorRGBA (ColorPicker &cp, C Str &text) {Color c; c.fromHex(text);                     cp._set     (c.asVec4()                             ); cp.toGui(true , true , true , false);} // update RGB, HSB, ALPHA

static Str ColorRed  (C ColorPicker &cp) {return cp._real ? TextFlt(cp._rgba.x) : TextInt(Round(255*cp._rgba.x));}
static Str ColorGreen(C ColorPicker &cp) {return cp._real ? TextFlt(cp._rgba.y) : TextInt(Round(255*cp._rgba.y));}
static Str ColorBlue (C ColorPicker &cp) {return cp._real ? TextFlt(cp._rgba.z) : TextInt(Round(255*cp._rgba.z));}
static Str ColorAlpha(C ColorPicker &cp) {return cp._real ? TextFlt(cp._rgba.w) : TextInt(Round(255*cp._rgba.w));}
static Str ColorHue  (C ColorPicker &cp) {return cp._real ? TextFlt(cp._hsb .x) : TextInt(Round(255*cp._hsb .x));}
static Str ColorSat  (C ColorPicker &cp) {return cp._real ? TextFlt(cp._hsb .y) : TextInt(Round(255*cp._hsb .y));}
static Str ColorLum  (C ColorPicker &cp) {return cp._real ? TextFlt(cp._hsb .z) : TextInt(Round(255*cp._hsb .z));}
static Str ColorRGBA (C ColorPicker &cp) {return Color(cp._rgba).asHex();}

static void Mode(ColorPicker &cp) {cp.mode(!cp._real);}
/******************************************************************************/
ColorPicker& ColorPicker::create(C Str &name)
{
   SetTextStyle();

   Gui+=super::create(Rect_C(0, 0, 1.01f, 0.69f), S+"Color Picker"+(name.is() ? S+" - \""+name+'"' : S)); T.button[2].show();

   T+=_sat_lum.create(Rect_LU(0.04f, -0.04f                      , 0.555f, 0.555f            ), this);
   T+=_hue    .create(Rect_LU(_sat_lum.rect().ru()+Vec2(0.04f, 0), 0.05f, _sat_lum.rect().h()), this);
   T+=_color  .create(Rect_LU(_hue    .rect().ru()+Vec2(0.04f, 0), 0.14f, 0.14f              ), this);
   T+=_tnew   .create(Vec2   (_color  .rect().max.x+0.01f, _color.rect().lerpY(0.75f)), "New", &ColorPickerTS);
   T+=_told   .create(Vec2   (_color  .rect().max.x+0.01f, _color.rect().lerpY(0.25f)), "Old", &ColorPickerTS);
   T+=_mode   .create(Rect_U (Avg(_color.rect().min.x, clientWidth()-0.04f), _color.rect().min.y-0.01f, 0.2f, 0.04f), "Byte / Real").func(Mode, T).focusable(false); _mode.mode=BUTTON_TOGGLE;

   DATA_TYPE dt=DATA_INT;
  _props.del();
   ASSERT(CP_PROPS_RGB==0);
  _props.New().create("Red"  , MemberDesc(dt).setFunc(ColorRed  , ColorRed  ));
  _props.New().create("Green", MemberDesc(dt).setFunc(ColorGreen, ColorGreen));
  _props.New().create("Blue" , MemberDesc(dt).setFunc(ColorBlue , ColorBlue ));
   ASSERT(CP_PROPS_ALPHA==3);
  _props.New().create("Alpha", MemberDesc(dt).setFunc(ColorAlpha, ColorAlpha));
   ASSERT(CP_PROPS_RGBA==4);
   Property &rgba=_props.New().create("RGBA", MemberDesc(DATA_STR).setFunc(ColorRGBA, ColorRGBA)).desc("Color in RRGGBBAA Hexadecimal format");
   ASSERT(CP_PROPS_HSB==5);
  _props.New().create("Hue", MemberDesc(dt).setFunc(ColorHue, ColorHue));
  _props.New().create("Sat", MemberDesc(dt).setFunc(ColorSat, ColorSat));
  _props.New().create("Lum", MemberDesc(dt).setFunc(ColorLum, ColorLum));

   REPAO(_props).autoData(this);

   Rect r=AddProperties(_props, T, Vec2(_color.rect().min.x, _mode.rect().min.y-0.01f), 0.045f, 0.15f, &ColorPickerTS);
   rgba.textline.maxLength(8).resize(Vec2(rgba.textline.rect().h(), 0));

  _real=true; mode(false);
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
