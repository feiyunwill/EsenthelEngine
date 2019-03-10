/******************************************************************************

   Use 'MusicManager's 'Music' and 'Ambient' to play different 'Playlist's.

   A 'Playlist' is a list of multiple songs.

/******************************************************************************/
const_mem_addr struct Playlist // List of Music Tracks !! must be stored in constant memory address !!
{
   // get
   Int songs (     )C {return _songs.elms();} // get number           of      songs in this playlist
   Str song  (Int i)C;                        // get the file name    of i-th song  in this playlist,  null     on fail
   UID songID(Int i)C;                        // get the file name ID of i-th song  in this playlist, 'UIDZero' on fail

   // operations
   void operator+=(C Str &name); // add    song to   playlist, 'name'=song file name
   void operator+=(C UID &id  ); // add    song to   playlist, 'id'  =song file name ID
   void operator-=(C Str &name); // remove song from playlist, 'name'=song file name
   void operator-=(C UID &id  ); // remove song from playlist, 'id'  =song file name ID

#if EE_PRIVATE
   Int globalSong(Int song, Bool shuffle, Randomizer &random); // get global song index, from desired local/playlist song
   Int   nextSong(          Bool shuffle, Randomizer &random); // get global song index
   Int   prevSong(          Bool shuffle, Randomizer &random); // get global song index
#endif

  ~Playlist();
   Playlist();

#if !EE_PRIVATE
private:
#endif
   Int       _cur;
   Memc<Int> _songs;
};
/******************************************************************************/
struct MusicManager
{
   Bool       shuffle   ; // if select songs from Playlist in random order, default=true
   FADE_CURVE fade_curve; // curve used for song crossfade                , default=FADE_LINEAR
   Flt        fade_in   , // fade in  time for starting  new songs        , default=0.5
              fade_out  , // fade out time for finishing old songs        , default=3.0
              time_reset; // minimum  time left needed to play a song again from its last position instead of playing it from the start, default=10, for example: if song "A" was playing, and user manually switched to song "B", the last position of "A" song is remembered before switching to "B" song, then if user switches back to "A" song, the engine first checks if last position of "A" song has at least 'time_reset' seconds before the end of the song, if yes then song "A" is played from its last remembered position, if not then it is played from the start of the song

   UID (*select_song)(C UID &next); // pointer to custom function (may be null) called when next song needs to be selected, return 'UID' of the song you wish to play, or 'UIDZero' for no song. 'next'=next song ID queued in the history, set to 'UIDZero' if there's no next song available. If this function is not specified then song will be selected based on active playlist. !! Warning: this may get called on a secondary thread !!

   // get
   Str       name    ()C;                        // get     current song file name
   UID       id      ()C;                        // get     current song file name ID
   Flt       frac    ()C;   void frac(Flt frac); // get/set current song fraction position, 0..1
   Flt       time    ()C;   void time(Flt time); // get/set current song time     position, 0..length
   Flt       length  ()C;                        // get     current song length in seconds
   Flt       fade    ()C;                        // get     current song fade value
   Playlist* playlist()C {return _playlist;}     // get     active  playlist

   // set
   void set (const_mem_addr Playlist *playlist             );                         // set active   playlist, this playlist will be used after current song finishes playing
   void set (const_mem_addr Playlist &playlist             ) {set (&playlist      );} // set active   playlist, this playlist will be used after current song finishes playing
   void play(const_mem_addr Playlist *playlist, Int song=-1);                         // set and play playlist with 'song' i-th song in the playlist (-1=last played song in the playlist)
   void play(const_mem_addr Playlist &playlist, Int song=-1) {play(&playlist, song);} // set and play playlist with 'song' i-th song in the playlist (-1=last played song in the playlist)

   void play(C Str &song_name); // play song by its file name   , without changing current playlist
   void play(C UID &song_id  ); // play song by its file name ID, without changing current playlist

   void stop(); // stop any playback, without changing current playlist

   void next(); // skip to the next     song from active playlist
   void prev(); // skip to the previous song from active playlist

   void maxHistory(Int max_history); // set max number of songs to store in the history for 'prev/next' methods, use <=0 to disable history

   // data callback
   MusicManager& callback(SoundDataCallback *callback);   SoundDataCallback* callback()C {return _callback;} // set/get data callback, it will be called every time a new portion of data is processed by the sound

#if EE_PRIVATE
   void del     ();
   void playSong(Int global_song);

   // lock required
   Flt  lockedTimeLeft()C; // get current song remaining time
   void lockedUpdate  ();
   void swap    ();
   void storePos(Bool i);
   void fadeIn  (Bool i);
   void fadeOut (Bool i);
   void del     (Bool i);
   void set     (Bool i, Int global_song);
#endif

#if !EE_PRIVATE
private:
#endif
   Int                _song [2], _history_max, _history_pos;
   Sound              _sound[2];
   VOLUME_GROUP       _volume_group;
   Playlist          *_playlist;
   SoundDataCallback *_callback;
   Randomizer         _random;
   Memc<Int>          _history;

   explicit MusicManager(VOLUME_GROUP volume_group);
}extern
   Music  , // Music   MusicManager with VOLUME_MUSIC   sound VOLUME_GROUP
   Ambient; // Ambient MusicManager with VOLUME_AMBIENT sound VOLUME_GROUP
/******************************************************************************/
#if EE_PRIVATE
void   ShutMusic();
void UpdateMusic();
#endif
/******************************************************************************/
