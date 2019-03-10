/******************************************************************************/
#include "stdafx.h"
/******************************************************************************

   Rendering is sorted by:
      -Shaders
      -Materials
      -Meshes
      -Instances

   Most Materials will use only 1 Shader (only few cases will use more)
   Because of that, Materials have one link ('MaterialShader') to the Shader already included in Material object (to reduce overhead).
   In cases where Material needs another shader, then that link can point to next link in 'MaterialShaders' container (this is a linked list, where links can continuously point to next links in that container)
   With this mechanism we have unique MaterialShader objects (Material -> Shader), however during rendering we need to have links the other way around (Shader -> Material).
   Because first we set Shaders, then iterate all Materials belonging to this Shader.
   All draw calls will store 'ShaderDraw' first, and then that object will have a linked list of all Materials belonging to this Shader ('ShaderMaterial' objects).
   Then 'ShaderMaterial' has a linked list of all meshes 'ShaderMaterialMesh', which then have instances '*ShaderMaterialMeshInstance'.

   'ShaderMaterial' needs to be separate from 'MaterialShader', because:
      'MaterialShader' is a linked list of   shaders for 1 material
      'ShaderMaterial' is a linked list of materials for 1 shader

   However all instances are managed per MeshPart.Variation, which is not perfect, because for example, MeshPart can have multiple variations, and some of them may have the same material.
   Even though material is the same, they will not be instanced together. We would have to manage some sort of "unique variation" list in the MeshPart, which would complicate things.

TODO: most of (highlight, shader_param_changes, stencil) instance parameters are never used, can we do something about it? to remove overhead and memory usage
TODO: on stereo rendering some pixels are wasted, use a stencil mask, see - http://media.steampowered.com/apps/valve/2015/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
/******************************************************************************/
#define MAX_MATRIX_INSTANCE (GL ? MAX_MATRIX_DX9 : MAX_MATRIX_DX10) // if instancing is supported then we have all possible matrixes
/******************************************************************************/
namespace EE{
/******************************************************************************/
static Color    Highlight;
       Bool _SetHighlight(C Color &color)
{
   if(color!=Highlight){Renderer.highlight->set(Highlight=color); return true;}
   return false;
}
void SetDrawMask    (  UInt     draw_mask   ) {Renderer._mesh_draw_mask    =draw_mask;}
void SetVariation   (   Int     variation   ) {Renderer._mesh_variation_1  =variation-1;} // offset it over here, instead of every time in mesh draw
void SetHighlight   (C Color   &color       ) {Renderer._mesh_highlight    =color;}
void SetStencilValue(  Bool     terrain     ) {Renderer._mesh_stencil_value=(terrain      ? STENCIL_REF_TERRAIN  : STENCIL_REF_ZERO);}
void SetStencilMode (  Bool     terrain_only) {Renderer._mesh_stencil_mode =(terrain_only ? STENCIL_TERRAIN_TEST : STENCIL_NONE    );}
void SetBlendAlpha  (ALPHA_MODE alpha       ) {Renderer._mesh_blend_alpha  =alpha;}
void SetEarlyZ      (  Bool     on          ) {Renderer._mesh_early_z      =on;}
void SetBehindBias  (  Flt      distance    ) {Sh.h_BehindBias->setConditional(distance);}

static INLINE void DisableSkinning() {Sh.h_VtxSkinning->setConditional(0.0f);}
static INLINE void  EnableSkinning() {Sh.h_VtxSkinning->setConditional(1.0f);}
/******************************************************************************
// SHADER PARAM CHANGES
/******************************************************************************/
void LinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)
{
   if(!Renderer._shader_param_changes)Renderer._shader_param_changes=&changes;else Exit("Trying to 'LinkShaderParamChanges' while there are other changes that weren't unlinked.");
}
void UnlinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)
{
   if(Renderer._shader_param_changes==&changes)Renderer._shader_param_changes=null;else Exit("Trying to 'UnlinkShaderParamChanges' that weren't linked.");
}
struct ShaderParamRestore
{
   ShaderParam *param;
   Vec4         value;

   void restore()C {param->setSafe(value);} // have to use 'setSafe' because the 'ShaderParam' can be smaller than what we're setting

   void operator=(ShaderParam *param) {T.param=param; T.value=*(Vec4*)param->_data;} // set param and remember its current value
};
static   Memc<ShaderParamRestore> RestoreChanges;
static C Memc<ShaderParamChange >   *LastChanges;

static Bool SetShaderParamChanges(C Memc<ShaderParamChange> *changes=null)
{
   if(LastChanges!=changes)
   {
      // restore previous changes if any
      REPAO(RestoreChanges).restore(); // restore in backwards order
      RestoreChanges.clear();

      // apply new changes
      if(LastChanges=changes)REPA(*LastChanges)
      {
       C ShaderParamChange &change=(*LastChanges)[i];
         if(change.param)
         {
            RestoreChanges.add(change.param); // add for restoration
            change.param->setSafe(change.value); // apply change, have to use 'setSafe' because the 'ShaderParam' can be smaller than what we're setting
         }
      }
      return true;
   }
   return false;
}
/******************************************************************************/
void ShaderMaterial::unlink()
{
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   if(..)material->unlink(); // need to detect if this is 'material' or 'umm', and we can't check smm->mesh->_umm because this could have already been changed with MeshPart.material call, or this could be ShaderMaterial for shadow which ignores umm
   else  umm     ->unlink();
#else
   if(ShaderMaterialMesh *smm=ShaderMaterialMeshes.addr(first_shader_material_mesh))
   {
      if(C Material                *material=smm->Variation().material())material->unlink();
      if(  UniqueMultiMaterialData *umm     =smm->mesh->_umm            )umm     ->unlink();
   }
#endif
}
/******************************************************************************/
Memc<Material::MaterialShader        > MaterialShaders;
Memc<      ShaderDraw                > ShaderDraws, MultiMaterialShaderDraws;
Memc<      ShaderMaterial            >       ShaderMaterials;
Memc<      ShaderMaterialMesh        >       ShaderMaterialMeshes;
Memc< SolidShaderMaterialMeshInstance>  SolidShaderMaterialMeshInstances;
Memc<ShadowShaderMaterialMeshInstance> ShadowShaderMaterialMeshInstances;
Memc<                 AmbientInstance>                  AmbientInstances;

Memc<     SkeletonShader                    > SkeletonShaders;
Memc<     SkeletonBlendShader               > SkeletonBlendShaders;
Memc<     SkeletonShaderMaterial            > SkeletonShaderMaterials, SkeletonBlendShaderMaterials;
Memc<     SkeletonShaderMaterialMeshInstance> SkeletonShadowShaderMaterialMeshInstances;
Memc<SkeletonSolidShaderMaterialMeshInstance>  SkeletonSolidShaderMaterialMeshInstances;
Memc<SkeletonBlendShaderMaterialMeshInstance>  SkeletonBlendShaderMaterialMeshInstances;
     SkeletonInstances                         SkeletonSolidInstances, SkeletonShadowInstances;
Memc<SkeletonAmbientInstance                >  SkeletonAmbientInstances;

BlendInstancesClass BlendInstances;
ClothInstances      SolidClothInstances, ShadowClothInstances;

GameObjects PaletteObjects, Palette1Objects, OverlayObjects, SolidObjects, AmbientObjects, OutlineObjects, BehindObjects;
GameAreas   PaletteAreas  , Palette1Areas;
/******************************************************************************/
#if 0 // this doesn't work, because we need to adjust ViewMatrix and not ProjMatrix (for example Shadows in Forward Renderer will not work, because VS_PS.pos in shaders depends on ViewMatrix only, it affects shadows, and reflections, etc.)
// precomputed view matrixes were generated for the first eye, so for next eye we need to adjust the projection matrix (instead of adjusting camera and recalculating view matrixes which would be slower)
static void BeginPrecomputedViewMatrix() {if(/*Renderer._stereo && */Renderer._eye!=0)SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye], -D.eyeDistance());}
static void   EndPrecomputedViewMatrix() {if(/*Renderer._stereo && */Renderer._eye!=0)SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye]);}
struct EyeCache
{
   Int cur_eye, normal_eye, precomputed_eye;

   EyeCache() {cur_eye=normal_eye=Renderer._eye; precomputed_eye=0;}

   void setEye(Int eye) {if(cur_eye!=eye){cur_eye=eye; SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye], eye ? 0 : -D.eyeDistance());}}

   INLINE void BeginPrecomputedViewMatrix() {setEye(precomputed_eye);}
   INLINE void   EndPrecomputedViewMatrix() {setEye(     normal_eye);}
};
static INLINE void SetViewOffset() {}
static INLINE void SetViewMatrix(C Matrix &view_matrix, Int i=0) {ViewMatrix[i]=view_matrix;} // !! Warning: this doesn't call 'setChanged' !!
#else
static INLINE void BeginPrecomputedViewMatrix() {}
static INLINE void   EndPrecomputedViewMatrix() {}
struct EyeCache
{
   INLINE void BeginPrecomputedViewMatrix() {}
   INLINE void   EndPrecomputedViewMatrix() {}
};
static Flt ViewOffset;
static INLINE void SetViewOffset() {ViewOffset=(Renderer._eye ? -D.eyeDistance() : 0);}
static INLINE void SetViewMatrix(C Matrix &view_matrix, Int i=0) {ViewMatrix[i]=view_matrix; ViewMatrix[i]._x+=ViewOffset;} // !! Warning: this doesn't call 'setChanged' !!
#endif
/******************************************************************************/
// EARLY Z
/******************************************************************************/
#if SUPPORT_EARLY_Z
Memc<EarlyZInstance> EarlyZInstances[2];
Bool HasEarlyZInstances()
{
   return (EarlyZInstances[0].elms()+EarlyZInstances[1].elms())>0;
}
void DrawEarlyZInstances()
{
   SetViewOffset();
   BeginPrecomputedViewMatrix();
   D.depth(true);
   SetMatrixCount();
   DisableSkinning();
   Renderer._shader_early_z->start(); // this shader doesn't require any textures
   // TODO: this could be optimized to group in state changes (mesh->instance) and allow instancing
   FREPA(EarlyZInstances)
   {
    C Memc<EarlyZInstance> &instances=EarlyZInstances[i];
      if(instances.elms())
      {
         D.cull(i);
         FREPA(instances)
         {
          C EarlyZInstance &instance=instances[i];
            SetViewMatrix(instance.view_matrix); Sh.h_ViewMatrix->setChanged();
            Renderer._shader_early_z->commit();
            instance.mesh->set().drawFull();
         }
      }
   }
   ShaderEnd();
   EndPrecomputedViewMatrix();
}
void ClearEarlyZInstances()
{
   EarlyZInstances[0].clear();
   EarlyZInstances[1].clear();
}
#endif
/******************************************************************************/
// SOLID
/******************************************************************************/
static INLINE void DrawSolidInstances(Bool forward)
{
   SetViewOffset();
   BeginPrecomputedViewMatrix();

   const Bool instancing=D.hwInstancing();
   D.depth(true);

   // solid
   SetMatrixCount();
   DisableSkinning();
   FREPA(ShaderDraws)
   {
      ShaderDraw &shader_draw=ShaderDraws[i]; shader_draw.unlink(); // have to unlink 'shader_draw' instead of 'shader' (because it can be FRST)
      Shader     &shader     =shader_draw.shader->getShader(forward); shader.start();
      for(ShaderMaterial *shader_material=&ShaderMaterials[shader_draw.first_shader_material]; ; )
      {
         ShaderMaterialMesh  *shader_material_mesh=&ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
      #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
       C Material            &material            =*shader_material->material;
      #else
       C MeshPart::Variation *variation           =&shader_material_mesh->Variation(); // have to access 'variation' here, because we need it to access 'material' since we're not storing it in the 'ShaderMaterial'
       C Material            &material            = variation->getMaterial();
      #endif
         material.unlinkSolid(); material.setSolid(); D.cull(material.cull); shader.commitTex();

         for(;;)
         {
          C MeshPart   &mesh=*shader_material_mesh->mesh;
         #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            mesh.unlinkSolid();
         #else
            variation->unlinkSolid();
         #endif
          C MeshRender &render=mesh.render.set();
          C Bool        instancing_mesh=(instancing && !(render.flag()&VTX_SKIN)); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
            Sh.h_VtxHeightmap->setConditional(mesh._vtx_heightmap);

            for(SolidShaderMaterialMeshInstance *instance=&SolidShaderMaterialMeshInstances[shader_material_mesh->first_instance]; ; )
            {
               SetViewMatrix        (instance->view_matrix         ); Sh.h_ViewMatrix->setChanged();
               SetFastVel           (instance->vel                 );
               SetFastAngVel        (instance->ang_vel_shader      );
              _SetHighlight         (instance->highlight           );
               SetShaderParamChanges(instance->shader_param_changes);
               D.stencilRef         (instance->stencil_value       );
               Int instances=1;
               if( instancing_mesh)for(; instance->next_instance>=0; )
               {
                  SolidShaderMaterialMeshInstance &next=SolidShaderMaterialMeshInstances[instance->next_instance];
                  if(next.highlight           ==instance->highlight
                  && next.shader_param_changes==instance->shader_param_changes
                  && next.stencil_value       ==instance->stencil_value
               #if !PER_INSTANCE_VEL
                  && next.vel==instance->vel
               #endif
               #if !PER_INSTANCE_ANG_VEL
                  && next.ang_vel_shader==instance->ang_vel_shader
               #endif
                  )
                  {
                     instance=&next;
                     SetViewMatrix(next.view_matrix, instances);
                  #if PER_INSTANCE_VEL
                     SetFastVel(instances, next.vel);
                  #endif
                  #if PER_INSTANCE_ANG_VEL
                     not supported
                  #endif
                     if(++instances>=MAX_MATRIX_INSTANCE)break;
                  }else break;
               }
               SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull();

                                                       if(instance->next_instance<0)break;
               instance=&SolidShaderMaterialMeshInstances[instance->next_instance];
            }
                                                    if(shader_material_mesh->next_shader_material_mesh<0)break;
            shader_material_mesh=&ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
         #if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            variation           =&shader_material_mesh->Variation();
         #endif
         }

         SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
         ShaderEnd(); // needs to be called after every 'commitTex'
                                       if(shader_material->next_shader_material<0)break;
         shader_material=&ShaderMaterials[shader_material->next_shader_material];
      }
   }
   D.stencilRef(0);

   EndPrecomputedViewMatrix();

   // skeleton
   EnableSkinning();
   FREPA(SkeletonSolidInstances)
   {
          SkeletonInstance &skel=SkeletonSolidInstances[i]; skel.unlinkSolid(); skel.anim_skel->setMatrix();
      for(SkeletonShader   *skel_shader=&skel.skel_shader; ; )
      {
         Shader &shader=skel_shader->shader->getShader(forward); shader.start();
         for(SkeletonShaderMaterial *skel_shader_material=&skel_shader->material; ; )
         {
          C Material &material=*skel_shader_material->material; material.setSolid(); D.cull(material.cull); shader.commitTex();
            Bool shader_params_changed=true;
            for(SkeletonSolidShaderMaterialMeshInstance *instance=&SkeletonSolidShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance]; ; )
            {
                  shader_params_changed|=_SetHighlight         (instance->highlight);
                  shader_params_changed|= SetShaderParamChanges(instance->shader_param_changes);
               if(shader_params_changed){shader_params_changed=false; shader.commit();}
               instance->mesh->set().draw(); // for skeleton have to use 'draw' instead of 'drawFull'
                                                               if(instance->next_instance<0)break;
               instance=&SkeletonSolidShaderMaterialMeshInstances[instance->next_instance];
            }
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            ShaderEnd(); // needs to be called after every 'commitTex'
                                                       if(skel_shader_material->next_skeleton_shader_material<0)break;
            skel_shader_material=&SkeletonShaderMaterials[skel_shader_material->next_skeleton_shader_material];
         }
                                   if(skel_shader->next_skeleton_shader<0)break;
         skel_shader=&SkeletonShaders[skel_shader->next_skeleton_shader];
      }
   }

   // cloth
   if(SolidClothInstances.elms())
   {
      SetOneMatrix   ();
      SetFastAngVel  ();
      SetVtxNrmMulAdd(false);

      {
         SetDefaultVAO(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
         FREPA(SolidClothInstances)
         {
            ClothInstance &ci=SolidClothInstances[i];
            Shader   &shader  = ci.shader  ->getShader(forward);
          C Material &material=*ci.material; material.setSolid(); D.cull(material.cull);
            SetFastVel  (ci.vel      );
           _SetHighlight(ci.highlight);
            shader.begin(); ci.cloth->_drawPhysical();
            ShaderEnd();
         }
      }

      SetVtxNrmMulAdd(true);
   }

   BeginPrecomputedViewMatrix();

   // multi material (at last stage, because these are usually heightmaps on the bottom)
#define MULTI_MATERIAL_INSTANCING 0 // these are heightmaps, and we never need instancing for them because they're drawn only 1 time each
   SetMatrixCount();
   DisableSkinning();
   FREPA(MultiMaterialShaderDraws)
   {
      ShaderDraw &shader_draw=MultiMaterialShaderDraws[i]; shader_draw.unlink(); // have to unlink 'shader_draw' instead of 'shader' (because it can be FRST)
      Shader     &shader     =shader_draw.shader->getShader(forward); shader.start();
      for(ShaderMaterial *shader_material=&ShaderMaterials[shader_draw.first_shader_material]; ; )
      {
         ShaderMaterialMesh      *shader_material_mesh=&ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
      #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
         UniqueMultiMaterialData &umm                 =*shader_material->umm; umm.unlink();
      #else
         UniqueMultiMaterialData &umm                 =*shader_material_mesh->mesh->_umm; umm.unlink();
      #endif
       C UniqueMultiMaterialKey  &umm_key             =UniqueMultiMaterialMap.dataInMapToKey(umm);
         Bool cull=true;
         if(C Material *m=umm_key.m[0]){m->setMulti(0); cull&=m->cull;}else{MaterialDefault.setMulti(0); cull&=MaterialDefault.cull;}
         if(C Material *m=umm_key.m[1]){m->setMulti(1); cull&=m->cull;}
         if(C Material *m=umm_key.m[2]){m->setMulti(2); cull&=m->cull;}
         if(C Material *m=umm_key.m[3]){m->setMulti(3); cull&=m->cull;}
         D.cull(cull); shader.commitTex();

         for(;;)
         {
          C MeshPart            &mesh     =*shader_material_mesh->mesh;
          C MeshRender          &render   = mesh.render.set();
         #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            mesh.unlinkSolid();
         #else
          C MeshPart::Variation &variation= shader_material_mesh->Variation(); variation.unlinkSolid();
         #endif
         #if MULTI_MATERIAL_INSTANCING
          C Bool                 instancing_mesh=(instancing && !(render.flag()&VTX_SKIN)); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
         #endif
            Sh.h_VtxHeightmap->setConditional(mesh._vtx_heightmap);

            for(SolidShaderMaterialMeshInstance *instance=&SolidShaderMaterialMeshInstances[shader_material_mesh->first_instance]; ; )
            {
               SetViewMatrix        (instance->view_matrix         ); Sh.h_ViewMatrix->setChanged();
               SetFastVel           (instance->vel                 );
               SetFastAngVel        (instance->ang_vel_shader      );
              _SetHighlight         (instance->highlight           );
               SetShaderParamChanges(instance->shader_param_changes);
               D.stencilRef         (instance->stencil_value       );
               Int instances=1;
            #if MULTI_MATERIAL_INSTANCING
               if( instancing_mesh)for(; instance->next_instance>=0; )
               {
                  SolidShaderMaterialMeshInstance &next=SolidShaderMaterialMeshInstances[instance->next_instance];
                  if(next.highlight           ==instance->highlight
                  && next.shader_param_changes==instance->shader_param_changes
                  && next.stencil_value       ==instance->stencil_value
               #if !PER_INSTANCE_VEL
                  && next.vel==instance->vel
               #endif
               #if !PER_INSTANCE_ANG_VEL
                  && next.ang_vel_shader==instance->ang_vel_shader
               #endif
                  )
                  {
                     instance=&next;
                     SetViewMatrix(next.view_matrix, instances);
                  #if PER_INSTANCE_VEL
                     SetFastVel(instances, next.vel);
                  #endif
                  #if PER_INSTANCE_ANG_VEL
                     not supported
                  #endif
                     if(++instances>=MAX_MATRIX_INSTANCE)break;
                  }else break;
               }
               SetMatrixCount(instances);
            #endif
               shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull();

                                                       if(instance->next_instance<0)break;
               instance=&SolidShaderMaterialMeshInstances[instance->next_instance];
            }

                                                    if(shader_material_mesh->next_shader_material_mesh<0)break;
            shader_material_mesh=&ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
         }

         SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
         ShaderEnd(); // needs to be called after every 'commitTex'
                                       if(shader_material->next_shader_material<0)break;
         shader_material=&ShaderMaterials[shader_material->next_shader_material];
      }
   }
   D.stencilRef(0);

   // finish
  _SetHighlight(TRANSPARENT);
   EndPrecomputedViewMatrix();

   FREPAO(SolidObjects)->drawSolid();
}
void ClearSolidInstances()
{
   MaterialShaders                    .clear();
           ShaderDraws                .clear(); MultiMaterialShaderDraws.clear();
           ShaderMaterials            .clear();
           ShaderMaterialMeshes       .clear();
      SolidShaderMaterialMeshInstances.clear();

   SkeletonShaders                         .clear();
   SkeletonShaderMaterials                 .clear();
   SkeletonSolidShaderMaterialMeshInstances.clear();
   SkeletonSolidInstances                  .clear();

   SolidClothInstances.clear();
          SolidObjects.clear();
}
void DrawSolidInstances()
{
   if(Renderer._cur_type!=RT_FORWARD)DrawSolidInstances(false);
   else                              DrawSolidInstances(true );
}
/******************************************************************************/
// AMBIENT
/******************************************************************************/
static Int Compare(C AmbientInstance &a, C AmbientInstance &b)
{
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   if(Int c=ComparePtr(a.shader                       , b.shader                       ))return c;
   if(Int c=ComparePtr(a.material                     , b.material                     ))return c;
#else
   if(Int c=ComparePtr(a.variation->shader[RM_AMBIENT], b.variation->shader[RM_AMBIENT]))return c;
   if(Int c=ComparePtr(a.variation->material()        , b.variation->material()        ))return c;
#endif
   if(Int c=ComparePtr(a.mesh                         , b.mesh                         ))return c;
   return 0;
}
static Int Compare(C SkeletonAmbientInstance &a, C SkeletonAmbientInstance &b)
{
   if(Int c=ComparePtr(a.anim_skel                    , b.anim_skel                    ))return c;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   if(Int c=ComparePtr(a.shader                       , b.shader                       ))return c;
   if(Int c=ComparePtr(a.material                     , b.material                     ))return c;
#else
   if(Int c=ComparePtr(a.variation->shader[RM_AMBIENT], b.variation->shader[RM_AMBIENT]))return c;
   if(Int c=ComparePtr(a.variation->material()        , b.variation->material()        ))return c;
#endif
   if(Int c=ComparePtr(a.mesh                         , b.mesh                         ))return c;
   return 0;
}
void SortAmbientInstances()
{
#if SUPPORT_MATERIAL_AMBIENT
           AmbientInstances.sort(Compare);
   SkeletonAmbientInstances.sort(Compare);
#endif
}
void DrawAmbientInstances()
{
#if SUPPORT_MATERIAL_AMBIENT
   SetViewOffset();
   BeginPrecomputedViewMatrix();
   SetMatrixCount();
   DisableSkinning();
   FREPA(AmbientInstances)
   {
      // TODO: this could be optimized to group in state changes (shader->material->mesh->instance) and allow instancing, this is already sorted, we would just need to detect changes between this and next instance
    C AmbientInstance     &instance = AmbientInstances[i];
    C MeshPart            &mesh     =*instance.mesh;
   #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
      Shader              &shader   =*instance.shader;
    C Material            &material =*instance.material;
   #else
    C MeshPart::Variation &variation= instance.Variation();
      Shader              &shader   =*variation.shader[RM_AMBIENT];
    C Material            &material = variation.getMaterial();
   #endif
      material.setAmbient(); D.cull(material.cull);
      SetViewMatrix        (instance.view_matrix         ); Sh.h_ViewMatrix->setChanged();
      SetShaderParamChanges(instance.shader_param_changes);
      shader.begin(); mesh.render.set().drawFull(); ShaderEnd();
      SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
   }

   EndPrecomputedViewMatrix();
   EnableSkinning();
   FREPA(SkeletonAmbientInstances)
   {
      // TODO: this could be optimized to group in state changes (shader->material->mesh->instance), this is already sorted, we would just need to detect changes between this and next instance
    C SkeletonAmbientInstance &instance = SkeletonAmbientInstances[i];
    C MeshPart                &mesh     =*instance.mesh;
   #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
      Shader                  &shader   =*instance.shader;
    C Material                &material =*instance.material;
   #else
    C MeshPart::Variation     &variation= instance.Variation();
      Shader                  &shader   =*variation.shader[RM_AMBIENT];
    C Material                &material = variation.getMaterial();
   #endif
      material.setAmbient(); D.cull(material.cull);
      instance.anim_skel->setMatrix();
      SetShaderParamChanges(instance.shader_param_changes);
      shader.begin(); mesh.render.set().draw(); ShaderEnd(); // for skeleton have to use 'draw' instead of 'drawFull'
      SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
   }
#endif

   FREPAO(AmbientObjects)->drawAmbient();
}
void ClearAmbientInstances()
{
           AmbientInstances.clear();
   SkeletonAmbientInstances.clear();
           AmbientObjects  .clear();
}
/******************************************************************************/
// SHADOW
/******************************************************************************/
static Int MaterialShadersNum, ShaderDrawsNum, ShaderMaterialsNum, ShaderMaterialMeshesNum, SkeletonShadersNum, SkeletonShaderMaterialsNum;
void PrepareShadowInstances()
{
   // remember current number of elements in these containers (they may already have data in Forward renderer)
   MaterialShadersNum             =MaterialShaders     .elms();
           ShaderDrawsNum         =ShaderDraws         .elms();
           ShaderMaterialsNum     =ShaderMaterials     .elms();
           ShaderMaterialMeshesNum=ShaderMaterialMeshes.elms();

   SkeletonShadersNum        =SkeletonShaders        .elms();
   SkeletonShaderMaterialsNum=SkeletonShaderMaterials.elms();
}
void DrawShadowInstances()
{
   // this doesn't require 'ViewOffset' and 'BeginPrecomputedViewMatrix' because shadows are drawn only 1 time, and not for each eye

   const Bool instancing=D.hwInstancing();

   // solid
   SetMatrixCount();
   DisableSkinning();
   for(Int i=ShaderDrawsNum; i<ShaderDraws.elms(); i++)
   {
      ShaderDraw &shader_draw=ShaderDraws[i]; shader_draw.unlink();
      Shader     &shader     =shader_draw.shader->getShader(false); shader.start();
   #if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
      Bool        reuse_default_material=ReuseDefaultMaterialForNonSkinnedShadowShader(&shader);
   #endif
      for(ShaderMaterial *shader_material=&ShaderMaterials[shader_draw.first_shader_material]; ; )
      {
         ShaderMaterialMesh  *shader_material_mesh=&ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
      #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
       C Material            &material            =*shader_material->material;
      #else
       C MeshPart::Variation *variation           =&shader_material_mesh->Variation(); // have to access 'variation' here, because we need it to access 'material' since we're not storing it in the 'ShaderMaterial'
       C Material            &material            = variation->getShadowMaterial(reuse_default_material);
      #endif
         material.unlinkShadow(); material.setShadow(); D.cull(material.cull); shader.commitTex();

         for(;;)
         {
          C MeshPart   &mesh  =*shader_material_mesh->mesh;
          C MeshRender &render= mesh.render.set();
          C Bool        instancing_mesh=(instancing && !(render.flag()&VTX_SKIN)); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
         #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            mesh.unlinkShadow();
         #else
            variation->unlinkShadow();
         #endif

            for(ShadowShaderMaterialMeshInstance *instance=&ShadowShaderMaterialMeshInstances[shader_material_mesh->first_instance]; ; )
            {
               ViewMatrix[0]        =instance->view_matrix          ; Sh.h_ViewMatrix->setChanged(); // this doesn't need 'ViewOffset'
               SetShaderParamChanges(instance->shader_param_changes);
               Int instances=1;
               if( instancing_mesh)for(; instance->next_instance>=0; )
               {
                  ShadowShaderMaterialMeshInstance &next=ShadowShaderMaterialMeshInstances[instance->next_instance];
                  if(next.shader_param_changes==instance->shader_param_changes)
                  {
                     instance=&next;
                     ViewMatrix[instances]=next.view_matrix; // this doesn't need 'ViewOffset'
                     if(++instances>=MAX_MATRIX_INSTANCE)break;
                  }else break;
               }
               SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull();

                                                        if(instance->next_instance<0)break;
               instance=&ShadowShaderMaterialMeshInstances[instance->next_instance];
            }

                                                    if(shader_material_mesh->next_shader_material_mesh<0)break;
            shader_material_mesh=&ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
         #if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            variation           =&shader_material_mesh->Variation();
         #endif
         }

         SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
         ShaderEnd(); // needs to be called after every 'commitTex'
                                       if(shader_material->next_shader_material<0)break;
         shader_material=&ShaderMaterials[shader_material->next_shader_material];
      }
   }

   // skeleton
   EnableSkinning();
   FREPA(SkeletonShadowInstances)
   {
          SkeletonInstance &skel=SkeletonShadowInstances[i]; skel.unlinkShadow(); skel.anim_skel->setMatrix();
      for(SkeletonShader   *skel_shader=&skel.skel_shader; ; )
      {
         Shader &shader=skel_shader->shader->getShader(false); shader.start();
         for(SkeletonShaderMaterial *skel_shader_material=&skel_shader->material; ; )
         {
          C Material &material=*skel_shader_material->material; material.setShadow(); D.cull(material.cull); shader.commitTex();
            Bool shader_params_changed=true;
            for(SkeletonShaderMaterialMeshInstance *instance=&SkeletonShadowShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance]; ; )
            {
                  shader_params_changed|= SetShaderParamChanges(instance->shader_param_changes);
               if(shader_params_changed){shader_params_changed=false; shader.commit();}
               instance->mesh->set().draw(); // for skeleton have to use 'draw' instead of 'drawFull'
                                                                if(instance->next_instance<0)break;
               instance=&SkeletonShadowShaderMaterialMeshInstances[instance->next_instance];
            }
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            ShaderEnd(); // needs to be called after every 'commitTex'
                                                       if(skel_shader_material->next_skeleton_shader_material<0)break;
            skel_shader_material=&SkeletonShaderMaterials[skel_shader_material->next_skeleton_shader_material];
         }
                                   if(skel_shader->next_skeleton_shader<0)break;
         skel_shader=&SkeletonShaders[skel_shader->next_skeleton_shader];
      }
   }

   // we can't just clear all containers, because in Forward renderer there's still data needed for solid shaders which are processed after shadows, so restore to what we had before drawing shadows
   MaterialShaders                    .setNum(MaterialShadersNum);
           ShaderDraws                .setNum(        ShaderDrawsNum);
           ShaderMaterials            .setNum(        ShaderMaterialsNum);
           ShaderMaterialMeshes       .setNum(        ShaderMaterialMeshesNum);
     ShadowShaderMaterialMeshInstances.clear ();

   SkeletonShaders                          .setNum(SkeletonShadersNum);
   SkeletonShaderMaterials                  .setNum(SkeletonShaderMaterialsNum);
   SkeletonShadowShaderMaterialMeshInstances.clear();
   SkeletonShadowInstances                  .clear();

   // physical
   if(ShadowClothInstances.elms())
   {
      SetOneMatrix   ();
      SetVtxNrmMulAdd(false);

      // cloth
      {
         SetDefaultVAO(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
         FREPA(ShadowClothInstances)
         {
            ClothInstance &ci=ShadowClothInstances[i];
            Shader   &shader  = ci.shader  ->asShader();
          C Material &material=*ci.material; material.setShadow(); D.cull(material.cull);
            shader.begin(); ci.cloth->_drawPhysical();
            ShaderEnd();
         }
         ShadowClothInstances.clear();
      }

      SetVtxNrmMulAdd(true);
   }
}
/******************************************************************************/
// BLEND
/******************************************************************************/
Int Compare(C BlendInstance &a, C BlendInstance &b)
{
   if(Int z=Compare   (a.z         , b.z         ))return z;
    // compare other values in case 'z' is the same (which means the same object draws many parts with the same matrix, which may cause flickering)
   if(Int c=ComparePtr(a.s.shader  , b.s.shader  ))return c;
   if(Int c=ComparePtr(a.s.material, b.s.material))return c;
   if(Int c=ComparePtr(a.s.mesh    , b.s.mesh    ))return c;
                                                   return 0;
}
void ClearBlendInstances()
{
           BlendInstances                  .clear();
   SkeletonBlendShaders                    .clear();
   SkeletonBlendShaderMaterials            .clear();
   SkeletonBlendShaderMaterialMeshInstances.clear();
}
void DrawBlendInstances()
{
   SetViewOffset();
   EyeCache ec;

   const ALPHA_MODE alpha     =(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_BLEND_FACTOR);
   const Bool       instancing=D.hwInstancing();
   D.depth(true );
   D.alpha(alpha);
   REPA(BlendInstances) // go from back to start
   {
      BlendInstance *object=&BlendInstances[i]; switch(object->type)
      {
         case BlendInstance::SOLID:
         {
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            // this doesn't use velocities
            Shader     &shader  =*object->s.shader  ; shader.start();
         solid_shader:
          C Material   &material=*object->s.material; material.setBlend(); D.cull(material.cull); D.depthWrite(material._depth_write); Renderer.needDepthTest(); shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
         solid_shader_material:
          C MeshRender &render  = object->s.mesh->render.set();
           _SetHighlight         (object->s.highlight);
            D.stencil            (object->s.stencil_mode);
            SetShaderParamChanges(object->s.shader_param_changes);
            SetViewMatrix        (object->s.view_matrix); Sh.h_ViewMatrix->setChanged();
            const Bool instancing_mesh=(instancing && !(render.flag()&VTX_SKIN)); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
            Int instances=1;
            for(; i; ) // if there's next one
            {
               BlendInstance &next=BlendInstances[i-1]; // grab next
               if(next.s.shader==&shader // same shader
             //&& next.type==BlendInstance::SOLID not needed since if the shader is the same, then type should be the same too, make debug assertion below just in case
               )
               {
                  DEBUG_ASSERT(next.type==BlendInstance::SOLID, "Blend Instance type");
                  i--; // we will process this instance
                  if(next.s.material==&material) // same material
                  {
                     if(&next.s.mesh->render==&render
                     && next.s.highlight==Highlight
                     && next.s.shader_param_changes==LastChanges
                     && next.s.stencil_mode==D._stencil)
                     {
                        if(instancing_mesh)
                        {
                           if(!InRange(instances, MAX_MATRIX_INSTANCE)) // there's no room for this instance
                           {
                              SetMatrixCount(instances); shader.commit(); render.drawInstanced(instances); // draw what we have
                              instances=0; Sh.h_ViewMatrix->setChanged(); // reset counter and mark as modified
                           }
                           SetViewMatrix(next.s.view_matrix, instances);
                           instances++;
                        }else
                        {
                           SetMatrixCount(); shader.commit(); render.drawFull(); // draw what we have
                           SetViewMatrix(next.s.view_matrix); Sh.h_ViewMatrix->setChanged();
                        }
                     }else // we have the same shader/material, but different mesh/params
                     {
                        SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull(); // draw what we have
                        object=&next; goto solid_shader_material;
                     }
                  }else // we have the same shader, but different material/mesh/params
                  {
                     SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull(); ShaderEnd(); // draw what we have, 'ShaderEnd' needs to be called after every 'commitTex'
                     SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                     object=&next; goto solid_shader;
                  }
               }else break;
            }
            SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull();
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            ShaderEnd(); // needs to be called after every 'commitTex'
         }break;

         case BlendInstance::SOLID_BLST:
         {
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            Shader     &shader  = object->s.shader->asBlendShader(); shader.start();
         solid_blst_shader:
          C Material   &material=*object->s.material; material.setBlend(); D.cull(material.cull); D.depthWrite(material._depth_write); Renderer.needDepthTest(); shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
         solid_blst_shader_material:
          C MeshRender &render  = object->s.mesh->render.set();
           _SetHighlight         (object->s.highlight);
            D.stencil            (object->s.stencil_mode);
            SetShaderParamChanges(object->s.shader_param_changes);
            SetViewMatrix        (object->s.view_matrix); Sh.h_ViewMatrix->setChanged();
            SetFastVel           (object->s.vel);
            SetFastAngVel        (object->s.ang_vel_shader);
            const Bool instancing_mesh=(instancing && !(render.flag()&VTX_SKIN)); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
            Int instances=1;
            for(; i; ) // if there's next one
            {
               BlendInstance &next=BlendInstances[i-1]; // grab next
               if(next.s.blst==object->s.blst // same shader
             //&& next.type==BlendInstance::SOLID_BLST not needed since if the shader is the same, then type should be the same too, make debug assertion below just in case
               )
               {
                  DEBUG_ASSERT(next.type==BlendInstance::SOLID_BLST, "Blend Instance type");
                  i--; // we will process this instance
                  if(next.s.material==&material) // same material
                  {
                     if(&next.s.mesh->render==&render
                  #if !PER_INSTANCE_VEL
                     && next.s.vel==object->s.vel
                  #endif
                  #if !PER_INSTANCE_ANG_VEL
                     && next.s.ang_vel_shader==object->s.ang_vel_shader
                  #endif
                     && next.s.highlight==Highlight
                     && next.s.shader_param_changes==LastChanges
                     && next.s.stencil_mode==D._stencil)
                     {
                        if(instancing_mesh)
                        {
                           if(!InRange(instances, MAX_MATRIX_INSTANCE)) // there's no room for this instance
                           {
                              SetMatrixCount(instances); shader.commit(); render.drawInstanced(instances); // draw what we have
                              instances=0; Sh.h_ViewMatrix->setChanged(); // reset counter and mark as modified
                           }
                           SetViewMatrix(next.s.view_matrix, instances);
                        #if PER_INSTANCE_VEL
                           SetFastVel(instances, next.s.vel);
                        #endif
                        #if PER_INSTANCE_ANG_VEL
                           not supported
                        #endif
                           instances++;
                        }else
                        {
                           SetMatrixCount(); shader.commit(); render.drawFull(); // draw what we have
                           SetViewMatrix (next.s.view_matrix); Sh.h_ViewMatrix->setChanged();
                           SetFastVel    (next.s.vel);
                           SetFastAngVel (next.s.ang_vel_shader);
                        }
                     }else // we have the same shader/material, but different mesh/params
                     {
                        SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull(); // draw what we have
                        object=&next; goto solid_blst_shader_material;
                     }
                  }else // we have the same shader, but different material/mesh/params
                  {
                     SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull(); ShaderEnd(); // draw what we have, 'ShaderEnd' needs to be called after every 'commitTex'
                     SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                     object=&next; goto solid_blst_shader;
                  }
               }else break;
            }
            SetMatrixCount(instances); shader.commit(); if(instances>1)render.drawInstanced(instances);else render.drawFull();
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            ShaderEnd(); // needs to be called after every 'commitTex'
         }break;

         case BlendInstance::SOLID_FUR:
         {
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            Flt scale=object->s.view_matrix.x.length()/D._view_active.fov_tan.y; if(FovPerspective(D.viewFovMode()))scale/=object->s.view_matrix.pos.z;
            D.stencil            (STENCIL_NONE);
            SetMatrixCount       ();
            SetFurVelCount       ();
            SetViewMatrix        (object->s.view_matrix); Sh.h_ViewMatrix->setChanged();
            SetVelFur            (object->s.view_matrix, object->s.vel);
           _SetHighlight         (object->s.highlight);
            SetShaderParamChanges(object->s.shader_param_changes);

            Shader     &shader  =*object->s.shader;
          C Material   &material=*object->s.material; material.setBlend(); D.cull(material.cull); D.depthWrite(material._depth_write); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
          C MeshRender &render  = object->s.mesh->render.set();
            shader.begin(); DrawFur(render, shader, scale);
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            ShaderEnd();
         }break;

         case BlendInstance::SKELETON:
         {
            ec.EndPrecomputedViewMatrix();
            EnableSkinning();
                SkeletonBlendInstance &skel=object->skeleton; skel.unlinkBlend(); skel.anim_skel->setMatrix(); Bool fur_set=false; Flt fur_scale;
            for(SkeletonBlendShader   *skel_shader=&skel.skel_shader; ; )
            {
               if(skel_shader->type==BlendInstance::SOLID_FUR && !fur_set)
               {
                  fur_set=true; skel.anim_skel->setFurVel(); // !! needs to be called before 'shader.start' !!
                  fur_scale=skel.anim_skel->matrix().x.length()/D._view_active.fov_tan.y; if(FovPerspective(D.viewFovMode()))fur_scale/=DistPointPlane(skel.anim_skel->pos(), CamMatrix.pos, CamMatrix.z);
               }
               Shader &shader=skel_shader->shader->getBlendShader(skel_shader->type==BlendInstance::SOLID_BLST); shader.start();
               for(SkeletonShaderMaterial *skel_shader_material=&skel_shader->material; ; )
               {
                C Material &material=*skel_shader_material->material; material.setBlend(); D.cull(material.cull); D.depthWrite(material._depth_write); Renderer.needDepthTest(); shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
                  Bool shader_params_changed=true;
                  for(SkeletonBlendShaderMaterialMeshInstance *instance=&SkeletonBlendShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance]; ; )
                  {
                     D.stencil(instance->stencil_mode);
                        shader_params_changed|=_SetHighlight         (instance->highlight);
                        shader_params_changed|= SetShaderParamChanges(instance->shader_param_changes);
                     if(shader_params_changed){shader_params_changed=false; shader.commit();}
                                                                    instance->mesh->set();
                     if(skel_shader->type!=BlendInstance::SOLID_FUR)instance->mesh->draw(); // for skeleton have to use 'draw' instead of 'drawFull'
                     else                                  DrawFur(*instance->mesh, shader, fur_scale);
                                                                     if(instance->next_instance<0)break;
                     instance=&SkeletonBlendShaderMaterialMeshInstances[instance->next_instance];
                  }
                  SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                  ShaderEnd(); // needs to be called after every 'commitTex'
                                                                  if(skel_shader_material->next_skeleton_shader_material<0)break;
                  skel_shader_material=&SkeletonBlendShaderMaterials[skel_shader_material->next_skeleton_shader_material];
               }
                                              if(skel_shader->next_skeleton_shader<0)break;
               skel_shader=&SkeletonBlendShaders[skel_shader->next_skeleton_shader];
            }
         }break;

         case BlendInstance::BLEND_OBJ : ec.EndPrecomputedViewMatrix(); D.stencil(STENCIL_NONE); _SetHighlight(TRANSPARENT); object->blend_obj ->      drawBlend(); D.alpha(alpha); D.depth(true); break;
         case BlendInstance:: GAME_OBJ : ec.EndPrecomputedViewMatrix(); D.stencil(STENCIL_NONE); _SetHighlight(TRANSPARENT); object-> game_obj ->      drawBlend(); D.alpha(alpha); D.depth(true); break;
         case BlendInstance:: GAME_AREA: ec.EndPrecomputedViewMatrix(); D.stencil(STENCIL_NONE); _SetHighlight(TRANSPARENT); object-> game_area->customDrawBlend(); D.alpha(alpha); D.depth(true); break;
      }
   }

   // finish
   ec.EndPrecomputedViewMatrix();
   D.depthWrite(true);
   D.stencil   (STENCIL_NONE);
  _SetHighlight(TRANSPARENT);
}
/******************************************************************************/
// PALETTE
/******************************************************************************/
void DrawPaletteObjects()
{
   FREPAO(PaletteObjects)->      drawPalette();
   FREPAO(PaletteAreas  )->customDrawPalette();
}
void DrawPalette1Objects()
{
   FREPAO(Palette1Objects)->      drawPalette1();
   FREPAO(Palette1Areas  )->customDrawPalette1();
}
/******************************************************************************/
// OVERLAY
/******************************************************************************/
void DrawOverlayObjects()
{
   FREPAO(OverlayObjects)->drawOverlay();
}
/******************************************************************************/
// OUTLINE
/******************************************************************************/
void DrawOutlineObjects()
{
   FREPAO(OutlineObjects)->drawOutline();
}
/******************************************************************************/
// BEHIND
/******************************************************************************/
void DrawBehindObjects()
{
   FREPAO(BehindObjects)->drawBehind();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ShutInstances()
{
         MaterialShaders                    .del();
                 ShaderDraws                .del();
    MultiMaterialShaderDraws                .del();
                 ShaderMaterials            .del();
                 ShaderMaterialMeshes       .del();
            SolidShaderMaterialMeshInstances.del();
           ShadowShaderMaterialMeshInstances.del();
                            AmbientInstances.del();
   SkeletonShaders                          .del();
   SkeletonBlendShaders                     .del();
   SkeletonShaderMaterials                  .del();
   SkeletonBlendShaderMaterials             .del();
   SkeletonShadowShaderMaterialMeshInstances.del();
   SkeletonSolidShaderMaterialMeshInstances .del();
   SkeletonBlendShaderMaterialMeshInstances .del();
   SkeletonSolidInstances                   .del();
   SkeletonShadowInstances                  .del();
   SkeletonAmbientInstances                 .del();

   BlendInstances.del();

   SolidClothInstances.del(); ShadowClothInstances.del();

   PaletteObjects.del(); Palette1Objects.del(); OverlayObjects.del(); SolidObjects.del(); AmbientObjects.del(); OutlineObjects.del(); BehindObjects.del();
   PaletteAreas  .del(); Palette1Areas  .del();
}
void InitInstances()
{
       //MaterialShaders                    .reserve(16); no need because this is used only in special cases where materials use more than 1 shader
                 ShaderDraws                .reserve(16);
    MultiMaterialShaderDraws                .reserve( 4);
                 ShaderMaterials            .reserve(16);
                 ShaderMaterialMeshes       .reserve(16);
            SolidShaderMaterialMeshInstances.reserve(16);
           ShadowShaderMaterialMeshInstances.reserve(16);
   SkeletonShaders                          .reserve(16);
   SkeletonBlendShaders                     .reserve(16);
   SkeletonShaderMaterials                  .reserve(16);
   SkeletonBlendShaderMaterials             .reserve(16);
   SkeletonShadowShaderMaterialMeshInstances.reserve(16);
   SkeletonSolidShaderMaterialMeshInstances .reserve(16);
   SkeletonBlendShaderMaterialMeshInstances .reserve(16);
   SkeletonSolidInstances                   .reserve(16);
   SkeletonShadowInstances                  .reserve(16);
   BlendInstances                           .reserve(16);
}
void ClearInstances()
{
   Renderer._fur_is=false;

   // unlink default materials because shadow draws may reuse them
   MaterialDefault      .unlink();
   MaterialDefaultNoCull.unlink();

#if SUPPORT_EARLY_Z
   ClearEarlyZInstances();
#endif
                                                           MaterialShaders                    .clear();
   REPAO(             ShaderDraws         ).unlink();              ShaderDraws                .clear();
   REPAO(MultiMaterialShaderDraws         ).unlink(); MultiMaterialShaderDraws                .clear();
   REPAO(             ShaderMaterials     ).unlink();              ShaderMaterials            .clear(); // !! unlink before 'ShaderMaterialMeshes' !! because 'ShaderMaterial.unlink' makes use of 'ShaderMaterialMeshes'
   REPAO(             ShaderMaterialMeshes).unlink();              ShaderMaterialMeshes       .clear(); // !! unlink after  'ShaderMaterials'      !!
                                                              SolidShaderMaterialMeshInstances.clear();
                                                             ShadowShaderMaterialMeshInstances.clear();
                                                                              AmbientInstances.clear();

                                 SkeletonShaders                          .clear();
                                 SkeletonBlendShaders                     .clear();
                                 SkeletonShaderMaterials                  .clear();
                                 SkeletonBlendShaderMaterials             .clear();
                                 SkeletonShadowShaderMaterialMeshInstances.clear();
                                 SkeletonSolidShaderMaterialMeshInstances .clear();
                                 SkeletonBlendShaderMaterialMeshInstances .clear();
   REPAO(SkeletonSolidInstances ).unlinkSolid (); SkeletonSolidInstances  .clear();
   REPAO(SkeletonShadowInstances).unlinkShadow(); SkeletonShadowInstances .clear();
                                                  SkeletonAmbientInstances.clear();

   REPAO(BlendInstances).unlink(); BlendInstances.clear();

   SolidClothInstances.clear(); ShadowClothInstances.clear();

   PaletteObjects.clear(); Palette1Objects.clear(); OverlayObjects.clear(); SolidObjects.clear(); AmbientObjects.clear(); OutlineObjects.clear(); BehindObjects.clear();
   PaletteAreas  .clear(); Palette1Areas  .clear();

#if DEBUG && 0
   #pragma message("!! Warning: Use this only for debugging !!")
   Materials.  lock(); REPA(Materials)DYNAMIC_ASSERT(Materials.lockedData(i).canBeRemoved(), "Material still has some instance references after rendering finished");
   Materials.unlock();
#endif
}
/******************************************************************************/
}
/******************************************************************************/
