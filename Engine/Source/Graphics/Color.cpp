/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static Color ColorArray[]=
{
   RED   , GREEN, BLUE  ,
   YELLOW, CYAN , PURPLE,
   ORANGE, TURQ , BROWN ,
};
/******************************************************************************/
Color::Color(C Vec &color)
{
   r=FltToByte(color.x);
   g=FltToByte(color.y);
   b=FltToByte(color.z);
   a=255;
}
Color::Color(C Vec4 &color)
{
   r=FltToByte(color.x);
   g=FltToByte(color.y);
   b=FltToByte(color.z);
   a=FltToByte(color.w);
}
Color::Color(C VecB &color)
{
   r=color.x;
   g=color.y;
   b=color.z;
   a=255;
}
Color::Color(C VecB4 &color)
{
#if 0
   r=color.x;
   g=color.y;
   b=color.z;
   a=color.w;
#else
   u=color.u;
#endif
}
Vec Color::asVec()C
{
   return Vec(r/255.0f,
              g/255.0f,
              b/255.0f);
}
Vec4 Color::asVec4()C
{
   return Vec4(r/255.0f,
               g/255.0f,
               b/255.0f,
               a/255.0f);
}
StrO Color::asText()C {StrO s; s.reserve(18); s+=r; s+=", "; s+=g; s+=", "; s+=b; s+=", "; s+=a; return s;} // 18 because of 4x"255" + 3x", " = 4*3 + 3*2 = 12 + 6 = 18
StrO Color::asHex ()C {ASSERT(SIZE(T)==4); return TextHexMem(this, SIZE(T), false);}

Bool Color::fromHex(C Str &t)
{
   ASSERT(SIZE(T)==4); return TextHexMem(t, this, SIZE(T));
}
/******************************************************************************/
Color ColorI(Int i) {return ColorArray[Mod(i, Elms(ColorArray))];}
/******************************************************************************/
Color ColorInverse(C Color &color)
{
   return Color(255-color.r, 255-color.g, 255-color.b, color.a);
}
Color ColorBrightness(Flt brightness)
{
   Byte b=FltToByte(brightness);
   return Color(b, b, b);
}
Color ColorBrightness(C Color &color, Flt brightness)
{
   return Color(Mid(RoundPos(color.r*brightness), 0, 255),
                Mid(RoundPos(color.g*brightness), 0, 255),
                Mid(RoundPos(color.b*brightness), 0, 255), color.a);
}
Color ColorBrightnessB(C Color &color, Byte brightness)
{
   return Color((color.r*brightness+128)/255,
                (color.g*brightness+128)/255,
                (color.b*brightness+128)/255, color.a);
}
Color ColorAlpha(Flt alpha)
{
   return Color(255, 255, 255, FltToByte(alpha));
}
Color ColorAlpha(C Color &color, Flt alpha)
{
   return Color(color.r, color.g, color.b, Mid(RoundPos(color.a*alpha), 0, 255));
}
Color ColorBA(Flt brightness, Flt alpha)
{
                  Byte b=FltToByte(brightness);
   return Color(b, b, b, FltToByte(alpha     ));
}
Color ColorBA(C Color &color, Flt brightness, Flt alpha)
{
   return Color(Mid(RoundPos(color.r*brightness), 0, 255),
                Mid(RoundPos(color.g*brightness), 0, 255),
                Mid(RoundPos(color.b*brightness), 0, 255),
                Mid(RoundPos(color.a*alpha     ), 0, 255));
}
Color ColorMul(Flt mul)
{
   Byte x=FltToByte(mul);
   return Color(x, x, x, x);
}
Color ColorMul(C Color &color, Flt mul)
{
   return Color(Mid(RoundPos(color.r*mul), 0, 255),
                Mid(RoundPos(color.g*mul), 0, 255),
                Mid(RoundPos(color.b*mul), 0, 255),
                Mid(RoundPos(color.a*mul), 0, 255));
}
Color ColorMul(C Color &c0, C Color &c1)
{
   return Color((c0.r*c1.r+128)/255,
                (c0.g*c1.g+128)/255,
                (c0.b*c1.b+128)/255,
                (c0.a*c1.a+128)/255);
}
Color ColorMul(C Color &c0, C Color &c1, C Color &c2)
{
   const UInt div=255*255, div_2=div/2;
   return Color((c0.r*c1.r*c2.r+div_2)/div,
                (c0.g*c1.g*c2.g+div_2)/div,
                (c0.b*c1.b*c2.b+div_2)/div,
                (c0.a*c1.a*c2.a+div_2)/div);
}
Color ColorAdd(C Color &c0, C Color &c1)
{
   return Color(Mid(c0.r+c1.r, 0, 255),
                Mid(c0.g+c1.g, 0, 255),
                Mid(c0.b+c1.b, 0, 255),
                Mid(c0.a+c1.a, 0, 255));
}
Color ColorAdd(C Color &c0, C Color &c1, C Color &c2)
{
   return Color(Mid(c0.r+c1.r+c2.r, 0, 255),
                Mid(c0.g+c1.g+c2.g, 0, 255),
                Mid(c0.b+c1.b+c2.b, 0, 255),
                Mid(c0.a+c1.a+c2.a, 0, 255));
}
Color ColorMulZeroAlpha(C Color &color, Flt mul)
{
   return Color(Mid(RoundPos(color.r*mul), 0, 255),
                Mid(RoundPos(color.g*mul), 0, 255),
                Mid(RoundPos(color.b*mul), 0, 255), 0);
}
Color ColorMulZeroAlpha(C Color &c0, C Color &c1)
{
   return Color((c0.r*c1.r+128)/255,
                (c0.g*c1.g+128)/255,
                (c0.b*c1.b+128)/255, 0);
}
/******************************************************************************/
Color Avg(C Color &c0, C Color &c1)
{
   return Color((c0.r+c1.r+1)>>1,
                (c0.g+c1.g+1)>>1,
                (c0.b+c1.b+1)>>1,
                (c0.a+c1.a+1)>>1);
}
Color Avg(C Color &c0, C Color &c1, C Color &c2)
{
   return Color((c0.r+c1.r+c2.r+1)/3,
                (c0.g+c1.g+c2.g+1)/3,
                (c0.b+c1.b+c2.b+1)/3,
                (c0.a+c1.a+c2.a+1)/3);
}
Color Avg(C Color &c0, C Color &c1, C Color &c2, C Color &c3)
{
   return Color((c0.r+c1.r+c2.r+c3.r+2)>>2,
                (c0.g+c1.g+c2.g+c3.g+2)>>2,
                (c0.b+c1.b+c2.b+c3.b+2)>>2,
                (c0.a+c1.a+c2.a+c3.a+2)>>2);
}
/******************************************************************************/
Color Lerp(C Color &c0, C Color &c1, Flt step)
{
   Flt step1=1-step;
   return Color(Mid(RoundPos(c0.r*step1 + c1.r*step), 0, 255),
                Mid(RoundPos(c0.g*step1 + c1.g*step), 0, 255),
                Mid(RoundPos(c0.b*step1 + c1.b*step), 0, 255),
                Mid(RoundPos(c0.a*step1 + c1.a*step), 0, 255));
}
Color Lerp(C Color &c0, C Color &c1, C Color &c2, C Vec &blend)
{
   return Color(Mid(RoundPos(c0.r*blend.x + c1.r*blend.y + c2.r*blend.z), 0, 255),
                Mid(RoundPos(c0.g*blend.x + c1.g*blend.y + c2.g*blend.z), 0, 255),
                Mid(RoundPos(c0.b*blend.x + c1.b*blend.y + c2.b*blend.z), 0, 255),
                Mid(RoundPos(c0.a*blend.x + c1.a*blend.y + c2.a*blend.z), 0, 255));
}
/******************************************************************************/
Color ColorHue(Flt hue)
{
   hue=6*Frac(hue);
   Byte x=RoundU(Frac(hue)*255);
   if(hue<1)return Color(255  ,     x,     0);
   if(hue<2)return Color(255-x, 255  ,     0);
   if(hue<3)return Color(    0, 255  ,     x);
   if(hue<4)return Color(    0, 255-x, 255  );
   if(hue<5)return Color(    x,     0, 255  );
            return Color(  255,     0, 255-x);
}
Color ColorHue(C Color &color, Flt hue)
{
   Vec  hsb=RgbToHsb(color.asVec());
   Color  c=ColorHSB(hsb.x+hue, hsb.y, hsb.z); c.a=color.a;
   return c;
}
Vec ColorHue(C Vec &color, Flt hue)
{
   Vec hsb=RgbToHsb(color); hsb.x+=hue; return HsbToRgb(hsb);
}
Color ColorHSB(Flt h, Flt s, Flt b)
{
   SAT(s);
   SAT(b)*=255;
        h=6*Frac(h);
   Flt  f=  Frac(h);
   Byte v=RoundU(b),
        p=RoundU(b*(1-s)),
        q=RoundU(b*(1-s*f)),
        t=RoundU(b*(1-s*(1-f)));
   if(h<1)return Color(v, t, p);
   if(h<2)return Color(q, v, p);
   if(h<3)return Color(p, v, t);
   if(h<4)return Color(p, q, v);
   if(h<5)return Color(t, p, v);
          return Color(v, p, q);
}
Vec RgbToHsb(C Vec &rgb)
{
   Flt max=rgb.max(),
       min=rgb.min(),
       d  =max-min;

   Vec O;
   if(d    <=  0)O.x=0;else
   if(rgb.x>=max)
   {
      if(rgb.y>=rgb.z)O.x=(rgb.y-rgb.z)/d;
      else            O.x=(rgb.y-rgb.z)/d+6;
   }else
   if(rgb.y>=max)O.x=(rgb.z-rgb.x)/d+2;
   else          O.x=(rgb.x-rgb.y)/d+4;

   O.x/=6;
   O.y =(max ? 1-min/max : 1);
   O.z = max;

   return O;
}
Vec HsbToRgb(C Vec &hsb)
{
   Flt h=Frac(hsb.x)*6,
       s=Sat (hsb.y),
       f=Frac(h),
       v=hsb.z,
       p=hsb.z*(1-s      ),
       q=hsb.z*(1-s*(  f)),
       t=hsb.z*(1-s*(1-f));
   if(h<1)return Vec(v, t, p);
   if(h<2)return Vec(q, v, p);
   if(h<3)return Vec(p, v, t);
   if(h<4)return Vec(p, q, v);
   if(h<5)return Vec(t, p, v);
          return Vec(v, p, q);
}
/******************************************************************************/
Vec RgbToYuv(C Vec &rgb)
{
   return Vec( 66/256.0f*rgb.x + 129/256.0f*rgb.y +  25/256.0f*rgb.z +  16/256.0f,
              -38/256.0f*rgb.x -  74/256.0f*rgb.y + 112/256.0f*rgb.z + 128/256.0f,
              112/256.0f*rgb.x -  94/256.0f*rgb.y -  18/256.0f*rgb.z + 128/256.0f);
}
Vec YuvToRgb(C Vec &yuv)
{
   Vec YUV(yuv.x -  16/256.0f,
           yuv.y - 128/256.0f,
           yuv.z - 128/256.0f);
   return Vec(298/256.0f * YUV.x                      + 409/256.0f * YUV.z,
              298/256.0f * YUV.x - 100/256.0f * YUV.y - 208/256.0f * YUV.z,
              298/256.0f * YUV.x + 516/256.0f * YUV.y                     );
}
/******************************************************************************/
Int ColorDiffSum(C Color &x, C Color &y)
{
   return Abs(x.r - y.r)
         +Abs(x.g - y.g)
         +Abs(x.b - y.b)
         +Abs(x.a - y.a);
}
Int ColorDiffMax(C Color &x, C Color &y)
{
   return Max(Abs(x.r - y.r),
              Abs(x.g - y.g),
              Abs(x.b - y.b),
              Abs(x.a - y.a));
}
Flt ColorDiffMax(C Vec &x, C Vec &y)
{
   return Max(Abs(x.x - y.x),
              Abs(x.y - y.y),
              Abs(x.z - y.z));
}
/******************************************************************************/
Color Blend(C Color &base, C Color &color)
{
   Color out;
#if 1 // faster Int version
   UInt  base_w=(255-color.a)*base.a, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
        color_w=     color.a *255   ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want, mul by 255 to match the scale of 'base_w'
   if(UInt sum=base_w+color_w)
   {
       base_w=(base_w*0xFFFF + sum/2)/sum;
      color_w=0xFFFF-base_w;
      out.r=(base.r*base_w + color.r*color_w + 0x8000)>>16;
      out.g=(base.g*base_w + color.g*color_w + 0x8000)>>16;
      out.b=(base.b*base_w + color.b*color_w + 0x8000)>>16;
   }else out.r=out.g=out.b=0;
#else // slower Flt version
   Flt  base_w=((255-color.a)*base.a)/255.0f, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5), divide by 255.0f to match the scale of 'color_w' which is 0..255, it will be normalized with 'sum' below
       color_w=      color.a                ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.r=RoundU(base.r*base_w + color.r*color_w);
      out.g=RoundU(base.g*base_w + color.g*color_w);
      out.b=RoundU(base.b*base_w + color.b*color_w);
   }else out.r=out.g=out.b=0;
#endif
   out.a=base.a+(color.a*(255-base.a)+128)/255;
   return out;
}
Vec4 FastBlend(C Vec4 &base, C Vec4 &color)
{
   return Vec4(base.xyz*(1-color.w) + color.xyz*  color.w  ,
               base.w               + color.w  *(1-base.w));
}
Vec4 Blend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   Flt  base_w=(1-color.w)*base.w, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
       color_w=   color.w        ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.xyz=base.xyz*base_w + color.xyz*color_w;
   }else out.xyz.zero();
   out.w=base.w+color.w*(1-base.w);
   return out;
}
Vec4 FastPremultipliedBlend(C Vec4 &base, C Vec4 &color)
{
   return Vec4(base.xyz*(1-color.w) + color.xyz          ,
               base.w               + color.w*(1-base.w));
}
Vec4 PremultipliedBlend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   Flt  base_w=(1-color.w)*base.w, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
       color_w=   color.w        ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.xyz=base.xyz*base_w + color.xyz;
   }else out.xyz.zero();
   out.w=base.w+color.w*(1-base.w);
   return out;
}
Vec4 AdditiveBlend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   out.xyz=base.xyz*base .w + color.xyz*color.w;
   out.w  =base.w  +color.w*(1-base.w);
   if(out.w)out.xyz/=out.w;
   return out;
}
/******************************************************************************/
Flt SRGBToLinear(Flt s) {return (s<=0.04045f  ) ? s/12.92f : Pow((s+0.055f)/1.055f, 2.4f);} // convert 0..1 srgb   to 0..1 linear
Flt LinearToSRGB(Flt l) {return (l<=0.0031308f) ? l*12.92f : Pow(l, 1/2.4f)*1.055f-0.055f;} // convert 0..1 linear to 0..1 srgb

Vec SRGBToLinear(C Vec &s) {return Vec(SRGBToLinear(s.x), SRGBToLinear(s.y), SRGBToLinear(s.z));}
Vec LinearToSRGB(C Vec &l) {return Vec(LinearToSRGB(l.x), LinearToSRGB(l.y), LinearToSRGB(l.z));}

Flt LinearLumOfLinearColor(C Vec &l) {return              Dot(      l        , ColorLumWeight2) ;}
Flt LinearLumOfSRGBColor  (C Vec &s) {return              Dot(SRGBToLinear(s), ColorLumWeight2) ;}
Flt   SRGBLumOfLinearColor(C Vec &l) {return LinearToSRGB(Dot(      l        , ColorLumWeight2));}
Flt   SRGBLumOfSRGBColor  (C Vec &s) {return LinearToSRGB(Dot(SRGBToLinear(s), ColorLumWeight2));}
/******************************************************************************/
#if WINDOWS_OLD && DX11
   #include <Icm.h>
#endif
Str GetColorProfilePath() // get the path for current monitor color profile
{
#if WINDOWS_OLD && DX11
   if(SwapChain)
   {
      IDXGIOutput *output=null; SwapChain->GetContainingOutput(&output); if(output)
      {
         DXGI_OUTPUT_DESC desc; Bool ok=OK(output->GetDesc(&desc)); output->Release();
         if(ok)
         {
            DISPLAY_DEVICEW displayDevice; Zero(displayDevice); displayDevice.cb=SIZE(displayDevice);
            for(Int i=0; EnumDisplayDevicesW(desc.DeviceName, i, &displayDevice, 0); i++)
               //if(FlagAll(displayDevice.StateFlags, DISPLAY_DEVICE_ACTIVE|DISPLAY_DEVICE_ATTACHED))
            {
               wchar_t file_name[MAX_PATH];
               DLL mscms; if(mscms.createFile("Mscms.dll"))
               if(auto WcsGetDefaultColorProfile=(decltype(&::WcsGetDefaultColorProfile))mscms.getFunc("WcsGetDefaultColorProfile")) // available on Vista+
             //if(BOOL (WINAPI *WcsGetDefaultColorProfile)(WCS_PROFILE_MANAGEMENT_SCOPE scope, PCWSTR pDeviceName, COLORPROFILETYPE cptColorProfileType, COLORPROFILESUBTYPE cpstColorProfileSubType, DWORD dwProfileID, DWORD cbProfileName, LPWSTR pProfileName)=(decltype(WcsGetDefaultColorProfile))mscms.getFunc("WcsGetDefaultColorProfile")) // available on Vista+
               if(WcsGetDefaultColorProfile(WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER, displayDevice.DeviceKey, CPT_ICC, CPST_RGB_WORKING_SPACE, 0, SIZE(file_name), file_name))
               if(file_name[0])
               {
                  if(FullPath(WChar(file_name)))return file_name;
                  return SystemPath(SP_SYSTEM).tailSlash(true)+"spool/drivers/color/"+file_name;
               }
               break;
            }
         }
      }
   }
#endif
   return S;
}
/******************************************************************************
#include "../../../ThirdPartyLibs/QCMS/lib/qcms.h"
   Str p=GetColorProfilePath(); if(p.is())
   {
   #if 0
      PROFILE profile;
      profile.dwType      =PROFILE_FILENAME;
      profile.pProfileData=ConstCast(p());
      profile.cbDataSize  =(p.length()+1)*SIZE(Char);

      if(HPROFILE dest_profile=OpenColorProfileW(&profile, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING))
      {
         LOGCOLORSPACEA lcs; Zero(lcs);
         lcs.lcsSignature=LCS_SIGNATURE; 
         lcs.lcsVersion=0x400;
         lcs.lcsSize=SIZE(lcs);
         lcs.lcsCSType=LCS_sRGB; 
         lcs.lcsIntent=LCS_GM_GRAPHICS; // this is Relative Colorimetric, alternatively try LCS_GM_ABS_COLORIMETRIC

         if(HTRANSFORM transform=CreateColorTransformA(&lcs, dest_profile, null, BEST_MODE|USE_RELATIVE_COLORIMETRIC))
         {
            COLOR src, dest;
            REPD(y, img.h())
            REPD(x, img.w())
            {
               Vec4 c=img.colorF(x, y);
               src.rgb.red  =Mid(RoundPos(c.x*65535), 0, 65535);
               src.rgb.green=Mid(RoundPos(c.y*65535), 0, 65535);
               src.rgb.blue =Mid(RoundPos(c.z*65535), 0, 65535);
               TranslateColors(transform, &src, 1, COLOR_RGB, &dest, COLOR_RGB);
               c.x=dest.rgb.red  /65535.0f;
               c.y=dest.rgb.green/65535.0f;
               c.z=dest.rgb.blue /65535.0f;
               img.colorF(x, y, c);
            }
            DeleteColorTransform(transform);
         }
         CloseColorProfile(dest_profile);
      }
   #else
      if(qcms_profile *qcms_dest=qcms_profile_from_unicode_path(p))
      {
         if(qcms_profile *qcms_srgb=qcms_profile_sRGB())
         {
            if(qcms_transform *transform=qcms_transform_create(qcms_srgb, QCMS_DATA_RGB_8, qcms_dest, QCMS_DATA_RGB_8, QCMS_INTENT_RELATIVE_COLORIMETRIC))
            {
                           REPD(y, img.h())
                           REPD(x, img.w())
                           {
                              Color c=img.color(x, y), d;
                              qcms_transform_data(transform, &c, &d, 1); d.a=c.a;
                              img.color(x, y, d);
                           }

               qcms_transform_release(transform);
            }
            qcms_profile_release(qcms_srgb);
         }
         qcms_profile_release(qcms_dest);
      }
   #endif
   }
/******************************************************************************/
}
/******************************************************************************/
