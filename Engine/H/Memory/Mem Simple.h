/******************************************************************************

   Use 'Mems' for simple continuous memory based dynamic array container.

   'Mems' stores elements in continuous memory, for example:
      [ABCDE]

   'Mems' memory container works very similar to 'Memc' except:
      -'Mems' uses less memory than 'Memc'
      -'Mems' is slower for adding/removing elements than 'Memc'

   It is perfectly suited for data initialized only once.

   'Mems' will allocate only as much memory as needed, this means
      that when creating new elements, the whole array needs to be
      reallocated, thus changing the memory address of all elements.

/******************************************************************************/
T1(const_mem_addr TYPE) struct Mems // Simple Continuous Memory Based Container
{
   // manage
   Mems& clear(); // remove all elements and free helper memory
   Mems& del  (); // remove all elements and free helper memory

   // get / set
   Int  elms    ()C; // number of elements
   UInt elmSize ()C; // size   of element
   UInt memUsage()C; // memory usage

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
   Mems& removeLast(                                  ); // remove last element                        , all remaining elements are kept in the same order, this method changes the memory address of all elements
   Mems& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , all remaining elements are kept in the same order, this method changes the memory address of all elements, 'keep_order'=this parameter is ignored for 'Mems' because it always keeps order (it is kept here only for compatibility with other memory containers)
   Mems& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, all remaining elements are kept in the same order, this method changes the memory address of all elements, 'keep_order'=this parameter is ignored for 'Mems' because it always keeps order (it is kept here only for compatibility with other memory containers)

   TYPE popFirst(       Bool keep_order=true); // get first element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (Int i, Bool keep_order=true); // get i-th  element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (                           ); // get last  element and remove it from the container

   Mems& setNum    (Int num); // set number of elements to 'num'                                                                              , this method changes the memory address of all elements
   Mems& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method changes the memory address of all elements
   Int   addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method changes the memory address of all elements

   // values
   T1(VALUE) Int   find   (C VALUE &value)C {REPA(T)if(T[i]==value)return i; return -1;                                       } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool  has    (C VALUE &value)C {return find(value)>=0;                                                           } // check if 'value' is present in container
   T1(VALUE) Mems& add    (C VALUE &value)  {New()=value; return T;                                                           } // add      'value' to container                                                                                       , this method changes the memory address of all elements
   T1(VALUE) Bool  include(C VALUE &value)  {if(!has(value)){add(value); return true;} return false;                          } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE) Bool  exclude(C VALUE &value)  {Int i=find(value); if(i>=0){remove(i);   return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE) Bool  toggle (C VALUE &value)  {Int i=find(value); if(i>=0){remove(i);   return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   T1(VALUE)   Bool  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                        } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                         } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                       } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   Mems& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt (i)=value;                          return     T;} // add      'value' (using binary search)                                                                                                    , this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;}            return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){remove(i)      ; return true;}            return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE)   Bool  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   // order
   Mems&           sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function
   Mems&   reverseOrder(                                 ); // reverse   order of elements
   Mems& randomizeOrder(                                 ); // randomize order of elements
   Mems&    rotateOrder(Int offset                       ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
   Mems&      swapOrder(Int i  , Int j                   ); // swap      order of 'i' and 'j' elements
   Mems&      moveElm  (Int elm, Int new_index           ); // move 'elm' element to new position located at 'new_index'

   // misc
                      Mems& operator=(C Mems  <TYPE      >  &src); // copy elements using assignment operator
                      Mems& operator=(C Memc  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Mems& operator=(C Memt  <TYPE, size>  &src); // copy elements using assignment operator
                      Mems& operator=(C Memb  <TYPE      >  &src); // copy elements using assignment operator
                      Mems& operator=(C Memx  <TYPE      >  &src); // copy elements using assignment operator
                      Mems& operator=(C Meml  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Mems& operator=(C MemPtr<TYPE, size>  &src); // copy elements using assignment operator
                      Mems& operator=(  Mems  <TYPE      > &&src); // copy elements using assignment operator
#if EE_PRIVATE
   void  copyTo  (  TYPE *dest)C; // copy raw memory of all elements to   'dest'
   Mems& copyFrom(C TYPE *src ) ; // copy raw memory of all elements from 'src'
   void   setFrom(  TYPE* &data, Int elms); // this takes ownership of 'data' and sets that pointer to null
   void   setTemp(  TYPE*  data, Int elms); // this is not safe !!
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   T1(USER) Bool save(File &f, C USER &user)C; // save elements with their own 'save' method and 'user' parameter, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   T1(USER) Bool load(File &f, C USER &user) ; // load elements with their own 'load' method and 'user' parameter, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   T2(USER, USER1) Bool save(File &f, C USER &user, C USER1 &user1)C; // save elements with their own 'save' method and 'user, user1' parameter, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   T2(USER, USER1) Bool load(File &f, C USER &user, C USER1 &user1) ; // load elements with their own 'load' method and 'user, user1' parameter, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool saveRaw    (File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail
   Bool loadRaw    (File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail
   Bool saveRawData(File &f)C; // save raw memory of elements (                     elements raw memory), false on fail
   Bool loadRawData(File &f) ; // load raw memory of elements (                     elements raw memory), false on fail

#if EE_PRIVATE
   Bool _saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
   Bool _loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
   Bool _save   (File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail, deprecated - do not use
   Bool _load   (File &f) ; // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail, deprecated - do not use
#endif

  ~Mems(            );
   Mems(            );
   Mems(C Mems  &src);
   Mems(  Mems &&src);

private:
   TYPE *_data;
   Int   _elms;
};
/******************************************************************************/
#if EE_PRIVATE
T1(const_mem_addr TYPE) STRUCT(FixedMems , Mems<TYPE>) // Unresizable Mems container
#else
T1(const_mem_addr TYPE) STRUCT_PRIVATE(FixedMems , Mems<TYPE>) // Unresizable Mems container
#endif
//{
   // get / set
   Int  elms    ()C {return super::elms    ();} // number of elements
   UInt elmSize ()C {return super::elmSize ();} // size   of element
   UInt memUsage()C {return super::memUsage();} // memory usage

   TYPE* data      (     )  {return super::data      ( );} // get pointer to the start of the elements
 C TYPE* data      (     )C {return super::data      ( );} // get pointer to the start of the elements
   TYPE* addr      (Int i)  {return super::addr      (i);} // get i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C {return super::addr      (i);} // get i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i)  {return super::operator[](i);} // get i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C {return super::operator[](i);} // get i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& first     (     )  {return super::first     ( );} // get first element
 C TYPE& first     (     )C {return super::first     ( );} // get first element
   TYPE& last      (     )  {return super::last      ( );} // get last  element
 C TYPE& last      (     )C {return super::last      ( );} // get last  element

   Int  index   (C TYPE *elm)C {return super::index   (elm);} // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C {return super::contains(elm);} // check if memory container actually contains element, testing is done by comparing elements memory address only
};
/******************************************************************************/
T1(TYPE) Int Elms(C      Mems<TYPE> &mems) {return mems.elms();}
T1(TYPE) Int Elms(C FixedMems<TYPE> &mems) {return mems.elms();}
/******************************************************************************/
