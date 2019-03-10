/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Menu is always added to the Desktop, and not the parent, this is so that other elements don't occlude menus.
      Setting just higher 'GuiObj.baseLevel' would not be enough, because that's only within the parent.

         For example, 2 Comboboxes with their own menu added on the desktop:
            -Desktop
               -ComboBox
                  -ComboBox.Menu
               -ComboBox
                  -ComboBox.Menu

         Menu from first ComboBox would be occluded by second ComboBox.

         Instead Menu's are added to the root
            -Desktop
               -ComboBox
               -ComboBox
               -ComboBox.Menu
               -ComboBox.Menu

         baseLevel is still used, to put them always on top of other objects (including windows)

   Menu's are sorted in the Desktop children array, by their number of Parents/Owners,
      so, a Menu belonging to Window, will be before a Menu belonging to Desktop.
      -Desktop
         -WindowMenu
         -DesktopMenu
      This is so that Window Menu's can be processed before Desktop, to check for keyboard shortcuts first.

      However current implementation is not perfect, because:
         Menu's are sorted in Desktop children array at the moment when they're added to the Desktop.
         However at this point, parents of Menu Owners may not be fully set yet.
         And when Owners are assigned to other Parents, then the Menu's that belong to them, are not re-sorted.

      TODO: solve somehow the problem of processing keyboard shortcuts for leaf Menu's first, because currently Menu's are not always sorted correctly.

/******************************************************************************/
GuiPC::GuiPC(C GuiPC &old, Menu &menu)
{
   T=old;
   visible   &=menu.visible();
   enabled   &=menu.enabled(); if(menu._owner)enabled&=menu._owner->enabledFull();
   client_rect=menu._crect;
   clip       =client_rect; // set instead of intersection because child Menu's would be clipped fully
   offset     =client_rect.lu();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void MenuElm::zero()
{
   on       =0;
   disabled =0;
  _flag     =0;
  _func_immediate=false;
  _func_user     =null;
  _func          =null;
  _func2         =null;
  _menu          =null;
}
MenuElm::MenuElm() {zero();}
MenuElm& MenuElm::del()
{
   Delete(_menu);
   zero(); return T;
}
MenuElm& MenuElm::create(C Str &name, void (*func)(), Bool immediate)
{
   del();

   T.name=T.display_name=name;
   T.func(func, immediate);

   return T;
}
MenuElm& MenuElm::create(C Str &name, void (*func)(Ptr), Ptr user, Bool immediate)
{
   del();

   T.name=T.display_name=name;
   T.func(func, user, immediate);

   return T;
}
MenuElm& MenuElm::create(C MenuElm &src, Menu *parent)
{
   if(this!=&src)
   {
      del();

      T.name=src.name;
      T.display_name=src.display_name;
      T.on=src.on;
      T.disabled=src.disabled;

      if(src._menu)New(T._menu)->create(*src._menu)._parent=parent;

      T._flag          =src._flag;
      T._func_immediate=src._func_immediate;
      T._func_user     =src._func_user;
      T._func          =src._func;
      T._func2         =src._func2;
      T._desc          =src._desc;
      T._kbsc          =src._kbsc;
      T._kbsc2         =src._kbsc2;
   }
   return T;
}
/******************************************************************************/
MenuElm& MenuElm::flag   (  Byte  flag) {T._flag =flag; return T;}
MenuElm& MenuElm::kbsc   (C KbSc &kbsc) {T._kbsc =kbsc; return T;}
MenuElm& MenuElm::kbsc2  (C KbSc &kbsc) {T._kbsc2=kbsc; return T;}
MenuElm& MenuElm::desc   (C Str  &desc) {T._desc =desc; return T;}
MenuElm& MenuElm::setOn  (  Bool  on  ) {T. on   =on  ; return T;}
MenuElm& MenuElm::display(C Str  &name) {if(name.is())display_name=name; return T;}
MenuElm& MenuElm::func   (void (*func)(        ),           Bool immediate) {T._func=func; T._func2=null; T._func_user=null; T._func_immediate=immediate; return T;} // clear the other function as only one function type should be available at a time
MenuElm& MenuElm::func   (void (*func)(Ptr user), Ptr user, Bool immediate) {T._func=null; T._func2=func; T._func_user=user; T._func_immediate=immediate; return T;} // clear the other function as only one function type should be available at a time
void     MenuElm::call   ()
{
   if(_func )if(_func_immediate)_func (          );else Gui.addFuncCall(_func             );
   if(_func2)if(_func_immediate)_func2(_func_user);else Gui.addFuncCall(_func2, _func_user);
}
/******************************************************************************/
Bool MenuElm::pushable()
{
   return (flag()&MENU_TOGGLABLE) || _func || _func2;
}
void MenuElm::push()
{
   if(flag()&MENU_TOGGLABLE)on^=1;
   call();
}
void Menu::push(C Str &elm)
{
   Str path=elm;
   for(GuiObj *go=this; go->is(GO_MENU); go=go->parent())if(go->asMenu()._func)
   {
      GuiObj *menu  =this,
             *parent=menu->parent();
      for(; parent->is(GO_MENU); )
      {
         Menu &parent_menu=parent->asMenu();
         FREP( parent_menu.elms())if(parent_menu.elm(i).menu()==menu)
         {
            path=parent_menu.elm(i).name+'\\'+path;
            goto found;
         }
         return;
      found:;
         menu  =parent;
         parent=parent->parent();
      }
      go->asMenu()._func(path, go->asMenu()._func_user);
      break;
   }
}
/******************************************************************************/
void Menu::zero()
{
  _crect.zero();
  _no_child_draw=false;
  _selectable_offset=-1;
  _kb        =null;
  _owner     =null;
  _func_user =null;
  _func      =null;
  _base_level=GBL_MENU;
}
Menu::Menu() {zero();}
Menu& Menu::del()
{
  _skin.clear();
   list.del();
  _elms.del();
   super::del(); zero(); return T;
}
Menu& Menu::create()
{
   del();

  _type   =GO_MENU;
  _visible=false; // keep hidden at creation stage
  _kb     =&list;
   list.create(); list._parent=this;
   list.cur_mode=LCM_MOUSE;
   list.flag   |= LIST_ROLLABLE;
   list.flag   &=~LIST_SEARCHABLE;
   setSize();
   return T;
}
Menu& Menu::create(C Menu &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type=GO_MENU;

        _kb=_owner=null;
        _no_child_draw    =src._no_child_draw;
        _selectable_offset=src._selectable_offset;
        _func_user        =src._func_user;
        _func             =src._func;
        _crect            =src._crect;
        _skin             =src._skin;

         list.create(src.list)._parent=this;

        _elms.setNum(src.elms()); FREPAO(_elms).create(src._elms[i], this);
      }
   }
   return T;
}
/******************************************************************************/
// GET
/******************************************************************************/
GuiObj* Menu::owner()C {return Owner();}

void Menu::operator()(C Str &command, Bool on, SET_MODE mode)
{
   CChar *start=_GetStart(command);
   if( Is(start))REPA(_elms)
   {
      MenuElm &elm=_elms[i];
      if(Equal(elm.name, start))
      {
         start=_GetStartNot(command);
         if(!Is(start))if(elm.on!=on)
         {
            elm.on^=true;
            if(mode!=QUIET)elm.call();
         }
         if(Menu *menu=elm.menu())(*menu)(start, on, mode);
         break;
      }
   }
}
Bool Menu::operator()(C Str &command)C
{
   CChar *start=_GetStart(command);
   if( Is(start))REPA(_elms)
   {
    C MenuElm &elm=_elms[i];
      if(Equal(elm.name, start))
      {
         start=_GetStartNot(command);
         if(!Is(start)           )return elm.on;
         if(Menu *menu=elm.menu())return (*menu)(start);
                                  return false;
      }
   }
   return false;
}
Bool Menu::exists(C Str &command)C
{
   CChar *start=_GetStart(command);
   if( Is(start))REPA(_elms)
   {
    C MenuElm &elm=_elms[i];
      if(Equal(elm.name, start))
      {
         start=_GetStartNot(command);
         if(!Is(start)           )return true;
         if(Menu *menu=elm.menu())return menu->exists(start);
                                  return false;
      }
   }
   return false;
}
/******************************************************************************/
// SET
/******************************************************************************/
static MemberDesc MenuElmFlag(MEMBER(MenuElm, _flag));
Menu& Menu::setData(CChar8 *data[], Int elms, C MemPtr<Bool> &visible, Bool keep_cur)
{
   ListColumn columns[]=
   {
      ListColumn(),
   };
   columns[0].width  =LCW_MAX_DATA_PARENT;
   columns[0].md.type=DATA_CHAR8_PTR;

   setColumns(columns, Elms(columns), true);
   setData<CChar8*>(data, elms, visible, keep_cur); // call not self but the template
   return T;
}
Menu& Menu::setData(CChar *data[], Int elms, C MemPtr<Bool> &visible, Bool keep_cur)
{
   ListColumn columns[]=
   {
      ListColumn(),
   };
   columns[0].width  =LCW_MAX_DATA_PARENT;
   columns[0].md.type=DATA_CHAR_PTR;

   setColumns(columns, Elms(columns), true);
   setData<CChar*>(data, elms, visible, keep_cur); // call not self but the template
   return T;
}
Menu& Menu::setData(C Node<MenuElm> &node)
{
   Bool children   =false, togglable=false;
   Flt  width_name =0,
        width_kbsc =0,
        width_kbsc2=0;
   GuiSkin   *skin      =list.getSkin();
   TextStyle *text_style=(skin ? skin->list.text_style() : null); TextStyleParams ts(text_style, false); ts.size=list.textSizeActual();
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
   if(!ts.font() && skin)ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#endif

  _elms.clear().setNum(node.children.elms()); Memt<Bool> visible; visible.setNum(_elms.elms());
   FREPA(_elms)
   {
    C Node<MenuElm> &child= node.children[i];
           MenuElm  &elm  =_elms         [i];
      elm.create(child, this);
      elm.disabled|=!elm.name.is();
      visible[i]   =!FlagTest(elm.flag(), MENU_HIDDEN);
      if(child.children.elms() && !elm._menu) // if menu was already created, then ignore the following code
      {
         Menu &menu=New(elm._menu)->create(); menu._parent=this;
         menu.skin     (   T.skin(), false);
         menu.list.skin(list.skin());
         menu.setData(child); // call 'setData' after setting the skin
      }
      children|=(elm._menu!=null);
      if(visible[i])
      {
         togglable|=FlagTest(elm.flag(), MENU_TOGGLABLE);
                             MAX(width_name , ts.textWidth(elm.display_name    ));
         if(elm.kbsc ().is())MAX(width_kbsc , ts.textWidth(elm.kbsc ().asText()));
         if(elm.kbsc2().is())MAX(width_kbsc2, ts.textWidth(elm.kbsc2().asText()));
      }
   }

   ListColumn columns[]=
   {
      ListColumn(MEMBER(MenuElm,  on          ), 0.05f, "on"   ), // 0
      ListColumn(MEMBER(MenuElm,  display_name), 0.00f, "name" ), // 1
      ListColumn(MEMBER(MenuElm, _kbsc        ), 0.00f, "kbsc" ), // 2
      ListColumn(MEMBER(MenuElm, _kbsc2       ), 0.00f, "kbsc2"), // 3
      ListColumn(MEMBER(MenuElm, _menu        ), 0.05f, "menu" ), // 4
   }; ASSERT(MENU_COLUMN_CHECK==0 && MENU_COLUMN_NAME==1 && MENU_COLUMN_KBSC==2 && MENU_COLUMN_KBSC2==3 && MENU_COLUMN_SUB==4 && MENU_COLUMN_NUM==5);
   columns[MENU_COLUMN_CHECK].md.type=DATA_CHECK;
   columns[MENU_COLUMN_CHECK].sort=&MenuElmFlag; // use 'sort' to point to '_flag' so we can access 'MENU_TOGGLABLE' in 'DataGuiImage'
   columns[MENU_COLUMN_NAME ].width  =width_name +ts.size.x*(0.5f+((width_kbsc || width_kbsc2) ? 0.5f : 0));
   columns[MENU_COLUMN_KBSC ].width  =width_kbsc +ts.size.x* 0.5f;
   columns[MENU_COLUMN_KBSC2].width  =width_kbsc2+ts.size.x*(0.5f+(               width_kbsc   ? 0.5f : 0)); // keyboard shortcuts are right-aligned, so we need to add space to kbsc2 (on the right) if kbsc (on the left) exists
   columns[MENU_COLUMN_CHECK].visible(togglable     );
   columns[MENU_COLUMN_KBSC ].visible(width_kbsc !=0);
   columns[MENU_COLUMN_KBSC2].visible(width_kbsc2!=0);
   columns[MENU_COLUMN_SUB  ].visible(children   !=0);

   disabled(node.disabled);
   list.setColumns(columns, Elms(columns), true).setData(_elms, visible).setElmDesc(MEMBER(MenuElm, _desc));

   return T;
}
Menu& Menu::setColumns(ListColumn *column, Int columns, Bool columns_hidden)
{
  _elms.del();
   list.setColumns(column, columns, columns_hidden);
   return T;
}
/******************************************************************************/
Menu& Menu::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   T._skin=skin;
   if(sub_objects)
   {
      list.skin(skin);
      REPA(_elms)if(Menu *menu=_elms[i].menu())menu->skin(skin, true);
   }
   return T;
}
/******************************************************************************/
Menu& Menu::setSize(Bool touch)
{
   GuiSkin *skin=getSkin();
   Flt h=(skin ? skin->menu.list_elm_height : 0.043f);
   list.elmHeight(h*(touch ? 2 : 1)).textSize(h, 0);
   REPA(_elms)if(Menu *menu=_elms[i].menu())menu->setSize(touch);
   return T;
}
/******************************************************************************/
Menu& Menu::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
     _crect=rect;
      if(GuiSkin *skin=getSkin())
      {
         if(C PanelPtr &panel=skin->menu.normal)
         {
            Rect padd; panel->innerPadding(T.rect(), padd);
           _crect.min+=padd.min;
           _crect.max-=padd.max;
         }
        _crect.min+=skin->menu.padding;
        _crect.max-=skin->menu.padding;
         if(!_crect.validX())_crect.setX(_crect.centerX());
         if(!_crect.validY())_crect.setY(_crect.centerY());
      }
      super::rect(rect); // call this last
   }
   return T;
}
Menu& Menu::move(C Vec2 &delta)
{
 //if(delta.any()) looks fast so skip this check
   {
      super::move(delta);
     _crect+=delta;
   }
   return T;
}
Menu& Menu::moveClamp(C Vec2 &delta)
{
   Vec2 d=delta;
   if(rect().max.x+d.x> D.w())d.x= D.w()-rect().max.x;
   if(rect().min.y+d.y<-D.h())d.y=-D.h()-rect().min.y;
   if(rect().min.x+d.x<-D.w())d.x=-D.w()-rect().min.x;
   if(rect().max.y+d.y> D.h())d.y= D.h()-rect().max.y;
   return move(d);
}
Menu& Menu::pos  (C Vec2 &pos) {return moveClamp(pos-T.pos  ());}
Menu& Menu::posRU(C Vec2 &pos) {return moveClamp(pos-T.posRU());}
Menu& Menu::posLD(C Vec2 &pos) {return moveClamp(pos-T.posLD());}
Menu& Menu::posL (C Vec2 &pos) {return moveClamp(pos-T.posL ());}
Menu& Menu::posR (C Vec2 &pos) {return moveClamp(pos-T.posR ());}
Menu& Menu::posD (C Vec2 &pos) {return moveClamp(pos-T.posD ());}
Menu& Menu::posU (C Vec2 &pos) {return moveClamp(pos-T.posU ());}
Menu& Menu::posC (C Vec2 &pos) {return moveClamp(pos-T.posC ());}

Menu& Menu::posAround(C Rect &rect, Flt align)
{
   Vec2 size=T.size(),
        pos =rect.ld(); pos.x+=Lerp(rect.w()-size.x+paddingR(), -paddingL(), LerpR(-1.0f, 1.0f, align));
   Rect screen_rect(0, -D.h(), 0, D.h()); // only Y are needed
   Flt  h_below=(Rect_LU(0, pos.y         , 0, size.y)&screen_rect).h(), // visible height when below the 'rect'
        h_above=(Rect_LD(0, pos.y+rect.h(), 0, size.y)&screen_rect).h(); // visible height when above the 'rect'

   return T.pos((h_above>h_below+EPS) ? pos+Vec2(0, rect.h()+size.y) : pos); // select position according to which visible height is bigger, use EPS to more often place the Menu below
}
void Menu::clientSize(C Vec2 &size)
{
   Vec2 s=size;
   if(GuiSkin *skin=getSkin())
   {
      if(Panel *panel=skin->menu.normal())
      {
         Rect padd; panel->innerPadding(T.rect(), padd);
         s+=padd.min;
         s+=padd.max;
      }
      s+=skin->menu.padding*2;
   }
   T.size(s);
}
Menu& Menu::show()
{
   if(hidden())
   {
      list.cur=-1;
      super::show();
   }
   return T;
}
/******************************************************************************/
Menu& Menu::clearElmSelectable(                ) {_selectable_offset=-1                        ; return T;}
Menu& Menu::  setElmSelectable(Bool &selectable) {_selectable_offset=UInt(UIntPtr(&selectable)); return T;}
/******************************************************************************/
Menu& Menu::func(void (*func)(C Str &path, Ptr user), Ptr user)
{
   T._func     =func;
   T._func_user=user;
   return T;
}
/******************************************************************************/
ListColumn* Menu::listColumn()
{
   if(elms() && InRange(1, list.columns()))return &list.column(1);
   if(          InRange(0, list.columns()))return &list.column(0);
                                           return  null;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
GuiObj* Menu::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible() && gpc.visible)
   {
      GuiPC gpc2(gpc, T);
      FREPA(_elms)if(Menu *menu=_elms[i].menu())if(GuiObj *go=menu->test(gpc2, pos, mouse_wheel))return go;
      if(Cuts(pos, rect()))
      {
         mouse_wheel=this;
         if(GuiObj *go=list.test(gpc2, pos, mouse_wheel))return go;
         return this;
      }
   }
   return null;
}
/******************************************************************************/
void Menu::hideAll() {if(GuiObj *root=last(GO_MENU))root->deactivate();}
/******************************************************************************/
void Menu::checkKeyboardShortcuts()
{
   if(enabled())REPA(_elms)
   {
      MenuElm &e=_elms[i]; if(!e.disabled)
      {
         if(e._kbsc.pd())
         {
            e._kbsc.eat();
            e.push();
              push(e.name);
         }else
         if(e._kbsc2.pd())
         {
            e._kbsc2.eat();
            e.push();
              push(e.name);
         }
         if(Menu *menu=e.menu())menu->checkKeyboardShortcuts();
      }
   }
}
static inline Flt ScrollSpeed(Menu &menu) {return menu.list.elmHeight()*14*Time.ad();} // speed of 14 elements per second
void Menu::update(C GuiPC &gpc)
{
   GuiPC gpc2(gpc, T);
   if(   gpc2.enabled)
   {
      // mouse wheel
      if(Ms.wheel() && Gui.wheel()==this)
      {
         Vec2 d(0, -list._height_ez*Ms.wheel());
         if(Ms.test(_crect+d))
         {
            move(d);
            gpc2=GuiPC(gpc, T); // reset GuiPC because move can affect position
         }
      }

      // list
      list.update(gpc2);

      if(gpc2.visible)
      {
         if(_elms.elms())
         {
            if(Ms._action)_no_child_draw=false;
            if(Gui.menu()==this)
            {
               if(Kb.k.any())_no_child_draw=true;
            }else
            {
               if(Ms._action && list.contains(Gui.ms())){                activate();                                              }else // activate self on mouse    action
               if(Kb.k(KB_LEFT) && Gui.menu()==_kb     ){Int c=list.cur; activate(); list.cur=c; Kb.eatKey(); _no_child_draw=true;}     // activate self on keyboard action
            }
         }

         // handle input
         if(Gui.menu()==this)
         {
            // scroll (priority: 1 - touch hold, 2 - mouse hover with previous mouse action, 3 - current element)
          C Vec2 *pos=null;
            REPA(Touches)if(Touches[i].on() && Gui.menu()->contains(Touches[i].guiObj())                    ){pos=&Touches[i].pos(); break;}
                                    if(!pos && Gui.menu()->contains(Gui       .ms    ()) && !list._kb_action){pos=&Ms        .pos();       }
            if(pos) // use detection basing on the 'pos' screen position
            {
               Flt margin=Max(0.0f, D.h()-paddingT()-list.elmHeight()*1.0f); // use max 0 to don't go to lower half of the screen
               if(pos->y>=margin) // top of the screen
               {
                  Flt d=D.h()-rect().max.y; if(d<0)T.move(Vec2(0, Max(d, -ScrollSpeed(T))));
               }else
               if(pos->y<=-margin) // bottom of the screen
               {
                  Flt d=-D.h()-rect().min.y; if(d>0)T.move(Vec2(0, Min(d, ScrollSpeed(T))));
               }
            }else // use detection basing on the 'list.cur'
            if(InRange(list.cur, list.visibleElms()))
            {
               Flt y=list.visToLocalY(list.cur)+gpc2.offset.y,
                   d=D.h()-(y+paddingT());
               if( d<0) // top of the screen (check this before bottom of the screen)
               {
                  T.move(Vec2(0, Max(d, -ScrollSpeed(T))));
               }else
               {
                  d=-D.h()-(y-list.elmHeight()-paddingB());
                  if(d>0) // bottom of the screen
                  {
                     T.move(Vec2(0, Min(d, ScrollSpeed(T))));
                  }
               }
            }

            // input
            Bool  enter_only=Kb.k(KB_RIGHT), by_touch=false;
          C Vec2 *rs_pos    =null; if(Ms.br(0) && Gui.menu()->contains(Gui.ms()))rs_pos=&Ms.pos(); REPA(Touches)if(Touches[i].rs() && Gui.menu()->contains(Touches[i].guiObj())){rs_pos=&Touches[i].pos(); by_touch=true;} // release pos
            if(rs_pos || enter_only || ((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first()))
            {
               Bool entered=false;
               if(InRange(list.cur, list.visibleElms()))
               {
                  if(_elms.elms())
                  {
                     if(MenuElm *e=(MenuElm*)list.visToData(list.cur))if(!e->disabled)
                     {
                        Menu *menu=e->menu();
                        if((e->flag()&MENU_NOT_SELECTABLE) // can't be selected (so enter only)
                        || enter_only                      // or we actually want to enter only
                        || (menu && !e->pushable())        // has child menu and can't be pushed
                        || (menu &&  by_touch     )        // has child menu and we're entering by touch
                        )
                        {
                           if(menu) // has child menu
                           {
                              if(0 && D.smallSize())menu->posC(0);
                              else             menu->pos(Vec2(rs_pos ? by_touch ? Avg(rs_pos->x-menu->rect().w()*0.5f, menu->rect().min.x) // move half way if by touch, to avoid finger covering menu
                                                                                :     rs_pos->x-menu->rect().w()*0.5f
                                                                                : _crect.max.x,
                                                                                rect().max.y+list.visToLocalY(list.cur))); // move the sub menu under the mouse cursor

                                            menu->activate();
                                            menu->list.cur=((menu->list.elms() && !by_touch) ? 0 : -1);
                              if(enter_only)menu->_no_child_draw=true ; // when moving right using keyboard, don't display children of selected element
                                                  _no_child_draw=false;
                              entered=true;
                           }
                        }else
                        {
                           e->push();
                              push(e->name);
                           if(!(e->flag()&(MENU_TOGGLABLE|MENU_NO_CLOSE)) && Gui.menu()==this)hideAll();
                        }
                     }
                  }else
                  {
                     if(Ptr data=list.visToData(list.cur))
                     {
                        if(_selectable_offset>=0 && !*(Bool*)((Byte*)data+_selectable_offset))goto skip;
                        if(ListColumn *lc=listColumn())push(lc->md.asText(data, lc->precision));
                     }
                     hideAll();
                  skip:;
                  }
               }

               if(enter_only && !entered)if(GuiObj *go=first(GO_MENU_BAR)) // activate next MenuBar element
               {
                  MenuBar &g=go->asMenuBar();
                  FREPA(g)if(!g.elm((g._lit+1+i)%g.elms()).menu.disabled()){g._push=(g._lit+1+i)%g.elms(); break;}
               }

               Ms.eat(0); Kb.eatKey();
            }else
            if(Kb.k(KB_LEFT))
            {
               if(GuiObj *go=first(GO_MENU_BAR)) // activate previous 'MenuBar' element
               {
                  MenuBar &g=go->asMenuBar();
                  REPA(g)if(!g.elm((g._lit+i)%g.elms()).menu.disabled()){g._push=(g._lit+i)%g.elms(); Kb.eatKey(); break;}
               }
            }else
            if(contains(Gui.ms()) && Ms.bp(2) || Kb.kf(KB_ESC)) // close all menu's (check mouse focus too in case we've just activated the Menu in this frame with the use of Ms.bp(2) in such case the Menu would get immediately closed without showing up at all)
            {
               hideAll();
               Ms.eat(2); Kb.eatKey();
            }else
            if(contains(Gui.ms()) && Ms.bp(1) || Kb.kf(KB_NAV_BACK)) // activate parent menu (check mouse focus too in case we've just activated the Menu in this frame with the use of Ms.bp(1) in such case the Menu would get immediately closed without showing up at all)
            {
               if(parent())
               {
                  parent()->activate();
                  if(parent()->type()==GO_MENU)
                  {
                     Menu &menu=parent()->asMenu();
                     menu.list.lit=menu.list.cur=-1; // immediatelly clear lit/cur to avoid 1 frame delay
                  }
               }
               Ms.eat(1); Kb.eatKey();
            }
         }

         // set child visibility
         REPA(_elms)
         {
            MenuElm &elm=_elms[i];
            if(Menu *menu=elm.menu())if(!elm.disabled)
            {
               if(Gui.menu()==this && Ms._action && menu->visible() && menu->contains(Gui.ms()))menu->activate(); // activate child on mouse action
               if(menu->contains(Gui.menu()))list.cur=list.absToVis(i);

               Bool visible=(list.visToAbs(list.cur)==i && !_no_child_draw);
               if(menu->visible()!=visible)
               {
                  if(visible)
                  {
                     Flt  w=menu->rect().w(), r=D.w()-_crect.max.x+EPS, l=_crect.min.x-(-D.w()); // visible space on the right/left
                     Vec2 pos((r>=w || r>=l) // put on right side if there's enough space to fit all, or right side has more visible space
                            ? _crect.max.x : _crect.min.x-w, rect().max.y+list.visToLocalY(list.absToVis(i)));
                     menu->pos (pos); // first use 'pos' to use clamping
                     pos.x-=menu->rect().min.x; pos.y=0;
                     menu->move(pos); // then use 'move' to skip clamping to force x
                  }
                  menu->visible(visible);
               }
            }
         }
      }

      // check shortcuts
      if(Kb.k.any()) // shortcuts are detected based on char or key press, so check them only if there's something pressed
         if(!parent()->is(GO_MENU)) // don't check Menu's that are children of other Menu's, check only root Menu's, and if it's accessible, then it will check its children
            if(GuiObj *owner=Owner())
      {
         GuiObj *owner_window=owner->first(GO_WINDOW); // get owner Window (first Window that contains the owner)
         if(!owner_window                          // if there's no Window (owner is assigned to Desktop)
         ||  owner_window->contains(Gui.window())) // if      owner Window contains focused Window (focused Window is a child of owner Window)
            checkKeyboardShortcuts();
      }

      // children
      REPA(_elms)if(Menu *menu=_elms[i].menu())menu->update(gpc2);
   }
}
/******************************************************************************/
void Menu::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      if(GuiSkin *skin=getSkin())
      {
         D.clip();
         if(skin->menu.normal        )skin->menu.normal->draw(skin->menu.normal_color, rect());else
         if(skin->menu.normal_color.a)            rect().draw(skin->menu.normal_color);
      }
      GuiPC gpc2(gpc, T); list.draw(gpc2); REPA(_elms)if(Menu *menu=_elms[i].menu())menu->draw(gpc2);
   }
}
/******************************************************************************/
}
/******************************************************************************/
