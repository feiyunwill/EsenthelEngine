/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
HeightBrushClass HeightBrush;
/******************************************************************************/

/******************************************************************************/
      void HeightBrushClass::AlignWaypoint::set(EditWaypoint &wp, EditWaypointPoint &p) {T.wp=&wp; T.p=&p;}
   void HeightBrushClass::ModeChanged(HeightBrushClass &hb) {SetHmOp((hb.mode()>=0) ? OP_HM_HEIGHT : OP_HM_NONE);}
   HeightBrushClass& HeightBrushClass::create()
   {
      cchar *text[HB_NUM]=
      {
      #if MOBILE
         MLTC(u"Up"       , PL, u"Wyżej"        , DE, u"Höher"         , RU, u"Вверх"      , PO, u"Aumentar"         , CN, u"上"   ),
         MLTC(u"Down"     , PL, u"Niżej"        , DE, u"Tiefer"        , RU, u"Вниз"       , PO, u"Diminuir"         , CN, u"下"   ),
      #else
         MLTC(u"Down / Up", PL, u"Niżej / Wyżej", DE, u"Tiefer / Höher", RU, u"Вниз/Вверх" , PO, u"Diminuir/Aumentar", CN, u"下/上" ),
      #endif
         MLTC(u"Level"    , PL, u"Wyrównaj"     , DE, u"Ebnen"         , RU, u"Уровень"    , PO, u"Nivelar"          , CN, u"级别"  ),
         MLTC(u"Flatten"  , PL, u"Spłaszcz"     , DE, u"Flach Ebnen"   , RU, u"Выравнивать", PO, u"Nivelar Plano"    , CN, u"平的级别"),
         MLTC(u"Average"  , PL, u"Uśrednij"     , DE, u"Average"       , RU, u"Усреднение" , PO, u"Média"            , CN, u"平均值" ),
         MLTC(u"Soften"   , PL, u"Zmiękcz"      , DE, u"Soften"        , RU, u"Мягкость"   , PO, u"Suavizar"         , CN, u"变柔软" ),
         MLTC(u"Noise"    , PL, u"Szum"         , DE, u"Noise"         , RU, u"шум"        , PO, u"Desnivelar"       , CN, u"干扰"  ),
      };

      ::EE::Window::create("Height"/*MLTC(u"Height Brush", PL,u"Wysokość", DE,u"Höhen Pinsel", RU,u"Высота кисти", PO,u"Pincel do Heightmap")*/);
      clientRect(Rect_RU(WorldEdit.rect().w(), -defaultBarHeight(), 0.22f, 0.32f));
      T+=mode.create(Rect_LU(0, clientSize()).extend(-0.01f), 0, text, Elms(text)).func(ModeChanged, T);
   #if MOBILE
      mode.tab(HB_ADD    ).desc("Move Terrain Up with LMB");
      mode.tab(HB_SUB    ).desc("Move Terrain Down with LMB");
   #else
      mode.tab(HB_ADD_SUB).desc("Move Terrain Down with LMB and Up with RMB");
   #endif
      mode.tab(HB_LEVEL  ).desc("Set Terrain Level with RMB");
      mode.tab(HB_FLAT   ).desc("Flatten Terrain with RMB");
      mode.tab(HB_AVG    ).desc("Average Terrain with RMB");
      mode.tab(HB_SOFT   ).desc("Soften Terrain with RMB");
      mode.tab(HB_NOISE  ).desc("Apply Rough Noise with LMB and Soft Noise with RMB");

      // create noise lookup textures
      REPA(noise)
      {
         noise[i].createSoft(HeightBrushNoiseRes, HeightBrushNoiseRes, 1, IMAGE_I8);
         REPD(y, noise[i].h())
         REPD(x, noise[i].w())noise[i].pixel(x, y, Random(256));
         switch(i)
         {
            case 0: noise[i].blur(2, false).normalize(); break;
            case 1: noise[i].blur(5, false).normalize(); break;
         }
      }
      return T;
   }
   void HeightBrushClass::GetHeight(Cell<Area> &cell, Cursor &cur, int thread_index) {HeightBrush.getHeight(cell(), cur);}
          void HeightBrushClass::getHeight(     Area  &area, Cursor &cur)
   {
      if(area.hm && Brush.affects(cur.edge(), area.xy))
      {
         ThreadMayUseGPUData();

         Heightmap2 &hm      =*area.hm;
         int         res     =WorldEdit.hmRes();
         Vec2        area_pos=area.pos2D();
         flt         dist, height;

         switch(mode())
         {
            case HB_LEVEL:
            case HB_FLAT : dist=-1; height=0; break;

            case HB_AVG: dist= 0; height=0; break;
         }

         REPD(y, res)
         REPD(x, res)
         {
            Vec2 pos(x,y); pos*=WorldEdit.areaSize()/(res-1); pos+=area_pos; // (pos/res+cell)*UNIT == pos*UNIT/res + cell*UNIT
            flt  pos_y=hm.height(x, y);

            switch(mode())
            {
               case HB_LEVEL:
               case HB_FLAT :
               {
                  flt d=Dist(pos, cur.edge());
                  if(dist<0 || d<dist){dist=d; height=pos_y;}
               }break;

               case HB_AVG:
               {
                  flt d=Brush.power(pos, false, cur);
                  height+=d*pos_y;
                  dist  +=d;
               }break;
            }
         }

         switch(mode())
         {
            case HB_LEVEL:
            case HB_FLAT : if(dist>=0)
            {
               SyncLocker locker(lock);
               if(cur._brush_height_help<0 || dist<cur._brush_height_help){cur._brush_height_help=dist; cur._brush_height=height;}
            }break;

            case HB_AVG:
            {
               SyncLocker locker(lock);
               cur._brush_height     +=height;
               cur._brush_height_help+=dist;
            }break;
         }
      }
   }
   void HeightBrushClass::UpdateHeight(Cell<Area> &cell, ptr user, int thread_index) {HeightBrush.updateHeight(cell());}
          void HeightBrushClass::updateHeight(     Area  &area)
   {
      if(area.hm && Brush.affected(area.xy))
      {
         ThreadMayUseGPUData();

         area.setChangedHeight(false);

         // include in changed
         {
            SyncLocker locker(lock); changed_areas.include(&area);
         }

         // detect objects
         Memt<Obj*> ground_objs;
         if(WorldEdit.hm_align)REPA(area.objs)if(area.objs[i]->onGround())ground_objs.add(area.objs[i]);

         // heightmap
         Heightmap2 &hm       =*area.hm;
         int         res      =WorldEdit.hmRes();
         flt         noise_mul=Min(WorldEdit.areaSize()/(res-1)*2, 1.0f); // use 1.0 limit because at greater scales the noise is too sharp
         int         nx       =Mod(Round(area.xy.x*(res-1)*noise_mul), HeightBrushNoiseRes),
                     ny       =Mod(Round(area.xy.y*(res-1)*noise_mul), HeightBrushNoiseRes);
         flt         size     =Brush.size,
                     speed    =Brush.speed;
         Vec2        area_pos =area .pos2D();
         Image      *noise    =null;
         switch(mode())
         {
         #if MOBILE
            case HB_ADD    :
            case HB_SUB    : if(Ms.b(0))speed=-speed; break;
         #else
            case HB_ADD_SUB: if(Ms.b(0))speed=-speed; break;
         #endif
            case HB_NOISE  : noise=&T.noise[Ms.b(1)]; break;
         }

         REPD(y, res)
         REPD(x, res)
         {
            Vec2 pos(x, y); pos*=WorldEdit.areaSize()/flt(res-1); pos+=area_pos; // (pos/res+cell)*UNIT == pos*UNIT/res + cell*UNIT
            if(flt d=Brush.powerTotal(pos))
            {
               flt pos_y=hm.height(x, y);
               switch(mode())
               {
               #if MOBILE
                  case HB_ADD    : pos_y+=d*speed; break;
                  case HB_SUB    : pos_y-=d*speed; break;
               #else
                  case HB_ADD_SUB: pos_y+=d*speed; break;
               #endif
                  case HB_AVG    : REP(WorldEdit.curTotal()){Cursor &cur=WorldEdit.curAll(i); if(cur.valid() && cur.on())AdjustValTime(pos_y,     cur._brush_height,       Lerp(1.0f, 0.5f, Brush.power(pos, true, cur)), speed);} break;
                  case HB_LEVEL  : REP(WorldEdit.curTotal()){Cursor &cur=WorldEdit.curAll(i); if(cur.valid() && cur.on())AdjustValTime(pos_y,     cur._brush_height,       Lerp(1.0f, 0.5f, Brush.power(pos, true, cur)), speed);} break;
                  case HB_FLAT   : REP(WorldEdit.curTotal()){Cursor &cur=WorldEdit.curAll(i); if(cur.valid() && cur.on())              pos_y+=Mid(cur._brush_height-pos_y, -speed, speed)*Brush.power(pos, true, cur)         ;} break;
                  case HB_NOISE  : pos_y+=d*speed*(noise->pixelFCubic(x*noise_mul+nx, y*noise_mul+ny, false)*2-1); break;
                  case HB_SOFT   :
                  {
                     flt sum=0;
                     if(x>    0)sum+=hm.height(x-1, y);else if(area.hm_l())sum+=area.hm_l()->height(res-2, y);else if(y==0 && area.hm_lb())sum+=area.hm_lb()->height(res-2, res-1);else if(y==res-1 && area.hm_lf())sum+=area.hm_lf()->height(res-2,     0);else sum+=pos_y;
                     if(x<res-1)sum+=hm.height(x+1, y);else if(area.hm_r())sum+=area.hm_r()->height(    1, y);else if(y==0 && area.hm_rb())sum+=area.hm_rb()->height(    1, res-1);else if(y==res-1 && area.hm_rf())sum+=area.hm_rf()->height(    1,     0);else sum+=pos_y;
                     if(y>    0)sum+=hm.height(x, y-1);else if(area.hm_b())sum+=area.hm_b()->height(x, res-2);else if(x==0 && area.hm_lb())sum+=area.hm_lb()->height(res-1, res-2);else if(x==res-1 && area.hm_rb())sum+=area.hm_rb()->height(    0, res-2);else sum+=pos_y;
                     if(y<res-1)sum+=hm.height(x, y+1);else if(area.hm_f())sum+=area.hm_f()->height(x,     1);else if(x==0 && area.hm_lf())sum+=area.hm_lf()->height(res-1,     1);else if(x==res-1 && area.hm_rf())sum+=area.hm_rf()->height(    0,     1);else sum+=pos_y;
                     AdjustValTime(pos_y, sum*0.25f, Max(Lerp(1.0f, 0.5f, d), 0.1f), speed);
                  }break;
               }
               hm.height(x, y, pos_y);
               hm.mesh.ext.includeY(pos_y*WorldEdit.areaSize()); // this needs to be done here because box is updated only on 'hmBuild' which is called occasionally
            }
         }

         // meshes
         flt  scale=(res-1)/WorldEdit.areaSize();
         Vec2 move =area.xy*-(res-1);
         REP(hm.mesh.lods())
         {
            MeshLod &lod=hm.mesh.lod(i);
            REPA(lod)
            {
               MeshRender &mshr  =lod .parts[i].render;
               int         offset=mshr.vtxOfs(VTX_POS);
               if(offset>=0)if(byte *vtx=(byte*)mshr.vtxLock())
               {
                  vtx+=offset;
                  REP(mshr.vtxs())
                  {
                     Vec &pos  =*(Vec*)vtx;
                          pos.y=hm.heightLinear(pos.x*scale+move.x, pos.z*scale+move.y)*WorldEdit.areaSize(); // because vertexes may be re-positioned during simplification, we can't use Round and access direct pixel, we need to use filtering
                     vtx+=mshr.vtxSize();
                  }
                  mshr.vtxUnlock();
               }
            }
         }

         // align objects
         REPAO(ground_objs)->alignTerrain(false, WorldUndo::TerrainType); // this change was caused by editing the Terrain so we need to set proper type
      }
   }
   void HeightBrushClass::detectWaypoints(C RectI &rect)
   {
      REPA(WorldEdit.visible_waypoints)
      {
         EditWaypoint &wp=*WorldEdit.visible_waypoints[i]; if(!wp.removed) //if(Cuts(wp.area, rect))
         {
            REPA(wp)
            {
               EditWaypointPoint &p=wp[i]; if(!p.removed)if(Cuts(WorldEdit.worldToArea(p.pos), rect))
               {
                  if(Abs(p.pos.y-WorldEdit.hmHeight(p.pos))<=0.1f)align_waypoints.New().set(wp, p);
               }
            }
         }
      }
   }
   void HeightBrushClass::alignWaypoints()
   {
      REPA(align_waypoints)
      {
         EditWaypointPoint &p=*align_waypoints[i].p;
         flt                y=WorldEdit.hmHeight(p.pos);
         if(!Equal(p.pos.y, y))
         {
            EditWaypoint *wp=align_waypoints[i].wp;
            if(wp)WorldEdit.undos.set(*wp, WorldUndo::TerrainType); // this change was caused by editing the Terrain so we need to set proper type
            p.setPos(Vec(p.pos.x, y, p.pos.z)); // align to heightmap
            WorldEdit.setChangedWaypoint(wp);
         }
      }
      align_waypoints.clear();
   }
   void HeightBrushClass::update()
   {
      bool view_on=false;
      if(visibleFull() && OpHm==OP_HM_HEIGHT)
      {
         REP(WorldEdit.curTotal())if(WorldEdit.curAll(i).onViewport() && WorldEdit.curAll(i).on()){view_on=true; break;}
         if(view_on)
         {
            // check if any cursor from those on the viewport is on but not pushed
            bool valid_on=false; REP(WorldEdit.curTotal())if(WorldEdit.curAll(i).valid() && WorldEdit.curAll(i).on()){valid_on=true; break;}
            if(  valid_on
            &&  !Kb.alt()) // rotate image
            {
               // get height
               switch(mode())
               {
                  case HB_LEVEL:
                  case HB_FLAT : REP(WorldEdit.curTotal())
                  {
                     Cursor &cur=WorldEdit.curAll(i); if(cur.valid() && cur.pd())
                     {
                        cur._brush_height     = 0;
                        cur._brush_height_help=-1;
                        WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(cur.edge()), GetHeight, cur);
                     }
                  }break;

                  case HB_AVG: REP(WorldEdit.curTotal())
                  {
                     Cursor &cur=WorldEdit.curAll(i); if(cur.valid())
                     {
                        cur._brush_height     =0;
                        cur._brush_height_help=0;
                        WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(cur.edge()), GetHeight, cur);
                        if(cur._brush_height_help)cur._brush_height/=cur._brush_height_help;
                     }
                  }break;
               }

               RectI affected_areas=Brush.affectedAreas();

               // add waypoints to the list
               if(WorldEdit.hm_align)detectWaypoints(affected_areas);

               // update heightmaps
               WorldEdit.grid.mtFunc(WorkerThreads, affected_areas, UpdateHeight);

               // align edges
               REPA(changed_areas)
               {
                  Area &area=*changed_areas[i];
                  if(Heightmap2 *hm=area.hm)
                  {
                     int res=WorldEdit.hmRes();

                     // first vertical
                     if(Heightmap2 *b=area.hm_b())REP(res)hm->height(i, 0    , b->height(i, res-1));
                     if(Heightmap2 *f=area.hm_f())REP(res)hm->height(i, res-1, f->height(i,     0));

                     // then horizontal
                     if(Heightmap2 *l=area.hm_l())REP(res)hm->height(0    , i, l->height(res-1, i));
                     if(Heightmap2 *r=area.hm_r())REP(res)hm->height(res-1, i, r->height(    0, i));

                     // then the corner
                     if(Heightmap2 *lf=area.hm_lf())hm->height(0, res-1, lf->height(res-1, 0));
                  }
               }

               // align waypoints
               alignWaypoints();

               // normals
               if(!(Time.frame()&0x7))updateChanged();
            }
         }
      }

      if(!view_on)updateChanged();
   }
   void HeightBrushClass::AreaUpdate(Area* &area, ptr user, int thread_index)
   {
      if(area->hm)
      {
         ThreadMayUseGPUData();
         area->hmBuild();
         area->setChangedHeight(); // this function may be called a few frames later, after the builder already finished processing it, so call changed height to make sure to rebuild from latest heightmap mesh that we've just built
      }
   }
   void HeightBrushClass::updateChanged()
   {
      if(changed_areas.elms())
      {
         WorkerThreads.process1(changed_areas, AreaUpdate);
                                changed_areas.clear();
      }
   }
/******************************************************************************/
