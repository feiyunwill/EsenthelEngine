/******************************************************************************/
#include "stdafx.h"
#if IOS

#include "../../../ThirdPartyLibs/begin.h"

#include "../../../ThirdPartyLibs/AdMob/GADAdSize.h"
#include "../../../ThirdPartyLibs/AdMob/GADBannerView.h"
#include "../../../ThirdPartyLibs/AdMob/GADBannerViewDelegate.h"
#include "../../../ThirdPartyLibs/AdMob/GADInterstitial.h"
#include "../../../ThirdPartyLibs/AdMob/GADInterstitialDelegate.h"
#include "../../../ThirdPartyLibs/AdMob/GADRequest.h"
#include "../../../ThirdPartyLibs/AdMob/GADRequestError.h"

#include "../../../ThirdPartyLibs/end.h"

#include "../Platforms/iOS/iOS.h"

static Bool InterstitialLoaded, InterstitialShow, BannerVisible;
static Str8 InterstitialUnitID;

@interface EsenthelBannerView : GADBannerView<GADBannerViewDelegate>
{
}
-(void)adViewDidReceiveAd:(GADBannerView*)bannerView;
-(void)adView:(GADBannerView*)view didFailToReceiveAdWithError:(GADRequestError*)error;
@end
@implementation EsenthelBannerView
   -(void)adViewDidReceiveAd:(GADBannerView*)bannerView                                   {AdMob._banner_state=AdMobClass::DONE ; AdMob._banner_size=Round(Vec2(bannerView.frame.size.width, bannerView.frame.size.height)*ScreenScale); if(BannerVisible)AdMob.bannerShow();}
   -(void)adView:(GADBannerView*)view didFailToReceiveAdWithError:(GADRequestError*)error {AdMob._banner_state=AdMobClass::ERROR;}
@end

@interface EsenthelInterstitial : NSObject<GADInterstitialDelegate>
{
}
-(void)interstitialDidReceiveAd:(GADInterstitial*)interstitial;
-(void)interstitial:(GADInterstitial*)interstitial didFailToReceiveAdWithError:(GADRequestError*)error;
-(void)interstitialDidDismissScreen:(GADInterstitial*)interstitial;
-(void)interstitialWillDismissScreen:(GADInterstitial *)interstitial;
@end
@implementation EsenthelInterstitial
   -(void)interstitialDidReceiveAd:(GADInterstitial*)interstitial                                         {AdMob._fs_state=AdMobClass::DONE ; InterstitialLoaded=true; if(InterstitialShow){InterstitialShow=false; AdMob.fsShow();}}
   -(void)interstitial:(GADInterstitial*)interstitial didFailToReceiveAdWithError:(GADRequestError*)error {AdMob._fs_state=AdMobClass::ERROR;}
   -(void)interstitialDidDismissScreen:(GADInterstitial*)interstitial
   {
      AdMob.fsCreate(InterstitialUnitID); // when ad is closed, it cannot be displayed again, so reload another one
      if(ViewController)[ViewController.view layoutSubviews]; // if the device is rotated when view is displayed, then the render buffers don't get resized automatically, so do it manually
   }
   -(void)interstitialWillDismissScreen:(GADInterstitial*)interstitial {}
@end

#endif
namespace EE{
/******************************************************************************/
AdMobClass AdMob;
#if IOS
static GADRequest           *Request;
static EsenthelBannerView   *Banner;
static GADInterstitial      *Interstitial;
static EsenthelInterstitial *InterstitialDelegate;

static void ResizeAd()
{
   VecI2 old=AdMob.bannerPos(); AdMob._banner_pos.x^=1; // change to force set
   AdMob.bannerPos(old.x, old.y);
}
#endif
/******************************************************************************/
AdMobClass::~AdMobClass()
{
#if IOS
       fsDel(); // release 'Interstitial'
   bannerDel(); // release 'Banner'
   [InterstitialDelegate release]; InterstitialDelegate=null; // release after interstitial because it's using it
#endif
}
AdMobClass::AdMobClass()
{
  _banner_state=_fs_state=NONE;
  _banner_pos.set(0, 1);
  _banner_size.zero();
#if IOS
   Request=[GADRequest request];
   InterstitialDelegate=[[EsenthelInterstitial alloc] init];
#endif
}
AdMobClass& AdMobClass::testMode(Bool on)
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID setAdRequest=jni->GetStaticMethodID(ActivityClass, "setAdRequest", "(Z)V"))
         jni->CallStaticVoidMethod(ActivityClass, setAdRequest, jboolean(on));
#elif IOS
   Request.testDevices=nil;
   if(on)
      if(NSClassFromString(@"ASIdentifierManager"))
         if(NSString *device_id=[[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString])
   {
      Str8  dev_id    =AppleString(device_id);
      UID   dev_id_md5=MD5Mem(dev_id(), dev_id.length());
            dev_id    =TextHexMem(&dev_id_md5, SIZE(dev_id_md5));
      REPAO(dev_id._d)=CaseDown(dev_id._d[i]); // AdMob expects case down
      if(NSStringAuto str=dev_id)Request.testDevices=[NSArray arrayWithObjects: str, nil];
   }
#endif
   return T;
}
/******************************************************************************/
AdMobClass& AdMobClass::bannerDel()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
      if(JMethodID adDel=jni->GetMethodID(ActivityClass, "adDel", "(Z)V"))
         jni->CallVoidMethod(Activity, adDel, jboolean(true));
#elif IOS
   if(Banner)
   {
      Banner.delegate=nil;
      [Banner release];
      Banner=null;
   }
#endif
  _banner_state=NONE;
  _banner_size.zero();
   return T;
}
AdMobClass& AdMobClass::fsDel()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
      if(JMethodID adDel=jni->GetMethodID(ActivityClass, "adDel", "(Z)V"))
         jni->CallVoidMethod(Activity, adDel, jboolean(false));
#elif IOS
   if(Interstitial)
   {
      Interstitial.delegate=nil;
      [Interstitial release];
      Interstitial=null;
      InterstitialLoaded=false;
   }
#endif
  _fs_state=NONE;
   return T;
}
/******************************************************************************/
AdMobClass& AdMobClass::bannerCreate(C Str8 &unit_id, BANNER_TYPE type)
{
   if(unit_id.is())
   {
   #if ANDROID
     _banner_state=LOADING;
     _banner_size .zero();
      JNI jni;
      if(jni && ActivityClass && Activity)
         if(JMethodID adCreate=jni->GetMethodID(ActivityClass, "adCreate", "(Ljava/lang/String;I)V"))
         if(JString j_unit_id=unit_id)
            jni->CallVoidMethod(Activity, adCreate, j_unit_id(), jint(type));
   #elif IOS
      const GADAdSize *size=null;
      switch(type)
      {
         case BANNER          : size=&kGADAdSizeBanner         ; break;
         case MEDIUM_RECTANGLE: size=&kGADAdSizeMediumRectangle; break;
         case FULL_BANNER     : size=&kGADAdSizeFullBanner     ; break;
         case LEADERBOARD     : size=&kGADAdSizeLeaderboard    ; break;
         case SMART_BANNER    : size=&((DirToAxis(App.orientation())==AXIS_X) ? kGADAdSizeSmartBannerLandscape : kGADAdSizeSmartBannerPortrait); break;
      }
      if(size)
      {
         bannerDel();
         if(Request && ViewController)
         {
           _banner_state=LOADING;
            ResizeAdPtr=ResizeAd;
            Banner=[[EsenthelBannerView alloc] initWithAdSize:*size];
            Banner.adUnitID=NSStringAuto(unit_id);
            Banner.rootViewController=ViewController;
            [Banner setDelegate:Banner];
            [Banner loadRequest:Request];
         }
      }
   #endif
   }
   return T;
}
AdMobClass& AdMobClass::fsCreate(C Str8 &unit_id)
{
   if(unit_id.is())
   {
   #if ANDROID
     _fs_state=LOADING;
      JNI jni;
      if(jni && ActivityClass && Activity)
         if(JMethodID adCreate=jni->GetMethodID(ActivityClass, "adCreate", "(Ljava/lang/String;I)V"))
         if(JString j_unit_id=unit_id)
            jni->CallVoidMethod(Activity, adCreate, j_unit_id(), jint(INTERSTITIAL));
   #elif IOS
      fsDel();
      if(Request)
         if(NSStringAuto str=unit_id)
      {
         InterstitialUnitID=unit_id;
        _fs_state=LOADING;
         Interstitial=[[GADInterstitial alloc] initWithAdUnitID:str];
         [Interstitial setDelegate:InterstitialDelegate];
         [Interstitial loadRequest:Request];
      }
   #endif
   }
   return T;
}
/******************************************************************************/
AdMobClass& AdMobClass::bannerVisible(Bool visible)
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
      if(JMethodID adVisible=jni->GetMethodID(ActivityClass, "adVisible", "(ZZ)V"))
         jni->CallVoidMethod(Activity, adVisible, jboolean(true), jboolean(visible));
#elif IOS
   BannerVisible=visible;
   if(ViewController && Banner)
   {
      if(visible)
      {
         [ViewController.view addSubview:Banner];
      }else
      {
         [Banner removeFromSuperview];
      }
   }
#endif
   return T;
}
AdMobClass& AdMobClass::fsVisible(Bool visible)
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
      if(JMethodID adVisible=jni->GetMethodID(ActivityClass, "adVisible", "(ZZ)V"))
         jni->CallVoidMethod(Activity, adVisible, jboolean(false), jboolean(visible));
#elif IOS
   if(visible)
   {
      if(Interstitial && InterstitialLoaded && ViewController)
      {
         InterstitialLoaded=false; // can't reuse again
         [Interstitial presentFromRootViewController:ViewController];
      }else InterstitialShow=true;
   }else
   {
      InterstitialShow=false;
      // there's no method to hide
   }
#endif
   return T;
}
AdMobClass& AdMobClass::bannerShow() {return bannerVisible(true );}
AdMobClass& AdMobClass::bannerHide() {return bannerVisible(false);}

AdMobClass& AdMobClass::fsShow() {return fsVisible(true );}
AdMobClass& AdMobClass::fsHide() {return fsVisible(false);}
/******************************************************************************/
AdMobClass& AdMobClass::bannerPos(Int x, Int y)
{
   if(_banner_pos.x!=x || _banner_pos.y!=y)
   {
     _banner_pos.set(x, y);
   #if ANDROID
      JNI jni;
      if(jni && ActivityClass && Activity)
         if(JMethodID adPos=jni->GetMethodID(ActivityClass, "adPos", "(II)V"))
            jni->CallVoidMethod(Activity, adPos, jint(x), jint(y));
   #elif IOS
      if(Banner)
      {
         CGRect rect=Banner.frame;
         Vec2   res =Vec2(D.resW(), D.resH())/ScreenScale,
                size(rect.size.width, rect.size.height),
                pos ((x<0) ? 0 : (x>0) ? res.x-size.x : (res.x-size.x)*0.5f,
                     (y>0) ? 0 : (y<0) ? res.y-size.y : (res.y-size.y)*0.5f);
         Banner.frame=CGRectMake(pos.x, pos.y, size.x, size.y);
      }
   #endif
   }
   return T;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_adState(JNIEnv *env, jclass clazz, jboolean banner, jint state)
{
   (banner ? AdMob._banner_state : AdMob._fs_state)=AdMobClass::STATE(state);
}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_bannerSize(JNIEnv *env, jclass clazz, jint width, jint height)
{
   AdMob._banner_size.set(width, height);
}
#endif
/******************************************************************************/
