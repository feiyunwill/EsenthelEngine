/******************************************************************************

   Have to keep this as a separate file, so it won't be linked if unused.
   Because it's linked separately, its name can't include spaces (due to Android building toolchain).

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
#include "../../../../ThirdPartyLibs/begin.h"
#include "../../../../ThirdPartyLibs/PVRTC/PVRTex/PVRTextureDefines.h"
#include "../../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
extern Bool _CompressPVRTC(Int w, Int h, CPtr data, Int type, Int quality, Ptr &compressed_data, Int &compressed_size);
/******************************************************************************/
Bool _CompressPVRTC(C Image &src, Image &dest, Int quality)
{
   Bool ok=false;
   if((dest.hwType()==IMAGE_PVRTC1_2 || dest.hwType()==IMAGE_PVRTC1_4) && src.lockRead())
   {
      if(dest.lock(LOCK_WRITE))
      {
         Image temp; if(temp.createTry(PaddedWidth(dest.w(), dest.h(), 0, dest.hwType()), PaddedHeight(dest.w(), dest.h(), 0, dest.hwType()), 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1)) //  use R8G8B8A8 because PVRTEX operates on that format
         {
            ok=true;

            REPD(z, dest.d())
            {
               // copy 'src' to 'temp'
               REPD(y, temp.h())
               REPD(x, temp.w())temp.color(x, y, src.color3D(Min(x, src.w()-1), Min(y, src.h()-1), z)); // use clamping to avoid black borders

               // compress
               Ptr data=null;
               Int size=0;
               if(quality<0)quality=((dest.hwType()==IMAGE_PVRTC1_2) ? GetPVRTCQuality() : 4); // for PVRTC1_2 default to specified settings, for others use best quality as it's not so slow for those types
               switch(quality)
               {
                  case  0: quality=pvrtexture::ePVRTCFastest; break;
                  case  1: quality=pvrtexture::ePVRTCFast   ; break;
                  case  2: quality=pvrtexture::ePVRTCNormal ; break;
                  case  3: quality=pvrtexture::ePVRTCHigh   ; break;
                  default: quality=pvrtexture::ePVRTCBest   ; break;
               }
               if(_CompressPVRTC(temp.w(), temp.h(), temp.data(), (dest.hwType()==IMAGE_PVRTC1_2) ? ePVRTPF_PVRTCI_2bpp_RGBA : ePVRTPF_PVRTCI_4bpp_RGBA, quality, data, size))
               {
                  if(size==temp.w()*temp.h()*ImageTI[dest.hwType()].bit_pp/8)
                  {
                     Byte  *dest_data=dest.data() + z*dest.pitch2();
                     Int        pitch=ImagePitch  (temp.w(), temp.h(), 0, dest.hwType()), // compressed pitch of 'data'
                             blocks_y=ImageBlocksY(dest.w(), dest.h(), 0, dest.hwType());
                     REPD(y, blocks_y)Copy(dest_data + y*dest.pitch(), (Byte*)data + y*pitch, dest.pitch(), pitch); // zero remaining Pow2Padded data to avoid garbage
                     Int written=blocks_y*dest.pitch(); Zero(dest_data+written, dest.pitch2()-written); // zero remaining Pow2Padded data to avoid garbage
                  }else ok=false;
                  free(data); // was allocated using 'malloc'
               }else ok=false;
               if(!ok)break;
            }
         }
         dest.unlock();
      }
      src.unlock();
   }
   return ok;
}
/******************************************************************************/
}
/******************************************************************************/
