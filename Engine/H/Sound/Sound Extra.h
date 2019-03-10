/******************************************************************************

   Use 'AppVolume' to set the total application volume in the operating system.

   Use 'SoundRecord' to record sound data.

/******************************************************************************/
struct AppVolumeClass // Application Volume (in the system) Control
{
#if EE_PRIVATE
   void del   ();
   Bool create();
   void muteEx(Bool mute);   Bool muteEx()C;   Bool muteFinal()C;
#endif

   // get / set
   void volume(Flt  volume);   Flt  volume(); // set/get volume (0..1)
   void mute  (Bool mute  );   Bool mute  (); // set/get mute   (true/false)

#if !EE_PRIVATE
private:
#endif
   AppVolumeClass();
}extern
   AppVolume;
/******************************************************************************/
const_mem_addr struct SoundRecord // !! must be stored in constant memory address !!
{
   struct Device
   {
      Str name;

      Device() {_id.zero();}

   #if !EE_PRIVATE
   private:
   #endif
      UID _id;
   };

   static void GetDevices(MemPtr<Device> devices); // get list of sound record devices in the system

   // manage
   void del   (); // manually delete
   Bool create(Device *device=null, Int bits=16, Int channels=1, Int frequency=44100); // start capturing using specified 'device' (use null for default), 'bits'=number of bits per sample (8 or 16), 'channels'=number of channels (1 or 2), 'frequency'=recording frequency, false on fail

   // get
   Bool is()C {return _handle!=null;} // if sound record is currently created

   // operations
   void update(); // update the sound record to get the most recent data, normally you don't need to call this as the buffer will get automatically updated over time, however you may want to call this if you need to make sure that all latest data was processed (if you want immediate access - lowest latency)

   // callbacks
   virtual void receivedData(CPtr data, Int size) {} // this method will be called upon receiving audio data, you should override this method to be able to process it, warning: this may get called on secondary thread

#if EE_PRIVATE
   Int  curPosNoLock()C;
   void updateNoLock();
#endif

  ~SoundRecord() {del();}
   SoundRecord();

#if !EE_PRIVATE
private:
#endif
#if WINDOWS_OLD
#if EE_PRIVATE
   #if DIRECT_SOUND
      IDirectSoundCapture8      *_handle;
      IDirectSoundCaptureBuffer *_dscb;
   #elif OPEN_AL
      ALCdevice                 *_handle;
      UIntPtr                    _block;
   #else
      Ptr _handle, _dscb;
   #endif
#else
   Ptr  _handle, _dscb;
#endif
   Int  _pos, _size;
#elif WINDOWS_NEW
#if EE_PRIVATE
  _SoundRecord *_handle;
#else
   Ptr  _handle;
#endif
#elif APPLE
#if EE_PRIVATE
   AudioComponentInstance _handle; ASSERT(SIZE(AudioComponentInstance)==SIZE(Ptr));
#else
   Ptr  _handle;
#endif
   UInt _flags;
#elif ANDROID
   Ptr  _handle;
#else
#if EE_PRIVATE
   #if OPEN_AL
      ALCdevice *_handle;
      Byte       _block;
   #else
      Ptr  _handle;
      Byte _block;
   #endif
#else
   Ptr  _handle;
   Byte _block;
#endif
#endif

   NO_COPY_CONSTRUCTOR(SoundRecord);
};
/******************************************************************************/
#define OPUS_FREQUENCY            48000 // Opus Audio highest supported frequency 48kHz
#define OPUS_BYTES                    2 // Opus Audio is always stored using  2-byte samples
#define OPUS_BITS                    16 // Opus Audio is always stored using 16-bit  samples
#define OPUS_FRAME_SIZE_BEST_QUALITY 20 // Opus Audio frame size that provides the best quality, according to the documentation https://wiki.xiph.org/OpusFAQ#What_frame_size_should_I_use.3F - For file encoding, using a frame size larger than 20 ms will usually result in worse quality for the same bitrate because it constrains the encoder in the decisions it can make.
/******************************************************************************/
struct OpusEncoder
{
   // manage
   Bool create(Int channels, Bool voice, Int frequency=OPUS_FREQUENCY); // create encoder, 'channels'=number of channels (1 or 2), 'voice'=if optimize encoder for Voice Over Internet (otherwise universal encoder is chosen, which is preferred for sounds and music), 'frequency'=only following values are supported: 48000, 24000, 16000, 12000, 8000 (if another value is provided, it will be rounded to the nearest supported value), false on fail

   // get / set
   Int  frequency   ()C;                                            // get     frequency
   Int  bytes       ()C {return  OPUS_BYTES        ;}               // get     bytes per sample
   Int  bits        ()C {return  OPUS_BITS         ;}               // get     bits  per sample
   Int  channels    ()C {return _channels          ;}               // get     number of channels
   Int  block       ()C {return  bytes()*channels();}               // get     block size (bytes*channels)
   Int  frameSamples()C {return _frame_samples     ;}               // get     number of samples per frame
   Int  frameLength ()C;   OpusEncoder& frameLength(Int  length  ); // get/set frame length in milliseconds, smaller values result in worse quality but smaller latency(delay), higher values result in better quality but higher latency(delay) (2 .. 60 range supported), however Opus provides best quality for OPUS_FRAME_SIZE_BEST_QUALITY
   Int  bitRate     ()C;   OpusEncoder& bitRate    (Int  bit_rate); // get/set encoding bit-rate in bits per second (500 .. 512,000 range supported)
   Bool vbr         ()C;   OpusEncoder& vbr        (Bool on      ); // get/set variable bit-rate (VBR), default=true
   Flt  complexity  ()C;   OpusEncoder& complexity (Flt  complex ); // get/set computational complexity (0..1, 0=fastest/low quality, 1=slowest/high quality)
#if EE_PRIVATE
   void reset       ();
   Int  delay       ()C;

   Bool flush(Int &flushed_samples, MemPtr<Byte> compressed_data, MemPtr<Int> packet_sizes); // flush any remaining buffered data, 'flushed_samples'=number of samples in the last packet that were originally set in 'encode' method, returns false on fail
#endif

   // encode
   Bool encode(CPtr data, Int size, MemPtr<Byte> compressed_data, MemPtr<Int> packet_sizes); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, 'compressed_data'=this will contain compressed data of all packets in sequential order (data of first packet will be stored first, followed by second, third etc.), 'packet_sizes'=sizes of individual packets (in bytes), returns false on fail. For example if 'packet_sizes' has 2 elements, then 2 packets were created, 'packet_sizes[0]' specifies size of first packet, and 'packet_sizes[1]' specifies size of second packet, size of 'compressed_data' will be "packet_sizes[0]+packet_sizes[1]", it will contain data of first packet, followed by second packet. This method should be called continuously, once encoder gathers enough data it will generate compressed packets, in other cases when there's not yet enough data gathered, no packets will be generated.

   OpusEncoder&   del();
  ~OpusEncoder() {del();}
   OpusEncoder();

private:
   Ptr        _encoder;
   Int        _channels, _frame_samples, _last_sample_size;
   I16        _last_sample[2];
   Memc<Byte> _data;

   NO_COPY_CONSTRUCTOR(OpusEncoder);
};
/******************************************************************************/
struct OpusDecoder
{
   // manage
   Bool create(Int channels, Int frequency=OPUS_FREQUENCY); // create decoder, 'channels'=number of channels (1 or 2), 'frequency'=only following values are supported: 48000, 24000, 16000, 12000, 8000 (if another value is provided, it will be rounded to the nearest supported value), false on fail
   void reset(); // reset state, call when seeking or wanting to decode a different stream

   // get
   Int frequency()C;                      // get frequency
   Int bytes    ()C {return  OPUS_BYTES;} // get bytes per sample
   Int bits     ()C {return  OPUS_BITS ;} // get bits  per sample
   Int channels ()C {return _channels  ;} // get number of channels

   // decode
   Bool decode(CPtr packet_data, Int packet_size, MemPtr<Byte> decompressed_data              ); // decode encoded 'packet_data' of 'packet_size' into 'decompressed_data', 'decompressed_data'=will hold decompressed data of 16-bit samples, returns false on fail
   Int  decode(CPtr packet_data, Int packet_size, Ptr decompressed_data, Int decompressed_size); // decode encoded 'packet_data' of 'packet_size' into 'decompressed_data', 'decompressed_data'=will hold decompressed data of 16-bit samples, 'decompressed_size'=size of 'decompressed_data', returns actual size (in bytes) of decoded data, or -1 if error occurred

   OpusDecoder&   del();
  ~OpusDecoder() {del();}
   OpusDecoder();

private:
   Ptr _decoder;
   Int _channels;

   NO_COPY_CONSTRUCTOR(OpusDecoder);
};
/******************************************************************************/
struct SndRawEncoder // Raw Encoder into Esenthel SND file format
{
   Bool create(File &f, Int frequency, Int channels, Long samples=-1); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'frequency'=sample rate per second, 'channels'=number of channels (1=mono, 2=stereo), 'samples'=number of samples in audio (if you don't know it yet, then set -1), returns false on fail
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   SndRawEncoder&   del();
  ~SndRawEncoder() {del();}
   SndRawEncoder();
#if EE_PRIVATE
   void zero();
#endif

private:
   File *_f;
   Byte  _block;
   Long  _samples_pos;

   NO_COPY_CONSTRUCTOR(SndRawEncoder);
};
/******************************************************************************/
struct WavEncoder // Raw Encoder into WAV file format
{
   Bool create(File &f, Int bits, Int frequency, Int channels, Long samples=-1); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'bits'=number of bits per sample (use 8 for 8-bit samples or 16 for 16-bit samples), 'frequency'=sample rate per second, 'channels'=number of channels (1=mono, 2=stereo), 'samples'=number of samples in audio (if you don't know it yet, then set -1), returns false on fail
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   WavEncoder&   del();
  ~WavEncoder() {del();}
   WavEncoder();
#if EE_PRIVATE
   void zero();
#endif

private:
   File *_f;
   Long  _size_pos;

   NO_COPY_CONSTRUCTOR(WavEncoder);
};
/******************************************************************************/
#if EE_PRIVATE
struct SndVorbisEncoder // Vorbis Encoder into Esenthel SND file format
{
   Bool create(File &f, Long samples, Int frequency=44100, Int channels=2, Flt quality=0.2f); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'samples'=number of samples in audio, 'frequency'=sample rate per second, 'channels'=number of channels (1=mono, 2=stereo), 'quality'=sound quality (-0.1 .. 1.0), for 44100Hz stereo, following quality results in Kbit/s bit rate: -0.1->45, 0.0->64, 0.1->80, 0.2->96, 0.3->112, 0.4->128, 0.5->160, 0.6->192, 0.7->224, 0.8->256, 0.9->320, 1.0->500, returns false on fail
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   SndVorbisEncoder&   del();
  ~SndVorbisEncoder() {del();}
   SndVorbisEncoder() {_encoder=null;}

private:
   Ptr _encoder;

   NO_COPY_CONSTRUCTOR(SndVorbisEncoder);
};
#endif
/******************************************************************************/
struct OggVorbisEncoder // Vorbis Encoder into OGG file format
{
   Bool create(File &f, Int frequency=44100, Int channels=2, Flt quality=0.2f); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'frequency'=sample rate per second, 'channels'=number of channels (1=mono, 2=stereo), 'quality'=sound quality (-0.1 .. 1.0), for 44100Hz stereo, following quality results in Kbit/s bit rate: -0.1->45, 0.0->64, 0.1->80, 0.2->96, 0.3->112, 0.4->128, 0.5->160, 0.6->192, 0.7->224, 0.8->256, 0.9->320, 1.0->500, returns false on fail
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   OggVorbisEncoder&   del();
  ~OggVorbisEncoder() {del();}
   OggVorbisEncoder() {_encoder=null;}

private:
   Ptr _encoder;

   NO_COPY_CONSTRUCTOR(OggVorbisEncoder);
};
/******************************************************************************/
struct SndOpusEncoder // Opus Encoder into Esenthel SND file format
{
   Bool create(File &f, Long samples, Int frequency, Int channels, Int bit_rate, Bool vbr=true); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'samples'=number of samples in audio, 'frequency'=sample rate per second, 'channels'=number of channels (1=mono, 2=stereo), 'bit_rate'=encoding bit-rate in bits per second (500 .. 512,000 range supported), 'vbr'=variable bit-rate, returns false on fail
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   Int frequency   ()C;                                          // get     frequency
   Int bytes       ()C;                                          // get     bytes per sample
   Int bits        ()C;                                          // get     bits  per sample
   Int channels    ()C;                                          // get     number of channels
   Int block       ()C;                                          // get     block size (bytes*channels)
   Int frameSamples()C;                                          // get     number of samples per frame
   Int bitRate     ()C;   SndOpusEncoder& bitRate(Int bit_rate); // get/set encoding bit-rate in bits per second (500 .. 512,000 range supported)

   SndOpusEncoder&   del();
  ~SndOpusEncoder() {del();}
   SndOpusEncoder();
#if EE_PRIVATE
   void zero();
   void write(Memt<Byte> &compressed_data, Memt<Int> &packet_sizes);
#endif

private:
   OpusEncoder _encoder;
   File       *_f;
   Memc<U16>   _packet_sizes;
   Long        _packet_sizes_pos;
   UInt        _packet_sizes_expected;
   Int         _frequency;

   NO_COPY_CONSTRUCTOR(SndOpusEncoder);
};
/******************************************************************************/
struct OggOpusEncoder // Opus Encoder into OGG file format
{
   Bool create(File &f, Int channels, Int bit_rate, Bool vbr=true); // initialize the encoder, 'f'=file to write to (it must be already opened for writing), 'channels'=number of channels (1=mono, 2=stereo), 'bit_rate'=encoding bit-rate in bits per second (500 .. 512,000 range supported), 'vbr'=variable bit-rate, returns false on fail. There is no option to specify custom frequency, as Opus inside Ogg is always encoded at 48kHZ (therefore input to this class must be already in 48kHz)
   Bool encode(CPtr data, Int size); // feed the encoder with additional portion of data, 'data'=array of 16-bit samples (this should contain data for all channels specified in 'create' method, stereo should have per-channel data interleaved), 'size'=total raw length of 'data' in bytes, returns false on fail
   Bool finish(); // you can optionally call this once after entire sound data has been passed to 'encode' calls to verify that last portion of data was compressed successfully, you don't need to call this as it is always called in 'del' method, returns false on fail

   OggOpusEncoder&   del();
  ~OggOpusEncoder() {del();}
   OggOpusEncoder() {_encoder=null;}

private:
   Ptr _encoder;

   NO_COPY_CONSTRUCTOR(OggOpusEncoder);
};
/******************************************************************************/
struct SoundDataCallback
{
   virtual void data(Ptr data, Int size, C SoundStream &stream, Long raw_pos)=NULL;
};
struct Spectrometer : SoundDataCallback
{
   Spectrometer& del   ();
   Spectrometer& create(Int resolution);

   void get(Flt *meter, Int meter_elms, Flt time, FILTER_TYPE filter=FILTER_LINEAR); // set 'meter' of 'meter_elms' (recommended size is 'Spectrometer.resolution') to spectrometer reading at 'time' sound time, 'filter'=filtering (only FILTER_NONE, FILTER_LINEAR and FILTER_CUBIC are supported)
   // 'meter' array will have intensities for ranges from 1 Hz to SoundFrequency/2 Hz
   // which means that for 44100 Hz Sound, last 'meter' element will have 22050 Hz intensity
   //              and for 48000 Hz Sound, last 'meter' element will have 24000 Hz intensity
   //              and for 96000 Hz Sound, last 'meter' element will have 48000 Hz intensity

   Int resolution()C {return _image.w();} // get spectrometer resolution

  ~Spectrometer() {del();}
   Spectrometer();
   Spectrometer(Int resolution);

   virtual void data(Ptr data, Int size, C SoundStream &stream, Long raw_pos)override;

private:
   Int       _frequency;
   Flt       _window_length;
   Ptr       _fft;
   Image     _image;
   Memc<Flt> _samples, _fft_out;

#if EE_PRIVATE
   void free();
   void zero();
#endif
   NO_COPY_CONSTRUCTOR(Spectrometer);
};
/******************************************************************************/
#if EE_PRIVATE
extern Memc<SoundRecord*> SoundRecords;
#endif
/******************************************************************************/
