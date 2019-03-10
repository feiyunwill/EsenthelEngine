/******************************************************************************/
struct _Memx // Block Based Extended Container Base - Do not use this class, use 'Memx' instead
{
   void del  ();
   void clear();

    Int   absElms ()C {return               _abs.elms();}
    Int validElms ()C {return             _valid.elms();}
    Int      elms ()C {return             _valid.elms();}
   UInt   elmSize ()C {return _abs.elmSize()-SIZE(UInt);}
   UInt   memUsage()C;
#if EE_PRIVATE
   UInt   maxElms ()C {return _valid.maxElms();}
#endif

   Ptr   absElm  (Int i)C {return (Byte*)_abs  [i]+SIZE(UInt);}
   Ptr validElm  (Int i)C {return absElm(_valid[i]);}
   Ptr addr      (Int i)C {return InRange(i, T) ? T[i] : null;}
   Ptr operator[](Int i)C {return validElm(i);}
   Ptr first     (     )C {return validElm(0);}
   Ptr last      (     )C {return validElm(validElms()-1);}
   Ptr New       (     );
   Ptr NewAt     (Int i);

   Int  validToAbsIndex(Int  valid)C;
   Int  absToValidIndex(Int  abs  )C;
   Int  validIndex     (CPtr elm  )C;
   Int    absIndex     (CPtr elm  )C;
   Bool   contains     (CPtr elm  )C {return validIndex(elm)>=0;}
#if EE_PRIVATE
   Int    absIndexFastUnsafeValid(CPtr elm)C; // fast version that doesn't do any safety checks, assumes that "elm!=null" and points exactly to the element, and it is valid (not removed)
#endif

   void removeAbs  (Int  i  , Bool keep_order=false);
   void removeValid(Int  i  , Bool keep_order=false);
   void removeData (CPtr elm, Bool keep_order=false);
   void removeLast ();

   void setNum(Int num);
   Int  addNum(Int num);

   void reverseOrder();
   void    swapOrder(Int i  , Int j);
   void moveElm     (Int elm, Int new_index);
   void moveToStart (Int elm);
   void moveToEnd   (Int elm);

   Bool saveRaw(File &f)C;
   Bool loadRaw(File &f) ;

#if EE_PRIVATE
   void moveElmLeftUnsafe(Int elm, Int new_index);

   void copyTo  ( Ptr dest)C;
   void copyFrom(CPtr src ) ;
   void verify  ()C;
#endif

 ~_Memx() {del();}

private:
  _Memb       _abs;
   Memc<UInt> _valid, _invalid;
   void     (*_new)(Ptr elm),
            (*_del)(Ptr elm);

   explicit _Memx(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm));
      void _reset(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm));

   NO_COPY_CONSTRUCTOR(_Memx);

   T1(TYPE) friend struct Memx;
   T1(TYPE) friend struct MemxAbstract;
};
/******************************************************************************/
