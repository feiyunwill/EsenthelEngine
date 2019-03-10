/******************************************************************************/
struct MP4
{
   struct Track
   {
      struct STTS
      {
         UInt count, delta;
      };
      struct STSC
      {
         UInt first_chunk, frames_per_chunk, frame_desc_index;
      };
      struct Part
      {
         Long pos;
         UInt frames;
         UInt frame_duration; Mems<UInt> frame_durations;
         UInt frame_size    ; Mems<UInt> frame_sizes    ;

         UInt frameDuration(Int i)C {return InRange(i, frame_durations) ? frame_durations[i] : frame_duration;}
         UInt frameSize    (Int i)C {return InRange(i, frame_sizes    ) ? frame_sizes    [i] : frame_size    ;}

         Long dataSize()C;
      };

      Int        id;
      UInt       frame_size, channels, sample_rate, time_scale, avg_bit_rate;
      Long       duration;
      Mems<UInt> frame_sizes;
      Mems<Long> chunk_offset;
      Mems<Byte> decoder_config;
      Mems<STTS> stts;
      Mems<STSC> stsc;
      Mems<Part> parts;

       Int frames()C;
      UInt frameRangeSize(Int start, Int end)C;
      void frameChunk (Int frame, Int &chunk, Int &chunk_frame)C;
      Long chunkOffset(Int i)C;
      Long frameOffset(Int i)C;
      UInt frameSize  (Int i)C;
      Long  dataSize  ()C;
    C Part* findPart  (Int &frame)C;

      Track() {id=-1; frame_size=channels=sample_rate=time_scale=avg_bit_rate=0; duration=-1;}
   };

   UInt        time_scale;
   Long        duration;
   Memc<Track> tracks;

#if EE_PRIVATE
   Track* findTrack(Int id);
   void zero();
   Bool read(File &f, Long max_pos, Track *track, Long moof_pos);
#endif
   Bool read(File &f);

   MP4();
};
/******************************************************************************/
