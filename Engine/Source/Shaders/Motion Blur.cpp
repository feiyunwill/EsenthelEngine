/******************************************************************************/
#include "!Header.h"
/******************************************************************************

   TexCoord.x=0 -> Left, TexCoord.x=1 -> Right
   TexCoord.y=0 -> Up  , TexCoord.y=1 -> Down

   Warning: because max_dir_length is encoded in Signed RT Z channel without MAD range adjustment, it loses 1 bit precision

/******************************************************************************/
#define      MAX_BLUR_SAMPLES 7
#define VARIABLE_BLUR_SAMPLES 0 // 0=is much faster for all 3 cases (no/small/full blur), default=0
#define     TEST_BLUR_PIXELS  0 // test what pixels actually get blurred (they will be set to RED color) use only for debugging
#define ALWAYS_DILATE_LENGTH  0 // 0=gave better results (when camera was rotated slightly), default=0
#define                ROUND  1 // if apply rounding (makes smaller res buffers look more similar to full res), default=1
/******************************************************************************/
BUFFER(MotionBlur)
   Vec4 MotionUVMulAdd;
   Vec4 MotionVelScaleLimit;
   Vec2 MotionPixelSize;
BUFFER_END
/******************************************************************************/
#define DEPTH_TOLERANCE 0.1f // 10 cm
inline Flt DepthBlend(Flt z_test, Flt z_base)
{
   return Sat((z_base-z_test)/DEPTH_TOLERANCE+1); // we can apply overlap only if tested depth is smaller
}
inline Bool InFront(Flt z_test, Flt z_base)
{
   return z_test<=z_base+DEPTH_TOLERANCE; // if 'z_test' is in front of 'z_base' with DEPTH_TOLERANCE
}
/******************************************************************************/
void Explosion_VS(VtxInput vtx,
              out Vec  outPos:TEXCOORD0,
              out Vec  outVel:TEXCOORD1,
              out Vec4 outVtx:POSITION )
{
   outVel=mul((Matrix3)CamMatrix, Normalize(vtx.pos())*Step);
   outPos=TransformPos(vtx.pos());
   outVtx=Project     ( outPos  );
}
void Explosion_PS(Vec  inPos:TEXCOORD0,
                  Vec  inVel:TEXCOORD1,
           #if MODEL==SM_3
              out Vec4 outCol:COLOR0,
           #endif
              out Vec4 outVel:COLOR1) // #BlendRT
{
   // there's no NaN because inPos.z is always >0 in PixelShader
   inVel/=inPos.z;
#if !SIGNED_VEL_RT
   inVel=inVel*0.5f+0.5f;
#endif
   outVel.xyz=inVel.xyz;
   outVel.w  =0;
#if MODEL==SM_3
   outCol=0; // must write on DX9 otherwise compilation will fail
#endif
}
/******************************************************************************/
void ClearSkyVel_VS(VtxInput vtx,
                out Vec4 outVel:TEXCOORD,
                out Vec4 outVtx:POSITION)
{
   Vec pos=Vec(ScreenToPosXY(vtx.tex()), 1); // we shouldn't normalize this vector, instead, we should keep it at Z=1 so we don't have to divide by Z later

   outVel.xyz=Cross(pos, CamAngVel);
#if !SIGNED_VEL_RT
   outVel.xyz=outVel.xyz*0.5f+0.5f;
#endif

   outVel.w=0;
   outVtx  =Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
}
Vec4 ClearSkyVel_PS(Vec4 inVel:TEXCOORD):COLOR {return inVel;} // yes, per-vertex precision is enough, as it generates the same results as if drawing a half sky ball mesh (results with the half ball mesh were the same as the one from this pixel shader)
TECHNIQUE(ClearSkyVel, ClearSkyVel_VS(), ClearSkyVel_PS());
/******************************************************************************/
void Convert_VS(VtxInput vtx,
            out Vec2 outTex  :TEXCOORD0,
            out Vec2 outPos  :TEXCOORD1, // position relative to viewport center scaled from UV to ScreenPos
            out Vec2 outPosXY:TEXCOORD2,
            out Vec4 outVtx  :POSITION ,
        uniform Int  mode              )
{
              outTex  =vtx.tex();
              outPos  =outTex*MotionUVMulAdd.xy+MotionUVMulAdd.zw;
   if(mode==0)outPosXY=ScreenToPosXY(outTex);
              outVtx  =vtx.pos4(); AdjustPixelCenter(outVtx);
}
Vec4 Convert_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                NOPERSP Vec2 inPos  :TEXCOORD1, // position relative to viewport center scaled from UV to ScreenPos
                NOPERSP Vec2 inPosXY:TEXCOORD2,
                uniform Int  mode             ,
                uniform Bool do_clamp         ,
                uniform Int  pixels=MAX_MOTION_BLUR_PIXEL_RANGE):COLOR
{
   Vec blur;
   if(mode==0)
   {
      Vec pos=(do_clamp ? GetPosLinear(UVClamp(inTex, do_clamp)) : GetPosLinear(inTex, inPosXY));
      blur=GetVelocitiesCameraOnly(pos);
   }else
   if(mode==1)
   {
      blur=TexLod(Col, UVClamp(inTex, do_clamp)).xyz; // have to use linear filtering because we may draw to smaller RT
   #if !SIGNED_VEL_RT // convert 0..1 -> -1..1 (*2-1) and fix zero, unsigned texture formats don't have a precise zero when converted to signed, because both 127/255*2-1 and 128/255*2-1 aren't zeros, 127: (127/255-0.5)==-0.5/255, 128: (128/255-0.5)==0.5/255, 129: (129/255-0.5)==1.5/255, so let's compare <=1.0/255
      blur=((Abs(blur-0.5f)<=1.0f/255) ? Vec(0, 0, 0) : blur*2-1); // this performs comparisons for all channels separately, force 0 when source value is close to 0.5, otherwise scale to -1..1
   #endif
   }

   // see "C:\Users\Greg\SkyDrive\Code\Tests\Motion Blur.cpp"
   // the following generates 2 delta vectors in screen space, they have different lengths (when blur.z!=0) due to perspective correction, one points towards center, and one away from center
   blur*=MotionVelScaleLimit.xyz;
   Vec4 delta=Vec4((inPos+blur.xy)/(1+blur.z)-inPos,  // #1 is along   blur vector (delta.xy)
                   (inPos-blur.xy)/(1-blur.z)-inPos); // #2 is against blur vector (delta.zw)

   // store only one direction and its length
   // delta.xy.setLength(Avg(delta.xy.length(), delta.zw.length()))
   Flt len0=Length(delta.xy), len1=Length(delta.zw), len=Min(Avg(len0, len1), MotionVelScaleLimit.w); // alternatively we could use "Max" instead of "Avg"
   if( len0)delta.xy*=(len + ROUND*0.5f/pixels)/len0; // NaN, add half pixel length which will trigger rounding effect
            delta.z  = len; // here don't include rounding

#if !SIGNED_VEL_RT
   delta.xy=delta.xy*0.5f+0.5f; // scale XY -1..1 -> 0..1, but leave Z in 0..1
#endif
   return delta;
}
/******************************************************************************/
Vec4 Dilate_PS(NOPERSP Vec2 inTex:TEXCOORD                    ,
               uniform Int  range=1                           ,
               uniform Int  pixels=MAX_MOTION_BLUR_PIXEL_RANGE,
               uniform Bool depth=false                       ):COLOR
{
   Vec4 blur;
   blur.xyz=TexPoint(Col, inTex).xyz;
   blur.w=0;
#if !SIGNED_VEL_RT
   blur.xy=blur.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
#endif
   Flt len=Length(blur.xy), // can't use 'blur.z' as length here, because it's max length of all nearby pixels and not just this pixel
       z_base; if(depth)z_base=TexDepthLinear(inTex); // use linear filtering because RT can be smaller
   // this function iterates nearby pixels and calculates how much the should blur over this one (including angle between pixels, distances, and their blur velocities)
   UNROLL for(Int y=-range; y<=range; y++)
   UNROLL for(Int x=-range; x<=range; x++)if(x || y)
   {
      Vec2 t=inTex+Vec2(x, y)*ColSize.xy;
      Vec  b=TexPoint(Col, t).xyz;
   #if !SIGNED_VEL_RT
      b.xy=b.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
   #endif

   #if 1 // version that multiplies length by cos between pixels (faster and more accurate for diagonal blurs, however bloats slightly)
      Flt l=Abs(Dot(b.xy, Normalize(Vec2(x, y)))); // we want this to be proportional to 'b.xy' length
      if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
      l-=Dist(x, y)/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
      if(l>len){blur.xy=b.xy*(l/Length(b.xy)); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;}
   #else // version that does line intersection tests (slower)
      Flt b_len=Length(b.xy), // can't use 'b.z' as length here, because it's max length of all nearby pixels and not just this pixel
          l=b_len; if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
      l-=Dist(x, y)/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
      if(l>len)
      {
         const Flt eps=0.7f; // 1.0f would include neighbors too, and with each call to this function we would bloat by 1 pixel, this needs to be slightly below SQRT2_2 0.7071067811865475 to avoid diagonal bloating too
         Flt line_dist=Dot(b.xy, Vec2(y, -x)); if(Abs(line_dist)<=b_len*eps) // Vec2 perp_n=Vec2(b.y, -b.x)/b_len; Flt line_dist=Dot(perp_n, Vec2(x, y)); if(Abs(line_dist)<=eps)..   (don't try to do "Vec2(y, -x)/eps" instead of "b_len*eps", because compiler can optimize the Dot better without it)
         {blur.xy=b.xy*(l/b_len); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;}
      }
   #endif

      if(ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z);
   }

#if !SIGNED_VEL_RT
   blur.xy=blur.xy*0.5f+0.5f; // scale XY -1..1 -> 0..1, but leave Z in 0..1
#endif
   return blur;
}
/******************************************************************************/
Vec4 DilateX_PS(NOPERSP Vec2 inTex:TEXCOORD                    ,
                uniform Int  range                             ,
                uniform Bool diagonal=false                    ,
                uniform Int  pixels=MAX_MOTION_BLUR_PIXEL_RANGE,
                uniform Bool depth=false                       ):COLOR
{
   Vec4 blur=TexPoint(Col, inTex); // XY=Dir, Z=Max Dir length of all nearby pixels
#if !SIGNED_VEL_RT
   blur.xy=blur.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
#endif
   Flt  len=Length(blur.xy), // can't use 'blur.z' as length here, because it's max length of all nearby pixels and not just this pixel
        z_base; if(depth)z_base=TexDepthLinear(inTex); // use linear filtering because RT can be smaller
   Vec2 t; t.y=inTex.y;

   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      t.x=inTex.x+ColSize.x*i;
      Vec b=TexPoint(Col, t).xyz;
   #if !SIGNED_VEL_RT
      b.xy=b.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
   #endif

      Flt ll=Abs(b.x), l=ll;
      if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
      l-=Abs(Flt(i))/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
      if(l>len){blur.xy=b.xy*(l/(diagonal ? Length(b.xy) : ll)); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;} // when not doing diagonal then use 'll' to artificially boost intensity, because it helps in making it look more like "Dilate"

      if(ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z);
   }

   if(diagonal)
   {
      range=Round(range*SQRT2_2);
      UNROLL for(Int i=-range; i<=range; i++)if(i)
      {
         t=inTex+ColSize.xy*i;
         Vec b=TexPoint(Col, t).xyz;
      #if !SIGNED_VEL_RT
         b.xy=b.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
      #endif

         Flt l=Abs(Dot(b.xy, Vec2(SQRT2_2, SQRT2_2)));
         if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
         l-=Abs(i*SQRT2)/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
         if(l>len){blur.xy=b.xy*(l/Length(b.xy)); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;}

         if(ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z);
      }
   }

#if !SIGNED_VEL_RT
   blur.xy=blur.xy*0.5f+0.5f; // scale XY -1..1 -> 0..1, but leave Z in 0..1
#endif
   return blur;
}
/******************************************************************************/
Vec4 DilateY_PS(NOPERSP Vec2 inTex:TEXCOORD                    ,
                uniform Int  range                             ,
                uniform Bool diagonal=false                    ,
                uniform Int  pixels=MAX_MOTION_BLUR_PIXEL_RANGE,
                uniform Bool depth=false                       ):COLOR
{
   Vec4 blur=TexPoint(Col, inTex); // XY=Dir, Z=Max Dir length of all nearby pixels
#if !SIGNED_VEL_RT
   blur.xy=blur.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
#endif
   Flt  len=Length(blur.xy), // can't use 'blur.z' as length here, because it's max length of all nearby pixels and not just this pixel
        z_base; if(depth)z_base=TexDepthLinear(inTex); // use linear filtering because RT can be smaller
   Vec2 t; t.x=inTex.x;

   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      t.y=inTex.y+ColSize.y*i;
      Vec b=TexPoint(Col, t).xyz;
   #if !SIGNED_VEL_RT
      b.xy=b.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
   #endif

      Flt ll=Abs(b.y), l=ll;
      if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
      l-=Abs(Flt(i))/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
      if(l>len){blur.xy=b.xy*(l/(diagonal ? Length(b.xy) : ll)); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;} // when not doing diagonal then use 'll' to artificially boost intensity, because it helps in making it look more like "Dilate"

      if(ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z);
   }

   if(diagonal)
   {
      range=Round(range*SQRT2_2);
      UNROLL for(Int i=-range; i<=range; i++)if(i)
      {
         t=inTex+ColSize.xy*Vec2(i, -i);
         Vec b=TexPoint(Col, t).xyz;
      #if !SIGNED_VEL_RT
         b.xy=b.xy*2-1; // scale XY 0..1 -> -1..1, but leave Z in 0..1
      #endif

         Flt l=Abs(Dot(b.xy, Vec2(SQRT2_2, -SQRT2_2)));
         if(depth)l*=DepthBlend(TexDepthLinear(t), z_base); // use linear filtering because RT can be smaller
         l-=Abs(i*SQRT2)/pixels; // when loop is unrolled and 'pixels' is uniform then it can be evaluated to a constant expression
         if(l>len){blur.xy=b.xy*(l/Length(b.xy)); if(!ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z); len=l;}

         if(ALWAYS_DILATE_LENGTH)blur.z=Max(blur.z, b.z);
      }
   }

#if !SIGNED_VEL_RT
   blur.xy=blur.xy*0.5f+0.5f; // scale XY -1..1 -> 0..1, but leave Z in 0..1
#endif
   return blur;
}
/******************************************************************************/
Vec4 SetDirs_PS(NOPERSP Vec2 inTex:TEXCOORD, // goes simultaneously in both ways from starting point and notices how far it can go, travelled distance is put into texture
                uniform Bool do_clamp      ,
                uniform Int  pixels=MAX_MOTION_BLUR_PIXEL_RANGE):COLOR
{
   // Input: Col  - pixel   velocity
   //        Col1 - dilated velocity (main blur direction)

   Vec blur_dir=TexPoint(Col1, inTex).xyz; // XY=Dir, Z=Max Dir length of all nearby pixels
#if !SIGNED_VEL_RT
   blur_dir.xy=((Abs(blur_dir.xy-0.5f)<=1.0f/255) ? Vec2(0, 0) : blur_dir.xy*2-1); // this performs comparisons for all channels separately, force 0 when source value is close to 0.5, otherwise scale to -1..1
#endif

   Vec4 dirs=0;

#if 0 && !ALWAYS_DILATE_LENGTH // when we don't always dilate the length, then we could check it here instead of 'blur_dir.xy' length, performance results were similar, at the moment it's not sure which version is better
   BRANCH if(blur_dir.z>0)
   {
         Flt blur_dir_length2=Length2(blur_dir.xy);
#else
         Flt blur_dir_length2=Length2(blur_dir.xy);
   BRANCH if(blur_dir_length2>Sqr(0.5f/pixels)) // Length(blur_dir.xy)*pixels>0.5f. Check 'blur_dir.xy' instead of 'blur_dir.z' because can be big (because of nearby pixels moving) but 'blur_dir.xy' can be zero. Always check for 0.5 regardless of ROUND (this was tested and it looks much better)
   {
#endif
      /*
         This algorithm works by going in 2 directions, starting from the center, directions are opposite:
            left <--- center ---> right
         Left and Right are just examples, because directions can go in any way (up/down, diagonal, etc)
         This function needs to calculate the length of those directions - how much we want to travel in each direction.
         Those vectors will be used in the final blur, inside it, all pixels along the way will be included.
         So we need to choose the lengths carefully, to avoid blurring with non-moving objects that are in front of us.
         At the start we take the movement velocity of the starting center pixel, to see how much it should blur.
         Along the way we test if there are other moving pixels, and if they overlap with the starting center position,
         if so, we extend blur length up to that pixel, but not further. We also check depth values of encountered pixels,
         and calculate the minimum of them in each direction, to know if they will form an obstacle, that will block next pixels.
         We also allow a special case when all pixels move in the same direction (this happens when rotating the camera).
         In this loop we can't break, because even if we encounter some obstacles blocking pixels, we have to keep going,
         because we may find a pixel later, that is not blocked by any obstacles and overlaps the center.
      */

      // calculate both directions
      dirs.xy= blur_dir.xy*MotionPixelSize;
      dirs.zw=-blur_dir.xy*MotionPixelSize;
   #if 0 // no need to do clamping here because we do it anyway below for the final dirs
      if(do_clamp)
      {
         dirs.xy=UVClamp(inTex+dirs.xy, do_clamp)-inTex; // first calculate target and clamp it
         dirs.zw=UVClamp(inTex+dirs.zw, do_clamp)-inTex; // first calculate target and clamp it
      }
   #endif
      Flt  blur_dir_length=Sqrt(blur_dir_length2);
      Vec2 blur_dir_normalized=blur_dir.xy/blur_dir_length;
      Flt  pixel_range=blur_dir_length*pixels; dirs/=pixel_range; // 'dirs' is now 1 pixel length (we need this length, because we compare blur lengths/distances to "Int i" step)
      Int  range0=0, range1=0;

      Vec2 pixel_vel=TexPoint(Col, inTex).xy;
   #if !SIGNED_VEL_RT
      pixel_vel=pixel_vel*2-1;
   #endif
      Flt length0=Abs(Dot(pixel_vel, blur_dir_normalized))*pixels, length1=length0, // how many pixels in left and right this pixel wants to move starting from the center. We want this to be proportional to 'pixel_vel' length, so don't normalize. This formula gives us how much this pixel wants to travel along the main blur direction
          depth0=TexDepthLinear(inTex), depth1=depth0, // depth values of left and right pixels with most movement, use linear filtering because RT can be smaller
          depth_min0=depth0, depth_min1=depth1, // minimum of encountered left and right depth values
          same_vel0=true, same_vel1=true; // if all encountered so far in this direction have the same velocity (this is to support cases where pixels move in the same direction but have diffent positions, for example when rotating the camera)
      Int allowed0=0, allowed1=0;

      Vec2 t0=inTex, t1=inTex;
   #if 0 // slower
      Int samples=pixels; UNROLL for(Int i=1; i<=samples; i++)
   #elif MODEL==SM_GL
      Int samples=Round(blur_dir.z*pixels); LOOP for(Int i=1; i<=samples; i++) // GL version breaks when 'Ceil' is used so for GL we always have to use 'Round'
   #else
      Int samples=Round(blur_dir.z*pixels); LOOP for(Int i=1; i<=samples; i++)
   #endif
      {
         t0+=dirs.xy;
         t1+=dirs.zw;
         Vec2 c0=TexLod(Col, t0).xy, c1=TexLod(Col, t1).xy; // use linear filtering because texcoords are not rounded
      #if !SIGNED_VEL_RT
         c0=c0*2-1;
         c1=c1*2-1;
      #endif
         Flt z0=TexDepthLinear(t0), z1=TexDepthLinear(t1); // use linear filtering because RT can be smaller and because texcoords are not rounded

      #if 0
         Flt l0=Length(c0)*pixels, l1=Length(c1)*pixels;
      #else
         Flt l0=Abs(Dot(c0, blur_dir_normalized))*pixels, l1=Abs(Dot(c1, blur_dir_normalized))*pixels;
      #endif

       //if(InFront(z0, depth_min0)) // if this sample is in front of all encountered so far in this direction, this check isn't needed because we do depth tests either way below
            if(l0>=i // this sample movement reaches the center
            && i>length0) // and it extends blurring to the left, compared to what we already have
         {
            length0= i; // extend possible blurring only up to this sample, but not any further
             depth0=z0;
         }

       //if(InFront(z1, depth_min1)) // if this sample is in front of all encountered so far in this direction, this check isn't needed because we do depth tests either way below
            if(l1>=i // this sample movement reaches the center
            && i>length1) // and it extends blurring to the right, compared to what we already have
         {
            length1= i; // extend possible blurring only up to this sample, but not any further
             depth1=z1;
         }

         depth_min0=Min(depth_min0, z0);
         depth_min1=Min(depth_min1, z1);

         // TODO: can this be improved?
         same_vel0*=(Dist2(pixel_vel, c0)<=Sqr(1.5f/pixels));
         same_vel1*=(Dist2(pixel_vel, c1)<=Sqr(1.5f/pixels));

         Bool allow0=(InFront(depth0, depth_min0) || same_vel0),
              allow1=(InFront(depth1, depth_min1) || same_vel1);

         allowed0+=allow0;
         allowed1+=allow1;

         if(length0>=i && allow0)range0=i;
         if(length1>=i && allow1)range1=i;
      }

      Flt dir_length0=range0/Flt(pixels),
          dir_length1=range1/Flt(pixels);

   #if 1
      // normally with the formula above, we can get only integer precision, 'range0' and 'range1' can only be set to integer steps, based on which we set vectors
      // we get multiples of pixel ranges, without fraction, this gets much worse with smaller resolutions, in which steps are bigger
      // to solve this problem, if there were no obstacles found, then maximize directions by original smooth value
      Flt actual_length=blur_dir_length - ROUND*0.5f/pixels, // we need to subtract what we've added before, no need to do Sat, because we're inside BRANCH if that tests for length
            test_length=blur_dir_length + 1.0f/128         ; // we have to use a bigger value than 'actual_length' and 'blur_dir_length' to improve chances of this going through (the epsilon was tested carefully it supports both small and big velocities)
      if(allowed0==samples && (range1>0 || samples<=1) && test_length>=dir_length0)dir_length0=actual_length; // if there were no obstacles in this direction (also check opposite direction because of linear filtering using neighbor sample which causes visible leaking, we check that we've blurred at least one pixel "range>0", however if the velocity is <1 less than 1 pixel then range will not get >0, to support very small smooth velocities, we have to do another check for "samples <= 1"), then try using original
      if(allowed1==samples && (range0>0 || samples<=1) && test_length>=dir_length1)dir_length1=actual_length; // if there were no obstacles in this direction (also check opposite direction because of linear filtering using neighbor sample which causes visible leaking, we check that we've blurred at least one pixel "range>0", however if the velocity is <1 less than 1 pixel then range will not get >0, to support very small smooth velocities, we have to do another check for "samples <= 1"), then try using original
   #endif

      dirs.xy= blur_dir_normalized.xy*dir_length0;
      dirs.zw=-blur_dir_normalized.xy*dir_length1;
      if(do_clamp) // clamp final dirs
      {
         dirs.xy=(UVClamp(inTex+dirs.xy*MotionPixelSize, do_clamp)-inTex)/MotionPixelSize; // first calculate target and clamp it
         dirs.zw=(UVClamp(inTex+dirs.zw*MotionPixelSize, do_clamp)-inTex)/MotionPixelSize; // first calculate target and clamp it
      }
   }

#if !SIGNED_VEL_RT
   dirs=dirs*0.5f+0.5f; // scale -1..1 -> 0..1
#endif
   return dirs;
}
/******************************************************************************/
Vec4 Blur_PS(NOPERSP Vec2 inTex:TEXCOORD,
             NOPERSP PIXEL              ,
             uniform Bool dither        ,
             uniform Bool do_clamp=false):COLOR // no need to do clamp because we've already done that in 'SetDirs'
{
   // Input: Col  - color
   //        Col1 - 2 blur ranges (XY, ZW)

   Vec4 blur=TexLod(Col1, inTex); // use linear filtering because 'Col1' may be smaller
#if !SIGNED_VEL_RT
   blur=((Abs(blur-0.5f)<=1.0f/255) ? Vec4(0, 0, 0, 0) : blur*2-1); // this performs comparisons for all channels separately, force 0 when source value is close to 0.5, otherwise scale to -1..1
#endif

   Vec4 color=Vec4(TexLod(Col, inTex).rgb, 1); // force full alpha so back buffer effects can work ok, can't use 'TexPoint' because 'Col' can be supersampled

   BRANCH if(any(blur)) // we can use 'any' here because small values got clipped out already in 'SetDirs'
   {
      Vec2 dir0=blur.xy*MotionPixelSize,
           dir1=blur.zw*MotionPixelSize;
      if(do_clamp)
      {
         dir0=UVClamp(inTex+dir0, do_clamp)-inTex; // first calculate target and clamp it
         dir1=UVClamp(inTex+dir1, do_clamp)-inTex; // first calculate target and clamp it
      }
      Int samples=MAX_BLUR_SAMPLES;
   #if VARIABLE_BLUR_SAMPLES
      Flt eps=0.15f;
      #if 1
         samples=Ceil(Sat(Max(Abs(blur))/MotionVelScaleLimit.w+eps)*samples);
      #else
         samples=Ceil(Sat(Sqrt(Max(Length2(blur.xy), Length2(blur.zw)))/MotionVelScaleLimit.w+eps)*samples); // have to calculate max of 2 lengths, because one can be clipped due to an obstacle
      #endif
   #endif

      Vec2 t1=inTex;
      dir0/=samples;
      dir1/=samples;

   #if VARIABLE_BLUR_SAMPLES
      LOOP
   #else
      UNROLL
   #endif
         for(Int i=1; i<=samples; i++) // start from 1 because we've already got #0 before
      {
         // TODO: implement new high quality mode that doesn't use 'SetDirs' but calculates per-sample weights based on velocity and depth (for this have to use 'do_clamp')
         color.rgb+=TexLod(Col, inTex+=dir0).rgb; // use linear filtering
         color.rgb+=TexLod(Col,   t1 +=dir1).rgb; // use linear filtering
      }
      color.rgb/=samples*2+1;

   #if TEST_BLUR_PIXELS
      color.r=1;
   #endif
   #if 0 // test how many samples were used for blurring
      color.rgb=samples/16.0f;
   #endif
   }

   if(dither)color.rgb+=DitherValueColor(pixel);
   return color;
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
TECHNIQUE(Explosion, Explosion_VS(), Explosion_PS());

TECHNIQUE(Convert  , Convert_VS(0), Convert_PS(0, false));
TECHNIQUE(ConvertC , Convert_VS(0), Convert_PS(0, true ));
TECHNIQUE(ConvertH , Convert_VS(1), Convert_PS(1, false));
TECHNIQUE(ConvertHC, Convert_VS(1), Convert_PS(1, true ));

TECHNIQUE(Dilate, Draw_VS(), Dilate_PS());

TECHNIQUE(SetDirs , Draw_VS(), SetDirs_PS(false));
TECHNIQUE(SetDirsC, Draw_VS(), SetDirs_PS(true ));

TECHNIQUE(Blur , Draw_VS(), Blur_PS(false));
TECHNIQUE(BlurD, Draw_VS(), Blur_PS(true ));

#define DILATE(range)                                           \
TECHNIQUE(DilateX##range , Draw_VS(), DilateX_PS(range      )); \
TECHNIQUE(DilateY##range , Draw_VS(), DilateY_PS(range      )); \
TECHNIQUE(DilateXD##range, Draw_VS(), DilateX_PS(range, true)); \
TECHNIQUE(DilateYD##range, Draw_VS(), DilateY_PS(range, true));

DILATE(1)
DILATE(2)
DILATE(4)
DILATE(6)
DILATE(8)
DILATE(12)
DILATE(16)
DILATE(24)
DILATE(32)
/******************************************************************************/
