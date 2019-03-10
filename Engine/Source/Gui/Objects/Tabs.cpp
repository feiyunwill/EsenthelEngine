/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// TAB
/******************************************************************************/
void Tab::   addChild(GuiObj &child) {_children.add   (child, T);}
void Tab::removeChild(GuiObj &child) {_children.remove(child   );}

Tab& Tab::del()
{
  _children.del();
   super::del(); return T;
}
Tab& Tab::create(C Str &text)
{
  _children.del();
   super::create(text);
  _type       =GO_TAB;
   mode       =BUTTON_TOGGLE;
  _sub_type   =BUTTON_TYPE_TAB_HORIZONTAL; // set initial sub type (this is the best guess)
  _focusable  =false;
  _pixel_align=false;
   return T;
}
Tab& Tab::create(C Tab &src)
{
   if(this!=&src)
   {
     _children.del();
      if(!src.is())del();else
      {
         super::create(src);
        _type=GO_TAB;
      }
   }
   return T;
}
Tab& Tab::setCornerTab(Bool right, Bool top)
{
  _sub_type=(top ? right ? BUTTON_TYPE_TAB_TOP_RIGHT    : BUTTON_TYPE_TAB_TOP_LEFT
                 : right ? BUTTON_TYPE_TAB_BOTTOM_RIGHT : BUTTON_TYPE_TAB_BOTTOM_LEFT);
   return T;
}
Tab& Tab::text(C Str &text)
{
   if(!Equal(super::text, text, true))
   {
      super::text=text;
      if(parent()->is(GO_TABS)) // Tab rect may depend on text width
      {
         Tabs &tabs=parent()->asTabs();
         if(tabs.autoSize() && tabs.actualLayout()==TABS_HORIZONTAL)tabs.setRect();
      }
   }
   return T;
}

Bool Tab::load(File &f, CChar *path)
{
   del();
   if(super::load(f, path))
   {
     _type=GO_TAB;
      return true;
   }
   return false;
}
/******************************************************************************/
// TABS
/******************************************************************************/
void Tabs::zero()
{
  _valid         =false;
  _auto_size     =false;
  _layout        =_actual_layout=TABS_AUTO;
  _sel           =-1;
  _space         =0;
  _func          =null;
  _func_user     =null;
  _func_immediate=false;
}
Tabs::Tabs() {zero();}
Tabs& Tabs::del()
{
  _tabs.del  ();
  _skin.clear();
   super::del(); zero(); return T;
}
void Tabs::setButtonSubType()
{
   // detect first and last visible tab
   Int first_i=0, last_i=0;
   FREPA(T)if(tab(i).visible()){first_i=i; break;}
    REPA(T)if(tab(i).visible()){ last_i=i; break;}

   // setup sub types
   BUTTON_TYPE first, middle, last;
   if(_actual_layout==TABS_HORIZONTAL){first=BUTTON_TYPE_TAB_LEFT; middle=BUTTON_TYPE_TAB_HORIZONTAL; last=BUTTON_TYPE_TAB_RIGHT ;}
   else                               {first=BUTTON_TYPE_TAB_TOP ; middle=BUTTON_TYPE_TAB_VERTICAL  ; last=BUTTON_TYPE_TAB_BOTTOM;}

   REPA(T)
   {
      Tab &tab=T.tab(i);
      if(tab._sub_type<BUTTON_TYPE_TAB_CORNER_START || tab._sub_type>BUTTON_TYPE_TAB_CORNER_END) // if wasn't set to be a corner
         tab._sub_type=((i==first_i) ? first : (i==last_i) ? last : middle);
   }
}
void Tabs::setParams()
{
  _type=GO_TABS;
   REPA(T)
   {
      Tab &tab=T.tab(i); tab._parent=this; tab._pixel_align=false;
   }
   setButtonSubType();
}
Tabs& Tabs::create(CChar8 **text, Int num, Bool auto_size)
{
   del();

  _visible=true;
  _tabs.setNum(num);
   FREPAO(_tabs).create(text ? text[i] : null); // no need to apply skin for tabs because at this point Tabs skin is null
   T._auto_size=auto_size;
   setParams();
   return T;
}
Tabs& Tabs::create(CChar **text, Int num, Bool auto_size)
{
   del();

  _visible=true;
  _tabs.setNum(num);
   FREPAO(_tabs).create(text ? text[i] : null); // no need to apply skin for tabs because at this point Tabs skin is null
   T._auto_size=auto_size;
   setParams();
   return T;
}
Tabs& Tabs::create(C Tabs &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         del();
         copyParams(src);
        _type          =GO_TABS;
        _skin          =src._skin;
        _valid         =src._valid;
        _auto_size     =src._auto_size;
        _layout        =src._layout;
        _actual_layout =src._actual_layout;
        _sel           =src._sel;
        _space         =src._space;
        _func          =src._func;
        _func_user     =src._func_user;
        _func_immediate=src._func_immediate;
        _tabs.setNum(src.tabs()); FREPAO(_tabs).create(src.tab(i))._parent=this;
      }
   }
   return T;
}
void Tabs::removeChild(GuiObj &child)
{
   FREPA(T)tab(i).removeChild(child);
}
/******************************************************************************/
Tabs& Tabs::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void Tabs::call(Bool sound)
{
   if(sound)Gui.playClickSound();
   if(_func)if(_func_immediate)_func(_func_user);else Gui.addFuncCall(_func, _func_user);
}
/******************************************************************************/
TABS_LAYOUT Tabs::actualLayout()C
{
   TABS_LAYOUT layout=T.layout();
   if(layout==TABS_AUTO)
   {
      Flt aspect=2.8f, w=rect().w(), h=rect().h();
      layout=((AbsScale((w/_tabs.elms())/h, aspect) < AbsScale(w/(h/_tabs.elms()), aspect)) ? TABS_HORIZONTAL : TABS_VERTICAL);
   }
   return layout;
}
void Tabs::setRect()
{
   if(_tabs.elms())
   {
      Flt w=rect().w(),
          h=rect().h();

     _actual_layout=actualLayout();
      setButtonSubType();
      if(_actual_layout==TABS_HORIZONTAL)
      {
         w-=(_tabs.elms()-1)*space();
         if(autoSize())
         {
            Flt x=rect().min.x, text_width=0, size=h;
            FREPA(T)text_width+=tab(i).textWidth(&size); Flt padd=(w-text_width)/_tabs.elms();
            FREPA(T)
            {
               Tab &tab=T.tab(i); Flt w=tab.textWidth(&size)+padd;
               tab.rect(Rect(x, rect().min.y, x+w, rect().max.y));
               x+=w+space();
            }
         }else
         {
            w/=_tabs.elms();
            REPA(T)tab(i).rect(Rect(rect().min.x+(w+space())*i, rect().min.y, rect().min.x+(w+space())*i+w, rect().max.y));
         }
      }else
      {
         h-=(_tabs.elms()-1)*space(); h/=_tabs.elms();
         REPA(T)tab(i).rect(Rect(rect().min.x, rect().max.y-(h+space())*i-h, rect().max.x, rect().max.y-(h+space())*i));
      }
   }
}
Tabs& Tabs::rect(C Rect &rect, Flt space, Bool auto_size)
{
   if(T.rect()!=rect || T.space()!=space || T.autoSize()!=auto_size)
   {
      T._rect     =rect;
      T._space    =space;
      T._auto_size=auto_size;
      setRect();
   }
   return T;
}
/******************************************************************************/
Tabs& Tabs::rect(C Rect &rect) {return T.rect(rect, space(), autoSize());}
/******************************************************************************/
Tabs& Tabs::layout(TABS_LAYOUT layout)
{
   MIN(layout, TABS_NUM);
   if(T.layout()!=layout)
   {
      T._layout=layout;
      setRect();
   }
   return T;
}
/******************************************************************************/
Tabs& Tabs::valid(Bool on) {T._valid=on; return T;}
Tabs& Tabs::set(Int i, SET_MODE mode)
{
   if(!InRange(i, _tabs))
   {
      if(valid())return T; // don't set to -1 if 'valid' is enabled
      i=-1; // set sel to -1
   }
   if(T()!=i)
   {
      Bool kb=contains(Gui.kb());
      if(valid() && InRange(T(), T)) // set highlight for previously selected tab if there's a hoverable MT above it
      {
         Tab &tab=T.tab(T());
         Bool lit=false; if(enabled())REPA(MT)if(MT.guiObj(i)==&tab && MT.hoverable(i)){lit=true; break;}
         tab._lit=lit;
      }
     _sel=i; REPAD(j, T)tab(j).set(i==j);
      if(valid() && InRange(T(), T))tab(T())._lit=0; // disable highlight for newly selected tab
      if(kb)if(InRange(T(), T))tab(T()).kbSet();else kbSet();
      if(mode!=QUIET)call(mode!=NO_SOUND);
   }
   return T;
}
Tabs& Tabs::toggle(Int i, SET_MODE mode) {return set((T()==i) ? -1 : i, mode);}
/******************************************************************************/
Tabs& Tabs::skin(C GuiSkinPtr &skin)
{
  _skin=skin;
   REPAO(_tabs).skin=skin;
   return T;
}
/******************************************************************************/
Tabs& Tabs::move(C Vec2 &delta)
{
   if(delta.any())
   {
             super::move(delta);
      REPA(T)tab(i).move(delta);
   }
   return T;
}
/******************************************************************************/
Tab& Tabs::New(C Str &text, Int i)
{
   if(i<0)i=tabs();else if(i>tabs())i=tabs();
   if(_sel>=i)_sel++;
   Tab &tab=_tabs.NewAt(i).create(text); tab._parent=this; tab.skin=skin();
   setRect();
   return tab;
}
Tabs& Tabs::remove(Int i)
{
   if(InRange(i, T))
   {
      if(_sel==i)_sel=-1;else
      if(_sel> i)_sel--;
     _tabs.removeValid(i, true);
      setRect();
   }
   return T;
}
/******************************************************************************/
Tabs& Tabs::desc(C Str &desc)
{
          super::desc(desc);
   REPA(T)tab(i).desc(desc);
   return T;
}
/******************************************************************************/
GuiObj* Tabs::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible() && gpc.visible && Cuts(pos, gpc.clip))
   {
      if(InRange(T(), T))if(GuiObj *go=tab(T())._children.test(gpc, pos, mouse_wheel))return go;
      if(Cuts(pos, rect()+gpc.offset))
      {
         REPA(T)if(GuiObj *go=tab(i).test(gpc, pos, mouse_wheel))return go;
       //return this; // don't return self
      }
   }
   return null;
}
/******************************************************************************/
void Tabs::update(C GuiPC &gpc)
{
   GuiPC gpc2(gpc, visible(), enabled());
   Bool  enabled=gpc2.enabled;
   FREPA(T)
   {
      gpc2.enabled=(enabled && !(valid() && i==T())); // force disabled if 'valid' is enabled and we're processing activated Tab (this way we can't click pushed Tab)
      Tab &tab=T.tab(i); tab.update(gpc2);
      if((T()==i)!=tab())toggle(i); // if Tab update state changed
   }
   gpc2.enabled=enabled; // restore 'enabled' after loop, because it was changed inside it
   Bool visible=gpc2.visible;
   FREPA(T)
   {
      gpc2.visible=(visible && i==T());
      tab(i)._children.update(gpc2);
   }
}
void Tabs::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      setButtonSubType();
      Rect     r=rect()+gpc.offset; Vec2 ofs=D.alignScreenToPixelOffset(r.lu()); r+=ofs;
      GuiPC gpc2(gpc, visible(), enabled()); gpc2.offset+=ofs; // adjust offset so that all tabs have the same pixel alignment as the first one
      FREPA(T)  tab(i  ).          draw(gpc2);
      if(T()>=0)tab(T())._children.draw(gpc );
   }
}
/******************************************************************************/
Bool Tabs::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.putMulti(Byte(1), _valid, _auto_size, _sel, _space, _layout); // version
      f._putAsset(_skin.name(path));
      f.cmpUIntV(_tabs.elms()); FREPA(_tabs)if(!_tabs[i].save(f, path))return false;
      return f.ok();
   }
   return false;
}
Bool Tabs::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 1:
      {
         f.getMulti(_valid, _auto_size, _sel, _space, _layout);
        _skin.require(f._getAsset(), path);
        _tabs.clear().setNum(f.decUIntV()); FREPA(_tabs)if(!_tabs[i].load(f, path))goto error;
         if(f.ok())
         {
           _actual_layout=actualLayout();
            setParams();
            return true;
         }
      }break;

      case 0:
      {
         f._getStr();
         f>>_valid>>_auto_size>>_sel>>_space>>_layout;
        _tabs.clear().setNum(f.getInt()); FREPA(_tabs){Tab &tab=_tabs[i]; if(!tab.load(f, path))goto error; tab.enabled(true);} // restore enabled because in the past tabs were disabled on click
         if(f.ok())
         {
           _actual_layout=actualLayout();
            setParams();
            return true;
         }
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
void Tab::parentClientRectChanged(C Rect *old_client, C Rect *new_client)
{
   notifyChildrenOfClientRectChange(old_client, new_client); // pass on to the children
}
void Tabs::parentClientRectChanged(C Rect *old_client, C Rect *new_client)
{
   notifyChildrenOfClientRectChange(old_client, new_client); // pass on to the children
}
/******************************************************************************/
}
/******************************************************************************/
