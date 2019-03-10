/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
 

/******************************************************************************/
   void EditWaypointPoint::setPos(C Vec &pos  ) {T.pos=pos; pos_time.getUTC();}
   void EditWaypointPoint::setRemoved(bool removed) {T.removed=removed; removed_time.getUTC();}
   bool EditWaypointPoint::newer(C EditWaypointPoint &src)C
   {
      return removed_time>src.removed_time || pos_time>src.pos_time || order_time>src.order_time;
   }
   bool EditWaypointPoint::equal(C EditWaypointPoint &src)C
   {
      return removed_time==src.removed_time && pos_time==src.pos_time && order_time==src.order_time;
   }
   bool EditWaypointPoint::sync(C EditWaypointPoint &src)
   {
      bool changed=false;
      // 'order_time' is synced in 'EditWaypoint.sync'
      changed|=Sync(removed_time, src.removed_time, removed, src.removed);
      changed|=Sync(    pos_time, src.    pos_time,     pos, src.    pos);
      return changed;
   }
   bool EditWaypointPoint::undo(C EditWaypointPoint &src)
   {
      bool changed=false;
      changed|=Undo(removed_time, src.removed_time, removed, src.removed);
      changed|=Undo(    pos_time, src.    pos_time,     pos, src.    pos);
      changed|=Undo(  order_time, src.  order_time);
      return changed;
   }
   bool EditWaypointPoint::save(File &f)C
   {
      f.cmpUIntV(0);
      f<<id<<removed<<pos<<removed_time<<pos_time<<order_time;
      return f.ok();
   }
   bool EditWaypointPoint::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>id>>removed>>pos>>removed_time>>pos_time>>order_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   int                EditWaypoint::FindI(C Memc<EditWaypointPoint> &waypoints, C UID &waypoint_id) {REPA(waypoints)if(waypoints[i].id==waypoint_id)return i; return -1;}
   EditWaypointPoint* EditWaypoint::Find(  Memc<EditWaypointPoint> &waypoints, C UID &waypoint_id) {int i=FindI(waypoints, waypoint_id); return (i>=0) ? &waypoints[i] : null;}
   void EditWaypoint::del() {clear();}
   int                EditWaypoint::findI(C UID &waypoint_id)C {return FindI(T, waypoint_id);}
   EditWaypointPoint* EditWaypoint::find(C UID &waypoint_id)  {return Find (T, waypoint_id);}
 C EditWaypointPoint* EditWaypoint::find(C UID &waypoint_id)C {return ConstCast(T).find(waypoint_id);}
   int EditWaypoint::liveIndex(C UID &waypoint_id) {int n=0; FREPA(T)if(!T[i].removed){if(T[i].id==waypoint_id)return n; n++;} return -1;}
   bool EditWaypoint::newer(C EditWaypoint &src)C
   {
      if(loop_mode_time>src.loop_mode_time)return true;
      if(  removed_time>src.  removed_time)return true;
      if(     name_time>src.     name_time)return true;
      REPA(T){C EditWaypointPoint &e=T[i], *s=src.find(e.id); if(!s || e.newer(*s))return true;}
      return false;
   }
   bool EditWaypoint::equal(C EditWaypoint &src)C
   {
      if(loop_mode_time!=src.loop_mode_time)return false;
      if(  removed_time!=src.  removed_time)return false;
      if(     name_time!=src.     name_time)return false;
      if(elms()!=src.elms())return false;
      REPA(T){C EditWaypointPoint &e=T[i], *s=src.find(e.id); if(!s || !e.equal(*s))return false;}
      return true;
   }
   bool EditWaypoint::getRect(Rect &rect)C
   {
      bool   found=false; REPA(T)if(!T[i].removed)Include(rect, found, T[i].pos.xz());
      return found;
   }
   bool  EditWaypoint::hasExisting()C { REPA(T)if(!T[i].removed)return    true; return   false;}
   UID EditWaypoint::firstExisting()C {FREPA(T)if(!T[i].removed)return T[i].id; return UIDZero;}
   UID EditWaypoint::existingNear(C UID &point_id)C
   {
      REPA(T)if(T[i].id==point_id)
      {
                            REPD(j, i)if(!T[j].removed)return T[j].id; // try before
         for(int j=i+1; j<elms(); j++)if(!T[j].removed)return T[j].id; // try after
         break;
      }
      return UIDZero;
   }
   Vec EditWaypoint::pos()C
   {
      FREPA(T)if(!T[i].removed)return T[i].pos;
      if(elms())return T[0].pos;
      return Vec(0);
   }
   int EditWaypoint::existingPoints()C
   {
      int    n=0; REPA(T)if(!T[i].removed)n++;
      return n;
   }
   EditWaypoint& EditWaypoint::setRemoved(bool                    removed  ) {T.removed  =removed  ;   removed_time.getUTC(); return T;}
   EditWaypoint& EditWaypoint::setLoopMode(Game::Waypoint::LOOP_MODE loop_mode) {T.loop_mode=loop_mode; loop_mode_time.getUTC(); return T;}
   EditWaypoint& EditWaypoint::setName(C Str                  &name     ) {T.name     =name     ;      name_time.getUTC(); return T;}
   bool EditWaypoint::sync(C EditWaypoint &src)
   {
      bool changed=false;
      changed|=Sync(loop_mode_time, src.loop_mode_time, loop_mode, src.loop_mode);
      changed|=Sync(  removed_time, src.  removed_time,   removed, src.  removed);
      changed|=Sync(     name_time, src.     name_time,      name, src.     name);
      Memc<EditWaypointPoint> this_waypoints, src_waypoints=src;
      Swap(SCAST(Memc<EditWaypointPoint>, T), this_waypoints);
      for(; this_waypoints.elms() || src_waypoints.elms(); )
      {
         EditWaypointPoint *t=this_waypoints.addr(0), // this elm
                           *s= src_waypoints.addr(0), // src  elm
                           *t_in_s=(t ? Find( src_waypoints, t->id) : null), // this in src
                           *s_in_t=(s ? Find(this_waypoints, s->id) : null); // src  in this
         if(t && t_in_s && t_in_s->order_time>t->order_time) // this elm is present in src and there it has newer order
         {
            changed=true; t_in_s->sync(*t); this_waypoints.removeData(t, true);
         }else
         if(s && s_in_t && s_in_t->order_time>s->order_time) // src elm is present in this and there it has newer order
         {
            changed=true; s_in_t->sync(*s); src_waypoints.removeData(s, true);
         }else
         if(t && (!s || t->order_time>=s->order_time)) // there is this elm which is same or newer than src elm
         {
            if(t_in_s){changed|=t->sync(*t_in_s); src_waypoints.removeData(t_in_s, true);}
            Swap(*t, New()); this_waypoints.removeData(t, true);
         }else
         if(s)
         {
            changed=true; // always changed because we're taking src element instead of this elm
            if(s_in_t){changed|=s->sync(*s_in_t); this_waypoints.removeData(s_in_t, true);}
            Swap(*s, New()); src_waypoints.removeData(s, true);
         }else Exit("EditWaypoint::sync");
      }
      return changed;
   }
   void EditWaypoint::undo(C EditWaypoint &src)
   {
      bool changed=false;
      changed|=Undo(loop_mode_time, src.loop_mode_time, loop_mode, src.loop_mode);
      changed|=Undo(  removed_time, src.  removed_time,   removed, src.  removed);
      changed|=Undo(     name_time, src.     name_time,      name, src.     name);
      Memc<EditWaypointPoint> this_waypoints, src_waypoints=src;
      Swap(SCAST(Memc<EditWaypointPoint>, T), this_waypoints);
      for(; this_waypoints.elms() || src_waypoints.elms(); )
      {
         EditWaypointPoint *t=this_waypoints.addr(0), // this elm
                           *s= src_waypoints.addr(0), // src  elm
                           *t_in_s=(t ? Find( src_waypoints, t->id) : null), // this in src
                           *s_in_t=(s ? Find(this_waypoints, s->id) : null); // src  in this
         if(t && !t_in_s) // this elm is present and it is not found in src (which means that it was created later, so add it first as removed)
         {
            if(!t->removed){t->removed=true; t->removed_time++;} // mark as removed if necessary
            Swap(*t, New()); this_waypoints.removeData(t, true);
         }else
         if(s)
         {
            if(s_in_t) // 's_in_t' is newer, so undo and insert this one
            {
               s_in_t->undo(*s); Swap(*s_in_t, New());
            }else
            {
               Swap(*s, New());
            }
            this_waypoints.removeData(s_in_t, true);
             src_waypoints.removeData(s     , true);
         }else Exit("EditWaypoint::undo");
      }
   }
   int EditWaypoint::move(C UID &point_id, int index)
   {
      int elm=FindI(T, point_id);
      if(InRange(elm, T))
      {
         T[elm].order_time.getUTC();
         moveElm(elm, index);
         if(elm>=index)index++;
      }
      return index;
   }
   EditWaypoint& EditWaypoint::reverse()
   {
      reverseOrder(); REPAO(T).order_time.getUTC(); return T;
   }
   EditWaypoint& EditWaypoint::rotateLeft()
   {
      if(elms()){T[0].order_time.getUTC(); rotateOrder(-1);} return T; // old 1st is now new last
   }
   EditWaypoint& EditWaypoint::rotateRight()
   {
      if(elms()){rotateOrder(1); T[0].order_time.getUTC();} return T; // old last is now new 1st
   }
   void EditWaypoint::create(C Game::Waypoint &src, C Str &name)
   {
      setName(name).setRemoved(false).setLoopMode(src.loop_mode);
      FREPA(src)New().setPos(src.points[i].pos);
   }
   bool EditWaypoint::copyTo(Game::Waypoint &w)
   {
      w.del();
      if(!removed)
      {
         w.loop_mode=loop_mode;
         FREPA(T)if(!T[i].removed)w.points.New().pos=T[i].pos;
         w.updateTotalLengths();
      }
      return w.points.elms()>0;
   }
   void EditWaypoint::draw(C Color &color, int edge_smooth_steps) {Game::Waypoint w; copyTo(w); w.draw(color, color, WaypointRadius, edge_smooth_steps);}
   void EditWaypoint::drawText(C UID &sel, C UID &lit)
   {
      int n=0; TextStyleParams ts; ts.size=0.055f; Vec2 screen; FREPA(T)if(!T[i].removed){if(PosToScreen(T[i].pos, screen)){ts.color=((T[i].id==sel) ? LitSelColor : (T[i].id==lit) ? LitColor : DefColor); D.text(ts, screen, S+n);} n++;}
   }
   bool EditWaypoint::save(File &f)C
   {
      f.cmpUIntV(2);
      f<<removed<<loop_mode<<name<<removed_time<<loop_mode_time<<name_time;
      ::EE::Memc< ::EditWaypointPoint>::save(f);
      return f.ok();
   }
   bool EditWaypoint::load(File &f)
   {
      del();
      switch(f.decUIntV())
      {
         case 2:
         {
            f>>removed>>loop_mode>>name>>removed_time>>loop_mode_time>>name_time;
            if(::EE::Memc< ::EditWaypointPoint>::load(f))
               if(f.ok())return true;
         }break;

         case 1:
         {
            f>>removed>>loop_mode; GetStr2(f, name); f>>removed_time>>loop_mode_time>>name_time;
            if(::EE::Memc< ::EditWaypointPoint>::load(f))
               if(f.ok())return true;
         }break;

         case 0:
         {
            f>>removed>>loop_mode>>removed_time>>loop_mode_time>>name_time; GetStr(f, name);
            if(::EE::Memc< ::EditWaypointPoint>::load(f))
               if(f.ok())return true;
         }break;
      }
      return false;
   }
   bool EditWaypoint::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      del(); return false;
   }
EditWaypointPoint::EditWaypointPoint() : removed(false), id(UID().randomizeValid()), pos(0) {}

EditWaypoint::EditWaypoint() : removed(false), loop_mode(Game::Waypoint::SINGLE) {}

/******************************************************************************/
