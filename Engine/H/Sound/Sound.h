/******************************************************************************

   Use 'SoundVolume' to set  volume for different sound groups
   Use 'SoundPlay'   to play custom sounds
   Use 'Sound'       to play custom sounds and control them later
   Use 'Listener'    to set  3D listener parameters which receives 3D sounds

/******************************************************************************/
enum VOLUME_GROUP : Byte // Sound Volume Group
{
   VOLUME_FX     , // Sound Effects
   VOLUME_MUSIC  , // Music
   VOLUME_AMBIENT, // Ambient Surroundings
   VOLUME_VOICE  , // Voices
   VOLUME_UI     , // User Interface
   VOLUME_NUM    , // number of volume groups
};
/******************************************************************************/
struct SoundVolumeClass
{
   void fx     (Flt v);   Flt fx     ()C {return _v[VOLUME_FX     ];} // set/get fx      volume (0..1)
   void music  (Flt v);   Flt music  ()C {return _v[VOLUME_MUSIC  ];} // set/get music   volume (0..1)
   void ambient(Flt v);   Flt ambient()C {return _v[VOLUME_AMBIENT];} // set/get ambient volume (0..1)
   void voice  (Flt v);   Flt voice  ()C {return _v[VOLUME_VOICE  ];} // set/get voice   volume (0..1)
   void ui     (Flt v);   Flt ui     ()C {return _v[VOLUME_UI     ];} // set/get UI      volume (0..1)
   void global (Flt v);   Flt global ()C {return _v[VOLUME_NUM    ];} // set/get global  volume (0..1)

#if EE_PRIVATE
   void update(); // update sound volumes
#endif

#if !EE_PRIVATE
private:
#endif
   Flt _v[VOLUME_NUM+1]; // +1 to make room for global

   SoundVolumeClass();
}extern
   SoundVolume; // Main Sound Volume Control
/******************************************************************************/
struct ListenerClass
{
#if EE_PRIVATE
   Bool create();
   UInt updateNoLock(); // update listener
   void commitNoLock(); // update all 3D sounds parameters, only after this call all 3D sound changes are updated, this should be called once per frame
#endif

   // get
 C Vec& pos  ()C {return _orn.pos    ;} // get listener position
 C Vec& dir  ()C {return _orn.dir    ;} // get listener forward direction
 C Vec& up   ()C {return _orn.perp   ;} // get listener up      direction
   Vec  right()C {return _orn.cross();} // get listener right   direction
 C Vec& vel  ()C {return _vel        ;} // get listener velocity

   // set
   ListenerClass& pos(C Vec &pos                        ); // set listener position
   ListenerClass& orn(C Vec &dir, C Vec &up=Vec(0, 1, 0)); // set listener orientation, 'dir'=forward direction (must be normalized), 'up'=up direction (must be normalized)
   ListenerClass& vel(C Vec &vel                        ); // set listener velocity

#if !EE_PRIVATE
private:
#endif
   OrientP _orn;
   Vec     _vel;
   Int     _flag;

   ListenerClass();
}extern
   Listener; // Main 3D Sound Listener Control
/******************************************************************************/
enum FADE_CURVE : Byte
{
   FADE_LINEAR,
   FADE_SQRT  ,
   FADE_EQUAL_POWER=FADE_SQRT,
};
#if EE_PRIVATE
enum SOUND_FLAG
{
   SOUND_CHANGED_POS   =1<<0,
   SOUND_CHANGED_VEL   =1<<1,
   SOUND_CHANGED_ORN   =1<<2, // used for Listener
   SOUND_CHANGED_RANGE =1<<3,
   SOUND_CHANGED_VOLUME=1<<4,
   SOUND_CHANGED_SPEED =1<<5,
   SOUND_CHANGED_TIME  =1<<6,
   SOUND_CHANGING_TIME =1<<7,
   SOUND_REMOVE        =1<<8, // this can be set only if no 'Sound' references a '_Sound' anymore
   SOUND_NO_REF        =1<<9, // if there's no external reference to this sound
 //SOUND_CHANGED_PAN   =, this is not used
 //SOUND_CHANGED_PLAY  =, this is not needed since it's always checked
};
const_mem_addr struct _Sound // can be moved however 'memAddressChanged' needs to be called afterwards
{
   Bool               _playing, _buffer_playing, _loop, _is3D, stream_loaded, deleted;
   Byte                last_buffer, kill;
   VOLUME_GROUP        volume_group;
   FADE_CURVE         _fade_curve;
   Int                 flag;
   Long                raw_pos;
   Flt                _volume, _speed, _actual_speed, _range, _time, _fade, _fade_d, priority;
   Vec                _pos, _vel;
   Str                _name;
   SoundDataCallback *_callback;
   SoundStream        _stream;
   SoundBuffer        _buffer;

   SoundStream& stream();
 C SoundStream& stream()C {return ConstCast(T).stream();}

   void zero();
   void del ();
   void init(C Str &name, const_mem_addr SoundCallback *call, Bool is3D, VOLUME_GROUP volume_group);
   Bool init(C _Sound &src);

   // stop / play
   void stop ();
   void pause();
   void play ();

   // get / set
                                 Bool noRef    ()C {return FlagTest(AtomicGet(flag), SOUND_NO_REF);} // if there's no external reference to this sound
                                 Bool is       ()C;                   // if created
                                 Bool is3D     ()C {return _is3D   ;} // if is 3D
                                 Bool playing  ()C {return _playing;} // if playing
                          SOUND_CODEC codec    ()C;                   //     get source codec
                               C Str& name     ()C;                   //     get source file name
                                 UID  id       ()C;                   //     get source file name ID
                                 Long size     ()C;                   //     get source raw  size   in bytes
                                 Flt  length   ()C;                   //     get source time length in seconds
                                 Long samples  ()C;                   //     get source number of samples
                                 Int  channels ()C;                   //     get source number of channels
                                 Int  frequency()C;                   //     get source frequency
                                 Int  bitRate  ()C;                   //     get source bit rate
                                 Flt  timeLeft ()C;                   //     get sound remaining time   , 0..length (), "length()-time()"
   void raw   (  Long raw   );   Long raw      ()C;                   // set/get sound raw      position, 0..size   ()
   void sample(  Long sample);   Long sample   ()C;                   // set/get sound sample   position, 0..samples()
   void time  (  Flt  t     );   Flt  time     ()C {return _time   ;} // set/get sound time     position, 0..length ()
   void frac  (  Flt  f     );   Flt  frac     ()C;                   // set/get sound fraction position, 0..1
   void loop  (  Bool loop  );   Bool loop     ()C {return _loop   ;} // set/get sound looping          , true/false
   void volume(  Flt  volume);   Flt  volume   ()C {return _volume ;} // set/get sound volume           , 0..1
   void speed (  Flt  speed );   Flt  speed    ()C {return _speed  ;} // set/get sound speed            , 0..3, default=1
   void range (  Flt  range );   Flt  range    ()C {return _range  ;} // set/get sound 3D range         , 0..Inf
   void pos   (C Vec &pos   ); C Vec& pos      ()C {return _pos    ;} // set/get sound 3D position
   void vel   (C Vec &vel   ); C Vec& vel      ()C {return _vel    ;} // set/get sound 3D velocity

   void preciseRaw (Long raw);   Long preciseRaw ()C; // set/get sound raw      position, 0..size  ()
   void preciseTime(Flt  t  );   Flt  preciseTime()C; // set/get sound time     position, 0..length()
   void preciseFrac(Flt  f  );   Flt  preciseFrac()C; // set/get sound fraction position, 0..1

   // operations
   Flt  actualSpeed  ()C;
   void setVolume    ();
   void setSpeed     ();
   Bool update       (Flt dt);
   void updatePlaying(Int thread_index);
   void memAddressChanged();

   // fade
                                                Flt        fade     ()C {return _fade      ;} //     get current fade value
   void fadeCurve        (FADE_CURVE curve );   FADE_CURVE fadeCurve()C {return _fade_curve;} // set/get fade curve
   void fadeInFromSilence(Flt fade_duration);
   void fadeIn           (Flt fade_duration);
   void fadeOut          (Flt fade_duration);

   // io
   Bool save      (File &f, CChar *path=null)C; // 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Int  loadResult(File &f, CChar *path=null) ; // 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path)

   // stream
   void setBuffer    (Byte *buffer, Int size);
   Bool setBuffer    (Bool  buffer, Int thread_index);
   Bool setNextBuffer(Int thread_index) {return setBuffer(!last_buffer, thread_index);} // remember that 'last_buffer' can be 0xFF
   Bool testBuffer   (Int thread_index);

 ~_Sound() {del();}
  _Sound() {stream_loaded=deleted=false; flag=0; zero();}
   NO_COPY_CONSTRUCTOR(_Sound);
};
#endif
/******************************************************************************/
struct Sound
{
   // manage
#if EE_PRIVATE
   Sound& _create(C Str &name, const_mem_addr SoundCallback *call, Bool is3D, VOLUME_GROUP volume_group);
#endif
   Sound& del   ();
   Sound& close (); // delete and wait until file handle is released, this method is slower than 'del', use it only if you need to modify the sound file
   Sound& create(               C Str         &name,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create             sound, 'name'=sound file name   ,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& create(               C UID         &id  ,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create             sound, 'id'  =sound file name ID,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& create(const_mem_addr SoundCallback &call,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create             sound, 'call'=sound call back   ,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& create(               C Str         &name, C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create          3D sound, 'name'=sound file name   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& create(               C UID         &id  , C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create          3D sound, 'id'  =sound file name ID, 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& create(const_mem_addr SoundCallback &call, C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX);                                                                      // create          3D sound, 'call'=sound call back   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (               C Str         &name,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(name,             loop, volume, volume_group).play();} // create and play    sound, 'name'=sound file name   ,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (               C UID         &id  ,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(id  ,             loop, volume, volume_group).play();} // create and play    sound, 'id'  =sound file name ID,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (const_mem_addr SoundCallback &call,                          Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(call,             loop, volume, volume_group).play();} // create and play    sound, 'call'=sound call back   ,                                                                       , 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (               C Str         &name, C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(name, pos, range, loop, volume, volume_group).play();} // create and play 3D sound, 'name'=sound file name   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (               C UID         &id  , C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(id  , pos, range, loop, volume, volume_group).play();} // create and play 3D sound, 'id'  =sound file name ID, 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)
   Sound& play  (const_mem_addr SoundCallback &call, C Vec &pos, Flt range=1, Bool loop=false, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX) {return create(call, pos, range, loop, volume, volume_group).play();} // create and play 3D sound, 'call'=sound call back   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'loop'=if sound is looped, 'volume'=sound volume (0..1)

   // stop / play
   Sound& stop (); // stop
   Sound& pause(); // pause
   Sound& play (); // play

   // get / set
                                   Bool is       ()C; // if created
                                   Bool playing  ()C; // if playing
                            SOUND_CODEC codec    ()C; //     get source codec
                                CChar8* codecName()C; //     get source codec name
                                 C Str& name     ()C; //     get source file  name
                                   UID  id       ()C; //     get source file  name ID
                                   Long size     ()C; //     get source raw   size   in bytes
                                   Flt  length   ()C; //     get source time  length in seconds
                                   Long samples  ()C; //     get source number of samples
                                   Int  channels ()C; //     get source number of channels
                                   Int  frequency()C; //     get source frequency
                                   Int  bitRate  ()C; //     get source bit rate
                                   Flt  timeLeft ()C; //     get sound remaining time     , 0..length (), "length()-time()"
   Sound& raw   (  Long raw   );   Long raw      ()C; // set/get sound raw      position  , 0..size   ()
   Sound& sample(  Long sample);   Long sample   ()C; // set/get sound sample   position  , 0..samples()
   Sound& time  (  Flt  t     );   Flt  time     ()C; // set/get sound time     position  , 0..length ()
   Sound& frac  (  Flt  f     );   Flt  frac     ()C; // set/get sound fraction position  , 0..1
   Sound& loop  (  Bool loop  );   Bool loop     ()C; // set/get sound looping            , true/false
   Sound& volume(  Flt  volume);   Flt  volume   ()C; // set/get sound volume             , 0..1
   Sound& speed (  Flt  speed );   Flt  speed    ()C; // set/get sound speed              , 0..3, default=1
   Sound& range (  Flt  range );   Flt  range    ()C; // set/get sound 3D range multiplier, 0..Inf
   Sound& pos   (C Vec &pos   ); C Vec& pos      ()C; // set/get sound 3D position
   Sound& vel   (C Vec &vel   ); C Vec& vel      ()C; // set/get sound 3D velocity

   // volume fade
                                                  Flt        fade     ()C; //     get current fade value
   Sound& fadeCurve        (FADE_CURVE curve );   FADE_CURVE fadeCurve()C; // set/get fade curve
   Sound& fadeInFromSilence(Flt fade_duration);                            // perform volume fade-in  from silence              to max  volume
   Sound& fadeIn           (Flt fade_duration);                            // perform volume fade-in  from current volume level to max  volume
   Sound& fadeOut          (Flt fade_duration);                            // perform volume fade-out from current volume level to zero volume

   // data callback
   Sound& callback(SoundDataCallback *callback);   SoundDataCallback* callback()C; // set/get data callback, it will be called every time a new portion of data is processed by the sound

   // io
   Bool save(File &f, CChar *path=null)C; // 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

           ~Sound() {del();}
            Sound() {sound=null;}
            Sound(C Sound &src);
   void operator=(C Sound &src);

private:
#if EE_PRIVATE
  _Sound *sound;
#else
   Ptr sound;
#endif
};
/******************************************************************************/
Bool CacheSound(C Str &name); // cache sound file into memory so it won't be played every time from the disk, usage of this function is optional, you can call it at some initialization stage of your game, false on fail
Bool CacheSound(C UID &id  ); // cache sound file into memory so it won't be played every time from the disk, usage of this function is optional, you can call it at some initialization stage of your game, false on fail

void SoundPlay(               C Str         &name,                          Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play    sound, 'name'=sound file name                                                                           , 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)
void SoundPlay(               C UID         &id  ,                          Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play    sound, 'id'  =sound file name ID                                                                        , 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)
void SoundPlay(const_mem_addr SoundCallback &call,                          Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play    sound, 'call'=sound call back                                                                           , 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)
void SoundPlay(               C Str         &name, C Vec &pos, Flt range=1, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play 3D sound, 'name'=sound file name   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)
void SoundPlay(               C UID         &id  , C Vec &pos, Flt range=1, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play 3D sound, 'id'  =sound file name ID, 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)
void SoundPlay(const_mem_addr SoundCallback &call, C Vec &pos, Flt range=1, Flt volume=1, VOLUME_GROUP volume_group=VOLUME_FX, Flt speed=1); // play 3D sound, 'call'=sound call back   , 'pos'=sound position in World, 'range'=sound range multiplier (0..Inf), 'volume'=sound volume (0..1), 'speed'=sound speed (0..3)

Flt SoundMinVolume    ();   void SoundMinVolume    (Flt volume); // get/set minimum volume required to play a sound            , default=0.02, if volume of a sound is less than the specified value then it will be skipped for faster processing
Int SoundMaxConcurrent();   void SoundMaxConcurrent(Int max   ); // get/set maximum number of concurrent sounds                , default=16  , if number of sounds exceeds the specified limit then sounds with lower priority will be skipped for faster processing, specifying negative value (-1) is the same as allowing unlimited number of concurrent sounds
Int SoundMaxThreads   ();   void SoundMaxThreads   (Int max   ); // get/set maximum number of threads used for sound processing, default=1
/******************************************************************************/
#if EE_PRIVATE
extern SyncLock SoundAPILock; //    Sound API Lock
extern Bool     SoundAPI    , // if Sound API is available
                SoundFunc   ; // if we can access Sound functions

void    InitSound  ();
void    ShutSound  ();
void    InitSound2 ();
void    ShutSound2 ();
void  UpdateSound  ();
void   PauseSound  ();
void  ResumeSound  ();
void  VolumeSound  ();
void EmulateSound3D();
void   SpeedSound  ();
Bool PlayingAnySound();

#define MAX_SOUND_SPEED 2
inline Flt SoundSpeed(Flt speed) {return Mid(speed, 0.0f, (Flt)MAX_SOUND_SPEED);}

#if HAS_THREADS
   #define SOUND_TIMER       26                                // 26 ms which is 38.5 Hz(fps), recommended value to be between 17ms(58.8Hz fps) .. 34ms(29.4Hz fps), also the callback will be triggered at least once per frame (due to 'SoundEvent' being triggered at the end of each frame to immediately process any changes), shorter timers result in smaller memory usage at the cost of additional overhead on the CPU
   #define SOUND_TIME        (SOUND_TIMER*2*2*MAX_SOUND_SPEED) // 2 (2 half buffers) * 2 (safety due to sounds being started at different times) * MAX_SOUND_SPEED
   #define SOUND_TIME_RECORD (SOUND_TIMER*2*2)                 // 2 (2 half buffers) * 2 (safety due to sounds being started at different times), this doesn't need MAX_SOUND_SPEED because sounds are always recorded with speed=1
#else
   #define SOUND_TIMER         50
   #define SOUND_TIME        1200 // when there are no threads available, set a big sound buffer, to allow some tolerance for pauses during loading
   #define SOUND_TIME_RECORD  500 // when there are no threads available, set a big sound buffer, to allow some tolerance for pauses during loading
#endif

#endif
/******************************************************************************/
