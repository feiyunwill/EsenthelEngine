/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_JPG
   #include "../../../../ThirdPartyLibs/begin.h"

   #include <setjmp.h>
   #if 1 // JpegTurbo
      #include "../../../../ThirdPartyLibs/JpegTurbo/lib/jpeglib.h"
   #else // Jpeg
      #include "../../../../ThirdPartyLibs/Jpeg/jpeglib.h"
   #endif

   #include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE{
/******************************************************************************/
#if SUPPORT_JPG
struct jpeg_error_mgr_ex : jpeg_error_mgr
{
   jmp_buf jump_buffer;
};
static void my_error_exit(j_common_ptr cinfo)
{
   jpeg_error_mgr_ex &err=*(jpeg_error_mgr_ex*)cinfo->err;
 //(*cinfo->err->output_message)(cinfo);
   longjmp(err.jump_buffer, 1);
}
#endif
/******************************************************************************/
Bool Image::ImportJPG(File &f)
{
#if SUPPORT_JPG
   struct JPGReader : jpeg_source_mgr
   {
      File &f;
      Byte  data[4096];
      Bool  start_of_file;
      Long  start_pos;

      static void InitSource(j_decompress_ptr cinfo)
      {
         JPGReader &src=*(JPGReader*)cinfo->src;
         src.start_of_file=true;
      }
      static boolean FillInputBuffer(j_decompress_ptr cinfo)
      {
         JPGReader &src   =*(JPGReader*)cinfo->src;
         Int        nbytes=src.f.getReturnSize(src.data, SIZE(src.data));
         if(nbytes<=0) // if no more data available
         {
            if(src.start_of_file)return false; // if just started reading then fail
            // otherwise insert a fake EOI marker, and allowing reading incomplete JPG files
            src.data[0]=(JOCTET)0xFF;
            src.data[1]=(JOCTET)JPEG_EOI;
            nbytes=2;
         }
         src.next_input_byte=src.data;
         src.bytes_in_buffer=nbytes;
         src.start_of_file  =false;
         return true;
      }
      static void SkipInputData(j_decompress_ptr cinfo, long num_bytes)
      {
         JPGReader &src=*(JPGReader*)cinfo->src;
         if(num_bytes>0)
         {
            while(num_bytes>src.bytes_in_buffer)
            {
               num_bytes-=(long)src.bytes_in_buffer;
               FillInputBuffer(cinfo);
            }
            src.next_input_byte+=num_bytes;
            src.bytes_in_buffer-=num_bytes;
         }
      }
      static void TermSource(j_decompress_ptr cinfo) {}

      JPGReader(File &f) : f(f)
      {
         init_source      =InitSource;
         fill_input_buffer=FillInputBuffer;
         skip_input_data  =SkipInputData;
         resync_to_restart=jpeg_resync_to_restart; // use libJpeg method
         term_source      =TermSource;
         bytes_in_buffer  =0;
         next_input_byte  =null;
         start_pos        =f.pos();
      }
     ~JPGReader()
      {
         f.pos(Max(start_pos, f.pos()-(Int)bytes_in_buffer)); // go back with any unprocessed data left, to be able to read them after reading the JPEG (this is needed because 'FillInputBuffer' reads ahead of what's needed)
      }
   };

   	Bool  ok=false, created=false;
      JPGReader              jpg(f);
      jpeg_error_mgr_ex       jerr ;
      jpeg_decompress_struct  cinfo; cinfo.err=jpeg_std_error(&jerr); jerr.error_exit=my_error_exit; if(setjmp(jerr.jump_buffer))goto error; // setup jump position that will be reached upon jpeg error
      jpeg_create_decompress(&cinfo); cinfo.src=&jpg;
   if(jpeg_read_header      (&cinfo, true))
   {
   	jpeg_start_decompress(&cinfo);

      if(cinfo.output_components==1 && cinfo.out_color_space==JCS_GRAYSCALE
      || cinfo.output_components==3 && cinfo.out_color_space==JCS_RGB)
         if(createSoftTry(cinfo.output_width, cinfo.output_height, 1, (cinfo.output_components==1) ? IMAGE_L8 : IMAGE_R8G8B8))
      {
         created=true;
         for(; cinfo.output_scanline<cinfo.output_height; ){JSAMPROW row=T.data()+cinfo.output_scanline*pitch(); jpeg_read_scanlines(&cinfo, &row, 1);}
         ok=true;
      }
   	jpeg_finish_decompress(&cinfo);
   }
error:
   jpeg_destroy_decompress(&cinfo);
   if(!ok && !created)del(); return ok; // if failed but the image was created, then it's possible that some data was read, keep that in case user wants to preview what was read
#else
   del(); return false;
#endif
}
/******************************************************************************/
Bool Image::ExportJPG(File &f, Flt quality, Int sub_sample)C
{
#if SUPPORT_JPG
   struct JPGWriter : jpeg_destination_mgr
   {
      File &f;
      Byte  data[4096];

      static void InitDestination(j_compress_ptr cinfo)
      {
         JPGWriter &dest=*(JPGWriter*)cinfo->dest;
         dest.next_output_byte=dest.data;
         dest.free_in_buffer  =SIZE(dest.data);
      }
      static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
      {
         JPGWriter &dest=*(JPGWriter*)cinfo->dest;
         dest.f<<dest.data;
         dest.next_output_byte=dest.data;
         dest.free_in_buffer  =SIZE(dest.data);
         return true;
      }
      static void TermDestination(j_compress_ptr cinfo)
      {
         JPGWriter &dest=*(JPGWriter*)cinfo->dest;
         dest.f.put(dest.data, SIZE(dest.data)-(Int)dest.free_in_buffer);
      }

      JPGWriter(File &f) : f(f)
      {
         next_output_byte   =null;
         free_in_buffer     =0;
         init_destination   =InitDestination;
         empty_output_buffer=EmptyOutputBuffer;
         term_destination   =TermDestination;
      }
   };

   Int q=RoundPos(quality*100);
   if( q<   0)q= 80;else // default to 80
   if( q==  0)q=  1;else // min quality for jpeg is actually 1
   if( q> 100)q=100;     // max quality is 100

 C Image *src=this;
   Image temp;

   if(!src->is  ())return false;
   if( src->cube())if(temp.fromCube(*src, ImageTI[src->type()].compressed ? IMAGE_R8G8B8 : -1, IMAGE_SOFT))src=&temp;else return false;

   if(src->hwType()!=IMAGE_L8 && src->hwType()!=IMAGE_A8 && src->hwType()!=IMAGE_I8
   && src->hwType()!=IMAGE_R8G8B8
#if JCS_EXTENSIONS
   && src->hwType()!=IMAGE_R8G8B8A8
   && src->hwType()!=IMAGE_R8G8B8X8
   && src->hwType()!=IMAGE_B8G8R8
   && src->hwType()!=IMAGE_B8G8R8A8
   && src->hwType()!=IMAGE_B8G8R8X8
#endif
   )
      if(src->copyTry(temp, -1, -1, -1, (src->type()==IMAGE_I16) ? IMAGE_L8 : IMAGE_R8G8B8, IMAGE_SOFT, 1))src=&temp;else return false;

   if(src->lockRead())
   {
      JPGWriter             jpg(f);
      jpeg_error_mgr_ex     jerr ;
      jpeg_compress_struct  cinfo; cinfo.err=jpeg_std_error(&jerr); jerr.error_exit=my_error_exit; if(setjmp(jerr.jump_buffer)){jpeg_destroy_compress(&cinfo); return false;} // setup jump position that will be reached upon jpeg error
      jpeg_create_compress(&cinfo);

      switch(src->hwType())
      {
         case IMAGE_L8    :
         case IMAGE_A8    :
         case IMAGE_I8    : cinfo.in_color_space=JCS_GRAYSCALE; cinfo.input_components=1; break;
         case IMAGE_R8G8B8: cinfo.in_color_space=JCS_RGB      ; cinfo.input_components=3; break;
      #if JCS_EXTENSIONS
         case IMAGE_R8G8B8A8:
         case IMAGE_R8G8B8X8: cinfo.in_color_space=JCS_EXT_RGBX; cinfo.input_components=4; break;
         case IMAGE_B8G8R8  : cinfo.in_color_space=JCS_EXT_BGR ; cinfo.input_components=3; break;
         case IMAGE_B8G8R8A8:
         case IMAGE_B8G8R8X8: cinfo.in_color_space=JCS_EXT_BGRX; cinfo.input_components=4; break;
      #endif
      }
      cinfo.image_width =src->w();
      cinfo.image_height=src->h();
      cinfo.dest        =&jpg;

      jpeg_set_defaults(&cinfo);

      if(cinfo.comp_info && cinfo.in_color_space!=JCS_GRAYSCALE)switch(sub_sample)
      {
                  case 0: cinfo.comp_info[0].h_samp_factor=1; cinfo.comp_info[0].v_samp_factor=1; break;
                  case 1: cinfo.comp_info[0].h_samp_factor=2; cinfo.comp_info[0].v_samp_factor=1; break;
         default: case 2: cinfo.comp_info[0].h_samp_factor=2; cinfo.comp_info[0].v_samp_factor=2; break;
      }

      jpeg_set_quality     (&cinfo, q, true);
      jpeg_start_compress  (&cinfo, true); for(; cinfo.next_scanline<cinfo.image_height; ){JSAMPROW data=ConstCast(src->data()+cinfo.next_scanline*src->pitch()); jpeg_write_scanlines(&cinfo, &data, 1);}
      jpeg_finish_compress (&cinfo);
      jpeg_destroy_compress(&cinfo);

      src->unlock();
      return f.ok();
   }
#endif
   return false;
}
/******************************************************************************/
Bool Image::ExportJPG(C Str &name, Flt quality, Int sub_sample)C
{
#if SUPPORT_JPG
   File f; if(f.writeTry(name)){if(ExportJPG(f, quality, sub_sample) && f.flush())return true; f.del(); FDelFile(name);}
#endif
   return false;
}
Bool Image::ImportJPG(C Str &name)
{
#if SUPPORT_JPG
   File f; if(f.readTry(name))return ImportJPG(f);
#endif
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
