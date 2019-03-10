/******************************************************************************

   Use 'FixedArray' for a fixed size array that supports replacing its type.
   Use 'FixedElm'   for a single element   that supports replacing its type.

   An example:
      int                normal_array[4];
      FixedArray<int, 4>  fixed_array   ; // 'fixed_array' is the same as 'normal_array' (stores 4 int's)

   'FixedArray' stores elements in continuous memory, for example:
      [ABCDE]

   'FixedArray' has a constant number of elements, pre-defined at the moment of array declaration,
      which means that it does not support adding/removing elements.
   
   'FixedArray' however allows to dynamically change the class of elements in this array.

   If you need to store only one element then you can use 'FixedElm' instead.

/******************************************************************************/
template<typename TYPE, Int NUM> struct FixedArray // array of fixed size, with the option to dynamically replace the type of elements
{
   TYPE& operator[](Int i)  {RANGE_ASSERT(i, NUM); return _data ? *(TYPE*)((Byte*)_data+i*_elm_size) : _data_org[i];} // get i-th element in the container
 C TYPE& operator[](Int i)C {RANGE_ASSERT(i, NUM); return _data ? *(TYPE*)((Byte*)_data+i*_elm_size) : _data_org[i];} // get i-th element in the container

   Int elms   ()C {return  NUM     ;} // get number of          elements in this container
   Int elmSize()C {return _elm_size;} // get size   of a single element  in this container

   T1(EXTENDED) FixedArray& replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

              FixedArray&   del(); // manually delete, this deletes objects which were dynamically created through the 'replaceClass' method, and resets the container element class to default 'TYPE'
             ~FixedArray() {del();}
              FixedArray();
              FixedArray(C FixedArray &src);
   FixedArray& operator=(C FixedArray &src);

private:
   Int  _elm_size;
   TYPE _data_org[NUM], *_data;
};
/******************************************************************************/
template<typename TYPE> struct FixedElm // single element, with the option to dynamically replace its type (works like 'FixedArray' but stores only one element)
{
   TYPE& operator()()  {return _data ? *(TYPE*)_data : _data_org;}     TYPE* operator->()  {return &T();}     TYPE* operator&()  {return &T();}   operator   TYPE&()  {return T();} // get the element
 C TYPE& operator()()C {return _data ? *(TYPE*)_data : _data_org;}   C TYPE* operator->()C {return &T();}   C TYPE* operator&()C {return &T();}   operator C TYPE&()C {return T();} // get the element

   T1(EXTENDED) FixedElm& replaceClass(); // replace the type of class of the element, previous element is automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

              FixedElm&   del(); // manually delete, this deletes the element which was dynamically created through the 'replaceClass' method, and resets the element class to default 'TYPE'
             ~FixedElm() {del();}
              FixedElm();
              FixedElm(C FixedElm &src);
   FixedElm& operator=(C FixedElm &src);

private:
   TYPE _data_org, *_data;
};
/******************************************************************************/
template<typename TYPE, Int NUM>   inline Int Elms(C FixedArray<TYPE, NUM> &array) {return array.elms();}
/******************************************************************************/
