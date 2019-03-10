/******************************************************************************/
/******************************************************************************/
class FileSizeGetter
{
   class Elm
   {
      UID  id;
      uint file_size;
   };
   Memc<Elm> elms;

   // get
   bool created()C;
   bool busy   (); 
   bool get    ();

   // manage
   void clear();
   void stop ();
   void del  ();
   void get(C Str &path);
  ~FileSizeGetter();   

private:
   Str       path;
   Memc<Elm> elms_thread;
   SyncLock  lock;
   Thread    thread;

   void cleanup();

   static bool Func(Thread &thread);
          bool func();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
