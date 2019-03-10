/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
#if WINDOWS_NEW
using namespace concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Devices;
using namespace Windows::Devices::Input;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;

// must hold global refs, otherwise events will not be called
static Sensors::Accelerometer ^accelerometer;
static Sensors::    Gyrometer ^    gyrometer;
static Sensors:: Magnetometer ^ magnetometer;

static EventRegistrationToken MagnetometerToken, MouseToken;
static TypedEventHandler<Sensors::Magnetometer^, MagnetometerReadingChangedEventArgs^> ^MagnetometerHandler;
/******************************************************************************/
static void Kb_push(KB_KEY key, Int scan_code)
{
   Kb.push(key, scan_code);
   // !! queue characters after push !!
   if(Kb.anyCtrl() && !Kb.anyAlt()) // if Control with no Alt is on, then 'OnCharacterReceived' will not be called, so we must add this char here
   {
      if(key>='A' && key<='Z')Kb.queue(Char(key + (Kb.anyShift() ? 0 : 'a'-'A')), scan_code);else
      if(key>='0' && key<='9')Kb.queue(Char(key                                ), scan_code);
   }
}
struct AppEvent
{
   enum TYPE : Byte
   {
      KEY_DOWN,
      CHAR    ,
      KEY_UP  ,
   };

   TYPE type;
   union
   {
      Char   c  ;
      KB_KEY key;
      Byte   scan_code;
   };

   void keyDown(KB_KEY key, Byte scan_code) {type=KEY_DOWN; T.key=key; T.scan_code=scan_code;}
   void chr    (Char   c  , Byte scan_code) {type=CHAR    ; T.c  =c  ; T.scan_code=scan_code;}
   void keyUp  (KB_KEY key                ) {type=KEY_UP  ; T.key=key;}

   void execute()
   {
      switch(type)
      {
         case KEY_DOWN: Kb_push   (key, scan_code); break;
         case CHAR    : Kb.queue  (c  , scan_code); break;
         case KEY_UP  : Kb.release(key           ); break;
      }
   }

   AppEvent() {} // needed because of union
};
static Memc<AppEvent> Events;
void Application::ExecuteRecordedEvents()
{
   FREPAO(Events).execute(); // run in order
   Events.clear();
}
/******************************************************************************/
Bool Application::Fullscreen() {return Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->IsFullScreenMode;}

void SetMagnetometerRefresh(Flt interval)
{
   if(magnetometer)
   {
      magnetometer->ReadingChanged -= MagnetometerToken;
      if(interval>=0) // enable
      {
         magnetometer->ReportInterval=Max(RoundU(interval*1000), accelerometer->MinimumReportInterval);
         MagnetometerToken=(magnetometer->ReadingChanged += MagnetometerHandler);
      }else
      {
         magnetometer->ReportInterval=0;
      }
   }
}

#define KEY_EVENTS 1 // 1=better (can catch key events that occur when ALT is pressed) 0=(can't catch ALT+keys)

ref struct FrameworkView sealed : IFrameworkView
{
   // IFrameworkView methods
   virtual void Initialize(CoreApplicationView^ applicationView)
   {
      applicationView->Activated  += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &FrameworkView::OnActivated );
      CoreApplication::Suspending += ref new      EventHandler<SuspendingEventArgs^                      >(this, &FrameworkView::OnSuspending);
      CoreApplication::Resuming   += ref new      EventHandler<             Object^                      >(this, &FrameworkView::OnResuming  );
   }
   virtual void SetWindow(CoreWindow^ window) // called before 'Load'
   {
      App._hwnd=reinterpret_cast<Ptr>(window);

      DisplayInformation^ display_info = DisplayInformation::GetForCurrentView();
      ScreenScale=display_info->RawPixelsPerViewPixel;

      window->SizeChanged       += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &FrameworkView::OnWindowSizeChanged);
      window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &FrameworkView::OnVisibilityChanged);
      window->Closed            += ref new TypedEventHandler<CoreWindow^,        CoreWindowEventArgs^>(this, &FrameworkView::OnWindowClosed     );
      window->Activated         += ref new TypedEventHandler<CoreWindow^,   WindowActivatedEventArgs^>(this, &FrameworkView::OnWindowActivated  );

            display_info->                DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &FrameworkView::OnDpiChanged                );
            display_info->        OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &FrameworkView::OnOrientationChanged        );
      DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &FrameworkView::OnDisplayContentsInvalidated);

      MouseToken = (MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &FrameworkView::OnMouseMoved));

      window->PointerMoved        += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerMoved);
      window->PointerEntered      += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerEntered);
      window->PointerExited       += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerExited);
      window->PointerCaptureLost  += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerExited);
      window->PointerPressed      += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerPressed);
      window->PointerReleased     += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerReleased);
      window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &FrameworkView::OnPointerWheelChanged);

   #if KEY_EVENTS
      window->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &FrameworkView::OnAcceleratorKeyActivated);
   #else
      window->KeyDown           += ref new TypedEventHandler<CoreWindow^,               KeyEventArgs^>(this, &FrameworkView::OnKeyDown);
      window->KeyUp             += ref new TypedEventHandler<CoreWindow^,               KeyEventArgs^>(this, &FrameworkView::OnKeyUp);
      window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &FrameworkView::OnCharacterReceived);
   #endif

      Gamepad::GamepadAdded   += ref new EventHandler<Gamepad^>(this, &FrameworkView::OnGamepadAdded);
      Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(this, &FrameworkView::OnGamepadRemoved);

      if(accelerometer=Accelerometer::GetDefault())
      {
         accelerometer->ReportInterval = Max(16, accelerometer->MinimumReportInterval);
         accelerometer->ReadingChanged += ref new TypedEventHandler<Sensors::Accelerometer^, AccelerometerReadingChangedEventArgs^>(this, &FrameworkView::OnAccelerometerChanged);
      }
      if(gyrometer=Gyrometer::GetDefault())
      {
         gyrometer->ReportInterval = Max(16, gyrometer->MinimumReportInterval);
         gyrometer->ReadingChanged += ref new TypedEventHandler<Sensors::Gyrometer^, GyrometerReadingChangedEventArgs^>(this, &FrameworkView::OnGyrometerChanged);
      }
      if(magnetometer=Magnetometer::GetDefault())
      {
         MagnetometerHandler=ref new TypedEventHandler<Sensors::Magnetometer^, MagnetometerReadingChangedEventArgs^>(this, &FrameworkView::OnMagnetometerChanged);
      }
    //SystemNavigationManager::GetForCurrentView()->AppViewBackButtonVisibility = Windows::UI::Core::AppViewBackButtonVisibility::Visible; // display back button on app title bar
      SystemNavigationManager::GetForCurrentView()->BackRequested += ref new EventHandler<BackRequestedEventArgs^>(this, &FrameworkView::OnBackRequested);

      ApplicationView::GetForCurrentView()->SetPreferredMinSize(Size(Max(1, PixelsToDips(1)), Max(1, PixelsToDips(1)))); // using <1 means to use system default min size, so use Max 1 to always set a custom size

      setOrientation(display_info->CurrentOrientation, display_info->NativeOrientation); // !! call this before 'setMode' !!
      setMode();
   }
   virtual void Load(String^ entryPoint) // called after 'SetWindow'
   {
   }
   virtual void Run() // changing full screen mode 'TryEnterFullScreenMode' will work only here
   {
      if(!App._closed) // only if app didn't call 'Exit'
      {
         if(!App.create())App._close=true; // put app create here because only at this stage 'RequestDisplayMode' will work
         App.loop();
      }
      MouseDevice::GetForCurrentView()->MouseMoved -= MouseToken; // !! without this, app will crash on Windows Phone when closing the app !!
   }
   virtual void Uninitialize() // this is called only when app requested to finish (and NOT when user clicked the close box)
   {
      if(App._closed)return; // do nothing if app called 'Exit'
      App.del();
   }

   // custom callbacks
   void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
   {
      CoreWindow::GetForCurrentThread()->Activate();
   }
   void OnSuspending(Object^ sender, SuspendingEventArgs^ args)
   {
      if(App._closed)return; // do nothing if app called 'Exit'
      SuspendingDeferral^ deferral=args->SuspendingOperation->GetDeferral();
      create_task([this,  deferral]()
      {
         if(!App._closed) // only if app didn't call 'Exit', check this again in case 'Exit' was called later
         {
            if(App.save_state)App.save_state(); // call this first as it's most important
          //App.setActive(false); just use 'OnWindowActivated'
            PauseSound();
            App.lowMemory(); // reduce memory usage (this is optional)
         }

         if(D3D)
         {
            IDXGIDevice3 *device; if(OK(D3D->QueryInterface(__uuidof(IDXGIDevice3), (Ptr*)&device)))
            {
               device->Trim   ();
               device->Release();
            }
         }

         deferral->Complete();
      });
   }
   void OnResuming(Object^ sender, Object^ args)
   {
      if(App._closed)return; // do nothing if app called 'Exit'
      ResumeSound();
    //App.setActive(true); just use 'OnWindowActivated'
   }
   void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
   {
      setMode();
   }
   void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
   {
      App._minimized=!args->Visible;
   }
   void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args) {App._close=true;} // this is not called, but do this just in case, as the tutorial does
   void OnWindowActivated(CoreWindow^ sender, WindowActivatedEventArgs^ args)
   {
      if(App._closed)return; // do nothing if app called 'Exit'
      App.setActive(args->WindowActivationState!=CoreWindowActivationState::Deactivated);
   }
   void OnDpiChanged(DisplayInformation^ sender, Object^ args)
   {
      ScreenScale=sender->RawPixelsPerViewPixel;
      setMode();
      D.aspectRatioEx(false); // bug workaround: on Windows when app is started on a secondary monitor, then initially SwapChain output will point to the main monitor output, which will make the aspect calculation incorrect if app starts fullscreen, however it was noticed that 'OnDpiChanged' will be called soon after that, and at this point, output will be correct, so reset aspect ratio here with correct output
   }
   void OnOrientationChanged(DisplayInformation^ sender, Object^ args)
   {
      setOrientation(sender->CurrentOrientation, sender->NativeOrientation);
      setMode();
   }
   void OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
   {
    /*// The D3D Device is no longer valid if the default adapter changed since the device
      // was created or if the device has been removed.

      // First, get the information for the default adapter from when the device was created.
      ComPtr<IDXGIDevice3> dxgiDevice;
      DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

      ComPtr<IDXGIAdapter> deviceAdapter;
      DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

      ComPtr<IDXGIFactory2> deviceFactory;
      DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

      ComPtr<IDXGIAdapter1> previousDefaultAdapter;
      DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

      DXGI_ADAPTER_DESC previousDesc;
      DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));

      // Next, get the information for the current default adapter.

      ComPtr<IDXGIFactory2> currentFactory;
      DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

      ComPtr<IDXGIAdapter1> currentDefaultAdapter;
      DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

      DXGI_ADAPTER_DESC currentDesc;
      DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));

      // If the adapter LUIDs don't match, or if the device reports that it has been removed,
      // a new D3D device must be created.

      if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
      previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
      FAILED(m_d3dDevice->GetDeviceRemovedReason()))
      {
         // Release references to resources related to the old device.
         dxgiDevice = nullptr;
         deviceAdapter = nullptr;
         deviceFactory = nullptr;
         previousDefaultAdapter = nullptr;

         // Create a new device and swap chain.
         HandleDeviceLost();
      }*/
   }
   void OnMouseMoved(MouseDevice^ mouseDevice, MouseEventArgs^ args) // this is not clipped by desktop (if mouse is moved but the cursor remains in the same position, it will still generate move events). WINDOWS_NEW BUG: this will not be called if mouse hovers over title bar or border, but if mouse is moved quickly outside without touching the borders, then it will continue to receive data
   {
      Ms._delta_relative.x+=args->MouseDelta.X;
      Ms._delta_relative.y-=args->MouseDelta.Y;
   }
   void OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
            // this is handled in 'Ms.update' because this is not called when mouse is outside the window
         }break;

         default: // pen, touch
         {
            if(Touch *touch=FindTouchByHandle(CPtr(pointer->PointerId)))
            {
               VecI2 posi(DipsToPixels(pointer->Position.X), DipsToPixels(pointer->Position.Y));
               touch->_deltai+=posi-touch->_posi;
               touch->_posi   =posi;
               touch->_pos    =D.windowPixelToScreen(posi);
            }
         }break;
      }
   }
   void OnPointerEntered(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
            Ms._on_client=true;
          //Ms._deltai=Ms._window_posi-posi; don't calculate delta here to avoid big jumps
          //Ms._window_posi=posi; this is handled in 'Ms.update'
         }break;

         default: // pen, touch
         {
            CPtr   id=CPtr(pointer->PointerId);
            VecI2  posi(DipsToPixels(pointer->Position.X), DipsToPixels(pointer->Position.Y));
            Vec2   pos=D.windowPixelToScreen(posi);
            Touch *touch=FindTouchByHandle(id);
            if(   !touch)touch=&Touches.New().init(posi, pos, id, pointer->PointerDevice->PointerDeviceType==PointerDeviceType::Pen);else
            {
               touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch exited in same/previous frame)
               touch->_posi  =posi;
               touch->_pos   =pos;
            }
         }break;
      }
   }
   void OnPointerExited(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
            Ms._on_client=false;
          //Ms._deltai=Ms._window_posi-posi; this is handled in 'Ms.update'
          //Ms._window_posi=posi; this is handled in 'Ms.update'
         }break;

         default: // pen, touch
         {
            if(Touch *touch=FindTouchByHandle(CPtr(pointer->PointerId)))
            {
               VecI2 posi(DipsToPixels(pointer->Position.X), DipsToPixels(pointer->Position.Y));
               touch->_deltai+=posi-touch->_posi;
               touch->_posi   =posi;
               touch->_pos    =D.windowPixelToScreen(posi);
               touch->_remove =true;
               if(touch->_state&BS_ON) // check for state in case it was manually eaten
               {
                  touch->_state|= BS_RELEASED;
                  touch->_state&=~BS_ON;
               }
            }
         }break;
      }
   }
   void OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
         /* Handled in 'Ms.update' because this won't be called if there's already one button pressed
            if(pointer->Properties->  IsLeftButtonPressed)Ms.push(0);
            if(pointer->Properties-> IsRightButtonPressed)Ms.push(1);
            if(pointer->Properties->IsMiddleButtonPressed)Ms.push(2);
            if(pointer->Properties->    IsXButton1Pressed)Ms.push(3);
            if(pointer->Properties->    IsXButton2Pressed)Ms.push(4);*/
         }break;

         default: // pen, touch
         {
            if(Touch *touch=FindTouchByHandle(CPtr(pointer->PointerId)))
            {
               touch->_state=BS_ON|BS_PUSHED;
               touch->_force=1;
            }
         }break;
      }
   }
   void OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
         /* Handled in 'Ms.update' because this won't be called if there's already one button pressed
            if(!pointer->Properties->  IsLeftButtonPressed)Ms.release(0);
            if(!pointer->Properties-> IsRightButtonPressed)Ms.release(1);
            if(!pointer->Properties->IsMiddleButtonPressed)Ms.release(2);
            if(!pointer->Properties->    IsXButton1Pressed)Ms.release(3);
            if(!pointer->Properties->    IsXButton2Pressed)Ms.release(4);*/
         }break;

         default: // pen, touch
         {
            if(Touch *touch=FindTouchByHandle(CPtr(pointer->PointerId)))
               if(touch->_state&BS_ON) // check for state in case it was manually eaten
            {
               touch->_state|= BS_RELEASED;
               touch->_state&=~BS_ON;
            }
         }break;
      }
   }
   void OnPointerWheelChanged(CoreWindow^ sender, PointerEventArgs^ args)
   {
      PointerPoint ^pointer=args->CurrentPoint;
      switch(pointer->PointerDevice->PointerDeviceType)
      {
         case PointerDeviceType::Mouse:
         {
            if(pointer->Properties->IsHorizontalMouseWheel)Ms._wheel.x+=Flt(pointer->Properties->MouseWheelDelta)/WHEEL_DELTA;
            else                                           Ms._wheel.y+=Flt(pointer->Properties->MouseWheelDelta)/WHEEL_DELTA;
         }break;
      }
   }
   void OnBackRequested(Object^ sender, BackRequestedEventArgs ^args)
   {
      args->Handled=true; // disable app close on back press
      Kb.push   (KB_NAV_BACK, -1);
      Kb.release(KB_NAV_BACK); // release immediately because there's no callback for a release
   }
#if KEY_EVENTS
   void OnAcceleratorKeyActivated(CoreDispatcher^ sender, AcceleratorKeyEventArgs^ args)
   {
      // !! Warning: On International keyboards RightAlt (AltGr) also triggers LeftCtrl, there's no way to workaround this, LeftCtrl is pressed down, and notification of RightAlt can happen even a few frames later !!
      Int scan_code=args->KeyStatus.ScanCode;
   #if DEBUG && 0
      #pragma message("!! Warning: Use this only for debugging !!")
      Str s;
      switch(Windows::UI::Core::CoreAcceleratorKeyEventType type=args->EventType)
      {
         case Windows::UI::Core::CoreAcceleratorKeyEventType::Character          : s=S+"Char "+Char(args->VirtualKey)+' '; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemCharacter    : s=S+"SysChar "+Char(args->VirtualKey)+' '; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::UnicodeCharacter   : s="UniChar"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown            : s="KeyDown"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyUp              : s="KeyUp"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown      : s="SysKeyDown"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp        : s="SysKeyUp"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::DeadCharacter      : s="DeadChar"; break;
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemDeadCharacter: s="SysDeadChar"; break;
      }
      KB_KEY key=KB_KEY(args->VirtualKey);
      LogN(S+"frame:"+Time.frame()+", "+s+" scan_code:"+scan_code+' '+key+'('+Kb.keyName(key)+"), ext:"+args->KeyStatus.IsExtendedKey+", released:"+args->KeyStatus.IsKeyReleased+", wasDown:"+args->KeyStatus.WasKeyDown+", repeat:"+args->KeyStatus.RepeatCount);
   #endif
      switch(args->EventType)
      {
         case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown      :
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown:
         {
            KB_KEY key=KB_KEY(args->VirtualKey);
            switch(key)
            {
               case KB_CTRL : key=(args->KeyStatus.IsExtendedKey ? KB_RCTRL  : KB_LCTRL ); break;
               case KB_SHIFT: key=((scan_code==42)               ? KB_LSHIFT : KB_RSHIFT); break; // 42=KB_LSHIFT, 54=KB_RSHIFT
               case KB_ALT  : key=(args->KeyStatus.IsExtendedKey ? KB_RALT   : KB_LALT  ); break;
            }
            if(App._loop)Events.New().keyDown(key, scan_code);else Kb.push(key, scan_code); // if app is in special loop, then we need to record events, and execute them later
         }break;

         case Windows::UI::Core::CoreAcceleratorKeyEventType::Character      :
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemCharacter:
         {
            Char c=(Char)args->VirtualKey;
            if(App._loop)Events.New().chr(c, scan_code);else Kb.queue(c, scan_code); // if app is in special loop, then we need to record events, and execute them later
         }break;

         case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyUp      :
         case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp:
         {
            KB_KEY key=KB_KEY(args->VirtualKey);
            switch(key)
            {
               case KB_CTRL : key=(args->KeyStatus.IsExtendedKey ? KB_RCTRL  : KB_LCTRL ); break;
               case KB_SHIFT: key=((scan_code==42)               ? KB_LSHIFT : KB_RSHIFT); break; // 42=KB_LSHIFT, 54=KB_RSHIFT
               case KB_ALT  : key=(args->KeyStatus.IsExtendedKey ? KB_RALT   : KB_LALT  ); break;
            }
            if(App._loop)Events.New().keyUp(key);else Kb.release(key); // if app is in special loop, then we need to record events, and execute them later
         }break;
      }
   }
#else
   void OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args) // mouse buttons are not passed here
   {
      Int    scan_code=args->KeyStatus.ScanCode;
      KB_KEY key      =KB_KEY(args->VirtualKey);
      //LogN(S+"OnKeyDown: scan_code:"+scan_code+' '+key+'('+Kb.keyName(key)+"), ext:"+args->KeyStatus.IsExtendedKey+", released:"+args->KeyStatus.IsKeyReleased+", wasDown:"+args->KeyStatus.WasKeyDown+", repeat:"+args->KeyStatus.RepeatCount);
      switch(key)
      {
         case KB_CTRL : key=(args->KeyStatus.IsExtendedKey ? KB_RCTRL  : KB_LCTRL ); break;
         case KB_SHIFT: key=((scan_code==42)               ? KB_LSHIFT : KB_RSHIFT); break; // 42=KB_LSHIFT, 54=KB_RSHIFT
         case KB_ALT  : key=(args->KeyStatus.IsExtendedKey ? KB_RALT   : KB_LALT  ); break;
      }
      if(App._loop)Events.New().keyDown(key, scan_code);else Kb_push(key, scan_code); // if app is in special loop, then we need to record events, and execute them later
   }
   void OnCharacterReceived(CoreWindow^ sender, CharacterReceivedEventArgs^ args)
   {
      Char c=Char(args->KeyCode);
      Int  scan_code=args->KeyStatus.ScanCode;
      if(App._loop)Events.New().chr(c, scan_code);else Kb.queue(c, scan_code); // if app is in special loop, then we need to record events, and execute them later
   }
   void OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args) // mouse buttons are not passed here
   {
      KB_KEY key=KB_KEY(args->VirtualKey);
      //LogN(S+"OnKeyUp: scan_code:"+(args->KeyStatus.ScanCode)+' '+key+'('+Kb.keyName(key)+"), ext:"+args->KeyStatus.IsExtendedKey+", released:"+args->KeyStatus.IsKeyReleased+", wasDown:"+args->KeyStatus.WasKeyDown+", repeat:"+args->KeyStatus.RepeatCount);
      switch(key)
      {
         case KB_CTRL : key=(args->KeyStatus.IsExtendedKey ? KB_RCTRL  : KB_LCTRL ); break;
         case KB_SHIFT: key=((scan_code==42)               ? KB_LSHIFT : KB_RSHIFT); break; // 42=KB_LSHIFT, 54=KB_RSHIFT
         case KB_ALT  : key=(args->KeyStatus.IsExtendedKey ? KB_RALT   : KB_LALT  ); break;
      }
      if(App._loop)Events.New().keyUp(key);else Kb.release(key); // if app is in special loop, then we need to record events, and execute them later
   }
#endif

   void OnGamepadAdded  (Object^ sender, Gamepad ^gamepad) {ListJoypads();}
   void OnGamepadRemoved(Object^ sender, Gamepad ^gamepad) {ListJoypads();}

   void OnAccelerometerChanged(Sensors::Accelerometer^ accelerometer, AccelerometerReadingChangedEventArgs^ args)
   {
      AccelerometerValue.set(args->Reading->AccelerationX, args->Reading->AccelerationY, -args->Reading->AccelerationZ)*=9.80665f;
   }
   void OnGyrometerChanged(Sensors::Gyrometer^ Gyrometer, GyrometerReadingChangedEventArgs^ args)
   {
      GyroscopeValue.set(args->Reading->AngularVelocityX, args->Reading->AngularVelocityY, -args->Reading->AngularVelocityZ);
   }
   void OnMagnetometerChanged(Sensors::Magnetometer^ Magnetometer, MagnetometerReadingChangedEventArgs^ args)
   {
      MagnetometerValue.set(args->Reading->MagneticFieldX, args->Reading->MagneticFieldY, -args->Reading->MagneticFieldZ);
   }
   // custom methods
   void setOrientation(DisplayOrientations orientation, DisplayOrientations native_orientation)
   {
	   DXGI_MODE_ROTATION rotation=DXGI_MODE_ROTATION_IDENTITY;
	   switch(native_orientation) // this can only be Landscape or Portrait even though the DisplayOrientations enum has other values
	   {
	      case DisplayOrientations::Landscape: switch(orientation)
		   {
		    //case DisplayOrientations::Landscape       : rotation=DXGI_MODE_ROTATION_IDENTITY ; break;
		      case DisplayOrientations::Portrait        : rotation=DXGI_MODE_ROTATION_ROTATE270; break;
		      case DisplayOrientations::LandscapeFlipped: rotation=DXGI_MODE_ROTATION_ROTATE180; break;
		      case DisplayOrientations::PortraitFlipped : rotation=DXGI_MODE_ROTATION_ROTATE90 ; break;
		   }break;

	      case DisplayOrientations::Portrait: switch(orientation)
		   {
		      case DisplayOrientations::Landscape       : rotation=DXGI_MODE_ROTATION_ROTATE90 ; break;
		    //case DisplayOrientations::Portrait        : rotation=DXGI_MODE_ROTATION_IDENTITY ; break;
		      case DisplayOrientations::LandscapeFlipped: rotation=DXGI_MODE_ROTATION_ROTATE270; break;
		      case DisplayOrientations::PortraitFlipped : rotation=DXGI_MODE_ROTATION_ROTATE180; break;
		   }break;
	   }

   #if 0 // set based on absolute pose (this will return landscape for laptops)
      switch(orientation)
      {
         default                                   : App._orientation=DIR_UP   ; break; // DisplayOrientations::Portrait
         case DisplayOrientations::PortraitFlipped : App._orientation=DIR_DOWN ; break;
         case DisplayOrientations::Landscape       : App._orientation=DIR_LEFT ; break;
         case DisplayOrientations::LandscapeFlipped: App._orientation=DIR_RIGHT; break;
      }
   #else // set based on relative rotation (this will return portrait for laptops)
      switch(rotation)
      {
         case DXGI_MODE_ROTATION_IDENTITY : App._orientation=DIR_UP   ; break;
         case DXGI_MODE_ROTATION_ROTATE180: App._orientation=DIR_DOWN ; break;
         case DXGI_MODE_ROTATION_ROTATE90 : App._orientation=DIR_LEFT ; break;
         case DXGI_MODE_ROTATION_ROTATE270: App._orientation=DIR_RIGHT; break;
      }
   #endif
   }
   void setMode()
   {
      if(App._closed)return; // do nothing if app called 'Exit'
      VecI2 mode(DipsToPixels(App.Hwnd()->Bounds.Width), DipsToPixels(App.Hwnd()->Bounds.Height));
      D.modeSet(mode.x, mode.y, -1);
   }
};
ref struct FrameworkViewSource sealed : IFrameworkViewSource
{
   virtual IFrameworkView^ CreateView()
   {
      return ref new FrameworkView();
   }
};
void Application::loopUntil(Bool &finished, Bool wait)
{
   if(!finished) // first check if we already finished and don't need to do anything
   {
     _loop=true; // specify that we're inside this special loop, this is needed so if any events occur during callback processing, then we will record them and execute them at a later time, for example this is done so no key pushes are detected at this stage (which can occur in State Update or Draw) but they will be detected once Update and Draw are finished
      for(;;) // start loop
      {
         Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent); // this may call our callbacks, 'ProcessOneAndAllPending'= can't be used because apparently tasks don't count as events, and this will wait until some other events occur, even though we have tasks waiting
         if(finished)break;
         if(wait)Time.wait(1);
      }
     _loop=false;
   }
}
#endif
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if WINDOWS_OLD
INT WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR cmd_line, Int)
{
   if(CChar *cmd=WChar(GetCommandLine())) // need to use 'GetCommandLine' to get wide char, because 'cmd_line' is ANSI only
   {
      // can be:
      // c:\path\file.exe param
      // "c:\path\fi le.exe" param
      // file.exe param
      // "fi le.exe" param
      if(cmd[0]=='"'){if(cmd=TextPos(cmd+1, '"'))cmd+=((cmd[1]==' ') ? 2 : 1);}else // skip '"' and ' '
      if(cmd=TextPos(cmd, ' '))cmd++;
      App._cmd_line=cmd;
   }
      App._hinstance=hinstance;
   if(App.create())App.loop();
      App.del   ();
   return 0;
}
#elif WINDOWS_NEW
[MTAThread] int main(Array<String^> ^args)
{
/* TODO: WINDOWS_NEW setting initial window size and fullscreen mode - check in the future
   changing these didn't make any difference at this launch, only next launch got affected
   if(1)ApplicationView::PreferredLaunchViewSize = Size(300, 300); 
   else ApplicationView::PreferredLaunchViewSize = Size(600, 600);
   ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
 //ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::Auto;
 //ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::FullScreen;*/
   if(args)FREP(args->Length)
   {
      if(i>0)App._cmd_line+='\n'; // separate with new lines, to allow having arguments with spaces in value to be presented as one argument
             App._cmd_line+=args->get(i)->Data();
   }
   Windows::ApplicationModel::Core::CoreApplication::Run(ref new FrameworkViewSource());
   return 0;
}
#endif
/******************************************************************************/
