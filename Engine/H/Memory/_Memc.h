/******************************************************************************/
struct _Memc // Continuous Memory Based Container Base - Do not use this class, use 'Memc' instead
{
   void del  ();
   void clear();

   Int  elms    ()C {return _elms    ;}
   UInt elmSize ()C {return _elm_size;}
   Ptr  data    ()C {return _data    ;}
   UInt memUsage()C {return _elm_size*_max_elms;}
#if EE_PRIVATE
   UInt maxElms    ()C {return _max_elms  ;}
   Bool initialized()C {return _elm_size>0;}
#endif

   Ptr addr      (Int i)C {return InRange(i, _elms) ?       (Byte*)_data +        i *_elm_size : null;}
   Ptr addrFirst (     )C {return            _elms  ?       (Byte*)_data                       : null;}
   Ptr addrLast  (     )C {return            _elms  ?       (Byte*)_data + (_elms-1)*_elm_size : null;}
   Ptr operator[](Int i)C {  RANGE_ASSERT(i, _elms); return (Byte*)_data +        i *_elm_size;}
   Ptr operator()(Int i);
   Ptr first     (     )C {return T[0        ];}
   Ptr last      (     )C {return T[elms()-1 ];}
   Ptr New       (     )  {return T[addNum(1)];}
   Ptr NewAt     (Int i);

   Int  index   (CPtr elm)C;
   Bool contains(CPtr elm)C {return index(elm)>=0;}

   void removeLast();
   void remove    (Int  i,        Bool keep_order=false);
   void removeNum (Int  i, Int n, Bool keep_order=false);
   void removeData(CPtr elm,      Bool keep_order=false);

   void setNum    (Int num);
   void setNumZero(Int num);
   Int  addNum    (Int num);
   Int  addNumZero(Int num);
   void reserve   (Int num);

   Bool binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C;

   void           sort(Int compare(CPtr a, CPtr b));
   void   reverseOrder();
   void randomizeOrder();
   void    rotateOrder(Int offset);
   void      swapOrder(Int i  , Int j);
   void      moveElm  (Int elm, Int new_index);

   Bool saveRaw(File &f)C;
   Bool loadRaw(File &f) ;

#if EE_PRIVATE
   void moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp);

   void copyTo  ( Ptr dest)C;
   void copyFrom(CPtr src ) ;

   Bool _saveRaw(File &f)C;
   Bool _loadRaw(File &f) ;
#endif

 ~_Memc() {del();}

private:
   Int    _elms;
   UInt   _elm_size, _max_elms;
   Ptr    _data;
   void (*_new)(Ptr elm),
        (*_del)(Ptr elm);

   explicit _Memc(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));
      void _reset(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));

   NO_COPY_CONSTRUCTOR(_Memc);

   T1(TYPE) friend struct  Memc;
   T1(TYPE) friend struct  MemcAbstract;
            friend struct _MemcThreadSafe;
};
/******************************************************************************/
