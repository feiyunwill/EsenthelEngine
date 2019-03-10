/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
/******************************************************************************

   World Objects are synchronized in following way:
      -Assuming that there will be a lot of objects in all worlds
      -Taking that in mind, synchronizing will not exchange all object Version's
      -Instead it will exchange AreaVer.obj_ver (version of all objects in an area)
      -if local ver is different than server, then request all objects in that area from the server
      -if after syncing all objects are identical in local as from the server (including number of objects) then set local AreaVer.obj_ver to the one from server
      -in other case {if(syncing changed anything)randomize local AreaVer.obj_ver} and {send different objects from that area to the server}
      -modifying objects on the client side in world editor will make them sent individually (not its area and all objects located there, however if many objects were modified in one area, then try groupping and sending those modified objects)
   This approach is the best I could think of, alternatives are:
      -1. exchange versions for all objects in all worlds separately (this would be lots of ID's and Version's each time client is connected to Server)
      -2. modifying objects would send all objects from that area to the Server with the new AreaVer.obj_ver to somehow prevent (upon reconnecting) Server sending objects from modified areas since last time which we already have (however this would result in too big bandwidth each time an object is modified)
      -Using above approach I use least bandwitdh (the only downside is AreaVer.obj_ver is modified on the client only at connecting to Server after receiving Server data)

/******************************************************************************/
SynchronizerClass Synchronizer;
/******************************************************************************/

/******************************************************************************/
   int SynchronizerClass::CompareDepth(C ElmDepth &a, C ElmDepth &b)
   {
      if(int c=Compare(a.depth   , b.depth   ))return c;
      return   Compare(a.elm->name, b.elm->name);
   }
      int SynchronizerClass::ElmFile::size()C {return elm.size()+data.size();}
      void SynchronizerClass::ElmFile::process()
      {
         if(compress)
         {
            data.pos(0); File temp; Compress(data, temp.writeMem(), compress_fast ? ServerNetworkCompression : ClientNetworkCompression, compress_fast ? ServerNetworkCompressionLevel : ClientNetworkCompressionLevel); Swap(data, temp);
         }
      }
      void SynchronizerClass::ElmFile::store(File &f)
      {
         process();
         elm.pos(0); elm.copy(f); data.pos(0); data.copy(f);
      }
      bool SynchronizerClass::WorldSync::empty()C {return !set_area.elms() && !set_obj.elms();}
      int  SynchronizerClass::WorldSync::elms()C {return  set_area.elms() +   set_obj.elms();}
      void SynchronizerClass::WorldSync::getServerVer(C UID &world_id) {if(!server_ver)server_ver=Server.world_vers.find(world_id);}
   bool SynchronizerClass::Create(WorldSync &world_sync, C UID &world_id, ptr)
   {
      world_sync.getServerVer(world_id);
      world_sync.local_ver=Proj.worldVerRequire(world_id);
      return world_sync.server_ver || world_sync.local_ver;
   }
      bool SynchronizerClass::MiniMapSync::empty()C {return !set_image.elms();}
      int  SynchronizerClass::MiniMapSync::elms()C {return  set_image.elms();}
      void SynchronizerClass::MiniMapSync::getServerVer(C UID &mini_map_id) {if(!server_ver)server_ver=Server.mini_map_vers.find(mini_map_id);}
   bool SynchronizerClass::Create(MiniMapSync &mini_map_sync, C UID &mini_map_id, ptr)
   {
      mini_map_sync.getServerVer(mini_map_id);
      mini_map_sync.local_ver=Proj.miniMapVerRequire(mini_map_id);
      return mini_map_sync.server_ver || mini_map_sync.local_ver;
   }
  SynchronizerClass::~SynchronizerClass() {clearSync(); thread.del();}
   bool SynchronizerClass::CompressFunc(Thread &thread) {return Synchronizer.compressFunc();}
          bool SynchronizerClass::compressFunc()
   {
      if(set_elm_full_file.elms() && Server.smallBuf()) // full elements first
      {
         SyncLockerEx locker(lock); if(set_elm_full_file.elms())
         {
            ElmFile elm; Swap(elm, set_elm_full_file.first()); set_elm_full_file.remove(0, true); compressing=true;
            {
               locker.off();
               File cmd; elm.store(cmd.writeMem());
               cmd.pos(0); locker.on(); Swap(cmd, cmds.New()); compressing=false;
            }
         }
      }else
      if(set_elm_long_file.elms() && Server.smallBuf()) // then long elemenets
      {
         SyncLockerEx locker(lock); if(set_elm_long_file.elms())
         {
            ElmFile elm; Swap(elm, set_elm_long_file.first()); set_elm_long_file.remove(0, true); compressing=true;
            {
               locker.off();
               File cmd; elm.store(cmd.writeMem());
               cmd.pos(0); locker.on(); Swap(cmd, cmds.New()); compressing=false;
            }
         }
      }else
      if(set_tex.elms() && Server.smallBuf()) // last textures
      {
         SyncLockerEx locker(lock); if(set_tex.elms())
         {
            UID tex_id=set_tex[0]; set_tex.remove(0, true); File tex_data; if(tex_data.readTry(tex_path+EncodeFileName(tex_id))) // texture can be read on secondary thread because each texture file is written only once (name stands for texture hash, so it will never be overwritten with different data)
            {
               compressing=true; locker.off();
               File cmd; if(ClientWriteSetTexture(cmd.writeMem(), tex_id, tex_data))
               {
                  cmd.pos(0); locker.on(); Swap(cmd, cmds.New()); compressing=false;
               }
            }
         }
      }else
      {
         Time.wait(1);
      }
      return true;
   }
   int SynchronizerClass::elmFileSize() {int size=0; SyncLocker locker(lock); REPA(set_elm_full_file)size+=set_elm_full_file[i].size(); REPA(set_elm_long_file)size+=set_elm_long_file[i].size(); return size;}
   int SynchronizerClass::queuedElms()
   {
      int n=set_elm_full_file.elms() + set_elm_long_file.elms() + set_elm_full.elms() + set_elm_long.elms() + set_elm_short.elms() + set_tex.elms() + cmds.elms() + compressing;
      REPA(   world_sync)n+=   world_sync.lockedData(i).elms();
      REPA(mini_map_sync)n+=mini_map_sync.lockedData(i).elms();
      return n;
   }
   void SynchronizerClass::clearSync()
   {
      thread.stop();
      if(lock.created()) // this can be called after destructor
      {
         SyncLocker locker(lock);
         set_elm_full .clear(); set_elm_full_file.clear();
         set_elm_long .clear(); set_elm_long_file.clear();
         set_elm_short.clear();
         set_tex      .clear();
         cmds         .clear();
         delayed_world_sync.del();
                 world_sync.del();
              mini_map_sync.del();
      }
   }
   bool SynchronizerClass::getCmds(Memc<File> &cmds)
   {
      cmds.clear();
      if(T.cmds.elms())
      {
         SyncLocker locker(lock);
         Swap(cmds, T.cmds);
      }
      return cmds.elms()>0;
   }
   void SynchronizerClass::setArea(C UID &world_id, C VecI2 &area_xy)
   {
      if(Server.canWrite() && world_id.valid())if(WorldSync *ws=world_sync.get(world_id))ws->set_area.binaryInclude(area_xy, Compare);
   }
   void SynchronizerClass::setObjs(C UID &world_id, Memc<UID> &obj_ids)
   {
      if(Server.canWrite() && world_id.valid() && obj_ids.elms())if(WorldSync *ws=world_sync.get(world_id))REPA(obj_ids)ws->set_obj.binaryInclude(obj_ids[i], Compare);
   }
   void SynchronizerClass::setMiniMapImage(C UID &mini_map_id, C VecI2 &image_xy)
   {
      if(Server.canWrite() && mini_map_id.valid())if(MiniMapSync *mms=mini_map_sync.get(mini_map_id))mms->set_image.binaryInclude(image_xy, Compare);
   }
   void SynchronizerClass::delayedSetArea(C UID &world_id, C VecI2 &area_xy) // !! must be multi-threaded SAFE !!
   {
      if(Server.canWrite() && world_id.valid())
      {
         MapLock ml(delayed_world_sync); // use lock because this may be called on multiple threads by 'Area.setServer'
         if(WorldSync *ws=delayed_world_sync.get(world_id))ws->set_area.binaryInclude(area_xy, Compare);
      }
   }
   void SynchronizerClass::delayedSetObj(C UID &world_id, C MemPtr<UID> &obj_id) // !! must be multi-threaded SAFE !!
   {
      if(Server.canWrite() && world_id.valid())
      {
         MapLock ml(delayed_world_sync); // use lock because this may be called on multiple threads by 'Area.setChangedObj'
         if(WorldSync *ws=delayed_world_sync.get(world_id))
            REPA(obj_id)if(obj_id[i].valid())ws->set_obj.binaryInclude(obj_id[i], Compare);
      }
   }
   void SynchronizerClass::erasing(C UID &elm_id)
   {
      delayed_world_sync.removeKey(elm_id);
              world_sync.removeKey(elm_id);
           mini_map_sync.removeKey(elm_id);
           set_elm_full .exclude  (elm_id, true);
           set_elm_long .exclude  (elm_id, true);
           set_elm_short.exclude  (elm_id, true);
   }
   void SynchronizerClass::erasingTex(C UID &tex_id)
   {
      SyncLocker locker(lock);
      set_tex.exclude(tex_id, true);
   }
   void SynchronizerClass::setElmFull(C UID &elm_id) {if(elm_id.valid()){/*SyncLocker locker(lock);*/                              set_elm_full .include(elm_id);}}
   void SynchronizerClass::setElmLong(C UID &elm_id) {if(elm_id.valid()){/*SyncLocker locker(lock);*/                              set_elm_long .include(elm_id);}}
   void SynchronizerClass::setElmShort(C UID &elm_id) {if(elm_id.valid()){/*SyncLocker locker(lock);*/ if(!set_elm_long.has(elm_id))set_elm_short.include(elm_id);}}
   void SynchronizerClass::setTex(C UID &tex_id) {if(tex_id.valid()){  SyncLocker locker(lock);                                set_tex      .include(tex_id);}}
   void SynchronizerClass::createLocal(Project &local, Memx<Elm> &server) // process recursively to create parents first
   {
      FREPA(server)
      {
         Elm &s=server[i],
             &l=local.getElm(s.id);
         if( !l.type) // just added
         {
            // skip 'name', 'name_time' and 'data' because it was not sent yet
       if(1)l.name="_Downloading..";
            l.type=s.type;
            l.flag=s.flag;
            l.    parent_id  =s.    parent_id;
            l.    parent_time=s.    parent_time;
            l.   removed_time=s.   removed_time;
            l.no_publish_time=s.no_publish_time;
            l.opened(false); // keep new downloaded elements as closed
         }
      }
   }
   void SynchronizerClass::sync(Project &local, Project &server)
   {
      if(!Proj.testElmsNum())return;
      Proj.setListCurSel();
      {
         SyncLocker locker(lock);
         game_path=Proj.game_path;
         edit_path=Proj.edit_path;
          tex_path=Proj. tex_path;
      }
      createLocal(local, server.elms);
      Server.texs=server.texs;
      Proj.setHierarchy(); // set hierarchy needed for various checks

      // compare versions
      Memc<UID     > get_names, get_textures, set_textures, get_short, get_long, set_long;
      Memc<Elm*    > set_names, set_parents, set_removed, set_no_publish;
      Memc<ElmDepth> set_full;
      FREPA(local.elms)
      {
         Elm &l=local.elms[i], *s=server.findElm(l.id);
         if( !s)set_full.New().elm=&l;else // if server doesn't have the element then send it fully
         {
            if(l.name_time<s->name_time)get_names.add(l.id);else
            if(l.name_time>s->name_time)set_names.add(&l);

            if(l.parent_time<s->parent_time)l.setParent(s->parent_id, s->parent_time);else
            if(l.parent_time>s->parent_time)set_parents.add(&l);

            if(l.removed_time<s->removed_time)l.setRemoved(s->removed(), s->removed_time);else
            if(l.removed_time>s->removed_time)set_removed.add(&l);

            if(l.no_publish_time<s->no_publish_time)l.setNoPublish(s->noPublish(), s->no_publish_time);else
            if(l.no_publish_time>s->no_publish_time)set_no_publish.add(&l);

            // data
            if(s->type==l.type) // just in case
               if(!ElmManualSync(l.type))
            {
               if(s->data && (!l.data || s->data->ver!=l.data->ver)) // server has data and (local doesn't or has different version) -> get data
               {
                  if(l.initialized())get_short.add(l.id); // if we have some data, then download the small version, to see what we need to synchronize, as perhaps some parts are already OK
                  else               get_long .add(l.id); // if we have nothing  , then download everything
               }else
               if(l.data && !s->data) // local has data but server doesn't
               {
                  set_long.add(l.id);
               }
            }
         }
      }

      // textures
      Memc<UID> local_texs; local.getTextures(local_texs);
      REPA(server.texs)if(!local .texs.binaryHas(server.texs[i], Compare))get_textures.add(server.texs[i]);
      REPA( local_texs)if(!server.texs.binaryHas( local_texs[i], Compare))set_textures.add( local_texs[i]);

      // sync
      Memc<UID> temp;
                           Server.getElmNames ( get_names     );
                           Server.getElmShort ( get_short     );
                           Server.getElmLong  ( get_long      );
                           Server.getTextures ( get_textures  );
      FREPA(set_names     )Server.renameElm   (*set_names  [i]);
      FREPA(set_parents   )Server.setElmParent(*set_parents[i]);
      FREPA(set_removed   ){Elm &elm=*set_removed   [i]; temp(0)=elm.id; Server.   removeElms(temp, elm.  removed(), elm.   removed_time);}
      FREPA(set_no_publish){Elm &elm=*set_no_publish[i]; temp(0)=elm.id; Server.noPublishElms(temp, elm.noPublish(), elm.no_publish_time);}

      // send elms
      REPAO(set_full).depth=Proj.depth(set_full[i].elm); set_full.sort(CompareDepth); // sort by depth so root elements are sent first
      FREPA(set_full)Server.setElmFull(set_full[i].elm->id);

      // set data (after sending elements)
      FREPA(set_long)Server.setElmLong(set_long[i]);

      // send textures (after sending elements)
      FREPA(set_textures)Server.setTex(set_textures[i]);

      Proj.setList();

      if(!thread.active())thread.create(CompressFunc, this);

      FREPA(Proj.elms)if(Proj.elms[i].type==ELM_WORLD   )Server.getWorldVer  (Proj.elms[i].id);
      FREPA(Proj.elms)if(Proj.elms[i].type==ELM_MINI_MAP)Server.getMiniMapVer(Proj.elms[i].id);
   }
   void SynchronizerClass::syncWorld(C UID &world_id)
   {
      if(world_id.valid())
         if(WorldSync *ws=world_sync.get(world_id))
      {
         Memc<AreaSync> area_get;
         Memc<UID     > waypoints_get, lakes_get, rivers_get;

         // check local to send
         if(Server.canWrite() && ws->local_ver) // only if can write
         {
            REPA(ws->local_ver->areas) // areas
            {
               VecI2        area_xy= ws-> local_ver->areas.lockedKey (i);
               AreaVer & local_area= ws-> local_ver->areas.lockedData(i),
                       *server_area=(ws->server_ver ? ws->server_ver->areas.find(area_xy) : null);
               if(uint area_sync_flag=local_area.compare(server_area))
               {
                  if( area_sync_flag&AREA_SYNC_HM )ws->set_area.binaryInclude(area_xy, Compare); // don't send objects in this way
                  if((area_sync_flag&AREA_SYNC_OBJ) && !server_area)area_get.New().set(AREA_SYNC_OBJ, area_xy); // if objects need to be synced and this will not be processed below again "!server_area"
               }
            }
            REPA(ws->local_ver->waypoints) // waypoints
            {
             C UID     &waypoint_id =ws->local_ver->waypoints.lockedKey (i);
             C Version &waypoint_ver=ws->local_ver->waypoints.lockedData(i);
               if(!ws->server_ver || !ws->server_ver->waypoints.find(waypoint_id)) // if was not found on the server at all
               {
                  EditWaypoint waypoint; if(waypoint.load(Proj.editWaypointPath(world_id, waypoint_id)))Server.setWaypoint(world_id, waypoint_id, waypoint_ver, waypoint);
               }
            }
            REPA(ws->local_ver->lakes) // lakes
            {
             C UID     &lake_id =ws->local_ver->lakes.lockedKey (i);
             C Version &lake_ver=ws->local_ver->lakes.lockedData(i).ver;
               if(!ws->server_ver || !ws->server_ver->lakes.find(lake_id)) // if was not found on the server at all
               {
                  Lake lake; if(lake.load(Proj.editLakePath(world_id, lake_id)))Server.setLake(world_id, lake_id, lake_ver, lake);
               }
            }
            REPA(ws->local_ver->rivers) // rivers
            {
             C UID     &river_id =ws->local_ver->rivers.lockedKey (i);
             C Version &river_ver=ws->local_ver->rivers.lockedData(i).ver;
               if(!ws->server_ver || !ws->server_ver->rivers.find(river_id)) // if was not found on the server at all
               {
                  River river; if(river.load(Proj.editRiverPath(world_id, river_id)))Server.setRiver(world_id, river_id, river_ver, river);
               }
            }
         }

         // check server to receive
         if(ws->server_ver)
         {
            REPA(ws->server_ver->areas) // areas
            {
               VecI2        area_xy= ws->server_ver->areas.lockedKey (i);
               AreaVer &server_area= ws->server_ver->areas.lockedData(i),
                       * local_area=(ws-> local_ver ? ws->local_ver->areas.find(area_xy) : null);
               if(uint area_sync_flag=server_area.compare(local_area))
               {
                  if(area_sync_flag&AREA_SYNC_REMOVED) // if we need to delete the heightmap, then no need to receive data from server but just delete it
                  {
                     Proj.hmDel(world_id, area_xy, &server_area.hm_removed_time);
                     FlagDisable(area_sync_flag, AREA_SYNC_REMOVED);
                  }
                  if(area_sync_flag)area_get.New().set(area_sync_flag, area_xy);
               }
            }
            REPA(ws->server_ver->waypoints) // waypoints
            {
             C UID     &waypoint_id =ws->server_ver->waypoints.lockedKey (i);
             C Version &waypoint_ver=ws->server_ver->waypoints.lockedData(i);
               Version *   local_ver=(ws->local_ver ? ws->local_ver->waypoints.find(waypoint_id) : null);
               if(!local_ver || waypoint_ver!=*local_ver)waypoints_get.add(waypoint_id); // if not found in local, or is of different version, then receive from server and sync
            }
            REPA(ws->server_ver->lakes) // lakes
            {
             C UID      & lake_id =ws->server_ver->lakes.lockedKey (i);
             C Version  & lake_ver=ws->server_ver->lakes.lockedData(i).ver;
               WaterVer *local_ver=(ws->local_ver ? ws->local_ver->lakes.find(lake_id) : null);
               if(!local_ver || lake_ver!=local_ver->ver)lakes_get.add(lake_id); // if not found in local, or is of different version, then receive from server and sync
            }
            REPA(ws->server_ver->rivers) // rivers
            {
             C UID      &river_id =ws->server_ver->rivers.lockedKey (i);
             C Version  &river_ver=ws->server_ver->rivers.lockedData(i).ver;
               WaterVer *local_ver=(ws->local_ver ? ws->local_ver->rivers.find(river_id) : null);
               if(!local_ver || river_ver!=local_ver->ver)rivers_get.add(river_id); // if not found in local, or is of different version, then receive from server and sync
            }
         }
         Server.getWorldAreas    (world_id,      area_get);
         Server.getWorldWaypoints(world_id, waypoints_get);
         Server.getWorldLakes    (world_id,     lakes_get);
         Server.getWorldRivers   (world_id,    rivers_get);
      }
   }
   void SynchronizerClass::syncMiniMap(C UID &mini_map_id)
   {
      if(mini_map_id.valid())
         if(MiniMapSync *mms=mini_map_sync.get(mini_map_id))
      {
         // sync settings
         if(mms->server_ver)Proj.syncMiniMapSettings(mini_map_id, mms->server_ver->settings, mms->server_ver->time); // try to replace from server
         if(mms-> local_ver)
            if(!mms->server_ver || mms->local_ver->time>mms->server_ver->time)Server.setMiniMapSettings(mini_map_id, mms->local_ver->settings, mms->local_ver->time); // send to server

         // sync images
         Memc<VecI2> get_image;

         // check local to send
         if(Server.canWrite() && mms->local_ver) // only if can write
            if(!mms->server_ver || mms->local_ver->time>=mms->server_ver->time) // send only if the time of the images is same/newer than on the server (check for same in case some images did not finish sending yet)
               FREPA(mms->local_ver->images)
                  if(!mms->server_ver || mms->local_ver->time>mms->server_ver->time || !mms->server_ver->images.binaryHas(mms->local_ver->images[i], Compare)) // if server ver doesn't exist, or is old, or is the same time but doesn't have the image yet
                     mms->set_image.binaryInclude(mms->local_ver->images[i], Compare); // mark to be sent

         // check server to receive
         if(mms->server_ver)
            if(!mms->local_ver || mms->server_ver->time>=mms->local_ver->time) // receive only if the time of the images is same/newer than on the client (check for same in case some images did not finish sending yet)
               FREPA(mms->server_ver->images)
                  if(!mms->local_ver || mms->server_ver->time>mms->local_ver->time || !mms->local_ver->images.binaryHas(mms->server_ver->images[i], Compare)) // if local ver doesn't exist, or is old, or is the same time but doesn't have the image yet
                     get_image.binaryInclude(mms->server_ver->images[i], Compare); // mark to be received

         Server.getMiniMapImages(mini_map_id, get_image);
      }
   }
   void SynchronizerClass::update()
   {
      // move 'delayed_world_sync' to 'world_sync'
      if(!delayed_world_sync.elms())last_delayed_time=Time.realTime();else // if there are no elements then set last time to current time so after adding an element it won't be sent right away
      if(Time.realTime()-last_delayed_time>=SendAreasDelay) // if enough time has passed
      {
         last_delayed_time=Time.realTime();
         MapLock ml(delayed_world_sync);
         FREPA(delayed_world_sync)
         {
            UID     world_id=delayed_world_sync.lockedKey(i);
            if(WorldSync *ws=world_sync.get(world_id))
            {
               WorldSync &dws=delayed_world_sync.lockedData(i);
               FREPA(dws.set_area)ws->set_area.binaryInclude(dws.set_area[i], Compare);
               FREPA(dws.set_obj )ws->set_obj .binaryInclude(dws.set_obj [i], Compare);
            }
         }
         delayed_world_sync.del();
      }

      // if we have any elements to send
      if(set_elm_full.elms() || set_elm_long.elms() || set_elm_short.elms() || world_sync.elms() || mini_map_sync.elms())
      {
         const uint time=Time.curTimeMs(), delay=16; // 1000ms/60fps

         // set elm short
         if(Server.smallBuf())for(; set_elm_short.elms()>0; )
         {
            UID elm_id=set_elm_short[0]; Proj.flushElm(elm_id); set_elm_short.remove(0, true); // flush before sending to make sure that file has latest data, do this before removing from element to send, in case flush will include that element in the send-to list
            if(Elm *elm=Proj.findElm(elm_id))
            {
               ClientSendSetElmShort(Server, *elm, Proj);
               if(Time.curTimeMs()-time>=delay || !Server.smallBuf())goto skip;
            }
         }

         // set elm full
         if(elmFileSize()<=ServerSendBufSize)for(; set_elm_full.elms()>0; )
         {
            UID elm_id=set_elm_full[0]; Proj.flushElm(elm_id); set_elm_full.remove(0, true); // flush before sending to make sure that file has latest data, do this before removing from element to send, in case flush will include that element in the send-to list
            if(Elm *elm=Proj.findElm(elm_id))
            {
               ElmFile elm_file; ClientWriteSetElmFull(elm_file.elm.writeMem(), elm_file.data.writeMem(), *elm, Proj, elm_file.compress);
               {SyncLocker locker(lock); Swap(elm_file, set_elm_full_file.New());}
               if(Time.curTimeMs()-time>=delay || elmFileSize()>ServerSendBufSize)goto skip;
            }
         }

         // set elm long
         if(elmFileSize()<=ServerSendBufSize)for(; set_elm_long.elms()>0; )
         {
            UID elm_id=set_elm_long[0]; Proj.flushElm(elm_id); set_elm_long.remove(0, true); // flush before sending to make sure that file has latest data, do this before removing from element to send, in case flush will include that element in the send-to list
            if(Elm *elm=Proj.findElm(elm_id))
            {
               ElmFile elm_file; ClientWriteSetElmLong(elm_file.elm.writeMem(), elm_file.data.writeMem(), *elm, Proj, elm_file.compress);
               {SyncLocker locker(lock); Swap(elm_file, set_elm_long_file.New());}
               if(Time.curTimeMs()-time>=delay || elmFileSize()>ServerSendBufSize)goto skip;
            }
         }

         // world sync
         if(elmFileSize()<=ServerSendBufSize)for(int i=0; i<world_sync.elms(); )
         {
            UID  world_id=world_sync.lockedKey (i);
            WorldSync &ws=world_sync.lockedData(i);

            // iterate all areas to send, before sending check which parts of the area actually need to be sent
            REPAD(j, ws.set_area)
            {
               VecI2 area_xy=ws.set_area[j]; ws.set_area.remove(j, true);
               if(ws.local_ver)
                  if(AreaVer *local_area=ws.local_ver->areas.find(area_xy))
                     if(!local_area->oldHm())ws.set_area.NewAt(j)=area_xy;else // send only if the data is old to make sure that new changes will set new timestamp (if data is not old then put it back to send it later)
               {
                  ws.getServerVer(world_id); // try getting 'server_ver' if it's not set yet (this is needed in case the server ver is received after starting syncing)
                     AreaVer *server_area=(ws.server_ver ? ws.server_ver->areas.find(area_xy) : null);
                  if(uint     area_sync_flag=(local_area->compare(server_area)&AREA_SYNC_HM)) // if we have anything to send, don't send objects here
                  {
                     if(!server_area && ws.server_ver)server_area=ws.server_ver->areas.get(area_xy);
                     if( server_area)server_area->sync(*local_area); // we're sending new data so let's update the 'server_area' to assume that after sending the data it receives them

                     Heightmap temp, *hm=null;
                     if(area_sync_flag&(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR))hm=Proj.hmGet(world_id, area_xy, temp);
                     ElmFile elm_file; elm_file.compress=elm_file.compress_fast=true;
                     ClientWriteSetWorldArea(elm_file.elm.writeMem(), elm_file.data.writeMem(), world_id, area_xy, area_sync_flag, *local_area, hm);
                     {SyncLocker locker(lock); Swap(elm_file, set_elm_full_file.New());} // !! WARNING: here is 'set_elm_full_file', but we can use it !!
                     if(Time.curTimeMs()-time>=delay || elmFileSize()>ServerSendBufSize)goto skip;
                  }
               }
            }

            // iterate all objects to send, take first then iterate all remaining and group those that are from the same area (for better IO performance, send them as group)
            REPAD(j, ws.set_obj)if(InRange(j, ws.set_obj)) // extra check because we can remove multiple objects during one iteration
            {
               Memc<UID> obj_ids; obj_ids.add(ws.set_obj[j]); ws.set_obj.remove(j, true);
               if(ws.local_ver)
                  if(ObjVer *obj_ver=ws.local_ver->obj.find(obj_ids.first())) // get object version
               {
                  VecI2 area_xy=obj_ver->area_xy; // get area of that object
                  REPA(ws.set_obj)if(ObjVer *obj_ver=ws.local_ver->obj.find(ws.set_obj[i]))if(obj_ver->area_xy==area_xy) // get all objects from that area
                  {
                     obj_ids.binaryInclude(ws.set_obj[i], Compare); ws.set_obj.remove(i, true);
                  }
                  // now we have all ID's of objects to send
                  Memc<ObjData> objs; Proj.objGet(world_id, area_xy, obj_ids, objs); // get objects from areas
                  ElmFile elm_file; elm_file.compress=elm_file.compress_fast=true;
                  ClientWriteSetWorldObjs(elm_file.elm.writeMem(), elm_file.data.writeMem(), world_id, area_xy, objs, Proj.edit_path); // send to server
                  {SyncLocker locker(lock); Swap(elm_file, set_elm_full_file.New());} // !! WARNING: here is 'set_elm_full_file', but we can use it !!
                  if(Time.curTimeMs()-time>=delay || elmFileSize()>ServerSendBufSize)goto skip;
               }
            }
            if(ws.empty())world_sync.remove(i);else i++;
         }

         // mini map sync
         if(elmFileSize()<=ServerSendBufSize)for(int i=0; i<mini_map_sync.elms(); )
         {
            UID  mini_map_id=mini_map_sync.lockedKey (i);
            MiniMapSync &mms=mini_map_sync.lockedData(i);
            for(; mms.set_image.elms(); )
            {
               VecI2 image_xy=mms.set_image.last(); mms.set_image.removeLast();
               if(mms.local_ver)
               {
                  File  image_data; image_data.readTry(Proj.gamePath(mini_map_id).tailSlash(true)+image_xy);
                  ElmFile elm_file; elm_file.compress=true;
                  ClientWriteSetMiniMapImage(elm_file.elm.writeMem(), elm_file.data.writeMem(), mini_map_id, image_xy, mms.local_ver->time, image_data);
                  {SyncLocker locker(lock); Swap(elm_file, set_elm_full_file.New());} // !! WARNING: here is 'set_elm_full_file', but we can use it !!
                  if(Time.curTimeMs()-time>=delay || elmFileSize()>ServerSendBufSize)goto skip;
               }
            }
            if(mms.empty())mini_map_sync.remove(i);else i++;
         }

      skip:;
      }
   }
SynchronizerClass::SynchronizerClass() : compressing(false), world_sync(Compare, Create), delayed_world_sync(Compare, Create), mini_map_sync(Compare, Create), last_delayed_time(0) {}

SynchronizerClass::ElmDepth::ElmDepth() : depth(0), elm(null) {}

SynchronizerClass::ElmFile::ElmFile() : compress(false), compress_fast(false) {}

SynchronizerClass::WorldSync::WorldSync() : server_ver(null), local_ver(null) {}

SynchronizerClass::MiniMapSync::MiniMapSync() : server_ver(null), local_ver(null) {}

/******************************************************************************/
