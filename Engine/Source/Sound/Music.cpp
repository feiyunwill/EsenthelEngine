/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Music currently needs locking 'SongsLock', because 'playSong' can be called on both Sound Thread and by the user.

/******************************************************************************/
enum
{
   OLD,
   CUR,
};
/******************************************************************************/
struct Song
{
   Str name;
   Flt time;

   Song() {time=0;}
};
static Memb<Song> Songs; // 'Memb' is needed so that elements don't change memory addresses (currently needed for 'SongName')
static SyncLock   SongsLock;
/******************************************************************************/
MusicManager Music  (VOLUME_MUSIC  ),
             Ambient(VOLUME_AMBIENT);
/******************************************************************************/
// functions below assume lock
static   Bool LockedSongID  (Int global_song, UID &id) {return InRange(global_song, Songs) && DecodeFileName(Songs[global_song].name, id);} // !! does not zero 'id' on fail !!
static C Str& LockedSongName(Int global_song         ) {return InRange(global_song, Songs) ?                 Songs[global_song].name : S;}
static   Int  LockedSongFind(CChar *name             ) {if(Is(name))FREPA(Songs)if(EqualPath(name, Songs[i].name))return i; return -1;}
static   Int  LockedSongGet (CChar *name             )
{
   if(Is(name))
   {
      Int i=LockedSongFind(name); if(i>=0)return i; // find existing
      Songs.New().name=name; return Songs.elms()-1; // add new one
   }
   return -1;
}
static Int LockedSongGet(C UID &id) {return id.valid() ? LockedSongGet(_EncodeFileName(id)) : -1;}

static C Str& SongName(Int global_song) // !! since 'Songs' is 'Memb' and never deleted, we can just return a reference !!
{
   if(InRange(global_song, Songs))
   {
      SyncLocker locker(SongsLock); return LockedSongName(global_song);
   }
   return S;
}
static Int SongGet(CChar *name) {if(Is(name)  ){SyncLocker locker(SongsLock); return LockedSongGet(name);} return -1;}
static Int SongGet(C UID &id  ) {if(id.valid()){SyncLocker locker(SongsLock); return LockedSongGet(id  );} return -1;}
/******************************************************************************/
// PLAYLIST
/******************************************************************************/
Playlist::~Playlist()
{
   if(Music  .playlist()==this)Music  .set(null);
   if(Ambient.playlist()==this)Ambient.set(null);
}
Playlist::Playlist()
{
  _cur=-1;
}
Str Playlist::song(Int i)C
{
   if(InRange(i, _songs))
   {
      SyncLocker locker(SongsLock); // needed for both '_songs' and 'LockedSongName'
      if(InRange(i, _songs))return LockedSongName(_songs[i]);
   }
   return S;
}
UID Playlist::songID(Int i)C
{
   if(InRange(i, _songs))
   {
      SyncLocker locker(SongsLock); // needed for both '_songs' and 'LockedSongName'
      UID id; if(InRange(i, _songs) && LockedSongID(_songs[i], id))return id;
   }
   return UIDZero;
}
void Playlist::operator+=(C Str &name)
{
   if(name.is())
   {
      SyncLocker locker(SongsLock); // needed for both '_songs' and 'LockedSongGet'
     _songs.include(LockedSongGet(name));
   }
}
void Playlist::operator-=(C Str &name)
{
   if(name.is())
   {
      SyncLocker locker(SongsLock); // needed for both '_songs' and 'LockedSongFind'
      Int song=LockedSongFind(name); if(song>=0)REPA(_songs)if(_songs[i]==song)
      {
        _songs.remove(i, true);
         if(_cur==i)_cur=-1;else
         if(_cur> i)_cur-- ;
         break;
      }
   }
}
void Playlist::operator+=(C UID &id) {if(id.valid())T+=_EncodeFileName(id);}
void Playlist::operator-=(C UID &id) {if(id.valid())T-=_EncodeFileName(id);}

Int Playlist::globalSong(Int song, Bool shuffle, Randomizer &random)
{
   // !! in the first part we don't access '_songs' so we don't need to lock !!
   if(!InRange(song, _songs)) // if not in range
   {
      song=_cur; // pick previous
      if(!InRange(song, _songs)) // if still not in range
      {
         if(_songs.elms()==0)song=-1; // no songs available
         else                song=(shuffle ? random(_songs.elms()) : 0); // set random or first
      }
   }
  _cur=song;

   // !! now we have to lock !!
   if(InRange(song, _songs))
   {
      SyncLocker locker(SongsLock); // needed for '_songs'
      if(InRange(song, _songs))return _songs[song];
   }
   return -1;
}
Int Playlist::nextSong(Bool shuffle, Randomizer &random)
{
   // !! in the first part we don't access '_songs' so we don't need to lock !!
   Int song;
   if(_songs.elms()==0)song=-1;else // no       songs available
   if(_songs.elms()==1)song= 0;else // only one song  available
   if(_cur         < 0)song=(shuffle ? random(_songs.elms()) : 0);else // there was no previous so use random or first
   {
      if(shuffle)
      {
      #if 1 // works with 'prevSong'
         REP(16) // 16 attempts to find a next different song
         {
            song=random(_songs.elms());
            if(song!=_cur)break; // found a different one then stop looking
         }
      #else // simpler but doesn't work with 'prevSong'
         song=random(_songs.elms()-1);
         if(song==_cur)song=_songs.elms()-1; // if selected the same song again then switch to last one
      #endif
      }else
      {
         song=(_cur+1)%_songs.elms(); // get next in order
      }
   }
  _cur=song;

   // !! now we have to lock !!
   if(InRange(song, _songs))
   {
      SyncLocker locker(SongsLock); // needed for '_songs'
      if(InRange(song, _songs))return _songs[song];
   }
   return -1;
}
Int Playlist::prevSong(Bool shuffle, Randomizer &random)
{
   // !! in the first part we don't access '_songs' so we don't need to lock !!
   Int song;
   if(_songs.elms()==0)song=-1;else // no       songs available
   if(_songs.elms()==1)song= 0;else // only one song  available
   if(_cur         < 0)song=(shuffle ? random.back().back()(_songs.elms()) : 0);else // there was no previous so use random or first
   {
      if(shuffle)
      {
      #if 1 // works with 'nextSong'
         REP(16) // 16 attempts to find a previous different song
         {
            song=random.back().back()(_songs.elms());
            if(song!=_cur)break; // found a different one then stop looking
         }
      #else // simpler but doesn't work with 'nextSong'
         song=random.back().back()(_songs.elms()-1);
         if(song==_cur)song=_songs.elms()-1; // if selected the same song again then switch to last one
      #endif
      }else
      {
         song=(_cur+_songs.elms()-1)%_songs.elms(); // get previous in order
      }
   }
  _cur=song;

   // !! now we have to lock !!
   if(InRange(song, _songs))
   {
      SyncLocker locker(SongsLock); // needed for '_songs'
      if(InRange(song, _songs))return _songs[song];
   }
   return -1;
}
/******************************************************************************/
// MUSIC MANAGER
/******************************************************************************/
MusicManager::MusicManager(VOLUME_GROUP volume_group)
{
   shuffle   =true;
   fade_curve=FADE_LINEAR;
   fade_in   =0.5f;
   fade_out  =   3;
   time_reset=  10;
   select_song=null;

   T._song[0]=-1;
   T._song[1]=-1;
   T._volume_group=volume_group;
   T._playlist    =null;
   T._callback    =null;
   T._history_max=T._history_pos=0;
   T._random.randomize();
}
/******************************************************************************/
void MusicManager::del()
{
  _sound[0].del();
  _sound[1].del();
  _song[0]=-1;
  _song[1]=-1;
  _playlist=null;
   select_song=null;
}
/******************************************************************************/
Flt MusicManager::lockedTimeLeft()C {return _sound[CUR].timeLeft();} // !! assumes lock !!

// use locks in case '_sound' gets modified on Sound thread inside 'MusicManager.lockedUpdate'
Str  MusicManager::name  (        )C {SyncLocker locker(SongsLock); return _sound[CUR].name  ();}
UID  MusicManager::id    (        )C {SyncLocker locker(SongsLock); return _sound[CUR].id    ();}
Flt  MusicManager::length(        )C {SyncLocker locker(SongsLock); return _sound[CUR].length();}
Flt  MusicManager::time  (        )C {SyncLocker locker(SongsLock); return _sound[CUR].time  ();}
Flt  MusicManager::fade  (        )C {SyncLocker locker(SongsLock); return _sound[CUR].fade  ();}
Flt  MusicManager::frac  (        )C {SyncLocker locker(SongsLock); return _sound[CUR].frac  ();}
void MusicManager::time  (Flt time)  {SyncLocker locker(SongsLock);        _sound[CUR].time  (time);}
void MusicManager::frac  (Flt frac)  {SyncLocker locker(SongsLock);        _sound[CUR].frac  (frac);}
/******************************************************************************/
void MusicManager::set(Playlist *playlist)
{
   T._playlist=playlist;
   if(_song[CUR]<0)play(playlist); // start playing if nothing playing right now
}
void MusicManager::play(Playlist *playlist, Int song)
{
   if(playlist!=T._playlist || song>=0) // if different playlist or a song was specified
   {
      T._playlist=playlist;
      playSong(playlist ? playlist->globalSong(song, shuffle, _random) : -1);
   }
}
/******************************************************************************/
void MusicManager::play(C UID &song_id  ) {playSong(SongGet(song_id  ));}
void MusicManager::play(C Str &song_name) {playSong(SongGet(song_name));}
void MusicManager::stop(                ) {playSong(-1);}
/******************************************************************************/
// !! methods below assume lock !!
void MusicManager::swap() // !! assumes lock !!
{
   Swap(_sound[0], _sound[1]);
   Swap(_song [0], _song [1]);
}
void MusicManager::storePos(Bool i) // !! assumes lock !!
{
   Int global_song=T._song[i]; if(InRange(global_song, Songs))
   {
      Flt time=T._sound[i].time();
      {
       //SyncLocker locker(SongsLock); if(InRange(global_song, Songs)) - no need to check because we assume lock
            Songs[global_song].time=time;
      }
   }
}
void MusicManager::fadeIn (Bool i) {Sound &sound=T._sound[i]; if(sound.playing()){             sound.fadeIn (fade_in );}               } // !! assumes lock !!
void MusicManager::fadeOut(Bool i) {Sound &sound=T._sound[i]; if(sound.playing()){storePos(i); sound.fadeOut(fade_out);}               } // !! assumes lock !!
void MusicManager::del    (Bool i) {Sound &sound=T._sound[i]; if(sound.playing()){storePos(i); sound.del    (        );} T._song[i]=-1;} // !! assumes lock !!

void MusicManager::set(Bool i, Int global_song) // !! assumes "InRange(global_song, Songs)" and locked !!
{
   T._sound[i].create(Songs[global_song].name, false, 1, _volume_group).fadeCurve(fade_curve).fadeInFromSilence(fade_in);
   T._song [i]=global_song;
}
/******************************************************************************/
void MusicManager::playSong(Int global_song)
{
   SyncLocker locker(SongsLock); // !! use lock because this method can be called by both Sound and User Thread, this is also needed below for 'Songs' access and various methods !!
   // !! setting callbacks should be performed in such a way, that only one sound at a time has the callback !!
   if(InRange(global_song, Songs))
   {
      if(global_song==_song[CUR] && _sound[CUR].playing()) // if the song is already playing on CUR slot
      {
         Sound &sound=_sound[CUR];
         if(sound.timeLeft()<time_reset) // if remaining song time is less than minimum desired
         {
            del       (OLD);
            swap      (   );
            fadeOut   (OLD); _sound[OLD].callback(null);
            set       (CUR, global_song);
           _sound[CUR].callback(_callback).play(); // !! don't use 'sound' reference in case 'swap' makes it invalid
         }else
         {
            fadeIn(CUR);
         }
      }else
      if(global_song==_song[OLD] && _sound[OLD].playing()) // if the song is already playing on OLD slot
      {
         swap   (   );
         fadeOut(OLD); _sound[OLD].callback(null);
         fadeIn (CUR);
         Sound &sound=_sound[CUR]; // !! get reference after calling 'swap' in case it would make it invalid
         if(sound.timeLeft()<time_reset)sound.time(0); // if remaining song time is less than minimum desired, then start from the beginning
         sound.callback(_callback);
      }else // song is not on OLD or CUR
      {
         del    (OLD);
         swap   (   );
         fadeOut(OLD); _sound[OLD].callback(null);
         set    (CUR, global_song);
         Sound &sound=_sound[CUR]; // !! get reference after calling 'swap' in case it would make it invalid
         Flt time=0;
         {
          //SyncLocker locker(SongsLock); if(InRange(global_song, Songs)) - lock already called
               time=Songs[global_song].time; // play from last position
         }
         if(sound.length()-time<time_reset)time=0; // if remaining song time is less than minimum desired, then start from the beginning
         sound.time(time).callback(_callback).play();
      }
      if(_history_max)
      {
       //SyncLocker locker(SongsLock); if(_history_max) - lock already called
         if(!InRange(_history_pos-1, _history) || _history[_history_pos-1]!=global_song) // check that previous song is different (if the same then do nothing)
         if( InRange(_history_pos  , _history) && _history[_history_pos  ]==global_song)_history_pos++;else // if next song is the same, then increase position
         {
            if(_history.elms()>=_history_max) // at the limit
            {
               if(_history_pos>_history.elms()-_history_pos) // remove undo
               {
                 _history.remove(0, true);
                 _history_pos--;
                //MAX(_history_pos, 0); not needed since we check "_history_pos>" above
               }else // remove redo
               {
                 _history.removeLast();
                //MIN(_history_pos, _history.elms()); not needed since we check "_history_pos>" above
               }
            }
           _history.NewAt(_history_pos++)=global_song; // don't remove redos, but just insert new history between undos/redos
         }
      }
   }else
   {
      fadeOut(OLD);
      fadeOut(CUR);
   }
}
/******************************************************************************/
void MusicManager::next()
{
   auto select=T.select_song; // copy first to temp var to avoid multi-threading issues
   if(InRange(_history_pos, _history))
   {
      SyncLocker locker(SongsLock);
      if(InRange(_history_pos, _history)) // check again after having lock
      {
         Int song=_history[_history_pos];
         if(select)
         {
            UID next; if(!LockedSongID(song, next))next.zero();
            song=LockedSongGet(select(next));
         }
         playSong(song);
         return;
      }
   }
   if(            select             )play    (select(UIDZero));else
   if(Playlist *playlist=T.playlist())playSong(playlist->nextSong(shuffle, _random)); // copy first to temp var to avoid multi-threading issues
   else                               playSong(-1);
}
void MusicManager::prev()
{
   if(_history_max) // want to use history
   {
      if(_history_pos) // have previous
      {
         SyncLocker locker(SongsLock);
         if(_history_pos) // check again after having lock
         {
            Int song=_history[_history_pos-1]; // get last played song
            if( song!=_song[CUR] || !_sound[CUR].playing())playSong( song                     );else // if it's different than what playing now, or it stopped playing, then play it but don't change history position
            if(_history_pos>1                             )playSong(_history[--_history_pos-1]);     // play the one before that and change history position !! it's important to decrease '_history_pos' before calling 'playSong' so it can detect that previous song is the same !!
         }
      }
      // if want history, but history not available, then do nothing
   }else
   if(Playlist *playlist=T.playlist())playSong(playlist->prevSong(shuffle, _random)); // copy first to temp var to avoid multi-threading issues
 //else                               playSong(-1); do nothing
}
void MusicManager::lockedUpdate()
{
   if(_song[CUR]>=0 && lockedTimeLeft()<=fade_out)next();
}
/******************************************************************************/
void MusicManager::maxHistory(Int max_history)
{
   MAX(max_history, 0);
   if( max_history!=_history_max)
   {
      SyncLocker locker(SongsLock); // use lock to avoid multi-thread issues
      Int prevs=_history_pos         ,     nexts=_history.elms()-_history_pos, history_2=max_history/2,
      new_prevs=Min(prevs, history_2), new_nexts=Min(nexts, history_2), left=max_history-new_prevs-new_nexts;

         Int add_prevs=Min(prevs-new_prevs, left); if(add_prevs>0){new_prevs+=add_prevs; left-=add_prevs;}else
      {
         Int add_nexts=Min(nexts-new_nexts, left); if(add_nexts>0){new_nexts+=add_nexts; left-=add_nexts;}
      }

      // first remove last nexts if any
     _history.setNum(prevs+new_nexts);
      // now remove first prevs if any
      Int remove_prevs=prevs-new_prevs;
     _history.removeNum(0, remove_prevs, true);
     _history_pos-=remove_prevs;
     _history_max =max_history;
   }
}
/******************************************************************************/
MusicManager& MusicManager::callback(SoundDataCallback *callback)
{
   if(T._callback!=callback)
   {
      SyncLocker locker(SongsLock); // use lock to avoid issues when 'playSong' swaps sounds
      T._callback=callback;
     _sound[CUR].callback(callback);
   }
   return T;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutMusic()
{
   SyncLocker locker(SongsLock); // lock just in case

   // delete Music and Ambient first
   Music  .del();
   Ambient.del();

   // now delete Songs
   Songs.del();
}
void UpdateMusic()
{
   SyncLocker locker(SongsLock); // lock needed for 'lockedUpdate'
   Music  .lockedUpdate();
   Ambient.lockedUpdate();
}
/******************************************************************************/
}
/******************************************************************************/
