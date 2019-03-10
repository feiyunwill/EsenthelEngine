/******************************************************************************/
#include "stdafx.h"
#define SUPPORT_OPEN_VR ((WINDOWS_OLD && !DX9) || MAC || LINUX) // DX9 not supported
#if     SUPPORT_OPEN_VR
   #if LINUX
      #define COMPILER_GCC // needs to be defined for Linux otherwise "openvr.h" will complain about unsupported platform
   #endif
   #include "../../../ThirdPartyLibs/begin.h"
   #include "../../../ThirdPartyLibs/OpenVR/headers/openvr.h"
   #include "../../../ThirdPartyLibs/end.h"
#endif
/******************************************************************************/
namespace EE{
/******************************************************************************/
static struct OpenVRApi : VirtualRealityApi
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

   OpenVRApi();

   void setOverlaySizeDepth();
   Bool    connect();
   void disconnect();

#if SUPPORT_OPEN_VR
private:
   vr::IVRSystem        *_vr;
   vr::IVRCompositor    *_compositor;
   vr::IVROverlay       *_overlay;
   vr::VROverlayHandle_t _overlay_id;
   Bool                  _overlay_visible, _connected;
   ImageRC               _render, _gui;
#endif
}OpenVR;
#if SUPPORT_OPEN_VR
static void SetPose(Matrix &m, C vr::HmdMatrix34_t &pose)
{
   m.x  .set( pose.m[0][0],  pose.m[1][0], -pose.m[2][0]);
   m.y  .set( pose.m[0][1],  pose.m[1][1], -pose.m[2][1]);
   m.z  .set(-pose.m[0][2], -pose.m[1][2],  pose.m[2][2]);
   m.pos.set( pose.m[0][3],  pose.m[1][3], -pose.m[2][3]);
}
static void SetPose(vr::HmdMatrix34_t &pose, C Matrix &m)
{
   pose.m[0][0]= m.x  .x; pose.m[1][0]= m.x  .y; pose.m[2][0]=-m.x  .z;
   pose.m[0][1]= m.y  .x; pose.m[1][1]= m.y  .y; pose.m[2][1]=-m.y  .z;
   pose.m[0][2]=-m.z  .x; pose.m[1][2]=-m.z  .y; pose.m[2][2]= m.z  .z;
   pose.m[0][3]= m.pos.x; pose.m[1][3]= m.pos.y; pose.m[2][3]=-m.pos.z;
}
#endif
/******************************************************************************/
Bool VirtualReality::OpenVRDetected()C
{
#if SUPPORT_OPEN_VR
   return vr::VR_IsHmdPresent();
#endif
   return false;
}
Bool VirtualReality::OpenVRInit() {return VR.init(OpenVR);}
/******************************************************************************/
OpenVRApi::OpenVRApi()
{
#if SUPPORT_OPEN_VR
  _vr             =null;
  _compositor     =null;
  _overlay        =null;
  _overlay_id     =vr::k_ulOverlayHandleInvalid;
  _overlay_visible=false;
  _connected      =false;
#endif
}
/******************************************************************************/
#if SUPPORT_OPEN_VR
void OpenVRApi::disconnect()
{
   VR.delImages(); // !! need to call 'VR.delImages' and not 'T.delImages' !!
   if(_connected)
   {
     _connected=false;
      VR.disconnected(); // call 'disconnected' after clearing '_connected' so 'VR.active' is false (needed for some things including 'D.setSync')
   }
}
Bool OpenVRApi::connect()
{
   disconnect();

  _connected=true;

   if(vr::IVRExtendedDisplay *d=vr::VRExtendedDisplay())
   {
      int32_t x=0, y=0; uint32_t w=0, h=0; d->GetWindowBounds(&x, &y, &w, &h);
      VR._res.set(w, h);
   }else VR._res.zero();

   VR._adapter_id=0; int32_t adapter_index=-1; _vr->GetDXGIOutputInfo(&adapter_index); if(adapter_index>=0) // if want a custom adapter
   {
   #if DX11
      SyncLocker lock(D._lock);
      IDXGIFactory1 *factory=null; CreateDXGIFactory1(__uuidof(IDXGIFactory1), (Ptr*)&factory); if(factory)
      {
         IDXGIAdapter *adapter=null; factory->EnumAdapters(adapter_index, &adapter); if(adapter)
         {
            DXGI_ADAPTER_DESC desc; if(OK(adapter->GetDesc(&desc)))
            {
               ASSERT(SIZE(desc.AdapterLuid)==SIZE(VR._adapter_id));
               Copy(&VR._adapter_id, &desc.AdapterLuid, SIZE(VR._adapter_id));
            }
            adapter->Release();
         }
         factory->Release();
      }
   #endif
   }

   VR._name[0] ='\0'; _vr->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String, VR._name, Elms(VR._name));
   VR._refresh =       _vr->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
   VR._eye_dist=       _vr->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float);
   if(VR._eye_dist<=0) // if unknown/invalid, then calculate differently:
   {
      vr::HmdMatrix34_t left =_vr->GetEyeToHeadTransform(vr::Eye_Left );
      vr::HmdMatrix34_t right=_vr->GetEyeToHeadTransform(vr::Eye_Right);
      Matrix l, r; SetPose(l, left); SetPose(r, right);
      VR._eye_dist=Dist(l.pos, r.pos);
   }

   Flt l, r, t, b; _vr->GetProjectionRaw(vr::Eye_Left, &l, &r, &t, &b);
   VR.setFOVTan(Abs(l), Abs(r), Abs(t), Abs(b));
   return VR.connected(); // call 'connected' after having set '_connected' so 'VR.active' is true (needed for some things including 'D.setSync')
}
#endif
Bool OpenVRApi::init()
{
#if SUPPORT_OPEN_VR
   if(!_vr)
   {
      vr::EVRInitError error; if(_vr=vr::VR_Init(&error, vr::VRApplication_Scene))
      {
         if(_compositor=vr::VRCompositor())
         {
           _compositor->SetTrackingSpace(vr::TrackingUniverseSeated); // this will have 2 effects: 1) 'ResetSeatedZeroPose' will affect Matrix returned by 'WaitGetPoses' (correct behavior, because without this call, calling recenter does not affect 'WaitGetPoses') 2) it will make "chaperone" bounds disappear (the blue circle at the bottom which specifies play area size)
            if(_overlay=vr::VROverlay())
            {
              _overlay->CreateOverlay("Esenthel", "Gui", &_overlay_id); if(_overlay_id!=vr::k_ulOverlayHandleInvalid)
               {
                 _overlay->SetHighQualityOverlay(_overlay_id);
                 _overlay->HideOverlay(_overlay_id); _overlay_visible=false;
               #if GL // for OpenGL need to flip vertically
                  vr::VRTextureBounds_t rect; rect.uMin=0; rect.uMax=1; rect.vMin=1; rect.vMax=0;
                 _overlay->SetOverlayTextureBounds(_overlay_id, &rect);
               #endif
               }
            }
            connect();
         }else
         {
           _vr=null;
            vr::VR_Shutdown();
         }
      }
   }
   return _vr!=null;
#endif
   return false;
}
void OpenVRApi::shut()
{
#if SUPPORT_OPEN_VR
   if(_vr)
   {
      disconnect();
      if(_overlay)
      {
         if(_overlay_id!=vr::k_ulOverlayHandleInvalid)_overlay->DestroyOverlay(_overlay_id);
        _overlay=null;
      }
     _overlay_visible=false;
     _overlay_id     =vr::k_ulOverlayHandleInvalid;
     _compositor     =null;
     _vr             =null;
      vr::VR_Shutdown();
   }
#endif
}
/******************************************************************************/
Bool OpenVRApi::active()C
{
#if SUPPORT_OPEN_VR
   return _connected;
#endif
   return false;
}
Matrix OpenVRApi::matrixCur()C
{
#if SUPPORT_OPEN_VR
   if(_vr)
   {
      vr::TrackedDevicePose_t pose; _vr->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseSeated, 0, &pose, 1);
      if(pose.bDeviceIsConnected && pose.bPoseIsValid)
      {
         Matrix m; SetPose(m, pose.mDeviceToAbsoluteTracking); return m;
      }
   }
#endif
   return VR._matrix; // return last known matrix
}
void OpenVRApi::recenter()
{
#if SUPPORT_OPEN_VR
   if(_vr)_vr->ResetSeatedZeroPose();
#endif
}
void OpenVRApi::setOverlaySizeDepth()
{
#if SUPPORT_OPEN_VR
   if(_overlay && _overlay_id!=vr::k_ulOverlayHandleInvalid)
   {
      Matrix m; m.setPos(0, 0, VR.guiDepth());
      vr::HmdMatrix34_t pose; SetPose(pose, m);
     _overlay->SetOverlayTransformTrackedDeviceRelative(_overlay_id, vr::k_unTrackedDeviceIndex_Hmd, &pose);
     _overlay->SetOverlayWidthInMeters(_overlay_id, VR.guiSize()*VR.guiDepth()*_gui.aspect());
   }
#endif
}
void OpenVRApi::changedGuiDepth() {setOverlaySizeDepth();}
void OpenVRApi::changedGuiSize () {setOverlaySizeDepth();}
void OpenVRApi::update()
{
#if SUPPORT_OPEN_VR
   if(_vr)
   {
      VR._has_render=false;
	   vr::VREvent_t event; while(_vr->PollNextEvent(&event, SIZE(event))) {}
      vr::TrackedDevicePose_t pose;
      vr::EVRCompositorError error;
      {
         SyncLocker locker(D._lock); // without this lock, 'WaitGetPoses' will either crash or deadlock when simultaneously doing GPU operations on other threads
         error=_compositor->WaitGetPoses(&pose, 1, null, 0);
      }
      if(error==vr::VRCompositorError_None)
      {
         // TODO: support connecting/disconnecting !bDeviceIsConnected
         if(pose.bDeviceIsConnected && pose.bPoseIsValid)
         {
            GyroscopeValue.set(pose.vAngularVelocity.v[0], pose.vAngularVelocity.v[1], -pose.vAngularVelocity.v[2]);
            SetPose(VR._matrix, pose.mDeviceToAbsoluteTracking);
         }
      }
	 /*for(vr::TrackedDeviceIndex_t i=0; i<vr::k_unMaxTrackedDeviceCount; i++)
	   {
		   vr::VRControllerState_t state; if(_vr->GetControllerState(i, &state))
         {
         }
      }*/
   }
#endif
}
void OpenVRApi::draw()
{
#if SUPPORT_OPEN_VR
   if(active())
   {
      if(!VR._has_render)_render.clearFull(Vec4Zero, true); // if render was not set, then clear

      vr::Texture_t t;
      t.eType=GPU_API(vr::API_DirectX, vr::API_DirectX, vr::API_OpenGL);

      if(_overlay && _overlay_id!=vr::k_ulOverlayHandleInvalid)
      {
         if(_overlay_visible!=VR.draw_2d)
         {
            if(_overlay_visible=VR.draw_2d)_overlay->ShowOverlay(_overlay_id);
            else                           _overlay->HideOverlay(_overlay_id);
         }
         if(_overlay_visible) // this needs to be called per-frame
         {
            t.handle=(Ptr)_gui._txtr;
            t.eColorSpace=vr::ColorSpace_Gamma; // TODO: support gamma correct sRGB rendering
	        _overlay->SetOverlayTexture(_overlay_id, &t);
         }
      }

      t.handle=(Ptr)_render._txtr;
      t.eColorSpace=vr::ColorSpace_Gamma; // TODO: support gamma correct sRGB rendering

      vr::VRTextureBounds_t rect;
   #if GL // for OpenGL need to flip vertically
      rect.vMin=1; rect.vMax=0;
   #else
      rect.vMin=0; rect.vMax=1;
   #endif
      rect.uMin=0.0f; rect.uMax=0.5f; _compositor->Submit(vr::Eye_Left , &t, &rect);
      rect.uMin=0.5f; rect.uMax=1.0f; _compositor->Submit(vr::Eye_Right, &t, &rect);

     _compositor->PostPresentHandoff(); // even though headers suggest this should be called after Present/flipping, best performance was observed when this was called here (tested in a simple project with high GPU usage, lots of post-process enabled, results were around 30fps when called here, 28 when called after flip, 25 without calling this at all)
   }
#endif
}
void OpenVRApi::delImages()
{
#if SUPPORT_OPEN_VR
   if(_overlay && _overlay_id!=vr::k_ulOverlayHandleInvalid)_overlay->ClearOverlayTexture(_overlay_id);
  _render.del();
  _gui   .del();
#endif
}
Bool OpenVRApi::createGuiImage()
{
#if SUPPORT_OPEN_VR
   if(_gui.createTry(VR.guiRes().x, VR.guiRes().y, 1, IMAGE_R8G8B8A8, IMAGE_RT, 1))
   {
      if(_overlay && _overlay_id!=vr::k_ulOverlayHandleInvalid)
      {
         vr::Texture_t t;
         t.handle=(Ptr)_gui._txtr;
         t.eType=GPU_API(vr::API_DirectX, vr::API_DirectX, vr::API_OpenGL);
         t.eColorSpace=vr::ColorSpace_Gamma; // TODO: support gamma correct sRGB rendering
	     _overlay->SetOverlayTexture(_overlay_id, &t);
         setOverlaySizeDepth();
      }
      return true;
   }
#endif
   return false;
}
Bool OpenVRApi::createRenderImage()
{
#if SUPPORT_OPEN_VR
   if(_vr)
   {
      uint32_t w=0, h=0; _vr->GetRecommendedRenderTargetSize(&w, &h);
      Clamp(w*=2, 2, D.maxTexSize()); // *2 also makes sure that both eyes have the same width
      Clamp(h   , 1, D.maxTexSize());
      return _render.createTry(w, h, 1, IMAGE_R8G8B8A8, IMAGE_RT, 1);
   }
#endif
   return false;
}
ImageRC* OpenVRApi::getNewRender()
{
#if SUPPORT_OPEN_VR
   if(_render.is())return &_render;
#endif
   return null;
}
ImageRC* OpenVRApi::getNewGui()
{
#if SUPPORT_OPEN_VR
   if(_gui.is())return &_gui;
#endif
   return null;
}
ImageRC* OpenVRApi::getLastRender()
{
#if SUPPORT_OPEN_VR
   return &_render;
#endif
   return null;
}
ImageRC* OpenVRApi::getLastGui()
{
#if SUPPORT_OPEN_VR
   return &_gui;
#endif
   return null;
}
/******************************************************************************/
}
/******************************************************************************/
