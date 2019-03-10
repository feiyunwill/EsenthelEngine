/******************************************************************************/
class VideoOptions : PropWin
{
   class Advanced : PropWin
   {
      static cchar8 *TexFilter_t[]=
      {
         "1",
         "2",
         "4",
         "8",
         "16",
      };
      static cchar8 *Density_t[]=
      {
         "0.25",
         "0.50",
         "0.75",
         "1.00",
         "1.25",
         "1.50",
         "2.00",
      };
      static cchar8 *DensityFilter_t[]=
      {
         "None",
         "Linear",
         "Cubic",
         "Cubic+ (slow)",
      };
      static FILTER_TYPE DensityFilter_v[]=
      {
         FILTER_NONE,
         FILTER_LINEAR,
         FILTER_CUBIC_FAST,
         FILTER_CUBIC,
      };
      static cchar8 *TexUse_t[]=
      {
         "Never",            // 0
         "Single Materials", // 1
         "Multi Materials",  // 2
      }; ASSERT(TEX_USE_DISABLE==0 && TEX_USE_SINGLE==1 && TEX_USE_MULTI==2);
      static cchar8 *EdgeDetect_t[]=
      {
         "Off",  // 0
         "Thin", // 1
         "Fat",  // 2
      }; ASSERT(EDGE_DETECT_NONE==0 && EDGE_DETECT_THIN==1 && EDGE_DETECT_FAT==2);
      static cchar8 *Precision_t[]=
      {
          "8 bit", // 0
         "10 bit", // 1
      }; ASSERT(IMAGE_PRECISION_8==0 && IMAGE_PRECISION_10==1);
      static cchar8 *Stage_t[]=
      {
         "Default"          , // 0
         "Depth"            , // 1
         "Color (Unlit)"    , // 2
         "Normal"           , // 3
         "Velocity"         , // 4
         "Light"            , // 5
         "Light + AO"       , // 6
         "Ambient Occlusion", // 7
         "Color (Lit)"      , // 8
         "Reflection"       , // 9
         "Water Color"      , // 10
         "Water Normal"     , // 11
         "Water Light"      , // 12
      }; ASSERT(RS_DEFAULT==0 && RS_DEPTH==1 && RS_COLOR==2 && RS_NORMAL==3 && RS_VEL==4 && RS_LIGHT==5 && RS_LIGHT_AO==6 && RS_AO==7 && RS_LIT_COLOR==8
             && RS_REFLECTION==9 && RS_WATER_COLOR==10 && RS_WATER_NORMAL==11 && RS_WATER_LIGHT==12);
      static cchar8 *ShadowReduceFlicker_t[]=
      {
         "Off",
         "Medium",
         "High",
      };

      static Str  Fov          (C Advanced &adv             ) {return RadToDeg(adv.fov);}
      static void Fov          (  Advanced &adv, C Str &text) {adv.setFov(DegToRad(TextFlt(text)));}
      static Str  TexFilter    (C Advanced &adv             ) {Str f=D.texFilter(); REPA(TexFilter_t)if(f==TexFilter_t[i])return i; return -1;}
      static void TexFilter    (  Advanced &adv, C Str &text) {int i=TextInt(text); if(InRange(i, TexFilter_t))D.texFilter(TextInt(TexFilter_t[i]));}
      static Str  TexMipFilter (C Advanced &adv             ) {return D.texMipFilter();}
      static void TexMipFilter (  Advanced &adv, C Str &text) {D.texMipFilter(TextBool(text));}
      static Str  DetailTexUse (C Advanced &adv             ) {return D.texDetail();}
      static void DetailTexUse (  Advanced &adv, C Str &text) {D.texDetail(TEXTURE_USAGE(TextInt(text)));}
      static Str  ReflectTexUse(C Advanced &adv             ) {return D.texReflection();}
      static void ReflectTexUse(  Advanced &adv, C Str &text) {D.texReflection(TEXTURE_USAGE(TextInt(text)));}
      static Str  Samples      (C Advanced &adv             ) {return D.samples()>1;}
      static void Samples      (  Advanced &adv, C Str &text) {D.samples(TextBool(text) ? 4 : 1); VidOpt.setVis();}
      static Str  Density      (C Advanced &adv             ) {int nearest=-1; flt dist; REPA(Density_t){flt d=Abs(D.density()-TextFlt(Density_t[i])); if(nearest<0 || d<dist){nearest=i; dist=d;}} return nearest;}
      static void Density      (  Advanced &adv, C Str &text) {int i=TextInt(text); if(InRange(i, Density_t))D.density(TextFlt(Density_t[i]));}
      static Str  DensityFilter(C Advanced &adv             ) {REPA(DensityFilter_v)if(D.densityFilter()==DensityFilter_v[i])return i; return S;}
      static void DensityFilter(  Advanced &adv, C Str &text) {int i=TextInt(text); if(InRange(i, DensityFilter_v))D.densityFilter(DensityFilter_v[i]);}
      static Str  GrassRange   (C Advanced &adv             ) {return D.grassRange();}
      static void GrassRange   (  Advanced &adv, C Str &text) {D.grassRange(TextFlt(text)); WorldEdit.setObjVisibility();} // grass objects will be hidden for 0
      static Str  GrassDensity (C Advanced &adv             ) {return D.grassDensity();}
      static void GrassDensity (  Advanced &adv, C Str &text) {D.grassDensity(TextFlt(text));}
      static Str  SoftParticle (C Advanced &adv             ) {return D.particlesSoft();}
      static void SoftParticle (  Advanced &adv, C Str &text) {D.particlesSoft(TextBool(text));}
    //static Str  ColorPalette (C Advanced &adv             ) {return Renderer.color_palette.name();}
    //static void ColorPalette (  Advanced &adv, C Str &text) {Renderer.color_palette.get(ImageName(text));}
    //static Str  ColorPalette1(C Advanced &adv             ) {return Renderer.color_palette1.name();}
    //static void ColorPalette1(  Advanced &adv, C Str &text) {Renderer.color_palette1.get(ImageName(text));}
      static Str  VolLight     (C Advanced &adv             ) {return D.volLight();}
      static void VolLight     (  Advanced &adv, C Str &text) {D.volLight(TextBool(text));}
      static Str  MaxLights    (C Advanced &adv             ) {return D.maxLights();}
      static void MaxLights    (  Advanced &adv, C Str &text) {D.maxLights(TextInt(text));}
      static Str  EdgeDetect   (C Advanced &adv             ) {return D.edgeDetect();}
      static void EdgeDetect   (  Advanced &adv, C Str &text) {D.edgeDetect(EDGE_DETECT_MODE(TextInt(text)));}
      static Str  Stage        (C Advanced &adv             ) {return Renderer.stage;}
      static void Stage        (  Advanced &adv, C Str &text) {Renderer.stage=(RENDER_STAGE)TextInt(text);}
      static Str  EyeAdaptBrigh(C Advanced &adv             ) {return D.eyeAdaptationBrightness();}
      static void EyeAdaptBrigh(  Advanced &adv, C Str &text) {       D.eyeAdaptationBrightness(TextFlt(text));}
      static Str  Exclusive    (C Advanced &adv             ) {return D.exclusive();}
      static void Exclusive    (  Advanced &adv, C Str &text) {       D.exclusive(TextBool(text));}
      static Str  MonitorPrec  (C Advanced &adv             ) {return D.monitorPrecision();}
      static void MonitorPrec  (  Advanced &adv, C Str &text) {       D.monitorPrecision(IMAGE_PRECISION(TextInt(text)));}
      static Str  Dither       (C Advanced &adv             ) {return D.dither();}
      static void Dither       (  Advanced &adv, C Str &text) {       D.dither(TextBool(text));}
      static Str  ColRTPrec    (C Advanced &adv             ) {return D.highPrecColRT();}
      static void ColRTPrec    (  Advanced &adv, C Str &text) {       D.highPrecColRT(TextBool(text));}
      static Str  NrmCalcPrec  (C Advanced &adv             ) {return D.highPrecNrmCalc();}
      static void NrmCalcPrec  (  Advanced &adv, C Str &text) {       D.highPrecNrmCalc(TextBool(text));}
      static Str  NrmRTPrec    (C Advanced &adv             ) {return D.highPrecNrmRT();}
      static void NrmRTPrec    (  Advanced &adv, C Str &text) {       D.highPrecNrmRT(TextBool(text));}
      static Str  LumRTPrec    (C Advanced &adv             ) {return D.highPrecLumRT();}
      static void LumRTPrec    (  Advanced &adv, C Str &text) {       D.highPrecLumRT(TextBool(text));}
      static Str  LitColRTPrec (C Advanced &adv             ) {return D.litColRTPrecision();}
      static void LitColRTPrec (  Advanced &adv, C Str &text) {       D.litColRTPrecision(IMAGE_PRECISION(TextInt(text)));}
      static Str  BloomSat     (C Advanced &adv             ) {return DefaultEnvironment.bloom.saturate;}
      static void BloomSat     (  Advanced &adv, C Str &text) {       DefaultEnvironment.bloom.saturate=TextBool(text);}
      static Str  AmbLight     (C Advanced &adv             ) {return DefaultEnvironment.ambient.color.max();}
      static void AmbLight     (  Advanced &adv, C Str &text) {       DefaultEnvironment.ambient.color=TextFlt(text);}
      static Str  AOContrast   (C Advanced &adv             ) {return D.ambientContrast();}
      static void AOContrast   (  Advanced &adv, C Str &text) {       D.ambientContrast(TextFlt(text));}
      static Str  AORange      (C Advanced &adv             ) {return D.ambientRange().x;}
      static void AORange      (  Advanced &adv, C Str &text) {       D.ambientRange(TextFlt(text));}
      static Str  ShadowFlicker(C Advanced &adv             ) {return D.shadowReduceFlicker();}
      static void ShadowFlicker(  Advanced &adv, C Str &text) {D.shadowReduceFlicker(TextInt(text));}
      static Str  ShadowFrac   (C Advanced &adv             ) {return D.shadowFrac();}
      static void ShadowFrac   (  Advanced &adv, C Str &text) {D.shadowFrac(TextFlt(text));}
      static Str  ShadowFade   (C Advanced &adv             ) {return D.shadowFade();}
      static void ShadowFade   (  Advanced &adv, C Str &text) {D.shadowFade(TextFlt(text));}
      static Str  AllowGlow    (C Advanced &adv             ) {return D.glowAllow();}
      static void AllowGlow    (  Advanced &adv, C Str &text) {D.glowAllow(TextBool(text));}
      static Str  SimplePrec   (C Advanced &adv             ) {return Renderer.simplePrecision();}
      static void SimplePrec   (  Advanced &adv, C Str &text) {Renderer.simplePrecision(TextBool(text));}
      static Str  MaterialBlend(C Advanced &adv             ) {return D.materialBlend();}
      static void MaterialBlend(  Advanced &adv, C Str &text) {D.materialBlend(TextBool(text));}
      static Str  TexLod       (C Advanced &adv             ) {return D.texLod();}
      static void TexLod       (  Advanced &adv, C Str &text) {D.texLod(TextInt(text));}

      flt fov;

      void setFov(flt fov)
      {
         Clamp(fov, DegToRad(0.001), DegToRad(120));
         T.fov=fov;
         D.viewFov(fov);
             ObjEdit.v4.perspFov(fov);
            AnimEdit.v4.perspFov(fov);
           WorldEdit.v4.perspFov(fov);
         TexDownsize.v4.perspFov(fov);
      }
      void ctor()
      {
         fov=DefaultFOV; // initialize here in case it gets initialized before DefaultFov, this may help on Linux where it's by default zero
      }
      void create()
      {
         int  tex_filter=Elms(TexFilter_t); FREPA(TexFilter_t)if(TextInt(TexFilter_t[i])>D.maxTexFilter()){tex_filter=i; break;}
         props.New().create("Field of View"        , MemberDesc(DATA_REAL).setFunc(Fov          , Fov          )).range(0.001, 120).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).desc("Set Field of View");
         props.New().create("Exclusive Fullscreen" , MemberDesc(DATA_BOOL).setFunc(Exclusive    , Exclusive    )).desc("If fullscreen mode should be exclusive\nExclusive mode offers better performance\nNon-exclusive mode offers faster Alt+Tab switching");
         props.New().create("Texture Filtering"    , MemberDesc(         ).setFunc(TexFilter    , TexFilter    )).setEnum(TexFilter_t, tex_filter).desc("Configure Texture Anisotropic Filtering Quality");
         props.New().create("Texture Mip Filtering", MemberDesc(DATA_BOOL).setFunc(TexMipFilter , TexMipFilter )).desc("Configure Texture Mip Map Filtering");
         props.New().create("Detail Texture"       , MemberDesc(         ).setFunc(DetailTexUse , DetailTexUse )).setEnum(TexUse_t, Elms(TexUse_t)).desc("Set when Detail Texture is used");
         props.New().create("Reflection Texture"   , MemberDesc(         ).setFunc(ReflectTexUse, ReflectTexUse)).setEnum(TexUse_t, Elms(TexUse_t)).desc("Set when Reflection Texture is used");
      #if WINDOWS
         props.New().create("Multi Sampling"       , MemberDesc(DATA_BOOL).setFunc(Samples, Samples)).desc("Set Multi Sampling Anti-Aliasing");
      #endif
         props.New().create("Pixel Density"        , MemberDesc(         ).setFunc(Density      , Density      )).setEnum(Density_t      , Elms(Density_t      )).desc("Set Rendering Pixel Density");
         props.New().create("Upscale Filtering"    , MemberDesc(         ).setFunc(DensityFilter, DensityFilter)).setEnum(DensityFilter_t, Elms(DensityFilter_t)).desc("Set Pixel Density Filtering when Upscaling");
         props.New().create(MLTC(u"Grass Range", PL, u"Zasięg Trawy", DE, u"Gras Reichweite", RU, u"Диапазон травы", PO, u"Alcance da relva"), MemberDesc(DATA_INT ).setFunc(GrassRange  , GrassRange  )).range(0, 1000).desc("Set visible grass range\nvalue of 0 hides grass objects");
       //props.New().create("Grass Density"        , MemberDesc(DATA_REAL).setFunc(GrassDensity, GrassDensity)).range(0, 1).desc("Set visible grass density");
         props.New().create("Soft Particles"       , MemberDesc(DATA_BOOL).setFunc(SoftParticle, SoftParticle)).desc("Enable Soft Particles");
         // TODO: use ELM ID IMAGE for color palette
       //props.New().create("Color Palette"        , MemberDesc(DATA_IMAGEPTR).setFunc(ColorPalette , ColorPalette )).setFile(SUPPORTED_IMAGE_EXT, "image").desc("Specify Color Palette used for RM_PALETTE rendering,\nsuch as palette based particles.\nClear to empty to speedup rendering.");
       //props.New().create("Color Palette 1"      , MemberDesc(DATA_IMAGEPTR).setFunc(ColorPalette1, ColorPalette1)).setFile(SUPPORTED_IMAGE_EXT, "image").desc("Specify Color Palette used for RM_PALETTE1 rendering,\nsuch as palette based particles.\nClear to empty to speedup rendering.");
       //props.New().create("Volumetric Lighting"  , MemberDesc(DATA_BOOL).setFunc(VolLight, VolLight)).desc("Enable Volumetric Lighting for Ligts");
         props.New().create("Max Lights"                 , MemberDesc(DATA_INT ).setFunc(MaxLights    , MaxLights    )).range(0, 255).desc("Limit number of lights on the scene (0=unlimited)");
         props.New().create("Reduce Shadow Flickering"   , MemberDesc(DATA_BOOL).setFunc(ShadowFlicker, ShadowFlicker))/*.setEnum(ShadowReduceFlicker_t, Elms(ShadowReduceFlicker_t))*/.desc("This option reduces directional light shadow map flickering when rotating the camera,\nhowever at the expense of slightly increasing the shadow map blockiness.\nEnable only when the flickering is really disturbing.");
         props.New().create("Shadow Range Fraction"      , MemberDesc(DATA_REAL).setFunc(ShadowFrac   , ShadowFrac   )).range(0, 1).desc("This option can limit directional lights shadowing range to a fraction of the viewport range.");
         props.New().create("Shadow Fade  Fraction"      , MemberDesc(DATA_REAL).setFunc(ShadowFade   , ShadowFade   )).range(0, 1).desc("This option specifies at which part of the shadowing range,\nshadow fading occurs for directional lights.");
         props.New().create("Edge Detect"                , MemberDesc(         ).setFunc(EdgeDetect   , EdgeDetect   )).setEnum(EdgeDetect_t, Elms(EdgeDetect_t)).desc("Detect Edges");
         props.New().create("Rendering Stage"            , MemberDesc(         ).setFunc(Stage        , Stage        )).setEnum(Stage_t,  Elms(Stage_t)).desc("Display specified rendering stage.\nSome options are available only in Deferred Renderer.");
         props.New().create("Eye Adaptation Brightness"  , MemberDesc(DATA_REAL).setFunc(EyeAdaptBrigh, EyeAdaptBrigh)).range(0, 2).desc("Total light scale for Eye Adaptation Effect");
         props.New().create("Dither"                     , MemberDesc(DATA_BOOL).setFunc(Dither       , Dither       )).desc("If enable color dithering, which smoothens color gradients.");
         props.New().create("Monitor Precision"          , MemberDesc(         ).setFunc(MonitorPrec  , MonitorPrec  )).setEnum(Precision_t, Elms(Precision_t)).desc("Specify the exact precision of your Monitor Screen.\n8 bit per channel = 24 bit total\n10 bit per channel = 30 bit total\nIf you're not sure what your monitor supports, leave this option at \"8 bit\"\n\nAvoid setting higher precision than what your screen can actually support,\nbecause instead of getting higher quality results you will get lower quality.");
         props.New().create("High Precision Lit Color RT", MemberDesc(DATA_BOOL).setFunc(LitColRTPrec , LitColRTPrec )).desc("Enable high precision lit color render target\nThis increases precision of colors adjusted by lighting.");
         props.New().create("High Precision Color RT"    , MemberDesc(DATA_BOOL).setFunc(ColRTPrec    , ColRTPrec    )).desc("Enable high precision color render target\nThis increases precision of material color textures in Deferred Renderer.");
         props.New().create("High Precision Normal Calc" , MemberDesc(DATA_BOOL).setFunc(NrmCalcPrec  , NrmCalcPrec  )).desc("Enable high precision normal calculation\nThis increases precision of specular lighting in Deferred Renderer.");
         props.New().create("High Precision Normal RT"   , MemberDesc(DATA_BOOL).setFunc(NrmRTPrec    , NrmRTPrec    )).desc("Enable high precision normal render target\nThis increases precision of specular lighting in Deferred Renderer.");
         props.New().create("High Precision Light RT"    , MemberDesc(DATA_BOOL).setFunc(LumRTPrec    , LumRTPrec    )).desc("Enable high precision light render target\nThis increases lighting precision in Deferred Renderer.");
         props.New().create("Bloom Saturate"             , MemberDesc(DATA_BOOL).setFunc(BloomSat     , BloomSat     ));
         props.New().create("Ambient Light"              , MemberDesc(DATA_REAL).setFunc(AmbLight     , AmbLight     )).range(0, 1);
         props.New().create("Ambient Occlusion Contrast" , MemberDesc(DATA_REAL).setFunc(AOContrast   , AOContrast   )).range(0, 2);
         props.New().create("Ambient Occlusion Range"    , MemberDesc(DATA_REAL).setFunc(AORange      , AORange      )).range(0, 2);
         props.New().create("Allow Glow"                 , MemberDesc(DATA_BOOL).setFunc(AllowGlow    , AllowGlow    )).desc("If allow glow effect on the scene when detected.");
         props.New().create("Material Blend Per Pixel"   , MemberDesc(DATA_BOOL).setFunc(MaterialBlend, MaterialBlend)).desc("If Multiple Materials should be blended with per-pixel precision.\nFor this effect to work, your Materials should have a bump map.");
         props.New().create("Simple Renderer Per Pixel"  , MemberDesc(DATA_BOOL).setFunc(SimplePrec   , SimplePrec   )).desc("If Simple renderer should use per-pixel precision,\nper-vertex precision is used otherwise.");
      #if WINDOWS
         props.New().create("Tex Lod"                    , MemberDesc(DATA_INT ).setFunc(TexLod       , TexLod       )).desc("Minimum Texture LOD usage").range(0, 14).mouseEditSpeed(1);
      #endif

         super.create("Advanced Video Options"); autoData(this); button[2].show();
      }
      virtual Advanced& hide()override
      {
         VidOpt.advanced_show.set(false, QUIET);
         super.hide();
         return T;
      }
   }

   static cchar8 *Render_t[]=
   {
      "Deferred", // 0
      "Forward" , // 1
      "Simple"  , // 2
   }; ASSERT(RT_DEFERRED==0 && RT_FORWARD==1 && RT_SIMPLE==2);
   static cchar8 *EdgeSoften_t[]=
   {
      "Off" , // 0
      "FXAA", // 1
   #if SUPPORT_MLAA
      "MLAA", // 2
   #endif
      "SMAA", // 2
   }; ASSERT(EDGE_SOFTEN_NONE==0 && EDGE_SOFTEN_FXAA==1 && !SUPPORT_MLAA && EDGE_SOFTEN_SMAA==2+SUPPORT_MLAA);
   static cchar8 *ShadowSize_t[]=
   {
      "512",
      "768",
      "1024",
      "1536",
      "2048",
   };
   static cchar8 *ShadowNum_t[]=
   {
      "1",
      "2",
      "3",
      "4",
      "5",
      "6",
   };
   static cchar8 *ShadowSoft_t[]=
   {
      "Off", // 0
      "1", // 1
      "2", // 2
      "3", // 3
      "4", // 4
      "5", // 5
   }; ASSERT(SHADOW_SOFT_NUM==6);
   static cchar8 *BumpMode_t[]=
   {
      "Flat"    , // 0
      "Normal"  , // 1
      "Parallax", // 2
      "Relief"  , // 3
   }; ASSERT(BUMP_FLAT==0 && BUMP_NORMAL==1 && BUMP_PARALLAX==2 && BUMP_RELIEF==3 && BUMP_NUM==4);
   static cchar8 *MotionMode_t[]=
   {
      "Off",
      "Camera Only",
      "Per Object",
   };
   static cchar8 *AO_t[]=
   {
      "Off",
      "Low",
      "Medium",
      "High",
      "Ultra",
   };

   class Skin
   {
      cchar8 *name;
      UID     id;
   }
   static Skin skins[]=
   {
      {"Light"          , UID(3649875776, 1074192063, 580730756, 799774185)}, // set default to be first
      {"Light+ (Slower)", UID(488640649, 1158676950, 284834467, 1632509456)},
      {"Light Grey"     , UID(3437250805, 1246126940, 1016483747, 2787407393)},
      {"Dark Grey"      , UID(3070792110, 1173521724, 2244074370, 3689187718)},
      {"Dark"           , UID(3068860333, 1264140570, 2890469249, 3573156331)},
      {"Dark+ (Slower)" , UID(662686602, 1124017201, 3699889282, 3574186813)},
      {"Neon"           , UID(3258331985, 1215022077, 3210484880, 2285543245)},
      {"Neon+ (Slower)" , UID(1080853348, 1102506000, 3817480329, 1344453301)},
   };

   static void Mode       (  VideoOptions &vo, C Str &t) {int m=TextInt(t); if(InRange(m, D.modes()))D.mode(D.modes()[m].x, D.modes()[m].y);}
   static Str  Full       (C VideoOptions &vo          ) {return D.full();}
   static void Full       (  VideoOptions &vo, C Str &t) {       D.full(TextBool(t));}
   static Str  Sync       (C VideoOptions &vo          ) {return D.sync();}
   static void Sync       (  VideoOptions &vo, C Str &t) {       D.sync(TextBool(t));}
   static Str  Render     (C VideoOptions &vo          ) {return Renderer.type();}
   static void Render     (  VideoOptions &vo, C Str &t) {       Renderer.type(RENDER_TYPE(TextInt(t))); vo.setVis();}
   static Str  EdgeSoft   (C VideoOptions &vo          ) {return D.edgeSoften();}
   static void EdgeSoft   (  VideoOptions &vo, C Str &t) {       D.edgeSoften(EDGE_SOFTEN_MODE(TextInt(t)));}
   static Str  Shadow     (C VideoOptions &vo          ) {return D.shadowMode()==SHADOW_MAP;}
   static void Shadow     (  VideoOptions &vo, C Str &t) {       D.shadowMode(TextBool(t) ? SHADOW_MAP : SHADOW_NONE); vo.setVis(); if(Demo && TextBool(t))Gui.msgBox(S, "Shadows are available only in the full version");}
   static Str  ShadowSize (C VideoOptions &vo          ) {REPA(ShadowSize_t)if(D.shadowMapSize()>=TextInt(ShadowSize_t[i]))return i; return "2";} // go from end to check biggest first
   static void ShadowSize (  VideoOptions &vo, C Str &t) {int s=TextInt(t); if(InRange(s, ShadowSize_t))D.shadowMapSize(TextInt(ShadowSize_t[s]));}
   static Str  ShadowNum  (C VideoOptions &vo          ) {return D.shadowMapNum()-1;}
   static void ShadowNum  (  VideoOptions &vo, C Str &t) {       D.shadowMapNum(TextInt(t)+1);}
   static Str  ShadowSoft (C VideoOptions &vo          ) {return D.shadowSoft();}
   static void ShadowSoft (  VideoOptions &vo, C Str &t) {       D.shadowSoft(TextInt(t));}
   static Str  ShadowJit  (C VideoOptions &vo          ) {return D.shadowJitter();}
   static void ShadowJit  (  VideoOptions &vo, C Str &t) {       D.shadowJitter(TextBool(t));}
   static Str  BumpMode   (C VideoOptions &vo          ) {return D.bumpMode();}
   static void BumpMode   (  VideoOptions &vo, C Str &t) {       D.bumpMode(BUMP_MODE(TextInt(t)));}
   static Str  MotionMode (C VideoOptions &vo          ) {return D.motionMode();}
   static void MotionMode (  VideoOptions &vo, C Str &t) {       D.motionMode(MOTION_MODE(TextInt(t))); vo.setVis();}
   static Str  AO         (C VideoOptions &vo          ) {return D.ambientMode();}
   static void AO         (  VideoOptions &vo, C Str &t) {       D.ambientMode(AMBIENT_MODE(TextInt(t)));}
   static Str  EyeAdapt   (C VideoOptions &vo          ) {return D.eyeAdaptation();}
   static void EyeAdapt   (  VideoOptions &vo, C Str &t) {       D.eyeAdaptation(TextBool(t));}
   static Str  Scale      (C VideoOptions &vo          ) {return vo.scale;}
   static void Scale      (  VideoOptions &vo, C Str &t) {       vo.setScale(TextFlt(t));}
   static Str  ScaleWin   (C VideoOptions &vo          ) {return vo.scale_win;}
   static void ScaleWin   (  VideoOptions &vo, C Str &t) {       vo.setScaleWin(TextBool(t));}
   static void SkinChanged(  VideoOptions &vo          ) {if(vo.skin && InRange(vo.skin.combobox(), skins))SetGuiSkin(skins[vo.skin.combobox()].id);}

   Property *full=null, *mode=null, *shd=null, *shd_siz=null, *shd_num=null, *shd_sft=null, *shd_jit=null, *bump=null, *ao=null, *skin=null;
   Button    advanced_show;
   Advanced  advanced;
   flt       scale=1;
   bool      scale_win=true;

   static void ShowAdvanced(VideoOptions &vo) {vo.advanced.visibleActivate(vo.advanced_show());}

   void setScale()
   {
      D.scale(scale_win ? scale*D.screenH()/flt(D.resH())*(950./1080) : scale);
   }
   void setScale   (flt  scale) {T.scale    =scale; setScale();}
   void setScaleWin(bool scale) {T.scale_win=scale; setScale();}
   UID  skinID   (C Str &name)C {REPA(skins)if(skins[i].name==name)return skins[i].id; return UIDZero;}
   int  skinIndex(C UID &id  )C {REPA(skins)if(skins[i].id  ==id  )return i; return -1;}
   Str  skinName (           )C {return skin ? skin.combobox.text : S;}
   void resize()
   {
      if(mode)mode.combobox.setText(VecI2(D.resW(), D.resH()), true, QUIET);
      if(full)full.set(D.full(), QUIET);
   }
   void ctor()
   {
      advanced.ctor();
   }
   void create()
   {
      ListColumn mode_list_column[]=
      {
         ListColumn(DATA_VECI2, 0, SIZE(VecI2), LCW_MAX_DATA_PARENT, "Size"),
      };
      ListColumn theme_list_column[]=
      {
         ListColumn(MEMBER(Skin, name), LCW_MAX_DATA_PARENT, "Name"),
      };

   #if DESKTOP
      mode   =&props.New().create("Resolution"       , MemberDesc(         ).setTextToDataFunc(Mode        )).setEnum(); mode.combobox.setColumns(mode_list_column, Elms(mode_list_column)).setData(ConstCast(D.modes()));
      full   =&props.New().create("Fullscreen"       , MemberDesc(DATA_BOOL).setFunc(Full      , Full      ))                                          .desc("Enable full screen mode");
               props.New().create("Synchronization"  , MemberDesc(DATA_BOOL).setFunc(Sync      , Sync      ))                                          .desc("Enable screen synchronization\nLimits framerate to screen refresh rate to increase smoothness.");
   #endif
               props.New().create("Renderer"         , MemberDesc(         ).setFunc(Render    , Render    )).setEnum(Render_t    , Elms(Render_t    )).desc("Renderer type\nSimple and Forward renderers may work faster, but have limited number of special effects.");
               props.New().create("Edge Softening"   , MemberDesc(         ).setFunc(EdgeSoft  , EdgeSoft  )).setEnum(EdgeSoften_t, Elms(EdgeSoften_t)).desc("Set edge softening");
      shd    =&props.New().create("Shadows"          , MemberDesc(DATA_BOOL).setFunc(Shadow    , Shadow    ))                                          .desc("Enable shadows");
      shd_siz=&props.New().create("Shadowmap Size"   , MemberDesc(         ).setFunc(ShadowSize, ShadowSize)).setEnum(ShadowSize_t, Elms(ShadowSize_t)).desc("Shadow map resolution\nhigher resolutions reduce blockiness of shadows.");
      shd_num=&props.New().create("Shadowmap Number" , MemberDesc(         ).setFunc(ShadowNum , ShadowNum )).setEnum(ShadowNum_t , Elms(ShadowNum_t )).desc("Shadow map number,\ndetermines the number of shadow maps used during rendering.");
      shd_sft=&props.New().create("Shadows Softing"  , MemberDesc(         ).setFunc(ShadowSoft, ShadowSoft)).setEnum(ShadowSoft_t, Elms(ShadowSoft_t)).desc("Enable shadows softing");
      shd_jit=&props.New().create("Shadows Jittering", MemberDesc(DATA_BOOL).setFunc(ShadowJit , ShadowJit ))                                          .desc("Enable jittering on shadows,\nworks best when enabled with shadow softing.");
      bump   =&props.New().create("Bump Mapping"     , MemberDesc(         ).setFunc(BumpMode  , BumpMode  )).setEnum(BumpMode_t  , Elms(BumpMode_t  )).desc("Simulate bumpy surfaces");
               props.New().create("Motion Blur"      , MemberDesc(         ).setFunc(MotionMode, MotionMode)).setEnum(MotionMode_t, Elms(MotionMode_t)).desc("Blur fast moving objects");
      ao     =&props.New().create("Ambient Occlusion", MemberDesc(         ).setFunc(AO        , AO        )).setEnum(AO_t        , Elms(AO_t        )).desc("Darkens occluded areas");
               props.New().create("Eye Adaptation"   , MemberDesc(DATA_BOOL).setFunc(EyeAdapt  , EyeAdapt  ))                                          .desc("Enables automatic screen brightness adjustment");
//if(D.shaderModel()>=SM_5)props.New().create("Tesselation", MemberDesc(DATA_BOOL).setFunc(Tesselation, Tesselation))                                  ;
               props.New().create("Gui Scale"        , MemberDesc(DATA_REAL).setFunc(Scale     , Scale     )).mouseEditSpeed(0.5)
               #if MOBILE
                  .range(0.9, 3.0);
               #else
                  .range(0.6, 1.5);
               #endif
            #if DESKTOP
               props.New().create("Gui Scale to Window", MemberDesc(DATA_BOOL).setFunc(ScaleWin  , ScaleWin  )).desc("Gui Scale will be automatically adjusted by application window size");
            #endif
      skin   =&props.New().create("Gui Skin"           , MemberDesc()).setEnum(); skin.combobox.func(SkinChanged, T).menu.setColumns(theme_list_column, Elms(theme_list_column), true).setData(skins, Elms(skins)); skin.combobox.text_size*=0.95;

      Rect r=super.create("Video Options", Vec2(0.02, -0.02), 0.041, 0.050, 0.27); autoData(this); button[2].show();
      skin.set(skinIndex(Gui.skin.id()), QUIET); // call after 'autoData'
      T+=advanced_show.create(Rect_U(clientWidth()/2, r.min.y-0.015, 0.3, 0.055), "Advanced").func(ShowAdvanced, T); advanced_show.mode=BUTTON_TOGGLE;
      super.resize(Vec2(0, 0.07));
      setVis();
      pos(Vec2(D.w()-rect().w(), D.h()));

      advanced.create();
   }
   void setVis()
   {
      if(shd    )shd    .visible(Renderer.type()!=RT_SIMPLE                                 );
      if(shd_siz)shd_siz.visible(Renderer.type()!=RT_SIMPLE   && D.shadowMode()==SHADOW_MAP );
      if(shd_num)shd_num.visible(Renderer.type()!=RT_SIMPLE   && D.shadowMode()==SHADOW_MAP );
      if(shd_sft)shd_sft.visible(Renderer.type()==RT_DEFERRED && D.shadowMode()!=SHADOW_NONE);
      if(shd_jit)shd_jit.visible(Renderer.type()!=RT_SIMPLE   && D.shadowMode()==SHADOW_MAP );
      if(bump   )bump   .visible(Renderer.type()!=RT_SIMPLE                                 );
      if(ao     )ao     .visible(Renderer.type()!=RT_SIMPLE                                 );
   }
   void hideAll()
   {
      hide();
      advanced.hide();
   }
   virtual Window& show()override
   {
      Misc .vid_opt.set(true, QUIET);
      Projs.vid_opt.set(true, QUIET);
      return super.show();
   }
   virtual VideoOptions& hide()override
   {
      Misc .vid_opt.set(false, QUIET);
      Projs.vid_opt.set(false, QUIET);
      super.hide();
      return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible())setTitle(S+"Video Options - "+TextReal(Time.fps(), 1)+" Fps");
   }
}
VideoOptions VidOpt;
/******************************************************************************/
