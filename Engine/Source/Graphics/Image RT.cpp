/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum IMAGE_TYPE_CREATE_RESULT : Byte
{
   UNKNOWN,
   FAILED ,
   SUCCESS,
};
static IMAGE_TYPE_CREATE_RESULT ImageTypeCreateResult[2][IMAGE_ALL_TYPES]; // [MultiSample][IMAGE_TYPE]
struct ImageRTType
{
   IMAGE_TYPE types[6]; // 6 is max IMAGE_TYPE elms per ImageRTType
};
static const ImageRTType ImageRTTypes[]=
{
   {IMAGE_DEFAULT                                                                               }, // 0 IMAGERT_RGBA
   {IMAGE_R10G10B10A2  , IMAGE_DEFAULT                                                          }, // 1 IMAGERT_RGB
   {IMAGE_R10G10B10A2  , IMAGE_F16_3, IMAGE_F16_4, IMAGE_DEFAULT                                }, // 2 IMAGERT_RGB_P
   {IMAGE_F16_4        , IMAGE_DEFAULT                                                          }, // 3 IMAGERT_RGBA_H
   {IMAGE_F16_3        , IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_DEFAULT                          }, // 4 IMAGERT_RGB_H
   {IMAGE_F32_4        , IMAGE_F16_4, IMAGE_DEFAULT                                             }, // 5 IMAGERT_RGBA_F
   {IMAGE_F32_3        , IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_DEFAULT}, // 6 IMAGERT_RGB_F
   {IMAGE_R8G8B8A8_SIGN, IMAGE_F16_4                                                            }, // 7 IMAGERT_RGBA_S
   {IMAGE_F32          , IMAGE_F16                                                              }, // 8 IMAGERT_F32
   {IMAGE_F16          , IMAGE_F32                                                              }, // 9 IMAGERT_F16
   {
   #if DX9
      IMAGE_A8, IMAGE_L8A8,
   #else
      IMAGE_R8, IMAGE_R8G8,
   #endif
      IMAGE_DEFAULT}, // 10 IMAGERT_ONE
   {
   #if !DX9
      IMAGE_R8_SIGN, IMAGE_R8G8_SIGN,
   #endif
      IMAGE_F16,
   #if !DX9
      IMAGE_R8G8B8A8_SIGN,
   #endif
      IMAGE_F32}, // 11 IMAGERT_ONE_S
   {
   #if !DX9
      IMAGE_R8G8,
   #endif
      IMAGE_DEFAULT}, // 12 IMAGERT_TWO
   {
   #if !DX9
      IMAGE_R8G8_SIGN, IMAGE_R8G8B8A8_SIGN,
   #endif
      IMAGE_F16_2}, // 13 IMAGERT_TWO_S
   {
   #if DX9
      IMAGE_INTZ, /*IMAGE_RAWZ, */IMAGE_DF24, // read why IMAGE_RAWZ is disabled in 'RendererClass::rtCreate()'
   #endif
      IMAGE_D24S8, IMAGE_D24X8, IMAGE_D32, IMAGE_D16}, // 14 IMAGERT_DS
}; ASSERT(IMAGERT_RGBA==0 && IMAGERT_RGB==1 && IMAGERT_RGB_P==2 && IMAGERT_RGBA_H==3 && IMAGERT_RGB_H==4 && IMAGERT_RGBA_F==5 && IMAGERT_RGB_F==6 && IMAGERT_RGBA_S==7 && IMAGERT_F32==8 && IMAGERT_F16==9 && IMAGERT_ONE==10 && IMAGERT_ONE_S==11 && IMAGERT_TWO==12 && IMAGERT_TWO_S==13 && IMAGERT_DS==14 && IMAGERT_NUM==15);
static ImageRTType ImageRTTypesOK[2][Elms(ImageRTTypes)]; // [MultiSample][IMAGERT_NUM], this keeps info about result of creating different IMAGE_TYPE for 1-sample and multi-sample, this is because some formats may fail to create multi-sampled but succeed with 1-sample (for simplication this assumes that there will be only one type of multi-sample, like only 4x, but not 4x and 8x)
static CChar8 *ImageRTName[]=
{
   "RGBA"  , // 0
   "RGB"   , // 1
   "RGB_P" , // 2
   "RGBA_H", // 3
   "RGB_H" , // 4
   "RGBA_F", // 5
   "RGB_F" , // 6
   "RGBA_S", // 7
   "F32"   , // 8
   "F16"   , // 9
   "ONE"   , // 10
   "ONE_S" , // 11
   "TWO"   , // 12
   "TWO_S" , // 13
   "DS"    , // 14
}; ASSERT(IMAGERT_RGBA==0 && IMAGERT_RGB==1 && IMAGERT_RGB_P==2 && IMAGERT_RGBA_H==3 && IMAGERT_RGB_H==4 && IMAGERT_RGBA_F==5 && IMAGERT_RGB_F==6 && IMAGERT_RGBA_S==7 && IMAGERT_F32==8 && IMAGERT_F16==9 && IMAGERT_ONE==10 && IMAGERT_ONE_S==11 && IMAGERT_TWO==12 && IMAGERT_TWO_S==13 && IMAGERT_DS==14 && IMAGERT_NUM==15);
/******************************************************************************/
static const IMAGERT_TYPE GetImageRTTypeLookup[IMAGE_PRECISION_NUM][2]= // [precision][alpha]
{
   // no alpha   ,     alpha
   {IMAGERT_RGB  , IMAGERT_RGBA  }, // 0 IMAGE_PRECISION_8
   {IMAGERT_RGB_P, IMAGERT_RGBA_P}, // 1 IMAGE_PRECISION_10
   {IMAGERT_RGB_H, IMAGERT_RGBA_H}, // 2 IMAGE_PRECISION_16
   {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 3 IMAGE_PRECISION_24
   {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 4 IMAGE_PRECISION_32
   {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 5 IMAGE_PRECISION_64
}; ASSERT(IMAGE_PRECISION_8==0 && IMAGE_PRECISION_10==1 && IMAGE_PRECISION_16==2 && IMAGE_PRECISION_24==3 && IMAGE_PRECISION_32==4 && IMAGE_PRECISION_64==5 && IMAGE_PRECISION_NUM==6);
IMAGERT_TYPE GetImageRTType(                 Bool       alpha, IMAGE_PRECISION     precision) {return GetImageRTTypeLookup[precision][alpha];}
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha, IMAGE_PRECISION max_precision)
{
   if(type==IMAGE_R10G10B10A2)return IMAGERT_RGB; // because the implementation operates on Bool alpha (yes/no) and not how many alpha bits we want, then as result we may actually increase precision when operating on 'IMAGE_R10G10B10A2' with 'allow_alpha'=true, so treat this as a special case and always return IMAGERT_RGB
 C ImageTypeInfo &ti=ImageTI[type]; return GetImageRTType(ti.a && allow_alpha, Min(ti.precision, max_precision));
}
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha)
{
   if(type==IMAGE_R10G10B10A2)return IMAGERT_RGB; // because the implementation operates on Bool alpha (yes/no) and not how many alpha bits we want, then as result we may actually increase precision when operating on 'IMAGE_R10G10B10A2' with 'allow_alpha'=true, so treat this as a special case and always return IMAGERT_RGB
 C ImageTypeInfo &ti=ImageTI[type]; return GetImageRTType(ti.a && allow_alpha, ti.precision);
}
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type)
{
   if(type==IMAGE_R10G10B10A2)return IMAGERT_RGB; // because the implementation operates on Bool alpha (yes/no) and not how many alpha bits we want, then as result we may actually increase precision when operating on 'IMAGE_R10G10B10A2', so treat this as a special case and always return IMAGERT_RGB
 C ImageTypeInfo &ti=ImageTI[type]; return GetImageRTType(ti.a>0, ti.precision);
}
/******************************************************************************/
void ResetImageTypeCreateResult()
{
   Zero(ImageTypeCreateResult); ASSERT(UNKNOWN==0); // assume that all are UNKNOWN (zero)
   CopyFast(ImageRTTypesOK[0], ImageRTTypes);
   CopyFast(ImageRTTypesOK[1], ImageRTTypes);
}
static Int CompareDesc(C ImageRC &image, C ImageRTDesc &desc)
{
   if(Int c=Compare(image.w      (), desc.size.x ))return c;
   if(Int c=Compare(image.h      (), desc.size.y ))return c;
   if(Int c=Compare(image.type   (), desc._type  ))return c;
   if(Int c=Compare(image.samples(), desc.samples))return c;
   return 0;
}
static inline Bool ImageRTCreate(ImageRC &image, C ImageRTDesc &desc)
{
   Bool ok;
   if(ImageTI[desc._type].d) // if this is a depth buffer
   {
             ok=image.createTryEx(desc.size.x, desc.size.y, 1, desc._type, IMAGE_DS_RT, 1, desc.samples); // try first as a render target
   #if DX9 || GL // for DX10+ IMAGE_DS_RT is the same as IMAGE_DS so don't bother checking it again
      if(!ok)ok=image.createTryEx(desc.size.x, desc.size.y, 1, desc._type, IMAGE_DS   , 1, desc.samples);
   #endif
   }else
   {
      ok=image.createTryEx(desc.size.x, desc.size.y, 1, desc._type, IMAGE_RT, 1, desc.samples);
   }
   if(ok)Time.skipUpdate();
   return ok;
}
static inline void Set(ImageRTPtr &p, ImageRC &rt) // this is called only when "_ptr_num==0"
{
   rt._ptr_num++; p._data=&rt; rt.discard();
}
ImageRTPtr& ImageRTPtr::clear()
{
   if(_data)
   {
      DEBUG_ASSERT(_data->_ptr_num, "ImageRC._ptr_num should be >0");
          _data->_ptr_num--;
      if(!_data->_ptr_num)_data->discard();
          _data=null;
   }
   return T;
}
ImageRTPtr& ImageRTPtr::operator=(C ImageRTPtr &p)
{
   if(T!=p)
   {
      clear();
      if(T._data=p._data){T._data->_ptr_num++; T._last_index=p._last_index;} // assign to new
   }
   return T;
}
ImageRTPtr& ImageRTPtr::operator=(ImageRC *p)
{
   if(T!=p)
   {
      clear();
      if(T._data=p)T._data->_ptr_num++; // assign to new
   }
   return T;
}
ImageRTPtr::ImageRTPtr(C ImageRTPtr &p)
{
   if(T._data=p._data){T._data->_ptr_num++; T._last_index=p._last_index;}else T._last_index=-1;
}
ImageRTPtr::ImageRTPtr(ImageRC *p)
{
   if(T._data=p)T._data->_ptr_num++;else T._last_index=-1;
}
/******************************************************************************/
Bool ImageRTPtr::find(C ImageRTDesc &desc)
{
   clear(); // clear first so we can find the same Image if possible

   Bool                       multi_sample          =(desc.samples>1);
   IMAGE_TYPE_CREATE_RESULT (&itcr)[IMAGE_ALL_TYPES]=ImageTypeCreateResult[multi_sample];
   ImageRTType               &types                 =ImageRTTypesOK       [multi_sample][desc.rt_type];
again:
   ConstCast(desc._type)=types.types[0];

   Bool found; if(InRange(_last_index, Renderer._rts) && !CompareDesc(Renderer._rts[_last_index], desc))found=true; // in range and matches ("!CompareDesc" -> match)
   else found=Renderer._rts.binarySearch(desc, _last_index, CompareDesc);

   if(found)
   {
      // check '_last_index' first
      ImageRC &rt=Renderer._rts[_last_index];
      if(rt.available()){Set(T, rt); return true;}

      // check all neighbors with the same desc
      for(Int i=_last_index-1;         i>=0             ; i--) {ImageRC &rt=Renderer._rts[i]; if(CompareDesc(rt, desc))break; if(rt.available()){Set(T, rt); T._last_index=i; return true;}}
      for(Int i=_last_index+1; InRange(i, Renderer._rts); i++) {ImageRC &rt=Renderer._rts[i]; if(CompareDesc(rt, desc))break; if(rt.available()){Set(T, rt); T._last_index=i; return true;}}
   }

   switch(itcr[desc._type])
   {
      case UNKNOWN:
      {
         ImageRC temp; Bool ok=ImageRTCreate(temp, desc); // try to create first as a standalone variable (not in 'Renderer._rts') in case it fails so we don't have to remove it
         itcr[desc._type]=(ok ? SUCCESS : FAILED);
         if(ok){ImageRC &rt=Renderer._rts.NewAt(_last_index); Swap(rt, temp); Set(T, rt); return true;}
         // fail
         if(desc._type!=IMAGE_NONE)
         {
            MoveFastN(&types.types[0], &types.types[1], ELMS(types.types)-1); // move all elements from index 1 and right, to the left by 1, to index 0..
            types.types[ELMS(types.types)-1]=IMAGE_NONE; // set last type as none
            if(types.types[0]!=IMAGE_NONE)goto again; // try the new type
         }
      }break;

      case SUCCESS:
      {
         ImageRC &rt=Renderer._rts.NewAt(_last_index); if(!ImageRTCreate(rt, desc))Exit(S+"Can't create Render Target "+desc.size.x+'x'+desc.size.y); // Exit because SUCCESS always assumes to succeed
         Set(T, rt); return true;
      }break;
   }
   return false;
}
/******************************************************************************/
ImageRTPtr& ImageRTPtr::get(C ImageRTDesc &desc)
{
   if(!find(desc))Exit(S+"Can't create Render Target "+desc.size.x+'x'+desc.size.y+' '+ImageRTName[desc.rt_type]+", samples:"+desc.samples);
   return T;
}
ImageRTPtr& ImageRTPtr::getDS(Int w, Int h, Byte samples, Bool reuse_main)
{
   clear(); // clear first so we can find the same Image if possible
   if(reuse_main)
   {
      ImageRC &ds=Renderer._main_ds, *cur_ds=Renderer._cur_main_ds;
   #if GL
      // on OpenGL we can't reuse '_main_ds' because it has different flipping orientation as it is always paired with '_main' in the main FBO
      // <- here don't check for 'ds'
      if(cur_ds==&ds)cur_ds=null; // if 'cur_ds' is 'ds' then don't check it either
   #else
      if(              /*ds .available() && */    ds .accessible() &&     ds .w()==w &&     ds .h()==h &&     ds .samples()==samples){T=   &ds; return T;} // if ds is not used (actually don't check this because '_gui_ds' can be set to it at the start of each frame), accessible and compatible then we can use it
   #endif
      if(cur_ds && /*cur_ds->available() && */cur_ds->accessible() && cur_ds->w()==w && cur_ds->h()==h && cur_ds->samples()==samples){T=cur_ds; return T;} // if ds is not used (actually don't check this because '_gui_ds' can be set to it at the start of each frame), accessible and compatible then we can use it
   }
   get(ImageRTDesc(w, h, IMAGERT_DS, samples)); return T;
}
/******************************************************************************/
Bool        ImageRTPtr::find(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples) {return find(ImageRTDesc(w, h, rt_type, samples));}
ImageRTPtr& ImageRTPtr:: get(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples) {return  get(ImageRTDesc(w, h, rt_type, samples));}
/******************************************************************************/
}
/******************************************************************************/
