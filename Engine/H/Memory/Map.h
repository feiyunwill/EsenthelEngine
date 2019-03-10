/******************************************************************************

   Use 'Map' to quickly access custom data by creating it from specified key.
      Once 'Map' creates a resource, it will keep it in the memory for faster access.

   Objects in 'Map' containers are stored using 'Memx' container,
      which means that the memory address of the elements remains constant as long as the elements exist.

/******************************************************************************/
enum MAP_MODE : Byte // Map Mode
{
   MAP_EXIT     , //       load data, Exit  on fail
   MAP_NULL     , //       load data, null  on fail
   MAP_DUMMY    , //       load data, dummy on fail       (pointer to empty data, initialized with constructor but without the 'load' method)
   MAP_ALL_NULL , // don't load data, always return null
   MAP_ALL_DUMMY, // don't load data, always return dummy (pointer to empty data, initialized with constructor but without the 'load' method)
};
#if EE_PRIVATE
enum MAP_ELM_FLAG // Map Element Flag
{
   MAP_ELM_DUMMY  =0x1,
   MAP_ELM_LOADING=0x2,
};
#endif
/******************************************************************************/
T2(KEY, DATA) struct Map : _Map // Map - container for dynamically created elements, consisting of unique keys and their corresponding data, Map is multi-threaded safe
{
   struct Elm : _Map::Elm
   {
      DATA data;
      KEY  key ;
      Desc desc;
   };

   // manage
   Map& clear(); // remove all elements
   Map& del  (); // remove all elements and free helper memory

   // get
   Int elms    ()C; // get number of elements in container
   Int dataSize()C; // get size of DATA element

   DATA* find      (C KEY &key); // find    element, don't create if not found, null on fail
   DATA* get       (C KEY &key); // get     element,       create if not found, null on fail
   DATA* operator()(C KEY &key); // require element,       create if not found, Exit on fail (unless different MAP_MODE selected)

   Int    findAbsIndex(C KEY &key)C; // find    element absolute index, don't create if not found,   -1 on fail
   Int     getAbsIndex(C KEY &key) ; // get     element absolute index,       create if not found,   -1 on fail
   Int requireAbsIndex(C KEY &key) ; // require element absolute index,       create if not found, Exit on fail (unless different MAP_MODE selected)

   Bool containsKey   (C KEY  &key )C; // check if map contains an element with specified key
   Bool containsData  (C DATA *data)C; // check if map contains an element, testing is done by comparing elements memory address only
 C KEY* dataToKey     (C DATA *data)C; // get element key, this will return pointer to element's key if that element is      stored in this Map, null on fail
 C KEY* dataInMapToKey(C DATA *data)C; // get element key, this will return pointer to element's key,   that element must be stored in this Map or be null, this method is faster than 'key' because it does not check if element is stored in this Map
 C KEY& dataInMapToKey(C DATA &data)C; // get element key, this will return            element's key,   that element must be stored in this Map           , this method is faster than 'key' because it does not check if element is stored in this Map
   Int  dataToIndex   (C DATA *data)C; // get element index in map, -1 on fail (if not stored in this Map)

   // operations
 C KEY & key       (Int i)C; // access i-th element key  from container
   DATA& operator[](Int i) ; // access i-th element data from container
 C DATA& operator[](Int i)C; // access i-th element data from container

 C KEY & absKey (Int abs_i)C; // access i-th absolute element key  from container, 'abs_i'=absolute index of the element
   DATA& absData(Int abs_i) ; // access i-th absolute element data from container, 'abs_i'=absolute index of the element
 C DATA& absData(Int abs_i)C; // access i-th absolute element data from container, 'abs_i'=absolute index of the element

   MAP_MODE mode(MAP_MODE mode); // set map mode, returns previous mode

   void remove    (  Int   i   ); // remove i-th element from container
   void removeKey (C KEY  &key ); // remove      element from container
   void removeData(C DATA *data); // remove      element from container
   Bool replaceKey(C KEY  &src, C KEY &dest); // replace existing element 'src' key with 'dest', false on fail

   T1(EXTENDED) Map& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'DATA' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   Map& operator=(C Map &src); // create from 'src'

   explicit Map(Int compare(C KEY &a, C KEY &b)=Compare, Bool create(DATA &data, C KEY &key, Ptr user)=null, Ptr user=null, Int block_elms=64); // 'compare'=function which compares two keys, 'create'=function that creates 'data' on the base of the constant 'key'
};
/******************************************************************************/
T2(KEY, DATA) struct ThreadSafeMap : _MapTS // Thread Safe Map
{
   // manage
   ThreadSafeMap& clear(); // remove all elements
   ThreadSafeMap& del  (); // remove all elements and free helper memory

   // get
   Int elms    ()C; // get number of elements in container
   Int dataSize()C; // get size of DATA element

   DATA* find      (C KEY &key); // find    element, don't create if not found, null on fail
   DATA* get       (C KEY &key); // get     element,       create if not found, null on fail
   DATA* operator()(C KEY &key); // require element,       create if not found, Exit on fail (unless different MAP_MODE selected)

   Int    findAbsIndex(C KEY &key)C; // find    element absolute index, don't create if not found,   -1 on fail
   Int     getAbsIndex(C KEY &key) ; // get     element absolute index,       create if not found,   -1 on fail
   Int requireAbsIndex(C KEY &key) ; // require element absolute index,       create if not found, Exit on fail (unless different MAP_MODE selected)

   Bool containsKey   (C KEY  &key )C; // check if map contains an element with specified key
   Bool containsData  (C DATA *data)C; // check if map contains an element, testing is done by comparing elements memory address only
 C KEY* dataToKey     (C DATA *data)C; // get element key, this will return pointer to element's key if that element is      stored in this Map, null on fail
 C KEY* dataInMapToKey(C DATA *data)C; // get element key, this will return pointer to element's key,   that element must be stored in this Map or be null, this method is faster than 'key' because it does not check if element is stored in this Map
 C KEY& dataInMapToKey(C DATA &data)C; // get element key, this will return            element's key,   that element must be stored in this Map           , this method is faster than 'key' because it does not check if element is stored in this Map
   Int  dataToIndex   (C DATA *data)C; // get element index in map, -1 on fail (if not stored in this Map)

   // operations
   void   lock()C; //   lock elements container, unlock must be called after locking container
   void unlock()C; // unlock elements container, this   must be called after locking the container

 C KEY & lockedKey (Int i)C; // access i-th element key  from container, this can be used after locking and before unlocking the container
   DATA& lockedData(Int i) ; // access i-th element data from container, this can be used after locking and before unlocking the container
 C DATA& lockedData(Int i)C; // access i-th element data from container, this can be used after locking and before unlocking the container

 C KEY & lockedAbsKey (Int abs_i)C; // access i-th absolute element key  from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element
   DATA& lockedAbsData(Int abs_i) ; // access i-th absolute element data from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element
 C DATA& lockedAbsData(Int abs_i)C; // access i-th absolute element data from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element

   MAP_MODE mode(MAP_MODE mode); // set map mode, returns previous mode

   void remove    (  Int   i   ); // remove i-th element from container
   void removeKey (C KEY  &key ); // remove      element from container
   void removeData(C DATA *data); // remove      element from container
   Bool replaceKey(C KEY  &src, C KEY &dest); // replace existing element 'src' key with 'dest', false on fail

   T1(EXTENDED) ThreadSafeMap& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'DATA' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   ThreadSafeMap& operator=(C ThreadSafeMap &src); // create from 'src'

   explicit ThreadSafeMap(Int compare(C KEY &a, C KEY &b)=Compare, Bool create(DATA &data, C KEY &key, Ptr user)=null, Ptr user=null, Int block_elms=64); // 'compare'=function which compares two keys, 'create'=function that creates 'data' on the base of the constant 'key'
};
/******************************************************************************/
struct MapLock // Map Lock (automatically locks and unlocks the map at object creation and destruction)
{
   explicit MapLock(_MapTS &map) : _map(map) {_map.  lock();}
           ~MapLock(           )             {_map.unlock();}

private:
  _MapTS &_map;
   NO_COPY_CONSTRUCTOR(MapLock);
};
/******************************************************************************/
