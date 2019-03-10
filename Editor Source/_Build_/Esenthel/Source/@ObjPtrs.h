/******************************************************************************/
// hide and set keyboard focus to the Edit     view
/******************************************************************************/
template<typename TYPE> class ObjPtrs // warning: this must be stored only in global space
{
   bool         deleted;
   Memc<TYPE*> *objs; //=null; do not set this to null because this may have been already used before the constructor was called

  ~ObjPtrs();

   static int ComparePtr(TYPE*C &a, TYPE*C &b);

   void include(TYPE &obj);
   void exclude(TYPE &obj);

   TYPE& operator[](int i);

public:
   ObjPtrs();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
