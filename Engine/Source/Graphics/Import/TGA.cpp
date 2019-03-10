/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum TGA_FORMAT : Byte
{
   TGA_NULL    = 0,
   TGA_Map     = 1,
   TGA_RGB     = 2,
   TGA_Mono    = 3,
   TGA_RLEMap  = 9,
   TGA_RLERGB  =10,
   TGA_RLEMono =11,
   TGA_CompMap =32,
   TGA_CompMap4=33,
};
#pragma pack(push, 1)
struct TgaHeader
{
   Byte       IdLength;  // Image ID Field Length
   Byte       CmapType;  // Color Map Type
   TGA_FORMAT ImageType; // Image Type

   UShort CmapIndex;     // First Entry Index
   UShort CmapLength;    // Color Map Length
   Byte   CmapEntrySize; // Color Map Entry Size

   UShort X_Origin;    // X-origin of Image
   UShort Y_Origin;    // Y-origin of Image
   UShort ImageWidth;  // Image Width
   UShort ImageHeight; // Image Height
   Byte   PixelDepth;  // Pixel Depth
   Byte   ImagDesc;    // Image Descriptor
};
#pragma pack(pop)
struct TGA
{
   Bool              mono;
   UInt              bit_pp;
   MemtN<Color, 256> palette;

   void readUncompressed(Image &image, File &f, Int y, Int x_offset, Int width)
   {
	   switch(bit_pp)
	   {
	      case 8:
	      {
            if(palette.elms())FREPD(x, width)
            {
               Byte pixel; f>>pixel;
               image.color(x+x_offset, y, InRange(pixel, palette) ? palette[pixel] : TRANSPARENT);
            }else
	         if(image.bytePP()==1)f.get(image.data() + x_offset + y*image.pitch(), width);else
	         FREPD(x, width)
	         {
	            Byte pixel; f>>pixel;
	            image.color(x+x_offset, y, Color(pixel, pixel, pixel));
	         }
	      }break;

	      case 15:
	      case 16:
	      {
	         FREPD(x, width)
	         {
	            UShort pixel; f>>pixel;
	            image.color(x+x_offset, y, Color((pixel>>7)&0xF8, (pixel>>2)&0xF8, (pixel&0x1F)<<3));
	         }
		   }break;

	      case 24:
	      {
	         if(image.hwType()==IMAGE_R8G8B8 || image.hwType()==IMAGE_B8G8R8)
	         {
	            Byte *data=image.data() + x_offset*3 + y*image.pitch();
	            f.get(data, width*3);
	            if(image.hwType()==IMAGE_R8G8B8)REP(width)Swap(data[i*3+0], data[i*3+2]); // swap Red with Blue
	         }else
	         if(image.hwType()==IMAGE_B8G8R8A8)
	         {
	            VecB4 *data=(VecB4*)(image.data() + x_offset*4 + y*image.pitch());
	            FREPD(x, width)
	            {
	               Byte pixel[3]; f>>pixel;
	               (data++)->set(pixel[0], pixel[1], pixel[2], 255);
	            }
	         }else
	         FREPD(x, width)
	         {
	            Byte pixel[3]; f>>pixel;
	            image.color(x+x_offset, y, Color(pixel[2], pixel[1], pixel[0], 255));
	         }
		   }break;

	      case 32:
	      {
	         if(image.hwType()==IMAGE_R8G8B8A8 || image.hwType()==IMAGE_B8G8R8A8)
	         {
	            Byte *data=image.data() + x_offset*4 + y*image.pitch();
	            f.get(data, width*4);
	            if(image.hwType()==IMAGE_R8G8B8A8)REP(width)Swap(data[i*4+0], data[i*4+2]); // swap Red with Blue
	         }else
	         FREPD(x, width)
	         {
	            VecB4 pixel; f>>pixel;
	            image.color(x+x_offset, y, Color(pixel.z, pixel.y, pixel.x, pixel.w));
	         }
		   }break;
	   }
   }
   Byte readCompressed(Image &image, File &f, Int y, Byte rleLeftover)
   {
      Byte rle;
      Int  filePos=0;
      for(int x=0; x<image.w(); )
      {
         if(rleLeftover==255)f>>rle;else
         {
            rle=rleLeftover;
            rleLeftover=255;
         }

         if(rle&128) // RLE-encoded packet
         {
            rle-=127; // calculate real repeat count
            if(x+rle>image.w())
            {
               rleLeftover=Byte(128+(rle-(image.w()-x)-1));
               filePos=f.pos();
               rle=Byte(image.w()-x);
            }
            switch(bit_pp)
            {
               case 32:
               {
                  VecB4 pixel; f>>pixel; Color color(pixel.z, pixel.y, pixel.x, pixel.w);
                  REP(rle)image.color(x+i, y, color);
               }break;

               case 24:
               {
	               Byte pixel[3]; f>>pixel; Color color(pixel[2], pixel[1], pixel[0], 255);
	               REP(rle)image.color(x+i, y, color);
               }break;

               case 15:
               case 16:
               {
	               UShort pixel; f>>pixel; Color color((pixel>>7)&0xF8, (pixel>>2)&0xF8, (pixel&0x1F)<<3);
	               REP(rle)image.color(x+i, y, color);
	            }break;

               case 8:
               {
                  Byte pixel; f>>pixel;
                  if(palette.elms()){Color c=(InRange(pixel, palette) ? palette[pixel] : TRANSPARENT); REP(rle)image.color(x+i, y, c);}else
                  if(image.bytePP()==1)REP(rle)image.pixel(x+i, y, pixel);else{Color c(pixel, pixel, pixel); REP(rle)image.color(x+i, y, c);}
               }break;
            }
            if(rleLeftover!=255)f.pos(filePos);
         }else // raw packet
         {
            rle++; // calculate real repeat count
            if(x+rle>image.w())
            {
               rleLeftover=Byte(rle-(image.w()-x)-1);
               rle=Byte(image.w()-x);
            }
            readUncompressed(image, f, y, x, rle);
         }
         x+=rle;
      }
      return rleLeftover;
   }
};
/******************************************************************************/
Bool Image::ImportTGA(File &f, Int type, Int mode, Int mip_maps)
{
   if(mode!=IMAGE_2D && mode!=IMAGE_SOFT)mode=IMAGE_SOFT;
   if(mip_maps<0)mip_maps=1;

	TGA tga;
   TgaHeader header; f>>header;

   Bool compressed, map;
   switch(Unaligned(header.ImageType))
   {
      case TGA_Map : map=true ; tga.mono=false; compressed=false; break;
      case TGA_RGB : map=false; tga.mono=false; compressed=false; break;
      case TGA_Mono: map=false; tga.mono=true ; compressed=false; break;

      case TGA_RLEMap : map=true ; tga.mono=false; compressed=true; break;
      case TGA_RLERGB : map=false; tga.mono=false; compressed=true; break;
      case TGA_RLEMono: map=false; tga.mono=true ; compressed=true; break;

      default: return false;
   }

   if( Unaligned(header.ImageWidth)==0  ||  Unaligned(header.ImageHeight  )==0)return false;
   if( Unaligned(header.PixelDepth)!=8  &&  Unaligned(header.PixelDepth   )!=15 && Unaligned(header.PixelDepth)!=16 && Unaligned(header.PixelDepth)!=24 && Unaligned(header.PixelDepth)!=32)return false;
   if( Unaligned(header.CmapType  )!=0  &&  Unaligned(header.CmapType     )!=1 )return false; // only 0 and 1 types are supported
   if( Unaligned(header.CmapType  )     &&  Unaligned(header.CmapEntrySize)!=24)return false; // if color map exists but entry is not 24-bit
   if((Unaligned(header.CmapType  )!=0) !=  map                                )return false; // if color map existence is different than map type
   if((Unaligned(header.CmapType  )!=0) != (Unaligned(header.CmapLength   )!=0))return false; // if color map existence is different than map length
   f.skip(Unaligned(header.IdLength)); // skip descriptor

   tga.bit_pp=Unaligned(header.PixelDepth);
   tga.palette.setNum(Unaligned(header.CmapLength));
   FREPA(tga.palette){Color &c=tga.palette[i]; f>>c.b>>c.g>>c.r; c.a=255;}

   Bool mirror_x=FlagTest(Unaligned(header.ImagDesc), 16),
        mirror_y=FlagTest(Unaligned(header.ImagDesc), 32);

   if(type<=0)if(map)type=IMAGE_R8G8B8;else switch(Unaligned(header.PixelDepth))
   {
      case 15:
      case 16: type=(tga.mono ? IMAGE_I16 : IMAGE_R8G8B8  ); break;
      case 24: type=(tga.mono ? IMAGE_I24 : IMAGE_R8G8B8  ); break;
      case 32: type=(tga.mono ? IMAGE_I32 : IMAGE_B8G8R8A8); break; // TGA uses BGRA order
      case 8 : type=            IMAGE_L8                   ; break;
   }

   if(createTry(Unaligned(header.ImageWidth), Unaligned(header.ImageHeight), 1, ImageTI[type].compressed ? IMAGE_B8G8R8A8 : IMAGE_TYPE(type), IMAGE_MODE(mode), ImageTI[type].compressed ? 1 : mip_maps) // TGA uses BGRA order
   && lock(LOCK_WRITE))
   {
      Byte rleLeftover=255;
      FREPD(y, T.h())
      {
         if(f.end())return false;
         Int dy=(mirror_y ? y : T.h()-y-1);
         if(compressed)rleLeftover=tga.  readCompressed(T, f, dy, rleLeftover);
         else                      tga.readUncompressed(T, f, dy, 0, T.w());
      }
      if(mirror_x)mirrorX();

      unlock();
      if(ImageTI[type].compressed)
      {
         if(!copyTry(T, -1, -1, -1, type, -1, mip_maps))goto error;
      }else updateMipMaps();
      return true;
   }
error:
   del(); return false;
}
/******************************************************************************/
Bool Image::ExportTGA(File &f)C
{
   Image  temp;
 C Image *src=this;
   IMAGE_TYPE uncompressed_type=src->type(); if(ImageTI[uncompressed_type].compressed)uncompressed_type=IMAGE_B8G8R8A8; // TGA uses BGRA order
   if(src->cube      ()){   temp.fromCube(*src ,             uncompressed_type, IMAGE_SOFT   ); src=&temp;}
   if(src->compressed()){if(src->copyTry ( temp, -1, -1, -1, uncompressed_type, IMAGE_SOFT, 1)) src=&temp;else return false;}

   Bool ok=false;
   if(src->lockRead())
   {
      Byte byte_pp=((ImageTI[T.type()].channels==1) ? 1 : ImageTI[T.type()].a ? 4 : 3); // use T.type to have precise information about source type

      TgaHeader header; Zero(header);
      Unaligned(header.ImageType  , (byte_pp<=1) ? TGA_Mono : TGA_RGB);
     _Unaligned(header.ImageWidth , src->w());
     _Unaligned(header.ImageHeight, src->h());
     _Unaligned(header.PixelDepth , byte_pp*8);
     _Unaligned(header.ImagDesc   , 32); // mirror_y
      f<<header;

      FREPD(y, src->h())switch(byte_pp)
      {
         case 1:
         {
	         if(src->bytePP()==1)f.put(src->data() + y*src->pitch(), src->w());else
	         FREPD(x, src->w())f.putByte(FltToByte(src->pixelF(x, y)));
         }break;

         case 3:
         {
	         if(src->hwType()==IMAGE_B8G8R8)f.put(src->data() + y*src->pitch(), src->w()*3);else
	         FREPD(x, src->w()){Color c=src->color(x, y); Byte pixel[3]={c.b, c.g, c.r}; f<<pixel;}
         }break;

	      case 4:
	      {
	         if(src->hwType()==IMAGE_B8G8R8A8)f.put(src->data() + y*src->pitch(), src->w()*4);else
	         FREPD(x, src->w()){Color c=src->color(x, y); Swap(c.r, c.b); f<<c;}
		   }break;
      }

      ok=f.ok();
      src->unlock();
   }
   return ok;
}
/******************************************************************************/
Bool Image::ImportTGA(  File &f   ) {return ImportTGA(f   , -1);}
Bool Image::ImportTGA(C Str  &name) {return ImportTGA(name, -1);}
Bool Image::ExportTGA(C Str  &name)C
{
   File f; if(f.writeTry(name)){if(ExportTGA(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Image::ImportTGA(C Str &name, Int type, Int mode, Int mip_maps)
{
   File f; if(f.readTry(name))return ImportTGA(f, type, mode, mip_maps);
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
