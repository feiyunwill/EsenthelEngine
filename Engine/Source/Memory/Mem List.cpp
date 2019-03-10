/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void _Meml::del()
{
   clear();
}
void _Meml::clear()
{
   for(MemlNode *node=first(), *next; node; node=next)
   {
      next=node->next();
      if(_del)_del(node->data());
      Free(node);
   }
  _first=_last=null;
  _elms=0;
}
void _Meml::setNum(Int num)
{
   MAX(num, 0);
   for(; _elms>num; )removeLast();
   for(; _elms<num; )add       ();
}
void _Meml::setNumZero(Int num)
{
   MAX(num, 0);
   for(; _elms>num; )removeLast();
   for(; _elms<num; )addZero   ();
}
Int _Meml::addNum(Int num) {Int index=elms(); setNum(elms()+num); return index;}
/******************************************************************************/
Ptr _Meml::New  (     ) {return add()->data();}
Ptr _Meml::NewAt(Int i)
{
   if(i>=elms())return New();
   MemlNode *node=first(); REPD(j, i)node=node->next();
   return addBefore(node)->data();
}
/******************************************************************************/
MemlNode* _Meml::add()
{
   MemlNode *node=(MemlNode*)Alloc<Byte>(SIZE(MemlNode)+_elm_size);
   if(_last)_last->_next=node;else _first=node;
   node->_prev=_last;
   node->_next= null;
  _last=node;
  _elms++;
   if(_new)_new(node->data());
   return node;
}
MemlNode* _Meml::addZero()
{
   MemlNode *node=(MemlNode*)AllocZero<Byte>(SIZE(MemlNode)+_elm_size);
   if(_last)_last->_next=node;else _first=node;
   node->_prev=_last;
   node->_next= null;
  _last=node;
  _elms++;
   if(_new)_new(node->data());
   return node;
}
MemlNode* _Meml::addBefore(MemlNode *x)
{
   if(!x)return null;
   MemlNode *node=(MemlNode*)Alloc<Byte>(SIZE(MemlNode)+_elm_size);
   if(x->_prev)x->_prev->_next=node;else _first=node;
   node->_next=x;
   node->_prev=x->_prev;
   x   ->_prev=node;
  _elms++;
   if(_new)_new(node->data());
   return node;
}
MemlNode* _Meml::addAfter(MemlNode *x)
{
   if(!x)return null;
   MemlNode *node=(MemlNode*)Alloc<Byte>(SIZE(MemlNode)+_elm_size);
   if(x->_next)x->_next->_prev=node;else _last=node;
   node->_prev=x;
   node->_next=x->_next;
   x   ->_next=node;
  _elms++;
   if(_new)_new(node->data());
   return node;
}
void _Meml::remove(MemlNode *node, Bool keep_order)
{
   if(node)
   {
      if(_del)_del(node->data());
     _elms--;
      if(_first==node)_first= node->_next;
      if(_last ==node)_last = node->_prev;
      if( node->_prev) node->_prev->_next=node->_next;
      if( node->_next) node->_next->_prev=node->_prev;
      Free(node);
   }
}
void _Meml::removeData(CPtr elm, Bool keep_order)
{
   if(elm)remove( (MemlNode*) ( ((Byte*)elm)-SIZE(MemlNode) ) );
}
void _Meml::removeIndex(Int i, Bool keep_order)
{
   if(Ptr data=T[i])removeData(data);
}
/******************************************************************************/
Int _Meml::index(CPtr elm)C
{
   Int j=0; if(elm)MFREP(T){if(i->data()==elm)return j; j++;}
   return -1;
}
Ptr _Meml::_element(Int j)C
{
   if(InRange(j, _elms))
   {
      if(  j<(_elms>>1)){MFREP(T)if(!j--)return i->data();} // first  half
      else{j= _elms-j-1;  MREP(T)if(!j--)return i->data();} // second half
   }
   return null;
}
Ptr _Meml::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside _Meml.operator()(Int i)");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
/******************************************************************************/
void _Meml::copyTo  ( Ptr dest)C {if(dest)MFREP(T){CopyFast(dest, i->data(), _elm_size);        dest=(Byte*)dest+_elm_size;}}
void _Meml::copyFrom(CPtr src )  {        MFREP(T){Copy    (i->data(), src , _elm_size); if(src)src =(Byte*)src +_elm_size;}}
/******************************************************************************/
void _Meml::reverseOrder()
{
   for(MemlNode *node=first(); node; )
   {
      MemlNode *next=node->next();
      Swap(node->_prev, node->_next);
      node=next;
   }
   Swap(_first, _last);
}
void _Meml::swapOrder(Int i, Int j)
{
   if(InRange(i, elms()) && InRange(j, elms()) && i!=j)
   {
      if(i>j)Swap(i, j); // make i<j
      MemlNode *a=null, *b; // these initializations are not needed but kept to avoid compile warnings
      Int index=0; MFREPD(node, T)
      {
         if(i==index)a=node;else // 'i' is 1st
         if(j==index)            // 'j' is 2nd (now we already have 'a')
         {
            b=node;

            if(_first==a)_first=b;
            if(_last ==b)_last =a;

            if(a->_prev)a->_prev->_next=b; // the one before 'a' now should point to 'b'
            if(b->_next)b->_next->_prev=a; // the one after  'b' now should point to 'a'

            if(a->_next==b) // watch out if 'a' and 'b' are neighbors
            {
               a->_next=b->_next;
               b->_prev=a->_prev;
               b->_next=a;
               a->_prev=b;
            }else
            {
               a->_next->_prev=b; // the one after  'a' now should point to 'b'
               b->_prev->_next=a; // the one before 'b' now should point to 'a'
               Swap(a->_prev, b->_prev);
               Swap(a->_next, b->_next);
            }
            return;
         }
         index++;
      }
   }
}
/******************************************************************************/
Bool _Meml::saveRaw(File &f)C
{
   f.cmpUIntV(elms()); MFREP(T)f.put(i->data(), elmSize());
   return f.ok();
}
Bool _Meml::loadRaw(File &f)
{
   setNum(f.decUIntV()); MFREP(T)f.getFast(i->data(), elmSize());
   if(f.ok())return true;
   clear(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
