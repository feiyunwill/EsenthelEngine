/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_CHUNK CC4('C','H','N','K')
/******************************************************************************/
static Str _DataPath;
/******************************************************************************/
Str CurDir()
{
#if WINDOWS
   wchar_t path[MAX_LONG_PATH]; path[0]=0; GetCurrentDirectory(Elms(path), path);
   return  path;
#else
   Char8  path[MAX_UTF_PATH]; path[0]=0; getcwd(path, Elms(path));
   return FromUTF8(path);
#endif
}
void CurDir(C Str &dir)
{
#if WINDOWS
   SetCurrentDirectory(dir);
#else
   chdir(UnixPathUTF8(dir));
#endif
}
/******************************************************************************/
C Str& DataPath(           ) {return _DataPath;}
  void DataPath(C Str &path)
{
   Str p=NormalizePath(MakeFullPath(path)).tailSlash(true); // copy first to a temp variable, to avoid using move assignment which would change pointer address of the '_DataPath' string, this is to avoid thread issues, for example if one thread would cast DataPath to CChar* and was processing that, while the other thread changed DataPath and released that CChar* then the first thread would crash
  _DataPath=p; // adjust '_DataPath' in only one operation to maximize thread-safety
}
/******************************************************************************/
Str MakeFullPath(C Str &path, FILE_PATH type, Bool keep_empty)
{
   if(type==FILE_CUR || type==FILE_DATA) // do not adjust Android Assets because they're always accessed without any paths (just asset name)
      if(!keep_empty || path.is())
         if(!FullPath(path))return ((type==FILE_CUR) ? CurDir().tailSlash(true) : DataPath())+path; // 'DataPath' already has 'tailSlash'
   return path;
}
/******************************************************************************/
// FILE INFO
/******************************************************************************/
FileInfo::FileInfo() {zero();}
FileInfo::FileInfo(C PakFile &pf)
{
   type           =pf.type();
   attrib         =0;
   size           =pf.data_size;
   modify_time_utc=pf.modify_time_utc;
}
#if WINDOWS
void FileInfo::from(WIN32_FIND_DATA &fd)
{
   if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   {
      type=FSTD_DIR;
      size=0;
   }else
   {
      type=FSTD_FILE;
      size=((ULong(fd.nFileSizeHigh)<<32)|fd.nFileSizeLow);
   }

   SYSTEMTIME sys_time; FileTimeToSystemTime(&fd.ftLastWriteTime, &sys_time);
   modify_time_utc.year  =sys_time.wYear;
   modify_time_utc.month =sys_time.wMonth;
   modify_time_utc.day   =sys_time.wDay;
   modify_time_utc.hour  =sys_time.wHour;
   modify_time_utc.minute=sys_time.wMinute;
   modify_time_utc.second=sys_time.wSecond;

   attrib=0;
   if(fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN  )attrib|=FATTRIB_HIDDEN;
   if(fd.dwFileAttributes&FILE_ATTRIBUTE_READONLY)attrib|=FATTRIB_READ_ONLY;
}
#endif
Bool FileInfo::getSystem(C Str &name)
{
   if(name.is())
   {
      if(IsDrive(name))
      {
         Memt<Drive> drives; GetDrives(drives);
         REPA(drives)if(EqualPath(drives[i].path, name))
         {
            zero(); type=FSTD_DRIVE;
            return true;
         }
      }
   #if WINDOWS
      WIN32_FILE_ATTRIBUTE_DATA fad; if(GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
      {
         if(fad.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
         {
            type=FSTD_DIR;
            size=0;
         }else
         {
            type=FSTD_FILE;
            size=((ULong(fad.nFileSizeHigh)<<32)|fad.nFileSizeLow);
         }

         SYSTEMTIME sys_time; FileTimeToSystemTime(&fad.ftLastWriteTime, &sys_time);
         modify_time_utc.year  =sys_time.wYear;
         modify_time_utc.month =sys_time.wMonth;
         modify_time_utc.day   =sys_time.wDay;
         modify_time_utc.hour  =sys_time.wHour;
         modify_time_utc.minute=sys_time.wMinute;
         modify_time_utc.second=sys_time.wSecond;

         attrib=0;
         if(fad.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN  )attrib|=FATTRIB_HIDDEN;
         if(fad.dwFileAttributes&FILE_ATTRIBUTE_READONLY)attrib|=FATTRIB_READ_ONLY;

         return true;
      }
   #else
      struct stat stats; if(!lstat(UnixPathUTF8(name), &stats))
      {
         attrib=0;

      #ifdef UF_HIDDEN
         if(stats.st_flags&UF_HIDDEN)attrib|=FATTRIB_HIDDEN;else if(CChar *base=_GetBase(name))if(*base=='.')attrib|=FATTRIB_HIDDEN;
      #else
         if(CChar *base=_GetBase(name))if(*base=='.')attrib|=FATTRIB_HIDDEN;
      #endif
         if(!(stats.st_mode&S_IWUSR) && !(stats.st_mode&S_IWGRP) && !(stats.st_mode&S_IWOTH))attrib|=FATTRIB_READ_ONLY;

         if(S_ISLNK(stats.st_mode))
         {
            type=FSTD_LINK;
            size=stats.st_size;
         }else
         if(S_ISDIR(stats.st_mode))
         {
            type=FSTD_DIR;
            size=0;
         }else
         {
            type=FSTD_FILE;
            size=stats.st_size;
         }

      #if APPLE
         tm gmt; gmtime_r(&stats.st_mtimespec.tv_sec, &gmt);
      #else
         tm gmt; time_t t=stats.st_mtime; gmtime_r(&t, &gmt);
      #endif
         modify_time_utc.year  =gmt.tm_year+1900;
         modify_time_utc.month =gmt.tm_mon+1;
         modify_time_utc.day   =gmt.tm_mday;
         modify_time_utc.hour  =gmt.tm_hour;
         modify_time_utc.minute=gmt.tm_min;
         modify_time_utc.second=gmt.tm_sec;

         return true;
      }
   #endif
   }
   zero(); return false;
}
Bool FileInfo::get(C Str &name)
{
   if(name.is())
   {
      if(C PaksFile *psf=Paks.find(name)){T=*psf->file; return true;}
      if(getSystem(name))return true;
      if(DataPath().is() && !FullPath(name)){Char full[MAX_LONG_PATH]; MergePath(full, DataPath(), name); if(getSystem(full))return true;}
   }
   zero(); return false;
}
Bool operator==(C FileInfo &f0, C FileInfo &f1)
{
   if(f0.type==f1.type)switch(f0.type)
   {
      case FSTD_DRIVE:
      case FSTD_DIR  : return true;

      case FSTD_LINK:
      case FSTD_FILE: return f0.size==f1.size && !Compare(f0.modify_time_utc, f1.modify_time_utc, 1);
   }
   return false;
}
/******************************************************************************/
// FILE FIND
/******************************************************************************/
void FileFind::zero()
{
  _state     =NONE;
  _drive     =0;
  _drive_type=DRIVE_UNDEFINED;
  _handle    =PLATFORM(INVALID_HANDLE_VALUE, null);
}
void FileFind::del()
{
#if WINDOWS
   if(_handle!=INVALID_HANDLE_VALUE)FindClose(_handle);
#else
   if(_handle)closedir(_handle);
#endif
   zero();
}
FileFind::~FileFind() {del ();}
FileFind:: FileFind() {zero();}
FileFind:: FileFind(C Str &path, C Str &ext) : FileFind() {find(path, ext);}
/******************************************************************************/
#if WINDOWS
Bool FileFind::findValid(WIN32_FIND_DATA &fd)
{
   for(; fd.cFileName[0]; )
   {
      if(fd.cFileName[0]=='.')if(!fd.cFileName[1] || (fd.cFileName[1]=='.' && !fd.cFileName[2]))goto dot; // "." or ".."
      if(!_ext.is() || _ext==_GetExt(WChar(fd.cFileName)))
      {
         name=fd.cFileName;
         super::from(fd);
         return true;
      }
   dot:;
      if(!FindNextFile(_handle, &fd))break;
   }
   return false;
}
#endif
Bool FileFind::findNext()
{
   if(_drive)
   {
      for(;;)
      {
         Memt<Drive> drives; GetDrives(drives);
         Byte i=_drive-1; if(i>=drives.elms())break; _drive++;
         name      =drives[i].path;
        _drive_type=drives[i].type;
         type      =FSTD_DRIVE;
         attrib    =0;
         size      =0;
         modify_time_utc.zero();
         return true;
      }
   }else
   {
   #if WINDOWS
      WIN32_FIND_DATA fd;
      if(FindNextFile(_handle, &fd))if(findValid(fd))return true;
   #else
      for(; dirent *file=readdir(_handle); ) // 'readdir' is multi-thread safe, no need to use 'readdir_r' which is now deprecated
      {
         name=FromUTF8(file->d_name);
      #if APPLE // on Apple we need to precompose strings
         if(HasUnicode(name))name=NSStringAuto(name);
      #endif
         if( name.is())if(name[0]=='.')if(!name[1] || (name[1]=='.' && !name[2]))continue;
         if(!_ext.is() || Equal(_ext, _GetExt(name)))
         {
            super::getSystem(pathName());
            return true;
         }
      }
   #endif
   }
  _state=NONE;
   return false;
}
void FileFind::find(C Str &path, C Str &ext)
{
   del();

   T._ext =ext;
   T._path=path; _path.tailSlash(true);

#if WINDOWS
   WIN32_FIND_DATA fd;
    //_handle =FindFirstFile  (_path+'*', &fd);
      _handle =FindFirstFileEx(_path+'*', FindExInfoBasic, &fd, FindExSearchNameMatch, null, 0); // this is supposed to be faster than 'FindFirstFile', due to 'FindExInfoBasic' not processing 'cAlternateFileName'
   if(_handle!=INVALID_HANDLE_VALUE)if(findValid(fd))_state=FILE_WAITING;
#else
   if(_handle=opendir(_path.is() ? UnixPathUTF8(_path)() : "."))_state=NEED_CHECK;
#endif
}
void FileFind::findDrives()
{
   del();

  _drive=1;
  _state=NEED_CHECK;
}
Bool FileFind::operator()()
{
   switch(_state)
   {
      case NEED_CHECK  :                    return findNext();
      case FILE_WAITING: _state=NEED_CHECK; return true;
      default          :                    return false;
   }
}
/******************************************************************************/
// BACKGROUND FILE FIND
/******************************************************************************/
static Bool BackgroundFileFindThreadFunc (Thread &thread    ) {return ((BackgroundFileFind*)thread.user)->update();}
static Int  BackgroundFileFindComparePath(C Str &a, C Str &b) {return ComparePath(b, a);} // return in reversed order because later we're taking last element from paths
/******************************************************************************/
BackgroundFileFind& BackgroundFileFind::del()
{
  _thread.del(); // delete the thread first so it will not process data on released members
  _files .del();
  _paths .del();
  _filter=null;
   return T;
}
/******************************************************************************/
Bool BackgroundFileFind::getFiles(Memc<File> &files)
{
   files.clear();
   if(T._files.elms())
   {
      SyncLocker locker(_lock);
      Swap(files, T._files);
   }
   return files.elms()>0;
}
/******************************************************************************/
void BackgroundFileFind::find(C Str &path, Bool (*filter)(C Str &name))
{
   SyncLocker locker(_lock);
  _files.clear();
  _paths.setNum(1)[0]=path;
  _filter=filter;
  _find_id++;
   if(!_thread.active())_thread.create(BackgroundFileFindThreadFunc, this);
}
/******************************************************************************/
void BackgroundFileFind::clear()
{
   SyncLocker locker(_lock);
  _files.clear();
  _paths.clear();
  _filter=null;
  _find_id++;
}
/******************************************************************************/
Bool BackgroundFileFind::update()
{
   for(; !_thread.wantStop(); )
   {
      // get path
      SyncLockerEx locker(_lock); if(!_paths.elms())return false;
      Str path=_paths.pop(); UInt find_id=T._find_id; // get unique id of the 'find' operation
      locker.off();

      // find files in path
      Memc<Str> new_paths;
      for(FileFind ff(path); !_thread.wantStop() && ff(); )
      {
         File file; FileInfo &fi=file; fi=ff; file.name=ff.pathName();
         SyncLockerEx locker(_lock);
         if(find_id==T._find_id) // check id's in case during this loop new 'find' was called
         {
            if(!_filter || _filter(file.name))
            {
               if(file.type==FSTD_DRIVE || file.type==FSTD_DIR)new_paths.add(file.name); // add folder to paths list
               Swap(_files.New(), file);
            }
         }else
         {
            new_paths.clear();
            break;
         }
      }

      // add new paths
      if(new_paths.elms())
      {
         locker.on (); if(find_id!=T._find_id)new_paths.clear();else FREPA(_paths)new_paths.add(_paths[i]);
         locker.off(); new_paths.sort(BackgroundFileFindComparePath);
         locker.on (); if(find_id==T._find_id)Swap(_paths, new_paths);
         locker.off();
      }
   }
   return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
static Bool FListEx(C Str &path, FILE_LIST_MODE func(C FileFind &ff, Ptr user), Ptr user)
{
   for(FileFind ff(path); ff(); )switch(ff.type)
   {
      case FSTD_LINK:
      case FSTD_FILE: if(func(ff, user)==FILE_LIST_BREAK)return false; break;

      case FSTD_DIR:
      {
         switch(func(ff, user))
         {
          //case FILE_LIST_SKIP    : break; do nothing
            case FILE_LIST_BREAK   : return false;
            case FILE_LIST_CONTINUE: if(!FListEx(ff.pathName(), func, user))return false; break;
         }
      }break;
   }
   return true;
}
void FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, Ptr user), Ptr user) {FListEx(path, func, user);}
/******************************************************************************/
Bool FEqual(C Str &a, C Str &b, Cipher *a_cipher, Cipher *b_cipher)
{
   File fa, fb;
   if(fa.readTry(a, a_cipher)
   && fb.readTry(b, b_cipher))return fa.equal(fb);
   return false;
}
Bool FCopy(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *src_cipher, Cipher *dest_cipher, CChar *safe_overwrite_suffix)
{
   if(EqualPath(src,      dest))return src_cipher==dest_cipher; Str full_dest=MakeFullPath(dest); // we can't make 'src' to be full path because it can be path from 'DataPath' or 'Pak'
   if(EqualPath(src, full_dest))return src_cipher==dest_cipher;
   if(overwrite!=FILE_OVERWRITE_NEVER || !FExistSystem(full_dest))
   {
      File d, s; if(s.readTry(src, src_cipher))
      {
         // get source info
       C PakFile  *pf=null;
         FileInfo  fi;
         if(s._pak)pf=s._pak->find(src, false);else fi.get(src);

         // check
         if(overwrite==FILE_OVERWRITE_DIFFERENT)
         {
            if(pf)fi=*pf;
            if(fi==FileInfoSystem(full_dest))return true;
         }

         // write
      #if APPLE || LINUX
         if(s._pak ? (pf && (pf->flag&PF_STD_LINK)) : (fi.type==FSTD_LINK)) // if this is a symbolic link
         {
            FDelFile(full_dest); // delete first because 'CreateSymLink' will fail if file already exists
            if(!CreateSymLink(full_dest, DecodeSymLink(s)))return false;
         }else
      #endif
         if(Is(safe_overwrite_suffix))
         {
            if(!SafeOverwrite(s, full_dest, pf ? &pf->modify_time_utc : fi.type ? &fi.modify_time_utc : null, dest_cipher, safe_overwrite_suffix))return false;
         }else
         {
            if(!d.writeTry(full_dest, dest_cipher) || !s.copy(d) || !d.flush())return false;
            d.del(); // release handle so we can apply file params

            if(pf     )FTimeUTC(full_dest, pf->modify_time_utc);else
            if(fi.type)FTimeUTC(full_dest,  fi.modify_time_utc);
         }

         // apply params
         if(fi.type)FAttrib(full_dest, fi.attrib);
         return true;
      }
   }
   return false;
}
Bool FCopy(Pak &pak, C PakFile &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *dest_cipher, CChar *safe_overwrite_suffix)
{
   Bool ok=true;
   if(!(src.flag&PF_REMOVED))
   {
      if(src.children_num || (src.flag&PF_STD_DIR)) // folder
      {
         if(ok=(FCreateDir(dest) || FileInfoSystem(dest).type==FSTD_DIR))
            if(src.children_num)
         {
            Str dest_slash=dest; dest_slash.tailSlash(true);
            FREP(src.children_num)
            {
             C PakFile &file=pak.file(src.children_offset+i);
               ok&=FCopy(pak, file, dest_slash+file.name, overwrite, dest_cipher, safe_overwrite_suffix);
            }
         }
      }else // file
      if(overwrite!=FILE_OVERWRITE_NEVER || !FExistSystem(dest))
      {
         // check
         if(overwrite==FILE_OVERWRITE_DIFFERENT && FileInfo(src)==FileInfoSystem(dest))return true;

         File s, d; if(!s.readTry(src, pak))ok=false;else
         {
         #if APPLE || LINUX
            if(src.flag&PF_STD_LINK)
            {
               FDelFile(dest); // delete first because 'CreateSymLink' will fail if file already exists
               if(CreateSymLink(dest, DecodeSymLink(s)))FTimeUTC(dest, src.modify_time_utc);else ok=false;
            }else
         #endif
            if(Is(safe_overwrite_suffix))
            {
               if(!SafeOverwrite(s, dest, &src.modify_time_utc, dest_cipher, safe_overwrite_suffix))return false;
            }else
            if(!d.writeTry(dest, dest_cipher) || !s.copy(d) || !d.flush())ok=false;else
            {
               d.del(); // release handle so we can apply file params
               FTimeUTC(dest, src.modify_time_utc);
            }
         }
      }
   }
   return ok;
}
Bool FCopy(Pak &pak, C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *dest_cipher, CChar *safe_overwrite_suffix)
{
   if(C PakFile *pak_file=pak.find(src, false))return FCopy(pak, *pak_file, dest, overwrite, dest_cipher, safe_overwrite_suffix);
   return false;
}
Bool FCopy(Pak &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *dest_cipher)
{
   Bool ok=true;
   FCreateDirs(dest);
   Str dest_path=dest; dest_path.tailSlash(true);
   FREP(src.rootFiles())
   {
    C PakFile &file=src.file(i);
      ok&=FCopy(src, file, dest_path+file.name, overwrite, dest_cipher);
   }
   return ok;
}
Bool FCopyDir(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *src_cipher, Cipher *dest_cipher)
{
   Bool ok=true;
   if(FExistSystem(dest) || FCreateDirs(dest))for(FileFind ff(src); ff(); )
   {
      Str name=dest; name.tailSlash(true)+=ff.name;
      switch(ff.type)
      {
         case FSTD_LINK:
         case FSTD_FILE: if(overwrite!=FILE_OVERWRITE_DIFFERENT || ff!=FileInfoSystem(name))ok&=FCopy   (ff.pathName(), name, overwrite, src_cipher, dest_cipher); break;
         case FSTD_DIR :                                                                    ok&=FCopyDir(ff.pathName(), name, overwrite, src_cipher, dest_cipher); break;
      }
   }else  ok=false;
   return ok;
}
/******************************************************************************/
Bool FTimeUTC(C Str &name, C DateTime &time_utc) // Android OS has a bug which makes the 'utimes' and 'File.setLastModified' always fail - https://code.google.com/p/android/issues/detail?id=18624
{
   if(name.is())
   {
   #if WINDOWS
      #if WINDOWS_OLD
         HANDLE file=CreateFile(name, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_DELETE, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null); // FILE_FLAG_BACKUP_SEMANTICS is needed for opening folders
      #else
         CREATEFILE2_EXTENDED_PARAMETERS ex; Zero(ex);
         ex.dwSize=SIZE(ex);
       //ex.dwFileAttributes=0;
         ex.dwFileFlags=FILE_FLAG_BACKUP_SEMANTICS; // needed for opening folders
         HANDLE file=CreateFile2(name, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_DELETE, OPEN_EXISTING, &ex);
      #endif
      if(file!=INVALID_HANDLE_VALUE)
      {
         FILETIME  file_time;
         SYSTEMTIME sys_time;
         sys_time.wYear  =time_utc.year;
         sys_time.wMonth =time_utc.month;
         sys_time.wDay   =time_utc.day;
         sys_time.wHour  =time_utc.hour;
         sys_time.wMinute=time_utc.minute;
         sys_time.wSecond=time_utc.second;
         sys_time.wDayOfWeek   =0;
         sys_time.wMilliseconds=0;
         SystemTimeToFileTime(&sys_time, &file_time);
         Bool ok=(SetFileTime(file, null, null, &file_time)!=0);
         CloseHandle(file);
         return ok;
      }
   #else
      timeval tv[2];
      tv[0].tv_sec =time_utc.seconds1970();
      tv[0].tv_usec=0;
      tv[1]=tv[0];
   #if ANDROID || WEB
      if(! utimes(UnixPathUTF8(name), tv))
   #else
      if(!lutimes(UnixPathUTF8(name), tv)) // this does not follow symbolic links
   #endif
      {
         FlushIO();
         return true;
      }
   #endif
   }
   return false;
}
Bool FAttrib(C Str &name, UInt attrib)
{
   if(name.is())
   {
   #if WINDOWS
      UInt out=0;
      if(attrib&FATTRIB_READ_ONLY)out|=FILE_ATTRIBUTE_READONLY;
      if(attrib&FATTRIB_HIDDEN   )out|=FILE_ATTRIBUTE_HIDDEN  ;
      return SetFileAttributes(name, out)!=0;
   #else
      Str8 utf=UnixPathUTF8(name);
      struct stat stats; if(!lstat(utf, &stats))
      {
         UInt mode =(stats.st_mode&(S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX));
         Bool write=!FlagTest(attrib, FATTRIB_READ_ONLY);
         FlagSet(mode, S_IWUSR, write);
         FlagSet(mode, S_IWGRP, write);
         FlagSet(mode, S_IWOTH, write);
      #ifdef UF_HIDDEN
         UInt flags=stats.st_flags; FlagSet(flags, UF_HIDDEN, FlagTest(attrib, FATTRIB_HIDDEN));
         if(!lchflags(utf, flags) && !lchmod(utf, mode))
      #else
         if(!chmod(utf, mode))
      #endif
         {
            FlushIO();
            return true;
         }
      }
   #endif
   }
   return false;
}
UInt FAttrib(C Str &name)
{
   UInt out=0;
   if(name.is())
   {
   #if WINDOWS_OLD
      UInt attrib =GetFileAttributes(name);
      if(  attrib!=INVALID_FILE_ATTRIBUTES)
      {
         if(attrib&FILE_ATTRIBUTE_HIDDEN  )out|=FATTRIB_HIDDEN   ;
         if(attrib&FILE_ATTRIBUTE_READONLY)out|=FATTRIB_READ_ONLY;
      }
   #elif WINDOWS_NEW
      WIN32_FILE_ATTRIBUTE_DATA fad; if(GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
      {
         if(fad.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN  )out|=FATTRIB_HIDDEN;
         if(fad.dwFileAttributes&FILE_ATTRIBUTE_READONLY)out|=FATTRIB_READ_ONLY;
      }
   #else
      struct stat stats; if(!lstat(UnixPathUTF8(name), &stats))
      {
      #ifdef UF_HIDDEN
         if(stats.st_flags&UF_HIDDEN)out|=FATTRIB_HIDDEN;else if(CChar *base=_GetBase(name))if(*base=='.')out|=FATTRIB_HIDDEN;
      #else
         if(CChar *base=_GetBase(name))if(*base=='.')out|=FATTRIB_HIDDEN;
      #endif
         if(!(stats.st_mode&S_IWUSR) && !(stats.st_mode&S_IWGRP) && !(stats.st_mode&S_IWOTH))out|=FATTRIB_READ_ONLY;
      }
   #endif
   }
   return out;
}
static Long FSizeDir(C Str &name)
{
   Long   size=0; for(FileFind ff(name); ff(); )if(ff.type==FSTD_DIR)size+=FSizeDir(ff.pathName());else size+=ff.size;
   return size;
}
Long FSize(C Str &name)
{
   FileInfoSystem fi(name);
   switch(fi.type)
   {
      case FSTD_NONE: return -1;
      case FSTD_DIR : return FSizeDir(name);
      default       : return fi.size;
   }
}
/******************************************************************************/
Bool FExistSystem(C Str &name)
{
   if(name.is())
   {
   #if WINDOWS_OLD
      return GetFileAttributes(name)!=INVALID_FILE_ATTRIBUTES;
   #elif WINDOWS_NEW
      WIN32_FILE_ATTRIBUTE_DATA fad; return GetFileAttributesEx(name, GetFileExInfoStandard, &fad)!=0;
   #else
      struct stat stats; return !lstat(UnixPathUTF8(name), &stats);
   #endif
   }
   return false;
}
Bool FExist(C Str &name)
{
   if(name.is())
   {
      Bool full_path=FullPath(name);
      if( !full_path && Paks.find(name))return true;
      if(            FExistSystem(name))return true;
      if(DataPath().is() && !full_path){Char full[MAX_LONG_PATH]; MergePath(full, DataPath(), name); if(FExistSystem(full))return true;}
   }
   return false;
}
Str FFirst(C Str &name, C Str &ext)
{
   Str file; FREP(10000)
   {
      file=name; file+=i; if(ext.is()){file+='.'; file+=ext;}
      if(!FExistSystem(file))return file;
   }
   return S;
}
Str FFirstUp(C Str &name)
{
   for(Str path=name; path.is(); path=GetPath(path))if(FExistSystem(path))return path;
   return S;
}
/******************************************************************************/
Bool FRename(C Str &src, C Str &dest)
{
   Str s=MakeFullPath(src), d=MakeFullPath(dest);
   if(EqualPath(s, d) && Equal(GetBase(s), GetBase(d), true))return true; // allow changing case
#if WINDOWS
   return MoveFileEx(s, d, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
   #if MAC
      if(FileInfoSystem(s).type!=FSTD_LINK) // links can't be processed by 'FSPathMoveObjectSync' because the target file will be moved instead of the link
      {
         Str8   dest_base=UnixPathUTF8(GetBase(d));
         CFStringRef name=(dest_base.is() ? CFStringCreateWithCString(kCFAllocatorDefault, dest_base, kCFStringEncodingUTF8) : null);
         Bool          ok=(FSPathMoveObjectSync(UnixPathUTF8(s), UnixPathUTF8(GetPath(d)), name, null, kFSFileOperationOverwrite|kFSFileOperationSkipPreflight)==noErr);
         if(name)CFRelease(name);
         if(ok)return true; // return only on success, if failed, then try methods below (this can fail for current app's exe file)
      }
   #endif
      if(!rename(UnixPathUTF8(s), UnixPathUTF8(d))){FlushIO(); return true;}
      if(errno==EXDEV)if(FCopy(s, d))return FDel(s);
      return false;
#endif
}
#if WINDOWS
Bool FDelFile  (C Str &name) {return DeleteFile     (name      )!=0;}
Bool FCreateDir(C Str &name) {return CreateDirectory(name, null)!=0;}
Bool FDelDir   (C Str &name) {return RemoveDirectory(name      )!=0;}
#else
Bool FDelFile  (C Str &name) {if(!unlink(UnixPathUTF8(name)                         )){FlushIO(); return true;} return false;} // 'remove' will delete both files/dirs
Bool FCreateDir(C Str &name) {if(!mkdir (UnixPathUTF8(name), S_IRWXU|S_IRWXG|S_IRWXO)){FlushIO(); return true;} return false;}
Bool FDelDir   (C Str &name) {if(!rmdir (UnixPathUTF8(name)                         )){FlushIO(); return true;} return false;}
#endif

static Bool FDel(FSTD_TYPE type, C Str &name)
{
   switch(type)
   {
      case FSTD_LINK:
      case FSTD_FILE: return FDelFile(name);
      case FSTD_DIR : return FDelDirs(name);
   }
   return false;
}
static inline Bool FDel(C FileFind &ff) {return FDel(ff.type, ff.pathName());}

Bool FCreateDirs(C Str &name)
{
   if(!name.is())return true;
   Str   path=name; path.tailSlash(false);
   Char  temp[MAX_LONG_PATH];
   FREPA(temp)
   {
      Char c=path[i];
      if( !c)return FCreateDir(path);
      if(IsSlash(c) && !(i && path[i-1]==L':')){temp[i]=0; FCreateDir(temp);}
      temp[i]=c;
   }
   return false;
}
Bool FDelDirs(C Str &name)
{
   return FDelInside(name) && FDelDir(name); // delete dir contents and the dir itself
}
Bool FDelInside(C Str &name)
{
   if(name.is())
   {
      Bool   ok=true; for(FileFind ff(name); ff(); )ok&=FDel(ff); // keep removing other files even if one fails
      return ok;
   }
   return false;
}
Bool FDel(C Str &name) {return FDel(FileInfoSystem(name).type, name);}
#if WINDOWS_NEW
struct FileRecycler
{
   Bool ok;

   FileRecycler(C Str &name)
   {
      auto get_file=concurrency::create_task(Windows::Storage::StorageFile::GetFileFromPathAsync(ref new Platform::String(WindowsPath(name)))); // 'WindowsPath' must be used or exception will occur when using '/' instead of '\'
      if(App.mainThread())
      {
         ok=false;
         get_file.then([this](concurrency::task<Windows::Storage::StorageFile^> get_file)
         {
            try
            {
               concurrency::create_task(get_file.get()->DeleteAsync(Windows::Storage::StorageDeleteOption::Default)).then([this]()
               {
                  ok=true;
               });
            }
            catch(...){ok=true;}
         });
         App.loopUntil(ok);
      }else
      {
         try
         {
            auto del=concurrency::create_task(get_file.get()->DeleteAsync(Windows::Storage::StorageDeleteOption::Default));
            del.wait();
         }
         catch(...){}
      }
      ok=!FExistSystem(name);
   }
};
struct FolderRecycler
{
   Bool ok;

   FolderRecycler(C Str &name)
   {
      auto get_folder=concurrency::create_task(Windows::Storage::StorageFolder::GetFolderFromPathAsync(ref new Platform::String(WindowsPath(name)))); // 'WindowsPath' must be used or exception will occur when using '/' instead of '\'
      if(App.mainThread())
      {
         ok=false;
         get_folder.then([this](concurrency::task<Windows::Storage::StorageFolder^> get_folder)
         {
            try
            {
               concurrency::create_task(get_folder.get()->DeleteAsync(Windows::Storage::StorageDeleteOption::Default)).then([this]()
               {
                  ok=true;
               });
            }
            catch(...){ok=true;}
         });
         App.loopUntil(ok);
      }else
      {
         try
         {
            auto del=concurrency::create_task(get_folder.get()->DeleteAsync(Windows::Storage::StorageDeleteOption::Default));
            del.wait();
         }
         catch(...){}
      }
      ok=!FExistSystem(name);
   }
};
#endif
Bool FRecycle(C Str &name, Bool hidden)
{
   if(name.is())
   {
   #if WINDOWS_OLD
      Str full=MakeFullPath(name); // must be full path
      if(HasDrive(full))
      {
         Char path[MAX_LONG_PATH+1]; path[SetReturnLength(path, full, Elms(path)-1)+1]=0; // must be "double null terminated" - "\0\0"
         SHFILEOPSTRUCT sh; Zero(sh);
         sh.wFunc = FO_DELETE;
         sh.pFrom =WChar(path);
         sh.fFlags=(FOF_ALLOWUNDO|(hidden ? FOF_NO_UI : 0));
         return !SHFileOperation(&sh);
      }
   #elif WINDOWS_NEW
      switch(FileInfoSystem(name).type)
      {
         case FSTD_FILE: return   FileRecycler(name).ok;
         case FSTD_DIR : return FolderRecycler(name).ok;
      }
   #elif MAC
      FSRef fs_ref; if(FSPathMakeRef((UInt8*)(UnixPathUTF8(name)()), &fs_ref, null)==noErr)return FSMoveObjectToTrashSync(&fs_ref, null, 0)==noErr;
   #elif LINUX
      Str full=MakeFullPath(name); // must be full path
      if(HasDrive(full))
      {
         Str trash=SystemPath(SP_TRASH); if(trash.is())
         {
            Str base=GetBase(full), first=FFirst(trash+"/files/"+base); if(first.is())
            {
               FileText f; if(f.write(trash+"/info/"+GetBase(first)+".trashinfo", UTF_8_NAKED)) // spec requires writing to "info" first, UTF is probably not supported
               {
                  DateTime dt; dt.getLocal();
                  f.putLine("[Trash Info]");
                  f.putLine(S+"Path="+UnixPath(full));
                  f.putLine(S+"DeletionDate="+dt.year+'-'+dt.month+'-'+dt.day+'T'+dt.hour+':'+dt.minute+':'+dt.second);
                  if(f.flushOK())
                  {
                     f.del();
                     if(FRename(full, first))return true;
                  }
               }
            }
         }
      }
      return FDel(name);
   #else
      return FDel(name);
   #endif
   }
   return false;
}
static Bool FMoveDirEx(C Str &src, C Str &dest)
{
   Bool ok=true;
   FCreateDir(dest);
   Str  src_t= src;  src_t.tailSlash(true);
   Str dest_t=dest; dest_t.tailSlash(true);
   for(FileFind ff(src); ff(); )switch(ff.type)
   {
      case FSTD_LINK:
      case FSTD_FILE: ok&=FRename   (src_t+ff.name, dest_t+ff.name); break;
      case FSTD_DIR : ok&=FMoveDirEx(src_t+ff.name, dest_t+ff.name); break;
   }
   ok&=FDelDir(src);
   return ok;
}
Bool FMoveDir(C Str &src, C Str &dest)
{
   Str s=MakeFullPath(src ),
       d=MakeFullPath(dest);
   if(!StartsPath(d, s))
   {
      if(FExistSystem(s))
      {
                FCreateDirs(   d);
         return FMoveDirEx (s, d);
      }
      return false;
   }
   return EqualPath(s, d);
}
static Bool FReplaceDirEx(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *src_cipher, Cipher *dest_cipher) // !! assumes that 'src' and 'dest' have 'tailSlash(true)' !!
{
   Bool ok=true;

   // first remove all 'dest' elements not present in 'src' to free disk space, also remove elements of different types (for example on 'src' it's a file while on 'dest' it's a folder)
   for(FileFind ff(dest); ff(); )if(FileInfoSystem(src+ff.name).type!=ff.type)ok&=FDel(ff);

   // create folder
   FCreateDir(dest);

   // now copy all 'src' elements to 'dest'
   for(FileFind ff(src); ff(); )
      if(ff.type==FSTD_DIR)ok&=FReplaceDirEx(ff.pathName().tailSlash(true), dest+ff.name+'\\', overwrite, src_cipher, dest_cipher);
      else                 ok&=FCopy        (ff.pathName()                , dest+ff.name     , overwrite, src_cipher, dest_cipher);

   return ok;
}
Bool FReplaceDir(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite, Cipher *src_cipher, Cipher *dest_cipher)
{
   Str s=MakeFullPath(src), d=MakeFullPath(dest);
   if(!StartsPath(d, s))
   {
             FCreateDirs  (d);
      return FReplaceDirEx(s.tailSlash(true), d.tailSlash(true), overwrite, src_cipher, dest_cipher);
   }
   return EqualPath(s, d) && src_cipher==dest_cipher;
}
/******************************************************************************/
void GetDrives(MemPtr<Drive> drives)
{
   drives.clear();
#if WINDOWS_OLD
   UInt       drv=GetLogicalDrives();
   FREP(32)if(drv&(1<<i))
   {
      Drive &d=drives.New();
      d.path.reserve(3)+=Char('A'+i); d.path+=":\\";
      switch(GetDriveType(d.path))
      {
         case DRIVE_FIXED    : d.type=DRIVE_DISK     ; break;
         case DRIVE_CDROM    : d.type=DRIVE_OPTICAL  ; break;
         case DRIVE_REMOVABLE: d.type=DRIVE_USB      ; break;
         default             : d.type=DRIVE_UNDEFINED; break;
      }
      if(i>=2) // skip "A:" and "B:" floppy drives because querying their 'GetVolumeInformation' may be slow (even few seconds), don't remove this, as there can be computers without the floppy drives, but the drivers can still be installed
      {
         wchar_t name[MAX_LONG_PATH+1]; name[0]=0; GetVolumeInformation(d.path, name, Elms(name), null, null, null, null, 0);
          d.name=name;
      }
   }
#elif WINDOWS_NEW
   // TODO: WINDOWS_NEW 'GetDrives'
#elif MAC
   FREP(32)
   {
      FSVolumeRefNum ref;
      HFSUniStr255   name;
      FSVolumeInfo   info;
      if(FSGetVolumeInfo(kFSInvalidVolumeRefNum, 1+i, &ref, kFSVolInfoFlags, &info, &name, null)!=noErr)break;

      Bool     read_only=((info.flags&kFSVolFlagSoftwareLockedMask)!=0);
      Char8    url_path[MAX_UTF_PATH];
      CFURLRef url; FSCopyURLForVolume(ref, &url); CFURLGetFileSystemRepresentation(url, true, (UInt8*)url_path, Elms(url_path)); if(url)CFRelease(url);

      Drive &d=drives.New();
      d.type=(read_only ? DRIVE_OPTICAL : DRIVE_DISK);
      d.path=Replace(FromUTF8(url_path), '/', '\\'); d.path.tailSlash(true); // make sure slash is added to preserve consistency with Windows version
      FREP(name.length)d.name+=(Char)name.unicode[i];
   }
#elif ANDROID
   {
      Drive &d=drives.New();
      d.path="\\";
      d.name="Drive";
      d.type=DRIVE_DISK;
   }
   if(AndroidSDCardPath.is())
   {
      Drive &d=drives.New();
      d.path=AndroidSDCardPath;
      d.name="SD Card";
      d.type=DRIVE_SD_CARD;
   }
#else
   #if 1
      Drive &d=drives.New();
      d.path="\\";
      d.name="Drive";
      d.type=DRIVE_DISK;
   #else
      Memc<struct statfs> st; Int fs=getfsstat(null, 0, MNT_NOWAIT); st.setNum(Max(0, fs)+16); fs=getfsstat(st.data(), st.elms()*st.elmSize(), MNT_NOWAIT);
      FREP(fs)if(st[i].f_blocks && st[i].f_bsize)
      {
         Drive &d=drives.New();
         d.path=st[i].f_mntonname; d.path.tailSlash(true);
         d.type=((st[i].f_flags&MNT_RDONLY) ? DRIVE_OPTICAL : DRIVE_DISK);
      }
   #endif
#endif
}
/******************************************************************************/
Bool GetDriveSize(C Str &path, Long *free, Long *total)
{
#if WINDOWS
   ULARGE_INTEGER _free, _total; if(path.is() && GetDiskFreeSpaceEx(path, &_free, &_total, null)){if(free)*free=_free.QuadPart; if(total)*total=_total.QuadPart; return true;}
#elif IOS && 0 // no need to use this, as 'statvfs' works fine
   if(NSStringAuto _path=UnixPath(path))
      if(NSDictionary *dict=[[NSFileManager defaultManager] attributesOfFileSystemForPath:_path error:nil])
   {
      if(free )*free =[[dict objectForKey:NSFileSystemFreeSize] longLongValue];
      if(total)*total=[[dict objectForKey:NSFileSystemSize    ] longLongValue];
    //[dict release]; do not release as it will crash
      return true;
   }
#elif ANDROID && __ANDROID_API__<21 // on Android, only API 21 and above has 'statvfs', for below we need to use Java
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID driveSizeFree =jni->GetStaticMethodID(ActivityClass, "driveSizeFree" , "(Ljava/lang/String;)J"))
   if(JMethodID driveSizeTotal=jni->GetStaticMethodID(ActivityClass, "driveSizeTotal", "(Ljava/lang/String;)J"))
   if(JString p=JString(jni, UnixPath(path)))
   {
      Long _free =jni->CallStaticLongMethod(ActivityClass, driveSizeFree , p()),
           _total=jni->CallStaticLongMethod(ActivityClass, driveSizeTotal, p());
      if(_free>=0 && _total>=0)
      {
         if(free )*free =_free;
         if(total)*total=_total;
         return true;
      }
   }
#else
   struct statvfs stats; if(!statvfs(UnixPathUTF8(path), &stats))
   { // !! need to use 'f_frsize' instead of 'f_bsize' because on Apple it won't return correct results !!
      if(free )*free =ULong(stats.f_frsize)*stats.f_bavail; // use 'f_bavail' instead of 'f_bfree' because that matches what File Managers report about being free on Mac/Linux (on iOS it makes no difference)
      if(total)*total=ULong(stats.f_frsize)*stats.f_blocks;
      return true;
   }
#endif
   if(free )*free =-1;
   if(total)*total=-1;
   return false;
}
/******************************************************************************/
Bool SafeOverwrite(File &src, C Str &dest, C DateTime *modify_time_utc, Cipher *dest_cipher, C Str &suffix, ReadWriteSync *rws)
{
   Bool ok=false;
   if(dest.is())
   {
      Bool locked=false;
      Str  temp=dest+suffix;
      if(rws && !suffix.is()){rws->enterWrite(); locked=true;} // if there's no suffix then it means we're writing directly to the target, so write lock needs to be enabled at start
      File f; if(f.writeTry(temp, dest_cipher))
      {
         ok=(src.copy(f) && f.flush());
         f.del(); // release file handle so we can set file time/params and rename/remove if needed
         if(ok)
         {
            if(modify_time_utc)FTimeUTC(temp, *modify_time_utc);
            if(suffix.is())
            {
               if(rws){rws->enterWrite(); locked=true;} // lock before renaming
               if(!FRename(temp, dest))
               {
                  ok=false;
                  UInt attr=FAttrib(dest); if(attr&FATTRIB_READ_ONLY)if(FAttrib(dest, attr&(~FATTRIB_READ_ONLY))) // if failed, then try clearing READ_ONLY attribute
                  {
                     FAttrib(temp, attr); // keep READ_ONLY on the new file
                     ok=FRename(temp, dest);
                  }
               }
            }
         }
         if(!ok)FDelFile(temp); // don't leave temp files
      }
      if(locked)rws->leaveWrite();
   }
   return ok;
}
Bool SafeOverwrite(FileText &src, C Str &dest, C DateTime *modify_time_utc, Cipher *dest_cipher, C Str &suffix, ReadWriteSync *rws)
{
   Bool ok=false;
   Long pos=src.pos(); // remember position to restore later
   if(src._f.pos(  0))ok=SafeOverwrite(src._f, dest, modify_time_utc, dest_cipher, suffix, rws);
      src._f.pos(pos); // restore position
   return ok;
}
/******************************************************************************/
Bool CreateSymLink(C Str &name, C Str &target)
{
#if APPLE || LINUX
   return !symlink(UnixPathUTF8(target), UnixPathUTF8(name)); // this will fail if 'name' already exists
#else
   return false;
#endif
}
Str DecodeSymLink(File &f)
{
   if(f.left()<=MAX_UTF_PATH) // paths should only be in this range
   {
      Char8 s[MAX_UTF_PATH+1]; s[f.getReturnSize(s, MAX_UTF_PATH)]='\0'; return FromUTF8(s);
   }
   return S;
}
/******************************************************************************/
void InitIO()
{
#if FILE_MEMB_UNION
   ASSERT(OFFSET(File, _mem)==OFFSET(File, _memb));
#else
   ASSERT(OFFSET(File, _mem)!=OFFSET(File, _memb));
#endif

   // Data Path
  _DataPath._d.setNumZero(MAX_LONG_PATH); // allocate string memory up-front, so when changing it later, the CChar* pointer will not be different (to avoid multi-threading issues), use '_d.setNumZero' instead of 'reserve' so that all characters are zero at start
#if WINDOWS_OLD && DEBUG // automatically set data path when building engine in debug mode
   DataPath(GetPath(_GetPath(_GetPath(GetPath(__FILE__))))+"\\Data"); // set 'DataPath' to the "Engine.pak" "Data" folder which is detected based on relative location of this CPP file
#endif

   // Current Directory
#if WEB // WEB has this handled in main
#elif IOS
   CurDir(App.exe()); // on iOS set path to the application (since on iOS it's a folder, which contains resource files)
#else
   CurDir(GetPath(App.exe()));
#endif
}
void FlushIO()
{
#if WEB
   EM_ASM(FS.syncfs(false, function(err){});); // save data
#endif
}
#if !WINDOWS
Bool UnixReadFile(CChar8 *file, Char8 *data, Int size)
{
   Bool ok=false;
   if(data && size>0)
   {
      Int read=0;
      int fd =open(file, O_RDONLY|O_NONBLOCK);
      if( fd>=0)
      {
         read=::read(fd, data, size-1); // leave room for nul character
         if(read>=0 && read<size)ok=true;else read=0;
         close(fd);
      }
      data[read]='\0';
   }
   return ok;
}
#endif
/******************************************************************************/
}
/******************************************************************************/
