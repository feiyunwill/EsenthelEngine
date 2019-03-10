/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define SKIP_SPACE 1 // if skip drawing space that was at the end of the line but didn't fit
#define CC4_TXDS CC4('T','X','D','S')
#define            DefaultShade 230
static const Color DefaultSelectionColor(51, 153, 255, 64);
Memc<TextLineSplit8 > Tls8 ;
Memc<TextLineSplit16> Tls16;
DEFINE_CACHE(TextStyle, TextStyles, TextStylePtr, "Text Style");
/******************************************************************************/
static Int Length(CChar8 *text, AUTO_LINE_MODE auto_line, Int max_length)
{
   Int length=-1;
   MAX (max_length, 1); // always allow drawing at least 1 character
   FREP(max_length+1) // iterate those characters (and 1 more) to check if within this range (or right after) we have special characters
   {
      Char8 c=text[i];
                                    if(c=='\0' || c=='\n')return i; // if we've encountered end or new line, then we have to stop there, return length exclusive because we won't draw this character
      if(auto_line!=AUTO_LINE_SPLIT)if(c==' '            )length=i; // if we want to avoid splitting words into multiple lines, then remember last space that's in this range
   }
   if(length>=0)length+=(length<max_length);else // to support drawing cursors and selections, draw the space too (increase length to include it) if it can fit in the 'max_length' range
   { // if nothing was found
      length=max_length; // draw all characters that fit in this space
      if(auto_line==AUTO_LINE_SPACE)for(; ; length++) // if we're not splitting words then we have to keep going forward outside of the range that fits in the space until we find first character that can split
      {
         Char8 c=text[length];
         if(c=='\0'
         || c=='\n'
         || c==' ' )break; // and find a first character that can split text, set length exclusive because we won't draw this character
      }
   }
   return length;
}
static Int Length(CChar *text, AUTO_LINE_MODE auto_line, Int max_length)
{
   Int length=-1;
   MAX (max_length, 1); // always allow drawing at least 1 character
   FREP(max_length+1) // iterate those characters (and 1 more) to check if within this range (or right after) we have special characters
   {
      Char c=text[i];
                                    if(c=='\0' || c=='\n')return i; // if we've encountered end or new line, then we have to stop there, return length exclusive because we won't draw this character
      if(auto_line!=AUTO_LINE_SPLIT)if(c==' '            )length=i; // if we want to avoid splitting words into multiple lines, then remember last space that's in this range
   }
   if(length>=0)length+=(length<max_length);else // to support drawing cursors and selections, draw the space too (increase length to include it) if it can fit in the 'max_length' range
   { // if nothing was found
      length=max_length; // draw all characters that fit in this space
      if(auto_line==AUTO_LINE_SPACE)for(; ; length++) // if we're not splitting words then we have to keep going forward outside of the range that fits in the space until we find first character that can split
      {
         Char c=text[length];
         if(c=='\0'
         || c=='\n'
         || c==' ' )break; // and find a first character that can split text, set length exclusive because we won't draw this character
      }
   }
   return length;
}
/******************************************************************************/
void Set(MemPtr<TextLineSplit8> tls, CChar8 *text, C TextStyleParams &text_style, Flt width, AUTO_LINE_MODE auto_line) // have to set at least one line to support drawing cursor when text is empty
{
   tls.clear();
   if(CChar8 *t=text)switch(auto_line)
   {
      default: // AUTO_LINE_NONE
      {
         tls.New().set(t, -1, 0);
         for(Int length=0; t[0]; t++)
         {
            if(t[0]!='\n')length++;else
            {
               tls.last().length=length; length=0;
               tls.New ().set(t+1, -1, (t+1)-text);
            }
         }
      }break;

      case AUTO_LINE_SPACE:
      case AUTO_LINE_SPACE_SPLIT:
      case AUTO_LINE_SPLIT:
      {
         for(width+=EPS; ; ) // needed when drawing text in 'width' calculated from 'textWidth'
         {
            Int length=Length(t, auto_line, text_style.textPos(t, width, false)); // check how many characters can we fit in this space
            tls.New().set(t, length, t-text);
            t+=length;
            if(*t=='\0')break;
            if(*t=='\n'/* || !length*/ // if next character is a new line, or we didn't advance ('length' doesn't need to be checked because it can be 0 only for '\0' or '\n' which are already checked)
            || SKIP_SPACE && *t==' ' && text_style.spacing!=SPACING_CONST
            )t++; // then skip this character 
         }
      }break;
   }else tls.New().set(null, 0, 0); // have to set at least one line
}
void Set(MemPtr<TextLineSplit16> tls, CChar *text, C TextStyleParams &text_style, Flt width, AUTO_LINE_MODE auto_line) // have to set at least one line to support drawing cursor when text is empty
{
   tls.clear();
   if(CChar *t=text)switch(auto_line)
   {
      default: // AUTO_LINE_NONE
      {
         tls.New().set(t, -1, 0);
         for(Int length=0; t[0]; t++)
         {
            if(t[0]!='\n')length++;else
            {
               tls.last().length=length; length=0;
               tls.New ().set(t+1, -1, (t+1)-text);
            }
         }
      }break;
      
      case AUTO_LINE_SPACE:
      case AUTO_LINE_SPACE_SPLIT:
      case AUTO_LINE_SPLIT:
      {
         for(width+=EPS; ; ) // needed when drawing text in 'width' calculated from 'textWidth'
         {
            Int length=Length(t, auto_line, text_style.textPos(t, width, false)); // check how many characters can we fit in this space
            tls.New().set(t, length, t-text);
            t+=length;
            if(*t=='\0')break;
            if(*t=='\n'/* || !length*/ // if next character is a new line, or we didn't advance ('length' doesn't need to be checked because it can be 0 only for '\0' or '\n' which are already checked)
            || SKIP_SPACE && *t==' ' && text_style.spacing!=SPACING_CONST
            )t++; // then skip this character 
         }
      }break;
   }else tls.New().set(null, 0, 0); // have to set at least one line
}
static Bool SetLine(TextLineSplit16 &tls, CChar *text, C TextStyleParams &text_style, Flt width, AUTO_LINE_MODE auto_line, Int line)
{
   if(CChar *t=text)if(line>=0)for(width+=EPS; ; ) // needed when drawing text in 'width' calculated from 'textWidth'
   {
      Int length=Length(t, auto_line, (auto_line==AUTO_LINE_NONE) ? INT_MAX-1 : text_style.textPos(t, width, false)); // check how many characters can we fit in this space, use "INT_MAX-1" so "FREP(max_length+1)" inside 'Length' can finish
      if(!line){tls.set(t, length, t-text); return true;}
      line--;
      t+=length;
      if(*t=='\0')break;
      if(*t=='\n'/* || !length*/ // if next character is a new line, or we didn't advance ('length' doesn't need to be checked because it can be 0 only for '\0' or '\n' which are already checked)
      || SKIP_SPACE && *t==' ' && text_style.spacing!=SPACING_CONST && auto_line!=AUTO_LINE_NONE
      )t++; // then skip this character 
   }
   return false;
}
Vec2 TextStyleParams::textIndex(CChar *text, Int index, Flt width, AUTO_LINE_MODE auto_line)C
{
   if(index<=0 || !text)return 0;
   CChar *t=text; Int line=0; for(width+=EPS; ; line++) // needed when drawing text in 'width' calculated from 'textWidth'
   {
      Int length=Length(t, auto_line, (auto_line==AUTO_LINE_NONE) ? INT_MAX-1 : textPos(t, width, false)); // check how many characters can we fit in this space, use "INT_MAX-1" so "FREP(max_length+1)" inside 'Length' can finish
      Int offset=t-text;
      t+=length;
   #if 1 // this check will set the cursor for the next line for split lines
      if(*t!='\n')length--;
   #endif
      if(index<=offset+length || *t=='\0')
         return Vec2(textWidth(text+offset, index-offset), line*lineHeight());
      if(*t=='\n'/* || !length*/ // if next character is a new line, or we didn't advance ('length' doesn't need to be checked because it can be 0 only for '\0' or '\n' which are already checked)
      || SKIP_SPACE && *t==' ' && spacing!=SPACING_CONST && auto_line!=AUTO_LINE_NONE
      )t++; // then skip this character 
   }
}
/******************************************************************************/
Font* TextStyleParams::getFont()C
{
   if(_font)return _font;
   if(GuiSkin *skin=Gui.skin())return skin->font(); // copy to temp 'skin' in case of multi-thread issues
   return null;
}
Flt TextStyleParams::posY(Flt y)C
{
   if(C Font *font=getFont())
   {
      Flt ysize      =size .y/font->height(), // height of 1 font texel
          y_align_mul=align.y*0.5f+0.5f;
       y+=size.y*y_align_mul+ysize*font->paddingT();
   }
   return y;
}
void TextStyleParams::posY(Flt y, Vec2 &range)C
{
   if(C Font *font=getFont())
   {
      Flt ysize      =size .y/font->height(), // height of 1 font texel
          y_align_mul=align.y*0.5f+0.5f;
      y+=size.y*y_align_mul;
      range.x=y-size.y-ysize*font->paddingB(); // min
      range.y=y       +ysize*font->paddingT(); // max
   }
}
void TextStyleParams::posYI(Flt y, Vec2 &range)C
{
   if(C Font *font=getFont())
   {
      Flt ysize      =size .y/font->height(), // height of 1 font texel
          y_align_mul=align.y*0.5f+0.5f;
      y-=size.y*y_align_mul;
      range.x=y       -ysize*font->paddingT(); // min
      range.y=y+size.y+ysize*font->paddingB(); // max
   }
}
/******************************************************************************/
void TextStyleParams::setPerPixelSize()
{
   if(C Font *font=getFont())size=D.pixelToScreenSize(font->height());
}
Flt TextStyleParams::textWidth(C Str &str, Int max_length)C
{
   if(Int length=str.length())
      if(C Font *font=getFont())
   {
      if(max_length>=0 && max_length<length){if(!max_length)return 0; length=max_length;}

      Flt    xsize=size.x/font->height(),
             space=size.x*T.space.x;
      Int    base_chars, width=font->textWidth(base_chars, spacing, str, max_length);
      return width*xsize + space*(base_chars-(spacing!=SPACING_CONST)); // calculate spacing only between base characters (ignoring combining), we're including spacing between the characters, so we need to set 1 less (except the case for SPACING_CONST where we need to have spacing for all characters)
   }
   return 0;
}
Flt TextStyleParams::textWidth(C Str8 &str, Int max_length)C
{
   if(Int length=str.length())
      if(C Font *font=getFont())
   {
      if(max_length>=0 && max_length<length){if(!max_length)return 0; length=max_length;}

      Flt    xsize=size.x/font->height(),
             space=size.x*T.space.x;
      Int    base_chars, width=font->textWidth(base_chars, spacing, str, max_length);
      return width*xsize + space*(base_chars-(spacing!=SPACING_CONST)); // calculate spacing only between base characters (ignoring combining), we're including spacing between the characters, so we need to set 1 less (except the case for SPACING_CONST where we need to have spacing for all characters)
   }
   return 0;
}
Flt TextStyleParams::textWidth(CChar *text, Int max_length)C
{
   if(Int length=Length(text))
      if(C Font *font=getFont())
   {
      if(max_length>=0 && max_length<length){if(!max_length)return 0; length=max_length;}

      Flt    xsize=size.x/font->height(),
             space=size.x*T.space.x;
      Int    base_chars, width=font->textWidth(base_chars, spacing, text, max_length);
      return width*xsize + space*(base_chars-(spacing!=SPACING_CONST)); // calculate spacing only between base characters (ignoring combining), we're including spacing between the characters, so we need to set 1 less (except the case for SPACING_CONST where we need to have spacing for all characters)
   }
   return 0;
}
Flt TextStyleParams::textWidth(CChar8 *text, Int max_length)C
{
   if(Int length=Length(text))
      if(C Font *font=getFont())
   {
      if(max_length>=0 && max_length<length){if(!max_length)return 0; length=max_length;}

      Flt    xsize=size.x/font->height(),
             space=size.x*T.space.x;
      Int    base_chars, width=font->textWidth(base_chars, spacing, text, max_length);
      return width*xsize + space*(base_chars-(spacing!=SPACING_CONST)); // calculate spacing only between base characters (ignoring combining), we're including spacing between the characters, so we need to set 1 less (except the case for SPACING_CONST where we need to have spacing for all characters)
   }
   return 0;
}
/******************************************************************************/
Int TextStyleParams::textPos(CChar8 *text, Flt x, Bool round)C
{
   Int pos=0;
   if(Is(text))
      if(C Font *font=getFont())
   {
      Flt space=size.x*T.space.x;
      if(spacing==SPACING_CONST)
      {
         x/=space; if(round)x+=0.5f;
         pos=Trunc(x);
      #if 0 // fast
         Clamp(pos, 0, Length(text));
      #else // CHARF_COMBINING
         CChar8 *start=text; for(Int base_chars=0; ; base_chars++)
         {
            Char8 c=*text; if(!c || base_chars>=pos){pos=text-start; break;}
         skip:
            Char8 next=*++text;
            if(CharFlagFast(next)&CHARF_COMBINING)goto skip;
         }
      #endif
      }else
      for(Flt xsize=size.x/font->height(); ; )
      {
         Char8 c=*text; if(!c)break;
         CChar8 *start=text;
      skip1:
         Char8 next=*++text;
         if(CharFlagFast(next)&CHARF_COMBINING)goto skip1;
         Flt w=font->charWidth(c, next, spacing)*xsize;
         if(x<=(round ? w*0.5f : w))break;
         x-=w+space;
         pos+=text-start; // advance by how many characters were processed
      }
   }
   return pos;
}
Int TextStyleParams::textPos(CChar *text, Flt x, Bool round)C
{
   Int pos=0;
   if(Is(text))
      if(C Font *font=getFont())
   {
      Flt space=size.x*T.space.x;
      if(spacing==SPACING_CONST)
      {
         x/=space; if(round)x+=0.5f;
         pos=Trunc(x);
      #if 0 // fast
         Clamp(pos, 0, Length(text));
      #else // CHARF_COMBINING
         CChar *start=text; for(Int base_chars=0; ; base_chars++)
         {
            Char c=*text; if(!c || base_chars>=pos){pos=text-start; break;}
         skip:
            Char next=*++text;
            if(CharFlagFast(next)&CHARF_COMBINING)goto skip;
         }
      #endif
      }else
      for(Flt xsize=size.x/font->height(); ; )
      {
         Char c=*text; if(!c)break;
         CChar *start=text;
      skip1:
         Char next=*++text;
         if(CharFlagFast(next)&CHARF_COMBINING)goto skip1;
         Flt w=font->charWidth(c, next, spacing)*xsize;
         if(x<=(round ? w*0.5f : w))break;
         x-=w+space;
         pos+=text-start; // advance by how many characters were processed
      }
   }
   return pos;
}
Int TextStyleParams::textPos(CChar *text, Flt x, Flt y, Bool round, Flt width, AUTO_LINE_MODE auto_line, Bool &eol)C
{
   Int line=Trunc(y/lineHeight()); if(line<0){eol=false; return 0;}
   TextLineSplit16 tls; if(!SetLine(tls, text, T, width, auto_line, line)){eol=true; return Length(text);}
   Int pos=textPos(text+tls.offset, x, round);
   if(eol=(pos>=tls.length))pos=tls.length; // yes this must check ">=" and not ">" because we need to set "eol=(pos>=tls.length)" because we need it for correct double-clicking word selection
   return tls.offset+pos;
}
/******************************************************************************/
Int TextStyleParams::textLines(CChar8 *text, Flt width, AUTO_LINE_MODE auto_line, Flt *actual_width)C
{
   Memt<TextLineSplit8> tls; Set(tls, text, T, width, auto_line);
   if(actual_width)
   {
      *actual_width=0; REPA(tls){TextLineSplit8 &t=tls[i]; MAX(*actual_width, textWidth(t.text, t.length));}
   }
   return tls.elms();
}
Int TextStyleParams::textLines(CChar *text, Flt width, AUTO_LINE_MODE auto_line, Flt *actual_width)C
{
   Memt<TextLineSplit16> tls; Set(tls, text, T, width, auto_line);
   if(actual_width)
   {
      *actual_width=0; REPA(tls){TextLineSplit16 &t=tls[i]; MAX(*actual_width, textWidth(t.text, t.length));}
   }
   return tls.elms();
}
/******************************************************************************/
TextStyleParams& TextStyleParams::resetColors(Bool gui)
{
   // use 'Gui.skin.text.text_style', 'Gui.skin.text_style' if available
   if(GuiSkin *skin=Gui.skin())
      if(TextStyle *text_style=(gui ? skin->text.text_style() : skin->text_style()))
   {
      shadow   =text_style->shadow;
      shade    =text_style->shade;
      color    =text_style->color;
      selection=text_style->selection;
      return T;
   }

   // otherwise set defaults
   shadow   =255;
   shade    =DefaultShade;
   color    =WHITE;
   selection=DefaultSelectionColor;
   return T;
}
TextStyleParams& TextStyleParams::reset(Bool gui)
{
   // use 'Gui.skin.text.text_style', 'Gui.skin.text_style' if available
   if(GuiSkin *skin=Gui.skin())
      if(TextStyle *text_style=(gui ? skin->text.text_style() : skin->text_style()))
         {T=*text_style; return T;}

   // otherwise set defaults
   pixel_align=true;
   align.set(0    , 0    );
   size .set(0.08f, 0.08f);
   space.set(0.06f, 1    );
   spacing  =SPACING_NICE;
   shadow   =255;
   shade    =DefaultShade;
   color    =WHITE;
   selection=DefaultSelectionColor;
  _font     =null; // keep as null to always use the current value of 'Gui.skin.font'
   edit     =null;

   return T;
}
/******************************************************************************/
TextStyle& TextStyle::font       (C FontPtr &font) {_font=font; super::font(_font()); return T;}
TextStyle& TextStyle::resetColors(  Bool     gui ) {super::resetColors(gui); return T;}
TextStyle& TextStyle::reset      (  Bool     gui ) {super::reset      (gui); if(GuiSkin *skin=Gui.skin())if(TextStyle *text_style=(gui ? skin->text.text_style() : skin->text_style())){font(text_style->font()); return T;} font(null); return T;} // copy to temp 'skin' and 'text_style' in case of multi-thread issues, call 'font' to always make sure that '_font' from 'TextStyle' and 'TextStyleParams' is synchronized
TextStyle::TextStyle             (               ) {                         if(GuiSkin *skin=Gui.skin())if(TextStyle *text_style=                                 skin->text_style() ){font(text_style->font()); return  ;} font(null);          } // copy to temp 'skin' and 'text_style' in case of multi-thread issues, call 'font' to always make sure that '_font' from 'TextStyle' and 'TextStyleParams' is synchronized
/******************************************************************************/
// DRAW
/******************************************************************************/
struct TextInput
{
   CChar8 *t8 ;
   CChar  *t16;

   Bool is()C {return Is(t8) || Is(t16);}
   Char c ()C {return t8 ? Char8To16Fast(*t8) : t16 ? *t16 : 0;} // we can assume that Str was already initialized
   CPtr p ()C {return t8 ? CPtr(t8) : CPtr(t16);}

   void operator++(int) {if(t8)t8++; if(t16)t16++;}

   TextInput(CChar8 *t) {t8 =t; t16=null;}
   TextInput(CChar  *t) {t16=t; t8 =null;}
};
static Int CompareCode(C TextCodeData &code, C CPtr &pos) {return ComparePtr(code.pos, pos);}
static Int    FindCode(C TextCodeData *code, Int codes, CPtr cur_pos)
{
   Int    index;
   Bool   found=BinarySearch(code, codes, cur_pos, index, CompareCode);
   if(    found)for(; index>0 && code[index-1].pos==cur_pos; )index--; // in case if many codes would point to the same place
   return found ? index : index-1; // if we haven't found at exact position, then we need to grab the one before selected position like this: "<code>some tex|t here" when starting drawing of | we need to use the <code>
}
static void SetCode(C TextCodeData *code, C TextStyleParams &text_style, Bool sub_pixel)
{
   VI.flush();
   if(sub_pixel){             Color c=((code && code-> color_mode!=TextCodeData::DEFAULT) ? code->color  : text_style.color )        ; D.alphaFactor(c); c.r=c.g=c.b=c.a; VI.color(c);}
   else         {VI.color            (((code && code-> color_mode!=TextCodeData::DEFAULT) ? code->color  : text_style.color )       );
                 Sh.h_FontShadow->set(((code && code->shadow_mode!=TextCodeData::DEFAULT) ? code->shadow : text_style.shadow)/255.0f);}
}
void DrawKeyboardCursor(C Vec2 &pos, Flt height)
{
   ALPHA_MODE alpha=D.alpha(ALPHA_INVERT); Rect_U(pos, height/11.0f, height).draw(WHITE);
                    D.alpha(alpha       );
}
void DrawKeyboardCursorOverwrite(C Vec2 &pos, Flt height, C TextStyleParams &text_style, Char chr)
{
   if(C Font *font=text_style.getFont())
   {
      Flt w=((text_style.spacing==SPACING_CONST) ? text_style.colWidth() : chr ? text_style.size.x/font->height()*font->charWidth(chr) : height*0.5f),
      min_w=height/5.0f; // use min width because some characters are just too thin
      ALPHA_MODE alpha=D.alpha(ALPHA_INVERT); Rect_LU(pos, w, height).extendX(Max((min_w-w)*0.5f, 0)).draw(WHITE); // extend both left and right
                       D.alpha(alpha       );
   }
}
void TextStyleParams::drawMain(Flt x, Flt y, TextInput ti, Int max_length, C TextCodeData *code, Int codes, Int offset)C
{
   Int cur=SIGN_BIT, sel=SIGN_BIT; ASSERT(Int(SIGN_BIT)<0); // these must be negative
   if(edit && App.active()){if(edit->cur>=0)cur=edit->cur-offset; if(edit->sel>=0)sel=edit->sel-offset;} // set only if valid (so we keep SIGN_BIT otherwise)
   if(ti.is() || cur>=0 || sel>=0) // we have some text to draw, or we may encounter cursor or selection ahead
      if(C Font *font=getFont())
   {
      Int pos=0, cur_chr=-1;
      Flt cur_x, sel_x;
   #if DEBUG
      cur_x=sel_x=0; // to prevent run-time check exceptions on debug mode
   #endif

      // set
      Flt xsize       =size.x/font->height(), // width  of 1 font texel
          ysize       =size.y/font->height(), // height of 1 font texel
          space       =size.x*T.space.x,
          x_align_mul =align.x*0.5f-0.5f,
          y_align_mul =align.y*0.5f+0.5f,
          total_width =(Equal(x_align_mul, 0) ? 0 : ti.t8 ? textWidth(ti.t8, max_length) : textWidth(ti.t16, max_length)), // don't calculate text width when not needed
          total_height=size.y;
          x          +=total_width *x_align_mul - xsize*font->paddingL();
          y          +=total_height*y_align_mul + ysize*font->paddingT();

      if(spacing==SPACING_CONST)x+=space*0.5f; // put the cursor at the center where characters will be drawn, later this will be repositioned by half char width
      Vec2 p(x, y); if(pixel_align)D.alignScreenToPixel(p);

      // draw
      if(ti.is())
      {
         Flt xsize_2=xsize*0.5f;

         // texture bias
         {
         #if DX9
            U32 bias=(U32&)D._font_sharpness; bias^=SIGN_BIT; D3D->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, bias);
         #elif DX11
            // bias is set using 'SamplerFont' which is accessed in 'Font' shader
         #elif GL
            // bias is set using Font.setGLFont and Image.setGLFont
         #endif
         }

         // sub-pixel rendering
         ALPHA_MODE alpha;
         Bool       sub_pixel=font->_sub_pixel;
         if(sub_pixel){alpha=D.alpha(Renderer.inside() ? ALPHA_FONT_DEC : ALPHA_FONT); VI.color2(TRANSPARENT);}else // if drawing text while rendering, then decrease the alpha channel (glow)
         if(Renderer.inside())D.alpha(ALPHA_BLEND_DEC); // if drawing text while rendering, then decrease the alpha channel (glow), but don't bother to restore it, as in Rendering, alpha blending is always set for each call

         Int            cur_code_index=FindCode(code, codes, ti.p());
       C TextCodeData * cur_code      =(InRange(cur_code_index  , codes) ? &code[cur_code_index  ] : null),
                      *next_code      =(InRange(cur_code_index+1, codes) ? &code[cur_code_index+1] : null);

         // font params
         Flt  contrast;
         Byte lum=color.lum(); if(!lum)contrast=1;else
         {
            Flt pixels =Renderer.screenToPixelSize(size).min();
            if( pixels>=32)contrast=1;else
            {
               contrast=32/pixels;
               contrast=Sqrt(contrast); // or alternative: contrast=Log2(contrast+1);
               contrast=Max(1, contrast);
               contrast=Lerp(1.0f, contrast, lum/255.0f);
            }
         }
         Sh.h_FontContrast->set(contrast    );
         Sh.h_FontShade   ->set(shade/255.0f);
         SetCode(cur_code, T, sub_pixel);

         // font depth
         if(D._text_depth) // apply new state
         {
            D .depthLock (true );
            D .depthWrite(false); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
            VI.shader    (Sh.h_FontD);
         }

         for(Char c=ti.c(); c; pos++)
         {
            if(!max_length--)break;

            // check if encountered next code
            for(; next_code && ti.p()>=next_code->pos; )
            {
                cur_code_index++;
                cur_code=next_code;
               next_code=(InRange(cur_code_index+1, codes) ? &code[cur_code_index+1] : null);
               SetCode(cur_code, T, sub_pixel);
            }

            // next character
            Char n=(ti.t8 ? Char8To16Fast(*++ti.t8) : *++ti.t16);

            // draw character
            if(c!=' ') // don't draw space sign, potentially we could check it below using "fc.height" instead, however this is faster
            {
               UInt index=font->_wide_to_font[U16(c)]; if(InRange(index, font->_chrs))
               {
                C Font::Chr &fc=font->_chrs[index];
                //if(fc.height) // potentially we could check for empty characters (full width space, half width space, nbsp, however in most cases we will have something to draw, so this check will slow down things
                  {
                     Vec2 c_pos=p;
                     if(spacing==SPACING_CONST)c_pos.x-=xsize_2*fc.width ; // move back by half of the character width
                                               c_pos.y-=ysize  *fc.offset;
                     if(pixel_align)D.alignScreenXToPixel(c_pos.x);

                     VI.image(&font->_images[fc.image]);
                     Rect_LU rect(c_pos, xsize*fc.width_padd, ysize*fc.height_padd);
                     if(sub_pixel)VI.imagePart(rect, fc.tex);
                     else         VI.font     (rect, fc.tex);

                     // combining
                     UInt flag=CharFlagFast(n); if(flag&CHARF_COMBINING)
                     {
                        c_pos.x+=xsize_2*fc.width; // after 'c_pos' was pixel aligned, move by half of the character width to put it at centerX of 'c' character
                        c_pos.y =p.y; // reset Y pos
                        if(flag&CHARF_STACK)c_pos.y+=ysize*font->paddingB(); // skip shadow padding at the bottom, because next character is drawn after base character and on top, so its bottom shadow must not overlap the base

                     again:
                        UInt index=font->_wide_to_font[U16(n)]; if(InRange(index, font->_chrs))
                        {
                           if(!max_length--)break;

                           // update positions
                           if(cur==pos){cur_x=p.x; cur_chr=c;}
                           if(sel==pos){sel_x=p.x;}
                           pos++;

                         C Font::Chr &fc=font->_chrs[index];

                           Vec2 n_pos=c_pos;
                           n_pos.x-=xsize_2*fc.width; // move back by half of the character width
                           n_pos.y-=ysize  *fc.offset;
                           if(pixel_align)D.alignScreenXToPixel(n_pos.x);

                           VI.image(&font->_images[fc.image]);
                           Rect_LU rect(n_pos, xsize*fc.width_padd, ysize*fc.height_padd);
                           if(sub_pixel)VI.imagePart(rect, fc.tex);
                           else         VI.font     (rect, fc.tex);

                           n=(ti.t8 ? Char8To16Fast(*++ti.t8) : *++ti.t16);
                           flag=CharFlagFast(n); if(flag&CHARF_COMBINING) // if next character is combining too
                           {
                              if(flag&CHARF_STACK)c_pos.y+=ysize*fc.height_padd; // move position higher, to stack combining characters on top of each other (needed for THAI)
                              goto again;
                           }
                        }
                     }
                  }
               }
            }

            // update positions
            if(cur==pos){cur_x=p.x; cur_chr=c;}
            if(sel==pos){sel_x=p.x;}
            p.x+=space + xsize*font->charWidth(c, n, spacing);
            c=n;
         }
         VI.end();

         // texture bias
         {
         #if DX9
            D3D->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, 0);
         #elif DX11
            // bias is set using 'SamplerFont' which is accessed in 'Font' shader
         #elif GL
            // bias is set using 'Font.setGLFont' and 'Image.setGLFont'
         #endif
         }

         // sub-pixel
         if(sub_pixel)D.alpha(alpha);
         
         // font depth
         if(D._text_depth) // reset old state
         {
            D.depthUnlock();
            D.depthWrite (true);
         }
      }

      // selection
      if(sel!=SIGN_BIT && cur!=SIGN_BIT) // have some selection
      {
         Int min, max; Rect rect;
         if(sel<cur)
         {
            min=sel; rect.min.x=sel_x;
            max=cur; rect.max.x=cur_x;
         }else
         {
            min=cur; rect.min.x=cur_x;
            max=sel; rect.max.x=sel_x;
         }
         if(max>=0 && min<pos) // if selection intersects with this text
         {
            if(min<=0  )rect.min.x=  x; // if minimum is before the start of this text, then use starting position
            if(max>=pos)rect.max.x=p.x; // if maximum is after  the end   of this text, then use current  position
            if(spacing==SPACING_CONST){rect.min.x-=space*0.5f; rect.max.x-=space*0.5f;} // revert what we've applied
            Flt h=lineHeight();
            rect. setY(y-h, p.y); D.alignScreenYToPixel(rect.min.y); // use "y-h" instead of "p.y-h" to get exact value of the next line
            rect.moveY((h-size.y)/2); // adjust rectangle so text is at selection center
            rect.draw(selection);
         }
      }

      // cursor
      if(cur==pos){Char c=ti.c(); if(!c || c=='\n' || (SKIP_SPACE && c==' ' && spacing!=SPACING_CONST /*&& auto_line!=AUTO_LINE_NONE*/)){cur_x=p.x; cur_chr=0;}} // allow drawing cursor at the end only if it's followed by new line or null (this prevents drawing cursors 2 times for split lines - at the end of 1st line and at the beginning of 2nd line)
      if(cur_chr>=0 && !Kb._hidden)
      {
         if(spacing==SPACING_CONST)cur_x-=space*0.5f; // revert what we've applied
         if(!edit->overwrite)DrawKeyboardCursor         (Vec2(cur_x, p.y), total_height);
         else                DrawKeyboardCursorOverwrite(Vec2(cur_x, p.y), total_height, T, Char(cur_chr));
      }
   }
}
struct FontDraw
{
   struct Rect2
   {
      Rect src, dest;
   };
   Rect2  rect[65536/SIZE(Rect2)];
   Int    rects, mip_map;
   Flt    contrast, shadow;
   Vec4   color;
   RectI  clip;
   Bool   sub_pixel;
 C Image *src;
   Image &dest;

   FontDraw(Image &dest, Int mip_map, Bool sub_pixel) : dest(dest), mip_map(Max(0, mip_map)), sub_pixel(sub_pixel) {rects=0; clip.set(0, 0, dest.w(), dest.h()); clear();}
  ~FontDraw() {unlock();}

   void setCode(C TextCodeData *code, C TextStyleParams &text_style)
   {
      flush();
    //if(sub_pixel){Color c=((code && code-> color_mode!=TextCodeData::DEFAULT) ? code->color  : text_style.color )        ; D.alphaFactor(c); c.r=c.g=c.b=c.a; VI.color(c);}else
                   {color =((code && code-> color_mode!=TextCodeData::DEFAULT) ? code->color  : text_style.color ).asVec4();
                    shadow=((code && code->shadow_mode!=TextCodeData::DEFAULT) ? code->shadow : text_style.shadow)/255.0f  ;}
   }
   void flush()
   {
      FREP(rects)
      {
       C Rect2 &r=rect[i];
         RectI dest;
         dest.set(Trunc(r.dest.min.x), Trunc(r.dest.max.y), Ceil(r.dest.max.x), Ceil(r.dest.min.y));
         dest&=clip;
         Vec2 mul_add_x((r.src.max.x-r.src.min.x)                         / (r.dest.max.x - r.dest.min.x)  * src->lw(),
         (r.src.min.x + (r.src.max.x-r.src.min.x) * (0.5f - r.dest.min.x) / (r.dest.max.x - r.dest.min.x)) * src->lw() - 0.5f);
         Vec2 mul_add_y((r.src.min.y-r.src.max.y)                         / (r.dest.max.y - r.dest.min.y)  * src->lh(),
         (r.src.max.y + (r.src.min.y-r.src.max.y) * (0.5f - r.dest.min.y) / (r.dest.max.y - r.dest.min.y)) * src->lh() - 0.5f);

         for(Int y=dest.min.y; y<dest.max.y; y++)
         {
          //Flt fy=LerpR(r.dest.min.y, r.dest.max.y, y+0.5f); // fy=(y+0.5f - r.dest.min.y)/(r.dest.max.y - r.dest.min.y)
          //Flt ty=Lerp (r.src .max.y, r.src .min.y, fy)*src->lh()-0.5f; // ty=(r.src.max.y + (r.src.min.y-r.src.max.y) * fy)*src->lh()-0.5f
            // ty=(r.src.max.y + (r.src.min.y-r.src.max.y) * (y+0.5f - r.dest.min.y)/(r.dest.max.y - r.dest.min.y))*src->lh()-0.5f
            // ty=y *                (r.src.min.y-r.src.max.y)                         / (r.dest.max.y - r.dest.min.y)  * src->lh()
            //      + (r.src.max.y + (r.src.min.y-r.src.max.y) * (0.5f - r.dest.min.y) / (r.dest.max.y - r.dest.min.y)) * src->lh() - 0.5f
            Flt ty=y*mul_add_y.x+mul_add_y.y;
            for(Int x=dest.min.x; x<dest.max.x; x++)
            {
             //Flt fx=LerpR(r.dest.min.x, r.dest.max.x, x+0.5f);
             //Flt tx=Lerp (r.src .min.x, r.src .max.x, fx)*src->lw()-0.5f;
               Flt tx=x*mul_add_x.x+mul_add_x.y;

               Vec4 c=src->colorFLinear(tx, ty);

               Flt  a=Min(c.y*contrast, 1), // font opacity, scale up by 'contrast' to improve quality when font is very small
                    s=    c.w*shadow      ; // font shadow

               // Flt final_alpha=1-(1-s)*(1-a);
               // 1-(1-s)*(1-a)
               // 1-(1-a-s+sa)
               // 1-1+a+s-sa
               // a + s - s*a
               Flt final_alpha=a+s-s*a;

               // ALPHA_MERGE:
               Vec4 out;
               out.xyz=color.xyz*(a*color.w);
               out.w  =color.w*final_alpha;

               T.dest.merge(x, y, out);
            }
         }
      }
      rects=0;
   }
   void clear () {src=null;}
   void unlock() {if(src){flush(); src->unlock();}}
   Bool draw(C Image &image, C Rect &rect_src, C Rect &rect_dest)
   {
      if(src!=&image)
      {
         unlock();
         if(image.lockRead(Min(mip_map, image.mipMaps()-1)))
         {
            src=&image;
         }else
         {
            clear(); return false;
         }
      }
      if(!InRange(rects, rect))flush();
      Rect2 &r=rect[rects++];
      r.src =rect_src;
      r.dest=rect_dest;
      return true;
   }
};
void TextStyleParams::drawMainSoft(Image &image, Flt x, Flt y, TextInput ti, Int max_length, C TextCodeData *code, Int codes, Int offset)C
{
   if(ti.is()) // we have some text to draw
      if(C Font *font=getFont())
   {
      // set
      Flt xsize       = size.x/font->height(), // width  of 1 font texel
          ysize       =-size.y/font->height(), // height of 1 font texel
          space       = size.x*T.space.x,
          x_align_mul =align.x*0.5f-0.5f,
          y_align_mul =align.y*0.5f+0.5f,
          total_width =(Equal(x_align_mul, 0) ? 0 : ti.t8 ? textWidth(ti.t8, max_length) : textWidth(ti.t16, max_length)), // don't calculate text width when not needed
          total_height=-size.y;
          x          +=total_width *x_align_mul - xsize*font->paddingL();
          y          +=total_height*y_align_mul + ysize*font->paddingT();

      if(spacing==SPACING_CONST)x+=space*0.5f; // put the cursor at the center where characters will be drawn, later this will be repositioned by half char width
      Vec2 p(x, y); if(pixel_align)p=Round(p);

      // draw
    //if(ti.is())
      {
         Flt xsize_2=xsize*0.5f;

         // sub-pixel rendering
         Flt      max_size=Max(Abs(xsize), Abs(ysize)), mip_map=0.5f*Log2(Sqr(1/max_size));
         FontDraw draw(image, Round(mip_map), font->_sub_pixel);

         Int            cur_code_index=FindCode(code, codes, ti.p());
       C TextCodeData * cur_code      =(InRange(cur_code_index  , codes) ? &code[cur_code_index  ] : null),
                      *next_code      =(InRange(cur_code_index+1, codes) ? &code[cur_code_index+1] : null);

         // font params
         Byte lum=color.lum(); if(!lum)draw.contrast=1;else
         {
            Flt pixels =size.min();
            if( pixels>=32)draw.contrast=1;else
            {
               draw.contrast=32/pixels;
               draw.contrast=Sqrt(draw.contrast); // or alternative: draw.contrast=Log2(draw.contrast+1);
               draw.contrast=Max(1, draw.contrast);
               draw.contrast=Lerp(1.0f, draw.contrast, lum/255.0f);
            }
         }

         draw.setCode(cur_code, T);

         for(Char c=ti.c(); c; )
         {
            if(!max_length--)break;

            // check if encountered next code
            for(; next_code && ti.p()>=next_code->pos; )
            {
                cur_code_index++;
                cur_code=next_code;
               next_code=(InRange(cur_code_index+1, codes) ? &code[cur_code_index+1] : null);
               draw.setCode(cur_code, T);
            }

            // next character
            Char n=(ti.t8 ? Char8To16Fast(*++ti.t8) : *++ti.t16);

            // draw character
            if(c!=' ') // don't draw space sign, potentially we could check it below using "fc.height" instead, however this is faster
            {
               UInt index=font->_wide_to_font[U16(c)]; if(InRange(index, font->_chrs))
               {
                C Font::Chr &fc=font->_chrs[index];
                //if(fc.height) // potentially we could check for empty characters (full width space, half width space, nbsp, however in most cases we will have something to draw, so this check will slow down things
                  {
                     Vec2 c_pos=p;
                     if(spacing==SPACING_CONST)c_pos.x-=xsize_2*fc.width ; // move back by half of the character width
                                               c_pos.y-=ysize  *fc.offset;
                     if(pixel_align)c_pos.x=Round(c_pos.x);

                     draw.draw(font->_images[fc.image], fc.tex, Rect_LU(c_pos, xsize*fc.width_padd, ysize*fc.height_padd));

                     // combining
                     UInt flag=CharFlagFast(n); if(flag&CHARF_COMBINING)
                     {
                        c_pos.x+=xsize_2*fc.width; // after 'c_pos' was pixel aligned, move by half of the character width to put it at centerX of 'c' character
                        c_pos.y =p.y; // reset Y pos
                        if(flag&CHARF_STACK)c_pos.y+=ysize*font->paddingB(); // skip shadow padding at the bottom, because next character is drawn after base character and on top, so its bottom shadow must not overlap the base

                     again:
                        UInt index=font->_wide_to_font[U16(n)]; if(InRange(index, font->_chrs))
                        {
                           if(!max_length--)break;

                         C Font::Chr &fc=font->_chrs[index];

                           Vec2 n_pos=c_pos;
                           n_pos.x-=xsize_2*fc.width; // move back by half of the character width
                           n_pos.y-=ysize  *fc.offset;
                           if(pixel_align)n_pos.x=Round(n_pos.x);

                           draw.draw(font->_images[fc.image], fc.tex, Rect_LU(n_pos, xsize*fc.width_padd, ysize*fc.height_padd));

                           n=(ti.t8 ? Char8To16Fast(*++ti.t8) : *++ti.t16);
                           flag=CharFlagFast(n); if(flag&CHARF_COMBINING) // if next character is combining too
                           {
                              if(flag&CHARF_STACK)c_pos.y+=ysize*fc.height_padd; // move position higher, to stack combining characters on top of each other (needed for THAI)
                              goto again;
                           }
                        }
                     }
                  }
               }
            }

            // update positions
            p.x+=space + xsize*font->charWidth(c, n, spacing);
            c=n;
         }
      }
   }
}
/******************************************************************************/
void TextStyleParams::drawSplit(C Rect &rect, Memc<TextLineSplit8> &tls, C TextCodeData *code, Int codes)C
{
   Flt  h=lineHeight();
   Vec2 p(rect.lerpX(align.x*-0.5f+0.5f),
          Lerp(rect.min.y+(tls.elms()-1)*h, rect.max.y, align.y*-0.5f+0.5f));
   if(pixel_align)D.alignScreenToPixel(p); // align here to prevent jittering between lines when moving the whole text

 C Rect &clip=D._clip ? D._clip_rect : D.viewRect();
   Vec2 range; posY(p.y, range);
   Int  start=Max(           0, Floor((range.y-clip.max.y)/h)),
        end  =Min(tls.elms()-1, Ceil ((range.x-clip.min.y)/h));

#if DEBUG && 0
   D.clip(null);
   D.lineX(RED, range.y                 , -D.w(), D.w()); // max
   D.lineX(RED, range.x-(tls.elms()-1)*h, -D.w(), D.w()); // min
   D.text(0, D.h()*0.9f, S+start+' '+end);
#endif

   for(; start<=end; start++){auto &t=tls[start]; drawMain(p.x, p.y-start*h, t.text, t.length, code, codes, t.offset);}
}
void TextStyleParams::drawSplit(C Rect &rect, Memc<TextLineSplit16> &tls, C TextCodeData *code, Int codes)C
{
   Flt  h=lineHeight();
   Vec2 p(rect.lerpX(align.x*-0.5f+0.5f),
          Lerp(rect.min.y+(tls.elms()-1)*h, rect.max.y, align.y*-0.5f+0.5f));
   if(pixel_align)D.alignScreenToPixel(p); // align here to prevent jittering between lines when moving the whole text

 C Rect &clip=D._clip ? D._clip_rect : D.viewRect();
   Vec2 range; posY(p.y, range);
   Int  start=Max(           0, Floor((range.y-clip.max.y)/h)),
        end  =Min(tls.elms()-1, Ceil ((range.x-clip.min.y)/h));

#if DEBUG && 0
   D.clip(null);
   D.lineX(RED, range.y                 , -D.w(), D.w()); // max
   D.lineX(RED, range.x-(tls.elms()-1)*h, -D.w(), D.w()); // min
   D.text(0, D.h()*0.9f, S+start+' '+end);
#endif

   for(; start<=end; start++){auto &t=tls[start]; drawMain(p.x, p.y-start*h, t.text, t.length, code, codes, t.offset);}
}
/******************************************************************************/
void TextStyleParams::drawSplitSoft(Image &image, C Rect &rect, Memt<TextLineSplit8> &tls, C TextCodeData *code, Int codes)C
{
   Flt  h=lineHeight();
   Vec2 p(rect.lerpX(align.x*-0.5f+0.5f),
          Lerp(rect.max.y-(tls.elms()-1)*h, rect.min.y, align.y*-0.5f+0.5f));
   if(pixel_align)p=Round(p); // align here to prevent jittering between lines when moving the whole text

   Rect clip(0, image.size());
   Vec2 range; posYI(p.y, range);
   Int  start=Max(           0, Floor((clip.min.y-range.x)/h)),
        end  =Min(tls.elms()-1, Ceil ((clip.max.y-range.y)/h));

   for(; start<=end; start++){auto &t=tls[start]; drawMainSoft(image, p.x, p.y+start*h, t.text, t.length, code, codes, t.offset);}
}
void TextStyleParams::drawSplitSoft(Image &image, C Rect &rect, Memt<TextLineSplit16> &tls, C TextCodeData *code, Int codes)C
{
   Flt  h=lineHeight();
   Vec2 p(rect.lerpX(align.x*-0.5f+0.5f),
          Lerp(rect.max.y-(tls.elms()-1)*h, rect.min.y, align.y*-0.5f+0.5f));
   if(pixel_align)p=Round(p); // align here to prevent jittering between lines when moving the whole text

   Rect clip(0, image.size());
   Vec2 range; posYI(p.y, range);
   Int  start=Max(           0, Floor((clip.min.y-range.x)/h)),
        end  =Min(tls.elms()-1, Ceil ((clip.max.y-range.y)/h));

   for(; start<=end; start++){auto &t=tls[start]; drawMainSoft(image, p.x, p.y+start*h, t.text, t.length, code, codes, t.offset);}
}
/******************************************************************************/
void TextStyleParams::drawCode(C Rect &rect, CChar8 *t, AUTO_LINE_MODE auto_line, C TextCodeData *code, Int codes)C {Set(Tls8 , t, T, rect.w(), auto_line); drawSplit(rect, Tls8 , code, codes);}
void TextStyleParams::drawCode(C Rect &rect, CChar  *t, AUTO_LINE_MODE auto_line, C TextCodeData *code, Int codes)C {Set(Tls16, t, T, rect.w(), auto_line); drawSplit(rect, Tls16, code, codes);}
/******************************************************************************/
void DisplayDraw::text(C TextStyleParams &text_style, Flt x, Flt y, CChar8 *t) {text_style.drawMain(x, y, t);}
void DisplayDraw::text(C TextStyleParams &text_style, Flt x, Flt y, CChar  *t) {text_style.drawMain(x, y, t);}

void DisplayDraw::text(                               Flt x, Flt y, CChar8 *t) {if(Gui.skin && Gui.skin->text_style)Gui.skin->text_style->drawMain(x, y, t);}
void DisplayDraw::text(                               Flt x, Flt y, CChar  *t) {if(Gui.skin && Gui.skin->text_style)Gui.skin->text_style->drawMain(x, y, t);}

void DisplayDraw::text(C TextStyleParams &text_style, C Rect &rect, CChar8 *t, AUTO_LINE_MODE auto_line) {text_style.drawCode(rect, t, auto_line);}
void DisplayDraw::text(C TextStyleParams &text_style, C Rect &rect, CChar  *t, AUTO_LINE_MODE auto_line) {text_style.drawCode(rect, t, auto_line);}

void DisplayDraw::text(                               C Rect &rect, CChar8 *t, AUTO_LINE_MODE auto_line) {if(Gui.skin && Gui.skin->text_style)Gui.skin->text_style->drawCode(rect, t, auto_line);}
void DisplayDraw::text(                               C Rect &rect, CChar  *t, AUTO_LINE_MODE auto_line) {if(Gui.skin && Gui.skin->text_style)Gui.skin->text_style->drawCode(rect, t, auto_line);}
/******************************************************************************/
void TextStyleParams::drawSoft(Image &image, Flt x, Flt y, CChar8 *t)C {drawMainSoft(image, x, y, t);}
void TextStyleParams::drawSoft(Image &image, Flt x, Flt y, CChar  *t)C {drawMainSoft(image, x, y, t);}

void TextStyleParams::drawSoft(Image &image, C Rect &rect, CChar8 *t, AUTO_LINE_MODE auto_line)C {Memt<TextLineSplit8 > tls8 ; Set(tls8 , t, T, rect.w(), auto_line); drawSplitSoft(image, rect, tls8 );} // use temp because this can be called outside of Draw
void TextStyleParams::drawSoft(Image &image, C Rect &rect, CChar  *t, AUTO_LINE_MODE auto_line)C {Memt<TextLineSplit16> tls16; Set(tls16, t, T, rect.w(), auto_line); drawSplitSoft(image, rect, tls16);} // use temp because this can be called outside of Draw
/******************************************************************************/
// IO
/******************************************************************************/
#pragma pack(push, 1)
struct TextStyleDesc
{
   SPACING_MODE spacing;
   Byte         shadow, shade;
   Color        color, selection;
   Vec2         align, size, space;
};
#pragma pack(pop)

Bool TextStyle::save(File &f, CChar *path)C
{
   f.putMulti(UInt(CC4_TXDS), Byte(4)); // version

   TextStyleDesc desc;
   Unaligned(desc.spacing  , spacing);
   Unaligned(desc.shadow   , shadow);
   Unaligned(desc.shade    , shade);
   Unaligned(desc.color    , color);
   Unaligned(desc.selection, selection);
   Unaligned(desc.align    , align);
   Unaligned(desc.size     , size);
   Unaligned(desc.space    , space);
   f<<desc;
   f._putAsset(font().name(path));
   return f.ok();
}
Bool TextStyle::load(File &f, CChar *path)
{
   pixel_align=true;
   edit=null;
   if(f.getUInt()==CC4_TXDS)switch(f.decUIntV()) // version
   {
      case 4:
      {
         TextStyleDesc desc; if(f.getFast(desc))
         {
            Unaligned(spacing  , desc.spacing);
            Unaligned(shadow   , desc.shadow);
            Unaligned(shade    , desc.shade);
            Unaligned(color    , desc.color);
            Unaligned(selection, desc.selection);
            Unaligned(align    , desc.align);
            Unaligned(size     , desc.size);
            Unaligned(space    , desc.space);
           _font.require(f._getAsset(), path); super::font(_font());
            if(f.ok())return true;
         }
      }break;

      case 3:
      {
         #pragma pack(push, 1)
         struct TextStyleDesc
         {
            SPACING_MODE spacing;
            Byte         shadow, shade;
            Color        color;
            Vec2         align, size, space;
         };
         #pragma pack(pop)
         TextStyleDesc desc; if(f.get(desc))
         {
            Unaligned(spacing, desc.spacing);
            Unaligned(shadow , desc.shadow);
            Unaligned(shade  , desc.shade);
            Unaligned(color  , desc.color);
            Unaligned(align  , desc.align);
            Unaligned(size   , desc.size);
            Unaligned(space  , desc.space);
           _font.require(f._getStr(), path); super::font(_font());
            selection=DefaultSelectionColor;
            if(f.ok())return true;
         }
      }break;

      case 2:
      {
         #pragma pack(push, 4)
         struct TextStyleDesc
         {
            SPACING_MODE spacing;
            Byte         shadow, shade;
            Color        color;
            Vec2         align, size, space;
         };
         #pragma pack(pop)
         TextStyleDesc desc; if(f.get(desc))
         {
            Unaligned(spacing, desc.spacing);
            Unaligned(shadow , desc.shadow);
            Unaligned(shade  , desc.shade);
            Unaligned(color  , desc.color);
            Unaligned(align  , desc.align);
            Unaligned(size   , desc.size);
            Unaligned(space  , desc.space);
           _font.require(f._getStr(), path); super::font(_font());
            selection=DefaultSelectionColor;
            if(f.ok())return true;
         }
      }break;

      case 1:
      {
         #pragma pack(push, 4)
         struct TextStyleDesc
         {
            SPACING_MODE spacing;
            Byte         shadow, shade;
            VecB4        color;
            Vec2         align, size, space;
         };
         #pragma pack(pop)
         TextStyleDesc desc; if(f.get(desc))
         {
            Unaligned(spacing, desc.spacing);
            Unaligned(shadow , desc.shadow);
            Unaligned(shade  , desc.shade);
                      color  .set(Unaligned(desc.color.z), Unaligned(desc.color.y), Unaligned(desc.color.x), Unaligned(desc.color.w));
            Unaligned(align  , desc.align);
            Unaligned(size   , desc.size);
            Unaligned(space  , desc.space);
           _font.require(f._getStr(), path); super::font(_font());
            selection=DefaultSelectionColor;
            if(f.ok())return true;
         }
      }break;

      case 0:
      {
         #pragma pack(push, 4)
         struct TextStyleDesc
         {
            SPACING_MODE spacing;
            Byte         shadow, shade;
            VecB4        color;
            Vec2         align, size, space;
         };
         #pragma pack(pop)
         TextStyleDesc desc; if(f.get(desc))
         {
            Unaligned(spacing, desc.spacing);
            Unaligned(shadow , desc.shadow);
                      shade  =DefaultShade;
                      color  .set(Unaligned(desc.color.z), Unaligned(desc.color.y), Unaligned(desc.color.x), Unaligned(desc.color.w));
            Unaligned(align  , desc.align);
            Unaligned(size   , desc.size);
            Unaligned(space  , desc.space);
           _font.require(f._getStr8(), path); super::font(_font());
            selection=DefaultSelectionColor;
            if(f.ok())return true;
         }
      }break;
   }
   reset(); return false;
}

Bool TextStyle::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool TextStyle::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
void TextStyle::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Text Style \""         +name+"\"",
                       PL,S+u"Nie można wczytać stylu tekstu \""+name+"\""));
}
/******************************************************************************/
// TEXT CODE
/******************************************************************************/
static TextCodeData& NewCode(MemPtr<TextCodeData> codes, Str &text)
{
   TextCodeData &code=codes.New(); code.pos=(CPtr)text.length();
   if(codes.elms()>=2)
   {
      TextCodeData &last=codes[codes.elms()-2];
      code.shadow_mode=((last.shadow_mode==TextCodeData::DEFAULT) ? TextCodeData::DEFAULT : TextCodeData::PREV); code.shadow=last.shadow;
      code. color_mode=((last. color_mode==TextCodeData::DEFAULT) ? TextCodeData::DEFAULT : TextCodeData::PREV); code.color =last.color ;
      code.nocode_mode=((last.nocode_mode==TextCodeData::DEFAULT) ? TextCodeData::DEFAULT : TextCodeData::PREV);
   }else
   {
      code.shadow_mode=code.color_mode=code.nocode_mode=TextCodeData::DEFAULT;
      code.shadow=255;
      code.color =WHITE;
   }
   return code;
}
static Bool GetDig(Byte &dig, CChar* &text)
{
   Int i=CharInt(*text);
   if( i>=0){dig=i; text++; return true;}
   return false;
}
static Color GetColor(CChar *text)
{
   text=_SkipWhiteChars(text);
   Byte dig[8], digs=0; FREP(8)if(GetDig(dig[i], text))digs++;else break;
   switch(digs)
   {
      case  3: return Color( dig[0]*255/0xF   ,  dig[1]*255/0xF   ,  dig[2]*255/0xF                       ); // RGB
      case  4: return Color( dig[0]*255/0xF   ,  dig[1]*255/0xF   ,  dig[2]*255/0xF   ,  dig[3]*255/0xF   ); // RGBA
      case  6: return Color((dig[0]<<4)|dig[1], (dig[2]<<4)|dig[3], (dig[4]<<4)|dig[5]                    ); // RRGGBB
      case  8: return Color((dig[0]<<4)|dig[1], (dig[2]<<4)|dig[3], (dig[4]<<4)|dig[5], (dig[6]<<4)|dig[7]); // RRGGBBAA
      default: return WHITE;
   }
}
static Byte GetByte(CChar *text)
{
   text=_SkipWhiteChars(text);
   Byte dig[2], digs=0; FREP(2)if(GetDig(dig[i], text))digs++;else break;
   switch(digs)
   {
      case  1: return  dig[0]*255/0xF;
      case  2: return (dig[0]<<4)|dig[1];
      default: return 255;
   }
}
void SetTextCode(C Str &code, Str &text, MemPtr<TextCodeData> codes)
{
   // 'codes' initially uses 'pos' as Int
   text .clear();
   codes.clear();

   Bool               nocode=false;
   MemtN<Color, 1024> colors ;
   MemtN<Byte , 1024> shadows;

   for(CChar *cc=code(); cc; )
   {
      Char c=*cc++; if(!c)break;
      if(c=='[') // code sign
      {
         if(!nocode && (Starts(cc, "col=") || Starts(cc, "color=")))
         {
            if(cc=TextPos(cc, '='))
            {
               Color         col =GetColor(cc+1);
               TextCodeData &code=NewCode(codes, text);
               code.color_mode=TextCodeData::NEW;
               code.color     =col; colors.New()=col;
            }
            if(cc=TextPos(cc, ']'))cc++;
         }else
         if(!nocode && (Starts(cc, "shd=") || Starts(cc, "shadow=")))
         {
            if(cc=TextPos(cc, '='))
            {
               Byte          shd =GetByte(cc+1);
               TextCodeData &code=NewCode(codes, text);
               code.shadow_mode=TextCodeData::NEW;
               code.shadow     =shd; shadows.New()=shd;
            }
            if(cc=TextPos(cc, ']'))cc++;
         }else
         if(!nocode && Starts(cc, "/col"))
         {
            TextCodeData &code=NewCode(codes, text);
            colors.removeLast();
            if(colors.elms())
            {
               code.color_mode=TextCodeData::OLD;
               code.color     =colors.last();
            }else
            {
               code.color_mode=TextCodeData::DEFAULT;
               code.color     =WHITE;
            }
            if(cc=TextPos(cc, ']'))cc++;
         }else
         if(!nocode && Starts(cc, "/sh"))
         {
            TextCodeData &code=NewCode(codes, text);
            shadows.removeLast();
            if(shadows.elms())
            {
               code.shadow_mode=TextCodeData::OLD;
               code.shadow     =shadows.last();
            }else
            {
               code.shadow_mode=TextCodeData::DEFAULT;
               code.shadow     =255;
            }
            if(cc=TextPos(cc, ']'))cc++;
         }else
         if(Starts(cc, "nocode"))
         {
            nocode=true;
            TextCodeData &code=NewCode(codes, text);
            code.nocode_mode=TextCodeData::NEW;
            if(cc=TextPos(cc, ']'))cc++;
         }else
         if(Starts(cc, "/nocode"))
         {
            nocode=false;
            TextCodeData &code=NewCode(codes, text);
            code.nocode_mode=TextCodeData::DEFAULT;
            if(cc=TextPos(cc, ']'))cc++;
         }else
         {
            text+='[';
         }
      }else
      {
         text+=c;
      }
   }

   // update codes
   REPA(codes)codes[i].pos=CPtr(text()+UIntPtr(codes[i].pos)); // convert Int to Ptr
}
Str GetTextCode(C Str &text, C TextCodeData *code, Int codes)
{
   Str out;
   Int cur_code=0, colors=0, shadows=0, nocodes=0;
   for(Int i=0; ; i++)
   {
      for(; InRange(cur_code, codes) && text()+i>=code[cur_code].pos; )
      {
       C TextCodeData &c=code[cur_code++];

         switch(c.color_mode)
         {
            case TextCodeData::NEW: out+="[color="; out+=c.color.asHex(); out+=']'; colors++; break;

            case TextCodeData::DEFAULT: if(!colors)break; // on purpose without break
            case TextCodeData::OLD    : colors--; out+="[/color]"; break;
         }

         switch(c.shadow_mode)
         {
            case TextCodeData::NEW: out+="[shadow="; out+=TextHex(U32(c.shadow), 2); out+=']'; shadows++; break;

            case TextCodeData::DEFAULT: if(!shadows)break; // on purpose without break
            case TextCodeData::OLD    : shadows--; out+="[/shadow]"; break;
         }

         switch(c.nocode_mode)
         {
            case TextCodeData::NEW: out+="[nocode]"; nocodes++; break;

            case TextCodeData::DEFAULT: if(!nocodes)break; // on purpose without break
            case TextCodeData::OLD    : nocodes--; out+="[/nocode]"; break;
         }
      }
      if(i>=text.length())break;
       out+=text[i];
   }
   return out;
}
/******************************************************************************/
TextCode& TextCode::clear(           )  {                       T._text.clear(); _codes.clear(); return T;}
TextCode& TextCode::set  (C Str &text)  {                       T._text=text   ; _codes.clear(); return T;}
TextCode& TextCode::code (C Str &code)  {       SetTextCode(code, _text        , _codes       ); return T;}
Str       TextCode::code (           )C {return GetTextCode(T(), _codes.data(), _codes.elms());}

void TextCode::draw(C TextStyleParams &text_style, C Rect &rect, AUTO_LINE_MODE auto_line)C
{
   text_style.drawCode(rect, T(), auto_line, _codes.data(), _codes.elms());
}
void TextCode::draw(C Rect &rect, AUTO_LINE_MODE auto_line)C
{
   if(Gui.skin && Gui.skin->text_style)Gui.skin->text_style->drawCode(rect, T(), auto_line, _codes.data(), _codes.elms());
}
/******************************************************************************/
}
/******************************************************************************/
