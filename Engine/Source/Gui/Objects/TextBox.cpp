/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define TEXTBOX_OFFSET 0.16f // set >=0.06 (at this value cursor is aligned with the TextBox rect left edge) this value is applied to the left and right of the text to add some margin
#define TEXTBOX_MARGIN 2.5f
/******************************************************************************/
void TextBox::zero()
{
   kb_lit=true;
  _slidebar_size=0.05f;

  _can_select=false;
  _word_wrap =true;
  _max_length=-1;

  _func_immediate=false;
  _func_user     =null;
  _func          =null;

  _edit.reset();
  _crect.zero();
}
TextBox::TextBox() {zero();}
TextBox& TextBox::del()
{
   slidebar[0].del  ();
   slidebar[1].del  ();
   view       .del  ();
   hint       .del  ();
  _text       .del  ();
  _skin       .clear();
   super::del(); zero(); return T;
}
void TextBox::setParent()
{
   slidebar[0]._parent=slidebar[1]._parent=view._parent=this;
}
void TextBox::setParams()
{
  _type=GO_TEXTBOX;
   view._sub_type=BUTTON_TYPE_REGION_VIEW;
   setParent();
}
TextBox& TextBox::create(C Str &text)
{
   del();

  _visible   =true;
  _rect.max.x= 0.3f;
  _rect.min.y=-0.3f;

   view       .create().hide().mode=BUTTON_CONTINUOUS;
   slidebar[0].create().setLengths(0, 0).hide();
   slidebar[1].create().setLengths(0, 0).hide();
   view._focusable=slidebar[0]._focusable=slidebar[1]._focusable=false;
   setParams();

   return set(text, QUIET);
}
TextBox& TextBox::create(C TextBox &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type          =GO_TEXTBOX;
         kb_lit        =src. kb_lit;
         hint          =src. hint;
        _slidebar_size =src._slidebar_size;
        _can_select    =src._can_select;
        _word_wrap     =src._word_wrap;
        _max_length    =src._max_length;
        _func_immediate=src._func_immediate;
        _func_user     =src._func_user;
        _func          =src._func;
        _crect         =src._crect;
        _skin          =src._skin;
        _text          =src._text;
        _edit          =src._edit;
         view       .create(src.view       );
         slidebar[0].create(src.slidebar[0]);
         slidebar[1].create(src.slidebar[1]);
         setParent();
         setTextInput();
      }
   }
   return T;
}
/******************************************************************************/
void TextBox::setTextInput()C
{
#if ANDROID
   if(Gui.kb()==this)Kb.setTextInput(T(), (_edit.sel<0) ? cursor() : _edit.sel, cursor(), false);
#endif
}
TextBox& TextBox::slidebarSize(Flt size)
{
   MAX(size, 0);
   if(_slidebar_size!=size)
   {
     _slidebar_size=size;
      if(wordWrap())setVirtualSize();else setButtons(); // in 'wordWrap' mode, virtual size is dependent on the slidebar size
   }
   return T;
}
void TextBox::setVirtualSize()
{
   Vec2 size=0;
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->textline.text_style())
   {
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      TextStyleParams ts=*text_style; if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #else
    C TextStyle &ts=*text_style;
   #endif

      Flt offset2=ts.size.x*(TEXTBOX_OFFSET*2), w=rect().w()-offset2; // decrease available width for text by offset for both sides
      Int lines  =ts.textLines(T(), w, wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE, &size.x);
      size.y=lines*ts.lineHeight();
      if(wordWrap()) // check if in word wrap we're exceeding lines beyond client rectangle, in that case slidebar has to be made visible, which reduces client width
      {
         Flt client_height=rect().h(); // here can't use 'clientHeight' because it may not be available yet
         if(size.y>client_height+EPS) // exceeds client height
         { // recalculate using smaller width
            Int lines=ts.textLines(T(), w-slidebarSize(), /*wordWrap() ? */AUTO_LINE_SPACE_SPLIT/* : AUTO_LINE_NONE*/, &size.x);
            size.y=lines*ts.lineHeight();
         }
      }
      size.x+=offset2; // we've calculated text widths with offset removed, but here we're calculating virtual size, and it needs to include it
   }
   slidebar[0]._length_total=size.x;
   slidebar[1]._length_total=size.y;
   setButtons();
}
/******************************************************************************/
TextBox& TextBox::maxLength(Int max_length)
{
   if(   max_length<0)max_length=-1;
   if(T._max_length!= max_length)
   {
      T._max_length=max_length;
      if(max_length>=0 && _text.length()>max_length)
      {
        _text.clip(     max_length);
         MIN(_edit.cur, max_length);
         MIN(_edit.sel, max_length);
         setVirtualSize();
         setTextInput();
      }
   }
   return T;
}
/******************************************************************************/
Bool TextBox::cursorChanged(Int position)
{
   Clamp(position, 0, _text.length());
   if(cursor()!=position)
   {
     _edit.cur=position;
      return true;
   }
   return false;
}
TextBox& TextBox::cursor(Int position)
{
   if(cursorChanged(position))
   {
      setTextInput();

      // make cursor visible
      if(GuiSkin *skin=getSkin())
         if(TextStyle *text_style=skin->textline.text_style())
      {
      #if DEFAULT_FONT_FROM_CUSTOM_SKIN
         TextStyleParams ts=*text_style; if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
      #else
       C TextStyle &ts=*text_style;
      #endif

         Flt offset=ts.size.x*TEXTBOX_OFFSET,
             margin=ts.size.x*TEXTBOX_MARGIN;
         Vec2 pos=ts.textIndex(T(), cursor(), virtualWidth() - offset*2, wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE); // here Y is 0..Inf
         pos.x+=offset;
         Flt pos_left  =pos.x-margin,
             pos_right =pos.x+margin,
             pos_bottom=pos.y+ts.size.y; // bottom position of the cursor (add because Y is inverted)
         if(pos_left<slidebar[0].offset() || pos_right >clientWidth ()+slidebar[0].offset())scrollFitX(pos_left, pos_right , true);
         if(pos.y   <slidebar[1].offset() || pos_bottom>clientHeight()+slidebar[1].offset())scrollFitY(pos.y   , pos_bottom, true);
      }
   }
   return T;
}
/******************************************************************************/
Bool TextBox::setChanged(C Str &text, SET_MODE mode)
{
   Str t=text; if(_max_length>=0)t.clip(_max_length);
   if(!Equal(T._text, t, true))
   {
      T._text    = t;
      T._edit.sel=-1;
      setVirtualSize();
      if(cursor()>t.length())cursorChanged(t.length());

      if(mode!=QUIET)call();
      return true;
   }
   return false;
}
TextBox& TextBox::set(C Str &text, SET_MODE mode)
{
   if(setChanged(text, mode))setTextInput();
   return T;
}
TextBox& TextBox::clear(SET_MODE mode) {return set(S, mode);}
/******************************************************************************/
TextBox& TextBox::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void TextBox::call()
{
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
TextBox& TextBox::selectNone()
{
   if(_edit.sel>=0)
   {
     _edit.sel=-1;
      setTextInput();
   }
   return T;
}
TextBox& TextBox::selectAll()
{
   if(_text.is())
      if(_edit.sel!=0 || _edit.cur!=_text.length())
   {
     _edit.sel=0;
     _edit.cur=_text.length();
      setTextInput();
   }
   return T;
}
/******************************************************************************/
void TextBox::setButtons()
{
   Bool vertical, horizontal;
   Flt  width =virtualWidth (),
        height=virtualHeight();

   Rect srect=_crect=rect();
   if( vertical  =(slidebar[1].is() && height>clientHeight()+EPS)){srect.max.x-=slidebarSize();                _crect.max.x-=slidebarSize();}
   if( horizontal=(slidebar[0].is() && width >clientWidth ()+EPS)){srect.min.y+=slidebarSize(); if(!wordWrap())_crect.min.y+=slidebarSize();
   if(!vertical && slidebar[1].is() && height>clientHeight()+EPS ){srect.max.x-=slidebarSize();                _crect.max.x-=slidebarSize(); vertical=true;}}

   slidebar[0].setLengths(clientWidth (), width ).rect(Rect(rect().min.x               ,  rect().min.y, srect  .max.x, rect().min.y+slidebarSize())); slidebar[0].visible(slidebar[0]._usable && !wordWrap());
   slidebar[1].setLengths(clientHeight(), height).rect(Rect(rect().max.x-slidebarSize(), srect  .min.y,  rect().max.x, rect().max.y               )); slidebar[1].visible(slidebar[1]._usable               );
   view                                          .rect(Rect(rect().max.x-slidebarSize(),  rect().min.y,  rect().max.x, rect().min.y+slidebarSize())).visible(slidebar[0]._usable && slidebar[1]._usable);
}
TextBox& TextBox::wordWrap(Bool wrap)
{
   if(_word_wrap!=wrap)
   {
     _word_wrap=wrap;
      setVirtualSize();
   }
   return T;
}
/******************************************************************************/
TextBox& TextBox::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      super::rect(rect);
      if(wordWrap())setVirtualSize();else setButtons(); // in 'wordWrap' mode, virtual size is dependent on the rectangle
   }
   return T;
}
TextBox& TextBox::move(C Vec2 &delta)
{
   if(delta.any())
   {
      super::move(delta);
     _crect  +=   delta ;
      view       .move(delta);
      slidebar[0].move(delta);
      slidebar[1].move(delta);
   }
   return T;
}
/******************************************************************************/
TextBox& TextBox::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   if(_skin!=skin)
   {
     _skin=skin;
      if(sub_objects)
      {
               view     .skin=skin ;
         REPAO(slidebar).skin(skin);
      }
      setVirtualSize(); // new skin can have different text style, so have to recalculate
   }
   return T;
}
/******************************************************************************/
GuiObj* TextBox::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      Bool priority=!Kb.shift(); // when 'Kb.shift' disabled (default) then priority=1 (vertical), when enabled then priority=0 (horizontal)
      if(slidebar[ priority]._usable)mouse_wheel=&slidebar[ priority];else // check  priority slidebar first
      if(slidebar[!priority]._usable)mouse_wheel=&slidebar[!priority];     // check !priority slidebar next

      GuiPC gpc2(gpc, visible(), enabled());
      if(GuiObj *go=slidebar[0].test(gpc2, pos, mouse_wheel))return go;
      if(GuiObj *go=slidebar[1].test(gpc2, pos, mouse_wheel))return go;
      if(GuiObj *go=view       .test(gpc2, pos, mouse_wheel))return go;

      return go;
   }
   return null;
}
/******************************************************************************/
void TextBox::moveCursor(Int lines, Int pages)
{
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->textline.text_style())
   {
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      TextStyleParams ts=*text_style; if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #else
    C TextStyle &ts=*text_style;
   #endif

      Flt offset2=ts.size.x*(TEXTBOX_OFFSET*2), width=virtualWidth() - offset2;
      AUTO_LINE_MODE auto_line=(wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE);
      if(!Kb.k.shift())_edit.sel=-1;else if(_edit.sel<0)_edit.sel=_edit.cur;
      if(pages)
      {
         Int page_lines=Trunc(clientHeight()/ts.lineHeight());
         lines+=pages*Max(1, page_lines); // always move at least one line
      }
      Vec2 pos=ts.textIndex(T(), cursor(), width, auto_line);
      pos.y+=ts.lineHeight()*(lines+0.5f); // add 0.5 to get rounding
      Bool eol; _edit.cur=ts.textPos(T(), pos.x, pos.y, true, width, auto_line, eol);
   }
}
/******************************************************************************/
void TextBox::update(C GuiPC &gpc)
{
   GuiPC gpc2(gpc, visible(), enabled());
   if(   gpc2.enabled)
   {
      view.update(gpc2);
      if(view())
      {
         if(Gui.ms()==&view)
         {
            Ms.freeze();
            slidebar[0].setOffset(slidebar[0]._offset + Ms.d().x*2);
            slidebar[1].setOffset(slidebar[1]._offset - Ms.d().y*2);
         }
         REPA(Touches)
         {
            Touch &t=Touches[i]; if(t.guiObj()==&view && t.on())
            {
               slidebar[0].setOffset(slidebar[0]._offset + t.d().x*2);
               slidebar[1].setOffset(slidebar[1]._offset - t.d().y*2);
            }
         }
      }

      // scroll horizontally
      if(Ms.wheelX()
      && (Gui.wheel()==&slidebar[0] || Gui.wheel()==&slidebar[1]) // if has focus on any of slidebars
      && slidebar[0]._usable) // we will scroll only horizontally, so check if that's possible
         slidebar[0].scroll(Ms.wheelX()*(slidebar[0]._scroll_mul*slidebar[0].length()+slidebar[0]._scroll_add), slidebar[0]._scroll_immediate);

      slidebar[0].update(gpc2);
      slidebar[1].update(gpc2);

      // update text here
      if(Gui.kb()==this)
      {
         Int  cur    =_edit.cur;
         Bool changed= EditText(_text, _edit, true);
         if(  changed)
         {
            if(_max_length>=0 && _text.length()>_max_length)
            {
              _text.clip(     _max_length);
               MIN(_edit.cur, _max_length);
               MIN(_edit.sel, _max_length);
            }
            setVirtualSize();
            call();
         }
         if(Kb.k(KB_UP  )){moveCursor(-1, 0); Kb.eatKey();}
         if(Kb.k(KB_DOWN)){moveCursor( 1, 0); Kb.eatKey();}
         if(Kb.k(KB_PGUP)){moveCursor(0, -1); Kb.eatKey();}
         if(Kb.k(KB_PGDN)){moveCursor(0,  1); Kb.eatKey();}
         if(cur!=_edit.cur || changed){cur=_edit.cur; _edit.cur=-1; cursor(cur);} // set -1 to force adjustment of offset and calling 'setTextInput'
      }
    C Vec2 *touch_pos  =null;
      Byte  touch_state=0   ; if(Gui.ms()==this && (Ms._button[0]&(BS_ON|BS_PUSHED))){touch_pos=&Ms.pos(); touch_state=Ms._button[0];} if(!touch_pos)REPA(Touches)if(Touches[i].guiObj()==this && (Touches[i]._state&(BS_ON|BS_PUSHED))){touch_pos=&Touches[i].pos(); touch_state=Touches[i]._state;}
      if(_text.is() && touch_pos)
      {
         if(GuiSkin *skin=getSkin())
            if(TextStyle *text_style=skin->textline.text_style())
         {
         #if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts=*text_style; if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
         #else
          C TextStyle &ts=*text_style;
         #endif

            Flt  offset      =ts.size.x*TEXTBOX_OFFSET;
            Vec2 relative_pos=*touch_pos-gpc.offset,
                  clipped_pos=relative_pos&_crect; // have to clip so after we start selecting and move mouse outside the client rectangle, we don't set cursor from outside, instead start smooth scrolling when mouse is outside
            Bool eol; Int pos=ts.textPos(T(), clipped_pos.x - _crect.min.x + slidebar[0].offset() - offset, _crect.max.y - clipped_pos.y + slidebar[1].offset(), !ButtonDb(touch_state) && !_edit.overwrite, virtualWidth() - offset*2, wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE, eol);

            if(ButtonDb(touch_state))
            {
               if(eol && pos && _text[pos-1]!='\n')pos--; // if double-clicked at the end of the line and previous character isn't a new line (have to check for this because if we don't then we would go to previous line when clicking on empty lines), then go back, have to check 'eol' and not "_text[pos]=='\n'" because this line could be split because of too long text (in this case there will be 'eol' but no '\n')
               Char c=_text[Min(pos, _text.length()-1)];
               if(c && c!='\n') // if not end and not new line
               {
                 _edit.cur=_edit.sel=pos;
                  CHAR_TYPE type=CharType(c);
                  for(; _edit.sel                && CharType(_text[_edit.sel-1])==type; _edit.sel--);
                  for(; _edit.cur<_text.length() && CharType(_text[_edit.cur  ])==type; _edit.cur++);
                  if (  _edit.sel==_edit.cur)_edit.sel=-1;
               }else
               {
                 _edit.cur=pos; _edit.sel=-1; // when double-clicking on an empty line, just leave the cursor in its position
               }
              _can_select=false;
               setTextInput();
            }else
            if(_can_select)
            {
               if(ButtonPd(touch_state))
               {
                  if(_edit.cur!=pos || _edit.sel>=0)
                  {
                    _edit.cur=_edit.sel=-1; cursor(pos); // set -1 to force adjustment of offset and calling 'setTextInput'
                  }
               }else
               if(pos!=_edit.cur)
               {
                  if(_edit.sel<0)_edit.sel=_edit.cur;
                                 _edit.cur=pos;
                  setTextInput();
               }

               if(relative_pos.x<_crect.min.x)slidebar[0].button[1].push();else
               if(relative_pos.x>_crect.max.x)slidebar[0].button[2].push();
               if(relative_pos.y<_crect.min.y)slidebar[1].button[2].push();else
               if(relative_pos.y>_crect.max.y)slidebar[1].button[1].push();
            }
         }
      }else _can_select=true;
   }
}
void TextBox::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      GuiSkin *skin=getSkin();
      Rect     rect=T.rect()+gpc.offset, ext_rect;
      if(skin && skin->region.normal)skin->region.normal->extendedRect(rect, ext_rect);else ext_rect=rect;
      if(Cuts(ext_rect, gpc.clip))
      {
         if(skin)
         {
            D.clip(gpc.clip);
            if(skin->region.normal        )skin->region.normal->draw(skin->region.normal_color, rect);else
            if(skin->region.normal_color.a)                rect.draw(skin->region.normal_color);

            // text
            if(TextStyle *text_style=skin->textline.text_style())
            {
               Bool enabled=(T.enabled() && gpc.enabled),
                    active=(Gui.kb()==this && enabled && ((Gui._overlay_textline==this) ? Equal(Gui._overlay_textline_offset, gpc.offset) : true)); // if this is the overlay textline, then draw cursor and editing only if it matches the overlay offset
               if(T().is() || active || hint.is())
               {
                C Color *text_color; // never null
                  if(enabled)text_color=&skin->textline.  normal_text_color;
                  else       text_color=&skin->textline.disabled_text_color;

                  TextStyleParams ts=*text_style; ts.align.set(1, -1); ts.color=ColorMul(ts.color, *text_color);
               #if DEFAULT_FONT_FROM_CUSTOM_SKIN
                  if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
               #endif

                  Rect text_rect=_crect+gpc.offset;
                  D.clip(text_rect&gpc.clip);
                  Flt offset=TEXTBOX_OFFSET*ts.size.x;
                  if(T().is() || active)
                  {
                     text_rect.min.x+=offset - slidebar[0].offset(); text_rect.max.x=text_rect.min.x+virtualWidth () - offset*2;
                     text_rect.max.y+=         slidebar[1].offset(); text_rect.min.y=text_rect.max.y-virtualHeight()           ;
                     if(active)ts.edit=&_edit;
                     D.text(ts, text_rect, T(), wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE);
                  #if DEBUG && 0 // draw cursor position
                     Vec2 pos=ts.textIndex(T(), cursor(), text_rect.w(), wordWrap() ? AUTO_LINE_SPACE_SPLIT : AUTO_LINE_NONE);
                     (pos*Vec2(1, -1) + text_rect.lu()).draw(RED);
                  #endif
                  }else
                  if(hint.is())
                  {
                     text_rect.extendX(-offset); // we could've set virtualSize to full client size as a special case in 'setVirtualSize', however to just do this here, is faster (also because most of the time, hint is not displayed)
                     ts.color.a=((ts.color.a*96)>>8); ts.size*=0.85f; D.text(ts, text_rect, hint, AUTO_LINE_SPACE_SPLIT);
                  }
               }
            }
         }
         view       .draw(gpc);
         slidebar[0].draw(gpc);
         slidebar[1].draw(gpc);

         if(kb_lit && Gui.kb()==this){D.clip(gpc.clip); Gui.kbLit(this, rect, skin);}
      }
   }
}
/******************************************************************************/
Bool TextBox::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      // no need to save '_crect' because we always reset it after loading
      f.putMulti(Byte(1), kb_lit, _word_wrap, _max_length, _slidebar_size); // version
      f<<hint<<_text;
      f.putAsset(_skin.id());
      if(view       .save(f, path))
      if(slidebar[0].save(f, path))
      if(slidebar[1].save(f, path))
         return f.ok();
   }
   return false;
}
Bool TextBox::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 1:
      {
         f.getMulti(kb_lit, _word_wrap, _max_length, _slidebar_size);
         f>>hint>>_text;
        _skin.require(f.getAssetID(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); setVirtualSize(); return true;} // have to reset virtual size in case text style has different values now, and because we're not saving '_crect'
      }break;

      case 0:
      {
         f.getMulti(kb_lit, _word_wrap, _max_length, _slidebar_size)._getStr2(hint)._getStr2(_text);
        _skin.require(f.getAssetID(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); setVirtualSize(); return true;} // have to reset virtual size in case text style has different values now, and because we're not saving '_crect'
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
