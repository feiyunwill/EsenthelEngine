/******************************************************************************/
#include "stdafx.h"
#define SUPPORT_OCULUS (WINDOWS_OLD && DX11)

#if SUPPORT_OCULUS
   #include "../../../ThirdPartyLibs/begin.h"
 //#include "../../../ThirdPartyLibs/Oculus/LibOVR/Include/OVR.h"
   #include "../../../ThirdPartyLibs/Oculus/LibOVR/Include/OVR_CAPI.h"
#if DX11
   #include "../../../ThirdPartyLibs/Oculus/LibOVR/Include/OVR_CAPI_D3D.h"
#elif GL
   #include "../../../ThirdPartyLibs/Oculus/LibOVR/Include/OVR_CAPI_GL.h"
#endif
   #include "../../../ThirdPartyLibs/end.h"
#endif
/******************************************************************************/
namespace EE{
/******************************************************************************/
static struct OculusRiftApi : VirtualRealityApi
{
   virtual Bool init()override;
   virtual void shut()override;

   virtual Bool   active         ()C override;
   virtual Matrix matrixCur      ()C override;
   virtual void   recenter       ()  override;
   virtual void   changedGuiDepth()  override;
   virtual void   changedGuiSize ()  override;
   virtual void   update         ()  override;
   virtual void   draw           ()  override;

   virtual void          delImages()override;
   virtual Bool    createGuiImage ()override;
   virtual Bool createRenderImage ()override;

   virtual ImageRC* getNewRender ()override;
   virtual ImageRC* getNewGui    ()override;
   virtual ImageRC* getLastRender()override;
   virtual ImageRC* getLastGui   ()override;

   Bool    connect();
   void disconnect();

   void setGuiSizeX();
   void setGuiSize ();

   OculusRiftApi();

#if SUPPORT_OCULUS
//private:
   Bool           _initialized;
   ovrSession     _session;
   ovrLayerEyeFov _layer_render;
   ovrLayerQuad   _layer_gui;
#endif
}OculusRift;
/******************************************************************************/
#if SUPPORT_OCULUS
static struct ovrTexture
{
   ImageRC            image;
   ovrSwapTextureSet *texture_set;
#if DX11
   ID3D11RenderTargetView   *rtv[3];
   ID3D11ShaderResourceView *srv[3];
#endif

  ~ovrTexture() {del();}
   ovrTexture()
   {
      texture_set=null;
   #if DX11
      REPAO(rtv)=null;
      REPAO(srv)=null;
   #endif
   }
   void del()
   {
      image.zero(); // !! zero without deleting, because members were just copied !!
   #if DX11
      if(D.created())
      {
         REPA(rtv)RELEASE(rtv[i]);
         REPA(srv)RELEASE(srv[i]);
      }else
      {
         REPAO(rtv)=null;
         REPAO(srv)=null;
      }
   #endif
      if(OculusRift._session)ovr_DestroySwapTextureSet(OculusRift._session, texture_set); texture_set=null;
   }
   Bool create(Int w, Int h)
   {
      del();

      // TODO: support gamma correct sRGB rendering
      image._size=image._hw_size.set(w, h, 1);
      image._type=image._hw_type=IMAGE_R8G8B8A8;
      image._mode=IMAGE_RT;
      image._mms=1;
      image._samples=1;
      image._byte_pp=ImageTI[image.type()].byte_pp;
      image. setPartial();

   #if DX11
      D3D11_TEXTURE2D_DESC desc;
      desc.Width             =image.w();
      desc.Height            =image.h();
      desc.MipLevels         =1;
      desc.Format            =DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      desc.Usage             =D3D11_USAGE_DEFAULT;
      desc.BindFlags         =D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
      desc.MiscFlags         =0;
      desc.CPUAccessFlags    =0;
      desc.SampleDesc.Count  =1;
      desc.SampleDesc.Quality=0;
      desc.ArraySize         =1;
    //SyncLockerEx locker(D._lock); lock not needed for DX11 'D3D'
      if(ovr_CreateSwapTextureSetD3D11(OculusRift._session, D3D, &desc, ovrSwapTextureSetD3D11_Typeless, &texture_set)==ovrSuccess)
      {
         if  (texture_set->TextureCount>Elms(rtv))Exit("Oculus Rift Swap Chain has too many textures");
         FREP(texture_set->TextureCount)
         {
            ovrD3D11Texture &tex=(ovrD3D11Texture&)texture_set->Textures[i];

            D3D11_RENDER_TARGET_VIEW_DESC rtvd; Zero(rtvd);
            rtvd.Format       =DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvd.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
            if(!OK(D3D->CreateRenderTargetView(tex.D3D11.pTexture, &rtvd, &rtv[i])))goto error;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvd; Zero(srvd);
            srvd.Format             =DXGI_FORMAT_R8G8B8A8_UNORM;
            srvd.ViewDimension      =D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels=1;
            if(!OK(D3D->CreateShaderResourceView(tex.D3D11.pTexture, &srvd, &srv[i])))goto error;
         }
         return true;
      }
   #endif

   error:
      del(); return false;
   }
   ImageRC* getImage()
   {
      if(texture_set)
      {
         if( ++texture_set->CurrentIndex>=texture_set->TextureCount)texture_set->CurrentIndex=0;
         Int i=texture_set->CurrentIndex;
         image.unlock(); // unlock if it was locked by the user
      #if DX11
         ovrD3D11Texture &tex=(ovrD3D11Texture&)texture_set->Textures[i];
         image._txtr=tex.D3D11.pTexture;
         image._rtv =rtv[i];
         image._srv =srv[i];
      #endif
         return &image;
      }
      return null;
   }
}RenderTexture, GuiTexture;

static void SetPose(Matrix &m, C ovrPosef &pose)
{
   ASSERT(SIZE(pose.Orientation)==SIZE(Quaternion)); Quaternion q=(Quaternion&)pose.Orientation; CHS(q.z); m.orn()=q;
   ASSERT(SIZE(pose.Position   )==SIZE(Vec       )); m.pos.set(pose.Position.x, pose.Position.y, -pose.Position.z); // Oculus is right-handed
}
static void SetPose(ovrPosef &pose, C Matrix &m)
{
   ASSERT(SIZE(pose.Orientation)==SIZE(Quaternion)); (Quaternion&)pose.Orientation=Quaternion(m); CHS(pose.Orientation.z);
   ASSERT(SIZE(pose.Position   )==SIZE(Vec       )); ((Vec&)pose.Position).set(m.pos.x, m.pos.y, -m.pos.z); // Oculus is right-handed
}
void OculusRiftApi::disconnect()
{
   VR.delImages(); // !! need to call 'VR.delImages' and not 'T.delImages' !!
   if(_session)
   {
      ovr_Destroy(_session); _session=null;
      VR.disconnected(); // call 'disconnected' after clearing '_session' so 'VR.active' is false (needed for some things including 'D.setSync')
   }
}
Bool OculusRiftApi::connect()
{
   disconnect();

   ASSERT(SIZE(VR._adapter_id)==SIZE(ovrGraphicsLuid));
   if(ovr_Create(&_session, (ovrGraphicsLuid*)&VR._adapter_id)==ovrSuccess)
   {
      D.clearFade();

      ovrHmdDesc desc=ovr_GetHmdDesc(_session);

      Set(VR._name, desc.ProductName);
      VR._refresh=desc.DisplayRefreshRate;
      VR._res.set(desc.Resolution.w, desc.Resolution.h);

      VR.setFOVTan(desc.DefaultEyeFov[0].LeftTan, desc.DefaultEyeFov[0].RightTan, desc.DefaultEyeFov[0].UpTan, desc.DefaultEyeFov[0].DownTan);

      ovrEyeRenderDesc eye_render_desc[2]; REPAO(eye_render_desc)=ovr_GetRenderDesc(_session, ovrEyeType(i), desc.DefaultEyeFov[i]);
      ASSERT(SIZE(eye_render_desc[0].HmdToEyeViewOffset)==SIZE(Vec));
      VR._eye_dist=Dist((Vec&)eye_render_desc[0].HmdToEyeViewOffset,
                        (Vec&)eye_render_desc[1].HmdToEyeViewOffset);

      REP(2)_layer_render.Fov[i]=desc.DefaultEyeFov[i]; // eyes

      return VR.connected(); // call 'connected' after having set '_session' so 'VR.active' is true (needed for some things including 'D.setSync')
   }
   return false;
}
#endif
/******************************************************************************/
OculusRiftApi::OculusRiftApi()
{
#if SUPPORT_OCULUS
  _initialized=false;

  _session=null;

   Zero(_layer_render);
  _layer_render.Header.Type =ovrLayerType_EyeFov;
  _layer_render.Header.Flags=0;

   Zero(_layer_gui);
  _layer_gui.Header.Type =ovrLayerType_Quad;
  _layer_gui.Header.Flags=ovrLayerFlag_HeadLocked;
   Matrix m; m.identity(); SetPose(_layer_gui.QuadPoseCenter, m);
  _layer_gui.QuadPoseCenter.Position.z=-1; // Oculus is right-handed
  _layer_gui.QuadSize.y=1;
#endif
}
/******************************************************************************/
Bool VirtualReality::OculusRiftInit    () {return VR.init(OculusRift);}
Bool VirtualReality::OculusRiftDetected()C
{
#if SUPPORT_OCULUS
   return ovr_Detect(0).IsOculusHMDConnected!=ovrFalse; // according to headers, this can be called before 'ovr_Initialize'
#else
   return false;
#endif
}
Bool OculusRiftApi::active()C
{
#if SUPPORT_OCULUS
   return _session!=null;
#else
   return false;
#endif
}
/******************************************************************************/
Bool OculusRiftApi::init()
{
#if SUPPORT_OCULUS
   if(!_initialized && ovr_Initialize(null)==ovrSuccess)
   {
     _initialized=true;
      connect();
   }
   return _initialized;
#endif
   return false;
}
void OculusRiftApi::shut()
{
#if SUPPORT_OCULUS
   if(_initialized)
   {
      disconnect();
     _initialized=false;
      ovr_Shutdown();
   }
#endif
}
/******************************************************************************/
Matrix OculusRiftApi::matrixCur()C
{
#if SUPPORT_OCULUS
   if(_session)
   {
      ovrTrackingState ts=ovr_GetTrackingState(_session, 0, ovrFalse); // according to headers, "time 0" will get "the most recent sensor reading"
      if(ts.StatusFlags&(ovrStatus_OrientationTracked|ovrStatus_PositionTracked))
      {
         Matrix m; SetPose(m, ts.HeadPose.ThePose);
         return m;
      }
   }
#endif
   return VR._matrix; // return last known matrix
}
/******************************************************************************/
void OculusRiftApi::setGuiSizeX()
{
#if SUPPORT_OCULUS
  _layer_gui.QuadSize.x=_layer_gui.QuadSize.y*GuiTexture.image.aspect();
#endif
}
void OculusRiftApi::setGuiSize()
{
#if SUPPORT_OCULUS
  _layer_gui.QuadSize.y=VR._gui_size*-_layer_gui.QuadPoseCenter.Position.z; // Oculus is right-handed
#endif
   setGuiSizeX();
}
void OculusRiftApi::changedGuiDepth()
{
#if SUPPORT_OCULUS
  _layer_gui.QuadPoseCenter.Position.z=-VR.guiDepth(); // Oculus is right-handed
   setGuiSize();
#endif
}
void OculusRiftApi::changedGuiSize()
{
#if SUPPORT_OCULUS
   setGuiSize();
#endif
}
/******************************************************************************/
void OculusRiftApi::delImages()
{
#if SUPPORT_OCULUS
   RenderTexture.del();
      GuiTexture.del();
#endif
}
Bool OculusRiftApi::createGuiImage()
{
#if SUPPORT_OCULUS
   if(GuiTexture.create(VR.guiRes().x, VR.guiRes().y))
   {
     _layer_gui.ColorTexture=GuiTexture.texture_set;
     _layer_gui.Viewport.Pos .x=_layer_gui.Viewport.Pos.y=0;
     _layer_gui.Viewport.Size.w=GuiTexture.image.w();
     _layer_gui.Viewport.Size.h=GuiTexture.image.h();
      setGuiSizeX();
      return true;
   }
#endif
   return false;
}
Bool OculusRiftApi::createRenderImage()
{
#if SUPPORT_OCULUS
   ovrHmdDesc desc=ovr_GetHmdDesc(_session);
   ovrSizei   size=ovr_GetFovTextureSize(_session, ovrEye_Left, desc.DefaultEyeFov[0], VR.pixelDensity());
   Clamp(size.w*=2, 2, D.maxTexSize()); // *2 also makes sure that both eyes have the same width
   Clamp(size.h   , 1, D.maxTexSize());
   if(RenderTexture.create(size.w, size.h))
   {
     _layer_render.ColorTexture[0]=RenderTexture.texture_set; // setting only to 0 means that texture contains data for both eyes
     _layer_render.ColorTexture[1]=null;
     _layer_render.Viewport[0].Pos .x=_layer_render.Viewport[0].Pos .y=_layer_render.Viewport[1].Pos .y=0;
     _layer_render.Viewport[1].Pos .x=_layer_render.Viewport[0].Size.w=_layer_render.Viewport[1].Size.w=RenderTexture.image.w()/2;
     _layer_render.Viewport[0].Size.h=_layer_render.Viewport[1].Size.h=RenderTexture.image.h();
      return true;
   }
#endif
   return false;
}
/******************************************************************************/
ImageRC* OculusRiftApi::getNewRender()
{
#if SUPPORT_OCULUS
   return RenderTexture.getImage();
#endif
   return null;
}
ImageRC* OculusRiftApi::getNewGui()
{
#if SUPPORT_OCULUS
   return GuiTexture.getImage();
#endif
   return null;
}
ImageRC* OculusRiftApi::getLastRender()
{
#if SUPPORT_OCULUS
   return &RenderTexture.image;
#endif
   return null;
}
ImageRC* OculusRiftApi::getLastGui()
{
#if SUPPORT_OCULUS
   return &GuiTexture.image;
#endif
   return null;
}
/******************************************************************************/
void OculusRiftApi::recenter()
{
#if SUPPORT_OCULUS
   if(_session)ovr_RecenterPose(_session);
#endif
}
/******************************************************************************/
void OculusRiftApi::update()
{
#if SUPPORT_OCULUS
   if(_initialized)
   {
      VR._has_render=false;

      if(_session)
      {
        _layer_render.SensorSampleTime=ovr_GetTimeInSeconds(); // needs to be set at the same time when obtaining sensor data
         ovrTrackingState ts=ovr_GetTrackingState(_session, ovr_GetPredictedDisplayTime(_session, 0), ovrTrue);
         if(ts.StatusFlags&ovrStatus_HmdConnected)
         {
          C ovrVector3f &a=ts.RawSensorData.Accelerometer; AccelerometerValue.set(-a.x, -a.y,  a.z);
          C ovrVector3f &g=ts.RawSensorData.Gyro         ;     GyroscopeValue.set( g.x,  g.y, -g.z);
          C ovrVector3f &m=ts.RawSensorData.Magnetometer ;  MagnetometerValue.set( m.x,  m.y, -m.z);
            if(ts.HandStatusFlags[0]&(ovrStatus_OrientationTracked|ovrStatus_PositionTracked))SetPose(VR._left , ts.HandPoses[0].ThePose);
            if(ts.HandStatusFlags[1]&(ovrStatus_OrientationTracked|ovrStatus_PositionTracked))SetPose(VR._right, ts.HandPoses[1].ThePose);
            if(ts.StatusFlags       &(ovrStatus_OrientationTracked|ovrStatus_PositionTracked))
            {
               SetPose(VR._matrix, ts.HeadPose.ThePose);
            #if 0 // slower
               ovrVector3f HmdToEyeViewOffset[2]=
               {
                  {D.eyeDistance()*-0.5f, 0, 0},
                  {D.eyeDistance()* 0.5f, 0, 0},
               };
               ovr_CalcEyePoses(ts.HeadPose.ThePose, HmdToEyeViewOffset, _layer_render.RenderPose);
            #else
               Vec eye_offset=VR._matrix.x*D.eyeDistance(); CHS(eye_offset.z); // need to change z because Oculus is right-handed
               ASSERT(SIZE(ts.HeadPose.ThePose.Position)==SIZE(Vec));
               (Vec&)ts.HeadPose.ThePose.Position-=eye_offset*0.5f; _layer_render.RenderPose[0]=ts.HeadPose.ThePose;
               (Vec&)ts.HeadPose.ThePose.Position+=eye_offset     ; _layer_render.RenderPose[1]=ts.HeadPose.ThePose;
            #endif
            }
         }else disconnect(); // lost connection
      }else
      if(VR.OculusRiftDetected()) // try to connect
      {
         connect();
      }
   }
#endif
}
void OculusRiftApi::draw()
{
#if SUPPORT_OCULUS
   if(active())
   {
   #if 0 // this made absolutely no difference, TODO: test in the future with a newer SDK
      FlagSet(_layer_render.Header.Flags, ovrLayerFlag_HighQuality, Ms.b(0));
      FlagSet(_layer_gui   .Header.Flags, ovrLayerFlag_HighQuality, Ms.b(1));
   #endif

      ovrLayerHeader *layer[2]; Int layers=0;
      if(VR._has_render)layer[layers++]=&_layer_render.Header;
      if(VR. draw_2d   )layer[layers++]=&_layer_gui   .Header;
      ovr_SubmitFrame(_session, 0, null, layer, layers);
   }
#endif
}
/******************************************************************************/
}
/******************************************************************************/
