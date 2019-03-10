/******************************************************************************/
#include "stdafx.h"
#define TEST_LIGHT_RECT (DEBUG && 0)
#define FLAT_SHADOW_MAP 1 // TODO try implementing DX11 Cube Shadow Maps?
#define LOCAL_SHADOW_MAP_FROM (1.0f/64) // 0.015625m, 1.5 cm

namespace EE{
/******************************************************************************

   Hardware Shadow Map RT is created with resolution:
      x=D.shadowMapSizeActual()*2
      y=D.shadowMapSizeActual()*3

   Like this (2x3):
      SS
      SS
      SS

/******************************************************************************/
static Matrix           ShdMatrix    [2]; //           [Eye]
static Matrix4          ShdMatrix4[6][2]; // [MapIndex][Eye]
static Matrix4          HsmMatrix, HsmMatrixCone;
static Memc<FloatIndex> LightImportance;
       Memc<Light>      Lights;
            Light       CurrentLight;
static Bool             CurrentLightOn  [2];
static Rect             CurrentLightRect[2];
/******************************************************************************/
static inline Int      HsmX        (DIR_ENUM dir) {return dir& 1;}
static inline Int      HsmY        (DIR_ENUM dir) {return dir>>1;}
static inline Matrix4& HsmMatrixSet(DIR_ENUM dir)
{
   Int x=HsmX(dir),
       y=HsmY(dir);
   HsmMatrix.pos.x=(x ? 0.75f : 0.25f)      ;
   HsmMatrix.pos.y=Avg(0, 1.0f/3)+(1.0f/3)*y;
   return HsmMatrix;
}
static inline void MatrixFovScaleX(Matrix &matrix, Flt scale) {matrix.x.x/=scale; matrix.y.x/=scale; matrix.z.x/=scale; matrix.pos.x/=scale;}
static inline void MatrixFovScaleY(Matrix &matrix, Flt scale) {matrix.x.y/=scale; matrix.y.y/=scale; matrix.z.y/=scale; matrix.pos.y/=scale;}
static inline void MatrixFovScale (Matrix &matrix, Flt scale) {MatrixFovScaleX(matrix, scale); MatrixFovScaleY(matrix, scale);}
/******************************************************************************/
static inline void SetShadowOpacity(Flt opacity) {Sh.h_ShdOpacity->set(Vec2(opacity, 1-opacity));} // shd=Lerp(1, shd, shadow_opacity) -> shd=1*(1-shadow_opacity) + shd*(shadow_opacity) -> shd=shd*shadow_opacity + 1-shadow_opacity

static inline void SetShdMatrix()
{
         Sh.h_ShdMatrix    ->set(ShdMatrix    [Renderer._eye]);
   REP(6)Sh.h_ShdMatrix4[i]->set(ShdMatrix4[i][Renderer._eye]);
}
/******************************************************************************/
INLINE Shader* GetShdDir (Int map_num, Bool clouds, Bool multi_sample) {Shader* &s=Sh.h_ShdDir[map_num-1][clouds][multi_sample]; if(SLOW_SHADER_LOAD && !s)s=Sh.getShdDir (map_num, clouds, multi_sample); return s;}
INLINE Shader* GetShdPnt (                          Bool multi_sample) {Shader* &s=Sh.h_ShdPnt                   [multi_sample]; if(SLOW_SHADER_LOAD && !s)s=Sh.getShdPnt (                 multi_sample); return s;}
INLINE Shader* GetShdCone(                          Bool multi_sample) {Shader* &s=Sh.h_ShdCone                  [multi_sample]; if(SLOW_SHADER_LOAD && !s)s=Sh.getShdCone(                 multi_sample); return s;}

INLINE Shader* GetLightDir (Bool shadow,             Bool multi_sample, Bool quality) {Shader* &s=Sh.h_LightDir [shadow]       [multi_sample][quality]; if(SLOW_SHADER_LOAD && !s)s=Sh.getLightDir (shadow,        multi_sample, quality); return s;}
INLINE Shader* GetLightPnt (Bool shadow,             Bool multi_sample, Bool quality) {Shader* &s=Sh.h_LightPnt [shadow]       [multi_sample][quality]; if(SLOW_SHADER_LOAD && !s)s=Sh.getLightPnt (shadow,        multi_sample, quality); return s;}
INLINE Shader* GetLightSqr (Bool shadow,             Bool multi_sample, Bool quality) {Shader* &s=Sh.h_LightSqr [shadow]       [multi_sample][quality]; if(SLOW_SHADER_LOAD && !s)s=Sh.getLightSqr (shadow,        multi_sample, quality); return s;}
INLINE Shader* GetLightCone(Bool shadow, Bool image, Bool multi_sample, Bool quality) {Shader* &s=Sh.h_LightCone[shadow][image][multi_sample][quality]; if(SLOW_SHADER_LOAD && !s)s=Sh.getLightCone(shadow, image, multi_sample, quality); return s;}
/******************************************************************************/
static Flt ShadowStep(Int i, Int num) // 0..1
{
   if(i==0  )return 0;
   if(i==num)return 1;
   if(D.shadow_step)return D.shadow_step(i, num);
   Flt s=Flt(i)/num;
   return Pow(s, D.shadowMapSplit().x + D.shadowMapSplit().y*s); // Pow(s, 2) == s*s produces results nearly identical to "Lerp(1/((1-s)*D.viewRange()), s, s);"
}
/******************************************************************************/
static inline Flt GetBias() {return (D.shadowJitter() ? SQRT2 : SQRT2_2)/D.shadowMapSizeActual();}
/******************************************************************************/
static void ApplyViewSpaceBias(Flt &mp_z_z)
{
   if(FovPerspective(D.viewFovMode())) // needed only for perspective because it can produce big errors
   {
      mp_z_z=ProjMatrix.z.z;
      ProjMatrix.z.z+=(REVERSE_DEPTH ? -1.0 : 1.0)/(1ull<<ImageTI[Renderer._ds->hwType()].d); // this adjusts the value that is responsible for 'LinearizeDepth' by moving everything back by 1 value (in depth buffer bit precision)
      if(ProjMatrix.z.z==mp_z_z) // if adding hasn't changed anything, then change by 1 bit (this can happen for small values)
         if(REVERSE_DEPTH)DecRealByBit(ProjMatrix.z.z);
         else             IncRealByBit(ProjMatrix.z.z);
      SetProjMatrix();
   }
}
static void RestoreViewSpaceBias(Flt mp_z_z)
{
   if(FovPerspective(D.viewFovMode())){ProjMatrix.z.z=mp_z_z; SetProjMatrix();}
}
void RendererClass::getShdRT()
{ // always do 'get' to call 'discard'
                                  Renderer._shd_1s.get(ImageRTDesc(Renderer._ds_1s->w(), Renderer._ds_1s->h(), IMAGERT_ONE));
   if(Renderer._ds->multiSample())Renderer._shd_ms.get(ImageRTDesc(Renderer._ds   ->w(), Renderer._ds   ->h(), IMAGERT_ONE, Renderer._ds->samples()));
   D.alpha(ALPHA_NONE);
}

static void GetLum()
{
                                   Renderer._lum   .get(ImageRTDesc(Renderer._col->w(), Renderer._col->h(), D.highPrecLumRT() ? IMAGERT_RGBA_H : IMAGERT_RGBA, Renderer._col->samples())); // here Alpha is used for specular
   if(Renderer._lum->multiSample())Renderer._lum_1s.get(ImageRTDesc(Renderer._lum->w(), Renderer._lum->h(), D.highPrecLumRT() ? IMAGERT_RGBA_H : IMAGERT_RGBA));else Renderer._lum_1s=Renderer._lum;
}
void RendererClass::getLumRT()
{
   if(!_lum)
   {
      GetLum();
      if(_lum_1s!=_lum)_lum_1s->clearViewport();
                       _lum   ->clearViewport(_ao ? Vec4Zero : Vec4(D.ambientColor(), 0)); // if '_ao' is not available then set '_lum' to 'ambientColor' (set '_lum' instead of '_lum_1s' because it is the one that is read in both 1-sample and multi-sample ColLight shaders, if this is changed then adjust all clears to '_lum_1s' and '_lum' in this file)
   }
}
static Bool SetLum()
{
   Bool set=!Renderer._lum; if(set)GetLum();
   Renderer.set(Renderer._lum_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests, start with '_lum_1s' so '_lum' will be processed later, because at the end we still have to render ambient from 3d meshes to '_lum' this way we avoid changing RT's
   if(set)
   {
      D.depth2DOff(); D.clearCol((Renderer._lum_1s!=Renderer._lum || Renderer._ao) ? Vec4Zero : Vec4(D.ambientColor(), 0));
      D.depth2DOn ();
   }
   D.alpha(ALPHA_ADD); Sh.h_ImageNrm[0]->set(Renderer._nrm);
   return set;
}

static void                GetWaterLum  () {Renderer._water_lum.get(ImageRTDesc(Renderer._water_ds->w(), Renderer._water_ds->h(), IMAGERT_RGBA));} // here Alpha is used for specular
       void RendererClass::getWaterLumRT() {if(!_water_lum){GetWaterLum(); _water_lum->clearViewport(Vec4(D.ambientColor(), 0));}}
static void                SetWaterLum  ()
{
   Bool set=!Renderer._water_lum; if(set)GetWaterLum();
   Renderer.set(Renderer._water_lum(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
   if(set)
   {
      D.depth2DOff(); D.clearCol(Vec4(D.ambientColor(), 0));
      D.depth2DOn ();
   }
   D.alpha(ALPHA_ADD); Sh.h_ImageNrm[0]->set(Renderer._water_nrm);
}

static void MapSoft()
{
   if(D.shadowSoft()) // this needs to be in sync with 'D.ambientSoft'
   {
      ImageRTDesc rt_desc(Renderer._shd_1s->w(), Renderer._shd_1s->h(), IMAGERT_ONE);
      if(D.shadowSoft()>=5)
      {
         ImageRTPtr temp; temp.get(rt_desc);
         Renderer.set(            temp(), Renderer._ds_1s(), true, NEED_DEPTH_READ);                              REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])Sh.h_ShdBlurX->draw(Renderer._shd_1s, &CurrentLightRect[Renderer._eye]); // use DS because it may be used for 'D.depth2D' optimization
         Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); Renderer._shd_1s->discard(); REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])Sh.h_ShdBlurY->draw(            temp, &CurrentLightRect[Renderer._eye]); // use DS because it may be used for 'D.depth2D' optimization
      }else
      {
         ImageRTPtr src=Renderer._shd_1s; Renderer._shd_1s.get(rt_desc);
         Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization
         REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])Sh.h_ShdBlur[D.shadowSoft()-1]->draw(src, &CurrentLightRect[Renderer._eye]);
      }
   }
}
static void RestoreLightSettings()
{
   SetCam(ActiveCam.matrix, false); Frustum=FrustumMain;
}
static void RestoreShadowMapSettings()
{
   D._view_active.set3DFrom(D._view_main).setProjMatrix(false);
   RestoreLightSettings();
}
/******************************************************************************/
LightCone::LightCone(Flt length, C VecD &pos, C Vec &dir, C Vec &color, Flt vol, Flt vol_max)
{
   T.pyramid.scale=1;
   T.pyramid.h    =length;
   T.pyramid.setPosDir(pos, dir);
   T.color        =color;
   T.falloff      =0.5f;
   T.vol          =vol;
   T.vol_max      =vol_max;
}
/******************************************************************************/
Flt LightPoint::range()C
{
   return Sqrt(power/(4.0f/255));
}
/******************************************************************************/
void LightDir  ::add(Bool shadow        , CPtr light_src                                                                       ) {           if(color.max()          >EPS_COL                       && Renderer.firstPass()){Lights.New().set(T,       shadow        , light_src);}}
void LightPoint::add(Flt  shadow_opacity, CPtr light_src                                                                       ) {Rect rect; if(color.max()*power    >EPS_COL && toScreenRect(rect) && Renderer.firstPass()){Lights.New().set(T, rect, shadow_opacity, light_src);}}
void LightSqr  ::add(Flt  shadow_opacity, CPtr light_src                                                                       ) {Rect rect; if(color.max()*range    >EPS_COL && toScreenRect(rect) && Renderer.firstPass()){Lights.New().set(T, rect, shadow_opacity, light_src);}}
void LightCone ::add(Flt  shadow_opacity, CPtr light_src, Image *image, Flt image_scale, C Color &image_add, Flt image_specular) {Rect rect; if(color.max()*pyramid.h>EPS_COL && toScreenRect(rect) && Renderer.firstPass())
   {
      Light &l=Lights.New();
      l.set(T, rect, shadow_opacity, light_src);
      l.image         =image;
      l.image_scale   =image_scale;
      l.image_add     =image_add;
      l.image_specular=image_specular;
   }
}
/******************************************************************************/
#pragma pack(push, 4)
struct GpuLightDir
{
   Vec dir, color;
   Flt spec, vol, vol_exponent, vol_steam;
};
struct GpuLightPoint
{
   Flt power, vol, vol_max;
   Vec pos, color;
   Flt spec;
};
struct GpuLightSqr
{
   Flt range, vol, vol_max;
   Vec pos, color;
   Flt spec;
};
struct GpuLightCone
{
   Matrix3 mtrx; // must be at start because of ATI OpenGL bug
   Flt     length, scale, vol, vol_max;
   Vec2    falloff;
   Vec     pos, color;
   Flt     spec;
};
#pragma pack(pop)
void LightDir::set()
{
   GpuLightDir l;
   l.dir         .fromDivNormalized(dir, CamMatrix.orn()).chs();
   l.color       =color;
   l.spec        =color.max();
   l.vol         =vol;
   l.vol_exponent=vol_exponent;
   l.vol_steam   =vol_steam;
   Sh.h_Light_dir->set(l);
}
void LightPoint::set(Flt shadow_opacity)
{
   GpuLightPoint l;
   l.power  =power;
   l.vol    =vol*shadow_opacity;
   l.vol_max=vol_max;
   l.pos    .fromDivNormalized(pos, CamMatrix);
   l.color  =color;
   l.spec   =color.max();
   Sh.h_Light_pnt->set(l);
}
void LightSqr::set(Flt shadow_opacity)
{
   GpuLightSqr l;
   l.range  =range;
   l.vol    =vol*shadow_opacity;
   l.vol_max=vol_max;
   l.pos    .fromDivNormalized(pos, CamMatrix);
   l.color  =color;
   l.spec   =color.max();
   Sh.h_Light_sqr->set(l);
}
void LightCone::set(Flt shadow_opacity)
{
   GpuLightCone l;
   // angular intensity = Length(pos)*l.falloff.x+l.falloff.y             falloff=0 Y|\         falloff=1 Y|  |
   // falloff=0         ->            l.falloff.x=-1    l.falloff.y=1                | \                   |  |
   // falloff=1         ->            l.falloff.x=-Inf  l.falloff.y=Inf              +--\X                 +--|X
   l.falloff.x   =-1.0f/Mid(falloff, EPS, 1.0f);
   l.falloff.y   =-l.falloff.x;
   l.vol         = vol*shadow_opacity;
   l.vol_max     = vol_max;
   l.color       = color;
   l.spec        = color.max();
   l.length      = pyramid.h;
   l.scale       = pyramid.scale;
   l.mtrx.x      =-pyramid.cross()/pyramid.scale;
   l.mtrx.y      =-pyramid.perp   /pyramid.scale;
   l.mtrx.z      =-pyramid.dir;
   l.mtrx.    divNormalized(             CamMatrix.orn());
   l.pos .fromDivNormalized(pyramid.pos, CamMatrix);
   Sh.h_Light_cone->set(l);
}
/******************************************************************************/
enum SHADOW_MAP_FLAG
{
   SM_FRUSTUM=0x1, // perform frustum tests
   SM_CLOUDS =0x2,
};
struct MatrixFovFrac
{
   Vec2    fov; // no need for 'VecD2'
   MatrixM matrix;
   Rect    frac; // 0..1 (fraction of the viewport that we actually need)
};
static void DrawShadowMap(DIR_ENUM dir, C MatrixM &cam_matrix, UInt flag, Flt view_from, Flt view_range, C Vec2 &fov, FOV_MODE fov_mode, Int border, Flt bias, C MatrixFovFrac *frustum=null)
{
#if FLAT_SHADOW_MAP
   {
      // apply cam matrix bias
      MatrixM cam_matrix_biased=cam_matrix;
      if(CurrentLight.type==LIGHT_DIR)cam_matrix_biased-=cam_matrix.z*bias;

      // set viewport
      RectI viewport_rect;
      if(CurrentLight.type==LIGHT_CONE)
      {
         viewport_rect.set(0, 0, D.shadowMapSizeActual()*2, D.shadowMapSizeActual()*2); // cone lights use 2x shadow map size
      }else
      {
         Int x=HsmX(dir),
             y=HsmY(dir);
         viewport_rect.set(x*D.shadowMapSizeActual(), y*D.shadowMapSizeActual(), (x+1)*D.shadowMapSizeActual(), (y+1)*D.shadowMapSizeActual());
      }
      if(frustum) // in 'D.shadowReduceFlicker', the needed area may not be the entire viewport rect, so use scissor
         D.clipAllow(RectI(Floor(viewport_rect.lerpX(frustum->frac.min.x)),
                           Floor(viewport_rect.lerpY(frustum->frac.min.y)),
                           Ceil (viewport_rect.lerpX(frustum->frac.max.x)),
                           Ceil (viewport_rect.lerpY(frustum->frac.max.y)))&viewport_rect); // AND with original viewport in case frac is slightly <0 || >1 or after floor/ceil we get +-1 values

      viewport_rect.extend(-border);
      D._view_active.set(viewport_rect, view_from, view_range, fov, fov_mode);
      SetCam(cam_matrix_biased, false); // camera without frustum

      if(frustum)Frustum.set(D._view_active.range,       frustum->fov, frustum->matrix);
      else       Frustum.set(D._view_active.range, D._view_active.fov,      cam_matrix);

      if(flag&SM_FRUSTUM)if(!FrustumMain(Frustum))return; // check if shadow frustum is visible (lies in main camera view frustum)
      D._view_active.setViewport(false).setProjMatrix(false);
   #if DX9 // on DX10+ and OpenGL clearing depth buffer clears it fully, so it can't be divided into steps
      if(CurrentLight.type!=LIGHT_DIR)D.clearDepth(); // depth buffer is cleared separately for local lights, directional light always uses all parts so it can be cleared all at once
   #endif

      // set matrix converting from shadow map to main camera space (required for tesselation - adaptive tesselation factors)
      Matrix temp; cam_matrix.divNormalized(ActiveCam.matrix, temp); // temp = cam_matrix/ActiveCam.matrix
      Sh.h_ShdMatrix->set(temp);

      D.samplerShadow(); D.bias(BIAS_SHADOW); D.depth(true); PrepareShadowInstances(); Renderer._render(); DrawShadowInstances();
      D.sampler2D    (); D.bias(BIAS_ZERO);

      if((flag&SM_CLOUDS) && Renderer._cld_map.is()) // clouds are only for directional lights
      {
         Int    x =HsmX(dir),
                y =HsmY(dir);
         Image *rt=Renderer._cur[0], *rtz=Renderer._cur_ds;
      #if DX9
         Byte col_write=D._col_write[0]; D.colWrite(COL_WRITE_RGBA); // restore color writing
      #endif
         Renderer      .set      (&Renderer._cld_map, null, false);
         D._view_active.set      (RectI(x*D.cloudsMapSize(), y*D.cloudsMapSize(), (x+1)*D.cloudsMapSize(), (y+1)*D.cloudsMapSize()), view_from, view_range, fov, fov_mode).setViewport(false).setProjMatrix(false); // viewport
         SetCam                  (cam_matrix, true); // camera and frustum
         Clouds        .shadowMap();
         Renderer      .set      (rt, rtz, false);
      #if DX9
         D.colWrite(col_write);
      #endif
      }
   }
#else
   {
                         Renderer      .setCube   (Renderer._shd_map_null, &Renderer._shd_map, dir);
                         D._view_active.set       (RectI(0, 0, Renderer.resW(), Renderer.resH()), view_from, view_range, fov, fov_mode).setViewport(false).setProjMatrix(false); // viewport
                         SetCam                   (cam_matrix, true); // camera and frustum
      if(flag&SM_FRUSTUM)if(!FrustumMain          (Frustum))return;
                         Sh            .clear     (Vec4(D._view_active.range*2));
                         D             .clearDepth();

      D.sampler3D(); D.bias(BIAS_SHADOW); D.depth(true); PrepareShadowInstances(); Renderer._render(); DrawShadowInstances();
      D.sampler2D(); D.bias(BIAS_ZERO);

      if((flag&SM_CLOUDS) && Renderer._cld_map.is())
      {
         Renderer      .setCube  (Renderer._cld_map, null, dir);
         D._view_active.set      (RectI(0, 0, Renderer.resW(), Renderer.resH()), view_from, view_range, fov, fov_mode).setViewport(false).setProjMatrix(false); // viewport
         SetCam                  (cam_matrix, true); // camera and frustum
         Clouds        .shadowMap();
      }
   }
#endif
}
/******************************************************************************/
static void StartVol()
{
   Bool clear=false;
   if(!Renderer._vol)
   {
      Renderer._vol.get(ImageRTDesc(Renderer.fxW()>>2, Renderer.fxH()>>2, IMAGERT_RGB)); // doesn't use Alpha
      clear=true;
      VL.load();
   }
   D       .alpha(ALPHA_ADD);
   Renderer.set  (Renderer._vol(), null, false); if(clear)D.clearCol();

   if(Renderer._stereo) // for stereoscopic we need to set correct eye, otherwise it was already set
   {
    //SetCam(EyeMatrix[Renderer._eye], false); this is not needed because all positions in shaders are based on ShdMatrix
      SetShdMatrix();
      Renderer.setEyeParams();
   }
   CurrentLight.set(); // call after setting the camera
}
static void ApplyVolumetric(LightDir &light, Int shd_map_num, Bool cloud_vol)
{
   if(Renderer.hasVolLight() && light.vol>EPS_COL && shd_map_num>0)REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])
   {
      StartVol();
      VL.h_VolDir[shd_map_num-1][cloud_vol]->draw(Renderer._vol(), &CurrentLightRect[Renderer._eye]);
   }
}
static void ApplyVolumetric(LightPoint &light)
{
   if(Renderer.hasVolLight() && light.vol>EPS_COL)REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])
   {
      StartVol();
      VL.h_Light_point_range->set(light.range());
      VL.h_VolPnt->draw(Renderer._vol(), &CurrentLightRect[Renderer._eye]);
   }
}
static void ApplyVolumetric(LightSqr &light)
{
   if(Renderer.hasVolLight() && light.vol>EPS_COL)REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])
   {
      StartVol();
      VL.h_VolSqr->draw(Renderer._vol(), &CurrentLightRect[Renderer._eye]);
   }
}
static void ApplyVolumetric(LightCone &light)
{
   if(Renderer.hasVolLight() && light.vol>EPS_COL)REPS(Renderer._eye, Renderer._eye_num)if(CurrentLightOn[Renderer._eye])
   {
      StartVol();
      VL.h_VolCone->draw(Renderer._vol(), &CurrentLightRect[Renderer._eye]);
   }
}
/******************************************************************************/
static Bool StereoCurrentLightRect() // this relies on current Camera Matrix
{
   if(!CurrentLight.toScreenRect(CurrentLight.rect))return false;

   // apply projection offset
   Flt po=ProjMatrixEyeOffset[Renderer._eye]*D.w()*0.5f;
   if(CurrentLight.rect.min.x>-D.w()+EPS)CurrentLight.rect.min.x+=po;
   if(CurrentLight.rect.max.x< D.w()-EPS)CurrentLight.rect.max.x+=po;

   // apply viewport offset
   Flt vo=D.w()*0.5f*SignBool(Renderer._eye!=0);
   CurrentLight.rect.min.x+=vo;
   CurrentLight.rect.max.x+=vo;

   // clamp and test if valid
   if(Renderer._eye){if(CurrentLight.rect.min.x<0)if(CurrentLight.rect.max.x>0)CurrentLight.rect.min.x=0;else return false;}
   else             {if(CurrentLight.rect.max.x>0)if(CurrentLight.rect.min.x<0)CurrentLight.rect.max.x=0;else return false;}

   return true;
}
static Bool GetCurrentLightRect()
{
   return Renderer._stereo ? StereoCurrentLightRect() : true;
}
static Bool SetLightEye(Bool shadow=false)
{
   CurrentLightOn[Renderer._eye]=false;
   if(Renderer._stereo)
   {
      SetCam(EyeMatrix[Renderer._eye], false);
      if(!StereoCurrentLightRect())return false;
      Renderer.setEyeParams();
   }
   if(shadow)
   {
      SetShdMatrix();
      CurrentLightOn  [Renderer._eye]=true;
      CurrentLightRect[Renderer._eye]=CurrentLight.rect;
   }else
   {
      CurrentLight.set();
   }
   return true;
}
/******************************************************************************/
static DIR_ENUM ShdDirStepDir[6]=
{
   DIR_FORWARD,
   DIR_BACK   ,
   DIR_RIGHT  ,
   DIR_LEFT   ,
   DIR_UP     ,
   DIR_DOWN   ,
};
static Byte ConnectedPoints[8][3]= // contains the indexes of 3 other points that this point is connected to
{ // points connected to point #: (the first value is from the other side, the 2 next values are +1 and -1 modulo)
   {4, 1, 3}, // 0, example: point 0 is connected to point 4 from the other side and mod(0+1)=1 and mod(0-1)=3
   {5, 2, 0}, // 1
   {6, 3, 1}, // 2
   {7, 0, 2}, // 3
   {0, 5, 7}, // 4
   {1, 6, 4}, // 5
   {2, 7, 5}, // 6
   {3, 4, 6}, // 7
};
static Flt ShadowFrac(C VecD &start, C VecD &end)
{
   return (start.y<=Renderer.lowest_visible_point) ? 0 // if start is below lowest visible point, then we don't need any shadows
        : (start.y<=end.y                        ) ? 1 // if start is below end (we're looking up, and not down, then we need full shadows)
        :                                            PointOnPlaneStep(start.y-Renderer.lowest_visible_point, end.y-Renderer.lowest_visible_point);
}
static Bool ShadowMap(LightDir &light)
{
   // init
   RENDER_MODE mode=Renderer(); Renderer.mode(RM_SHADOW);
   D.alpha(ALPHA_NONE); // disable alpha

#if DX9
   Renderer.set(&Renderer._shd_map_null, &Renderer._shd_map, false); // on DX9 there always needs to be a RT set
   D.clearDepth( ); // clear all shadow map parts at once, because for directional lights we'll use them all anyway
   D.colWrite  (0); // disable color writes
#else
   Renderer.set(null, &Renderer._shd_map, false);
   D.clearDepth(); // clear all shadow map parts at once, because for directional lights we'll use them all anyway
#endif

   Bool cloud_shd=false,
        cloud_vol=false;
   if(Clouds.draw && Clouds.volumetric.drawable() && Renderer._cld_map.is())
   {
      if(Clouds.volumetric.shadow>EPS_COL)cloud_shd=true;
      if(Renderer.hasVolLight() && CurrentLight.vol()>EPS_COL)cloud_vol=true;
   }

   // set light direction orientation
   Matrix3 orn; orn.setDir(light.dir); // set any orientation initially

   // set steps
   Bool point_clip   =(Renderer.lowest_visible_point>-DBL_MAX),
        clip         =D._clip; Rect clip_rect=D._clip_rect;
   Int  steps        =D.shadowMapNumActual();
   Int  points=8, points_1=points-1;
   VecD point[8], point_temp[8*3], *point_ptr=(point_clip ? point_temp : point);
   UInt flag         =((cloud_shd || cloud_vol) ? SM_CLOUDS : 0);
   Flt  bias         =GetBias(),
        range        =Renderer._shd_range*SHADOW_MAP_DIR_RANGE_MUL, // view range of the shadow map render target (this needs to be increased in case there are shadow occluders located distant from the camera), TODO: make SHADOW_MAP_DIR_RANGE_MUL configurable?
        shd_from     =D.viewFromActual(),                       // where does the shadow start in the main render target
        shd_to       =Max(Renderer._shd_range, shd_from+0.01f), // where does the shadow end   in the main render target (must be slightly larger than starting point)
        shd_from_frac=shd_from/D.viewRange(),
        shd_to_frac  =shd_to  /D.viewRange();

   if(point_clip && !D.shadowReduceFlicker())
   {
      Flt frac=shd_from_frac+0.01f/D.viewRange(); // must be slightly larger than starting point
      if(FrustumMain.points==5) // perspective (pyramid frustum)
      {
         MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[1]));
         MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[2]));
         MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[3]));
         MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[4]));
      }else // orthogonal (box frustum)
      {
         MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[4]));
         MAX(frac, ShadowFrac(FrustumMain.point[1], FrustumMain.point[5]));
         MAX(frac, ShadowFrac(FrustumMain.point[2], FrustumMain.point[6]));
         MAX(frac, ShadowFrac(FrustumMain.point[3], FrustumMain.point[7]));
      }
      MIN(shd_to_frac, frac);
   }
   
   Flt s0=Lerp(shd_from_frac, shd_to_frac, ShadowStep(0, steps));

   if(FrustumMain.points==5) // perspective (pyramid frustum)
   {
      point[0]=Lerp(FrustumMain.point[0], FrustumMain.point[1], s0);
      point[1]=Lerp(FrustumMain.point[0], FrustumMain.point[2], s0);
      point[2]=Lerp(FrustumMain.point[0], FrustumMain.point[3], s0);
      point[3]=Lerp(FrustumMain.point[0], FrustumMain.point[4], s0);
   }else // orthogonal (box frustum)
   {
      point[0]=Lerp(FrustumMain.point[0], FrustumMain.point[4], s0);
      point[1]=Lerp(FrustumMain.point[1], FrustumMain.point[5], s0);
      point[2]=Lerp(FrustumMain.point[2], FrustumMain.point[6], s0);
      point[3]=Lerp(FrustumMain.point[3], FrustumMain.point[7], s0);
   }
   FREP(steps) // go from start because 's0' and 'point' are already set for first step, and we calculate 's1' and 'point' for next steps in the loop
   {
      Flt //s0=Lerp(shd_from_frac, shd_to_frac, ShadowStep(i  , steps)); // convert to shd_from_frac .. shd_to_frac, already calculated
            s1=Lerp(shd_from_frac, shd_to_frac, ShadowStep(i+1, steps)), // convert to shd_from_frac .. shd_to_frac
            view_main_max=s1*D.viewRange();
      Sh.h_ShdStep[i]->set(view_main_max); // this value specifies at what depth what shadow map index should be used

      if(FrustumMain.points==5) // perspective (pyramid frustum)
      {
         point[4]=Lerp(FrustumMain.point[0], FrustumMain.point[1], s1);
         point[5]=Lerp(FrustumMain.point[0], FrustumMain.point[2], s1);
         point[6]=Lerp(FrustumMain.point[0], FrustumMain.point[3], s1);
         point[7]=Lerp(FrustumMain.point[0], FrustumMain.point[4], s1);
      }else // orthogonal (box frustum)
      {
         point[4]=Lerp(FrustumMain.point[0], FrustumMain.point[4], s1);
         point[5]=Lerp(FrustumMain.point[1], FrustumMain.point[5], s1);
         point[6]=Lerp(FrustumMain.point[2], FrustumMain.point[6], s1);
         point[7]=Lerp(FrustumMain.point[3], FrustumMain.point[7], s1);
      }

      MatrixM light_matrix; light_matrix.orn()=orn;

      if(point_clip) // if we want to clip the point edges
      {
         points=0;
         REPA(point) // iterate all original points
         {
          C VecD &p=point[i];
            if(p.y>=Renderer.lowest_visible_point)point_temp[points++]=p;else // if the point is above visible area, then include it as it is
            { // if the point is below
               Dbl d=p.y-Renderer.lowest_visible_point; // calculate Y distance to the plane
               REPD(j, 3) // iterate all 3 connected points to this one
               {
                C VecD &p1=point[ConnectedPoints[i][j]]; // get j-th connected point to point 'i'
                  if(p1.y>Renderer.lowest_visible_point) // other point must be above visible area (if both are below then we can just ignore this edge), here use > instead of >=, because if 'p1' is == then it's going to be included anyway in the check above, and the calculated point on plane below would be same as 'p1', so we would have the same point listed twice
                  {
                     Dbl d1=p1.y-Renderer.lowest_visible_point; // calculate Y distance to the plane
                     point_temp[points++]=PointOnPlane(p, p1, d, d1); // add the point that is intersection of the edge and the plane
                  }
               }
            }
         }
         if(points<3) // min 3 points required to construct a volume, otherwise we would have zero dimension frustum or even "points_1==-1"
         {
            light_matrix.pos.zero(); REPA(point)light_matrix.pos+=point[i]; light_matrix.pos/=Elms(point); // set position in the center of all points
            D._view_active.setFrom(0).setRange(range).setFov(1, FOV_ORTHO).setProjMatrix(false); // set 'ProjMatrix' projection matrix needed below to calculate correct transformation matrix, needed in case there will be some pixels outside of the 'Renderer.lowest_visible_point'
            goto skip;
         }
         points_1=points-1;
      }

      {
         // convert Frustum points onto 2D light orientation space
         VecD2 frustum_points_2D[Elms(point_temp)];
         REP(points){C VecD &p=point_ptr[i]; frustum_points_2D[i].set(Dot(p, light_matrix.x), Dot(p, light_matrix.y));}
         MatrixFovFrac frustum;
         VecD2 axis; if(BestFit(frustum_points_2D, points, axis)) // find best orientation alignment
         {
            frustum.matrix.y=axis.x*light_matrix.x + axis.y*light_matrix.y;
            frustum.matrix.x=Cross(frustum.matrix.y, light_matrix.z);
         }else
         {
            frustum.matrix.x=light_matrix.x;
            frustum.matrix.y=light_matrix.y;
         }
         if(!D.shadowReduceFlicker()) // we can use best fit matrix for light matrix only if 'shadowReduceFlicker' is disabled, because 'shadowReduceFlicker' works in a way that it always uses biggest worst possible alignment, so doing this would actually work against 'shadowReduceFlicker'
         {
            light_matrix.x=frustum.matrix.x;
            light_matrix.y=frustum.matrix.y;
         }

         // set sizes
         Dbl x[2], y[2], z[2];
         {
            // setup values from last point
            x[0]=x[1]=DistPointPlane(point_ptr[points_1], light_matrix.x);
            y[0]=y[1]=DistPointPlane(point_ptr[points_1], light_matrix.y);
            z[0]=z[1]=DistPointPlane(point_ptr[points_1], light_matrix.z);
            REP(points_1) // now iterate all remaining points and adjust the values
            {
               Dbl d=DistPointPlane(point_ptr[i], light_matrix.x); if(d<x[0])x[0]=d;else if(d>x[1])x[1]=d;
                   d=DistPointPlane(point_ptr[i], light_matrix.y); if(d<y[0])y[0]=d;else if(d>y[1])y[1]=d;
                   d=DistPointPlane(point_ptr[i], light_matrix.z); if(d<z[0])z[0]=d;else if(d>z[1])z[1]=d;
            }
         }

         // align sizes
         if(D.shadowReduceFlicker())
         {
            Dbl   max_dist=Sqrt(Max(Dist2(point[0], point[6]), Dist2(point[4], point[6]))); // Max(diagonal back<->front, diagonal front<->front), here use 'point' instead of 'point_ptr' (we need original points)
            VecD2 center(Avg(x[0], x[1]), Avg(y[0], y[1]));
            {
               Dbl a=max_dist/D.shadowMapSizeActual();
               Dbl o=AlignTrunc(center.x, 1024.0); center.x-=o; center.x=AlignFloor(center.x, a); center.x+=o; // offsetting by 1024 helps with precision issues at very big distances (512 km), we use Trunc in this case, because we want to move towards zero
                   o=AlignTrunc(center.y, 1024.0); center.y-=o; center.y=AlignFloor(center.y, a); center.y+=o;
            }
            Dbl max_dist_2=max_dist/2;
            Dbl min=center.x-max_dist_2; frustum.frac.setX((x[0]-min)/max_dist, (x[1]-min)/max_dist); x[0]=min; x[1]=center.x+max_dist_2;
                min=center.y-max_dist_2; frustum.frac.setY((y[0]-min)/max_dist, (y[1]-min)/max_dist); y[0]=min; y[1]=center.y+max_dist_2;
         }else
         {
            Dbl a=(x[1]-x[0])/D.shadowMapSizeActual(); x[0]=AlignFloor(x[0], a); x[1]=AlignFloor(x[1], a);
                a=(y[1]-y[0])/D.shadowMapSizeActual(); y[0]=AlignFloor(y[0], a); y[1]=AlignFloor(y[1], a);
         }

         // calculate fov and position
         Vec2 fov((x[1]-x[0])*0.5f, (y[1]-y[0])*0.5f); // no need for 'VecD2'
         light_matrix.pos=Avg(x[0], x[1])*light_matrix.x
                         +Avg(y[0], y[1])*light_matrix.y
                            +(z[1]-range)*light_matrix.z;

         // !! Warning: this modifies 'x' and 'y' to frustum fov !!
         if(D.shadowReduceFlicker()) // we need to set it only if we're going to use it
         {
            x[0]=x[1]=DistPointPlane(point_ptr[points_1], frustum.matrix.x);
            y[0]=y[1]=DistPointPlane(point_ptr[points_1], frustum.matrix.y);
            REP(points_1) // now iterate all remaining points and adjust the values
            {
               Dbl d=DistPointPlane(point_ptr[i], frustum.matrix.x); if(d<x[0])x[0]=d;else if(d>x[1])x[1]=d;
                   d=DistPointPlane(point_ptr[i], frustum.matrix.y); if(d<y[0])y[0]=d;else if(d>y[1])y[1]=d;
            }
            frustum.fov.set((x[1]-x[0])*0.5f, (y[1]-y[0])*0.5f);
            frustum.matrix.z  =light_matrix.z;
            frustum.matrix.pos=Avg(x[0], x[1])*frustum.matrix.x
                              +Avg(y[0], y[1])*frustum.matrix.y
                                 +(z[1]-range)*frustum.matrix.z;
         }

         // draw shadow map
         Flt step_bias=fov.max()*bias
                      +DepthError(0, range, range, false, ImageTI[Renderer._shd_map.hwType()].d);
         DrawShadowMap(ShdDirStepDir[i], light_matrix, flag, 0, range, fov, FOV_ORTHO, 0, step_bias, D.shadowReduceFlicker() ? &frustum : null); // set 'frustum' only if we need it, because if 'shadowReduceFlicker' is disabled, then we've already aligned the matrix and we don't need to do anything more
      }
   skip:

      // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix
      // potentially we could do 'light_matrix.inverse(true);" and then CamMatrix*light_matrix, however 'divNormalized' is just as fast
      if(Renderer._stereo)
      {
                   EyeMatrix[0].divNormalized(light_matrix, ShdMatrix[0]);
                   EyeMatrix[1].divNormalized(light_matrix, ShdMatrix[1]);
      }else ActiveCam.matrix   .divNormalized(light_matrix, ShdMatrix[0]);

   #if FLAT_SHADOW_MAP
      {
         Matrix4 mp; ProjMatrix.mul(HsmMatrixSet(ShdDirStepDir[i]), mp);
         REPS(Renderer._eye, Renderer._eye_num)ShdMatrix[Renderer._eye].mul(mp, ShdMatrix4[i][Renderer._eye]);
      }
   #else // cube shadow maps
      REPS(Renderer._eye, Renderer._eye_num)
      {
         // scale down from -fov..fov to -1..1 range
         Matrix m=ShdMatrix[Renderer._eye];
         MatrixFovScaleX(m, D._view_active.fov.x);
         MatrixFovScaleY(m, D._view_active.fov.y);

         Matrix4 &d=ShdMatrix4[i][Renderer._eye]; d=m;

         // copy 'z' to 'w' which will be used as final 'z' comparison value
         d.x  .w=d.x  .z;
         d.y  .w=d.y  .z;
         d.z  .w=d.z  .z;
         d.pos.w=d.pos.z;

         // apply bias
         d.pos.w-=step_bias;

         // adjust orientation for cube texture
         switch(ShdDirStepDir[i])
         {
            case DIR_FORWARD:                                                              d.x.z=0; d.y.z=0; d.z.z=0; d.pos.z= 1; break;
            case DIR_BACK   :    CHS(d.x.x);   CHS(d.y.x);   CHS(d.z.x);     CHS(d.pos.x); d.x.z=0; d.y.z=0; d.z.z=0; d.pos.z=-1; break;
            case DIR_RIGHT  : d.x.z=-d.x.x; d.y.z=-d.y.x; d.z.z=-d.z.x; d.pos.z=-d.pos.x;  d.x.x=0; d.y.x=0; d.z.x=0; d.pos.x= 1; break;
            case DIR_LEFT   : d.x.z= d.x.x; d.y.z= d.y.x; d.z.z= d.z.x; d.pos.z= d.pos.x;  d.x.x=0; d.y.x=0; d.z.x=0; d.pos.x=-1; break;
            case DIR_UP     : d.x.z=-d.x.y; d.y.z=-d.y.y; d.z.z=-d.z.y; d.pos.z=-d.pos.y;  d.x.y=0; d.y.y=0; d.z.y=0; d.pos.y= 1; break;
            case DIR_DOWN   : d.x.z= d.x.y; d.y.z= d.y.y; d.z.z= d.z.y; d.pos.z= d.pos.y;  d.x.y=0; d.y.y=0; d.z.y=0; d.pos.y=-1; break;
         }
      }
   #endif

      s0=s1;
      CopyFastN(&point[0], &point[4], 4);
   }

   // restore settings
   Renderer.mode(mode);
#if DX9
   D.colWrite(COL_WRITE_RGBA); // restore color writes
#endif
   RestoreShadowMapSettings();
   D.clip(clip ? &clip_rect : null); // no need to reset 'D.clipAllow' because it will be reset in the nearest RT change

   return cloud_shd;
}
/******************************************************************************/
static void ShadowMap(Flt range, VecD &pos)
{
   RENDER_MODE mode=Renderer(); Renderer.mode(RM_SHADOW);
   D.alpha(ALPHA_NONE); // disable alpha

#if DX9
   Renderer.set(&Renderer._shd_map_null, &Renderer._shd_map, false); // on DX9 there always needs to be a RT set
   D.colWrite(0); // disable color writes
#else
   Renderer.set(null, &Renderer._shd_map, false);
   D.clearDepth(); // clear all at once, must be done here because DX10+ and GL depth clear always clears full depth buffer
#endif

   Int border=0;
#if FLAT_SHADOW_MAP
   {
      Flt detail=Sat(CurrentLight.rect.size().max() / Min(D.w2(), D.h2())) * D.shadowMapSizeLocal();
      Int half  =D.shadowMapSizeActual()/2;
          border=Mid(half-RoundPos(detail*half), 0, half-1);
   }
#endif
   Int map_size=D.shadowMapSizeActual()-border*2;

   // render to shadow maps
   {
      UInt flag=(FrustumMain(pos) ? 0 : SM_FRUSTUM); // if the light source is inside the main frustum, then we don't need to test all light frustums because they all will be required
   #if FLAT_SHADOW_MAP
      Vec2 fov =2*Atan(1+2.0f/map_size); // padd 2 pixels of shadow map, needed for correct filtering at borders of each of 6 shadow maps, this doesn't require adjusting any other parameters, because it affects 'ProjMatrix' which is used later for matrix calculation
   #else
      Vec2 fov =PI_2;
   #endif
    //Flt view_main_max=DistPointPlane(pos, ActiveCam.matrix.pos, ActiveCam.matrix.z)+range;
      DrawShadowMap(DIR_FORWARD, MatrixM().setPos   (pos                                        ), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
      DrawShadowMap(DIR_BACK   , MatrixM().setPosDir(pos, VecDir[DIR_BACK ], VecDir[DIR_UP     ]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
      DrawShadowMap(DIR_LEFT   , MatrixM().setPosDir(pos, VecDir[DIR_LEFT ], VecDir[DIR_UP     ]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
      DrawShadowMap(DIR_RIGHT  , MatrixM().setPosDir(pos, VecDir[DIR_RIGHT], VecDir[DIR_UP     ]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
      DrawShadowMap(DIR_DOWN   , MatrixM().setPosDir(pos, VecDir[DIR_DOWN ], VecDir[DIR_FORWARD]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
      DrawShadowMap(DIR_UP     , MatrixM().setPosDir(pos, VecDir[DIR_UP   ], VecDir[DIR_BACK   ]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
   }

   // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix, ShdMatrix=CamMatrix-pos
   if(Renderer._stereo)
   {
         ShdMatrix[0].orn()=       EyeMatrix[0].orn(); ShdMatrix[0].pos=       EyeMatrix[0].pos-pos;
         ShdMatrix[1].orn()=       EyeMatrix[1].orn(); ShdMatrix[1].pos=       EyeMatrix[1].pos-pos;
   }else{ShdMatrix[0].orn()=ActiveCam.matrix   .orn(); ShdMatrix[0].pos=ActiveCam.matrix   .pos-pos;}

#if FLAT_SHADOW_MAP
   {
      Flt     f=D.shadowMapSizeActual()/Flt(map_size), bias=1-GetBias(); // for local lights instead of offsetting camera by bias, we scale it, because the shadow map is perspective so it needs to depend on distance to light
      Matrix  m;
      Matrix4 mp;
      ProjMatrix.mul(HsmMatrixSet(DIR_RIGHT  ), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s; m.x.x=-s.x.z; m.y.x=-s.y.z; m.z.x=-s.z.z; m.pos.x=-s.pos.z; m.x.z= s.x.x; m.y.z= s.y.x; m.z.z= s.z.x; m.pos.z= s.pos.x; MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[0][Renderer._eye]);} // right
      ProjMatrix.mul(HsmMatrixSet(DIR_LEFT   ), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s; m.x.x= s.x.z; m.y.x= s.y.z; m.z.x= s.z.z; m.pos.x= s.pos.z; m.x.z=-s.x.x; m.y.z=-s.y.x; m.z.z=-s.z.x; m.pos.z=-s.pos.x; MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[1][Renderer._eye]);} // left
      ProjMatrix.mul(HsmMatrixSet(DIR_UP     ), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s; m.x.z= s.x.y; m.y.z= s.y.y; m.z.z= s.z.y; m.pos.z= s.pos.y; m.x.y=-s.x.z; m.y.y=-s.y.z; m.z.y=-s.z.z; m.pos.y=-s.pos.z; MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[2][Renderer._eye]);} // up
      ProjMatrix.mul(HsmMatrixSet(DIR_DOWN   ), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s; m.x.z=-s.x.y; m.y.z=-s.y.y; m.z.z=-s.z.y; m.pos.z=-s.pos.y; m.x.y= s.x.z; m.y.y= s.y.z; m.z.y= s.z.z; m.pos.y= s.pos.z; MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[3][Renderer._eye]);} // down
      ProjMatrix.mul(HsmMatrixSet(DIR_BACK   ), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s;  CHS(m.x.x);   CHS(m.y.x);   CHS(m.z.x);    CHS(m.pos.x);     CHS(m.x.z);  CHS(m.y.z);   CHS(m.z.z);    CHS(m.pos.z);   MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[5][Renderer._eye]);} // back
      ProjMatrix.mul(HsmMatrixSet(DIR_FORWARD), mp); REPS(Renderer._eye, Renderer._eye_num){Matrix &s=ShdMatrix[Renderer._eye]; m=s;                                                                                                                         MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[4][Renderer._eye]);} // forward
   }
#endif

   // restore settings
   Renderer.mode(mode); // restore rendering mode to correctly restore viewport
#if DX9
   D.colWrite(COL_WRITE_RGBA); // restore color writes
#endif
   RestoreShadowMapSettings();
}
/******************************************************************************/
static void ShadowMap(LightCone &light)
{
   RENDER_MODE mode=Renderer(); Renderer.mode(RM_SHADOW);
   D.alpha(ALPHA_NONE); // disable alpha

#if DX9
   Renderer.set(&Renderer._shd_map_null, &Renderer._shd_map, false); // on DX9 there always needs to be a RT set
   D.colWrite(0); // disable color writes
#else
   Renderer.set(null, &Renderer._shd_map, false);
   D.clearDepth(); // clear all at once, must be done here because DX10+ and GL depth clear always clears full depth buffer
#endif

   Int map_size=D.shadowMapSizeActual(), border=0;
#if FLAT_SHADOW_MAP
   {
      map_size*=2; // for FLAT_SHADOW_MAP Cone we can use 2x map size because we have 2x3 sized full shadow map
      Flt detail=Sat(CurrentLight.rect.size().max() / Min(D.w2(), D.h2())) * D.shadowMapSizeLocal() * (D.shadowMapSizeCone()*0.5f);
      Int half  =map_size/2;
          border=Mid(half-RoundPos(detail*half), 0, half-1);
      map_size-=border*2;
   }
#endif

   // shadow map
   MatrixM light_matrix=light.pyramid;
 //Flt view_main_max=DistPointPlane(light_matrix.pos, ActiveCam.matrix.pos, ActiveCam.matrix.z)+light.pyramid.h; // this is not precise but worst case scenario
   DrawShadowMap(DIR_FORWARD, light_matrix, 0, LOCAL_SHADOW_MAP_FROM, light.pyramid.h, Vec2(2*Atan(light.pyramid.scale)), FOV_XY, border, 0); // use DIR_FORWARD because cube map shadows require that

   // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix
   // potentially we could do "light_matrix.inverse(true);" and then CamMatrix*LightMatrix, however 'divNormalized' is just as fast
   if(Renderer._stereo)
   {
                EyeMatrix[0].divNormalized(light_matrix, ShdMatrix[0]);
                EyeMatrix[1].divNormalized(light_matrix, ShdMatrix[1]);
   }else ActiveCam.matrix   .divNormalized(light_matrix, ShdMatrix[0]);

#if FLAT_SHADOW_MAP
   {
      Flt     f=D.shadowMapSizeActual()/Flt(map_size), bias=1-GetBias(); // for local lights instead of offsetting camera by bias, we scale it, because the shadow map is perspective so it needs to depend on distance to light
      Matrix4 mp; ProjMatrix.mul(HsmMatrixCone, mp);
      REPS(Renderer._eye, Renderer._eye_num){Matrix m=ShdMatrix[Renderer._eye]; MatrixFovScale(m, f); m.scale(bias); m.mul(mp, ShdMatrix4[0][Renderer._eye]);}
   }
#endif
   REPS(Renderer._eye, Renderer._eye_num)MatrixFovScale(ShdMatrix[Renderer._eye], light.pyramid.scale);

   // restore settings
   Renderer.mode(mode); // restore rendering mode to correctly restore viewport
#if DX9
   D.colWrite(COL_WRITE_RGBA); // restore color writes
#endif
   RestoreShadowMapSettings();
}
/******************************************************************************/
Flt Light::range()C
{
   switch(type)
   {
      case LIGHT_POINT: return point.range()  ;
      case LIGHT_SQR  : return sqr  .range    ;
      case LIGHT_CONE : return cone .pyramid.h;
      default         : return 0              ;
   }
}
Flt Light::vol()C
{
   switch(type)
   {
      case LIGHT_DIR  : return dir  .vol;
      case LIGHT_POINT: return point.vol;
      case LIGHT_SQR  : return sqr  .vol;
      case LIGHT_CONE : return cone .vol;
      default         : return 0        ;
   }
}
VecD Light::pos()C
{
   switch(type)
   {
      case LIGHT_POINT: return point.pos        ;
      case LIGHT_SQR  : return sqr  .pos        ;
      case LIGHT_CONE : return cone .pyramid.pos;
      default         : return 0                ;
   }
}
Bool Light::toScreenRect(Rect &rect)C
{
   switch(type)
   {
      case LIGHT_DIR  : return dir  .toScreenRect(rect);
      case LIGHT_POINT: return point.toScreenRect(rect);
      case LIGHT_SQR  : return sqr  .toScreenRect(rect);
      case LIGHT_CONE : return cone .toScreenRect(rect);
      default         : return false;
   }
}
void Light::scalePower(Flt scale)
{
   switch(type)
   {
      case LIGHT_DIR  : dir  .color    *=scale; break;
      case LIGHT_POINT: point.power    *=scale; break;
      case LIGHT_SQR  : sqr  .range    *=scale; break;
      case LIGHT_CONE : cone .pyramid.h*=scale; break;
   }
}
/******************************************************************************/
void Light::set()
{
   switch(type)
   {
      case LIGHT_DIR  : dir  .set(              ); break;
      case LIGHT_POINT: point.set(shadow_opacity); break;
      case LIGHT_SQR  : sqr  .set(shadow_opacity); break;
      case LIGHT_CONE : cone .set(shadow_opacity); break;
   }
}
/******************************************************************************/
static Bool CanDoShadow()
{
   return D.shadowMode() && D.shadowSupported() && Renderer._cur_type!=RT_SIMPLE && FovPerspective(D.viewFovMode());
}
void Light::set(LightDir   &light,               Bool shadow        , CPtr light_src) {Zero(T); type=LIGHT_DIR  ; dir  =light; T.rect=D.viewRect(); T.shadow=(shadow                 && CanDoShadow()); T.shadow_opacity=(T.shadow                          ); T.src=light_src;}
void Light::set(LightPoint &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src) {Zero(T); type=LIGHT_POINT; point=light; T.rect=        rect; T.shadow=(shadow_opacity>EPS_COL && CanDoShadow()); T.shadow_opacity=(T.shadow ? Sat(shadow_opacity) : 0); T.src=light_src;}
void Light::set(LightSqr   &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src) {Zero(T); type=LIGHT_SQR  ; sqr  =light; T.rect=        rect; T.shadow=(shadow_opacity>EPS_COL && CanDoShadow()); T.shadow_opacity=(T.shadow ? Sat(shadow_opacity) : 0); T.src=light_src;}
void Light::set(LightCone  &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src) {Zero(T); type=LIGHT_CONE ; cone =light; T.rect=        rect; T.shadow=(shadow_opacity>EPS_COL && CanDoShadow()); T.shadow_opacity=(T.shadow ? Sat(shadow_opacity) : 0); T.src=light_src;}
/******************************************************************************/
void Light::draw()
{
   // !! Here process water lum first, because after drawing all lights, we still have to apply ambient from meshes, so if we process water lum first, and then lum, then we don't need to change RT's to lum again when drawing ambient from meshes !!
   SetShadowOpacity(shadow_opacity);

          CurrentLight=T;
   switch(CurrentLight.type)
   {
      case LIGHT_DIR:
      {
         Int  shd_map_num;
         Bool cloud=false;
         if(CurrentLight.shadow)
         {
            shd_map_num=D.shadowMapNumActual();
            cloud=ShadowMap(CurrentLight.dir);
         }

         D.depth2DOn();

         // water lum first, as noted above in the comments
         if(Renderer._water_nrm)
         {
            D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdDir(Mid(shd_map_num, 1, 6), cloud, false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightDir(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.stencil(STENCIL_NONE);
         }

         // lum
         if(CurrentLight.shadow)
         {
            Renderer.getShdRT();
            Flt mp_z_z; ApplyViewSpaceBias(mp_z_z);
            if(!Renderer._ds->multiSample())
            {
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdDir(shd_map_num, cloud, false)->draw(null, &CurrentLight.rect);
            }else
            { // we can ignore 'Renderer.hasStencilAttached' because we would have to apply for all samples of '_shd_ms' and '_shd_1s' which will happen anyway below
               Renderer.set(Renderer._shd_ms(), Renderer._ds   (), true, NEED_DEPTH_READ); D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdDir(shd_map_num, cloud, true )->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); D.stencil(STENCIL_NONE                       ); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdDir(shd_map_num, cloud, false)->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization, for all stencil samples because they are needed for smoothing
            }
            RestoreViewSpaceBias(mp_z_z);
            MapSoft();
            ApplyVolumetric(CurrentLight.dir, shd_map_num, false); // TODO: cloud shd/vol is disabled
         }
         Bool clear=SetLum();
         if(!Renderer._ds->multiSample()) // 1-sample
         {
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightDir(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
         #if TEST_LIGHT_RECT
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
         #endif
         }else // multi-sample
         {
            if(Renderer.hasStencilAttached()) // if we can use stencil tests, then process 1-sample pixels using 1-sample shader, if we can't use stencil then all pixels will be processed using multi-sample shader later below
            {
               D.stencil(STENCIL_MSAA_TEST, 0);
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightDir(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
            }
            Renderer.set(Renderer._lum(), Renderer._ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
            if(clear)
            {
               D.depth2DOff(); D.stencil(STENCIL_NONE); D.clearCol(Renderer._ao ? Vec4Zero : Vec4(D.ambientColor(), 0));
               D.depth2DOn ();
            }
          /*if(Renderer.hasStencilAttached()) not needed because stencil tests are disabled without stencil RT */D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightDir(CurrentLight.shadow, true, D.highPrecNrmCalcIs())->draw(Renderer._shd_ms, &CurrentLight.rect);
            D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_POINT:
      {
         if(CurrentLight.shadow)ShadowMap(CurrentLight.point.range(), CurrentLight.point.pos);

         D.depth2DOn();

         // water lum first, as noted above in the comments
         if(Renderer._water_nrm)
         {
            D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightPnt(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.stencil(STENCIL_NONE);
         }

         // lum
         if(CurrentLight.shadow)
         {
            Renderer.getShdRT();
            Flt mp_z_z; ApplyViewSpaceBias(mp_z_z);
            if(!Renderer._ds->multiSample())
            {
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }else
            { // we can ignore 'Renderer.hasStencilAttached' because we would have to apply for all samples of '_shd_ms' and '_shd_1s' which will happen anyway below
               Renderer.set(Renderer._shd_ms(), Renderer._ds   (), true, NEED_DEPTH_READ); D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(true )->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); D.stencil(STENCIL_NONE                       ); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization, for all stencil samples because they are needed for smoothing
            }
            RestoreViewSpaceBias(mp_z_z);
            MapSoft();
            ApplyVolumetric(CurrentLight.point);
         }
         Bool clear=SetLum();
         if(!Renderer._ds->multiSample()) // 1-sample
         {
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightPnt(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
         #if TEST_LIGHT_RECT
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
         #endif
         }else // multi-sample
         {
            if(Renderer.hasStencilAttached()) // if we can use stencil tests, then process 1-sample pixels using 1-sample shader, if we can't use stencil then all pixels will be processed using multi-sample shader later below
            {
               D.stencil(STENCIL_MSAA_TEST, 0);
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightPnt(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
            }
            Renderer.set(Renderer._lum(), Renderer._ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
            if(clear)
            {
               D.depth2DOff(); D.stencil(STENCIL_NONE); D.clearCol(Renderer._ao ? Vec4Zero : Vec4(D.ambientColor(), 0));
               D.depth2DOn ();
            }
          /*if(Renderer.hasStencilAttached()) not needed because stencil tests are disabled without stencil RT */D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightPnt(CurrentLight.shadow, true, D.highPrecNrmCalcIs())->draw(Renderer._shd_ms, &CurrentLight.rect);
            D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_SQR:
      {
         if(CurrentLight.shadow)ShadowMap(CurrentLight.sqr.range, CurrentLight.sqr.pos);

         D.depth2DOn();

         // water lum first, as noted above in the comments
         if(Renderer._water_nrm)
         {
            D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightSqr(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.stencil(STENCIL_NONE);
         }

         // lum
         if(CurrentLight.shadow)
         {
            Renderer.getShdRT();
            Flt mp_z_z; ApplyViewSpaceBias(mp_z_z);
            if(!Renderer._ds->multiSample())
            {
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }else
            { // we can ignore 'Renderer.hasStencilAttached' because we would have to apply for all samples of '_shd_ms' and '_shd_1s' which will happen anyway below
               Renderer.set(Renderer._shd_ms(), Renderer._ds   (), true, NEED_DEPTH_READ); D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(true )->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); D.stencil(STENCIL_NONE                       ); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization, for all stencil samples because they are needed for smoothing
            }
            RestoreViewSpaceBias(mp_z_z);
            MapSoft();
            ApplyVolumetric(CurrentLight.sqr);
         }
         Bool clear=SetLum();
         if(!Renderer._ds->multiSample()) // 1-sample
         {
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightSqr(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
         #if TEST_LIGHT_RECT
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
         #endif
         }else // multi-sample
         {
            if(Renderer.hasStencilAttached()) // if we can use stencil tests, then process 1-sample pixels using 1-sample shader, if we can't use stencil then all pixels will be processed using multi-sample shader later below
            {
               D.stencil(STENCIL_MSAA_TEST, 0);
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightSqr(CurrentLight.shadow, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
            }
            Renderer.set(Renderer._lum(), Renderer._ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
            if(clear)
            {
               D.depth2DOff(); D.stencil(STENCIL_NONE); D.clearCol(Renderer._ao ? Vec4Zero : Vec4(D.ambientColor(), 0));
               D.depth2DOn ();
            }
          /*if(Renderer.hasStencilAttached()) not needed because stencil tests are disabled without stencil RT */D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightSqr(CurrentLight.shadow, true, D.highPrecNrmCalcIs())->draw(Renderer._shd_ms, &CurrentLight.rect);
            D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_CONE:
      {
         if(CurrentLight.shadow)ShadowMap(CurrentLight.cone);

         D.depth2DOn();

         // water lum first, as noted above in the comments
         if(Renderer._water_nrm)
         {
            D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdCone(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightCone(CurrentLight.shadow, CurrentLight.image?1:0, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.stencil(STENCIL_NONE);
         }

         // lum
         if(CurrentLight.shadow)
         {
            Renderer.getShdRT();
            Flt mp_z_z; ApplyViewSpaceBias(mp_z_z);
            if(!Renderer._ds->multiSample())
            {
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdCone(false)->draw(null, &CurrentLight.rect);
            }else
            { // we can ignore 'Renderer.hasStencilAttached' because we would have to apply for all samples of '_shd_ms' and '_shd_1s' which will happen anyway below
               Renderer.set(Renderer._shd_ms(), Renderer._ds   (), true, NEED_DEPTH_READ); D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdCone(true )->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization
               Renderer.set(Renderer._shd_1s(), Renderer._ds_1s(), true, NEED_DEPTH_READ); D.stencil(STENCIL_NONE                       ); REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdCone(false)->draw(null, &CurrentLight.rect); // use DS because it may be used for 'D.depth2D' optimization, for all stencil samples because they are needed for smoothing
            }
            RestoreViewSpaceBias(mp_z_z);
            MapSoft();
            ApplyVolumetric(CurrentLight.cone);
         }
         if(CurrentLight.image)
         {
            Sh.h_LightMapScale   ->set(CurrentLight.image_scale   );
            Sh.h_LightMapColAdd  ->set(CurrentLight.image_add     );
            Sh.h_LightMapSpecular->set(CurrentLight.image_specular);
            Sh.h_ImageCol[1]     ->set(CurrentLight.image         );
         }
         Bool clear=SetLum();
         if(!Renderer._ds->multiSample()) // 1-sample
         {
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightCone(CurrentLight.shadow, CurrentLight.image?1:0, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
         #if TEST_LIGHT_RECT
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
         #endif
         }else // multi-sample
         {
            if(Renderer.hasStencilAttached()) // if we can use stencil tests, then process 1-sample pixels using 1-sample shader, if we can't use stencil then all pixels will be processed using multi-sample shader later below
            {
               D.stencil(STENCIL_MSAA_TEST, 0);
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightCone(CurrentLight.shadow, CurrentLight.image?1:0, false, D.highPrecNrmCalcIs())->draw(Renderer._shd_1s, &CurrentLight.rect);
            }
            Renderer.set(Renderer._lum(), Renderer._ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
            if(clear)
            {
               D.depth2DOff(); D.stencil(STENCIL_NONE); D.clearCol(Renderer._ao ? Vec4Zero : Vec4(D.ambientColor(), 0));
               D.depth2DOn ();
            }
          /*if(Renderer.hasStencilAttached()) not needed because stencil tests are disabled without stencil RT */D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightCone(CurrentLight.shadow, CurrentLight.image?1:0, true, D.highPrecNrmCalcIs())->draw(Renderer._shd_ms, &CurrentLight.rect);
            D.stencil(STENCIL_NONE);
         }
      }break;
   }
   D.depth2DOff();
   Renderer._shd_1s.clear();
   Renderer._shd_ms.clear();
}
/******************************************************************************/
void Light::drawForward(Image *dest, ALPHA_MODE alpha)
{
   SetShadowOpacity(shadow_opacity);

          CurrentLight=T;
   switch(CurrentLight.type)
   {
      case LIGHT_DIR:
      {
         Int shd_map_num;
         if(CurrentLight.shadow)
         {
            shd_map_num=D.shadowMapNumActual();
            ShadowMap(CurrentLight.dir);
            Renderer._frst_light_offset=OFFSET(FRST, dir_shd[Mid(shd_map_num, 1, 6)-1]);
         }else
         {
            Renderer._frst_light_offset=OFFSET(FRST, dir);
         }

         Renderer.set(dest, Renderer._ds(), true);
         D.alpha(alpha);
         D.set3D();
         if(Renderer.firstPass())
         {
            D.stencil(STENCIL_ALWAYS_SET, 0);
         }else
         {  // we need to generate list of objects
            Renderer.mode(RM_PREPARE); Frustum.set(); Renderer._render(); // set frustum after setting render mode
            D.clipAllow(true);
         }
         Renderer.mode(RM_SOLID);
         REPS(Renderer._eye, Renderer._eye_num)
         {
            Renderer.setEyeViewport();
            if(CurrentLight.shadow)SetShdMatrix();
            CurrentLight.dir.set();
            if(Renderer.secondaryPass())D.clip(Renderer._clip); // clip rendering to area affected by the light
            DrawSolidInstances(); Renderer._render();
         }
         ClearSolidInstances();
         D.set2D();

         if(Renderer.firstPass()){D.stencil(STENCIL_NONE); Renderer.resolveDepth();}

         // water lum
         if(Renderer._water_nrm)
         {
            D.depth2DOn(); D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdDir(Mid(shd_map_num, 1, 6), false, false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightDir(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.depth2DOff(); D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_POINT:
      {
         if(CurrentLight.shadow)
         {
            ShadowMap(CurrentLight.point.range(), CurrentLight.point.pos);
            Renderer._frst_light_offset=OFFSET(FRST, pnt_shd);
         }else
         {
            Renderer._frst_light_offset=OFFSET(FRST, pnt);
         }

         Renderer.set(dest, Renderer._ds(), true);
         D.alpha(alpha);
         D.set3D();
         if(Renderer.firstPass())
         {
            D.stencil(STENCIL_ALWAYS_SET, 0);
         }else
         {  // we need to generate list of objects
            Renderer.mode(RM_PREPARE); Frustum.from(BoxD(CurrentLight.point.range(), CurrentLight.point.pos)); // set frustum after setting render mode
            Renderer._render();
            D.clipAllow(true);
         }
         Renderer.mode(RM_SOLID);
         REPS(Renderer._eye, Renderer._eye_num)
         {
            Renderer.setEyeViewport();
            GetCurrentLightRect(); // if(.. would require ClearSolidInstances afterwards, call this after setting viewport and camera
            if(CurrentLight.shadow)SetShdMatrix();
            CurrentLight.point.set(CurrentLight.shadow_opacity);
            if(Renderer.secondaryPass())D.clip(CurrentLight.rect&Renderer._clip); // clip rendering to area affected by the light
            DrawSolidInstances(); Renderer._render();
         }
         ClearSolidInstances();
         D.set2D();

         if(Renderer.firstPass()){D.stencil(STENCIL_NONE); Renderer.resolveDepth();}

         // water lum
         if(Renderer._water_nrm)
         {
            D.depth2DOn(); D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightPnt(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.depth2DOff(); D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_SQR:
      {
         if(CurrentLight.shadow)
         {
            ShadowMap(CurrentLight.sqr.range, CurrentLight.sqr.pos);
            Renderer._frst_light_offset=OFFSET(FRST, sqr_shd);
         }else
         {
            Renderer._frst_light_offset=OFFSET(FRST, sqr);
         }

         Renderer.set(dest, Renderer._ds(), true);
         D.alpha(alpha);
         D.set3D();
         if(Renderer.firstPass())
         {
            D.stencil(STENCIL_ALWAYS_SET, 0);
         }else
         {  // we need to generate list of objects
            Renderer.mode(RM_PREPARE); Frustum.from(BoxD(CurrentLight.sqr.range, CurrentLight.sqr.pos)); // set frustum after setting render mode
            Renderer._render();
            D.clipAllow(true);
         }
         Renderer.mode(RM_SOLID);
         REPS(Renderer._eye, Renderer._eye_num)
         {
            Renderer.setEyeViewport();
            GetCurrentLightRect(); // if(.. would require ClearSolidInstances afterwards, call this after setting viewport and camera
            if(CurrentLight.shadow)SetShdMatrix();
            CurrentLight.sqr.set(CurrentLight.shadow_opacity);
            if(Renderer.secondaryPass())D.clip(CurrentLight.rect&Renderer._clip); // clip rendering to area affected by the light
            DrawSolidInstances(); Renderer._render();
         }
         ClearSolidInstances();
         D.set2D();

         if(Renderer.firstPass()){D.stencil(STENCIL_NONE); Renderer.resolveDepth();}

         // water lum
         if(Renderer._water_nrm)
         {
            D.depth2DOn(); D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdPnt(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightSqr(CurrentLight.shadow, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.depth2DOff(); D.stencil(STENCIL_NONE);
         }
      }break;

      case LIGHT_CONE:
      {
         if(CurrentLight.shadow)
         {
            ShadowMap(CurrentLight.cone);
            Renderer._frst_light_offset=OFFSET(FRST, cone_shd);
         }else
         {
            Renderer._frst_light_offset=OFFSET(FRST, cone);
         }

         Renderer.set(dest, Renderer._ds(), true);
         D.alpha(alpha);
         D.set3D();
         if(Renderer.firstPass())
         {
            D.stencil(STENCIL_ALWAYS_SET, 0);
         }else
         {  // we need to generate list of objects
            Renderer.mode(RM_PREPARE); Frustum.from(CurrentLight.cone.pyramid); // set frustum after setting render mode
            Renderer._render();
            D.clipAllow(true);
         }
         Renderer.mode(RM_SOLID);
         REPS(Renderer._eye, Renderer._eye_num)
         {
            Renderer.setEyeViewport();
            GetCurrentLightRect(); // if(.. would require ClearSolidInstances afterwards, call this after setting viewport and camera
            if(CurrentLight.shadow)SetShdMatrix();
            CurrentLight.cone.set(CurrentLight.shadow_opacity);
            if(Renderer.secondaryPass())D.clip(CurrentLight.rect&Renderer._clip); // clip rendering to area affected by the light
            DrawSolidInstances(); Renderer._render();
         }
         ClearSolidInstances();
         D.set2D();

         if(Renderer.firstPass()){D.stencil(STENCIL_NONE); Renderer.resolveDepth();}

         // water lum
         if(Renderer._water_nrm)
         {
            D.depth2DOn(); D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
            Sh.h_ImageDepth->set(Renderer._water_ds); // set water depth

            if(CurrentLight.shadow)
            {
               // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
               Renderer.getShdRT();
               Renderer.set(Renderer._shd_1s(), Renderer._water_ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
               REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye(true))GetShdCone(false)->draw(null, &CurrentLight.rect);
            }

            SetWaterLum();
            REPS(Renderer._eye, Renderer._eye_num)if(SetLightEye())GetLightCone(CurrentLight.shadow, CurrentLight.image?1:0, false, true)->draw(Renderer._shd_1s, &CurrentLight.rect); // always use Quality Specular for Water

            Sh.h_ImageDepth->set(Renderer._ds_1s); // restore default depth
            D.depth2DOff(); D.stencil(STENCIL_NONE);
         }
      }break;
   }
   Renderer._shd_1s.clear();
   Renderer._shd_ms.clear();
}
/******************************************************************************/
#define MAX_LIGHTS 255 // max amount of lights allowed when limiting lights
struct LightFade
{
   Flt  fade;
   CPtr src ;
};
static LightFade LightFades[2][MAX_LIGHTS];
static Int       LightFadesNum=0;
static Bool      LightFadeIndex=false;
static Light     LightTemp[MAX_LIGHTS];

void LimitLights()
{
   if(Lights.elms() && D._max_lights)
   {
      MIN(D._max_lights, MAX_LIGHTS);

      // limit number of lights
      if(Lights.elms()>D._max_lights)
      {
         LightImportance.setNum(Lights.elms());
         REPA(Lights)
         {
            Light &l=Lights[i];
            LightImportance[i].f=((l.type==LIGHT_DIR) ? 1-l.dir.color.max() : 1+Dist2(ActiveCam.at, l.pos()));
            LightImportance[i].i=i;
         }
         LightImportance.sort(FloatIndex::Compare);
         REP(D._max_lights)LightTemp[i]=Lights[LightImportance[i].i];
         Lights.setNum(D._max_lights); REPAO(Lights)=LightTemp[i];
      }

      // fade in
      Int         old_fades=LightFadesNum; LightFadesNum=0;
      LightFade (&old_fade)[MAX_LIGHTS]=LightFades[ LightFadeIndex],
                (&new_fade)[MAX_LIGHTS]=LightFades[!LightFadeIndex];
      REPA(Lights)
      {
         Light &l=Lights[i];
         if(l.src)
         {
            LightFade &lf=new_fade[LightFadesNum++];
            lf.src =l.src;
            lf.fade=Time.rd()*3; REP(old_fades)if(old_fade[i].src==l.src){lf.fade+=old_fade[i].fade; break;} SAT(lf.fade);
            l.scalePower(lf.fade);
         }
      }
      LightFadeIndex^=1;
   }
}
void SortLights()
{
   // put main directional light at start (this is needed for RM_BLEND_LIGHT*, RT_FORWARD and RT_SIMPLE)
   if(Lights.elms()>1)
   {
      Int main =-1;
      Flt power= 0; REPA(Lights)
      {
         Light &light=Lights[i];
         if(light.type==LIGHT_DIR)
         {
            Flt p=light.dir.color.max();
            if(main<0 || p>power){main=i; power=p;}
         }
      }
      if(main>0)Swap(Lights[0], Lights[main]); // >0 already handles !=-1 (not found) and !=0 (no need to move)
   }
}
void DrawLights()
{
   if(Lights.elms())
   {
      REPAO(Lights).draw(); // apply in backward order to leave main shadow map in the end
      RestoreLightSettings(); // restore this because camera matrix may have changed even when not rendering shadows, but when using stereoscopic rendering and setting cameras for each eye
      MaterialClear();
   }
}
/******************************************************************************/
void ShutLight() {Lights.del(); LightImportance.del();}
void InitLight()
{
   HsmMatrix.x  .x= 0.5f/2;
   HsmMatrix.y  .y=-0.5f/3;
   HsmMatrix.z  .z= 1;
   HsmMatrix.pos.w= 1;
   
   HsmMatrixCone=HsmMatrix;
   HsmMatrixCone.pos.x=0.5f;
   HsmMatrixCone.pos.y=Avg(0, 2.0f/3);
}
/******************************************************************************/
}
/******************************************************************************/
