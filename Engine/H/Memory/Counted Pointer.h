/******************************************************************************/
struct PtrCounter
{
   Bool anyPtrs()C {return _ptrs>0;}

   PtrCounter() {_ptrs=-1;}
  ~PtrCounter();

#if !EE_PRIVATE
private:
#endif
   Int _ptrs;
#if EE_PRIVATE
private:
#endif
   void decRef(void (*unload)(Ptr elm));
   void incRef(Bool (*  load)(Ptr elm));
   T1(TYPE) friend struct CountedPtr;
};
/******************************************************************************/
T1(TYPE) struct CountedPtr // Counted Element Pointer - can hold a reference to a TYPE object which must be based on 'PtrCounter'
{
   // operators
   TYPE* operator ()  (                  )C {return  T._data            ;} // access the data, you can use the returned pointer   as long as this 'CountedPtr' object exists and not modified
   TYPE* operator ->  (                  )C {return  T._data            ;} // access the data, you can use the returned pointer   as long as this 'CountedPtr' object exists and not modified
   TYPE& operator *   (                  )C {return *T._data            ;} // access the data, you can use the returned reference as long as this 'CountedPtr' object exists and not modified
   Bool  operator ==  (  null_t          )C {return  T._data==null      ;} // if pointers are equal
   Bool  operator !=  (  null_t          )C {return  T._data!=null      ;} // if pointers are different
   Bool  operator ==  (C TYPE       *data)C {return  T._data==data      ;} // if pointers are equal
   Bool  operator !=  (C TYPE       *data)C {return  T._data!=data      ;} // if pointers are different
   Bool  operator ==  (C CountedPtr &eptr)C {return  T._data==eptr._data;} // if pointers are equal
   Bool  operator !=  (C CountedPtr &eptr)C {return  T._data!=eptr._data;} // if pointers are different
         operator Bool(                  )C {return  T._data!=null      ;} // if pointer  is  valid

   // operations
   CountedPtr& clear    (                   ); // clear the pointer to  null , this automatically decreases the reference count of current data
   CountedPtr& operator=(  null_t           ); // clear the pointer to  null , this automatically decreases the reference count of current data
   CountedPtr& operator=(  TYPE       * data); // set       pointer to 'data', this automatically decreases the reference count of current data and increases the reference count of the new data
   CountedPtr& operator=(C CountedPtr & eptr); // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data
   CountedPtr& operator=(  CountedPtr &&eptr); // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data

   // constructors / destructors
   CountedPtr(  null_t=null      ); // initialize the pointer with  null
   CountedPtr(  TYPE       * data); // initialize the pointer with 'data', this automatically increases the reference count of the    'data'
   CountedPtr(C CountedPtr & eptr); // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
   CountedPtr(  CountedPtr &&eptr); // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
  ~CountedPtr(                   ); // release    the pointer            , this automatically decreases the reference count of current data

private:
   TYPE *_data;
   static void DecRef(TYPE *data);
   static void IncRef(TYPE *data);
};
/******************************************************************************/
void DelayRemoveTime(Flt time); // set amount of time (in seconds) after which unused elements are removed (<=0 value specifies immediate unloading), default=0
void DelayRemoveNow (        ); // immediately remove all elements marked for delay removal at a later time to free as much memory as possible
#if EE_PRIVATE
void DelayRemoveUpdate();
#endif
/******************************************************************************/
