/******************************************************************************/
#ifdef __linux__
   #define USE_DLL 1
#else
   #define USE_DLL 0
#endif

#if USE_DLL
   #include "stdafx.h"
   #define private   public
   #define protected public
#endif

#include "../begin.h"
#include "PVRTex/PVRTexture.h"
#include "PVRTex/PVRTextureUtilities.h"
#include "../end.h"

#if USE_DLL
pvrtexture::PixelType::PixelType(uint64 Type) {PixelTypeID=Type;}
pvrtexture::PixelType::PixelType(uint8 C1Name, uint8 C2Name, uint8 C3Name, uint8 C4Name, uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits)
{
   PixelTypeChar[0]=C1Name;
   PixelTypeChar[1]=C2Name;
   PixelTypeChar[2]=C3Name;
   PixelTypeChar[3]=C4Name;
   PixelTypeChar[4]=C1Bits;
   PixelTypeChar[5]=C2Bits;
   PixelTypeChar[6]=C3Bits;
   PixelTypeChar[7]=C4Bits;
}
#endif

namespace EE{
/******************************************************************************/
typedef       unsigned char Byte;
typedef       void        * Ptr ;
typedef const void        *CPtr ;
typedef       bool          Bool;
typedef       int           Int ;
/******************************************************************************/
Bool _CompressPVRTC(Int w, Int h, CPtr data, Int type, Int quality, Ptr &compressed_data, Int &compressed_size)
{
   Bool ok=false;

   compressed_data=NULL;
   compressed_size=0;

   using namespace pvrtexture;

#if USE_DLL
   Byte _texture[SIZE(CPVRTexture)]; memset(_texture, 0, SIZE(_texture));
   CPVRTexture &texture=(CPVRTexture&)_texture;
   PVRTextureHeaderV3 &header=texture.m_sHeader;
   header=PVRTextureHeaderV3();
   header.u32Width=w;
   header.u32Height=h;
   header.u64PixelFormat=PVRStandard8PixelType.PixelTypeID;
   texture.m_stDataSize=w*h*4; // assumes that image is RGBA (size Color == 4)
   texture.m_pTextureData=(uint8*)malloc(texture.m_stDataSize); memcpy(texture.m_pTextureData, data, Int(texture.m_stDataSize)); // !! 'texture.m_pTextureData' must have its own unique memory !!
   typedef bool (*TranscodeType)(CPVRTexture& sTexture, const PixelType ptFormat, const EPVRTVariableType eChannelType, const EPVRTColourSpace eColourspace, const ECompressorQuality eQuality, const bool bDoDither);
   #if DEBUG
      TranscodeType temp=Transcode; // make sure that the type is OK
   #endif
   DLL dll;
   #ifdef _WIN32
      #if X64
         if(dll.createFile("C:/Esenthel/ThirdPartyLibs/PVRTC/PVRTex/Windows_x86_64/Dynamic/PVRTexLib.dll"))
         if(TranscodeType Transcode=(TranscodeType)dll.getFunc("?Transcode@pvrtexture@@YA_NAEAVCPVRTexture@1@TPixelType@1@W4EPVRTVariableType@@W4EPVRTColourSpace@@W4ECompressorQuality@1@_N@Z"))
      #else
         if(dll.createFile("C:/Esenthel/ThirdPartyLibs/PVRTC/PVRTex/Windows_x86_32/Dynamic/PVRTexLib.dll"))
         if(TranscodeType Transcode=(TranscodeType)dll.getFunc("?Transcode@pvrtexture@@YA_NAAVCPVRTexture@1@TPixelType@1@W4EPVRTVariableType@@W4EPVRTColourSpace@@W4ECompressorQuality@1@_N@Z"))
      #endif
   #elif defined __linux__
      if(dll.createFile("libPVRTexLib.so"))
      if(TranscodeType Transcode=(TranscodeType)dll.getFunc("_ZN10pvrtexture9TranscodeERNS_11CPVRTextureENS_9PixelTypeE17EPVRTVariableType16EPVRTColourSpaceNS_18ECompressorQualityEb"))
   #endif
#else
   CPVRTextureHeader header(PVRStandard8PixelType.PixelTypeID, h, w);
   CPVRTexture       texture(header, data);
#endif
   if(Transcode(texture, EPVRTPixelFormat(type), ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB, ECompressorQuality(quality), false)) // dithering makes smooth gradients (like sky) very bad so always disable them
   {
   #if USE_DLL
      Ptr data=texture.m_pTextureData;
      Int size=texture.m_stDataSize;
   #else
      Ptr data=texture.getDataPtr ();
      Int size=texture.getDataSize();
   #endif
      if( data && size>0)
      {
         memcpy(compressed_data=malloc(size), data, compressed_size=size);
         ok=true;
      }
   }
#if USE_DLL
   free(texture.m_pTextureData);
#endif
   return ok;
}
/******************************************************************************/
} // namespace
/******************************************************************************/
