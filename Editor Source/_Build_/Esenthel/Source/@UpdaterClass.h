/******************************************************************************/
/******************************************************************************/
class UpdaterClass
{
   static cchar8 *TutorialsProjID;
   static const int MaxDownloadAttempts;

   static bool CreateFailedDownload(int &failed, C Str &file, ptr user);

   bool    ready, show, has_update;
   Thread  thread;
   Patcher patcher;
   Str     path, update_path;
   Memc<Patcher::LocalFile> local_files;
   Memc<int>               local_remove;
   Map<Str, int>           failed_download; // doesn't need to be thread-safe

   static FILE_LIST_MODE Filter(C FileFind &ff, UpdaterClass &updater);
   static FILE_LIST_MODE FilterUpdate(C FileFind &ff, UpdaterClass &updater);
   static FILE_LIST_MODE HasUpdate(C FileFind &ff, UpdaterClass &updater);

   bool hasUpdate();
   bool updating(); 
   flt  progress(); 

   static bool Update(Thread &thread);
   bool update(); // !! this is called on secondary thread !!

   void create();
   void del();
  ~UpdaterClass(); // to manually delete the thread before other members

public:
   UpdaterClass();
};
/******************************************************************************/
/******************************************************************************/
extern UpdaterClass Updater;
/******************************************************************************/
