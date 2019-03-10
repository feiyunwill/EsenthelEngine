/******************************************************************************

   'UIntPtr' was chosen for in-memory compression, to achieve:
      -full speed at 32-bit platforms (UInt)
      -32-bit platforms can't do 64-bit in-memory compressions
      -64-bit support on 64-bit platforms

   'Long' was chosen for stream compression, to achieve:
      -64-bit support on both 32/64

   Stream based LZ4 uses custom block separation (not LZ4Frame API, because that one is inefficient, generates UInt chunk sizes instead of cmpUIntV)

   Some functions have 'NOINLINE' because they use a lot of stack memory,
      and some compilers (Apple) when trying to inline them, would cause a crash.

   Some functions use 'MemWrote' for direct decompression, and some use 'dest.skip' with 'MemFinished'.
      LZ4 and ZSTD require the previous content of dest buffer to be still accessible, so encryption needs to be performed at the end.

/******************************************************************************/
#include "stdafx.h"

#define SUPPORT_LIZARD 0 // disable because LZ4 gives similar results
#define SUPPORT_BROTLI 0 // disable because ZSTD is considered better

#include "../../../ThirdPartyLibs/begin.h"

#if SUPPORT_SNAPPY
   #include "../../../ThirdPartyLibs/Snappy/snappy-c.h"
#endif

#if SUPPORT_LZ4
   #if __clang__
      #define LZ4_DISABLE_DEPRECATE_WARNINGS // fails to compile without it
   #endif
   #include "../../../ThirdPartyLibs/LZ4/lz4.h"
   #include "../../../ThirdPartyLibs/LZ4/lz4hc.h"
#endif

#if SUPPORT_LIZARD
   #include "../../../ThirdPartyLibs/Lizard/lib/lizard_compress.h"
   #include "../../../ThirdPartyLibs/Lizard/lib/lizard_decompress.h"
#endif

#if SUPPORT_ZSTD
   #define ZSTD_STATIC_LINKING_ONLY
   #define ZSTD_WINDOWLOG_SAVE 0 // !! do not change in the future !!
   #include "../../../ThirdPartyLibs/Zstd/lib/zstd.h"
#endif

#if SUPPORT_BROTLI
   #include "../../../ThirdPartyLibs/Brotli/lib/enc/encode.h"
   #include "../../../ThirdPartyLibs/Brotli/lib/dec/decode.h"
#endif

#if SUPPORT_LZMA
extern "C"
{
   #define Bool   LZMABool
   #define UInt32 LZMAUInt32
   #include "../../../ThirdPartyLibs/LZMA/lzma/C/LzmaDec.h"
   #include "../../../ThirdPartyLibs/LZMA/lzma/C/LzmaEnc.h"
   #define LZMA2 0 // no significant improvement over LZMA
   #if     LZMA2
      #include "../../../ThirdPartyLibs/LZMA/lzma/C/Lzma2Dec.h"
      #include "../../../ThirdPartyLibs/LZMA/lzma/C/Lzma2Enc.h"
   #endif
   #undef Bool
   #undef UInt32
}
#endif

#if SUPPORT_LZHAM
   #include "../../../ThirdPartyLibs/LZHAM/Source/lzham_comp.h"
   #include "../../../ThirdPartyLibs/LZHAM/Source/lzham_decomp.h"
#endif

#if SUPPORT_ZLIB
   #if WINDOWS
      #include "../../../ThirdPartyLibs/zlib/zlib.h" // use custom zlib library on windows
   #else
      #include <zlib.h> // use built-in zlib library on other platforms
   #endif
#endif

#include "../../../ThirdPartyLibs/end.h"

#pragma warning(disable:4267) // conversion from 'size_t' to 'Int', possible loss of data

#define BUF_SIZE 65536
/******************************************************************************/
namespace EE{
/******************************************************************************/
static Bool MemDecompress(File &f, Long decompressed_size)
{
   return f._type==FILE_MEM           // continuous memory
       && f.left()>=decompressed_size // file has enough room for decompressed data
       && decompressed_size>=0;       // decompressed size is known
}
static Bool MemWrote(File &f, UIntPtr size)
{
   if(f._cipher)f._cipher->encrypt(f.memFast(), f.memFast(), size, f.posCipher());
   return f.skip(size);
}
static void MemFinished(File &f, Ptr mem, Long wrote, Int cipher_offset)
{
   if(f._cipher)f._cipher->encrypt(mem, mem, wrote, cipher_offset);
}
static Ptr  CompressAlloc(Ptr p, size_t size) {return Alloc(size   );}
static void CompressFree (Ptr p, Ptr address) {       Free (address);}
/******************************************************************************/
// ZLIB
/******************************************************************************/
#if SUPPORT_ZLIB

#define ZLIB_COMPRESSBOUND(x) ((x) + ((x)>>12) + ((x)>>14) + ((x)>>25) + 13) // taken from source code

static UIntPtr ZLIBSize(UIntPtr src_size) {return compressBound(src_size);}  // get size needed for 'dest' buffer when compressing 'src_size' bytes

static Bool ZLIBCompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size, Int compression_level) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, 'compression_level'=(0..9) (0=fastest/worst, 9=slowest/best), false on fail
{
   uLongf size=dest_size; Bool   ok=(compress2((Byte*)dest, &size, (Byte*)src, src_size, Mid(compression_level, 1, 9))==Z_OK);
     dest_size=     size; return ok;
}
static Bool ZLIBDecompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   uLongf size=dest_size; Bool   ok=(uncompress((Byte*)dest, &size, (Byte*)src, src_size)==Z_OK);
     dest_size=     size; return ok;
}
/******************************************************************************/
NOINLINE static Bool ZLIBCompressMem(File &src, File &dest, Int compression_level)
{
   Memt<Byte,                    BUF_SIZE > s;
   Memt<Byte, ZLIB_COMPRESSBOUND(BUF_SIZE)> d;
   s.setNum(src.left()); if(!src.getFast(s.data(), s.elms()))return false;
   d.setNum(ZLIBSize(s.elms())); UIntPtr dest_size=d.elms();
   if(ZLIBCompress(s.data(), s.elms(), d.data(), dest_size, compression_level))
   {
      s.del(); // delete 's' first to release memory
      return dest.put(d.data(), dest_size);
   }
   return false;
}
NOINLINE static Bool ZLIBDecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size) // decompress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing, 'compressed_size' must be correctly specified in order to decompress the data, false on fail
{
   Memt<Byte, ZLIB_COMPRESSBOUND(BUF_SIZE)> s; s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))return false;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(ZLIBDecompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)return MemWrote(dest, dest_size);
   }else
   {
      Memt<Byte, BUF_SIZE> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(ZLIBDecompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
/******************************************************************************/
NOINLINE static Bool ZLIBCompress(File &src, File &dest, Int compression_level, DataCallback *callback) // compress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing, 'compression_level'=(0..9) (0=fastest/worst, 9=slowest/best), compression occurs from the current position of 'src' to the end of the file, false on fail
{
   Bool     ok=false;
   z_stream stream; Zero(stream);
   if(deflateInit(&stream, Mid(compression_level, 1, 9))==Z_OK) // 0=no compression
   {
      Byte in[BUF_SIZE], out[BUF_SIZE];
      for(;;)
      {
         Int read=src.getReturnSize(in, SIZE(in)); // always proceed even if 'read'==0 in case of empty files or if we've encountered end of file
         if(callback)callback->data(in, read);
         stream.avail_in=read;
         stream. next_in=in;
         Int status;
         do{
            stream.avail_out=SIZE(out);
            stream. next_out=     out ;
            status=deflate(&stream, src.end() ? Z_FINISH : Z_NO_FLUSH);
            if(status==Z_STREAM_ERROR)goto error;
            if(!dest.put(out, SIZE(out)-stream.avail_out))goto error;
         }while(stream.avail_out==0);
         if(    stream.avail_in)goto error; // if there's still some input left, then it failed
         if(src.end()){if(status==Z_STREAM_END)ok=true; break;} // finished
         if(read<=0  )goto error; // if not end of 'src' but failed to read anything, then it failed
      }
   error:
      deflateEnd(&stream);
   }
   return ok;
}
NOINLINE static Bool ZLIBDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback) // decompress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing, 'compressed_size' must be correctly specified in order to decompress the data, false on fail
{
   Bool     ok=false;
   z_stream stream; Zero(stream);
   if(inflateInit(&stream)==Z_OK)
   {
      Long  start=dest.pos();
      Byte  in[BUF_SIZE], out[BUF_SIZE];
      ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Bool  direct=MemDecompress(dest, decompressed_size);
      for(;;)
      {
         Int read=src.getReturnSize(in, SIZE(in)); if(read<=0)goto error;
         stream.avail_in=read;
         stream. next_in=in;

         Int status=Z_STREAM_END; // initialize to OK in case we finish early due to 'break' in 'direct' mode
         do{
            UInt avail_out; Ptr next_out;
            if(direct)
            {
               avail_out=dest.left   (); if(!avail_out)break; // if reached end of file (this extra check is needed because the main loop continues as long as avail_out==0)
                next_out=dest.memFast();
            }else
            {
               avail_out=SIZE(out);
                next_out=     out ;
            }
            stream.avail_out=      avail_out;
            stream. next_out=(Byte*)next_out;
            status=inflate(&stream, Z_NO_FLUSH);
            switch(status)
            {
               case Z_NEED_DICT   :
               case Z_STREAM_ERROR:
               case Z_DATA_ERROR  :
               case Z_MEM_ERROR   : goto error;
            }
            avail_out-=stream.avail_out; // now 'avail_out' = how much decoded
            if(callback)callback->data(next_out, avail_out);
            if(direct ? !MemWrote(dest, avail_out)
                      : !dest.put(out , avail_out))goto error;
         }while(stream.avail_out==0);
         if(status==Z_STREAM_END){if(dest.pos()-start==decompressed_size)ok=true; break;}
      }
   error:
      src.unlimit(temp_size, temp_offset);
      inflateEnd(&stream);
   }
   return ok;
}
#endif
/******************************************************************************/
// LZMA
/******************************************************************************/
#if SUPPORT_LZMA
static ISzAlloc LzmaMem={CompressAlloc, CompressFree};
/******************************************************************************/
struct StreamIn : ISeqInStream
{
   File         &file;
   DataCallback *callback;

   static SRes Get(Ptr stream, Ptr data, size_t *size) {return ((StreamIn*)stream)->get(data, size);}
          SRes get(            Ptr data, size_t *size) {*size=file.getReturnSize(data, *size); if(callback)callback->data(data, *size); return SZ_OK;}

   StreamIn(File &file, DataCallback *callback) : file(file), callback(callback) {Read=Get;}
};
struct StreamOut : ISeqOutStream
{
   File &file;

   static size_t Put(Ptr stream, CPtr data, size_t size) {return ((StreamOut*)stream)->put(data, size);}
          size_t put(            CPtr data, Int    size) {return file.putReturnSize(data, size);}

   StreamOut(File &file) : file(file) {Write=Put;}
};
/******************************************************************************/
NOINLINE static SRes Decode(CLzmaDec *state, File &dest, File &src, Long decompressed_size, DataCallback *callback)
{
   Bool   known_size=(decompressed_size>=0), direct=MemDecompress(dest, decompressed_size);
   Byte   inBuf[BUF_SIZE], outBuf[BUF_SIZE];
   size_t inPos=0, inSize=0;
   SRes   res;
   LzmaDec_Init(state);
   for(;;)
   {
      if(inPos==inSize) // if entire 'in' buffer was processed
      {
         inPos =0;
         inSize=src.getReturnSize(inBuf, SIZE(inBuf));
      }
      SizeT inProcessed=inSize-inPos, outProcessed;
      Byte *out;
      if(direct)
      {
         out  =(Byte*)dest.memFast();
         outProcessed=dest.left   ();
      }else
      {
         out         =     outBuf ;
         outProcessed=SIZE(outBuf);
      }
      ELzmaFinishMode finishMode=LZMA_FINISH_ANY;
      if(known_size && outProcessed>decompressed_size)
      {
         outProcessed=decompressed_size;
         finishMode=LZMA_FINISH_END;
      }
      
      ELzmaStatus status; res=LzmaDec_DecodeToBuf(state, out, &outProcessed, inBuf+inPos, &inProcessed, finishMode, &status);
      if(callback)callback->data(out, outProcessed);
      if(direct ? !MemWrote(dest  , outProcessed)
                : !dest.put(outBuf, outProcessed)){res=SZ_ERROR_WRITE; break;}

       inPos           += inProcessed;
      decompressed_size-=outProcessed;

      if(res!=SZ_OK || known_size && decompressed_size==0)break;
      if(inProcessed==0 && outProcessed==0)
      {
         if(known_size || status!=LZMA_STATUS_FINISHED_WITH_MARK)res=SZ_ERROR_DATA;
         break;
      }
   }
   return res;
}
#if LZMA2
NOINLINE static SRes Decode2(CLzma2Dec *state, File &dest, File &src, Long decompressed_size, DataCallback *callback)
{
   Bool   known_size=(decompressed_size>=0), direct=MemDecompress(dest, decompressed_size);
   Byte   inBuf[BUF_SIZE], outBuf[BUF_SIZE];
   size_t inPos=0, inSize=0;
   Lzma2Dec_Init(state);
   for(;;)
   {
      if(inPos==inSize) // if entire 'in' buffer was processed
      {
         inPos =0;
         inSize=src.getReturnSize(inBuf, SIZE(inBuf));
      }
      SizeT inProcessed=inSize-inPos, outProcessed;
      Byte *out;
      if(direct)
      {
         out  =(Byte*)dest.memFast();
         outProcessed=dest.left   ();
      }else
      {
         out         =     outBuf ;
         outProcessed=SIZE(outBuf);
      }
      ELzmaFinishMode finishMode=LZMA_FINISH_ANY;
      if(known_size && outProcessed>decompressed_size)
      {
         outProcessed=decompressed_size;
         finishMode=LZMA_FINISH_END;
      }
      
      ELzmaStatus status; SRes res=Lzma2Dec_DecodeToBuf(state, out, &outProcessed, inBuf+inPos, &inProcessed, finishMode, &status);
      if(callback)callback->data(out, outProcessed);
      if(direct ? !MemWrote(dest  , outProcessed)
                : !dest.put(outBuf, outProcessed))return SZ_ERROR_WRITE;

       inPos           += inProcessed;
      decompressed_size-=outProcessed;

      if(res!=SZ_OK || known_size && decompressed_size==0)return res;
      if(inProcessed==0 && outProcessed==0)
      {
         if(known_size || status!=LZMA_STATUS_FINISHED_WITH_MARK)return SZ_ERROR_DATA;
         return res;
      }
   }
}
#endif
static SRes Decode(File &dest, File &src, Long decompressed_size, DataCallback *callback)
{
   Byte header[LZMA_PROPS_SIZE]; if(!src.getFast(header))return SZ_ERROR_READ;
   CLzmaDec state; LzmaDec_Construct(&state);
   RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &LzmaMem));
   SRes res=Decode(&state, dest, src, decompressed_size, callback);
   LzmaDec_Free(&state, &LzmaMem);
   return res;
}
#if LZMA2
static SRes Decode2(File &dest, File &src, Long decompressed_size, DataCallback *callback)
{
   Byte prop; if(!src.getFast(prop))return SZ_ERROR_READ;
   CLzma2Dec state; Lzma2Dec_Construct(&state);
   RINOK(Lzma2Dec_Allocate(&state, prop, &LzmaMem));
   SRes res=Decode2(&state, dest, src, decompressed_size, callback);
   Lzma2Dec_Free(&state, &LzmaMem);
   return res;
}
#endif
/******************************************************************************/
static void SetProps(CLzmaEncProps &props, Int compression_level, Bool multi_threaded, Long uncompressed_size)
{
   LzmaEncProps_Init(&props);
   props.writeEndMark=false; // we don't need to write the marker, because we set both 'File.limit' and 'decompressed_size', disabling this results in 5 less bytes
   props.level=Mid(compression_level, 0, 9);
   if(uncompressed_size>=0)props.reduceSize=uncompressed_size; // if size is known, this will greatly reduce memory usage for small files but with high 'compression_level'
   props.numThreads=((multi_threaded && Cpu.threads()>1) ? 2 : 1);
}
#if LZMA2
static void SetProps(CLzma2EncProps &props, Int compression_level, Bool multi_threaded, Long uncompressed_size)
{
   Lzma2EncProps_Init(&props);
   props.lzmaProps.writeEndMark=false; // we don't need to write the marker, because we set both 'File.limit' and 'decompressed_size', disabling this results in 5 less bytes
   props.lzmaProps.level=Mid(compression_level, 0, 9);
   if(uncompressed_size>=0)props.lzmaProps.reduceSize=uncompressed_size; // if size is known, this will greatly reduce memory usage for small files but with high 'compression_level'
   // TODO: test these settings
   props.lzmaProps.numThreads=((multi_threaded && Cpu.threads()>1) ? 2 : 1);
   //props.numBlockThreads=-1; 
   //props.numTotalThreads=-1;
}
#endif
/******************************************************************************/
static UInt LZMADictSize(Int compression_level, Long size)
{
   CLzmaEncProps props; SetProps(props, compression_level, false, size);
   return LzmaEncProps_GetDictSize(&props);
}
static Bool LZMACompress(File &src, File &dest, Int compression_level, Bool multi_threaded, DataCallback *callback)
{
   Bool ok=false;
   if(CLzmaEncHandle enc=LzmaEnc_Create(&LzmaMem))
   {
      CLzmaEncProps props; SetProps(props, compression_level, multi_threaded, src.left());
      if(LzmaEnc_SetProps(enc, &props)==SZ_OK)
      {
         Byte   header[LZMA_PROPS_SIZE];
         size_t headerSize=LZMA_PROPS_SIZE;
         if(LzmaEnc_WriteProperties(enc, header, &headerSize)==SZ_OK)
            if(dest.put(header, headerSize))
         {
            StreamIn  stream_in (src, callback);
            StreamOut stream_out(dest);
            ok=(LzmaEnc_Encode(enc, &stream_out, &stream_in, null, &LzmaMem, &LzmaMem)==SZ_OK);
         }
      }
      LzmaEnc_Destroy(enc, &LzmaMem, &LzmaMem);
   }
   return ok;
}
static Bool LZMADecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   ULong size, offset; src.limit(size, offset, compressed_size);
   Bool ok=(Decode(dest, src, decompressed_size, callback)==SZ_OK);
   src.unlimit(size, offset);
   return ok;
}
#if LZMA2
static Bool LZMA2Compress(File &src, File &dest, Int compression_level, Bool multi_threaded, DataCallback *callback)
{
   Bool ok=false;
   if(CLzma2EncHandle enc=Lzma2Enc_Create(&LzmaMem, &LzmaMem))
   {
      CLzma2EncProps props; SetProps(props, compression_level, multi_threaded, src.left());
      if(Lzma2Enc_SetProps(enc, &props)==SZ_OK)
      {
         Byte prop=Lzma2Enc_WriteProperties(enc);
         if(dest.put(prop))
         {
            StreamIn  stream_in (src, callback);
            StreamOut stream_out(dest);
            ok=(Lzma2Enc_Encode(enc, &stream_out, &stream_in, null)==SZ_OK);
         }
      }
      Lzma2Enc_Destroy(enc);
   }
   return ok;
}
static Bool LZMA2Decompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   ULong size, offset; src.limit(size, offset, compressed_size);
   Bool ok=(Decode2(dest, src, decompressed_size, callback)==SZ_OK);
   src.unlimit(size, offset);
   return ok;
}
#endif
#endif
/******************************************************************************/
// SNAPPY
/******************************************************************************/
#if SUPPORT_SNAPPY

#define SNAPPY_BUF_SIZE 65536 // chunks to support streaming !! don't change in the future !!
#define SNAPPY_COMPRESSBOUND(x) (32 + (x) + (x)/6) // taken from Snappy source code

static UIntPtr SNAPPYSize(UIntPtr src_size) {return snappy_max_compressed_length(src_size);}

static Bool SNAPPYCompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, false on fail
{
   size_t size=dest_size; Bool   ok=(snappy_compress((char*)src, src_size, (char*)dest, &size)==SNAPPY_OK);
     dest_size=     size; return ok;
}
static Bool SNAPPYDecompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   size_t size=dest_size; Bool   ok=(snappy_uncompress((char*)src, src_size, (char*)dest, &size)==SNAPPY_OK);
     dest_size=     size; return ok;
}
/******************************************************************************/
NOINLINE static Bool SNAPPYCompressMem(File &src, File &dest, DataCallback *callback)
{
   Memt<Byte, SNAPPY_BUF_SIZE > s; s.setNum(src.left()); if(src.getFast(s.data(), s.elms()))
   {
      Memt<Byte, SNAPPY_COMPRESSBOUND(SNAPPY_BUF_SIZE)> d; d.setNum(SNAPPYSize(s.elms())); UIntPtr dest_size=d.elms();
      if(SNAPPYCompress(s.data(), s.elms(), d.data(), dest_size))
      {
         if(callback)callback->data(s.data(), s.elms());
         s.del(); // delete 's' first to release memory
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
NOINLINE static Bool SNAPPYDecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Memt<Byte, SNAPPY_COMPRESSBOUND(SNAPPY_BUF_SIZE)> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))goto error;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(SNAPPYDecompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)
      {
         if(callback)callback->data(dest.memFast(), dest_size);
         return MemWrote(dest, dest_size);
      }
   }else
   {
      Memt<Byte, SNAPPY_BUF_SIZE> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(SNAPPYDecompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         if(callback)callback->data(d.data(), dest_size);
         return dest.put(d.data(), dest_size);
      }
   }
error:
   return false;
}
/******************************************************************************/
NOINLINE static Bool SNAPPYCompressStream(File &src, File &dest, DataCallback *callback)
{
   Byte s[SNAPPY_BUF_SIZE], d[SNAPPY_COMPRESSBOUND(SNAPPY_BUF_SIZE)];
   for(; !src.end(); )
   {
      Int read=src.getReturnSize(s, SIZE(s)); if(read<=0)goto error;
      UIntPtr dest_size=SIZE(d); if(!SNAPPYCompress(s, read, d, dest_size))goto error;
      if(callback)callback->data(s, read);
      dest.cmpUIntV(dest_size-1); if(!dest.put(d, dest_size))goto error;
   }
   return dest.ok();
error:
   return false;
}
NOINLINE static Bool SNAPPYDecompressStream(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Byte s[SNAPPY_COMPRESSBOUND(SNAPPY_BUF_SIZE)], d[SNAPPY_BUF_SIZE];
   Bool ok=false, direct=MemDecompress(dest, decompressed_size);
  ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
   Long start=dest.pos();
   for(; !src.end(); )
   {
      UInt chunk=src.decUIntV()+1; if(chunk>SIZE(s))goto error;
      if(!src.getFast(s, chunk))goto error; // needs exactly 'chunk' amount
      if(direct)
      {
         UIntPtr dest_size=dest.left(); if(!SNAPPYDecompress(s, chunk, dest.memFast(), dest_size))goto error;
         if(callback)callback->data(dest.memFast(), dest_size);
         if(!MemWrote(dest, dest_size))goto error;
      }else
      {
         UIntPtr dest_size=SIZE(d); if(!SNAPPYDecompress(s, chunk, d, dest_size))goto error;
         if(callback)callback->data(d, dest_size);
         if(!dest.put(d, dest_size))goto error;
      }
   }
   if(src.ok() && dest.pos()-start==decompressed_size)ok=true;
error:
   src.unlimit(temp_size, temp_offset);
   return ok;
}
/******************************************************************************/
// keep as separate functions, because both of them use a lot of stack memory which could crash if combined together
static Bool SNAPPYCompress(File &src, File &dest, DataCallback *callback)
{
   return (src.left()<=SNAPPY_BUF_SIZE) ? SNAPPYCompressMem   (src, dest, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                        : SNAPPYCompressStream(src, dest, callback);
}
static Bool SNAPPYDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   return (decompressed_size<=SNAPPY_BUF_SIZE) ? SNAPPYDecompressMem   (src, dest, compressed_size, decompressed_size, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                               : SNAPPYDecompressStream(src, dest, compressed_size, decompressed_size, callback);
}
#endif
/******************************************************************************/
// RLE
/******************************************************************************/
#if SUPPORT_RLE
Int CmpUIntVSize(UInt u)
{
   Int    i=1; for(; u>=128; u>>=7)i++;
   return i;
}
static void CmpUIntV(UInt u, Byte *data, UIntPtr &pos)
{
   data[pos++]=(u&127)|((u>=128)<<7);
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
               u>>=7; data[pos++]=u;
            }
         }
      }
   }
}
static Bool DecUIntV(UInt &u, C Byte *data, UIntPtr size, UIntPtr &pos)
{
   if(!InRange(pos, size))return false; Byte v=data[pos++]; u=(v&127);
   if(v&128)
   {
      if(!InRange(pos, size))return false; v=data[pos++]; u|=((v&127)<<7);
      if(v&128)
      {
         if(!InRange(pos, size))return false; v=data[pos++]; u|=((v&127)<<(7+7));
         if(v&128)
         {
            if(!InRange(pos, size))return false; v=data[pos++]; u|=((v&127)<<(7+7+7));
            if(v&128)
            {
               if(!InRange(pos, size))return false; v=data[pos++]; u|=((v&127)<<(7+7+7+7));
            }
         }
      }
   }
   return true;
}
/******************************************************************************/
static UIntPtr RLESize(UIntPtr src_size) {return src_size+MaxCmpUIntVSize;}

static Bool RLECompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, false on fail
{
   if(dest_size<RLESize(src_size))return false;
    IntPtr to=src_size-2;
   UIntPtr src_pos=0, dest_pos=0;
 C Byte *s=(Byte*)src;
   Byte *d=(Byte*)dest;
   for(; src_pos<src_size; )
   {
   find_equal:
      for(IntPtr i=src_pos; i<to; i++)
      {
         Byte b=s[i]; if(b==s[i+1] && b==s[i+2]) // find 3 equal
         {
            IntPtr different=i-src_pos; if(different>0)
            {
               UIntPtr count=(different-1)<<1, count_size=CmpUIntVSize(count);
               if(count_size>1) // if the counter size is big, then we need to check some extra data for matching 'b' byte, otherwise we might be losing space instead of gaining, when compressing the 3 bytes, because the 'count' marker uses size too (for example 65 different bytes, 3 same bytes, 65 different bytes, would generate 2-byte size + 65-byte data + 1-byte size + 1-byte data + 2-byte size + 65-byte data, losing 2-bytes on different data but gaining only 1-byte on same data)
               {
                  IntPtr j=i+3, k=j+count_size-2+1; // j=first index to check, k=last index to check (inclusive, add extra 1 to avoid switching between same/different chunks, to avoid CPU overhead, when the size would be the same in both cases 1)one big different 2)different+same)
                  if(!InRange(k, src_size))goto too_short_range; // out of range (skip entirely because there's no data after these 3 for a possible larger match)
                  for(; j<=k; j++)if(s[j]!=b){i=j-1; goto too_short_data;} // different byte (continue looking for another match, set "i=j-1" because 'i' will get ++ in the 'for' loop)
               }

               // write any different before those 3
               CmpUIntV(count, d, dest_pos);
               CopyFast(d+dest_pos, s+src_pos, different); dest_pos+=different; src_pos+=different;
            }

            // calculate how many same bytes are there, now "i==src_pos"
            for(IntPtr j=i+3; ; j++)if(!InRange(j, src_size) || s[j]!=b) // out of range or different byte
            {
               // write i .. j-1
               IntPtr equal=j-i;
               CmpUIntV(((equal-3)<<1)|1, d, dest_pos);
               d[dest_pos++]=b;
               src_pos=j;
               break;
            }

            goto find_equal; // after finding equal we can proceed again with the search
         }
      too_short_data:;
      }
   too_short_range:

      // didn't found any 3 equal, write remaining different
      IntPtr different=src_size-src_pos; if(different>0)
      {
         CmpUIntV((different-1)<<1, d, dest_pos);
         CopyFast(d+dest_pos, s+src_pos, different); dest_pos+=different; //src_pos+=different; not needed since we're finishing
      }
      break; // finish
   }
   dest_size=dest_pos; return true;
}
static Bool RLEDecompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
 C Byte *s=(Byte*)src;
   Byte *d=(Byte*)dest;
   UIntPtr src_pos=0, dest_pos=0;
   for(; src_pos<src_size; )
   {
      UInt length; if(!DecUIntV(length, s, src_size, src_pos))return false;
      if(  length&0x1)
      {
         length=(length>>1)+3;
         if(dest_pos+length > dest_size
         ||  src_pos        >= src_size)return false;
         SetMem(d+dest_pos, s[src_pos++], length); dest_pos+=length;
      }else
      {
         length=(length>>1)+1;
         if(dest_pos+length > dest_size
         ||  src_pos+length >  src_size)return false;
         CopyFast(d+dest_pos, s+src_pos, length); dest_pos+=length; src_pos+=length;
      }
   }
   dest_size=dest_pos; return true;
}
/******************************************************************************/
NOINLINE static Bool RLECompressMem(File &src, File &dest)
{
   Memt<Byte> s, d;
   s.setNum(src.left()); if(!src.getFast(s.data(), s.elms()))return false;
   d.setNum(RLESize(s.elms())); UIntPtr dest_size=d.elms();
   if(RLECompress(s.data(), s.elms(), d.data(), dest_size))
   {
      s.del(); // delete 's' first to release memory
      return dest.put(d.data(), dest_size);
   }
   return false;
}
NOINLINE static Bool RLEDecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size)
{
   Memt<Byte> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))return false;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(RLEDecompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)return MemWrote(dest, dest_size);
   }else
   {
      Memt<Byte> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(RLEDecompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
/******************************************************************************/
NOINLINE static Bool RLECompress(File &src, File &dest, DataCallback *callback)
{
   Memt<Byte> sm; sm.setNum(src.left()); Byte *s=sm.data(); if(src.getFast(s, sm.elms()))
   {
      if(callback)callback->data(s, sm.elms());
      Int   src_pos=0, to=sm.elms()-2;
      for(; src_pos<sm.elms(); )
      {
      find_equal:
         for(Int i=src_pos; i<to; i++)
         {
            Byte b=s[i]; if(b==s[i+1] && b==s[i+2]) // find 3 equal
            {
               Int different=i-src_pos; if(different>0)
               {
                  UInt count=(different-1)<<1, count_size=CmpUIntVSize(count);
                  if(count_size>1) // if the counter size is big, then we need to check some extra data for matching 'b' byte, otherwise we might be losing space instead of gaining, when compressing the 3 bytes, because the 'count' marker uses size too (for example 65 different bytes, 3 same bytes, 65 different bytes, would generate 2-byte size + 65-byte data + 1-byte size + 1-byte data + 2-byte size + 65-byte data, losing 2-bytes on different data but gaining only 1-byte on same data)
                  {
                     Int j=i+3, k=j+count_size-2+1; // j=first index to check, k=last index to check (inclusive, add extra 1 to avoid switching between same/different chunks, to avoid CPU overhead, when the size would be the same in both cases 1)one big different 2)different+same)
                     if(!InRange(k, sm))goto too_short_range; // out of range (skip entirely because there's no data after these 3 for a possible larger match)
                     for(; j<=k; j++)if(s[j]!=b){i=j-1; goto too_short_data;} // different byte (continue looking for another match, set "i=j-1" because 'i' will get ++ in the 'for' loop)
                  }

                  // write any different before those 3
                  dest.cmpUIntV(count);
                  if(!dest.put(s+src_pos, different))return false; src_pos+=different;
               }

               // calculate how many same bytes are there, now "i==src_pos"
               for(Int j=i+3; ; j++)if(!InRange(j, sm) || s[j]!=b) // out of range or different byte
               {
                  // write i .. j-1
                  Int equal=j-i;
                  dest.cmpUIntV(((equal-3)<<1)|1).putByte(b);
                  src_pos=j;
                  break;
               }

               goto find_equal; // after finding equal we can proceed again with the search
            }
         too_short_data:;
         }
      too_short_range:

         // didn't found any 3 equal, write remaining different
         Int different=sm.elms()-src_pos; if(different>0)
         {
            dest.cmpUIntV((different-1)<<1);
            if(!dest.put(s+src_pos, different))return false; //src_pos+=different; not needed since we're finishing
         }
         break; // finish
      }
      return dest.ok();
   }
   return false;
}
static Bool RLEDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
  ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
   Long start=dest.pos();
   Bool direct=MemDecompress(dest, decompressed_size), ok=false;
   for(; !src.end(); )
   {
      UInt length; src.decUIntV(length);
      if(  length&0x1)
      {
         length=(length>>1)+3;
         Byte b=src.getByte();
         if(direct)
         {
            if(dest.left()<length)goto error;
            SetMem(dest.memFast(), b, length);
            if(callback)callback->data(dest.memFast(), length);
            if(!MemWrote(dest, length))goto error;
         }else
         {
            if(callback)REP(length)callback->data(&b, SIZE(b));
            REP(length)dest.putByte(b);
         }
      }else
      {
         length=(length>>1)+1;
         if(callback ? !src.copy(dest, *callback, length)
                     : !src.copy(dest,            length))goto error;
      }
   }
   ok=(src.ok() && dest.ok() && dest.pos()-start==decompressed_size);
error:
   src.unlimit(temp_size, temp_offset);
   return ok;
}
#endif
/******************************************************************************/
// LZ4
/******************************************************************************/
#if SUPPORT_LZ4
#define LZ4_BUF_SIZE 65536 // headers say that up to 64Kb need to be kept in memory !! don't change in the future !!

static UIntPtr LZ4Size(UIntPtr size) {return LZ4_compressBound(size);}

static Bool LZ4Compress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size, Int compression_level) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, false on fail
{
   auto size=((compression_level>0) ? LZ4_compress_HC((char*)src, (char*)dest, src_size, dest_size, compression_level) : LZ4_compress_default((char*)src, (char*)dest, src_size, dest_size));
   if(  size>0){dest_size=size; return true;}
   return false;
}
static Bool LZ4Decompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   auto size =LZ4_decompress_safe((char*)src, (char*)dest, src_size, dest_size);
   if(  size>=0){dest_size=size; return true;} // must check for >=0 because <0 are errors
   return false;
}
/******************************************************************************/
NOINLINE static Bool LZ4CompressMem(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   Memt<Byte, LZ4_BUF_SIZE> s; s.setNum(src.left()); if(src.getFast(s.data(), s.elms()))
   {
      Memt<Byte, LZ4_COMPRESSBOUND(LZ4_BUF_SIZE)> d; d.setNum(LZ4Size(s.elms())); UIntPtr dest_size=d.elms();
      if(LZ4Compress(s.data(), s.elms(), d.data(), dest_size, compression_level))
      {
         s.del(); // delete 's' first to release memory
         if(callback)callback->data(s.data(), s.elms());
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
NOINLINE static Bool LZ4DecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Memt<Byte, LZ4_COMPRESSBOUND(LZ4_BUF_SIZE)> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))goto error;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(LZ4Decompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)
      {
         if(callback)callback->data(dest.memFast(), dest_size);
         return MemWrote(dest, dest_size);
      }
   }else
   {
      Memt<Byte, LZ4_BUF_SIZE> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(LZ4Decompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         if(callback)callback->data(d.data(), dest_size);
         return dest.put(d.data(), dest_size);
      }
   }
error:
   return false;
}
/******************************************************************************/
NOINLINE static Bool LZ4CompressStream(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   Byte s[LZ4_BUF_SIZE*2], d[LZ4_COMPRESSBOUND(LZ4_BUF_SIZE)]; Int s_pos=0;
   union
   {
      LZ4_streamHC_t lz4_hc;
      LZ4_stream_t   lz4;
   };
   if(compression_level>0)LZ4_resetStreamHC(&lz4_hc, compression_level);
   else                   LZ4_resetStream  (&lz4);
   for(; !src.end(); )
   {
      Int read=Min(Min(LZ4_BUF_SIZE, SIZEI(s)), src.left());
      if(s_pos>SIZE(s)-read)s_pos=0; // if reading will exceed buffer size
      read=src.getReturnSize(&s[s_pos], read); if(read<=0)goto error;
      if(callback)callback->data(&s[s_pos], read);
      auto size=((compression_level>0) ? LZ4_compress_HC_continue  (&lz4_hc, (char*)&s[s_pos], (char*)d, read, SIZE(d)   )
                                       : LZ4_compress_fast_continue(&lz4   , (char*)&s[s_pos], (char*)d, read, SIZE(d), 1));
      if(size>0)
      {
         dest.cmpUIntV(size-1);
         if(!dest.put(d, size))goto error;
      }else goto error;
      s_pos+=read;
   }
   return dest.ok();
error:
   return false;
}
NOINLINE static Bool LZ4DecompressStream(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Bool ok=false;
   LZ4_streamDecode_t lz4; if(LZ4_setStreamDecode(&lz4, null, 0))
   {
     ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Int  cipher_offset=dest.posCipher();
      Long start=dest.pos();
      Ptr  mem=dest.memFast();
      Byte s[LZ4_COMPRESSBOUND(LZ4_BUF_SIZE)], d[LZ4_BUF_SIZE*2]; Int d_pos=0;
      Bool direct=MemDecompress(dest, decompressed_size);
      for(; !src.end(); )
      {
         UInt chunk=src.decUIntV()+1; if(chunk>SIZE(s))goto error;
         if(!src.getFast(s, chunk))goto error; // need exactly 'chunk' amount
         if(direct)
         {
            auto size=LZ4_decompress_safe_continue(&lz4, (char*)s, (char*)dest.memFast(), chunk, dest.left()); if(size<0)goto error;
            if(callback)callback->data(dest.memFast(), size);
            if(!dest.skip(size))goto error; // can't use 'MemWrote' and 'Cipher' here because LZ4 may still access previously decompressed data
         }else
         {
            if(d_pos>SIZE(d)-LZ4_BUF_SIZE)d_pos=0; // if writing will exceed buffer size (this assumes that up to LZ4_BUF_SIZE can be written at one time)
            auto size=LZ4_decompress_safe_continue(&lz4, (char*)s, (char*)d+d_pos, chunk, SIZE(d)-d_pos); if(size<0)goto error;
            if(callback)callback->data(d+d_pos, size);
            if(!dest.put(d+d_pos, size))goto error; d_pos+=size;
         }
      }
      {
         Long wrote=dest.pos()-start;
         if(direct)MemFinished(dest, mem, wrote, cipher_offset);
         if(src.ok() && wrote==decompressed_size)ok=true;
      }
   error:
      src.unlimit(temp_size, temp_offset);
   }
   return ok;
}
/******************************************************************************/
// keep as separate functions, because both of them use a lot of stack memory which could crash if combined together
static Bool LZ4Compress(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   return (src.left()<=LZ4_BUF_SIZE) ? LZ4CompressMem   (src, dest, compression_level, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                     : LZ4CompressStream(src, dest, compression_level, callback);
}
static Bool LZ4Decompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   return (decompressed_size<=LZ4_BUF_SIZE) ? LZ4DecompressMem   (src, dest, compressed_size, decompressed_size, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                            : LZ4DecompressStream(src, dest, compressed_size, decompressed_size, callback);
}
#endif
/******************************************************************************/
// LIZARD
/******************************************************************************/
#if SUPPORT_LIZARD
#define LIZARD_BUF_SIZE 65536 // headers say that up to 64Kb need to be kept in memory !! don't change in the future !!

static UIntPtr LIZARDSize(UIntPtr size) {return Lizard_compressBound(size);}

static Bool LIZARDCompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size, Int compression_level) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, false on fail
{
   auto size=Lizard_compress((char*)src, (char*)dest, src_size, dest_size, Mid(compression_level, LIZARD_MIN_CLEVEL, LIZARD_MAX_CLEVEL));
   if(  size>0){dest_size=size; return true;}
   return false;
}
static Bool LIZARDDecompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   auto size =Lizard_decompress_safe((char*)src, (char*)dest, src_size, dest_size);
   if(  size>=0){dest_size=size; return true;} // must check for >=0 because <0 are errors
   return false;
}
/******************************************************************************/
NOINLINE static Bool LIZARDCompressMem(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   Memt<Byte, LIZARD_BUF_SIZE> s; s.setNum(src.left()); if(src.getFast(s.data(), s.elms()))
   {
      Memt<Byte, LIZARD_COMPRESSBOUND(LIZARD_BUF_SIZE)> d; d.setNum(LIZARDSize(s.elms())); UIntPtr dest_size=d.elms();
      if(LIZARDCompress(s.data(), s.elms(), d.data(), dest_size, Mid(compression_level, LIZARD_MIN_CLEVEL, LIZARD_MAX_CLEVEL)))
      {
         s.del(); // delete 's' first to release memory
         if(callback)callback->data(s.data(), s.elms());
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
NOINLINE static Bool LIZARDDecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Memt<Byte, LIZARD_COMPRESSBOUND(LIZARD_BUF_SIZE)> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))goto error;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(LIZARDDecompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)
      {
         if(callback)callback->data(dest.memFast(), dest_size);
         return MemWrote(dest, dest_size);
      }
   }else
   {
      Memt<Byte, LIZARD_BUF_SIZE> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(LIZARDDecompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         if(callback)callback->data(d.data(), dest_size);
         return dest.put(d.data(), dest_size);
      }
   }
error:
   return false;
}
/******************************************************************************/
NOINLINE static Bool LIZARDCompressStream(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   Bool ok=false;
   if(Lizard_stream_t *lizard=Lizard_createStream(Mid(compression_level, LIZARD_MIN_CLEVEL, LIZARD_MAX_CLEVEL)))
   {
      Byte s[LIZARD_BUF_SIZE*2], d[LIZARD_COMPRESSBOUND(LIZARD_BUF_SIZE)]; Int s_pos=0;
      for(; !src.end(); )
      {
         Int read=Min(Min(LIZARD_BUF_SIZE, SIZEI(s)), src.left());
         if(s_pos>SIZE(s)-read)s_pos=0; // if reading will exceed buffer size
         read=src.getReturnSize(&s[s_pos], read); if(read<=0)goto error;
         if(callback)callback->data(&s[s_pos], read);
         auto size=Lizard_compress_continue(lizard, (char*)&s[s_pos], (char*)d, read, SIZE(d));
         if(size>0)
         {
            dest.cmpUIntV(size-1);
            if(!dest.put(d, size))goto error;
         }else goto error;
         s_pos+=read;
      }
      ok=dest.ok();
   error:
      Lizard_freeStream(lizard);
   }
   return ok;
}
NOINLINE static Bool LIZARDDecompressStream(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Bool ok=false;
   Lizard_streamDecode_t lizard; if(Lizard_setStreamDecode(&lizard, null, 0))
   {
     ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Int  cipher_offset=dest.posCipher();
      Long start=dest.pos();
      Ptr  mem=dest.memFast();
      Byte s[LIZARD_COMPRESSBOUND(LIZARD_BUF_SIZE)], d[LIZARD_BUF_SIZE*2]; Int d_pos=0;
      Bool direct=MemDecompress(dest, decompressed_size);
      for(; !src.end(); )
      {
         UInt chunk=src.decUIntV()+1; if(chunk>SIZE(s))goto error;
         if(!src.getFast(s, chunk))goto error; // need exactly 'chunk' amount
         if(direct)
         {
            auto size=Lizard_decompress_safe_continue(&lizard, (char*)s, (char*)dest.memFast(), chunk, dest.left()); if(size<0)goto error;
            if(callback)callback->data(dest.memFast(), size);
            if(!dest.skip(size))goto error; // can't use 'MemWrote' and 'Cipher' here because LIZARD may still access previously decompressed data
         }else
         {
            if(d_pos>SIZE(d)-LIZARD_BUF_SIZE)d_pos=0; // if writing will exceed buffer size (this assumes that up to LIZARD_BUF_SIZE can be written at one time)
            auto size=Lizard_decompress_safe_continue(&lizard, (char*)s, (char*)d+d_pos, chunk, SIZE(d)-d_pos); if(size<0)goto error;
            if(callback)callback->data(d+d_pos, size);
            if(!dest.put(d+d_pos, size))goto error; d_pos+=size;
         }
      }
      {
         Long wrote=dest.pos()-start;
         if(direct)MemFinished(dest, mem, wrote, cipher_offset);
         if(src.ok() && wrote==decompressed_size)ok=true;
      }
   error:
      src.unlimit(temp_size, temp_offset);
   }
   return ok;
}
/******************************************************************************/
// keep as separate functions, because both of them use a lot of stack memory which could crash if combined together
static Bool LIZARDCompress(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   return (src.left()<=LIZARD_BUF_SIZE) ? LIZARDCompressMem   (src, dest, compression_level, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                        : LIZARDCompressStream(src, dest, compression_level, callback);
}
static Bool LIZARDDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   return (decompressed_size<=LIZARD_BUF_SIZE) ? LIZARDDecompressMem   (src, dest, compressed_size, decompressed_size, callback) // for small files prefer in-memory compression to avoid writing chunk sizes !! once placed here, don't change in the future because will break compatibility !!
                                               : LIZARDDecompressStream(src, dest, compressed_size, decompressed_size, callback);
}
#endif
/******************************************************************************/
// ZSTD
#if SUPPORT_ZSTD
ASSERT(ZSTD_BLOCKSIZE_MAX==128*1024); // this value needs to be constant

static ZSTD_customMem ZSTDMem={CompressAlloc, CompressFree, null};

static UIntPtr ZSTDSize(UIntPtr size) {return ZSTD_compressBound(size);}

static UInt ZSTDDictSizeLog2(Long size)
{
   return (size<0) ? ZSTD_WINDOWLOG_MAX_32 // if the size is unknown, then use max possible dict size
                   : Mid(Log2Ceil(Unsigned(size)), ZSTD_WINDOWLOG_MIN, ZSTD_WINDOWLOG_MAX_32); // always limit to ZSTD_WINDOWLOG_MAX_32 ignoring ZSTD_WINDOWLOG_MAX_64 to keep consistency for both platforms
}
/******************************************************************************
static Bool ZSTDCompressFrame(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size, Int compression_level) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, false on fail
{
   auto size=ZSTD_compress(dest, dest_size, src, src_size, Mid(compression_level, 1, ZSTD_maxCLevel()));
   if(!ZSTD_isError(size)){dest_size=size; return true;}
   return false;
}
static Bool ZSTDDecompressFrame(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   auto size=ZSTD_decompress(dest, dest_size, src, src_size);
   if(!ZSTD_isError(size)){dest_size=size; return true;}
   return false;
}
/******************************************************************************
NOINLINE static Bool ZSTDCompressMemFrame(File &src, File &dest, Int compression_level)
{
   Memt<Byte> s, d;
   s.setNum(src.left()); if(!src.getFast(s.data(), s.elms()))return false;
   d.setNum(ZSTDSize(s.elms())); UIntPtr dest_size=d.elms();
   if(ZSTDCompressFrame(s.data(), s.elms(), d.data(), dest_size, compression_level))
   {
      s.del(); // delete 's' first to release memory
      return dest.put(d.data(), dest_size);
   }
   return false;
}
NOINLINE static Bool ZSTDDecompressMemFrame(File &src, File &dest, Long compressed_size, Long decompressed_size)
{
   Memt<Byte> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))return false;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(ZSTDDecompressFrame(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)return MemWrote(dest, dest_size);
   }else
   {
      Memt<Byte> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(ZSTDDecompressFrame(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
/******************************************************************************
NOINLINE static Bool ZSTDCompressFrame(File &src, File &dest, Int compression_level)
{
   Bool ok=false;
   if(ZSTD_CCtx *ctx=ZSTD_createCCtx_advanced(ZSTDMem))
   {
      ZSTD_parameters params=ZSTD_getParams(Mid(compression_level, 1, ZSTD_maxCLevel()), src.left(), 0);
   #if 0 // don't write anything to speedup processing
      params.fParams.contentSizeFlag=true;
      params.fParams.   noDictIDFlag=true;
      params.fParams.   checksumFlag=true;
   #endif
      if(!ZSTD_isError(ZSTD_compressBegin_advanced(ctx, null, 0, params, src.left())))
      {
         // sizes for 'window_size', 'block_size', 's', 'd' were taken from "zstd" tutorial, "zbuff_compress.c" file, "ZBUFF_compressInit_advanced" function
       C Int window_size=1<<params.cParams.windowLog, block_size=Min(window_size, ZSTD_BLOCKSIZE_MAX);
         Memt<Byte> s, d; s.setNum(window_size+block_size); d.setNum(ZSTDSize(block_size)+1); Int s_pos=0;
         for(; !src.end(); )
         {
            Int read=Min(ZSTD_BLOCKSIZE_MAX, Min(s.elms(), src.left())); // ZSTD_BLOCKSIZE_MAX taken from 'ZBUFF_recommendedCInSize' (without this, 'ZSTD_compressContinue' may fail with 'dest' too small error)
            if(s_pos>s.elms()-read)s_pos=0; // if reading will exceed buffer size
            read=src.getReturnSize(&s[s_pos], read); if(read<=0)goto error;
            auto size=ZSTD_compressContinue(ctx, d.data(), d.elms(), &s[s_pos], read); if(ZSTD_isError(size))goto error;
            if(!dest.put(d.data(), size))goto error;
            s_pos+=read;
         }
         auto size=ZSTD_compressEnd(ctx, d.data(), d.elms()); if(ZSTD_isError(size))goto error;
         if(dest.put(d.data(), size))ok=true;
      }
   error:
      ZSTD_freeCCtx(ctx);
   }
   return ok;
}
NOINLINE static Bool ZSTDDecompressFrame(File &src, File &dest, Long compressed_size, Long decompressed_size)
{
   Bool ok=false;
   if(ZSTD_DCtx *ctx=ZSTD_createDCtx_advanced(ZSTDMem))
   {
      ZSTD_decompressBegin(ctx);
     ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Byte header[ZSTD_frameHeaderSize_max];
      Long pos=src.pos();
      Int read=src.getReturnSize(header, SIZE(header));
      src.pos(pos);
      ZSTD_frameParams frame; if(!ZSTD_getFrameParams(&frame, header, read))
      {
         Int  cipher_offset=dest.posCipher();
         Long start=dest.pos();
         Ptr  mem=dest.memFast();
         // sizes for 'block_size', 's', 'd' were taken from "zstd" tutorial, "zbuff_decompress.c" file, "ZBUFF_decompressContinue" function
       C auto block_size=Min(frame.windowSize, ZSTD_BLOCKSIZE_MAX);
         Memt<Byte> s, d; s.setNum(block_size); Int d_pos=0;
         Bool direct=MemDecompress(dest, decompressed_size); if(!direct)d.setNum(frame.windowSize+block_size);
         for(;;)
         {
            auto size=ZSTD_nextSrcSizeToDecompress(ctx); if(!size){if(dest.pos()-start==decompressed_size)ok=true; break;} if(ZSTD_isError(size) || size>s.elms())break;
            if(!src.getFast(s.data(), size))break; // need exactly 'size' amount
            if(direct)
            {
               size=ZSTD_decompressContinue(ctx, dest.memFast(), dest.left(), s.data(), size); if(ZSTD_isError(size))break;
               if(!dest.skip(size))break; // can't use 'MemWrote' and 'Cipher' here because ZSTD may still access previously decompressed data
            }else
            {
               if(d_pos>d.elms()-block_size)d_pos=0; // if decompressing will exceed buffer size
               size=ZSTD_decompressContinue(ctx, &d[d_pos], d.elms()-d_pos, s.data(), size); if(ZSTD_isError(size))break;
               if(size>0){if(!dest.put(&d[d_pos], size))break; d_pos+=size;}
            }
         }
         if(direct)MemFinished(dest, mem, dest.pos()-start, cipher_offset);
      }
      src.unlimit(temp_size, temp_offset);
      ZSTD_freeDCtx(ctx);
   }
   return ok;
}
/******************************************************************************/
NOINLINE static Bool ZSTDCompress(File &src, File &dest, Int compression_level, DataCallback *callback)
{
   Bool ok=false;
   if(ZSTD_CCtx *ctx=ZSTD_createCCtx_advanced(ZSTDMem))
   {
      ZSTD_parameters params=ZSTD_getParams(Mid(compression_level, 1, ZSTD_maxCLevel()), src.left(), 0);
   #if 0 // don't write anything to speedup processing (for block-based this is most likely ignored)
      params.fParams.contentSizeFlag=true;
      params.fParams.   noDictIDFlag=true;
      params.fParams.   checksumFlag=true;
   #endif
   #if ZSTD_WINDOWLOG_SAVE
      dest.putByte(params.cParams.windowLog);
   #else
      params.cParams.windowLog=ZSTDDictSizeLog2(src.left());
   #endif
      if(!ZSTD_isError(ZSTD_compressBegin_advanced(ctx, null, 0, params, src.left())))
      {
         Bool single=(src.left()<=ZSTD_BLOCKSIZE_MAX);
       C Int  window_size=1<<params.cParams.windowLog;
         Memt<Byte> s, d; Int s_pos=0;
         s.setNum(         Min(window_size+ZSTD_BLOCKSIZE_MAX, src.left()) );
         d.setNum(ZSTDSize(Min(            ZSTD_BLOCKSIZE_MAX, src.left())));
         for(; !src.end(); )
         {
            Int read=Min(ZSTD_BLOCKSIZE_MAX, Min(s.elms(), src.left()));
            if(s_pos>s.elms()-read)s_pos=0; // if reading will exceed buffer size
            read=src.getReturnSize(&s[s_pos], read); if(read<=0)goto error;
            if(callback)callback->data(&s[s_pos], read);
            auto size=ZSTD_compressBlock(ctx, d.data(), d.elms(), &s[s_pos], read); if(ZSTD_isError(size))goto error; // 'ZSTD_compressBlock' returns 0 if failed to compress
            dest.cmpUIntV(single ? (size>0) : size); // for single we store (compressed ? 1 : 0), for multi we store (compressed ? size : 0)
            if(size>0){if(!dest.put( d.data(), size))goto error;} // compressed
            else      {if(!dest.put(&s[s_pos], read))goto error;} // failed to compress
            s_pos+=read;
         }
         if(dest.ok())ok=true;
      }
   error:
      ZSTD_freeCCtx(ctx);
   }
   return ok;
}
NOINLINE static Bool ZSTDDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   Bool ok=false;
   if(ZSTD_DCtx *ctx=ZSTD_createDCtx_advanced(ZSTDMem))
   {
      ZSTD_decompressBegin(ctx);
     ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Int  cipher_offset=dest.posCipher();
      Long start=dest.pos();
      Ptr  mem=dest.memFast();
      Bool single=(decompressed_size<=ZSTD_BLOCKSIZE_MAX);
   #if ZSTD_WINDOWLOG_SAVE
    C Int  window_size=1<<src.getByte();
   #else
    C Int  window_size=1<<ZSTDDictSizeLog2(decompressed_size);
   #endif
      Memt<Byte> s, d; s.setNum(Min((Int)ZSTDSize(ZSTD_BLOCKSIZE_MAX), compressed_size)); Int d_pos=0;
      Bool direct=MemDecompress(dest, decompressed_size); if(!direct)d.setNum(window_size+ZSTD_BLOCKSIZE_MAX); // here don't apply "Min(decompressed_size,", so "if(d_pos>d.elms()-ZSTD_BLOCKSIZE_MAX)" doesn't wrap the 'd_pos' to 0
      for(; !src.end(); )
      {
         UInt chunk; src.decUIntV(chunk);
         if(  chunk) // compressed
         {
            if(single)
            {
               if(chunk>1)goto error; // for single 'chunk' can be either 0 or 1
               chunk=src.left();
            }
            if(chunk>s.elms())goto error;
            if(!src.getFast(s.data(), chunk))goto error; // need exactly 'chunk' amount
            if(direct)
            {
               auto size=ZSTD_decompressBlock(ctx, dest.memFast(), dest.left(), s.data(), chunk); if(ZSTD_isError(size))goto error;
               if(callback)callback->data(dest.memFast(), size);
               if(!dest.skip(size))goto error; // can't use 'MemWrote' and 'Cipher' here because ZSTD may still access previously decompressed data
            }else
            {
               if(d_pos>d.elms()-ZSTD_BLOCKSIZE_MAX)d_pos=0; // if decompressing will exceed buffer size
               auto size=ZSTD_decompressBlock(ctx, &d[d_pos], d.elms()-d_pos, s.data(), chunk); if(ZSTD_isError(size))goto error;
               if(callback)callback->data(&d[d_pos], size);
               if(!dest.put(&d[d_pos], size))goto error; d_pos+=size;
            }
         }else // un-compressed
         {
            Int size=Min(ZSTD_BLOCKSIZE_MAX, src.left()); // for 'single' this will have the same value, so we can ignore it
            if(direct)
            {
               Ptr mem=dest.memFast();
               if(!src.getFast(mem, size)
               || ZSTD_isError(ZSTD_insertBlock(ctx, mem, size))
               || !dest.skip(size))goto error; // can't use 'MemWrote' and 'Cipher' here because ZSTD may still access previously decompressed data
               if(callback)callback->data(mem, size);
            }else
            {
               if(d_pos>d.elms()-size) // if reading will exceed buffer size
               {
                  if(size>d.elms())goto error; // if reading doesn't fit in buffer size
                  d_pos=0;
               }
               if(!src.getFast(&d[d_pos], size)
               || ZSTD_isError(ZSTD_insertBlock(ctx, &d[d_pos], size))
               || !dest.put(&d[d_pos], size))goto error;
               if(callback)callback->data(&d[d_pos], size);
               d_pos+=size;
            }
         }
      }
      {
         Long wrote=dest.pos()-start;
         if(direct)MemFinished(dest, mem, wrote, cipher_offset);
         if(src.ok() && wrote==decompressed_size)ok=true;
      }
   error:
      src.unlimit(temp_size, temp_offset);
      ZSTD_freeDCtx(ctx);
   }
   return ok;
}
#endif
/******************************************************************************/
// BROTLI
/******************************************************************************/
#if SUPPORT_BROTLI
static UIntPtr BrotliSize(UIntPtr size) {return BrotliEncoderMaxCompressedSize(size);}

static UInt BrotliDictSizeLog2(Long size)
{
   return (size<0) ? kBrotliMaxWindowBits // if the size is unknown, then use max possible dict size
                   : Mid(Log2Ceil(Unsigned(size)), kBrotliMinWindowBits, kBrotliMaxWindowBits);
}

static Bool BrotliCompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size, Int compression_level) // compress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to compressed size, 'compression_level'=(0..9) (0=fastest/worst, 9=slowest/best), false on fail
{
   size_t size=dest_size; Bool   ok=(BrotliEncoderCompress(Mid(compression_level, 0, 11), BrotliDictSizeLog2(src_size), BROTLI_DEFAULT_MODE, src_size, (uint8_t*)src, &size, (uint8_t*)dest)==1);
     dest_size=     size; return ok;
}
static Bool BrotliDecompress(CPtr src, UIntPtr src_size, Ptr dest, UIntPtr &dest_size) // decompress data, 'src'=source buffer, 'src_size'=source size, 'dest'=destination buffer, 'dest_size'=destination size, before calling it should be set to maximum 'dest' buffer capacity, after calling it'll be set to decompressed size, false on fail
{
   size_t size=dest_size; Bool   ok=(BrotliDecompressBuffer(src_size, (uint8_t*)src, &size, (uint8_t*)dest)==BROTLI_RESULT_SUCCESS);
     dest_size=     size; return ok;
}
/******************************************************************************/
NOINLINE static Bool BrotliCompressMem(File &src, File &dest, Int compression_level)
{
   Memt<Byte> s, d;
   s.setNum(src.left()); if(!src.getFast(s.data(), s.elms()))return false;
   d.setNum(BrotliSize(s.elms())); UIntPtr dest_size=d.elms();
   if(BrotliCompress(s.data(), s.elms(), d.data(), dest_size, compression_level))
   {
      s.del(); // delete 's' first to release memory
      return dest.put(d.data(), dest_size);
   }
   return false;
}
NOINLINE static Bool BrotliDecompressMem(File &src, File &dest, Long compressed_size, Long decompressed_size)
{
   Memt<Byte> s;
   s.setNum(compressed_size); if(!src.getFast(s.data(), s.elms()))return false;
   if(MemDecompress(dest, decompressed_size))
   {
      UIntPtr dest_size=decompressed_size;
      if(BrotliDecompress(s.data(), s.elms(), dest.memFast(), dest_size) && dest_size==decompressed_size)return MemWrote(dest, dest_size);
   }else
   {
      Memt<Byte> d; d.setNum(decompressed_size); UIntPtr dest_size=d.elms();
      if(BrotliDecompress(s.data(), s.elms(), d.data(), dest_size) && dest_size==decompressed_size)
      {
         s.del(); // delete 's' first to release memory
         return dest.put(d.data(), dest_size);
      }
   }
   return false;
}
/******************************************************************************/
NOINLINE static Bool BrotliCompress(File &src, File &dest, Int compression_level)
{
   Bool ok=false;
   if(BrotliEncoderState *brotli=BrotliEncoderCreateInstance(CompressAlloc, CompressFree, null))
   {
      BrotliEncoderSetParameter(brotli, BROTLI_PARAM_MODE   , BROTLI_MODE_GENERIC);
      BrotliEncoderSetParameter(brotli, BROTLI_PARAM_QUALITY, Mid(compression_level, 0, 11));
      BrotliEncoderSetParameter(brotli, BROTLI_PARAM_LGWIN  , BrotliDictSizeLog2(src.left()));
    //BrotliEncoderSetParameter(brotli, BROTLI_PARAM_LGBLOCK, lgblock);

      Byte in[BUF_SIZE], out[BUF_SIZE];
      for(;;)
      {
         Int read=src.getReturnSize(in, SIZE(in)); // always proceed even if 'read'==0 in case of empty files or if we've encountered end of file
         size_t  avail_in=read, avail_out;
       C uint8_t *next_in=in  ;
         do
         {
                    avail_out=SIZE(out);
            uint8_t *next_out=     out ;
            if(!BrotliEncoderCompressStream(brotli, src.end() ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS, &avail_in, &next_in, &avail_out, &next_out, null))goto error;
            if(!dest.put(out, SIZE(out)-avail_out))goto error;
         }while(avail_out==0);
         if(avail_in )goto error; // if there's still some input left, then it failed
         if(src.end()){ok=true; break;}
         if(read<=0  )goto error; // if not end of 'src' but failed to read anything, then it failed
      }
   error:
      BrotliEncoderDestroyInstance(brotli);
   }
   return ok;
}
NOINLINE static Bool BrotliDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size)
{
   Bool ok=false;
   if(BrotliState *brotli=BrotliCreateState(CompressAlloc, CompressFree, null))
   {
      Long  start=dest.pos();
      Byte  in[BUF_SIZE], out[BUF_SIZE];
      ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Bool  direct=MemDecompress(dest, decompressed_size);

      for(;;)
      {
         Int read=src.getReturnSize(in, SIZE(in)); if(read<=0)goto error;
         size_t  avail_in=read,         avail_out;
       C uint8_t *next_in=in  ; uint8_t *next_out;
         BrotliResult result;
         do{
            if(direct)
            {
               avail_out=       dest.left   ();
                next_out=(Byte*)dest.memFast();
            }else
            {
               avail_out=SIZE(out);
                next_out=     out ;
            }
            result=BrotliDecompressStream(&avail_in, &next_in, &avail_out, &next_out, null, brotli);
            if(result==BROTLI_RESULT_ERROR)goto error;
            if(direct ? !MemWrote(dest, dest.left()-avail_out)
                      : !dest.put(out , SIZE(out)  -avail_out))goto error;
         }while(result==BROTLI_RESULT_NEEDS_MORE_OUTPUT);
         if(result==BROTLI_RESULT_SUCCESS){if(dest.pos()-start==decompressed_size)ok=true; break;}
      }
   error:
      src.unlimit(temp_size, temp_offset);
      BrotliDestroyState(brotli);
   }
   return ok;
}
#endif
/******************************************************************************/
// LZHAM
/******************************************************************************/
#if SUPPORT_LZHAM
static UInt LZHAMDictSizeLog2(Long size) // !! don't change this function, because the compression/decompression values must match, so all compressed data from the past must use the same values for the decompressor !!
{
   return (size<0) ? LZHAM_MAX_DICT_SIZE_LOG2_X86 // if the size is unknown, then use max possible dict size
                   : Mid(Log2Ceil(Unsigned(size)), LZHAM_MIN_DICT_SIZE_LOG2, LZHAM_MAX_DICT_SIZE_LOG2_X86); // always limit to LZHAM_MAX_DICT_SIZE_LOG2_X86 ignoring LZHAM_MAX_DICT_SIZE_LOG2_X64 to keep consistency for both platforms
}
static UInt LZHAMDictSize(Long size) {return 1<<LZHAMDictSizeLog2(size);}

NOINLINE static Bool LZHAMCompress(File &src, File &dest, Int compression_level, Bool multi_threaded, DataCallback *callback)
{
   using namespace lzham;
   Bool ok=false;

   lzham_compress_params comp_params; Zero(comp_params);
   comp_params.m_struct_size   =SIZE(comp_params);
   comp_params.m_dict_size_log2=LZHAMDictSizeLog2(src.left());
   comp_params.m_level         =(lzham_compress_level)Mid(compression_level, LZHAM_COMP_LEVEL_FASTEST, LZHAM_TOTAL_COMP_LEVELS-1);
 //if(compression_level>comp_params.m_level)comp_params.m_compress_flags|=LZHAM_COMP_FLAG_EXTREME_PARSING; // don't enable this as it's super slow (30x slower) and without significant improvement
   comp_params.m_compress_flags|=LZHAM_COMP_FLAG_DETERMINISTIC_PARSING; // make sure that compressed data is always the same
   comp_params.m_max_helper_threads=(multi_threaded ? -1 : 0);

   if(lzham_compress_state_ptr cs=lzham_lib_compress_init(&comp_params))
   {
      Byte in[BUF_SIZE], out[BUF_SIZE], *next_in; Int avail_in=0;
      for(;;)
      {
         if(!avail_in)
         {
            avail_in=src.getReturnSize(in, SIZE(in)); // always proceed even if 'avail_in'==0 in case of empty files or if we've encountered end of file
             next_in=in;
            if(callback)callback->data(in, avail_in);
         }
         lzham_compress_status_t status;
         do{
            size_t in_size=avail_in, out_size=SIZE(out);
            status=lzham_lib_compress2(cs, next_in, &in_size, out, &out_size, src.end() ? LZHAM_FINISH : LZHAM_NO_FLUSH);
            if(status>=LZHAM_COMP_STATUS_FIRST_FAILURE_CODE)goto error;
            if(!dest.put(out, out_size))goto error;
             next_in+=in_size;
            avail_in-=in_size;
         }while(status==LZHAM_COMP_STATUS_HAS_MORE_OUTPUT);
         if(!avail_in && status!=LZHAM_COMP_STATUS_NOT_FINISHED && src.end()){if(status==LZHAM_COMP_STATUS_SUCCESS)ok=true; break;}
      }
   error:
      lzham_lib_compress_deinit(cs);
   }
   return ok;
}
NOINLINE static Bool LZHAMDecompress(File &src, File &dest, Long compressed_size, Long decompressed_size, DataCallback *callback)
{
   using namespace lzham;
   Bool ok=false,
        direct=MemDecompress(dest, decompressed_size);

   lzham_decompress_params comp_params; Zero(comp_params);
   comp_params.m_struct_size   =SIZE(comp_params);
   comp_params.m_dict_size_log2=LZHAMDictSizeLog2(decompressed_size);
   if(direct)comp_params.m_decompress_flags=LZHAM_DECOMP_FLAG_OUTPUT_UNBUFFERED;

   if(lzham_decompress_state_ptr cs=lzham_lib_decompress_init(&comp_params))
   {
      Long start=dest.pos();
     ULong temp_size, temp_offset; src.limit(temp_size, temp_offset, compressed_size);
      Byte in[BUF_SIZE], out[BUF_SIZE], *next_in; Int avail_in=0;
      for(;;)
      {
         if(!avail_in)
         {
            avail_in=src.getReturnSize(in, SIZE(in)); if(avail_in<=0)break;
             next_in=in;
         }
         lzham_decompress_status_t status;
         do{
            size_t in_size=avail_in, out_size;
            Ptr   out_buf;
            if(direct)
            {
               out_buf =dest.memFast();
               out_size=dest.left   ();
            }else
            {
               out_buf =     out ;
               out_size=SIZE(out);
            }
            status=lzham_lib_decompress(cs, next_in, &in_size, (Byte*)out_buf, &out_size, src.end());
            if(status>=LZHAM_DECOMP_STATUS_FIRST_FAILURE_CODE)goto error;
            if(callback)callback->data(out_buf, out_size);
            if(direct ? !MemWrote(dest, out_size) : !dest.put(out, out_size))goto error;
             next_in+=in_size;
            avail_in-=in_size;
         }while(status==LZHAM_DECOMP_STATUS_HAS_MORE_OUTPUT);
         if(!avail_in && status!=LZHAM_DECOMP_STATUS_NOT_FINISHED && src.end()){if(status==LZHAM_DECOMP_STATUS_SUCCESS && dest.pos()-start==decompressed_size)ok=true; break;}
      }
   error:
      src.unlimit(temp_size, temp_offset);
      lzham_lib_decompress_deinit(cs);
   }
   return ok;
}
#endif
/******************************************************************************/
// CUSTOM
/******************************************************************************/
Bool CompressRaw(File &src, File &dest, COMPRESS_TYPE type, Int compression_level, Bool multi_threaded, DataCallback *callback)
{
   switch(type)
   {
      case COMPRESS_NONE  : return callback ? src.copy(dest, *callback) : src.copy(dest);
   #if SUPPORT_ZLIB
      case COMPRESS_ZLIB  : return   ZLIBCompress(src, dest, compression_level                , callback);
   #endif
   #if SUPPORT_LZMA
      case COMPRESS_LZMA  : return   LZMACompress(src, dest, compression_level, multi_threaded, callback);
   #endif
   #if SUPPORT_SNAPPY
      case COMPRESS_SNAPPY: return SNAPPYCompress(src, dest                                   , callback);
   #endif
   #if SUPPORT_RLE
      case COMPRESS_RLE   : return    RLECompress(src, dest                                   , callback);
   #endif
   #if SUPPORT_LZ4
      case COMPRESS_LZ4   : return    LZ4Compress(src, dest, compression_level                , callback);
   #endif
   #if SUPPORT_LIZARD
      case COMPRESS_LIZARD: return LIZARDCompress(src, dest, compression_level                , callback);
   #endif
   #if SUPPORT_LZHAM
      case COMPRESS_LZHAM : return  LZHAMCompress(src, dest, compression_level, multi_threaded, callback);
   #endif
   #if SUPPORT_BROTLI
      case COMPRESS_BROTLI: return BrotliCompress(src, dest, compression_level                , callback);
   #endif
   #if SUPPORT_ZSTD
      case COMPRESS_ZSTD  : return   ZSTDCompress(src, dest, compression_level                , callback);
   #endif
      default             : return false;
   }
}
Bool DecompressRaw(File &src, File &dest, COMPRESS_TYPE type, ULong compressed_size, ULong decompressed_size, Bool memory, DataCallback *callback)
{
   if(memory)dest.writeMemFixed(decompressed_size);
   switch(type)
   {
      case COMPRESS_NONE  : return callback ? src.copy(dest, *callback, decompressed_size) : src.copy(dest, decompressed_size);
   #if SUPPORT_ZLIB
      case COMPRESS_ZLIB  : return   ZLIBDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_LZMA
      case COMPRESS_LZMA  : return   LZMADecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_SNAPPY
      case COMPRESS_SNAPPY: return SNAPPYDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_RLE
      case COMPRESS_RLE   : return    RLEDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_LZ4
      case COMPRESS_LZ4   : return    LZ4Decompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_LIZARD
      case COMPRESS_LIZARD: return LIZARDDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_LZHAM
      case COMPRESS_LZHAM : return  LZHAMDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_BROTLI
      case COMPRESS_BROTLI: return BrotliDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
   #if SUPPORT_ZSTD
      case COMPRESS_ZSTD  : return   ZSTDDecompress(src, dest, compressed_size, decompressed_size, callback);
   #endif
      default             : return false;
   }
}
/******************************************************************************/
Bool Compress(File &src, File &dest, COMPRESS_TYPE type, Int compression_level, Bool multi_threaded, DataCallback *callback)
{
   Long uncompressed_size=src.left(), dest_start=dest.pos();
   if(  uncompressed_size<MIN_COMPRESSABLE_SIZE+1)type=COMPRESS_NONE; // add +1 because compressed types require storing 'compressed_size' which uses memory as well

start:
   // save header
   dest.putByte  (type+1           ); //  compression type +1, because 0 was old version=0
   dest.cmpULongV(uncompressed_size); // decompressed size
   Int   compressed_size_bytes;
   ULong compressed_size, compressed_size_pos; if(type) // make room for compressed size to be written later, because we don't know it yet
   {
      compressed_size_bytes=ByteHi(Unsigned(uncompressed_size)); // estimate how many bytes we need
      compressed_size_pos  =dest.pos();
      dest.put(null, compressed_size_bytes); // write zeros
   }

   // compress
   if(!CompressRaw(src, dest, type, compression_level, multi_threaded, callback))return false;

   if(type) // write compressed size
   {
      Long cur_pos=dest.pos();
      compressed_size=cur_pos-compressed_size_pos-compressed_size_bytes;
      if(ByteHi(compressed_size)>compressed_size_bytes) // if we need more than what was reserved, then fallback into COMPRESS_NONE
      {
         if(!src .skip(-uncompressed_size))return false; // go back to what was read
         if(!dest.pos ( dest_start       ))return false; // go back to what was written
         if(!dest.trim(                  ))return false; // remove everything that was written
         type=COMPRESS_NONE;
      #if 1
         callback=null; // we can just stop calling the callback, because most likely it's used for calculating the hash, so if we've already calculated it, no reason to calculate again
      #else
         if(callback)callback->reset(); // reset callback because we will write data all over again
      #endif
         goto start;
      }
      if(!dest.pos( compressed_size_pos                   ))return false;
      if(!dest.put(&compressed_size, compressed_size_bytes))return false;
      if(!dest.pos( cur_pos                               ))return false;
   }
   return dest.ok();
}
/******************************************************************************/
Bool Decompress(File &src, File &dest, Bool memory, DataCallback *callback)
{
   switch(UInt type=src.decUIntV())
   {
      default:
      {
         type--;
         ULong    compressed_size, decompressed_size=src.decULongV();
         if(type){compressed_size=0; src.getFast(&compressed_size, ByteHi(decompressed_size));}
         else     compressed_size=decompressed_size;

         if(src.ok())return DecompressRaw(src, dest, COMPRESS_TYPE(type), compressed_size, decompressed_size, memory, callback);
      }break;

      case 0:
      {
         COMPRESS_TYPE     type=COMPRESS_TYPE(src.getByte ());
         UInt decompressed_size=              src.decUIntV() ;
         UInt   compressed_size=              src.getUInt () ;

         if(src.ok())
         {
            if(memory)dest.writeMemFixed(decompressed_size);
            switch(type)
            {
               case COMPRESS_NONE  : return callback ? src.copy(dest, *callback, decompressed_size) : src.copy(dest, decompressed_size);
            #if SUPPORT_ZLIB
               case COMPRESS_ZLIB  : return   ZLIBDecompress   (src, dest, compressed_size, decompressed_size, callback);
            #endif
            #if SUPPORT_LZMA
               case COMPRESS_LZMA  : return   LZMADecompress   (src, dest, compressed_size, decompressed_size, callback);
            #endif
            #if SUPPORT_SNAPPY
               case COMPRESS_SNAPPY: return SNAPPYDecompressMem(src, dest, compressed_size, decompressed_size, callback);
            #endif
            #if SUPPORT_RLE
               case COMPRESS_RLE   : return    RLEDecompress   (src, dest, compressed_size, decompressed_size, callback);
            #endif
            #if SUPPORT_LZ4
               case COMPRESS_LZ4   : return    LZ4DecompressMem(src, dest, compressed_size, decompressed_size, callback);
            #endif
            #if SUPPORT_LZHAM
               case COMPRESS_LZHAM : return  LZHAMDecompress   (src, dest, compressed_size, decompressed_size, callback);
            #endif
            }
         }
      }break;
   }
   return false;
}
Bool DecompressHeader(File &src, COMPRESS_TYPE &type, ULong &compressed_size, ULong &decompressed_size)
{
   switch(UInt ver=src.decUIntV())
   {
      default:
      {
         type=COMPRESS_TYPE(ver-1);
         decompressed_size=src.decULongV();
         if(type){compressed_size=0; src.getFast(&compressed_size, ByteHi(decompressed_size));}
         else     compressed_size=decompressed_size;

         if(src.ok())return true;
      }break;

      case 0:
      {
                      type=COMPRESS_TYPE(src.getByte ());
         decompressed_size=              src.decUIntV() ;
           compressed_size=              src.getUInt () ;

         if(src.ok())return true;
      }break;
   }

   return false;
}
/******************************************************************************/
Bool Compressable(C Str &ext)
{
   if(ext=="jpg" || ext=="jpeg" || ext=="png"  || ext=="webp" || ext=="bpg" || ext=="flif" // image
   || ext=="mp3" || ext=="wma"  || ext=="ogg"  || ext=="flac" || ext=="m4a" || ext=="opus" || ext=="weba" // sound
   || ext=="avi" || ext=="mpg"  || ext=="mpeg" || ext=="mp4"  || ext=="m4v" || ext=="mkv" || ext=="wmv" || ext=="rmvb" || ext=="divx" || ext=="ogm" || ext=="ogv" || ext=="theora" || ext=="webm" || ext=="vob" || ext=="flv" // video
   || ext=="zip" || ext=="rar"  || ext=="7z"   || ext=="gz"   || ext=="bz2" || ext=="tgz" || ext=="tbz" || ext=="xz" // archives ("tar" itself is not compressed)
   )return false;
    return true;
}
CChar8* CompressionName(COMPRESS_TYPE type)
{
   switch(type)
   {
      case COMPRESS_NONE  : return "None";
      case COMPRESS_ZLIB  : return "Zlib";
      case COMPRESS_LZMA  : return "LZMA";
      case COMPRESS_SNAPPY: return "Snappy";
      case COMPRESS_RLE   : return "RLE";
      case COMPRESS_LZ4   : return "LZ4";
      case COMPRESS_LIZARD: return "Lizard";
      case COMPRESS_LZHAM : return "LZHAM";
      case COMPRESS_ZSTD  : return "Zstd";
      case COMPRESS_BROTLI: return "Brotli";
      default             : return null;
   }
}
VecI2 CompressionLevels(COMPRESS_TYPE type)
{
   switch(type)
   {
      default             : return 0; // single compression level
      case COMPRESS_ZLIB  : return VecI2(1,  9); // 0=no compression
      case COMPRESS_LZMA  : return VecI2(0,  9);
   #if SUPPORT_LZHAM
      case COMPRESS_LZHAM : return VecI2(LZHAM_COMP_LEVEL_FASTEST, LZHAM_TOTAL_COMP_LEVELS-1);
   #endif
   #if SUPPORT_LZ4
      case COMPRESS_LZ4   : return VecI2(0, LZ4HC_CLEVEL_MAX); // 0=LZ4, 1..LZ4HC_CLEVEL_MAX=LZ4_HC
   #endif
   #if SUPPORT_LIZARD
      case COMPRESS_LIZARD: return VecI2(LIZARD_MIN_CLEVEL, LIZARD_MAX_CLEVEL);
   #endif
      case COMPRESS_BROTLI: return VecI2(0, 11); // taken from "enc/encode.h" header
   #if SUPPORT_ZSTD
      case COMPRESS_ZSTD  : return VecI2(1, ZSTD_maxCLevel()); // 0=never used
   #endif
   }
}
UInt CompressionMemUsage(COMPRESS_TYPE type, Int compression_level, Long uncompressed_size)
{
   switch(type)
   {
      default: return 0;

      case COMPRESS_ZLIB:
      {
         const UInt windowBits=15, memLevel=8;
         return (1<<(windowBits+2)) + (1<<(memLevel+9)); // taken from "ThirdPartyLibs\Zlib\zconf.h" and http://www.zlib.net/zlib_tech.html
      }

   #if SUPPORT_LZMA
      case COMPRESS_LZMA:
      {
         const UInt dict_size=LZMADictSize(compression_level, uncompressed_size),
                   state_size=16*1024; // 16 KB default
         return RoundU(dict_size*11.5f) + 6*1024*1024 + state_size; // (dict_size * 11.5 + 6 MB) + state_size, taken from "ThirdPartyLibs\LZMA\lzma\lzma.txt"
      }
   #endif

      case COMPRESS_SNAPPY: return 32768+76490; // this was observed by monitoring calls to 'Alloc', which resulted in 1x32768 and 1x76490
    //case COMPRESS_RLE   : return 0; RLE doesn't use any extra memory

   #if SUPPORT_LZ4
    //case COMPRESS_LZ4: return 1<<LZ4_MEMORY_USAGE; 'LZ4_streamHC_t' and 'LZ4_stream_t' are allocated on the stack
   #endif

   #if SUPPORT_LIZARD
      case COMPRESS_LIZARD: return Lizard_sizeofState(Mid(compression_level, LIZARD_MIN_CLEVEL, LIZARD_MAX_CLEVEL));
   #endif

   #if SUPPORT_ZSTD
      case COMPRESS_ZSTD:
      {
         if(uncompressed_size<0)uncompressed_size=LONG_MAX; // if size is unknown then use max possible
         U64 srcSize=(uncompressed_size ? uncompressed_size : 1); // don't use zero, because ZSTD treats it as unknown
         ZSTD_compressionParameters params=ZSTD_getCParams(Mid(compression_level, 1, ZSTD_maxCLevel()), srcSize, 0);
      #if !ZSTD_WINDOWLOG_SAVE
         params.windowLog=ZSTDDictSizeLog2(uncompressed_size);
      #endif
       C Int window_size=1<<params.windowLog;
         return MemtMemUsage(         Min(window_size+ZSTD_BLOCKSIZE_MAX, uncompressed_size) ) // Memt s;
               +MemtMemUsage(ZSTDSize(Min(            ZSTD_BLOCKSIZE_MAX, uncompressed_size))) // Memt d;
               +ZSTD_estimateCCtxSize_usingCParams(params);                                        // ZSTD_CCtx
      }
   #endif

   #if SUPPORT_BROTLI
      case COMPRESS_BROTLI: return ;
   #endif

   #if SUPPORT_LZHAM
      case COMPRESS_LZHAM:
      {
       /*Based on https://github.com/richgel999/lzham_codec
         It's approximately (max_probes=128 at level -m4): comp_mem = min(512 * 1024, dict_size / 8) * max_probes * 6 + dict_size * 9 + 22020096
         Compression mem usage examples from Windows lzhamtest_x64 (note the equation is pretty off for small dictionary sizes):
             32KB: 11MB
            128KB: 21MB
            512KB: 63MB
              1MB: 118MB
              8MB: 478MB
             64MB: 982MB
            128MB: 1558MB
            256MB: 2710MB
            512MB: 5014MB */
         const UInt dict_size=LZHAMDictSize(uncompressed_size), max_probes=128;
         return Min(512*1024, dict_size/8)*max_probes*6 + dict_size*9 + 22020096;
      }
   #endif
   }
}
UInt DecompressionMemUsage(COMPRESS_TYPE type, Int compression_level, Long uncompressed_size)
{
   switch(type)
   {
      default: return 0;

      case COMPRESS_ZLIB:
      {
         const UInt windowBits=15;
         return (1<<windowBits) + 1440*2*SIZE(Int); // taken from "ThirdPartyLibs\Zlib\zconf.h" and http://www.zlib.net/zlib_tech.html
      }

   #if SUPPORT_LZMA
      case COMPRESS_LZMA:
      {
         const UInt dict_size=LZMADictSize(compression_level, uncompressed_size),
                   state_size=16*1024; // 16 KB default
         return dict_size + state_size; // taken from "ThirdPartyLibs\LZMA\lzma\lzma.txt"
      }
   #endif

    //case COMPRESS_SNAPPY: return 0; this was observed by monitoring calls to 'Alloc', which resulted in no calls
    //case COMPRESS_RLE   : return 0; RLE doesn't use any extra memory

   #if SUPPORT_LZ4
    //case COMPRESS_LZ4: return 1<<LZ4_MEMORY_USAGE; 'LZ4_streamDecode_t' is allocated on the stack
   #endif

   #if SUPPORT_LIZARD
    //case COMPRESS_LIZARD: return 0; 'Lizard_streamDecode_t' is allocated on the stack
   #endif

   #if SUPPORT_ZSTD
      case COMPRESS_ZSTD:
      {
         if(uncompressed_size<0)uncompressed_size=LONG_MAX; // if size is unknown then use max possible
      #if ZSTD_WINDOWLOG_SAVE
         U64 srcSize=(uncompressed_size ? uncompressed_size : 1); // don't use zero, because ZSTD treats it as unknown
         ZSTD_compressionParameters params=ZSTD_getCParams(Mid(compression_level, 1, ZSTD_maxCLevel()), srcSize, 0);
       C Int window_size=1<<params.windowLog;
      #else
       C Int window_size=1<<ZSTDDictSizeLog2(uncompressed_size);
      #endif
         return MemtMemUsage(ZSTDSize(Min(ZSTD_BLOCKSIZE_MAX, uncompressed_size))) // Memt s;
               +MemtMemUsage( window_size+ZSTD_BLOCKSIZE_MAX                     ) // Memt d;
               +ZSTD_estimateDCtxSize();                                           // ZSTD_DCtx
      }
   #endif

   #if SUPPORT_BROTLI
      case COMPRESS_BROTLI: return ;
   #endif

   #if SUPPORT_LZHAM
      case COMPRESS_LZHAM:
      {
       /*Based on https://github.com/richgel999/lzham_codec
           Buffered mode: decomp_mem = dict_size + ~34KB for work tables
         Unbuffered mode: decomp_mem = ~34KB */
         const UInt dict_size=LZHAMDictSize(uncompressed_size);
         return dict_size + 34*1024;
      }
   #endif
   }
}
/******************************************************************************/
}
/******************************************************************************/
