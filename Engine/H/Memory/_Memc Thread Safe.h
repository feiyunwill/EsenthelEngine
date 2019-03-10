/******************************************************************************/
struct _MemcThreadSafe // Thread-Safe Continuous Memory Based Container Base - Do not use this class, use 'MemcThreadSafe' instead
{
   void clear();
   void del  ();

   // get / set
   Int  elms    ()C {return _memc.elms    ();}
   UInt elmSize ()C {return _memc.elmSize ();}
   UInt memUsage()C {return _memc.memUsage();}

   Ptr lockedData (     )C {return _memc.data ( );}
   Ptr lockedAddr (Int i)C {return _memc.addr (i);}
   Ptr lockedElm  (Int i)C {return _memc      [i];}
   Ptr lockedFirst(     )C {return _memc.first( );}
   Ptr lockedLast (     )C {return _memc.last ( );}
   Ptr lockedNew  (     )  {return _memc.New  ( );}
   Ptr lockedNewAt(Int i)  {return _memc.NewAt(i);}

   Int  index   (CPtr elm)C;
   Bool contains(CPtr elm)C {return index(elm)>=0;}

   void removeLast();
   void remove    (Int  i  , Bool keep_order=false);
   void removeData(CPtr elm, Bool keep_order=false);

   void setNum    (Int num);
   void setNumZero(Int num);
   Int  addNum    (Int num);

   Bool binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C;

   void           sort(Int compare(CPtr a, CPtr b));
   void   reverseOrder();
   void randomizeOrder();
   void    rotateOrder(Int offset);
   void      swapOrder(Int i  , Int j);
   void      moveElm  (Int elm, Int new_index);

   // operations
   void   lock()C {_lock.on ();}
   void unlock()C {_lock.off();}

private:
  _Memc     _memc;
   SyncLock _lock;

   explicit _MemcThreadSafe(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));

   T1(TYPE) friend struct MemcThreadSafe;
};
/******************************************************************************/
