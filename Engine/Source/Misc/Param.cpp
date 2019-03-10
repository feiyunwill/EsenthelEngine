/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Param::zero()
{
   type=PARAM_TYPE(0);
   enum_type=null;
   name   .clear();
   value.s.clear();
   value.b=0;
   value.i=0;
   value.f=0;
   value.v2.zero();
   value.v .zero();
   value.v4.zero();
   value.c .zero();
   value.id.zero();
}
Param::Param() {zero();}
/******************************************************************************/
Bool Param::asBool()C
{
   switch(type)
   {
      case PARAM_BOOL    : return value.b;
      case PARAM_INT     : return value.i!=0;
      case PARAM_FLT     : return value.f!=0;
      case PARAM_VEC2    : return value.v2.any();
      case PARAM_VEC     : return value.v .any();
      case PARAM_VEC4    : return value.v4.any();
      case PARAM_COLOR   : return value.c .any();
      case PARAM_STR     : return TextBool(value.s);
      case PARAM_ENUM    : return value.id.valid() || value.s.is();
      case PARAM_ID      : return value.id.valid();
      case PARAM_ID_ARRAY: return arrayIDs()>0;
      default            : return false;
   }
}
Int Param::asInt()C
{
   switch(type)
   {
      case PARAM_BOOL    : return value.b;
      case PARAM_INT     : return value.i;
      case PARAM_FLT     : return Round(value.f   );
      case PARAM_VEC2    : return Round(value.v2.x);
      case PARAM_VEC     : return Round(value.v .x);
      case PARAM_VEC4    : return Round(value.v4.x);
      case PARAM_COLOR   : return value.c.r;
      case PARAM_STR     : return TextInt(value.s);
      case PARAM_ENUM    : return asEnum();
    //case PARAM_ID      : return
    //case PARAM_ID_ARRAY: return
      default            : return 0;
   }
}
Flt Param::asFlt()C
{
   switch(type)
   {
      case PARAM_BOOL    : return value.b;
      case PARAM_INT     : return value.i;
      case PARAM_FLT     : return value.f;
      case PARAM_VEC2    : return value.v2.x;
      case PARAM_VEC     : return value.v .x;
      case PARAM_VEC4    : return value.v4.x;
      case PARAM_COLOR   : return value.c .r/255.0f;
      case PARAM_STR     : return TextFlt(value.s);
      case PARAM_ENUM    : return asEnum();
    //case PARAM_ID      : return
    //case PARAM_ID_ARRAY: return
      default            : return 0;
   }
}
Vec2 Param::asVec2()C
{
   switch(type)
   {
      case PARAM_BOOL    : return value.b    ;
      case PARAM_INT     : return value.i    ;
      case PARAM_FLT     : return value.f    ;
      case PARAM_VEC2    : return value.v2   ;
      case PARAM_VEC     : return value.v .xy;
      case PARAM_VEC4    : return value.v4.xy;
      case PARAM_COLOR   : return Vec2(value.c.r, value.c.g)/255.0f;
      case PARAM_STR     : return TextVec2(value.s);
    //case PARAM_ENUM    : return asEnum();
    //case PARAM_ID      : return
    //case PARAM_ID_ARRAY: return
      default            : return 0;
   }
}
Vec Param::asVec()C
{
   switch(type)
   {
      case PARAM_BOOL    : return         value.b         ;
      case PARAM_INT     : return         value.i         ;
      case PARAM_FLT     : return         value.f         ;
      case PARAM_VEC2    : return     Vec(value.v2, 0    );
      case PARAM_VEC     : return         value.v         ;
      case PARAM_VEC4    : return         value.v4.xyz    ;
      case PARAM_COLOR   : return         value.c .asVec();
      case PARAM_STR     : return TextVec(value.s        );
    //case PARAM_ENUM    : return asEnum();
    //case PARAM_ID      : return
    //case PARAM_ID_ARRAY: return
      default            : return 0;
   }
}
Vec4 Param::asVec4()C
{
   switch(type)
   {
      case PARAM_BOOL    : return          value.b        ;
      case PARAM_INT     : return          value.i        ;
      case PARAM_FLT     : return          value.f        ;
      case PARAM_VEC2    : return     Vec4(value.v2, 0, 0);
      case PARAM_VEC     : return     Vec4(value.v    , 0);
      case PARAM_VEC4    : return          value.v4       ;
      case PARAM_COLOR   : return          value.c .asVec4();
      case PARAM_STR     : return TextVec4(value.s);
    //case PARAM_ENUM    : return asEnum();
    //case PARAM_ID      : return
    //case PARAM_ID_ARRAY: return
      default            : return 0;
   }
}
Color Param::asColor()C
{
   switch(type)
   {
      case PARAM_BOOL    : return value.b ? WHITE : TRANSPARENT;
      case PARAM_INT     : return Color(value.i, value.i, value.i, 255);
      case PARAM_FLT     : return Color(FltToByte(value.f   ), FltToByte(value.f   ), FltToByte(value.f));
      case PARAM_VEC2    : return Color(FltToByte(value.v2.x), FltToByte(value.v2.y),                  0);
      case PARAM_VEC     : return value.v;
      case PARAM_VEC4    : return value.v4;
      case PARAM_COLOR   : return value.c;
      case PARAM_STR     : return TextColor(value.s);
    //case PARAM_ID      :
    //case PARAM_ID_ARRAY:
      default            : return TRANSPARENT;
   }
}
UID Param::asID()C
{
   switch(type)
   {
    //case PARAM_BOOL    : return UID(value.b, 0, 0, 0);
    //case PARAM_INT     : return UID(value.i, 0, 0, 0);
      case PARAM_ID      : return value.id;
      case PARAM_ID_ARRAY: if(arrayIDs()>=1)return *(UID*)value.s(); break;
      case PARAM_STR     : {UID id; id.fromText(value.s); return id;}
      case PARAM_ENUM    : return value.id;
   }
   return UIDZero;
}
UID Param::asID(Int id_index)C
{
   switch(type)
   {
      case PARAM_ID      : if(        id_index==0          )return        value.id            ; break;
      case PARAM_ID_ARRAY: if(InRange(id_index, arrayIDs()))return ((UID*)value.s())[id_index]; break;
   }
   return UIDZero;
}
UID Param::asIDMod(Int id_index)C
{
   switch(type)
   {
      case PARAM_ID      : return value.id;
      case PARAM_ID_ARRAY: if(Int ids=arrayIDs())return ((UID*)value.s())[Mod(id_index, ids)]; break;
   }
   return UIDZero;
}
Str Param::asText(Int precision)C
{
   switch(type)
   {
      default            : return S;
      case PARAM_BOOL    : return TextInt (value.b);
      case PARAM_INT     : return TextInt (value.i);
      case PARAM_FLT     : return TextReal(value.f, precision);
      case PARAM_VEC2    : return value.v2.asText(precision);
      case PARAM_VEC     : return value.v .asText(precision);
      case PARAM_VEC4    : return value.v4.asText(precision);
      case PARAM_COLOR   : return value.c .asText();
      case PARAM_STR     : return value.s;
      case PARAM_ENUM    : return value.s;
      case PARAM_ID      : return value.id.asHex();
      case PARAM_ID_ARRAY:
      {
         Str s; if(Int ids=arrayIDs())
         {
            s.reserve(ids*32 + (ids-1)); // 32=characters for one ID, (ids-1)=number of '\n' between ID's
            UID *id=(UID*)value.s();
            FREP(ids)s.line()+=id[i].asHex(); // list in order
         }
         return s;
      }
   }
}
Int Param::asEnum()C
{
   switch(type)
   {
      case PARAM_INT : return value.i;
      case PARAM_ENUM: if(enum_type){Int i=enum_type->find(value.id); if(i>=0)return i; i=enum_type->find(value.s); if(i>=0)return i;} break; // try ID first, then try Name
   }
   return -1;
}
/******************************************************************************/
Int Param::arrayIDs()C {return value.s.length()/(SIZE(UID)/SIZE(Char));} // !! only for PARAM_ID_ARRAY !!
Int Param::     IDs()C
{
   switch(type)
   {
      case PARAM_ID      : return 1;
      case PARAM_ID_ARRAY: return arrayIDs();
      default            : return 0;
   }
}
UInt Param::memUsage()C
{
   return value.s.memUsage();
}
Bool Param::hasID(C UID &id)C
{
   switch(type)
   {
      case PARAM_ID      : return value.id==id;
      case PARAM_ID_ARRAY:
      {
         UID *ids=(UID*)value.s();
         REP(arrayIDs())if(ids[i]==id)return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Param& Param::clearValue()
{
   switch(type)
   {
      case PARAM_BOOL    : value.b=0; break;
      case PARAM_INT     : value.i=0; break;
      case PARAM_FLT     : value.f=0; break;
      case PARAM_VEC2    : value.v2.zero (); break;
      case PARAM_VEC     : value.v .zero (); break;
      case PARAM_VEC4    : value.v4.zero (); break;
      case PARAM_COLOR   : value.c .zero (); break;
      case PARAM_ID      : value.id.zero (); break;
      case PARAM_ID_ARRAY: value.s .clear(); break;
      case PARAM_STR     : value.s .clear(); break;
      case PARAM_ENUM    : value.s .clear(); value.id.zero(); break;
   }
   return T;
}
Param& Param::setValue(Int i)
{
   switch(type)
   {
      case PARAM_BOOL    : value.b =(i!=0); break;
      case PARAM_INT     : value.i =i; break;
      case PARAM_FLT     : value.f =i; break;
      case PARAM_VEC2    : value.v2=i; break;
      case PARAM_VEC     : value.v =i; break;
      case PARAM_VEC4    : value.v4=i; break;
      case PARAM_STR     : value.s =i; break;
      case PARAM_COLOR   : value.c .set(i, i, i, 255); break;
    //case PARAM_ID      : value.id.set(i, 0, 0,   0); break; don't change
    //case PARAM_ID_ARRAY: break; don't change
      case PARAM_ENUM    :
      {
         if(enum_type && InRange(i, *enum_type))
         {
            value.s =(*enum_type)[i].name;
            value.id=(*enum_type)[i].id  ;
         }else
         {
            value.s .clear();
            value.id.zero();
         }
      }break;
   }
   return T;
}
Param& Param::setValue(C Str &s)
{
   switch(type)
   {
      case PARAM_BOOL    : value.b =TextBool (s); break;
      case PARAM_INT     : value.i =TextInt  (s); break;
      case PARAM_FLT     : value.f =TextFlt  (s); break;
      case PARAM_VEC2    : value.v2=TextVec2 (s); break;
      case PARAM_VEC     : value.v =TextVec  (s); break;
      case PARAM_VEC4    : value.v4=TextVec4 (s); break;
      case PARAM_STR     : value.s =          s ; break;
      case PARAM_COLOR   : value.c =TextColor(s); break;
      case PARAM_ID      : value.id.fromText (s); break;
      case PARAM_ID_ARRAY:
      { // this should handle the case when 's' is 'this.s'
         Memt<Str> lines; Split(lines, s, '\n');
         Memt<UID> ids  ; FREPA(lines){UID temp; if(temp.fromText(lines[i]))Swap(ids.New(), temp);}
         setAsIDArray(ids, false); // disallow changing type because we're in 'setValue' which can change only the value
      }break;

      case PARAM_ENUM:
      {
         value.s .clear();
         value.id.zero ();
         if(enum_type)
         {
            Int e=enum_type->find(s); if(InRange(e, *enum_type))
            {
               value.s =(*enum_type)[e].name;
               value.id=(*enum_type)[e].id  ;
            }
         }
      }break;
   }
   return T;
}
Param& Param::setValue(C Param &src)
{
   if(type==src.type)value=src.value;else switch(type)
   {
      case PARAM_BOOL : value.b =src.asBool (); break;
      case PARAM_INT  : value.i =src.asInt  (); break;
      case PARAM_FLT  : value.f =src.asFlt  (); break;
      case PARAM_VEC2 : value.v2=src.asVec2 (); break;
      case PARAM_VEC  : value.v =src.asVec  (); break;
      case PARAM_VEC4 : value.v4=src.asVec4 (); break;
      case PARAM_COLOR: value.c =src.asColor(); break;
      case PARAM_ID   : value.id=src.asID   (); break;

      case PARAM_ID_ARRAY: switch(src.type)
      {
         case PARAM_ID      : setAsIDArray(ConstCast(src.value.id), false); break; // disallow changing type because we're in 'setValue' which can change only the value
       //case PARAM_ID_ARRAY: this case is already handled in "type==src.type"
         case PARAM_STR     : setValue(src.value.s); break;
         default            : value.s.clear(); break;
      }break;

      case PARAM_STR: switch(src.type)
      {
         case PARAM_BOOL    : if(!src.value.b         )value.s.clear();else value.s=src.asText(); break;
         case PARAM_INT     : if(!src.value.i         )value.s.clear();else value.s=src.asText(); break;
         case PARAM_FLT     : if(!src.value.f         )value.s.clear();else value.s=src.asText(); break;
         case PARAM_VEC2    : if(!src.value.v2.any  ())value.s.clear();else value.s=src.asText(); break;
         case PARAM_VEC     : if(!src.value.v .any  ())value.s.clear();else value.s=src.asText(); break;
         case PARAM_VEC4    : if(!src.value.v4.any  ())value.s.clear();else value.s=src.asText(); break;
         case PARAM_COLOR   : if(!src.value.c .any  ())value.s.clear();else value.s=src.asText(); break;
         case PARAM_ID      : if(!src.value.id.valid())value.s.clear();else value.s=src.asText(); break;
         case PARAM_ID_ARRAY:                                               value.s=src.asText(); break; // 'asText' will already return an empty string if there are no ID's
         case PARAM_ENUM    : value.s=src.value.s; break;
       //case PARAM_STR     : value.s=src.value.s; break; this case is already handled in "type==src.type"
         default            : value.s.clear();     break;
      }break;

      case PARAM_ENUM: switch(src.type)
      {
         case PARAM_ENUM: value.s =src.value.s ; value.id=src.value.id; if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s =(*enum_type)[i].name;else{i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
         case PARAM_STR : value.s =src.value.s ; value.id.zero ();      if(enum_type){Int i=enum_type->find(value.s ); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id  ;} break;
         case PARAM_ID  : value.id=src.value.id; value.s .clear();      if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s =(*enum_type)[i].name;} break;
         default        : value.s.clear();       value.id.zero (); break;
      }break;
   }
   return T;
}
Param& Param::setType(PARAM_TYPE type, Enum *enum_type)
{
   if(InRange(type, PARAM_NUM))
      if(T.type!=type || (type==PARAM_ENUM && T.enum_type!=enum_type))
   {
      switch(type)
      {
         case PARAM_BOOL : value.b =asBool (); value.s.clear(); break;
         case PARAM_INT  : value.i =asInt  (); value.s.clear(); break;
         case PARAM_FLT  : value.f =asFlt  (); value.s.clear(); break;
         case PARAM_VEC2 : value.v2=asVec2 (); value.s.clear(); break;
         case PARAM_VEC  : value.v =asVec  (); value.s.clear(); break;
         case PARAM_VEC4 : value.v4=asVec4 (); value.s.clear(); break;
         case PARAM_COLOR: value.c =asColor(); value.s.clear(); break;
         case PARAM_ID   : value.id=asID   (); value.s.clear(); break;

         case PARAM_ID_ARRAY: switch(T.type)
         {
            case PARAM_ID : setAsIDArray(value.id, false); break; // disallow changing type because we're in 'setType' which assumes that 'type' will be set precisely
            case PARAM_STR: T.type=type; setValue(value.s); break; // change the type before calling 'setValue'
            default       : value.s.clear(); break;
         }break;

         case PARAM_STR: switch(T.type)
         {
            case PARAM_BOOL    : if(!value.b         )value.s.clear();else value.s=asText(); break;
            case PARAM_INT     : if(!value.i         )value.s.clear();else value.s=asText(); break;
            case PARAM_FLT     : if(!value.f         )value.s.clear();else value.s=asText(); break;
            case PARAM_VEC2    : if(!value.v2.any  ())value.s.clear();else value.s=asText(); break;
            case PARAM_VEC     : if(!value.v .any  ())value.s.clear();else value.s=asText(); break;
            case PARAM_VEC4    : if(!value.v4.any  ())value.s.clear();else value.s=asText(); break;
            case PARAM_COLOR   : if(!value.c .any  ())value.s.clear();else value.s=asText(); break;
            case PARAM_ID      : if(!value.id.valid())value.s.clear();else value.s=asText(); break;
            case PARAM_ID_ARRAY:                                           value.s=asText(); break; // 'asText' will already return an empty string if there are no ID's
            case PARAM_ENUM    :                  break; // keep 'value.s'
            default            : value.s.clear(); break;
         }break;

         case PARAM_ENUM:
         {
            T.enum_type=enum_type;
            switch(T.type)
            {
               case PARAM_ENUM:                   if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s =(*enum_type)[i].name;else{i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break; // keep 'value.s', 'value.id'
               case PARAM_STR : value.id.zero (); if(enum_type){Int i=enum_type->find(value.s ); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id  ;} break; // keep 'value.s'
               case PARAM_ID  : value.s .clear(); if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s =(*enum_type)[i].name;} break; // keep 'value.id'
               default        : value.s .clear(); value.id.zero(); break;
            }
         }break;
      }
         T.type=type; // change type at the end
      if(T.type!=PARAM_ENUM)T.enum_type=null;
   }
   return T;
}
/******************************************************************************/
Param& Param::setTypeValue(C Param &src)
{
   T.     type=src.     type;
   T.enum_type=src.enum_type;
   T.    value=src.    value;
   return T;
}
/******************************************************************************/
Param& Param::setAsIDArray(C MemPtr<UID> &ids, Bool allow_PARAM_ID_type)
{
   enum_type=null;
   value.s.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   if(allow_PARAM_ID_type && ids.elms()==1)
   {
      type=PARAM_ID; 
      value.id=ids[0];
   }else
   {
      type=PARAM_ID_ARRAY;
      if(ids.elms())
      {
         Int chars=ids.elms()*(SIZE(UID)/SIZE(Char));
         value.s.reserve(chars); ids.copyTo((UID*)value.s());
         value.s._d[value.s._length=chars]='\0'; // this is a string so it needs to have a nul terminate char
      }
   }
   return T;
}
Param& Param::includeAsIDArray(C MemPtr<UID> &ids, Bool allow_PARAM_ID_type)
{
   if(Int id_num=IDs()) // if have any existing ID's
   {
      Memt<UID> temp; temp.setNum(id_num);
      if(type==PARAM_ID)temp[0]=value.id;else temp.copyFrom((UID*)value.s());
      FREPA(ids)temp.include(ids[i]); // include in order
      return setAsIDArray(temp, allow_PARAM_ID_type);
   }
   return setAsIDArray(ids, allow_PARAM_ID_type);
}
/******************************************************************************/
Bool Param::save(File &f, CChar *path)C
{
   f.putMulti(Byte(5), type); // version
   if(type==PARAM_ENUM)f.putAsset(Enums.id(enum_type));
   f.putStr(name);
   switch(type)
   {
      case PARAM_BOOL    : f<<value.b ; break;
      case PARAM_INT     : f<<value.i ; break;
      case PARAM_FLT     : f<<value.f ; break;
      case PARAM_VEC2    : f<<value.v2; break;
      case PARAM_VEC     : f<<value.v ; break;
      case PARAM_VEC4    : f<<value.v4; break;
      case PARAM_COLOR   : f<<value.c ; break;
      case PARAM_ID      : f<<value.id; break;
      case PARAM_ENUM    : f<<value.id<<value.s; break;
      case PARAM_STR     : f<<value.s ; break;
      case PARAM_ID_ARRAY: {Int ids=arrayIDs(); f.cmpUIntV(ids); f.putN((UID*)value.s(), ids);} break;
   }
   return f.ok();
}
Bool Param::load(File &f, CChar *path)
{
   value.s.clear();
   switch(f.decUIntV()) // version
   {
      case 5:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f.getAssetID(), path);else enum_type=null;
         f.getStr(name);
         switch(type)
         {
            default            : goto error;
            case PARAM_BOOL    : f>>value.b ; break;
            case PARAM_INT     : f>>value.i ; break;
            case PARAM_FLT     : f>>value.f ; break;
            case PARAM_VEC2    : f>>value.v2; break;
            case PARAM_VEC     : f>>value.v ; break;
            case PARAM_VEC4    : f>>value.v4; break;
            case PARAM_COLOR   : f>>value.c ; break;
            case PARAM_ID      : f>>value.id; break;
            case PARAM_ENUM    : f>>value.id>>value.s; if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s=(*enum_type)[i].name;else{i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
            case PARAM_STR     : f>>value.s ; break;
            case PARAM_ID_ARRAY: if(UInt ids=f.decUIntV())
            {
               Int chars=ids*(SIZE(UID)/SIZE(Char));
               value.s.reserve(chars); f.getN((UID*)value.s(), ids);
               value.s._d[value.s._length=chars]='\0'; // this is a string so it needs to have a nul terminate char
            }break;
         }
         if(f.ok())return true;
      }break;

      case 4:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f.getAssetID(), path);else enum_type=null;
         f._getStr2(name);
         switch(type)
         {
            default            : goto error;
            case PARAM_BOOL    : f>>value.b ; break;
            case PARAM_INT     : f>>value.i ; break;
            case PARAM_FLT     : f>>value.f ; break;
            case PARAM_VEC2    : f>>value.v2; break;
            case PARAM_VEC     : f>>value.v ; break;
            case PARAM_VEC4    : f>>value.v4; break;
            case PARAM_COLOR   : f>>value.c ; break;
            case PARAM_ID      : f>>value.id; break;
            case PARAM_ENUM    : f>>value.id; f._getStr2(value.s); if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s=(*enum_type)[i].name;else{i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
            case PARAM_STR     : f._getStr2(value.s); break;
            case PARAM_ID_ARRAY: if(UInt ids=f.decUIntV())
            {
               Int chars=ids*(SIZE(UID)/SIZE(Char));
               value.s.reserve(chars); f.getN((UID*)value.s(), ids);
               value.s._d[value.s._length=chars]='\0'; // this is a string so it needs to have a nul terminate char
            }break;
         }
         if(f.ok())return true;
      }break;

      case 3:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f._getStr(), path);else enum_type=null;
         f._getStr(name);
         switch(type)
         {
            case PARAM_BOOL : f>>value.b ; break;
            case PARAM_INT  : f>>value.i ; break;
            case PARAM_FLT  : f>>value.f ; break;
            case PARAM_VEC2 : f>>value.v2; break;
            case PARAM_VEC  : f>>value.v ; break;
            case PARAM_VEC4 : f>>value.v4; break;
            case PARAM_COLOR: f>>value.c ; break;
            case PARAM_ID   : f>>value.id; if(!value.id.valid())type=PARAM_ID_ARRAY; break; // upgrade old file format's empty ID to PARAM_ID_ARRAY with no elements
            case PARAM_ENUM : f>>value.id; f._getStr(value.s); if(enum_type){Int i=enum_type->find(value.id); if(InRange(i, *enum_type))value.s=(*enum_type)[i].name;else{i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
            case PARAM_STR  :              f._getStr(value.s); break;
            default         : goto error;
         }
         if(f.ok())return true;
      }break;

      case 2:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f._getStr(), path);else enum_type=null;
         f._getStr(name);
         switch(type)
         {
            case PARAM_BOOL : f >>      value.b ; break;
            case PARAM_INT  : f >>      value.i ; break;
            case PARAM_FLT  : f >>      value.f ; break;
            case PARAM_VEC2 : f >>      value.v2; break;
            case PARAM_VEC  : f >>      value.v ; break;
            case PARAM_VEC4 : f >>      value.v4; break;
            case PARAM_COLOR: f >>      value.c ; break;
            case PARAM_STR  : f._getStr(value.s); break;
            case PARAM_ENUM : f._getStr(value.s); value.id.zero(); if(enum_type){Int i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;} break;
            default         : goto error;
         }
         if(f.ok())return true;
      }break;

      case 1:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f._getStr16(), path);else enum_type=null;
         name=f._getStr16();
         switch(type)
         {
            case PARAM_BOOL: f>>value.b              ; break;
            case PARAM_INT : f>>value.i              ; break;
            case PARAM_FLT : f>>value.f              ; break;
            case PARAM_STR :    value.s=f._getStr16(); break;
            case PARAM_ENUM:    value.s=f._getStr16(); value.id.zero(); if(enum_type){Int i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;} break;
            default        : goto error;
         }
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>type; if(type==PARAM_ENUM)enum_type=Enums(f._getStr8(), path);else enum_type=null;
         name=f._getStr8();

         switch(type)
         {
            case PARAM_BOOL: f>>value.b; break;
            case PARAM_INT : f>>value.i; break;
            case PARAM_FLT : f>>value.f; break;
            case PARAM_STR : {Char8 str[256]; f>>str; value.s=str;} break;
            case PARAM_ENUM: {Char8 str[256]; f>>str; value.s=str; value.id.zero(); if(enum_type){Int i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
            default        : goto error;
         }
         if(f.ok())return true;
      }break;
   }
error:
   zero(); return false;
}
void Param::loadOld(File &f)
{
   value.s.clear();
   f>>type;
   Char8 name[40]; f>>name; T.name=name;
   if(type==PARAM_ENUM){Char8 name[256]; f>>name; enum_type=Enums(name);}else enum_type=null;
   switch(type)
   {
      case PARAM_BOOL: f>>value.b; break;
      case PARAM_INT : f>>value.i; break;
      case PARAM_FLT : f>>value.f; break;
      case PARAM_STR : {Char8 str[256]; f>>str; value.s=str;} break;
      case PARAM_ENUM: {Char8 str[256]; f>>str; value.s=str; value.id.zero(); if(enum_type){Int i=enum_type->find(value.s); if(InRange(i, *enum_type))value.id=(*enum_type)[i].id;}} break;
   }
   if(f.ok())return;
   zero();
}
/******************************************************************************/
Int CompareValue(C Param &p0, C Param &p1)
{
   if(Int i=Compare(p0.type, p1.type))return i; // type
   switch(p0.type) // value
   {
      case PARAM_BOOL    : return Compare(p0.value.b , p1.value.b );
      case PARAM_INT     : return Compare(p0.value.i , p1.value.i );
      case PARAM_FLT     : return Compare(p0.value.f , p1.value.f );
      case PARAM_VEC2    : return Compare(p0.value.v2, p1.value.v2);
      case PARAM_VEC     : return Compare(p0.value.v , p1.value.v );
      case PARAM_VEC4    : return Compare(p0.value.v4, p1.value.v4);
      case PARAM_COLOR   : return Compare(p0.value.c , p1.value.c );
      case PARAM_STR     : return Compare(p0.value.s , p1.value.s );
      case PARAM_ID      : return Compare(p0.value.id, p1.value.id);
      case PARAM_ID_ARRAY:
      {
         Int ids0=p0.arrayIDs(), ids1=p1.arrayIDs(), ids=Min(ids0, ids1);
         FREP(ids)if(Int c=Compare(p0.asID(i), p1.asID(i)))return c;
         return Compare(ids0, ids1);
      }break;

      case PARAM_ENUM:
      {
         if(p0.enum_type && p1.enum_type)
         {
            if(p0.enum_type!=p1.enum_type)return Compare(p0.enum_type->name, p1.enum_type->name);
         }else
         if(p0.enum_type)return +1;else
         if(p1.enum_type)return -1;
      }return Compare(p0.value.s, p1.value.s);
   }
   return 0;
}
Int Compare(C Param &p0, C Param &p1)
{
   if(Int i=Compare(p0.name, p1.name))return i; // name
   return CompareValue(p0, p1);
}
/******************************************************************************/
}
/******************************************************************************/
