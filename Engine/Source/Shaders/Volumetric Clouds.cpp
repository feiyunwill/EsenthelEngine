/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
#include "Volumetric.h"
/******************************************************************************/
struct VolCloudClass
{
   Flt  AC4_bottom,
        AC4_top   ,
        A2_inv    ,
        max_steps ;
   Vec2 pos       ;
   Vec  pixels    ;
};
struct VolCloudMapClass
{
   Flt  curve, curve2, height, thickness, tex_scale, steps, shadow;
   Vec2 pos;
   Vec  cam;
};
BUFFER(VolCloud)
   VolCloudClass    Cloud;
BUFFER_END
BUFFER(VolCloudMap)
   VolCloudMapClass CloudMap;
BUFFER_END
/******************************************************************************/
// CLOUDS
/******************************************************************************
   clouds = -Cloud.curve*x*x + Cloud.height
   ray    = m*x = dir.y/Length(dir.xz)*x
      m*x = -Cloud.curve*x*x + Cloud.height
      Cloud.curve*x*x + m*x - Cloud.height = 0
    A=Cloud.curve, B=m, C=-Cloud.height
       Delta=B*B - 4*A*C = m*m + 4*Cloud.curve*Cloud.height
   SqrtDelta=Sqrt(m*m + 4*Cloud.curve*Cloud.height)
          x0=(-m-SqrtDelta)/(2*Cloud.curve)
          x1=(-m+SqrtDelta)/(2*Cloud.curve)

          x1=(SqrtDelta-m)/(2*Cloud.curve)
          x1=(SqrtDelta-m)*A2_inv
/******************************************************************************/
void Clouds_VS(VtxInput vtx,
           out Vec  dir   :TEXCOORD,
           out Vec4 outVtx:POSITION)
{
   dir=Transform3(Vec(ScreenToPosXY(vtx.tex()), 1), CamMatrix); // world-space position
   outVtx=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
}
#if DX9
   Vec4
#else
   Vec2
#endif
   Clouds_PS(NOPERSP Vec dir:TEXCOORD):COLOR // 'dir'=world-space position
{
   Flt a=Sat(dir.y*8); // alternatively, 'a' could be calculated as "a=Sat(max_from-from)"
#if FLOW
   BRANCH if(a<=EPS_COL)return 0;
#endif

   dir/=Length(dir.xz);

   Flt from=(Sqrt(dir.y*dir.y + Cloud.AC4_bottom)-dir.y)*Cloud.A2_inv,
       to  =(Sqrt(dir.y*dir.y + Cloud.AC4_top   )-dir.y)*Cloud.A2_inv, delta=to-from;

   Vec pos=Vec( 1, dir.x*from +Cloud.pos.x, dir.z*from +Cloud.pos.y); // correct way would be to start from "1-0.5f/Cloud.pixels.x", however it's really unnoticeable
       dir=Vec(-1, dir.x*delta            , dir.z*delta            );

   Flt pixels=Length(dir*Cloud.pixels);
   Int steps =Mid   (pixels, 2, Cloud.max_steps);
   dir/=steps;

   Vec2 col=0;
   LOOP for(Int i=0; i<steps; i++)
   {
   #if MODEL>=SM_4
      Vec2 sample=Vol.SampleLevel(SamplerLinearCWW, pos, 0).rg;
      /* test code for adding detail if(Z)
      {
         Flt s=0.5f;
         Flt m;
         if(X>=2.5)m=2.0*(1-Sqr(1-sample.y))*s;else
         if(X>=1.5)m=1.1*Sqrt(sample.y)*s;else
         if(X)m=0.5*Sat(sample.y*10)*s;else
                 m=3*sample.y*s;

              sample.y+=(Vol1.SampleLevel(SamplerLinearWrap, pos*S*Z  , 0).r)*m;
         if(Y)sample.y+=(Vol1.SampleLevel(SamplerLinearWrap, pos*S*Z*2, 0).r)*m/2;

         sample.y=Sat(sample.y);
      }*/
   #elif !DX9
      Vec2 sample=Tex3DLod(Vol, pos).rg;
   #else
      Vec2 sample=Tex3DLod(Vol, pos).ra;
   #endif

      Flt alpha=sample.y*(1-col.y);

      col.x+=alpha*sample.x;
      col.y+=alpha;

      pos+=dir;
   }

   col.x/=col.y+EPS; // NaN
   col.y*=a;
#if DX9
   return Vec4(col, 0, 0);
#else
   return col;
#endif
}
/******************************************************************************/
// SHADOW MAP
/******************************************************************************/
void CloudsMap_VS(VtxInput vtx,
              out Vec  outPos:TEXCOORD0,
              out Vec  outDir:TEXCOORD1,
              out Vec4 outVtx:POSITION )
{
   outPos=CloudMap.cam // this is CamMatrix.pos-ActiveCam.matrix.pos (light camera relative to main camera)
         +CamMatrix[0]*(vtx.pos2().x/ProjMatrix[0][0])  // ProjMatrix.x.x which is 1/fov.x
         +CamMatrix[1]*(vtx.pos2().y/ProjMatrix[1][1]); // ProjMatrix.y.y which is 1/fov.y
   outDir=CamMatrix[2];

   outVtx=vtx.pos4(); AdjustPixelCenter(outVtx);
}
#if DX9
   Vec4 
#else
   Flt
#endif
   CloudsMap_PS(NOPERSP Vec pos:TEXCOORD0, // world-space position, relative to main camera
                NOPERSP Vec dir:TEXCOORD1  // world-space direction
               ):COLOR
{
/* clouds = -Cloud.curve*x*x + Cloud.height
   ray    = pos + dir*t

   x = (pos + dir*t).xz.length()   (this is the X as in, distance from cloud Y axis)
   x = (pos.xz + dir.xz*t).length()
   x*x = (pos.xz + dir.xz*t).length2()
   Length2(v) = Sqr(v.x) + Sqr(v.y)
   x*x = Sqr(pos.x + dir.x*t) + Sqr(pos.z + dir.z*t)
   clouds = -Cloud.curve*(Sqr(pos.x + dir.x*t) + Sqr(pos.z + dir.z*t)) + Cloud.height
   pos.y + dir.y*t = -Cloud.curve*(pos.x*pos.x + dir.x*dir.x*t*t + 2*pos.x*dir.x*t   +   pos.z*pos.z + dir.z*dir.z*t*t + 2*pos.z*dir.z*t) + Cloud.height
   pos.y + dir.y*t +  Cloud.curve*(pos.x*pos.x + dir.x*dir.x*t*t + 2*pos.x*dir.x*t   +   pos.z*pos.z + dir.z*dir.z*t*t + 2*pos.z*dir.z*t) - Cloud.height = 0

   Cloud.curve*(dir.x*dir.x + dir.z*dir.z)*t*t + (dir.y+Cloud.curve*2*(pos.x*dir.x + pos.z*dir.z))*t + pos.y+Cloud.curve*(pos.x*pos.x + pos.z*pos.z)-Cloud.height 
   Flt a  =      CloudMap.curve  *(dir.x*dir.x + dir.z*dir.z),
       b  =dir.y+CloudMap.curve*2*(pos.x*dir.x + pos.z*dir.z),
       c  =pos.y+CloudMap.curve  *(pos.x*pos.x + pos.z*pos.z)-CloudMap.height,
       d_u=b*b - 4*a*(c-CloudMap.thickness), // -thickness because 'c' is calculated with -CloudMap.height so we're subtracting
       d_l=b*b - 4*a* c                    ; */
   Flt a2 =      CloudMap.curve2*(dir.x*dir.x + dir.z*dir.z), // this is a*2, which allows better performance
       b  =dir.y+CloudMap.curve2*(pos.x*dir.x + pos.z*dir.z),
       c  =pos.y+CloudMap.curve *(pos.x*pos.x + pos.z*pos.z)-CloudMap.height,
       d_u=b*b - 2*a2*(c-CloudMap.thickness), // here "4*a" was replaced with "2*a2", -thickness because 'c' is calculated with -CloudMap.height so we're subtracting
       d_l=b*b - 2*a2* c                    ; // here "4*a" was replaced with "2*a2"

   BRANCH if(d_u<=0 || d_l<=0)return 1; // 1=full light (zero shadow)

   Flt t_u=(-b - Sqrt(d_u))/a2, // here "(2*a)" was replaced with "a2"
       t_l=(-b - Sqrt(d_l))/a2, // here "(2*a)" was replaced with "a2"
       delta=(t_l-t_u)*CloudMap.tex_scale;

   pos=Vec( 0, (pos.x+dir.x*t_u)*CloudMap.tex_scale+CloudMap.pos.x, (pos.z+dir.z*t_u)*CloudMap.tex_scale+CloudMap.pos.y); // start from upper layer (above clouds)
   dir=Vec( 1,        dir.x*delta                                 ,        dir.z*delta                                 ); // go down                (below clouds)

   Int steps=CloudMap.steps;
   dir/=steps;

   Flt density=0;
   LOOP for(Int i=0; i<steps; i++)
   {
   #if MODEL>=SM_4
      Flt alpha=Vol.SampleLevel(SamplerLinearCWW, pos, 0).g;
   #elif !DX9
      Flt alpha=Tex3DLod(Vol, pos).g;
   #else
      Flt alpha=Tex3DLod(Vol, pos).a;
   #endif

      density+=alpha*(1-density);
      pos    +=dir;
   }
   return 1-density*CloudMap.shadow; // Lerp(1, 1-density, CloudMap.shadow) = 1*(1-CloudMap.shadow) + (1-density)*CloudMap.shadow = 1-CloudMap.shadow+CloudMap.shadow - density*CloudMap.shadow
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void CloudsDraw_VS(VtxInput vtx,
               out Vec2 outTex:TEXCOORD0,
               out Vec  outPos:TEXCOORD1,
               out Vec4 outVtx:POSITION )
{
   outTex=vtx.tex();
   outPos=Vec(ScreenToPosXY(vtx.tex()), 1);
   outVtx=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
}
Vec4 CloudsDraw_PS(NOPERSP Vec2 inTex:TEXCOORD0,
                   NOPERSP Vec  inPos:TEXCOORD1):COLOR
{
   Vec2 clouds=TexLod(Col, inTex).xy; // can't use TexPoint because Col may be smaller
#if 1
   clouds.y*=Sat(TexDepthPoint(inTex)*Length(inPos)*SkyFracMulAdd.x+SkyFracMulAdd.y);
#else
   Vec pos=GetPosPoint(inTex, inPosXY); clouds.y*=Sat(Length(pos)*SkyFracMulAdd.x+SkyFracMulAdd.y);
#endif
   return Vec4(Color[0].rgb*clouds.x, clouds.y);
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE(Clouds    , Clouds_VS    (), Clouds_PS    ());
TECHNIQUE(CloudsMap , CloudsMap_VS (), CloudsMap_PS ());
TECHNIQUE(CloudsDraw, CloudsDraw_VS(), CloudsDraw_PS());
/******************************************************************************/
