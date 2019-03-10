/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#if !IS_POW_2(INPUT_COMBO_NUM)
   #error INPUT_COMBO_NUM must be a power of 2
#endif
/******************************************************************************/
#if ANDROID
ASensorEventQueue *SensorEventQueue;
#elif IOS
CLLocationManager *LocationManager[2];
CMMotionManager   *CoreMotionMgr;
#endif
Vec				   AccelerometerValue, GyroscopeValue, MagnetometerValue;
Bool              LocationBackground[2];
Dbl               LocationLat[2], LocationLon[2];
Flt               LocationAlt[2], LocationAcc[2], LocationSpd[2], LocationInterval[2]={-1, -1}, MagnetometerInterval=-1;
DateTime          LocationTim[2];
InputComboClass   InputCombo;
InputDevicesClass InputDevices;
/******************************************************************************/
// ACCELEROMETER, GYROSCOPE, MAGNETOMETER, LOCATION
/******************************************************************************/
C Vec& Accelerometer() {return AccelerometerValue;}
C Vec& Gyroscope    () {return     GyroscopeValue;}
C Vec& Magnetometer () {return  MagnetometerValue;}

  Dbl       LocationLatitude (Bool gps) {return LocationLat[gps];}
  Dbl       LocationLongitude(Bool gps) {return LocationLon[gps];}
  Flt       LocationAltitude (Bool gps) {return LocationAlt[gps];}
  Flt       LocationAccuracy (Bool gps) {return LocationAcc[gps];}
  Flt       LocationSpeed    (Bool gps) {return LocationSpd[gps];}
C DateTime& LocationTimeUTC  (Bool gps) {return LocationTim[gps];}
/******************************************************************************/
#if !WINDOWS_NEW
void SetMagnetometerRefresh(Flt interval)
{
#if ANDROID
   if(SensorEventQueue)
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
         if(C ASensor *magnetometer=ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_MAGNETIC_FIELD))
   {
      if(interval>=0) // enable
      {
            ASensorEventQueue_setEventRate (SensorEventQueue, magnetometer, RoundU(interval*1000000));
            ASensorEventQueue_enableSensor (SensorEventQueue, magnetometer);
      }else ASensorEventQueue_disableSensor(SensorEventQueue, magnetometer); // disable
   }
#elif IOS
   const Bool gps=true;
   if(CLLocationManager *lm=LocationManager[gps])
      if(interval>=0 && [CLLocationManager headingAvailable])
   {
      [lm startUpdatingHeading];
   }else
   {
      [lm stopUpdatingHeading];
   }
#endif
}
#endif
/******************************************************************************/
void SetLocationRefresh(Flt interval, Bool gps)
{
#if ANDROID
   JNI jni; if(jni && EsenthelLocationListener[gps])
   {
      if(interval>=0) // enable
      {
         RequirePermission(PERMISSION_LOCATION);

         ULong min_time=RoundU(interval*1000);
         jni->CallVoidMethod(LocationManager, requestLocationUpdates, gps ? GPS_PROVIDER() : NETWORK_PROVIDER(), min_time, 0.0f, EsenthelLocationListener[gps]());
      }else // disable
      {
         jni->CallVoidMethod(LocationManager, removeUpdates, EsenthelLocationListener[gps]());
      }
      if(jni->ExceptionCheck())
      {
      #if DEBUG
         jni->ExceptionDescribe();
      #endif
         jni->ExceptionClear();
      }
   }
#elif IOS
   if(CLLocationManager *lm=LocationManager[gps])
      if(interval>=0 && [CLLocationManager locationServicesEnabled])
   {
      if(LocationBackground[gps]){if([lm respondsToSelector:@selector(requestAlwaysAuthorization   )])[lm requestAlwaysAuthorization   ];}
      else                       {if([lm respondsToSelector:@selector(requestWhenInUseAuthorization)])[lm requestWhenInUseAuthorization];}
      if(gps)[lm startUpdatingLocation];
      else   [lm startMonitoringSignificantLocationChanges];
   }else
   {
      if(gps)[lm stopUpdatingLocation];
      else   [lm stopMonitoringSignificantLocationChanges];
   }
#endif
}
/******************************************************************************/
void MagnetometerRefresh(Flt interval) {MAX(interval, 0); if(!Equal(MagnetometerInterval, interval))SetMagnetometerRefresh(MagnetometerInterval=interval);}
void MagnetometerDisable(            ) {                  if(!Equal(MagnetometerInterval,       -1))SetMagnetometerRefresh(MagnetometerInterval=      -1);}

void LocationRefresh(Flt interval, Bool gps, Bool background) {MAX(interval, 0); if(!Equal(LocationInterval[gps], interval) || LocationBackground[gps]!=background){LocationBackground[gps]=background; SetLocationRefresh(LocationInterval[gps]=interval, gps);}}
void LocationDisable(              Bool gps                 ) {                  if(!Equal(LocationInterval[gps],       -1)                                       )                                     SetLocationRefresh(LocationInterval[gps]=      -1, gps); }

LOCATION_AUTHORIZATION_STATUS LocationAuthorizationStatus()
{
#if ANDROID
   return HasPermission(PERMISSION_LOCATION) ? LAS_BACKGROUND : LAS_DENIED;
#elif IOS
   switch([CLLocationManager authorizationStatus])
   {
      case kCLAuthorizationStatusRestricted         : return LAS_RESTRICTED;
      case kCLAuthorizationStatusDenied             : return LAS_DENIED;
      case kCLAuthorizationStatusAuthorizedWhenInUse: return LAS_FOREGROUND;
      case kCLAuthorizationStatusAuthorizedAlways   : return LAS_BACKGROUND;
   }
#endif
   return LAS_UNKNOWN;
}
/******************************************************************************/
#if ANDROID
static ULong LocationTime[2];
void UpdateLocation(jobject location, Bool gps, JNI &jni)
{
   if(location && jni && getTime)
   {
      ULong time=jni->CallLongMethod(location, getTime); // milliseconds since 1st Jan 1970
      if(time!=LocationTime[gps])
      {
         LocationTim[gps].from1970ms(LocationTime[gps]=time);
         LocationLat[gps]=jni->CallDoubleMethod(location, getLatitude );
         LocationLon[gps]=jni->CallDoubleMethod(location, getLongitude);
         LocationAlt[gps]=jni->CallDoubleMethod(location, getAltitude );
         LocationAcc[gps]=jni->CallFloatMethod (location, getAccuracy );
         LocationSpd[gps]=jni->CallFloatMethod (location, getSpeed    );
      }
   }
}
void UpdateLocation(Bool gps, JNI &jni)
{
   if(jni && getLastKnownLocation)
   {
      JObject location=JObject(jni, jni->CallObjectMethod(LocationManager, getLastKnownLocation, gps ? GPS_PROVIDER() : NETWORK_PROVIDER()));
      if(jni->ExceptionCheck())
      {
      #if DEBUG
         jni->ExceptionDescribe();
      #endif
         location.clear(); jni->ExceptionClear();
      }
      UpdateLocation(location, gps, jni);
   }
}
void UpdateLocation(JNI &jni)
{
   REP(2)UpdateLocation(i!=0, jni);
}
#endif
/******************************************************************************/
// INPUT BUTTON
/******************************************************************************/
Bool InputButton::on()C
{
   switch(type)
   {
      case INPUT_KEYBOARD: return                            Kb             .b(KB_KEY(button));
      case INPUT_MOUSE   : return                            Ms             .b(       button );
      case INPUT_JOYPAD  : return InRange(device, Joypads) ? Joypads[device].b(       button ) : false;
   }
   return false;
}
Bool InputButton::pd()C
{
   switch(type)
   {
      case INPUT_KEYBOARD: return                            Kb             .bp(KB_KEY(button));
      case INPUT_MOUSE   : return                            Ms             .bp(       button );
      case INPUT_JOYPAD  : return InRange(device, Joypads) ? Joypads[device].bp(       button ) : false;
   }
   return false;
}
Bool InputButton::rs()C
{
   switch(type)
   {
      case INPUT_KEYBOARD: return                            Kb             .br(KB_KEY(button));
      case INPUT_MOUSE   : return                            Ms             .br(       button );
      case INPUT_JOYPAD  : return InRange(device, Joypads) ? Joypads[device].br(       button ) : false;
   }
   return false;
}
Bool InputButton::db()C
{
   switch(type)
   {
      case INPUT_KEYBOARD: return                            Kb             .bd(KB_KEY(button));
      case INPUT_MOUSE   : return                            Ms             .bd(       button );
      case INPUT_JOYPAD  : return InRange(device, Joypads) ? Joypads[device].bd(       button ) : false;
   }
   return false;
}
Str InputButton::name()C
{
   switch(type)
   {
      case INPUT_KEYBOARD: return                            Kb             .   keyName(KB_KEY(button));
      case INPUT_MOUSE   : return                            Ms             .buttonName(       button );
      case INPUT_JOYPAD  : return InRange(device, Joypads) ? Joypads[device].buttonName(       button ) : S;
      default            : return S;
   }
}
Bool InputButton::operator==(C InputButton &b)C
{
   return T.button==b.button
       && T.type  ==b.type
       && T.device==b.device;
}
Bool InputButton::operator!=(C InputButton &b)C
{
   return T.button!=b.button
       || T.type  !=b.type
       || T.device!=b.device;
}
/******************************************************************************/
// INPUT
/******************************************************************************/
Bool Input::on()C {return b[0].on() || b[1].on() || b[2].on();}
Bool Input::pd()C {return b[0].pd() || b[1].pd() || b[2].pd();}
Bool Input::rs()C {return b[0].rs() || b[1].rs() || b[2].rs();}
Bool Input::db()C {return b[0].db() || b[1].db() || b[2].db();}

void Input::set(C Str &name, UInt group)
{
   T.name =name ;
   T.group=group;
}
void Input::set(C Str &name, UInt group, C InputButton &b0, C InputButton *b1, C InputButton *b2)
{
   T.name =name ;
   T.group=group;
         b[0]= b0;
   if(b1)b[1]=*b1;else Zero(b[1]);
   if(b2)b[2]=*b2;else Zero(b[2]);
}
Bool Input::operator()(C InputButton &b)C
{
   return T.b[0]==b
       || T.b[1]==b
       || T.b[2]==b;
}
/******************************************************************************/
// INPUT COMBO
/******************************************************************************/
#define INPUT_COMBO_AND (INPUT_COMBO_NUM-1)

InputComboClass::InputComboClass()
{
   dt=0.22f;
}
void InputComboClass::clear()
{
   pos=length=0;
}
void InputComboClass::add(C InputButton &inp_btn)
{
   t[pos]=Time.appTime();
   b[pos]=inp_btn;
   pos   =((pos+1)&INPUT_COMBO_AND);
   length=Min(length+1, Elms(b));
}
Bool InputComboClass::operator()(C Input &i0, C Input &i1)C
{
   if(length>=2)
   {
      Byte p1=((pos-1)&INPUT_COMBO_AND),
           p0=((pos-2)&INPUT_COMBO_AND);

      return i1(b[p1]) && (Time.appTime()-t[p1]<dt)
          && i0(b[p0]) && (t[p1]         -t[p0]<dt);
   }
   return false;
}
Bool InputComboClass::operator()(C Input &i0, C Input &i1, C Input &i2)C
{
   if(length>=3)
   {
      Byte p2=((pos-1)&INPUT_COMBO_AND),
           p1=((pos-2)&INPUT_COMBO_AND),
           p0=((pos-3)&INPUT_COMBO_AND);

      return i2(b[p2]) && (Time.appTime()-t[p2]<dt)
          && i1(b[p1]) && (t[p2]         -t[p1]<dt)
          && i0(b[p0]) && (t[p1]         -t[p0]<dt);
   }
   return false;
}
Bool InputComboClass::operator()(C Input &i0, C Input &i1, C Input &i2, C Input &i3)C
{
   if(length>=4)
   {
      Byte p3=((pos-1)&INPUT_COMBO_AND),
           p2=((pos-2)&INPUT_COMBO_AND),
           p1=((pos-3)&INPUT_COMBO_AND),
           p0=((pos-4)&INPUT_COMBO_AND);

      return i3(b[p3]) && (Time.appTime()-t[p3]<dt)
          && i2(b[p2]) && (t[p3]         -t[p2]<dt)
          && i1(b[p1]) && (t[p2]         -t[p1]<dt)
          && i0(b[p0]) && (t[p1]         -t[p0]<dt);
   }
   return false;
}
Bool InputComboClass::operator()(C Input &i0, C Input &i1, C Input &i2, C Input &i3, C Input &i4)C
{
   if(length>=5)
   {
      Byte p4=((pos-1)&INPUT_COMBO_AND),
           p3=((pos-2)&INPUT_COMBO_AND),
           p2=((pos-3)&INPUT_COMBO_AND),
           p1=((pos-4)&INPUT_COMBO_AND),
           p0=((pos-5)&INPUT_COMBO_AND);

      return i4(b[p4]) && (Time.appTime()-t[p4]<dt)
          && i3(b[p3]) && (t[p4]         -t[p3]<dt)
          && i2(b[p2]) && (t[p3]         -t[p2]<dt)
          && i1(b[p1]) && (t[p2]         -t[p1]<dt)
          && i0(b[p0]) && (t[p1]         -t[p0]<dt);
   }
   return false;
}
#undef INPUT_COMBO_AND
/******************************************************************************/
// INPUT
/******************************************************************************/
void InputDevicesClass::del()
{
   ShutJoypads();
   Ms.del();
   Kb.del();
#if WINDOWS_OLD
   RELEASE(DI);
#elif IOS
   [CoreMotionMgr release]; CoreMotionMgr=null;
#endif
   VR.shut(); // !! delete as last, after the mouse, because it may try to reset the mouse cursor, so we need to make sure that mouse cursor was already deleted !!
}
/******************************************************************************/
Bool InputDevicesClass::create()
{
   if(LogInit)LogN("InputDevicesClass.create");
#if WINDOWS_OLD
   if(LogInit)LogN("DirectInput8Create");
   if(OK(DirectInput8Create(App._hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (Ptr*)&DI, null)))
#endif
   {
      Kb  .create();
      Ms  .create();
      InitJoypads();
    //VR  .init  (); this is now to be called manually in 'InitPre'
   #if IOS
      CoreMotionMgr=[[CMMotionManager alloc] init];
      CoreMotionMgr.accelerometerUpdateInterval=1.0f/60; // 60 Hz
      CoreMotionMgr.         gyroUpdateInterval=1.0f/60; // 60 Hz
   #endif
      if(App.active())acquire(true);
      return true;
   }
   return false;
}
/******************************************************************************/
void InputDevicesClass::update()
{
         Kb      .update();
         Ms      .update();
   REPAO(Joypads).update();
           TouchesUpdate();
               VR.update();
#if ANDROID
   if(!(Time.frame()%60))UpdateLocation(Jni); // update once per 60-frames, because it was reported that this can generate plenty of messages in the log
#elif IOS
   if(CoreMotionMgr)
   {
      if(CMAccelerometerData *acceleration=CoreMotionMgr.accelerometerData)
         AccelerometerValue.set(acceleration.acceleration.x, acceleration.acceleration.y, -acceleration.acceleration.z)*=9.80665f;
      if(CMGyroData *gyroscope=CoreMotionMgr.gyroData)
         GyroscopeValue.set(gyroscope.rotationRate.x, gyroscope.rotationRate.y, -gyroscope.rotationRate.z);
   }
#endif
}
void InputDevicesClass::clear()
{
         Kb      .clear();
         Ms      .clear();
   REPAO(Joypads).clear();
           TouchesClear();
}
/******************************************************************************/
void InputDevicesClass::acquire(Bool on)
{
#if WINDOWS && !(_WIN32_WINNT>=_WIN32_WINNT_WIN10) // this is now deprecated
   XInputEnable(on);
#endif
         Kb      .acquire(on);
         Ms      .acquire(on);
   REPAO(Joypads).acquire(on);
#if ANDROID
   if(SensorEventQueue)
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
   {
      if(C ASensor *accelerometer=ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_ACCELEROMETER))
      {
         if(on) // start monitoring the accelerometer
         {
               ASensorEventQueue_setEventRate (SensorEventQueue, accelerometer, 1000*1000/60); // 60 events per second
               ASensorEventQueue_enableSensor (SensorEventQueue, accelerometer);
         }else ASensorEventQueue_disableSensor(SensorEventQueue, accelerometer); // stop monitoring accelerometer
      }
      if(C ASensor *gyroscope=ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_GYROSCOPE))
      {
         if(on) // start monitoring the gyroscope
         {
               ASensorEventQueue_setEventRate (SensorEventQueue, gyroscope, 1000*1000/60); // 60 events per second
               ASensorEventQueue_enableSensor (SensorEventQueue, gyroscope);
         }else ASensorEventQueue_disableSensor(SensorEventQueue, gyroscope); // stop monitoring gyroscope
      }
   }
   if(on)UpdateLocation(Jni);
#elif IOS
   if(CoreMotionMgr)
   {
      if(on)
      {
         [CoreMotionMgr startAccelerometerUpdates];
         [CoreMotionMgr startGyroUpdates         ];
      }else
      {
         [CoreMotionMgr stopAccelerometerUpdates];
         [CoreMotionMgr stopGyroUpdates         ];
      }
   }
#endif
   REPA(LocationInterval)SetLocationRefresh(on ?     LocationInterval[i] : -1, i!=0);
                     SetMagnetometerRefresh(on ? MagnetometerInterval    : -1);
}
/******************************************************************************/
// TEXT EDIT
/******************************************************************************/
static void TextSelCopy(C Str &str, TextEdit &edit) // assumes that 'sel' and 'cur' are in 'str' range (or sel<0)
{
   if(edit.sel<0)ClipSet(str);else // copy all
   if(Int length=Abs(edit.sel-edit.cur)) // copy part
   {
      Memt<Char> temp; temp.setNum(length+1);
       CopyFastN(temp.data(), str()+Min(edit.sel, edit.cur), length);
                 temp[length]=0;
         ClipSet(temp.data());
   }else ClipSet(S); // copy nothing
}
static Bool TextSelRem(Str &str, TextEdit &edit) // assumes that 'sel' and 'cur' are in 'str' range (or sel<0)
{
   if(edit.sel<0) // remove all
   {
      str .clear();
      edit.cur= 0;
      edit.sel=-1;
   }else // remove part
   {
      Int pos=Min(edit.sel, edit.cur),
          cut=Abs(edit.sel- edit.cur);
      str .remove(pos, cut);
      edit.cur=pos;
      edit.sel=-1;
   }
   return true;
}
static void SkipCombiningLeft (C Str &str, TextEdit &edit) {for(; CharFlagFast(str[edit.cur])&CHARF_COMBINING && edit.cur>0; )edit.cur--;}
static void SkipCombiningRight(C Str &str, TextEdit &edit) {for(; CharFlagFast(str[edit.cur])&CHARF_COMBINING              ; )edit.cur++;}
static Bool Processed         (  Str &str, TextEdit &edit, Bool multi_line, C Keyboard::Key &key, Bool &changed)
{
   if(!key.winCtrl())
   {
      switch(key.k)
      {
         case KB_HOME:
         {
            if(!key.shift())edit.sel=-1;else if(edit.sel<0)edit.sel=edit.cur; 
            if(!multi_line || key.ctrlCmd())edit.cur=0;else for(; edit.cur>0 && str[edit.cur-1]!='\n'; )edit.cur--;
            return true;
         }break;

         case KB_END:
         {
            if(!key.shift())edit.sel=-1;else if(edit.sel<0)edit.sel=edit.cur;
            if(!multi_line || key.ctrlCmd())edit.cur=str.length();else for(; edit.cur<str.length() && str[edit.cur]!='\n'; )edit.cur++;
            return true;
         }break;

         case KB_LEFT:
         {
            if(edit.sel>=0 && !key.shift()){edit.cur=Min(edit.cur, edit.sel); edit.sel=-1;}else // cancel selection
            if(edit.cur)
            {
               if(key.shift() && edit.sel<0)edit.sel=edit.cur; // start selection from cursor
               edit.cur--; SkipCombiningLeft(str, edit);
               if(key.ctrlCmd())for(CHAR_TYPE ct=CharType(str[edit.cur]); edit.cur; )
               {
                  CHAR_TYPE nt=CharType(str[edit.cur-1]);
                  if(ct==CHART_SPACE)ct=nt;
                  if(ct!=nt)break;
                  edit.cur--; SkipCombiningLeft(str, edit);
               }
            }
            return true;
         }break;

         case KB_RIGHT:
         {
            if(edit.sel>=0 && !key.shift()){edit.cur=Max(edit.cur, edit.sel); edit.sel=-1;}else // cancel selection
            if(edit.cur<str.length())
            {
               if(key.shift() && edit.sel<0)edit.sel=edit.cur; // start selection from cursor
               edit.cur++; SkipCombiningRight(str, edit);
               if(key.ctrlCmd())for(CHAR_TYPE ct=CharType(str[edit.cur-1]); edit.cur<str.length(); )
               {
                  CHAR_TYPE nt=CharType(str[edit.cur]);
                  if(ct!=nt){for(; edit.cur<str.length() && str[edit.cur]==' '; )edit.cur++; break;}
                  edit.cur++; SkipCombiningRight(str, edit);
               }
            }
            return true;
         }break;

         case KB_BACK:
         {
            if(edit.sel>=0)changed|=TextSelRem(str, edit);else
            if(edit.cur)
            {
               if(key.ctrlCmd())
               {
                  edit.sel=edit.cur;
                  edit.cur--;
                  for(CHAR_TYPE ct=CharType(str[edit.cur]); edit.cur; edit.cur--)
                  {
                     CHAR_TYPE nt=CharType(str[edit.cur-1]);
                     if(ct!=nt)break;
                  }
                  TextSelRem(str, edit);
               }else
               {
                  str.remove(--edit.cur);
               }
               changed=true;
            }
            return true;
         }break;

         case KB_TAB: if(multi_line)
         {
            if(edit.sel>=0)changed|=TextSelRem(str, edit);
          /*if(edit.overwrite && edit.cur<str.length())str.    _d[edit.cur]='\t';
            else*/                                     str.insert(edit.cur, '\t');
            edit.cur++;
            changed=true;
            return true;
         }break;

         case KB_ENTER: if(multi_line)
         {
            if(edit.sel>=0)changed|=TextSelRem(str, edit);
          /*if(edit.overwrite && edit.cur<str.length())str.    _d[edit.cur]='\n';
            else*/                                     str.insert(edit.cur, '\n');
            edit.cur++;
            changed=true;
            return true;
         }break;

         case KB_INS:
         {
            if(key.ctrlCmd()) // copy
            {
            copy:
               if(!edit.password)TextSelCopy(str, edit);
            }else
            if(key.shift()) // paste
            {
            paste:
               if(edit.sel>=0)changed|=TextSelRem(str, edit);
               Str clip=ClipGet();
               if( clip.is())
               {
                  str.insert(edit.cur, clip);
                             edit.cur+=clip.length();
                  changed=true;
               }
            }else
               edit.overwrite^=1;
            return true;
         }break;

         case KB_DEL:
         {
            if(key.shift()) // cut
            {
            cut:
               if(!edit.password)
               {
                           TextSelCopy(str, edit);
                  changed|=TextSelRem (str, edit);
               }
            }else
            {
               if(edit.sel>=0)changed|=TextSelRem(str, edit);else
               if(edit.cur<str.length())
               {
                  if(key.ctrlCmd())
                  {
                     edit.sel=edit.cur;
                     edit.cur++;
                     for(CHAR_TYPE ct=CharType(str[edit.cur-1]); edit.cur<str.length(); edit.cur++)
                     {
                        CHAR_TYPE nt=CharType(str[edit.cur]);
                        if(ct==CHART_SPACE)ct=nt;
                        if(ct!=nt){for(; edit.cur<str.length() && str[edit.cur]==' '; )edit.cur++; break;}
                     }
                     TextSelRem(str, edit);
                  }else
                  {
                     Int num=1; for(; CharFlagFast(str[edit.cur+num])&CHARF_COMBINING; )num++; // del all combining characters after deleted one
                     str.remove(edit.cur, num);
                  }
                  changed=true;
               }
            }
            return true;
         }break;

         case KB_C: if(key.ctrlCmd())goto copy ; break;
         case KB_V: if(key.ctrlCmd())goto paste; break;
         case KB_X: if(key.ctrlCmd())goto cut  ; break;

         case KB_A: if(key.ctrlCmd())
         {
            if(str.length())
            {
               edit.sel=0;
               edit.cur=str.length();
            }
            return true;
         }break;
      }

      if(!key.ctrlCmd() && !key.lalt())if(key.c)
      {
         if(edit.sel>=0)changed|=TextSelRem(str, edit);
         if(edit.overwrite && edit.cur<str.length())
         {
            Int num=0; for(; CharFlagFast(str[edit.cur+1+num])&CHARF_COMBINING; )num++; str.remove(edit.cur+1, num); // del all combining characters after replaced one
               str._d[edit.cur]=key.c;
         }else str.insert(edit.cur, key.c);
         edit.cur++;
         changed=true;
         return true;
      }
   }
   return false;
}
Bool EditText(Str &str, TextEdit &edit, Bool multi_line)
{
   Bool changed=false;
   if(edit.cur>str.length() || edit.cur<0)edit.cur=str.length();
   if(edit.sel>str.length()              )edit.sel=str.length();

   if(Ms.bp(4)) // make 4-th mouse button as Backspace
   {
      Keyboard::Key key; key.clear();
      key.k=KB_BACK;
      Processed(str, edit, multi_line, key, changed);
      Ms.eat(4);
   }

   if(Processed(str, edit, multi_line, Kb.k, changed))
   {
   again:
      Kb.eatKey(); // eat only if processed
      if(C Keyboard::Key *key=Kb.nextKeyPtr()) // get next key without moving it
         if(Processed(str, edit, multi_line, *key, changed)) // if can process this key, then eat it, otherwise, leave it to appear in the next frame so other codes before 'EditText' can try to process it
      {
         Kb.nextInQueue(); // move 'key' to 'Kb.k'
         goto again;
      }
   }

   if(edit.sel==edit.cur)edit.sel=-1;
   return changed;
}
/******************************************************************************/
}
/******************************************************************************/
