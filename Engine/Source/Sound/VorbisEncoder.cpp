/******************************************************************************

   Keep this as a separate file, because if the functions are not used,
      then they won't be linked, and app size won't be increased.

   Because it's linked separately, its name can't include spaces (due to Android building toolchain).

/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_VORBIS_ENC
   #include "../../../ThirdPartyLibs/begin.h"
   #include "../../../ThirdPartyLibs/Vorbis/include/vorbis/vorbisenc.h"
   #include "../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
// OGG VORBIS
/******************************************************************************/
struct _OggVorbisEncoder
{
   ogg_stream_state os;
   ogg_page         og;
   ogg_packet       op;
   vorbis_info      vi;
   vorbis_comment   vc;
   vorbis_dsp_state vd;
   vorbis_block     vb;
   File             *f;

   void        zero() {Zero(T);}
  _OggVorbisEncoder() {zero();}
 ~_OggVorbisEncoder() {del();}
   void                del()
   {
      ogg_stream_clear    (&os);
      vorbis_block_clear  (&vb);
      vorbis_dsp_clear    (&vd);
      vorbis_comment_clear(&vc);
      vorbis_info_clear   (&vi);
      zero();
   }
   Bool init(File &f, Int frequency, Int channels, Flt quality)
   {
      // quality = -0.1->45, 0.0->64, 0.1->80, 0.2->96, 0.3->112, 0.4->128, 0.5->160, 0.6->192, 0.7->224, 0.8->256, 0.9->320, 1.0->500
      del();
      if(frequency>0 && channels>=1 && channels<=2)
      {
         vorbis_info_init    (&vi);
         if(vorbis_encode_init_vbr(&vi, channels, frequency, Mid(quality, -0.1f, 1.0f)))goto error; // may fail if quality is out of range
         vorbis_comment_init (&vc);
         vorbis_analysis_init(&vd, &vi);
         vorbis_block_init   (&vd, &vb);
         ogg_stream_init     (&os, Random());

         ogg_packet header, header_comm, header_code;
         vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
         ogg_stream_packetin      (&os, &header);
         ogg_stream_packetin      (&os, &header_comm);
         ogg_stream_packetin      (&os, &header_code);

         for(;;)
         {
            if(!ogg_stream_flush(&os, &og))break;
            f.put(og.header, og.header_len);
            f.put(og.  body, og.  body_len);
         }
         if(f.ok()){T.f=&f; return true;}
      }
   error:
      del(); return false;
   }
   Bool encodeEx(CPtr data, Int size)
   {
      if(f)
      {
         Int samples=size/(2*vi.channels);
         if(Flt **buffer=vorbis_analysis_buffer(&vd, samples))
         {
            I16 *d=(I16*)data;
            if(vi.channels==2)REP(samples)
            {
               buffer[0][i]=d[i*2  ]/32768.0f;
               buffer[1][i]=d[i*2+1]/32768.0f;
            }else REP(samples)
            {
               buffer[0][i]=d[i]/32768.0f;
            }
            vorbis_analysis_wrote(&vd, samples);
            for(Int eos=0; vorbis_analysis_blockout(&vd, &vb)==1; )
            {
               vorbis_analysis(&vb, null);
               vorbis_bitrate_addblock(&vb);
               for(; vorbis_bitrate_flushpacket(&vd, &op); )
               {
                  ogg_stream_packetin(&os, &op);
                  for(; !eos; )
                  {
                     if(!ogg_stream_pageout(&os, &og))break;
                     f->put(og.header, og.header_len);
                     f->put(og.  body, og.  body_len);
                     if(ogg_page_eos(&og))eos=1;
                  }
               }
            }
            return true;
         }
      }
      return false;
   }
   Bool encode(CPtr data, Int size)
   {
      for(Byte *d=(Byte*)data; size>0; ) // encode in chunks, because: vorbis compressor will crash when passing a big size, vorbis compressor allocates a temporary buffer for every data passed
      {
         Int write=Min(size, 65536);
         if(!encodeEx(d, write))return false;
         d   +=write;
         size-=write;
      }
      return true;
   }
   Bool finish()
   {
      if(f)
      {
         Bool ok=encodeEx(null, 0); // encode end of stream, use 'encodeEx' because 'encode' will do nothing for 0 size
         f=null;
         return ok;
      }
      return true;
   }
};
/******************************************************************************/
OggVorbisEncoder& OggVorbisEncoder::del()
{
   if(_encoder)
   {
      ((_OggVorbisEncoder*)_encoder)->finish();
      Delete((_OggVorbisEncoder*&)_encoder);
   }
   return T;
}
Bool OggVorbisEncoder::create(File &f, Int frequency, Int channels, Flt quality)
{
   if(_encoder)
   {
      ((_OggVorbisEncoder*)_encoder)->finish();
   }else
   {
      New((_OggVorbisEncoder*&)_encoder);
   }
   if(((_OggVorbisEncoder*)_encoder)->init(f, frequency, channels, quality))return true;
   Delete((_OggVorbisEncoder*&)_encoder);
   return false;
}
Bool OggVorbisEncoder::encode(CPtr data, Int size)
{
   if(!size)return true;
   if(!data || size<=0 || !_encoder)return false;
   return ((_OggVorbisEncoder*)_encoder)->encode(data, size);
}
Bool OggVorbisEncoder::finish()
{
   return _encoder ? ((_OggVorbisEncoder*)_encoder)->finish() : true;
}
/******************************************************************************/
// ESENTHEL SND VORBIS
/******************************************************************************

   Vorbis generates packets with variable frame size (not constant like Opus)
      -therefore both indexes would need to be saved (packet source position and packet size)
      -when both as U16 are saved, then generated file is larger than OGG Vorbis
      -some other technique could be figured out, for example store cmpIntV as deltas, however that would introduce more complexity and provide results similar to OGG
      -because of that, SndVorbisEncoder is not finished, and OGG Vorbis is recommended instead

/******************************************************************************/
#if 0 // not finished yet
struct _SndVorbisEncoder
{
   ogg_packet       op;
   vorbis_info      vi;
   vorbis_dsp_state vd;
   vorbis_block     vb;
   File             *f;
   Long              pos;

   void        zero() {Zero(op); Zero(vi); Zero(vd); Zero(vb); f=null; pos=0;}
  _SndVorbisEncoder() {zero();}
 ~_SndVorbisEncoder() {del();}
   void                del()
   {
      vorbis_block_clear(&vb);
      vorbis_dsp_clear  (&vd);
      vorbis_info_clear (&vi);
      zero();
   }
   Bool init(File &f, Long samples, Int frequency, Int channels, Flt quality)
   {
      // quality = -0.1->45, 0.0->64, 0.1->80, 0.2->96, 0.3->112, 0.4->128, 0.5->160, 0.6->192, 0.7->224, 0.8->256, 0.9->320, 1.0->500
      del();
      if(frequency>0 && channels>=1 && channels<=2)
      {
         vorbis_info_init    (&vi);
         if(vorbis_encode_init_vbr(&vi, channels, frequency, quality))goto error;
         vorbis_analysis_init(&vd, &vi);
         vorbis_block_init   (&vd, &vb);

         if(SaveSndHeader(f, SND_VORBIS, channels, frequency, samples))
         {
            T.f=&f;
            return true;
         }
      }
   error:
      del(); return false;
   }
   Bool encodeEx(CPtr data, Int size)
   {
      if(f)
      {
         Int samples=size/(2*vi.channels);
         if(Flt **buffer=vorbis_analysis_buffer(&vd, samples))
         {
            I16 *d=(I16*)data;
            if(vi.channels==2)REP(samples)
            {
               buffer[0][i]=d[i*2  ]/32768.0f;
               buffer[1][i]=d[i*2+1]/32768.0f;
            }else REP(samples)
            {
               buffer[0][i]=d[i]/32768.0f;
            }
            vorbis_analysis_wrote(&vd, samples);
            for(; vorbis_analysis_blockout(&vd, &vb)==1; )
            {
               vorbis_analysis(&vb, null);
               vorbis_bitrate_addblock(&vb);
               for(; vorbis_bitrate_flushpacket(&vd, &op); )
               {
                  UInt samples=op.granulepos-pos; pos=op.granulepos;
                  f->putUShort(samples);
                  f->putUShort(op.bytes);
                  if(!f->put(op.packet, op.bytes))return false;
               }
            }
            return true;
         }
      }
      return false;
   }
   Bool encode(CPtr data, Int size)
   {
      for(Byte *d=(Byte*)data; size>0; ) // encode in chunks, because: vorbis compressor will crash when passing a big size, vorbis compressor allocates a temporary buffer for every data passed
      {
         Int write=Min(size, 65536);
         if(!encodeEx(d, write))return false;
         d   +=write;
         size-=write;
      }
      return true;
   }
   Bool finish()
   {
      if(f)
      {
         Bool ok=encodeEx(null, 0); // encode end of stream, use 'encodeEx' because 'encode' will do nothing for 0 size
         f=null;
         return ok;
      }
      return true;
   }
};
/******************************************************************************/
SndVorbisEncoder& SndVorbisEncoder::del()
{
   if(_encoder)
   {
      ((_SndVorbisEncoder*)_encoder)->finish();
      Delete((_SndVorbisEncoder*&)_encoder);
   }
   return T;
}
Bool SndVorbisEncoder::create(File &f, Long samples, Int frequency, Int channels, Flt quality)
{
   if(_encoder)
   {
      ((_SndVorbisEncoder*)_encoder)->finish();
   }else
   {
      New((_SndVorbisEncoder*&)_encoder);
   }
   if(((_SndVorbisEncoder*)_encoder)->init(f, samples, frequency, channels, quality))return true;
   Delete((_SndVorbisEncoder*&)_encoder);
   return false;
}
Bool SndVorbisEncoder::encode(CPtr data, Int size)
{
   if(!size)return true;
   if(!data || size<=0 || !_encoder)return false;
   return ((_SndVorbisEncoder*)_encoder)->encode(data, size);
}
Bool SndVorbisEncoder::finish()
{
   return _encoder ? ((_SndVorbisEncoder*)_encoder)->finish() : true;
}
#endif
/******************************************************************************/
}
#endif
/******************************************************************************/
