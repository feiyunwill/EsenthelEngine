/******************************************************************************/
struct _Meml // List Based Container Base - Do not use this class, use 'Meml' instead
{
   void del  ();
   void clear();

   Int  elms    ()C {return _elms    ;}
   UInt elmSize ()C {return _elm_size;}
   UInt memUsage()C {return  elms()*elmSize();}

   Ptr addr      (Int i)C {return InRange(i, _elms) ?       _element(i) : null;}
   Ptr operator[](Int i)C {  RANGE_ASSERT(i, _elms); return _element(i);}
   Ptr operator()(Int i);
   Ptr New       (     );
   Ptr NewAt     (Int i);

   MemlNode* add      (              );
   MemlNode* addZero  (              );
   MemlNode* addBefore(MemlNode *node);
   MemlNode* addAfter (MemlNode *node);

   MemlNode* first()C {return _first;}
   MemlNode* last ()C {return _last ;}

   Int  index   (CPtr elm)C;
   Bool contains(CPtr elm)C {return index(elm)>=0;}

   void removeFirst(                Bool keep_order=false) {remove(_first, keep_order);}
   void removeLast (                                     ) {remove(_last             );}
   void remove     (MemlNode *node, Bool keep_order=false);
   void removeData (CPtr      elm , Bool keep_order=false);
   void removeIndex(Int       i   , Bool keep_order=false);

   void setNum    (Int num);
   void setNumZero(Int num);
   Int  addNum    (Int num);

   void reverseOrder();
   void    swapOrder(Int i, Int j);

   Bool saveRaw(File &f)C;
   Bool loadRaw(File &f) ;

#if EE_PRIVATE
   void copyTo  ( Ptr dest)C;
   void copyFrom(CPtr src ) ;
#endif

 ~_Meml() {del();}

private:
   MemlNode *_first, *_last;
   Int       _elms;
   UInt      _elm_size;
   void    (*_new)(Ptr elm),
           (*_del)(Ptr elm);

   Ptr _element(Int i)C;

   explicit _Meml(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm)) {_elms=0; T._elm_size=elm_size; _first=_last=null; T._new=_new; T._del=_del;}

   NO_COPY_CONSTRUCTOR(_Meml);

   T1(TYPE) friend struct Meml;
};
/******************************************************************************/
