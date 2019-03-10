/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Bool DataIsImage(DATA_TYPE type)
{
   switch(type)
   {
      case DATA_IMAGE    :
      case DATA_IMAGE_PTR:
      case DATA_IMAGEPTR :
         return true;
   }
   return false;
}
Bool DataIsText(DATA_TYPE type)
{
   switch(type)
   {
      case DATA_CHAR8    :
      case DATA_CHAR8_PTR:
      case DATA_CHAR     :
      case DATA_CHAR_PTR :
      case DATA_STR8     :
      case DATA_STR      :
         return true;
   }
   return false;
}
Flt DataAlign(DATA_TYPE type)
{
   switch(type)
   {
      case DATA_PTR  :
      case DATA_BOOL :
      case DATA_INT  :
      case DATA_UINT :
      case DATA_REAL :
      case DATA_VEC2 :
      case DATA_VEC  :
      case DATA_VEC4 :
      case DATA_VECI2:
      case DATA_VECI :
      case DATA_VECI4:
      case DATA_VECU2:
      case DATA_VECU :
      case DATA_VECU4:
      case DATA_KBSC :
         return -1;
   }
   return +1;
}
C ImagePtr& DataGuiImage(CPtr data, C ListColumn &list_col, Color &color)
{
   if(data)
   {
      CPtr member=(Byte*)data+list_col.md.offset;
      switch(list_col.md.type)
      {
         case DATA_MENU_PTR:
         {
            if(*(Menu**)member)
               if(GuiSkin *skin=Gui.skin())
                  {color=skin->menu.sub_menu_color; return skin->menu.sub_menu;}
         }break;

         case DATA_CHECK:
         {
            Bool on=*(Bool*)member;
            if(list_col.sort) // if we know information about 'MenuElm._flag'
            {
               if(list_col.sort->asInt(data)&MENU_TOGGLABLE)
                  if(GuiSkin *skin=Gui.skin())
                     {color=skin->menu.check_color; if(!on)color.a=((color.a*32)>>8); return skin->menu.check;}
            }else
            {
               if(on)
                  if(GuiSkin *skin=Gui.skin())
                     {color=skin->menu.check_color; return skin->menu.check;}
            }
         }break;
      }
   }
   return ImageNull;
}
static Int CompareImage(Image *a, Image *b)
{
   if(Int c=Compare(a ? a->size3() : VecI(0), b ? b->size3() : VecI(0)))return c; // first compare sizes
   return   Compare(a, b); // compare by memory addresses
}
/******************************************************************************/
MemberDesc& MemberDesc::set(DATA_TYPE type, UInt offset, Int size)
{
   T.type=type; T.offset=offset; T.size=size;
   T._data_to_text=null; T._text_to_data=null; T._compare=null;
   return T;
}
/******************************************************************************/
// BOOL
/******************************************************************************/
static INLINE Bool AsBool(CChar8 *s) {return s && *s!='0';}
static INLINE Bool AsBool(CChar  *s) {return s && *s!='0';}

Bool MemberDesc::asBool(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextBool(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return AsBool( (Char8* )data);
            case DATA_CHAR8_PTR: return AsBool(*(Char8**)data);
            case DATA_CHAR     : return AsBool( (Char * )data);
            case DATA_CHAR_PTR : return AsBool(*(Char **)data);
            case DATA_STR8     : return AsBool(*(Str8 * )data);
            case DATA_STR      : return AsBool(*(Str  * )data);
            case DATA_PTR      : return       (*(Ptr  * )data)!=null;

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return (*(I8 *)data)!=0;
               case 2: return (*(I16*)data)!=0;
               case 4: return (*(I32*)data)!=0;
               case 8: return (*(I64*)data)!=0;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return (*(U8 *)data)!=0;
               case 2: return (*(U16*)data)!=0;
               case 4: return (*(U32*)data)!=0;
               case 8: return (*(U64*)data)!=0;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return (*(Half*)data)!=0;
               case SIZE(Flt ): return (*(Flt *)data)!=0;
               case SIZE(Dbl ): return (*(Dbl *)data)!=0;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return ((VecH2*)data)->any();
               case SIZE(Vec2 ): return ((Vec2 *)data)->any();
               case SIZE(VecD2): return ((VecD2*)data)->any();
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return ((VecH*)data)->any();
               case SIZE(Vec ): return ((Vec *)data)->any();
               case SIZE(VecD): return ((VecD*)data)->any();
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return ((VecH4*)data)->any();
               case SIZE(Vec4 ): return ((Vec4 *)data)->any();
               case SIZE(VecD4): return ((VecD4*)data)->any();
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return ((VecSB2*)data)->any();
               case SIZE(VecI2 ): return ((VecI2 *)data)->any();
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return ((VecSB*)data)->any();
               case SIZE(VecI ): return ((VecI *)data)->any();
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return ((VecSB4*)data)->any();
               case SIZE(VecI4 ): return ((VecI4 *)data)->any();
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return ((VecB2 *)data)->any();
               case SIZE(VecUS2): return ((VecUS2*)data)->any();
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return ((VecB* )data)->any();
               case SIZE(VecUS): return ((VecUS*)data)->any();
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return ((VecB4*)data)->any();
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return ((VecB *)data)->any();
               case SIZE(VecB4): return ((VecB4*)data)->any();
               case SIZE(VecH ): return ((VecH *)data)->any();
               case SIZE(VecH4): return ((VecH4*)data)->any();
               case SIZE(Vec  ): return ((Vec  *)data)->any();
               case SIZE(VecD ): return ((VecD *)data)->any();
               case SIZE(Vec4 ): return ((Vec4 *)data)->any();
               case SIZE(VecD4): return ((VecD4*)data)->any();
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->valid();
            }break;
         }
      }
   }
   return false;
}
/******************************************************************************/
void MemberDesc::fromBool(Ptr data, Bool value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextInt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextInt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=value; break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value; break;
               case 2: *(I16*)data=value; break;
               case 4: *(I32*)data=value; break;
               case 8: *(I64*)data=value; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value; break;
               case 2: *(U16*)data=value; break;
               case 4: *(U32*)data=value; break;
               case 8: *(U64*)data=value; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=value; break;
               case SIZE(VecI2 ): *(VecI2 *)data=value; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=value; break;
               case SIZE(VecI ): *(VecI *)data=value; break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=value; break;
               case SIZE(VecI4 ): *(VecI4 *)data=value; break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=value; break;
               case SIZE(VecUS2): *(VecUS2*)data=value; break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecUS): *(VecUS*)data=value; break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=value; break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=(value ? 255 : 0); break;
               case SIZE(VecB4): *(VecB4*)data=(value ? 255 : 0); break;
               case SIZE(VecH ): *(VecH *)data=(value          ); break;
               case SIZE(VecH4): *(VecH4*)data=(value          ); break;
               case SIZE(Vec  ): *(Vec  *)data=(value ?   1 : 0); break;
               case SIZE(VecD ): *(VecD *)data=(value ?   1 : 0); break;
               case SIZE(Vec4 ): *(Vec4 *)data=(value ?   1 : 0); break;
               case SIZE(VecD4): *(VecD4*)data=(value ?   1 : 0); break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(value, 0, 0, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// INT
/******************************************************************************/
Int MemberDesc::asInt(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextInt(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextInt( (Char8* )data);
            case DATA_CHAR8_PTR: return TextInt(*(Char8**)data);
            case DATA_CHAR     : return TextInt( (Char * )data);
            case DATA_CHAR_PTR : return TextInt(*(Char **)data);
            case DATA_STR8     : return TextInt(*(Str8 * )data);
            case DATA_STR      : return TextInt(*(Str  * )data);
            case DATA_PTR      : return UIntPtr(*(Ptr  * )data);

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return Round(*(Half*)data);
               case SIZE(Flt ): return Round(*(Flt *)data);
               case SIZE(Dbl ): return Round(*(Dbl *)data);
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return Round((*(VecH2*)data).x);
               case SIZE(Vec2 ): return Round((*(Vec2 *)data).x);
               case SIZE(VecD2): return Round((*(VecD2*)data).x);
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return Round((*(VecH*)data).x);
               case SIZE(Vec ): return Round((*(Vec *)data).x);
               case SIZE(VecD): return Round((*(VecD*)data).x);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return Round((*(VecH4*)data).x);
               case SIZE(Vec4 ): return Round((*(Vec4 *)data).x);
               case SIZE(VecD4): return Round((*(VecD4*)data).x);
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return (*(VecSB2*)data).x;
               case SIZE(VecI2 ): return (*(VecI2 *)data).x;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).x;
               case SIZE(VecI ): return (*(VecI *)data).x;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).x;
               case SIZE(VecI4 ): return (*(VecI4 *)data).x;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return (*(VecB2 *)data).x;
               case SIZE(VecUS2): return (*(VecUS2*)data).x;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x;
               case SIZE(VecUS): return (*(VecUS*)data).x;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).x;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return       (*(VecB *)data).x;
               case SIZE(Color): return       (*(Color*)data).r;
               case SIZE(VecH ): return Round((*(VecH *)data).x);
               case SIZE(VecH4): return Round((*(VecH4*)data).x);
               case SIZE(Vec  ): return Round((*(Vec  *)data).x);
               case SIZE(VecD ): return Round((*(VecD *)data).x);
               case SIZE(Vec4 ): return Round((*(Vec4 *)data).x);
               case SIZE(VecD4): return Round((*(VecD4*)data).x);
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->i[0];
            }break;
         }
      }
   }
   return 0;
}
Int MemberDesc::asEnum(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextInt(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextInt( (Char8* )data);
            case DATA_CHAR8_PTR: return TextInt(*(Char8**)data);
            case DATA_CHAR     : return TextInt( (Char * )data);
            case DATA_CHAR_PTR : return TextInt(*(Char **)data);
            case DATA_STR8     : return TextInt(*(Str8 * )data);
            case DATA_STR      : return TextInt(*(Str  * )data);
            case DATA_PTR      : return UIntPtr(*(Ptr  * )data);

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return Round(*(Half*)data);
               case SIZE(Flt ): return Round(*(Flt *)data);
               case SIZE(Dbl ): return Round(*(Dbl *)data);
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return Round((*(VecH2*)data).x);
               case SIZE(Vec2 ): return Round((*(Vec2 *)data).x);
               case SIZE(VecD2): return Round((*(VecD2*)data).x);
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return Round((*(VecH*)data).x);
               case SIZE(Vec ): return Round((*(Vec *)data).x);
               case SIZE(VecD): return Round((*(VecD*)data).x);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return Round((*(VecH4*)data).x);
               case SIZE(Vec4 ): return Round((*(Vec4 *)data).x);
               case SIZE(VecD4): return Round((*(VecD4*)data).x);
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return (*(VecSB2*)data).x;
               case SIZE(VecI2 ): return (*(VecI2 *)data).x;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).x;
               case SIZE(VecI ): return (*(VecI *)data).x;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).x;
               case SIZE(VecI4 ): return (*(VecI4 *)data).x;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return (*(VecB2 *)data).x;
               case SIZE(VecUS2): return (*(VecUS2*)data).x;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x;
               case SIZE(VecUS): return (*(VecUS*)data).x;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).x;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return       (*(VecB *)data).x;
               case SIZE(Color): return       (*(Color*)data).r;
               case SIZE(VecH ): return Round((*(VecH *)data).x);
               case SIZE(VecH4): return Round((*(VecH4*)data).x);
               case SIZE(Vec  ): return Round((*(Vec  *)data).x);
               case SIZE(VecD ): return Round((*(VecD *)data).x);
               case SIZE(Vec4 ): return Round((*(Vec4 *)data).x);
               case SIZE(VecD4): return Round((*(VecD4*)data).x);
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->i[0];
            }break;
         }
      }
   }
   return -1;
}
/******************************************************************************/
void MemberDesc::fromInt(Ptr data, Int value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextInt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextInt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value; break;
               case 2: *(I16*)data=value; break;
               case 4: *(I32*)data=value; break;
               case 8: *(I64*)data=value; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value; break;
               case 2: *(U16*)data=value; break;
               case 4: *(U32*)data=value; break;
               case 8: *(U64*)data=value; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=value; break;
               case SIZE(VecI2 ): *(VecI2 *)data=value; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=value; break;
               case SIZE(VecI ): *(VecI *)data=value; break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=value; break;
               case SIZE(VecI4 ): *(VecI4 *)data=value; break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=value; break;
               case SIZE(VecUS2): *(VecUS2*)data=value; break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecUS): *(VecUS*)data=value; break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=value; break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecB4): *(VecB4*)data=value; break;
               case SIZE(VecH ): *(VecH *)data=value; break;
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec  ): *(Vec  *)data=value; break;
               case SIZE(VecD ): *(VecD *)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(value, 0, 0, 0);
            }break;
         }
      }
   }
}
void MemberDesc::fromUInt(Ptr data, UInt value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextInt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextInt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value; break;
               case 2: *(I16*)data=value; break;
               case 4: *(I32*)data=value; break;
               case 8: *(I64*)data=value; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value; break;
               case 2: *(U16*)data=value; break;
               case 4: *(U32*)data=value; break;
               case 8: *(U64*)data=value; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=value; break;
               case SIZE(VecI2 ): *(VecI2 *)data=value; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=value; break;
               case SIZE(VecI ): *(VecI *)data=value; break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=value; break;
               case SIZE(VecI4 ): *(VecI4 *)data=value; break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=value; break;
               case SIZE(VecUS2): *(VecUS2*)data=value; break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecUS): *(VecUS*)data=value; break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=value; break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecB4): *(VecB4*)data=value; break;
               case SIZE(VecH ): *(VecH *)data=value; break;
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec  ): *(Vec  *)data=value; break;
               case SIZE(VecD ): *(VecD *)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(value, 0, 0, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// LONG
/******************************************************************************/
Long MemberDesc::asLong(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextLong(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextLong( (Char8* )data);
            case DATA_CHAR8_PTR: return TextLong(*(Char8**)data);
            case DATA_CHAR     : return TextLong( (Char * )data);
            case DATA_CHAR_PTR : return TextLong(*(Char **)data);
            case DATA_STR8     : return TextLong(*(Str8 * )data);
            case DATA_STR      : return TextLong(*(Str  * )data);
            case DATA_PTR      : return UIntPtr (*(Ptr  * )data);

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return RoundL(*(Half*)data);
               case SIZE(Flt ): return RoundL(*(Flt *)data);
               case SIZE(Dbl ): return RoundL(*(Dbl *)data);
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return RoundL((*(VecH2*)data).x);
               case SIZE(Vec2 ): return RoundL((*(Vec2 *)data).x);
               case SIZE(VecD2): return RoundL((*(VecD2*)data).x);
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return RoundL((*(VecH*)data).x);
               case SIZE(Vec ): return RoundL((*(Vec *)data).x);
               case SIZE(VecD): return RoundL((*(VecD*)data).x);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return RoundL((*(VecH4*)data).x);
               case SIZE(Vec4 ): return RoundL((*(Vec4 *)data).x);
               case SIZE(VecD4): return RoundL((*(VecD4*)data).x);
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return (*(VecSB2*)data).x;
               case SIZE(VecI2 ): return (*(VecI2 *)data).x;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).x;
               case SIZE(VecI ): return (*(VecI *)data).x;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).x;
               case SIZE(VecI4 ): return (*(VecI4 *)data).x;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return (*(VecB2 *)data).x;
               case SIZE(VecUS2): return (*(VecUS2*)data).x;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x;
               case SIZE(VecUS): return (*(VecUS*)data).x;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).x;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return        (*(VecB *)data).x ;
               case SIZE(Color): return        (*(Color*)data).r ;
               case SIZE(VecH ): return RoundL((*(VecH *)data).x);
               case SIZE(VecH4): return RoundL((*(VecH4*)data).x);
               case SIZE(Vec  ): return RoundL((*(Vec  *)data).x);
               case SIZE(VecD ): return RoundL((*(VecD *)data).x);
               case SIZE(Vec4 ): return RoundL((*(Vec4 *)data).x);
               case SIZE(VecD4): return RoundL((*(VecD4*)data).x);
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->l[0];
            }break;
         }
      }
   }
   return 0;
}
void MemberDesc::fromLong(Ptr data, Long value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextInt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextInt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value; break;
               case 2: *(I16*)data=value; break;
               case 4: *(I32*)data=value; break;
               case 8: *(I64*)data=value; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value; break;
               case 2: *(U16*)data=value; break;
               case 4: *(U32*)data=value; break;
               case 8: *(U64*)data=value; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=value; break;
               case SIZE(VecI2 ): *(VecI2 *)data=value; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=value; break;
               case SIZE(VecI ): *(VecI *)data=value; break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=value; break;
               case SIZE(VecI4 ): *(VecI4 *)data=value; break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=value; break;
               case SIZE(VecUS2): *(VecUS2*)data=value; break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecUS): *(VecUS*)data=value; break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=value; break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecB4): *(VecB4*)data=value; break;
               case SIZE(VecH ): *(VecH *)data=value; break;
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec  ): *(Vec  *)data=value; break;
               case SIZE(VecD ): *(VecD *)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(value, 0);
            }break;
         }
      }
   }
}
void MemberDesc::fromULong(Ptr data, ULong value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextInt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextInt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value; break;
               case 2: *(I16*)data=value; break;
               case 4: *(I32*)data=value; break;
               case 8: *(I64*)data=value; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value; break;
               case 2: *(U16*)data=value; break;
               case 4: *(U32*)data=value; break;
               case 8: *(U64*)data=value; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=value; break;
               case SIZE(VecI2 ): *(VecI2 *)data=value; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=value; break;
               case SIZE(VecI ): *(VecI *)data=value; break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=value; break;
               case SIZE(VecI4 ): *(VecI4 *)data=value; break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=value; break;
               case SIZE(VecUS2): *(VecUS2*)data=value; break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecUS): *(VecUS*)data=value; break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=value; break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=value; break;
               case SIZE(VecB4): *(VecB4*)data=value; break;
               case SIZE(VecH ): *(VecH *)data=value; break;
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec  ): *(Vec  *)data=value; break;
               case SIZE(VecD ): *(VecD *)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(value, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// FLT
/******************************************************************************/
Flt MemberDesc::asFlt(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextFlt(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextFlt( (Char8* )data);
            case DATA_CHAR8_PTR: return TextFlt(*(Char8**)data);
            case DATA_CHAR     : return TextFlt( (Char * )data);
            case DATA_CHAR_PTR : return TextFlt(*(Char **)data);
            case DATA_STR8     : return TextFlt(*(Str8 * )data);
            case DATA_STR      : return TextFlt(*(Str  * )data);
            case DATA_PTR      : return        (*(Ptr  * )data)!=null;

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return *(Half*)data;
               case SIZE(Flt ): return *(Flt *)data;
               case SIZE(Dbl ): return *(Dbl *)data;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return (*(VecH2*)data).x;
               case SIZE(Vec2 ): return (*(Vec2 *)data).x;
               case SIZE(VecD2): return (*(VecD2*)data).x;
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return (*(VecH*)data).x;
               case SIZE(Vec ): return (*(Vec *)data).x;
               case SIZE(VecD): return (*(VecD*)data).x;
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return (*(VecH4*)data).x;
               case SIZE(Vec4 ): return (*(Vec4 *)data).x;
               case SIZE(VecD4): return (*(VecD4*)data).x;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return (*(VecSB2*)data).x;
               case SIZE(VecI2 ): return (*(VecI2 *)data).x;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).x;
               case SIZE(VecI ): return (*(VecI *)data).x;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).x;
               case SIZE(VecI4 ): return (*(VecI4 *)data).x;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return (*(VecB2 *)data).x;
               case SIZE(VecUS2): return (*(VecUS2*)data).x;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x;
               case SIZE(VecUS): return (*(VecUS*)data).x;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).x;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x/255.0f;
               case SIZE(Color): return (*(Color*)data).r/255.0f;
               case SIZE(VecH ): return (*(VecH *)data).x;
               case SIZE(VecH4): return (*(VecH4*)data).x;
               case SIZE(Vec  ): return (*(Vec  *)data).x;
               case SIZE(VecD ): return (*(VecD *)data).x;
               case SIZE(Vec4 ): return (*(Vec4 *)data).x;
               case SIZE(VecD4): return (*(VecD4*)data).x;
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->i[0];
            }break;
         }
      }
   }
   return 0;
}
/******************************************************************************/
void MemberDesc::fromFlt(Ptr data, Flt value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, Dbl(value)); // if we're passing to a callback then copy in higher precision
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextFlt(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextFlt(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=Round (value); break;
               case 2: *(I16*)data=Round (value); break;
               case 4: *(I32*)data=Round (value); break;
               case 8: *(I64*)data=RoundL(value); break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=RoundU (value); break;
               case 2: *(U16*)data=RoundU (value); break;
               case 4: *(U32*)data=RoundU (value); break;
               case 8: *(U64*)data=RoundUL(value); break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=Round(value); break;
               case SIZE(VecI2 ): *(VecI2 *)data=Round(value); break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=Round(value); break;
               case SIZE(VecI ): *(VecI *)data=Round(value); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=Round(value); break;
               case SIZE(VecI4 ): *(VecI4 *)data=Round(value); break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=Round(value); break;
               case SIZE(VecUS2): *(VecUS2*)data=Round(value); break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=Round(value); break;
               case SIZE(VecUS): *(VecUS*)data=Round(value); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=Round(value); break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=FltToByte(value); break;
               case SIZE(VecB4): *(VecB4*)data=FltToByte(value); break;
               case SIZE(VecH ): *(VecH *)data=          value ; break;
               case SIZE(VecH4): *(VecH4*)data=          value ; break;
               case SIZE(Vec  ): *(Vec  *)data=          value ; break;
               case SIZE(VecD ): *(VecD *)data=          value ; break;
               case SIZE(Vec4 ): *(Vec4 *)data=          value ; break;
               case SIZE(VecD4): *(VecD4*)data=          value ; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(RoundU(value), 0, 0, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// DBL
/******************************************************************************/
Dbl MemberDesc::asDbl(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextDbl(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextDbl( (Char8* )data);
            case DATA_CHAR8_PTR: return TextDbl(*(Char8**)data);
            case DATA_CHAR     : return TextDbl( (Char * )data);
            case DATA_CHAR_PTR : return TextDbl(*(Char **)data);
            case DATA_STR8     : return TextDbl(*(Str8 * )data);
            case DATA_STR      : return TextDbl(*(Str  * )data);
            case DATA_PTR      : return        (*(Ptr  * )data)!=null;

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return *(Half*)data;
               case SIZE(Flt ): return *(Flt *)data;
               case SIZE(Dbl ): return *(Dbl *)data;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return (*(VecH2*)data).x;
               case SIZE(Vec2 ): return (*(Vec2 *)data).x;
               case SIZE(VecD2): return (*(VecD2*)data).x;
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return (*(VecH*)data).x;
               case SIZE(Vec ): return (*(Vec *)data).x;
               case SIZE(VecD): return (*(VecD*)data).x;
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return (*(VecH4*)data).x;
               case SIZE(Vec4 ): return (*(Vec4 *)data).x;
               case SIZE(VecD4): return (*(VecD4*)data).x;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return (*(VecSB2*)data).x;
               case SIZE(VecI2 ): return (*(VecI2 *)data).x;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).x;
               case SIZE(VecI ): return (*(VecI *)data).x;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).x;
               case SIZE(VecI4 ): return (*(VecI4 *)data).x;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return (*(VecB2 *)data).x;
               case SIZE(VecUS2): return (*(VecUS2*)data).x;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x;
               case SIZE(VecUS): return (*(VecUS*)data).x;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).x;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).x/255.0;
               case SIZE(Color): return (*(Color*)data).r/255.0;
               case SIZE(VecH ): return (*(VecH *)data).x;
               case SIZE(VecH4): return (*(VecH4*)data).x;
               case SIZE(Vec  ): return (*(Vec  *)data).x;
               case SIZE(VecD ): return (*(VecD *)data).x;
               case SIZE(Vec4 ): return (*(Vec4 *)data).x;
               case SIZE(VecD4): return (*(VecD4*)data).x;
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->i[0];
            }break;
         }
      }
   }
   return 0;
}
/******************************************************************************/
void MemberDesc::fromDbl(Ptr data, Dbl value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, TextDbl(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, TextDbl(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=         value          ; break;
            case DATA_STR  :    *(Str  *)data=         value          ; break;

            case DATA_BOOL: *(Bool*)data=(value!=0); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=Round (value); break;
               case 2: *(I16*)data=Round (value); break;
               case 4: *(I32*)data=Round (value); break;
               case 8: *(I64*)data=RoundL(value); break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=RoundU (value); break;
               case 2: *(U16*)data=RoundU (value); break;
               case 4: *(U32*)data=RoundU (value); break;
               case 8: *(U64*)data=RoundUL(value); break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value; break;
               case SIZE(Flt ): *(Flt *)data=value; break;
               case SIZE(Dbl ): *(Dbl *)data=value; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=value; break;
               case SIZE(Vec ): *(Vec *)data=value; break;
               case SIZE(VecD): *(VecD*)data=value; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=value; break;
               case SIZE(Vec4 ): *(Vec4 *)data=value; break;
               case SIZE(VecD4): *(VecD4*)data=value; break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=Round(value); break;
               case SIZE(VecI2 ): *(VecI2 *)data=Round(value); break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=Round(value); break;
               case SIZE(VecI ): *(VecI *)data=Round(value); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=Round(value); break;
               case SIZE(VecI4 ): *(VecI4 *)data=Round(value); break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=Round(value); break;
               case SIZE(VecUS2): *(VecUS2*)data=Round(value); break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=Round(value); break;
               case SIZE(VecUS): *(VecUS*)data=Round(value); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=Round(value); break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=FltToByte(value); break;
               case SIZE(VecB4): *(VecB4*)data=FltToByte(value); break;
               case SIZE(VecH ): *(VecH *)data=          value ; break;
               case SIZE(VecH4): *(VecH4*)data=          value ; break;
               case SIZE(Vec  ): *(Vec  *)data=          value ; break;
               case SIZE(VecD ): *(VecD *)data=          value ; break;
               case SIZE(Vec4 ): *(Vec4 *)data=          value ; break;
               case SIZE(VecD4): *(VecD4*)data=          value ; break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(RoundU(value), 0, 0, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// VEC2
/******************************************************************************/
Vec2 MemberDesc::asVec2(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextVec2(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextVec2( (Char8* )data);
            case DATA_CHAR8_PTR: return TextVec2(*(Char8**)data);
            case DATA_CHAR     : return TextVec2( (Char * )data);
            case DATA_CHAR_PTR : return TextVec2(*(Char **)data);
            case DATA_STR8     : return TextVec2(*(Str8 * )data);
            case DATA_STR      : return TextVec2(*(Str  * )data);
            case DATA_PTR      : return         (*(Ptr  * )data)!=null;

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return (Flt)*(Half*)data;
               case SIZE(Flt ): return      *(Flt *)data;
               case SIZE(Dbl ): return      *(Dbl *)data;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return *(VecH2*)data;
               case SIZE(Vec2 ): return *(Vec2 *)data;
               case SIZE(VecD2): return *(VecD2*)data;
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return (*(VecH*)data).xy;
               case SIZE(Vec ): return (*(Vec *)data).xy;
               case SIZE(VecD): return (*(VecD*)data).xy;
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return (*(VecH4*)data).xy;
               case SIZE(Vec4 ): return (*(Vec4 *)data).xy;
               case SIZE(VecD4): return (*(VecD4*)data).xy;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return *(VecSB2*)data;
               case SIZE(VecI2 ): return *(VecI2 *)data;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).xy;
               case SIZE(VecI ): return (*(VecI *)data).xy;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).xy;
               case SIZE(VecI4 ): return (*(VecI4 *)data).xy;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return *(VecB2 *)data;
               case SIZE(VecUS2): return *(VecUS2*)data;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).xy;
               case SIZE(VecUS): return (*(VecUS*)data).xy;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).xy;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): {C VecB  &v=(*(VecB *)data); return Vec2(v.x/255.0f, v.y/255.0f);}
               case SIZE(Color): {C Color &c=(*(Color*)data); return Vec2(c.r/255.0f, c.g/255.0f);}
               case SIZE(VecH ): return      (*(VecH *)data).xy;
               case SIZE(VecH4): return      (*(VecH4*)data).xy;
               case SIZE(Vec  ): return      (*(Vec  *)data).xy;
               case SIZE(VecD ): return      (*(VecD *)data).xy;
               case SIZE(Vec4 ): return      (*(Vec4 *)data).xy;
               case SIZE(VecD4): return      (*(VecD4*)data).xy;
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return Vec2(((UID*)data)->i[0], ((UID*)data)->i[1]);
            }break;
         }
      }
   }
   return 0;
}
/******************************************************************************/
void MemberDesc::fromVec2(Ptr data, C Vec2 &value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, VecD2(value)); // if we're passing to a callback then copy in higher precision
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, Str8(value), size  ); break;
            case DATA_CHAR : Set((Char *)data, Str8(value), size/2); break;
            case DATA_STR8 :    *(Str8 *)data=      value          ; break;
            case DATA_STR  :    *(Str  *)data=      value          ; break;

            case DATA_BOOL: *(Bool*)data=value.any(); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=Round (value.x); break;
               case 2: *(I16*)data=Round (value.x); break;
               case 4: *(I32*)data=Round (value.x); break;
               case 8: *(I64*)data=RoundL(value.x); break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=RoundU (value.x); break;
               case 2: *(U16*)data=RoundU (value.x); break;
               case 4: *(U32*)data=RoundU (value.x); break;
               case 8: *(U64*)data=RoundUL(value.x); break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value.x; break;
               case SIZE(Flt ): *(Flt *)data=value.x; break;
               case SIZE(Dbl ): *(Dbl *)data=value.x; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=value; break;
               case SIZE(Vec2 ): *(Vec2 *)data=value; break;
               case SIZE(VecD2): *(VecD2*)data=value; break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): ((VecH*)data)->set(value, 0); break;
               case SIZE(Vec ): ((Vec *)data)->set(value, 0); break;
               case SIZE(VecD): ((VecD*)data)->set(value, 0); break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): ((VecH4*)data)->set(value, 0, 0); break;
               case SIZE(Vec4 ): ((Vec4 *)data)->set(value, 0, 0); break;
               case SIZE(VecD4): ((VecD4*)data)->set(value, 0, 0); break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): ((VecSB2*)data)->set(Round(value.x), Round(value.y)); break;
               case SIZE(VecI2 ): *(VecI2 *)data=Round(value); break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): ((VecSB*)data)->set(Round(value.x), Round(value.y), 0); break;
               case SIZE(VecI ): ((VecI *)data)->set(Round(value  )                , 0); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): ((VecSB4*)data)->set(Round(value.x), Round(value.y), 0, 0); break;
               case SIZE(VecI4 ): ((VecI4 *)data)->set(Round(value)                  , 0, 0); break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): ((VecB2 *)data)->set(Round(value.x), Round(value.y)); break;
               case SIZE(VecUS2): *(VecUS2*)data=Round(value); break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): ((VecB *)data)->set(Round(value.x), Round(value.y), 0); break;
               case SIZE(VecUS): ((VecUS*)data)->set(Round(value  )                , 0); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): ((VecB4*)data)->set(Round(value.x), Round(value.y), 0, 0); break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): ((VecB *)data)->set(FltToByte(value.x), FltToByte(value.y),        0         ); break;
               case SIZE(Color): ((Color*)data)->set(FltToByte(value.x), FltToByte(value.y),        0,     255); break;
               case SIZE(VecH ): ((VecH *)data)->set(          value.x ,           value.y , HalfZero         ); break;
               case SIZE(VecH4): ((VecH4*)data)->set(          value.x ,           value.y , HalfZero, HalfOne); break;
               case SIZE(Vec  ): ((Vec  *)data)->set(          value.x ,           value.y ,        0         ); break;
               case SIZE(VecD ): ((VecD *)data)->set(          value.x ,           value.y ,        0         ); break;
               case SIZE(Vec4 ): ((Vec4 *)data)->set(          value.x ,           value.y ,        0,       1); break;
               case SIZE(VecD4): ((VecD4*)data)->set(          value.x ,           value.y ,        0,       1); break;
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->set(RoundU(value.x), RoundU(value.y), 0, 0);
            }break;
         }
      }
   }
}
/******************************************************************************/
// VECD2
/******************************************************************************/
VecD2 MemberDesc::asVecD2(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextVecD2(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return TextVecD2( (Char8* )data);
            case DATA_CHAR8_PTR: return TextVecD2(*(Char8**)data);
            case DATA_CHAR     : return TextVecD2( (Char * )data);
            case DATA_CHAR_PTR : return TextVecD2(*(Char **)data);
            case DATA_STR8     : return TextVecD2(*(Str8 * )data);
            case DATA_STR      : return TextVecD2(*(Str  * )data);
            case DATA_PTR      : return          (*(Ptr  * )data)!=null;

            case DATA_BOOL: return *(Bool*)data;

            case DATA_INT: switch(size)
            {
               case 1: return *(I8 *)data;
               case 2: return *(I16*)data;
               case 4: return *(I32*)data;
               case 8: return *(I64*)data;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return *(U8 *)data;
               case 2: return *(U16*)data;
               case 4: return *(U32*)data;
               case 8: return *(U64*)data;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return (Flt)*(Half*)data;
               case SIZE(Flt ): return      *(Flt *)data;
               case SIZE(Dbl ): return      *(Dbl *)data;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return *(VecH2*)data;
               case SIZE(Vec2 ): return *(Vec2 *)data;
               case SIZE(VecD2): return *(VecD2*)data;
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return (*(VecH*)data).xy;
               case SIZE(Vec ): return (*(Vec *)data).xy;
               case SIZE(VecD): return (*(VecD*)data).xy;
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return (*(VecH4*)data).xy;
               case SIZE(Vec4 ): return (*(Vec4 *)data).xy;
               case SIZE(VecD4): return (*(VecD4*)data).xy;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return *(VecSB2*)data;
               case SIZE(VecI2 ): return *(VecI2 *)data;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return (*(VecSB*)data).xy;
               case SIZE(VecI ): return (*(VecI *)data).xy;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return (*(VecSB4*)data).xy;
               case SIZE(VecI4 ): return (*(VecI4 *)data).xy;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return *(VecB2 *)data;
               case SIZE(VecUS2): return *(VecUS2*)data;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return (*(VecB *)data).xy;
               case SIZE(VecUS): return (*(VecUS*)data).xy;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return (*(VecB4*)data).xy;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): {C VecB  &v=(*(VecB *)data); return VecD2(v.x/255.0, v.y/255.0);}
               case SIZE(Color): {C Color &c=(*(Color*)data); return VecD2(c.r/255.0, c.g/255.0);}
               case SIZE(VecH ): return      (*(VecH *)data).xy;
               case SIZE(VecH4): return      (*(VecH4*)data).xy;
               case SIZE(Vec  ): return      (*(Vec  *)data).xy;
               case SIZE(VecD ): return      (*(VecD *)data).xy;
               case SIZE(Vec4 ): return      (*(Vec4 *)data).xy;
               case SIZE(VecD4): return      (*(VecD4*)data).xy;
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->is();
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return VecD2(((UID*)data)->i[0], ((UID*)data)->i[1]);
            }break;
         }
      }
   }
   return 0;
}
/******************************************************************************/
// COLOR
/******************************************************************************/
Vec4 MemberDesc::asColor(CPtr data)C
{
   if(data)
   {
      if(_data_to_text)
      {
         return TextVec4(_data_to_text(data));
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_BOOL: return *(Bool*)data;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return Vec4(   (*(VecH*)data), 1);
               case SIZE(Vec ): return Vec4(   (*(Vec *)data), 1);
               case SIZE(VecD): return Vec4(Vec(*(VecD*)data), 1);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return *(VecH4*)data;
               case SIZE(Vec4 ): return *(Vec4 *)data;
               case SIZE(VecD4): return *(VecD4*)data;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB):     {C VecSB &v=*(VecSB*)data; return Vec4(SByteToSFlt(v.x), SByteToSFlt(v.y), SByteToSFlt(v.z), 1);}
               case SIZE(VecI ): return Vec4(Vec(*(VecI *)data)/255.0f, 1);
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): {C VecSB4 &v=*(VecSB4*)data; return Vec4(SByteToSFlt(v.x), SByteToSFlt(v.y), SByteToSFlt(v.z), SByteToSFlt(v.w));}
               case SIZE(VecI4 ):  return Vec4(*(VecI4 *)data)/255.0f;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return Vec4(Vec(*(VecB *)data)/255.0f, 1);
               case SIZE(VecUS): return Vec4(Vec(*(VecUS*)data)/255.0f, 1);
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return Vec4(*(VecB4*)data)/255.0f;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return Vec4(Vec(*(VecB *)data)/255.0f, 1);
               case SIZE(Color): return          ((Color*)data)->asVec4();
               case SIZE(VecH ): return     Vec4(*(VecH *)data, 1);
               case SIZE(VecH4): return          *(VecH4*)data;
               case SIZE(Vec  ): return     Vec4(*(Vec  *)data, 1);
               case SIZE(VecD ): return     Vec4(*(VecD *)data, 1);
               case SIZE(Vec4 ): return          *(Vec4 *)data;
               case SIZE(VecD4): return          *(VecD4*)data;
            }break;
         }
      }
   }
   return 0;
}
/******************************************************************************/
void MemberDesc::fromColor(Ptr data, C Vec4 &color)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, color);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=color.xyz; break;
               case SIZE(Vec ): *(Vec *)data=color.xyz; break;
               case SIZE(VecD): *(VecD*)data=color.xyz; break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=color; break;
               case SIZE(Vec4 ): *(Vec4 *)data=color; break;
               case SIZE(VecD4): *(VecD4*)data=color; break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): ((VecSB*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y), SFltToSByte(color.z)); break;
               case SIZE(VecI ): ((VecI *)data)->set( FltToByte (color.x),  FltToByte (color.y),  FltToByte (color.z)); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): ((VecSB4*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y), SFltToSByte(color.z), SFltToSByte(color.w)); break;
               case SIZE(VecI4 ): ((VecI4 *)data)->set( FltToByte (color.x),  FltToByte (color.y),  FltToByte (color.z),  FltToByte (color.w)); break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): ((VecB *)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)); break;
               case SIZE(VecUS): ((VecUS*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z), FltToByte(color.w)); break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): ((VecB* )data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)); break;
               case SIZE(Color): *(Color*)data=color    ; break;
               case SIZE(VecH ): *(VecH *)data=color.xyz; break;
               case SIZE(VecH4): *(VecH4*)data=color    ; break;
               case SIZE(Vec  ): *(Vec  *)data=color.xyz; break;
               case SIZE(VecD ): *(VecD *)data=color.xyz; break;
               case SIZE(Vec4 ): *(Vec4 *)data=color    ; break;
               case SIZE(VecD4): *(VecD4*)data=color    ; break;
            }break;
         }
      }
   }
}
/******************************************************************************/
// TEXT
/******************************************************************************/
Str MemberDesc::asText(CPtr data, Int precision)C
{
   Char8 temp[256];
   if(data)
   {
      if(_data_to_text)
      {
         return _data_to_text(data);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : return  (Char8* )data;
            case DATA_CHAR8_PTR: return *(Char8**)data;
            case DATA_CHAR     : return  (Char * )data;
            case DATA_CHAR_PTR : return *(Char **)data;
            case DATA_STR8     : return *(Str8 * )data;
            case DATA_STR      : return *(Str  * )data;
            case DATA_PTR      : return (SIZE(Ptr)==4) ? TextHex(*(U32*)data, temp,  8, 0, true)
                                                       : TextHex(*(U64*)data, temp, 16, 0, true);

            case DATA_BOOL: return TextInt(*(Bool*)data, temp);

            case DATA_INT: switch(size)
            {
               case 1: return TextInt(*(I8 *)data, temp);
               case 2: return TextInt(*(I16*)data, temp);
               case 4: return TextInt(*(I32*)data, temp);
               case 8: return TextInt(*(I64*)data, temp);
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: return TextInt((UInt)*(U8 *)data, temp);
               case 2: return TextInt((UInt)*(U16*)data, temp);
               case 4: return TextInt(      *(U32*)data, temp);
               case 8: return TextInt(      *(U64*)data, temp);
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return TextReal(*(Half*)data, temp, (precision==INT_MAX) ? PRECISION_HALF : precision);
               case SIZE(Flt ): return TextReal(*(Flt *)data, temp, (precision==INT_MAX) ? PRECISION_FLT  : precision);
               case SIZE(Dbl ): return TextReal(*(Dbl *)data, temp, (precision==INT_MAX) ? PRECISION_DBL  : precision);
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return ((VecH2*)data)->asText(precision);
               case SIZE(Vec2 ): return ((Vec2 *)data)->asText(precision);
               case SIZE(VecD2): return ((VecD2*)data)->asText(precision);
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return ((VecH*)data)->asText(precision);
               case SIZE(Vec ): return ((Vec *)data)->asText(precision);
               case SIZE(VecD): return ((VecD*)data)->asText(precision);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return ((VecH4*)data)->asText(precision);
               case SIZE(Vec4 ): return ((Vec4 *)data)->asText(precision);
               case SIZE(VecD4): return ((VecD4*)data)->asText(precision);
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): return Str()=*(VecSB2*)data;
               case SIZE(VecI2 ): return Str()=*(VecI2 *)data;
            }break;
            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): return Str()=*(VecSB*)data;
               case SIZE(VecI ): return Str()=*(VecI *)data;
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return Str()=*(VecSB4*)data;
               case SIZE(VecI4 ): return Str()=*(VecI4 *)data;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): return Str()=*(VecB2 *)data;
               case SIZE(VecUS2): return Str()=*(VecUS2*)data;
            }break;
            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): return Str()=*(VecB *)data;
               case SIZE(VecUS): return Str()=*(VecUS*)data;
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return Str()=*(VecB4*)data;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): return Str()=*(VecB *)data;
               case SIZE(Color): return Str()=*(VecB4*)data;
               case SIZE(VecH ): return ((VecH *)data)->asText(precision);
               case SIZE(VecH4): return ((VecH4*)data)->asText(precision);
               case SIZE(Vec  ): return ((Vec  *)data)->asText(precision);
               case SIZE(VecD ): return ((VecD *)data)->asText(precision);
               case SIZE(Vec4 ): return ((Vec4 *)data)->asText(precision);
               case SIZE(VecD4): return ((VecD4*)data)->asText(precision);
            }break;

            case DATA_IMAGE_PTR:
            {
               if(size==SIZE(Image*))return Images.name(*(Image**)data);
            }break;

            case DATA_IMAGEPTR:
            {
               if(size==SIZE(ImagePtr))return Images.name(*(ImagePtr*)data);
            }break;

            case DATA_KBSC:
            {
               if(size==SIZE(KbSc))return ((KbSc*)data)->asText();
            }break;

            case DATA_DATE_TIME:
            {
               if(size==SIZE(DateTime))return ((DateTime*)data)->asText(true);
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return ((UID*)data)->asHex();
            }break;
         }
      }
   }
   return S;
}
/******************************************************************************/
void MemberDesc::fromText(Ptr data, C Str &text)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, text);
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, text, size  ); break;
            case DATA_CHAR : Set((Char *)data, text, size/2); break;
            case DATA_STR8 :    *(Str8 *)data= text         ; break;
            case DATA_STR  :    *(Str  *)data= text         ; break;

            case DATA_BOOL: *(Bool*)data=TextBool(text); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=TextInt (text); break;
               case 2: *(I16*)data=TextInt (text); break;
               case 4: *(I32*)data=TextInt (text); break;
               case 8: *(I64*)data=TextLong(text); break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=TextUInt (text); break;
               case 2: *(U16*)data=TextUInt (text); break;
               case 4: *(U32*)data=TextUInt (text); break;
               case 8: *(U64*)data=TextULong(text); break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=TextFlt(text); break;
               case SIZE(Flt ): *(Flt *)data=TextFlt(text); break;
               case SIZE(Dbl ): *(Dbl *)data=TextDbl(text); break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): *(VecH2*)data=TextVec2 (text); break;
               case SIZE(Vec2 ): *(Vec2 *)data=TextVec2 (text); break;
               case SIZE(VecD2): *(VecD2*)data=TextVecD2(text); break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): *(VecH*)data=TextVec (text); break;
               case SIZE(Vec ): *(Vec *)data=TextVec (text); break;
               case SIZE(VecD): *(VecD*)data=TextVecD(text); break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): *(VecH4*)data=TextVec4 (text); break;
               case SIZE(Vec4 ): *(Vec4 *)data=TextVec4 (text); break;
               case SIZE(VecD4): *(VecD4*)data=TextVecD4(text); break;
            }break;

            case DATA_VECI2: switch(size)
            {
               case SIZE(VecSB2): *(VecSB2*)data=TextVecSB2(text); break;
               case SIZE(VecI2 ): *(VecI2 *)data=TextVecI2 (text); break;
            }break;

            case DATA_VECI: switch(size)
            {
               case SIZE(VecSB): *(VecSB*)data=TextVecSB(text); break;
               case SIZE(VecI ): *(VecI *)data=TextVecI (text); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): *(VecSB4*)data=TextVecSB4(text); break;
               case SIZE(VecI4 ): *(VecI4 *)data=TextVecI4 (text); break;
            }break;

            case DATA_VECU2: switch(size)
            {
               case SIZE(VecB2 ): *(VecB2 *)data=TextVecB2 (text); break;
               case SIZE(VecUS2): *(VecUS2*)data=TextVecUS2(text); break;
            }break;

            case DATA_VECU: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=TextVecB (text); break;
               case SIZE(VecUS): *(VecUS*)data=TextVecUS(text); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): *(VecB4*)data=TextVecB4(text); break;
            }break;

            case DATA_COLOR: switch(size)
            {
               case SIZE(VecB ): *(VecB *)data=TextVecB (text); break;
               case SIZE(VecB4): *(VecB4*)data=TextVecB4(text); break;
               case SIZE(VecH ): *(VecH *)data=TextVec  (text); break;
               case SIZE(VecH4): *(VecH4*)data=TextVec4 (text); break;
               case SIZE(Vec  ): *(Vec  *)data=TextVec  (text); break;
               case SIZE(VecD ): *(VecD *)data=TextVec  (text); break;
               case SIZE(Vec4 ): *(Vec4 *)data=TextVec4 (text); break;
               case SIZE(VecD4): *(VecD4*)data=TextVec4 (text); break;
            }break;

            case DATA_IMAGE_PTR:
            {
               if(size==SIZE(Image*))*(Image**)data=Images.get(text);
            }break;

            case DATA_IMAGEPTR:
            {
               if(size==SIZE(ImagePtr))((ImagePtr*)data)->get(text);
            }break;

            case DATA_DATE_TIME:
            {
               if(size==SIZE(DateTime))((DateTime*)data)->fromText(text);
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))((UID*)data)->fromText(text);
            }break;
         }
      }
   }
}
/******************************************************************************/
// UID
/******************************************************************************/
UID MemberDesc::asUID(CPtr data)C
{
   UID id;
   if(data)
   {
      if(_data_to_text)
      {
         id.fromText(_data_to_text(data)); return id;
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8    : id.fromText( (Char8* )data); return id;
            case DATA_CHAR8_PTR: id.fromText(*(Char8**)data); return id;
            case DATA_CHAR     : id.fromText( (Char * )data); return id;
            case DATA_CHAR_PTR : id.fromText(*(Char **)data); return id;
            case DATA_STR8     : id.fromText(*(Str8 * )data); return id;
            case DATA_STR      : id.fromText(*(Str  * )data); return id;

            case DATA_INT :
            case DATA_UINT: switch(size)
            {
               case 1: return UID(*(U8 *)data, 0, 0, 0);
               case 2: return UID(*(U16*)data, 0, 0, 0);
               case 4: return UID(*(U32*)data, 0, 0, 0);
               case 8: return UID(*(U64*)data, 0);
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): return UID(RoundU(*(Half*)data), 0, 0, 0);
               case SIZE(Flt ): return UID(RoundU(*(Flt *)data), 0, 0, 0);
               case SIZE(Dbl ): return UID(RoundU(*(Dbl *)data), 0, 0, 0);
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): return UID(RoundU(((VecH2*)data)->x), RoundU(((VecH2*)data)->y), 0, 0);
               case SIZE(Vec2 ): return UID(RoundU(((Vec2 *)data)->x), RoundU(((Vec2 *)data)->y), 0, 0);
               case SIZE(VecD2): return UID(RoundU(((VecD2*)data)->x), RoundU(((VecD2*)data)->y), 0, 0);
            }break;
            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): return UID(RoundU(((VecH*)data)->x), RoundU(((VecH*)data)->y), RoundU(((VecH*)data)->z), 0);
               case SIZE(Vec ): return UID(RoundU(((Vec *)data)->x), RoundU(((Vec *)data)->y), RoundU(((Vec *)data)->z), 0);
               case SIZE(VecD): return UID(RoundU(((VecD*)data)->x), RoundU(((VecD*)data)->y), RoundU(((VecD*)data)->z), 0);
            }break;
            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): return UID(RoundU(((VecH4*)data)->x), RoundU(((VecH4*)data)->y), RoundU(((VecH4*)data)->z), RoundU(((VecH4*)data)->w));
               case SIZE(Vec4 ): return UID(RoundU(((Vec4 *)data)->x), RoundU(((Vec4 *)data)->y), RoundU(((Vec4 *)data)->z), RoundU(((Vec4 *)data)->w));
               case SIZE(VecD4): return UID(RoundU(((VecD4*)data)->x), RoundU(((VecD4*)data)->y), RoundU(((VecD4*)data)->z), RoundU(((VecD4*)data)->w));
            }break;

            case DATA_VECI2: switch(size)
            {
             //case SIZE(VecSB2): return UID(..);
               case SIZE(VecI2 ): return UID(((VecI2*)data)->x, ((VecI2*)data)->y, 0, 0);
            }break;
            case DATA_VECI: switch(size)
            {
             //case SIZE(VecSB): return UID(..);
               case SIZE(VecI ): return UID(((VecI*)data)->x, ((VecI*)data)->y, ((VecI*)data)->z, 0);
            }break;
            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): return UID(((VecSB4*)data)->u, 0, 0, 0);
               case SIZE(VecI4 ): return UID(((VecI4 *)data)->x, ((VecI4*)data)->y, ((VecI4*)data)->z, ((VecI4*)data)->w);
            }break;

            case DATA_VECU2: switch(size)
            {
             //case SIZE(VecB2 ): return UID(..);
               case SIZE(VecUS2): return UID(((VecUS2*)data)->x, ((VecUS2*)data)->y, 0, 0);
            }break;
            case DATA_VECU: switch(size)
            {
             //case SIZE(VecB ): return UID(..);
               case SIZE(VecUS): return UID(((VecUS*)data)->x, ((VecUS*)data)->y, ((VecUS*)data)->z, 0);
            }break;
            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): return UID(((VecB4*)data)->u, 0, 0, 0);
            }break;

            case DATA_COLOR: switch(size)
            {
             //case SIZE(VecB ): return UID(..);
               case SIZE(Color): return UID(((VecB4*)data)->u, 0, 0, 0);
               case SIZE(VecH ): return UID(RoundU(((VecH *)data)->x), RoundU(((VecH *)data)->y), RoundU(((VecH *)data)->z), 0);
               case SIZE(Vec  ): return UID(RoundU(((Vec  *)data)->x), RoundU(((Vec  *)data)->y), RoundU(((Vec  *)data)->z), 0);
               case SIZE(VecD ): return UID(RoundU(((VecD *)data)->x), RoundU(((VecD *)data)->y), RoundU(((VecD *)data)->z), 0);
               case SIZE(VecH4): return UID(RoundU(((VecH4*)data)->x), RoundU(((VecH4*)data)->y), RoundU(((VecH4*)data)->z), RoundU(((VecH4*)data)->w));
               case SIZE(Vec4 ): return UID(RoundU(((Vec4 *)data)->x), RoundU(((Vec4 *)data)->y), RoundU(((Vec4 *)data)->z), RoundU(((Vec4 *)data)->w));
               case SIZE(VecD4): return UID(RoundU(((VecD4*)data)->x), RoundU(((VecD4*)data)->y), RoundU(((VecD4*)data)->z), RoundU(((VecD4*)data)->w));
            }break;

            case DATA_UID:
            {
               if(size==SIZE(UID))return *(UID*)data;
            }break;
         }
      }
   }
   return UIDZero;
}
/******************************************************************************/
void MemberDesc::fromUID(Ptr data, C UID &value)
{
   if(data)
   {
      if(_text_to_data)
      {
         _text_to_data(data, value.asHex());
      }else
      {
         data=(Byte*)data+offset;
         switch(type)
         {
            case DATA_CHAR8: Set((Char8*)data, value.asHex(), size  ); break;
            case DATA_CHAR : Set((Char *)data, value.asHex(), size/2); break;
            case DATA_STR8 :    *(Str8 *)data= value.asHex()         ; break;
            case DATA_STR  :    *(Str  *)data= value.asHex()         ; break;

            case DATA_BOOL: *(Bool*)data=value.valid(); break;

            case DATA_INT: switch(size)
            {
               case 1: *(I8 *)data=value.i[0]; break;
               case 2: *(I16*)data=value.i[0]; break;
               case 4: *(I32*)data=value.i[0]; break;
               case 8: *(I64*)data=value.l[0]; break;
            }break;

            case DATA_UINT: switch(size)
            {
               case 1: *(U8 *)data=value.i[0]; break;
               case 2: *(U16*)data=value.i[0]; break;
               case 4: *(U32*)data=value.i[0]; break;
               case 8: *(U64*)data=value.l[0]; break;
            }break;

            case DATA_REAL: switch(size)
            {
               case SIZE(Half): *(Half*)data=value.i[0]; break;
               case SIZE(Flt ): *(Flt *)data=value.i[0]; break;
               case SIZE(Dbl ): *(Dbl *)data=value.i[0]; break;
            }break;

            case DATA_VEC2: switch(size)
            {
               case SIZE(VecH2): ((VecH2*)data)->set(value.i[0], value.i[1]); break;
               case SIZE(Vec2 ): ((Vec2 *)data)->set(value.i[0], value.i[1]); break;
               case SIZE(VecD2): ((VecD2*)data)->set(value.i[0], value.i[1]); break;
            }break;

            case DATA_VEC: switch(size)
            {
               case SIZE(VecH): ((VecH*)data)->set(value.i[0], value.i[1], value.i[2]); break;
               case SIZE(Vec ): ((Vec *)data)->set(value.i[0], value.i[1], value.i[2]); break;
               case SIZE(VecD): ((VecD*)data)->set(value.i[0], value.i[1], value.i[2]); break;
            }break;

            case DATA_VEC4: switch(size)
            {
               case SIZE(VecH4): ((VecH4*)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
               case SIZE(Vec4 ): ((Vec4 *)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
               case SIZE(VecD4): ((VecD4*)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
            }break;

            case DATA_VECI2: switch(size)
            {
             //case SIZE(VecSB2): ((VecSB2*)data)->set(); break;
               case SIZE(VecI2 ): ((VecI2 *)data)->set(value.i[0], value.i[1]); break;
            }break;

            case DATA_VECI: switch(size)
            {
             //case SIZE(VecSB): ((VecSB*)data)->set(); break;
               case SIZE(VecI ): ((VecI *)data)->set(value.i[0], value.i[1], value.i[2]); break;
            }break;

            case DATA_VECI4: switch(size)
            {
               case SIZE(VecSB4): ((VecSB4*)data)->u=value.i[0]; break;
               case SIZE(VecI4 ): ((VecI4 *)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
            }break;

            case DATA_VECU2: switch(size)
            {
             //case SIZE(VecB2 ): ((VecB2 *)data)->set(); break;
               case SIZE(VecUS2): ((VecUS2*)data)->set(value.i[0], value.i[1]); break;
            }break;

            case DATA_VECU: switch(size)
            {
             //case SIZE(VecB ): ((VecB *)data)->set(); break;
               case SIZE(VecUS): ((VecUS*)data)->set(value.i[0], value.i[1], value.i[2]); break;
            }break;

            case DATA_VECU4: switch(size)
            {
               case SIZE(VecB4): ((VecB4*)data)->u=value.i[0]; break;
            }break;

            case DATA_COLOR: switch(size)
            {
             //case SIZE(VecB ): ((VecB *)data)->u=; break;
               case SIZE(VecB4): ((VecB4*)data)->u=value.i[0]; break;
               case SIZE(VecH ): ((VecH *)data)->set(value.i[0], value.i[1], value.i[2]); break;
               case SIZE(Vec  ): ((Vec  *)data)->set(value.i[0], value.i[1], value.i[2]); break;
               case SIZE(VecD ): ((VecD *)data)->set(value.i[0], value.i[1], value.i[2]); break;
               case SIZE(VecH4): ((VecH4*)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
               case SIZE(Vec4 ): ((Vec4 *)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
               case SIZE(VecD4): ((VecD4*)data)->set(value.i[0], value.i[1], value.i[2], value.i[3]); break;
            }break;

            case DATA_UID: switch(size)
            {
               case SIZE(UID): *(UID*)data=value; break;
            }break;
         }
      }
   }
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
Image* MemberDesc::asImage(CPtr data)C
{
   if(data)
   {
      data=(Byte*)data+offset;
      switch(type)
      {
         case DATA_IMAGE    : return   (Image   *)data   ;
         case DATA_IMAGE_PTR: return  *(Image*  *)data   ;
         case DATA_IMAGEPTR : return (*(ImagePtr*)data)();
      }
   }
   return null;
}
/******************************************************************************/
Bool MemberDesc::integer()C
{
   switch(type)
   {
      case DATA_BOOL :
      case DATA_INT  :
      case DATA_UINT :
      case DATA_VECI2:
      case DATA_VECI :
      case DATA_VECI4:
      case DATA_VECU2:
      case DATA_VECU :
      case DATA_VECU4:
      case DATA_UID  :
         return true;

      case DATA_COLOR: return size==SIZE(VecB) || size==SIZE(Color);
   }
   return false;
}
Bool MemberDesc::dblPrecision()C
{
   switch(type)
   {
      case DATA_REAL : return size==SIZE(Dbl  );
      case DATA_VEC2 : return size==SIZE(VecD2);
      case DATA_VEC  : return size==SIZE(VecD );
      case DATA_VEC4 : return size==SIZE(VecD4);
      case DATA_COLOR: return size==SIZE(VecD ) || size==SIZE(VecD4);
   }
   return false;
}
/******************************************************************************/
// COMPARE
/******************************************************************************/
Int MemberDesc::compare(CPtr obj0, CPtr obj1)C
{
   if(obj0 && obj1)
   {
      if(_compare     )return _compare(obj0, obj1);
      if(_data_to_text)return  CompareNumber(_data_to_text(obj0), _data_to_text(obj1));

      Byte *d0=(Byte*)obj0+offset,
           *d1=(Byte*)obj1+offset;
      switch(type)
      {
         case DATA_CHAR8    : return CompareNumber( (Char8* )d0,  (Char8* )d1);
         case DATA_CHAR     : return CompareNumber( (Char * )d0,  (Char * )d1);
         case DATA_CHAR8_PTR: return CompareNumber(*(Char8**)d0, *(Char8**)d1);
         case DATA_CHAR_PTR : return CompareNumber(*(Char **)d0, *(Char **)d1);
         case DATA_STR8     : return CompareNumber(*(Str8 * )d0, *(Str8 * )d1);
         case DATA_STR      : return CompareNumber(*(Str  * )d0, *(Str  * )d1);

         case DATA_BOOL:
         {
            Int i0=*(Bool*)d0,
                i1=*(Bool*)d1;
            if(i0<i1)return -1;
            if(i0>i1)return +1;
         }break;

         case DATA_INT:
         {
            Int i0, i1;
            switch(size)
            {
               case  1: i0=*(I8 *)d0; i1=*(I8 *)d1; break;
               case  2: i0=*(I16*)d0; i1=*(I16*)d1; break;
               case  4: i0=*(I32*)d0; i1=*(I32*)d1; break;
               case  8: {I64 i0=*(I64*)d0, i1=*(I64*)d1; if(i0<i1)return -1; if(i0>i1)return +1;}
               default: return 0;
            }
            if(i0<i1)return -1;
            if(i0>i1)return +1;
         }break;

         case DATA_PTR :
         case DATA_UINT:
         {
            UInt u0, u1;
            switch(size)
            {
               case  1: u0=*(U8 *)d0; u1=*(U8 *)d1; break;
               case  2: u0=*(U16*)d0; u1=*(U16*)d1; break;
               case  4: u0=*(U32*)d0; u1=*(U32*)d1; break;
               case  8: {U64 u0=*(U64*)d0, u1=*(U64*)d1; if(u0<u1)return -1; if(u0>u1)return +1;}
               default: return 0;
            }
            if(u0<u1)return -1;
            if(u0>u1)return +1;
         }break;

         case DATA_REAL: switch(size)
         {
            case SIZE(Half): return Compare(*(Half*)d0, *(Half*)d1);
            case SIZE(Flt ): return Compare(*(Flt *)d0, *(Flt *)d1);
            case SIZE(Dbl ): return Compare(*(Dbl *)d0, *(Dbl *)d1);
         }break;

         case DATA_VEC2: switch(size)
         {
            case SIZE(VecH2): return Compare(*(VecH2*)d0, *(VecH2*)d1);
            case SIZE(Vec2 ): return Compare(*(Vec2 *)d0, *(Vec2 *)d1);
            case SIZE(VecD2): return Compare(*(VecD2*)d0, *(VecD2*)d1);
         }break;

         case DATA_VEC: switch(size)
         {
            case SIZE(VecH): return Compare(*(VecH*)d0, *(VecH*)d1);
            case SIZE(Vec ): return Compare(*(Vec *)d0, *(Vec *)d1);
            case SIZE(VecD): return Compare(*(VecD*)d0, *(VecD*)d1);
         }break;

         case DATA_VEC4: switch(size)
         {
            case SIZE(VecH4): return Compare(*(VecH4*)d0, *(VecH4*)d1);
            case SIZE(Vec4 ): return Compare(*(Vec4 *)d0, *(Vec4 *)d1);
            case SIZE(VecD4): return Compare(*(VecD4*)d0, *(VecD4*)d1);
         }break;

         case DATA_VECI2: switch(size)
         {
            case SIZE(VecSB2): return Compare(*(VecSB2*)d0, *(VecSB2*)d1);
            case SIZE(VecI2 ): return Compare(*(VecI2 *)d0, *(VecI2 *)d1);
         }break;

         case DATA_VECI: switch(size)
         {
            case SIZE(VecSB): return Compare(*(VecSB*)d0, *(VecSB*)d1);
            case SIZE(VecI ): return Compare(*(VecI *)d0, *(VecI *)d1);
         }break;

         case DATA_VECI4: switch(size)
         {
            case SIZE(VecSB4): return Compare(*(VecSB4*)d0, *(VecSB4*)d1);
            case SIZE(VecI4 ): return Compare(*(VecI4 *)d0, *(VecI4 *)d1);
         }break;

         case DATA_VECU2: switch(size)
         {
            case SIZE(VecB2 ): return Compare(*(VecB2 *)d0, *(VecB2 *)d1);
            case SIZE(VecUS2): return Compare(*(VecUS2*)d0, *(VecUS2*)d1);
         }break;

         case DATA_VECU: switch(size)
         {
            case SIZE(VecB ): return Compare(*(VecB *)d0, *(VecB *)d1);
            case SIZE(VecUS): return Compare(*(VecUS*)d0, *(VecUS*)d1);
         }break;

         case DATA_VECU4: switch(size)
         {
            case SIZE(VecB4): return Compare(*(VecB4*)d0, *(VecB4*)d1);
         }break;

         case DATA_DATE_TIME:
         {
            if(size==SIZE(DateTime))return Compare(*(DateTime*)d0, *(DateTime*)d1);
         }break;

         case DATA_UID:
         {
            if(size==SIZE(UID))return Compare(*(UID*)d0, *(UID*)d1);
         }break;

         case DATA_IMAGE_PTR:                      return CompareImage( *(Image*  *)d0   ,  *(Image*  *)d1   );
         case DATA_IMAGEPTR :                      return CompareImage((*(ImagePtr*)d0)(), (*(ImagePtr*)d1)());
         case DATA_IMAGE    : if(size==SIZE(Image))return CompareImage(  (Image   *)d0   ,   (Image   *)d1   ); break;
      }
   }
   return 0;
}
/******************************************************************************/
}
/******************************************************************************/
