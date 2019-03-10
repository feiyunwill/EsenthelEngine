/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define  ELM(i        ) data[i]
#define COPY(dest, src) dest=src
#define SWAP(a, b     ) Swap(ELM(a), ELM(b))
T1(TYPE) static INLINE void _Sort(TYPE *data, Int elms)
{
   if(elms>1)
   {
      Memt<VecI2> stack; stack.New().set(0, elms-1);
      for(; stack.elms(); )
      {
         VecI2 lr=stack.pop();
         Int   l =lr.x,
               r =lr.y;
         if(r-l>48) // 48 gave best results
         {
            // quicksort
            Int m=UInt(r+l)/2;
            if(ELM(l)>ELM(r))SWAP(l, r);
            if(ELM(l)>ELM(m))SWAP(l, m);
            if(ELM(m)>ELM(r))SWAP(m, r);

            Int  i=l, j=r;
            TYPE temp=ELM(m);
            for(; i<=j; )
            {
               for(; ELM(i)<temp; )i++; // find first element from left  that is larger  or equal to 'temp'
               for(; ELM(j)>temp; )j--; // find first element from right that is smaller or equal to 'temp'
               if(i<=j)
               {
                  SWAP(i, j);
                  i++;
                  j--;
               }
            }
            if(l<j)stack.New().set(l, j);
            if(i<r)stack.New().set(i, r);
         }else
         {
            // insertion sort
            for(Int i=l+1; i<=r; i++)
            {
               TYPE temp=ELM(i);
            #if 1 // faster
               Int j; for(j=i; j>l && ELM(j-1)>temp; j--)ELM(j)=ELM(j-1);
               COPY(ELM(j), temp);
            #else
               Int j; for(j=i; j>l && ELM(j-1)>temp; j--);
              _MoveElmLeftUnsafe(data, SIZE(TYPE), i, j, &temp);
            #endif
            }
         }
      }
   }
}
#undef SWAP
#undef COPY
#undef ELM
void Sort(Int *data, Int elms) {_Sort(data, elms);}
void Sort(Flt *data, Int elms) {_Sort(data, elms);}
void Sort(Dbl *data, Int elms) {_Sort(data, elms);}
/******************************************************************************/
#define  ELM(i        ) (((Byte*)data)+(i)*elm_size)
#define COPY(dest, src) CopyFast(dest, src, elm_size)
#define SWAP(a, b     ) SwapFast(ELM(a), ELM(b), elm_size)
void _Sort(Ptr data, Int elms, Int elm_size, Int compare(CPtr a, CPtr b))
{
   if(elms>1)
   {
      // init
      Memt<Byte >   buf; buf.setNum(elm_size); Ptr temp=buf.data();
      Memt<VecI2> stack; stack.New().set(0, elms-1);
      for(; stack.elms(); )
      {
         VecI2 lr=stack.pop();
         Int   l =lr.x,
               r =lr.y;
         if(r-l>16) // 16 gave the best result
         {
            // quicksort
            Int m=UInt(r+l)/2;
            if(compare(ELM(l), ELM(r))>0)SWAP(l, r);
            if(compare(ELM(l), ELM(m))>0)SWAP(l, m);
            if(compare(ELM(m), ELM(r))>0)SWAP(m, r);

            Int i=l, j=r;
            COPY(temp, ELM(m)); 
            for(; i<=j; )
            {
               for(; compare(ELM(i), temp)<0; )i++; // find first element from left  that is larger  or equal to 'temp'
               for(; compare(ELM(j), temp)>0; )j--; // find first element from right that is smaller or equal to 'temp'
               if(i<=j)
               {
                  SWAP(i, j);
                  i++;
                  j--;
               }
            }
            if(l<j)stack.New().set(l, j);
            if(i<r)stack.New().set(i, r);
         }else
         {
            // insertion sort
            for(Int i=l+1; i<=r; i++)
            {
            #if 0 // slower
               COPY(temp, ELM(i));
               Int j; for(j=i; j>l && compare(ELM(j-1), temp)>0; j--)COPY(ELM(j), ELM(j-1));
               COPY(ELM(j), temp);
            #else
               CPtr test=ELM(i);
               Int j; for(j=i; j>l && compare(ELM(j-1), test)>0; j--);
              _MoveElmLeftUnsafe(data, elm_size, i, j, temp);
            #endif
            }
         }
      }
   }
}
#undef  ELM
#define ELM(i) memb[i]
void _Sort(_Memb &memb, Int compare(CPtr a, CPtr b))
{
   Int elm_size=memb.elmSize();

   if(memb.elms()>1)
   {
      // init
      Memt<Byte >   buf; buf.setNum(elm_size); Ptr temp=buf.data();
      Memt<VecI2> stack; stack.New().set(0, memb.elms()-1);
      for(; stack.elms(); )
      {
         VecI2 lr=stack.pop();
         Int   l =lr.x,
               r =lr.y;
         if(r-l>16) // 16 gave the best result
         {
            // quicksort
            Int m=UInt(r+l)/2;
            if(compare(ELM(l), ELM(r))>0)SWAP(l, r);
            if(compare(ELM(l), ELM(m))>0)SWAP(l, m);
            if(compare(ELM(m), ELM(r))>0)SWAP(m, r);

            Int i=l, j=r;
            COPY(temp, ELM(m)); 
            for(; i<=j; )
            {
               for(; compare(ELM(i), temp)<0; )i++; // find first element from left  that is larger  or equal to 'temp'
               for(; compare(ELM(j), temp)>0; )j--; // find first element from right that is smaller or equal to 'temp'
               if(i<=j)
               {
                  SWAP(i, j);
                  i++;
                  j--;
               }
            }
            if(l<j)stack.New().set(l, j);
            if(i<r)stack.New().set(i, r);
         }else
         {
            // insertion sort
            for(Int i=l+1; i<=r; i++)
            {
            #if 0 // slower
               COPY(temp, ELM(i));
               Int j; for(j=i; j>l && compare(ELM(j-1), temp)>0; j--)COPY(ELM(j), ELM(j-1));
               COPY(ELM(j), temp);
            #else
               CPtr test=ELM(i);
               Int j; for(j=i; j>l && compare(ELM(j-1), test)>0; j--);
               memb.moveElmLeftUnsafe(i, j, temp);
            #endif
            }
         }
      }
   }
}
#undef  COPY
#undef   ELM
#define  ELM(i   ) memx[i]
#undef  SWAP
#define SWAP(a, b) memx.swapOrder(a, b)
void _Sort(_Memx &memx, Int compare(CPtr a, CPtr b))
{
   if(memx.elms()>1)
   {
      Memt<VecI2> stack; stack.New().set(0, memx.elms()-1);
      for(; stack.elms(); )
      {
         VecI2 lr=stack.pop();
         Int   l =lr.x,
               r =lr.y;
         if(r-l>16) // 16 gave the best result
         {
            // quicksort
            Int m=UInt(r+l)/2;
            if(compare(ELM(l), ELM(r))>0)SWAP(l, r);
            if(compare(ELM(l), ELM(m))>0)SWAP(l, m);
            if(compare(ELM(m), ELM(r))>0)SWAP(m, r);

            Int i=l, j=r;
            Ptr temp=ELM(m); // we can use this because address remains the same
            for(; i<=j; )
            {
               for(; compare(ELM(i), temp)<0; )i++; // find first element from left  that is larger  or equal to 'temp'
               for(; compare(ELM(j), temp)>0; )j--; // find first element from right that is smaller or equal to 'temp'
               if(i<=j)
               {
                  SWAP(i, j);
                  i++;
                  j--;
               }
            }
            if(l<j)stack.New().set(l, j);
            if(i<r)stack.New().set(i, r);
         }else
         {
            // insertion sort
            for(Int i=l+1; i<=r; i++)
            {
            #if 0 // slower
               Ptr temp=ELM(i); // we can use this because address remains the same
               Int j; for(j=i; j>l && compare(ELM(j-1), temp)>0; j--)SWAP(j, j-1);
            #else
               CPtr test=ELM(i);
               Int j; for(j=i; j>l && compare(ELM(j-1), test)>0; j--);
               memx.moveElmLeftUnsafe(i, j);
            #endif
            }
         }
      }
   }
}
#undef   ELM
#define  ELM(i   ) meml[i]
#undef  SWAP
#define SWAP(a, b) meml.swapOrder(a, b)
void _Sort(_Meml &meml, Int compare(CPtr a, CPtr b)) // TODO: this is slow for Meml
{
   if(meml.elms()>1)
   {
      Memt<VecI2> stack; stack.New().set(0, meml.elms()-1);
      for(; stack.elms(); )
      {
         VecI2 lr=stack.pop();
         Int   l =lr.x,
               r =lr.y;
         if(r-l>16) // 16 gave the best result
         {
            // quicksort
            Int m=UInt(r+l)/2;
            if(compare(ELM(l), ELM(r))>0)SWAP(l, r);
            if(compare(ELM(l), ELM(m))>0)SWAP(l, m);
            if(compare(ELM(m), ELM(r))>0)SWAP(m, r);

            Int i=l, j=r;
            Ptr temp=ELM(m); // we can use this because address remains the same
            for(; i<=j; )
            {
               for(; compare(ELM(i), temp)<0; )i++; // find first element from left  that is larger  or equal to 'temp'
               for(; compare(ELM(j), temp)>0; )j--; // find first element from right that is smaller or equal to 'temp'
               if(i<=j)
               {
                  SWAP(i, j);
                  i++;
                  j--;
               }
            }
            if(l<j)stack.New().set(l, j);
            if(i<r)stack.New().set(i, r);
         }else
         {
            // insertion sort
            for(Int i=l+1; i<=r; i++)
            {
               Ptr temp=ELM(i); // we can use this because address remains the same
               Int j; for(j=i; j>l && compare(ELM(j-1), temp)>0; j--)SWAP(j, j-1);
            }
         }
      }
   }
}
#undef SWAP
#undef ELM
/******************************************************************************/
Bool _BinarySearch(CPtr data, Int elms, Int elm_size, CPtr value, Int &index, Int compare(CPtr a, CPtr b))
{
   Int l=0, r=elms; for(; l<r; )
   {
      Int mid=UInt(l+r)/2,
          c  =compare(((Byte*)data)+mid*elm_size, value);
      if( c<0)l=mid+1;else
      if( c>0)r=mid  ;else {index=mid; return true;}
   }
   index=l; return false;
}
Bool _BinarySearch(C _Memb &data, CPtr value, Int &index, Int compare(CPtr a, CPtr b))
{
   Int l=0, r=data.elms(); for(; l<r; )
   {
      Int mid=UInt(l+r)/2,
          c  =compare(data[mid], value);
      if( c<0)l=mid+1;else
      if( c>0)r=mid  ;else {index=mid; return true;}
   }
   index=l; return false;
}
Bool _BinarySearch(C _Memx &data, CPtr value, Int &index, Int compare(CPtr a, CPtr b))
{
   Int l=0, r=data.elms(); for(; l<r; )
   {
      Int mid=UInt(l+r)/2,
          c  =compare(data[mid], value);
      if( c<0)l=mid+1;else
      if( c>0)r=mid  ;else {index=mid; return true;}
   }
   index=l; return false;
}
Bool _BinarySearch(C _Meml &data, CPtr value, Int &index, Int compare(CPtr a, CPtr b))
{
   Int pos=0; MemlNode *node=data.first(); // get first node
   Int l=0, r=data.elms(); for(; l<r; )
   {
      Int mid=UInt(l+r)/2;
      for(; pos<mid; pos++)node=node->next(); // go forward
      for(; pos>mid; pos--)node=node->prev(); // go back
      Int c=compare(node->data(), value);
      if( c<0)l=mid+1;else
      if( c>0)r=mid  ;else {index=mid; return true;}
   }
   index=l; return false;
}
/******************************************************************************/
}
/******************************************************************************/
