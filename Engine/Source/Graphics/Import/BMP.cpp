/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#pragma pack(push, 1)
struct BitmapFileHeader
{
   UShort type;
   UInt   size, reserved, offBits;
};
struct BitmapInfoHeader
{
   UInt   size;
   Int    width, height;
   UShort planes, bitCount;
   UInt   compression, sizeImage;
   Int    xPelsPerMeter, yPelsPerMeter;
   UInt   clrUsed, clrImportant;
};
#pragma pack(pop)
/******************************************************************************/
Bool Image::ImportBMPRaw(File &f, Bool ico)
{
   BitmapInfoHeader bmih; f>>bmih;
   if(Unaligned(bmih.planes)==1)
      if(Unaligned(bmih.bitCount)==8 || Unaligned(bmih.bitCount)==24 || Unaligned(bmih.bitCount)==32)
   {
      if(ico)Unaligned(bmih.height, Unaligned(bmih.height)/2); // ICO files have height 2x bigger
      MemtN<Color, 256> palette; if(Unaligned(bmih.bitCount)==8){f.getN(palette.setNum(256).data(), 256); REPA(palette)Swap(palette[i].r, palette[i].b); if(ico)REPAO(palette).a=(i ? 255 : 0);} // load palette, setup alpha for ICO
      if(createSoftTry(Unaligned(bmih.width), Unaligned(bmih.height), 1, (Unaligned(bmih.bitCount)==24) ? IMAGE_R8G8B8 : (Unaligned(bmih.bitCount)==32) ? IMAGE_B8G8R8A8 : ico ? IMAGE_B8G8R8A8 : IMAGE_R8G8B8)) // ICO has alpha channel, BMP uses BGRA order
      {
         Int zeros=Ceil4(pitch())-pitch();
         REPD(y, T.h())
         {
            switch(Unaligned(bmih.bitCount))
            {
               case  8: FREPD(x, T.w())color(x, y, palette[f.getByte()]);                                                  break;
               case 24: {Byte *data=T.data() + y*pitch(); f.get(data, T.w()*3); REP(T.w())Swap(data[i*3+0], data[i*3+2]);} break; // FREPD(x, T.w()){Color c; f>>c.b; f>>c.g; f>>c.r; c.a=255; color(x, y, c);}
               case 32: {Byte *data=T.data() + y*pitch(); f.get(data, T.w()*4);                                          } break; // FREPD(x, T.w()){Color c; f>>c.b; f>>c.g; f>>c.r; f>>c.a ; color(x, y, c);}
            }
            f.skip(zeros);
         }
         if(f.ok())return true;
      }
   }
   del(); return false;
}
Bool Image::ImportBMP(File &f)
{
   BitmapFileHeader bmfh; f>>bmfh;
   if(Unaligned(bmfh.type)==0x4D42)return ImportBMPRaw(f);
   del(); return false;
}
Bool Image::ImportBMP(C Str &name)
{
   File f; if(f.readTry(name))return ImportBMP(f);
   del(); return false;
}
/******************************************************************************/
static void GetSizeZeros(C Image &image, Byte byte_pp, Int &size, Int &zeros)
{
   size=image.w()*byte_pp; zeros=Ceil4(size)-size;
   size=(size+zeros)*image.h();
}
Bool Image::ExportBMPRaw(File &f, Byte byte_pp, Bool ico)C // assumes that Image is locked and of accessible data
{
   Int size, zeros; GetSizeZeros(T, byte_pp, size, zeros);
   Int mask_size; if(ico){mask_size=Ceil4(Ceil8(w())/8)*h(); size+=mask_size;} // include mask
   BitmapInfoHeader bmih;
   Unaligned(bmih.size         , SIZEU(BitmapInfoHeader));
   Unaligned(bmih.width        ,               w());
   Unaligned(bmih.height       , ico ? h()*2 : h()); // ICO files need to have height 2x bigger
  _Unaligned(bmih.planes       , 1);
  _Unaligned(bmih.bitCount     , byte_pp*8);
  _Unaligned(bmih.compression  , 0);
  _Unaligned(bmih.sizeImage    , size);
   Unaligned(bmih.xPelsPerMeter, 0xB12);
   Unaligned(bmih.yPelsPerMeter, 0xB12);
  _Unaligned(bmih.clrUsed      , (byte_pp==1) ? 256 : 0);
  _Unaligned(bmih.clrImportant , (byte_pp==1) ? 256 : 0);
   f<<bmih;

   if(byte_pp==1)FREP(256)f.putUInt(VecB4(i, i, i, 255).u); // palette

   REPD(y, T.h())
   {
      switch(byte_pp)
      {
         case 1:
         {
            if(bytePP()==1)f.put(data() + y*pitch(), T.w());else
            FREPD(x, T.w())f.putByte(FltToByte(pixelF(x, y)));
         }break;

         case 3:
         {
            if(hwType()==IMAGE_B8G8R8)f.put(data()+y*pitch(), T.w()*3);else
            FREPD(x, T.w()){Color c=color(x, y); Byte pixel[3]={c.b, c.g, c.r}; f<<pixel;}
         }break;

         case 4:
         {
            if(hwType()==IMAGE_B8G8R8A8)f.put(data()+y*pitch(), T.w()*4);else // BMP uses BGRA order
            FREPD(x, T.w()){Color c=color(x, y); Swap(c.r, c.b); f<<c;}
         }break;
      }
      f.put(null, zeros);
   }
   if(ico)f.put(null, mask_size);
   return f.ok();
}
Bool Image::ExportBMP(File &f)C
{
   Image  temp;
 C Image *src=this;
   IMAGE_TYPE uncompressed_type=src->type(); if(ImageTI[uncompressed_type].compressed)uncompressed_type=IMAGE_B8G8R8A8; // BMP uses BGRA order
   if(src->cube      ()){   temp.fromCube(*src ,             uncompressed_type, IMAGE_SOFT   ); src=&temp;}
   if(src->compressed()){if(src->copyTry ( temp, -1, -1, -1, uncompressed_type, IMAGE_SOFT, 1)) src=&temp;else return false;}

   Bool ok=false;
   if(src->lockRead())
   {
      Byte byte_pp=((ImageTI[T.type()].channels==1) ? 1 : ImageTI[T.type()].a ? 4 : 3); // use T.type to have precise information about source type
      Int  size, zeros; GetSizeZeros(*src, byte_pp, size, zeros);

      BitmapFileHeader bmfh;
     _Unaligned(bmfh.type    , 0x4D42);
     _Unaligned(bmfh.size    , SIZE(BitmapFileHeader)+SIZE(BitmapInfoHeader)+((byte_pp==1) ? 256*4 : 0)+size);
     _Unaligned(bmfh.reserved, 0);
     _Unaligned(bmfh.offBits , SIZE(BitmapFileHeader)+SIZE(BitmapInfoHeader)+((byte_pp==1) ? 256*4 : 0));
      f<<bmfh;

      ok=src->ExportBMPRaw(f, byte_pp);
      src->unlock();
   }
   return ok;
}
Bool Image::ExportBMP(C Str &name)C
{
   File f; if(f.writeTry(name)){if(ExportBMP(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
