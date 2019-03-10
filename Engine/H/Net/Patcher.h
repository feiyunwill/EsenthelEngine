/******************************************************************************

   Use 'Patcher' for patching files on local device,
      according to server files uploaded using "Uploader" tool.

/******************************************************************************/
const_mem_addr struct Patcher // class for automatic downloading file updates from a http server !! must be stored in constant memory address !!
{
   struct LocalFile // use this class to describe an element stored on local device
   {
      enum TYPE : Byte
      {
         PAK_FILE   , // PakFile          (       file   inside Pak  archive)
         SYSTEM_FILE, // system file      (normal file   on     your drive  )
         SYSTEM_DIR , // system directory (normal folder on     your drive  )
      };

      TYPE     type           ; // specifies how the file is stored on the local device
      Str      full_name      ; // file full name (path+name) relative to your data storage folder (don't include the drive here, instead, use path relative to the folder which was used for uploading your files)
      Long     file_size      ; // file size in bytes (leave 0 for folders)
      UInt     xxHash64_32    ; // file hash (this is optional, it can be left at 0 which will disable checking for hash differences, however when enabled, it can provide more reliable detecting differences between file versions)
      DateTime modify_time_utc; // file modification time in UTC time zone

      LocalFile& set(C Str &full_name, C FileInfo &fi); // set from 'full_name' and 'FileInfo'

      LocalFile() {type=PAK_FILE; file_size=0; xxHash64_32=0; modify_time_utc.zero();}
   };

   struct Downloaded
   {
      Bool      success        ; // file downloaded successfully, if this is equal to false it means that the file was not found or it was different than expected
      FSTD_TYPE type           ; // type of the element, can be FSTD_FILE, FSTD_DIR or FSTD_LINK
      Int       index          ; // file index in 'Patcher.index' Pak
      UInt      xxHash64_32    ; // file data hash (can be 0 if not calculated)
      DateTime  modify_time_utc; // file modification time in UTC time zone
      Str       full_name      ; // full name of downloaded file
      File      data           ; // file data

      Downloaded() {success=false; index=-1; xxHash64_32=0; type=FSTD_NONE; modify_time_utc.zero();}
   #if EE_PRIVATE
      void create     (C Pak &pak, Int index, Download &download, Cipher *cipher);
      void createEmpty(C Pak &pak, Int index);
      void createFail (C Pak &pak, Int index);
   #endif
   };

   struct InstallerInfo
   {
      Int      size           ; // file size
      UInt     xxHash64_32    ; // file data hash
      DateTime modify_time_utc; // file modification time in UTC time zone

   #if EE_PRIVATE
      void zero() {size=0; xxHash64_32=0; modify_time_utc.zero();}
   #endif
      InstallerInfo();
   };

   // local file list
   static void SetList(MemPtr<LocalFile> local_files, C Str    &dir ); // automatically create 'local_files' list from 'dir'  folder on your local device (this will process all system files and folders inside 'dir'  and list them in 'local_files' container)
   static void SetList(MemPtr<LocalFile> local_files, C PakSet &paks); // automatically create 'local_files' list from 'paks' set of paks                 (this will process all pak files                inside 'paks' and list them in 'local_files' container)

   // manage
   Patcher& create(C Str &http_dir, C Str &upload_name, Cipher *cipher=null); // create the patcher according to provided internet address, 'http_dir'=case-sensitive http address of the directory where files were uploaded (for example: "http://www.domain.com/download"), 'upload_name'=case-sensitive name of the upload (for example: "GameName"), 'cipher'=cipher keys used during upload, these parameters must match the ones you have provided in the Uploader tool

   // get
      // installer information
      Patcher& downloadInstallerInfo     (); // initialize downloading of the installer information, downloading will begin on secondary thread
      DWNL_STATE       installerInfoState(); // get state  of installer information download, once it's equal to DWNL_DONE you can access it
    C InstallerInfo*   installerInfo     (); // get access to installer information (this will return a valid pointer only if 'installerInfoState' is equal to DWNL_DONE), null on fail

      // installer
      Patcher& downloadInstaller        (); // initialize downloading of the installer, downloading will begin on secondary thread (this automatically initiates downloading of installer information if it wasn't previously downloaded)
      DWNL_STATE       installerState   (); // get state  of installer download, once it's equal to DWNL_DONE you can access it
      Int              installerProgress(); // get number of downloaded bytes for the installer
    C Mems<Byte>*      installer        (); // get access to raw bytes of installer .exe file which you can write on disk, quit current installer and run the updated version (this will return a valid pointer only if 'installerState' is equal to DWNL_DONE), null on fail

      // index
      Patcher& downloadIndex     (); // initialize downloading of the index (list of all current files on the server), downloading will begin on secondary thread
      DWNL_STATE       indexState(); // get state  of index download, once it's equal to DWNL_DONE you can access its information
    C Pak*             index     (); // get access to the index of files on the server (this will return a valid pointer only if 'indexState' is equal to DWNL_DONE), null on fail

      // file
      Patcher& downloadFile(Int         i         ); // initialize downloading of i-th file in index from server, downloading will begin on secondary thread, this method should be called after downloading the index
      Bool  getNextDownload(Downloaded &downloaded); // get next downloaded file, this method checks if there is a file fully downloaded, if there is one, it sets information about it into 'downloaded' parameter and returns true, if there is no file downloaded at this moment, then false is returned and given parameter is unmodified

   // stats
   Long progress ()C; // get number of bytes that have been already downloaded (this includes only files, this does not include installer info, installer, index and overhead)
   Long filesSize()C; // get number of bytes of all files requested to be downloaded (this includes files that have been downloaded and those that still need to be downloaded)
   Int  filesLeft()C; // get number of files that are still waiting to be downloaded

   // compare
   Bool compare(C MemPtr<LocalFile> &local_files, MemPtr<Int> local_remove, MemPtr<Int> server_download); // compare list of files from server (index) with list of files on local device (local_files), 'local_remove'=list of indexes from 'local_files' that need to be removed (those files are not present in the server, therefore we should remove them), 'server_download'=list of indexes from "index->file" that need to be downloaded from the server (those files aren't present in the local device or are of different version, therefore we should download latest version and replace existing one if any), this method relies on 'index', so you should call it only after downloading the index using 'downloadIndex' method with 'indexState' returning DWNL_DONE, false on fail ('index' not yet available)

   static void Compare(C MemPtr<LocalFile> &src_files, C MemPtr<LocalFile> &dest_files, MemPtr<Int> dest_remove, MemPtr<Int> src_copy); // compare list of files from source ('src_files') with list of files from destination ('dest_files'), 'dest_remove'=list of indexes from 'dest_files' that need to be removed (those files are not present in the source, therefore we should remove them), 'src_copy'=list of indexes from 'src_files' that need to be copied from the source to the destination (those files aren't present in destination or are of different version, therefore we should copy latest version from the source and replace existing one in the destination, if any)

   Patcher&   del(); // delete manually
  ~Patcher() {del();}
   Patcher();

#if EE_PRIVATE
   Bool is    () {return _http.is();}
   void zero  ();
   void update();
#endif

private:
   struct FileDownload : Download
   {
      Int index;
   };
   Str              _http, _name;
   Cipher          *_cipher;
   Bool             _pak_available, _inst_info_available, _inst_available;
   Download         _pak_download , _inst_info_download , _inst_download ;
   FileDownload     _file_download[6];
   Int              _files_left;
   Long             _bytes_downloaded;
   Pak              _pak;
   InstallerInfo    _inst_info;
   Mems<Byte>       _inst;
   Memc<Int >       _to_download;
   Memc<Downloaded> _downloaded;
   SyncLock         _lock;
   Thread           _thread;
};
/******************************************************************************/
