/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   '_order' is of length "elms()*2" because it stores indexes sorted by name+id (2 in total per each element)

/******************************************************************************/
#define CC4_ENUM            CC4('E','N','U','M')
#define ENUM_CASE_SENSITIVE false
/******************************************************************************/
Cache<Enum> Enums("Enum");
/******************************************************************************/
Enum::Enum(C Enum &src) : Enum() {T=src;}
Enum& Enum::del()
{
   name[0]=0;
   Free(_order);
  _elms.del();
   return T;
}
/******************************************************************************/
struct EnumElmPtr
{
 C Enum::Elm *elm;
   UInt       index;

   void set(C Enum::Elm &elm, UInt index) {T.elm=&elm; T.index=index;}

   static Int CompareName(C EnumElmPtr &ep0, C EnumElmPtr &ep1) {return Compare(ep0.elm->name, ep1.elm->name, ENUM_CASE_SENSITIVE);}
   static Int CompareID  (C EnumElmPtr &ep0, C EnumElmPtr &ep1) {return Compare(ep0.elm->id  , ep1.elm->id                       );}
};
void Enum::sort()
{
   Mems<EnumElmPtr> elm_ptr; elm_ptr.setNum(elms().elms()); REPAO(elm_ptr).set(T[i], i);
   elm_ptr.sort(EnumElmPtr::CompareName); UInt *name_order=nameOrder(); REPA(T)name_order[i]=elm_ptr[i].index;
   elm_ptr.sort(EnumElmPtr::CompareID  ); UInt *  id_order=  idOrder(); REPA(T)  id_order[i]=elm_ptr[i].index;
}
Enum& Enum::create(C Str &name, C MemPtr<Elm> &elms)
{
   del();

   Set(T.name, name);
   T._elms=elms;
   Alloc(_order, T.elms().elms()*2);
   sort();

   return T;
}
Enum& Enum::operator=(C Enum &src)
{
   if(this!=&src)
   {
      Set(name, src.name);
     _elms=src._elms;
      CopyN(Alloc(Free(_order), elms().elms()*2), src._order, elms().elms()*2);
   }
   return T;
}
/******************************************************************************/
Int Enum::find(CChar8 *name)C
{
   if(Is(name))
   {
   #if 0
      REPA(T)if(Equal(T[i].name, name, ENUM_CASE_SENSITIVE))return i;
   #else
      UInt *name_order=nameOrder();
      for(Int l=0, r=elms().elms(); l<r; )
      {
         Int mid=UInt(l+r)/2,
             compare=Compare(name, T[name_order[mid]].name, ENUM_CASE_SENSITIVE);
         if(!compare  )return        name_order[mid];
         if( compare<0)r=mid;
         else          l=mid+1;
      }
   #endif
   }
   return -1;
}
Int Enum::find(CChar *name)C
{
   if(Is(name))
   {
   #if 0
      REPA(T)if(Equal(T[i].name, name, ENUM_CASE_SENSITIVE))return i;
   #else
      UInt *name_order=nameOrder();
      for(Int l=0, r=elms().elms(); l<r; )
      {
         Int mid=UInt(l+r)/2,
             compare=Compare(name, T[name_order[mid]].name, ENUM_CASE_SENSITIVE);
         if(!compare  )return        name_order[mid];
         if( compare<0)r=mid;
         else          l=mid+1;
      }
   #endif
   }
   return -1;
}
Int Enum::find(C UID &id)C
{
   if(id.valid())
   {
      UInt *id_order=idOrder();
      for(Int l=0, r=elms().elms(); l<r; )
      {
         Int mid=UInt(l+r)/2,
             compare=Compare(id, T[id_order[mid]].id);
         if(!compare  )return      id_order[mid];
         if( compare<0)r=mid;
         else          l=mid+1;
      }
   }
   return -1;
}
Int Enum::findByIDUInt(UInt id)C
{
   if(id)
   {
      UInt *id_order=idOrder();
      for(Int l=0, r=elms().elms(); l<r; )
      {
         Int mid=UInt(l+r)/2,
             compare=Compare(id, T[id_order[mid]].id.i[3]); // use 3rd UInt because UID's are sorted by most significant first
         if(!compare  )return      id_order[mid];
         if( compare<0)r=mid;
         else          l=mid+1;
      }
   }
   return -1;
}
Int Enum::operator()(CChar8 *name)C {Int i=find(name); if(i<0)Exit(S+"Element \""+name      +"\" not found in enum \""+Enums.name(this)+"\""); return i;}
Int Enum::operator()(CChar  *name)C {Int i=find(name); if(i<0)Exit(S+"Element \""+name      +"\" not found in enum \""+Enums.name(this)+"\""); return i;}
Int Enum::operator()(C UID  &id  )C {Int i=find(id  ); if(i<0)Exit(S+"Element \""+id.asHex()+"\" not found in enum \""+Enums.name(this)+"\""); return i;}

CChar8* Enum::elmName  (Int i)C {return InRange(i, T) ? T[i].name    : null   ;}
C UID & Enum::elmID    (Int i)C {return InRange(i, T) ? T[i].id      : UIDZero;}
UInt    Enum::elmIDUInt(Int i)C {return InRange(i, T) ? T[i].id.i[3] : 0      ;} // use 3rd UInt because UID's are sorted by most significant first
/******************************************************************************/
Bool Enum::save(File &f)C
{
   f.putMulti(UInt(CC4_ENUM), Byte(3)); // version
   f.putStr  (name);
   f.cmpUIntV(elms().elms()); FREPA(T){C Elm &elm=_elms[i]; f.putStr(elm.name)<<elm.id;}
   f.putN    (_order, elms().elms()*2);
   return f.ok();
}
Bool Enum::load(File &f)
{
   del();
   if(f.getUInt()==CC4_ENUM)switch(f.decUIntV()) // version
   {
      case 3:
      {
         f.getStr(name);
        _elms.setNum(f.decUIntV()); FREPA(T){Elm &elm=_elms[i]; f.getStr(elm.name)>>elm.id;}
         f.getN(Alloc(_order, elms().elms()*2), elms().elms()*2);
         if(f.ok())return true;
      }break;

      case 2:
      {
         f._getStr2(name);
        _elms.setNum(f.decUIntV()); FREPA(T){Elm &elm=_elms[i]; f._getStr2(elm.name)>>elm.id;}
         f.getN(Alloc(_order, elms().elms()*2), elms().elms()*2);
         if(f.ok())return true;
      }break;

      case 1:
      {
         Set(name, f._getStr());
        _elms.setNum(f.decUIntV()); FREPA(T){Elm &elm=_elms[i]; Set(elm.name, f._getStr()); f>>elm.id;}
         f.getN(Alloc(_order, elms().elms()*2), elms().elms()*2);
         if(f.ok())return true;
      }break;

      case 0:
      {
         f.getByte(); // old version byte
         f>>name;
        _elms.setNum(f.getInt()); FREPA(T){Elm &elm=_elms[i]; f>>elm.name; elm.id=MD5Mem(Str8(elm.name), Length(elm.name));}
         f.skip(SIZE(UInt)*elms().elms()); // skip old '_order'
         if(f.ok())
         {
            Alloc(_order, elms().elms()*2);
            sort();
            return true;
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
Bool Enum::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Enum::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
Bool Enum::saveH(C Str &name)C
{
   FileText f; if(f.write(name, UTF_8)) // use UTF-8 Encoding because file may get used on Mac XCode GCC which doesn't handle UTF-16 properly
   {
      f.putLine("/******************************************************************************/");
      f.putLine(S+"enum "+T.name);
      f.putLine('{').depth++;
      FREPA(T)f.putLine(S+T[i].name+",");
      f.depth--;  f.putLine("};");
      f.putLine("/******************************************************************************/");
      return true;
   }
   return false;
}
/******************************************************************************/
void Enum::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load enum \""+name+"\"");}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutEnum() {Enums.del();}
/******************************************************************************/
}
/******************************************************************************/
