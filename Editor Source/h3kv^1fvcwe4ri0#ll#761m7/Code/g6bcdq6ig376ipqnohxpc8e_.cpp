/******************************************************************************/
class Client : ConnectionServer.Client
{
   class WorldSync
   {
      static bool Create(byte &area_sync_flag, C VecI2 &area_xy, ptr) {area_sync_flag=0; return true;}

      Map<VecI2, byte> areas(Compare, Create);
   }
   class MiniMapSync
   {
      Memc<VecI2> images;
   }

   bool                  connection_version_ok=false, // if client/server use the same version of the protocol
                         disconnecting=false;
   flt                   disconnect_time=0;
   Str                   license_key;
   OS_VER                os_ver=OS_UNKNOWN;
   User                 *user=null;
   Project              *project=null; // project that this user works on
   Memc<UID>             send_tex, send_elm_short, send_elm_long, send_elm_full;
   Map<UID,   WorldSync>    world_sync(Compare);
   Map<UID, MiniMapSync> mini_map_sync(Compare);

  ~Client()
   {
      setUser(null); // unlink from current user

      // client is removed so add client connection usage stats to the server
      Server.bytes_sent    +=connection.sent    ();
      Server.bytes_received+=connection.received();
   }

   bool smallBuf() {return connection.queued()<=ClientSendBufSize;} // if buffer usage is small

   void clear() {license_key.clear(); os_ver=OS_UNKNOWN; setUser(null); setProject(null);}

   void disconnect() {if(!disconnecting){disconnecting=true; clear(); disconnect_time=Time.appTime()+4;}} // give a few seconds to flush all remaining data

   void setUser(User *user)
   {
      if(T.user!=user)
      {
         if(T.user     )T.user.changeClients(-1); // unlink old
         if(T.user=user)T.user.changeClients(+1); //   link new
      }
   }

   void setProject(Project *project)
   {
      if(T.project!=project)
      {
         T.project=project;
         send_tex      .clear();
         send_elm_short.clear();
         send_elm_long .clear();
         send_elm_full .clear();
            world_sync .del  ();
         mini_map_sync .del  ();
      }
   }

   virtual bool update()override
   {
      if(!super.update())return false;

      if(disconnecting)return connection.queued() && Time.appTime()<disconnect_time; // keep going as long as we have queued data to send and time limit wasn't reached

      REP(8)if(!connection.receive(0))break;else // process 8 commands at once
      {
         byte cmd=connection.data.getByte();
         if(  cmd>=CS_NUM)return false; // invalid command

         // version must be checked first, before all other commands
         if(cmd==CS_VERSION_CHECK)ServerSendVersion(connection, connection_version_ok=ServerRecvVersion(connection.data));
         if(!connection_version_ok)disconnect(); // if client/server use different versions then don't continue but terminate the connection instead
         else                      switch(cmd) // at this stage we're sure that the version is ok, so check other commands
         {
            case CS_LOGIN:
            {
               clear();

               Str email, license_key; UID pass; OS_VER os_ver; ServerRecvLogin(connection.data, email, pass, license_key, os_ver);
               LOGIN_RESULT result=(AllowUserRegistration() ? LOGIN_EMAIL_NOT_FOUND_REGISTRATION_AVAILABLE : LOGIN_EMAIL_NOT_FOUND_REGISTRATION_UNAVAILABLE);
               USER_ACCESS  access=UA_NO_ACCESS;
               REPA(Users)
               {
                  User &user=Users[i];
                  if(user.email==email)
                  {
                     result=LOGIN_EMAIL_FOUND_INVALID_PASS;
                     if(user.pass==pass)
                     {
                        result=LOGIN_SUCCESS;
                        user.login_date.getUTC();
                        if(user.access!=UA_NO_ACCESS)
                        {
                           setUser(&user);
                           T.license_key=license_key; // (ValidLicenseKey(license_key) ? license_key : S); allow any keys in case of steam subscriptions
                           T.os_ver     =os_ver;
                           result=Server.connected(T);
                           access=((result==LOGIN_SUCCESS) ? user.access : UA_NO_ACCESS);
                        }
                     }
                     break;
                  }
               }
               ServerSendLogin(connection, result, access); if(access==UA_NO_ACCESS)disconnect();
            }break;

            case CS_REGISTER:
            {
               clear();

               Str email, name; UID pass; ServerRecvRegister(connection.data, email, pass, name);
               ServerSendRegister(connection, RegisterUser(email, name, pass)); disconnect(); // disconnect because on fail and on success the user still has NO_ACCESS
            }break;

            case CS_FORGOT_PASS:
            {
               clear();

               Str email; ServerRecvForgotPass(connection.data, email);
               ServerSendForgotPass(connection, ForgotPass(email)); disconnect();
            }break;

            case CS_CHANGE_PASS:
            {
               Str email; UID new_pass, old_pass; uint change_pass_key; ServerRecvChangePass(connection.data, email, new_pass, old_pass, change_pass_key);
               if(!user){ServerSendChangePass(connection, ChangePassword(email, change_pass_key, new_pass)); disconnect();}else // not logged in -> use key and new pass -> disconnect
               if(user.email!=email   )ServerSendChangePass(connection, CHANGE_PASS_EMAIL_NOT_FOUND);else // logged in -> verify email -> use old and new pass
               if(user.pass !=old_pass)ServerSendChangePass(connection, CHANGE_PASS_INVALID_PASS   );else
               {
                  user.pass=new_pass; SaveSettings(); ServerSendChangePass(connection, CHANGE_PASS_SUCCESS);
               }
            }break;

            case CS_LICENSE_KEY: if(user)
            {
               Str license_key; ServerRecvLicenseKey(connection.data, license_key);
               T.license_key=license_key; // (ValidLicenseKey(license_key) ? license_key : S); allow any keys in case of steam subscriptions
               LOGIN_RESULT result=Server.connected(T);
               if(result!=LOGIN_SUCCESS){ServerSendLogin(connection, result, user.access); disconnect();}
            }break;

            case CS_PROJECTS_LIST: if(user)
            {
               ServerSendProjectsList(connection, Projects);
            }break;

            case CS_PROJECT_OPEN: if(user)
            {
               UID proj_id; Str proj_name; ServerRecvProjectOpen(connection.data, proj_id, proj_name);
               setProject(GetProject(proj_id, proj_name));
            }break;

            case CS_PROJECT_DATA: if(user && project)
            {
               ServerSendProjectData(connection, *project);
            }break;

            case CS_PROJECT_SETTINGS: if(user && user.canWriteCode() && project)
            {
               Project temp; if(ServerRecvProjectSettings(connection.data, temp))if(project.id==temp.id)if(project.syncSettings(temp))
               {
                  File f; ServerWriteProjectSettings(f.writeMem(), *project); Server.distribute(f, project, this);
               }
            }break;

            case CS_NEW_ELM: if(user && user.canWrite() && project)
            {
               Elm  elm; ServerRecvNewElm(connection.data, elm);
               Elm &proj_elm=project.getElm(elm.id);
               if( !proj_elm.type) // if was just added
               {
                  File f; ServerWriteNewElm(f.writeMem(), elm, project.id); Server.distribute(f, project, this);
                  Swap(proj_elm, elm);
               }
            }break;

            case CS_RENAME_ELM: if(user && user.canWrite() && project)
            {
               UID elm_id; Str name; TimeStamp name_time; ServerRecvRenameElm(connection.data, elm_id, name, name_time);
               if(Elm *elm=project.findElm(elm_id))
                  if(name_time>elm.name_time)
               {
                  elm.setName(name, name_time);
                  File f; ServerWriteRenameElm(f.writeMem(), elm_id, name, name_time, project.id); Server.distribute(f, project, this);
               }
            }break;

            case CS_SET_ELM_PARENT: if(user && user.canWrite() && project)
            {
               UID elm_id, parent_id; TimeStamp parent_time; ServerRecvSetElmParent(connection.data, elm_id, parent_id, parent_time);
               if(Elm *elm=project.findElm(elm_id))
                  if(parent_time>elm.parent_time)
               {
                  elm.setParent(parent_id, parent_time);
                  File f; ServerWriteSetElmParent(f.writeMem(), elm_id, parent_id, parent_time, project.id); Server.distribute(f, project, this);
               }
            }break;

            case CS_REMOVE_ELMS: if(user && user.canWrite() && project)
            {
               Memc<UID> elm_ids; bool removed; TimeStamp removed_time; ServerRecvRemoveElms(connection.data, elm_ids, removed, removed_time);
               Memc<UID> processed; REPA(elm_ids)if(Elm *elm=project.findElm(elm_ids[i]))if(removed_time>elm.removed_time)
               {
                  elm.setRemoved(removed, removed_time); processed.add(elm.id);
               }
               File f; ServerWriteRemoveElms(f.writeMem(), processed, removed, removed_time, project.id); Server.distribute(f, project, this);
            }break;

            case CS_NO_PUBLISH_ELMS: if(user && user.canWrite() && project)
            {
               Memc<UID> elm_ids; bool no_publish; TimeStamp no_publish_time; ServerRecvNoPublishElms(connection.data, elm_ids, no_publish, no_publish_time);
               Memc<UID> processed; REPA(elm_ids)if(Elm *elm=project.findElm(elm_ids[i]))if(no_publish_time>elm.no_publish_time)
               {
                  elm.setNoPublish(no_publish, no_publish_time); processed.add(elm.id);
               }
               File f; ServerWriteNoPublishElms(f.writeMem(), processed, no_publish, no_publish_time, project.id); Server.distribute(f, project, this);
            }break;

            case CS_GET_ELM_NAMES: if(user && project)
            {
               Memc<UID> elm_ids; ServerRecvGetElmNames(connection.data, elm_ids);
               Memc<ElmName> elm_names; FREPA(elm_ids)if(Elm *elm=project.findElm(elm_ids[i]))elm_names.New().set(*elm);
               ServerSendGetElmNames(connection, elm_names, project.id);
            }break;

            // TEXTURES
            case CS_GET_TEXTURES: if(user && project)
            {
               Memc<UID> tex_ids; ServerRecvGetTextures(connection.data, tex_ids);
               FREPA(tex_ids)send_tex.include(tex_ids[i]);
            }break;

            case CS_SET_TEXTURE: if(user && user.canWrite() && project)
            {
               UID tex_id; File cmpr_tex_data; ServerRecvSetTexture(connection.data, tex_id, cmpr_tex_data.writeMem());
               if(project.texs.binaryInclude(tex_id, Compare))
               {
                  cmpr_tex_data.pos(0); SafeOverwrite(cmpr_tex_data, project.tex_path+EncodeFileName(tex_id));
                  Server.distributeTex(tex_id, project, this);
               }
            }break;

            // ELM DATA
            case CS_GET_ELM_SHORT: if(user && project)
            {
               Memc<UID> elm_ids; ServerRecvGetElmShort(connection.data, elm_ids); FREPA(elm_ids)send_elm_short.include(elm_ids[i]);
            }break;

            case CS_GET_ELM_LONG: if(user && project)
            {
               Memc<UID> elm_ids; ServerRecvGetElmLong(connection.data, elm_ids); FREPA(elm_ids)send_elm_long.include(elm_ids[i]);
            }break;

            case CS_SET_ELM_FULL: if(user && user.canWrite() && project)
            {
               Elm elm; File data, extra; if(ServerRecvSetElmFull(connection.data, elm, data.writeMem(), extra.writeMem()))
               {
                  if(ElmManualSync(elm.type))Delete(elm.data); // client may always send the Elm.Data, however for manually synced elements, we need to discard this data, and wait for it later during manual sync
                  Elm &proj_elm=project.getElm(elm.id);
                  if( !proj_elm.type) // doesn't exist yet
                  {
                     Swap(proj_elm, elm);
                     if(ElmHasFile(proj_elm.type))
                     {
                                                        data.pos(0); SafeOverwrite( data, project.basePath(proj_elm));
                        if(ElmSendBoth(proj_elm.type)){extra.pos(0); SafeOverwrite(extra, project.gamePath(proj_elm));}
                     }
                     Server.distributeElmFull(proj_elm.id, project, this);
                  }else
                  if(elm.type==proj_elm.type)
                  {
                     if(elm.      name_time>proj_elm.      name_time)proj_elm.setName     (elm.name       , elm.      name_time);
                     if(elm.    parent_time>proj_elm.    parent_time)proj_elm.setParent   (elm.parent_id  , elm.    parent_time);
                     if(elm.   removed_time>proj_elm.   removed_time)proj_elm.setRemoved  (elm.removed  (), elm.   removed_time);
                     if(elm.no_publish_time>proj_elm.no_publish_time)proj_elm.setNoPublish(elm.noPublish(), elm.no_publish_time);
                     if(elm.data && (!proj_elm.data || proj_elm.data.ver!=elm.data.ver))
                     {
                        data.pos(0); extra.pos(0); bool elm_newer_src, src_newer_elm; if(project.syncElm(proj_elm, elm, data, extra, true, elm_newer_src, src_newer_elm))Server.distributeElmLong(proj_elm.id, project, this);
                     }
                  }
               }
            }break;

            case CS_SET_ELM_LONG: if(user && user.canWrite() && project)
            {
               Elm elm; File data, extra;
               if(ServerRecvSetElmLong(connection.data, elm, data.writeMem(), extra.writeMem()))
                  if(Elm *proj_elm=project.findElm(elm.id))
                     if(elm.type==proj_elm.type)
                        if(elm.data && (!proj_elm.data || proj_elm.data.ver!=elm.data.ver))
               {
                  data.pos(0); extra.pos(0); bool elm_newer_src, src_newer_elm; if(project.syncElm(*proj_elm, elm, data, extra, true, elm_newer_src, src_newer_elm))Server.distributeElmLong(proj_elm.id, project, this);
               }
            }break;

            case CS_SET_ELM_SHORT: if(user && user.canWrite() && project)
            {
               Elm elm; File data, extra;
               if(ServerRecvSetElmShort(connection.data, elm, data.writeMem()))
                  if(Elm *proj_elm=project.findElm(elm.id))
                     if(elm.type==proj_elm.type)
                        if(elm.data && (!proj_elm.data || proj_elm.data.ver!=elm.data.ver))
               {
                  data.pos(0); bool elm_newer_src, src_newer_elm; if(project.syncElm(*proj_elm, elm, data, extra, false, elm_newer_src, src_newer_elm))Server.distributeElmShort(proj_elm.id, project, this);
               }
            }break;
            
            // WORLD
            case CS_GET_WORLD_VER: if(user && project)
            {
               UID world_id; ServerRecvGetWorldVer(connection.data, world_id);
               ServerSendGetWorldVer(connection, project.worldVerGet(world_id), world_id, project.id);
            }break;

            // AREA
            case CS_GET_WORLD_AREAS: if(user && project)
            {
               UID world_id; Memc<AreaSync> areas; ServerRecvGetWorldAreas(connection.data, world_id, areas);
               if(WorldSync *ws=world_sync.get(world_id))FREPA(areas)if(byte *area_sync_flag=ws.areas.get(areas[i].xy))*area_sync_flag|=areas[i].flag;
            }break;

            case CS_SET_WORLD_AREA: if(user && user.canWrite() && project)
            {
               UID world_id; VecI2 area_xy; byte area_sync_flag; AreaVer ver; Heightmap hm; Memc<ObjData> objs;
               if(ServerRecvSetWorldArea(connection.data, world_id, area_xy, area_sync_flag, ver, hm, project.game_path, project.edit_path))
                  if(uint flag=project.syncArea(world_id, area_xy, area_sync_flag, ver, hm, objs))
                     Server.distributeWorldArea(world_id, area_xy, flag, project, this);
            }break;

            // OBJS
            case CS_SET_WORLD_OBJS: if(user && user.canWrite() && project)
            {
               UID world_id; VecI2 area_xy; Memc<ObjData> objs; Map<VecI2, Memc<ObjData>> obj_modified(Compare);
               if(ServerRecvSetWorldObjs(connection.data, world_id, area_xy, objs, project.edit_path))
               {
                  project.syncObj(world_id, area_xy, objs, &obj_modified);
                  FREPA(obj_modified)
                  {
                     File f; ServerWriteSetWorldObjs(f.writeMem(), world_id, obj_modified.key(i), obj_modified[i], project.edit_path, project.id);
                     Server.distribute(f, project, this);
                  }
               }
            }break;

            // WAYPOINT
            case CS_GET_WORLD_WAYPOINTS: if(user && project)
            {
               UID world_id; Memc<UID> waypoints; ServerRecvGetWorldWaypoints(connection.data, world_id, waypoints);
               Str edit, game;
               if(project.getWorldPaths(world_id, edit, game))
               if(WorldVer *world_ver=project.worldVerGet(world_id))
               {
                  edit+="Waypoint\\";
                  FREPA(waypoints)
                  {
                          C UID &waypoint_id =waypoints[i];
                     if(Version *waypoint_ver=world_ver.waypoints.find(waypoint_id))
                     {
                        EditWaypoint waypoint; waypoint.load(edit+EncodeFileName(waypoint_id));
                        File f; ServerWriteSetWorldWaypoint(f.writeMem(), world_id, waypoint_id, *waypoint_ver, waypoint, project.id); f.pos(0); connection.send(f, -1, false);
                     }
                  }
               }
            }break;

            case CS_SET_WORLD_WAYPOINT: if(user && user.canWrite() && project)
            {
               UID world_id, waypoint_id; Version waypoint_ver; EditWaypoint waypoint;
               if(ServerRecvSetWorldWaypoint(connection.data, world_id, waypoint_id, waypoint_ver, waypoint))
                  if(project.syncWaypoint(world_id, waypoint_id, waypoint_ver, waypoint))
               {
                  File f; ServerWriteSetWorldWaypoint(f.writeMem(), world_id, waypoint_id, waypoint_ver, waypoint, project.id);
                  Server.distribute(f, project, this);
               }
            }break;

            // LAKE
            case CS_GET_WORLD_LAKES: if(user && project)
            {
               UID world_id; Memc<UID> lakes; ServerRecvGetWorldLakes(connection.data, world_id, lakes);
               Str edit, game;
               if(project.getWorldPaths(world_id, edit, game))
               if(WorldVer *world_ver=project.worldVerGet(world_id))
               {
                  edit+="Lake\\";
                  FREPA(lakes)
                  {
                           C UID &lake_id =lakes[i];
                     if(WaterVer *lake_ver=world_ver.lakes.find(lake_id))
                     {
                        Lake lake; lake.load(edit+EncodeFileName(lake_id));
                        File f; ServerWriteSetWorldLake(f.writeMem(), world_id, lake_id, lake_ver.ver, lake, project.id); f.pos(0); connection.send(f, -1, false);
                     }
                  }
               }
            }break;

            case CS_SET_WORLD_LAKE: if(user && user.canWrite() && project)
            {
               UID world_id, lake_id; Version lake_ver; Lake lake;
               if(ServerRecvSetWorldLake(connection.data, world_id, lake_id, lake_ver, lake))
                  if(project.syncLake(world_id, lake_id, lake_ver, lake))
               {
                  File f; ServerWriteSetWorldLake(f.writeMem(), world_id, lake_id, lake_ver, lake, project.id);
                  Server.distribute(f, project, this);
               }
            }break;

            // RIVER
            case CS_GET_WORLD_RIVERS: if(user && project)
            {
               UID world_id; Memc<UID> rivers; ServerRecvGetWorldRivers(connection.data, world_id, rivers);
               Str edit, game;
               if(project.getWorldPaths(world_id, edit, game))
               if(WorldVer *world_ver=project.worldVerGet(world_id))
               {
                  edit+="River\\";
                  FREPA(rivers)
                  {
                           C UID &river_id =rivers[i];
                     if(WaterVer *river_ver=world_ver.rivers.find(river_id))
                     {
                        River river; river.load(edit+EncodeFileName(river_id));
                        File f; ServerWriteSetWorldRiver(f.writeMem(), world_id, river_id, river_ver.ver, river, project.id); f.pos(0); connection.send(f, -1, false);
                     }
                  }
               }
            }break;

            case CS_SET_WORLD_RIVER: if(user && user.canWrite() && project)
            {
               UID world_id, river_id; Version river_ver; River river;
               if(ServerRecvSetWorldRiver(connection.data, world_id, river_id, river_ver, river))
                  if(project.syncRiver(world_id, river_id, river_ver, river))
               {
                  File f; ServerWriteSetWorldRiver(f.writeMem(), world_id, river_id, river_ver, river, project.id);
                  Server.distribute(f, project, this);
               }
            }break;

            // MINI MAP
            case CS_GET_MINI_MAP_VER: if(user && project)
            {
               UID mini_map_id; ServerRecvGetMiniMapVer(connection.data, mini_map_id);
               ServerSendGetMiniMapVer(connection, project.miniMapVerGet(mini_map_id), mini_map_id, project.id);
            }break;

            case CS_GET_MINI_MAP_IMAGES: if(user && project)
            {
               UID mini_map_id; Memc<VecI2> images; ServerRecvGetMiniMapImages(connection.data, mini_map_id, images);
               if(MiniMapSync *mms=mini_map_sync.get(mini_map_id))FREPA(images)mms.images.binaryInclude(images[i], Compare);
            }break;

            case CS_SET_MINI_MAP_SETTINGS: if(user && user.canWrite() && project)
            {
               UID mini_map_id; Game.MiniMap.Settings settings; TimeStamp settings_time;
               if(ServerRecvSetMiniMapSettings(connection.data, mini_map_id, settings, settings_time))
                  if(project.syncMiniMapSettings(mini_map_id, settings, settings_time))
               {
                  File f; ServerWriteSetMiniMapSettings(f.writeMem(), mini_map_id, settings, settings_time, project.id);
                  Server.distribute(f, project, this);
               }
            }break;

            case CS_SET_MINI_MAP_IMAGE: if(user && user.canWrite() && project)
            {
               UID mini_map_id; VecI2 image_xy; TimeStamp image_time; bool image_is; File cmpr_image_data;
               if(ServerRecvSetMiniMapImage(connection.data, mini_map_id, image_xy, image_time, image_is, cmpr_image_data.writeMem()))
               {
                  if(!image_is)cmpr_image_data.del();
                  if(project.syncMiniMapImage(mini_map_id, image_xy, image_time, cmpr_image_data))
                     Server.distributeMiniMapImage(mini_map_id, image_xy, project, this);
               }
            }break;

            // CODE
            case CS_GET_CODE_VER: if(user && project)
            {
               Memc<ElmTypeVer> elm_type_vers; if(ServerRecvGetCodeVer(connection.data, elm_type_vers))
               {
                  Memc<ElmCodeData> elm_code_datas;
                  FREPA(project.elms)
                  {
                     Elm &server_elm=project.elms[i];
                     if(server_elm.type==ELM_CODE || server_elm.type==ELM_APP)
                     {
                        if(ElmTypeVer *client_elm=elm_type_vers.binaryFind(server_elm.id, ElmTypeVer.Compare))
                           if(server_elm.type==client_elm.type && (server_elm.data ? server_elm.data.ver : Version())==client_elm.ver)continue; // elements are the same so we can skip them
                        // add element from the server
                        ElmCodeData &ecd=elm_code_datas.New(); ecd.set(server_elm);
                        if(ecd.type==ELM_CODE)LoadCode(ecd.code.current, project.codePath(server_elm.id));
                     }
                  }
                  ServerSendSetCodeData(connection, elm_code_datas, project.id);
               }
            }break;

            case CS_SET_CODE_DATA: if(user && user.canWrite() && project)
            {
               Memc<ElmCodeData> elm_code_datas; if(ServerRecvSetCodeData(connection.data, elm_code_datas))
               {
                  bool resync=false;
                  if(RequireAllCodeMatchForSync && user.canWriteCode())REPA(elm_code_datas) // check if all codes can be updated (do this only if we're actually syncing codes, and if EE is configured to require this)
                  {
                     ElmCodeData &ecd=elm_code_datas[i];
                     if(ecd.type==ELM_CODE)if(Elm *elm=project.findElm(ecd.id))if(elm.type==ecd.type)if(ElmCode *code_data=elm.codeData()) // check only codes
                        if(code_data.ver) // continue checking only if the server has valid data
                     {
                        Str code; LoadCode(code, project.codePath(elm.id));
                        if(!Equal(code, ecd.code.base, true)){resync=true; break;} // if at least one code can't be updated, then mark that resync is needed, and break loop
                     }
                  }
                  Memc<ElmCodeBase> succeeded;
                  if(!resync) // if we can proceed with updating (all codes are ok)
                     REPA(elm_code_datas)
                  {
                     ElmCodeData &ecd=elm_code_datas[i];
                     if(Elm *elm=project.findElm(ecd.id))if(elm.type==ecd.type)switch(elm.type)
                     {
                        case ELM_APP : elm.appData().sync(ecd.app, user.canWriteCode()); break;
                        case ELM_CODE: if(user.canWriteCode())if(ElmCode *code_data=elm.codeData())
                        {
                           Str path=project.codePath(elm.id);
                           Str code; LoadCode(code, path);
                           if(code_data.ver // perform checking only if the server has valid data (this will allow to overwrite server codes when client base code is different but the server doesn't have yet any data, this will help in case of using multiple servers)
                           && !Equal(code, ecd.code.base, true))resync=true;else // if server code is different than client base code then it means that resync is required (someone modified code on the server between client code sync)
                           if(SaveCode(ecd.code.current, path)) // set server code from client current code
                           {
                              code_data.ver=ecd.ver; // set server version from client version
                              succeeded.New().set(elm.id, ecd.code.current);
                           }
                        }break;
                     }
                  }
                  if(user.canWriteCode())ServerSendCodeSyncStatus(connection, succeeded, resync, project.id);
               }
            }break;
         }
      }

      // send data
      if(connection_version_ok && smallBuf())
      {
         // TODO: loop with timeout ?

         // send elm short
         if(send_elm_short.elms() && smallBuf())
         {
            UID elm_id=send_elm_short.first(); send_elm_short.remove(0, true);
            if(Elm *elm=project.findElm(elm_id))
            {
               File f; ServerWriteSetElmShort(f.writeMem(), *elm, *project); f.pos(0); connection.send(f, -1, false);
            }
         }

         // send elm long
         if(send_elm_long.elms() && smallBuf())
         {
            UID elm_id=send_elm_long.first(); send_elm_long.remove(0, true);
            if(Elm *elm=project.findElm(elm_id))
            {
               File f; ServerWriteSetElmLong(f.writeMem(), *elm, *project); f.pos(0); connection.send(f, -1, false);
            }
         }

         // send elm full
         if(send_elm_full.elms() && smallBuf())
         {
            UID elm_id=send_elm_full.first(); send_elm_full.remove(0, true);
            if(Elm *elm=project.findElm(elm_id))
            {
               File f; ServerWriteSetElmFull(f.writeMem(), *elm, *project); f.pos(0); connection.send(f, -1, false);
            }
         }

         // send textures
         if(send_tex.elms() && smallBuf())
         {
            UID tex_id=send_tex.first(); send_tex.remove(0, true);
            if( project.texs.binaryHas(tex_id, Compare))
            {
               File data; data.readTry(project.tex_path+EncodeFileName(tex_id)); data.pos(0);
               File f; ServerWriteSetTexture(f.writeMem(), tex_id, data, project.id); f.pos(0); connection.send(f, -1, false);
            }
         }

         // send world areas
         if(world_sync.elms() && smallBuf())
         {
            UID  world_id=world_sync.key(0);
            WorldSync &ws=world_sync    [0];
            if(ws.areas.elms())
            {
               VecI2 area_xy       =ws.areas.key(ws.areas.elms()-1);
               byte  area_sync_flag=ws.areas    [ws.areas.elms()-1]; ws.areas.remove(ws.areas.elms()-1);
               if(WorldVer *world_ver=project.worldVerGet(world_id))
                  if(AreaVer *area_ver=world_ver.areas.get(area_xy))
               {
                  Heightmap temp, *hm=null;
                  Memc<ObjData> objs;
                  if(area_sync_flag&(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR|AREA_SYNC_OBJ))hm=project.hmObjGet(world_id, area_xy, temp, objs, FlagTest(area_sync_flag, AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR), FlagTest(area_sync_flag, AREA_SYNC_OBJ));
                  ServerSendSetWorldArea(connection, world_id, area_xy, area_sync_flag, *area_ver, hm, objs, project.id, project.edit_path);
               }
            }
            if(!ws.areas.elms())world_sync.remove(0);
         }

         // send mini map images
         if(mini_map_sync.elms() && smallBuf())
         {
            UID  mini_map_id=mini_map_sync.key(0);
            MiniMapSync &mms=mini_map_sync    [0];
            if(mms.images.elms())
            {
               VecI2 image_xy=mms.images.last(); mms.images.removeLast();
               if(MiniMapVer *mini_map_ver=project.miniMapVerGet(mini_map_id))
               {
                  File f; f.readTry(project.gamePath(mini_map_id).tailSlash(true)+image_xy);
                  ServerSendSetMiniMapImage(connection, mini_map_id, image_xy, mini_map_ver.time, f, project.id);
               }
            }
            if(!mms.images.elms())mini_map_sync.remove(0);
         }
      }

      return true;
   }
}
/******************************************************************************/
