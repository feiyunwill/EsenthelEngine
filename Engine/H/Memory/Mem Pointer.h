/******************************************************************************

   Use 'MemPtr' to operate on any kind of memory container.

   'MemPtr' is a pointer to one of all supported memory container types:
      'Mems', 'Memc', 'Memt', 'Memb', 'Memx', 'Meml' or regular continuous memory.

   'MemPtr' allows to write one function which can operate on all memory container types.

      For example, instead of writing multiple functions which accept different containers:
         Int Sum(C Memc<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}
         Int Sum(C Memb<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}
         ..

      You can write just one function:
         Int Sum(C MemPtr<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}

         This function will accept all memory container types, so you can do the following:
            Memc<Int> memc_values; Sum(memc_values);
            Memb<Int> memb_values; Sum(memb_values);
            ..

/******************************************************************************/
template<typename TYPE, Int Memt_size> struct MemPtr // Memory Container Pointer, 'Memt_size'=size of the 'Memt' helper stack memory
{
   // manage
   MemPtr& clear(); // remove all elements
   MemPtr& del  (); // remove all elements and free helper memory

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

   operator   Bool()C; // if points to something (not null)
   Bool  resizable()C; // if supports adding/removing elements
   Bool continuous()C; // if elements are stored in continuous memory

   // remove
   MemPtr& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   MemPtr& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   MemPtr& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   MemPtr& setNum    (Int num); // set number of elements to 'num'                                                                              , this method changes the memory address of all elements
   MemPtr& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method changes the memory address of all elements
   Int     addNum    (Int num); // add 'num' elements, return index of first added element                                                      , this method changes the memory address of all elements

   // values
   T1(VALUE) Int     find   (C VALUE &value                       )C {REPA(T)if(T[i]==value)return i; return -1;                                                 } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool    has    (C VALUE &value                       )C {return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) MemPtr& add    (C VALUE &value                       )  {New()=value; return T;                                                                     } // add      'value' to container                                                                                       , this method changes the memory address of all elements
   T1(VALUE) Bool    include(C VALUE &value                       )  {if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE) Bool    exclude(C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE) Bool    toggle (C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   T1(VALUE)   Bool    binarySearch (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool    binaryHas    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);                                                              } // check if 'value' (using binary search) is present in container
   T1(VALUE)   TYPE*   binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; return binarySearch(value, i, compare) ? &T[i] : null;                                               } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE*   binaryFind   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {return ConstCast(T).binaryFind(value, compare);                                                             } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   MemPtr& binaryAdd    (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;        binarySearch(value, i, compare); NewAt (i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryInclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryExclude(C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(    binarySearch(value, i, compare)){remove(i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryToggle (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(   !binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   // order
   MemPtr&         sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function
   MemPtr& reverseOrder(                                 ); // reverse order of elements
   MemPtr&    swapOrder(Int i, Int j                     ); // swap order of 'i' and 'j' valid elements

   // misc
                          MemPtr& operator=(C TYPE                    &src           ); // copy elements using assignment operator
   template<Int src_elms> MemPtr& operator=(C TYPE                   (&src)[src_elms]); // copy elements using assignment operator
                          MemPtr& operator=(C Mems  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C Memc  <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtr& operator=(C Memt  <TYPE,  src_size> &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C Memb  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C Memx  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C Meml  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C MemPtr<TYPE, Memt_size> &src           ); // copy elements using assignment operator (this must be specified even though method below should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)
   template<Int src_size> MemPtr& operator=(C MemPtr<TYPE,  src_size> &src           ); // copy elements using assignment operator (this will allow copying from 'MemPtr' with other sizes)

#if EE_PRIVATE
   void    copyTo  (  TYPE *dest)C; // copy raw memory of all elements to   'dest'
   MemPtr& copyFrom(C TYPE *src ) ; // copy raw memory of all elements from 'src '
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail
   Bool loadRaw(File &f) ; // load raw memory of elements (number of elements + elements raw memory), false on fail

   // initialize 'MemPtr' to point to source
                          MemPtr& point(       null_t=null                         );
                          MemPtr& point(       TYPE             &src               );
                          MemPtr& point(       TYPE             *src, Int src_elms );
   template<Int src_elms> MemPtr& point(       TYPE            (&src)    [src_elms]);
                          MemPtr& point(Mems  <TYPE           > &src               );
                          MemPtr& point(Memc  <TYPE           > &src               );
                          MemPtr& point(Memt  <TYPE, Memt_size> &src               );
                          MemPtr& point(Memb  <TYPE           > &src               );
                          MemPtr& point(Memx  <TYPE           > &src               );
                          MemPtr& point(Meml  <TYPE           > &src               );
                          MemPtr& point(MemPtr<TYPE, Memt_size> &src               );

                          MemPtr(         null_t=null                         ) {point(null         );}
                          MemPtr(         TYPE             &src               ) {point(src          );}
   template<Int src_elms> MemPtr(         TYPE            (&src)    [src_elms]) {point(src          );}
                          MemPtr(         TYPE             *src, Int src_elms ) {point(src, src_elms);}
                          MemPtr(  Mems  <TYPE           > &src               ) {point(src          );}
                          MemPtr(  Memc  <TYPE           > &src               ) {point(src          );}
 //T1(EXTENDED)           MemPtr(  Memc  <EXTENDED       > &src               ) {point(src          );} introduces compilation problems
                          MemPtr(  Memt  <TYPE, Memt_size> &src               ) {point(src          );}
                          MemPtr(  Memb  <TYPE           > &src               ) {point(src          );}
 //T1(EXTENDED)           MemPtr(  Memb  <EXTENDED       > &src               ) {point(src          );} introduces compilation problems
                          MemPtr(  Memx  <TYPE           > &src               ) {point(src          );}
 //T1(EXTENDED)           MemPtr(  Memx  <EXTENDED       > &src               ) {point(src          );} introduces compilation problems
                          MemPtr(  Meml  <TYPE           > &src               ) {point(src          );}
                       #if WINDOWS // this compiles only on Windows, on other platforms it fails to compile. The following only prevents from pointing to const pointers (normal constructor is not needed, because in it all we want is to copy all members)
                          MemPtr(  MemPtr<TYPE, Memt_size> &src               ) {point(src          );}
                          MemPtr(C MemPtr<TYPE, Memt_size> &src               )=delete;
                       #endif

   enum MODE
   {
      PTR ,
      MEMS,
      MEMC,
      MEMT,
      MEMB,
      MEMX,
      MEML,
   };

   MODE mode()C {return _mode;} // get type of container from which 'MemPtr' was created

   Mems<TYPE           >* mems() {return (_mode==MEMS) ? _mems : null;}
   Memc<TYPE           >* memc() {return (_mode==MEMC) ? _memc : null;}
   Memt<TYPE, Memt_size>* memt() {return (_mode==MEMT) ? _memt : null;}
   Memb<TYPE           >* memb() {return (_mode==MEMB) ? _memb : null;}
   Memx<TYPE           >* memx() {return (_mode==MEMX) ? _memx : null;}
   Meml<TYPE           >* meml() {return (_mode==MEML) ? _meml : null;}

private:
   union
   {
      TYPE                  *_ptr ;
      Mems<TYPE           > *_mems;
      Memc<TYPE           > *_memc;
      Memt<TYPE, Memt_size> *_memt;
      Memb<TYPE           > *_memb;
      Memx<TYPE           > *_memx;
      Meml<TYPE           > *_meml;
   };
   MODE _mode;
   Int  _elms;

   MemPtr& operator=(null_t)=delete;

   friend struct Mems<TYPE>;
   friend struct Memc<TYPE>;
   friend struct Memt<TYPE, Memt_size>;
   friend struct Memb<TYPE>;
   friend struct Memx<TYPE>;
   friend struct Meml<TYPE>;
};
/******************************************************************************/
template<const_mem_addr typename TYPE, Int Memt_elms> struct MemPtrN : MemPtr<TYPE, SIZE(TYPE)*Memt_elms> // Memory Container Pointer, 'Memt_elms'=number of elements of the 'Memt'
{
   // copy elements using assignment operator
                          MemPtrN& operator=(C TYPE                     &src           ); // copy elements using assignment operator
   template<Int src_elms> MemPtrN& operator=(C TYPE                    (&src)[src_elms]); // copy elements using assignment operator
                          MemPtrN& operator=(C Mems   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C Memc   <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtrN& operator=(C Memt   <TYPE,  src_size> &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C Memb   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C Memx   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C Meml   <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtrN& operator=(C MemPtr <TYPE,  src_size> &src           ); // copy elements using assignment operator (this will allow copying from 'MemPtr' with other sizes)
                          MemPtrN& operator=(C MemPtrN<TYPE, Memt_elms> &src           ); // copy elements using assignment operator (this must be specified even though method above should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)

   // initialize 'MemPtrN' to point to source
                          MemPtrN(       null_t=null                                    ) {}
                          MemPtrN(       TYPE                        &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   template<Int src_elms> MemPtrN(       TYPE                       (&src)    [src_elms]) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(       TYPE                        *src, Int src_elms ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src, src_elms) {}
                          MemPtrN(Mems  <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memc  <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memt  <TYPE, SIZE(TYPE)*Memt_elms> &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memb  <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memx  <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Meml  <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(MemPtr<TYPE, SIZE(TYPE)*Memt_elms> &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}

   MemPtrN& operator=(null_t)=delete;
};
/******************************************************************************/
template<typename TYPE, Int size>   inline Int Elms(C MemPtr<TYPE, size> &memp) {return memp.elms();}
/******************************************************************************/
