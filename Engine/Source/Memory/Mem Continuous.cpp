/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void _Memc::_reset(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T.~_Memc();
   new(this)_Memc(elm_size, _new, _del);
}
_Memc::_Memc(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T._elms    =0;
   T._elm_size=elm_size;
   T._max_elms=0;
   T._data    =null;
   T._new     =_new;
   T._del     =_del;
}
/******************************************************************************/
void _Memc::del()
{
   clear();
   Free (_data);
  _max_elms=0;
}
void _Memc::clear()
{
   if(_del)REPA(T)_del(T[i]);
  _elms=0;
}
/******************************************************************************/
void _Memc::reserve(Int num)
{
   if(num>_max_elms)
   {
      if(!initialized())Exit("Attempting to create an object of zero size in 'Memc' container.\nThe container is not initialized or it is abstract and 'replaceClass' hasn't been called.");
     _max_elms=CeilPow2(num);
     _Realloc(_data, _max_elms*elmSize(), elms()*elmSize());
   }
}
void _Memc::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
      if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]);
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]);
     _elms=num; // set '_elms' after accessing new elements to avoid range assert
   }
}
void _Memc::setNumZero(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
      Zero(T[old_elms], elmSize()*(elms()-old_elms));
      if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]);
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]);
     _elms=num; // set '_elms' after accessing new elements to avoid range assert
   }
}
/******************************************************************************/
Int _Memc::addNum    (Int num) {Int index=elms(); setNum    (elms()+num); return index;}
Int _Memc::addNumZero(Int num) {Int index=elms(); setNumZero(elms()+num); return index;}
/******************************************************************************/
Ptr _Memc::NewAt(Int i)
{
   Clamp(i, 0, elms());
   Int old_elms=elms(); _elms++; // increase '_elms' before accessing new elements to avoid range assert
   if(elms()>_max_elms)
   {
     _max_elms=CeilPow2(elms());
      Ptr temp=Alloc(_max_elms*elmSize()); // copy everything to a new buffer
      CopyFast((Byte*)temp                , T[0],           i *elmSize());
      CopyFast((Byte*)temp+(i+1)*elmSize(), T[i], (old_elms-i)*elmSize());
      Free(_data); _data=temp;
   }else
   if(i<old_elms)
   {
      MoveFast(T[i+1], T[i], (old_elms-i)*elmSize());
   }
   Ptr elm=T[i]; if(_new)_new(elm); return elm;
}
void _Memc::removeLast()
{
   if(elms())
   {
      if(_del)_del(T[elms()-1]);
     _elms--;
   }
}
void _Memc::remove(Int i, Bool keep_order)
{
   if(InRange(i, T))
   {
      if(_del)_del(T[i]);
      if(i<elms()-1)
      {
         if(keep_order)MoveFast(T[i], T[     i+1], elmSize()*(elms()-1-i));
         else          CopyFast(T[i], T[elms()-1], elmSize());
      }
     _elms--;
   }
}
void _Memc::removeNum(Int i, Int n, Bool keep_order)
{
   if(i<0){n+=i; i=0;} // if 'i' is before the start, then move it to start and reduce number of elements to remove
   if(n>0 && InRange(i, T)) // if we want to remove elements and the index fits
   {
      MIN(n, elms()-i); // minimize what we can actually remove
      if(_del)REPD(j, n)_del(T[i+j]); // delete those elements
      if(i<elms()-n) // if there are any elements after those being removed
      {
                      if(keep_order)MoveFast(T[i], T[     i+n], elmSize()*(elms()-n-i));else // move all    elements after i(+n) to left
         {Int m=Min(n, elms()-i-n); CopyFast(T[i], T[elms()-m], elmSize()*        m   );}    // move last m elements to i-th
      }
     _elms-=n;
   }
}
void _Memc::removeData(CPtr elm, Bool keep_order)
{
   remove(index(elm), keep_order);
}
/******************************************************************************/
Ptr _Memc::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside _Memc.operator()(Int i)");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
/******************************************************************************/
Int _Memc::index(CPtr elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<UIntPtr(elms()*elmSize()))return i/elmSize(); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
/******************************************************************************/
Bool _Memc::binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C {return _BinarySearch(data(), elms(), elmSize(), value, index, compare);}

void _Memc::          sort(Int compare(CPtr a, CPtr b)) {          _Sort(data(), elms(), elmSize(), compare       );}
void _Memc::  reverseOrder(                           ) {  _ReverseOrder(data(), elms(), elmSize()                );}
void _Memc::randomizeOrder(                           ) {_RandomizeOrder(data(), elms(), elmSize()                );}
void _Memc::   rotateOrder(Int offset                 ) {   _RotateOrder(data(), elms(), elmSize(), offset        );}
void _Memc::     moveElm  (Int elm, Int new_index     ) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index);}
void _Memc::     swapOrder(Int i  , Int j             ) {if(InRange(i, T) && InRange(j, T))Swap(T[i], T[j], elmSize());}

void _Memc::moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp) {_MoveElmLeftUnsafe(data(), elmSize(), elm, new_index, temp);}
/******************************************************************************/
void _Memc::copyTo  ( Ptr dest)C {Copy(dest  , data(), elms()*elmSize());}
void _Memc::copyFrom(CPtr src )  {Copy(data(), src   , elms()*elmSize());}
/******************************************************************************
void _Memc::copyRaw(_Memc &dest)
{
   if(this!=&dest)
   {
      dest.del();

      dest._elms    =_elms;
      dest._elm_size=_elm_size;
      dest._max_elms=_elms;
      dest._new     =_new ;
      dest._del     =_del ;
      dest._data    =       Alloc(_elms*_elm_size);
      CopyFast(dest._data, _data, _elms*_elm_size);
   }
}
/******************************************************************************/
Bool _Memc::saveRaw(File &f)C
{
   f.cmpUIntV(elms());
   f.put     (data(), elms()*elmSize());
   return f.ok();
}
Bool _Memc::loadRaw(File &f)
{
   setNum(f.decUIntV());
   f.getFast(data(), elms()*elmSize());
   if(f.ok())return true;
   clear(); return false;
}
Bool _Memc::_saveRaw(File &f)C
{
   f.putInt(elms());
   f.put   (data(), elms()*elmSize());
   return f.ok();
}
Bool _Memc::_loadRaw(File &f)
{
   setNum(f.getInt());
   f.getFast(data(), elms()*elmSize());
   if(f.ok())return true;
   clear(); return false;
}
/******************************************************************************/
// MEMC THREAD SAFE
/******************************************************************************/
_MemcThreadSafe::_MemcThreadSafe(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm)) : _memc(elm_size, _new, _del) {}

void _MemcThreadSafe::clear() {SyncLocker locker(_lock); _memc.clear();}
void _MemcThreadSafe::del  () {SyncLocker locker(_lock); _memc.del  ();}

Int _MemcThreadSafe::index(CPtr elm)C {SyncLocker locker(_lock); return _memc.index(elm);}

void _MemcThreadSafe::removeLast(                         ) {SyncLocker locker(_lock); _memc.removeLast(               );}
void _MemcThreadSafe::remove    (Int  i  , Bool keep_order) {SyncLocker locker(_lock); _memc.remove    (i  , keep_order);}
void _MemcThreadSafe::removeData(CPtr elm, Bool keep_order) {SyncLocker locker(_lock); _memc.removeData(elm, keep_order);}

void _MemcThreadSafe::setNum    (Int num) {SyncLocker locker(_lock);        _memc.setNum    (num);}
void _MemcThreadSafe::setNumZero(Int num) {SyncLocker locker(_lock);        _memc.setNumZero(num);}
Int  _MemcThreadSafe::addNum    (Int num) {SyncLocker locker(_lock); return _memc.addNum    (num);}

Bool _MemcThreadSafe::binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C {SyncLocker locker(_lock); return _memc.binarySearch(value, index, compare);}

void _MemcThreadSafe::          sort(Int compare(CPtr a, CPtr b)) {SyncLocker locker(_lock); _memc.          sort(compare       );}
void _MemcThreadSafe::  reverseOrder(                           ) {SyncLocker locker(_lock); _memc.  reverseOrder(              );}
void _MemcThreadSafe::randomizeOrder(                           ) {SyncLocker locker(_lock); _memc.randomizeOrder(              );}
void _MemcThreadSafe::   rotateOrder(Int offset                 ) {SyncLocker locker(_lock); _memc.   rotateOrder(offset        );}
void _MemcThreadSafe::     swapOrder(Int i  , Int j             ) {SyncLocker locker(_lock); _memc.     swapOrder(i, j          );}
void _MemcThreadSafe::     moveElm  (Int elm, Int new_index     ) {SyncLocker locker(_lock); _memc.     moveElm  (elm, new_index);}
/******************************************************************************/
}
/******************************************************************************/
