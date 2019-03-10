/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   All 'SoundBuffer' methods (and thus Sound API) are called only by Sound Threads,
      they manage the Locking, so we don't need to do any locks here,
      therefore SOUND_API_LOCK_WEAK is marked as an empty macro.

/******************************************************************************/
#define SOUND_API_LOCK_FORCE     SyncLocker locker(SoundAPILock);
#define SOUND_API_LOCK_WEAK      //SOUND_API_LOCK_FORCE not needed
#define OPERATION_SET            (!XAUDIO2_COMMIT_NOW) // make sure that this is not XAUDIO2_COMMIT_NOW
#define DIRECT_SOUND_RANGE_SCALE 1.75f // this was tested by playing sound at different positions (1,0,0), (2,0,0), (4,0,0) with range 1 and comparing recorded volume to XAudio version
#define      OPEN_AL_RANGE_SCALE 0.75f // this was tested by playing sound at different positions (1,0,0), (2,0,0), (4,0,0) with range 1 and comparing recorded volume to XAudio version

#if DIRECT_SOUND
static IDirectSound           *DS;
static IDirectSound3DListener *DSL;

static VIRTUALIZATION_MODE Virtualization=VIRT_HIGH;
#elif XAUDIO
       IXAudio2               *XAudio;
static IXAudio2MasteringVoice *XAudioMasteringVoice;
static X3DAUDIO_HANDLE         X3DAudio;
static X3DAUDIO_LISTENER       X3DListener;
static Int                     XAudioChannels;
#elif OPEN_AL
static ALCcontext *ALContext;
static ALCdevice  *ALDevice;
#elif OPEN_SL
static SLObjectItf              SLEngineObject;
static SLEngineItf              SLEngineEngine;
static SLObjectItf              SLOutputMixObject;
static SLEnvironmentalReverbItf SLOutputMixEnvironmentalReverb;
static SLObjectItf              SLListenerObject;
static SL3DLocationItf          SLListenerLocation;
static SL3DDopplerItf           SLListenerDoppler;
static SL3DCommitItf            SLListenerCommit;

#define SL_POS_UNIT 0.001f // milimeter
#endif

Bool          SoundAPI, SoundFunc;
ListenerClass Listener;
/******************************************************************************/
void SoundBuffer::zero()
{
  _par.zero();
#if DIRECT_SOUND
  _lock_data=null;
  _lock_size=0;
  _s        =null;
  _s3d      =null;
#elif XAUDIO
  _sv=null;
  _3d=false;
#elif OPEN_AL
  _3d=false;
  _source=0;
  _buffer[0]=_buffer[1]=0;
#elif OPEN_SL
  _3d                  =false;
  _processed           =0;
  _volume=_range       =1;
  _pos                 .zero();
   player_object       =null;
   player_play         =null;
   player_volume       =null;
   player_playback_rate=null;
   player_buffer_queue =null;
   player_location     =null;
   player_doppler      =null;
   player_source       =null;
#endif
}
SoundBuffer::SoundBuffer() {zero();}
/******************************************************************************/
void SoundBuffer::del()
{
   SOUND_API_LOCK_WEAK;
   stop();
#if DIRECT_SOUND
   if(DS)
   {
      RELEASE(_s3d);
      RELEASE(_s  );
   }
#elif XAUDIO
   if(XAudio)
   {
      if(_sv)_sv->DestroyVoice();
   }
  _data.del(); // delete the buffer after '_sv'
#elif OPEN_AL
   if(ALContext)
   {
      if(_source   )alDeleteSources(1, &_source   );
      if(_buffer[0])alDeleteBuffers(1, &_buffer[0]);
      if(_buffer[1])alDeleteBuffers(1, &_buffer[1]);
   }
#elif OPEN_SL
   if(SLOutputMixObject)
   {
      if(player_object)(*player_object)->Destroy(player_object);
   }
  _data.del(); // delete the buffer after 'player_object'
#endif
   zero();
}
/******************************************************************************/
#if XAUDIO
/*static struct XAudioVoiceCallback : IXAudio2VoiceCallback
{
   virtual void OnVoiceProcessingPassStart(UINT32 BytesRequired)override {}
   virtual void OnVoiceProcessingPassEnd()override {}
   virtual void OnStreamEnd()override {}
   virtual void OnBufferStart(Ptr user)override {}
   virtual void OnBufferEnd(Ptr user)override {}
   virtual void OnLoopEnd(Ptr user)override {}
   virtual void OnVoiceError(Ptr user, HRESULT error)override {}
}XAVC;*/
#elif OPEN_SL
static void PlayerCallback(SLAndroidSimpleBufferQueueItf buffer_queue, void *context)
{
   if(SoundBuffer *buffer=(SoundBuffer*)context)
   {
      AtomicInc(buffer->_processed);
   }
}
#endif
void SoundBuffer::memAddressChanged() // !! can't be called when the SoundBuffer is playing/paused !! this is a special case which should handle locking !! because it's rarely called and should lock only when needed
{
#if OPEN_SL
   if(player_buffer_queue)
   {
      SOUND_API_LOCK_FORCE;
      (*player_buffer_queue)->RegisterCallback(player_buffer_queue, PlayerCallback, this); // !! this can be changed only in SL_PLAYSTATE_STOPPED state !! https://www.khronos.org/registry/sles/specs/OpenSL_ES_Specification_1.0.1.pdf
   }
#endif
}
Bool SoundBuffer::create(Int frequency, Int bits, Int channels, Int samples, Bool is3D)
{
   del();

   if(SoundAPI && frequency>=1 && (bits==8 || bits==16 || bits==24) && (channels>=1 && channels<=2) && samples>0)
   {
    C Int bytes=bits/8;

   #if OPEN_AL
      if(is3D)channels=1; // OpenAL supports only mono 3D
   #endif

     _par.frequency=frequency;
     _par.bytes    =bytes;
     _par.channels =channels;
     _par.block    =channels*bytes;
     _par.bit_rate =channels*bits*frequency;
     _par.size     =_par.block*samples;

   #if DIRECT_SOUND || XAUDIO
      WAVEFORMATEX wf; Zero(wf);
      wf.wFormatTag     =WAVE_FORMAT_PCM;
      wf.nChannels      =channels;
      wf.nSamplesPerSec =frequency;
      wf.wBitsPerSample =bits; 
      wf.nBlockAlign    =_par.block;
      wf.nAvgBytesPerSec=_par.block*frequency;
   #endif
   #if DIRECT_SOUND
      DSBUFFERDESC dsbd; Zero(dsbd);
      dsbd.dwSize       =SIZE(DSBUFFERDESC);
      dsbd.dwFlags      =(is3D ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN)|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_STATIC|((App.flag&APP_WORK_IN_BACKGROUND) ? DSBCAPS_GLOBALFOCUS : 0);
      dsbd.dwBufferBytes=_par.size;
      dsbd.lpwfxFormat  =&wf;
      
      SOUND_API_LOCK_WEAK;
      Bool ok=false;
      if(!is3D)ok=OK(DS->CreateSoundBuffer(&dsbd, &_s, null));else
      {
         if(!ok && Virtualization>=VIRT_HIGH){dsbd.guid3DAlgorithm=DS3DALG_HRTF_FULL        ; ok=OK(DS->CreateSoundBuffer(&dsbd, &_s, null));}
         if(!ok && Virtualization>=VIRT_LOW ){dsbd.guid3DAlgorithm=DS3DALG_HRTF_LIGHT       ; ok=OK(DS->CreateSoundBuffer(&dsbd, &_s, null));}
         if(!ok && Virtualization>=VIRT_NONE){dsbd.guid3DAlgorithm=DS3DALG_NO_VIRTUALIZATION; ok=OK(DS->CreateSoundBuffer(&dsbd, &_s, null));}
         if( ok && !OK(_s->QueryInterface(IID_IDirectSound3DBuffer, (Ptr*)&_s3d)))ok=false;
      }
      if(ok)return true;
   #elif XAUDIO
      SOUND_API_LOCK_WEAK;
      if(OK(XAudio->CreateSourceVoice(&_sv, &wf, 0, MAX_SOUND_SPEED/*, &XAVC*/)))
      {
        _3d=is3D; // if sound will be 3 dimensional
        _data.setNum(_par.size); // allocate total buffer size to have memory for sound data, XAudio needs it constantly during playback - "but the actual audio data referenced by pBuffer must remain valid until the buffer has been fully consumed by XAudio2 (which is indicated by the IXAudio2VoiceCallback::OnBufferEnd callback)" https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.ixaudio2sourcevoice.ixaudio2sourcevoice.submitsourcebuffer(v=vs.85).aspx
         return true;
      }
   #elif OPEN_AL
      if(frequency<=192000)
      {
         SOUND_API_LOCK_WEAK;
         alGenBuffers(Elms(_buffer),  _buffer);
         alGenSources(            1, &_source);
             _3d=is3D; // if sound will be 3 dimensional
         if(!_3d)alSourcei(_source, AL_SOURCE_RELATIVE, true); // 2d sounds are relative to listener
         return true;
      }
   #elif OPEN_SL
      if(frequency<=192000)
      {
         ASSERT(SL_SAMPLINGRATE_48==48000000); // verify the scale
         Bool try3D=(is3D && SLListenerLocation);

         SLDataLocator_AndroidSimpleBufferQueue buffer_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
         SLDataFormat_PCM                       format_pcm  ={SL_DATAFORMAT_PCM, SLuint32(channels), SLuint32(frequency*1000), SLuint32(bits), SLuint32(bits), (channels==2) ? SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
         SLDataSource                           audio_src   ={&buffer_queue, &format_pcm};
         SLDataLocator_OutputMix                output_mix  ={SL_DATALOCATOR_OUTPUTMIX, SLOutputMixObject};
         SLDataSink                             audio_sink  ={&output_mix, null};
         // Asus Transformer Prime and TF300 have buggy OpenSL implementation (specifying SL_BOOLEAN_FALSE for 'required' will always fail even if the interface is available), we need to always try using SL_BOOLEAN_TRUE for required
         REPD(t3   , try3D ? 2 : 1)
         REPD(vol  , 2)
         REPD(speed, 2)
         {
            SLInterfaceID ids       []={SL_IID_BUFFERQUEUE, SL_IID_VOLUME                           , SL_IID_PLAYBACKRATE                       };
            SLboolean     required  []={SL_BOOLEAN_TRUE   , vol ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, speed ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE};
            SLInterfaceID ids3D     []={SL_IID_BUFFERQUEUE, SL_IID_VOLUME                           , SL_IID_PLAYBACKRATE                       , SL_IID_3DLOCATION, SL_IID_3DSOURCE/*, SL_IID_3DDOPPLER*/}; // don't create doppler to increase performance
            SLboolean     required3D[]={SL_BOOLEAN_TRUE   , vol ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, speed ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, SL_BOOLEAN_TRUE  , SL_BOOLEAN_TRUE/*, SL_BOOLEAN_FALSE*/};

            SOUND_API_LOCK_WEAK;
            if((*SLEngineEngine     )->CreateAudioPlayer(SLEngineEngine, &player_object, &audio_src, &audio_sink, t3 ? Elms(ids3D) : Elms(ids), t3 ? ids3D : ids, t3 ? required3D : required)==SL_RESULT_SUCCESS)
            if((*player_object      )->Realize          (player_object, SL_BOOLEAN_FALSE                        )==SL_RESULT_SUCCESS)
            if((*player_object      )->GetInterface     (player_object, SL_IID_PLAY       , &player_play        )==SL_RESULT_SUCCESS)
            if((*player_object      )->GetInterface     (player_object, SL_IID_BUFFERQUEUE, &player_buffer_queue)==SL_RESULT_SUCCESS)
            if((*player_buffer_queue)->RegisterCallback (player_buffer_queue, PlayerCallback, this              )==SL_RESULT_SUCCESS)
            {
              _3d=is3D; // if sound will be 3 dimensional
              _data.setNum(_par.size); // allocate total buffer size to have memory for sound data, OpenSL needs it constantly during playback, this was tested, after calling 'Enqueue' the data was zeroed, which resulted in no sound being played, which means that as long as the sound is playing, the buffer data must be present

                     (*player_object)->GetInterface(player_object, SL_IID_VOLUME      , &player_volume       ); // this is optional
                     (*player_object)->GetInterface(player_object, SL_IID_PLAYBACKRATE, &player_playback_rate); // this is optional
               if(t3)(*player_object)->GetInterface(player_object, SL_IID_3DLOCATION  , &player_location     ); // this is optional
               if(t3)(*player_object)->GetInterface(player_object, SL_IID_3DSOURCE    , &player_source       ); // this is optional
               if(t3)(*player_object)->GetInterface(player_object, SL_IID_3DDOPPLER   , &player_doppler      ); // this is optional

               if(_3d && !player_location && player_volume) // if should be 3D but not available, then volume will be adjusted manually
                  (*player_volume)->EnableStereoPosition(player_volume, SL_BOOLEAN_TRUE); // enable stereo panning

               return true;
            }
         }
      }
   #endif
      del();
   }
   return false;
}
/******************************************************************************/
#if DIRECT_SOUND
Bool SoundBuffer::lock(Int pos, Int size)
{
   if(_s && !_lock_data)
   {
      if(size<0)size=_par.size-pos;
      DWORD dw; SOUND_API_LOCK_WEAK; if(OK(_s->Lock(pos, size, &_lock_data, &dw, null, null, 0))){_lock_size=dw; return true;}
     _lock_data=null;
     _lock_size=0;
   }
   return false;
}
void SoundBuffer::unlock()
{
   if(_s && _lock_data)
   {
      SOUND_API_LOCK_WEAK; _s->Unlock(_lock_data, _lock_size, null, 0);
     _lock_data=null;
     _lock_size=0;
   }
}
#endif
/******************************************************************************/
#if DIRECT_SOUND
Bool SoundBuffer::is  ()C {return _s  !=null;}
Bool SoundBuffer::is3D()C {return _s3d!=null;}
#elif XAUDIO
Bool SoundBuffer::is  ()C {return _sv!=null;}
Bool SoundBuffer::is3D()C {return _3d      ;}
#elif OPEN_AL
Bool SoundBuffer::is  ()C {return _source!=0;}
Bool SoundBuffer::is3D()C {return _3d       ;}
#elif OPEN_SL
Bool SoundBuffer::is  ()C {return player_object!=null;}
Bool SoundBuffer::is3D()C {return _3d                ;}
#else
Bool SoundBuffer::is  ()C {return false;}
Bool SoundBuffer::is3D()C {return false;}
#endif
/******************************************************************************/
void SoundBuffer::raw(Int raw)
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->SetCurrentPosition(raw);}
#elif XAUDIO
   // XAudio does not support setting buffer position
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcei(_source, AL_BYTE_OFFSET, raw);}
#elif OPEN_SL
   // OpenSL does not support setting buffer position
#endif
}
Int SoundBuffer::raw()C
{
#if DIRECT_SOUND
   if(_s){DWORD play=0, write=0; SOUND_API_LOCK_WEAK; if(OK(_s->GetCurrentPosition(&play, &write)))return play;}
#elif XAUDIO
   if(_sv)
   {
      SOUND_API_LOCK_WEAK; XAUDIO2_VOICE_STATE state;
   #if WINDOWS_OLD
     _sv->GetState(&state);
   #else
     _sv->GetState(&state, 0);
   #endif
      return (state.SamplesPlayed*_par.block)%_par.size;
   }
#elif OPEN_AL
   if(_source){Int raw=0; SOUND_API_LOCK_WEAK; alGetSourcei(_source, AL_BYTE_OFFSET, &raw); return raw;}
#elif OPEN_SL
   if(player_play && _par.block && _par.frequency)
   {
      SLmillisecond time=0;
      SOUND_API_LOCK_WEAK;
      if((*player_play)->GetPosition(player_play, &time)==SL_RESULT_SUCCESS) // this will be in range 0..Inf (not to 0..buffer_time, because of that we need to do mod)
       //if(UInt buffer_time=_par.size/_par.block*1000/_par.frequency) // length of sound buffer in milliseconds, this is useless because this is an approximation due to integer division losing fraction
         return (U64(time)*_par.frequency/1000*_par.block)%_par.size; // need to convert to U64 so we won't overflow, mul by block at the end to make sure that this is a multiple of block
   }
#endif
   return 0;
}
/******************************************************************************/
void SoundBuffer::pan(Flt pan) // 'pan' should be in range -1..1
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->SetPan(Round(pan*DSBPAN_RIGHT));} ASSERT(-(DSBPAN_LEFT)==DSBPAN_RIGHT);
#elif XAUDIO
   if(_sv){Flt vols[]={Sat(1-pan), Sat(1+pan)}; SOUND_API_LOCK_WEAK; _sv->SetChannelVolumes(2, vols, OPERATION_SET);}
#elif OPEN_AL
 //if(_source && !_3d){SOUND_API_LOCK_WEAK; alSource3f(_source, AL_POSITION, pan, 0, 0);} this is not smooth and does not work for stereo sounds
#elif OPEN_SL
   if(player_volume){SOUND_API_LOCK_WEAK; (*player_volume)->SetStereoPosition(player_volume, Round(pan*1000));} // will work only if 'EnableStereoPosition' was called
#endif
}
#if OPEN_SL
Bool SoundBuffer::emulate3D()
{
   if(_3d && !player_location && player_volume) // if should be 3D, but it's not available, then adjust volume manually
   {
      Vec delta =_pos-Listener.pos();
      Flt dist  =delta.length(),
          volume=T._volume;
      if( dist>_range)volume*=_range/dist;
      if( dist>     1)delta /=dist; // normalize only if the length is greater than 1 so that we can still have panning when the distance is less than 1
      Flt pan=Dot(delta, Listener.right());

      SOUND_API_LOCK_WEAK;
      T.pan(pan);
      SLmillibel linear=Max(SL_MILLIBEL_MIN, Round(Lerp(-10000, 0, Pow(volume, 0.1f)))); // this matches DirectSound version and sounds ok
      (*player_volume)->SetVolumeLevel(player_volume, linear);
      return true;
   }
   return false;
}
#endif
void SoundBuffer::volume(Flt volume) // 'volume' should be in range 0..1
{
#if DIRECT_SOUND
   if(_s)
   {
      if(_s3d)volume/=_par.channels; // 3D stereo sounds will play at different volumes than 3D mono, so adjust, do this here and not in 'range' because there we can affect volumes only outside of 'range' and not inside
      Int linear=Round(Lerp(DSBVOLUME_MIN, DSBVOLUME_MAX, Pow(volume, 0.1f)));
      SOUND_API_LOCK_WEAK; _s->SetVolume(linear);
   }
#elif XAUDIO
   if(_sv)
   {
      if(_3d)volume/=_par.channels; // 3D stereo sounds will play at different volumes than 3D mono, so adjust, do this here and not in 'range' because there we can affect volumes only outside of 'range' and not inside (we could adjust the 'dsp.pMatrixCoefficients' however it has much more elements that just one volume, and most likely 'setParams' will be called very frequently, while 'volume' not so much)
      SOUND_API_LOCK_WEAK; _sv->SetVolume(volume, OPERATION_SET);
   }
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcef(_source, AL_GAIN, volume);}
#elif OPEN_SL
   if(player_volume)
   {
      T._volume=volume;
      if(!emulate3D()) // if not emulating 3D
      {
         SLmillibel linear=Max(SL_MILLIBEL_MIN, Round(Lerp(-10000, 0, Pow(T._volume, 0.1f)))); // this matches DirectSound version and sounds ok
         SOUND_API_LOCK_WEAK; (*player_volume)->SetVolumeLevel(player_volume, linear);
      }
   }
#endif
}
#if XAUDIO
static Flt ChannelAzimuths[16]; // all zeros
void SoundBuffer::setParams(C _Sound &sound, Bool pos_range, Bool doppler)
{
   if(_sv)
   {
      MemtN<Flt, 2*8*4>     matrix; // default channels for a sound = 1..2, max supported number of speakers = 8, mul by 4 for safety
      X3DAUDIO_EMITTER      emitter;
      X3DAUDIO_DSP_SETTINGS dsp;
      UInt                  flag=0;
      if(pos_range) // !! process this first because of "Zero(emitter)" !!
      {
         flag|=X3DAUDIO_CALCULATE_MATRIX;

         Zero(emitter);
         emitter.ChannelCount=_par.channels;
         emitter.pChannelAzimuths=ChannelAzimuths; DEBUG_ASSERT(emitter.ChannelCount<=Elms(ChannelAzimuths), "ChannelAzimuths");
         emitter.InnerRadius=0.15f; // avg dist between ears = 0.2, so radius should be 0.1 or 0.2, let's use average of that
         emitter.CurveDistanceScaler=Max(FLT_MIN, sound._range);

         dsp.SrcChannelCount=_par.channels;
         dsp.DstChannelCount=XAudioChannels;
         dsp.pMatrixCoefficients=matrix.setNum(dsp.SrcChannelCount*dsp.DstChannelCount).data();
      }
      if(doppler)
      {
         flag|=X3DAUDIO_CALCULATE_DOPPLER;

         emitter.Velocity.x=sound._vel.x; emitter.Velocity.y=sound._vel.y; emitter.Velocity.z=sound._vel.z;
         emitter.DopplerScaler=1;
      }

      emitter.Position.x=sound._pos.x; emitter.Position.y=sound._pos.y; emitter.Position.z=sound._pos.z; // this is needed for both
      X3DAudioCalculate(X3DAudio, &X3DListener, &emitter, flag, &dsp);
      SOUND_API_LOCK_WEAK;
      if(pos_range)_sv->SetOutputMatrix  (XAudioMasteringVoice, _par.channels, XAudioChannels, matrix.data(), OPERATION_SET); // 'SetOutputMatrix' and 'SetVolume' can be set independently
      if(doppler  )_sv->SetFrequencyRatio(                 SoundSpeed(sound._actual_speed*dsp.DopplerFactor), OPERATION_SET);
   }
}
#endif
void SoundBuffer::frequency(Int frequency)
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->SetFrequency(Mid(frequency, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX));}
#elif XAUDIO || OPEN_AL || OPEN_SL
   speed(frequency/Flt(_par.frequency));
#endif
}
void SoundBuffer::speed(Flt speed)
{
#if DIRECT_SOUND
   frequency(Round(_par.frequency*speed));
#elif XAUDIO
   if(_sv){SOUND_API_LOCK_WEAK; _sv->SetFrequencyRatio(speed, OPERATION_SET);}
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcef(_source, AL_PITCH, speed);}
#elif OPEN_SL
   if(player_playback_rate)
   {
      SLpermille min_rate=500, max_rate=2000, step_size; SLuint32 capabilities; // default limit is 0.5 to 2.0 (tested on HTC Evo 3D Android 2.3)
      SOUND_API_LOCK_WEAK;
      (*player_playback_rate)->GetRateRange(player_playback_rate, 0, &min_rate, &max_rate, &step_size, &capabilities);
      (*player_playback_rate)->SetRate     (player_playback_rate, Mid(Round(speed*1000), min_rate, max_rate));
   }
#endif
}
void SoundBuffer::pos(C Vec &pos)
{
#if DIRECT_SOUND
   if(_s3d){SOUND_API_LOCK_WEAK; _s3d->SetPosition(pos.x, pos.y, pos.z, DS3D_DEFERRED);}
#elif XAUDIO
   // handled in 'setParams'
#elif OPEN_AL
   if(_source && _3d){SOUND_API_LOCK_WEAK; alSource3f(_source, AL_POSITION, pos.x, pos.y, pos.z);}
#elif OPEN_SL
   T._pos=pos;
   if(!emulate3D() && player_location)
   {
      SLVec3D location; location.x=Round(pos.x/SL_POS_UNIT); location.y=Round(pos.y/SL_POS_UNIT); location.z=Round(pos.z/SL_POS_UNIT);
      SOUND_API_LOCK_WEAK; (*player_location)->SetLocationCartesian(player_location, &location);
   }
#endif
}
Vec SoundBuffer::pos()C
{
#if DIRECT_SOUND
   if(_s3d){D3DVECTOR pos; SOUND_API_LOCK_WEAK; if(OK(_s3d->GetPosition(&pos)))return Vec(pos.x, pos.y, pos.z);}
#elif XAUDIO
   // unavailable
#elif OPEN_AL
   if(_source && _3d){Vec pos; SOUND_API_LOCK_WEAK; alGetSource3f(_source, AL_POSITION, &pos.x, &pos.y, &pos.z); return pos;}
#elif OPEN_SL
   #if 1 // faster
      return _pos;
   #else
      if(player_location)
      {
         SLVec3D pos;
         SOUND_API_LOCK_WEAK;
         if((*player_location)->GetLocationCartesian(player_location, &pos)==SL_RESULT_SUCCESS)
            return Vec(pos.x*SL_POS_UNIT, pos.y*SL_POS_UNIT, pos.z*SL_POS_UNIT);
      }
   #endif
#endif
   return VecZero;
}
void SoundBuffer::vel(C Vec &vel)
{
#if DIRECT_SOUND
   if(_s3d){SOUND_API_LOCK_WEAK; _s3d->SetVelocity(vel.x, vel.y, vel.z, DS3D_DEFERRED);}
#elif XAUDIO
   // handled in 'setParams'
#elif OPEN_AL
   if(_source && _3d){SOUND_API_LOCK_WEAK; alSource3f(_source, AL_VELOCITY, vel.x, vel.y, vel.z);}
#elif OPEN_SL
   if(player_doppler)
   {
      SLVec3D velocity; velocity.x=Round(vel.x/SL_POS_UNIT); velocity.y=Round(vel.y/SL_POS_UNIT); velocity.z=Round(vel.z/SL_POS_UNIT);
      SOUND_API_LOCK_WEAK; (*player_doppler)->SetVelocityCartesian(player_doppler, &velocity);
   }
#endif
}
Vec SoundBuffer::vel()C
{
#if DIRECT_SOUND
   if(_s3d){D3DVECTOR vel; SOUND_API_LOCK_WEAK; if(OK(_s3d->GetVelocity(&vel)))return Vec(vel.x, vel.y, vel.z);}
#elif XAUDIO
   // unavailable
#elif OPEN_AL
   if(_source && _3d){Vec vel; SOUND_API_LOCK_WEAK; alGetSource3f(_source, AL_VELOCITY, &vel.x, &vel.y, &vel.z); return vel;}
#elif OPEN_SL
   if(player_doppler)
   {
      SLVec3D vel;
      SOUND_API_LOCK_WEAK;
      if((*player_doppler)->GetVelocityCartesian(player_doppler, &vel)==SL_RESULT_SUCCESS)
         return Vec(vel.x*SL_POS_UNIT, vel.y*SL_POS_UNIT, vel.z*SL_POS_UNIT);
   }
#endif
   return VecZero;
}
void SoundBuffer::range(Flt range)
{
   MAX(range, 0.0f);
#if DIRECT_SOUND
   if(_s3d){SOUND_API_LOCK_WEAK; _s3d->SetMinDistance(range*DIRECT_SOUND_RANGE_SCALE, DS3D_DEFERRED);}
#elif XAUDIO
   // handled in 'setParams'
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcef(_source, AL_REFERENCE_DISTANCE, range*OPEN_AL_RANGE_SCALE);}
#elif OPEN_SL
   T._range=range;
   if(!emulate3D() && player_source){SOUND_API_LOCK_WEAK; (*player_source)->SetRolloffDistances(player_source, RoundPos(range/SL_POS_UNIT), SL_MILLIMETER_MAX);}
#endif
}
Flt SoundBuffer::range()C
{
#if DIRECT_SOUND
   if(_s3d){Flt range=0; SOUND_API_LOCK_WEAK; if(OK(_s3d->GetMinDistance(&range)))return range/DIRECT_SOUND_RANGE_SCALE;}
#elif XAUDIO
   // unavailable
#elif OPEN_AL
   Flt range=0; if(_source){SOUND_API_LOCK_WEAK; alGetSourcef(_source, AL_REFERENCE_DISTANCE, &range);} return range/OPEN_AL_RANGE_SCALE;
#elif OPEN_SL
   #if 1 // faster
      return _range;
   #else
      if(player_source)
      {
         SLmillimeter min, max;
         SOUND_API_LOCK_WEAK;
         if((*player_source)->GetRolloffDistances(player_source, &min, &max)==SL_RESULT_SUCCESS)
            return min*SL_POS_UNIT;
      }
   #endif
#endif
   return 0;
}
/******************************************************************************/
void SoundBuffer::stop()
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->Stop(); raw(0);}
#elif XAUDIO
   if(_sv){SOUND_API_LOCK_WEAK; _sv->Stop(0, OPERATION_SET); _sv->FlushSourceBuffers();} // 'FlushSourceBuffers' - removes all pending audio buffers from this voice's queue
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourceStop(_source); alSourcei(_source, AL_BUFFER, 0);} // calling "alSourcei(_source, AL_BUFFER, 0)" means clearing the buffer queue, this is correct
#elif OPEN_SL
   SOUND_API_LOCK_WEAK;
   if(player_play        )(*player_play        )->SetPlayState(player_play        , SL_PLAYSTATE_STOPPED);
   if(player_buffer_queue)(*player_buffer_queue)->Clear       (player_buffer_queue                      ); _processed=0; // clear the queue
#endif
}
void SoundBuffer::pause()
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->Stop();}
#elif XAUDIO
   if(_sv){SOUND_API_LOCK_WEAK; _sv->Stop(0, OPERATION_SET);}
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcePause(_source);}
#elif OPEN_SL
   if(player_play){SOUND_API_LOCK_WEAK; (*player_play)->SetPlayState(player_play, SL_PLAYSTATE_PAUSED);}
#endif
}
void SoundBuffer::toggle(Bool loop)
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; if(playing())_s->Stop();else _s->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);}
#elif XAUDIO
   // unavailable
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; if(playing())alSourcePause(_source);else alSourcePlay(_source);}
#elif OPEN_SL
   if(player_play){SOUND_API_LOCK_WEAK; (*player_play)->SetPlayState(player_play, playing() ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING);}
#endif
}
Bool SoundBuffer::playing()C
{
#if DIRECT_SOUND
   DWORD status=0; if(_s){SOUND_API_LOCK_WEAK; _s->GetStatus(&status);} return FlagTest(status, DSBSTATUS_PLAYING);
#elif XAUDIO
   // unavailable
   return false;
#elif OPEN_AL
   Int state=AL_STOPPED; if(_source){SOUND_API_LOCK_WEAK; alGetSourcei(_source, AL_SOURCE_STATE, &state);} return state==AL_PLAYING;
#elif OPEN_SL
   SLuint32 state=SL_PLAYSTATE_STOPPED; if(player_play){SOUND_API_LOCK_WEAK; (*player_play)->GetPlayState(player_play, &state);} return state==SL_PLAYSTATE_PLAYING;
#else
   return false;
#endif
}
void SoundBuffer::play(Bool loop)
{
#if DIRECT_SOUND
   if(_s){SOUND_API_LOCK_WEAK; _s->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);}
#elif XAUDIO
   if(_sv){SOUND_API_LOCK_WEAK; _sv->Start(0, OPERATION_SET);}
#elif OPEN_AL
   if(_source){SOUND_API_LOCK_WEAK; alSourcePlay(_source);} // don't touch AL_LOOPING because OpenAL buffers won't swap
#elif OPEN_SL
   if(player_play){SOUND_API_LOCK_WEAK; (*player_play)->SetPlayState(player_play, SL_PLAYSTATE_PLAYING);}
#endif
}
/******************************************************************************/
// 3D SOUND LISTENER
/******************************************************************************/
#if 0 // this is cached in members
Vec ListenerClass::pos()
{
#if DIRECT_SOUND
   if(DSL){D3DVECTOR pos; SOUND_API_LOCK_WEAK; if(OK(DSL->GetPosition(&pos)))return Vec(pos.x, pos.y, pos.z);}
#elif XAUDIO
   return Vec(X3DListener.Position.x, X3DListener.Position.y, X3DListener.Position.z);
#elif OPEN_AL
   Vec pos; SOUND_API_LOCK_WEAK; alGetListener3f(AL_POSITION, &pos.x, &pos.y, &pos.z); return pos;
#elif OPEN_SL
   if(SLListenerLocation)
   {
      SLVec3D pos;
      SOUND_API_LOCK_WEAK;
      if((*SLListenerLocation)->GetLocationCartesian(SLListenerLocation, &pos)==SL_RESULT_SUCCESS)
         return Vec(pos.x*SL_POS_UNIT, pos.y*SL_POS_UNIT, pos.z*SL_POS_UNIT);
   }
#endif
   return VecZero;
}
#endif
ListenerClass::ListenerClass() {_orn.pos.zero(); _orn.dir.set(0, 0, 1); _orn.perp.set(0, 1, 0); _vel.zero(); _flag=SOUND_CHANGED_POS|SOUND_CHANGED_VEL|SOUND_CHANGED_ORN;} // set flag to make sure that we set params in case the SoundAPI uses different initial values
ListenerClass& ListenerClass::pos(C Vec &pos) {if(T.pos()!=pos){T._orn.pos=pos; AtomicOr(_flag, SOUND_CHANGED_POS);} return T;} // modify first and enable flag at the end
ListenerClass& ListenerClass::vel(C Vec &vel) {if(T.vel()!=vel){T._vel    =vel; AtomicOr(_flag, SOUND_CHANGED_VEL);} return T;} // modify first and enable flag at the end
ListenerClass& ListenerClass::orn(C Vec &dir, C Vec &up)
{
   Orient orn(dir, up); orn.fixPerp();
   if(SCAST(Orient, T._orn)!=orn)
   {
      SCAST(Orient, T._orn)=orn;
      AtomicOr(_flag, SOUND_CHANGED_ORN); // modify first and enable flag at the end
   }
   return T;
}
UInt ListenerClass::updateNoLock() // requires 'SoundAPILock'
{
   if(_flag)
   {
      UInt flag=AtomicDisable(_flag, SOUND_CHANGED_POS|SOUND_CHANGED_VEL|SOUND_CHANGED_ORN); // disable these flags first, so in case the user modifies listener parameters while this function is running, it will be activated again

   #if DIRECT_SOUND
      if(DSL)
      {
         if(flag&SOUND_CHANGED_POS)DSL->SetPosition   (pos().x, pos().y, pos().z,                         DS3D_DEFERRED);
         if(flag&SOUND_CHANGED_VEL)DSL->SetVelocity   (vel().x, vel().y, vel().z,                         DS3D_DEFERRED);
         if(flag&SOUND_CHANGED_ORN)DSL->SetOrientation(dir().x, dir().y, dir().z, up().x, up().y, up().z, DS3D_DEFERRED);
      }
   #elif XAUDIO
      if(flag&SOUND_CHANGED_POS){X3DListener.Position   .x=pos().x; X3DListener.Position   .y=pos().y; X3DListener.Position   .z=pos().z;}
      if(flag&SOUND_CHANGED_VEL){X3DListener.Velocity   .x=vel().x; X3DListener.Velocity   .y=vel().y; X3DListener.Velocity   .z=vel().z;}
      if(flag&SOUND_CHANGED_ORN){X3DListener.OrientFront.x=dir().x; X3DListener.OrientFront.y=dir().y; X3DListener.OrientFront.z=dir().z; 
                                 X3DListener.OrientTop  .x=up ().x; X3DListener.OrientTop  .y=up ().y; X3DListener.OrientTop  .z=up ().z;}
   #elif OPEN_AL
      if(ALContext)
      {
         if(flag&SOUND_CHANGED_POS)alListener3f(AL_POSITION, pos().x, pos().y, pos().z); 
         if(flag&SOUND_CHANGED_VEL)alListener3f(AL_VELOCITY, vel().x, vel().y, vel().z);
         if(flag&SOUND_CHANGED_ORN){Flt f[]={-dir().x, -dir().y, -dir().z, up().x, up().y, up().z}; alListenerfv(AL_ORIENTATION, f);}
      }
   #elif OPEN_SL
      if(SLListenerLocation)
      {
         if(flag&SOUND_CHANGED_POS)
         {
            SLVec3D location; location.x=Round(pos().x/SL_POS_UNIT); location.y=Round(pos().y/SL_POS_UNIT); location.z=Round(pos().z/SL_POS_UNIT);
            (*SLListenerLocation)->SetLocationCartesian(SLListenerLocation, &location);
         }
         if(flag&SOUND_CHANGED_ORN)
         {
            SLVec3D front; front.x=Round(dir().x*1024); front.y=Round(dir().y*1024); front.z=Round(dir().z*1024);
            SLVec3D above; above.x=Round(up ().x*1024); above.y=Round(up ().y*1024); above.z=Round(up ().z*1024);
            (*SLListenerLocation)->SetOrientationVectors(SLListenerLocation, &front, &above); // length of vectors is free of choice
         }
      }
      if(flag&SOUND_CHANGED_VEL)if(SLListenerDoppler)
      {
         SLVec3D velocity; velocity.x=Round(vel().x/SL_POS_UNIT); velocity.y=Round(vel().y/SL_POS_UNIT); velocity.z=Round(vel().z/SL_POS_UNIT);
         (*SLListenerDoppler)->SetVelocityCartesian(SLListenerDoppler, &velocity);
      }
   #endif
      return flag;
   }
   return 0;
}
void ListenerClass::commitNoLock() // requires 'SoundAPILock', also because of 'EmulateSound3D'
{
   // commit changes, do this always, not only on Listener change, because sound positions on DirectSound are called with DS3D_DEFERRED and on XAudio not just 3D is deferred but most of the operations
#if DIRECT_SOUND
   if(DSL)DSL->CommitDeferredSettings();
#elif XAUDIO
   if(XAudio)XAudio->CommitChanges(XAUDIO2_COMMIT_ALL);
#elif OPEN_AL
   // alcProcessContext alcSuspendContext are not used because they seem to be a no-op and there's no way to test them
#elif OPEN_SL
   if(SLListenerCommit)(*SLListenerCommit)->Commit(SLListenerCommit);else EmulateSound3D(); // if listener is not available then it means 3D Audio is simulated manually
#endif
}
Bool ListenerClass::create()
{
#if DIRECT_SOUND
   if(DS)
   {
      DSBUFFERDESC dsbd; Zero(dsbd);
      dsbd.dwSize =SIZE(DSBUFFERDESC);
      dsbd.dwFlags=DSBCAPS_PRIMARYBUFFER|DSBCAPS_CTRL3D;

      SOUND_API_LOCK_FORCE;
      IDirectSoundBuffer *DSB; if(!OK(DS->CreateSoundBuffer(&dsbd, &DSB, null)))return false;
      DSB->QueryInterface(IID_IDirectSound3DListener, (Ptr*)&DSL);
      RELEASE(DSB);
   }
   return DSL!=null;
#elif XAUDIO
   if(XAudioMasteringVoice)
   {
   #if WINDOWS_OLD
      XAUDIO2_DEVICE_DETAILS details; if(OK(XAudio->GetDeviceDetails(0, &details)))
      {
         X3DAudioInitialize(details.OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, X3DAudio);
         return true;
      }
   #else
      DWORD channel_mask;
      if(OK(XAudioMasteringVoice->GetChannelMask(&channel_mask)))
         if(OK(X3DAudioInitialize(channel_mask, X3DAUDIO_SPEED_OF_SOUND, X3DAudio)))return true;
   #endif
   }
#elif OPEN_AL
   return true;
#elif OPEN_SL
   return true;
#endif
   return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
#pragma runtime_checks("", off)
static UInt GetSpeakerConfig()
{
#if DIRECT_SOUND
   if(DS)
   {
      DWORD config; DS->GetSpeakerConfig(&config);
      SetLastError(0); // clear error 14007
      return DSSPEAKER_CONFIG(config);
   }
#endif
   return 0;
}
#pragma runtime_checks("", restore)
void InitSound()
{
   if(SoundFunc)return; // return if it was already created
   if(LogInit)LogN("InitSound");
   SOUND_API_LOCK_FORCE;
   SoundFunc=true;

#if DIRECT_SOUND
   if(OK(DirectSoundCreate(null, &DS, null)))
   if(OK(DS->SetCooperativeLevel(App.Hwnd(), DSSCL_PRIORITY)))
#elif XAUDIO
   if(OK(XAudio2Create(&XAudio, 0)))
   if(OK(XAudio->CreateMasteringVoice(&XAudioMasteringVoice))) // even though it is not used for anything, it is still needed because sound buffer creation would fail without it
   {
      XAUDIO2_VOICE_DETAILS details; XAudioMasteringVoice->GetVoiceDetails(&details);
      XAudioChannels=details.InputChannels;
   }

   if(XAudioMasteringVoice)
#elif OPEN_AL
	if(ALDevice=alcOpenDevice(null))
	{
		if(ALContext=alcCreateContext(ALDevice, null))
		{
			Str desc=alcGetString(ALDevice, ALC_DEVICE_SPECIFIER);
			alcMakeContextCurrent(ALContext);
		}else
		{
			alcCloseDevice(ALDevice); ALDevice=null;
		}
	}

	if(ALContext)
#elif OPEN_SL
   SLInterfaceID om_ids     []={SL_IID_NULL     };
   SLboolean     om_required[]={SL_BOOLEAN_FALSE};
   // SL_ENGINEOPTION_THREADSAFE is disabled for better performance since we don't need it
   if(                       slCreateEngine(&SLEngineObject   , 0, null, 0, null, null                               )==SL_RESULT_SUCCESS)
   if((*SLEngineObject   )->Realize        ( SLEngineObject   , SL_BOOLEAN_FALSE                                     )==SL_RESULT_SUCCESS)
   if((*SLEngineObject   )->GetInterface   ( SLEngineObject   , SL_IID_ENGINE, &SLEngineEngine                       )==SL_RESULT_SUCCESS)
   if((*SLEngineEngine   )->CreateOutputMix( SLEngineEngine   , &SLOutputMixObject, Elms(om_ids), om_ids, om_required)==SL_RESULT_SUCCESS)
   if((*SLOutputMixObject)->Realize        ( SLOutputMixObject, SL_BOOLEAN_FALSE                                     )==SL_RESULT_SUCCESS)
   {
      // create optional stuff
      // this could fail if the environmental reverb effect is not available, because the feature is not present, excessive CPU load, or the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
      /*if((*SLOutputMixObject)->GetInterface(SLOutputMixObject, SL_IID_ENVIRONMENTALREVERB, &SLOutputMixEnvironmentalReverb)==SL_RESULT_SUCCESS)
      {
         (*SLOutputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(SLOutputMixEnvironmentalReverb, &reverbSettings);
      }*/

      SLInterfaceID l_ids     []={SL_IID_3DLOCATION, SL_IID_3DCOMMIT /*, SL_IID_3DDOPPLER*/}; // don't create doppler effect to increase performance
      SLboolean     l_required[]={SL_BOOLEAN_TRUE  , SL_BOOLEAN_FALSE/*, SL_BOOLEAN_FALSE*/};
      if((*SLEngineEngine  )->CreateListener(SLEngineEngine  , &SLListenerObject, Elms(l_ids), l_ids, l_required)==SL_RESULT_SUCCESS)
      if((*SLListenerObject)->Realize       (SLListenerObject, SL_BOOLEAN_FALSE                                 )==SL_RESULT_SUCCESS)
      if((*SLListenerObject)->GetInterface  (SLListenerObject, SL_IID_3DLOCATION, &SLListenerLocation           )==SL_RESULT_SUCCESS)
      {
         // create optional stuff
         (*SLListenerObject)->GetInterface(SLListenerObject, SL_IID_3DDOPPLER, &SLListenerDoppler);

         if((*SLListenerObject)->GetInterface(SLListenerObject, SL_IID_3DCOMMIT, &SLListenerCommit)==SL_RESULT_SUCCESS)
         {
            (*SLListenerCommit)->SetDeferred(SLListenerCommit, SL_BOOLEAN_TRUE); // set deferred 3D committing
         }
      }
   }

   if(SLOutputMixObject)
#endif
   {
   #if WINDOWS_OLD
      SetLastError(0); // clear error 1407
   #endif
      Listener.create();
      SoundAPI=true;
   }
   InitSound2(); // create the thread even when SoundAPI failed so we can still process sounds without playing them
}
void ShutSound()
{
   SoundFunc=SoundAPI=false; // this also disables creation of new objects
   ShutSound2(); // destroy thread and sounds
   ShutMusic ();

   SOUND_API_LOCK_FORCE; // lock after deleting 'SoundThread'
#if DIRECT_SOUND
   RELEASE(DSL);
   RELEASE(DS );
#elif XAUDIO
   if(XAudioMasteringVoice){XAudioMasteringVoice->DestroyVoice(); XAudioMasteringVoice=null;}
   RELEASE(XAudio);
#elif OPEN_AL
	              alcMakeContextCurrent(null     );
	if(ALContext){alcDestroyContext    (ALContext); ALContext=null;}
	if(ALDevice ){alcCloseDevice       (ALDevice ); ALDevice =null;}
#elif OPEN_SL
   SLListenerLocation=null;
   SLListenerDoppler =null;
   SLListenerCommit  =null;
   if(SLListenerObject){(*SLListenerObject)->Destroy(SLListenerObject); SLListenerObject=null;}

   if(SLOutputMixObject){(*SLOutputMixObject)->Destroy(SLOutputMixObject); SLOutputMixObject=null;}

   SLEngineEngine=null;
   if(SLEngineObject){(*SLEngineObject)->Destroy(SLEngineObject); SLEngineObject=null;}
#endif

   AppVolume.del();
}
/******************************************************************************/
}
/******************************************************************************/
