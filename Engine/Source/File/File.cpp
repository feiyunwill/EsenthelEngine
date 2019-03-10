/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   'File._cipher_offset' is needed because of:
      -'Pak._cipher_per_file' can be either false or true
      -'File.limit/unlimit' is used for 'DecompressRaw' and 'ChunkReader' which will break if "Pak._cipher_per_file==true"
      -Android can have assets located within the APK with an offset (we can get a stdio file descriptor for APK and a custom file inside it at a certain offset in that file) which will break if "Pak._cipher_per_file==false"

   "Pak._cipher_per_file==true" is recommended because it improves Steam patching system:
      making updates will require smaller uploads/downloads, because Steam reuses same 1MB chunks:
      https://partner.steamgames.com/documentation/steampipe#Building_Efficient_Depots

   Because of buffering, the actual position (file descriptor) may be different than File.pos.
      Because in FILE_STD_READ  more data is read, and placed into the buffer     (descriptor being ahead of 'File.pos')
          and in FILE_STD_WRITE data is buffered, and written once buffer is full ('File.pos' being ahead of descriptor)
      See 'File.posFile'

/******************************************************************************
   
   BUF_SIZE was chosen based on speed tests using following code:

      Str file="d:/test.dat";
      if(MOBILE)file=SystemPath(SP_APP_DATA).tailSlash(true)+"test.dat";
      long size=(1024<<20); int mul=(MOBILE ? 1 : 4);

      File f; f.write(file); REP(mul)f.put(null, size); size*=mul;
      extern int BUF_SIZE;
      LogDel();
      ProcPriority(2);
      LogN(S+"file size:"+size);
      for(BUF_SIZE=4096; BUF_SIZE<=(16<<20); BUF_SIZE*=2)
      {
         f.del(); f.read(file);
         Str s=S+"BUF_SIZE:"+BUF_SIZE;
         byte temp[4096];
         flt t=Time.curTime(); f.pos(0); for(; !f.end(); )f.getByte(); s+=S+", 1:"+(Time.curTime()-t);
             t=Time.curTime(); f.pos(0); for(; !f.end(); )f.getUInt(); s+=S+", 4:"+(Time.curTime()-t);
             t=Time.curTime(); f.pos(0); for(; !f.end(); )f>>temp    ; s+=S+", 4k:"+(Time.curTime()-t);
         LogN(s);
      }

   Results on Asus Zenbook Prime, 15" 512GB SSD Win Laptop:
      file size:4294967296
      BUF_SIZE:4096,  1:56.978, 4:16.192, 4k:2.755
      BUF_SIZE:8192,  1:55.714, 4:15.340, 4k:2.754
      BUF_SIZE:16384,  1:55.269, 4:14.835, 4k:1.635
      BUF_SIZE:32768,  1:55.097, 4:14.684, 4k:1.404
      BUF_SIZE:65536,  1:54.998, 4:14.600, 4k:1.337 <- BEST
      BUF_SIZE:131072,  1:54.920, 4:14.636, 4k:1.416
      BUF_SIZE:262144,  1:54.976, 4:14.791, 4k:1.363
      BUF_SIZE:524288,  1:54.506, 4:14.718, 4k:1.354
      BUF_SIZE:1048576,  1:54.886, 4:14.592, 4k:1.361
      BUF_SIZE:2097152,  1:55.293, 4:14.513, 4k:1.364
      BUF_SIZE:4194304,  1:55.138, 4:14.704, 4k:1.523
      BUF_SIZE:8388608,  1:55.132, 4:14.898, 4k:1.715

   Results on MacBook Air 13" Mid 2013 128 GB SSD:
      file size:1073741824
      BUF_SIZE:4096,  1:22.485, 4:6.513, 4k:0.461
      BUF_SIZE:8192,  1:21.548, 4:6.062, 4k:0.461
      BUF_SIZE:16384,  1:21.118, 4:6.077, 4k:0.329
      BUF_SIZE:32768,  1:21.408, 4:5.797, 4k:0.340
      BUF_SIZE:65536,  1:21.656, 4:5.807, 4k:0.275 <- BEST
      BUF_SIZE:131072,  1:21.296, 4:5.955, 4k:0.331
      BUF_SIZE:262144,  1:21.188, 4:5.991, 4k:0.325
      BUF_SIZE:524288,  1:20.682, 4:6.036, 4k:0.321
      BUF_SIZE:1048576,  1:21.438, 4:6.011, 4k:0.342
      BUF_SIZE:2097152,  1:21.487, 4:5.918, 4k:0.368
      BUF_SIZE:4194304,  1:21.127, 4:5.932, 4k:0.414
      BUF_SIZE:8388608,  1:20.767, 4:6.236, 4k:0.428
      BUF_SIZE:16777216,  1:20.651, 4:5.961, 4k:0.425

   Results on iPad Mini 2:
      file size:1073741824
      BUF_SIZE:4096,  1:34.880, 4:11.120, 4k:6.909
      BUF_SIZE:8192,  1:34.407, 4:10.864, 4k:6.891
      BUF_SIZE:16384,  1:34.199, 4:10.639, 4k:6.811
      BUF_SIZE:32768,  1:34.171, 4:10.387, 4k:6.737
      BUF_SIZE:65536,  1:34.142, 4:10.139, 4k:6.747
      BUF_SIZE:131072,  1:34.137, 4:9.875, 4k:6.739
      BUF_SIZE:262144,  1:34.117, 4:9.860, 4k:6.738
      BUF_SIZE:524288,  1:34.292, 4:10.006, 4k:6.507
      BUF_SIZE:1048576,  1:34.184, 4:10.021, 4k:5.950
      BUF_SIZE:2097152,  1:36.769, 4:12.563, 4k:5.979
      BUF_SIZE:4194304,  1:38.040, 4:13.808, 4k:5.956
      BUF_SIZE:8388608,  1:38.799, 4:14.405, 4k:5.961
      BUF_SIZE:16777216,  1:38.953, 4:14.765, 4k:5.981

/******************************************************************************/
#if DESKTOP
   #define      BUF_SIZE (1<<16) // 64 KB
#else
   #define      BUF_SIZE (1<<15) // 32 KB (for Mobile devices use smaller buffers because they usually have less RAM)
#endif
   #define TEMP_BUF_SIZE (1<<17) // 128 KB

#define ALLOW_REFLUSH 1 // if allow secondary flush with remaining data when first failed
/******************************************************************************/
#if WINDOWS
static INLINE Long Seek(Int handle, Long offset, Int mode) {return _lseeki64(handle, offset, mode);}
#elif APPLE
static INLINE Long Seek(Int handle, Long offset, Int mode) {return  lseek   (handle, offset, mode);} // on Apple 'lseek' is already 64-bit
#elif LINUX || ANDROID || WEB
static INLINE Long Seek(Int handle, Long offset, Int mode) {return  lseek64 (handle, offset, mode);}
#endif

static INLINE Int Write(Int handle, CPtr data, Int size) {return PLATFORM(_write, write)(handle, data, size);}
static INLINE Int Read (Int handle,  Ptr data, Int size) {return PLATFORM(_read , read )(handle, data, size);}
/******************************************************************************/
// !! if adding any members here then adjust 'copyToAndDiscard', 'writeMemFixed' !!
void File::zeroNoBuf()
{
  _type=FILE_NONE; _writable=false; _ok=true; _path=FILE_PATH(0);
  _buf_pos=_buf_len=_cipher_offset=_handle=0;
  _offset=_pos=_size=0;
  _pak=null;
  _cipher=null;
  _mem=null;
#if ANDROID
  _aasset=null;
#endif
}
void File::zero()
{
   zeroNoBuf();
  _buf_size=0;
  _buf=null;
}

File::File(                                            )          {zero();}
File::File(C Str     &name             , Cipher *cipher) : File() {read   (name      , cipher);}
File::File(C UID     &id               , Cipher *cipher) : File() {read   (id        , cipher);}
File::File(C Str     &name, C Pak &pak                 ) : File() {read   (name, pak         );}
File::File(C PakFile &file, C Pak &pak                 ) : File() {read   (file, pak         );}
File::File( CPtr      data,   Int  size, Cipher *cipher) : File() {readMem(data, size, cipher);}

void File::delBuf(        ) {Free(_buf); _buf_size=0;}
Bool File::setBuf(Int size) {if(size>_buf_size){delBuf(); if(_buf=Alloc(size))_buf_size=size;} return !size || _buf!=null;}

File& File::del()
{
   close(); delBuf(); // !! delete buffer after closing, because it may be used by 'flush' !!
#if !FILE_MEMB_UNION
  _memb.del();
#endif
   return T;
}
void File::close()
{
   switch(_type)
   {
      case FILE_STD_READ :
      case FILE_STD_WRITE:
      {
         flush(); // remember that this 'flush' may fail, and '_buf_len' may remain >0, however it will be cleared in 'zeroNoBuf'
       ::PLATFORM(_close, close)(_handle);
      #if WEB
         if(_writable)FlushIO(); // check for '_writable' instead of 'FILE_STD_WRITE' because File could have been created as writable but later switched to FILE_STD_READ
      #endif
      }break;

      case FILE_MEM: if(_writable)Free(_mem); break; // write memory was dynamically allocated

      case FILE_MEMB: if(FILE_MEMB_UNION)DTOR(_memb);else _memb.clear(); break;
   }
#if ANDROID
   if(_aasset)AAsset_close((AAsset*)_aasset);
#endif
   zeroNoBuf();
}

File& File::reset()
{
   if(_type==FILE_MEMB)
   {
     _memb.reset();
     _ok=true;
     _cipher_offset=0;
     _offset=_pos=_size=0;
   }
   return T;
}

File& File::append (C Str     &name                , Cipher  *cipher) {if(!appendTry   (name      , cipher))Exit(MLT(S+"Can't append \""+          name+'"', PL,S+u"Nie można nadpisać \""+          name+'"')); return T;}
File& File::write  (C Str     &name                , Cipher  *cipher) {if(! writeTry   (name      , cipher))Exit(MLT(S+"Can't create \""+          name+'"', PL,S+u"Nie można utworzyć \""+          name+'"')); return T;}
File& File::readStd(C Str     &name                , Cipher  *cipher) {if(!  readStdTry(name      , cipher))Exit(MLT(S+"Can't open \""  +          name+'"', PL,S+u"Nie można otworzyć \""+          name+'"')); return T;}
File& File::read   (C PakFile &file, C Pak    &pak                  ) {if(!  readTry   (file, pak         ))Exit(MLT(S+"Can't open \""  +     file.name+'"', PL,S+u"Nie można otworzyć \""+     file.name+'"')); return T;}
File& File::read   (C Str     &name, C Pak    &pak                  ) {if(!  readTry   (name, pak         ))Exit(MLT(S+"Can't open \""  +          name+'"', PL,S+u"Nie można otworzyć \""+          name+'"')); return T;}
File& File::read   (C UID     &id  , C Pak    &pak                  ) {if(!  readTry   (id  , pak         ))Exit(MLT(S+"Can't open \""  +id.asCString()+'"', PL,S+u"Nie można otworzyć \""+id.asCString()+'"')); return T;}
File& File::read   (C Str     &name, C PakSet &paks                 ) {if(!  readTry   (name, paks        ))Exit(MLT(S+"Can't open \""  +          name+'"', PL,S+u"Nie można otworzyć \""+          name+'"')); return T;}
File& File::read   (C UID     &id  , C PakSet &paks                 ) {if(!  readTry   (id  , paks        ))Exit(MLT(S+"Can't open \""  +id.asCString()+'"', PL,S+u"Nie można otworzyć \""+id.asCString()+'"')); return T;}
File& File::read   (C Str     &name                , Cipher  *cipher) {if(!  readTry   (name      , cipher))Exit(MLT(S+"Can't open \""  +          name+'"', PL,S+u"Nie można otworzyć \""+          name+'"')); return T;}
File& File::read   (C UID     &id                  , Cipher  *cipher) {if(!  readTry   (id        , cipher))Exit(MLT(S+"Can't open \""  +id.asCString()+'"', PL,S+u"Nie można otworzyć \""+id.asCString()+'"')); return T;}

Bool File::copyToAndDiscard(Mems<Byte> &dest)
{
   if(T._type==FILE_MEM && T._writable && !_cipher && posAbs()==0) // check if we can just swap memories
   {  // swap memories, perhaps 'File' may reuse 'Mems' data for future operations
      Ptr data=dest.data(); Int elms=dest.elms(); dest.setTemp((Byte*)T._mem, T._size);
      T._ok           =true;
      T._cipher_offset=0;
      T._offset       =0;
      T._pos          =0;
      T._size         =elms;
      T._cipher       =null;
      T._mem          =data;
      return true;
   }else
   {
      dest.setNum(left());
      return getFast(dest.data(), dest.elms());
   }
}
File& File::writeMemFixed(Int size, Cipher *cipher)
{
   if(T._size==size && T._type==FILE_MEM && T._writable) // check if we can reuse existing memory
   {
      T._ok           =true;
      T._cipher_offset=0;
      T._offset       =0;
      T._pos          =0;
      T._cipher       =cipher;
   }else
   {
      close();
      if(!size || (_mem=Alloc(size)))
      {
         T._type    =FILE_MEM;
         T._writable=true;
         T._size    =size;
         T._cipher  =cipher;
      }
   }
   return T;
}
File& File::writeMem(UInt block_elms, Cipher *cipher)
{
   if(T._type==FILE_MEMB && _memb.blockElms()==block_elms/* && T._writable - MEMB is always writable*/)
   {
      reset();
      T._cipher=cipher;
   }else
   {
      close();
      T._type    =FILE_MEMB;
      T._writable=true;
      T._cipher  =cipher;
   #if FILE_MEMB_UNION
      CTOR(T._memb, block_elms);
   #else
      if(_memb.blockElms()!=block_elms)
      {
         DTOR(_memb);
         CTOR(_memb, block_elms);
      }
   #endif
   }
   return T;
}
File& File::readMem(CPtr data, Int size, Cipher *cipher)
{
   close();
   if(size>=0)
   {
      T._type    =FILE_MEM;
      T._writable=false;
      T._mem     =(Ptr)data;
      T._size    =size;
      T._cipher  =cipher;
   }
   return T;
}
/******************************************************************************/
#if WINDOWS
Bool File::appendTry     (C Str &name, Cipher *cipher) {close(); if(name.is() && !_wsopen_s(&_handle, name, _O_BINARY|_O_RDWR|_O_CREAT         , _SH_DENYWR, _S_IREAD|_S_IWRITE)){SetLastError(0); _type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR; _pos=_size=Max(0, _filelengthi64(_handle)); Seek(_handle, 0, SEEK_END); _cipher=cipher; return true;} _type=FILE_NONE; _close(_handle); _handle=0;} return false;} // clear error 183 file already exists
Bool File:: writeTry     (C Str &name, Cipher *cipher) {close(); if(name.is() && !_wsopen_s(&_handle, name, _O_BINARY|_O_RDWR|_O_CREAT|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE)){SetLastError(0); _type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR;                                                                         _cipher=cipher; return true;} _type=FILE_NONE; _close(_handle); _handle=0;} return false;} // clear error 183 file already exists
Bool File::  readStdTryEx(C Str &name, Cipher *cipher, UInt max_buf_size)
{
   close(); if(name.is())
   {
      if(!_wsopen_s(&_handle, name, _O_BINARY|_O_RDONLY, _SH_DENYNO, _S_IREAD|_S_IWRITE))
      {
        _type=FILE_STD_READ; _size=Max(0, _filelengthi64(_handle)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_CUR; _cipher=cipher; return true;}
        _type=FILE_NONE    ; _close(_handle); _handle=0; _size=0; return false;
      }
      if(DataPath().is() && !FullPath(name))
      {
         Char path_name[MAX_LONG_PATH]; MergePath(path_name, DataPath(), name);
         if(!_wsopen_s(&_handle, WChar(path_name), _O_BINARY|_O_RDONLY, _SH_DENYNO, _S_IREAD|_S_IWRITE))
         {
           _type=FILE_STD_READ; _size=Max(0, _filelengthi64(_handle)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_DATA; _cipher=cipher; return true;}
           _type=FILE_NONE    ; _close(_handle); _handle=0; _size=0; return false;
         }
      }
   }
   return false;
}
#elif APPLE
static void DetectSymLink(File &f, C Str8 &unix_name)
{
   struct stat stats; if(!fstat(f._handle, &stats))if(S_ISLNK(stats.st_mode))
   {
      ASSERT(BUF_SIZE>=MAX_UTF_PATH); // we're reading directly to the File buffer, so make sure that it was set big enough to hold a link path
      ssize_t read=readlink(unix_name, (char*)f._buf, Min(f._buf_size, f.size())); if(read>0)f._buf_len=Int(read);
   }
}
Bool File::appendTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open(UnixPathUTF8(name), O_RDWR|O_CREAT|        O_EXLOCK|O_NONBLOCK|O_SYMLINK, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR; _pos=_size=Max(0, Seek(_handle, 0, SEEK_END)); _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File:: writeTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open(UnixPathUTF8(name), O_RDWR|O_CREAT|O_TRUNC|O_EXLOCK|O_NONBLOCK|O_SYMLINK, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR;                                                _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File::  readStdTryEx(C Str &name, Cipher *cipher, UInt max_buf_size)
{
   close(); if(name.is())
   {
      Str8 unix_name=UnixPathUTF8(name);
     _handle=open(unix_name, O_RDONLY|O_NONBLOCK|O_SYMLINK); if(_handle>=0)
      {
        _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_CUR; Seek(_handle, 0, SEEK_SET); _cipher=cipher; DetectSymLink(T, unix_name); return true;}
        _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
      }
      if(DataPath().is() && !FullPath(name))
      {
         Char path_name[MAX_LONG_PATH]; MergePath(path_name, DataPath(), name);
         unix_name=UnixPathUTF8(path_name);
        _handle=open(unix_name, O_RDONLY|O_NONBLOCK|O_SYMLINK); if(_handle>=0)
         {
           _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_DATA; Seek(_handle, 0, SEEK_SET); _cipher=cipher; DetectSymLink(T, unix_name); return true;}
           _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
         }
      }
   }
   return false;
}
#elif LINUX || WEB
Bool File::appendTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open64(UnixPathUTF8(name), O_RDWR|O_CREAT|        O_NONBLOCK, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR; _pos=_size=Max(0, Seek(_handle, 0, SEEK_END)); _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File:: writeTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open64(UnixPathUTF8(name), O_RDWR|O_CREAT|O_TRUNC|O_NONBLOCK, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR;                                                _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File::  readStdTryEx(C Str &name, Cipher *cipher, UInt max_buf_size)
{
   close(); if(name.is())
   {
      Str8 unix_name=UnixPathUTF8(name);
     _handle=open64(unix_name, O_RDONLY|O_NONBLOCK); if(_handle>=0)
      {
        _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_CUR; Seek(_handle, 0, SEEK_SET); _cipher=cipher; return true;}
        _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
      }
      if(DataPath().is() && !FullPath(name))
      {
         Char path_name[MAX_LONG_PATH]; MergePath(path_name, DataPath(), name);
         unix_name=UnixPathUTF8(path_name);
        _handle=open64(unix_name, O_RDONLY|O_NONBLOCK); if(_handle>=0)
         {
           _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_DATA; Seek(_handle, 0, SEEK_SET); _cipher=cipher; return true;}
           _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
         }
      }
   }
   return false;
}
#elif ANDROID // Android has 'open64' only on API 21 and newer, however it's the same as 'open' with O_LARGEFILE
Bool File::appendTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open(UnixPathUTF8(name), O_RDWR|O_CREAT|        O_NONBLOCK|O_LARGEFILE, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR; _pos=_size=Max(0, Seek(_handle, 0, SEEK_END)); _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File:: writeTry     (C Str &name, Cipher *cipher) {close(); if(name.is()){_handle=open(UnixPathUTF8(name), O_RDWR|O_CREAT|O_TRUNC|O_NONBLOCK|O_LARGEFILE, S_IRWXU|S_IRWXG|S_IRWXO); if(_handle>=0){_type=FILE_STD_WRITE; if(setBuf(BUF_SIZE)){_writable=true; _path=FILE_CUR;                                                _cipher=cipher; return true;} _type=FILE_NONE; ::close(_handle); _handle=0;}} return false;}
Bool File::  readStdTryEx(C Str &name, Cipher *cipher, UInt max_buf_size, Bool *processed)
{
   if(processed)*processed=false;
   close(); if(name.is())
   {
      // assets
      if(!FullPath(name) && AssetManager)
         if(AAsset *asset=AAssetManager_open(AssetManager, UnixPathUTF8(name), AASSET_MODE_RANDOM))
      {
         Bool ok=false;
      #if __ANDROID_API__>=13
         off64_t size=AAsset_getLength64(asset);
      #else
         off_t   size=AAsset_getLength  (asset);
      #endif
         if( size<0)ok=false;else
         if(!size  ) // no need for opening the file using Android API when it has no size
         {
            T._type    =FILE_MEM;
            T._writable=false;
            T._size    =0;
            T._cipher  =cipher;
            ok=true;
         }else // size>0
         {
         #if __ANDROID_API__>=13
            off64_t offset, length; int fd=AAsset_openFileDescriptor64(asset, &offset, &length);
         #else
            off_t   offset, length; int fd=AAsset_openFileDescriptor  (asset, &offset, &length);
         #endif
            if(fd>=0)
            {
              _size=length;
               if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size))))
               {
                  T._handle  =fd;
                  T._type    =FILE_STD_READ;
                  T._writable=false;
                  T._path    =FILE_ANDROID_ASSET;
                  T._cipher  =cipher;
                  Seek(_handle, T._offset=offset, SEEK_SET);
                  ok=true;
               }else
               {
                 _size=0;
                ::close(fd);
               }
            }else // file is compressed, needs to be processed using 'AAsset_*' functions
            if(_mem=(Ptr)AAsset_getBuffer(asset))
            {
               if(processed)*processed=true; // file had to be decompressed
               T._type    =FILE_MEM;
               T._writable=false;
               T._path    =FILE_ANDROID_ASSET;
               T._size    =size;
               T._cipher  =cipher;
               T._aasset  =asset;
               return true; // return here and don't close 'asset' because that would make the '_mem' buffer invalid
            }
         }
         AAsset_close(asset);
         return ok;
      }

      // original path
     _handle=open(UnixPathUTF8(name), O_RDONLY|O_NONBLOCK|O_LARGEFILE); if(_handle>=0)
      {
        _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_CUR; Seek(_handle, 0, SEEK_SET); _cipher=cipher; return true;}
        _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
      }

      // data path
      if(DataPath().is() && !FullPath(name))
      {
         Char path_name[MAX_LONG_PATH]; MergePath(path_name, DataPath(), name);
        _handle=open(UnixPathUTF8(path_name), O_RDONLY|O_NONBLOCK|O_LARGEFILE); if(_handle>=0)
         {
           _type=FILE_STD_READ; _size=Max(0, Seek(_handle, 0, SEEK_END)); if(setBuf(Min(max_buf_size, Min(BUF_SIZE, _size)))){_writable=false; _path=FILE_DATA; Seek(_handle, 0, SEEK_SET); _cipher=cipher; return true;}
           _type=FILE_NONE    ; ::close(_handle); _handle=0; _size=0; return false;
         }
      }
   }
   return false;
}
#endif
/******************************************************************************/
Bool File::readTryRaw(C PakFile &file, C Pak &pak)
{
   if(!(file.flag&PF_REMOVED))switch(pak._file_type)
   {
      case FILE_MEM:
      {
         readMem(pak._data, file.data_size_compressed, pak._file_cipher);
         T._pak          =&pak;
         T._offset       = pak._data_offset+file.data_offset;
         T._cipher_offset=(pak._cipher_per_file ? 0 : pak._file_cipher_offset+file.data_offset);
      }return true;

      case FILE_STD_READ:
      {
         if(T._type==FILE_STD_READ && !T._writable && T._pak==&pak) // this 'File' object already points to some 'PakFile' in the same 'Pak' as requested, which means that it's using the same stdio file, and we can re-use it
         {
           _offset=pak._data_offset+file.data_offset;
            if(Seek(_handle, _offset, SEEK_SET)==_offset)
            {
               T._size=file.data_size_compressed;
               if(setBuf(Min(BUF_SIZE, T._size)))
               {
                  clearBuf();
                  T._ok           =true;
                  T._pos          =0;
                  T._cipher       =pak._file_cipher; // re-apply cipher because this could have been changed in 'readTryEx'
                  T._cipher_offset=(pak._cipher_per_file ? 0 : pak._file_cipher_offset+file.data_offset);
                  return true;
               }
            }
         }else
         if(readStdTryEx(pak.pakFileName(), pak._file_cipher, file.data_size_compressed))
         {
           _offset=pak._data_offset+file.data_offset;
            if(Seek(_handle, _offset, SEEK_SET)==_offset)
            {
               T._pak          =&pak;
               T._size         =file.data_size_compressed;
               T._cipher_offset=(pak._cipher_per_file ? 0 : pak._file_cipher_offset+file.data_offset);
               return true;
            }
         }
      }break;
   }
   close(); return false;
}
Bool File::readTryEx(C PakFile &file, C Pak &pak, Cipher *cipher, Bool *processed)
{
   if(readTryRaw(file, pak))
   {
      Bool p=false;
      if(file.compression) // first decompress with original cipher
      {
         p=true; // we had to perform processing
         File temp; if(!DecompressRaw(T, temp, file.compression, file.data_size_compressed, file.data_size, true))goto error;
         Swap(T, temp); pos(0);
      }
      if(cipher) // if we want to use custom cipher on top of what's already available (don't check for 'cipher!=T._cipher' because we want to add new cipher on top of existing)
      {
         if(T._cipher) // get rid of existing cipher by decrypting it to a memory file
         {
            p=true; // we had to perform processing
            File temp; if(!copy(temp.writeMemFixed(size(), null)))goto error; // we can't apply 'cipher' here and later clear it, because that would encrypt it with 'cipher', however what we want is to decrypt it with 'cipher'
            Swap(T, temp); pos(0);
         }else _cipher_offset=0; // if there's no cipher yet, but there was '_cipher_offset', then we need to clear it, as it would affect the new cipher
         T._cipher=cipher; // here we can just set '_cipher' instead of calling 'cipher' method, because in both cases (decompress+decrypt) we're writing to memory files so these operations are not needed
      }

      // this method calls 'readTryRaw' which then calls 'readStdTryEx' which on Android can set 'processed' to true, however not in this case, because we're loading a 'PakFile' from 'Pak', and if the File for the 'Pak' had to be processed, then 'Pak' itself decompressed it into a memory based 'File' FILE_MEM inside 'Pak.load'

      if(processed)*processed=p; return true;

   error:
      close();
   }//else close(); no need to call 'close' here, because 'readTryRaw' will already call it
   if(processed)*processed=false; return false;
}

Bool File::readTryEx(C Str &name, C PakSet &paks, Cipher *cipher, Bool *processed)
{
 //if(name.is())) in most cases the 'name' is going to be specified
   {
      SafeSyncLockerEx locker(paks._lock); // required if we call 'File.read' and 'PakSet.add' in multiple threads simultaneously
      if(C PaksFile *file=paks.find(name))
      {
         Pak     &pak=*file->pak ;
       C PakFile &pf =*file->file;
         locker.off(); // now when references have been copied, we can unlock

         return readTryEx(pf, pak, cipher, processed);
      }
   }
   close(); if(processed)*processed=false; return false;
}
Bool File::readTryEx(C UID &id, C PakSet &paks, Cipher *cipher, Bool *processed)
{
 //if(id.valid())) in most cases the 'id' is going to be specified
   {
      SafeSyncLockerEx locker(paks._lock); // required if we call 'File.read' and 'PakSet.add' in multiple threads simultaneously
      if(C PaksFile *file=paks.find(id))
      {
         Pak     &pak=*file->pak ;
       C PakFile &pf =*file->file;
         locker.off(); // now when references have been copied, we can unlock

         return readTryEx(pf, pak, cipher, processed);
      }
   }
   close(); if(processed)*processed=false; return false;
}

Bool File::readTryEx(C Str &name, Cipher *cipher, Bool *processed)
{
                                      if(readTryEx   (name, Paks, cipher, processed))return true;
#if !ANDROID
   if(processed)*processed=false; return readStdTryEx(name, cipher);
#else
                                  return readStdTryEx(name, cipher, UINT_MAX, processed);
#endif
}
Bool File::readTryEx(C UID &id, Cipher *cipher, Bool *processed)
{
                                      if(readTryEx   (                id , Paks, cipher, processed))return true;
#if !ANDROID
   if(processed)*processed=false; return readStdTryEx(_EncodeFileName(id), cipher);
#else
                                  return readStdTryEx(_EncodeFileName(id), cipher, UINT_MAX, processed);
#endif
}

Bool File::readStdTry(C Str     &name,   Cipher *cipher) {return readStdTryEx(name,       cipher);}
Bool File::readTry   (C Str     &name,   Cipher *cipher) {return readTryEx   (name,       cipher, null);}
Bool File::readTry   (C UID     &id  ,   Cipher *cipher) {return readTryEx   (id  ,       cipher, null);}
Bool File::readTry   (C Str     &name, C PakSet &paks  ) {return readTryEx   (name, paks, null  , null);}
Bool File::readTry   (C UID     &id  , C PakSet &paks  ) {return readTryEx   (id  , paks, null  , null);}
Bool File::readTry   (C PakFile &file, C Pak    &pak   ) {return readTryEx   (file, pak , null  , null);}

Bool File::readTry(C Str &name, C Pak &pak)
{
   if(C PakFile *file=pak.find(name, false))return readTry(*file, pak);
   close(); return false;
}
Bool File::readTry(C UID &id, C Pak &pak)
{
   if(C PakFile *file=pak.find(id, false))return readTry(*file, pak);
   close(); return false;
}
/******************************************************************************/
Ptr File::mem()
{
   switch(_type)
   {
      case FILE_MEM: return memFast();
   }
   return null;
}

Long File::posFile()C
{
   switch(_type)
   {
      default            : return pos();
      case FILE_STD_READ : return pos()+_buf_len; // in read  mode, the actual position is ahead
      case FILE_STD_WRITE: return pos()-_buf_len; // in write mode, the actual position is behind
   }
}

Bool File::pos(Long pos)
{
   if(pos==T._pos)return true;
   if(pos>=0)switch(_type)
   {
      case FILE_STD_READ:
      {
         {
            Long delta =pos-T._pos;
            if(  delta<=_buf_len  // we skip  forward and we still have    data in the buffer
            &&  -delta<=_buf_pos) // we skip backward and we have previous data in the buffer
            {
              _buf_len-=delta;
              _buf_pos+=delta;
               T._pos=pos; return true; // don't seek (because file position is already in order with the buffer)
            }
         }
         clearBuf(); // if we're going to seek then we need to clear the buffer if any

      seek:      Long abs_pos=Seek(_handle, _offset+pos, SEEK_SET); if(abs_pos<0)return false;
              T._pos =abs_pos-_offset; // set position to what was actually set, in case it's different than what requested
      }return T._pos==pos;

      case FILE_STD_WRITE:
      {
         if(_buf_len && !flushDo())
         {
            //return false; alternative approach is to just return false, however since 'pos' was called, then we take it with higher priority, and try to do what was requested
         #if ALLOW_REFLUSH
           _ok=false; // error occurred
            clearBuf(); // if we're allowing re-flush then it means some data could be left in the buffer, however because we're seeking, then we need to always discard it, so it's not saved at a different position
         #endif
         }
      }goto seek;

      case FILE_MEM :
      case FILE_MEMB: T._pos=pos; return true;
   }
   return false;
}
/******************************************************************************/
// COMPRESS / DECOMPRESS
/******************************************************************************/
File& File::cmpIntV(Int i)
{
   Byte data[MaxCmpUIntVSize]; Int pos=0;
   Bool negative=(i<0); UInt u=(negative ? -(i+1) : i);
   data[pos++]=((u&63)|(negative<<6)|((u>=64)<<7)); // 0
   if(u>=64)
   {
      u>>=6; data[pos++]=((u&127)|((u>=128)<<7)); // 1
      if(u>=128)
      {
         u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 2
         if(u>=128)
         {
            u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 3
            if(u>=128)
            {
               u>>=7; data[pos++]=(u); // 4
            }
         }
      }
   }
   put(data, pos);
   return T;
}
File& File::decIntV(Int &i)
{
   Byte v; T>>v;
   Bool negative=((v>>6)&1);
   UInt u=(v&63);
   if(v&128)
   {
      T>>v; u|=((v&127)<<6);
      if(v&128)
      {
         T>>v; u|=((v&127)<<(6+7));
         if(v&128)
         {
            T>>v; u|=((v&127)<<(6+7+7));
            if(v&128)
            {
               T>>v; u|=(v<<(6+7+7+7));
            }
         }
      }
   }
   i=(negative ? -1-Int(u) : Int(u));
   return T;
}

File& File::cmpUIntV(UInt u)
{
   Byte data[MaxCmpUIntVSize]; Int pos=0;
   data[pos++]=((u&127)|((u>=128)<<7));
   if(u>=128)
   {
      u>>=7; data[pos++]=((u&127)|((u>=128)<<7));
      if(u>=128)
      {
         u>>=7; data[pos++]=((u&127)|((u>=128)<<7));
         if(u>=128)
         {
            u>>=7; data[pos++]=((u&127)|((u>=128)<<7));
            if(u>=128)
            {
               u>>=7; data[pos++]=(u);
            }
         }
      }
   }
   put(data, pos);
   return T;
}
File& File::decUIntV(UInt &u)
{
   Byte v; T>>v; u=(v&127);
   if(v&128)
   {
      T>>v; u|=((v&127)<<7);
      if(v&128)
      {
         T>>v; u|=((v&127)<<(7+7));
         if(v&128)
         {
            T>>v; u|=((v&127)<<(7+7+7));
            if(v&128)
            {
               T>>v; u|=(v<<(7+7+7+7));
            }
         }
      }
   }
   return T;
}

File& File::cmpLongV(Long l)
{
   Byte data[MaxCmpULongVSize]; Int pos=0;
   Bool negative=(l<0); ULong u=(negative ? -(l+1) : l);
   data[pos++]=((u&63)|(negative<<6)|((u>=64)<<7)); // 0
   if(u>=64)
   {
      u>>=6; data[pos++]=((u&127)|((u>=128)<<7)); // 1
      if(u>=128)
      {
         u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 2
         if(u>=128)
         {
            u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 3
            if(u>=128)
            {
               u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 4
               if(u>=128)
               {
                  u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 5
                  if(u>=128)
                  {
                     u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 6
                     if(u>=128)
                     {
                        u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 7
                        if(u>=128)
                        {
                           u>>=7; data[pos++]=(u); // 8
                        }
                     }
                  }
               }
            }
         }
      }
   }
   put(data, pos);
   return T;
}
File& File::decLongV(Long &l)
{
   Byte  v; T>>v;
   Bool  negative=((v>>6)&1);
   ULong u=(v&63);
   if(v&128)
   {
      T>>v; u|=((v&127)<<6);
      if(v&128)
      {
         T>>v; u|=((v&127)<<(6+7));
         if(v&128)
         {
            T>>v; u|=((v&127)<<(6+7+7));
            if(v&128)
            {
               T>>v; u|=(U64(v&127)<<(6+7+7+7)); // << 27 requires U64
               if(v&128)
               {
                  T>>v; u|=(U64(v&127)<<(6+7+7+7+7));
                  if(v&128)
                  {
                     T>>v; u|=(U64(v&127)<<(6+7+7+7+7+7));
                     if(v&128)
                     {
                        T>>v; u|=(U64(v&127)<<(6+7+7+7+7+7+7));
                        if(v&128)
                        {
                           T>>v; u|=(U64(v)<<(6+7+7+7+7+7+7+7));
                        }
                     }
                  }
               }
            }
         }
      }
   }
   l=(negative ? -1-Long(u) : Long(u));
   return T;
}

File& File::cmpULongVMax(ULong u)
{
   Byte   data[MaxCmpULongVSize];
          data[0]=((u&127)|(1<<7)); // 0
   u>>=7; data[1]=((u&127)|(1<<7)); // 1
   u>>=7; data[2]=((u&127)|(1<<7)); // 2
   u>>=7; data[3]=((u&127)|(1<<7)); // 3
   u>>=7; data[4]=((u&127)|(1<<7)); // 4
   u>>=7; data[5]=((u&127)|(1<<7)); // 5
   u>>=7; data[6]=((u&127)|(1<<7)); // 6
   u>>=7; data[7]=((u&127)|(1<<7)); // 7
   u>>=7; data[8]=( u            ); // 8
   put(data);
   return T;
}
File& File::cmpULongV(ULong u)
{
   Byte data[MaxCmpULongVSize]; Int pos=0;
   data[pos++]=((u&127)|((u>=128)<<7)); // 0
   if(u>=128)
   {
      u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 1
      if(u>=128)
      {
         u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 2
         if(u>=128)
         {
            u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 3
            if(u>=128)
            {
               u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 4
               if(u>=128)
               {
                  u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 5
                  if(u>=128)
                  {
                     u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 6
                     if(u>=128)
                     {
                        u>>=7; data[pos++]=((u&127)|((u>=128)<<7)); // 7
                        if(u>=128)
                        {
                           u>>=7; data[pos++]=(u); // 8
                        }
                     }
                  }
               }
            }
         }
      }
   }
   put(data, pos);
   return T;
}
File& File::decULongV(ULong &u)
{
   Byte v; T>>v; u=(v&127);
   if(v&128)
   {
      T>>v; u|=((v&127)<<7);
      if(v&128)
      {
         T>>v; u|=((v&127)<<(7+7));
         if(v&128)
         {
            T>>v; u|=((v&127)<<(7+7+7));
            if(v&128)
            {
               T>>v; u|=(U64(v&127)<<(7+7+7+7)); // << 28 requires U64
               if(v&128)
               {
                  T>>v; u|=(U64(v&127)<<(7+7+7+7+7));
                  if(v&128)
                  {
                     T>>v; u|=(U64(v&127)<<(7+7+7+7+7+7));
                     if(v&128)
                     {
                        T>>v; u|=(U64(v&127)<<(7+7+7+7+7+7+7));
                        if(v&128)
                        {
                           T>>v; u|=(U64(v)<<(7+7+7+7+7+7+7+7));
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return T;
}

File& File::cmpFlt3cm(C Flt &r) {UInt u=Mid(Round(r*100)+8388608, 0, 16777216-1); put(&u, 3); ASSERT(SIZE(u)>=3); return T;}
File& File::decFlt3cm(  Flt &r) {Int  i=0; get(&i, 3); r=(i-8388608)/100.0f;                  ASSERT(SIZE(i)>=3); return T;}

File& File::cmpSatFlt1(C Flt &r) {  putByte  ( FltToByte(r)       ); return T;}
File& File::cmpSatFlt2(C Flt &r) {  putUShort(RoundU(Sat(r)*65535)); return T;} // it's better to clamp flt for bigger values
File& File::decSatFlt1(  Flt &r) {r=getByte  ()/  255.0f;            return T;}
File& File::decSatFlt2(  Flt &r) {r=getUShort()/65535.0f;            return T;}

File& File::cmpAngle1(C Flt &r) {  putByte  (RoundU(AngleFull(r)*(  256/PI2))); return T;} // use 'AngleFull' to make angle smaller and thus 'RoundPos' work with better precision, there's no need for 'Mid' or "&" because 1) AngleFull already sets 0..PI2 range, 2) putByte   takes only  Byte  as param
File& File::cmpAngle2(C Flt &r) {  putUShort(RoundU(AngleFull(r)*(65536/PI2))); return T;} // use 'AngleFull' to make angle smaller and thus 'RoundPos' work with better precision, there's no need for 'Mid' or "&" because 1) AngleFull already sets 0..PI2 range, 2) putUShort takes only UShort as param
File& File::decAngle1(  Flt &r) {r=getByte  ()*(PI2/  256);                     return T;}
File& File::decAngle2(  Flt &r) {r=getUShort()*(PI2/65536);                     return T;}

File& File::cmpDir2(C Vec &v)
{
   Byte x=Mid(RoundPos(v.x*127+128), 0, 255), // 8-bits for X
        y=Mid(RoundPos(v.y* 63+ 64), 0, 127); // 7-bits for Y
   Bool z=(v.z<0);                            // 1-bit  for Z sign
   putUShort(x|(y<<8)|(z<<15));
   return T;
}
File& File::cmpDir3(C Vec &v)
{
   UInt x=Mid(RoundPos(v.x*2047+2048), 0, 4095), // 12-bits for X
        y=Mid(RoundPos(v.y*1023+1024), 0, 2047); // 11-bits for Y
   Bool z=(v.z<0);                               //  1-bit  for Z sign
   UInt c=(x|(y<<12)|(z<<23)); put(&c, 3); ASSERT(SIZE(c)>=3);
   return T;
}
File& File::decDir2(Vec &v)
{
   UShort c=getUShort();
   Int    x=((c    )&255),
          y=((c>> 8)&127);
   Bool   z=((c>>15)&  1);
   v.x=(x-128)/127.0f;
   v.y=(y- 64)/ 63.0f;
   v.z=CalcZ(v.xy); if(z)CHS(v.z);
   v.normalize();
   return T;
}
File& File::decDir3(Vec &v)
{
   UInt c; get(&c, 3); ASSERT(SIZE(c)>=3); // clearing 'c' to zero first is not needed, since we're reading bits only from first 3 bytes below
    Int x=((c    )&4095),
        y=((c>>12)&2047);
   Bool z=((c>>23)&   1);
   v.x=(x-2048)/2047.0f;
   v.y=(y-1024)/1023.0f;
   v.z=CalcZ(v.xy); if(z)CHS(v.z);
   v.normalize();
   return T;
}

File& File::cmpOrient2(C Matrix3 &m)
{
   Vec  angles=m.angles();
   Byte x=(Round(angles.x*(32/PI2))&31), // 5-bits for X
        y=(Round(angles.y*(64/PI2))&63), // 6-bits for Y (because most objects are on ground, and just rotated along Y axis, so give more precision to that axis)
        z=(Round(angles.z*(32/PI2))&31); // 5-bits for Z
   putUShort(x|(y<<5)|(z<<11));
   return T;
}
File& File::cmpOrient3(C Matrix3 &m)
{
   Vec  angles=m.angles();
   Byte b[]={Byte(Round(angles.x*(256/PI2))), Byte(Round(angles.y*(256/PI2))), Byte(Round(angles.z*(256/PI2)))};
   T<<b;
   return T;
}
File& File::cmpOrient4(C Matrix3 &m)
{
   Vec  angles=m.angles();
   UInt x=(Round(angles.x*(2048/PI2))&2047), // 11-bits for X
        y=(Round(angles.y*(2048/PI2))&2047), // 11-bits for Y
        z=(Round(angles.z*(1024/PI2))&1023); // 10-bits for Z
   putUInt(x|(y<<11)|(z<<22));
   return T;
}
File& File::decOrient2(Matrix3 &m)
{
   UShort c=getUShort();
   Byte   x=( c     &31),
          y=((c>> 5)&63),
          z=( c>>11    );
   m.setRotateZ(z*(PI2/32)).rotateXY(x*(PI2/32), y*(PI2/64));
   return T;
}
File& File::decOrient3(Matrix3 &m)
{
   Byte b[3]; T>>b;
   m.setRotateZ(b[2]*(PI2/256)).rotateXY(b[0]*(PI2/256), b[1]*(PI2/256));
   return T;
}
File& File::decOrient4(Matrix3 &m)
{
   UInt c=getUInt(),
        x=( c     &2047),
        y=((c>>11)&2047),
        z=( c>>22      );
   m.setRotateZ(z*(PI2/1024)).rotateXY(x*(PI2/2048), y*(PI2/2048));
   return T;
}
/******************************************************************************/
// MISC
/******************************************************************************/
UInt File::memUsage()C
{
   UInt   mem=_buf_size;
   switch(_type)
   {
      case FILE_MEM : if(_writable)mem+=_size           ; break;
   #if FILE_MEMB_UNION
      case FILE_MEMB:              mem+=_memb.memUsage(); break;
   #endif
   }
#if !FILE_MEMB_UNION
   mem+=_memb.memUsage();
#endif
   return mem;
}
/******************************************************************************/
Bool File::flushDo()
{
   DEBUG_ASSERT(_type==FILE_STD_WRITE && _buf_len, "File.flushDo");
   Int written =Write(_handle, _buf, _buf_len);
   if( written==_buf_len){_buf_len=0; return true;} // check this first because most likely this will happen, so return early
#if ALLOW_REFLUSH
   if(written>0) // this avoids errors (<0) and when no data was written (==0)
   {
     _buf_len-=written; // decrease what we've written
      MoveFast(_buf, (Byte*)_buf+written, _buf_len); // since writing always uses start of the buffer, we need to copy remaining data at the start
   }
   // here '_pos, _buf_len, _ok' shouldn't be adjusted, because no data is lost, some was written and remaining is still in the buffer
#else
   if(written>0)_pos+=written; // this avoids errors (<0) and when no data was written (==0)
  _pos-=_buf_len; // normally '_pos' is located already ahead at '_buf_len' position (assumes that everything was written), but if we've written less, then we need to set it back to what was lost
  _buf_len=0; // discard data
  _ok=false; // data was lost
#endif
   return false;
}
Bool File::flushOK() {return flush() && ok();}
Bool File::flush  ()
{
   if(_buf_len && _type==FILE_STD_WRITE)return flushDo();
   return true;
}
Bool File::sync()
{
   if(_writable && (_type==FILE_STD_WRITE || _type==FILE_STD_READ)) // check 'FILE_STD_READ' too, because we may have written data previously in FILE_STD_WRITE, and later switched to FILE_STD_READ
   {
   #if WINDOWS
      #if 1 // faster
         return FlushFileBuffers((HANDLE)_get_osfhandle(_handle));
      #else // internally calls 'FlushFileBuffers' which means this has bigger overhead
         return _commit(_handle)==0;
      #endif
   #else
      return fsync(_handle)==0;
   #endif
   }
   return true;
}
/******************************************************************************/
void File::cipher(Cipher *cipher)
{
   if(T._cipher!=cipher)
   {
      if(_type==FILE_STD_READ) // if we're in reading mode (we can skip FILE_STD_WRITE because any data saved in the buffer is saved with correct cipher already, and we don't want to change it)
         if(Int buf_size=_buf_pos+_buf_len) // and we have any data in buffer that we've read ahead
      {
         auto offset=posCipher()-_buf_pos;
         if(T._cipher)T._cipher->encrypt(_buf, _buf, buf_size, offset); // we have to encrypt entire buffer back using old cipher
         if(   cipher)   cipher->decrypt(_buf, _buf, buf_size, offset); // decrypt using new cipher
      }
      T._cipher=cipher;
   }
}
void File::cipherOffset(Int offset)
{
   if(T._cipher_offset!=offset)
   {
      if(_type==FILE_STD_READ) // if we're in reading mode (we can skip FILE_STD_WRITE because any data saved in the buffer is saved with correct cipher offset already, and we don't want to change it)
         if(_cipher) // we have any cipher
            if(Int buf_size=_buf_pos+_buf_len) // and we have any data in buffer that we've read ahead
      {
        _cipher->encrypt(_buf, _buf, buf_size, posCipher()-_buf_pos); T._cipher_offset=offset; // we have to encrypt entire buffer back using old cipher offset
        _cipher->decrypt(_buf, _buf, buf_size, posCipher()-_buf_pos);                          // decrypt using new cipher offset
      }
      T._cipher_offset=offset;
   }
}
/******************************************************************************/
Bool File::size(Long size)
{
   if(size==T.size())return true;
   if(size<0 || _offset)return false;
   if(!flush() && ALLOW_REFLUSH)_ok=false; // if flush failed, then set as ok=false, because the data will be discarded
   switch(_type)
   {
      case FILE_STD_READ :
      case FILE_STD_WRITE: //if(_writable) we can skip this check because the system calls will just fail for non-writable
      {
      #if WINDOWS
         if(!_chsize_s  (_handle, size))
      #elif APPLE || (ANDROID && __ANDROID_API__<12) // on Apple 'ftruncate' is already 64-bit, while Android has 'ftruncate64' only on API 12 and above
         if(!ftruncate  (_handle, size))
      #else
         if(!ftruncate64(_handle, size))
      #endif
         {
            // no need to call 'setBuf' because we can only change size for files in write mode, an in this mode files always have BUF_SIZE buffer
            // no need to clip current position to size limit, because 'File' allows position being after the end
           _size=size;
            discardBuf(false); // disable flushing because we've already tried it at the start, and doing it again could change the file size
         }
      }break;

      case FILE_MEM:
      {
         if(_writable)_Realloc(_mem, size, _size); // write mode allocated memory manually, so we need to reallocate it
        _size=size;
      }break;

      case FILE_MEMB: _size=_memb.setNum(size).elms(); break; // set '_size' from what 'Memb' will actually have
   }
   return _size==size;
}
/******************************************************************************/
Int File::getReturnSize(Ptr data, Int size)
{
   Long old_pos=_pos;
   if(!data)skip(size);else
   {
      MIN(size, left());
      if (size>0)switch(_type)
      {
         case FILE_STD_WRITE:
         {
            if(!discardBuf(true))break; // if this fails then we can't read, because we're expecting to read at the current position
           _type=FILE_STD_READ; // set new mode and fall down for reading
         } // !! no break on purpose !!
         case FILE_STD_READ :
         {
            if(_buf_len) // have data in the buffer
            {
            get_from_buffer:
               Int l=Min(_buf_len, size);
               CopyFast(data, (Byte*)_buf+_buf_pos, l);
              _buf_pos+=l;
              _buf_len-=l;
              _pos +=l;
               size-=l; if(size<=0)break;
               data =(Byte*)data+l; // can be placed after break
            }
            // read from file
            if(size>=_buf_size/2) // in this case it will be faster to don't use buffering and just read directly to target memory
                                  // "size>=_buf_size" was tested as well, but it was slower
            {
               // read to 'data'
               Int l =Read(_handle, data, size);
               if( l<=0)break;
               if(_cipher)_cipher->decrypt(data, data, l, posCipher());
              _pos +=l;
               size-=l; if(size<=0)break;
               data =(Byte*)data+l; // can be placed after break
            }
            // read to buffer
            Int l =Read(_handle, _buf, Min(_buf_size, left())); // read as much as possible
            if( l<=0)break;
            if(_cipher)_cipher->decrypt(_buf, _buf, l, posCipher()); // decrypt entire buffer at the same time (faster)
           _buf_pos=0;
           _buf_len=l;
            goto get_from_buffer;
         }break;

         case FILE_MEM:
         {
            Ptr src=memFast();
            if(_cipher)_cipher->decrypt(data, src, size, posCipher());
            else               CopyFast(data, src, size             );
           _pos+=size;
          //data=(Byte*)data+size; not needed
          //size=               0; not needed
         }break;

         case FILE_MEMB:
         {
            for(;;)
            {
               Int pos_abs=posAbs(), // Long not needed because later this is used only as Int memb element index
                   elm    =pos_abs & _memb.mask(), // index of element in a block
                   l      =Min(size, _memb.blockElms()-elm); // how much elements left in this block
               if(_cipher)_cipher->decrypt(data, &_memb[pos_abs], l, posCipher());
               else               CopyFast(data, &_memb[pos_abs], l             );
              _pos +=l;
               size-=l; if(size<=0)break;
               data =(Byte*)data+l; // can be placed after break
            }
         }break;
      }
   }
   return _pos-old_pos; // return number of bytes read
}
Int File::putReturnSize(CPtr data, Int size)
{
   Long old_pos=_pos;
   if(size>0)
   {
      switch(_type)
      {
         case FILE_STD_READ:
         {
            if(!_writable || !discardBuf(false))break; // if this fails then we can't write, because we're expecting to write at the current position, no need for flush because FILE_STD_READ doesn't need flush
           _type=FILE_STD_WRITE; // set new mode and fall down for writing
         } // !! no break on purpose !!
         case FILE_STD_WRITE:
         {
            if(!_cipher && data) // buffering optional (check for 'data' because it's allowed to be null and write zeros)
            {
               do{
                  if(_buf_len || size<_buf_size) // copy to buffer
                  {
                     Int l=Min(size, _buf_size-_buf_len);
                     CopyFast((Byte*)_buf+_buf_len, data, l);
                    _buf_len+=l;
                     data =(Byte*)data+l;
                    _pos +=l;
                     size-=l;

                     if(_buf_len>=_buf_size && !flushDo())break; // if buffer is full then flush and check for errors
                  }else // direct write
                  {
                     Int written=Write(_handle, data, size); if(written<=0)break;
                     data =(Byte*)data+written;
                    _pos +=written;
                     size-=written;
                  }
               }while(size>0);
            }else // buffering necessary
            {
               do{
                  // copy to buffer
                  Int l=Min(size, _buf_size-_buf_len);
                  if(_cipher)_cipher->encrypt((Byte*)_buf+_buf_len, data, l, posCipher());
                  else                   Copy((Byte*)_buf+_buf_len, data, l             );
                 _buf_len+=l;
                  if(data)data=(Byte*)data+l;
                 _pos +=l;
                  size-=l;

                  if(_buf_len>=_buf_size && !flushDo())break; // if buffer is full then flush and check for errors
               }while(size>0);
            }
         }break;

         case FILE_MEM: if(_writable)
         {
            Int write=Min(size, left()); // don't write more than is allocated
            Ptr dest =memFast();
            if(_cipher)_cipher->encrypt(dest, data, write, posCipher());
            else                   Copy(dest, data, write             );
           _pos +=write;
            size-=write;
         }break;

         case FILE_MEMB:
         {
            Int dest_elms=posAbs()+size; if(dest_elms>_memb.elms())_memb.setNum(dest_elms); // Long not needed because is used for Memb based on 32-bits
            do{
               Int pos_abs=posAbs(), // Long not needed because later is used only as Int memb element index
                   elm    =pos_abs & _memb.mask(), // index of element in a block
                   l      =Min(size, _memb.blockElms()-elm); // how much elements left in this block
               if(_cipher)_cipher->encrypt(&_memb[pos_abs], data, l, posCipher());
               else                   Copy(&_memb[pos_abs], data, l             );
               if(data)data=(Byte*)data+l;
              _pos +=l;
               size-=l;
            }while(size>0);
         }break;
      }
      MAX(_size, _pos);
   }
   return _pos-old_pos; // return number of bytes written
}
Bool File::getFast(Ptr data, Int size)
{
   if(getReturnSize(data, size)==size)return true;
  _ok=false; return false; // set error
}
Bool File::get(Ptr data, Int size)
{
   Int read=getReturnSize(data, size);
   if( read==size)return true; // check this first because this is what's most likely going to happen
   if( data)Zero((Byte*)data+read, size-read); // zero unread data, this is important because methods such as f.getInt, f.decUIntV, .. don't check for status, however they're expected to return zeros
  _ok=false; return false; // set error
}
Bool File::put(CPtr data, Int size)
{
   Int written=putReturnSize(data, size);
   if( written==size)return true; // check this first because this is what's most likely going to happen
  _ok=false; return false; // set error
}
/******************************************************************************/
File& File::putStr(CChar8 *t)
{
   Int  length =Length    (t);
   Bool unicode=HasUnicode(t);

   cmpIntV(unicode ? -length : length);
   if(length)
   {
      if(unicode)
      {
         Memt<Char> temp; temp.setNum(length+1); Set(temp.data(), t, temp.elms());
         putN(temp.data(), length);
      }else
      {
         putN(t, length);
      }
   }
   return T;
}
File& File::putStr(CChar *t)
{
   Int  length =Length    (t);
   Bool unicode=HasUnicode(t);

   cmpIntV(unicode ? -length : length);
   if(length)
   {
      if(unicode)
      {
         putN(t, length);
      }else
      {
         Memt<Char8> temp; temp.setNum(length+1); Set(temp.data(), t, temp.elms());
         putN(temp.data(), length);
      }
   }
   return T;
}
File& File::putStr(C Str8 &s) // keep this function to allow having '\0' chars in the middle
{
   Int  length =s.length();
   Bool unicode=HasUnicode(s);

   cmpIntV(unicode ? -length : length);
   if(length)
   {
      if(unicode){Memt<Char> temp; temp.setNum(length); FREPAO(temp)=Char8To16Fast(s[i]); putN(temp.data(), length);} // we can assume that Str was already initialized
      else       {                                                                        putN(        s(), length);}
   }
   return T;
}
File& File::putStr(C Str &s) // keep this function to allow having '\0' chars in the middle
{
   Int  length =s.length();
   Bool unicode=HasUnicode(s);

   cmpIntV(unicode ? -length : length);
   if(length)
   {
      if(unicode){                                                                         putN(        s(), length);}
      else       {Memt<Char8> temp; temp.setNum(length); FREPAO(temp)=Char16To8Fast(s[i]); putN(temp.data(), length);} // we can assume that Str was already initialized
   }
   return T;
}
File& File::skipStr()
{
   Int length; decIntV(length);
   if( length<0){CHS(length); length*=SIZE(Char );} // unicode
   else                       length*=SIZE(Char8);
   if(left()<length) // length too long
   {
     _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else
   }else skip(length);
   return T;
}
File& File::getStr(Str8 &s) // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   Int length; decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      s.reserve(length);
      Memt<Char> temp; temp.setNum(length); getN(temp.data(), length);
      FREPA(temp)s._d[i]=Char16To8Fast(temp[i]); s._d[s._length=length]=0; // we can assume that Str was already initialized
   }else
   if(length)
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      s.reserve(length);
      getN(s._d.data(), length); s._d[s._length=length]=0;
   }

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   s.clear(); return T;
}
File& File::getStr(Str &s) // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   Int length; decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      s.reserve(length);
      getN(s._d.data(), length); s._d[s._length=length]=0;
   }else
   if(length)
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      s.reserve(length);
      Char8 *temp=(Char8*)s._d.data(); getN(temp, length); // we can re-use the string memory because it uses Char which has 2x Char8 capacity
      s._d[s._length=length]=0; // because we're processing from the end, then start with the end too
      REP(length)s._d[i]=Char8To16Fast(temp[i]); // need to process from the end to not overwrite the source, we can assume that Str was already initialized
   }

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   s.clear(); return T;
}
File& File::getStr(Char8 *t, Int t_elms)
{
   Int length; decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         Memt<Char> temp; temp.setNum(read); getN(temp.data(), read);
         FREP(read)t[i]=Char16To8Fast(temp[i]); t[read]=0; // we can assume that Str was already initialized
         length-=read;
      }
      length*=SIZE(Char);
   }else
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         getN(t, read); t[read]=0;
         length-=read;
      }
   }
   skip(length);

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   if(t && t_elms>0)t[0]='\0'; return T;
}
File& File::getStr(Char *t, Int t_elms)
{
   Int length; decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         getN(t, read); t[read]=0;
         length-=read;
      }
      length*=SIZE(Char);
   }else
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         Char8 *temp=(Char8*)t; getN(temp, read); // we can re-use the char array memory because it uses Char which has 2x Char8 capacity
         t[read]=0; // because we're processing from the end, then start with the end too
         REP(read)t[i]=Char8To16Fast(temp[i]); // need to process from the end to not overwrite the source, we can assume that Str was already initialized
         length-=read;
      }
   }
   skip(length);

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   if(t && t_elms>0)t[0]='\0'; return T;
}
Str File::getStr() {Str s; getStr(s); return s;} // warning: this must handle having '\0' chars in the middle
/******************************************************************************/
File& File::putAsset(C UID &id)
{
   Int used; REPAS(used, id.b)if(id.b[used])break; used++;
   putByte(used).put(id.b, used);
   return T;
}
File& File::putAsset(CChar *t)
{
   if(!Is(t))putByte(0);else
   {
      UID id; if(DecodeFileName(t, id))putAsset(id);else putByte(0xFF).putStr(t);
   }
   return T;
}
UID File::getAssetID()
{
   Byte b; T>>b;
   if(  b<=SIZE(UID))
   {
      UID id; if(getFast(id.b, b))
      {
         Byte *d=id.b+b; REP(SIZE(id)-b)d[i]=0; return id;
      }
   }else
   if(b==0xFF)skipStr();
 //else       error
   return UIDZero;
}
File& File::getAsset(Str &s)
{
   Byte b; T>>b;
   if(  b<=SIZE(UID))
   {
      if(!b)s.clear();else
      {
         UID id; if(getFast(id.b, b))
         {
            Byte *d=id.b+b; REP(SIZE(id)-b)d[i]=0; s=_EncodeFileName(id);
         }else s.clear();
      }
   }else
   if(b==0xFF)getStr(s);
   else       s.clear(); // error
   return T;
}
Str File::getAsset() {Str s; getAsset(s); return s;}
/******************************************************************************/
// Deprecated, do not use
File& File::_decIntV(Int &i)
{
   Byte v; T>>v;
   Bool positive=((v>>6)&1);
   UInt u=(v&63);
   if(v&128)
   {
      T>>v; u|=((v&127)<<6);
      if(v&128)
      {
         T>>v; u|=((v&127)<<(6+7));
         if(v&128)
         {
            T>>v; u|=((v&127)<<(6+7+7));
            if(v&128)
            {
               T>>v; u|=(v<<(6+7+7+7));
            }
         }
      }
   }
   i=(positive ? u+1 : -Int(u));
   return T;
}
Str   File::_getStr2(      ) {Str s; _getStr2(s); return s;} // warning: this must handle having '\0' chars in the middle
File& File::_getStr2(Str &s) // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   Int length; _decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      s.reserve(length);
      getN(s._d.data(), length); s._d[s._length=length]=0;
   }else
   if(length)
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      s.reserve(length);
      Char8 *temp=(Char8*)s._d.data(); getN(temp, length); // we can re-use the string memory because it uses Char which has 2x Char8 capacity
      s._d[s._length=length]=0; // because we're processing from the end, then start with the end too
      REP(length)s._d[i]=Char8To16Fast(temp[i]); // need to process from the end to not overwrite the source, we can assume that Str was already initialized
   }

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   s.clear(); return T;
}
File& File::_getStr2(Str8 &s) // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   Int length; _decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      s.reserve(length);
      Memt<Char> temp; temp.setNum(length); getN(temp.data(), length);
      FREPA(temp)s._d[i]=Char16To8Fast(temp[i]); s._d[s._length=length]=0; // we can assume that Str was already initialized
   }else
   if(length)
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      s.reserve(length);
      getN(s._d.data(), length); s._d[s._length=length]=0;
   }

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   s.clear(); return T;
}
File& File::_getStr2(Char8 *t, Int t_elms)
{
   Int length; _decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         Memt<Char> temp; temp.setNum(read); getN(temp.data(), read);
         FREP(read)t[i]=Char16To8Fast(temp[i]); t[read]=0; // we can assume that Str was already initialized
         length-=read;
      }
      length*=SIZE(Char);
   }else
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         getN(t, read); t[read]=0;
         length-=read;
      }
   }
   skip(length);

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   if(t && t_elms>0)t[0]='\0'; return T;
}
File& File::_getStr2(Char *t, Int t_elms)
{
   Int length; _decIntV(length);
   if( length<0) // unicode
   {
      CHS(length);
      if(left()<length*SIZEI(Char))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         getN(t, read); t[read]=0;
         length-=read;
      }
      length*=SIZE(Char);
   }else
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         Char8 *temp=(Char8*)t; getN(temp, read); // we can re-use the char array memory because it uses Char which has 2x Char8 capacity
         t[read]=0; // because we're processing from the end, then start with the end too
         REP(read)t[i]=Char8To16Fast(temp[i]); // need to process from the end to not overwrite the source, we can assume that Str was already initialized
         length-=read;
      }
   }
   skip(length);

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   if(t && t_elms>0)t[0]='\0'; return T;
}
File& File::_putStr(C Str8 &s) // warning: this must handle having '\0' chars in the middle
{
   UInt length =s.length();
   Bool unicode=HasUnicode(s);

   putUInt(unicode ? length^SIGN_BIT : length);
   if(length)
   {
      if(unicode){Memt<Char> temp; temp.setNum(length); FREPAO(temp)=Char8To16Fast(s[i]); putN(temp.data(), length);} // we can assume that Str was already initialized
      else       {                                                                        putN(        s(), length);}
   }
   return T;
}
File& File::_putStr(C Str &s) // warning: this must handle having '\0' chars in the middle
{
   UInt length =s.length();
   Bool unicode=HasUnicode(s);

   putUInt(unicode ? length^SIGN_BIT : length);
   if(length)
   {
      if(unicode){                                                                         putN(        s(), length);}
      else       {Memt<Char8> temp; temp.setNum(length); FREPAO(temp)=Char16To8Fast(s[i]); putN(temp.data(), length);} // we can assume that Str was already initialized
   }
   return T;
}
File& File::_getStr(Str8 &s) // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   UInt length=getUInt();
   if(  length&SIGN_BIT) // unicode
   {
         length^=SIGN_BIT; MIN(length, left()/SIZEI(Char));
      if(length)
      {
         s.reserve(length);
         Memt<Char> temp; temp.setNum(length); getN(temp.data(), length);
         FREPA(temp)s._d[i]=Char16To8Fast(temp[i]); // we can assume that Str was already initialized
         s._d[s._length=length]=0;
      }
   }else
   {
      MIN(length, left());
      if (length)
      {
         s.reserve(length);
         getN(s._d.data(), length);
         s._d[s._length=length]=0;
      }
   }
   return T;
}
Str   File::_getStr(      ) {Str s; _getStr(s); return s;} // warning: this must handle having '\0' chars in the middle
File& File::_getStr(Str &s)                                // warning: this must handle having '\0' chars in the middle
{
   s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   UInt length=getUInt();
   if(  length&SIGN_BIT) // unicode
   {
         length^=SIGN_BIT; MIN(length, left()/SIZEI(Char));
      if(length)
      {
         s.reserve(length);
         getN(s._d.data(), length);
         s._d[s._length=length]=0;
      }
   }else
   {
      MIN(length, left());
      if (length)
      {
         s.reserve(length);
         Memt<Char8> temp; temp.setNum(length); getN(temp.data(), length);
         FREPA(temp)s._d[i]=Char8To16Fast(temp[i]); // we can assume that Str was already initialized
         s._d[s._length=length]=0;
      }
   }
   return T;
}
File& File::_getStr(Char *t, Int t_elms)
{
   UInt length=getUInt();
   if(  length&SIGN_BIT) // unicode
   {
      length^=SIGN_BIT;
      if(left()<length*SIZEI(Char))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         getN(t, read); t[read]=0;
         length-=read;
      }
      length*=SIZE(Char);
   }else
   {
      if(left()<length*SIZEI(Char8))goto length_too_long;
      if(t && t_elms>0)
      {
         Int read=Min(length, t_elms-1);
         Char8 *temp=(Char8*)t; getN(temp, read); // we can re-use the char array memory because it uses Char which has 2x Char8 capacity
         t[read]=0; // because we're processing from the end, then start with the end too
         REP(read)t[i]=Char8To16Fast(temp[i]); // need to process from the end to not overwrite the source, we can assume that Str was already initialized
         length-=read;
      }
   }
   skip(length);

   if(ok())return T;
   goto error;

length_too_long:
  _ok=false; pos(size()); // if the length was too long then go at the end of file, in case the user will try to read more data after this call, this is important so that the partially available string data is not treated as something else

error:
   if(t && t_elms>0)t[0]='\0'; return T;
}
Str8 File::_getStr8()
{
   Str8 s;
   UInt length=getUInt();
        s._d.setNum( length+1);
   getN(s._d.data(), length+1);
        s._length  = length   ;
   return s;
}
Str File::_getStr16()
{
   Str s;
   UInt length=getUInt();
        s._d.setNum( length+1);
   getN(s._d.data(), length+1);
        s._length  = length   ;
   return s;
}
File& File::_putAsset(CChar *t)
{
   if(!Is(t))putByte(0);else {UID id; if(DecodeFileName(t, id))putByte(2)<<id;else putByte(1).putStr(t);}
   return T;
}
Str File::_getAsset()
{
   switch(getByte())
   {
      default: return S;
      case  1: return _getStr2();
      case  2: {UID id; T>>id; return _EncodeFileName(id);}
   }
}
/******************************************************************************/
 Short File::getBEShort () { Short i   ; T>>i; SwapEndian(i); return i;}
UShort File::getBEUShort() {UShort i   ; T>>i; SwapEndian(i); return i;}
 Int   File::getBEInt24 () {Int24  i   ; T>>i; Swap(i.b[0], i.b[2]); return i.asInt();}
UInt   File::getBEUInt24() {Byte   b[3]; T>>b; return b[2] | (b[1]<<8) | (b[0]<<16);}
 Int   File::getBEInt   () { Int   i   ; T>>i; SwapEndian(i); return i;}
UInt   File::getBEUInt  () {UInt   i   ; T>>i; SwapEndian(i); return i;}
 Long  File::getBELong  () { Long  i   ; T>>i; SwapEndian(i); return i;}
ULong  File::getBEULong () {ULong  i   ; T>>i; SwapEndian(i); return i;}
/******************************************************************************/
Bool File::equal(File &f, Long max_size)
{
   if(max_size<0 || left()<max_size || f.left()<max_size){if(left()!=f.left())return false; max_size=left();} // if 'max_size' is not specified or any of the files has less than expected bytes, then continue only if both have same amount of bytes less, and test only those bytes
   if(max_size>0)
   {
      Memt<Byte, TEMP_BUF_SIZE> temp; temp.setNum(TEMP_BUF_SIZE); Int buf_size=temp.elms()/2; Ptr buf=temp.data(), buf2=temp.data()+buf_size;
      for(; max_size>0; )
      {
         Int l=Min(buf_size, max_size); max_size-=l;
         if(!getFast (buf, l) || !f.getFast(buf2, l))return false;
         if(!EqualMem(buf,                  buf2, l))return false;
      }
   }
   return true;
}
Bool File::copy(File &dest, Long max_size)
{
   if(max_size<0)max_size=left();
   if(max_size>0)
   {
      Memt<Byte, TEMP_BUF_SIZE> temp; temp.setNum(TEMP_BUF_SIZE); Ptr buf=temp.data(); Int buf_size=temp.elms();
      REP(     max_size/buf_size )if(!getFast(buf, buf_size) || !dest.put(buf, buf_size))return false;
      buf_size=max_size%buf_size; if(!getFast(buf, buf_size) || !dest.put(buf, buf_size))return false;
   }
   return true;
}
Bool File::copy(File &dest, DataCallback &callback, Long max_size)
{
   if(max_size<0)max_size=left();
   if(max_size>0)
   {
      Memt<Byte, TEMP_BUF_SIZE> temp; temp.setNum(TEMP_BUF_SIZE); Ptr buf=temp.data(); Int buf_size=temp.elms();
      REP(     max_size/buf_size ){if(!getFast(buf, buf_size) || !dest.put(buf, buf_size))return false; callback.data(buf, buf_size);}
      buf_size=max_size%buf_size;  if(!getFast(buf, buf_size) || !dest.put(buf, buf_size))return false; callback.data(buf, buf_size);
   }
   return true;
}
Bool File::copyEncrypt(File &dest, Cipher &cipher, Long max_size)
{
   if(max_size<0)max_size=left();
   if(max_size>0)
   {
      Memt<Byte, TEMP_BUF_SIZE> temp; temp.setNum(TEMP_BUF_SIZE); Ptr buf=temp.data(); Int buf_size=temp.elms(), offset=0;
      REP(     max_size/buf_size ){if(!getFast(buf, buf_size))return false; cipher.encrypt(buf, buf, buf_size, offset); if(!dest.put(buf, buf_size))return false; offset+=buf_size;}
      buf_size=max_size%buf_size;  if(!getFast(buf, buf_size))return false; cipher.encrypt(buf, buf, buf_size, offset); if(!dest.put(buf, buf_size))return false;
   }
   return true;
}
/******************************************************************************/
void File::  clearBuf() {_buf_pos=_buf_len=0;} // !! this does not reset the system file handle position, if the buffer read something, then the position is ahead !!
Bool File::discardBuf(Bool flush)
{
   if(_buf_pos || _buf_len) // if there's any data in the buffer
   {
      Long pos=T.pos(); // remember current position before doing any operation
      if(flush && !T.flush() && ALLOW_REFLUSH)_ok=false; // !! do this after remembering position because this method may change it !! if we had some data to save which failed, then only disable '_ok', but still proceed because here the priority is to set correct file position
      T._pos=posFile(); // set actual position, so calling "T.pos(pos)" will proceed because current position is different than desired
      clearBuf(); // !! do this after calling 'posFile' !! always clear buffer (in case read mode or in case write mode flush fail)
      return T.pos(pos); // set remembered position
   }
   return true;
}
/******************************************************************************/
void File::  limit(ULong &total_size, ULong &applied_offset, Long new_size) {total_size=size(); applied_offset=pos(); T._size=  new_size; T._offset+=applied_offset; T._cipher_offset+=applied_offset; T._pos-=applied_offset;}
void File::unlimit(ULong &total_size, ULong &applied_offset               ) {                                         T._size=total_size; T._offset-=applied_offset; T._cipher_offset-=applied_offset; T._pos+=applied_offset; applied_offset=0;}
/******************************************************************************/
UInt File::crc32(Long max_size)
{
   Byte  buf[TEMP_BUF_SIZE];
   CRC32 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return 0;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
UInt File::xxHash32(Long max_size)
{
   Byte     buf[TEMP_BUF_SIZE];
 ::xxHash32 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return 0;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
#pragma runtime_checks("", off)
UInt  File::xxHash64_32(Long max_size) {return xxHash64(max_size);} // we're interested only in low 32 bits
#pragma runtime_checks("", restore)
ULong File::xxHash64   (Long max_size)
{
   Byte     buf[TEMP_BUF_SIZE];
 ::xxHash64 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return 0;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
UInt  File::spookyHash32 (Long max_size) {return spookyHash128(max_size).i[0];} // can use part of 128-bit because this is how 'SpookyHash' works
ULong File::spookyHash64 (Long max_size) {return spookyHash128(max_size).l[0];} // can use part of 128-bit because this is how 'SpookyHash' works
UID   File::spookyHash128(Long max_size)
{
   Byte       buf[TEMP_BUF_SIZE];
   SpookyHash hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return UIDZero;
      hash.update(buf, left); max_size-=left;
   }
   return hash.hash128();
}
ULong File::metroHash64(Long max_size)
{
   Byte        buf[TEMP_BUF_SIZE];
   MetroHash64 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return 0;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
UID File::metroHash128(Long max_size)
{
   Byte         buf[TEMP_BUF_SIZE];
   MetroHash128 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return UIDZero;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
UID File::md5(Long max_size)
{
   Byte buf[TEMP_BUF_SIZE];
   MD5  hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left))return UIDZero;
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
SHA1::Hash File::sha1(Long max_size)
{
   Byte buf[TEMP_BUF_SIZE];
   SHA1 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left)){hash._hash.zero(); return hash._hash;}
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
SHA2::Hash File::sha2(Long max_size)
{
   Byte buf[TEMP_BUF_SIZE];
   SHA2 hash;
   if(max_size<0)max_size=left();
   for(;;)
   {
      Int left=Min(SIZEI(buf), max_size); if(left<=0)break;
      if(!getFast(buf, left)){hash._hash.zero(); return hash._hash;}
      hash.update(buf, left); max_size-=left;
   }
   return hash();
}
/******************************************************************************/
}
/******************************************************************************/
