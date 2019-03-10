/******************************************************************************/
#include "!Header.h"
#include "Layered Clouds.h"
/******************************************************************************/
void LayeredClouds_VS
(
   VtxInput vtx,

   out Vec  outPos:TEXCOORD0,
   out Vec4 outTex:TEXCOORD1, // xyz=3d tex coord, w=alpha
   out Vec4 outVtx:POSITION 
)
{
   Vec pos=vtx.pos();

   outTex.xyz=pos.xyz*Vec(LCScale, 1, LCScale);
   outTex.w  =CloudAlpha(outTex.y);
      pos.y  =pos.y*LCScaleY+(-LCScaleY+1); // (pos.y-1)*LCScaleY+1
   outVtx=Project(outPos=TransformPos(pos));
}
/******************************************************************************/
Vec4 LayeredClouds_PS(Vec   inPos :TEXCOORD0,
                      Vec4  inTex :TEXCOORD1,
                      PIXEL                 ,
                  out VecH4 outMask:COLOR1  ,
              uniform Int   num             ,
              uniform Bool  blend           ,
              uniform Bool  mask            ):COLOR
{
   Half a=Sat(inTex.w);
   if(blend)
   {
      Flt range=TexDepthPoint(PIXEL_TO_SCREEN)/Normalize(inPos).z; // 0..Viewport.range
          a   *=Sat(range*LCRange.x+LCRange.y);
   }
   Vec2  uv=Normalize(inTex.xyz).xz;
   VecH4 color;
   if(num>=4){Vec4 tex=Tex(Col3, uv*CL[3].scale + CL[3].position)*CL[3].color; if(num==4)color=tex;else color=Lerp(color, tex, tex.a);}
   if(num>=3){Vec4 tex=Tex(Col2, uv*CL[2].scale + CL[2].position)*CL[2].color; if(num==3)color=tex;else color=Lerp(color, tex, tex.a);}
   if(num>=2){Vec4 tex=Tex(Col1, uv*CL[1].scale + CL[1].position)*CL[1].color; if(num==2)color=tex;else color=Lerp(color, tex, tex.a);}
   if(num>=1){Vec4 tex=Tex(Col , uv*CL[0].scale + CL[0].position)*CL[0].color; if(num==1)color=tex;else color=Lerp(color, tex, tex.a);}

   color.a*=a;
   if(mask)
   {
      outMask.rgb=0;
      outMask.a  =Sat(color.a*LCMaskContrast.x+LCMaskContrast.y); // (color.a-0.5)*LCMaskContrast+0.5
   }
   return color;
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE(Clouds1  , LayeredClouds_VS(), LayeredClouds_PS(1, false, false));
TECHNIQUE(Clouds2  , LayeredClouds_VS(), LayeredClouds_PS(2, false, false));
TECHNIQUE(Clouds3  , LayeredClouds_VS(), LayeredClouds_PS(3, false, false));
TECHNIQUE(Clouds4  , LayeredClouds_VS(), LayeredClouds_PS(4, false, false));
TECHNIQUE(Clouds1B , LayeredClouds_VS(), LayeredClouds_PS(1, true , false));
TECHNIQUE(Clouds2B , LayeredClouds_VS(), LayeredClouds_PS(2, true , false));
TECHNIQUE(Clouds3B , LayeredClouds_VS(), LayeredClouds_PS(3, true , false));
TECHNIQUE(Clouds4B , LayeredClouds_VS(), LayeredClouds_PS(4, true , false));
TECHNIQUE(Clouds1M , LayeredClouds_VS(), LayeredClouds_PS(1, false, true ));
TECHNIQUE(Clouds2M , LayeredClouds_VS(), LayeredClouds_PS(2, false, true ));
TECHNIQUE(Clouds3M , LayeredClouds_VS(), LayeredClouds_PS(3, false, true ));
TECHNIQUE(Clouds4M , LayeredClouds_VS(), LayeredClouds_PS(4, false, true ));
TECHNIQUE(Clouds1BM, LayeredClouds_VS(), LayeredClouds_PS(1, true , true ));
TECHNIQUE(Clouds2BM, LayeredClouds_VS(), LayeredClouds_PS(2, true , true ));
TECHNIQUE(Clouds3BM, LayeredClouds_VS(), LayeredClouds_PS(3, true , true ));
TECHNIQUE(Clouds4BM, LayeredClouds_VS(), LayeredClouds_PS(4, true , true ));
/******************************************************************************/
