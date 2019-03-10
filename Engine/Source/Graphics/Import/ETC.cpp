/******************************************************************************/
#include "stdafx.h"

#define ETC_LIB_TEXGENPACK 1 // only decompression, faster than ETC_LIB_RG and ETC_LIB_ETCPACK
#define ETC_LIB_RG         2 // only ETC1
#define ETC_LIB_ETCPACK    3

#define ETC1_DEC ETC_LIB_TEXGENPACK
#define ETC2_DEC ETC_LIB_TEXGENPACK

#include "../../../../ThirdPartyLibs/begin.h"

#if ETC1_DEC==ETC_LIB_RG
   #include "../../../../ThirdPartyLibs/RG-ETC1/rg_etc1.cpp"
#endif

#if ETC1_DEC==ETC_LIB_TEXGENPACK || ETC2_DEC==ETC_LIB_TEXGENPACK
   namespace TGP
   {
      #include "../../../../ThirdPartyLibs/TexGenPack/etc2.c"
   }
#endif

#include "../../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
Bool (*CompressETC)(C Image &src, Image &dest, Int quality, Bool perceptual);
/******************************************************************************/
#if ETC1_DEC==ETC_LIB_RG
void DecompressBlockETC1(C Byte *b, Color (&block)[4][4]) {rg_etc1::unpack_etc1_block(b, (UInt*)block);}
#elif ETC1_DEC==ETC_LIB_TEXGENPACK
void DecompressBlockETC1(C Byte *b, Color (&block)[4][4]) {   TGP::draw_block4x4_etc1(b, (UInt*)block);}
#elif ETC1_DEC==ETC_LIB_ETCPACK
void DecompressBlockETC1(C Byte *b, Color (&block)[4][4])
{
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockDiffFlipC(b0, b1, block[0][0].c, 4, 4, 0, 0, 4);
   FREPD(y, 4)FREPD(x, 4)block[y][x].a=255;
}
#endif
#if ETC2_DEC==ETC_LIB_TEXGENPACK
void DecompressBlockETC2  (C Byte *b, Color (&block)[4][4]) {TGP::draw_block4x4_etc2_rgb8        (b, (UInt*)block);}
void DecompressBlockETC2A1(C Byte *b, Color (&block)[4][4]) {TGP::draw_block4x4_etc2_punchthrough(b, (UInt*)block);}
void DecompressBlockETC2A8(C Byte *b, Color (&block)[4][4]) {TGP::draw_block4x4_etc2_eac         (b, (UInt*)block);}
#endif
#if ETC2_DEC==ETC_LIB_ETCPACK
void DecompressBlockETC2(C Byte *b, Color (&block)[4][4])
{
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC2c(b0, b1, block[0][0].c, 4, 4, 0, 0, 4);
   FREPD(y, 4)FREPD(x, 4)block[y][x].a=255;
}
void DecompressBlockETC2A1(C Byte *b, Color (&block)[4][4])
{
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC21BitAlphaC(b0, b1, block[0][0].c, null, 4, 4, 0, 0, 4);
}
void DecompressBlockETC2A8(C Byte *b, Color (&block)[4][4])
{
   UInt b0=((UInt*)b)[2], b1=((UInt*)b)[3]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC2c (  b0, b1,  block[0][0].c, 4, 4, 0, 0, 4);
   ETCPACK::decompressBlockAlphaC((Byte*)b, &block[0][0].a, 4, 4, 0, 0, 4);
}
#endif

void DecompressBlockETC1(C Byte *b, Color *dest, Int pitch)
{
#if ETC1_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockDiffFlipC(b0, b1, dest->c, pitch/4, 4, 0, 0, 4);
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      FREPD(x, 4)dest[x].a=255;
      dest=(Color*)((Byte*)dest+pitch);
   }
#else
   Color block[4][4];
#if ETC1_DEC==ETC_LIB_RG
   rg_etc1::unpack_etc1_block(b, (UInt*)block);
#elif ETC1_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc1(b, (UInt*)block);
#endif
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      CopyFast(dest, block[y], SIZE(Color)*4);
      dest=(Color*)((Byte*)dest+pitch);
   }
#endif
}
void DecompressBlockETC2(C Byte *b, Color *dest, Int pitch)
{
#if ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC2c(b0, b1, dest->c, pitch/4, 4, 0, 0, 4);
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      FREPD(x, 4)dest[x].a=255;
      dest=(Color*)((Byte*)dest+pitch);
   }
#else
   Color block[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_rgb8(b, (UInt*)block);
#endif
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      CopyFast(dest, block[y], SIZE(Color)*4);
      dest=(Color*)((Byte*)dest+pitch);
   }
#endif
}
void DecompressBlockETC2A1(C Byte *b, Color *dest, Int pitch)
{
#if ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC21BitAlphaC(b0, b1, dest->c, null, pitch/4, 4, 0, 0, 4);
#else
   Color block[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_punchthrough(b, (UInt*)block);
#endif
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      CopyFast(dest, block[y], SIZE(Color)*4);
      dest=(Color*)((Byte*)dest+pitch);
   }
#endif
}
void DecompressBlockETC2A8(C Byte *b, Color *dest, Int pitch)
{
#if ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[2], b1=((UInt*)b)[3]; SwapEndian(b0); SwapEndian(b1); pitch/=4;
   ETCPACK::decompressBlockETC2c (  b0, b1,  dest->c, pitch, 4, 0, 0, 4);
   ETCPACK::decompressBlockAlphaC((Byte*)b, &dest->a, pitch, 4, 0, 0, 4);
#else
   Color block[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_eac(b, (UInt*)block);
#endif
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      CopyFast(dest, block[y], SIZE(Color)*4);
      dest=(Color*)((Byte*)dest+pitch);
   }
#endif
}
/******************************************************************************/
Color DecompressPixelETC1(C Byte *b, Int x, Int y)
{
   Color rgba[4][4];
#if ETC1_DEC==ETC_LIB_RG
   rg_etc1::unpack_etc1_block(b, (UInt*)rgba);
#elif ETC1_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc1(b, (UInt*)rgba);
#elif ETC1_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockDiffFlipC(b0, b1, rgba[0][0].c, 4, 4, 0, 0, 4);
   rgba[y][x].a=255;
#endif
   return rgba[y][x];
}
Color DecompressPixelETC2(C Byte *b, Int x, Int y)
{
   Color rgba[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_rgb8(b, (UInt*)rgba);
#elif ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC2c(b0, b1, rgba[0][0].c, 4, 4, 0, 0, 4);
   rgba[y][x].a=255;
#endif
   return rgba[y][x];
}
Color DecompressPixelETC2A1(C Byte *b, Int x, Int y)
{
   Color rgba[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_punchthrough(b, (UInt*)rgba);
#elif ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[0], b1=((UInt*)b)[1]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC21BitAlphaC(b0, b1, rgba[0][0].c, null, 4, 4, 0, 0, 4);
#endif
   return rgba[y][x];
}
Color DecompressPixelETC2A8(C Byte *b, Int x, Int y)
{
   Color rgba[4][4];
#if ETC2_DEC==ETC_LIB_TEXGENPACK
   TGP::draw_block4x4_etc2_eac(b, (UInt*)rgba);
#elif ETC2_DEC==ETC_LIB_ETCPACK
   UInt b0=((UInt*)b)[2], b1=((UInt*)b)[3]; SwapEndian(b0); SwapEndian(b1);
   ETCPACK::decompressBlockETC2c (  b0, b1,  rgba[0][0].c, 4, 4, 0, 0, 4);
   ETCPACK::decompressBlockAlphaC((Byte*)b, &rgba[0][0].a, 4, 4, 0, 0, 4);
#endif
   return rgba[y][x];
}
/******************************************************************************/
}
/******************************************************************************/
