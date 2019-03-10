/******************************************************************************

   Use 'Memb' for block based dynamic array container.

   'Memb' stores elements in blocks, for example:
      block 0: [ABCD]
      block 1: [EFGH]
      block 2: [IJ..]
      ..

   Creating new elements in 'Memb' container does not change the memory address of its elements.
      Only new blocks are allocated when needed.

   Removing existing elements however, will move others into different position in the block,
      thus changing their memory address.

/******************************************************************************/
T1(const_mem_addr TYPE) struct Memb : _Memb // Block Based Container
{
   // manage
   Memb& clear(); // remove all elements
   Memb& del  (); // remove all elements and free helper memory

   // get / set
   Int    elms    ()C; // number of elements
   UInt   elmSize ()C; // size   of element
   UInt blockElms ()C; // number of elements per block
   UInt   memUsage()C; // memory usage

   TYPE* addr      (Int i) ; // get    i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get    i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int i) ; // get    i-th  element, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& first     (     ) ; // get    first element
 C TYPE& first     (     )C; // get    first element
   TYPE& last      (     ) ; // get    last  element
 C TYPE& last      (     )C; // get    last  element
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method may      change the memory address of all        elements

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   Memb& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   Memb& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   Memb& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   TYPE popFirst(       Bool keep_order=true); // get first element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (Int i, Bool keep_order=true); // get i-th  element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (                           ); // get last  element and remove it from the container

   Memb& setNum    (Int num); // set number of elements to 'num'                                                                              , this method does not change the memory address of any of the elements
   Memb& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method does not change the memory address of any of the elements
   Int   addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method does not change the memory address of any of the elements

   // values
   T1(VALUE) Int   find   (C VALUE &value                       )C {REPA(T)if(T[i]==value)return i; return -1;                                                 } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool  has    (C VALUE &value                       )C {return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) Memb& add    (C VALUE &value                       )  {New()=value; return T;                                                                     } // add      'value' to container                                                                                       , this method does not change the memory address of any of the elements
   T1(VALUE) Bool  include(C VALUE &value                       )  {if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method does not change the memory address of any of the elements
   T1(VALUE) Bool  exclude(C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method may      change the memory address of some       elements
   T1(VALUE) Bool  toggle (C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method may      change the memory address of some       elements

   T1(VALUE)   Bool  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                              } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                               } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                             } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   Memb& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt (i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method may change the memory address of some elements
   T1(VALUE)   Bool  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method may change the memory address of some elements
   T1(VALUE)   Bool  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){remove(i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method may change the memory address of some elements
   T1(VALUE)   Bool  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method may change the memory address of some elements

   // order
   Memb&         sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function, this method may change the memory address of all elements
   Memb& reverseOrder(                                 ); // swap order of elements, this method may change the memory address of all elements
   Memb&    swapOrder(Int i  , Int j                   ); // swap order of 'i' and 'j' valid elements
   Memb&      moveElm(Int elm, Int new_index           ); // move 'elm' element to new position located at 'new_index'

   // misc
                      Memb& operator=(C Mems  <TYPE      >  &src); // copy elements using assignment operator
                      Memb& operator=(C Memc  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memb& operator=(C Memt  <TYPE, size>  &src); // copy elements using assignment operator
                      Memb& operator=(C Memb  <TYPE      >  &src); // copy elements using assignment operator
                      Memb& operator=(C Memx  <TYPE      >  &src); // copy elements using assignment operator
                      Memb& operator=(C Meml  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memb& operator=(C MemPtr<TYPE, size>  &src); // copy elements using assignment operator
                      Memb& operator=(  Memb  <TYPE      > &&src); // copy elements using assignment operator

   T1(EXTENDED) Memb& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memb<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memb<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
#if EE_PRIVATE
   void  copyTo  (  TYPE *dest)C {_Memb::copyTo  (dest);          } // copy raw memory of all elements to   'dest'
   Memb& copyFrom(C TYPE *src )  {_Memb::copyFrom(src ); return T;} // copy raw memory of all elements from 'src '
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail
   Bool loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail

   explicit Memb(Int block_elms=32); // 'block_elms'=number of elements per block
            Memb(C Memb  &src     );
            Memb(  Memb &&src     );
};
/******************************************************************************/
T1(TYPE) struct MembAbstract : _Memb // Block Based Container which allows storage of abstract classes, 'replaceClass' should be called before creating new elements in it
{
   // manage
   MembAbstract& clear(); // remove all elements
   MembAbstract& del  (); // remove all elements and free helper memory

   // get / set
   Int    elms   ()C; // number of elements
   UInt   elmSize()C; // size   of element
   UInt blockElms()C; // number of elements per block

   TYPE* addr      (Int i) ; // get    i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get    i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int i) ; // get    i-th  element, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& first     (     ) ; // get    first element
 C TYPE& first     (     )C; // get    first element
   TYPE& last      (     ) ; // get    last  element
 C TYPE& last      (     )C; // get    last  element
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method may      change the memory address of all        elements

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   MembAbstract& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   MembAbstract& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   MembAbstract& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   MembAbstract& setNum    (Int num); // set number of elements to 'num'                                                                              , this method does not change the memory address of any of the elements
   MembAbstract& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method does not change the memory address of any of the elements
   Int           addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method does not change the memory address of any of the elements

   T1(EXTENDED) MembAbstract& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memb<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memb<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE

   explicit MembAbstract(Int block_elms=32); // 'block_elms'=number of elements per block
};
/******************************************************************************/
T1(TYPE) struct MembConst : Memb<TYPE> // Block Based Container which allows modifying elements even when being 'const'
{
   TYPE* addr      (Int i)C; // get i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i)C; // get i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& first     (     )C; // get first element
   TYPE& last      (     )C; // get last  element

   T1(BASE) operator   MembConst<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C MembConst<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE

   explicit MembConst(Int block_elms=32) : Memb<TYPE>(block_elms) {} // 'block_elms'=number of elements per block
};
/******************************************************************************/
inline Int Elms(C _Memb &memb) {return memb.elms();}
/******************************************************************************/
