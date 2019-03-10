/******************************************************************************/
#if EE_PRIVATE
struct MainShaderClass
{
   Str         path;
   ShaderFile *shader;

   // get
   Shader* find(C Str8 &name); // find shader, null on fail
   Shader*  get(C Str8 &name); //  get shader, Exit on fail

   // effects
   static void clear(                C Vec4  &color,                                 C Rect *rect=null);
   static void draw (C Image &image,                                                 C Rect *rect=null);
   static void draw (C Image &image, C Color &color, C Color &color_add=TRANSPARENT, C Rect *rect=null);
   static void draw (C Image &image, C Vec4  &color, C Vec4  &color_add=Vec4Zero   , C Rect *rect=null);

   // private
   void del           ();
   void createSamplers();
   void create        ();
   void compile       ();
   void getTechniques ();
   void connectRT     ();

   ShaderImage
      *h_ImageCol[4], *h_ImageColMS,
      *h_ImageNrm[4], *h_ImageNrmMS,
      *h_ImageDet[4],
      *h_ImageMac[4],
      *h_ImageRfl[4],
      *h_ImageDepth, *h_ImageDepthMS,
      *h_ImageLum  , *h_ImageLumMS  ,
      *h_ImageShdMap[2],
      *h_ImageCub,
      *h_ImageVol[2];

   ShaderParam
      *h_ColSize    ,
      *h_ColClamp   ,
      *h_RTSizeI    ,
      *h_PixelOffset,
      *h_Coords     ,
      *h_Viewport   ,
      *h_DepthWeightScale,

      *h_CamAngVel ,
      *h_ObjAngVel ,
      *h_ObjVel    ,
      *h_ViewMatrix,
      *h_CamMatrix ,
      *h_ProjMatrix,
      *h_FurVel    ,
      *h_ClipPlane ,

      *h_Material        ,
      *h_MultiMaterial[4],

      *h_Light_dir ,
      *h_Light_pnt ,
      *h_Light_sqr ,
      *h_Light_cone,

      *h_Step         ,
      *h_Color[2]     ,
      *h_BehindBias   ,
      *h_AllowBackFlip,

      *h_VtxSkinning ,
      *h_VtxHeightmap,
      *h_VtxNrmMulAdd,

      *h_FontShadow  ,
      *h_FontContrast,
      *h_FontShade   ,
      *h_FontDepth   ,

      *h_LightMapScale   ,
      *h_LightMapColAdd  ,
      *h_LightMapSpecular,

      *h_GrassRangeMulAdd,
      *h_BendFactor,

      *h_Volume,

      *h_RippleParams,

      *h_AmbientMaterial,
      *h_AmbientContrast,
      *h_AmbientRange   ,
      *h_AmbientScale   ,
      *h_AmbientBias    ,
      *h_NightShadeColor,

      *h_HdrBrightness,
      *h_HdrMaxDark,
      *h_HdrMaxBright,
      *h_HdrWeight,

      *h_TesselationDensity,

      *h_Sun            ,
      *h_SkyFracMulAdd  ,
      *h_SkyDnsMulAdd   ,
      *h_SkyDnsExp      ,
      *h_SkyHorExp      ,
      *h_SkyBoxBlend    ,
      *h_SkyHorCol      ,
      *h_SkySkyCol      ,
      *h_SkyStarOrn     ,
      *h_SkySunHighlight,
      *h_SkySunPos      ,

      *h_FogColor_Density     ,
      *h_LocalFogColor_Density,
      *h_LocalFogInside       ,

      *h_VertexFogMulAdd,
      *h_VertexFogColor ,

      *h_ShdJitter     ,
      *h_ShdRange      ,
      *h_ShdRangeMulAdd,
      *h_ShdOpacity    ,
      *h_ShdStep[6]    ,
      *h_ShdMatrix     ,
      *h_ShdMatrix4[6] ,

      *h_ParticleFrames,

      *h_DecalParams,
      *h_OverlayParams,
      
      *h_SMAAThreshold;

   // SHADERS
   Shader
      *h_Draw2DFlat        ,
      *h_Draw3DFlat        ,
      *h_Draw2DCol         ,
      *h_Draw3DCol         ,
      *h_Draw2DTex         ,
      *h_Draw2DTexC        ,
      *h_Draw2DTexCol      ,
      *h_Draw3DTex   [2][2], // [AlphaTest] [Fog]
      *h_Draw3DTexCol[2][2], // [AlphaTest] [Fog]
      *h_DrawTexX          ,
    //*h_DrawTexZ          ,
      *h_DrawTexW          ,
      *h_DrawTexXC         ,
      *h_DrawTexWC         ,
      *h_DrawTexXCD        ,
      *h_DrawTexWCD        ,
    //*h_DrawTexNrm        ,
      *h_Simple            ,

      *h_DrawMask,
      *h_DrawCubeFace,

      *h_Font ,
      *h_FontD,

      *h_Laser[2],

      *h_PaletteDraw,

      // BASIC 2D
      *h_SetCol ,
      *h_Draw   ,
      *h_DrawC  ,
      *h_DrawA  ,
      *h_DrawMs1,
      *h_DrawMsN,
      *h_DrawMsM,

      // BLUR
      #define SHADER_BLUR_RANGE 5 // 5 pixel range in both directions
      *h_BlurX[2], // [High]
      *h_BlurY[2], // [High]
    /**h_BlurX_X,
      *h_BlurX_W,
      *h_BlurY_X,
      *h_BlurY_W,*/

      // MAX
      *h_MaxX,
      *h_MaxY,

      // VIDEO
      *h_YUV,
      *h_YUVA,

      // 2D FX
      *h_ColTrans       ,
      *h_ColTransHB     ,
      *h_ColTransHSB    ,
      *h_Ripple         ,
      *h_Titles         ,
      *h_Fade           ,
      *h_Wave           ,
      *h_RadialBlur     ,
      *h_Outline        ,
      *h_OutlineDS      ,
      *h_OutlineClip    ,
      *h_OutlineApply   ,
      *h_EdgeDetect     ,
      *h_EdgeDetectApply,
      *h_DetectMSCol    ,
    //*h_DetectMSNrm    ,

   #if DX9
      *h_LinearizeDepthRAWZ[2], // [Perspective]
   #endif
      *h_LinearizeDepth[2][3], // [Perspective] [MultiSample]
      *h_ResolveDepth,
      *h_SetDepth,
      *h_Dither,
      *h_Combine,
      *h_CombineMS,
      *h_CombineSS,
      *h_CombineSSAlpha,

      // FOG
      *h_Fog[3]  , // [MultiSample]
      *h_FogBox  ,
      *h_FogBox0 ,
      *h_FogBox1 ,
      *h_FogHgt  ,
      *h_FogHgt0 ,
      *h_FogHgt1 ,
      *h_FogBall ,
      *h_FogBall0,
      *h_FogBall1;
   void initFogBoxShaders ();   INLINE void loadFogBoxShaders () {if(SLOW_SHADER_LOAD)initFogBoxShaders ();}
   void initFogHgtShaders ();   INLINE void loadFogHgtShaders () {if(SLOW_SHADER_LOAD)initFogHgtShaders ();}
   void initFogBallShaders();   INLINE void loadFogBallShaders() {if(SLOW_SHADER_LOAD)initFogBallShaders();}

   Shader
      // VOLUME
      *h_Volume0[2], // [RedGreen as LumAlpha]
      *h_Volume1[2], // [RedGreen as LumAlpha]
      *h_Volume2[2], // [RedGreen as LumAlpha]

      // EDGE SOFTEN
      *h_FXAA     ,
   #if SUPPORT_MLAA
      *h_MLAAEdge ,
      *h_MLAABlend,
      *h_MLAA     ,
   #endif
      *h_SMAAEdge ,
      *h_SMAABlend,
      *h_SMAA     ,

      // PARTICLE
      *h_Bilb                ,
      *h_Particle[2][2][3][2], // [Palette] [Soft] [Anim] [Motion stretch affects opacity]

      // POINT
      *h_DrawTexPoint ,
      *h_DrawTexPointC,

      // CUBIC
      *h_DrawTexCubicFast    ,
      *h_DrawTexCubicFastC   ,
      *h_DrawTexCubicFast1   ,
      *h_DrawTexCubicFastD   ,
      *h_DrawTexCubicFastRGB ,
      *h_DrawTexCubicFastRGBD,
      *h_DrawTexCubic        ,
      *h_DrawTexCubicC       ,
      *h_DrawTexCubic1       ,
      *h_DrawTexCubicD       ,
      *h_DrawTexCubicRGB     ,
      *h_DrawTexCubicRGBD    ;
   void initCubicShaders();   INLINE void loadCubicShaders() {if(SLOW_SHADER_LOAD)initCubicShaders();}

   // SHADOWS
   Shader
      *h_ShdDir[6][2][2], // [NumberOfMaps] [Clouds] [MultiSample]
      *h_ShdPnt      [2], //                         [MultiSample]
      *h_ShdCone     [2], //                         [MultiSample]
      *h_ShdBlur     [4], // [Quality]
      *h_ShdBlurX       ,
      *h_ShdBlurY       ;
   Shader* getShdDir (Int map_num, Bool clouds, Bool multi_sample);
   Shader* getShdPnt (                          Bool multi_sample);
   Shader* getShdCone(                          Bool multi_sample);

   // LIGHT
   Shader
      *h_LightDir [2]   [2][2], // [Shadow]         [MultiSample] [QualityUnpack]
      *h_LightPnt [2]   [2][2], // [Shadow]         [MultiSample] [QualityUnpack]
      *h_LightSqr [2]   [2][2], // [Shadow]         [MultiSample] [QualityUnpack]
      *h_LightCone[2][2][2][2]; // [Shadow] [Image] [MultiSample] [QualityUnpack]
   Shader* getLightDir (Bool shadow,             Bool multi_sample, Bool quality);
   Shader* getLightPnt (Bool shadow,             Bool multi_sample, Bool quality);
   Shader* getLightSqr (Bool shadow,             Bool multi_sample, Bool quality);
   Shader* getLightCone(Bool shadow, Bool image, Bool multi_sample, Bool quality);

   // COL LIGHT
   Shader
      *h_ColLight[3][2][2][2]; // [Multisample] [AmbientOcclusion] [CelShade] [NightShade]
   Shader* getColLight(Int multi_sample, Bool ao, Bool cel_shade, Bool night_shade);

   // BLOOM
   ShaderParam
      *h_BloomParams;
   Shader
      *h_BloomDS[2][2][2][2], // [Glow] [UVClamp] [HalfRes] [Saturate]
      *h_Bloom  [2]         ; // [Dither]
   Shader* getBloomDS(Bool glow, Bool viewport_clamp, Bool half, Bool saturate);
   Shader* getBloom  (Bool dither);

   // SKY
   Shader
      *h_SunRaysMask[2]      , // [Mask]
      *h_SunRays    [2][2]   , // [High] [Jitter]
      *h_SkyTF[2]   [2]   [2], // [Textures(0->1, 1->2)]         [Cloud  ]               [Dither] (Textures   +Flat)
      *h_SkyT [2]      [3][2], // [Textures(0->1, 1->2)]                   [MultiSample] [Dither] (Textures        )
      *h_SkyAF[2][2][2]   [2], // [PerVertex           ] [Stars] [Cloud  ]               [Dither] (Atmospheric+Flat)
      *h_SkyA [2][2][2][3][2]; // [PerVertex           ] [Stars] [Density] [MultiSample] [Dither] (Atmospheric     )
   Shader* getSunRaysMask(Bool mask);
   Shader* getSunRays    (Bool high, Bool jitter);
   Shader* getSkyTF(Int textures,                Bool cloud  ,                   Bool dither);
   Shader* getSkyT (Int textures,                              Int multi_sample, Bool dither);
   Shader* getSkyAF(Bool per_vertex, Bool stars, Bool cloud  ,                   Bool dither);
   Shader* getSkyA (Bool per_vertex, Bool stars, Bool density, Int multi_sample, Bool dither);
}extern
   Sh;

struct AmbientOcclusion
{
   ShaderFile *shader;
   Shader     *h_AO[4][2][2]; // [Quality] [Jitter] [Normal]

   Shader* get(Int quality, Bool jitter, Bool normal);
}extern
   AO;

struct LayeredCloudsFx
{
   ShaderFile  *shader;
   ShaderParam *h_CL[4], *range;
   Shader      *h_Clouds[4][2][2]; // [#Layers] [Blend] [Draw Mask to 2nd RT]

   void    load();
   Shader* get(Int layers, Bool blend, Bool mask);
}extern
   LC;

struct VolumetricCloudsFx
{
   ShaderFile  *shader;
   ShaderParam *h_Cloud, *h_CloudMap;
   Shader      *h_Clouds, *h_CloudsDraw, *h_CloudsMap;

   void load();
}extern
   VolCloud;

struct VolumetricLights
{
   ShaderFile  *shader;
   ShaderParam *h_Light_point_range;
   Shader      *h_VolDir[6][2], // [ShdMapNum] [Clouds]
               *h_VolPnt      ,
               *h_VolSqr      ,
               *h_VolCone     ,
               *h_Volumetric  ,
               *h_VolumetricA ;

   void load();
}extern
   VL;

struct HDR
{
   ShaderFile *shader;
   Shader     *h_HdrDS[2] , // [Step]
              *h_HdrUpdate,
              *h_Hdr      ;

   void load();
}extern
   Hdr;

struct MotionBlur
{
   ShaderFile  *shader;
   ShaderParam *h_MotionUVMulAdd     ,
               *h_MotionVelScaleLimit,
               *h_MotionPixelSize    ;
   Shader      *h_Explosion          ,
               *h_ClearSkyVel        ,
               *h_Convert      [2][2], // [High][Clamp]
               *h_Dilate             ,
               *h_SetDirs      [2]   , // [Clamp]
               *h_Blur         [2]   ; // [Dither]

   struct Pixel
   {
      Int     pixels;
      Shader *h_DilateX[2], // [Diagonal]
             *h_DilateY[2]; // [Diagonal]
   }pixels[9];

   void load();
 C Pixel* pixel(Int pixel, Bool diagonal);

}extern
   Mtn;

struct DepthOfField
{
   ShaderFile  *shader;
   ShaderParam *h_DofParams;
   Shader      *h_DofDS[2][2][2], // [Clamp ][Realistic][Half]
               *h_Dof  [2][2]   ; // [Dither][Realistic]

   struct Pixel
   {
      Int     pixels;
      Shader *h_BlurX,
             *h_BlurY;
   }pixels[11];

   void load();
   Shader* getDS(Bool clamp , Bool realistic, Bool half);
   Shader* get  (Bool dither, Bool realistic);
 C Pixel& pixel(Int pixel);

}extern
   Dof;

struct WaterShader
{
   ShaderFile *shader;
   Shader     *h_Ocean [2]      , // [FakeReflect]
              *h_Lake  [2]      , // [FakeReflect]
              *h_River [2]      , // [FakeReflect]
              *h_OceanL[2][7][2], // [FakeReflect] [Shadows] [Soft]
              *h_LakeL [2][7][2], // [FakeReflect] [Shadows] [Soft]
              *h_RiverL[2][7][2], // [FakeReflect] [Shadows] [Soft]
              *h_Apply [2][2]   , // [Refract] [Depth]
              *h_Under [2]      ; // [Refract]

   void load();
}extern
   WS;

extern ShaderImage::Sampler SamplerPoint, SamplerLinearWrap, SamplerLinearWCC, SamplerLinearCWC, SamplerLinearCWW, SamplerLinearClamp, SamplerFont, SamplerAnisotropic, SamplerShadowMap;

void CreateAnisotropicSampler();
void CreateFontSampler       ();

Str8 TechNameSimple    (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel, Int tess);
Str8 TechNameDeferred  (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int detail, Int macro, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tess);
Str8 TechNameForward   (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int detail, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx,   Int light_dir, Int light_dir_shd, Int light_dir_shd_num,   Int light_point, Int light_point_shd,   Int light_sqr, Int light_sqr_shd,   Int light_cone, Int light_cone_shd,   Int tess);
Str8 TechNameBlendLight(Int skin, Int color    , Int textures, Int bump_mode, Int alpha_test, Int alpha, Int light_map, Int rflct, Int fx, Int per_pixel, Int shadow_maps);
Str8 TechNamePosition  (Int skin, Int textures, Int test_blend, Int fx, Int tess);
Str8 TechNameBlend     (Int skin, Int color, Int rflct, Int textures, Int light_map);
Str8 TechNameSetColor  (Int skin, Int textures, Int tess);
Str8 TechNameBehind    (Int skin, Int textures);
Str8 TechNameEarlyZ    (Int skin);
Str8 TechNameAmbient   (Int skin, Int alpha_test);
Str8 TechNameOverlay   (Int skin, Int normal);
Str8 TechNameFurBase   (Int skin, Int size, Int diffuse);
Str8 TechNameFurSoft   (Int skin, Int size, Int diffuse);
Str8 TechNameTattoo    (Int skin, Int tess);

struct DefaultShaders
{
   Bool valid,
        detail, macro, reflect, light_map,
        mtrl_blend,
        heightmap,
        normal, color, size,
        fur, blend, grass, leaf, ambient,
        alpha, alpha_test, alpha_blend, alpha_blend_light,
        skin,
        tess;
   Byte materials, textures, bump, fx;

   void      init(C Material *material[4], UInt mesh_base_flag, Int lod_index, Bool heightmap);
   DefaultShaders(C Material *material[4], UInt mesh_base_flag, Int lod_index, Bool heightmap) {init(material, mesh_base_flag, lod_index, heightmap);}
   DefaultShaders(C Material *material   , UInt mesh_base_flag, Int lod_index, Bool heightmap);

   Shader* EarlyZ ()C;
   Shader* Simple ()C;
   Shader* Solid  (Bool mirror=false)C;
   Shader* Ambient()C;
   Shader* Outline()C;
   Shader* Behind ()C;
   Shader* Fur    ()C;
   Shader* Shadow ()C;
   Shader* Blend  ()C;
   Shader* Overlay()C;
   Shader* get    (RENDER_MODE mode)C;
   FRST  * Frst   ()C;
   BLST  * Blst   ()C;

   void set(Shader *shader[RM_SHADER_NUM], FRST **frst, BLST **blst);
};
#endif
/******************************************************************************/
