/******************************************************************************/
#include "stdafx.h"
#if MAC
#include "../Platforms/Mac/MyApplication.h"
#elif IOS
#include "../Platforms/iOS/iOS.h"
#endif
namespace EE{
/******************************************************************************/
Bool LogInit=false;

  ASSERT(MEMBER_SIZE(ImageTypeInfo,  format   )==SIZE(UInt));
  ASSERT(MEMBER_SIZE(VtxIndBuf    , _prim_type)==SIZE(UInt));
#if WINDOWS_OLD
   #define SM_DIGITIZER      94
   #define SM_MAXIMUMTOUCHES 95
  ASSERT(SIZE(D3DVECTOR  )==SIZE(Vec    )); // shaders
//ASSERT(SIZE(D3DXVECTOR4)==SIZE(Vec4   )); // shaders
//ASSERT(SIZE(D3DXMATRIX )==SIZE(Matrix4)); // shaders

   static Bool ShutCOM=false;
#elif LINUX
   static Atom _NET_WM_ICON;
   static int (*OldErrorHandler)(::Display *d, XErrorEvent *e);
   static int      ErrorHandler (::Display *d, XErrorEvent *e)
   {
      if(e->error_code==BadWindow)return 0;
      return OldErrorHandler ? OldErrorHandler(d, e) : 0;
   }
#endif
/******************************************************************************/
Application App;
/******************************************************************************/
Application::Application()
{
#if 0 // there's only one 'Application' global 'App' and it doesn't need clearing members to zero
   flag=0;
       active_wait=0;
   background_wait=0;
   cipher=null;
   receive_data=null;
   save_state=null;
   paused=null;
   resumed=null;
   drop=null;
   quit=null;
   exit=null;
   low_memory=null;
   notification=null;
  _active=_initialized=_minimized=_maximized=_close=_closed=_del_self_at_exit=_elevated=false;
#if WINDOWS_NEW
  _loop=false;
#endif
  _stay_awake=AWAKE_OFF;
 //_mem_leaks=0; don't set this as it could have been already modified
  _process_id=_parent_process_id=0;
  _hwnd=null;
  _window_pos=_window_size=_window_resized=_desktop_size.zero();
  _desktop_area=_bound=_bound_maximized.zero();
#if WINDOWS_OLD
  _style_window=_style_window_maximized=_style_full=0;
  _hinstance=null;
#endif
#if WINDOWS
  _icon=null;
#endif
#endif
   x          =-1;
   y          = 1;
  _lang       = EN;
  _orientation= DIR_UP;
  _thread_id  = GetThreadId();
  _back_text  = "Running in background";
}
Application& Application::name(C Str &name) {T._name=name; return T;}
/******************************************************************************/
Memx<Notification> Notifications;
#if ANDROID
static void RemoveNotification(Int id)
{
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID removeNotification=jni->GetStaticMethodID(ActivityClass, "removeNotification", "(I)V"))
      jni->CallStaticVoidMethod(ActivityClass, removeNotification, jint(id));
}
static void SetNotification(Int id, C Str &title, C Str &text, Bool dismissable)
{
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID setNotification=jni->GetStaticMethodID(ActivityClass, "setNotification", "(ILjava/lang/String;Ljava/lang/String;Z)V"))
   if(JString   j_title=JString(jni, title))
   if(JString   j_text =JString(jni, text ))
      jni->CallStaticVoidMethod(ActivityClass, setNotification, jint(id), j_title(), j_text(), jboolean(dismissable));
}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_notification(JNIEnv *env, jclass clazz, jint id, jboolean selected)
{
   if(Notifications.absToValidIndex(id)>=0) // if present
   {
      Notification &notification=Notifications.absElm(id);
      if(!selected)notification._visible=false; // if was dismissed then set as hidden (not visible), so calling 'hide' won't do anything and calling 'set' could restore it
      if(auto call=App.notification)call(notification, selected);else Notifications.removeAbs(id, true); // if there's no callback then remove the notification
   }else
   if(selected)RemoveNotification(id); // if not found and selected (not dismissed), then remove manually
}
#endif
Notification::~Notification()
{
   hide();
}
Notification::Notification()
{
   user=null;
  _dismissable=_visible=false;
}
void Notification::hide()
{
   if(_visible)
   {
   #if ANDROID
      Int abs=Notifications.absIndex(this);
      if( abs>=0)RemoveNotification(abs);
   #endif
     _visible=false;
   }
}
void Notification::remove()
{
   Notifications.removeData(this, true);
   // !! do not do anything here because this notification is no longer valid !!
}
void Notification::set(C Str &title, C Str &text, Bool dismissable)
{
   if(!Equal(_title, title, true)
   || !Equal(_text , text , true)
   ||  _dismissable!=dismissable
   || !_visible)
   {
      T._title      =title;
      T._text       =text;
      T._dismissable=dismissable;
   #if ANDROID
      Int valid=Notifications.validIndex(this); if(valid>=0) // if present
      {
         Int abs=Notifications.validToAbsIndex(valid); if(abs>=0)
         {
            SetNotification(abs, title, text, dismissable);
            T._visible=true;
         }
      }
   #endif
   }
}
void HideNotifications()
{
#if ANDROID // on Android, force closing app does not hide notifications, so if there's a non-dismissable notification, it will get stuck
   //REPA(Notifications)if(!Notifications[i].dismissable())Notifications.removeValid(i, true);
   Notifications.clear(); // actually hide all notifications, so if a notification is shown, and app force closed, then the notification has ID belonging to a Notification that no longer exists (because force close removed it), so it can't be found in the 'Notifications' list anymore
#endif
}
Application& Application::backgroundText(C Str &text)
{
   if(!Equal(T._back_text, text, true))
   {
      T._back_text=text;
   #if ANDROID
      if(!App.active())SetNotification(-1, App.name(), text, false); // adjust existing, -1=BACKGROUND_NOTIFICATION_ID from Java
   #endif
   }
   return T;
}
/******************************************************************************/
#if WINDOWS || ANDROID
Bool Application::minimized()C {return _minimized;}
Bool Application::maximized()C {return _maximized;}
#elif LINUX
Bool Application::minimized()C {return WindowMinimized(hwnd());}
Bool Application::maximized()C {return _maximized;} // '_maximized' is obtained in 'ConfigureNotify' system message
#elif MAC
Bool Application::minimized()C {return _minimized;} // '_minimized' is obtained in 'windowDidMiniaturize, windowDidDeminiaturize'
Bool Application::maximized()C {return WindowMaximized(hwnd());}
#elif IOS
Bool Application::minimized()C {return false;}
Bool Application::maximized()C {return true ;}
#elif WEB
Bool Application::minimized()C {return false;}
Bool Application::maximized()C {return _maximized;}
#endif

Bool Application::mainThread()C {return GetThreadId()==_thread_id;}

UInt Application::parentProcessID()C
{
#if WINDOWS_OLD
   if(!_parent_process_id)
   {
      HANDLE snap =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
      if(    snap!=INVALID_HANDLE_VALUE)
      {
         PROCESSENTRY32 proc; proc.dwSize=SIZE(PROCESSENTRY32);
         if(Process32First(snap, &proc))do
         {
            if(proc.th32ProcessID==processID()){_parent_process_id=proc.th32ParentProcessID; break;}
         }while(Process32Next(snap, &proc));
         CloseHandle(snap);
      }
   }
#endif
   return _parent_process_id;
}

DIR_ENUM Application::orientation()C
{
#if IOS
 //switch([[UIDevice currentDevice] orientation]) this is faulty (if the app starts rotated, then this has wrong value that doesn't get updated until device is rotated)
   switch([UIApplication sharedApplication].statusBarOrientation)
   {
      default                                      : return DIR_UP;
      case UIInterfaceOrientationPortraitUpsideDown: return DIR_DOWN;
      case UIInterfaceOrientationLandscapeLeft     : return DIR_RIGHT;
      case UIInterfaceOrientationLandscapeRight    : return DIR_LEFT;
   }
#else
   return _orientation;
#endif
}
/******************************************************************************/
Application& Application::icon(C Image &icon)
{
#if WINDOWS_OLD
   HICON hicon=CreateIcon(icon);
   if(hwnd())
   {
      SendMessage(Hwnd(), WM_SETICON, ICON_BIG  , (LPARAM)hicon);
      SendMessage(Hwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hicon);
   }
   if(_icon)DestroyIcon(_icon); _icon=hicon;
#elif LINUX
   if(XDisplay && hwnd() && _NET_WM_ICON)
   {
      Image temp; C Image *src=(icon.is() ? &icon : null);
      if(src && src->compressed())if(src->copyTry(temp, -1, -1, 1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1))src=&temp;else src=null;
      if(src && src->is() && src->lockRead())
      {
         Memt<long> data; data.setNum(2+src->w()*src->h());
         data[0]=src->w();
         data[1]=src->h();
         FREPD(y, src->h())
         FREPD(x, src->w())
         {
            Color col=src->color(x, y);
            VecB4 c(col.b, col.g, col.r, col.a);
            data[2+x+y*src->w()]=c.u;
         }
         XChangeProperty(XDisplay, Hwnd(), _NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data.data(), data.elms());
         src->unlock();
      }else
      {
         XDeleteProperty(XDisplay, Hwnd(), _NET_WM_ICON);
      }
      XFlush(XDisplay);
     _icon.del(); // delete at end in case it's 'icon'
   }else
   {
      // remember it so it will be set later
      icon.copyTry(_icon, -1, -1, 1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1);
   }
#endif
   return T;
}
/******************************************************************************/
Application& Application::lang(LANG_TYPE lang)
{
   if(T._lang!=lang)
   {
      T._lang=lang;
      Gui.setText();
   }
   return T;
}
/******************************************************************************/
#if MAC
static Bool            AssertionIDValid=false;
static IOPMAssertionID AssertionID;
#endif
Application& Application::stayAwake(AWAKE_MODE mode)
{
   if(_stay_awake!=mode)
   {
     _stay_awake=mode;
   #if DESKTOP
      if(mode==AWAKE_SCREEN) // if we want to keep the screen on
         if(!(active() || FlagTest(App.flag, APP_WORK_IN_BACKGROUND))) // however the app is not focused
            mode=AWAKE_OFF; // then disable staying awake
   #endif
   #if WINDOWS_OLD
      SetThreadExecutionState(ES_CONTINUOUS|((mode==AWAKE_OFF) ? 0 : (mode==AWAKE_SCREEN) ? ES_DISPLAY_REQUIRED : ES_SYSTEM_REQUIRED));
   #elif WINDOWS_NEW
      static Windows::System::Display::DisplayRequest DR; // can't be set as a global var, because crash will happen at its constructor due to system not initialized yet
      if(mode==AWAKE_OFF)DR.RequestRelease();else DR.RequestActive();
   #elif MAC
      if(AssertionIDValid){IOPMAssertionRelease(AssertionID); AssertionIDValid=false;} // release current
      if(mode && IOPMAssertionCreateWithName((mode==AWAKE_SCREEN) ? kIOPMAssertionTypeNoDisplaySleep : kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, CFSTR("Busy"), &AssertionID)==kIOReturnSuccess)AssertionIDValid=true;
   #elif ANDROID
      if(mode==AWAKE_SYSTEM) // if we want to keep the system on
         if(!(active() || FlagTest(App.flag, APP_WORK_IN_BACKGROUND))) // however the app is not focused
            mode=AWAKE_OFF; // then disable staying awake
      JNI jni;
      if(jni && ActivityClass)
      if(JMethodID stayAwake=jni->GetStaticMethodID(ActivityClass, "stayAwake", "(I)V"))
         jni->CallStaticVoidMethod(ActivityClass, stayAwake, jint(mode));
   #elif IOS
      [UIApplication sharedApplication].idleTimerDisabled=(mode!=AWAKE_OFF);
   #elif LINUX
      // TODO: add 'stayAwake' support for Linux
   #endif
   }
   return T;
}
/******************************************************************************/
void Application::setActive(Bool active)
{
   if(T.active()!=active)
   {
      T._active=active;

   #if WINDOWS_OLD
      if(D.full()) // full screen
      {
      #if DX11
         if(!SwapChainDesc.Windowed && SwapChain) // DX10+ true full screen
         {
            SyncLocker locker(D._lock);
            if(active)
            {
               SwapChain->SetFullscreenState(true, null);
               SwapChain->ResizeTarget(&SwapChainDesc.BufferDesc);
            }else
            {
               WindowMinimize(true);
               SwapChain->SetFullscreenState(false, null);
            }
         }
      #endif

         // OpenGL || DX non-exclusive
      #if DX9 || DX11
         if(!D.exclusive())
      #endif
         {
            if(active){SetDisplayMode(    ); WindowReset   (true );}
            else      {WindowMinimize(true); SetDisplayMode(     );}
         }
      }
   #elif MAC
      if(D.full() && !active)WindowHide();
      SetDisplayMode();
   #elif LINUX
      if(D.full() && !active)WindowMinimize();
      SetDisplayMode();
   #endif

      Time        .skipUpdate();
      InputDevices.acquire   (active);

      if(_initialized)
      {
         if(active){if(T.resumed)T.resumed();}
         else      {if(T. paused)T. paused();}
      }
   #if DESKTOP || ANDROID // also on Android in case a new Activity/Window was created, call this after potential 'paused/resumed' in case user modifies APP_WORK_IN_BACKGROUND which affect 'stayAwake'
      if(_stay_awake){AWAKE_MODE temp=_stay_awake; _stay_awake=AWAKE_OFF; stayAwake(temp);} // reset sleeping when app gets de/activated
   #endif
     _initialized=true;
   #if IOS
      if(EAGLView *view=GetUIView())[view setUpdate];
   #endif
   }
}
void Application::close()
{
   if(quit)quit();else _close=true;
}
/******************************************************************************/
Bool Application::testInstance()
{
   if(flag&APP_EXIT_IMMEDIATELY)return false;

   if(flag&(APP_ON_RUN_EXIT|APP_ON_RUN_WAIT))
   {
      Memt<UInt> id; ProcList(id);
      REPA(id)if(processID()!=id[i] && StartsPath(ProcName(id[i]), exe())) // use 'StartsPath' because on Mac 'ProcName' returns the file inside APP folder
      {
         if(flag&APP_ON_RUN_WAIT)ProcWait(id[i]);else
         {
            if(Ptr hwnd=ProcWindow(id[i]))WindowActivate(hwnd);
            return false;
         }
      }
   }
   return true;
}
void Application::deleteSelf()
{
   if(_del_self_at_exit)
   {
   #if WINDOWS_OLD
      Str base=_GetBase(exe());
      #if 0 // this won't work for various reasons, possibly 'createMem' does not support labels and goto (because each command is executed separately), and possibly because 'ConsoleProcess' is a child process which will get closed if the parent (the executable) gets closed, however the child can only delete the file once the parent process is no longer running
         ConsoleProcess cp;
         cp.createMem(S+":Repeat\n"
                       +"del \""     +base+"\" >> NUL\n"       // delete EXE
                       +"if exist \""+base+"\" goto Repeat", // if exists then try again
                      GetPath(exe()));
      #else
         if(HasUnicode(base))
         {
            if(!App.renameSelf(GetPath(exe())+'\\'+FFirst("temp ", "tmp")))return;
            base=_GetBase(exe());
         }
         Str bat=S+exe()+".bat";
         FileText f; if(f.write(bat, ANSI)) // BAT doesn't support wide chars
         {
            f.putText(S+":Repeat\n"
                       +"del \""     +        base+"\" >> NUL\n"      // delete EXE
                       +"if exist \""+        base+"\" goto Repeat\n"
                       +"del \""     +GetBase(bat)+"\" >> NUL");      // delete BAT
            f.del();
            Run(bat, S, true);
         }
      #endif
   #elif MAC
      FDelDirs(exe()); // under Mac OS application is not an "exe" file, but a folder with "app" extension
   #elif LINUX
      FDelFile(exe());
   #endif
   }
}
Bool Application::renameSelf(C Str &dest)
{
   if(FRename(_exe, dest))
   {
     _exe=dest;
      return true;
   }
   return false;
}
void Application::deleteSelfAtExit()
{
  _del_self_at_exit=true;
}
void Application::detectMemLeaks()
{
   if((flag&APP_MEM_LEAKS) && _mem_leaks)
   {
   #if WINDOWS
     _cexit();
      if(Int m=Abs(_mem_leaks))
      {
         ListMemLeaks();
         showError(S+m+" Memory Leak(s)");
      }
     _exit(-1); // manual exit after cleaning with '_cexit'
   #elif 0 && MAC
      LogN("Application Memory Leaks Remaining:");
   #endif
   }
}
#if WINDOWS_NEW
ref struct Exiter sealed
{
   Exiter(Platform::String ^title, Platform::String ^error)
   {
      if(auto dialog=ref new Windows::UI::Popups::MessageDialog(error, title)) // this does not require 'FixNewLine'
      {
         App._closed=true; // disable callback processing
         dialog->Commands->Append(ref new Windows::UI::Popups::UICommand("OK", ref new Windows::UI::Popups::UICommandInvokedHandler(this, &Exiter::onOK)));
         dialog->ShowAsync();
       //try // this can crash if app didn't finish initializing, however using try/catch didn't help
         {
            Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
         }
       //catch(...){ExitNow();}
      }
   }
   void onOK(Windows::UI::Popups::IUICommand^ command)
   {
      Windows::ApplicationModel::Core::CoreApplication::Exit();
   }
};
#elif ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_closedError(JNIEnv *env, jclass clazz) {ExitNow();}
#endif
void Application::showError(CChar *error)
{
   if(Is(error))
   {
      if(D.full() && App.hwnd())
      {
         WindowHide();
      #if DX11 // hiding window on DX10+ is not enough, try disabling Fullscreen
       //ChangeDisplaySettings(null, 0); this didn't help
         if(SwapChain
      #if WINDOWS_OLD
         && !SwapChainDesc.Windowed // if true fullscreen
      #endif
         )
         {
            if(App.threadID()==GetThreadId()) // we can make call to 'SetFullscreenState' only on the main thread, calling it on others made no difference
            {
               SyncLocker locker(D._lock); // we should always be able to lock on the main thread
               if(SwapChain)SwapChain->SetFullscreenState(false, null);
            }else App._close=true; // request the main thread to close the app (but don't call 'App.close' because that would call quit)
         }
      #endif
      }
      CChar *title=MLTC(u"Error", PL,u"Błąd", RU,u"Ошибка", PO,u"Erro", CN,u"错误");

   #if WINDOWS
      OutputDebugString(WChar(error)); OutputDebugStringA("\n"); // first write to console
   #endif
   #if WINDOWS_OLD
    //SetCursor(LoadCursor(null, IDC_ARROW)); // reset cursor first, in case app had it disabled, actually this didn't help
      ClipCursor(null); // disable cursor clipping first
      MessageBox(null, WChar(error), WChar(title), MB_OK|MB_TOPMOST|MB_ICONERROR); // use OS 'MessageBox' instead of EE 'WindowMsgBox' to avoid memory allocation when creating Str objects, because this may be called when out of memory
   #elif WINDOWS_NEW
      Exiter(ref new Platform::String(WChar(title)), ref new Platform::String(WChar(error)));
   #elif LINUX // on Linux additionally display the error in the console in case the OS doesn't support displaying messages boxes
      fputs(UTF8(error), stdout); fputs("\n", stdout); fflush(stdout); // without the flush, messages won't be displayed immediately
      WindowMsgBox(title, error, true);
   #elif ANDROID
      // first write to console, use '__android_log_write' with 'ANDROID_LOG_ERROR' instead of 'Log' which uses 'ANDROID_LOG_INFO'
      Memc<Str> lines; Split(lines, error, '\n'); // android has limit for too long messages
      FREPA(lines){Str8 line=UTF8(lines[i]); if(line.is())__android_log_write(ANDROID_LOG_ERROR, "Esenthel", line.is() ? line : " ");} // '__android_log_write' will crash if text is null or ""

   #if 0 // output the error into root of SD Card so users can send the text file
      Str path=SystemPath(SP_SD_CARD); if(!path.is())path=SystemPath(SP_APP_DATA_PUBLIC); if(!path.is())path=SystemPath(SP_APP_DATA);
      FileText f; if(f.append(path.tailSlash(true)+"Esenthel Error.txt"))f.putText(error);
   #endif
      
      if(ActivityClass)
      {
         if(!minimized()) // if app is not minimized then we can show message box, this works OK even when app is starting (InitPre)
         {
            JNI jni;
            if(jni && ActivityClass)
            if(JMethodID messageBox=jni->GetStaticMethodID(ActivityClass, "messageBox", "(Ljava/lang/String;Ljava/lang/String;Z)V"))
               if(JString ti=JString(jni, title))
               if(JString te=JString(jni, error))
                  jni->CallStaticVoidMethod(ActivityClass, messageBox, ti(), te(), jboolean(true));

            for(; !AndroidApp->destroyRequested && ALooper_pollAll(-1, null, null, null)>=0; )Time.wait(1); // since the message box is only queued, we need to wait until it's actually displayed, need to check for 'destroyRequested' as well, in case the system decided to close the app before 'closedError' got called
         }else // can't display a message box if app is minimized, so display a toast instead
         {
            Str message=S+App.name()+" exited"; if(Is(error))message.line()+=error;
            JNI jni;
            if(jni && ActivityClass)
            if(JMethodID toast=jni->GetStaticMethodID(ActivityClass, "toast", "(Ljava/lang/String;)V"))
               if(JString text=JString(jni, message))
            {
               jni->CallStaticVoidMethod(ActivityClass, toast, text());
               Time.wait(4000); // wait 4 seconds because toast will disappear as soon as we crash
            }
         }
      }
   #elif IOS
      if(NSStringAuto str=error)NSLog(@"%@", str()); // first write to console

      // display message box
	   if(NSString *ns_title=AppleString(title)) // have to use 'AppleString' because it will get copied in the local function below
      {
   	   if(NSString *ns_text=AppleString(error)) // have to use 'AppleString' because it will get copied in the local function below
         {
            dispatch_async(dispatch_get_main_queue(), ^{ // this is needed in case we're calling from a secondary thread
               if(UIAlertController *alert_controller=[UIAlertController alertControllerWithTitle:ns_title message:ns_text preferredStyle:UIAlertControllerStyleAlert])
               {
                  [alert_controller addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {ExitNow();} ]];
                  [[[[UIApplication sharedApplication] keyWindow] rootViewController] presentViewController:alert_controller animated:YES completion:nil];
                //[alert_controller release]; release will crash
               }
            });
            [ns_text release];
         }
         [ns_title release];
      }

     _closed=true; if(EAGLView *view=GetUIView())[view setUpdate]; // disable callback processing and stop updating
      [[NSRunLoop mainRunLoop] run];
   #elif WEB // on Web display the error as both console output and message box
      fputs(UTF8(error), stdout); // first write to console
      WindowMsgBox(title, error, true);
   #else
      WindowMsgBox(title, error, true);
   #endif
   }
}
void Application::lowMemory()
{
   // call this first before releasing caches
   if(low_memory)low_memory();

   // release memory from caches (process in order from parents to base elements)
                DelayRemoveNow();
   Environments.delayRemoveNow();
   Objects     .delayRemoveNow();
   Meshes      .delayRemoveNow();
   PhysBodies  .delayRemoveNow();
   WaterMtrls  .delayRemoveNow();
   Materials   .delayRemoveNow();
   GuiSkins    .delayRemoveNow();
   Panels      .delayRemoveNow();
   PanelImages .delayRemoveNow();
   TextStyles  .delayRemoveNow();
   Fonts       .delayRemoveNow();
   ImageAtlases.delayRemoveNow();
   Images      .delayRemoveNow();
}
/******************************************************************************/
static RectI GetDesktopArea()
{
   RectI recti(0, 0, App.desktopW(), App.desktopH());
#if WINDOWS_OLD
   RECT rect; SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0); recti.set(rect.left, rect.top, rect.right, rect.bottom);
#elif LINUX
   if(XDisplay)
   if(Atom FIND_ATOM(_NET_WORKAREA))
   {
      Atom           type  =NULL;
      int            format=0;
      unsigned long  items =0, bytes_after=0;
      unsigned char *data  =null;
      if(!XGetWindowProperty(XDisplay, DefaultRootWindow(XDisplay), _NET_WORKAREA, 0, 16, 0, XA_CARDINAL, &type, &format, &items, &bytes_after, &data))
         if(long *l=(long*)data)if(items>=4)
      {
         long left  =l[0],
              top   =l[1],
              width =l[2],
              height=l[3];
         recti.set(left, top, left+width, top+height);
      }
      if(data)XFree(data);
   }
#elif MAC
   NSRect rect=[[NSScreen mainScreen] visibleFrame];
   recti.min.x=               Round(rect.origin.x); recti.max.x=recti.min.x+Round(rect.size.width );
   recti.max.y=App.desktopH()-Round(rect.origin.y); recti.min.y=recti.max.y-Round(rect.size.height);
#elif WEB
   // it's not possible to get correct results, because on Chrome: this value is adjusted by "System DPI/Scaling", but not 'D.browserZoom', and does not change when zooming. Because "System DPI/Scaling" is unknown, it can't be calculated.
   recti.min.set(JavaScriptRunI("screen.availLeft" ), JavaScriptRunI("screen.availTop"   ));
   recti.max.set(JavaScriptRunI("screen.availWidth"), JavaScriptRunI("screen.availHeight"))+=recti.min;
#endif
   return recti;
}
static Str GetAppPathName()
{
#if WINDOWS
   wchar_t module[MAX_LONG_PATH]; GetModuleFileName(null, module, Elms(module)); return module;
#elif APPLE
   Str app;
   if(CFBundleRef bundle=CFBundleGetMainBundle())
   {
      if(CFURLRef url=CFBundleCopyBundleURL(bundle))
      {
         Char8 url_path[MAX_UTF_PATH]; CFURLGetFileSystemRepresentation(url, true, (UInt8*)url_path, Elms(url_path));
         app=FromUTF8(url_path);
         CFRelease(url);
      }
      CFRelease(bundle);
   }
   return app;
#elif LINUX
   char path[MAX_UTF_PATH]; path[0]='\0'; ssize_t r=readlink("/proc/self/exe", path, SIZE(path)); if(InRange(r, path))path[r]='\0';
   return FromUTF8(path);
#elif ANDROID
   return AndroidAppPath; // obtained externally
#elif WEB
   return S;
#endif
}
static Bool GetProcessElevation()
{
#if WINDOWS_OLD
   Bool   elevated=false;
   HANDLE token   =null;

   if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
   {
      TOKEN_ELEVATION elevation;
      DWORD           size;
      if(GetTokenInformation(token, TokenElevation, &elevation, SIZE(elevation), &size)) // elevation supported (>= Windows Vista)
      {
         elevated=(elevation.TokenIsElevated!=0);
      }else // elevation not supported (< Windows Vista)
      {
         elevated=true;
      }
   }

   if(token){CloseHandle(token); token=null;}

   return elevated;
#elif WINDOWS_NEW
   return false;
#else
   return true;
#endif
}
/******************************************************************************/
#if WINDOWS_OLD
static BOOL CALLBACK EnumResources(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR user)
{
   if(HRSRC resource=FindResource(hModule, lpName, lpType))
   {
      Paks.addMem(LockResource(LoadResource(null, resource)), SizeofResource(null, resource), (Cipher*)user, false);
      return true;
   }
   Exit(MLTC(u"Can't load resource data from exe", PL,u"Nie można wczytać danych z pliku exe"));
   return false;
}
#endif
void LoadEmbeddedPaks(Cipher *cipher)
{
#if WINDOWS_OLD
   EnumResourceNames(App._hinstance, L"PAK", EnumResources, IntPtr(cipher)); // iterate through all "PAK" resources embedded in .exe file
   SetLastError(0); // clear error 1813 of not found resource type
#elif MAC
   for(FileFind ff(App.exe()+"/Contents/Resources", "pak"); ff(); )Paks.add(ff.pathName(), cipher, false); // iterate all PAK files inside APP resources folder
#elif LINUX
   File f; if(f.readStdTry(App.exe()))for(Long next=f.size(); f.pos(next-2*4); )
   {
      Long skip=f.getUInt(); // !! use Long and not UInt, because of "-skip" below, which would cause incorrect behavior
      UInt end =f.getUInt();
      #define CC4_CHNK CC4('C', 'H', 'N', 'K')
      if(end==CC4_CHNK && f.skip(-skip))
      {
         next=f.pos();
         if(f.getUInt()==CC4_CHNK)
         {
            UInt size=f.getUInt();
            if(  size+4*4==skip) // 2xCHNK + 2xSIZE = 4 TOTAL
            {
               ULong   total_size, applied_offset;
               f.limit(total_size, applied_offset, size);
               switch(f.getUInt())
               {
                  case CC4('P', 'A', 'K', 0):
                  {
                     Paks.addTry(App.exe(), cipher, false, f.posAbs()); // use 'addTry' unlike on other platforms, because this could be not EE data (very unlikely)
                  }break;

                  case CC4('I', 'C', 'O', 'N'):
                  {
                     Image icon; if(icon.ImportTry(f, -1, IMAGE_SOFT, 1))if(icon.is())App.icon(icon);
                  }break;
               }
               f.unlimit(total_size, applied_offset);
               continue; // try next chunk
            }
         }
      }
      break;
   }
#endif
   Paks.rebuild();
}
/******************************************************************************/
Bool Application::create0()
{
#if WINDOWS_OLD
   ShutCOM=OK(CoInitialize(null)); // required by: creating shortctuts - IShellLink, Unicode IME support, ITaskbarList3, Visual Studio Installation detection - SetupConfiguration, SHOpenFolderAndSelectItems
   TouchesSupported=(GetSystemMetrics(SM_MAXIMUMTOUCHES)>0);
#elif WINDOWS_NEW
   TouchesSupported=(Windows::Devices::Input::TouchCapabilities().TouchPresent>0);
#elif MAC
   [MyApplication sharedApplication]; // this allocates 'NSApp' application as our custom class
   [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
   [NSApp finishLaunching];
   [NSApp setDelegate:[[MyAppDelegate alloc] init]]; // don't release delegate, instead it's released in 'MyApplication' dealloc
#elif LINUX
   XInitThreads();
   XDisplay=XOpenDisplay(null);
   OldErrorHandler=XSetErrorHandler(ErrorHandler); // set custom error handler, since I've noticed that occasionally BadWindow errors get generated, so just ignore them
   if(XDisplay)FIND_ATOM(_NET_WM_ICON);
#endif

   T._thread_id   =GetThreadId(); // !! adjust the thread ID here, because on WINDOWS_NEW it will be a different value !!
   T._elevated    =GetProcessElevation();
   T._process_id  =PLATFORM(GetCurrentProcessId(), getpid());
   T._desktop_size=D.screen();
   T._desktop_area=GetDesktopArea(); // !! call after getting '_desktop_size' !!
   T._exe         =GetAppPathName();

  Time.create(); // set first, to start measuring init time
   InitHash  ();
   InitMisc  ();
   InitIO    (); // init IO early in case we want to output logs to files
   InitMesh  ();
   InitSRGB  ();
   InitSocket();
   InitWindow();
   InitStream();
   InitState ();
      Kb.init();
       D.init();
#if WEB
   InitSound (); // on WEB init sound before the 'Preload' and 'InitPre' so we can play some music while waiting
#endif

   return true;
}
Bool Application::create1()
{
   if(LogInit)LogN("InitPre");
   InitPre();
#if LINUX
   if(!XDisplay && !(flag&APP_ALLOW_NO_XDISPLAY))Exit("Can't open XDisplay");
#endif
   if(!testInstance())return false;
       windowCreate();
       InitSound   ();
   if(!InputDevices.create())Exit(MLTC(u"Can't create DirectInput", PL,u"Nie można utworzyć DirectInput"));
   if(!D           .create())return false;
#if ANDROID
   if(_stay_awake){AWAKE_MODE temp=_stay_awake; _stay_awake=AWAKE_OFF; stayAwake(temp);} // on Android we need to apply this after window was created
#endif
   if(LogInit)LogN("Init");
   if(!Init        ())return false;
   return true;
}
Bool Application::create()
{
   return create0() && create1();
}
static void FadeOut()
{
#if DESKTOP
   if(App.flag&APP_FADE_OUT)
   {
      Bool fade_sound=PlayingAnySound(), fade_window=!D.full();
      Byte alpha=WindowGetAlpha();
   #if WINDOWS_OLD
      ANIMATIONINFO ai; ai.cbSize=SIZE(ai); SystemParametersInfo(SPI_GETANIMATION, SIZE(ai), &ai, 0); if(ai.iMinAnimate)fade_window=false; // if Windows has animations enabled, then don't fade manually
   #elif WINDOWS_NEW || LINUX // WindowsNew and Linux don't support 'WindowAlpha'
      fade_window=false;
   #endif
      if(!fade_window)WindowHide();
      if(fade_sound || fade_window)
      {
         const Int step=1;
         const Flt vol=SoundVolume.global(), length=0.2f;
         const Dbl end_time=Time.curTime()+length-step/1000.0f;
         for(;;)
         {
            Flt remaining=end_time-Time.curTime(), frac=Max(0, remaining/length);
            if(fade_sound ){SoundVolume.global(vol*frac); UpdateSound();}
            if(fade_window)WindowAlpha(Round(alpha*frac));
         #if MAC // on Mac we have to update events, otherwise 'WindowAlpha' won't do anything. We have to do it even when not fading window (when exiting from fullscreen mode) because without it, the window will be drawn as a restored window
            for(; NSEvent *event=[NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]; ) // 'distantPast' will not wait for any new events but return those that happened already
               [NSApp sendEvent:event];
         #endif
            if(remaining<=0)break; // check this after setting values, to make sure we will set 0
            UpdateThreads();
            Time.wait(Min(Round(remaining*1000), step));
         }
      }
   }
#endif
}
void Application::del()
{
   { // do brackets to make sure that any temp objects created here are destroyed before 'detectMemLeaks' is called
      if(LogInit)LogN("ShutState");
      ShutState       ();
      if(LogInit)LogN("Shut");
      Shut            ();

     _initialized=false; setActive(false); // set '_initialized' to false to prevent from calling custom 'paused' callback in 'setActive', call 'setActive' because app will no longer be active, this is needed in case 'InputDevices.acquire' is called at a later stage, this is also needed because currently we need to disable disable magnetometer callback for WINDOWS_NEW (otherwise it will crash on Windows Phone when closing app)
      FlushIO          ();
      ShutObj          ();
      Physics.del      ();
      D      .del      ();
      FadeOut          ();
      ShutSound        ();
      ShutEnum         ();
      ShutAnimation    ();
      ShutStream       ();
      ShutSocket       ();
      windowDel        ();
      ShutWindow       ();
      Paks         .del(); // !! delete after deleting sound  !! because sound streaming can still use file data
      InputDevices .del(); // !! delete after deleting window !! because releasing some joypads may take some time and window would be left visible
      HideNotifications();
   #if WINDOWS_OLD
      if(ShutCOM){ShutCOM=false; CoUninitialize();} // required by 'CoInitialize'
   #elif LINUX
      if(XDisplay){XCloseDisplay(XDisplay); XDisplay=null;}
   #elif MAC
      stayAwake(AWAKE_OFF); // on Mac disable staying awake, because we've created 'AssertionID' for it
   #endif
      deleteSelf();
   }
#if 1 // reduce mem leaks logging on Mac
  _exe.del(); _name.del(); _cmd_line.del(); _back_text.del();
#endif
  _closed=true; // !! this needs to be set before 'detectMemLeaks' because that may trigger calling destructors !!
   detectMemLeaks();
}
/******************************************************************************/
void Application::update()
{
   Time        .update();
   InputDevices.update();
   Renderer    .update();
   D       .fadeUpdate();
     _callbacks.update();
   if(!(UpdateState() && DrawState()))_close=true;
   InputDevices.clear();
}
/******************************************************************************/
void Application::coInitialize(UInt dwCoInit)
{
#if WINDOWS_OLD
   ShutWindow(); // close interfaces
   if(ShutCOM)CoUninitialize(); // close COM
   ShutCOM=OK(CoInitializeEx(null, dwCoInit)); // init COM
   InitWindow(); // init interfaces
#endif
}
/******************************************************************************/
void Break()
{
#if WINDOWS
   __debugbreak();
#elif LINUX
     asm("int3");
#elif !(MOBILE && !IOS_SIMULATOR) && !WEB // everything except MobileDevice and Web
   __asm{int 3};
#endif
}
void ExitNow()
{
   if(App.flag&APP_BREAKPOINT_ON_ERROR)Break();
#if WEB
   emscripten_exit_with_live_runtime(); // '_exit' would allow calling global destructors, 'emscripten_exit_with_live_runtime' does not allow it because it gets not caught and the browser stops, alternative is to use 'abort'
#else
  _exit(-1);
#endif
}
void ExitEx(CChar *error)
{
   if(App.exit)App.exit(error);
   FlushIO();
   App.showError(error);
   ExitNow();
}
void Exit(C Str &error)
{
   if(App.flag&APP_CALLSTACK_ON_ERROR)
   {
      Str stack; if(GetCallStack(stack))ExitEx(Str(error).line()+"Current Call Stack:\n"+stack);
   }
   ExitEx(error);
}
void StartEEManually(Ptr dll_module_instance)
{
#if WINDOWS_OLD
      App._hinstance=(dll_module_instance ? (HINSTANCE)dll_module_instance : GetModuleHandle(null));
   if(App.create())App.loop();
      App.del   ();
#else
   Exit("'StartEEManually' is unsupported on this platform");
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if MAC || LINUX
int main(int argc, char *argv[])
{
   for(Int i=1; i<argc; i++) // start from #1, because #0 is always the executable name (if file name has spaces, then they're included in the #0 argv)
   {
      if(i>1)App._cmd_line+='\n'; // separate with new lines, to allow having arguments with spaces in value to be presented as one argument (for example, on Linux running: "./Project test" abc def "123 456" gives 4 arguments: {"./Project test", "abc", "def", "123 456"}
             App._cmd_line+=argv[i];
   }
   if(App.create())App.loop();
      App.del   ();
   return 0;
}
#elif IOS
int main(int argc, char *argv[])
{
   extern Bool DontRemoveThisOriOSAppDelegateClassWontBeLinked  ; DontRemoveThisOriOSAppDelegateClassWontBeLinked  =false;
   extern Bool DontRemoveThisOrMyViewControllerClassWontBeLinked; DontRemoveThisOrMyViewControllerClassWontBeLinked=false;
   extern Bool DontRemoveThisOrEAGLViewClassWontBeLinked        ; DontRemoveThisOrEAGLViewClassWontBeLinked        =false;
   return UIApplicationMain(argc, argv, nil, nil);
}
#endif
/******************************************************************************/
