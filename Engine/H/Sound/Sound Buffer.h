/******************************************************************************/
#if EE_PRIVATE
/******************************************************************************/
enum VIRTUALIZATION_MODE // 2D Sound Virtualization Mode
{
   VIRT_NONE, // none
   VIRT_LOW , // low
   VIRT_HIGH, // high
};
/******************************************************************************/
const_mem_addr struct SoundBuffer // can be moved however 'memAddressChanged' needs to be called afterwards
{
   // manage
   void del   ();
   Bool create(Int frequency, Int bits, Int channels, Int samples, Bool is3D=false);

   // operations
   void memAddressChanged(); // !! can't be called when the SoundBuffer is playing/paused !!
#if OPEN_SL
   Bool emulate3D();
#endif
#if DIRECT_SOUND
   Bool   lock(Int pos=0, Int size=-1);
   void unlock(                      );
#endif

   // get / set
                                       Bool is   ()C;
                                       Bool is3D ()C;
   void raw      (  Int  raw      );   Int  raw  ()C;
   void pan      (  Flt  pan      );
   void volume   (  Flt  volume   );
   void frequency(  Int  frequency);
   void speed    (  Flt  speed    );
   void range    (  Flt  range    );   Flt  range()C;
   void pos      (C Vec &pos      );   Vec  pos  ()C;
   void vel      (C Vec &vel      );   Vec  vel  ()C;

   void setParams(C _Sound &sound, Bool pos_range, Bool doppler);

   // stop / play
   void stop   ();
   void pause  ();
   void toggle (Bool loop);
   Bool playing()C;
   void play   (Bool loop);

#if EE_PRIVATE
   void zero();
#endif
  ~SoundBuffer() {del();}
   SoundBuffer();

#if !EE_PRIVATE
private:
#endif
   SoundStream::Params _par;
#if DIRECT_SOUND
   Ptr                  _lock_data;
   UInt                 _lock_size;
   IDirectSoundBuffer   *_s;
   IDirectSound3DBuffer *_s3d;
#elif XAUDIO
   IXAudio2SourceVoice *_sv;
   Mems<Byte>           _data;
   Bool                 _3d;
#elif OPEN_AL
   Bool _3d;
   UInt _buffer[2], _source;
#elif OPEN_SL
   Bool                         _3d;
   Int                          _processed;
   Flt                          _volume, _range;
   Vec                          _pos;
   SLObjectItf                   player_object;
   SLPlayItf                     player_play;
   SLVolumeItf                   player_volume;
   SLPlaybackRateItf             player_playback_rate;
   SLAndroidSimpleBufferQueueItf player_buffer_queue;
   SL3DLocationItf               player_location;
   SL3DDopplerItf                player_doppler;
   SL3DSourceItf                 player_source;
   Mems<Byte>                   _data;
#endif

   NO_COPY_CONSTRUCTOR(SoundBuffer);
};
/******************************************************************************/
#if XAUDIO
extern IXAudio2 *XAudio;
#endif
/******************************************************************************/
#endif
/******************************************************************************/
