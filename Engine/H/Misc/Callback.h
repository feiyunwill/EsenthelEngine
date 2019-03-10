/******************************************************************************/
struct Callback
{
   void (*func)();

   void set (void func()) {T.func=func;}
   void call(           ) {     func();}
};
struct CallbackUser
{
   void (*func)(Ptr user);
   Ptr    user;

   void set (void func(Ptr user), Ptr user=null) {T.func=func; T.user=user;}
   void call(                                  ) {  func(user);}
};
/******************************************************************************/
struct Callbacks
{
#if EE_PRIVATE
   Bool initialized()C {return _callbacks.initialized();}
#endif
            void add(void func(          )                 );                                  // add custom function to the queue, it will be called in the 'update' method
            void add(void func(Ptr   user), Ptr   user=null);                                  // add custom function to the queue, it will be called in the 'update' method
   T1(TYPE) void add(void func(TYPE *user), TYPE *user=null) {add((void(*)(Ptr))func,  user);} // add custom function to the queue, it will be called in the 'update' method
   T1(TYPE) void add(void func(TYPE &user), TYPE &user     ) {add((void(*)(Ptr))func, &user);} // add custom function to the queue, it will be called in the 'update' method

            void include(void func(          )                 );                                      // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
            void include(void func(Ptr   user), Ptr   user=null);                                      // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
   T1(TYPE) void include(void func(TYPE *user), TYPE *user=null) {include((void(*)(Ptr))func,  user);} // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
   T1(TYPE) void include(void func(TYPE &user), TYPE &user     ) {include((void(*)(Ptr))func, &user);} // add custom function to the queue if it's not listed yet, it will be called in the 'update' method

            void exclude(void func(          )                 );                                      // remove function from the queue
            void exclude(void func(Ptr   user), Ptr   user=null);                                      // remove function from the queue
   T1(TYPE) void exclude(void func(TYPE *user), TYPE *user=null) {exclude((void(*)(Ptr))func,  user);} // remove function from the queue
   T1(TYPE) void exclude(void func(TYPE &user), TYPE &user     ) {exclude((void(*)(Ptr))func, &user);} // remove function from the queue

   void update(); // call all callbacks and remove them from the queue

   void del(); // manually remove all queued callbacks and delete all used memory

protected:
   Memc<Callback    > _callbacks     , _temp_callbacks;
   Memc<CallbackUser> _callbacks_user, _temp_callbacks_user;
};
/******************************************************************************/
STRUCT_PRIVATE(ThreadSafeCallbacks , Callbacks)
//{
#if EE_PRIVATE
   Bool initialized()C {return super::initialized();}
#endif
            void add(void func(          )                 );                                  // add custom function to the queue, it will be called in the 'update' method
            void add(void func(Ptr   user), Ptr   user=null);                                  // add custom function to the queue, it will be called in the 'update' method
   T1(TYPE) void add(void func(TYPE *user), TYPE *user=null) {add((void(*)(Ptr))func,  user);} // add custom function to the queue, it will be called in the 'update' method
   T1(TYPE) void add(void func(TYPE &user), TYPE &user     ) {add((void(*)(Ptr))func, &user);} // add custom function to the queue, it will be called in the 'update' method

            void include(void func(          )                 );                                      // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
            void include(void func(Ptr   user), Ptr   user=null);                                      // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
   T1(TYPE) void include(void func(TYPE *user), TYPE *user=null) {include((void(*)(Ptr))func,  user);} // add custom function to the queue if it's not listed yet, it will be called in the 'update' method
   T1(TYPE) void include(void func(TYPE &user), TYPE &user     ) {include((void(*)(Ptr))func, &user);} // add custom function to the queue if it's not listed yet, it will be called in the 'update' method

            void exclude(void func(          )                 );                                      // remove function from the queue
            void exclude(void func(Ptr   user), Ptr   user=null);                                      // remove function from the queue
   T1(TYPE) void exclude(void func(TYPE *user), TYPE *user=null) {exclude((void(*)(Ptr))func,  user);} // remove function from the queue
   T1(TYPE) void exclude(void func(TYPE &user), TYPE &user     ) {exclude((void(*)(Ptr))func, &user);} // remove function from the queue

   void update(); // call all callbacks and remove them from the queue

   void del(); // manually remove all queued callbacks and delete all used memory

   ThreadSafeCallbacks() {}

private:
   SyncLock _lock;
   NO_COPY_CONSTRUCTOR(ThreadSafeCallbacks);
};
/******************************************************************************/
