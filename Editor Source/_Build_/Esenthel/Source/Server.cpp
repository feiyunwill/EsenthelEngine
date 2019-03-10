/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ServerClass Server;
/******************************************************************************/

/******************************************************************************/
   bool ServerClass::loggedIn()C {return logged_in;}
   bool ServerClass::canRead()C {return loggedIn() && proj_opened;}
   bool ServerClass::canWrite()C {return loggedIn() && proj_opened && CanWrite    (access);}
   bool ServerClass::canWriteCode()C {return loggedIn() && proj_opened && CanWriteCode(access);}
   bool ServerClass::smallBuf()C {return queued()<=ServerSendBufSize;}
   long ServerClass::sentTotal()C {return sent+::EE::Connection::sent    ();}
   long ServerClass::rcvdTotal()C {return rcvd+::EE::Connection::received();}
   void ServerClass::clearProj()
   {
      texs.clear();
         world_vers.del();
      mini_map_vers.del();
      Synchronizer.clearSync();
   }
   void ServerClass::logout() {stopConnect();}
   void ServerClass::stopConnect()
   {
      Misc.online.set("Offline");
      ::EE::Connection::del();
      logged_email.del();
      if(projects.elms())
      {
         projects.del();
         if(StateActive==&StateProjectList)Projs.refresh();
      }
      after_connect=version_sent=version_ok=logged_in=reconnect=allow_reconnect=proj_opened=send_proj_settings=false; access=UA_NO_ACCESS; action=NONE;
      clearProj();
   }
   void ServerClass::startConnect(ACTION action)
   {
      T.action=action;
      sent+=::EE::Connection::sent    ();
      rcvd+=::EE::Connection::received();
      clientConnectToServer(conn_addr);
      after_connect=true;
   }
   void ServerClass::connect(SockAddr &addr, C Str &email, C Str &pass, bool allow_reconnect)
   {
      stopConnect();
      Misc.online.set("Connecting");
      conn_addr =addr;
      conn_email=email;
      conn_pass =pass;
      T.allow_reconnect=allow_reconnect;
      startConnect(LOGIN);
   }
   void ServerClass::Register(Str name)
   {
      if(conn_addr.valid() && ValidEmail(conn_email) && ValidPass(conn_pass))
      {
         stopConnect();
         conn_name=name;
         startConnect(REGISTER);
      }
   }
   void ServerClass::forgotPass(SockAddr &addr, C Str &email)
   {
      stopConnect();
      conn_addr =addr;
      conn_email=email;
      startConnect(FORGOT_PASS);
   }
   void ServerClass::changePass(SockAddr &addr, C Str &email, C Str &new_pass, uint change_pass_key)
   {
      stopConnect();
      conn_addr    =addr;
      conn_email   =email;
      conn_pass    =new_pass;
      conn_pass_key=change_pass_key;
      startConnect(CHANGE_PASS);
   }
   void ServerClass::licenseKey(C Str &license_key              ) {if(loggedIn()) ClientSendLicenseKey    (T, license_key);}
   void ServerClass::projectOpen(C UID &proj_id, C Str &proj_name) {if(loggedIn()){ClientSendProjectOpen   (T, proj_id, proj_name); proj_opened=proj_id.valid();}}
   void ServerClass::projectDataRequest(                                ) {if(canRead ()) ClientSendGetProjectData(T);}
   void ServerClass::projectSetSettings(                                ) {if(canWrite())send_proj_settings=true;}
   void ServerClass::syncCodes(  Memc<ElmTypeVer > &elms       ) {if(canRead ())ClientSendGetCodeVer (T, elms);}
   void ServerClass::syncCodes(C Memc<ElmCodeData> &elms       ) {if(canWrite())ClientSendSetCodeData(T, elms);}
   void    ServerClass::newElm(     Elm  &elm                                     ) {if(canWrite())ClientSendNewElm       (T, elm );}
   void ServerClass::renameElm(     Elm  &elm                                     ) {if(canWrite())ClientSendRenameElm    (T, elm.id, elm.name     , elm.  name_time);}
   void ServerClass::setElmParent(     Elm  &elm                                     ) {if(canWrite())ClientSendSetElmParent (T, elm.id, elm.parent_id, elm.parent_time);}
   void ServerClass::removeElms(Memc<UID> &elms, bool removed   , C TimeStamp &time) {if(canWrite())ClientSendRemoveElms   (T, elms  , removed   , time);}
   void ServerClass::noPublishElms(Memc<UID> &elms, bool no_publish, C TimeStamp &time) {if(canWrite())ClientSendNoPublishElms(T, elms  , no_publish, time);}
   void ServerClass::getElmNames(Memc<UID> &elms                                    ) {if(canRead ())ClientSendGetElmNames  (T, elms);}
   void ServerClass::getTextures(Memc<UID> &texs                                    ) {if(canRead ())ClientSendGetTextures  (T, texs);}
   void ServerClass::getElmShort(Memc<UID> &elms                                    ) {if(canRead ())ClientSendGetElmShort  (T, elms);}
   void ServerClass::getElmLong(Memc<UID> &elms                                    ) {if(canRead ())ClientSendGetElmLong   (T, elms);}
   void ServerClass::getElmLong(   C UID  &elm_id                                  ) {Memc<UID> elms; elms.add(elm_id); getElmLong(elms);}
   void ServerClass::setTex(C UID &tex_id) {if(canWrite() && tex_id.valid() && texs.binaryInclude(tex_id, Compare))Synchronizer.setTex(tex_id);}
   void ServerClass::setElmShort(C UID &elm_id) {if(canWrite())Synchronizer.setElmShort(elm_id);}
   void ServerClass::setElmLong(C UID &elm_id) {if(canWrite())Synchronizer.setElmLong (elm_id);}
   void ServerClass::setElmFull(C UID &elm_id) {if(canWrite())Synchronizer.setElmFull (elm_id);}
   void ServerClass::setWaypoint(C UID &world_id, C UID &waypoint_id, C Version &waypoint_ver, EditWaypoint &waypoint) {if(canWrite())ClientSendSetWorldWaypoint(T, world_id, waypoint_id, waypoint_ver, waypoint);}
   void ServerClass::setLake(C UID &world_id, C UID &    lake_id, C Version &    lake_ver, Lake         &lake    ) {if(canWrite())ClientSendSetWorldLake    (T, world_id,     lake_id,     lake_ver, lake    );}
   void ServerClass::setRiver(C UID &world_id, C UID &   river_id, C Version &   river_ver, River        &river   ) {if(canWrite())ClientSendSetWorldRiver   (T, world_id,    river_id,    river_ver, river   );}
   void ServerClass::getWorldVer(C UID &world_id                           ) {if(canRead() && world_id.valid() && !world_vers.find(world_id))ClientSendGetWorldVer      (T, world_id           );}
   void ServerClass::getWorldAreas(C UID &world_id, Memc<AreaSync> &areas    ) {if(canRead() && world_id.valid()                              )ClientSendGetWorldAreas    (T, world_id, areas    );}
   void ServerClass::getWorldWaypoints(C UID &world_id, Memc<UID     > &waypoints) {if(canRead() && world_id.valid()                              )ClientSendGetWorldWaypoints(T, world_id, waypoints);}
   void ServerClass::getWorldLakes(C UID &world_id, Memc<UID     > &lakes    ) {if(canRead() && world_id.valid()                              )ClientSendGetWorldLakes    (T, world_id, lakes    );}
   void ServerClass::getWorldRivers(C UID &world_id, Memc<UID     > &rivers   ) {if(canRead() && world_id.valid()                              )ClientSendGetWorldRivers   (T, world_id, rivers   );}
   void ServerClass::getMiniMapVer(C UID &mini_map_id                     ) {if(canRead() && mini_map_id.valid() && !mini_map_vers.find(mini_map_id))ClientSendGetMiniMapVer   (T, mini_map_id        );}
   void ServerClass::getMiniMapImages(C UID &mini_map_id, Memc<VecI2> &images) {if(canRead() && mini_map_id.valid()                                    )ClientSendGetMiniMapImages(T, mini_map_id, images);}
   void ServerClass::setMiniMapSettings(C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time) {if(canWrite() && mini_map_id.valid())ClientSendSetMiniMapSettings(T, mini_map_id, settings, settings_time);}
   void ServerClass::update(ProjectEx *proj, bool busy)
   {
      if(reconnect && Time.realTime()>=reconnect_time)connect(conn_addr, conn_email, conn_pass, true);
      if(after_connect)
      {
         // check server state
         switch(state())
         {
            case CONNECT_INVALID         : if(allow_reconnect || loggedIn()){stopConnect(); Misc.online.set("Connecting"); T.reconnect=true; reconnect_time=Time.realTime()+6;}else{if(!version_ok)Gui.msgBox(S, "Can't connect to server.\nYou have entered invalid server address,\nor the server is unavailable,\nor you're not connected to the Internet."); stopConnect();} break; // if 'version_ok' then it means we indeed got connected ok, but got disconnected to some other failure (like wrong password)
            case CONNECT_CONNECTING      :
            case CONNECT_AWAIT_GREET     : if(Server.life()>6000){Gui.msgBox(S, "Server Connection Timeout"); stopConnect();} break;
            case CONNECT_VERSION_CONFLICT: Gui.msgBox(S, "This application is of different version than the server application.\nPlease update both applications."); stopConnect(); break;
            case CONNECT_GREETED         : if(!version_sent){version_sent=true; ClientSendVersion(T);} break;
         }

         // check if we have scheduled data to send
         if(version_ok)switch(action)
         {
            case LOGIN      : action=NONE; ClientSendLogin     (T, conn_email, conn_pass, ServerLicenseKey, OSVer()); break;
            case REGISTER   : action=NONE; ClientSendRegister  (T, conn_email, conn_pass, conn_name                ); break;
            case FORGOT_PASS: action=NONE; ClientSendForgotPass(T, conn_email                                      ); break;
            case CHANGE_PASS: action=NONE; ClientSendChangePass(T, conn_email, conn_pass, S, conn_pass_key         ); break;
         }

         // check if any data received from server
         const uint time=Time.curTimeMs(), delay=16; // 1000ms/60fps
         for(; receive(0); )
         {
            switch(data.getByte())
            {
               case CS_VERSION_CHECK:
               {
                  if(ClientRecvVersion(data))version_ok=true;else{Gui.msgBox(S, "This application is of different version than the server application.\nPlease update both applications."); stopConnect();}
               }break;

               case CS_LOGIN:
               {
                  LOGIN_RESULT result; USER_ACCESS access; ClientRecvLogin(data, result, access);
                  switch(result)
                  {
                     case LOGIN_EMAIL_FOUND_INVALID_PASS                : Gui.msgBox(S, "Invalid password"); break;
                     case LOGIN_EMAIL_NOT_FOUND_REGISTRATION_UNAVAILABLE: Gui.msgBox(S, "E-mail was not found.\nUser registration is disabled on this server."); break;
                     case LOGIN_EMAIL_NOT_FOUND_REGISTRATION_AVAILABLE  : ::Register.activate(); break;
                     case LOGIN_LICENSE_USED                            : Gui.msgBox(S, "Server already has a different user using your License Key."); stopConnect(); break;
                     case LOGIN_DEMO_USER_NOT_ALLOWED                   : Gui.msgBox(S, "Can't connect to server using demo version when the server has 2 or more clients."); stopConnect(); break;
                     case LOGIN_SUCCESS:
                     {
                        if(access==UA_NO_ACCESS || !InRange(access, UA_NUM)){Gui.msgBox(S, "Your account doesn't have access to the server.\nPlease contact server administrator to enable access."); stopConnect();}else
                        {
                           logged_email=conn_email; logged_in=true; T.access=access; ClientSendProjectsListRequest(T);
                           if(access==UA_READ_ONLY)Gui.msgBox(S, "Your account is limited to read-only operations.");
                           Misc.online.set((access==UA_READ_ONLY) ? "Read Only" : "Online");
                           if(proj)proj->resumeServer(); // needed if reconnected after lost connection
                        }
                     }break;
                  }
               }break;

               case CS_REGISTER:
               {
                  REGISTER_RESULT result; ClientRecvRegister(data, result);
                  switch(result)
                  {
                     case REGISTER_REGISTRATION_UNAVAILABLE: Gui.msgBox(S, "User registration is disabled on this server."); break;
                     case REGISTER_INVALID_EMAIL           : Gui.msgBox(S, "Invalid e-mail for user registration."); break;
                     case REGISTER_EMAIL_USED              : Gui.msgBox(S, "E-mail entered for registration is already used."); break;
                     case REGISTER_SUCCESS                 : Gui.msgBox(S, "Registration success.\nPlease contact the server administrator to activate your account."); break;
                  }
               }break;

               case CS_FORGOT_PASS:
               {
                  FORGOT_PASS_RESULT result; ClientRecvForgotPass(data, result);
                  switch(result)
                  {
                     case FORGOT_PASS_EMAIL_NOT_FOUND: Gui.msgBox(S, "Invalid e-mail used in forgot password."); break;
                     case FORGOT_PASS_KEY_SENT       : ChangePass.activate(true); break;
                     case FORGOT_PASS_NO_SEND_MAIL   : Gui.msgBox(S, "Server doesn't support sending e-mails (it doesn't have SMTP installed).\nPlease contact the server administrator to change the password for you."); break;
                  }
               }break;

               case CS_CHANGE_PASS:
               {
                  CHANGE_PASS_RESULT result; ClientRecvChangePass(data, result);
                  switch(result)
                  {
                     case CHANGE_PASS_INVALID_KEY    : Gui.msgBox(S, "Invalid key."); ChangePass.hide(); break;
                     case CHANGE_PASS_INVALID_PASS   : Gui.msgBox(S, "Invalid current password."); break;
                     case CHANGE_PASS_EMAIL_NOT_FOUND: Gui.msgBox(S, "Invalid e-mail for change password."); ChangePass.hide(); break;
                     case CHANGE_PASS_SUCCESS        : Gui.msgBox(S, "Password changed."); ChangePass.hide(); break;
                  }
               }break;

               case CS_PROJECTS_LIST: ClientRecvProjectsList(data, projects); Projs.refresh(); break;

               case CS_PROJECT_DATA: if(proj)
               {
                  Project temp; if(ClientRecvProjectData(data, temp))if(temp.id==proj->id)Synchronizer.sync(*proj, temp);
               }break;

               case CS_PROJECT_SETTINGS: if(proj)
               {
                  Project temp; if(ClientRecvProjectSettings(data, temp))if(temp.id==proj->id)
                  {
                     if(proj-> syncSettings(temp)){ProjSettings.toGui(); CodeEdit.makeAuto();} // set auto header in case Data Encryption options have changed
                     if(proj->newerSettings(temp))projectSetSettings(); // if after receiving settings from server we have newer values from it, then send it to the server
                  }
               }break;

               case CS_NEW_ELM: if(proj)
               {
                  Elm elm; UID proj_id; ClientRecvNewElm(data, elm, proj_id);
                  if(proj_id==proj->id && proj->testElmsNum())
                  {
                     proj->setListCurSel();
                     Elm &proj_elm=proj->getElm(elm.id); if(!proj_elm.type)Swap(elm, proj_elm); // if just added then swap, otherwise we can't create a new element if it already exists so do nothing
                     proj->setList();
                  }
               }break;

               case CS_RENAME_ELM: if(proj)
               {
                  UID elm_id, proj_id; Str name; TimeStamp name_time; ClientRecvRenameElm(data, elm_id, name, name_time, proj_id);
                  if(proj_id==proj->id)
                     if(Elm *elm=proj->findElm(elm_id))
                        if(name_time>elm->name_time)
                  {
                     proj->setElmName(*elm, name, name_time);
                     proj->refresh(false, false);
                  }
               }break;

               case CS_SET_ELM_PARENT: if(proj)
               {
                  UID elm_id, parent_id, proj_id; TimeStamp parent_time; ClientRecvSetElmParent(data, elm_id, parent_id, parent_time, proj_id);
                  if(proj_id==proj->id)
                     if(Elm *elm=proj->findElm(elm_id))
                        if(parent_time>elm->parent_time)
                  {
                     proj->setListCurSel(); elm->setParent(parent_id, parent_time);
                     proj->setList();
                     proj->activateSources(); // rebuild sources if needed
                  }
               }break;

               case CS_REMOVE_ELMS: if(proj)
               {
                  Memc<UID> elm_ids; bool removed; TimeStamp removed_time; UID proj_id; ClientRecvRemoveElms(data, elm_ids, removed, removed_time, proj_id);
                  if(proj_id==proj->id)
                  {
                     proj->setListCurSel();
                     FREPA(elm_ids)if(Elm *elm=proj->findElm(elm_ids[i]))if(removed_time>elm->removed_time)elm->setRemoved(removed, removed_time);
                     proj->setList(false);
                     proj->activateSources(); // rebuild sources if needed
                     WorldEdit.delayedValidateRefs();
                  }
               }break;

               case CS_NO_PUBLISH_ELMS: if(proj)
               {
                  Memc<UID> elm_ids; bool no_publish; TimeStamp no_publish_time; UID proj_id; ClientRecvNoPublishElms(data, elm_ids, no_publish, no_publish_time, proj_id);
                  if(proj_id==proj->id)
                  {
                     proj->setListCurSel();
                     FREPA(elm_ids)if(Elm *elm=proj->findElm(elm_ids[i]))if(no_publish_time>elm->no_publish_time)elm->setNoPublish(no_publish, no_publish_time);
                     proj->setList(false);
                     proj->activateSources(); // rebuild sources if needed
                     WorldEdit.delayedValidateRefs();
                  }
               }break;

               case CS_GET_ELM_NAMES: if(proj)
               {
                  Memc<ElmName> elm_names; UID proj_id; ClientRecvGetElmNames(data, elm_names, proj_id);
                  if(proj_id==proj->id)
                  {
                     FREPA(elm_names)
                     {
                        ElmName &elm_name=elm_names[i];
                        if(Elm *elm=proj->findElm(elm_name.id))if(elm_name.time>elm->name_time)proj->setElmName(*elm, elm_name.name, elm_name.time);
                     }
                     proj->refresh(false, false);
                  }
               }break;

               case CS_SET_TEXTURE: if(proj)
               {
                  UID tex_id, proj_id; File tex_data;
                  if(ClientRecvSetTexture(data, tex_id, tex_data.writeMem(), proj_id))
                     if(proj_id==proj->id)
                        if(proj->includeTex(tex_id))
                  {
                     Str path=proj->texPath(tex_id);
                     tex_data.pos(0); if(SafeOverwrite(tex_data, path)){SavedImage(path); proj->savedTex(tex_id, tex_data.size());}
                  }
               }break;

               case CS_SET_ELM_FULL: if(proj)
               {
                  Elm elm; UID proj_id; File elm_data, elm_extra;
                  if(ClientRecvSetElmFull(data, elm, elm_data.writeMem(), elm_extra.writeMem(), proj_id))
                     if(proj_id==proj->id && proj->testElmsNum())
                  {
                     bool created=false;
                     proj->setListCurSel();
                     Elm &proj_elm=proj->getElm(elm.id);
                     if( !proj_elm.type) // doesn't exist yet
                     {
                        elm.opened(false); // keep received elements as closed
                        Swap(proj_elm, elm);
                        elm_data.pos(0); elm_extra.pos(0); proj->receivedData(proj_elm, elm_data, elm_extra);
                        created=true;
                     }else
                     if(elm.type==proj_elm.type)
                     {
                        if(elm.      name_time>proj_elm.      name_time)proj->setElmName(proj_elm, elm.name       , elm.      name_time);
                        if(elm.    parent_time>proj_elm.    parent_time)proj_elm.setParent   (    elm.parent_id  , elm.    parent_time);
                        if(elm.   removed_time>proj_elm.   removed_time)proj_elm.setRemoved  (    elm.removed  (), elm.   removed_time);
                        if(elm.no_publish_time>proj_elm.no_publish_time)proj_elm.setNoPublish(    elm.noPublish(), elm.no_publish_time);
                        if(elm.data && (!proj_elm.data || elm.data->ver!=proj_elm.data->ver))
                        {
                           elm_data.pos(0); elm_extra.pos(0); proj->syncElm(proj_elm, elm, elm_data, elm_extra, true);
                        }
                     }
                     proj->setList();
                     if(created)WorldEdit.delayedValidateRefs();
                  }
               }break;

               case CS_SET_ELM_LONG: if(proj)
               {
                  Elm elm; UID proj_id; File elm_data, elm_extra;
                  if(ClientRecvSetElmLong(data, elm, elm_data.writeMem(), elm_extra.writeMem(), proj_id))
                     if(proj_id==proj->id)
                        if(Elm *proj_elm=proj->findElm(elm.id, elm.type))
                           if(elm.data && (!proj_elm->data || elm.data->ver!=proj_elm->data->ver))
                  {
                     elm_data.pos(0); elm_extra.pos(0); proj->syncElm(*proj_elm, elm, elm_data, elm_extra, true);
                  }
               }break;

               case CS_SET_ELM_SHORT: if(proj)
               {
                  Elm elm; UID proj_id; File elm_data, elm_extra;
                  if(ClientRecvSetElmShort(data, elm, elm_data.writeMem(), proj_id))
                     if(proj_id==proj->id)
                        if(Elm *proj_elm=proj->findElm(elm.id, elm.type))
                           if(elm.data && (!proj_elm->data || elm.data->ver!=proj_elm->data->ver))
                  {
                     elm_data.pos(0); proj->syncElm(*proj_elm, elm, elm_data, elm_extra, false);
                  }
               }break;

               case CS_GET_WORLD_VER: if(proj)
               {
                  UID world_id, proj_id; WorldVer world_ver;
                  ClientRecvGetWorldVer(data, world_ver, world_id, proj_id);
                  if(proj_id==proj->id)
                     if(WorldVer *server_world_ver=world_vers.get(world_id))
                  {
                     Swap(*server_world_ver, world_ver);
                     Synchronizer.syncWorld(world_id);
                  }
               }break;

               case CS_SET_WORLD_AREA: if(proj)
               {
                  UID world_id, proj_id; VecI2 area_xy; byte area_sync_flag; AreaVer ver; Heightmap hm; Memc<ObjData> objs; Memc<UID> local_objs_newer;
                  if(ClientRecvSetWorldArea(data, world_id, area_xy, area_sync_flag, ver, hm, objs, proj_id, proj->game_path, proj->edit_path))
                     if(proj_id==proj->id)
                  {
                     proj->syncArea(world_id, area_xy, area_sync_flag, ver, hm, objs, &local_objs_newer);
                     Synchronizer.setObjs(world_id, local_objs_newer);
                  }
               }break;

               case CS_SET_WORLD_OBJS: if(proj)
               {
                  UID world_id, proj_id; VecI2 area_xy; Memc<ObjData> objs;
                  if(ClientRecvSetWorldObjs(data, world_id, area_xy, objs, proj->edit_path, proj_id))
                     if(proj_id==proj->id)
                        proj->syncObj(world_id, area_xy, objs);
               }break;

               case CS_SET_WORLD_WAYPOINT: if(proj)
               {
                  UID world_id, waypoint_id, proj_id; Version waypoint_ver; EditWaypoint waypoint;
                  if(ClientRecvSetWorldWaypoint(data, world_id, waypoint_id, waypoint_ver, waypoint, proj_id))
                     if(proj_id==proj->id)
                        proj->syncWaypoint(world_id, waypoint_id, waypoint_ver, waypoint);
               }break;

               case CS_SET_WORLD_LAKE: if(proj)
               {
                  UID world_id, lake_id, proj_id; Version lake_ver; Lake lake;
                  if(ClientRecvSetWorldLake(data, world_id, lake_id, lake_ver, lake, proj_id))
                     if(proj_id==proj->id)
                        proj->syncLake(world_id, lake_id, lake_ver, lake);
               }break;

               case CS_SET_WORLD_RIVER: if(proj)
               {
                  UID world_id, river_id, proj_id; Version river_ver; River river;
                  if(ClientRecvSetWorldRiver(data, world_id, river_id, river_ver, river, proj_id))
                     if(proj_id==proj->id)
                        proj->syncRiver(world_id, river_id, river_ver, river);
               }break;

               case CS_GET_MINI_MAP_VER: if(proj)
               {
                  UID mini_map_id, proj_id; MiniMapVer mini_map_ver;
                  ClientRecvGetMiniMapVer(data, mini_map_ver, mini_map_id, proj_id);
                  if(proj_id==proj->id)
                     if(MiniMapVer *server_mini_map_ver=mini_map_vers.get(mini_map_id))
                  {
                     Swap(*server_mini_map_ver, mini_map_ver);
                     Synchronizer.syncMiniMap(mini_map_id);
                  }
               }break;

               case CS_SET_MINI_MAP_SETTINGS: if(proj)
               {
                  UID mini_map_id; Game::MiniMap::Settings settings; TimeStamp settings_time; UID proj_id;
                  if(ClientRecvSetMiniMapSettings(data, mini_map_id, settings, settings_time, proj_id))
                     if(proj_id==proj->id)
                        if(proj->syncMiniMapSettings(mini_map_id, settings, settings_time))
                           Preview.elmChanged(mini_map_id);
               }break;

               case CS_SET_MINI_MAP_IMAGE: if(proj)
               {
                  UID mini_map_id, proj_id; VecI2 image_xy; File image_data; TimeStamp image_time;
                  if(ClientRecvSetMiniMapImage(data, mini_map_id, image_xy, image_time, image_data.writeMem(), proj_id))
                     if(proj_id==proj->id)
                        if(proj->syncMiniMapImage(mini_map_id, image_xy, image_time, image_data))
                           Preview.elmChanged(mini_map_id);
               }break;

               case CS_SET_CODE_DATA: if(proj)
               {
                  Memc<ElmCodeData> elm_code_datas; UID proj_id;
                  if(ClientRecvSetCodeData(data, elm_code_datas, proj_id))if(proj_id==proj->id)proj->syncCodes(elm_code_datas);
               }break;

               case CS_CODE_SYNC_STATUS: if(proj)
               {
                  Memc<ElmCodeBase> elm_code_bases; bool resync; UID proj_id;
                  ClientRecvCodeSyncStatus(data, elm_code_bases, resync, proj_id);
                  if(proj_id==proj->id)proj->syncCodes(elm_code_bases, resync);
               }break;
            }
            if(Time.curTimeMs()-time>=delay)break;
         }

         // send project settings
         if(send_proj_settings && proj && proj->oldSettings()){send_proj_settings=false; ClientSendProjectSettings(T, *proj);}

         // update synchronizer
         Synchronizer.update();

         // add background thread commands to be sent
         Memc<File> cmds; if(Synchronizer.getCmds(cmds))FREPA(cmds)send(cmds[i], -1, false);

         // send all queued commands
         flush(0);
      }

      flt d=Time.realTime()-stats_time;
      if( d>=1)
      {
         stats_time=Time.realTime();
         sending=uint(sentTotal()-stats_sent)/d; stats_sent=sentTotal();
          rcving=uint(rcvdTotal()-stats_rcvd)/d; stats_rcvd=rcvdTotal();
      }
      Misc.online.desc(S+"Access: "+UserAccessText[access].name
                        +"\nSent: "+FileSize  (sentTotal())+  ",   Received: " +FileSize  (rcvdTotal())
                     +"\nSending: "+FileSizeKB(sending    )+"/s,   Receiving: "+FileSizeKB(rcving)+"/s"
             "\nElements to send: "+(Synchronizer.queuedElms()+(queued()>0))); // if there is data in the buffer then count it as one extra element

      EditServer.update(busy);
   }
  ServerClass::~ServerClass() {stopConnect();}
ServerClass::ServerClass() : after_connect(false), version_sent(false), version_ok(false), logged_in(false), reconnect(false), allow_reconnect(false), proj_opened(false), send_proj_settings(false), access(UA_NO_ACCESS), action(NONE), conn_pass_key(0), sending(0), rcving(0), sent(0), rcvd(0), stats_sent(0), stats_rcvd(0), reconnect_time(0), stats_time(0), world_vers(Compare), mini_map_vers(Compare) {}

/******************************************************************************/
