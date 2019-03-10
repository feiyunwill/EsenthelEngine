/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define DDSCAPS2_CUBEMAP           0x200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x1000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x2000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x4000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x8000
#define DDSCAPS2_VOLUME            0x200000
#define DDPF_FOURCC                0x4
#define DDPF_LUMINANCE             0x20000
/******************************************************************************/
#pragma pack(push, 4)
struct DDS_PIXELFORMAT
{
   UInt Size, Flags, FourCC, RGBBitCount, RBitMask, GBitMask, BBitMask, ABitMask;

   IMAGE_TYPE type()
   {
      if(!(Flags&DDPF_FOURCC))FourCC=0;
      if(FourCC==CC4('D', 'X', '1', '0'))return IMAGE_NONE; // special DX10+ format

      if(FourCC==CC4('D', 'X', 'T', '1'))return IMAGE_DXT1;
      if(FourCC==CC4('D', 'X', 'T', '2'))return IMAGE_DXT3;
      if(FourCC==CC4('D', 'X', 'T', '3'))return IMAGE_DXT3;
      if(FourCC==CC4('D', 'X', 'T', '4'))return IMAGE_DXT5;
      if(FourCC==CC4('D', 'X', 'T', '5'))return IMAGE_DXT5;

      // fix 'RGBBitCount' for these formats, because it may be 0
      if(FourCC==111){RGBBitCount=16  ; return IMAGE_F16  ;}
      if(FourCC==112){RGBBitCount=16*2; return IMAGE_F16_2;}
      if(FourCC==113){RGBBitCount=16*4; return IMAGE_F16_4;}
      if(FourCC==114){RGBBitCount=32  ; return IMAGE_F32  ;}
      if(FourCC==115){RGBBitCount=32*2; return IMAGE_F32_2;}
      if(FourCC==116){RGBBitCount=32*4; return IMAGE_F32_4;}

      if(Flags&DDPF_LUMINANCE)
      {
         if(RGBBitCount== 8 && RBitMask==0x000000FF && GBitMask==0x00000000 && BBitMask==0x00000000 && ABitMask==0x00000000)return IMAGE_L8;
         if(RGBBitCount==16 && RBitMask==0x000000FF && GBitMask==0x00000000 && BBitMask==0x00000000 && ABitMask==0x0000FF00)return IMAGE_L8A8;
         if(RGBBitCount==16 && RBitMask==0x0000FFFF && GBitMask==0x00000000 && BBitMask==0x00000000 && ABitMask==0x00000000)return IMAGE_I16;
      }
      if(RGBBitCount==32 && RBitMask==0x00FF0000 && GBitMask==0x0000FF00 && BBitMask==0x000000FF && ABitMask==0xFF000000)return IMAGE_B8G8R8A8;
      if(RGBBitCount==32 && RBitMask==0x000000FF && GBitMask==0x0000FF00 && BBitMask==0x00FF0000 && ABitMask==0xFF000000)return IMAGE_R8G8B8A8;
      if(RGBBitCount==24 && RBitMask==0x000000FF && GBitMask==0x0000FF00 && BBitMask==0x00FF0000 && ABitMask==0x00000000)return IMAGE_R8G8B8;
      if(RGBBitCount==16 && RBitMask==0x000000FF && GBitMask==0x0000FF00 && BBitMask==0x00000000 && ABitMask==0x00000000)return IMAGE_R8G8;
      if(RGBBitCount== 8 && RBitMask==0x000000FF && GBitMask==0x00000000 && BBitMask==0x00000000 && ABitMask==0x00000000)return IMAGE_R8;
      if(RGBBitCount== 8 && RBitMask==0x00000000 && GBitMask==0x00000000 && BBitMask==0x00000000 && ABitMask==0x000000FF)return IMAGE_A8;
      if(RGBBitCount==16 && RBitMask==0x00000F00 && GBitMask==0x000000F0 && BBitMask==0x0000000F && ABitMask==0x00000000)return IMAGE_B4G4R4X4;
      if(RGBBitCount==16 && RBitMask==0x00000F00 && GBitMask==0x000000F0 && BBitMask==0x0000000F && ABitMask==0x0000F000)return IMAGE_B4G4R4A4;
      if(RGBBitCount==16 && RBitMask==0x00007C00 && GBitMask==0x000003E0 && BBitMask==0x0000001F && ABitMask==0x00000000)return IMAGE_B5G5R5X1;
      if(RGBBitCount==16 && RBitMask==0x00007C00 && GBitMask==0x000003E0 && BBitMask==0x0000001F && ABitMask==0x00008000)return IMAGE_B5G5R5A1;
      if(RGBBitCount==16 && RBitMask==0x0000F800 && GBitMask==0x000007E0 && BBitMask==0x0000001F && ABitMask==0x00000000)return IMAGE_B5G6R5;
      if(RGBBitCount==24 && RBitMask==0x00FF0000 && GBitMask==0x0000FF00 && BBitMask==0x000000FF && ABitMask==0x00000000)return IMAGE_B8G8R8;
      if(RGBBitCount==32 && RBitMask==0x00FF0000 && GBitMask==0x0000FF00 && BBitMask==0x000000FF && ABitMask==0x00000000)return IMAGE_B8G8R8X8;
      if(RGBBitCount==32 && RBitMask==0x000000FF && GBitMask==0x0000FF00 && BBitMask==0x00FF0000 && ABitMask==0x00000000)return IMAGE_R8G8B8X8;
      if(RGBBitCount==32 && RBitMask==0x000003FF && GBitMask==0x000FFC00 && BBitMask==0x3FF00000 && ABitMask==0xC0000000)return IMAGE_R10G10B10A2;
      return IMAGE_NONE;
   }
};
struct DDS_HEADER
{
   UInt            Size, Flags, Height, Width, PitchOrLinearSize, Depth, MipMapCount, Reserved1[11];
   DDS_PIXELFORMAT pf;
   UInt            Caps, Caps2, Caps3, Caps4, Reserved2;

   IMAGE_TYPE load(File &f)
   {
      ASSERT(SIZE(DDS_HEADER)==124 && SIZE(DDS_PIXELFORMAT)==32);
      if(f.getFast(T))
      if(Size==SIZE(DDS_HEADER) && pf.Size==SIZE(DDS_PIXELFORMAT))
      if(!(Caps2&DDSCAPS2_CUBEMAP) && !(Caps2&DDSCAPS2_VOLUME)) // Cube and Volume textures are not currently supported
      if(IMAGE_TYPE type=pf.type())
      {
         switch(type) // many DDS writers incorrectly setup 'PitchOrLinearSize', so we have to adjust it manually, according to https://docs.microsoft.com/en-us/windows/desktop/direct3ddds/dx-graphics-dds-pguide#dds-file-layout
         {
            case IMAGE_BC1:                                 PitchOrLinearSize=DivCeil4(Width)* 8; break;
            case IMAGE_BC2: case IMAGE_BC3: case IMAGE_BC7: PitchOrLinearSize=DivCeil4(Width)*16; break;

            default: PitchOrLinearSize=DivCeil8(Width*pf.RGBBitCount); break;
         }
         return type;
      }
      return IMAGE_NONE;
   }
};
#pragma pack(pop)
/******************************************************************************/
Bool Image::ImportDDS(File &f, Int type, Int mode, Int mip_maps)
{
   if(mode>=0 && mode!=IMAGE_2D && mode!=IMAGE_SOFT)return false; // if mode is specified, and not SOFT/2D then fail
   if(f.getUInt()==CC4('D','D','S',' '))
   {
      DDS_HEADER header; if(IMAGE_TYPE t=header.load(f))
      {
         if(type    <=0)type    =t; if(type>=IMAGE_TYPES)type=IMAGE_R8G8B8A8;
         if(mode    < 0)mode    =IMAGE_SOFT; // always default to SOFT like all other file formats
         if(mip_maps< 0)mip_maps=((header.MipMapCount>1 && mode==IMAGE_2D) ? 0 : 1);

         if(t==type && createTry(header.Width, header.Height, 1, t, IMAGE_MODE(mode), mip_maps, false)  // if conversion is not required, then try desired values
         ||            createTry(header.Width, header.Height, 1, t, IMAGE_SOFT      ,        1, false)) // otherwise import as soft
            if(lock(LOCK_WRITE))
         {
            Int pitch   =ImagePitch  (w(), h(), 0, hwType()), // use "w(), h()" instead of "hwW(), hwH()" because we want to read only valid pixels and zero others
                blocks_y=ImageBlocksY(w(), h(), 0, hwType()); // use "w(), h()" instead of "hwW(), hwH()" because we want to read only valid pixels and zero others
            if(header.PitchOrLinearSize>=pitch)
            {
               Int   skip=header.PitchOrLinearSize-pitch, zero=T.pitch()-pitch;
               Byte *data=T.data();
               FREPD(y, blocks_y)
               {
                  f.getFast(data, pitch);
                  f.skip(skip);
                  Zero(data+pitch, zero);
                  data+=T.pitch();
               }
               Zero(data, T.pitch2()-blocks_y*T.pitch());
               unlock();
               if(f.ok())
               {
                  if(type!=T.type() || mode!=T.mode())
                  {
                     if(!copyTry(T, -1, -1, -1, type, mode, mip_maps))goto error;
                  }else updateMipMaps();
                  return true;
               }
            }
         }
      }
   }
error:
   del(); return false;
}
Bool Image::ImportDDS(C Str &name, Int type, Int mode, Int mip_maps)
{
   File f; if(f.readTry(name))return ImportDDS(f, type, mode, mip_maps);
   del(); return false;
}
Bool Image::ImportDDS(C Str  &name) {return ImportDDS(name, -1);}
Bool Image::ImportDDS(  File &f   ) {return ImportDDS(f   , -1);}
/******************************************************************************/
//Bool ExportDX    (C Str &name, GPU_API(D3DXIMAGE_FILEFORMAT, D3DX11_IMAGE_FILE_FORMAT, UInt) format)C; // export using DirectX
//Bool Image::ExportDDS(C Str &name)C {return ExportDX(name, GPU_API(D3DXIFF_DDS, D3DX11_IFF_DDS, 0));}
Bool Image::ExportDDS(C Str &name)C {return false;}
/******************************************************************************/
}
/******************************************************************************/
