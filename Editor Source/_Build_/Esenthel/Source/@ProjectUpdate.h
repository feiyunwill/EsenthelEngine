/******************************************************************************/
/******************************************************************************/
class ProjectUpdate
{
   Memc<UID> texs;
   Project  *proj;
   SyncLock  lock;

   static void Error    (C Str &error);                      
   static void ErrorLoad(C Str &path );                                     
   static void ErrorSave(C Str &path );                                     
   static void UpdateTex(  UID &tex_id, ProjectUpdate&pu, int thread_index);
          void updateTex(C UID &tex_id);
   void start(Project &proj, Threads &threads);
   void stop(Threads &threads);

public:
   ProjectUpdate();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
