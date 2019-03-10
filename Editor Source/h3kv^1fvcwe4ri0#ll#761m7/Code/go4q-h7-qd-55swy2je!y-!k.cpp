/******************************************************************************/
class WaypointListClass : ClosableWindow
{
   class Elm
   {
      EditWaypoint *waypoint=null;
   }
   bool     _changed=false;
   Button    refresh;
   TextLine  filter;
   Button    show_removed;
   Region    region;
   List<Elm> list;
   Memx<Elm> data;

   static void FilterChanged(WaypointListClass &ol) {ol.setVisible();}
   static void          Hide(WaypointListClass &ol) {WorldEdit.waypoint_list.set(false);}

   static Str WaypointName  (C Elm &elm) {return elm.waypoint ?   elm.waypoint.name               : S;}
   static Str WaypointPoints(C Elm &elm) {return elm.waypoint ? S+elm.waypoint.existingPoints()   : S;}
   static Str WaypointPosX  (C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint.pos().x, 1) : S;}
   static Str WaypointPosY  (C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint.pos().y, 1) : S;}
   static Str WaypointPosZ  (C Elm &elm) {return elm.waypoint ? TextReal(elm.waypoint.pos().z, 1) : S;}

   void setChanged() {_changed=true;}
   WaypointListClass& create()
   {
      super.create(Rect_RD(D.w(), -D.h(), 0.88, 1.2), "Waypoint List").hide(); button[1].show(); button[2].show().func(Hide, T); flag|=WIN_RESIZABLE;

      flt y=-0.05;
      T+=filter.create(Rect_LU(0.03, -0.02, 0.6, 0.05)).func(FilterChanged, T).desc("Find waypoint\nType waypoint name or its ID to find it"); filter.kbSet(); filter.reset.show(); filter.show_find=true; filter.hint="Find Waypoint";
      T+=show_removed.create(Rect_LU(filter.rect().ru(), filter.rect().h())).func(FilterChanged, T).focusable(false).desc("Show removed waypoints"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      T+=region.create().slidebarSize(0.04);
      ListColumn lc[]=
      {
         ListColumn(WaypointName  , 0.35, "Name"  ), // 0
         ListColumn(WaypointPoints, 0.11, "Points"), // 1
         ListColumn(WaypointPosX  , 0.12, "Pos.x" ), // 2
         ListColumn(WaypointPosY  , 0.12, "Pos.y" ), // 3
         ListColumn(WaypointPosZ  , 0.12, "Pos.z" ), // 4
      };
      lc[1].text_align=lc[2].text_align=lc[3].text_align=lc[4].text_align=-1;
      lc[2].precision=lc[3].precision=lc[4].precision=1;
      region+=list.create(lc, Elms(lc)); list.flag|=LIST_TYPE_SORT|LIST_RESIZABLE_COLUMNS; list.cur_mode=LCM_ALWAYS;
      list.elmHeight(0.035).textSize(0, 1).columnHeight(0.045);

      rect(rect());
      return T;
   }

   void setCur()
   {
      list.cur=-1; FREPA(list)if(Elm *data=list.visToData(i))if(data.waypoint==WorldEdit.sel_waypoint){list.cur=i; break;}
   }
   void setVisible()
   {
      UID  filter_id;
      bool filter_is_id=filter_id.fromText(filter());
      Memt<bool> visible;
      FREPA(data)
      {
         Elm &elm=data[i];
         bool vis=false;
         if(elm.waypoint)
            if(show_removed() || (!elm.waypoint.removed && elm.waypoint.hasExisting()))
         {
            if(filter_is_id)
            {
               if(C UID *id=WorldEdit.ID(elm.waypoint))vis=(filter_id==*id);
            }else
            if(filter().is())vis=ContainsAll(elm.waypoint.name, filter());
            else             vis=true;
         }
         visible.add(vis);
      }
      list.setData(data, visible);
      setCur();
   }
   void set()
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

   virtual Rect sizeLimit()C override
   {
      Rect   r=super.sizeLimit(); r.min.set(0.4, 0.3);
      return r;
   }
           C Rect& rect()C {return super.rect();}
   virtual Window& rect(C Rect &rect)override
   {
      super .rect(rect);
      region.rect(Rect(0, -clientHeight(), clientWidth(), filter.rect().min.y).extend(-0.03));
      show_removed.rect(Rect_RU(region.rect().max.x, filter.rect().max.y, filter.rect().h(), filter.rect().h()));
      filter.size(Vec2(region.rect().w()-show_removed.rect().w(), filter.rect().h()));
      return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      visible(Mode()==MODE_WORLD && WorldEdit.mode()==WorldView.WAYPOINT && WorldEdit.waypoint_list());
      if(visible() && gpc.visible && _changed)set(); // first set data
      int cur=list.cur;
      super.update(gpc); // now update list
      if(visible() && gpc.visible)
      {
         REPA(MT)if(MT.guiObj(i)==&list && MT.bp(i))if(Elm *elm=list())
         {
            if(elm.waypoint && MT.bd(i) && list.selMode()==LSM_SET)WorldEdit.v4.moveTo(elm.waypoint.pos()); // camera center
         }
         if(cur!=list.cur)if(Elm *elm=list())if(elm.waypoint)WorldEdit.selWaypoint(elm.waypoint, elm.waypoint.firstExisting());
      }
   }
   void removed(EditWaypoint &waypoint)
   {
      REPA(data)if(data[i].waypoint==&waypoint){data[i].waypoint=null; setChanged(); break;}
   }
}
WaypointListClass WaypointList;
/******************************************************************************/
