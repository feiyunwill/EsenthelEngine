/******************************************************************************

   Use 'Meml' for list based dynamic array container.
   
   'Meml' preserves elements memory address in every operation.

   'Meml' stores elements independently, for example:
      null <- A <-> B <-> C -> null

/******************************************************************************/
// Meml iterate macros
#define MFREP(    meml)   for(MemlNode *i=(meml).first(); i; i=i->next()) // forward repeat
#define MFREPD(i, meml)   for(MemlNode *i=(meml).first(); i; i=i->next()) // forward repeat with definition
#define MREP(     meml)   for(MemlNode *i=(meml).last (); i; i=i->prev()) //         repeat
#define MREPD( i, meml)   for(MemlNode *i=(meml).last (); i; i=i->prev()) //         repeat with definition

// safe Meml iterate macros, use when 'i' may be deleted
#define SMFREP(    meml)   for(MemlNode *i=(meml).first(), *_next_=(i ? i->next() : null);  i;  i=_next_, _next_=(_next_ ? _next_->next() : null)) // forward repeat
#define SMFREPD(i, meml)   for(MemlNode *i=(meml).first(), *_next_=(i ? i->next() : null);  i;  i=_next_, _next_=(_next_ ? _next_->next() : null)) // forward repeat with definition
#define SMREP(     meml)   for(MemlNode *i=(meml).last (), *_prev_=(i ? i->prev() : null);  i;  i=_prev_, _prev_=(_prev_ ? _prev_->prev() : null)) //         repeat
#define SMREPD( i, meml)   for(MemlNode *i=(meml).last (), *_prev_=(i ? i->prev() : null);  i;  i=_prev_, _prev_=(_prev_ ? _prev_->prev() : null)) //         repeat with definition
/******************************************************************************/
struct MemlNode // Memory List Node, single node of Memory List
{
   Ptr       data() {return ((Byte*)this)+SIZE(T);} // get pointer to elements data
   MemlNode* prev() {return _prev                ;} // get previous   element
   MemlNode* next() {return _next                ;} // get next       element

#if !EE_PRIVATE
private:
#endif
   MemlNode *_prev, *_next;
 //TYPE      _data;
   MemlNode() {}
   NO_COPY_CONSTRUCTOR(MemlNode);
};
/******************************************************************************/
T1(TYPE) struct Meml : _Meml // List Based Container
{
   // manage
   Meml& del  (); // remove all elements
   Meml& clear(); // remove all elements

   // get / set
   Int  elms    ()C; // get number of elements
   UInt elmSize ()C; // get size   of element
   UInt memUsage()C; // get memory usage

   TYPE* addr      (Int       i   ) ; // get i-th MemlNode data address, null is returned if index is out of range
 C TYPE* addr      (Int       i   )C; // get i-th MemlNode data address, null is returned if index is out of range
   TYPE& operator[](Int       i   ) ; // get i-th MemlNode data, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int       i   )C; // get i-th MemlNode data, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int       i   ) ; // get i-th MemlNode data, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& operator[](MemlNode *node) ; // get      MemlNode data, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](MemlNode *node)C; // get      MemlNode data, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& New       (              ) ; // create new element at the  end                                                                                        , this method does not change the memory address of any of the elements
   TYPE& NewAt     (Int       i   ) ; // create new element at i-th index, all old element valid indexes starting from i-th position will be moved to the right, this method does not change the memory address of any of the elements

   MemlNode* add      (              ); // add element at the end of the list, this method does not change the memory address of any of the elements
   MemlNode* addBefore(MemlNode *node); // add element before 'node'         , this method does not change the memory address of any of the elements
   MemlNode* addAfter (MemlNode *node); // add element after  'node'         , this method does not change the memory address of any of the elements

   MemlNode* first()C; // get first element
   MemlNode* last ()C; // get last  element

   MemlNode* loopPrev(MemlNode *node)C {return node ? (node->prev() ? node->prev() : last ()) : null;} // get looped previous element, loopPrev(first())==last ()
   MemlNode* loopNext(MemlNode *node)C {return node ? (node->next() ? node->next() : first()) : null;} // get looped next     element, loopNext(last ())==first()

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   // remove
   Meml& removeFirst(                Bool keep_order=false); // remove first element                             , this method does not change the memory address of any of the remaining elements, 'keep_order'=this parameter is ignored for 'Meml' because it always keeps order (it is kept here only for compatibility with other memory containers)
   Meml& removeLast (                                     ); // remove last  element                             , this method does not change the memory address of any of the remaining elements
   Meml& remove     (MemlNode *node, Bool keep_order=false); // remove       element                             , this method does not change the memory address of any of the remaining elements, 'keep_order'=this parameter is ignored for 'Meml' because it always keeps order (it is kept here only for compatibility with other memory containers)
   Meml& removeData (C TYPE   *elm , Bool keep_order=false); // remove       element by giving its memory address, this method does not change the memory address of any of the remaining elements, 'keep_order'=this parameter is ignored for 'Meml' because it always keeps order (it is kept here only for compatibility with other memory containers)
   Meml& removeIndex(Int       i   , Bool keep_order=false); // remove i-th  element                             , this method does not change the memory address of any of the remaining elements, 'keep_order'=this parameter is ignored for 'Meml' because it always keeps order (it is kept here only for compatibility with other memory containers)

   Meml& setNum    (Int num); // set number of elements to 'num',                                                                               this method does not change the memory address of any of the elements
   Meml& setNumZero(Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method does not change the memory address of any of the elements

   // values
   T1(VALUE) Bool binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive

   // order
   Meml&         sort(Int compare(C TYPE &a, C TYPE &b)); // sort elements with custom comparing function, method does not change the memory address of any of the elements
   Meml& reverseOrder(                                 ); // reverse order of elements          (reversing modifies only indexes), this method does not change the memory address of any of the elements
   Meml&    swapOrder(Int i, Int j                     ); // swap order of 'i' and 'j' elements (swapping  modifies only indexes), this method does not change the memory address of any of the elements

   // misc
                      Meml& operator=(C Mems  <TYPE      >  &src); // copy elements using assignment operator
                      Meml& operator=(C Memc  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Meml& operator=(C Memt  <TYPE, size>  &src); // copy elements using assignment operator
                      Meml& operator=(C Memb  <TYPE      >  &src); // copy elements using assignment operator
                      Meml& operator=(C Memx  <TYPE      >  &src); // copy elements using assignment operator
                      Meml& operator=(C Meml  <TYPE      >  &src); // copy elements using assignment operator
   template<Int size> Meml& operator=(C MemPtr<TYPE, size>  &src); // copy elements using assignment operator
                      Meml& operator=(  Meml  <TYPE      > &&src); // copy elements using assignment operator

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Meml();
   Meml(C Meml  &src);
   Meml(  Meml &&src);
};
/******************************************************************************/
inline Int Elms(C _Meml &meml) {return meml.elms();}
/******************************************************************************/
