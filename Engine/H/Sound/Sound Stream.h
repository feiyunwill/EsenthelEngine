/******************************************************************************

   Use 'SoundCallback' to play dynamically generated sound data.

   Use 'SoundStream' to manually decode audio streams.

/******************************************************************************/
enum SOUND_CODEC : Byte
{
   SOUND_NONE       , // none
   SOUND_WAV        , // Waveform
   SOUND_FLAC       , // Free Lossless Audio Codec
   SOUND_OGG_VORBIS , // Ogg  Vorbis
   SOUND_OGG_OPUS   , // Ogg  Opus
   SOUND_WEBM_VORBIS, // WebM Vorbis
   SOUND_WEBM_OPUS  , // WebM Opus
   SOUND_SND_VORBIS , // Esenthel Sound Vorbis
   SOUND_SND_OPUS   , // Esenthel Sound Opus
   SOUND_MP3        , // MPEG Layer III
   SOUND_AAC        , // Advanced Audio Coding
   SOUND_DYNAMIC    , // data is provided dynamically from user provided 'SoundCallback'
   SOUND_NUM        , // number of sound codecs
};
CChar8* CodecName(SOUND_CODEC codec); // get codec name
#if EE_PRIVATE
enum SND_CODEC : Byte
{
   SND_RAW_16, // Raw 16-bit
   SND_VORBIS,
   SND_OPUS  ,
};
#endif
/******************************************************************************/
const_mem_addr struct SoundCallback // Sound Callback used to dynamically create sound data !! must be stored in constant memory address !! this object is passed on to functions which store pointer to it, therefore it must be stored in a constant memory address
{
   virtual Bool create(Byte &bits, Byte &channels, Int &frequency, Long &size, Int &bit_rate)=NULL; // 'create' will be called upon creation of the sound, you should override this method and inside it fill all of the parameters, 'bits'=number of bits per sample (use 8 for 8-bit samples and 16 for 16-bit samples), 'channels'=number of channels (valid values are '1'=mono, '2'=stereo), 'frequency'=number of samples per second (valid values are 1..192000, recommended value is 44100 or 48000), 'size'=total size of the uncompressed sound data in bytes (use -1 if the size is unknown), 'bit_rate'=this parameter is optional, it specifies the average number of compressed data bytes needed to fill a 1 second of uncompressed data (use -1 if unknown), return false on fail, warning: this may get called on secondary thread
   virtual Bool raw   (Long  raw           ) {return true;} // 'raw' will be called when the sound is being requested to jump to specified position, the position is specified in raw bytes of uncompressed data, return false on fail, warning: this may get called on secondary thread
   virtual Int  set   (Ptr   data, Int size)=NULL;          // 'set' will be called when the sound is being requested to fill the 'data' buffer with raw uncompressed sound data of 'size' length, you should not write more data than requested 'size', return the number of bytes written or -1 on fail, warning: this may get called on secondary thread
   virtual void del   (                    ) {}             // 'del' will be called when the sound is being destroyed, and this callback will no longer be accessed by it, warning: this may get called on secondary thread
};
/******************************************************************************/
const_mem_addr struct SoundStream // can be moved however 'memAddressChanged' needs to be called afterwards
{
   // get
   Int         block    ()C {return _par.block;} // bytes*channels
   Int         bytes    ()C {return _par.bytes;}
   Int         bits     ()C {return _par.bits();}
   Int         channels ()C {return _par.channels;}
   Int         frequency()C {return _par.frequency;}
   Int         bitRate  ()C {return _par.bit_rate;}
   Long        size     ()C {return _par.size;}
   Long        samples  ()C {return _par.samples();}
   Flt         length   ()C {return _par.length();}
   SOUND_CODEC codec    ()C {return _codec;}
   CChar8*     codecName()C {return  CodecName(_codec);} // get codec name

   // manage
   void del   (                                      ); // delete manually
   Bool create(             C Str           &name    ); // create from file
   Bool create(             C UID           &id      ); // create from file
   Bool create(const_mem_addr SoundCallback &callback); // create from sound callback

                                        Bool sample(Long sample); //     set stream position, false on fail
   Long pos ()C {return        _pos;}   Bool pos   (Long pos   ); // get/set stream position, false on fail
   Long left()C {return size()-_pos;}                             // get     bytes left to process in the stream

   Int set(Ptr data, Int size); // decode stream into 'data' buffer up to 'size' number of bytes, this method returns the actual number of bytes written (can be less than 'size', or <=0 on error)

   void fastSeek(); // if allow fast but not precise seeking, default=disabled
   void memAddressChanged(); // call this if 'SoundStream' object was moved to another memory address

   SoundStream();
  ~SoundStream() {del();}

#if !EE_PRIVATE
private:
#endif
   struct Params // parameters
   {
      Byte bytes, channels, block;
      Int  frequency, bit_rate;
      Long size;

      void zero() {bytes=channels=block=0; frequency=bit_rate=0; size=0;}

      Int  bits   ()C {return bytes*8;}
      Long samples()C {if(Int b=block)return size/b; return 0;}
      Flt  length ()C; // get length in seconds
   };

   SOUND_CODEC    _codec;
   union
   {
      UInt        _raw_ofs;
      Ptr         _extra;
   };
   Long           _pos;
   File           _f;
   Params         _par;
   SoundCallback *_callback;

#if EE_PRIVATE
   Bool open(C Str &name);
#endif
   NO_COPY_CONSTRUCTOR(SoundStream);
};
/******************************************************************************/
struct SoundHeader
{
   SOUND_CODEC codec;
   Byte        bytes, bits, channels;
   Int         frequency, bit_rate;
   Long        size;
   Flt         length;

   CChar8* codecName()C; // get codec name

   // io
   Bool load(C Str &name);

   void           zero();
   SoundHeader() {zero();}
};
/******************************************************************************/
Bool SaveWavHeader(File &f, Int bits, Int channels, Int frequency, UInt size); // 'bits'=number of bits per sample (use 8 for 8-bit samples or 16 for 16-bit samples), 'channels'=number of channels (use 1 for mono and 2 for stereo), 'frequency'=sample rate, 'size'=size of audio data in bytes, after writing this header to a file you can store the raw audio data
/******************************************************************************/
#if EE_PRIVATE
Bool SaveSndHeader(File &f, SND_CODEC codec, Int channels, Int frequency, Long samples); // 'channels'=number of channels (use 1 for mono and 2 for stereo), 'frequency'=sample rate, 'samples'=number of audio samples

void InitStream();
void ShutStream();
#endif
/******************************************************************************/
