/******************************************************************************/
      Thread        CompressThread;
const COMPRESS_TYPE CompressionType=COMPRESS_LZMA;
const int           CompressionLevel=5;
/******************************************************************************/
bool CompressFunc(Thread &thread)
{
again:
   if(IORead.elms())
   {
      IOFileData file;
      {
         MemcThreadSafeLock lock(IORead);
         if(!IORead.elms())goto again; // element could've been removed
         Swap(file, IORead.lockedElm(0)); IORead.remove(0, true);
      }
      if(!file.error && Compressable(GetExt(file.name)))
      {
         File src, compressed;
         src.readMem(file.data.data(), file.data.elms());
         if(Compress(src, compressed.writeMem(src.size()/8), CompressionType, CompressionLevel))
            if(compressed.size()<file.data.elms())
         {
            file.compressed=true;
            file.data.setNum(compressed.size());
            compressed.pos(0); compressed.get(file.data.data(), file.data.elms());
         }
      }
      SendFileObjs.lock();
      REPA(SendFileObjs)
      {
         SendFileObj &sf=*SendFileObjs.lockedElm(i); if(sf.id==file.id)
         {
            sf.files.  lock(); Swap(sf.files.lockedNew(), file);
            sf.files.unlock();
            break;
         }
      }
      SendFileObjs.unlock();
   }else Time.wait(1);
   return true;
}
/******************************************************************************/
void StartCompress()
{
   CompressThread.create(CompressFunc);
}
/******************************************************************************/
