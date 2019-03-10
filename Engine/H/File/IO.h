/******************************************************************************

   Use 'Drive' and 'GetDrives' to access your device drives information.

   Use 'FileInfo' to access information about files and folders.

   Use 'FileFind' to find files.

   Use 'DataPath' to specify additional search path for accessing files.

   Use 'F*' functions to handle any file operations.

/******************************************************************************/
// DRIVE
/******************************************************************************/
enum DRIVE_TYPE : Byte // Drive Type
{
   DRIVE_UNDEFINED, // undefined
   DRIVE_DISK     , // disk      (HDD/SSD)
   DRIVE_OPTICAL  , // optical   (CD/DVD/Blu-Ray)
   DRIVE_USB      , // removable (USB)
   DRIVE_SD_CARD  , // removable (SD Card)
};
struct Drive // Drive in the Operating System
{
   DRIVE_TYPE type; // drive type
   Str        path, // for example "C:\" "D:\" on Windows, and "\" "Volumes\Bootcamp\" on Mac
              name; // for example "Hard Disk Drive", "Macintosh HD", "BOOTCAMP"
};
/******************************************************************************/
// FILE INFO
/******************************************************************************/
enum FSTD_TYPE : Byte // Standard File Type
{
   FSTD_NONE , // none
   FSTD_DRIVE, // drive
   FSTD_DIR  , // folder
   FSTD_FILE , // file
   FSTD_LINK , // symbolic link, this type can occur on Unix systems (this includes retrieving a list of files from Unix FTP server)
};
enum FATTRIB_FLAG // File Attribute Flags
{
   FATTRIB_READ_ONLY=0x1, // read-only
   FATTRIB_HIDDEN   =0x2, // hidden
};
struct FileInfo // File Information
{
   FSTD_TYPE type           ; // file type
   Byte      attrib         ; // file attributes, FATTRIB_FLAG
   DateTime  modify_time_utc; // file modification time (UTC time zone)
   Long      size           ; // file size

#if EE_PRIVATE
   #if WINDOWS
      void from(WIN32_FIND_DATA &fd);
   #endif
   void zero() {Zero(T);}
#endif

   Bool getSystem(C Str &name); // get info from 'name' file/folder in the system, false on fail
   Bool get      (C Str &name); // get info from 'name' file/folder              , false on fail, search is performed additionally in 'Paks' and 'DataPath'

   friend Bool operator==(C FileInfo &f0, C FileInfo &f1);                    // warning: file dates are compared with 1 second tolerance (because some file systems have lower precision for times), file attributes are not taken into account in this function
   friend Bool operator!=(C FileInfo &f0, C FileInfo &f1) {return !(f0==f1);} // warning: file dates are compared with 1 second tolerance (because some file systems have lower precision for times), file attributes are not taken into account in this function

   FileInfo(               );
   FileInfo(C Str     &name) {get(name);}
   FileInfo(C PakFile &pf  );
};
struct FileInfoSystem : FileInfo // System File Information, this is the same thing as 'FileInfo' except the default constructor always uses 'getSystem' method
{
   FileInfoSystem(C Str &name) {getSystem(name);}
};
/******************************************************************************/
// FILE FIND
/******************************************************************************/
STRUCT(FileFind , FileInfo) // File Finder
//{
   Str name; // currently encountered file base name (does not include path), this is valid when "Bool operator()" returns true

   // operations
   void find      (C Str &path, C Str &ext=S); // start searching in 'path', accept only 'ext' extensions (set empty for all)
   void findDrives(                         ); // start searching for drives
 
   // get
   Bool       operator()();                       // if  has file for processing
 C Str&       path      ()C {return _path      ;} // get searching path
   Str        pathName  ()C {return _path+ name;} // get encountered file full path name
   DRIVE_TYPE driveType ()C {return _drive_type;} // get encountered drive type (valid when current file is of FSTD_DRIVE type)

           ~FileFind();
            FileFind();
   explicit FileFind(C Str &path, C Str &ext=S);

private:
#if EE_PRIVATE
   enum STATE : Byte
   {
      NONE        ,
      FILE_WAITING,
      NEED_CHECK  ,
   };
#endif
   Str        _path, _ext;
   Byte       _state, _drive;
   DRIVE_TYPE _drive_type;
#if EE_PRIVATE
   PLATFORM(Ptr, DIR*) _handle;
#else
   Ptr        _handle;
#endif

#if EE_PRIVATE
#if WINDOWS
   Bool findValid(WIN32_FIND_DATA &fd);
#endif
   void zero    ();
   void del     ();
   Bool findNext();
#endif
   NO_COPY_CONSTRUCTOR(FileFind);
};
/******************************************************************************/
struct BackgroundFileFind
{
   struct File : FileInfo
   {
      Str name;
   };

   static Int Compare(C File &a, C File &b) {return ComparePath(a.name, b.name);}

   // get
   Bool finished(                 )C {return !_thread.active();} // if finished finding files
   Bool getFiles(Memc<File> &files);                             // get new files found since last call to 'getFiles' and till this moment, returns true if any files were returned

   // operations
   void find (C Str &path, Bool (*filter)(C Str &name)=null); // start finding files in path
   void clear(                                             ); // stop  current search and clear all files
#if EE_PRIVATE
   Bool update();
#endif

            BackgroundFileFind&   del();
           ~BackgroundFileFind() {del();}
            BackgroundFileFind() {}
   explicit BackgroundFileFind(C Str &path, Bool (*filter)(C Str &name)=null) {find(path, filter);}

private:
   UInt       _find_id;
   Memc<File> _files;
   Memc<Str>  _paths;
   SyncLock   _lock;
   Thread     _thread;
   Bool     (*_filter)(C Str &name);
};
/******************************************************************************/
// MAIN
/******************************************************************************/
enum FILE_OVERWRITE_MODE
{
   FILE_OVERWRITE_NEVER    , // never  overwrite
   FILE_OVERWRITE_ALWAYS   , // always overwrite
   FILE_OVERWRITE_DIFFERENT, //        overwrite only if size or modification time (with 1 second tolerance) is different
};
/******************************************************************************/
C Str& DataPath(           ); // get additional search path, used for opening files
  void DataPath(C Str &path); // set additional search path, used for opening files, given 'path' will have automatically added '/' character at the end if not present, and will be converted from relative to global (for example DataPath("../data") will set a complete path like "d:/game/data/" ending with '/' character - this is only an example)

void GetDrives(MemPtr<Drive> drives); // get Drives in the system
Bool GetDriveSize(C Str &path, Long *free=null, Long *total=null); // get drive size in bytes of specified path. Warning: on some platforms, if you don't have write permission to the specified path, then 'free' value may be incorrect, always use a path where you actually intend to write data, false is returned on fail

Str  CurDir(          ); // get current working directory, by default it's always set to path of the application executable file (an exception is iOS platform, there the path is set inside the executable, since the executable is actually a folder)
void CurDir(C Str &dir); // set current working directory

Str MakeFullPath(C Str &path, FILE_PATH type=FILE_CUR, Bool keep_empty=true); // if the path is not 'FullPath', then full path is returned as if 'path' was relative to 'type', 'keep_empty'=if the 'path' is empty then don't make it a full path but return an empty path

Bool FEqual      (            C Str     &a  , C Str &b   ,                                                      Cipher *  a_cipher=null, Cipher *   b_cipher=null                                   ); // if files have the same data
Bool FCopy       (            C Str     &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS, Cipher *src_cipher=null, Cipher *dest_cipher=null, CChar *safe_overwrite_suffix=null); // copy    'src'     file  to 'dest' file  , false on fail, 'safe_overwrite_suffix'=if specified then 'SafeOverwrite' function will be used
Bool FCopy       (  Pak &pak, C PakFile &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS,                          Cipher *dest_cipher=null, CChar *safe_overwrite_suffix=null); // copy    'src' pak file  to 'dest' file  , false on fail, 'safe_overwrite_suffix'=if specified then 'SafeOverwrite' function will be used
Bool FCopy       (  Pak &pak, C Str     &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS,                          Cipher *dest_cipher=null, CChar *safe_overwrite_suffix=null); // copy    'src' pak file  to 'dest' file  , false on fail, 'safe_overwrite_suffix'=if specified then 'SafeOverwrite' function will be used
Bool FCopy       (  Pak &src                , C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS,                          Cipher *dest_cipher=null                                   ); // extract 'src' pak files to 'dest' folder, false on fail
Bool FCopyDir    (C Str &src                , C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS, Cipher *src_cipher=null, Cipher *dest_cipher=null                                   ); // copy    'src' folder    to 'dest' folder, false on fail
Bool FReplaceDir (C Str &src                , C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_ALWAYS, Cipher *src_cipher=null, Cipher *dest_cipher=null                                   ); // copy    'src' folder    to 'dest' folder and remove all contents of 'dest' which are not present in 'src', false on fail
Str  FFirst      (C Str &name, C Str      &ext=S   ); // find first non-existing file, sample usage: FFirst("path/name ", "bmp") will return "path/name 0.bmp", "path/name 1.bmp", "path/name 2.bmp", ..
Str  FFirstUp    (C Str &name                      ); // find first     existing file/folder by going up in the path, for example when using FFirstUp("c:/folder1/folder2/file.ext") and in the case when only "c:/folder1" exists, but not "c:/folder1/folder2/file.ext", then "c:/folder1" will be returned. empty string is returned when nothing was found
Bool FTimeUTC    (C Str &name, C DateTime &time_utc); // set      file/folder modification time (in UTC time zone), false on fail
Bool FAttrib     (C Str &name,   UInt      attrib  ); // set      file           attributes, 'attrib'=FATTRIB_FLAG, false on fail
UInt FAttrib     (C Str &name                      ); // get      file           attributes           FATTRIB_FLAG
Long FSize       (C Str &name                      ); // get      file/folder size   in the system, -1 on fail
Bool FExistSystem(C Str &name                      ); // if       file/folder exists in the system
Bool FExist      (C Str &name                      ); // if       file/folder exists        , search is performed additionaly in 'Paks' and 'DataPath'
Bool FRename     (C Str &src , C Str &dest         ); // rename   file/folder, false on fail, if 'dest' file already exists then it will be overwritten by 'src'
Bool FMoveDir    (C Str &src , C Str &dest         ); // move          folder, false on fail
Bool FDelFile    (C Str &name                      ); // delete   file       , false on fail
Bool FDelDir     (C Str &name                      ); // delete  empty folder, false on fail (this will delete the folder only if it's empty, this will fail if it's not empty)
Bool FDelDirs    (C Str &name                      ); // delete        folder, false on fail (this will delete the folder and all elements inside it)
Bool FDelInside  (C Str &name                      ); // delete inside folder, false on fail (this will delete all elements inside provided path, but not the path itself)
Bool FDel        (C Str &name                      ); // delete   file/folder, false on fail (this function will call 'FDelFile' or 'FDelDirs' depending on if the element is a file or folder)
Bool FRecycle    (C Str &name, Bool hidden=true    ); // recycle  file/folder, false on fail, 'hidden'=if recycle with no OS UI and no confirmations (this function will move the element into system's "Recycle Bin")
Bool FCreateDir  (C Str &name                      ); // create        folder, false on fail
Bool FCreateDirs (C Str &name                      ); // create series of folders, usage: FCreateDirs("c:\a\b\c") creates "a", then "b" and "c", false on fail

Bool SafeOverwrite(File     &src, C Str &dest, C DateTime *modify_time_utc=null, Cipher *dest_cipher=null, C Str &suffix="@new", ReadWriteSync *rws=null); // safely overwrite 'dest' file from 'src' data, this first writes the full data (starting from current 'src' position) to a temporary name made from dest+suffix, after the file is written successfully, its modification time is set to 'modify_time_utc' if not null, after that the file name is changed to the final 'dest' replacing any existing file at that location, 'rws'=(optional) if specified then it will have its 'enterWrite' called just before replacing the file and 'leaveWrite' afterwards, false on fail
Bool SafeOverwrite(FileText &src, C Str &dest, C DateTime *modify_time_utc=null, Cipher *dest_cipher=null, C Str &suffix="@new", ReadWriteSync *rws=null); // safely overwrite 'dest' file from 'src' data, this first writes the full data (starting from zero    'src' position) to a temporary name made from dest+suffix, after the file is written successfully, its modification time is set to 'modify_time_utc' if not null, after that the file name is changed to the final 'dest' replacing any existing file at that location, 'rws'=(optional) if specified then it will have its 'enterWrite' called just before replacing the file and 'leaveWrite' afterwards, false on fail

enum FILE_LIST_MODE
{
   FILE_LIST_CONTINUE, // return this enum if you want to continue listing files
   FILE_LIST_SKIP    , // return this enum if currently processed element is a folder and you don't want to list elements inside it
   FILE_LIST_BREAK   , // return this enum if you don't want to continue listing files, but return from the recursion as soon as possible
};
         void FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, Ptr   user), Ptr   user=null); // list all elements inside specified path (in recursive mode) and report them into custom 'func' callback function, 'user'=custom user data
T1(TYPE) void FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE *user), TYPE *user=null); // list all elements inside specified path (in recursive mode) and report them into custom 'func' callback function, 'user'=custom user data
T1(TYPE) void FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE &user), TYPE &user     ); // list all elements inside specified path (in recursive mode) and report them into custom 'func' callback function, 'user'=custom user data

Bool CreateSymLink(C Str &name, C Str &target); // create a symbolic link, link will be created at 'name' location and it will be pointing to 'target' file, this function is used only on Unix systems and only for symbolic links, if you wish to create a shortcut then please use 'CreateShortcut' function instead, false on fail
Str  DecodeSymLink(File &f                   ); // this function is meant for resolving target file of a symbolic link, to use it pass an opened 'f' file of FSTD_LINK type to this function and it will return the target path, S on fail
/******************************************************************************/
#if EE_PRIVATE
void  InitIO();
void FlushIO();
Bool UnixReadFile(CChar8 *file, Char8 *data, Int size);
#endif
/******************************************************************************/
