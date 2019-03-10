/******************************************************************************/
/******************************************************************************/
class BuilderClass
{
   static void Process    (AreaBuild &area, BuilderClass &builder, int thread_index);
   static void ProcessPath(AreaBuild &area, BuilderClass &builder, int thread_index);

   static bool IOFunc(Thread &thread);
          bool ioFunc(              );

   bool                            finished_processing, background;
   PathSettings                    path_settings;
   Str                             game_path, edit_path;
   SyncLock                        lock; // we need to operate on separate 'SyncLock' (and not 'areas.lock') because we use 'Swap' for 'areas' and thus we would operate on 2 'SyncLock's
   ThreadSafeMap<VecI2, AreaBuild> areas; // areas currently being processed, make thread-safe just in case
   Memc<EmbedObj>                  embed_objs; // embedded objects loaded for desired areas
   Memc<WaterMesh>                 waters; // waters loaded for desired areas
   RectI                           embed_objs_rect, water_rect;
   WorldVer                       *world_ver;
   Thread                          io_thread;
   SyncEvent                       io_wait;

  ~BuilderClass(); // delete thread before other members

   // get
   bool processing(                    ); // if we're currently processing some areas
   bool finished  (C UID *world_id=null); // if builded all areas for rebuilt

   // operations
   void stop();
   bool process(WorldVer &world_ver, C ElmWorld &world_data, VecI2 area_xy);
   void update(bool background, C UID *priority_world_id=null);

public:
   BuilderClass();
};
/******************************************************************************/
/******************************************************************************/
extern BuilderClass Builder;
/******************************************************************************/
