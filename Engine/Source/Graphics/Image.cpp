/******************************************************************************/
#include "stdafx.h"
#include "../Platforms/iOS/iOS.h"
namespace EE{
/******************************************************************************/
#include "Import/BC.h"
#include "Import/ETC.h"
#include "Import/PVRTC.h"

#if DX9
   // make sure that there's direct mapping for 'D3DMULTISAMPLE_TYPE'
   ASSERT(D3DMULTISAMPLE_2_SAMPLES==2 && D3DMULTISAMPLE_3_SAMPLES==3 && D3DMULTISAMPLE_16_SAMPLES==16);

   // make sure that there's direct mapping for cube face
   ASSERT(D3DCUBEMAP_FACE_POSITIVE_X==DIR_RIGHT
       && D3DCUBEMAP_FACE_NEGATIVE_X==DIR_LEFT
       && D3DCUBEMAP_FACE_POSITIVE_Y==DIR_UP
       && D3DCUBEMAP_FACE_NEGATIVE_Y==DIR_DOWN
       && D3DCUBEMAP_FACE_POSITIVE_Z==DIR_FORWARD
       && D3DCUBEMAP_FACE_NEGATIVE_Z==DIR_BACK);
#elif DX11
   // make sure that there's direct mapping for cube face
   ASSERT(D3D11_TEXTURECUBE_FACE_POSITIVE_X==DIR_RIGHT
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_X==DIR_LEFT
       && D3D11_TEXTURECUBE_FACE_POSITIVE_Y==DIR_UP
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_Y==DIR_DOWN
       && D3D11_TEXTURECUBE_FACE_POSITIVE_Z==DIR_FORWARD
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_Z==DIR_BACK);
#elif GL
   // make sure that there's direct mapping for cube face
   ASSERT(GL_TEXTURE_CUBE_MAP_POSITIVE_X-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_RIGHT
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_X-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_LEFT
       && GL_TEXTURE_CUBE_MAP_POSITIVE_Y-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_UP
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_Y-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_DOWN
       && GL_TEXTURE_CUBE_MAP_POSITIVE_Z-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_FORWARD
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_Z-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_BACK);
#endif

#define GL_ETC1_RGB8                                0x8D64
#define GL_COMPRESSED_RGBA_S3TC_DXT1                0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3                0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5                0x83F3
#define GL_COMPRESSED_RGBA_BPTC_UNORM               0x8E8C
#define GL_COMPRESSED_RGB8_ETC2                     0x9274
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_RGBA8_ETC2                    0x9278
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG         0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG         0x8C03
#define GL_BGR                                      0x80E0
#define GL_BGRA                                     0x80E1
#define GL_UNSIGNED_INT_2_10_10_10_REV              0x8368
#define GL_ALPHA8                                   0x803C
#define GL_LUMINANCE8                               0x8040
#define GL_LUMINANCE8_ALPHA8                        0x8045
#define GL_LUMINANCE16                              0x8042
#define GL_LUMINANCE                                0x1909
#define GL_LUMINANCE_ALPHA                          0x190A
#define GL_TEXTURE_MAX_ANISOTROPY                   0x84FE
#define GL_HALF_FLOAT_OES                           0x8D61

#define GL_SWIZZLE (GL && !GL_ES) // Modern Desktop OpenGL (3.2) does not support GL_ALPHA8, GL_LUMINANCE8, GL_LUMINANCE8_ALPHA8, use swizzle instead
/******************************************************************************/
DEFINE_CACHE(Image, Images, ImagePtr, "Image");
 const ImagePtr ImageNull;
static SyncLock ImageSoftLock; // it's important to use a separate lock from 'D._lock' so we don't need to wait for GPU to finish drawing
/******************************************************************************/
const ImageTypeInfo ImageTI[IMAGE_ALL_TYPES]= // !! in case multiple types have the same format, preferred version must be specified in 'ImageFormatToType' !!
{
   {"None"       , false,  0,  0,   0, 0, 0, 0,   0,0, 0, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, 0)},

   {"B8G8R8A8"   , false,  4, 32,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A8R8G8B8, DXGI_FORMAT_B8G8R8A8_UNORM, 0       )},
   {"R8G8B8A8"   , false,  4, 32,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A8B8G8R8, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8)},
   {"R8G8B8"     , false,  3, 24,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN , DXGI_FORMAT_UNKNOWN       , GL_RGB8 )},
   {"R8G8"       , false,  2, 16,   8, 8, 0, 0,   0,0, 2, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN , DXGI_FORMAT_R8G8_UNORM    , GL_RG8  )},
   {"R8"         , false,  1,  8,   8, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN , DXGI_FORMAT_R8_UNORM      , GL_R8   )},

   {"A8"         , false,  1,  8,   0, 0, 0, 8,   0,0, 1, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A8  , DXGI_FORMAT_A8_UNORM, GL_SWIZZLE ? GL_R8  : GL_ALPHA8           )},
   {"L8"         , false,  1,  8,   0, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_8 , GPU_API(D3DFMT_L8  , DXGI_FORMAT_UNKNOWN , GL_SWIZZLE ? GL_R8  : GL_LUMINANCE8       )},
   {"L8A8"       , false,  2, 16,   0, 0, 0, 8,   0,0, 2, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A8L8, DXGI_FORMAT_UNKNOWN , GL_SWIZZLE ? GL_RG8 : GL_LUMINANCE8_ALPHA8)},

   {"BC1"        , true ,  0,  4,   5, 6, 5, 1,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_DXT1, DXGI_FORMAT_BC1_UNORM, GL_COMPRESSED_RGBA_S3TC_DXT1)},
   {"BC2"        , true ,  1,  8,   5, 6, 5, 4,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_DXT3, DXGI_FORMAT_BC2_UNORM, GL_COMPRESSED_RGBA_S3TC_DXT3)},
   {"BC3"        , true ,  1,  8,   5, 6, 5, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_DXT5, DXGI_FORMAT_BC3_UNORM, GL_COMPRESSED_RGBA_S3TC_DXT5)},

   {"I8"         , false,  1,  8,   8, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_UNKNOWN           , 0)},
   {"I16"        , false,  2, 16,  16, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_16, GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_UNKNOWN           , 0)},
   {"I24"        , false,  3, 24,  24, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_24, GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_UNKNOWN           , 0)},
   {"I32"        , false,  4, 32,  32, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_32, GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_UNKNOWN           , 0)},
   {"F16"        , false,  2, 16,  16, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_16, GPU_API(D3DFMT_R16F         , DXGI_FORMAT_R16_FLOAT         , GL_R16F   )},
   {"F32"        , false,  4, 32,  32, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_32, GPU_API(D3DFMT_R32F         , DXGI_FORMAT_R32_FLOAT         , GL_R32F   )},
   {"F16_2"      , false,  4, 32,  16,16, 0, 0,   0,0, 2, IMAGE_PRECISION_16, GPU_API(D3DFMT_G16R16F      , DXGI_FORMAT_R16G16_FLOAT      , GL_RG16F  )},
   {"F32_2"      , false,  8, 64,  32,32, 0, 0,   0,0, 2, IMAGE_PRECISION_32, GPU_API(D3DFMT_G32R32F      , DXGI_FORMAT_R32G32_FLOAT      , GL_RG32F  )},
   {"F16_3"      , false,  6, 48,  16,16,16, 0,   0,0, 3, IMAGE_PRECISION_16, GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_UNKNOWN           , GL_RGB16F )},
   {"F32_3"      , false, 12, 96,  32,32,32, 0,   0,0, 3, IMAGE_PRECISION_32, GPU_API(D3DFMT_UNKNOWN      , DXGI_FORMAT_R32G32B32_FLOAT   , GL_RGB32F )},
   {"F16_4"      , false,  8, 64,  16,16,16,16,   0,0, 4, IMAGE_PRECISION_16, GPU_API(D3DFMT_A16B16G16R16F, DXGI_FORMAT_R16G16B16A16_FLOAT, GL_RGBA16F)},
   {"F32_4"      , false, 16,128,  32,32,32,32,   0,0, 4, IMAGE_PRECISION_32, GPU_API(D3DFMT_A32B32G32R32F, DXGI_FORMAT_R32G32B32A32_FLOAT, GL_RGBA32F)},

   {"PVRTC1_2"   , true ,  0,  2,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)},
   {"PVRTC1_4"   , true ,  0,  4,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)},

   {"ETC1"       , true ,  0,  4,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_ETC1_RGB8)},
   {"ETC2"       , true ,  0,  4,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGB8_ETC2)},
   {"ETC2_A1"    , true ,  0,  4,   8, 8, 8, 1,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)},
   {"ETC2_A8"    , true ,  1,  8,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA8_ETC2)},

   {"BC7"        , true ,  1,  8,   7, 7, 7, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_BC7_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM)},

   {"R10G10B10A2", false,  4, 32,  10,10,10, 2,   0,0, 4, IMAGE_PRECISION_10, GPU_API(D3DFMT_A2B10G10R10, DXGI_FORMAT_R10G10B10A2_UNORM, GL_RGB10_A2)},

   {null           , false,  0,  0,   0, 0, 0, 0,   0,0, 0, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN                        , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"B4G4R4X4"     , false,  2, 16,   4, 4, 4, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_X4R4G4B4                       , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"B4G4R4A4"     , false,  2, 16,   4, 4, 4, 4,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A4R4G4B4                       , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"B5G5R5X1"     , false,  2, 16,   5, 5, 5, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_X1R5G5B5                       , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"B5G5R5A1"     , false,  2, 16,   5, 5, 5, 1,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_A1R5G5B5                       , DXGI_FORMAT_B5G5R5A1_UNORM   , 0                    )},
   {"B5G6R5"       , false,  2, 16,   5, 6, 5, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_R5G6B5                         , DXGI_FORMAT_B5G6R5_UNORM     , 0                    )},
   {"B8G8R8"       , false,  3, 24,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_R8G8B8                         , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"B8G8R8X8"     , false,  4, 32,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_X8R8G8B8                       , DXGI_FORMAT_B8G8R8X8_UNORM   , 0                    )},
   {"R8G8B8X8"     , false,  4, 32,   8, 8, 8, 0,   0,0, 3, IMAGE_PRECISION_8 , GPU_API(D3DFMT_X8B8G8R8                       , DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"R8G8B8A8_SIGN", false,  4, 32,   8, 8, 8, 8,   0,0, 4, IMAGE_PRECISION_8 , GPU_API(D3DFMT_Q8W8V8U8                       , DXGI_FORMAT_R8G8B8A8_SNORM   , GL_RGBA8_SNORM       )},
   {"R8G8_SIGN"    , false,  2, 16,   8, 8, 0, 0,   0,0, 2, IMAGE_PRECISION_8 , GPU_API(D3DFMT_V8U8                           , DXGI_FORMAT_R8G8_SNORM       , GL_RG8_SNORM         )},
   {"R8_SIGN"      , false,  1,  8,   8, 0, 0, 0,   0,0, 1, IMAGE_PRECISION_8 , GPU_API(D3DFMT_UNKNOWN                        , DXGI_FORMAT_R8_SNORM         , GL_R8_SNORM          )},
   {"D16"          , false,  2, 16,   0, 0, 0, 0,  16,0, 1, IMAGE_PRECISION_16, GPU_API(D3DFMT_D16                            , DXGI_FORMAT_D16_UNORM        , GL_DEPTH_COMPONENT16 )},
   {"D24X8"        , false,  4, 32,   0, 0, 0, 0,  24,0, 1, IMAGE_PRECISION_24, GPU_API(D3DFMT_D24X8                          , DXGI_FORMAT_D24_UNORM_S8_UINT, GL_DEPTH_COMPONENT24 )},
   {"D24S8"        , false,  4, 32,   0, 0, 0, 0,  24,8, 2, IMAGE_PRECISION_24, GPU_API(D3DFMT_D24S8                          , DXGI_FORMAT_D24_UNORM_S8_UINT, GL_DEPTH24_STENCIL8  )},
   {"D32"          , false,  4, 32,   0, 0, 0, 0,  32,0, 1, IMAGE_PRECISION_32, GPU_API(D3DFMT_D32                            , DXGI_FORMAT_D32_FLOAT        , GL_DEPTH_COMPONENT32F)},
   {"RAWZ"         , false,  4, 32,   0, 0, 0, 0,  24,8, 2, IMAGE_PRECISION_24, GPU_API(D3DFORMAT(MAKEFOURCC('R','A','W','Z')), DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"INTZ"         , false,  4, 32,   0, 0, 0, 0,  24,8, 2, IMAGE_PRECISION_24, GPU_API(D3DFORMAT(MAKEFOURCC('I','N','T','Z')), DXGI_FORMAT_UNKNOWN          , 0                    )},
   {"DF24"         , false,  4, 32,   0, 0, 0, 0,  24,0, 1, IMAGE_PRECISION_24, GPU_API(D3DFORMAT(MAKEFOURCC('D','F','2','4')), DXGI_FORMAT_UNKNOWN          , 0                    )}, // DF24 does not have stencil buffer
   {"NULL"         , false,  0,  0,   0, 0, 0, 0,   0,0, 0, IMAGE_PRECISION_8 , GPU_API(D3DFORMAT(MAKEFOURCC('N','U','L','L')), DXGI_FORMAT_UNKNOWN          , 0                    )},
}; ASSERT(IMAGE_ALL_TYPES==52);
/******************************************************************************/
Bool IsSoft(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return false;
      
      case IMAGE_SOFT:
      case IMAGE_SOFT_CUBE: return true;
   }
}
Bool IsHW(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return true;
      
      case IMAGE_SOFT:
      case IMAGE_SOFT_CUBE: return false;
   }
}
Bool IsCube(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return false;

      case IMAGE_CUBE     :
      case IMAGE_SOFT_CUBE:
      case IMAGE_RT_CUBE  : return true;
   }
}
/******************************************************************************/
IMAGE_TYPE BytesToImageType(Int byte_pp)
{
   switch(byte_pp)
   {
      default: return IMAGE_NONE;
      case  1: return IMAGE_I8;
      case  2: return IMAGE_I16;
      case  3: return IMAGE_I24;
      case  4: return IMAGE_I32;
   }
}
/******************************************************************************/
Int PaddedWidth(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4)w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
   Int mw=Max(1, w>>mip);
   if(ImageTI[type].compressed)switch(type)
   {
      case IMAGE_PVRTC1_2: return Max(Ceil8(mw), 16); // blocks are sized 8x4 pixels, min texture size is 16x8
      case IMAGE_PVRTC1_4: return Max(Ceil4(mw),  8); // blocks are sized 4x4 pixels, min texture size is  8x8
      default            : return     Ceil4(mw)     ; // blocks are sized 4x4 pixels, min texture size is  4x4
   }                       return           mw      ;
}
Int PaddedHeight(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4)w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
   Int mh=Max(1, h>>mip);
   if(ImageTI[type].compressed)switch(type)
   {
      case IMAGE_PVRTC1_2: return Max(Ceil4(mh), 8); // blocks are sized 8x4 pixels, min texture size is 16x8
      case IMAGE_PVRTC1_4: return Max(Ceil4(mh), 8); // blocks are sized 4x4 pixels, min texture size is  8x8
      default            : return     Ceil4(mh)    ; // blocks are sized 4x4 pixels, min texture size is  4x4
   }                       return           mh     ;
}
Int ImagePitch(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   w=PaddedWidth(w, h, mip, type);
   return ImageTI[type].compressed ? w*ImageTI[type].bit_pp/2 : w*ImageTI[type].byte_pp; // all compressed formats use 4 rows per block (4*bit_pp/8 == bit_pp/2)
}
Int ImageBlocksY(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   h=PaddedHeight(w, h, mip, type);
   return ImageTI[type].compressed ? h/4 : h; // all compressed formats use 4 rows per block
}
Int ImageMipSize(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   return ImagePitch  (w, h, mip, type)
         *ImageBlocksY(w, h, mip, type);
}
Int ImageMipSize(Int w, Int h, Int d, Int mip, IMAGE_TYPE type)
{
   return ImageMipSize(w, h, mip, type)*Max(1, d>>mip);
}
UInt ImageSize(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps)
{
   UInt   size=0; REP(mip_maps)size+=ImageMipSize(w, h, d, i, type); if(IsCube(mode))size*=6;
   return size;
}
/******************************************************************************/
Int TotalMipMaps(Int w, Int h, Int d, IMAGE_TYPE type)
{
   if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4){w=CeilPow2(w); h=CeilPow2(h);} // PVRTC1 format supports only Pow2 sizes
   Int    total=0; for(Int i=Max(w, h, d); i>=1; i>>=1)total++;
   return total;
}
static Bool ForceDisableMipMaps(C Image &image)
{
   return D._tex_pow2 && (image.hwW()!=CeilPow2(image.hwW()) || image.hwH()!=CeilPow2(image.hwH()));
}
/******************************************************************************/
Bool CompatibleLock(LOCK_MODE cur, LOCK_MODE lock)
{
   switch(cur)
   {
      default             :              // no lock yet
      case LOCK_READ_WRITE: return true; // full access

      case LOCK_WRITE     :
      case LOCK_APPEND    : return lock==LOCK_WRITE || lock==LOCK_APPEND;

      case LOCK_READ      : return lock==LOCK_READ;
   }
}
/******************************************************************************/
GPU_API(D3DFORMAT, DXGI_FORMAT, UInt) ImageTypeToFormat(Int                                   type  ) {return InRange(type, ImageTI) ? ImageTI[type].format : GPU_API(D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN, 0);}
IMAGE_TYPE                            ImageFormatToType(GPU_API(D3DFORMAT, DXGI_FORMAT, UInt) format
#if GL
, IMAGE_TYPE type
#endif
)
{
   // check these which are listed multiple times (then return the preferred version), or not listed at all but known
   switch(format)
   {
   #if DX9
      case D3DFMT_UNKNOWN: return IMAGE_NONE;
   #elif DX11
      case DXGI_FORMAT_UNKNOWN: return IMAGE_NONE;

      case DXGI_FORMAT_BC1_TYPELESS  :
      case DXGI_FORMAT_BC1_UNORM_SRGB: return IMAGE_BC1;

      case DXGI_FORMAT_BC2_TYPELESS  :
      case DXGI_FORMAT_BC2_UNORM_SRGB: return IMAGE_BC2;

      case DXGI_FORMAT_BC3_TYPELESS  :
      case DXGI_FORMAT_BC3_UNORM_SRGB: return IMAGE_BC3;

      case DXGI_FORMAT_BC7_TYPELESS  :
      case DXGI_FORMAT_BC7_UNORM_SRGB: return IMAGE_BC7;

      case DXGI_FORMAT_R16_TYPELESS  : return IMAGE_D16;
      case DXGI_FORMAT_R24G8_TYPELESS: return IMAGE_D24S8;
      case DXGI_FORMAT_R32_TYPELESS  : return IMAGE_D32;

      case DXGI_FORMAT_D24_UNORM_S8_UINT: return IMAGE_D24S8;
   #elif GL
      case 0: return IMAGE_NONE;

      #if GL_SWIZZLE
         case GL_R8 : if(type==IMAGE_A8 || type==IMAGE_L8)return type; return IMAGE_R8;
         case GL_RG8: if(type==IMAGE_L8A8                )return type; return IMAGE_R8G8;
      #endif
   #endif
   }
   FREPA(ImageTI)if(ImageTI[i].format==format)return IMAGE_TYPE(i);
   return IMAGE_NONE;
}
/******************************************************************************/
IMAGE_TYPE ImageTypeIncludeAlpha(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_R8G8B8:
      case IMAGE_R8G8  :
      case IMAGE_R8    : return IMAGE_R8G8B8A8;

      case IMAGE_L8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32: return IMAGE_L8A8;

      case IMAGE_BC1: return IMAGE_BC7; // BC1 has only 1-bit alpha which is not enough

      case IMAGE_F16  :
      case IMAGE_F16_2:
      case IMAGE_F16_3: return IMAGE_F16_4;

      case IMAGE_F32  :
      case IMAGE_F32_2:
      case IMAGE_F32_3: return IMAGE_F32_4;

      case IMAGE_ETC1   :
      case IMAGE_ETC2   :
      case IMAGE_ETC2_A1: return IMAGE_ETC2_A8; // ETC2_A1 has only 1-bit alpha which is not enough
   }
}
IMAGE_TYPE ImageTypeExcludeAlpha(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_R8G8B8A8: return IMAGE_R8G8B8;
      case IMAGE_B8G8R8A8: return IMAGE_B8G8R8;

      case IMAGE_L8A8: return IMAGE_L8;

      case IMAGE_BC2:
      case IMAGE_BC3:
      case IMAGE_BC7: return IMAGE_BC1;

      case IMAGE_F16_4: return IMAGE_F16_3;
      case IMAGE_F32_4: return IMAGE_F32_3;

      case IMAGE_ETC2_A1:
      case IMAGE_ETC2_A8: return IMAGE_ETC2;
   }
}
/******************************************************************************/
#if GL
UInt SourceGLFormat(IMAGE_TYPE type)
{
   switch(type)
   {
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32: return GL_LUMINANCE;

#if GL_SWIZZLE
      case IMAGE_A8  : return GL_RED;
      case IMAGE_L8  : return GL_RED;
      case IMAGE_L8A8: return GL_RG;
#else
      case IMAGE_A8  : return GL_ALPHA;
      case IMAGE_L8  : return GL_LUMINANCE;
      case IMAGE_L8A8: return GL_LUMINANCE_ALPHA;
#endif

      case IMAGE_F16    :
      case IMAGE_F32    :
      case IMAGE_R8     :
      case IMAGE_R8_SIGN: return GL_RED;

      case IMAGE_F16_2    :
      case IMAGE_F32_2    :
      case IMAGE_R8G8     :
      case IMAGE_R8G8_SIGN: return GL_RG;

      case IMAGE_F16_3   :
      case IMAGE_F32_3   :
      case IMAGE_R8G8B8  :
      case IMAGE_R8G8B8X8: return GL_RGB;

      case IMAGE_F16_4        :
      case IMAGE_F32_4        :
      case IMAGE_R8G8B8A8     :
      case IMAGE_R8G8B8A8_SIGN:
      case IMAGE_R10G10B10A2  : return GL_RGBA;

      case IMAGE_B4G4R4X4:
      case IMAGE_B5G5R5X1:
      case IMAGE_B5G6R5  :
      case IMAGE_B8G8R8  :
      case IMAGE_B8G8R8X8: return GL_BGR;

      case IMAGE_B4G4R4A4:
      case IMAGE_B5G5R5A1:
      case IMAGE_B8G8R8A8: return GL_BGRA;

      case IMAGE_D24S8: return GL_DEPTH_STENCIL;

      case IMAGE_D16  :
      case IMAGE_D24X8:
      case IMAGE_D32  : return GL_DEPTH_COMPONENT;

      default: return 0;
   }
}
/******************************************************************************/
UInt SourceGLType(IMAGE_TYPE type)
{
   switch(type)
   {
      case IMAGE_F16  :
      case IMAGE_F16_2:
      case IMAGE_F16_3:
      case IMAGE_F16_4: return D.shaderModelGLES2() ? GL_HALF_FLOAT_OES : GL_HALF_FLOAT; // GLES2 requires GL_HALF_FLOAT_OES (this was tested on WebGL1)

      case IMAGE_F32  :
      case IMAGE_F32_2:
      case IMAGE_F32_3:
      case IMAGE_F32_4: return GL_FLOAT;

      case IMAGE_D16  : return GL_UNSIGNED_SHORT;
      case IMAGE_D24S8: return GL_UNSIGNED_INT_24_8;
      case IMAGE_D24X8: return GL_UNSIGNED_INT;
      case IMAGE_D32  : return GL_FLOAT;

      case IMAGE_I8 : return GL_UNSIGNED_BYTE ;
      case IMAGE_I16: return GL_UNSIGNED_SHORT;
      case IMAGE_I32: return GL_UNSIGNED_INT  ;

      case IMAGE_R10G10B10A2: return GL_UNSIGNED_INT_2_10_10_10_REV;

      case IMAGE_B4G4R4X4:
      case IMAGE_B4G4R4A4: return GL_UNSIGNED_SHORT_4_4_4_4;

      case IMAGE_B5G5R5X1:
      case IMAGE_B5G5R5A1: return GL_UNSIGNED_SHORT_5_5_5_1;

      case IMAGE_B5G6R5: return GL_UNSIGNED_SHORT_5_6_5;

      case IMAGE_R8G8B8A8_SIGN:
      case IMAGE_R8G8_SIGN    :
      case IMAGE_R8_SIGN      : return GL_BYTE;

      case IMAGE_B8G8R8A8:
      case IMAGE_R8G8B8A8:
      case IMAGE_R8G8B8  :
      case IMAGE_R8G8    :
      case IMAGE_R8      :
      case IMAGE_A8      :
      case IMAGE_L8      :
      case IMAGE_L8A8    :
      case IMAGE_B8G8R8  :
      case IMAGE_B8G8R8X8:
      case IMAGE_R8G8B8X8: return GL_UNSIGNED_BYTE;

      default: return 0;
   }
}
#endif
/******************************************************************************/
// MANAGE
/******************************************************************************/
Image& Image::operator=(C Image &src                                                       )           {if(this!=&src)src.copy(T); return T;}
       Image::Image    (                                                                   )           {zero();}
       Image::Image    (C Image &src                                                       ) : Image() {src.copy(T);}
       Image::Image    (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps) : Image() {create(w, h, d, type, mode, mip_maps);}
/******************************************************************************/
Image& Image::del()
{
   unlock();
   if(D.created())
   {
      if(is() && hw()) // remove image from 'ShaderImages'
      {
         ShaderImages.lock  (); REPA(ShaderImages){ShaderImage &image=ShaderImages.lockedData(i); if(image.get()==this)image.set(null);}
         ShaderImages.unlock();
      }
   #if DX9
      if(/*_base || */_surf || _txtr || _cube || _vol)
      {
         D.texClear(_base);
         SyncLocker locker(D._lock);
         if(D.created())
         {
          //RELEASE(_base); don't release '_base' because it's just a copy of one of below
            RELEASE(_surf);
            RELEASE(_txtr);
            RELEASE(_cube);
            RELEASE(_vol );
         }
      }
   #elif DX11
      if(_txtr || _vol || _srv || _rtv || _dsv || _rdsv)
      {
         D.texClear(_srv);
       //SyncLocker locker(D._lock); lock not needed for DX11 'Release'
         if(D.created())
         {
            // release children first
            RELEASE(_rdsv);
            RELEASE(_dsv );
            RELEASE(_rtv );
            RELEASE(_srv );
            // now main resources
            RELEASE(_txtr);
            RELEASE(_vol );
         }
      }
   #elif GL
      if(_txtr || _rb)
      {
         D.texClear(_txtr);
      #if GL_LOCK
         SyncLocker locker(D._lock);
      #endif
         if(D.created())
         {
            glDeleteTextures     (1, &_txtr); _txtr=0; // clear while in lock
            glDeleteRenderbuffers(1, &_rb  ); _rb  =0; // clear while in lock
         }
      }
   #endif
   }
   Free(_data_all);
   zero(); return T;
}
/******************************************************************************
void Image::duplicate(C Image &src)
{
   if(this!=&src)
   {
      del();

   #if DX9
      if(_surf=src._surf)_surf->AddRef();
      if(_txtr=src._txtr)_txtr->AddRef();
      if(_vol =src._vol )_vol ->AddRef();
      if(_cube=src._cube)_cube->AddRef();
         _base=src._base; // don't 'AddRef' '_base' because it's just a copy of one of above
   #elif DX11
      if(_txtr=src._txtr)_txtr->AddRef();
      if(_vol =src._vol )_vol ->AddRef();
      if(_srv =src._srv )_srv ->AddRef();
      if(_rtv =src._rtv )_rtv ->AddRef();
      if(_dsv =src._dsv )_dsv ->AddRef();
      if(_rdsv=src._rdsv)_rdsv->AddRef();
   #elif GL
      _dup =true     ;
      _txtr=src._txtr;
      _rb  =src._rb  ;
      _w_s =src._w_s ;
      _w_t =src._w_t ;
      _w_r =src._w_r ;
   #endif

     _type   =src._type   ;
     _hw_type=src._hw_type;
     _mode   =src._mode   ;
     _mms    =src._mms    ;
     _samples=src._samples;
     _byte_pp=src._byte_pp;
     _partial=src._partial;
     _part   =src._part   ;
     _pitch  =src._pitch  ;
     _pitch2 =src._pitch2 ;
        _size=src.   _size;
     _hw_size=src._hw_size;

   //_lmm, _lcf, _lock_mode, _lock_count, _lock_size, _discard, _data, _data_all - ignored
      this may break because of _data, _data_all
   }
}
/******************************************************************************/
void Image::setPartial()
{
   if(_partial=(w()!=hwW() || h()!=hwH() || d()!=hwD()))_part.set(Flt(w())/hwW(), Flt(h())/hwH(), Flt(d())/hwD());
   else                                                 _part=1;
}
Image& Image::setInfo(Int w, Int h, Int d, Int type, IMAGE_MODE mode)
{
#if DX9
   D3DSURFACE_DESC desc;
   if(_txtr && !_surf)_txtr->GetSurfaceLevel(0, &_surf);

   if((_surf && OK(_surf->GetDesc     (   &desc)))
   || (_txtr && OK(_txtr->GetLevelDesc(0, &desc))))
   {
     _hw_size.x=desc.Width;
     _hw_size.y=desc.Height;
     _hw_size.z=1;
     _hw_type  =ImageFormatToType(desc.Format);
     _samples  =((desc.MultiSampleType==D3DMULTISAMPLE_NONE) ? 1 : desc.MultiSampleType);
   }else
   if(_cube && OK(_cube->GetLevelDesc(0, &desc)))
   {
     _hw_size.x=desc.Width;
     _hw_size.y=desc.Height;
     _hw_size.z=1;
     _hw_type  =ImageFormatToType(desc.Format);
     _samples  =((desc.MultiSampleType==D3DMULTISAMPLE_NONE) ? 1 : desc.MultiSampleType);
   }else
   if(_vol)
   {
      D3DVOLUME_DESC desc;
      if(OK(_vol->GetLevelDesc(0, &desc)))
      {
        _hw_size.x=desc.Width;
        _hw_size.y=desc.Height;
        _hw_size.z=desc.Depth;
        _hw_type  =ImageFormatToType(desc.Format);
        _samples  =1;
      }
   }

   if(_txtr)T._base=_txtr;else
   if(_cube)T._base=_cube;else
   if(_vol )T._base=_vol ;else
            T._base= null;

   if(_base)T._mms=_base->GetLevelCount();
#elif DX11
   // lock not needed for DX11 'D3D'
   if(_txtr)
   {
      D3D11_TEXTURE2D_DESC desc; _txtr->GetDesc(&desc);
     _mms      =desc.MipLevels;
     _samples  =desc.SampleDesc.Count;
     _hw_size.x=desc.Width;
     _hw_size.y=desc.Height;
     _hw_size.z=1;
     _hw_type  =ImageFormatToType(desc.Format);

      if(desc.Format==DXGI_FORMAT_R16_TYPELESS)
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC srvd; Zero(srvd); srvd.Format=DXGI_FORMAT_R16_UNORM;
         if(desc.SampleDesc.Count<=1){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D  ; srvd.Texture2D.MipLevels=1;}
         else                        {srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2DMS;}
         D3D->CreateShaderResourceView(_txtr, &srvd, &_srv);

         D3D11_DEPTH_STENCIL_VIEW_DESC dsvd; Zero(dsvd); dsvd.Format=DXGI_FORMAT_D16_UNORM; dsvd.ViewDimension=((desc.SampleDesc.Count>1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
                                               D3D->CreateDepthStencilView(_txtr, &dsvd, &_dsv );
         dsvd.Flags=D3D11_DSV_READ_ONLY_DEPTH; D3D->CreateDepthStencilView(_txtr, &dsvd, &_rdsv); // D16 does not have stencil, this will work only on DX11.0 but not 10.0, 10.1
      }else
      if(desc.Format==DXGI_FORMAT_R32_TYPELESS)
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC srvd; Zero(srvd); srvd.Format=DXGI_FORMAT_R32_FLOAT;
         if(desc.SampleDesc.Count<=1){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D  ; srvd.Texture2D.MipLevels=1;}
         else                        {srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2DMS;}
         D3D->CreateShaderResourceView(_txtr, &srvd, &_srv);

         D3D11_DEPTH_STENCIL_VIEW_DESC dsvd; Zero(dsvd); dsvd.Format=DXGI_FORMAT_D32_FLOAT; dsvd.ViewDimension=((desc.SampleDesc.Count>1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
                                               D3D->CreateDepthStencilView(_txtr, &dsvd, &_dsv );
         dsvd.Flags=D3D11_DSV_READ_ONLY_DEPTH; D3D->CreateDepthStencilView(_txtr, &dsvd, &_rdsv); // D32 does not have stencil, this will work only on DX11.0 but not 10.0, 10.1
      }else
      if(desc.Format==DXGI_FORMAT_R24G8_TYPELESS)
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC srvd; Zero(srvd); srvd.Format=DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
         if(desc.SampleDesc.Count<=1){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D  ; srvd.Texture2D.MipLevels=1;}
         else                        {srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2DMS;}
         D3D->CreateShaderResourceView(_txtr, &srvd, &_srv);

         D3D11_DEPTH_STENCIL_VIEW_DESC dsvd; Zero(dsvd); dsvd.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; dsvd.ViewDimension=((desc.SampleDesc.Count>1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
                                                                             D3D->CreateDepthStencilView(_txtr, &dsvd, &_dsv );
         dsvd.Flags=(D3D11_DSV_READ_ONLY_DEPTH|D3D11_DSV_READ_ONLY_STENCIL); D3D->CreateDepthStencilView(_txtr, &dsvd, &_rdsv); // this will work only on DX11.0 but not 10.0, 10.1
      }else
      {
         if(mode==IMAGE_2D || mode==IMAGE_3D || mode==IMAGE_CUBE || mode==IMAGE_RT || mode==IMAGE_RT_CUBE || mode==IMAGE_DS || mode==IMAGE_DS_RT || mode==IMAGE_SHADOW_MAP)D3D->CreateShaderResourceView(_txtr, null, &_srv);
         if(                  mode==IMAGE_RT    || mode==IMAGE_RT_CUBE   )D3D->CreateRenderTargetView(_txtr, null, &_rtv);
         if(mode==IMAGE_DS || mode==IMAGE_DS_RT || mode==IMAGE_SHADOW_MAP)D3D->CreateDepthStencilView(_txtr, null, &_dsv);
      }
   }else
   if(_vol)
   {
      D3D11_TEXTURE3D_DESC desc; _vol->GetDesc(&desc);
     _mms      =desc.MipLevels;
     _samples  =1;
     _hw_size.x=desc.Width;
     _hw_size.y=desc.Height;
     _hw_size.z=desc.Depth;
     _hw_type  =ImageFormatToType(desc.Format);
      D3D->CreateShaderResourceView(_vol, null, &_srv);
   }
#elif GL
   if(_txtr)switch(mode)
   {
      case IMAGE_2D          :
      case IMAGE_RT          :
      case IMAGE_SURF_SCRATCH:
      case IMAGE_SURF_SYSTEM :
      case IMAGE_SURF        :
      case IMAGE_DS          :
      case IMAGE_DS_RT       :
      case IMAGE_SHADOW_MAP  :
      {
      #if !GL_ES // texture info is unavailable on OpenGL ES, so just trust in what we've set
         Int format;
         D.texBind               (GL_TEXTURE_2D, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
        _hw_size.z=1;
      }break;

      case IMAGE_3D:
      {
      #if !GL_ES // texture info is unavailable on OpenGL ES, so just trust in what we've set
         Int format;
         D.texBind               (GL_TEXTURE_3D, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH          , &_hw_size.z);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
      }break;

      case IMAGE_CUBE:
      case IMAGE_RT_CUBE:
      {
      #if !GL_ES // texture info is unavailable on OpenGL ES, so just trust in what we've set
         Int format;
         D.texBind               (GL_TEXTURE_CUBE_MAP, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
        _hw_size.z=1;
      }break;
   }else
   if(_rb)
   {
      Int format;
      glBindRenderbuffer          (GL_RENDERBUFFER, _rb);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH          , &_hw_size.x);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT         , &_hw_size.y);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, & format   );
     _hw_type  =ImageFormatToType(format, hwType());
     _hw_size.z=1;
   }
#endif

  _size.x =          ((w   >0) ? w    : hwW   ());
  _size.y =          ((h   >0) ? h    : hwH   ());
  _size.z =          ((d   >0) ? d    : hwD   ());
  _type   =IMAGE_TYPE((type>0) ? type : hwType());
  _mode   =                      mode            ;
  _byte_pp=ImageTI[hwType()].byte_pp; // keep a copy for faster access
   if(is()){MAX(_mms, 1); MAX(_samples, 1);}
   setPartial();
   return T;
}
Image& Image::forceInfo(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode)
{
  _hw_size.x=w;
  _hw_size.y=h;
  _hw_size.z=d;
  _hw_type  =type;
   return setInfo(w, h, d, type, mode);
}
/******************************************************************************/
void Image::setGLParams()
{
#if GL
#if GL_LOCK
   SyncLocker locker(D._lock);
#endif
   if(D.created() && _txtr)
   {
      Bool mip_maps=(mipMaps()>1), filterable=true;
      if(D.shaderModelGLES2() && mip_maps && mipMaps()!=TotalMipMaps(w(), h(), d(), hwType()))mip_maps=false; // GLES2 requires full chain of mip-maps
   #if GL_ES
      filterable=(ImageTI[hwType()].precision<IMAGE_PRECISION_32); // GLES2/3 don't support filtering F32 textures, without this check reading from F32 textures will fail - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
   #endif
      UInt target;
      switch(mode())
      {
         case IMAGE_2D          :
         case IMAGE_RT          :
         case IMAGE_SURF_SCRATCH:
         case IMAGE_SURF_SYSTEM :
         case IMAGE_SURF        :
         {
            target=GL_TEXTURE_2D;
         #if GL_ES
            if(mip_maps && ForceDisableMipMaps(T))mip_maps=false;
         #endif
         }break;

         case IMAGE_3D: target=GL_TEXTURE_3D; break;

         case IMAGE_CUBE:
         case IMAGE_RT_CUBE: target=GL_TEXTURE_CUBE_MAP; break;

         default: return;
      }
      D.texBind(target, _txtr);

      // first call those that can generate GL ERROR and we're OK with that, so we will call 'glGetError' to clear them
                  {glTexParameteri(target, GL_TEXTURE_MAX_LEVEL     , mip_maps ? mipMaps()-1 : 0); glGetError();} // clear error in case GL_TEXTURE_MAX_LEVEL is not supported (can happen on GLES2)
      if(mip_maps){glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY,      Max(D.texFilter(), 1)); glGetError();} // clear error in case anisotropy is not supported (can happen when using ETC2 on "Galaxy Note 4")

      // now call thost that must succeed
                   glTexParameteri(target, GL_TEXTURE_MIN_FILTER    , mip_maps ? (filterable ? D.texMipFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST) : filterable ? GL_LINEAR : GL_NEAREST);
                   glTexParameteri(target, GL_TEXTURE_MAG_FILTER    , (filterable && (D.texFilter() || mode()!=IMAGE_2D)) ? GL_LINEAR : GL_NEAREST);
   #if GL_SWIZZLE
      switch(hwType())
      {
         case IMAGE_A8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_RED );
         }break;

         case IMAGE_L8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
         }break;

         case IMAGE_L8A8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
         }break;
      }
   #endif
   }
#endif
}
/******************************************************************************/
void Image::setGLFont()
{
#if GL && defined GL_TEXTURE_LOD_BIAS
#if GL_LOCK
   SyncLocker locker(D._lock);
#endif
   if(D.created() && _txtr && mipMaps()>1)switch(mode())
   {
      case IMAGE_2D          :
      case IMAGE_RT          :
      case IMAGE_SURF_SCRATCH:
      case IMAGE_SURF_SYSTEM :
      case IMAGE_SURF        :
      {
         D.texBind      (GL_TEXTURE_2D, _txtr);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -D.fontSharpness());
         glFlush        (); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
      }break;
   }
#endif
}
/******************************************************************************/
Bool Image::createTryEx(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Byte samples, IMAGE_TYPE type_on_fail, C Image *src)
{
   // verify parameters
   if(w<=0 || h<=0 || d<=0 || type==IMAGE_NONE){del(); return !w && !h && !d;}

   if((d!=1 && mode!=IMAGE_SOFT && mode!=IMAGE_3D) // "d!=1" can be specified only for SOFT or 3D
#if DX9
   || samples>16 // DX9 does not support more than 16 samples
#endif
   || !InRange(type, IMAGE_ALL_TYPES))goto error; // type out of range

   if(type_on_fail==type || !InRange(type_on_fail, IMAGE_ALL_TYPES))type_on_fail=IMAGE_NONE;
   MAX(samples, 1);

   {
      // mip maps
      Int      total_mip_maps=TotalMipMaps(w, h, d, type); // don't use hardware texture size hwW(), hwH(), hwD(), so that number of mip-maps will always be the same (and not dependant on hardware capabilities like TexPow2 sizes), also because 1x1 image has just 1 mip map, but if we use padding then 4x4 block would generate 3 mip maps
      if(mip_maps<=0)mip_maps=total_mip_maps ; // if mip maps not specified (or we want multiple mip maps with type that requires full chain) then use full chain
      else       MIN(mip_maps,total_mip_maps); // don't use more than maximum allowed

      // check if already matches what we want
      if(T.w()==w && T.h()==h && T.d()==d && T.type()==type && T.mode()==mode && T.mipMaps()==mip_maps && T.samples()==samples && !src)
      {
         unlock(); // unlock if was locked
         return true;
      }

      // create as new
   #if GL
      Memt<Byte> temp;
   #endif
      // hardware size (do after calculating mip-maps)
      VecI  hw_size        (PaddedWidth(w, h, 0, type        ), PaddedHeight(w, h, 0, type        ), d);
      VecI2 hw_size_on_fail(PaddedWidth(w, h, 0, type_on_fail), PaddedHeight(w, h, 0, type_on_fail)   );
      if(TotalMipMaps(hw_size_on_fail.x, hw_size_on_fail.y, d, type_on_fail)<mip_maps) // if the secondary type can't fit all required mip maps, then we need to increase it, this is needed in case the desired 'type' size is bumped up due to its requirements (like PVRTC) thus making its total number of mip maps bigger
         hw_size_on_fail.set(PaddedWidth (hw_size.x, hw_size.y, 0, type_on_fail),
                             PaddedHeight(hw_size.x, hw_size.y, 0, type_on_fail));
      Bool pow2=false;
      switch(mode)
      {
         case IMAGE_2D: switch(D._tex_pow2)
         {
            case 2: pow2=true; break; // require always
            case 1:                   // conditional
            {
            #if !GL // on OpenGL we will create a non-pow2 texture, but we won't upload mip maps
               pow2=(mip_maps>1); // force pow2 size if we have mip maps
            #endif
            }break;
         }break;
         case IMAGE_3D  : pow2=D._tex_pow2_3d  ; break;
         case IMAGE_CUBE: pow2=D._tex_pow2_cube; break;
      }
      if(pow2)
      {
         hw_size.x=CeilPow2(hw_size.x); hw_size_on_fail.x=CeilPow2(hw_size_on_fail.x);
         hw_size.y=CeilPow2(hw_size.y); hw_size_on_fail.y=CeilPow2(hw_size_on_fail.y);
         hw_size.z=CeilPow2(hw_size.z);
      }
   #if DX11
      D3D11_SUBRESOURCE_DATA *initial_data=null; MemtN<D3D11_SUBRESOURCE_DATA, 32*6> res_data; // 32 mip maps * 6 faces
   #endif
      if(src) // if 'src' specified
      {
      #if DX11 || GL // !! if adding more API's here, then allow them in 'Load' "Image IO.cpp" too !! only DX11, GL support this
         if(src==this // can't be created from self
         || src->hwType()!=type || src->hwSize3()!=hw_size || src->mipMaps()!=mip_maps || !src->soft() || src->cube()!=IsCube(mode)) // 'src' must match exactly what was requested
      #endif
            goto error;

         type_on_fail=IMAGE_NONE; // no alternative types
      #if DX11
       C Byte *data=src->softData(); Int faces=src->faces();
         initial_data=res_data.setNum(mip_maps*faces).data();
         FREPD(m, mip_maps)
         { // have to specify HW sizes, because all images (HW and SOFT) are stored like that
            Int mip_pitch =ImagePitch  (src->hwW(), src->hwH(), m, src->hwType())           , // X
                mip_pitch2=ImageBlocksY(src->hwW(), src->hwH(), m, src->hwType())*mip_pitch , // X*Y
                mip_size  =                  Max(1, src->hwD()>>m               )*mip_pitch2; // X*Y*Z
            FREPD(f, faces)
            {
               D3D11_SUBRESOURCE_DATA &srd=initial_data[D3D11CalcSubresource(m, f, mip_maps)];
               srd.pSysMem         =data;
               srd.SysMemPitch     =mip_pitch;
               srd.SysMemSlicePitch=mip_pitch2;
               data+=mip_size;
            }
         }
      #endif
      }

   #if DX9 || GL_LOCK
      SyncLockerEx locker(D._lock, IsHW(mode)); // lock not needed for DX11 'D3D'
   #endif
      if(IsHW(mode) && !D.created())goto error; // device not yet created

      del();

      // create
     _hw_size=hw_size;
      switch(mode)
      {
         case IMAGE_SOFT:
         case IMAGE_SOFT_CUBE:
         {
           _hw_type=type; // set before 'setInfo' because it affects 'byte_pp'
           _mms    =mip_maps;
            setInfo(w, h, d, type, mode);
            Alloc(_data_all, memUsage());
            lockSoft(); // set default lock members to main mip map
         }return true;

      #if DX9
         case IMAGE_SURF_SCRATCH:                         if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type        ].format, D3DPOOL_SCRATCH                              , &_surf, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type_on_fail].format, D3DPOOL_SCRATCH                              , &_surf, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_SURF_SYSTEM :                         if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type        ].format, D3DPOOL_SYSTEMMEM                            , &_surf, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type_on_fail].format, D3DPOOL_SYSTEMMEM                            , &_surf, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_SURF        :                         if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type        ].format, D3DPOOL_DEFAULT                              , &_surf, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateOffscreenPlainSurface(hwW(), hwH(),                                         ImageTI[type_on_fail].format, D3DPOOL_DEFAULT                              , &_surf, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_2D          :                         if(samples==1 && OK(D3D->CreateTexture              (hwW(), hwH(),        mip_maps, 0                    , ImageTI[type        ].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_txtr, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateTexture              (hwW(), hwH(),        mip_maps, 0                    , ImageTI[type_on_fail].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_txtr, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_3D          :                         if(samples==1 && OK(D3D->CreateVolumeTexture        (hwW(), hwH(), hwD(), mip_maps, 0                    , ImageTI[type        ].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_vol , null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateVolumeTexture        (hwW(), hwH(), hwD(), mip_maps, 0                    , ImageTI[type_on_fail].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_vol , null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_CUBE        :                         if(samples==1 && OK(D3D->CreateCubeTexture          (hwW(),               mip_maps, 0                    , ImageTI[type        ].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_cube, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateCubeTexture          (hwW(),               mip_maps, 0                    , ImageTI[type_on_fail].format, D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, &_cube, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_RT_CUBE     :                         if(samples==1 && OK(D3D->CreateCubeTexture          (hwW(),                      1, D3DUSAGE_RENDERTARGET, ImageTI[type        ].format, D3DPOOL_DEFAULT                              , &_cube, null))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(samples==1 && OK(D3D->CreateCubeTexture          (hwW(),                      1, D3DUSAGE_RENDERTARGET, ImageTI[type_on_fail].format, D3DPOOL_DEFAULT                              , &_cube, null))){setInfo(w, h, d, type, mode); return true;}} break;
         case IMAGE_DS_RT       :                         if(samples==1 && OK(D3D->CreateTexture              (hwW(), hwH(),               1, D3DUSAGE_DEPTHSTENCIL, ImageTI[type        ].format, D3DPOOL_DEFAULT                              , &_txtr, null))){setInfo(w, h, d, type, mode); return true;}  break;
         case IMAGE_SHADOW_MAP  :                         if(samples==1 && OK(D3D->CreateTexture              (hwW(), hwH(),               1, D3DUSAGE_DEPTHSTENCIL, ImageTI[type        ].format, D3DPOOL_DEFAULT                              , &_txtr, null))){setInfo(w, h, d, type, mode); return true;}  break;

         case IMAGE_DS:
            if(OK(D3D->CreateDepthStencilSurface(hwW(), hwH(), ImageTI[type].format, (samples>1) ? D3DMULTISAMPLE_TYPE(samples) : D3DMULTISAMPLE_NONE, 0, false, &_surf, null))){setInfo(w, h, d, type, mode); return true;}
         break;

         case IMAGE_RT:
            if(samples>1)
            {
                                                             if(OK(D3D->CreateRenderTarget(hwW(), hwH(), ImageTI[type        ].format, D3DMULTISAMPLE_TYPE(samples), 0, false, &_surf, null))){setInfo(w, h, d, type, mode); clearHw(); return true;}
               if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(OK(D3D->CreateRenderTarget(hwW(), hwH(), ImageTI[type_on_fail].format, D3DMULTISAMPLE_TYPE(samples), 0, false, &_surf, null))){setInfo(w, h, d, type, mode); clearHw(); return true;}}
            }else
            {
                                                             if(OK(D3D->CreateTexture(hwW(), hwH(), 1, D3DUSAGE_RENDERTARGET, ImageTI[type        ].format, D3DPOOL_DEFAULT, &_txtr, null))){setInfo(w, h, d, type, mode); clearHw(); return true;}
               if(type_on_fail){_hw_size.xy=hw_size_on_fail; if(OK(D3D->CreateTexture(hwW(), hwH(), 1, D3DUSAGE_RENDERTARGET, ImageTI[type_on_fail].format, D3DPOOL_DEFAULT, &_txtr, null))){setInfo(w, h, d, type, mode); clearHw(); return true;}}
            }
         break;
      #elif DX11
         case IMAGE_2D:
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =0;
            desc.CPUAccessFlags    =0;
            desc.SampleDesc.Count  =1;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =1;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, initial_data, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            }
         }break;

         case IMAGE_SURF_SCRATCH:
         case IMAGE_SURF_SYSTEM :
         case IMAGE_SURF        :
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_STAGING;
            desc.BindFlags         =0;
            desc.MiscFlags         =0;
            desc.CPUAccessFlags    =D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
            desc.SampleDesc.Count  =1;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =1;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, initial_data, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            }
         }break;

         case IMAGE_RT:
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =0;
            desc.CPUAccessFlags    =0;
            desc.SampleDesc.Count  =samples;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =1;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); SyncLocker locker(D._lock); clearHw(); return true;} // 'clearHw' needs lock, clear render targets to zero at start (especially important for floating point RT's), use 'clearHW' instead of 'initial_data' because that would require large memory allocations
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); SyncLocker locker(D._lock); clearHw(); return true;} // 'clearHw' needs lock, clear render targets to zero at start (especially important for floating point RT's), use 'clearHW' instead of 'initial_data' because that would require large memory allocations
            }
         }break;

         case IMAGE_3D:
         {
            D3D11_TEXTURE3D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.Depth             =hwD();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =0;
            desc.CPUAccessFlags    =0;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture3D(&desc, initial_data, &_vol))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture3D(&desc, null, &_vol))){setInfo(w, h, d, type, mode); return true;}
            }
         }break;

         case IMAGE_CUBE:
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =D3D11_RESOURCE_MISC_TEXTURECUBE;
            desc.CPUAccessFlags    =0;
            desc.SampleDesc.Count  =1;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =6;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, initial_data, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            }
         }break;

         case IMAGE_RT_CUBE:
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =D3D11_RESOURCE_MISC_TEXTURECUBE;
            desc.CPUAccessFlags    =0;
            desc.SampleDesc.Count  =samples;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =6;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); SyncLocker locker(D._lock); clearHw(); return true;} // 'clearHw' needs lock, clear render targets to zero at start (especially important for floating point RT's), use 'clearHW' instead of 'initial_data' because that would require large memory allocations
            if(type_on_fail)
            {
               desc.Width =_hw_size.x=hw_size_on_fail.x;
               desc.Height=_hw_size.y=hw_size_on_fail.y;
               desc.Format=ImageTI[type_on_fail].format;
               if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); SyncLocker locker(D._lock); clearHw(); return true;} // 'clearHw' needs lock, clear render targets to zero at start (especially important for floating point RT's), use 'clearHW' instead of 'initial_data' because that would require large memory allocations
            }
         }break;

         case IMAGE_DS:
         case IMAGE_DS_RT:
         case IMAGE_SHADOW_MAP:
         {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width             =hwW();
            desc.Height            =hwH();
            desc.MipLevels         =mip_maps;
            desc.Format            =ImageTI[type].format; if(desc.Format==DXGI_FORMAT_D24_UNORM_S8_UINT)desc.Format=DXGI_FORMAT_R24G8_TYPELESS;else if(desc.Format==DXGI_FORMAT_D32_FLOAT)desc.Format=DXGI_FORMAT_R32_TYPELESS;else if(desc.Format==DXGI_FORMAT_D16_UNORM)desc.Format=DXGI_FORMAT_R16_TYPELESS;
            desc.Usage             =D3D11_USAGE_DEFAULT;
            desc.BindFlags         =D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags         =0;
            desc.CPUAccessFlags    =0;
            desc.SampleDesc.Count  =samples;
            desc.SampleDesc.Quality=0;
            desc.ArraySize         =1;
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); return true;}
            FlagDisable(desc.BindFlags, D3D11_BIND_SHADER_RESOURCE); // disable shader reading
            if(desc.Format!=DXGI_FORMAT_UNKNOWN && OK(D3D->CreateTexture2D(&desc, null, &_txtr))){setInfo(w, h, d, type, mode); return true;}
         }break;
      #elif GL
         case IMAGE_2D          :
         case IMAGE_RT          :
         case IMAGE_SURF_SCRATCH:
         case IMAGE_SURF_SYSTEM :
         case IMAGE_SURF        :
         {
            glGenTextures(1, &_txtr);
            if(_txtr)
            {
               T._mms    =mip_maps;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =1;
               T._mode   =mode;
               T._type   =T._hw_type=type;

               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               UInt format=ImageTI[hwType()].format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
               if(src)
               {
                C Byte *data=src->softData(); FREPD(m, mipMaps()) // order important
                  {
                     if(m==1) // check at the start of mip-map #1, to skip this when there's only one mip-map
                     {
                        if(ForceDisableMipMaps(T))break;
                        if(glGetError()!=GL_NO_ERROR)goto error; // if first mip failed, then skip remaining
                     }
                     VecI2 size(Max(1, hwW()>>m), Max(1, hwH()>>m));
                     Int   mip_size=ImageMipSize(size.x, size.y, 0, hwType());
                     if(!compressed())glTexImage2D(GL_TEXTURE_2D, m, format, size.x, size.y, 0, gl_format, gl_type, data);
                     else   glCompressedTexImage2D(GL_TEXTURE_2D, m, format, size.x, size.y, 0, mip_size, data);
                     data+=mip_size;
                  }
               }else
               if(!compressed())
               {
                  if(mode==IMAGE_RT)temp.setNumZero(CeilGL(memUsage())); // clear render targets to zero at start (especially important for floating point RT's)
                  glTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, gl_format, gl_type, temp.dataNull());
               }else
               {
                  Int mip_size=ImageMipSize(hwW(), hwH(), 0, hwType());
                  if(WEB)temp.setNumZero(CeilGL(mip_size)); // for WEB, null can't be specified in 'glCompressedTexImage'
                  glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, mip_size, temp.dataNull());
               }

               if(glGetError()!=GL_NO_ERROR) // error
               {
                  if(!type_on_fail)goto error;
                 _hw_size.xy=hw_size_on_fail;
                 _hw_type   =type_on_fail;
                  format    =ImageTI[hwType()].format;
                  if(!compressed())
                  {
                     if(mode==IMAGE_RT)temp.setNumZero(CeilGL(memUsage()));else temp.del(); // clear render targets to zero at start (especially important for floating point RT's)
                     glTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), temp.dataNull());
                  }else
                  {
                     Int mip_size=ImageMipSize(hwW(), hwH(), 0, hwType());
                     if(WEB)temp.setNumZero(CeilGL(mip_size));else temp.del(); // for WEB, null can't be specified in 'glCompressedTexImage'
                     glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, mip_size, temp.dataNull());
                  }
                  if(glGetError()!=GL_NO_ERROR)goto error; // error
               }

               glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               setInfo(w, h, d, type, mode);
            #if GL_ES
               if(mode!=IMAGE_RT)Alloc(_data_all, CeilGL(memUsage()));
            #endif
               return true;
            }
         }break;

         case IMAGE_3D: if(D.notShaderModelGLES2())
         {
            glGenTextures(1, &_txtr);
            if(_txtr)
            {
               T._mms    =mip_maps;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =d;
               T._mode   =mode;
               T._type   =T._hw_type=type;

               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               UInt format=ImageTI[hwType()].format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
               if(src)
               {
                C Byte *data=src->softData(); FREPD(m, mipMaps()) // order important
                  {
                     if(m==1) // check at the start of mip-map #1, to skip this when there's only one mip-map
                     {
                        if(glGetError()!=GL_NO_ERROR)goto error; // if first mip failed, then skip remaining
                     }
                     VecI size(Max(1, hwW()>>m), Max(1, hwH()>>m), Max(1, hwD()>>m));
                     Int  mip_size=ImageMipSize(size.x, size.y, size.z, 0, hwType());
                     if(!compressed())glTexImage3D(GL_TEXTURE_3D, m, format, size.x, size.y, size.z, 0, gl_format, gl_type, data);
                     else   glCompressedTexImage3D(GL_TEXTURE_3D, m, format, size.x, size.y, size.z, 0, mip_size, data);
                     data+=mip_size;
                  }
               }else
               if(!compressed())glTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, gl_format, gl_type, null);
               else   glCompressedTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, ImageMipSize(hwW(), hwH(), hwD(), 0, hwType()), null);

               if(glGetError()!=GL_NO_ERROR) // error
               {
                  if(!type_on_fail)goto error;
                 _hw_size.xy=hw_size_on_fail;
                 _hw_type   =type_on_fail;
                  format    =ImageTI[hwType()].format;
                  if(!compressed())glTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), null);
                  else   glCompressedTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, ImageMipSize(hwW(), hwH(), hwD(), 0, hwType()), null);
                  if(glGetError()!=GL_NO_ERROR)goto error; // error
               }

               glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               setInfo(w, h, d, type, mode);
            #if GL_ES
            	Alloc(_data_all, CeilGL(memUsage()));
            #endif
            	return true;
            }
         }break;

         case IMAGE_CUBE:
         case IMAGE_RT_CUBE:
         {
            glGenTextures(1, &_txtr);
            if(_txtr)
            {
               T._mms    =mip_maps;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =1;
               T._mode   =mode;
               T._type   =T._hw_type=type;

               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               // set params which are set only at creation time, so they don't need to be set again later
               glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            again_cube:
               UInt format=ImageTI[hwType()].format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
             C Byte *data=(src ? src->softData() : null); Int mip_maps=(src ? mipMaps() : 1); FREPD(m, mip_maps) // order important
               {
                  VecI2 size(Max(1, hwW()>>m), Max(1, hwH()>>m));
                  Int   mip_size=ImageMipSize(size.x, size.y, 0, hwType());
                  if(!m && !src)
                     if(mode==IMAGE_RT_CUBE || WEB && compressed())data=temp.setNumZero(CeilGL(mip_size)).dataNull(); // clear render targets to zero at start (especially important for floating point RT's), for WEB null can't be specified in 'glCompressedTexImage'
                  FREPD(f, 6) // faces, order important
                  {
                     if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, m, format, size.x, size.y, 0, gl_format, gl_type, data);
                     else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, m, format, size.x, size.y, 0, mip_size, data);

                     if(!m && !f && glGetError()!=GL_NO_ERROR) // check for error only on the first mip and face
                     {
                        if(!type_on_fail)goto error;
                       _hw_size.xy=hw_size_on_fail;
                       _hw_type   =type_on_fail; type_on_fail=IMAGE_NONE; // disable trying this again
                        goto again_cube;
                     }
                     
                     if(src)data+=mip_size;
                  }
               }

               if(glGetError()==GL_NO_ERROR) // ok
               {
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  setInfo(w, h, d, type, mode);
               #if GL_ES
                  if(mode!=IMAGE_RT_CUBE)Alloc(_data_all, CeilGL(memUsage()));
               #endif
            	   return true;
               }
            }
         }break;

         case IMAGE_DS:
         {
            glGenRenderbuffers(1, &_rb);
            if(_rb)
            {
               T._mms    =1;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =1;
               T._mode   =mode;
               T._type   =T._hw_type=type;
             //LogN(S+"x:"+hwW()+", y:"+hwH()+", type:"+ImageTI[hwType()].name);

               glGetError(); // clear any previous errors

               glBindRenderbuffer   (GL_RENDERBUFFER, _rb);
               glRenderbufferStorage(GL_RENDERBUFFER, ImageTI[hwType()].format, hwW(), hwH());

               if(glGetError()==GL_NO_ERROR) // ok
               {
                  setInfo(w, h, d, type, mode);
                  return true;
               }
            }
         }break;

         case IMAGE_DS_RT:
         {
            glGenTextures(1, &_txtr);
            if(_txtr)
            {
               T._mms    =1;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =1;
               T._mode   =mode;
               T._type   =T._hw_type=type;

               glGetError(); // clear any previous errors

               D.texBind      (GL_TEXTURE_2D, _txtr);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              {glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL , 0); glGetError();} // clear error in case GL_TEXTURE_MAX_LEVEL is not supported (can happen on GLES2)

               glTexImage2D(GL_TEXTURE_2D, 0, ImageTI[hwType()].format, hwW(), hwH(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), null);

               if(glGetError()==GL_NO_ERROR) // ok
               {
                  setInfo(w, h, d, type, mode);
                  return true;
               }
            }
         }break;

         case IMAGE_SHADOW_MAP: if(!WEB || D.notShaderModelGLES2()) // WebGL succeeds to create shadow map on GLES2 while not really supporting it, because of that shaders that use shadows fail to load complaining about 'sampler2DShadow' is not supported, however it works on Android GLES2
         {
            glGenTextures(1, &_txtr);
            if(_txtr)
            {
               T._mms    =1;
               T._samples=1;
               T._size.x =w;
               T._size.y =h;
               T._size.z =1;
               T._mode   =mode;
               T._type   =T._hw_type=type;

               glGetError(); // clear any previous errors

               D.texBind      (GL_TEXTURE_2D, _txtr);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S      , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T      , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER  , GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER  , GL_LINEAR);
              {glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL   , 0); glGetError();} // clear error in case GL_TEXTURE_MAX_LEVEL is not supported (can happen on GLES2)
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, REVERSE_DEPTH ? GL_GEQUAL : GL_LEQUAL);

               glTexImage2D(GL_TEXTURE_2D, 0, ImageTI[hwType()].format, hwW(), hwH(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), null);

               if(glGetError()==GL_NO_ERROR) // ok
               {
                  setInfo(w, h, d, type, mode);
                  return true;
               }
            }
         }break;
      #endif
      }
   }
error:
   del(); return false;
}
Bool Image::createTry(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Bool rgba_on_fail)
{
   return createTryEx(w, h, d, type, mode, mip_maps, 1, rgba_on_fail ? IMAGE_DEFAULT : IMAGE_NONE);
}
Image& Image::create(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Bool rgba_on_fail)
{
   if(!createTry(w, h, d, type, mode, mip_maps, rgba_on_fail))Exit(MLT(S+"Can't create texture "       +w+'x'+h+'x'+d+", type "+ImageTI[type].name+", mode "+mode+".",
                                                                   PL,S+u"Nie można utworzyć tekstury "+w+'x'+h+'x'+d+", typ " +ImageTI[type].name+", tryb "+mode+"."));
   return T;
}
/******************************************************************************/
static Bool Decompress(C Image &src, Image &dest) // assumes that 'src' and 'dest' are 2 different objects, 'src' is compressed, and 'dest' not yet created
{
   void (*decompress_block)(C Byte *b, Color (&block)[4][4]), (*decompress_block_pitch)(C Byte *b, Color *dest, Int pitch);
   switch(src.hwType())
   {
      default: return false;

      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4: return DecompressPVRTC(src, dest);

      case IMAGE_BC1    : decompress_block=DecompressBlockBC1   ; decompress_block_pitch=DecompressBlockBC1   ; break;
      case IMAGE_BC2    : decompress_block=DecompressBlockBC2   ; decompress_block_pitch=DecompressBlockBC2   ; break;
      case IMAGE_BC3    : decompress_block=DecompressBlockBC3   ; decompress_block_pitch=DecompressBlockBC3   ; break;
      case IMAGE_BC7    : decompress_block=DecompressBlockBC7   ; decompress_block_pitch=DecompressBlockBC7   ; break;
      case IMAGE_ETC1   : decompress_block=DecompressBlockETC1  ; decompress_block_pitch=DecompressBlockETC1  ; break;
      case IMAGE_ETC2   : decompress_block=DecompressBlockETC2  ; decompress_block_pitch=DecompressBlockETC2  ; break;
      case IMAGE_ETC2_A1: decompress_block=DecompressBlockETC2A1; decompress_block_pitch=DecompressBlockETC2A1; break;
      case IMAGE_ETC2_A8: decompress_block=DecompressBlockETC2A8; decompress_block_pitch=DecompressBlockETC2A8; break;
   }
   Bool ok=false;
   if(dest.is() || dest.createTry(src.w(), src.h(), src.d(), IMAGE_R8G8B8A8, IMAGE_SOFT, 1)) // use 'IMAGE_R8G8B8A8' because Decompress Block functions operate on 'Color'
      if(dest.size3()==src.size3())
         if(dest.lock(LOCK_WRITE))
   {
      if(src.lockRead())
      {
         ok=true;

         Int full_blocks_x=         dest.w()/4,
             full_blocks_y=         dest.h()/4,
              all_blocks_x=DivCeil4(dest.w()),
              all_blocks_y=DivCeil4(dest.h()),
              x_mul       =ImageTI[src.hwType()].bit_pp*2; // *2 because (4*4 colors / 8 bits)
         REPD(z, dest.d())
         {
            Color color[4][4];
            Int   done_x=0, done_y=0;
            if(dest.hwType()==IMAGE_R8G8B8A8  // decompress directly into 'dest'
            && dest.  type()==IMAGE_R8G8B8A8) // check 'type' too in case we have to perform color adjustment
            {
               // process full blocks only
             C Byte * src_data_z= src.data() + z* src.pitch2();
               Byte *dest_data_z=dest.data() + z*dest.pitch2();
               REPD(by, full_blocks_y)
               {
                  const Int py=by*4; // pixel
                C Byte * src_data_y= src_data_z + by* src.pitch();
                  Byte *dest_data_y=dest_data_z + py*dest.pitch();
                  REPD(bx, full_blocks_x)
                  {
                     const Int px=bx*4; // pixel
                     decompress_block_pitch(src_data_y + bx*x_mul, (Color*)(dest_data_y + px*4), dest.pitch());
                  }
               }
               done_x=full_blocks_x;
               done_y=full_blocks_y;
            }

            // process right blocks (excluding the shared corner)
            for(Int by=     0; by<done_y      ; by++)
            for(Int bx=done_x; bx<all_blocks_x; bx++)
            {
               Int px=bx*4, py=by*4; // pixel
               decompress_block(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
               REPD(y, 4)
               REPD(x, 4)dest.color3D(px+x, py+y, z, color[y][x]);
            }

            // process bottom blocks (including the shared corner)
            for(Int by=done_y; by<all_blocks_y; by++)
            for(Int bx=     0; bx<all_blocks_x; bx++)
            {
               Int px=bx*4, py=by*4; // pixel
               decompress_block(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
               REPD(y, 4)
               REPD(x, 4)dest.color3D(px+x, py+y, z, color[y][x]);
            }
         }
         src.unlock();
      }
      dest.unlock();
   }
   return ok;
}
/******************************************************************************/
static Bool Compress(C Image &src, Image &dest, Bool mtrl_base_1=false) // assumes that 'src' and 'dest' are 2 different objects, 'src' is created as non-compressed, and 'dest' is created as compressed
{
   switch(dest.hwType())
   {
      case IMAGE_BC1:
      case IMAGE_BC2:
      case IMAGE_BC3: return               CompressBC (src, dest, mtrl_base_1);
      case IMAGE_BC7: return CompressBC7 ? CompressBC7(src, dest) : false;

      case IMAGE_ETC1   :
      case IMAGE_ETC2   :
      case IMAGE_ETC2_A1:
      case IMAGE_ETC2_A8: return CompressETC ? CompressETC(src, dest, -1, mtrl_base_1 ? false : true) : false;

      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4: return CompressPVRTC ? CompressPVRTC(src, dest, -1) : false;
   }
   return false;
}
/******************************************************************************/
static Bool CopyMipMaps(C Image &src, Image &dest) // this assumes that "&src != &dest"
{
   if(dest.hwType()==src.hwType()
   && dest.  type()==src.  type() // check 'type' too in case we have to perform color adjustment
   && dest.w()<=src.w()
   && dest.h()<=src.h()
   && dest.d()<=src.d())
      FREP(src.mipMaps()) // find location of first 'dest' mip-map in 'src'
   {
      Int src_mip_w=Max(1, src.w()>>i),
          src_mip_h=Max(1, src.h()>>i),
          src_mip_d=Max(1, src.d()>>i);
      if( src_mip_w==dest.w() && src_mip_h==dest.h() && src_mip_d==dest.d()) // i-th 'src' mip-map is the same as first 'dest' mip-map
      {
         if(src.mipMaps()-i>=dest.mipMaps()) // if 'src' has all mip-maps needed in 'dest'
         {
            Int faces=dest.faces();
            FREPD(mip , dest.mipMaps())
             REPD(face, faces)
            {
               if(!src .lockRead(          i+mip, DIR_ENUM(face)))return false;
               if(!dest.lock    (LOCK_WRITE, mip, DIR_ENUM(face))){src.unlock(); return false;}
               Int blocks_y=Min(ImageBlocksY(src .hwW(), src .hwH(), i+mip, src .hwType()),
                                ImageBlocksY(dest.hwW(), dest.hwH(),   mip, dest.hwType()));
               REPD(z, dest.ld())
               {
                C Byte * src_data= src.data() + z* src.pitch2();
                  Byte *dest_data=dest.data() + z*dest.pitch2();
                  if(dest.pitch()==src.pitch())CopyFast(dest_data, src_data, Min(dest.pitch2(), src.pitch2()));
                  else        REPD(y, blocks_y)CopyFast(dest_data + y*dest.pitch(), src_data + y*src.pitch(), Min(dest.pitch(), src.pitch()));
               }
               dest.unlock();
               src .unlock();
            }
            return true;
         }
         break;
      }
   }
   return false;
}
/******************************************************************************/
Bool Image::copyTry(Image &dest, Int w, Int h, Int d, Int type, Int mode, Int mip_maps, FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool keep_edges, Bool mtrl_base_1, Bool rgba_on_fail)C
{
   if(!is()){dest.del(); return true;}

   // adjust settings
   if(type<= 0)type=T.type();
   if(mode<=-1)mode=T.mode();
   if(h   <= 0)h   =T.h();
   if(d   <= 0)d   =T.d();
   if(w   <= 0)w   =(( cube() && !IsCube(IMAGE_MODE(mode))                       ) ? T.w()*6 // if converting from cube to non-cube                                      then use "w*6" size
                   : (!cube() &&  IsCube(IMAGE_MODE(mode)) && aspect()>AvgF(1, 6)) ? T.w()/6 // if converting from non-cube to cube and we have all 6 faces in the image then use "w/6" size
                   :                                                                 T.w()); // otherwise use "w" size

   // mip maps
   Int src_total_mip_maps=TotalMipMaps(T.w(), T.h(), T.d(),          T.type()),
      dest_total_mip_maps=TotalMipMaps(  w  ,   h  ,   d  , IMAGE_TYPE(type) );

   if(mip_maps<=-1)
   {
      if(w==T.w() && h==T.h() && d==T.d())mip_maps=T.mipMaps();else // same size
      if(T.mipMaps()< src_total_mip_maps )mip_maps=T.mipMaps();else // less than total
      if(T.mipMaps()==1                  )mip_maps=          1;else // use  only one
                                          mip_maps=          0;     // auto-detect mip maps
   }
   if(mip_maps<=0)mip_maps=dest_total_mip_maps ; // if mip maps not specified then use full chain
   else       MIN(mip_maps,dest_total_mip_maps); // don't use more than maximum allowed

   // check if doesn't require conversion
   if(this==&dest && w==T.w() && h==T.h() && d==T.d() && type==T.type() && mip_maps==T.mipMaps() && mode==T.mode())return true;

   // check for cubes
   // FIXME CUBE's should be optimized, for case when both are IsCube
 C Image *src=this;
   Image  temp_src; if(src->cube())if(temp_src.fromCube(*src, IMAGE_R8G8B8A8))src=&temp_src;else return false;
   if(IsCube((IMAGE_MODE)mode))return dest.toCube(*src, h, type, (IMAGE_MODE)mode, filter);

   // convert
   {
      // create destination
      Image temp_dest, &target=((src==&dest) ? temp_dest : dest);
      if(!target.createTry(w, h, d, IMAGE_TYPE(type), IMAGE_MODE(mode), mip_maps, rgba_on_fail))return false;

      // copy
      if(
      (src->size3()==target.size3() // if we use the same size (for which case 'filter' and 'keep_edges' are ignored)
      || (
            (filter==FILTER_BEST || filter==FILTER_DOWN) // we're going to use default filter for downsampling (which is typically used for mip-map generation)
         && !keep_edges                                  // we're not keeping the edges                        (which is typically used for mip-map generation)
         )
      )
      && CopyMipMaps(*src, target)) // and we were able to copy all mip-maps needed in the destination from source
      {
         // this does not require 'updateMipMaps'
      }else
      {
         if(src->size3()==target.size3() && src->hwType()==target.hwType()) // if match in size and hardware type
         {
            if(!src->copySoft(target, FILTER_NONE, clamp, alpha_weight, keep_edges))return false; // do raw memory copy
         }else
         if(src->size3()==target.size3() && src->compressed() && !target.compressed() // if match in size and just want to be decompressed
         && (src->hwType()==IMAGE_BC1  || src->hwType()==IMAGE_BC2  || src->hwType()==IMAGE_BC3     || src->hwType()==IMAGE_BC7       // currently only BC
          || src->hwType()==IMAGE_ETC1 || src->hwType()==IMAGE_ETC2 || src->hwType()==IMAGE_ETC2_A1 || src->hwType()==IMAGE_ETC2_A8)) //           and  ETC decompressors have this implemented
         {
            if(!Decompress(*src, target))return false;
         }else
         {
            Image decompressed_src, resized_src;
            if(src  ->compressed()){if(!Decompress(*src, decompressed_src))return false; src=&decompressed_src;} // take decompressed source
            if(target.compressed()) // target wants to be compressed
            {
               if(src->size3()!=target.size3()) // resize needed
               {
                  if(!resized_src.createTry(target.w(), target.h(), target.d(), src->hwType(), IMAGE_SOFT, 1))return false;
                  if(!src->copySoft(resized_src, filter, clamp, alpha_weight, keep_edges))return false; src=&resized_src; decompressed_src.del(); // we don't need 'decompressed_src' anymore so delete it to release memory
               }
               if(!Compress(*src, target, mtrl_base_1))return false;
            }else
            {
               if(!src->copySoft(target, filter, clamp, alpha_weight, keep_edges))return false;
            }
         }
         target.updateMipMaps(FILTER_BEST, clamp, alpha_weight, mtrl_base_1);
      }
      if(&target!=&dest)Swap(dest, target);
      return true;
   }
}
void Image::copy(Image &dest, Int w, Int h, Int d, Int type, Int mode, Int mip_maps, FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool keep_edges, Bool mtrl_base_1, Bool rgba_on_fail)C
{
   if(!copyTry(dest, w, h, d, type, mode, mip_maps, filter, clamp, alpha_weight, keep_edges, mtrl_base_1, rgba_on_fail))Exit(MLTC(u"Can't copy Image", PL,u"Nie można skopiować Image"));
}
/******************************************************************************/
Bool Image::toCube(C Image &src, Int size, Int type, IMAGE_MODE mode, FILTER_TYPE filter)
{
   if(!src.cube() && src.is())
   {
      if(type<=0      )type=src.type();
      if(size< 0      )size=src.h   ();
      if(!IsCube(mode))mode=IMAGE_SOFT_CUBE;
      Bool  one=(src.aspect()<AvgF(1, 6)); // source is only 1 face, not "6*face"
      Image temp;
      if(temp.createTry(size, size, 1, IMAGE_TYPE(type), mode, 1))
      {
         if(one)
         {
            REP(6)if(!temp.injectMipMap(src, 0, DIR_ENUM(i), filter))return false;
         }else
         {
          C Image *s=&src;
            Image  decompressed; if(src.compressed()){if(!src.copyTry(decompressed, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))return false; s=&decompressed;}
            Image  face; // keep outside the loop in case we can reuse it
            REP(6)
            {
               s->crop(face, i*s->w()/6, 0, s->w()/6, s->h());

               DIR_ENUM cube_face;
               switch(i)
               {
                  case 0: cube_face=DIR_LEFT   ; break;
                  case 1: cube_face=DIR_FORWARD; break;
                  case 2: cube_face=DIR_RIGHT  ; break;
                  case 3: cube_face=DIR_BACK   ; break;
                  case 4: cube_face=DIR_DOWN   ; break;
                  case 5: cube_face=DIR_UP     ; break;
               }
               if(!temp.injectMipMap(face, 0, cube_face, filter))return false; // inject face
            }
         }
         Swap(T, temp.updateMipMaps());
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Image::fromCube(C Image &src, Int type, IMAGE_MODE mode)
{
   if(src.cube())
   {
      if(type<=0     )type=src.type();
      if(IsCube(mode))mode=IMAGE_SOFT;

      // extract 6 faces
      Int   size=src.h();
      Image temp; if(!temp.createTry(size*6, size, 1, ImageTI[type].compressed ? IMAGE_R8G8B8A8 : IMAGE_TYPE(type), ImageTI[type].compressed ? IMAGE_SOFT : mode, 1))return false;
      if(temp.lock(LOCK_WRITE))
      {
         Image face; // keep outside the loop in case we can reuse it
         REPD(f, 6)
         {
            DIR_ENUM cube_face;
            switch(f)
            {
               case 0: cube_face=DIR_LEFT   ; break;
               case 1: cube_face=DIR_FORWARD; break;
               case 2: cube_face=DIR_RIGHT  ; break;
               case 3: cube_face=DIR_BACK   ; break;
               case 4: cube_face=DIR_DOWN   ; break;
               case 5: cube_face=DIR_UP     ; break;
            }
            if(!src.extractMipMap(face, temp.hwType(), IMAGE_SOFT, 0, cube_face))return false; // extract face
            // copy non-compressed 2D face to non-compressed 6*2D
            if(!face.lockRead())return false;
            REPD(y, size)
            {
               CopyFast(temp.data() + y*temp.pitch() + f*temp.bytePP()*size,
                        face.data() + y*face.pitch()                       , face.bytePP()*size);
            }
            face.unlock();
         }

         temp.unlock();
         if(temp.copyTry(temp, -1, -1, -1, type, mode)){Swap(T, temp); return true;}
      }
   }
   return false;
}
/******************************************************************************/
// LOCK
/******************************************************************************/
Byte* Image::softData(Int mip_map, DIR_ENUM cube_face)
{
   return _data_all+ImageSize(hwW(), hwH(), hwD(), hwType(), mode(), mip_map)+(cube_face ? ImageMipSize(hwW(), hwH(), hwD(), mip_map, hwType())*cube_face : 0); // call 'ImageMipSize' only when needed because most likely 'cube_face' is zero
}
void Image::lockSoft()
{
  _pitch      =ImagePitch  (hwW(), hwH(), lMipMap(), hwType());
  _pitch2     =ImageBlocksY(hwW(), hwH(), lMipMap(), hwType())*_pitch;
  _lock_size.x=Max(1, w()>>lMipMap());
  _lock_size.y=Max(1, h()>>lMipMap());
  _lock_size.z=Max(1, d()>>lMipMap());
  _data       =softData(lMipMap(), lCubeFace());
}
Bool Image::lock(LOCK_MODE lock, Int mip_map, DIR_ENUM cube_face)
{
   if(InRange(mip_map, mipMaps()) && InRange(cube_face, 6)) // this already handles the case of "is()"
   {
      if(mode()==IMAGE_SOFT)
      {
         if(mipMaps()==1)return true; // if there's only one mip-map then we don't need to do anything
         SyncLocker locker(ImageSoftLock);
         if(!_lock_count) // if wasn't locked yet
         {
           _lock_count=1;
           _lmm       =mip_map; lockSoft(); // set '_lmm' before calling 'lockSoft'
            return true;
         }
         if(lMipMap()==mip_map){_lock_count++; return true;} // we want the same mip-map that's already locked
      }else
      if(mode()==IMAGE_SOFT_CUBE)
      {
         SyncLocker locker(ImageSoftLock);
         if(!_lock_count) // if wasn't locked yet
         {
           _lock_count=1;
           _lmm       =mip_map; _lcf=cube_face; lockSoft(); // set '_lmm, _lcf' before calling 'lockSoft'
            return true;
         }
         if(lMipMap()==mip_map && lCubeFace()==cube_face){_lock_count++; return true;} // we want the same mip-map and cube-face that's already locked
      }else
      if(lock) // we want to set a proper lock
      {
         SyncLocker locker(D._lock);
         if(D.created())
         {
            if(!_lock_mode)switch(mode()) // first lock
            {
            #if DX9
               case IMAGE_SURF        :
               case IMAGE_SURF_SYSTEM :
               case IMAGE_SURF_SCRATCH: if(_surf)
               {
                  D3DLOCKED_RECT lr;
                  if(OK(_surf->LockRect(&lr, null, (lock==LOCK_READ) ? D3DLOCK_READONLY : 0)))
                  {
                    _lock_size.x=w();
                    _lock_size.y=h();
                    _lock_size.z=1;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =lr.Pitch;
                    _pitch2     =lr.Pitch*ImageBlocksY(hwW(), hwH(), mip_map, hwType());
                    _data       =(Byte*)lr.pBits;
                     return true;
                  }
               }break;

               case IMAGE_2D: if(_txtr)
               {
                  D3DLOCKED_RECT lr;
                  if(OK(_txtr->LockRect(mip_map, &lr, null, (lock==LOCK_READ) ? D3DLOCK_READONLY : 0)))
                  {
                     if(lock!=LOCK_READ && mip_map>0)_txtr->AddDirtyRect(null); // this is needed in case editing mip maps (not the first one, because that has automatically dirty rect set)
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =lr.Pitch;
                    _pitch2     =lr.Pitch*ImageBlocksY(hwW(), hwH(), mip_map, hwType());
                    _data       =(Byte*)lr.pBits;
                     return true;
                  }
               }break;

               case IMAGE_3D: if(_vol)
               {
                  D3DLOCKED_BOX lb;
                  if(OK(_vol->LockBox(mip_map, &lb, null, (lock==LOCK_READ) ? D3DLOCK_READONLY : 0)))
                  {
                     if(lock!=LOCK_READ && mip_map>0)_vol->AddDirtyBox(null); // this is needed in case editing mip maps (not the first one, because that has automatically dirty rect set)
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=Max(1, d()>>mip_map);
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =lb.  RowPitch;
                    _pitch2     =lb.SlicePitch;
                    _data       =(Byte*)lb.pBits;
                     return true;
                  }
               }break;

               case IMAGE_CUBE: if(_cube)
               {
                  D3DLOCKED_RECT lr;
                  if(OK(_cube->LockRect(D3DCUBEMAP_FACES(cube_face), mip_map, &lr, null, (lock==LOCK_READ) ? D3DLOCK_READONLY : 0)))
                  {
                     if(lock!=LOCK_READ && mip_map>0)_cube->AddDirtyRect(D3DCUBEMAP_FACES(cube_face), null); // this is needed in case editing mip maps (not the first one, because that has automatically dirty rect set)
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                    _lcf        =cube_face;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =lr.Pitch;
                    _pitch2     =lr.Pitch*ImageBlocksY(hwW(), hwH(), mip_map, hwType());
                    _data       =(Byte*)lr.pBits;
                     return true;
                  }
               }break;

               case IMAGE_RT: if(lock==LOCK_READ) // DX9 supports only reading for now
               {
                  Image temp; if(temp.capture(T))if(temp.lockRead())
                  {
                    _lock_size =temp._lock_size;
                    _lmm       =mip_map;
                  //_lcf       =0;
                    _lock_mode =lock;
                    _lock_count=1;
                    _pitch     =temp.pitch ();
                    _pitch2    =temp.pitch2();
                     Alloc(_data, pitch2()); CopyFast(_data, temp.data(), pitch2());
                     return true;
                  }
               }break;
            #elif DX11
               case IMAGE_RT:
               case IMAGE_DS: case IMAGE_DS_RT:
               case IMAGE_2D: if(_txtr)
               {
                  Int blocks_y=ImageBlocksY(hwW(), hwH(), mip_map, hwType()),
                      pitch   =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2  =pitch*blocks_y;

                  if(lock==LOCK_WRITE)Alloc(_data, pitch2);else
                  {
                     // get from GPU
                     Image temp; if(temp.createTry(PaddedWidth(hwW(), hwH(), mip_map, hwType()), PaddedHeight(hwW(), hwH(), mip_map, hwType()), 1, hwType(), IMAGE_SURF, 1, false))
                     {
                        D3DC->CopySubresourceRegion(temp._txtr, D3D11CalcSubresource(0, 0, temp.mipMaps()), 0, 0, 0, _txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), null);
                        if(temp.lockRead())
                        {
                           Alloc(_data, pitch2);
                           REPD(y, blocks_y)CopyFast(data()+y*pitch, temp.data()+y*temp.pitch(), pitch);
                        }
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_SURF_SCRATCH:
               case IMAGE_SURF_SYSTEM :
               case IMAGE_SURF        : if(_txtr)
               {
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(_txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), (lock==LOCK_READ) ? D3D11_MAP_READ : (lock==LOCK_WRITE) ? D3D11_MAP_WRITE : D3D11_MAP_READ_WRITE, 0, &map))) // staging does not support D3D11_MAP_WRITE_DISCARD
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _data       =(Byte*)map.pData;
                    _pitch      =       map.  RowPitch;
                    _pitch2     =       map.DepthPitch;
                     return true;
                  }
               }break;

               case IMAGE_3D: if(_vol)
               {
                  Int ld      =Max(1, d()>>mip_map),
                      blocks_y=ImageBlocksY(hwW(), hwH(), mip_map, hwType()),
                      pitch   =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2  =pitch *blocks_y,
                      pitch3  =pitch2*ld;
                  if(lock==LOCK_WRITE)Alloc(_data, pitch3);else
                  {
                     // get from GPU
                     D3D11_TEXTURE3D_DESC desc;
                     desc.Width         =PaddedWidth (hwW(), hwH(), mip_map, hwType());
                     desc.Height        =PaddedHeight(hwW(), hwH(), mip_map, hwType());
                     desc.Depth         =ld;
                     desc.MipLevels     =1;
                     desc.Format        =ImageTI[hwType()].format;
                     desc.Usage         =D3D11_USAGE_STAGING;
                     desc.BindFlags     =0;
                     desc.MiscFlags     =0;
                     desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;

                     ID3D11Texture3D *temp; if(OK(D3D->CreateTexture3D(&desc, null, &temp)))
                     {
                        D3DC->CopySubresourceRegion(temp, D3D11CalcSubresource(0,0,1), 0, 0, 0, _vol, D3D11CalcSubresource(mip_map, 0, mipMaps()), null);
                        D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, D3D11CalcSubresource(0,0,1), D3D11_MAP_READ, 0, &map)))
                        {
                           Alloc(_data, pitch3);
                           REPD(z, ld)
                           {
                              Byte *src =(Byte*)map.pData  +z*map.DepthPitch ,
                                   *dest=            data()+z*         pitch2;
                              REPD(y, blocks_y)CopyFast(dest+y*pitch, src+y*map.RowPitch, pitch);
                           }
                           D3DC->Unmap(temp, D3D11CalcSubresource(0,0,1));
                        }
                        RELEASE(temp);
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=ld;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_CUBE: if(_txtr)
               {
                  Int blocks_y=ImageBlocksY(hwW(), hwH(), mip_map, hwType()),
                      pitch   =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2  =pitch*blocks_y;
                  if(lock==LOCK_WRITE)Alloc(_data, pitch2);else
                  {
                     // get from GPU
                     Image temp; if(temp.createTry(PaddedWidth(hwW(), hwH(), mip_map, hwType()), PaddedHeight(hwW(), hwH(), mip_map, hwType()), 1, hwType(), IMAGE_SURF, 1, false))
                     {
                        D3DC->CopySubresourceRegion(temp._txtr, D3D11CalcSubresource(0, 0, temp.mipMaps()), 0, 0, 0, _txtr, D3D11CalcSubresource(mip_map, cube_face, mipMaps()), null);
                        if(temp.lockRead())
                        {
                           Alloc(_data, pitch2);
                           REPD(y, blocks_y)CopyFast(data()+y*pitch, temp.data()+y*temp.pitch(), pitch);
                        }
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                    _lcf        =cube_face;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;
            #elif GL
               case IMAGE_2D          :
               case IMAGE_RT          :
               case IMAGE_SURF_SCRATCH:
               case IMAGE_SURF_SYSTEM :
               case IMAGE_SURF        :
               case IMAGE_DS_RT       : if(_txtr)
               {
                  Int pitch =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2=ImageBlocksY(hwW(), hwH(), mip_map, hwType())*pitch;
               #if GL_ES // 'glGetTexImage' not available on GL ES
                  if(mode()==IMAGE_RT) // must use 'glReadPixels'
                  {
                     if(!compressed())
                     {
                        Alloc(_data, CeilGL(pitch2));
                        if(lock!=LOCK_WRITE) // get from GPU
                        {
                           Image *rt[Elms(Renderer._cur)], *ds;
                           Bool   restore_viewport=!D._view_active.full;
                           REPAO(rt)=Renderer._cur[i];
                                 ds =Renderer._cur_ds;

                           Renderer.set(this, null, false); // put 'this' to FBO
                           glGetError(); // clear any previous errors
                           UInt format=SourceGLFormat(hwType());
                           Bool ok    =false;
                           Int  pw=PaddedWidth (hwW(), hwH(), mip_map, hwType()),
                                ph=PaddedHeight(hwW(), hwH(), mip_map, hwType()),
                                type=SourceGLType(hwType());
                           glReadPixels(0, 0, pw, ph, format , type, _data); ok=(glGetError()==GL_NO_ERROR);
                           if(!ok)Free(_data);

                           // restore settings
                           Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
                        }
                     }
                  }else
                  {
                     if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it
                     if( _data_all)_data=softData(mip_map);
                  }
               #else
                  Alloc(_data, CeilGL(pitch2));
                  if(lock!=LOCK_WRITE) // get from GPU
                  {
                     glGetError(); // clear any previous errors
                                          D.texBind(GL_TEXTURE_2D, _txtr);
                     if(!compressed())glGetTexImage(GL_TEXTURE_2D, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glGetCompressedTexImage(GL_TEXTURE_2D, mip_map, data());
                     if(glGetError()!=GL_NO_ERROR)Free(_data);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_3D: if(_txtr)
               {
                  Int ld    =Max(1, d()>>mip_map),
                      pitch =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2=ImageBlocksY(hwW(), hwH(), mip_map, hwType())*pitch;

               #if GL_ES // 'glGetTexImage' not available on GL ES
                  if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it
                  if( _data_all)_data=softData(mip_map);
               #else
                  Alloc(_data, CeilGL(pitch2*ld));
                  if(lock!=LOCK_WRITE) // get from GPU
                  {
                     glGetError(); // clear any previous errors
                                          D.texBind(GL_TEXTURE_3D, _txtr);
                     if(!compressed())glGetTexImage(GL_TEXTURE_3D, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glGetCompressedTexImage(GL_TEXTURE_3D, mip_map, data());
                     if(glGetError()!=GL_NO_ERROR)Free(_data);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=ld;
                    _lmm        =mip_map;
                  //_lcf        =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_CUBE: if(_txtr)
               {
                  Int pitch =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
                      pitch2=ImageBlocksY(hwW(), hwH(), mip_map, hwType())*pitch;

               #if GL_ES
                //if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it, can't do it here, because we have 6 faces, but we can lock only 1
                  if( _data_all)_data=softData(mip_map, cube_face);
               #else
                  Alloc(_data, CeilGL(pitch2));
                  if(lock!=LOCK_WRITE) // get from GPU
                  {
                     glGetError(); // clear any previous errors
                                          D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                     if(!compressed())glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, data());
                     if(glGetError()!=GL_NO_ERROR)Free(_data);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lmm        =mip_map;
                    _lcf        =cube_face;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;
            #endif
            }else
            if(CompatibleLock(_lock_mode, lock) && lMipMap()==mip_map && lCubeFace()==cube_face){_lock_count++; return true;} // there was already a lock, just increase the counter and return success
         }
      }
   }
   return false;
}
/******************************************************************************/
Image& Image::unlock()
{
   if(_lock_count>0) // if image was locked
   {
      if(soft())
      {
       //for IMAGE_SOFT we don't need "if(mipMaps()>1)" check, because for IMAGE_SOFT, '_lock_count' will be set only if image has mip-maps, and since we've already checked "_lock_count>0" before, then we know we have mip-maps
         SafeSyncLocker locker(ImageSoftLock);
         if(_lock_count>0) // if locked
            if(!--_lock_count) // if unlocked now
               if(lMipMap()!=0 || lCubeFace()!=0) // if last locked mip-map or cube-face was not main
         {
           _lmm=0; _lcf=DIR_ENUM(0); lockSoft(); // set default lock members to main mip map and cube face, set '_lmm, _lcf' before calling 'lockSoft'
         }
      }else
      {
         SafeSyncLockerEx locker(D._lock);
         if(_lock_count>0)if(!--_lock_count)switch(mode())
         {
         #if DX9
            case IMAGE_SURF        :
            case IMAGE_SURF_SYSTEM :
            case IMAGE_SURF_SCRATCH:
               if(D.created())_surf->UnlockRect();
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            break;

            case IMAGE_2D:
               if(D.created())_txtr->UnlockRect(lMipMap());
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            break;

            case IMAGE_3D:
               if(D.created())_vol->UnlockBox(lMipMap());
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            break;

            case IMAGE_CUBE:
            {
               if(D.created())_cube->UnlockRect(D3DCUBEMAP_FACES(lCubeFace()), lMipMap());
              _lock_size.zero();
              _lmm      =0;
              _lcf      =DIR_ENUM(0);
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            }break;

            case IMAGE_RT:
            {
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;
         #elif DX11
            case IMAGE_RT:
            case IMAGE_2D:
            case IMAGE_DS: case IMAGE_DS_RT:
            {
               if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(lMipMap(), 0, mipMaps()), null, data(), pitch(), pitch2());
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;

            case IMAGE_SURF_SCRATCH:
            case IMAGE_SURF_SYSTEM :
            case IMAGE_SURF        :
            {
               if(D3DC)D3DC->Unmap(_txtr, D3D11CalcSubresource(lMipMap(), 0, mipMaps()));
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            }break;

            case IMAGE_3D:
            {
               if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_vol, D3D11CalcSubresource(lMipMap(), 0, mipMaps()), null, data(), pitch(), pitch2());
              _lock_size.zero();
              _lmm      =0;
            //_lcf      =0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;

            case IMAGE_CUBE:
            {
               if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(lMipMap(), lCubeFace(), mipMaps()), null, data(), pitch(), pitch2());
              _lock_size.zero();
              _lmm      =0;
              _lcf      =DIR_ENUM(0);
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;
         #elif GL
            case IMAGE_2D          :
            case IMAGE_RT          :
            case IMAGE_SURF_SCRATCH:
            case IMAGE_SURF_SYSTEM :
            case IMAGE_SURF        :
            case IMAGE_DS_RT       :
            {
               if(_lock_mode!=LOCK_READ && D.created())
                  if(!(lMipMap() && ForceDisableMipMaps(T))) // don't upload mip maps if not allowed
               {
               #if GL_ES
                  if(mode()==IMAGE_RT)
                  {
                     // glDrawPixels
                  }else
               #endif
                  {
                    _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                         D.texBind(GL_TEXTURE_2D, _txtr);
                     if(!compressed())glTexImage2D(GL_TEXTURE_2D, lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glCompressedTexImage2D(GL_TEXTURE_2D, lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, pitch2(), data());
                                           glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                     locker.on(); _lock_count--;
                  }
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lmm      =0;
               //_lcf      =0;
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
                 _discard  =false;
               #if GL_ES
                  if(mode()==IMAGE_RT)Free(_data);else _data=null;
               #else
                  Free(_data);
               #endif
               }
            }break;

            case IMAGE_3D:
            {
               if(_lock_mode!=LOCK_READ && D.created())
               {
                 _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_3D, _txtr);
                  if(!compressed())glTexImage3D(GL_TEXTURE_3D, lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), Max(1, hwD()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                  else   glCompressedTexImage3D(GL_TEXTURE_3D, lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), Max(1, hwD()>>lMipMap()), 0, pitch2()*ld(), data());
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  locker.on(); _lock_count--;
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lmm      =0;
               //_lcf      =0;
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
               #if GL_ES
                 _data     =null;
               #else
                  Free(_data);
               #endif
               }
            }break;

            case IMAGE_CUBE:
            {
               if(_lock_mode!=LOCK_READ && D.created())
               {
                 _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                  if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+lCubeFace(), lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                  else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+lCubeFace(), lMipMap(), ImageTI[hwType()].format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, pitch2(), data());
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  locker.on(); _lock_count--;
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lmm      =0;
                 _lcf      =DIR_ENUM(0);
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
               #if GL_ES
                 _data     =null;
               #else
                  Free(_data);
               #endif
               }
            }break;
         #endif
         }
      }
   }
   return T;
}
  Bool   Image::  lockRead(Int mip_map, DIR_ENUM cube_face)C {return ConstCast(T).  lock(LOCK_READ, mip_map, cube_face);}
C Image& Image::unlock    (                               )C {return ConstCast(T).unlock();}
/******************************************************************************/
Bool Image::setFrom(CPtr data, Int data_pitch, Int mip_map, DIR_ENUM cube_face)
{
   if(data)
   {
      Int valid_pitch   =ImagePitch  (w(), h(), mip_map, hwType()),
          valid_blocks_y=ImageBlocksY(w(), h(), mip_map, hwType());
   #if DEBUG && 0 // force HW size
      #pragma message("!! Warning: Use this only for debugging !!")
      Memt<Byte> temp;
      {
         Int hw_pitch   =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
             hw_blocks_y=ImageBlocksY(hwW(), hwH(), mip_map, hwType()),
             hw_pitch2  =hw_pitch*hw_blocks_y;
         temp.setNum(hw_pitch2);
         Int copy_pitch=Min(hw_pitch, data_pitch, valid_pitch);
         FREP(valid_blocks_y)Copy(temp.data()+hw_pitch*i, (Byte*)data+data_pitch*i, copy_pitch);
         data=temp.data(); data_pitch=hw_pitch;
      }
   #endif
   #if DX11
      if(hw() && InRange(mip_map, mipMaps()) && InRange(cube_face, 6))
      {
         Int data_pitch2=data_pitch*valid_blocks_y; // 'data_pitch2' could be moved into a method parameter
         SyncLocker locker(D._lock); if(D3DC)switch(mode())
         {
            case IMAGE_RT:
            case IMAGE_2D:
            case IMAGE_DS: case IMAGE_DS_RT:
            {
               D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), null, data, data_pitch, data_pitch2);
            }return true;

            case IMAGE_3D:
            {
               D3DC->UpdateSubresource(_vol, D3D11CalcSubresource(mip_map, 0, mipMaps()), null, data, data_pitch, data_pitch2);
            }return true;

            case IMAGE_CUBE:
            {
               D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(mip_map, cube_face, mipMaps()), null, data, data_pitch, data_pitch2);
            }return true;
         }
      }
   #elif GL // GL can accept only HW sizes
      Int hw_pitch   =ImagePitch  (hwW(), hwH(), mip_map, hwType()),
          hw_blocks_y=ImageBlocksY(hwW(), hwH(), mip_map, hwType()),
          hw_pitch2  =hw_pitch*hw_blocks_y;
      if( hw_pitch==data_pitch && InRange(mip_map, mipMaps()) && InRange(cube_face, 6) && D.created())switch(mode())
      {
         case IMAGE_2D          :
         case IMAGE_RT          :
         case IMAGE_SURF_SCRATCH:
         case IMAGE_SURF_SYSTEM :
         case IMAGE_SURF        :
         case IMAGE_DS_RT       :
         { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                D.texBind(GL_TEXTURE_2D, _txtr);
            if(!compressed())glTexImage2D(GL_TEXTURE_2D, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data);
            else   glCompressedTexImage2D(GL_TEXTURE_2D, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), 0, hw_pitch2, data);
                                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
           _discard=false;
         }return true;

         case IMAGE_3D:
         { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
            Int d=Max(1, hwD()>>mip_map);
                                D.texBind(GL_TEXTURE_3D, _txtr);
            if(!compressed())glTexImage3D(GL_TEXTURE_3D, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), d, 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data);
            else   glCompressedTexImage3D(GL_TEXTURE_3D, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), d, 0, hw_pitch2*d, data);
                                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
         }return true;

         case IMAGE_CUBE:
         { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
            if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data);
            else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, ImageTI[hwType()].format, Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), 0, hw_pitch2, data);
                                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
         }return true;
      }
   #endif
      if(lock(LOCK_WRITE, mip_map, cube_face))
      {
         Byte *dest_data =T.data();
   const Int   copy_pitch=Min(T.pitch(), data_pitch, valid_pitch),
               zero_pitch=T.pitch ()-copy_pitch,
               pitch2    =T.pitch ()*valid_blocks_y,
               zero      =T.pitch2()-pitch2; // how much to zero = total - what was set
         FREPD(z, ld())
         {
            if(copy_pitch==data_pitch && !zero_pitch) // if all pitches are the same (copy_pitch, data_pitch, T.pitch)
            {  // we can copy both XY in one go !! use 'pitch2' and not 'T.pitch2', because 'T.pitch2' may be bigger !!
               CopyFast(dest_data, data, pitch2);
               dest_data+=            pitch2;
                    data =(Byte*)data+pitch2;
            }else
            FREPD(y, valid_blocks_y) // copy each line separately
            {
               CopyFast(dest_data, data, copy_pitch);
               Zero    (dest_data+copy_pitch, zero_pitch); // zero remaining data to avoid garbage
               dest_data+=               T.pitch();
                    data =(Byte*)data+data_pitch;
            }
            Zero(dest_data, zero); // zero remaining data to avoid garbage
            dest_data+=zero;
         }
         unlock();
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Image& Image::freeOpenGLESData()
{
#if GL_ES
   unlock();
   if(hw())Free(_data_all);
#endif
   return T;
}
/******************************************************************************/
Image& Image::updateMipMaps(FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool mtrl_base_1, Int mip_start)
{
   MAX(mip_start, 0);
   if (InRange(mip_start+1, mipMaps())) // if we can set the next one
   {
      Image temp; // keep outside the loop in case we can reuse the image
      REPD(f, faces())
      {
         extractMipMap(temp, ImageTI[type()].compressed ? IMAGE_R8G8B8A8 : type(), IMAGE_SOFT, mip_start, DIR_ENUM(f)); // use 'type' instead of 'hwType' (this is correct)
         for(Int mip=mip_start; ++mip<mipMaps(); )
         {
            temp.downSample(filter, clamp, alpha_weight);
            injectMipMap(temp, mip, DIR_ENUM(f), FILTER_BEST, clamp, mtrl_base_1);
         }
      }
   }
   return T;
}
/******************************************************************************/
// GET
/******************************************************************************/
Int Image::faces()C {return is() ? cube() ? 6 : 1 : 0;}
/******************************************************************************/
UInt Image::    memUsage()C {return ImageSize(hwW(), hwH(), hwD(), hwType(), mode(), mipMaps());}
UInt Image::typeMemUsage()C {return ImageSize(hwW(), hwH(), hwD(),   type(), mode(), mipMaps());}
/******************************************************************************/
Bool Image::map()
{
#if DX9
   del(); if(OK(D3D->GetRenderTarget(0, &T._surf))){setInfo(0, 0, 0, 0, IMAGE_SURF); return true;}
#elif DX11
   del(); if(OK(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (Ptr*)&_txtr))){setInfo(0, 0, 0, 0, IMAGE_RT); return true;}
#elif DX12
   https://msdn.microsoft.com/en-us/library/windows/desktop/mt427784(v=vs.85).aspx
   In Direct3D 11, applications could call GetBuffer( 0, .. ) only once. Every call to Present implicitly changed the resource identity of the returned interface. Direct3D 12 no longer supports that implicit resource identity change, due to the CPU overhead required and the flexible resource descriptor design. As a result, the application must manually call GetBuffer for every each buffer created with the swapchain. The application must manually render to the next buffer in the sequence after calling Present. Applications are encouraged to create a cache of descriptors for each buffer, instead of re-creating many objects each Present.
#elif IOS
   del();
   if(EAGLView *view=GetUIView())
   {
      glGenRenderbuffers(1, &_rb); if(_rb)
      {
         glGetError(); // clear any previous errors
         glBindRenderbuffer(GL_RENDERBUFFER, _rb);
         [MainContext.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)view.layer];
         setInfo(0, 0, 0, 0, IMAGE_SURF); // this has a valid '_rb' so it can detect the size and type
         D._res=size(); D.densityUpdate();
         return true;
      }
   }
#elif ANDROID || WEB
   // on Android and Web 'Renderer._main' has 'setInfo' called externally in the main loop
   return true;
#elif DESKTOP
   forceInfo(D.resW(), D.resH(), 1, type() ? type() : IMAGE_DEFAULT, IMAGE_SURF); return true;
#endif
   return false;
}
/******************************************************************************/
void Image::unmap()
{
#if DX9 || DX11
   del();
#elif IOS
   if(_rb)
   {
      glBindRenderbuffer(GL_RENDERBUFFER, _rb);
      [MainContext.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:nil]; // detach existing renderbuffer from the drawable object
      del();
   }
#else
   // on other platforms we're not responsible for the 'Renderer._main' as the system creates it and deletes it, don't delete it here, to preserve info about IMAGE_TYPE and samples
#endif
}
/******************************************************************************/
// HARDWARE
/******************************************************************************/
void SetRects(C Image &src, C Image &dest, RectI &rect_src, RectI &rect_dest, C Rect &rect)
{
   Rect uv=D.screenToUV(rect);

   // first the smaller Image must be set, and then the bigger Image must be set proportionally

   if(dest.hwW()<src.hwW()){rect_dest.setX(Round(uv.min.x*dest.hwW()), Round(uv.max.x*dest.hwW())); rect_src .setX(rect_dest.min.x* src.hwW()/dest.hwW(), rect_dest.max.x* src.hwW()/dest.hwW());}
   else                    {rect_src .setX(Round(uv.min.x* src.hwW()), Round(uv.max.x* src.hwW())); rect_dest.setX(rect_src .min.x*dest.hwW()/ src.hwW(), rect_src .max.x*dest.hwW()/ src.hwW());}

   if(dest.hwH()<src.hwH()){rect_dest.setY(Round(uv.min.y*dest.hwH()), Round(uv.max.y*dest.hwH())); rect_src .setY(rect_dest.min.y* src.hwH()/dest.hwH(), rect_dest.max.y* src.hwH()/dest.hwH());}
   else                    {rect_src .setY(Round(uv.min.y* src.hwH()), Round(uv.max.y* src.hwH())); rect_dest.setY(rect_src .min.y*dest.hwH()/ src.hwH(), rect_src .max.y*dest.hwH()/ src.hwH());}
}
void Image::discard()
{
#if DX11
   if(D3DC1)D3DC1->DiscardView(_rtv ? &SCAST(ID3D11View, *_rtv) : &SCAST(ID3D11View, *_dsv)); // will not crash if parameter is null
#elif GL && GL_ES // do this only on GLES, because on desktop it requires GL 4.3 TODO:
   // this should be called only if this image is already attached to current FBO - https://community.arm.com/graphics/b/blog/posts/mali-performance-2-how-to-correctly-handle-framebuffers
   // 'glInvalidateFramebuffer' can be called at the start of rendering (right after attaching to   FBO) to specify that we don't need previous     contents of this RT
   //                                     and at the end   of rendering (     BEFORE detaching from FBO) to specify that we don't need to store the contents of this RT
   if(D.notShaderModelGLES2()) // not available on GLES2
   {
      // !! remember that images can have only '_txtr' or '_rb' or nothing at all (if they're provided by the system) but we still should discard them !!
      if(!IOS && D.mainFBO()) // iOS doesn't have main FBO
      { // for main FBO we need to setup different values - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glInvalidateFramebuffer.xhtml
         if(Renderer._cur_ds==this) // no need to check '_cur_ds_id' because main FBO always has texture 0
         {
            GLenum attachments[]={GL_DEPTH, GL_STENCIL};
            glInvalidateFramebuffer(GL_FRAMEBUFFER, ImageTI[hwType()].s ? 2 : 1, attachments); _discard=false;
         }else
         if(Renderer._cur[0]==this) // check '_cur' because '_txtr' can be 0 for RenderBuffers
         {
            GLenum attachment=GL_COLOR;
            glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment); _discard=false;
         }else
           _discard=true; // discard at next opportunity when we want to attach it to FBO
      }else
      {
         GLenum attachment;
         if(Renderer._cur_ds==this && Renderer._cur_ds_id==_txtr)attachment=(ImageTI[hwType()].s ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT);else // check both '_cur_ds' and '_cur_ds_id' because '_cur_ds_id' will be 0 when Image is a RenderBuffer or temporarily unbound Texture (only Textures can be temporarily unbound), this will work OK for RenderBuffers because both '_cur_ds_id' and '_txtr' will be zero
         if(Renderer._cur[0]==this                              )attachment=GL_COLOR_ATTACHMENT0;else // check '_cur' because '_txtr' can be 0 for RenderBuffers
         if(Renderer._cur[1]==this                              )attachment=GL_COLOR_ATTACHMENT1;else // check '_cur' because '_txtr' can be 0 for RenderBuffers
         if(Renderer._cur[2]==this                              )attachment=GL_COLOR_ATTACHMENT2;else // check '_cur' because '_txtr' can be 0 for RenderBuffers
         if(Renderer._cur[3]==this                              )attachment=GL_COLOR_ATTACHMENT3;else // check '_cur' because '_txtr' can be 0 for RenderBuffers
            {_discard=true; return;} // discard at next opportunity when we want to attach it to FBO
         glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment); _discard=false;
      }
   }
#endif
}
#if DX9
void Image::clearHw(C Color &color)
{
   if(_surf)D3D->ColorFill(_surf, null, VecB4(color.b, color.g, color.r, color.a).u); // 'ColorFill' accepts colors in BGRA
}
#elif DX11
void Image::clearHw(C Vec4 &color)
{
   if(_rtv)D3DC->ClearRenderTargetView(_rtv, color.c);
}
void Image::clearDS(Byte s)
{
   if(_dsv)D3DC->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH|(ImageTI[hwType()].s ? D3D11_CLEAR_STENCIL : 0), 1, s);
}
void Image::copyMs(Image &dest, Bool restore_rt, Bool multi_sample, C RectI *rect)C
{
   if(this!=&dest)
   {
      Image *rt[Elms(Renderer._cur)], *ds;
      Bool   restore_viewport;
      if(restore_rt)
      {
         REPAO(rt)=Renderer._cur[i];
               ds =Renderer._cur_ds;
         restore_viewport=!D._view_active.full;
      }

      Renderer.set(&dest, null, false);
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);

      VI.shader(!multiSample() ? Sh.h_Draw    : // 1s->1s, 1s->ms
                !multi_sample  ? Sh.h_DrawMs1 : // #0->1s, #0->ms
            dest.multiSample() ? Sh.h_DrawMsM : // ms->ms
                                 Sh.h_DrawMsN); // ms->1s
                                VI.image  (this);
                                VI.setType(VI_2D_TEX, VI_STRIP);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx (4))
      {
         if(!rect)
         {
            v[0].pos.set(-1,  1);
            v[1].pos.set( 1,  1);
            v[2].pos.set(-1, -1);
            v[3].pos.set( 1, -1);

            v[0].tex.set(0, 0);
            v[1].tex.set(1, 0);
            v[2].tex.set(0, 1);
            v[3].tex.set(1, 1);
         }else
         {
            Rect frac(rect->min.x/Flt(dest.hwW())*2-1, rect->max.y/Flt(dest.hwH())*-2+1,
                      rect->max.x/Flt(dest.hwW())*2-1, rect->min.y/Flt(dest.hwH())*-2+1);
            v[0].pos.set(frac.min.x, frac.max.y);
            v[1].pos.set(frac.max.x, frac.max.y);
            v[2].pos.set(frac.min.x, frac.min.y);
            v[3].pos.set(frac.max.x, frac.min.y);

            Rect tex(Flt(rect->min.x)/hwW(), Flt(rect->min.y)/hwH(),
                     Flt(rect->max.x)/hwW(), Flt(rect->max.y)/hwH());
            v[0].tex.set(tex.min.x, tex.min.y);
            v[1].tex.set(tex.max.x, tex.min.y);
            v[2].tex.set(tex.min.x, tex.max.y);
            v[3].tex.set(tex.max.x, tex.max.y);
         }
      }
      VI.end();

      D.alpha(alpha);
      if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
   }
}
void Image::copyMs(Image &dest, Bool restore_rt, Bool multi_sample, C Rect &rect)C
{
   copyMs(dest, restore_rt, multi_sample, &Round(D.screenToUV(rect)*size()));
}
#endif
/******************************************************************************/
void Image::copyHw(Image &dest, Bool restore_rt, C RectI *rect_src, C RectI *rect_dest, Bool *flipped)C
{
   if(flipped)*flipped=false;
   if(this!=&dest)
   {
   #if DX9
      if(this==&Renderer._main || multiSample()) // in DX9 cannot directly copy from 'main' and multi-sampled surfaces
      {
         if(_surf && dest._surf)
         {
            RECT rs, rd;
            if(rect_src ){rs.left=Max(rect_src ->min.x, 0); rs.right=Min(rect_src ->max.x,      w()); if(rs.left>=rs.right)return; rs.top=Max(rect_src ->min.y, 0); rs.bottom=Min(rect_src ->max.y,      h()); if(rs.top>=rs.bottom)return;}
            if(rect_dest){rd.left=Max(rect_dest->min.x, 0); rd.right=Min(rect_dest->max.x, dest.w()); if(rd.left>=rd.right)return; rd.top=Max(rect_dest->min.y, 0); rd.bottom=Min(rect_dest->max.y, dest.h()); if(rd.top>=rd.bottom)return;}
            D3D->StretchRect(_surf, rect_src ? &rs : null, dest._surf, rect_dest ? &rd : null, D3DTEXF_LINEAR);
         }
         return;
      }
   #elif GL
      if(this==&Renderer._main) // in OpenGL cannot directly copy from main
      {
         if(dest._txtr)
         {
            RectI rs(0, 0,    T.w(),    T.h()); if(rect_src )rs&=*rect_src ; if(rs.min.x>=rs.max.x || rs.min.y>=rs.max.y)return;
            RectI rd(0, 0, dest.w(), dest.h()); if(rect_dest)rd&=*rect_dest; if(rd.min.x>=rd.max.x || rd.min.y>=rd.max.y)return;

         #if GL_ES
            // remember render target settings
            Image *rt[Elms(Renderer._cur)], *ds;
            Bool   restore_viewport;
            if(restore_rt)
            {
               REPAO(rt)=Renderer._cur[i];
                     ds =Renderer._cur_ds;
               restore_viewport=!D._view_active.full;
            }
            Renderer.set(&Renderer._main, null, false); // put '_main' to FBO

            dest._discard=false;
            if(rs.w()==rd.w() && rs.h()==rd.h() && hwType()==dest.hwType() && flipped) // 'glCopyTexSubImage2D' flips image, does not support stretching and format conversion
            {
              *flipped=true;
               D.texBind          (GL_TEXTURE_2D, dest._txtr); // set destination texture
               glCopyTexSubImage2D(GL_TEXTURE_2D, 0, rd.min.x, rd.min.y, rs.min.x, rs.min.y, rs.w(), rs.h()); // copy partial FBO to texture (this will copy the image flipped vertically)
            }else
            {
               ImageRTPtr temp(ImageRTDesc(w(), h(), GetImageRTType(type())));

               D.texBind          (GL_TEXTURE_2D, temp->_txtr); // set destination texture
               glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, temp->w(), temp->h()); // copy entire FBO to texture (this will copy the image flipped vertically)

               // flip
               RectI rect_src_flipped; if(rect_src)rect_src_flipped=*rect_src;else rect_src_flipped.set(0, 0, w(), h()); Swap(rect_src_flipped.min.y, rect_src_flipped.max.y); // set flipped rectangle
               temp->copyHw(dest, false, &rect_src_flipped, rect_dest); // perform additional copy
            }

            // restore settings
            if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
         #else
            // remember settings
            Image *rt[Elms(Renderer._cur)], *ds;
            Bool   restore_viewport;
            if(restore_rt)
            {
               REPAO(rt)=Renderer._cur[i];
                     ds =Renderer._cur_ds;
               restore_viewport=!D._view_active.full;
            }

            Renderer.set(&dest, null, false); // put 'dest' to FBO
            glBindFramebuffer(GL_READ_FRAMEBUFFER,   0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
            glBlitFramebuffer(rs.min.x, h()-rs.min.y, rs.max.x, h()-rs.max.y,
                              rd.min.x,     rd.min.y, rd.max.x,     rd.max.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glBindFramebuffer(GL_FRAMEBUFFER, D._fbo); // restore framebuffer, for this function GL_FRAMEBUFFER acts as both GL_READ_FRAMEBUFFER and GL_DRAW_FRAMEBUFFER at the same time

            // restore settings
            if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
         #endif
         }
         return;
      }
   #endif
      // remember settings
      Image *rt[Elms(Renderer._cur)], *ds;
      Bool   restore_viewport;
      if(restore_rt)
      {
         REPAO(rt)=Renderer._cur[i];
               ds =Renderer._cur_ds;
         restore_viewport=!D._view_active.full;
      }

      Renderer.set(&dest, null, false);
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);

      VI.image  (this);
      VI.shader (Sh.h_Draw);
      VI.setType(VI_2D_TEX, VI_STRIP);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
      {
         if(!rect_dest)
         {
            v[0].pos.set(-1,  1);
            v[1].pos.set( 1,  1);
            v[2].pos.set(-1, -1);
            v[3].pos.set( 1, -1);
         }else
         {
            Rect frac(rect_dest->min.x/Flt(dest.hwW())*2-1, -rect_dest->max.y/Flt(dest.hwH())*2+1,
                      rect_dest->max.x/Flt(dest.hwW())*2-1, -rect_dest->min.y/Flt(dest.hwH())*2+1);
            v[0].pos.set(frac.min.x, frac.max.y);
            v[1].pos.set(frac.max.x, frac.max.y);
            v[2].pos.set(frac.min.x, frac.min.y);
            v[3].pos.set(frac.max.x, frac.min.y);
         }

         if(!rect_src)
         {
            v[0].tex.set(0, 0);
            v[1].tex.set(1, 0);
            v[2].tex.set(0, 1);
            v[3].tex.set(1, 1);
         }else
         {
            Rect tex(Flt(rect_src->min.x)/hwW(), Flt(rect_src->min.y)/hwH(),
                     Flt(rect_src->max.x)/hwW(), Flt(rect_src->max.y)/hwH());
            v[0].tex.set(tex.min.x, tex.min.y);
            v[1].tex.set(tex.max.x, tex.min.y);
            v[2].tex.set(tex.min.x, tex.max.y);
            v[3].tex.set(tex.max.x, tex.max.y);
         }
      #if GL
         if(!D.mainFBO()) // in OpenGL when drawing to RenderTarget the 'dest.pos.y' must be flipped
         {
            CHS(v[0].pos.y);
            CHS(v[1].pos.y);
            CHS(v[2].pos.y);
            CHS(v[3].pos.y);
         }
      #endif
      }
      VI.end();

      // restore settings
      D.alpha(alpha);
      if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
   }
}
void Image::copyHw(Image &dest, Bool restore_rt, C Rect &rect)C
{
   RectI rect_src, rect_dest;
   SetRects(T, dest, rect_src, rect_dest, rect);
   copyHw(dest, restore_rt, &rect_src, &rect_dest);
}
/******************************************************************************/
void Image::clearFull(C Vec4 &color, Bool restore_rt)
{
#if DX11
   clearHw(color);
#else
   Image *rt[Elms(Renderer._cur)], *ds;
   Bool   restore_viewport;
   if(restore_rt)
   {
      REPAO(rt)=Renderer._cur[i];
            ds =Renderer._cur_ds;
      restore_viewport=!D._view_active.full;
   }

   if(color.min()>=0 && color.max()<=1)
   {
   #if DX9
      clearHw(color); return; // no need to restore RT's as this doesn't change them
   #else
      Renderer.set(this, null, false); D.clearCol(color);
   #endif
   }else
   {
      Renderer.set(this, null, false); Bool clip=D._clip_allow; D.clipAllow(false); ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Sh.clear(color);
                                                                D.clipAllow(clip );                  D.alpha(alpha     );
   }

   if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
#endif
}
void Image::clearViewport(C Vec4 &color, Bool restore_rt)
{
   if(D._view_main.full)clearFull(color, restore_rt);else
   {
      Image *rt[Elms(Renderer._cur)], *ds;
      Bool   restore_viewport;
      if(restore_rt)
      {
         REPAO(rt)=Renderer._cur[i];
               ds =Renderer._cur_ds;
         restore_viewport=!D._view_active.full;
      }

      Renderer.set(this, null, true);
      if(color.min()>=0 && color.max()<=1)D.clearCol(color);else
      {
         Bool clip=D._clip_allow; D.clipAllow(false); ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Sh.clear(color);
                                  D.clipAllow(clip );                  D.alpha(alpha     );
      }

      if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
   }
}
/******************************************************************************/
Bool Image::capture(C Image &src)
{
#if DX9
   if(src._surf)
   {
      if(ImageTI[src.hwType()].d) // depth buffer
      {
         if(src.depthTexture())
         {
            SyncLocker locker(D._lock);
            if(createTry(src.w(), src.h(), 1, IMAGE_F32, IMAGE_RT, 1, false))
            {
               src.copyHw(T, true);
               return true;
            }
         }
      }else
      {
         if(size()!=src.size() || hwType()!=src.hwType()
         ||(mode()!=IMAGE_SURF_SCRATCH && mode()!=IMAGE_SURF_SYSTEM && mode()!=IMAGE_SURF) // only these modes can receive 'GetRenderTargetData'
         )createTry(src.w(), src.h(), 1, src.hwType(), IMAGE_SURF_SYSTEM, 1, false);

         if(_surf)
         {
            SyncLocker locker(D._lock);
            if(OK(D3D->GetRenderTargetData(src._surf, _surf)))
            {
               Time.skipUpdate();
               return true;
            }
         }
      }
   }
#elif DX11
   if(src._txtr)
   {
      SyncLocker locker(D._lock);
      if(src.multiSample())
      {
         if(createTry(src.w(), src.h(), 1, src.hwType(), IMAGE_RT, 1, false))
         {
            D3DC->ResolveSubresource(_txtr, 0, src._txtr, 0, ImageTI[src.hwType()].format);
            return true;
         }
      }else
      if(createTry(PaddedWidth(src.hwW(), src.hwH(), 0, src.hwType()), PaddedHeight(src.hwW(), src.hwH(), 0, src.hwType()), 1, src.hwType(), IMAGE_SURF, 1, false))
      {
         D3DC->CopySubresourceRegion(_txtr, D3D11CalcSubresource(0, 0, mipMaps()), 0, 0, 0, src._txtr, D3D11CalcSubresource(0, 0, src.mipMaps()), null);
         return true;
      }
   }
#elif GL
   if(src.lockRead())
   {
      Bool ok=false;
      if(createTry(src.w(), src.h(), 1, src.hwType(), IMAGE_SOFT, 1, false))ok=src.copySoft(T, FILTER_NO_STRETCH);
      src.unlock();
      return ok;
   }else
   {
      Bool depth=(ImageTI[src.hwType()].d>0);
      if( !depth || src.depthTexture())
      {
         SyncLocker locker(D._lock);
         if(createTry(src.w(), src.h(), 1, depth ? IMAGE_F32 : src.hwType(), IMAGE_RT, 1, false))
         {
            src.copyHw(T, true);
            return true;
         }
      }
   }
#endif
   return false;
}
/******************************************************************************/
Bool Image::accessible()C
{
#if GL
   return _rb || _txtr; // on some platforms with OpenGL, 'Renderer._main' and 'Renderer._main_ds' are provided by the system, so their values may be zero, and we can't directly access it
#else
   return true;
#endif
}
Bool Image::depthTexture()C
{
#if DX9 || GL
   return mode()==IMAGE_DS_RT;
#else
   return _dsv && _srv; // on DX10+ IMAGE_DS and IMAGE_DS_RT is the same
#endif
}
Bool Image::compatible(C Image &image)C
{
   return size3()==image.size3() && samples()==image.samples();
}
/******************************************************************************/
}
/******************************************************************************/
