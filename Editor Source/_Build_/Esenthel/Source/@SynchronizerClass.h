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
class SynchronizerClass
{
   class ElmDepth
   {
      int  depth;
      Elm *elm;

public:
   ElmDepth();
   };
   static int CompareDepth(C ElmDepth &a, C ElmDepth &b);

   class ElmFile
   {
      bool compress, compress_fast;
      File elm, data;

      int size()C;

      void process();
      void store(File &f);

public:
   ElmFile();
   };

   class WorldSync // class handling synchronization of a single world (what elements still need to be sent)
   {
      WorldVer   *server_ver, *local_ver;
      Memc<VecI2> set_area;
      Memc<UID  > set_obj;

      bool empty()C;
      int  elms ()C;

      void getServerVer(C UID &world_id);

public:
   WorldSync();
   };
   static bool Create(WorldSync &world_sync, C UID &world_id, ptr);

   class MiniMapSync // class handling synchronization of a single mini map (what elements still need to be sent)
   {
      MiniMapVer *server_ver, *local_ver;
      Memc<VecI2> set_image;

      bool empty()C;
      int  elms ()C;

      void getServerVer(C UID &mini_map_id);

public:
   MiniMapSync();
   };
   static bool Create(MiniMapSync &mini_map_sync, C UID &mini_map_id, ptr);

   bool                            compressing;
   Memc<ElmFile                  > set_elm_full_file, set_elm_long_file;
   Memc<UID                      > set_elm_full, set_elm_long, set_elm_short, set_tex;
   Memc<File                     > cmds;
   ThreadSafeMap<UID,   WorldSync>    world_sync, delayed_world_sync; // make thread-safe just in case
   ThreadSafeMap<UID, MiniMapSync> mini_map_sync;                                      // make thread-safe just in case
   Str                             game_path, edit_path, tex_path;
   Thread                          thread;
   SyncLock                        lock;
   flt                             last_delayed_time;

  ~SynchronizerClass(); // delete thread before other members

   static bool CompressFunc(Thread &thread);
          bool compressFunc();

   int elmFileSize();
   int queuedElms ();

   void clearSync();
   bool getCmds(Memc<File> &cmds);
   void setArea(C UID &world_id, C VecI2 &area_xy);
   void setObjs(C UID &world_id, Memc<UID> &obj_ids);
   void setMiniMapImage(C UID &mini_map_id, C VecI2 &image_xy);
   void delayedSetArea(C UID &world_id, C VecI2 &area_xy); // !! must be multi-threaded SAFE !!
   void delayedSetObj(C UID &world_id, C MemPtr<UID> &obj_id); // !! must be multi-threaded SAFE !!
   void erasing(C UID &elm_id);
   void erasingTex(C UID &tex_id);
   void setElmFull (C UID &elm_id);                    
   void setElmLong (C UID &elm_id);                    
   void setElmShort(C UID &elm_id);                     // if we're already sending long version then skip short
   void setTex     (C UID &tex_id);                    
   void createLocal(Project &local, Memx<Elm> &server); // process recursively to create parents first
   void sync(Project &local, Project &server);

   void syncWorld(C UID &world_id);
   void syncMiniMap(C UID &mini_map_id);
   void update();

public:
   SynchronizerClass();
};
/******************************************************************************/
/******************************************************************************/
extern SynchronizerClass Synchronizer;
/******************************************************************************/
