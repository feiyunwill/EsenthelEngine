/******************************************************************************/
class IOFile
{
   Str  name;
   int  uncompressed_size=0;
   long offset=0;
   UID  id;
   DateTime modify_time_utc;
   
   void set(C Str &name, C UID &id) {T.name=name; T.id=id;}
}
/******************************************************************************/
class IOFileData : IOFile
{
   Memc<byte> data; // use Memc because most likely this size will be decreased after compression
   bool       finished=false, error=false, compressed=false;

   uint memUsage()C {return data.memUsage();}
   
   bool load(File &f)
   {
      id.zero();
      f>>name;
      byte b=f.getByte();
      finished  =FlagTest(b, 1<<0);
      error     =FlagTest(b, 1<<1);
      compressed=FlagTest(b, 1<<2);
      f>>id.i[0];
      if(finished)f>>modify_time_utc;
      offset=f.decULongV();
      return offset>=0 && name.is() && !(b&~(1|2|4));
   }
   bool append(C Str &file_name, File &file_data)
   {
      Str temp_name=file_name+"@new"+id.i[0];
      if(!error)
      {
         FCreateDirs(GetPath(file_name));
         File temp; if(temp.appendTry(temp_name))if(temp.pos()==offset)
         {
            if(compressed){long pos=temp.pos();                if(Decompress(file_data, temp)){uncompressed_size=temp.pos()-pos; goto ok;}}
            else          {uncompressed_size=file_data.left(); if(file_data.copy(temp)       )                                   goto ok; }
         }
         error=true;
      ok:;
         temp.del();
         if(!error && finished)
         {
            FTimeUTC(temp_name, modify_time_utc);
            if(FExistSystem(file_name))FRecycle(file_name);
            error=!FRename(temp_name, file_name);
         }
      }
      if(finished || error){TempNames.binaryExclude(temp_name, ComparePathCI); if(error)FDelFile(temp_name);}
      else                  TempNames.binaryInclude(temp_name, ComparePathCI);
      return !error;
   }
}
MemcThreadSafe<IOFile    > IOToRead;
MemcThreadSafe<IOFileData>   IORead;
/******************************************************************************/
Thread IOThread;
/******************************************************************************/
bool IOFunc(Thread &thread)
{
again:
   if(IOToRead.elms() && LowMemUsage())
   {
      IOFileData fd;
      {
         MemcThreadSafeLock lock(IOToRead);
         if(!IOToRead.elms())goto again; // element could've been removed
         SCAST(IOFile, fd)=IOToRead.lockedElm(0); // copy without removing
      }
      File file;
      if(!file.readStdTry(fd.name))fd.error=true;else
      if(fd.offset>file.size())fd.error=true;else
      if(!file.pos(fd.offset))fd.error=true;else
      {
         fd.uncompressed_size=Min(32<<20, file.left()); // up to 32MB
         fd.data.setNum(fd.uncompressed_size);
         if(!file.get(fd.data.data(), fd.data.elms()))fd.error=true;else
         if(fd.finished=file.end())fd.modify_time_utc=FileInfoSystem(fd.name).modify_time_utc;
      }
      {
         MemcThreadSafeLock lock(IOToRead);
         if(IOToRead.elms()) // this element that's being processed, could've been removed
         {
            IOFile &elm=IOToRead.lockedElm(0); if(fd.id==elm.id && Equal(fd.name, elm.name, true)) // process only if it matches what we've obtained at the start
            {
               if(fd.finished || fd.error)IOToRead.remove(0, true);   // remove this element
               else                       elm.offset+=fd.data.elms(); // update this element to note that it was partly processed
            }
         }
      }
      IORead.  lock(); Swap(IORead.lockedNew(), fd);
      IORead.unlock();
   }else Time.wait(1);
   return true;
}
/******************************************************************************/
void StartIO()
{
   IOThread.create(IOFunc);
}
/******************************************************************************/
