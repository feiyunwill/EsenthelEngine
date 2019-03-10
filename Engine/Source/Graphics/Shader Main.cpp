/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************/
MainShaderClass    Sh;
AmbientOcclusion   AO;
   LayeredCloudsFx LC;
VolumetricCloudsFx VolCloud;
VolumetricLights   VL;
HDR                Hdr;
MotionBlur         Mtn;
DepthOfField       Dof;
WaterShader        WS;

ShaderImage::Sampler SamplerPoint, SamplerLinearWrap, SamplerLinearWCC, SamplerLinearCWC, SamplerLinearCWW, SamplerLinearClamp, SamplerFont, SamplerAnisotropic, SamplerShadowMap;
/******************************************************************************/
// MAIN SHADER
/******************************************************************************/
#if DX11
void CreateAnisotropicSampler()
{
   D3D11_SAMPLER_DESC  sd; Zero(sd);
   if(D.texFilter()> 1)sd.Filter=D3D11_FILTER_ANISOTROPIC;else
   if(D.texFilter()==1)sd.Filter=(D.texMipFilter() ? D3D11_FILTER_MIN_MAG_MIP_LINEAR       : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT       );else
                       sd.Filter=(D.texMipFilter() ? D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT : D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.MipLODBias    =0;
   sd.MaxAnisotropy =Mid(D.texFilter(), 1, 16);
   sd.MinLOD        =Max(D.texLod   (), 0    );
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
   SamplerAnisotropic.create(sd);
}
void CreateFontSampler()
{
   D3D11_SAMPLER_DESC sd; Zero(sd);
   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.MipLODBias    =-D.fontSharpness();
   sd.MaxAnisotropy =1;
   sd.MinLOD        =0;
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
   SamplerFont.create(sd);
}
#endif
/******************************************************************************/
Shader* MainShaderClass::find(C Str8 &name) {return shader->find(name);}
Shader* MainShaderClass:: get(C Str8 &name) {return shader-> get(name);}
/******************************************************************************/
void MainShaderClass::del()
{
   // delete all to detect GPU memory leaks when using D3D_DEBUG
   shader        =null;
   ShaderFiles  .del();
   ShaderParams .del(); // params before buffers, because they point to them
   ShaderBuffers.del();
   ShaderImages .del();

   SamplerPoint      .del();
   SamplerLinearWrap .del();
   SamplerLinearWCC  .del();
   SamplerLinearCWC  .del();
   SamplerLinearCWW  .del();
   SamplerLinearClamp.del();
   SamplerFont       .del();
   SamplerAnisotropic.del();
   SamplerShadowMap  .del();
}
void MainShaderClass::createSamplers()
{
#if DX9
   REPAO(SamplerPoint.filter )=D3DTEXF_POINT;
   REPAO(SamplerPoint.address)=D3DTADDRESS_CLAMP;

   REPAO(SamplerLinearWrap.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerLinearWrap.address)=D3DTADDRESS_WRAP;

   REPAO(SamplerLinearWCC.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerLinearWCC.address)=D3DTADDRESS_CLAMP; SamplerLinearWCC.address[0]=D3DTADDRESS_WRAP;

   REPAO(SamplerLinearCWC.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerLinearCWC.address)=D3DTADDRESS_CLAMP; SamplerLinearCWC.address[1]=D3DTADDRESS_WRAP;

   REPAO(SamplerLinearCWW.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerLinearCWW.address)=D3DTADDRESS_WRAP; SamplerLinearCWW.address[0]=D3DTADDRESS_CLAMP;

   REPAO(SamplerLinearClamp.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerLinearClamp.address)=D3DTADDRESS_CLAMP;

   REPAO(SamplerFont.filter )=D3DTEXF_LINEAR;
   REPAO(SamplerFont.address)=D3DTADDRESS_CLAMP;
#elif DX11
   D3D11_SAMPLER_DESC sd; Zero(sd);
   sd.MipLODBias    =0;
   sd.MaxAnisotropy =1;
   sd.MinLOD        =0;
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_POINT;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerPoint.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   SamplerLinearWrap.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearWCC.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearCWC.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   SamplerLinearCWW.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearClamp.create(sd);

   sd.Filter  =D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.ComparisonFunc=(REVERSE_DEPTH ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL);
   SamplerShadowMap.create(sd);
   
   CreateAnisotropicSampler();
   CreateFontSampler       ();
#elif GL
   REPAO(SamplerPoint.filter )=GL_NEAREST;
   REPAO(SamplerPoint.address)=GL_CLAMP_TO_EDGE;

   REPAO(SamplerLinearWrap.filter )=GL_LINEAR;
   REPAO(SamplerLinearWrap.address)=GL_REPEAT;

   REPAO(SamplerLinearWCC.filter )=GL_LINEAR;
   REPAO(SamplerLinearWCC.address)=GL_CLAMP_TO_EDGE; SamplerLinearWCC.address[0]=GL_REPEAT;

   REPAO(SamplerLinearCWC.filter )=GL_LINEAR;
   REPAO(SamplerLinearCWC.address)=GL_CLAMP_TO_EDGE; SamplerLinearCWC.address[1]=GL_REPEAT;

   REPAO(SamplerLinearCWW.filter )=GL_LINEAR;
   REPAO(SamplerLinearCWW.address)=GL_REPEAT; SamplerLinearCWW.address[0]=GL_CLAMP_TO_EDGE;

   REPAO(SamplerLinearClamp.filter )=GL_LINEAR;
   REPAO(SamplerLinearClamp.address)=GL_CLAMP_TO_EDGE;

   REPAO(SamplerFont.filter )=GL_LINEAR;
   REPAO(SamplerFont.address)=GL_CLAMP_TO_EDGE;
#endif
}
void MainShaderClass::create()
{
   if(LogInit)LogN("MainShaderClass.create");
   compile();

   if(D.shaderModel()>=SM_4)path="Shader\\4\\" ;else
   if(D.shaderModel()>=SM_3)path="Shader\\3\\" ;else
                            path="Shader\\GL\\";

   shader=ShaderFiles("Main"); if(!shader)Exit("Can't load the Main Shader");
#define MEASURE_SHADER_LOAD_TIME 0
#if     MEASURE_SHADER_LOAD_TIME
 //#define SLOW_SHADER_LOAD 0
   #pragma message("!! Warning: Use this only for debugging !!")
   Flt t=Time.curTime();
#endif
   getTechniques();
#if MEASURE_SHADER_LOAD_TIME
   Exit(S+"Shaders Loaded in: "+(Time.curTime()-t)+'s');
#endif
   connectRT    ();
}
/******************************************************************************/
void MainShaderClass::clear(                C Vec4  &color,                     C Rect *rect) {Sh.h_Color[0]->set(color);                                Sh.h_SetCol->draw(null , rect);}
void MainShaderClass::draw (C Image &image                ,                     C Rect *rect) {                                                          Sh.h_Draw  ->draw(image, rect);}
void MainShaderClass::draw (C Image &image, C Vec4  &color, C Vec4  &color_add, C Rect *rect) {Sh.h_Color[0]->set(color); Sh.h_Color[1]->set(color_add); Sh.h_DrawC ->draw(image, rect);}
void MainShaderClass::draw (C Image &image, C Color &color, C Color &color_add, C Rect *rect) {Sh.h_Color[0]->set(color); Sh.h_Color[1]->set(color_add); Sh.h_DrawC ->draw(image, rect);}
/******************************************************************************/
Shader* MainShaderClass::getBloomDS(Bool glow, Bool viewport_clamp, Bool half, Bool saturate) {return get(S8+"Bloom"+(glow?'G':'\0')+"DS"+(viewport_clamp?'C':'\0')+(half?'H':'\0')+(saturate?'S':'\0'));}
Shader* MainShaderClass::getBloom  (Bool dither                                             ) {return get(S8+"Bloom"+(dither?'D':'\0'));}

Shader* MainShaderClass::getShdDir (Int map_num, Bool clouds, Bool multi_sample) {return get(S8+"ShdDir"+map_num+(clouds?'C':'\0')+(multi_sample?'M':'\0'));}
Shader* MainShaderClass::getShdPnt (                          Bool multi_sample) {return get(S8+"ShdPnt"                          +(multi_sample?'M':'\0'));}
Shader* MainShaderClass::getShdCone(                          Bool multi_sample) {return get(S8+"ShdCone"                         +(multi_sample?'M':'\0'));}

Shader* MainShaderClass::getLightDir (Bool shadow,             Bool multi_sample, Bool quality) {return get(S8+"LightDir" +(shadow?'S':'\0')+((quality && !multi_sample)?'Q':'\0')                 +(multi_sample?'M':'\0'));} // MSAA doesn't have quality version (to make it faster)
Shader* MainShaderClass::getLightPnt (Bool shadow,             Bool multi_sample, Bool quality) {return get(S8+"LightPnt" +(shadow?'S':'\0')+((quality && !multi_sample)?'Q':'\0')                 +(multi_sample?'M':'\0'));} // MSAA doesn't have quality version (to make it faster)
Shader* MainShaderClass::getLightSqr (Bool shadow,             Bool multi_sample, Bool quality) {return get(S8+"LightSqr" +(shadow?'S':'\0')+((quality && !multi_sample)?'Q':'\0')                 +(multi_sample?'M':'\0'));} // MSAA doesn't have quality version (to make it faster)
Shader* MainShaderClass::getLightCone(Bool shadow, Bool image, Bool multi_sample, Bool quality) {return get(S8+"LightCone"+(shadow?'S':'\0')+((quality && !multi_sample)?'Q':'\0')+(image?'I':'\0')+(multi_sample?'M':'\0'));} // MSAA doesn't have quality version (to make it faster)

Shader* MainShaderClass::getColLight(Int multi_sample, Bool ao, Bool cel_shade, Bool night_shade) {return get(S8+"ColLight"+multi_sample+(ao?'A':'\0')+(cel_shade?'C':'\0')+(night_shade?'N':'\0'));}

Shader* MainShaderClass::getSunRaysMask(Bool mask             ) {return get(S8+"SunRaysMask"+(mask?'1':'\0'));}
Shader* MainShaderClass::getSunRays    (Bool high, Bool jitter) {return get(S8+"SunRays"    +(high?'H':'\0')+(jitter?'J':'\0'));}

Shader* MainShaderClass::getSkyTF(Int textures,                Bool cloud  ,                   Bool dither) {return get(S8+"SkyTF"+textures+(cloud?'C':'\0')+(dither?'D':'\0'));}
Shader* MainShaderClass::getSkyT (Int textures,                              Int multi_sample, Bool dither) {return get(S8+"SkyT" +textures+multi_sample+(dither?'D':'\0'));}
Shader* MainShaderClass::getSkyAF(Bool per_vertex, Bool stars, Bool cloud  ,                   Bool dither) {return get(S8+"SkyAF"+(per_vertex?'V':'\0')+(stars?'S':'\0')+(cloud?'C':'\0')+(dither?'D':'\0'));}
Shader* MainShaderClass::getSkyA (Bool per_vertex, Bool stars, Bool density, Int multi_sample, Bool dither) {return get(S8+"SkyA" +(per_vertex?'V':'\0')+(stars?'S':'\0')+(density?'P':'\0')+multi_sample+(dither?'D':'\0'));}

void MainShaderClass::initCubicShaders()
{
   if(!h_DrawTexCubic) // check if not yet initialized because this is called multiple times for SLOW_SHADER_LOAD
   {
      h_DrawTexCubic    =get("DrawTexCubic");
      h_DrawTexCubicC   =get("DrawTexCubicC");
      h_DrawTexCubic1   =get("DrawTexCubic1");
      h_DrawTexCubicD   =get("DrawTexCubicD");
      h_DrawTexCubicRGB =get("DrawTexCubicRGB");
      h_DrawTexCubicRGBD=get("DrawTexCubicRGBD");
   }
}
void MainShaderClass::initFogBoxShaders()
{
   if(!h_FogBox) // check if not yet initialized because this is called multiple times for SLOW_SHADER_LOAD
   {
      h_FogBox =get("FogBox"  );
      h_FogBox0=get("FogBox0" );
      h_FogBox1=get("FogBox1" );
   }
}
void MainShaderClass::initFogHgtShaders()
{
   if(!h_FogHgt) // check if not yet initialized because this is called multiple times for SLOW_SHADER_LOAD
   {
      h_FogHgt =get("FogHgt"  );
      h_FogHgt0=get("FogHgt0" );
      h_FogHgt1=get("FogHgt1" );
   }
}
void MainShaderClass::initFogBallShaders()
{
   if(!h_FogBall) // check if not yet initialized because this is called multiple times for SLOW_SHADER_LOAD
   {
      h_FogBall =get("FogBall" );
      h_FogBall0=get("FogBall0");
      h_FogBall1=get("FogBall1");
   }
}

void MainShaderClass::getTechniques()
{
   // images
   h_ImageCol[0]   =ShaderImages("Col"    );
   h_ImageCol[1]   =ShaderImages("Col1"   );
   h_ImageCol[2]   =ShaderImages("Col2"   );
   h_ImageCol[3]   =ShaderImages("Col3"   );
   h_ImageNrm[0]   =ShaderImages("Nrm"    );
   h_ImageNrm[1]   =ShaderImages("Nrm1"   );
   h_ImageNrm[2]   =ShaderImages("Nrm2"   );
   h_ImageNrm[3]   =ShaderImages("Nrm3"   );
   h_ImageDet[0]   =ShaderImages("Det"    );
   h_ImageDet[1]   =ShaderImages("Det1"   );
   h_ImageDet[2]   =ShaderImages("Det2"   );
   h_ImageDet[3]   =ShaderImages("Det3"   );
   h_ImageMac[0]   =ShaderImages("Mac"    );
   h_ImageMac[1]   =ShaderImages("Mac1"   );
   h_ImageMac[2]   =ShaderImages("Mac2"   );
   h_ImageMac[3]   =ShaderImages("Mac3"   );
   h_ImageRfl[0]   =ShaderImages("Rfl"    );
   h_ImageRfl[1]   =ShaderImages("Rfl1"   );
   h_ImageRfl[2]   =ShaderImages("Rfl2"   );
   h_ImageRfl[3]   =ShaderImages("Rfl3"   );
   h_ImageLum      =ShaderImages("Lum"    );
   h_ImageShdMap[0]=ShaderImages("ShdMap" ); h_ImageShdMap[0]->_sampler=&SamplerLinearClamp;
   h_ImageShdMap[1]=ShaderImages("ShdMap1"); h_ImageShdMap[1]->_sampler=&SamplerLinearClamp;
   h_ImageColMS    =ShaderImages("ColMS"  );
   h_ImageNrmMS    =ShaderImages("NrmMS"  );
   h_ImageLumMS    =ShaderImages("LumMS"  );
   h_ImageDepth    =ShaderImages("Depth"  );
   h_ImageDepthMS  =ShaderImages("DepthMS");
   h_ImageCub      =ShaderImages("Cub"    );
   h_ImageVol[0]   =ShaderImages("Vol"    );
   h_ImageVol[1]   =ShaderImages("Vol1"   );

   h_ColSize         =GetShaderParam("ColSize" );
   h_ColClamp        =GetShaderParam("ColClamp");
   h_RTSizeI         =GetShaderParam("RTSizeI" );
   h_Coords          =GetShaderParam("Coords"  );
   h_Viewport        =GetShaderParam("Viewport");
   h_DepthWeightScale=GetShaderParam("DepthWeightScale");
#if DX9
   h_PixelOffset=GetShaderParam("PixelOffset");
#endif

   h_CamAngVel =GetShaderParam("CamAngVel" );
   h_ObjAngVel =GetShaderParam("ObjAngVel" );
   h_ObjVel    =GetShaderParam("ObjVel"    );
   h_ViewMatrix=GetShaderParam("ViewMatrix");
   h_CamMatrix =GetShaderParam("CamMatrix" );
   h_ProjMatrix=GetShaderParam("ProjMatrix");
   h_FurVel    =GetShaderParam("FurVel"    );
   h_ClipPlane =GetShaderParam("ClipPlane" );

              ConstCast(Renderer.highlight     )=GetShaderParam("Highlight");
   h_Material=ConstCast(Renderer.material_color)=GetShaderParam("Material");
   h_MultiMaterial[0]=GetShaderParam("MultiMaterial0");
   h_MultiMaterial[1]=GetShaderParam("MultiMaterial1");
   h_MultiMaterial[2]=GetShaderParam("MultiMaterial2");
   h_MultiMaterial[3]=GetShaderParam("MultiMaterial3");

   h_Light_dir =GetShaderParam("Light_dir"  );
   h_Light_pnt =GetShaderParam("Light_point");
   h_Light_sqr =GetShaderParam("Light_sqr"  );
   h_Light_cone=GetShaderParam("Light_cone" );

   h_Step         =GetShaderParam("Step"         );
   h_Color[0]     =GetShaderParam("Color[0]"     );
   h_Color[1]     =GetShaderParam("Color[1]"     );
   h_BehindBias   =GetShaderParam("BehindBias"   );
   h_AllowBackFlip=GetShaderParam("AllowBackFlip");

   h_VtxSkinning =GetShaderParam("VtxSkinning" );
   h_VtxHeightmap=GetShaderParam("VtxHeightmap");
#if DX9 // required
   h_VtxNrmMulAdd=GetShaderParam("VtxNrmMulAdd"); SetVtxNrmMulAdd(true);
#endif

   h_LightMapScale   =GetShaderParam("LightMapScale");
   h_LightMapColAdd  =GetShaderParam("LightMapColAdd");
   h_LightMapSpecular=GetShaderParam("LightMapSpecular");

   h_GrassRangeMulAdd=GetShaderParam("GrassRangeMulAdd");
   h_BendFactor      =GetShaderParam("BendFactor");

   ConstCast(Renderer.ambient_color)=GetShaderParam("AmbColor"       ); Renderer.ambient_color->set(D.ambientColor   ());
                   h_AmbientMaterial=GetShaderParam("AmbMaterial"    );
                   h_AmbientContrast=GetShaderParam("AmbContrast"    );      h_AmbientContrast->set(D.ambientContrast());
                   h_AmbientRange   =GetShaderParam("AmbRange"       );      h_AmbientRange   ->set(D.ambientRange   ());
                   h_AmbientScale   =GetShaderParam("AmbScale"       );      h_AmbientScale   ->set(D.ambientScale   ());
                   h_AmbientBias    =GetShaderParam("AmbBias"        );      h_AmbientBias    ->set(D.ambientBias    ());
                   h_NightShadeColor=GetShaderParam("NightShadeColor");      h_NightShadeColor->set(D.nightShadeColor());

   h_HdrBrightness=GetShaderParam("HdrBrightness"); h_HdrBrightness->set(D.eyeAdaptationBrightness());
   h_HdrMaxDark   =GetShaderParam("HdrMaxDark"   ); h_HdrMaxDark   ->set(D.eyeAdaptationMaxDark   ());
   h_HdrMaxBright =GetShaderParam("HdrMaxBright" ); h_HdrMaxBright ->set(D.eyeAdaptationMaxBright ());
   h_HdrWeight    =GetShaderParam("HdrWeight"    ); h_HdrWeight    ->set(D.eyeAdaptationWeight()/4  );

   h_TesselationDensity=GetShaderParam("TesselationDensity"); h_TesselationDensity->set(D.tesselationDensity());

   // TECHNIQUES
   h_Draw2DFlat  =get("Draw2DFlat");
   h_Draw3DFlat  =get("Draw3DFlat");
   h_Draw2DCol   =get("Draw2DCol");
   h_Draw3DCol   =get("Draw3DCol");
   h_Draw2DTex   =get("Draw2DTex");
   h_Draw2DTexC  =get("Draw2DTexC");
   h_Draw2DTexCol=get("Draw2DTexCol");
   REPD(at, 2)
   REPD(f , 2)
   {
      h_Draw3DTex   [at][f]=get(S8+"Draw3DTex"   +(at?"AT":"")+(f?'F':'\0'));
      h_Draw3DTexCol[at][f]=get(S8+"Draw3DTexCol"+(at?"AT":"")+(f?'F':'\0'));
   }
   h_PaletteDraw=get("PaletteDraw");
   h_Simple     =get("Simple");
   h_DrawTexX   =get("DrawTexX");
 //h_DrawTexZ   =get("DrawTexZ"); used by Editor
   h_DrawTexW   =get("DrawTexW");
   h_DrawTexXC  =get("DrawTexXC");
   h_DrawTexWC  =get("DrawTexWC");
   h_DrawTexXCD =get("DrawTexXCD");
   h_DrawTexWCD =get("DrawTexWCD");
 //h_DrawTexNrm =get("DrawTexNrm"); used by Editor
   h_DrawMask   =get("DrawMask");

   // POINT (these can be null if failed to load)
   h_DrawTexPoint =find("DrawTexPoint");
   h_DrawTexPointC=find("DrawTexPointC");

   // CUBIC (these can be null if failed to load)
   h_DrawTexCubicFast    =find("DrawTexCubicFast");
   h_DrawTexCubicFastC   =find("DrawTexCubicFastC");
   h_DrawTexCubicFast1   =find("DrawTexCubicFast1");
   h_DrawTexCubicFastD   =find("DrawTexCubicFastD");
   h_DrawTexCubicFastRGB =find("DrawTexCubicFastRGB");
   h_DrawTexCubicFastRGBD=find("DrawTexCubicFastRGBD");
#if !SLOW_SHADER_LOAD
   initCubicShaders();
#endif

   // FONT
   h_FontShadow  =GetShaderParam("FontShadow"  );
   h_FontContrast=GetShaderParam("FontContrast");
   h_FontShade   =GetShaderParam("FontShade"   );
   h_FontDepth   =GetShaderParam("FontDepth"   );
   h_Font =get("Font");
   h_FontD=get("FontD"); // font with custom depth

   // BASIC 2D
   h_Dither=get("Dither");
   h_SetCol=get("SetCol");
   h_Draw  =get("Draw"  );
   h_DrawC =get("DrawC" );
   h_DrawA =get("DrawA" );
   if(D.shaderModel()>=SM_4)
   {
                                 h_DrawMs1=get("DrawMs1");
                                 h_DrawMsN=get("DrawMsN");
      if(D.shaderModel()>=SM_4_1)h_DrawMsM=get("DrawMsM");
   }

   // BLOOM
   h_BloomParams=GetShaderParam("BloomParams");
#if !SLOW_SHADER_LOAD
   REPD(g, 2)
   REPD(c, 2)
   REPD(h, 2)
   REPD(s, 2)h_BloomDS[g][c][h][s]=getBloomDS(g, c, h, s);
   REPD(d, 2)h_Bloom  [d]         =getBloom  (d);
#endif

   // BLUR
   REPD(h, 2)
   {
      h_BlurX[h]=get(S8+"BlurX"+(h?'H':'\0'));
      h_BlurY[h]=get(S8+"BlurY"+(h?'H':'\0'));
   }

   // DEPTH
#if DX9
   REPD(p, 2)h_LinearizeDepthRAWZ[p]=get(S8+"LinearizeDepthRAWZ"+(p?'P':'\0'));
#endif
   REPD(m, (D.shaderModel()>=SM_4_1) ? 3 : (D.shaderModel()>=SM_4) ? 2 : 1)
   REPD(p, 2)h_LinearizeDepth[p][m]=get(S8+"LinearizeDepth"+(p?'P':'\0')+m);

#if GL_ES
   h_SetDepth=find("SetDepth"); // GL ES 2.0 may fail because of depth writing
#else
   h_SetDepth=get ("SetDepth");
#endif

   if(D.shaderModel()>=SM_4)
   {
      h_ResolveDepth=get("ResolveDepth");
      h_DetectMSCol =get("DetectMSCol");
    //h_DetectMSNrm =get("DetectMSNrm");
   }

#if !SLOW_SHADER_LOAD
                            h_Combine       =get("Combine");
   if(D.shaderModel()>=SM_4)h_CombineMS     =get("CombineMS");
                            h_CombineSS     =get("CombineSS");
                            h_CombineSSAlpha=get("CombineSSAlpha");
#endif

   // SKY
   h_Sun            =GetShaderParam("Sun"            );
   h_SkyDnsExp      =GetShaderParam("SkyDnsExp"      );
   h_SkyHorExp      =GetShaderParam("SkyHorExp"      );
   h_SkyBoxBlend    =GetShaderParam("SkyBoxBlend"    );
   h_SkyHorCol      =GetShaderParam("SkyHorCol"      );
   h_SkySkyCol      =GetShaderParam("SkySkyCol"      );
   h_SkyStarOrn     =GetShaderParam("SkyStarOrn"     );
   h_SkyFracMulAdd  =GetShaderParam("SkyFracMulAdd"  );
   h_SkyDnsMulAdd   =GetShaderParam("SkyDnsMulAdd"   );
   h_SkySunHighlight=GetShaderParam("SkySunHighlight");
   h_SkySunPos      =GetShaderParam("SkySunPos"      );
#if !SLOW_SHADER_LOAD
   REPD(dither, 2)
   {
      REPD(t, 2)
      REPD(c, 2)h_SkyTF[t][c][dither]=getSkyTF(t+1, c, dither);

      REPD(v, 2)
      REPD(s, 2)
      REPD(c, 2)h_SkyAF[v][s][c][dither]=getSkyAF(v, s, c, dither);

      REPD(m, (D.shaderModel()>=SM_4_1) ? 3 : (D.shaderModel()>=SM_4) ? 2 : 1)
      {
         REPD(t, 2)h_SkyT[t][m][dither]=getSkyT(t+1, m, dither);

         REPD(v, 2)
         REPD(s, 2)
         REPD(d, 2)h_SkyA[v][s][d][m][dither]=getSkyA(v, s, d, m, dither);
      }
   }

   REPD(m, 2)h_SunRaysMask[m]=getSunRaysMask(m);
   REPD(h, 2)
   REPD(j, 2)h_SunRays[h][j]=getSunRays(h, j);
 //h_SunRaysSoft=get("SunRaysSoft");
#endif

   // SHADOWS
   REPAO(h_ShdStep      )=GetShaderParam(S8+"ShdStep["+i+']');
         h_ShdJitter     =GetShaderParam("ShdJitter");
         h_ShdRange      =GetShaderParam("ShdRange");
         h_ShdRangeMulAdd=GetShaderParam("ShdRangeMulAdd");
         h_ShdOpacity    =GetShaderParam("ShdOpacity");
         h_ShdMatrix     =GetShaderParam("ShdMatrix");
   REPAO(h_ShdMatrix4   )=GetShaderParam(S8+"ShdMatrix4["+i+']');

   // can be used for shadows in deferred and AO
   h_ShdBlur[0]=get("ShdBlur4");
   h_ShdBlur[1]=get("ShdBlur6");
   h_ShdBlur[2]=get("ShdBlur8");
   h_ShdBlur[3]=get("ShdBlur12");
   h_ShdBlurX  =get("ShdBlurX2");
   h_ShdBlurY  =get("ShdBlurY2");

#if !SLOW_SHADER_LOAD
   if(!D.deferredUnavailable())
   {
      // SHADOW
      REPD(m, (D.shaderModel()>=SM_4_1) ? 2 : 1)
      {
         REPD(n, 6)
         REPD(c, 2)h_ShdDir[n][c][m]=getShdDir (n+1, c, m);
                   h_ShdPnt      [m]=getShdPnt (        m);
                   h_ShdCone     [m]=getShdCone(        m);
      }

      // LIGHT
      REPD(m, (D.shaderModel()>=SM_4_1) ? 2 : 1) // multisample
      REPD(s, 2) // shadow
      REPD(q, 2) // quality unpack
      {
                   h_LightDir [s]   [m][q]=getLightDir (s,    m, q);
                   h_LightPnt [s]   [m][q]=getLightPnt (s,    m, q);
                   h_LightSqr [s]   [m][q]=getLightSqr (s,    m, q);
         REPD(i, 2)h_LightCone[s][i][m][q]=getLightCone(s, i, m, q);
      }

      // COL LIGHT
      REPD(m, (D.shaderModel()>=SM_4_1) ? 3 : 1)
      REPD(a, 2)
      REPD(c, 2)
      REPD(n, 2)h_ColLight[m][a][c][n]=getColLight(m, a, c, n);
   }
#endif

   // PARTICLE
   h_ParticleFrames=GetShaderParam("ParticleFrames");
   h_Bilb          =get("Bilb");
   REPD(p, 2)
   REPD(s, 2)
   REPD(a, 3)
   REPD(m, 2)
      h_Particle[p][s][a][m]=get(S8+"ParticleTex"+(p?'P':'\0')+(s?'S':'\0')+((a==0) ? "" : (a==1) ? "A" : "AA")+(m?'M':'\0'));

   // FOG
   h_VertexFogMulAdd=GetShaderParam("VertexFogMulAdd");
   h_VertexFogColor =GetShaderParam("VertexFogColor");

   h_FogColor_Density     =GetShaderParam(     "FogColor_Density");
   h_LocalFogColor_Density=GetShaderParam("LocalFogColor_Density");
   h_LocalFogInside       =GetShaderParam("LocalFogInside"       );
                                 h_Fog[0]=get("Fog");
   if(D.shaderModel()>=SM_4)
   {
                                 h_Fog[1]=get("FogN");
      if(D.shaderModel()>=SM_4_1)h_Fog[2]=get("FogM");
   }

#if !SLOW_SHADER_LOAD
   initFogBoxShaders ();
   initFogHgtShaders ();
   initFogBallShaders();
#endif

   // DECAL
   h_DecalParams  =GetShaderParam(  "DecalParams");
   h_OverlayParams=GetShaderParam("OverlayParams");

   // OTHER
   h_Laser[0]=get("Laser");
   h_Laser[1]=get("LaserN");
}
void MainShaderClass::connectRT()
{
   if(h_ImageShdMap[0])h_ImageShdMap[0]->set(Renderer._shd_map);
   if(h_ImageShdMap[1])h_ImageShdMap[1]->set(Renderer._cld_map);
}
/******************************************************************************/
// EFFECTS
/******************************************************************************/
void VolumetricCloudsFx::load()
{
   if(!shader)if(shader=ShaderFiles("Volumetric Clouds"))
   {
      h_Cloud     =GetShaderParam("Cloud");
      h_CloudMap  =GetShaderParam("CloudMap");
      h_Clouds    =shader->get("Clouds");
      h_CloudsDraw=shader->get("CloudsDraw");
      h_CloudsMap =shader->get("CloudsMap");
   }
}
/******************************************************************************/
void VolumetricLights::load()
{
   if(!shader)if(shader=ShaderFiles("Volumetric Lights"))
   {
      h_Light_point_range=GetShaderParam("Light_point_range");

      REPD(n, 6)
      REPD(c, 2)h_VolDir[n][c]=shader->get(S8+"VolDir"+(n+1)+(c?'C':'\0'));
                h_VolPnt      =shader->get(   "VolPnt" );
                h_VolSqr      =shader->get(   "VolSqr" );
                h_VolCone     =shader->get(   "VolCone");
      h_Volumetric =shader->get("Volumetric" );
      h_VolumetricA=shader->get("VolumetricA");
   }
}
/******************************************************************************/
void HDR::load()
{
   if(!shader)if(shader=ShaderFiles("Hdr"))
   {
      REPD(s, 2)h_HdrDS[s] =shader->get(S8+"HdrDS"+s  );
                h_HdrUpdate=shader->get(   "HdrUpdate");
                h_Hdr      =shader->get(   "Hdr"      );
   }   
}
/******************************************************************************/
void MotionBlur::load()
{
   if(!shader)if(shader=ShaderFiles("Motion Blur"))
   {
      h_MotionUVMulAdd     =GetShaderParam("MotionUVMulAdd");
      h_MotionVelScaleLimit=GetShaderParam("MotionVelScaleLimit");
      h_MotionPixelSize    =GetShaderParam("MotionPixelSize");

      h_Explosion=shader->get("Explosion");

      h_ClearSkyVel=shader->get("ClearSkyVel");

      REPD(h, 2)
      REPD(c, 2)h_Convert[h][c]=shader->get(S8+"Convert"+(h?'H':'\0')+(c?'C':'\0'));

      h_Dilate=shader->get("Dilate");

      REPD(c, 2)h_SetDirs[c]=shader->get(S8+"SetDirs"+(c?'C':'\0'));

      REPD(d, 2)h_Blur[d]=shader->get(S8+"Blur"+(d?'D':'\0'));

      pixels[0].pixels=1;
      pixels[1].pixels=2;
      pixels[2].pixels=4;
      pixels[3].pixels=6;
      pixels[4].pixels=8;
      pixels[5].pixels=12;
      pixels[6].pixels=16;
      pixels[7].pixels=24;
      pixels[8].pixels=32;
      ASSERT(ELMS(pixels)==9);
   }
}
C MotionBlur::Pixel* MotionBlur::pixel(Int pixel, Bool diagonal)
{
   if(pixel<=0)return null;
   Pixel *p;
   FREPA(pixels) // start from the smallest to find exact match or bigger, order is important
   {
      p=&pixels[i]; if(p->pixels>=pixel)break; // if this covers desired range of pixels to blur
   }
   if(!p->h_DilateX[diagonal])
   {
      p->h_DilateX[diagonal]=shader->get(S8+"DilateX"+(diagonal?'D':'\0')+p->pixels);
      p->h_DilateY[diagonal]=shader->get(S8+"DilateY"+(diagonal?'D':'\0')+p->pixels);
   }
   return p;
}
/******************************************************************************/
Shader* DepthOfField::getDS(Bool clamp , Bool realistic, Bool half) {return shader->get(S8+"DofDS"+(clamp?'C':'\0')+(realistic?'R':'\0')+(half?'H':'\0')+(D.shaderModel()>=SM_4_1 ? 'G' : '\0'));}
Shader* DepthOfField::get  (Bool dither, Bool realistic           ) {return shader->get(S8+"Dof"+(dither?'D':'\0')+(realistic?'R':'\0'));}

void DepthOfField::load()
{
   if(!shader)if(shader=ShaderFiles("Depth of Field"))
   {
      h_DofParams=GetShaderParam("DofParams");

   #if !SLOW_SHADER_LOAD
      REPD(c, 2)
      REPD(r, 2)
      REPD(h, 2)h_DofDS[c][r][h]=getDS(c, r, h);

      REPD(d, 2)
      REPD(r, 2)h_Dof[d][r]=get(d, r);
   #endif

      pixels[ 0].pixels=2;
      pixels[ 1].pixels=3;
      pixels[ 2].pixels=4;
      pixels[ 3].pixels=5;
      pixels[ 4].pixels=6;
      pixels[ 5].pixels=7;
      pixels[ 6].pixels=8;
      pixels[ 7].pixels=9;
      pixels[ 8].pixels=10;
      pixels[ 9].pixels=11;
      pixels[10].pixels=12;
      ASSERT(ELMS(pixels)==11);
   }
}
C DepthOfField::Pixel& DepthOfField::pixel(Int pixel)
{
   Pixel *p;
   FREPA(pixels) // start from the smallest to find exact match or bigger, order is important
   {
      p=&pixels[i]; if(p->pixels>=pixel)break; // if this covers desired range of pixels to blur
   }
   if(!p->h_BlurX)
   {
      p->h_BlurX=shader->get(S8+"DofBlurX"+p->pixels);
      p->h_BlurY=shader->get(S8+"DofBlurY"+p->pixels);
   }
   return *p;
}
/******************************************************************************/
void WaterShader::load()
{
   if(!shader)if(shader=ShaderFiles("Water"))
   {
      REPD(fake_reflect, 2) // fake reflection
      {
         h_Lake [fake_reflect]=shader->get(S8+"Lake" +(fake_reflect?'F':'\0'));
         h_River[fake_reflect]=shader->get(S8+"River"+(fake_reflect?'F':'\0'));
         h_Ocean[fake_reflect]=shader->get(S8+"Ocean"+(fake_reflect?'F':'\0'));

         REPD(shadow, 7)
         REPD(soft  , 2)
         {
            h_LakeL [fake_reflect][shadow][soft]=shader->get(S8+"LakeL" +shadow+(soft?'S':'\0')+(fake_reflect?'F':'\0'));
            h_RiverL[fake_reflect][shadow][soft]=shader->get(S8+"RiverL"+shadow+(soft?'S':'\0')+(fake_reflect?'F':'\0'));
            h_OceanL[fake_reflect][shadow][soft]=shader->get(S8+"OceanL"+shadow+(soft?'S':'\0')+(fake_reflect?'F':'\0'));
         }
      }
      REPD(r, 2)
      REPD(d, DX11 ? 2 : 1)h_Apply[r][d]=shader->get(S8+"Apply"+(r?'R':'\0')+(d?'D':'\0'));

      REPD(r, 2)h_Under[r]=shader->get(S8+"Under"+(r?'R':'\0'));
   }
}
/******************************************************************************/
}
/******************************************************************************/
