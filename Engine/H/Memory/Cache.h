/******************************************************************************

   Use 'Cache' to quickly access custom data by loading it from specified file.
      Once 'Cache' loads a resource, it will keep it in the memory for faster access.
      You can also use 'Cache' to access file name of already loaded element.

   Objects in 'Cache' containers are stored using 'Memx' container,
      which means that the memory address of the elements remains constant as long as the elements exist.

   'Cache' loads the data and stores it forever (as long as the cache lives),
      in order to load the data for as long as it is needed, you must use 'CacheElmPtr'.

   'CacheElmPtr' can be treated like a typical pointer, with the exception that when copied or destroyed,
      it automatically increases/decreses the reference count of an element inside a cache.
      Once the reference count of an element drops to zero, it means that there are no more 'CacheElmPtr'
      pointers referencing the data, and the data is released.

   Engine uses the 'Cache' class to store many types of data, for example:
      'Images'    - cache for images   , with 'ImagePtr'    typedef for the 'CacheElmPtr' template pointer
      'Materials' - cache for materials, with 'MaterialPtr' typedef for the 'CacheElmPtr' template pointer
      'Meshes'    - cache for meshes   , with 'MeshPtr'     typedef for the 'CacheElmPtr' template pointer
      ..

/******************************************************************************/
enum CACHE_MODE : Byte // Cache Mode
{
   CACHE_EXIT      , //       load data, Exit  on fail
   CACHE_NULL      , //       load data, null  on fail
   CACHE_DUMMY     , //       load data, dummy on fail       (pointer to empty data with correct path, initialized with constructor but without the 'load' method)
   CACHE_ALL_NULL  , // don't load data, always return null
   CACHE_ALL_DUMMY , // don't load data, always return dummy (pointer to empty data with correct path, initialized with constructor but without the 'load' method)
   CACHE_DUMMY_NULL, // don't load data,        return dummy (pointer to empty data with correct path, initialized with constructor but without the 'load' method) if a file exists at specified path, if a file does not exist then null is returned
};
#if EE_PRIVATE
enum CACHE_ELM_FLAG // Cache Element Flag
{
   CACHE_ELM_DUMMY       =1<<0, // if element was not found but created anyway
   CACHE_ELM_LOADING     =1<<1, // if element is still being loaded (for example, during loading of element A, it loads element B, which tries to access A which didn't finish loading yet)
   CACHE_ELM_STD_PTR     =1<<2, // if element was accessed by standard C++ pointer (not reference counted pointer)
   CACHE_ELM_DELAY_REMOVE=1<<3, // if element reached zero references and was added to the '_delay_remove'
};
#endif
/******************************************************************************/
// CACHE
/******************************************************************************/
T1(TYPE) struct Cache : _Cache // Cache - container for dynamically loaded data, used for fast accessing data through file name, 'Cache' is multi-threaded safe
{
   struct Elm : _Cache::Elm
   {
      TYPE data;
      Desc desc;
   };

   // manage
   CACHE_MODE mode          (CACHE_MODE mode); // set cache mode, returns previous mode
   Cache&     caseSensitive (Bool  sensitive); // set if cache should use case sensitive paths for accessing resources, default=false
   Cache&     delayRemove   (Flt   time     ); // set amount of time (in seconds) after which unused elements are removed from cache (<=0 value specifies immediate unloading), default=0
   Cache&     delayRemoveNow(               ); // immediately remove all elements marked for delay removal at a later time to free as much memory as possible
   Cache&     delayRemoveInc(               ); // increase the cache "delay remove" counter thanks to which elements will not be immediately removed when they're no longer referenced, 'delayRemoveDec' should be called after this method
   Cache&     delayRemoveDec(               ); // decrease the cache "delay remove" counter thanks to which elements will not be immediately removed when they're no longer referenced, this should be called after 'delayRemoveInc', once the counter goes back to zero then all non referenced elements will be removed

   // get object and store it forever (as long as the Cache lives)
   TYPE* find      (C Str &file, CChar *path=null); // find    object by its file name   , don't load if not found, null on fail
   TYPE* find      (C UID &id  , CChar *path=null); // find    object by its file name ID, don't load if not found, null on fail
   TYPE* get       (C Str &file, CChar *path=null); // get     object by its file name   ,       load if not found, null on fail
   TYPE* get       (C UID &id  , CChar *path=null); // get     object by its file name ID,       load if not found, null on fail
   TYPE* operator()(C Str &file, CChar *path=null); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   TYPE* operator()(C UID &id  , CChar *path=null); // require object by its file name ID,       load if not found, Exit on fail (unless different CACHE_MODE selected)

   // get object file name (this will return pointer to internally stored file name assuming that the object is stored in this Cache)
                                 CChar* name(C             TYPE        *data, CChar *path=null)C;                             // get object file name, null on fail
   template<Cache<TYPE> &CACHE>  CChar* name(C CacheElmPtr<TYPE,CACHE> &data, CChar *path=null)C {return name(data(), path);} // get object file name, null on fail

   // get object file name ID (this will return ID of the object file name assuming that the object is stored in the cache and its file name was created using 'EncodeFileName')
                                 UID id(C             TYPE        *data)C;                     // get object file name ID, 'UIDZero' on fail
   template<Cache<TYPE> &CACHE>  UID id(C CacheElmPtr<TYPE,CACHE> &data)C {return id(data());} // get object file name ID, 'UIDZero' on fail

   // get
   Int elms()C; // get number of elements in container

                                 Bool contains(C             TYPE        *data            )C;                                  // check if cache contains this object
   template<Cache<TYPE> &CACHE>  Bool contains(C CacheElmPtr<TYPE,CACHE> &data            )C {return contains(data()       );} // check if cache contains this object
                                 Int  ptrCount(C             TYPE        *data            )C;                                  // check if cache contains this object and return current number of active pointer references to it, -1 is returned if object is not stored in this cache
   template<Cache<TYPE> &CACHE>  Int  ptrCount(C CacheElmPtr<TYPE,CACHE> &data            )C {return ptrCount(data()       );} // check if cache contains this object and return current number of active pointer references to it, -1 is returned if object is not stored in this cache
                                 Bool dummy   (C             TYPE        *data            )C;                                  // check if cache contains this object and it's a dummy (it was not loaded but created as empty)
   template<Cache<TYPE> &CACHE>  Bool dummy   (C CacheElmPtr<TYPE,CACHE> &data            )C {return dummy   (data()       );} // check if cache contains this object and it's a dummy (it was not loaded but created as empty)
                                 void dummy   (C             TYPE        *data, Bool dummy) ;                                  //       if cache contains this object then set its dummy state (this can be used for example if object was first loaded as a dummy, but then you've downloaded/generated/saved its data, and now need to update the dummy state)
   template<Cache<TYPE> &CACHE>  void dummy   (C CacheElmPtr<TYPE,CACHE> &data, Bool dummy)  {     T.dummy   (data(), dummy);} //       if cache contains this object then set its dummy state (this can be used for example if object was first loaded as a dummy, but then you've downloaded/generated/saved its data, and now need to update the dummy state)

   // operations
   void   lock      (     )C; //   lock      elements          container, unlock must be called after locking container
 C Desc&  lockedDesc(Int i)C; // access i-th element desc from container, this   can  be used   after locking and before unlocking the container
   TYPE&  lockedData(Int i) ; // access i-th element data from container, this   can  be used   after locking and before unlocking the container
 C TYPE&  lockedData(Int i)C; // access i-th element data from container, this   can  be used   after locking and before unlocking the container
   void unlock      (     )C; // unlock      elements          container, this   must be called after locking the container
#if EE_PRIVATE
   void canBeRemoved(Bool CanBeRemoved(C TYPE &data)) {lock(); _can_be_removed=(Bool(*)(CPtr))CanBeRemoved; unlock();}
#endif

   void removeData(C TYPE *data); // manually remove object from cache, this is ignored for objects which still are accessed by some CacheElmPtr's

   void update(); // update cache to process all delay removed elements

   T1(EXTENDED) Cache& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   void setLoadUser(Ptr user); // if this method is called, then elements will be loaded using their "Bool load(C Str &name, Ptr user)" method instead, with 'user' as its parameter

   Cache& operator=(C Cache &src); // create from 'src'

   Cache& clear(); // remove all elements
   Cache& del  (); // remove all elements and free helper memory

   explicit Cache(CChar8 *name=null, Int block_elms=64); // 'name'=cache name (this value is optional, it will be used when displaying an error message when cache element failed to load)
};
/******************************************************************************/
// CACHE ELEMENT POINTER
/******************************************************************************/
template<typename TYPE, Cache<TYPE> &CACHE>   struct CacheElmPtr // Cache Element Pointer - can hold a reference to a TYPE based object in the CACHE cache, number of active references for a given object is stored in the cache
{
   // operators
   TYPE* operator ()  (                   )C {return  T._data            ;} // access the data, you can use the returned pointer   as long as this 'CacheElmPtr' object exists and not modified
   TYPE* operator ->  (                   )C {return  T._data            ;} // access the data, you can use the returned pointer   as long as this 'CacheElmPtr' object exists and not modified
   TYPE& operator *   (                   )C {return *T._data            ;} // access the data, you can use the returned reference as long as this 'CacheElmPtr' object exists and not modified
   Bool  operator ==  (  null_t           )C {return  T._data==null      ;} // if pointers are equal
   Bool  operator !=  (  null_t           )C {return  T._data!=null      ;} // if pointers are different
   Bool  operator ==  (C TYPE        *data)C {return  T._data==data      ;} // if pointers are equal
   Bool  operator !=  (C TYPE        *data)C {return  T._data!=data      ;} // if pointers are different
   Bool  operator ==  (C CacheElmPtr &eptr)C {return  T._data==eptr._data;} // if pointers are equal
   Bool  operator !=  (C CacheElmPtr &eptr)C {return  T._data!=eptr._data;} // if pointers are different
         operator Bool(                   )C {return  T._data!=null      ;} // if pointer  is  valid

   // get object file name (this will return pointer to internally stored file name assuming that the object is stored in the cache)
   CChar* name(CChar *path=null)C; // get object file name, null on fail

   // get object file name ID (this will return ID of the object file name assuming that the object is stored in the cache and its file name was created using 'EncodeFileName')
   UID id()C; // get object file name ID, 'UIDZero' on fail

   // get
   Bool dummy(          )C; // check if this object is a dummy (it was not loaded but created as empty)
   void dummy(Bool dummy) ; // set dummy state for this object (this can be used for example if object was first loaded as a dummy, but then you've downloaded/generated/saved its data, and now need to update the dummy state)

   // operations
   CacheElmPtr& clear    (                    ); // clear the pointer to  null , this automatically decreases the reference count of current data
   CacheElmPtr& operator=(  null_t            ); // clear the pointer to  null , this automatically decreases the reference count of current data
   CacheElmPtr& operator=(  TYPE        * data); // set       pointer to 'data', this automatically decreases the reference count of current data and increases the reference count of the new data
   CacheElmPtr& operator=(C CacheElmPtr & eptr); // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data
   CacheElmPtr& operator=(  CacheElmPtr &&eptr); // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data

   // get object and store it temporarily (as long as it is referenced by at least one 'CacheElmPtr')
   CacheElmPtr& find     (CChar  *file, CChar *path=null); // find    object by its file name   , don't load if not found, null on fail
   CacheElmPtr& find     (CChar8 *file, CChar *path=null); // find    object by its file name   , don't load if not found, null on fail
   CacheElmPtr& find     (C Str  &file, CChar *path=null); // find    object by its file name   , don't load if not found, null on fail
   CacheElmPtr& find     (C Str8 &file, CChar *path=null); // find    object by its file name   , don't load if not found, null on fail
   CacheElmPtr& find     (C UID  &id  , CChar *path=null); // find    object by its file name ID, don't load if not found, null on fail
   CacheElmPtr& get      (CChar  *file, CChar *path=null); // get     object by its file name   ,       load if not found, null on fail
   CacheElmPtr& get      (CChar8 *file, CChar *path=null); // get     object by its file name   ,       load if not found, null on fail
   CacheElmPtr& get      (C Str  &file, CChar *path=null); // get     object by its file name   ,       load if not found, null on fail
   CacheElmPtr& get      (C Str8 &file, CChar *path=null); // get     object by its file name   ,       load if not found, null on fail
   CacheElmPtr& get      (C UID  &id  , CChar *path=null); // get     object by its file name ID,       load if not found, null on fail
   CacheElmPtr& require  (CChar  *file, CChar *path=null); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr& require  (CChar8 *file, CChar *path=null); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr& require  (C Str  &file, CChar *path=null); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr& require  (C Str8 &file, CChar *path=null); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr& require  (C UID  &id  , CChar *path=null); // require object by its file name ID,       load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr& operator=(CChar  *file                  ); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected), works exactly the same as 'require' method, however without the option of specifying additional 'path'
   CacheElmPtr& operator=(CChar8 *file                  ); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected), works exactly the same as 'require' method, however without the option of specifying additional 'path'
   CacheElmPtr& operator=(C Str  &file                  ); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected), works exactly the same as 'require' method, however without the option of specifying additional 'path'
   CacheElmPtr& operator=(C Str8 &file                  ); // require object by its file name   ,       load if not found, Exit on fail (unless different CACHE_MODE selected), works exactly the same as 'require' method, however without the option of specifying additional 'path'
   CacheElmPtr& operator=(C UID  &id                    ); // require object by its file name ID,       load if not found, Exit on fail (unless different CACHE_MODE selected), works exactly the same as 'require' method, however without the option of specifying additional 'path'

   // constructors / destructors
   CacheElmPtr(  null_t=null       ); // initialize the pointer with  null
   CacheElmPtr(  TYPE        * data); // initialize the pointer with 'data', this automatically increases the reference count of the    'data'
   CacheElmPtr(C CacheElmPtr & eptr); // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
   CacheElmPtr(  CacheElmPtr &&eptr); // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
   CacheElmPtr(C Str         & file); // initialize the pointer with 'file', this automatically increases the reference count of the    'file', works exactly the same as 'operator=(C Str  &file)', require object, load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr(C Str8        & file); // initialize the pointer with 'file', this automatically increases the reference count of the    'file', works exactly the same as 'operator=(C Str8 &file)', require object, load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr(  CChar       * file); // initialize the pointer with 'file', this automatically increases the reference count of the    'file', works exactly the same as 'operator=(CChar  *file)', require object, load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr(  CChar8      * file); // initialize the pointer with 'file', this automatically increases the reference count of the    'file', works exactly the same as 'operator=(CChar8 *file)', require object, load if not found, Exit on fail (unless different CACHE_MODE selected)
   CacheElmPtr(C UID         & id  ); // initialize the pointer with 'id'  , this automatically increases the reference count of the    'file', works exactly the same as 'operator=(C UID  &id  )', require object, load if not found, Exit on fail (unless different CACHE_MODE selected)
  ~CacheElmPtr(                    ); // release    the pointer            , this automatically decreases the reference count of current data

private:
   TYPE *_data;
};
/******************************************************************************/
struct CacheLock // Cache Lock (automatically locks and unlocks the cache at object creation and destruction)
{
   explicit CacheLock(C _Cache &cache) : _cache(cache) {_cache.  lock();}
           ~CacheLock(               )                 {_cache.unlock();}

private:
C _Cache &_cache;
   NO_COPY_CONSTRUCTOR(CacheLock);
};
/******************************************************************************/
#define DECLARE_CACHE(TYPE, cache_name, ptr_name            )   extern Cache<TYPE> cache_name            ;   typedef CacheElmPtr<TYPE, cache_name> ptr_name; // this declares a Cache and a CacheElmPtr in a header, the cache stores 'TYPE' data, the cache name is 'cache_name' and the name for the pointer to elements is 'ptr_name'
#define  DEFINE_CACHE(TYPE, cache_name, ptr_name, debug_name)          Cache<TYPE> cache_name(debug_name);   typedef CacheElmPtr<TYPE, cache_name> ptr_name; // this defines  a Cache and a CacheElmPtr in C++ file
#if EE_PRIVATE
#define DEFINE_CACHE_EX(TYPE, cache_name, ptr_name, debug_name, block_elms)   Cache<TYPE> cache_name(debug_name, block_elms);   typedef CacheElmPtr<TYPE, cache_name> ptr_name; // this defines a Cache and a CacheElmPtr in C++ file
#endif
/******************************************************************************/
