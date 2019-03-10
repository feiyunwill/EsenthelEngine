/******************************************************************************

   DX9 requires deletion of IMAGE_RT on Display.lost

/******************************************************************************/
#include "stdafx.h"

#include "../../../ThirdPartyLibs/begin.h"

#if SUPPORT_THEORA
   #include "../../../ThirdPartyLibs/Theora/include/theora/theora.h"
#endif

#if SUPPORT_VORBIS || SUPPORT_OPUS || SUPPORT_VP // may be used by Vorbis/Opus/VP inside WEBM
   #pragma warning(push)
   #pragma warning(disable:4996) // strcpy
   #include "../../../ThirdPartyLibs/VP/libvpx/third_party/libwebm/mkvparser/mkvparser.h"
   #include "../../../ThirdPartyLibs/VP/libvpx/third_party/libwebm/mkvparser/mkvparser.cc"
   #pragma warning(pop)
#endif

#if SUPPORT_VP
   #include "../../../ThirdPartyLibs/VP/libvpx/vpx/vpx_decoder.h"
   #include "../../../ThirdPartyLibs/VP/libvpx/vpx/vp8dx.h"
   #include "../../../ThirdPartyLibs/VP/libvpx/webmdec.h"
   #include "../../../ThirdPartyLibs/VP/libvpx/webmdec.cc" // always include to avoid having to do separate WindowsUniversal libs for 32/64
   #undef UNUSED
#endif

#include "../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
#if DX9
Memx<Image> VideoTextures;
#endif
/******************************************************************************/
#if SUPPORT_THEORA
struct Theora : VideoCodec
{
   Int              p;
   ogg_sync_state   oy;
   ogg_page         og;
   ogg_stream_state vo;
   ogg_stream_state to;
   ogg_packet       op;
   theora_info      ti;
   theora_comment   tc;
   theora_state     td;

   void zero()
   {
      p=0;
      Zero(oy);
      Zero(og);
      Zero(vo);
      Zero(to);
      Zero(op);
      Zero(ti);
      Zero(tc);
      Zero(td);
   }
   void del()
   {
      if(p)
      {
         ogg_stream_clear    (&to);
         theora_clear        (&td);
         theora_comment_clear(&tc);
         theora_info_clear   (&ti);
      }
      ogg_sync_clear(&oy);
      zero();
   }
   virtual Bool create(Video &video)
   {
      del();

      ogg_sync_init(&oy);

      theora_comment_init(&tc);
      theora_info_init   (&ti);

      for(;;)
      {
         if(!buffer_data(video))break;
         Bool read=false;
         while(ogg_sync_pageout(&oy, &og)>0)
         {
            ogg_stream_state test;

            if(!ogg_page_bos(&og))
            {
               queue_page();
               goto done;
            }

            ogg_stream_init     (&test, ogg_page_serialno(&og));
            ogg_stream_pagein   (&test, &og);
            ogg_stream_packetout(&test, &op);

            if(!p && theora_decode_header(&ti, &tc, &op)>=0)
            {
               CopyFast(&to, &test, SIZE(test));
               p=1;
            }else
            {
               ogg_stream_clear(&test);
            }
            read=true;
         }
         if(!read)break;
      }
   done:;

      while(p && p<3)
      {
         int ret;
         while(p && p<3 && (ret=ogg_stream_packetout(&to, &op)))
         {
            if(ret<0 || theora_decode_header(&ti, &tc, &op))return false; // error parsing Theora stream headers; corrupt stream?
            if(++p==3)break;
         }

         if(ogg_sync_pageout(&oy, &og)>0)queue_page();else
         if(!buffer_data(video))return false; // end of file while searching for codec headers
      }

      if(p)
      {
         theora_decode_init(&td, &ti);
      }else
      {
         theora_info_clear   (&ti);
         theora_comment_clear(&tc);
      }

      while(ogg_sync_pageout(&oy, &og)>0)queue_page();
      return p!=0;
   }

   void queue_page() {if(p)ogg_stream_pagein(&to, &og);}

   Int buffer_data(Video &video)
   {
      int bytes=video._file.getReturnSize(ogg_sync_buffer(&oy, 4096), 4096);
      ogg_sync_wrote(&oy, bytes);
      return bytes;
   }
   virtual RESULT nextFrame(Video &video, Flt &time)
   {
      for(;;)
      {
         for(; p; )
         {
            if(ogg_stream_packetout(&to, &op)<=0)break;
            theora_decode_packetin (&td, &op);
            time=theora_granule_time(&td, td.granulepos);
            return OK;
         }
         if(video._file.end())return END;
         for(buffer_data(video); ogg_sync_pageout(&oy, &og)>0; )queue_page();
      }
   }
   virtual void frameToImage(Video &video)
   {
      yuv_buffer yuv; theora_decode_YUVout(&td, &yuv);
      if(yuv.y_width>0 && yuv.y_height>0)
      {
         Int offset_x=ti.offset_x    , offset_x_uv=(offset_x*yuv.uv_width )/yuv.y_width ,
             offset_y=ti.offset_y    , offset_y_uv=(offset_y*yuv.uv_height)/yuv.y_height,
             width   =ti.frame_width ,    width_uv=( width  *yuv.uv_width )/yuv.y_width ,
             height  =ti.frame_height,   height_uv=(height  *yuv.uv_height)/yuv.y_height;

         yuv.y+=offset_x    + offset_y   *yuv. y_stride;
         yuv.u+=offset_x_uv + offset_y_uv*yuv.uv_stride;
         yuv.v+=offset_x_uv + offset_y_uv*yuv.uv_stride;

         video.frameToImage(width, height, width_uv, height_uv, yuv.y, yuv.u, yuv.v, yuv.y_stride, yuv.uv_stride, yuv.uv_stride);
      }
   }

  ~Theora() {del ();}
   Theora() {zero();}
};
#endif
/******************************************************************************/
#if SUPPORT_VP
struct MkvReader : mkvparser::IMkvReader
{
   File &f;

   MkvReader(File &f) : f(f) {}

   virtual int Read(long long pos, long len, unsigned char* buf) // 0=OK, -1=error
   {
      if(len==0)return 0;
      if(!f.pos(pos))return -1;
      return f.getFast(buf, len) ? 0 : -1;
   }
   virtual int Length(long long* total, long long* available) // 0=OK, -1=error
   {
      if(total    )*total    =f.size();
      if(available)*available=f.size();
      return 0;
   }
};
struct VP : VideoCodec
{
   Bool             valid;
   uint8_t         *buf;
   size_t           buffer_size;
   VpxInputContext  input;
   WebmInputContext webm;
   vpx_codec_ctx_t  decoder;

   void zero()
   {
      valid=false; buf=null; buffer_size=0;
      Zero(input); Zero(webm); Zero(decoder);
   }
   void del()
   {
      vpx_codec_destroy(&decoder);
      webm_free(&webm);
      DeleteN(buf);
      zero();
   }
   virtual Bool create(Video &video)
   {
      del();
      webm.reader=new MkvReader(video._file);
      if(file_is_webm(&webm, &input))
      {
         input.file_type=FILE_TYPE_WEBM;
         if(input.fourcc==CC4('V', 'P', '9', '0'))
         if(!webm_guess_framerate(&webm, &input))
         {
            Flt fps=Flt(input.framerate.numerator)/input.framerate.denominator;
            vpx_codec_dec_cfg_t cfg; Zero(cfg);
            cfg.threads=Min(2, Cpu.threads());
            if(!vpx_codec_dec_init(&decoder, vpx_codec_vp9_dx(), &cfg, 0))
            {
               valid=true;
               return true;
            }
         }
      }
      del(); return false;
   }
   virtual RESULT nextFrame(Video &video, Flt &time)
   {
      if(valid)
      {
         Int result=webm_read_frame(&webm, &buf, &buffer_size); // 0=ok, 1=end, -1=error
         if( result==1)return END;
         if(!result && !vpx_codec_decode(&decoder, buf, (unsigned int)buffer_size, null, 0))
         {
            time=webm.timestamp_ns/1000000000.0;
            return OK;
         }
         del();
      }
      return ERROR;
   }
   virtual void frameToImage(Video &video)
   {
      if(valid)
      {
         vpx_codec_iter_t iter=null;
         if(vpx_image_t *image=vpx_codec_get_frame(&decoder, &iter))
         {
            Int w=image->d_w, h=image->d_h;
            video.frameToImage(w, h, (w+1)/2, (h+1)/2, image->planes[0], image->planes[1], image->planes[2], image->stride[0], image->stride[1], image->stride[2]);
         }
      }
   }

   VP() {zero();}
  ~VP() {del ();}
};
#endif
/******************************************************************************/
void Video::zero()
{
  _codec    =VIDEO_NONE;
  _loop     =false;
  _mode     =DEFAULT;
  _w=_h=_br=0;
  _time     =0;
  _time_past=0;
  _fps      =0;
  _d        =null;
  _tex_ptr  =null;
}
Video::Video() {zero();}
void Video::release()
{
   Delete(_d);
  _file.del();
}
void Video::del()
{
   release();
  _lum .del();
  _u   .del();
  _v   .del();
  _tex .del();
#if DX9
   if(_tex_ptr){VideoTextures.removeData(_tex_ptr); _tex_ptr=null;}
#endif
   zero();
}
Bool Video::create(C Str &name, Bool loop, MODE mode)
{
   del();
   if(!name.is())return true;
   if(_file.readTry(name))
   {
   #if SUPPORT_VP
      VP vp;
      if(vp.create(T))
      {
        _codec=VIDEO_VP9;
        _w    =vp.input.width;
        _h    =vp.input.height;
        _br   =0;
        _fps  =Flt(vp.input.framerate.numerator)/vp.input.framerate.denominator;
        _d    =new VP; Swap(*(VP*)_d, vp);
      }else
   #endif
      {
      #if SUPPORT_THEORA
        _file.pos(0); // reset file position after VP attempt
         Theora theora;
         if(theora.create(T))
         {
           _codec=VIDEO_THEORA;
           _w    =theora.ti.frame_width;
           _h    =theora.ti.frame_height;
           _br   =((theora.ti.target_bitrate>0) ? theora.ti.target_bitrate : (theora.ti.keyframe_data_target_bitrate>0) ? theora.ti.keyframe_data_target_bitrate : 0);
           _fps  =Flt(theora.ti.fps_numerator)/theora.ti.fps_denominator;
           _d    =new Theora; Swap(*(Theora*)_d, theora);
         }
      #endif
      }
      if(_codec)
      {
        _loop=loop;
        _mode=mode;
      #if DX9
         if(_mode==IMAGE)_tex_ptr=&VideoTextures.New(); // needed only for IMAGE because only this mode uses IMAGE_RT
      #endif
         return true;
      }
   }
   del(); return false;
}
Bool Video::create(C UID &id, Bool loop, MODE mode) {return create(id.valid() ? _EncodeFileName(id) : null, loop, mode);}
/******************************************************************************/
CChar8* Video::codecName()C {return CodecName(codec());}
/******************************************************************************/
Bool Video::frameToImage(Int w, Int h, Int w2, Int h2, CPtr lum_data, CPtr u_data, CPtr v_data, Int lum_pitch, Int u_pitch, Int v_pitch)
{
#if DX9
   #define VIDEO_IMAGE_TYPE IMAGE_L8
#else
   #define VIDEO_IMAGE_TYPE IMAGE_R8
#endif
   if(_mode==ALPHA)
   {
      if(_lum.w()!=w || _lum.h()!=h)if(!_lum.create2DTry(w, h, VIDEO_IMAGE_TYPE, 1, false))return false;
     _lum.setFrom(lum_data, lum_pitch);
   }else
   {
      if(_lum.w()!=w  || _lum.h()!=h ){if(!_lum.create2DTry(w , h , VIDEO_IMAGE_TYPE, 1, false))return false; if(!Sh.h_YUV)AtomicSet(Sh.h_YUV, Sh.get("YUV"));}
      if(_u  .w()!=w2 || _u  .h()!=h2) if(!_u  .create2DTry(w2, h2, VIDEO_IMAGE_TYPE, 1, false))return false;
      if(_v  .w()!=w2 || _v  .h()!=h2) if(!_v  .create2DTry(w2, h2, VIDEO_IMAGE_TYPE, 1, false))return false;

     _lum.setFrom(lum_data, lum_pitch);
     _u  .setFrom(  u_data,   u_pitch);
     _v  .setFrom(  v_data,   v_pitch);

      if(_mode==IMAGE) // if want to create a texture
      {
      #if DX9
         Image *tex= _tex_ptr;
      #else
         Image *tex=&_tex;
      #endif
         if(tex)
         {
            if(tex->w()!=w || tex->h()!=h)tex->create(w, h, 1, IMAGE_DEFAULT, IMAGE_RT, 1);

            SyncLocker locker(D._lock); // needed for drawing in case this is called outside of Draw
            Image *rt[Elms(Renderer._cur)], *rtz=Renderer._cur_ds; REPAO(rt)=Renderer._cur[i];
            Renderer.set(tex, null, false);
            ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
            draw(D.rect()); // use specified rectangle without fitting via 'drawFs' or 'drawFit'
            D.alpha(alpha);
            Renderer.set(rt[0], rt[1], rt[2], rt[3], rtz, true);
         }
      }
   }
   return true;
}
void Video::frameToImage()
{
   if(_d)_d->frameToImage(T);
}
Bool Video::nextFrame()
{
   if(_d)
   {
      Flt time; switch(_d->nextFrame(T, time))
      {
         default             : error:  release(); break;
         case VideoCodec::OK : ok   : _time=time+_time_past; return true;
         case VideoCodec::END: // finished playing
         {
            if(_loop)
            {
               // restart
              _file.pos(0);
              _time_past=_time;
               if(_d->create(T))if(_d->nextFrame(T, time)==VideoCodec::OK)goto ok; // reset and set the first frame
            }
            goto error;
         }break;
      }
   }
   return false;
}
/******************************************************************************/
C Image& Video::image()C
{
#if DX9
   if(_tex_ptr)return *_tex_ptr;
#endif
   return _tex;
}
/******************************************************************************/
Bool Video::update(Flt time)
{
   if(_d)
   {
      Bool added=false;
      for(; T.time()<=time; )if(nextFrame())added=true;else break; // use <= so update(0) will set first frame
      if(added)frameToImage();
      return added || _d; // if there was a frame added or if the video still exists
   }
   return false;
}
/******************************************************************************/
void Video::drawAlphaFs (C Video &alpha, FIT_MODE fit)C {return drawAlpha(alpha, T.fit(D.rect(), fit));}
void Video::drawAlphaFit(C Video &alpha, C Rect &rect)C {return drawAlpha(alpha, T.fit(  rect       ));}
void Video::drawAlpha   (C Video &alpha, C Rect &rect)C
{
   if(_lum.is())
   {
      if(!Sh.h_YUVA)AtomicSet(Sh.h_YUVA, Sh.get("YUVA"));
      Sh .h_ImageCol[1]->set(_u); MaterialClear();
      Sh .h_ImageCol[2]->set(_v);
      Sh .h_ImageCol[3]->set(alpha._lum);
      VI .shader(Sh.h_YUVA);
     _lum.draw (rect);
      VI .clear(); // force clear to reset custom shader, in case 'draw' doesn't process drawing
   }
}

void Video::drawFs (FIT_MODE fit)C {return draw(T.fit(D.rect(), fit));}
void Video::drawFit(C Rect &rect)C {return draw(T.fit(  rect       ));}
void Video::draw   (C Rect &rect)C
{
   if(_lum.is())
   {
      Sh .h_ImageCol[1]->set(_u); MaterialClear();
      Sh .h_ImageCol[2]->set(_v);
      VI .shader(Sh.h_YUV);
     _lum.draw (rect);
      VI .clear(); // force clear to reset custom shader, in case 'draw' doesn't process drawing
   }
}
/******************************************************************************/
CChar8* CodecName(VIDEO_CODEC codec)
{
   switch(codec)
   {
      default           : return null;
      case VIDEO_THEORA : return "Theora";
      case VIDEO_VP9    : return "VP9";
   }
}
/******************************************************************************/
#if DX9
void VideoTexturesLost()
{
   REPA(VideoTextures)
   {
      Image &image=VideoTextures[i];
      image.copyTry(image, -1, -1, -1, -1, IMAGE_2D); // replace with 2D texture so it can be used to draw on the IMAGE_RT later
   }
}
void VideoTexturesReset()
{
   REPA(VideoTextures)
   {
      Image &image=VideoTextures[i];
      Image temp; Swap(temp, image);
      if(image.createTry(temp.w(), temp.h(), 1, temp.type(), IMAGE_RT, 1))temp.copyHw(image, true);
   }
}
#endif
/******************************************************************************/
}
/******************************************************************************/
