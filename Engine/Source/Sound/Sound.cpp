/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   'SoundCallback' needs to have its 'del' method called.

   When accessing 'Sound.sound' member always check 'SoundFunc' too,
      because after 'ShutSound' all '_Sound' have been deleted, and pointers are no longer valid.

   If "_Sound.stream_loaded==false" then '_Sound._stream' may be loaded from any thread using '_Sound.stream()' method.
   If "_Sound.stream_loaded==true"  then '_Sound._stream' can be read+write on sound threads and read (but only simple methods) on other threads.

   '_Sound._buffer' can be used only on sound threads.

   '_Sound._buffer._par' can be different than '_Sound._stream._par'
      -channels can be different for OpenAL 3D (needs downmixing)
      -size for _buffer means buffer size, while size for _stream means stream size

   '_Sound._name' cannot be deleted because it can be accessed on other threads.

   'SoundMemx' can be used on all threads so it needs 'SoundMemxLock', however on non-sound threads only New elements can be created.
   'SoundMemxPlaying' can be used only on sound threads.

   Methods with "!! requires 'SoundAPILock' !!" comment need to be surrounded by a 'SoundAPILock' lock.

   Main  Thread  operates on  Sound.time (main), raw , frac. Time was chosen to be the main value because it's most commonly used on the user side, its usage does not require knowning about the SoundStream.
   Sound Threads operate  on _Sound.raw  (main), time, frac. Raw  was chosen to be the main value because sound buffer data operate on it.
      However, when _Sound is not playing ("_buffer_playing==false"), then _Sound operates on '_Sound._time'.

   At the moment, 'VOLUME_GROUP' and 'SoundVolume' multipliers don't contribute to Sound priority,
      to avoid extra calculations.

/******************************************************************************/
ALIGN_ASSERT(_Sound, _callback); // must have native alignment because we use it in atomic operations for get/set on multiple threads

#define DEL_BUFFER_WHEN_NOT_PLAYING 1 // 0 is not fully implemented so don't use

#define SOUND_API_THREAD_SAFE (DIRECT_SOUND || XAUDIO || OPEN_AL) // DirectSound, XAudio, OpenAL are thread-safe, OpenSL is thread-safe only with SL_ENGINEOPTION_THREADSAFE flag enabled which however is not used in Esenthel since thread-safety is handled manually

#define UPDATE_2X WINDOWS_NEW // on WINDOWS_NEW we have to operate 2x faster due to low-latency sound recording, TODO: can this be improved so we don't have to call 2x more frequently? perhaps do sound recording on a separate dedicated thread

#if SOUND_API_THREAD_SAFE
   #define SOUND_API_LOCK
   #define SOUND_API_LOCK_COND
   #define SOUND_API_LOCK_OFF
   #define SOUND_API_LOCK_SET
#else
   static Bool SoundAPILockDo; // if locking should be done
   #define SOUND_API_LOCK      SyncLockerEx locker(SoundAPILock)
   #define SOUND_API_LOCK_COND SyncLockerEx locker(SoundAPILock, SoundAPILockDo)
   #define SOUND_API_LOCK_OFF  locker.off()
   #define SOUND_API_LOCK_SET  locker.set(SoundAPILockDo)
#endif

#if XAUDIO
static Int ListenerChanged;
#elif OPEN_AL
static Mems< Mems<Byte> > SoundThreadBuffer;
#endif

enum
{
   SOUND_PAUSED     =1<<0,
   SOUND_WANT_PAUSED=1<<1,
   SOUND_WAIT       =1<<2, // temporarily wait (special request used when wanting to pause sound thread until finished some task)
};
enum
{
   SOUND_LOAD_OK,
   SOUND_LOAD_EMPTY,
   SOUND_LOAD_ERROR,
};

static Memx<_Sound >    SoundMemx;
static Memc<_Sound*>    SoundMemxPlaying;
static Thread           SoundThread;
static Threads          SoundThreads;
static SyncEvent        SoundEvent, SoundWaiting;
static Bool             SoundWaited;
static Int              SoundPause, SoundMaxConc=16, SoundMaxThread=1;
static Flt              SoundMinVol=0.020f, // min volume required for a sound to play, this value was chosen by playing music at half volume, and then playing a sound with different volumes, the value was chosen at which the sound couldn't be heard
                        SoundPlayPriority=0.012f, // if a sound is already playing, then increase its priority to avoid playing/stopping due to minor priority differences which causes overhead
                        SoundGroupPriority[VOLUME_NUM];
static Dbl              SoundTime; // time of the last sound update
       SoundVolumeClass SoundVolume;
       SyncLock         SoundAPILock, SoundMemxLock, SoundStreamLock, SoundWait;

Flt SoundMinVolume    () {return SoundMinVol   ;}   void SoundMinVolume    (Flt volume) {SoundMinVol   =Sat(volume);}
Int SoundMaxConcurrent() {return SoundMaxConc  ;}   void SoundMaxConcurrent(Int max   ) {SoundMaxConc  =    max    ;}
Int SoundMaxThreads   () {return SoundMaxThread;}   void SoundMaxThreads   (Int max   ) {SoundMaxThread=    max    ;}
/******************************************************************************/
// SOUND VOLUME CONTROL
/******************************************************************************/
     SoundVolumeClass::SoundVolumeClass() {REPAO(_v)=1;}
void SoundVolumeClass::fx     (Flt v) {SAT(v); if(_v[VOLUME_FX     ]!=v){_v[VOLUME_FX     ]=v; update();}}
void SoundVolumeClass::music  (Flt v) {SAT(v); if(_v[VOLUME_MUSIC  ]!=v){_v[VOLUME_MUSIC  ]=v; update();}}
void SoundVolumeClass::ambient(Flt v) {SAT(v); if(_v[VOLUME_AMBIENT]!=v){_v[VOLUME_AMBIENT]=v; update();}}
void SoundVolumeClass::voice  (Flt v) {SAT(v); if(_v[VOLUME_VOICE  ]!=v){_v[VOLUME_VOICE  ]=v; update();}}
void SoundVolumeClass::ui     (Flt v) {SAT(v); if(_v[VOLUME_UI     ]!=v){_v[VOLUME_UI     ]=v; update();}}
void SoundVolumeClass::global (Flt v) {SAT(v); if(_v[VOLUME_NUM    ]!=v){_v[VOLUME_NUM    ]=v; update();}}
void SoundVolumeClass::update (     ) {VolumeSound();}
/******************************************************************************/
// _SOUND
/******************************************************************************/
void _Sound::zero()
{
 //stream_loaded=false; this is set only in constructor, don't clear it here to avoid unnecessary stream reloads
 //deleted=false; this is set only in constructor, don't clear it here because we need to know if it got deleted
 //flag=0; this is set only in constructor, don't clear it here in case 'Sound' has set 'SOUND_REMOVE', if this would get called due to an error during sound update then it would get cleared, we also need to keep SOUND_NO_REF
  _playing=_buffer_playing=_loop=_is3D=false;
   last_buffer=0xFF;
   kill=0;
   volume_group=VOLUME_FX;
  _fade_curve=FADE_LINEAR;
   raw_pos=0;
  _volume=_speed=_actual_speed=_range=_fade=1;
  _fade_d=_time=0;
   priority=-1;
  _pos=_vel.zero();
  _callback=null;
}
void _Sound::del() // !! requires 'SoundAPILock' !!
{
   deleted=true; // set this first in case other threads access this '_Sound'
  _buffer.del(); // !! requires 'SoundAPILock' !! delete the buffer first

   // we can delete the stream only because we're accessing simple members from it
   if(stream_loaded)_stream.del();else // if 'stream_loaded' then we can access it on this thread
   { // otherwise we need to use 'SoundStreamLock' in case it's being loaded on secondary thread
      SyncLocker locker(SoundStreamLock); _stream.del(); // delete always to clear '_stream._callback'
   }

   // don't delete the '_name' because we may be accessing it on a secondary thread (for both get 'name' method and stream load)
   zero();
   if(noRef())flag|=SOUND_REMOVE; // if there are no references to this sound then we can remove it, no need for 'AtomicOr' since it won't be processed by other thread
}
/******************************************************************************/
void _Sound::init(C Str &name, SoundCallback *call, Bool is3D, VOLUME_GROUP volume_group) // !! warning: 'name' can be 'T._name' !!
{
   T._name=name;
   T._is3D=is3D;
   T._stream._callback=call;

   T.volume_group=(VOLUME_GROUP)Mid(volume_group, 0, VOLUME_NUM-1); // clamp it just in case because later we're using it as array index
   setSpeed(); // !! call this after setting 'volume_group' because that affects the result !!
}
Bool _Sound::init(C _Sound &src)
{
   init(src.name(), src._stream._callback, src.is3D(), src.volume_group);

  _fade      =src._fade;
  _fade_d    =src._fade_d;
  _fade_curve=src._fade_curve;

  _callback=src._callback;

   loop  (src.loop  ());
   time  (src.time  ());
   volume(src.volume());
   speed (src.speed ());
   range (src.range ());
   pos   (src.pos   ());
   vel   (src.vel   ());

   Bool playing=src.playing(); // copy before checking if deleted

   if(src.deleted)return false; // !! it is important to check if the sound got deleted after copying all parameters, which may mean that some of them were not set fully !!

   if(playing)play(); // call this at the end
   return true;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
Flt _Sound::actualSpeed()C
{
   Flt speed=T._speed;
   if(volume_group!=VOLUME_MUSIC && volume_group!=VOLUME_AMBIENT && volume_group!=VOLUME_UI)speed*=Time.speed();
   return speed;
}
void _Sound::setVolume() // !! requires 'SoundAPILock' !!
{
   Flt volume=T._volume*SoundVolume._v[volume_group]*SoundVolume.global(); // adjust the volume by fade and global modifiers
   switch(_fade_curve)
   {
      case FADE_LINEAR: volume*=         _fade ; break;
      case FADE_SQRT  : volume*=SqrtFast(_fade); break; // can use sqrt fast because '_fade' always guaranteed to be 0..1
   }
#if !WINDOWS_OLD
   if(AppVolume.muteFinal())volume=0;else volume*=AppVolume.volume(); // non Windows platforms don't support 'AppVolume' directly, so let's simulate it here
#endif
  _buffer.volume(volume); // !! requires 'SoundAPILock' !!
}
void _Sound::setSpeed()
{
  _actual_speed=actualSpeed();
}
/******************************************************************************/
void _Sound::stop () {_playing=false; time(0);}
void _Sound::pause() {_playing=false;}
void _Sound::play () {_playing=true ;}
/******************************************************************************/
// GET / SET
/******************************************************************************/
void _Sound::memAddressChanged()
{
   {
      SyncLocker locker(SoundStreamLock);
     _stream.memAddressChanged();
   }
  _buffer.memAddressChanged(); // this handles locking on its own
}
SoundStream& _Sound::stream() // !! this may be called on the main thread !!
{
   if(!stream_loaded && !deleted) // don't load if already deleted
   {
      if(_name.is())
      {
         SoundStream temp; temp.create(_name); // load to temporary first to minimize time needed for Lock (in worst case we'll just load # times on # threads, but most likely this won't happen)
         temp.fastSeek(); // for Sound playback enable fast seeking
         if(!stream_loaded)
         {
            {
               SyncLocker locker(SoundStreamLock); if(!stream_loaded)
               {
                  Swap(temp, _stream);
                 _stream.memAddressChanged();
                  stream_loaded=true; // set as last
               }
            }
            temp.memAddressChanged(); // 'temp' may have been swapped above, but call this here outside of the lock to minimize the lock time, call this in case it's needed for proper shut down
         }
      }else // for callback we always need to use Lock because we're storing it in '_stream._callback' for simplicity
      {
         SyncLocker locker(SoundStreamLock); if(!stream_loaded)
         {
            if(_stream._callback)_stream.create(*_stream._callback);
            stream_loaded=true;
         }
      }
   }
   return _stream;
}

C Str& _Sound::name()C {return deleted ? S : _name;} // !! this is also important for '_Sound.save' !! since we're not deleting '_name' to avoid synchronization then return no name if it already got deleted
  UID  _Sound::id  ()C {UID id; if(!deleted && DecodeFileName(_name, id))return id; return UIDZero;}

Bool        _Sound::is       ()C {return !deleted;}
Int         _Sound::channels ()C {return stream().channels ();}
Int         _Sound::frequency()C {return stream().frequency();}
Int         _Sound::bitRate  ()C {return stream().bitRate  ();}
Long        _Sound::size     ()C {return stream().size     ();}
Long        _Sound::samples  ()C {return stream().samples  ();}
Flt         _Sound::length   ()C {return stream().length   ();}
SOUND_CODEC _Sound::codec    ()C {return stream().codec    ();}

void _Sound::loop  (  Bool loop  ) {T._loop=loop;}
void _Sound::volume(  Flt  volume) {SAT(volume  ); if(T._volume!=volume){T._volume=volume; AtomicOr(flag, SOUND_CHANGED_VOLUME);}}
void _Sound::range (  Flt  range ) {MAX(range, 0); if(T._range !=range ){T._range =range ; AtomicOr(flag, SOUND_CHANGED_RANGE );}}
void _Sound::pos   (C Vec &pos   ) {               if(T._pos   !=pos   ){T._pos   =pos   ; AtomicOr(flag, SOUND_CHANGED_POS   );}}
void _Sound::vel   (C Vec &vel   ) {               if(T._vel   !=vel   ){T._vel   =vel   ; AtomicOr(flag, SOUND_CHANGED_VEL   );}}
void _Sound::speed (  Flt  speed ) {MAX(speed, 0); if(T._speed !=speed ){T._speed =speed ; AtomicOr(flag, SOUND_CHANGED_SPEED );}}

void _Sound::raw     (Long raw   )  {C SoundStream &stream=T.stream(); if(Long size   =stream.size   ())time(raw   *(Dbl(stream.length ())/size   ));else time(0);} // use Dbl because index can be huge (more than INT_MAX)
void _Sound::sample  (Long sample)  {C SoundStream &stream=T.stream(); if(Long samples=stream.samples())time(sample*(Dbl(stream.length ())/samples));else time(0);} // use Dbl because index can be huge (more than INT_MAX)
Long _Sound::raw     (           )C {C SoundStream &stream=T.stream(); if(Dbl  length =stream.length ())return   time()*(stream.size   () /length  );   return 0 ;} // use Dbl because index can be huge (more than INT_MAX)
Long _Sound::sample  (           )C {C SoundStream &stream=T.stream(); if(Dbl  length =stream.length ())return   time()*(stream.samples() /length  );   return 0 ;} // use Dbl because index can be huge (more than INT_MAX)
void _Sound::frac    (Flt f      )  {time(f*length());}
void _Sound::time    (Flt t      )  {if(T._time!=t){AtomicOr(flag, SOUND_CHANGING_TIME); T._time=t; AtomicOr(flag, SOUND_CHANGED_TIME);}} // changing time value needs to be surrounded by flags because it gets modified on the sound thread as well
Flt  _Sound::frac    (           )C {if(Flt l=length())return time()/l; return 0;}
Flt  _Sound::timeLeft(           )C {return length()-time();}
/******************************************************************************/
// 'precise*' functions have '_stream' already available
void _Sound::preciseRaw(Long raw)
{
   if(Int block=_stream.block()) // copy to temp in case the value is changed in another thread
   {
      Long old_pos=raw_pos;
      raw_pos=(raw/block)*block;
      if(kill // if started killing 
      && _stream.size()>=0 // this can be negative for unlimited audio (such as callbacks)
      && old_pos>=_stream.size() // because the sound reached the end
      && raw_pos< _stream.size())kill=0; // and new position is before the end, then cancel killing
   }
}
Long _Sound::preciseRaw()C // !! requires 'SoundAPILock' !!
{
   if(last_buffer==0xFF)return raw_pos;

   // buffer is for a short time (not entire sound) so it can operate on 32-bit Int
   Int buffer_pos=_buffer.raw(), // !! requires 'SoundAPILock' !!
       pos       = buffer_pos-_buffer._par.size; // go back because 'raw_pos' is already ahead

#if DIRECT_SOUND || XAUDIO
   if(last_buffer==0)
   {
      Int  buffer_half=_buffer._par.size/2;
      pos-=buffer_half; // go back half the buffer, because after 'buffer_pos' crossed the half, new buffer data was set, advancing 'raw_pos' by half buffer, however 'buffer_pos' remains the same
      if(buffer_pos<buffer_half)pos+=_buffer._par.size; // if 'buffer_pos' got back to the start due to wrapping, then we need to add by entire buffer (what was wrapped), because 'raw_pos' wasn't changed during wrapping
   }
#elif OPEN_AL // OPEN_AL doesn't need this, because its '_buffer.raw' is always for a single buffer and not the total
#elif OPEN_SL // OPEN_SL '_buffer.raw' is very imprecise and inconsistent with '_buffer._processed', because of that, a custom implementation had to be done, based on commented code below
   Int buffer_half=_buffer._par.size/2;
   if(last_buffer==0)
   {
      pos-=buffer_half; // go back half the buffer, because after 'buffer_pos' crossed the half, new buffer data was set, advancing 'raw_pos' by half buffer, however 'buffer_pos' remains the same
   }else
   {
      if(buffer_pos>buffer_half)pos-=_buffer._par.size;
   }
 //static Int last; LogName(S); LogN(S+last_buffer+' '+buffer_pos+'/'+_buffer._par.size+", rp:"+raw_pos+", pos:"+pos+", d:"+(pos-last)); last=pos;
#endif

   // this won't be needed if we would operate on samples
#if OPEN_AL
   pos=pos*_stream.block()/_buffer._par.block; // OpenAL may have buffer/stream block values different due to downmixed 3D stereo to mono
#endif

   Long total_pos=raw_pos+pos; // !! need to use Long now !!
   if(loop())
   {
      Long size=_stream.size(); if(size>=0)total_pos=Mod(total_pos, size); // size can be negative for unlimited audio
   }
   return total_pos;
}
void _Sound::preciseFrac(Flt f)  {preciseRaw(RoundL(Dbl(f)* _stream.size ()                     ));} // use Dbl because 'preciseRaw' can be huge (more than INT_MAX)
void _Sound::preciseTime(Flt t)  {preciseRaw(RoundL(Dbl(t)*(_stream.block()*_stream.frequency())));} // use Dbl because 'preciseRaw' can be huge (more than INT_MAX)
Flt  _Sound::preciseTime(     )C {                 Int  div=_stream.block()*_stream.frequency()   ; return div ? Dbl(preciseRaw())/div : 0;} // !! requires 'SoundAPILock' !! use Dbl because 'preciseRaw' can be huge (more than INT_MAX)
Flt  _Sound::preciseFrac(     )C {                 Long div=_stream.size ()                       ; return div ? Dbl(preciseRaw())/div : 0;} // !! requires 'SoundAPILock' !! use Dbl because 'preciseRaw' can be huge (more than INT_MAX)
/******************************************************************************/
// FADE
/******************************************************************************/
void _Sound::fadeCurve(FADE_CURVE curve)
{
 //if(_fade_curve!=curve)
   {
     _fade_curve=curve;
    //AtomicOr(flag, SOUND_CHANGED_VOLUME); skip this because any fade curve changes should be set before actual fading
   }
}
#define EPS_TIME 0.01f
void _Sound::fadeInFromSilence(Flt fade_duration) // !! this may be called on the main thread !!
{
   if(fade_duration>EPS_TIME) // fade over time
   {
      Flt d=1.0f/fade_duration;
     _fade_d=d; // first set the new delta in case we're fading out slowly, and setting "_fade=0" could immediately delete the sound
     _fade  =0; // set fade start to zero
     _fade_d=d; // in case the fade got cleared before we've set the "_fade=0" then reset delta
    //AtomicOr(flag, SOUND_CHANGED_VOLUME); // need to refresh volume due to '_fade' change HOWEVER this is not needed since we're enabling '_fade_d' which will trigger adjusting volume on its own
   }else // fade immediately
   {
     _fade_d=0; // clear this first to disable fading
     _fade  =1;
      AtomicOr(flag, SOUND_CHANGED_VOLUME); // need to refresh volume due to '_fade' change
   }
}
void _Sound::fadeIn(Flt fade_duration) // !! this may be called on the main thread !!
{
   if(fade_duration>EPS_TIME) // fade over time
   {
     _fade_d=1.0f/fade_duration;
   }else // fade immediately
   {
     _fade_d=0; // clear this first to disable fading
     _fade  =1;
      AtomicOr(flag, SOUND_CHANGED_VOLUME); // need to refresh volume due to '_fade' change
   }
}
void _Sound::fadeOut(Flt fade_duration) // !! this may be called on the main thread !!
{
   if(fade_duration>EPS_TIME) // fade over time
   {
     _fade_d=-1.0f/fade_duration;
   }else // fade immediately
   {
     _fade_d=-1; // set negative value so that '_fade' goes below zero and gets deleted
     _fade  = 0; // set zero and not a negative to avoid having invalid ranges, it will be decreased either way due to '_fade_d' and value <0 will be processed in the update loop
    //AtomicOr(flag, SOUND_CHANGED_VOLUME); no need to refresh volume because the sound will get deleted
   }
}
/******************************************************************************/
// BUFFERING
/******************************************************************************/
void _Sound::setBuffer(Byte *buffer, Int size) // no extra care needed
{
   SoundStream &stream=T.stream();
   Int      size_start=size;
   Long  raw_pos_start=raw_pos, raw_size=stream.size(); // this can be negative for unlimited audio (such as callbacks)
   Byte  *buffer_start=buffer;

   for(; size>0; )
   {
      Long set, clear=0;
      if(raw_size>=0)
      {
         if(_loop && (raw_pos<0 || raw_pos>=raw_size)        )raw_pos=Mod(raw_pos, raw_size);     // adjust 'raw_pos' if it got out of range
         if(          raw_pos<0                              )clear  =-raw_pos              ;else // if data position is before the available data then clear
         if(                       raw_pos>=raw_size && !kill)kill   =2                     ;     // if data position is after  the available data then kill
      }
      if(kill)clear=size; // if killing then clear

      if(clear)
      {
         set=Min(size, clear);
         SetMem(buffer, (stream.bytes()==1) ? 0x80 : 0, set);
      }else
      {
         set=size; if(raw_size>=0)MIN(set, raw_size-raw_pos); // at once process only up to the allowed data range if specified
         set=(stream.pos(raw_pos) ? stream.set(buffer, set) : 0);
         if(set<=0)kill=2;
      }
      buffer +=set;
      raw_pos+=set;
      size   -=set;
   }

   if(SoundDataCallback *callback=_callback)callback->data(buffer_start, size_start, stream, raw_pos_start); // first copy to temp var to avoid multi-threading issues

   if(raw_size>=0)
      if(_loop && (raw_pos<0 || raw_pos>=raw_size))raw_pos=Mod(raw_pos, raw_size); // adjust 'raw_pos' if it got out of range
}
/******************************************************************************/
Bool _Sound::setBuffer(Bool buffer, Int thread_index) // this manages locking on its own
{
   if(kill && !--kill)return false;
   last_buffer=buffer;
#if DIRECT_SOUND
   Int size=_buffer._par.size/2; // we're setting only half of the buffer
   SOUND_API_LOCK_COND; if(_buffer.lock(buffer ? size : 0, size)) // !! requires 'SoundAPILock' !!
   {
      SOUND_API_LOCK_OFF; setBuffer((Byte*)_buffer._lock_data, _buffer._lock_size);
      SOUND_API_LOCK_SET; _buffer.unlock(); // !! requires 'SoundAPILock' !!
      return true;
   }
   return false;
#elif XAUDIO
   if(_buffer._sv)
   {
      Int   size=_buffer._data.elms()/2; // we're setting only half of the buffer
      Byte *data=_buffer._data.data(); if(buffer)data+=size;
      setBuffer(data, size);
      XAUDIO2_BUFFER ab;
      ab.Flags     =0;
      ab.AudioBytes=size;
      ab.pAudioData=data;
      ab.PlayBegin =0;
      ab.PlayLength=0;
      ab.LoopBegin =0;
      ab.LoopLength=0;
      ab.LoopCount =0;
      ab.pContext  =this;
      SOUND_API_LOCK_COND;
      return OK(_buffer._sv->SubmitSourceBuffer(&ab, null)); // !! requires 'SoundAPILock' !!
   }
   return false;
#elif OPEN_AL
   Bool downmix=(_buffer._par.channels==1 && _stream.channels()==2),
         stereo=(_buffer._par.channels==2),
         bit16 =(_buffer._par.bytes   >=2); // use >= to include 24-bit which will be downsampled
   Int   size  = _buffer._par.size; if(!downmix)size/=2; // we're setting only half of the buffer (don't do this for downmixing, because for that we need initially 2x more)
   Byte *buffer_data, temp[512*1024]; // 512 KB
   if(size<=SIZE(temp))buffer_data=temp;else // if we can fit all data in stack then use it
   {
      Mems<Byte> &buffer=SoundThreadBuffer[thread_index];
      if(size>buffer.elms())buffer.clear().setNum(size); // 'clear' first to avoid unnecessary copy of old elements
      buffer_data=buffer.data();
   }
   setBuffer(buffer_data, size);
   if(_buffer._par.bytes==3) // convert 24-bit to 16-bit
   {
      size=size/3*2; // we now have less data (div by 3 first to avoid overflow, it's OK because size is always divisible by 3 here)
      I16 *d=(I16*)buffer_data, *s=(I16*)(buffer_data+1); // start source from 1 byte higher because we have to ignore first low 8-bits, and just get higher 16-bits
      Int samples=size/SIZE(*d); FREP(samples){d[i]=*s; s=(I16*)(((Byte*)s)+3);} // advance source by 3 bytes
   }
   if(downmix)
   {
      size/=2; // changing from stereo to mono cuts size in half
      if(bit16)
      {
         I16 *d=(I16*)buffer_data; Int samples=size/SIZE(*d); FREP(samples)d[i]=(d[i*2]+d[i*2+1])/2;
      }else
      {
         U8 *d=(U8*)buffer_data; Int samples=size/SIZE(*d); FREP(samples)d[i]=(d[i*2]+d[i*2+1])/2;
      }
   }
   SOUND_API_LOCK_COND;
   alBufferData        (                     _buffer._buffer[buffer], bit16 ? (stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16) : (stereo ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8), buffer_data, size, _buffer._par.frequency); // !! requires 'SoundAPILock' !! 'alBufferData' copies the buffer into its internal memory
   alSourceQueueBuffers(_buffer._source, 1, &_buffer._buffer[buffer]);
   return true;
#elif OPEN_SL
   if(_buffer.player_buffer_queue)
   {
      Int   size=_buffer._data.elms()/2; // we're setting only half of the buffer
      Byte *data=_buffer._data.data(); if(buffer)data+=size;
      setBuffer(data, size);
      SOUND_API_LOCK_COND;
      return (*_buffer.player_buffer_queue)->Enqueue(_buffer.player_buffer_queue, data, size)==SL_RESULT_SUCCESS; // !! requires 'SoundAPILock' !!
   }
   return false;
#else
   return false;
#endif
}
/******************************************************************************/
Bool _Sound::testBuffer(Int thread_index) // this manages locking on its own
{
#if DIRECT_SOUND
   SOUND_API_LOCK_COND; if((_buffer.raw()>=_buffer._par.size/2)==last_buffer) // remember that 'last_buffer' can be 0xFF
   {
      SOUND_API_LOCK_OFF; return setNextBuffer(thread_index);
   }
#elif XAUDIO
   if(_buffer._sv)
   {
      SOUND_API_LOCK_COND;
      XAUDIO2_VOICE_STATE state;
   #if WINDOWS_OLD
     _buffer._sv->GetState(&state);
   #else
     _buffer._sv->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
   #endif
      REP(2-state.BuffersQueued)if(!setNextBuffer(thread_index))return false;
      // unlike OpenAL we don't need to check if buffer is no longer playing due to running out of buffers, because XAudio will auto-play when adding new buffers if it's in play mode - "If the voice is started and has no buffers queued, the new buffer will start playing immediately" https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.ixaudio2sourcevoice.ixaudio2sourcevoice.submitsourcebuffer(v=vs.85).aspx
   }
#elif OPEN_AL
   SOUND_API_LOCK_COND;
   Int processed=0; if(_buffer._source)alGetSourcei(_buffer._source, AL_BUFFERS_PROCESSED, &processed); // !! requires 'SoundAPILock' !!
   if( processed>0)
   {
      REP(processed)
      {
         UInt buffer=0; alSourceUnqueueBuffers(_buffer._source, 1, &buffer); // !! requires 'SoundAPILock' !!
         SOUND_API_LOCK_OFF;
         if(buffer==_buffer._buffer[0]){if(!setBuffer(false, thread_index))return false;}else
         if(buffer==_buffer._buffer[1]){if(!setBuffer(true , thread_index))return false;}else
            return false; // unknown buffer
         SOUND_API_LOCK_SET; // reset lock, needed for both 'alSourceUnqueueBuffers' at the top and '_buffer.playing', '_buffer.play' below
      }
      if(!_buffer.playing())_buffer.play(true); // !! requires 'SoundAPILock' !! continue playing if it stopped (for example ran out of buffers - processed==2)
   }
#elif OPEN_SL
   if(Int processed=AtomicGet(_buffer._processed))
   {
      REP(processed)if(!setNextBuffer(thread_index))return false;
      AtomicSub(_buffer._processed, processed);
      // unlike OpenAL we don't need to check if buffer is no longer playing due to running out of buffers, because OpenSL will auto-play when adding new buffers if it's in play mode, check comments on 'Enqueue' function - https://www.khronos.org/registry/sles/specs/OpenSL_ES_Specification_1.0.1.pdf
   }
#endif
   return true;
}
/******************************************************************************/
Bool _Sound::update(Flt dt) // !! requires 'SoundAPILock' !!
{
   // process fade
   if(Any(_fade_d))
   {
     _fade+=_fade_d*dt;
      if(_fade>1){  _fade=1; _fade_d=0;         }else // don't check for >=1 in case '_fade_d' is <0 (want to fade out) however 'dt'=0
      if(_fade<0){/*_fade=0; _fade_d=0;*/ del();}     // !! requires 'SoundAPILock' !! don't clear because members are already cleared in 'del', don't check for <0 in case '_fade_d' is >0 (want to fade in) however 'dt'=0
      AtomicOr(flag, SOUND_CHANGED_VOLUME);
   }

   // remove
   if(AtomicGet(flag)&SOUND_REMOVE)
   {
      del(); // !! requires 'SoundAPILock' !!
      return false;
   }

   // calculate priority
   if(playing())
   {
      // first calculate based on actual volume
      priority=_volume;
      if(is3D())
      {
         Flt dist=Dist(_pos, Listener.pos());
         if( dist>_range)priority*=_range/dist;
      }

      // add modifiers
      priority+=SoundGroupPriority[volume_group];
      if(_buffer_playing)priority+=SoundPlayPriority; // increase priority for sounds that are already playing
   }else priority=-1; // if it's not playing then set priority below zero (zero is lowest possible 'SoundMinVolume' value, so it will never be played)

   return true;
}
void _Sound::updatePlaying(Int thread_index)
{
   { // brackets to avoid compile errors on GCC
      // create
      if(!_buffer.is())
      {
         SoundStream &stream=T.stream();

         SOUND_API_LOCK_COND;
         if(!_buffer.create(stream.frequency(), stream.bits(), stream.channels(), Ceil2(stream.frequency()*SOUND_TIME/1000), is3D())) // !! requires 'SoundAPILock' !! we need to use 'Ceil2' because we're operating on half buffers everywhere, so we need to make sure that their sizes will be the same
            goto error;

      #if DEL_BUFFER_WHEN_NOT_PLAYING
         AtomicOr(T.flag, SOUND_CHANGED_POS|SOUND_CHANGED_VEL|SOUND_CHANGED_RANGE|SOUND_CHANGED_VOLUME|SOUND_CHANGED_SPEED|SOUND_CHANGED_TIME); // after creating buffer we need to apply: pos/vel/range (in case they were set before but buffer got deleted and need to be reset again), volume and speed because they are affected by global parameters, also reset the time because it's modified when sound is paused
      #endif
      }

      Flt old_time=_time; // remember current '_time' value before checking "SOUND_CHANGED_TIME|SOUND_CHANGING_TIME" flags, this is important to know the state of value before checking for 'SOUND_CHANGING_TIME', to properly detect if it's being changed by the user on another thread

      // process changes
   #if !DEL_BUFFER_WHEN_NOT_PLAYING
      if(!_buffer_playing)AtomicOr(T.flag, SOUND_CHANGED_VOLUME|SOUND_CHANGED_SPEED|SOUND_CHANGED_TIME); // if not playing yet then we need to apply volume and speed because they are affected by global parameters, also reset the time because it's modified when sound is paused
   #endif
      UInt flag=AtomicGet(T.flag), handled=SOUND_CHANGED_POS|SOUND_CHANGED_VEL|SOUND_CHANGED_ORN|SOUND_CHANGED_RANGE|SOUND_CHANGED_VOLUME|SOUND_CHANGED_SPEED|SOUND_CHANGED_TIME|SOUND_CHANGING_TIME; // flags handled here, need to check for SOUND_CHANGED_ORN because of Listener changes
   #if XAUDIO
      flag|=ListenerChanged;
   #endif
      if(flag&handled)
      {
         flag=AtomicDisable(T.flag, handled); // disable these flags first, so in case the user modifies parameters while this function is running, it will be activated again, don't surround this with another 'if' (to avoid extra branching) because most likely they will return handled flags
      #if XAUDIO
         flag|=ListenerChanged;
      #endif
         SOUND_API_LOCK_COND; // below !! requires 'SoundAPILock' !!
      #if XAUDIO // XAudio processes changes together
         if(_is3D)
         {
            if(flag&SOUND_CHANGED_SPEED)setSpeed(); // !! call this first before 'setParams' !!
            if(flag&(SOUND_CHANGED_POS|SOUND_CHANGED_ORN|SOUND_CHANGED_RANGE|SOUND_CHANGED_VEL|SOUND_CHANGED_SPEED)) // need to check for SOUND_CHANGED_ORN because of Listener changes
              _buffer.setParams(T, FlagTest(flag, SOUND_CHANGED_POS|SOUND_CHANGED_ORN|SOUND_CHANGED_RANGE), FlagTest(flag, SOUND_CHANGED_POS|SOUND_CHANGED_VEL|SOUND_CHANGED_SPEED));
         }else
         {
            if(flag&SOUND_CHANGED_SPEED){setSpeed(); _buffer.speed(SoundSpeed(_actual_speed));}
         }
      #else
         if(flag&SOUND_CHANGED_POS   )_buffer.pos  (_pos);
         if(flag&SOUND_CHANGED_VEL   )_buffer.vel  (_vel);
         if(flag&SOUND_CHANGED_RANGE )_buffer.range(_range);
         if(flag&SOUND_CHANGED_SPEED ){setSpeed(); _buffer.speed(SoundSpeed(_actual_speed));}
      #endif
         if(flag&SOUND_CHANGED_VOLUME)setVolume();
         if(flag&SOUND_CHANGED_TIME  )preciseTime(time());
      }

      // buffer data
      if(_buffer_playing) // if already playing
      {
         if(!testBuffer(thread_index))goto error; // check if we need to update playback buffers
         if(!(flag&(SOUND_CHANGING_TIME|SOUND_CHANGED_TIME))) // update time position only if it wasn't modified by the user
         {
            Flt precise_time; {SOUND_API_LOCK_COND; precise_time=preciseTime();} // !! requires 'SoundAPILock' !!
            AtomicCAS(_time, old_time, precise_time);
         }
      }else // start playback
      {
         // set buffer data for both halfs
         if(!setBuffer(false, thread_index) || !setBuffer(true, thread_index))goto error;
        _buffer_playing=true;
         SOUND_API_LOCK_COND; _buffer.play(true); // !! requires 'SoundAPILock' !!
      }
   }
   return;

error:;
   SOUND_API_LOCK_COND; del(); // !! requires 'SoundAPILock' !!
}
/******************************************************************************/
// IO
/******************************************************************************/
#pragma pack(push, 1)
enum
{
   SD_3D     =1<<0,
   SD_LOOP   =1<<1,
   SD_PLAYING=1<<2,
};
struct SoundDesc // 'priority' does not need to be saved because it's always calculated when needed
{
   Byte         flag;
   VOLUME_GROUP volume_group;
   Flt          time, volume, speed, range, fade, fade_d; // use 'time' instead of 'raw' in case sound file format changes (for example frequency or bits which would make 'raw' invalid)
   Vec          pos, vel;
};
#pragma pack(pop)
/******************************************************************************/
Bool _Sound::save(File &f, CChar *path)C // !! Warning: Fade Curve is not saved !!
{
   // copy params to temp object first
   SoundDesc desc;
  _Unaligned(desc.flag        , (is3D() ? SD_3D : 0) | (loop() ? SD_LOOP : 0) | (playing() ? SD_PLAYING : 0));
   Unaligned(desc.volume_group, volume_group);
   Unaligned(desc.time        , time());
   Unaligned(desc.volume      , volume());
   Unaligned(desc.speed       , speed());
   Unaligned(desc.range       , range());
   Unaligned(desc.fade        , _fade);
   Unaligned(desc.fade_d      , _fade_d);
   Unaligned(desc.pos         , pos());
   Unaligned(desc.vel         , vel());

   f.cmpUIntV(0); // version
 C Str &name=T.name(); // !! it is important to check if the sound is being/got deleted after copying all parameters !!
   f.putAsset(name); if(name.is())f<<desc;
   return f.ok();
}
Int _Sound::loadResult(File &f, CChar *path) // this does not call delete on its own, it expects the caller to delete if necessary
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f.getAsset(_name); if(_name.is())
         {
            SoundDesc desc; if(f.getFast(desc) && f.ok())
            {
               Byte flag=Unaligned(desc.flag);
               init(_name, null, FlagTest(flag, SD_3D), Unaligned(desc.volume_group));

               Unaligned(_fade  , desc.fade  );
               Unaligned(_fade_d, desc.fade_d);

               loop  (FlagTest(flag, SD_LOOP));
               time  (Unaligned(desc.time  ));
               volume(Unaligned(desc.volume));
               speed (Unaligned(desc.speed ));
               range (Unaligned(desc.range ));
               pos   (Unaligned(desc.pos   ));
               vel   (Unaligned(desc.vel   ));

               if(FlagTest(flag, SD_PLAYING))play(); // call this last after all parameters have been set
               return SOUND_LOAD_OK;
            }
         }else if(f.ok())return SOUND_LOAD_EMPTY;
      }break;
   }
   return SOUND_LOAD_ERROR;
}
/******************************************************************************/
// SOUND
/******************************************************************************/
Sound::Sound(C Sound &src) : Sound() {T=src;}

Sound& Sound::del()
{
   if(sound)
   {
      if(SoundFunc)AtomicOr(sound->flag, SOUND_REMOVE);
      sound=null;
   }
   return T;
}
Sound& Sound::close()
{
   if(sound)
   {
      if(SoundFunc)
      {
         SyncLocker lock(SoundWait);
         if(!SoundWaited) // check if we already know that the sound is waiting, can check this only after lock, this is important in case there are 2 or more 'close' calls on main thread, for example first call is processed OK, and if next call is made right after the first one, then 'SoundWait' may get locked before the sound thread gets it, which means that it won't be able to signal 'SoundWaiting' event for the 2nd time, because it's still locked due to the first call
         {
            AtomicOr(SoundPause, SOUND_WAIT); // request waiting
            SoundEvent.on(); // wake up sound thread
            SoundWaiting.wait(); // wait until sound thread is waiting
            SoundWaited=true; // we now know that the sound thread is waiting
         }
         {
            SyncLocker locker_memx(SoundMemxLock);
            SOUND_API_LOCK;
            SoundMemx.removeData(sound, true); // !! requires 'SoundAPILock' !!
         }
         AtomicDisable(SoundPause, SOUND_WAIT); // disable waiting request
      }
      sound=null;
   }
   return T;
}
Sound& Sound::_create(C Str &name, SoundCallback *call, Bool is3D, VOLUME_GROUP volume_group) // !! call 'call.del' if not passed down !!
{
   del();
   if((name.is() || call) && SoundFunc)
   {
      SyncLocker locker(SoundMemxLock);
      sound=&SoundMemx.New();
      sound->init(name, call, is3D, volume_group); call=null; // 'call' will be processed inside, don't modify it anymore
   }
   if(call){call->del(); call=null;}
   return T;
}
void Sound::operator=(C Sound &src)
{
   if(this!=&src)
   {
      del();
      if(SoundFunc)
      {
         SyncLocker locker(SoundMemxLock);
         if(src.is())
         {
            sound=&SoundMemx.New();
            if(!sound->init(*src.sound))del();
         }
      }
   }
}
Sound& Sound::create(C Str &name,                        Bool loop, Flt volume, VOLUME_GROUP volume_group) {return name.is() ? _create(name, null, false, volume_group).                      loop(loop).volume(volume) : del();}
Sound& Sound::create(C Str &name, C Vec &pos, Flt range, Bool loop, Flt volume, VOLUME_GROUP volume_group) {return name.is() ? _create(name, null, true , volume_group).pos(pos).range(range).loop(loop).volume(volume) : del();}

Sound& Sound::create(C UID &id,                        Bool loop, Flt volume, VOLUME_GROUP volume_group) {return id.valid() ? _create(_EncodeFileName(id), null, false, volume_group).                      loop(loop).volume(volume) : del();}
Sound& Sound::create(C UID &id, C Vec &pos, Flt range, Bool loop, Flt volume, VOLUME_GROUP volume_group) {return id.valid() ? _create(_EncodeFileName(id), null, true , volume_group).pos(pos).range(range).loop(loop).volume(volume) : del();}

Sound& Sound::create(SoundCallback &call,                        Bool loop, Flt volume, VOLUME_GROUP volume_group) {return _create(S, &call, false, volume_group).                      loop(loop).volume(volume);} // !! call 'call.del' if not passed down !!
Sound& Sound::create(SoundCallback &call, C Vec &pos, Flt range, Bool loop, Flt volume, VOLUME_GROUP volume_group) {return _create(S, &call, true , volume_group).pos(pos).range(range).loop(loop).volume(volume);} // !! call 'call.del' if not passed down !!
/******************************************************************************/
Sound& Sound::stop () {if(sound && SoundFunc)sound->stop (); return T;}
Sound& Sound::pause() {if(sound && SoundFunc)sound->pause(); return T;}
Sound& Sound::play () {if(sound && SoundFunc)sound->play (); return T;}

CChar8*     Sound::codecName()C {return CodecName(codec());}
SOUND_CODEC Sound::codec    ()C {return (sound && SoundFunc) ? sound->codec    () : SOUND_NONE;}
  Bool      Sound::is       ()C {return (sound && SoundFunc) ? sound->is       () : false  ;}
  Bool      Sound::playing  ()C {return (sound && SoundFunc) ? sound->playing  () : false  ;}
C Str&      Sound::name     ()C {return (sound && SoundFunc) ? sound->name     () : S      ;}
  UID       Sound::id       ()C {return (sound && SoundFunc) ? sound->id       () : UIDZero;}
  Long      Sound::size     ()C {return (sound && SoundFunc) ? sound->size     () : 0      ;}
  Long      Sound::samples  ()C {return (sound && SoundFunc) ? sound->samples  () : 0      ;}
  Flt       Sound::length   ()C {return (sound && SoundFunc) ? sound->length   () : 0      ;}
  Int       Sound::channels ()C {return (sound && SoundFunc) ? sound->channels () : 0      ;}
  Int       Sound::frequency()C {return (sound && SoundFunc) ? sound->frequency() : 0      ;}
  Int       Sound::bitRate  ()C {return (sound && SoundFunc) ? sound->bitRate  () : 0      ;}
  Long      Sound::raw      ()C {return (sound && SoundFunc) ? sound->raw      () : 0      ;}
  Long      Sound::sample   ()C {return (sound && SoundFunc) ? sound->sample   () : 0      ;}
  Flt       Sound::timeLeft ()C {return (sound && SoundFunc) ? sound->timeLeft () : 0      ;}
  Flt       Sound::time     ()C {return (sound && SoundFunc) ? sound->time     () : 0      ;}
  Flt       Sound::frac     ()C {return (sound && SoundFunc) ? sound->frac     () : 0      ;}
  Bool      Sound::loop     ()C {return (sound && SoundFunc) ? sound->loop     () :   false;}
  Flt       Sound::volume   ()C {return (sound && SoundFunc) ? sound->volume   () :       0;}
  Flt       Sound::fade     ()C {return (sound && SoundFunc) ? sound->fade     () :       0;}
 FADE_CURVE Sound::fadeCurve()C {return (sound && SoundFunc) ? sound->fadeCurve() : FADE_LINEAR;}
  Flt       Sound::speed    ()C {return (sound && SoundFunc) ? sound->speed    () :       0;}
  Flt       Sound::range    ()C {return (sound && SoundFunc) ? sound->range    () :       0;}
C Vec&      Sound::pos      ()C {return (sound && SoundFunc) ? sound->pos      () : VecZero;}
C Vec&      Sound::vel      ()C {return (sound && SoundFunc) ? sound->vel      () : VecZero;}

Sound& Sound::raw   (  Long raw   ) {if(sound && SoundFunc)sound->raw   (raw   ); return T;}
Sound& Sound::sample(  Long sample) {if(sound && SoundFunc)sound->sample(sample); return T;}
Sound& Sound::time  (  Flt  t     ) {if(sound && SoundFunc)sound->time  (t     ); return T;}
Sound& Sound::frac  (  Flt  f     ) {if(sound && SoundFunc)sound->frac  (f     ); return T;}
Sound& Sound::loop  (  Bool loop  ) {if(sound && SoundFunc)sound->loop  (loop  ); return T;}
Sound& Sound::volume(  Flt  volume) {if(sound && SoundFunc)sound->volume(volume); return T;}
Sound& Sound::speed (  Flt  speed ) {if(sound && SoundFunc)sound->speed (speed ); return T;}
Sound& Sound::range (  Flt  range ) {if(sound && SoundFunc)sound->range (range ); return T;}
Sound& Sound::pos   (C Vec &pos   ) {if(sound && SoundFunc)sound->pos   (pos   ); return T;}
Sound& Sound::vel   (C Vec &vel   ) {if(sound && SoundFunc)sound->vel   (vel   ); return T;}

Sound&             Sound::callback(SoundDataCallback *callback)  {     if(sound && SoundFunc)   sound->_callback=callback; return T;}
SoundDataCallback* Sound::callback(                           )C {return (sound && SoundFunc) ? sound->_callback : null;}

Sound& Sound::fadeCurve        (FADE_CURVE curve ) {if(sound && SoundFunc)sound->fadeCurve        (curve        ); return T;}
Sound& Sound::fadeInFromSilence(Flt fade_duration) {if(sound && SoundFunc)sound->fadeInFromSilence(fade_duration); return T;}
Sound& Sound::fadeIn           (Flt fade_duration) {if(sound && SoundFunc)sound->fadeIn           (fade_duration); return T;}
Sound& Sound::fadeOut          (Flt fade_duration) {if(sound && SoundFunc)sound->fadeOut          (fade_duration); return T;}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Sound::save(File &f, CChar *path)C
{
   SyncLocker locker_memx(SoundMemxLock); // use lock even though we don't need it, to minimize the risk of data being modified during saving
   SOUND_API_LOCK;                        // use lock even though we don't need it, to minimize the risk of data being modified during saving
   if(is())
   {
      f.putBool(true);
      if(!sound->save(f, path))return false;
   }else
   {
      f.putBool(false);
   }
   return f.ok();
}
/******************************************************************************/
Bool Sound::load(File &f, CChar *path)
{
   del(); // always delete, because for loading we need to modify a new '_Sound'
   if(!f.getBool())return f.ok(); // no sound
   if(SoundFunc) // can load sounds only with 'SoundMemx' available
   {
      SyncLocker locker(SoundMemxLock);
      sound=&SoundMemx.New();
      switch(sound->loadResult(f, path))
      {
         case SOUND_LOAD_OK   :        return true;
         case SOUND_LOAD_EMPTY: del(); return true;
         default              : del(); break; // SOUND_LOAD_ERROR
      }
   }
   return false;
}
/******************************************************************************/
#if OPEN_SL
// !! this can be called only inside 'Listener' on sound thread !! because it's called only there, it already had 'SoundAPILock' lock applied so we don't need to call it again, since it's the sound thread then we can use 'SoundMemxPlaying'
void EmulateSound3D() {REPAO(SoundMemxPlaying)->_buffer.emulate3D();} // !! requires 'SoundAPILock' !!
#endif
// following can be called on the main thread, because of that we can't use 'SoundMemxPlaying'
void VolumeSound() {SyncLocker locker_memx(SoundMemxLock); REPA(SoundMemx)AtomicOr(SoundMemx[i].flag, SOUND_CHANGED_VOLUME);}
void  SpeedSound() {SyncLocker locker_memx(SoundMemxLock); REPA(SoundMemx)AtomicOr(SoundMemx[i].flag, SOUND_CHANGED_SPEED );}

static Int CompareSound(C _Sound &a, C _Sound &b)
{
   return Compare(b.priority, a.priority); // compare reversed so that we start with highest priority
}
/******************************************************************************/
static void UpdatePlaying(_Sound* &sound, Ptr user, Int thread_index) {sound->updatePlaying(thread_index);}
#if UPDATE_2X
static Bool UpdateStep;
#endif
static Bool UpdateSound2(Thread &thread)
{
   SoundEvent.wait(SOUND_TIMER/(UPDATE_2X ? 2 : 1));

again:
   if(AtomicGet(SoundPause))
   {
      Int        state=AtomicGet(SoundPause);
      Bool want_paused=FlagTest(state, SOUND_WANT_PAUSED),
                paused=FlagTest(state, SOUND_PAUSED     );
      if(  want_paused!=paused)
      {
         SOUND_API_LOCK;
         if(want_paused){REPAO(SoundMemxPlaying)->_buffer.pause(    ); AtomicOr     (SoundPause, SOUND_PAUSED);                          } // !! requires 'SoundAPILock' !!   pause all playing sounds
         else           {REPAO(SoundMemxPlaying)->_buffer.play (true); AtomicDisable(SoundPause, SOUND_PAUSED); SoundTime=Time.curTime();} // !! requires 'SoundAPILock' !! unpause all playing sounds
      }
      if(state&SOUND_WAIT) // if want to wait temporarily
      {
         SoundWaiting.on(); // message other thread that we will wait now
         SyncLocker lock(SoundWait); // since other thread already is inside 'SoundWait', this will continue only after other thread finished
         SoundWaited=false; // sound thread is no longer waiting, can clear this only after lock
      }
      if(want_paused)
      {
         SoundEvent.wait(); // wait for unpause as long as it takes
      #if HAS_THREADS
         goto again; // if we have threads then it means we have waited and we can just check again
      #else
         return true; // if we haven't waited then we need to return so we don't update anything since we're paused
      #endif
      }
   }

#if UPDATE_2X
   if(UpdateStep^=1) // we have to update every 2nd step, because this function is called 2x more frequently
#endif
   {
      UpdateMusic(); // !! do not surround music by any locks, because it doesn't need any, it operates only on 'Sound' which was designed to don't require any locks !!
      SoundMemxPlaying.clear();
      Int max_concurrent=SoundMaxConcurrent();
      Flt min_vol       =SoundMinVolume    ();
      Dbl time=Time.curTime(); Flt dt=time-SoundTime; SoundTime=time; // get time that passed since the last update

      SyncLockerEx locker_memx(SoundMemxLock);
   #if !SOUND_API_THREAD_SAFE
      SyncLockerEx locker_api (SoundAPILock); SoundAPILockDo=false; // disable 'SoundAPILock' because we're already covered by the lock here
   #endif
      REPA(SoundMemx)if(!SoundMemx[i].update(dt))SoundMemx.removeValid(i, true); // !! requires 'SoundAPILock' !!
      SoundMemx.sort(CompareSound);

      // detect sounds which should be playing
      if(!SoundAPI)max_concurrent=0;else if(max_concurrent<0)max_concurrent=SoundMemx.elms();else MIN(max_concurrent, SoundMemx.elms()); // needs to be done after 'SoundMemxLock' and after removing sounds from 'SoundMemx'
      Int i=0; for(; i<max_concurrent; i++) // go from the start with highest priority
      {
        _Sound &sound=SoundMemx[i];
         if(sound.priority<=min_vol)break;
         SoundMemxPlaying.add(&sound);
      }
      for(; i<SoundMemx.elms(); i++) // all remaining sounds should not be played
      {
        _Sound &sound=SoundMemx[i]; if(!sound.deleted)
         {
            if(sound._buffer_playing)
            {
               // remember time position to restart it later, do this before adjusting other members in case they affect the result
               Flt time=sound._time; // remember this before checking flags
               if(!(AtomicGet(sound.flag)&(SOUND_CHANGED_TIME|SOUND_CHANGING_TIME)))
                  AtomicCAS(sound._time, time, sound.preciseTime()); // !! requires 'SoundAPILock' !!

               sound._buffer_playing=false;
            #if DEL_BUFFER_WHEN_NOT_PLAYING
               sound.last_buffer=0xFF;
               sound._buffer.del(); // !! requires 'SoundAPILock' !! delete the buffer to free up memory
            #else
               ..
            #endif
            }else
            if(sound.playing() && !sound.loop()) // update time but only if the sound is not looped, if it's looped then for simplicity we skip this
            {  // update time over here because above it is already set from precise value
               if(AtomicGet(sound.flag)&SOUND_CHANGED_SPEED) // check if speed was changed, which affects time updates
               {
                  AtomicDisable(sound.flag, SOUND_CHANGED_SPEED); // disable flag first, so in case the user modifies parameters while this function is running, it will be activated again
                  sound.setSpeed();
               }
               Flt time=sound._time; AtomicCAS(sound._time, time, time+sound._actual_speed*dt); // update time only if it wasn't changed on another thread, normally we should also set SOUND_CHANGED_TIME flag, however to avoid doing it everytime, since the sound is paused, then we just apply that flag once when the sound is unpaused. This is a safe change since it's a small relative change using 'AtomicCAS' which can be performed even if time was modified on another thread.
               if(sound.time()>=sound.length())sound.del(); // !! requires 'SoundAPILock' !! if reached the end, then delete it
            }
         }
      }
      locker_memx.off(); // finished operating on 'SoundMemx', further sound update will be done using 'SoundMemxPlaying'

      if(SoundMemxPlaying.elms())
      {
      #if XAUDIO
         ListenerChanged=
      #endif
            Listener.updateNoLock(); // !! requires 'SoundAPILock' !!

      #if HAS_THREADS
         Int threads =Mid(SoundMaxThreads(), 1, Cpu.threads())-1; // since we always create 'SoundThread' (this thread) then we need one less for 'SoundThreads', because we will use this one too
         if( threads!=SoundThreads.threads())SoundThreads.create(false, threads, SoundThread.priority(), "EE.Sound #");
      #endif

         Int max_threads=SoundThreads.threads1(); // add 1 extra thread to process on this one too !! do not minimize here with 'SoundMemxPlaying.elms()' because we need to allocate 'SoundThreadBuffer' below for all threads, as we don't know which ones are going to wake up !!
      #if OPEN_AL
         if( max_threads>SoundThreadBuffer.elms())SoundThreadBuffer.setNum(max_threads);
      #endif

      #if !SOUND_API_THREAD_SAFE
            SoundAPILockDo=(SoundMemxPlaying.elms()>1 && max_threads>1); // we need to do locking if we're going to process multiple sounds on multiple threads
         if(SoundAPILockDo)locker_api.off(); // release from this thread so other threads can capture it
      #endif

         SoundThreads.process1(SoundMemxPlaying, UpdatePlaying, null, max_threads);

      #if !SOUND_API_THREAD_SAFE
         if(SoundAPILockDo)locker_api.on(); // re-enable for this thread because it's needed for calls done below (Listener)
      #endif
      }

      Listener.commitNoLock(); // !! requires 'SoundAPILock' !! call this always, not only when 'SoundMemxPlaying.elms' because on XAudio some operations are deferred
   }

#if WINDOWS_NEW || !APPLE && !ANDROID && (DIRECT_SOUND || OPEN_AL)
   if(SoundRecords.elms())
   {
      SyncLocker locker(SoundAPILock); // sound record methods always require lock, read why in the sound record class
      REPAO(SoundRecords)->updateNoLock(); // !! requires 'SoundAPILock' !!
   }
#endif
   return true;
}
void ShutSound2()
{
   SoundThread.stop(); ResumeSound(); SoundEvent.on(); // resume to avoid waiting forever, enable event so the 'SoundThread' can wake up immediately
   SoundThread.del ();
#if OPEN_AL
   SoundThreadBuffer.del(); // delete this after deleting the thread
#endif

   SyncLocker locker_memx(SoundMemxLock);
   SyncLocker locker_api (SoundAPILock );
   SoundMemxPlaying.del();
   SoundMemx       .del();
}
void InitSound2()
{
   SoundGroupPriority[VOLUME_MUSIC]=1; // increase priority for Music
   SoundTime=Time.curTime();
   SoundThread.create(UpdateSound2, null, 2, false, "EE.Sound"); // here parameter "2" is thread priority
}
void UpdateSound()
{
   SoundEvent.on();
}
/******************************************************************************/
void PauseSound() // in order to work on 'SoundMemxPlaying' we would need to delete the thread
{
#if XAUDIO
   if(XAudio)XAudio->StopEngine();
#else
   AtomicOr(SoundPause, SOUND_WANT_PAUSED); SoundEvent.on(); // wake up the thread immediately to perform the changes
#endif
}
void ResumeSound()
{
#if XAUDIO
   if(XAudio)XAudio->StartEngine();
#else
   AtomicDisable(SoundPause, SOUND_WANT_PAUSED); SoundEvent.on(); // wake up the thread immediately to perform the changes
#endif
}
/******************************************************************************/
Bool PlayingAnySound() {return SoundMemxPlaying.elms()>0;}
/******************************************************************************/
static void SoundPlay2D(C Str &name, SoundCallback *call, Flt volume, VOLUME_GROUP volume_group, Flt speed) // !! call 'call.del' if not passed down !!
{
   if(SoundAPI) // test for 'SoundAPI' because there's no point in creating dummy sounds if they won't be played
   {
      SyncLocker locker(SoundMemxLock);
     _Sound &sound=SoundMemx.New(); FlagEnable(sound.flag, SOUND_NO_REF);
      sound.init(name, call, false, volume_group); call=null; // 'call' will be processed inside, don't modify it anymore
      sound.volume(volume); sound.speed(speed);
      sound.play();
   }
   if(call){call->del(); call=null;}
}
static void SoundPlay3D(C Str &name, SoundCallback *call, C Vec &pos, Flt range, Flt volume, VOLUME_GROUP volume_group, Flt speed) // !! call 'call.del' if not passed down !!
{
   if(SoundAPI) // test for 'SoundAPI' because there's no point in creating dummy sounds if they won't be played
   {
      SyncLocker locker(SoundMemxLock);
     _Sound &sound=SoundMemx.New(); FlagEnable(sound.flag, SOUND_NO_REF);
      sound.init(name, call, true, volume_group); call=null; // 'call' will be processed inside, don't modify it anymore
      sound.volume(volume); sound.speed(speed); sound.pos(pos); sound.range(range);
      sound.play();
   }
   if(call){call->del(); call=null;}
}

void SoundPlay(C Str         &name,                        Flt volume, VOLUME_GROUP volume_group, Flt speed) {if(name .is())SoundPlay2D(name               ,  null,             volume, volume_group, speed);}
void SoundPlay(C Str         &name, C Vec &pos, Flt range, Flt volume, VOLUME_GROUP volume_group, Flt speed) {if(name .is())SoundPlay3D(name               ,  null, pos, range, volume, volume_group, speed);}
void SoundPlay(C UID         &id  ,                        Flt volume, VOLUME_GROUP volume_group, Flt speed) {if(id.valid())SoundPlay2D(_EncodeFileName(id),  null,             volume, volume_group, speed);}
void SoundPlay(C UID         &id  , C Vec &pos, Flt range, Flt volume, VOLUME_GROUP volume_group, Flt speed) {if(id.valid())SoundPlay3D(_EncodeFileName(id),  null, pos, range, volume, volume_group, speed);}
void SoundPlay(SoundCallback &call,                        Flt volume, VOLUME_GROUP volume_group, Flt speed) {              SoundPlay2D(S                  , &call,             volume, volume_group, speed);}
void SoundPlay(SoundCallback &call, C Vec &pos, Flt range, Flt volume, VOLUME_GROUP volume_group, Flt speed) {              SoundPlay3D(S                  , &call, pos, range, volume, volume_group, speed);}
/******************************************************************************/
}
/******************************************************************************/
