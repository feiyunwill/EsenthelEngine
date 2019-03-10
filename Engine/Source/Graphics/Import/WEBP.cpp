/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_WEBP
   #include "../../../../ThirdPartyLibs/begin.h"

   #include "../../../../ThirdPartyLibs/Webp/src/webp/decode.h"
   #include "../../../../ThirdPartyLibs/Webp/src/webp/encode.h"

   #include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE{
/******************************************************************************/
Bool Image::ImportWEBP(File &f)
{
   Bool ok=false;
#if SUPPORT_WEBP
   if(f.getUInt()==CC4('R','I','F','F'))
   {
      UInt size; f>>size;
      if(f.left()>=size
      && f.getUInt()==CC4('W','E','B','P'))
      {
         f.skip(-12);
         WebPDecoderConfig            config;
         WebPDecBuffer         *const output_buffer=&config.output;
         WebPBitstreamFeatures *const bitstream    =&config.input;
         if(WebPInitDecoderConfig(&config))
         {
            //config.options.use_threads=1; made no difference in performance
            Memt<Byte> temp; temp.setNum(size+8); // we have to add 8 for the RIFF+size data
            if(f.getFast(temp.data(), temp.elms()))
               if(WebPGetFeatures(temp.data(), temp.elms(), bitstream)==VP8_STATUS_OK)
            {
               IMAGE_TYPE type;
               if(bitstream->has_alpha)
               {
                  type                     =IMAGE_R8G8B8A8;
                  output_buffer->colorspace=MODE_RGBA;
               }else
               {
                  type                     =IMAGE_R8G8B8;
                  output_buffer->colorspace=MODE_RGB;
               }
               if(createSoftTry(bitstream->width, bitstream->height, 1, type))
               {
                  output_buffer->is_external_memory=true;
                  output_buffer->u.RGBA.stride=pitch ();
                  output_buffer->u.RGBA.size  =pitch2();
                  output_buffer->u.RGBA.rgba  =data  ();
                  if(WebPDecode(temp.data(), temp.elms(), &config)==VP8_STATUS_OK)ok=true;
               }
            }
            WebPFreeDecBuffer(output_buffer);
         }
      }
   }
#endif
   if(!ok)del(); return ok;
}
Bool Image::ImportWEBP(C Str &name)
{
#if SUPPORT_WEBP
   File f; if(f.readTry(name))return ImportWEBP(f);
#endif
   del(); return false;
}
/******************************************************************************/
#if SUPPORT_WEBP
static int WEBPWriter(const uint8_t* data, size_t data_size, const WebPPicture* const pic)
{
   File  &f=*(File*)pic->custom_ptr;
   return f.put(data, (Int)data_size);
}
#endif
Bool Image::ExportWEBP(File &f, Flt rgb_quality, Flt alpha_quality)C
{
   Bool ok=false;
#if SUPPORT_WEBP
 C Image *src=this;
   Image  temp;
   if(src->cube  ()                ){    temp.fromCube(*src ,             IMAGE_B8G8R8A8, IMAGE_SOFT   );              src=&temp;}
   if(src->hwType()!=IMAGE_B8G8R8A8){if(!src->copyTry ( temp, -1, -1, -1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1))return false; src=&temp;}

   if(src->w()<=WEBP_MAX_DIMENSION
   && src->h()<=WEBP_MAX_DIMENSION)
      if(src->lockRead())
   {
      WebPPicture picture;
      WebPConfig  config;
      if(WebPPictureInit(&picture)
      && WebPConfigInit (&config))
      {
         Int q=RoundPos(rgb_quality*100);
         if( q<  0)q=100;else // default to 100=lossless
         if( q>100)q=100;

         Int aq=RoundPos(alpha_quality*100);
         if( aq<  0)aq=  q;else // default to 'q'=rgb_quality
         if( aq>100)aq=100;

       //config .pass         =10; // very little difference, not sure if better or worse
       //config .method       =6; // this affects only lossy, 5 and 6 are supposed to be better quality, however they actually made image lose detail
       //config .thread_level =2; // 1=no performance difference, >=1 actually makes the compression to fail
       //config .near_lossless=; // made things much worse
         config .lossless     =(q>=100);
         config .      quality= q;
         config .alpha_quality=aq;
         config .exact        =true;
         picture.width        =src->w();
         picture.height       =src->h();
      #if 1 // RGBA
         picture.use_argb     =true;
         picture.argb         =(uint32_t*)src->data();
         picture.argb_stride  =src->pitch()/src->bytePP(); // in pixel units
      #else // YUVA, this didn't improve the quality, we could use it mainly if we had the source already in YUV format
         Image Y, U, V, A;
         Y.createSoftTry(src->w(), src->h(), 1, IMAGE_L8);
         U.createSoftTry(src->w(), src->h(), 1, IMAGE_L8);
         V.createSoftTry(src->w(), src->h(), 1, IMAGE_L8);
         if(ImageTI[src->type()].a)
            if(src->copyTry(A, -1, -1, -1, IMAGE_A8, IMAGE_SOFT, 1))
         {
            picture.a       =(uint8_t*)A.data();
            picture.a_stride=A.pitch()/A.bytePP(); // in pixel units
         }
         REPD(y, src->h())
         REPD(x, src->w())
         {
            Vec yuv=RgbToYuv(src->colorF(x, y).xyz);
            Y.pixelF(x, y, yuv.x);
            U.pixelF(x, y, yuv.y);
            V.pixelF(x, y, yuv.z);
         }
         U.downSample();
         V.downSample();
         picture.use_argb =false;
         picture.y        =(uint8_t*)Y.data();
         picture.u        =(uint8_t*)U.data();
         picture.v        =(uint8_t*)V.data();
         picture.y_stride =Y.pitch()/Y.bytePP(); // in pixel units
         picture.uv_stride=U.pitch()/U.bytePP(); // in pixel units
      #endif
         picture.custom_ptr   =&f;
         picture.writer       =WEBPWriter;
         if(WebPEncode(&config, &picture))ok=f.ok();
         WebPPictureFree(&picture);
      }
      src->unlock();
   }
#endif
   return ok;
}
Bool Image::ExportWEBP(C Str &name, Flt rgb_quality, Flt alpha_quality)C
{
#if SUPPORT_WEBP
   File f; if(f.writeTry(name)){if(ExportWEBP(f, rgb_quality, alpha_quality) && f.flush())return true; f.del(); FDelFile(name);}
#endif
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
