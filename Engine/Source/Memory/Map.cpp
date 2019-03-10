/******************************************************************************/
#include "stdafx.h"
namespace EE{
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

/******************************************************************************/
typedef Map<Int, Int> MapInt; ASSERT(OFFSET(MapInt::Elm, data)==0); // '_data_offset' is not used because it's assumed to be always 0
/******************************************************************************/
_Map::_Map(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src)) : _memx(block_elms)
{
  _elms=0;
  _key_offset=/*_data_offset=*/_desc_offset=_data_size=0;
  _mode=MAP_EXIT;
  _order=null;
  _user    =user;
  _compare =compare;
  _create  =create;
  _copy_key=copy_key;
}
_MapTS::_MapTS(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src)) : _Map(block_elms, compare, create, user, copy_key)
{
  _d_lock=0;
}
void _Map::clear() {_memx.clear(); _elms=0;              } // here can't Free '_order' because it assumes that its size is '_memx.maxElms()'
void _Map::del  () {_memx.del  (); _elms=0; Free(_order);}

void _MapTS::clear() {SyncUnlocker unlocker(D._lock); SyncLocker locker(_lock); super::clear();}
void _MapTS::del  () {SyncUnlocker unlocker(D._lock); SyncLocker locker(_lock); super::del  ();}

Byte _Map::mode(Byte mode) {Byte old_mode=T._mode; T._mode=mode; return old_mode;}

CPtr _Map::key       (Int i)C {return elmKey (*_order[i]);}
 Ptr _Map::operator[](Int i)  {return elmData(*_order[i]);}

CPtr _Map::absKey (Int abs_i)C {return elmKey (_memx.absElm(abs_i));}
CPtr _Map::absData(Int abs_i)C {return elmData(_memx.absElm(abs_i));}

INLINE Int _Map::dataInMapToAbsIndex(CPtr data)C {return data ? _memx.absIndexFastUnsafeValid(dataElm(data)) : -1;}
/******************************************************************************/
void _MapTS::lock()C
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
void _MapTS::unlock()C
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
_Map::Elm* _Map::findElm(CPtr key, Int &stop)C
{
   Int l=0, r=_elms; for(; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=T._compare(key, elmKey(*_order[mid]));
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
void _Map::addToOrder(Elm &elm, Int index)
{
   MoveFastN(_order+index+1, _order+index, _elms-index); // faster version of: for(Int i=_elms; i>index; i--)_order[i]=_order[i-1];
  _order[index]=&elm; _elms++;
}
void _Map::removeFromOrder(Int index)
{
  _elms--;
   MoveFastN(_order+index, _order+index+1, _elms-index); // faster version of: for(Int i=index; i<_elms; i++)_order[i]=_order[i+1];
}
/******************************************************************************/
Int _Map::findAbsIndex(CPtr key)C {return dataInMapToAbsIndex(find(key));}
Ptr _Map::find        (CPtr key)C
{
   Int i; if(Elm *elm=findElm(key, i))if(!(elmDesc(*elm).flag&MAP_ELM_LOADING))return elmData(*elm);
   return null;
}
Ptr _MapTS::find(CPtr key)C
{
   SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
   SyncLocker     locker(  _lock);
   return super::find(key);
}
Int _MapTS::findAbsIndex(CPtr key)C
{
   SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
   SyncLocker     locker(  _lock);
   return super::findAbsIndex(key);
}
/******************************************************************************/
Int _Map::getAbsIndex(CPtr key) {return dataInMapToAbsIndex(get(key));}
Ptr _Map::get        (CPtr key)
{
   Int stop;

   // find existing
   if(Elm *elm=findElm(key, stop))return (elmDesc(*elm).flag&MAP_ELM_LOADING) ? null : elmData(*elm);

   // always null
   if(_mode==MAP_ALL_NULL)return null;

   // new element
   Int max_elms =_memx.maxElms(); Elm &elm=_memx.New(); Desc &desc=elmDesc(elm); Ptr data=elmData(elm), elm_key=elmKey(elm); desc.flag=0;
   if( max_elms!=_memx.maxElms())Realloc(_order, _memx.maxElms(), max_elms);

   // always dummy
   if(_mode==MAP_ALL_DUMMY)
   {
   dummy:
      desc.flag|=MAP_ELM_DUMMY;
     _copy_key(elm_key, key);
      addToOrder(elm, stop);
      return data;
   }

   // create
   {
      desc.flag|=MAP_ELM_LOADING;
     _copy_key(elm_key, key); // copy key before creating
      addToOrder(elm, stop);

      // try to load
      if(_create ? _create(data, elm_key, _user) : true)
      {
         FlagDisable(desc.flag, MAP_ELM_LOADING);
         return data;
      }

      if(findElm(key, stop))
      {
         removeFromOrder(stop); // don't try to optimize by using the same 'stop' calculated from 'addToOrder' as it may have changed, because inside 'create' there could be other objects created
         FlagDisable(desc.flag, MAP_ELM_LOADING);
      }
   }

   // dummy on fail
   if(_mode==MAP_DUMMY)goto dummy;

   // null
  _memx.removeData(&elm);
   return null;
}
Ptr _MapTS::get(CPtr key)
{
   SyncUnlocker unlocker(D._lock);
   SyncLocker     locker(  _lock);
   return super::get(key);
}
Int _MapTS::getAbsIndex(CPtr key)
{
   SyncUnlocker unlocker(D._lock);
   SyncLocker     locker(  _lock);
   return super::getAbsIndex(key);
}
/******************************************************************************/
void _Map::getFailed()C
{
   if(_mode==MAP_EXIT)Exit("Can't create object in 'Map' from key");
}
Ptr _Map  ::operator()     (CPtr key) {  if(Ptr data=get        (key)                 )return data;      getFailed(); return null;}
Ptr _MapTS::operator()     (CPtr key) {  if(Ptr data=get        (key)                 )return data;      getFailed(); return null;}
Int _Map  ::requireAbsIndex(CPtr key) {Int abs_index=getAbsIndex(key); if(abs_index>=0)return abs_index; getFailed(); return   -1;}
Int _MapTS::requireAbsIndex(CPtr key) {Int abs_index=getAbsIndex(key); if(abs_index>=0)return abs_index; getFailed(); return   -1;}
/******************************************************************************/
Bool _Map  ::containsKey(CPtr key)C {return find(key)!=null;}
Bool _MapTS::containsKey(CPtr key)C {return find(key)!=null;}
/******************************************************************************/
Bool _Map::containsData(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm))if(!(elmDesc(*elm).flag&MAP_ELM_LOADING))return true;
   return false;
}
Bool _MapTS::containsData(CPtr data)C
{
   if(data)
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      return super::containsData(data);
   }
   return false;
}
/******************************************************************************
Int         absIndex(C Elm *elm )C {return _memx.  absIndex(        elm  );} // this is NOT thread-safe
Int       validIndex(C Elm *elm )C {return _memx.validIndex(        elm  );} // this is NOT thread-safe
Int         absIndex( CPtr  data)C {return         absIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
Int       validIndex( CPtr  data)C {return       validIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero

INLINE Int _Map::absIndex(CPtr data)C // this function assumes that 'data' is not null
{
#if 1 // fast, this can work because '_data_offset' is now zero
   DEBUG_ASSERT(_data_offset==0, "Cache data offset should be zero but it isn't");
   return _memx.absIndex(dataElm(data));
#else
   #if WINDOWS
      __try                               {return _memx.absIndex(dataElm(data));} // use 'absIndex' only when exception control is possible
      __except(EXCEPTION_EXECUTE_HANDLER) {return -1;}
   #else
      return _memx._abs.index(data);
   #endif
#endif
}
/******************************************************************************/
CPtr _Map::dataToKey(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm))return elmKey(*elm); // key does not change while loading, so ignore MAP_ELM_LOADING
   return null;
}
CPtr _MapTS::dataToKey(CPtr data)C
{
   if(data)
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      return super::dataToKey(data);
   }
   return null;
}
/******************************************************************************/
Int _Map::dataToIndex(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm)) // only after we know that this element belongs to this container then we can access its key
   {
      Int i; if(findElm(elmKey(*elm), i))return i;
   }
   return -1;
}
Int _MapTS::dataToIndex(CPtr data)C
{
   if(data)
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      return super::dataToIndex(data);
   }
   return -1;
}
/******************************************************************************/
void _Map::remove(Int i)
{
   if(InRange(i, _elms))
   {
      Elm *elm=_order[i];
      removeFromOrder(i);
     _memx.removeData(elm);
   }
}
void _MapTS::remove(Int i)
{
   if(InRange(i, _elms))
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      super::remove(i);
   }
}
void _Map::removeKey(CPtr key)
{
   Int i; if(Elm *elm=findElm(key, i))
   {
      removeFromOrder(i);
     _memx.removeData(elm);
   }
}
void _MapTS::removeKey(CPtr key)
{
   SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
   SyncLocker     locker(  _lock);
   super::removeKey(key);
}
void _Map::removeData(CPtr data)
{
 C Elm *elm=dataElm(data); if(containsElm(elm)) // only after we know that this element belongs to this container then we can access its key
   {
      Int i; if(findElm(elmKey(*elm), i))
      {
         removeFromOrder(i);
        _memx.removeData(elm);
      }
   }
}
void _MapTS::removeData(CPtr data)
{
   if(data)
   {
      SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);
      super::removeData(data);
   }
}
/******************************************************************************/
Bool _Map::replaceKey(CPtr src_key, CPtr dest_key)
{
   Int src_i, dest_i;
   Elm * src_elm=findElm( src_key,  src_i),
       *dest_elm=findElm(dest_key, dest_i);
   if(src_elm || dest_elm) // if at least one element was found
   {
      if(src_elm!=dest_elm) // it's not the same element (this would mean that both keys are the same and we don't need to do anything)
      {
         if(src_elm && dest_elm)
         {
            // these assertions are to make sure of the following member order in class: data, key, desc, allowing to calculate the 'key_size'
            typedef Map<Long, Long>::Elm MapElmTest; // needed because macro can't support comma
            ASSERT(OFFSET(MapElmTest, data)==0 // 'data' is first
                && OFFSET(MapElmTest, key )> 0 // 'key' is after 'data'
                && OFFSET(MapElmTest, desc)>OFFSET(MapElmTest, key) // 'desc' is after 'key'
                && SIZE(MapElmTest)==MEMBER_SIZE(MapElmTest, data)+MEMBER_SIZE(MapElmTest, key)+MEMBER_SIZE(MapElmTest, desc)); // only data+key+desc in class
            Int key_size=_desc_offset-_key_offset; // !! Warning: this may include some padding between KEY and DESC, however we're just swapping between valid 'Map.Elm' KEY objects, so it's OK, alternative would be to store '_key_size' in '_Map' but that would increase class size

            Swap(elmKey(*src_elm), elmKey(*dest_elm), key_size); // swap in case 'src_key', 'dest_key' point to 'elmKey'
            Swap(_order[src_i], _order[dest_i]); // adjust order position
         }else
         {
            if(src_elm)
            {
              _copy_key(elmKey(*src_elm), dest_key); // adjust key "src_elm.key = dest_key"
               if(dest_i>src_i)dest_i--; // we're moving element from the order to the right, which means that one slot is already occupied by this element
               MoveElm(_order, _elms, src_i, dest_i); // adjust order position
            }else
            if(dest_elm)
            {
              _copy_key(elmKey(*dest_elm), src_key); // adjust key "dest_elm.key = src_key"
               if(src_i>dest_i)src_i--; // we're moving element from the order to the right, which means that one slot is already occupied by this element
               MoveElm(_order, _elms, dest_i, src_i); // adjust order position
            }
         }
      }
      return true;
   }
   return false;
}
Bool _MapTS::replaceKey(CPtr src_key, CPtr dest_key)
{
   SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
   SyncLocker     locker(  _lock);
   return super::replaceKey(src_key, dest_key);
}
/******************************************************************************/
void _Map::from(C _Map &src)
{
   del();

  _mode=src._mode;
//_compare, _copy_key - keep

#if 0 // modify dest type to match src type (this is not needed)
   _key_offset=src. _key_offset;
//_data_offset=src._data_offset;
  _desc_offset=src._desc_offset;
  _data_size  =src._data_size;
  _user       =src._user;
  _create     =src._create;
  _memx._reset(src._memx.elmSize(), src._memx.blockElms(), src._memx._new, src._memx._del);
#endif

  _elms       =src._elms;
  _memx.setNum(src._memx.elms());  // pre allocate memory for elements
   Alloc(_order, _memx.maxElms()); // initialize order exactly the same way as source
   REPA(_memx)                     //      setup order exactly the same way as source
   {
    C Elm  & src_elm =   *src._order[i]; Int memx_index=src._memx.validIndex(&src_elm); DYNAMIC_ASSERT(InRange(memx_index, _memx), "Invalid element index in Map.operator=(C Map &src)"); // get valid index of i-th element in memx container
      Elm  &dest_elm =_memx[memx_index]; _order[i]=&dest_elm; // set valid index of i-th element
    C Desc & src_desc=src.elmDesc( src_elm);
      Desc &dest_desc=    elmDesc(dest_elm);
      dest_desc=src_desc; // copy desc
     _copy_key(elmKey(dest_elm), src.elmKey(src_elm)); // copy key
   }
}
/******************************************************************************/
}
/******************************************************************************/
