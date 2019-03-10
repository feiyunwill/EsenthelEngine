/******************************************************************************/
#include "!Header.h"
#include "Hdr.h"

#define SIMPLE    1 // mode 0 is not fully developed
#define BRIGHT    1 // if apply adjustment for scenes where half pixels are bright, and other half are dark, in that case prefer focus on brighter, to avoid making already bright pixels too bright
#define GEOMETRIC 0 // don't use geometric mean, because of cases when bright sky is mostly occluded by dark objects, then entire scene will get brighter, making the sky look too bright and un-realistic
/******************************************************************************/
// HDR
/******************************************************************************/
Vec4 HdrDS_PS(NOPERSP Vec2 inTex:TEXCOORD,
              uniform Int  step          ):COLOR
{
   Vec2 tex_min=inTex-ColSize.xy,
        tex_max=inTex+ColSize.xy;
   if(step==0)
   {
      // use linear filtering because we're downsampling
      Vec sum=TexLod(Col, Vec2(tex_min.x, tex_min.y)).rgb
             +TexLod(Col, Vec2(tex_max.x, tex_min.y)).rgb
             +TexLod(Col, Vec2(tex_min.x, tex_max.y)).rgb
             +TexLod(Col, Vec2(tex_max.x, tex_max.y)).rgb;
      Flt lum;
   #if SIMPLE
      lum=Max(sum*HdrWeight);
   #else
      // fast approximation, more precise but slower would be to call 'SRGBToLinear' for every pixel, or mark that 'Col' is a SRGB texture for step==0
      Vec lin=SRGBToLinear(sum*0.25f);
      lum=Dot(lin, ColorLumWeight2); // this gives us 'LinearLumOfSRGBColor'
   #endif
   #if BRIGHT
      lum=Sqr(lum);
   #endif
   #if GEOMETRIC
      lum=log2(Max(lum, EPS)); // NaN
   #endif
      return lum;
   }else
   {
      // use linear filtering because we're downsampling
      return Avg(TexLod(Col, Vec2(tex_min.x, tex_min.y)).x,
                 TexLod(Col, Vec2(tex_max.x, tex_min.y)).x,
                 TexLod(Col, Vec2(tex_min.x, tex_max.y)).x,
                 TexLod(Col, Vec2(tex_max.x, tex_max.y)).x);
   }
}
/******************************************************************************/
Vec4 HdrUpdate_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Flt lum=TexPoint(Col, Vec2(0, 0)).x;
#if GEOMETRIC
   lum=exp2(lum); // we've applied 'log2' above, so revert it back
#endif
#if BRIGHT
   lum=Sqrt(lum); // we've applied 'Sqr' above, so revert it back
#endif
   lum=HdrBrightness/Max(lum, EPS_COL); // desired scale
   lum=Mid(lum, HdrMaxDark, HdrMaxBright);
   return Lerp(lum, TexPoint(Lum, Vec2(0, 0)).x, Step);
}
/******************************************************************************/
Vec4 Hdr_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec4 col=TexLod  (Col, inTex); // can't use 'TexPoint' because 'Col' can be supersampled
   Flt  lum=TexPoint(Lum, Vec2(0, 0)).x;
#if SIMPLE
   col.rgb*=lum;
#else
   col.rgb=LinearToSRGB(SRGBToLinear(col.rgb)*lum);
#endif
   return col;
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE(HdrDS0, Draw_VS(), HdrDS_PS(0));
TECHNIQUE(HdrDS1, Draw_VS(), HdrDS_PS(1));

TECHNIQUE(HdrUpdate, Draw_VS(), HdrUpdate_PS());
TECHNIQUE(Hdr      , Draw_VS(),       Hdr_PS());
/******************************************************************************/
