/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "../../H/Misc/MP4.h"
/******************************************************************************

   Documentation available here:
      http://l.web.umkc.edu/lizhu/teaching/2016sp.video-communication/ref/mp4.pdf

/******************************************************************************/
enum
{
   #define COPYRIGHT_SYMBOL Char8(0xA9)

   CO64     =CC4('c','o','6','4'),
   CTTS     =CC4('c','t','t','s'),
   ESDS     =CC4('e','s','d','s'),
   FTYP     =CC4('f','t','y','p'),
   GENRE2   =CC4('g','n','r','e'),
   MDHD     =CC4('m','d','h','d'),
   MDIA     =CC4('m','d','i','a'),
   MINF     =CC4('m','i','n','f'),
   MOOF     =CC4('m','o','o','f'),
   MOOV     =CC4('m','o','o','v'),
   MP4A     =CC4('m','p','4','a'),
   MVHD     =CC4('m','v','h','d'),
   STBL     =CC4('s','t','b','l'),
   STCO     =CC4('s','t','c','o'),
   STSC     =CC4('s','t','s','c'),
   STSD     =CC4('s','t','s','d'),
   STSZ     =CC4('s','t','s','z'),
   STTS     =CC4('s','t','t','s'),
   STZ2     =CC4('s','t','z','2'),
   TFHD     =CC4('t','f','h','d'),
   TKHD     =CC4('t','k','h','d'),
   TRAF     =CC4('t','r','a','f'),
   TRAK     =CC4('t','r','a','k'),
   TRUN     =CC4('t','r','u','n'),
   USER_DATA=CC4('u','d','t','a'),
   TITLE    =CC4(COPYRIGHT_SYMBOL,'n','a','m'),
   ARTIST   =CC4(COPYRIGHT_SYMBOL,'A','R','T'),
   WRITER   =CC4(COPYRIGHT_SYMBOL,'w','r','t'),
   ALBUM    =CC4(COPYRIGHT_SYMBOL,'a','l','b'),
   DATE     =CC4(COPYRIGHT_SYMBOL,'d','a','y'),
   TOOL     =CC4(COPYRIGHT_SYMBOL,'t','o','o'),
   COMMENT  =CC4(COPYRIGHT_SYMBOL,'c','m','t'),
   GENRE1   =CC4(COPYRIGHT_SYMBOL,'g','e','n'),
};
/******************************************************************************/
void MP4::zero() {time_scale=0; duration=-1;}
MP4::MP4() {zero();}
/******************************************************************************/
Long MP4::Track::Part::dataSize()C
{
   if(frame_sizes.elms())
   {
      Long size=0; REPA(frame_sizes)size+=frame_sizes[i]; return size;
   }
   return ULong(frames)*frame_size;
}
Int MP4::Track::frames()C
{
   Int frames=0;
   if(stts.elms())REPA(stts )frames+=stts [i].count;
   else           REPA(parts)frames+=parts[i].frames;
   return frames;
}
UInt MP4::Track::frameRangeSize(Int start, Int end)C
{
   if(frame_size)return (end-start)*frame_size;
   MAX(start, 0);
   MIN(end  , frame_sizes.elms());
   UInt   size=0; for(; start<end; start++)size+=frame_sizes[start];
   return size;
}
Long MP4::Track::chunkOffset(Int i)C
{
   return InRange(i, chunk_offset) ? chunk_offset[i] : 0;
}
void MP4::Track::frameChunk(Int frame, Int &chunk, Int &chunk_frame)C
{
   Int chunk_i=1, frames=0; // start with 1 because MP4 has chunk indexes starting from 1
   if(stsc.elms())
   {
      UInt chunk_frames=stsc[0].frames_per_chunk;
      for(Int i=1; i<stsc.elms(); i++)
      {
       C STSC &next=stsc[i];
         Int chunks=next.first_chunk-chunk_i, end=frames+chunks*chunk_frames;
         if(frame<end)break;
         chunk_i     =next.first_chunk;
         chunk_frames=next.frames_per_chunk;
         frames      =end;
      }
      if(chunk_frames)
      {
         Int chunks=(frame-frames)/chunk_frames;
         chunk_i+=chunks;
         frames +=chunks*chunk_frames;
      }
   }
   chunk      =chunk_i-1; // convert from 1 to 0 as the first index
   chunk_frame=frames;
}
C MP4::Track::Part* MP4::Track::findPart(Int &frame)C
{
   FREPA(parts)
   {
    C Part &part=parts[i];
      if(InRange(frame, part.frames))return &part;
      frame-=part.frames;
   }
   return null;
}
Long MP4::Track::frameOffset(Int i)C
{
   if(stsc.elms())
   {
      Int chunk, chunk_frame; frameChunk(i, chunk, chunk_frame);
      return chunkOffset(chunk)+frameRangeSize(chunk_frame, i);
   }
   if(C Part *part=findPart(i))
   {
      Long pos=part->pos;
      if(part->frame_sizes.elms())for(; --i>=0; )pos+=part->frame_sizes[i];
      else                                       pos+=part->frame_size *i ;
      return pos;
   }
   return 0;
}
UInt MP4::Track::frameSize(Int i)C
{
   if(InRange(i, frame_sizes))return frame_sizes[i];
   if(C Part *part=findPart(i))return part->frameSize(i);
   return frame_size;
}
Long MP4::Track::dataSize()C
{
   Long size=0;
   if(frame_sizes.elms())
   {
      REPA(frame_sizes)size+=frame_sizes[i];
   }else
   if(parts.elms())
   {
      REPA(parts)size+=parts[i].dataSize();
   }else
      size=ULong(frames())*frame_size;
   return size;
}
MP4::Track* MP4::findTrack(Int id)
{
   REPA(tracks)if(tracks[i].id==id)return &tracks[i];
   return null;
}
/******************************************************************************/
static Bool ReadHeader(File &f, UInt &type, Long &size)
{
   struct Header
   {
      UInt size, type;
   }header;
   if(!f.getFast(header))return false;

   switch(header.size)
   {
      case 0x00000000: size=f.left()+SIZE(header); break; // BigEndian 0 = to the end of the file (including size of the header)
      case 0x01000000: size=f.getBELong()        ; break; // BigEndian 1 = 64-bit size
      default        : SwapEndian(header.size); size=header.size; break;
   }
   if(size<0)return false;
   type=header.type;
   return true;
}
static UInt ReadDescLength(File &f)
{
   Byte b;
   UInt i=0, length=0;
   do
   {
      b=f.getByte();
      length=(length<<7)|(b&0x7F);
   }while((b&0x80) && ++i<4);
   return length;
}
Bool MP4::read(File &f, Long max_pos, Track *track, Long moof_pos)
{
 //ft.depth++;
   UInt tfhd_default_frame_duration=0, tfhd_default_frame_size=0;
   Long tfhd_base_data_offset=moof_pos;
   Long      trun_data_offset=tfhd_base_data_offset;
   for(;;)
   {
      Long pos=f.pos(); if(pos>=max_pos)break; // reached the end for this parent
      UInt type; Long size; if(!ReadHeader(f, type, size))goto error;
      Long end=pos+size, data_pos=f.pos(), data_size=end-data_pos;
   #if DEBUG
      Char8 c[]={Char8(type&0xFF), Char8((type>>8)&0xFF), Char8((type>>16)&0xFF), Char8((type>>24)&0xFF)};
    //ft.startLine(); FREPA(c)ft.putChar(c[i]); ft.putText(S+" - "+data_pos+", "+data_size); ft.endLine();
   #endif
      switch(type)
      {
         // have children nodes
         case MOOV:
         case TRAK:
         case MDIA:
         case MINF:
         case STBL:
         case MOOF:
         case TRAF:
      #if 0 // not interested in
         case USER_DATA:
         case TITLE:
         case ARTIST:
         case WRITER:
         case ALBUM:
         case DATE:
         case TOOL:
         case COMMENT:
         case GENRE1:
         case GENRE2:
      #endif
         {
            if(!read(f, end, (type==TRAK) ? &tracks.New() : track, (type==MOOF) ? pos : moof_pos))goto error;
         }break;

         case MVHD:
         {
            switch(f.getByte()) // version
            {
               case 0:
               {
                  UInt flags=f.getBEUInt24();
                  UInt create_time=f.getBEUInt();
                  UInt modify_time=f.getBEUInt();
                  time_scale=f.getBEUInt();
                  duration  =f.getBEUInt(); if(duration==UINT_MAX)duration=-1;
               }break;
            }
         }break;

         case TKHD: if(track)
         {
            Byte ver  =f.getByte();
            UInt flags=f.getBEUInt24();
            switch(ver)
            {
               case 0:
               {
                  UInt create_time=f.getBEUInt();
                  UInt modify_time=f.getBEUInt();
                  track->id=f.getBEUInt();
                  f.getBEUInt();
                  track->duration=f.getBEUInt(); if(track->duration==UINT_MAX)track->duration=-1;
               }break;

               case 1:
               {
                  ULong create_time=f.getBEULong();
                  ULong modify_time=f.getBEULong();
                  track->id=f.getBEUInt();
                  f.getBEUInt();
                  track->duration=f.getBELong();
               }break;
            }
         }break;

         case MDHD: if(track)
         {
            switch(f.getBEUInt()) // version
            {
               case 0:
               {
                  UInt create_time=f.getBEUInt();
                  UInt modify_time=f.getBEUInt();
                  track->time_scale=f.getBEUInt();
                  track->duration  =f.getBEUInt(); if(track->duration==UINT_MAX)track->duration=-1;
               }break;

               case 1:
               {
                  ULong create_time=f.getBEULong();
                  ULong modify_time=f.getBEULong();
                  track->time_scale=f.getBEUInt();
                  track->duration  =f.getBELong();
               }break;
            }
         }break;

         case STSD: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags      =f.getBEUInt24();
               UInt entry_count=f.getBEUInt();
               FREP(entry_count)
               {
                  UInt   type; Long size, pos=f.pos(); if(!ReadHeader(f, type, size))break;
                  switch(type)
                  {
                     case MP4A:
                     {
                        f.skip(6); // reserved
                        U16 data_reference_index=f.getBEUShort();
                        f.getBEUInt();
                        f.getBEUInt();
                        track->channels=f.getBEUShort();
                        UInt sample_size=f.getBEUShort();
                        f.getBEUShort();
                        f.getBEUShort();
                        track->sample_rate=f.getBEUShort();
                        f.getBEUShort();

                        if(!ReadHeader(f, type, size))break;
                        if(type==ESDS)
                        {
                           Byte ver=f.getByte(); if(ver==0)
                           {
                              UInt flags=f.getBEUInt24();
                              Byte tag  =f.getByte();
                              if(tag==3)
                              {
                                if(ReadDescLength(f)<20)break;
                                f.skip(3);
                              }else f.skip(2);
                              if(f.getByte()!=4)break;
                              if(ReadDescLength(f)<13)break;
                              Byte audio_type=f.getByte();
                              f.getBEUInt();
                              UInt max_bitrate=f.getBEUInt();
                              track->avg_bit_rate=f.getBEUInt();
                              if(f.getByte()!=5)break;
                              UInt decoder_config_len=ReadDescLength(f);
                              Mems<Byte> &decoder_config=track->decoder_config;
                              decoder_config.setNum(decoder_config_len); if(!decoder_config.loadRawData(f)){decoder_config.clear(); break;}
                           }
                        }
                     }break;
                  }
                  f.pos(pos+size);
               }
            }
         }break;

         case STSZ: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags             =f.getBEUInt24();
                    track->frame_size =f.getBEUInt  ();
               Mems<UInt> &frame_sizes=track->frame_sizes;
               if(track->frame_size==0)
               {
                  frame_sizes.setNum(f.getBEUInt());
                  frame_sizes.loadRawData(f);
                  REPA(frame_sizes)SwapEndian(frame_sizes[i]);
               }else frame_sizes.clear();
            }
         }break;

         case STZ2:
         {
            
         }break;

         case STCO: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags=f.getBEUInt24();
               Mems<Long> &chunk_offset=track->chunk_offset.setNum(f.getBEUInt());
               FREPAO(chunk_offset)=f.getBEUInt();
            }
         }break;

         case CO64: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags=f.getBEUInt24();
               Mems<Long> &chunk_offset=track->chunk_offset.setNum(f.getBEUInt());
               chunk_offset.loadRawData(f); REPA(chunk_offset)SwapEndian(chunk_offset[i]);
            }
         }break;

         case STSC: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags=f.getBEUInt24();
               Mems<Track::STSC> &stsc=track->stsc.setNum(f.getBEUInt());
               stsc.loadRawData(f); ASSERT(SIZE(Track::STSC)==12);
               REPA(stsc){Track::STSC &s=stsc[i]; SwapEndian(s.first_chunk); SwapEndian(s.frames_per_chunk); SwapEndian(s.frame_desc_index);}
            }
         }break;

      /* struct CTTS
         {
            UInt frame_count, frame_offset;
         };
         Mems<CTTS> ctts;
         case CTTS: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags=f.getBEUInt24();
               Mems<Track::CTTS> &ctts=track->ctts.setNum(f.getBEUInt());
               ctts.loadRawData(f); ASSERT(SIZE(Track::CTTS)==8);
               REPA(ctts){Track::CTTS &s=ctts[i]; SwapEndian(s.frame_count); SwapEndian(s.frame_offset);}
            }
         }break; */

         case STTS: if(track)
         {
            Byte ver=f.getByte(); if(ver==0)
            {
               UInt flags=f.getBEUInt24();
               Mems<Track::STTS> &stts=track->stts.setNum(f.getBEUInt());
               stts.loadRawData(f); ASSERT(SIZE(Track::STTS)==8);
               REPA(stts){Track::STTS &s=stts[i]; SwapEndian(s.count); SwapEndian(s.delta);}
            }
         }break;

         // DASH
         case TFHD:
         {
            Byte ver  =f.getByte();
            UInt flags=f.getBEUInt24();
            switch(ver)
            {
               case 0:
               {
                  track=findTrack(f.getBEUInt()); // when encountering 'TFHD' then switch 'track' to the one from the header data in this level, so 'TRUN' can use it
                  trun_data_offset=tfhd_base_data_offset=((flags&0x000001) ? f.getBELong() : moof_pos);
                  if(flags&0x000002)UInt frame_description_index=f.getBEUInt();
                  tfhd_default_frame_duration=((flags&0x000008) ? f.getBEUInt() : 0);
                  tfhd_default_frame_size    =((flags&0x000010) ? f.getBEUInt() : 0);
                  if(flags&0x000020)UInt default_frame_flags=f.getBEUInt();
               }break;
            }
         }break;

         case TRUN: if(track) // 'track' should be available because this chunk is processed in the same level and after 'TFHD' which finds the track for us
         {
            Byte ver  =f.getByte();
            UInt flags=f.getBEUInt24();
            switch(ver)
            {
               case 0:
               {
                  if(UInt frame_count=f.getBEUInt())
                  {
                     if(flags&0x000001)trun_data_offset=tfhd_base_data_offset+f.getBEInt(); // if not present, then we have to reuse the last value
                     if(flags&0x000004)UInt first_frame_flags=f.getBEUInt();
                     Track::Part &part=track->parts.New();
                     part.pos           =trun_data_offset;
                     part.frames        =frame_count;
                     part.frame_duration=tfhd_default_frame_duration;
                     part.frame_size    =tfhd_default_frame_size    ;
                     if(flags&0x000100)part.frame_durations.setNum(frame_count);
                     if(flags&0x000200)part.frame_sizes    .setNum(frame_count);
                     FREP(frame_count)
                     {
                        UInt frame_duration=tfhd_default_frame_duration; if(flags&0x000100)part.frame_durations[i]=frame_duration=f.getBEUInt();
                        UInt frame_size    =tfhd_default_frame_size    ; if(flags&0x000200)part.frame_sizes    [i]=frame_size    =f.getBEUInt();
                        if(flags&0x000400)UInt frame_flags=f.getBEUInt();
                        if(flags&0x000800)UInt frame_composition_time_offset=f.getBEUInt();
                        trun_data_offset+=frame_size;
                     }
                  }
               }break;
            }
         }break;
      }
      if( f.pos()>end            // if we're already after end
      || !f.pos(end))goto error; // or failed to skip
   }
 //ft.depth--;
   return true;
error:
 //ft.depth--;
   return false;
}
Bool MP4::read(File &f)
{
   tracks.clear();
   zero();
   return read(f, f.size(), null, 0);
}
/******************************************************************************/
}
/******************************************************************************/
