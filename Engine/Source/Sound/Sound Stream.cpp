/******************************************************************************/
#include "stdafx.h"

#define CC4_SND CC4('S', 'N', 'D', '\0')

#include "../../../ThirdPartyLibs/begin.h"

#if SUPPORT_FLAC
   #define FLAC__NO_DLL
   #include "../../../ThirdPartyLibs/Flac/lib/include/FLAC/all.h"
#endif

#if SUPPORT_VORBIS || SUPPORT_OPUS
   #include "../../../ThirdPartyLibs/VP/libvpx/third_party/libwebm/mkvparser/mkvparser.h"
   #define NANOS 1000000000
#endif

#if SUPPORT_VORBIS
   #include "../../../ThirdPartyLibs/Vorbis/include/vorbis/vorbisfile.h"
#endif

#if SUPPORT_OPUS || SUPPORT_OPUS_ENC
namespace
{
   #include "../../../ThirdPartyLibs/Opus/lib/include/opus.h"
   #include "../../../ThirdPartyLibs/Opus/file/src/internal.h"
   #include "../../../ThirdPartyLibs/Opus/file/include/opusfile.h"
   #include "../../../ThirdPartyLibs/Ogg/include/ogg/ogg.h"
   typedef OpusEncoder *OE;
   typedef OpusDecoder *OD;
}

#define OPUS_MAX_FRAME_SAMPLES  2880
#define OPUS_FRAME_HISTORY      32 // tests have shown we need to decode at least 32 packets earlier to get same results
#define OPUS_FRAME_HISTORY_FAST 4 // 4 frames was enough to produce satisfactory results

static Int OpusMaxFrameSize(Int channels)
{
   return (OPUS_MAX_FRAME_SAMPLES*SIZE(I16))*channels;
}
#endif

#if SUPPORT_MP3
   #define MINIMP3_ONLY_MP3
   #if !((WINDOWS && ARM) || WEB)
      #define MINIMP3_ONLY_SIMD
   #endif
 //#define MINIMP3_NONSTANDARD_BUT_LOGICAL
 //#define MINIMP3_FLOAT_OUTPUT
   #define MINIMP3_IMPLEMENTATION
   #define MINIMP3_NO_STDIO
   #define MAX_FRAME_SYNC_MATCHES 8
   #include "../../../ThirdPartyLibs/MiniMP3/minimp3.h"
   #define MP3_FRAME_SAMPLES  1152 // MPEG-1 Layer 3
   #define MP3_FRAME_SAMPLES2 576  // MPEG-2 Layer 3
   #define MP3_SAMPLE_DELAY   529  // http://mp3decoders.mp3-tech.org/decoders_lame.html or 528 http://lame.sourceforge.net/tech-FAQ.txt
#endif

#if SUPPORT_AAC
   #define INT64 AAC_INT64 // on Linux there are name conflicts
   #include "../../../ThirdPartyLibs/FDK-AAC/lib/libAACdec/include/aacdecoder_lib.h"
   #undef INT64
#endif

#include "../../../ThirdPartyLibs/end.h"

#pragma warning(disable:4267) // 64bit - conversion from 'size_t' to 'Int', possible loss of data
namespace EE{
/******************************************************************************/
struct FileData
{
   Mems<Byte> data; // file data

   Bool load(C Str &name) // load, false on fail
   {
      File f; if(f.readTry(name))
      {
         data.setNum(f.size());
         if(f.getFast(data.data(), data.elms()))return true;
      }
      data.del(); return false;
   }
};
/******************************************************************************/
DECLARE_CACHE(FileData, FileDatas, FileDataPtr);
 DEFINE_CACHE(FileData, FileDatas, FileDataPtr, "Sound");
/******************************************************************************/
CChar8* CodecName(SOUND_CODEC codec)
{
   switch(codec)
   {
      default           : return null;
      case SOUND_WAV    : return "WAV";
      case SOUND_FLAC   : return "Flac";
      case SOUND_MP3    : return "MP3";
      case SOUND_AAC    : return "AAC";
      case SOUND_DYNAMIC: return "Dynamic";

      case SOUND_SND_VORBIS :
      case SOUND_OGG_VORBIS :
      case SOUND_WEBM_VORBIS: return "Vorbis";

      case SOUND_SND_OPUS :
      case SOUND_OGG_OPUS :
      case SOUND_WEBM_OPUS: return "Opus";
   }
}
/******************************************************************************/
static inline Short SampleFltToShort(Flt x) {return Mid(Trunc(x*32768.0f), -32768, 32767);} // this code matches "Vorbis File" library
/******************************************************************************/
// WAV
/******************************************************************************/
Bool SaveWavHeader(File &f, Int bits, Int channels, Int frequency, UInt size)
{
   Int bytes=bits/8, block=channels*bytes;
   f.putUInt  (CC4('R','I','F','F'));
   f.putUInt  (size+36);
   f.putUInt  (CC4('W','A','V','E'));
   f.putUInt  (CC4('f','m','t',' '));
   f.putInt   (0x10);
   f.putUShort(0x01);
   f.putUShort(channels);
   f.putInt   (frequency);
   f.putInt   (block*frequency);
   f.putUShort(block);
   f.putUShort(bits);
   f.putUInt  (CC4('d','a','t','a'));
   f.putUInt  (size);
   return f.ok();
}
static SOUND_CODEC LoadWavHeader(File &f, SoundStream::Params &params)
{
   SOUND_CODEC codec=SOUND_NONE;
   if(f.getUInt()==CC4('R','I','F','F'))
   {
      f.getInt();
      if(f.getUInt()==CC4('W','A','V','E'))
      for(; !f.end(); )
      {
         UInt data=f.getUInt();
         UInt size=f.getUInt();
         if(data==CC4('f','m','t',' '))
         {
            switch(f.getUShort()) // type
            {
               case  1: codec=SOUND_WAV; break; // PCM
               case 85: codec=SOUND_MP3; break;
               default: goto error; // unsupported
            }
            params.channels =f.getUShort(); if(params.channels!=1 && params.channels!=2)goto error;
            params.frequency=f.getInt   (); f.skip(6);
            U16 bits        =f.getUShort();
            switch(codec)
            {
               case SOUND_WAV: if(bits!=8 && bits!=16 && bits!=24 && bits!=32)goto error; break;
               case SOUND_MP3: break; // allow all bits because actual data will be loaded in 'LoadMP3Header'
            }
            params.bytes=bits/8;
            size-=16; // 16 bytes were read
         }else 
         if(data==CC4('d','a','t','a'))
         {
            params.size    =size;
            params.block   =params.channels*params.bytes;
            params.bit_rate=params.channels*params.bits()*params.frequency;
            if(codec)return codec; goto error;
         }
         f.skip(size);
      }
   }
error:
   params.zero(); return SOUND_NONE;
}
/******************************************************************************/
// SND
/******************************************************************************/
Bool SaveSndHeader(File &f, SND_CODEC codec, Int channels, Int frequency, Long samples)
{
   // !! don't modify 'samples' if negative, because it may mean unknown number of samples, 'SndRawEncoder' can use that !!
   f.putMulti(UInt(CC4_SND), Byte(0), codec, Byte(channels), Int(frequency)); // CC4, version, codec, channels, frequency
   f.cmpULongV(samples); // !! samples must be saved as last, because 'SndRawEncoder' will offset by 'MaxCmpULongVSize' !!
   return f.ok();
}
Bool LoadSndHeader(File &f, SoundStream::Params &params, SND_CODEC &codec)
{
   if(f.getUInt()==CC4_SND)switch(f.decUIntV())
   {
      case 0:
      {
         f>>codec;
         params.channels =f.getByte();
         params.frequency=f.getInt ();
         Long samples=f.decULongV();
         params.bytes    =2; // all codecs currently support only 16-bit
         params.block    =params.bytes*params.channels;
         params.size     =params.block*samples;
         params.bit_rate =(samples ? f.size()*8*params.frequency/samples : 0);
      }return f.ok();
   }
   codec=SND_RAW_16;
   params.zero();
   return false;
}
/******************************************************************************/
// WEBM
/******************************************************************************/
#if SUPPORT_VORBIS || SUPPORT_OPUS
struct MkvReader : mkvparser::IMkvReader
{
   File *file;

   MkvReader()=delete;
   MkvReader(File &file) : file(&file) {}

   virtual int Read(long long pos, long len, unsigned char *buf)override // 0=OK, -1=error
   {
      if(len==0)return 0;
      if(!file->pos(pos))return -1;
      return file->getFast(buf, len) ? 0 : -1;
   }
   virtual int Length(long long* total, long long *available)override // 0=OK, -1=error
   {
      if(total    )*total    =file->size();
      if(available)*available=file->size();
      return 0;
   }
};

struct WebmFile : MkvReader
{
   #define USE_TIME_RAW 0
   mkvparser::Segment    *segment;
 C mkvparser::Cluster    *cluster;
 C mkvparser::BlockEntry *block_entry;
 C mkvparser::Block      *block;
   Byte                   extra_frames;
   UShort                 delay;
   Int                    buffer_pos, frame_index, track_number, buf_block, frequency;
   Memc<Byte>             buffer;

#if USE_TIME_RAW
   instead of time<->raw it would be better if this would operate directly on frame_size, frame_pos, decoded_frame_size..
   struct TimeRaw
   {
      Long time, raw;

      void set(Long time, Long raw) {T.time=time; T.raw=raw;}

      static Int Compare(C TimeRaw &tr, C Long &raw) {return ::Compare(tr.raw, raw);}
   };
   Mems<TimeRaw> time_raw;

   virtual Bool decodeFrameSize(Ptr temp, Int temp_size, Int &size)=NULL;

   Bool setTimeRaw()
   {
      Memc<TimeRaw> tr;
      Long size=0;
    C mkvparser::BlockEntry *block_entry;
      if(C mkvparser::Tracks *tracks=segment->GetTracks())
      if(C mkvparser::Track  *track =tracks->GetTrackByNumber(track_number))
      if(!track->GetFirst(block_entry))
      if(C mkvparser::Cluster *cluster=block_entry->GetCluster())
      if(C mkvparser::Block   *block=block_entry->GetBlock())
      {
       C mkvparser::Cluster *last=null;
      again:
         if(last!=cluster) // need to record only at the start of the cluster (and not on every block inside it, because Vorbis will fail to loop, it must be decoded from the start of the cluster)
         {
            last=cluster;
            tr.New().set(block->GetTime(cluster), size);
         }
         FREP(block->GetFrameCount())
         {
          C mkvparser::Block::Frame &frame=block->GetFrame(i);
            Byte temp[65536];
            Int  frame_size;
            if( frame.len>SIZE(temp)
            || !file->pos(frame.pos)
            || !file->getFast(temp, frame.len)
            || !decodeFrameSize(temp, frame.len, frame_size))goto error;
            size+=frame_size;
         }
         if(advance(cluster, block_entry, block))goto again;
      }
    //Flt time=size/Dbl(frequency*buf_block); // this may be bigger than SoundStream.length() because here the last frame size is reported in full
      time_raw=tr;
      return true;
   error:
      clearPtr();
      return false;
   }
#endif

   void clearBuffer() {buffer_pos=0; buffer.clear();}
   void clearPtr()
   {
      cluster    =null;
      block_entry=null;
      block      =null;
   }
   inline Bool advance(C mkvparser::Cluster* &cluster, C mkvparser::BlockEntry* &block_entry, C mkvparser::Block* &block)
   {
   advance:
      cluster->GetNext(block_entry, block_entry);
      if(!block_entry || block_entry->EOS())
      {
         cluster=segment->GetNext(cluster);
         if(!cluster || cluster->EOS())return false;
         cluster->GetFirst(block_entry);
         if(!block_entry)return false;
      }
      block=block_entry->GetBlock();
      if(!block)return false;
      if( block->GetTrackNumber()!=track_number)goto advance;
      return true;
   }
   Bool advance()
   {
      if(advance(cluster, block_entry, block))return true;
      clearPtr(); return false;
   }
   virtual Bool pos(Long pos)
   {
      if(pos<0)return false;
      pos+=delay;
      clearBuffer();
      if(segment)
      {
         Long time;
      #if USE_TIME_RAW
       C TimeRaw *tr;
         if(time_raw.elms()) // use time raw map
         {
            Int index; if(!time_raw.binarySearch(pos, index, TimeRaw::Compare))index--; // if didn't found an exact match, then we need to use the previous one
            tr=&time_raw[index];
            time=tr->time;
         }else
      #endif
            time=pos*NANOS/(buf_block*frequency);

         if(C mkvparser::Tracks *tracks=segment->GetTracks())
         if(C mkvparser::Track  *track =tracks->GetTrackByNumber(track_number))
      #if 1 // this is more precise
         if(cluster=segment->FindCluster(time))
         if(block_entry=cluster->GetEntry(track, time))
         if(block=block_entry->GetBlock())
         if(block->GetTrackNumber()==track_number || advance())
      #else
         if(!track->Seek(time, block_entry))
         if(cluster=block_entry->GetCluster())
         if(block=block_entry->GetBlock())
      #endif
         {
            Long block_time=block->GetTime(cluster);
            if(block_time>time)goto error;

            frame_index=0;

            REP(extra_frames)
            {
               block_time--; // we need previous frame, so just look for a frame using time smaller by 1 from the next frame
               if(cluster=segment->FindCluster(block_time))
               if(block_entry=cluster->GetEntry(track, block_time))
               if(block=block_entry->GetBlock())
               if(block->GetTrackNumber()==track_number || advance())
                  goto found_prev_block;
               goto error;
            found_prev_block:
               block_time=block->GetTime(cluster);
            }

         #if USE_TIME_RAW
            if(time_raw.elms())buffer_pos=pos-tr->raw;else this is wrong because we may have used previous block
         #endif
                               buffer_pos=pos-(block_time*frequency/NANOS)*buf_block; // mul by block last to make sure we've got alignment
            return true;
         }
      }
   error:
      clearPtr();
      return false;
   }
   virtual Bool decode(Ptr &data, Int &size, Ptr temp, Int temp_size, Int &read)=NULL;
   Int set(Ptr data, Int size)
   {
      if(size>0)
      {
         Int read=0;
      again:
         if(!buffer.elms() && block)
         {
            if(frame_index>=block->GetFrameCount())
            {
               if(!advance())goto error;
               frame_index=0;
            }
         #if 0 // log
            if(frame_index==0)
            {
               Long time_ns=block->GetTime(cluster);
               Flt time=time_ns/Dbl(NANOS);
               Flt time2=ppp/Dbl(frequency*buf_block);
               LogN(S+time+' '+time2+' '+(time2-time));
            }
         #endif
          C mkvparser::Block::Frame &frame=block->GetFrame(frame_index++);
            Byte temp[65536];
            if( frame.len>SIZE(temp)
            || !file->pos(frame.pos)
            || !file->getFast(temp, frame.len)
            || !decode(data, size, temp, frame.len, read))goto error;
            if(!read && !buffer.elms())goto again; // if 'decode' succeeded, but nothing was 'read' or placed to the 'buffer' then try again (this can happen for Vorbis)
         }
         Int buffer_left=Min(buffer.elms()-buffer_pos, size);
         if( buffer_left>0) // if we have any left in the buffer
         {
            CopyFast(data, buffer.data()+buffer_pos, buffer_left);
            buffer_pos+=buffer_left; if(buffer_pos>=buffer.elms())clearBuffer(); // if we've read the entire buffer then clear it
            data       =(Byte*)data+buffer_left;
            read      +=buffer_left;
            size      -=buffer_left;
         }else
         if(buffer.elms() && buffer_pos>=buffer.elms()) // this can happen if we had some data in the buffer, but we had to skip everything
         {
            buffer_pos-=buffer.elms(); // we've skipped this
            buffer.clear();
            goto again; // try reading again instead of returning 0 which implies an error
         }
         return read;
      }
   error:
      return 0;
   }
   virtual ~WebmFile() {Delete(segment);}
            WebmFile()=delete;
            WebmFile(C SoundStream::Params &params, File &file, Int track_number, mkvparser::Segment* &segment, Int extra_frames) : MkvReader(file)
   {
      T.extra_frames=extra_frames;
      T.delay=0;
      T.buffer_pos=0;
      T.frame_index=0;
      T.buf_block=params.block;
      T.frequency=params.frequency;
      T.track_number=track_number;
      T.cluster=null;
      T.block_entry=null;
      T.block=null;
      if(T.segment=segment)
      {
         ConstCast(T.segment->m_pReader)=this;
         if(T.cluster=T.segment->GetFirst())
         {
            T.cluster->GetFirst(T.block_entry);
            if(T.block_entry)if(T.block=T.block_entry->GetBlock())if(T.block->GetTrackNumber()!=track_number)advance();
         }
         segment=null; // set this to null because we've taken ownership
      }
   }
};
#endif
/******************************************************************************/
// VORBIS
/******************************************************************************/
#if SUPPORT_VORBIS
static Int BitRate(C vorbis_info &vi)
{
   return (vi.bitrate_nominal>0) ? vi.bitrate_nominal : (vi.bitrate_lower>0 && vi.bitrate_upper>0) ? AvgI(UInt(vi.bitrate_lower), UInt(vi.bitrate_upper)) : 0;
}

static ov_callbacks VorbisCallbacks;

static size_t VorbisRead (void *data, size_t size, size_t nmemb, void *file) {return ((File*)file)->getReturnSize(data, nmemb*size);}
static long   VorbisTell (void *file                                       ) {return ((File*)file)->pos          (                );}
static Int    VorbisClose(void *file                                       ) {     /*((File*)file)->del          (                );*/ return 0;} // don't do anything in case it wants to close on load fail, but we still need the file to try other codecs, also we delete the file manually, so this is not needed
static Int    VorbisSeek (void *file, ogg_int64_t offset, Int whence       ) // should return -1 if unseekable (fail) and 0 otherwise
{
   File &f=*(File*)file;
   switch(whence)
   {
      case SEEK_CUR: offset+=f.pos (); break;
      case SEEK_END: offset+=f.size(); break;
   }
   return f.pos(offset)-1;
}

static OggVorbis_File* LoadOggVorbisHeader(File &f, SoundStream::Params &params)
{
   OggVorbis_File *vorbis=Alloc<OggVorbis_File>();
   if(OK(ov_open_callbacks(&f, vorbis, null, 0, VorbisCallbacks)))
   {
      vorbis_info *vi=ov_info(vorbis, -1);
      if(vi)
      {
         params.frequency=vi->rate;
         params.channels =vi->channels;
         params.bit_rate =BitRate(*vi);
         params.bytes    =2;
         params.block    =params.channels*params.bytes;
         params.size     =params.block   *ov_pcm_total(vorbis, -1);
         return vorbis;
      }
      ov_clear(vorbis);
   }
   params.zero(); Free(vorbis); return null;
}

STRUCT(WebmVorbisFile , WebmFile)
//{
   vorbis_info      vi;
   vorbis_dsp_state vd;
   vorbis_block     vb;

   static ULong xiph_lace_value(C Byte **np)
   {
    C Byte *p=*np;
      ULong lace=*p++, value=lace; while(lace==255){lace=*p++; value+=lace;}
      *np=p; return value;
   }
   static C Byte* GetPrivateDataPart(C Byte *private_data, size_t private_size, int part, size_t &part_size)
   {
      part_size=0;
    C Byte *result=null, *p=private_data;
      Int   count =*p++ + 1;
      if(count==3 && *p>=part)
      {
         ULong sizes[3], total=0;
         Int   i=0;
         while(--count)
         {
            sizes[i]=xiph_lace_value(&p);
            total  +=sizes[i];
            i++;
         }
         sizes[i]=private_size-total-(p-private_data);
         for(i=0; i<part; ++i)p+=sizes[i];
         result   =p;
         part_size=sizes[part];
      }
      return result;
   }

  ~WebmVorbisFile()
   {
      vorbis_block_clear(&vb);
      vorbis_dsp_clear  (&vd);
      vorbis_info_clear (&vi);
   }
   WebmVorbisFile(SoundStream::Params &params, File &file, Int track_number, mkvparser::Segment* &segment, C Byte *priv_data, Int priv_data_size) : WebmFile(params, file, track_number, segment, 1)
   {
      Zero(vi);
      Zero(vd);
      Zero(vb);
      if(priv_data && priv_data_size && priv_data[0]==2)
      {
         vorbis_comment vc; Zero(vc);

         vorbis_info_init   (&vi);
         vorbis_comment_init(&vc);

         Int ogg_packet_num=0, error=0;
         for(Int h=0; h<3 && !error; h++)
         {
               size_t  length=0;
            if(C Byte *data  =GetPrivateDataPart(priv_data, priv_data_size, h, length))
            {
               ogg_packet packet;
               packet.packet    =ConstCast(data);
               packet.bytes     =length;
               packet.b_o_s     =(h==0);
               packet.e_o_s     =false;
               packet.granulepos=0;
               packet.packetno  =ogg_packet_num++;

               error=vorbis_synthesis_headerin(&vi, &vc, &packet);
            }
         }
         if(!error
         && !vorbis_synthesis_init(&vd, &vi)
         && !vorbis_block_init    (&vd, &vb))
         {
            Int bit_rate=BitRate(vi); if(bit_rate>0)params.bit_rate=bit_rate;
         }
         vorbis_comment_clear(&vc);
      }
   }
   virtual Bool pos(Long pos)override
   {
   #if USE_TIME_RAW
    //if(!time_raw.elms())setTimeRaw();
   #endif
      if(super::pos(pos))
      {
         vorbis_synthesis_restart(&vd);
         return true;
      }
      return false;
   }
#if USE_TIME_RAW
   virtual Bool decodeFrameSize(Ptr temp, Int temp_size, Int &size)override
   {
      ogg_packet packet;
      packet.packet    =(Byte*)temp;
      packet.bytes     =temp_size;
      packet.b_o_s     =false;
      packet.e_o_s     =false;
      packet.granulepos=-1;
      packet.packetno  =-1;
      size=vorbis_packet_blocksize(&vi, &packet); if(size<0)return false;
      size*=buf_block/vi.channels; // this is OK because 'buf_block' will always be a multiple of channels
      return true;
   }
#endif
   virtual Bool decode(Ptr &data, Int &size, Ptr temp, Int temp_size, Int &read)override
   {
      ogg_packet packet;
      packet.packet    =(Byte*)temp;
      packet.bytes     =temp_size;
      packet.b_o_s     =false;
      packet.e_o_s     =false;
      packet.granulepos=-1;
      packet.packetno  =-1;

      if(!vorbis_synthesis        (&vb, &packet)
      && !vorbis_synthesis_blockin(&vd, &vb))
      {
         for(;;)
         {
            Flt **pcm=null;
            Int   decoded_samples=vorbis_synthesis_pcmout(&vd, &pcm); if(decoded_samples<=0)break;
            Int   pcm_samples=decoded_samples;
            if(buffer_pos) // we need to skip some samples
            {
               Int skip_samples=Min(buffer_pos/buf_block, pcm_samples);
               REP(vi.channels)pcm[i]+=skip_samples;
               buffer_pos -=skip_samples*buf_block;
               pcm_samples-=skip_samples;
            }
            if(size>0) // write directly to 'data'
            {
               Int  data_samples=size/buf_block,
                   write_samples=Min(data_samples, pcm_samples);
               FREPD(s, write_samples)
               FREPD(c, vi.channels)*((I16*&)data)++=SampleFltToShort(pcm[c][s]);
               Int r=write_samples*buf_block;
               size-=r;
               read+=r;
               pcm_samples-=write_samples;
               REP(vi.channels)pcm[i]+=write_samples;
            }
            if(pcm_samples) // write leftovers to buffer
            {
               Int start=buffer.addNum(pcm_samples*buf_block);
               I16 *buf =(I16*)&buffer[start];
               FREPD(s, pcm_samples)
               FREPD(c, vi.channels)*buf++=SampleFltToShort(pcm[c][s]);
            }
            if(vorbis_synthesis_read(&vd, decoded_samples))return false;
         }
         return true;
      }
      return false;
   }
};

// decoder could be written based on 'WebmVorbisFile' and 'SndVorbisEncoder'
struct SndVorbisFile
{
   File *file;

   Bool pos(Long pos)
   {
      return false;
   }
   Int set(Ptr data, Int size)
   {
      return 0;
   }
   Bool init(SoundStream &stream)
   {
      T.file=&stream._f;
      return false;
   }
};
#endif
/******************************************************************************/
// OPUS
/******************************************************************************/
#if SUPPORT_OPUS
static OpusFileCallbacks OpusCallbacks;

static int        OpusRead (void *file, unsigned char *data, int size) {return ((File*)file)->getReturnSize(data, size);}
static opus_int64 OpusTell (void *file                               ) {return ((File*)file)->pos          (          );}
static int        OpusClose(void *file                               ) {     /*((File*)file)->del          (          );*/ return 0;} // don't do anything in case it wants to close on load fail, but we still need the file to try other codecs, also we delete the file manually, so this is not needed
static int        OpusSeek (void *file, opus_int64 offset, int whence)
{
   File &f=*(File*)file;
   switch(whence)
   {
      case SEEK_CUR: offset+=f.pos (); break;
      case SEEK_END: offset+=f.size(); break;
   }
   return f.pos(offset)-1;
}

static OggOpusFile* LoadOggOpusHeader(File &f, SoundStream::Params &params)
{
   if(OggOpusFile *opus=op_open_callbacks(&f, &OpusCallbacks, null, 0, null))
   {
      ogg_int64_t samples=op_pcm_total(opus, -1);
      params.frequency=OPUS_FREQUENCY;
      params.bytes    =OPUS_BYTES;
      params.channels =Min(op_channel_count(opus, 0), 2); // support only mono/stereo, it's okay to limit here, because 'op_read_stereo' will handle down-mixing
      params.block    =params.channels*params.bytes;
      params.size     =params.block   *samples;
      params.bit_rate =(samples ? f.size()*8*params.frequency/samples : 0);
      return opus;
    //op_free(opus);
   }
   params.zero(); return null;
}

STRUCT(WebmOpusFile , WebmFile)
//{
   OpusDecoder od;

#if USE_TIME_RAW
   virtual Bool decodeFrameSize(Ptr temp, Int temp_size, Int &size)override
   {
      size=opus_packet_get_nb_samples((Byte*)temp, temp_size, od.frequency()); if(size<0)return false;
      size*=buf_block;
      return true;
   }
#endif
   virtual Bool pos(Long pos)override
   {
      if(super::pos(pos))
      {
         od.reset();
         return true;
      }
      return false;
   }
   virtual Bool decode(Ptr &data, Int &size, Ptr temp, Int temp_size, Int &read)override
   {
      if(buffer_pos==0 && size>=OpusMaxFrameSize(od.channels())) // decode directly into 'data', we can do this only if we're not skipping (buffer_pos==0) and if we know that 'data' can fit an entire frame
      {
         Int decoded=od.decode(temp, temp_size, data, size);
         if( decoded<=0)return false;
         data =(Byte*)data+decoded;
         read+=decoded;
         size-=decoded;
      }else
      if(!od.decode(temp, temp_size, buffer))return false;
      return true;
   }

   WebmOpusFile(SoundStream::Params &params, File &file, Int track_number, mkvparser::Segment* &segment, C Byte *priv_data, Int priv_data_size) : WebmFile(params, file, track_number, segment, OPUS_FRAME_HISTORY)
   {
      if(priv_data_size>=12 && EqualMem(priv_data, "OpusHead", 8))
      {
         buffer_pos=delay=(priv_data[10]|(priv_data[11]<<8))*params.block;
         MAX(params.size-=delay, 0);
      }
      od.create(params.channels);
   }
};

struct SndOpusFile
{
   OpusDecoder od;
   File       *file;
   Mems<U16>   packet_sizes;
   Memc<Byte>  buffer;
   Byte        extra_frames;
   U16         delay;
   Int         packet_index, buffer_pos;
   UInt        frame_size, audio_offset;

   Bool pos(Long raw)
   {
      if(raw<0)return false;
      raw+=delay;
      od.reset(); // need to reset state
      packet_index=Unsigned(raw)/frame_size;
      MAX(packet_index-=extra_frames, 0);
      buffer_pos=raw-packet_index*frame_size; buffer.clear();
      Long pos=audio_offset; for(Int i=0, end=Min(packet_sizes.elms(), packet_index); i<end; i++)pos+=packet_sizes[i];
      return file->pos(pos);
   }
   Int set(Ptr data, Int size) // !! Warning: 'size' should be clamped to skip the zero padding at the end !! this is done in 'SoundStream.set' because 'SndOpusFile' doesn't store current position and total size (to reduce memory usage)
   {
      Int read=0;
      for(; size>0; )
      {
         Int left=buffer.elms()-buffer_pos;
         if( left>0) // if something left in buffer
         {
            Int copy=Min(size, left);
            CopyFast(data, &buffer[buffer_pos], copy);
            size      -=copy;
            buffer_pos+=copy;
            read      +=copy;
            data       =(Byte*)data+copy;
            if(buffer_pos>=buffer.elms()){buffer_pos=0; buffer.clear();}
         }else // need to decode
         {
            if(!InRange(packet_index, packet_sizes))break;
            Byte packet_data[65536];
            Int  packet_size=packet_sizes[packet_index++];
            if(  packet_size>SIZE(packet_data) || !file->getFast(packet_data, packet_size))break;

            if(buffer_pos==0 && size>=frame_size) // decode directly into 'data', we can do this only if we're not skipping (buffer_pos==0) and if we know that 'data' can fit an entire frame
            {
               Int decoded=od.decode(packet_data, packet_size, data, size);
               if( decoded<=0)break;
               read+=decoded;
               size-=decoded;
               data =(Byte*)data+decoded;
            }else
            {
               if(!od.decode(packet_data, packet_size, buffer))break;
               if(buffer_pos>=buffer.elms()) // if we have to skip everything
               {
                  buffer_pos-=buffer.elms(); // we've skipped this
                  buffer.clear();
               }
            }
         }
      }
      return read;
   }
   Bool init(SoundStream &stream)
   {
      if(od.create(stream.channels(), stream.frequency()))
      {
         delay=stream._f.decUIntV();
         const UInt frame_samples=OPUS_FRAME_SIZE_BEST_QUALITY*48; // 'SndOpusEncoder' always uses OPUS_FRAME_SIZE_BEST_QUALITY
         packet_sizes.setNum(DivCeil(ULong(stream.samples()+delay), (ULong)frame_samples));
         if(stream._f.getN(packet_sizes.data(), packet_sizes.elms()))
         {
            T.extra_frames= OPUS_FRAME_HISTORY;
            T.delay      *= stream.block(); // convert from sample to raw
            T.frame_size  = stream.block()*frame_samples;
            T.packet_index= 0;
            T.buffer_pos  = delay;
            T.file        =&stream._f;
            T.audio_offset= stream._f.pos();
            return true;
         }
      }
      return false;
   }
};
#endif
/******************************************************************************/
// WEBM VORBIS+OPUS
/******************************************************************************/
#if SUPPORT_VORBIS || SUPPORT_OPUS
static WebmFile* LoadWebmHeader(File &f, SoundStream::Params &params, SOUND_CODEC &codec)
{
   codec=SOUND_NONE;
   MkvReader reader(f);
   long long pos=0;
   mkvparser::EBMLHeader ebmlHeader; if(ebmlHeader.Parse(&reader, pos)>=0)
   {
      mkvparser::Segment *segment=null;
      mkvparser::Segment::CreateInstance(&reader, pos, segment);
      if(segment)
      {
         if(segment->Load()>=0)
         if(C mkvparser::SegmentInfo *segment_info=segment->GetInfo())
         if(C mkvparser::Tracks *tracks=segment->GetTracks())
         {
            Int  tracks_num=tracks->GetTracksCount();
            FREP(tracks_num) // process in order
               if(C mkvparser::Track *track=tracks->GetTrackByIndex(i))
               if(track->GetType()==mkvparser::Track::kAudio)
            {
            #if SUPPORT_VORBIS
               if(Equal(track->GetCodecId(), "A_VORBIS"))codec=SOUND_WEBM_VORBIS;
            #endif
            #if SUPPORT_OPUS
               if(Equal(track->GetCodecId(), "A_OPUS"  ))codec=SOUND_WEBM_OPUS;
            #endif
               if(codec) // stop on first found
               {
                C mkvparser::AudioTrack &audio_track=SCAST(C mkvparser::AudioTrack, *track);
                  Int frequency=RoundPos(audio_track.GetSamplingRate());
                  if(audio_track.GetChannels()>=1 && audio_track.GetChannels()<=2 // support only mono/stereo
                  && frequency>=1) // to avoid div by zero
                  {
                     Long duration_ns=segment_info->GetDuration();
                     params.channels =audio_track.GetChannels();
                     params.frequency=frequency;
                     params.bytes    =2;
                     params.block    =params.channels*params.bytes;
                     params.size     =(duration_ns*params.frequency/NANOS)*params.block; // mul by 'params.block' last to make sure it's a multiple
                     params.bit_rate =(params.size ? f.size()*8*params.block*params.frequency/params.size : 0);
                     size_t priv_data_size; C Byte *priv_data=audio_track.GetCodecPrivate(priv_data_size);
                  #if SUPPORT_VORBIS
                     if(codec==SOUND_WEBM_VORBIS)return new WebmVorbisFile(params, f, audio_track.GetNumber(), segment, priv_data, priv_data_size);
                  #endif
                  #if SUPPORT_OPUS
                     if(codec==SOUND_WEBM_OPUS  )return new WebmOpusFile(params, f, audio_track.GetNumber(), segment, priv_data, priv_data_size);
                  #endif
                  }
               }
            }
         }
         Delete(segment);
      }
   }
   params.zero(); return null;
}
#endif
/******************************************************************************/
// MP3
/******************************************************************************/
#if SUPPORT_MP3
struct MP3
{
   mp3dec_t   dec;
   File      *file;
   Int        buf_pos , // amount of bytes to skip
              buf_left, // amount of bytes still in the buffer
              block   ,
              frame_index,
              frame_samples;
   Byte       buf[MP3_FRAME_SAMPLES*SIZE(I16)*2]; // max samples * sample size * 2 channels
   Mems<UInt> frame_pos;

   Bool pos(Long sample)
   {
      if(sample>=0)
      {
         sample+=MP3_SAMPLE_DELAY;
         Int frame_index=sample/frame_samples; if(InRange(frame_index, frame_pos))
         {
            MAX(frame_index-=2, 0); // "I suggest start from-middle decoding 2 frames earlier and discard it's data." according to https://github.com/lieff/minimp3/issues/38
            T.frame_index=frame_index;
            buf_pos =(sample-frame_index*frame_samples)*block;
            buf_left=0;
            mp3dec_init(&dec); // reset decoder state
            return file->pos(frame_pos[frame_index]);
         }
      }
      return false;
   }
   Int decode(const uint8_t *mp3, int mp3_bytes, Byte *pcm) // return number of bytes decoded (not samples)
   {
      mp3dec_frame_info_t frame_info;
      Int samples=mp3dec_decode_frame(&dec, mp3, mp3_bytes, (mp3d_sample_t*)pcm, &frame_info);
      if( samples==frame_samples)return frame_samples*block; // all OK
      switch(samples)
      {
         default: return 0; // failed
         case -1: // reservoir failed
         {
            if(1 // actually always continue because some MP3's can have reservoir broken at the start
            || buf_pos>=frame_samples*block) // here allow to continue ONLY if we intend to skip this entire frame (this is needed because of how the MP3 library works, when seeking we need to go 2 frames before, reservoir is lost which causes failures)
            {
               Zero(pcm+buf_pos, frame_samples*block-buf_pos); // zero only data that we're going to use (and not skip)
               return frame_samples*block;
            }
         }return 0;
      }
   }
   Int set(Ptr data, Int size)
   {
      Int read=0;
      for(; size>0; )
      {
         if(buf_left>0) // if something left in buffer
         {
            Int copy=Min(size, buf_left);
            CopyFast(data, buf+buf_pos, copy);
            data     =(Byte*)data+copy;
            size    -=copy;
            read    +=copy;
            buf_left-=copy; if(buf_left>0)buf_pos+=copy;else buf_pos=0;
         }else // need to decode
         {
            Int  frame_size;
            Byte frame_data[MAX_L3_FRAME_PAYLOAD_BYTES];
            if(InRange(frame_index+1, frame_pos)) // if there is a next frame
            {
               frame_size=frame_pos[++frame_index]-file->pos(); // calculate distance between next frame position and current position
            }else
            if(InRange(frame_index, frame_pos)) // last frame
            {
               frame_index++; // increase so we won't process again
               frame_size=file->left();
            }else break;
            MIN(frame_size, SIZEI(frame_data));
            if(!file->getFast(frame_data, frame_size))break;

            if(buf_pos==0 && size>=SIZE(buf)) // decode directly into 'data', we can do this only if we're not skipping (buf_pos==0) and if we know that 'data' can fit an entire frame
            {
               Int decoded=decode(frame_data, frame_size, (Byte*)data); if(decoded<=0)break;
               read+=decoded;
               size-=decoded;
               data =(Byte*)data+decoded;
            }else
            {
               Int decoded=decode(frame_data, frame_size, buf); if(decoded<=0)break;
                  buf_left =decoded-buf_pos;
               if(buf_left<=0) // if we had to skip everything
               {
                  buf_left=0; // nothing left
                  buf_pos-=decoded; // we've skipped this
               }
            }
         }
      }
      return read;
   }
   MP3(File &f, SoundStream::Params &params, Int frame_samples, Memt<UInt> &frame_pos)
   {
      T.file=&f;
      T.frame_pos=frame_pos;
      T.block=params.block;
      T.buf_pos=MP3_SAMPLE_DELAY*block;
      T.buf_left=T.frame_index=0;
      T.frame_samples=frame_samples;
      mp3dec_init(&dec);
   }
};
static Int mp3dec_skip_id3v2(const uint8_t *buf, UInt buf_size)
{
   if(buf_size>10 && !strncmp((char *)buf, "ID3", 3))
        return (((buf[6] & 0x7f) << 21) | ((buf[7] & 0x7f) << 14) | ((buf[8] & 0x7f) << 7) | (buf[9] & 0x7f)) + 10;
   return 0;
}
static MP3* LoadMP3Header(File &f, SoundStream::Params &params)
{
   params.zero();

   Byte temp[65536];
   Int  buf_size=Min(f.left(), SIZEI(temp));
   if(!f.getFast(temp, buf_size))return null;

   Int id3v2size=mp3dec_skip_id3v2(temp, buf_size); if(id3v2size>buf_size)return null;

   Byte *buf      =temp+id3v2size;
         buf_size-=     id3v2size;

   mp3dec_t dec; mp3dec_init(&dec);
   Int        frame_samples;
   Memt<UInt> frame_pos;
   ULong      bit_rate=0;

   for(;;)
   {
      if(buf_size<MAX_FRAME_SYNC_MATCHES*MAX_L3_FRAME_PAYLOAD_BYTES && !f.end()) // if not enough data left for N frames, but still have some data in the file
      {
         CopyFast(temp, buf, buf_size); // copy leftover data to the start
         buf=temp; // we now have data at the start
         Int read=Min(f.left(), SIZEI(temp)-buf_size); // calculate how much we can read
         if(!f.getFast(buf+buf_size, read))return null; // read extra data
         buf_size+=read;
      }

      mp3dec_frame_info_t frame_info;
      Int samples=mp3dec_decode_frame(&dec, buf, buf_size, null, &frame_info); Int offset_size=frame_info.frame_offset+frame_info.frame_size; if(!offset_size)break;
      if( samples>0)
      {
         if(!frame_pos.elms())
         {
            if(frame_info.channels<1 || frame_info.channels>2)return null; // support only mono and stereo
            if(samples!=MP3_FRAME_SAMPLES && samples!=MP3_FRAME_SAMPLES2)return null; // support only these samples
            params.channels =frame_info.channels;
            params.frequency=frame_info.hz;
            frame_samples   =samples;
         }else
         {
            if(params.channels !=frame_info.channels
            || params.frequency!=frame_info.hz
            || frame_samples   !=samples)return null;
         }
         frame_pos.add(f.pos()-buf_size+frame_info.frame_offset);
         bit_rate+=frame_info.bitrate_kbps;
      }
      buf     +=offset_size;
      buf_size-=offset_size;
   }
   if(frame_pos.elms())
   {
      Long total_samples=Long(frame_pos.elms())*frame_samples-MP3_SAMPLE_DELAY;
      params.bytes   =2;
      params.block   =params.channels*params.bytes;
      params.size    =params.block*total_samples;
      params.bit_rate=(1 ? bit_rate*1000/frame_pos.elms() : total_samples ? f.size()*8*params.frequency/total_samples : 0);
      f.pos(frame_pos[0]); // go to the first frame
      return new MP3(f, params, frame_samples, frame_pos);
   }
   return null;
}
#endif
/******************************************************************************/
// FLAC
/******************************************************************************/
#if SUPPORT_FLAC
struct FLAC
{
   FLAC__StreamDecoder *decoder;
   File                *file;
   Memc<Byte>           buffer;
   Int                  buffer_pos, read, to_read;
   Long                 seek;
   Ptr                  data;

   static FLAC__StreamDecoderReadStatus Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
   {
      File &f=*((FLAC*)client_data)->file;
      if(*bytes<=0)return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
      *bytes=f.getReturnSize(buffer, *bytes);
      return (*bytes>0) ? FLAC__STREAM_DECODER_READ_STATUS_CONTINUE : FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
   }
   static FLAC__StreamDecoderSeekStatus Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
   {
      File &f=*((FLAC*)client_data)->file;
      return f.pos(absolute_byte_offset) ? FLAC__STREAM_DECODER_SEEK_STATUS_OK : FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
   }
   static FLAC__StreamDecoderTellStatus Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
   {
      File &f=*((FLAC*)client_data)->file;
      *absolute_byte_offset=f.pos();
      return FLAC__STREAM_DECODER_TELL_STATUS_OK;
   }
   static FLAC__StreamDecoderLengthStatus Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
   {
      File &f=*((FLAC*)client_data)->file;
      *stream_length=f.size();
      return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
   }
   static FLAC__bool Eof(const FLAC__StreamDecoder *decoder, void *client_data)
   {
      File &f=*((FLAC*)client_data)->file;
      return f.end();
   }
   static void Meta(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
   {
      if(metadata && metadata->type==FLAC__METADATA_TYPE_STREAMINFO)
      {
         FLAC &flac=*(FLAC*)client_data; if(flac.to_read<0) // only if we're expecting metadata
         {
          C FLAC__StreamMetadata_StreamInfo &stream=metadata->data.stream_info;
            File &f=*flac.file;
            SoundStream::Params &params=*(SoundStream::Params*)flac.data;
            params.frequency=stream.sample_rate;
            params.channels =stream.channels;
            params.bytes    =stream.bits_per_sample/8;
            params.block    =params.channels*params.bytes;
            params.size     =params.block   *stream.total_samples;
            params.bit_rate =(stream.total_samples ? f.size()*8*params.frequency/stream.total_samples : 0);
         }
      }
   }
   static FLAC__StreamDecoderWriteStatus Write(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
   {
      FLAC &flac=*(FLAC*)client_data;
      if(flac.to_read>0) // process this only if we want to read something, this is to avoid cases when this callback is called during some other flac call like seeking (yes this can happen)
      {
         Int frame_samples=frame->header.blocksize,
             channels     =frame->header.channels,
             bits         =frame->header.bits_per_sample,
             bytes        =bits/8,
             block        =bytes*channels,
             to_read_samples=flac.to_read/block,
             read_samples =0,
             n=Min(to_read_samples, frame_samples);

         switch(bits)
         {
            default: flac.to_read=0; break;

            case 8:
            {
               U8 *data=(U8*)flac.data;
               for(; read_samples<n; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples]+128; // FLAC data will be signed, however we need unsigned

               Int read=read_samples*block; // this needs to be set before processing left-over data to the 'buffer'
               flac.   read+=read;
               flac.to_read-=read;
               flac.   data =data;

               if(Int left_samples=frame_samples-read_samples)
               {
                  data=(U8*)flac.buffer.setNum(left_samples*block).data();
                  for(; read_samples<frame_samples; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples]+128; // FLAC data will be signed, however we need unsigned
               }
            }break;

            case 16:
            {
               I16 *data=(I16*)flac.data;
               for(; read_samples<n; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples];

               Int read=read_samples*block; // this needs to be set before processing left-over data to the 'buffer'
               flac.   read+=read;
               flac.to_read-=read;
               flac.   data =data;

               if(Int left_samples=frame_samples-read_samples)
               {
                  data=(I16*)flac.buffer.setNum(left_samples*block).data();
                  for(; read_samples<frame_samples; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples];
               }
            }break;

            case 24:
            {
               Int24 *data=(Int24*)flac.data;
               for(; read_samples<n; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples];

               Int read=read_samples*block; // this needs to be set before processing left-over data to the 'buffer'
               flac.   read+=read;
               flac.to_read-=read;
               flac.   data =data;

               if(Int left_samples=frame_samples-read_samples)
               {
                  data=(Int24*)flac.buffer.setNum(left_samples*block).data();
                  for(; read_samples<frame_samples; read_samples++)FREPD(c, channels)*data++=buffer[c][read_samples];
               }
            }break;
         }
      }
      return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
   }
   static void Error(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {}

   void    del() {if(decoder){FLAC__stream_decoder_delete(decoder); decoder=null;}}
  ~FLAC() {del();}
   FLAC(File &f, SoundStream::Params &params)
   {
      if(decoder=FLAC__stream_decoder_new())
      {
         buffer_pos=read=0;
         seek=-1;
         to_read=-1; // set that we're expecting meta
         data=&params; // store pointer to the 'params' which will be used in 'Meta' callback (one time use)
         file=&f;
            FLAC__stream_decoder_set_md5_checking             (decoder, false);
         if(FLAC__stream_decoder_init_stream                  (decoder, Read, Seek, Tell, Length, Eof, Write, Meta, Error, this)==FLAC__STREAM_DECODER_INIT_STATUS_OK)
         if(FLAC__stream_decoder_process_until_end_of_metadata(decoder))
         {
            data=null;
            to_read=0;
         #if 0 // accessing values here results in 0, use 'Meta' callback instead
            FLAC__uint64 samples=FLAC__stream_decoder_get_total_samples  (decoder);
            params.frequency    =FLAC__stream_decoder_get_sample_rate    (decoder);
            params.channels     =FLAC__stream_decoder_get_channels       (decoder);
            params.bytes        =FLAC__stream_decoder_get_bits_per_sample(decoder)/8;
            params.block        =params.channels*params.bytes;
            params.size         =params.block   *samples;
            params.bit_rate     =(samples ? f.size()*8*params.frequency/samples : 0);
         #endif
            return; // ok
         }
         del();
      }
   }
   void clearBuffer() {buffer_pos=0; buffer.clear();}
   Bool pos(Long sample)
   {
      clearBuffer();
      if(sample>=0)
      {
         seek=sample; // don't seek here, because it may trigger calling 'Write' function with the sound data for desired sample
         return true;
      }
      return false;
   }
   Int set(Ptr data, Int size)
   {
      Int read=0, buffer_left=Min(buffer.elms()-buffer_pos, size);
      if( buffer_left>0) // if we have any left in the buffer
      {
         CopyFast(data, buffer.data()+buffer_pos, buffer_left);
         buffer_pos+=buffer_left; if(buffer_pos>=buffer.elms())clearBuffer(); // if we've read the entire buffer then clear it
         data       =(Byte*)data+buffer_left;
         read      +=buffer_left;
         size      -=buffer_left;
      }
      if(size>0) // if we still need more
      {
         T.   data=data;
         T.   read=0;
         T.to_read=size;
         if(seek>=0) // if seek was requested
         {
            FLAC__bool ok=FLAC__stream_decoder_seek_absolute(decoder, seek); // this may trigger 'Write' function
            seek=-1; // disable seeking
            if(!ok)goto finish;
            if(read+=T.read)goto finish;
         }
         if(FLAC__stream_decoder_process_single(decoder))read+=T.read;
      finish:
         T.to_read=0; // make sure to clear this because 'Write' callback can be called at any operation (for example when seeking)
      }
      return read;
   }
};
extern "C"
{
   FILE* fopen_utf8(const char *filename, const char *mode) {return null;} // create a dummy function which is needed for FLAC
}
static FLAC* LoadFLACHeader(File &f, SoundStream::Params &params)
{
   params.zero();
   FLAC *flac=new FLAC(f, params);
   if(flac->decoder)return flac;
   delete flac;     return null;
}
#endif
/******************************************************************************/
// AAC
/******************************************************************************/
#if SUPPORT_AAC
static Bool Decode(HANDLE_AACDECODER decoder, UCHAR *packet_data, UInt packet_size, Ptr data, Int data_size, UInt flag)
{
   UINT   valid=packet_size;
   return aacDecoder_Fill       (decoder, &packet_data, &packet_size, &valid          )==AAC_DEC_OK
       && aacDecoder_DecodeFrame(decoder, (Short*)data,    data_size/SIZE(Short), flag)==AAC_DEC_OK;
}
struct AAC
{
   struct Part
   {
      Long      pos;
      Mems<U16> packet_sizes;
   };

   HANDLE_AACDECODER decoder;
   Int               part_index, packet_index, buffer_pos;
   UInt              frame_size, flag, delay;
   File             *file;
   Memc<Byte>        buffer;
   Mems<Part>        parts;

   AAC() {decoder=null; part_index=packet_index=buffer_pos=0; frame_size=flag=delay=0; file=null;}
  ~AAC() {if(decoder){aacDecoder_Close(decoder); decoder=null;}}

   inline Bool decode(UCHAR *packet_data, UInt packet_size, Ptr data, Int data_size)
   {
      Bool   ok=Decode(decoder, packet_data, packet_size, data, data_size, flag); flag=0;
      return ok;
   }

   void advance(            ) {if(InRange(part_index, parts))advance(parts[part_index]);}
   void advance(C Part &part)
   {
      if(++packet_index>=part.packet_sizes.elms())
      {
         packet_index=0;
         if(InRange(++part_index, parts))
         {
          C Part &part=parts[part_index];
            file->pos(part.pos);
         }
      }
   }
   Bool pos(Long raw)
   {
      if(raw<0)return false;
      raw+=delay;
      Long frame_index=Unsigned(raw)/frame_size;
      MAX( frame_index-=3, 0); // go back at least 3 frames
      buffer_pos=raw-frame_index*frame_size; buffer.clear();
      FREPA(parts)
      {
       C Part &part=parts[i];
         if(InRange(frame_index, part.packet_sizes))
         {
                    flag=AACDEC_CLRHIST|AACDEC_INTR; // this will reset the state
              part_index=i;
            packet_index=frame_index;
            Long pos=part.pos; REP(packet_index)pos+=part.packet_sizes[i]; return file->pos(pos);
         }
         frame_index-=part.packet_sizes.elms();
      }
      return false;
   }
   Int set(Ptr data, Int size)
   {
      Int read=0;
      for(; size>0; )
      {
         Int left=buffer.elms()-buffer_pos;
         if( left>0) // if something left in buffer
         {
            Int copy=Min(size, left);
            CopyFast(data, &buffer[buffer_pos], copy);
            size      -=copy;
            buffer_pos+=copy;
            read      +=copy;
            data       =(Byte*)data+copy;
            if(buffer_pos>=buffer.elms()){buffer_pos=0; buffer.clear();}
         }else // need to decode
         {
            if(!InRange(part_index, parts))break; C Part &part=parts[part_index];
            if(!InRange(packet_index, part.packet_sizes))break;
            Byte packet_data[65536];
            Int  packet_size=part.packet_sizes[packet_index];
            if(  packet_size>SIZE(packet_data) || !file->getFast(packet_data, packet_size))break;
            advance(part);

            if(buffer_pos==0 && size>=frame_size) // decode directly into 'data', we can do this only if we're not skipping (buffer_pos==0) and if we know that 'data' can fit an entire frame
            {
               if(!decode(packet_data, packet_size, data, size))break;
               read+=frame_size;
               size-=frame_size;
               data =(Byte*)data+frame_size;
            }else
            {
               buffer.setNum(frame_size);
               if(!decode(packet_data, packet_size, buffer.data(), buffer.elms())){buffer.clear(); break;}
               if(buffer_pos>=buffer.elms()) // if we have to skip everything
               {
                  buffer_pos-=buffer.elms(); // we've skipped this
                  buffer.clear();
               }
            }
         }
      }
      return read;
   }
};
#include "../../H/Misc/MP4.h"
static AAC* LoadAACHeader(File &f, SoundStream::Params &params, Int audio_track=0, Bool limit_to_stereo=true)
{
   params.zero();
   MP4 mp4; if(mp4.read(f))FREPA(mp4.tracks)
   {
      MP4::Track &track=mp4.tracks[i]; if(track.decoder_config.elms() && !audio_track--)
      {
         if(HANDLE_AACDECODER decoder=aacDecoder_Open(TT_MP4_RAW, 1))
         {
            UCHAR *ASC_data=track.decoder_config.data();
            UINT   ASC_size=track.decoder_config.elms();
            if(limit_to_stereo)aacDecoder_SetParam(decoder, AAC_PCM_MAX_OUTPUT_CHANNELS, 2);
            if(aacDecoder_ConfigRaw(decoder, &ASC_data, &ASC_size)==AAC_DEC_OK)
               if(CStreamInfo *info=aacDecoder_GetStreamInfo(decoder))
               if(track.frames())
            {
               // we have to decode one frame to get actual parameters of the audio stream, because 'info' at this point is just a guidance
               UInt       frame_size  =track.frameSize  (0);
               Long       frame_offset=track.frameOffset(0);
               Memt<Byte> frame_data; frame_data.setNum(frame_size);
               if(f.pos(frame_offset) && frame_data.loadRawData(f))
               {
                  Int channels=Max(2, track.channels, info->channelConfig, info->aacNumChannels); if(limit_to_stereo)MIN(channels, 2); // use Max(2 because in one case (track.channels=1, info->channelConfig=1, info->aacNumChannels=0, but info->numChannels=2)
                  Int frame_samples=Max(2048, info->aacSamplesPerFrame); // Max(2048, because in one case info->aacSamplesPerFrame=1024, but info->frameSize=2048
                  Memt<Byte> pcm_data; pcm_data.setNum(frame_samples*channels*SIZE(Short));
                  if(Decode(decoder, frame_data.data(), frame_data.elms(), pcm_data.data(), pcm_data.elms(), 0))
                  {
                     params.bytes    =2;
                     params.channels =info->numChannels;
                     params.block    =params.channels*params.bytes;
                     params.frequency=info->sampleRate;
                     // time in seconds = track.duration / track.time_scale
                     params.size=((track.duration>=0 && track.time_scale) ? track.duration*params.frequency/track.time_scale // use given duration if known
                                                                          : Long(track.frames())*info->frameSize             // calculate time of all frames
                                 );
                     MAX(params.size-=info->outputDelay, 0); params.size*=params.block;
                     params.bit_rate=track.avg_bit_rate;
                     if(!params.bit_rate && params.size)params.bit_rate=track.dataSize()*8*params.block*params.frequency/params.size;

                     AAC *aac=new AAC;
                     aac->decoder=decoder;
                     aac->frame_size=info->frameSize*params.block; // 'info->frameSize' is number of samples in 1 channel
                     aac->file=&f;
                     aac->buffer_pos=aac->delay=info->outputDelay*params.block;
                     aac->buffer.setNum(aac->frame_size).copyFrom(pcm_data.data()); // copy what we've decoded
                     if(aac->buffer_pos>=aac->buffer.elms()) // if we have to skip everything
                     {
                        aac->buffer_pos-=aac->buffer.elms(); // we've skipped this
                        aac->buffer.clear();
                     }

                     // setup aac->parts
                     Long data_pos=-1;
                  #if 0 // slow method
                     FREP(track.frames())
                     {
                        UInt frame_size  =track.frameSize  (i);
                        Long frame_offset=track.frameOffset(i);
                        if(data_pos!=frame_offset || !aac->parts.elms())aac->parts.New().pos=data_pos=frame_offset;
                        aac->parts.last().packet_sizes.add(frame_size);
                        data_pos+=frame_size;
                     }
                  #else
                     Int parts=0; Mems<U16> *part_packet_sizes=null;
                     if(track.parts.elms())
                     {
                        aac->parts.setNum(track.parts.elms()); // reserve memory because most likely this size we're going to get
                        FREPA(track.parts)
                        {
                         C MP4::Track::Part &part=track.parts[i]; if(part.frames)
                           {
                              if(data_pos!=part.pos || !parts){AAC::Part &aac_part=aac->parts(parts++); aac_part.pos=data_pos=part.pos; part_packet_sizes=&aac_part.packet_sizes;}
                              Int first=part_packet_sizes->addNum(part.frames);
                              if(part.frame_sizes.elms())
                              {
                                 REPA(part.frame_sizes)data_pos+=((*part_packet_sizes)[first+i]=part.frame_sizes[i]);
                              }else
                              {
                                 for(Int i=first; i<part_packet_sizes->elms(); i++)(*part_packet_sizes)[i]=part.frame_size;
                                 data_pos+=part.frames*part.frame_size;
                              }
                           }
                        }
                     }else
                     if(track.stsc.elms())
                     {
                        aac->parts.setNum(track.chunk_offset.elms()); // reserve memory because most likely this size we're going to get
                        Int cur_frame=0, total_frames=track.frames(), chunk_frames=track.stsc[0].frames_per_chunk, chunk_i=1; // start with 1 because MP4 has chunk indexes starting from 1
                        if(track.frame_sizes.elms())MIN(total_frames, track.frame_sizes.elms());
                        for(Int i=1; i<track.stsc.elms(); i++)
                        {
                         C MP4::Track::STSC &next=track.stsc[i];

                           // process chunks in this run
                           if(chunk_frames>0)for(; chunk_i<next.first_chunk; chunk_i++)
                           {
                              if(Int add_frames=Min(total_frames-cur_frame, chunk_frames))
                              {
                                 Long chunk_offset=track.chunkOffset(chunk_i-1); // -1 because we need 0-based indexes for 'chunkOffset'
                                 if(data_pos!=chunk_offset || !parts){AAC::Part &aac_part=aac->parts(parts++); aac_part.pos=data_pos=chunk_offset; part_packet_sizes=&aac_part.packet_sizes;}
                                 Int first=part_packet_sizes->addNum(add_frames);
                                 if(track.frame_sizes.elms())
                                 {
                                    FREP(add_frames)data_pos+=((*part_packet_sizes)[first+i]=track.frame_sizes[cur_frame++]);
                                 }else
                                 {
                                    REP(add_frames)(*part_packet_sizes)[first+i]=track.frame_size;
                                    cur_frame+=add_frames;
                                    data_pos +=add_frames*track.frame_size;
                                 }
                              }
                           }else chunk_i=next.first_chunk;

                           chunk_frames=next.frames_per_chunk;
                        }

                        // process all chunks that are left
                        if(chunk_frames>0)for(chunk_i--; chunk_i<track.chunk_offset.elms(); chunk_i++) // first convert to 0-based indexes
                           if(Int add_frames=Min(total_frames-cur_frame, chunk_frames))
                        {
                           Long chunk_offset=track.chunk_offset[chunk_i];
                           if(data_pos!=chunk_offset || !parts){AAC::Part &aac_part=aac->parts(parts++); aac_part.pos=data_pos=chunk_offset; part_packet_sizes=&aac_part.packet_sizes;}
                           Int first=part_packet_sizes->addNum(add_frames);
                           if(track.frame_sizes.elms())
                           {
                              FREP(add_frames)data_pos+=((*part_packet_sizes)[first+i]=track.frame_sizes[cur_frame++]);
                           }else
                           {
                              REP(add_frames)(*part_packet_sizes)[first+i]=track.frame_size;
                              cur_frame+=add_frames;
                              data_pos +=add_frames*track.frame_size;
                           }
                        }
                     }
                     aac->parts.setNum(parts); // remove allocated but unprocessed parts (this can happen)
                  #endif

                     aac->advance(); // !! call after 'aac->parts' are setup !! we've already read one packet, so we need to advance

                     return aac;
                  }
               }
            }
            aacDecoder_Close(decoder); // error
         }
         break;
      }
   }
   return null;
}
#endif
/******************************************************************************/
// SOUND CACHE
/******************************************************************************/
Bool CacheSound(C Str &name)
{
   return FileDatas.get(name)!=null; // load and keep forever
}
Bool CacheSound(C UID &id)
{
   return FileDatas.get(id)!=null; // load and keep forever
}
/******************************************************************************/
// SOUND STREAM
/******************************************************************************/
Flt SoundStream::Params::length()C {Int div=block*frequency; return div ? Dbl(size)/div : 0;} // use Dbl because 'size' can be huge (more than INT_MAX)
/******************************************************************************/
void SoundStream::del()
{
   switch(_codec)
   {
   #if SUPPORT_FLAC
      case SOUND_FLAC: if(FLAC* &flac=(FLAC*&)_extra)Delete(flac); break;
   #endif
   #if SUPPORT_VORBIS
      case SOUND_OGG_VORBIS : if(OggVorbis_File* &vorbis=(OggVorbis_File*&)_extra){ov_clear(vorbis); Free(vorbis);} break;
      case SOUND_WEBM_VORBIS: if(WebmVorbisFile* &vorbis=(WebmVorbisFile*&)_extra)Delete(vorbis); break;
      case SOUND_SND_VORBIS : if( SndVorbisFile* &vorbis=( SndVorbisFile*&)_extra)Delete(vorbis); break;
   #endif
   #if SUPPORT_OPUS
      case SOUND_OGG_OPUS : if( OggOpusFile* &opus=( OggOpusFile*&)_extra){op_free(opus); opus=null;} break;
      case SOUND_WEBM_OPUS: if(WebmOpusFile* &opus=(WebmOpusFile*&)_extra)Delete(opus); break;
      case SOUND_SND_OPUS : if( SndOpusFile* &opus=( SndOpusFile*&)_extra)Delete(opus); break;
   #endif
   #if SUPPORT_MP3
      case SOUND_MP3: if(MP3* &mp3=(MP3*&)_extra)Delete(mp3); break;
   #endif
   #if SUPPORT_AAC
      case SOUND_AAC: if(AAC* &aac=(AAC*&)_extra)Delete(aac); break;
   #endif
   }
   if(_callback){_callback->del(); _callback=null;}
  _codec=SOUND_NONE;
  _pos    =0;
  _raw_ofs=0; // !! clear '_raw_ofs' after releasing '_extra' in case they form a union !!
  _f  .del();
  _par.zero();
}
SoundStream::SoundStream()
{
  _codec   =SOUND_NONE;
  _pos     =0;
  _raw_ofs =0;
  _extra   =null;
  _callback=null;
  _par.zero();
}
/******************************************************************************/
Bool SoundStream::open(C Str &name)
{
   if(name.is())
   {
      if(FileData *fd=FileDatas.find(name)){_f.readMem(fd->data.data(), fd->data.elms()); return true;} // try from cache if was loaded
      return _f.readTry(name); // try from file on disk
   }
   return false;
}
Bool SoundStream::create(C Str &name) // !! warning: set T._codec as last thing because it indicates that stream is loaded !!
{
   del();
   if(open(name))
   {
      UInt   cc4=_f.getUInt(); // first check cc4 to avoid libs reading other types of formats (in one case mp3 lib read a flac file and reported it was mp3)
      switch(cc4)
      {
         case CC4_SND: // Esenthel Sound
         {
           _f.pos(0); SND_CODEC codec; if(LoadSndHeader(_f, _par, codec))switch(codec)
            {
               case SND_RAW_16: _raw_ofs=_f.pos(); _codec=SOUND_WAV; return true; // set '_codec' as the last thing

            #if SUPPORT_VORBIS
               case SND_VORBIS:
               {
                  SndVorbisFile temp; if(temp.init(T))
                  {
                     SndVorbisFile *extra=new SndVorbisFile; Swap(*extra, temp);
                    _extra=extra; _codec=SOUND_SND_VORBIS; return true; // set '_codec' as the last thing
                  }
               }break;
            #endif

            #if SUPPORT_OPUS
               case SND_OPUS:
               {
                  SndOpusFile temp; if(temp.init(T))
                  {
                     SndOpusFile *extra=new SndOpusFile; Swap(*extra, temp);
                    _extra=extra; _codec=SOUND_SND_OPUS; return true; // set '_codec' as the last thing
                  }
               }break;
            #endif
            }
         }break;

         case CC4('R','I','F','F'): // WAV
         {
           _f.pos(0); switch(LoadWavHeader(_f, _par)) // reset file position after previous attempt
            {
               case SOUND_MP3: goto load_mp3;
               case SOUND_WAV: _raw_ofs=_f.pos(); _codec=SOUND_WAV; return true; // set '_codec' as the last thing
            }
         }break;

         case CC4('O', 'g', 'g', 'S'): // Ogg Vorbis and Opus
         {
         #if SUPPORT_VORBIS
           _f.pos(0); if(_extra=LoadOggVorbisHeader(_f, _par)){_codec=SOUND_OGG_VORBIS; return true;} // reset file position after previous attempt and set '_codec' as the last thing
         #endif

         #if SUPPORT_OPUS
           _f.pos(0); if(_extra=LoadOggOpusHeader(_f, _par)){_codec=SOUND_OGG_OPUS; return true;} // reset file position after previous attempt and set '_codec' as the last thing
         #endif
         }break;

         case 0xA3DF451A: // WebM Vorbis and Opus
         {
         #if SUPPORT_VORBIS || SUPPORT_OPUS
           _f.pos(0); SOUND_CODEC codec; if(_extra=LoadWebmHeader(_f, _par, codec)){_codec=codec; return true;} // reset file position after previous attempt and set '_codec' as the last thing
         #endif
         }break;

         case CC4('f', 'L', 'a', 'C'): // Flac
         {
         #if SUPPORT_FLAC
           _f.pos(0); if(_extra=LoadFLACHeader(_f, _par)){_codec=SOUND_FLAC; return true;} // reset file position after previous attempt and set '_codec' as the last thing
         #endif
         }break;

         default:
         {
         #if SUPPORT_AAC // MP4
            if(!(cc4&0xFFFFFF)) // first UInt is the chunk size in BigEndian format, and for 'ftyp' it usually should be very small, so check 3 highest bytes that they are zero (have to check lowest because it's BigEndian)
            {
              _f.pos(4); if(_f.getUInt()==CC4('f','t','y','p'))
               {
                 _f.pos(0); if(_extra=LoadAACHeader(_f, _par)){_codec=SOUND_AAC; return true;} // reset file position after previous attempt and set '_codec' as the last thing
               }
            }
         #endif

            if((cc4&0xFFFFFF)==CC4('I', 'D', '3', 0) // inside ID3 tag
            || (cc4&0xE0FF)==0xE0FF // 0b11100000 == 0xE0 mask (3bit on-frame sync, 2bit off-MPEG type, 2bit off-layer type, 1bit off-CRC) http://www.mp3-tech.org/programmer/frame_header.html
            )                       // 0b11100000 == 0xE0      (111               , ignored           , ignored            , ignored     ) https://en.wikipedia.org/wiki/MP3#File_structure
            {
            #if SUPPORT_MP3
              _f.pos(0); // reset file position after previous attempt
            #endif
            load_mp3:;
            #if SUPPORT_MP3
              if(_extra=LoadMP3Header(_f, _par)){_codec=SOUND_MP3; return true;} // set '_codec' as the last thing
            #endif
            }
         }break;
      }
      del();
   }
   return false;
}
Bool SoundStream::create(C UID &id) {return create(_EncodeFileName(id));}
Bool SoundStream::create(SoundCallback &callback)
{
   del();
   Byte bits=0, channels=0; Int frequency=0, bit_rate=-1; Long size=-1;
   if(callback.create(bits, channels, frequency, size, bit_rate))
      if(bits     ==8 || bits     ==16 || bits==24)
      if(channels >=1 && channels <=2)
      if(frequency>=1 && frequency<=192000)
   {
     _par.bytes    = bits/8;
     _par.channels = channels;
     _par.frequency= frequency;
     _par.size     = size;
     _par.bit_rate = bit_rate;
     _par.block    = channels*_par.bytes;
     _callback     =&callback;
     _codec        = SOUND_DYNAMIC;
      return true;
   }
   callback.del(); return false;
}
/******************************************************************************/
Bool SoundStream::pos(Long pos)
{
   if(T._pos==pos)return true;
   switch(_codec)
   {
      case SOUND_WAV: if(pos>=0 && _f.pos(pos+_raw_ofs))goto ok; break;
   #if SUPPORT_FLAC
      case SOUND_FLAC: if(((FLAC*)_extra)->pos(pos/_par.block))goto ok; break;
   #endif
   #if SUPPORT_VORBIS
      case SOUND_OGG_VORBIS : if(!ov_pcm_seek((OggVorbis_File*)_extra, pos/_par.block))goto ok; break;
      case SOUND_WEBM_VORBIS: if(((WebmVorbisFile*)_extra)->pos(pos))goto ok; break;
      case SOUND_SND_VORBIS : if((( SndVorbisFile*)_extra)->pos(pos))goto ok; break;
   #endif
   #if SUPPORT_OPUS
      case SOUND_OGG_OPUS : if(!op_pcm_seek((OggOpusFile*)_extra, pos/_par.block))goto ok; break;
      case SOUND_WEBM_OPUS: if(((WebmOpusFile*)_extra)->pos(pos))goto ok; break;
      case SOUND_SND_OPUS : if((( SndOpusFile*)_extra)->pos(pos))goto ok; break;
   #endif
   #if SUPPORT_MP3
      case SOUND_MP3: if(((MP3*)_extra)->pos(pos/_par.block))goto ok; break;
   #endif
   #if SUPPORT_AAC
      case SOUND_AAC: if(((AAC*)_extra)->pos(pos))goto ok; break;
   #endif
      case SOUND_DYNAMIC: if(_callback->raw(pos))goto ok; break;
   }
   return false;
ok:
   T._pos=pos; return true;
}
Bool SoundStream::sample(Long sample) {return pos(sample*block());}
/******************************************************************************/
Int SoundStream::set(Ptr data, Int size)
{
   switch(_codec)
   {
      case SOUND_WAV: size=_f.getReturnSize(data, size); break;
   #if SUPPORT_FLAC
      case SOUND_FLAC: size=((FLAC*)_extra)->set(data, size); break;
   #endif
   #if SUPPORT_VORBIS
      case SOUND_OGG_VORBIS : size=ov_read((OggVorbis_File*)_extra, (char*)data, size, 0, 2, 1, null); break;
      case SOUND_WEBM_VORBIS: size=((WebmVorbisFile*)_extra)->set(data, Min(size, left())); break; // 'WebmVorbisFile' may generate some extra data at the end when seeking, so we need to limit what we can return to skip this padding
      case SOUND_SND_VORBIS : size=(( SndVorbisFile*)_extra)->set(data,     size         ); break;
   #endif
   #if SUPPORT_OPUS
      case SOUND_OGG_OPUS:
      {
         OggOpusFile *opus=(OggOpusFile*)_extra;
         switch(_par.channels)
         {
            default: size=0; break;
            case  2: size=op_read_stereo(opus, (opus_int16*)data, size/SIZE(opus_int16))*_par.block; break;
            case  1:
            {
               int link, samples=op_read(opus, (opus_int16*)data, size/SIZE(opus_int16), &link);
               size=((op_channel_count(opus, link)==1) ? samples*_par.block : 0);
            }break;
         }
      }break;
      case SOUND_WEBM_OPUS: size=((WebmOpusFile*)_extra)->set(data,     size         ); break;
      case SOUND_SND_OPUS : size=(( SndOpusFile*)_extra)->set(data, Min(size, left())); break; // 'SndOpusEncoder' may have introduced some zero padding which would result in extra data at the end, so we need to limit what we can return to skip this padding
   #endif
   #if SUPPORT_MP3
      case SOUND_MP3: size=((MP3*)_extra)->set(data, size); break;
   #endif
   #if SUPPORT_AAC
      case SOUND_AAC: size=((AAC*)_extra)->set(data, size); break;
   #endif
      case SOUND_DYNAMIC: size=_callback->set(data, size); break;
      default           : size=0; break;
   }
   if(size>0)T._pos+=size; return size;
}
/******************************************************************************/
void SoundStream::fastSeek()
{
   switch(_codec)
   {
   #if SUPPORT_OPUS
      case SOUND_WEBM_OPUS: ((WebmOpusFile*)_extra)->extra_frames=OPUS_FRAME_HISTORY_FAST; break;
      case SOUND_SND_OPUS : (( SndOpusFile*)_extra)->extra_frames=OPUS_FRAME_HISTORY_FAST; break;
   #endif
   }
}
void SoundStream::memAddressChanged()
{
   switch(_codec)
   {
   #if SUPPORT_FLAC
      case SOUND_FLAC: ((FLAC*)_extra)->file=&_f; break;
   #endif
   #if SUPPORT_VORBIS
      case SOUND_OGG_VORBIS : ((OggVorbis_File*)_extra)->datasource=&_f; break;
      case SOUND_WEBM_VORBIS: ((WebmVorbisFile*)_extra)->file      =&_f; break;
      case SOUND_SND_VORBIS : (( SndVorbisFile*)_extra)->file      =&_f; break;
   #endif
   #if SUPPORT_OPUS
      case SOUND_OGG_OPUS : (( OggOpusFile*)_extra)->stream=&_f; break;
      case SOUND_WEBM_OPUS: ((WebmOpusFile*)_extra)->file  =&_f; break;
      case SOUND_SND_OPUS : (( SndOpusFile*)_extra)->file  =&_f; break;
   #endif
   #if SUPPORT_MP3
      case SOUND_MP3: ((MP3*)_extra)->file=&_f; break;
   #endif
   #if SUPPORT_AAC
      case SOUND_AAC: ((AAC*)_extra)->file=&_f; break;
   #endif
   }
}
/******************************************************************************/
void    SoundHeader::zero     ()  {bytes=bits=channels=0; frequency=bit_rate=0; size=0; length=0; codec=SOUND_NONE;}
CChar8* SoundHeader::codecName()C {return CodecName(codec);}
Bool    SoundHeader::load     (C Str &name)
{
   SoundStream stream; if(stream.create(name))
   {
      bytes    =stream.bytes();
      bits     =stream.bits();
      channels =stream.channels();
      frequency=stream.frequency();
      bit_rate =stream.bitRate();
      size     =stream.size();
      length   =stream.length();
      codec    =stream.codec();
      return true;
   }
   zero(); return false;
}
/******************************************************************************/
static const Int OpusFreqs[]={8000, 12000, 16000, 24000, 48000}; // these are the only frequencies supported by Opus, values taken from comments in 'opus_encoder_create'
static       Int OpusFrequency(Int freq) // !! do not change in the future, because old 'SndOpusFile' will fail !!
{
   for(Int i=Elms(OpusFreqs); --i>0; )
   {
      Int avg=AvgI(OpusFreqs[i], OpusFreqs[i-1]);
      if(freq>=avg)return OpusFreqs[i]; // yes use >= instead of >, because this allows precise mapping if 'OpusFreqs' had 48000-2, 48000-1, 48000, 48000+1, 48000+2
   }
   return OpusFreqs[0];
}
/******************************************************************************/
OpusEncoder::OpusEncoder() {_encoder=null; _channels=_frame_samples=0;}
OpusEncoder& OpusEncoder::del()
{
#if SUPPORT_OPUS_ENC
   if(_encoder){opus_encoder_destroy((OE)_encoder); _encoder=null; _channels=_frame_samples=0; _data.del();}
#endif
   return T;
}
Bool OpusEncoder::create(Int channels, Bool voice, Int frequency)
{
#if SUPPORT_OPUS_ENC
   del();
   if(channels>=1 && channels<=2) // among others, this is needed for '_last_sample' which has room for up to 2 channels
   if(_encoder=opus_encoder_create(OpusFrequency(frequency), channels, voice ? OPUS_APPLICATION_VOIP : OPUS_APPLICATION_AUDIO, null))
   {
      T._channels=channels;
      T._last_sample_size=0;
      frameLength(OPUS_FRAME_SIZE_BEST_QUALITY); // 2.5 .. 60 range is valid
      complexity(1); // turn on highest complexity by default
      return true;
   }
#endif
   return false;
}
void OpusEncoder::reset()
{
#if SUPPORT_OPUS_ENC
   if(_encoder)
   {
      opus_encoder_ctl((OE)_encoder, OPUS_RESET_STATE);
      T._last_sample_size=0;
      T._data.clear();
   }
#endif
}

#if SUPPORT_OPUS_ENC
static const Int OpusValidFramesize[]={120, 240, 480, 960, 1920, 2880}; // ranges were taken from comments in 'opus_encode' function, 120 (2.5 ms), 960 (20 ms), 2880 (60 ms)
#endif

Int          OpusEncoder::frameLength()C {return _frame_samples/48;} // 48 because of OPUS_FREQUENCY/1000 = 48,000/1000 = 48
OpusEncoder& OpusEncoder::frameLength(Int length)
{
#if SUPPORT_OPUS_ENC
   if(_channels)
   {
      Int frame_size=length*48, nearest=0, dist;
      REPA(OpusValidFramesize)
      {
         Int value=OpusValidFramesize[i], d=Abs(frame_size-value);
         if(!nearest || d<dist){dist=d; nearest=value;}
      }
     _frame_samples=nearest; // set once at the end to avoid multi-threading issues
   }
#endif
   return T;
}

Int OpusEncoder::frequency()C
{
   Int bit_rate=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_GET_SAMPLE_RATE(&bit_rate));
#endif
   return bit_rate;
}

Int OpusEncoder::bitRate()C
{
   Int bit_rate=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_GET_BITRATE(&bit_rate));
#endif
   return bit_rate;
}
OpusEncoder& OpusEncoder::bitRate(Int bit_rate)
{
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_SET_BITRATE(Mid(bit_rate, 500, 512000))); // limits are described in 'OPUS_SET_BITRATE' header comments
#endif
   return T;
}

Bool OpusEncoder::vbr()C
{
   Int vbr=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_GET_VBR(&vbr));
#endif
   return vbr==1;
}
OpusEncoder& OpusEncoder::vbr(Bool on)
{
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_SET_VBR(on ? 1 : 0));
#endif
   return T;
}

Flt OpusEncoder::complexity()C
{
   Int complexity=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_GET_COMPLEXITY(&complexity));
#endif
   return complexity/10.0f;
}
OpusEncoder& OpusEncoder::complexity(Flt complexity)
{
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_SET_COMPLEXITY(Mid(RoundPos(complexity*10), 0, 10))); // range of 0..10 described in 'OPUS_SET_COMPLEXITY' header comments
#endif
   return T;
}

Int OpusEncoder::delay()C
{
   Int delay=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)opus_encoder_ctl((OE)_encoder, OPUS_GET_LOOKAHEAD(&delay));
#endif
   return delay;
}

#if SUPPORT_OPUS_ENC
static Bool OpusEncode(OE encoder, opus_int16 *data, Int frame_size, MemPtr<Byte> compressed_data, MemPtr<Int> packet_sizes)
{
   const Int max_packet_size=4096, // max 60 ms * 512000 bits per second = 60*512000/8/1000 = 3840 (round up to 4096 if there are any headers needed)
             size=compressed_data.elms(); // remember current size
   if(compressed_data.continuous()) // compress directly to 'compressed_data'
   {
      compressed_data.addNum(max_packet_size); // make room for another packet
      Int packet_size=opus_encode(encoder, data, frame_size, &compressed_data[size], max_packet_size);
      if( packet_size>0)
      {
         packet_sizes.add(packet_size);
         compressed_data.setNum(size+packet_size);
         return true;
      }
      // failed
      compressed_data.setNum(size); // restore original size
   }else // compress to 'temp' memory
   {
      Byte temp[max_packet_size];
      Int packet_size=opus_encode(encoder, data, frame_size, temp, SIZE(temp));
      if( packet_size>0)
      {
         packet_sizes.add(packet_size);
         compressed_data.addNum(packet_size);
         FREP(packet_size)compressed_data[size+i]=temp[i];
         return true;
      }
   }
   return false;
}
#endif
Bool OpusEncoder::encode(CPtr data, Int size, MemPtr<Byte> compressed_data, MemPtr<Int> packet_sizes)
{
   Bool ok=false;
   compressed_data.clear();
   packet_sizes   .clear();
#if SUPPORT_OPUS_ENC
   if(_encoder)
   {
      ok=true;
      if(data && size>0)
      {
         // store 'data' in '_last_sample'
         {
            Int block=T.block(), copy=Min(size, block), new_size=block-copy;
          //if(_last_sample_size+copy>block) // if what we will have will exceed the storage
          //if(_last_sample_size>block-copy)
            if(_last_sample_size>new_size  ) // have to move data in '_last_sample' to the left
            {
               MoveFast((Byte*)_last_sample, (Byte*)_last_sample+(_last_sample_size-new_size), new_size); // copy last 'new_size' bytes from '_last_sample'
              _last_sample_size=new_size;
            }
            CopyFast((Byte*)_last_sample+_last_sample_size, (Byte*)data+(size-copy), copy); // copy last 'copy' bytes from 'data'
           _last_sample_size+=copy;
         }
         const Int frame_samples=frameSamples(), // copy this first into a temp variable in case it gets changed on a secondary thread
                   frame_raw    =frame_samples*SIZE(I16)*_channels;
         for(; size>0 && ok; )
         {
            if(_data.elms() || size<frame_raw) // need to buffer
            {
               Int copy=Min(size, frame_raw-_data.elms()); // this can be negative if encoder was first called with a bigger frame which would allocate '_data' to some size, and then frame size got decreased
               if( copy>0)
               {
                  CopyFast(&_data[_data.addNum(copy)], data, copy);
                  data =(Byte*)data+copy;
                  size-=copy;
               }
               if(_data.elms()>=frame_raw) // if we've gathered enough to fill a frame
               {
                  ok&=OpusEncode((OE)_encoder, (opus_int16*)_data.data(), frame_samples, compressed_data, packet_sizes);
                  if(_data.elms()>frame_raw) // this can happen if frame size got changed, and now we have leftovers
                  {
                     MoveFast(_data.data(), _data.data()+frame_raw, _data.elms()-frame_raw); // move leftovers to the start
                    _data.addNum(-frame_raw); // remove processed elements
                  }else _data.clear();
               }
            }else // encode directly from provided data
            {
               ok&=OpusEncode((OE)_encoder, (opus_int16*)data, frame_samples, compressed_data, packet_sizes);
               data =(Byte*)data+frame_raw;
               size-=frame_raw;
            }
         }
      }
   }
#endif
   return ok;
}
Bool OpusEncoder::flush(Int &flushed_samples, MemPtr<Byte> compressed_data, MemPtr<Int> packet_sizes) // !! has to be called only once, because it always adds zero padding for delay !!
{
   Bool ok=false;
   compressed_data.clear();
   packet_sizes   .clear();
   flushed_samples=0;
#if SUPPORT_OPUS_ENC
   if(_encoder)
   {
      Int block=T.block();
      Zero((Byte*)_last_sample+_last_sample_size, block-_last_sample_size); // zero unwritten data
      // have to add delay data
   #if 0 // don't add zeros because it causes sudden jumps and pop/clicks in the last audio data
     _data.addNumZero(delay()*block); // as zero bytes
   #else // duplicate last encoded sample multiple times
      Int delay=T.delay();
      Byte *dest=&_data[_data.addNum(delay*block)];
      FREP(delay){CopyFast(dest, _last_sample, block); dest+=block;}
   #endif
      flushed_samples=_data.elms()/block; // must include delay, but not zero padding added for last frame below
      ok=true;
      for(Int pos=0, left=_data.elms(); left>0; )
      {
         Int frame_samples=frameSamples();
      #if 0 // do not attempt to reduce the frame size if we want to write less data, because it can drastically reduce the quality and cause pop/clicks, especially for looped audio
         Int left_samples=left/block;
         if( left_samples<frame_samples) // if we want to write less samples than encoder frame samples, this check is important, without it we could request a bigger frame size than encoder's, which could result in fewer frames than expected
         {
            Int smallest_frame_i=0; for(; left_samples>OpusValidFramesize[smallest_frame_i] && InRange(smallest_frame_i+1, OpusValidFramesize); smallest_frame_i++); // find smallest frame capable of storing all samples
            frame_samples=OpusValidFramesize[smallest_frame_i];
         }
      #endif
         Int frame_raw=frame_samples*block;
         if(left<frame_raw)_data.addNumZero(frame_raw-left); // if what we have is not enough to fit in frame then padd with zeros, here it's actually better to use zeroes than '_last_sample' because it will give more accurate results
         ok&=OpusEncode((OE)_encoder, (opus_int16*)&_data[pos], frame_samples, compressed_data, packet_sizes);
         left-=frame_raw;
         pos +=frame_raw;
      }
     _data.clear(); _last_sample_size=0;
   }
#endif
   return ok;
}
/******************************************************************************/
OpusDecoder::OpusDecoder() {_decoder=null; _channels=0;}
OpusDecoder& OpusDecoder::del()
{
#if SUPPORT_OPUS
   if(_decoder){opus_decoder_destroy((OD)_decoder); _decoder=null; _channels=0;}
#endif
   return T;
}
Bool OpusDecoder::create(Int channels, Int frequency)
{
#if SUPPORT_OPUS
   del();
   if(channels>=1 && channels<=2)
   if(_decoder=opus_decoder_create(OpusFrequency(frequency), channels, null))
   {
      T._channels=channels;
      return true;
   }
#endif
   return false;
}
void OpusDecoder::reset()
{
#if SUPPORT_OPUS
   if(_decoder)opus_decoder_ctl((OD)_decoder, OPUS_RESET_STATE);
#endif
}

Int OpusDecoder::frequency()C
{
   Int bit_rate=0;
#if SUPPORT_OPUS
   if(_decoder)opus_decoder_ctl((OD)_decoder, OPUS_GET_SAMPLE_RATE(&bit_rate));
#endif
   return bit_rate;
}

Bool OpusDecoder::decode(CPtr packet_data, Int packet_size, MemPtr<Byte> decompressed_data)
{
   decompressed_data.clear();
   if(packet_size<=0)return true;
#if SUPPORT_OPUS
   if(_decoder && packet_data)
   {
      const Int block=SIZE(I16)*_channels,
                max_frame_size=OPUS_MAX_FRAME_SAMPLES*block;
      if(decompressed_data.continuous())
      {
         decompressed_data.setNum(max_frame_size);
         Int decoded_samples=opus_decode((OD)_decoder, (Byte*)packet_data, packet_size, (opus_int16*)decompressed_data.data(), OPUS_MAX_FRAME_SAMPLES, 0); // keep 'decode_fec' as 0 because when set to 1 decoding may fail if it was not started from first packet
         if( decoded_samples>0)
         {
            decompressed_data.setNum(decoded_samples*block);
            return true;
         }
         decompressed_data.clear();
      }else
      if(_channels<=2)
      {
         Byte temp[OPUS_MAX_FRAME_SAMPLES*SIZE(I16)*2]; // *2 = 2 channels (stereo)
         Int decoded_samples=opus_decode((OD)_decoder, (Byte*)packet_data, packet_size, (opus_int16*)temp, OPUS_MAX_FRAME_SAMPLES, 0); // keep 'decode_fec' as 0 because when set to 1 decoding may fail if it was not started from first packet
         if( decoded_samples>0)
         {
            decompressed_data.setNum(decoded_samples*block).copyFrom(temp);
            return true;
         }
      }
   }
#endif
   return false;
}
Int OpusDecoder::decode(CPtr packet_data, Int packet_size, Ptr decompressed_data, Int decompressed_size)
{
   if(packet_size<=0)return 0; // nothing was decoded
#if SUPPORT_OPUS
   if(_decoder && packet_data)
   {
      const Int block=SIZE(I16)*_channels,
                max_frame_samples=decompressed_size/block;

      Int decoded_samples=opus_decode((OD)_decoder, (Byte*)packet_data, packet_size, (opus_int16*)decompressed_data, max_frame_samples, 0); // keep 'decode_fec' as 0 because when set to 1 decoding may fail if it was not started from first packet
      if( decoded_samples>0)return decoded_samples*block;
   }
#endif
   return -1; // error
}
/******************************************************************************/
// OGG OPUS ENCODER
/******************************************************************************/
struct _OggOpusEncoder
{
   ogg_stream_state os;
   ogg_page         og;
   ogg_packet       op;
   OpusEncoder      oe;
   File             *f;

   void      zero() {Zero(os); Zero(og); Zero(op); f=null;}
  _OggOpusEncoder() {zero();}
 ~_OggOpusEncoder() {del();}
   void              del()
   {
      ogg_stream_clear(&os);
      oe.del();
      zero();
   }
   Bool init(File &f, Int channels, Int bit_rate, Bool vbr)
   {
      del();
      if(oe.create(channels, false))
      {
         oe.bitRate(bit_rate).vbr(vbr);
         ogg_stream_init(&os, Random());

      #pragma pack(push, 1)
         struct OggHeader
         {
             Char8 magicSignature[8];
             Byte  version, channelCount;
             U16   preSkip;
             UInt  sampleRate;
             I16   outputGain;
             Byte  mappingFamily;
         }header=
         {
            {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'},
            1, // ver
            (Byte)channels,
            (U16 )oe.delay(), // pre skip
            OPUS_FREQUENCY, // frequency
            0, // output gain
            0, // channel mapping family
         };
         op.packet=(Byte*)&header;
         op.bytes =SIZE(header);
         op.b_o_s =true;
         ogg_stream_packetin(&os, &op);

         struct OggCommentHeader
         {
            Byte  magicSignature[8];
            U32   vendorStringLength;
          //Char8 vendorString[];
            U32   userCommentListLength;
          //comments..
         }comment=
         {
            {'O', 'p', 'u', 's', 'T', 'a', 'g', 's'},
            0,
            0,
         };
         op.packet=(Byte*)&comment;
         op.bytes =SIZE(comment);
         op.b_o_s =false;
         op.packetno++;
         ogg_stream_packetin(&os, &op);
      #pragma pack(pop)

         for(;;)
         {
            if(!ogg_stream_flush(&os, &og))break;
            f.put(og.header, og.header_len);
            f.put(og.  body, og.  body_len);
         }
         if(f.ok()){T.f=&f; return true;}
      }
   //error:
      del(); return false;
   }
   Bool encodeEx(CPtr data, Int size)
   {
      if(f)
      {
         Memt<Byte> compressed_data;
         Memt<Int > packet_sizes;
         Int        flushed_samples;
         if(data ? !oe.encode(data, size,      compressed_data, packet_sizes)
                 : !oe.flush (flushed_samples, compressed_data, packet_sizes))return false;
         Byte *save_data=compressed_data.data();
         if(!data && !packet_sizes.elms())packet_sizes.add(0); // add a dummy packet if flushing but there's no audio
         Int final_packet=(data ? -1 : packet_sizes.elms()-1); // index of the final packet in stream (only set when flushing)
         FREPA(packet_sizes)
         {
            Bool last=(i==final_packet);
            op.packet=save_data;
            op.bytes =packet_sizes[i]; save_data+=op.bytes;
            op.packetno++;
            if(last)
            {
                  op.e_o_s=true;
                  op.granulepos+=flushed_samples%oe.frameSamples(); // last packet had only few samples
            }else op.granulepos+=                oe.frameSamples();
            ogg_stream_packetin(&os, &op);
            for(;;)
            {
               if(last ? !ogg_stream_flush  (&os, &og)
                       : !ogg_stream_pageout(&os, &og))break;
               f->put(og.header, og.header_len);
               f->put(og.  body, og.  body_len);
            }
         }
         return true;
      }
      return false;
   }
   Bool encode(CPtr data, Int size)
   {
      for(Byte *d=(Byte*)data; size>0; ) // encode in chunks to avoid big memory allocation inside 'OpusEncoder' for encoded data
      {
         Int write=Min(size, 65536);
         if(!encodeEx(d, write))return false;
         d   +=write;
         size-=write;
      }
      return true;
   }
   Bool finish()
   {
      if(f)
      {
         Bool ok=encodeEx(null, 0); // encode end of stream, use 'encodeEx' because 'encode' will do nothing for 0 size
         f=null;
         return ok;
      }
      return true;
   }
};
/******************************************************************************/
OggOpusEncoder& OggOpusEncoder::del()
{
#if SUPPORT_OPUS_ENC
   if(_encoder)
   {
      ((_OggOpusEncoder*)_encoder)->finish();
      Delete((_OggOpusEncoder*&)_encoder);
   }
#endif
   return T;
}
Bool OggOpusEncoder::create(File &f, Int channels, Int bit_rate, Bool vbr)
{
#if SUPPORT_OPUS_ENC
   if(_encoder)
   {
      ((_OggOpusEncoder*)_encoder)->finish();
   }else
   {
      New((_OggOpusEncoder*&)_encoder);
   }
   if(((_OggOpusEncoder*)_encoder)->init(f, channels, bit_rate, vbr))return true;
   Delete((_OggOpusEncoder*&)_encoder);
#endif
   return false;
}
Bool OggOpusEncoder::encode(CPtr data, Int size)
{
#if SUPPORT_OPUS_ENC
   if(!size)return true;
   if(!data || size<=0 || !_encoder)return false;
   return ((_OggOpusEncoder*)_encoder)->encode(data, size);
#else
   return false;
#endif
}
Bool OggOpusEncoder::finish()
{
#if SUPPORT_OPUS_ENC
   return _encoder ? ((_OggOpusEncoder*)_encoder)->finish() : true;
#else
   return false;
#endif
}
/******************************************************************************/
// SND OPUS ENCODER
/******************************************************************************/
SndOpusEncoder::SndOpusEncoder() {zero();}
void            SndOpusEncoder::zero() {_f=null; _packet_sizes_pos=0; _packet_sizes_expected=0; _frequency=0;}
SndOpusEncoder& SndOpusEncoder::del () {finish(); return T;}
Bool SndOpusEncoder::create(File &f, Long samples, Int frequency, Int channels, Int bit_rate, Bool vbr)
{
   del();
   if(samples>=0 && frequency>=0 && _encoder.create(channels, false, frequency) && SaveSndHeader(f, SND_OPUS, channels, frequency, samples))
   {
     _f=&f;
     _frequency=frequency; bitRate(bit_rate); // set bit rate after frequency
     _encoder.vbr(vbr);
      Int delay=_encoder.delay(); f.cmpUIntV(delay);
      ULong frames=DivCeil(ULong(samples+delay), (ULong)frameSamples()); // calculate number of frames
     _packet_sizes_pos     =f.pos();
     _packet_sizes_expected=frames;
      f.put(null, SIZE(U16)*frames); // make room for frame sizes (can use U16 sizes, because max frame size is 4096)
      return f.ok();
   }
   return false;
}
void SndOpusEncoder::write(Memt<Byte> &compressed_data, Memt<Int> &packet_sizes)
{
   Byte *data=compressed_data.data();
   FREPA(packet_sizes)
   {
      Int size=packet_sizes[i];
     _packet_sizes.add(size);
     _f->put(data, size);
      data+=size;
   }
}
Bool SndOpusEncoder::encode(CPtr data, Int size)
{
   if(_f)
   {
      Memt<Byte> compressed_data;
      Memt<Int > packet_sizes;

      for(Byte *d=(Byte*)data; size>0; ) // encode in chunks to avoid big memory allocation inside 'OpusEncoder' for encoded data
      {
         Int s=Min(size, 65536);
         if(!_encoder.encode(d, s, compressed_data, packet_sizes)){del(); return false;}
         write(compressed_data, packet_sizes);
         d   +=s;
         size-=s;
      }
      return _f->ok();
   }
   return false;
}
Bool SndOpusEncoder::finish()
{
   Bool ok=false;
   if(_f)
   {
      Int        flushed_samples;
      Memt<Byte> compressed_data;
      Memt<Int > packet_sizes;
      ok=_encoder.flush(flushed_samples, compressed_data, packet_sizes);
      write(compressed_data, packet_sizes);
      Long pos=_f->pos();
      if(_f->pos(_packet_sizes_pos))
      {
         ok&=(_packet_sizes_expected==_packet_sizes.elms());
        _f->putN(_packet_sizes.data(), Min(_packet_sizes_expected, _packet_sizes.elms())); // limit to what was written and what was expected (leave remaining as zeros if any)
      }else ok=false;
      ok&=(_f->pos(pos) && _f->ok());
   }
  _encoder.del();
  _packet_sizes.clear();
   zero();
   return ok;
}
SndOpusEncoder& SndOpusEncoder::bitRate(Int bit_rate)
{
   if(frequency()>0 && bit_rate>=0)
   {
      bit_rate=DivRound(ULong(bit_rate)*_encoder.frequency(), (ULong)frequency()); // currently sounds are encoded without resampling, always at '_encoder.frequency()', so for lower frequencies we actually need to increase the bit-rate
     _encoder.bitRate(bit_rate);
   }
   return T;
} 
Int SndOpusEncoder::bitRate()C
{
   if(Int enc_freq=_encoder.frequency())return DivRound(ULong(_encoder.bitRate())*frequency(), (ULong)enc_freq);
   return 0;
}
/******************************************************************************/
// SND RAW ENCODER
/******************************************************************************/
SndRawEncoder::SndRawEncoder() {zero();}
void           SndRawEncoder::zero() {_f=null; _block=0; _samples_pos=-1;}
SndRawEncoder& SndRawEncoder::del () {finish(); return T;}
Bool SndRawEncoder::create(File &f, Int frequency, Int channels, Long samples)
{
   del();
   if(SaveSndHeader(f, SND_RAW_16, channels, frequency, (samples<0) ? -1 : samples)) // always use -1 for unknown to reserve max possible size for sample count
   {
      if(samples<0)_samples_pos=f.pos(); // if unknown then remember position to write later
     _block=channels*2; // 16-bit samples
     _f=&f;
      return f.ok();
   }
   return false;
}
Bool SndRawEncoder::encode(CPtr data, Int size)
{
   return _f ? _f->put(data, size) : false;
}
Bool SndRawEncoder::finish()
{
   Bool ok=false;
   if(_f)
   {
      ok=true;
      if(_samples_pos>=0) // need to write sample count
      {
         ok=false;
         Long pos=_f->pos(); if(_f->pos(_samples_pos-MaxCmpULongVSize))
         {
            ULong samples=(_block ? (pos-_samples_pos)/_block : 0);
           _f->cmpULongVMax(samples);
            ok=(_f->pos(pos) && _f->ok());
         }
      }
   }
   zero(); return ok;
}
/******************************************************************************/
// WAV ENCODER
/******************************************************************************/
WavEncoder::WavEncoder() {zero();}
void        WavEncoder::zero() {_f=null; _size_pos=-1;}
WavEncoder& WavEncoder::del () {finish(); return T;}
Bool WavEncoder::create(File &f, Int bits, Int frequency, Int channels, Long samples)
{
   del();
   if(SaveWavHeader(f, bits, channels, frequency, samples*channels*bits/8))
   {
      if(samples<0)_size_pos=f.pos(); // if unknown then remember position to write later
     _f=&f;
      return f.ok();
   }
   return false;
}
Bool WavEncoder::encode(CPtr data, Int size)
{
   return _f ? _f->put(data, size) : false;
}
Bool WavEncoder::finish()
{
   Bool ok=false;
   if(_f)
   {
      ok=true;
      if(_size_pos>=0) // need to write sample count
      {
         ok=false;
         Long pos=_f->pos(); if(_f->pos(_size_pos-44+4))
         {
            UInt size=pos-_size_pos;
           _f->putUInt(size+36);
            if(_f->pos(_size_pos-4))_f->putUInt(size);
            ok=(_f->pos(pos) && _f->ok());
         }
      }
   }
   zero(); return ok;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void InitStream()
{
#if SUPPORT_VORBIS
   VorbisCallbacks. read_func=VorbisRead;
   VorbisCallbacks. seek_func=VorbisSeek;
   VorbisCallbacks. tell_func=VorbisTell;
   VorbisCallbacks.close_func=VorbisClose;
#endif

#if SUPPORT_OPUS
   OpusCallbacks.read =OpusRead;
   OpusCallbacks.seek =OpusSeek;
   OpusCallbacks.tell =OpusTell;
   OpusCallbacks.close=OpusClose;
#endif
}
void ShutStream()
{
}
/******************************************************************************/
}
/******************************************************************************/
