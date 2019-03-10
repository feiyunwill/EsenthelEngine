/******************************************************************************/
#include "stdafx.h"

#include "../../../../ThirdPartyLibs/begin.h"

#define BC7_LIB_DIRECTX    1
#define BC7_LIB_TEXGENPACK 2 // faster than BC7_LIB_DIRECTX
#define BC7_DEC BC7_LIB_TEXGENPACK

#if BC7_DEC==BC7_LIB_DIRECTX
   #include "../../../../ThirdPartyLibs/DirectXMath/include.h"
   #if !WINDOWS
      #define register
   #endif
   #include "../../../../ThirdPartyLibs/DirectXTex/BC.h"
 //#include "../../../../ThirdPartyLibs/DirectXTex/BC.cpp"
   #include "../../../../ThirdPartyLibs/DirectXTex/BC6HBC7.cpp"
#elif BC7_DEC==BC7_LIB_TEXGENPACK
   namespace TGP
   {
      #pragma runtime_checks("", off)
      #include "../../../../ThirdPartyLibs/TexGenPack/bptc.c"
      #pragma runtime_checks("", restore)
   }
#endif

#include "../../../../ThirdPartyLibs/end.h"

namespace EE{
/******************************************************************************/
Bool (*CompressBC7)(C Image &src, Image &dest);
/******************************************************************************/
static inline UInt B15(UInt x) {return (x*255/*+ 7*/)/15;} // scale from 0..15 to 0..255 byte range (this version exactly matches float with Round, +7 is not needed in this case, function will return the same value with or without it)
static inline UInt B31(UInt x) {return (x*255  +15  )/31;} // scale from 0..31 to 0..255 byte range (this version exactly matches float with Round)
static inline UInt B63(UInt x) {return (x*255  +31  )/63;} // scale from 0..63 to 0..255 byte range (this version exactly matches float with Round)

static inline void Col565(Color &color, UShort u) {color.set(B31((u>>11)&0x1F), B63((u>>5)&0x3F), B31(u&0x1F), 255);}

static inline U64 GetU64(CPtr data) {return Unaligned(*(U64*)data);}
/******************************************************************************/
// BLOCK
/******************************************************************************/
static inline void _DecompressBlockBC1(C Byte *b, Color color[4], UInt &cis)
{
   UShort c0=*(UShort*)(b  ),
          c1=*(UShort*)(b+2);
   Col565(color[0], c0);
   Col565(color[1], c1);
   if(c0<=c1)
   {
      color[2].r=(color[0].r + color[1].r + 1)/2;
      color[2].g=(color[0].g + color[1].g + 1)/2;
      color[2].b=(color[0].b + color[1].b + 1)/2;
      color[2].a=255;

      color[3].zero();
   }else
   {
      color[2].r=(color[0].r*2 + color[1].r*1 + 1)/3;
      color[2].g=(color[0].g*2 + color[1].g*1 + 1)/3;
      color[2].b=(color[0].b*2 + color[1].b*1 + 1)/3;
      color[2].a=255;

      color[3].r=(color[0].r*1 + color[1].r*2 + 1)/3;
      color[3].g=(color[0].g*1 + color[1].g*2 + 1)/3;
      color[3].b=(color[0].b*1 + color[1].b*2 + 1)/3;
      color[3].a=255;
   }
   cis=*(UInt*)(b+4);
}
void DecompressBlockBC1(C Byte *b, Color (&block)[4][4])
{
   Color color[4]; UInt cis; _DecompressBlockBC1(b, color, cis);
   REPD(y, 4)
   REPD(x, 4)
   {
      Int i=x+(y<<2),         // pixel index
         ci=((cis>>(2*i))&3); // color index
      block[y][x]=color[ci];
   }
}
void DecompressBlockBC1(C Byte *b, Color *dest, Int pitch)
{
   Color color[4]; UInt cis; _DecompressBlockBC1(b, color, cis);
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      REPD(x, 4)
      {
         Int i=x+(y<<2),         // pixel index
            ci=((cis>>(2*i))&3); // color index
         dest[x]=color[ci];
      }
      dest=(Color*)((Byte*)dest+pitch);
   }
}
/******************************************************************************/
static inline void _DecompressBlockBC2(C Byte *b, Color color[4], UInt &cis)
{
   UShort c0=*(UShort*)(b+ 8),
          c1=*(UShort*)(b+10);
   Col565(color[0], c0);
   Col565(color[1], c1);

   color[2].r=(color[0].r*2 + color[1].r*1 + 1)/3;
   color[2].g=(color[0].g*2 + color[1].g*1 + 1)/3;
   color[2].b=(color[0].b*2 + color[1].b*1 + 1)/3;

   color[3].r=(color[0].r*1 + color[1].r*2 + 1)/3;
   color[3].g=(color[0].g*1 + color[1].g*2 + 1)/3;
   color[3].b=(color[0].b*1 + color[1].b*2 + 1)/3;

   cis=*(UInt*)(b+12);
}
void DecompressBlockBC2(C Byte *b, Color (&block)[4][4])
{
   Color color[4]; UInt cis; _DecompressBlockBC2(b, color, cis);
   ULong alpha=GetU64(b);
   REPD(y, 4)
   REPD(x, 4)
   {
      Int i=x+(y<<2),         // pixel index
         ci=((cis>>(2*i))&3); // color index
      Color &col=color[ci];
      block[y][x].set(col.r, col.g, col.b, B15((alpha>>(4*i))&15));
   }
}
void DecompressBlockBC2(C Byte *b, Color *dest, Int pitch)
{
   Color color[4]; UInt cis; _DecompressBlockBC2(b, color, cis);
   ULong alpha=GetU64(b);
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      REPD(x, 4)
      {
         Int i=x+(y<<2),         // pixel index
            ci=((cis>>(2*i))&3); // color index
         Color &col=color[ci];
         dest[x].set(col.r, col.g, col.b, B15((alpha>>(4*i))&15));
      }
      dest=(Color*)((Byte*)dest+pitch);
   }
}
/******************************************************************************/
static inline void _DecompressBlockBC3(C Byte *b, Color color[4], UInt &cis, Byte alpha[8], U64 &ais)
{
  _DecompressBlockBC2(b, color, cis);

   alpha[0]=b[0];
   alpha[1]=b[1];
   if(alpha[0]>alpha[1])
   {
      alpha[2]=(alpha[0]*6 + alpha[1]*1 + 3)/7;
      alpha[3]=(alpha[0]*5 + alpha[1]*2 + 3)/7;
      alpha[4]=(alpha[0]*4 + alpha[1]*3 + 3)/7;
      alpha[5]=(alpha[0]*3 + alpha[1]*4 + 3)/7;
      alpha[6]=(alpha[0]*2 + alpha[1]*5 + 3)/7;
      alpha[7]=(alpha[0]*1 + alpha[1]*6 + 3)/7;
   }else
   {
      alpha[2]=(alpha[0]*4 + alpha[1]*1 + 2)/5;
      alpha[3]=(alpha[0]*3 + alpha[1]*2 + 2)/5;
      alpha[4]=(alpha[0]*2 + alpha[1]*3 + 2)/5;
      alpha[5]=(alpha[0]*1 + alpha[1]*4 + 2)/5;
      alpha[6]=                              0;
      alpha[7]=                            255;
   }
   ais=GetU64(b+2);
}
void DecompressBlockBC3(C Byte *b, Color (&block)[4][4])
{
   Color color[4]; UInt cis; Byte alpha[8]; U64 ais; _DecompressBlockBC3(b, color, cis, alpha, ais);
   REPD(y, 4)
   REPD(x, 4)
   {
      Int i=x+(y<<2),         // pixel index
         ai=((ais>>(3*i))&7), // alpha index
         ci=((cis>>(2*i))&3); // color index
      Color &col=color[ci];
      block[y][x].set(col.r, col.g, col.b, alpha[ai]);
   }
}
void DecompressBlockBC3(C Byte *b, Color *dest, Int pitch)
{
   Color color[4]; UInt cis; Byte alpha[8]; U64 ais; _DecompressBlockBC3(b, color, cis, alpha, ais);
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      REPD(x, 4)
      {
         Int i=x+(y<<2),         // pixel index
            ai=((ais>>(3*i))&7), // alpha index
            ci=((cis>>(2*i))&3); // color index
         Color &col=color[ci];
         dest[x].set(col.r, col.g, col.b, alpha[ai]);
      }
      dest=(Color*)((Byte*)dest+pitch);
   }
}
/******************************************************************************/
void DecompressBlockBC7(C Byte *b, Color (&block)[4][4])
{
#if BC7_DEC==BC7_LIB_DIRECTX
   DirectX::D3DXDecodeBC7(block, b);
#elif BC7_DEC==BC7_LIB_TEXGENPACK
   TGP::draw_block4x4_bptc(b, (UInt*)block);
#endif
}
void DecompressBlockBC7(C Byte *b, Color *dest, Int pitch)
{
   Color block[4][4];
#if BC7_DEC==BC7_LIB_DIRECTX
   DirectX::D3DXDecodeBC7(block, b);
#elif BC7_DEC==BC7_LIB_TEXGENPACK
   TGP::draw_block4x4_bptc(b, (UInt*)block);
#endif
   FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
   {
      CopyFast(dest, block[y], SIZE(Color)*4);
      dest=(Color*)((Byte*)dest+pitch);
   }
}
/******************************************************************************/
// PIXEL
/******************************************************************************/
Color DecompressPixelBC1(C Byte *b, Int x, Int y)
{
   Int i=x+(y<<2),                  // pixel index
      ci=((*(U32*)(b+4)>>(2*i))&3); // color index

   // color
   Color  color[4];
   UShort c0=*(UShort*)(b  ),
          c1=*(UShort*)(b+2);
   Col565(color[0], c0);
   Col565(color[1], c1);

   color[ci].a=255;
   if(c0<=c1)
   {
      if(ci==2)
      {
         color[2].r=(color[0].r + color[1].r + 1)/2;
         color[2].g=(color[0].g + color[1].g + 1)/2;
         color[2].b=(color[0].b + color[1].b + 1)/2;
      }else
      if(ci==3)
      {
         color[3].zero();
      }
   }else
   {
      if(ci==2)
      {
         color[2].r=(color[0].r*2 + color[1].r*1 + 1)/3;
         color[2].g=(color[0].g*2 + color[1].g*1 + 1)/3;
         color[2].b=(color[0].b*2 + color[1].b*1 + 1)/3;
      }else
      if(ci==3)
      {
         color[3].r=(color[0].r*1 + color[1].r*2 + 1)/3;
         color[3].g=(color[0].g*1 + color[1].g*2 + 1)/3;
         color[3].b=(color[0].b*1 + color[1].b*2 + 1)/3;
      }
   }
   return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC2(C Byte *b, Int x, Int y)
{
   Int i=x+(y<<2),                   // pixel index
      ci=((*(U32*)(b+12)>>(2*i))&3); // color index

   // color
   Color  color[4];
   UShort c0=*(UShort*)(b+ 8),
          c1=*(UShort*)(b+10);
   Col565(color[0], c0);
   Col565(color[1], c1);
   if(ci==2)
   {
      color[2].r=(color[0].r*2 + color[1].r*1 + 1)/3;
      color[2].g=(color[0].g*2 + color[1].g*1 + 1)/3;
      color[2].b=(color[0].b*2 + color[1].b*1 + 1)/3;
   }else
   if(ci==3)
   {
      color[3].r=(color[0].r*1 + color[1].r*2 + 1)/3;
      color[3].g=(color[0].g*1 + color[1].g*2 + 1)/3;
      color[3].b=(color[0].b*1 + color[1].b*2 + 1)/3;
   }
          color[ci].a=B15((GetU64(b)>>(4*i))&15);
   return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC3(C Byte *b, Int x, Int y)
{
   U64 ais=GetU64(b+2);
   Int i  =x+(y<<2),                   // pixel index
      ai  =((ais>>(3*i))&7),           // alpha index
      ci  =((*(U32*)(b+12)>>(2*i))&3); // color index

   // alpha
   Byte alpha[8];
   alpha[0]=b[0];
   alpha[1]=b[1];
   switch(ai)
   {
      case 2: alpha[2]=((alpha[0]>alpha[1]) ? (alpha[0]*6 + alpha[1]*1 + 3)/7 : (alpha[0]*4 + alpha[1]*1 + 2)/5); break;
      case 3: alpha[3]=((alpha[0]>alpha[1]) ? (alpha[0]*5 + alpha[1]*2 + 3)/7 : (alpha[0]*3 + alpha[1]*2 + 2)/5); break;
      case 4: alpha[4]=((alpha[0]>alpha[1]) ? (alpha[0]*4 + alpha[1]*3 + 3)/7 : (alpha[0]*2 + alpha[1]*3 + 2)/5); break;
      case 5: alpha[5]=((alpha[0]>alpha[1]) ? (alpha[0]*3 + alpha[1]*4 + 3)/7 : (alpha[0]*1 + alpha[1]*4 + 2)/5); break;
      case 6: alpha[6]=((alpha[0]>alpha[1]) ? (alpha[0]*2 + alpha[1]*5 + 3)/7 :                               0); break;
      case 7: alpha[7]=((alpha[0]>alpha[1]) ? (alpha[0]*1 + alpha[1]*6 + 3)/7 :                             255); break;
   }

   // color
   Color  color[4];
   UShort c0=*(UShort*)(b+ 8),
          c1=*(UShort*)(b+10);
   Col565(color[0], c0);
   Col565(color[1], c1);
   if(ci==2)
   {
      color[2].r=(color[0].r*2 + color[1].r*1 + 1)/3;
      color[2].g=(color[0].g*2 + color[1].g*1 + 1)/3;
      color[2].b=(color[0].b*2 + color[1].b*1 + 1)/3;
   }else
   if(ci==3)
   {
      color[3].r=(color[0].r*1 + color[1].r*2 + 1)/3;
      color[3].g=(color[0].g*1 + color[1].g*2 + 1)/3;
      color[3].b=(color[0].b*1 + color[1].b*2 + 1)/3;
   }
          color[ci].a=alpha[ai];
   return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC7(C Byte *b, Int x, Int y)
{
   Color  block[4][4]; DecompressBlockBC7(b, block);
   return block[y][x];
}
/******************************************************************************/
// COMPRESS
/******************************************************************************/
struct BC1
{
   U16 rgb[2]; // 565 colors
   U32 bitmap; // 2bpp rgb bitmap
};
struct BC2
{
   UInt bitmap[2]; // 4bpp alpha bitmap
   BC1  bc1      ; // BC1 rgb data
};
struct BC3
{
   Byte alpha[2] ; // alpha values
   Byte bitmap[6]; // 3bpp alpha bitmap
   BC1  bc1      ; // BC1 rgb data
};
static const Flt fEpsilon =Sqr(0.25f/64);
static const Flt pC3    []={2.0f/2.0f, 1.0f/2.0f, 0.0f/2.0f};
static const Flt pD3    []={0.0f/2.0f, 1.0f/2.0f, 2.0f/2.0f};
static const Flt pC4    []={3.0f/3.0f, 2.0f/3.0f, 1.0f/3.0f, 0.0f/3.0f};
static const Flt pD4    []={0.0f/3.0f, 1.0f/3.0f, 2.0f/3.0f, 3.0f/3.0f};
static const Flt pC6    []={5.0f/5.0f, 4.0f/5.0f, 3.0f/5.0f, 2.0f/5.0f, 1.0f/5.0f, 0.0f/5.0f};
static const Flt pD6    []={0.0f/5.0f, 1.0f/5.0f, 2.0f/5.0f, 3.0f/5.0f, 4.0f/5.0f, 5.0f/5.0f};
static const Flt pC8    []={7.0f/7.0f, 6.0f/7.0f, 5.0f/7.0f, 4.0f/7.0f, 3.0f/7.0f, 2.0f/7.0f, 1.0f/7.0f, 0.0f/7.0f};
static const Flt pD8    []={0.0f/7.0f, 1.0f/7.0f, 2.0f/7.0f, 3.0f/7.0f, 4.0f/7.0f, 5.0f/7.0f, 6.0f/7.0f, 7.0f/7.0f};
static const Int pSteps3[]={0, 2, 1};
static const Int pSteps4[]={0, 2, 3, 1};
static const Int pSteps6[]={0, 2, 3, 4, 5, 1};
static const Int pSteps8[]={0, 2, 3, 4, 5, 6, 7, 1};

static void Decode565(Vec &col, UInt w565)
{
   col.set(((w565>>11)&31)/31.0f,
           ((w565>> 5)&63)/63.0f,
           ((w565>> 0)&31)/31.0f);
}
static UInt Encode565(C Vec &col)
{
   return ((col.x<=0) ? 0 : (col.x>=1) ? (31<<11) : (RoundPos(col.x*31)<<11))
        | ((col.y<=0) ? 0 : (col.y>=1) ? (63<< 5) : (RoundPos(col.y*63)<< 5))
        | ((col.z<=0) ? 0 : (col.z>=1) ? (31<< 0) : (RoundPos(col.z*31)<< 0));
}
static void OptimizeRGB(Vec &color_a, Vec &color_b, Vec4 (&color)[16], Int steps)
{
 C Flt *pC, *pD; if(steps==3){pC=pC3; pD=pD3;}else{pC=pC4; pD=pD4;}

   Vec min=color[15].xyz, max=min;
   REP(15)
   {
    C Vec &c=color[i].xyz;
      MIN(min.x, c.x); MAX(max.x, c.x);
      MIN(min.y, c.y); MAX(max.y, c.y);
      MIN(min.z, c.z); MAX(max.z, c.z);
   }
   Vec dir=max-min;
   Flt l=dir.length2();
   if(l<=FLT_MIN){color_a=min; color_b=max; return;} // Single color block.. no need to root-find
   dir/=l;
   Vec mid=Avg(min, max);
   Flt fDir[4]={0,0,0,0};
   FREPA(color)
   {
      Vec c=color[i].xyz-mid;
      c*=dir;
      fDir[0]+=Sqr(c.x + c.y + c.z);
      fDir[1]+=Sqr(c.x + c.y - c.z);
      fDir[2]+=Sqr(c.x - c.y + c.z);
      fDir[3]+=Sqr(c.x - c.y - c.z);
   }
   Int iDirMax=0; Flt fDirMax=fDir[0]; for(Int iDir=1; iDir<4; iDir++)if(fDir[iDir]>fDirMax)fDirMax=fDir[iDirMax=iDir];
   if(iDirMax&2)Swap(min.y, max.y);
   if(iDirMax&1)Swap(min.z, max.z);

   // Two color block.. no need to root-find
   if(l<1.0f/4096){color_a=min; color_b=max; return;}

   // Use Newton's Method to find local minima of sum-of-squares error
   Flt fSteps=steps-1;
   REP(16) // original version had 8, but 16 generates better results
   {
      // Calculate new steps
      Vec pSteps[4];
      FREP(steps)pSteps[i]=min*pC[i] + max*pD[i];

      // Calculate color direction
      dir=max-min;
      l=dir.length2();
      if(l<1.0f/4096)break;
      dir*=fSteps/l; // divide by 'l' and not 'Sqrt(l)' because we don't want to use it to calculate unit distances, but fraction distances

      // Evaluate function, and derivatives
      Flt start=Dot(min, dir), d2X=0, d2Y=0; Vec dX=0, dY=0;

      FREPA(color)
      {
         Flt dot=Dot(color[i].xyz, dir)-start;

         Int s;
         if(dot<=0     )s=      0;else
         if(dot>=fSteps)s=steps-1;else
                        s=RoundPos(dot);

         Vec diff=pSteps[s]-color[i].xyz;
         Flt fC=pC[s]/8, fD=pD[s]/8;

         d2X+=fC*pC[s];
         dX +=fC*diff;

         d2Y+=fD*pD[s];
         dY +=fD*diff;
      }

      // Move endpoints
      if(d2X>0)min-=dX/d2X;
      if(d2Y>0)max-=dY/d2Y;

   #if 0 // original
      if(Sqr(dX.x)<fEpsilon && Sqr(dX.y)<fEpsilon && Sqr(dX.z)<fEpsilon
      && Sqr(dY.x)<fEpsilon && Sqr(dY.y)<fEpsilon && Sqr(dY.z)<fEpsilon)break;
   #else
      if(dX.length2()<fEpsilon && dY.length2()<fEpsilon)break;
   #endif
   }

   color_a=min;
   color_b=max;
}
static void _CompressBC1(BC1 &bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a, Flt alpha_ref) // !! Warning: 'color' gets modified !!, 'alpha_ref'=use zero to disable alpha encoding
{
   Int steps;
   if(alpha_ref>0)
   {
      if(dither_a)
      {
         Flt error[16]; Zero(error);
         FREPA(color)
         {
            Flt &dest=color[i].w, alpha=dest+error[i];
                 dest=RoundPos(alpha);
            Flt  diff=alpha-dest;

            if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
            if(i<12)
            {
               if( i&3    )error[i+3]+=diff*(3.0f/16);
                           error[i+4]+=diff*(5.0f/16);
               if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
            }
         }
      }

      Int transparent=0;
      REPA(color)if(color[i].w<alpha_ref)transparent++;
      if(transparent==16)
      {
         bc.rgb[0]=0x0000;
         bc.rgb[1]=0xffff;
         bc.bitmap=0xffffffff;
         return;
      }
      steps=(transparent ? 3 : 4);
   }else steps=4;

   Vec4 col  [16];
   Vec  error[16];
   if(dither_rgb)Zero(error);

   FREPA(col)
   {
      Vec clr=color[i].xyz;
      if(dither_rgb)clr+=error[i];

      col[i].x=RoundPos(clr.x*31)/31.0f;
      col[i].y=RoundPos(clr.y*63)/63.0f;
      col[i].z=RoundPos(clr.z*31)/31.0f;

      if(dither_rgb)
      {
         Vec diff=clr-col[i].xyz;

         if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
         if(i<12)
         {
            if( i&3    )error[i+3]+=diff*(3.0f/16);
                        error[i+4]+=diff*(5.0f/16);
            if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
         }
      }

      if(weight)col[i].xyz*=*weight;
   }

   Vec color_a, color_b;
   OptimizeRGB(color_a, color_b, col, steps);

   if(weight)
   {
      color_a/=*weight;
      color_b/=*weight;
   }
   UInt wColorA=Encode565(color_a), wColorB=Encode565(color_b);
   if(steps==4 && wColorA==wColorB)
   {
      bc.rgb[0]=wColorA;
      bc.rgb[1]=wColorB;
      bc.bitmap=0x00000000;
      return;
   }
   Decode565(color_a, wColorA);
   Decode565(color_b, wColorB);
   if(weight)
   {
      color_a*=*weight;
      color_b*=*weight;
   }

   // Calculate color steps
   Vec step[4];
   if((steps==3)==(wColorA<=wColorB))
   {
      bc.rgb[0]=wColorA;
      bc.rgb[1]=wColorB;
      step[0]=color_a;
      step[1]=color_b;
   }else
   {
      bc.rgb[0]=wColorB;
      bc.rgb[1]=wColorA;
      step[0]=color_b;
      step[1]=color_a;
   }

   const Int *pSteps;
   if(steps==3)
   {
      pSteps=pSteps3;
      step[2]=Avg(step[0], step[1]);
   }else
   {
      pSteps=pSteps4;
   #if 0
      step[2]=Lerp(step[0], step[1], 1.0f/3);
      step[3]=Lerp(step[0], step[1], 2.0f/3);
   #else
      Vec d=(step[1]-step[0])/3;
      step[2]=step[0]+d;
      step[3]=step[0]+d*2;
   #endif
   }

   // Calculate color direction
   Flt fSteps=steps-1;
   Vec dir=step[1]-step[0]; if(wColorA!=wColorB)dir*=fSteps/dir.length2(); // divide by 'length2' and not 'length' because we don't want to use it to calculate unit distances, but fraction distances
   Flt start=Dot(step[0], dir);

   // Encode colors
   UInt bitmap=0;
   if(dither_rgb)Zero(error);

   FREPA(color)
   {
      if(steps==3 && color[i].w<alpha_ref)bitmap=(3<<30)|(bitmap>>2);else
      {
         Vec clr=color[i].xyz;
         if(weight)clr*=*weight;
         if(dither_rgb)clr+=error[i];
         Flt dot=Dot(clr, dir)-start;

         Int s;
         if(dot<=0.0f  )s=0;else
         if(dot>=fSteps)s=1;else
                        s=pSteps[RoundPos(dot)];

         bitmap=(s<<30)|(bitmap>>2);

         if(dither_rgb)
         {
            Vec diff=clr-step[s];

            if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
            if(i<12)
            {
               if( i&3    )error[i+3]+=diff*(3.0f/16);
                           error[i+4]+=diff*(5.0f/16);
               if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
            }
         }
      }
   }
   bc.bitmap=bitmap;
}
static void CompressBC1(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {return _CompressBC1(*(BC1*)bc, color, weight, dither_rgb, dither_a, 0.5f);}
static void CompressBC2(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a)
{
   BC2 &bc2=*(BC2*)bc;
  _CompressBC1(bc2.bc1, color, weight, dither_rgb, false, 0);

   bc2.bitmap[0]=0;
   bc2.bitmap[1]=0;
   Flt error[16]; if(dither_a)Zero(error);
   FREPA(color)
   {
      Flt alpha=color[i].w;
      if(dither_a)alpha+=error[i];
      UInt u=RoundPos(alpha*15);
      bc2.bitmap[i>>3]>>=4;
      bc2.bitmap[i>>3] |=(u<<28);
      if(dither_a)
      {     
         Flt diff=alpha-u/15.0f;
         if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
         if(i<12)
         {
            if( i&3    )error[i+3]+=diff*(3.0f/16);
                        error[i+4]+=diff*(5.0f/16);
            if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
         }
      }
   }
}
static void OptimizeAlpha(Flt *pX, Flt *pY, const Flt *pPoints, Int cSteps)
{
   const Flt *pC=(6==cSteps) ? pC6 : pC8;
   const Flt *pD=(6==cSteps) ? pD6 : pD8;

   const Flt MIN_VALUE=0;
   const Flt MAX_VALUE=1;

   // Find Min and Max points, as starting point
   Flt fX=MAX_VALUE;
   Flt fY=MIN_VALUE;
   if(cSteps==8)
   {
      FREP(16)
      {
         MIN(fX, pPoints[i]);
         MAX(fY, pPoints[i]);
      }
   }else
   {
      FREP(16)
      {
         if(pPoints[i]<fX && pPoints[i]>MIN_VALUE)fX=pPoints[i];
         if(pPoints[i]>fY && pPoints[i]<MAX_VALUE)fY=pPoints[i];
      }
      if(fX==fY)fY=MAX_VALUE;
   }

   // Use Newton's Method to find local minima of sum-of-squares error
   Flt fSteps=cSteps-1;
   for(Int iIteration=0; iIteration<8; iIteration++)
   {
      if(fY-fX < 1.0f/256)break;
      Flt fScale=fSteps/(fY-fX);

      // Calculate new steps
      Flt pSteps[8];

      for(Int iStep=0; iStep<cSteps; iStep++)pSteps[iStep]=pC[iStep]*fX + pD[iStep]*fY;

      if(cSteps==6)
      {
         pSteps[6]=MIN_VALUE;
         pSteps[7]=MAX_VALUE;
      }

      // Evaluate function, and derivatives
      Flt dX=0, dY=0, d2X=0, d2Y=0;
      FREP(16)
      {
         Flt fDot=(pPoints[i]-fX)*fScale;
         Int iStep;
         if(fDot<=     0)iStep=(cSteps==6 && (pPoints[i]<= fX   *0.5f)) ? 6 :          0;else
         if(fDot>=fSteps)iStep=(cSteps==6 && (pPoints[i]>=(fY+1)*0.5f)) ? 7 : (cSteps-1);else
                         iStep=RoundPos(fDot);
         if(iStep<cSteps)
         {
            Flt diff=pSteps[iStep]-pPoints[i];

            dX +=pC[iStep]*diff;
            d2X+=pC[iStep]*pC[iStep];

            dY +=pD[iStep]*diff;
            d2Y+=pD[iStep]*pD[iStep];
         }
      }

      // Move endpoints
      if(d2X>0)fX-=dX/d2X;
      if(d2Y>0)fY-=dY/d2Y;

      if(fX>fY)Swap(fX, fY);

      if(Sqr(dX)<1.0f/64 && Sqr(dY)<1.0f/64)break;
   }

   *pX=Mid(fX, MIN_VALUE, MAX_VALUE);
   *pY=Mid(fY, MIN_VALUE, MAX_VALUE);
}
static void CompressBC3(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a)
{
   BC3 &bc3=*(BC3*)bc;
  _CompressBC1(bc3.bc1, color, weight, dither_rgb, false, 0);

   Flt fAlpha[16], error[16], fMinAlpha=color[0].w, fMaxAlpha=color[0].w;
   if(dither_a)Zero(error);
   FREPA(color)
   {
      Flt fAlph=color[i].w;
      if(dither_a)fAlph+=error[i];
      fAlpha[i]=RoundPos(fAlph*255)/255.0f;
      if(fAlpha[i]<fMinAlpha)fMinAlpha=fAlpha[i];else
      if(fAlpha[i]>fMaxAlpha)fMaxAlpha=fAlpha[i];
    
      if(dither_a)
      {
         Flt diff=fAlph-fAlpha[i];
         if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
         if(i<12)
         {
            if( i&3    )error[i+3]+=diff*(3.0f/16);
                        error[i+4]+=diff*(5.0f/16);
            if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
         }
      }
   }
   if(fMinAlpha>=1)
   {
      bc3.alpha[0]=0xff;
      bc3.alpha[1]=0xff;
      Zero(bc3.bitmap);
      return;
   }

   // Optimize and Quantize Min and Max values
   Int  uSteps=((fMinAlpha==0) || (fMaxAlpha==1)) ? 6 : 8;
   Flt  fAlphaA, fAlphaB; OptimizeAlpha(&fAlphaA, &fAlphaB, fAlpha, uSteps);
   Byte bAlphaA=RoundPos(fAlphaA*255); fAlphaA=bAlphaA/255.0f;
   Byte bAlphaB=RoundPos(fAlphaB*255); fAlphaB=bAlphaB/255.0f;

   // Setup block
   if(uSteps==8 && bAlphaA==bAlphaB)
   {
      bc3.alpha[0]=bAlphaA;
      bc3.alpha[1]=bAlphaB;
      Zero(bc3.bitmap);
      return;
   }

   const Int *pSteps;
   Flt fStep[8];

   if(uSteps==6)
   {
      bc3.alpha[0]=bAlphaA;
      bc3.alpha[1]=bAlphaB;

      fStep[0]=fAlphaA;
      fStep[1]=fAlphaB;

      for(Int i=1; i<5; i++)fStep[i+1]=(fStep[0]*(5-i) + fStep[1]*i)/5;

      fStep[6]=0;
      fStep[7]=1;

      pSteps=pSteps6;
   }else
   {
      bc3.alpha[0]=bAlphaB;
      bc3.alpha[1]=bAlphaA;

      fStep[0]=fAlphaB;
      fStep[1]=fAlphaA;

      for(Int i=1; i<7; i++)fStep[i+1]=(fStep[0]*(7-i) + fStep[1]*i)/7;

      pSteps=pSteps8;
   }

   // Encode alpha bitmap
   Flt fSteps=uSteps-1, fScale=(fStep[0]!=fStep[1]) ? (fSteps/(fStep[1]-fStep[0])) : 0;
   if(dither_a)Zero(error);
   for(Int iSet=0; iSet<2; iSet++)
   {
      UInt dw=0;
      Int  iMin=iSet*8;
      Int  iLim=iMin+8;
      for(Int i=iMin; i<iLim; i++)
      {
         Flt fAlph=color[i].w;
         if(dither_a)fAlph+=error[i];
         Flt fDot=(fAlph-fStep[0])*fScale;

         Int iStep;
         if(fDot<=0     )iStep=(6==uSteps && (fAlph<= fStep[0]        *0.5f)) ? 6 : 0;else
         if(fDot>=fSteps)iStep=(6==uSteps && (fAlph>=(fStep[1] + 1.0f)*0.5f)) ? 7 : 1;else
                         iStep=pSteps[RoundPos(fDot)];

         dw=(iStep<<21)|(dw>>3);

         if(dither_a)
         {
            Flt diff=fAlph-fStep[iStep];
            if((i&3)!=3)error[i+1]+=diff*(7.0f/16);
            if(i<12)
            {
               if( i&3    )error[i+3]+=diff*(3.0f/16);
                           error[i+4]+=diff*(5.0f/16);
               if((i&3)!=3)error[i+5]+=diff*(1.0f/16);
            }
         }
      }

      bc3.bitmap[0+iSet*3]=((Byte*)&dw)[0];
      bc3.bitmap[1+iSet*3]=((Byte*)&dw)[1];
      bc3.bitmap[2+iSet*3]=((Byte*)&dw)[2];
   }
}
/******************************************************************************/
static const Vec BCWeights=ColorLumWeight*0.65f;
Bool CompressBC(C Image &src, Image &dest, Bool mtrl_base_1) // no need to store this in a separate CPP file, because its code size is small
{
   Bool ok=false;
   if(dest.hwType()==IMAGE_BC1 || dest.hwType()==IMAGE_BC2 || dest.hwType()==IMAGE_BC3)
   {
   #if 1
      if(src.lockRead())
      {
         if(dest.lock(LOCK_WRITE))
         {
            ok=true;
            void (*compress_block)(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a)=((dest.hwType()==IMAGE_BC1) ? CompressBC1 :
                                                                                                                (dest.hwType()==IMAGE_BC2) ? CompressBC2 :
                                                                                                                (dest.hwType()==IMAGE_BC3) ? CompressBC3 : null);
            Int  x_blocks=dest.hwW()/4, // operate on HW size to process partial and Pow2Padded blocks too
                 y_blocks=dest.hwH()/4,
                 x_mul   =((dest.hwType()==IMAGE_BC1) ? 8 : 16);
            Vec4 rgba[16];
            Vec  weight(1, 1, 0.5f); // #MaterialTextureChannelOrder - NrmX, NrmY, Spec, Alpha

            REPD( z, dest.d())
            REPD(by, y_blocks)
            {
               Int py=by*4, yo[4]; REPAO(yo)=Min(py+i, src.h()-1); // use clamping to avoid black borders
               Byte *dest_data=dest.data() + by*dest.pitch() + z*dest.pitch2();
               REPD(bx, x_blocks)
               {
                  Int px=bx*4, xo[4]; REPAO(xo)=Min(px+i, src.w()-1); // use clamping to avoid black borders
                  src.gather(rgba, xo, Elms(xo), yo, Elms(yo), &z, 1);
                  if(!mtrl_base_1)
                  {
                     Vec4 min, max; MinMax(rgba, 4*4, min, max);
                  #if 1 // this gave better results
                     weight=BCWeights + max.xyz + max.xyz-min.xyz; // max + delta = max + (max-min)
                  #else
                     weight=LinearToSRGB(ColorLumWeight2);
                  #endif
                  }
                  compress_block(dest_data + bx*x_mul, rgba, &weight, true, true);
               }
            }

            dest.unlock();
         }
         src.unlock();
      }
   #elif 1 // Intel ISPC
      only BC1 now supported
      Image temp; C Image *s=&src;
      if(s->hwType()!=IMAGE_R8G8B8A8 || s->w()!=dest.hwW() || s->h()!=dest.hwH())
      {
         if(s->copyTry(temp, dest.hwW(), dest.hwH(), 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1, FILTER_NO_STRETCH, true))s=&temp;else return false; // we need to cover the area for entire HW size, to process partial and Pow2Padded blocks too
      }
      if(s->lockRead())
      {
         if(dest.lock(LOCK_WRITE))
         {
            ok=true;
            Int x_blocks=dest.hwW()/4, // operate on HW size to process partial and Pow2Padded blocks too
                y_blocks=dest.hwH()/4,
                x_mul   =((dest.hwType()==IMAGE_BC1) ? 8 : 16);
            rgba_surface surf;
            surf.width =s->w    ();
            surf.height=s->h    ();
            surf.stride=s->pitch();
            REPD(z, dest.d())
            {
               surf.ptr=ConstCast(s->data()+z*s->pitch2());
               if(dest.pitch()==x_blocks*x_mul)CompressBlocksBC1(&surf, dest.data()+z*dest.pitch2());else
               {
                  split into multiple calls here
               }
            }
            dest.unlock();
         }
         s->unlock();
      }
   #else
      if(src.d()==1) // use Driver compression if possible (faster and more accurate than Squish library)
      {
      #if DX9
         Image src_surf, dest_surf; if(src.copyTry  (src_surf, src .w(), src .h(), src .d(), IMAGE_B8G8R8A8, IMAGE_SURF_SCRATCH)
                              && dest_surf.createTry(          dest.w(), dest.h(), dest.d(), dest.hwType() , IMAGE_SURF_SCRATCH, 1, false))
         {
            src_surf.dither(dest.hwType());
            SyncLockerEx locker(D._lock);
            if(OK(D3DXLoadSurfaceFromSurface(dest_surf._surf, null, null, src_surf._surf, null, null, D3DX_FILTER_NONE, 0)))
            {
               locker.off();
               if(dest.injectMipMap(dest_surf, 0))return true;
            }
         }
      #elif DX11
         Image src_txtr, dest_txtr; if(src.copyTry  (src_txtr, src .w(), src .h(), src .d(), IMAGE_R8G8B8A8, IMAGE_SURF_SCRATCH)
                              && dest_txtr.createTry(          dest.w(), dest.h(), dest.d(), dest.hwType() , IMAGE_SURF_SCRATCH, 1, false))
         {
            src_txtr.dither(dest.hwType());
            D3DX11_TEXTURE_LOAD_INFO info;
            info.NumElements=1;
            info.NumMips    =1;
            info.Filter     =D3DX11_FILTER_NONE;
            SyncLockerEx locker(D._lock);
            if(OK(D3DX11LoadTextureFromTexture(D3DC, src_txtr._txtr, &info, dest_txtr._txtr)))
            {
               locker.off();
               if(dest.injectMipMap(dest_txtr, 0))return true;
            }
         }
      #elif GL && !GL_ES // Nvidia OpenGL driver compression is not as good as DX (ATI is unknown)
       C Image *s=&src;
         Image temp; if(s->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))s=&temp.dither(dest.hwType());
         Image dest_txtr; if(dest_txtr.createTry(dest.w(), dest.h(), dest.d(), dest.hwType(), IMAGE_2D, 1, false))
         {
            if(s->lockRead())
            {
               SyncLocker locker(D._lock);
               glGetError(); // clear any previous errors
               D.texBind(GL_TEXTURE_2D, dest_txtr._txtr);
               glTexImage2D(GL_TEXTURE_2D, 0, ImageTI[dest_txtr.hwType()].format, s->w(), s->h(), 0, SourceGLFormat(s->hwType()), SourceGLType(s->hwType()), s->data());
               glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               Bool ok=(glGetError()==GL_NO_ERROR);
               s->unlock();
               if(ok)if(dest.injectMipMap(dest_txtr, 0))return true;
            }
         }
      #endif
      }
      if(src.lockRead())
      {
         if(dest.lock(LOCK_WRITE))
         {
            ok=true;

            Int flags=0;
            if(dest.hwType()==IMAGE_BC1)flags|=squish::kDxt1;else
            if(dest.hwType()==IMAGE_BC2)flags|=squish::kDxt3;else
            if(dest.hwType()==IMAGE_BC3)flags|=squish::kDxt5;

            if(quality==0)flags|=squish::kColourRangeFit           ;else
            if(quality==1)flags|=squish::kColourClusterFit         ;else
            if(quality==2)flags|=squish::kColourIterativeClusterFit;

            if(perceptual)flags|=squish::kColourMetricPerceptual;
            else          flags|=squish::kColourMetricUniform   ;

            if(alpha_weight)flags|=squish::kWeightColourByAlpha;

            Int x_blocks=dest.w()/4,
                y_blocks=dest.h()/4,
                x_mul   =((dest.hwType()==IMAGE_BC1) ? 8 : 16);
            REPD( z, dest.d())
            REPD(by, y_blocks)
            REPD(bx, x_blocks)
            {
               Int px=bx*4, py=by*4; // pixel

               Color rgba[4][4];
               REPD(y, 4)
               REPD(x, 4)rgba[y][x]=src.color3D(px+x, py+y, z);

               squish::Compress((squish::u8*)rgba, dest.data() + bx*x_mul + by*dest.pitch() + z*dest.pitch2(), flags);
            }

            dest.unlock();
         }
         src.unlock();
      }
   #endif
   }
   return ok;
}
/******************************************************************************/
}
/******************************************************************************/
