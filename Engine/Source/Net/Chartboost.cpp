/******************************************************************************/
#include "stdafx.h"

#if IOS
#include "../Platforms/iOS/iOS.h"

static void ChartboostCall(ChartboostClass::RESULT result)
{
   if(void (*callback)(ChartboostClass::RESULT)=EE::Chartboost.callback)callback(result);
}

@interface EsenthelChartboostDelegate : NSObject<ChartboostDelegate>
{
   // following methods must use 'AppVolume.muteEx' instead of 'PauseSound/ResumeSound' because it's possible that the app is still running while the video is displayed, and new sounds could be started by the app
}
@end
@implementation EsenthelChartboostDelegate
 //- (BOOL)shouldRequestInterstitial:(CBLocation)location {return YES;}
 //- (BOOL)shouldDisplayInterstitial:(CBLocation)location {return YES;}
   - (void)didCacheInterstitial:(CBLocation)location {ChartboostCall(ChartboostClass::INTERSTITIAL_LOADED);}
   - (void)didFailToLoadInterstitial:(CBLocation)location withError:(CBLoadError)error {ChartboostCall(ChartboostClass::INTERSTITIAL_LOAD_FAIL);}
   - (void)didDisplayInterstitial:(CBLocation)location {ChartboostCall(ChartboostClass::INTERSTITIAL_DISPLAYED);}
   - (void)didDismissInterstitial:(CBLocation)location {AppVolume.muteEx(false); ChartboostCall(ChartboostClass::INTERSTITIAL_CLOSED);}
 //- (void)didCloseInterstitial:(CBLocation)location {AppVolume.muteEx(false); ChartboostCall(ChartboostClass::INTERSTITIAL_CLOSED);}
   - (void)didClickInterstitial:(CBLocation)location {ChartboostCall(ChartboostClass::INTERSTITIAL_CLICKED);}

 //- (BOOL)shouldDisplayRewardedVideo:(CBLocation)location {return YES;}
   - (void)didCacheRewardedVideo:(CBLocation)location {ChartboostCall(ChartboostClass::REWARDED_VIDEO_LOADED);}
   - (void)didFailToLoadRewardedVideo:(CBLocation)location withError:(CBLoadError)error {ChartboostCall(ChartboostClass::REWARDED_VIDEO_LOAD_FAIL);}
   - (void)didDisplayRewardedVideo:(CBLocation)location {AppVolume.muteEx(true); ChartboostCall(ChartboostClass::REWARDED_VIDEO_DISPLAYED);}
   - (void)willDisplayVideo:(CBLocation)location {AppVolume.muteEx(true); /*ChartboostCall(ChartboostClass::REWARDED_VIDEO_DISPLAYED);*/}
   - (void)didDismissRewardedVideo:(CBLocation)location {AppVolume.muteEx(false); ChartboostCall(ChartboostClass::REWARDED_VIDEO_CLOSED);}
 //- (void)didCloseRewardedVideo:(CBLocation)location {AppVolume.muteEx(false); ChartboostCall(ChartboostClass::REWARDED_VIDEO_CLOSED);}
   - (void)didCompleteRewardedVideo:(CBLocation)location withReward:(int)reward {AppVolume.muteEx(false); ChartboostCall(ChartboostClass::REWARDED_VIDEO_COMPLETED);}
   - (void)didClickRewardedVideo:(CBLocation)location {ChartboostCall(ChartboostClass::REWARDED_VIDEO_CLICKED);}
@end
#endif
namespace EE{
/******************************************************************************/
ChartboostClass Chartboost;
/******************************************************************************/
#if IOS
static EsenthelChartboostDelegate *EsenthelChartboostDelegatePtr;
static void InitChartboost()
{
   if(NSString *app_id       =[[[NSBundle mainBundle] infoDictionary] objectForKey:@"ChartboostAppID"       ])if(app_id       .length)
   if(NSString *app_signature=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"ChartboostAppSignature"])if(app_signature.length)
   {
      EsenthelChartboostDelegatePtr=[[EsenthelChartboostDelegate alloc] init];
      [::Chartboost startWithAppId:app_id
                      appSignature:app_signature
                          delegate:EsenthelChartboostDelegatePtr];
   }
}
#endif
ChartboostClass::~ChartboostClass()
{
#if IOS
   [::Chartboost setDelegate:nil];
   [EsenthelChartboostDelegatePtr release]; EsenthelChartboostDelegatePtr=null; // have to release only after we will not use Chartboost anymore, and we've cleared the delegate, if we would release inside 'InitChartboost' then it would get destroyed immediately as chartboost does not increase the ref cound for it
#endif
}
ChartboostClass::ChartboostClass()
{
#if IOS
   InitChartboostPtr=InitChartboost;
#endif
}
/******************************************************************************/
Bool ChartboostClass::visible()C // this does not work on Android, most likely it's not compatible with OpenGL and 'Chartboost.setImpressionsUseActivities'
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID chartboostVisible=jni->GetStaticMethodID(ActivityClass, "chartboostVisible", "()Z"))
      return jni->CallStaticBooleanMethod(ActivityClass, chartboostVisible);
#elif IOS
   return [::Chartboost isAnyViewVisible];
#endif
   return false;
}
/******************************************************************************/
ChartboostClass& ChartboostClass::interstitialLoad()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostInterstitialLoad=jni->GetStaticMethodID(ActivityClass, "chartboostInterstitialLoad", "()V"))
         jni->CallStaticVoidMethod(ActivityClass, chartboostInterstitialLoad);
#elif IOS
   [::Chartboost cacheInterstitial:CBLocationDefault];
#endif
   return T;
}
ChartboostClass& ChartboostClass::interstitialShow()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostInterstitialShow=jni->GetStaticMethodID(ActivityClass, "chartboostInterstitialShow", "()V"))
         jni->CallStaticVoidMethod(ActivityClass, chartboostInterstitialShow);
#elif IOS
   [::Chartboost showInterstitial:CBLocationDefault];
#endif
   return T;
}
Bool ChartboostClass::interstitialAvailable()C
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostInterstitialAvailable=jni->GetStaticMethodID(ActivityClass, "chartboostInterstitialAvailable", "()Z"))
         return jni->CallStaticBooleanMethod(ActivityClass, chartboostInterstitialAvailable);
#elif IOS
   return [::Chartboost hasInterstitial:CBLocationDefault];
#endif
   return false;
}
/******************************************************************************/
ChartboostClass& ChartboostClass::rewardedVideoLoad()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostRewardedVideoLoad=jni->GetStaticMethodID(ActivityClass, "chartboostRewardedVideoLoad", "()V"))
         jni->CallStaticVoidMethod(ActivityClass, chartboostRewardedVideoLoad);
#elif IOS
   [::Chartboost cacheRewardedVideo:CBLocationDefault];
#endif
   return T;
}
ChartboostClass& ChartboostClass::rewardedVideoShow()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostRewardedVideoShow=jni->GetStaticMethodID(ActivityClass, "chartboostRewardedVideoShow", "()V"))
         jni->CallStaticVoidMethod(ActivityClass, chartboostRewardedVideoShow);
#elif IOS
   [::Chartboost showRewardedVideo:CBLocationDefault];
#endif
   return T;
}
Bool ChartboostClass::rewardedVideoAvailable()C
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID chartboostRewardedVideoAvailable=jni->GetStaticMethodID(ActivityClass, "chartboostRewardedVideoAvailable", "()Z"))
         return jni->CallStaticBooleanMethod(ActivityClass, chartboostRewardedVideoAvailable);
#elif IOS
   return [::Chartboost hasRewardedVideo:CBLocationDefault];
#endif
   return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_chartboost(JNIEnv *env, jclass clazz, jint result)
{
   if(void (*callback)(ChartboostClass::RESULT)=Chartboost.callback)callback(ChartboostClass::RESULT(result));
}
#endif
/******************************************************************************/
