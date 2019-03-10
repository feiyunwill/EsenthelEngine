/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define SELECT_DIST_2 Sqr(0.013f)
#define SPEED         0.005f
#define BUF_BUTTONS   256

#if WINDOWS_OLD
   enum COOP_MODE
   {
      BACKGROUND,
      FOREGROUND,
      MOUSE_MODE=BACKGROUND, // use background mode so we can get correct information about relative movement
   };
   #define MS_RAW_INPUT 1
#elif WINDOWS_NEW
   using namespace Windows::System;
   using namespace Windows::UI::Core;
#endif
/******************************************************************************
#if WINDOWS_OLD
#define MOUSEEVENTF_MASK      0xFFFFFF00
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define SET_HOOK              1
static HHOOK MsHook;
static LRESULT CALLBACK MsLLProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   if(nCode>=0)
   {
    //MSLLHOOKSTRUCT &ms=*(MSLLHOOKSTRUCT*)lParam;
      MOUSEHOOKSTRUCT  &ms=*(MOUSEHOOKSTRUCT*)lParam;
      if((ms.dwExtraInfo&MOUSEEVENTF_MASK)==MOUSEEVENTF_FROMTOUCH)
         return 1;
   }
   return CallNextHookEx(MsHook, nCode, wParam, lParam);
}
static void SetHook() {if(!MsHook)MsHook=SetWindowsHookEx(WH_MOUSE_LL, MsLLProc, App._hinstance, 0);}
//static void SetHook() {if(!MsHook)MsHook=SetWindowsHookEx(WH_MOUSE, MsLLProc, App._hinstance, GetCurrentThreadId());}
static void  UnHook() {if( MsHook){UnhookWindowsHookEx(MsHook); MsHook=null;}}
#endif
/******************************************************************************/
struct MouseCursor
{
   void del   (); // delete manually
   Bool create(C Image &image, C VecI2 &hot_spot=VecI2(0, 0)); // create from image, false on fail

  ~MouseCursor()  {del();}
#if LINUX
   MouseCursor()  {       _cursor =NULL;}
   Bool     is()C {return _cursor!=NULL;}
#else
   MouseCursor()  {       _cursor =null;}
   Bool     is()C {return _cursor!=null;}
#endif

#if EE_PRIVATE
   #if WINDOWS_OLD
      HCURSOR _cursor;
   #elif WINDOWS_NEW
      CoreCursor ^_cursor;
   #elif MAC
      NSCursor *_cursor;
   #elif LINUX
      XCursor _cursor;
   #else
      Ptr _cursor;
   #endif
#else
private:
   Ptr   _cursor;
#endif
   Image _image;

   NO_COPY_CONSTRUCTOR(MouseCursor);
};
/******************************************************************************/
void MouseCursor::del()
{
#if WINDOWS_OLD
   if(_cursor){DestroyIcon(_cursor); _cursor=null;}
#elif WINDOWS_NEW
  _cursor=null;
#elif MAC
   [_cursor release]; _cursor=null;
#elif LINUX
   if(_cursor){if(XDisplay)XFreeCursor(XDisplay, _cursor); _cursor=NULL;}
#endif
  _image.del();
}
Bool MouseCursor::create(C Image &image, C VecI2 &hot_spot)
{
   del();
#if WINDOWS_OLD
  _cursor=CreateIcon(image, &hot_spot);
#elif WINDOWS_NEW
   // TODO: WINDOWS_NEW currently there's no way to dynamically create a 'CoreCursor'
#elif MAC // Mac must keep the cursor image data, it is stored in 'T._image', if released then cursor image data gets corrupted, that's why it must be kept in memory (yes that was tested)
   image.copy(_image, -1, -1, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1);
   REPD(y, _image.h())
   REPD(x, _image.w())
   {
      // premultiply alpha (required by Mac OS)
      Color c=_image.color(x, y);
      c.r=c.r*c.a/255;
      c.g=c.g*c.a/255;
      c.b=c.b*c.a/255;
     _image.color(x, y, c);
   }

   unsigned char *image_data=_image.data();
   if(NSBitmapImageRep *bitmap=[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&image_data
                                                                       pixelsWide:_image.w()
                                                                       pixelsHigh:_image.h()
                                                                    bitsPerSample:8
                                                                  samplesPerPixel:4
                                                                         hasAlpha:YES
                                                                         isPlanar:NO
                                                                   colorSpaceName:NSCalibratedRGBColorSpace
                                                                      bytesPerRow:_image.pitch()
                                                                     bitsPerPixel:32])
   {
      if(NSImage *ns_image=[[NSImage alloc] initWithSize:NSMakeSize(_image.w(), _image.h())])
      {
         [ns_image addRepresentation:bitmap];
         NSPoint point; point.x=hot_spot.x; point.y=hot_spot.y;
        _cursor=[[NSCursor alloc] initWithImage:ns_image hotSpot:point];
         [ns_image release];
      }
      [bitmap release];
   }
#elif LINUX
   if(XDisplay)
   {
      Image temp; C Image *src=&image;
      if(src->compressed())if(src->copyTry(temp, -1, -1, 1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1))src=&temp;else src=null;
      if(src && src->lockRead())
      {
         if(XcursorImage *image=XcursorImageCreate(src->w(), src->h()))
         {
            image->xhot =hot_spot.x;
            image->yhot =hot_spot.y;
            image->delay=0;
            VecB4 *bgra=(VecB4*)image->pixels;
            FREPD(y, src->h())
            FREPD(x, src->w())
            {
               Color c=src->color(x, y);
               bgra++->set(c.b, c.g, c.r, c.a);
            }
           _cursor=XcursorImageLoadCursor(XDisplay, image);
            XcursorImageDestroy(image);
         }
         src->unlock();
      }
   }
#elif WEB
   // TODO: Web mouse cursor
#endif

#if LINUX
   return _cursor!=NULL;
#else
   return _cursor!=null;
#endif
}
/******************************************************************************/
#if WINDOWS_OLD
   static const Byte Keys[]=
   {
      VK_LBUTTON ,
      VK_RBUTTON ,
      VK_MBUTTON ,
      VK_XBUTTON1,
      VK_XBUTTON2,
   };
#elif WINDOWS_NEW
   static const Byte Keys[]=
   {
      (Byte)VirtualKey::LeftButton,
      (Byte)VirtualKey::RightButton,
      (Byte)VirtualKey::MiddleButton,
      (Byte)VirtualKey::XButton1,
      (Byte)VirtualKey::XButton2,
   };
#elif MAC
          VecI2 MouseIgnore;
   static Bool  MouseClipOn;
   static RectI MouseClipRect;
#elif LINUX
   static MouseCursor MsCurEmpty;
   static XWindow     Grab;
#endif

static Bool DelayPush;
Mouse       Ms;
MouseCursor MsCur;
/******************************************************************************/
Mouse::Mouse()
{
   REPAO(_button)=0;
  _selecting=_dragging=_first=_detected=_on_client=_freezed=_clip_rect_on=_clip_window=_freeze=_action=_locked=false;
  _visible=_want_cur_hw=true;
  _cur=-1;
  _speed=SPEED; _wheel_time=_start_time=0;
  _pos=_delta=_delta_clp=_delta_relative=_start_pos=_wheel=_wheel_f=0;
  _window_posi=_desktop_posi=_deltai=_hot_spot=_wheel_i=0;
  _clip_rect.zero();
  _did=null;
  _button_name[0]="Mouse1";
  _button_name[1]="Mouse2";
  _button_name[2]="Mouse3";
  _button_name[3]="Mouse4";
  _button_name[4]="Mouse5";
  _button_name[5]="Mouse6";
  _button_name[6]="Mouse7";
  _button_name[7]="Mouse8";
}
void Mouse::del()
{
  _image=null; // clear the pointer because display and images are already deleted, and attempting to use it afterwards will result in a crash
#if WINDOWS_OLD
#if MS_RAW_INPUT
   RAWINPUTDEVICE rid[1];

   rid[0].usUsagePage=0x01;
   rid[0].usUsage    =0x02; // mouse
   rid[0].dwFlags    =RIDEV_REMOVE;
   rid[0].hwndTarget =App.Hwnd();

   RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
#else
   RELEASE(_did);
#endif
#elif LINUX
   if(Grab){XDestroyWindow(XDisplay, Grab); Grab=NULL;}
#endif
}
void Mouse::create()
{
   if(LogInit)LogN("Mouse.create");
#if WINDOWS_OLD
#if MS_RAW_INPUT
   RAWINPUTDEVICE rid[1];

   rid[0].usUsagePage=0x01;
   rid[0].usUsage    =0x02; // mouse
   rid[0].dwFlags    =((MOUSE_MODE==BACKGROUND) ? RIDEV_INPUTSINK : 0);
   rid[0].hwndTarget =App.Hwnd();

   RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));

   Memt<RAWINPUTDEVICELIST> devices;
	UINT num_devices=0; GetRawInputDeviceList(null, &num_devices, SIZE(RAWINPUTDEVICELIST));
again:
   devices.setNum(num_devices);
	Int out=GetRawInputDeviceList(devices.data(), &num_devices, SIZE(RAWINPUTDEVICELIST));
   if(out<0) // error
   {
      if(Int(num_devices)>devices.elms())goto again; // need more memory
      devices.clear();
   }else
   {
      if(out<devices.elms())devices.setNum(out);
      FREPA(devices)
      {
       C RAWINPUTDEVICELIST &device=devices[i];
         if(device.dwType==RIM_TYPEMOUSE){_detected=true; break;}
       /*UInt size=0; if(Int(GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICENAME, null, &size))>=0)
         {
            Memt<Char> name; name.setNum(size+1); Int r=GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICENAME, name.data(), &size);
            if(r>=0 && size==r && r+1==name.elms())
            {
               name.last()='\0'; // in case it's needed
               Str n=name.data();
            }
         }*/
      }
   }
#else
   if(InputDevices.DI) // need to use DirectInput to be able to obtain '_delta_relative'
   if(OK(InputDevices.DI->CreateDevice(GUID_SysMouse, &_did, null)))
   {
      if(OK(_did->SetDataFormat(&c_dfDIMouse2)))
      if(OK(_did->SetCooperativeLevel(App.Hwnd(), DISCL_NONEXCLUSIVE|((MOUSE_MODE==FOREGROUND) ? DISCL_FOREGROUND : DISCL_BACKGROUND))))
      {
         DIPROPDWORD dipdw;
         dipdw.diph.dwSize      =SIZE(DIPROPDWORD );
         dipdw.diph.dwHeaderSize=SIZE(DIPROPHEADER);
         dipdw.diph.dwObj       =0;
         dipdw.diph.dwHow       =DIPH_DEVICE;
         dipdw.dwData           =BUF_BUTTONS;
        _did->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

         if(MOUSE_MODE==BACKGROUND)_did->Acquire(); // in background mode we always want the mouse to be acquired
        _detected=true;
         goto ok;
      }
      RELEASE(_did);
   }
ok:;
#endif
#elif WINDOWS_NEW
   update(); // 'update' to get '_window_posi' needed below, and set initial value of '_desktop_posi'
  _detected=(Windows::Devices::Input::MouseCapabilities().MousePresent>0); // set '_detected' after 'update' because that may set it falsely based on setting initial '_desktop_posi'
  _on_client=Cuts(_window_posi, RectI(0, 0, D.resW(), D.resH())); // set initial value of '_on_client', because 'OnPointerEntered' is not called at start
#elif DESKTOP // assume that desktops always have a mouse
  _detected=true;
#endif
#if LINUX
   // create empty cursor
   Image temp(1, 1, 1, IMAGE_L8A8, IMAGE_SOFT, 1); temp.color(0, 0, TRANSPARENT);
   MsCurEmpty.create(temp);

   // create window for grabbing
   if(XDisplay)
   {
      XSetWindowAttributes win_attr; Zero(win_attr);
      win_attr.event_mask       =ButtonPressMask|ButtonReleaseMask|StructureNotifyMask;
      win_attr.override_redirect=true;
      if(Grab=XCreateWindow(XDisplay, DefaultRootWindow(XDisplay), -1, -1, 1, 1, 0, 0, InputOnly, CopyFromParent, CWEventMask|CWOverrideRedirect, &win_attr)) // set at -1,-1 pos 1,1 size (outside of desktop, with valid size, invalid size would fail to create window)
      if(XMapRaised(XDisplay, Grab)==1)
      {
         XSync(XDisplay, false);
         REP(1024) // attempts to check if window was mapped, otherwise grabbing will fail (last time this was tested, the loop was not needed, but since other 'XCheckTypedWindowEvent' in the engine required it, then keep this one just in case)
         {
            XEvent event; if(XCheckTypedWindowEvent(XDisplay, Grab, MapNotify, &event))break;
            usleep(1);
         }
      }
   }
#endif
}
/******************************************************************************/
CChar8* Mouse::buttonName(Int x)C
{
   return InRange(x, _button_name) ? _button_name[x] : null;
}
/******************************************************************************/
void Mouse::speed(Flt speed)  {       T._speed=speed*SPEED;}
Flt  Mouse::speed(         )C {return T._speed      /SPEED;}
/******************************************************************************/
void Mouse::pos(C Vec2 &pos)
{
   T._pos=pos;
   VecI2 posi=D.screenToWindowPixelI(pos);
#if WINDOWS_OLD
   POINT point={posi.x, posi.y};
   ClientToScreen(App.Hwnd(), &point);
   SetCursorPos(point.x, point.y);
#elif WINDOWS_NEW
   if(App.hwnd())
   {
      Windows::Foundation::Rect bounds=App.Hwnd()->Bounds;
      App.Hwnd()->PointerPosition=Windows::Foundation::Point(bounds.X+PixelsToDips(posi.x), bounds.Y+PixelsToDips(posi.y));
   }
#elif MAC
   RectI   client=WindowRect(true);
   CGPoint point; point.x=posi.x+client.min.x; point.y=posi.y+client.min.y;
   CGWarpMouseCursorPosition(point);
#elif LINUX
   if(XDisplay)XWarpPointer(XDisplay, NULL, App.Hwnd(), 0, 0, 0, 0, posi.x, posi.y);
#endif
}
/******************************************************************************/
static void Clip(RectI *rect) // 'rect' is in window client space, full rect is (0, 0), (D.resW, D.resH)
{
   // !! we can't disable clipping (set rect to null) when rect covers entire screen and in full screen, because computer can have multiple monitors connected, and we may want to prevent going to other monitors !!
#if WINDOWS_OLD
   if(!rect)ClipCursor(null);else
   {
      RECT r;
      r.left=rect->min.x; r.right =rect->max.x;
      r.top =rect->min.y; r.bottom=rect->max.y;

      POINT p={0, 0}; ClientToScreen(App.Hwnd(), &p);
      r.left+=p.x; r.right +=p.x;
      r.top +=p.y; r.bottom+=p.y;

      ClipCursor(&r);
   }
#elif WINDOWS_NEW
 /*if(App.hwnd()) // this only prevents any action taken on the title bar, like move window, minimize/maximize/close, but the cursor can still move outside the window and activate other apps on click
   {
      if(rect)App.Hwnd()->    SetPointerCapture();
      else    App.Hwnd()->ReleasePointerCapture();
   }*/
   if(rect && !Cuts(Ms._window_posi, *rect) && App.Hwnd())
   {
      VecI2 posi=Ms._window_posi; Ms._window_posi&=*rect; posi-=Ms._window_posi;
      Ms._desktop_posi-=posi;
      Ms._deltai      +=posi;
      Windows::Foundation::Rect bounds=App.Hwnd()->Bounds;
      App.Hwnd()->PointerPosition=Windows::Foundation::Point(bounds.X+PixelsToDips(Ms._window_posi.x), bounds.Y+PixelsToDips(Ms._window_posi.y));
   }
#elif MAC
   if(rect)
   {
      MouseClipRect=*rect;
      if(MouseClipRect.max.x>MouseClipRect.min.x)MouseClipRect.max.x--; // decrease width  by 1 pixel
      if(MouseClipRect.max.y>MouseClipRect.min.y)MouseClipRect.max.y--; // decrease height by 1 pixel
   }
   MouseClipOn=(rect!=null);
   CGAssociateMouseAndMouseCursorPosition(!MouseClipOn); // freeze mouse cursor when needed
   // !! warning: clipping using CGAssociateMouseAndMouseCursorPosition + CGWarpMouseCursorPosition will introduce delay in mouse movement, because 'CGAssociateMouseAndMouseCursorPosition' always freezes mouse, and 'CGWarpMouseCursorPosition' moves it manually based on detected input, there used to be "CGSetLocalEventsSuppressionInterval(0)" removing this delay, but it's now deprecated and its replacement doesn't affect clipping anymore
#elif LINUX
   if(XDisplay)
   {
      if(!rect)XUngrabPointer(XDisplay, CurrentTime);else
      {
         Bool custom=(*rect!=RectI(0, 0, D.resW(), D.resH()));
         if(  custom)
         {
            VecI2 pos=0; XWindow child=NULL; XTranslateCoordinates(XDisplay, App.Hwnd(), DefaultRootWindow(XDisplay), 0, 0, &pos.x, &pos.y, &child); // convert to desktop space
               XMoveResizeWindow(XDisplay, Grab, rect->min.x+pos.x, rect->min.y+pos.y, rect->w(), rect->h());
         }else XMoveResizeWindow(XDisplay, Grab, -1, -1, 1, 1); // move outside of desktop area
         XFlush(XDisplay);
         XGrabPointer(XDisplay, App.Hwnd(), false, ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask, GrabModeAsync, GrabModeAsync, custom ? Grab : App.Hwnd(), NULL, CurrentTime);
      }
   }
#elif WEB
   if(rect)emscripten_request_pointerlock(null, true);
   else    emscripten_exit_pointerlock   ();
#endif
}
void Mouse::clipUpdate()
{
   if(App.active() && (_freezed || _clip_rect_on || _clip_window))
   {
      RectI recti, window_rect;
      if(_clip_window)
      {
         window_rect=D.screenToWindowPixelI(D.rect()); // usually this is (0, 0)..(D.resW, D.resH) however for VR it's calculated based on its GuiTexture
      #if MAC // on Mac having mouse on the window border captures cursor for resizing, and mouse clicks do nothing
         if(!D.full())
         {
            const Int border=3;
            window_rect.min.x+=border;
            window_rect.max.x-=border;
            window_rect.max.y-=border;
         }
      #endif
      }
      if(_freezed)
      {
         Vec2  p =pos(); if(_clip_rect_on)p&=_clip_rect;
         VecI2 pi=D.screenToWindowPixelI(p);
         if(_clip_window)pi&=window_rect;
         recti=pi;
      }else
      if(_clip_rect_on)
      {
         recti=D.screenToWindowPixelI(_clip_rect);
         if(_clip_window)
         { // intersect min/max separately to make sure 'recti' isn't invalid, doing just "recti&=window_rect" could result in invalid rectangle and system call could be ignored
            recti.min&=window_rect;
            recti.max&=window_rect;
         }
      }else // window
      {
         recti=window_rect;
      }
   #if !WINDOWS_NEW // can't do this for WINDOWS_NEW because we need 'recti' to be inclusive
      recti.max++;
   #endif
      Clip(&recti);
   }else Clip(null);
}
Mouse& Mouse::clip(C Rect *rect, Int window)
{
   Bool rect_on=(rect!=null),
        win    =((window<0) ? _clip_window : (window!=0)); // <0 - keep old, >=0 - set new
   if(_clip_rect_on!=rect_on || (rect_on && _clip_rect!=*rect) || _clip_window!=win) // if something changes
   {
      if(_clip_rect_on=rect_on)_clip_rect=*rect;
     _clip_window=win;
      clipUpdate();
   }
   return T;
}
Mouse& Mouse::freeze() {_freeze=true; return T;}
/******************************************************************************/
#if WINDOWS_NEW
static void MouseResetVisibility() {Ms.resetVisibility();}
#endif
void Mouse::resetVisibility()
{
#if WINDOWS_NEW
   if(!App.mainThread()){App._callbacks.include(MouseResetVisibility); return;} // for Windows New this can be called only on the main thread
#endif

   Int cur; // -1=system default, 0=hidden, 1=custom hardware
   if(!App.active()
#if !WINDOWS_NEW
   || !_on_client
#endif
   )cur=-1;else // for example: not on the window
   if(hidden())cur=0;else // for example: want to be hidden
   if(MsCur.is())cur=1;else // for example: our own custom hardware cursor
   if(_image)cur=0;else // for example: our own custom non-hardware cursor
      cur=-1; // use default

#if WINDOWS_OLD
   SetCursor((cur<0) ? LoadCursor(null, IDC_ARROW) : (cur==0) ? null : MsCur._cursor);
#elif WINDOWS_NEW
   if(App.hwnd())
   {
      App.Hwnd()->PointerCursor=((cur<0) ? ref new CoreCursor(CoreCursorType::Arrow, 0) : (cur==0) ? null : MsCur._cursor);
     _locked=(App.Hwnd()->PointerCursor==null);
   }
#elif MAC
   if(cur)[((cur>0) ? MsCur._cursor : [NSCursor arrowCursor]) set];
   Bool visible=(cur!=0); if(visible!=CGCursorIsVisible())if(visible)[NSCursor unhide];else [NSCursor hide];
#elif LINUX
   if(XDisplay && App.hwnd())XDefineCursor(XDisplay, App.Hwnd(), (cur<0) ? NULL : (cur==0) ? MsCurEmpty._cursor : MsCur._cursor);
#endif
}
void Mouse::resetCursor()
{
   Bool hardware=(_want_cur_hw && !VR.active()); // can't use hardware cursor in VR mode (it can be enabled there, however that would also require drawing it manually on the gui surface, and that would result in cursor being drawn twice on the window - 1-on gui surface 2-using hardware cursor)

   if(hardware && _image)MsCur.create(*_image, _hot_spot);
   else                  MsCur.del();

   resetVisibility();
}
Mouse& Mouse::cursor(C ImagePtr &image, C VecI2 &hot_spot, Bool hardware, Bool reset)
{
   if(T._image!=image || _want_cur_hw!=hardware || T._hot_spot!=hot_spot || reset)
   {
      T._image      =image;
      T._hot_spot   =hot_spot;
      T._want_cur_hw=hardware;
      resetCursor();
   }
   return T;
}
/******************************************************************************/
Mouse& Mouse::visible(Bool show)
{
   if(_visible!=show)
   {
     _visible=show;
      resetVisibility();
   }
   return T;
}
/******************************************************************************/
void Mouse::eat(Int b)
{
	if(InRange(b, _button))FlagDisable(_button[b], BS_NOT_ON);
}
void Mouse::eatWheel() {_wheel.zero(); _wheel_i.zero();} // don't clear '_wheel_f' because it should continue to be accumulated
void Mouse::eat     () {REPA(_button)eat(i);}
/******************************************************************************/
void Mouse::acquire(Bool on)
{
#if WINDOWS_OLD
#if !MS_RAW_INPUT
   if(MOUSE_MODE==FOREGROUND && _did){if(on)_did->Acquire();else _did->Unacquire();} // we need to change acquire only if we're operating in Foreground mode
#endif
#if SET_HOOK
   if(on)SetHook();else UnHook();
#endif
#endif
   clipUpdate();
   resetVisibility();
}
/******************************************************************************/
void Mouse::clear()
{
   eatWheel();
  _delta_relative.zero();
  _deltai        .zero();
   REPAO(_button)&=~BS_NOT_ON;
   if(DelayPush){DelayPush=false; push(0);}
}
/******************************************************************************/
void Mouse::push(Byte b, Flt double_click_time)
{
   if(InRange(b, _button) && !(_button[b]&BS_ON))
   {
      if(br(b) && !b) // if the button was released in the same frame, then don't push it now, but delay for the next frame, so 'tapped' 'tappedFirst' can be properly detected (this is only for first button because of double clicks)
      {
         DelayPush=true;
      }else
      {
         InputCombo.add(InputButton(INPUT_MOUSE, b));
        _button[b]|=BS_PUSHED|BS_ON;
        _detected  =true;
         if(_cur==b && _first && Time.appTime()<=_start_time+double_click_time+Time.ad())
         {
           _button[b]|=BS_DOUBLE;
           _first=false;
         }else
         {
           _first=true;
         }
        _cur       =b;
        _start_pos =pos();
        _start_time=Time.appTime();
      }
   }
}
void Mouse::release(Byte b)
{
   if(InRange(b, _button) && (_button[b]&BS_ON))
   {
      FlagDisable(_button[b], BS_ON      );
      FlagEnable (_button[b], BS_RELEASED);
      if(!selecting() && life()<=0.25f+Time.ad())_button[b]|=BS_TAPPED;
   }
}
/******************************************************************************/
void Mouse::update()
{
   // clip
   if(_freeze){if(!_freezed){_freezed=true ; clipUpdate();} _freeze=false;}
   else       {if( _freezed){_freezed=false; clipUpdate();}               }
#if WINDOWS_NEW
   if(App.active() && (_freezed || _clip_rect_on || _clip_window))clipUpdate();
#endif

   {
   #if WINDOWS_OLD
   #if !MS_RAW_INPUT
      // button state
      if(_did)
      {
         DIMOUSESTATE2 dims; if(OK(_did->GetDeviceState(SIZE(dims), &dims)))
         {
           _delta_relative.x= dims.lX;
           _delta_relative.y=-dims.lY;
         }else _did->Acquire(); // try to re-acquire if lost access for some reason

         DIDEVICEOBJECTDATA didods[BUF_BUTTONS]; DWORD elms=BUF_BUTTONS; if(OK(_did->GetDeviceData(SIZE(DIDEVICEOBJECTDATA), didods, &elms, 0)))FREP(elms) // process in order
         {
            ASSERT(DIMOFS_BUTTON0+1==DIMOFS_BUTTON1
                && DIMOFS_BUTTON1+1==DIMOFS_BUTTON2
                && DIMOFS_BUTTON2+1==DIMOFS_BUTTON3
                && DIMOFS_BUTTON3+1==DIMOFS_BUTTON4
                && DIMOFS_BUTTON4+1==DIMOFS_BUTTON5
                && DIMOFS_BUTTON5+1==DIMOFS_BUTTON6
                && DIMOFS_BUTTON6+1==DIMOFS_BUTTON7); // check that macros are continuous
          C DIDEVICEOBJECTDATA &didod=didods[i];
            Int b=didod.dwOfs-DIMOFS_BUTTON0; if(b>=5 && InRange(b, 8)) // buttons 0..4 are processed in 'WindowMsg'
            {
               if(didod.dwData&0x80){if(MOUSE_MODE==FOREGROUND || App.active())push(b);}else release(b);
            }
         }
      }
   #endif

      // need to manually check for releases because WM_*BUTTONUP aren't processed when mouse is outside of client window even when app is still active
      REP(Min(Elms(_button), Elms(Keys)))
      {
      #if 1 // this is faster
         if(b(i) && GetKeyState     (Keys[i])>=0)release(i);
      #else
         if(b(i) && GetAsyncKeyState(Keys[i])>=0)release(i);
      #endif
      }

      // position and delta
      POINT p; if(GetCursorPos(&p))
      {
        _deltai.x=p.x-_desktop_posi.x; _desktop_posi.x=p.x;
        _deltai.y=p.y-_desktop_posi.y; _desktop_posi.y=p.y;

         ScreenToClient(App.Hwnd(), &p);
        _window_posi.x=p.x;
        _window_posi.y=p.y;
      }
     _on_client=(InRange(_window_posi.x, D.resW()) && InRange(_window_posi.y, D.resH()) && WindowMouse()==App.hwnd());
      // 'resetVisibility' is always called in 'WM_SETCURSOR'
   #elif WINDOWS_NEW
      if(App.hwnd())
      {
         VecI2 posi(DipsToPixels(App.Hwnd()->PointerPosition.X), DipsToPixels(App.Hwnd()->PointerPosition.Y));
        _deltai      =_desktop_posi-posi; // calc based on '_desktop_posi' because '_window_posi' is relative to window position (so if we move the window based on delta issues could happen)
        _desktop_posi=              posi;
         Windows::Foundation::Rect bounds=App.Hwnd()->Bounds;
        _window_posi.set(posi.x-DipsToPixels(bounds.X),
                         posi.y-DipsToPixels(bounds.Y));

         // need to check buttons manually, because 'OnPointerPressed' will not catch events for other buttons if one button is already pressed
         REP(Min(Elms(_button), Elms(Keys)))
         {
         #if 1 // this is faster
            Int on=((Int)App.Hwnd()->GetKeyState     (VirtualKey(Keys[i])) & (Int)CoreVirtualKeyStates::Down);
         #else
            Int on=((Int)App.Hwnd()->GetAsyncKeyState(VirtualKey(Keys[i])) & (Int)CoreVirtualKeyStates::Down);
         #endif
            if(!on)release(i);else if(_on_client)push(i); // don't push when clicking on title bar or another system window (but allow releases in that case)
         }
      }
      // '_on_client' is managed through 'OnPointerEntered' and 'OnPointerExited' callbacks
   #elif MAC
      // '_on_client' is managed through 'mouseEntered' and 'mouseExited' callbacks
      VecI2 screen=D.screen();
      RectI client=WindowRect(true);

      // desktop position
      VecI2 old_posi=_desktop_posi;

      // apply clipping
      if(MouseClipOn)
      {
         VecI2 cur_pos=_desktop_posi;

         RectI clip=MouseClipRect;
               clip.min+=client.min; MAX(clip.min.x,          2); MAX(clip.min.y,          2); // padd to don't touch edges in order to avoid popping dock
               clip.max+=client.min; MIN(clip.max.x, screen.x-3); MIN(clip.max.y, screen.y-3);

        _desktop_posi.x=Round(_desktop_posi.x+_delta_relative.x);
        _desktop_posi.y=Round(_desktop_posi.y-_delta_relative.y);
        _desktop_posi &=clip;

         MouseIgnore+=_desktop_posi-cur_pos;
         CGPoint p; p.x=_desktop_posi.x; p.y=_desktop_posi.y;
         CGWarpMouseCursorPosition(p);
      }else
      {
         NSPoint p=[NSEvent mouseLocation];
        _desktop_posi.x=         Round(p.x);
        _desktop_posi.y=screen.y-Round(p.y);
      }

     _deltai=_desktop_posi-old_posi;

      // window position
     _window_posi=_desktop_posi-client.min;
   #elif LINUX
      if(XDisplay)
      {
         int x, y;
         unsigned int mask;
         XWindow root, child;
         XQueryPointer(XDisplay, App.Hwnd(), &root, &child, &x, &y, &_window_posi.x, &_window_posi.y, &mask);
        _deltai.x=x-_desktop_posi.x; _desktop_posi.x=x;
        _deltai.y=y-_desktop_posi.y; _desktop_posi.y=y;
         // '_on_client' is managed through 'EnterNotify' and 'LeaveNotify' events
      }
   #else
      // desktop and win pos obtained externally in main loop
     _on_client=(InRange(_window_posi.x, D.resW()) && InRange(_window_posi.y, D.resH()));
      #if WEB
        _on_client|=_locked;
      #endif
   #endif
   }

   Vec2 old=_pos;
  _delta_relative*=_speed;

#if WINDOWS_NEW || WEB
   if(_locked) // for WINDOWS_NEW and WEB when '_locked', the '_window_posi' never changes so we need to manually adjust the '_pos' based on '_delta_relative'
   {
      if(!_freezed)_pos+=_delta_relative*(D.size().max()*0.7f); // make movement speed dependent on the screen size

      // clip
      if(_clip_rect_on)
      {
         Clamp(_pos.x, _clip_rect.min.x, _clip_rect.max.x);
         Clamp(_pos.y, _clip_rect.min.y, _clip_rect.max.y);
      }else
      {
         Clamp(_pos.x, -D.w(), D.w());
         Clamp(_pos.y, -D.h(), D.h());
      }
   }else
#endif
   {
     _pos=D.windowPixelToScreen(_window_posi);
   }

                _delta_clp=_pos-old;
                _delta    =_sv_delta.update(_delta_relative); // yes, mouse delta smoothing is needed, especially for low fps (for example ~40), without this, player camera rotation was not smooth
   if(Time.ad())_vel      =_sv_vel  .update(_delta_clp/Time.ad(), Time.ad()); // use '_delta_clp' to match exact cursor position

   // dragging
   if(b(_cur))
   {
      if(!selecting() && Dist2(pos(), startPos())*Sqr(D.scale())>=SELECT_DIST_2     )_selecting=true; // skip 'D.smallSize' because mouse input is independent on screen size
      if(!dragging () && selecting() &&                   life()>=DragTime+Time.ad())_dragging =true;
   }else
   if(!br(_cur))_dragging=_selecting=false; // disable dragging only if button not on and not released, to allow detection inside the game for "release after dragging"

   // wheel
   if(_wheel.any())
   {
      if(Time.appTime()>_wheel_time+1)_wheel_f=_wheel;else _wheel_f+=_wheel; // if enough time passed without any wheel movement then start from scratch, otherwise keep accumulating
     _wheel_time=Time.appTime(); // remember current app time
      if(Int i=RoundEps(_wheel_f.x, 0.1f)){_wheel_i.x+=i; _wheel_f.x-=i;} // increase integer counters if enough movement occurred, use custom epsilon instead of 0.5f, we need it >0 to make sure that for example 2.999f will get converted to 3
      if(Int i=RoundEps(_wheel_f.y, 0.1f)){_wheel_i.y+=i; _wheel_f.y-=i;} // increase integer counters if enough movement occurred, use custom epsilon instead of 0.5f, we need it >0 to make sure that for example 2.999f will get converted to 3
   }

   // action
  _action   =(pixelDelta().any() || _wheel.any() || bp(0));
  _detected|=_action;
}
/******************************************************************************/
void Mouse::draw()
{
   if(_image && _detected && visible() && !MsCur.is() && _on_client
#if !WINDOWS_OLD
   && App.active()
#endif
   )
   {
      Vec2 pos=D.screenAlignedToPixel(T.pos());
      if(_hot_spot.any())
      {
         Vec2 size=D.pixelToScreenSize(_hot_spot);
         pos.x-=size.x;
         pos.y+=size.y;
      }
     _image->draw(Rect_LU(pos, D.pixelToScreenSize(_image->size())));
   }
}
/******************************************************************************/
}
/******************************************************************************/
