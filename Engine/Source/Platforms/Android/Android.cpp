/******************************************************************************

   Android platform application life cycle:
      -constructors for global objects are called
      -'android_main' is     called       (with full loop)
      -'android_main' can be called again (with full loop)
      -..
      -if OS decides to kill the app, then probably destructors will be called (but it is unknown)

   When using WORK_IN_BACKGROUND then 'android_main' with full loop will continue to run even when app is closed/minimized.
   In that case when trying to remove the app from recent apps list (for example by "close all" or "swipe" in the apps list)
      then the loop will be stopped and 'android_main' will return, HOWEVER 'BackgroundThread' will continue to run.
   After that, 'android_main' may be called again if the app is started again.

   JNI object life exists as long as we're in native (C++) mode, it will be automatically deleted once we're back to Java.
      Therefore objects that want to be stored globally across multiple threads, must use 'NewGlobalRef' and later 'DeleteGlobalRef'.
      'jfieldID' and 'jmethodID' should NOT be passed to 'NewGlobalRef', 'DeleteGlobalRef' or 'DeleteLocalRef'.

   https://developer.android.com/training/articles/perf-jni.html

   JNI Sample Usage documentation link:
      http://android.wooyd.org/JNIExample/

   'KeyCharacterMapGet' does not convert meta to accents, what we would need are accented characters to be used with 'getDeadChar'.
      However NDK input does not provide COMBINING_ACCENT and COMBINING_ACCENT_MASK for key codes.
      I still need to use 'KeyCharacterMap' instead of converting buttons to characters, because on Swiftkey Android 2.3 pressing '(' results in 'k' character, and 'KeyCharacterMap' fixes that.

   Type Signature Java Type 
   Z              boolean 
   B              byte 
   C              char 
   S              short 
   I              int 
   J              long 
   F              float 
   D              double 
   L*;            "*" class
   [*             *[] array 
   (params)ret    method

   For example, the Java method:     long f(int n, String s, int[] arr); 
   has the following type signature: (ILjava/lang/String;[I)J
/******************************************************************************/
#include "stdafx.h"
#if ANDROID
ASSERT(SIZE(Long)>=SIZE(Ptr)); // some pointers are processed using Long's on Java
namespace EE{
/******************************************************************************/
#ifndef AMETA_CAPS_LOCK_ON
   #define AMETA_CAPS_LOCK_ON 0x100000
#endif
#ifndef AINPUT_SOURCE_STYLUS
   #define AINPUT_SOURCE_STYLUS (0x00004000|AINPUT_SOURCE_CLASS_POINTER)
#endif

#ifndef AMOTION_EVENT_ACTION_HOVER_MOVE
   #define AMOTION_EVENT_ACTION_HOVER_MOVE 7
#endif
#ifndef AMOTION_EVENT_ACTION_SCROLL
   #define AMOTION_EVENT_ACTION_SCROLL 8
#endif
#ifndef AMOTION_EVENT_ACTION_HOVER_ENTER
   #define AMOTION_EVENT_ACTION_HOVER_ENTER 9
#endif
#ifndef AMOTION_EVENT_ACTION_HOVER_EXIT
   #define AMOTION_EVENT_ACTION_HOVER_EXIT 10
#endif

#ifndef AMOTION_EVENT_AXIS_VSCROLL
   #define AMOTION_EVENT_AXIS_VSCROLL 9
#endif
#ifndef AMOTION_EVENT_AXIS_HSCROLL
   #define AMOTION_EVENT_AXIS_HSCROLL 10
#endif

#ifndef AINPUT_SOURCE_JOYSTICK
   #define AINPUT_SOURCE_JOYSTICK 0x01000010
#endif

#ifndef AMOTION_EVENT_AXIS_X
   #define AMOTION_EVENT_AXIS_X 0
#endif
#ifndef AMOTION_EVENT_AXIS_Y
   #define AMOTION_EVENT_AXIS_Y 1
#endif
#ifndef AMOTION_EVENT_AXIS_RX
   #define AMOTION_EVENT_AXIS_RX 12
#endif
#ifndef AMOTION_EVENT_AXIS_RY
   #define AMOTION_EVENT_AXIS_RY 13
#endif
#ifndef AMOTION_EVENT_AXIS_HAT_X
   #define AMOTION_EVENT_AXIS_HAT_X 15
#endif
#ifndef AMOTION_EVENT_AXIS_HAT_Y
   #define AMOTION_EVENT_AXIS_HAT_Y 16
#endif

#ifndef HISTORY_CURRENT
   #define HISTORY_CURRENT (-0x80000000)
#endif

#ifndef AMETA_CTRL_ON
   #define AMETA_CTRL_ON 0x1000
#endif
#ifndef AMETA_CTRL_LEFT_ON
   #define AMETA_CTRL_LEFT_ON 0x2000
#endif
#ifndef AMETA_CTRL_RIGHT_ON
   #define AMETA_CTRL_RIGHT_ON 0x4000
#endif

#if DEBUG
   #define LOG(x) LogN(x)
#else
   #define LOG(x) if(LogInit)LogN(x)
#endif
   #define LOG2(x)
/******************************************************************************/
enum ROTATION
{
   ROTATION_0  ,
   ROTATION_90 ,
   ROTATION_180,
   ROTATION_270,
};
/******************************************************************************/
JNI Jni(null);

static JavaVM        *JVM;
       jobject        Activity;
       JClass         ActivityClass, ClipboardManagerClass, InputDeviceClass, KeyCharacterMapClass;
       JObject        DefaultDisplay, ClipboardManager, LocationManager, KeyCharacterMap,
                      GPS_PROVIDER, NETWORK_PROVIDER, EsenthelLocationListener[2];
       JMethodID      getRotation,
                      InputDeviceGetDevice, InputDeviceGetName,
                      KeyCharacterMapLoad, KeyCharacterMapGet,
                      getLastKnownLocation, getLatitude, getLongitude, getAltitude, getAccuracy, getSpeed, getTime, requestLocationUpdates, removeUpdates;
       Int            AndroidSDK;
       AAssetManager *AssetManager;
       android_app   *AndroidApp;
       Str8           AndroidPackageName;
       Str            AndroidAppPath, AndroidAppDataPath, AndroidAppDataPublicPath, AndroidPublicPath, AndroidSDCardPath;
static KB_KEY         KeyMap[256];
static Byte           ShiftMap[3][128], JoyMap[256];
static Bool           Initialized, // !! This may be set to true when app is restarted (without previous crashing) !!
                      KeyboardLoaded, PossibleTap;
static Int            KeyboardDeviceID;
static Dbl            PossibleTapTime;
static VecI2          LastMousePos;
static Thread         BackgroundThread;
#if __ANDROID_API__<14
static JClass         MotionEventClass;
static JMethodID      getButtonState, getRawAxisValue;
#endif
/******************************************************************************/
static Bool BackgroundFunc(Thread &thread)
{
   LOG("BackgroundFunc");
   ThreadMayUseGPUData(); // here we call Update functions which normally assume to be called on the main thread, so make sure GPU can be used just like on the main thread
   Jni.attach();
   LOG("BackgroundFunc Loop");
   for(;;) // do a manual loop to avoid the overhead of 'ThreadMayUseGPUData' and 'ThreadFunc' (which is needed only if we may want to pause the thread, which we won't)
   {
      if(App.background_wait)thread._resume.wait(App.background_wait); // wait requested time, Warning: here we reuse its 'SyncEvent' because it's used only for pausing/resuming
      if(App._close) // first check if we want to close manually
      {
         App.del(); // manually call shut down
         ExitNow(); // do complete reset (including state of global variables) by killing the process
      }
      if(thread.wantStop())break; // check after waiting and before updating
      App.update();
   }
   Jni.del();
   LOG("BackgroundFunc End");
   return false;
}
/******************************************************************************/
void JNI::clear()
{
   _=null;
   attached=false;
}
void JNI::del()
{
   if(_ && attached)JVM->DetachCurrentThread();
   clear();
}
void JNI::attach()
{
   if(!_)
   {
      if(!JVM)Exit("JVM is null");
      JVM->GetEnv((Ptr*)&_, JNI_VERSION_1_6);
      if(!_)
      {
         JVM->AttachCurrentThread(&_, null);
         if(_)attached=true;
      }
      if(!_)Exit("Can't attach thread to Java Virtual Machine");
   }
}
JObject& JObject::clear     () {_=null; _global=false; return T;}
JObject& JObject::del       () {if(_ && _jni){if(_global)_jni->DeleteGlobalRef(_);else _jni->DeleteLocalRef(_);} _=null; _global=false; return T;}
JObject& JObject::makeGlobal() {if(!_global && _jni && _){jobject j=_jni->NewGlobalRef(_); del(); _=j; _global=true;} return T;}
JObject& JObject::operator= (jobject j) {del(); _=j; _global=false; return T;}

JClass& JClass::operator=(CChar8 *name) {del(); _=_jni->FindClass     (name); _global=false; return T;}
JClass& JClass::operator=(jobject obj ) {del(); _=_jni->GetObjectClass(obj ); _global=false; return T;}
JClass& JClass::operator=(jclass  cls ) {del(); _=                     cls  ; _global=false; return T;}

JClass::JClass(          CChar8 *name) : JObject(     name ? Jni->FindClass     (name) : null) {}
JClass::JClass(          jobject obj ) : JObject(            Jni->GetObjectClass(obj )       ) {}
JClass::JClass(          jclass  cls ) : JObject(                                cls         ) {}
JClass::JClass(JNI &jni, CChar8 *name) : JObject(jni, name ? jni->FindClass     (name) : null) {}
JClass::JClass(JNI &jni, jobject obj ) : JObject(jni,        jni->GetObjectClass(obj )       ) {}
JClass::JClass(JNI &jni, jclass  cls ) : JObject(jni,                            cls         ) {}

JString& JString::operator=(CChar8 *t) {T=_jni->NewStringUTF(        t              ); return T;}
JString& JString::operator=(CChar  *t) {T=_jni->NewString   ((jchar*)t  ,  Length(t)); return T;}
JString& JString::operator=(C Str8 &s) {T=_jni->NewStringUTF(        s()            ); return T;}
JString& JString::operator=(C Str  &s) {T=_jni->NewString   ((jchar*)s(), s.length()); return T;}

Int     JObjectArray::elms      (     )C {return _jni->GetArrayLength       (T   );}
jobject JObjectArray::operator[](Int i)C {return _jni->GetObjectArrayElement(T, i);}

     JObjectArray::JObjectArray(JNI &jni, int elms) : JObject(jni, jni->NewObjectArray(elms, jni->FindClass("java/lang/String"), NULL)) {}
void JObjectArray::set(Int i, CChar8 *t) {if(_ && _jni)_jni->SetObjectArrayElement(T, i, _jni->NewStringUTF(t));}
/******************************************************************************/
static Str JavaInputDeviceName(Int i)
{
   if(Jni && InputDeviceGetDevice && InputDeviceGetName)
      if(JObject device=Jni->CallStaticObjectMethod(InputDeviceClass, InputDeviceGetDevice, jint(i)))
         if(JString name=Jni->CallObjectMethod(device, InputDeviceGetName))
            return name.str();
   return S;
}
/******************************************************************************/
static void UpdateOrientation()
{
 //switch(AConfiguration_getOrientation(AndroidApp->config)) this appears to have weird enum values: ACONFIGURATION_ORIENTATION_ANY, ACONFIGURATION_ORIENTATION_PORT, ACONFIGURATION_ORIENTATION_LAND, ACONFIGURATION_ORIENTATION_SQUARE
   if(Jni && getRotation)switch(Jni->CallIntMethod(DefaultDisplay, getRotation))
   {
      case ROTATION_0  : App._orientation=DIR_UP   ; break;
      case ROTATION_90 : App._orientation=DIR_LEFT ; break;
      case ROTATION_180: App._orientation=DIR_DOWN ; break;
      case ROTATION_270: App._orientation=DIR_RIGHT; break;
   }
}
static void UpdateSize(ANativeWindow &window)
{
   VecI2 res(ANativeWindow_getWidth (&window),
             ANativeWindow_getHeight(&window));
   if(D.res()!=res && res.x>0 && res.y>0)
   {
      LOG(S+"Resize: "+res.x+", "+res.y);
      Renderer._main.forceInfo(res.x, res.y, 1, Renderer._main.type(), Renderer._main.mode()); // '_main_ds' will be set in 'rtCreate'
      D.modeSet(res.x, res.y);
   }
}
/******************************************************************************/
static Char AdjustByShift(Char c, Bool shift, Bool caps)
{
   if(shift || caps)
   {
      if(InRange(U16(c), ShiftMap[0]))return ShiftMap[(shift | (caps<<1))-1][U16(c)];
      if(shift!=caps)return CaseUp(c);
   }
   return c;
}
static int32_t InputCallback(android_app *app, AInputEvent *event)
{
   LOG2("InputCallback");
   Int device=AInputEvent_getDeviceId(event);
   switch(    AInputEvent_getType    (event))
   {
      case AINPUT_EVENT_TYPE_MOTION:
      {
         Int  source      =(AInputEvent_getSource(event) & ~AINPUT_SOURCE_CLASS_POINTER), // disable 'AINPUT_SOURCE_CLASS_POINTER' because all have it (including AINPUT_SOURCE_MOUSE and AINPUT_SOURCE_STYLUS)
              action      =AMotionEvent_getAction(event),
              action_type = (action&AMOTION_EVENT_ACTION_MASK),
              action_index=((action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
         Bool stylus      =FlagTest(source, AINPUT_SOURCE_STYLUS);
         if(source&AINPUT_SOURCE_JOYSTICK)
         {
            if(action_type==AMOTION_EVENT_ACTION_MOVE
         #if __ANDROID_API__<14
            && getRawAxisValue
         #endif
            )
            {
               Joypad &joy=Joypads(action_index); if(!joy._name.is()){joy._name=JavaInputDeviceName(device); if(!joy._name.is())joy._name="JoyPad";}
            #if __ANDROID_API__>=14
               joy.dir     .set(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, action_index),
                               -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, action_index));
               joy.dir_a[0].set(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X    , action_index),
                               -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y    , action_index));
               joy.dir_a[1].set(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RX   , action_index),
                               -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RY   , action_index));
            #elif X64
               joy.dir     .set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_HAT_X), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_HAT_Y), jint(action_index), jint(HISTORY_CURRENT)));
               joy.dir_a[0].set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_X    ), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_Y    ), jint(action_index), jint(HISTORY_CURRENT)));
               joy.dir_a[1].set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_RX   ), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_RY   ), jint(action_index), jint(HISTORY_CURRENT)));
            #else
               joy.dir     .set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_HAT_X), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_HAT_Y), jint(action_index), jint(HISTORY_CURRENT)));
               joy.dir_a[0].set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_X    ), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_Y    ), jint(action_index), jint(HISTORY_CURRENT)));
               joy.dir_a[1].set(Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_RX   ), jint(action_index), jint(HISTORY_CURRENT)),
                               -Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_RY   ), jint(action_index), jint(HISTORY_CURRENT)));
            #endif
            }
         }else
         if((source&AINPUT_SOURCE_MOUSE) && !stylus)  // check for stylus because on "Samsung Galaxy Note 2" stylus input generates both "AINPUT_SOURCE_STYLUS|AINPUT_SOURCE_MOUSE" at the same time
         {
            if(action_type==AMOTION_EVENT_ACTION_SCROLL
         #if __ANDROID_API__<14
            && getRawAxisValue
         #endif
            )
            {
            #if __ANDROID_API__>=14
               Ms._wheel.x+=AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, action_index);
               Ms._wheel.y+=AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, action_index);
            #elif X64
               Ms._wheel.x+=Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_HSCROLL), jint(action_index), jint(HISTORY_CURRENT));
               Ms._wheel.y+=Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jlong(event), jint(AMOTION_EVENT_AXIS_VSCROLL), jint(action_index), jint(HISTORY_CURRENT));
            #else
               Ms._wheel.x+=Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_HSCROLL), jint(action_index), jint(HISTORY_CURRENT));
               Ms._wheel.y+=Jni->CallStaticFloatMethod(MotionEventClass, getRawAxisValue, jint (event), jint(AMOTION_EVENT_AXIS_VSCROLL), jint(action_index), jint(HISTORY_CURRENT));
            #endif
            }

            // get button states
            Int button_state=0;
            if( // if we have access to button states
            #if __ANDROID_API__>=14
               true
            #else
               getButtonState
            #endif
            )
            {
            #if __ANDROID_API__>=14
               button_state=AMotionEvent_getButtonState(event);
            #elif X64
               button_state=Jni->CallStaticIntMethod(MotionEventClass, getButtonState, jlong(event));
            #else
               button_state=Jni->CallStaticIntMethod(MotionEventClass, getButtonState, jint (event));
            #endif

               if(action_type==AMOTION_EVENT_ACTION_DOWN && !button_state){PossibleTap=true; PossibleTapTime=Time.appTime();}else // 'getButtonState' does not detect tapping on the touchpad, so we need to detect it according to 'AMOTION_EVENT_ACTION_DOWN', also proceed only if no buttons are pressed in case this event is triggered by secondary mouse button
               if(PossibleTap && (button_state || (LastMousePos-Ms.desktopPos()).abs().max()>=6))PossibleTap=false; // if we've pressed a button or moved away too much then it's definitely not a tap
               if(action_type==AMOTION_EVENT_ACTION_UP   &&  PossibleTap ){PossibleTap=false; if(Time.appTime()<=PossibleTapTime+0.33f+Time.ad())Ms.push(0, 0.33f);} // this is a tap so push the button and it will be released line below because 'button_state' is 0, use 0.33f time limit because on Asus Transformer Prime tapping can result in times as long as 0.318s
               REPA(Ms._button)if(FlagTest(button_state, 1<<i)!=Ms.b(i))if(Ms.b(i))Ms.release(i);else Ms.push(i);
            }else // if not then rely on actions
            switch(action_type)
            {
               case AMOTION_EVENT_ACTION_DOWN: Ms.push(0); break;

               case AMOTION_EVENT_ACTION_UP    :
               case AMOTION_EVENT_ACTION_CANCEL: Ms.release(0); break;

               case AMOTION_EVENT_ACTION_POINTER_DOWN: Ms.push(0); break;

               case AMOTION_EVENT_ACTION_POINTER_UP: Ms.release(0); break;
            }

            // get scrolling and cursor position
            if(action_type!=AMOTION_EVENT_ACTION_UP // this can happen on release of TouchPad scroll, where the position is still at the dragged position
            && AMotionEvent_getPointerCount(event)>=1)
            {
               VecI2 pos(Round(AMotionEvent_getRawX(event, 0)),
                         Round(AMotionEvent_getRawY(event, 0)));
               if(action_type==AMOTION_EVENT_ACTION_MOVE // if we're dragging

               // we have access to button states
            #if __ANDROID_API__<14
               && getButtonState
            #endif

               && !button_state) // none of them are pressed
               { // this is TouchPad scroll (with 2 fingers on the TouchPad)
                  const Flt mul=16.0f/D.screen().min(); // trigger 16 full mouse wheel events when moving mouse from one side to another (this value was set to match results on a Windows laptop, use 'min' or 'max' to get the same results even when device got rotated and that would cause resolution to change)
                  Ms._wheel.x-=(pos.x-LastMousePos.x)*mul;
                  Ms._wheel.y+=(pos.y-LastMousePos.y)*mul;
               }else
               {
                  Ms._desktop_posi=pos;
                  Ms. _window_posi.x=Round(AMotionEvent_getX(event, 0));
                  Ms. _window_posi.y=Round(AMotionEvent_getY(event, 0));
               }
               LastMousePos=pos;
            }
         }else
         {
            switch(action_type)
            {
               case AMOTION_EVENT_ACTION_HOVER_ENTER: // touch is not pressed but appeared
               case AMOTION_EVENT_ACTION_DOWN       : // touch was    pressed (1st pointer)
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr   pid  =(CPtr)AMotionEvent_getPointerId(event, i);
                     Vec2   pixel(AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)),
                            pos  =D.windowPixelToScreen(pixel);
                     VecI2  posi =Round(pixel);
                     Touch *touch=FindTouchByHandle(pid);
                     if(   !touch)touch=&Touches.New().init(posi, pos, pid, stylus);else
                     {
                        touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
                        if(action_type!=AMOTION_EVENT_ACTION_HOVER_ENTER)touch->reinit(posi, pos); // re-initialize for push (don't do this for hover because it can be called the same frame that release is called, and for release we want to keep the original values)
                     }
                     if(action_type!=AMOTION_EVENT_ACTION_HOVER_ENTER){touch->_state=BS_ON|BS_PUSHED; touch->_force=1;}
                  }
               }break;

               case AMOTION_EVENT_ACTION_POINTER_DOWN: // touch was pressed (secondary pointer)
               {
                  CPtr   pid  =(CPtr)AMotionEvent_getPointerId(event, action_index);
                  Vec2   pixel(AMotionEvent_getX(event, action_index), AMotionEvent_getY(event, action_index)),
                         pos  =D.windowPixelToScreen(pixel);
                  VecI2  posi =Round(pixel);
                  Touch *touch=FindTouchByHandle(pid);
                  if(   !touch)touch=&Touches.New().init(posi, pos, pid, stylus);else
                  {
                     // re-initialize for push
                     touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
                     touch-> reinit(posi, pos);
                  }
                  touch->_state=BS_ON|BS_PUSHED;
                  touch->_force=1;
               }break;

               case AMOTION_EVENT_ACTION_MOVE:       // touch is     pressed and was moved
               case AMOTION_EVENT_ACTION_HOVER_MOVE: // touch is not pressed and was moved
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr   pid  =(CPtr)AMotionEvent_getPointerId(event, i);
                     Vec2   pixel(AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)),
                            pos  =D.windowPixelToScreen(pixel);
                     VecI2  posi =Round(pixel);
                     Touch *touch=FindTouchByHandle(pid);
                     if(   !touch)touch=&Touches.New().init(posi, pos, pid, stylus);else
                     {  // update
                        touch->_deltai+=posi-touch->_posi;
                        touch->_posi   =posi;
                        touch->_pos    =pos ;
                        if(!touch->_state)touch->_gui_obj=Gui.objAtPos(touch->pos()); // when hovering then also update gui object (check for 'state' because hover can be called the same frame that release is called, and for release we want to keep the original values)
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_UP:
               case AMOTION_EVENT_ACTION_CANCEL: // release all touches
               {
                  REPA(Touches)
                  {
                     Touch &touch=Touches[i];
                     touch._remove=true;
                     if(touch._state&BS_ON) // check for state in case it was manually eaten
                     {
                        touch._state|= BS_RELEASED;
                        touch._state&=~BS_ON;
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_POINTER_UP: // release one touch
               {
                  CPtr pid=(CPtr)AMotionEvent_getPointerId(event, action_index);
                  if(Touch *touch=FindTouchByHandle(pid))
                  {
                     Vec2  pixel(AMotionEvent_getX(event, action_index), AMotionEvent_getY(event, action_index));
                     VecI2 posi =Round(pixel);
                     touch->_deltai+=posi-touch->_posi;
                     touch->_posi   =posi;
                     touch->_pos    =D.windowPixelToScreen(pixel);
                     touch->_remove =true;
                     if(touch->_state&BS_ON) // check for state in case it was manually eaten
                     {
                        touch->_state|= BS_RELEASED;
                        touch->_state&=~BS_ON;
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_HOVER_EXIT: // hover exited screen
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr pid=(CPtr)AMotionEvent_getPointerId(event, i);
                     if(Touch *touch=FindTouchByHandle(pid))
                     {
                        if(touch->on()) // if was pressed then release it (so application can process release)
                        {
                           touch->_remove= true;
                           touch->_state|= BS_RELEASED;
                           touch->_state&=~BS_ON;
                        }else // if not pressed then just remove it
                        {
                           Touches.removeData(touch, true);
                        }
                     }
                  }
               }break;
            }
         }
#if 0
   Int pc=AMotionEvent_getPointerCount(event);
   Str o=S+"event:"+TextHex((UInt)source, -1, 0, true)+", action:"+Int(action_type)+", action_index:"+Int(action_index)+", ptr_cnt:"+pc+", ptr_id:";
   FREP(pc)o+=S+CPtr(AMotionEvent_getPointerId(event, i))+" | ";
   o+=S+"Touches:"+Touches.elms()+" "; FREPA(Touches)o+=S+'('+Touches[i]._state+", "+Touches[i].pos()+')';
   LogN(o);
#endif
      }return 1;

      case AINPUT_EVENT_TYPE_KEY:
      {
         Int    code =AKeyEvent_getKeyCode  (event),
                meta =AKeyEvent_getMetaState(event);
         Bool   ctrl =FlagTest(meta, (Int)AMETA_CTRL_ON     ),
                shift=FlagTest(meta, (Int)AMETA_SHIFT_ON    ),
                alt  =FlagTest(meta, (Int)AMETA_ALT_ON      ),
                caps =FlagTest(meta, (Int)AMETA_CAPS_LOCK_ON);
         Byte   bcode=Byte(code);
         KB_KEY key  =KeyMap[bcode];
         Byte   joy  =JoyMap[bcode];
         /*if(shift && !ctrl && !alt && !Kb.anyWin())
         {
            if(key==KB_BACK ){key=KB_DEL; Kb._disable_shift=true;}else // Shift+Back  = Del (this is     universal behaviour on Android platform)
            if(key==KB_ENTER){key=KB_INS; Kb._disable_shift=true;}     // Shift+Enter = Ins (this is non-universal behaviour on Android platform)
         }*/
         Joypad *joypad=null; if(joy){Bool empty=!InRange(0, Joypads); joypad=&Joypads(0); if(empty)joypad->_name=JavaInputDeviceName(device); joy--;}

//LogN(S+"dev:"+device+", code:"+code+", meta:"+meta+", key:"+key+", action:"+(int)AKeyEvent_getAction(event));
//if(!key)LogN(S+"key:"+code);

         // check for characters here still because 'dispatchKeyEvent' does not catch keys from hardware keyboards (like tablet with attachable keyboard)
         Char chr='\0';
         if(KeySource!=KEY_JAVA)
         {
            // get 'KeyCharacterMap' for this device
            if(!joypad)
               if(!KeyboardLoaded || KeyboardDeviceID!=device)
            {
               KeyboardLoaded  =true;
               KeyboardDeviceID=device;
               if(KeyCharacterMapGet)
               {
                  KeyCharacterMap=Jni->CallStaticObjectMethod(KeyCharacterMapClass, KeyCharacterMapLoad, jint(KeyboardDeviceID));
                  if(Jni->ExceptionCheck()){KeyCharacterMap.clear(); Jni->ExceptionClear();}else KeyCharacterMap.makeGlobal();
               }
            }

            if(KeyCharacterMap)
            {
               FlagDisable(meta, Int(AMETA_CTRL_ON|AMETA_CTRL_LEFT_ON|AMETA_CTRL_RIGHT_ON)); // disable CTRL in meta because if it would be present then 'chr' would be zero, we can't remove LALT yet because Swiftkey Android 2.3 relies on it
               chr=Jni->CallIntMethod(KeyCharacterMap, KeyCharacterMapGet, jint(code), jint(meta));
               if(!chr && (meta&AMETA_ALT_LEFT_ON))
               {
                  FlagDisable(meta, (Int)AMETA_ALT_LEFT_ON);
                  FlagSet(meta, (Int)AMETA_ALT_ON, FlagTest(meta, (Int)AMETA_ALT_RIGHT_ON)); // setup correct ALT mask
                  chr=Jni->CallIntMethod(KeyCharacterMap, KeyCharacterMapGet, jint(code), jint(meta));
               }
            }else
            if(!Kb.b(KB_RALT))chr=AdjustByShift(Kb._key_char[key], shift, caps);

            if(chr)KeySource=KEY_CPP; // if detected and character then set CPP source
         }

         switch(AKeyEvent_getAction(event))
         {
            case AKEY_EVENT_ACTION_DOWN:
            {
               if(joypad)joypad->push(joy);
                         Kb     .push(key, code); Kb.queue(chr, code); // !! queue characters after push !!
            }break;

            case AKEY_EVENT_ACTION_UP:
            {
               if(joypad)joypad->release(joy);
                         Kb     .release(key);
            }break;

            case AKEY_EVENT_ACTION_MULTIPLE:
            {
               if(joypad)
               {
                  joypad->push   (joy);
                  joypad->release(joy);
               }
               Kb.push   (key, code); Kb.queue(chr, code); // !! queue characters after push !!
               Kb.release(key      );
            }break;
         }
         // on some systems 'dispatchKeyEvent' will not be called if we return 1
         if(chr              // if we did receive a character then we don't need 'dispatchKeyEvent' anymore
         || joypad           // no point in forwarding joy buttons further
         || key==KB_NAV_BACK // on Asus Transformer Prime pressing this hardware keyboard key will close the app
         )return 1;
      }return 0; // call 'dispatchKeyEvent'

      default:
      {
       /*Int device=AInputEvent_getDeviceId(event);
         LogN(S+"Unknown input type, dev:"+device);*/
      }break;
   }
   return 0;
}
/******************************************************************************/
enum ANDROID_STATE
{
   AS_FOCUSED=1<<0,
   AS_STOPPED=1<<1,
   AS_PAUSED =1<<2,
};
static Byte AndroidState=0;
static void SetActive()
{
   Bool    active=((AndroidState&(AS_FOCUSED|AS_STOPPED|AS_PAUSED))==AS_FOCUSED);
   App._maximized=((AndroidState&(           AS_STOPPED|AS_PAUSED))==         0);
   App._minimized=!App._maximized;
   if(App.active()!=active)
   {
      if(active)
      {
         if(Jni && ActivityClass)
         if(JMethodID stopBackgroundService=Jni->GetMethodID(ActivityClass, "stopBackgroundService", "()V"))
            Jni->CallVoidMethod(Activity, stopBackgroundService);

         LOG2("ResumeSound");
         ResumeSound();
         LOG2("Kb.refreshTextInput");
         Kb.refreshTextInput();
      }
      LOG(S+"App.setActive("+active+")");
      App.setActive(active);
      if(!active)
      {
         if(App.flag&APP_WORK_IN_BACKGROUND)
         {
            if(Jni && ActivityClass)
            if(JMethodID startBackgroundService=Jni->GetMethodID(ActivityClass, "startBackgroundService", "(Ljava/lang/String;)V"))
            if(JString   j_text=JString(Jni, App.backgroundText()))
               Jni->CallVoidMethod(Activity, startBackgroundService, j_text());
         }else
         {
            LOG2("PauseSound");
            PauseSound();
         }
      }
      LOG2("SetActive finished");
   }
}
static void CmdCallback(android_app *app, int32_t cmd)
{
   switch(cmd)
   {
      default: LOG(S+"CmdCallback ("+cmd+")"); break;

      case APP_CMD_INIT_WINDOW:
      {
         LOG("APP_CMD_INIT_WINDOW");
         if(app->window)
         {
            if(!Initialized)
            {
               Initialized=true;
               if(!App.create())Exit("Failed to initialize the application"); // something failed, and current state of global variables is in failed mode (they won't be restored at next app launch because Android does not reset the state of global variables) that's why need to do complete reset by killing the process
            }else
            {
               D.androidOpen();
            }
         }
      }break;

      case APP_CMD_TERM_WINDOW:
      {
         LOG("APP_CMD_TERM_WINDOW");
         D.androidClose();
      }break;

      case APP_CMD_CONFIG_CHANGED:
      {
         LOG("APP_CMD_CONFIG_CHANGED");
         Kb.refreshTextInput();
       /*EGLint w=-1, h=-1;
         eglQuerySurface(display, surface, EGL_WIDTH , &w);
         eglQuerySurface(display, surface, EGL_HEIGHT, &h);
         LOG(S+"EGL: w:"+w+", h:"+h);*/
         UpdateOrientation();
       //if(app->window)UpdateSize(*app->window); // at this stage, old window size may occur, instead we need to check this every frame
      }break;

      case APP_CMD_WINDOW_REDRAW_NEEDED:
      {
         LOG("APP_CMD_WINDOW_REDRAW_NEEDED");
         if(D.created())DrawState();
      }break;

      case APP_CMD_GAINED_FOCUS: LOG("APP_CMD_GAINED_FOCUS"); FlagSet(AndroidState, AS_FOCUSED, true ); SetActive(); break;
      case APP_CMD_LOST_FOCUS  : LOG("APP_CMD_LOST_FOCUS"  ); FlagSet(AndroidState, AS_FOCUSED, false); SetActive(); break;
      case APP_CMD_START       : LOG("APP_CMD_START"       ); FlagSet(AndroidState, AS_STOPPED, false); SetActive(); break;
      case APP_CMD_STOP        : LOG("APP_CMD_STOP"        ); FlagSet(AndroidState, AS_STOPPED, true ); SetActive(); break;
      case APP_CMD_PAUSE       : LOG("APP_CMD_PAUSE"       ); FlagSet(AndroidState, AS_PAUSED , true ); SetActive(); break;
      case APP_CMD_RESUME      : LOG("APP_CMD_RESUME"      ); FlagSet(AndroidState, AS_PAUSED , false); SetActive(); break;

      case APP_CMD_DESTROY             : LOG("APP_CMD_DESTROY"             ); break;
      case APP_CMD_SAVE_STATE          : LOG("APP_CMD_SAVE_STATE"          ); if(App.save_state)App.save_state(); break;
      case APP_CMD_WINDOW_RESIZED      : LOG("APP_CMD_WINDOW_RESIZED"      ); break;
      case APP_CMD_CONTENT_RECT_CHANGED: LOG("APP_CMD_CONTENT_RECT_CHANGED"); break;
      case APP_CMD_LOW_MEMORY          : LOG("APP_CMD_LOW_MEMORY"          ); App.lowMemory(); break;
   }
}
/******************************************************************************/
static void JavaInit()
{
   AndroidApp->onAppCmd    =  CmdCallback;
   AndroidApp->onInputEvent=InputCallback;
 //AndroidApp->activity->callbacks->onNativeWindowResized=WindowResized; // this is never called
   JVM         =AndroidApp->activity->vm;
   Activity    =AndroidApp->activity->clazz; // this is not stored as 'JObject' so we can always assign it
   AndroidSDK  =AndroidApp->activity->sdkVersion;
   AssetManager=AndroidApp->activity->assetManager;
   LOG(S);
   LOG(S+"Start - Already Initialized: "+Initialized+", JVM: "+JVM+", JNI:"+Ptr(Jni)+", SDK:"+AndroidSDK);
   Jni.attach();
   if(Jni && Activity)if(ActivityClass=Activity)ActivityClass.makeGlobal();
}
static void JavaShut()
{
   LOG("JavaShut");
   KeyCharacterMapClass.del();
   KeyCharacterMap     .del();
   KeyCharacterMapLoad =null;
   KeyCharacterMapGet  =null;
   InputDeviceClass    .del();
   InputDeviceGetName  =null;
   InputDeviceGetDevice=null;
   DefaultDisplay      .del();
   Activity            =null;
   getRotation         =null;

#if 0 // we need these even after 'JavaShut'
   ActivityClass        .del();
   ClipboardManagerClass.del();
   ClipboardManager     .del();
   REPAO(EsenthelLocationListener).del();
   LocationManager      .del();
   GPS_PROVIDER.del(); NETWORK_PROVIDER.del();
   getLastKnownLocation=getLatitude=getLongitude=getAltitude=getAccuracy=getSpeed=getTime=requestLocationUpdates=removeUpdates=null;
#endif

#if __ANDROID_API__<14
   getButtonState=getRawAxisValue=null;
   MotionEventClass.del();
#endif

   Jni.del();
}
static void JavaGetAppPackage()
{
   LOG("JavaGetAppPackage");
   if(!AndroidPackageName.is() && ActivityClass && Activity)
   if(JMethodID getPackageName=Jni->GetMethodID(ActivityClass, "getPackageName", "()Ljava/lang/String;"))
   if(JString package_name=Jni->CallObjectMethod(Activity, getPackageName))
      AndroidPackageName=package_name.str();
}
static void JavaGetAppName()
{
   LOG("JavaGetAppName");
   //JClass Context="android/content/Context")
   //JMethodID getApplicationContext=Jni->GetMethodID(Context, "getApplicationContext", "()Landroid/content/Context;");
   //JMethodID getPackageName=Jni->GetMethodID(Context, "getPackageName", "()Ljava/lang/String;");
   //JMethodID getApplicationContext=Jni->GetStaticMethodID(Context, "getApplicationContext", "()Landroid/content/Context;");
   //JMethodID getPackageName=Jni->GetStaticMethodID(Context, "getPackageName", "()Ljava/lang/String;");

   if(Jni && ActivityClass)
   if(JClass FileClass="java/io/File")
   if(JMethodID getAbsolutePath=Jni->GetMethodID(FileClass, "getAbsolutePath", "()Ljava/lang/String;"))
   {
      // code and assets
      if(JMethodID getPackageCodePath=Jni->GetMethodID     (ActivityClass, "getPackageCodePath", "()Ljava/lang/String;"))
      if(JString            code_path=Jni->CallObjectMethod(Activity     ,  getPackageCodePath))
         AndroidAppPath=    code_path.str().replace('/', '\\');

      // resources
      //if(JMethodID getPackageResourcePath=Jni->GetMethodID     (ActivityClass, "getPackageResourcePath", "()Ljava/lang/String;"))
      //if(JString            resource_path=Jni->CallObjectMethod(Activity     ,  getPackageResourcePath)){}

      // app data private
      if(JMethodID getFilesDir     =Jni->GetMethodID     (ActivityClass, "getFilesDir", "()Ljava/io/File;"))
      if(JObject      files_dir    =Jni->CallObjectMethod(Activity     ,  getFilesDir))
      if(JString      files_dir_str=Jni->CallObjectMethod(files_dir    ,  getAbsolutePath))
         AndroidAppDataPath=files_dir_str.str().replace('/', '\\');

      // app data public
      if(JMethodID getExternalFilesDir      =Jni->GetMethodID     (ActivityClass     , "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;"))
      if(JObject      external_files_dir    =Jni->CallObjectMethod(Activity          ,  getExternalFilesDir, null))
      if(JString      external_files_dir_str=Jni->CallObjectMethod(external_files_dir,  getAbsolutePath))
         AndroidAppDataPublicPath=external_files_dir_str.str().replace('/', '\\');

      // public
      if(JClass    Environment="android/os/Environment")
      if(JMethodID getExternalStorageDirectory    =Jni-> GetStaticMethodID    (Environment, "getExternalStorageDirectory", "()Ljava/io/File;"))
      if(JObject      externalStorageDirectory    =Jni->CallStaticObjectMethod(Environment,  getExternalStorageDirectory))
      if(JString      externalStorageDirectory_str=Jni->CallObjectMethod      (externalStorageDirectory, getAbsolutePath))
         AndroidPublicPath=externalStorageDirectory_str.str().replace('/', '\\');

      // SD Card
      // this method gets array of files using 'getExternalFilesDirs', and ignores 'AndroidAppDataPublicPath' obtained with 'getExternalFilesDir', then it removes the shared end like "external/Android/App", "sd_card/Android/App" to leave only "sd_card"
      if(JMethodID getExternalFilesDirs=Jni->GetMethodID     (ActivityClass, "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;"))
      if(JObjectArray              dirs=Jni->CallObjectMethod(Activity,  getExternalFilesDirs, null))
      {
         Int length=dirs.elms();
         FREP(length)
            if(JObject dir=dirs[i]) // check this because it can be null
            if(JString dir_path=Jni->CallObjectMethod(dir, getAbsolutePath))
         {
            Str path=dir_path.str(); if(path.is() && !EqualPath(path, AndroidAppDataPublicPath))
            {
               path.replace('/', '\\');
               Int same=0;
               FREPA(path)
               {
                  Char a=path[path.length()-1-i], b=AndroidAppDataPublicPath[AndroidAppDataPublicPath.length()-1-i]; // last chars
                  if(IsSlash(a) && IsSlash(b))same=i+1;else if(a!=b)break;
               }
               path.removeLast(same);
               AndroidSDCardPath=path;
               break;
            }
         }
      }
      if(Jni->ExceptionCheck())Jni->ExceptionClear(); // clear in case 'getExternalFilesDirs' was not found
   }
   if(LogInit && AndroidPublicPath.is() && !EqualPath(AndroidPublicPath, GetPath(LogName()))){Str temp=LogName(); LogN(S+"Found public storage directory and continuing log there: "+AndroidPublicPath); LogName(Str(AndroidPublicPath).tailSlash(true)+"Esenthel Log.txt"); LogN(S+"Continuing log from: "+temp);}
}
static void JavaLooper()
{
   // prepare the Looper, this is needed for 'ClipboardManager' and 'LocationListener'
   LOG("JavaLooper");
   if(Jni)
   if(JClass LooperClass="android/os/Looper")
   if(JMethodID prepare=Jni->GetStaticMethodID(LooperClass, "prepare", "()V"))
   {
      Jni->CallStaticVoidMethod(LooperClass, prepare);
      if(Jni->ExceptionCheck())
      {
      #if DEBUG
         Jni->ExceptionDescribe();
      #endif
         Jni->ExceptionClear();
      }
   }
}
static void JavaGetInput()
{
   LOG("JavaGetInput");

#if __ANDROID_API__<14
   if(!MotionEventClass)
      if(Jni)
      if(MotionEventClass="android/view/MotionEvent")
      {
         getButtonState =Jni->GetStaticMethodID(MotionEventClass, "nativeGetButtonState" , X64 ? "(J)I"    : "(I)I"   ); if(Jni->ExceptionCheck()){getButtonState =null; Jni->ExceptionClear();} // Java exception can occur when methods not found
         getRawAxisValue=Jni->GetStaticMethodID(MotionEventClass, "nativeGetRawAxisValue", X64 ? "(JIII)F" : "(IIII)F"); if(Jni->ExceptionCheck()){getRawAxisValue=null; Jni->ExceptionClear();} // Java exception can occur when methods not found
      }
#endif

   if(!InputDeviceClass)
      if(Jni)
      if(InputDeviceClass="android/view/InputDevice")
   {
      InputDeviceGetDevice=Jni->GetStaticMethodID(InputDeviceClass, "getDevice", "(I)Landroid/view/InputDevice;"); if(Jni->ExceptionCheck()){InputDeviceGetDevice=null; Jni->ExceptionClear();} // Java exception can occur when methods not found
      InputDeviceGetName  =Jni->GetMethodID      (InputDeviceClass, "getName"  , "()Ljava/lang/String;"         ); if(Jni->ExceptionCheck()){InputDeviceGetName  =null; Jni->ExceptionClear();} // Java exception can occur when methods not found
   }
}
static void JavaGetScreenSize()
{
   LOG("JavaGetScreenSize");
   if(!DefaultDisplay)
      if(Jni && ActivityClass)
   {
      // DisplayMetrics display_metrics=new DisplayMetrics();
      // getWindowManager().getDefaultDisplay().getMetrics(display_metrics);
      // w=display_metrics.widthPixels;
      // h=display_metrics.heightPixels;

      // DisplayMetrics display_metrics=new DisplayMetrics();
      //if(JClass DisplayMetricsClass="android/util/DisplayMetrics")
      //if(JMethodID DisplayMetricsCtor=Jni->GetMethodID(DisplayMetricsClass, "<init>", "()V"))
      //if(JObject display_metrics=Jni->NewObject(DisplayMetricsClass, DisplayMetricsCtor))

      // Get WindowManager
      if(JClass WindowManagerClass="android/view/WindowManager")
      if(JMethodID getWindowManager=Jni->GetMethodID(ActivityClass, "getWindowManager", "()Landroid/view/WindowManager;"))
      if(JObject window_manager=Jni->CallObjectMethod(Activity, getWindowManager))
      {
         // Get DefaultDisplay
         if(JClass Display="android/view/Display")
         if(JMethodID getDefaultDisplay=Jni->GetMethodID(WindowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;"))
         if(DefaultDisplay=Jni->CallObjectMethod(window_manager, getDefaultDisplay))if(DefaultDisplay.makeGlobal())
         {
            // Get Display methods
                      getRotation=Jni->GetMethodID(Display, "getRotation", "()I"); // set this as the last one so only one 'if' can be performed in 'UpdateOrientation'
            JMethodID getWidth   =Jni->GetMethodID(Display, "getWidth"   , "()I");
            JMethodID getHeight  =Jni->GetMethodID(Display, "getHeight"  , "()I");
          //JMethodID getMetrics =Jni->GetMethodID(Display, "getMetrics" , "(Landroid/util/DisplayMetrics;)V");
            Int rotation=(getRotation ? Jni->CallIntMethod(DefaultDisplay, getRotation) : ROTATION_0),
                width   =(getWidth    ? Jni->CallIntMethod(DefaultDisplay, getWidth   ) : 800       ),
                height  =(getHeight   ? Jni->CallIntMethod(DefaultDisplay, getHeight  ) : 600       );
            if(rotation==ROTATION_90 || rotation==ROTATION_270)Swap(width, height);

            App._desktop_size.set(width, height);
         }
      }
   }
   UpdateOrientation();
}
static void JavaKeyboard()
{
   LOG("JavaKeyboard");
   if(!KeyCharacterMapGet)
      if(Jni)
      if(KeyCharacterMapClass="android/view/KeyCharacterMap")
      if(KeyCharacterMapLoad=Jni->GetStaticMethodID(KeyCharacterMapClass, "load", "(I)Landroid/view/KeyCharacterMap;"))
         KeyCharacterMapGet=Jni->GetMethodID(KeyCharacterMapClass, "get", "(II)I");
}
static void JavaClipboard()
{
   LOG("JavaClipboard");
   if(!ClipboardManager)
      if(Jni && ActivityClass)
      if(JClass ContextClass="android/content/Context")
      if(JFieldID CLIPBOARD_SERVICEField=Jni->GetStaticFieldID(ContextClass, "CLIPBOARD_SERVICE", "Ljava/lang/String;"))
      if(JObject CLIPBOARD_SERVICE=Jni->GetStaticObjectField(ContextClass, CLIPBOARD_SERVICEField))
      if(ClipboardManagerClass="android/text/ClipboardManager")if(ClipboardManagerClass.makeGlobal()) // android/content/ClipboardManager
      if(JMethodID getSystemService=Jni->GetMethodID(ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"))
      {
         ClipboardManager=Jni->CallObjectMethod(Activity, getSystemService, CLIPBOARD_SERVICE());
         if(Jni->ExceptionCheck())
         {
         #if DEBUG
            Jni->ExceptionDescribe();
         #endif
            ClipboardManager.clear(); ClipboardManagerClass.clear(); Jni->ExceptionClear(); LOG("ClipboardManager failed");
         }else ClipboardManager.makeGlobal();
      }
}
static void JavaLocation()
{
   LOG("JavaLocation");
   if(!LocationManager && Jni && ActivityClass)
   if(JMethodID getClassLoader     =Jni->GetMethodID(ActivityClass, "getClassLoader", "()Ljava/lang/ClassLoader;"))
   if(JObject      ClassLoader     =Jni->CallObjectMethod(Activity, getClassLoader))
   if(JClass       ClassLoaderClass="java/lang/ClassLoader")
   if(JMethodID loadClass=Jni->GetMethodID(ClassLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"))
   if(JClass ContextClass="android/content/Context")
   if(JFieldID LOCATION_SERVICEField=Jni->GetStaticFieldID(ContextClass, "LOCATION_SERVICE", "Ljava/lang/String;"))
   if(JObject LOCATION_SERVICE=Jni->GetStaticObjectField(ContextClass, LOCATION_SERVICEField))
   if(JClass LocationManagerClass="android/location/LocationManager")
   if(JMethodID getSystemService=Jni->GetMethodID(ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"))
   {
      LocationManager=Jni->CallObjectMethod(Activity, getSystemService, LOCATION_SERVICE());
      if(Jni->ExceptionCheck())
      {
      #if DEBUG
         Jni->ExceptionDescribe();
      #endif
         LocationManager.clear(); LocationManagerClass.clear(); Jni->ExceptionClear(); LOG("LocationManager failed");
      }else LocationManager.makeGlobal();

      if(LocationManager)
      if(JClass LocationClass="android/location/Location")
      if(JFieldID     GPS_PROVIDERField=Jni->GetStaticFieldID(LocationManagerClass,     "GPS_PROVIDER", "Ljava/lang/String;"))
      if(JFieldID NETWORK_PROVIDERField=Jni->GetStaticFieldID(LocationManagerClass, "NETWORK_PROVIDER", "Ljava/lang/String;"))
      if(    GPS_PROVIDER      =Jni->GetStaticObjectField(LocationManagerClass,     GPS_PROVIDERField))if(    GPS_PROVIDER.makeGlobal())
      if(NETWORK_PROVIDER      =Jni->GetStaticObjectField(LocationManagerClass, NETWORK_PROVIDERField))if(NETWORK_PROVIDER.makeGlobal())
      if(getLastKnownLocation  =Jni->GetMethodID(LocationManagerClass, "getLastKnownLocation"  , "(Ljava/lang/String;)Landroid/location/Location;"))
      if(getLatitude           =Jni->GetMethodID(LocationClass       , "getLatitude"           , "()D"))
      if(getLongitude          =Jni->GetMethodID(LocationClass       , "getLongitude"          , "()D"))
      if(getAltitude           =Jni->GetMethodID(LocationClass       , "getAltitude"           , "()D"))
      if(getAccuracy           =Jni->GetMethodID(LocationClass       , "getAccuracy"           , "()F"))
      if(getSpeed              =Jni->GetMethodID(LocationClass       , "getSpeed"              , "()F"))
      if(getTime               =Jni->GetMethodID(LocationClass       , "getTime"               , "()J")) // set this as the last one so only one 'if' can be performed in 'UpdateLocation'
      if(requestLocationUpdates=Jni->GetMethodID(LocationManagerClass, "requestLocationUpdates", "(Ljava/lang/String;JFLandroid/location/LocationListener;)V"))
      if(removeUpdates         =Jni->GetMethodID(LocationManagerClass, "removeUpdates"         , "(Landroid/location/LocationListener;)V"))
      if(JString EsenthelLocationListenerName=AndroidPackageName+"/EsenthelActivity$EsenthelLocationListener")
      {
         JClass EsenthelLocationListenerClass=(jclass)Jni->CallObjectMethod(ClassLoader, loadClass, EsenthelLocationListenerName());
         if(Jni->ExceptionCheck()){EsenthelLocationListenerClass.clear(); Jni->ExceptionClear(); LOG("EsenthelLocationListenerClass failed");}

         if(EsenthelLocationListenerClass)
         if(JMethodID EsenthelLocationListenerCtor=Jni->GetMethodID(EsenthelLocationListenerClass, "<init>", "(Z)V"))
         REP(2)
         {
            EsenthelLocationListener[i]=Jni->NewObject(EsenthelLocationListenerClass, EsenthelLocationListenerCtor, jboolean(i)); // set this as the last one so only one 'if' can be performed in 'SetLocationRefresh'
            if(Jni->ExceptionCheck()){EsenthelLocationListener[i].clear(); Jni->ExceptionClear(); LOG(S+"EsenthelLocationListener["+i+"] failed");}else EsenthelLocationListener[i].makeGlobal();
         }
      }
   }

   if(Jni)Jni->ExceptionClear(); // 'ExceptionClear' must be called in case some func returned null
   UpdateLocation(Jni);
}
/******************************************************************************/
static void InitSensor()
{
   LOG("InitSensor");
   if(!SensorEventQueue)
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
         SensorEventQueue=ASensorManager_createEventQueue(sensor_manager, AndroidApp->looper, LOOPER_ID_USER, null, null);
}
static void ShutSensor()
{
   LOG("ShutSensor");
   InputDevices.acquire(false); // disable all sensors just in case
   if(SensorEventQueue)
   {
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
         ASensorManager_destroyEventQueue(sensor_manager, SensorEventQueue);
      SensorEventQueue=null;
   }
}
/******************************************************************************/
static void InitKeyMap()
{
   KeyMap[AKEYCODE_0]=KB_0;
   KeyMap[AKEYCODE_1]=KB_1;
   KeyMap[AKEYCODE_2]=KB_2;
   KeyMap[AKEYCODE_3]=KB_3;
   KeyMap[AKEYCODE_4]=KB_4;
   KeyMap[AKEYCODE_5]=KB_5;
   KeyMap[AKEYCODE_6]=KB_6;
   KeyMap[AKEYCODE_7]=KB_7;
   KeyMap[AKEYCODE_8]=KB_8;
   KeyMap[AKEYCODE_9]=KB_9;

   KeyMap[AKEYCODE_A]=KB_A;
   KeyMap[AKEYCODE_B]=KB_B;
   KeyMap[AKEYCODE_C]=KB_C;
   KeyMap[AKEYCODE_D]=KB_D;
   KeyMap[AKEYCODE_E]=KB_E;
   KeyMap[AKEYCODE_F]=KB_F;
   KeyMap[AKEYCODE_G]=KB_G;
   KeyMap[AKEYCODE_H]=KB_H;
   KeyMap[AKEYCODE_I]=KB_I;
   KeyMap[AKEYCODE_J]=KB_J;
   KeyMap[AKEYCODE_K]=KB_K;
   KeyMap[AKEYCODE_L]=KB_L;
   KeyMap[AKEYCODE_M]=KB_M;
   KeyMap[AKEYCODE_N]=KB_N;
   KeyMap[AKEYCODE_O]=KB_O;
   KeyMap[AKEYCODE_P]=KB_P;
   KeyMap[AKEYCODE_Q]=KB_Q;
   KeyMap[AKEYCODE_R]=KB_R;
   KeyMap[AKEYCODE_S]=KB_S;
   KeyMap[AKEYCODE_T]=KB_T;
   KeyMap[AKEYCODE_U]=KB_U;
   KeyMap[AKEYCODE_V]=KB_V;
   KeyMap[AKEYCODE_W]=KB_W;
   KeyMap[AKEYCODE_X]=KB_X;
   KeyMap[AKEYCODE_Y]=KB_Y;
   KeyMap[AKEYCODE_Z]=KB_Z;

   KeyMap[131]=KB_F1;
   KeyMap[132]=KB_F2;
   KeyMap[133]=KB_F3;
   KeyMap[134]=KB_F4;
   KeyMap[135]=KB_F5;
   KeyMap[136]=KB_F6;
   KeyMap[137]=KB_F7;
   KeyMap[138]=KB_F8;
   KeyMap[139]=KB_F9;
   KeyMap[140]=KB_F10;
   KeyMap[141]=KB_F11;
   KeyMap[142]=KB_F12;

   KeyMap[111           ]=KB_ESC;
   KeyMap[AKEYCODE_ENTER]=KB_ENTER;
   KeyMap[AKEYCODE_SPACE]=KB_SPACE;
   KeyMap[AKEYCODE_DEL  ]=KB_BACK;
   KeyMap[AKEYCODE_TAB  ]=KB_TAB;

   KeyMap[113                 ]=KB_LCTRL;
   KeyMap[114                 ]=KB_RCTRL;
   KeyMap[AKEYCODE_SHIFT_LEFT ]=KB_LSHIFT;
   KeyMap[AKEYCODE_SHIFT_RIGHT]=KB_RSHIFT;
   KeyMap[AKEYCODE_ALT_LEFT   ]=KB_LALT;
   KeyMap[AKEYCODE_ALT_RIGHT  ]=KB_RALT;
   KeyMap[117                 ]=KB_LWIN;
   KeyMap[118                 ]=KB_RWIN;
   KeyMap[AKEYCODE_MENU       ]=KB_MENU;

   KeyMap[AKEYCODE_DPAD_UP   ]=KB_UP;
   KeyMap[AKEYCODE_DPAD_DOWN ]=KB_DOWN;
   KeyMap[AKEYCODE_DPAD_LEFT ]=KB_LEFT;
   KeyMap[AKEYCODE_DPAD_RIGHT]=KB_RIGHT;

   KeyMap[124               ]=KB_INS;
   KeyMap[112               ]=KB_DEL;
   KeyMap[122               ]=KB_HOME;
   KeyMap[123               ]=KB_END;
   KeyMap[AKEYCODE_PAGE_UP  ]=KB_PGUP;
   KeyMap[AKEYCODE_PAGE_DOWN]=KB_PGDN;

   KeyMap[AKEYCODE_MINUS        ]=KB_SUB;
   KeyMap[AKEYCODE_EQUALS       ]=KB_EQUAL;
   KeyMap[AKEYCODE_LEFT_BRACKET ]=KB_LBRACKET;
   KeyMap[AKEYCODE_RIGHT_BRACKET]=KB_RBRACKET;
   KeyMap[AKEYCODE_SEMICOLON    ]=KB_SEMICOLON;
   KeyMap[AKEYCODE_APOSTROPHE   ]=KB_APOSTROPHE;
   KeyMap[AKEYCODE_COMMA        ]=KB_COMMA;
   KeyMap[AKEYCODE_PERIOD       ]=KB_DOT;
   KeyMap[AKEYCODE_SLASH        ]=KB_SLASH;
   KeyMap[AKEYCODE_BACKSLASH    ]=KB_BACKSLASH;
   KeyMap[AKEYCODE_GRAVE        ]=KB_TILDE;
   KeyMap[120                   ]=KB_PRINT;
   KeyMap[121                   ]=KB_PAUSE;
   KeyMap[115                   ]=KB_CAPS;
   KeyMap[143                   ]=KB_NUM;
   KeyMap[116                   ]=KB_SCROLL;

   KeyMap[154]=KB_NPDIV;
   KeyMap[155]=KB_NPMUL;
   KeyMap[156]=KB_NPSUB;
   KeyMap[157]=KB_NPADD;
   KeyMap[158]=KB_NPDEL;

   KeyMap[144]=KB_NP0;
   KeyMap[145]=KB_NP1;
   KeyMap[146]=KB_NP2;
   KeyMap[147]=KB_NP3;
   KeyMap[148]=KB_NP4;
   KeyMap[149]=KB_NP5;
   KeyMap[150]=KB_NP6;
   KeyMap[151]=KB_NP7;
   KeyMap[152]=KB_NP8;
   KeyMap[153]=KB_NP9;

   KeyMap[AKEYCODE_VOLUME_DOWN     ]=KB_VOL_DOWN;
   KeyMap[AKEYCODE_VOLUME_UP       ]=KB_VOL_UP;
   KeyMap[AKEYCODE_MUTE            ]=KB_VOL_MUTE;
   KeyMap[AKEYCODE_BACK            ]=KB_NAV_BACK;
   KeyMap[AKEYCODE_MEDIA_PREVIOUS  ]=KB_MEDIA_PREV;
   KeyMap[AKEYCODE_MEDIA_NEXT      ]=KB_MEDIA_NEXT;
   KeyMap[AKEYCODE_MEDIA_PLAY_PAUSE]=KB_MEDIA_PLAY;
   KeyMap[AKEYCODE_MEDIA_STOP      ]=KB_MEDIA_STOP;
   KeyMap[AKEYCODE_SEARCH          ]=KB_FIND;

   KeyMap[168]=KB_ZOOM_IN ; // AKEYCODE_ZOOM_IN
   KeyMap[169]=KB_ZOOM_OUT; // AKEYCODE_ZOOM_OUT

   JoyMap[AKEYCODE_BUTTON_A     ]=1;
   JoyMap[AKEYCODE_BUTTON_B     ]=2;
   JoyMap[AKEYCODE_BUTTON_X     ]=3;
   JoyMap[AKEYCODE_BUTTON_Y     ]=4;
   JoyMap[AKEYCODE_BUTTON_L1    ]=5;
   JoyMap[AKEYCODE_BUTTON_R1    ]=6;
   JoyMap[AKEYCODE_BUTTON_L2    ]=7;
   JoyMap[AKEYCODE_BUTTON_R2    ]=8;
   JoyMap[AKEYCODE_BUTTON_SELECT]=9;
   JoyMap[AKEYCODE_BUTTON_START ]=10;
   JoyMap[AKEYCODE_BUTTON_THUMBL]=11;
   JoyMap[AKEYCODE_BUTTON_THUMBR]=12;

   REPD(shift, 2)
   REPD(caps , 2)if(shift || caps)
   {
      Int m=((shift | (caps<<1))-1);
      REPA(ShiftMap[m])ShiftMap[m][i]=((shift==caps) ? Char8(i) : CaseUp(Char8(i)));
      if(shift)
      {
         ShiftMap[m][Byte('`')]='~';
         ShiftMap[m][Byte('1')]='!';
         ShiftMap[m][Byte('2')]='@';
         ShiftMap[m][Byte('3')]='#';
         ShiftMap[m][Byte('4')]='$';
         ShiftMap[m][Byte('5')]='%';
         ShiftMap[m][Byte('6')]='^';
         ShiftMap[m][Byte('7')]='&';
         ShiftMap[m][Byte('8')]='*';
         ShiftMap[m][Byte('9')]='(';
         ShiftMap[m][Byte('0')]=')';
         ShiftMap[m][Byte('-')]='_';
         ShiftMap[m][Byte('=')]='+';
         ShiftMap[m][Byte('[')]='{';
         ShiftMap[m][Byte(']')]='}';
         ShiftMap[m][Byte(';')]=':';
         ShiftMap[m][Byte('\'')]='"';
         ShiftMap[m][Byte('\\')]='|';
         ShiftMap[m][Byte(',')]='<';
         ShiftMap[m][Byte('.')]='>';
         ShiftMap[m][Byte('/')]='?';
      }
   }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
extern "C"
{

JNIEXPORT void JNICALL Java_com_esenthel_Native_connected(JNIEnv *env, jclass clazz, jboolean supports_items, jboolean supports_subs);
JNIEXPORT void JNICALL Java_com_esenthel_Native_location (JNIEnv *env, jclass clazz, jboolean gps, jobject location) {JNI jni(env); UpdateLocation(location, gps!=0, jni);}

}
/******************************************************************************/
//jint JNI_OnLoad(JavaVM *vm, void *reserved) {LOG("JNI_OnLoad"); return JNI_VERSION_1_6;} don't use since it's called not before 'android_main' but during the first 'ALooper_pollAll'
void android_main(android_app *app)
{
   // !! call before 'JavaInit' !!
   if(BackgroundThread.created()) // stop thread if running
   {
      BackgroundThread.stop(); // request thread to be stopped
      BackgroundThread._resume.on(); // wake up, Warning: here we reuse its 'SyncEvent' because it's used only for pausing/resuming
      BackgroundThread.del(); // wait and delete
   }

   // strip prevention
   Java_com_esenthel_Native_connected(null, null, Store.supportsItems(), Store.supportsSubscriptions()); // don't remove this, or the linker will strip all Java Native functions

   // init
   if(LogInit)
   {
      if(FExistSystem("/sdcard"        ))LogName(        "/sdcard/Esenthel Log.txt");else
      if(FExistSystem("/mnt/sdcard"    ))LogName(    "/mnt/sdcard/Esenthel Log.txt");else
      if(FExistSystem("/mnt/sdcard-ext"))LogName("/mnt/sdcard-ext/Esenthel Log.txt");
   }
   AndroidApp=app;
   JavaInit();
   if(!Initialized) // these can be called only once
   {
      InitKeyMap       ();
      JavaGetAppPackage();
      JavaGetAppName   ();
   }
   JavaLooper       ();
   JavaGetInput     ();
   JavaGetScreenSize();
   JavaKeyboard     ();
   JavaClipboard    ();
   JavaLocation     ();
   InitSensor       ();
   LOG("LoopStart");
   for(;;)
   {
      VecI2 old_posi=Ms.desktopPos();

      LOG2("ALooper_pollAll");

      Bool wait_end_set=false; Int wait=(App.active() ? App.active_wait : 0); UInt wait_end; // don't wait for !active, because we already wait after the event loop, and that would make 2 waits

      Int id, events; android_poll_source *source;
   wait:
      while((id=ALooper_pollAll(wait, null, &events, (void**)&source))>=0) // process all events, using negative 'wait' for 'ALooper_pollAll' means unlimited wait
      {
         LOG2(S+"ALooper Source:"+Ptr(source)+", id:"+id);
         if(source)source->process(AndroidApp, source); // process this event
         LOG2(S+"ALooper processed");

         if(id==LOOPER_ID_USER) // sensor data
         {
            ASensorEvent event;
         #if DEBUG
            if(!SensorEventQueue)LOG("Received sensor event but the 'SensorEventQueue' is null");
         #endif
            while(ASensorEventQueue_getEvents(SensorEventQueue, &event, 1)>0)switch(event.type)
            {
               case ASENSOR_TYPE_ACCELEROMETER : AccelerometerValue.set(-event.acceleration.x, -event.acceleration.y,  event.acceleration.z); break;
               case ASENSOR_TYPE_GYROSCOPE     :     GyroscopeValue.set( event.vector      .x,  event.vector      .y, -event.vector      .z); break;
               case ASENSOR_TYPE_MAGNETIC_FIELD:  MagnetometerValue.set( event.magnetic    .x,  event.magnetic    .y, -event.magnetic    .z); break;
            }
         }

         wait=0; // don't wait for next events, in case app 'activate' status changed or it was requested to be closed or destroyed
         // no need to check for 'App._close' or 'AndroidApp->destroyRequested' here, because we will do it below since we're setting wait=0 we don't risk with unlimited waits
      }
      if(App._close) // first check if we want to close manually
      {
         App.del(); // manually call shut down
         ExitNow(); // do complete reset (including state of global variables) by killing the process
      }
      if(AndroidApp->destroyRequested)break; // this is triggered by 'ANativeActivity_finish', just break out of the loop so app can get minimized
      if(!App.active()) // we may need to wait
         if(wait=((App.flag&APP_WORK_IN_BACKGROUND) ? App.background_wait : -1)) // how long
      {
         if(wait>0) // finite wait
         {
            if(wait_end_set) // if we already set the end time limit
            {
               wait=wait_end-Time.curTimeMs(); if(wait<=0)goto stop; // calculate remaining time
            }else
            {
               wait_end=Time.curTimeMs()+wait;
               wait_end_set=true;
            }
         }//else wait=-1; // use negative wait to force unlimited wait in 'ALooper_pollAll', no need for this, because 'wait' is already negative
         goto wait;
      }
   stop:

      // process input
      Ms._deltai          = Ms. desktopPos()-old_posi;
      Ms._delta_relative.x= Ms._deltai.x;
      Ms._delta_relative.y=-Ms._deltai.y;

      LOG2(S+"AndroidApp->window:"+(AndroidApp->window!=null));
   #if DEBUG
      if(!eglGetCurrentContext())LOG("No current EGL Context available on the main thread");
   #endif
      if(AndroidApp->window) // this may be unavailable if device is being (dis)connected to dock, or app is working in the background
         UpdateSize(*AndroidApp->window);
      App.update();
   }

   LOG("LoopEnd");
   KeyboardLoaded=false;
   ShutSensor(); // !! call before 'JavaShut' !!
   JavaShut(); // !! call before 'BackgroundThread' !!

   // app had input callback set, if there were some touches then they won't be unregistered now since app will be deleted, and because memory state can be preserved on Android, then manually remove them, without this touches may be preserved when rotating the screen, even though we're no longer touching it
   REPA(Touches)
   {
      Touch &touch=Touches[i];
      if(touch.on()) // if was pressed then release it (so application can process release)
      {
         touch._remove= true;
         touch._state|= BS_RELEASED;
         touch._state&=~BS_ON;
      }else // if not pressed then just remove it
      {
         Touches.remove(i, true);
      }
   }

   if((App.flag&APP_WORK_IN_BACKGROUND) && App.background_wait>=0)
   {
      LOG("BackgroundThread.create");
      BackgroundThread.create(BackgroundFunc, null, 0, false, "BackgroundThread"); // !! call after 'JavaShut' !!
   }
   LOG("End");
}
/******************************************************************************/
#endif // ANDROID
/******************************************************************************/
