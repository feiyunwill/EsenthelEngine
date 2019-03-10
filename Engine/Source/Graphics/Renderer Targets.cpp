/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   RT       : Width      , Height     , Type                                                                    , Samples    , Comments
  _main     : D.   resW(), D.   resH(),                                                             IMAGERT_RGBA, 1          , COLOR RGB, Opacity
  _main_ds  : D.   resW(), D.   resH(),                                                             IMAGERT_RGBA, 1          , this is the Main DepthStencil buffer to be used together with '_main' RT, on OpenGL (except iOS) it is provided by the system
  _ds       : D.renderW(), D.renderW(),                                                             IMAGERT_DS  , D.samples()
  _ds_1s    : D.renderW(), D.renderW(),                                                             IMAGERT_DS  , 1          , if '_ds' is Multi-Sampled then this is created as a standalone 1-sampled depth buffer, otherwise it's a duplicate of '_ds'
  _col      : D.renderW(), D.renderH(),                        D.highPrecColRT() ? IMAGERT_RGBA_P : IMAGERT_RGBA, D.samples(), COLOR RGB, GLOW
  _nrm      : D.renderW(), D.renderH(), D.signedNrmRT() ? .. : D.highPrecNrmRT() ? IMAGERT_RGBA_P : IMAGERT_RGBA, D.samples(), NRM   XYZ, SPEC
  _vel      : D.renderW(), D.renderH(), D.signedVelRT() ? IMAGERT_RGB_S : IMAGERT_RGB                           , D.samples(), VEL   XYZ
  _pos      : D.renderW(), D.renderH(),                                                             IMAGERT_F32 , 1          , Linearized depth
  _lum      : D.renderW(), D.renderH(),                        D.highPrecLumRT() ? IMAGERT_RGBA_H : IMAGERT_RGBA, D.samples(), LIGHT RGB, LIGHT SPEC
  _lum_1s   : D.renderW(), D.renderH(),                        D.highPrecLumRT() ? IMAGERT_RGBA_H : IMAGERT_RGBA, 1          , LIGHT RGB, LIGHT SPEC. if '_lum' is Multi-Sampled then this is created as a standalone 1-sampled depth buffer, otherwise it's a duplicate of '_lum'

  _water_col: D.renderW(), D.renderH(),                                                             IMAGERT_RGBA, 1          , COLOR RGB, UNUSED
  _water_nrm: D.renderW(), D.renderH(), D.signedNrmRT() ? IMAGERT_RGBA_S : IMAGERT_RGBA                         , 1          , NRM   XYZ, SPEC. High precision is not needed because we just use better UnpackNormal in the shader
  _water_ds : D.renderW(), D.renderH(),                                                             IMAGERT_DS  , 1          , Water Depth
  _water_lum: D.renderW(), D.renderH(),                                                             IMAGERT_RGBA, 1          , LIGHT RGB, LIGHT SPEC

  '_gui' is set to '_main', unless stereoscopic rendering is enabled then it's set to VR RT

   If '_ds' is multi-sampled on DX10+ then:
      In Deferred Renderer:
         -'_ds_1s' is set to down-sampled copy of '_ds'
         -both '_ds' and '_ds_1s' have STENCIL_REF_MSAA set
      In Non-Deferred Renderer:
         -if 'slowCombine'              then '_ds_1s' has STENCIL_REF_MSAA set
         -if "Fog.draw || Sky.isActual" then '_ds'    has STENCIL_REF_MSAA set

   In OpenGL (except iOS):
      '_main' and '_main_ds' don't have _rb and _txtr set, because they're provided by the system and not created by the engine.
      This means that when setting '_main' it's always paired with '_main_ds' depth buffer, and '_main_ds' can't be read as a depth texture.

   In OpenGL:
      '_main' and '_main_ds' are flipped vertically when compared to other render targets.

/******************************************************************************/
void RendererClass::createShadowMap()
{
   SyncLocker locker(D._lock);

   // shadow maps
   D._shd_map_size_actual=Max(0, Min(D.shadowMapSize()*3, D.maxTexSize())/3);
   Int shd_map_w=D.shadowMapSizeActual()*2,
       shd_map_h=D.shadowMapSizeActual()*3;
#if DX9
  _shd_map_null.del();
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D32  , IMAGE_SHADOW_MAP, 1)) // on GeForce 650m this is not available, but try anyway
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D24X8, IMAGE_SHADOW_MAP, 1)) // we don't need stencil so avoid it in case it causes performance penalty
       _shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D24S8, IMAGE_SHADOW_MAP, 1);

   if( _shd_map.is())
   if(!_shd_map_null.createTryEx(_shd_map.w(), _shd_map.h(), 1, IMAGE_NULL, IMAGE_RT, 1, 1, IMAGE_B8G8R8A8))_shd_map.del();
#else
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D32  , IMAGE_SHADOW_MAP, 1)) // D32 shadow maps have no performance penalty (tested on GeForce 650m) so use them if possible
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D24X8, IMAGE_SHADOW_MAP, 1)) // we don't need stencil so avoid it in case it causes performance penalty
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D24S8, IMAGE_SHADOW_MAP, 1))
   if(!_shd_map.createTryEx(shd_map_w, shd_map_h, 1, IMAGE_D16  , IMAGE_SHADOW_MAP, 1)){}
#endif
   if(!_shd_map.is())D._shd_map_size_actual=0;

   // cloud shadow maps
#if DX9
   if(!_cld_map.createTryEx(D.cloudsMapSize()*2, D.cloudsMapSize()*3, 1, IMAGE_A8      , IMAGE_RT, 1, 1, IMAGE_L8A8    ))
       _cld_map.createTryEx(D.cloudsMapSize()*2, D.cloudsMapSize()*3, 1, IMAGE_B8G8R8A8, IMAGE_RT, 1, 1, IMAGE_R8G8B8A8);
#else
   if(!_cld_map.createTryEx(D.cloudsMapSize()*2, D.cloudsMapSize()*3, 1, IMAGE_R8      , IMAGE_RT, 1, 1, IMAGE_R8G8))
       _cld_map.createTry  (D.cloudsMapSize()*2, D.cloudsMapSize()*3, 1, IMAGE_R8G8B8A8, IMAGE_RT, 1);
#endif

   Sh.connectRT();
   D.shadowJitterSet();
}
void RendererClass::rtClear()
{
  _h0          .clear();
  _h1          .clear();
  _q0          .clear();
  _q1          .clear();
  _col         .clear();
  _nrm         .clear();
  _vel         .clear();
  _lum         .clear();
  _lum_1s      .clear();
  _shd_1s      .clear();
  _shd_ms      .clear();
  _ds          .clear();
  _ds_1s       .clear();
  _water_col   .clear();
  _water_nrm   .clear();
  _water_ds    .clear();
  _water_lum   .clear();
  _vol         .clear();
  _ao          .clear();
  _mirror_rt   .clear();
  _outline_rt  .clear();
  _sky_coverage.clear();
  _final       .clear();
   // don't clear '_back' and '_back_ds' here in case they are used
}
void RendererClass::rtClean()
{
   SyncLocker locker(D._lock);
   rtClear();
   REPA(_rts)if(_rts[i].available())_rts.removeValid(i);
}
void RendererClass::rtDel()
{
   SyncLocker locker(D._lock);

#if GL
   if(FBO) // detach all render targets
   {
      D.fbo(FBO); // set custom frame buffer
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D  , 0, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0   );
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0   );
   #if !IOS
    //D.fbo(0); // set default frame buffer (iOS doesn't have it) !! Don't do this, because we clear all attachments and '_cur' to null, we assume that no RT's are set, however that's possible only in a custom FBO, so we need to keep it, so that the next 'set' RT change will be able to properly detect the change !!
   #endif
   }
#endif

   rtClear();
   D.clearFade(); // _fade.clear(); this is already cleared in 'clearFade'
  _back   .clear();
  _back_ds.clear();

  _gui   =_cur_main   =&_main;
  _gui_ds=_cur_main_ds=&_main_ds;

   unmapMain();
#if DX9 || DX11 || IOS // only on these platforms we're creating custom '_main_ds', on other platforms the system creates it, so we're not deleting (to keep the info about IMAGE_TYPE and samples)
  _main_ds.del();
#endif
  _cld_map     .del();
  _shd_map     .del();
  _shd_map_null.del();
   REPAO(_eye_adapt_scale).del();
  _rts.clear();
   REPAO(_cur       )=null; _cur_ds   =null;
   REPAO(_cur_id    )=NULL; _cur_ds_id=NULL;
   REPAO(_cur_ds_ids)=NULL;
}
Bool RendererClass::rtCreate()
{
   if(LogInit)LogN("RendererClass.rtCreate");
   SyncLocker locker(D._lock);

   rtDel();
   ResetImageTypeCreateResult();

   if(!D.canDraw())return true; // don't bother with render targets if the device can't draw (can happen when using 'APP_ALLOW_NO_GPU')

   if(!mapMain())return false;

   // depth
#if DX9
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_INTZ , IMAGE_DS_RT, 1, _main.samples()))
 //if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_RAWZ , IMAGE_DS_RT, 1, _main.samples())) disable for now because all shader depth reads use standard depth format while RAWZ requires a special shader, enabling would require converting to IMAGE_F32 that mimics standard depth buffer, however that would destroy stencil and we would have to use 2 separate Images
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_DF24 , IMAGE_DS_RT, 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24S8, IMAGE_DS   , 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24X8, IMAGE_DS   , 1, _main.samples()))return false;
#elif DX11
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24S8, IMAGE_DS_RT, 1, _main.samples()))return false;
#elif IOS // on iOS we have access to '_main' so let's keep '_main_ds' the same
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24S8, IMAGE_DS_RT, 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D32  , IMAGE_DS_RT, 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24X8, IMAGE_DS_RT, 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D16  , IMAGE_DS_RT, 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24S8, IMAGE_DS   , 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D32  , IMAGE_DS   , 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D24X8, IMAGE_DS   , 1, _main.samples()))
   if(!_main_ds.createTryEx(_main.w(), _main.h(), 1, IMAGE_D16  , IMAGE_DS   , 1, _main.samples()))return false;
#else // other platforms have '_main_ds' linked with '_main' provided by the system
  _main_ds.forceInfo(_main.w(), _main.h(), 1, _main_ds.type() ? _main_ds.type() : IMAGE_D24S8, IMAGE_DS)._samples=_main.samples(); // if we know the type then use it, otherwise assume the default IMAGE_D24S8
#endif

   createShadowMap();

   // eye adaptation
  _eye_adapt_scale_cur=0; if(!_eye_adapt_scale[0].createTryEx(1, 1, 1, IMAGE_F32, IMAGE_RT, 1, 1, IMAGE_F16)
                          || !_eye_adapt_scale[1].createTryEx(1, 1, 1, IMAGE_F32, IMAGE_RT, 1, 1, IMAGE_F16))REPAO(_eye_adapt_scale).del();

   setMain();

   Sh.connectRT();
   return true;
}
/******************************************************************************/
void RendererClass::update()
{
   if(_t_measure)
   {
      Dbl t=Time.curTime();
      if( t>_t_last_measure+1)
      {
         Flt mul=1.0f/_t_measures[0];

        _t_reflection  [0]=_t_reflection  [1]*mul;            _t_reflection  [1]=0;
        _t_prepare     [0]=_t_prepare     [1]*mul;            _t_prepare     [1]=0;
        _t_solid       [0]=_t_solid       [1]*mul;            _t_solid       [1]=0;
        _t_overlay     [0]=_t_overlay     [1]*mul;            _t_overlay     [1]=0;
        _t_water       [0]=_t_water       [1]*mul;            _t_water       [1]=0;
        _t_light       [0]=_t_light       [1]*mul;            _t_light       [1]=0;
        _t_sky         [0]=_t_sky         [1]*mul;            _t_sky         [1]=0;
        _t_blend       [0]=_t_blend       [1]*mul;            _t_blend       [1]=0;
        _t_palette     [0]=_t_palette     [1]*mul;            _t_palette     [1]=0;
        _t_behind      [0]=_t_behind      [1]*mul;            _t_behind      [1]=0;
        _t_rays        [0]=_t_rays        [1]*mul;            _t_rays        [1]=0;
        _t_refract     [0]=_t_refract     [1]*mul;            _t_refract     [1]=0;
        _t_volumetric  [0]=_t_volumetric  [1]*mul;            _t_volumetric  [1]=0;
        _t_post_process[0]=_t_post_process[1]*mul;            _t_post_process[1]=0;
        _t_gpu_wait    [0]=_t_gpu_wait    [1]/_t_measures[1]; _t_gpu_wait    [1]=0; // '_t_gpu_wait' has it's own counter (_t_measures[1]) because it's called once per frame, while others can be called multiple times per frame

        _t_last_measure=t;
        _t_measures[0]=0;
        _t_measures[1]=0;
      }
   }
}
/******************************************************************************/
void RendererClass::setMain() // !! requires 'D._lock' !! this is called after RT creation, and when VR GuiTexture is created/deleted/changed, and at the end of frame drawing for stereo mode (to advance to the next VR frame)
{
#if DX12
   map needs to be called for all images, cache the values, and call setMain in every frame
#endif
   if(VR.active() && (_gui=VR.getNewGui()))
   {
     _gui_ds.getDS(_gui->w(), _gui->h());
   }else
   {
     _gui   =&_main;
     _gui_ds=&_main_ds;
   }
  _cur_main   =_gui   .rc();
  _cur_main_ds=_gui_ds.rc();

   set(_cur_main, _cur_main_ds, false);
}
Bool RendererClass::mapMain()
{
   return _main.map();
}
void RendererClass::unmapMain()
{
  _main.unmap();
  _cur   [0]=null;
  _cur_id[0]=NULL;
}
/******************************************************************************/
Rect RendererClass::colClamp(C VecI2 &size)
{
   Rect r((D.viewRect().min.x+D.w())*size.x/D.w2(), (D.h()-D.viewRect().max.y)*size.y/D.h2(),
          (D.viewRect().max.x+D.w())*size.x/D.w2(), (D.h()-D.viewRect().min.y)*size.y/D.h2());
   RectI ri=RoundGPU(r);
   r.min=(ri.min+0.5f)/size; // yes +0.5 is needed
   r.max=(ri.max-0.5f)/size; // yes -0.5 is needed
   return r;
}
Rect RendererClass::screenToPixel(C Rect &screen)
{
   return Rect((screen.min.x+D.w())*Renderer.resW()/D.w2(), (D.h()-screen.max.y)*Renderer.resH()/D.h2(),
               (screen.max.x+D.w())*Renderer.resW()/D.w2(), (D.h()-screen.min.y)*Renderer.resH()/D.h2());
}
RectI RendererClass::screenToPixelI(C Rect &screen)
{
   return RoundGPU(screenToPixel(screen));
}

Rect RendererClass::pixelToScreen(C RectI &pixel)
{
   return Rect(pixel.min.x*D.w2()/Renderer.resW()-D.w(), D.h()-pixel.max.y*D.h2()/Renderer.resH(),
               pixel.max.x*D.w2()/Renderer.resW()-D.w(), D.h()-pixel.min.y*D.h2()/Renderer.resH());
}
Vec2 RendererClass::pixelToScreenSize(Flt pixel)
{
   return Vec2(pixel*D.w2()/Renderer.resW(),
               pixel*D.h2()/Renderer.resH());
}
Vec2 RendererClass::screenToPixelSize(C Vec2 &screen)
{
   return Vec2(screen.x*Renderer.resW()/D.w2(),
               screen.y*Renderer.resH()/D.h2());
}
/******************************************************************************/
#if GL
static void SwitchedFBO()
{
   // update settings that depend on main FBO being active 
   D.cullGL     (); // adjust culling according to Y axis
   SetProjMatrix(); // flip Y 3D coords when Rendering To Texture
}
static inline Bool EqualRT(C Image *a, C Image *b)
{
   UInt a_txtr, a_rb; if(a){a_txtr=a->_txtr; a_rb=a->_rb;}else a_txtr=a_rb=0;
   UInt b_txtr, b_rb; if(b){b_txtr=b->_txtr; b_rb=b->_rb;}else b_txtr=b_rb=0;
   return a_txtr==b_txtr && a_rb==b_rb;
}
static inline Bool EqualDS(C Image *a, C Image *b, UInt a_txtr)
{
   UInt         a_rb; if(a){                 a_rb=a->_rb;}else        a_rb=0;
   UInt b_txtr, b_rb; if(b){b_txtr=b->_txtr; b_rb=b->_rb;}else b_txtr=b_rb=0;
   return a_txtr==b_txtr && a_rb==b_rb;
}
static Bool EqualTxtr(C Image *a, C Image *b) {return (a ? a->_txtr : 0)==(b ? b->_txtr : 0);} // simpler version that checks texture ID's only, this can be used for #1+ RT's which never use RenderBuffers but only textures
#endif
#if DX9
void RendererClass::setCube(Image &cube, Image *ds, DIR_ENUM dir)
{
   if(cube._cube)
   {
   #if GL
      Bool was_main_fbo=D.mainFBO();
   #endif
      D3DCUBEMAP_FACES cf;
      switch(dir)
      {
         case DIR_LEFT : cf=D3DCUBEMAP_FACE_NEGATIVE_X; break;
         case DIR_RIGHT: cf=D3DCUBEMAP_FACE_POSITIVE_X; break;
         case DIR_DOWN : cf=D3DCUBEMAP_FACE_NEGATIVE_Y; break;
         case DIR_UP   : cf=D3DCUBEMAP_FACE_POSITIVE_Y; break;
         case DIR_BACK : cf=D3DCUBEMAP_FACE_NEGATIVE_Z; break;
         default       : cf=D3DCUBEMAP_FACE_POSITIVE_Z; break;
      }
      IDirect3DSurface9 *surf=null; cube._cube->GetCubeMapSurface(cf, 0, &surf);
     _cur   [0]=&cube;
     _cur_id[0]= surf;
      D3D->SetRenderTarget(0, surf);
      RELEASE(surf);

      IDirect3DSurface9 *ids=(ds ? ds->_surf : null);
      if(_cur_ds_id!=ids){_cur_ds=ds; D3D->SetDepthStencilSurface(_cur_ds_id=ids);}

     _res.set(cube.w(), cube.h());
      if(Sh.h_RTSizeI)Sh.h_RTSizeI->set(Vec2(_res));
   #if DX9
      D.viewportForce(RectI(0, 0, resW(), resH())); // DX9 automatically sets full viewport on RT change, force current viewport values
   #elif GL
      if(was_main_fbo!=D.mainFBO())SwitchedFBO();
   #endif
      D._view_active.setRect(RectI(0, 0, resW(), resH())).setViewport(); // set full viewport
      D.clipAllow(_cur[0]==_cur_main);
   }
}
#endif
#define R Renderer
#if DX11
void RendererClass::setDSLookup()
{
   if(R._cur_ds)
   {
      R._cur_ds_ids[  NO_DEPTH_READ]=R._cur_ds-> _dsv;
      R._cur_ds_ids[NEED_DEPTH_READ]=R._cur_ds->_rdsv;
      R._cur_ds_ids[WANT_DEPTH_READ]=R._cur_ds_ids[R._cur_ds_ids[NEED_DEPTH_READ] ? NEED_DEPTH_READ : NO_DEPTH_READ]; // use RDSV if available, if not then DSV
   }else
   {
      R._cur_ds_ids[  NO_DEPTH_READ]=null;
      R._cur_ds_ids[WANT_DEPTH_READ]=null;
      R._cur_ds_ids[NEED_DEPTH_READ]=null;
   }
}
void RendererClass::setDS(ID3D11DepthStencilView *dsv)
{
   if(R._cur_ds_id!=dsv)
   {
      if(dsv==R._cur_ds_ids[NO_DEPTH_READ])D.texClear(R._cur_ds->_srv); // if we're writing to depth then we need to unbind it from reading (because DirectX will do it)
      D3DC->OMSetRenderTargets(Elms(R._cur_id), R._cur_id, R._cur_ds_id=dsv);
   }
}
void RendererClass::needDepthTest() {if(D._depth_write || !R._cur_ds_id)setDS(R._cur_ds_ids[  NO_DEPTH_READ]);}
void RendererClass::wantDepthRead() {                                   setDS(R._cur_ds_ids[WANT_DEPTH_READ]);}
void RendererClass::needDepthRead() {                                   setDS(R._cur_ds_ids[NEED_DEPTH_READ]);}
#elif WEB
void RendererClass::setDSLookup()
{
 /*R._cur_ds_ids[WANT_DEPTH_READ]=*/R._cur_ds_ids[NO_DEPTH_READ]=(R._cur_ds ? R._cur_ds->_txtr : 0); // WANT_DEPTH_READ will always be the same as NO_DEPTH_READ so never use it
 //R._cur_ds_ids[NEED_DEPTH_READ]=NULL; this will always be null so no need to change it, this is already cleared at startup, besides it's never accessed anyway
}
void RendererClass::setDS(UInt ds_txtr_id)
{
   if(R._cur_ds_id!=ds_txtr_id)
   {
      R._cur_ds_id=ds_txtr_id;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D, ds_txtr_id, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ds_txtr_id, 0); // this could fail if Image doesn't have stencil component
   }
}
void RendererClass::needDepthTest() {setDS(R._cur_ds_ids[NO_DEPTH_READ]);}
#endif
#undef R
void RendererClass::set(Image *t0, Image *t1, Image *t2, Image *t3, Image *ds, Bool custom_viewport, DEPTH_READ_MODE depth_read_mode)
{
   Int changed=0;
#if DX9
   enum
   {
      RT0=0x1,
      RTN=0x2,
      DS =0x4,
   };
   IDirect3DSurface9 *id0=(t0 ? t0->_surf : null),
                     *id1=(t1 ? t1->_surf : null),
                     *id2=(t2 ? t2->_surf : null),
                     *id3=(t3 ? t3->_surf : null),
                     *ids=(ds ? ds->_surf : null);

   // a render target can't be attached to multiple slots, so that's why 1-2-3 slots need first to be detached
   if(_cur_id[1]!=id1 || _cur_id[2]!=id2 || _cur_id[3]!=id3)
   {
      if(_cur_id[3]){_cur[3]=null; changed|=RTN; D3D->SetRenderTarget(3, _cur_id[3]=null);}
      if(_cur_id[2]){_cur[2]=null; changed|=RTN; D3D->SetRenderTarget(2, _cur_id[2]=null);}
      if(_cur_id[1]){_cur[1]=null; changed|=RTN; D3D->SetRenderTarget(1, _cur_id[1]=null);}
   }

   if(_cur_id[0]!=id0){_cur[0]=t0; changed|=RT0; D3D->SetRenderTarget       (0, _cur_id[0]=id0);}
   if(_cur_id[1]!=id1){_cur[1]=t1; changed|=RTN; D3D->SetRenderTarget       (1, _cur_id[1]=id1);}
   if(_cur_id[2]!=id2){_cur[2]=t2; changed|=RTN; D3D->SetRenderTarget       (2, _cur_id[2]=id2);}
   if(_cur_id[3]!=id3){_cur[3]=t3; changed|=RTN; D3D->SetRenderTarget       (3, _cur_id[3]=id3);}
   if(_cur_ds_id!=ids){_cur_ds=ds; changed|=DS ; D3D->SetDepthStencilSurface(   _cur_ds_id=ids);}
#elif DX11
   ID3D11RenderTargetView *id0=(t0 ? t0->_rtv : null),
                          *id1=(t1 ? t1->_rtv : null),
                          *id2=(t2 ? t2->_rtv : null),
                          *id3=(t3 ? t3->_rtv : null);
   ID3D11DepthStencilView *ids=(ds ? (depth_read_mode==NEED_DEPTH_READ
                                   || depth_read_mode==WANT_DEPTH_READ && ds->_rdsv) ? ds->_rdsv : ds->_dsv : null);

   if(_cur_id[0]!=id0 || _cur_id[1]!=id1 || _cur_id[2]!=id2 || _cur_id[3]!=id3 || _cur_ds_id!=ids)
   {
      if(id0 &&                  _cur_id[0]!=id0)D.texClear(t0->_srv);
      if(id1 &&                  _cur_id[1]!=id1)D.texClear(t1->_srv);
      if(id2 &&                  _cur_id[2]!=id2)D.texClear(t2->_srv);
      if(id3 &&                  _cur_id[3]!=id3)D.texClear(t3->_srv);
      if(ids && ids==ds->_dsv && _cur_ds_id!=ids)D.texClear(ds->_srv); // if we're writing to depth then we need to unbind it from reading (because DirectX will do it)

      changed=true;
     _cur[0]=t0; _cur_id[0]=id0;
     _cur[1]=t1; _cur_id[1]=id1;
     _cur[2]=t2; _cur_id[2]=id2;
     _cur[3]=t3; _cur_id[3]=id3;
     _cur_ds=ds; _cur_ds_id=ids;
      D3DC->OMSetRenderTargets(Elms(_cur_id), _cur_id, _cur_ds_id); ASSERT(ELMS(_cur_id)<=D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
   }else
   if(_cur_ds!=ds) // even if we're not changing RenderTargetView it's still possible we're changing RenderTarget Image, this can happen on DX10 when using NEED_DEPTH_READ, '_rdsv' null and last '_cur_ds_id' also null
   {
      changed=true;
     _cur_ds =ds;
   }
#elif GL
   // !! '_cur_id' is not set for GL, only '_cur_ds_id' is !!
   Image    *set_ds =((WEB && depth_read_mode==NEED_DEPTH_READ) ? null : ds); // if we require reading from the depth buffer, then we can't set it
   Bool was_main_fbo=D.mainFBO(),
            main_fbo=(t0==&_main || ds==&_main_ds), // check 'ds' and not 'set_ds' !!
          change_0, change_ds;
   if(main_fbo)
   {
      change_0 =((_cur[0]==&_main   )!=(t0==&_main   ));
      change_ds=((_cur_ds==&_main_ds)!=(ds==&_main_ds)); // check 'ds' and not 'set_ds' !!
   }else
   {
      change_0 =!EqualRT(_cur[0], t0);
      change_ds=!EqualDS(_cur_ds, set_ds, _cur_ds_id);
   }

   if(main_fbo!=was_main_fbo || change_0 || change_ds || !EqualTxtr(_cur[1], t1) || !EqualTxtr(_cur[2], t2) || !EqualTxtr(_cur[3], t3))
   {
   #if !IOS // there is no default frame buffer on iOS
      D.colWriteAllow((main_fbo && t0!=&_main) ? 0 : COL_WRITE_RGBA); // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting null RT and '_main_ds' DS, '_main' is set either way but with color writes disabled
      D.   depthAllow(!main_fbo || ds==&_main_ds); // check 'ds' and not 'set_ds' !!
      if(main_fbo)
      {
         D.fbo(0); // set default frame buffer
        _cur_ds_id=0; // main FBO always has 0 depth txtr ID
      #if GL_ES // do this only on GLES, because on desktop it requires GL 4.3 TODO:
         if(D.notShaderModelGLES2()) // this check is needed because on GLES2 we don't have 'glInvalidateFramebuffer'
         {
            // discard, for main FBO we need to setup different values - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glInvalidateFramebuffer.xhtml
            GLenum attachment[3]; GLsizei attachments=0; // RT0+Depth+Stencil
            if(_main   ._discard){_main   ._discard=false; attachment[attachments++]=GL_COLOR;}
            if(_main_ds._discard){_main_ds._discard=false; attachment[attachments++]=GL_DEPTH; if(ImageTI[_main_ds.hwType()].s)attachment[attachments++]=GL_STENCIL;}
            if(attachments)glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments, attachment);
         }
      #endif
      }else
   #endif
      {
      #if !IOS // on iOS there's only one FBO and nothing else, so it is set during creation and not here
         D.fbo(FBO); // set custom frame buffer
      #endif

      #if IOS // on iOS there's only one FBO, so we can safely apply only changed RT's, otherwise we need to always set all of them
         if(change_ds)
      #endif
         {
            if(!set_ds)
            {
              _cur_ds_id=0;
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0);
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            }else
            if(_cur_ds_id=set_ds->_txtr)
            {
               glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D,                               _cur_ds_id    , 0);
               glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ImageTI[set_ds->hwType()].s ? _cur_ds_id : 0, 0);
            }else
            {
            //_cur_ds_id=0; already set above
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER,                               set_ds->_rb    );
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ImageTI[set_ds->hwType()].s ? set_ds->_rb : 0);
            }
         }

      #if IOS // on iOS there's only one FBO, so we can safely apply only changed RT's, otherwise we need to always set all of them
         if(change_0)
      #endif
         {
            if(!t0       )glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,            0);else
            if( t0->_txtr)glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D  , t0->_txtr, 0);else
	                       glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t0->_rb     );
         }

         if(D.notShaderModelGLES2()) // this check is needed because on GLES2 we don't have MRT, and there's no 'glInvalidateFramebuffer'
         {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, t1 ? t1->_txtr : 0, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, t2 ? t2->_txtr : 0, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, t3 ? t3->_txtr : 0, 0);

            GLenum buffers[]={GLenum(t0 ? GL_COLOR_ATTACHMENT0 : GL_NONE),
                              GLenum(t1 ? GL_COLOR_ATTACHMENT1 : GL_NONE),
                              GLenum(t2 ? GL_COLOR_ATTACHMENT2 : GL_NONE),
                              GLenum(t3 ? GL_COLOR_ATTACHMENT3 : GL_NONE)};
            glDrawBuffers(Elms(buffers), buffers);
            glReadBuffer (buffers[0]);

            // discard
         #if GL_ES // do this only on GLES, because on desktop it requires GL 4.3 TODO:
            GLenum attachment[ELMS(_cur)+1]; GLsizei attachments=0; // RT's+DS
            if(t0     &&     t0->_discard){    t0->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT0;}
            if(t1     &&     t1->_discard){    t1->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT1;}
            if(t2     &&     t2->_discard){    t2->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT2;}
            if(t3     &&     t3->_discard){    t3->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT3;}
            if(set_ds && set_ds->_discard){set_ds->_discard=false; attachment[attachments++]=(ImageTI[set_ds->hwType()].s ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT);}
            if(attachments)glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments, attachment);
         #endif
         }

      #if DEBUG
         GLenum error =glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if(    error!=GL_FRAMEBUFFER_COMPLETE)
         {
            LogN(S+"Invalid OpenGL FramebufferStatus: "+error+", frame:"+Time.frame()+", t0:"+(t0 ? t0->w() : 0)+", ds:"+(ds ? ds->w() : 0));
            Break();
            switch(error)
            {
               case GL_FRAMEBUFFER_UNSUPPORTED                  : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT        : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: error=0; break;
            #if GL_ES
               case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS        : error=0; break;
            #else
               case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER       : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER       : error=0; break;
            #endif
            }
         }
      #endif
      }

     _cur[0] =t0;
     _cur[1] =t1;
     _cur[2] =t2;
     _cur[3] =t3;
     _cur_ds =ds;
      changed=true;
   }else
   if(_cur_ds!=ds) // check this in case 'set_ds' was cleared to null
   {
     _cur_ds =ds;
      changed=true;
   #if !IOS // there is no default frame buffer on iOS
      D.depthAllow(!main_fbo || ds==&_main_ds); // check 'ds' and not 'set_ds' !!
   #endif
   }
#endif

   if(changed)
   {
      if(Image *main=(t0 ? t0 : ds))
      {
        _res=main->size();
         if(Sh.h_RTSizeI)Sh.h_RTSizeI->setConditional(_res);
      }

   #if DX9
      if(changed&RT0) // DX9 automatically performs some changes on RT #0 change (but not on the others, this was tested)
      {
         RectI full(0, 0, resW(), resH());
         D.viewportForce(full); // DX9 automatically sets full viewport, so we need to adjust cached viewport values to math those of the DX9
         D.    clipForce(full); // DX9 automatically sets full clipping (scissor rect)
      }
   #elif GL
      if(was_main_fbo!=main_fbo)SwitchedFBO();
   #endif
      D._view_active.setRect(custom_viewport ? screenToPixelI(D.viewRect()) : RectI(0, 0, resW(), resH())).setViewport();
      D.clipAllow(_cur[0]==_cur_main);
      D.validateCoords(); // viewport was changed, also for DX9 (pixel offset) and OpenGL (flip Y 2D coords when Rendering To Texture)
   }else // render targets weren't changed, so set viewport only
   {
      RectI rect(custom_viewport ? screenToPixelI(D.viewRect()) : RectI(0, 0, resW(), resH()));
      if(   rect!=D._view_active.recti) // over here we can do a quick != check first, because the Render Targets haven't changed (Renderer.resW(), resH() is the same, and that affects 'setRect')
      {
         D._view_active.setRect(rect).setViewport();
         D.validateCoords(); // viewport was changed, also for DX9 (pixel offset) and OpenGL (flip Y 2D coords when Rendering To Texture)
      }
   }
}
/******************************************************************************/
void RendererClass::setMainViewport()
{
   if(_stereo)
   {
      D._view_active.setRect(Renderer.screenToPixelI(D._view_rect)).setViewport(false).setShader();
      SetProjMatrix();
      SetCam(ActiveCam.matrix, false);
      D.validateCoords();
      D.setViewFovTan();
   }
}
void RendererClass::setEyeViewport()
{
   if(_stereo)
   {
      D._view_active.setRect(Renderer.screenToPixelI(D._view_eye_rect[_eye])).setViewport(false).setShader(&ProjMatrixEyeOffset[_eye]); // 'setShader' needed for 'PosToScreen' and 'fur'
      SetProjMatrix(ProjMatrixEyeOffset[_eye]);
      SetCam(EyeMatrix[_eye], false);
      D.validateCoords(_eye);
      D.setViewFovTan();
   }
}
Rect* RendererClass::setEyeParams()
{
   if(_stereo)
   {
      RectI rect=D._view_active.recti;
      D._view_active.setRect(Renderer.screenToPixelI(D._view_eye_rect[_eye])).setShader(&ProjMatrixEyeOffset[_eye]).setRect(rect);
      return &D._view_eye_rect[_eye];
   }
   return &D._view_rect;
}
/******************************************************************************/
void RendererClass::     hasGlow() {_has_glow=true;}
void RendererClass::finalizeGlow()
{
   if(!ImageTI[_col->type()].a || !D.glowAllow() || !D.bloomAllow() || fastCombine())_has_glow=false; // glow can be done only if we have Alpha Channel in the RT, if we're allowing bloom processing (because it'd done together in the same shader), if we're allowing glow, and if 'fastCombine' is not active
}
/******************************************************************************/
Bool RendererClass::capture(Image &image, Int w, Int h, Int type, Int mode, Int mip_maps, Bool alpha)
{
   if(image.capture(_main))
   {
      if(type<=0)type=image.type();else MIN(type, IMAGE_TYPES);
      if(!_ds_1s)alpha=false;

      if(ImageTI[type].a && ImageTI[image.type()].a && !alpha && image.lock()) // dest has alpha and src has alpha, and don't want to manually set alpha
      {
         REPD(y, image.h())
         REPD(x, image.w())
         {
            Color color=image.color(x, y); color.a=255; // force full alpha
                        image.color(x, y,  color);
         }
         image.unlock();
      }

      if(image.copyTry(image, w, h, 1, type, mode, mip_maps))
      {
         if(alpha && ImageTI[image.type()].a && image.lock()) // set alpha from depth
         {
            Image depth; if(depth.capture(*_ds_1s) && depth.lockRead())
            {
               Image alpha(depth.w(), depth.h(), 1, IMAGE_A8, IMAGE_SOFT, 1);
               REPD(y, depth.h())
               REPD(x, depth.w())
               {
                  Flt w=depth.pixelF(x, y);
               #if REVERSE_DEPTH
                  alpha.pixB(x, y)=((w>0.0f) ? 0xFF : 0);
               #else
                  alpha.pixB(x, y)=((w<1.0f) ? 0xFF : 0);
               #endif
               }
               depth.unlock();

               alpha.resize(image.w(), image.h(), FILTER_LINEAR);
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Color color=image.color(x, y); color.a=alpha.pixB(x, y);
                              image.color(x, y,  color);
               }
            }else
            {
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Color color=image.color(x, y); color.a=255; // force full alpha
                              image.color(x, y,  color);
               }
            }
            image.unlock().updateMipMaps();
         }
         return true;
      }
   }
   image.del(); return false;
}
Bool RendererClass::screenShot(C Str &name, Bool alpha)
{
   FCreateDirs(GetPath(name));
   Image temp;
   if(alpha) // with alpha
   {
      if(capture(temp, -1, -1, IMAGE_DEFAULT, IMAGE_SOFT, 1, true))return temp.Export(name);
   }else
   if(temp.capture(_main)) // no alpha
   {
      if(ImageTI[temp.type()].a)temp.copyTry(temp, -1, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT, 1); // if captured image has alpha channel then let's remove it
      return temp.Export(name);
   }
   return false;
}
Bool RendererClass::screenShots(C Str &name, C Str &ext, Bool alpha)
{
   Str n=FFirst(name, ext);
   return n.is() ? screenShot(n, alpha) : false;
}
/******************************************************************************/
void RendererClass::timeMeasure(Bool on)
{
   if(_t_measure!=on)
   {
     _t_measure=on;
     _t_last_measure=Time.curTime();
   }
}
/******************************************************************************/
}
/******************************************************************************/
