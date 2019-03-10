/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define DELAY_REMOVE_STEP (1.0f/8) // number of steps per 'delay_remove' time to check for element removal
ASSERT(OFFSET(Cache<Image>::Elm, data)==0); // '_data_offset' is not used because it's assumed to be always 0
static SyncLock DelayRemoveLock;
static UIntPtr  DelayRemoveThreadID;
static Flt      DelayRemoveWaited;
/******************************************************************************

   We must unlock 'D._lock' everywhere before we want to lock 'T._lock'

   This is to avoid deadlocks, when one thread locks first 'D._lock' and then 'T._lock'
                            and another thread locks first 'T._lock' and then 'D._lock'

   For example:
      -background loading thread calls               (Images._lock, D._lock.on)
      -inside 'Draw' during drawing we load an image (D._lock.on, Images._lock) -> deadlock
   Instead of it, during drawing it will be: D._lock.on, D._lock.off, Images.lock and unlocks...

   We have to do this even if we're not going to perform and 'D' operations:
      -background loading thread calls                                 (Images._lock, D._lock.on)
      -during 'Draw' drawing (where D is already locked) we use 'find' (D._lock.on, Images._lock) -> deadlock


   Normally 'Desc.delay_remove' should be stored as 'Dbl', however we store it as 'Flt',
      to reduce memory usage (because it's stored for every cache element and it can be a lot of them).
   We can do this because we don't need high precision for this, as typically the delay times are set to several seconds,
      and the 'Flt' error after 1 month of application run-time is 0.25s and after 1 year is 2s (see "Timer.cpp" file).
   These errors are acceptable.

   When unloading elements in 'processDelayRemove' we store how much each element waited in the global 'DelayRemoveWaited'
      so its every child when unloading elements can use that time.
   TODO: However if that child element was loaded and unloaded for extra cache elm ptr handle just before 'processDelayRemove' was called,
      then it'll still going to be unloaded faster, because we store remove time only when the last handle is released.
      To avoid this we would probably have to store remove times in elements,
         but that would increase memory usage (all elements would need it, while now only those need it in the removal queue).

/******************************************************************************/
C _Cache::Desc& _Cache::lockedDesc(Int i)C {return elmDesc(*_order[i]);}
   CPtr         _Cache::lockedData(Int i)C {return elmData(*_order[i]);}

_Cache::_Cache(CChar8 *name, Int block_elms, Bool (*load)(Ptr data, C Str &file)) : _memx(block_elms)
{
  _d_lock=0;
  _case_sensitive=false;
  _elms=0;
/*_data_offset=*/_desc_offset=0;
  _mode=CACHE_EXIT;
  _debug_name=name;
  _order=null;
  _user =null;
  _load =load;
  _load_user=null;
  _can_be_removed=null;
  _delay_remove_counter=0; _delay_remove_time=0; _delay_remove_check=0;
}
void _Cache::clear        (                                                            ) {SyncUnlocker unlocker(D._lock); SyncLocker locker(_lock); _elms=0; _memx.clear(); _delay_remove.clear();} // here can't Free '_order' because it assumes that its size is '_memx.maxElms()'
void _Cache::del          (                                                            ) {SyncUnlocker unlocker(D._lock); SyncLocker locker(_lock); _elms=0; _memx.del  (); _delay_remove.del  (); Free(_order);}
Byte _Cache::mode         (Byte mode                                                   ) {Byte old_mode=T._mode; T._mode=mode; return old_mode;}
void _Cache::caseSensitive(Bool sensitive                                              ) {T._case_sensitive=sensitive;}
void _Cache::setLoadUser  (Bool (*load_user)(Ptr data, C Str &file, Ptr user), Ptr user) {T._user=user; T._load=null; T._load_user=load_user;}
void _Cache::delayRemove  (Flt  time                                                   )
{
   Bool adjust_existing=true;
   MAX(time, 0);
   if( time!=_delay_remove_time)
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);

      Flt delta=time-_delay_remove_time; // how much are we increasing the delay
     _delay_remove_time  =time; // set new value
     _delay_remove_check+=delta*DELAY_REMOVE_STEP; // adjust check time
      if(adjust_existing)REPAO(_delay_remove).time+=delta; // adjust element removal times
      update();
   }
}
/******************************************************************************/
void _Cache::lock()C
{
   Byte d_locked=0;
   if(!_lock.owned()) // if we don't have the '_lock' yet
   {
   #if SYNC_UNLOCK_SINGLE
      if(D._lock.owned()){d_locked=true; D._lock.off();} // if we own the 'D._lock' then disable it and remember that we had it
   #else
      for(; D._lock.owned(); d_locked++)D._lock.off(); // if we own the 'D._lock' then disable it and remember that we had it
   #endif
   }
  _lock.on();
   if(d_locked)_d_lock=d_locked; // if we've disabled 'D._lock' then set how many times
}
void _Cache::unlock()C
{
   Byte d_locked=_d_lock; // remember if we had 'D._lock' disabled
       _d_lock  =0; // disable before releasing '_lock'
  _lock.off();

   if(!_lock.owned()) // if we no longer own '_lock' then we can restore 'D._lock' if we disabled it
   {
   #if SYNC_UNLOCK_SINGLE
      if(d_locked)D._lock.on();
   #else
      REP(d_locked)D._lock.on();
   #endif
   }else // if we still own '_lock' then we should keep the variable about 'D._lock'
   {
     _d_lock=d_locked;
   }
}
/******************************************************************************/
_Cache::Elm* _Cache::findExact(CChar *file, Int &stop)
{
   Int l=0, r=_elms; for(; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=ComparePath(file, elmDesc(*_order[mid]).file(), _case_sensitive);
      if(!compare)
      {
         stop=mid;
         return _order[mid];
      }
      if(compare<0)r=mid;
      else         l=mid+1;
   }
   stop=l;
   return null;
}
/******************************************************************************/
_Cache::Elm* _Cache::findElm(CChar *file, CChar *path)
{
   Int  stop;
   Elm *elm;

   if(Is(path) && !FullPath(file)) // try using "path+file" (precisely "SkipStartPath(path, DataPath()) + file"), this needs to be done first, in case there's a "file.dat" and "folder/file.dat" loaded, but we're looking for "file.dat" with "path=folder", this is also needed when loading sub-assets for a file at its location when no 'DataPath' was specified, for example, loading a material for a mesh: "file=material", path="c:/project/data"
   {
      CChar *rel_path=_SkipStartPath(path, DataPath()); // skip the 'DataPath' here, because loading assets assumes to be done relative to 'DataPath' whenever possible, so if the assets were loaded from 'DataPath' then their path will not include the 'DataPath'
      if(Is( rel_path)) // if the path is empty then don't bother checking this, as it's the same as "findExact(file)" below
      {
         Char path_file[MAX_LONG_PATH]; MergePath(path_file, rel_path, file);
         if(elm=findExact(path_file, stop))return elm;
      }
   }
                                                                   if(elm=findExact(file , stop))return elm; // try using 'file'
   if(DataPath().is())if(CChar *after=_AfterPath(file, DataPath()))return findExact(after, stop);            // try using 'file' without the 'DataPath' if it was specified
   return null;
}
/******************************************************************************/
Int _Cache::findDelayRemove(Elm &elm)
{
   REPA(_delay_remove)if(_delay_remove[i].elm==&elm)return i;
   return -1;
}
/******************************************************************************/
void _Cache::addToOrder(Elm &elm)
{
   Int stop; findExact(elmDesc(elm).file(), stop);
   MoveFastN(_order+stop+1, _order+stop, _elms-stop); // faster version of: for(Int i=_elms; i>stop; i--)_order[i]=_order[i-1];
  _order[stop]=&elm; _elms++;
}
void _Cache::removeFromOrder(Elm &elm)
{
   Int stop; if(findExact(elmDesc(elm).file(), stop))
   {
     _elms--; MoveFastN(_order+stop, _order+stop+1, _elms-stop); // faster version of: for(Int i=stop; i<_elms; i++)_order[i]=_order[i+1];
   }
}
/******************************************************************************/
Ptr _Cache::validElmData(Elm &elm, Bool counted)
{
   Desc &desc=elmDesc(elm);
   if( !(desc.flag&CACHE_ELM_LOADING))
   {
      if(counted)desc.ptr_num++;else FlagEnable(desc.flag, CACHE_ELM_STD_PTR);
      return elmData(elm);
   }
   return null;
}
/******************************************************************************/
Ptr _Cache::_find(CChar *file, CChar *path, Bool counted)
{
   if(Is(file)) // valid file name
   {
      // lock
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);

      if(Elm *elm=findElm(file, path))return validElmData(*elm, counted);
   }
   return null;
}
/******************************************************************************/
inline static CChar* HelperPath(CChar *file, CChar *path, Char (&dest)[MAX_LONG_PATH]=ConstCast(TempChar<MAX_LONG_PATH>()).c)
{
   MergePath(dest, _SkipStartPath(path, DataPath()), file);
   return    dest;
}

Ptr _Cache::_get(CChar *file, CChar *path, Bool counted)
{
   if(Is(file)) // valid file name
   {
      SyncUnlocker unlocker(D._lock);
      SyncLocker     locker(  _lock);

      // find existing
      if(Elm *elm=findElm(file, path))return validElmData(*elm, counted);

      // always null
      if(_mode==CACHE_ALL_NULL)return null;

      // new element
      Int max_elms =_memx.maxElms(); Elm &elm=_memx.New(); Desc &desc=elmDesc(elm); Ptr data=elmData(elm); desc.ptr_num=(counted ? 1 : 0); desc.flag=(counted ? 0 : CACHE_ELM_STD_PTR);
      if( max_elms!=_memx.maxElms())Realloc(_order, _memx.maxElms(), max_elms);
      file=_SkipStartPath(file, DataPath());

      // always dummy
      if(_mode==CACHE_ALL_DUMMY)
      {
      dummy:
         desc.flag|=CACHE_ELM_DUMMY;
         if(Is(path)){desc.file=HelperPath(file, path); if(!FExist(desc.file) && FExist(file))desc.file=file;} // if a file does not exist at path, but does exist at absolute, then use absolute
         else         desc.file=file; // use absolute when no path available
         addToOrder(elm);
         return data;
      }else
      if(_mode==CACHE_DUMMY_NULL) // dummy on exist, null on fail
      {
         Bool exist=false;
         if(Is(path)){desc.file=HelperPath(file, path); if(FExist(desc.file))                 exist=true; }
         if( !exist ){                                  if(FExist(     file)){desc.file=file; exist=true;}}
         if(  exist )
         {
            desc.flag|=CACHE_ELM_DUMMY;
            addToOrder(elm);
            return data;
         }
        _memx.removeData(&elm);
         return null;
      }

      // load
      {
         desc.flag|=CACHE_ELM_LOADING;

         // in path
         if(Is(path))
         {
            desc.file=HelperPath(file, path); // set file before loading
            addToOrder(elm);

            // try to load
            if(_load ? _load(data, desc.file) : _load_user(data, desc.file, _user))
            {
            ok:
               FlagDisable(desc.flag, CACHE_ELM_LOADING);
               return data;
            }

            removeFromOrder(elm); // !! don't try to optimize by using the same 'stop' calculated from 'addToOrder' as it may have changed, because inside 'load' there could be other objects loaded !!
         }

         // absolute
         {
            desc.file=file; // set file before loading
            addToOrder(elm);

            // try to load
            if(_load ? _load(data, desc.file) : _load_user(data, desc.file, _user))goto ok;

            removeFromOrder(elm); // !! don't try to optimize by using the same 'stop' calculated from 'addToOrder' as it may have changed, because inside 'load' there could be other objects loaded !!
         }

         FlagDisable(desc.flag, CACHE_ELM_LOADING);
      }

      // dummy on fail
      if(_mode==CACHE_DUMMY)goto dummy;

      // null
     _memx.removeData(&elm);
   }
   return null;
}
/******************************************************************************/
Ptr _Cache::_require(CChar *file, CChar *path, Bool counted)
{
   if(Ptr data=_get(file, path, counted))return data;
   if(Is(file) && _mode==CACHE_EXIT)
   {
              Str error =MLT(S+"Can't load "+_debug_name+" \""+file+'"', PL,S+u"Nie można wczytać \""  +file+'"');
      if(Is(path))error+=MLT(S+"\nAdditional path \""         +path+'"', PL,S+u"\nDodatkowa ścieżka \""+path+'"');
             Exit(error);
   }
   return null;
}
/******************************************************************************/
Ptr _Cache::_find   (C UID &id, CChar *path, Bool counted) {return id.valid() ? _find   (_EncodeFileName(id), path, counted) : null;}
Ptr _Cache::_get    (C UID &id, CChar *path, Bool counted) {return id.valid() ? _get    (_EncodeFileName(id), path, counted) : null;}
Ptr _Cache::_require(C UID &id, CChar *path, Bool counted) {return id.valid() ? _require(_EncodeFileName(id), path, counted) : null;}
/******************************************************************************
INLINE Int _Cache::absIndex(CPtr data)C // this function assumes that 'data' is not null
{
#if 1 // fast, this can work because '_data_offset' is now zero
   DEBUG_ASSERT(_data_offset==0, "Cache data offset should be zero but it isn't");
   return _memx.absIndex(dataElm(data));
#else // safe
   #if WINDOWS
      __try                               {return _memx.absIndex(dataElm(data));} // use 'absIndex' only when exception control is possible
      __except(EXCEPTION_EXECUTE_HANDLER) {return -1;}
   #else
      return _memx._abs.index(data);
   #endif
#endif
}
/******************************************************************************/
Bool _Cache::_contains(CPtr data)C
{
   if(C Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))if(!(elmDesc(*elm).flag&CACHE_ELM_LOADING))return true;
   }
   return false;
}
Bool _Cache::_dummy(CPtr data)C
{
   if(C Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))if(elmDesc(*elm).flag&CACHE_ELM_DUMMY)return true;
   }
   return false;
}
void _Cache::_dummy(CPtr data, Bool dummy)
{
   if(Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))FlagSet(elmDesc(*elm).flag, CACHE_ELM_DUMMY, dummy);
   }
}
Int _Cache::_ptrCount(CPtr data)C
{
   if(C Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))return elmDesc(*elm).ptr_num;
   }
   return -1;
}
CChar* _Cache::_name(CPtr data, CChar *path)C
{
   if(C Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))
      {
       C Desc &desc=elmDesc(*elm);
         if(!(desc.flag&CACHE_ELM_LOADING)) // name may change while loading
         {
            if(Is(path))return _SkipStartPath(desc.file(), _SkipStartPath(path, DataPath())); // must be '_SkipStartPath' because we're returning CChar*
                        return                desc.file();
         }
      }
   }
   return null;
}
UID _Cache::_id(CPtr data)C
{
   if(C Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))return FileNameID(elmDesc(*elm).file()); // ID does not change while loading, so ignore CACHE_ELM_LOADING
   }
   return UIDZero;
}
/******************************************************************************/
void _Cache::_removeData(CPtr data)
{
   if(Elm *elm=dataElm(data))
   {
      SyncUnlocker unlocker(D._lock); // this must be used also since later 'D._lock' can be locked when deleting the resource
      SyncLocker     locker(  _lock);
      if(contains(elm))
      {
         Desc &desc=elmDesc(*elm);
         FlagDisable(desc.flag, CACHE_ELM_STD_PTR);
         if(!desc.ptr_num) // if there are no more pointers accessing this element
         {
            if(desc.flag&CACHE_ELM_DELAY_REMOVE)_delay_remove.remove(findDelayRemove(*elm)); // if was listed in the 'delay_remove' then remove it from it
            removeFromOrder(*elm);
           _memx.removeData( elm);
         }
      }
   }
}
/******************************************************************************/
void _Cache::_incRef(CPtr data)
{
   if(Elm *elm=dataElm(data))
   #if !SYNC_LOCK_SAFE // if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means cache was already initialized)
      if(_elms)
   #endif
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      if(contains(elm))elmDesc(*elm).ptr_num++;
   }
}
void _Cache::_decRef(CPtr data)
{
   if(Elm *elm=dataElm(data))
   #if !SYNC_LOCK_SAFE // if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means cache was already initialized)
      if(_elms)
   #endif
   {
      SyncUnlocker unlocker(D._lock); // this must be used also since later 'D._lock' can be locked when deleting the resource
      SyncLocker     locker(  _lock);
      if(contains(elm))
      {
         Desc &desc=elmDesc(*elm); DEBUG_ASSERT(desc.ptr_num>0, "'_Cache.decRef' Decreasing 'ptr_num' when it's already zero");
         if(!--desc.ptr_num && !(desc.flag&CACHE_ELM_STD_PTR)) // if there are no more pointers accessing this element
         {
            Flt delay_remove_time=_delay_remove_time;
            if( delay_remove_time   >0 && GetThreadId()==DelayRemoveThreadID)delay_remove_time-=DelayRemoveWaited; // if want to use delayed remove by time and we're unloading because of parent getting delay unloaded, then decrease the time which the parent already waited
            if( delay_remove_time   >0 // if want to use delayed remove by time
            || _delay_remove_counter>0 // if want to use delayed remove temporarily
            || (_can_be_removed && !_can_be_removed(data))) // or it can't be removed right now
            {
               Flt time=Time.appTime()+delay_remove_time; // set removal time
               if(desc.flag&CACHE_ELM_DELAY_REMOVE) // if already listed
               {
                  Int i=findDelayRemove(*elm); DEBUG_ASSERT(i>=0, "'_Cache.decRef' Element has CACHE_ELM_DELAY_REMOVE but isn't listed in '_delay_remove'");
                 _delay_remove[i].time=time;
               }else // not yet listed
               {
                  FlagEnable(desc.flag, CACHE_ELM_DELAY_REMOVE);
                  DelayRemove &remove=_delay_remove.New();
                  remove.elm =elm;
                  remove.time=time;
               }
            }else // remove now
            {
               if(desc.flag&CACHE_ELM_DELAY_REMOVE)_delay_remove.remove(findDelayRemove(*elm)); // if was listed in the 'delay_remove' then remove it from it
               removeFromOrder(*elm);
              _memx.removeData( elm);
            }
         }
      }
   }
}
/******************************************************************************/
void _Cache::processDelayRemove(Bool always)
{
   if(_delay_remove.elms())
   {
      SyncUnlocker   unlocker(D._lock); // this must be used also since later 'D._lock' can be locked when deleting the resource
      SyncLocker delay_locker(DelayRemoveLock); DelayRemoveThreadID=GetThreadId(); // support only one 'processDelayRemove' at a time, because we use only one global 'DelayRemoveWaited' base on 'DelayRemoveThreadID' for all caches/threads
      SyncLocker       locker(  _lock);
     _delay_remove_check=Time.appTime()+_delay_remove_time*DELAY_REMOVE_STEP; // perform next check at this time
      REPA(_delay_remove)
      {
         DelayRemove &remove=_delay_remove[i];
         if(always || Time.appTime()>=remove.time) // if always remove or enough time has passed (use >= so when having zero delay time then it will be processed immediately)
         {
            Elm &elm=*remove.elm; Desc &desc=elmDesc(elm); _delay_remove.remove(i); // access before removal and remove afterwards
            if(desc.ptr_num || (desc.flag&CACHE_ELM_STD_PTR)) // if there is something accessing this element now
            {
               FlagDisable(desc.flag, CACHE_ELM_DELAY_REMOVE); // keep the element but disable the 'CACHE_ELM_DELAY_REMOVE' flag since we've removed it from the '_delay_remove' container
            }else // nothing accessing this element
            {
               // remove element from cache
               DelayRemoveWaited=Max(0, Time.appTime()-remove.time+_delay_remove_time); // get how much time this element was waiting to be removed, set this before removing this element, so its children will be able to access it in the destructor
               removeFromOrder( elm);
              _memx.removeData(&elm);
            }
         }
      }
      DelayRemoveWaited  =0; // clear back to zero before clearing 'DelayRemoveThreadID', in case some other thread has ID=0, and thus would use 'DelayRemoveWaited' from this thread
      DelayRemoveThreadID=0;
   }
}
void _Cache::delayRemoveInc() {   AtomicInc(_delay_remove_counter);}
void _Cache::delayRemoveDec() {if(AtomicDec(_delay_remove_counter)==1)update();}
void _Cache::delayRemoveNow() {                                                                                           processDelayRemove(true );}
void _Cache::update        () {if(_delay_remove.elms() && _delay_remove_counter==0 && Time.appTime()>=_delay_remove_check)processDelayRemove(false);}
/******************************************************************************/
void _Cache::_lockedFrom(C _Cache &src)
{
   del();

  _case_sensitive   =src._case_sensitive;
  _mode             =src._mode;
  _debug_name       =src._debug_name;
  _delay_remove_time=src._delay_remove_time;
//_delay_remove_check, _delay_remove_counter - leave them

#if 0 // modify dest type to match src type (this is not needed)
//_data_offset   =src._data_offset;
  _desc_offset   =src._desc_offset;
  _user          =src._user;
  _load          =src._load;
  _load_user     =src._load_user;
  _can_be_removed=src._can_be_removed;
  _memx._reset(src._memx.elmSize(), src._memx.blockElms(), src._memx._new, src._memx._del);
#endif

  _elms       =src._elms;
  _memx.setNum(src._memx.elms());  // pre allocate memory for elements
   Alloc(_order, _memx.maxElms()); // initialize order exactly the same way as source
   REPA(src)                       //      setup order exactly the same way as source
   {
    C Elm  & src_elm =   *src._order[i]; Int memx_index=src._memx.validIndex(&src_elm); DYNAMIC_ASSERT(InRange(memx_index, _memx), "Invalid element index in Cache.operator=(C Cache &src)"); // get valid index of i-th element in memx container
      Elm  &dest_elm =_memx[memx_index]; _order[i]=&dest_elm; // set valid index of i-th element
    C Desc & src_desc=src.elmDesc( src_elm);
      Desc &dest_desc=    elmDesc(dest_elm);
      dest_desc=src_desc; // copy desc
      dest_desc.ptr_num=0; // there are no pointers referencing elements created in dest, because if there were any pointers, then they reference only elements in the source, this must be cleared so reference counting will work correctly
      FlagDisable(dest_desc.flag, CACHE_ELM_STD_PTR); // these elements aren't specifically referenced, as only source elements are
      // don't set anything in '_delay_remove' because this is set based on elements that are no longer referenced, however in current situation all elements have 'ptr_num=0' and 'CACHE_ELM_STD_PTR' disabled, so all of them would have to be removed
      // TODO: what's the best way to handle adjusting CACHE_ELM_STD_PTR flag and setting '_delay_remove', because currently it's always disabled. Sample use when this cache copying happens is for Editor source, copying Project class objects, which has "Cache<WorldVer> world_vers;" member (this is not reference counted, but only accessed through CACHE_ELM_STD_PTR)
   }
}
/******************************************************************************/
}
/******************************************************************************/
