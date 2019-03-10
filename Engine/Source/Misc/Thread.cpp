/******************************************************************************

   'ReadWriteSync' is better than 'std::shared_mutex' because:
      -allows for 'enterRead'  followed by 'enterWrite' on the same thread, 'shared_mutex' does not allow this
      -allows for 'enterWrite' followed by 'enterRead'  on the same thread, 'shared_mutex' does not allow this
      -it's Write-preferring (all new readers blocked when a writer was requested), 'shared_mutex' preference is unspecified

/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define STACK_SIZE (1024*1024) // set 1MB stack size
#if !HAS_THREADS
ThreadEmulation EmulatedThreads;
#endif
/******************************************************************************/
// 32-bit AtomicGet and AtomicSet are simple:
// https://github.com/mintomic/mintomic/tree/master/include/mintomic/private

#if WINDOWS // versions with _ are faster than those without
#if !X64
  Long AtomicGet(C Long &x) {Long old=x;        return _InterlockedCompareExchange64((LONG64*)&x, old, old);} // this version was the fastest
//Long AtomicGet(C Long &x) {                   return _InterlockedCompareExchange64((LONG64*)&x,   0,   0);} // slightly slower (this can work with 0 because it compares x to 0 and sets 0 only then) but always returns the old value despite if it was zero or not
//Long AtomicGet(C Long &x) {Long old; do old=x; while(_InterlockedCompareExchange64((LONG64*)&x, old, old)!=old); return old;}

void AtomicSet(Long &x, Long y) {Long old; do old=x; while(_InterlockedCompareExchange64((LONG64*)&x, y, old)!=old);}
#endif

Int AtomicInc(Int &x) {return _InterlockedIncrement((LONG*)&x)-1;} // 'InterlockedIncrement' returns the new value
Int AtomicDec(Int &x) {return _InterlockedDecrement((LONG*)&x)+1;} // 'InterlockedDecrement' returns the new value

Int AtomicAdd(Int &x, Int y) {return _InterlockedExchangeAdd((LONG*)&x,  y);} // 'InterlockedExchangeAdd' returns the old value
Int AtomicSub(Int &x, Int y) {return _InterlockedExchangeAdd((LONG*)&x, -y);} // 'InterlockedExchangeAdd' returns the old value

#if X64
Long AtomicAdd(Long &x, Long y) {return _InterlockedExchangeAdd64((LONG64*)&x,  y);} // 'InterlockedExchangeAdd64' returns the old value
Long AtomicSub(Long &x, Long y) {return _InterlockedExchangeAdd64((LONG64*)&x, -y);} // 'InterlockedExchangeAdd64' returns the old value
#else
Long AtomicAdd(Long &x, Long y) {return InterlockedExchangeAdd64((LONG64*)&x,  y);} // 'InterlockedExchangeAdd64' returns the old value
Long AtomicSub(Long &x, Long y) {return InterlockedExchangeAdd64((LONG64*)&x, -y);} // 'InterlockedExchangeAdd64' returns the old value
#endif

Int AtomicAnd    (Int &x, Int y) {return _InterlockedAnd((LONG*)&x,  y);} // 'InterlockedAnd' returns the old value
Int AtomicDisable(Int &x, Int y) {return _InterlockedAnd((LONG*)&x, ~y);} // 'InterlockedAnd' returns the old value
Int AtomicOr     (Int &x, Int y) {return _InterlockedOr ((LONG*)&x,  y);} // 'InterlockedOr'  returns the old value
Int AtomicXor    (Int &x, Int y) {return _InterlockedXor((LONG*)&x,  y);} // 'InterlockedXor' returns the old value

Int AtomicGetSet(Int &x, Int y) {return _InterlockedExchange((LONG*)&x, y);} // 'InterlockedExchange' returns the old value

Bool AtomicCAS(Int  &x, Int  compare, Int  new_value) {return _InterlockedCompareExchange  ((LONG  *)&x,       new_value,       compare)==      compare;} // 'InterlockedCompareExchange' returns the old value
Bool AtomicCAS(Long &x, Long compare, Long new_value) {return _InterlockedCompareExchange64((LONG64*)&x,       new_value,       compare)==      compare;} // 'InterlockedCompareExchange' returns the old value
Bool AtomicCAS(Flt  &x, Flt  compare, Flt  new_value) {return _InterlockedCompareExchange  ((LONG  *)&x, (Int&)new_value, (Int&)compare)==(Int&)compare;} // 'InterlockedCompareExchange' returns the old value
#else
#if !X64
  Long AtomicGet(C Long &x) {Long old=x; return          __sync_val_compare_and_swap(&ConstCast(x), old, old);}              // 'ConstCast' is used to mute a warning, it can be used because 'x' will be modified to 'old' only if it's equal to 'old' already
//Long AtomicGet(C Long &x) {            return          __sync_val_compare_and_swap(&ConstCast(x),   0,   0);}              // 'ConstCast' is used to mute a warning, it can be used because 'x' will be modified to 'old' only if it's equal to 'old' already
//Long AtomicGet(C Long &x) {Long old; do old=x; while(!__sync_bool_compare_and_swap(&ConstCast(x), old, old)); return old;} // 'ConstCast' is used to mute a warning, it can be used because 'x' will be modified to 'old' only if it's equal to 'old' already

void AtomicSet(Long &x, Long y) {Long old; do old=x; while(!__sync_bool_compare_and_swap(&x, old, y));}
#endif

Int AtomicInc(Int &x) {return __sync_fetch_and_add(&x, +1);} // '__sync_fetch_and_add' returns the old value
Int AtomicDec(Int &x) {return __sync_fetch_and_add(&x, -1);} // '__sync_fetch_and_add' returns the old value

Int AtomicAdd(Int &x, Int y) {return __sync_fetch_and_add(&x, y);} // '__sync_fetch_and_add' returns the old value
Int AtomicSub(Int &x, Int y) {return __sync_fetch_and_sub(&x, y);} // '__sync_fetch_and_sub' returns the old value

Long AtomicAdd(Long &x, Long y) {return __sync_fetch_and_add(&x, y);} // '__sync_fetch_and_add' returns the old value
Long AtomicSub(Long &x, Long y) {return __sync_fetch_and_sub(&x, y);} // '__sync_fetch_and_sub' returns the old value

Int AtomicAnd    (Int &x, Int y) {return __sync_fetch_and_and(&x,  y);} // '__sync_fetch_and_and' returns the old value
Int AtomicDisable(Int &x, Int y) {return __sync_fetch_and_and(&x, ~y);} // '__sync_fetch_and_and' returns the old value
Int AtomicOr     (Int &x, Int y) {return __sync_fetch_and_or (&x,  y);} // '__sync_fetch_and_or'  returns the old value
Int AtomicXor    (Int &x, Int y) {return __sync_fetch_and_xor(&x,  y);} // '__sync_fetch_and_xor' returns the old value

Int AtomicGetSet(Int &x, Int y) {return __sync_lock_test_and_set(&x, y);} // '__sync_lock_test_and_set' returns the old value

Bool AtomicCAS(Int  &x, Int  compare, Int  new_value) {return __sync_bool_compare_and_swap(      &x,       compare,       new_value);}
Bool AtomicCAS(Long &x, Long compare, Long new_value) {return __sync_bool_compare_and_swap(      &x,       compare,       new_value);}
Bool AtomicCAS(Flt  &x, Flt  compare, Flt  new_value) {return __sync_bool_compare_and_swap((Int*)&x, (Int&)compare, (Int&)new_value);}
#endif
/******************************************************************************/
#undef  GetThreadId
UIntPtr Thread::id()C {return PLATFORM(::GetThreadId(_handle), (UIntPtr)_handle);}
UIntPtr GetThreadId() {return _GetThreadId();}
#define GetThreadId _GetThreadId

#if WINDOWS_OLD
UIntPtr GetThreadIdFromWindow(Ptr hwnd) {return GetWindowThreadProcessId((HWND)hwnd, null);}
#else
UIntPtr GetThreadIdFromWindow(Ptr hwnd) {return (hwnd==App.hwnd()) ? App.threadID() : 0;}
#endif
/******************************************************************************/
void SetThreadName(C Str8 &name, UIntPtr thread_id)
{
#if WINDOWS
   #pragma pack(push, 8)
   struct ThreadName
   {
      DWORD  dwType;
      LPCSTR szName;
      DWORD  dwThreadID;
      DWORD  dwFlags;
   };
   #pragma pack(pop)

   ThreadName info;
   info.dwType    =0x1000;
   info.szName    =name;
   info.dwThreadID=thread_id;
   info.dwFlags   =0;

   __try{RaiseException(0x406D1388, 0, SIZE(info)/SIZE(ULONG_PTR), (ULONG_PTR*)&info);}
   __except(EXCEPTION_EXECUTE_HANDLER) {}
   // TODO: check 'SetThreadDescription'
#elif APPLE
   if(thread_id==GetThreadId()) // on Apple can set the name of current thread only
   {
   #if 1 // works the same but less overhead
      pthread_setname_np(name); // doesn't crash if 'name' is null
   #else
      if(NSStringAuto ns_name=name)[[NSThread currentThread] setName:ns_name];
   #endif
   }
#elif ANDROID || LINUX
   pthread_setname_np((pthread_t)thread_id, name ? name() : ""); // 'pthread_setname_np' will crash on Linux if `name` is null
#endif
}
/******************************************************************************/
// SYNCHRONIZATION LOCK
/******************************************************************************/
#if !HAS_THREADS
        SyncLock::~SyncLock()  {_is=false;}
        SyncLock:: SyncLock()  {_lock_count=0; _owner=0; _is=true;}
   Bool SyncLock:: tryOn   ()C {on(); return true;}
   void SyncLock:: on      ()C {if(  _lock_count++==0)_owner=GetThreadId();}
   void SyncLock:: off     ()C {if(--_lock_count  ==0)_owner=0;}
   Bool SyncLock:: owned   ()C {return _lock_count>0;}
   Bool SyncLock:: created ()C {return _is!=0;}
#elif WINDOWS
      Bool SyncLock::owned  ()C {return _lock.OwningThread==(HANDLE)GetThreadId();}
      Bool SyncLock::created()C {return _lock.DebugInfo!=null;}

   #if SUPPORT_WINDOWS_XP
      static BOOL (WINAPI *InitializeCriticalSectionEx)(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount, DWORD Flags);
      static Bool          InitializeCriticalSectionExTried;
      SyncLock::SyncLock()
      {
         if(InitializeCriticalSectionEx)
         {
         ex:
            InitializeCriticalSectionEx(&_lock, 0, CRITICAL_SECTION_NO_DEBUG_INFO);
         }else
         {
            if(!InitializeCriticalSectionExTried)
            {
               if(HMODULE kernel=GetModuleHandle(L"Kernel32.dll"))InitializeCriticalSectionEx=(decltype(InitializeCriticalSectionEx))GetProcAddress(kernel, "InitializeCriticalSectionEx"); // available on Vista+
               InitializeCriticalSectionExTried=true;
               if(InitializeCriticalSectionEx)goto ex;
            }
            InitializeCriticalSection(&_lock);
         }
         DEBUG_ASSERT(created(), "SyncLock");
      }
   #else
      SyncLock:: SyncLock() {InitializeCriticalSectionEx(&_lock, 0, CRITICAL_SECTION_NO_DEBUG_INFO); DEBUG_ASSERT(created(), "SyncLock");}
   #endif
      SyncLock::~SyncLock() {    DeleteCriticalSection  (&_lock); DEBUG_ASSERT(!created(), "SyncLock");}

   #if SYNC_LOCK_SAFE
      Bool SyncLock::tryOn()C {return created() ? TryEnterCriticalSection(&_lock)!=0 : false;}
      void SyncLock::on   ()C {    if(created())     EnterCriticalSection(&_lock);}
      void SyncLock::off  ()C {    if(created())     LeaveCriticalSection(&_lock);}
   #else
      Bool SyncLock::tryOn()C {return TryEnterCriticalSection(&_lock);}
      void SyncLock::on   ()C {          EnterCriticalSection(&_lock);}
      void SyncLock::off  ()C {          LeaveCriticalSection(&_lock);}
   #endif
#else
#define CUSTOM_RECURSIVE 0
Bool SyncLock::created()C {return _is!=0;}

SyncLock::SyncLock()
{
#if CUSTOM_RECURSIVE
   pthread_mutex_init(&_lock, null);
#else
   pthread_mutexattr_t                attr;
   pthread_mutexattr_init   (        &attr);
   pthread_mutexattr_settype(        &attr, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init       (&_lock, &attr);
   pthread_mutexattr_destroy(        &attr);
#endif
  _lock_count=0;
  _owner=0;
  _is=true;
}
SyncLock::~SyncLock()
{
   if(owned())REP(_lock_count)off();
  _is=false;
   pthread_mutex_destroy(&_lock);
  _lock_count=0;
  _owner=0;
}
Bool SyncLock::owned()C
{
   return _lock_count>0 && _owner==GetThreadId();
}
Bool SyncLock::tryOn()C
{
#if SYNC_LOCK_SAFE
   if(created())
#endif
   {
   #if CUSTOM_RECURSIVE
      if(owned())
      {
        _lock_count++;
         return true;
      }
      if(pthread_mutex_trylock(&_lock)==0)
      {
        _owner=GetThreadId();
        _lock_count++;
         return true;
      }
   #else
      if(pthread_mutex_trylock(&_lock)==0)
      {
         if(!_lock_count)_owner=GetThreadId();
             _lock_count++;
         return true;
      }
   #endif
   }
   return false;
}
void SyncLock::on()C
{
#if SYNC_LOCK_SAFE
   if(created())
#endif
   {
   #if CUSTOM_RECURSIVE
      if(!owned())
      {
         pthread_mutex_lock(&_lock);
        _owner=GetThreadId();
      }
     _lock_count++;
   #else
      pthread_mutex_lock(&_lock);
      if(!_lock_count)_owner=GetThreadId();
          _lock_count++;
   #endif
   }
}
void SyncLock::off()C
{
#if SYNC_LOCK_SAFE
   if(created())
#endif
   {
   #if CUSTOM_RECURSIVE
         _lock_count--;
      if(_lock_count<=0)
      {
        _owner=0;
         pthread_mutex_unlock(&_lock);
      }
   #else
         _lock_count--;
      if(_lock_count<=0)_owner=0;
      pthread_mutex_unlock(&_lock);
   #endif
   }
}
#endif
/******************************************************************************/
// SYNCHRONIZATION EVENT
/******************************************************************************/
#if !HAS_THREADS
     SyncEvent:: SyncEvent(Bool auto_off   )  {_handle=null; _condition=false; _auto_off=auto_off;}
     SyncEvent::~SyncEvent(                )  {}
void SyncEvent:: on       (                )C {_condition=true ;}
void SyncEvent:: off      (                )C {_condition=false;}
Bool SyncEvent:: wait     (                )C {Bool ok=_condition; if(_auto_off)_condition=false; return ok;}
Bool SyncEvent:: wait     (Int milliseconds)C {Bool ok=_condition; if(_auto_off)_condition=false; return ok;}
#elif WINDOWS
     SyncEvent:: SyncEvent(Bool auto_off   )  {     _handle=                 CreateEvent(null, !auto_off, false, null);               }
     SyncEvent::~SyncEvent(                )  {  if(_handle){                CloseHandle(_handle                     ); _handle=null;}}
void SyncEvent:: on       (                )C {/*if(_handle)*/                  SetEvent(_handle                     );               }                                         // checking for handle!=null is not needed, as the function will do nothing on null
void SyncEvent:: off      (                )C {/*if(_handle)*/                ResetEvent(_handle                     );               }                                         // checking for handle!=null is not needed, as the function will do nothing on null
Bool SyncEvent:: wait     (                )C {/*if(_handle)*/return WaitForSingleObject(_handle,                                    INFINITE)!=WAIT_TIMEOUT; return true;} // checking for handle!=null is not needed, as the function will return -1  on null, WAIT_TIMEOUT=258 so result will be -1!=258 -> true
Bool SyncEvent:: wait     (Int milliseconds)C {/*if(_handle)*/return WaitForSingleObject(_handle, (milliseconds>=0) ? milliseconds : INFINITE)!=WAIT_TIMEOUT; return true;} // checking for handle!=null is not needed, as the function will return -1  on null, WAIT_TIMEOUT=258 so result will be -1!=258 -> true
#else
SyncEvent::SyncEvent(Bool auto_off)
{
  _condition=false; _auto_off=auto_off;
   Alloc(_handle); pthread_cond_init (_handle, null);
   Alloc(_mutex ); pthread_mutex_init(_mutex , null);
}
SyncEvent::~SyncEvent()
{
   if(_handle)
   {
      pthread_mutex_lock   (_mutex); _condition=false; pthread_cond_destroy(_handle); Free(_handle);
      pthread_mutex_unlock (_mutex);
      pthread_mutex_destroy(_mutex); Free(_mutex);
   }
}
void SyncEvent::on()C
{
   if(_handle)
   {
      pthread_mutex_lock  (_mutex); _condition=true; if(_auto_off)pthread_cond_signal(_handle);else pthread_cond_broadcast(_handle); // _auto_off ? wake up 1 : wake up all
      pthread_mutex_unlock(_mutex);
   }
}
void SyncEvent::off()C
{
   if(_handle)
   {
      pthread_mutex_lock  (_mutex); _condition=false;
      pthread_mutex_unlock(_mutex);
   }
}
Bool SyncEvent::wait()C
{
   if(_handle)
   {
      pthread_mutex_lock(_mutex);
      for(; !_condition && !pthread_cond_wait(_handle, _mutex); ); // check for '_condition' first in case it's already met to avoid 'pthread_cond_wait' overhead, have to check in a loop because 'pthread_cond_wait' may return if OS interrupted it, 'pthread_cond_wait' automatically unlocks and locks the mutex
      Bool ok=_condition;
      if(_auto_off)_condition=false;
      pthread_mutex_unlock(_mutex);
      return ok;
   }
   return true;
}
Bool SyncEvent::wait(Int milliseconds)C
{
   if(_handle)
   {
      pthread_mutex_lock(_mutex);
      if(!_condition) // if condition not met yet
      {
         if(milliseconds<0) // infinite wait
         {
            for(; !_condition && !pthread_cond_wait(_handle, _mutex); ); // check for '_condition' first in case it's already met to avoid 'pthread_cond_wait' overhead, have to check in a loop because 'pthread_cond_wait' may return if OS interrupted it, 'pthread_cond_wait' automatically unlocks and locks the mutex
         }else
         if(milliseconds>0) // timed wait
         {
            timeval  tv; gettimeofday(&tv, null);
            timespec ts; ts.tv_sec  =tv.tv_sec      ;
                         ts.tv_nsec =tv.tv_usec*1000;

                         ts.tv_nsec+=(milliseconds%1000)*1000000;
                         ts.tv_sec +=(milliseconds/1000) + ts.tv_nsec/1000000000;
                         ts.tv_nsec%=1000000000;

            // 'ts' specifies the end time at which waiting always fails (this is the "time position" and not "time duration")
            for(; !_condition && !pthread_cond_timedwait(_handle, _mutex, &ts); ); // 'pthread_cond_timedwait' automatically unlocks and locks the mutex, keep waiting in the loop in case it returns multiple times before the end of the time, but somehow with the condition still set to false, in that case keep waiting still as long as it returns success and not timeout or other error
         }
      }
      Bool ok=_condition;
      if(_auto_off)_condition=false;
      pthread_mutex_unlock(_mutex);
      return ok;
   }
   return true;
}
#endif
/******************************************************************************/
// SYNC COUNTER
/******************************************************************************/
#if !HAS_THREADS
     SyncCounter:: SyncCounter(                )  {_handle=null; _counter=0;}
     SyncCounter::~SyncCounter(                )  {}
void SyncCounter:: operator+= (Int count       )C {_counter=Mid(Long(_counter)+count, (Long)INT_MIN, (Long)INT_MAX);} // clamp to prevent overflow
Bool SyncCounter:: wait       (                )C {if(_counter>0){_counter--; return true;} return false;}
Bool SyncCounter:: wait       (Int milliseconds)C {if(_counter>0){_counter--; return true;} return false;}
#elif WINDOWS
     SyncCounter:: SyncCounter(                )  {     _handle=             CreateSemaphore(null, 0, INT_MAX, null);}
     SyncCounter::~SyncCounter(                )  {  if(_handle){                CloseHandle(_handle               ); _handle=null;}}
void SyncCounter:: operator+= (Int count       )C {/*if(_handle)*/          ReleaseSemaphore(_handle, count, null  );}                                                          // checking for handle!=null is not needed, as the function will do nothing on null
Bool SyncCounter:: wait       (                )C {/*if(_handle)*/return WaitForSingleObject(_handle,                                    INFINITE)!=WAIT_TIMEOUT; return true;} // checking for handle!=null is not needed, as the function will return -1  on null, WAIT_TIMEOUT=258 so result will be -1!=258 -> true
Bool SyncCounter:: wait       (Int milliseconds)C {/*if(_handle)*/return WaitForSingleObject(_handle, (milliseconds>=0) ? milliseconds : INFINITE)!=WAIT_TIMEOUT; return true;} // checking for handle!=null is not needed, as the function will return -1  on null, WAIT_TIMEOUT=258 so result will be -1!=258 -> true
#else
SyncCounter::SyncCounter()
{
  _counter=0;
   Alloc(_handle); pthread_cond_init (_handle, null);
   Alloc(_mutex ); pthread_mutex_init(_mutex , null);
}
SyncCounter::~SyncCounter()
{
   if(_handle)
   {
      pthread_mutex_lock   (_mutex); _counter=0; pthread_cond_destroy(_handle); Free(_handle);
      pthread_mutex_unlock (_mutex);
      pthread_mutex_destroy(_mutex); Free(_mutex);
   }
}
void SyncCounter::operator+=(Int count)C
{
   if(_handle)
   {
      pthread_mutex_lock  (_mutex); _counter=Mid(Long(_counter)+count, (Long)INT_MIN, (Long)INT_MAX); if(count>1)pthread_cond_broadcast(_handle);else pthread_cond_signal(_handle); // clamp to prevent overflow, if adding more than 1 then call 'pthread_cond_broadcast' to unlock all waiting threads, 'pthread_cond_signal' will unlock only 1
      pthread_mutex_unlock(_mutex);
   }
}
Bool SyncCounter::wait()C
{
   if(_handle)
   {
      pthread_mutex_lock(_mutex);
      for(; _counter<=0 && !pthread_cond_wait(_handle, _mutex); ); // check for '_condition' first in case it's already met to avoid 'pthread_cond_wait' overhead, have to check in a loop because 'pthread_cond_wait' may return if OS interrupted it, 'pthread_cond_wait' automatically unlocks and locks the mutex
      Bool ok=false; if(_counter>0){_counter--; ok=true;}
      pthread_mutex_unlock(_mutex);
      return ok;
   }
   return true;
}
Bool SyncCounter::wait(Int milliseconds)C
{
   if(_handle)
   {
      pthread_mutex_lock(_mutex);
      if(_counter<=0) // if condition not met yet
      {
         if(milliseconds<0) // infinite wait
         {
            for(; _counter<=0 && !pthread_cond_wait(_handle, _mutex); ); // check for '_condition' first in case it's already met to avoid 'pthread_cond_wait' overhead, have to check in a loop because 'pthread_cond_wait' may return if OS interrupted it, 'pthread_cond_wait' automatically unlocks and locks the mutex
         }else
         if(milliseconds>0) // timed wait
         {
            timeval  tv; gettimeofday(&tv, null);
            timespec ts; ts.tv_sec  =tv.tv_sec      ;
                         ts.tv_nsec =tv.tv_usec*1000;

                         ts.tv_nsec+=(milliseconds%1000)*1000000;
                         ts.tv_sec +=(milliseconds/1000) + ts.tv_nsec/1000000000;
                         ts.tv_nsec%=1000000000;

            // 'ts' specifies the end time at which waiting always fails (this is the "time position" and not "time duration")
            for(; _counter<=0 && !pthread_cond_timedwait(_handle, _mutex, &ts); ); // 'pthread_cond_timedwait' automatically unlocks and locks the mutex, keep waiting in the loop in case it returns multiple times before the end of the time, but somehow with the condition still set to false, in that case keep waiting still as long as it returns success and not timeout or other error
         }
      }
      Bool ok=false; if(_counter>0){_counter--; ok=true;}
      pthread_mutex_unlock(_mutex);
      return ok;
   }
   return true;
}
#endif
/******************************************************************************/
// READER WRITER SYNC
/******************************************************************************/
void ReadWriteSync::enterRead()
{
   UIntPtr thread_id=GetThreadId();

   // check if this thread already has a lock present, it's important to test without '_write_lock' yet, in case some thread called 'enterWrite'
   if(_locks.elms()) // this is safe because we're interested only in locks from this thread, so if this thread has made a lock before, then it will be available here without the need of enabling '_locks_lock'
   {
     _locks_lock.on();
      REPA(_locks) // iterate all locks
      {
         Lock &lock=_locks[i];
         if(   lock.thread_id==thread_id){lock.locks++; _locks_lock.off(); return;} // if found an existing for this thread, then increase the lock counter, unlock, and return immediately without unlocking again below
      }
     _locks_lock.off(); // unlock this first, because we need to lock '_write_lock' before '_locks_lock' when adding a new lock
   }

   // create new lock on this thread
  _write_lock.on (); // this prevents creating a new reader lock when a different thread has called 'enterWrite'
  _locks_lock.on (); Lock &l=_locks.New(); l.thread_id=thread_id; l.locks=1; // even though we've temporarily unlocked '_locks_lock' above, we can always create a new lock here, without checking again if there's already listed, because even if other thread will add a new lock, it will always be a different thread's lock
  _locks_lock.off();
  _write_lock.off();
}
void ReadWriteSync::leaveRead()
{
 //if(_locks.elms()) don't check this, because if we're calling 'leaveRead', then most likely we are locked
   {
      UIntPtr thread_id=GetThreadId();
     _locks_lock.on();
      REPA(_locks)
      {
         Lock &lock=_locks[i];
         if(   lock.thread_id==thread_id)
         {
            if(--lock.locks<=0) // this is the last lock
            {
              _locks  .remove(i); // remove it first
              _locks_lock .off(); // unlock before signaling
              _left_reading.on(); // signal that we've finished reading
               return; // return immediately without unlocking again below
            }
            break; // we've found a lock for this thread, so no need to look any further
         }
      }
     _locks_lock.off();
   }
}

void ReadWriteSync::enterWrite()
{
   UIntPtr thread_id=GetThreadId();
  _write_lock.on(); // block adding new 'readers' and 'writers'

   // wait until all 'readers' from other threads will exit
again:
   if(_locks.elms())
   {
     _locks_lock.on();
      REPA(_locks)if(_locks[i].thread_id!=thread_id) // if found at least one reader from another thread
      {
        _locks_lock.off();    // unlock first before waiting
        _left_reading.wait(); // wait for any thread to finish reading
         goto again;          // check again
      }
     _locks_lock.off(); // no readers were found, unlock and return
   }
}
void ReadWriteSync::leaveWrite()
{
  _write_lock.off();
}

Bool ReadWriteSync::ownedRead()
{
   if(_locks.elms())
   {
      UIntPtr thread_id=GetThreadId();
      SyncLocker lock(_locks_lock); REPA(_locks)if(_locks[i].thread_id==thread_id)return true;
   }
   return false;
}
/******************************************************************************/
// SIMPLE READER WRITER SYNC
/******************************************************************************/
void SimpleReadWriteSync::enterWrite()C
{
  _lock.on();
   for(; AtomicGet(_readers); )_finished.wait(); // wait for readers to finish
}
void SimpleReadWriteSync::leaveWrite()C
{
  _lock.off();
}
void SimpleReadWriteSync::enterRead()C
{
   SyncLocker lock(_lock); // make sure there is no writer
   AtomicInc(_readers);
}
void SimpleReadWriteSync::leaveRead()C
{
   if(AtomicDec(_readers)==1)_finished.on();
}
/******************************************************************************/
// THREAD
/******************************************************************************/
#if HAS_THREADS
#if WINDOWS
static unsigned int WINAPI ThreadFunc(Ptr user)
#else
static Ptr ThreadFunc(Ptr user)
#endif
{
   // !! Do not use objects with destructors here because '_endthreadex' below does not allow for destructors !!
   ((Thread*)user)->func();
#if WINDOWS
  _endthreadex(0);
#endif
   return 0;
}
#endif
void Thread::func()
{
   Cpu.set();
#if APPLE
   if(_name.is()){SetThreadName(_name); _name.del();} // delete because it's not needed anymore
#endif
again:
   for(; !wantStop(); ) // main thread loop
   {
      if(wantPause())
      {
        _paused=true;
        _resume.wait();
        _paused=false;
         goto again; // check if after unpause we want to stop or pause again
      }

      if(!_func(T))break;
   }
   ThreadFinishedUsingGPUData(); // automatically disable OpenGL context if it was enabled, deactivating any active context on a thread is required before exiting the thread
#if !WINDOWS
  _finished.on(); // !! we have to notify of finish before clearing '_active', because '_finished.on' operates on SyncEvent, so it needs to be valid until this 'on' call finishes, if we would clear '_active' before that, then this 'Thread' memory could have been already released and we would be operating on invalid memory !!
#endif
  _active=false;
   // !! do not perform any operations after clearing '_active' because this object could have been already deleted !!
}
void Thread::zero()
{
   user=null;
  _want_stop=_want_pause=_paused=_active=false;
  _priority=0;
  _handle=NULL;
  _func=null;
}
void Thread::stop()
{
   if(active())
   {
   #if HAS_THREADS
     _want_stop=true;
      resume();
   #else
      EmulatedThreads.exclude(T);
     _active=false;
   #endif
   }
}
void Thread::cancelStop()
{
#if HAS_THREADS
  _want_stop=false;
#endif
}
void Thread::pause()
{
   if(active())
   {
     _want_pause=true;
   #if !HAS_THREADS
      if(!_paused) // ignore if it's already paused
      {
        _paused=true;
         EmulatedThreads.exclude(T);
      }
   #endif
   }
}
void Thread::resume()
{
   if(_want_pause) // no need to check for 'active' because '_want_pause' can be enabled only for active threads
   {
     _want_pause=false; // disable this as soon as possible
   #if !HAS_THREADS
      if(_paused)
      {
        _paused=false;
         EmulatedThreads.include(T);
      }
   #else
     _resume.on();
   #endif
   }
}
#if HAS_THREADS && !WINDOWS
#define PRIORITY_POLICY (APPLE ? SCHED_OTHER : SCHED_RR) // SCHED_OTHER gives better results on Apple but it's not available on Android/Linux, SCHED_RR (Mac/iOS 15..47, Android 1..99), SCHED_OTHER (Mac/iOS 15..47, Android 0..0)
static const Int PriorityBase =sched_get_priority_min(PRIORITY_POLICY),
                 PriorityRange=sched_get_priority_max(PRIORITY_POLICY)-PriorityBase;
/* Tested using following program:
const int PR=3;
int  v[PR*2+1]; Memx<Thread> threads;
bool Func(Thread &t) {AtomicInc(v[t.priority()+PR]); return true;}
void InitPre() {App.flag|=APP_WORK_IN_BACKGROUND; DataPath("../Data");}
bool Init   () {for(int p=-PR; p<=PR; p++)REP(2)threads.New().create(Func, null, p, true); FREPAO(threads).resume(); return true;}
void Shut   () {threads.del();}
bool Update () {if(Kb.bp(KB_ESC))return false; return true;}
void Draw   () {D.clear(TURQ); Str s; int max=0; REPA(v)MAX(max, v[i]); FREPA(v){if(i)s+=", "; s+=Flt(v[i])/max;} D.text(0, 0, s);}
*/
#endif
void Thread::priority(Int priority)
{
   Clamp(priority, -3, 3);
   if(active() && T._priority!=priority)
   {
      T._priority=priority;
   #if HAS_THREADS
      #if WINDOWS
         ASSERT(THREAD_PRIORITY_LOWEST==-2 && THREAD_PRIORITY_HIGHEST==2);
         SetThreadPriority(_handle, (priority<-2) ? THREAD_PRIORITY_IDLE : (priority>2) ? THREAD_PRIORITY_TIME_CRITICAL : priority);
      #else
       //LogN(S+PriorityBase+' '+(PriorityBase+PriorityRange));
         sched_param param; param.sched_priority=PriorityBase+PriorityRange*(priority+3)/6; // div by 6 because "priority==3" should give max
         pthread_setschedparam(_handle, PRIORITY_POLICY, &param);
      #endif
   #endif
   }
}
void Thread::kill()
{
   if(created())
   {
      if(active())
      {
      #if WINDOWS_NEW || ANDROID // WINDOWS_NEW doesn't have 'TerminateThread', ANDROID doesn't have 'pthread_cancel'
         stop(); resume(); wait();
      #elif WINDOWS_OLD
         TerminateThread(_handle, 0);
      #else
         pthread_cancel(_handle);
      #endif
      }
   #if WINDOWS
      CloseHandle(_handle);
   #else
    //pthread_join(_handle, null); this has to be called for joinable threads to release system resources, but we use detached, which auto-release when threads finish
     _handle=NULL;
   //_finished.off(); don't do this here, instead do it in 'create', to give other threads waiting for this one longer chance to detect finish
   #endif
     _resume.off();
      zero();
   }
}
Bool Thread::wait(Int time)
{
   if(active())
   {
   #if !HAS_THREADS
      if(time<0) // infinite wait
      {
         for(; active(); )EmulatedThreads.update();
      }else
      if(!time)EmulatedThreads.update();else // single step
      {
         // timed wait
         for(UInt start=Time.curTimeMs(); ; )
         {
            EmulatedThreads.update();
            if(!active() || (Time.curTimeMs()-start)>=time)break; // this code was tested OK for UInt overflow
         }
      }
      return !active();
   #elif WINDOWS
      return WaitForSingleObject(_handle, (time>=0) ? time : INFINITE)!=WAIT_TIMEOUT;
   #else
      return _finished.wait(time);
   #endif
   }
   return true;
}
void Thread::del(Int time)
{
   if(created())
   {
      if(active())
      {
         stop  (    ); if(_priority<0)priority(0);
         resume(    );
         wait  (time);
      }
      kill();
   }
}
Bool Thread::create(Bool func(Thread &thread), Ptr user, Int priority, Bool paused, C Str8 &name)
{
   del(); if(!func)return true;

   T._active    =true;
   T._func      =func;
   T. user      =user;
   T._want_pause=paused;
 //T._sleep     =Max(0, sleep);

#if !HAS_THREADS
   T._handle=pthread_t(this); // set dummy handle, which is used in 'created' method
   T._paused=paused; if(!paused)EmulatedThreads.include(T);
#elif WINDOWS
 //if(_handle=          CreateThread(null, STACK_SIZE, ThreadFunc, this, CREATE_SUSPENDED, null)) this causes memory leaks on some systems
   if(_handle=(HANDLE)_beginthreadex(null, STACK_SIZE, ThreadFunc, this, CREATE_SUSPENDED, null)) // threads are always created in suspended mode on Windows to access the 'handle' first and only then resume the thread
#else
   pthread_attr_t attr;
   Bool           ok=false;

   if(!pthread_attr_init(&attr))
   {
   #if APPLE
      T._name=name; // keep for Apple because on that platform we can set the name only when called from that thread
   #endif
      if(!pthread_attr_setstacksize  (&attr, STACK_SIZE))
      if(!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) // create as detached to auto-release system resources without having to call 'pthread_join'
      {
        _finished.off(); // reset here instead when deleting to give other threads a longer chance to wait for finish
         if(!pthread_create(&_handle, &attr, ThreadFunc, this))ok=true;
      }
      pthread_attr_destroy(&attr);
   }

   if(ok)
#endif
   {
   #if !APPLE
      SetThreadName(name, id()); // for non-Apple platforms we can change the name from any thread
   #endif
      T.priority(priority);
   #if WINDOWS
      ResumeThread(_handle); // CREATE_SUSPENDED was used so resume it
   #endif
      return true;
   }
   zero(); return false;
}
Thread::Thread()
#if !WINDOWS
   : _finished(false) // disable 'auto_off' to allow multiple threads waiting for this one to be woken up
#endif
{
   zero();
}
Thread::Thread(Bool func(Thread &thread), Ptr user, Int priority, Bool paused)
{
   zero(); create(func, user, priority, paused);
}
/******************************************************************************/
// THREADS
/******************************************************************************/
enum THREADS_FUNC_MODE : Byte
{
   TF_INDEX,
   TF_DATA,
   TF_DATA_PTR,
   TF_MEMB,
   TF_MEMX,
};
/******************************************************************************/
INLINE Bool Threads::callsLeft() {return _calls.elms()>0;} // this check is OK, we don't need to compare against '_calls_pos' because in all places we call 'checkEnd' which will clear '_calls' if we reached the end
/******************************************************************************/
static Bool ThreadsFunc(Threads::ThreadEx &thread)
{
   Threads &threads=*(Threads*)thread.user;
   Int      thread_index=threads._threads.index(&thread);

   // process priority elements first
   Int processed=0; // !! in first part set this only to priority elements (from 'process' methods), because it will be added to '_processed' !!
   if(threads._left>0) // have elements to process
   {
   process:
      Int index=AtomicDec(threads._left);
      if( index>0)
      {
         index=threads._elms-index; // process elements starting from zero index
         Ptr data;
         switch(threads._func_mode)
         {
            case TF_INDEX   : data=                               Ptr(index)                  ; break;
            case TF_DATA    : data=        (Byte*)threads._func_data+ index*threads._elm_size ; break;
            case TF_DATA_PTR: data=*(Ptr*)((Byte*)threads._func_data+ index*threads._elm_size); break;
            case TF_MEMB    : data=             (*threads._func_memb)[index]                  ; break;
            case TF_MEMX    : data=             (*threads._func_memx)[index]                  ; break;
         }
         threads._func(data, threads._func_user, thread_index);
       //if(thread.wantStop())return false; not needed since '_left' is zeroed at the start of 'Threads.del'
         processed++;
         goto process; // proceed to next element
      }
      if(processed)
         if(AtomicAdd(threads._processed, processed) // set what was processed by this thread, do this in just one atomic operation (and not many times inside the loop)
          +processed>=threads._elms) // add the 'AtomicAdd' result (which is the old value of 'threads._processed') and 'processed' to get the total number of elements processed at this time
            threads._finished.on(); // notify of finished processing only after all elements have been finished (to avoid overhead of finished notification on every thread, and the waiting thread being woken up multiple times)
   }

   // process queued elements next
   if(threads.callsLeft())
   {
      SyncLockerEx locker(threads._lock_calls);
      if(threads.callsLeft())
      {
         if(threads._ordered){thread.call=threads._calls[threads._calls_pos++]; threads.checkEnd();}
         else                {thread.call=threads._calls.last(); threads._calls.removeLast();}
         // can't check 'threads._waiting' here during lock, because 'Threads.wait' could happen later, during the 'call.call'
         locker.off();
         thread.call.call (thread_index);
         thread.call.clear(); // clear as soon as finished processing so other threads checking for this can see that it's now empty
         if(Int waiting=AtomicGet(threads._waiting))threads._queued_finished+=waiting; // notify of finished processing only if there are any waiting threads (to avoid overhead of +=), this is needed for 'Threads.wait', this is not perfect because there is a small possibility that when 2 threads are waiting, 1 of them would quickly consume both notifications
       //if(thread.wantStop())return false; not needed since we're about to return anyway
         processed++;
      }
   }

   if(!processed)threads._wake_threads.wait(); // sleep only if we haven't processed any data (this is important for 2 reasons: #1 to avoid overhead of the 'wait' function, #2 in a scenario where we have big number of calls queued, always calling 'wait' could potentially quickly consume all requested wake ups on a single thread preventing other threads from waking up), sleep until any thread was requested to wake up
   return true;
}
/******************************************************************************/
void Threads::del()
{
   AtomicSet(_left, 0); // this will stop processing priority elements so we don't have to check 'wantStop' every time
   REPAO(_threads).stop(); // notify all threads that we want to stop
  _wake_threads+=_threads.elms(); // wake up all of them in case they are sleeping
  _threads.del(); // delete threads before anything else
  _calls  .del(); _calls_pos=0;
  _func=null;
  _func_data=null;
  _func_memb=null;
  _func_memx=null;
  _func_user=null;
  _func_mode=TF_INDEX;
  _ordered=false;
  _left=_processed=_elms=0;
  _elm_size=0;
  _waiting=0;
}
void Threads::create(Bool ordered, Int threads, Int priority, C Str8 &name)
{
   del();
   T._ordered=ordered;
#if !HAS_THREADS
   Clamp(threads, 0, 1); // there's no point in creating more threads than 1 since they'd all be processed on the main thread either way
#endif
   T._threads.setNum(threads);
  
   ASSERT_BASE_EXTENDED<Thread, ThreadEx>();
   REPAO(_threads).create((Bool(*)(Thread&))ThreadsFunc, this, priority, false, name.is() ? name+i : name);
}
/******************************************************************************/
Bool Threads::wantStop     ()C {return _threads.elms() ? _threads[0].wantStop() : false;}
Int  Threads::activeThreads()C
{
   Int paused=0; REPA(_threads)paused+=_threads[i].wantPause();
   return threads()-paused;
}
Threads& Threads::activeThreads(Int active)
{
   Clamp(active, 0, threads());
   for(Int i=active; i<threads(); i++)_threads[i].pause (); // pause unwanted threads first
         REP(active                  )_threads[i].resume(); // resume  wanted threads after
   return T;
}
Int      Threads::priority(            )C {return _threads.elms() ? _threads[0].priority() : 0;}
Threads& Threads::priority(Int priority)  {REPAO( _threads).priority(priority); return T;}
/******************************************************************************/
// !! 'elm_index' MUST BE 'IntPtr' and not 'Int' because we're casting to '_func' of 'Ptr' type !!
void Threads::_process(Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread)
{
#if HAS_THREADS
   if (max_threads<0)max_threads=threads();else MIN(max_threads, threads()+allow_processing_on_this_thread);
   MIN(max_threads, elms);
   if (max_threads>1)
   {
      SyncLocker locker(_lock_process); // this allows multiple 'process' calls on multiple threads
     _func_mode=TF_INDEX; _func=(void (*)(Ptr data, Ptr user, Int thread_index))func; _func_user=user; _elms=elms; AtomicSet(_left, _elms); // set '_left' as last

      // wake up threads
     _wake_threads+=max_threads-allow_processing_on_this_thread;

      // process on this thread
      if(allow_processing_on_this_thread)
      {
         Int processed=0;
      process:
         Int index=AtomicDec(_left);
         if( index>0)
         {
            index=_elms-index; // process elements starting from zero index
            func(index, user, threads()); // set 'thread_index' to be "last_thread_index+1", have to use 'threads' because we don't know which threads (with what indexes) are going to wake up, but we have to make sure they won't overlap
            processed++;
            goto process; // proceed to next element
         }
         if(processed)AtomicAdd(_processed, processed); // set what was processed by this thread, do this in just one atomic operation (not inside the loop)
      }

      // wait until all finished
      for(; _processed<_elms; )_finished.wait();
     _elms=0; _processed=0; // other members don't need to be cleared
   }else
   {
#endif
      FREP(elms)func(i, user, 0);
#if HAS_THREADS
   }
#endif
}
void Threads::_process(Ptr data, Int elms, Int elm_size, void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread, Bool data_ptr)
{
#if HAS_THREADS
   if (max_threads<0)max_threads=threads();else MIN(max_threads, threads()+allow_processing_on_this_thread);
   MIN(max_threads, elms);
   if (max_threads>1)
   {
      SyncLocker locker(_lock_process); // this allows multiple 'process' calls on multiple threads
     _func_mode=(data_ptr ? TF_DATA_PTR : TF_DATA); _func=func; _func_data=data; _func_user=user; _elm_size=elm_size; _elms=elms; AtomicSet(_left, _elms); // set '_left' as last

      // wake up threads
     _wake_threads+=max_threads-allow_processing_on_this_thread;

      // process on this thread
      if(allow_processing_on_this_thread)
      {
         Int processed=0;
      process:
         Int index=AtomicDec(_left);
         if( index>0)
         {
            index=_elms-index; // process elements starting from zero index
            Ptr  d=(Byte*)data+index*elm_size; if(data_ptr)d=*(Ptr*)d;
            func(d, user, threads()); // set 'thread_index' to be "last_thread_index+1", have to use 'threads' because we don't know which threads (with what indexes) are going to wake up, but we have to make sure they won't overlap
            processed++;
            goto process; // proceed to next element
         }
         if(processed)AtomicAdd(_processed, processed); // set what was processed by this thread, do this in just one atomic operation (not inside the loop)
      }

      // wait until all finished
      for(; _processed<_elms; )_finished.wait();
     _elms=0; _processed=0; // other members don't need to be cleared
   }else
   {
#endif
      FREP(elms)
      {
         Ptr  d=(Byte*)data+i*elm_size; if(data_ptr)d=*(Ptr*)d;
         func(d, user, 0);
      }
#if HAS_THREADS
   }
#endif
}
void Threads::_process(_Memb &data, void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread)
{
#if HAS_THREADS
   if (max_threads<0)max_threads=threads();else MIN(max_threads, threads()+allow_processing_on_this_thread);
   MIN(max_threads, data.elms());
   if (max_threads>1)
   {
      SyncLocker locker(_lock_process); // this allows multiple 'process' calls on multiple threads
     _func_mode=TF_MEMB; _func=func; _func_memb=&data; _func_user=user; _elms=data.elms(); AtomicSet(_left, _elms); // set '_left' as last

      // wake up threads
     _wake_threads+=max_threads-allow_processing_on_this_thread;

      // process on this thread
      if(allow_processing_on_this_thread)
      {
         Int processed=0;
      process:
         Int index=AtomicDec(_left);
         if( index>0)
         {
            index=_elms-index; // process elements starting from zero index
            func(data[index], user, threads()); // set 'thread_index' to be "last_thread_index+1", have to use 'threads' because we don't know which threads (with what indexes) are going to wake up, but we have to make sure they won't overlap
            processed++;
            goto process; // proceed to next element
         }
         if(processed)AtomicAdd(_processed, processed); // set what was processed by this thread, do this in just one atomic operation (not inside the loop)
      }

      // wait until all finished
      for(; _processed<_elms; )_finished.wait();
     _elms=0; _processed=0; // other members don't need to be cleared
   }else
   {
#endif
      FREPA(data)func(data[i], user, 0);
#if HAS_THREADS
   }
#endif
}
void Threads::_process(_Memx &data, void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread)
{
#if HAS_THREADS
   if (max_threads<0)max_threads=threads();else MIN(max_threads, threads()+allow_processing_on_this_thread);
   MIN(max_threads, data.elms());
   if (max_threads>1)
   {
      SyncLocker locker(_lock_process); // this allows multiple 'process' calls on multiple threads
     _func_mode=TF_MEMX; _func=func; _func_memx=&data; _func_user=user; _elms=data.elms(); AtomicSet(_left, _elms); // set '_left' as last

      // wake up threads
     _wake_threads+=max_threads-allow_processing_on_this_thread;

      // process on this thread
      if(allow_processing_on_this_thread)
      {
         Int processed=0;
      process:
         Int index=AtomicDec(_left);
         if( index>0)
         {
            index=_elms-index; // process elements starting from zero index
            func(data[index], user, threads()); // set 'thread_index' to be "last_thread_index+1", have to use 'threads' because we don't know which threads (with what indexes) are going to wake up, but we have to make sure they won't overlap
            processed++;
            goto process; // proceed to next element
         }
         if(processed)AtomicAdd(_processed, processed); // set what was processed by this thread, do this in just one atomic operation (not inside the loop)
      }

      // wait until all finished
      for(; _processed<_elms; )_finished.wait();
     _elms=0; _processed=0; // other members don't need to be cleared
   }else
   {
#endif
      FREPA(data)func(data[i], user, 0);
#if HAS_THREADS
   }
#endif
}
/******************************************************************************/
void Threads::free() // !! assumes that '_lock_calls' is locked !!
{
   if(_calls_pos>=1024) // this will occur only for '_ordered', 1024 value was chosen so that calls are moved to the start of the container only after a decent portion has been processed, this is so that they are not moved everytime because that would cause significant slow down if there are many calls in the container
   {
     _calls.removeNum(0, _calls_pos, true); // remove all that have been processed
     _calls_pos=0;
   }
}
void Threads::checkEnd() // !! assumes that '_lock_calls' is locked !!
{
   if(_calls_pos>=_calls.elms()){_calls.clear(); _calls_pos=0;} // if none are left, then clear, this is important because "callsLeft()" can be simplified thanks to this
}
/******************************************************************************/
// !! 'elm_index' MUST BE IntPtr and not Int because we're casting to '_func' of Ptr type !!
void Threads::_queue(Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user)
{
   if(elms>0)
   {
      if(_threads.elms())
      {
         {
            SyncLocker locker(_lock_calls);
            free();
            Int start=_calls.addNum(elms); REP(elms)_calls[start+i]._set(Ptr(i), (void (*)(Ptr data, Ptr user, Int thread_index))func, user);
         }
        _wake_threads+=Min(_threads.elms(), elms);
      }else
      FREP(elms) // process in order
      {
         func(i, user, 0); // if have no threads then call on this thread
      }
   }
}
void Threads::_queue(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user)
{
   if(_threads.elms())
   {
      {
         SyncLocker locker(_lock_calls);
         free();
        _calls.New()._set(data, func, user);
      }
     _wake_threads++;
   }else
   {
      func(data, user, 0); // if have no threads then call on this thread
   }
}
Threads& Threads::queue(C MemPtr<Call> &calls)
{
   if(calls.elms())
   {
      if(_threads.elms())
      {
         {
            SyncLocker locker(_lock_calls);
            free();
            FREPA(calls)_calls.add(calls[i]);
         }
        _wake_threads+=Min(_threads.elms(), calls.elms());
      }else
      {
         FREPAO(calls).call(0); // if have no threads then call on this thread
      }
   }
   return T;
}
/******************************************************************************/
Threads& Threads::cancel()
{
   if(_calls.elms())
   {
      SyncLocker locker(_lock_calls);
     _calls.clear(); _calls_pos=0;
      if(_waiting)_queued_finished+=_waiting; // access '_waiting' only during lock !! if there's any thread waiting then notify of potential finish
   }
   return T;
}
Int Threads::_cancel(void func(Ptr data, Ptr user, Int thread_index))
{
   Int canceled=0; if(_calls.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls[i].func==func){_calls.remove(i, _ordered); canceled++;}
      if(canceled)
      {
         checkEnd();
         if(_waiting)_queued_finished+=_waiting; // access '_waiting' only during lock !! if there's any thread waiting then notify of potential finish
      }
   }
   return canceled;
}
Int Threads::_cancel(void func(Ptr data, Ptr user, Int thread_index), Ptr user)
{
   Int canceled=0; if(_calls.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls[i].isFuncUser(func, user)){_calls.remove(i, _ordered); canceled++;}
      if(canceled)
      {
         checkEnd();
         if(_waiting)_queued_finished+=_waiting; // access '_waiting' only during lock !! if there's any thread waiting then notify of potential finish
      }
   }
   return canceled;
}
Int Threads::_cancel(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user)
{
   Int canceled=0; if(_calls.elms())
   {
      Call call(data, func, user);
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls[i]==call){_calls.remove(i, _ordered); canceled++;}
      if(canceled)
      {
         checkEnd();
         if(_waiting)_queued_finished+=_waiting; // access '_waiting' only during lock !! if there's any thread waiting then notify of potential finish
      }
   }
   return canceled;
}
/******************************************************************************/
Threads& Threads::wait()
{
#if !SYNC_LOCK_SAFE
   if(_lock_calls.created())
#endif
   for(Bool not_added=true; ; )
   {
    //if(callsLeft())goto wait; // this can be outside of 'locker', but in that case it must be checked before '_threads'. It can't be outside anymore because we need to modify '_waiting' during lock
      { // braces to create scope for 'locker' !! we can modify '_waiting' only here !!
         SyncLocker locker(_lock_calls);
        _waiting+=not_added; not_added=false; // add this 'wait' call to the waiting list (if not yet added), have to do this here before checking '_threads[i].call' to make sure that thread processing function will already have this call's '_waiting' after processing the 'thread.call.call' because that one isn't covered by '_lock_calls' lock
                                 if(callsLeft())goto wait;
         REPA(_threads)if(_threads[i].call.is())goto wait;
        _waiting--; // _waiting-=added; if haven't found then remove from the waiting list (if added)
      }
      break;
   wait:
     _queued_finished.wait(); // wait when 'locker' is off !!
   }
   return T;
}
void Threads::_wait(void func(Ptr data, Ptr user, Int thread_index))
{
#if !SYNC_LOCK_SAFE
   if(_lock_calls.created())
#endif
   for(Bool not_added=true; ; )
   {
      { // braces to create scope for 'locker' !! we can modify '_waiting' only here !!
         SyncLocker locker(_lock_calls);
        _waiting+=not_added; not_added=false; // add this 'wait' call to the waiting list (if not yet added), have to do this here before checking '_threads[i].call' to make sure that thread processing function will already have this call's '_waiting' after processing the 'thread.call.call' because that one isn't covered by '_lock_calls' lock
         for(Int i=_calls.elms(); --i>=_calls_pos; )if(  _calls[i].     func==func)goto wait;
                                      REPA(_threads)if(_threads[i].call.func==func)goto wait;
        _waiting--; // _waiting-=added; if haven't found then remove from the waiting list (if added)
      }
      break;
   wait:
     _queued_finished.wait(); // wait when 'locker' is off !!
   }
}
void Threads::_wait(void func(Ptr data, Ptr user, Int thread_index), Ptr user)
{
#if !SYNC_LOCK_SAFE
   if(_lock_calls.created())
#endif
   for(Bool not_added=true; ; )
   {
      { // braces to create scope for 'locker' !! we can modify '_waiting' only here !!
         SyncLocker locker(_lock_calls);
        _waiting+=not_added; not_added=false; // add this 'wait' call to the waiting list (if not yet added), have to do this here before checking '_threads[i].call' to make sure that thread processing function will already have this call's '_waiting' after processing the 'thread.call.call' because that one isn't covered by '_lock_calls' lock
         for(Int i=_calls.elms(); --i>=_calls_pos; )if(  _calls[i].     isFuncUser(func, user))goto wait;
                                      REPA(_threads)if(_threads[i].call.isFuncUser(func, user))goto wait;
        _waiting--; // _waiting-=added; if haven't found then remove from the waiting list (if added)
      }
      break;
   wait:
     _queued_finished.wait(); // wait when 'locker' is off !!
   }
}
void Threads::_wait(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user)
{
#if !SYNC_LOCK_SAFE
   if(_lock_calls.created())
#endif
   {
      Call call(data, func, user);
      for(Bool not_added=true; ; )
      {
         { // braces to create scope for 'locker' !! we can modify '_waiting' only here !!
            SyncLocker locker(_lock_calls);
           _waiting+=not_added; not_added=false; // add this 'wait' call to the waiting list (if not yet added), have to do this here before checking '_threads[i].call' to make sure that thread processing function will already have this call's '_waiting' after processing the 'thread.call.call' because that one isn't covered by '_lock_calls' lock
            for(Int i=_calls.elms(); --i>=_calls_pos; )if(  _calls[i]     ==call)goto wait;
                                         REPA(_threads)if(_threads[i].call==call)goto wait;
           _waiting--; // _waiting-=added; if haven't found then remove from the waiting list (if added)
         }
         break;
      wait:
        _queued_finished.wait(); // wait when 'locker' is off !!
      }
   }
}
/******************************************************************************/
Int Threads::queued()C
{
   if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      Int    queued=_calls.elms()-_calls_pos; REPA(_threads)queued+=_threads[i].call.is();
      return queued;
   }
   return 0;
}
Int Threads::_queued(void func(Ptr data, Ptr user, Int thread_index))C
{
   Int queued=0; if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i].     func==func)queued++;
                                   REPA(_threads)if(_threads[i].call.func==func)queued++;
   }
   return queued;
}
Int Threads::_queued(void func(Ptr data, Ptr user, Int thread_index), Ptr user)C
{
   Int queued=0; if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i].     isFuncUser(func, user))queued++;
                                   REPA(_threads)if(_threads[i].call.isFuncUser(func, user))queued++;
   }
   return queued;
}
Int Threads::_queued(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user)C
{
   Int queued=0; if(_threads.elms())
   {
      Call call(data, func, user);
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i]     ==call)queued++;
                                   REPA(_threads)if(_threads[i].call==call)queued++;
   }
   return queued;
}
/******************************************************************************/
Bool Threads::busy()C
{
   if(_calls  .elms())return true;
   if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      REPA(_threads)if(_threads[i].call.is())return true;
   }
   return false;
}
Bool Threads::_busy(void func(Ptr data, Ptr user, Int thread_index))C
{
   if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i].     func==func)return true;
                                   REPA(_threads)if(_threads[i].call.func==func)return true;
   }
   return false;
}
Bool Threads::_busy(void func(Ptr data, Ptr user, Int thread_index), Ptr user)C
{
   if(_threads.elms())
   {
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i].     isFuncUser(func, user))return true;
                                   REPA(_threads)if(_threads[i].call.isFuncUser(func, user))return true;
   }
   return false;
}
Bool Threads::_busy(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user)C
{
   if(_threads.elms())
   {
      Call call(data, func, user);
      SyncLocker locker(_lock_calls);
      for(Int i=_calls.elms(); --i>=_calls_pos; )if(_calls  [i]     ==call)return true;
                                   REPA(_threads)if(_threads[i].call==call)return true;
   }
   return false;
}
/******************************************************************************/
// MULTI-THREADED FUNCTION CALLER
/******************************************************************************/
struct MTFC
{
   Int                 index, // index  of element  to process next
                       elms ; // number of elements to process
   void              (*func)(Int elm_index, Ptr user, Int thread_index);
   Ptr                 user;
   MemtN<Thread, 16-1> threads; // one less because we will make calls on the caller thread as well

   static Bool Func(Thread &thread)
   {
      MTFC &mtfc=*(MTFC*)thread.user;
      for(Int thread_index=mtfc.threads.index(&thread); ; )
      {
         Int i=AtomicInc(mtfc.index);
         if(InRange(i, mtfc.elms))mtfc.func(i, mtfc.user, thread_index);else break;
      }
      return false;
   }
   MTFC(Int elms, void func(Int elm_index, Ptr user, Int thread_index), Ptr user, Int threads)
   {
      T.index=0;
      T.elms =elms;
      T.func =func;
      T.user =user;

      T.threads.setNum(Min(elms, threads)-1); // allocate all threads at start, because they need to be in constant memory, allocate one less, because we will make calls on this thread as well
      REPAO(T.threads).create(Func, this);

      // make calls on this thread
      for(Int thread_index=T.threads.elms(); ; )
      {
         Int i=AtomicInc(index);
         if(InRange(i, elms))func(i, user, thread_index);else break;
      }

      // wait for other threads
      REPAO(T.threads).wait();
   }
};
void MultiThreadedCall(Int elms, void func(Int elm_index, Ptr user, Int thread_index), Ptr user, Int threads)
{
#if HAS_THREADS
   if(elms>1 && threads>1){MTFC(elms, func,   user, threads);}else
#endif
                           FREP(elms) func(i, user, 0      );
}
/******************************************************************************/
struct MTFC_Ptr // MTFC for Ptr
{
   Byte  *data;
   Int    elm_size;
   void (*func)(Ptr data, Ptr user, Int thread_index);
   Ptr    user;

   MTFC_Ptr(Ptr data, Int elm_size, void func(Ptr data, Ptr user, Int thread_index), Ptr user) {T.data=(Byte*)data; T.elm_size=elm_size; T.func=func; T.user=user;}

   static void Func   (Int elm_index, MTFC_Ptr &mtfc, Int thread_index) {mtfc.func(        mtfc.data+elm_index*mtfc.elm_size , mtfc.user, thread_index);}
   static void FuncPtr(Int elm_index, MTFC_Ptr &mtfc, Int thread_index) {mtfc.func(*(Ptr*)(mtfc.data+elm_index*mtfc.elm_size), mtfc.user, thread_index);}
};
struct MTFC_Memb // MTFC for Memb
{
  _Memb  &data;
   void (*func)(Ptr data, Ptr user, Int thread_index);
   Ptr    user;

   MTFC_Memb(_Memb &data, void func(Ptr data, Ptr user, Int thread_index), Ptr user) : data(data) {T.func=func; T.user=user;}

   static void Func   (Int elm_index, MTFC_Memb &mtfc, Int thread_index) {mtfc.func(        mtfc.data[elm_index] , mtfc.user, thread_index);}
   static void FuncPtr(Int elm_index, MTFC_Memb &mtfc, Int thread_index) {mtfc.func(*(Ptr*)(mtfc.data[elm_index]), mtfc.user, thread_index);}
};
struct MTFC_Memx // MTFC for Memx
{
  _Memx  &data;
   void (*func)(Ptr data, Ptr user, Int thread_index);
   Ptr    user;

   MTFC_Memx(_Memx &data, void func(Ptr data, Ptr user, Int thread_index), Ptr user) : data(data) {T.func=func; T.user=user;}

   static void Func   (Int elm_index, MTFC_Memx &mtfc, Int thread_index) {mtfc.func(        mtfc.data[elm_index] , mtfc.user, thread_index);}
   static void FuncPtr(Int elm_index, MTFC_Memx &mtfc, Int thread_index) {mtfc.func(*(Ptr*)(mtfc.data[elm_index]), mtfc.user, thread_index);}
};
void _MultiThreadedCall( Ptr   data, Int elms, Int elm_size, void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr) {MTFC_Ptr  mtfc(data, elm_size, func, user); MultiThreadedCall(     elms  , data_ptr ? *MTFC_Ptr ::FuncPtr : *MTFC_Ptr ::Func, mtfc, threads);}
void _MultiThreadedCall(_Memb &data,                         void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr) {MTFC_Memb mtfc(data,           func, user); MultiThreadedCall(data.elms(), data_ptr ? *MTFC_Memb::FuncPtr : *MTFC_Memb::Func, mtfc, threads);}
void _MultiThreadedCall(_Memx &data,                         void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr) {MTFC_Memx mtfc(data,           func, user); MultiThreadedCall(data.elms(), data_ptr ? *MTFC_Memx::FuncPtr : *MTFC_Memx::Func, mtfc, threads);}
/******************************************************************************/
// THREAD EMULATION
/******************************************************************************/
ThreadEmulation::ThreadEmulation() {_process_left=0; _process_type=_time=0;}

void ThreadEmulation::include(Thread &thread)
{
   if(thread.sleep())
   {
      REPA(_delayed_threads)if(_delayed_threads[i].thread==&thread)return;
     _delayed_threads.New().thread=&thread;
   }else
   {
     _rt_threads.include(&thread);
   }
}
void ThreadEmulation::exclude(Thread &thread)
{
   if(thread.sleep())
   {
      REPA(_delayed_threads)if(_delayed_threads[i].thread==&thread)
      {
         if(_process_type==1 && i<_process_left)_process_left--; // if this element was going to be processed soon
        _delayed_threads.remove(i, true);
         break;
      }
   }else
   {
      REPA(_rt_threads)if(_rt_threads[i]==&thread)
      {
         if(_process_type==0 && i<_process_left)_process_left--; // if this element was going to be processed soon
        _rt_threads.remove(i, true);
         break;
      }
   }
}

void ThreadEmulation::update()
{
   // real-time threads
   for(_process_type=0, _process_left=_rt_threads.elms(); _process_left--; )
   {
      Thread &thread=*_rt_threads[_process_left]; // inside the function below, '_process_left' points to the element being processed right now
      if(!thread._func(thread))
      {
         thread._active=false;
         exclude(thread);
      }
   }

   // delayed threads
   UInt time=Time.curTimeMs(), delta=time-_time; // this code was tested OK for UInt overflow
   if(delta>0)
   {
     _time=time;

      for(_process_type=1, _process_left=_delayed_threads.elms(); _process_left--; )
      {
         DelayedThread &del_thread=_delayed_threads[_process_left]; // inside the function below, '_process_left' points to the element being processed right now
         del_thread.waited+=delta;
         Int sleep=del_thread.thread->sleep();
         if(del_thread.waited>=sleep)
         {
            del_thread.waited-=sleep;
            Thread &thread=*del_thread.thread; // keep a standalone reference in case 'del_thread' gets removed in the function
            if(!thread._func(thread))
            {
               thread._active=false;
               exclude(thread);
            }
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
