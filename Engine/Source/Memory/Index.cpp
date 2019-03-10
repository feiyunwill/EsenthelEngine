/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void IndexGroup   ::changeVal(Int from, Int to) {REP(           num  )if(elm[i]==from ) elm[i]=to;}
void IndexPtrGroup::changeVal(Ptr from, Ptr to) {REP(           num  )if(elm[i]==from ) elm[i]=to;}
void IndexGroup   ::   remVal(Int value       ) {for(Int i=0; i<num; )if(elm[i]==value) elm[i]=elm[--num];        else i++;}
void IndexPtrGroup::   remVal(Ptr value       ) {for(Int i=0; i<num; )if(elm[i]==value) elm[i]=elm[--num];        else i++;}
void IndexGroup   ::   subVal(Int value       ) {for(Int i=0; i<num; )if(elm[i]==value){elm[i]=elm[--num]; break;}else i++;}
void IndexPtrGroup::   subVal(Ptr value       ) {for(Int i=0; i<num; )if(elm[i]==value){elm[i]=elm[--num]; break;}else i++;}
void IndexGroup   ::   subElm(Int i           ) {if (InRange(i, num) )                  elm[i]=elm[--num];}
void IndexPtrGroup::   subElm(Int i           ) {if (InRange(i, num) )                  elm[i]=elm[--num];}
/******************************************************************************/
Index& Index::del()
{
   Free(group);
   Free(groups_elm);
   Free(elm_group);
   Zero(T); return T;
}
IndexPtr& IndexPtr::del()
{
   Free(group);
   Free(groups_elm);
   Zero(T); return T;
}
Index& Index::create(Int groups, Int elms_max)
{
   del();

   AllocZero(group     , groups  );
   Alloc    (groups_elm, elms_max);
   Alloc    ( elm_group, elms_max);
   T.groups  =groups;
   T.elms_max=elms_max;

   return T;
}
IndexPtr& IndexPtr::create(Int groups)
{
   del();

   AllocZero(group, groups);
   T.groups=groups;

   return T;
}
Index& Index::reset()
{
   ZeroN(group, groups);
   if(!elms_max)
   {
      Free(elm_group);
      Free(groups_elm);
   }
   elms=group_elms_max=0;
   return T;
}
IndexPtr& IndexPtr::reset()
{
   ZeroN(group, groups);
   Free (groups_elm);
   elms=group_elms_max=0;
   return T;
}
Index& Index::set(Int *elm_group)
{
   // create groups_elm array
   if(!groups_elm)
   {
      Int elms=0; IndexGroup *ig=group;
      REP(groups)elms+=(ig++)->num;
      Alloc(groups_elm, elms);
   }

   // set group element offsets
   IndexGroup *ig=group;
   elms=0; REP(groups)
   {
      if(ig->num>group_elms_max)group_elms_max=ig->num;
            ig->elm=&groups_elm[elms];
      elms+=ig->num;
            ig->num=0;
            ig++;
   }

   // optionally fill group_elm array
   if(!elm_group)elm_group=T.elm_group;
   if( elm_group)FREP(elms)addElm(*elm_group++, i);
   return T;
}
IndexPtr& IndexPtr::set()
{
   // create group_elm array
   if(!groups_elm)
   {
      Int elms=0; IndexPtrGroup *ig=group;
      REP(groups)elms+=(ig++)->num;
      Alloc(groups_elm, elms);
   }

   // set group element offsets
   IndexPtrGroup *ig=group;
   elms=0; REP(groups)
   {
      if(ig->num>group_elms_max)group_elms_max=ig->num;
            ig->elm=&groups_elm[elms];
      elms+=ig->num;
            ig->num=0;
            ig++;
   }
   return T;
}
/******************************************************************************/
Bool  ElmIs(C MemPtr<Bool> &is, Int i) {return InRange(i, is) && is[i];}
Int CountIs(C MemPtr<Bool> &is)
{
   Int    n=0; REPA(is)n+=is[i];
   return n;
}
void CreateIs(MemPtr<Bool> is, C MemPtr<Int> &selection, Int elms)
{
   is.setNum(elms); REPAO(is)=false; REPA(selection){Int index=selection[i]; if(InRange(index, is))is[index]=true;}
}
void CreateIsNot(MemPtr<Bool> is, C MemPtr<Int> &selection, Int elms)
{
   is.setNum(elms); REPAO(is)=true; REPA(selection){Int index=selection[i]; if(InRange(index, is))is[index]=false;}
}
void CreateFaceIs(MemPtr<Bool> tri_is, MemPtr<Bool> quad_is, C MemPtr<Int> &faces, Int tris, Int quads)
{
    tri_is.setNum( tris); REPAO( tri_is)=false;
   quad_is.setNum(quads); REPAO(quad_is)=false;
   REPA(faces)
   {
      UInt f=faces[i];
      if(f&SIGN_BIT){f^=SIGN_BIT; if(InRange(f, quad_is))quad_is[f]=true;}
      else          {             if(InRange(f,  tri_is)) tri_is[f]=true;}
   }
}
void CreateFaceIsNot(MemPtr<Bool> tri_is, MemPtr<Bool> quad_is, C MemPtr<Int> &faces, Int tris, Int quads)
{
    tri_is.setNum( tris); REPAO( tri_is)=true;
   quad_is.setNum(quads); REPAO(quad_is)=true;
   REPA(faces)
   {
      UInt f=faces[i];
      if(f&SIGN_BIT){f^=SIGN_BIT; if(InRange(f, quad_is))quad_is[f]=false;}
      else          {             if(InRange(f,  tri_is)) tri_is[f]=false;}
   }
}
/******************************************************************************/
void SetFaceIndex(Ptr data, C VecI *tri, Int tris, C VecI4 *quad, Int quads, Bool bit16)
{
   if(bit16)
   {
      Copy32To16(data, tri, tris*3);
      VecUS *t=(VecUS*)data+tris; REP(quads){(t++)->set(quad->x, quad->y, quad->w); (t++)->set(quad->w, quad->y, quad->z); quad++;}
   }else
   {
      Copy32To32(data, tri, tris*3);
      VecI *t=(VecI*)data+tris; REP(quads){(t++)->set(quad->x, quad->y, quad->w); (t++)->set(quad->w, quad->y, quad->z); quad++;}
   }
}
/******************************************************************************/
void SetRemap(MemPtr<Int> remap, C MemPtr<Bool> &is, Int elms)
{
   remap.setNum(elms);
   Int full=Min(remap.elms(), is.elms()), i=0, index=0;
   for(; i<      full  ; i++)remap[i]=(is[i] ? index++ : -1);
   for(; i<remap.elms(); i++)remap[i]=                   -1 ;
}
void IndRemap(C MemPtr<Int> &remap, Int *ind, Int elms)
{
   for(; elms-->0; ind++)*ind=(InRange(*ind, remap) ? remap[*ind] : -1);
}
/******************************************************************************/
Bool IndSave(File &f, CPtr ind, Int inds, Int elms)
{
   Int bytes=4;
   if(elms>0)
   {
      if(elms<=0x00000100)bytes=1;else
      if(elms<=0x00010000)bytes=2;else
      if(elms<=0x01000000)bytes=3;
   }
   Int        size=inds*bytes;
   CPtr       data=ind;
   Memt<Byte> temp;
   if(bytes!=4)
   {
      data=temp.setNum(size).data();
      switch(bytes)
      {
         case 3: Copy32To24(temp.data(), ind, inds); break;
         case 2: Copy32To16(temp.data(), ind, inds); break;
         case 1: Copy32To8 (temp.data(), ind, inds); break;
      }
   }
   f.putByte(bytes*8); // save as bits
   f.put(data, size);
   return f.ok();
}
Bool IndLoad(File &f, Ptr ind, Int inds)
{
   Int        bytes=f.getByte()/8,
              size=inds*bytes;
   Ptr        data=ind;
   Memt<Byte> temp; if(bytes!=4)data=temp.setNum(size).data();
   f.getFast(data, size);
   switch(bytes)
   {
      case  4: break;
      case  3: Copy24To32(ind, data, inds); break;
      case  2: Copy16To32(ind, data, inds); break;
      case  1: Copy8To32 (ind, data, inds); break;
      default: return false;
   }
   return f.ok();
}
/******************************************************************************/
}
/******************************************************************************/
