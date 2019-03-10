/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
WaypointListClass WaypointList;
/******************************************************************************/

/******************************************************************************/
   void WaypointListClass::FilterChanged(WaypointListClass &ol) {ol.setVisible();}
   void          WaypointListClass::Hide(WaypointListClass &ol) {WorldEdit.waypoint_list.set(false);}
   Str WaypointListClass::WaypointName(C Elm &elm) {return elm.waypoint ?   elm.waypoint->name               : S;}
   Str WaypointListClass::WaypointPoints(C Elm &elm) {return elm.waypoint ? S+elm.waypoint->existingPoints()   : S;}
   Str WaypointListClass::WaypointPosX(C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint->pos().x, 1) : S;}
   Str WaypointListClass::WaypointPosY(C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint->pos().y, 1) : S;}
   Str WaypointListClass::WaypointPosZ(C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint->pos().z, 1) : S;}
   void WaypointListClass::setChanged() {_changed=true;}
   WaypointListClass& WaypointListClass::create()
   {
      ::EE::Window::create(Rect_RD(D.w(), -D.h(), 0.88f, 1.2f), "Waypoint List").hide(); button[1].show(); button[2].show().func(Hide, T); flag|=WIN_RESIZABLE;

      flt y=-0.05f;
      T+=filter.create(Rect_LU(0.03f, -0.02f, 0.6f, 0.05f)).func(FilterChanged, T).desc("Find waypoint\nType waypoint name or its ID to find it"); filter.kbSet(); filter.reset.show(); filter.show_find=true; filter.hint="Find Waypoint";
      T+=show_removed.create(Rect_LU(filter.rect().ru(), filter.rect().h())).func(FilterChanged, T).focusable(false).desc("Show removed waypoints"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      T+=region.create().slidebarSize(0.04f);
      ListColumn lc[]=
      {
         ListColumn(WaypointName  , 0.35f, "Name"  ), // 0
         ListColumn(WaypointPoints, 0.11f, "Points"), // 1
         ListColumn(WaypointPosX  , 0.12f, "Pos.x" ), // 2
         ListColumn(WaypointPosY  , 0.12f, "Pos.y" ), // 3
         ListColumn(WaypointPosZ  , 0.12f, "Pos.z" ), // 4
      };
      lc[1].text_align=lc[2].text_align=lc[3].text_align=lc[4].text_align=-1;
      lc[2].precision=lc[3].precision=lc[4].precision=1;
      region+=list.create(lc, Elms(lc)); list.flag|=LIST_TYPE_SORT|LIST_RESIZABLE_COLUMNS; list.cur_mode=LCM_ALWAYS;
      list.elmHeight(0.035f).textSize(0, 1).columnHeight(0.045f);

      rect(rect());
      return T;
   }
   void WaypointListClass::setCur()
   {
      list.cur=-1; FREPA(list)if(Elm *data=list.visToData(i))if(data->waypoint==WorldEdit.sel_waypoint){list.cur=i; break;}
   }
   void WaypointListClass::setVisible()
   {
      UID  filter_id;
      bool filter_is_id=filter_id.fromText(filter());
      Memt<bool> visible;
      FREPA(data)
      {
         Elm &elm=data[i];
         bool vis=false;
         if(elm.waypoint)
            if(show_removed() || (!elm.waypoint->removed && elm.waypoint->hasExisting()))
         {
            if(filter_is_id)
            {
               if(C UID *id=WorldEdit.ID(elm.waypoint))vis=(filter_id==*id);
            }else
            if(filter().is())vis=ContainsAll(elm.waypoint->name, filter());
            else             vis=true;
         }
         visible.add(vis);
      }
      list.setData(data, visible);
      setCur();
   }
   void WaypointListClass::set()
   {
      data.clear();
      FREPA(WorldEdit.waypoints)
      {
         EditWaypoint &wp=WorldEdit.waypoints.lockedData(i);
         data.New().waypoint=&wp;
      }
      setVisible();
     _changed=false;
   }
   Rect WaypointListClass::sizeLimit()C 
{
      Rect   r=::EE::Window::sizeLimit(); r.min.set(0.4f, 0.3f);
      return r;
   }
           C Rect& WaypointListClass::rect()C {return ::EE::Window::rect();}
   Window& WaypointListClass::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      region.rect(Rect(0, -clientHeight(), clientWidth(), filter.rect().min.y).extend(-0.03f));
      show_removed.rect(Rect_RU(region.rect().max.x, filter.rect().max.y, filter.rect().h(), filter.rect().h()));
      filter.size(Vec2(region.rect().w()-show_removed.rect().w(), filter.rect().h()));
      return T;
   }
   void WaypointListClass::update(C GuiPC &gpc)
{
      visible(Mode()==MODE_WORLD && WorldEdit.mode()==WorldView::WAYPOINT && WorldEdit.waypoint_list());
      if(visible() && gpc.visible && _changed)set(); // first set data
      int cur=list.cur;
      ::EE::ClosableWindow::update(gpc); // now update list
      if(visible() && gpc.visible)
      {
         REPA(MT)if(MT.guiObj(i)==&list && MT.bp(i))if(Elm *elm=list())
         {
            if(elm->waypoint && MT.bd(i) && list.selMode()==LSM_SET)WorldEdit.v4.moveTo(elm->waypoint->pos()); // camera center
         }
         if(cur!=list.cur)if(Elm *elm=list())if(elm->waypoint)WorldEdit.selWaypoint(elm->waypoint, elm->waypoint->firstExisting());
      }
   }
   void WaypointListClass::removed(EditWaypoint &waypoint)
   {
      REPA(data)if(data[i].waypoint==&waypoint){data[i].waypoint=null; setChanged(); break;}
   }
WaypointListClass::WaypointListClass() : _changed(false) {}

WaypointListClass::Elm::Elm() : waypoint(null) {}

/******************************************************************************/
