/******************************************************************************

   Use 'Memt' for temporary memory based dynamic array container.

   'Memt' stores elements in continuous memory, for example:
      [ABCDE...]

   'Memt' uses a predefined chunk of memory which is kept on the stack.
      Initially this chunk of memory is used for creating new elements, thanks to which, memory allocation is not needed.
      If the number of elements becomes bigger than what the chunk can store,
      a memory allocation is made, and elements are moved to a new area.

   'Memt' initially (for small number of elements) does not require memory allocation.

   'Memt' container reserves some extra memory for adding new elements.
      If creating a new element when there is no extra memory available,
      the container will reallocate the whole array into a new bigger one,
      thus changing the address of all elements.

   'Memt' is recommended to be used in performance critical operations.

   !! Warning: because 'Memt' uses decent amount of stack memory (by default set to 64 KB for each 'Memt')
      the application may crash if too many stack 'Memt' objects are used at once on one thread !!
      This does not affect 'Memt' objects allocated globally on the heap, or dynamically using memory containers.
      Typically thread stacks have 1 MB of memory.

/******************************************************************************/
template<const_mem_addr typename TYPE, Int size> struct Memt // Temporary Memory Based Container, 'TYPE'=type of elements to be stored in this container, 'size'=memory size (in bytes) that can be used for storing the elements without having to allocate any dynamic memory
{
   // manage
   Memt& clear(); // remove all elements and free helper memory
   Memt& del  (); // remove all elements and free helper memory

   // get / set
   Int  elms    ()C; // number of elements
   UInt elmSize ()C; // size   of element
   UInt memUsage()C; // memory usage
#if EE_PRIVATE
   UInt maxElms ()C {return _max_elms;}

   TYPE* dataNull()  {return elms() ? data() : null;} // get pointer to the start of the elements and null if there are no elements
 C TYPE* dataNull()C {return elms() ? data() : null;} // get pointer to the start of the elements and null if there are no elements
#endif

   TYPE* data      (     ) ; // get    pointer to the start of the elements
 C TYPE* data      (     )C; // get    pointer to the start of the elements
   TYPE* addr      (Int i) ; // get    i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get    i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int i) ; // get    i-th  element, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& first     (     ) ; // get    first element
 C TYPE& first     (     )C; // get    first element
   TYPE& last      (     ) ; // get    last  element
 C TYPE& last      (     )C; // get    last  element
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method changes the memory address of all elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method changes the memory address of all elements

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   Memt& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   Memt& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   Memt& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   TYPE popFirst(       Bool keep_order=true); // get first element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (Int i, Bool keep_order=true); // get i-th  element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (                           ); // get last  element and remove it from the container

   Memt& setNum    (Int num); // set number of elements to 'num'                                                                              , this method changes the memory address of all elements
   Memt& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method changes the memory address of all elements
   Int   addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method changes the memory address of all elements
   Memt& reserve   (Int num); // pre-allocate memory for 'num' elements

   // values
   T1(VALUE) Int   find   (C VALUE &value                       )C {REPA(T)if(T[i]==value)return i; return -1;                                                 } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool  has    (C VALUE &value                       )C {return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) Memt& add    (C VALUE &value                       )  {New()=value; return T;                                                                     } // add      'value' to container                                                                                       , this method changes the memory address of all elements
   T1(VALUE) Bool  include(C VALUE &value                       )  {if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE) Bool  exclude(C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE) Bool  toggle (C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   T1(VALUE)   Bool  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                              } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                               } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                             } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   Memt& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt (i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){remove(i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   // order
   Memt&           sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function
   Memt&   reverseOrder(                                 ); // reverse   order of elements
   Memt& randomizeOrder(                                 ); // randomize order of elements
   Memt&    rotateOrder(Int offset                       ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
   Memt&      swapOrder(Int i  , Int j                   ); // swap      order of 'i' and 'j' elements
   Memt&      moveElm  (Int elm, Int new_index           ); // move 'elm' element to new position located at 'new_index'

   // misc
                          Memt& operator=(C Mems  <TYPE          > &src); // copy elements using assignment operator
                          Memt& operator=(C Memc  <TYPE          > &src); // copy elements using assignment operator
                          Memt& operator=(C Memt  <TYPE,     size> &src); // copy elements using assignment operator (this must be specified even though method below should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)
   template<Int src_size> Memt& operator=(C Memt  <TYPE, src_size> &src); // copy elements using assignment operator (this will allow copying from 'Memt' with other sizes)
                          Memt& operator=(C Memb  <TYPE          > &src); // copy elements using assignment operator
                          Memt& operator=(C Memx  <TYPE          > &src); // copy elements using assignment operator
                          Memt& operator=(C Meml  <TYPE          > &src); // copy elements using assignment operator
   template<Int src_size> Memt& operator=(C MemPtr<TYPE, src_size> &src); // copy elements using assignment operator

#if EE_PRIVATE
   void  copyTo  (  TYPE *dest)C {CopyN(dest  , data(), elms());          } // copy raw memory of all elements to   'dest'
   Memt& copyFrom(C TYPE *src )  {CopyN(data(), src   , elms()); return T;} // copy raw memory of all elements from 'src '
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool saveRaw    (File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail
   Bool loadRaw    (File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail
   Bool saveRawData(File &f)C; // save raw memory of elements (                     elements raw memory), false on fail
   Bool loadRawData(File &f) ; // load raw memory of elements (                     elements raw memory), false on fail

#if EE_PRIVATE
   Bool loadRawDataFast(File &f); // load raw memory of elements (elements raw memory), without zeroing on fail, false on fail

   Bool _saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
   Bool _loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
#endif

  ~Memt(           );
   Memt(           );
   Memt(C Memt &src);

private:
   TYPE *_data;
   Int   _elms, _max_elms;
   Byte  _temp[size];
};
/******************************************************************************/
template<const_mem_addr typename TYPE, Int Memt_elms> struct MemtN : Memt<TYPE, SIZE(TYPE)*Memt_elms> // Temporary Memory Based Container, 'TYPE'=type of elements to be stored in this container, 'Memt_elms'=number of elements that can be stored without having to allocate any dynamic memory
{
                          MemtN& operator=(C Mems  <TYPE           > &src); // copy elements using assignment operator
                          MemtN& operator=(C Memc  <TYPE           > &src); // copy elements using assignment operator
   template<Int src_size> MemtN& operator=(C Memt  <TYPE,  src_size> &src); // copy elements using assignment operator (this will allow copying from 'Memt' with other sizes)
                          MemtN& operator=(C MemtN <TYPE, Memt_elms> &src); // copy elements using assignment operator (this must be specified even though method above should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)
                          MemtN& operator=(C Memb  <TYPE           > &src); // copy elements using assignment operator
                          MemtN& operator=(C Memx  <TYPE           > &src); // copy elements using assignment operator
                          MemtN& operator=(C Meml  <TYPE           > &src); // copy elements using assignment operator
   template<Int src_size> MemtN& operator=(C MemPtr<TYPE,  src_size> &src); // copy elements using assignment operator
};
/******************************************************************************/
template<typename TYPE, Int size>   inline Int Elms(C Memt<TYPE, size> &memt) {return memt.elms();}
/******************************************************************************/
#if EE_PRIVATE
inline UIntPtr MemtMemUsage(UIntPtr mem, UInt stack=65536) {return mem>stack ? mem : 0;}
#endif
/******************************************************************************/
