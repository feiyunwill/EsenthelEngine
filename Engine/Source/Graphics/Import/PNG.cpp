/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_PNG
   #include "../../../../ThirdPartyLibs/begin.h"

   #define PNG_NO_STDIO
   #include "../../../../ThirdPartyLibs/Png/src/png.h"

   #include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE{
/******************************************************************************/
#if SUPPORT_PNG
   static void PngReadData (png_structp png_ptr, png_bytep data, png_size_t size) {if(!((File*)png_get_io_ptr(png_ptr))->get(data, (Int)size))png_error(png_ptr, "Read Error" );}
   static void PngWriteData(png_structp png_ptr, png_bytep data, png_size_t size) {if(!((File*)png_get_io_ptr(png_ptr))->put(data, (Int)size))png_error(png_ptr, "Write Error");}
#endif

Bool Image::ImportPNG(File &f)
{
#if SUPPORT_PNG
   Bool             ok=false, created=false;
   png_uint_32       width=0,
                    height=0,
                  channels=0;
   int          color_type=0,
                 bit_depth=0;
   png_structp     png_ptr=null;
   png_infop      info_ptr=null;
   Memt<png_byte*> row_ptrs;

   png_byte signature[8]; f>>signature; if(png_sig_cmp(signature, 0, 8))goto error;
    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, null, null, null); if(! png_ptr)goto error; if(setjmp(png_jmpbuf(png_ptr)))goto error;
   info_ptr=png_create_info_struct(png_ptr                                ); if(!info_ptr)goto error;

   {
      // initialize the png structure
      png_set_read_fn  (png_ptr, (png_voidp)&f, PngReadData);
      png_set_sig_bytes(png_ptr, 8);
      png_read_info    (png_ptr, info_ptr);
      png_get_IHDR     (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, null, null, null);

      if(color_type!=PNG_COLOR_TYPE_GRAY    && bit_depth==16)png_set_strip_16(png_ptr);
      if(color_type==PNG_COLOR_TYPE_GRAY    && bit_depth==16)png_set_swap    (png_ptr);
      if(color_type==PNG_COLOR_TYPE_PALETTE || bit_depth < 8)png_set_expand  (png_ptr);
      if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)    )png_set_expand  (png_ptr);

      Dbl gamma; if(png_get_gAMA        (png_ptr, info_ptr, &gamma))png_set_gamma(png_ptr, 2.2, gamma); // if required set gamma conversion
                    png_read_update_info(png_ptr, info_ptr); // after the transformations have been registered update info_ptr data
                    png_get_IHDR        (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, null, null, null); // get again width, height and the new bit-depth and color-type
           channels=png_get_channels    (png_ptr, info_ptr);

      switch(color_type)
      {
         default                       : goto error;
         case PNG_COLOR_TYPE_GRAY      : if(channels!=1 || (bit_depth!=8 && bit_depth!=16))goto error; createSoftTry(width, height, 1, (bit_depth==8) ? IMAGE_L8 : IMAGE_I16); break;
         case PNG_COLOR_TYPE_GRAY_ALPHA: if(channels!=2 ||  bit_depth!=8                  )goto error; createSoftTry(width, height, 1,                  IMAGE_L8A8          ); break;
         case PNG_COLOR_TYPE_RGB       : if(channels!=3 ||  bit_depth!=8                  )goto error; createSoftTry(width, height, 1,                  IMAGE_R8G8B8        ); break;
         case PNG_COLOR_TYPE_RGB_ALPHA : if(channels!=4 ||  bit_depth!=8                  )goto error; createSoftTry(width, height, 1,                  IMAGE_R8G8B8A8      ); break;
      }
      created=true;

      row_ptrs.setNum(height); REPAO(row_ptrs)=data()+pitch()*i; // set the individual row-pointers to point at the correct offsets
      png_read_image(png_ptr, row_ptrs.data()); // now we can go ahead and just read the whole image
      png_read_end  (png_ptr, null           ); // read the end marker
      ok=f.ok();
   }

error:;
   png_destroy_read_struct(&png_ptr, &info_ptr, null);
   if(!ok && !created)del(); return ok; // if failed but the image was created, then it's possible that some data was read, keep that in case user wants to preview what was read
#else
   del(); return false;
#endif
}
Bool Image::ExportPNG(File &f, Flt compression_level)C
{
#if SUPPORT_PNG
 C Image *src=this;
   Image  temp;

   if(!src->is  ())return false;
   if( src->cube()){if(!temp.fromCube(*src, ImageTI[src->type()].compressed ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT))return false; src=&temp;}

   Int bit_depth=0, color_type=0;
   switch(src->hwType())
   {
      case IMAGE_I16     : bit_depth=16; color_type=PNG_COLOR_TYPE_GRAY      ; break;
      case IMAGE_L8      : bit_depth= 8; color_type=PNG_COLOR_TYPE_GRAY      ; break;
      case IMAGE_A8      : bit_depth= 8; color_type=PNG_COLOR_TYPE_GRAY      ; break;
      case IMAGE_L8A8    : bit_depth= 8; color_type=PNG_COLOR_TYPE_GRAY_ALPHA; break;
      case IMAGE_R8G8B8  : bit_depth= 8; color_type=PNG_COLOR_TYPE_RGB       ; break;
      case IMAGE_R8G8B8A8: bit_depth= 8; color_type=PNG_COLOR_TYPE_RGB_ALPHA ; break;

      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32: if(!src->copyTry(temp, -1, -1, 1, IMAGE_I16, IMAGE_SOFT, 1))return false; src=&temp; bit_depth=16; color_type=PNG_COLOR_TYPE_GRAY; break;

      default: if(!src->copyTry(temp, -1, -1, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))return false; src=&temp; bit_depth=8; color_type=PNG_COLOR_TYPE_RGB_ALPHA; break;
   }

   if(src->lockRead())
   {
      Bool                ok=false;
      png_structp     png_ptr=null;
      png_infop      info_ptr=null;
      Memt<png_byte*> row_ptrs;
      
       png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, null, null, null); if(! png_ptr)goto error; if(setjmp(png_jmpbuf(png_ptr)))goto error;
      info_ptr=png_create_info_struct (png_ptr                                ); if(!info_ptr)goto error;

      {
         if(compression_level>=0)png_set_compression_level(png_ptr, Mid(RoundPos(compression_level*9), 0, 9));
         png_set_write_fn(png_ptr, (png_voidp)&f, PngWriteData, null); // initialize the png structure
         png_set_IHDR    (png_ptr, info_ptr, src->w(), src->h(), bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
         png_write_info  (png_ptr, info_ptr);
         if(color_type==PNG_COLOR_TYPE_GRAY && bit_depth==16)png_set_swap(png_ptr);

         row_ptrs.setNum(src->h()); REPAO(row_ptrs)=ConstCast(src->data()+i*src->pitch()); // set the individual row-pointers to point at the correct offsets
         png_write_image(png_ptr, row_ptrs.data()); // write out the entire image data in one call
         png_write_end  (png_ptr, info_ptr       ); // write the end marker
         ok=f.ok();
      }

   error:;
      png_destroy_write_struct(&png_ptr, &info_ptr);
      src->unlock();
      return ok;
   }
#endif
   return false;
}
/******************************************************************************/
Bool Image::ExportPNG(C Str &name, Flt compression_level)C
{
#if SUPPORT_PNG
   File f; if(f.writeTry(name)){if(ExportPNG(f, compression_level) && f.flush())return true; f.del(); FDelFile(name);}
#endif
   return false;
}
Bool Image::ImportPNG(C Str &name)
{
#if SUPPORT_PNG
   File f; if(f.readTry(name))return ImportPNG(f);
#endif
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
