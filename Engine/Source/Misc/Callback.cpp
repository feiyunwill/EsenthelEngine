/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Callbacks::add(void func(        )          ) {if(func!=null)_callbacks     .New().set(func      );}
void Callbacks::add(void func(Ptr user), Ptr user) {if(func!=null)_callbacks_user.New().set(func, user);}

void Callbacks::include(void func(        )          ) {if(func!=null){REPA(_callbacks     )if(_callbacks     [i].func==func                                 )return; add(func      );}}
void Callbacks::include(void func(Ptr user), Ptr user) {if(func!=null){REPA(_callbacks_user)if(_callbacks_user[i].func==func && _callbacks_user[i].user==user)return; add(func, user);}}

void Callbacks::exclude(void func(        )          ) {if(func!=null)REPA(_callbacks     )if(_callbacks     [i].func==func                                 )_callbacks.remove(i, true);}
void Callbacks::exclude(void func(Ptr user), Ptr user) {if(func!=null)REPA(_callbacks_user)if(_callbacks_user[i].func==func && _callbacks_user[i].user==user)_callbacks.remove(i, true);}

void Callbacks::del()
{
  _callbacks     .del(); _temp_callbacks     .del();
  _callbacks_user.del(); _temp_callbacks_user.del();
}

void Callbacks::update()
{
   // call in order in which they were queued, operate on temp in case callbacks themself queue new callbacks

   // backup both callbacks first
   Swap(_callbacks     , _temp_callbacks     );
   Swap(_callbacks_user, _temp_callbacks_user);

   // now call them and clear them
   FREPAO(_temp_callbacks     ).call(); _temp_callbacks     .clear();
   FREPAO(_temp_callbacks_user).call(); _temp_callbacks_user.clear();
}

void ThreadSafeCallbacks::add    (void func(        )          ) {if(func!=null){SyncLocker locker(_lock); super::add    (func      );}}
void ThreadSafeCallbacks::add    (void func(Ptr user), Ptr user) {if(func!=null){SyncLocker locker(_lock); super::add    (func, user);}}
void ThreadSafeCallbacks::include(void func(        )          ) {if(func!=null){SyncLocker locker(_lock); super::include(func      );}}
void ThreadSafeCallbacks::include(void func(Ptr user), Ptr user) {if(func!=null){SyncLocker locker(_lock); super::include(func, user);}}
#if SYNC_LOCK_SAFE
void ThreadSafeCallbacks::exclude(void func(        )          ) {if(func!=null){SyncLocker locker(_lock); super::exclude(func      );}}
void ThreadSafeCallbacks::exclude(void func(Ptr user), Ptr user) {if(func!=null){SyncLocker locker(_lock); super::exclude(func, user);}}
#else
void ThreadSafeCallbacks::exclude(void func(        )          ) {if(func!=null && _callbacks     .elms()){SyncLocker locker(_lock); super::exclude(func      );}}
void ThreadSafeCallbacks::exclude(void func(Ptr user), Ptr user) {if(func!=null && _callbacks_user.elms()){SyncLocker locker(_lock); super::exclude(func, user);}}
#endif
void ThreadSafeCallbacks::update (                             ) {if(_callbacks.elms() || _callbacks_user.elms()){SyncLocker locker(_lock); super::update();}}
void ThreadSafeCallbacks::del    (                             ) {SyncLocker locker(_lock); super::del();}
/******************************************************************************/
}
/******************************************************************************/
