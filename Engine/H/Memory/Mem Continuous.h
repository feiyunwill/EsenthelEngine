/******************************************************************************

   Use 'Memc' for continuous memory based dynamic array container.

   'Memc' stores elements in continuous memory, for example:
      [ABCDE...]

   'Memc' container reserves some extra memory for adding new elements.
      If creating a new element when there is no extra memory available,
      the container will reallocate the whole array into a new bigger one,
      thus changing the address of all elements.

/******************************************************************************/
T1(const_mem_addr TYPE) struct Memc : _Memc // Continuous Memory Based Container
{
   // manage
   Memc& clear(); // remove all elements
   Memc& del  (); // remove all elements and free helper memory

   // get / set
   Int  elms    ()C; // number of elements
   UInt elmSize ()C; // size   of element
   UInt memUsage()C; // memory usage

   TYPE* data      (     ) ; // get    pointer to the start of the elements
 C TYPE* data      (     )C; // get    pointer to the start of the elements
   TYPE* addr      (Int i) ; // get    i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get    i-th  element address, null is returned if index is out of range
   TYPE* addrFirst (     ) ; // get    first element address, null is returned if element doesn't exist
 C TYPE* addrFirst (     )C; // get    first element address, null is returned if element doesn't exist
   TYPE* addrLast  (     ) ; // get    last  element address, null is returned if element doesn't exist
 C TYPE* addrLast  (     )C; // get    last  element address, null is returned if element doesn't exist
   TYPE& operator[](Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int i) ; // get    i-th  element, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& first     (     ) ; // get    first element
 C TYPE& first     (     )C; // get    first element
   TYPE& last      (     ) ; // get    last  element
 C TYPE& last      (     )C; // get    last  element
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method may change the memory address of all elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method may change the memory address of all elements

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   Memc& removeLast(                                         ); // remove last element                                                                                                                                                                         , this method does not change the memory address of any of the remaining elements
   Memc& remove    (  Int   i  ,        Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element  to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   Memc& removeNum (  Int   i  , Int n, Bool keep_order=false); // remove 'n' elements starting from i-th     , if 'keep_order'=false then moves the last elements to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   Memc& removeData(C TYPE *elm,        Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element  to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   TYPE popFirst(       Bool keep_order=true); // get first element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (Int i, Bool keep_order=true); // get i-th  element and remove it from the container, if 'keep_order'=true then moves all elements after i-th to the left (keeping order)
   TYPE pop     (                           ); // get last  element and remove it from the container

   Memc& setNum    (Int num); // set number of elements to 'num'                                                                              , this method may change the memory address of all elements
   Memc& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method may change the memory address of all elements
   Int   addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method may change the memory address of all elements

   // values
   T1(VALUE) Int   find   (C VALUE &value                       )C {REPA(T)if(T[i]==value)return i; return -1;                                                 } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool  has    (C VALUE &value                       )C {return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) Memc& add    (C VALUE &value                       )  {New()=value; return T;                                                                     } // add      'value' to container                                                                                       , this method may change the memory address of all elements
   T1(VALUE) Bool  include(C VALUE &value                       )  {if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method may change the memory address of all elements
   T1(VALUE) Bool  exclude(C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method may change the memory address of all elements
   T1(VALUE) Bool  toggle (C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method may change the memory address of all elements

   T1(VALUE)   Bool  binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                              } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                               } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE* binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                             } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   Memc& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt (i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method may change the memory address of all elements
   T1(VALUE)   Bool  binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method may change the memory address of all elements
   T1(VALUE)   Bool  binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){remove(i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method may change the memory address of all elements
   T1(VALUE)   Bool  binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method may change the memory address of all elements

   // order
   Memc&           sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function, this method may change the memory address of all elements
   Memc&   reverseOrder(                                 ); // reverse   order of elements, this method     changes the memory address of all elements
   Memc& randomizeOrder(                                 ); // randomize order of elements, this method may change  the memory address of all elements
   Memc&    rotateOrder(Int offset                       ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
   Memc&      swapOrder(Int i  , Int j                   ); // swap      order of 'i' and 'j' elements
   Memc&      moveElm  (Int elm, Int new_index           ); // move 'elm' element to new position located at 'new_index'

   // misc
                      Memc& operator=(C Mems  <TYPE      >  &src); // copy elements using assignment operator
                      Memc& operator=(C Memc  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memc& operator=(C Memt  <TYPE, size>  &src); // copy elements using assignment operator
                      Memc& operator=(C Memb  <TYPE      >  &src); // copy elements using assignment operator
                      Memc& operator=(C Memx  <TYPE      >  &src); // copy elements using assignment operator
                      Memc& operator=(C Meml  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Memc& operator=(C MemPtr<TYPE, size>  &src); // copy elements using assignment operator
                      Memc& operator=(  Memc  <TYPE      > &&src); // copy elements using assignment operator

   T1(EXTENDED) Memc& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memc<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memc<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
#if EE_PRIVATE
   void  copyTo  (  TYPE *dest)C {_Memc::copyTo  (dest);          } // copy raw memory of all elements to   'dest'
   Memc& copyFrom(C TYPE *src )  {_Memc::copyFrom(src ); return T;} // copy raw memory of all elements from 'src '
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   T1(USER) Bool save(File &f, C USER &user)C; // save elements with their own 'save' method and 'user' parameter, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   T1(USER) Bool load(File &f, C USER &user) ; // load elements with their own 'load' method and 'user' parameter, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   T2(USER, USER1) Bool save(File &f, C USER &user, C USER1 &user1)C; // save elements with their own 'save' method and 'user, user1' parameter, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   T2(USER, USER1) Bool load(File &f, C USER &user, C USER1 &user1) ; // load elements with their own 'load' method and 'user, user1' parameter, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail
   Bool loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail

#if EE_PRIVATE
   Bool _saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
   Bool _loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail, deprecated - do not use
   Bool _save   (File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail, deprecated - do not use
   Bool _load   (File &f) ; // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail, deprecated - do not use
#endif

   Memc(            );
   Memc(C Memc  &src);
   Memc(  Memc &&src);
};
/******************************************************************************/
T1(TYPE) struct MemcAbstract : _Memc // Continuous Memory Based Container which allows storage of abstract classes, 'replaceClass' should be called before creating new elements in it
{
   // manage
   MemcAbstract& clear(); // remove all elements
   MemcAbstract& del  (); // remove all elements and free helper memory

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
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method may      change the memory address of all        elements

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   MemcAbstract& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   MemcAbstract& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   MemcAbstract& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   MemcAbstract& setNum    (Int num); // set number of elements to 'num'                                                                              , this method does not change the memory address of any of the elements
   MemcAbstract& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method does not change the memory address of any of the elements
   Int           addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method does not change the memory address of any of the elements

   T1(EXTENDED) MemcAbstract& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   T1(BASE) operator   Memc<BASE>&() ; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
   T1(BASE) operator C Memc<BASE>&()C; // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE

   MemcAbstract();
};
/******************************************************************************/
inline Int Elms(C _Memc &memc) {return memc.elms();}
/******************************************************************************/
#if EE_PRIVATE
T2(A, B) struct std__pair
{
   A first;
   B second;

   std__pair() {}
   std__pair(C A &a, C B &b) : first(a), second(b) {}
};
T1(TYPE) STRUCT_PRIVATE(std__unique_ptr , Mems<TYPE>)
//{
   TYPE& operator[](Int i)  {return super::operator[](i);}
 C TYPE& operator[](Int i)C {return super::operator[](i);}

   Bool  operator()(            )C {return super::elms()!=0;}
   Bool  operator! (            )C {return super::elms()==0;}
   Bool  operator==(C TYPE *data)C {return super::data()==data;}
   Bool  operator!=(C TYPE *data)C {return super::data()!=data;}
   void  reset     (Int     elms)  {       super::setNum(elms);}
   TYPE* get       (            )  {return super::data();}

   std__unique_ptr(        ) {}
   std__unique_ptr(Int elms) {reset(elms);}
};
T1(TYPE) STRUCT_PRIVATE(std__vector , Memc<TYPE>)
//{
   TYPE& operator[](Int i)  {return super::operator[](i);}
 C TYPE& operator[](Int i)C {return super::operator[](i);}

   Bool empty()C {return !super::elms();}
   Int  size ()C {return  super::elms();}

   TYPE* begin() {return super::data();}
   TYPE* end  () {return super::data()+super::elms();}

 C TYPE* cbegin()C {return super::data();}
 C TYPE* cend  ()C {return super::data()+super::elms();}

   void        clear(         ) {super::clear();}
   void emplace_back(C TYPE &t) {super::add(t);}
   void    push_back(C TYPE &t) {super::add(t);}
   void     pop_back(         ) {super::removeLast();}
   TYPE&        back(         ) {return super::last();}
   void      resize (Int elms ) {super::setNumZero(elms);} // yes, zero is required !!
   void      reserve(Int elms ) {super::reserve(elms);}

   std__vector() {}
   std__vector(Int elms                 ) {resize(elms);}
   std__vector(Int elms, C TYPE &def_val) {resize(elms); REPAO(T)=def_val;}
};
T1(TYPE) inline Int Elms(C std__vector<TYPE> &vector) {return vector.size();}
#endif
/******************************************************************************/
