/******************************************************************************/
struct _Memb // Block Based Container Base - Do not use this class, use 'Memb' instead
{
   void del  ();
   void clear();

   Int    elms    ()C {return _elms      ;}
   UInt   elmSize ()C {return _elm_size  ;}
   UInt blockElms ()C {return _block_elms;}
   UInt blockSize ()C {return _block_elms*_elm_size;}
   UInt   memUsage()C {return _blocks*blockSize();}

   Ptr addr      (Int i)C {return InRange(i, _elms) ?       _element(i) : null;}
   Ptr operator[](Int i)C {  RANGE_ASSERT(i, _elms); return _element(i);}
   Ptr operator()(Int i);
   Ptr first     (     )C {return T[0        ];}
   Ptr last      (     )C {return T[elms()-1 ];}
   Ptr New       (     )  {return T[addNum(1)];}
   Ptr NewAt     (Int i);

   Int  index   (CPtr elm)C;
   Bool contains(CPtr elm)C {return index(elm)>=0;}

   void removeLast();
   void remove    (Int  i  , Bool keep_order=false);
   void removeData(CPtr elm, Bool keep_order=false);

   void setNum    (Int num);
   void setNumZero(Int num);
   Int  addNum    (Int num);

   void reverseOrder();
   void    swapOrder(Int i  , Int j);
   void      moveElm(Int elm, Int new_index);

   Bool saveRaw(File &f)C;
   Bool loadRaw(File &f) ;

#if EE_PRIVATE
   UInt mask()C {return _block_elms-1;} // '_block_elms' is always a power of 2, so -1 can be used as an index mask for all elements in a block

   void moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp);

   void copyTo  ( Ptr dest)C;
   void copyFrom(CPtr src ) ;
   void reset   (         ) ;
#endif

 ~_Memb() {del();}

private:
   Int    _elms;
   UInt   _elm_size, _block_elms, _blocks, _shr;
   Ptr   *_ptr;
   void (*_new)(Ptr elm),
        (*_del)(Ptr elm);
   Ptr  _element(Int i)C;

   explicit _Memb(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm));
      void _reset(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm));

   NO_COPY_CONSTRUCTOR(_Memb);

   T1(TYPE) friend struct  Memb;
   T1(TYPE) friend struct  MembAbstract;
            friend struct _Memx;
};
/******************************************************************************/
