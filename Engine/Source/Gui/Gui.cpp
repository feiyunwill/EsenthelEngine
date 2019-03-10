/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
struct QueuedMsgBox // queue these commands for thread-safety, this is so that 'Gui.msgBox' does not require 'Gui.cs' lock, this is important in case for example the main thread is deleting a thread inside a 'Gui.update' callback "Gui.update -> thread.del()" while that thread is "if(thread.wantStop())Gui.msgBox(failed)" calling 'Gui.msgBox' would require 'Gui.cs' lock which is already locked on the main thread
{
   Str          title, text; 
   TextStylePtr text_style;

   void set(C Str &title, C Str &text, C TextStylePtr &text_style) {T.title=title; T.text=text; T.text_style=text_style;}
};
static Memc<QueuedMsgBox> QueuedMsgBoxs;
static SyncLock           QueuedMsgBoxLock;
/******************************************************************************/
STRUCT(MsgBox , Dialog)
//{
   static Memx<MsgBox> MsgBoxs;

   static void Del  (MsgBox &mb) {SyncLocker locker(Gui._lock); MsgBoxs.removeData(&mb);}
   static void Close(MsgBox &mb) {if(Gui.window_fade)mb.fadeOut();else Del(mb);}

   static MsgBox* Find(CPtr id) {REPA(MsgBoxs){MsgBox &mb=MsgBoxs[i]; if(mb.id==id)return &mb;} return null;}

   CPtr id;

   MsgBox& create(C Str &title, C Str &text, C TextStylePtr &text_style, CPtr id)
   {
      T.id=id;
      Gui+=super::create(title, text, Memt<Str>().add("OK"), text_style).level(65536);
      buttons[0].func(Close, T);
      if(Gui.window_fade){super::hide(); fadeIn();}
      return T;
   }
   virtual MsgBox& hide() {Del(T); return T;}
};
Memx<MsgBox> MsgBox::MsgBoxs;
/******************************************************************************/
GUI Gui;
/******************************************************************************/
// GUI
/******************************************************************************/
GUI::GUI() : _desktops(4), default_skin(3649875776, 1074192063, 580730756, 799774185)
{
   allow_window_fade=true;
   window_fade=false;
   desc_delay=0.3f;
   resize_radius=0.022f;
   click_sound_id.zero();

   draw_keyboard_highlight=DrawKeyboardHighlight;
   draw_description       =DrawDescription;
   draw_imm               =DrawIMM;

   window_fade_in_speed =9;
   window_fade_out_speed=6;
   window_fade_scale    =0.85f;

   dialog_padd         =0.03f;
   dialog_button_height=0.06f;
   dialog_button_padd  =dialog_button_height*2;
   dialog_button_margin=dialog_button_height;

  _window_buttons_right=!OSMac(); // check for 'OSMac' instead of 'MAC' so it can be detected on WEB too, check for Mac instead of Apple, because iOS is also Apple, but since it's touchscreen based, prefer right side because most people are right-handed and it's easier to have the buttons on the right side
  _drag_want=_dragging=false;
  _drag_user  =null;
  _drag_start =null;
  _drag_cancel=null;
  _drag_finish=null;
  _pass_char='*';
  _kb=_ms=_ms_src=_ms_lit=_wheel=_desc=_touch_desc=null;
  _menu=null;
  _window=_window_lit=null;
  _desktop=null;
}
/******************************************************************************/
void GUI::screenChanged(Flt old_width, Flt old_height)
{
   FREPAO(_desktops).rect(D.rect());

   if(Menu *menu=Gui.menu())
      if(GuiObj *owner=menu->Owner())
         if(owner->type()==GO_MENU_BAR && owner->parent()->is(GO_DESKTOP))
            menu->move(Vec2(old_width-D.w(), 0));
}
/******************************************************************************/
Bool GUI::Switch()
{
   if(Kb.alt())
   {
      // switch windows
   }else
   {
      // switch in window
      if(GuiObj *c=kb())
      if(GuiObj *p=c->parent())
      {
         switch(c->type())
         {
            case GO_MENU    :
            case GO_CUSTOM  :
            case GO_DESKTOP :
            case GO_VIEWPORT:
            case GO_REGION  :
            case GO_TEXTBOX :
            case GO_WINDOW  : return false;
            case GO_LIST    : if(p->type()==GO_MENU    )return false; c=p; p=c->parent(); if(!p)return false;  break;
            case GO_TEXTLINE: if(p->type()==GO_COMBOBOX){             c=p; p=c->parent(); if(!p)return false;} break;
         }
         Bool next=!Kb.k.shift();
         switch(p->type())
         {
            case GO_DESKTOP: return p->asDesktop()._children.Switch(*c, next);
            case GO_REGION : return p->asRegion ()._children.Switch(*c, next);
            case GO_TAB    : return p->asTab    ()._children.Switch(*c, next);
            case GO_WINDOW : return p->asWindow ()._children.Switch(*c, next);
            case GO_TABS   : {Tabs &tabs=p->asTabs(); if(InRange(tabs(), tabs))return tabs.tab(tabs())._children.Switch(*c, next);} break;
         }
      }
   }
   return false;
}
/******************************************************************************/
void GUI::setText()
{
   REPAO(_desktops).setText();
}
/******************************************************************************/
GuiObj* GUI::objAtPos(C Vec2 &pos)C
{
   GuiObj *mouse_wheel=null;
   return desktop() ? desktop()->asDesktop().test(pos, mouse_wheel) : null;
}
/******************************************************************************/
Color GUI::backgroundColor()C {if(GuiSkin *skin=Gui.skin())return skin->background_color; return         WHITE;}
Color GUI::    borderColor()C {if(GuiSkin *skin=Gui.skin())return skin->    border_color; return Color(0, 112);}
/******************************************************************************/
TextLine* GUI::overlayTextLine(Vec2 &offset)
{
   Rect kb_rect; if(Kb.softCoverage(kb_rect) && kb()->is(GO_TEXTLINE))
   {
      TextLine &tl=kb()->asTextLine();
      Rect_LU   tl_rect(tl.screenPos(), tl.size());
      if(Cuts(tl_rect, kb_rect) || tl_rect.min.y<-D.h())
      {
         // try to move above kb rect first (because when typing with fingers the hands are usually downwards, so they would occlude what's below them)
         if(kb_rect.max.y+tl_rect.h()<=D.h()) // if it fits in the visible screen area
         {
            offset=kb_rect.up()+tl_rect.size()*Vec2(-0.5f, 1.0f)-tl.pos();
         }else // move at the bottom of the screen
         {
            offset=Vec2(0, -D.h())+tl_rect.size()*Vec2(-0.5f, 1.0f)-tl.pos();
         }
         return &tl;
      }
   }
   return null;
}
/******************************************************************************/
void GUI::msgBox(C Str &title, C Str &text, C TextStylePtr &text_style)
{
   SyncLocker locker(QueuedMsgBoxLock);
   REPA(QueuedMsgBoxs)
   {
      QueuedMsgBox &qmb=QueuedMsgBoxs[i];
      if(Equal(qmb.title, title, true) && Equal(qmb.text, text, true) && qmb.text_style==text_style)return; // if already exists then do nothing
   }
   QueuedMsgBoxs.New().set(title, text, text_style);
}
Dialog& GUI::getMsgBox(CPtr id)
{
   SyncLocker locker(_lock);
   if(id)if(MsgBox *mb=MsgBox::Find(id))return *mb;
   return MsgBox::MsgBoxs.New().create(S, S, null, id); // always create to set the 'id' and 'Window.level'
}
Dialog* GUI::   findMsgBox(CPtr id) {if(id){SyncLocker locker(_lock); return MsgBox::Find(id);} return null;}
void    GUI::    delMsgBox(CPtr id) {if(id){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))MsgBox::MsgBoxs.removeData(mb);}}
void    GUI::fadeOutMsgBox(CPtr id) {if(id){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))mb->fadeOut();}}
void    GUI::  closeMsgBox(CPtr id) {if(id){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))MsgBox::Close(*mb);}}
/******************************************************************************/
GUI& GUI::passwordChar(Char c) // Warning: this is not thread-safe
{
   if(_pass_char!=c)
   {
     _pass_char=c;
     _pass_temp.clear();
   }
   return T;
}
C Str& GUI::passTemp(Int length) // Warning: this is not thread-safe
{
   if(   length<_pass_temp.length()  )_pass_temp.clip(length);else
   for(; length>_pass_temp.length(); )_pass_temp+=_pass_char;
   return _pass_temp;
}
/******************************************************************************/
static void SetWindowButtons(GuiObj &go)
{
   if(go.type()==GO_WINDOW)go.asWindow().setButtons();
   REP(go.childNum())if(GuiObj *child=go.child(i))SetWindowButtons(*child);
}
GUI& GUI::windowButtonsRight(Bool right)
{
   if(_window_buttons_right!=right)
   {
     _window_buttons_right=right;
      REPA(_desktops)SetWindowButtons(_desktops[i]);
   }
   return T;
}
/******************************************************************************/
void GUI::playClickSound()C
{
   SoundPlay(click_sound_id, 1, VOLUME_UI);
}
/******************************************************************************/
Vec2 GUI::dragPos()C
{
   if(dragging())
   {
      Touch *touch=FindTouch(_drag_touch_id);
      return touch ? touch->pos() : Ms.pos();
   }
   return 0;
}
void GUI::dragCancel()
{
   if(_drag_cancel){if(dragging())_drag_cancel(_drag_user); _drag_cancel=null;} // call cancel only if actually started dragging
  _drag_want=_dragging=false;
}
void GUI::drag(C Str &name, Touch *touch)
{
   dragCancel();
   if(_drag_want=true) // previously this checked for "if(_drag_want=name.is())" however empty names are now allowed
   {
      if(touch)touch->_scrolling=false; // when dragging is initiated, then unmark touch from scrolling
     _drag_touch_id=(touch ? touch->id () :        0);
     _drag_pos     =(touch ? touch->pos() : Ms.pos());
     _drag_name    =name;
     _drag_user    =null;
     _drag_start   =null;
     _drag_cancel  =null;
     _drag_finish  =null;
   }
}
void GUI::drag(void finish(Ptr user, GuiObj *obj, C Vec2 &screen_pos), Ptr user, Touch *touch, void start(Ptr user), void cancel(Ptr user))
{
   dragCancel();
   if(_drag_want=true) // previously this checked for "if(_drag_want=(finish!=null))" however empty functions are now allowed
   {
      if(touch)touch->_scrolling=false; // when dragging is initiated, then unmark touch from scrolling
     _drag_touch_id=(touch ? touch->id () :        0);
     _drag_pos     =(touch ? touch->pos() : Ms.pos());
     _drag_name    .clear();
     _drag_user    =user;
     _drag_start   =start;
     _drag_cancel  =cancel;
     _drag_finish  =finish;
   }
}
void GUI::dragDraw()
{
   if(dragging())if(Image *image=image_drag())image->draw(Rect_D(dragPos(), Vec2(0.05f*image->aspect(), 0.05f)));
}
/******************************************************************************/
void GUI::addFuncCall(void func(        )          ) {_callbacks.add(func      );}
void GUI::addFuncCall(void func(Ptr user), Ptr user) {_callbacks.add(func, user);}
/******************************************************************************/
void GUI::update()
{
   Dbl t=Time.curTime();
//_time_d_fade_in =Time.ad()*60;
  _time_d_fade_out=Time.ad()*14;
   SyncLocker locker(_lock);

   // test
  _ms_lit          =null;
  _wheel           =null;
  _overlay_textline=overlayTextLine(_overlay_textline_offset);
   if(Ms.detected() && Ms.visible() && Ms._on_client && desktop())_ms_lit=desktop()->asDesktop().test(Ms.pos(), _wheel); // don't set 'msLit' if mouse is not detected or hidden or on another window (do checks if mouse was focused on other window but now moves onto our window, and with buttons pressed in case for drag and drop detection and we would want to highlight the target gui object at which we're gonna drop the files)
   Byte ms_button=0; REPA(Ms._button)ms_button|=Ms._button[i];
   if(!(ms_button&(BS_ON|BS_RELEASED)))_ms=_ms_src=msLit();
   if(App.active())
   {
      if((ms_button&BS_PUSHED) && msLit())msLit()->activate();
      REPA(Touches)if(Touches[i].pd())if(GuiObj *go=Touches[i].guiObj())go->activate();
      if(Kb.k(KB_TAB))if(Switch())Kb.eatKey();
   }
  _window_lit=&msLit()->first(GO_WINDOW)->asWindow();

   // update
   FREPAO(_desktops).update();

   // callbacks
  _callbacks.update();

   // add message boxes after update and callbacks, so they're immediately displayed when created
   if(QueuedMsgBoxs.elms())
   {
      SyncLocker locker(QueuedMsgBoxLock);
      FREPA(QueuedMsgBoxs) // process in order
      {
       C QueuedMsgBox &qmb=QueuedMsgBoxs[i];
         REPA(MsgBox::MsgBoxs)
         {
          C MsgBox &mb=MsgBox::MsgBoxs[i];
            if(Equal(mb.title, qmb.title, true) && Equal(mb.text(), qmb.text, true) && mb.text.text_style==qmb.text_style)goto skip; // if already exists then do nothing
         }
         // create new one
         MsgBox::MsgBoxs.New().create(qmb.title, qmb.text, qmb.text_style, null);
      skip:;
      }
      QueuedMsgBoxs.clear();
   }

   // mouse description
   if(_desc!=ms()) // if there is a new object under mouse than previous description
   {
      if(!Ms.pixelDelta().any())_desc=null;else // set it only if we've moved the mouse (to eliminate showing description by elements activated with keyboard, touch or programatically)
      {
        _desc     =ms();
        _desc_time=Time.appTime();
      }
   }else
   if(Ms.pixelDelta().any() || (ms_button&(BS_ON|BS_PUSHED|BS_RELEASED)))_desc_time=Time.appTime(); // if there was a mouse action then reset the timer

   // touch description
   Touch  *stylus=null; REPA(Touches){Touch &touch=Touches[i]; if(touch.stylus() && !touch.on() && !touch.rs()){stylus=&touch; break;}}
   GuiObj *stylus_obj=(stylus ? stylus->guiObj() : null);
   if(_touch_desc!=stylus_obj) // if there is a new object under stylus than previous description
   {
     _touch_desc     =stylus_obj;
     _touch_desc_time=Time.appTime();
   }

   // drag !! process after objects update, because '_List.update' requires that 'Gui.dragging()' is still active at the moment of dragging release !!
   if(_drag_want)
   {
      Touch *touch=FindTouch(_drag_touch_id);
      if(!_drag_touch_id && Ms.bp(1))dragCancel();else // if mouse dragging and RMB pressed then cancel dragging
      if( _drag_touch_id ? (!touch || touch->rs()) : (!Ms.b(0) || Ms.br(0))) // finish dragging
      {
         if(dragging())
            if(GuiObj *lit=(touch ? objAtPos(touch->pos()) : msLit()))
         {
            if(_drag_name.is() && App.drop){Memc<Str> names; names.add(_drag_name); App.drop( names    , lit, touch ? touch->pos() : Ms.pos());} // don't use 'Memt' here, because that would reduce the stack memory, and in this method all gui object updates are called, so it's best to give them as much as possible, and also this is called only on drag finish, so once in a long time
            if(_drag_finish               )                                     _drag_finish(_drag_user, lit, touch ? touch->pos() : Ms.pos());
           _drag_cancel=null; // clear cancel so we won't call it, because we've already called finish

            if(!menu()) // if menu was not activated during custom function calls
               if(lit=(touch ? objAtPos(touch->pos()) : msLit())) // detect lit again because previous may got deleted during custom function calls
                  lit->activate(); // activate object at which we've drag and dropped
         }
         dragCancel(); // clear drag related members
      }else // start dragging
      if(!dragging() && (touch ? touch->dragging() : Ms.dragging()))
      {
        _dragging=true;
         if(_drag_start)_drag_start(_drag_user);
      }
   }

  _update_time=Time.curTime()-t;
}
/******************************************************************************/
static void DrawPanelText(C Panel *panel, C Color &panel_color, Flt padding, C TextStyleParams &ts, C Vec2 &pos, CChar *text, Bool mouse)
{
   if(Is(text))
   {
      Set(Tls16, text, ts, D.w2()-padding*2, AUTO_LINE_SPACE_SPLIT);

      Flt width =0,
          height=Tls16.elms()*ts.lineHeight();
      REPA(Tls16){TextLineSplit16 &t=Tls16[i]; MAX(width, ts.textWidth(t.text, t.length));}

      Rect_LU r(pos, width, height); r.extend(padding); if(mouse)
      {
         Int height=32; // default mouse cursor height
         if(Ms._image)height=Ms._image->h()-Ms._hot_spot.y;
         Flt y=D.pixelToScreenSize().y*height;
         r.min.y-=y;
         r.max.y-=y;
      }
      if(r.max.x> D.w()){r.min.x-=r.max.x-D.w(); r.max.x=D.w();} if(r.min.x<-D.w()){r.max.x+=-D.w()-r.min.x; r.min.x=-D.w();}
      if(r.min.y<-D.h()){r+=Vec2(0, pos.y-r.min.y+padding)    ;} if(r.max.y> D.h()){r.min.y-= r.max.y-D.h(); r.max.y= D.h();}

      if(panel        )panel->draw(panel_color, r);else
      if(panel_color.a)     r.draw(panel_color);

      ts.drawSplit(r.extend(-padding), Tls16, null, 0);
   }
}
static void DrawDescriptionObj(GuiObj &obj, C Vec2 &pos, Flt start_time, Bool mouse)
{
   Bool   immediate=false;
   CChar *text=obj.desc();
   switch(obj.type())
   {
      case GO_LIST:
      {
        _List &list=obj.asList();
         immediate=FlagTest(list.flag, LIST_IMMEDIATE_DESC);
         if(list._desc_offset>=0)
            if(Ptr data=list.screenToData(pos))text=*(Char**)((Byte*)data+list._desc_offset);
      }break;
   }
   if(Is(text) && (immediate || Time.appTime()-start_time>=Gui.desc_delay) && Gui.draw_description)Gui.draw_description(&obj, pos, text, mouse);
}
/******************************************************************************/
void DrawKeyboardHighlight(GuiObj *obj, C Rect &rect, C GuiSkin *skin)
{
   if(skin && skin->keyboard_highlight_color.a)
      rect.draw(skin->keyboard_highlight_color, false);
}
void DrawDescription(GuiObj *obj, C Vec2 &pos, CChar *text, Bool mouse)
{
   if(Gui.skin)
      if(TextStyle *text_style=Gui.skin->desc.text_style())
         DrawPanelText(Gui.skin->desc.normal(), Gui.skin->desc.normal_color, Gui.skin->desc.padding, *text_style, pos, text, mouse);
}
void DrawIMM(GuiObj *obj)
{
   if((Kb.immBuffer().is() || Kb.immCandidate().elms()) && obj && obj->type()==GO_TEXTLINE)
      if(Gui.skin)
         if(TextStyle *text_style=Gui.skin->imm.text_style())
   {
      TextLine      &textline=obj->asTextLine();
      TextEdit       edit; edit.cur=Kb.immCursor(); if(Kb.immCandidate().elms())edit.sel=Kb.immSelection().y;
      TextStyleParams ts=*text_style; if(Kb.immBuffer().is())ts.edit=&edit;
      Str              t=Kb.immBuffer(); if(Kb.immCandidate().elms()){if(t.is())t+='\n'; FREPA(Kb.immCandidate())t.space()+=S+(i+1)%10+Kb.immCandidate()[i];} // use %10 so "10" will be displayed as "0" (because pressing 0 button activates that candidate)
      DrawPanelText(Gui.skin->imm.normal(), Gui.skin->imm.normal_color, Gui.skin->imm.padding, ts, textline.screenPos()+Vec2(0, -textline.rect().h()), t, false);
   }
}
/******************************************************************************/
void GUI::draw()
{
   SyncLocker locker(_lock);

   if(_desktop)
   {
     _desktop->asDesktop().draw();
      D.clip(); // reset clip after drawing all gui objects

      // show description
      if(draw_description)
      {
         if(      _desc)                                                                                         DrawDescriptionObj(*      _desc, Ms   .pos(),       _desc_time, true );
         if(_touch_desc)REPA(Touches){Touch &touch=Touches[i]; if(touch.stylus() && touch.guiObj()==_touch_desc){DrawDescriptionObj(*_touch_desc, touch.pos(), _touch_desc_time, false); break;}}
      }

      // show imm
      if(draw_imm && kb()->is(GO_TEXTLINE))draw_imm(kb());
   }
}
/******************************************************************************/
void GUI::del()
{
#if 0
   image_shadow   .clear();
   image_drag     .clear();
   image_resize_x .clear();
   image_resize_y .clear();
   image_resize_ld.clear();
   image_resize_lu.clear();
   image_resize_ru.clear();
   image_resize_rd.clear();

   skin.clear();
#endif

   MsgBox::MsgBoxs.del();
  _desktops       .del();
  _callbacks      .del();
   Tls8           .del();
   Tls16          .del();
   GuiSkins       .del();
   Panels         .del();
   PanelImages    .del();
}
void GUI::create()
{
   if(LogInit)LogN("GUI.create");

   if(D._can_draw) // will cause Exit on Linux without XDisplay, so skip loading if can't draw
      if(skin=default_skin)
   {
      EmptyGuiSkin.list.   cursor_color=skin->list.   cursor_color;
      EmptyGuiSkin.list.highlight_color=skin->list.highlight_color;
      EmptyGuiSkin.list.selection_color=skin->list.selection_color;
   }
   EmptyGuiSkin.region.normal_color.a=0;

   image_shadow   .get("Gui/shadow.img");
   image_drag     .get("Gui/drag.img");
   image_resize_x .get("Gui/resize_x.img");
   image_resize_y .get("Gui/resize_y.img");
   image_resize_ld.get("Gui/resize_ru.img");
   image_resize_lu.get("Gui/resize_rd.img");
   image_resize_ru.get("Gui/resize_ru.img");
   image_resize_rd.get("Gui/resize_rd.img");

  _desktops.New().activate();
}
/******************************************************************************/
}
/******************************************************************************/
