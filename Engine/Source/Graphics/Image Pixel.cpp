/******************************************************************************
   
   'CubicFastSharp' is equal to 'Lerp4Weights' in following way:

      const int n=100; REP(n)
      {
         Flt  step=i/Flt(n-1);
         Vec4 w; Lerp4Weights(w, step);
         Flt  c=CubicFastSharp(step*2);
         Vec2(step*0.5f+0.5f, w.x).draw(RED);
         Vec2(step*0.5f     , w.y).draw(GREEN);
         Vec2(step          , c  ).draw(BLUE);
      }

   Upscaling should be done in sRGB Gamma space to allow for smooth gradients - looks much better !!

   After many tests for creating mip maps REP(3)image.downSample(FILTER_DOWN);
      it was noticed that doing Gamma Correct filtering does not improve quality significantly, as FILTER_DOWN has sharpening filter which already makes it look good.
      Using Gamma Correct filtering made bigger difference for image.resize(image.w()/8, image.h()/8) though

/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "Import/BC.h"
#include "Import/ETC.h"

#define SUPPORT_DEPTH_TO_COLOR 0

#define ALPHA_LIMIT 0.0625f // alpha limit at which colors start to blend to RGB, warning: if increasing this value then it might cause overflow for integer processing (for 'CWA8AlphaLimit')

/*
   Flt CW[8][8];
   Int smallest=-1; Flt dist=0;
   for(Int i=1; i<34984; i++)
   {
      Flt d=0;
      REPD(y, 8)
      {
         Flt fy2=Sqr(-1.75f+0.5f*y);
         REPD(x, 8)
         {
            Flt fx2=Sqr(-1.75f+0.5f*x), w=Cubic(Sqrt(fx2+fy2));
            CW[y][x]=w/0.00417041779f*i;
            MAX(d, Abs(Round(CW[y][x])-CW[y][x]));
         }
      }
      if(smallest<0 || d<dist)
      {
         Long sum=0; REPD(y, 8)REPD(x, 8)sum+=255*CW[y][x];
         if(sum<0x7FFFFFFF){dist=d; smallest=i;}
      }
      if(smallest==1321)
      {
         Long sum=0;
         REPD(y, 8)
         REPD(x, 8)sum+=255*CW[y][x];
         Int x=CW[4][4];
             x*=255; // color
             x*=255; // alpha
             x*=64;
         Str s;
         s.line()+=S+"static const Int CW8Sum="+sum+"/255;";
         s.line()+=S+"static const Int CW8[8][8]={";
         FREPD(y, 8)
         {
            s.line()+="   {";
            FREPD(x, 8)
            {
               if(x)s+=", ";
               s+=Round(CW[y][x]);
            }
            s+="},";
         }
         s.line()+="};";
         ClipSet(s.line());
      }
   }

   Flt CWA[8][8];
   Int sum=0;
   REPD(y, 8)
   {
      Flt fy2=Sqr(-1.75f+0.5f*y);
      REPD(x, 8)
      {
         Flt fx2=Sqr(-1.75f+0.5f*x), w=Cubic(Sqrt(fx2+fy2));
         CWA[y][x]=w*0xFF08/2.9391276836395264;
         sum+=Round(CWA[y][x]);
      }
   }
   Int s=sum;


   Dbl CFSMW8[8][8], W=0;
   REPD(y, 8)
   {
      Dbl fy2=Sqr(-1.75+0.5*y);
      REPD(x, 8)
      {
         Dbl fx2=Sqr(-1.75+0.5*x), w=fx2+fy2; w=((w<Sqr(CUBIC_FAST_RANGE)) ? CubicFastSmooth(Sqrt(w)) : 0);
         W+=w;
         CFSMW8[y][x]=w;
      }
   }
   REPD(y, 8)
   REPD(x, 8)CFSMW8[y][x]/=W;

   Str s;
   s.line()+=S+"static const Flt CFSMW8[8][8]= // [y][x]";
   s.line()+='{';
   FREPD(y, 8)
   {
      s.line()+="   {";
      FREPD(x, 8)
      {
         if(x)s+=", ";
         s+=CFSMW8[y][x]; s+='f';
      }
      s+="},";
   }
   s.line()+="};";
   ClipSet(s.line());
*/
static const Int CW8Sum=237400608/255;
static const Int CW8[8][8]={
   {0, 0, -1321, -6558, -6558, -1321, 0, 0},
   {0, -6558, -21317, -22888, -22888, -21317, -6558, 0},
   {-1321, -21317, -8477, 56591, 56591, -8477, -21317, -1321},
   {-6558, -22888, 56591, 238767, 238767, 56591, -22888, -6558},
   {-6558, -22888, 56591, 238767, 238767, 56591, -22888, -6558},
   {-1321, -21317, -8477, 56591, 56591, -8477, -21317, -1321},
   {0, -6558, -21317, -22888, -22888, -21317, -6558, 0},
   {0, 0, -1321, -6558, -6558, -1321, 0, 0},
};
static const Int CWA8Sum=5571097/255;
static const Int CWA8AlphaLimit=CWA8Sum*255*ALPHA_LIMIT;
static const Int CWA8[8][8]={
   {0, 0, -31, -154, -154, -31, 0, 0},
   {0, -154, -500, -537, -537, -500, -154, 0},
   {-31, -500, -199, 1328, 1328, -199, -500, -31},
   {-154, -537, 1328, 5603, 5603, 1328, -537, -154},
   {-154, -537, 1328, 5603, 5603, 1328, -537, -154},
   {-31, -500, -199, 1328, 1328, -199, -500, -31},
   {0, -154, -500, -537, -537, -500, -154, 0},
   {0, 0, -31, -154, -154, -31, 0, 0},
};

static const Flt CFSMW8[8][8]= // [y][x]
{
   {0.000000000f, 0.000000000f, 0.000025184f, 0.000355931f, 0.000355931f, 0.000025184f, 0.000000000f, 0.000000000f},
   {0.000000000f, 0.000355931f, 0.004531289f, 0.010840487f, 0.010840487f, 0.004531289f, 0.000355931f, 0.000000000f},
   {0.000025184f, 0.004531289f, 0.023553867f, 0.049229048f, 0.049229048f, 0.023553867f, 0.004531289f, 0.000025184f},
   {0.000355931f, 0.010840487f, 0.049229048f, 0.096126321f, 0.096126321f, 0.049229048f, 0.010840487f, 0.000355931f},
   {0.000355931f, 0.010840487f, 0.049229048f, 0.096126321f, 0.096126321f, 0.049229048f, 0.010840487f, 0.000355931f},
   {0.000025184f, 0.004531289f, 0.023553867f, 0.049229048f, 0.049229048f, 0.023553867f, 0.004531289f, 0.000025184f},
   {0.000000000f, 0.000355931f, 0.004531289f, 0.010840487f, 0.010840487f, 0.004531289f, 0.000355931f, 0.000000000f},
   {0.000000000f, 0.000000000f, 0.000025184f, 0.000355931f, 0.000355931f, 0.000025184f, 0.000000000f, 0.000000000f},
};

#define J1 PLATFORM(_j1, j1)
/******************************************************************************/
// SRGB
/******************************************************************************/
static Flt  SRGBToLinearArray[256];
static Byte LinearToSRGBArray[3139]; // 3139=smallest array size which preserves "LinearToSRGBFast(SRGBToLinearFast(s))==s"

static Flt  ByteSRGBToLinear(Byte s) {return           SRGBToLinear(s/255.0f) ;} // convert 0..255 srgb to 0..1 linear
static Byte LinearToByteSRGB(Flt  l) {return FltToByte(LinearToSRGB(l       ));} // convert 0..1 linear to 0..255 srgb

static INLINE Flt SRGBToLinearFast(Byte s) {return SRGBToLinearArray[s];}
static       Byte LinearToSRGBFast(Flt  l) {return LinearToSRGBArray[Mid(RoundPos(l*(Elms(LinearToSRGBArray)-1)), 0, Elms(LinearToSRGBArray)-1)];}

void InitSRGB()
{
   REPAO(SRGBToLinearArray)=ByteSRGBToLinear(i);
   REPAO(LinearToSRGBArray)=LinearToByteSRGB(i/Flt(Elms(LinearToSRGBArray)-1));

#if DEBUG && 0 // do some debug checks
   FREP(Elms(SRGBToLinearArray)-1)DYNAMIC_ASSERT(SRGBToLinearArray[i]<=SRGBToLinearArray[i+1], "SRGBToLinearArray[i] > SRGBToLinearArray[i+1]");
   FREP(Elms(LinearToSRGBArray)-1)DYNAMIC_ASSERT(LinearToSRGBArray[i]<=LinearToSRGBArray[i+1], "LinearToSRGBArray[i] > LinearToSRGBArray[i+1]");
   FREP(                      256)DYNAMIC_ASSERT(LinearToSRGBFast(SRGBToLinearFast(i))==i    , "LinearToSRGBFast(SRGBToLinearFast(s))!=s");
#if 0
   Str s;
   s.line(); FREPA(SRGBToLinearArray)s.space()+=SRGBToLinearArray[i];
   s.line(); FREPA(SRGBToLinearArray)s.space()+=LinearToByteSRGB(SRGBToLinearArray[i])-LinearToSRGBFast(SRGBToLinearArray[i]);
   Exit(s);
#endif
#endif
}
#if 0 // approximate functions
/*
   Error was calculated using:
   Flt d0=0; Int d1=0;
   REP(256)
   {
      Flt  f=i/255.0f,
           l1=ByteSRGBToLinear  (i),
           l2=SRGBToLinearApprox(i);
      Byte s1=LinearToByteSRGB  (f),
           s2=LinearToSRGBApprox(f);
      d0+=Abs(l1-l2);
      d1+=Abs(s1-s2);
   }
   Exit(S+d0/256+' '+d1/255.0f/256);
*/
#define SRGB_MODE 0
static Flt SRGBToLinearApprox(Byte s)
{
   Flt f=s/255.0f;
#if   SRGB_MODE==0 // average error = 0.023
   return Sqr(f);
#elif SRGB_MODE==1 // average error = 0.004
   return Pow(f, 2.2f);
#else              // average error = 0.001
   return f*(f*(f*0.305306011f+0.682171111f)+0.012522878f);
#endif
}
static Byte LinearToSRGBApprox(Flt l)
{
   if(l<=0)return 0;
#if   SRGB_MODE==0 // average error = 0.023
   Flt s=SqrtFast(l);
#elif SRGB_MODE==1 // average error = 0.004
   Flt s=Pow(l, 1/2.2f);
#else              // average error = 0.001
   Flt s1=SqrtFast(l),
       s2=SqrtFast(s1),
       s3=SqrtFast(s2),
       s =0.585122381f*s1 + 0.783140355f * s2 - 0.368262736f*s3;
#endif
   return Min(RoundPos(s*255.0f), 255);
}
#endif
/******************************************************************************/
// FILTERING
/******************************************************************************/
static Flt Linear(Flt x) {ABS(x); if(x>=1)return 0; return 1-x;}
/******************************************************************************/
#define CEIL(x) int(x+0.99f)
ASSERT(CEIL(0.0f)==0);
ASSERT(CEIL(0.4f)==1);
ASSERT(CEIL(0.5f)==1);
ASSERT(CEIL(0.6f)==1);
ASSERT(CEIL(0.9f)==1);
ASSERT(CEIL(1.0f)==1);
ASSERT(CEIL(1.1f)==2);

#define CUBIC_FAST_RANGE       2
#define CUBIC_FAST_SAMPLES    CEIL(CUBIC_FAST_RANGE)
#define CUBIC_MED_SHARPNESS   (2/2.5f) // (2/2.65f) is smooth and correctly works with gradients, but (2/2.5f) is sharper and looks like JincJinc for regular images
#define CUBIC_SHARP_SHARPNESS (2/2.5f) // (2/2.65f) is smooth and correctly works with gradients, but (2/2.5f) is sharper and looks like JincJinc for regular images
#define CUBIC_MED_RANGE       (2/CUBIC_MED_SHARPNESS  )
#define CUBIC_SHARP_RANGE     (2/CUBIC_SHARP_SHARPNESS)
#define CUBIC_MED_SAMPLES     CEIL(CUBIC_MED_RANGE    )
#define CUBIC_SHARP_SAMPLES   CEIL(CUBIC_SHARP_RANGE  )

static INLINE Flt Cubic(Flt x, const Flt blur, const Flt sharpen)
{
   Flt x2=x*x,
       x3=x*x*x;
   return (x<=1) ? ((12-9*blur-6*sharpen)/6*x3 + (-18+12*blur+6*sharpen)/6*x2 +                             (6-2*blur         )/6)
                 : ((-blur-6*sharpen    )/6*x3 + (6*blur+30*sharpen    )/6*x2 + (-12*blur-48*sharpen)/6*x + (8*blur+24*sharpen)/6);
}
static INLINE Flt CatmullRom       (Flt x) {return Cubic(x, 0.0f  , 0.5f  );}
static INLINE Flt MitchellNetravali(Flt x) {return Cubic(x, 1.0f/3, 1.0f/3);}
static INLINE Flt Robidoux         (Flt x) {return Cubic(x, 12/(19+9*SQRT2), 113/(58+216*SQRT2));}
static INLINE Flt RobidouxSharp    (Flt x) {return Cubic(x,  6/(13+7*SQRT2),   7/( 2+ 12*SQRT2));}

static INLINE Flt CubicFast      (Flt x) {return Cubic(x, 1.0f/3, 1.0f/3);}
static INLINE Flt CubicFastSmooth(Flt x) {return Cubic(x, 1.0f  , 0.000f);}
static INLINE Flt CubicFastSharp (Flt x) {return Cubic(x, 0.0f  , 0.500f);}
static INLINE Flt CubicMed       (Flt x) {return Cubic(x, 0.0f  , 0.400f);}
static INLINE Flt CubicSharp     (Flt x) {return Cubic(x, 0.0f  , 0.500f);}

static Flt CubicFast2      (Flt xx) {return CubicFast      (SqrtFast(xx));}
static Flt CubicFastSmooth2(Flt xx) {return CubicFastSmooth(SqrtFast(xx));}
static Flt CubicFastSharp2 (Flt xx) {return CubicFastSharp (SqrtFast(xx));}
static Flt CubicMed2       (Flt xx) {return CubicMed       (SqrtFast(xx));}
static Flt CubicSharp2     (Flt xx) {return CubicSharp     (SqrtFast(xx));}
/******************************************************************************/
#define SINC_RANGE      2
#define JINC_HALF_RANGE 1.2196698912665045f
#define JINC_RANGE      2.233130455f
#define JINC_SMOOTH     0.42f // 0.0 (sharp) .. 0.5 (smooth)
#define JINC_SAMPLES    CEIL(JINC_RANGE)

//static inline Flt sinc(Flt x) {return x ? sin(x)/x : 1.0f;}
//static inline Flt jinc(Flt x) {return x ?  j1(x)/x : 0.5f;}

static INLINE Flt SincSinc(Flt x)
{
 //return Sinc(x)*Sinc(x*JINC_SMOOTH);
   x*=PI; Flt xx=Sqr(x); return xx ? Sin(x)*Sin(x*JINC_SMOOTH)/xx : JINC_SMOOTH;
}
static INLINE Flt JincJinc(Flt x)
{
 //return Jinc(x)*Jinc(x*JINC_SMOOTH);
   x*=PI; Flt xx=Sqr(x); return xx ? J1(x)*J1(x*JINC_SMOOTH)/xx : 0.25f*JINC_SMOOTH;
}
static Flt SincSinc2(Flt xx) {return SincSinc(SqrtFast(xx));}
static Flt JincJinc2(Flt xx) {return JincJinc(SqrtFast(xx));}
/******************************************************************************/
static void Add(Vec4 &color, Vec &rgb, C Vec4 &sample, Bool alpha_weight) // here 'weight'=1
{
   if(alpha_weight)
   {
         rgb      +=sample.xyz         ; // RGB
         color.xyz+=sample.xyz*sample.w; // RGB*Alpha
         color.w  +=           sample.w; //     Alpha
   }else color    +=sample             ;
}
static void Add(Vec4 &color, Vec &rgb, C Vec4 &sample, Flt weight, Bool alpha_weight)
{
   if(alpha_weight)
   {
         rgb      +=sample.xyz*weight; // RGB*      weight
         weight   *=sample.w         ; // adjust 'weight' by Alpha
         color.xyz+=sample.xyz*weight; // RGB*Alpha*weight
         color.w  +=           weight; //     Alpha*weight
   }else color    +=sample    *weight;
}
static void Normalize(Vec4 &color, C Vec &rgb, Bool alpha_weight, Bool high_precision) // here 'weight'=1
{
   if(alpha_weight)
   {
      // normalize
      if(color.w>0)color.xyz/=color.w;

      // for low opacities we need to preserve the original RGB, because, at opacity=0 RGB information is totally lost, and because the image may be used for drawing, in which case the GPU will use bilinear filtering, and we need to make sure that mostly transparent pixel RGB values aren't artificially upscaled too much, also because at low opacities the alpha precision is low (because normally images are in 8-bit format, and we get alpha values like 1,2,3,..) and because we need to divide by alpha, inaccuracies may occur
      if(color.w<ALPHA_LIMIT)
      {
         if(color.w<=0)
         {
            color.w=0; // 'color.w' can be negative due to sharpening
            color.xyz=rgb;
         }else
         if(!high_precision) // no need to do this for high precision
         {
            Flt blend=color.w/ALPHA_LIMIT; // color.xyz = Lerp(rgb, color.xyz, blend);
            color.xyz*=blend;
            color.xyz+=rgb*(1-blend);
         }
      }
   }
}
static void Normalize(Vec4 &color, C Vec &rgb, Flt weight, Bool alpha_weight, Bool high_precision) // warning: 'weight' must be non-zero
{
   if(alpha_weight)
   {
      // normalize
      if(color.w>0)
      {
         color.xyz/=color.w;
         color.w  /=weight;
      }

      // for low opacities we need to preserve the original RGB, because, at opacity=0 RGB information is totally lost, and because the image may be used for drawing, in which case the GPU will use bilinear filtering, and we need to make sure that mostly transparent pixel RGB values aren't artificially upscaled too much, also because at low opacities the alpha precision is low (because normally images are in 8-bit format, and we get alpha values like 1,2,3,..) and because we need to divide by alpha, inaccuracies may occur
      if(color.w<ALPHA_LIMIT)
      {
         if(color.w<=0)
         {
            color.w=0; // 'color.w' can be negative due to sharpening
            color.xyz=rgb/weight;
         }else
         if(!high_precision) // no need to do this for high precision
         {
            Flt blend=color.w/ALPHA_LIMIT; // color.xyz = Lerp(rgb, color.xyz, blend);
            color.xyz*=blend;
            color.xyz+=rgb*((1-blend)/weight);
         }
      }
   }else color/=weight;
}
/******************************************************************************/
// PIXEL / COLOR
/******************************************************************************/
void Image::pixel(Int x, Int y, UInt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(bytePP())
      {
         case 1: (*(U8 *)data)=pixel&0xFF; break;
         case 2: (*(U16*)data)=pixel&0xFFFF; break;
         case 3: (*(U16*)data)=pixel&0xFFFF; data[2]=(pixel>>16)&0xFF; break;
         case 4: (*(U32*)data)=pixel; break;
      }
   }
}
/******************************************************************************/
void Image::pixel3D(Int x, Int y, Int z, UInt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Byte *data=T.data() + x*bytePP() + y*pitch() + z*pitch2();
      switch(bytePP())
      {
         case 1: (*(U8 *)data)=pixel&0xFF; break;
         case 2: (*(U16*)data)=pixel&0xFFFF; break;
         case 3: (*(U16*)data)=pixel&0xFFFF; data[2]=(pixel>>16)&0xFF; break;
         case 4: (*(U32*)data)=pixel; break;
      }
   }
}
/******************************************************************************/
UInt Image::pixel(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(bytePP())
      {
         case 1: return *(U8 *)data;
         case 2: return *(U16*)data;
         case 3: return *(U16*)data | (data[2]<<16);
         case 4: return *(U32*)data;
      }
   }
   return 0;
}
/******************************************************************************/
UInt Image::pixel3D(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch() + z*pitch2();
      switch(bytePP())
      {
         case 1: return *(U8 *)data;
         case 2: return *(U16*)data;
         case 3: return *(U16*)data | (data[2]<<16);
         case 4: return *(U32*)data;
      }
   }
   return 0;
}
/******************************************************************************/
static void SetPixelF(Byte *data, IMAGE_TYPE type, Flt pixel)
{
   switch(type)
   {
      case IMAGE_F32  : (*(Flt *)data)=pixel; break;
      case IMAGE_F32_2: (*(Vec2*)data)=pixel; break;
      case IMAGE_F32_3: (*(Vec *)data)=pixel; break;
      case IMAGE_F32_4: (*(Vec4*)data)=pixel; break;

      case IMAGE_F16  : {U16 *d=(U16*)data; d[0]=               Half(pixel).data;} break;
      case IMAGE_F16_2: {U16 *d=(U16*)data; d[0]=d[1]=          Half(pixel).data;} break;
      case IMAGE_F16_3: {U16 *d=(U16*)data; d[0]=d[1]=d[2]=     Half(pixel).data;} break;
      case IMAGE_F16_4: {U16 *d=(U16*)data; d[0]=d[1]=d[2]=d[3]=Half(pixel).data;} break;

      case IMAGE_R8G8B8X8:
      case IMAGE_B8G8R8X8:
      case IMAGE_B8G8R8A8:
      case IMAGE_R8G8B8A8: {VecB4 &v=*(VecB4*)data; v.x=v.y=v.z=FltToByte(pixel); v.w=255;} break;
      case IMAGE_B8G8R8  :
      case IMAGE_R8G8B8  : {VecB  &v=*(VecB *)data; v.x=v.y=v.z=FltToByte(pixel);} break;
      case IMAGE_R8G8    : {VecB2 &v=*(VecB2*)data; v.x=v.y=    FltToByte(pixel);} break;

      case IMAGE_R8_SIGN      : {Byte  &v=*(Byte *)data; v  =        SFltToSByte(pixel);         } break;
      case IMAGE_R8G8_SIGN    : {VecB2 &v=*(VecB2*)data; v.x=v.y=    SFltToSByte(pixel);         } break;
      case IMAGE_R8G8B8A8_SIGN: {VecB4 &v=*(VecB4*)data; v.x=v.y=v.z=SFltToSByte(pixel); v.w=255;} break;

      case IMAGE_R10G10B10A2: {UInt v=Mid(RoundPos(pixel*0x3FF), 0, 0x3FF); (*(UInt*)data)=v|(v<<10)|(v<<20)|(3<<30);} break;

      case IMAGE_R8 :
      case IMAGE_A8 :
      case IMAGE_L8 :
      case IMAGE_I8 : (*(U8 *)data)=FltToByte( pixel)             ; break; // it's okay   to clamp int for small  values
      case IMAGE_I16: (*(U16*)data)=RoundU(Sat(pixel)*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundU(Sat(pixel)*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU(Sat(pixel)*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values
   }
}
void Image::pixelF(Int x, Int y, Flt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixelF(data() + x*bytePP() + y*pitch(), hwType(), pixel);
}
/******************************************************************************/
void Image::pixel3DF(Int x, Int y, Int z, Flt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixelF(data() + x*bytePP() + y*pitch() + z*pitch2(), hwType(), pixel);
}
/******************************************************************************/
Flt Image::pixelF(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_F32  : return *(Flt*)data;
         case IMAGE_F32_2: return *(Flt*)data;
         case IMAGE_F32_3: return *(Flt*)data;
         case IMAGE_F32_4: return *(Flt*)data;

         case IMAGE_F16  : return *(Half*)data;
         case IMAGE_F16_2: return *(Half*)data;
         case IMAGE_F16_3: return *(Half*)data;
         case IMAGE_F16_4: return *(Half*)data;

         case IMAGE_B8G8R8  :
         case IMAGE_B8G8R8X8:
         case IMAGE_B8G8R8A8: return ((VecB4*)data)->z/Flt(0xFF);

         case IMAGE_R8      :
         case IMAGE_R8G8    :
         case IMAGE_R8G8B8  :
         case IMAGE_R8G8B8X8:
         case IMAGE_R8G8B8A8:
         case IMAGE_A8      :
         case IMAGE_L8      :
         case IMAGE_L8A8    :
         case IMAGE_I8      : return (*(U8*)data)/Flt(0x000000FFu);

         case IMAGE_D16: if(GL)return (*(U16*)data)/Flt(0x0000FFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I16:       return (*(U16*)data)/Flt(0x0000FFFFu);

         case IMAGE_D32 :       return GL ? (*(Flt*)data)*2-1 : *(Flt*)data;
       //case IMAGE_D32I: if(GL)return (*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I32 :       return (*(U32*)data)/Dbl(0xFFFFFFFFu); // Dbl required to get best precision

         case IMAGE_D24S8:
         case IMAGE_D24X8: if(GL)return (*(U16*)(data+1) | (data[3]<<16))/Flt(0x00FFFFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I24  :       return (*(U16*) data    | (data[2]<<16))/Flt(0x00FFFFFFu)    ; // here Dbl is not required, this was tested

         case IMAGE_R10G10B10A2: return ((*(UInt*)data)&0x3FF)/Flt(0x3FF);

         case IMAGE_R8_SIGN      :
         case IMAGE_R8G8_SIGN    :
         case IMAGE_R8G8B8A8_SIGN: return SByteToSFlt(*(U8*)data);

         case IMAGE_BC1     :
         case IMAGE_BC2     :
         case IMAGE_BC3     :
         case IMAGE_BC7     :
         case IMAGE_PVRTC1_2:
         case IMAGE_PVRTC1_4:
         case IMAGE_ETC1    :
         case IMAGE_ETC2    :
         case IMAGE_ETC2_A1 :
         case IMAGE_ETC2_A8 : return decompress(x, y).r/255.0f;

         case IMAGE_B4G4R4X4: return (((*(U16*)data)>> 8)&0x0F)/15.0f;
         case IMAGE_B4G4R4A4: return (((*(U16*)data)>> 8)&0x0F)/15.0f;
         case IMAGE_B5G5R5X1: return (((*(U16*)data)>>10)&0x1F)/31.0f;
         case IMAGE_B5G5R5A1: return (((*(U16*)data)>>10)&0x1F)/31.0f;
         case IMAGE_B5G6R5  : return (((*(U16*)data)>>11)&0x1F)/31.0f;
      }
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixel3DF(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch() + z*pitch2();
      switch(hwType())
      {
         case IMAGE_F32  : return *(Flt*)data;
         case IMAGE_F32_2: return *(Flt*)data;
         case IMAGE_F32_3: return *(Flt*)data;
         case IMAGE_F32_4: return *(Flt*)data;

         case IMAGE_F16  : return *(Half*)data;
         case IMAGE_F16_2: return *(Half*)data;
         case IMAGE_F16_3: return *(Half*)data;
         case IMAGE_F16_4: return *(Half*)data;

         case IMAGE_B8G8R8  :
         case IMAGE_B8G8R8X8:
         case IMAGE_B8G8R8A8: return ((VecB4*)data)->z/Flt(0xFF);

         case IMAGE_R8      :
         case IMAGE_R8G8    :
         case IMAGE_R8G8B8  :
         case IMAGE_R8G8B8X8:
         case IMAGE_R8G8B8A8:
         case IMAGE_A8      :
         case IMAGE_L8      :
         case IMAGE_L8A8    :
         case IMAGE_I8      : return (*(U8*)data)/Flt(0x000000FFu);

         case IMAGE_D16: if(GL)return (*(U16*)data)/Flt(0x0000FFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I16:       return (*(U16*)data)/Flt(0x0000FFFFu);

         case IMAGE_D32 :       return GL ? (*(Flt*)data)*2-1 : *(Flt*)data;
       //case IMAGE_D32I: if(GL)return (*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I32 :       return (*(U32*)data)/Dbl(0xFFFFFFFFu); // Dbl required to get best precision

         case IMAGE_D24S8:
         case IMAGE_D24X8: if(GL)return (*(U16*)(data+1) | (data[3]<<16))/Flt(0x00FFFFFFu)*2-1; // !! else fall through no break on purpose !!
         case IMAGE_I24  :       return (*(U16*) data    | (data[2]<<16))/Flt(0x00FFFFFFu)    ; // here Dbl is not required, this was tested

         case IMAGE_R10G10B10A2: return ((*(UInt*)data)&0x3FF)/Flt(0x3FF);

         case IMAGE_R8_SIGN      :
         case IMAGE_R8G8_SIGN    :
         case IMAGE_R8G8B8A8_SIGN: return SByteToSFlt(*(U8*)data);

         case IMAGE_BC1     :
         case IMAGE_BC2     :
         case IMAGE_BC3     :
         case IMAGE_BC7     :
         case IMAGE_PVRTC1_2:
         case IMAGE_PVRTC1_4:
         case IMAGE_ETC1    :
         case IMAGE_ETC2    :
         case IMAGE_ETC2_A1 :
         case IMAGE_ETC2_A8 : return decompress3D(x, y, z).r/255.0f;

         case IMAGE_B4G4R4X4: return (((*(U16*)data)>> 8)&0x0F)/15.0f;
         case IMAGE_B4G4R4A4: return (((*(U16*)data)>> 8)&0x0F)/15.0f;
         case IMAGE_B5G5R5X1: return (((*(U16*)data)>>10)&0x1F)/31.0f;
         case IMAGE_B5G5R5A1: return (((*(U16*)data)>>10)&0x1F)/31.0f;
         case IMAGE_B5G6R5  : return (((*(U16*)data)>>11)&0x1F)/31.0f;
      }
   }
   return 0;
}
/******************************************************************************/
static void SetColorF(Byte *data, IMAGE_TYPE type, C Vec4 &color)
{
   switch(type)
   {
      case IMAGE_F32  : (*(Flt *)data)=color.x  ; break;
      case IMAGE_F32_2: (*(Vec2*)data)=color.xy ; break;
      case IMAGE_F32_3: (*(Vec *)data)=color.xyz; break;
      case IMAGE_F32_4: (*(Vec4*)data)=color    ; break;

      case IMAGE_F16  : (*(Half *)data)=color.x  ; break;
      case IMAGE_F16_2: (*(VecH2*)data)=color.xy ; break;
      case IMAGE_F16_3: (*(VecH *)data)=color.xyz; break;
      case IMAGE_F16_4: (*(VecH4*)data)=color    ; break;

      case IMAGE_A8 : (*(U8 *)data)=FltToByte( color.w             ); break; // it's okay   to clamp int for small  values
      case IMAGE_L8 : (*(U8 *)data)=FltToByte( color.xyz.max()     ); break; // it's okay   to clamp int for small  values
      case IMAGE_I8 : (*(U8 *)data)=FltToByte( color.x             ); break; // it's okay   to clamp int for small  values
      case IMAGE_I16: (*(U16*)data)=RoundU(Sat(color.x)*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundU(Sat(color.x)*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU(Sat(color.x)*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values

      case IMAGE_L8A8: ((VecB2*)data)->set(FltToByte(color.xyz.max()), FltToByte(color.w)); break;

      case IMAGE_B8G8R8A8: ((VecB4*)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8: ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z), FltToByte(color.w)); break;
      case IMAGE_R8G8B8  : ((VecB *)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)                    ); break;
      case IMAGE_R8G8    : ((VecB2*)data)->set(FltToByte(color.x), FltToByte(color.y)                                        ); break;
      case IMAGE_R8      : *(Byte *)data  =    FltToByte(color.x)                                                             ; break;
      case IMAGE_B8G8R8  : ((VecB *)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x)                    ); break;
      case IMAGE_B8G8R8X8: ((VecB4*)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x),                255); break;
      case IMAGE_R8G8B8X8: ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z),                255); break;

      case IMAGE_R8_SIGN      : (*(Byte *)data)=     SFltToSByte(color.x)                                                                   ; break;
      case IMAGE_R8G8_SIGN    : ( (VecB2*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y)                                            ); break;
      case IMAGE_R8G8B8A8_SIGN: ( (VecB4*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y), SFltToSByte(color.z), SFltToSByte(color.w)); break;

      case IMAGE_R10G10B10A2: {(*(UInt*)data)=Mid(RoundPos(color.x*0x3FF), 0, 0x3FF)|(Mid(RoundPos(color.y*0x3FF), 0, 0x3FF)<<10)|(Mid(RoundPos(color.z*0x3FF), 0, 0x3FF)<<20)|(Mid(RoundPos(color.w*3), 0, 3)<<30);} break;
   }
}
static void SetColorF(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color)
{
   if(type==hw_type)normal: return SetColorF(data, hw_type, color); // first check if types are the same, the most common case
   Vec4 c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_R8G8B8:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : c.set(color.x, color.y, color.z, 1); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2: c.set(color.x, color.y, 0, 1); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32: c.set(color.x, 0, 0, 1); break;

      case IMAGE_A8  :                         c.set(0, 0, 0, color.w);  break;
      case IMAGE_L8  : {Flt l=color.xyz.max(); c.set(l, l, l,       1);} break;
      case IMAGE_L8A8: {Flt l=color.xyz.max(); c.set(l, l, l, color.w);} break;

      default: goto normal;
   }
   SetColorF(data, hw_type, c);
}
void Image::colorF(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorF(data() + x*bytePP() + y*pitch(), type(), hwType(), color);
}
void Image::color3DF(Int x, Int y, Int z, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorF(data() + x*bytePP() + y*pitch() + z*pitch2(), type(), hwType(), color);
}
/******************************************************************************/
static inline void ApplyBlend(Vec4 &src, C Vec4 &color) {src=Blend(src, color);}
void Image::blend(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src;
      Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; src.set(c.x/255.0f, c.y/255.0f, c.z/255.0f, c.w/255.0f); ApplyBlend(src, color); c.set(FltToByte(src.x), FltToByte(src.y), FltToByte(src.z), FltToByte(src.w));} break;
         case IMAGE_F32_4   : ApplyBlend(*(Vec4*)data, color); break;
         default            :
         {
            src=colorF(x, y);
            ApplyBlend(src, color);
            SetColorF(data, type(), hwType(), src);
         }break;
      }
   }
}
static inline void ApplyMerge(Vec4 &src, C Vec4 &color) {src=PremultipliedBlend(src, color);}
void Image::merge(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src;
      Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; src.set(c.x/255.0f, c.y/255.0f, c.z/255.0f, c.w/255.0f); ApplyMerge(src, color); c.set(FltToByte(src.x), FltToByte(src.y), FltToByte(src.z), FltToByte(src.w));} break;
         case IMAGE_F32_4   : ApplyMerge(*(Vec4*)data, color); break;
         default            :
         {
            src=colorF(x, y);
            ApplyMerge(src, color);
            SetColorF(data, type(), hwType(), src);
         }break;
      }
   }
}
/******************************************************************************/
Vec4 Image::colorF(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_F32  : return Vec4(*(Flt *)data, 0, 0, 1);
         case IMAGE_F32_2: return Vec4(*(Vec2*)data,    0, 1);
         case IMAGE_F32_3: return Vec4(*(Vec *)data,       1);
         case IMAGE_F32_4: return      *(Vec4*)data          ;

         case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1); break;
         case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1); break;
         case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1); break;
         case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]); break;

         case IMAGE_A8: return Vec4(0, 0, 0, Flt(*(U8*)data)/0x000000FFu);
         case IMAGE_L8: return Vec4(Vec(     Flt(*(U8*)data)/0x000000FFu), 1);
         case IMAGE_I8: return Vec4(Vec(     Flt(*(U8*)data)/0x000000FFu), 1);

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I16:       return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)    ), 1);

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D32 :       return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
       //case IMAGE_D32I: if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I32 :       return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)    ), 1); // Dbl required to get best precision

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D24S8:
         case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(data+1) | (data[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I24  :       return Vec4(Vec((*(U16*) data    | (data[2]<<16))/Flt(0x00FFFFFFu)    ), 1); // here Dbl is not required, this was tested

         case IMAGE_L8A8: {VecB2 &c=*(VecB2*)data; Flt l=c.x/255.0f; return Vec4(l, l, l, c.y/255.0f);}

         case IMAGE_B8G8R8A8: {VecB4 &c=*(VecB4*)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f, c.w/255.0f);}
         case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f, c.w/255.0f);}
         case IMAGE_B8G8R8X8: {VecB4 &c=*(VecB4*)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f,          1);}
         case IMAGE_R8G8B8X8: {VecB4 &c=*(VecB4*)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f,          1);}
         case IMAGE_B8G8R8  : {VecB  &c=*(VecB *)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f,          1);}
         case IMAGE_R8G8B8  : {VecB  &c=*(VecB *)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f,          1);}
         case IMAGE_R8G8    : {VecB2 &c=*(VecB2*)data; return Vec4(c.x/255.0f, c.y/255.0f,          0,          1);}
         case IMAGE_R8      : {Byte   c=*(Byte *)data; return Vec4(c  /255.0f,          0,          0,          1);}

         case IMAGE_R8_SIGN      : {Byte  &c=*(Byte *)data; return Vec4(SByteToSFlt(c  ),                0,               0 ,               1 );}
         case IMAGE_R8G8_SIGN    : {VecB2 &c=*(VecB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),               0 ,               1 );}
         case IMAGE_R8G8B8A8_SIGN: {VecB4 &c=*(VecB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

         case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4((u&0x3FF)/1023.0f, ((u>>10)&0x3FF)/1023.0f, ((u>>20)&0x3FF)/1023.0f, (u>>30)/3.0f);}

         case IMAGE_BC1     :
         case IMAGE_BC2     :
         case IMAGE_BC3     :
         case IMAGE_BC7     :
         case IMAGE_PVRTC1_2:
         case IMAGE_PVRTC1_4:
         case IMAGE_ETC1    :
         case IMAGE_ETC2    :
         case IMAGE_ETC2_A1 :
         case IMAGE_ETC2_A8 : return decompress(x, y).asVec4();
      }
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::color3DF(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
    C Byte *data=T.data() + x*bytePP() + y*pitch() + z*pitch2();
      switch(hwType())
      {
         case IMAGE_F32  : return Vec4(*(Flt *)data, 0, 0, 1);
         case IMAGE_F32_2: return Vec4(*(Vec2*)data,    0, 1);
         case IMAGE_F32_3: return Vec4(*(Vec *)data,       1);
         case IMAGE_F32_4: return      *(Vec4*)data          ;

         case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1); break;
         case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1); break;
         case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1); break;
         case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]); break;

         case IMAGE_A8: return Vec4(0, 0, 0, Flt(*(U8*)data)/0x000000FFu);
         case IMAGE_L8: return Vec4(Vec(     Flt(*(U8*)data)/0x000000FFu), 1);
         case IMAGE_I8: return Vec4(Vec(     Flt(*(U8*)data)/0x000000FFu), 1);

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I16:       return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)    ), 1);

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D32 :       return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
       //case IMAGE_D32I: if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I32 :       return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)    ), 1); // Dbl required to get best precision

      #if SUPPORT_DEPTH_TO_COLOR
         case IMAGE_D24S8:
         case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(data+1) | (data[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
      #endif
         case IMAGE_I24  :       return Vec4(Vec((*(U16*) data    | (data[2]<<16))/Flt(0x00FFFFFFu)    ), 1); // here Dbl is not required, this was tested

         case IMAGE_L8A8: {VecB2 &c=*(VecB2*)data; Flt l=c.x/255.0f; return Vec4(l, l, l, c.y/255.0f);}

         case IMAGE_B8G8R8A8: {VecB4 &c=*(VecB4*)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f, c.w/255.0f);}
         case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f, c.w/255.0f);}
         case IMAGE_B8G8R8X8: {VecB4 &c=*(VecB4*)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f,          1);}
         case IMAGE_R8G8B8X8: {VecB4 &c=*(VecB4*)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f,          1);}
         case IMAGE_B8G8R8  : {VecB  &c=*(VecB *)data; return Vec4(c.z/255.0f, c.y/255.0f, c.x/255.0f,          1);}
         case IMAGE_R8G8B8  : {VecB  &c=*(VecB *)data; return Vec4(c.x/255.0f, c.y/255.0f, c.z/255.0f,          1);}
         case IMAGE_R8G8    : {VecB2 &c=*(VecB2*)data; return Vec4(c.x/255.0f, c.y/255.0f,          0,          1);}
         case IMAGE_R8      : {Byte   c=*(Byte *)data; return Vec4(c  /255.0f,          0,          0,          1);}

         case IMAGE_R8_SIGN      : {Byte  &c=*(Byte *)data; return Vec4(SByteToSFlt(c  ),                0,               0 ,               1 );}
         case IMAGE_R8G8_SIGN    : {VecB2 &c=*(VecB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),               0 ,               1 );}
         case IMAGE_R8G8B8A8_SIGN: {VecB4 &c=*(VecB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

         case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4((u&0x3FF)/1023.0f, ((u>>10)&0x3FF)/1023.0f, ((u>>20)&0x3FF)/1023.0f, (u>>30)/3.0f);}

         case IMAGE_BC1     :
         case IMAGE_BC2     :
         case IMAGE_BC3     :
         case IMAGE_BC7     :
         case IMAGE_PVRTC1_2:
         case IMAGE_PVRTC1_4:
         case IMAGE_ETC1    :
         case IMAGE_ETC2    :
         case IMAGE_ETC2_A1 :
         case IMAGE_ETC2_A8 : return decompress3D(x, y, z).asVec4();
      }
   }
   return 0;
}
/******************************************************************************/
// LINEAR
/******************************************************************************/
Flt Image::pixelFLinear(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
      }
      Flt p[2][2]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]

      return p[0][0]*(1-x)*(1-y)
            +p[0][1]*(  x)*(1-y)
            +p[1][0]*(1-x)*(  y)
            +p[1][1]*(  x)*(  y);
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixel3DFLinear(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      Int zo[2]; zo[0]=Floor(z); z-=zo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
         zo[1]=zo[0]+1; if(zo[1]<0)zo[0]=zo[1]=0;else if(zo[0]>=ld())zo[0]=zo[1]=ld()-1;else if(zo[0]<0)zo[0]=0;else if(zo[1]>=ld())zo[1]=ld()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld();
      }

      Flt p[2][2][2]; gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]

      return p[0][0][0]*(1-x)*(1-y)*(1-z)
            +p[0][0][1]*(  x)*(1-y)*(1-z)
            +p[0][1][0]*(1-x)*(  y)*(1-z)
            +p[0][1][1]*(  x)*(  y)*(1-z)
            +p[1][0][0]*(1-x)*(1-y)*(  z)
            +p[1][0][1]*(  x)*(1-y)*(  z)
            +p[1][1][0]*(1-x)*(  y)*(  z)
            +p[1][1][1]*(  x)*(  y)*(  z);
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFLinear(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
      }

      Vec4 c[2][2]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      if(alpha_weight)
      {
         Vec  rgb  =0;
         Vec4 color=0;
         Add(color, rgb, c[0][0], (1-x)*(1-y), alpha_weight);
         Add(color, rgb, c[0][1], (  x)*(1-y), alpha_weight);
         Add(color, rgb, c[1][0], (1-x)*(  y), alpha_weight);
         Add(color, rgb, c[1][1], (  x)*(  y), alpha_weight);
         Normalize(color, rgb, alpha_weight, highPrecision());
         return color;
      }else
         return c[0][0]*(1-x)*(1-y)
               +c[0][1]*(  x)*(1-y)
               +c[1][0]*(1-x)*(  y)
               +c[1][1]*(  x)*(  y);
   }
   return 0;
}
Vec4 Image::colorFLinearTTNF32_4(Flt x, Flt y, Bool clamp)C // !! this assumes that image is already locked, exists and is of F32_4 type
{
   Int xo[2]; xo[0]=Floor(x); x-=xo[0];
   Int yo[2]; yo[0]=Floor(y); y-=yo[0];
   if(clamp)
   {
      xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
      yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
   }else
   {
      xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
      yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
   }

 C Vec4 &c00=pixF4(xo[0], yo[0]),
        &c01=pixF4(xo[1], yo[0]),
        &c10=pixF4(xo[0], yo[1]),
        &c11=pixF4(xo[1], yo[1]);
   Vec4 color=0;
   if(c00.w>0 || c01.w>0 || c10.w>0 || c11.w>0) // we're only interested in interpolation if there will be any alpha
   {
      Vec rgb=0;
      Add(color, rgb, c00, (1-x)*(1-y), true);
      Add(color, rgb, c01, (  x)*(1-y), true);
      Add(color, rgb, c10, (1-x)*(  y), true);
      Add(color, rgb, c11, (  x)*(  y), true);
      Normalize(color, rgb, true, true);
   }
   return color;
}
/******************************************************************************/
Vec4 Image::color3DFLinear(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      Int zo[2]; zo[0]=Floor(z); z-=zo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
         zo[1]=zo[0]+1; if(zo[1]<0)zo[0]=zo[1]=0;else if(zo[0]>=ld())zo[0]=zo[1]=ld()-1;else if(zo[0]<0)zo[0]=0;else if(zo[1]>=ld())zo[1]=ld()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld();
      }

      Vec4 c[2][2][2]; gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      if(alpha_weight)
      {
         Vec  rgb  =0;
         Vec4 color=0;
         Add(color, rgb, c[0][0][0], (1-x)*(1-y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][0][1], (  x)*(1-y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][1][0], (1-x)*(  y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][1][1], (  x)*(  y)*(1-z), alpha_weight);
         Add(color, rgb, c[1][0][0], (1-x)*(1-y)*(  z), alpha_weight);
         Add(color, rgb, c[1][0][1], (  x)*(1-y)*(  z), alpha_weight);
         Add(color, rgb, c[1][1][0], (1-x)*(  y)*(  z), alpha_weight);
         Add(color, rgb, c[1][1][1], (  x)*(  y)*(  z), alpha_weight);
         Normalize(color, rgb, alpha_weight, highPrecision());
         return color;
      }else
         return c[0][0][0]*(1-x)*(1-y)*(1-z)
               +c[0][0][1]*(  x)*(1-y)*(1-z)
               +c[0][1][0]*(1-x)*(  y)*(1-z)
               +c[0][1][1]*(  x)*(  y)*(1-z)
               +c[1][0][0]*(1-x)*(1-y)*(  z)
               +c[1][0][1]*(  x)*(1-y)*(  z)
               +c[1][1][0]*(1-x)*(  y)*(  z)
               +c[1][1][1]*(  x)*(  y)*(  z);
   }
   return 0;
}
/******************************************************************************/
// CUBIC FAST
/******************************************************************************/
Flt Image::pixelFCubicFast(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicFastSmooth(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicFastSharp(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFCubicFast(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::colorFCubicFastSmooth(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::colorFCubicFastSharp(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixel3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFast2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSmooth2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSharp2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixelFCubic(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[CUBIC_MED_SAMPLES*2], yo[CUBIC_MED_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[CUBIC_MED_SAMPLES*2], yw[CUBIC_MED_SAMPLES*2];
      Flt p [CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_MED_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_MED_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt weight=0, v=0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_MED_RANGE))
         {
            w=CubicMed2(w*Sqr(CUBIC_MED_SHARPNESS)); v+=p[y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicSharp(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[CUBIC_SHARP_SAMPLES*2], yo[CUBIC_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[CUBIC_SHARP_SAMPLES*2], yw[CUBIC_SHARP_SAMPLES*2];
      Flt p [CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt weight=0, v=0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_SHARP_RANGE))
         {
            w=CubicSharp2(w*Sqr(CUBIC_SHARP_SHARPNESS)); v+=p[y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubic(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_MED_SAMPLES*2], yo[CUBIC_MED_SAMPLES*2], zo[CUBIC_MED_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_MED_SAMPLES*2], yw[CUBIC_MED_SAMPLES*2], zw[CUBIC_MED_SAMPLES*2];
      Flt p [CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_MED_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_MED_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_MED_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_MED_RANGE))
         {
            w=CubicMed2(w*Sqr(CUBIC_MED_SHARPNESS)); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicSharp(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_SHARP_SAMPLES*2], yo[CUBIC_SHARP_SAMPLES*2], zo[CUBIC_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_SHARP_SAMPLES*2], yw[CUBIC_SHARP_SAMPLES*2], zw[CUBIC_SHARP_SAMPLES*2];
      Flt p [CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_SHARP_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_SHARP_RANGE))
         {
            w=CubicSharp2(w*Sqr(CUBIC_SHARP_SHARPNESS)); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFCubic(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[CUBIC_MED_SAMPLES*2], yo[CUBIC_MED_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[CUBIC_MED_SAMPLES*2], yw[CUBIC_MED_SAMPLES*2];
      Vec4 c [CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_MED_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_MED_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_MED_RANGE))
         {
            w=CubicMed2(w*Sqr(CUBIC_MED_SHARPNESS)); Add(color, rgb, c[y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::colorFCubicSharp(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[CUBIC_SHARP_SAMPLES*2], yo[CUBIC_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[CUBIC_SHARP_SAMPLES*2], yw[CUBIC_SHARP_SAMPLES*2];
      Vec4 c [CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_SHARP_RANGE))
         {
            w=CubicSharp2(w*Sqr(CUBIC_SHARP_SHARPNESS)); Add(color, rgb, c[y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubic(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_MED_SAMPLES*2], yo[CUBIC_MED_SAMPLES*2], zo[CUBIC_MED_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_MED_SAMPLES*2], yw[CUBIC_MED_SAMPLES*2], zw[CUBIC_MED_SAMPLES*2];
      Vec4 c [CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_MED_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_MED_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_MED_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_MED_RANGE))
         {
            w=CubicMed2(w*Sqr(CUBIC_MED_SHARPNESS)); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicSharp(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_SHARP_SAMPLES*2], yo[CUBIC_SHARP_SAMPLES*2], zo[CUBIC_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_SHARP_SAMPLES*2], yw[CUBIC_SHARP_SAMPLES*2], zw[CUBIC_SHARP_SAMPLES*2];
      Vec4 c [CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2][CUBIC_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_SHARP_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_SHARP_RANGE))
         {
            w=CubicSharp2(w*Sqr(CUBIC_SHARP_SHARPNESS)); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFast2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSmooth2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSharp2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************/
// CUBIC ORTHO
/******************************************************************************
Flt Image::pixelFCubicOrtho(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
   #if 0
      return Lerp4(
               Lerp4(p[0][0], p[0][1], p[0][2], p[0][3], x),
               Lerp4(p[1][0], p[1][1], p[1][2], p[1][3], x),
               Lerp4(p[2][0], p[2][1], p[2][2], p[2][3], x),
               Lerp4(p[3][0], p[3][1], p[3][2], p[3][3], x), y);
   #else // optimized
      Vec4 xb; Lerp4Weights(xb, x);
      return Lerp4(
               p[0][0]*xb.x + p[0][1]*xb.y + p[0][2]*xb.z + p[0][3]*xb.w,
               p[1][0]*xb.x + p[1][1]*xb.y + p[1][2]*xb.z + p[1][3]*xb.w,
               p[2][0]*xb.x + p[2][1]*xb.y + p[2][2]*xb.z + p[2][3]*xb.w,
               p[3][0]*xb.x + p[3][1]*xb.y + p[3][2]*xb.z + p[3][3]*xb.w, y);
   #endif
   }
   return 0;
}
/******************************************************************************
Flt Image::pixel3DFCubicOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      Int zo[4]; zo[0]=Floor(z); z-=zo[0]; zo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
         zo[1]=Mid(zo[0]+1, 0, ld()-1); zo[2]=Mid(zo[0]+2, 0, ld()-1); zo[3]=Mid(zo[0]+3, 0, ld()-1); Clamp(zo[0], 0, ld()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld(); zo[2]=(zo[0]+2)%ld(); zo[3]=(zo[0]+3)%ld();
      }

      Flt p000=pixel3DF(xo[0], yo[0], zo[0]), p100=pixel3DF(xo[1], yo[0], zo[0]), p200=pixel3DF(xo[2], yo[0], zo[0]), p300=pixel3DF(xo[3], yo[0], zo[0]),
          p010=pixel3DF(xo[0], yo[1], zo[0]), p110=pixel3DF(xo[1], yo[1], zo[0]), p210=pixel3DF(xo[2], yo[1], zo[0]), p310=pixel3DF(xo[3], yo[1], zo[0]),
          p020=pixel3DF(xo[0], yo[2], zo[0]), p120=pixel3DF(xo[1], yo[2], zo[0]), p220=pixel3DF(xo[2], yo[2], zo[0]), p320=pixel3DF(xo[3], yo[2], zo[0]),
          p030=pixel3DF(xo[0], yo[3], zo[0]), p130=pixel3DF(xo[1], yo[3], zo[0]), p230=pixel3DF(xo[2], yo[3], zo[0]), p330=pixel3DF(xo[3], yo[3], zo[0]),

          p001=pixel3DF(xo[0], yo[0], zo[1]), p101=pixel3DF(xo[1], yo[0], zo[1]), p201=pixel3DF(xo[2], yo[0], zo[1]), p301=pixel3DF(xo[3], yo[0], zo[1]),
          p011=pixel3DF(xo[0], yo[1], zo[1]), p111=pixel3DF(xo[1], yo[1], zo[1]), p211=pixel3DF(xo[2], yo[1], zo[1]), p311=pixel3DF(xo[3], yo[1], zo[1]),
          p021=pixel3DF(xo[0], yo[2], zo[1]), p121=pixel3DF(xo[1], yo[2], zo[1]), p221=pixel3DF(xo[2], yo[2], zo[1]), p321=pixel3DF(xo[3], yo[2], zo[1]),
          p031=pixel3DF(xo[0], yo[3], zo[1]), p131=pixel3DF(xo[1], yo[3], zo[1]), p231=pixel3DF(xo[2], yo[3], zo[1]), p331=pixel3DF(xo[3], yo[3], zo[1]),

          p002=pixel3DF(xo[0], yo[0], zo[2]), p102=pixel3DF(xo[1], yo[0], zo[2]), p202=pixel3DF(xo[2], yo[0], zo[2]), p302=pixel3DF(xo[3], yo[0], zo[2]),
          p012=pixel3DF(xo[0], yo[1], zo[2]), p112=pixel3DF(xo[1], yo[1], zo[2]), p212=pixel3DF(xo[2], yo[1], zo[2]), p312=pixel3DF(xo[3], yo[1], zo[2]),
          p022=pixel3DF(xo[0], yo[2], zo[2]), p122=pixel3DF(xo[1], yo[2], zo[2]), p222=pixel3DF(xo[2], yo[2], zo[2]), p322=pixel3DF(xo[3], yo[2], zo[2]),
          p032=pixel3DF(xo[0], yo[3], zo[2]), p132=pixel3DF(xo[1], yo[3], zo[2]), p232=pixel3DF(xo[2], yo[3], zo[2]), p332=pixel3DF(xo[3], yo[3], zo[2]),

          p003=pixel3DF(xo[0], yo[0], zo[3]), p103=pixel3DF(xo[1], yo[0], zo[3]), p203=pixel3DF(xo[2], yo[0], zo[3]), p303=pixel3DF(xo[3], yo[0], zo[3]),
          p013=pixel3DF(xo[0], yo[1], zo[3]), p113=pixel3DF(xo[1], yo[1], zo[3]), p213=pixel3DF(xo[2], yo[1], zo[3]), p313=pixel3DF(xo[3], yo[1], zo[3]),
          p023=pixel3DF(xo[0], yo[2], zo[3]), p123=pixel3DF(xo[1], yo[2], zo[3]), p223=pixel3DF(xo[2], yo[2], zo[3]), p323=pixel3DF(xo[3], yo[2], zo[3]),
          p033=pixel3DF(xo[0], yo[3], zo[3]), p133=pixel3DF(xo[1], yo[3], zo[3]), p233=pixel3DF(xo[2], yo[3], zo[3]), p333=pixel3DF(xo[3], yo[3], zo[3]);

   #if 0
      return Lerp4(
                Lerp4(
                  Lerp4(p000, p100, p200, p300, x),
                  Lerp4(p010, p110, p210, p310, x),
                  Lerp4(p020, p120, p220, p320, x),
                  Lerp4(p030, p130, p230, p330, x), y),

                Lerp4(
                  Lerp4(p001, p101, p201, p301, x),
                  Lerp4(p011, p111, p211, p311, x),
                  Lerp4(p021, p121, p221, p321, x),
                  Lerp4(p031, p131, p231, p331, x), y),

                Lerp4(
                  Lerp4(p002, p102, p202, p302, x),
                  Lerp4(p012, p112, p212, p312, x),
                  Lerp4(p022, p122, p222, p322, x),
                  Lerp4(p032, p132, p232, p332, x), y),

                Lerp4(
                  Lerp4(p003, p103, p203, p303, x),
                  Lerp4(p013, p113, p213, p313, x),
                  Lerp4(p023, p123, p223, p323, x),
                  Lerp4(p033, p133, p233, p333, x), y), z);
   #else // optimized
      Vec4 xb, yb; Lerp4Weights(xb, x);
                   Lerp4Weights(yb, y);
      return Lerp4(
                (p000*xb.x + p100*xb.y + p200*xb.z + p300*xb.w)*yb.x
               +(p010*xb.x + p110*xb.y + p210*xb.z + p310*xb.w)*yb.y
               +(p020*xb.x + p120*xb.y + p220*xb.z + p320*xb.w)*yb.z
               +(p030*xb.x + p130*xb.y + p230*xb.z + p330*xb.w)*yb.w,

                (p001*xb.x + p101*xb.y + p201*xb.z + p301*xb.w)*yb.x
               +(p011*xb.x + p111*xb.y + p211*xb.z + p311*xb.w)*yb.y
               +(p021*xb.x + p121*xb.y + p221*xb.z + p321*xb.w)*yb.z
               +(p031*xb.x + p131*xb.y + p231*xb.z + p331*xb.w)*yb.w,

                (p002*xb.x + p102*xb.y + p202*xb.z + p302*xb.w)*yb.x
               +(p012*xb.x + p112*xb.y + p212*xb.z + p312*xb.w)*yb.y
               +(p022*xb.x + p122*xb.y + p222*xb.z + p322*xb.w)*yb.z
               +(p032*xb.x + p132*xb.y + p232*xb.z + p332*xb.w)*yb.w,

                (p003*xb.x + p103*xb.y + p203*xb.z + p303*xb.w)*yb.x
               +(p013*xb.x + p113*xb.y + p213*xb.z + p313*xb.w)*yb.y
               +(p023*xb.x + p123*xb.y + p223*xb.z + p323*xb.w)*yb.z
               +(p033*xb.x + p133*xb.y + p233*xb.z + p333*xb.w)*yb.w, z);
   #endif
   }
   return 0;
}
/******************************************************************************
Flt Image::pixelFLanczosOrtho(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], xs=0, ys=0;
      Flt p [LANCZOS_SAMPLES*2][LANCZOS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt fy=0; REPAD(y, yo)
      {
         Flt fx=0; REPAD(x, xo)fx+=p[y][x]*xw[x];
         fy+=fx*yw[y];
      }
      return fy/(xs*ys);
   }
   return 0;
}
/******************************************************************************
Vec4 Image::colorFCubicOrtho(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb  =0;
      Vec4 color=0;
      Vec4 xb; Lerp4Weights(xb, x);
      Vec4 yb; Lerp4Weights(yb, y);
      Add(color, rgb, c[0][0], xb.x*yb.x, alpha_weight);
      Add(color, rgb, c[0][1], xb.y*yb.x, alpha_weight);
      Add(color, rgb, c[0][2], xb.z*yb.x, alpha_weight);
      Add(color, rgb, c[0][3], xb.w*yb.x, alpha_weight);

      Add(color, rgb, c[1][0], xb.x*yb.y, alpha_weight);
      Add(color, rgb, c[1][1], xb.y*yb.y, alpha_weight);
      Add(color, rgb, c[1][2], xb.z*yb.y, alpha_weight);
      Add(color, rgb, c[1][3], xb.w*yb.y, alpha_weight);

      Add(color, rgb, c[2][0], xb.x*yb.z, alpha_weight);
      Add(color, rgb, c[2][1], xb.y*yb.z, alpha_weight);
      Add(color, rgb, c[2][2], xb.z*yb.z, alpha_weight);
      Add(color, rgb, c[2][3], xb.w*yb.z, alpha_weight);

      Add(color, rgb, c[3][0], xb.x*yb.w, alpha_weight);
      Add(color, rgb, c[3][1], xb.y*yb.w, alpha_weight);
      Add(color, rgb, c[3][2], xb.z*yb.w, alpha_weight);
      Add(color, rgb, c[3][3], xb.w*yb.w, alpha_weight);
      Normalize(color, rgb, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::color3DFCubicOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      Int zo[4]; zo[0]=Floor(z); z-=zo[0]; zo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
         zo[1]=Mid(zo[0]+1, 0, ld()-1); zo[2]=Mid(zo[0]+2, 0, ld()-1); zo[3]=Mid(zo[0]+3, 0, ld()-1); Clamp(zo[0], 0, ld()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld(); zo[2]=(zo[0]+2)%ld(); zo[3]=(zo[0]+3)%ld();
      }

      Vec4 c000=color3DF(xo[0], yo[0], zo[0]), c100=color3DF(xo[1], yo[0], zo[0]), c200=color3DF(xo[2], yo[0], zo[0]), c300=color3DF(xo[3], yo[0], zo[0]),
           c010=color3DF(xo[0], yo[1], zo[0]), c110=color3DF(xo[1], yo[1], zo[0]), c210=color3DF(xo[2], yo[1], zo[0]), c310=color3DF(xo[3], yo[1], zo[0]),
           c020=color3DF(xo[0], yo[2], zo[0]), c120=color3DF(xo[1], yo[2], zo[0]), c220=color3DF(xo[2], yo[2], zo[0]), c320=color3DF(xo[3], yo[2], zo[0]),
           c030=color3DF(xo[0], yo[3], zo[0]), c130=color3DF(xo[1], yo[3], zo[0]), c230=color3DF(xo[2], yo[3], zo[0]), c330=color3DF(xo[3], yo[3], zo[0]),

           c001=color3DF(xo[0], yo[0], zo[1]), c101=color3DF(xo[1], yo[0], zo[1]), c201=color3DF(xo[2], yo[0], zo[1]), c301=color3DF(xo[3], yo[0], zo[1]),
           c011=color3DF(xo[0], yo[1], zo[1]), c111=color3DF(xo[1], yo[1], zo[1]), c211=color3DF(xo[2], yo[1], zo[1]), c311=color3DF(xo[3], yo[1], zo[1]),
           c021=color3DF(xo[0], yo[2], zo[1]), c121=color3DF(xo[1], yo[2], zo[1]), c221=color3DF(xo[2], yo[2], zo[1]), c321=color3DF(xo[3], yo[2], zo[1]),
           c031=color3DF(xo[0], yo[3], zo[1]), c131=color3DF(xo[1], yo[3], zo[1]), c231=color3DF(xo[2], yo[3], zo[1]), c331=color3DF(xo[3], yo[3], zo[1]),

           c002=color3DF(xo[0], yo[0], zo[2]), c102=color3DF(xo[1], yo[0], zo[2]), c202=color3DF(xo[2], yo[0], zo[2]), c302=color3DF(xo[3], yo[0], zo[2]),
           c012=color3DF(xo[0], yo[1], zo[2]), c112=color3DF(xo[1], yo[1], zo[2]), c212=color3DF(xo[2], yo[1], zo[2]), c312=color3DF(xo[3], yo[1], zo[2]),
           c022=color3DF(xo[0], yo[2], zo[2]), c122=color3DF(xo[1], yo[2], zo[2]), c222=color3DF(xo[2], yo[2], zo[2]), c322=color3DF(xo[3], yo[2], zo[2]),
           c032=color3DF(xo[0], yo[3], zo[2]), c132=color3DF(xo[1], yo[3], zo[2]), c232=color3DF(xo[2], yo[3], zo[2]), c332=color3DF(xo[3], yo[3], zo[2]),

           c003=color3DF(xo[0], yo[0], zo[3]), c103=color3DF(xo[1], yo[0], zo[3]), c203=color3DF(xo[2], yo[0], zo[3]), c303=color3DF(xo[3], yo[0], zo[3]),
           c013=color3DF(xo[0], yo[1], zo[3]), c113=color3DF(xo[1], yo[1], zo[3]), c213=color3DF(xo[2], yo[1], zo[3]), c313=color3DF(xo[3], yo[1], zo[3]),
           c023=color3DF(xo[0], yo[2], zo[3]), c123=color3DF(xo[1], yo[2], zo[3]), c223=color3DF(xo[2], yo[2], zo[3]), c323=color3DF(xo[3], yo[2], zo[3]),
           c033=color3DF(xo[0], yo[3], zo[3]), c133=color3DF(xo[1], yo[3], zo[3]), c233=color3DF(xo[2], yo[3], zo[3]), c333=color3DF(xo[3], yo[3], zo[3]);

   #if 0
      return Lerp4(
                Lerp4(
                  Lerp4(c000, c100, c200, c300, x),
                  Lerp4(c010, c110, c210, c310, x),
                  Lerp4(c020, c120, c220, c320, x),
                  Lerp4(c030, c130, c230, c330, x), y),

                Lerp4(
                  Lerp4(c001, c101, c201, c301, x),
                  Lerp4(c011, c111, c211, c311, x),
                  Lerp4(c021, c121, c221, c321, x),
                  Lerp4(c031, c131, c231, c331, x), y),

                Lerp4(
                  Lerp4(c002, c102, c202, c302, x),
                  Lerp4(c012, c112, c212, c312, x),
                  Lerp4(c022, c122, c222, c322, x),
                  Lerp4(c032, c132, c232, c332, x), y),

                Lerp4(
                  Lerp4(c003, c103, c203, c303, x),
                  Lerp4(c013, c113, c213, c313, x),
                  Lerp4(c023, c123, c223, c323, x),
                  Lerp4(c033, c133, c233, c333, x), y), z);
   #else // optimized
      Vec4 xb, yb; Lerp4Weights(xb, x);
                   Lerp4Weights(yb, y);
      return Lerp4(
                (c000*xb.x + c100*xb.y + c200*xb.z + c300*xb.w)*yb.x
               +(c010*xb.x + c110*xb.y + c210*xb.z + c310*xb.w)*yb.y
               +(c020*xb.x + c120*xb.y + c220*xb.z + c320*xb.w)*yb.z
               +(c030*xb.x + c130*xb.y + c230*xb.z + c330*xb.w)*yb.w,

                (c001*xb.x + c101*xb.y + c201*xb.z + c301*xb.w)*yb.x
               +(c011*xb.x + c111*xb.y + c211*xb.z + c311*xb.w)*yb.y
               +(c021*xb.x + c121*xb.y + c221*xb.z + c321*xb.w)*yb.z
               +(c031*xb.x + c131*xb.y + c231*xb.z + c331*xb.w)*yb.w,

                (c002*xb.x + c102*xb.y + c202*xb.z + c302*xb.w)*yb.x
               +(c012*xb.x + c112*xb.y + c212*xb.z + c312*xb.w)*yb.y
               +(c022*xb.x + c122*xb.y + c222*xb.z + c322*xb.w)*yb.z
               +(c032*xb.x + c132*xb.y + c232*xb.z + c332*xb.w)*yb.w,

                (c003*xb.x + c103*xb.y + c203*xb.z + c303*xb.w)*yb.x
               +(c013*xb.x + c113*xb.y + c213*xb.z + c313*xb.w)*yb.y
               +(c023*xb.x + c123*xb.y + c223*xb.z + c323*xb.w)*yb.z
               +(c033*xb.x + c133*xb.y + c233*xb.z + c333*xb.w)*yb.w, z);
   #endif
   }
   return 0;
}
/******************************************************************************
Vec4 Image::colorFLanczosOrtho(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], xs=0, ys=0;
      Vec4 c [LANCZOS_SAMPLES*2][LANCZOS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb  =0;
      Vec4 color=0;
      REPAD(y, yo)
      REPAD(x, xo)Add(color, rgb, c[y][x], xw[x]*yw[y], alpha_weight);
      Normalize(color, rgb, alpha_weight, highPrecision());
      return color/(xs*ys);
   }
   return 0;
}
/******************************************************************************
Flt Image::pixel3DFLanczosOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], zc[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], zw[LANCZOS_SAMPLES*2], xs=0, ys=0, zs=0;
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         zc[i]=zi-LANCZOS_SAMPLES+1+i; zw[i]=LanczosSharp(z-zc[i]); zs+=zw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zc[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zc[i]=Mod(zc[i], ld());
         }
      }
      Flt fz=0; REPAD(z, zc)
      {
         Flt fy=0; REPAD(y, yo)
         {
            Flt fx=0; REPAD(x, xo)fx+=pixel3DF(xo[x], yo[y], zc[z])*xw[x];
            fy+=fx*yw[y];
         }
         fz+=fy*zw[z];
      }
      return fz/(xs*ys*zs);
   }
   return 0;
}
/******************************************************************************
Vec4 Image::color3DFLanczosOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], zc[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], zw[LANCZOS_SAMPLES*2], xs=0, ys=0, zs=0;
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         zc[i]=zi-LANCZOS_SAMPLES+1+i; zw[i]=LanczosSharp(z-zc[i]); zs+=zw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zc[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zc[i]=Mod(zc[i], ld());
         }
      }
      Vec4 cz=0; REPAD(z, zc)
      {
         Vec4 cy=0; REPAD(y, yo)
         {
            Vec4 cx=0; REPAD(x, xo)cx+=color3DF(xo[x], yo[y], zc[z])*xw[x];
            cy+=cx*yw[y];
         }
         cz+=cy*zw[z];
      }
      return cz/(xs*ys*zs);
   }
   return 0;
}
/******************************************************************************/
// AREA
/******************************************************************************/
Vec4 Image::areaColorAverage(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Rect_C rect (pos, Max(size.x-1, 0), Max(size.y-1, 0)); // 'size' here means scale, subtract 1 to convert to inclusive coordinates
      RectI  recti(Floor(rect.min.x), Floor(rect.min.y), Ceil(rect.max.x), Ceil(rect.max.y)); // inclusive coordinates. Have to use Ceil for max, because we need to process the neighbor pixel too (for example if rect.min.x=0.5 and rect.max.x=0.5, then we need to process both x=0 and x=1 pixels)
      Int    xo[2], yo[2];

      if(clamp)
      {
         xo[0]=Mid(recti.min.x, 0, lw()-1); yo[0]=Mid(recti.min.y, 0, lh()-1);
         xo[1]=Mid(recti.max.x, 0, lw()-1); yo[1]=Mid(recti.max.y, 0, lh()-1);
      }else
      {
         xo[0]=Mod(recti.min.x, lw()); yo[0]=Mod(recti.min.y, lh());
         xo[1]=Mod(recti.max.x, lw()); yo[1]=Mod(recti.max.y, lh());
      }

      if(recti.min==recti.max)return colorF(xo[0], yo[0]); // if coordinates are the same, then return this pixel

      // calculate blending factors
      Flt l=1+recti.min.x-rect.min.x, r=1+rect.max.x-recti.max.x, // l=1-(rect.min.x-recti.min.x), r=1-(recti.max.x-rect.max.x)
          t=1+recti.min.y-rect.min.y, b=1+rect.max.y-recti.max.y; // t=1-(rect.min.y-recti.min.y), b=1-(recti.max.y-rect.max.y)

      Vec  rgb  =0;
      Vec4 color=0;

      // add inside
      for(Int                                                y=recti.min.y+1; y<recti.max.y; y++)
      for(Int yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh())), x=recti.min.x+1; x<recti.max.x; x++)Add(color, rgb, colorF(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()), yo), alpha_weight);

      // add sides
      if(recti.min.y==recti.max.y)
      for(Int x=recti.min.x+1; x<recti.max.x; x++)
      {
         Int xo=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
         Add(color, rgb, colorF(xo, yo[0]), alpha_weight);
      }else
      for(Int x=recti.min.x+1; x<recti.max.x; x++)
      {
         Int  xo=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
         Vec4 c[2]; gather(c, &xo, 1, yo, Elms(yo));
         Add(color, rgb, c[0], t, alpha_weight); // top
         Add(color, rgb, c[1], b, alpha_weight); // bottom
      }

      if(recti.min.x==recti.max.x)
      for(Int y=recti.min.y+1; y<recti.max.y; y++)
      {
         Int yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         Add(color, rgb, colorF(xo[0], yo), alpha_weight);
      }else
      for(Int y=recti.min.y+1; y<recti.max.y; y++)
      {
         Int  yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         Vec4 c[2]; gather(c, xo, Elms(xo), &yo, 1);
         Add(color, rgb, c[0], l, alpha_weight); // left
         Add(color, rgb, c[1], r, alpha_weight); // right
      }

      // add corners
      if(recti.min.y==recti.max.y)
      {
         Vec4 c[2]; gather(&c[0], xo, Elms(xo), yo, 1);
         Add(color, rgb, c[0], l, alpha_weight);
         Add(color, rgb, c[1], r, alpha_weight);
      }else
      if(recti.min.x==recti.max.x)
      {
         Vec4 c[2]; gather(&c[0], xo, 1, yo, Elms(yo));
         Add(color, rgb, c[0], t, alpha_weight);
         Add(color, rgb, c[1], b, alpha_weight);
      }else
      {
         Vec4 c[2][2]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         Add(color, rgb, c[0][0], l*t, alpha_weight);
         Add(color, rgb, c[0][1], r*t, alpha_weight);
         Add(color, rgb, c[1][0], l*b, alpha_weight);
         Add(color, rgb, c[1][1], r*b, alpha_weight);
      }

      Normalize(color, rgb, (rect.w()+1)*(rect.h()+1), alpha_weight, highPrecision()); // weight is always non-zero here
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorLinear(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C // this is orthogonal
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 s(Max(1, size.x*0.75f), Max(1, size.y*0.75f)); // 0.5 is too sharp, 1.0 is too blurry, 0.75 is best and gives same results as Avg(a,b)
      Vec2 x_mul_add; x_mul_add.x=1/s.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=1/s.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-s.x), x1=FloorSpecial(pos.x+s.x),
          y0=CeilSpecial(pos.y-s.y), y1=FloorSpecial(pos.y+s.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=Linear(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=Linear(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorCubicFast(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFastSmooth2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubicFastSharp(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFastSharp2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubic(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_MED_RANGE, size.x*CUBIC_MED_RANGE), Max(CUBIC_MED_RANGE, size.y*CUBIC_MED_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_MED_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_MED_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_MED_RANGE))
            {
               w=CubicMed2(w*Sqr(CUBIC_MED_SHARPNESS)); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubicSharp(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_SHARP_RANGE, size.x*CUBIC_SHARP_RANGE), Max(CUBIC_SHARP_RANGE, size.y*CUBIC_SHARP_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_SHARP_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_SHARP_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_SHARP_RANGE))
            {
               w=CubicSharp2(w*Sqr(CUBIC_SHARP_SHARPNESS)); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::areaColorCubicOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_2(Max(2, size.x*2), Max(2, size.y*2));
      Vec2 x_mul_add; x_mul_add.x=2/size_2.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=2/size_2.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_2.x), x1=FloorSpecial(pos.x+size_2.x),
          y0=CeilSpecial(pos.y-size_2.y), y1=FloorSpecial(pos.y+size_2.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<4) // Cubic returns 0 for values >=2, since we use Sqr, check for 4
            {
               w=CubicSmoothFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubicOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_2(Max(2, size.x*2), Max(2, size.y*2));
      Vec2 x_mul_add; x_mul_add.x=2/size_2.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=2/size_2.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_2.x), x1=FloorSpecial(pos.x+size_2.x),
          y0=CeilSpecial(pos.y-size_2.y), y1=FloorSpecial(pos.y+size_2.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=CubicFastSharp(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=CubicFastSharp(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::areaColorCubicSharpGammaCorrect(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_2(Max(2, size.x*2), Max(2, size.y*2));
      Vec2 x_mul_add; x_mul_add.x=2/size_2.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=2/size_2.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_2.x), x1=FloorSpecial(pos.x+size_2.x),
          y0=CeilSpecial(pos.y-size_2.y), y1=FloorSpecial(pos.y+size_2.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;

      Vec  lin_srgb   =0;
      Vec4 lin_scolor =0;

      Flt  lin_weight=0;
      Vec  lin_rgb   =0;
      Vec4 lin_color =0;

      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<4) // Cubic returns 0 for values >=2, since we use Sqr, check for 4
            {
               Flt W=CubicSharpFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
               Vec4 c=colorF(xi, yi);
               Add(color, rgb, c, W, alpha_weight); weight+=W;
               if(w<1)
               {
                  w=Linear(SqrtFast(w));
                  Add(lin_scolor, lin_srgb, c, w, alpha_weight);
                  //c.x=SRGBToLinear(c.x);
                  //c.y=SRGBToLinear(c.y);
                  //c.z=SRGBToLinear(c.z);
                  c.x=Pow(c.x, 1.18f);
                  c.y=Pow(c.y, 1.18f);
                  c.z=Pow(c.z, 1.18f);
                  Add(lin_color, lin_rgb, c, w, alpha_weight); lin_weight+=w;
               }
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      Normalize(lin_scolor, lin_srgb, lin_weight, alpha_weight, highPrecision());
      Normalize(lin_color, lin_rgb, lin_weight, alpha_weight, highPrecision());
      //lin_color.x=LinearToSRGB(lin_color.x);
      //lin_color.y=LinearToSRGB(lin_color.y);
      //lin_color.z=LinearToSRGB(lin_color.z);
      lin_color.x=Pow(lin_color.x, 1.0f/1.18f);
      lin_color.y=Pow(lin_color.y, 1.0f/1.18f);
      lin_color.z=Pow(lin_color.z, 1.0f/1.18f);
      return lin_color+color-lin_scolor;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::areaColorLanczosOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(LANCZOS_SAMPLES, size.x*LANCZOS_SAMPLES), Max(LANCZOS_SAMPLES, size.y*LANCZOS_SAMPLES));
      Vec2 x_mul_add; x_mul_add.x=LANCZOS_SAMPLES/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=LANCZOS_SAMPLES/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=LanczosSharp(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=LanczosSharp(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, highPrecision());
      return color;
   }
   return 0;
}
/******************************************************************************/
#if 1 // faster, but reaches max 254 values instead of 255
static inline Byte SByteToByte(SByte s) {return (s<=0) ? 0 : (s<<1);}
#else
static inline Byte SByteToByte(SByte s) {return (s<=0) ? 0 : (s*255+63)/127;}
#endif
/******************************************************************************/
static Color PixelToColor(IMAGE_TYPE type, UInt pixel) // convert pixel to color
{
   switch(type)
   {
      case IMAGE_B8G8R8A8: return Color((pixel>>16)&0xFF, (pixel>> 8)&0xFF,  pixel     &0xFF, pixel>>24);
      case IMAGE_R8G8B8A8: return Color( pixel     &0xFF, (pixel>> 8)&0xFF, (pixel>>16)&0xFF, pixel>>24);
      case IMAGE_R8G8B8  : return Color( pixel     &0xFF, (pixel>> 8)&0xFF, (pixel>>16)&0xFF,       255);
      case IMAGE_R8G8    : return Color( pixel     &0xFF, (pixel>> 8)&0xFF,                0,       255);
      case IMAGE_R8      : return Color( pixel     &0xFF,                0,                0,       255);
      case IMAGE_A8      : return Color(               0,                0,                0, pixel    );
      case IMAGE_L8      : return Color( pixel          ,  pixel          ,  pixel          ,       255);
      case IMAGE_L8A8    : return Color( pixel     &0xFF,  pixel     &0xFF,  pixel&0xFF     , pixel>> 8);
      case IMAGE_I8      : return Color( pixel          ,  pixel          ,  pixel          ,       255);
      case IMAGE_I16     : return Color( pixel>> 8      ,  pixel>>8       ,  pixel>>8       ,       255);
      case IMAGE_I24     : return Color( pixel>>16      ,  pixel>>16      ,  pixel>>16      ,       255);
      case IMAGE_I32     : return Color( pixel>>24      ,  pixel>>24      ,  pixel>>24      ,       255);
      default            : return Color(               0,                0,                0,         0);

      case IMAGE_BC1     :
      case IMAGE_BC2     :
      case IMAGE_BC3     :
      case IMAGE_BC7     :
      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4:
      case IMAGE_ETC1    :
      case IMAGE_ETC2    :
      case IMAGE_ETC2_A1 :
      case IMAGE_ETC2_A8 : return Color(pixel&0xFF, (pixel>>8)&0xFF, (pixel>>16)&0xFF, pixel>>24);

      case IMAGE_B4G4R4X4   : return Color(((pixel>> 8)&0x0F)<<4, ((pixel>> 4)&0x0F)<<4, ((pixel    )&0x0F)<<4,                       255);
      case IMAGE_B4G4R4A4   : return Color(((pixel>> 8)&0x0F)<<4, ((pixel>> 4)&0x0F)<<4, ((pixel    )&0x0F)<<4, ((pixel>>12)&0x0F)<<4    );
      case IMAGE_B5G5R5X1   : return Color(((pixel>>10)&0x1F)<<3, ((pixel>> 5)&0x1F)<<3, ((pixel    )&0x1F)<<3,                       255);
      case IMAGE_B5G5R5A1   : return Color(((pixel>>10)&0x1F)<<3, ((pixel>> 5)&0x1F)<<3, ((pixel    )&0x1F)<<3,  (pixel&0x8000) ? 255 : 0);
      case IMAGE_B5G6R5     : return Color(((pixel>>11)&0x1F)<<3, ((pixel>> 5)&0x3F)<<2, ((pixel    )&0x1F)<<3,                       255);
      case IMAGE_B8G8R8     : return Color( (pixel>>16)&0xFF    ,  (pixel>> 8)&0xFF    ,   pixel     &0xFF    ,                       255);
      case IMAGE_B8G8R8X8   : return Color( (pixel>>16)&0xFF    ,  (pixel>> 8)&0xFF    ,   pixel     &0xFF    ,                       255);
      case IMAGE_R8G8B8X8   : return Color( (pixel    )&0xFF    ,  (pixel>> 8)&0xFF    ,  (pixel>>16)&0xFF    ,                       255);
      case IMAGE_R10G10B10A2: return Color( (pixel>> 2)&0xFF    ,  (pixel>>12)&0xFF    ,  (pixel>>22)&0xFF    ,  (pixel>>30)*255/3       );

      case IMAGE_R8_SIGN      : return Color(SByteToByte(pixel&0xFF),                            0,                            0 ,                   255 );
      case IMAGE_R8G8_SIGN    : return Color(SByteToByte(pixel&0xFF), SByteToByte((pixel>>8)&0xFF),                            0 ,                   255 );
      case IMAGE_R8G8B8A8_SIGN: return Color(SByteToByte(pixel&0xFF), SByteToByte((pixel>>8)&0xFF), SByteToByte((pixel>>16)&0xFF), SByteToByte(pixel>>24));
   }
}
/******************************************************************************/
static UInt ColorToPixel(IMAGE_TYPE type, C Color &color) // convert color to pixel
{
   switch(type)
   {
      case IMAGE_B8G8R8A8: return color.b | (color.g<<8) | (color.r<<16) | (color.a<<24);
      case IMAGE_R8G8B8A8: return color.r | (color.g<<8) | (color.b<<16) | (color.a<<24);
      case IMAGE_R8G8B8  : return color.r | (color.g<<8) | (color.b<<16);
      case IMAGE_R8G8    : return color.r | (color.g<<8);
      case IMAGE_R8      : return color.r;
      case IMAGE_A8      : return color.a;
      case IMAGE_L8      : return color.lum();
      case IMAGE_L8A8    : return color.lum()|(color.a<<8);
      case IMAGE_I8      : return color.r;
      case IMAGE_I16     : return color.r<<8;
      case IMAGE_I24     : return color.r<<16;
      case IMAGE_I32     : return color.r<<24;
      default            : return 0;

      case IMAGE_BC1     :
      case IMAGE_BC2     :
      case IMAGE_BC3     :
      case IMAGE_BC7     :
      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4:
      case IMAGE_ETC1    :
      case IMAGE_ETC2    :
      case IMAGE_ETC2_A1 :
      case IMAGE_ETC2_A8 : return color.r | (color.g<<8) | (color.b<<16) | (color.a<<24);

      case IMAGE_B4G4R4X4: return ((color.r>>4)<< 8) | ((color.g>>4)<< 4) | (color.b>> 4) | 0xF000;
      case IMAGE_B4G4R4A4: return ((color.r>>4)<< 8) | ((color.g>>4)<< 4) | (color.b>> 4) | ((color.a>>4)<<12);
      case IMAGE_B5G5R5X1: return ((color.r>>3)<<10) | ((color.g>>3)<< 5) | (color.b>> 3) | 0x8000;
      case IMAGE_B5G5R5A1: return ((color.r>>3)<<10) | ((color.g>>3)<< 5) | (color.b>> 3) | ((color.a>>7)<<15);
      case IMAGE_B5G6R5  : return ((color.r>>3)<<11) | ((color.g>>2)<< 5) | (color.b>> 3);
      case IMAGE_B8G8R8  : return ( color.r    <<16) | ( color.g    << 8) | (color.b    );
      case IMAGE_B8G8R8X8: return ( color.r    <<16) | ( color.g    << 8) | (color.b    ) | 0xFF000000;
      case IMAGE_R8G8B8X8: return ( color.r        ) | ( color.g    << 8) | (color.b<<16) | 0xFF000000;

      case IMAGE_R8_SIGN      : return (color.r>>1);
      case IMAGE_R8G8_SIGN    : return (color.r>>1) | ((color.g>>1)<<8);
      case IMAGE_R8G8B8A8_SIGN: return (color.r>>1) | ((color.g>>1)<<8) | ((color.b>>1)<<16) | ((color.a>>1)<<24);

      case IMAGE_R10G10B10A2: return (color.r*1023+127)/255 | (((color.g*1023+127)/255)<<10) | (((color.b*1023+127)/255)<<20) | (((color.a*3+127)/255)<<30);
   }
}
static UInt ColorToPixel(IMAGE_TYPE type, IMAGE_TYPE hw_type, C Color &color) // convert color to pixel
{
   if(type==hw_type)normal: return ColorToPixel(hw_type, color); // first check if types are the same, the most common case
   Color c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_R8G8B8:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : c.set(color.r, color.g, color.b, 255); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2: c.set(color.r, color.g, 0, 255); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32: c.set(color.r, 0, 0, 255); break;

      case IMAGE_A8  : c.set(0, 0, 0    , color.a); break;
      case IMAGE_L8  : c.set(color.lum(),     255); break;
      case IMAGE_L8A8: c.set(color.lum(), color.a); break;

      default: goto normal;
   }
   return ColorToPixel(hw_type, c);
}
/******************************************************************************/
void  Image::color  (Int x, Int y,        C Color &color)  {                                           if(highPrecision())        colorF  (x, y,    color.asVec4());else pixel  (x, y,    ColorToPixel(type(), hwType(), color          ));}
void  Image::color3D(Int x, Int y, Int z, C Color &color)  {                                           if(highPrecision())        color3DF(x, y, z, color.asVec4());else pixel3D(x, y, z, ColorToPixel(type(), hwType(), color          ));}
Color Image::color  (Int x, Int y                       )C {return compressed() ? decompress  (x, y   ) : highPrecision() ? Color(colorF  (x, y                   )) :                    PixelToColor(        hwType(), pixel  (x, y   ));}
Color Image::color3D(Int x, Int y, Int z                )C {return compressed() ? decompress3D(x, y, z) : highPrecision() ? Color(color3DF(x, y, z                )) :                    PixelToColor(        hwType(), pixel3D(x, y, z));}
/******************************************************************************/
// COMPRESSION
/******************************************************************************/
Color Image::decompress(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh()))switch(hwType()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      case IMAGE_BC1    : return DecompressPixelBC1   (data() + (x>>2)* 8 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_BC2    : return DecompressPixelBC2   (data() + (x>>2)*16 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_BC3    : return DecompressPixelBC3   (data() + (x>>2)*16 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_BC7    : return DecompressPixelBC7   (data() + (x>>2)*16 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_ETC1   : return DecompressPixelETC1  (data() + (x>>2)* 8 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_ETC2   : return DecompressPixelETC2  (data() + (x>>2)* 8 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_ETC2_A1: return DecompressPixelETC2A1(data() + (x>>2)* 8 + (y>>2)*pitch(), x&3, y&3);
      case IMAGE_ETC2_A8: return DecompressPixelETC2A8(data() + (x>>2)*16 + (y>>2)*pitch(), x&3, y&3);
   }
   return TRANSPARENT;
}
Color Image::decompress3D(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()))switch(hwType()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      case IMAGE_BC1    : return DecompressPixelBC1   (data() + (x>>2)* 8 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_BC2    : return DecompressPixelBC2   (data() + (x>>2)*16 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_BC3    : return DecompressPixelBC3   (data() + (x>>2)*16 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_BC7    : return DecompressPixelBC7   (data() + (x>>2)*16 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_ETC1   : return DecompressPixelETC1  (data() + (x>>2)* 8 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_ETC2   : return DecompressPixelETC2  (data() + (x>>2)* 8 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_ETC2_A1: return DecompressPixelETC2A1(data() + (x>>2)* 8 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
      case IMAGE_ETC2_A8: return DecompressPixelETC2A8(data() + (x>>2)*16 + (y>>2)*pitch() + z*pitch2(), x&3, y&3);
   }
   return TRANSPARENT;
}
/******************************************************************************/
// GATHER
/******************************************************************************/
void Image::gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   switch(hwType())
   {
      case IMAGE_F32: FREPD(y, y_offsets)
      {
       C Flt *pixel=(Flt*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]];
      }break;

      case IMAGE_R8:
      case IMAGE_A8:
      case IMAGE_L8:
      case IMAGE_I8: FREPD(y, y_offsets)
      {
       C Byte *pixel=data()+y_offset[y]*pitch();
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Flt(0xFFu);
      }break;

      case IMAGE_I16: FREPD(y, y_offsets)
      {
       C U16 *pixel=(U16*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Flt(0xFFFFu);
      }break;

      case IMAGE_I32: FREPD(y, y_offsets)
      {
       C U32 *pixel=(U32*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Dbl(0xFFFFFFFFu); // Dbl required to get best precision
      }break;

      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; *pixels++=src.b/255.0f;}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; *pixels++=src.r/255.0f;}
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; *pixels++=src.x/255.0f;}
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*pixels++=pixelF(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.b, src.g, src.r);}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]].v3;
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color(x_offset[x], y_offset[y]).v3;
      }break;
   }
}
/******************************************************************************/
void Image::gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.b, src.g, src.r, src.a);}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(src.x, src.y, src.z);}
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.b/255.0f, src.g/255.0f, src.r/255.0f, src.a/255.0f);}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.r/255.0f, src.g/255.0f, src.b/255.0f, src.a/255.0f);}
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(src.x/255.0f, src.y/255.0f, src.z/255.0f, 1);}
      }break;

      case IMAGE_F32_4: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_F32_3: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(color[x_offset[x]], 1);
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=colorF(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   switch(hwType())
   {
      case IMAGE_F32: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Flt *pixel_y=(Flt*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]];
         }
      }break;

      case IMAGE_R8:
      case IMAGE_A8:
      case IMAGE_L8:
      case IMAGE_I8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Byte *pixel_y=data_z+y_offset[y]*pitch();
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Flt(0xFFu);
         }
      }break;

      case IMAGE_I16: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C U16 *pixel_y=(U16*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Flt(0xFFFFu);
         }
      }break;

      case IMAGE_I32: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C U32 *pixel_y=(U32*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Dbl(0xFFFFFFFFu); // Dbl required to get best precision
         }
      }break;

      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; *pixels++=src.b/255.0f;}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; *pixels++=src.r/255.0f;}
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; *pixels++=src.x/255.0f;}
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*pixels++=pixel3DF(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.b, src.g, src.r);}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]].v3;
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3D(x_offset[x], y_offset[y], z_offset[z]).v3;
      }break;
   }
}
/******************************************************************************/
void Image::gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.b, src.g, src.r, src.a);}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(src.x, src.y, src.z);}
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3D(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.b/255.0f, src.g/255.0f, src.r/255.0f, src.a/255.0f);}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.r/255.0f, src.g/255.0f, src.b/255.0f, src.a/255.0f);}
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(src.x/255.0f, src.y/255.0f, src.z/255.0f, 1);}
         }
      }break;

      case IMAGE_F32_4: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_F32_3: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(color_y[x_offset[x]], 1);
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3DF(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
static Bool NeedMultiChannel(IMAGE_TYPE src, IMAGE_TYPE dest)
{
   return ImageTI[src].channels>1 || src!=dest;
}
static INLINE void StorePixel(Image &image, Byte* &dest_data_y, Int x, Int y, Int z, Flt pixel)
{
   switch(image.hwType())
   {
      case IMAGE_I8 :
      case IMAGE_R8 :
      case IMAGE_L8 :
      case IMAGE_A8 : *dest_data_y++=FltToByte(pixel); break;
      case IMAGE_F32: *(Flt*)dest_data_y=pixel; dest_data_y+=SIZE(Flt); break;
      default       :  image.pixel3DF(x, y, z, pixel); break;
   }
}
static INLINE void StoreColor(Image &image, Byte* &dest_data_y, Int x, Int y, Int z, C Vec4 &color)
{
   if(image.type()==image.hwType())switch(image.hwType()) // check 'type' too in case we have to perform color adjustment
   {
      case IMAGE_I8      :
      case IMAGE_R8      : *dest_data_y++=FltToByte(color.x); return;

      case IMAGE_B8G8R8A8: ((VecB4*)dest_data_y)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x), FltToByte(color.w)); dest_data_y+=4; return;
      case IMAGE_R8G8B8A8: *(Color*)dest_data_y=color; dest_data_y+=4; return;
      case IMAGE_R8G8B8  :  dest_data_y[0]=FltToByte(color.x); dest_data_y[1]=FltToByte(color.y); dest_data_y[2]=FltToByte(color.z); dest_data_y+=3; return;
      case IMAGE_L8      : *dest_data_y++=FltToByte(color.xyz.max()); return;

      case IMAGE_L8A8    : *dest_data_y++=FltToByte(color.xyz.max()); // !! no break/return on purpose so we can set Alpha channel below !!
      case IMAGE_A8      : *dest_data_y++=FltToByte(color.w); return;
   }
   image.color3DF(x, y, z, color);
}
Bool Image::copySoft(Image &dest, FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool keep_edges, Flt sharp_smooth)C // this does not support compressed images
{
   if(this==&dest        )return true;
   if(cube()!=dest.cube())return false;

   REPD(f, faces())
   {
      if(!   T.lockRead(        0, (DIR_ENUM)f))             return false;
      if(!dest.lock(LOCK_WRITE, 0, (DIR_ENUM)f)){T.unlock(); return false;}

      if(T.size3()==dest.size3()) // no resize
      {
         if(T.hwType()==dest.hwType()  // no retype
         && T.  type()==dest.  type()) // check 'type' too in case we have to perform color adjustment
         {
            Int valid_blocks_y=ImageBlocksY(T.w(), T.h(), 0, T.hwType()); // use "w(), h()" instead of "hwW(), hwH()" to copy only valid pixels
            REPD(z, T.d())
            {
             C Byte *s=T   .data() + z*T   .pitch2();
               Byte *d=dest.data() + z*dest.pitch2();
               if(T.pitch()==dest.pitch() )CopyFast(d, s, Min(T.pitch2(), dest.pitch2()));
               else REPD(y, valid_blocks_y)CopyFast(d + y*dest.pitch(), s + y*T.pitch(), Min(T.pitch(), dest.pitch()));
               // TODO: we could zero remaining data to avoid garbage
            }
         }else // retype
         {
            if(T   .hwType()==IMAGE_R8G8B8
            && dest.hwType()==IMAGE_R8G8B8A8  // very common case for importing images
            && dest.  type()==IMAGE_R8G8B8A8) // check 'type' too in case we have to perform color adjustment
            {
               REPD(z, T.d())
               REPD(y, T.h())
               {
                C VecB  *s=(VecB *)(T   .data() + y*T   .pitch() + z*T   .pitch2());
                  VecB4 *d=(VecB4*)(dest.data() + y*dest.pitch() + z*dest.pitch2());
                  REPD(x, T.w()){(d++)->set(s->x, s->y, s->z, 255); s++;}
               }
            }else
            if(T   .hwType()==IMAGE_R8G8B8
            && dest.hwType()==IMAGE_B8G8R8A8  // very common case for importing images
            && dest.  type()==IMAGE_B8G8R8A8) // check 'type' too in case we have to perform color adjustment
            {
               REPD(z, T.d())
               REPD(y, T.h())
               {
                C VecB  *s=(VecB *)(T   .data() + y*T   .pitch() + z*T   .pitch2());
                  VecB4 *d=(VecB4*)(dest.data() + y*dest.pitch() + z*dest.pitch2());
                  REPD(x, T.w()){(d++)->set(s->z, s->y, s->x, 255); s++;}
               }
            }else
            if(T   .highPrecision()
            && dest.highPrecision()) // high precision requires FP
            {
               REPD(z, T.d())
               REPD(y, T.h())
               REPD(x, T.w())dest.color3DF(x, y, z, T.color3DF(x, y, z));
            }else
            {
               REPD(z, T.d())
               REPD(y, T.h())
               REPD(x, T.w())dest.color3D(x, y, z, T.color3D(x, y, z));
            }
         }
      }else
      if(filter==FILTER_NO_STRETCH)
      {
         if(dest.hwType()==IMAGE_R8G8B8A8  // common case for encoding BC7
         && dest.  type()==IMAGE_R8G8B8A8) // check 'type' too in case we have to perform color adjustment
         {
            Int x_blocks=dest.w()/4,
                y_blocks=dest.h()/4;
            REPD(z, dest.d())
            {
               Byte *dest_z=dest.data() + z*dest.pitch2();
               Int   zo    =(clamp ? Min(z, T.d()-1) : z%T.d());
               REPD(by, y_blocks)
               {
                  Int py=by*4, yo[4]; // pixel and offset
                  REPAO(yo)=(clamp ? Min(py+i, T.h()-1) : (py+i)%T.h());
                  Byte *dest_y=dest_z + py*dest.pitch();
                  REPD(bx, x_blocks)
                  {
                     Int px=bx*4, xo[4]; // pixel and offset
                     REPAO(xo)=(clamp ? Min(px+i, T.w()-1) : (px+i)%T.w());
                     Color col[4][4];
                     T.gather(col[0], xo, Elms(xo), yo, Elms(yo), &zo, 1);
                     Byte *dest_x=dest_y + px*SIZE(Color);
                     REP(4)CopyFast(dest_x + i*dest.pitch(), col[i], SIZE(Color)*4);
                  }
               }

               // process partial blocks
               x_blocks*=4;
               y_blocks*=4;

               // process right side (excluding shared corner)
               if(x_blocks!=dest.w())
                  for(Int y=       0; y<y_blocks; y++)
                  for(Int x=x_blocks; x<dest.w(); x++)dest.color3D(x, y, z, clamp ? T.color3D(Min(x, w()-1), Min(y, h()-1), zo) : T.color3D(x%w(), y%h(), zo));

               // process bottom side (including shared corner)
             //if(y_blocks!=dest.h()) not needed since we're starting with Y's and this will be checked on its own
                  for(Int y=y_blocks; y<dest.h(); y++)
                  for(Int x=       0; x<dest.w(); x++)dest.color3D(x, y, z, clamp ? T.color3D(Min(x, w()-1), Min(y, h()-1), zo) : T.color3D(x%w(), y%h(), zo));
            }
         }else
         if(T   .highPrecision()
         && dest.highPrecision()) // high precision requires FP
         {
            REPD(z, dest.d())
            REPD(y, dest.h())
            REPD(x, dest.w())dest.color3DF(x, y, z, clamp ? T.color3DF(Min(x, w()-1), Min(y, h()-1), Min(z, d()-1)) : T.color3DF(x%w(), y%h(), z%d()));
         }else
         {
            REPD(z, dest.d())
            REPD(y, dest.h())
            REPD(x, dest.w())dest.color3D(x, y, z, clamp ? T.color3D(Min(x, w()-1), Min(y, h()-1), Min(z, d()-1)) : T.color3D(x%w(), y%h(), z%d()));
         }
      }else // resize
      {
         if(!ImageTI[hwType()].a)alpha_weight=false; // disable 'alpha_weight' if the source doesn't have it
         Bool t_high_prec=T.highPrecision(), high_prec=(t_high_prec || dest.highPrecision());

         // check for optimized downscale
         if(dest.w()==Max(1, T.w()>>1)
         && dest.h()==Max(1, T.h()>>1)
         && dest.d()==Max(1, T.d()>>1) // 2x downsample
         && !keep_edges
         && (Equal(sharp_smooth, 1) || filter==FILTER_NONE)
         )
         {
            if(T.d()<=1) // 2D
            {
               switch(filter)
               {
                  case FILTER_NONE: REPD(y, dest.h())
                  {
                     Int yc=y*2;
                     if(high_prec)REPD(x, dest.w())dest.colorF(x, y, colorF(x*2, yc));
                     else         REPD(x, dest.w())dest.color (x, y, color (x*2, yc));
                  }goto finish;

                  case FILTER_LINEAR: REPD(y, dest.h())
                  {
                     Int yc[2]; yc[0]=y*2; yc[1]=(clamp ? Min(yc[0]+1, h()-1) : (yc[0]+1)%h()); // yc[0] is always OK
                     REPD(x, dest.w())
                     {
                        Int xc[2]; xc[0]=x*2; xc[1]=(clamp ? Min(xc[0]+1, w()-1) : (xc[0]+1)%w()); // xc[0] is always OK
                        if(high_prec)
                        {
                           Vec4 col, c[2][2]; gather(&c[0][0], xc, Elms(xc), yc, Elms(yc)); // [y][x]
                           if(!alpha_weight)
                           {
                              col.w=Avg(c[0][0].w, c[0][1].w, c[1][0].w, c[1][1].w);
                           linear_rgb_f:
                              col.x=Avg(c[0][0].x, c[0][1].x, c[1][0].x, c[1][1].x);
                              col.y=Avg(c[0][0].y, c[0][1].y, c[1][0].y, c[1][1].y);
                              col.z=Avg(c[0][0].z, c[0][1].z, c[1][0].z, c[1][1].z);
                           }else
                           {
                              Flt a=c[0][0].w+c[0][1].w+c[1][0].w+c[1][1].w;
                              if(!a){col.w=0; goto linear_rgb_f;}
                              col.w=a/4;
                              col.x=(c[0][0].x*c[0][0].w + c[0][1].x*c[0][1].w + c[1][0].x*c[1][0].w + c[1][1].x*c[1][1].w)/a;
                              col.y=(c[0][0].y*c[0][0].w + c[0][1].y*c[0][1].w + c[1][0].y*c[1][0].w + c[1][1].y*c[1][1].w)/a;
                              col.z=(c[0][0].z*c[0][0].w + c[0][1].z*c[0][1].w + c[1][0].z*c[1][0].w + c[1][1].z*c[1][1].w)/a;
                           }
                           dest.colorF(x, y, col);
                        }else
                        {
                           Color col, c[2][2]; gather(&c[0][0], xc, Elms(xc), yc, Elms(yc)); // [y][x]
                           if(!alpha_weight)
                           {
                              col.a=((c[0][0].a+c[0][1].a+c[1][0].a+c[1][1].a+2)>>2);
                           linear_rgb:
                              col.r=((c[0][0].r+c[0][1].r+c[1][0].r+c[1][1].r+2)>>2);
                              col.g=((c[0][0].g+c[0][1].g+c[1][0].g+c[1][1].g+2)>>2);
                              col.b=((c[0][0].b+c[0][1].b+c[1][0].b+c[1][1].b+2)>>2);
                           }else
                           {
                              UInt a=c[0][0].a+c[0][1].a+c[1][0].a+c[1][1].a;
                              if( !a){col.a=0; goto linear_rgb;}
                              col.a=((a+2)>>2); UInt a_2=a>>1;
                              col.r=(c[0][0].r*c[0][0].a + c[0][1].r*c[0][1].a + c[1][0].r*c[1][0].a + c[1][1].r*c[1][1].a + a_2)/a;
                              col.g=(c[0][0].g*c[0][0].a + c[0][1].g*c[0][1].a + c[1][0].g*c[1][0].a + c[1][1].g*c[1][1].a + a_2)/a;
                              col.b=(c[0][0].b*c[0][0].a + c[0][1].b*c[0][1].a + c[1][0].b*c[1][0].a + c[1][1].b*c[1][1].a + a_2)/a;
                           }
                           dest.color(x, y, col);
                        }
                     }
                  }goto finish;

                  case FILTER_BEST:
                  case FILTER_CUBIC_FAST_SHARP: ASSERT(FILTER_DOWN==FILTER_CUBIC_FAST_SHARP);
                  {
                     if(!high_prec)
                     {
                        REPD(y, dest.h())
                        {
                           Int yc[8]; yc[3]=y*2; // 'y[3]' is always OK
                           if(clamp){yc[0]=Max(yc[3]-3, 0  ); yc[1]=Max(yc[3]-2, 0  ); yc[2]=Max(yc[3]-1, 0  ); yc[4]=Min(yc[3]+1, h()-1); yc[5]=Min(yc[3]+2, h()-1); yc[6]=Min(yc[3]+3, h()-1); yc[7]=Min(yc[3]+4, h()-1);}
                           else     {yc[0]=Mod(yc[3]-3, h()); yc[1]=Mod(yc[3]-2, h()); yc[2]=Mod(yc[3]-1, h()); yc[4]=   (yc[3]+1)%h()   ; yc[5]=   (yc[3]+2)%h()   ; yc[6]=   (yc[3]+3)%h()   ; yc[7]=   (yc[3]+4)%h()   ;}
                           REPD(x, dest.w())
                           {
                              Int xc[8]; xc[3]=x*2; // 'x[3]' is always OK
                              if(clamp){xc[0]=Max(xc[3]-3, 0  ); xc[1]=Max(xc[3]-2, 0  ); xc[2]=Max(xc[3]-1, 0  ); xc[4]=Min(xc[3]+1, w()-1); xc[5]=Min(xc[3]+2, w()-1); xc[6]=Min(xc[3]+3, w()-1); xc[7]=Min(xc[3]+4, w()-1);}
                              else     {xc[0]=Mod(xc[3]-3, w()); xc[1]=Mod(xc[3]-2, w()); xc[2]=Mod(xc[3]-1, w()); xc[4]=   (xc[3]+1)%w()   ; xc[5]=   (xc[3]+2)%w()   ; xc[6]=   (xc[3]+3)%w()   ; xc[7]=   (xc[3]+4)%w()   ;}
                              Color col, c[8][8]; // [y][x]
                           #if 1 // read 8x8
                              gather(&c[0][0], xc, Elms(xc), yc, Elms(yc));
                           #else // read 4x1, 8x6, 4x1, performance is the same
                              gather(&c[0][2], xc+2, Elms(xc)-4, yc  , 1         ); // top
                              gather(&c[1][0], xc  , Elms(xc)  , yc+1, Elms(yc)-2); // center
                              gather(&c[7][2], xc+2, Elms(xc)-4, yc+7, 1         ); // bottom
                           #endif
                              if(!alpha_weight)
                              {
                                 col.a=Mid((/*c[0][0].a*CW8[0][0]  + c[0][1].a*CW8[0][1]*/+ c[0][2].a*CW8[0][2] + c[0][3].a*CW8[0][3] + c[0][4].a*CW8[0][4] + c[0][5].a*CW8[0][5] +/*c[0][6].a*CW8[0][6] +  c[0][7].a*CW8[0][7]*/
                                          /*+ c[1][0].a*CW8[1][0]*/+ c[1][1].a*CW8[1][1]  + c[1][2].a*CW8[1][2] + c[1][3].a*CW8[1][3] + c[1][4].a*CW8[1][4] + c[1][5].a*CW8[1][5] +  c[1][6].a*CW8[1][6] +/*c[1][7].a*CW8[1][7]*/
                                            + c[2][0].a*CW8[2][0]  + c[2][1].a*CW8[2][1]  + c[2][2].a*CW8[2][2] + c[2][3].a*CW8[2][3] + c[2][4].a*CW8[2][4] + c[2][5].a*CW8[2][5] +  c[2][6].a*CW8[2][6] +  c[2][7].a*CW8[2][7]
                                            + c[3][0].a*CW8[3][0]  + c[3][1].a*CW8[3][1]  + c[3][2].a*CW8[3][2] + c[3][3].a*CW8[3][3] + c[3][4].a*CW8[3][4] + c[3][5].a*CW8[3][5] +  c[3][6].a*CW8[3][6] +  c[3][7].a*CW8[3][7]
                                            + c[4][0].a*CW8[4][0]  + c[4][1].a*CW8[4][1]  + c[4][2].a*CW8[4][2] + c[4][3].a*CW8[4][3] + c[4][4].a*CW8[4][4] + c[4][5].a*CW8[4][5] +  c[4][6].a*CW8[4][6] +  c[4][7].a*CW8[4][7]
                                            + c[5][0].a*CW8[5][0]  + c[5][1].a*CW8[5][1]  + c[5][2].a*CW8[5][2] + c[5][3].a*CW8[5][3] + c[5][4].a*CW8[5][4] + c[5][5].a*CW8[5][5] +  c[5][6].a*CW8[5][6] +  c[5][7].a*CW8[5][7]
                                          /*+ c[6][0].a*CW8[6][0]*/+ c[6][1].a*CW8[6][1]  + c[6][2].a*CW8[6][2] + c[6][3].a*CW8[6][3] + c[6][4].a*CW8[6][4] + c[6][5].a*CW8[6][5] +  c[6][6].a*CW8[6][6] +/*c[6][7].a*CW8[6][7]*/
                                          /*+ c[7][0].a*CW8[7][0]  + c[7][1].a*CW8[7][1]*/+ c[7][2].a*CW8[7][2] + c[7][3].a*CW8[7][3] + c[7][4].a*CW8[7][4] + c[7][5].a*CW8[7][5] +/*c[7][6].a*CW8[7][6] +  c[7][7].a*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
                              cubic_sharp_rgb:
                                 col.r=Mid((/*c[0][0].r*CW8[0][0]  + c[0][1].r*CW8[0][1]*/+ c[0][2].r*CW8[0][2] + c[0][3].r*CW8[0][3] + c[0][4].r*CW8[0][4] + c[0][5].r*CW8[0][5] +/*c[0][6].r*CW8[0][6] +  c[0][7].r*CW8[0][7]*/
                                          /*+ c[1][0].r*CW8[1][0]*/+ c[1][1].r*CW8[1][1]  + c[1][2].r*CW8[1][2] + c[1][3].r*CW8[1][3] + c[1][4].r*CW8[1][4] + c[1][5].r*CW8[1][5] +  c[1][6].r*CW8[1][6] +/*c[1][7].r*CW8[1][7]*/
                                            + c[2][0].r*CW8[2][0]  + c[2][1].r*CW8[2][1]  + c[2][2].r*CW8[2][2] + c[2][3].r*CW8[2][3] + c[2][4].r*CW8[2][4] + c[2][5].r*CW8[2][5] +  c[2][6].r*CW8[2][6] +  c[2][7].r*CW8[2][7]
                                            + c[3][0].r*CW8[3][0]  + c[3][1].r*CW8[3][1]  + c[3][2].r*CW8[3][2] + c[3][3].r*CW8[3][3] + c[3][4].r*CW8[3][4] + c[3][5].r*CW8[3][5] +  c[3][6].r*CW8[3][6] +  c[3][7].r*CW8[3][7]
                                            + c[4][0].r*CW8[4][0]  + c[4][1].r*CW8[4][1]  + c[4][2].r*CW8[4][2] + c[4][3].r*CW8[4][3] + c[4][4].r*CW8[4][4] + c[4][5].r*CW8[4][5] +  c[4][6].r*CW8[4][6] +  c[4][7].r*CW8[4][7]
                                            + c[5][0].r*CW8[5][0]  + c[5][1].r*CW8[5][1]  + c[5][2].r*CW8[5][2] + c[5][3].r*CW8[5][3] + c[5][4].r*CW8[5][4] + c[5][5].r*CW8[5][5] +  c[5][6].r*CW8[5][6] +  c[5][7].r*CW8[5][7]
                                          /*+ c[6][0].r*CW8[6][0]*/+ c[6][1].r*CW8[6][1]  + c[6][2].r*CW8[6][2] + c[6][3].r*CW8[6][3] + c[6][4].r*CW8[6][4] + c[6][5].r*CW8[6][5] +  c[6][6].r*CW8[6][6] +/*c[6][7].r*CW8[6][7]*/
                                          /*+ c[7][0].r*CW8[7][0]  + c[7][1].r*CW8[7][1]*/+ c[7][2].r*CW8[7][2] + c[7][3].r*CW8[7][3] + c[7][4].r*CW8[7][4] + c[7][5].r*CW8[7][5] +/*c[7][6].r*CW8[7][6] +  c[7][7].r*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
                                 col.g=Mid((/*c[0][0].g*CW8[0][0]  + c[0][1].g*CW8[0][1]*/+ c[0][2].g*CW8[0][2] + c[0][3].g*CW8[0][3] + c[0][4].g*CW8[0][4] + c[0][5].g*CW8[0][5] +/*c[0][6].g*CW8[0][6] +  c[0][7].g*CW8[0][7]*/
                                          /*+ c[1][0].g*CW8[1][0]*/+ c[1][1].g*CW8[1][1]  + c[1][2].g*CW8[1][2] + c[1][3].g*CW8[1][3] + c[1][4].g*CW8[1][4] + c[1][5].g*CW8[1][5] +  c[1][6].g*CW8[1][6] +/*c[1][7].g*CW8[1][7]*/
                                            + c[2][0].g*CW8[2][0]  + c[2][1].g*CW8[2][1]  + c[2][2].g*CW8[2][2] + c[2][3].g*CW8[2][3] + c[2][4].g*CW8[2][4] + c[2][5].g*CW8[2][5] +  c[2][6].g*CW8[2][6] +  c[2][7].g*CW8[2][7]
                                            + c[3][0].g*CW8[3][0]  + c[3][1].g*CW8[3][1]  + c[3][2].g*CW8[3][2] + c[3][3].g*CW8[3][3] + c[3][4].g*CW8[3][4] + c[3][5].g*CW8[3][5] +  c[3][6].g*CW8[3][6] +  c[3][7].g*CW8[3][7]
                                            + c[4][0].g*CW8[4][0]  + c[4][1].g*CW8[4][1]  + c[4][2].g*CW8[4][2] + c[4][3].g*CW8[4][3] + c[4][4].g*CW8[4][4] + c[4][5].g*CW8[4][5] +  c[4][6].g*CW8[4][6] +  c[4][7].g*CW8[4][7]
                                            + c[5][0].g*CW8[5][0]  + c[5][1].g*CW8[5][1]  + c[5][2].g*CW8[5][2] + c[5][3].g*CW8[5][3] + c[5][4].g*CW8[5][4] + c[5][5].g*CW8[5][5] +  c[5][6].g*CW8[5][6] +  c[5][7].g*CW8[5][7]
                                          /*+ c[6][0].g*CW8[6][0]*/+ c[6][1].g*CW8[6][1]  + c[6][2].g*CW8[6][2] + c[6][3].g*CW8[6][3] + c[6][4].g*CW8[6][4] + c[6][5].g*CW8[6][5] +  c[6][6].g*CW8[6][6] +/*c[6][7].g*CW8[6][7]*/
                                          /*+ c[7][0].g*CW8[7][0]  + c[7][1].g*CW8[7][1]*/+ c[7][2].g*CW8[7][2] + c[7][3].g*CW8[7][3] + c[7][4].g*CW8[7][4] + c[7][5].g*CW8[7][5] +/*c[7][6].g*CW8[7][6] +  c[7][7].g*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
                                 col.b=Mid((/*c[0][0].b*CW8[0][0]  + c[0][1].b*CW8[0][1]*/+ c[0][2].b*CW8[0][2] + c[0][3].b*CW8[0][3] + c[0][4].b*CW8[0][4] + c[0][5].b*CW8[0][5] +/*c[0][6].b*CW8[0][6] +  c[0][7].b*CW8[0][7]*/
                                          /*+ c[1][0].b*CW8[1][0]*/+ c[1][1].b*CW8[1][1]  + c[1][2].b*CW8[1][2] + c[1][3].b*CW8[1][3] + c[1][4].b*CW8[1][4] + c[1][5].b*CW8[1][5] +  c[1][6].b*CW8[1][6] +/*c[1][7].b*CW8[1][7]*/
                                            + c[2][0].b*CW8[2][0]  + c[2][1].b*CW8[2][1]  + c[2][2].b*CW8[2][2] + c[2][3].b*CW8[2][3] + c[2][4].b*CW8[2][4] + c[2][5].b*CW8[2][5] +  c[2][6].b*CW8[2][6] +  c[2][7].b*CW8[2][7]
                                            + c[3][0].b*CW8[3][0]  + c[3][1].b*CW8[3][1]  + c[3][2].b*CW8[3][2] + c[3][3].b*CW8[3][3] + c[3][4].b*CW8[3][4] + c[3][5].b*CW8[3][5] +  c[3][6].b*CW8[3][6] +  c[3][7].b*CW8[3][7]
                                            + c[4][0].b*CW8[4][0]  + c[4][1].b*CW8[4][1]  + c[4][2].b*CW8[4][2] + c[4][3].b*CW8[4][3] + c[4][4].b*CW8[4][4] + c[4][5].b*CW8[4][5] +  c[4][6].b*CW8[4][6] +  c[4][7].b*CW8[4][7]
                                            + c[5][0].b*CW8[5][0]  + c[5][1].b*CW8[5][1]  + c[5][2].b*CW8[5][2] + c[5][3].b*CW8[5][3] + c[5][4].b*CW8[5][4] + c[5][5].b*CW8[5][5] +  c[5][6].b*CW8[5][6] +  c[5][7].b*CW8[5][7]
                                          /*+ c[6][0].b*CW8[6][0]*/+ c[6][1].b*CW8[6][1]  + c[6][2].b*CW8[6][2] + c[6][3].b*CW8[6][3] + c[6][4].b*CW8[6][4] + c[6][5].b*CW8[6][5] +  c[6][6].b*CW8[6][6] +/*c[6][7].b*CW8[6][7]*/
                                          /*+ c[7][0].b*CW8[7][0]  + c[7][1].b*CW8[7][1]*/+ c[7][2].b*CW8[7][2] + c[7][3].b*CW8[7][3] + c[7][4].b*CW8[7][4] + c[7][5].b*CW8[7][5] +/*c[7][6].b*CW8[7][6] +  c[7][7].b*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
                              }else
                              {
                                 Int w[8][8]={{/*CWA8[0][0]*c[0][0].a*/0,/*CWA8[0][1]*c[0][1].a*/0, CWA8[0][2]*c[0][2].a, CWA8[0][3]*c[0][3].a, CWA8[0][4]*c[0][4].a, CWA8[0][5]*c[0][5].a,/*CWA8[0][6]*c[0][6].a*/0,/*CWA8[0][7]*c[0][7].a*/0},
                                              {/*CWA8[1][0]*c[1][0].a*/0,  CWA8[1][1]*c[1][1].a   , CWA8[1][2]*c[1][2].a, CWA8[1][3]*c[1][3].a, CWA8[1][4]*c[1][4].a, CWA8[1][5]*c[1][5].a,  CWA8[1][6]*c[1][6].a   ,/*CWA8[1][7]*c[1][7].a*/0},
                                              {  CWA8[2][0]*c[2][0].a   ,  CWA8[2][1]*c[2][1].a   , CWA8[2][2]*c[2][2].a, CWA8[2][3]*c[2][3].a, CWA8[2][4]*c[2][4].a, CWA8[2][5]*c[2][5].a,  CWA8[2][6]*c[2][6].a   ,  CWA8[2][7]*c[2][7].a   },
                                              {  CWA8[3][0]*c[3][0].a   ,  CWA8[3][1]*c[3][1].a   , CWA8[3][2]*c[3][2].a, CWA8[3][3]*c[3][3].a, CWA8[3][4]*c[3][4].a, CWA8[3][5]*c[3][5].a,  CWA8[3][6]*c[3][6].a   ,  CWA8[3][7]*c[3][7].a   },
                                              {  CWA8[4][0]*c[4][0].a   ,  CWA8[4][1]*c[4][1].a   , CWA8[4][2]*c[4][2].a, CWA8[4][3]*c[4][3].a, CWA8[4][4]*c[4][4].a, CWA8[4][5]*c[4][5].a,  CWA8[4][6]*c[4][6].a   ,  CWA8[4][7]*c[4][7].a   },
                                              {  CWA8[5][0]*c[5][0].a   ,  CWA8[5][1]*c[5][1].a   , CWA8[5][2]*c[5][2].a, CWA8[5][3]*c[5][3].a, CWA8[5][4]*c[5][4].a, CWA8[5][5]*c[5][5].a,  CWA8[5][6]*c[5][6].a   ,  CWA8[5][7]*c[5][7].a   },
                                              {/*CWA8[6][0]*c[6][0].a*/0,  CWA8[6][1]*c[6][1].a   , CWA8[6][2]*c[6][2].a, CWA8[6][3]*c[6][3].a, CWA8[6][4]*c[6][4].a, CWA8[6][5]*c[6][5].a,  CWA8[6][6]*c[6][6].a   ,/*CWA8[6][7]*c[6][7].a*/0},
                                              {/*CWA8[7][0]*c[7][0].a*/0,/*CWA8[7][1]*c[7][1].a*/0, CWA8[7][2]*c[7][2].a, CWA8[7][3]*c[7][3].a, CWA8[7][4]*c[7][4].a, CWA8[7][5]*c[7][5].a,/*CWA8[7][6]*c[7][6].a*/0,/*CWA8[7][7]*c[7][7].a*/0}};
                                 Int div=/*w[0][0]  + w[0][1]*/+ w[0][2] + w[0][3] + w[0][4] + w[0][5]/*+ w[0][6]  + w[0][7]*/
                                       /*+ w[1][0]*/+ w[1][1]  + w[1][2] + w[1][3] + w[1][4] + w[1][5]  + w[1][6]/*+ w[1][7]*/
                                         + w[2][0]  + w[2][1]  + w[2][2] + w[2][3] + w[2][4] + w[2][5]  + w[2][6]  + w[2][7]
                                         + w[3][0]  + w[3][1]  + w[3][2] + w[3][3] + w[3][4] + w[3][5]  + w[3][6]  + w[3][7]
                                         + w[4][0]  + w[4][1]  + w[4][2] + w[4][3] + w[4][4] + w[4][5]  + w[4][6]  + w[4][7]
                                         + w[5][0]  + w[5][1]  + w[5][2] + w[5][3] + w[5][4] + w[5][5]  + w[5][6]  + w[5][7]
                                       /*+ w[6][0]*/+ w[6][1]  + w[6][2] + w[6][3] + w[6][4] + w[6][5]  + w[6][6]/*+ w[6][7]*/
                                       /*+ w[7][0]  + w[7][1]*/+ w[7][2] + w[7][3] + w[7][4] + w[7][5]/*+ w[7][6]  + w[7][7]*/;
                                 if(div<=0){col.a=0; goto cubic_sharp_rgb;}
                                 col.a=Min(DivRound(div, CWA8Sum), 255); // here "div>0" so no need to do "Max(0, "
                                 if(div<CWA8AlphaLimit) // below this limit, lerp to RGB
                                 {
                                    // instead of lerping actual colors, we lerp just the weights, it is an approximation and does not provide the same results as float version, however it is faster
                                    // weights are lerped between "CWA8[y][x]" (alpha_weight=false) and "CWA8[y][x]*c[y][x].a" (alpha_weight=true)
                                    // since the right side has a scale of "c[y][x].a", we multiply the left side by "Max(col.a, 1)" (average alpha value, and max 1 to avoid having zero weights and division by zero later)
                                 #if 0 // float version
                                  C Flt blend=Flt(div)/CWA8AlphaLimit;
                                    REPD(y, 8)
                                    REPD(x, 8)w[y][x]=Lerp(CWA8[y][x]*Max(col.a, 1), w[y][x], blend);
                                 #else // integer version
                                  C Int d=256, blend=div/d, blend1=(CWA8AlphaLimit/d-blend)*Max(col.a, 1);
                                    REPD(y, 8)
                                    REPD(x, 8)w[y][x]=(CWA8[y][x]*blend1 + w[y][x]*blend)>>10;
                                 #endif

                                    // recalculate 'div'
                                    div=/*w[0][0]  + w[0][1]*/+ w[0][2] + w[0][3] + w[0][4] + w[0][5]/*+ w[0][6]  + w[0][7]*/
                                      /*+ w[1][0]*/+ w[1][1]  + w[1][2] + w[1][3] + w[1][4] + w[1][5]  + w[1][6]/*+ w[1][7]*/
                                        + w[2][0]  + w[2][1]  + w[2][2] + w[2][3] + w[2][4] + w[2][5]  + w[2][6]  + w[2][7]
                                        + w[3][0]  + w[3][1]  + w[3][2] + w[3][3] + w[3][4] + w[3][5]  + w[3][6]  + w[3][7]
                                        + w[4][0]  + w[4][1]  + w[4][2] + w[4][3] + w[4][4] + w[4][5]  + w[4][6]  + w[4][7]
                                        + w[5][0]  + w[5][1]  + w[5][2] + w[5][3] + w[5][4] + w[5][5]  + w[5][6]  + w[5][7]
                                      /*+ w[6][0]*/+ w[6][1]  + w[6][2] + w[6][3] + w[6][4] + w[6][5]  + w[6][6]/*+ w[6][7]*/
                                      /*+ w[7][0]  + w[7][1]*/+ w[7][2] + w[7][3] + w[7][4] + w[7][5]/*+ w[7][6]  + w[7][7]*/;
                                 }
                                 Int div_2=div>>1;
                                 col.r=Mid((/*c[0][0].r*w[0][0]  + c[0][1].r*w[0][1]*/+ c[0][2].r*w[0][2] + c[0][3].r*w[0][3] + c[0][4].r*w[0][4] + c[0][5].r*w[0][5] +/*c[0][6].r*w[0][6] +  c[0][7].r*w[0][7]*/
                                          /*+ c[1][0].r*w[1][0]*/+ c[1][1].r*w[1][1]  + c[1][2].r*w[1][2] + c[1][3].r*w[1][3] + c[1][4].r*w[1][4] + c[1][5].r*w[1][5] +  c[1][6].r*w[1][6] +/*c[1][7].r*w[1][7]*/
                                            + c[2][0].r*w[2][0]  + c[2][1].r*w[2][1]  + c[2][2].r*w[2][2] + c[2][3].r*w[2][3] + c[2][4].r*w[2][4] + c[2][5].r*w[2][5] +  c[2][6].r*w[2][6] +  c[2][7].r*w[2][7]
                                            + c[3][0].r*w[3][0]  + c[3][1].r*w[3][1]  + c[3][2].r*w[3][2] + c[3][3].r*w[3][3] + c[3][4].r*w[3][4] + c[3][5].r*w[3][5] +  c[3][6].r*w[3][6] +  c[3][7].r*w[3][7]
                                            + c[4][0].r*w[4][0]  + c[4][1].r*w[4][1]  + c[4][2].r*w[4][2] + c[4][3].r*w[4][3] + c[4][4].r*w[4][4] + c[4][5].r*w[4][5] +  c[4][6].r*w[4][6] +  c[4][7].r*w[4][7]
                                            + c[5][0].r*w[5][0]  + c[5][1].r*w[5][1]  + c[5][2].r*w[5][2] + c[5][3].r*w[5][3] + c[5][4].r*w[5][4] + c[5][5].r*w[5][5] +  c[5][6].r*w[5][6] +  c[5][7].r*w[5][7]
                                          /*+ c[6][0].r*w[6][0]*/+ c[6][1].r*w[6][1]  + c[6][2].r*w[6][2] + c[6][3].r*w[6][3] + c[6][4].r*w[6][4] + c[6][5].r*w[6][5] +  c[6][6].r*w[6][6] +/*c[6][7].r*w[6][7]*/
                                          /*+ c[7][0].r*w[7][0]  + c[7][1].r*w[7][1]*/+ c[7][2].r*w[7][2] + c[7][3].r*w[7][3] + c[7][4].r*w[7][4] + c[7][5].r*w[7][5] +/*c[7][6].r*w[7][6] +  c[7][7].r*w[7][7]*/ + div_2)/div, 0, 255);
                                 col.g=Mid((/*c[0][0].g*w[0][0]  + c[0][1].g*w[0][1]*/+ c[0][2].g*w[0][2] + c[0][3].g*w[0][3] + c[0][4].g*w[0][4] + c[0][5].g*w[0][5] +/*c[0][6].g*w[0][6] +  c[0][7].g*w[0][7]*/
                                          /*+ c[1][0].g*w[1][0]*/+ c[1][1].g*w[1][1]  + c[1][2].g*w[1][2] + c[1][3].g*w[1][3] + c[1][4].g*w[1][4] + c[1][5].g*w[1][5] +  c[1][6].g*w[1][6] +/*c[1][7].g*w[1][7]*/
                                            + c[2][0].g*w[2][0]  + c[2][1].g*w[2][1]  + c[2][2].g*w[2][2] + c[2][3].g*w[2][3] + c[2][4].g*w[2][4] + c[2][5].g*w[2][5] +  c[2][6].g*w[2][6] +  c[2][7].g*w[2][7]
                                            + c[3][0].g*w[3][0]  + c[3][1].g*w[3][1]  + c[3][2].g*w[3][2] + c[3][3].g*w[3][3] + c[3][4].g*w[3][4] + c[3][5].g*w[3][5] +  c[3][6].g*w[3][6] +  c[3][7].g*w[3][7]
                                            + c[4][0].g*w[4][0]  + c[4][1].g*w[4][1]  + c[4][2].g*w[4][2] + c[4][3].g*w[4][3] + c[4][4].g*w[4][4] + c[4][5].g*w[4][5] +  c[4][6].g*w[4][6] +  c[4][7].g*w[4][7]
                                            + c[5][0].g*w[5][0]  + c[5][1].g*w[5][1]  + c[5][2].g*w[5][2] + c[5][3].g*w[5][3] + c[5][4].g*w[5][4] + c[5][5].g*w[5][5] +  c[5][6].g*w[5][6] +  c[5][7].g*w[5][7]
                                          /*+ c[6][0].g*w[6][0]*/+ c[6][1].g*w[6][1]  + c[6][2].g*w[6][2] + c[6][3].g*w[6][3] + c[6][4].g*w[6][4] + c[6][5].g*w[6][5] +  c[6][6].g*w[6][6] +/*c[6][7].g*w[6][7]*/
                                          /*+ c[7][0].g*w[7][0]  + c[7][1].g*w[7][1]*/+ c[7][2].g*w[7][2] + c[7][3].g*w[7][3] + c[7][4].g*w[7][4] + c[7][5].g*w[7][5] +/*c[7][6].g*w[7][6] +  c[7][7].g*w[7][7]*/ + div_2)/div, 0, 255);
                                 col.b=Mid((/*c[0][0].b*w[0][0]  + c[0][1].b*w[0][1]*/+ c[0][2].b*w[0][2] + c[0][3].b*w[0][3] + c[0][4].b*w[0][4] + c[0][5].b*w[0][5] +/*c[0][6].b*w[0][6] +  c[0][7].b*w[0][7]*/
                                          /*+ c[1][0].b*w[1][0]*/+ c[1][1].b*w[1][1]  + c[1][2].b*w[1][2] + c[1][3].b*w[1][3] + c[1][4].b*w[1][4] + c[1][5].b*w[1][5] +  c[1][6].b*w[1][6] +/*c[1][7].b*w[1][7]*/
                                            + c[2][0].b*w[2][0]  + c[2][1].b*w[2][1]  + c[2][2].b*w[2][2] + c[2][3].b*w[2][3] + c[2][4].b*w[2][4] + c[2][5].b*w[2][5] +  c[2][6].b*w[2][6] +  c[2][7].b*w[2][7]
                                            + c[3][0].b*w[3][0]  + c[3][1].b*w[3][1]  + c[3][2].b*w[3][2] + c[3][3].b*w[3][3] + c[3][4].b*w[3][4] + c[3][5].b*w[3][5] +  c[3][6].b*w[3][6] +  c[3][7].b*w[3][7]
                                            + c[4][0].b*w[4][0]  + c[4][1].b*w[4][1]  + c[4][2].b*w[4][2] + c[4][3].b*w[4][3] + c[4][4].b*w[4][4] + c[4][5].b*w[4][5] +  c[4][6].b*w[4][6] +  c[4][7].b*w[4][7]
                                            + c[5][0].b*w[5][0]  + c[5][1].b*w[5][1]  + c[5][2].b*w[5][2] + c[5][3].b*w[5][3] + c[5][4].b*w[5][4] + c[5][5].b*w[5][5] +  c[5][6].b*w[5][6] +  c[5][7].b*w[5][7]
                                          /*+ c[6][0].b*w[6][0]*/+ c[6][1].b*w[6][1]  + c[6][2].b*w[6][2] + c[6][3].b*w[6][3] + c[6][4].b*w[6][4] + c[6][5].b*w[6][5] +  c[6][6].b*w[6][6] +/*c[6][7].b*w[6][7]*/
                                          /*+ c[7][0].b*w[7][0]  + c[7][1].b*w[7][1]*/+ c[7][2].b*w[7][2] + c[7][3].b*w[7][3] + c[7][4].b*w[7][4] + c[7][5].b*w[7][5] +/*c[7][6].b*w[7][6] +  c[7][7].b*w[7][7]*/ + div_2)/div, 0, 255);
                              }
                              dest.color(x, y, col);
                           }
                        }
                        goto finish;
                     } // if(!high_prec)
                  }break;

                  case FILTER_CUBIC_FAST_SMOOTH: // used by 'transparentToNeighbor'
                  {
                     REPD(y, dest.h())
                     {
                        Int yc[8]; yc[3]=y*2; // 'y[3]' is always OK
                        if(clamp){yc[0]=Max(yc[3]-3, 0  ); yc[1]=Max(yc[3]-2, 0  ); yc[2]=Max(yc[3]-1, 0  ); yc[4]=Min(yc[3]+1, h()-1); yc[5]=Min(yc[3]+2, h()-1); yc[6]=Min(yc[3]+3, h()-1); yc[7]=Min(yc[3]+4, h()-1);}
                        else     {yc[0]=Mod(yc[3]-3, h()); yc[1]=Mod(yc[3]-2, h()); yc[2]=Mod(yc[3]-1, h()); yc[4]=   (yc[3]+1)%h()   ; yc[5]=   (yc[3]+2)%h()   ; yc[6]=   (yc[3]+3)%h()   ; yc[7]=   (yc[3]+4)%h()   ;}
                        REPD(x, dest.w())
                        {
                           Int xc[8]; xc[3]=x*2; // 'x[3]' is always OK
                           if(clamp){xc[0]=Max(xc[3]-3, 0  ); xc[1]=Max(xc[3]-2, 0  ); xc[2]=Max(xc[3]-1, 0  ); xc[4]=Min(xc[3]+1, w()-1); xc[5]=Min(xc[3]+2, w()-1); xc[6]=Min(xc[3]+3, w()-1); xc[7]=Min(xc[3]+4, w()-1);}
                           else     {xc[0]=Mod(xc[3]-3, w()); xc[1]=Mod(xc[3]-2, w()); xc[2]=Mod(xc[3]-1, w()); xc[4]=   (xc[3]+1)%w()   ; xc[5]=   (xc[3]+2)%w()   ; xc[6]=   (xc[3]+3)%w()   ; xc[7]=   (xc[3]+4)%w()   ;}
                           Vec rgb=0; Vec4 color=0, c[8][8]; // [y][x]
                           gather(&c[0][0], xc, Elms(xc), yc, Elms(yc));
                           REPD(x, 8)
                           REPD(y, 8)if(Flt w=CFSMW8[y][x])Add(color, rgb, c[y][x], w, alpha_weight);
                           Normalize(color, rgb, alpha_weight, t_high_prec);
                           dest.colorF(x, y, color);
                        }
                     }
                  }goto finish;
               } // switch(filter)
            }else // 3D
            {
               switch(filter)
               {
                  case FILTER_NONE: REPD(z, dest.d())
                  {
                     Int zc=z*2; REPD(y, dest.h())
                     {
                        Int yc=y*2;
                        if(high_prec)REPD(x, dest.w())dest.color3DF(x, y, z, color3DF(x*2, yc, zc));
                        else         REPD(x, dest.w())dest.color3D (x, y, z, color3D (x*2, yc, zc));
                     }
                  }goto finish;

                  case FILTER_LINEAR:
                  {
                     if(!high_prec)
                     {
                        REPD(z, dest.d())
                        {
                           Int zc[2]; zc[0]=z*2; zc[1]=(clamp ? Min(zc[0]+1, d()-1) : (zc[0]+1)%d()); // zc[0] is always OK
                           REPD(y, dest.h())
                           {
                              Int yc[2]; yc[0]=y*2; yc[1]=(clamp ? Min(yc[0]+1, h()-1) : (yc[0]+1)%h()); // yc[0] is always OK
                              REPD(x, dest.w())
                              {
                                 Int xc[2]; xc[0]=x*2; xc[1]=(clamp ? Min(xc[0]+1, w()-1) : (xc[0]+1)%w()); // xc[0] is always OK
                                 Color col, c[2][2][2]; gather(&c[0][0][0], xc, Elms(xc), yc, Elms(yc), zc, Elms(zc)); // [z][y][x]
                                 if(!alpha_weight)
                                 {
                                    col.a=((c[0][0][0].a+c[0][0][1].a+c[0][1][0].a+c[0][1][1].a+c[1][0][0].a+c[1][0][1].a+c[1][1][0].a+c[1][1][1].a+4)>>3);
                                 linear_rgb_3D:
                                    col.r=((c[0][0][0].r+c[0][0][1].r+c[0][1][0].r+c[0][1][1].r+c[1][0][0].r+c[1][0][1].r+c[1][1][0].r+c[1][1][1].r+4)>>3);
                                    col.g=((c[0][0][0].g+c[0][0][1].g+c[0][1][0].g+c[0][1][1].g+c[1][0][0].g+c[1][0][1].g+c[1][1][0].g+c[1][1][1].g+4)>>3);
                                    col.b=((c[0][0][0].b+c[0][0][1].b+c[0][1][0].b+c[0][1][1].b+c[1][0][0].b+c[1][0][1].b+c[1][1][0].b+c[1][1][1].b+4)>>3);
                                 }else
                                 {
                                    UInt a=c[0][0][0].a+c[0][0][1].a+c[0][1][0].a+c[0][1][1].a+c[1][0][0].a+c[1][0][1].a+c[1][1][0].a+c[1][1][1].a;
                                    if( !a){col.a=0; goto linear_rgb_3D;}
                                    col.a=((a+4)>>3); UInt a_2=a>>1;
                                    col.r=(c[0][0][0].r*c[0][0][0].a + c[0][0][1].r*c[0][0][1].a + c[0][1][0].r*c[0][1][0].a + c[0][1][1].r*c[0][1][1].a + c[1][0][0].r*c[1][0][0].a + c[1][0][1].r*c[1][0][1].a + c[1][1][0].r*c[1][1][0].a + c[1][1][1].r*c[1][1][1].a + a_2)/a;
                                    col.g=(c[0][0][0].g*c[0][0][0].a + c[0][0][1].g*c[0][0][1].a + c[0][1][0].g*c[0][1][0].a + c[0][1][1].g*c[0][1][1].a + c[1][0][0].g*c[1][0][0].a + c[1][0][1].g*c[1][0][1].a + c[1][1][0].g*c[1][1][0].a + c[1][1][1].g*c[1][1][1].a + a_2)/a;
                                    col.b=(c[0][0][0].b*c[0][0][0].a + c[0][0][1].b*c[0][0][1].a + c[0][1][0].b*c[0][1][0].a + c[0][1][1].b*c[0][1][1].a + c[1][0][0].b*c[1][0][0].a + c[1][0][1].b*c[1][0][1].a + c[1][1][0].b*c[1][1][0].a + c[1][1][1].b*c[1][1][1].a + a_2)/a;
                                 }
                                 dest.color3D(x, y, z, col);
                              }
                           }
                        }
                        goto finish;
                     }
                  }break;
               }
            }
         }
         
         // any scale
         {
            // for scale 1->1 offset=0.0
            //           2->1 offset=0.5
            //           3->1 offset=1.0
            //           4->1 offset=1.5
            Vec2 x_mul_add, y_mul_add, z_mul_add;
            if(keep_edges)
            {
               x_mul_add.set(Flt(T.w()-1)/(dest.w()-1), 0);
               y_mul_add.set(Flt(T.h()-1)/(dest.h()-1), 0);
               z_mul_add.set(Flt(T.d()-1)/(dest.d()-1), 0);
            }else
            {
               x_mul_add.x=Flt(T.w())/dest.w(); x_mul_add.y=x_mul_add.x*0.5f-0.5f;
               y_mul_add.x=Flt(T.h())/dest.h(); y_mul_add.y=y_mul_add.x*0.5f-0.5f;
               z_mul_add.x=Flt(T.d())/dest.d(); z_mul_add.y=z_mul_add.x*0.5f-0.5f;
            }
            Vec size(x_mul_add.x, y_mul_add.x, z_mul_add.x); size*=sharp_smooth;
            if(filter!=FILTER_NONE && (size.x>1 || size.y>1) && T.d()==1 && dest.d()==1) // if we're downsampling (any scale is higher than 1) then we must use more complex 'areaColor*' methods
            {
               Vec4 (Image::*area_color)(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C; // pointer to class method
               switch(filter)
               {
                //case FILTER_AVERAGE          : area_color=&Image::areaColorAverage        ; break;
                  case FILTER_LINEAR           : area_color=&Image::areaColorLinear         ; break;
                  case FILTER_CUBIC_FAST       : area_color=&Image::areaColorCubicFast      ; break;
                  case FILTER_CUBIC_FAST_SMOOTH: area_color=&Image::areaColorCubicFastSmooth; break;
                  default                      : // FILTER_BEST
                  case FILTER_CUBIC_FAST_SHARP : area_color=&Image::areaColorCubicFastSharp ; break; ASSERT(FILTER_DOWN==FILTER_CUBIC_FAST_SHARP);
                  case FILTER_CUBIC            : area_color=&Image::areaColorCubic          ; break;
                  case FILTER_CUBIC_SHARP      : area_color=&Image::areaColorCubicSharp     ; break;
               }
               Vec2 pos;
               REPD(y, dest.h())
               {
                  pos.y=y*y_mul_add.x+y_mul_add.y;
                  REPD(x, dest.w())
                  {
                     pos.x=x*x_mul_add.x+x_mul_add.y;
                     dest.colorF(x, y, (T.*area_color)(pos, size.xy, clamp, alpha_weight));
                  }
               }
            }else
            if((filter==FILTER_CUBIC || filter==FILTER_CUBIC_SHARP || filter==FILTER_BEST) // optimized Cubic/Best upscale
            && T.d()==1)
            {
               Flt (*Func)(Flt x)=((filter==FILTER_CUBIC_SHARP) ? CubicSharp2 : CubicMed2); ASSERT(CUBIC_MED_SAMPLES==CUBIC_SHARP_SAMPLES && CUBIC_MED_RANGE==CUBIC_SHARP_RANGE && CUBIC_MED_SHARPNESS==CUBIC_SHARP_SHARPNESS);
               REPD(z, dest.d())
               {
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  FREPD(y, dest.h())
                  {
                     Byte *dest_data_y=dest_data_z + y*dest.pitch();
                     Flt   sy=y*y_mul_add.x+y_mul_add.y,
                           sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
                     Int   xo[CUBIC_MED_SAMPLES*2], yo[CUBIC_MED_SAMPLES*2], xi=Floor(sx), yi=Floor(sy);
                     Flt   yw[CUBIC_MED_SAMPLES*2];
                     REPA( xo)
                     {
                        xo[i]=xi-CUBIC_MED_SAMPLES+1+i;
                        yo[i]=yi-CUBIC_MED_SAMPLES+1+i; yw[i]=Sqr(sy-yo[i]);
                        if(clamp)
                        {
                           Clamp(xo[i], 0, lw()-1);
                           Clamp(yo[i], 0, lh()-1);
                        }else
                        {
                           xo[i]=Mod(xo[i], lw());
                           yo[i]=Mod(yo[i], lh());
                        }
                     }
                     if(NeedMultiChannel(T.type(), dest.type()))
                     {
                        Vec4 c[CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
                        gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, CUBIC_MED_SAMPLES*2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+CUBIC_MED_SAMPLES; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset=(x_offset+1)%(CUBIC_MED_SAMPLES*2);
                           }

                           Flt  weight=0;
                           Vec  rgb   =0;
                           Vec4 color =0;
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)%(CUBIC_MED_SAMPLES*2);
                              Flt xw=Sqr(sx-(xi-CUBIC_MED_SAMPLES+1+x));
                              REPAD(y, yo)
                              {
                                 Flt w=xw+yw[y]; if(w<Sqr(CUBIC_MED_RANGE))
                                 {
                                    w=Func(w*Sqr(CUBIC_MED_SHARPNESS)); Add(color, rgb, c[xc][y], w, alpha_weight); weight+=w;
                                 }
                              }
                           }
                           Normalize(color, rgb, weight, alpha_weight, t_high_prec);
                           StoreColor(dest, dest_data_y, x, y, z, color);
                        }
                     }else
                     {
                        Flt v[CUBIC_MED_SAMPLES*2][CUBIC_MED_SAMPLES*2];
                        gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, CUBIC_MED_SAMPLES*2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+CUBIC_MED_SAMPLES; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset=(x_offset+1)%(CUBIC_MED_SAMPLES*2);
                           }

                           Flt weight=0, value=0;
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)%(CUBIC_MED_SAMPLES*2);
                              Flt xw=Sqr(sx-(xi-CUBIC_MED_SAMPLES+1+x));
                              REPAD(y, yo)
                              {
                                 Flt w=xw+yw[y]; if(w<Sqr(CUBIC_MED_RANGE))
                                 {
                                    w=Func(w*Sqr(CUBIC_MED_SHARPNESS)); value+=v[xc][y]*w; weight+=w;
                                 }
                              }
                           }
                           StorePixel(dest, dest_data_y, x, y, z, value/weight);
                        }
                     }
                  }
               }
            }else
            if((filter==FILTER_CUBIC_FAST || filter==FILTER_CUBIC_FAST_SMOOTH || filter==FILTER_CUBIC_FAST_SHARP) // optimized CubicFast upscale
            && T.d()==1)
            {
               Flt (*Func)(Flt x)=((filter==FILTER_CUBIC_FAST) ? CubicFast2 : (filter==FILTER_CUBIC_FAST_SMOOTH) ? CubicFastSmooth2 : CubicFastSharp2);
               REPD(z, dest.d())
               {
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  FREPD(y, dest.h())
                  {
                     Byte *dest_data_y=dest_data_z + y*dest.pitch();
                     Flt   sy=y*y_mul_add.x+y_mul_add.y,
                           sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
                     Int   xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], xi=Floor(sx), yi=Floor(sy);
                     Flt   yw[CUBIC_FAST_SAMPLES*2];
                     REPA( xo)
                     {
                        xo[i]=xi-CUBIC_FAST_SAMPLES+1+i;
                        yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(sy-yo[i]);
                        if(clamp)
                        {
                           Clamp(xo[i], 0, lw()-1);
                           Clamp(yo[i], 0, lh()-1);
                        }else
                        {
                           xo[i]=Mod(xo[i], lw());
                           yo[i]=Mod(yo[i], lh());
                        }
                     }
                     if(NeedMultiChannel(T.type(), dest.type()))
                     {
                        Vec4 c[CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
                        gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, CUBIC_FAST_SAMPLES*2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+CUBIC_FAST_SAMPLES; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset=(x_offset+1)%(CUBIC_FAST_SAMPLES*2);
                           }

                           Flt  weight=0;
                           Vec  rgb   =0;
                           Vec4 color =0;
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)%(CUBIC_FAST_SAMPLES*2);
                              Flt xw=Sqr(sx-(xi-CUBIC_FAST_SAMPLES+1+x));
                              REPAD(y, yo)
                              {
                                 Flt w=xw+yw[y]; if(w<Sqr(CUBIC_FAST_RANGE))
                                 {
                                    w=Func(w); Add(color, rgb, c[xc][y], w, alpha_weight); weight+=w;
                                 }
                              }
                           }
                           Normalize(color, rgb, weight, alpha_weight, t_high_prec);
                           StoreColor(dest, dest_data_y, x, y, z, color);
                        }
                     }else
                     {
                        Flt v[CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
                        gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, CUBIC_FAST_SAMPLES*2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+CUBIC_FAST_SAMPLES; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset=(x_offset+1)%(CUBIC_FAST_SAMPLES*2);
                           }

                           Flt weight=0, value=0;
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)%(CUBIC_FAST_SAMPLES*2);
                              Flt xw=Sqr(sx-(xi-CUBIC_FAST_SAMPLES+1+x));
                              REPAD(y, yo)
                              {
                                 Flt w=xw+yw[y]; if(w<Sqr(CUBIC_FAST_RANGE))
                                 {
                                    w=Func(w); value+=v[xc][y]*w; weight+=w;
                                 }
                              }
                           }
                           StorePixel(dest, dest_data_y, x, y, z, value/weight);
                        }
                     }
                  }
               }
            }else
            if(filter==FILTER_LINEAR // optimized Linear upscale, this is used for Texture Sharpness calculation
            && T.d()==1)
            {
               REPD(z, dest.d())
               {
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  FREPD(y, dest.h())
                  {
                     Byte *dest_data_y=dest_data_z + y*dest.pitch();
                     Flt   sy=y*y_mul_add.x+y_mul_add.y,
                           sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
                     Int   xo[2], yo[2], xi=Floor(sx), yi=Floor(sy);
                     Flt   yw[2]; yw[1]=sy-yi; yw[0]=1-yw[1];
                     REPA( xo)
                     {
                        xo[i]=xi+i;
                        yo[i]=yi+i;
                        if(clamp)
                        {
                           Clamp(xo[i], 0, lw()-1);
                           Clamp(yo[i], 0, lh()-1);
                        }else
                        {
                           xo[i]=Mod(xo[i], lw());
                           yo[i]=Mod(yo[i], lh());
                        }
                     }
                     if(NeedMultiChannel(T.type(), dest.type()))
                     {
                        Vec4 c[2][2];
                        gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, 2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+1; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset^=1;
                           }

                           Vec  rgb  =0;
                           Vec4 color=0;
                           Flt  xw[2]; xw[1]=sx-xi; xw[0]=1-xw[1];
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)&1;
                              REPAD(y, yo)Add(color, rgb, c[xc][y], xw[x]*yw[y], alpha_weight);
                           }
                           Normalize(color, rgb, alpha_weight, t_high_prec);
                           StoreColor(dest, dest_data_y, x, y, z, color);
                        }
                     }else
                     {
                        Flt v[2][2];
                        gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
                        REPD(x, 2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

                        Int x_offset=0;
                        FREPD(x, dest.w())
                        {
                           Flt sx=x*x_mul_add.x+x_mul_add.y;
                           Int xi2=Floor(sx); if(xi!=xi2)
                           {
                              xi=xi2;
                              Int xo_last=xi+1; if(clamp)Clamp(xo_last, 0, lw()-1);else xo_last=Mod(xo_last, lw());
                              gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
                              x_offset^=1;
                           }

                           Flt value=0, xw[2]; xw[1]=sx-xi; xw[0]=1-xw[1];
                           REPAD(x, xo)
                           {
                              Int xc=(x+x_offset)&1;
                              REPAD(y, yo)value+=v[xc][y]*xw[x]*yw[y];
                           }
                           StorePixel(dest, dest_data_y, x, y, z, value);
                        }
                     }
                  }
               }
            }else
            if(NeedMultiChannel(T.type(), dest.type()))
            {
               REPD(z, dest.d())
               {
                  Flt sz=z*z_mul_add.x+z_mul_add.y;
                  REPD(y, dest.h())
                  {
                     Flt sy=y*y_mul_add.x+y_mul_add.y;
                     REPD(x, dest.w())
                     {
                        Flt  sx=x*x_mul_add.x+x_mul_add.y;
                        Vec4 color;
                        switch(filter)
                        {
                           case FILTER_NONE             : color=((T.d()<=1) ? T.colorF               (RoundPos(sx), RoundPos(sy)                     ) : T.color3DF               (RoundPos(sx), RoundPos(sy), RoundPos(sz)       )); break;
                           case FILTER_LINEAR           : color=((T.d()<=1) ? T.colorFLinear         (         sx ,          sy , clamp, alpha_weight) : T.color3DFLinear         (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST       : color=((T.d()<=1) ? T.colorFCubicFast      (         sx ,          sy , clamp, alpha_weight) : T.color3DFCubicFast      (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST_SMOOTH: color=((T.d()<=1) ? T.colorFCubicFastSmooth(         sx ,          sy , clamp, alpha_weight) : T.color3DFCubicFastSmooth(         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST_SHARP : color=((T.d()<=1) ? T.colorFCubicFastSharp (         sx ,          sy , clamp, alpha_weight) : T.color3DFCubicFastSharp (         sx ,          sy ,          sz , clamp)); break;
                           default                      : // FILTER_BEST
                           case FILTER_CUBIC            : color=((T.d()<=1) ? T.colorFCubic          (         sx ,          sy , clamp, alpha_weight) : T.color3DFCubic          (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_SHARP      : color=((T.d()<=1) ? T.colorFCubicSharp     (         sx ,          sy , clamp, alpha_weight) : T.color3DFCubicSharp     (         sx ,          sy ,          sz , clamp)); break;
                        }
                        dest.color3DF(x, y, z, color);
                     }
                  }
               }
            }else
            {
               REPD(z, dest.d())
               {
                  Flt sz=z*z_mul_add.x+z_mul_add.y;
                  REPD(y, dest.h())
                  {
                     Flt sy=y*y_mul_add.x+y_mul_add.y;
                     REPD(x, dest.w())
                     {
                        Flt sx=x*x_mul_add.x+x_mul_add.y,
                            pix;
                        switch(filter)
                        {
                           case FILTER_NONE             : pix=((T.d()<=1) ? T.pixelF               (RoundPos(sx), RoundPos(sy)       ) : T.pixel3DF               (RoundPos(sx), RoundPos(sy), RoundPos(sz)       )); break;
                           case FILTER_LINEAR           : pix=((T.d()<=1) ? T.pixelFLinear         (         sx ,          sy , clamp) : T.pixel3DFLinear         (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST       : pix=((T.d()<=1) ? T.pixelFCubicFast      (         sx ,          sy , clamp) : T.pixel3DFCubicFast      (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST_SMOOTH: pix=((T.d()<=1) ? T.pixelFCubicFastSmooth(         sx ,          sy , clamp) : T.pixel3DFCubicFastSmooth(         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_FAST_SHARP : pix=((T.d()<=1) ? T.pixelFCubicFastSharp (         sx ,          sy , clamp) : T.pixel3DFCubicFastSharp (         sx ,          sy ,          sz , clamp)); break;
                           default                      : // FILTER_BEST
                           case FILTER_CUBIC            : pix=((T.d()<=1) ? T.pixelFCubic          (         sx ,          sy , clamp) : T.pixel3DFCubic          (         sx ,          sy ,          sz , clamp)); break;
                           case FILTER_CUBIC_SHARP      : pix=((T.d()<=1) ? T.pixelFCubicSharp     (         sx ,          sy , clamp) : T.pixel3DFCubicSharp     (         sx ,          sy ,          sz , clamp)); break;
                        }
                        dest.pixel3DF(x, y, z, pix);
                     }
                  }
               }
            }
         }
      }

   finish:
      dest.unlock();
         T.unlock();
   }
   return true;
}
/******************************************************************************/
}
/******************************************************************************/
