/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// keep these methods so that the hidden Memc/_Memc can be initialized/deleted
TextCode::~TextCode() {}
TextCode:: TextCode() {}
/******************************************************************************/
void Text::zero()
{
   auto_line=AUTO_LINE_NONE;
  _code=null; _codes=0;
}
Text::Text() {zero();}
Text& Text::del()
{
   Free(_code); _codes=0;
  _text      .del  ();
   text_style.clear();
   skin      .clear();
   super::del(); zero(); return T;
}
Text& Text::create(C Str &text, C TextStylePtr &text_style)
{
   del();

     _type      =GO_TEXT;
     _visible   =true;
   T._text      =text;
   T. text_style=text_style;

   return T;
}
Text& Text::create(C Text &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type      =GO_TEXT;
         auto_line =src. auto_line ;
         text_style=src. text_style;
         skin      =src. skin      ;
        _text      =src._text      ;
         CopyN(Alloc(Free(T._code), T._codes=src._codes), src._code, src._codes);
         REP(_codes)_code[i].pos=Ptr(T._text()+UInt((CChar*)src._code[i].pos-src._text())); // adjust pos
      }
   }
   return T;
}
/******************************************************************************/
Text& Text::clear()
{
   Free(_code); _codes=0; _text.del();
   return T;
}
Text& Text::set(C Str &text)
{
   if(is()) // only if created, this prevents modifying values after destructor was already called
   {
      Free(_code); _codes=0; T._text=text;
   }
   return T;
}
Text& Text::code(C Str &code)
{
   if(is()) // only if created, this prevents modifying values after destructor was already called
   {
      Memt<TextCodeData> codes; SetTextCode(code, _text, codes);
      Free(_code); Alloc(_code, _codes=codes.elms()); codes.copyTo(_code);
   }
   return T;
}
Str Text::code()C {return GetTextCode(T(), _code, _codes);}
/******************************************************************************/
TextStyle* Text::getTextStyle()C
{
   if(text_style)return text_style();
   if(GuiSkin *skin=getSkin())return skin->text.text_style();
   return null;
}
Vec2 Text::textSize()C
{
   if(TextStyle *text_style=getTextStyle())
   {
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      TextStyleParams ts=*text_style; if(!ts.font())if(GuiSkin *skin=getSkin())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #else
    C TextStyle &ts=*text_style;
   #endif

      Flt width; Int lines=ts.textLines(_text, rect().w(), auto_line, &width);
      return Vec2(width, lines*ts.lineHeight());
   }
   return 0;
}
/******************************************************************************/
void Text::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible && T().is())
      if(TextStyle *text_style=getTextStyle())
   {
      Rect rect=T.rect()+gpc.offset;
      D.clip(gpc.clip);

   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      GuiSkin *skin; if(!text_style->font() && (skin=getSkin())) // adjust font in case it's empty and the custom skin has a different font than the Gui.skin
      {
         TextStyleParams ts=*text_style; ts.font(skin->font()); ts.drawCode(rect, _text, auto_line, _code, _codes);
      }else
   #endif
         text_style->drawCode(rect, _text, auto_line, _code, _codes);
   }
}
/******************************************************************************/
Bool Text::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.putMulti(Byte(6), auto_line)<<_text; // version
      f.putAsset(text_style.id());
      f.putAsset(skin      .id());
      f.cmpUIntV(_codes); FREP(_codes){TextCodeData &c=_code[i]; f.putMulti(c.shadow_mode, c.color_mode, c.nocode_mode, c.shadow, c.color, UInt((CChar*)c.pos-_text()));}
      return f.ok();
   }
   return false;
}
Bool Text::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 6:
      {
        _type=GO_TEXT;

         f>>auto_line>>_text;
         text_style.require(f.getAssetID(), path);
         skin      .require(f.getAssetID(), path);
        _codes=f.decUIntV(); Alloc(_code, _codes); FREP(_codes){TextCodeData &c=_code[i]; UInt pos; f.getMulti(c.shadow_mode, c.color_mode, c.nocode_mode, c.shadow, c.color, pos); c.pos=Ptr(_text()+pos);}
         if(f.ok())return true;
      }break;

      case 5:
      {
        _type=GO_TEXT;

         f>>auto_line; f._getStr2(_text);
         text_style.require(f._getAsset(), path);
         skin      .require(f._getAsset(), path);
        _codes=f.decUIntV(); Alloc(_code, _codes); FREP(_codes){TextCodeData &c=_code[i]; UInt pos; f.getMulti(c.shadow_mode, c.color_mode, c.nocode_mode, c.shadow, c.color, pos); c.pos=Ptr(_text()+pos);}
         if(f.ok())return true;
      }break;

      case 4:
      {
        _type=GO_TEXT;

         f>>auto_line; f._getStr(_text); text_style.require(f._getStr(), path); f._getStr();
         f>>_codes; Alloc(_code, _codes); FREP(_codes){TextCodeData &c=_code[i]; c.shadow_mode=TextCodeData::MODE(f.getByte()); c.color_mode=TextCodeData::MODE(f.getByte()); c.nocode_mode=TextCodeData::MODE(f.getByte()); f>>c.shadow>>c.color; c.pos=Ptr(_text()+f.getUInt());}
         if(f.ok())return true;
      }break;

      case 3:
      {
        _type=GO_TEXT;

         f>>auto_line; f._getStr(_text); text_style.require(f._getStr(), path); f._getStr();
         f>>_codes; Alloc(_code, _codes); FREP(_codes){TextCodeData &c=_code[i]; c.shadow_mode=TextCodeData::MODE(f.getByte()); f.skip(3); c.color_mode=TextCodeData::MODE(f.getByte()); f.skip(3); c.nocode_mode=TextCodeData::DEFAULT; f>>c.shadow>>c.color; c.pos=Ptr(_text()+f.getUInt());}
         if(f.ok())return true;
      }break;

      case 2:
      {
        _type=GO_TEXT;

         f>>auto_line; f._getStr(_text); text_style.require(f._getStr(), path); f._getStr();
         if(f.ok())return true;
      }break;

      case 1:
      {
        _type=GO_TEXT;

        _text=f._getStr16(); text_style.require(f._getStr16(), path); f._getStr16();
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_TEXT;

        _text=f._getStr8(); text_style.require(f._getStr8(), path); f._getStr8();
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
