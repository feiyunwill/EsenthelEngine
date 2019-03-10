/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
/******************************************************************************/
BUFFER(VolLight)
   Vec VolMax=Vec(1, 1, 1);
   Flt Light_point_range;
BUFFER_END
/******************************************************************************/
Vec4 VolDir_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
               NOPERSP Vec2 inPosXY:TEXCOORD1,
               NOPERSP PIXEL                 ,
               uniform Int  num              ,
               uniform Bool cloud            ):COLOR
{
   Vec obj   =GetPosLinear(inTex, inPosXY); // use linear filtering because we may be drawing to a smaller RT
   Flt power =0,
       length=Length(obj);
   if( length>ShdRange)
   {
      obj  *=ShdRange/length;
      length=ShdRange;
   }

   VecH2 jitter_value=ShadowJitter(pixel.xy);

   Int  steps=80;
   LOOP for(Int i=0; i<steps; i++)
   {
      Vec pos=ShadowDirTransform(obj*(Flt(i+1)/(steps+1)), num);
      if(cloud)power+=CompareDepth(pos, jitter_value, true)*CompareDepth2(pos);
      else     power+=CompareDepth(pos, jitter_value, true);
   }

   power =Pow(power /steps   , Light_dir.vol_exponent_steam.y);
   power*=Pow(length/ShdRange, Light_dir.vol_exponent_steam.y*(1-Light_dir.vol_exponent_steam.z));
   power*=Light_dir.vol_exponent_steam.x;
   return Vec4(Light_dir.color.rgb*power, 0);
}
/******************************************************************************/
Vec4 VolPnt_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
               NOPERSP Vec2 inPosXY:TEXCOORD1,
               NOPERSP PIXEL                 ):COLOR
{
   Vec obj   =GetPosLinear(inTex, inPosXY); // use linear filtering because we may be drawing to a smaller RT
   Flt power =0,
       length=Length(obj);
   if( length>Viewport.range)
   {
      obj  *=Viewport.range/length;
      length=Viewport.range;
   }
   Vec from =ShdMatrix[3],
       to   =Transform(obj, ShdMatrix);

   VecH2 jitter_value=ShadowJitter(pixel.xy);

   Int steps=48;

   LOOP for(Int i=0; i<steps; i++)
   {
      // TODO: optimize
      Vec pos=Lerp(from, to, Flt(i)/Flt(steps));
      power+=ShadowPointValue(obj*(Flt(i)/steps), jitter_value, true)*LightSqrDist(pos, Light_point_range);
   }
   return Vec4(Light_point.color.rgb*Min(Light_point.vol_max, Light_point.vol*power*(length/steps)), 0);
}
/******************************************************************************/
Vec4 VolSqr_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
               NOPERSP Vec2 inPosXY:TEXCOORD1,
               NOPERSP PIXEL                 ):COLOR
{
   Vec obj   =GetPosLinear(inTex, inPosXY); // use linear filtering because we may be drawing to a smaller RT
   Flt power =0,
       length=Length(obj);
   if( length>Viewport.range)
   {
      obj  *=Viewport.range/length;
      length=Viewport.range;
   }
   Vec from =ShdMatrix[3],
       to   =Transform(obj, ShdMatrix);
   Int steps=48;

   VecH2 jitter_value=ShadowJitter(pixel.xy);

   LOOP for(Int i=0; i<steps; i++)
   {
      // TODO: optimize
      Vec pos=Lerp(from, to, Flt(i)/Flt(steps));
      power+=ShadowPointValue(obj*(Flt(i)/steps), jitter_value, true)*LightSqrDist(pos);
   }
   return Vec4(Light_sqr.color.rgb*Min(Light_sqr.vol_max, Light_sqr.vol*power*(length/steps)), 0);
}
/******************************************************************************/
Vec4 VolCone_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                NOPERSP Vec2 inPosXY:TEXCOORD1,
                NOPERSP PIXEL                 ):COLOR
{
   Vec obj   =GetPosLinear(inTex, inPosXY), // use linear filtering because we may be drawing to a smaller RT
       scale =Vec(Light_cone.scale, Light_cone.scale, 1);
   Flt power =0,
       length=Length(obj);
   if( length>Viewport.range)
   {
      obj  *=Viewport.range/length;
      length=Viewport.range;
   }
   Vec from =ShdMatrix[3],
       to   =Transform(obj, ShdMatrix);
   Int steps=48;

   VecH2 jitter_value=ShadowJitter(pixel.xy);

   LOOP for(Int i=0; i<steps; i++)
   {
      // TODO: optimize
      Vec pos=Lerp(from, to, Flt(i)/Flt(steps));
      Flt cur=Max(Abs(pos));
      if( pos.z>=cur)
      {
         power+=ShadowConeValue(obj*(Flt(i)/steps), jitter_value, true)*LightConeDist(pos*scale)*LightConeAngle(pos.xy/pos.z);
      }
   }
   return Vec4(Light_cone.color.rgb*Min(Light_cone.vol_max, Light_cone.vol*power*(length/steps)), 0);
}
/******************************************************************************/
Vec4 Volumetric_PS(NOPERSP Vec2 inTex:TEXCOORD,
                   uniform Bool add           ,
                   uniform Int  samples=6     ):COLOR
{
   Vec vol=TexLod(Col, inTex).rgb; // use linear filtering because Col may be smaller

   UNROLL for(Int i=0; i<samples; i++)
   {
      Vec2 t;
      if(samples== 4)t=ColSize.xy*BlendOfs4 [i]+inTex;
    //if(samples== 5)t=ColSize.xy*BlendOfs5 [i]+inTex;
      if(samples== 6)t=ColSize.xy*BlendOfs6 [i]+inTex;
      if(samples== 8)t=ColSize.xy*BlendOfs8 [i]+inTex;
    //if(samples== 9)t=ColSize.xy*BlendOfs9 [i]+inTex;
      if(samples==12)t=ColSize.xy*BlendOfs12[i]+inTex;
    //if(samples==13)t=ColSize.xy*BlendOfs13[i]+inTex;

      vol+=TexLod(Col, t).rgb; // use linear filtering because Col may be smaller and texcoords are not rounded
   }
   vol/=samples+1;
   vol =Min(vol, VolMax);

   if(add)return Vec4(vol, 0);                                 // alpha blending : ALPHA_ADD
   else   {Flt max=Max(vol); return Vec4(vol/(EPS+max), max);} // alpha blending : ALPHA_BLEND_DEC
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE(VolDir1, DrawPosXY_VS(), VolDir_PS(1, false));   TECHNIQUE(VolDir1C, DrawPosXY_VS(), VolDir_PS(1, true));
TECHNIQUE(VolDir2, DrawPosXY_VS(), VolDir_PS(2, false));   TECHNIQUE(VolDir2C, DrawPosXY_VS(), VolDir_PS(2, true));
TECHNIQUE(VolDir3, DrawPosXY_VS(), VolDir_PS(3, false));   TECHNIQUE(VolDir3C, DrawPosXY_VS(), VolDir_PS(3, true));
TECHNIQUE(VolDir4, DrawPosXY_VS(), VolDir_PS(4, false));   TECHNIQUE(VolDir4C, DrawPosXY_VS(), VolDir_PS(4, true));
TECHNIQUE(VolDir5, DrawPosXY_VS(), VolDir_PS(5, false));   TECHNIQUE(VolDir5C, DrawPosXY_VS(), VolDir_PS(5, true));
TECHNIQUE(VolDir6, DrawPosXY_VS(), VolDir_PS(6, false));   TECHNIQUE(VolDir6C, DrawPosXY_VS(), VolDir_PS(6, true));

TECHNIQUE(VolPnt , DrawPosXY_VS(), VolPnt_PS ());
TECHNIQUE(VolSqr , DrawPosXY_VS(), VolSqr_PS ());
TECHNIQUE(VolCone, DrawPosXY_VS(), VolCone_PS());

TECHNIQUE(Volumetric , Draw_VS(), Volumetric_PS(false));
TECHNIQUE(VolumetricA, Draw_VS(), Volumetric_PS(true ));
/******************************************************************************/
