/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
void WaypointRol() {if(WorldEdit.sel_waypoint){WorldEdit.undos.set(*WorldEdit.sel_waypoint); WorldEdit.setChangedWaypoint(&WorldEdit.sel_waypoint->rotateLeft ());}}
void WaypointRor() {if(WorldEdit.sel_waypoint){WorldEdit.undos.set(*WorldEdit.sel_waypoint); WorldEdit.setChangedWaypoint(&WorldEdit.sel_waypoint->rotateRight());}}
void WaypointRev() {if(WorldEdit.sel_waypoint){WorldEdit.undos.set(*WorldEdit.sel_waypoint); WorldEdit.setChangedWaypoint(&WorldEdit.sel_waypoint->reverse    ());}}

void WaypointPreChanged(C Property &prop) {if(WorldEdit.sel_waypoint)WorldEdit.undos.set         (*WorldEdit.sel_waypoint);}
void WaypointChanged   (C Property &prop) {                          WorldEdit.setChangedWaypoint( WorldEdit.sel_waypoint);}

void WaypointIDCopy(  WorldView &world             ) {ClipSet(WaypointID(world));}
void WaypointID    (  WorldView &world, C Str &text) {}
Str  WaypointID    (C WorldView &world             ) {if(C UID *waypoint_id=WorldEdit.ID(world.sel_waypoint))return waypoint_id->asCString(); return S;}
void WaypointName  (  WorldView &world, C Str &text) {if(world.sel_waypoint)world.sel_waypoint->setName(text);}
Str  WaypointName  (C WorldView &world             ) {return world.sel_waypoint ? world.sel_waypoint->name : S;}
void WaypointLoop  (  WorldView &world, C Str &text) {if(world.sel_waypoint)world.sel_waypoint->setLoopMode(Game::Waypoint::LOOP_MODE(TextInt(text)));}
Str  WaypointLoop  (C WorldView &world             ) {return world.sel_waypoint ? world.sel_waypoint->loop_mode : -1;}
/******************************************************************************/
void WorldView::setVisibleWaypoints()
{
   visible_waypoints.clear();
   REPA(waypoints)
   {
      EditWaypoint &waypoint=waypoints.lockedData(i); Rect rect;
      if(!waypoint.removed && waypoint.getRect(rect)) // add only existing waypoints, and check for getRect in case it has no existing points and returns Rect(0) which would be Area(0, 0) for all empty waypoints
         if(Cuts(worldToArea(rect), visible_area))visible_waypoints.add(&waypoint);
   }
   if(sel_waypoint)
   {
      visible_waypoints.include(sel_waypoint);
      REPAO(waypoint_props).toGui();
   }
   bool visible=(sel_waypoint!=null); // allow displaying removed waypoints too (because 'WaypointList' can select them)
   waypoint_props_region.visible(visible);
   waypoint_pos         .visible(visible);
}
/******************************************************************************/
void WorldView::flushWaypoints()
{
   REPA(changed_waypoints)
   {
      EditWaypoint &wp   =*changed_waypoints[i];
      if(C UID     *wp_id=ID(&wp))
      {
              Save(wp, edit_waypoint_path+EncodeFileName(*wp_id)); // save edit ver
         Str game_path=game_waypoint_path+EncodeFileName(*wp_id);
         Game::Waypoint g; if(wp.copyTo(g))Save(g, game_path);else FDelFile(game_path); // save game ver
         if(ver)if(Version *wp_ver=ver->waypoints.find(*wp_id))Server.setWaypoint(elm_id, *wp_id, *wp_ver, wp); // send to server
      }
   }
   changed_waypoints.clear();
}
/******************************************************************************/
void WorldView::updateWaypoint()
{
   lit_waypoint=null;
   if(mode()==WAYPOINT)
   {
      if(Edit::Viewport4::View *view=v4.getView(Gui.ms()))
      {
         // get lit
         view->setViewportCamera();
         flt dist;
         REPA(visible_waypoints)
         {
            Vec2 pos; EditWaypoint &wp=*visible_waypoints[i]; REPA(wp)if(!wp[i].removed && PosToScreen(wp[i].pos, pos))
            {
               flt d=Dist(pos, Ms.pos());
               if(!lit_waypoint || d<dist){lit_waypoint=&wp; lit_waypoint_point=wp[i].id; dist=d;}
            }
         }
         if(lit_waypoint && dist>0.1f)lit_waypoint=null;

         // select
         if(Ms.bp(0))selWaypoint(lit_waypoint, lit_waypoint_point);
      }
      if(cur.view())
      {
         // edit
         if(cur.valid() && (waypoint_op()==WPO_INS && Ms.bp(1) || Kb.kf(KB_INS)))
         {
            if(!sel_waypoint || sel_waypoint->removed) // new waypoint needs to be created
            {
               EditWaypoint &wp=waypoints(UID().randomizeValid())->setName(S+"Waypoint "+waypoints.elms());
               WaypointList.setChanged(); selWaypoint(&wp, UIDZero);
            }
            // undo
            WorldEdit.undos.set(*sel_waypoint); // we can remember the waypoint even after it was created because new waypoints have no points which makes them non existing

            // insert point
            EditWaypointPoint &point=sel_waypoint->NewAt(sel_waypoint->findI(sel_waypoint_point)+1);
            point.order_time.getUTC();
            point.setPos(cur.pos());

            selWaypoint(sel_waypoint, point.id);
            setChangedWaypoint(sel_waypoint);
         }else
         if(waypoint_op()==WPO_DEL && Ms.bp(1) || Kb.kf(KB_DEL))
         {
            if(sel_waypoint && !sel_waypoint->removed && !Ms.bp(1)) // don't remove selection with RMB (delete highlighted with RMB)
            {
               if(EditWaypointPoint *p=sel_waypoint->find(sel_waypoint_point))
               {
                  WorldEdit.undos.set(*sel_waypoint);
                  EditWaypoint *waypoint=sel_waypoint;
                  p->setRemoved(true);
                  UID near=sel_waypoint->existingNear(sel_waypoint_point); selWaypoint(near.valid() ? sel_waypoint : null, near);
                  setChangedWaypoint(waypoint);
                  WaypointList.setChanged();
               }
            }else
            if(lit_waypoint && !lit_waypoint->removed)
            {
               if(EditWaypointPoint *p=lit_waypoint->find(lit_waypoint_point))
               {
                  WorldEdit.undos.set(*lit_waypoint);
                  p->setRemoved(true);
                  if(sel_waypoint==lit_waypoint && sel_waypoint_point==lit_waypoint_point)
                  {
                     UID near=sel_waypoint->existingNear(sel_waypoint_point); selWaypoint(near.valid() ? sel_waypoint : null, near);
                  }
                  setChangedWaypoint(lit_waypoint);
                  lit_waypoint_point.zero();
                  WaypointList.setChanged();
               }
            }
         }

         switch(waypoint_op())
         {
            case -1            :
            case WPO_MOVE      :
            case WPO_MOVE_ALL  :
            case WPO_MOVE_Y    :
            case WPO_MOVE_ALL_Y:
            case WPO_ROT       :
            {
               if(Ms.b(1) && sel_waypoint && !sel_waypoint->removed)
                  if(EditWaypointPoint *p=sel_waypoint->find(sel_waypoint_point))
                     if(!p->removed)
               {
                  WorldEdit.undos.set(*sel_waypoint);

                  Vec  pos=p->pos;
                  bool first_push=true; REP(curTotal())if(curAll(i).notFirstEdit()){first_push=false; break;}
                  if(!v4.perspective() || !v4.fpp())waypoint_edit_speed=CamMoveScale(v4.perspective());else if(first_push)waypoint_edit_speed=Dist(pos, ActiveCam.matrix.pos)*0.75f; // in FPP mode automatically detect the speed according to waypoint distance to camera, but only at the moment of first button push

                  Vec vec(0);
                  Vec x=ActiveCam.matrix.x; x.y=0;     x.normalize();
                  Vec z=ActiveCam.matrix.z; z.y=0; if(!z.normalize())z=ActiveCam.matrix.y;else if(Dot(z, ActiveCam.matrix.y)<0)z.chs();

                  vec+=x*(Ms.d().x*waypoint_edit_speed)
                      +z*(Ms.d().y*waypoint_edit_speed);

                  int op=waypoint_op();
                  if(Kb.ctrlCmd() && Kb.shift())op=WPO_MOVE_ALL_Y;else
                  if(Kb.ctrlCmd()              )op=WPO_MOVE_Y    ;else
                  if(                Kb.shift())op=WPO_MOVE_ALL  ;else
                  if(Kb.alt()                  )op=WPO_ROT       ;
                  switch(op)
                  {
                     default:
                     {
                        pos+=vec;
                        if(hm_align){bool found; flt y=hmHeight(pos, &found); if(found)pos.y=y;}
                        p->setPos(pos);
                     }break;
                     
                     case WPO_MOVE_ALL:
                     {
                        REPA(*sel_waypoint)
                        {
                           EditWaypointPoint &p=(*sel_waypoint)[i]; pos=p.pos; pos+=vec;
                           if(hm_align){bool found; flt y=hmHeight(pos, &found); if(found)pos.y=y;}
                           p.setPos(pos);
                        }
                     }break;

                     case WPO_MOVE_Y:
                     {
                        pos.y+=Ms.d().y*waypoint_edit_speed;
                        p->setPos(pos);
                     }break;

                     case WPO_MOVE_ALL_Y:
                     {
                        REPA(*sel_waypoint){EditWaypointPoint &p=(*sel_waypoint)[i]; pos=p.pos; pos.y+=Ms.d().y*waypoint_edit_speed; p.setPos(pos);}
                     }break;

                     case WPO_ROT:
                     {
                        Vec center=0; int n=0; REPA(*sel_waypoint){EditWaypointPoint &p=(*sel_waypoint)[i]; if(!p.removed){center+=p.pos; n++;}}
                        if(n)
                        {
                           center/=n;
                           REPA(*sel_waypoint)
                           {
                              EditWaypointPoint &p=(*sel_waypoint)[i];
                              Vec2 p2=p.pos.xz();
                              p2-=center.xz();
                              p2.rotate(Ms.d().sum());
                              p2+=center.xz();
                              pos.set(p2.x, p.pos.y, p2.y);
                              if(hm_align){bool found; flt y=hmHeight(pos, &found); if(found)pos.y=y;}
                              p.setPos(pos);
                           }
                        }
                     }break;
                  }

                  setChangedWaypoint(sel_waypoint);
               }
            }break;
         }
      }
   }

   // flush waypoints after some delay
   if(!changed_waypoints.elms())last_waypoint_flush_time=Time.realTime();else // if there are no elements then set last time to current time so after adding an element it won't be sent right away
   if(Time.realTime()-last_waypoint_flush_time>=FlushWaypointsDelay) // if enough time has passed
   {
      last_waypoint_flush_time=Time.realTime();
      flushWaypoints();
   }
}
/******************************************************************************/

/******************************************************************************/
