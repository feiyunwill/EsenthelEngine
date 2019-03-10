/******************************************************************************

   Use 'SyncLock'      for multi-threaded synchronization
   Use 'SyncLocker'    for simplified usage of the 'SyncLock'
   Use 'SyncEvent'     for multi-threaded synchronization
   Use 'ReadWriteSync' to  handle synchronization between multiple "reader" and "writer" threads, where multiple "readers" can access a resource at once, while only 1 "writer" (and no other "readers" or "writers") can access a resource at once
   Use 'Thread'        to  run functions simultaneously on the Cpu

   Use 'MultiThreadedCall' to process data in multi-threaded mode by splitting the work into multiple threads

   Use 'Proc' functions for OS processes management.

/******************************************************************************/
#if EE_PRIVATE
   #define SYNC_LOCK_SAFE 0 // if 'SyncLock' methods should check if it was initialized first, 0=faster and less safe, 1=slower and more safe
#endif
struct SyncLock // Synchronization Lock (multi-threaded safe)
{
   Bool created()C; // if           lock is still created and not yet deleted in the constructor
   Bool owned  ()C; // if           lock is owned by current thread
   Bool tryOn  ()C; // try entering lock, false on fail (after receiving 'true' you must call the 'off' method, while after receiving 'false' you may not call the 'off' method)
   void on     ()C; //     enter    lock
   void off    ()C; //     leave    lock

  ~SyncLock();
   SyncLock();

private:
#if EE_PRIVATE
   union
   {
   #if WINDOWS
      struct // total 40 bytes
      {
         mutable CRITICAL_SECTION _lock; // SIZE(_lock)==40 on x64
      };
      ULong _b[5]; // 5 ULong = 40 bytes = max possible size of union
   #else
      struct // total 80 bytes
      {
         mutable pthread_mutex_t _lock      ; // SIZE(_lock)==64 on x64
         mutable Int             _lock_count; // SIZE(     )== 4
         mutable UIntPtr         _owner     ; // SIZE(     )== 8 on x64
         mutable UInt            _is        ; // SIZE(_is  )== 4
      };
      ULong _b[10]; // 10 ULong = 80 bytes = max possible size of union
   #endif
   };
   // !! use ULong above and below to force ULong alignment, if Byte[] is set, it could cause crash !! "struct A {Bool b; SyncLock lock;}" would have different alignments
#else
   ULong _lock[PLATFORM(5, 10)];
#endif
   NO_COPY_CONSTRUCTOR(SyncLock);
};
#if EE_PRIVATE
   ASSERT(SIZE(SyncLock)==SIZE(ULong)*PLATFORM(5, 10));
#endif

struct SyncLocker // Synchronization Locker (automatically locks and unlocks the Synchronization Lock at object creation and destruction)
{
   explicit SyncLocker(C SyncLock &lock) : _lock(lock) {_lock.on ();}
           ~SyncLocker(                )               {_lock.off();}

private:
 C SyncLock &_lock;
   NO_COPY_CONSTRUCTOR(SyncLocker);
};

struct SyncLockerEx // Synchronization Locker Extended
{
   void on () {if(!_on){_on=true ; _lock.on ();}} // manually   lock
   void off() {if( _on){_on=false; _lock.off();}} // manually unlock

   void set(Bool on) {if(on!=_on){if(_on=on)_lock.on();else _lock.off();}} // set if lock should be enabled

   Bool tryOn() {if(!_on && _lock.tryOn())_on=true; return _on;} // try entering locking, false on fail

   explicit SyncLockerEx(C SyncLock &lock, Bool on=true) : _lock(lock) {if(_on=on)lock.on();}
           ~SyncLockerEx(                              )               {off();}

private:
   Bool      _on;
 C SyncLock &_lock;
   NO_COPY_CONSTRUCTOR(SyncLockerEx);
};
#if EE_PRIVATE

#if SYNC_LOCK_SAFE
   typedef SyncLocker   SafeSyncLocker;
   typedef SyncLockerEx SafeSyncLockerEx;
#else
   struct SafeSyncLocker
   {
      explicit SafeSyncLocker(C SyncLock &lock) {if(lock.created()){_lock=&lock; lock. on ();}else _lock=null;}
              ~SafeSyncLocker(                ) {                      if(_lock)_lock->off();}

   private:
    C SyncLock *_lock;
      NO_COPY_CONSTRUCTOR(SafeSyncLocker);
   };
   struct SafeSyncLockerEx
   {
      void on () {if(!_on && _lock){_on=true ; _lock->on ();}} // manually   lock
      void off() {if( _on && _lock){_on=false; _lock->off();}} // manually unlock

      void set(Bool on) {if(on!=_on && _lock){if(_on=on)_lock->on();else _lock->off();}} // set if lock should be enabled

      Bool tryOn() {if(!_on && _lock && _lock->tryOn())_on=true; return _on;} // try entering locking, false on fail

      explicit SafeSyncLockerEx(C SyncLock &lock, Bool on=true) {if(lock.created()){_lock=&lock; if(_on=on)lock.on();}else{_lock=null; _on=false;}}
              ~SafeSyncLockerEx(                              ) {off();}

   private:
      Bool      _on;
    C SyncLock *_lock;
      NO_COPY_CONSTRUCTOR(SafeSyncLockerEx);
   };
#endif

#define SYNC_UNLOCK_SINGLE 0 // can't use single because locks can be locked multiple times, and we need to unlock all

#if SYNC_UNLOCK_SINGLE // single
struct SyncUnlocker
{
   explicit SyncUnlocker(C SyncLock &lock) : _lock(lock) {if(_owned=lock.owned())_lock.off();}
           ~SyncUnlocker(                )               {if(_owned             )_lock.on ();}

private:
   Bool      _owned;
 C SyncLock &_lock;
   NO_COPY_CONSTRUCTOR(SyncUnlocker);
};
#else // multi
struct SyncUnlocker
{
   explicit SyncUnlocker(C SyncLock &lock) : _lock(lock) {for(_owned=0; lock.owned(); _owned++)_lock.off();}
           ~SyncUnlocker(                )               {REP(_owned                          )_lock.on ();}

private:
   Int       _owned;
 C SyncLock &_lock;
   NO_COPY_CONSTRUCTOR(SyncUnlocker);
};
#endif
#endif
/******************************************************************************/
struct SyncEvent // Synchronization Event (multi-threaded safe)
{
   void on  (                )C; //   activate
   void off (                )C; // deactivate
   Bool wait(                )C; // wait                until activated                     , false on timeout
   Bool wait(Int milliseconds)C; // wait 'milliseconds' until activated (<0 = infinite wait), false on timeout

           ~SyncEvent();
   explicit SyncEvent(Bool auto_off=true); // 'auto_off'=if automatically call 'off' upon a successful 'wait'

#if EE_PRIVATE
   Bool is()C {return _handle!=null;} // if created
   #if WINDOWS
      HANDLE handle()C {return _handle;}
   #endif
#endif

private:
#if EE_PRIVATE
   PLATFORM(HANDLE, pthread_cond_t*) _handle;
#else
   Ptr _handle;
#endif
#if !WINDOWS
#if EE_PRIVATE
   PLATFORM(Ptr, pthread_mutex_t*) _mutex;
#else
   Ptr _mutex;
#endif
   mutable Bool _condition, _auto_off;
#endif
   NO_COPY_CONSTRUCTOR(SyncEvent);
};
/******************************************************************************/
struct SyncCounter // Synchronization Counter (multi-threaded safe)
{
   void operator+=(Int count)C;        // activate
   void operator++(int      )C {T+=1;} // activate 1

   Bool wait(                )C; // wait                until activated                     , false on timeout
   Bool wait(Int milliseconds)C; // wait 'milliseconds' until activated (<0 = infinite wait), false on timeout

  ~SyncCounter();
   SyncCounter();

#if EE_PRIVATE
   Bool is()C {return _handle!=null;} // if created
   #if WINDOWS
      HANDLE handle()C {return _handle;}
   #endif
#endif

private:
#if EE_PRIVATE
   PLATFORM(HANDLE, pthread_cond_t*) _handle;
#else
   Ptr _handle;
#endif
#if !WINDOWS
#if EE_PRIVATE
   PLATFORM(Ptr, pthread_mutex_t*) _mutex;
#else
   Ptr _mutex;
#endif
   mutable Int _counter;
#endif
   NO_COPY_CONSTRUCTOR(SyncCounter);
};
/******************************************************************************/
struct ReadWriteSync // allows multiple "readers/writers" synchronization (multi-threaded safe)
{
   void enterRead(); // enter reading (shared mode)
   void leaveRead(); // leave reading (shared mode)

   void enterWrite(); // enter writing (exclusive mode)
   void leaveWrite(); // leave writing (exclusive mode)

   Bool ownedRead ();                               // if read  lock is owned by current thread
   Bool ownedWrite()C {return _write_lock.owned();} // if write lock is owned by current thread

   Bool created()C {return _locks_lock.created();} // if lock is still created and not yet deleted in the constructor

private:
   struct Lock
   {
      UIntPtr thread_id;
      Int     locks;
   };
   SyncLock   _locks_lock, _write_lock;
   SyncEvent  _left_reading;
   Memc<Lock> _locks;
};

struct ReadLock // 'ReadWriteSync' Read Lock (automatically locks and unlocks for reading the 'ReadWriteSync' at object creation and destruction)
{
   explicit ReadLock(ReadWriteSync &lock) : _lock(lock) {_lock.enterRead();}
           ~ReadLock(                   )               {_lock.leaveRead();}

private:
   ReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(ReadLock);
};

struct WriteLock // 'ReadWriteSync' Write Lock (automatically locks and unlocks for writing the 'ReadWriteSync' at object creation and destruction)
{
   explicit WriteLock(ReadWriteSync &lock) : _lock(lock) {_lock.enterWrite();}
           ~WriteLock(                   )               {_lock.leaveWrite();}

private:
   ReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(WriteLock);
};

struct WriteLockEx // 'ReadWriteSync' Write Lock Extended
{
   void on () {if(!_on){_on=true ; _lock.enterWrite();}} // manually   lock
   void off() {if( _on){_on=false; _lock.leaveWrite();}} // manually unlock

   void set(Bool on) {if(on!=_on){if(_on=on)_lock.enterWrite();else _lock.leaveWrite();}} // set if lock should be enabled

   explicit WriteLockEx(ReadWriteSync &lock, Bool on=true) : _lock(lock) {if(_on=on)lock.enterWrite();}
           ~WriteLockEx(                                 )               {off();}

private:
   Bool           _on;
   ReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(WriteLockEx);
};

#if EE_PRIVATE
   #if SYNC_LOCK_SAFE
      typedef WriteLock SafeWriteLock;
   #else
      struct SafeWriteLock
      {
         explicit SafeWriteLock(ReadWriteSync &lock) {if(lock.created()){_lock=&lock; lock. enterWrite();}else _lock=null;}
                 ~SafeWriteLock(                   ) {                      if(_lock)_lock->leaveWrite();}

      private:
         ReadWriteSync *_lock;
         NO_COPY_CONSTRUCTOR(SafeWriteLock);
      };
   #endif
#endif
/******************************************************************************/
struct SimpleReadWriteSync // !! NOT REENTRANT - does not support 'enterWrite' if already 'enterRead' was called on the same thread - deadlock will occur !!
{
   void enterRead()C; // enter reading (shared mode)
   void leaveRead()C; // leave reading (shared mode)

   void enterWrite()C; // enter writing (exclusive mode)
   void leaveWrite()C; // leave writing (exclusive mode)

private:
           SyncLock  _lock;
           SyncEvent _finished;
   mutable Int       _readers;
};

struct SimpleReadLock // SimpleReadWriteSync Read Lock (automatically locks and unlocks for reading the 'SimpleReadWriteSync' at object creation and destruction)
{
   explicit SimpleReadLock(C SimpleReadWriteSync &lock) : _lock(lock) {_lock.enterRead();}
           ~SimpleReadLock(                           )               {_lock.leaveRead();}

private:
 C SimpleReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(SimpleReadLock);
};

struct SimpleWriteLock // SimpleReadWriteSync Write Lock (automatically locks and unlocks for writing the 'SimpleReadWriteSync' at object creation and destruction)
{
   explicit SimpleWriteLock(C SimpleReadWriteSync &lock) : _lock(lock) {_lock.enterWrite();}
           ~SimpleWriteLock(                           )               {_lock.leaveWrite();}

private:
 C SimpleReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(SimpleWriteLock);
};

struct SimpleReadLockEx // SimpleReadWriteSync Read Lock Extended
{
   void on () {if(!_on){_on=true ; _lock.enterRead();}} // manually   lock
   void off() {if( _on){_on=false; _lock.leaveRead();}} // manually unlock

   void set(Bool on) {if(on!=_on){if(_on=on)_lock.enterRead();else _lock.leaveRead();}} // set if lock should be enabled

   explicit SimpleReadLockEx(C SimpleReadWriteSync &lock, Bool on=true) : _lock(lock) {if(_on=on)lock.enterRead();}
           ~SimpleReadLockEx(                                         )               {off();}

private:
   Bool                 _on;
 C SimpleReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(SimpleReadLockEx);
};

struct SimpleWriteLockEx // SimpleReadWriteSync Write Lock Extended
{
   void on () {if(!_on){_on=true ; _lock.enterWrite();}} // manually   lock
   void off() {if( _on){_on=false; _lock.leaveWrite();}} // manually unlock

   void set(Bool on) {if(on!=_on){if(_on=on)_lock.enterWrite();else _lock.leaveWrite();}} // set if lock should be enabled

   explicit SimpleWriteLockEx(C SimpleReadWriteSync &lock, Bool on=true) : _lock(lock) {if(_on=on)lock.enterWrite();}
           ~SimpleWriteLockEx(                                         )               {off();}

private:
   Bool                 _on;
 C SimpleReadWriteSync &_lock;
   NO_COPY_CONSTRUCTOR(SimpleWriteLockEx);
};
/******************************************************************************/
const_mem_addr struct Thread // Thread !! must be stored in constant memory address !!
{
   Ptr user; // user data

   // get
   Bool created  ()C {return _handle!=NULL;} // if the thread is created             (this will be false only if the thread hasn't yet been created, or if it has been manually deleted, if the thread was created but stopped running, the return value will be still true)
   Bool active   ()C {return _active      ;} // if the thread is created and running (this will be false      if the thread hasn't yet been created, or if it has been manually deleted,                         or it stopped running                                     )
   Bool wantStop ()C {return _want_stop   ;} // if the thread is requested to be stopped by the 'stop'  method
   Bool wantPause()C {return _want_pause  ;} // if the thread is requested to be paused  by the 'pause' method
   Bool paused   ()C {return _paused      ;} // if the thread is currently       paused
   Int  priority ()C {return _priority    ;} // get    thread priority (-3..3)
   UIntPtr     id()C;                        // get    thread ID, 0 on fail

   // manage
   Bool create    (Bool func(Thread &thread), Ptr user=null, Int priority=0, Bool paused=false, C Str8 &name=S8); // create, 'func'=function which will be called in the created thread, 'user'=custom user data, 'priority'=thread priority (-3..3), 'paused'=if start the thread in paused mode, 'name'=thread name, threads have a default stack size of 1MB, if the thread will require performing operations on GPU data, then you must call 'ThreadMayUseGPUData' inside the thread, please check that function comments for more info
   void del       (Int milliseconds=-1); // delete thread, if it's active then wait 'milliseconds' until finishes (<0 = infinite wait), if after the waiting time it's still active then the thread will be killed
   void       stop(                   ); // stop   thread, notifies that the thread should stop calling 'func' and exit, this method doesn't however wait until the thread exits
   void cancelStop(                   ); // cancel thread stopping, this method has no effect if the thread has already been stopped
   void pause     (                   ); // pause  thread, 'func' will no longer be called until the thread is resumed
   void resume    (                   ); // resume thread from paused state
   void priority  (Int priority       ); // set    thread priority, 'priority'=-3..3
   void kill      (                   ); // kill   thread, immediately shut down the thread, usage of this method is not recommended because it may cause memory leaks
   Bool wait      (Int milliseconds=-1); // wait   until the thread finishes processing (<0 = infinite wait), false on timeout

#if EE_PRIVATE
   void zero ();
   void func ();
   Int  sleep()C {return 0;} // get thread sleeping (0..Inf), amount of time (in milliseconds) to sleep between function calls
#endif

           ~Thread() {del();}
            Thread();
   explicit Thread(Bool func(Thread &thread), Ptr user=null, Int priority=0, Bool paused=false); // create, 'func'=function which will be called in the created thread, 'user'=custom user data, 'priority'=thread priority (-3..3), 'paused'=if start the thread in paused mode, threads have a default stack size of 1MB, if the thread will require performing operations on GPU data, then you must call 'ThreadMayUseGPUData' inside the thread, please check that function comments for more info

#if !EE_PRIVATE
private:
#endif
   Bool      _want_stop, _want_pause, _paused, _active;
   SByte     _priority;
#if EE_PRIVATE
   PLATFORM(HANDLE, pthread_t) _handle;
#else
   Ptr       _handle;
#endif
   Bool    (*_func)(Thread &thread);
   SyncEvent _resume;
#if !WINDOWS
   SyncEvent _finished;
#endif
#if APPLE
   Str8      _name;
#endif

   NO_COPY_CONSTRUCTOR(Thread);
};
/******************************************************************************/
const_mem_addr struct Threads // Worker Threads, allow to process data on multiple threads !! must be stored in constant memory address !!
{
   struct Call
   {
      Ptr    data, user;
      void (*func)(Ptr data, Ptr user, Int thread_index);

      T1(DATA           )   void set(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {_set((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
      T2(DATA, USER_DATA)   void set(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_set((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
      T2(DATA, USER_DATA)   void set(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_set((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

      void _set(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user=null) {T.data=data; T.user=user; T.func=func;}
      void clear() {_set(null, null);}
   #if EE_PRIVATE
      void call (Int thread_index)C {func(data, user, thread_index);}

      Bool is        (         )C {return func!=null;}
      Bool operator==(C Call &c)C {return data==c.data && user==c.user && func==c.func;}
      Bool isFuncUser(void func(Ptr data, Ptr user, Int thread_index), Ptr user)C {return T.func==func && T.user==user;}

      Call() {}
      Call(Ptr data, void func(Ptr data, Ptr user, Int thread_index), Ptr user=null) {_set(data, func, user);}
   #endif
   };

   void del   (                                                                        ); // delete the threads without finishing all queued work
   void create(Bool ordered, Int threads=Cpu.threads(), Int priority=0, C Str8 &name=S8); // 'ordered'=if process queued calls in the order as they were given (this will be a bit slower), 'priority'=threads priority (-3..3), 'name'=Threads name

   // perform multi-threaded call on 'func' function, by giving a unique 'elm_index' from the "0 .. elms-1" range as the function parameter, this function will return only after all calls have been processed, 'max_threads'=max threads to wake up and perform processing (threads that are already awake may also do processing) -1=default value which is 'threads', 'thread_index' will always be 0..'threads'-1 (or 0 if there are no threads), consider using 'process1' for better performance
                   void process(Int elms, void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user, max_threads, false);}
   T1(USER_DATA)   void process(Int elms, void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user, max_threads, false);}
   T1(USER_DATA)   void process(Int elms, void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func, &user, max_threads, false);}

   // perform multi-threaded call on 'func' function, by giving a unique 'data' element from the memory container as the function parameter, this function will return only after all calls have been processed, 'max_threads'=max threads to wake up and perform processing (threads that are already awake may also do processing) -1=default value which is 'threads', 'thread_index' will always be 0..'threads'-1 (or 0 if there are no threads), consider using 'process1' for better performance
   T1(DATA           )   void process(Mems<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T1(DATA           )   void process(Mems<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T1(DATA           )   void process(Memc<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T1(DATA           )   void process(Memc<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T1(DATA           )   void process(Memt<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T1(DATA           )   void process(Memt<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T1(DATA           )   void process(Memb<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false);}
   T1(DATA           )   void process(Memx<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false);}

   T2(DATA, USER_DATA)   void process(Mems<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Mems<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memc<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Memc<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memt<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Memt<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memb<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false);}
   T2(DATA, USER_DATA)   void process(Memx<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, false);}

   T2(DATA, USER_DATA)   void process(Mems<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Mems<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memc<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Memc<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memt<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, false);}
   T2(DATA, USER_DATA)   void process(Memt<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false, true );}
   T2(DATA, USER_DATA)   void process(Memb<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false);}
   T2(DATA, USER_DATA)   void process(Memx<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, false);}

   // perform multi-threaded call on 'func' function, by giving a unique 'elm_index' from the "0 .. elms-1" range as the function parameter, this function will return only after all calls have been processed, 'max_threads'=max threads to wake up and perform processing (threads that are already awake may also do processing) -1=default value which is 'threads', 'thread_index' will always be 0..'threads' (inclusive), which means if you're using per-thread data, then make sure to allocate 'threads'+1 per-thread data (use 'threads1' method to get the number of elements), this method is preferred instead of 'process' offering better performance, however caution must be taken to allocate 1 extra per-thread data
                   void process1(Int elms, void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user, max_threads, true);}
   T1(USER_DATA)   void process1(Int elms, void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user, max_threads, true);}
   T1(USER_DATA)   void process1(Int elms, void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func, &user, max_threads, true);}

   // perform multi-threaded call on 'func' function, by giving a unique 'data' element from the memory container as the function parameter, this function will return only after all calls have been processed, 'max_threads'=max threads to wake up and perform processing (threads that are already awake may also do processing) -1=default value which is 'threads', 'thread_index' will always be 0..'threads' (inclusive), which means if you're using per-thread data, then make sure to allocate 'threads'+1 per-thread data (use 'threads1' method to get the number of elements), this method is preferred instead of 'process' offering better performance, however caution must be taken to allocate 1 extra per-thread data
   T1(DATA           )   void process1(Mems<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T1(DATA           )   void process1(Mems<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T1(DATA           )   void process1(Memc<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T1(DATA           )   void process1(Memc<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T1(DATA           )   void process1(Memt<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T1(DATA           )   void process1(Memt<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T1(DATA           )   void process1(Memb<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true);}
   T1(DATA           )   void process1(Memx<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true);}

   T2(DATA, USER_DATA)   void process1(Mems<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Mems<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memc<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Memc<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memt<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Memt<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memb<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true);}
   T2(DATA, USER_DATA)   void process1(Memx<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, max_threads, true);}

   T2(DATA, USER_DATA)   void process1(Mems<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Mems<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memc<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Memc<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memt<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, false);}
   T2(DATA, USER_DATA)   void process1(Memt<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true, true );}
   T2(DATA, USER_DATA)   void process1(Memb<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true);}
   T2(DATA, USER_DATA)   void process1(Memx<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int max_threads=-1) {_process(data       ,                              (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, max_threads, true);}

   // queue calls on 'func' function, by giving a unique 'elm_index' from the "0 .. elms-1" range as the function parameter, this function will queue the calls for processing and return immediately without waiting for them to be processed
                   void queue(Int elms, void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null) {_queue(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   void queue(Int elms, void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_queue(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   void queue(Int elms, void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_queue(elms, (void (*)(IntPtr elm_index, Ptr user, Int thread_index))func, &user);}

   // queue call on 'func' function with 'data' and 'user' parameters to be performed on one of the worker threads, this function will queue the object for processing and return immediately without waiting for it to be processed
   T1(DATA           )   void queue(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {_queue((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void queue(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_queue((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void queue(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_queue((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   Threads& queue(C MemPtr<Call> &calls); // queue multiple calls

   // cancel queued calls on 'func' function with 'data' and 'user' parameters, returns the number of canceled calls, please note that call that's already in progress can't be canceled
                   Int cancelFuncUser(void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Int cancelFuncUser(void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Int cancelFuncUser(void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     ) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

                   Int cancelFunc(void func(IntPtr elm_index, Ptr        user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Int cancelFunc(void func(IntPtr elm_index, USER_DATA *user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Int cancelFunc(void func(IntPtr elm_index, USER_DATA &user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   T1(DATA           )   Int cancel(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {return _cancel((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int cancel(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {return _cancel((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int cancel(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {return _cancel((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Int cancelFuncUser(void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int cancelFuncUser(void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int cancelFuncUser(void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Int cancelFunc(void func(DATA &data, Ptr        user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Int cancelFunc(void func(DATA &data, USER_DATA *user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Int cancelFunc(void func(DATA &data, USER_DATA &user, Int thread_index)) {return _cancel((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   Threads& cancel(); // cancel all queued calls

   // wait until queued calls on 'func' function with 'data' and 'user' parameters have finished processing
                   void waitFuncUser(void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   void waitFuncUser(void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   void waitFuncUser(void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

                   void waitFunc(void func(IntPtr elm_index, Ptr        user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   void waitFunc(void func(IntPtr elm_index, USER_DATA *user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   void waitFunc(void func(IntPtr elm_index, USER_DATA &user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   T1(DATA           )   void wait(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {_wait((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void wait(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_wait((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void wait(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_wait((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   void waitFuncUser(void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void waitFuncUser(void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   void waitFuncUser(void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     ) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   void waitFunc(void func(DATA &data, Ptr        user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   void waitFunc(void func(DATA &data, USER_DATA *user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   void waitFunc(void func(DATA &data, USER_DATA &user, Int thread_index)) {_wait((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   Threads& wait(); // wait until all queued calls have finished processing

   // get number of queued calls on 'func' function with 'data' and 'user' parameters still waiting in the queue for processing (this includes calls being currently processed)
                   Int queuedFuncUser(void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Int queuedFuncUser(void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Int queuedFuncUser(void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     ) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

                   Int queuedFunc(void func(IntPtr elm_index, Ptr        user, Int thread_index)) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Int queuedFunc(void func(IntPtr elm_index, USER_DATA *user, Int thread_index)) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Int queuedFunc(void func(IntPtr elm_index, USER_DATA &user, Int thread_index)) {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   T1(DATA           )   Int queued(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null)C {return _queued((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int queued(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null)C {return _queued((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int queued(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     )C {return _queued((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Int queuedFuncUser(void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null)C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int queuedFuncUser(void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null)C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Int queuedFuncUser(void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     )C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Int queuedFunc(void func(DATA &data, Ptr        user, Int thread_index))C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Int queuedFunc(void func(DATA &data, USER_DATA *user, Int thread_index))C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Int queuedFunc(void func(DATA &data, USER_DATA &user, Int thread_index))C {return _queued((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   Int queued()C; // get number of all calls still waiting in the queue for processing (this includes calls being currently processed)

   // check if there are any queued calls on 'func' function with 'data' and 'user' parameters still waiting in the queue for processing (this includes calls being currently processed)
                   Bool busyFuncUser(void func(IntPtr elm_index, Ptr        user, Int thread_index), Ptr        user=null) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Bool busyFuncUser(void func(IntPtr elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T1(USER_DATA)   Bool busyFuncUser(void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     ) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

                   Bool busyFunc(void func(IntPtr elm_index, Ptr        user, Int thread_index)) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Bool busyFunc(void func(IntPtr elm_index, USER_DATA *user, Int thread_index)) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T1(USER_DATA)   Bool busyFunc(void func(IntPtr elm_index, USER_DATA &user, Int thread_index)) {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   T1(DATA           )   Bool busy(DATA &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null)C {return _busy((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Bool busy(DATA &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null)C {return _busy((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Bool busy(DATA &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     )C {return _busy((Ptr)&data, (void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Bool busyFuncUser(void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null)C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Bool busyFuncUser(void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null)C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func,  user);}
   T2(DATA, USER_DATA)   Bool busyFuncUser(void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     )C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func, &user);}

   T1(DATA           )   Bool busyFunc(void func(DATA &data, Ptr        user, Int thread_index))C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Bool busyFunc(void func(DATA &data, USER_DATA *user, Int thread_index))C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}
   T2(DATA, USER_DATA)   Bool busyFunc(void func(DATA &data, USER_DATA &user, Int thread_index))C {return _busy((void (*)(Ptr data, Ptr user, Int thread_index))func);}

   Bool busy()C; // if there's something being processed right now

   Bool     wantStop()C;                                         // get     if threads were requested to be stopped by the 'del' method
   Int      threads ()C {return _threads.elms()  ;}              // get     how many threads were created for this object
   Int      threads1()C {return _threads.elms()+1;}              // get     how many threads were created for this object + 1, use this method when allocating per-thread data to be used for 'process1' methods
   Int activeThreads()C;   Threads& activeThreads(Int active  ); // get/set how many threads should be active (remaining threads will be paused)
   Int      priority()C;   Threads& priority     (Int priority); // get/set threads priority, 'priority'=-3..3

  ~Threads() {del();}

#if !EE_PRIVATE
private:
#endif
   struct ThreadEx : Thread
   {
      Call call;

      ThreadEx() {call.clear();}
   };
   Mems<ThreadEx> _threads;
   Memc<Call    > _calls;
   SyncCounter    _wake_threads, _queued_finished;
   SyncEvent      _finished;
   SyncLock       _lock_calls, _lock_process;
   void         (*_func)(Ptr data, Ptr user, Int thread_index);
   union
   {
      Ptr         _func_data;
     _Memb       *_func_memb;
     _Memx       *_func_memx;
   };
   Ptr            _func_user;
   Byte           _func_mode;
   Bool           _ordered;
   Int            _left, _processed, _elms, _elm_size, _calls_pos, _waiting;

#if EE_PRIVATE
   Bool callsLeft();
   void free     ();
   void checkEnd ();
#endif

#if EE_PRIVATE // "if" on purpose because there's already 'private' above, but we want to set it as private even in engine mode
private:
#endif
   void _process(             Int elms,               void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread);
   void _process( Ptr   data, Int elms, Int elm_size, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread, Bool data_ptr);
   void _process(_Memb &data,                         void func(Ptr    data     , Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread);
   void _process(_Memx &data,                         void func(Ptr    data     , Ptr user, Int thread_index), Ptr user, Int max_threads, Bool allow_processing_on_this_thread);

   void _queue (Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user);
   void _queue (Ptr data, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user);
   Int  _cancel(Ptr data, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user);
   Int  _cancel(          void func(Ptr    data     , Ptr user, Int thread_index), Ptr user);
   Int  _cancel(          void func(Ptr    data     , Ptr user, Int thread_index)          );
   void _wait  (Ptr data, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user);
   void _wait  (          void func(Ptr    data     , Ptr user, Int thread_index), Ptr user);
   void _wait  (          void func(Ptr    data     , Ptr user, Int thread_index)          );
   Int  _queued(Ptr data, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user)C;
   Int  _queued(          void func(Ptr    data     , Ptr user, Int thread_index), Ptr user)C;
   Int  _queued(          void func(Ptr    data     , Ptr user, Int thread_index)          )C;
   Bool _busy  (Ptr data, void func(Ptr    data     , Ptr user, Int thread_index), Ptr user)C;
   Bool _busy  (          void func(Ptr    data     , Ptr user, Int thread_index), Ptr user)C;
   Bool _busy  (          void func(Ptr    data     , Ptr user, Int thread_index)          )C;
};
/******************************************************************************/
struct ConsoleProcess // allows running console processes and reading their output
{
   // get
   Bool created ()C;                     // if the process is created             (this will be false only if the process hasn't yet been created, or if it has been manually deleted, if the process was created but stopped running, the return value will be still true)
   Bool active  ()C;                     // if the process is created and running (this will be false      if the process hasn't yet been created, or if it has been manually deleted,                          or it stopped running                                     )
   Int  exitCode()C {return _exit_code;} // get exit code of the process

   // manage
   void del      (); // request process to be closed and close all handles without killing it
   Bool create   (C Str &name  , C Str &params=S, Bool hidden=true, Bool binary=false); // 'name'=process file to execute, 'params'=custom params to be passed to the process, 'hidden'=if start the process as hidden, 'binary'=if treat the output as binary data
   Bool createMem(C Str &script, C Str &cur_dir , Bool hidden=true, Bool binary=false); // 'script'=list of commands to be executed, multiple commands should be separated with new line '\n' character, commands will be executed by cmd.exe on Windows (they should be BAT compatible) and bash on Unix (they should be SH compatible), 'cur_dir'=directory at which the commands should be executed, 'hidden'=if start the process as hidden, 'binary'=if treat the output as binary data

   // operations
   Bool wait(Int milliseconds=-1); // wait 'milliseconds' for process to close (-1 for infinite), false on fail
   void stop(); // request process to be closed
   void kill(); // kill the process
   
   // io
   Str get(); // read new data that the console did output

  ~ConsoleProcess() {del();}
   ConsoleProcess() {_binary=false; _exit_code=-1; _proc_id=0; _proc=_out_read=_in_write=PLATFORM(null, 0);}

#if !EE_PRIVATE
private:
#endif
   Bool               _binary;
   Int                _exit_code;
   UInt               _proc_id;
#if EE_PRIVATE
PLATFORM(HANDLE, Int) _proc, _out_read, _in_write;
#else
   PLATFORM(Ptr, Int) _proc, _out_read, _in_write;
#endif
   Str8               _data;
   SyncLock           _lock;
   Thread             _thread;

   NO_COPY_CONSTRUCTOR(ConsoleProcess);
};
/******************************************************************************/
// Atomic operations
Int AtomicInc(Int &x); // increase value of 'x' by 1 in an atomic operation and return its previous value, this is a thread-safe version of function "return x++;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Int AtomicDec(Int &x); // decrease value of 'x' by 1 in an atomic operation and return its previous value, this is a thread-safe version of function "return x--;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)

Int  AtomicAdd(Int  &x, Int  y); // increase value of 'x' by 'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int  old=x; x+=y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Long AtomicAdd(Long &x, Long y); // increase value of 'x' by 'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Long old=x; x+=y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Int  AtomicSub(Int  &x, Int  y); // decrease value of 'x' by 'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int  old=x; x-=y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Long AtomicSub(Long &x, Long y); // decrease value of 'x' by 'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Long old=x; x-=y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)

Int AtomicAnd    (Int &x, Int y); // and value of 'x' by  'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int old=x; x&= y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Int AtomicDisable(Int &x, Int y); // and value of 'x' by '~y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int old=x; x&=~y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Int AtomicOr     (Int &x, Int y); // or  value of 'x' by  'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int old=x; x|= y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Int AtomicXor    (Int &x, Int y); // xor value of 'x' by  'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int old=x; x^= y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)

Int  AtomicGet(C Int  &x        ); // get value of 'x'        in an atomic operation, this is a thread-safe version of function "return x;" (this allows to access the value across multiple threads without usage of Synchronization Locks), this method
Long AtomicGet(C Long &x        ); // get value of 'x'        in an atomic operation, this is a thread-safe version of function "return x;" (this allows to access the value across multiple threads without usage of Synchronization Locks), this method
Flt  AtomicGet(C Flt  &x        ); // get value of 'x'        in an atomic operation, this is a thread-safe version of function "return x;" (this allows to access the value across multiple threads without usage of Synchronization Locks)
void AtomicSet(  Int  &x, Int  y); // set value of 'x' to 'y' in an atomic operation, this is a thread-safe version of function "x=y;     " (this allows to modify the value across multiple threads without usage of Synchronization Locks)
void AtomicSet(  Long &x, Long y); // set value of 'x' to 'y' in an atomic operation, this is a thread-safe version of function "x=y;     " (this allows to modify the value across multiple threads without usage of Synchronization Locks)
void AtomicSet(  Flt  &x, Flt  y); // set value of 'x' to 'y' in an atomic operation, this is a thread-safe version of function "x=y;     " (this allows to modify the value across multiple threads without usage of Synchronization Locks)

Int AtomicGetSet(Int &x, Int y); // set value of 'x' to 'y' in an atomic operation and return its previous value, this is a thread-safe version of function "Int old=x; x=y; return old;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)

Bool AtomicCAS(Int  &x, Int  compare, Int  new_value); // set value of 'x' to 'new_value' if 'x' is equal to 'compare' in an atomic operation, this is a thread-safe version of function "if(x==compare){x=new_value; return true;} return false;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Bool AtomicCAS(Long &x, Long compare, Long new_value); // set value of 'x' to 'new_value' if 'x' is equal to 'compare' in an atomic operation, this is a thread-safe version of function "if(x==compare){x=new_value; return true;} return false;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)
Bool AtomicCAS(Flt  &x, Flt  compare, Flt  new_value); // set value of 'x' to 'new_value' if 'x' is equal to 'compare' in an atomic operation, this is a thread-safe version of function "if(x==compare){x=new_value; return true;} return false;" (this allows to modify the value across multiple threads without usage of Synchronization Locks)

// Thread functions
UIntPtr GetThreadId          (                                             ); // get current thread id
UIntPtr GetThreadIdFromWindow(Ptr hwnd                                     ); // get id of the thread which owns the OS window handle
void    SetThreadName        (C Str8 &name, UIntPtr thread_id=GetThreadId()); // set custom thread name for debugging purpose

void ThreadMayUseGPUData       (); // call    this from a secondary thread if you expect the thread to perform any operations on GPU data (like Mesh, Material, Image, Shaders, ..., this includes any operation like creating, editing, loading, saving, deleting, ...). This function is best called at the start of the thread, it needs to be called at least once, further calls are ignored. Once the function is called, the thread locks a secondary OpenGL context (if no context is available, then the function waits until other threads finish processing and release their context lock, amount of OpenGL contexts is specified in 'D.secondaryOpenGLContexts'). Context lock is automatically released once the thread exits. This call is required only for OpenGL renderer.
void ThreadFinishedUsingGPUData(); // calling this function is optional (it does not need to be called manually), call it if you wish to manually release a thread from locking an OpenGL context. Threads automatically call this function at end of their life, which means that they automatically release any locked contexts, however in some scenarios you may want to manually release any locked context if you wish to provide more contexts for background processing on other threads. This call is used only for OpenGL renderer.

// Multi-threaded calls
   // base functions, do not use
   void _MultiThreadedCall( Ptr   data, Int elms, Int elm_size, void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr);
   void _MultiThreadedCall(_Memb &data,                         void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr);
   void _MultiThreadedCall(_Memx &data,                         void func(Ptr data, Ptr user, Int thread_index), Ptr user, Int threads, Bool data_ptr);

   // perform multi-threaded call on 'func' function, by giving a unique 'elm_index' from the "0 .. elms-1" range as the function parameter
                   void MultiThreadedCall(Int elms, void func(Int elm_index, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads());
   T1(USER_DATA)   void MultiThreadedCall(Int elms, void func(Int elm_index, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {MultiThreadedCall(elms, (void (*)(Int elm_index, Ptr user, Int thread_index))func,  user, threads);}
   T1(USER_DATA)   void MultiThreadedCall(Int elms, void func(Int elm_index, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {MultiThreadedCall(elms, (void (*)(Int elm_index, Ptr user, Int thread_index))func, &user, threads);}

   // perform multi-threaded call on 'func' function, by giving a unique 'data' element from the memory container as the function parameter
   T1(DATA           )   void MultiThreadedCall(Mems<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T1(DATA           )   void MultiThreadedCall(Mems<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T1(DATA           )   void MultiThreadedCall(Memc<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T1(DATA           )   void MultiThreadedCall(Memc<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T1(DATA           )   void MultiThreadedCall(Memt<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T1(DATA           )   void MultiThreadedCall(Memt<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T1(DATA           )   void MultiThreadedCall(Memb<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T1(DATA           )   void MultiThreadedCall(Memb<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T1(DATA           )   void MultiThreadedCall(Memx<DATA > &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T1(DATA           )   void MultiThreadedCall(Memx<DATA*> &data, void func(DATA &data, Ptr        user, Int thread_index), Ptr        user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Mems<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Mems<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memc<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memc<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memt<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memt<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memb<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memb<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memx<DATA > &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memx<DATA*> &data, void func(DATA &data, USER_DATA *user, Int thread_index), USER_DATA *user=null, Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func,  user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Mems<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Mems<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memc<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memc<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memt<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memt<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data.data(), data.elms(), data.elmSize(), (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memb<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memb<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, true );}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memx<DATA > &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, false);}
   T2(DATA, USER_DATA)   void MultiThreadedCall(Memx<DATA*> &data, void func(DATA &data, USER_DATA &user, Int thread_index), USER_DATA &user     , Int threads=Cpu.threads()) {_MultiThreadedCall(data                                    , (void (*)(Ptr data, Ptr user, Int thread_index))func, &user, threads, true );}

// Process functions
void ProcPriority(  Int  priority               ); // set   process priority (-2..2)
void ProcClose   ( UInt  id                     ); // close process
void ProcClose   (C Str &name                   ); // close process
Bool ProcKill    ( UInt  id                     ); // kill  process, false on fail
Bool ProcKill    (C Str &name                   ); // kill  process, false on fail
Bool ProcWait    ( UInt  id, Int milliseconds=-1); // wait  for process to exit (<0 = infinite wait), false on timeout
Ptr  ProcWindow  ( UInt  id                     ); // get   OS window handle of process
Str  ProcName    ( UInt  id                     ); // get   process name
UInt ProcFind    (C Str &name                   ); // find  process ID based on its executable name, name can be either a full path or just the base name
void ProcList    (         MemPtr<UInt> id      ); // get   list of process ID's   , after calling this function the 'id'      will contain a list of process ID's
void ProcModules (UInt id, MemPtr<Str > modules ); // get   list of process modules, after calling this function the 'modules' will contain a list of process modules

struct ProcessAccess
{
   UInt proc_id; // Process ID that has a handle opened to our process
   Bool write  ; // if the handle has write permission
};
Bool GetProcessesAccessingThisProcess(MemPtr<ProcessAccess> proc, Bool write_only=false, Mems<Byte> *temp=null); // get a list of processes that are accessing this process (you may need to run with admin rights to detect all processes), 'write_only'=if only detect processes that have write permission, 'temp'=optional container for temporary memory allocation that can be reused instead of allocating new memory each time, false on fail
/******************************************************************************/
#if EE_PRIVATE
struct ThreadEmulation
{
   void include(Thread &thread);
   void exclude(Thread &thread);

   void update();

   ThreadEmulation();

private:
   struct DelayedThread
   {
      Int     waited;
      Thread *thread;

      DelayedThread() {waited=0;}
   };
   Int                  _process_left;
   UInt                 _process_type, _time;
   Memc<       Thread*>      _rt_threads;
   Memc<DelayedThread > _delayed_threads;
}extern
   EmulatedThreads;

   #if HAS_THREADS
      INLINE void UpdateThreads() {}
   #else
      INLINE void UpdateThreads() {EmulatedThreads.update();}
   #endif

        INLINE UIntPtr _GetThreadId() {return PLATFORM(GetCurrentThreadId(), (UIntPtr)pthread_self());}
   #define GetThreadId _GetThreadId // use this macro so all engine functions access '_GetThreadId' directly
#endif
/******************************************************************************/
