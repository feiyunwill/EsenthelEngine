/******************************************************************************/
void WaterPreChanged(C Property &prop) {if(WorldEdit.sel_lake)WorldEdit.undos.set     (*WorldEdit.sel_lake); if(WorldEdit.sel_river)WorldEdit.undos.set      (*WorldEdit.sel_river);}
void WaterChanged   (C Property &prop) {                      WorldEdit.setChangedLake( WorldEdit.sel_lake);                        WorldEdit.setChangedRiver( WorldEdit.sel_river);}

void WaterMaterial(  WorldView &world, C Str &t) {}
Str  WaterMaterial(C WorldView &world          ) {return Proj.elmFullName(world.sel_lake ? world.sel_lake.material : world.sel_river ? world.sel_river.material : UIDZero);}
void WaterDepth   (  WorldView &world, C Str &t) {if(world.sel_lake)world.sel_lake.setDepth(TextFlt(t)); if(world.sel_river)world.sel_river.setDepth(TextFlt(t));}
Str  WaterDepth   (C WorldView &world          ) {return world.sel_lake ? world.sel_lake.depth : world.sel_river ? world.sel_river.depth : 1;}
void WaterTexScale(  WorldView &world, C Str &t) {if(world.sel_river)world.sel_river.setTexScale(Contains(t, ',') ? TextVec2(t) : Vec2(TextFlt(t)));}
Str  WaterTexScale(C WorldView &world          ) {return world.sel_lake ? world.sel_lake.tex_scale : world.sel_river ? world.sel_river.tex_scale : 1;}
void WaterSmooth  (  WorldView &world, C Str &t) {if(world.sel_river)world.sel_river.setSmooth(TextInt(t));}
Str  WaterSmooth  (C WorldView &world          ) {return world.sel_river ? world.sel_river.smooth : 0;}
/******************************************************************************/
void WorldView.setVisibleWaters()
{
   visible_lakes .clear();
   visible_rivers.clear();
   REPA(lakes)
   {
      Lake &lake=lakes.lockedData(i); Rect rect;
      if(!lake.removed && lake.getRect(rect))
         if(Cuts(worldToArea(rect), visible_area))visible_lakes.add(&lake.checkMesh(Proj));
   }
   REPA(rivers)
   {
      River &river=rivers.lockedData(i); Rect rect;
      if(!river.removed && river.getRect(rect))
         if(Cuts(worldToArea(rect), visible_area))visible_rivers.add(&river.checkMesh(Proj));
   }
   if(sel_lake )visible_lakes .include(sel_lake );
   if(sel_river)visible_rivers.include(sel_river);
   for(int i=WAO_LAKE_NUM; i<WAO_RIVER_NUM; i++)if(InRange(i, water_op))water_op.tab(i).visible(sel_river && !sel_river.removed); // set river tabs to be visible only when river is selected
   setWaterVis();
}
/******************************************************************************/
void WorldView.flushWater()
{
   REPA(changed_lakes)
   {
          Lake &lake   =*changed_lakes[i];
      if(C UID *lake_id= lakes.dataToKey(&lake))
      {
         Save(lake, lake_path+EncodeFileName(*lake_id)); // save edit ver
         if(ver)if(WaterVer *lake_ver=ver.lakes.find(*lake_id))Server.setLake(elm_id, *lake_id, lake_ver.ver, lake); // send to server
      }
   }
   REPA(changed_rivers)
   {
         River &river   =*changed_rivers[i];
      if(C UID *river_id= rivers.dataToKey(&river))
      {
         Save(river, river_path+EncodeFileName(*river_id)); // save edit ver
         if(ver)if(WaterVer *river_ver=ver.rivers.find(*river_id))Server.setRiver(elm_id, *river_id, river_ver.ver, river); // send to server
      }
   }
   changed_lakes .clear();
   changed_rivers.clear();
}
/******************************************************************************/
void WorldView.lakeDel(Lake *lake, int poly, int point)
{
   if(lake)
   {
      bool changed=false;
      if(InRange(poly, lake.polys))
      {
         undos.set(*lake);
         if(InRange(point, lake.polys[poly]))
         {
            lake.polys[poly].remove(point, true); changed=true; // remove point
            if(sel_lake==lake && sel_lake_poly==poly && sel_lake_point>=point)MAX(--sel_lake_point, 0);
            if(lit_lake==lake && lit_lake_poly==poly)if(lit_lake_point==point)lit_lake_point=-1;else if(lit_lake_point>point)lit_lake_point--;
         }
         if(!lake.polys[poly].elms()) // empty poly
         {
            lake.polys.remove(poly, true); changed=true; // remove poly
            if(sel_lake==lake && sel_lake_poly==poly)sel_lake_poly=-1;else if(sel_lake_poly>poly)MAX(--sel_lake_poly, 0);
            if(lit_lake==lake && lit_lake_poly==poly)lit_lake_poly=-1;else if(lit_lake_poly>poly)lit_lake_poly--;
         }
         if(!lake.polys.elms()) // empty lake
         {
            if(sel_lake==lake)sel_lake=null;
            if(lit_lake==lake)lit_lake=null;
         }
      }
      if(changed){lake.polys_time.getUTC(); setChangedLake(lake);}
   }
}
void WorldView.riverDel(River *river, int vtx)
{
   if(river)
   {
      bool changed=false;
      if(InRange(vtx, river.vtxs))
      {
         changed=true;
         undos.set(*river);

         // remove vtx
         river.vtxs.remove(vtx, true);

         // fix edges
         REPA(river.edges)
         {
            VecI2 &ind=river.edges[i];
            if(ind.x==vtx || ind.y==vtx)river.edges.remove(i);else
            {
               if(ind.x>vtx)ind.x--;
               if(ind.y>vtx)ind.y--;
            }
         }

         if(sel_river==river && sel_river_point>=vtx)MAX(--sel_river_point, 0);
         if(lit_river==river)if(lit_river_point==vtx)lit_river_point=-1;else if(lit_river_point>vtx)lit_river_point--;

         if(!river.vtxs.elms()) // empty river
         {
            if(sel_river==river)sel_river=null;
            if(lit_river==river)lit_river=null;
         }
      }
      if(changed){river.vtx_edge_time.getUTC(); setChangedRiver(river);}
   }
}
/******************************************************************************/
void WorldView.updateWater()
{
   lit_lake =null;
   lit_river=null;
   if(mode()==WATER)
   {
      if(Edit.Viewport4.View *view=v4.getView(Gui.ms()))
      {
         // get lit
         view.setViewportCamera();
         flt dist;
         REPA(visible_lakes) // lakes
         {
            Lake &lake=*visible_lakes[i]; REPAD(p, lake.polys)
            {
               Vec2 pos; Memc<Vec> &poly=lake.polys[p]; REPA(poly)if(PosToScreen(poly[i], pos))
               {
                  flt d=Dist(pos, Ms.pos());
                  if((!lit_lake && !lit_river) || d<dist){lit_lake=&lake; lit_lake_poly=p; lit_lake_point=i; lit_river=null; dist=d;}
               }
            }
         }
         REPA(visible_rivers) // rivers
         {
            Vec2 pos; River &river=*visible_rivers[i]; REPA(river.vtxs)if(PosToScreen(river.vtxs[i].pos, pos))
            {
               flt d=Dist(pos, Ms.pos());
               if( d<0.06) // require more precision for rivers
                  if((!lit_lake && !lit_river) || d<dist){lit_river=&river; lit_river_point=i; lit_lake=null; dist=d;}
            }
         }
         if(lit_lake || lit_river)
            if(dist>0.1){lit_lake=null; lit_river=null;}

         // select
         if(Ms.bp(0))
         {
            sel_lake =lit_lake ; sel_lake_poly=lit_lake_poly; sel_lake_point =lit_lake_point ;
            sel_river=lit_river;                              sel_river_point=lit_river_point;
            flushWater(); setVisibleWaters(); waterToGui();
         }
      }

      if(cur.view())
      {
         // edit
         if(cur.valid() && (water_op()==WAO_NEW_POINT && Ms.bp(1) || (!Kb.k.ctrlCmd() && Kb.kf(KB_INS))))
         {
            if(sel_lake && !sel_lake.removed)
            {
               if(InRange(sel_lake_poly, sel_lake.polys) && InRange(sel_lake_point, sel_lake.polys[sel_lake_poly]))
               {
                  undos.set(*sel_lake);
                  Memc<Vec> &poly=sel_lake.polys[sel_lake_poly];
                  poly.NewAt(++sel_lake_point)=cur.pos();
                  sel_lake.polys_time.getUTC(); setChangedLake(sel_lake);
               }else goto standalone;
            }else
            if(sel_river && !sel_river.removed)
            {
               if(InRange(sel_river_point, sel_river.vtxs))
               {
                  undos.set(*sel_river);
                  if(lit_river==sel_river && lit_river_point!=sel_river_point && InRange(lit_river_point, sel_river.vtxs)) // connect existing points
                  {
                     bool add=true;
                     REPA(sel_river.edges)
                     {
                        VecI2 &edge=sel_river.edges[i];
                        if(edge.x==sel_river_point && edge.y==lit_river_point){add=false;                 break;} // that connection already exists
                        if(edge.y==sel_river_point && edge.x==lit_river_point){add=false; edge.reverse(); break;} // that connection already exists but reveresed -> reverse it
                     }
                     if(add) // add new connection
                     {
                        sel_river.edges.New().set(sel_river_point, lit_river_point);
                     }
                     sel_river_point=lit_river_point;
                  }else // create new point connected to previous
                  {
                     River.Vtx old=sel_river.vtxs[sel_river_point]; // copy because 'New' will change memory address
                     bool      found_hm; flt old_hm_y=hmHeight(old.pos, &found_hm);
                     int       new_point=sel_river.vtxs.elms();
                     sel_river.vtxs .New().set(old.radius, cur.pos()+Vec(0, found_hm ? old.pos.y-old_hm_y : sel_river.depth/2, 0));
                     sel_river.edges.New().set(sel_river_point, new_point); sel_river_point=new_point;
                  }
                  sel_river.vtx_edge_time.getUTC(); setChangedRiver(sel_river);
               }else goto standalone;
            }else goto standalone;
         }else
         if(cur.valid() && (water_op()==WAO_NEW_WATER && Ms.bp(1) || (Kb.k.ctrlCmd() && Kb.kf(KB_INS))))
         {
         standalone:
            if((!sel_lake || sel_lake.removed) && (!sel_river || sel_river.removed)) // nothing selected
            {
               if(water_mtrl)
               {
                  // create new water
                  if(water_mode()==0){sel_river=null; sel_lake = lakes(UID().randomizeValid()); sel_lake .material=water_mtrl.id(); sel_lake .material_time.getUTC();} // lake
                  else               {sel_lake =null; sel_river=rivers(UID().randomizeValid()); sel_river.material=water_mtrl.id(); sel_river.material_time.getUTC();} // river
                  setVisibleWaters();
               }else 
               {
                  highlightWaterMtrl(); Gui.msgBox(S, "Please drag and drop a material into the water material slot in order to create new water elements");
               }
            }
            if(sel_lake && !sel_lake.removed)
            {
               if(sel_lake)undos.set(*sel_lake); // we can set undo after creating because water have no points by default (which makes them empty/removed)
               sel_lake_point=0;
               sel_lake_poly =sel_lake.polys.elms(); sel_lake.polys.New().add(cur.pos()); sel_lake.polys_time.getUTC(); setChangedLake(sel_lake); // create new poly with just 1 point
            }else 
            if(sel_river && !sel_river.removed)
            {
               if(sel_river)undos.set(*sel_river); // we can set undo after creating because water have no points by default (which makes them empty/removed)
               sel_river_point=sel_river.vtxs.elms(); sel_river.vtxs.New().pos=cur.pos()+Vec(0, sel_river.depth/2, 0); sel_river.vtx_edge_time.getUTC(); setChangedRiver(sel_river); // create just 1 vtx
            }
         }else
         if(water_op()==WAO_DEL_POINT && Ms.bp(1) || Kb.kf(KB_DEL))
         {
            // process selections first, then highlights
            if(sel_lake  && !sel_lake .removed && !Ms.bp(1)) lakeDel(sel_lake , sel_lake_poly, sel_lake_point);else // don't use RMB to delete selected
            if(sel_river && !sel_river.removed && !Ms.bp(1))riverDel(sel_river,               sel_river_point);else // don't use RMB to delete selected
            if(lit_lake  && !lit_lake .removed             ) lakeDel(lit_lake , lit_lake_poly, lit_lake_point);else
            if(lit_river && !lit_river.removed             )riverDel(lit_river,               lit_river_point);
         }

         switch(water_op())
         {
            case -1                  :
            case WAO_MOVE            :
            case WAO_MOVE_ALL        :
            case WAO_ROT             :
            case WAO_MOVE_ALL_Y      :
            case WAO_MOVE_Y          :
            case WAO_RIVER_POINT_SIZE:
            case WAO_RIVER_SIZE      :
            {
               Lake  *lake =((sel_lake  && !sel_lake .removed && InRange(sel_lake_poly, sel_lake.polys) && InRange(sel_lake_point , sel_lake.polys[sel_lake_poly])) ? sel_lake  : null);
               River *river=((sel_river && !sel_river.removed &&                                           InRange(sel_river_point, sel_river.vtxs               )) ? sel_river : null);
               Vec   *lake_pos=(lake  ? &lake .polys[sel_lake_poly][sel_lake_point ]     : null),
                    *river_pos=(river ? &river.vtxs                [sel_river_point].pos : null);
               if(Ms.b(1) && (lake || river))
               {
                  if(lake )undos.set(*lake );
                  if(river)undos.set(*river);

                  bool first_push=true; REP(curTotal())if(curAll(i).notFirstEdit()){first_push=false; break;}
                  if(!v4.perspective() || !v4.fpp())water_edit_speed=CamMoveScale(v4.perspective());else if(first_push)water_edit_speed=Dist(lake_pos ? *lake_pos : *river_pos, ActiveCam.matrix.pos)*0.75; // in FPP mode automatically detect the speed according to waypoint distance to camera, but only at the moment of first button push

                  Vec vec(0);
                  Vec x=ActiveCam.matrix.x; x.y=0;     x.normalize();
                  Vec z=ActiveCam.matrix.z; z.y=0; if(!z.normalize())z=ActiveCam.matrix.y;else if(Dot(z, ActiveCam.matrix.y)<0)z.chs();

                  vec+=x*(Ms.d().x*water_edit_speed)
                      +z*(Ms.d().y*water_edit_speed);

                  int op=water_op(); if(lake && (op==WAO_MOVE_Y || op==WAO_RIVER_POINT_SIZE || op==WAO_RIVER_SIZE))op=-1; // force default for lakes when river op is selected
                  if(Kb.ctrlCmd() && Kb.shift() && Kb.alt())op=WAO_RIVER_SIZE      ;else
                  if(Kb.ctrlCmd() &&               Kb.alt())op=WAO_RIVER_POINT_SIZE;else
                  if(Kb.ctrlCmd() && Kb.shift()            )op=WAO_MOVE_ALL_Y      ;else
                  if(Kb.ctrlCmd()                          )op=WAO_MOVE_Y          ;else
                  if(                Kb.shift()            )op=WAO_MOVE_ALL        ;else
                  if(                              Kb.alt())op=WAO_ROT             ;
                  switch(op)
                  {
                     default:
                     {
                        if( lake_pos)(* lake_pos)+=vec;
                        if(river_pos)(*river_pos)+=vec;
                     }break;
                     
                     case WAO_MOVE_ALL:
                     {
                        if(lake )REPAD(p, lake.polys)REPAO(lake .polys[p])+=vec;
                        if(river)                    REPAO(river.vtxs).pos+=vec;
                     }break;

                     case WAO_MOVE_Y:
                     {
                        flt y=Ms.d().y*water_edit_speed;
                        if(lake     )REPAD(p, lake.polys)REPAO(lake.polys[p]).y+=y;
                        if(river_pos)river_pos.y+=y;
                     }break;

                     case WAO_MOVE_ALL_Y:
                     {
                        flt y=Ms.d().y*water_edit_speed;
                        if(lake )REPAD(p, lake.polys)REPAO(lake .polys[p]).y+=y;
                        if(river)                    REPAO(river.vtxs).pos.y+=y;
                     }break;

                     case WAO_ROT:
                     {
                        Vec center=0; int n=0;
                        if(lake )REPAD(p, lake.polys)REPA(lake.polys[p]){center+=lake .polys[p][i]; n++;}
                        if(river)                    REPA(river.vtxs   ){center+=river.vtxs[i].pos; n++;}
                        if(n)
                        {
                           center/=n;
                           if(lake)REPAD(p, lake.polys)REPA(lake.polys[p])
                           {
                              Vec &pos=lake.polys[p][i]; Vec2 p2=pos.xz();
                              p2-=center.xz();
                              p2.rotate(Ms.d().sum());
                              p2+=center.xz();
                              pos.set(p2.x, pos.y, p2.y);
                           }
                           if(river)REPA(river.vtxs)
                           {
                              Vec &pos=river.vtxs[i].pos; Vec2 p2=pos.xz();
                              p2-=center.xz();
                              p2.rotate(Ms.d().sum());
                              p2+=center.xz();
                              pos.set(p2.x, pos.y, p2.y);
                           }
                        }
                     }break;

                     case WAO_RIVER_POINT_SIZE:
                     {
                        if(river)river.vtxs[sel_river_point].radius*=ScaleFactor(Ms.d().sum());
                     }break;

                     case WAO_RIVER_SIZE:
                     {
                        if(river)REPAO(river.vtxs).radius*=ScaleFactor(Ms.d().sum());
                     }break;
                  }

                  if(lake )lake .   polys_time.getUTC(); setChangedLake (lake );
                  if(river)river.vtx_edge_time.getUTC(); setChangedRiver(river);
               }
            }break;
         }
      }
   }

   // flush waters after some delay
   if(!changed_lakes.elms() && !changed_rivers.elms())last_water_flush_time=Time.realTime();else // if there are no elements then set last time to current time so after adding an element it won't be sent right away
   if(Time.realTime()-last_water_flush_time>=FlushWaterDelay) // if enough time has passed
   {
      last_water_flush_time=Time.realTime();
      flushWater();
   }
}
/******************************************************************************/
