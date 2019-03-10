/******************************************************************************/
class StoreFiles : InternetCache
{
   bool initialized=false;

   void init()
   {
      if(!initialized)
      {
         initialized=true;
         create("Bin/Store.dat", &WorkerThreads, null, COMPRESS_LZ4, 2);
      }
   }
   ImagePtr getImage(C Str &url)
   {
      init(); return super.getImage(url);
   }
}
StoreFiles IC;
/******************************************************************************/
