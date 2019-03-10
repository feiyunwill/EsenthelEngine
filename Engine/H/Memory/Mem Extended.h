/******************************************************************************

   Use 'Memx' for extended block based dynamic array container.

   'Memx' preserves elements memory address in every operation.

   'Memx' stores elements in blocks, however unlike 'Memb' it allows to have "holes" between elements, for example:
      block 0: [A.CD]
      block 1: [.F.H]
      block 2: [IJ..]
      ..

   Creating new elements in 'Memx' container does not change the memory address of its elements.
      Only new blocks are allocated when needed.

   Removing existing elements does not change the memory address of its elements.
      Removed elements are marked as invalid ("holes").

/******************************************************************************/
T1(TYPE) struct Memx : _Memx // Block Based Extended Container
{
   // manage
   Memx& clear(); // remove all elements
   Memx& del  (); // remove all elements and free helper memory

   // get / set
    Int   absElms ()C; // number of absolute elements
    Int validElms ()C; // number of valid    elements
    Int      elms ()C; // number of valid    elements
   UInt   elmSize ()C; // size   of          element
   UInt   memUsage()C; // memory usage

   TYPE&   absElm  (Int i) ; // get i-th  absolute element
 C TYPE&   absElm  (Int i)C; // get i-th  absolute element
   TYPE& validElm  (Int i) ; // get i-th  valid    element
 C TYPE& validElm  (Int i)C; // get i-th  valid    element
   TYPE* addr      (Int i) ; // get i-th  valid    element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get i-th  valid    element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get i-th  valid    element
 C TYPE& operator[](Int i)C; // get i-th  valid    element
   TYPE& first     (     ) ; // get first valid    element
 C TYPE& first     (     )C; // get first valid    element
   TYPE& last      (     ) ; // get last  valid    element
 C TYPE& last      (     )C; // get last  valid    element
   TYPE& New       (     ) ; // create    new      element, this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int i) ; // create    new      element at i-th valid index, all old element valid indexes starting from i-th position will be moved to the right, this method does not change the memory address of any of the elements

   Int validToAbsIndex(  Int valid)C; // convert valid to absolute index, -1 on fail
   Int absToValidIndex(  Int   abs)C; // convert absolute to valid index, -1 on fail
   Int validIndex     (C TYPE *elm)C; // get valid    index of element in container, -1 on fail, testing is done by comparing elements memory address only
   Int   absIndex     (C TYPE *elm)C; // get absolute index of element in container, -1 on fail, testing is done by comparing elements memory address only
   Bool  contains     (C TYPE *elm)C; // check if memory container actually contains element   , testing is done by comparing elements memory address only

   // remove
   Memx& removeAbs  (  Int   i  , Bool keep_order=false); // remove i-th absolute element               , if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   Memx& removeValid(  Int   i  , Bool keep_order=false); // remove i-th valid    element               , if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   Memx& removeData (C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   Memx& removeLast (                                  ); // remove last valid    element

   T1(VALUE)   Bool  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                                        } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                                         } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                                       } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   Memx& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt      (i)=value;                                     return     T;} // add      'value' (using binary search)                                                                                                    , this method does not change the memory address of any of the elements
   T1(VALUE)   Bool  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt      (i)=value; return true;}                       return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method does not change the memory address of any of the elements
   T1(VALUE)   Bool  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){removeValid(i, true); return true;}                       return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method does not change the memory address of any of the elements
   T1(VALUE)   Bool  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt      (i)=value; return true;} removeValid(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method does not change the memory address of any of the elements

   // order
   Memx&         sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function (sorting   modifies only "valid indexes"), this method does not change the memory address of any of the elements
   Memx& reverseOrder(                                 ); // reverse order of elements                    (reversing modifies only "valid indexes"), this method does not change the memory address of any of the elements
   Memx&    swapOrder(Int i  , Int j                   ); // swap    order of 'i' and 'j' valid elements  (swapping  modifies only "valid indexes"), this method does not change the memory address of any of the elements
   Memx& moveElm     (Int elm, Int new_index           ); // move 'elm' valid element to 'new_index'      (moving    modifies only "valid indexes"), this method does not change the memory address of any of the elements
   Memx& moveToStart (Int elm                          ); // move 'elm' valid element to the start        (moving    modifies only "valid indexes"), this method does not change the memory address of any of the elements
   Memx& moveToEnd   (Int elm                          ); // move 'elm' valid element to the end          (moving    modifies only "valid indexes"), this method does not change the memory address of any of the elements

   // misc
                      Memx& operator=(C Mems  <TYPE      >  &src); // copy elements using assignment operator
                      Memx& operator=(C Memc  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memx& operator=(C Memt  <TYPE, size>  &src); // copy elements using assignment operator
                      Memx& operator=(C Memb  <TYPE      >  &src); // copy elements using assignment operator
                      Memx& operator=(C Memx  <TYPE      >  &src); // copy elements using assignment operator
                      Memx& operator=(C Meml  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memx& operator=(C MemPtr<TYPE, size>  &src); // copy elements using assignment operator
                      Memx& operator=(  Memx  <TYPE      > &&src); // copy elements using assignment operator

   T1(EXTENDED) Memx& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memx<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memx<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   explicit Memx(Int block_elms=32); // 'block_elms'=number of elements per block
            Memx(C Memx  &src     );
            Memx(  Memx &&src     );
};
/******************************************************************************/
T1(TYPE) struct MemxAbstract : _Memx // Block Based Extended Container which allows storage of abstract classes, 'replaceClass' should be called before creating new elements in it
{
   // manage
   MemxAbstract& clear(); // remove all elements
   MemxAbstract& del  (); // remove all elements and free helper memory

   // get / set
    Int   absElms ()C; // number of absolute elements
    Int validElms ()C; // number of valid    elements
    Int      elms ()C; // number of valid    elements
   UInt   elmSize ()C; // size   of          element
   UInt   memUsage()C; // memory usage

   TYPE&   absElm  (Int i) ; // get i-th  absolute element
 C TYPE&   absElm  (Int i)C; // get i-th  absolute element
   TYPE& validElm  (Int i) ; // get i-th  valid    element
 C TYPE& validElm  (Int i)C; // get i-th  valid    element
   TYPE* addr      (Int i) ; // get i-th  valid    element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get i-th  valid    element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get i-th  valid    element
 C TYPE& operator[](Int i)C; // get i-th  valid    element
   TYPE& first     (     ) ; // get first valid    element
 C TYPE& first     (     )C; // get first valid    element
   TYPE& last      (     ) ; // get last  valid    element
 C TYPE& last      (     )C; // get last  valid    element
   TYPE& New       (     ) ; // create    new      element, this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int i) ; // create    new      element at i-th valid index, all old element valid indexes starting from i-th position will be moved to the right, this method does not change the memory address of any of the elements

   Int validToAbsIndex(  Int valid)C; // convert valid to absolute index, -1 on fail
   Int absToValidIndex(  Int   abs)C; // convert absolute to valid index, -1 on fail
   Int validIndex     (C TYPE *elm)C; // get valid    index of element in container, -1 on fail, testing is done by comparing elements memory address only
   Int   absIndex     (C TYPE *elm)C; // get absolute index of element in container, -1 on fail, testing is done by comparing elements memory address only
   Bool  contains     (C TYPE *elm)C; // check if memory container actually contains element   , testing is done by comparing elements memory address only

   // remove
   MemxAbstract& removeAbs  (  Int   i  , Bool keep_order=false); // remove i-th absolute element               , if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   MemxAbstract& removeValid(  Int   i  , Bool keep_order=false); // remove i-th valid    element               , if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   MemxAbstract& removeData (C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then "valid index" of last valid element is changed to "valid index" of specified element, if 'keep_order'=true then all "valid indexes" are changed (keeping order), in both cases memory addressess of all elements are preserved (only order of "valid indexes" is handled differently), this method does not change the memory address of any of the elements
   MemxAbstract& removeLast (                                  ); // remove last valid    element

   T1(EXTENDED) MemxAbstract& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memx<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memx<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE

   explicit MemxAbstract(Int block_elms=32); // 'block_elms'=number of elements per block
};
/******************************************************************************/
inline Int Elms(C _Memx &memx) {return memx.elms();}
/******************************************************************************/
