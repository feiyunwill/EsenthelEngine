/******************************************************************************/
#include "stdafx.h"

#if APPLE
   #ifndef PAGE_SIZE
      #define PAGE_SIZE 4096 // Apple has 4k page size - https://developer.apple.com/library/mac/documentation/performance/conceptual/managingmemory/articles/aboutmemory.html "In both OS X and iOS, the size of a page is 4 kilobytes."
   #endif
#endif
#if IOS
   #define RISKY_MEM 1
#endif

namespace EE{
/******************************************************************************

   'valid', 'invalid' - they point into index in 'abs'
   data in 'abs' - first there is UInt index pointing into 'valid' or 'invalid' (SIGN_BIT), after UInt there is actual data

/******************************************************************************/
#if RISKY_MEM
static inline Bool RiskyMem(CPtr data) {return (UIntPtr(data)&(PAGE_SIZE-1))<SIZE(UInt);} // if the data lies at the page boundary making the previous UInt to belong to a different page
#endif
/******************************************************************************/
void _Memx::_reset(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T.~_Memx();
   new(this)_Memx(elm_size, block_elms, _new, _del);
}
_Memx::_Memx(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm)) : _abs(SIZE(UInt)+elm_size, block_elms, null, null)
{
   T._new=_new;
   T._del=_del;
}
void _Memx::del()
{
   clear();
      _abs.del();
    _valid.del();
  _invalid.del();
}
void _Memx::clear()
{
   if(_del)REPA(_valid)_del(validElm(i));
      _abs.clear();
    _valid.clear();
  _invalid.clear();
}
/******************************************************************************/
Ptr _Memx::New()
{
   UInt  i; if(_invalid.elms())i=_invalid.pop();else i=_abs.addNum(1);
   UInt  v=_valid.addNum(1);
           _valid[v]=i;
   Ptr elm=absElm(i);
   ((UInt*)elm)[-1]=v; // store valid index
   if(_new)_new(elm);else if(!elmSize())Exit("Attempting to create an object of zero size in 'Memx' container.\nThe container is not initialized or it is abstract and 'replaceClass' hasn't been called.");
   return elm;
}
Ptr _Memx::NewAt(Int i)
{
   Clamp(i, 0, elms());
   Ptr elm=New(); // now 'elm' is at the end
   for(Int j=elms()-2; j>=i; j--)swapOrder(j, j+1);
   return elm;
}
/******************************************************************************/
void _Memx::removeAbs(Int i, Bool keep_order)
{
   if(InRange(i, _abs))
   {
      Ptr  elm  =absElm(i);
      UInt index=((UInt*)elm)[-1];
      if(!(index&SIGN_BIT))removeValid(index, keep_order); // if actually valid
   }
}
void _Memx::removeValid(Int i, Bool keep_order)
{
   if(InRange(i, _valid))
   {
      // element info
      UInt abs=_valid [i  ];
      Ptr  elm= absElm(abs);

      // put removed to invalid
      UInt inv=_invalid.addNum(1);
               _invalid[inv]=abs;
      ((UInt*)elm)[-1]=inv^SIGN_BIT;

      if(keep_order)
      {
                 _valid.remove(i, true); // remove from valid, 'true' is required because of UInt indexes
         for(; i<_valid.elms(); i++)
         {
            Ptr elm=validElm(i);
            ((UInt*)elm)[-1]=i; // set updated index
         }
      }else
      {
            _valid.remove(i, false); // remove from valid, 'false' is required because of UInt indexes
         if(_valid.elms()>i       )  // fix another valid which was moved in 'remove'
         {
            Ptr elm=validElm(i);
            ((UInt*)elm)[-1]=i; // set updated index of element which was placed into removed element place
         }
      }

      // delete element
      if(_del)_del(elm);
   }
}
void _Memx::removeData(CPtr elm, Bool keep_order)
{
   removeValid(validIndex(elm), keep_order);
}
void _Memx::removeLast()
{
   removeValid(elms()-1);
}
void _Memx::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      REP(num-elms())New();
   }else
   if(num<elms()) // remove elements
   {
      REP(elms()-num)removeValid(num+i);
   }
}
Int  _Memx::addNum(Int num) {Int index=elms(); setNum(elms()+num); return index;}
void _Memx::reverseOrder()
{
   Int last=elms()-1;
   REP(elms()/2)
   {
      Int    j=last-i;
      UInt &ai=_valid[i], // absolute index of i-th valid element
           &aj=_valid[j]; // absolute index of j-th valid element
      Swap(*(UInt*)(_abs[ai]), *((UInt*)_abs[aj]));
      Swap(              ai  ,               aj  );
   }
}
void _Memx::swapOrder(Int i, Int j)
{
   if(InRange(i, elms()) && InRange(j, elms()) && i!=j)
   {
      UInt &ai=_valid[i], // absolute index of i-th valid element
           &aj=_valid[j]; // absolute index of j-th valid element
      Swap(*(UInt*)(_abs[ai]), *((UInt*)_abs[aj]));
      Swap(              ai  ,               aj  );
   }
}
void _Memx::moveElm(Int elm, Int new_index)
{
   if(InRange(elm, elms()))
   {
      Clamp(new_index, 0, elms()-1); if(new_index!=elm)
      {
         if(new_index>elm) // moving from left to right
         {
            for(Int i=elm+1; i<=new_index; i++){Int abs=_valid[i]; (*(UInt*)(_abs[abs]))--;} // elements now have smaller valid indexes
         }else // moving from right to left
         {
            for(Int i=elm-1; i>=new_index; i--){Int abs=_valid[i]; (*(UInt*)(_abs[abs]))++;} // elements now have bigger  valid indexes
         }
         Int abs=_valid[elm]; *(UInt*)(_abs[abs])=new_index;
        _valid.moveElm(elm, new_index);
      }
   }
}
void _Memx::moveElmLeftUnsafe(Int elm, Int new_index)
{
   if(new_index!=elm)
   {
   #if 0 // not needed since we're always moving left in this function
      if(new_index>elm) // moving from left to right
      {
         for(Int i=elm+1; i<=new_index; i++){Int abs=_valid[i]; (*(UInt*)(_abs[abs]))--;} // elements now have smaller valid indexes
      }else // moving from right to left
   #endif
      {
         for(Int i=elm-1; i>=new_index; i--){Int abs=_valid[i]; (*(UInt*)(_abs[abs]))++;} // elements now have bigger  valid indexes
      }
      Int abs=_valid[elm]; *(UInt*)(_abs[abs])=new_index;
      UInt temp; ASSERT(SIZE(*_valid.data())==SIZE(temp)); _valid.moveElmLeftUnsafe(elm, new_index, &temp);
   }
}
void _Memx::moveToStart(Int i) {moveElm(i,        0);}
void _Memx::moveToEnd  (Int i) {moveElm(i, elms()-1);}
/******************************************************************************/
void _Memx::copyTo  ( Ptr dest)C {if(dest)FREPA(T){CopyFast(dest, T[i], elmSize());        dest=(Byte*)dest+elmSize();}}
void _Memx::copyFrom(CPtr src )  {        FREPA(T){Copy    (T[i], src , elmSize()); if(src)src =(Byte*)src +elmSize();}}
/******************************************************************************/
UInt _Memx::memUsage()C
{
   return _abs.memUsage()
       +_valid.memUsage()
     +_invalid.memUsage();
}
/******************************************************************************/
Int _Memx::validToAbsIndex(Int valid)C
{
   return InRange(valid, _valid) ? _valid[valid] : -1;
}
Int _Memx::absToValidIndex(Int abs)C
{
   if(InRange(abs, _abs))
   {
      Ptr  elm  =absElm(abs);
      UInt index=((UInt*)elm)[-1];
      if(!(index&SIGN_BIT))return index; // if not removed
   }
   return -1;
}
Int _Memx::absIndex(CPtr elm)C
{
   if(elm && absElms()) // check if there are still elements left (in case memory was already released by destructor)
   {
   #if RISKY_MEM
      if(RiskyMem(elm))return _abs.index(elm); // when the memory is risky then do a slow/safe version to avoid EXC_BAD_ACCESS
   #endif
      UInt index=((UInt*)elm)[-1];
      if(  index&SIGN_BIT) // removed
      {
         index^=SIGN_BIT; if(InRange(index, _invalid))
         {
            Int abs_index=_invalid[index];
            if( absElm(abs_index)==elm)return abs_index;
         }
      }else
      {
         if(InRange(index, _valid))
         {
            Int abs_index=_valid[index];
            if( absElm(abs_index)==elm)return abs_index;
         }
      }
   }
   return -1;
}
Int _Memx::absIndexFastUnsafeValid(CPtr elm)C
{
   UInt index=((UInt*)elm)[-1]; return _valid[index];
}
Int _Memx::validIndex(CPtr elm)C
{
   if(elm && validElms()) // check if there are still elements left (in case memory was already released by destructor)
   {
   #if RISKY_MEM
      if(RiskyMem(elm) && _abs.index(elm)<0)return -1; // when the memory is risky then do a slow/safe check to avoid EXC_BAD_ACCESS
   #endif
      UInt index=((UInt*)elm)[-1];
      if(!(index&SIGN_BIT)) // if index points to a valid element
         if(InRange(index, _valid)) // if index fits
            if(validElm(index)==elm)return index; // if memory address fits
   }
   return -1;
}
/******************************************************************************/
void _Memx::verify()C
{
   REPA(T)
   {
      Int abs=validToAbsIndex(i);
      if(!InRange(abs, _abs))Exit("Invalid abs index");
      Int valid=absToValidIndex(abs);
      if( valid!=i)Exit("Invalid valid index");
   }
}
/******************************************************************************/
Bool _Memx::saveRaw(File &f)C
{
   f.cmpUIntV(elms());
   FREPA(T)f.put(T[i], elmSize());
   return f.ok();
}
Bool _Memx::loadRaw(File &f)
{
   setNum(f.decUIntV());
   FREPA(T)f.getFast(T[i], elmSize());
   if(f.ok())return true;
   clear(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
