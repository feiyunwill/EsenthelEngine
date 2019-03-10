/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
SunClass    Sun;
Bool        AstrosDraw=true;
Memc<Astro> Astros;
/******************************************************************************/
Astro::Astro()
{
   draw=true;

   blend=true;
   glow=0;
   size=0.15f;
   pos.set(-SQRT3_3, SQRT3_3, -SQRT3_3);
   image_color=WHITE;

   light_color    =0;
   light_vol      =0; // 0.2f
   light_vol_exp  =1.0f;
   light_vol_steam=0.5f;
}
/******************************************************************************/
void Astro::light()
{
   if(is())
   {
      LightDir(-pos, light_color, light_vol, light_vol_exp, light_vol_steam).add(true, this);
   }
}
void Astro::Draw()
{
   if(is() && Frustum(BallM(D.viewRange()*size*SQRT2, CamMatrix.pos+pos*D.viewRange())))
   {
      // TODO: apply per-pixel softing based on depth buffer, exactly like particle softing (draw closer to camera, but scale XY size, along CamMatrix.xy) and modify pixel shader
      Renderer._has_glow|=(glow!=0);
      D .alphaFactor(VecB4(0, 0, 0, glow));
      D .alpha      (blend ? ALPHA_BLEND_FACTOR : ALPHA_ADD_FACTOR);
      VI.image      (image());
      VI.setType    (VI_3D_TEX_COL, VI_STRIP);
      if(Vtx3DTexCol *v=(Vtx3DTexCol*)VI.addVtx(4))
      {
         Matrix3 m; m.setRotation(Vec(0, 1, 0), pos); // use top as original position, because this makes the most realistic UV rotation when the object rotates around the sky
       C Vec &right=(m.x*=size),
             &up   =(m.z*=size);

         v[0].pos=pos-right+up;
         v[1].pos=pos+right+up;
         v[2].pos=pos-right-up;
         v[3].pos=pos+right-up;
         Flt depth=0; REP(4)MAX(depth, Dot(v[i].pos, CamMatrix.z)); // project onto viewport far plane, so it's always at the back, need to find max depth of all vertexes, because if vertexes would be projected individually, then UV would be stretched unrealistically
         Flt range=D.viewRange()-0.01f, // make it smaller to avoid being clipped by viewport far plane
             scale=range/depth;
         REP(4)v[i].pos*=scale;
         v[0].color=v[1].color=v[2].color=v[3].color=image_color;
         v[0].tex.set(             0,              0);
         v[1].tex.set(image->_part.x,              0);
         v[2].tex.set(             0, image->_part.y);
         v[3].tex.set(image->_part.x, image->_part.y);
      }
      VI.end();

      MaterialClear(); // because alpha factor
   }
}
/******************************************************************************/
SunClass::SunClass()
{
   glow=128;
   light_color=0.7f;

   highlight_front=0.10f;
   highlight_back =0.07f;

   rays_color   =0.05f; // don't set more than 0.1f because it will require jittering
   rays_jitter  =-1   ; // auto
   rays_mode    =SUN_RAYS_HIGH;
  _rays_res     =FltToByteScale(0.25f);
  _rays_mask_res=FltToByteScale(1.0f ); // we can use full res because it makes a small performance difference
}
Bool SunClass::wantRays()C {return is() && rays_mode && rays_color.max()>EPS_COL;}

Flt       SunClass::raysRes(         )C {return   ByteScaleToFlt(_rays_res);}
SunClass& SunClass::raysRes(Flt scale)  {Byte res=FltToByteScale(scale); if(res!=_rays_res){_rays_res=res; Renderer.rtClean();} return T;}

Flt       SunClass::raysMaskRes(         )C {return   ByteScaleToFlt(_rays_mask_res);}
SunClass& SunClass::raysMaskRes(Flt scale)  {Byte res=FltToByteScale(scale); if(res!=_rays_mask_res){_rays_mask_res=res; Renderer.rtClean();} return T;}

#pragma pack(push, 4)
struct GpuSun
{
   Vec2 pos2;
   Vec  pos, color;
};
#pragma pack(pop)

INLINE Shader* GetSunRaysMask(Bool mask             ) {Shader* &s=Sh.h_SunRaysMask[mask]        ; if(SLOW_SHADER_LOAD && !s)s=Sh.getSunRaysMask(mask        ); return s;}
INLINE Shader* GetSunRays    (Bool high, Bool jitter) {Shader* &s=Sh.h_SunRays    [high][jitter]; if(SLOW_SHADER_LOAD && !s)s=Sh.getSunRays    (high, jitter); return s;}

void SunClass::drawRays(Image *coverage, Vec &color)
{
   GpuSun sun;
   sun.pos =pos*CamMatrixInv.orn();
   sun.pos2=D.screenToUV(_pos2);

   Flt alpha=Sat(sun.pos.z/0.18f); // if sun is close to camera (or behind it) then decrease opacity
   if(Fog.draw && Fog.affect_sky)alpha*=VisibleOpacity(Fog.density, D.viewRange()); // rays are in ALPHA_ADD mode, so we need to downscale the color
   color=rays_color*alpha;

   if(Renderer._cur[0]!=Renderer._col()) // if rendering to a separate RT, then render with full color, so later when upscaling, we can use dithering with higher precision, during dithering the color will be set to the right value
   {
      sun.color=1;
   }else // if rendering to the main target, then use final color
   {
      sun.color=color;
   }

   REPS(Renderer._eye, Renderer._eye_num)
   {
      Rect *rect=Renderer.setEyeParams(), ext_rect; if(!D._view_main.full){ext_rect=*rect; rect=&ext_rect.extend(Renderer.pixelToScreenSize(1/*+(rays_soft?SHADER_BLUR_RANGE:0)*/));} // always extend by 1 because we may apply this RT to the final RT usin linear filtering
      if(Renderer._stereo)
      {
         sun.pos    =pos; sun.pos.divNormalized(ActiveCam.matrix.orn());
         sun.pos2   =D.screenToUV(_pos2);
         sun.pos2.x+=ProjMatrixEyeOffset[Renderer._eye]*0.5f;
         sun.pos2.x*=0.5f;
         if(Renderer._eye)sun.pos2.x+=0.5f;
      }
      Sh.h_Sun->set(sun);
      Bool jitter=((rays_jitter<0) ? rays_color.max()>0.1f+EPS_COL : rays_jitter!=0); // for auto, enable jittering only if rays have a high brightness
      switch(_actual_rays_mode)
      {
         case SUN_RAYS_HIGH: GetSunRays(true , jitter)->draw(coverage       , rect); break;
         default           : GetSunRays(false, jitter)->draw(Renderer._ds_1s, rect); break;
      }
   }
}
/******************************************************************************/
void AstroPrepare()
{
   Sun._actual_rays_mode=SUN_RAYS_OFF;

   if(AstrosDraw)
   {
      if(Sun.wantRays()
      && Renderer.canReadDepth() && !Renderer.mirror()
      && PosToFullScreen(CamMatrix.pos+Sun.pos*D.viewRange(), Sun._pos2) && FovPerspective(D.viewFovMode())
      && !(Fog.draw && Fog.affect_sky && VisibleOpacity(Fog.density, D.viewRange())<=EPS_COL) // if fog is too dense then don't draw sun rays
      )
      {
         if(Sun.rays_mode==SUN_RAYS_HIGH && D._mrt_post_process && D._max_rt>=2)
         {
            Renderer._sky_coverage.get(ImageRTDesc(Renderer._col->w(), Renderer._col->h(), IMAGERT_ONE, Renderer._col->samples()));
            Renderer._sky_coverage->clearViewport(1); // set full initially, which we will decrease with clouds and finally depth tests
               Sun._actual_rays_mode=SUN_RAYS_HIGH;
         }else Sun._actual_rays_mode=SUN_RAYS_LOW;
      }
      Sun.light(); FREPAO(Astros).light();
   }
}
void AstroDraw()
{
   if(AstrosDraw)
   {
      Renderer.set(Renderer._col(), Renderer._ds(), true); // use DS for depth tests
      SetOneMatrix(MatrixM(CamMatrix.pos)); // normally we have to set matrixes after 'setEyeViewport', however since matrixes are always relative to the camera, and here we set exactly at the camera position, so the matrix will be the same for both eyes
      Sh.h_SkyFracMulAdd->set(Vec2(0, 1)); // astronomical objects are drawn as billboards which make use of sky fraction, so be sure to disable it before drawing
      D.depthWrite(false); D.depthFunc(FUNC_LESS_EQUAL); REPS(Renderer._eye, Renderer._eye_num){Renderer.setEyeViewport(); Sun.Draw(); FREPAO(Astros).Draw();}
      D.depthWrite(true ); D.depthFunc(FUNC_LESS      );
   }
}
Bool AstroDrawRays()
{
   if(Sun._actual_rays_mode)
   {
      Renderer.downSample(); // we're modifying existing RT, so downSample if needed

      if(Renderer._sky_coverage) // apply mask based on depth tests, have to do this at the end, once we have depth buffer set (including from blend phase), no need to check for 'canReadDepth' because that was already checked when setting '_sky_coverage'
      {
         const VecI2 res=ByteScaleRes(Renderer.fx(), Sun._rays_mask_res);
         if(res!=Renderer._sky_coverage()->size()) // draw to smaller RT, and combine sky coverage with depth tests
         {
            D.alpha(ALPHA_NONE);
            ImageRTPtr temp; temp.get(ImageRTDesc(res.x, res.y, IMAGERT_ONE));
            Renderer.set(temp(), null, true);
            GetSunRaysMask(true)->draw(Renderer._sky_coverage);
            Swap(temp, Renderer._sky_coverage);
         }else // apply depth tests to existing sky coverage
         {
            Renderer.set(Renderer._sky_coverage(), Renderer._ds(), true, NEED_DEPTH_READ); // '_ds' needed for 'depth2D' optimization
            D.alpha     (ALPHA_MUL);
            D.depth2DOn (); GetSunRaysMask(false)->draw();
            D.depth2DOff();
         }

         if(Renderer.stage==RS_SKY_COVERAGE && Renderer.set(Renderer._sky_coverage))return true;
      }

      const VecI2 res=ByteScaleRes(Renderer.fx(), Sun._rays_res);
      ImageRTPtr rt0; if(res!=Renderer._col->size())rt0.get(ImageRTDesc(res.x, res.y, IMAGERT_ONE));

      Renderer.set(rt0 ? rt0() : Renderer._col(), null, rt0==null); // set viewport only when drawing to 'Renderer._col'
      SetOneMatrix();
      D.alpha(rt0 ? ALPHA_NONE : ALPHA_ADD); // if drawing to rt0 then set, if drawing to Renderer._col then add

      Vec color; Sun.drawRays(Renderer._sky_coverage(), color);
      if(rt0)
      {
       /*if(Sun.rays_soft && shift>=2)
         {
            if(!Sh.h_BlurX_X)
            {
               Sh.h_BlurX_X=Sh.get("BlurX_X");
               Sh.h_BlurX_W=Sh.get("BlurX_W");
               Sh.h_BlurY_X=Sh.get("BlurY_X");
               Sh.h_BlurY_W=Sh.get("BlurY_W");
            }
            D.alpha(ALPHA_NONE);
            rt1.get(ImageRTDesc(Renderer.fxW()>>shift, Renderer.fxH()>>shift, IMAGERT_ONE));
            Renderer.set(rt1(), null, false); Sh.GPU_API(h_BlurX_W, h_BlurX_X, h_BlurX_X)->draw(rt0(), &D.viewRect()); // DX9 uses A8 while others use R8 RT
            Renderer.set(rt0(), null, false); Sh.GPU_API(h_BlurY_W, h_BlurY_X, h_BlurY_X)->draw(rt1(), &D.viewRect()); // DX9 uses A8 while others use R8 RT
         }*/
         Renderer.set(Renderer._col(), null, true);
         D.alpha(ALPHA_ADD);
         Sh.h_Color[0]->set(Vec4(color, 0)); // we've rendered the sun with full intensity, so now we need to make it darker
         Sh.h_Color[1]->set(Vec4Zero      );
         Bool dither=(D.dither() && !Renderer._col->highPrecision()); // don't do dithering for high precision RT
         Shader *shader;
       //if(Sun.rays_soft && shift==1)shader=Sh.h_SunRaysSoft;else
         if(dither                   )shader=Sh.GPU_API(h_DrawTexWCD, h_DrawTexXCD, h_DrawTexXCD);else // DX9 uses A8 while others use R8 RT
                                      shader=Sh.GPU_API(h_DrawTexWC , h_DrawTexXC , h_DrawTexXC );     // DX9 uses A8 while others use R8 RT
         REPS(Renderer._eye, Renderer._eye_num)shader->draw(rt0, Renderer._stereo ? &D._view_eye_rect[Renderer._eye] : &D.viewRect());
      }
      Renderer._sky_coverage.clear();
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
