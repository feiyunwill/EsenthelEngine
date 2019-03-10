/******************************************************************************

   Have to keep this as a separate file, so it won't be linked if unused.
   Because it's linked separately, its name can't include spaces (due to Android building toolchain).

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
#define ETC_LIB_ISPC    1 // only ETC1 compression, quality is good and comparable to ETC_LIB_ETCPACK with quality=0, but much faster
#define ETC_LIB_RG      2 // only ETC1
#define ETC_LIB_ETCPACK 3

#define ETC1_ENC ETC_LIB_ISPC
#define ETC2_ENC ETC_LIB_ETCPACK

#include "../../../../ThirdPartyLibs/begin.h"

#if ETC1_ENC==ETC_LIB_ISPC
   #ifndef ISPC_C__ESENTHEL_THIRDPARTYLIBS_BC7_ISPC_TEXCOMP_KERNEL_ISPC_H // this helps on Android where this header could have been already included due to files packed to one CPP file
      #include "../../../../ThirdPartyLibs/BC7/ispc_texcomp/ispc_texcomp.h"
   #endif
#endif

#if ETC1_ENC==ETC_LIB_RG
   #include "../../../../ThirdPartyLibs/RG-ETC1/rg_etc1.cpp"
#endif

#if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK
   namespace ETCPACK
   {
      #define EXHAUSTIVE_CODE_ACTIVE 0 // disable to reduce code size and because this mode is too slow
      #define printf(x,...)
      #define   exit(x)
      #pragma warning(push)
      #pragma warning(disable:4390) // ';': empty controlled statement found; is this the intent?
      #pragma runtime_checks("", off)
      #include "../../../../ThirdPartyLibs/ETCPack/source/etcdec.cxx"
      #undef exit
      #include "../../../../ThirdPartyLibs/ETCPack/source/etcpack.cxx"
      #pragma runtime_checks("", restore)
      #pragma warning(pop)
      #undef R
      #undef G
      #undef B
      #undef RED
      #undef GREEN
      #undef BLUE
      #undef SHIFT
      #undef MASK

   #if ETC2_ENC==ETC_LIB_ETCPACK
      static struct ETCInit
      {
         ETCInit() {setupAlphaTable();}
      }EI;
   #endif
   }
#endif


#include "../../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
Bool _CompressETC(C Image &src, Image &dest, Int quality, Bool perceptual)
{
   Bool ok=false, etc1=(dest.hwType()==IMAGE_ETC1);
   if(src.d()==dest.d())
   if(etc1 || dest.hwType()==IMAGE_ETC2 || dest.hwType()==IMAGE_ETC2_A1 || dest.hwType()==IMAGE_ETC2_A8)
   {
      Image temp; C Image *s=&src;
      if(etc1 && ETC1_ENC==ETC_LIB_ISPC)
      {
         if(s->hwType()!=IMAGE_R8G8B8A8 || s->w()!=dest.hwW() || s->h()!=dest.hwH()) // ISPC requires R8G8B8A8 format
         {
            if(s->copyTry(temp, dest.hwW(), dest.hwH(), -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1, FILTER_NO_STRETCH, true))s=&temp;else return false; // we need to cover the area for entire HW size, to process partial blocks too
         }
      }
      if(s->lockRead())
      {
         if(dest.lock(LOCK_WRITE))
         {
            ok=true;
            Int x_blocks=dest.hwW()/4, // operate on HW size to process partial and Pow2Padded blocks too
                y_blocks=dest.hwH()/4;
         #if ETC1_ENC==ETC_LIB_ISPC // ISPC
            Bool             fast;
            etc_enc_settings settings;
            rgba_surface     surf;
         #endif
         #if ETC1_ENC==ETC_LIB_RG // RG
            rg_etc1::etc1_pack_params pack;
         #endif
         #if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
            Int block_size;
         #endif

         #if ETC1_ENC==ETC_LIB_ISPC // ISPC
            if(etc1 && ETC1_ENC==ETC_LIB_ISPC)
            {
               fast=(dest.pitch()==x_blocks*8);
               GetProfile_etc_slow(&settings);
               surf.width =s->w    ();
               surf.height=s->h    ();
               surf.stride=s->pitch();
            }else
         #endif
         #if ETC1_ENC==ETC_LIB_RG // RG
            if(etc1 && ETC1_ENC==ETC_LIB_RG)
            {
               rg_etc1::pack_etc1_block_init();
               switch(quality)
               {
                  case 0: pack.m_quality=rg_etc1::cLowQuality   ; break; // makes gradients look bad
         default: case 1: pack.m_quality=rg_etc1::cMediumQuality; break; // much faster than high and almost as good
                  case 2: pack.m_quality=rg_etc1::cHighQuality  ; break; // very slow
               }
               pack.m_dithering=false; // dithering here is actually very bad
            }else
         #endif
         #if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
            if(etc1 ? ETC1_ENC==ETC_LIB_ETCPACK : ETC2_ENC==ETC_LIB_ETCPACK)
            {
               if(quality<0)quality=0; // default to 0 as 1 is just too slow
               quality=Mid(quality, 0, EXHAUSTIVE_CODE_ACTIVE)*2+Mid(perceptual, 0, 1);
               block_size=ImageTI[dest.hwType()].bit_pp*2;
            }else
         #endif
               {}

            REPD(z, dest.d())
            {
               // compress
            #if ETC1_ENC==ETC_LIB_ISPC // ISPC
               if(etc1 && ETC1_ENC==ETC_LIB_ISPC)
               {
                  surf.ptr=ConstCast(s->data() + z*s->pitch2());
                  if(fast)CompressBlocksETC1(&surf, dest.data()+z*dest.pitch2(), &settings);else
                  {
                     Byte *d=dest.data() + z*dest.pitch2();
                     surf.height=4;
                     REP(y_blocks)
                     {
                        CompressBlocksETC1(&surf, d, &settings);
                        surf.ptr+=s  ->pitch()*4;
                        d       +=dest.pitch();
                     }
                  }
               }else
            #endif
            #if ETC1_ENC==ETC_LIB_RG // RG
               if(etc1 && ETC1_ENC==ETC_LIB_RG)
               {
                  REPD(by, y_blocks)
                  {
                     Int py=by*4, yo[4]; REPAO(yo)=Min(py+i, src.h()-1); // use clamping to avoid black borders
                     Byte *dest_data=dest.data() + by*dest.pitch() + z*dest.pitch2();
                     REPD(bx, x_blocks)
                     {
                        Int px=bx*4, xo[4]; REPAO(xo)=Min(px+i, src.w()-1); // use clamping to avoid black borders
                        Color rgba[4][4]; src.gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &z, 1);
                        rg_etc1::pack_etc1_block((UInt*)(dest_data + bx*8), (UInt*)rgba, pack);
                     }
                  }
               }else
            #endif
            #if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
               if(etc1 ? ETC1_ENC==ETC_LIB_ETCPACK : ETC2_ENC==ETC_LIB_ETCPACK)
               {
                  REPD(by, y_blocks)
                  {
                     Int py=by*4, yo[4]; REPAO(yo)=Min(py+i, src.h()-1); // use clamping to avoid black borders
                     Byte *dest_data=dest.data() + by*dest.pitch() + z*dest.pitch2();
                     REPD(bx, x_blocks)
                     {
                        Int px=bx*4, xo[4]; REPAO(xo)=Min(px+i, src.w()-1); // use clamping to avoid black borders
                        VecB  rgb [4][4];
                        Color rgba[4][4];
                        Byte     a[4][4];
                        UInt *d=(UInt*)(dest_data + bx*block_size);
                        Color temp[4][4];
                        switch(dest.hwType())
                        {
                           case IMAGE_ETC1: src.gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &z, 1); switch(quality)
                           {
                              case 0: ETCPACK::compressBlockDiffFlipFast            (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                              case 1: ETCPACK::compressBlockDiffFlipFastPerceptual  (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                           #if EXHAUSTIVE_CODE_ACTIVE
                              case 2: ETCPACK::compressBlockETC1Exhaustive          (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                              case 3: ETCPACK::compressBlockETC1ExhaustivePerceptual(rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                           #endif
                           }break;

                           case IMAGE_ETC2: src.gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &z, 1); switch(quality)
                           {
                              case 0: ETCPACK::compressBlockETC2Fast                (rgb[0][0].c, null, temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGB_NO_MIPMAPS); break;
                              case 1: ETCPACK::compressBlockETC2FastPerceptual      (rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                           #if EXHAUSTIVE_CODE_ACTIVE
                              case 2: ETCPACK::compressBlockETC2Exhaustive          (rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                              case 3: ETCPACK::compressBlockETC2ExhaustivePerceptual(rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                           #endif
                           }break;

                           case IMAGE_ETC2_A1:
                           {
                              src.gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &z, 1);
                              REPD(y, 4)
                              REPD(x, 4)
                              {
                                 rgb[y][x]=  rgba[y][x].v3;
                                 a  [y][x]=((rgba[y][x].a>=128) ? 255 : 0); // manually adjust because compressor has issues
                              }
                              ETCPACK::compressBlockETC2Fast(rgb[0][0].c, &a[0][0], temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGBA1_NO_MIPMAPS);
                           }break;

                           case IMAGE_ETC2_A8:
                           {
                              src.gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &z, 1);
                              REPD(y, 4)
                              REPD(x, 4)
                              {
                                 rgb[y][x]=rgba[y][x].v3;
                                 a  [y][x]=rgba[y][x].a;
                              }

                              switch(quality)
                              {
                                 case 0: case 1: ETCPACK::compressBlockAlphaFast(&a[0][0], 0, 0, 4, 4, (Byte*)d); break;
                                 case 2: case 3: ETCPACK::compressBlockAlphaSlow(&a[0][0], 0, 0, 4, 4, (Byte*)d); break;
                              }

                              d+=2;

                              switch(quality)
                              {
                                 case 0: ETCPACK::compressBlockETC2Fast                (rgb[0][0].c, null, temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGB_NO_MIPMAPS); break;
                                 case 1: ETCPACK::compressBlockETC2FastPerceptual      (rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                              #if EXHAUSTIVE_CODE_ACTIVE
                                 case 2: ETCPACK::compressBlockETC2Exhaustive          (rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                                 case 3: ETCPACK::compressBlockETC2ExhaustivePerceptual(rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                              #endif
                              }
                           }break;
                        }
                        SwapEndian(d[0]);
                        SwapEndian(d[1]);
                     }
                  }
               }else
            #endif
                  {}

               if(!ok)break;
            }
            dest.unlock();
         }
         s->unlock();
      }
   }
   return ok;
}
/******************************************************************************/
}
/******************************************************************************/
