/******************************************************************************/
template<typename TYPE>
  ObjPtrs<TYPE>::~ObjPtrs() {deleted=true; Delete(objs);}
template<typename TYPE>
   int ObjPtrs<TYPE>::ComparePtr(TYPE*C &a, TYPE*C &b) {return Compare(a, b);}
template<typename TYPE>
   void ObjPtrs<TYPE>::include(TYPE &obj) {if(!deleted){if(!objs)New(objs); objs->binaryInclude(&obj, ComparePtr);}}
template<typename TYPE>
   void ObjPtrs<TYPE>::exclude(TYPE &obj) {             if( objs)           objs->binaryExclude(&obj, ComparePtr); }
template<typename TYPE>
   TYPE& ObjPtrs<TYPE>::operator[](int i) {return *((*objs)[i]);}
template<typename TYPE> int Elms(C ObjPtrs<TYPE> &obj_ptrs) {return obj_ptrs.objs ? obj_ptrs.objs->elms() : 0;}
   T1(TYPE) PropEx& PropEx::create(C Str &name,   TYPE       &member         ) {::EE::Property::create(name, member); return T;}
template<typename TYPE>
ObjPtrs<TYPE>::ObjPtrs() : deleted(false) {}

/******************************************************************************/
