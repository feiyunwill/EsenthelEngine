/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************/
#define BUMP_MAPPING   1
#define MULTI_MATERIAL 1
#define FORCE_LOG      0

#if WINDOWS // DirectX 9
   #define COMPILE_3  0
#endif
#if DX11   // DirectX 10+
   #define COMPILE_4  0
#endif
#if GL && !GL_ES // Desktop OpenGL
   #define COMPILE_GL 0
#endif

/**
#define MAIN
#define SIMPLE
#define DEFERRED
#define FORWARD // Forward Shaders in OpenGL compile almost an entire day and use ~5 GB memory during compilation
#define BLEND_LIGHT
#define AMBIENT
#define POSITION
#define BLEND
#define SET_COLOR
#define BEHIND
#define OVERLAY
#define EARLY_Z
#define FUR
#define AMBIENT_OCCLUSION
#define VOLUMETRIC_LIGHTS
#define VOLUMETRIC_CLOUDS
#define LAYERED_CLOUDS
#define HDR
#define MOTION_BLUR
#define DEPTH_OF_FIELD
#define WATER
#define WORLD_EDITOR
/******************************************************************************
#define DX10_INPUT_LAYOUT
/******************************************************************************/
// SHADER TECHNIQUE NAMES
/******************************************************************************/
Str8 TechNameSimple    (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test,            Int light_map, Int reflect, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel, Int tess) {return S8+"T"+skin+materials+textures+bump_mode+alpha_test+light_map+reflect+color+mtrl_blend+heightmap+fx+per_pixel+tess;}
Str8 TechNameDeferred  (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test,            Int light_map, Int detail, Int macro, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tess) {return S8+"T"+skin+materials+textures+bump_mode+alpha_test+light_map+detail+macro+rflct+color+mtrl_blend+heightmap+fx+tess;}
Str8 TechNameForward   (Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test,            Int light_map, Int detail, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx,   Int light_dir, Int light_dir_shd, Int light_dir_shd_num,   Int light_point, Int light_point_shd,   Int light_sqr, Int light_sqr_shd,   Int light_cone, Int light_cone_shd,   Int tess) {return S8+"T"+skin+materials+textures+bump_mode+alpha_test+light_map+detail+rflct+color+mtrl_blend+heightmap+fx+light_dir+light_dir_shd+light_dir_shd_num+light_point+light_point_shd+light_sqr+light_sqr_shd+light_cone+light_cone_shd+tess;}
Str8 TechNameBlendLight(Int skin, Int color    , Int textures, Int bump_mode, Int alpha_test, Int alpha, Int light_map, Int rflct, Int fx, Int per_pixel, Int shadow_maps) {return S8+"T"+skin+color+textures+bump_mode+alpha_test+alpha+light_map+rflct+fx+per_pixel+shadow_maps;}
Str8 TechNamePosition  (Int skin, Int textures, Int test_blend, Int fx, Int tess) {return S8+"T"+skin+textures+test_blend+fx+tess;}
Str8 TechNameBlend     (Int skin, Int color, Int rflct, Int textures, Int light_map) {return S8+"T"+skin+color+rflct+textures+light_map;}
Str8 TechNameSetColor  (Int skin, Int textures, Int tess) {return S8+"T"+skin+textures+tess;}
Str8 TechNameBehind    (Int skin, Int textures) {return S8+"T"+skin+textures;}
Str8 TechNameEarlyZ    (Int skin) {return S8+"T"+skin;}
Str8 TechNameAmbient   (Int skin, Int alpha_test) {return S8+"T"+skin+alpha_test;}
Str8 TechNameOverlay   (Int skin, Int normal) {return S8+"T"+skin+normal;}
Str8 TechNameFurBase   (Int skin, Int size, Int diffuse) {return S8+"Base"+skin+size+diffuse;}
Str8 TechNameFurSoft   (Int skin, Int size, Int diffuse) {return S8+"Soft"+skin+size+diffuse;}
Str8 TechNameTattoo    (Int skin, Int tess             ) {return S8+"T"+skin+tess;}
/******************************************************************************/
#if COMPILE_3 || COMPILE_4 || COMPILE_GL
/******************************************************************************/
// SHADER TECHNIQUE DECLARATIONS
/******************************************************************************/
static Str TechSimple(Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int reflect, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel, Int tess)
{
   Str params=             S+skin+','+materials+','+textures+','+bump_mode+','+alpha_test+','+light_map+','+reflect+','+color+','+mtrl_blend+','+heightmap+','+fx+','+per_pixel+','+tess,
       name  =TechNameSimple(skin  ,  materials  ,  textures  ,  bump_mode  ,  alpha_test  ,  light_map  ,  reflect  ,  color  ,  mtrl_blend  ,  heightmap  ,  fx  ,  per_pixel  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}
static ShaderGLSL TechSimpleGlsl(Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int reflect, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel, Int tess)
{
   return ShaderGLSL().set("Main", TechNameSimple(skin, materials, textures, bump_mode, alpha_test, light_map, reflect, color, mtrl_blend, heightmap, fx, per_pixel, tess))
      .par("skin"      , skin      )
      .par("materials" , materials )
      .par("textures"  , textures  )
      .par("bump_mode" , bump_mode )
      .par("alpha_test", alpha_test)
      .par("light_map" , light_map )
      .par("rflct"     , reflect   )
      .par("COLOR"     , color     )
      .par("mtrl_blend", mtrl_blend)
      .par("heightmap" , heightmap )
      .par("fx"        , fx        )
      .par("per_pixel" , per_pixel )
      .par("tesselate" , tess      );
}

static Str TechDeferred(Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int detail, Int macro, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tess)
{
   Str params=               S+skin+','+materials+','+textures+','+bump_mode+','+alpha_test+','+light_map+','+detail+','+macro+','+rflct+','+color+','+mtrl_blend+','+heightmap+','+fx+','+tess,
       name  =TechNameDeferred(skin  ,  materials  ,  textures  ,  bump_mode  ,  alpha_test  ,  light_map  ,  detail  ,  macro  ,  rflct  ,  color  ,  mtrl_blend  ,  heightmap  ,  fx  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}

static Str TechForward(Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int detail, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx,   Int light_dir, Int light_dir_shd, Int light_dir_shd_num,   Int light_point, Int light_point_shd,   Int light_sqr, Int light_sqr_shd,   Int light_cone, Int light_cone_shd,   Int tess)
{
   Str params=              S+skin+','+materials+','+textures+','+bump_mode+','+alpha_test+','+light_map+','+detail+','+rflct+','+color+','+mtrl_blend+','+heightmap+','+fx+','   +light_dir+','+light_dir_shd+','+light_dir_shd_num+   ','+light_point+','+light_point_shd+   ','+light_sqr+','+light_sqr_shd+   ','+light_cone+','+light_cone_shd+','+tess,
       name  =TechNameForward(skin  ,  materials  ,  textures  ,  bump_mode  ,  alpha_test  ,  light_map  ,  detail  ,  rflct  ,  color  ,  mtrl_blend  ,  heightmap  ,  fx  ,     light_dir  ,  light_dir_shd  ,  light_dir_shd_num     ,  light_point  ,  light_point_shd     ,  light_sqr  ,  light_sqr_shd     ,  light_cone  ,  light_cone_shd  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}
static Str TechForwardLight(Int skin, Int materials, Int textures, Int bump_mode, Int alpha_test, Int light_map, Int detail, Int rflct, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tess,   SHADER_MODEL model)
{
   Str names;
   REPD(shd, 2)
   {
      if(shd)for(Int maps=2; maps<=6; maps+=2) // 2, 4, 6 maps
      names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   true ,shd  ,maps,  false,false,  false,false,  false,false,  tess);else // light dir with shadow maps
      names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   true ,false,   0,  false,false,  false,false,  false,false,  tess);     // light dir no   shadow
      names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   false,false,   0,  true ,shd  ,  false,false,  false,false,  tess);     // light point
      names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   false,false,   0,  false,false,  true ,shd  ,  false,false,  tess);     // light sqr
      names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   false,false,   0,  false,false,  false,false,  true ,shd  ,  tess);     // light cone
   }  names+=TechForward(skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx,   false,false,   0,  false,false,  false,false,  false,false,  tess);     // no light
   return names;
}

static Str TechPosition(Int skin, Int textures, Int test_blend, Int fx, Int tess)
{
   Str params=               S+skin+','+textures+','+test_blend+','+fx+','+tess,
       name  =TechNamePosition(skin  ,  textures  ,  test_blend  ,  fx  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}

static Str TechBlend(Int skin, Int color, Int rflct, Int textures, Int light_map)
{
   Str params=            S+skin+','+color+','+rflct+','+textures+','+light_map,
       name  =TechNameBlend(skin  ,  color  ,  rflct  ,  textures,    light_map);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}
static ShaderGLSL TechBlendGlsl(Int skin, Int color, Int rflct, Int textures, Int light_map)
{
   return ShaderGLSL().set("Main", TechNameBlend(skin, color, rflct, textures, light_map))
      .par("skin"     , skin     )
      .par("COLOR"    , color    )
      .par("rflct"    , rflct    )
      .par("textures" , textures )
      .par("light_map", light_map);
}

static Str TechBlendLight(Int skin, Int color, Int textures, Int bump_mode, Int alpha_test, Int alpha, Int light_map, Int rflct, Int fx, Int per_pixel, Int shadow_maps)
{
   Str params=                 S+skin+','+color+','+textures+','+bump_mode+','+alpha_test+','+alpha+','+light_map+','+rflct+','+fx+','+per_pixel+','+shadow_maps,
       name  =TechNameBlendLight(skin  ,  color  ,  textures  ,  bump_mode  ,  alpha_test  ,  alpha  ,  light_map  ,  rflct  ,  fx  ,  per_pixel  ,  shadow_maps);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}
static ShaderGLSL TechBlendLightGlsl(Int skin, Int color, Int textures, Int bump_mode, Int alpha_test, Int alpha, Int light_map, Int rflct, Int fx, Int per_pixel, Int shadow_maps)
{
   return ShaderGLSL().set("Main", TechNameBlendLight(skin, color, textures, bump_mode, alpha_test, alpha, light_map, rflct, fx, per_pixel, shadow_maps))
      .par("skin"       , skin       )
      .par("COLOR"      , color      )
      .par("textures"   , textures   )
      .par("bump_mode"  , bump_mode  )
      .par("alpha_test" , alpha_test )
      .par("ALPHA"      , alpha      )
      .par("light_map"  , light_map  )
      .par("rflct"      , rflct      )
      .par("fx"         , fx         )
      .par("per_pixel"  , per_pixel  )
      .par("shadow_maps", shadow_maps);
}

static Str TechSetColor(Int skin, Int textures, Int tess)
{
   Str params=               S+skin+','+textures+','+tess,
       name  =TechNameSetColor(skin  ,  textures  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}

static Str TechBehind(Int skin, Int textures)
{
   Str params=             S+skin+','+textures,
       name  =TechNameBehind(skin  ,  textures);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}

static Str TechEarlyZ(Int skin)
{
   Str params=             S+skin,
       name  =TechNameEarlyZ(skin);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}

static Str TechAmbient(Int skin, Int alpha_test)
{
   Str params=              S+skin+','+alpha_test,
       name  =TechNameAmbient(skin  ,  alpha_test);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}

static Str TechOverlay(Int skin, Int normal)
{
   Str params=              S+skin+','+normal,
       name  =TechNameOverlay(skin  ,  normal);
   return S+"TECHNIQUE("+name+", VS("+params+"), PS("+params+"));";
}

static Str TechTattoo(Int skin, Int tess)
{
   Str params=             S+skin+','+tess,
       name  =TechNameTattoo(skin  ,  tess);
   return tess ? S+"TECHNIQUE_TESSELATION("+name+", VS("+params+"), PS("+params+"), HS("+params+"), DS("+params+"));"
               : S+"TECHNIQUE            ("+name+", VS("+params+"), PS("+params+")                                );";
}

static Str TechFurBase(Int skin, Int size, Int diffuse)
{
   Str params=              S+skin+','+size+','+diffuse,
       name  =TechNameFurBase(skin  ,  size  ,  diffuse);
   return S+"TECHNIQUE("+name+", Base_VS("+params+"), Base_PS("+params+"));";
}

static Str TechFurSoft(Int skin, Int size, Int diffuse)
{
   Str params=              S+skin+','+size+','+diffuse,
       name  =TechNameFurSoft(skin  ,  size  ,  diffuse);
   return S+"TECHNIQUE("+name+", Soft_VS("+params+"), Soft_PS("+params+"));";
}
/******************************************************************************/
// COMPILER
/******************************************************************************/
struct ShaderCompiler
{
   Str               src, dest;
   SHADER_MODEL      model;
   Memc<ShaderMacro> macros;
   Memc<ShaderGLSL > glsl;

   void compile()
   {
      Str messages; Bool ok=ShaderCompileTry(src, dest, model, macros, glsl, &messages);
      if(!ok || DEBUG || FORCE_LOG)if(messages.is())LogN(S+"Shader\n\""+src+"\"\nto file\n\""+dest+"\"\n"+messages);
      if(!ok)
      {
      #if !WINDOWS
         if(model==SM_3)Exit("Can't compile DX9 Shaders when not using Windows engine version");
      #endif
      #if !DX11
         if(model>=SM_4)Exit("Can't compile DX10+ Shaders when not using DX10+ engine version");
      #endif
      #if DX9 || DX11
         if(model>=SM_GL_ES_2 && model<=SM_GL)Exit("Can't compile OpenGL Shaders when not using OpenGL engine version");
      #endif
         Exit(S+"Error compiling shader\n\""+src+"\"\nto file\n\""+dest+"\"."+(messages.is() ? S+"\n\nCompilation Messages:\n"+messages : S));
      }
   }
};
/******************************************************************************/
static Memc<ShaderCompiler> ShaderCompilers;
/******************************************************************************/
static void Add(C Str &src, C Str &dest, SHADER_MODEL model, C MemPtr<ShaderGLSL> &glsl=null)
{
   ShaderCompiler &sc=ShaderCompilers.New();
   sc.src  =src  ;
   sc.dest =dest ;
   sc.model=model;
   sc.glsl =glsl ;
}
static void Add(C Str &src, C Str &dest, SHADER_MODEL model, C Str &names, C MemPtr<ShaderGLSL> &glsl=null)
{
   ShaderCompiler &sc=ShaderCompilers.New();
   sc.src  =src  ;
   sc.dest =dest ;
   sc.model=model;
   sc.glsl =glsl ;
   sc.macros.New().set("CUSTOM_TECHNIQUE", names);
}
/******************************************************************************/
// LISTING ALL SHADER TECHNIQUES
/******************************************************************************/
static void Compile(SHADER_MODEL model)
{
   if(!DataPath().is())Exit("Can't compile default shaders - 'DataPath' not specified");

   Str src,
       src_path=GetPath(GetPath(__FILE__))+"\\Shaders\\", // for example "C:/Esenthel/Engine/Src/Shaders/"
      dest_path=DataPath();                               // for example "C:/Esenthel/Data/Shader/4/"
   switch(model)
   {
      case SM_UNKNOWN: return;

      case SM_GL_ES_2:
      case SM_GL_ES_3:
      case SM_GL     : dest_path+="Shader\\GL\\"; break;

      case SM_3      : dest_path+="Shader\\3\\" ; break;
      default        : dest_path+="Shader\\4\\" ; break;
   }
   FCreateDirs(dest_path);

   // list first those that take the most time to compile

#ifdef FORWARD
{
#if GL && !X64 && COMPILE_GL
   #error "Can't compile GL forward shaders on 32-bit because you will run out of memory"
#endif
   Str names;

   // zero
   REPD(skin , 2)
   REPD(color, 2)names+=TechForward(skin, 1, 0, SBUMP_ZERO, false, false, false, false, color, false, false, FX_NONE,   false,false,0,   false,false,   false,false,   false,false,   false);

   REPD(tess     , (model>=SM_4) ? 2 : 1)
   REPD(heightmap, 2)
   {
      // 1 material, 0-2 textures, flat
      REPD(skin  , heightmap ? 1 : 2)
      REPD(detail, 2)
      REPD(rflct , 2)
      REPD(color , 2)
      {
         names+=TechForwardLight(skin, 1, 0, SBUMP_FLAT, false, false, detail, rflct, color, false, heightmap, FX_NONE, tess, model); // 1 material, 0 tex
         REPD(alpha_test, heightmap ? 1 : 2)
         REPD(light_map , 2)
         {
            names+=TechForwardLight(skin, 1, 1, SBUMP_FLAT, alpha_test, light_map, detail, rflct, color, false, heightmap, FX_NONE, tess, model); // 1 material, 1 tex
            names+=TechForwardLight(skin, 1, 2, SBUMP_FLAT, alpha_test, light_map, detail, rflct, color, false, heightmap, FX_NONE, tess, model); // 1 material, 2 tex
         }
      }

   #if BUMP_MAPPING
      // 1 material, 2 tex, normal
      REPD(skin      , heightmap ? 1 : 2)
      REPD(alpha_test, heightmap ? 1 : 2)
      REPD(light_map , 2)
      REPD(detail    , 2)
      REPD(rflct     , 2)
      REPD(color     , 2)names+=TechForwardLight(skin, 1, 2, SBUMP_NORMAL, alpha_test, light_map, detail, rflct, color, false, heightmap, FX_NONE, tess, model);
   #endif

   #if MULTI_MATERIAL
      for(Int materials=2; materials<=MAX_MTRLS; materials++)
      REPD(color     , 2)
      REPD(mtrl_blend, 2)
      REPD(rflct     , 2)
      {
         // 2-4 materials, 1-2 textures, flat
         for(Int textures=1; textures<=2; textures ++)names+=TechForwardLight(false, materials, textures, SBUMP_FLAT, false, false, false, rflct, color, mtrl_blend, heightmap, FX_NONE, tess, model);

      #if BUMP_MAPPING
         // 2-4 materials, 2 textures, normal
         names+=TechForwardLight(false, materials, 2, SBUMP_NORMAL, false, false, false, rflct, color, mtrl_blend, heightmap, FX_NONE, tess, model);
      #endif
      }
   #endif
   }

   // grass + leaf
   for(Int textures=1; textures<=2; textures++)
   REPD(color, 2)
   {
      names+=TechForwardLight(false, 1, textures, SBUMP_FLAT, true, false, false, false, color, false, false, FX_GRASS, false, model); // 1 material, 1-2 tex, grass, flat
      names+=TechForwardLight(false, 1, textures, SBUMP_FLAT, true, false, false, false, color, false, false, FX_LEAF , false, model); // 1 material, 1-2 tex, leaf , flat
      names+=TechForwardLight(false, 1, textures, SBUMP_FLAT, true, false, false, false, color, false, false, FX_LEAFS, false, model); // 1 material, 1-2 tex, leafs, flat
      if(textures==2)
      {
         names+=TechForwardLight(false, 1, textures, SBUMP_NORMAL, true, false, false, false, color, false, false, FX_GRASS, false, model); // 1 material, 1-2 tex, grass, normal
         names+=TechForwardLight(false, 1, textures, SBUMP_NORMAL, true, false, false, false, color, false, false, FX_LEAF , false, model); // 1 material, 1-2 tex, leaf , normal
         names+=TechForwardLight(false, 1, textures, SBUMP_NORMAL, true, false, false, false, color, false, false, FX_LEAFS, false, model); // 1 material, 1-2 tex, leafs, normal
      }
   }

   Add(src_path+"Forward.cpp", dest_path+"Forward", model, names);
}
#endif

#ifdef BLEND_LIGHT
{
#if GL && !X64 && COMPILE_GL
   #error "Can't compile GL Blend Light shaders on 32-bit because you will run out of memory"
#endif
   Str names; Memc<ShaderGLSL> glsl;

   REPD(per_pixel  , 2)
   REPD(shadow_maps, per_pixel ? 7 : 1) // 7=(6+off), 1=off
   {
      // base
      REPD(skin      , 2)
      REPD(color     , 2)
      REPD(textures  , 3)
      REPD(bump_mode , (per_pixel && textures==2) ? 2 : 1)
      REPD(alpha_test,               textures     ? 2 : 1)
      REPD(alpha     ,               textures     ? 2 : 1)
      REPD(light_map ,               textures     ? 2 : 1)
      REPD(rflct     , 2)
      {
                                            names+=(TechBlendLight    (skin, color, textures, bump_mode ? SBUMP_NORMAL: SBUMP_FLAT, alpha_test, alpha, light_map, rflct, FX_NONE, per_pixel, shadow_maps));
         if(shadow_maps==0 && bump_mode==0)glsl.add(TechBlendLightGlsl(skin, color, textures, bump_mode ? SBUMP_NORMAL: SBUMP_FLAT, alpha_test, alpha, light_map, rflct, FX_NONE, per_pixel, shadow_maps));
      }

      // grass+leaf
      REPD(color     , 2)
      REPD(textures  , 3)
      REPD(bump_mode , (per_pixel && textures==2) ? 2 : 1)
      REPD(alpha_test,               textures     ? 2 : 1)
      {
                                            names+=(TechBlendLight    (false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_GRASS, per_pixel, shadow_maps));
                                            names+=(TechBlendLight    (false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_LEAF , per_pixel, shadow_maps));
                                            names+=(TechBlendLight    (false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_LEAFS, per_pixel, shadow_maps));
         if(shadow_maps==0 && bump_mode==0)glsl.add(TechBlendLightGlsl(false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_GRASS, per_pixel, shadow_maps));
         if(shadow_maps==0 && bump_mode==0)glsl.add(TechBlendLightGlsl(false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_LEAF , per_pixel, shadow_maps));
         if(shadow_maps==0 && bump_mode==0)glsl.add(TechBlendLightGlsl(false, color, textures, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, false, false, FX_LEAFS, per_pixel, shadow_maps));
      }
   }

   Add(src_path+"Blend Light.cpp", dest_path+"Blend Light", model, names, glsl);
}
#endif

#ifdef DEFERRED
{
   Str names;

   // zero
   REPD(skin , 2)
   REPD(color, 2)names+=TechDeferred(skin, 1, 0, SBUMP_ZERO, false, false, false, false, false, color, false, false, FX_NONE, false);

   REPD(tess     , (model>=SM_4) ? 2 : 1)
   REPD(heightmap, 2)
   {
      // 1 material, 0-2 tex, flat
      REPD(skin  , heightmap ? 1 : 2)
      REPD(detail, 2)
      REPD(rflct , 2)
      REPD(color , 2)
      {
         names+=TechDeferred(skin, 1, 0, SBUMP_FLAT, false, false, detail, false, rflct, color, false, heightmap, FX_NONE, tess); // 1 material, 0 tex
         REPD(alpha_test, heightmap ? 1 : 2)
         REPD(light_map , 2)
         {
            names+=TechDeferred(skin, 1, 1, SBUMP_FLAT, alpha_test, light_map, detail, false, rflct, color, false, heightmap, FX_NONE, tess); // 1 material, 1 tex
            names+=TechDeferred(skin, 1, 2, SBUMP_FLAT, alpha_test, light_map, detail, false, rflct, color, false, heightmap, FX_NONE, tess); // 1 material, 2 tex
         }
      }

      // 1 material, 1-2 tex, flat, macro
      REPD(light_map, 2)
      REPD(color    , 2)
      for(Int textures=1; textures<=2; textures++)names+=TechDeferred(false, 1, textures, SBUMP_FLAT, false, light_map, false, true, false, color, false, heightmap, FX_NONE, tess);

   #if BUMP_MAPPING
      // 1 material, 2 tex, normal + parallax
      REPD(skin      , heightmap ? 1 : 2)
      REPD(alpha_test, heightmap ? 1 : 2)
      REPD(light_map , 2)
      REPD(detail    , 2)
      REPD(rflct     , 2)
      REPD(color     , 2)
      for(Int bump_mode=SBUMP_NORMAL; bump_mode<=SBUMP_PARALLAX_MAX; bump_mode++)if(bump_mode==SBUMP_NORMAL || bump_mode>=SBUMP_PARALLAX_MIN)
         names+=TechDeferred(skin, 1, 2, bump_mode, alpha_test, light_map, detail, false, rflct, color, false, heightmap, FX_NONE, tess);

      // 1 material, 1-2 tex, normal, macro
      REPD(light_map, 2)
      REPD(color    , 2)
      for(Int textures=1; textures<=2; textures++)names+=TechDeferred(false, 1, textures, SBUMP_NORMAL, false, light_map, false, true, false, color, false, heightmap, FX_NONE, tess);

      // 1 material, 2 tex, relief
      REPD(skin      , heightmap ? 1 : 2)
      REPD(alpha_test, heightmap ? 1 : 2)
      REPD(light_map , 2)
      REPD(detail    , 2)
      REPD(rflct     , 2)
      REPD(color     , 2)names+=TechDeferred(skin, 1, 2, SBUMP_RELIEF, alpha_test, light_map, detail, false, rflct, color, false, heightmap, FX_NONE, tess);
   #endif

   #if MULTI_MATERIAL
      for(Int materials=2; materials<=MAX_MTRLS; materials++)
      REPD(color     , 2)
      REPD(mtrl_blend, 2)
      REPD(rflct     , 2)
      {
         // 2-4 materials, 1-2 tex, flat
         REPD(detail, 2)
         for(Int textures=1; textures<=2; textures++)names+=TechDeferred(false, materials, textures, SBUMP_FLAT, false, false, detail, false, rflct, color, mtrl_blend, heightmap, FX_NONE, tess);

         // 2-4 materials, 1-2 tex, flat, macro
         for(Int textures=1; textures<=2; textures++)names+=TechDeferred(false, materials, textures, SBUMP_FLAT, false, false, false, true, rflct, color, mtrl_blend, heightmap, FX_NONE, tess);

      #if BUMP_MAPPING
         // 2-4 materials, 2 textures, normal + parallax
         REPD(detail, 2)
         for(Int bump_mode=SBUMP_NORMAL; bump_mode<=SBUMP_PARALLAX_MAX_MULTI; bump_mode++)if(bump_mode==SBUMP_NORMAL || bump_mode>=SBUMP_PARALLAX_MIN)
            names+=TechDeferred(false, materials, 2, bump_mode, false, false, detail, false, rflct, color, mtrl_blend, heightmap, FX_NONE, tess);

         // 2-4 materials, 2 textures, normal, macro
         names+=TechDeferred(false, materials, 2, SBUMP_NORMAL, false, false, false, true, rflct, color, mtrl_blend, heightmap, FX_NONE, tess);

         // 2-4 materials, 2 textures, relief
         REPD(detail, 2)
            names+=TechDeferred(false, materials, 2, SBUMP_RELIEF, false, false, detail, false, rflct, color, mtrl_blend, heightmap, FX_NONE, tess);
      #endif
      }
   #endif
   }

   // grass + leaf
   for(Int textures=1; textures<=2; textures++)
   REPD(color, 2)
   {
      names+=TechDeferred(false, 1, textures, SBUMP_FLAT, true, false, false, false, false, color, false, false, FX_GRASS, false); // 1 material, 1-2 tex, grass, flat
      names+=TechDeferred(false, 1, textures, SBUMP_FLAT, true, false, false, false, false, color, false, false, FX_LEAF , false); // 1 material, 1-2 tex, leaf , flat
      names+=TechDeferred(false, 1, textures, SBUMP_FLAT, true, false, false, false, false, color, false, false, FX_LEAFS, false); // 1 material, 1-2 tex, leafs, flat
      if(textures==2)
      {
         names+=TechDeferred(false, 1, textures, SBUMP_NORMAL, true, false, false, false, false, color, false, false, FX_GRASS, false); // 1 material, 1-2 tex, grass, normal
         names+=TechDeferred(false, 1, textures, SBUMP_NORMAL, true, false, false, false, false, color, false, false, FX_LEAF , false); // 1 material, 1-2 tex, leaf , normal
         names+=TechDeferred(false, 1, textures, SBUMP_NORMAL, true, false, false, false, false, color, false, false, FX_LEAFS, false); // 1 material, 1-2 tex, leafs, normal
      }
   }

   Add(src_path+"Deferred.cpp", dest_path+"Deferred", model, names);
}
#endif

#ifdef SIMPLE
{
   Str names; Memc<ShaderGLSL> glsl;

   REPD(per_pixel, 2)
   {
      // zero
      REPD(skin , 2)
      REPD(color, 2)
      {
          names+=(TechSimple    (skin, 1, 0, SBUMP_ZERO, false, false, false, color, false, false, FX_NONE, per_pixel, false));
         glsl.add(TechSimpleGlsl(skin, 1, 0, SBUMP_ZERO, false, false, false, color, false, false, FX_NONE, per_pixel, false));
      }

      REPD(tess     , (model>=SM_4) ? 2 : 1)
      REPD(heightmap, 2)
      {
         // 1 material, 0-2 textures
         REPD(skin    , heightmap ? 1 : 2)
         REPD(rflct   , 2)
         REPD(color   , 2)
       //REPD(instance, (model>=SM_4 && !skin) ? 2 : 1)
         {
             names+=(TechSimple    (skin, 1, 0, SBUMP_FLAT, false, false, rflct, color, false, heightmap, FX_NONE, per_pixel, tess)); // 1 material, 0 tex
            glsl.add(TechSimpleGlsl(skin, 1, 0, SBUMP_FLAT, false, false, rflct, color, false, heightmap, FX_NONE, per_pixel, tess));
            REPD(alpha_test, heightmap ? 1 : 2)
            REPD(light_map , 2)
            {
                names+=(TechSimple    (skin, 1, 1, SBUMP_FLAT, alpha_test, light_map, rflct, color, false, heightmap, FX_NONE, per_pixel, tess)); // 1 material, 1 tex
                names+=(TechSimple    (skin, 1, 2, SBUMP_FLAT, alpha_test, light_map, rflct, color, false, heightmap, FX_NONE, per_pixel, tess)); // 1 material, 2 tex
               glsl.add(TechSimpleGlsl(skin, 1, 1, SBUMP_FLAT, alpha_test, light_map, rflct, color, false, heightmap, FX_NONE, per_pixel, tess));
               glsl.add(TechSimpleGlsl(skin, 1, 2, SBUMP_FLAT, alpha_test, light_map, rflct, color, false, heightmap, FX_NONE, per_pixel, tess));
            }
         }

      #if MULTI_MATERIAL
         // 2-4 materials, 1 textures
         for(Int materials=2; materials<=MAX_MTRLS; materials++)
         REPD(color     , 2)
         REPD(mtrl_blend, 2)
         REPD(rflct     , 2)
         {
                       names+=(TechSimple    (false, materials, 1, SBUMP_FLAT, false, false, rflct, color, mtrl_blend, heightmap, FX_NONE, per_pixel, tess));
            if(!rflct)glsl.add(TechSimpleGlsl(false, materials, 1, SBUMP_FLAT, false, false, rflct, color, mtrl_blend, heightmap, FX_NONE, per_pixel, tess));
         }
      #endif
      }

      // grass + leaf
    //REPD(instance, (model>=SM_4) ? 2 : 1)
      REPD(color, 2)
      for(Int textures=1; textures<=2; textures++)
      {
          names+=(TechSimple    (false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_GRASS, per_pixel, false));
          names+=(TechSimple    (false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_LEAF , per_pixel, false));
          names+=(TechSimple    (false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_LEAFS, per_pixel, false));
         glsl.add(TechSimpleGlsl(false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_GRASS, per_pixel, false));
         glsl.add(TechSimpleGlsl(false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_LEAF , per_pixel, false));
         glsl.add(TechSimpleGlsl(false, 1, textures, SBUMP_FLAT, true, false, false, color, false, false, FX_LEAFS, per_pixel, false));
      }
   }

   // bone highlight
   REPD(skin, 2)names+=TechSimple(skin, 1, 0, SBUMP_FLAT, false, false, false, false, false, false, FX_BONE, true, false); // !! this name must be in sync with other calls in the engine that use FX_BONE !!

   Add(src_path+"Simple.cpp", dest_path+"Simple", model, names, glsl);
}
#endif

#ifdef MAIN
   Memc<ShaderGLSL> glsl;

   // Draw2DTex
   REPD(c, 2) // color
      glsl.New().set("Draw2DTex", S+"Draw2DTex"+(c?'C':'\0')).par("COLOR", c);

   // Draw3DTex
   REPD(c , 2) // color
   REPD(at, 2) // alpha_test
      glsl.New().set("Draw3DTex", S+"Draw3DTex"+(c?"Col":"")+(at?"AT":"")).par("COLOR", c).par("alpha_test", at);

   // font
   glsl.New().set("Font", "Font");

   // blur
   REPD(x, 2) // axis
   REPD(h, 2) // high
      glsl.New().set("Blur", S+"Blur"+(x?'X':'Y')+(h?'H':'\0')).par("axis", x ? '0' : '1').par("high", h);

   // bloom downsample
   REPD(g, 2) // glow
   REPD(c, 2) // clamp
   REPD(h, 2) // half
   REPD(s, 2) // saturate
      glsl.New().set("BloomDS", S+"Bloom"+(g?'G':'\0')+"DS"+(c?'C':'\0')+(h?'H':'\0')+(s?'S':'\0')).par("DoGlow", g).par("DoClamp", c).par("half", h).par("saturate", s);

   // bloom
   glsl.New().set("Bloom", "Bloom");

   // shd blur
   glsl.New().set("ShdBlurX", "ShdBlurX2").par("range", 2);
   glsl.New().set("ShdBlurY", "ShdBlurY2").par("range", 2);

   // particles
   REPD(p, 2) // palette
   REPD(a, 3) // anim
   REPD(m, 2) // motion
      glsl.New().set("Particle", S+"ParticleTex"+(p?'P':'\0')+((a==2)?"AA":(a==1)?"A":"")+(m?'M':'\0')).par("palette",  p ).par("anim",  a ).par("motion_stretch", "1").par("stretch_alpha",  m );
      glsl.New().set("Particle", S+"Bilb"                                                             ).par("palette", "0").par("anim", "0").par("motion_stretch", "0").par("stretch_alpha", "0");

   // sky
      // Textures Flat
      REPD(t, 2) // textures
      REPD(c, 2) // clouds
         glsl.New().set("Sky", S+"SkyTF"+(t+1)+(c?'C':'\0')).par("per_vertex", "0").par("DENSITY", "0").par("textures", t+1).par("stars", "0").par("clouds", c);
      // Atmospheric Flat
      REPD(v, 2) // per-vertex
      REPD(s, 2) // stars
      REPD(c, 2) // clouds
         glsl.New().set("Sky", S+"SkyAF"+(v?'V':'\0')+(s?'S':'\0')+(c?'C':'\0')).par("per_vertex", v).par("DENSITY", "0").par("textures", "0").par("stars", s).par("clouds", c);

   // AA
#if 0 // disable GLSL versions because neither Mac/Linux succeed in compiling them
   glsl.New().set("FXAA"     , "FXAA"); 
   glsl.New().set("SMAAEdge" , "SMAAEdgeColor");
   glsl.New().set("SMAABlend", "SMAABlend");
   glsl.New().set("SMAA"     , "SMAA");
#endif

   Add(src_path+"Main.cpp", dest_path+"Main", model, glsl);
#endif

#ifdef BLEND
{
   Str names; Memc<ShaderGLSL> glsl;

   REPD(skin     , 2)
   REPD(color    , 2)
   REPD(rflct    , 2)
   REPD(textures , 3)
   REPD(light_map, 2)
   {
       names+=(TechBlend    (skin, color, rflct, textures, light_map));
      glsl.add(TechBlendGlsl(skin, color, rflct, textures, light_map));
   }

   Add(src_path+"Blend.cpp", dest_path+"Blend", model, names, glsl);
}
#endif

#ifdef AMBIENT_OCCLUSION
   Add(src_path+"Ambient Occlusion.cpp", dest_path+"Ambient Occlusion", model);
#endif

#ifdef VOLUMETRIC_LIGHTS
   Add(src_path+"Volumetric Lights.cpp", dest_path+"Volumetric Lights", model);
#endif

#ifdef VOLUMETRIC_CLOUDS
   Add(src_path+"Volumetric Clouds.cpp", dest_path+"Volumetric Clouds", model);
#endif

#ifdef LAYERED_CLOUDS
   Add(src_path+"Layered Clouds.cpp", dest_path+"Layered Clouds", model);
#endif

#ifdef HDR
   Add(src_path+"Hdr.cpp", dest_path+"Hdr", model);
#endif

#ifdef MOTION_BLUR
   Add(src_path+"Motion Blur.cpp", dest_path+"Motion Blur", model);
#endif

#ifdef DEPTH_OF_FIELD
   Add(src_path+"Depth of Field.cpp", dest_path+"Depth of Field", model);
#endif

#ifdef WATER
   Add(src_path+"Water.cpp", dest_path+"Water", model);
#endif

#ifdef POSITION
{
   Str names;

   REPD(tess, (model>=SM_4) ? 2 : 1)
   {
      // base
      REPD(skin      , 2               )
      REPD(textures  , 3               )
      REPD(test_blend, textures ? 2 : 1)names+=TechPosition(skin, textures, test_blend, FX_NONE, tess);
   }

   // grass + leafs
   for(Int textures=1; textures<=2; textures++)
   REPD(test_blend, 2)
   {
      names+=TechPosition(0, textures, test_blend, FX_GRASS, false);
      names+=TechPosition(0, textures, test_blend, FX_LEAF , false);
      names+=TechPosition(0, textures, test_blend, FX_LEAFS, false);
   }

   Add(src_path+"Position.cpp", dest_path+"Position", model, names);
}
#endif

#ifdef SET_COLOR
{
   Str names;

   // base
   REPD(tess    , (model>=SM_4) ? 2 : 1)
   REPD(skin    , 2)
   REPD(textures, 3)names+=TechSetColor(skin, textures, tess);

   Add(src_path+"Set Color.cpp", dest_path+"Set Color", model, names);
}
#endif

#ifdef BEHIND
{
   Str names;

   // base
   REPD(skin    , 2)
   REPD(textures, 3)names+=TechBehind(skin, textures);

   Add(src_path+"Behind.cpp", dest_path+"Behind", model, names);
}
#endif

#ifdef AMBIENT
{
   Str names;

   // base
   REPD(skin      , 2)
   REPD(alpha_test, 3)names+=TechAmbient(skin, alpha_test);

   Add(src_path+"Ambient.cpp", dest_path+"Ambient", model, names);
}
#endif

#ifdef OVERLAY
{
   Str names;

   // base
   REPD(skin  , 2)
   REPD(normal, 2)names+=TechOverlay(skin, normal);

   Add(src_path+"Overlay.cpp", dest_path+"Overlay", model, names);
}
{
   Str names;

   // base
   REPD(tess, (model>=SM_4) ? 2 : 1)
   REPD(skin, 2)names+=TechTattoo(skin, tess);

   Add(src_path+"Tattoo.cpp", dest_path+"Tattoo", model, names);
}
#endif

#ifdef EARLY_Z
{
   Str names;

   // base
   REPD(skin, 2)names+=TechEarlyZ(skin);

   Add(src_path+"Early Z.cpp", dest_path+"Early Z", model, names);
}
#endif

#ifdef FUR
{
   Str names;

   // base
   REPD(skin   , 2)
   REPD(size   , 2)
   REPD(diffuse, 2)names+=TechFurBase(skin, size, diffuse);

   // soft
   REPD(skin   , 2)
   REPD(size   , 2)
   REPD(diffuse, 2)names+=TechFurSoft(skin, size, diffuse);

   Add(src_path+"Fur.cpp", dest_path+"Fur", model, names);
}
#endif

#ifdef WORLD_EDITOR
   Add(src_path+"World Editor.cpp", dest_path+"World Editor", model);
#endif

#ifdef DX10_INPUT_LAYOUT
   Add(src_path+"DX10+ Input Layout.cpp", S, model);
#endif
}
/******************************************************************************/
static void ThreadCompile(ShaderCompiler &shader_compiler, Ptr user, Int thread_index)
{
   ThreadMayUseGPUData();
   shader_compiler.compile();
}
/******************************************************************************/
#endif // COMPILE
/******************************************************************************/
void MainShaderClass::compile()
{
#if COMPILE_3 || COMPILE_4 || COMPILE_GL
   App.stayAwake(AWAKE_SYSTEM);

#if COMPILE_GL
   Compile(SM_GL);
#endif
#if COMPILE_3
   Compile(SM_3);
#endif
#if COMPILE_4
   Compile(SM_4);
#endif

   ProcPriority(-1); // compiling shaders may slow down entire CPU, so make this process have smaller priority
   Dbl t=Time.curTime();
   MultiThreadedCall(ShaderCompilers, ThreadCompile);
   LogN(S+"Shaders compiled in: "+Flt(Time.curTime()-t)+'s');

   App.stayAwake(AWAKE_OFF);
#endif
}
/******************************************************************************/
}
/******************************************************************************/
