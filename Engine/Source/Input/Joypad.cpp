/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
CChar* Joypad::_button_name[32];
MemtN<Joypad, 4> Joypads;
/******************************************************************************/
#if MAC
struct MacJoypad
{
   struct Elm
   {
      enum TYPE : Byte
      {
         PAD   ,
         BUTTON,
         AXIS  ,
      };
      TYPE               type;
      Int                index;
      IOHIDElementCookie cookie;
      Int                avg, max; // button_on=(val>avg);
      Flt                mul, add;

      void setPad   (C IOHIDElementCookie &cookie                    , Int max) {T.type=PAD   ; T.cookie=cookie; T.max  =max+1; T.mul=-PI2/T.max; T.add=PI_2;}
      void setButton(C IOHIDElementCookie &cookie, Int index, Int min, Int max) {T.type=BUTTON; T.cookie=cookie; T.index=index; T.avg=(min+max)/2;}
      void setAxis  (C IOHIDElementCookie &cookie, Int index, Int min, Int max) {T.type=AXIS  ; T.cookie=cookie; T.index=index; T.mul=2.0f/(max-min); T.add=-1-min*T.mul; if(index&1){CHS(mul); CHS(add);}} // change sign for vertical
   };

   static Int Compare(C Elm &a, C Elm                &b) {return ::Compare(UIntPtr(a.cookie), UIntPtr(b.cookie));}
   static Int Compare(C Elm &a, C IOHIDElementCookie &b) {return ::Compare(UIntPtr(a.cookie), UIntPtr(b       ));}

   Mems<Elm> elms;
   Byte      button[32];

   void         zero() {Zero(button);}
   MacJoypad() {zero();}

   NO_COPY_CONSTRUCTOR(MacJoypad);
};
static MemtN<MacJoypad, 4> MacJoypads;
static IOHIDManagerRef HidManager;
static UInt JoypadsID;
/******************************************************************************/
static NSMutableDictionary* JoypadCriteria(UInt32 inUsagePage, UInt32 inUsage)
{
   NSMutableDictionary* dict=[[NSMutableDictionary alloc] init];
   [dict setObject: [NSNumber numberWithInt: inUsagePage] forKey: (NSString*)CFSTR(kIOHIDDeviceUsagePageKey)];
   [dict setObject: [NSNumber numberWithInt: inUsage    ] forKey: (NSString*)CFSTR(kIOHIDDeviceUsageKey    )];
   return dict;
} 
static void JoypadAdded(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef device)
{
   Memt<MacJoypad::Elm> elms;
   Int                  buttons=0, axes=0;
   NSArray             *elements=(NSArray*)IOHIDDeviceCopyMatchingElements(device, null, kIOHIDOptionsTypeNone);
   FREP([elements count]) // process in order
   {
      IOHIDElementRef element=(IOHIDElementRef)[elements objectAtIndex: i];
      Int type =IOHIDElementGetType       (element),
          usage=IOHIDElementGetUsage      (element),
          page =IOHIDElementGetUsagePage  (element),
           min =IOHIDElementGetPhysicalMin(element),
           max =IOHIDElementGetPhysicalMax(element),
          lmin =IOHIDElementGetLogicalMin (element),
          lmax =IOHIDElementGetLogicalMax (element);
      IOHIDElementCookie cookie=IOHIDElementGetCookie(element);
      //CFStringRef elm_name=IOHIDElementGetName(element); NSLog(@"%@", (NSString*)elm_name);

      if(type==kIOHIDElementTypeInput_Misc || type==kIOHIDElementTypeInput_Axis || type==kIOHIDElementTypeInput_Button)
      {
         if((max-min==1) || page==kHIDPage_Button || type==kIOHIDElementTypeInput_Button){if(InRange(buttons, MEMBER(MacJoypad, button)))elms.New().setButton(cookie, buttons++, min, max);}else
         if(usage>=0x30 && usage<0x36                                                   )                                                elms.New().setAxis  (cookie, axes   ++, min, max); else
         if(usage==0x39                                                                 )                                                elms.New().setPad   (cookie,                lmax);
      }
   }

   if(elms.elms())
   {
      elms.sort(MacJoypad::Compare); // sort so 'binaryFind' can be used later

      NSString *name  =(NSString*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey     )); // do not release this !!
    //NSString *serial=(NSString*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey)); // do not release this ? this was null on "Logitech Rumblepad 2"
	   Int    vendorId=[(NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey    )) intValue];
	   Int   productId=[(NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey   )) intValue];

         Joypad & jp=   Joypads.New();
      MacJoypad &mjp=MacJoypads.New();
       jp._id  =JoypadsID++;
       jp._name=name;
       jp._did =device;
      mjp. elms=elms;
   }
}
static void JoypadRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef device)
{
   REPA(Joypads)if(Joypads[i]._did==device)
   {
         Joypads.remove(i, true);
      MacJoypads.remove(i, true);
   }
}
static void JoypadAction(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef value)
{
   IOHIDElementRef element=IOHIDValueGetElement (value  );
   IOHIDDeviceRef  device =IOHIDElementGetDevice(element); // or IOHIDQueueGetDevice((IOHIDQueueRef)inSender);
   REPA(Joypads)if(Joypads[i]._did==device)
   {
         Joypad & jp=   Joypads[i];
      MacJoypad &mjp=MacJoypads[i];
      if(MacJoypad::Elm *elm=mjp.elms.binaryFind(IOHIDElementGetCookie(element), MacJoypad::Compare))
      {
         Int val=IOHIDValueGetIntegerValue(value);
         switch(elm->type)
         {
            case MacJoypad::Elm::PAD:
            {
               if(InRange(val, elm->max))
               {
                  CosSin(jp.dir.x, jp.dir.y, val*elm->mul+elm->add);
               }else jp.dir.zero();
            }break;

            case MacJoypad::Elm::BUTTON:
            {
               mjp.button[elm->index]=(val>elm->avg);
            }break;

            case MacJoypad::Elm::AXIS:
            {
               switch(elm->index)
               {
                  case 0: jp.dir_a[0].x=val*elm->mul+elm->add; break;
                  case 1: jp.dir_a[0].y=val*elm->mul+elm->add; break;
                  case 2: jp.dir_a[1].x=val*elm->mul+elm->add; break;
                  case 3: jp.dir_a[1].y=val*elm->mul+elm->add; break;
               }
            }break;
         }
      }
      break;
   }
}
#endif
/******************************************************************************/
Joypad::~Joypad()
{
#if WINDOWS_OLD
   if(_did){_did->Unacquire(); RELEASE(_effect); RELEASE(_did);}
#endif
}
Joypad::Joypad()
{
  _vibration_axes=_xinput1=_offset_x=_offset_y=_connected=0;
  _id=0;
  _did=null;
  _effect=null;
   zero();
}
Str Joypad::buttonName(Int x)C
{
   if(InRange(x, _button_name))return _button_name[x];
   return S;
}
/******************************************************************************/
Bool Joypad::supportsVibrations()C
{
   return _xinput1 || _effect!=null;
}
Int Joypad::index()C {return Joypads.index(this);}
/******************************************************************************/
Joypad& Joypad::vibration(C Vec2 &force)
{
#if WINDOWS
   if(_xinput1)
   {
      XINPUT_VIBRATION vibration;
      vibration.wLeftMotorSpeed =RoundU(Sat(Abs(force.x))*0xFFFF);
      vibration.wRightMotorSpeed=RoundU(Sat(Abs(force.y))*0xFFFF);
      XInputSetState(_xinput1-1, &vibration);
   }
#if WINDOWS_OLD
   else
   if(_effect && _vibration_axes>=1 && _vibration_axes<=2)
   {
      LONG rglDirection[2]={0, 0};
      DICONSTANTFORCE cf;

      if(_vibration_axes==1)
      {
         cf.lMagnitude=RoundU(Min(force.length(), 1.0f)*DI_FFNOMINALMAX);
      }else
      {
         rglDirection[0]=Round (Mid(force.x, -1.0f, 1.0f)*DI_FFNOMINALMAX);
         rglDirection[1]=Round (Mid(force.y, -1.0f, 1.0f)*DI_FFNOMINALMAX);
         cf.lMagnitude  =RoundU(Min(force.length(), 1.0f)*DI_FFNOMINALMAX);
      }

      DIEFFECT eff; Zero(eff);
      eff.dwSize=SIZE(DIEFFECT);
      eff.dwFlags=DIEFF_CARTESIAN|DIEFF_OBJECTOFFSETS;
      eff.cAxes=_vibration_axes;
      eff.rglDirection=rglDirection;
      eff.lpEnvelope=0;
      eff. cbTypeSpecificParams=SIZE(DICONSTANTFORCE);
      eff.lpvTypeSpecificParams=&cf;
      eff.dwStartDelay=0;

     _effect->SetParameters(&eff, DIEP_DIRECTION|DIEP_TYPESPECIFICPARAMS|DIEP_START);
   }
#endif
#endif
   return T;
}
/******************************************************************************/
void Joypad::zero()
{
   Zero(_button);
   REPAO(_last_t)=-FLT_MAX;
         dir     .zero();
   REPAO(dir_a  ).zero();
   REPAO(dir_an ).zero();
   REPAO(trigger)=0;
}
void Joypad::clear()
{
   REPAO(_button)&=~BS_NOT_ON;
}
void Joypad::update(C Byte *on, Int elms)
{
   MIN(elms, Elms(_button));
   REP(elms)if((on[i]!=0)!=ButtonOn(_button[i]))if(on[i])push(i);else release(i);
}
void Joypad::update()
{
#if WINDOWS
   if(_xinput1)
   {
      XINPUT_STATE state;
      if(XInputGetState(_xinput1-1, &state)==ERROR_SUCCESS)
      {
         // buttons
         Byte x_button[]=
         {
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_A                ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_B                ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_X                ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_Y                ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_LEFT_SHOULDER    ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_RIGHT_SHOULDER   ),
                     state.Gamepad. bLeftTrigger>=XINPUT_GAMEPAD_TRIGGER_THRESHOLD ,
                     state.Gamepad.bRightTrigger>=XINPUT_GAMEPAD_TRIGGER_THRESHOLD ,
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_LEFT_THUMB       ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_RIGHT_THUMB      ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_START            ),
            FlagTest(state.Gamepad.wButtons     , XINPUT_GAMEPAD_BACK             ),
         };
         ASSERT(ELMS(x_button)<ELMS(T._button));
         update(x_button, Elms(x_button));

         // digital pad
         dir.x=FlagTest(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT)-FlagTest(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
         dir.y=FlagTest(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP   )-FlagTest(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
         Flt l=dir.length(); if(l>1)dir/=l;

         dir_a[0].x=state.Gamepad.sThumbLX/32768.0f;
         dir_a[0].y=state.Gamepad.sThumbLY/32768.0f;
         dir_a[1].x=state.Gamepad.sThumbRX/32768.0f;
         dir_a[1].y=state.Gamepad.sThumbRY/32768.0f;

         trigger[0]=state.Gamepad.bLeftTrigger /255.0f;
         trigger[1]=state.Gamepad.bRightTrigger/255.0f;
      }else zero();
   }
#if WINDOWS_OLD
   else
   if(_did->Poll())
   {
      // get data
      DIJOYSTATE dijs;
      if(!OK(_did->GetDeviceState(SIZE(dijs), &dijs)))
      {
         if(App.active())acquire(true); Zero(dijs);
         dijs.rgdwPOV[0]=UINT_MAX;
         dijs.lX          =32768;
         dijs.lY          =32768;
         dijs.lZ          =32768;
         dijs.lRx         =32768;
         dijs.lRy         =32768;
         dijs.lRz         =32768;
         dijs.rglSlider[0]=32768;
         dijs.rglSlider[1]=32768;
      }

      // process data
      ASSERT(ELMS(T._button)==ELMS(dijs.rgbButtons));
      update(dijs.rgbButtons, Elms(dijs.rgbButtons));

      switch(dijs.rgdwPOV[0])
      {
         case UINT_MAX: dir.zero(); break;
         case        0: dir.set( 0,  1); break;
         case     9000: dir.set( 1,  0); break;
         case    18000: dir.set( 0, -1); break;
         case    27000: dir.set(-1,  0); break;
         default      : CosSin(dir.x, dir.y, PI_2-DegToRad(dijs.rgdwPOV[0]/100.0f)); break;
      }

      const Flt mul=1.0f/32768;
      dir_a[0].x= (dijs.lX-32768)*mul;
      dir_a[0].y=-(dijs.lY-32768)*mul;
      if(_offset_x && _offset_y)
      {
         ASSERT(SIZE(dijs.lZ)==SIZE(Int));
         dir_a[1].x= (*(Int*)(((Byte*)&dijs)+_offset_x)-32768)*mul;
         dir_a[1].y=-(*(Int*)(((Byte*)&dijs)+_offset_y)-32768)*mul;
      }

      trigger[0]=(dijs.rglSlider[0]-32768)*mul;
      trigger[1]=(dijs.rglSlider[1]-32768)*mul;
   }
#endif
#elif MAC
   Int index=T.index(); if(InRange(index, MacJoypads))
   {
    C MacJoypad &mjp=MacJoypads[index];
      ASSERT(ELMS(T._button)==ELMS(mjp.button));
      update(mjp.button, Elms(mjp.button));
   }
#else
#endif
   REPA(dir_an)
   {
      dir_an[i]=dir_a[i];
      Flt div=Max(Abs(dir_an[i].x), Abs(dir_an[i].y));
      if( div>EPS)dir_an[i]/=dir_an[i].length()/div;
   }
}
void Joypad::push(Byte b)
{
   if(InRange(b, _button) && !(_button[b]&BS_ON))
   {
      Int device=index(); if(device>=0)InputCombo.add(InputButton(INPUT_JOYPAD, b, device));
     _button[b]|=BS_PUSHED|BS_ON;
      if(Time.appTime()-_last_t[b]<=DoubleClickTime+Time.ad())
      {
        _button[b]|=BS_DOUBLE;
        _last_t[b] =-FLT_MAX;
      }else
      {
        _last_t[b]=Time.appTime();
      }
   }
}
void Joypad::release(Byte b)
{
   if(InRange(b, _button) && (_button[b]&BS_ON))
   {
     _button[b]&=~BS_ON;
     _button[b]|= BS_RELEASED;
   }
}
/******************************************************************************/
void Joypad::acquire(Bool on)
{
#if WINDOWS_OLD
   if(_did){if(on){_did->Acquire(); if(_effect)_effect->Start(1, 0);}else _did->Unacquire();}
#endif
}
/******************************************************************************/
Joypad* FindJoypad(UInt id)
{
   REPA(Joypads)if(Joypads[i].id()==id)return &Joypads[i];
   return null;
}
static Joypad& GetJoypad(UInt id, Bool &added)
{
   added=false;
   Joypad *joypad=FindJoypad(id); if(!joypad){added=true; joypad=&Joypads.New(); joypad->_id=id;} joypad->_connected=true;
   return *joypad;
}
#if WINDOWS_OLD
static Bool IsXInputDevice(C GUID &pGuidProductFromDirectInput)
{
   IWbemLocator*         pIWbemLocator =null;
   IEnumWbemClassObject* pEnumDevices  =null;
   IWbemClassObject*     pDevices[20]  ={0};
   IWbemServices*        pIWbemServices=null;
   BSTR                  bstrNamespace =null;
   BSTR                  bstrDeviceID  =null;
   BSTR                  bstrClassName =null;
   DWORD                 uReturned     =0;
   bool                  bIsXinputDevice=false;
   UINT                  iDevice       =0;
   VARIANT               var;
   HRESULT               hr;

   // CoInit if needed
   hr=CoInitialize(null);
   bool bCleanupCOM=SUCCEEDED(hr);

   // Create WMI
   hr=CoCreateInstance(__uuidof(WbemLocator), null, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&pIWbemLocator);
   if(FAILED(hr) || pIWbemLocator==null)goto LCleanup;

   bstrNamespace=SysAllocString(L"\\\\.\\root\\cimv2"); if(!bstrNamespace)goto LCleanup;
   bstrClassName=SysAllocString(L"Win32_PNPEntity");    if(!bstrClassName)goto LCleanup;
   bstrDeviceID =SysAllocString(L"DeviceID");           if(!bstrDeviceID )goto LCleanup;

   // Connect to WMI
   hr=pIWbemLocator->ConnectServer(bstrNamespace, null, null, 0L, 0L, null, null, &pIWbemServices);
   if(FAILED(hr) || pIWbemServices==null)goto LCleanup;

   // Switch security level to IMPERSONATE
   CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, null, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, null, EOAC_NONE);

   hr=pIWbemServices->CreateInstanceEnum(bstrClassName, 0, null, &pEnumDevices);
   if(FAILED(hr) || pEnumDevices==null)goto LCleanup;

   // Loop over all devices
   for(;;)
   {
      hr=pEnumDevices->Next(10000, Elms(pDevices), pDevices, &uReturned); // Get 20 at a time
      if(FAILED(hr))goto LCleanup;
      if(uReturned==0)break;

      FREP(uReturned) // For each device, get its device ID
      {
         hr=pDevices[i]->Get(bstrDeviceID, 0L, &var, null, null);
         if(SUCCEEDED(hr) && var.vt==VT_BSTR && var.bstrVal!=null)
         {
            if(wcsstr(var.bstrVal, L"IG_")) // Check if the device ID contains "IG_".  If it does, then it's an XInput device
            {
               DWORD dwPid=0, dwVid=0; // If it does, then get the VID/PID from var.bstrVal
            #pragma warning(push)
            #pragma warning(disable:4996)
               WCHAR *strVid=wcsstr(var.bstrVal, L"VID_"); if(strVid && swscanf(strVid, L"VID_%4X", &dwVid)!=1)dwVid=0;
               WCHAR *strPid=wcsstr(var.bstrVal, L"PID_"); if(strPid && swscanf(strPid, L"PID_%4X", &dwPid)!=1)dwPid=0;
            #pragma warning(pop)

               if(MAKELONG(dwVid, dwPid)==pGuidProductFromDirectInput.Data1) // Compare the VID/PID to the DInput device
               {
                  bIsXinputDevice=true;
                  goto LCleanup;
               }
            }
         }   
         RELEASE(pDevices[i]);
      }
   }

LCleanup:
   if(bstrNamespace)SysFreeString(bstrNamespace);
   if(bstrDeviceID )SysFreeString(bstrDeviceID );
   if(bstrClassName)SysFreeString(bstrClassName);
   FREPA(pDevices)RELEASE(pDevices[i]);
   RELEASE(pEnumDevices  );
   RELEASE(pIWbemLocator );
   RELEASE(pIWbemServices);

   if(bCleanupCOM)CoUninitialize();

   return bIsXinputDevice;
}
static BOOL CALLBACK EnumAxes(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *user)
{
   Joypad &joypad=*(Joypad*)user;
   if(pdidoi->dwFlags&DIDOI_FFACTUATOR)joypad._vibration_axes++;

   // Logitech RumblePad 2 uses: (x0=lX, y0=lY, x1=lZ, y1=lRz)
   Int offset=0;
   if(pdidoi->guidType==GUID_ZAxis )offset=OFFSET(DIJOYSTATE, lZ );else
   if(pdidoi->guidType==GUID_RxAxis)offset=OFFSET(DIJOYSTATE, lRx);else
   if(pdidoi->guidType==GUID_RyAxis)offset=OFFSET(DIJOYSTATE, lRy);else
   if(pdidoi->guidType==GUID_RzAxis)offset=OFFSET(DIJOYSTATE, lRz);

   if(offset)
   {
      if(!joypad._offset_x       )joypad._offset_x=offset;else
      if( joypad._offset_x<offset)joypad._offset_y=offset;else // X axis is assumed to be specified before Y axis
      {
         joypad._offset_y=joypad._offset_x;
         joypad._offset_x=        offset;
      }
   }

   return DIENUM_CONTINUE;
}
static BOOL CALLBACK EnumJoypads(const DIDEVICEINSTANCE *DIDevInst, void*)
{
   if(!IsXInputDevice(DIDevInst->guidProduct)) // x controllers are listed elsewhere
   {
      UInt id=0; ASSERT(SIZE(DIDevInst->guidInstance)==SIZE(UID)); C UID &uid=(UID&)DIDevInst->guidInstance; REPA(uid.i)id^=uid.i[i];
      Bool added; Joypad &joypad=GetJoypad(id, added);
      if(  added)
      {
         IDirectInputDevice8 *did=null;
         if(OK(InputDevices.DI->CreateDevice(DIDevInst->guidInstance, &did, null)))
         if(OK(did->SetDataFormat      (&c_dfDIJoystick)))
         if(OK(did->SetCooperativeLevel(App.Hwnd(), DISCL_EXCLUSIVE|DISCL_FOREGROUND)))
         {
            Swap(joypad._did, did);
            joypad._name=DIDevInst->tszProductName;

            // disable auto centering ?
            DIPROPDWORD dipdw; Zero(dipdw);
            dipdw.diph.dwSize      =SIZE(DIPROPDWORD );
            dipdw.diph.dwHeaderSize=SIZE(DIPROPHEADER);
            dipdw.diph.dwObj       =0;
            dipdw.diph.dwHow       =DIPH_DEVICE;
            dipdw.dwData           =FALSE;
            OK(joypad._did->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph));

            // enumerate ff axes
                joypad._did->EnumObjects(EnumAxes, &joypad, DIDFT_AXIS);
            MIN(joypad._vibration_axes, 2);

            // create ff effect
            if(joypad._vibration_axes)
            {
               DWORD rgdwAxes    [2]={DIJOFS_X, DIJOFS_Y};
               LONG  rglDirection[2]={0, 0};
               DICONSTANTFORCE cf   ={0};

               DIEFFECT eff; Zero(eff);
               eff.dwSize=SIZE(DIEFFECT);
               eff.dwFlags=DIEFF_CARTESIAN|DIEFF_OBJECTOFFSETS;
               eff.dwDuration=INFINITE;
               eff.dwSamplePeriod=0;
               eff.dwGain=DI_FFNOMINALMAX;
               eff.dwTriggerButton=DIEB_NOTRIGGER;
               eff.dwTriggerRepeatInterval=0;
               eff.cAxes=joypad._vibration_axes;
               eff.rgdwAxes=rgdwAxes;
               eff.rglDirection=rglDirection;
               eff.lpEnvelope=0;
               eff.cbTypeSpecificParams=SIZE(DICONSTANTFORCE);
               eff.lpvTypeSpecificParams=&cf;
               eff.dwStartDelay=0;

               // Create the prepared effect
               joypad._did->CreateEffect(GUID_ConstantForce, &eff, &joypad._effect, null);
            }
         }
         if(!joypad._did)Joypads.removeData(&joypad); // if failed to create it then remove it
         RELEASE(did);
      }
   }
   return DIENUM_CONTINUE;
}
#endif
/******************************************************************************/
static Int CompareJoypad(C Joypad &a, C Joypad &b) {return Compare(a.id(), b.id());}
void ListJoypads()
{
#if WINDOWS
   REPAO(Joypads)._connected=false; // assume that all are disconnected

   FREP(4) // XInput supports only 4 controllers (process in order)
   {
      XINPUT_STATE state; if(XInputGetState(i, &state)==ERROR_SUCCESS) // if returned valid input
      {
         Bool added; Joypad &joypad=GetJoypad(i, added); // index is used for the ID for XInput controllers
         if(  added)
         {
            joypad._xinput1=i+1;
            joypad._name   ="X Controller";
         }
      }
   }
   #if WINDOWS_OLD
      InputDevices.DI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoypads, null, DIEDFL_ATTACHEDONLY/*|DIEDFL_FORCEFEEDBACK*/); // this would enumerate only devices with FF
   #endif

   #if WINDOWS_NEW
      //Windows::Gaming::Input::Gamepad::Gamepads;
      //Windows::Gaming::Input::Gamepad::GetCurrentReading;
      //Windows::Gaming::Input::Gamepad::Vibration;
   #endif

   REPA(Joypads)if(!Joypads[i]._connected)Joypads.remove(i); // remove disconnected joypads
   Joypads.sort(CompareJoypad); // sort remaining by their ID
#elif MAC
	if(HidManager=IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone))
	{
      NSMutableDictionary *criteria[]=
      {
         JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick),
         JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad),
	      JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController),
      };
      NSArray *criteria_array=[NSArray arrayWithObjects: criteria[0], criteria[1], criteria[2], __null];
      IOHIDManagerSetDeviceMatchingMultiple(HidManager, (CFArrayRef)criteria_array);
      REPA(criteria)[criteria[i] release];

      IOHIDManagerScheduleWithRunLoop(HidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
      IOReturn hid_open=IOHIDManagerOpen(HidManager, kIOHIDOptionsTypeNone);

      IOHIDManagerRegisterDeviceMatchingCallback(HidManager, JoypadAdded  , null);
      IOHIDManagerRegisterDeviceRemovalCallback (HidManager, JoypadRemoved, null);
      IOHIDManagerRegisterInputValueCallback    (HidManager, JoypadAction , null);
   }
#endif
}
void InitJoypads()
{
   if(LogInit)LogN("InitJoypads");
   Joypad::_button_name[ 0]=u"Joypad1";
   Joypad::_button_name[ 1]=u"Joypad2";
   Joypad::_button_name[ 2]=u"Joypad3";
   Joypad::_button_name[ 3]=u"Joypad4";
   Joypad::_button_name[ 4]=u"Joypad5";
   Joypad::_button_name[ 5]=u"Joypad6";
   Joypad::_button_name[ 6]=u"Joypad7";
   Joypad::_button_name[ 7]=u"Joypad8";
   Joypad::_button_name[ 8]=u"Joypad9";
   Joypad::_button_name[ 9]=u"Joypad10";
   Joypad::_button_name[10]=u"Joypad11";
   Joypad::_button_name[11]=u"Joypad12";
   Joypad::_button_name[12]=u"Joypad13";
   Joypad::_button_name[13]=u"Joypad14";
   Joypad::_button_name[14]=u"Joypad15";
   Joypad::_button_name[15]=u"Joypad16";
   Joypad::_button_name[16]=u"Joypad17";
   Joypad::_button_name[17]=u"Joypad18";
   Joypad::_button_name[18]=u"Joypad19";
   Joypad::_button_name[19]=u"Joypad20";
   Joypad::_button_name[20]=u"Joypad21";
   Joypad::_button_name[21]=u"Joypad22";
   Joypad::_button_name[22]=u"Joypad23";
   Joypad::_button_name[23]=u"Joypad24";
   Joypad::_button_name[24]=u"Joypad25";
   Joypad::_button_name[25]=u"Joypad26";
   Joypad::_button_name[26]=u"Joypad27";
   Joypad::_button_name[27]=u"Joypad28";
   Joypad::_button_name[28]=u"Joypad29";
   Joypad::_button_name[29]=u"Joypad30";
   Joypad::_button_name[30]=u"Joypad31";
   Joypad::_button_name[31]=u"Joypad32";

   ListJoypads();
}
void ShutJoypads()
{
      Joypads.del();
#if MAC
   MacJoypads.del();
   if(HidManager)
   {
      IOHIDManagerClose(HidManager, kIOHIDOptionsTypeNone);
      CFRelease(HidManager); HidManager=null;
   }
#endif
}
/******************************************************************************/
}
/******************************************************************************/
