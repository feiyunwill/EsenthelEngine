/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define OPAQUE_FRAC 0.8f
// Flt alpha=1-Sat((Abs(pos.z)-opaque_frac)/(1-opaque_frac));
// Flt alpha=Sat(Abs(pos.z)/-(1-opaque_frac) + (opaque_frac/(1-opaque_frac)+1));
// Flt alpha=Sat(Abs(pos.z)*(1/(opaque_frac-1)) + (opaque_frac/(1-opaque_frac)+1));
inline Flt OpaqueFracMul(Flt frac=OPAQUE_FRAC) {return 1.0f/(frac-1);}
inline Flt OpaqueFracAdd(Flt frac=OPAQUE_FRAC) {return frac/(1-frac)+1;}
/******************************************************************************/
void Decal::zero()
{
   terrain_only=false;
   color=1;
   matrix.identity();
   Zero(_shader);
}
void Decal::del()
{
  _material.clear();
   zero();
}
Decal::Decal() {zero();}
void Decal::setShader()
{
   REPD(f, 2)
   REPD(p, 2)
   {
      if(_material && _material->base_0)
      {
            _shader[f][p]=Sh.get(S+"Decal"+(f ? 'F' : '\0')+(p ? 'P' : _material->base_1 ? 'N' : '\0'));
      }else _shader[f][p]=null;
   }
}
Decal& Decal::material(C MaterialPtr &material)
{
   if(T._material!=material)
   {
      T._material=material;
      setShader();
   }
   return T;
}
Decal& Decal::setMatrix(C Matrix &object_world_matrix, C Matrix &decal_world_matrix)
{
   object_world_matrix.inverse(matrix);
    decal_world_matrix.mul    (matrix, matrix); // 'matrix'=local matrix
   return T;
}
/******************************************************************************/
void Decal::drawStatic(Flt alpha)C
{
   if(_shader[0][0] && Renderer.canReadDepth())
   {
      BallM ball((matrix.x+matrix.y+matrix.z).length(), matrix.pos);
      if(Frustum(ball))
      {
         Bool inside=Cuts(CamMatrix.pos, ball.extend(Frustum.view_quad_max_dist));
         if(Shader *shader=T._shader[inside][Renderer._palette_mode])
         {
            SetOneMatrix(matrix);

           _material->setBlend();
            Sh.h_DecalParams->set(Vec(OpaqueFracMul(), OpaqueFracAdd(), alpha));
            Sh.h_Color[0]   ->set(color);

            if(terrain_only)D.stencil(STENCIL_TERRAIN_TEST);
          //Bool cwrt; Byte cwrt_old; if(cwrt=(Renderer.cur[1] && tech!=Sh.h_DecalN)){cwrt_old=D._cwrt[1]; D.colWrite(0, 1);} it looks like if the shader doesn't write to RT then it's not applied

            Renderer.needDepthRead();
            D.alpha(Renderer._palette_mode ? ALPHA_ADD : ALPHA_BLEND_FACTOR);
            if(inside)
            {
               shader->draw(_material->base_0());
            }else
            {
               shader->begin(); D.cull(true); D.depth(true); D.depthWrite(false); MshrBox.set().drawFull(); ShaderEnd();
            }

          //if(cwrt)D.colWrite(cwrt_old, 1);
            if(terrain_only)D.stencil(STENCIL_NONE);
         }
      }
   }
}
void Decal::drawAnimated(C Matrix &object_world_matrix, Flt alpha)C
{
   if(_shader[0][0] && Renderer.canReadDepth())
   {
      Matrix m; matrix.mul(object_world_matrix, m);
      BallM  ball((m.x+m.y+m.z).length(), m.pos);
      if(Frustum(ball))
      {
         Bool inside=Cuts(CamMatrix.pos, ball.extend(Frustum.view_quad_max_dist));
         if(Shader *shader=T._shader[inside][Renderer._palette_mode])
         {
            SetOneMatrix(m);

           _material->setBlend();
            Sh.h_DecalParams->set(Vec(OpaqueFracMul(), OpaqueFracAdd(), alpha));
            Sh.h_Color[0]   ->set(color);

            if(terrain_only)D.stencil(STENCIL_TERRAIN_TEST);
          //Bool cwrt; Byte cwrt_old; if(cwrt=(Renderer.cur[1] && tech!=Sh.h_DecalN)){cwrt_old=D._cwrt[1]; D.colWrite(0, 1);} it looks like if the shader doesn't write to RT then it's not applied

            Renderer.needDepthRead();
            D.alpha(Renderer._palette_mode ? ALPHA_ADD : ALPHA_BLEND_FACTOR);
            if(inside)
            {
               shader->draw(_material->base_0());
            }else
            {
               shader->begin(); D.cull(true); D.depth(true); D.depthWrite(false); MshrBox.set().drawFull(); ShaderEnd();
            }

          //if(cwrt)D.colWrite(cwrt_old, 1);
            if(terrain_only)D.stencil(STENCIL_NONE);
         }
      }
   }
}
/******************************************************************************/
Bool Decal::save(File &f)C
{
   f.cmpUIntV(0); // version
   f<<terrain_only<<color<<matrix;
   f.putAsset(_material.id());
   return f.ok();
}
Bool Decal::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>terrain_only>>color>>matrix;
        _material=f.getAssetID();
         setShader();
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
