/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_TIF
   #include "../../../../ThirdPartyLibs/begin.h"
   #include "../../../../ThirdPartyLibs/Tiff/libtiff/tiffio.h"
   #include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE{
/******************************************************************************/
#if SUPPORT_TIF
struct TIFFile
{
   File &f;
   Long  begin, // allows reading   of TIF images from current file position and not from the file start
         end  ; // allows detection of actual TIF file size, useful if we have data before/after the TIF image in the file

   TIFFile(File &f) : f(f) {begin=f.pos(); end=0;}
};
static tmsize_t TIFRead(thandle_t file, void *data, tmsize_t size)
{
   if(TIFFile *f=(TIFFile*)file)
   {
      Int    read=f->f.getReturnSize(data, size); MAX(f->end, f->f.pos());
      return read;
   }
   return 0;
}
static tmsize_t TIFWrite(thandle_t file, void *data, tmsize_t size)
{
   if(TIFFile *f=(TIFFile*)file)
   {
      Int    written=f->f.putReturnSize(data, size); MAX(f->end, f->f.pos());
      return written;
   }
   return 0;
}
static toff_t TIFSeek(thandle_t file, toff_t offset, int mode)
{
   if(TIFFile *f=(TIFFile*)file)
   {
      switch(mode)
      {
         case SEEK_SET: offset+=f->begin   ; break;
         case SEEK_CUR: offset+=f->f.pos (); break;
         case SEEK_END: offset+=f->f.size(); break;
      }
      f->f.pos(offset);
      return f->f.pos()-f->begin;
   }
   return 0;
}
static int TIFClose(thandle_t file)
{
   return 0;
}
static toff_t TIFSize(thandle_t file)
{
   if(TIFFile *f=(TIFFile*)file)return f->f.size()-f->begin;
   return 0;
}
#endif
/******************************************************************************/
Bool Image::ImportTIF(File &f)
{
#if SUPPORT_TIF
   Bool ok=false;
   TIFFile tf(f);
   TIFFSetErrorHandler     (null); // disable error handlers, otherwise TIF library will create system message boxes
   TIFFSetErrorHandlerExt  (null);
   TIFFSetWarningHandler   (null);
   TIFFSetWarningHandlerExt(null);
   if(TIFF *tif=TIFFClientOpen("", "r", &tf, TIFRead, TIFWrite, TIFSeek, TIFClose, TIFSize, null, null)) // can't use null because crash will occur
   {
      uint32 w=0, h=0, samples=0, bits=0, photometric=0, compression=0;
      TIFFGetField(tif, TIFFTAG_IMAGEWIDTH     , &w          );
      TIFFGetField(tif, TIFFTAG_IMAGELENGTH    , &h          );
      TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples    );
      TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE  , &bits       );
      TIFFGetField(tif, TIFFTAG_PHOTOMETRIC    , &photometric);
      TIFFGetField(tif, TIFFTAG_COMPRESSION    , &compression);
      Int scanline_size=TIFFScanlineSize(tif);
      if( scanline_size*8==samples*w*bits)
      {
         IMAGE_TYPE type=IMAGE_NONE;
         switch(samples)
         {
            case 1: if(photometric==PHOTOMETRIC_MINISBLACK)if(bits==8)type=IMAGE_L8;else if(bits==16)type=IMAGE_I16;else if(bits==24)type=IMAGE_I24;else if(bits==32)type=IMAGE_I32; break;
            case 3: if(photometric==PHOTOMETRIC_RGB)if(bits==8)type=IMAGE_R8G8B8  ; break;
            case 4: if(photometric==PHOTOMETRIC_RGB)if(bits==8)type=IMAGE_R8G8B8A8; break;
         }
         if(type && createSoftTry(w, h, 1, type))
         {
            if(compression==COMPRESSION_NONE)
            {
               ok=true; FREPD(y, h)if(!TIFFReadScanline(tif, data()+y*pitch(), y)){ok=false; break;} // read in forward order to avoid file seeking going back
            }else
            if(!TIFFIsTiled(tif))
            {
               ok=true;
               uint32 rows_per_strip=0, subsampling_x=0, subsampling_y=0;
               TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP    , &rows_per_strip);
               TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRSUBSAMPLING, &subsampling_x, &subsampling_y);
               Memt<Byte> buf; buf.setNum(TIFFStripSize(tif));
               for(UInt row=0; row<h; )
               {
                  UInt rowstoread=rows_per_strip-row%rows_per_strip,
                       nrow=(row+rowstoread>h ? h-row : rowstoread),
                       nrowsub=nrow;
                  if((nrowsub%subsampling_y)!=0)nrowsub+=subsampling_y-nrowsub%subsampling_y;
                  tmsize_t read=TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0), buf.data(), (row%rows_per_strip+nrowsub)*scanline_size);
                  if(read!=nrow*scanline_size){ok=false; break;}
                  Byte *dest=data()+row*pitch(), *src=buf.data()+(row%rows_per_strip)*scanline_size;
                  REP(nrow)CopyFast(dest+i*pitch(), src+i*scanline_size, scanline_size);
                  row+=nrow;
               }
            }
         }
      }
      if(!ok)
         if(createSoftTry(w, h, 1, IMAGE_R8G8B8A8))
            if(TIFFReadRGBAImageOriented(tif, w, h, (uint32*)data(), ORIENTATION_TOPLEFT, 0))
      {
         ok=true;
         if(photometric!=PHOTOMETRIC_PALETTE) // keep palette as RGBA
            if(samples==1 || samples==3)ok=copyTry(T, -1, -1, -1, (samples==1) ? IMAGE_L8 : IMAGE_R8G8B8);
      }
      TIFFClose(tif);
      f.pos(tf.end); // set position at the end of the read data, we have to do this because file pointer may not necessary be at the end, also don't use just 'f.size' in case there is some data after TIFF
      if(ok && f.ok())return true;
   }
#endif
   del(); return false;
}
/******************************************************************************/
Bool Image::ExportTIF(File &f, Flt compression_level)C
{
#if SUPPORT_TIF
   Image  temp;
 C Image *src=this;
   IMAGE_TYPE uncompressed_type=src->type(); if(ImageTI[uncompressed_type].compressed)uncompressed_type=IMAGE_R8G8B8A8;
   if(src->cube      ()){   temp.fromCube(*src ,             uncompressed_type, IMAGE_SOFT   ); src=&temp;}
   if(src->compressed()){if(src->copyTry ( temp, -1, -1, -1, uncompressed_type, IMAGE_SOFT, 1)) src=&temp;else return false;}

   Bool ok=false;
   if(src->lockRead())
   {
      TIFFile tf(f);
      TIFFSetErrorHandler     (null); // disable error handlers, otherwise TIF library will create system message boxes
      TIFFSetErrorHandlerExt  (null);
      TIFFSetWarningHandler   (null);
      TIFFSetWarningHandlerExt(null);
      if(TIFF *tif=TIFFClientOpen("", "w", &tf, TIFRead, TIFWrite, TIFSeek, TIFClose, TIFSize, null, null)) // can't use null because crash will occur
      {
         ok=true;
       C ImageTypeInfo &image_ti=ImageTI[src->type()];
         Bool alpha   =(image_ti.a>0), bit16=(src->type()==IMAGE_I16);
         Byte channels= image_ti.channels; if(channels==2)channels=3;
         TIFFSetField(tif, TIFFTAG_IMAGEWIDTH     , src->w());
         TIFFSetField(tif, TIFFTAG_IMAGELENGTH    , src->h());
         TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels);
         TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE  , bit16 ? 16 : 8);
         TIFFSetField(tif, TIFFTAG_ORIENTATION    , ORIENTATION_TOPLEFT);
         TIFFSetField(tif, TIFFTAG_PLANARCONFIG   , PLANARCONFIG_CONTIG);
         TIFFSetField(tif, TIFFTAG_PHOTOMETRIC    , (channels==1) ? PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB);
         TIFFSetField(tif, TIFFTAG_COMPRESSION    , (compression_level<0) ? COMPRESSION_NONE : (compression_level>0.5f) ? COMPRESSION_LZW : COMPRESSION_NONE);
         TIFFSetField(tif, TIFFTAG_PREDICTOR      , PREDICTOR_HORIZONTAL);
         Memt<Byte> buf; buf.setNum(src->w()*channels*(bit16 ? 2 : 1));
         FREPD(y, src->h())
         {
            Byte *b=buf.data();
            if(bit16)CopyFast(b, src->data()+src->pitch()*y, buf.elms());else
            FREPD(x, src->w())
            {
               Color c=src->color(x, y);
               switch(channels)
               {
                  case 1:
                  {
                     b[0]=(alpha ? c.a : c.r);
                     b++;
                  }break;

                  case 3:
                  {
                     b[0]=c.r;
                     b[1]=c.g;
                     b[2]=c.b;
                     b+=3;
                  }break;

                  case 4:
                  {
                     b[0]=c.r;
                     b[1]=c.g;
                     b[2]=c.b;
                     b[3]=c.a;
                     b+=4;
                  }break;
               }
            }
            if(TIFFWriteScanline(tif, buf.data(), y)<0){ok=false; break;}
         }
         TIFFClose(tif);
         f.pos(tf.end); // set position at the end of the written data, we have to do this because 'TIFFClose' may perform seeking to write some data, also don't use just 'f.size' in case it had already some data before exporting
         ok&=f.ok();
      }
      src->unlock();
   }
   return ok;
#endif
   return false;
}
/******************************************************************************/
Bool Image::ExportTIF(C Str &name, Flt compression_level)C
{
   File f; if(f.writeTry(name)){if(ExportTIF(f, compression_level) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Image::ImportTIF(C Str &name)
{
   File f; if(f.readTry(name))return ImportTIF(f);
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
