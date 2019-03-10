/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#if WINDOWS_OLD
static Bool   tried, created;
static HMIXER handle;
static UInt   ctrl_id_vol,
              ctrl_id_mute;
#else
#define MUTE_USER 0x1
#define MUTE_EX   0x2
static Flt  Volume=1;
static Byte Mute  =0;
static void SetMute() {SoundVolume.update();}
#endif
/******************************************************************************/
AppVolumeClass AppVolume;
/******************************************************************************/
AppVolumeClass::AppVolumeClass() {}

void AppVolumeClass::del()
{
#if WINDOWS_OLD
   if(tried)
   {
      mixerClose(handle);
      handle=null;
   }
#endif
}
Bool AppVolumeClass::create()
{
#if WINDOWS_OLD
   if(tried)return created;
      tried=true;

   if(!mixerOpen(&handle, MIXER_OBJECTF_MIXER, 0, 0, 0))
   {
      MIXERLINE ml; Zero(ml);
      ml.cbStruct=SIZE(MIXERLINE);
      ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
      if(!mixerGetLineInfo((HMIXEROBJ)handle, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE))
      {
         MIXERCONTROL      mc ; Zero(mc );
         MIXERLINECONTROLS mlc; Zero(mlc);
         mlc.cbmxctrl=SIZE(MIXERCONTROL);
         mlc.cbStruct=SIZE(MIXERLINECONTROLS);
         mlc.dwLineID=ml.dwLineID;
         mlc.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME;
         mlc.cControls=1;
         mlc.pamxctrl=&mc;
         if(!mixerGetLineControls((HMIXEROBJ)handle, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE))
         {
            ctrl_id_vol=mc.dwControlID;
         
            Zero(mc );
            Zero(mlc);
            mlc.cbmxctrl=SIZE(MIXERCONTROL);
            mlc.cbStruct=SIZE(MIXERLINECONTROLS);
            mlc.dwLineID=ml.dwLineID;
            mlc.dwControlType=MIXERCONTROL_CONTROLTYPE_MUTE;
            mlc.cControls=1;
            mlc.pamxctrl=&mc;
            if(!mixerGetLineControls((HMIXEROBJ)handle, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE))
            {
               ctrl_id_mute=mc.dwControlID;
               return created=true;
            }
         }
      }
   }
   return false;
#else
   return true;
#endif
}
/******************************************************************************/
void AppVolumeClass::volume(Flt volume)
{
   SAT(volume);
#if WINDOWS_OLD
   if(create())
   {
      MIXERCONTROLDETAILS          mcd ; Zero(mcd );
      MIXERCONTROLDETAILS_UNSIGNED mcdu; Zero(mcdu);
      mcd.cbStruct =SIZE(MIXERCONTROLDETAILS);
      mcd.cbDetails=SIZE(MIXERCONTROLDETAILS_UNSIGNED);
      mcd.dwControlID=ctrl_id_vol;
      mcd.paDetails=&mcdu;
      mcd.cChannels=1;
      mcdu.dwValue =RoundU(0xFFFF*volume);
      mixerSetControlDetails((HMIXEROBJ)handle, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
   }
#else
   if(Volume!=volume)
   {
      Volume =volume;
      SoundVolume.update();
   }
#endif
}
Flt AppVolumeClass::volume()
{
#if WINDOWS_OLD
   if(create())
   {
      MIXERCONTROLDETAILS          mcd ; Zero(mcd );
      MIXERCONTROLDETAILS_UNSIGNED mcdu; Zero(mcdu);
      mcd.cbStruct =SIZE(MIXERCONTROLDETAILS);
      mcd.cbDetails=SIZE(MIXERCONTROLDETAILS_UNSIGNED);
      mcd.dwControlID=ctrl_id_vol;
      mcd.paDetails=&mcdu;
      mcd.cChannels=1;
      mixerGetControlDetails((HMIXEROBJ)handle, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
      return mcdu.dwValue/Flt(0xFFFF);
   }
   return 0;
#else
   return Volume;
#endif
}
/******************************************************************************/
void AppVolumeClass::mute(Bool mute)
{  
#if WINDOWS_OLD
   if(create())
   {
      MIXERCONTROLDETAILS         mcd ; Zero(mcd );
      MIXERCONTROLDETAILS_BOOLEAN mcdb; Zero(mcdb);
      mcd.cbStruct =SIZE(MIXERCONTROLDETAILS);
      mcd.cbDetails=SIZE(MIXERCONTROLDETAILS_BOOLEAN);
      mcd.dwControlID=ctrl_id_mute;
      mcd.paDetails=&mcdb;
      mcd.cChannels=1;
      mcdb.fValue  =mute;
      mixerSetControlDetails((HMIXEROBJ)handle, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
   }
#else
   if(FlagTest(Mute, MUTE_USER)!=mute){Mute^=MUTE_USER; SetMute();}
#endif
}
Bool AppVolumeClass::mute()
{  
#if WINDOWS_OLD
   if(create())
   {
      MIXERCONTROLDETAILS         mcd ; Zero(mcd );
      MIXERCONTROLDETAILS_BOOLEAN mcdb; Zero(mcdb);
      mcd.cbStruct =SIZE(MIXERCONTROLDETAILS);
      mcd.cbDetails=SIZE(MIXERCONTROLDETAILS_BOOLEAN);
      mcd.dwControlID=ctrl_id_mute;
      mcd.paDetails=&mcdb;
      mcd.cChannels=1;
      mixerGetControlDetails((HMIXEROBJ)handle, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
      return mcdb.fValue!=0;
   }
   return false;
#else
   return FlagTest(Mute, MUTE_USER);
#endif
}
#if !WINDOWS_OLD
void AppVolumeClass::muteEx   (Bool mute)  {if(FlagTest(Mute, MUTE_EX)!=mute){Mute^=MUTE_EX; SetMute();}}
Bool AppVolumeClass::muteEx   (         )C {return FlagTest(Mute, MUTE_EX);}
Bool AppVolumeClass::muteFinal(         )C {return Mute!=0;}
#endif
/******************************************************************************/
}
/******************************************************************************/
