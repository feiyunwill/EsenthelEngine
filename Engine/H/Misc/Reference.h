/******************************************************************************/
T1(TYPE) struct Reference // Reference, this is a pointer to an object which may be deleted but its memory is still accessible and possibly occupied by another object of similar class
{
   // get / set
   void  clear     ()  {_object=null; _object_id.zero ();            } // clear reference to null
   Bool  is        ()C {return        _object_id.valid();            } // check if the reference is not empty (        want to point to some object       )
   Bool  empty     ()C {return       !_object_id.valid();            } // check if the reference is     empty (doesn't want to point to any  object at all)
   Bool  valid     ()C {return  _object && _object->id()==_object_id;} // check if the reference is     valid (points to an existing object with matching ID)
   TYPE* validPtr  ()C {return   valid() ? _object : null           ;} // return a pointer   to object that is valid
 C UID & objectID  ()C {return  _object_id                          ;} // return                object ID
   TYPE& operator()()C {return *_object                             ;} // return a reference to object, usage of this method must be preceded by performing a reference validation using 'valid' method
   TYPE* operator->()C {return  _object                             ;} // return a reference to object, usage of this method must be preceded by performing a reference validation using 'valid' method

   // compare
   T1(OBJECT)  Bool  operator==(  OBJECT           *object)C {return T._object==     object && T._object_id==(object ? object->id() : UIDZero);} // if reference points to 'object'
   T1(OBJECT)  Bool  operator==(C OBJECT           *object)C {return T._object==     object && T._object_id==(object ? object->id() : UIDZero);} // if reference points to 'object'
   T1(OBJECT)  Bool  operator==(C OBJECT           &object)C {return T._object==    &object && T._object_id==          object .id()           ;} // if reference points to 'object'
   T1(TYPE2 )  Bool  operator==(C Reference<TYPE2> &ref   )C {return T._object==ref._object && T._object_id==     ref._object_id              ;} // if references are equal

   T1(OBJECT)  Bool  operator!=(  OBJECT           *object)C {return T._object!=     object || T._object_id!=(object ? object->id() : UIDZero);} // if reference doesn't point to 'object'
   T1(OBJECT)  Bool  operator!=(C OBJECT           *object)C {return T._object!=     object || T._object_id!=(object ? object->id() : UIDZero);} // if reference doesn't point to 'object'
   T1(OBJECT)  Bool  operator!=(C OBJECT           &object)C {return T._object!=    &object || T._object_id!=          object .id()           ;} // if reference doesn't point to 'object'
   T1(TYPE2 )  Bool  operator!=(C Reference<TYPE2> &ref   )C {return T._object!=ref._object || T._object_id!=     ref._object_id              ;} // if references are not equal

   // io
   Bool save(File &f)C; // save 'object_id', false on fail
   Bool load(File &f) ; // load 'object_id' and set 'object' pointer to null, actual linking pointer to the object should be performed later using 'link' method

   // link
   T1(WORLD) void link(WORLD &world); // link the reference with existing world object

   // construct
   Reference(         ) {clear();}
   Reference(TYPE &obj) {   T._object=&obj; T._object_id=obj .id();                         }
   Reference(TYPE *obj) {if(T._object= obj) T._object_id=obj->id();else T._object_id.zero();}

private:
   TYPE *_object;
   UID   _object_id;
};
/******************************************************************************/
