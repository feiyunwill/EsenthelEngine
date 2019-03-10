/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void _Memb::_reset(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T.~_Memb();
   new(this)_Memb(elm_size, block_elms, _new, _del);
}
_Memb::_Memb(Int elm_size, Int block_elms, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T._ptr   =null;
   T._elms  =0;
   T._blocks=0;
   T._elm_size  =elm_size;
   T._block_elms=Max  (1, CeilPow2(block_elms));
   T._shr       =BitHi(blockElms());
   T._new       =_new;
   T._del       =_del;
}
/******************************************************************************/
void _Memb::clear()
{
   if(_del)REPA(T)_del(T[i]);
  _elms=0;
}
void _Memb::del()
{
   clear();
   REP(_blocks)Free(_ptr[i]); Free(_ptr);
  _blocks=0;
}
void _Memb::reset() // remove all elements and leave the first block if it exists
{
   clear();
   if(_blocks>1)
   {
      for(Int i=_blocks; --i>=1; )Free(_ptr[i]); // release all blocks except #0
      Realloc(_ptr, 1, _blocks);
     _blocks=1;
   }
}
/******************************************************************************/
void _Memb::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      Int old_elms  = elms(); _elms=num;
      Int new_blocks=(elms()+blockElms()-1)>>_shr;
      if( new_blocks>_blocks)
      {
         if(!elmSize())Exit("Attempting to create an object of zero size in 'Memb' container.\nThe container is not initialized or it is abstract and 'replaceClass' hasn't been called.");
                                      new_blocks=CeilPow2(new_blocks);
                        Realloc(_ptr, new_blocks, _blocks);
         REP(new_blocks-_blocks)_ptr[_blocks++]=Alloc(blockSize());
      }
      if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]);
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]);
     _elms=num;
   }
}
void _Memb::setNumZero(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      Int old_elms  = elms(); _elms=num;
      Int new_blocks=(elms()+blockElms()-1)>>_shr;
      if( new_blocks>_blocks)
      {
         if(!elmSize())Exit("Attempting to create an object of zero size in 'Memb' container.\nThe container is not initialized or it is abstract and 'replaceClass' hasn't been called.");
                                      new_blocks=CeilPow2(new_blocks);
                        Realloc(_ptr, new_blocks, _blocks);
         REP(new_blocks-_blocks)_ptr[_blocks++]=Alloc(blockSize());
      }
      for(Int i=old_elms; i<elms(); i++){Ptr elm=T[i]; Zero(elm, elmSize()); if(_new)_new(elm);}
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]);
     _elms=num;
   }
}
/******************************************************************************/
Int _Memb::addNum(Int num) {Int index=elms(); setNum(elms()+num); return index;}
/******************************************************************************/
Ptr _Memb::NewAt(Int i)
{
   Clamp(i, 0, elms());
   New();
   for(Int j=elms()-1; j>i; j--)SwapFast(T[j], T[j-1], elmSize());
   return T[i];
}
/******************************************************************************/
void _Memb::removeLast()
{
   if(elms())
   {
      if(_del)_del(T[elms()-1]);
     _elms--;
   }
}
void _Memb::remove(Int i, Bool keep_order)
{
   if(InRange(i, T))
   {
      if(_del)_del(T[i]);
      if(elms()-1>i)
      {
         if(!keep_order              )CopyFast(T[i], T[elms()-1], elmSize());else
         for(Int j=i; j<elms()-1; j++)CopyFast(T[j], T[     j+1], elmSize());
      }
     _elms--;
   }
}
void _Memb::removeData(CPtr elm, Bool keep_order)
{
   remove(index(elm), keep_order);
}
/******************************************************************************/
Ptr _Memb::_element(Int i)C
{
   return (Byte*)(_ptr[i>>_shr]) + (i&mask())*elmSize();
}
Ptr _Memb::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside _Memb.operator()(Int i)");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
/******************************************************************************/
Int _Memb::index(CPtr elm)C
{
   if(elm)
   {
      UIntPtr  data       =UIntPtr(elm),
               block_size =blockSize();
      Int      used_blocks=(elms()+blockElms()-1)>>_shr;
      FREPD(b, used_blocks)
      {
         UIntPtr p=data-UIntPtr(_ptr[b]);
         if(p<block_size) // if belongs to this block, unsigned compare will already guarantee "i>=0 && "
         {
            Int i=b*blockElms() + p/elmSize();
            return InRange(i, T) ? i : -1;
         }
      }
   }
   return -1;
}
/******************************************************************************/
void _Memb::reverseOrder()
{
   Int last=elms()-1;
   REP(elms()/2)SwapFast(T[i], T[last-i], elmSize());
}
void _Memb::swapOrder(Int i, Int j)
{
   if(InRange(i, T) && InRange(j, T))Swap(T[i], T[j], elmSize());
}
void _Memb::moveElm(Int elm, Int new_index)
{
   if(InRange(elm, elms()))
   {
      Clamp(new_index, 0, elms()-1); if(new_index!=elm)
      {
         Memt<Byte> buf; buf.setNum(elmSize()); Ptr temp=buf.data();
         CopyFast(temp, T[elm], elmSize()); // copy element from data to temp memory
                           //                                                                                       E N        E N
         if(elm<new_index) // element is on the left, and we're moving it to the right, move the data to the left "0X123" -> "012X3"
         {
            for(Int i=elm; i<new_index; i++)CopyFast(T[i], T[i+1], elmSize());
         }    //                                                                                        N E        N E
         else // element is on the right, and we're moving it to the left, move the data to the right "012X3" -> "0X123"
         {
            for(Int i=elm; i>new_index; i--)CopyFast(T[i], T[i-1], elmSize());
         }
         CopyFast(T[new_index], temp, elmSize()); // copy element from temp memory back to the data
      }
   }
}
void _Memb::moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp) // !! assumes indexes are in range, "elm>=new_index", 'temp' can fit element !!
{
   if(new_index!=elm)
   {
      CopyFast(temp, T[elm], elmSize()); // copy element from data to temp memory
   #if 0 // not needed since we're always moving left in this function
                        //                                                                                       E N        E N
      if(elm<new_index) // element is on the left, and we're moving it to the right, move the data to the left "0X123" -> "012X3"
      {
         for(Int i=elm; i<new_index; i++)CopyFast(T[i], T[i+1], elmSize());
      }    //                                                                                        N E        N E
      else // element is on the right, and we're moving it to the left, move the data to the right "012X3" -> "0X123"
   #endif
      {
         for(Int i=elm; i>new_index; i--)CopyFast(T[i], T[i-1], elmSize());
      }
      CopyFast(T[new_index], temp, elmSize()); // copy element from temp memory back to the data
   }
}
/******************************************************************************/
Bool _Memb::saveRaw(File &f)C
{
   f.cmpUIntV(elms());
   if(elms())
   {
      Int  block_size =blockSize(),
           full_blocks=elms()>>_shr;
      FREP(full_blocks)f.put(_ptr[i          ],              block_size  );
                       f.put(_ptr[full_blocks], (elms()&mask())*elmSize()); // last block
   }
   return f.ok();
}
Bool _Memb::loadRaw(File &f)
{
   setNum(f.decUIntV());
   if(elms())
   {
      Int  block_size =blockSize(),
           full_blocks=elms()>>_shr;
      FREP(full_blocks)f.getFast(_ptr[i          ],              block_size  );
                       f.getFast(_ptr[full_blocks], (elms()&mask())*elmSize()); // last block
   }
   if(f.ok())return true;
   clear();  return false;
}
/******************************************************************************/
void _Memb::copyTo(Ptr dest)C
{
   if(elms() && dest)
   {
      Int  block_size =blockSize(),
           full_blocks=elms()>>_shr;
      FREP(full_blocks){CopyFast(dest, _ptr[i          ],              block_size  ); dest=(Byte*)dest+block_size;}
                        CopyFast(dest, _ptr[full_blocks], (elms()&mask())*elmSize()); // last block
   }
}
void _Memb::copyFrom(CPtr src)
{
   if(elms())
   {
      Int  block_size =blockSize(),
           full_blocks=elms()>>_shr;
      FREP(full_blocks){Copy(_ptr[i          ], src,              block_size  ); if(src)src=(Byte*)src+block_size;}
                        Copy(_ptr[full_blocks], src, (elms()&mask())*elmSize()); // last block
   }
}
/******************************************************************************
void _Memb::copyRaw(_Memb &dest)
{
   if(this!=&dest)
   {
      dest.del();

      dest._ptr=null;
      dest._shr=_shr;
      dest._elm_size  =elmSize  (); dest._elms  =0;
      dest._block_elms=blockElms(); dest._blocks=0;
      dest._new=_new;
      dest._del=_del;
      dest.setNum(_elms);
      if(_elms)
      {
         Int  block_size =blockSize(),
              full_blocks=elms()>>_shr;
         FREP(full_blocks)CopyFast(dest._ptr[i          ], _ptr[i   ],             block_size  );
                          CopyFast(dest._ptr[full_blocks], _ptr[full], (_elms&mask())*elmSize()); // last block
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
