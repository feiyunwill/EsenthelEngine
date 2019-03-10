/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************

   !! Warning: Never return the same shader for Multi-Materials as Single-Materials !!
      Because crash/memory corruption may occur, since they need to operate on different containers 'MultiMaterialShaderDraws' and 'ShaderDraws'

/******************************************************************************/
ShaderImage* FindShaderImage(CChar8 *name) {return ShaderImages.find(Str8Temp(name));}
ShaderImage*  GetShaderImage(CChar8 *name) {ShaderImage *image=FindShaderImage(name); if(!image)Exit(S+"Shader Image \""+name+"\" not found."); return image;}

ShaderParam* FindShaderParam(CChar8 *name)
{
   Str8Temp key(name); // use 'Str8Temp' to prevent memory allocation
   if(ShaderParam *sp=ShaderParams.find(key))return sp;

   // maybe we're looking for array member
   FREPA(key)
   {
      if(name[i]=='[') // check for existence of '['
      {
         Char8 parent_name[MAX_LONG_PATH]; if(InRange(i, parent_name))
         {
            i++; Set(parent_name, name, i); // copy all before '['
            if(ShaderParam *parent=ShaderParams.find(Str8Temp(parent_name)))
            {
               Int index=TextInt(name+i); if(InRange(index, parent->_elements))
               {
                  ShaderParams.lock  (); ShaderParam &elm=*ShaderParams(key); if(!elm.is())elm.initAsElement(*parent, index); // init only if not initialized yet, in case it just got initialized on another thread before the lock
                  ShaderParams.unlock();
                  return &elm;
               }
            }
         }
         break;
      }
   }

   return null;
}
ShaderParam* GetShaderParam(CChar8 *name) {ShaderParam *param=FindShaderParam(name); if(!param)Exit(S+"Shader Param \""+name+"\" not found."); return param;}
/******************************************************************************/
ShaderBuffer* FindShaderBuffer(CChar8 *name) {return ShaderBuffers.find(Str8Temp(name));}
ShaderBuffer*  GetShaderBuffer(CChar8 *name) {ShaderBuffer *sb=FindShaderBuffer(name); if(!sb)Exit(S+"Shader Buffer \""+name+"\" not found."); return sb;}
/******************************************************************************/
static Int Textures(C Material &material)
{
   if(material.base_0)
   {
      if(material.base_1)return 2;
                         return 1;
   }                     return 0;
}
static Int BumpMode(C Material &material, UInt mesh_base_flag)
{
   if(mesh_base_flag&VTX_NRM)
   {
      if((mesh_base_flag&VTX_TEX0) && (mesh_base_flag&VTX_TAN) && D.bumpMode()>=BUMP_NORMAL && material.base_0 && material.base_1)
      {
         if(D.bumpMode()>BUMP_NORMAL && material.bump>EPS_MATERIAL_BUMP)
         {
            if(D.bumpMode()==BUMP_RELIEF)return SBUMP_RELIEF;
            return Mid(Ceil(material.bump/0.0075f)+SBUMP_PARALLAX0, SBUMP_PARALLAX_MIN, SBUMP_PARALLAX_MAX);
         }
         if(material.rough>EPS_COL)return SBUMP_NORMAL;
      }
      return SBUMP_FLAT;
   }
   return SBUMP_ZERO;
}
static Bool Detail (C Material &material) {return material.    detail_map && material.det_power>EPS_COL &&  material.det_scale;}
static Bool Macro  (C Material &material) {return material.     macro_map;}
static Bool Reflect(C Material &material) {return material.reflection_map && material.reflect  >EPS_COL && !material.hasGrass() && !material.hasLeaf();}

static UInt FlagHeightmap(UInt mesh_base_flag, Bool heightmap)
{
   if(heightmap)
   {
      if(mesh_base_flag&VTX_POS)mesh_base_flag|=VTX_TEX0; // heightmap shaders generate tex from pos
      if(mesh_base_flag&VTX_NRM)mesh_base_flag|=VTX_TAN ; // heightmap shaders generate tan from nrm
   }
   return mesh_base_flag;
}
/******************************************************************************/
DefaultShaders::DefaultShaders(C Material *material, UInt mesh_base_flag, Int lod_index, Bool heightmap)
{
 C Material *materials[4]=
   {
      material,
      null    ,
      null    ,
      null    ,
   };
   init(materials, mesh_base_flag, lod_index, heightmap);
}
void DefaultShaders::init(C Material *material[4], UInt mesh_base_flag, Int lod_index, Bool heightmap)
{
   if(!mesh_base_flag){set_empty: valid=false; return;}
 C Material *m=(material ? material[0] : null); if(!m){if(D.drawNullMaterials())m=&MaterialDefault;else goto set_empty;}
   valid=true;

   mesh_base_flag=FlagHeightmap(mesh_base_flag, heightmap);

   // !! Never return the same shader for Multi-Materials as Single-Materials !!
 T.heightmap=heightmap;
   materials=1;
   textures =0;
   bump     =SBUMP_ZERO;
   detail   =false;
   macro    =false;
   reflect  =false;
   Bool tex =((mesh_base_flag&VTX_TEX0  )!=0);
   normal   =((mesh_base_flag&VTX_NRM   )!=0);
   color    =((mesh_base_flag&VTX_COLOR )!=0);
   size     =((mesh_base_flag&VTX_SIZE  )!=0);

   if(material)
   {
      MAX(textures, Textures(*m)); MAX(bump, BumpMode(*m, mesh_base_flag)); MAX(detail, Detail(*m)); MAX(macro, Macro(*m)); MAX(reflect, Reflect(*m));
      if(material[1]) // && (mesh_base_flag&VTX_MATERIAL)) we must always return a different shader even when there's no VTX_MATERIAL component, because we need a different shader for multi-material parts that have umm, as they operate on 'MultiMaterialShaderDraws' and not 'ShaderDraws', otherwise crash or memory corruption may occur, because ShaderBase.shader_index would point to wrong container
      {
         materials++; MAX(textures, Textures(*material[1])); MAX(bump, BumpMode(*material[1], mesh_base_flag)); MAX(detail, Detail(*material[1])); MAX(macro, Macro(*material[1])); MAX(reflect, Reflect(*material[1]));
         if(material[2])
         {
            materials++; MAX(textures, Textures(*material[2])); MAX(bump, BumpMode(*material[2], mesh_base_flag)); MAX(detail, Detail(*material[2])); MAX(macro, Macro(*material[2])); MAX(reflect, Reflect(*material[2]));
         #if MAX_MTRLS>=4
            if(material[3])
            {
               materials++; MAX(textures, Textures(*material[3])); MAX(bump, BumpMode(*material[3], mesh_base_flag)); MAX(detail, Detail(*material[3])); MAX(macro, Macro(*material[3])); MAX(reflect, Reflect(*material[3]));
            }
         #endif
         }
      }
      switch(D.texDetail())
      {
         case TEX_USE_DISABLE:                detail=false; break;
         case TEX_USE_SINGLE : if(materials>1)detail=false; break;
      }
      switch(D.texReflection())
      {
         case TEX_USE_DISABLE:                reflect=false; break;
         case TEX_USE_SINGLE : if(materials>1)reflect=false; break;
      }
      if(!D.texMacro() || lod_index<=0)macro =false; // disable macro  for LOD's=0
      if(                 lod_index> 0)detail=false; // disable detail for LOD's>0
      if(                 lod_index> 0)MIN(bump, SBUMP_NORMAL); // limit to normal mapping for LOD's>0
      if(!tex                         ){detail=macro=false; textures=0;}
      if(!normal                      )reflect=false;
      if(materials>1                  )MAX(textures, 1); // multi-materials don't support 0 textures
      if(materials>1 && (bump>SBUMP_PARALLAX_MAX_MULTI && bump<=SBUMP_PARALLAX_MAX))bump=SBUMP_PARALLAX_MAX_MULTI; // multi-materials have a different limit for parallax steps
   }

   fur              =(normal && tex                       && materials==1 &&                !heightmap && m->technique==MTECH_FUR  ); // this requires tex coordinates, but not a material texture, we can do fur with just material color and 'FurCol'
   blend            =(                                       materials==1 &&                !heightmap && m->technique==MTECH_BLEND); // this shouldn't require a texture, we can do alpha blending with just material color
   grass            =(normal &&                              materials==1 && textures>=1 && !heightmap && m->hasGrass            ());
   leaf             =(normal && (mesh_base_flag&VTX_HLP)  && materials==1 && textures>=1 && !heightmap && m->hasLeaf             () && D.bendLeafs());
   ambient          =(                                                                                    m->ambient.max()>EPS_COL); // this doesn't operate on a texture, 'materials' are checked in 'Ambient' method because this member is used only there
   alpha            =(                                       materials==1 && textures>=1 && !heightmap && m->hasAlpha           ()); // this is about having alpha channel in material textures so we need a texture
   alpha_test       =(                                       materials==1 && textures>=1 && !heightmap && m->hasAlphaTest       ());
   alpha_blend      =(                                       materials==1 &&                !heightmap && m->hasAlphaBlend      ()); // this shouldn't require a texture, we can do alpha blending with just material color
   alpha_blend_light=(                                       materials==1 &&                !heightmap && m->hasAlphaBlendLight ()); // this shouldn't require a texture, we can do alpha blending with just material color
    mtrl_blend      =(                                       materials> 1 && D.materialBlend()                                    ); // this is multi-material blending (blending between multiple materials)
   skin             =((mesh_base_flag&VTX_SKIN)==VTX_SKIN && materials==1 &&                !heightmap && !grass && !leaf         );
   fx               =(grass ? FX_GRASS : leaf ? (size ? FX_LEAFS : FX_LEAF) : FX_NONE);
   light_map        =((mesh_base_flag&VTX_TEX1) &&           materials==1 && textures>=1 && m->light_map && !fx);
   tess             =((lod_index<=0) && D.shaderModel()>=SM_5 && D.tesselation() && (!heightmap || D.tesselationHeightmap()) && normal && !fx);

   if(fx){detail=macro=false; MIN(bump, SBUMP_NORMAL);} // currently shaders with effects don't support detail/macro/fancy bump
}
Shader* DefaultShaders::EarlyZ()C
{
#if SUPPORT_EARLY_Z
   if(valid && !alpha_blend && !alpha_test && !fx && !tess)return ShaderFiles("Early Z")->get(TechNameEarlyZ(skin));
#endif
   return null;
}
Shader* DefaultShaders::Simple()C
{
   if(valid && !alpha_blend)
   {
      // !! Never return the same shader for Multi-Materials as Single-Materials !!
      Str8 name;
      if(normal      )name=TechNameSimple(skin, materials, (materials>1) ? 1 : textures, SBUMP_FLAT, alpha_test, light_map, reflect, color, mtrl_blend, heightmap, fx, Renderer.simplePrecision(), tess);else // simple supports only 1 texture for multi-materials (there's no alpha testing/blending, and no support for 0 textures)
      if(materials==1)name=TechNameSimple(skin, materials,                            0, SBUMP_ZERO, false     , light_map, reflect, color, mtrl_blend, heightmap, fx, Renderer.simplePrecision(), tess);
      return ShaderFiles("Simple")->get(name);
   }
   return null;
}
Shader* DefaultShaders::Solid(Bool mirror)C
{
   if(valid && !alpha_blend && Renderer.anyDeferred())
   {
      // !! Never return the same shader for Multi-Materials as Single-Materials !!
      if(fur)return ShaderFiles("Fur")->get(TechNameFurBase(skin, size, textures!=0));
      Bool detail=T.detail, tess=T.tess; Byte bump=T.bump; if(mirror){detail=false; tess=false; MIN(bump, SBUMP_NORMAL);} // disable detail tesselation and fancy bump for mirror
      Str8 name;
      if(normal      )name=TechNameDeferred(skin, materials, textures,  bump     , alpha_test, light_map, detail, macro, reflect, color, mtrl_blend, heightmap, fx, tess);else
      if(materials==1)name=TechNameDeferred(skin, materials,        0, SBUMP_ZERO, false     , light_map, false , false, reflect, color, mtrl_blend, heightmap, fx, tess);
      return ShaderFiles("Deferred")->get(name);
   }
   return null;
}
Shader* DefaultShaders::Ambient()C
{
#if SUPPORT_MATERIAL_AMBIENT
   if(valid && !alpha_blend && ambient && materials==1 && !heightmap && !fx)return ShaderFiles("Ambient")->get(TechNameAmbient(skin, alpha_test ? textures : 0));
#endif
   return null;
}
Shader* DefaultShaders::Outline()C
{
   if(valid && !alpha_blend && !fx)return ShaderFiles("Set Color")->get(TechNameSetColor(skin, alpha_test ? textures : 0, tess));
   return null;
}
Shader* DefaultShaders::Behind()C
{
   if(valid && !fx)return ShaderFiles("Behind")->get(TechNameBehind(skin, alpha_test ? textures : 0));
   return null;
}
Shader* DefaultShaders::Fur()C
{
   if(valid && fur)return ShaderFiles("Fur")->get(TechNameFurSoft(skin, size, textures!=0));
   return null;
}
Shader* DefaultShaders::Shadow()C
{
   if(valid && (!alpha_blend || alpha_test))return ShaderFiles("Position")->get(TechNamePosition(skin, alpha_test ? textures : 0, alpha_test && alpha_blend_light, fx, tess));
   return null;
}
Shader* DefaultShaders::Blend()C
{
   if(valid && blend) // "!blend" here will return null so BLST can be used in 'drawBlend'
      return ShaderFiles("Blend")->get(TechNameBlend(skin, color, reflect, textures, light_map));
   return null;
}
Shader* DefaultShaders::Overlay()C
{
   if(valid)return ShaderFiles("Tattoo")->get(TechNameTattoo(skin, tess));
   return null;
}
Shader* DefaultShaders::get(RENDER_MODE mode)C
{
   switch(mode)
   {
      default        : return null;
      case RM_EARLY_Z: return EarlyZ();
      case RM_SIMPLE : return Simple();
      case RM_SOLID  : return Solid();
      case RM_SOLID_M: return Solid(true);
      case RM_AMBIENT: return Ambient();
      case RM_OUTLINE: return Outline();
      case RM_BEHIND : return Behind();
      case RM_FUR    : return Fur();
      case RM_SHADOW : return Shadow();
      case RM_BLEND  : return Blend();
      case RM_OVERLAY: return Overlay();
   }
}
FRST* DefaultShaders::Frst()C
{
   if(valid && !alpha_blend && Renderer.anyForward())
   {
      FRSTKey key;
      key.skin      =skin;
      key.materials =materials;
      key.textures  =textures;
      key.bump_mode =Min(bump, SBUMP_NORMAL); // forward supports only normal bump
      key.alpha_test=alpha_test;
      key.light_map =light_map;
      key.detail    =(detail && materials==1); // forward doesn't support detail in multi-material
      key.rflct     =reflect;
      key.color     =color;
      key.mtrl_blend=mtrl_blend;
      key.fx        =fx;
      key.heightmap =heightmap;
      key.tess      =tess;
      return Frsts(key);
   }
   return null;
}
BLST* DefaultShaders::Blst()C
{
   if(valid
 //&& alpha_blend_light - always return because 'Mesh.drawBlend' may use it
   && normal // lighting requires vertex normals
   && materials==1
   )
   {
      BLSTKey key;
      key.per_pixel =((Renderer.type()==RT_SIMPLE) ? Renderer.simplePrecision() : true);
      key.color     =color;
      key.textures  =textures;
      key.bump_mode =Min(bump, (Renderer.type()==RT_SIMPLE) ? SBUMP_FLAT : SBUMP_NORMAL); // blend light supports only flat/normal bump
      key.alpha_test=alpha_test;
      key.alpha     =alpha;
      key.light_map =light_map;
      key.rflct     =reflect;
      key.skin      =skin;
      key.fx        =fx;
      return Blsts(key);
   }
   return null;
}
void DefaultShaders::set(Shader *shader[RM_SHADER_NUM], FRST **frst, BLST **blst)
{
   if(shader)
   {
      shader[RM_EARLY_Z]=EarlyZ();
      shader[RM_SIMPLE ]=Simple();
      shader[RM_SOLID  ]=Solid();
      shader[RM_SOLID_M]=Solid(true);
      shader[RM_AMBIENT]=Ambient();
      shader[RM_OUTLINE]=Outline();
      shader[RM_BEHIND ]=Behind();
      shader[RM_FUR    ]=Fur();
      shader[RM_SHADOW ]=Shadow();
      shader[RM_BLEND  ]=Blend();
      shader[RM_OVERLAY]=Overlay();
   }
   if(frst)*frst=Frst();
   if(blst)*blst=Blst();
}
/******************************************************************************/
}
/******************************************************************************/
