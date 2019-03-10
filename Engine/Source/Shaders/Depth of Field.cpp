/******************************************************************************/
#include "!Header.h"
/******************************************************************************
TODO: add slow but high quality circular bokeh DoF
   -create small / quarter res (or maybe even smaller) RT containing info about biggest blur radius
   -in the blur function iterate "radius x radius" using BRANCH/LOOP and the small RT image (perhaps need to use 'SamplerPoint')
   -sample intensity should be based on Length2(sample_distance)<=Sqr(sample_range) (making bool true/false)
   -probably should divide this by sample area "intensity / Sqr(sample_range)" (samples covering more areas are stretched and their intensity spreaded?)
/******************************************************************************/
#define SHOW_BLUR        0
#define SHOW_SMOOTH_BLUR 0
#define SHOW_BLURRED     0

#define FRONT_EXTEND 0 // 0 or 1, method for extending the front, default=0
#define DEPTH_TOLERANCE (FRONT_EXTEND ? 1.5f : 1.0f) // 1..2 are reasonable
#define FINAL_MODE  1 // 1(default)=maximize smooth blur only if it's closer, 0=always maximize smooth blur
#define FINAL_SCALE 4.0f // final blur scale, increases transition between sharp and blurred in 0..1/FINAL_SCALE step, instead of 0..1
/******************************************************************************/
BUFFER(Dof)
   Vec4 DofParams; // Intensity, Focus, SimpleMulAdd
BUFFER_END

inline Flt DofIntensity() {return DofParams.x;}
inline Flt DofFocus    () {return DofParams.y;}
inline Flt DofMul      () {return DofParams.z;}
inline Flt DofAdd      () {return DofParams.w;}
/******************************************************************************/
inline Flt Blur(Flt z, uniform Bool realistic)
{
   if(realistic)
   {
   #if 0 // F makes almost no difference
      Flt F=0.075f; return DofIntensity() /* * F */ * (z - DofFocus()) / (z * (DofFocus() - F));
   #else
      return DofIntensity()*(z-DofFocus())/(z*DofFocus()); // 'DofRange' ignored
   #endif
   }else return DofIntensity()*Mid(z*DofMul() + DofAdd(), -1, 1); // (z-DofFocus())/DofRange, z/DofRange - DofFocus()/DofRange
}
/******************************************************************************/
Vec4 DofDS_PS(NOPERSP Vec2 inTex:TEXCOORD,
              uniform Bool do_clamp      ,
              uniform Bool realistic     ,
              uniform Bool half          ,
              uniform Bool gather        ):COLOR
{
   Vec4 ret; // RGB=col, W=Blur
   if(half)
   {
      ret.rgb=TexLod(Col, UVClamp(inTex, do_clamp)).rgb; // use linear filtering because we're downsampling
   #if MODEL>=SM_4
      if(gather)ret.w=DEPTH_MIN(Depth.Gather(SamplerPoint, inTex));else
   #endif
      {
         Vec2 tex_min=inTex-ColSize.xy*0.5f,
              tex_max=inTex+ColSize.xy*0.5f;
         ret.w=DEPTH_MIN(TexDepthRawPoint(Vec2(tex_min.x, tex_min.y)),
                         TexDepthRawPoint(Vec2(tex_max.x, tex_min.y)),
                         TexDepthRawPoint(Vec2(tex_min.x, tex_max.y)),
                         TexDepthRawPoint(Vec2(tex_max.x, tex_max.y)));
      }
   }else // quarter
   {
      Vec2 tex_min=UVClamp(inTex-ColSize.xy, do_clamp),
           tex_max=UVClamp(inTex+ColSize.xy, do_clamp);
      Vec2 t00=Vec2(tex_min.x, tex_min.y),
           t10=Vec2(tex_max.x, tex_min.y),
           t01=Vec2(tex_min.x, tex_max.y),
           t11=Vec2(tex_max.x, tex_max.y);
      // use linear filtering because we're downsampling
      ret.rgb=(TexLod(Col, t00).rgb
              +TexLod(Col, t10).rgb
              +TexLod(Col, t01).rgb
              +TexLod(Col, t11).rgb)/4;
   #if MODEL>=SM_4
      if(gather)ret.w=DEPTH_MIN(DEPTH_MIN(Depth.Gather(SamplerPoint, t00)),
                                DEPTH_MIN(Depth.Gather(SamplerPoint, t10)),
                                DEPTH_MIN(Depth.Gather(SamplerPoint, t01)),
                                DEPTH_MIN(Depth.Gather(SamplerPoint, t11)));else
   #endif
      {
         // this is approximation because we would have to take 16 samples
         ret.w=DEPTH_MIN(TexDepthRawPoint(t00),
                         TexDepthRawPoint(t10),
                         TexDepthRawPoint(t01),
                         TexDepthRawPoint(t11));
      }
   }
   ret.w=Blur(LinearizeDepth(ret.w), realistic)*0.5f+0.5f;
   return ret;
}
/******************************************************************************/
inline Flt Center(Flt center_blur_u) // center_blur_u=0..1 (0.5=focus) here we can offload some of the calculation done for each sample, by making necessary adjustments to 'center_blur_u'
{
   return center_blur_u*2-1;
}
inline Flt Weight(Flt center_blur, Flt test_blur_u, 
   #if MODEL!=SM_GL // have to forget about uniform because it will fail on Mac
      uniform
   #endif
         Int dist, uniform Int range) // center_blur=-1..1 (0=focus), center_blur_u=0..1 (0.5=focus), test_blur_u=0..1 (0.5=focus)
{
   Flt f=dist/Flt(range+1),
     //center_blur=center_blur_u*2-1, // -1..1, we've already done this in 'Center'
         test_blur=  test_blur_u*2-1, // -1..1
      cb=Abs(center_blur), // 0..1
      tb=Abs(  test_blur), // 0..1
       b=Max(tb, cb // have to extend search radius for cases when center is in the front
        *Sat(FRONT_EXTEND ? (tb-center_blur)*DEPTH_TOLERANCE+1 : -center_blur*DEPTH_TOLERANCE)) // skip if: test focused and center in the back, or apply if: center in front
        *Sat((cb-test_blur)*DEPTH_TOLERANCE+1); // skip if: center focused and test in the back

   if(!b)return 0; // this check is needed only for low precision devices, or when using high precision RT's. Low precision unsigned RT's don't have exact zero, however we scale 'b' above and zero could be reached?

   Flt x=f/b; // NaN
   x=Sat(x); // x=Min(x, 1); to prevent for returning 'LerpCube' values outside 0..1 range
   return (1-LerpCube(x))/b; // weight, divide by 'b' to make narrower ranges more intense to preserve total intensity
   // !! if changing from 'LerpCube' to another function then we need to change 'WeightSum' as well !!
}
inline Flt FinalBlur(Flt blur, Flt blur_smooth) // 'blur'=-Inf .. Inf, 'blur_smooth'=0..1
{
   if(SHOW_BLURRED)return 1;
 //blur_smooth=(FINAL_MODE ? Sat(blur_smooth*-2+1) : Abs(blur_smooth*2-1)); already done in 'DofBlurY_PS'
   return Sat(Max(Abs(blur), blur_smooth)*FINAL_SCALE);
}
inline Flt WeightSum(uniform Int range) {return range+1;} // Sum of all weights for all "-range..range" steps, calculated using "Flt weight=0; for(Int dist=-range; dist<=range; dist++)weight+=BlendSmoothCube(dist/Flt(range+1));"
/******************************************************************************/
#define SCALE 0.5f // at the end we need 0 .. 0.5 range, and since we start with 0..1 we need to scale by "0.5"
Vec4 DofBlurX_PS(NOPERSP Vec2 inTex:TEXCOORD,
                 uniform Int  range         ):COLOR
{  //  INPUT: Col: RGB         , Blur
   // OUTPUT:      RGB BlurredX, BlurSmooth

   Vec4 center=TexPoint(Col, inTex);
   Flt  center_blur=Center(center.a),
        weight=0,
        blur_abs=0;
   Vec4 color =0;
   Vec2 t; t.y=inTex.y;
   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      t.x=inTex.x+ColSize.x*i;
      Vec4 c=TexPoint(Col, t);
      Flt  test_blur=c.a,
        #if MODEL==SM_GL
           w=Weight(center_blur, test_blur, (i>=0) ? i : -i, range);
        #else
           w=Weight(center_blur, test_blur, Abs(i), range);
        #endif
      weight  +=w;
      color   +=w*    c;
      blur_abs+=w*Abs(c.a * (2*SCALE) - (1*SCALE)); // SCALE here so we don't have to do it later
   }
   Flt b=Abs(center_blur),
       w=Lerp(WeightSum(range)-weight, 1, b);
   color   +=w*    center;
   blur_abs+=w*Abs(center.a * (2*SCALE) - (1*SCALE)); // SCALE here so we don't have to do it later
   weight  +=w;

   blur_abs/=weight;
 //return Vec4(color.rgb/weight, color.a/weight);
   return Vec4(color.rgb/weight, (color.a>=0.5f*weight) ? 0.5f+blur_abs : 0.5f-blur_abs); // color.a/weight>=0.5f ? .. : ..
}
#undef  SCALE
#define SCALE 1.0f // at the end we need 0..1 range, and since we start with 0..1 we need to scale by "1"
Vec4 DofBlurY_PS(NOPERSP Vec2 inTex:TEXCOORD,
                 uniform Int  range         ):COLOR
{  //  INPUT: Col: RGB BlurredX , BlurSmooth
   // OUTPUT:      RGB BlurredXY, BlurSmooth

   Vec4 center=TexPoint(Col, inTex);
   Flt  center_blur=Center(center.a),
        weight=0,
        blur_abs=0;
   Vec4 color =0;
   Vec2 t; t.x=inTex.x;
   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      t.y=inTex.y+ColSize.y*i;
      Vec4 c=TexPoint(Col, t);
      Flt  test_blur=c.a,
        #if MODEL==SM_GL
           w=Weight(center_blur, test_blur, (i>=0) ? i : -i, range);
        #else
           w=Weight(center_blur, test_blur, Abs(i), range);
        #endif
      weight  +=w;
      color   +=w*    c;
      blur_abs+=w*Abs(c.a * (2*SCALE) - (1*SCALE)); // SCALE here so we don't have to do it later
   }
   Flt b=Abs(center_blur),
       w=Lerp(WeightSum(range)-weight, 1, b);
   color   +=w*    center;
   blur_abs+=w*Abs(center.a * (2*SCALE) - (1*SCALE)); // SCALE here so we don't have to do it later
   weight  +=w;

   blur_abs/=weight;
 //color.a  =((color.a>=0.5f*weight) ? 0.5f+blur_abs : 0.5f-blur_abs); // color.a/weight>=0.5f ? .. : ..
   return Vec4(color.rgb/weight, FINAL_MODE ? ((color.a>=0.5f*weight) ? 0 : blur_abs) : blur_abs);
}
/******************************************************************************/
Vec4 Dof_PS(NOPERSP Vec2 inTex:TEXCOORD,
            NOPERSP PIXEL              ,
            uniform Bool dither        ,
            uniform Bool realistic     ):COLOR
{
   Flt z=TexDepthPoint(inTex),
       b=Blur(z, realistic);
#if SHOW_BLUR
   b=1-Abs(b); return Vec4(b, b, b, 1);
#endif
   Vec4 focus=TexLod(Col , inTex), // can't use 'TexPoint' because 'Col' can be supersampled
        blur =TexLod(Col1, inTex), // use linear filtering because 'Col1' may be smaller RT
        col;
     #if SHOW_SMOOTH_BLUR
        col.rgb=blur.a;
     #else
        col.rgb=Lerp(focus.rgb, blur.rgb, FinalBlur(b, blur.a));
     #endif
        col.a=1; // force full alpha so back buffer effects can work ok
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE    (DofDS    , Draw_VS(), DofDS_PS(false, false, false, false));
TECHNIQUE    (DofDSC   , Draw_VS(), DofDS_PS(true , false, false, false));
TECHNIQUE    (DofDSR   , Draw_VS(), DofDS_PS(false, true , false, false));
TECHNIQUE    (DofDSCR  , Draw_VS(), DofDS_PS(true , true , false, false));
TECHNIQUE    (DofDSH   , Draw_VS(), DofDS_PS(false, false, true , false));
TECHNIQUE    (DofDSCH  , Draw_VS(), DofDS_PS(true , false, true , false));
TECHNIQUE    (DofDSRH  , Draw_VS(), DofDS_PS(false, true , true , false));
TECHNIQUE    (DofDSCRH , Draw_VS(), DofDS_PS(true , true , true , false));
TECHNIQUE_4_1(DofDSG   , Draw_VS(), DofDS_PS(false, false, false, true ));
TECHNIQUE_4_1(DofDSCG  , Draw_VS(), DofDS_PS(true , false, false, true ));
TECHNIQUE_4_1(DofDSRG  , Draw_VS(), DofDS_PS(false, true , false, true ));
TECHNIQUE_4_1(DofDSCRG , Draw_VS(), DofDS_PS(true , true , false, true ));
TECHNIQUE_4_1(DofDSHG  , Draw_VS(), DofDS_PS(false, false, true , true ));
TECHNIQUE_4_1(DofDSCHG , Draw_VS(), DofDS_PS(true , false, true , true ));
TECHNIQUE_4_1(DofDSRHG , Draw_VS(), DofDS_PS(false, true , true , true ));
TECHNIQUE_4_1(DofDSCRHG, Draw_VS(), DofDS_PS(true , true , true , true ));

#define DOF_BLUR(range)   TECHNIQUE(DofBlurX##range, Draw_VS(), DofBlurX_PS(range));   TECHNIQUE(DofBlurY##range, Draw_VS(), DofBlurY_PS(range));
DOF_BLUR(2)
DOF_BLUR(3)
DOF_BLUR(4)
DOF_BLUR(5)
DOF_BLUR(6)
DOF_BLUR(7)
DOF_BLUR(8)
DOF_BLUR(9)
DOF_BLUR(10)
DOF_BLUR(11)
DOF_BLUR(12)

TECHNIQUE(Dof  , Draw_VS(), Dof_PS(false, false));
TECHNIQUE(DofD , Draw_VS(), Dof_PS(true , false));
TECHNIQUE(DofR , Draw_VS(), Dof_PS(false, true ));
TECHNIQUE(DofDR, Draw_VS(), Dof_PS(true , true ));
/******************************************************************************/
