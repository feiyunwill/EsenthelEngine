/******************************************************************************/
#include "!Header.h"
#include "Ambient Occlusion.h"
#include "Sky.h"
#include "Layered Clouds.h"
#include "Hdr.h"
#include "Water.h"
#include "Overlay.h"
#include "Simple.h"
#include "Volumetric.h"
#include "Vertex Fog.h"
#include "Fog.h"
#include "Fur.h"
/******************************************************************************/
ImageCube Cub;
/******************************************************************************/
VecH4 TexCubicFast(Vec2 inTex)
{
#if 0 // original
   Vec2 tex =inTex*ColSize.zw-0.5f,
        texi=Floor(tex),
        texf=tex-texi;
        texi-=0.5f; texi*=ColSize.xy;

   Vec4 c00=TexPoint(Col, texi+ColSize.xy*Vec2(0, 0)), c10=TexPoint(Col, texi+ColSize.xy*Vec2(1, 0)), c20=TexPoint(Col, texi+ColSize.xy*Vec2(2, 0)), c30=TexPoint(Col, texi+ColSize.xy*Vec2(3, 0)),
        c01=TexPoint(Col, texi+ColSize.xy*Vec2(0, 1)), c11=TexPoint(Col, texi+ColSize.xy*Vec2(1, 1)), c21=TexPoint(Col, texi+ColSize.xy*Vec2(2, 1)), c31=TexPoint(Col, texi+ColSize.xy*Vec2(3, 1)),
        c02=TexPoint(Col, texi+ColSize.xy*Vec2(0, 2)), c12=TexPoint(Col, texi+ColSize.xy*Vec2(1, 2)), c22=TexPoint(Col, texi+ColSize.xy*Vec2(2, 2)), c32=TexPoint(Col, texi+ColSize.xy*Vec2(3, 2)),
        c03=TexPoint(Col, texi+ColSize.xy*Vec2(0, 3)), c13=TexPoint(Col, texi+ColSize.xy*Vec2(1, 3)), c23=TexPoint(Col, texi+ColSize.xy*Vec2(2, 3)), c33=TexPoint(Col, texi+ColSize.xy*Vec2(3, 3));

   Vec4 c0=Lerp4(c00, c10, c20, c30, texf.x),
        c1=Lerp4(c01, c11, c21, c31, texf.x),
        c2=Lerp4(c02, c12, c22, c32, texf.x),
        c3=Lerp4(c03, c13, c23, c33, texf.x);

   return Lerp4(c0, c1, c2, c3, texf.y);
#else // optimized
   inTex*=ColSize.zw;
   Vec2 tc=Floor(inTex-0.5f)+0.5f,
        f=inTex-tc, f2=f*f, f3=f2*f,
        w0=f2-0.5f*(f3+f), w1=1.5f*f3-2.5f*f2+1.0f,
   #if 0
        w2=-1.5f*f3+2*f2+0.5f*f, w3=0.5f*(f3-f2);
   #else
        w3=0.5f*(f3-f2), w2=1.0f-w0-w1-w3;
   #endif

   tc*=ColSize.xy;
   Vec2 tc0=tc-ColSize.xy, tc3=tc+ColSize.xy*2;
#if 0 // 16 tex reads
   Vec2 tc2=tc+ColSize.xy;

 /*Flt w[4][4]={(w0.x*w0.y), (w1.x*w0.y), (w2.x*w0.y), (w3.x*w0.y),
                (w0.x*w1.y), (w1.x*w1.y), (w2.x*w1.y), (w3.x*w1.y),
                (w0.x*w2.y), (w1.x*w2.y), (w2.x*w2.y), (w3.x*w2.y),
                (w0.x*w3.y), (w1.x*w3.y), (w2.x*w3.y), (w3.x*w3.y)};*/

   return TexPoint(Col, Vec2(tc0.x, tc0.y))*(w0.x*w0.y)
         +TexPoint(Col, Vec2(tc .x, tc0.y))*(w1.x*w0.y)
         +TexPoint(Col, Vec2(tc0.x, tc .y))*(w0.x*w1.y)
         +TexPoint(Col, Vec2(tc .x, tc .y))*(w1.x*w1.y)

         +TexPoint(Col, Vec2(tc2.x, tc0.y))*(w2.x*w0.y)
         +TexPoint(Col, Vec2(tc3.x, tc0.y))*(w3.x*w0.y)
         +TexPoint(Col, Vec2(tc2.x, tc .y))*(w2.x*w1.y)
         +TexPoint(Col, Vec2(tc3.x, tc .y))*(w3.x*w1.y)
 
         +TexPoint(Col, Vec2(tc0.x, tc2.y))*(w0.x*w2.y)
         +TexPoint(Col, Vec2(tc .x, tc2.y))*(w1.x*w2.y)
         +TexPoint(Col, Vec2(tc0.x, tc3.y))*(w0.x*w3.y)
         +TexPoint(Col, Vec2(tc .x, tc3.y))*(w1.x*w3.y)

         +TexPoint(Col, Vec2(tc2.x, tc2.y))*(w2.x*w2.y)
         +TexPoint(Col, Vec2(tc3.x, tc2.y))*(w3.x*w2.y)
         +TexPoint(Col, Vec2(tc2.x, tc3.y))*(w2.x*w3.y)
         +TexPoint(Col, Vec2(tc3.x, tc3.y))*(w3.x*w3.y);
#else // 5 tex reads, corners are ignored because they're insignificant, 
   Vec2 w12=w1+w2, p=tc+(w2/w12)*ColSize.xy;
   Flt  wu=w12.x*w0.y, wd=w12.x*w3.y, wl=w12.y*w0.x, wr=w12.y*w3.x, wc=w12.x*w12.y;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return(Tex(Col, Vec2(  p.x, tc0.y))*wu // sample upper edge (2 texels), both weights are negative
         +Tex(Col, Vec2(  p.x, tc3.y))*wd // sample lower edge (2 texels), both weights are negative
         +Tex(Col, Vec2(tc0.x,   p.y))*wl // sample left  edge (2 texels), both weights are negative
         +Tex(Col, Vec2(tc3.x,   p.y))*wr // sample right edge (2 texels), both weights are negative
         +Tex(Col, Vec2(  p.x,   p.y))*wc // sample center     (4 texels), all  weights are positive
         )/(wu+wd+wl+wr+wc);
#endif
#endif
}
VecH TexCubicFastRGB(Vec2 inTex) // ignores alpha channel
{
   inTex*=ColSize.zw;
   Vec2 tc=Floor(inTex-0.5f)+0.5f,
        f=inTex-tc, f2=f*f, f3=f2*f,
        w0=f2-0.5f*(f3+f), w1=1.5f*f3-2.5f*f2+1.0f,
        w3=0.5f*(f3-f2), w2=1.0f-w0-w1-w3;
   tc*=ColSize.xy;
   Vec2 tc0=tc-ColSize.xy, tc3=tc+ColSize.xy*2,
        w12=w1  +w2      , p  =tc+(w2/w12)*ColSize.xy;
   Flt  wu =w12.x*w0.y   , wd =w12.x*w3.y, wl=w12.y*w0.x, wr=w12.y*w3.x, wc=w12.x*w12.y;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return(Tex(Col, Vec2(  p.x, tc0.y)).rgb*wu
         +Tex(Col, Vec2(  p.x, tc3.y)).rgb*wd
         +Tex(Col, Vec2(tc0.x,   p.y)).rgb*wl
         +Tex(Col, Vec2(tc3.x,   p.y)).rgb*wr
         +Tex(Col, Vec2(  p.x,   p.y)).rgb*wc)/(wu+wd+wl+wr+wc);
}
/******************************************************************************/
#define CUBIC_SAMPLES     3
#define CUBIC_RANGE       2
#define CUBIC_SHARPNESS   (2/2.5f)
#define CUBIC_QUALITY     2 // 0..2, 0=3.748 fps, 1=3.242 fps, 2=3.075 fps (however when using CUBIC_SKIP_SAMPLE it's now much faster)
#if MODEL!=SM_GL
   #define CUBIC_SKIP_SAMPLE (1 && CUBIC_QUALITY==2) // because the actual range is 2.5 then it means we need to process 5x5 samples (and not 6x6), this optimization can work only if actual range <= 2.5, also we can enable this only for CUBIC_QUALITY==2 because only this mode operates on single 1x1 pixels and not 2x2 blocks)
#else
   #define CUBIC_SKIP_SAMPLE 1 // otherwise fails to compile
#endif

inline Flt Cubic(Flt x, uniform Flt blur, uniform Flt sharpen)
{
   Flt x2=x*x,
       x3=x*x*x;
   return (x<=1) ? ((12-9*blur-6*sharpen)/6*x3 + (-18+12*blur+6*sharpen)/6*x2 +                             (6-2*blur         )/6)
                 : ((-blur-6*sharpen    )/6*x3 + (6*blur+30*sharpen    )/6*x2 + (-12*blur-48*sharpen)/6*x + (8*blur+24*sharpen)/6);
}
Flt CubicMed(Flt x) {return Cubic(x, 0.0f, 0.375f);}

VecH4 TexLerp(Flt x0, Flt x1, Flt y, Flt l, Flt r)
{
   Flt w=l+r;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return Tex(Col, Vec2((x0*l + x1*r)/w, y))*Half(w);
}
VecH TexLerpRGB(Flt x0, Flt x1, Flt y, Flt l, Flt r) // ignores alpha channel
{
   Flt w=l+r;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return Tex(Col, Vec2((x0*l + x1*r)/w, y)).rgb*Half(w);
}

VecH4 TexLerp(Vec2 t0, Vec2 t1, Flt lu, Flt ru, Flt lb, Flt rb)
{
#if 0 // slower
   return TexPoint(Col, Vec2(t0.x, t0.y))*lu
         +TexPoint(Col, Vec2(t1.x, t0.y))*ru
         +TexPoint(Col, Vec2(t0.x, t1.y))*lb
         +TexPoint(Col, Vec2(t1.x, t1.y))*rb;
#else
   Vec2 t=Vec2(t0.x, t0.y)*lu
         +Vec2(t1.x, t0.y)*ru
         +Vec2(t0.x, t1.y)*lb
         +Vec2(t1.x, t1.y)*rb;
   Flt w=lu+ru+lb+rb;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return Tex(Col, t/w)*Half(w);
#endif
}
VecH TexLerpRGB(Vec2 t0, Vec2 t1, Flt lu, Flt ru, Flt lb, Flt rb) // ignores alpha channel
{
   Vec2 t=Vec2(t0.x, t0.y)*lu
         +Vec2(t1.x, t0.y)*ru
         +Vec2(t0.x, t1.y)*lb
         +Vec2(t1.x, t1.y)*rb;
   Flt w=lu+ru+lb+rb;
   // keep 'Tex' in case we need LOD's (for example stretching in 1 dimension but shrinking in another)
   return Tex(Col, t/w).rgb*Half(w);
}

VecH4 TexCubic(Vec2 inTex)
{
   Vec2 pixel =inTex*ColSize.zw-0.5f,
        pixeli=Floor(pixel),
        offset       [CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE],
        offset_weight[CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE];

#if CUBIC_SKIP_SAMPLE
   pixeli+=(pixel-pixeli>=CUBIC_RANGE/CUBIC_SHARPNESS-(CUBIC_SAMPLES-1)); // if the left/top coordinate is completely out of range, then process the next pixel (the >= returns Vec2, so it modifies X and Y independently)
#endif

   UNROLL for(int i=0; i<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; i++)
   {
      offset       [i]=     pixeli+(i-(CUBIC_SAMPLES-1));
      offset_weight[i]=Sqr((pixel -offset[i])*CUBIC_SHARPNESS);
      offset       [i]=           (offset[i]+0.5f)*ColSize.xy;
   }
   Vec4 color =0;
   Flt  weight=0;
#if CUBIC_QUALITY>=2
   UNROLL for(int y=0; y<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; y++)
   UNROLL for(int x=0; x<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; x++)
   {
      Flt w=offset_weight[x].x+offset_weight[y].y; if(w<Sqr(CUBIC_RANGE))
      {
         w=CubicMed(Sqrt(w));
         color +=w*TexPoint(Col, Vec2(offset[x].x, offset[y].y)); // don't use "color+=w*Col.Load(VecI(offset[x].x without scale, offset[y].y without scale, 0)); because it is slower and doesn't support wrap/clamp
         weight+=w;
      }
   }
#else
   Flt weights[CUBIC_SAMPLES*2][CUBIC_SAMPLES*2]; // [y][x]
   UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y++)
   UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x++)
   {
      Flt w=offset_weight[x].x+offset_weight[y].y;
          w=(w<Sqr(CUBIC_RANGE) ? CubicMed(Sqrt(w)) : 0);
      weights[y][x]=w;
      weight      +=w;
   }
   #if CUBIC_QUALITY>=1 // medium quality
      UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y++ )
      UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x+=2)color+=TexLerp(offset[x].x, offset[x+1].x, offset[y].y, weights[y][x], weights[y][x+1]);
   #else // low quality
      UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y+=2)
      UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x+=2)color+=TexLerp(Vec2(offset[x].x, offset[y].y), Vec2(offset[x+1].x, offset[y+1].y), weights[y][x], weights[y][x+1], weights[y+1][x], weights[y+1][x+1]);
   #endif
#endif
   return color/weight;
}
VecH TexCubicRGB(Vec2 inTex) // ignores alpha channel
{
   Vec2 pixel =inTex*ColSize.zw-0.5f,
        pixeli=Floor(pixel),
        offset       [CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE],
        offset_weight[CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE];

#if CUBIC_SKIP_SAMPLE
   pixeli+=(pixel-pixeli>=CUBIC_RANGE/CUBIC_SHARPNESS-(CUBIC_SAMPLES-1)); // if the left/top coordinate is completely out of range, then process the next pixel (the >= returns Vec2, so it modifies X and Y independently)
#endif

   UNROLL for(int i=0; i<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; i++)
   {
      offset       [i]=     pixeli+(i-(CUBIC_SAMPLES-1));
      offset_weight[i]=Sqr((pixel -offset[i])*CUBIC_SHARPNESS);
      offset       [i]=           (offset[i]+0.5f)*ColSize.xy;
   }
   Vec color =0;
   Flt weight=0;
#if CUBIC_QUALITY>=2 // high quality
   UNROLL for(int y=0; y<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; y++)
   UNROLL for(int x=0; x<CUBIC_SAMPLES*2-CUBIC_SKIP_SAMPLE; x++)
   {
      Flt w=offset_weight[x].x+offset_weight[y].y; if(w<Sqr(CUBIC_RANGE))
      {
         w=CubicMed(Sqrt(w));
         color +=w*TexPoint(Col, Vec2(offset[x].x, offset[y].y)).rgb; // don't use "color+=w*Col.Load(VecI(offset[x].x without scale, offset[y].y without scale, 0)).rgb; because it is slower and doesn't support wrap/clamp
         weight+=w;
      }
   }
#else
   Flt weights[CUBIC_SAMPLES*2][CUBIC_SAMPLES*2]; // [y][x]
   UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y++)
   UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x++)
   {
      Flt w=offset_weight[x].x+offset_weight[y].y;
          w=(w<Sqr(CUBIC_RANGE) ? CubicMed(Sqrt(w)) : 0);
      weights[y][x]=w;
      weight      +=w;
   }
   #if CUBIC_QUALITY>=1 // medium quality
      UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y++ )
      UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x+=2)color+=TexLerpRGB(offset[x].x, offset[x+1].x, offset[y].y, weights[y][x], weights[y][x+1]);
   #else // low quality
      UNROLL for(int y=0; y<CUBIC_SAMPLES*2; y+=2)
      UNROLL for(int x=0; x<CUBIC_SAMPLES*2; x+=2)color+=TexLerpRGB(Vec2(offset[x].x, offset[y].y), Vec2(offset[x+1].x, offset[y+1].y), weights[y][x], weights[y][x+1], weights[y+1][x], weights[y+1][x+1]);
   #endif
#endif
   return color/weight;
}
/******************************************************************************/
// SHADERS
/******************************************************************************/
Vec4 Draw2DFlat_VS(VtxInput vtx):POSITION {return Vec4(vtx.pos2()*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);}
Vec4 Draw3DFlat_VS(VtxInput vtx):POSITION {return Project(TransformPos(vtx.pos()));}

Vec4 DrawFlat_PS():COLOR {return Color[0];}

TECHNIQUE(Draw2DFlat, Draw2DFlat_VS(), DrawFlat_PS());
TECHNIQUE(Draw3DFlat, Draw3DFlat_VS(), DrawFlat_PS());
#if !DX11 // THERE IS A BUG ON NVIDIA GEFORCE DX10+ when trying to clear normal render target using SetCol "Bool clear_nrm=(_nrm && !NRM_CLEAR_START && ClearNrm());", with D.depth2DOn(true) entire RT is cleared instead of background pixels only, this was verified on Windows 10 GeForce 650m, drivers 381, TODO: check again in the future
TECHNIQUE(SetCol    , Draw_VS      (), DrawFlat_PS());
#else // this version works OK
void SetCol_VS(VtxInput vtx,
           out Vec4 outCol:COLOR   ,
           out Vec4 outVtx:POSITION)
{
   outCol=Color[0];
   outVtx=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
}
Vec4 SetCol_PS(NOPERSP Vec4 inCol:COLOR):COLOR {return inCol;}
TECHNIQUE(SetCol, SetCol_VS(), SetCol_PS());
#endif
/******************************************************************************/
void Draw2DCol_VS(VtxInput vtx,
              out VecH4 outCol:COLOR   ,
              out Vec4  outVtx:POSITION)
{
   outCol=     vtx.color();
   outVtx=Vec4(vtx.pos  ().xy*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);
}
Vec4 Draw2DCol_PS(NOPERSP Vec4 inCol:COLOR):COLOR {return inCol;}

TECHNIQUE(Draw2DCol, Draw2DCol_VS(), Draw2DCol_PS());
/******************************************************************************/
void Draw3DCol_VS(VtxInput vtx,
              out VecH4 outCol:COLOR   ,
              out Vec4  outVtx:POSITION)
{
   outCol=vtx.color();
   outVtx=Project(TransformPos(vtx.pos()));
}
Vec4 Draw3DCol_PS(Vec4 inCol:COLOR):COLOR {return inCol;}

TECHNIQUE(Draw3DCol, Draw3DCol_VS(), Draw3DCol_PS());
/******************************************************************************/
void Draw2DTex_VS(VtxInput vtx,
              out Vec2 outTex:TEXCOORD,
              out Vec4 outVtx:POSITION)
{
   outTex=vtx.tex();
   outVtx=Vec4(vtx.pos2()*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);
}
Vec4 Draw2DTex_PS (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return      Tex(Col, inTex);}
Vec4 Draw2DTexC_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR {return      Tex(Col, inTex)*Color[0]+Color[1];}
Vec4 Draw2DTexA_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR {return Vec4(Tex(Col, inTex).rgb, Step);}
Vec4 DrawTexX_PS  (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return Vec4(Tex(Col, inTex).xxx, 1);}
Vec4 DrawTexY_PS  (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return Vec4(Tex(Col, inTex).yyy, 1);}
Vec4 DrawTexZ_PS  (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return Vec4(Tex(Col, inTex).zzz, 1);}
Vec4 DrawTexW_PS  (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return Vec4(Tex(Col, inTex).www, 1);}
Vec4 DrawTexNrm_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec nrm; nrm.xy=Tex(Col, inTex).xy*2-1; // #MaterialTextureChannelOrder
            nrm.z =CalcZ(nrm.xy);
            nrm   =Normalize(nrm)*0.5f+0.5f;
   return Vec4(nrm, 1);
}

TECHNIQUE(Draw2DTex , Draw2DTex_VS(),  Draw2DTex_PS());
TECHNIQUE(Draw2DTexC, Draw2DTex_VS(), Draw2DTexC_PS());
TECHNIQUE(DrawTexX  , Draw2DTex_VS(),   DrawTexX_PS());
TECHNIQUE(DrawTexY  , Draw2DTex_VS(),   DrawTexY_PS());
TECHNIQUE(DrawTexZ  , Draw2DTex_VS(),   DrawTexZ_PS());
TECHNIQUE(DrawTexW  , Draw2DTex_VS(),   DrawTexW_PS());
TECHNIQUE(DrawTexNrm, Draw2DTex_VS(), DrawTexNrm_PS());
TECHNIQUE(Draw      ,      Draw_VS(),  Draw2DTex_PS());
TECHNIQUE(DrawC     ,      Draw_VS(), Draw2DTexC_PS());
TECHNIQUE(DrawA     ,      Draw_VS(), Draw2DTexA_PS());

Vec4 DrawTexPoint_PS (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return TexPoint(Col, inTex);}
Vec4 DrawTexPointC_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR {return TexPoint(Col, inTex)*Color[0]+Color[1];}
TECHNIQUE(DrawTexPoint , Draw2DTex_VS(), DrawTexPoint_PS ());
TECHNIQUE(DrawTexPointC, Draw2DTex_VS(), DrawTexPointC_PS());
/******************************************************************************/
void Draw2DTexCol_VS(VtxInput vtx,
                 out Vec2  outTex:TEXCOORD,
                 out VecH4 outCol:COLOR   ,
                 out Vec4  outVtx:POSITION)
{
   outTex=     vtx.tex  ();
   outCol=     vtx.color();
   outVtx=Vec4(vtx.pos  ().xy*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);
}
Vec4 Draw2DTexCol_PS(NOPERSP Vec2  inTex:TEXCOORD,
                     NOPERSP VecH4 inCol:COLOR   ):COLOR
{
   VecH4  col=Tex(Col, inTex);
   return col*inCol;
}
TECHNIQUE(Draw2DTexCol, Draw2DTexCol_VS(), Draw2DTexCol_PS());
/******************************************************************************/
void Draw3DTex_VS(VtxInput vtx,
              out Vec2  outTex:TEXCOORD,
              out VecH4 outFog:COLOR   ,
              out Vec4  outVtx:POSITION,
          uniform Bool  fog=false)
{
   Vec pos=TransformPos(vtx.pos());
          outTex=vtx.tex();
   if(fog)outFog=Vec4(FogColor(), AccumulatedDensity(FogDensity(), Length(pos)));
          outVtx=Project(pos);
}
Vec4 Draw3DTex_PS(Vec2  inTex:TEXCOORD,
                  VecH4 inFog:COLOR   ,
          uniform Bool  alpha_test    ,
          uniform Bool  fog           ):COLOR
{
   VecH4 col=Tex(Col, inTex);
   if(alpha_test)clip(col.a-0.5f);
   if(fog)col.rgb=Lerp(col.rgb, inFog.rgb, inFog.a);
   return col;
}
TECHNIQUE(Draw3DTex   , Draw3DTex_VS(false), Draw3DTex_PS(false, false));
TECHNIQUE(Draw3DTexAT , Draw3DTex_VS(false), Draw3DTex_PS(true , false));
TECHNIQUE(Draw3DTexF  , Draw3DTex_VS(true ), Draw3DTex_PS(false, true ));
TECHNIQUE(Draw3DTexATF, Draw3DTex_VS(true ), Draw3DTex_PS(true , true ));
/******************************************************************************/
void Draw3DTexCol_VS(VtxInput vtx,
                 out Vec2  outTex:TEXCOORD,
                 out VecH4 outCol:COLOR   ,
                 out VecH4 outFog:COLOR1  ,
                 out Vec4  outVtx:POSITION,
             uniform Bool  fog=false)
{
   Vec pos=TransformPos(vtx.pos());
          outTex=vtx.tex  ();
          outCol=vtx.color();
   if(fog)outFog=Vec4(FogColor(), AccumulatedDensity(FogDensity(), Length(pos)));
          outVtx=Project(pos);
}
Vec4 Draw3DTexCol_PS(Vec2  inTex:TEXCOORD,
                     VecH4 inCol:COLOR   ,
                     VecH4 inFog:COLOR1  ,
             uniform Bool  alpha_test    ,
             uniform Bool  fog           ):COLOR
{
   VecH4 col=Tex(Col, inTex);
   if(alpha_test)clip(col.a-0.5f);
   col*=inCol;
   if(fog)col.rgb=Lerp(col.rgb, inFog.rgb, inFog.a);
   return col;
}
TECHNIQUE(Draw3DTexCol   , Draw3DTexCol_VS(false), Draw3DTexCol_PS(false, false));
TECHNIQUE(Draw3DTexColAT , Draw3DTexCol_VS(false), Draw3DTexCol_PS(true , false));
TECHNIQUE(Draw3DTexColF  , Draw3DTexCol_VS(true ), Draw3DTexCol_PS(false, true ));
TECHNIQUE(Draw3DTexColATF, Draw3DTexCol_VS(true ), Draw3DTexCol_PS(true , true ));
/******************************************************************************/
Vec4 DrawTexXC_PS(NOPERSP Vec2 inTex:TEXCOORD,
                  NOPERSP PIXEL,
          uniform Bool dither=false):COLOR
{
   VecH4 col=Tex(Col, inTex).x*Color[0]+Color[1];
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
Vec4 DrawTexWC_PS(NOPERSP Vec2 inTex:TEXCOORD,
                  NOPERSP PIXEL,
          uniform Bool dither=false):COLOR
{
   VecH4 col=Tex(Col, inTex).w*Color[0]+Color[1];
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
TECHNIQUE(DrawTexXC , Draw_VS(), DrawTexXC_PS());
TECHNIQUE(DrawTexWC , Draw_VS(), DrawTexWC_PS());
TECHNIQUE(DrawTexXCD, Draw_VS(), DrawTexXC_PS(true));
TECHNIQUE(DrawTexWCD, Draw_VS(), DrawTexWC_PS(true));
/******************************************************************************/
Vec4 DrawTexCubicFast_PS(NOPERSP Vec2 inTex:TEXCOORD,
                         NOPERSP PIXEL,
                 uniform Bool color,
                 uniform Bool dither):COLOR
{
   VecH4 col=TexCubicFast(inTex);
   if(color)col=col*Color[0]+Color[1];
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
Vec4 DrawTexCubicFastRGB_PS(NOPERSP Vec2 inTex:TEXCOORD,
                            NOPERSP PIXEL,
                    uniform Bool dither=false):COLOR
{
   VecH col=TexCubicFastRGB(inTex);
   if(dither)col.rgb+=DitherValueColor(pixel);
   return Vec4(col, 1);
}
TECHNIQUE(DrawTexCubicFast    , Draw2DTex_VS(), DrawTexCubicFast_PS(false, false));
TECHNIQUE(DrawTexCubicFastC   , Draw2DTex_VS(), DrawTexCubicFast_PS(true , false));
TECHNIQUE(DrawTexCubicFast1   ,      Draw_VS(), DrawTexCubicFast_PS(false, false));
TECHNIQUE(DrawTexCubicFastD   ,      Draw_VS(), DrawTexCubicFast_PS(false, true ));
TECHNIQUE(DrawTexCubicFastRGB ,      Draw_VS(), DrawTexCubicFastRGB_PS());
TECHNIQUE(DrawTexCubicFastRGBD,      Draw_VS(), DrawTexCubicFastRGB_PS(true));
/******************************************************************************/
Vec4 DrawTexCubic_PS(NOPERSP Vec2 inTex:TEXCOORD,
                     NOPERSP PIXEL              ,
                     uniform Bool color         ,
                     uniform Bool dither        ):COLOR
{
   VecH4 col=TexCubic(inTex);
   if(color)col=col*Color[0]+Color[1];
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
Vec4 DrawTexCubicRGB_PS(NOPERSP Vec2 inTex:TEXCOORD,
                        NOPERSP PIXEL              ,
                        uniform Bool dither=false  ):COLOR
{
   VecH col=TexCubicRGB(inTex);
   if(dither)col.rgb+=DitherValueColor(pixel);
   return Vec4(col, 1);
}
TECHNIQUE(DrawTexCubic    , Draw2DTex_VS(), DrawTexCubic_PS(false, false));
TECHNIQUE(DrawTexCubicC   , Draw2DTex_VS(), DrawTexCubic_PS(true , false));
TECHNIQUE(DrawTexCubic1   ,      Draw_VS(), DrawTexCubic_PS(false, false));
TECHNIQUE(DrawTexCubicD   ,      Draw_VS(), DrawTexCubic_PS(false, true ));
TECHNIQUE(DrawTexCubicRGB ,      Draw_VS(), DrawTexCubicRGB_PS());
TECHNIQUE(DrawTexCubicRGBD,      Draw_VS(), DrawTexCubicRGB_PS(true));
/******************************************************************************/
#if MODEL>=SM_4
Vec4 DrawMs1_PS(NOPERSP PIXEL):COLOR {return TexSample(ColMS, pixel.xy, 0);}
Vec4 DrawMsN_PS(NOPERSP PIXEL):COLOR
{
                                    Vec4 color =TexSample(ColMS, pixel.xy, 0);
   UNROLL for(Int i=1; i<MS_SAMPLES; i++)color+=TexSample(ColMS, pixel.xy, i);
   return color/MS_SAMPLES;
}
Vec4 DrawMsM_PS(NOPERSP PIXEL,
                   UInt index:SV_SampleIndex):COLOR
{
   return TexSample(ColMS, pixel.xy, index);
}
TECHNIQUE    (DrawMs1, DrawPixel_VS(), DrawMs1_PS());
TECHNIQUE    (DrawMsN, DrawPixel_VS(), DrawMsN_PS());
TECHNIQUE_4_1(DrawMsM, DrawPixel_VS(), DrawMsM_PS());
#endif
/******************************************************************************/
void DrawMask_VS(VtxInput vtx,
             out Vec2 outTexC:TEXCOORD0,
             out Vec2 outTexM:TEXCOORD1,
             out Vec4 outVtx :POSITION )
{
   outTexC=vtx.tex ();
   outTexM=vtx.tex1();
   outVtx =Vec4(vtx.pos2()*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);
}
Vec4 DrawMask_PS(NOPERSP Vec2 inTexC:TEXCOORD0,
                 NOPERSP Vec2 inTexM:TEXCOORD1):COLOR
{
   VecH4  col   =Tex(Col , inTexC)*Color[0]+Color[1];
          col.a*=Tex(Col1, inTexM).a;
   return col;
}
TECHNIQUE(DrawMask, DrawMask_VS(), DrawMask_PS());
/******************************************************************************/
void DrawCubeFace_VS(VtxInput vtx,
                 out Vec  outTex:TEXCOORD,
                 out Vec4 outVtx:POSITION)
{
   outTex=Vec (vtx.tex(), vtx.size());
   outVtx=Vec4(vtx.pos2()*Coords.xy+Coords.zw, REVERSE_DEPTH, 1);
}
Vec4 DrawCubeFace_PS(NOPERSP Vec inTex:TEXCOORD):COLOR {return TexCube(Rfl, inTex)*Color[0]+Color[1];}

TECHNIQUE(DrawCubeFace, DrawCubeFace_VS(), DrawCubeFace_PS());
/******************************************************************************/
BUFFER(Font)
   Flt FontShadow,
       FontContrast=1,
       FontShade,
       FontDepth;
BUFFER_END

void Font_VS(VtxInput vtx,
         out Vec2 outTex  :TEXCOORD0,
         out Flt  outShade:TEXCOORD1,
         out Vec4 outVtx  :POSITION ,
     uniform Bool custom_depth)
{
                   outTex  =     vtx.tex ();
                   outShade=     vtx.size();
   if(custom_depth)outVtx  =Vec4(vtx.pos ().xy*Coords.xy+Coords.zw, DelinearizeDepth(FontDepth), 1);
   else            outVtx  =Vec4(vtx.pos ().xy*Coords.xy+Coords.zw,               REVERSE_DEPTH, 1);
}
Vec4 Font_PS
(
   NOPERSP Vec2 inTex  :TEXCOORD0,
   NOPERSP Flt  inShade:TEXCOORD1
):COLOR
{
   // c=color, s=shadow, a=alpha

   // final=dest *(1-s) + 0*s; -> background after applying shadow
   // final=final*(1-a) + c*a; -> background after applying shadow after applying color

   // final=(dest*(1-s) + 0*s)*(1-a) + c*a;

   // final=dest*(1-s)*(1-a) + c*a;

#if DX11
   Vec2 as=Col.Sample(SamplerFont, inTex).ga;
#else
   Vec2 as=Tex(Col, inTex).ga;
#endif
   Flt  a =Sat(as.x*FontContrast), // font opacity, "Min(as.x*FontContrast, 1)", scale up by 'FontContrast' to improve quality when font is very small
        s =    as.y*FontShadow   ; // font shadow

   // Flt final_alpha=1-(1-s)*(1-a);
   // 1-(1-s)*(1-a)
   // 1-(1-a-s+sa)
   // 1-1+a+s-sa
   // a + s - s*a
   Flt final_alpha=a+s-s*a;

#if 1 // use for ALPHA_BLEND (this option is better because we don't need to set blend state specificaly for drawing fonts)
   return Vec4(Color[0].rgb*(Lerp(FontShade, 1, Sat(inShade))*a/(final_alpha+EPS)), Color[0].a*final_alpha); // NaN, division by 'final_alpha' is required because of the hardware ALPHA_BLEND formula, without it we would get dark borders around the font
#else // use for ALPHA_MERGE
   return Vec4(Color[0].rgb*(Lerp(FontShade, 1, Sat(inShade))*a*Color[0].a), Color[0].a*final_alpha);
#endif
}
TECHNIQUE(Font , Font_VS(false), Font_PS());
TECHNIQUE(FontD, Font_VS(true ), Font_PS());
/******************************************************************************/
void Laser_VS(VtxInput vtx,
          out Vec  outPos:TEXCOORD0,
          out Vec  outNrm:TEXCOORD1,
          out Vec4 outVtx:POSITION ,
      uniform Bool normals         )
{
       if(normals)outNrm=TransformDir(vtx.nrm());
   outVtx=Project(outPos=TransformPos(vtx.pos()));
}
void Laser_PS(Vec                 inPos:TEXCOORD0,
              Vec                 inNrm:TEXCOORD1,
          out DeferredSolidOutput output         ,
      uniform Bool                normals        )
{
   if(normals)
   {
         inNrm=Normalize(inNrm);
      Flt  stp=Max (-Dot(inNrm, Normalize(inPos)), -inNrm.z);
           stp=Sat (stp);
           stp=Pow (stp, Half(Step));
      Vec4 col=Lerp(Color[0], Color[1], stp);
      output.color(col.rgb);
      output.glow (col.a  );
   }else
   {
      output.color(Color[0].rgb);
      output.glow (Color[0].a  );
   }
   output.normal  (0);
   output.specular(0);
   output.velocity(0, inPos);
}
TECHNIQUE(Laser , Laser_VS(false), Laser_PS(false));
TECHNIQUE(LaserN, Laser_VS(true ), Laser_PS(true ));
/******************************************************************************/
void Simple_VS(VtxInput vtx,
           out Vec2  outTex:TEXCOORD,
           out VecH4 outCol:COLOR   ,
           out Vec4  outVtx:POSITION)
{
   outTex=vtx.tex();
   outCol=vtx.color();
   outVtx=Project(TransformPos(vtx.pos()));
}
Vec4 Simple_PS(Vec2  inTex:TEXCOORD,
               VecH4 inCol:COLOR   ):COLOR
{
   return Tex(Col, inTex)*inCol;
}
TECHNIQUE(Simple, Simple_VS(), Simple_PS());
/******************************************************************************/
BUFFER(LocalFog)
   Vec4 LocalFogColor_Density; // rgb=color, a=density
   Vec  LocalFogInside;
BUFFER_END

inline Vec LocalFogColor  () {return LocalFogColor_Density.rgb;}
inline Flt LocalFogDensity() {return LocalFogColor_Density.a  ;}
/******************************************************************************/
// TODO: optimize fog shaders
void FogBox_VS(VtxInput vtx,
           out Vec4    outVtx :POSITION ,
           out Vec     outPos :TEXCOORD0,
           out Vec     outTex :TEXCOORD1,
           out Vec4    outSize:TEXCOORD2,
           out Matrix3 outMat :TEXCOORD3)
{
   outMat[0]=Normalize(MatrixX(ViewMatrix[0])); outSize.x=Length(MatrixX(ViewMatrix[0]));
   outMat[1]=Normalize(MatrixY(ViewMatrix[0])); outSize.y=Length(MatrixY(ViewMatrix[0]));
   outMat[2]=Normalize(MatrixZ(ViewMatrix[0])); outSize.z=Length(MatrixZ(ViewMatrix[0]));
                                                outSize.w=Max(outSize.xyz);

   // convert to texture space (0..1)
   outTex=vtx.pos()*0.5f+0.5f;
   outVtx=Project(outPos=TransformPos(vtx.pos()));
}
void FogBox_PS
(
   PIXEL,

   Vec     inPos :TEXCOORD0,
   Vec     inTex :TEXCOORD1,
   Vec4    inSize:TEXCOORD2,
   Matrix3 inMat :TEXCOORD3,

   out Vec4 color:COLOR0,
   out Vec4 alpha:COLOR1,

   uniform Bool height
)
{
   Flt z  =TexDepthPoint(PIXEL_TO_SCREEN);
   Vec pos=inTex,
       dir=Normalize(inPos); dir*=Min((SQRT3*2)*inSize.w, (z-inPos.z)/dir.z);
       dir=mul(inMat, dir); // convert to box space

   // convert to texture space (0..1)
   dir=dir/(2*inSize.xyz);

   Vec end=pos+dir;

   if(end.x<0)end+=(0-end.x)/dir.x*dir;
   if(end.x>1)end+=(1-end.x)/dir.x*dir;
   if(end.y<0)end+=(0-end.y)/dir.y*dir;
   if(end.y>1)end+=(1-end.y)/dir.y*dir;
   if(end.z<0)end+=(0-end.z)/dir.z*dir;
   if(end.z>1)end+=(1-end.z)/dir.z*dir;

       dir =end-pos;
       dir*=inSize.xyz;
   Flt len =Length(dir)/inSize.w;

   Flt dns=LocalFogDensity();
   if(height){dns*=1-Avg(pos.y, end.y); len*=3;}

   color.rgb=LocalFogColor();
   color.a  =AccumulatedDensity(dns, len);
   alpha    =color.a;
}

void FogBoxI_VS(VtxInput vtx,
            out Vec2    outTex  :TEXCOORD0,
            out Vec2    outPosXY:TEXCOORD1,
            out Vec4    outSize :TEXCOORD2,
            out Matrix3 outMat  :TEXCOORD3,
            out Vec4    outVtx  :POSITION )
{
   outMat[0]=Normalize(MatrixX(ViewMatrix[0])); outSize.x=Length(MatrixX(ViewMatrix[0]));
   outMat[1]=Normalize(MatrixY(ViewMatrix[0])); outSize.y=Length(MatrixY(ViewMatrix[0]));
   outMat[2]=Normalize(MatrixZ(ViewMatrix[0])); outSize.z=Length(MatrixZ(ViewMatrix[0]));
                                                outSize.w=Max(outSize.xyz);

   outVtx  =Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   outTex  =vtx.tex();
   outPosXY=ScreenToPosXY(outTex);
}
void FogBoxI_PS
(
   NOPERSP Vec2    inTex  :TEXCOORD0,
   NOPERSP Vec2    inPosXY:TEXCOORD1,
   NOPERSP Vec4    inSize :TEXCOORD2,
   NOPERSP Matrix3 inMat  :TEXCOORD3,

   out Vec4 color:COLOR0,
   out Vec4 alpha:COLOR1,

   uniform Int  inside,
   uniform Bool height
)
{
   Vec pos=GetPosPoint(inTex, inPosXY),
       dir=Normalize(pos); dir*=Min((SQRT3*2)*inSize.w, (pos.z-Viewport.from)/dir.z);
       dir=mul(inMat, dir); // convert to box space

   // convert to texture space (0..1)
   pos=LocalFogInside/(2*inSize.xyz)+0.5f;
   dir=dir           /(2*inSize.xyz);

   if(inside==0)
   {
      if(pos.x<0)pos+=(0-pos.x)/dir.x*dir;
      if(pos.x>1)pos+=(1-pos.x)/dir.x*dir;
      if(pos.y<0)pos+=(0-pos.y)/dir.y*dir;
      if(pos.y>1)pos+=(1-pos.y)/dir.y*dir;
      if(pos.z<0)pos+=(0-pos.z)/dir.z*dir;
      if(pos.z>1)pos+=(1-pos.z)/dir.z*dir;
   }

   Vec end=pos+dir;

   if(end.x<0)end+=(0-end.x)/dir.x*dir;
   if(end.x>1)end+=(1-end.x)/dir.x*dir;
   if(end.y<0)end+=(0-end.y)/dir.y*dir;
   if(end.y>1)end+=(1-end.y)/dir.y*dir;
   if(end.z<0)end+=(0-end.z)/dir.z*dir;
   if(end.z>1)end+=(1-end.z)/dir.z*dir;

       dir =end-pos;
       dir*=inSize.xyz;
   Flt len =Length(dir)/inSize.w;

   Flt dns=LocalFogDensity();
   if(height){dns*=1-Avg(pos.y, end.y); len*=3;}

   color.rgb=LocalFogColor();
   color.a  =AccumulatedDensity(dns, len);
   alpha    =color.a;
}

void FogBall_VS(VtxInput vtx,
            out Vec4 outVtx :POSITION ,
            out Vec  outPos :TEXCOORD0,
            out Vec  outTex :TEXCOORD1,
            out Flt  outSize:TEXCOORD2)
{
   outTex =vtx.pos();
   outSize=Length(MatrixX(ViewMatrix[0]));
   outVtx =Project(outPos=TransformPos(vtx.pos()));
}
void FogBall_PS
(
   PIXEL,

   Vec inPos :TEXCOORD0,
   Vec inTex :TEXCOORD1,
   Flt inSize:TEXCOORD2,

   out Vec4 color:COLOR0,
   out Vec4 alpha:COLOR1
)
{
   Flt z  =TexDepthPoint(PIXEL_TO_SCREEN);
   Vec pos=Normalize    (inTex),
       dir=Normalize    (inPos); Flt max_length=(z-inPos.z)/(dir.z*inSize);
       dir=Transform3   (dir, CamMatrix); // convert to ball space

   // collision detection
   Vec p  =PointOnPlane(pos, dir);
   Flt s  =Length      (p       );
       s  =Sat         (1-s*s   );
   Vec end=p+Sqrt(s)*dir;

   Flt len=Min(Dist(pos, end), max_length);

   Flt dns=LocalFogDensity()*s;

   color.rgb=LocalFogColor();
   color.a  =AccumulatedDensity(dns, len);
   alpha    =color.a;
}

void FogBallI_VS(VtxInput vtx,
             out Vec2 outTex  :TEXCOORD0,
             out Vec2 outPosXY:TEXCOORD1,
             out Flt  outSize :TEXCOORD2,
             out Vec4 outVtx  :POSITION )
{
   outVtx  =Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   outTex  =vtx.tex();
   outPosXY=ScreenToPosXY(outTex);
   outSize =Length(MatrixX(ViewMatrix[0]));
}
void FogBallI_PS
(
   NOPERSP Vec2 inTex  :TEXCOORD0,
   NOPERSP Vec2 inPosXY:TEXCOORD1,
   NOPERSP Flt  inSize :TEXCOORD2,

   out Vec4 color:COLOR0,
   out Vec4 alpha:COLOR1,

   uniform Int inside
)
{
   Vec pos=GetPosPoint(inTex, inPosXY),
       dir=Normalize (pos); Flt max_length=(pos.z-Viewport.from)/(dir.z*inSize);
       dir=Transform3(dir, CamMatrix); // convert to ball space

   pos=LocalFogInside/inSize;

   // collision detection
   Vec p  =PointOnPlane(pos, dir);
   Flt s  =Length      (p       );
       s  =Sat         (1-s*s   );
   Vec end=p+dir*Sqrt(s);

   Flt len=Min(Dist(pos, end), max_length);

   Flt dns=LocalFogDensity()*s;

   color.rgb=LocalFogColor();
   color.a  =AccumulatedDensity(dns, len);
   alpha    =color.a;
}

TECHNIQUE(FogBox ,  FogBox_VS(),  FogBox_PS(   false));
TECHNIQUE(FogBox0, FogBoxI_VS(), FogBoxI_PS(0, false));
TECHNIQUE(FogBox1, FogBoxI_VS(), FogBoxI_PS(1, false));

TECHNIQUE(FogHgt ,  FogBox_VS(),  FogBox_PS(   true));
TECHNIQUE(FogHgt0, FogBoxI_VS(), FogBoxI_PS(0, true));
TECHNIQUE(FogHgt1, FogBoxI_VS(), FogBoxI_PS(1, true));

TECHNIQUE(FogBall ,  FogBall_VS(),  FogBall_PS( ));
TECHNIQUE(FogBall0, FogBallI_VS(), FogBallI_PS(0));
TECHNIQUE(FogBall1, FogBallI_VS(), FogBallI_PS(1));
/******************************************************************************/
struct VolumeClass
{
   Flt min_steps,
       max_steps,
       density_factor,
       precision;
   Vec size  ,
       pixels,
       inside;
};

BUFFER(Volume)
   VolumeClass Volume;
BUFFER_END

void Volume_VS(VtxInput vtx,
           out Vec4    outVtx:POSITION ,
           out Vec     outPos:TEXCOORD0,
           out Vec     outTex:TEXCOORD1,
           out Matrix3 outMat:TEXCOORD2,
       uniform Int     inside          )
{
   outMat[0]=Normalize(MatrixX(ViewMatrix[0]));
   outMat[1]=Normalize(MatrixY(ViewMatrix[0]));
   outMat[2]=Normalize(MatrixZ(ViewMatrix[0]));

   // convert to texture space (0..1)
   if(inside)outTex=Volume.inside/(2*Volume.size)+0.5f;
   else      outTex=vtx.pos()*0.5f+0.5f;

   outVtx=Project(outPos=TransformPos(vtx.pos()));
}
void Volume_PS
(
   PIXEL,

   Vec     inPos:TEXCOORD0,
   Vec     inTex:TEXCOORD1,
   Matrix3 inMat:TEXCOORD2,

   out Vec4 color:COLOR0,
   out Vec4 alpha:COLOR1,

   uniform Int  inside,
   uniform Bool LA=false
)
{
   Flt z  =TexDepthPoint(PIXEL_TO_SCREEN);
   Vec pos=inTex;
   Vec dir=Normalize(inPos); dir*=Min((SQRT3*2)*Max(Volume.size), (z-(inside ? Viewport.from : inPos.z))/dir.z);
       dir=mul(inMat, dir); // convert to box space

   // convert to texture space (0..1)
   dir=dir/(2*Volume.size);

   if(inside==1)
   {
      if(pos.x<0)pos+=(0-pos.x)/dir.x*dir;
      if(pos.x>1)pos+=(1-pos.x)/dir.x*dir;
      if(pos.y<0)pos+=(0-pos.y)/dir.y*dir;
      if(pos.y>1)pos+=(1-pos.y)/dir.y*dir;
      if(pos.z<0)pos+=(0-pos.z)/dir.z*dir;
      if(pos.z>1)pos+=(1-pos.z)/dir.z*dir;
   }

   Vec end=pos+dir;

   if(end.x<0)end+=(0-end.x)/dir.x*dir;
   if(end.x>1)end+=(1-end.x)/dir.x*dir;
   if(end.y<0)end+=(0-end.y)/dir.y*dir;
   if(end.y>1)end+=(1-end.y)/dir.y*dir;
   if(end.z<0)end+=(0-end.z)/dir.z*dir;
   if(end.z>1)end+=(1-end.z)/dir.z*dir;

       dir        =end-pos;
   Flt pixels     =Length(dir   *Volume.pixels);
   Int steps      =Mid   (pixels*Volume.precision, Volume.min_steps, Volume.max_steps);
   Flt steps_speed=       pixels/steps;

   Flt density_factor=Volume.density_factor;
       density_factor=1-Pow(1-density_factor, steps_speed); // modify by steps speed

   dir/=steps;

   if(LA)
   {
      Vec2 col=0;

      LOOP for(Int i=0; i<steps; i++)
      {
         Vec2 sample=Tex3DLod(Vol, pos).rg;
         Flt  alpha =sample.g*density_factor*(1-col.g);

         col.r+=alpha*sample.r;
         col.g+=alpha;

         pos+=dir;
      }

      col.r/=col.g+EPS; // NaN

      color=col.rrrg*Color[0]+Color[1];
      alpha=color.a;
   }else
   {
      Vec4 col=0;

      LOOP for(Int i=0; i<steps; i++)
      {
         Vec4 sample=Tex3DLod(Vol, pos);
         Flt  alpha =sample.a*density_factor*(1-col.a);

         col.rgb+=alpha*sample.rgb;
         col.a  +=alpha;

         pos+=dir;
      }

      col.rgb/=col.a+EPS; // NaN

      color=col*Color[0]+Color[1];
      alpha=color.a;
   }
}
TECHNIQUE(Volume0, Volume_VS(0), Volume_PS(0));
TECHNIQUE(Volume1, Volume_VS(1), Volume_PS(1));
TECHNIQUE(Volume2, Volume_VS(2), Volume_PS(2));

TECHNIQUE(Volume0LA, Volume_VS(0), Volume_PS(0, true));
TECHNIQUE(Volume1LA, Volume_VS(1), Volume_PS(1, true));
TECHNIQUE(Volume2LA, Volume_VS(2), Volume_PS(2, true));
/******************************************************************************/
inline VecH TexYUV(Vec2 inTex)
{
 /*Flt y=Tex(Col , inTex).x,
       u=Tex(Col1, inTex).x,
       v=Tex(Col2, inTex).x;

   Flt r=1.1643f*(y-0.0625f)                     + 1.5958f*(v-0.5f),
       g=1.1643f*(y-0.0625f) - 0.39173f*(u-0.5f) - 0.8129f*(v-0.5f),
       b=1.1643f*(y-0.0625f) + 2.017f  *(u-0.5f)                   ;*/

   // keep 'Tex' in case images have LOD's (however unlikely)
   Half y=Tex(Col , inTex).x*1.1643-0.07276875,
        u=Tex(Col1, inTex).x       -0.5,
        v=Tex(Col2, inTex).x       -0.5;

   return VecH(y             + 1.5958*v,
               y - 0.39173*u - 0.8129*v,
               y + 2.017  *u          );
}
VecH4 YUV_PS (NOPERSP Vec2 inTex:TEXCOORD):COLOR {return VecH4(TexYUV(inTex),                                    1);}
VecH4 YUVA_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR {return VecH4(TexYUV(inTex), Tex(Col3, inTex).x*1.1643-0.07276875);} // need to MulAdd because alpha image assumes to come from another YUV video

TECHNIQUE(YUV , Draw2DTex_VS(), YUV_PS ());
TECHNIQUE(YUVA, Draw2DTex_VS(), YUVA_PS());
/******************************************************************************/
// BLUR
/******************************************************************************/
#define WEIGHT4_0 0.250000000f
#define WEIGHT4_1 0.213388354f
#define WEIGHT4_2 0.124999993f
#define WEIGHT4_3 0.036611654f
// WEIGHT4_0 + WEIGHT4_1*2 + WEIGHT4_2*2 + WEIGHT4_3*2 = 1

#define WEIGHT5_0 0.200000014f
#define WEIGHT5_1 0.180901707f
#define WEIGHT5_2 0.130901704f
#define WEIGHT5_3 0.069098287f
#define WEIGHT5_4 0.019098295f
// WEIGHT5_0 + WEIGHT5_1*2 + WEIGHT5_2*2 + WEIGHT5_3*2 + WEIGHT5_4*2 = 1

#define WEIGHT6_0 0.166666668f
#define WEIGHT6_1 0.155502122f
#define WEIGHT6_2 0.125000001f
#define WEIGHT6_3 0.083333329f
#define WEIGHT6_4 0.041666662f
#define WEIGHT6_5 0.011164551f
// WEIGHT6_0 + WEIGHT6_1*2 + WEIGHT6_2*2 + WEIGHT6_3*2 + WEIGHT6_4*2 + WEIGHT6_5*2 = 1

// !!
// !! If changing number of samples then change also SHADER_BLUR_RANGE !!
// !!

#define TEST_BLUR 0
#if     TEST_BLUR
   Flt Test, Samples, Mode;
   Flt Weight(Int i, Int s)
   {
      return Mode ? Gaussian(Mode*i/Flt(s+1)) : BlendSmoothSin(i/Flt(s+1));
   }
#endif

Vec4 BlurX_PS(NOPERSP Vec2 inTex:TEXCOORD,
              uniform Int  samples       ):COLOR
{
#if TEST_BLUR
   if(Test){Int s=Round(Samples); Vec4 color=0; Flt weight=0; for(Int i=-s; i<=s; i++){Flt w=Weight(Abs(i), s); weight+=w; color.rgb+=w*TexPoint(Col, inTex+ColSize.xy*Vec2(i, 0)).rgb;} color.rgb/=weight; return color;}
#endif
   // use linear filtering because texcoords aren't rounded
   if(samples==4) // -3 .. 3
      return Vec4(TexLod(Col, inTex+ColSize.xy*Vec2( 0+WEIGHT4_1/(WEIGHT4_0/2+WEIGHT4_1), 0)).rgb*(WEIGHT4_0/2+WEIGHT4_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-0-WEIGHT4_1/(WEIGHT4_0/2+WEIGHT4_1), 0)).rgb*(WEIGHT4_0/2+WEIGHT4_1)
                 +TexLod(Col, inTex+ColSize.xy*Vec2( 2+WEIGHT4_3/(WEIGHT4_2  +WEIGHT4_3), 0)).rgb*(WEIGHT4_2  +WEIGHT4_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-2-WEIGHT4_3/(WEIGHT4_2  +WEIGHT4_3), 0)).rgb*(WEIGHT4_2  +WEIGHT4_3), 0);
   if(samples==5) // -4 .. 4
      return Vec4(TexLod(Col, inTex                                                       ).rgb* WEIGHT5_0
                 +TexLod(Col, inTex+ColSize.xy*Vec2( 1+WEIGHT5_2/(WEIGHT5_1+WEIGHT5_2), 0)).rgb*(WEIGHT5_1+WEIGHT5_2)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-1-WEIGHT5_2/(WEIGHT5_1+WEIGHT5_2), 0)).rgb*(WEIGHT5_1+WEIGHT5_2)
                 +TexLod(Col, inTex+ColSize.xy*Vec2( 3+WEIGHT5_4/(WEIGHT5_3+WEIGHT5_4), 0)).rgb*(WEIGHT5_3+WEIGHT5_4)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-3-WEIGHT5_4/(WEIGHT5_3+WEIGHT5_4), 0)).rgb*(WEIGHT5_3+WEIGHT5_4), 0);
   if(samples==6) // -5 .. 5
      return Vec4(TexLod(Col, inTex+ColSize.xy*Vec2( 0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb*(WEIGHT6_0/2+WEIGHT6_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb*(WEIGHT6_0/2+WEIGHT6_1)
                 +TexLod(Col, inTex+ColSize.xy*Vec2( 2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*(WEIGHT6_2  +WEIGHT6_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*(WEIGHT6_2  +WEIGHT6_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2( 4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*(WEIGHT6_4  +WEIGHT6_5)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(-4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*(WEIGHT6_4  +WEIGHT6_5), 0);
}
Vec4 BlurY_PS(NOPERSP Vec2 inTex:TEXCOORD,
              uniform Int  samples       ):COLOR
{
#if TEST_BLUR
   if(Test){Int s=Round(Samples); Vec4 color=0; Flt weight=0; for(Int i=-s; i<=s; i++){Flt w=Weight(Abs(i), s); weight+=w; color.rgb+=w*TexPoint(Col, inTex+ColSize.xy*Vec2(0, i)).rgb;} color.rgb/=weight; return color;}
#endif
   // use linear filtering because texcoords aren't rounded
   if(samples==4) // -3 .. 3
      return Vec4(TexLod(Col, inTex+ColSize.xy*Vec2(0,  0+WEIGHT4_1/(WEIGHT4_0/2+WEIGHT4_1))).rgb*(WEIGHT4_0/2+WEIGHT4_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -0-WEIGHT4_1/(WEIGHT4_0/2+WEIGHT4_1))).rgb*(WEIGHT4_0/2+WEIGHT4_1)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0,  2+WEIGHT4_3/(WEIGHT4_2  +WEIGHT4_3))).rgb*(WEIGHT4_2  +WEIGHT4_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -2-WEIGHT4_3/(WEIGHT4_2  +WEIGHT4_3))).rgb*(WEIGHT4_2  +WEIGHT4_3), 0);
   if(samples==5) // -4 .. 4
      return Vec4(TexLod(Col, inTex                                                       ).rgb* WEIGHT5_0
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0,  1+WEIGHT5_2/(WEIGHT5_1+WEIGHT5_2))).rgb*(WEIGHT5_1+WEIGHT5_2)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -1-WEIGHT5_2/(WEIGHT5_1+WEIGHT5_2))).rgb*(WEIGHT5_1+WEIGHT5_2)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0,  3+WEIGHT5_4/(WEIGHT5_3+WEIGHT5_4))).rgb*(WEIGHT5_3+WEIGHT5_4)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -3-WEIGHT5_4/(WEIGHT5_3+WEIGHT5_4))).rgb*(WEIGHT5_3+WEIGHT5_4), 0);
   if(samples==6) // -5 .. 5
      return Vec4(TexLod(Col, inTex+ColSize.xy*Vec2(0,  0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb*(WEIGHT6_0/2+WEIGHT6_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb*(WEIGHT6_0/2+WEIGHT6_1)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0,  2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*(WEIGHT6_2  +WEIGHT6_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*(WEIGHT6_2  +WEIGHT6_3)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0,  4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*(WEIGHT6_4  +WEIGHT6_5)
                 +TexLod(Col, inTex+ColSize.xy*Vec2(0, -4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*(WEIGHT6_4  +WEIGHT6_5), 0);
}

Vec4 BlurX_X_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   // use linear filtering because texcoords aren't rounded
   return TexLod(Col, inTex+ColSize.xy*Vec2( 0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).x*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).x*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2( 2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).x*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).x*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2( 4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).x*(WEIGHT6_4  +WEIGHT6_5)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).x*(WEIGHT6_4  +WEIGHT6_5);
}
Vec4 BlurX_W_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   // use linear filtering because texcoords aren't rounded
   return TexLod(Col, inTex+ColSize.xy*Vec2( 0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).w*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).w*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2( 2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).w*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).w*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2( 4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).w*(WEIGHT6_4  +WEIGHT6_5)
         +TexLod(Col, inTex+ColSize.xy*Vec2(-4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).w*(WEIGHT6_4  +WEIGHT6_5);
}

Vec4 BlurY_X_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   // use linear filtering because texcoords aren't rounded
   return TexLod(Col, inTex+ColSize.xy*Vec2(0,  0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).x*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).x*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0,  2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).x*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).x*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0,  4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).x*(WEIGHT6_4  +WEIGHT6_5)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).x*(WEIGHT6_4  +WEIGHT6_5);
}
Vec4 BlurY_W_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   // use linear filtering because texcoords aren't rounded
   return TexLod(Col, inTex+ColSize.xy*Vec2(0,  0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).w*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).w*(WEIGHT6_0/2+WEIGHT6_1)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0,  2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).w*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).w*(WEIGHT6_2  +WEIGHT6_3)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0,  4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).w*(WEIGHT6_4  +WEIGHT6_5)
         +TexLod(Col, inTex+ColSize.xy*Vec2(0, -4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).w*(WEIGHT6_4  +WEIGHT6_5);
}
TECHNIQUE(BlurX , Draw_VS(), BlurX_PS(4));
TECHNIQUE(BlurXH, Draw_VS(), BlurX_PS(6));
TECHNIQUE(BlurY , Draw_VS(), BlurY_PS(4));
TECHNIQUE(BlurYH, Draw_VS(), BlurY_PS(6));

TECHNIQUE(BlurX_X, Draw_VS(), BlurX_X_PS());
TECHNIQUE(BlurX_W, Draw_VS(), BlurX_W_PS());
TECHNIQUE(BlurY_X, Draw_VS(), BlurY_X_PS());
TECHNIQUE(BlurY_W, Draw_VS(), BlurY_W_PS());
/******************************************************************************/
// MAX
/******************************************************************************/
Vec4 MaxX_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   VecH color; // color=0; for(Int i=0; i<11; i++)color=Max(color, TexPoint(Col, inTex+ColSize.xy*Vec2(i-5, 0)).rgb*(BlendWeight[i]/BlendWeight[5])); original slower version
   // use linear filtering because texcoords aren't rounded
   color=           TexLod(Col, inTex+ColSize.xy*Vec2( 0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb ;
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(-0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb);
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2( 2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(-2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2( 4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(-4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   return Vec4(color, 0);
}
Vec4 MaxY_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   VecH color; // color=0; for(Int i=0; i<11; i++)color=Max(color, TexPoint(Col, inTex+ColSize.xy*Vec2(0, i-5)).rgb*(BlendWeight[i]/BlendWeight[5])); original slower version
   // use linear filtering because texcoords aren't rounded
   color=           TexLod(Col, inTex+ColSize.xy*Vec2(0,  0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb ;
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(0, -0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb);
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(0,  2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(0, -2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(0,  4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Col, inTex+ColSize.xy*Vec2(0, -4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   return Vec4(color, 0);
}
TECHNIQUE(MaxX, Draw_VS(), MaxX_PS());
TECHNIQUE(MaxY, Draw_VS(), MaxY_PS());
/******************************************************************************/
// 2D FX
/******************************************************************************/
BUFFER(ColTrans)
   Matrix ColTransMatrix;
   Vec    ColTransHsb;
BUFFER_END

Vec4 ColTrans_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   return Vec4(Transform(Tex(Col, inTex).rgb, ColTransMatrix), Step);
}
Vec4 ColTransHB_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec color     =Tex(Col, inTex).rgb;
   Flt brightness=Max(color); color=Transform(color, ColTransMatrix);
   Flt max       =Max(color);
   if( max)color*=ColTransHsb.z*brightness/max;
   return Vec4(color, Step);
}
Vec4 ColTransHSB_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec hsb=RgbToHsb(Tex(Col, inTex).rgb);
   return Vec4(HsbToRgb(Vec(hsb.x+ColTransHsb.x, hsb.y*ColTransHsb.y, hsb.z*ColTransHsb.z)), Step);
}
TECHNIQUE(ColTrans   , Draw_VS(),    ColTrans_PS());
TECHNIQUE(ColTransHB , Draw_VS(),  ColTransHB_PS());
TECHNIQUE(ColTransHSB, Draw_VS(), ColTransHSB_PS());
/******************************************************************************/
struct RippleClass
{
   Flt  xx, xy,
        yx, yy,
        stp,
        power,
        alpha_scale,
        alpha_add;
   Vec2 center;
};

BUFFER(Ripple)
   RippleClass Rppl;
BUFFER_END

Vec4 Ripple_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   inTex+=Vec2(Sin(inTex.x*Rppl.xx + inTex.y*Rppl.xy + Rppl.stp),
               Sin(inTex.x*Rppl.yx + inTex.y*Rppl.yy + Rppl.stp))*Rppl.power;

   VecH4  c=Tex(Col, inTex); c.a*=Sat((Rppl.alpha_scale*2)*Length(inTex-Rppl.center)+Rppl.alpha_add);
   return c;
}
TECHNIQUE(Ripple, Draw2DTex_VS(), Ripple_PS());
/******************************************************************************/
struct TitlesClass
{
   Flt stp,
       center,
       range,
       smooth,
       swirl;
};

BUFFER(Titles)
   TitlesClass Ttls;
BUFFER_END

Vec4 Titles_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Flt s=Sat((Abs(inTex.y-Ttls.center)-Ttls.range)/Ttls.smooth);
   inTex.x+=Sin(s*s*s*s*(PI*6)+Ttls.stp)*s*s*Ttls.swirl;
   Vec4 color=0; UNROLL for(Int i=-4; i<=4; i++)color+=Tex(Col, inTex+s*ColSize.xy*Vec2(i, 0));
   color/=9;
   color.a*=Pow(1-s, 0.6f);
   return color;
}
TECHNIQUE(Titles, Draw_VS(), Titles_PS());
/******************************************************************************/
Vec4 Fade_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   VecH4  c=Tex(Col, inTex); c.a*=Sat(Step*3+(inTex.x+inTex.y)*0.5f-1-Tex(Col1, inTex).a);
   return c;
}
TECHNIQUE(Fade, Draw_VS(), Fade_PS());
/******************************************************************************/
void Wave_VS(VtxInput vtx,
         out Vec2 outTex:TEXCOORD,
         out Vec4 outVtx:POSITION)
{
   outTex.x=vtx.tex().x*Color[0].x + vtx.tex().y*Color[0].y + Color[0].z;
   outTex.y=vtx.tex().x*Color[1].x + vtx.tex().y*Color[1].y + Color[1].z;
   outVtx  =vtx.pos4(); AdjustPixelCenter(outVtx);
}
TECHNIQUE(Wave, Wave_VS(), Draw2DTex_PS());
/******************************************************************************/
Vec4 RadialBlur_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
         Vec2 tex  =inTex/Color[0].z + (Color[0].xy-Color[0].xy/Color[0].z); // (inTex-RadialBlurPos)/step+RadialBlurPos
         Vec  color=0;
   const Int  steps=32;
   for(Int i=0; i<steps; i++)color+=Tex(Col, Lerp(inTex, tex, i/Flt(steps-1))).rgb;
   return Vec4(color/steps, Color[0].w);
}
TECHNIQUE(RadialBlur, Draw_VS(), RadialBlur_PS());
/******************************************************************************/
Vec4 Outline_PS(NOPERSP Vec2 inTex:TEXCOORD,
                uniform Bool clip_do       ,
                uniform Bool down_sample=false):COLOR
{
   if(down_sample)inTex=(Floor(inTex*ColSize.zw)+0.5f)*ColSize.xy; // we're rendering to a smaller RT, so inTex is located in the center between multiple texels, we need to align it so it's located at the center of the nearest one

   Vec4 col=TexLod(Col, inTex); // use linear filtering because 'Col' can be of different size
   Vec2 t0=inTex+ColSize.xy*(down_sample ? 2.5f : 0.5f), // 2.5 scale was the min value that worked OK with 2.0 density
        t1=inTex-ColSize.xy*(down_sample ? 2.5f : 0.5f);
   // use linear filtering because texcoords aren't rounded
#if 0
   if(Dist2(col.rgb, TexLod(Col, Vec2(t0.x, t0.y)).rgb)
     +Dist2(col.rgb, TexLod(Col, Vec2(t0.x, t1.y)).rgb)
     +Dist2(col.rgb, TexLod(Col, Vec2(t1.x, t0.y)).rgb)
     +Dist2(col.rgb, TexLod(Col, Vec2(t1.x, t1.y)).rgb)<=EPS_COL)col.a=0; // if all neighbors are the same then make this pixel transparent
#else // faster approximate
   if(Length2(col.rgb*4-TexLod(Col, Vec2(t0.x, t0.y)).rgb
                       -TexLod(Col, Vec2(t0.x, t1.y)).rgb
                       -TexLod(Col, Vec2(t1.x, t0.y)).rgb
                       -TexLod(Col, Vec2(t1.x, t1.y)).rgb)<=EPS_COL)col.a=0; // if all neighbors are the same then make this pixel transparent
#endif
   /* old code used for super sampling
	{
      Flt pos=TexDepthPoint(inTex);
      if(Dist2(col, TexLod(Col, inTex+ColSize.xy*Vec2(-1,  0)))*(TexDepthPoint(inTex+ColSize.xy*Vec2(-1,  0))>=pos)
        +Dist2(col, TexLod(Col, inTex+ColSize.xy*Vec2( 1,  0)))*(TexDepthPoint(inTex+ColSize.xy*Vec2( 1,  0))>=pos)
        +Dist2(col, TexLod(Col, inTex+ColSize.xy*Vec2( 0, -1)))*(TexDepthPoint(inTex+ColSize.xy*Vec2( 0, -1))>=pos)
        +Dist2(col, TexLod(Col, inTex+ColSize.xy*Vec2( 0,  1)))*(TexDepthPoint(inTex+ColSize.xy*Vec2( 0,  1))>=pos)<=EPS_COL)col.a=0;
	}*/
   if(clip_do)clip(col.a-EPS);
   return col;
}
Vec4 OutlineApply_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec4 color=TexLod(Col, inTex); // use linear filtering because 'Col' can be of different size
   for(Int i=0; i<6; i++)
   {
      Vec2 t=inTex+BlendOfs6[i]*ColSize.xy;
      Vec4 c=TexLod(Col, t); // use linear filtering because texcoords aren't rounded
      if(c.a>color.a)color=c;
   }
   return color;
}
TECHNIQUE(Outline     , Draw_VS(),      Outline_PS(false));
TECHNIQUE(OutlineDS   , Draw_VS(),      Outline_PS(false, true));
TECHNIQUE(OutlineClip , Draw_VS(),      Outline_PS(true ));
TECHNIQUE(OutlineApply, Draw_VS(), OutlineApply_PS());
/******************************************************************************/
Vec4 EdgeDetect_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Flt z =TexDepthPoint(inTex),
       zl=TexDepthPoint(inTex+ColSize.xy*Vec2(-1, 0)),
       zr=TexDepthPoint(inTex+ColSize.xy*Vec2( 1, 0)),
       zd=TexDepthPoint(inTex+ColSize.xy*Vec2( 0,-1)),
       zu=TexDepthPoint(inTex+ColSize.xy*Vec2( 0, 1)), soft=0.1f+z/50;

   Flt px  =Abs(zr-((z-zl)+z))/soft-0.5f, //cx=Sat(Max(Abs(zl-z), Abs(zr-z))/soft-0.5f), cx, cy doesn't work well in lower screen resolutions and with flat terrain
       py  =Abs(zu-((z-zd)+z))/soft-0.5f, //cy=Sat(Max(Abs(zu-z), Abs(zd-z))/soft-0.5f),
       edge=px+py;

   return Sat(1-edge); // saturate because this can be directly multiplied to dest using ALPHA_MUL
}
Vec4 EdgeDetectApply_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   const Int samples=6;
         Flt color  =TexPoint(Col, inTex).API(a, r, r); // DX9 uses A8 while others use R8 RT
   for(Int i=0; i<samples; i++)
   {
      Vec2 t=inTex+BlendOfs6[i]*ColSize.xy;
      color+=TexLod(Col, t).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
   }
   return color/(samples+1);
}
TECHNIQUE(EdgeDetect     , Draw_VS(),      EdgeDetect_PS());
TECHNIQUE(EdgeDetectApply, Draw_VS(), EdgeDetectApply_PS());
/******************************************************************************/
Vec4 Dither_PS(NOPERSP Vec2 inTex:TEXCOORD,
               NOPERSP PIXEL):COLOR
{
   Vec4 col=Vec4(TexLod(Col, inTex).rgb, 1); // force full alpha so back buffer effects can work ok, can't use 'TexPoint' because 'Col' can be of different size
   col.rgb+=DitherValueColor(pixel);
   return col;
}
TECHNIQUE(Dither, Draw_VS(), Dither_PS());
/******************************************************************************/
Vec4 CombineSSAlpha_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   return DEPTH_FOREGROUND(TexDepthRawPoint(inTex));
}
Vec4 Combine_PS(NOPERSP Vec2 inTex:TEXCOORD,
                NOPERSP PIXEL,
                uniform Int  sample        ):COLOR
{
   Vec4 col=TexLod(Col, inTex); // use linear filtering because Col can be of different size
#if MODEL>=SM_4
   if(sample==2) // multi sample
   {
      col.w =0; UNROLL for(Int i=0; i<MS_SAMPLES; i++)col.w+=DEPTH_FOREGROUND(TexDepthMSRaw(pixel.xy, i));
      col.w/=MS_SAMPLES;
      // here col.rgb is not premultiplied by alpha (it is at full scale), which means that it will not work as smooth when doing the blended support below

      // if any of the neighbor pixels are transparent then assume that there's no blended graphics in the area, and then return just the solid pixel to keep AA
      // use linear filtering because Col can be of different size
      if(Max(TexLod(Col, inTex+ColSize.xy*Vec2( 0, SQRT2)).rgb)<=0.15f
      || Max(TexLod(Col, inTex+ColSize.xy*Vec2(-1,    -1)).rgb)<=0.15f
      || Max(TexLod(Col, inTex+ColSize.xy*Vec2( 1,    -1)).rgb)<=0.15f)return col; // there can be bloom around solid pixels, so allow some tolerance
   }else
#endif
   if(sample==1)col.w=TexLod(Col1, inTex).API(a, r, r); // super sample, DX9 uses A8 while others use R8 RT, use linear filtering because Col can be of different size
   else         col.w=DEPTH_FOREGROUND(TexDepthRawPoint(inTex)); // single sample

   // support blended graphics (pixels with colors but without depth)
      col.w=Max(col); // treat luminance as opacity
   if(col.w>0)col.rgb/=col.w;
   return col;
}
TECHNIQUE(CombineSSAlpha, Draw_VS(), CombineSSAlpha_PS( ));
TECHNIQUE(Combine       , Draw_VS(),        Combine_PS(0));
TECHNIQUE(CombineSS     , Draw_VS(),        Combine_PS(1));
#if MODEL>=SM_4
TECHNIQUE(CombineMS     , Draw_VS(),        Combine_PS(2));
#endif
/******************************************************************************/
#if MODEL>=SM_4
void ResolveDepth_PS(NOPERSP PIXEL,
                         out Flt depth:DEPTH)
{
   // return the smallest of all samples
                                         depth=                 TexSample(ColMS, pixel.xy, 0).x;
   UNROLL for(Int i=1; i<MS_SAMPLES; i++)depth=DEPTH_MIN(depth, TexSample(ColMS, pixel.xy, i).x); // have to use minimum of depth samples to avoid shadow artifacts, by picking the samples that are closer to the camera, similar effect to what we do with view space bias (if Max is used, then shadow acne can occur for local lights)
}
TECHNIQUE(ResolveDepth, DrawPixel_VS(), ResolveDepth_PS());

// 'Depth'   can't be used because it's            1-sample
// 'DepthMs' can't be used because it's always multi-sampled (all samples are different)
void DetectMSCol_PS(NOPERSP PIXEL)
{
   Vec cols[4]={TexSample(ColMS, pixel.xy, 0).rgb, TexSample(ColMS, pixel.xy, 1).rgb, TexSample(ColMS, pixel.xy, 2).rgb, TexSample(ColMS, pixel.xy, 3).rgb}; // load 4-multi-samples of texel
 //if(all((cols[0]==cols[1])*(cols[0]==cols[2])*(cols[0]==cols[3])))discard;
   clip(Length2(cols[0].rgb*3-cols[1].rgb-cols[2].rgb-cols[3].rgb) - Sqr(2.0f/256)); // simplified and approximate version of testing if samples are identical, (cols[0].rgb-cols[1].rgb) + (cols[0].rgb-cols[2].rgb) + (cols[0].rgb-cols[3].rgb)
}
TECHNIQUE(DetectMSCol, DrawPixel_VS(), DetectMSCol_PS());
/*void DetectMSNrm_PS(NOPERSP PIXEL)
{
   Vec2 nrms[4]={TexSample(ColMS, pixel.xy, 0).xy, TexSample(ColMS, pixel.xy, 1).xy, TexSample(ColMS, pixel.xy, 2).xy, TexSample(ColMS, pixel.xy, 3).xy}; // load 4-multi-samples of texel
 //if(all((nrms[0]==nrms[1])*(nrms[0]==nrms[2])*(nrms[0]==nrms[3])))discard;
   clip(Length2(nrms[0].xy*3-nrms[1].xy-nrms[2].xy-nrms[3].xy) - Sqr(2.0f/256*(SIGNED_NRM_RT ? 2 : 1))); // simplified and approximate version of testing if samples are identical, (nrms[0].xy-nrms[1].xy) + (nrms[0].xy-nrms[2].xy) + (nrms[0].xy-nrms[3].xy), for SIGNED_NRM_RT we have to use bigger epsilon because we have 2x bigger value range (-1..1 instead of 0..1)
}
TECHNIQUE(DetectMSNrm, DrawPixel_VS(), DetectMSNrm_PS());*/
#endif

void SetDepth_PS(NOPERSP Vec2 inTex:TEXCOORD,
                  #if MODEL==SM_3
                     out Vec4 ret  :COLOR   ,
                  #endif
                     out Flt  depth:DEPTH   )
{
#if MODEL==SM_3
   ret=0;
#endif
   depth=TexLod(Col, inTex).x; // use linear filtering because this can be used for different size RT
}
TECHNIQUE(SetDepth, Draw_VS(), SetDepth_PS());

/*void RebuildDepth_PS(NOPERSP Vec2 inTex:TEXCOORD,
                         out Flt  depth:DEPTH   ,
                     uniform Bool perspective   )
{
   depth=DelinearizeDepth(TexLod(Col, inTex).x, perspective); // use linear filtering because this can be used for different size RT
}
TECHNIQUE(RebuildDepth , Draw_VS(), RebuildDepth_PS(false));
TECHNIQUE(RebuildDepthP, Draw_VS(), RebuildDepth_PS(true ));*/

Vec4 LinearizeDepth_PS(NOPERSP Vec2 inTex:TEXCOORD,
                       uniform Bool perspective   ):COLOR
{
   return LinearizeDepth(TexLod(Col, inTex).x, perspective); // use linear filtering because this can be used for different size RT
}
TECHNIQUE(LinearizeDepth0 , Draw_VS(), LinearizeDepth_PS(false));
TECHNIQUE(LinearizeDepthP0, Draw_VS(), LinearizeDepth_PS(true ));
#if MODEL>=SM_4
Vec4 LinearizeDepth1_PS(NOPERSP PIXEL,
                        uniform Bool perspective):COLOR
{
   return LinearizeDepth(TexSample(ColMS, pixel.xy, 0).x, perspective);
}
Vec4 LinearizeDepth2_PS(NOPERSP PIXEL,
                                UInt index:SV_SampleIndex,
                        uniform Bool perspective         ):COLOR
{
   return LinearizeDepth(TexSample(ColMS, pixel.xy, index).x, perspective);
}
TECHNIQUE    (LinearizeDepth1 , DrawPixel_VS(), LinearizeDepth1_PS(false));
TECHNIQUE    (LinearizeDepthP1, DrawPixel_VS(), LinearizeDepth1_PS(true ));
TECHNIQUE_4_1(LinearizeDepth2 , DrawPixel_VS(), LinearizeDepth2_PS(false));
TECHNIQUE_4_1(LinearizeDepthP2, DrawPixel_VS(), LinearizeDepth2_PS(true ));
#endif

#if DX9
Vec4 LinearizeDepthRAWZ_PS(NOPERSP Vec2 inTex:TEXCOORD,
                           uniform Bool perspective   ):COLOR
{
   Vec rawval=Floor(255.0f*TexPoint(Col, inTex).arg+0.5f);
   Flt w     =Dot  (rawval, Vec(0.996093809371817670572857294849, 0.0038909914428586627756752238080039, 1.5199185323666651467481343000015e-5)/255.0);
   return LinearizeDepth(w, perspective);
}
TECHNIQUE(LinearizeDepthRAWZ , Draw_VS(), LinearizeDepthRAWZ_PS(false));
TECHNIQUE(LinearizeDepthRAWZP, Draw_VS(), LinearizeDepthRAWZ_PS(true ));
#endif

Vec4 DrawDepth_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Flt frac=LinearizeDepth(TexLod(Col, inTex).x)/Viewport.range; // use linear filtering because this can be used for different size RT
   return Vec4(HsbToRgb(Vec(frac*2.57f, 1, 1)), 1); // the scale is set so the full range equals to blue color, to imitate sky color
}
TECHNIQUE(DrawDepth, Draw_VS(), DrawDepth_PS());
/******************************************************************************/
// SKY
/******************************************************************************/
inline Vec4 SkyColor(Vec inTex)
{
   Flt hor=Pow(1-Sat(inTex.y), SkyHorExp);
   return Lerp(SkySkyCol, SkyHorCol, hor);
}

inline VecH4 SkyTex(Vec inTex, Vec inTexStar, VecH4 inCol, Flt alpha, uniform Bool per_vertex, uniform Bool density, uniform Int textures, uniform Bool stars)
{
   if(density)alpha=Pow(SkyDnsExp, alpha)*SkyDnsMulAdd.x+SkyDnsMulAdd.y; // here 'alpha' means opacity of the sky which is used as the distance from start to end point, this function matches 'AccumulatedDensity'

   if(textures==2)return VecH4(Lerp(TexCube(Rfl, inTex).rgb, TexCube(Cub, inTex).rgb, SkyBoxBlend), alpha);else
   if(textures==1)return VecH4(     TexCube(Rfl, inTex).rgb,                                        alpha);else
   {
      if(!per_vertex)
      {
         inTex=Normalize(inTex);
         inCol=SkyColor (inTex);

         Flt cos      =Dot(SkySunPos, inTex),
             highlight=1+Sqr(cos)*((cos>0) ? SkySunHighlight.x : SkySunHighlight.y); // rayleigh
         inCol.rgb*=highlight;
      }

      if(stars)inCol.rgb=Lerp(TexCube(Rfl, inTexStar).rgb, inCol.rgb, inCol.a);
      return VecH4(inCol.rgb, alpha);
   }
}

void Sky_VS(VtxInput vtx,
        out Vec4  outVtx     :POSITION ,
        out Vec   outPos     :TEXCOORD0,
        out Vec   outTex     :TEXCOORD1,
        out Vec   outTexStar :TEXCOORD2,
        out Vec   outTexCloud:TEXCOORD3,
        out VecH4 outCol     :COLOR0   ,
        out VecH4 outColCloud:COLOR1   ,
    uniform Bool  per_vertex           ,
    uniform Bool  stars                ,
    uniform Bool  clouds               )
{
                                outTex    =             vtx.pos();
   if(stars     )               outTexStar=Transform   (vtx.pos(), SkyStarOrn);
                 outVtx=Project(outPos    =TransformPos(vtx.pos()            ));
   if(per_vertex)outCol=                   SkyColor    (vtx.pos());

   if(clouds)
   {
      outTexCloud=vtx.pos()*Vec(LCScale, 1, LCScale);
      outColCloud=CL[0].color; outColCloud.a*=Sat(CloudAlpha(vtx.pos().y));
   }
}

Vec4 Sky_PS(PIXEL,
            Vec   inPos     :TEXCOORD0,
            Vec   inTex     :TEXCOORD1,
            Vec   inTexStar :TEXCOORD2,
            Vec   inTexCloud:TEXCOORD3,
            VecH4 inCol     :COLOR0   ,
            VecH4 inColCloud:COLOR1   ,
    uniform Bool  per_vertex          ,
    uniform Bool  flat                ,
    uniform Bool  density             ,
    uniform Int   textures            ,
    uniform Bool  stars               ,
    uniform Bool  clouds              ,
    uniform Bool  dither              ):COLOR
{
   Flt alpha; if(flat)alpha=0;else // flat uses ALPHA_NONE
   {
      alpha=TexDepthPoint(PIXEL_TO_SCREEN)/Normalize(inPos).z;
      alpha=Sat(alpha*SkyFracMulAdd.x + SkyFracMulAdd.y);
   }
   VecH4 col=SkyTex(inTex, inTexStar, inCol, alpha, per_vertex, density, textures, stars);
   if(clouds)
   {
      Vec2  uv =Normalize(inTexCloud).xz;
      VecH4 tex=Tex(Col, uv*CL[0].scale + CL[0].position)*inColCloud;
      col.rgb=Lerp(col.rgb, tex.rgb, tex.a);
   }
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
#if MODEL>=SM_4
Vec4 Sky1_PS(PIXEL,
             Vec  inPos     :TEXCOORD0,
             Vec  inTex     :TEXCOORD1,
             Vec  inTexStar :TEXCOORD2,
             Vec  inTexCloud:TEXCOORD3,
             Vec4 inCol     :COLOR    ,
     uniform Bool per_vertex          ,
     uniform Bool density             ,
     uniform Int  textures            ,
     uniform Bool stars               ,
     uniform Bool dither              ):COLOR
{
   Flt pos_scale=Normalize(inPos).z, alpha=0;
   UNROLL for(Int i=0; i<MS_SAMPLES; i++){Flt dist=TexDepthMS(pixel.xy, i)/pos_scale; alpha+=Sat(dist*SkyFracMulAdd.x + SkyFracMulAdd.y);}
   alpha/=MS_SAMPLES;
   VecH4 col=SkyTex(inTex, inTexStar, inCol, alpha, per_vertex, density, textures, stars);
   if(dither)col.rgb+=DitherValueColor(pixel);
   return col;
}
Vec4 Sky2_PS(PIXEL,
             Vec  inPos     :TEXCOORD0     ,
             Vec  inTex     :TEXCOORD1     ,
             Vec  inTexStar :TEXCOORD2     ,
             Vec  inTexCloud:TEXCOORD3     ,
             Vec4 inCol     :COLOR         ,
             UInt index     :SV_SampleIndex,
     uniform Bool per_vertex               ,
     uniform Bool density                  ,
     uniform Int  textures                 ,
     uniform Bool stars                    ,
     uniform Bool dither                   ):COLOR
{
   Flt pos_scale=Normalize(inPos).z,
       alpha    =Sat(TexDepthMS(pixel.xy, index)/pos_scale*SkyFracMulAdd.x + SkyFracMulAdd.y);
   VecH4 col=SkyTex(inTex, inTexStar, inCol, alpha, per_vertex, density, textures, stars);
   // skip dither for MS because it won't be noticeable
   return col;
}
#endif
// Textures Flat
TECHNIQUE    (SkyTF1   , Sky_VS(false, false, false), Sky_PS (false, true , false, 1, false, false, false));
TECHNIQUE    (SkyTF2   , Sky_VS(false, false, false), Sky_PS (false, true , false, 2, false, false, false));
TECHNIQUE    (SkyTF1C  , Sky_VS(false, false, true ), Sky_PS (false, true , false, 1, false, true , false));
TECHNIQUE    (SkyTF2C  , Sky_VS(false, false, true ), Sky_PS (false, true , false, 2, false, true , false));
TECHNIQUE    (SkyTF1D  , Sky_VS(false, false, false), Sky_PS (false, true , false, 1, false, false, true ));
TECHNIQUE    (SkyTF2D  , Sky_VS(false, false, false), Sky_PS (false, true , false, 2, false, false, true ));
TECHNIQUE    (SkyTF1CD , Sky_VS(false, false, true ), Sky_PS (false, true , false, 1, false, true , true ));
TECHNIQUE    (SkyTF2CD , Sky_VS(false, false, true ), Sky_PS (false, true , false, 2, false, true , true ));

// Textures
TECHNIQUE    (SkyT10   , Sky_VS(false, false, false), Sky_PS (false, false, false, 1, false, false, false));
TECHNIQUE    (SkyT20   , Sky_VS(false, false, false), Sky_PS (false, false, false, 2, false, false, false));
TECHNIQUE    (SkyT10D  , Sky_VS(false, false, false), Sky_PS (false, false, false, 1, false, false, true ));
TECHNIQUE    (SkyT20D  , Sky_VS(false, false, false), Sky_PS (false, false, false, 2, false, false, true ));
#if MODEL>=SM_4 // Multi Sample
TECHNIQUE    (SkyT11   , Sky_VS(false, false, false), Sky1_PS(false, false, 1, false, false));
TECHNIQUE    (SkyT21   , Sky_VS(false, false, false), Sky1_PS(false, false, 2, false, false));
TECHNIQUE_4_1(SkyT12   , Sky_VS(false, false, false), Sky2_PS(false, false, 1, false, false));
TECHNIQUE_4_1(SkyT22   , Sky_VS(false, false, false), Sky2_PS(false, false, 2, false, false));
TECHNIQUE    (SkyT11D  , Sky_VS(false, false, false), Sky1_PS(false, false, 1, false, true ));
TECHNIQUE    (SkyT21D  , Sky_VS(false, false, false), Sky1_PS(false, false, 2, false, true ));
TECHNIQUE_4_1(SkyT12D  , Sky_VS(false, false, false), Sky2_PS(false, false, 1, false, true ));
TECHNIQUE_4_1(SkyT22D  , Sky_VS(false, false, false), Sky2_PS(false, false, 2, false, true ));
#endif

// Atmospheric Flat
TECHNIQUE    (SkyAF    , Sky_VS(false, false, false), Sky_PS(false, true ,false, 0, false, false, false));
TECHNIQUE    (SkyAFV   , Sky_VS(true , false, false), Sky_PS(true , true ,false, 0, false, false, false));
TECHNIQUE    (SkyAFS   , Sky_VS(false, true , false), Sky_PS(false, true ,false, 0, true , false, false));
TECHNIQUE    (SkyAFVS  , Sky_VS(true , true , false), Sky_PS(true , true ,false, 0, true , false, false));
TECHNIQUE    (SkyAFC   , Sky_VS(false, false, true ), Sky_PS(false, true ,false, 0, false, true , false));
TECHNIQUE    (SkyAFVC  , Sky_VS(true , false, true ), Sky_PS(true , true ,false, 0, false, true , false));
TECHNIQUE    (SkyAFSC  , Sky_VS(false, true , true ), Sky_PS(false, true ,false, 0, true , true , false));
TECHNIQUE    (SkyAFVSC , Sky_VS(true , true , true ), Sky_PS(true , true ,false, 0, true , true , false));
TECHNIQUE    (SkyAFD   , Sky_VS(false, false, false), Sky_PS(false, true ,false, 0, false, false, true ));
TECHNIQUE    (SkyAFVD  , Sky_VS(true , false, false), Sky_PS(true , true ,false, 0, false, false, true ));
TECHNIQUE    (SkyAFSD  , Sky_VS(false, true , false), Sky_PS(false, true ,false, 0, true , false, true ));
TECHNIQUE    (SkyAFVSD , Sky_VS(true , true , false), Sky_PS(true , true ,false, 0, true , false, true ));
TECHNIQUE    (SkyAFCD  , Sky_VS(false, false, true ), Sky_PS(false, true ,false, 0, false, true , true ));
TECHNIQUE    (SkyAFVCD , Sky_VS(true , false, true ), Sky_PS(true , true ,false, 0, false, true , true ));
TECHNIQUE    (SkyAFSCD , Sky_VS(false, true , true ), Sky_PS(false, true ,false, 0, true , true , true ));
TECHNIQUE    (SkyAFVSCD, Sky_VS(true , true , true ), Sky_PS(true , true ,false, 0, true , true , true ));

// Atmospheric
TECHNIQUE    (SkyA0    , Sky_VS(false, false, false), Sky_PS(false, false, false, 0, false, false, false));
TECHNIQUE    (SkyAV0   , Sky_VS(true , false, false), Sky_PS(true , false, false, 0, false, false, false));
TECHNIQUE    (SkyAS0   , Sky_VS(false, true , false), Sky_PS(false, false, false, 0, true , false, false));
TECHNIQUE    (SkyAVS0  , Sky_VS(true , true , false), Sky_PS(true , false, false, 0, true , false, false));
TECHNIQUE    (SkyAP0   , Sky_VS(false, false, false), Sky_PS(false, false, true , 0, false, false, false));
TECHNIQUE    (SkyAVP0  , Sky_VS(true , false, false), Sky_PS(true , false, true , 0, false, false, false));
TECHNIQUE    (SkyASP0  , Sky_VS(false, true , false), Sky_PS(false, false, true , 0, true , false, false));
TECHNIQUE    (SkyAVSP0 , Sky_VS(true , true , false), Sky_PS(true , false, true , 0, true , false, false));
TECHNIQUE    (SkyA0D   , Sky_VS(false, false, false), Sky_PS(false, false, false, 0, false, false, true ));
TECHNIQUE    (SkyAV0D  , Sky_VS(true , false, false), Sky_PS(true , false, false, 0, false, false, true ));
TECHNIQUE    (SkyAS0D  , Sky_VS(false, true , false), Sky_PS(false, false, false, 0, true , false, true ));
TECHNIQUE    (SkyAVS0D , Sky_VS(true , true , false), Sky_PS(true , false, false, 0, true , false, true ));
TECHNIQUE    (SkyAP0D  , Sky_VS(false, false, false), Sky_PS(false, false, true , 0, false, false, true ));
TECHNIQUE    (SkyAVP0D , Sky_VS(true , false, false), Sky_PS(true , false, true , 0, false, false, true ));
TECHNIQUE    (SkyASP0D , Sky_VS(false, true , false), Sky_PS(false, false, true , 0, true , false, true ));
TECHNIQUE    (SkyAVSP0D, Sky_VS(true , true , false), Sky_PS(true , false, true , 0, true , false, true ));

#if MODEL>=SM_4 // Multi Sample
TECHNIQUE    (SkyA1    , Sky_VS(false, false, false), Sky1_PS(false, false, 0, false, false));
TECHNIQUE    (SkyAV1   , Sky_VS(true , false, false), Sky1_PS(true , false, 0, false, false));
TECHNIQUE    (SkyAS1   , Sky_VS(false, true , false), Sky1_PS(false, false, 0, true , false));
TECHNIQUE    (SkyAVS1  , Sky_VS(true , true , false), Sky1_PS(true , false, 0, true , false));
TECHNIQUE    (SkyAP1   , Sky_VS(false, false, false), Sky1_PS(false, true , 0, false, false));
TECHNIQUE    (SkyAVP1  , Sky_VS(true , false, false), Sky1_PS(true , true , 0, false, false));
TECHNIQUE    (SkyASP1  , Sky_VS(false, true , false), Sky1_PS(false, true , 0, true , false));
TECHNIQUE    (SkyAVSP1 , Sky_VS(true , true , false), Sky1_PS(true , true , 0, true , false));
TECHNIQUE    (SkyA1D   , Sky_VS(false, false, false), Sky1_PS(false, false, 0, false, true ));
TECHNIQUE    (SkyAV1D  , Sky_VS(true , false, false), Sky1_PS(true , false, 0, false, true ));
TECHNIQUE    (SkyAS1D  , Sky_VS(false, true , false), Sky1_PS(false, false, 0, true , true ));
TECHNIQUE    (SkyAVS1D , Sky_VS(true , true , false), Sky1_PS(true , false, 0, true , true ));
TECHNIQUE    (SkyAP1D  , Sky_VS(false, false, false), Sky1_PS(false, true , 0, false, true ));
TECHNIQUE    (SkyAVP1D , Sky_VS(true , false, false), Sky1_PS(true , true , 0, false, true ));
TECHNIQUE    (SkyASP1D , Sky_VS(false, true , false), Sky1_PS(false, true , 0, true , true ));
TECHNIQUE    (SkyAVSP1D, Sky_VS(true , true , false), Sky1_PS(true , true , 0, true , true ));

TECHNIQUE_4_1(SkyA2    , Sky_VS(false, false, false), Sky2_PS(false, false, 0, false, false));
TECHNIQUE_4_1(SkyAV2   , Sky_VS(true , false, false), Sky2_PS(true , false, 0, false, false));
TECHNIQUE_4_1(SkyAS2   , Sky_VS(false, true , false), Sky2_PS(false, false, 0, true , false));
TECHNIQUE_4_1(SkyAVS2  , Sky_VS(true , true , false), Sky2_PS(true , false, 0, true , false));
TECHNIQUE_4_1(SkyAP2   , Sky_VS(false, false, false), Sky2_PS(false, true , 0, false, false));
TECHNIQUE_4_1(SkyAVP2  , Sky_VS(true , false, false), Sky2_PS(true , true , 0, false, false));
TECHNIQUE_4_1(SkyASP2  , Sky_VS(false, true , false), Sky2_PS(false, true , 0, true , false));
TECHNIQUE_4_1(SkyAVSP2 , Sky_VS(true , true , false), Sky2_PS(true , true , 0, true , false));
TECHNIQUE_4_1(SkyA2D   , Sky_VS(false, false, false), Sky2_PS(false, false, 0, false, true ));
TECHNIQUE_4_1(SkyAV2D  , Sky_VS(true , false, false), Sky2_PS(true , false, 0, false, true ));
TECHNIQUE_4_1(SkyAS2D  , Sky_VS(false, true , false), Sky2_PS(false, false, 0, true , true ));
TECHNIQUE_4_1(SkyAVS2D , Sky_VS(true , true , false), Sky2_PS(true , false, 0, true , true ));
TECHNIQUE_4_1(SkyAP2D  , Sky_VS(false, false, false), Sky2_PS(false, true , 0, false, true ));
TECHNIQUE_4_1(SkyAVP2D , Sky_VS(true , false, false), Sky2_PS(true , true , 0, false, true ));
TECHNIQUE_4_1(SkyASP2D , Sky_VS(false, true , false), Sky2_PS(false, true , 0, true , true ));
TECHNIQUE_4_1(SkyAVSP2D, Sky_VS(true , true , false), Sky2_PS(true , true , 0, true , true ));
#endif
/******************************************************************************/
// FOG
/******************************************************************************/
Vec4 Fog_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
            NOPERSP Vec2 inPosXY:TEXCOORD1):COLOR
{
   Vec pos=GetPosPoint(inTex, inPosXY);
   Flt dns=AccumulatedDensity(FogDensity(), Length(pos));

   return Vec4(FogColor(), dns);
}
#if MODEL>=SM_4
Vec4 FogN_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
             NOPERSP Vec2 inPosXY:TEXCOORD1,
             NOPERSP PIXEL                 ):COLOR
{
   Flt valid=EPS, dns=0;
   UNROLL for(Int i=0; i<MS_SAMPLES; i++)
   {
      Flt depth=TexDepthMSRaw(pixel.xy, i); if(DEPTH_FOREGROUND(depth))
      {
         Vec pos =GetPos(LinearizeDepth(depth), inPosXY);
             dns+=AccumulatedDensity(FogDensity(), Length(pos));
         valid++;
      }
   }
   return Vec4(FogColor(), dns/valid);
}
Vec4 FogM_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
             NOPERSP Vec2 inPosXY:TEXCOORD1,
             NOPERSP PIXEL                 ,
                     UInt index  :SV_SampleIndex):COLOR
{
   Vec pos=GetPosMS(pixel.xy, index, inPosXY);
   return Vec4(FogColor(), AccumulatedDensity(FogDensity(), Length(pos)));
}
#endif
TECHNIQUE    (Fog , DrawPosXY_VS(), Fog_PS ());
#if MODEL>=SM_4 // multi sample
TECHNIQUE    (FogN, DrawPosXY_VS(), FogN_PS());
TECHNIQUE_4_1(FogM, DrawPosXY_VS(), FogM_PS());
#endif
/******************************************************************************/
// SUN
/******************************************************************************/
struct SunClass
{
   Vec2 pos2;
   Vec  pos, color;
};

BUFFER(Sun)
   SunClass Sun;
BUFFER_END

Vec4 SunRaysMask_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                    NOPERSP Vec2 inPosXY:TEXCOORD1,
                    uniform Bool mask             ):COLOR
{
   if(mask) // for this version we have to use linear depth filtering, because RT is of different size, and otherwise too much artifacts/flickering is generated
   {
   #if REVERSE_DEPTH // we can use the simple version for REVERSE_DEPTH
      Half m=(Length2(GetPosLinear(inTex, inPosXY))>=Sqr(Viewport.range));
   #else // need safer
      Flt  z=TexDepthRawLinear(inTex);
      Half m=(DEPTH_BACKGROUND(z) || Length2(GetPos(LinearizeDepth(z), inPosXY))>=Sqr(Viewport.range));
   #endif
      return m*TexLod(Col, inTex).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because Col can be of different size
   }else // can use point filtering here
   {
   #if REVERSE_DEPTH // we can use the simple version for REVERSE_DEPTH
      return Length2(GetPosPoint(inTex, inPosXY))>=Sqr(Viewport.range);
   #else // need safer
      Flt z=TexDepthRawPoint(inTex);
      return DEPTH_BACKGROUND(z) || Length2(GetPos(LinearizeDepth(z), inPosXY))>=Sqr(Viewport.range);
   #endif
   }
}
TECHNIQUE(SunRaysMask , DrawPosXY_VS(), SunRaysMask_PS(false));
TECHNIQUE(SunRaysMask1, DrawPosXY_VS(), SunRaysMask_PS(true ));
/*Vec4 SunRaysSoft_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   const Int samples=6;
         Flt color  =TexLod(Col, inTex).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because Col can be of different size
   for(Int i=0; i<samples; i++)
   {
      Vec2 t=inTex+BlendOfs6[i]*ColSize.xy;
      color+=TexLod(Col, t).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
   }
   return Vec4(color/(samples+1)*Color[0].rgb, 0);
}
TECHNIQUE(SunRaysSoft, Draw_VS(), SunRaysSoft_PS());*/
Vec4 SunRays_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                NOPERSP Vec2 inPosXY:TEXCOORD1,
                NOPERSP PIXEL                 ,
                uniform Bool high             ,
                uniform Bool jitter           ):COLOR
{
   Vec  pos  =Normalize(Vec(inPosXY, 1));
   Flt  cos  =Dot(Sun.pos, pos),
        power=((cos>0) ? cos*cos : 0);
   Vec4 col  =0;

#if 0 // can't use 'clip' because we always have to set output (to 0 if no rays)
   clip(power-EPS_COL);
#else
   BRANCH if(power>EPS_COL)
#endif
   {
      const Int  steps  =40;
            Flt  light  =0;
            Vec2 sun_pos=Sun.pos2;

      // limit sun position
   #if 0
      if(sun_pos.x>1)
      {
         Flt frac=(1-inTex.x)/(sun_pos.x-inTex.x);
         sun_pos.x =1;
         sun_pos.y-=(1-frac)*(sun_pos.y-inTex.y);
       //power    *=frac;
      }else
      if(sun_pos.x<0)
      {
         Flt frac=(inTex.x)/(inTex.x-sun_pos.x);
         sun_pos.x =0;
         sun_pos.y-=(1-frac)*(sun_pos.y-inTex.y);
       //power    *=frac;
      }

      if(sun_pos.y>1)
      {
         Flt frac=(1-inTex.y)/(sun_pos.y-inTex.y);
         sun_pos.y =1;
         sun_pos.x-=(1-frac)*(sun_pos.x-inTex.x);
       //power    *=frac;
      }else
      if(sun_pos.y<0)
      {
         Flt frac=(inTex.y)/(inTex.y-sun_pos.y);
         sun_pos.y =0;
         sun_pos.x-=(1-frac)*(sun_pos.x-inTex.x);
       //power    *=frac;
      }
   #else
    //Flt frac=Max(Max(Vec2(0,0), Abs(sun_pos-0.5f)-0.5f)/Abs(sun_pos-inTex)); // Max(Max(0, Abs(sun_pos.x-0.5)-0.5)/Abs(sun_pos.x-inTex.x), Max(0, Abs(sun_pos.y-0.5)-0.5)/Abs(sun_pos.y-inTex.y));
      Flt frac=Max(Max(Vec2(0,0), Abs(sun_pos-Viewport.center)-Viewport.size/2)/Abs(sun_pos-inTex)); // Max(Max(0, Abs(sun_pos.x-0.5)-0.5)/Abs(sun_pos.x-inTex.x), Max(0, Abs(sun_pos.y-0.5)-0.5)/Abs(sun_pos.y-inTex.y));
      sun_pos-=(  frac)*(sun_pos-inTex);
    //power  *=(1-frac);
   #endif

      if(jitter)inTex+=(sun_pos-inTex)*(DitherValue(pixel)*(3.0f/steps)); // a good value is 2.5 or 3.0 (3.0 was slightly better)

      UNROLL for(Int i=0; i<steps; i++)
      {
         Vec2 t=Lerp(inTex, sun_pos, i/Flt(steps)); // /(steps) worked better than /(steps-1)
         if(high)light+=TexLod(Col, t).API(a, r, r); // pos and clouds combined together, DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
         else    light+=DEPTH_BACKGROUND(TexDepthRawPoint(t)); // use simpler version here unlike in 'SunRaysPre_PS' because this one is called for each step for each pixel
      }
      col.rgb=(light*power/steps)*Sun.color;
   }
   return col;
}
TECHNIQUE(SunRays  , DrawPosXY_VS(), SunRays_PS(false, false));
TECHNIQUE(SunRaysH , DrawPosXY_VS(), SunRays_PS(true , false));
TECHNIQUE(SunRaysJ , DrawPosXY_VS(), SunRays_PS(false, true ));
TECHNIQUE(SunRaysHJ, DrawPosXY_VS(), SunRays_PS(true , true ));
/******************************************************************************/
// SHADOW MAP
/******************************************************************************/
Vec4 ShdDir_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
               NOPERSP Vec2 inPosXY:TEXCOORD1,
               NOPERSP PIXEL                 ,
               uniform Int  num              ,
               uniform Bool cloud            ):COLOR
{
   return ShadowDirValue(GetPosPoint(inTex, inPosXY), ShadowJitter(pixel.xy), true, num, cloud);
}
#if MODEL>=SM_4
Vec4 ShdDirM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                NOPERSP PIXEL                      ,
                        UInt index  :SV_SampleIndex,
                uniform Int  num                   ,
                uniform Bool cloud                 ):COLOR
{
   return ShadowDirValue(GetPosMS(pixel.xy, index, inPosXY), ShadowJitter(pixel.xy), true, num, cloud);
}
#endif
TECHNIQUE(ShdDir1, DrawPosXY_VS(), ShdDir_PS(1, false));   TECHNIQUE(ShdDir1C, DrawPosXY_VS(), ShdDir_PS(1, true));
TECHNIQUE(ShdDir2, DrawPosXY_VS(), ShdDir_PS(2, false));   TECHNIQUE(ShdDir2C, DrawPosXY_VS(), ShdDir_PS(2, true));
TECHNIQUE(ShdDir3, DrawPosXY_VS(), ShdDir_PS(3, false));   TECHNIQUE(ShdDir3C, DrawPosXY_VS(), ShdDir_PS(3, true));
TECHNIQUE(ShdDir4, DrawPosXY_VS(), ShdDir_PS(4, false));   TECHNIQUE(ShdDir4C, DrawPosXY_VS(), ShdDir_PS(4, true));
TECHNIQUE(ShdDir5, DrawPosXY_VS(), ShdDir_PS(5, false));   TECHNIQUE(ShdDir5C, DrawPosXY_VS(), ShdDir_PS(5, true));
TECHNIQUE(ShdDir6, DrawPosXY_VS(), ShdDir_PS(6, false));   TECHNIQUE(ShdDir6C, DrawPosXY_VS(), ShdDir_PS(6, true));

TECHNIQUE_4_1(ShdDir1M, DrawPosXY_VS(), ShdDirM_PS(1, false));   TECHNIQUE_4_1(ShdDir1CM, DrawPosXY_VS(), ShdDirM_PS(1, true));
TECHNIQUE_4_1(ShdDir2M, DrawPosXY_VS(), ShdDirM_PS(2, false));   TECHNIQUE_4_1(ShdDir2CM, DrawPosXY_VS(), ShdDirM_PS(2, true));
TECHNIQUE_4_1(ShdDir3M, DrawPosXY_VS(), ShdDirM_PS(3, false));   TECHNIQUE_4_1(ShdDir3CM, DrawPosXY_VS(), ShdDirM_PS(3, true));
TECHNIQUE_4_1(ShdDir4M, DrawPosXY_VS(), ShdDirM_PS(4, false));   TECHNIQUE_4_1(ShdDir4CM, DrawPosXY_VS(), ShdDirM_PS(4, true));
TECHNIQUE_4_1(ShdDir5M, DrawPosXY_VS(), ShdDirM_PS(5, false));   TECHNIQUE_4_1(ShdDir5CM, DrawPosXY_VS(), ShdDirM_PS(5, true));
TECHNIQUE_4_1(ShdDir6M, DrawPosXY_VS(), ShdDirM_PS(6, false));   TECHNIQUE_4_1(ShdDir6CM, DrawPosXY_VS(), ShdDirM_PS(6, true));
/******************************************************************************/
Vec4 ShdPnt_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
               NOPERSP Vec2 inPosXY:TEXCOORD1,
               NOPERSP PIXEL):COLOR
{
   return ShadowPointValue(GetPosPoint(inTex, inPosXY), ShadowJitter(pixel.xy), true);
}
#if MODEL>=SM_4
Vec4 ShdPntM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                NOPERSP PIXEL,
                        UInt index  :SV_SampleIndex):COLOR
{
   return ShadowPointValue(GetPosMS(pixel.xy, index, inPosXY), ShadowJitter(pixel.xy), true);
}
#endif
TECHNIQUE    (ShdPnt , DrawPosXY_VS(), ShdPnt_PS ());
TECHNIQUE_4_1(ShdPntM, DrawPosXY_VS(), ShdPntM_PS());
/******************************************************************************/
Vec4 ShdCone_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                NOPERSP Vec2 inPosXY:TEXCOORD1,
                NOPERSP PIXEL):COLOR
{
   return ShadowConeValue(GetPosPoint(inTex, inPosXY), ShadowJitter(pixel.xy), true);
}
#if MODEL>=SM_4
Vec4 ShdConeM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                 NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                 NOPERSP PIXEL,
                         UInt index  :SV_SampleIndex):COLOR
{
   return ShadowConeValue(GetPosMS(pixel.xy, index, inPosXY), ShadowJitter(pixel.xy), true);
}
#endif
TECHNIQUE    (ShdCone , DrawPosXY_VS(), ShdCone_PS());
TECHNIQUE_4_1(ShdConeM, DrawPosXY_VS(), ShdConeM_PS());
/******************************************************************************/
Vec4 ShdBlur_PS(NOPERSP Vec2 inTex:TEXCOORD,
                uniform Int  samples       ):COLOR
{
   Flt  weight=0.25f,
        color =TexPoint(Col, inTex).API(a, r, r)*weight, // DX9 uses A8 while others use R8 RT
        z     =TexDepthPoint(inTex);
   Vec2 dw_mad=DepthWeightMAD(z);
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
      // use linear filtering because texcoords are not rounded
      Flt w=DepthWeight(z-TexDepthLinear(t), dw_mad);
      color +=w*TexLod(Col, t).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
      weight+=w;
   }
   return color/weight;
}
Vec4 ShdBlurX_PS(NOPERSP Vec2 inTex:TEXCOORD,
                 uniform Int  range         ):COLOR
{
   Flt  weight=0.5f,
        color =TexPoint(Col, inTex).API(a, r, r)*weight, // DX9 uses A8 while others use R8 RT
        z     =TexDepthPoint(inTex);
   Vec2 dw_mad=DepthWeightMAD(z), t; t.y=inTex.y;
   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      // use linear filtering because texcoords are not rounded
      t.x=ColSize.x*(2*i+((i>0) ? -0.5f : 0.5f))+inTex.x;
      Flt w=DepthWeight(z-TexDepthLinear(t), dw_mad);
      color +=w*TexLod(Col, t).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
      weight+=w;
   }
   return color/weight;
}
Vec4 ShdBlurY_PS(NOPERSP Vec2 inTex:TEXCOORD,
                 uniform Int  range         ):COLOR
{
   Flt  weight=0.5f,
        color =TexPoint(Col, inTex).API(a, r, r)*weight, // DX9 uses A8 while others use R8 RT
        z     =TexDepthPoint(inTex);
   Vec2 dw_mad=DepthWeightMAD(z), t; t.x=inTex.x;
   UNROLL for(Int i=-range; i<=range; i++)if(i)
   {
      // use linear filtering because texcoords are not rounded
      t.y=ColSize.y*(2*i+((i>0) ? -0.5f : 0.5f))+inTex.y;
      Flt w=DepthWeight(z-TexDepthLinear(t), dw_mad);
      color +=w*TexLod(Col, t).API(a, r, r); // DX9 uses A8 while others use R8 RT, use linear filtering because texcoords aren't rounded
      weight+=w;
   }
   return color/weight;
}
  TECHNIQUE(ShdBlur4 , Draw_VS(), ShdBlur_PS(4));
//TECHNIQUE(ShdBlur5 , Draw_VS(), ShdBlur_PS(5));
  TECHNIQUE(ShdBlur6 , Draw_VS(), ShdBlur_PS(6));
  TECHNIQUE(ShdBlur8 , Draw_VS(), ShdBlur_PS(8));
//TECHNIQUE(ShdBlur9 , Draw_VS(), ShdBlur_PS(9));
  TECHNIQUE(ShdBlur12, Draw_VS(), ShdBlur_PS(12));
//TECHNIQUE(ShdBlur13, Draw_VS(), ShdBlur_PS(13));
//TECHNIQUE(ShdBlurX1, Draw_VS(), ShdBlurX_PS(1));
//TECHNIQUE(ShdBlurY1, Draw_VS(), ShdBlurY_PS(1));
  TECHNIQUE(ShdBlurX2, Draw_VS(), ShdBlurX_PS(2));
  TECHNIQUE(ShdBlurY2, Draw_VS(), ShdBlurY_PS(2));
/******************************************************************************/
// LIGHT
/******************************************************************************/
BUFFER(LightMap)
   Flt LightMapScale =1,
       LightMapSpecular;
   Vec LightMapColAdd  ;
BUFFER_END

Vec4 LightDir_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                 NOPERSP Vec2 inPosXY:TEXCOORD1,
                 uniform Int  shadow           ,
                 uniform Bool quality          ):COLOR
{
   // shadow
   Half shd; if(shadow)shd=TexPoint(Col, inTex).API(a, r, r); // DX9 uses A8 while others use R8 RT

   // diffuse
   VecH4 nrm=GetNormal   (inTex, quality);
   Half  lum=LightDiffuse(nrm.xyz, Light_dir.dir); if(shadow)lum*=shd; clip(lum-EPS_COL);

   // specular
   VecH eye_dir =Normalize    (-Vec(inPosXY, 1));
   Half specular=LightSpecular(nrm.xyz, nrm.w, Light_dir.dir, eye_dir); if(shadow)specular*=shd;

   return Vec4(Light_dir.color.rgb*lum, Light_dir.color.a*specular);
}
#if MODEL>=SM_4
Vec4 LightDirM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                  NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                  NOPERSP PIXEL                      ,
                          UInt index  :SV_SampleIndex,
                  uniform Bool shadow                ,
                  uniform Bool quality               ):COLOR
{
   // shadow
   Half shd; if(shadow)shd=TexSample(ColMS, pixel.xy, index).x;

   // diffuse
   VecH4 nrm=GetNormalMS(pixel.xy, index, quality);
   Half  lum=LightDiffuse(nrm.xyz, Light_dir.dir); if(shadow)lum*=shd; clip(lum-EPS_COL);

   // specular
   VecH eye_dir =Normalize    (-Vec(inPosXY, 1));
   Half specular=LightSpecular(nrm.xyz, nrm.w, Light_dir.dir, eye_dir); if(shadow)specular*=shd;

   return Vec4(Light_dir.color.rgb*lum, Light_dir.color.a*specular);
}
#endif
TECHNIQUE    (LightDir  , DrawPosXY_VS(), LightDir_PS (false, false));
TECHNIQUE    (LightDirS , DrawPosXY_VS(), LightDir_PS (true , false));
TECHNIQUE    (LightDirQ , DrawPosXY_VS(), LightDir_PS (false, true ));
TECHNIQUE    (LightDirSQ, DrawPosXY_VS(), LightDir_PS (true , true ));
TECHNIQUE_4_1(LightDirM , DrawPosXY_VS(), LightDirM_PS(false, false));
TECHNIQUE_4_1(LightDirSM, DrawPosXY_VS(), LightDirM_PS(true , false));
// no Quality version for MSAA
/******************************************************************************/
Vec4 LightPnt_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                 NOPERSP Vec2 inPosXY:TEXCOORD1,
                 uniform Int  shadow           ,
                 uniform Bool quality          ):COLOR
{
   // shadow
   Half shd; if(shadow)
   {
      shd=ShadowFinal(TexPoint(Col, inTex).API(a, r, r)); // DX9 uses A8 while others use R8 RT
      clip(shd-EPS_COL);
   }

   // distance
   Vec  pos      =GetPosPoint(inTex, inPosXY),
        light_dir=Light_point.pos-pos;
   Half power    =LightPointDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormal   (inTex, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return Vec4(Light_point.color.rgb*lum, Light_point.color.a*specular);
}
#if MODEL>=SM_4
Vec4 LightPntM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                  NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                  NOPERSP PIXEL                      ,
                          UInt index  :SV_SampleIndex,
                  uniform Bool shadow                ,
                  uniform Bool quality               ):COLOR
{
   // shadow
   Half shd; if(shadow){shd=ShadowFinal(TexSample(ColMS, pixel.xy, index).x); clip(shd-EPS_COL);}

   // distance
   Vec  pos      =GetPosMS(pixel.xy, index, inPosXY),
        light_dir=Light_point.pos-pos;
   Half power    =LightPointDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormalMS (pixel.xy, index, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return Vec4(Light_point.color.rgb*lum, Light_point.color.a*specular);
}
#endif
TECHNIQUE    (LightPnt  , DrawPosXY_VS(), LightPnt_PS (false, false));
TECHNIQUE    (LightPntS , DrawPosXY_VS(), LightPnt_PS (true , false));
TECHNIQUE    (LightPntQ , DrawPosXY_VS(), LightPnt_PS (false, true ));
TECHNIQUE    (LightPntSQ, DrawPosXY_VS(), LightPnt_PS (true , true ));
TECHNIQUE_4_1(LightPntM , DrawPosXY_VS(), LightPntM_PS(false, false));
TECHNIQUE_4_1(LightPntSM, DrawPosXY_VS(), LightPntM_PS(true , false));
// no Quality version for MSAA
/******************************************************************************/
Vec4 LightSqr_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                 NOPERSP Vec2 inPosXY:TEXCOORD1,
                 uniform Int  shadow           ,
                 uniform Bool quality          ):COLOR
{
   // shadow
   Half shd; if(shadow)
   {
      shd=ShadowFinal(TexPoint(Col, inTex).API(a, r, r)); // DX9 uses A8 while others use R8 RT
      clip(shd-EPS_COL);
   }

   // distance
   Vec  pos      =GetPosPoint(inTex, inPosXY),
        light_dir=Light_sqr.pos-pos;
   Half power    =LightSqrDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormal   (inTex, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return Vec4(Light_sqr.color.rgb*lum, Light_sqr.color.a*specular);
}
#if MODEL>=SM_4
Vec4 LightSqrM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                  NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                  NOPERSP PIXEL                      ,
                          UInt index  :SV_SampleIndex,
                  uniform Bool shadow                ,
                  uniform Bool quality               ):COLOR
{
   // shadow
   Half shd; if(shadow){shd=ShadowFinal(TexSample(ColMS, pixel.xy, index).x); clip(shd-EPS_COL);}

   // distance
   Vec  pos      =GetPosMS(pixel.xy, index, inPosXY),
        light_dir=Light_sqr.pos-pos;
   Half power    =LightSqrDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormalMS (pixel.xy, index, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return Vec4(Light_sqr.color.rgb*lum, Light_sqr.color.a*specular);
}
#endif
TECHNIQUE    (LightSqr  , DrawPosXY_VS(), LightSqr_PS (false, false));
TECHNIQUE    (LightSqrS , DrawPosXY_VS(), LightSqr_PS (true , false));
TECHNIQUE    (LightSqrQ , DrawPosXY_VS(), LightSqr_PS (false, true ));
TECHNIQUE    (LightSqrSQ, DrawPosXY_VS(), LightSqr_PS (true , true ));
TECHNIQUE_4_1(LightSqrM , DrawPosXY_VS(), LightSqrM_PS(false, false));
TECHNIQUE_4_1(LightSqrSM, DrawPosXY_VS(), LightSqrM_PS(true , false));
// no Quality version for MSAA
/******************************************************************************/
Vec4 LightCone_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                  NOPERSP Vec2 inPosXY:TEXCOORD1,
                  uniform Int  shadow           ,
                  uniform Bool quality          ,
                  uniform Bool image            ):COLOR
{
   // shadow
   Half shd; if(shadow)
   {
      shd=ShadowFinal(TexPoint(Col, inTex).API(a, r, r)); // DX9 uses A8 while others use R8 RT
      clip(shd-EPS_COL);
   }

   // distance & angle
   Vec  pos      =GetPosPoint(inTex, inPosXY),
        light_dir=Light_cone.pos-pos,
        dir      =mul(Light_cone.mtrx, light_dir); dir.xy/=dir.z; clip(Vec(1-Abs(dir.xy), dir.z));
   Half power    =LightConeAngle(dir.xy)*LightConeDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormal   (inTex, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   if(image)
   {
      VecH map_col=Tex(Col1, dir.xy*(LightMapScale*0.5f)+0.5f).rgb;
      return Vec4(Light_cone.color.rgb*lum*(map_col+LightMapColAdd), Light_cone.color.a*specular + LightMapSpecular*Max(map_col.rgb)*power);
   }else
   {
      return Vec4(Light_cone.color.rgb*lum, Light_cone.color.a*specular);
   }
}
#if MODEL>=SM_4
Vec4 LightConeM_PS(NOPERSP Vec2 inTex  :TEXCOORD0     ,
                   NOPERSP Vec2 inPosXY:TEXCOORD1     ,
                   NOPERSP PIXEL                      ,
                           UInt index  :SV_SampleIndex,
                   uniform Bool shadow                ,
                   uniform Bool quality               ,
                   uniform Bool image                 ):COLOR
{
   // shadow
   Half shd; if(shadow){shd=ShadowFinal(TexSample(ColMS, pixel.xy, index).x); clip(shd-EPS_COL);}

   // distance & angle
   Vec  pos      =GetPosMS(pixel.xy, index, inPosXY),
        light_dir=Light_cone.pos-pos,
        dir      =mul(Light_cone.mtrx, light_dir); dir.xy/=dir.z; clip(Vec(1-Abs(dir.xy), dir.z));
   Half power    =LightConeAngle(dir.xy)*LightConeDist(light_dir); if(shadow)power*=shd; clip(power-EPS_COL);

   // diffuse
         light_dir=Normalize   (light_dir);
   VecH4 nrm      =GetNormalMS (pixel.xy, index, quality);
   Half  lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   if(image)
   {
      VecH map_col=Tex(Col1, dir.xy*(LightMapScale*0.5f)+0.5f).rgb;
      return Vec4(Light_cone.color.rgb*lum*(map_col+LightMapColAdd), Light_cone.color.a*specular + LightMapSpecular*Max(map_col.rgb)*power);
   }else
   {
      return Vec4(Light_cone.color.rgb*lum, Light_cone.color.a*specular);
   }
}
#endif
TECHNIQUE    (LightCone   , DrawPosXY_VS(), LightCone_PS (false, false, false));
TECHNIQUE    (LightConeS  , DrawPosXY_VS(), LightCone_PS (true , false, false));
TECHNIQUE    (LightConeQ  , DrawPosXY_VS(), LightCone_PS (false, true , false));
TECHNIQUE    (LightConeSQ , DrawPosXY_VS(), LightCone_PS (true , true , false));
TECHNIQUE    (LightConeI  , DrawPosXY_VS(), LightCone_PS (false, false, true ));
TECHNIQUE    (LightConeSI , DrawPosXY_VS(), LightCone_PS (true , false, true ));
TECHNIQUE    (LightConeQI , DrawPosXY_VS(), LightCone_PS (false, true , true ));
TECHNIQUE    (LightConeSQI, DrawPosXY_VS(), LightCone_PS (true , true , true ));
TECHNIQUE_4_1(LightConeM  , DrawPosXY_VS(), LightConeM_PS(false, false, false));
TECHNIQUE_4_1(LightConeSM , DrawPosXY_VS(), LightConeM_PS(true , false, false));
TECHNIQUE_4_1(LightConeIM , DrawPosXY_VS(), LightConeM_PS(false, false, true ));
TECHNIQUE_4_1(LightConeSIM, DrawPosXY_VS(), LightConeM_PS(true , false, true ));
// no Quality version for MSAA
/******************************************************************************/
// COL LIGHT
/******************************************************************************/
BUFFER(ColLight)
   VecH NightShadeColor;
BUFFER_END

Flt CelShade(Flt lum) {return TexLod(Det1, Vec2(lum, 0.5f)).x;} // have to use linear filtering

Vec ColLight(Vec4 color, Vec4 lum, Flt ao, Vec night_shade_col,
     uniform Bool    ao_do   ,
     uniform Bool   cel_shade,
     uniform Bool night_shade)
{
   // treat glow as if it's a light source, this will have 2 effects: 1) pixels will have color even without any lights 2) this will disable night shade effects and retain original color (not covered by night shade), this is because 'night_shade_intensity' is multiplied by "Sat(1-max_lum)"
#if 0 // simply adding doesn't provide good results
   lum.rgb+=color.w;
#else // instead lerp to 1, to avoid glow pixels getting too bright, because if color is mostly red (255, 40, 20), but if too much light is applied, then it could become more white 10*(255, 40, 20)=(2550, 400, 200), and we want pixels to glow with exact color as on the texture
   #if 0 // slower
      lum.rgb=Lerp(lum.rgb, 1, color.w);
   #else // faster
      lum.rgb=lum.rgb*(1-color.w) + color.w;
   #endif
#endif
   Flt max_lum=Max(lum.rgb);
   if(cel_shade)
   {
      max_lum=CelShade(max_lum);
      lum.rgb=max_lum;
   }
   Flt spec   =lum.w/Max(max_lum, EPS);
   Vec col_lit=(color.rgb+spec)*lum.rgb;
   if(night_shade)
   {
      Flt night_shade_intensity=Sat(1-max_lum)                 // only for low light
                               *SRGBLumOfSRGBColor(color.rgb); // set based on unlit color luminance
      if(ao_do)night_shade_intensity*=ao;
      col_lit+=night_shade_intensity*night_shade_col;
   }
   return col_lit;
}
Vec4 ColLight_PS(NOPERSP Vec2 inTex:TEXCOORD   ,
                 NOPERSP PIXEL,
                 uniform Int  multi_sample     ,
                 uniform Bool    ao_do   =false,
                 uniform Bool   cel_shade=false,
                 uniform Bool night_shade=false):COLOR
{
   Flt ao; Vec ambient; if(ao_do){ao=TexLod(Det, inTex).API(a, r, r); ambient=AmbColor*ao;} // use 'TexLod' because AO can be of different size and we need to use tex filtering, DX9 uses A8 while others use R8 RT
#if MODEL>=SM_4
   VecI p=VecI(pixel.xy, 0);
   if(multi_sample)
   {
      if(multi_sample==1) // 1 sample
      {
         Vec4     color=TexSample(ColMS, pixel.xy, 0),
                  lum  =TexSample(LumMS, pixel.xy, 0), // needed because Mesh Ambient is stored only in Multi Sampled Lum
                  lum1s=          Lum  .Load(p);
                  lum      +=lum1s  ;
         if(ao_do)lum  .rgb+=ambient;
                  color.rgb =ColLight(color, lum, ao, NightShadeColor, ao_do, cel_shade, night_shade);
         return   color;
      }else // n samples
      {
         Vec4 color_sum=0;
         Flt  valid_samples=EPS;
         Vec  night_shade_col=NightShadeColor; if(night_shade && ao_do)night_shade_col*=ao; // compute it once, and not inside 'ColLight'
         UNROLL for(Int i=0; i<MS_SAMPLES; i++)if(DEPTH_FOREGROUND(TexDepthMSRaw(pixel.xy, i))) // valid sample
         {
            Vec4     color=TexSample(ColMS, pixel.xy, i),
                     lum  =TexSample(LumMS, pixel.xy, i);
            if(ao_do)lum  .rgb+=ambient;
                     color.rgb =ColLight(color, lum, ao, night_shade_col, false, cel_shade, night_shade); // we've already adjusted 'night_shade_col' by 'ao', so set 'ao_do' as false
            color_sum+=color;
            valid_samples++;
         }
         return color_sum/valid_samples; // MS_SAMPLES
      }
   }else
   {
      Vec4     color=Col.Load(p),
               lum  =Lum.Load(p);
      if(ao_do)lum  .rgb+=ambient;
               color.rgb =ColLight(color, lum, ao, NightShadeColor, ao_do, cel_shade, night_shade);
      return   color;
   }
#else
   Vec4     color=TexPoint(Col, inTex),
            lum  =TexPoint(Lum, inTex);
   if(ao_do)lum  .rgb+=ambient;
            color.rgb =ColLight(color, lum, ao, NightShadeColor, ao_do, cel_shade, night_shade);
   return   color;
#endif
}
TECHNIQUE(ColLight0   , Draw_VS(), ColLight_PS(0));
TECHNIQUE(ColLight0A  , Draw_VS(), ColLight_PS(0, true));
TECHNIQUE(ColLight0C  , Draw_VS(), ColLight_PS(0, false, true));
TECHNIQUE(ColLight0AC , Draw_VS(), ColLight_PS(0, true , true));
TECHNIQUE(ColLight0N  , Draw_VS(), ColLight_PS(0, false, false, true));
TECHNIQUE(ColLight0AN , Draw_VS(), ColLight_PS(0, true , false, true));
TECHNIQUE(ColLight0CN , Draw_VS(), ColLight_PS(0, false, true , true));
TECHNIQUE(ColLight0ACN, Draw_VS(), ColLight_PS(0, true , true , true));
#if MODEL>=SM_4 // multi sample
TECHNIQUE(ColLight1   , Draw_VS(), ColLight_PS(1));
TECHNIQUE(ColLight1A  , Draw_VS(), ColLight_PS(1, true));
TECHNIQUE(ColLight1C  , Draw_VS(), ColLight_PS(1, false, true));
TECHNIQUE(ColLight1AC , Draw_VS(), ColLight_PS(1, true , true));
TECHNIQUE(ColLight1N  , Draw_VS(), ColLight_PS(1, false, false, true));
TECHNIQUE(ColLight1AN , Draw_VS(), ColLight_PS(1, true , false, true));
TECHNIQUE(ColLight1CN , Draw_VS(), ColLight_PS(1, false, true , true));
TECHNIQUE(ColLight1ACN, Draw_VS(), ColLight_PS(1, true , true , true));

TECHNIQUE(ColLight2   , Draw_VS(), ColLight_PS(2));
TECHNIQUE(ColLight2A  , Draw_VS(), ColLight_PS(2, true));
TECHNIQUE(ColLight2C  , Draw_VS(), ColLight_PS(2, false, true));
TECHNIQUE(ColLight2AC , Draw_VS(), ColLight_PS(2, true , true));
TECHNIQUE(ColLight2N  , Draw_VS(), ColLight_PS(2, false, false, true));
TECHNIQUE(ColLight2AN , Draw_VS(), ColLight_PS(2, true , false, true));
TECHNIQUE(ColLight2CN , Draw_VS(), ColLight_PS(2, false, true , true));
TECHNIQUE(ColLight2ACN, Draw_VS(), ColLight_PS(2, true , true , true));
#endif
/******************************************************************************/
// PARTICLE
/******************************************************************************/
#define ANIM_NONE   0
#define ANIM_YES    1
#define ANIM_SMOOTH 2

BUFFER(Particle)
   Vec2 ParticleFrames=Vec2(1, 1);
BUFFER_END

void Particle_VS(VtxInput vtx,
             out Vec4  outVtx :POSITION ,
             out VecH4 outCol :COLOR    ,
             out Vec2  outTex :TEXCOORD0,
             out Vec2  outZS  :TEXCOORD1,
             out Vec   outAnim:TEXCOORD2,
         uniform Bool  palette          ,
         uniform Bool  soft             ,
         uniform Int   anim             ,
         uniform Bool  motion_stretch   ,
         uniform Bool  stretch_alpha    )
{
   outTex=vtx.tex();
   outCol=vtx.color();

   Flt  size  =vtx.size(),
        angle =vtx._tan.w;
   Vec  pos   =TransformPos(vtx.pos());
   Vec2 offset=outTex*Vec2(2, -2)+Vec2(-1, 1), cos_sin; CosSin(cos_sin.x, cos_sin.y, angle); offset=Rotate(offset, cos_sin);

   if(motion_stretch)
      if(pos.z>0)
   {
      #define PARTICLE_PROJECT 100
      Vec  vel =TransformDir(vtx.tan()); if(vel.z<0)vel=-vel; // view space velocity, always make it along the camera direction, so we won't have a situation where the 'pos1' is behind the camera
      Vec  pos1=pos+vel/PARTICLE_PROJECT;
      Vec2 vel2=(pos1.xy/pos1.z - pos.xy/pos.z)*PARTICLE_PROJECT; // 2D velocity
      Flt  len =Length(vel2)+EPS;
    //if(len>0) // instead of using "if", add EPS line above - it's faster
      {
       //Flt  max_stretch=5; if(len>max_stretch){vel2*=max_stretch/len; len=max_stretch;} // NaN
         Vec2 x=vel2*(vel2.x/len),
              y=vel2*(vel2.y/len);
         offset=Vec2(offset.x*(x.x+1) + offset.y*y.x, offset.x*x.y + offset.y*(y.y+1));
         if(stretch_alpha)
         {
            if(palette)outCol  /=1+len; // in RM_PALETTE each component
            else       outCol.a/=1+len; // in RM_BLEND   only alpha
         }
      }
   }
   pos.xy+=offset*size;

   // sky
   Flt d=Length(pos), opacity=Sat(d*SkyFracMulAdd.x + SkyFracMulAdd.y);
   if(palette)outCol  *=opacity; // in RM_PALETTE each component
   else       outCol.a*=opacity; // in RM_BLEND   only alpha

   if(soft)
   {
      outZS.x=pos.z;
      outZS.y=size;

      if(pos.z >= -size)
      {
         Flt wanted_z=Max(Viewport.from+EPS, pos.z-size),
             scale   =wanted_z;
    if(pos.z)scale  /=pos   .z; // NaN
             pos.xyz*=scale;
      }
   }
   if(anim!=ANIM_NONE)
   {
      Flt frame=vtx.tex1().x;
   #if 0 // integer version
      #if MODEL>=SM_4
         #define INT UInt
      #else
         #define INT Int
      #endif
         INT frames=ParticleFrames.x*ParticleFrames.y;
         INT f     =INT(frame)%frames; // Trunc(frame)%frames; don't know why but doesn't work correctly
         if(anim==ANIM_SMOOTH) // frame blending
         {
            INT f1=(f+1)%frames;
            outAnim.xy =outTex;
            outAnim.z  =Frac(frame);
            outAnim.x +=f1%INT(ParticleFrames.x);
            outAnim.y +=f1/INT(ParticleFrames.x);
            outAnim.xy/=       ParticleFrames   ;
         }
         outTex.x+=f%INT(ParticleFrames.x);
         outTex.y+=f/INT(ParticleFrames.x);
         outTex  /=      ParticleFrames   ;
      #undef INT
   #else // float version
      Flt frames=ParticleFrames.x*ParticleFrames.y; frame=Frac(frame/frames)*frames; // frame=[0..frames)
      Flt f; frame=modf(frame, f);
      if(anim==ANIM_SMOOTH) // frame blending
      {
         Flt f1=f+1; if(f1+0.5f>=frames)f1=0; // f1=(f+1)%frames;
                outAnim.xy =outTex;
                outAnim.z  =frame ; // frame step [0..1)
         Flt y; outAnim.x +=ParticleFrames.x*modf(f1/ParticleFrames.x, y); // outAnim.x+=f1%INT(ParticleFrames.x);
                outAnim.y +=y                                            ; // outAnim.y+=f1/INT(ParticleFrames.x);
                outAnim.xy/=ParticleFrames                               ;
      }
      Flt y; outTex.x+=ParticleFrames.x*modf(f/ParticleFrames.x, y); // outTex.x+=f%INT(ParticleFrames.x);
             outTex.y+=y                                           ; // outTex.y+=f/INT(ParticleFrames.x);
             outTex  /=ParticleFrames                              ;
   #endif
   }
#if DX9 || GL // DX10+ should support all sizes, Image.partial
                        outTex    *=ColSize.xy;
   if(anim==ANIM_SMOOTH)outAnim.xy*=ColSize.xy;
#endif
   outVtx=Project(pos);
}
/******************************************************************************/
Vec4 Particle_PS(PIXEL,
                 VecH4 inCol :COLOR    ,
                 Vec2  inTex :TEXCOORD0,
                 Vec2  inZS  :TEXCOORD1,
                 Vec   inAnim:TEXCOORD2,
         uniform Bool  palette         ,
         uniform Bool  soft            ,
         uniform Int   anim            ):COLOR
{
   VecH4                tex=          Tex(Col, inTex    )           ;
   if(anim==ANIM_SMOOTH)tex=Lerp(tex, Tex(Col, inAnim.xy), inAnim.z);
   if(soft)
   {
      Flt  z0    =inZS.x-tex.a*inZS.y,
           z1    =inZS.x+tex.a*inZS.y;
           tex.a*=Sat((TexDepthPoint(PIXEL_TO_SCREEN)-z0)/inZS.y); // fade out at occluder
           tex.a*=Sat(z1/(z1-z0+EPS));                             // smooth visibility fraction when particle near (or behind) camera, NaN
   }
   if(palette)return inCol*tex.a;
   else       return inCol*tex  ;
}
/******************************************************************************/
TECHNIQUE(Bilb            , Particle_VS(false, false, ANIM_NONE  , false, false), Particle_PS(false, false, ANIM_NONE  ));

TECHNIQUE(ParticleTex     , Particle_VS(false, false, ANIM_NONE  , true , false), Particle_PS(false, false, ANIM_NONE  ));
TECHNIQUE(ParticleTexP    , Particle_VS(true , false, ANIM_NONE  , true , false), Particle_PS(true , false, ANIM_NONE  ));
TECHNIQUE(ParticleTexS    , Particle_VS(false, true , ANIM_NONE  , true , false), Particle_PS(false, true , ANIM_NONE  ));
TECHNIQUE(ParticleTexPS   , Particle_VS(true , true , ANIM_NONE  , true , false), Particle_PS(true , true , ANIM_NONE  ));
TECHNIQUE(ParticleTexA    , Particle_VS(false, false, ANIM_YES   , true , false), Particle_PS(false, false, ANIM_YES   ));
TECHNIQUE(ParticleTexPA   , Particle_VS(true , false, ANIM_YES   , true , false), Particle_PS(true , false, ANIM_YES   ));
TECHNIQUE(ParticleTexSA   , Particle_VS(false, true , ANIM_YES   , true , false), Particle_PS(false, true , ANIM_YES   ));
TECHNIQUE(ParticleTexPSA  , Particle_VS(true , true , ANIM_YES   , true , false), Particle_PS(true , true , ANIM_YES   ));
TECHNIQUE(ParticleTexAA   , Particle_VS(false, false, ANIM_SMOOTH, true , false), Particle_PS(false, false, ANIM_SMOOTH));
TECHNIQUE(ParticleTexPAA  , Particle_VS(true , false, ANIM_SMOOTH, true , false), Particle_PS(true , false, ANIM_SMOOTH));
TECHNIQUE(ParticleTexSAA  , Particle_VS(false, true , ANIM_SMOOTH, true , false), Particle_PS(false, true , ANIM_SMOOTH));
TECHNIQUE(ParticleTexPSAA , Particle_VS(true , true , ANIM_SMOOTH, true , false), Particle_PS(true , true , ANIM_SMOOTH));

TECHNIQUE(ParticleTexM    , Particle_VS(false, false, ANIM_NONE  , true , true ), Particle_PS(false, false, ANIM_NONE  ));
TECHNIQUE(ParticleTexPM   , Particle_VS(true , false, ANIM_NONE  , true , true ), Particle_PS(true , false, ANIM_NONE  ));
TECHNIQUE(ParticleTexSM   , Particle_VS(false, true , ANIM_NONE  , true , true ), Particle_PS(false, true , ANIM_NONE  ));
TECHNIQUE(ParticleTexPSM  , Particle_VS(true , true , ANIM_NONE  , true , true ), Particle_PS(true , true , ANIM_NONE  ));
TECHNIQUE(ParticleTexAM   , Particle_VS(false, false, ANIM_YES   , true , true ), Particle_PS(false, false, ANIM_YES   ));
TECHNIQUE(ParticleTexPAM  , Particle_VS(true , false, ANIM_YES   , true , true ), Particle_PS(true , false, ANIM_YES   ));
TECHNIQUE(ParticleTexSAM  , Particle_VS(false, true , ANIM_YES   , true , true ), Particle_PS(false, true , ANIM_YES   ));
TECHNIQUE(ParticleTexPSAM , Particle_VS(true , true , ANIM_YES   , true , true ), Particle_PS(true , true , ANIM_YES   ));
TECHNIQUE(ParticleTexAAM  , Particle_VS(false, false, ANIM_SMOOTH, true , true ), Particle_PS(false, false, ANIM_SMOOTH));
TECHNIQUE(ParticleTexPAAM , Particle_VS(true , false, ANIM_SMOOTH, true , true ), Particle_PS(true , false, ANIM_SMOOTH));
TECHNIQUE(ParticleTexSAAM , Particle_VS(false, true , ANIM_SMOOTH, true , true ), Particle_PS(false, true , ANIM_SMOOTH));
TECHNIQUE(ParticleTexPSAAM, Particle_VS(true , true , ANIM_SMOOTH, true , true ), Particle_PS(true , true , ANIM_SMOOTH));
/******************************************************************************/
Vec4 PaletteDraw_PS(NOPERSP Vec2 inTex:TEXCOORD):COLOR
{
   Vec4 particle=TexLod(Col, inTex); // use linear filtering in case in the future we support downsized palette intensities (for faster fill-rate)
   clip(Length2(particle)-Sqr(EPS_COL)); // 'clip' is faster than "BRANCH if(Length2(particle)>Sqr(EPS_COL))" (branch is however slightly faster when entire majority of pixels have some effect, however in most cases majority of pixels doesn't have anything so stick with 'clip')

   // have to use linear filtering because this is palette image
   Vec4 c0=TexLod(Col1, Vec2(particle.x, 0.5f/4)),
        c1=TexLod(Col1, Vec2(particle.y, 1.5f/4)),
        c2=TexLod(Col1, Vec2(particle.z, 2.5f/4)),
        c3=TexLod(Col1, Vec2(particle.w, 3.5f/4));
   Flt  a =Max(c0.a, c1.a, c2.a, c3.a);

   return Vec4((c0.rgb*c0.a
               +c1.rgb*c1.a
               +c2.rgb*c2.a
               +c3.rgb*c3.a)/(a+EPS), a); // NaN
}
TECHNIQUE(PaletteDraw, Draw_VS(), PaletteDraw_PS());
/******************************************************************************/
// DECAL
/******************************************************************************/
BUFFER(Decal)
   Vec DecalParams; // x=OpaqueFracMul, y=OpaqueFracAdd, z=alpha
BUFFER_END

inline Flt DecalOpaqueFracMul() {return DecalParams.x;}
inline Flt DecalOpaqueFracAdd() {return DecalParams.y;}
inline Flt DecalAlpha        () {return DecalParams.z;}

void Decal_VS(VtxInput vtx,
          out Vec4    outVtx    :POSITION ,
          out Matrix  outMatrix :TEXCOORD0,
          out Matrix3 outMatrixN:TEXCOORD3,
      uniform Bool    fullscreen          ,
      uniform Bool    normal              ,
      uniform Bool    palette             )
{
   outMatrix=ViewMatrix[0];
   outMatrix[0]/=Length2(outMatrix[0]);
   outMatrix[1]/=Length2(outMatrix[1]);
   outMatrix[2]/=Length2(outMatrix[2]);

   if(!palette && normal)
   {
      outMatrixN[0]=Normalize(outMatrix[0]);
      outMatrixN[1]=Normalize(outMatrix[1]);
      outMatrixN[2]=Normalize(outMatrix[2]);
   }

   if(fullscreen)
   {
      outVtx=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   }else
   {
      outVtx=Project(TransformPos(vtx.pos()));
   }
}
Vec4 Decal_PS(PIXEL,
              Matrix  inMatrix :TEXCOORD0,
              Matrix3 inMatrixN:TEXCOORD3,
          out Vec4    outNrm   :COLOR1   ,
      uniform Bool    normal             ,
      uniform Bool    palette            ):COLOR
{
   Vec pos  =GetPosPoint(PIXEL_TO_SCREEN);
       pos  =mul((Matrix3)inMatrix, pos-inMatrix[3]);
   Flt alpha=Sat(Abs(pos.z)*DecalOpaqueFracMul()+DecalOpaqueFracAdd());
 
   clip(Vec(1-Abs(pos.xy), alpha-EPS_COL));
   alpha*=DecalAlpha();

   pos.xy=pos.xy*0.5f+0.5f;

   Vec4 col=Tex(Col, pos.xy);

   if(palette)
   {
      return col.a*(Color[0]*MaterialColor())*alpha;
   }else
   {
      if(normal)
      {
         Vec4 tex_nrm =Tex(Nrm, pos.xy); // #MaterialTextureChannelOrder
         Flt  specular=tex_nrm.z*MaterialSpecular(); // specular is in 'nrm.z'

               Vec nrm;
                   nrm.xy =(tex_nrm.xy*2-1)*MaterialRough(); // normal is in 'nrm.xy'
       //if(detail)nrm.xy+=det.xy;
                   nrm.z  =CalcZ(nrm.xy);
                   nrm    =Transform(nrm, inMatrixN);

         col.a =tex_nrm.w; // alpha is in 'nrm.w'
         col  *=Color[0]*MaterialColor();
         col.a*=alpha;

      #if SIGNED_NRM_RT
         outNrm.xyz=nrm;
      #else
         outNrm.xyz=nrm*0.5f+0.5f;
      #endif
         outNrm.w=col.a; // alpha
      }else
      {
         col  *=Color[0]*MaterialColor();
         col.a*=alpha;
      }
      return col;
   }
}
TECHNIQUE(Decal  , Decal_VS(false, false, false), Decal_PS(false, false));
TECHNIQUE(DecalN , Decal_VS(false, true , false), Decal_PS(true , false));
TECHNIQUE(DecalP , Decal_VS(false, false, true ), Decal_PS(false, true ));
TECHNIQUE(DecalF , Decal_VS(true , false, false), Decal_PS(false, false));
TECHNIQUE(DecalFN, Decal_VS(true , true , false), Decal_PS(true , false));
TECHNIQUE(DecalFP, Decal_VS(true , false, true ), Decal_PS(false, true ));
/******************************************************************************/
// BLOOM
/******************************************************************************/
BUFFER(Bloom)
   VecH BloomParams; // x=original, y=scale, z=cut
BUFFER_END

void BloomDS_VS(VtxInput vtx,
            out Vec2 outTex:TEXCOORD,
            out Vec4 outVtx:POSITION,
        uniform Bool glow           ,
        uniform Bool do_clamp       ,
        uniform Bool half           )
{
   outTex=vtx.tex (); if(glow)outTex-=ColSize.xy*Vec2(half ? 0.5f : 1.5f, half ? 0.5f : 1.5f);
   outVtx=vtx.pos4(); AdjustPixelCenter(outVtx);
}
inline VecH BloomColor(VecH color, uniform Bool saturate)
{
   if(saturate)
   {
      return color*BloomParams.y+BloomParams.z;
   }else
   {
      Half col_lum=Max(color), lum=col_lum*BloomParams.y+BloomParams.z;
      return (lum>0) ? color*(lum/col_lum) : VecH(0, 0, 0);
   }
}
Vec4 BloomDS_PS(NOPERSP Vec2 inTex:TEXCOORD,
                uniform Bool glow          ,
                uniform Bool do_clamp      ,
                uniform Bool half          ,
                uniform Bool saturate      ):COLOR // "Max(0, " of the result is not needed because we're rendering to 1 byte per channel RT
{
   if(glow)
   {
      const Int res=(half ? 2 : 4);

      Vec  color=0;
      Vec4 glow =0;
      UNROLL for(Int y=0; y<res; y++)
      UNROLL for(Int x=0; x<res; x++)
      {
         Vec4    c=TexLod(Col, UVClamp(inTex+ColSize.xy*Vec2(x, y), do_clamp)); // can't use 'TexPoint' because 'Col' can be supersampled
         color   +=c.rgb;
         glow.rgb+=c.rgb*c.a;
         glow.a   =Max(glow.a, c.a);
      }
      glow.rgb*=2*glow.a/Max(Vec4(glow.rgb, EPS)); // NaN (increase by 2 because normally it's too small)
      return Vec4(Max(BloomColor(color, saturate), glow.rgb), 0);
   }else
   {
      if(half)
      {
         return Vec4(BloomColor(TexLod(Col, UVClamp(inTex, do_clamp)).rgb, saturate), 0);
      }else
      {
         Vec2 tex_min=UVClamp(inTex-ColSize.xy, do_clamp),
              tex_max=UVClamp(inTex+ColSize.xy, do_clamp);
         return Vec4(BloomColor(TexLod(Col, Vec2(tex_min.x, tex_min.y)).rgb
                               +TexLod(Col, Vec2(tex_max.x, tex_min.y)).rgb
                               +TexLod(Col, Vec2(tex_min.x, tex_max.y)).rgb
                               +TexLod(Col, Vec2(tex_max.x, tex_max.y)).rgb, saturate), 0);
      }
   }
}
TECHNIQUE(BloomDS    , BloomDS_VS(false, false, false), BloomDS_PS(false, false, false, false));
TECHNIQUE(BloomDSC   , BloomDS_VS(false, true , false), BloomDS_PS(false, true , false, false));
TECHNIQUE(BloomDSH   , BloomDS_VS(false, false, true ), BloomDS_PS(false, false, true , false));
TECHNIQUE(BloomDSCH  , BloomDS_VS(false, true , true ), BloomDS_PS(false, true , true , false));
TECHNIQUE(BloomGDS   , BloomDS_VS(true , false, false), BloomDS_PS(true , false, false, false));
TECHNIQUE(BloomGDSC  , BloomDS_VS(true , true , false), BloomDS_PS(true , true , false, false));
TECHNIQUE(BloomGDSH  , BloomDS_VS(true , false, true ), BloomDS_PS(true , false, true , false));
TECHNIQUE(BloomGDSCH , BloomDS_VS(true , true , true ), BloomDS_PS(true , true , true , false));
TECHNIQUE(BloomDSS   , BloomDS_VS(false, false, false), BloomDS_PS(false, false, false, true ));
TECHNIQUE(BloomDSCS  , BloomDS_VS(false, true , false), BloomDS_PS(false, true , false, true ));
TECHNIQUE(BloomDSHS  , BloomDS_VS(false, false, true ), BloomDS_PS(false, false, true , true ));
TECHNIQUE(BloomDSCHS , BloomDS_VS(false, true , true ), BloomDS_PS(false, true , true , true ));
TECHNIQUE(BloomGDSS  , BloomDS_VS(true , false, false), BloomDS_PS(true , false, false, true ));
TECHNIQUE(BloomGDSCS , BloomDS_VS(true , true , false), BloomDS_PS(true , true , false, true ));
TECHNIQUE(BloomGDSHS , BloomDS_VS(true , false, true ), BloomDS_PS(true , false, true , true ));
TECHNIQUE(BloomGDSCHS, BloomDS_VS(true , true , true ), BloomDS_PS(true , true , true , true ));
/******************************************************************************/
Vec4 Bloom_PS(NOPERSP Vec2 inTex:TEXCOORD,
              NOPERSP PIXEL              ,
              uniform Bool dither        ):COLOR // Saturation of the result is not needed because we're rendering to 1 byte per channel RT
{
   // final=src*original + Sat((src-cut)*scale)
   VecH col=TexLod(Col , inTex).rgb*BloomParams.x // can't use 'TexPoint' because 'Col'  can be supersampled
           +TexLod(Col1, inTex).rgb;              // can't use 'TexPoint' because 'Col1' can be smaller
   if(dither)col+=DitherValueColor(pixel);
   return Vec4(col, 1); // force full alpha so back buffer effects can work ok
}
TECHNIQUE(Bloom , Draw_VS(), Bloom_PS(false));
TECHNIQUE(BloomD, Draw_VS(), Bloom_PS(true ));
/******************************************************************************/
// FXAA
/******************************************************************************/
#define FxaaTex Image

#if MODEL>=SM_4
   #define FxaaInt2 VecI2
   #define FxaaTexTop(t, p      ) t.SampleLevel(SamplerLinearClamp, p, 0)
   #define FxaaTexOff(t, p, o, r) t.SampleLevel(SamplerLinearClamp, p, 0, o)
#else
   #define FxaaInt2 Vec2
   #define FxaaTexTop(t, p      ) TexLod(t, p)
   #define FxaaTexOff(t, p, o, r) TexLod(t, p+o*ColSize.xy)
#endif

#include "FXAA_config.h"
#include "FXAA.h"

Vec4 FXAA_PS(NOPERSP Vec2 pos:TEXCOORD):COLOR
{
   return FxaaPixelShader(pos, 0, Col, Col, Col, ColSize.xy, 0, 0, 0, 0.475f, 0.15f, 0.0833f, 8.0, 0.125, 0.05, Vec4(1.0, -1.0, 0.25, -0.25));
}
TECHNIQUE(FXAA, Draw_VS(), FXAA_PS());
/******************************************************************************
// MLAA
Copyright (C) 2011 Jorge Jimenez (jorge@iryoku.com)
Copyright (C) 2011 Belen Masia (bmasia@unizar.es) 
Copyright (C) 2011 Jose I. Echevarria (joseignacioechevarria@gmail.com) 
Copyright (C) 2011 Fernando Navarro (fernandn@microsoft.com) 
Copyright (C) 2011 Diego Gutierrez (diegog@unizar.es)
All rights reserved.
/******************************************************************************
#define MLAA_MAX_SEARCH_STEPS 6
#define MLAA_MAX_DISTANCE     32
#define MLAA_THRESHOLD        0.1f

Vec2 MLAAArea(Vec2 distance, Flt e1, Flt e2)
{
   Flt  areaSize=MLAA_MAX_DISTANCE*5;
   Vec2 pixcoord=MLAA_MAX_DISTANCE*Round(4*Vec2(e1, e2))+distance;
   Vec2 texcoord=pixcoord/(areaSize-1);
   return TexLod(Col1, texcoord).rg; // AreaMap
}
void MLAA_VS(VtxInput vtx,
         out Vec2 outTex         :TEXCOORD0,
         out Vec4 outTexOffset[2]:TEXCOORD1,
         out Vec4 outVtx         :POSITION )
{
   outVtx         =Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(outVtx); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   outTex         =vtx.tex();
   outTexOffset[0]=ColSize.xyxy*Vec4(-1, 0, 0,-1)+outTex.xyxy;
   outTexOffset[1]=ColSize.xyxy*Vec4( 1, 0, 0, 1)+outTex.xyxy;
}
Vec4 MLAAEdge_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
                 NOPERSP Vec4 offset[2]:TEXCOORD1):COLOR
{
   Flt L      =Dot(TexPoint(Col, texcoord    ).rgb, ColorLumWeight2);
   Flt Lleft  =Dot(TexPoint(Col, offset[0].xy).rgb, ColorLumWeight2);
   Flt Ltop   =Dot(TexPoint(Col, offset[0].zw).rgb, ColorLumWeight2);  
   Flt Lright =Dot(TexPoint(Col, offset[1].xy).rgb, ColorLumWeight2);
   Flt Lbottom=Dot(TexPoint(Col, offset[1].zw).rgb, ColorLumWeight2);

   Vec4 delta=Abs(L.xxxx-Vec4(Lleft, Ltop, Lright, Lbottom));
   Vec4 edges=step(Vec4(MLAA_THRESHOLD, MLAA_THRESHOLD, MLAA_THRESHOLD, MLAA_THRESHOLD), delta);

   if(Dot(edges, 1)==0)discard;

   return edges;
}

#if MODEL!=SM_GL
Flt MLAASearchXLeft (Vec2 texcoord) {Flt i, e=0; for(i=-1.5f; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(i, 0)).g; FLATTEN if(e<0.9f)break;} return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchXRight(Vec2 texcoord) {Flt i, e=0; for(i= 1.5f; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(i, 0)).g; FLATTEN if(e<0.9f)break;} return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYUp   (Vec2 texcoord) {Flt i, e=0; for(i=-1.5f; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(0, i)).r; FLATTEN if(e<0.9f)break;} return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYDown (Vec2 texcoord) {Flt i, e=0; for(i= 1.5f; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(0, i)).r; FLATTEN if(e<0.9f)break;} return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);}
#else // CG doesn't properly compile "break"
Flt MLAASearchXLeft (Vec2 texcoord) {Flt i, e=0; for(i=-1.5f; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(i, 0)).g; if(e<0.9f)return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);} return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchXRight(Vec2 texcoord) {Flt i, e=0; for(i= 1.5f; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(i, 0)).g; if(e<0.9f)return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);} return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYUp   (Vec2 texcoord) {Flt i, e=0; for(i=-1.5f; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(0, i)).r; if(e<0.9f)return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);} return Max(i+1.5f-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYDown (Vec2 texcoord) {Flt i, e=0; for(i= 1.5f; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Col, texcoord+ColSize.xy*Vec2(0, i)).r; if(e<0.9f)return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);} return Min(i-1.5f+2*e,  2*MLAA_MAX_SEARCH_STEPS);}
#endif

Vec4 MLAABlend_PS(NOPERSP Vec2 texcoord:TEXCOORD):COLOR
{
   Vec4 areas=0;
   Vec2 e=TexPoint(Col, texcoord).rg;

   BRANCH if(e.g) // Edge at north
   {
      Vec2 d     =Vec2(MLAASearchXLeft(texcoord), MLAASearchXRight(texcoord)); // Search distances to the left and to the right
      Vec4 coords=Vec4(d.x, -0.25f, d.y+1, -0.25f)*ColSize.xyxy+texcoord.xyxy; // Now fetch the crossing edges. Instead of sampling between edgels, we sample at -0.25, to be able to discern what value has each edge
      Flt  e1=TexLod(Col, coords.xy).r,
           e2=TexLod(Col, coords.zw).r;
      areas.rg=MLAAArea(Abs(d), e1, e2); // Ok, we know how this pattern looks like, now it is time for getting the actual area
   }

   BRANCH if(e.r) // Edge at west
   {
      Vec2 d     =Vec2(MLAASearchYUp(texcoord), MLAASearchYDown(texcoord)); // Search distances to the top and to the bottom
      Vec4 coords=Vec4(-0.25f, d.x, -0.25f, d.y+1)*ColSize.xyxy+texcoord.xyxy; // Now fetch the crossing edges (yet again)
      Flt  e1=TexLod(Col, coords.xy).g,
           e2=TexLod(Col, coords.zw).g;
      areas.ba=MLAAArea(Abs(d), e1, e2); // Get the area for this direction
   }

   return areas;
}
Vec4 MLAA_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
             NOPERSP Vec4 offset[2]:TEXCOORD1):COLOR
{
   // Fetch the blending weights for current pixel:
   Vec4 topLeft=TexPoint(Col1, texcoord);
   Flt  bottom =TexPoint(Col1, offset[1].zw).g,
        right  =TexPoint(Col1, offset[1].xy).a;
   Vec4 a      =Vec4(topLeft.r, bottom, topLeft.b, right),
        w      =a*a*a; // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform a weighted average, where the weight of each line is 'a' cubed, which favors blending and works well in practice.

   Flt sum=Dot(w, 1); // There is some blending weight with a value greater than 0?
   if( sum<1e-5)discard;

   // Add the contributions of the possible 4 lines that can cross this pixel:
#if 1 // use Bilinear Filtering to speedup calculations
   Vec4 color=TexLod(Col, texcoord-Vec2(0, a.r*ColSize.y))*w.r
             +TexLod(Col, texcoord+Vec2(0, a.g*ColSize.y))*w.g
             +TexLod(Col, texcoord-Vec2(a.b*ColSize.x, 0))*w.b
             +TexLod(Col, texcoord+Vec2(a.a*ColSize.x, 0))*w.a;
#else
   Vec4 C      =TexPoint(Col, texcoord    ),
        Cleft  =TexPoint(Col, offset[0].xy),
        Ctop   =TexPoint(Col, offset[0].zw),
        Cright =TexPoint(Col, offset[1].xy),
        Cbottom=TexPoint(Col, offset[1].zw),
        color  =Lerp(C, Ctop   , a.r)*w.r;
        color +=Lerp(C, Cbottom, a.g)*w.g;
        color +=Lerp(C, Cleft  , a.b)*w.b;
        color +=Lerp(C, Cright , a.a)*w.a;
#endif

   return color/sum; // Normalize the resulting color
}
TECHNIQUE(MLAAEdge , MLAA_VS(), MLAAEdge_PS ());
TECHNIQUE(MLAABlend, MLAA_VS(), MLAABlend_PS());
TECHNIQUE(MLAA     , MLAA_VS(), MLAA_PS     ());
/******************************************************************************/
#define SMAA_FLATTEN FLATTEN
#define SMAA_BRANCH  BRANCH
#if DX11
   #define SMAA_HLSL_4 1 // TODO: using SMAA_HLSL_4_1 would be faster, but it's not easy to mix SMAA 4.0 and 4.1 together in one shader, however it's only used for predication and SMAADepthEdgeDetectionPS which are not used
   #define PointSampler  SamplerPoint
   #define LinearSampler SamplerLinearClamp
#else
   #define SMAA_HLSL_3 1
#endif
#if GL
   #define mad(a, b, c) ((a)*(b) + (c))
#endif

BUFFER(SMAA)
   Flt SMAAThreshold=0.05f;
BUFFER_END

#include "SMAA_config.h"
#include "SMAA.h"

void SMAAEdge_VS(VtxInput vtx,
             out Vec2 texcoord :TEXCOORD0,
             out Vec4 offset[3]:TEXCOORD1,
             out Vec4 position :POSITION )
{
   position=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(position); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   texcoord=vtx.tex();
   SMAAEdgeDetectionVS(texcoord, offset);
}
void SMAABlend_VS(VtxInput vtx,
              out Vec2 texcoord :TEXCOORD0,
              out Vec2 pixcoord :TEXCOORD1,
              out Vec4 offset[3]:TEXCOORD2,
              out Vec4 position :POSITION )
{
   position=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(position); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   texcoord=vtx.tex();
   SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}
void SMAA_VS(VtxInput vtx,
         out Vec2 texcoord:TEXCOORD0,
         out Vec4 offset  :TEXCOORD1,
         out Vec4 position:POSITION )
{
   position=Vec4(vtx.pos2(), !REVERSE_DEPTH, 1); AdjustPixelCenter(position); // set Z to be at the end of the viewport, this enables optimizations by optional applying lighting only on solid pixels (no sky/background)
   texcoord=vtx.tex();
   SMAANeighborhoodBlendingVS(texcoord, offset);
}
Vec4 SMAAEdgeLuma_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
                     NOPERSP Vec4 offset[3]:TEXCOORD1):COLOR
{
   return Vec4(SMAALumaEdgeDetectionPS(texcoord, offset, Col), 0, 1);
}
Vec4 SMAAEdgeColor_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
                      NOPERSP Vec4 offset[3]:TEXCOORD1):COLOR
{
   return Vec4(SMAAColorEdgeDetectionPS(texcoord, offset, Col), 0, 1);
}
Vec4 SMAABlend_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
                  NOPERSP Vec2 pixcoord :TEXCOORD1,
                  NOPERSP Vec4 offset[3]:TEXCOORD2):COLOR
{
   return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, Col, Col1, Col2, 0);
}
Vec4 SMAA_PS(NOPERSP Vec2 texcoord:TEXCOORD0,
             NOPERSP Vec4 offset  :TEXCOORD1):COLOR
{
   return SMAANeighborhoodBlendingPS(texcoord, offset, Col, Col1);
}
//TECHNIQUE(SMAAEdgeLuma , SMAAEdge_VS (), SMAAEdgeLuma_PS ());
  TECHNIQUE(SMAAEdgeColor, SMAAEdge_VS (), SMAAEdgeColor_PS());
  TECHNIQUE(SMAABlend    , SMAABlend_VS(), SMAABlend_PS    ());
  TECHNIQUE(SMAA         , SMAA_VS     (), SMAA_PS         ());
/******************************************************************************/
#if 0
@GROUP "Draw3DTex" // params: COLOR, alpha_test
   @SHARED
      #include "Glsl.h"

         VAR HP Vec2 IO_tex;
      #if COLOR!=0
         VAR LP Vec4 IO_col;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      void main()
      {
          O_vtx=Project(TransformPos(vtx_pos()));
         IO_tex=vtx_tex();
      #if COLOR!=0
         IO_col=vtx_color();
      #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
         LP Vec4 col=Tex(Col, IO_tex);
      #if alpha_test!=0
         if(col.a<0.5)discard;
      #endif
      #if COLOR!=0
         col*=IO_col;
      #endif
         gl_FragColor=col;
      }
   @PS_END

@GROUP_END


@GROUP "Draw2DTex" // params: COLOR
   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_tex;
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 2D.h"

      void main()
      {
          O_vtx=Vec4(vtx_pos2()*Coords.xy+Coords.zw, Flt(REVERSE_DEPTH), 1.0);
         IO_tex=vtx_tex();
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl PS 2D.h"

      void main()
      {
      #if COLOR!=0
         gl_FragColor=Tex(Col, IO_tex)*Color[0]+Color[1];
      #else
         gl_FragColor=Tex(Col, IO_tex);
      #endif
      }
   @PS_END

@GROUP_END


@GROUP "Font"
   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_tex;
      VAR MP Flt  IO_shade;
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 2D.h"

      void main()
      {
          O_vtx  =Vec4(vtx_pos2()*Coords.xy+Coords.zw, Flt(REVERSE_DEPTH), 1.0);
         IO_tex  =vtx_tex ();
         IO_shade=vtx_size();
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl PS 2D.h"

      PAR MP Flt FontShadow, FontShade;

      void main()
      {
         LP Vec4 tex=Tex(Col, IO_tex);
         MP Flt  a  =tex.g,
                 s  =tex.a*FontShadow,
                 final_alpha=a+s-s*a;
         LP Flt  final_color=Lerp(FontShade, 1.0, Sat(IO_shade))*a/(final_alpha+EPS);

         gl_FragColor.rgb=Color[0].rgb*final_color;
         gl_FragColor.a  =Color[0].a  *final_alpha;
      }
   @PS_END

@GROUP_END


@GROUP "Blur" // params: axis, high

   @SHARED
      #include "Glsl.h"

      #define WEIGHT4_0 0.250000000
      #define WEIGHT4_1 0.213388354
      #define WEIGHT4_2 0.124999993
      #define WEIGHT4_3 0.036611654
      // WEIGHT4_0 + WEIGHT4_1*2 + WEIGHT4_2*2 + WEIGHT4_3*2 = 1

      #define WEIGHT6_0 0.166666668
      #define WEIGHT6_1 0.155502122
      #define WEIGHT6_2 0.125000001
      #define WEIGHT6_3 0.083333329
      #define WEIGHT6_4 0.041666662
      #define WEIGHT6_5 0.011164551
      // WEIGHT6_0 + WEIGHT6_1*2 + WEIGHT6_2*2 + WEIGHT6_3*2 + WEIGHT6_4*2 + WEIGHT6_5*2 = 1

         VAR HP Vec2 IO_tex0, IO_tex1, IO_tex2, IO_tex3;
      #if high!=0
         VAR HP Vec2 IO_tex4, IO_tex5;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 2D.h"

      void main()
      {
         O_vtx=vtx_pos4();
         HP Vec2 tex=vtx_tex();
      #if high==0
         #if axis==0 // X
            IO_tex0.y=tex.y; IO_tex0.x=tex.x+ColSize.x*( 0.0+WEIGHT4_1/(WEIGHT4_0/2.0+WEIGHT4_1));
            IO_tex1.y=tex.y; IO_tex1.x=tex.x+ColSize.x*(-0.0-WEIGHT4_1/(WEIGHT4_0/2.0+WEIGHT4_1));
            IO_tex2.y=tex.y; IO_tex2.x=tex.x+ColSize.x*( 2.0+WEIGHT4_3/(WEIGHT4_2    +WEIGHT4_3));
            IO_tex3.y=tex.y; IO_tex3.x=tex.x+ColSize.x*(-2.0-WEIGHT4_3/(WEIGHT4_2    +WEIGHT4_3));
         #else
            IO_tex0.x=tex.x; IO_tex0.y=tex.y+ColSize.y*( 0.0+WEIGHT4_1/(WEIGHT4_0/2.0+WEIGHT4_1));
            IO_tex1.x=tex.x; IO_tex1.y=tex.y+ColSize.y*(-0.0-WEIGHT4_1/(WEIGHT4_0/2.0+WEIGHT4_1));
            IO_tex2.x=tex.x; IO_tex2.y=tex.y+ColSize.y*( 2.0+WEIGHT4_3/(WEIGHT4_2    +WEIGHT4_3));
            IO_tex3.x=tex.x; IO_tex3.y=tex.y+ColSize.y*(-2.0-WEIGHT4_3/(WEIGHT4_2    +WEIGHT4_3));
         #endif
      #else
         #if axis==0 // X
            IO_tex0.y=tex.y; IO_tex0.x=tex.x+ColSize.x*( 0.0+WEIGHT6_1/(WEIGHT6_0/2.0+WEIGHT6_1));
            IO_tex1.y=tex.y; IO_tex1.x=tex.x+ColSize.x*(-0.0-WEIGHT6_1/(WEIGHT6_0/2.0+WEIGHT6_1));
            IO_tex2.y=tex.y; IO_tex2.x=tex.x+ColSize.x*( 2.0+WEIGHT6_3/(WEIGHT6_2    +WEIGHT6_3));
            IO_tex3.y=tex.y; IO_tex3.x=tex.x+ColSize.x*(-2.0-WEIGHT6_3/(WEIGHT6_2    +WEIGHT6_3));
            IO_tex4.y=tex.y; IO_tex4.x=tex.x+ColSize.x*( 4.0+WEIGHT6_5/(WEIGHT6_4    +WEIGHT6_5));
            IO_tex5.y=tex.y; IO_tex5.x=tex.x+ColSize.x*(-4.0-WEIGHT6_5/(WEIGHT6_4    +WEIGHT6_5));
         #else
            IO_tex0.x=tex.x; IO_tex0.y=tex.y+ColSize.y*( 0.0+WEIGHT6_1/(WEIGHT6_0/2.0+WEIGHT6_1));
            IO_tex1.x=tex.x; IO_tex1.y=tex.y+ColSize.y*(-0.0-WEIGHT6_1/(WEIGHT6_0/2.0+WEIGHT6_1));
            IO_tex2.x=tex.x; IO_tex2.y=tex.y+ColSize.y*( 2.0+WEIGHT6_3/(WEIGHT6_2    +WEIGHT6_3));
            IO_tex3.x=tex.x; IO_tex3.y=tex.y+ColSize.y*(-2.0-WEIGHT6_3/(WEIGHT6_2    +WEIGHT6_3));
            IO_tex4.x=tex.x; IO_tex4.y=tex.y+ColSize.y*( 4.0+WEIGHT6_5/(WEIGHT6_4    +WEIGHT6_5));
            IO_tex5.x=tex.x; IO_tex5.y=tex.y+ColSize.y*(-4.0-WEIGHT6_5/(WEIGHT6_4    +WEIGHT6_5));
         #endif
      #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
      #if high==0
         gl_FragColor.rgb=texture2DLod(Col, IO_tex0, 0.0).rgb*(WEIGHT4_0/2.0+WEIGHT4_1)
                         +texture2DLod(Col, IO_tex1, 0.0).rgb*(WEIGHT4_0/2.0+WEIGHT4_1)
                         +texture2DLod(Col, IO_tex2, 0.0).rgb*(WEIGHT4_2    +WEIGHT4_3)
                         +texture2DLod(Col, IO_tex3, 0.0).rgb*(WEIGHT4_2    +WEIGHT4_3);
      #else
         gl_FragColor.rgb=texture2DLod(Col, IO_tex0, 0.0).rgb*(WEIGHT6_0/2.0+WEIGHT6_1)
                         +texture2DLod(Col, IO_tex1, 0.0).rgb*(WEIGHT6_0/2.0+WEIGHT6_1)
                         +texture2DLod(Col, IO_tex2, 0.0).rgb*(WEIGHT6_2    +WEIGHT6_3)
                         +texture2DLod(Col, IO_tex3, 0.0).rgb*(WEIGHT6_2    +WEIGHT6_3)
                         +texture2DLod(Col, IO_tex4, 0.0).rgb*(WEIGHT6_4    +WEIGHT6_5)
                         +texture2DLod(Col, IO_tex5, 0.0).rgb*(WEIGHT6_4    +WEIGHT6_5);
      #endif
         gl_FragColor.a=0.0;
      }
   @PS_END

@GROUP_END


@GROUP "BloomDS" // params: DoGlow, DoClamp, half, saturate

   @SHARED
      #include "Glsl.h"

      PAR MP Vec BloomParams;

      #if DoGlow!=0
         #define res ((half!=0) ? 2 : 4)
         VAR HP Vec2 IO_tex;
      #elif half!=0
         VAR HP Vec2 IO_tex;
      #else
         VAR HP Vec2 IO_tex0, IO_tex1, IO_tex2, IO_tex3;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 2D.h"

      void main()
      {
         O_vtx=vtx_pos4();
         HP Vec2 tex=vtx_tex();
      #if DoGlow!=0
         IO_tex=tex-ColSize.xy*Vec2((half!=0) ? 0.5 : 1.5, (half!=0) ? 0.5 : 1.5);
      #elif half!=0
         IO_tex=tex;
      #else
         #if DoClamp!=0
            IO_tex0=tex-ColSize.xy;
            IO_tex1=tex+ColSize.xy;
         #else
            IO_tex0=tex+ColSize.xy*Vec2( 1.0,  1.0);
            IO_tex1=tex+ColSize.xy*Vec2(-1.0,  1.0);
            IO_tex2=tex+ColSize.xy*Vec2( 1.0, -1.0);
            IO_tex3=tex+ColSize.xy*Vec2(-1.0, -1.0);
         #endif
      #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl PS 2D.h"
      #include "Glsl VS 2D.h"

      MP Vec BloomColor(MP Vec color)
      {
      #if saturate!=0
         return color*BloomParams.y+BloomParams.z;
      #else
         MP Flt col_lum=Max(Max(color.r, color.g), color.b), lum=col_lum*BloomParams.y+BloomParams.z;
         return (lum>0.0) ? color*(lum/col_lum) : Vec(0.0, 0.0, 0.0);
      #endif
      }

      void main()
      {
      #if DoGlow!=0
         MP Vec  color=Vec (0.0, 0.0, 0.0);
         MP Vec4 glow =Vec4(0.0, 0.0, 0.0, 0.0);
         for(Int y=0; y<res; y++)
         for(Int x=0; x<res; x++)
         {
            MP Vec4 c=texture2DLod(Col, UVClamp(IO_tex+ColSize.xy*Vec2(x, y), DoClamp!=0), 0.0);
            color   +=c.rgb;
            glow.rgb+=c.rgb*c.a;
            glow.a   =Max(glow.a, c.a);
         }
         MP Flt eps=EPS;
         glow.rgb*=2.0*glow.a/Max(glow.rgb, eps); // NaN (increase by 2 because normally it's too small)
         gl_FragColor.rgb=Max(BloomColor(color), glow.rgb);
      #elif half!=0
         gl_FragColor.rgb=BloomColor(texture2DLod(Col, UVClamp(IO_tex, DoClamp!=0), 0.0).rgb);
      #else
         #if DoClamp!=0
            HP Vec2 tex_min=UVClamp(IO_tex0, true),
                    tex_max=UVClamp(IO_tex1, true);
            MP Vec col=texture2DLod(Col, Vec2(tex_min.x, tex_min.y), 0.0).rgb // keep this outside of Sat(..) if used, because compilation will fail on Galaxy Tab 2 and some others
                      +texture2DLod(Col, Vec2(tex_max.x, tex_min.y), 0.0).rgb
                      +texture2DLod(Col, Vec2(tex_min.x, tex_max.y), 0.0).rgb
                      +texture2DLod(Col, Vec2(tex_max.x, tex_max.y), 0.0).rgb;
         #else
            MP Vec col=texture2DLod(Col, IO_tex0, 0.0).rgb
                      +texture2DLod(Col, IO_tex1, 0.0).rgb
                      +texture2DLod(Col, IO_tex2, 0.0).rgb
                      +texture2DLod(Col, IO_tex3, 0.0).rgb;
         #endif
            gl_FragColor.rgb=BloomColor(col);
      #endif
         gl_FragColor.a=0.0;
      }
   @PS_END

@GROUP_END


@GROUP "Bloom"
   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_tex;
   @SHARED_END

   @VS
      #include "Glsl VS.h"

      void main()
      {
          O_vtx=vtx_pos4();
         IO_tex=vtx_tex ();
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      PAR LP Vec BloomParams;

      void main()
      {
         gl_FragColor.rgb=texture2DLod(Col, IO_tex, 0.0).rgb*BloomParams.x + texture2DLod(Col1, IO_tex, 0.0).rgb;
         gl_FragColor.a  =1.0; // force full alpha so back buffer effects can work ok
      }
   @PS_END

@GROUP_END


@GROUP "ShdBlurX" // params: range
   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_tex;
   @SHARED_END

   @VS
      #include "Glsl VS.h"

      void main()
      {
          O_vtx=Vec4(vtx_pos2(), Flt(!REVERSE_DEPTH), 1.0);
         IO_tex=vtx_tex();
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl Matrix.h"
      PAR HP Matrix4(ProjMatrix);
      #include "Glsl Depth.h"
      PAR HP Flt  DepthWeightScale;
      PAR HP Vec4 ColSize;

      inline HP Vec2 DepthWeightMAD(HP Flt depth) {return Vec2(-1.0/(depth*DepthWeightScale+0.004), 2.0);}
      inline MP Flt  DepthWeight   (MP Flt delta, HP Vec2 dw_mad) {return Sat(Abs(delta)*dw_mad.x + dw_mad.y);}

      void main()
      {
         MP Flt  weight=0.5,
                 color =               texture2DLod(Col  , IO_tex, 0.0).x*weight;
         HP Flt  z     =LinearizeDepth(texture2DLod(Depth, IO_tex, 0.0).x, true);
         HP Vec2 dw_mad=DepthWeightMAD(z), t; t.y=IO_tex.y;
         for(MP Int i=-range; i<=range; i++)if(i!=0)
         {
            t.x=ColSize.x*(Flt(2*i)+((i>0) ? -0.5 : 0.5))+IO_tex.x;
            MP Flt w=DepthWeight(z-LinearizeDepth(texture2DLod(Depth, t, 0.0).x, true), dw_mad);
            color +=w*texture2DLod(Col, t, 0.0).x;
            weight+=w;
         }
         gl_FragColor=Vec4(color/weight, 0.0, 0.0, 0.0);
      }
   @PS_END

@GROUP_END


@GROUP "ShdBlurY" // params: range
   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_tex;
   @SHARED_END

   @VS
      #include "Glsl VS.h"

      void main()
      {
          O_vtx=Vec4(vtx_pos2(), Flt(!REVERSE_DEPTH), 1.0);
         IO_tex=vtx_tex();
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl Matrix.h"
      PAR HP Matrix4(ProjMatrix);
      #include "Glsl Depth.h"
      PAR HP Flt  DepthWeightScale;
      PAR HP Vec4 ColSize;

      inline HP Vec2 DepthWeightMAD(HP Flt depth) {return Vec2(-1.0/(depth*DepthWeightScale+0.004), 2.0);}
      inline MP Flt  DepthWeight   (MP Flt delta, HP Vec2 dw_mad) {return Sat(Abs(delta)*dw_mad.x + dw_mad.y);}

      void main()
      {
         MP Flt  weight=0.5,
                 color =               texture2DLod(Col  , IO_tex, 0.0).x*weight;
         HP Flt  z     =LinearizeDepth(texture2DLod(Depth, IO_tex, 0.0).x, true);
         HP Vec2 dw_mad=DepthWeightMAD(z), t; t.x=IO_tex.x;
         for(MP Int i=-range; i<=range; i++)if(i!=0)
         {
            t.y=ColSize.y*(Flt(2*i)+((i>0) ? -0.5 : 0.5))+IO_tex.y;
            MP Flt w=DepthWeight(z-LinearizeDepth(texture2DLod(Depth, t, 0.0).x, true), dw_mad);
            color +=w*texture2DLod(Col, t, 0.0).x;
            weight+=w;
         }
         gl_FragColor=Vec4(color/weight, 0.0, 0.0, 0.0);
      }
   @PS_END

@GROUP_END


@GROUP "Particle" // params: palette, anim, motion_stretch, stretch_alpha

   @SHARED
      #include "Glsl.h"

      #define ANIM_NONE   0
      #define ANIM_YES    1
      #define ANIM_SMOOTH 2

      VAR LP Vec4 IO_col;
      VAR HP Vec2 IO_tex;
      #if anim==ANIM_SMOOTH
         VAR HP Vec2 IO_tex1;
         VAR LP Flt  IO_tex_blend;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 2D.h"
      #include "Glsl VS 3D.h"

      PAR MP Vec2 ParticleFrames;

      void main()
      {
         IO_tex=vtx_tex  ();
         IO_col=vtx_color();

         MP Flt  size  =vtx_size(),
                 angle =vtx_tanW();
         HP Vec  pos   =TransformPos(vtx_pos());
         MP Vec2 offset=IO_tex; offset=offset*Vec2(2.0, -2.0)+Vec2(-1.0, 1.0); offset=Rotate(offset, Vec2(Cos(angle), Sin(angle)));

         #if motion_stretch!=0
         if(pos.z>0.0)
         {
            #define PARTICLE_PROJECT 100.0
            MP Vec  vel =TransformDir(vtx_tan()); if(vel.z<0.0)vel=-vel;
            HP Vec  pos1=pos+vel/PARTICLE_PROJECT;
            MP Vec2 vel2=(pos1.xy/pos1.z - pos.xy/pos.z)*PARTICLE_PROJECT;
            MP Flt  len =Length(vel2)+EPS;
            {
               MP Vec2 x=vel2*(vel2.x/len),
                       y=vel2*(vel2.y/len);
               offset=Vec2(offset.x*(x.x+1.0) + offset.y*y.x, offset.x*x.y + offset.y*(y.y+1.0));
               if(stretch_alpha!=0)
               {
                  if(palette!=0)IO_col  /=1.0+len; // in RM_PALETTE each component
                  else          IO_col.a/=1.0+len; // in RM_BLEND   only alpha
               }
            }
         }
         #endif

         pos.xy+=offset*size;

         // sky
         MP Vec mp_pos =pos;
         MP Flt d      =Length(mp_pos);
         LP Flt opacity=Sat(d*SkyFracMulAdd.x + SkyFracMulAdd.y);
         if(palette!=0)IO_col  *=opacity; // in RM_PALETTE each component
         else          IO_col.a*=opacity; // in RM_BLEND   only alpha

         #if anim!=ANIM_NONE
         {
            MP Flt frames=ParticleFrames.x*ParticleFrames.y,
                   frame =Frac(vtx_tex1().x/frames)*frames; // frame=[0..frames)
            HP Flt f     =Floor(frame); // keep this as HP because flickering can occur if MP is used (for example when ParticleFrames is 5x5)
            #if anim==ANIM_SMOOTH // frame blending
            {
               MP Flt f1=f+1.0; if(f1+0.5>=frames)f1=0.0; // f1=(f+1)%frames;
               IO_tex1     =IO_tex;
               IO_tex_blend=frame-f; // [0..1) frac

               f1/=ParticleFrames.x;
               MP Flt y=Floor(f1);
               IO_tex1.y+=y;
               IO_tex1  /=ParticleFrames;
               IO_tex1.x+=f1-y;
               IO_tex1  *=ColSize.xy; // Image.partial
            }
            #endif
            f/=ParticleFrames.x;
            MP Flt y=Floor(f);
            IO_tex.y+=y;
            IO_tex  /=ParticleFrames;
            IO_tex.x+=f-y;
         }
         #endif
         IO_tex*=ColSize.xy; // Image.partial
         O_vtx=Project(pos);
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
         LP Vec4 tex=          Tex(Col, IO_tex );
      #if anim==ANIM_SMOOTH
                 tex=Lerp(tex, Tex(Col, IO_tex1), IO_tex_blend);
      #endif
         if(palette!=0)gl_FragColor=IO_col*tex.a;
         else          gl_FragColor=IO_col*tex  ;
      }
   @PS_END

@GROUP_END


@GROUP "Sky" // params: per_vertex, DENSITY, textures, stars, clouds

   @SHARED
      #include "Glsl.h"

      PAR HP Flt  SkyDnsExp      ; // need high precision
      PAR MP Flt  SkyHorExp      ,
                  SkyBoxBlend    ;
      PAR LP Vec4 SkyHorCol      ,
                  SkySkyCol      ;
      PAR MP Vec2 SkyDnsMulAdd   ,
                  SkySunHighlight;
      PAR MP Vec  SkySunPos      ;

      #define LCScale 0.2

      struct CloudLayer
      {
         LP Vec4 color;
         MP Vec2 scale;
         HP Vec2 position;
      };

      PAR MP Flt  LCScaleY;
      PAR MP Vec2 LCMaskContrast;

      PAR CloudLayer CL[1];

      inline LP Vec4 SkyColor(MP Flt y)
      {
         LP Flt hor=Pow(1.0-Sat(y), SkyHorExp);
         return Lerp(SkySkyCol, SkyHorCol, hor);
      }

         VAR MP Vec  IO_tex;
      #if stars!=0
         VAR MP Vec  IO_tex_star;
      #endif
      #if per_vertex!=0
         VAR LP Vec4 IO_col;
      #endif
      #if clouds!=0
         VAR HP Vec  IO_tex_cloud;
         VAR LP Vec4 IO_col_cloud;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      PAR MP Matrix3(SkyStarOrn);

      void main()
      {
          O_vtx=Project(TransformPos(vtx_pos()));
         IO_tex=vtx_pos();
      #if stars!=0
         IO_tex_star=Transform(vtx_pos(), SkyStarOrn);
      #endif
      #if per_vertex!=0
         IO_col=SkyColor(vtx_pos().y);
      #endif
         #if clouds!=0
         {
            MP Vec pos=vtx_pos(); pos*=Vec(LCScale, 1.0, LCScale);
            IO_col_cloud=CL[0].color; IO_col_cloud.a*=Sat(pos.y*8.0-0.15); // CloudAlpha
            IO_tex_cloud=pos;
         }
         #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      PAR ImageCube Cub;

      inline LP Vec SkyTex()
      {
         if(textures==2)return Vec(Lerp(TexCube(Rfl, IO_tex).rgb, TexCube(Cub, IO_tex).rgb, SkyBoxBlend));else
         if(textures==1)return Vec(     TexCube(Rfl, IO_tex).rgb                                        );else
         {
            LP Vec4 col;
            #if per_vertex==0
            {
               MP Vec tex=Normalize(IO_tex);
                      col=SkyColor (tex.y );

               MP Flt cos      =Dot(SkySunPos, tex);
               LP Flt highlight=1.0+Sqr(cos)*((cos>0.0) ? SkySunHighlight.x : SkySunHighlight.y); // rayleigh
               col.rgb*=highlight;
            }
            #else
               col=IO_col;
            #endif

            #if stars!=0
               col.rgb=Lerp(TexCube(Rfl, IO_tex_star).rgb, col.rgb, col.a);
            #endif

            return col.rgb;
         }
      }

      void main()
      {
         LP Vec col=SkyTex();
         #if clouds!=0
         {
            HP Vec2 uv=Normalize(IO_tex_cloud).xz;
            LP Vec4 tex=Tex(Col, uv*CL[0].scale+CL[0].position)*IO_col_cloud;
            col.rgb=Lerp(col.rgb, tex.rgb, tex.a);
         }
         #endif
         gl_FragColor.rgb=col;
         gl_FragColor.a  =0.0;
      }
   @PS_END

@GROUP_END


@GROUP "FXAA"
   @SHARED
      #include "Glsl.h"
      VAR HP Vec2 IO_tex;
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      void main()
      {
          O_vtx=vtx_pos4();
         IO_tex=vtx_tex ();
      }
   @VS_END

   @PS
      #define FXAA_GLSL_130 1
      #include "Glsl VS 2D.h"
      #include "Glsl PS.h"
      #include "FXAA_config.h"
      #include "FXAA.h"
      void main()
      {
         gl_FragColor=FxaaPixelShader(IO_tex, 0.0, Col, Col, Col, ColSize.xy, 0.0, 0.0, 0.0, 0.475, 0.15, 0.0833, 8.0, 0.125, 0.05, Vec4(1.0, -1.0, 0.25, -0.25));
      }
   @PS_END
@GROUP_END


@GROUP "SMAAEdge"
   @SHARED
      #define SMAA_GLSL_3 1
      #include "Glsl.h"
      #include "Glsl VS 2D.h"
      PAR MP Flt SMAAThreshold;
      #include "SMAA_config.h"
      VAR HP Vec2 texcoord;
      VAR HP Vec4 offset[3];
   @SHARED_END

   @VS
      #define SMAA_INCLUDE_PS 0
      #include "Glsl VS.h"
      #include "SMAA.h"
      void main()
      {
         O_vtx=vtx_pos4();
         texcoord=vtx_tex();
         SMAAEdgeDetectionVS(texcoord, offset);
      }
   @VS_END

   @PS
      #define SMAA_INCLUDE_VS 0
      #include "Glsl PS.h"
      #include "SMAA.h"
      void main()
      {
         gl_FragColor.rg=SMAAColorEdgeDetectionPS(texcoord, offset, Col);
         gl_FragColor.b=0;
         gl_FragColor.a=1;
      }
   @PS_END
@GROUP_END


@GROUP "SMAABlend"
   @SHARED
      #define SMAA_GLSL_3 1
      #include "Glsl.h"
      #include "Glsl VS 2D.h"
      #include "SMAA_config.h"
      VAR HP Vec2 texcoord, pixcoord;
      VAR HP Vec4 offset[3];
   @SHARED_END

   @VS
      #define SMAA_INCLUDE_PS 0
      #include "Glsl VS.h"
      #include "SMAA.h"
      void main()
      {
         O_vtx=vtx_pos4();
         texcoord=vtx_tex();
         SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
      }
   @VS_END

   @PS
      #define SMAA_INCLUDE_VS 0
      #include "Glsl PS.h"
      #include "SMAA.h"
      void main()
      {
         gl_FragColor=SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, Col, Col1, Col2, 0);
      }
   @PS_END
@GROUP_END


@GROUP "SMAA"
   @SHARED
      #define SMAA_GLSL_3 1
      #include "Glsl.h"
      #include "Glsl VS 2D.h"
      #include "SMAA_config.h"
      VAR HP Vec2 texcoord;
      VAR HP Vec4 offset;
   @SHARED_END

   @VS
      #define SMAA_INCLUDE_PS 0
      #include "Glsl VS.h"
      #include "SMAA.h"
      void main()
      {
         O_vtx=vtx_pos4();
         texcoord=vtx_tex();
         SMAANeighborhoodBlendingVS(texcoord, offset);
      }
   @VS_END

   @PS
      #define SMAA_INCLUDE_VS 0
      #include "Glsl PS.h"
      #include "SMAA.h"
      void main()
      {
         gl_FragColor=SMAANeighborhoodBlendingPS(texcoord, offset, Col, Col1);
      }
   @PS_END
@GROUP_END
#endif
