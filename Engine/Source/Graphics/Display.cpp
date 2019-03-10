/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define D3D_DEBUG     0
#define FORCE_D3D9_3  0
#define FORCE_D3D10_0 0
#define FORCE_D3D10_1 0

#define MAC_GL_MT 1 // enable multi threaded rendering, TODO: test on newer hardware if improves performance, all GL Contexts should have the same value, otherwise creating VAO's from VBO's on other threads could fail

#if D3D_DEBUG || FORCE_D3D9_3 || FORCE_D3D10_0 || FORCE_D3D10_1
   #pragma message("!! Warning: Use this only for debugging !!")
#endif

#define FORCE_MAIN_DISPLAY (DX9 || WINDOWS && GL) // for DX9 and WindowsOpenGL we need to use the main screen

#if 0
   #define LOG(x) LogN(x)
#else
   #define LOG(x)
#endif

#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
/******************************************************************************/
Display D;

#if DX9 // DirectX 9
   static IDirect3D9           *D3DBase;
          IDirect3DDevice9     *D3D;
   static IDirect3DQuery9      *Query;
   static D3DPRESENT_PARAMETERS D3DPP;
#elif DX11 // DirectX 10/11
   #define GDI_COMPATIBLE 0 // requires DXGI_FORMAT_B8G8R8A8_UNORM
   static Bool                  AllowTearing;
   static UInt                  PresentFlags;
          ID3D11Device         *D3D;
          ID3D11DeviceContext  *D3DC;
          ID3D11DeviceContext1 *D3DC1;
       #if WINDOWS_OLD
          IDXGIFactory1        *Factory;
          IDXGISwapChain       *SwapChain;
          DXGI_SWAP_CHAIN_DESC  SwapChainDesc;
       #else
          IDXGIFactory2        *Factory;
          IDXGISwapChain1      *SwapChain;
          DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
       #endif
          IDXGIAdapter         *Adapter;
          IDXGIOutput          *Output;
          ID3D11Query          *Query;
          D3D_FEATURE_LEVEL     FeatureLevel;
#elif GL // OpenGL
   #if WINDOWS
      static HDC hDC;
   #elif MAC
      NSOpenGLContext *OpenGLContext;
   #elif LINUX
      typedef void (*glXSwapIntervalType)(::Display *display, GLXDrawable drawable, int interval);
      static         glXSwapIntervalType   glXSwapInterval;
                   ::Display              *XDisplay;
                     GLXFBConfig           GLConfig;
      static         int                   vid_modes=0;
      static         XF86VidModeModeInfo **vid_mode=null;
   #elif ANDROID
      static EGLConfig  GLConfig;
      static EGLDisplay GLDisplay;
   #endif
               GLContext       MainContext;
   static Mems<GLContext> SecondaryContexts;
   static SyncLock        ContextLock;
   static SyncCounter     ContextUnlocked; // have to use counter and not event, because if there would be 2 unlocks at the same time while 2 other are waiting, then only 1 would get woken up with event
          UInt            FBO, VAO;
       #if VARIABLE_MAX_MATRIX
          Bool            MeshBoneSplit;
       #endif
#endif

#if WINDOWS_NEW
Flt ScreenScale; // can't initialize here because crash will occur
#elif IOS
Flt ScreenScale=([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)] ? [UIScreen mainScreen].nativeScale : [UIScreen mainScreen].scale);
#endif
/******************************************************************************/
static Bool ActualSync() {return D.sync() && !VR.active();} // we can synchronize only when not using VR, otherwise, it will handle synchronization based on the VR refresh rate
/******************************************************************************/
static Bool CustomMode;
#if MAC
extern NSOpenGLView *OpenGLView;
static CGDisplayModeRef GetDisplayMode(Int width, Int height)
{
   CFArrayRef       modes=CGDisplayCopyAllDisplayModes(CGMainDisplayID(), null);
   CGDisplayModeRef ret=null;
   Flt              refresh;
   REP(CFArrayGetCount(modes))
   {
      CGDisplayModeRef mode=(CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
      UInt flags=CGDisplayModeGetIOFlags(mode);
      Bool ok   =FlagTest(flags, kDisplayModeSafetyFlags);
      if(  ok)
      {
         Int w=CGDisplayModeGetWidth      (mode),
             h=CGDisplayModeGetHeight     (mode);
         Flt r=CGDisplayModeGetRefreshRate(mode);
         if(w==width && h==height)
            if(!ret || r>refresh) // choose highest refresh rate
         {
            ret    =mode;
            refresh=r;
         }
      }
   }
   CFRelease(modes);
   return ret;
}
#endif
Bool SetDisplayMode(Int mode)
{
   Bool  full=(D.full() && (mode==2 || mode==1 && App.active()));
   VecI2 size=(full ? D.res() : App.desktop());
   Bool  same=(D.screen()==size);
#if WINDOWS_OLD
   if(full)
   {
      if(same)return true;
      DEVMODE screen; Zero(screen);
      screen.dmSize              =SIZE(DEVMODE);
      screen.dmPelsWidth         =D.resW();
      screen.dmPelsHeight        =D.resH();
      screen.dmDisplayFixedOutput=DMDFO_STRETCH; // this will stretch to entire screen if aspect ratio is not the same
      screen.dmFields            =DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFIXEDOUTPUT;
   again:
      Int result=ChangeDisplaySettings(&screen, CDS_FULLSCREEN); if(result==DISP_CHANGE_SUCCESSFUL)
      {
         CustomMode=true;
         return true;
      }
      if(result==DISP_CHANGE_BADMODE && (screen.dmFields&DM_DISPLAYFIXEDOUTPUT)) // this can fail if trying to set the biggest resolution with stretching
         {FlagDisable(screen.dmFields, DM_DISPLAYFIXEDOUTPUT); screen.dmDisplayFixedOutput=0; goto again;} // try again without scaling
   }else
   {
      if(!CustomMode)return true;
      if(ChangeDisplaySettings(null, 0)==DISP_CHANGE_SUCCESSFUL)
      {
         CustomMode=false;
         return true;
      }
   }
#elif MAC
   if(OpenGLView)
   {
      REPA(Ms._button)Ms.release(i); // Mac will not detect mouse button release if it was pressed during screen changing, so we need to disable it manually

      if([OpenGLView isInFullScreenMode])[OpenGLView exitFullScreenModeWithOptions:nil];

      // set screen mode
      Bool ok=false;
      if(same)ok=true;else
      if(CGDisplayModeRef mode=GetDisplayMode(size.x, size.y))
         ok=(CGDisplaySetDisplayMode(kCGDirectMainDisplay, mode, null)==noErr);

      if(ok && full)
      {
      #if 0
         ok=[OpenGLView enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
      #else
         NSApplicationPresentationOptions options=NSApplicationPresentationHideDock|NSApplicationPresentationHideMenuBar;
         ok=[OpenGLView enterFullScreenMode:[NSScreen mainScreen] withOptions:@{NSFullScreenModeApplicationPresentationOptions:@(options)}];
      #endif
      }
      return ok;
   }
#elif LINUX
   if(XDisplay)
   {
      // set screen mode
      Bool ok=false;
      if(same)ok=true;else
      FREP(vid_modes)
      {
         XF86VidModeModeInfo &mode=*vid_mode[i];
         if(mode.hdisplay==size.x && mode.vdisplay==size.y)
            if(XF86VidModeSwitchToMode(XDisplay, DefaultScreen(XDisplay), &mode) && XFlush(XDisplay)){ok=true; break;}
      }
      return ok;  
   }
#endif
   return false;
}
#if WINDOWS_NEW
void RequestDisplayMode(Int w, Int h, Int full)
{
   if(full> 0)Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TryEnterFullScreenMode();else
   if(full==0)Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->ExitFullScreenMode    ();
   if(w>0 || h>0)
   {
      if(w<=0)w=D.resW();
      if(h<=0)h=D.resH();
      Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TryResizeView(Windows::Foundation::Size(PixelsToDips(w), PixelsToDips(h)));
   }
}
#endif

#if WINDOWS && GL
static void glewSafe()
{
#define V(x, y, z) {if(!x)x=y; if(!x)Exit("OpenGL " z " function not supported.\nGraphics Driver not installed or better video card is required.");}
   V(glGenRenderbuffers          , glGenRenderbuffersEXT          , "glGenRenderbuffers")
   V(glDeleteRenderbuffers       , glDeleteRenderbuffersEXT       , "glDeleteRenderbuffers")
   V(glRenderbufferStorage       , glRenderbufferStorageEXT       , "glRenderbufferStorage")
   V(glGetRenderbufferParameteriv, glGetRenderbufferParameterivEXT, "glGetRenderbufferParameteriv")
   V(glBindRenderbuffer          , glBindRenderbufferEXT          , "glBindRenderbuffer")

   V(glGenFramebuffers           , glGenFramebuffersEXT           , "glGenFramebuffers")
   V(glDeleteFramebuffers        , glDeleteFramebuffersEXT        , "glDeleteFramebuffers")
   V(glBindFramebuffer           , glBindFramebufferEXT           , "glBindFramebuffer")
   V(glBlitFramebuffer           , glBlitFramebufferEXT           , "glBlitFramebuffer")
#if DEBUG
   V(glCheckFramebufferStatus    , glCheckFramebufferStatusEXT    , "glCheckFramebufferStatus")
#endif   

   V(glFramebufferTexture2D      , glFramebufferTexture2DEXT      , "glFramebufferTexture2D")
   V(glFramebufferRenderbuffer   , glFramebufferRenderbufferEXT   , "glFramebufferRenderbuffer")

   V(glBlendColor                , glBlendColorEXT                , "glBlendColor")
   V(glBlendEquation             , glBlendEquationEXT             , "glBlendEquation")
   V(glBlendEquationSeparate     , glBlendEquationSeparateEXT     , "glBlendEquationSeparate")
   V(glBlendFuncSeparate         , glBlendFuncSeparateEXT         , "glBlendFuncSeparate")

   V(glColorMaski, glColorMaskIndexedEXT, "glColorMaski")
#undef V
}
#endif
/******************************************************************************/
// GL CONTEXT
/******************************************************************************/
#if GL
static Ptr GetCurrentContext()
{
#if WINDOWS
   return wglGetCurrentContext();
#elif MAC
   return CGLGetCurrentContext();
#elif LINUX
   return glXGetCurrentContext();
#elif ANDROID
   return eglGetCurrentContext();
#elif IOS
   return [EAGLContext currentContext];
#elif WEB
   return (Ptr)emscripten_webgl_get_current_context();
#endif
}
Bool GLContext::is()C
{
#if WEB
   return context!=NULL;
#else
   return context!=null;
#endif
}
GLContext::GLContext()
{
   locked=false;
#if WEB
   context=NULL;
#else
   context=null;
#endif
#if ANDROID
   surface=null;
#endif
}
void GLContext::del()
{
   if(context)
   {
   #if WINDOWS
      wglMakeCurrent(null, null); wglDeleteContext(context); context=null;
   #elif MAC
      CGLDestroyContext(context); context=null;
   #elif LINUX
      if(XDisplay){glXMakeCurrent(XDisplay, NULL, NULL); glXDestroyContext(XDisplay, context);} context=null;
   #elif ANDROID
      if(GLDisplay){eglMakeCurrent(GLDisplay, null, null, null); eglDestroyContext(GLDisplay, context);} context=null;
   #elif IOS
      [EAGLContext setCurrentContext:null]; [context release]; context=null;
   #elif WEB
      emscripten_webgl_destroy_context(context); context=NULL;
   #endif
   }
#if ANDROID
   if(surface){if(GLDisplay)eglDestroySurface(GLDisplay, surface); surface=null;}
#endif
}
Bool GLContext::createSecondary()
{
#if WINDOWS
   if(context=wglCreateContext(hDC))if(!wglShareLists(MainContext.context, context))return false;
#elif MAC
   CGLCreateContext(CGLGetPixelFormat(MainContext.context), MainContext.context, &context);
#elif LINUX
   context=glXCreateNewContext(XDisplay, GLConfig, GLX_RGBA_TYPE, MainContext.context, true);
#elif ANDROID
   EGLint attribs[]={EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
   if(surface=eglCreatePbufferSurface(GLDisplay, GLConfig, attribs))
   {
      EGLint ctx_attribs[]={EGL_CONTEXT_CLIENT_VERSION, (D.shaderModel()==SM_GL_ES_3) ? 3 : 2, EGL_NONE};
      context=eglCreateContext(GLDisplay, GLConfig, MainContext.context, ctx_attribs);
   }
#elif IOS
	context=[[EAGLContext alloc] initWithAPI:[MainContext.context API] sharegroup:[MainContext.context sharegroup]];
#elif WEB
   // currently WEB is not multi-threaded
   #if HAS_THREADS
      add support
   #endif
#endif
   if(context)
   {
      lock();
      // these settings are per-context
   #if MAC && MAC_GL_MT
      Bool mt_ok=(CGLEnable(context, kCGLCEMPEngine)!=kCGLNoError);
   #endif
      glPixelStorei(GL_PACK_ALIGNMENT  , 1);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      unlock(); // to clear 'locked'
      return true;
   }
   return false;
}
void GLContext::lock()
{
#if WINDOWS
   if(wglMakeCurrent(hDC, context))
#elif MAC
   if(CGLSetCurrentContext(context)==kCGLNoError)
#elif LINUX
   if(glXMakeCurrent(XDisplay, App.Hwnd(), context))
#elif ANDROID
   if(eglMakeCurrent(GLDisplay, surface, surface, context)==EGL_TRUE)
#elif IOS
   if([EAGLContext setCurrentContext:context]==YES)
#elif WEB
   if(emscripten_webgl_make_context_current(context)==EMSCRIPTEN_RESULT_SUCCESS)
#endif
   {
      locked=true;
   }else
   {
      Exit("Can't activate OpenGL Context.");
   }
}
void GLContext::unlock()
{
#if WINDOWS
   if(wglMakeCurrent(hDC, null))
#elif MAC
   if(CGLSetCurrentContext(null)==kCGLNoError)
#elif LINUX
   if(glXMakeCurrent(XDisplay, NULL, NULL))
#elif ANDROID
   if(eglMakeCurrent(GLDisplay, null, null, null)==EGL_TRUE)
#elif IOS
   if([EAGLContext setCurrentContext:null]==YES)
#elif WEB
   if(emscripten_webgl_make_context_current(NULL)==EMSCRIPTEN_RESULT_SUCCESS)
#endif
   {
      locked=false;
   }else
   {
      Exit("Can't deactivate OpenGL Context.");
   }
}
#endif
/******************************************************************************/
// DISPLAY
/******************************************************************************/
VecI2 Display::screen()C
{
#if WINDOWS_OLD
   return VecI2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#elif WINDOWS_NEW
   IDXGIOutput *output=null;
   if(SwapChain)
   {
      SyncLocker locker(_lock); if(SwapChain)SwapChain->GetContainingOutput(&output);
   }
   if(!output)
   {
      IDXGIFactory1 *factory=null; CreateDXGIFactory1(__uuidof(IDXGIFactory1), (Ptr*)&factory); if(factory)
      {
         IDXGIAdapter *adapter=null; factory->EnumAdapters(0, &adapter); if(adapter)
         {
            adapter->EnumOutputs(0, &output);
            adapter->Release();
         }
         factory->Release();
      }
   }
   if(output)
   {
      VecI2 size=-1; DXGI_OUTPUT_DESC desc; if(OK(output->GetDesc(&desc)))size.set(desc.DesktopCoordinates.right-desc.DesktopCoordinates.left, desc.DesktopCoordinates.bottom-desc.DesktopCoordinates.top);
      output->Release();
      if(size.x>=0)return size;
   }
#elif MAC
   if(CGDisplayModeRef mode=CGDisplayCopyDisplayMode(kCGDirectMainDisplay))
   {
      VecI2 s(CGDisplayModeGetWidth(mode), CGDisplayModeGetHeight(mode));
      CGDisplayModeRelease(mode);
      return s;
   }
#elif LINUX
   if(XDisplay)
   {
      int clock; XF86VidModeModeLine mode; if(XF86VidModeGetModeLine(XDisplay, DefaultScreen(XDisplay), &clock, &mode))return VecI2(mode.hdisplay, mode.vdisplay);
      Screen *screen=DefaultScreenOfDisplay(XDisplay); return VecI2(WidthOfScreen(screen), HeightOfScreen(screen));
   }
#elif ANDROID // fall down to the App.desktop()
#elif IOS
   CGSize size;
   if([[UIScreen mainScreen] respondsToSelector:@selector(nativeBounds)])
   {
      size=[[UIScreen mainScreen] nativeBounds].size; // 'nativeBounds' is not changed when device is rotated
   }else
   {
      size=[[UIScreen mainScreen] bounds].size; // 'bounds' is changed when device is rotated
      size.width *=ScreenScale;
      size.height*=ScreenScale;
   }
   return VecI2(RoundPos(size.width), RoundPos(size.height));
#elif WEB
   return VecI2(JavaScriptRunI("screen.width"), JavaScriptRunI("screen.height")); // it's not possible to get correct results, because on Chrome: this value is adjusted by "System DPI/Scaling", but not 'D.browserZoom', and does not change when zooming. Because "System DPI/Scaling" is unknown, it can't be calculated.
#endif
   return App.desktop(); // this is not changed when device is rotated (obtained at app startup)
}
/******************************************************************************/
void Display::setShader(C Material *material)
{
   if(created())
   {
     _set_shader_material=material;
      {Meshes     .lock(); REPA(Meshes     )Meshes     .lockedData(i).setShader(); Meshes     .unlock();}
      {ClothMeshes.lock(); REPA(ClothMeshes)ClothMeshes.lockedData(i).setShader(); ClothMeshes.unlock();}
                                                       if(set_shader)set_shader();
     _set_shader_material=null;
   }
}
Display& Display::drawNullMaterials(Bool on)
{
   if(_draw_null_mtrl!=on)
   {
     _draw_null_mtrl=on;
      setShader();
   }
   return T;
}
void Display::screenChanged(Flt old_width, Flt old_height)
{
   if(old_width>0 && old_height>0)
   {
                     Gui.screenChanged(old_width, old_height);
      if(screen_changed)screen_changed(old_width, old_height);
   }
}
#if DX9
Bool Display::validUsage(UInt usage, D3DRESOURCETYPE res_type, Int image_type)
{
   if(D3DBase)
   {
      D3DFORMAT format=ImageTypeToFormat(image_type);
      return OK(D3DBase->CheckDeviceFormat(D3DADAPTER_DEFAULT, _no_gpu ? D3DDEVTYPE_NULLREF : D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, usage, res_type, format))
          || OK(D3DBase->CheckDeviceFormat(D3DADAPTER_DEFAULT, _no_gpu ? D3DDEVTYPE_NULLREF : D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, usage, res_type, format));
   }
   return false;
}
#endif
Str8 Display::shaderModelName()C
{
   switch(shaderModel())
   {
      default        : return "Unknown"; // SM_UNKNOWN
      case SM_GL_ES_2: return "GL ES 2";
      case SM_GL_ES_3: return "GL ES 3";
      case SM_GL     : return "GL";
      case SM_3      : return "3";
      case SM_4      : return "4";
      case SM_4_1    : return "4.1";
      case SM_5      : return "5";
   }
}
Str8 Display::apiName()C
{
#if DX9
   return "DirectX 9";
#elif DX11
   return "DirectX 11";
#elif DX12
   return "DirectX 12";
#elif METAL
   return "Metal";
#elif VULKAN
   return "Vulkan";
#elif WEB // check this first before 'GL' and 'GL_ES'
   return "Web GL";
#elif GL_ES // check this first before 'GL'
   return "OpenGL ES";
#elif GL
   return "OpenGL";
#endif
}
Bool Display::smallSize()C
{
#if WINDOWS_NEW
   Dbl inches; if(OK(GetIntegratedDisplaySize(&inches)))return inches<7;
#elif ANDROID
   Int    size=((AndroidApp && AndroidApp->config) ? AConfiguration_getScreenSize(AndroidApp->config) : ACONFIGURATION_SCREENSIZE_NORMAL);
   return size==ACONFIGURATION_SCREENSIZE_SMALL || size==ACONFIGURATION_SCREENSIZE_NORMAL;
   // HTC EVO 3D             ( 4   inch) has ACONFIGURATION_SCREENSIZE_NORMAL
   // Samsung Galaxy Note 2  ( 5.5 inch) has ACONFIGURATION_SCREENSIZE_NORMAL
   // Asus Transformer Prime (10   inch) has ACONFIGURATION_SCREENSIZE_XLARGE
#elif IOS
   // UI_USER_INTERFACE_IDIOM UIUserInterfaceIdiomPhone UIUserInterfaceIdiomPad 
   return UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone;
#endif
   return false;
}
Flt Display::browserZoom()C
{
#if WEB
   return emscripten_get_device_pixel_ratio();
#else
   return 1;
#endif
}
Bool Display::deferredUnavailable  ()C {return created() &&       _max_rt<2     ;} // deferred requires at least 2 MRT's (#0 Color, #1 Nrm, #2 Vel optional)
Bool Display::deferredMSUnavailable()C {return created() && shaderModel()<SM_4_1;} // only Shader Model 4.1 (DX 10.1) and above support multi-sampled RT's
/******************************************************************************/
// MONITOR
/******************************************************************************/
Display::Monitor::Monitor()
{
   primary=false;
   full=work.zero();
}
#if WINDOWS_OLD
Bool Display::Monitor::set(HMONITOR monitor)
{
   MONITORINFO monitor_info; Zero(monitor_info); monitor_info.cbSize=SIZE(monitor_info);
   if(GetMonitorInfo(monitor, &monitor_info))
   {
      full.set(monitor_info.rcMonitor.left, monitor_info.rcMonitor.top, monitor_info.rcMonitor.right, monitor_info.rcMonitor.bottom);
      work.set(monitor_info.rcWork   .left, monitor_info.rcWork   .top, monitor_info.rcWork   .right, monitor_info.rcWork   .bottom);
      primary=FlagTest(monitor_info.dwFlags, MONITORINFOF_PRIMARY);
      return true;
   }
   return false;
}
static BOOL CALLBACK EnumMonitors(HMONITOR hmonitor, HDC dc, LPRECT rect, LPARAM dwData) {D._monitors.get(hmonitor); return true;} // continue
#elif WINDOWS_NEW
void Display::Monitor::set(C DXGI_OUTPUT_DESC &desc)
{
   primary=false;
   full=work.set(desc.DesktopCoordinates.left, desc.DesktopCoordinates.top, desc.DesktopCoordinates.right, desc.DesktopCoordinates.bottom);
}
#endif
static Bool Create(Display::Monitor &monitor, C Ptr &hmonitor, Ptr user)
{
#if WINDOWS_OLD
   return monitor.set(HMONITOR(hmonitor));
#else
   return true;
#endif
}
void Display::monitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size, C Monitor *monitor)C
{
   if(monitor)
   {
      full=monitor->full;
      work=monitor->work;
   }else
   {
      full.set(0, 0, App.desktopW(), App.desktopH());
      work=App.desktopArea();
   }
   max_normal_win_client_size.set(full.w()-App._bound.w(), full.h()-App._bound.h());
    maximized_win_client_size.set(work.w()+App._bound_maximized.min.x+App._bound_maximized.max.x, work.h()+App._bound_maximized.min.y+App._bound_maximized.max.y);
}
void Display::curMonitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size)
{
   Monitor *monitor=null;
#if WINDOWS_OLD
   if(App.hwnd()) // adjust to current monitor
   {
   #if 1
      if(HMONITOR hmonitor=MonitorFromWindow(App.Hwnd(), MONITOR_DEFAULTTONEAREST))
   #else
      RectI win_rect=WindowRect(false); // watch out because 'WindowRect' can return weird position when the window is minimized
      POINT p; p.x=win_rect.centerXI(); p.y=win_rect.centerYI();
      if(HMONITOR hmonitor=MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST))
   #endif
         monitor=_monitors.get(hmonitor);
   }
#elif WINDOWS_NEW
   if(SwapChain)
   {
      IDXGIOutput *output=null;
      {
         SyncLocker locker(_lock);
         if(SwapChain)SwapChain->GetContainingOutput(&output);
      }
      if(output)
      {
         DXGI_OUTPUT_DESC desc; if(OK(output->GetDesc(&desc)))
         {
            monitor=_monitors.get(desc.Monitor);
            if(monitor && !monitor->full.w())monitor->set(desc);
         }
         output->Release();
      }
   }
#endif
   T.monitor(full, work, max_normal_win_client_size, maximized_win_client_size, monitor);
}
void Display::mainMonitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size)C
{
 C Monitor *main=null;
   REPA(_monitors){C Monitor &monitor=_monitors[i]; if(monitor.primary){main=&monitor; break;}}
   monitor(full, work, max_normal_win_client_size, maximized_win_client_size, main);
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
Display::Display() : _monitors(Compare, Create, null, 4)
{
  _full            =MOBILE; // by default request fullscreen for MOBILE, on WINDOWS_PHONE this will hide the navigation bar
  _sync            =true;
  _exclusive       =true;
  _hp_col_rt       =false;
  _hp_nrm_rt       =false;
  _hp_lum_rt       =false;
  _hp_nrm_calc     =true;
  _dither          =true;
  _mtrl_blend      =true;
  _device_mem      =-1;
  _monitor_prec    =IMAGE_PRECISION_8;
  _lit_col_rt_prec =IMAGE_PRECISION_8;
  _aspect_mode     =(MOBILE ? ASPECT_SMALLER : ASPECT_Y);
  _tex_filter      =(MOBILE ? 2 : 16);
  _tex_mip_filter  =!MOBILE;
  _tex_detail      =(MOBILE ? TEX_USE_DISABLE : TEX_USE_MULTI);
  _density_filter  =(MOBILE ? FILTER_LINEAR : FILTER_CUBIC_FAST);
  _tex_lod         =0;
  _tex_macro       =true;
  _tex_reflect     =TEX_USE_MULTI;
  _font_sharpness  =0.75f;
  _bend_leafs      =true;
  _particles_soft  =!MOBILE;
  _particles_smooth=!MOBILE;

  _initialized=false;
  _resetting  =false;
  _began      =false;

  _allow_stereo=true;
  _density=127;
  _samples=1;
  _scale=1;
  _aspect_ratio=_aspect_ratio_want=_pixel_aspect=0;
  _pixel_size=_pixel_size_2=_pixel_size_inv=0;
  _window_pixel_to_screen_mul=1; // init to 1 to avoid div by 0 at app startup which could cause crash on Web
  _window_pixel_to_screen_add=0;
  _window_pixel_to_screen_scale=1;

  _amb_mode    =AMBIENT_FLAT;
  _amb_soft    =1;
  _amb_jitter  =true;
  _amb_normal  =true;
  _amb_res     =FltToByteScale(0.5f);
  _amb_color   =0.4f;
  _amb_contrast=1.2f;
  _amb_range   =0.3f;
  _amb_scale   =2.5f;
  _amb_bias    =0.3f;

  _ns_color.zero();

  _vol_light=false;
  _vol_add  =false;
  _vol_max  =1.0;

  _shd_mode           =(MOBILE ? SHADOW_NONE : SHADOW_MAP);
  _shd_soft           =0;
  _shd_jitter         =false;
  _shd_reduce         =false;
  _shd_frac           =1;
  _shd_fade           =1;
  _shd_map_num        =6;
  _shd_map_size       =1024;
  _shd_map_size_actual=0;
  _shd_map_size_l     =1;
  _shd_map_size_c     =1;
  _shd_map_split      .set(2, 1);
  _cld_map_size       =128;

  _bump_mode=(MOBILE ? BUMP_FLAT : BUMP_PARALLAX);

  _mtn_mode  =MOTION_NONE;
  _mtn_dilate=DILATE_ORTHO2;
  _mtn_scale =0.04f;
  _mtn_res   =FltToByteScale(1.0f/3);

  _dof_mode     =DOF_NONE;
  _dof_foc_mode =false;
//_dof_focus    =0;
  _dof_range    =30;
  _dof_intensity=1;

  _eye_adapt           =false;
  _eye_adapt_brightness=0.37f;
  _eye_adapt_max_dark  =0.5f;
  _eye_adapt_max_bright=2.0f;
  _eye_adapt_speed     =6.5f;
  _eye_adapt_weight.set(0.9f, 1, 0.7f); // use smaller value for blue, to make blue skies have brighter multiplier, because human eye sees blue color as darker than others

  _grass_range  =50;
  _grass_density=(MOBILE ? 0.5f : 1);
  _grass_shadow =false;
  _grass_mirror =false;

  _bloom_original=1.0f;
  _bloom_scale   =0.5f;
  _bloom_cut     =0.3f;
  _bloom_blurs   =1;
  _bloom_sat     =false;
  _bloom_max     =false;
  _bloom_half    =!MOBILE;
  _bloom_samples =!MOBILE;
          _bloom_allow=!MOBILE;
           _glow_allow=!MOBILE;
  _color_palette_allow=!MOBILE;

  _lod_factor       =1;
  _lod_factor_shadow=2;
  _lod_factor_mirror=2;

  _tesselation          =false;
  _tesselation_allow    =true;
  _tesselation_heightmap=false;
  _tesselation_density  =60;

  _outline_sky =false;
  _outline_mode=EDGE_DETECT_THIN;
  _edge_detect =EDGE_DETECT_NONE;
  _edge_soften =EDGE_SOFTEN_NONE;

  _fur_gravity  =-1    ;
  _fur_vel_scale=-0.75f;

  _eye_dist          =0.064f;

  _view_square_pixel =false;
  _view_main.fov_mode=FOV_Y;
  _view_fov          =
  _view_main.fov.x   =
  _view_main.fov.y   =DegToRad(70);
  _view_main.from    =_view_from=0.05f;
  _view_main.range   =100;
  _view_main.full    =true; // needed for 'viewReset' which will always set full viewport if last was full too

  _smaa_threshold=0.1f;
}
void Display::init() // make this as a method because if we put this to Display constructor, then 'SecondaryContexts' may not have been initialized yet
{
   secondaryOpenGLContexts(1); // default 1 secondary context

   // re-use cached result obtained at app startup, because if the app is currently fullscreen at a custom resolution, then the monitor will return that resolution, however this function is used for getting default resolutions
#if WINDOWS_OLD
   EnumDisplayMonitors(null, null, EnumMonitors, 0); // list all monitors at app startup so we can know their original sizes
#elif WINDOWS_NEW
   IDXGIFactory1 *factory=null; CreateDXGIFactory1(__uuidof(IDXGIFactory1), (Ptr*)&factory); if(factory)
   {
      IDXGIAdapter *adapter=null; factory->EnumAdapters(0, &adapter); if(adapter)
      {
         for(Int i=0; ; i++)
         {
            IDXGIOutput *output=null; adapter->EnumOutputs(i, &output); if(output)
            {
               DXGI_OUTPUT_DESC desc; if(OK(output->GetDesc(&desc)))_monitors.get(desc.Monitor)->set(desc);
               output->Release();
            }else break;
         }
         adapter->Release();
      }
      factory->Release();
   }
#endif
}
/******************************************************************************/
void Display::del()
{
   Gui.del(); // deleting Gui should be outside of '_lock' lock (because for example it can wait for a thread working in the background which is using '_lock')

   SyncLocker locker(_lock);

  _initialized=false;

             gamma(0); // reset gamma when closing app
                end();
       VR.delImages();
           ShutFont();
         ShutVtxInd();
  DisplayState::del();
             Sh.del();
       Renderer.del();
         Images.del();
         _modes.del();

#if DX9
   RELEASE(Query  );
   RELEASE(D3D    );
   RELEASE(D3DBase);
#elif DX11
   if(SwapChain)SwapChain->SetFullscreenState(false, null); // full screen state must be first disabled, according to http://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx#Destroying
   RELEASE(SwapChain);
   RELEASE(Output);
   RELEASE(Adapter);
   RELEASE(Factory);
   RELEASE(Query);
   RELEASE(D3DC1);
   RELEASE(D3DC);
   RELEASE(D3D);
#elif GL
      if(FBO){glDeleteFramebuffers(1, &FBO); FBO=0;}
      if(VAO){glDeleteVertexArrays(1, &VAO); VAO=0;}
      SecondaryContexts.del();
           MainContext .del();
   #if WINDOWS
      if(hDC){ReleaseDC(App.Hwnd(), hDC); hDC=null;}
      SetDisplayMode(0); // switch back to the desktop
   #elif MAC
      [OpenGLContext release]; OpenGLContext=null;
   #elif LINUX
      SetDisplayMode(0); // switch back to the desktop
      if(vid_mode){XFree(vid_mode); vid_mode=null;} vid_modes=0; // free after 'SetDisplayMode'
   #elif ANDROID
      if(GLDisplay){eglTerminate(GLDisplay); GLDisplay=null;}
   #endif
#endif
}
/******************************************************************************/
#if DX9
// codes used for detecting GPU VRAM
#define DDENUM_ATTACHEDSECONDARYDEVICES 0x00000001L
typedef BOOL (FAR PASCAL*LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef HRESULT (WINAPI*LPDIRECTDRAWENUMERATEEXA)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef BOOL (WINAPI*PfnCoSetProxyBlanket)(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);
static CLSID CLSID_WbemLocator={0x4590F811, 0x1D3A, 0x11D0, {0x89, 0x1F, 0, 0xAA, 0, 0x4B, 0x2E, 0x24}};
static GUID   IID_IWbemLocator={0xDC12A687, 0x737F, 0x11CF, {0x88, 0x4D, 0, 0xAA, 0, 0x4B, 0x2E, 0x24}};
struct Match
{
   HMONITOR monitor;
   Char8    driver_name[512];
   Bool     found;

   Match(HMONITOR monitor) : monitor(monitor) {found=false; driver_name[0]=0;}
};
static BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
   Match &match=*(Match*)lpContext;
   if(match.monitor==hm)
   {
          match.found=true;
      Set(match.driver_name, lpDriverName);
      return false; // stop enumerating
   }
   return true; // keep looking
}
static Bool GetDeviceIDFromHMonitor(HMONITOR hm, WCHAR *strDeviceID, int cchDeviceID)
{
   DLL ddraw;
   if( ddraw.createFile(u"Ddraw.dll"))
      if(LPDIRECTDRAWENUMERATEEXA DirectDrawEnumerateEx=(LPDIRECTDRAWENUMERATEEXA)ddraw.getFunc("DirectDrawEnumerateExA"))
   {
      Match match(hm); DirectDrawEnumerateEx(DDEnumCallbackEx, &match, DDENUM_ATTACHEDSECONDARYDEVICES);
      if(match.found)
      {
         DISPLAY_DEVICEA dispdev; Zero(dispdev); dispdev.cb=SIZE(dispdev);
         for(Int i=0; EnumDisplayDevicesA(null, i, &dispdev, 0); i++)
            if(!(dispdev.StateFlags&DISPLAY_DEVICE_MIRRORING_DRIVER   )  // skip devices that are monitors that echo another display
            &&  (dispdev.StateFlags&DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) // process only devices that are attached
            &&  Equal(match.driver_name, dispdev.DeviceName))
         {
            MultiByteToWideChar(CP_ACP, 0, dispdev.DeviceID, -1, strDeviceID, cchDeviceID);
            return true;
         }
      }
   }
   return false;
}
static Long DeviceMemory(Int adapter_index)
{
   Long size=-1;
   if(HMONITOR monitor=D3DBase->GetAdapterMonitor(adapter_index))
   {
      WCHAR strInputDeviceID[1024];
      if(GetDeviceIDFromHMonitor(monitor, strInputDeviceID, Elms(strInputDeviceID)))
      {
         IWbemLocator *pIWbemLocator=null; CoCreateInstance(CLSID_WbemLocator, null, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (Ptr*)&pIWbemLocator);
         if(           pIWbemLocator)
         {
            // Using the locator, connect to WMI in the given namespace
            BSTR Namespace=SysAllocString(L"\\\\.\\root\\cimv2");
            IWbemServices *pIWbemServices=null; pIWbemLocator->ConnectServer(Namespace, null, null, 0, 0, null, null, &pIWbemServices);
            if(            pIWbemServices)
            {
               DLL ole;
               if( ole.createFile(u"Ole32.dll"))
                  if(PfnCoSetProxyBlanket pfnCoSetProxyBlanket=(PfnCoSetProxyBlanket)ole.getFunc("CoSetProxyBlanket"))
                     pfnCoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, null, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, null, 0); // Switch security level to IMPERSONATE

               BSTR Win32_VideoController=SysAllocString(L"Win32_VideoController");
               IEnumWbemClassObject *pEnumVideoControllers=null; pIWbemServices->CreateInstanceEnum(Win32_VideoController, 0, null, &pEnumVideoControllers);
               if(                   pEnumVideoControllers)
               {
                  BSTR PNPDeviceID=SysAllocString(L"PNPDeviceID"),
                        AdapterRAM=SysAllocString(L"AdapterRAM");
                  IWbemClassObject *video_controllers[16]={0};
                  DWORD returned=0;
                  Bool  found=false;
                  pEnumVideoControllers->Reset(); // Get the first one in the list
                  if(OK(pEnumVideoControllers->Next(5000, Elms(video_controllers), video_controllers, &returned))) // 5 second timeout
                  {
                     FREP(returned)if(IWbemClassObject *controller=video_controllers[i])
                     {
                        if(!found)
                        {
                           VARIANT var;
                           if(OK(controller->Get(PNPDeviceID, 0, &var, null, null)))if(wcsstr(var.bstrVal, strInputDeviceID))found=true;
                           VariantClear(&var);

                           if(found)
                           {
                              if(OK(controller->Get(AdapterRAM, 0, &var, null, null)))size=var.ulVal;
                              VariantClear(&var);
                           }
                        }
                        controller->Release();
                     }
                  }
                  if(AdapterRAM )SysFreeString(AdapterRAM);
                  if(PNPDeviceID)SysFreeString(PNPDeviceID);
                  pEnumVideoControllers->Release();
               }
               if(Win32_VideoController)SysFreeString(Win32_VideoController);
               pIWbemServices->Release();
            }
            if(Namespace)SysFreeString(Namespace);
            pIWbemLocator->Release();
         }
      }
   }
   return size;
}
#endif
void Display::createDevice()
{
   if(LogInit)LogN("Display.createDevice");
   SyncLocker locker(_lock);
#if DX9
   if(!(D3DBase=Direct3DCreate9(D3D_SDK_VERSION)))Exit(MLTC(u"Direct3D not found.\nPlease install the newest DirectX.", PL,u"Direct3D nie odnaleziony.\nProszę zainstalować najnowszy DirectX."));

   // get device description
   if(!deviceName().is()){D3DADAPTER_IDENTIFIER9 id; if(OK(D3DBase->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id)))_device_name=id.Description;}

   // check required capabilites
   D3DCAPS9 caps; D3DBase->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
   if(caps.PrimitiveMiscCaps&D3DPMISCCAPS_NULLREFERENCE)Exit(MLTC(u"Video Card supporting minimum requirements not found.", PL,u"Nie odnaleziono karty graficznej spełniającej minimalne wymagania."));
   Int vs_ver  =D3DSHADER_VERSION_MAJOR(caps.VertexShaderVersion),
       ps_ver  =D3DSHADER_VERSION_MAJOR(caps. PixelShaderVersion);
  _shader_model=((ps_ver>=3) ? SM_3 : SM_UNKNOWN);

   if(shaderModel()<SM_3)if(!(App.flag&APP_ALLOW_NO_GPU))Exit(MLTC(u"Minimum Shader Model 3.0 required.\nA better video card or installing drivers is required.",
                                                               PL, u"Minimum Shader Model 3.0 wymagany.\nWymagana jest lepsza kart graficzna lub doinstalowanie sterowników."));

   // set create options
   UInt flag=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
   if((caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT) && vs_ver>=ps_ver) // use hardware vertex processing only if HW T&L and VS ver matches PS ver (in HW)
   {
   #if 1
      flag=D3DCREATE_MIXED_VERTEXPROCESSING; // this was tested and results in better performance for dynamic vertex buffers than D3DCREATE_HARDWARE_VERTEXPROCESSING, TODO: however https://msdn.microsoft.com/en-us/library/windows/desktop/bb172527(v=vs.85).aspx mentions that on Win10 this should be avoided, however tests are inconclusive, check again in the future using "Tests/Vertex Buffering.cpp"
   #else
      flag=D3DCREATE_HARDWARE_VERTEXPROCESSING;
      if(caps.DevCaps&D3DDEVCAPS_PUREDEVICE)flag|=D3DCREATE_PUREDEVICE; // this requires D3DCREATE_HARDWARE_VERTEXPROCESSING
   #endif
   }
 //if(App.flag&APP_DX_THREAD_SAFE)flag|=D3DCREATE_MULTITHREADED               ;
 //if(App.flag&APP_DX_MANAGEMENT )flag|=D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;
                                  flag|=D3DCREATE_FPU_PRESERVE                ;

   // enumerate display modes
	MemtN<VecI2, 128> modes;
   for(Int i=0; ; i++)
   {
      DEVMODE mode; Zero(mode); mode.dmSize=SIZE(mode);
      if(!EnumDisplaySettings(null, i, &mode))break;
      modes.include(VecI2(mode.dmPelsWidth, mode.dmPelsHeight));
   }
  _modes=modes;
  _modes.sort(Compare);

   // init
   if(!findMode())Exit("Valid display mode not found.");
   if(!exclusive() && full()){if(!SetDisplayMode(2))Exit("Can't set fullscreen mode."); adjustWindow();}
   if(OK(D3DBase->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, App.Hwnd(), flag, &D3DPP, &D3D)))
   {
     _can_draw=true;
     _no_gpu  =false;
   }else
   {
     _can_draw    =false;
     _no_gpu      =true;
     _shader_model=SM_3;
      if((App.flag&APP_ALLOW_NO_GPU) ? !OK(D3DBase->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, App.Hwnd(), flag, &D3DPP, &D3D)) : true)Exit(MLTC(u"Can't create Direct3D Device.", PL,u"Nie można utworzyć Direct3D."));
   }
   if(D3D)
   {
     _device_mem=DeviceMemory(D3DADAPTER_DEFAULT);
    //if(_device_mem<0)_device_mem=D3D->GetAvailableTextureMem(); // this is the total memory, including system memory (for example 4GB is returned when having only 2GB GPU)
      D3D->CreateQuery(D3DQUERYTYPE_EVENT, &Query);
   }
#elif DX11
   UInt flags=(D3D_DEBUG ? D3D11_CREATE_DEVICE_DEBUG : 0); // DO NOT include D3D11_CREATE_DEVICE_SINGLETHREADED to allow multi-threaded resource creation - https://docs.microsoft.com/en-us/windows/desktop/direct3d11/overviews-direct3d-11-render-multi-thread

   // ADAPTER = GPU
   // OUTPUT  = MONITOR

   U64 adapter_id=VR._adapter_id;
   if( adapter_id) // if want a custom adapter
   {
      IDXGIFactory1 *factory=null; CreateDXGIFactory1(__uuidof(IDXGIFactory1), (Ptr*)&factory); if(factory)
      {
         for(Int i=0; OK(factory->EnumAdapters(i, &Adapter)); i++)
         {
            if(!Adapter)break;
         #if DEBUG
            IDXGIOutput *output=null; for(Int i=0; OK(Adapter->EnumOutputs(i, &output)); i++)
            {
               DXGI_OUTPUT_DESC desc; output->GetDesc(&desc);
               RELEASE(output);
            }
         #endif
            DXGI_ADAPTER_DESC desc; if(OK(Adapter->GetDesc(&desc)))
            {
               ASSERT(SIZE(desc.AdapterLuid)==SIZE(adapter_id));
               if(EqualMem(&adapter_id, &desc.AdapterLuid, SIZE(adapter_id)))break; // if this is the adapter, then use it and don't look any more
            }
            RELEASE(Adapter);
         }
         RELEASE(factory); // release 'factory' because we need to obtain it from the D3D Device in case it will be different
      }
   }

   D3D_FEATURE_LEVEL *feature_level_force=null;
#if   FORCE_D3D9_3
   D3D_FEATURE_LEVEL fl=D3D_FEATURE_LEVEL_9_3 ; feature_level_force=&fl;
#elif FORCE_D3D10_0
   D3D_FEATURE_LEVEL fl=D3D_FEATURE_LEVEL_10_0; feature_level_force=&fl;
#elif FORCE_D3D10_1
   D3D_FEATURE_LEVEL fl=D3D_FEATURE_LEVEL_10_1; feature_level_force=&fl;
#endif

   if(OK(D3D11CreateDevice(Adapter, Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, null, flags, feature_level_force, feature_level_force ? 1 : 0, D3D11_SDK_VERSION, &D3D, &FeatureLevel, &D3DC)))
   {
     _can_draw=true;
     _no_gpu  =false;
      if(FeatureLevel<D3D_FEATURE_LEVEL_10_0)Exit("Minimum D3D Feature Level 10.0 required.\nA better GPU is required.");
   }else
   {
     _can_draw=true; // we can still draw on DX10+ by using D3D_DRIVER_TYPE_WARP
     _no_gpu  =true;
      if((App.flag&APP_ALLOW_NO_GPU) ? !OK(D3D11CreateDevice(null, D3D_DRIVER_TYPE_WARP, null, flags, null, 0, D3D11_SDK_VERSION, &D3D, &FeatureLevel, &D3DC)) : true)Exit(MLTC(u"Can't create Direct3D Device.", PL,u"Nie można utworzyć Direct3D."));
      RELEASE(Adapter); // D3D may have gotten a different adapter
   }
   if(D3D_DEBUG)D3D->SetExceptionMode(D3D11_RAISE_FLAG_DRIVER_INTERNAL_ERROR);

   D3DC->QueryInterface(__uuidof(ID3D11DeviceContext1), (Ptr*)&D3DC1);

  _shader_model=((FeatureLevel>=D3D_FEATURE_LEVEL_11_0) ? SM_5 : (FeatureLevel>=D3D_FEATURE_LEVEL_10_1) ? SM_4_1 : SM_4);

   IDXGIDevice1 *device=null; D3D->QueryInterface(__uuidof(IDXGIDevice1), (Ptr*)&device); if(device)
   {
      device->SetMaximumFrameLatency(1); // set max frame latency, for WINDOWS_OLD this doesn't seem to have any effect, however for WINDOWS_NEW it makes a big difference (it makes it work as WINDOWS_OLD), this may be related to the type of SwapChain, so always call this just in case, as without this, the latency is very slow (for example drawing something at mouse position and moving the mouse in circles)
      if(!Adapter)device->GetAdapter(&Adapter); // if adapter is unknown
      RELEASE(device);
   }
   if(!Adapter) // if adapter is unknown
   {
      IDXGIDevice *device=null; D3D->QueryInterface(__uuidof(IDXGIDevice), (Ptr*)&device); if(device)
      {
         device->GetAdapter(&Adapter);
         RELEASE(device);
      }
   }
   if(!Factory) // if Factory is unknown
   {
      if(Adapter)Adapter->GetParent(WINDOWS_OLD ? __uuidof(IDXGIFactory1) : __uuidof(IDXGIFactory2), (Ptr*)&Factory);
      if(!Factory)Exit("Can't access DXGIFactory.\nPlease install latest DirectX and Graphics Drivers.");
   }

   IDXGIFactory5 *factory5=null; Factory->QueryInterface(__uuidof(IDXGIFactory5), (Ptr*)&factory5); if(factory5)
   {
      int allow_tearing=false; // must be 'int' because 'bool' will fail
		if(OK(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, SIZE(allow_tearing))))AllowTearing=(allow_tearing!=0);
      factory5->Release();
   }

   if(Adapter)
   {
      DXGI_ADAPTER_DESC desc; if(OK(Adapter->GetDesc(&desc)))
      {
         if(!deviceName().is())_device_name=desc.Description;
        _device_mem=desc.DedicatedVideoMemory;
      }
      MemtN<VecI2, 128> modes; // store display modes for all outputs, in case user prefers to use another monitor rather than the main display
      IDXGIOutput *output=null; for(Int i=0; OK(Adapter->EnumOutputs(i, &output)); i++) // first output is always the main display
      {
         DXGI_FORMAT                                          mode=DXGI_FORMAT_R8G8B8A8_UNORM; // always use this mode in case system doesn't support 10-bit color
         UInt                                           descs_elms=0; output->GetDisplayModeList(mode, 0, &descs_elms, null); // get number of mode descs
         MemtN<DXGI_MODE_DESC, 128> descs; descs.setNum(descs_elms ); output->GetDisplayModeList(mode, 0, &descs_elms, descs.data()); // get mode descs
         FREPA(descs)modes.binaryInclude(VecI2(descs[i].Width, descs[i].Height), Compare); // add from the start to avoid unnecessary memory moves
         RELEASE(output);
      }
     _modes=modes;
   }

   // init
   if(!findMode())Exit("Valid display mode not found.");
#if WINDOWS_OLD
   if(!exclusive() && full()){if(!SetDisplayMode(2))Exit("Can't set fullscreen mode."); adjustWindow();}
again:
   Factory->CreateSwapChain(D3D, &SwapChainDesc, &SwapChain);
   if(!SwapChain && SwapChainDesc.BufferDesc.Format==DXGI_FORMAT_R10G10B10A2_UNORM){SwapChainDesc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; goto again;} // if failed with 10-bit then try again with 8-bit
   Factory->MakeWindowAssociation(App.Hwnd(), DXGI_MWA_NO_ALT_ENTER|DXGI_MWA_NO_WINDOW_CHANGES|DXGI_MWA_NO_PRINT_SCREEN); // this needs to be called after 'CreateSwapChain'
#else
again:
	Factory->CreateSwapChainForCoreWindow(D3D, (IUnknown*)App._hwnd, &SwapChainDesc, null, &SwapChain);
   if(!SwapChain && SwapChainDesc.Format==DXGI_FORMAT_R10G10B10A2_UNORM){SwapChainDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; goto again;} // if failed with 10-bit then try again with 8-bit
#endif
   if(!SwapChain)Exit("Can't create Direct3D Swap Chain.");
 //if( SwapChain && Output && !SwapChainDesc.Windowed)SwapChain->SetFullscreenState(true, Output); // if we want a custom output then we need to apply it now, otherwise the fullscreen can occur on the main display

   D3D11_QUERY_DESC query_desc={D3D11_QUERY_EVENT, 0};
   D3D->CreateQuery(&query_desc, &Query);
#elif GL
  _shader_model=(GL_ES ? SM_GL_ES_2 : SM_GL);
   if(FlagTest(App.flag, APP_ALLOW_NO_GPU)) // completely disable hardware on OpenGL because there's no way to know if it's available
   {
     _can_draw=false;
     _no_gpu  =true;
   }else
   {
     _can_draw=true;
     _no_gpu  =false;
   }
   #if WINDOWS
      PIXELFORMATDESCRIPTOR pfd=
      {
         SIZE(PIXELFORMATDESCRIPTOR), // size of 'pfd'
         1, // version number
         PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
         PFD_TYPE_RGBA,
         32, // color bits
         0, 0, 0, 0, 0, 0, // color bits ignored
         0, // no alpha buffer
         0, // shift bit ignored
         0, // no accumulation buffer
         0, 0, 0, 0, // accumulation bits ignored
         24, // 24-bit depth buffer
         8, // 8-bit stencil buffer
         0, // no auxiliary buffer
         PFD_MAIN_PLANE, // main drawing layer
         0, // reserved
         0, 0, 0 // layer masks ignored
      };
      Int PixelFormat;

      if(!(hDC                =            GetDC(App.Hwnd()            )))Exit("Can't create an OpenGL Device Context.");
      if(!(PixelFormat        =ChoosePixelFormat(hDC,              &pfd)))Exit("Can't find a suitable PixelFormat.");
      if(!(                       SetPixelFormat(hDC, PixelFormat, &pfd)))Exit("Can't set the PixelFormat.");
      if(!(MainContext.context= wglCreateContext(hDC                   )))Exit("Can't create an OpenGL Context.");
           MainContext.lock();

      if(glewInit()!=GLEW_OK || !GLEW_VERSION_3_2)Exit("OpenGL 3.2 support not available.\nGraphics Driver not installed or better video card is required."); // 3.2 needed for 'glDrawElementsBaseVertex', 3.1 needed for instancing, 3.0 needed for 'glColorMaski', 'gl_ClipDistance', 'glClearBufferfv', 'glGenVertexArrays', 'glMapBufferRange', otherwise 2.0 is good enough
         glewSafe();

      if(wglCreateContextAttribsARB)
      {
         const int attribs[]={WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                              WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                              0};
         if(HGLRC context=wglCreateContextAttribsARB(hDC, 0, attribs))
         {
            MainContext.del();
            MainContext.context=context;
            MainContext.lock();
         }
      }

      // enumerate display modes
      MemtN<VecI2, 128> modes;
      for(Int i=0; ; i++)
      {
         DEVMODE mode; Zero(mode); mode.dmSize=SIZE(mode);
         if(!EnumDisplaySettings(null, i, &mode))break;
         modes.include(VecI2(mode.dmPelsWidth, mode.dmPelsHeight));
      }
     _modes=modes;
     _modes.sort(Compare);
   #elif MAC
      const CGLPixelFormatAttribute profile_versions[]=
      {
         (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy  , // NSOpenGLProfileVersionLegacy
         (CGLPixelFormatAttribute)kCGLOGLPVersion_GL3_Core, // NSOpenGLProfileVersion3_2Core
         (CGLPixelFormatAttribute)kCGLOGLPVersion_GL4_Core, // NSOpenGLProfileVersion4_1Core
      };
      CGLPixelFormatObj pf=null;
      REPD (hw , 2) // HW acceleration, most important !! it's very important to check it first, in case device supports only 3.2 accelerated, and 4.1 perhaps could be done in software (if that's possible, it's very likely as one user with Intel HD 3300 which has 3.3 GL, reported poor performance without this) so first we check all profiles looking for accelerated, and if none are found, then try software !!
      REPAD(ver, profile_versions) // profile version
      REPD (buf, 2)
      {
         const CGLPixelFormatAttribute attribs[]=
         {
            buf ? kCGLPFATripleBuffer : kCGLPFADoubleBuffer, // triple/double buffered
            kCGLPFADepthSize  , (CGLPixelFormatAttribute)24, // depth buffer
            kCGLPFAStencilSize, (CGLPixelFormatAttribute) 8, // stencil
            kCGLPFAOpenGLProfile, profile_versions[ver], // version
            hw ? kCGLPFAAccelerated : kCGLPFAAllowOfflineRenderers, // HW/Soft
            (CGLPixelFormatAttribute)0, // end of list
         };
         GLint num_pixel_formats; CGLChoosePixelFormat(attribs, &pf, &num_pixel_formats); if(pf)goto found_pf;
      }
      Exit("Can't create an OpenGL Pixel Format.");
   found_pf:
      CGLCreateContext(pf, null, &MainContext.context);
      CGLDestroyPixelFormat(pf);
      if(!MainContext.context)Exit("Can't create an OpenGL Context.");
      if(MAC_GL_MT)Bool mt_ok=(CGLEnable(MainContext.context, kCGLCEMPEngine)!=kCGLNoError);
      MainContext.lock();

      OpenGLContext=[[NSOpenGLContext alloc] initWithCGLContextObj:MainContext.context];
      [OpenGLContext setView:OpenGLView];
      [App.Hwnd() makeKeyAndOrderFront:NSApp]; // show only after everything finished (including GL context to avoid any flickering)

      // enumerate display modes
      CGDirectDisplayID display[256];
      CGDisplayCount    displays=0;
      MemtN<VecI2, 128> modes;

      if(!CGGetActiveDisplayList(Elms(display), display, &displays))REP(displays)
         if(CFArrayRef display_modes=CGDisplayCopyAllDisplayModes(display[i], null))
      {
         REP(CFArrayGetCount(display_modes))
         {
            CGDisplayModeRef mode=(CGDisplayModeRef)CFArrayGetValueAtIndex(display_modes, i);
            UInt flags=CGDisplayModeGetIOFlags(mode);
            Bool ok   =FlagTest(flags, kDisplayModeSafetyFlags);
            if(  ok)modes.include(VecI2(CGDisplayModeGetWidth(mode), CGDisplayModeGetHeight(mode)));
         }
         CFRelease(display_modes);
      }
     _modes=modes;
     _modes.sort(Compare);
   #elif LINUX
      if(XDisplay)
      {
         #if 0 // 2.0 context
            if(!(MainContext.context=glXCreateNewContext(XDisplay, GLConfig, GLX_RGBA_TYPE, null, true)))Exit("Can't create a OpenGL Context.");
         #else // 3.0+ context (this does not link on some old graphics drivers when compiling, "undefined reference to glXCreateContextAttribsARB", it would need to be accessed using 'glXGetProcAddress')
            // access 'glXCreateContextAttribsARB', on Linux we don't need an existing GL context to be able to load extensions via 'glXGetProcAddressARB'
            typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC) (::Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
            PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB=(PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((C GLubyte*)"glXCreateContextAttribsARB");
            const int attribs[]=
            {
               GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
               GLX_CONTEXT_MINOR_VERSION_ARB, 2,
               0,
            };
            // create context
            if(!glXCreateContextAttribsARB
            || !(MainContext.context=glXCreateContextAttribsARB(XDisplay, GLConfig, null, true, attribs)))Exit("Can't create a OpenGL 3.2 Context.");
         #endif
         XSync(XDisplay, false); // Forcibly wait on any resulting X errors
         MainContext.lock();

         glXSwapInterval=(glXSwapIntervalType)glXGetProcAddressARB((C GLubyte*)"glXSwapIntervalEXT"); // access it via 'glXGetProcAddressARB' because some people have linker errors "undefined reference to 'glXSwapIntervalEXT'

         // get available modes
         MemtN<VecI2, 128> modes;
         if(XF86VidModeGetAllModeLines(XDisplay, DefaultScreen(XDisplay), &vid_modes, &vid_mode))for(int i=0; i<vid_modes; i++)
         {
            XF86VidModeModeInfo &vm=*vid_mode[i]; modes.include(VecI2(vm.hdisplay, vm.vdisplay));
         }
        _modes=modes;
        _modes.sort(Compare);
      }else
      {
        _can_draw=false;
        _no_gpu  =true;
         return;
      }
   #elif ANDROID
      if(LogInit)LogN("EGL");
      GLDisplay=eglGetDisplay(EGL_DEFAULT_DISPLAY); if(!GLDisplay)Exit("Can't get EGL Display"); if(eglInitialize(GLDisplay, null, null)!=EGL_TRUE)Exit("Can't initialize EGL Display");
      Bool has_alpha=false, bit16=false; Byte samples=1; IMAGE_TYPE ds_type=IMAGE_NONE;
      FREPD(gl_ver, 2)
         if(gl_ver || OSVerNumber().x>=18) // proceed only if we're trying GLES 2.0, or 3.0 AND AndroidAPI>=18 (which is the Android Version which started supporting 3.0), this is because Asus Transformer Prime TF201 succeeds with 3.0 context but it doesn't actually support it (TF201 has Android 4.1.1 which is API 16)
      {
         EGLint ctx_attribs[]=
         {
            EGL_CONTEXT_CLIENT_VERSION, (gl_ver==0) ? 3 : 2, // try OpenGL ES 3.0 context first, then fallback to 2.0
            EGL_NONE,
         };
         FREPD(c, 2) // colors  (process this as 1st in loop as it's most  important)
         FREPD(d, 3) // depth   (process this as 2nd in loop as it's more  important)
         FREPD(s, 2) // stencil (process this as 3rd in loop as it's less  important)
         FREPD(a, 2) // alpha   (process this as 4th in loop as it's least important)
         {
            has_alpha=(c==0 && a==0);
            bit16    =(c==1);
            ds_type  =((d==0) ? ((s==0) ? IMAGE_D24S8 : IMAGE_D24X8) : (d==1) ? IMAGE_D32 : IMAGE_D16);
            EGLint attribs[]=
            {
               EGL_SURFACE_TYPE   , EGL_WINDOW_BIT,
               EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
               EGL_BLUE_SIZE      , (c==0) ?  8 : 5,
               EGL_GREEN_SIZE     , (c==0) ?  8 : 6,
               EGL_RED_SIZE       , (c==0) ?  8 : 5,
               EGL_ALPHA_SIZE     , has_alpha ? 8 : 0,
               EGL_DEPTH_SIZE     , (d==0) ? 24 : (d==1) ? 32 : 16,
               EGL_STENCIL_SIZE   , (s==0) ?  8 : 0,
               EGL_NONE
            };
            if(LogInit)LogN(S+"Trying config GL:"+gl_ver+", C:"+c+", D:"+d+", S:"+s+", A:"+a);
            EGLint num_configs=0;
            if(eglChooseConfig(GLDisplay, attribs, &GLConfig, 1, &num_configs)==EGL_TRUE)
               if(num_configs>=1)
            {
               EGLint format; eglGetConfigAttrib(GLDisplay, GLConfig, EGL_NATIVE_VISUAL_ID, &format);
               ANativeWindow_setBuffersGeometry(AndroidApp->window, 0, 0, format);
               if(MainContext.surface=eglCreateWindowSurface(GLDisplay, GLConfig, AndroidApp->window, null))
               {
                  if(MainContext.context=eglCreateContext(GLDisplay, GLConfig, null, ctx_attribs))
                  {
                     if(gl_ver==0)_shader_model=SM_GL_ES_3; // we succeeded with creating a 3.0 context
                     goto context_ok;
                  }
                  MainContext.del();
               }
            }
         }
      }
      Exit("Can't create an OpenGL Context.");
   context_ok:
      MainContext.lock();
      if(LogInit)LogN("EGL OK");
      EGLint width, height;
      eglQuerySurface(GLDisplay, MainContext.surface, EGL_WIDTH , &width );
      eglQuerySurface(GLDisplay, MainContext.surface, EGL_HEIGHT, &height);
      Renderer._main   .forceInfo(width, height, 1, bit16 ? IMAGE_B5G6R5 : has_alpha ? IMAGE_R8G8B8A8 : IMAGE_R8G8B8X8, IMAGE_SURF)._samples=samples;
      Renderer._main_ds.forceInfo(width, height, 1, ds_type                                                           , IMAGE_DS  )._samples=samples;
      if(LogInit)LogN(S+"Renderer._main: "+Renderer._main.w()+'x'+Renderer._main.h()+", type: "+ImageTI[Renderer._main.hwType()].name+", ds_type: "+ImageTI[Renderer._main_ds.hwType()].name);
   #elif IOS
      if(MainContext.context=[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3])_shader_model=SM_GL_ES_3;else
      if(MainContext.context=[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2])
      {
        _shader_model=SM_GL_ES_2;
         // iOS has a bug in which it falsely returns success for creating ETC formats on OpenGL ES 2 even though they're not supported, so as a workaround, disable them completely
         ConstCast(ImageTI[IMAGE_ETC1   ].format)=0;
         ConstCast(ImageTI[IMAGE_ETC2   ].format)=0;
         ConstCast(ImageTI[IMAGE_ETC2_A1].format)=0;
         ConstCast(ImageTI[IMAGE_ETC2_A8].format)=0;
      }else Exit("Can't create a OpenGL ES 2.0 Context.");
      MainContext.context.multiThreaded=false; // disable multi-threaded rendering as enabled actually made things slower, TOOD: check again in the future !! if enabling then probably all contexts have to be enabled as well, secondary too, because VAO from VBO's on a secondary thread could fail, as in Dungeon Hero, needs checking !!
      MainContext.lock();
   #elif WEB
      EmscriptenWebGLContextAttributes attrs;
      emscripten_webgl_init_context_attributes(&attrs);
      attrs.minorVersion=0;
      attrs.alpha       =false; // this would enable compositing graphics using transparency onto the web page
      attrs.depth       =true;
      attrs.stencil     =true;
      attrs.antialias   =false;
      attrs.preserveDrawingBuffer=false;
      attrs.enableExtensionsByDefault=true;
      attrs.preferLowPowerToHighPerformance=false;
      for(Int gl_ver=2; gl_ver>=1; gl_ver--) // start from WebGL 2.0 (ES3) down to 1.0 (ES2)
      {
         attrs.majorVersion=gl_ver;
         if(MainContext.context=emscripten_webgl_create_context(null, &attrs))
         {
           _shader_model=((gl_ver>=2) ? SM_GL_ES_3 : SM_GL_ES_2);
            goto context_ok;
         }
      }
      Exit("Can't create an OpenGL Context.");
   context_ok:
      MainContext.lock();
      Byte samples=(attrs.antialias ? 4 : 1);
      int  width, height; emscripten_get_canvas_element_size(null, &width, &height);
      Renderer._main   .forceInfo(width, height, 1, IMAGE_R8G8B8A8                           , IMAGE_SURF)._samples=samples;
      Renderer._main_ds.forceInfo(width, height, 1, attrs.stencil ? IMAGE_D24S8 : IMAGE_D24X8, IMAGE_DS  )._samples=samples;
#endif

   if(!deviceName().is())
   {
     _device_name=(CChar8*)glGetString(GL_RENDERER);
   #if LINUX
     _device_name.removeOuterWhiteChars(); // Linux may have unnecessary spaces at the end
   #endif
   }

   if(LogInit)LogN("Secondary Contexts");
   if(SecondaryContexts.elms())
   {
      REPA(SecondaryContexts)if(!SecondaryContexts[i].createSecondary())
      {
         LogN(S+"Failed to create a Secondary OpenGL Context"
         #if ANDROID
            +", error:"+eglGetError()
         #endif
         );
         SecondaryContexts.remove(i); // remove after error code was displayed
      }
      MainContext.lock(); // lock main context because secondary were locked during creation to set some things
   }
   if(LogInit)LogN("Secondary Contexts OK");

   if(D.shaderModelGLES2()) // GLES2 requires internalFormat to be as follows, without it WebGL1 will fail - https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glTexImage2D.xml
   {
      #define GL_LUMINANCE       0x1909
      #define GL_LUMINANCE_ALPHA 0x190A
      ConstCast(ImageTI[IMAGE_R8G8B8A8].format)=GL_RGBA;
      ConstCast(ImageTI[IMAGE_R8G8B8  ].format)=GL_RGB;
      ConstCast(ImageTI[IMAGE_L8A8    ].format)=GL_LUMINANCE_ALPHA;
      ConstCast(ImageTI[IMAGE_L8      ].format)=GL_LUMINANCE;
      ConstCast(ImageTI[IMAGE_A8      ].format)=GL_ALPHA;
      ConstCast(ImageTI[IMAGE_F32_4   ].format)=GL_RGBA;
      ConstCast(ImageTI[IMAGE_F32_3   ].format)=GL_RGB;
      ConstCast(ImageTI[IMAGE_F16_4   ].format)=GL_RGBA;
      ConstCast(ImageTI[IMAGE_F16_3   ].format)=GL_RGB;
   }

   if(LogInit)
   {
      LogN(S+"Device Name: "      +_device_name);
      LogN(S+"Device Vendor: "    +(CChar8*)glGetString(GL_VENDOR    ));
      LogN(S+"Device Version: "   +(CChar8*)glGetString(GL_VERSION   ));
      LogN(S+"Device Extensions: "+(CChar8*)glGetString(GL_EXTENSIONS));
   #if GL_ES
      int i;
      glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS  , &i); LogN(S+"GL_MAX_VERTEX_UNIFORM_VECTORS: "+i);
      glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &i); LogN(S+"GL_MAX_FRAGMENT_UNIFORM_VECTORS: "+i);
   #endif
   }

   if(!findMode())Exit("Valid display mode not found.");
   setSync();

   if(LogInit)LogN("FBO");
                               glGenFramebuffers(1, &FBO); if(!FBO)Exit("Couldn't create OpenGL Frame Buffer Object (FBO)");
   if(D.notShaderModelGLES2()){glGenVertexArrays(1, &VAO); if(!VAO)Exit("Couldn't create OpenGL Vertex Arrays (VAO)");}

	#if WINDOWS
      if(full()){if(!SetDisplayMode(2))Exit("Can't set fullscreen mode."); adjustWindow();}
   #elif MAC
      if(!SetDisplayMode(2))Exit("Can't set display mode.");
   #elif LINUX
      if(full()){if(!SetDisplayMode(2))Exit("Can't set display mode."); adjustWindow();} // 'adjustWindow' because we need to set fullscreen state
   #elif IOS
      fbo(FBO); // set custom frame buffer, on iOS there's only one FBO and one FBO change, and it is here, this is because there's no default(0) fbo on this platform
   #endif

   // call these as soon as possible because they affect all images (including those created in the renderer)
	glPixelStorei(GL_PACK_ALIGNMENT  , 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

#if MOBILE
   T._modes.setNum(2);
   T._modes[1]=T._modes[0]=screen();
   T._modes[1].swap();
#endif
}
void Display::androidClose()
{
#if ANDROID
   SyncLocker locker(_lock);
	if(GLDisplay)
	{
         MainContext.unlock();
      if(MainContext.surface)eglDestroySurface(GLDisplay, MainContext.surface);
	}
   MainContext.surface=null;
#endif
}
void Display::androidOpen()
{
#if ANDROID
   SyncLocker locker(_lock);
   androidClose();
   if(GLDisplay && MainContext.context)
   {
      EGLint format; eglGetConfigAttrib(GLDisplay, GLConfig, EGL_NATIVE_VISUAL_ID, &format);
      ANativeWindow_setBuffersGeometry(AndroidApp->window, 0, 0, format);
      MainContext.surface=eglCreateWindowSurface(GLDisplay, GLConfig, AndroidApp->window, null); if(!MainContext.surface)Exit("Can't create EGLSurface.");
      MainContext.lock();
   }else Exit("OpenGL Display and MainContext not available.");
#endif
}
Bool Display::create()
{
if(LogInit)LogN("Display.create");
           createDevice();
               getGamma();
                getCaps();
      Sh.createSamplers();
   DisplayState::create();
      setDeviceSettings();
              Sh.create();
             InitMatrix(); // !! call this after creating main shaders, because it creates the "ObjMatrix, ObjVel" shader buffers !!
  if(!Renderer.rtCreate())Exit("Can't create Render Targets."); // !! call this after creating shaders because it modifies shader values !!
             InitVtxInd();
        Renderer.create();
           colorPalette(ImagePtr().get("Img/color palette.img"));
        VR.createImages(); // !! call this before 'after', because VR gui image may affect aspect ratio of display !!
                  after(false);
                  begin();
             Gui.create();

   // set default settings
   {Byte v=texFilter   (); _tex_filter    ^=1               ; texFilter   (v);}
   {Bool v=texMipFilter(); _tex_mip_filter^=1               ; texMipFilter(v);}
   {Bool v=bloomMaximum(); _bloom_max      =false           ; bloomMaximum(v);} // resetting will load shaders
   {auto v=edgeSoften  (); _edge_soften    =EDGE_SOFTEN_NONE; edgeSoften  (v);} // resetting will load shaders
   {auto v=edgeDetect  (); _edge_detect    =EDGE_DETECT_NONE; edgeDetect  (v);} // resetting will load shaders
   {Flt  v=grassRange  (); _grass_range    =-1              ; grassRange  (v);}
   lod               (_lod_factor, _lod_factor_shadow, _lod_factor_mirror);
   shadowJitterSet   ();
   MotionScaleChanged();
   SetMatrix         ();

  _initialized=true;

   return true;
}
/******************************************************************************/
Bool Display::created()
{
#if DX9
   return D3D!=null;
#elif DX11
   return D3DC!=null;
#elif GL
   return MainContext.is();
#endif
}
/******************************************************************************/
void ThreadMayUseGPUData()
{
#if GL && HAS_THREADS
   Ptr context=GetCurrentContext();
   if(!context)
   {
      ContextLock.on();
      for(;;)
      {
         REPA(SecondaryContexts)if(!SecondaryContexts[i].locked)
         {
            SecondaryContexts[i].lock();
            goto context_locked;
         }
         if(!SecondaryContexts.elms())Exit("No secondary OpenGL contexts have been created");
         ContextLock.off(); ContextUnlocked.wait(); // wait until any other context is unlocked
         ContextLock.on ();
      }
   context_locked:
      ContextLock.off();
   }
#endif   
}
void ThreadFinishedUsingGPUData()
{
#if GL && HAS_THREADS
   if(Ptr context=GetCurrentContext())
   {
      ContextLock.on();
      REPA(SecondaryContexts)if(SecondaryContexts[i].context==context)
      {
         SecondaryContexts[i].unlock();
         goto context_unlocked;
      }
   context_unlocked:
      ContextLock.off();
      ContextUnlocked++; // notify of unlocking
   }
#endif   
}
/******************************************************************************/
static Int DisplaySamples(Int samples)
{
   Clamp(samples, 1, 16);
   if(Renderer.anyDeferred() && D.deferredMSUnavailable())samples=1;
#if DX9
   if(D3DBase)
   {
      D3DFORMAT disp_format=D3DFMT_A8R8G8B8; if(!OK(D3DBase->CheckDeviceType(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, disp_format, D3DPP.Windowed)))disp_format=D3DFMT_X8R8G8B8;
      for(; samples>1; samples--)
      {
         DWORD col_levels=0, ds_levels=0;
         if(samples<=D3DMULTISAMPLE_16_SAMPLES) // there's no higher sample level on DX9
         {
            DWORD levels=0;
            if(OK(D3DBase->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, disp_format , D3DPP.Windowed, D3DMULTISAMPLE_TYPE(samples), &levels)))MAX(col_levels, levels);
            if(OK(D3DBase->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_D24S8, D3DPP.Windowed, D3DMULTISAMPLE_TYPE(samples), &levels)))MAX( ds_levels, levels);
            if(OK(D3DBase->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_D24X8, D3DPP.Windowed, D3DMULTISAMPLE_TYPE(samples), &levels)))MAX( ds_levels, levels);
         }
         if(col_levels && ds_levels)break; // if there are some quality levels, then accept this multi-sampling
      }
   }
#elif DX11
   if(samples>1)samples=4; // only 4 samples are supported in DX10+ implementation
#else
   samples=1; // other implementations don't support multi-sampling
#endif
   return samples;
}
Bool Display::findMode()
{
   SyncLocker locker(_lock);

#if WINDOWS_NEW // on WindowsNew we can't change mode here, we need to set according to what we've got, instead 'RequestDisplayMode' can be called
  _res =WindowSize(true);
  _full=App.Fullscreen();
#elif IOS
   // '_res' will be set in 'mapMain'
#elif MOBILE || WEB
   // Renderer._main is already available
  _res=Renderer._main.size();
#else
   RectI full, work; VecI2 max_normal_win_client_size, maximized_win_client_size;
    curMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);
#if FORCE_MAIN_DISPLAY
   if(resW()>=D.screenW() && resH()>=D.screenH())_full=true;
#else
   if(resW()>=full.w() && resH()>=full.h())_full=true; // force fullscreen only if both dimensions are equal-bigger because on Windows it's perfectly fine to have a window as wide as the whole desktop
#endif
   if(D.full())
   {
      Int nearest=-1; Int desired_area=res().mul(), area_error;
      REPA(_modes)
      {
         C VecI2 &mode=_modes[i];
         if(mode==res()){nearest=i; break;} // exact match
         Int ae=Abs(mode.mul()-desired_area);
         if(nearest<0 || ae<area_error){nearest=i; area_error=ae;}
      }
      if(nearest<0)return false;
     _res=_modes[nearest];
   }else
   {
      if(resW()>=Min(maximized_win_client_size.x, max_normal_win_client_size.x+1)
      && resH()>=Min(maximized_win_client_size.y, max_normal_win_client_size.y+1))_res=maximized_win_client_size;/*else
      {  don't limit for 2 reasons: 1) having multiple monitors we can make the window cover multiple monitors 2) on Windows initially we can drag the window only until 'max_normal_win_client_size', however after that we can drag again to make it bigger, and this time it will succeed, which means that making windows bigger than 'max_normal_win_client_size' is actually possible, unless it's a bug in the OS
         MIN(_res.x, max_normal_win_client_size.x);
         MIN(_res.y, max_normal_win_client_size.y);
      }*/
   }
#endif

#if DX9
   Zero(D3DPP);
   D3DPP.Windowed                  =(!exclusive() || !T.full()); // !! set this first !!
   D3DPP.BackBufferCount           =(sync() ? 2 : 1);
   D3DPP.BackBufferWidth           =resW();
   D3DPP.BackBufferHeight          =resH();
   D3DPP.BackBufferFormat          =D3DFMT_A8R8G8B8; if(D3DBase && !OK(D3DBase->CheckDeviceType(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DPP.BackBufferFormat, D3DPP.Windowed)))D3DPP.BackBufferFormat=D3DFMT_X8R8G8B8;
   D3DPP.EnableAutoDepthStencil    =false;
   D3DPP.hDeviceWindow             =App.Hwnd();
   D3DPP.SwapEffect                =D3DSWAPEFFECT_DISCARD;
   D3DPP.MultiSampleQuality        =0;
   D3DPP.MultiSampleType           =D3DMULTISAMPLE_NONE;
   D3DPP.PresentationInterval      =( sync()                         ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
   D3DPP.FullScreen_RefreshRateInHz=((_freq_want && !D3DPP.Windowed) ? _freq_want              : D3DPRESENT_RATE_DEFAULT      ); // set custom frequency only if desired and in true full-screen
#elif DX11
   Zero(SwapChainDesc);
   Bool sync=ActualSync();
   #if WINDOWS_OLD
      SwapChainDesc.OutputWindow      =App.Hwnd();
      SwapChainDesc.Windowed          =(!exclusive() || !T.full());
      SwapChainDesc.BufferCount       =(sync ? 3 : 2); // if we're rendering to VR display, then it has its own swap chain, and it handles the most intense rendering, so we don't need to have more buffers here, so keep it low to reduce memory usage
      SwapChainDesc.BufferDesc.Width  =resW();
      SwapChainDesc.BufferDesc.Height =resH();
      SwapChainDesc.BufferDesc.Format =(GDI_COMPATIBLE ? DXGI_FORMAT_B8G8R8A8_UNORM : highMonitorPrecision() ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM);
      SwapChainDesc.SampleDesc.Count  =1;
      SwapChainDesc.SampleDesc.Quality=0;
      SwapChainDesc.BufferUsage       =DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_SHADER_INPUT|DXGI_USAGE_BACK_BUFFER;
      SwapChainDesc.Flags             =DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH|(GDI_COMPATIBLE ? DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE : 0);
      if(_freq_want && !SwapChainDesc.Windowed) // set custom frequency only if desired and in true full-screen
      {
         SwapChainDesc.BufferDesc.RefreshRate.Numerator  =_freq_want;
         SwapChainDesc.BufferDesc.RefreshRate.Denominator=1;
      }

      SwapChainDesc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
   #if !GDI_COMPATIBLE
      #if 0
      /* disable this as last time this was tested, it provided slower results, also there was a few second slow down when changing sync during app runtime:
                                          windowed sync=false | windowed sync=true | fullscreen sync=true
         DXGI_SWAP_EFFECT_DISCARD             850 fps       |       42 fps       |       16.9 fps
         DXGI_SWAP_EFFECT_FLIP_DISCARD        655 fps       |       34 fps       |       16.4 fps       */
      {
         VecI4 ver=OSVerNumber();
         if(        ver.x>=10                 )SwapChainDesc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD   ;else // DXGI_SWAP_EFFECT_FLIP_DISCARD    is available on Windows 10
         if(Compare(ver, VecI4(6, 2, 0, 0))>=0)SwapChainDesc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;     // DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL is available on Windows 8 - https://msdn.microsoft.com/en-us/library/windows/desktop/bb173077(v=vs.85).aspx
      }
      #endif
   #endif
      if(AllowTearing && SwapChainDesc.SwapEffect==DXGI_SWAP_EFFECT_FLIP_DISCARD)SwapChainDesc.Flags|=DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      PresentFlags=((!sync && (SwapChainDesc.Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) && SwapChainDesc.Windowed) ? DXGI_PRESENT_ALLOW_TEARING : 0); // according to docs, we can use DXGI_PRESENT_ALLOW_TEARING only with DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING and in windowed mode - https://msdn.microsoft.com/en-us/library/windows/desktop/bb509554(v=vs.85).aspx

      SwapChainDesc.BufferDesc.Scaling=DXGI_MODE_SCALING_UNSPECIFIED; // can't always use 'DXGI_MODE_SCALING_STRETCHED' because it will fail to set highest supported resolution by the monitor, because that resolution never supports streching, so we set DXGI_MODE_SCALING_UNSPECIFIED which means it will be set according to the driver settings
      if(1 // find the monitor/output that we're going to use, and iterate all of its modes to check if that mode supports stretched mode
      && !SwapChainDesc.Windowed) // needed for exclusive fullscreen only and only when the driver has scaling set to DXGI_MODE_SCALING_CENTERED
      {
         IDXGIOutput *output;
         Bool         ok=false;
         if(SwapChain) // if we already have a swap chain, then reuse its output
         {
            output=null; SwapChain->GetContainingOutput(&output); if(output){ok=true; goto has_output;} // set 'ok' so break will be called
         }
         if(Adapter)
            if(HMONITOR monitor=MonitorFromWindow(App.Hwnd(), MONITOR_DEFAULTTONEAREST)) // get nearest monitor
               for(Int i=0; ; i++) // iterate all outputs
         {
            output=null; Adapter->EnumOutputs(i, &output); if(output)
            {
               DXGI_OUTPUT_DESC desc; ok=(OK(output->GetDesc(&desc)) && desc.Monitor==monitor); // if found the monitor that we're going to use
               if(ok)
               {
               has_output:
                  DXGI_FORMAT                                          mode=DXGI_FORMAT_R8G8B8A8_UNORM; // always use this mode in case system doesn't support 10-bit color
                  UInt                                           descs_elms=0; output->GetDisplayModeList(mode, 0, &descs_elms, null); // get number of mode descs
                  MemtN<DXGI_MODE_DESC, 128> descs; descs.setNum(descs_elms ); output->GetDisplayModeList(mode, 0, &descs_elms, descs.data()); // get mode descs
                  FREPA(descs)
                  {
                     C DXGI_MODE_DESC &mode=descs[i];
                     if(mode.Width==resW() && mode.Height==resH() && mode.Scaling!=DXGI_MODE_SCALING_UNSPECIFIED) // can't just check for ==DXGI_MODE_SCALING_STRETCHED because it's never listed, however DXGI_MODE_SCALING_CENTERED will be listed for modes that support stretching, so we use !=DXGI_MODE_SCALING_UNSPECIFIED to support both DXGI_MODE_SCALING_STRETCHED and DXGI_MODE_SCALING_CENTERED
                     {
                        SwapChainDesc.BufferDesc.Scaling=DXGI_MODE_SCALING_STRETCHED;
                        break;
                     }
                  }
               }
               output->Release();
               if(ok)break;
            }else break;
         }
      }
   #else // WINDOWS_NEW
      SwapChainDesc.Width =resW();
      SwapChainDesc.Height=resH();
      SwapChainDesc.Format=(GDI_COMPATIBLE ? DXGI_FORMAT_B8G8R8A8_UNORM : highMonitorPrecision() ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM);
      SwapChainDesc.Stereo=false;
      SwapChainDesc.SampleDesc.Count  =1;
      SwapChainDesc.SampleDesc.Quality=0;
      SwapChainDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_SHADER_INPUT|DXGI_USAGE_BACK_BUFFER;
      SwapChainDesc.BufferCount=(sync ? 3 : 2); // if we're rendering to VR display, then it has its own swap chain, and it handles the most intense rendering, so we don't need to have more buffers here, so keep it low to reduce memory usage
      SwapChainDesc.Scaling    =DXGI_SCALING_STRETCH;
      SwapChainDesc.SwapEffect =DXGI_SWAP_EFFECT_FLIP_DISCARD;
      SwapChainDesc.AlphaMode  =DXGI_ALPHA_MODE_IGNORE;
      SwapChainDesc.Flags      =0;
      if(AllowTearing && SwapChainDesc.SwapEffect==DXGI_SWAP_EFFECT_FLIP_DISCARD)SwapChainDesc.Flags|=DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      PresentFlags=((!sync && (SwapChainDesc.Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)/* && SwapChainDesc.Windowed*/) ? DXGI_PRESENT_ALLOW_TEARING : 0); // according to docs, we can use DXGI_PRESENT_ALLOW_TEARING only with DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING and in windowed mode, in UWP there's no windowed check because "UWP apps that enter fullscreen mode by calling Windows::UI::ViewManagement::ApplicationView::TryEnterFullscreen() are fullscreen borderless windows and may use the flag" - https://msdn.microsoft.com/en-us/library/windows/desktop/bb509554(v=vs.85).aspx
   #endif
#endif
   densityUpdate();
   return true;
}
/******************************************************************************/
CChar8* Display::AsText(RESET_RESULT result)
{
   switch(result)
   {
      case RESET_OK                  : return "RESET_OK";
      case RESET_DEVICE_NOT_CREATED  : return "RESET_DEVICE_NOT_CREATED";
      case RESET_CUSTOM_LOST_FAILED  : return "RESET_CUSTOM_LOST_FAILED";
      case RESET_CUSTOM_RESET_FAILED : return "RESET_CUSTOM_RESET_FAILED";
      case RESET_DEVICE_RESET_FAILED : return "RESET_DEVICE_RESET_FAILED";
      case RESET_RENDER_TARGET_FAILED: return "RESET_RENDER_TARGET_FAILED";
      default                        : return "RESET_UNKNOWN";
   }
}
void Display::ResetFailed(RESET_RESULT New, RESET_RESULT old)
{
   Exit(
     ((New==old) ? S+"Can't set display mode: "+AsText(New)
                 : S+"Can't set new display mode: "+AsText(New)
                  +"\nCan't set old display mode: "+AsText(old))
   #if DX9
      +((New==RESET_DEVICE_RESET_FAILED || old==RESET_DEVICE_RESET_FAILED) ? "\nDid you forget to delete IMAGE_RT Image in D.lost on DX9?" : "")
   #endif
       );
}

#if DX11
static Bool ResizeTarget()
{
#if WINDOWS_OLD
again:
   if(OK(SwapChain->ResizeTarget(&SwapChainDesc.BufferDesc)))return true;
   if(SwapChainDesc.BufferDesc.Format==DXGI_FORMAT_R10G10B10A2_UNORM){SwapChainDesc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; goto again;} // if failed with 10-bit then try again with 8-bit
#endif
   return false;
}
static Bool ResizeBuffers()
{
again:
#if WINDOWS_OLD
   if(OK(SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, SwapChainDesc.BufferDesc.Width, SwapChainDesc.BufferDesc.Height, SwapChainDesc.BufferDesc.Format, SwapChainDesc.Flags)))return true;
   if(SwapChainDesc.BufferDesc.Format==DXGI_FORMAT_R10G10B10A2_UNORM){SwapChainDesc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; goto again;} // if failed with 10-bit then try again with 8-bit
#else
   if(OK(SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, SwapChainDesc.           Width, SwapChainDesc.           Height, SwapChainDesc.           Format, SwapChainDesc.Flags)))return true;
   if(SwapChainDesc.Format==DXGI_FORMAT_R10G10B10A2_UNORM){SwapChainDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; goto again;} // if failed with 10-bit then try again with 8-bit
#endif
   return false;
}
#endif

Display::RESET_RESULT Display::ResetTry()
{
   SyncLocker locker(_lock);
  _resetting=true;
   RESET_RESULT result;

   if(!created())result=RESET_DEVICE_NOT_CREATED;else
   {
      Bool begin_end=_began;
      if(  begin_end)end();
      Renderer.rtDel();
   #if DX9
      VI.lost(); // dynamic buffers
      VideoTexturesLost(); // video textures (render targets)
      RELEASE(Query);
   #endif

      if(lost && !lost())result=RESET_CUSTOM_LOST_FAILED;else
      {
         Bool ok=true;
      #if WINDOWS
         #if (DX9 || DX11) && !WINDOWS_NEW // on WindowsNew we can't change mode here, we need to set according to what we've got, instead 'RequestDisplayMode' can be called
            if(!exclusive())ok=SetDisplayMode();
         #endif

         #if DX9
            if(ok)
            {
            reset:;
               ok=OK(D3D->Reset(&D3DPP));
               if(!ok && WindowActive()!=App.hwnd()) // if reset failed and we're not focused then wait a little and try again (this is important when using 'BackgroundLoader' and pressing Ctrl+Alt+Del, device is lost, fails to reset and secondary thread fails to load data)
               {
                  Time.wait(100);
                  goto reset;
               }
               setDeviceSettings();
            }
         #elif DX11
            #if WINDOWS_OLD
               if(ok&=OK(SwapChain->SetFullscreenState(!SwapChainDesc.Windowed, SwapChainDesc.Windowed ? null : Output)))
               if(ok&=ResizeTarget())
            #elif 0 // both 'SetFullscreenState' and 'ResizeTarget' fail on WINDOWS_NEW, instead, 'TryEnterFullScreenMode', 'ExitFullScreenMode', 'TryResizeView' are used
               DXGI_MODE_DESC mode; Zero(mode);
               mode.Format =SwapChainDesc.Format;
               mode.Width  =SwapChainDesc.Width;
               mode.Height =SwapChainDesc.Height;
               mode.Scaling=DXGI_MODE_SCALING_UNSPECIFIED;
               if(_freq_want && full()) // set custom frequency only if desired and in full-screen
               {
                  mode.RefreshRate.Numerator  =_freq_want;
                  mode.RefreshRate.Denominator=1;
               }
               if(ok&=OK(SwapChain->SetFullscreenState(full(), null)))
               if(ok&=OK(SwapChain->ResizeTarget(&mode)))
            #endif
            {
               // 'ResizeTarget' may select a different resolution than requested for fullscreen mode, so check what we've got
            #if WINDOWS_OLD
               if(!SwapChainDesc.Windowed)
            #else
               if(0) // if(full()) this shouldn't be performed for WINDOWS_NEW because for this we're not setting a custom display mode, but instead we're setting to what we've got
            #endif
               {
                  IDXGIOutput *output=null; SwapChain->GetContainingOutput(&output); if(output)
                  {
                     DXGI_OUTPUT_DESC desc; if(OK(output->GetDesc(&desc)))
                     {
                       _res.set(desc.DesktopCoordinates.right-desc.DesktopCoordinates.left, desc.DesktopCoordinates.bottom-desc.DesktopCoordinates.top);
                     #if WINDOWS_OLD
                        SwapChainDesc.BufferDesc.Width =resW();
                        SwapChainDesc.BufferDesc.Height=resH();
                     #else
                        // for WINDOWS_NEW this should adjust '_res' based on relative rotation
                        SwapChainDesc.Width =resW();
                        SwapChainDesc.Height=resH();
                     #endif
                        densityUpdate();
                     }
                     output->Release();
                  }
               }
               ok&=ResizeBuffers();
            }
         #elif GL
            if(ok)ok=SetDisplayMode();
         #endif
      #elif MAC || LINUX
         ok=SetDisplayMode();
      #else
         ok=true;
      #endif

         getCaps();
         if(!ok                 )result=RESET_DEVICE_RESET_FAILED ;else
         if(!Renderer.rtCreate())result=RESET_RENDER_TARGET_FAILED;else
         {
         #if DX9
            D3D->CreateQuery(D3DQUERYTYPE_EVENT, &Query);
            VI.reset(); // dynamic buffers
         #endif

            adjustWindow(); // !! call before 'after' so current monitor can be detected properly based on window position which affects the aspect ratio in 'after' !!
            after(true);
            begin(); // we need begin if 'begin_end' was enabled and also always for the following code
            {
               resetEyeAdaptation(); // make sure we're inside begin because this potentially may use drawing
            #if DX9
               VideoTexturesReset(); // video textures (render targets), make sure we're inside begin
            #endif
            }
            if(!begin_end)end(); // leave in the same state as before the reset

            Time.skipUpdate(2); // when resetting display skip 2 frames, because the slow down can occur for this long
            if(reset && !reset())result=RESET_CUSTOM_RESET_FAILED;
            else                 result=RESET_OK;
         }
      }
   }

  _resetting=false;
   return result;
}
void Display::Reset()
{
   RESET_RESULT result=ResetTry(); if(result!=RESET_OK)ResetFailed(result, result);
}
/******************************************************************************/
void Display::getGamma()
{
   Bool ok=false;
#if WINDOWS_OLD
   if(HDC hdc=GetDC(null)){ok=(GetDeviceGammaRamp(hdc, _gamma_array)!=0); ReleaseDC(null, hdc);}
#elif MAC
   Int capacity =CGDisplayGammaTableCapacity(kCGDirectMainDisplay);
   if( capacity>=1)
   {
      Memc<CGGammaValue> r, g, b; r.setNum(capacity); g.setNum(capacity); b.setNum(capacity);
      UInt samples=0; CGGetDisplayTransferByTable(kCGDirectMainDisplay, capacity, r.data(), g.data(), b.data(), &samples);
      if(  samples>1 && samples<=capacity)
      {
         ok=true;
         REP(256)
         {
            Int src=i*(samples-1)/255;
           _gamma_array[0][i]=RoundU(Sat(r[src])*0xFFFF);
           _gamma_array[1][i]=RoundU(Sat(g[src])*0xFFFF);
           _gamma_array[2][i]=RoundU(Sat(b[src])*0xFFFF);
         }
      }
   }
#elif LINUX
   if(XDisplay)
   {
      Int size=0; if(XF86VidModeGetGammaRampSize(XDisplay, DefaultScreen(XDisplay), &size))
      {
         if(size==256)
         {
            ok=(XF86VidModeGetGammaRamp(XDisplay, DefaultScreen(XDisplay), 256, _gamma_array[0], _gamma_array[1], _gamma_array[2])!=0);
         }else
         if(size>0)
         {
            Memc<UShort> r, g, b; r.setNum(size); g.setNum(size); b.setNum(size);
            if(XF86VidModeGetGammaRamp(XDisplay, DefaultScreen(XDisplay), size, r.data(), g.data(), b.data()))
            {
               ok=true;
               REP(256)
               {
                  Int src=i*(size-1)/255;
                 _gamma_array[0][i]=r[src];
                 _gamma_array[1][i]=g[src];
                 _gamma_array[2][i]=b[src];
               }
            }
         }
      }
   }
#endif
   if(!ok)REP(256)_gamma_array[0][i]=_gamma_array[1][i]=_gamma_array[2][i]=(i*0xFFFF+128)/255;
}
void Display::getCaps()
{
   if(LogInit)LogN("Display.getCaps");
#if DX9
   D3DDISPLAYMODE DM  ; D3D->GetDisplayMode(0, &DM);
   D3DCAPS9       caps; D3D->GetDeviceCaps ( &caps);
  _freq_got          =DM.RefreshRate;
  _max_rt            =Mid(caps.NumSimultaneousRTs, 1, 255);
  _max_tex_filter    =Mid(caps.MaxAnisotropy     , 1, 255);
  _max_tex_size      =Min(Int(caps.MaxTextureWidth), Int(caps.MaxTextureHeight));
  _tex_pow2          =(FlagTest(caps.TextureCaps, D3DPTEXTURECAPS_NONPOW2CONDITIONAL) ? 1 : FlagTest(caps.TextureCaps, D3DPTEXTURECAPS_POW2) ? 2 : 0); // 0=non-pow2 supported, 1=non-pow2 conditional, 2=non-pow2 not supported (pow2 required)
  _tex_pow2_3d       = FlagTest(caps.TextureCaps, D3DPTEXTURECAPS_VOLUMEMAP_POW2);
  _tex_pow2_cube     = FlagTest(caps.TextureCaps, D3DPTEXTURECAPS_CUBEMAP_POW2  );
  _shader_tex_lod    =(D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion)>=3);
  _mrt_const_bit_size=((caps.PrimitiveMiscCaps&D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS   )==0);
  _mrt_post_process  =((caps.PrimitiveMiscCaps&D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING)!=0
                      && validUsage(D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, IMAGE_B8G8R8A8)
                      && validUsage(D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, IMAGE_F32     )
                      && validUsage(D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, IMAGE_F16_4   ));
#elif DX11
   // values taken from - https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
   DXGI_SWAP_CHAIN_DESC desc;
   SwapChain->GetDesc(&desc); _freq_got=(desc.BufferDesc.RefreshRate.Denominator ? RoundPos(Flt(desc.BufferDesc.RefreshRate.Numerator)/desc.BufferDesc.RefreshRate.Denominator) : 0);
  _max_rt            =((FeatureLevel>=D3D_FEATURE_LEVEL_10_0) ? 8 : (FeatureLevel>=D3D_FEATURE_LEVEL_9_3) ? 4 : 1);
  _max_tex_filter    =((FeatureLevel>=D3D_FEATURE_LEVEL_9_2 ) ? 16 : 2);
  _max_tex_size      =((FeatureLevel>=D3D_FEATURE_LEVEL_11_0) ? 16384 : (FeatureLevel>=D3D_FEATURE_LEVEL_10_0) ? 8192 : (FeatureLevel>=D3D_FEATURE_LEVEL_9_3) ? 4096 : 2048);
  _tex_pow2          = // 0=non-pow2 supported, 1=non-pow2 conditional, 2=non-pow2 not supported (pow2 required)
  _tex_pow2_3d       =
  _tex_pow2_cube     =(FeatureLevel<=D3D_FEATURE_LEVEL_9_3);
  _shader_tex_lod    =(FeatureLevel>=D3D_FEATURE_LEVEL_10_0);
  _mrt_const_bit_size=false;
  _mrt_post_process  =true ;
#elif GL
   CChar8 *ext=(CChar8*)glGetString(GL_EXTENSIONS);
      _max_tex_size    =2048; glGetIntegerv(GL_MAX_TEXTURE_SIZE          , &_max_tex_size    );
   int aniso           =  16; glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, & aniso           ); _max_tex_filter =Mid(aniso         , 1, 255);
   int max_vtx_attrib  =   0; glGetIntegerv(GL_MAX_VERTEX_ATTRIBS        , & max_vtx_attrib  ); _max_vtx_attribs=Mid(max_vtx_attrib, 0, 255);
   if(shaderModelGLES2())_max_rt=1;else // we don't support MRT on GLES2 (there's no 'glDrawBuffers' function)
   {
      int max_draw_buffers=1; glGetIntegerv(GL_MAX_DRAW_BUFFERS     , &max_draw_buffers);
      int max_col_attach  =1; glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_col_attach  ); _max_rt=Mid(Min(max_draw_buffers, max_col_attach), 1, 255);
   }

   Bool npot=(notShaderModelGLES2() || Contains(ext, "GL_OES_texture_npot", false, true));
  _tex_pow2     =((npot || Contains(ext, "GL_NV_texture_npot_2D_mipmap", false, true)) ? 0 : 1); // 0=non-pow2 supported, 1=non-pow2 conditional, 2=non-pow2 not supported (pow2 required)
  _tex_pow2_3d  = !npot;
  _tex_pow2_cube= !npot;

  _shader_tex_lod    =(notShaderModelGLES2() || Contains(ext, "GL_ARB_shader_texture_lod", false, true) || Contains(ext, "GL_EXT_shader_texture_lod", false, true));
  _mrt_const_bit_size=false;
  _mrt_post_process  =true ;
   #if VARIABLE_MAX_MATRIX
      int max_vs_vectors=0, max_ps_vectors=0;
      glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS  , &max_vs_vectors);
      glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &max_ps_vectors);
      MeshBoneSplit=(Min(max_vs_vectors, max_ps_vectors)<768+256+256); // 768 for ObjMatrix, 256 for ObjVel, 256 extra
   #endif
#endif

#if IOS
  _freq_got=[UIScreen mainScreen].maximumFramesPerSecond;
#elif LINUX
   if(XDisplay)
   {
      int dotclock; XF86VidModeModeLine mode;
      if(XF86VidModeGetModeLine(XDisplay, DefaultScreen(XDisplay), &dotclock, &mode))
      {
         int total=mode.htotal*mode.vtotal;
        _freq_got=(total ? dotclock*1000/total : 0);
      }
   }
#endif
  
   if(!Physics.precision())Physics.precision(0); // adjust physics precision when possibility of screen refresh rate change
   densityUpdate(); // max texture size affects max allowed density
   if(_mrt_const_bit_size)_hp_col_rt=_hp_nrm_rt=false; // no need to disable '_hp_lum_rt' and '_lit_col_rt_prec' because those RT's are not used as MRT
  _samples=DisplaySamples(_samples);

   if(Renderer.anyDeferred() && deferredUnavailable())
   {
      if(Renderer.type              ()==RT_DEFERRED)Renderer.type              (RT_FORWARD);
      if(Water   .reflectionRenderer()==RT_DEFERRED)Water   .reflectionRenderer(RT_FORWARD);
   }
}
/******************************************************************************/
void Display::after(Bool resize_callback)
{
   if(LogInit)LogN("Display.after");
   if(!full() // if we're setting window
#if !WEB // for WEB set size even if we're maximized, because for WEB 'App.maximized' means the browser window and not the canvas size
   && !App.maximized() // which is not maximized
#endif
   )App._window_size=res();
   if(_gamma)gammaSet(); // force reset gamma
   aspectRatioEx(true, !resize_callback);
}
/******************************************************************************/
void Display::begin()
{
#if DX9
   if(D3D)
   {
      D3D->BeginScene();
      D._began=true;
   }
#endif
}
void Display::end()
{
#if DX9
   if(D3D)
   {
      D3D->EndScene();
      D._began=false;
   }
#endif
}
Bool Display::flip()
{
   if(created())
   {
   #if DX9
      end();
      Bool ok=OK(D3D->Present(null, null, null, null));
      begin();
      return ok;
   #elif DX11
      Bool sync=ActualSync(); if(!OK(SwapChain->Present(sync, sync ? 0 : PresentFlags)))return false; // we can use 'DXGI_PRESENT_ALLOW_TEARING' only when "sync==false", do this extra check here, because 'ActualSync' depends on VR which may disconnect after 'SwapChain' was created and 'PresentFlags' already set
      if(SwapChainDesc.SwapEffect!=DXGI_SWAP_EFFECT_DISCARD) // when using swap chain flip mode, 'Present' clears the backbuffer from 'OMSetRenderTargets', so reset it
         D3DC->OMSetRenderTargets(Elms(Renderer._cur_id), Renderer._cur_id, Renderer._cur_ds_id);
   #elif GL
      #if WINDOWS
         SwapBuffers(hDC);
      #elif MAC
         CGLFlushDrawable(MainContext.context); // same as "[[OpenGLView openGLContext] flushBuffer];"
      #elif LINUX
         glXSwapBuffers(XDisplay, App.Hwnd());
      #elif ANDROID
         eglSwapBuffers(GLDisplay, MainContext.surface);
      #elif IOS
         glBindRenderbuffer(GL_RENDERBUFFER, Renderer._main._rb);
         [MainContext.context presentRenderbuffer:GL_RENDERBUFFER];
      #elif WEB
         // this is done automatically on Web
      #endif
   #endif
   }
   return true;
}
void Display::flush()
{
   if(created())
   {
   #if DX9
      if(Query)
      {
         Query->Issue(D3DISSUE_END);
         while(S_FALSE==Query->GetData(null, 0, D3DGETDATA_FLUSH));
      }
   #elif DX11
      D3DC->Flush();
   #elif GL
      glFlush();
   #endif
   }
}
void Display::finish()
{
   if(created())
   {
   #if DX9
      if(Query)
      {
         Query->Issue(D3DISSUE_END);
         while(S_FALSE==Query->GetData(null, 0, D3DGETDATA_FLUSH));
      }
   #elif DX11
      if(Query)
      {
         D3DC->End(Query);
         BOOL done=FALSE; while(OK(D3DC->GetData(Query, &done, SIZE(done), 0)) && !done);
      }
   #elif GL
      glFinish();
   #endif
   }
}
/******************************************************************************/
// SETTINGS
/******************************************************************************/
void Display::adjustWindow()
{
   RectI full, work; VecI2 max_normal_win_client_size, maximized_win_client_size;

#if FORCE_MAIN_DISPLAY
   if(D.full())mainMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);else
#endif
                curMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);

#if DEBUG && 0
   LogN(S+"full:"+full.asText()+", work:"+work.asText()+", App._window_pos:"+App._window_pos+", D.res:"+D.res());
#endif

#if WINDOWS_OLD
   if(D.full()) // fullscreen
   {
      SetWindowLongPtr(App.Hwnd(), GWL_STYLE   , App._style_full);
      SetWindowPos    (App.Hwnd(), HWND_TOPMOST, full.min.x, full.min.y, resW(), resH(), 0);
   }else
   if(resW()>=maximized_win_client_size.x && resH()>=maximized_win_client_size.y) // maximized
   {
      SetWindowLongPtr(App.Hwnd(), GWL_STYLE, App._style_window_maximized);
   #if 0 // this doesn't work as expected
      SetWindowPos    (App.Hwnd(), HWND_TOP , work.min.x+App._bound_maximized.min.x, work.min.y-App._bound_maximized.max.y, resW()+App._bound_maximized.w(), resH()+App._bound_maximized.h(), SWP_NOACTIVATE); 
   #else
      SetWindowPos    (App.Hwnd(), HWND_TOP , work.min.x+App._bound_maximized.min.x, work.min.y-App._bound_maximized.max.y, resW()+App._bound_maximized.max.x, resH()-App._bound_maximized.min.y, SWP_NOACTIVATE);
   #endif
   }else // normal window
   {
      Int w=resW()+App._bound.w(),
          h=resH()+App._bound.h();

      if(App._window_pos.x==INT_MAX){if(App.x<=-1)App._window_pos.x=work.min.x;else if(!App.x)App._window_pos.x=work.centerXI()-w/2;else App._window_pos.x=work.max.x-w;}
      if(App._window_pos.y==INT_MAX){if(App.y>= 1)App._window_pos.y=work.min.y;else if(!App.y)App._window_pos.y=work.centerYI()-h/2;else App._window_pos.y=work.max.y-h;}

      // make sure the window is not completely outside of working area
      const Int b=32; Int r=b;
      if(!(App.flag&APP_NO_TITLE_BAR)) // has bar
      {
         Int size=GetSystemMetrics(SM_CXSIZE); // TODO: this should be OK because we're DPI-Aware, however it doesn't work OK
       /*if(HDC hdc=GetDC(App.Hwnd()))
         {
            size=DivCeil(size*GetDeviceCaps(hdc, LOGPIXELSX), 96);
            ReleaseDC(App.Hwnd(), hdc);
         }*/
         if(!(App.flag& APP_NO_CLOSE                   ))r+=size  ; // has close                button
         if(  App.flag&(APP_MINIMIZABLE|APP_MAXIMIZABLE))r+=size*2; // has minimize or maximize button (if any is enabled, then both will appear)
      }

      if(App._window_pos.x+b>work.max.x)App._window_pos.x=Max(work.min.x, work.max.x-b);else{Int p=App._window_pos.x+w; if(p-r<work.min.x)App._window_pos.x=Min(work.min.x+r, work.max.x)-w;}
      if(App._window_pos.y+b>work.max.y)App._window_pos.y=Max(work.min.y, work.max.y-b);else{Int p=App._window_pos.y+h; if(p-b<work.min.y)App._window_pos.y=Min(work.min.y+b, work.max.y)-h;}

      SetWindowLongPtr(App.Hwnd(), GWL_STYLE     , App._style_window);
      SetWindowPos    (App.Hwnd(), HWND_NOTOPMOST, App._window_pos.x, App._window_pos.y, w, h, SWP_NOACTIVATE);
   }
#elif MAC
   if(!D.full()) // on Mac don't adjust the window size/pos when in fullscreen because it's not needed, additionally it will cancel out original window position when later restoring
   {
      WindowSize(resW(), resH(), true);
      RectI r=WindowRect(false); WindowPos(r.min.x, r.min.y); // reset position because if Application was created in fullscreen mode, and then we've toggled to windowed mode, then because system MenuBar was hidden for fullscreen, App's window position could've gone underneath the menu bar, for the windowed mode the bar is unhidden, however just applying new window size doesn't reposition it so it doesn't collide with the MenuBar, that's why we need to trigger repositioning it manually, avoid calling "WindowMove(0, 0)" because that internally does nothing if the delta is zero
   }
#elif LINUX
   // set window fullscreen state
   if(App.hwnd())
   {
      // setting fullscreen mode will fail if window is not resizable, so force it to be just for this operation
      Bool set_resizable=(D.full() && !(App.flag&APP_RESIZABLE));
      if(  set_resizable)App.setWindowFlags(true);
    
      #define _NET_WM_STATE_REMOVE 0
      #define _NET_WM_STATE_ADD    1
      #define _NET_WM_STATE_TOGGLE 2
      Atom FIND_ATOM(_NET_WM_STATE), FIND_ATOM(_NET_WM_STATE_FULLSCREEN);
      XEvent e; Zero(e);
      e.xclient.type        =ClientMessage;
      e.xclient.window      =App.Hwnd();
      e.xclient.message_type=_NET_WM_STATE;
      e.xclient.format      =32;
      e.xclient.data.l[0]   =(D.full() ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE);
      e.xclient.data.l[1]   =_NET_WM_STATE_FULLSCREEN;
      e.xclient.data.l[2]   =0;
      e.xclient.data.l[3]   =1;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, &e);
      XSync(XDisplay, false);

      if(set_resizable)App.setWindowFlags();
   }

   // set window size
   if(!D.full())WindowSize(resW(), resH(), true);
#endif
}
Display::RESET_RESULT Display::modeTry(Int w, Int h, Int full)
{
         if(w   <=0)w= T.resW();
         if(h   <=0)h= T.resH();
   Bool f=((full< 0) ? T.full() : (full!=0));
   if(w==resW() && h==resH() && f==T.full())return RESET_OK;

   if(created())
   {
      SyncLocker locker(_lock);

      Int  cur_x   =T.resW(); T._res.x=w;
      Int  cur_y   =T.resH(); T._res.y=h;
      Bool cur_full=T.full(); T._full =f;

   #if WEB
      Renderer._main.forceInfo(w, h, 1, Renderer._main.type(), Renderer._main.mode()); // '_main_ds' will be set in 'rtCreate'
   #endif
      if(!findMode())return RESET_DEVICE_NOT_CREATED;
      if(cur_x==T.resW() && cur_y==T.resH() && cur_full==T.full())return RESET_OK; // new mode matches the current one, need to check again since 'findMode' may have adjusted the T.resW T.resH T.full values
      RESET_RESULT result=ResetTry();         if(result!=RESET_OK)return result  ; // reset the device

      Ms.clipUpdate();
   }else
   {
      T._res.x=w;
      T._res.y=h;
      T._full =f;
      densityUpdate();
   }
   return RESET_OK;
}
void Display::modeSet(Int w, Int h, Int full)
{
   RESET_RESULT result=modeTry(w, h, full);
   if(result!=RESET_OK)ResetFailed(result, result);
}
Display& Display::mode(Int w, Int h, Int full)
{
#if WINDOWS_NEW // on WindowsNew we can only request a change on the window
   if(created())
   {
      RequestDisplayMode(w, h, full);
      return T;
   }
#elif MOBILE
   return T; // we can't manually change mode for mobile platforms at all because they're always fullscreen
#elif WEB
   if(created())
   {
      Bool f=((full<0) ? T.full() : (full!=0));
      if(T.full()!=f)
      {
         if(f)
         {
            EmscriptenFullscreenStrategy f; Zero(f);
            f.scaleMode                =EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
            f.canvasResolutionScaleMode=EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
            f.filteringMode            =EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;
            emscripten_request_fullscreen_strategy(null, true, &f);
            return T; // after request was made, return and wait until Emscripten handles it
         }else
         {
            emscripten_exit_fullscreen();
            return T; // after request was made, return and wait until Emscripten handles it, we can't process canvas resizes here, because Emscripten still needs to do some stuff
         }
      }
      if(w<=0)w=resW();
      if(h<=0)h=resH();
      if(f // can't change resolution when in full screen mode
      || w==resW() && h==resH() // nothing to change
      )return T;

      // resize canvas
      Flt  zoom=browserZoom();
      Vec2 css_size=VecI2(w, h)/zoom; // calculate css
      emscripten_set_element_css_size   (null, css_size.x, css_size.y); // this accepts floating point sizes
      emscripten_set_canvas_element_size(null, w, h);
      extern Flt ScreenScale; ScreenScale=zoom; // here we maintain 1:1 pixel ratio so we can set scale to zoom
   }
#endif
   Int cur_w   =T.resW(),
       cur_h   =T.resH(),
       cur_full=T.full();
   RESET_RESULT result0=modeTry(w, h, full); // try to set new mode
   if(result0!=RESET_OK)
   {
      RESET_RESULT result1=modeTry(cur_w, cur_h, cur_full); // try to set old mode
      if(result1!=RESET_OK)ResetFailed(result0, result1);
   }
   return T;
}
Display& Display::toggle(Bool window_size)
{
   if(!created())_full^=1;else
   {
      if(full()     )mode(App._window_size.x, App._window_size.y, false);else // if app was in fullscreen then set windowed mode based on last known window size
      if(window_size)mode(App._window_size.x, App._window_size.y, true );else // if set        fullscreen using                           last known window size
      {  // set full screen based on resolution of the monitor
      #if FORCE_MAIN_DISPLAY
         mode(screenW(), screenH(), true);
      #else
         RectI full, work; VecI2 max_normal_win_client_size, maximized_win_client_size;
          curMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);
         mode(full.w(), full.h(), true);
      #endif
      }
   }
   return T;
}
Display& Display::full(Bool full, Bool window_size)
{
   if(!created())T._full=full;else
   if(full!=T.full())toggle(window_size);
   return T;
}
Bool     Display::highMonitorPrecision()C {return monitorPrecision()>IMAGE_PRECISION_8 && full() && exclusive();} // on Windows we need fullscreen and exclusive to be able to really enable it, without it, it will be only 8-bit
Display& Display::    monitorPrecision(IMAGE_PRECISION precision)
{
   Clamp(precision, IMAGE_PRECISION_8, IMAGE_PRECISION(IMAGE_PRECISION_NUM-1));
   if(!created())_monitor_prec=precision;else
   if(monitorPrecision()!=precision){_monitor_prec=precision; if(findMode())Reset();}
   return T;
}
Display& Display::exclusive(Bool exclusive)
{
   if(_exclusive!=exclusive)
   {
     _exclusive=exclusive;
   #if WINDOWS_OLD && (DX9 || DX11)
      if(created() && full())
      {
      #if DX11 // on DX11 have to disable 'SetFullscreenState' first, otherwise custom resolutions may get reverted to desktop resolution
         if(SwapChain)SwapChain->SetFullscreenState(false, null);
      #endif
         if(findMode())Reset();
      }
   #endif
   }
   return T;
}
/******************************************************************************/
void Display::validateCoords(Int eye)
{
   if(Sh.h_Coords)
   {
      Vec2 coords_mul(1/w(), 1/h()),
           coords_add=0;  // or coords_mul*_draw_offset;
      if(InRange(eye, 2)) // stereo
      {
         coords_add.x+=ProjMatrixEyeOffset[eye];
         coords_mul.x*=2;
      }else
      if(!_view_active.full)
      {
         Vec2 ds_vs(Flt(Renderer.resW())/_view_active.recti.w(),
                    Flt(Renderer.resH())/_view_active.recti.h());
         coords_mul  *=ds_vs;
         coords_add  *=ds_vs;
         coords_add.x-=Flt(_view_active.recti.min.x+_view_active.recti.max.x-Renderer.resW())/_view_active.recti.w();
         coords_add.y+=Flt(_view_active.recti.min.y+_view_active.recti.max.y-Renderer.resH())/_view_active.recti.h();
      }

   #if DX9
      coords_add.x-=1.0f/_view_active.recti.w();
      coords_add.y+=1.0f/_view_active.recti.h();
   #elif GL
      if(!mainFBO()) // in OpenGL when drawing to custom RenderTarget the 'dest.pos.y' must be flipped
      {
         CHS(coords_mul.y);
         CHS(coords_add.y);
      }
   #endif

      Sh.h_Coords->setConditional(Vec4(coords_mul, coords_add));
   }
}
/******************************************************************************/
void Display::sizeChanged()
{
   D._size =D._unscaled_size/D._scale;
   D._size2=D._size         *2;

 C VecI2 &res=((VR.active() && D._allow_stereo) ? VR.guiRes() : D.res());
   D._pixel_size    .set( w2()/res.x,  h2()/res.y); D._pixel_size_2=D._pixel_size*0.5f;
   D._pixel_size_inv.set(res.x/ w2(), res.y/ h2());

   // this is used by mouse/touch pointers
   D._window_pixel_to_screen_mul.set( D.w2()/(D.resW()-1),  // div by "resW-1" so we can have full -D.w .. D.w range !! if this is changed for some reason, then adjust 'rect.max' in 'Ms.clipUpdate' !!
                                     -D.h2()/(D.resH()-1)); // div by "resH-1" so we can have full -D.h .. D.h range !! if this is changed for some reason, then adjust 'rect.max' in 'Ms.clipUpdate' !!
   D._window_pixel_to_screen_add.set(-D.w(), D.h());
   if(VR.active()) // because VR Gui Rect may be different than Window Rect, we need to adjust scaling
   {
      // '_window_pixel_to_screen_scale' is set so that 'windowPixelToScreen' will always point to VR 'GuiTexture', no matter if 'D._allow_stereo' is enabled/disabled (drawing to 'GuiTexture' or System Window)
      D._window_pixel_to_screen_mul*=D._window_pixel_to_screen_scale;
      D._window_pixel_to_screen_add*=D._window_pixel_to_screen_scale;
   }

   viewReset();
}
Display& Display::scale(Flt scale)
{
   if(T._scale!=scale)
   {
      T._scale=scale;
      if(created())
      {
         Vec2 old_size=size();
           sizeChanged();
         screenChanged(old_size.x, old_size.y);
      }
   }
   return T;
}
void Display::densityUpdate()
{
again:
  _render_res=ByteScale2Res(res(), densityByte());
   if(_render_res.max()>maxTexSize() && maxTexSize()>0 && densityUpsample()) // if calculated size exceeds possible max texture size, then we need to clamp the density, don't try to go below 1.0 density
   {
      Flt  max_density_f=Flt(maxTexSize())/res().max();
      Byte max_density  =FltToByteScale2(max_density_f);
      if(_density>max_density)_density=max_density;else _density--;
      goto again;
   }
}
Bool Display::densityFast(Byte density)
{
   if(density!=densityByte())
   {
      T._density=density;
      densityUpdate();
      return true;
   }
   return false;
}
Flt      Display::density(           )C {return ByteScale2ToFlt(densityByte());}
Display& Display::density(Flt density)
{
   Byte b=FltToByteScale2(density);
   if(densityFast(b))Renderer.rtClean();
   return T;
}
Display& Display::densityFilter(FILTER_TYPE filter) {_density_filter=filter; return T;}
Display& Display::samples(Byte samples)
{
   samples=DisplaySamples(samples);
   if(T._samples!=samples){T._samples=samples; Renderer.rtClean();}
   return T;
}
/******************************************************************************/
Display& Display::highPrecColRT(Bool on)
{
   if(created() && _mrt_const_bit_size)on=false;
   if(_hp_col_rt!=on){_hp_col_rt=on; Renderer.rtClean();}
   return T;
}
Display& Display::highPrecNrmRT(Bool on)
{
   if(created() && _mrt_const_bit_size)on=false;
   if(_hp_nrm_rt!=on){_hp_nrm_rt=on; Renderer.rtClean();}
   return T;
}
Display& Display::highPrecLumRT(Bool on)
{
   // this does not require '_mrt_const_bit_size' because it's always used as a single RT and not one of many
   if(_hp_lum_rt!=on){_hp_lum_rt=on; Renderer.rtClean();}
   return T;
}
Display& Display::litColRTPrecision(IMAGE_PRECISION precision)
{
   // this does not require '_mrt_const_bit_size' because it's always used as a single RT and not one of many
   Clamp(precision, IMAGE_PRECISION_8, IMAGE_PRECISION(IMAGE_PRECISION_NUM-1));
   if(_lit_col_rt_prec!=precision){_lit_col_rt_prec=precision; Renderer.rtClean();}
   return T;
}
Display& Display::highPrecNrmCalc(Bool on) {_hp_nrm_calc=on; return T;}
/******************************************************************************/
void Display::setSync()
{
   SyncLocker locker(_lock);
   if(created())
   {
   #if DX9 || DX11
      if(findMode())Reset();
   #elif GL
         Bool sync=(T.sync() && !VR.active()); // if using VR then we have to disable screen sync, because HMD will handle this according to its own refresh rate
      #if WINDOWS
         wglSwapIntervalEXT(sync);
      #elif MAC
         Int value=sync; CGLSetParameter(MainContext.context, kCGLCPSwapInterval, &value);
      #elif LINUX
         if(glXSwapInterval)glXSwapInterval(XDisplay, App.Hwnd(), sync);
      #elif ANDROID
         eglSwapInterval(GLDisplay, sync);
      #elif WEB
         if(sync)emscripten_set_main_loop_timing(EM_TIMING_RAF       , 1);
         else    emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 0); // however be careful as on WEB disabling sync introduces stuttering
      #endif
   #endif
   }
}
Display& Display::sync(Bool sync) {if(T._sync!=sync){T._sync=sync; setSync();} return T;}
/******************************************************************************/
Display& Display::dither             (Bool             dither   ) {                                                                    if(T._dither          !=dither   ){T._dither          =dither   ;             } return T;}
Display& Display::maxLights          (Byte             max      ) {Clamp(max, 0, 255);                                                 if(T._max_lights      !=max      ){T._max_lights      =max      ;             } return T;}
Display& Display::texMacro           (Bool             use      ) {                                                                    if(T._tex_macro       !=use      ){T._tex_macro       =use      ; setShader();} return T;}
Display& Display::texDetail          (TEXTURE_USAGE    usage    ) {Clamp(usage, TEX_USE_DISABLE, TEXTURE_USAGE(TEX_USE_NUM-1));        if(T._tex_detail      !=usage    ){T._tex_detail      =usage    ; setShader();} return T;}
Display& Display::texReflection      (TEXTURE_USAGE    usage    ) {Clamp(usage, TEX_USE_DISABLE, TEXTURE_USAGE(TEX_USE_NUM-1));        if(T._tex_reflect     !=usage    ){T._tex_reflect     =usage    ; setShader();} return T;}
Display& Display::materialBlend      (Bool             per_pixel) {                                                                    if(T._mtrl_blend      !=per_pixel){T._mtrl_blend      =per_pixel; setShader();} return T;}
Display& Display::bendLeafs          (Bool             on       ) {                                                                    if(T._bend_leafs      !=on       ){T._bend_leafs      =on       ; setShader();} return T;}
Display& Display::outlineAffectSky   (Bool             on       ) {                                                                    if(T._outline_sky     !=on       ){T._outline_sky     =on       ;             } return T;}
Display& Display::outlineMode        (EDGE_DETECT_MODE mode     ) {Clamp(mode, EDGE_DETECT_NONE, EDGE_DETECT_MODE(EDGE_DETECT_NUM-1)); if(T._outline_mode    !=mode     ){T._outline_mode    =mode     ;             } return T;}
Display& Display::particlesSoft      (Bool             on       ) {                                                                    if(T._particles_soft  !=on       ){T._particles_soft  =on       ;             } return T;}
Display& Display::particlesSmoothAnim(Bool             on       ) {                                                                    if(T._particles_smooth!=on       ){T._particles_smooth=on       ;             } return T;}
Display& Display::eyeDistance        (Flt              dist     ) {                                                                    if(T._eye_dist        !=dist     ){T._eye_dist        =dist     ;             } return T;}

Display& Display::edgeDetect(EDGE_DETECT_MODE mode)
{
   Clamp(mode, EDGE_DETECT_NONE, EDGE_DETECT_MODE(EDGE_DETECT_NUM-1)); if(T._edge_detect!=mode)
   {
      T._edge_detect=mode; if(!Sh.h_EdgeDetect && mode && created())
      {
         Sh.h_EdgeDetect     =Sh.get("EdgeDetect");
         Sh.h_EdgeDetectApply=Sh.get("EdgeDetectApply");
      }
   }
   return T;
}
Display& Display::edgeSoften(EDGE_SOFTEN_MODE mode)
{
   Clamp(mode, EDGE_SOFTEN_NONE, EDGE_SOFTEN_MODE(EDGE_SOFTEN_NUM-1)); if(T._edge_soften!=mode)
   {
      T._edge_soften=mode; if(created())switch(mode) // techniques can be null if failed to load
      {
         case EDGE_SOFTEN_FXAA: if(!Sh.h_FXAA)
         {
            Sh.h_FXAA=Sh.find("FXAA");
         }break;

      #if SUPPORT_MLAA
         case EDGE_SOFTEN_MLAA: if(!Renderer._mlaa_area)
         {
            Sh.h_MLAAEdge =Sh.find("MLAAEdge" );
            Sh.h_MLAABlend=Sh.find("MLAABlend");
            Sh.h_MLAA     =Sh.find("MLAA"     );

            Renderer._mlaa_area.get("Img/MLAA Area.img");
         }break;
      #endif

         case EDGE_SOFTEN_SMAA: if(!Renderer._smaa_area)
         {
            Sh.h_SMAAEdge =Sh.find("SMAAEdgeColor"); // use 'SMAAEdgeColor' instead of 'SMAAEdgeLuma' to differentiate between different colors
            Sh.h_SMAABlend=Sh.find("SMAABlend"    );
            Sh.h_SMAA     =Sh.find("SMAA"         );
            Sh.h_SMAAThreshold=GetShaderParam("SMAAThreshold"); Sh.h_SMAAThreshold->set(D.smaaThreshold());

            Renderer._smaa_area  .get("Img/SMAA Area.img");
            Renderer._smaa_search.get("Img/SMAA Search.img");
         }break;
      }
   }
   return T;
}
Display& Display::smaaThreshold(Flt threshold)
{
   SAT(threshold); _smaa_threshold=threshold; if(Sh.h_SMAAThreshold)Sh.h_SMAAThreshold->setConditional(_smaa_threshold); return T;
}
Int      Display::secondaryOpenGLContexts(             )C {return GPU_API(0, 0, SecondaryContexts.elms());}
Display& Display::secondaryOpenGLContexts(Byte contexts)
{
#if GL && HAS_THREADS
   if(!created())SecondaryContexts.setNum(contexts);
#endif
   return T;
}
Bool Display::canUseGPUDataOnSecondaryThread()C
{
#if GL
   return created() && SecondaryContexts.elms(); // was created and there are some secondary GL contexts
#else
   return true;
#endif
}
/******************************************************************************/
Display& Display::aspectMode(ASPECT_MODE mode)
{
   Clamp(mode, ASPECT_MODE(0), ASPECT_MODE(ASPECT_NUM-1));
   if(T._aspect_mode!=mode)
   {
      T._aspect_mode=mode;
      aspectRatioEx();
   }
   return T;
}
void Display::aspectRatioEx(Bool force, Bool quiet)
{
   Flt aspect_ratio=_aspect_ratio_want;
#if DESKTOP || WEB
   RectI full, work; VecI2 max_normal_win_client_size, maximized_win_client_size;
    curMonitor(full, work, max_normal_win_client_size, maximized_win_client_size); // calculate based on current monitor, as connected monitors may have different aspects
   VecI2 size=full.size(); Flt desktop_aspect=(size.y ? Flt(size.x)/size.y : 1);

   if(aspect_ratio<=EPS)aspect_ratio=desktop_aspect; // if not specified then use default
#else
   Flt desktop_aspect=Renderer._main.aspect();
        aspect_ratio =Renderer._main.aspect();
#endif

   if(T._aspect_ratio!=aspect_ratio || force)
   {
      T._aspect_ratio=aspect_ratio;
      if(created())
      {
         Bool vr=(VR.active() && _allow_stereo);
	   #if DESKTOP || WEB
         Flt window_aspect=Flt(resW())/resH(),
      #else
         Flt window_aspect=Renderer._main.aspect(),
      #endif
                 vr_aspect=Flt(VR.guiRes().x)/VR.guiRes().y,
               mono_aspect=(D.full() ? aspect_ratio : aspect_ratio*window_aspect/desktop_aspect),
                    aspect=(vr ? vr_aspect : mono_aspect);
         Vec2     old_size=D.size();

         switch(aspectMode())
         {
            default            : aspect_y: _unscaled_size.y=1; _unscaled_size.x=_unscaled_size.y*aspect; break; // ASPECT_Y
            case ASPECT_X      : aspect_x: _unscaled_size.x=1; _unscaled_size.y=_unscaled_size.x/aspect; break;
            case ASPECT_SMALLER: if(aspect>=1)goto aspect_y; goto aspect_x;
         }
         T._pixel_aspect=(vr ? 1 : D.full() ? aspect_ratio/window_aspect : aspect_ratio/desktop_aspect);

         // '_window_pixel_to_screen_scale' is set so that 'windowPixelToScreen' will always point to VR 'GuiTexture', no matter if 'D._allow_stereo' is enabled/disabled (drawing to 'GuiTexture' or System Window)
         if(!VR.active())_window_pixel_to_screen_scale=1;else
         if( vr)
         {
            if(mono_aspect>vr_aspect)_window_pixel_to_screen_scale.set(mono_aspect/vr_aspect, 1);
            else                     _window_pixel_to_screen_scale.set(1, vr_aspect/mono_aspect);
         }else
         {
            if(mono_aspect>vr_aspect)_window_pixel_to_screen_scale=1;
            else                     _window_pixel_to_screen_scale=vr_aspect/mono_aspect;
         }

         sizeChanged();

         if(!quiet)screenChanged(old_size.x, old_size.y);
      }
   }
}
Display& Display::aspectRatio(Flt aspect_ratio)
{
   T._aspect_ratio_want=Max(0, aspect_ratio);
   aspectRatioEx(false);
   return T;
}
/******************************************************************************/
Display& Display::texFilter(Byte filter)
{
   if(created())MIN(filter, maxTexFilter());
   if(T._tex_filter!=filter)
   {
      T._tex_filter=filter;
      if(created())
      {
      #if DX9
         // texture filtering is set during rendering
      #elif DX11
         CreateAnisotropicSampler();
      #elif GL
         Images.lock  (); REPA(Images)Images.lockedData(i).setGLParams();
         Images.unlock();
      #endif
      }
   }
   return T;
}
Display& Display::texMipFilter(Bool on)
{
   if(T._tex_mip_filter!=on)
   {
      T._tex_mip_filter=on;
      if(created())
      {
      #if DX9
         // texture filtering is set during rendering
      #elif DX11
         CreateAnisotropicSampler();
      #elif GL
         Images.lock  (); REPA(Images)Images.lockedData(i).setGLParams();
         Images.unlock();
      #endif
      }
   }
   return T;
}
Display& Display::texLod(Byte lod)
{
   Clamp(lod, 0, 16);
   if(T._tex_lod!=lod)
   {
      T._tex_lod=lod;
      if(created())
      {
      #if DX11
         CreateAnisotropicSampler();
      #endif
      }
   }
   return T;
}
/******************************************************************************/
Display& Display::fontSharpness(Flt value)
{
   if(T._font_sharpness!=value)
   {
      T._font_sharpness=value;
      if(created())
      {
      #if DX9
         // sampler is set when drawing font
      #elif DX11
         CreateFontSampler();
      #elif GL
         Fonts.  lock(); REPA(Fonts)Fonts.lockedData(i).setGLFont();
         Fonts.unlock();
      #endif
      }
   }
   return T;
}
/******************************************************************************/
Display& Display::gamma   (Flt gamma) {if(T._gamma!=gamma){T._gamma=gamma; gammaSet();} return T;}
void     Display::gammaSet()
{
   if(created())
   {
      Flt  exp_want=ScaleFactor(gamma()*-0.5f);
      Bool separate=false; // if we can set gamma separately for the system (all monitors) and monitor in use
   #if DX9
      separate=!D3DPP.Windowed; // 'SetGammaRamp' will succeed only in true full screen
   #elif DX11
      #if WINDOWS_OLD
         separate=!SwapChainDesc.Windowed; // 'SetGammaControl' will succeed only in true full screen
      #else
         separate=true;
      #endif
   #endif

      // !! set system gamma first so it won't override device output gamma !!
      Flt exp=(separate ? 1 : exp_want);
      if(_gamma_all || exp!=1) // if custom gamma is already set, or we want to apply custom gamma
      {
      #if WINDOWS_OLD
         SyncLocker locker(_lock);
         if(HDC hdc=GetDC(null))
	      {
            UShort gr[3][256];
            REP(256)
            {
               gr[0][i]=RoundU(Pow(_gamma_array[0][i]/65535.0f, exp)*0xFFFF);
               gr[1][i]=RoundU(Pow(_gamma_array[1][i]/65535.0f, exp)*0xFFFF);
               gr[2][i]=RoundU(Pow(_gamma_array[2][i]/65535.0f, exp)*0xFFFF);
            }
		      SetDeviceGammaRamp(hdc, gr);
            ReleaseDC(null, hdc);
	      }
      #elif MAC
         CGGammaValue r[256], g[256], b[256];
         REP(256)
         {
            r[i]=Pow(_gamma_array[0][i]/65535.0f, exp);
            g[i]=Pow(_gamma_array[1][i]/65535.0f, exp);
            b[i]=Pow(_gamma_array[2][i]/65535.0f, exp);
         }
         SyncLocker locker(_lock);
         CGSetDisplayTransferByTable(kCGDirectMainDisplay, 256, r, g, b);
      #elif LINUX
         UShort r[256], g[256], b[256];
         REP(256)
         {
            r[i]=RoundU(Pow(_gamma_array[0][i]/65535.0f, exp)*0xFFFF);
            g[i]=RoundU(Pow(_gamma_array[1][i]/65535.0f, exp)*0xFFFF);
            b[i]=RoundU(Pow(_gamma_array[2][i]/65535.0f, exp)*0xFFFF);
         }
         XF86VidModeSetGammaRamp(XDisplay, DefaultScreen(XDisplay), 256, r, g, b);
      #endif
        _gamma_all=(exp!=1); // if gamma is set for all monitors
      }

      if(separate)
      {
         exp=exp_want;
      #if DX9
         D3DGAMMARAMP gr;
         REP(256)
         {
            gr.red  [i]=RoundU(Pow(_gamma_array[0][i]/65535.0f, exp)*0xFFFF);
            gr.green[i]=RoundU(Pow(_gamma_array[1][i]/65535.0f, exp)*0xFFFF);
            gr.blue [i]=RoundU(Pow(_gamma_array[2][i]/65535.0f, exp)*0xFFFF);
         }
         SyncLocker locker(_lock);
         D3D->SetGammaRamp(0, D3DSGR_CALIBRATE, &gr);
      #elif DX11
         SyncLocker locker(_lock);
         IDXGIOutput *output=null; SwapChain->GetContainingOutput(&output); if(output)
         {
            DXGI_GAMMA_CONTROL gc;
            gc.Scale .Red=gc.Scale .Green=gc.Scale .Blue=1;
            gc.Offset.Red=gc.Offset.Green=gc.Offset.Blue=0;
            REP(256)
            {
               gc.GammaCurve[i].Red  =Pow(_gamma_array[0][i]/65535.0f, exp);
               gc.GammaCurve[i].Green=Pow(_gamma_array[1][i]/65535.0f, exp);
               gc.GammaCurve[i].Blue =Pow(_gamma_array[2][i]/65535.0f, exp);
            }
            output->SetGammaControl(&gc);
            output->Release();
         }
      #endif
      }
   }
}
/******************************************************************************/
Display& Display::bumpMode(BUMP_MODE mode)
{
   Clamp(mode, BUMP_FLAT, BUMP_MODE(BUMP_NUM-1));
   if(_bump_mode!=mode){_bump_mode=mode; setShader();}
   return T;
}
/******************************************************************************/
Display& Display:: glowAllow   (Bool allow   ) {                     _glow_allow   =allow   ; return T;}
Display& Display::bloomAllow   (Bool allow   ) {                    _bloom_allow   =allow   ; return T;}
Display& Display::bloomOriginal(Flt  original) {MAX  (original, 0); _bloom_original=original; return T;}
Display& Display::bloomScale   (Flt  scale   ) {MAX  (scale   , 0); _bloom_scale   =scale   ; return T;}
Display& Display::bloomCut     (Flt  cut     ) {MAX  (cut     , 0); _bloom_cut     =cut     ; return T;}
Display& Display::bloomHalf    (Bool half    ) {                    _bloom_half    =half    ; return T;}
Display& Display::bloomBlurs   (Byte blurs   ) {Clamp(blurs, 0, 4); _bloom_blurs   =blurs   ; return T;}
Display& Display::bloomSamples (Bool high    ) {                    _bloom_samples =high    ; return T;}
Display& Display::bloomSaturate(Bool saturate) {                    _bloom_sat     =saturate; return T;}
Display& Display::bloomMaximum (Bool on      ) {if(_bloom_max!=on){ _bloom_max     =on      ; if(!Sh.h_MaxX && on && created()){Sh.h_MaxX=Sh.get("MaxX"); Sh.h_MaxY=Sh.get("MaxY");}} return T;}
Bool     Display::bloomUsed    (             )C{return bloomAllow() && (!Equal(bloomOriginal(), 1, EPS_COL) || !Equal(bloomScale(), 0, EPS_COL));}
/******************************************************************************/
Display& Display::volLight(Bool on ) {_vol_light=    on     ; return T;}
Display& Display::volAdd  (Bool add) {_vol_add  =    add    ; return T;}
Display& Display::volMax  (Flt  max) {_vol_max  =Max(max, 0); return T;}
/******************************************************************************/
Display& Display::shadowMode         (SHADOW_MODE mode) {Clamp(mode, SHADOW_NONE, SHADOW_MODE(SHADOW_NUM-1)); _shd_mode=mode; return T;}
Display& Display::shadowJitter       (Bool     on     ) {if(_shd_jitter!=on){_shd_jitter^=1; shadowJitterSet();} return T;}
Display& Display::shadowReduceFlicker(Bool     reduce ) {_shd_reduce    =    reduce                     ; return T;}
Display& Display::shadowFrac         (Flt      frac   ) {_shd_frac      =Sat(frac                      ); return T;}
Display& Display::shadowFade         (Flt      fade   ) {_shd_fade      =Sat(fade                      ); return T;}
Display& Display::shadowSoft         (Byte     soft   ) {_shd_soft      =Min(soft   , SHADOW_SOFT_NUM-1); return T;}
Display& Display::shadowMapNum       (Byte     map_num) {_shd_map_num   =Mid(map_num,    1,    6       ); return T;}
Display& Display::shadowMapSizeLocal (Flt      frac   ) {_shd_map_size_l=Sat(frac                      ); return T;}
Display& Display::shadowMapSizeCone  (Flt      factor ) {_shd_map_size_c=Mid(factor , 0.0f, 2.0f       ); return T;}
Display& Display::shadowMapSplit     (C Vec2  &factor ) {_shd_map_split .set(Max(2, factor.x), Max(0, factor.y)); return T;}

Display& Display::shadowMapSize(Int map_size)
{
   MAX(map_size, 0); if(_shd_map_size!=map_size){_shd_map_size=map_size; if(created())Renderer.createShadowMap();}
   return T;
}
Display& Display::cloudsMapSize(Int map_size)
{
   MAX(map_size, 0); if(_cld_map_size!=map_size){_cld_map_size=map_size; if(created())Renderer.createShadowMap();}
   return T;
}
Int Display::shadowMapNumActual()C
{
   return (Renderer._cur_type==RT_FORWARD) ? Ceil2(shadowMapNum()) // align to even numbers on RT_FORWARD
                                           :       shadowMapNum();
}
Bool Display::shadowSupported()C
{
   return Renderer._shd_map.is();
}
void Display::shadowJitterSet()
{
   if(Sh.h_ShdJitter)
   {
      Vec2 j=Flt(shadowJitter())/Renderer._shd_map.hwSize();
      Sh.h_ShdJitter->set(Vec4(j, j*-0.5f));
   }
}
/******************************************************************************/
Bool Display::aoWant()C
{
   return ambientMode()!=AMBIENT_FLAT
      && (ambientColor()+nightShadeColor()).max()>EPS_COL; // no need to calculate AO if it's too small
}
Flt      Display::ambientRes     (                       )C {return   ByteScaleToFlt(_amb_res);}
Display& Display::ambientRes     (  Flt          scale   )  {Byte res=FltToByteScale( scale  );    if(res!=_amb_res){_amb_res =res ; Renderer.rtClean();} return T;}
Display& Display::ambientMode    (  AMBIENT_MODE mode    )  {Clamp(mode, AMBIENT_FLAT, AMBIENT_MODE(AMBIENT_NUM-1)); _amb_mode=mode;                      return T;}
Display& Display::ambientSoft    (  Byte         soft    )  {MIN  (soft,                       AMBIENT_SOFT_NUM-1 ); _amb_soft=soft;                      return T;}
Display& Display::ambientJitter  (  Bool         jitter  )  {_amb_jitter=jitter;                                                                          return T;}
Display& Display::ambientNormal  (  Bool         normal  )  {_amb_normal=normal;                                                                          return T;}
Display& Display::ambientPower   (  Flt          power   )  {MAX(power, 0);                                             if(_amb_color   !=power   ){_amb_color   =power   ; if(Renderer.ambient_color)Renderer.ambient_color->set(ambientColor   ());} return T;}
Display& Display::ambientColor   (C Vec         &color   )  {Vec  c(Max(color.x, 0), Max(color.y, 0), Max(color.z, 0)); if(_amb_color   !=c       ){_amb_color   =c       ; if(Renderer.ambient_color)Renderer.ambient_color->set(ambientColor   ());} return T;}
Display& Display::ambientContrast(  Flt          contrast)  {MAX(contrast, 0);                                          if(_amb_contrast!=contrast){_amb_contrast=contrast; if(Sh.h_AmbientContrast  )Sh.h_AmbientContrast  ->set(ambientContrast());} return T;}
Display& Display::ambientRange   (C Vec2        &range   )  {Vec2 r(Max(range.x, 0.01f), Max(range.y, 0.01f));          if(_amb_range   !=r       ){_amb_range   =r       ; if(Sh.h_AmbientRange     )Sh.h_AmbientRange     ->set(ambientRange   ());} return T;}
Display& Display::ambientScale   (  Flt          scale   )  {MAX(scale, 0.05f);                                         if(_amb_scale   !=scale   ){_amb_scale   =scale   ; if(Sh.h_AmbientScale     )Sh.h_AmbientScale     ->set(ambientScale   ());} return T;}
Display& Display::ambientBias    (  Flt          bias    )  {SAT(bias);                                                 if(_amb_bias    !=bias    ){_amb_bias    =bias    ; if(Sh.h_AmbientBias      )Sh.h_AmbientBias      ->set(ambientBias    ());} return T;}
/******************************************************************************/
Display& Display::nightShadeColor(C Vec &color) {Vec c(Max(color.x, 0), Max(color.y, 0), Max(color.z, 0)); if(_ns_color!=c){_ns_color=c; if(Sh.h_NightShadeColor)Sh.h_NightShadeColor->set(_ns_color);} return T;}
/******************************************************************************/
Flt      Display::motionRes   (                  )C {return   ByteScaleToFlt(_mtn_res);}
Display& Display::motionRes   (Flt         scale )  {Byte res=FltToByteScale(scale); if(res!=_mtn_res){_mtn_res=res; Renderer.rtClean();}                                     return T;}
Display& Display::motionMode  (MOTION_MODE mode  )  {Clamp(mode , MOTION_NONE   , MOTION_MODE(MOTION_NUM-1));                       _mtn_mode  =mode ;                        return T;}
Display& Display::motionDilate(DILATE_MODE mode  )  {Clamp(mode , DILATE_MODE(0), DILATE_MODE(DILATE_NUM-1));                       _mtn_dilate=mode ;                        return T;}
Display& Display::motionScale (Flt         scale )  {MAX  (scale, 0                                        ); if(_mtn_scale!=scale){_mtn_scale =scale; MotionScaleChanged();} return T;} // this parameter affects 'Camera.set' -> 'CamMatrixInvMotionScale', and 'SetAngVelShader'
/******************************************************************************/
Display& Display::dofMode     (DOF_MODE mode     ) {Clamp(mode, DOF_NONE, DOF_MODE(DOF_NUM-1)); _dof_mode     =mode             ; return T;}
Display& Display::dofFocusMode(Bool     realistic) {                                            _dof_foc_mode =(realistic!=0)   ; return T;}
Display& Display::dofFocus    (Flt      z        ) {                                            _dof_focus    =Max(z        , 0); return T;}
Display& Display::dofRange    (Flt      range    ) {                                            _dof_range    =Max(range    , 0); return T;}
Display& Display::dofIntensity(Flt      intensity) {                                            _dof_intensity=Max(intensity, 0); return T;}
/******************************************************************************/
Display& Display::eyeAdaptation          (  Bool on        ) {                                                          _eye_adapt           =on        ;                                                                            return T;}
Display& Display::eyeAdaptationBrightness(  Flt  brightness) {MAX(brightness, 0); if(_eye_adapt_brightness!=brightness){_eye_adapt_brightness=brightness; if(Sh.h_HdrBrightness)Sh.h_HdrBrightness->set(eyeAdaptationBrightness());} return T;}
Display& Display::eyeAdaptationMaxDark   (  Flt  max_dark  ) {MAX(max_dark  , 0); if(_eye_adapt_max_dark  !=max_dark  ){_eye_adapt_max_dark  =max_dark  ; if(Sh.h_HdrMaxDark   )Sh.h_HdrMaxDark   ->set(eyeAdaptationMaxDark   ());} return T;}
Display& Display::eyeAdaptationMaxBright (  Flt  max_bright) {MAX(max_bright, 0); if(_eye_adapt_max_bright!=max_bright){_eye_adapt_max_bright=max_bright; if(Sh.h_HdrMaxBright )Sh.h_HdrMaxBright ->set(eyeAdaptationMaxBright ());} return T;}
Display& Display::eyeAdaptationSpeed     (  Flt  speed     ) {MAX(speed     , 1); if(_eye_adapt_speed     !=speed     ){_eye_adapt_speed     =speed     ;                                                                          } return T;}
Display& Display::eyeAdaptationWeight    (C Vec &weight    ) {                    if(_eye_adapt_weight    !=weight    ){_eye_adapt_weight    =weight    ; if(Sh.h_HdrWeight    )Sh.h_HdrWeight    ->set(eyeAdaptationWeight()/4  );} return T;}
Display& Display::resetEyeAdaptation     (  Flt  brightness)
{
   if(Renderer._eye_adapt_scale[0].is())
   {
      MAX(brightness, 0);
      SyncLocker locker(_lock);
      REPAO(Renderer._eye_adapt_scale).clearFull(brightness, true);
   }
   return T;
}
/******************************************************************************/
Display& Display::grassDensity(Flt  density) {_grass_density=Sat(density); return T;}
Display& Display::grassShadow (Bool on     ) {_grass_shadow =    on      ; return T;}
Display& Display::grassMirror (Bool on     ) {_grass_mirror =    on      ; return T;}
Display& Display::grassRange  (Flt  range  )
{
   MAX(range, 0); if(_grass_range!=range)
   {
     _grass_range    =range;
     _grass_range_sqr=Sqr(_grass_range);
      if(Sh.h_GrassRangeMulAdd)
      {
         Flt from=Sqr(_grass_range*0.8f), to=_grass_range_sqr, mul, add; if(to>from+EPSL){mul=1/(to-from); add=-from*mul;}else{mul=0; add=1;} // else{no grass} because distance is 0
         Sh.h_GrassRangeMulAdd->set(Vec2(mul, add));
      }
   }
   return T;
}
static Flt BendFactor;
Display& Display::grassUpdate()
{
   BendFactor+=Time.d();
   if(Sh.h_BendFactor)Sh.h_BendFactor->set(Vec4(1.6f, 1.2f, 1.4f, 1.1f)*BendFactor+Vec4(0.1f, 0.5f, 0.7f, 1.1f));
   return T;
}
/******************************************************************************/
Display& Display::furStaticGravity (Flt gravity  ) {_fur_gravity  =gravity  ; return T;}
Display& Display::furStaticVelScale(Flt vel_scale) {_fur_vel_scale=vel_scale; return T;}
/******************************************************************************/
void Display::lodSetCurrentFactor()
{
   // '_lod_current_factor' contains information about '_lod_factors_fov' in current rendering mode
  _lod_current_factor=_lod_factors_fov[Renderer()==RM_SHADOW][Renderer.mirror()];
}
void Display::lodUpdateFactors()
{
   // '_lod_fov2' is a value based on Fov (viewFovY()), squared
   if(FovPerspective(viewFovMode()))_lod_fov2=Sqr(Tan(viewFovY()*0.5f));
   else                             _lod_fov2=Sqr(    viewFovY()      );
   REPD(s, 2)
   REPD(m, 2)_lod_factors_fov[s][m]=_lod_factors[s][m]*_lod_fov2;
   lodSetCurrentFactor();
}
Display& Display::lod(Flt general, Flt shadow, Flt mirror)
{
   // set values
  _lod_factor       =Max(0, general);
  _lod_factor_shadow=Max(0, shadow );
  _lod_factor_mirror=Max(0, mirror );

   // build precomputed helper array
   REPD(s, 2)
   REPD(m, 2)
   {
      Flt &factor =_lod_factors[s][m];
           factor =_lod_factor;
      if(s)factor*=_lod_factor_shadow;
      if(m)factor*=_lod_factor_mirror;
           factor*=     factor; // make square
   }
   lodUpdateFactors();
   return T;
}
Display& Display::lodFactor      (Flt factor) {return lod(     factor, _lod_factor_shadow, _lod_factor_mirror);}
Display& Display::lodFactorShadow(Flt factor) {return lod(_lod_factor,      factor       , _lod_factor_mirror);}
Display& Display::lodFactorMirror(Flt factor) {return lod(_lod_factor, _lod_factor_shadow,      factor       );}
/******************************************************************************/
Display& Display::tesselationAllow    (Bool on     ) {                       if(_tesselation_allow    !=on     ) _tesselation_allow    =on     ;               return T;}
Display& Display::tesselation         (Bool on     ) {                       if(_tesselation          !=on     ){_tesselation          =on     ; setShader();} return T;}
Display& Display::tesselationHeightmap(Bool on     ) {                       if(_tesselation_heightmap!=on     ){_tesselation_heightmap=on     ; setShader();} return T;}
Display& Display::tesselationDensity  (Flt  density) {MAX(density, EPS_GPU); if(_tesselation_density  !=density){_tesselation_density  =density; if(Sh.h_TesselationDensity)Sh.h_TesselationDensity->set(tesselationDensity());} return T;}
/******************************************************************************/
void Display::setViewFovTan()
{
   Vec2 mul(Renderer.resW()/(_view_active.recti.w()*w()), Renderer.resH()/(_view_active.recti.h()*h()));
  _view_fov_tan_full=_view_active.fov_tan*mul;

   if(VR.active() && _allow_stereo
   && !(Renderer.inside() && !Renderer._stereo) // if we're inside rendering, and stereo is disabled, then we need to process this normally, this is so that we can do mono rendering in Gui, and use correct '_view_fov_tan' (PosToScreen,ScreenToPos) in that space
   )
   {
      Flt scale=VR.guiSize()*0.5f, aspect=Flt(VR.guiRes().x)/VR.guiRes().y;
     _view_fov_tan_gui.x=scale*aspect;
     _view_fov_tan_gui.y=scale;
      if(Renderer.inside() && Renderer._stereo)_view_fov_tan_gui.x*=0.5f;
     _view_fov_tan_gui*=mul;
   }else
   {
     _view_fov_tan_gui=_view_fov_tan_full;
   }

   if(Sh.h_DepthWeightScale)Sh.h_DepthWeightScale->set(_view_active.fov_tan.y*0.00714074011f);
}
void Display::viewUpdate()
{
   if(created())
   {
     _view_active=_view_main;
      if(_lock.owned())_view_active.setViewport(false); // set actual viewport only if we own the lock, this is because this method can be called outside of 'Draw' where we don't have the lock, however to avoid locking which could affect performance (for example GPU still owning the lock on other thread, for example for flipping back buffer, we would have to wait until it finished), we can skip setting the viewport because drawing is not allowed in update anyway. To counteract this skip here, instead we always reset the viewport at the start of Draw in 'DrawState'
     _view_active.setShader().setProjMatrix(true);
   }

  _view_from  =(FovPerspective(viewFovMode()) ? viewFrom() : 0);
  _view_fov   =(FovHorizontal (viewFovMode()) ? viewFovX() : viewFovY());
  _view_rect  =Renderer.pixelToScreen(viewRectI());
  _view_center=_view_rect.center();

   REPAO(_view_eye_rect)=_view_rect;
  _view_eye_rect[0].max.x=_view_eye_rect[1].min.x=_view_rect.centerX();

   setViewFovTan   ();
   validateCoords  ();
   lodUpdateFactors();
}
void Display::viewReset()
{
   Rect rect=viewRect(); _view_main.recti.set(0, -1); _view_rect.set(0, -1); if(_view_main.full)viewRect(null);else viewRect(rect); // if last was full then set full, otherwise set based on rect
}

Display& Display::view(C Rect  &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode) {_view_main.set(screenToPixelI(rect), from, range, fov, fov_mode); viewUpdate(); return T;}
Display& Display::view(C RectI &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode) {_view_main.set(               rect , from, range, fov, fov_mode); viewUpdate(); return T;}

Display& Display::viewRect (C RectI *rect              ) {RectI recti=(rect ? *rect : RectI(0, 0, Renderer.resW(), Renderer.resH())); if( viewRectI()      !=recti                        ){                       _view_main.setRect (recti      ).setFov(); viewUpdate();} return T;} // need to use 'Renderer.res' in case VR is enabled and we're rendering to its 'GuiTexture'
Display& Display::viewRect (C Rect  &rect              ) {RectI recti=screenToPixelI(rect);                                           if( viewRectI()      !=recti                        ){                       _view_main.setRect (recti      ).setFov(); viewUpdate();} return T;}
Display& Display::viewFrom (  Flt    from              ) {                                                                            if( viewFrom ()      !=from                         ){                       _view_main.setFrom (from       )         ; viewUpdate();} return T;}
Display& Display::viewRange(  Flt    range             ) {                                                                            if( viewRange()      !=range                        ){                       _view_main.setRange(range      )         ; viewUpdate();} return T;}
Display& Display::viewFov  (  Flt    fov, FOV_MODE mode) {                                                                            if( viewFov  ()      !=fov  || viewFovMode()!=mode  ){                       _view_main.setFov  (fov, mode  )         ; viewUpdate();} return T;}
Display& Display::viewFov  (C Vec2  &fov               ) {                                                                            if( viewFovXY()      !=fov  || viewFovMode()!=FOV_XY){                       _view_main.setFov  (fov, FOV_XY)         ; viewUpdate();} return T;}
Display& Display::viewForceSquarePixel(Bool on         ) {                                                                            if(_view_square_pixel!=on                           ){_view_square_pixel=on; _view_main.setFov  (           )         ; viewUpdate();} return T;}

Flt Display::viewQuadDist()C
{
   Vec    view_quad_max(_view_main.fov_tan.x*_view_main.from, _view_main.fov_tan.y*_view_main.from, _view_main.from);
   return view_quad_max.length();
}
/******************************************************************************/
// CLEAR
/******************************************************************************/
#define CLEAR_DEPTH_VALUE (!REVERSE_DEPTH) // Warning: for GL this is set at app startup and not here
#if DX9
// DX9 'clearDepth' clears partial depth buffer (only this covered by active viewport)
void Display::clear       (       C Color &color) {UInt flag=D3DCLEAR_TARGET; if(Renderer._cur_ds){     flag|=D3DCLEAR_ZBUFFER; if(ImageTI[Renderer._cur_ds->hwType()].s)flag|=D3DCLEAR_STENCIL;} D3D->Clear(0, null, flag            , VecB4(color.b, color.g, color.r, color.a).u, CLEAR_DEPTH_VALUE, 0); } // call will fail if D3DCLEAR_ZBUFFER/D3DCLEAR_STENCIL is specified but not available
void Display::clearCol    (       C Color &color) {                                                                                                                                               D3D->Clear(0, null, D3DCLEAR_TARGET , VecB4(color.b, color.g, color.r, color.a).u, CLEAR_DEPTH_VALUE, 0); }
void Display::clearDepth  (                     ) {                                                                                                                                               D3D->Clear(0, null, D3DCLEAR_ZBUFFER,                                           0, CLEAR_DEPTH_VALUE, 0); }
void Display::clearDS     (         Byte   s    ) {                           if(Renderer._cur_ds){UInt flag =D3DCLEAR_ZBUFFER; if(ImageTI[Renderer._cur_ds->hwType()].s)flag|=D3DCLEAR_STENCIL;  D3D->Clear(0, null, flag            ,                                           0, CLEAR_DEPTH_VALUE, s);}} // call will fail if                  D3DCLEAR_STENCIL is specified but not available
void Display::clearStencil(         Byte   s    ) {                                                                                                                                               D3D->Clear(0, null, D3DCLEAR_STENCIL,                                           0, CLEAR_DEPTH_VALUE, s); }
void Display::clearCol    (Int i, C Color &color) {RANGE_ASSERT(i, Renderer._cur); if(Image *image=Renderer._cur[i])image->clearHw(color);}
#elif DX11
void Display::clear(C Color &color)
{
   clearCol(color);
   clearDS (     );
}
void Display::clearCol(C Color &color) {return clearCol(color.asVec4());}
void Display::clearCol(C Vec4  &color)
{
   if(Renderer._cur[0])
   {
      if(D._view_active.full)Renderer._cur[0]->clearHw(color);else
      {
         Bool clip=D._clip_allow; D.clipAllow(false); ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Sh.clear(color);
                                  D.clipAllow(clip );                  D.alpha(alpha     );
      }
   }
}
void Display::clearCol(Int i, C Vec4 &color) {RANGE_ASSERT(i, Renderer._cur); if(Image *image=Renderer._cur[i])image->clearHw(color);}
// DX10+ 'clearDepth' always clears full depth buffer (viewport is ignored)
void Display::clearDepth  (      ) {if(Renderer._cur_ds)D3DC->ClearDepthStencilView(Renderer._cur_ds->_dsv, D3D11_CLEAR_DEPTH                                                                  , CLEAR_DEPTH_VALUE, 0);}
void Display::clearDS     (Byte s) {if(Renderer._cur_ds)D3DC->ClearDepthStencilView(Renderer._cur_ds->_dsv, D3D11_CLEAR_DEPTH|(ImageTI[Renderer._cur_ds->hwType()].s ? D3D11_CLEAR_STENCIL : 0), CLEAR_DEPTH_VALUE, s);}
void Display::clearStencil(Byte s) {if(Renderer._cur_ds)D3DC->ClearDepthStencilView(Renderer._cur_ds->_dsv,                                                            D3D11_CLEAR_STENCIL     , CLEAR_DEPTH_VALUE, s);}
#elif GL
void Display::clear(C Color &color)
{
   clearCol(color);
   clearDS (     );
}
void Display::clearCol(C Color &color) {return clearCol(color.asVec4());}
void Display::clearCol(C Vec4  &color)
{
   if(Renderer._cur[0])
   {
      if(D._clip_real)glDisable(GL_SCISSOR_TEST); // scissor test affects clear on OpenGL so we need to temporarily disable it to have the same functionality as in DirectX
      if(D._view_active.full)
      {
         glClearColor(color.x, color.y, color.z, color.w); // Values specified by glClearColor are clamped to the range [0,1] - https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glClearColor.xhtml
         glClear(GL_COLOR_BUFFER_BIT);
      }else
      {
         ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Sh.clear(color);
                          D.alpha(alpha     );
      }
      if(D._clip_real)glEnable(GL_SCISSOR_TEST);
   }
}
// 'glClearBufferfv' always clears full RT (viewport is ignored)
void Display::clearCol(Int i, C Vec4 &color)
{
   if(D.notShaderModelGLES2())
   {
      RANGE_ASSERT(i, Renderer._cur); if(Renderer._cur[i])glClearBufferfv(GL_COLOR, i, color.c);
   }
}
// GL 'clearDepth' always clears full depth buffer (viewport is ignored)
// Don't check for '_cur_ds_id' because this can be 0 for RenderBuffers
void Display::clearDepth  (      ) {if(Renderer._cur_ds){if(D._clip_real)glDisable(GL_SCISSOR_TEST);                    glClear(GL_DEPTH_BUFFER_BIT                                                                ); if(D._clip_real)glEnable(GL_SCISSOR_TEST);}}
void Display::clearDS     (Byte s) {if(Renderer._cur_ds){if(D._clip_real)glDisable(GL_SCISSOR_TEST); glClearStencil(s); glClear(GL_DEPTH_BUFFER_BIT|(ImageTI[Renderer._cur_ds->hwType()].s?GL_STENCIL_BUFFER_BIT:0)); if(D._clip_real)glEnable(GL_SCISSOR_TEST);}}
void Display::clearStencil(Byte s) {if(Renderer._cur_ds){if(D._clip_real)glDisable(GL_SCISSOR_TEST); glClearStencil(s); glClear(                                                           GL_STENCIL_BUFFER_BIT   ); if(D._clip_real)glEnable(GL_SCISSOR_TEST);}}
#endif
/******************************************************************************/
// CONVERT COORDINATES
/******************************************************************************/
Vec2 Display::screenToUV(C Vec2 &screen)
{
   return Vec2((screen.x+D.w())/D.w2(),
               (D.h()-screen.y)/D.h2());
}
Rect Display::screenToUV(C Rect &screen)
{
   return Rect((screen.min.x+D.w())/D.w2(), (D.h()-screen.max.y)/D.h2(),
               (screen.max.x+D.w())/D.w2(), (D.h()-screen.min.y)/D.h2());
}
Vec2 Display::UVToScreen(C Vec2 &uv)
{
   return Vec2(uv.x*D.w2()-D.w(),
               D.h()-uv.y*D.h2());
}
Vec2 Display::screenToPixel(C Vec2 &screen)
{
   return Vec2((screen.x+D.w())*D._pixel_size_inv.x,
               (D.h()-screen.y)*D._pixel_size_inv.y);
}
Rect Display::screenToPixel(C Rect &screen)
{
   return Rect((screen.min.x+D.w())*D._pixel_size_inv.x, (D.h()-screen.max.y)*D._pixel_size_inv.y,
               (screen.max.x+D.w())*D._pixel_size_inv.x, (D.h()-screen.min.y)*D._pixel_size_inv.y);
}

// use 'Round' instead of 'Floor' to match GPU vertex rounding, so that all 3 ways of rect drawing will be identical: Rect r; 1) r.draw 2) D.clip(r) full_rect.draw() 3) D.viewRect(r) full_rect.draw()
VecI2 Display::screenToPixelI(C Vec2 &screen) {return RoundGPU(screenToPixel(screen));}
RectI Display::screenToPixelI(C Rect &screen) {return RoundGPU(screenToPixel(screen));}

Vec2 Display::pixelToScreen(C Vec2 &pixel)
{
   return Vec2(pixel.x*D._pixel_size.x-D.w(),
               D.h()-pixel.y*D._pixel_size.y);
}
Vec2 Display::pixelToScreen(C VecI2 &pixel)
{
   return Vec2(pixel.x*D._pixel_size.x-D.w(),
               D.h()-pixel.y*D._pixel_size.y);
}
Rect Display::pixelToScreen(C Rect &pixel)
{
   return Rect(pixel.min.x*D._pixel_size.x-D.w(), D.h()-pixel.max.y*D._pixel_size.y,
               pixel.max.x*D._pixel_size.x-D.w(), D.h()-pixel.min.y*D._pixel_size.y);
}
Rect Display::pixelToScreen(C RectI &pixel)
{
   return Rect(pixel.min.x*D._pixel_size.x-D.w(), D.h()-pixel.max.y*D._pixel_size.y,
               pixel.max.x*D._pixel_size.x-D.w(), D.h()-pixel.min.y*D._pixel_size.y);
}
Vec2 Display::screenToPixelSize(C Vec2  &screen) {return screen*D._pixel_size_inv;}
Vec2 Display::pixelToScreenSize(  Flt    pixel ) {return  pixel*D._pixel_size    ;}
Vec2 Display::pixelToScreenSize(C Vec2  &pixel ) {return  pixel*D._pixel_size    ;}
Vec2 Display::pixelToScreenSize(C VecI2 &pixel ) {return  pixel*D._pixel_size    ;}

Vec2 Display::windowPixelToScreen(C Vec2 &pixel) // this is used by mouse/touch pointers
{
   return Vec2(pixel.x*D._window_pixel_to_screen_mul.x+D._window_pixel_to_screen_add.x,
               pixel.y*D._window_pixel_to_screen_mul.y+D._window_pixel_to_screen_add.y);
}
Vec2 Display::windowPixelToScreen(C VecI2 &pixel) // this is used by mouse/touch pointers
{
   return Vec2(pixel.x*D._window_pixel_to_screen_mul.x+D._window_pixel_to_screen_add.x,
               pixel.y*D._window_pixel_to_screen_mul.y+D._window_pixel_to_screen_add.y);
}
VecI2 Display::screenToWindowPixelI(C Vec2 &screen)
{
   return VecI2(Round((screen.x-D._window_pixel_to_screen_add.x)/D._window_pixel_to_screen_mul.x),
                Round((screen.y-D._window_pixel_to_screen_add.y)/D._window_pixel_to_screen_mul.y));
}
RectI Display::screenToWindowPixelI(C Rect &screen)
{  // since Y gets flipped, we need to swap min.y <-> max.y
   return RectI(Round((screen.min.x-D._window_pixel_to_screen_add.x)/D._window_pixel_to_screen_mul.x),
                Round((screen.max.y-D._window_pixel_to_screen_add.y)/D._window_pixel_to_screen_mul.y),
                Round((screen.max.x-D._window_pixel_to_screen_add.x)/D._window_pixel_to_screen_mul.x),
                Round((screen.min.y-D._window_pixel_to_screen_add.y)/D._window_pixel_to_screen_mul.y));
}

/* following functions were based on these codes:

   static Image img; if(!img.is()){img.create2D(16, 16, IMAGE_R8G8B8A8, 1); img.lock(); REPD(y,img.h())REPD(x,img.w())img.color(x, y, (x^y)&1 ? WHITE : BLACK); img.unlock();}
   for(Int i=0; i<D.resW(); i+=2)
   {
      Vec2 pos=D.pixelToScreen(Vec2(i*1.111f+0.5f, 0)), s=D.pixelToScreenSize(VecI2(1)), p;
      p=pos;
      Rect_LU(p.x, p.y, s.x, 0.1f).draw(RED, true);
      D.lineY(RED, p.x, p.y-0.2f, p.y-0.3f);

      p=pos-Vec2(0, 0.1f); D.screenAlignToPixel (p); Rect_LU(p.x, p.y, s.x, 0.1f).draw(RED, true);
      p=pos-Vec2(0, 0.1f); D.screenAlignToPixelL(p); D.lineY(RED, p.x, p.y-0.2f, p.y-0.3f);

      p=pos-Vec2(0, 0.4f); D.screenAlignToPixel(p);
      if(!(i%16))img.draw(Rect_LU(Vec2(p.x, p.y), D.pixelToScreenSize(img.size())));
   }
   D.text(0, D.h()-0.05f, "Rect");
   D.text(0, D.h()-0.15f, "Rect aligned");
   D.text(0, D.h()-0.25f, "Line");
   D.text(0, D.h()-0.35f, "Line aligned");
*/

Vec2 Display::screenAlignedToPixel      (C Vec2 &screen  ) {return pixelToScreen(screenToPixelI(screen));}
Vec2 Display::  alignScreenToPixelOffset(C Vec2 &screen  ) {return  screenAlignedToPixel(screen)-screen;}
void Display::  alignScreenToPixel      (  Vec2 &screen  ) {screen =screenAlignedToPixel(screen);}
void Display::  alignScreenToPixel      (  Rect &screen  ) {screen+=alignScreenToPixelOffset(screen.lu());}
void Display::  alignScreenXToPixel     (  Flt  &screen_x)
{
   Int pixel=RoundGPU((screen_x+D.w())*D._pixel_size_inv.x); // use 'RoundGPU' to match 'screenToPixelI'
    screen_x=pixel*D._pixel_size.x-D.w();
}
void Display::alignScreenYToPixel(Flt &screen_y)
{
   Int pixel=RoundGPU((D.h()-screen_y)*D._pixel_size_inv.y); // use 'RoundGPU' to match 'screenToPixelI'
    screen_y=D.h()-pixel*D._pixel_size.y;
}
/******************************************************************************/
// FADE
/******************************************************************************/
Bool Display::fading()C {return Renderer._fade || _fade_get;}
void Display::setFade(Flt seconds, Bool previous_frame)
{
   if(!VR.active()) // fading is currently not supported in VR mode
   {
      if(seconds<=0)
      {
         clearFade();
      }else
      if(previous_frame)
      {
         if(Renderer._main.is())
         {
            SyncLocker locker(_lock);
            Renderer._fade.get(ImageRTDesc(Renderer._main.w(), Renderer._main.h(), IMAGERT_RGB)); // doesn't use Alpha
            Renderer._main.copyHw(*Renderer._fade, true, null, null, &_fade_flipped);
           _fade_get =false  ;
           _fade_step=0      ;
           _fade_len =seconds;
         }
      }else
      {
        _fade_get=true;
        _fade_len=seconds;
      }
   }
}
void Display::clearFade()
{
   Renderer._fade.clear();
  _fade_get=_fade_flipped=false;
  _fade_step=_fade_len=0;
}
void Display::fadeUpdate()
{
   if(Renderer._fade && (_fade_step+=Time.ad()/_fade_len)>=1)clearFade();
}
void Display::fadeDraw()
{
   if(Renderer._fade)
   {
      Sh.h_Step->set(1-_fade_step);
      Sh.h_DrawA->draw(*Renderer._fade, _fade_flipped ? &Rect(-w(), h(), w(), -h()) : null);
   }
   if(_fade_get)
   {
     _fade_get =false;
     _fade_step=0    ;
      Renderer._fade.get(ImageRTDesc(Renderer._main.w(), Renderer._main.h(), IMAGERT_RGB)); // doesn't use Alpha
      Renderer._main.copyHw(*Renderer._fade, true, null, null, &_fade_flipped);
   }
}
/******************************************************************************/
// COLOR PALETTE
/******************************************************************************/
static void SetPalette(Int index, C ImagePtr &palette) // !! Warning: '_color_palette_soft' must be of IMAGE_SOFT IMAGE_R8G8B8A8 type because 'Particles.draw' codes require that !!
{
   D._color_palette[index]=palette;
   if(palette)palette->copyTry(D._color_palette_soft[index], -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1);
   else                        D._color_palette_soft[index].del();
}
Display& Display::colorPalette     (C ImagePtr &palette) {SetPalette(0, palette);    return T;}
Display& Display::colorPalette1    (C ImagePtr &palette) {SetPalette(1, palette);    return T;}
Display& Display::colorPaletteAllow(  Bool      on     ) {D._color_palette_allow=on; return T;}
/******************************************************************************/
// DIRECTX TEMPORARY DEVICE FOR DATA PROCESSING
/******************************************************************************/
#if WINDOWS_OLD
IDirect3DDevice9* GetD3D9()
{
#if DX9
   return D3D;
#else
   SyncLocker locker(D._lock);
   static IDirect3DDevice9 *dev=null;
   if(!dev)
   {
      if(IDirect3D9 *d3d=Direct3DCreate9(D3D_SDK_VERSION))
      {
         D3DPRESENT_PARAMETERS d3dpp; Zero(d3dpp);
         d3dpp.BackBufferWidth =0*D.resW();
         d3dpp.BackBufferHeight=0*D.resH();
         d3dpp.BackBufferCount =0*1;
         d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
         
         d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
         d3dpp.hDeviceWindow=App.Hwnd();
         d3dpp.Windowed=true;
         d3dpp.EnableAutoDepthStencil=false;
         d3dpp.Flags=0;
         d3dpp.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;
         d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;

         d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, App.Hwnd(), D3DCREATE_FPU_PRESERVE|D3DCREATE_NOWINDOWCHANGES|D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &dev);
      }
      if(!dev)Exit("Can't create DirectX 9 Device.\nTry using DX9 Engine version.");
   }
   return dev;
#endif
}
/******************************************************************************
ID3D10Device* GetD3D10()
{
   static ID3D10Device *dev=null;
#if DX11
   SyncLocker locker(D._lock);
           D3D10CreateDevice(null, D3D10_DRIVER_TYPE_HARDWARE , null, 0, D3D10_SDK_VERSION, &dev);
   if(!dev)D3D10CreateDevice(null, D3D10_DRIVER_TYPE_WARP     , null, 0, D3D10_SDK_VERSION, &dev);
   if(!dev)D3D10CreateDevice(null, D3D10_DRIVER_TYPE_REFERENCE, null, 0, D3D10_SDK_VERSION, &dev);
#endif
   if(!dev)Exit("Can't create DirectX 10 Device.\nTry using DX10 Engine version.");
   return dev;
}
/******************************************************************************/
#elif WINDOWS_NEW
Int DipsToPixels(Flt dips) {return Round(dips*ScreenScale);}
Flt PixelsToDips(Int pix ) {return       pix /ScreenScale ;}
#endif
/******************************************************************************/
}
/******************************************************************************/
