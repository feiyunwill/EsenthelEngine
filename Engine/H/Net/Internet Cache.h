/******************************************************************************/
struct InternetCache
{
   struct SrcFile
   {
    C PakFile    *pf; // from Pak  File
      DataSource *ds; // from Data Source

      void clear() {pf=null; ds=null;}

      SrcFile() {clear();}
   };

   // manage
   void del   (); // delete manually
   void create(C Str &db_name, const_mem_addr Threads *threads=null, const_mem_addr Cipher *cipher=null, COMPRESS_TYPE compress=COMPRESS_LZ4, Int image_mip_maps=-1); // create, 'db_name'=file name where the database will be located, 'threads'=worker threads that will import the images (if null then importing will be done on the main thread), 'cipher'=database file encryption, 'compress'=database file compression, 'image_mip_maps'=number of mip maps to be created when importing images (-1..Inf, -1=keep original value, 0=autodetect)

   // operations
   ImagePtr getImage(C Str &url               ); // get image from the internet, image may be empty at start if it's not yet downloaded, it will be automatically updated once it completes downloading
   Bool     getFile (C Str &url, SrcFile &file); // get file  from the internet, 'file' will contain a way to access this file, false is returned if file is not yet available and will be downloaded
   void     changed (C Str &url               ); // notify the cache that the file on the internet has just been changed and needs updating
   void     flush   (                         ); // flush updated files to disk, warning: this may update the Pak file, because of which existing file references 'SrcFile' obtained through 'getFile' will become invalid

  ~InternetCache() {del();}
   InternetCache() {_image_mip_maps=0; _compress=COMPRESS_NONE; _threads=null;}

#if !EE_PRIVATE
private:
#endif
   struct ImportImage : SrcFile
   {
      Bool     done;       // if finished importing
      ImagePtr image_ptr;  // image into which import
      Image    image_temp; // temp image which will have the data

      ImportImage() {done=false;}
   };
   const_mem_addr struct Downloaded : PakFileData
   {
      Mems<Byte> file_data;
   };

   COMPRESS_TYPE     _compress;
   Int               _image_mip_maps;
   Threads          *_threads;
   Pak               _pak;
   Memb<Downloaded>  _downloaded; // use 'Memb' to have const_mem_addr needed for threads and pointing to 'data'
   Download          _downloading[6];
   Memc<Str>         _to_download, _verified, _to_verify;
   Memx<ImportImage> _import_images; // use 'Memx' to have const_mem_addr needed for threads

#if EE_PRIVATE
   Bool busy  ()C;
   void enable();
   void update();
   void import(ImportImage &ii);
#endif
};
/******************************************************************************/
