/******************************************************************************

   Rendering to VR works in a following way:
      -3D is rendered into RenderTexture
      -2D is drawn    into    GuiTexture (assumed to have pixel_aspect=1)
      -the textures are submitted to HMD ('ovr_SubmitFrame' OculusRift, 'Submit' OpenVR)
      -the textures are also drawn to the System Window (without any warp shaders):
         -RenderTexture is drawn with FIT_FILL for the Left Eye only (with correct scale and position so that the eye focus is at the window center)
         -   GuiTexture is drawn with FIT_FULL

   Since VR GuiTexture can be set to a custom size, most likely it will be different than System Window size/resolution.
   Because of that, most display members are set based on VR GuiTexture, and not Window size, this includes:
      D.w, D.h, D.w2, D.h2, D.pixelToScreen, D.pixelToScreenSize, D.screenToPixel, D.screenToPixelSize, ..
   But DOES NOT INCLUDE:
      D.mode, D.res, D.resW, D.resH

   RenderTexture for simplicity, uses the same D.w and D.h values as GuiTexture.
      In both cases range (-D.w, -D.h) .. (D.w, D.h) covers the entire textures.
      However since the textures are not displayed with the same scale, visible screen position in one texture is not the same as in the other.
      To compensate for different texture sizes, 'D.viewFovTan' is calculated differently for RenderTexture/GuiTexture,
         to do correct mapping between 2D<->3D space.

   Just before doing drawing to the System Window, D.w, D.h are recalculated based on window size,
      to maintain correct aspect. Once window drawing is finished, D.w D.h are restored to the VR GuiTexture values.

   D.windowPixelToScreen is used to convert pixel in the System Window, to screen position on VR GuiTexture.
      This is used for converting mouse/touch pointer positions.

   To detect if we're rendering into VR, use:
      VR.active()      - this remains true, as long as VR is connected
      Renderer._stereo - this is set at the start of rendering, based on viewport being full and VR Active

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
static struct VirtualRealityDummyApi : VirtualRealityApi
{
   virtual Bool init()override
   {
      Set(VR._name, "Dummy VR");
      VR._res.set(1280, 720);
      VR._refresh =60;
      VR._eye_dist=0.064f;
      VR.setFOVTan(1.39f, 1.24f, 1.47f, 1.47f);
     _active=true; VR.connected(); // set active before calling connected
      return true;
   }
   virtual void shut()override
   {
      VR.delImages(); // !! need to call 'VR.delImages' and not 'T.delImages' !!
     _active=false; VR.disconnected(); // set active before calling connected
   }

   virtual Bool   active         ()C override {return _active;}
   virtual Matrix matrixCur      ()C override {return MatrixIdentity;}
   virtual void   recenter       ()  override {}
   virtual void   changedGuiDepth()  override {}
   virtual void   changedGuiSize ()  override {}
   virtual void   update         ()  override {}
   virtual void   draw           ()  override {}

   virtual void          delImages()override {_render.del(); _gui.del();}
   virtual Bool    createGuiImage ()override {return _gui   .createTry(VR.guiRes().x, VR.guiRes().y, 1, IMAGE_R8G8B8A8, IMAGE_RT, 1);}
   virtual Bool createRenderImage ()override {return _render.createTry(1280, 720, 1, IMAGE_R8G8B8A8, IMAGE_RT, 1);}

   virtual ImageRC* getNewRender ()override {return  _render.is() ? &_render : null;}
   virtual ImageRC* getNewGui    ()override {return  _gui   .is() ? &_gui    : null;}
   virtual ImageRC* getLastRender()override {return &_render;}
   virtual ImageRC* getLastGui   ()override {return &_gui   ;}

private:
   Bool    _active;
   ImageRC _render, _gui;
}VirtualRealityDummy;
/******************************************************************************/
       VirtualReality    VR;
static VirtualRealityApi VrNull;
/******************************************************************************/
static void ClampTexSize(Int &w, Int &h)
{
   if(D.maxTexSize()>0)
   {
      if(w>D.maxTexSize())
      {
         h=RoundPos(Flt(D.maxTexSize())/w*h);
         w=D.maxTexSize();
      }
      if(h>D.maxTexSize())
      {
         w=RoundPos(Flt(D.maxTexSize())/h*w);
         h=D.maxTexSize();
      }
   }
   MAX(w, 1);
   MAX(h, 1);
}
/******************************************************************************/
VirtualReality::VirtualReality()
{
   draw_2d=true;
  _has_render=false;
  _name[0]='\0';
  _eye_dist=0.064f;
  _density=1;
  _refresh=60;
  _gui_depth=_gui_size=1;
  _res.zero();
  _gui_res.set(1024, 1024);
  _fov=DegToRad(70);
  _matrix.identity();
  _left  .identity();
  _right .identity();
  _adapter_id=0;
  _api=&VrNull; // !! '_api' may never be null !!
#if 0 // not needed at start, also 'VR' is global so it will always be set to zero
  _left_eye_tex_aspect=1;
  _left_eye_tex_rect  .zero();
#endif
}
void VirtualReality::DummyInit() {VR.init(VirtualRealityDummy);}
Bool VirtualReality::init(VirtualRealityApi &api)
{
   if(_api==&api)return true; // already using this API
   shut(); // need to fully shut down existing API first
  _api=&api; // need to set this first in case 'init' will call 'VR._api' methods
   if(api.init()) // if API initialized OK
   {
      // put to API what was set earlier
      api.changedGuiDepth();
      api.changedGuiSize ();
      return true;
   }
  _api=&VrNull; return false; // if failed then revert back to null
}
void VirtualReality::shut()
{
   if(_api!=&VrNull)
   {
     _api->shut();
     _api=&VrNull; // !! '_api' may never be null !!
     _adapter_id=0; // disable forcing VR adapter
   }
}
Bool VirtualReality::connected()
{
   if(createImages())
   {
      Ms.resetCursor();
      Frustum.set();
   #if GL // for OpenGL we need to adjust screen synchronization, DirectX can control sync real-time in the 'Present' function
      D.setSync();
   #endif
      return true;
   }
   return false;
}
void VirtualReality::disconnected()
{
   if(D.created())
   {
      Renderer.setMain();
      D.aspectRatioEx();
   #if GL // for OpenGL we need to adjust screen synchronization, DirectX can control sync real-time in the 'Present' function
      D.setSync();
   #endif
   }
   Ms.clipUpdate(); // call 'clipUpdate' after 'aspectRatioEx', as display size affects mouse clip rect
   Ms.resetCursor();
   Frustum.set();
}
/******************************************************************************/
Bool   VirtualReality::active   ()C {return _api->active   ();}
Matrix VirtualReality::matrixCur()C {return _api->matrixCur();}
void   VirtualReality::recenter ()  {       _api->recenter ();}
void   VirtualReality::update   ()  {       _api->update   ();}
void   VirtualReality::draw     ()  {       _api->draw     ();}
void   VirtualReality::drawMain () // remember that we're rendering left eye, so it will be offsetted by eye distance
{
   if(Image *render=getLastRender())
   {
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
   #if DEBUG && 1
      if(Kb.b(KB_NPMUL)){D.clearCol(); render->drawFs(FIT_FULL, FILTER_LINEAR);}else
   #endif
      render->drawPart(Fit(_left_eye_tex_aspect, D.rect(), FIT_FILL), _left_eye_tex_rect);
      D.alpha(alpha);
   }else D.clearCol(); // clear because gui may not cover the entire window

   if(draw_2d)if(Image *image=getLastGui())
   {
      Rect screen=image->fit(D.rect(), FIT_FULL);
      image->draw(screen);
   }
}
/******************************************************************************/
void VirtualReality::setFOVTan(Flt left, Flt right, Flt up, Flt down)
{
   VR._fov.set(2*Atan(Avg(left, right)),
               2*Atan(Avg(up  , down )));

   Flt proj_center       =left/(left+right),
       proj_center_offset=Lerp(-1.0f, 1.0f, proj_center);
   ProjMatrixEyeOffset[0]= proj_center_offset;
   ProjMatrixEyeOffset[1]=-proj_center_offset;

   VR._left_eye_tex_aspect=Min(left, right)/Min(up, down);
   VR._left_eye_tex_rect.setC(proj_center*0.5f, 0.5f, Min(proj_center, 1-proj_center), 1); // calculate width based on how much we can go to the left and right edge of the texture
}
/******************************************************************************/
VirtualReality& VirtualReality::pixelDensity(Flt density)
{
   Clamp(density, 0, 2); if(T._density!=density){T._density=density; createRenderImage();} return T;
}
VirtualReality& VirtualReality::guiRes(Int w, Int h)
{
   ClampTexSize(w, h); if(_gui_res.x!=w || _gui_res.y!=h){_gui_res.set(w, h); createGuiImage();} return T;
}
VirtualReality& VirtualReality::guiDepth(Flt depth)
{
   MAX(depth, 0); if(_gui_depth!=depth){_gui_depth=depth; _api->changedGuiDepth();} return T;
}
VirtualReality& VirtualReality::guiSize(Flt size)
{
   MAX(size, 0); if(_gui_size!=size){_gui_size=size; _api->changedGuiSize(); D.setViewFovTan();} return T;
}

void VirtualReality::delImages()
{
  _has_render=false; _api->delImages();
}
Bool VirtualReality::createGuiImage()
{
   if(D.created() && active())
   {
      ClampTexSize(_gui_res.x, _gui_res.y);
      if(_api->createGuiImage())
      {
         SyncLockerEx locker(D._lock);
         Renderer.setMain();
         D.aspectRatioEx(); Ms.clipUpdate(); // call in this order, as display size affects mouse clip rect
         return true;
      }
      shut(); return false;
   }
   return true;
}
Bool VirtualReality::createRenderImage()
{
   if(D.created() && active())
   {
      if(_api->createRenderImage())
      {
        _has_render=false;
         return true;
      }
      shut(); return false;
   }
   return true;
}
Bool VirtualReality::createImages()
{
   return createRenderImage() && createGuiImage();
}

ImageRC* VirtualReality::getNewRender () {   if(ImageRC *image=_api->getNewRender ()){_has_render=true; return image;} return null;}
ImageRC* VirtualReality::getNewGui    () {return               _api->getNewGui    ()       ;}
ImageRC* VirtualReality::getLastRender() {return _has_render ? _api->getLastRender() : null;}
ImageRC* VirtualReality::getLastGui   () {return               _api->getLastGui   ()       ;}
/******************************************************************************/
}
/******************************************************************************/
