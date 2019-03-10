/******************************************************************************/
T1(const_mem_addr TYPE) struct MemcThreadSafe : _MemcThreadSafe // Thread-Safe Continuous Memory Based Container
{
   // manage
   MemcThreadSafe& clear(); // remove all elements
   MemcThreadSafe& del  (); // remove all elements and free helper memory

   // get / set
   Int  elms    ()C; // number of elements
   UInt elmSize ()C; // size   of element
   UInt memUsage()C; // memory usage

   TYPE* lockedData (     ) ; // get    pointer to the start of the elements, container must be locked first using the 'lock' method before using this method
 C TYPE* lockedData (     )C; // get    pointer to the start of the elements, container must be locked first using the 'lock' method before using this method
   TYPE* lockedAddr (Int i) ; // get    i-th  element address, null is returned if index is out of range, container must be locked first using the 'lock' method before using this method
 C TYPE* lockedAddr (Int i)C; // get    i-th  element address, null is returned if index is out of range, container must be locked first using the 'lock' method before using this method
   TYPE& lockedElm  (Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior, container must be locked first using the 'lock' method before using this method
 C TYPE& lockedElm  (Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior, container must be locked first using the 'lock' method before using this method
   TYPE& lockedFirst(     ) ; // get    first element, container must be locked first using the 'lock' method before using this method
 C TYPE& lockedFirst(     )C; // get    first element, container must be locked first using the 'lock' method before using this method
   TYPE& lockedLast (     ) ; // get    last  element, container must be locked first using the 'lock' method before using this method
 C TYPE& lockedLast (     )C; // get    last  element, container must be locked first using the 'lock' method before using this method
   TYPE& lockedNew  (     ) ; // create new   element at the  end                                                                              , this method may change the memory address of all elements, container must be locked first using the 'lock' method before using this method
   TYPE& lockedNewAt(Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method may change the memory address of all elements, container must be locked first using the 'lock' method before using this method

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   MemcThreadSafe& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   MemcThreadSafe& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   MemcThreadSafe& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   TYPE popFirst(       Bool keep_order=true); // get first element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (Int i, Bool keep_order=true); // get i-th  element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (                           ); // get last  element and remove it from the container

   MemcThreadSafe& setNum    (Int num); // set number of elements to 'num'                                                                              , this method may change the memory address of all elements
   MemcThreadSafe& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method may change the memory address of all elements
   Int             addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method may change the memory address of all elements

   // values
   T1(VALUE) Int             find   (C VALUE &value                       )C {SyncLocker locker(_lock); REPA(T)if(lockedElm(i)==value)return i; return -1;                                         } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool            has    (C VALUE &value                       )C {                          return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) MemcThreadSafe& add    (C VALUE &value                       )  {SyncLocker locker(_lock); lockedNew()=value; return T;                                                               } // add      'value' to container                                                                                       , this method may change the memory address of all elements
   T1(VALUE) Bool            include(C VALUE &value                       )  {SyncLocker locker(_lock); if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method may change the memory address of all elements
   T1(VALUE) Bool            exclude(C VALUE &value, Bool keep_order=false)  {SyncLocker locker(_lock); Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method may change the memory address of all elements
   T1(VALUE) Bool            toggle (C VALUE &value, Bool keep_order=false)  {SyncLocker locker(_lock); Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method may change the memory address of all elements

   T1(VALUE)   Bool                  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool                  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {                          Int i; return binarySearch(value, i, compare);                                                                   } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE*           lockedBinaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {                          Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                                    } // check if 'value' (using binary search) is present in container and return it, null on fail, container must be locked first using the 'lock' method before using this method
   T1(VALUE) C TYPE*           lockedBinaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {                    return ConstCast(T).binaryFind  (value,    compare);                                                                   } // check if 'value' (using binary search) is present in container and return it, null on fail, container must be locked first using the 'lock' method before using this method
   T1(VALUE)   MemcThreadSafe&       binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {SyncLocker locker(_lock); Int i;        binarySearch(value, i, compare); lockedNewAt(i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method may change the memory address of all elements
   T1(VALUE)   Bool                  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {SyncLocker locker(_lock); Int i; if(   !binarySearch(value, i, compare)){lockedNewAt(i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method may change the memory address of all elements
   T1(VALUE)   Bool                  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {SyncLocker locker(_lock); Int i; if(    binarySearch(value, i, compare)){remove     (i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method may change the memory address of all elements
   T1(VALUE)   Bool                  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {SyncLocker locker(_lock); Int i; if(   !binarySearch(value, i, compare)){lockedNewAt(i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method may change the memory address of all elements

   // order
   MemcThreadSafe&           sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function, this method may change the memory address of all elements
   MemcThreadSafe&   reverseOrder(                                 ); // reverse   order of elements, this method     changes the memory address of all elements
   MemcThreadSafe& randomizeOrder(                                 ); // randomize order of elements, this method may change  the memory address of all elements
   MemcThreadSafe&    rotateOrder(Int offset                       ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
   MemcThreadSafe&      swapOrder(Int i  , Int j                   ); // swap      order of 'i' and 'j' elements
   MemcThreadSafe&      moveElm  (Int elm, Int new_index           ); // move 'elm' element to new position located at 'new_index'

   // operations
   void   lock()C; //   lock this container, must be called before using methods which name starts with "locked", 'unlock' must be called after all of those operations
   void unlock()C; // unlock this container, must be called after  using methods which name starts with "locked"

   MemcThreadSafe();
};
/******************************************************************************/
inline Int Elms(C _MemcThreadSafe &memc) {return memc.elms();}
/******************************************************************************/
struct MemcThreadSafeLock // MemcThreadSafe Lock (automatically locks and unlocks the memory container at object creation and destruction)
{
   explicit MemcThreadSafeLock(_MemcThreadSafe &memc) : _memc(memc) {_memc.  lock();}
           ~MemcThreadSafeLock(                     )               {_memc.unlock();}

private:
  _MemcThreadSafe &_memc;
   NO_COPY_CONSTRUCTOR(MemcThreadSafeLock);
};
/******************************************************************************/
