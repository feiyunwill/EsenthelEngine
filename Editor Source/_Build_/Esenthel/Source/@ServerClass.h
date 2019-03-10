/******************************************************************************/
/******************************************************************************/
class ServerClass : Connection
{
   enum ACTION
   {
      NONE       ,
      LOGIN      ,
      REGISTER   ,
      FORGOT_PASS,
      CHANGE_PASS,
   };
   bool                           after_connect, version_sent, version_ok, logged_in, reconnect, allow_reconnect, proj_opened, send_proj_settings;
   USER_ACCESS                    access;
   ACTION                         action;
   SockAddr                       conn_addr;
   Str                            conn_email, conn_pass, conn_name, logged_email;
   uint                           conn_pass_key, sending, rcving;
   long                           sent, rcvd, stats_sent, stats_rcvd;
   flt                            reconnect_time, stats_time;
   Memx<Project>                  projects;
   Memc<UID>                      texs; // textures available on the server
   ThreadSafeMap<UID,   WorldVer>    world_vers; // element exists only if we have received data from server (even empty), make thread-safe just in case
   ThreadSafeMap<UID, MiniMapVer> mini_map_vers; // element exists only if we have received data from server (even empty), make thread-safe just in case

   // get
   bool loggedIn    ()C;
   bool canRead     ()C;
   bool canWrite    ()C;
   bool canWriteCode()C;
   bool smallBuf    ()C; // if buffer usage is small

   long sentTotal()C;
   long rcvdTotal()C;

   // operations
   void clearProj();
   void logout();     
   void stopConnect();
   void startConnect(ACTION action);
   void connect(SockAddr &addr, C Str &email, C Str &pass, bool allow_reconnect=false);
   void Register(Str name);
   void forgotPass(SockAddr &addr, C Str &email);
   void changePass(SockAddr &addr, C Str &email, C Str &new_pass, uint change_pass_key);
   void licenseKey        (C Str &license_key              );                         
   void projectOpen       (C UID &proj_id, C Str &proj_name);                         
   void projectDataRequest(                                );                         
   void projectSetSettings(                                );                         
   void syncCodes         (  Memc<ElmTypeVer > &elms       );                         
   void syncCodes         (C Memc<ElmCodeData> &elms       );                         

   void    newElm    (     Elm  &elm                                     );
   void renameElm    (     Elm  &elm                                     );
   void setElmParent (     Elm  &elm                                     );
   void removeElms   (Memc<UID> &elms, bool removed   , C TimeStamp &time);
   void noPublishElms(Memc<UID> &elms, bool no_publish, C TimeStamp &time);
   void getElmNames  (Memc<UID> &elms                                    );
   void getTextures  (Memc<UID> &texs                                    );
   void getElmShort  (Memc<UID> &elms                                    );
   void getElmLong   (Memc<UID> &elms                                    );
   void getElmLong   (   C UID  &elm_id                                  );

   void setTex     (C UID &tex_id); // send if it wasn't available on the server yet, this should be called everytime a texture is created in the project (OR reused in case it was created before but not yet sent because it was not used at that time)
   void setElmShort(C UID &elm_id); // this sets only ID TYPE DATA       but no NAME, PARENT, etc.
   void setElmLong (C UID &elm_id); // this sets only ID TYPE DATA FILES but no NAME, PARENT, etc.
   void setElmFull (C UID &elm_id);

   void setWaypoint(C UID &world_id, C UID &waypoint_id, C Version &waypoint_ver, EditWaypoint &waypoint);
   void setLake    (C UID &world_id, C UID &    lake_id, C Version &    lake_ver, Lake         &lake    );
   void setRiver   (C UID &world_id, C UID &   river_id, C Version &   river_ver, River        &river   );

   void getWorldVer      (C UID &world_id                           ); // request if don't have yet
   void getWorldAreas    (C UID &world_id, Memc<AreaSync> &areas    );
   void getWorldWaypoints(C UID &world_id, Memc<UID     > &waypoints);
   void getWorldLakes    (C UID &world_id, Memc<UID     > &lakes    );
   void getWorldRivers   (C UID &world_id, Memc<UID     > &rivers   );

   void getMiniMapVer     (C UID &mini_map_id                     ); // request if don't have yet
   void getMiniMapImages  (C UID &mini_map_id, Memc<VecI2> &images);

   void setMiniMapSettings(C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time);

   void update(ProjectEx *proj, bool busy);

  ~ServerClass();

public:
   ServerClass();
};
/******************************************************************************/
/******************************************************************************/
extern ServerClass Server;
/******************************************************************************/
