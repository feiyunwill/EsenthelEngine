/******************************************************************************/
class SendFileObj
{
   UID                        id;
   MemcThreadSafe<IOFileData> files;

   long memUsage()C
   {
      long usage=0;
      files.  lock(); REPA(files)usage+=files.lockedElm(i).memUsage();
      files.unlock();
      return usage;
   }

   SendFileObj() {id.randomize(); SendFileObjs.include(this);}
  ~SendFileObj()
   {
      SendFileObjs.exclude(this);

      // remove unprocessed files if any
      IOToRead.  lock(); REPA(IOToRead)if(IOToRead.lockedElm(i).id==id)IOToRead.remove(i, true);
      IOToRead.unlock();
        IORead.  lock(); REPA(  IORead)if(  IORead.lockedElm(i).id==id)  IORead.remove(i, true);
        IORead.unlock();
   }
}
MemcThreadSafe<SendFileObj*> SendFileObjs;
/******************************************************************************/
cchar8 *SizeSuffix[]={"", " KB", " MB", " GB", " TB"};
Str FileSize(long size)
{
   const int f=10;
   size*=f;
   int i=0; for(; i<Elms(SizeSuffix)-1 && size>=1000*f; i++, size>>=10); // check for "1000*f" instead of "1024*f", because we want to avoid displaying things like "1 001 MB"
   Str s=TextInt(size/f, -1, 3); if(size<100*f && i){s+=','; s+=size%10;} s+=SizeSuffix[i];
   return s;
}
/******************************************************************************/
bool SafeCopy(C Str &src, C Str &dest)
{
   File f; return f.readStdTry(src) && SafeOverwrite(f, dest, &NoTemp(FileInfoSystem(src).modify_time_utc), null, S+"@new"+Random());
}
bool ErrorCopy(C Str &src, C Str &dest)
{
   Gui.msgBox(S, S+"Error copying\n\""+src+"\"\nto\n\""+dest+'"');
   return false;
}
bool ErrorRecycle(C Str &name)
{
   Gui.msgBox(S, S+"Error recycling\n\""+name+"\"");
   return false;
}
bool ErrorCreateDir(C Str &name)
{
   Gui.msgBox(S, S+"Error creating folder\n\""+name+"\"");
   return false;
}
bool RecycleLoud  (C Str &name            ) {return FRecycle   (name     ) ? true : ErrorRecycle  (name);}
bool CreateDirLoud(C Str &name            ) {return FCreateDirs(name     ) ? true : ErrorCreateDir(name);}
bool SafeCopyLoud (C Str &src, C Str &dest) {return SafeCopy   (src, dest) ? true : ErrorCopy     (src, dest);}
/******************************************************************************/
Str ExpandPath(C Str &path, C Str &dest)
{
   if(FullPath(dest))return dest;
   return NormalizePath(Str(path).tailSlash(true)+dest);
}
/******************************************************************************/
void GetFileList(C Str &path, MemPtr<Pane.ElmBase> elms)
{
   elms.clear();
   FileFind ff; if(!path.is())ff.findDrives();else if(FullPath(path))ff.find(path);
   for(; ff(); )if(ff.name!=".DS_Store")
   {
      Pane.ElmBase &elm=elms.New();
      SCAST(FileInfo, elm)=ff;
      elm.name=ff.name.tailSlash(false); // drives may have slashes
   }
}
/******************************************************************************/
bool SendFiles(C Memc<Str2> &names, Connection &conn, long &progress, Thread &thread, bool report_errors)
{
   if(thread.wantStop())return false;
   SendFileObj obj;
   IOToRead.  lock(); FREPA(names)IOToRead.lockedNew().set(names[i].src, obj.id);
   IOToRead.unlock();
   File f; f.writeMem();
   FREPA(names)
   {
    C Str2 &name=names[i];
    again:
      if(thread.wantStop())return false;
      if(!obj.files.elms()){Time.wait(1); goto again;} // wait until file is ready

      IOFileData file;
      obj.files.  lock(); Swap(file, obj.files.lockedElm(0)); obj.files.remove(0, true);
      obj.files.unlock();
      if(!Equal(file.name, name.src, true))return false; // shouldn't happen
      if(file.error && report_errors){Gui.msgBox(S, S+"Error transferring file:\n\""+file.name+'"'); goto skip;} // if an error occured, then report if it's enabled on this side and and proceed to the next file, otherwise send information that the file failed, so the other side can report the error
      {
         f.reset().putByte(CMD_REPLACE).putStr(name.dest).putByte((file.finished<<0) | (file.error<<1) | (file.compressed<<2))<<file.id.i[0];
         if(file.finished)f<<file.modify_time_utc;
         f.cmpULongV(file.offset);
         f.put(file.data.data(), file.data.elms());
         f.pos(0);
         if(!conn.send(f, -1, false))return false;
         int to_send_total=conn.queued(), uncompressed_sent_last=0;
         for(;;)
         {
            if(thread.wantStop())return false;
            bool sent_all=conn.flush(1);
            int to_send=conn.queued(), sent=to_send_total-to_send, uncompressed_sent=(sent_all ? file.uncompressed_size : Round(dbl(sent)/to_send_total*file.uncompressed_size));
            progress+=uncompressed_sent-uncompressed_sent_last; uncompressed_sent_last=uncompressed_sent;
            if(sent_all)break;
         }
      }
   skip:
      if(file.finished || file.error)progress++;else goto again; // if finished or error then update progress, otherwise we're waiting for the same file again
   }
   return true;
}
/******************************************************************************/
class FileLister
{
   class File : Patcher.LocalFile
   {
      bool save(.File &f)C
      {
         f<<type<<file_size<<modify_time_utc<<full_name;
         return f.ok();
      }
      bool load(.File &f)
      {
         xxHash64_32=0;
         f>>type>>file_size>>modify_time_utc>>full_name;
         return f.ok();
      }
   }

   Str        path;
   Memc<File> files;
   Thread    &thread;

   Memc<Patcher.LocalFile>& Files() {return files;}

   static FILE_LIST_MODE ListFiles(C FileFind &ff, FileLister &lister)
   {
      if(lister.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name!=".DS_Store")lister.files.New().set(SkipStartPath(ff.pathName(), lister.path), ff);
      return FILE_LIST_CONTINUE;
   }

   FileLister(C Str &path, Thread &thread) : thread(thread) {T.path=Str(path).tailSlash(true); FList(path, ListFiles, T);}
}
/******************************************************************************/
class FileList
{
   Str                   name;
   FileInfo              fi;
   Memc<FileLister.File> files;
   
   bool save(File &f)C
   {
      f<<name<<fi;
      if(fi.type==FSTD_DIR)if(!files.save(f))return false;
      return f.ok();
   }
   bool load(File &f)
   {
      f>>name>>fi;
      if(fi.type!=FSTD_DIR)files.clear();else if(!files.load(f))return false;
      return f.ok();
   }
}
/******************************************************************************/
bool LowMemUsage()
{
   const long max_mem=(512<<20); // 512 MB
   long usage=0;
   IORead.  lock(); REPA(IORead)usage+=IORead.lockedElm(i).memUsage();
   IORead.unlock();
   SendFileObjs.  lock(); REPA(SendFileObjs)usage+=SendFileObjs.lockedElm(i).memUsage();
   SendFileObjs.unlock();
   return usage<max_mem;
}
/******************************************************************************/
