/******************************************************************************/
#include "stdafx.h"
#if WEB
namespace EE{
/******************************************************************************/
static Bool BackgroundUpdateTimeSet, Firefox;
static UInt BackgroundUpdateTime;
       Flt  ScreenScale=1, MouseWheelScale;
/******************************************************************************/
static void SetMaximized(Int window_width=0, Int window_height=0)
{
   VecI2 win_size(window_width  ? window_width  : JavaScriptRunI("window.outerWidth"),
                  window_height ? window_height : JavaScriptRunI("window.outerHeight"));
   App._maximized=(win_size.x>=JavaScriptRunI("screen.availWidth") && win_size.y>=JavaScriptRunI("screen.availHeight")); // this sets if the browser window is maximized
}
/******************************************************************************/
static EM_BOOL MouseMove(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
   Ms._delta_relative.x+=e->movementX;
   Ms._delta_relative.y-=e->movementY;
   VecI2 posi(RoundPos(e->screenX*ScreenScale),
              RoundPos(e->screenY*ScreenScale));
   Ms._deltai+=posi-Ms._desktop_posi;
   Ms._desktop_posi=posi;
   Ms. _window_posi.set(RoundPos(e->canvasX*ScreenScale), // have to use 'canvas' instead of 'client' because that one is the window client but not the canvas
                        RoundPos(e->canvasY*ScreenScale));
   return 0;
}
static EM_BOOL MouseDown(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
   if(Ms._on_client)
   {
      Int b=e->button; switch(b){case 1: b=2; break; case 2: b=1; break;}
      Ms.push(b);
      return 1;
   }
   return 0;
}
static EM_BOOL MouseUp(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
   Int b=e->button; switch(b){case 1: b=2; break; case 2: b=1; break;}
   Ms.release(b);
   return 1;
}
static EM_BOOL MouseWheel(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
   if(Ms._on_client || Ms.b(0)) // process the wheel only if on client or if there's a mouse button pressed (dragging from the client)
   {
      Ms._wheel.x+=e->deltaX*MouseWheelScale;
      Ms._wheel.y-=e->deltaY*MouseWheelScale;
      return 1; // eat it so it won't be processed by the browser
   }
   return 0;
}
static EM_BOOL MouseLock(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData)
{
   Ms._locked=pointerlockChangeEvent->isActive;
   return 1;
}
/******************************************************************************

   Currently used 'charCode' and 'keyCode' are deprecated in Emscripten, however they fit perfectly in the needs of the engine.
      'keyCode' maps exactly to KB_KEY (which are mapped to Windows VK_ Virtual Keys) that was tested on Chrome on Win and Mac.
   The suggested replacements are 'key' and 'code',
      'key'  should be used only in 'KeyPressed' as WM_CHAR, however it's UTF8 based and would cause overhead to decode.
      'code' should be treated as scan code, it produces different results than 'keyCode' on QWERTY/AZERTY leyouts, however it's UTF8 text-based and would cause overhead to decode.

/******************************************************************************/
static EM_BOOL KeyPressed(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
 //LogN(S+"KeyPressed, key:"+FromUTF8(e->key)+", code:"+FromUTF8(e->code)+", char:"+Char(e->charCode));
   Kb.queue(Char(e->charCode), e->keyCode);
   return 1;
}
static EM_BOOL KeyDown(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
 //LogN(S+"KeyDown, key:"+FromUTF8(e->key)+", code:"+FromUTF8(e->code)+", keyName:"+Kb.keyName(KB_KEY(e->keyCode))+", location:"+(Int)e->location);
   KB_KEY key=KB_KEY(e->keyCode);
   switch(e->location)
   {
      case DOM_KEY_LOCATION_LEFT: switch(key)
      {
         case KB_CTRL : key=KB_LCTRL ; break;
         case KB_SHIFT: key=KB_LSHIFT; break;
         case KB_ALT  : key=KB_LALT  ; break;
       //case KB_WIN  : key=KB_LWIN  ; break;
      }break;

      case DOM_KEY_LOCATION_RIGHT: switch(key)
      {
         case KB_CTRL : key=KB_RCTRL ; break;
         case KB_SHIFT: key=KB_RSHIFT; break;
         case KB_ALT  : key=KB_RALT  ; break;
       //case KB_WIN  : key=KB_RWIN  ; break;
      }break;
   }
   Kb.push(key, key);
   // !! queue characters after push !!
   if(e->ctrlKey) // if control is pressed then we need to eat it to avoid browser taking over Ctrl+A, Ctrl+C, Ctrl+V, etc.
   {
      if(key>='A' && key<='Z')Kb.queue(Char(key + (e->shiftKey ? 0 : 'a'-'A')), key);else
      if(key>='0' && key<='9')Kb.queue(Char(key                              ), key);
      return 1; // eat the key
   }
   return Kb.keyChar(key) ? 0 : 1; // if the key will generate a character then don't eat it (0) so KeyPress can get called, otherwise eat it (1)
}
static EM_BOOL KeyUp(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
 //LogN(S+"KeyUp, key:"+FromUTF8(e->key)+", code:"+FromUTF8(e->code)+", keyName:"+Kb.keyName(KB_KEY(e->keyCode))+", location:"+(Int)e->location);
   KB_KEY key=KB_KEY(e->keyCode);
   switch(e->location)
   {
      case DOM_KEY_LOCATION_LEFT: switch(key)
      {
         case KB_CTRL : key=KB_LCTRL ; break;
         case KB_SHIFT: key=KB_LSHIFT; break;
         case KB_ALT  : key=KB_LALT  ; break;
       //case KB_WIN  : key=KB_LWIN  ; break;
      }break;

      case DOM_KEY_LOCATION_RIGHT: switch(key)
      {
         case KB_CTRL : key=KB_RCTRL ; break;
         case KB_SHIFT: key=KB_RSHIFT; break;
         case KB_ALT  : key=KB_RALT  ; break;
       //case KB_WIN  : key=KB_RWIN  ; break;
      }break;
   }
   Kb.release(key);
   return 1;
}
/******************************************************************************/
static EM_BOOL TouchStart(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
   REP(e->numTouches)
   {
    C EmscriptenTouchPoint &s=e->touches[i]; if(s.isChanged)
      {
         Bool   stylus=false;
         CPtr   pid=CPtr(s.identifier);
         VecI2  posi(RoundPos(s.screenX*ScreenScale),
                     RoundPos(s.screenY*ScreenScale));
         Vec2   pos=D.windowPixelToScreen(Vec2(s.canvasX, s.canvasY)*ScreenScale);
         Touch *touch=FindTouchByHandle(pid);
         if(   !touch)touch=&Touches.New().init(posi, pos, pid, stylus);else
         {
            touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
            touch-> reinit(posi, pos);
         }
         touch->_state=BS_ON|BS_PUSHED;
         touch->_force=1;
      }
   }
   return 1;
}
static EM_BOOL TouchEnd(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
   REP(e->numTouches)
   {
    C EmscriptenTouchPoint &s=e->touches[i];
      if(s.isChanged)
         if(Touch *touch=FindTouchByHandle(CPtr(s.identifier)))
      {
         touch->_posi  .set(RoundPos(s.screenX*ScreenScale),
                            RoundPos(s.screenY*ScreenScale));
         touch->_pos   =D.windowPixelToScreen(Vec2(s.canvasX, s.canvasY)*ScreenScale);
         touch->_remove=true;
         if(touch->_state&BS_ON) // check for state in case it was manually eaten
         {
            touch->_state|= BS_RELEASED;
            touch->_state&=~BS_ON;
         }
      }
   }
   return 1;
}
static EM_BOOL TouchMove(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
   REP(e->numTouches)
   {
    C EmscriptenTouchPoint &s=e->touches[i];
      if(s.isChanged)
         if(Touch *touch=FindTouchByHandle(CPtr(s.identifier)))
      {
         VecI2 posi(RoundPos(s.screenX*ScreenScale),
                    RoundPos(s.screenY*ScreenScale));
         touch->_deltai+=posi-touch->_posi;
         touch->_posi   =posi;
         touch->_pos    =D.windowPixelToScreen(Vec2(s.canvasX, s.canvasY)*ScreenScale);
      }
   }
   return 1;
}
/******************************************************************************/
static EM_BOOL LostFocus(int eventType, const EmscriptenFocusEvent *e, void *userData)
{
   Kb.release(KB_LCTRL); Kb.release(KB_LSHIFT); Kb.release(KB_LALT); Kb.release(KB_LWIN);
   Kb.release(KB_RCTRL); Kb.release(KB_RSHIFT); Kb.release(KB_RALT); Kb.release(KB_RWIN);
   return 0;
}
static EM_BOOL VisibilityChanged(int eventType, const EmscriptenVisibilityChangeEvent *e, void *userData)
{
   if(e->hidden)PauseSound();else ResumeSound();
   App.setActive(!e->hidden);
   BackgroundUpdateTimeSet=false; // if app just got deactivated then make sure that we will recalc the time instead of using an old out-dated value
   return 0;
}
static EM_BOOL OrientationChanged(int eventType, const EmscriptenOrientationChangeEvent *e, void *userData)
{
   /* FIXME test on mobile devices
   switch(e->orientationIndex)
   {
      case EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY   : App._orientation=DIR_UP   ; break;
      case EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY : App._orientation=DIR_DOWN ; break;
      case EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY  : App._orientation=DIR_RIGHT; break;
      case EMSCRIPTEN_ORIENTATION_LANDSCAPE_SECONDARY: App._orientation=DIR_LEFT ; break;
   }*/
   return 0;
}
static EM_BOOL FullChanged(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData)
{
   return 0;
}
static EM_BOOL Resized(int eventType, const EmscriptenUiEvent *e, void *userData) // this gets called when window is resized or zoom is changed
{
   SetMaximized(e->windowOuterWidth, e->windowOuterHeight);
   int width, height; emscripten_get_canvas_element_size(null, &width, &height);
   EmscriptenFullscreenChangeEvent full; emscripten_get_fullscreen_status(&full);
#if 0
   LogN(S+"Resized:"+width+'x'+height+' '+full.isFullscreen+", Zoom:"+D.browserZoom());
   LogN(S+"inner:"+e->windowInnerWidth+'x'+e->windowInnerHeight+' '+e->windowInnerWidth*D.browserZoom()+'x'+e->windowInnerHeight*D.browserZoom());
   LogN(S+"body:"+e->documentBodyClientWidth+'x'+e->documentBodyClientHeight+' '+e->documentBodyClientWidth*D.browserZoom()+'x'+e->documentBodyClientHeight*D.browserZoom());
#endif
   if(!(App.flag&APP_WEB_DISABLE_AUTO_RESIZE) && !full.isFullscreen)
   {
      D._full=false; D._res.zero(); // clear values to make sure D.mode will get executed
      Flt zoom=D.browserZoom();
   #if 0 // can't use this because the values are always Int's
      D.mode(Max(1, RoundPos(e->windowInnerWidth *zoom)),
             Max(1, RoundPos(e->windowInnerHeight*zoom)));
   #else
      JavaScriptRun("var target=Module['canvas'];if(target){target.style.width='100vw';target.style.height='100vh';}"); // resize to max
      Dbl css_w, css_h; emscripten_get_element_css_size(null, &css_w, &css_h); // get actual size
      Int w=Max(1, RoundPos(css_w*zoom)), h=Max(1, RoundPos(css_h*zoom)); // calculate pixels
    //LogN(S+css_w+' '+css_h+' '+css_w*zoom+' '+css_h*zoom+' '+w+' '+h);
      D.mode(w, h);
   #endif
   }else
   if(width!=D.resW() || height!=D.resH() || full.isFullscreen!=D.full())
   {
      // handle bug when leaving fullscreen we still get the same full resolution, this can happen when pressing Esc in fullscreen, or just sometimes when manually disabling fullscreen
      if(D.full() && !full.isFullscreen // if leaving full screen
      && width==D.resW() && height==D.resH()) // but the resolution is the same
      {
         D._full=false; D._res.zero(); // clear values to make sure D.mode will get executed
         D.mode(App._window_size.x, App._window_size.y, false);
      }else D.modeSet(width, height, full.isFullscreen);
   }else // res wasn't changed but zoom may have been
   {
      Dbl css_w, css_h; emscripten_get_element_css_size(null, &css_w, &css_h);
      ScreenScale=D.resW()/css_w; // here can't use zoom, because we don't maintain 1:1 pixel ratio
   }
   return 0;
}
/******************************************************************************/
static void SetCallbacks()
{
   emscripten_set_fullscreenchange_callback   (0, 0, 1, FullChanged);
   emscripten_set_resize_callback             (0, 0, 1, Resized);
   emscripten_set_blur_callback               (0, 0, 1, LostFocus);
   emscripten_set_visibilitychange_callback   (   0, 1, VisibilityChanged);
   emscripten_set_orientationchange_callback  (   0, 1, OrientationChanged);

 //TODO: WEB joypad support
 //emscripten_set_gamepadconnected_callback   (   0, 1, GamepadConnected);
 //emscripten_set_gamepaddisconnected_callback(   0, 1, GamepadDisconnected);
 //emscripten_get_num_gamepads();
 //emscripten_get_gamepad_status(int index, EmscriptenGamepadEvent *gamepadState);

   emscripten_set_mousemove_callback        (0, 0, 1, MouseMove);
   emscripten_set_mousedown_callback        (0, 0, 1, MouseDown);
   emscripten_set_mouseup_callback          (0, 0, 1, MouseUp);
   emscripten_set_wheel_callback            (0, 0, 1, MouseWheel);
   emscripten_set_pointerlockchange_callback(0, 0, 1, MouseLock);

   emscripten_set_keypress_callback         (0, 0, 1, KeyPressed);
   emscripten_set_keydown_callback          (0, 0, 1, KeyDown);
   emscripten_set_keyup_callback            (0, 0, 1, KeyUp);

   emscripten_set_touchstart_callback       (0, 0, 1, TouchStart);
   emscripten_set_touchend_callback         (0, 0, 1, TouchEnd);
   emscripten_set_touchcancel_callback      (0, 0, 1, TouchEnd);
   emscripten_set_touchmove_callback        (0, 0, 1, TouchMove);
}
/******************************************************************************/
static void MainLoop()
{
   if(App._close)
   {
      App.del();
      emscripten_cancel_main_loop();
   }else
   {
      UpdateThreads();

      if(!App.active())
         if(Int wait=((App.flag&APP_WORK_IN_BACKGROUND) ? App.background_wait : -1))
      {
         if(wait>0) // finite wait
         {
            if(BackgroundUpdateTimeSet) // if we already have the end time limit
            {
               wait=BackgroundUpdateTime-Time.curTimeMs(); MAX(wait, 0); // calculate remaining time, convert to Int first before maximizing
            }else
            {
               BackgroundUpdateTime=Time.curTimeMs()+wait; BackgroundUpdateTimeSet=true;
            }
         }
         if(!wait)
         {
            App.update();
            BackgroundUpdateTime=Time.curTimeMs()+App.background_wait; BackgroundUpdateTimeSet=true; // for WEB immediately recalc new time, because browsers already introduce long delays between updates when app is not active
         }
         return;
      }

      App.update();
   }
}
static void PreloadLoop()
{
   UpdateThreads();
   Time.update();
   App._callbacks.update();
   if(!Preload()) // user has finished preloading
   {
      emscripten_cancel_main_loop(); // clear main loop (needs to be called before 'emscripten_set_main_loop')
      if(App.create1())
      {
         Firefox=Contains(D.deviceName(), "Mozilla");
         MouseWheelScale=(Firefox ? 1.0f/3 : 1.0f/100); // FireFox uses a different scale
         ScreenScale=D.browserZoom();
         SetCallbacks();
         emscripten_set_main_loop(MainLoop, 0, false); // don't set 'true' as the exception has already been called in 'emscripten_exit_with_live_runtime' below
      }
   }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
extern "C" EMSCRIPTEN_KEEPALIVE void FileSystemReady()
{
   SetMaximized();
   if(App.create0())
   {
      App.setActive(true);
      emscripten_set_main_loop(PreloadLoop, 0, false); // don't set 'true' as the exception has already been called in 'emscripten_exit_with_live_runtime' below
   }
}
int main() // initialize the FileSystem first
{
   EM_ASM
   ({
      // can't mount IDBFS to root "/" - https://github.com/kripken/emscripten/issues/5632
      FS.mkdir('/data'); FS.mount(IDBFS, {}, '/data');
      FS.syncfs(true, function(err) // load data
      {
         FS.chdir('/data');
         ccall('FileSystemReady', 'v', '', []); // call once the filesystem is ready
      });
   }, FileSystemReady);
   emscripten_exit_with_live_runtime(); // this will prevent the app from exiting and calling destructors, instead we'll just wait until the FS is ready
   return 0;
}
/******************************************************************************/
#endif // WEB
/******************************************************************************/
