/******************************************************************************

   For AO shader, Depth is linearized to 0 .. Viewport.range

/******************************************************************************/
#include "!Header.h"
#include "Ambient Occlusion.h"
/******************************************************************************/
#define AO0Elms 12
#define AO1Elms 28
#define AO2Elms 48
#define AO3Elms 80
#define DUAL    (MODEL!=SM_GL) // on Mac (13" MacBook Air, Intel HD Graphics 5000), there's a GPU Driver issue that makes Dual Ambient Occlusion have artifacts, so disable DUAL for OpenGL
/******************************************************************************/
BUFFER(AOConstants)
#if DUAL // divided into 2 equal groups, then Y X sorted
   const Vec AO0Vec[]={Vec(0.000, -0.707, 0.356), Vec(-0.354, -0.354, 0.600), Vec(0.354, -0.354, 0.600), Vec(-0.707, 0.000, 0.356), Vec(-0.354, 0.354, 0.600), Vec(0.354, 0.354, 0.600), Vec(-0.707, -0.707, 0.200), Vec(0.707, -0.707, 0.200), Vec(0.707, 0.000, 0.356), Vec(-0.707, 0.707, 0.200), Vec(0.000, 0.707, 0.356), Vec(0.707, 0.707, 0.200), };
   const Vec AO1Vec[]={Vec(-0.471, -0.471, 0.398), Vec(0.000, -0.471, 0.636), Vec(0.471, -0.471, 0.398), Vec(-0.707, -0.236, 0.319), Vec(-0.236, -0.236, 0.801), Vec(0.236, -0.236, 0.801), Vec(-0.471, 0.000, 0.636), Vec(0.471, 0.000, 0.636), Vec(-0.236, 0.236, 0.801), Vec(0.236, 0.236, 0.801), Vec(-0.471, 0.471, 0.398), Vec(0.000, 0.471, 0.636), Vec(0.471, 0.471, 0.398), Vec(-0.236, 0.707, 0.319), Vec(0.000, -0.943, 0.206), Vec(-0.707, -0.707, 0.200), Vec(-0.236, -0.707, 0.319), Vec(0.236, -0.707, 0.319), Vec(0.707, -0.707, 0.200), Vec(0.707, -0.236, 0.319), Vec(-0.943, 0.000, 0.206), Vec(0.943, 0.000, 0.206), Vec(-0.707, 0.236, 0.319), Vec(0.707, 0.236, 0.319), Vec(-0.707, 0.707, 0.200), Vec(0.236, 0.707, 0.319), Vec(0.707, 0.707, 0.200), Vec(0.000, 0.943, 0.206), };
   const Vec AO2Vec[]={Vec(0.000, -0.707, 0.356), Vec(-0.177, -0.530, 0.526), Vec(0.177, -0.530, 0.526), Vec(-0.354, -0.354, 0.600), Vec(0.000, -0.354, 0.778), Vec(0.354, -0.354, 0.600), Vec(-0.530, -0.177, 0.526), Vec(-0.177, -0.177, 0.884), Vec(0.177, -0.177, 0.884), Vec(0.530, -0.177, 0.526), Vec(-0.707, 0.000, 0.356), Vec(-0.354, 0.000, 0.778), Vec(0.354, 0.000, 0.778), Vec(0.707, 0.000, 0.356), Vec(-0.530, 0.177, 0.526), Vec(-0.177, 0.177, 0.884), Vec(0.177, 0.177, 0.884), Vec(0.530, 0.177, 0.526), Vec(-0.354, 0.354, 0.600), Vec(0.000, 0.354, 0.778), Vec(0.354, 0.354, 0.600), Vec(-0.177, 0.530, 0.526), Vec(0.177, 0.530, 0.526), Vec(0.000, 0.707, 0.356), Vec(-0.177, -0.884, 0.218), Vec(0.177, -0.884, 0.218), Vec(-0.707, -0.707, 0.200), Vec(-0.354, -0.707, 0.281), Vec(0.354, -0.707, 0.281), Vec(0.707, -0.707, 0.200), Vec(-0.530, -0.530, 0.315), Vec(0.530, -0.530, 0.315), Vec(-0.707, -0.354, 0.281), Vec(0.707, -0.354, 0.281), Vec(-0.884, -0.177, 0.218), Vec(0.884, -0.177, 0.218), Vec(-0.884, 0.177, 0.218), Vec(0.884, 0.177, 0.218), Vec(-0.707, 0.354, 0.281), Vec(0.707, 0.354, 0.281), Vec(-0.530, 0.530, 0.315), Vec(0.530, 0.530, 0.315), Vec(-0.707, 0.707, 0.200), Vec(-0.354, 0.707, 0.281), Vec(0.354, 0.707, 0.281), Vec(0.707, 0.707, 0.200), Vec(-0.177, 0.884, 0.218), Vec(0.177, 0.884, 0.218), };
   const Vec AO3Vec[]={Vec(-0.141, -0.707, 0.342), Vec(0.141, -0.707, 0.342), Vec(-0.283, -0.566, 0.437), Vec(0.000, -0.566, 0.517), Vec(0.283, -0.566, 0.437), Vec(-0.424, -0.424, 0.475), Vec(-0.141, -0.424, 0.666), Vec(0.141, -0.424, 0.666), Vec(0.424, -0.424, 0.475), Vec(-0.566, -0.283, 0.437), Vec(-0.283, -0.283, 0.724), Vec(0.000, -0.283, 0.853), Vec(0.283, -0.283, 0.724), Vec(0.566, -0.283, 0.437), Vec(-0.424, -0.141, 0.666), Vec(-0.141, -0.141, 0.924), Vec(0.141, -0.141, 0.924), Vec(0.424, -0.141, 0.666), Vec(-0.566, 0.000, 0.517), Vec(-0.283, 0.000, 0.853), Vec(0.283, 0.000, 0.853), Vec(0.566, 0.000, 0.517), Vec(-0.424, 0.141, 0.666), Vec(-0.141, 0.141, 0.924), Vec(0.141, 0.141, 0.924), Vec(0.424, 0.141, 0.666), Vec(0.707, 0.141, 0.342), Vec(-0.566, 0.283, 0.437), Vec(-0.283, 0.283, 0.724), Vec(0.000, 0.283, 0.853), Vec(0.283, 0.283, 0.724), Vec(0.566, 0.283, 0.437), Vec(-0.424, 0.424, 0.475), Vec(-0.141, 0.424, 0.666), Vec(0.141, 0.424, 0.666), Vec(0.424, 0.424, 0.475), Vec(-0.283, 0.566, 0.437), Vec(0.000, 0.566, 0.517), Vec(0.283, 0.566, 0.437), Vec(-0.141, 0.707, 0.342), Vec(-0.141, -0.990, 0.200), Vec(0.141, -0.990, 0.200), Vec(-0.283, -0.849, 0.221), Vec(0.000, -0.849, 0.243), Vec(0.283, -0.849, 0.221), Vec(-0.707, -0.707, 0.200), Vec(-0.424, -0.707, 0.257), Vec(0.424, -0.707, 0.257), Vec(0.707, -0.707, 0.200), Vec(-0.566, -0.566, 0.274), Vec(0.566, -0.566, 0.274), Vec(-0.707, -0.424, 0.257), Vec(0.707, -0.424, 0.257), Vec(-0.849, -0.283, 0.221), Vec(0.849, -0.283, 0.221), Vec(-0.990, -0.141, 0.200), Vec(-0.707, -0.141, 0.342), Vec(0.707, -0.141, 0.342), Vec(0.990, -0.141, 0.200), Vec(-0.849, 0.000, 0.243), Vec(0.849, 0.000, 0.243), Vec(-0.990, 0.141, 0.200), Vec(-0.707, 0.141, 0.342), Vec(0.990, 0.141, 0.200), Vec(-0.849, 0.283, 0.221), Vec(0.849, 0.283, 0.221), Vec(-0.707, 0.424, 0.257), Vec(0.707, 0.424, 0.257), Vec(-0.566, 0.566, 0.274), Vec(0.566, 0.566, 0.274), Vec(-0.707, 0.707, 0.200), Vec(-0.424, 0.707, 0.257), Vec(0.141, 0.707, 0.342), Vec(0.424, 0.707, 0.257), Vec(0.707, 0.707, 0.200), Vec(-0.283, 0.849, 0.221), Vec(0.000, 0.849, 0.243), Vec(0.283, 0.849, 0.221), Vec(-0.141, 0.990, 0.200), Vec(0.141, 0.990, 0.200), };
#else // Y X sorted
   const Vec AO0Vec[]={Vec(-0.707f, -0.707f, 0.200f), Vec(0.000f, -0.707f, 0.356f), Vec(0.707f, -0.707f, 0.200f), Vec(-0.354f, -0.354f, 0.600f), Vec(0.354f, -0.354f, 0.600f), Vec(-0.707f, -0.000f, 0.356f), Vec(0.707f, 0.000f, 0.356f), Vec(-0.354f, 0.354f, 0.600f), Vec(0.354f, 0.354f, 0.600f), Vec(-0.707f, 0.707f, 0.200f), Vec(-0.000f, 0.707f, 0.356f), Vec(0.707f, 0.707f, 0.200f), };
   const Vec AO1Vec[]={Vec(0.000f, -0.943f, 0.206f), Vec(-0.707f, -0.707f, 0.200f), Vec(-0.236f, -0.707f, 0.319f), Vec(0.236f, -0.707f, 0.319f), Vec(0.707f, -0.707f, 0.200f), Vec(-0.471f, -0.471f, 0.398f), Vec(0.000f, -0.471f, 0.636f), Vec(0.471f, -0.471f, 0.398f), Vec(-0.707f, -0.236f, 0.319f), Vec(-0.236f, -0.236f, 0.801f), Vec(0.236f, -0.236f, 0.801f), Vec(0.707f, -0.236f, 0.319f), Vec(-0.943f, -0.000f, 0.206f), Vec(-0.471f, -0.000f, 0.636f), Vec(0.471f, 0.000f, 0.636f), Vec(0.943f, 0.000f, 0.206f), Vec(-0.707f, 0.236f, 0.319f), Vec(-0.236f, 0.236f, 0.801f), Vec(0.236f, 0.236f, 0.801f), Vec(0.707f, 0.236f, 0.319f), Vec(-0.471f, 0.471f, 0.398f), Vec(-0.000f, 0.471f, 0.636f), Vec(0.471f, 0.471f, 0.398f), Vec(-0.707f, 0.707f, 0.200f), Vec(-0.236f, 0.707f, 0.319f), Vec(0.236f, 0.707f, 0.319f), Vec(0.707f, 0.707f, 0.200f), Vec(-0.000f, 0.943f, 0.206f), };
   const Vec AO2Vec[]={Vec(-0.177f, -0.884f, 0.218f), Vec(0.177f, -0.884f, 0.218f), Vec(-0.707f, -0.707f, 0.200f), Vec(-0.354f, -0.707f, 0.281f), Vec(0.000f, -0.707f, 0.356f), Vec(0.354f, -0.707f, 0.281f), Vec(0.707f, -0.707f, 0.200f), Vec(-0.530f, -0.530f, 0.315f), Vec(-0.177f, -0.530f, 0.526f), Vec(0.177f, -0.530f, 0.526f), Vec(0.530f, -0.530f, 0.315f), Vec(-0.707f, -0.354f, 0.281f), Vec(-0.354f, -0.354f, 0.600f), Vec(0.000f, -0.354f, 0.778f), Vec(0.354f, -0.354f, 0.600f), Vec(0.707f, -0.354f, 0.281f), Vec(-0.884f, -0.177f, 0.218f), Vec(-0.530f, -0.177f, 0.526f), Vec(-0.177f, -0.177f, 0.884f), Vec(0.177f, -0.177f, 0.884f), Vec(0.530f, -0.177f, 0.526f), Vec(0.884f, -0.177f, 0.218f), Vec(-0.707f, -0.000f, 0.356f), Vec(-0.354f, -0.000f, 0.778f), Vec(0.354f, 0.000f, 0.778f), Vec(0.707f, 0.000f, 0.356f), Vec(-0.884f, 0.177f, 0.218f), Vec(-0.530f, 0.177f, 0.526f), Vec(-0.177f, 0.177f, 0.884f), Vec(0.177f, 0.177f, 0.884f), Vec(0.530f, 0.177f, 0.526f), Vec(0.884f, 0.177f, 0.218f), Vec(-0.707f, 0.354f, 0.281f), Vec(-0.354f, 0.354f, 0.600f), Vec(-0.000f, 0.354f, 0.778f), Vec(0.354f, 0.354f, 0.600f), Vec(0.707f, 0.354f, 0.281f), Vec(-0.530f, 0.530f, 0.315f), Vec(-0.177f, 0.530f, 0.526f), Vec(0.177f, 0.530f, 0.526f), Vec(0.530f, 0.530f, 0.315f), Vec(-0.707f, 0.707f, 0.200f), Vec(-0.354f, 0.707f, 0.281f), Vec(-0.000f, 0.707f, 0.356f), Vec(0.354f, 0.707f, 0.281f), Vec(0.707f, 0.707f, 0.200f), Vec(-0.177f, 0.884f, 0.218f), Vec(0.177f, 0.884f, 0.218f), };
   const Vec AO3Vec[]={Vec(-0.141f, -0.990f, 0.200f), Vec(0.141f, -0.990f, 0.200f), Vec(-0.283f, -0.849f, 0.221f), Vec(0.000f, -0.849f, 0.243f), Vec(0.283f, -0.849f, 0.221f), Vec(-0.707f, -0.707f, 0.200f), Vec(-0.424f, -0.707f, 0.257f), Vec(-0.141f, -0.707f, 0.342f), Vec(0.141f, -0.707f, 0.342f), Vec(0.424f, -0.707f, 0.257f), Vec(0.707f, -0.707f, 0.200f), Vec(-0.566f, -0.566f, 0.274f), Vec(-0.283f, -0.566f, 0.437f), Vec(0.000f, -0.566f, 0.517f), Vec(0.283f, -0.566f, 0.437f), Vec(0.566f, -0.566f, 0.274f), Vec(-0.707f, -0.424f, 0.257f), Vec(-0.424f, -0.424f, 0.475f), Vec(-0.141f, -0.424f, 0.666f), Vec(0.141f, -0.424f, 0.666f), Vec(0.424f, -0.424f, 0.475f), Vec(0.707f, -0.424f, 0.257f), Vec(-0.849f, -0.283f, 0.221f), Vec(-0.566f, -0.283f, 0.437f), Vec(-0.283f, -0.283f, 0.724f), Vec(0.000f, -0.283f, 0.853f), Vec(0.283f, -0.283f, 0.724f), Vec(0.566f, -0.283f, 0.437f), Vec(0.849f, -0.283f, 0.221f), Vec(-0.990f, -0.141f, 0.200f), Vec(-0.707f, -0.141f, 0.342f), Vec(-0.424f, -0.141f, 0.666f), Vec(-0.141f, -0.141f, 0.924f), Vec(0.141f, -0.141f, 0.924f), Vec(0.424f, -0.141f, 0.666f), Vec(0.707f, -0.141f, 0.342f), Vec(0.990f, -0.141f, 0.200f), Vec(-0.849f, -0.000f, 0.243f), Vec(-0.566f, -0.000f, 0.517f), Vec(-0.283f, -0.000f, 0.853f), Vec(0.283f, 0.000f, 0.853f), Vec(0.566f, 0.000f, 0.517f), Vec(0.849f, 0.000f, 0.243f), Vec(-0.990f, 0.141f, 0.200f), Vec(-0.707f, 0.141f, 0.342f), Vec(-0.424f, 0.141f, 0.666f), Vec(-0.141f, 0.141f, 0.924f), Vec(0.141f, 0.141f, 0.924f), Vec(0.424f, 0.141f, 0.666f), Vec(0.707f, 0.141f, 0.342f), Vec(0.990f, 0.141f, 0.200f), Vec(-0.849f, 0.283f, 0.221f), Vec(-0.566f, 0.283f, 0.437f), Vec(-0.283f, 0.283f, 0.724f), Vec(-0.000f, 0.283f, 0.853f), Vec(0.283f, 0.283f, 0.724f), Vec(0.566f, 0.283f, 0.437f), Vec(0.849f, 0.283f, 0.221f), Vec(-0.707f, 0.424f, 0.257f), Vec(-0.424f, 0.424f, 0.475f), Vec(-0.141f, 0.424f, 0.666f), Vec(0.141f, 0.424f, 0.666f), Vec(0.424f, 0.424f, 0.475f), Vec(0.707f, 0.424f, 0.257f), Vec(-0.566f, 0.566f, 0.274f), Vec(-0.283f, 0.566f, 0.437f), Vec(-0.000f, 0.566f, 0.517f), Vec(0.283f, 0.566f, 0.437f), Vec(0.566f, 0.566f, 0.274f), Vec(-0.707f, 0.707f, 0.200f), Vec(-0.424f, 0.707f, 0.257f), Vec(-0.141f, 0.707f, 0.342f), Vec(0.141f, 0.707f, 0.342f), Vec(0.424f, 0.707f, 0.257f), Vec(0.707f, 0.707f, 0.200f), Vec(-0.283f, 0.849f, 0.221f), Vec(-0.000f, 0.849f, 0.243f), Vec(0.283f, 0.849f, 0.221f), Vec(-0.141f, 0.990f, 0.200f), Vec(0.141f, 0.990f, 0.200f), };
#endif
BUFFER_END
/******************************************************************************/
Vec4 AO_PS(NOPERSP Vec2 inTex  :TEXCOORD ,
           NOPERSP Vec2 inPosXY:TEXCOORD1,
           PIXEL                         ,
   uniform Int  mode                     ,
   uniform Bool jitter                   ,
   uniform Bool normals                  ):COLOR
{
   {
      Flt  z;
      Vec2 nrm;

      if(normals)
      {
         Vec p=GetPos(TexDepthRawPoint(inTex), inPosXY); z=p.z; // !! for AO shader depth is already linearized !!
      #if 1 // sharp normal, looks better
         Vec n=TexLod(Nrm, inTex).xyz; // use linear filtering because Nrm may be bigger
         #if !SIGNED_NRM_RT
            n-=0.5f; // normally it should be "n=n*2-1", however since the normal doesn't need to be normalized, we can just do -0.5
         #endif
      #else // smoothened normal, less detail
         Flt zl=TexDepthRawPoint(inTex+ColSize.xy*Vec2(-1, 0)),
             zr=TexDepthRawPoint(inTex+ColSize.xy*Vec2( 1, 0)),
             zd=TexDepthRawPoint(inTex+ColSize.xy*Vec2( 0,-1)),
             zu=TexDepthRawPoint(inTex+ColSize.xy*Vec2( 0, 1)),
             dl=z-zl, dr=zr-z, adr=Abs(dr), adl=Abs(dl),
             dd=z-zd, du=zu-z, adu=Abs(du), add=Abs(dd);

         Vec2 smooth_ru=Vec2(Sat(1-adr*2), Sat(1-adu*2)),
              smooth_ld=Vec2(Sat(1-adl*2), Sat(1-add*2));

         // read two normals from texture, convert to -1..1 scale and average them
         // ((n1*2-1) + (n2*2-1))/2 = n1+n2-1
         Vec n=TexLod(Nrm, inTex+ColSize.xy*0.5f*smooth_ru).xyz+ // apply 0.5 scale so we get a smooth texel from 4 values using bilinear filtering
               TexLod(Nrm, inTex-ColSize.xy*0.5f*smooth_ld).xyz; // apply 0.5 scale so we get a smooth texel from 4 values using bilinear filtering
         #if !SIGNED_NRM_RT
            n-=2*0.5f; // same as above (0.5f) but for 2 Tex reads
         #endif
      #endif
       //n=Normalize(n); 'n' does not need to be normalized, because following codes don't require that
         Vec dir_right=Normalize(Vec(ScreenToPosXY(inTex+Vec2(ColSize.x, 0)), 1)),
             dir_up   =Normalize(Vec(ScreenToPosXY(inTex+Vec2(0, ColSize.y)), 1));
      #if 0
         Vec pr=PointOnPlaneRay(Vec(0, 0, 0), p, n, dir_right),
             pu=PointOnPlaneRay(Vec(0, 0, 0), p, n, dir_up   );

         nrm=Vec2(pr.z-z, pu.z-z)*ColSize.zw;
      #else // optimized
         Flt pr_z=dir_right.z*Dot(p, n)/Dot(dir_right, n),
             pu_z=dir_up   .z*Dot(p, n)/Dot(dir_up   , n);

         nrm=Vec2(pr_z-z, pu_z-z)*ColSize.zw;
      #endif
      }else
      {
         // !! for AO shader depth is already linearized !!
             z =TexDepthRawPoint(inTex);
         Flt zl=TexDepthRawPoint(inTex-Vec2(ColSize.x, 0)),
             zr=TexDepthRawPoint(inTex+Vec2(ColSize.x, 0)),
             zd=TexDepthRawPoint(inTex-Vec2(0, ColSize.y)),
             zu=TexDepthRawPoint(inTex+Vec2(0, ColSize.y)),
             dl=z-zl, dr=zr-z,
             dd=z-zd, du=zu-z;

         nrm=Vec2((Abs(dl)<Abs(dr)) ? dl : dr,
                  (Abs(dd)<Abs(du)) ? dd : du)*ColSize.zw;
      }

      Vec2 cos_sin;
      if(jitter)
      {
         Flt a    =Sum(pixel.xy*Vec2(0.5f, 0.25f)),
             angle=0.8f;
         a=Frac(a)*angle - 0.5f*angle; // (Frac(a)-0.5f)*angle;
         CosSin(cos_sin.x, cos_sin.y, a);
      }

      Flt  scale =1/Max(1.0f, z),
           occl  =0,
           weight=EPS,
           bias  =AmbBias/AmbScale + z/1024; // increase bias with camera distance to deal with artifacts (best noticed in World Editor with only Ambient Light set to full, no other lights, terrain material set to white without textures, ambient contrast >=1 and ambient mode "Low")
   #if DUAL
      Vec2 offs_scale[2]={Viewport.size_fov_tan*scale*AmbRange.x, Viewport.size_fov_tan*scale*AmbRange.y};
   #else
      Vec2 offs_scale=Viewport.size_fov_tan*scale*AmbRange.x;
   #endif

      // required for later optimizations
   #if DUAL
      Flt  range[2]={-1/AmbScale/AmbRange.x, -1/AmbScale/AmbRange.y};
      Vec2 NRM  [2]={nrm*range[0]*-5       , nrm*range[1]*-5       };
      Flt  Z    [2]={ -z*range[0] - bias   ,  -z*range[1] - bias   };
   #else
      Flt range=-1/AmbScale/AmbRange.x;
      nrm*=range*-5;
      z   =-z*range - bias;
   #endif

      Int        elms;
      if(mode==0)elms=AO0Elms;else
      if(mode==1)elms=AO1Elms;else
      if(mode==2)elms=AO2Elms;else
                 elms=AO3Elms;
      Int        mid =elms/2;
      UNROLL for(Int i=0; i<elms; i++)
      {
         const Bool index=(i<mid);

         Vec        pattern;
         if(mode==0)pattern=AO0Vec[i];else
         if(mode==1)pattern=AO1Vec[i];else
         if(mode==2)pattern=AO2Vec[i];else
                    pattern=AO3Vec[i];

      #if DUAL
         Vec2      offs=pattern.xy*offs_scale[index];
      #else
         Vec2      offs=pattern.xy*offs_scale;
      #endif
         if(jitter)offs=Rotate(offs, cos_sin);
                   offs=Round (offs*ColSize.zw)*ColSize.xy; // doesn't make a big difference for pixels close to camera, but makes a HUGE difference for pixels far away, keep !! otherwise distant terrain gets unnaturally shaded

         Vec2 t=inTex+offs;

         // !! for AO shader depth is already linearized !!
      #if 0 // Unoptimized
         Flt range=AmbScale*AmbRange.x,
             delta=z - AmbBias*AmbRange - TexDepthRawPoint(t), // apply bias to Z and calculate delta between TexDepth at that location, can use point filtering because we've rounded 't'
             expected=Sum(nrm*offs), // this is the expected delta
             o=Sat((delta+expected)/(range/5)),
             w=BlendSqr(delta/range); // ignore samples that are out of range (Z dist)
         if(delta<=-range){o=0; w=1;} // if sample is behind us and out of range, then set it as brightening
      #else // Optimized
         #if DUAL
            Flt delta=TexDepthRawPoint(t)*range[index]+Z[index], // !! for AO shader depth is already linearized !! can use point filtering because we've rounded 't'
                o=Sat(delta*5+Sum(NRM[index]*offs)),
         #else
            Flt delta=TexDepthRawPoint(t)*range+z, // !! for AO shader depth is already linearized !! can use point filtering because we've rounded 't'
                o=Sat(delta*5+Sum(nrm*offs)),
         #endif
             w=BlendSqr(delta); // ignore samples that are out of range (Z dist)
         if(delta<=-1){o=0; w=1;} // if sample is behind us and out of range, then set it as brightening
      #endif

       //w     *=pattern.z; // focus on samples near to the center (XY dist)
         occl  +=w*o;
         weight+=w;
      }
      return 1-AmbContrast*occl/weight; // result is stored in One Channel 1 Byte RT so it doesn't need 'Sat' saturation
   }
}
/******************************************************************************/
TECHNIQUE(AO0  , DrawPosXY_VS(), AO_PS(0, false, false));
TECHNIQUE(AO1  , DrawPosXY_VS(), AO_PS(1, false, false));
TECHNIQUE(AO2  , DrawPosXY_VS(), AO_PS(2, false, false));
TECHNIQUE(AO3  , DrawPosXY_VS(), AO_PS(3, false, false));
TECHNIQUE(AO0J , DrawPosXY_VS(), AO_PS(0, true , false));
TECHNIQUE(AO1J , DrawPosXY_VS(), AO_PS(1, true , false));
TECHNIQUE(AO2J , DrawPosXY_VS(), AO_PS(2, true , false));
TECHNIQUE(AO3J , DrawPosXY_VS(), AO_PS(3, true , false));
TECHNIQUE(AO0N , DrawPosXY_VS(), AO_PS(0, false, true ));
TECHNIQUE(AO1N , DrawPosXY_VS(), AO_PS(1, false, true ));
TECHNIQUE(AO2N , DrawPosXY_VS(), AO_PS(2, false, true ));
TECHNIQUE(AO3N , DrawPosXY_VS(), AO_PS(3, false, true ));
TECHNIQUE(AO0JN, DrawPosXY_VS(), AO_PS(0, true , true ));
TECHNIQUE(AO1JN, DrawPosXY_VS(), AO_PS(1, true , true ));
TECHNIQUE(AO2JN, DrawPosXY_VS(), AO_PS(2, true , true ));
TECHNIQUE(AO3JN, DrawPosXY_VS(), AO_PS(3, true , true ));
/******************************************************************************/
