/******************************************************************************

   Use 'AdMob' or 'Chartboost' to display ads in your application (for Android and iOS).

   Usage of 'Chartboost' requires specifying following elements in the Application properties:
      -Chartboost App ID
      -Chartboost App Signature

/******************************************************************************/
struct AdMobClass
{
#if EE_PRIVATE
   // !! these enums must be equal to "EsenthelActivity.java" !!
#endif
   enum BANNER_TYPE : Byte
   {                    // Size   | Description         | Availability
      BANNER          , // 320x50 , Standard Banner     , Phones and Tablets
      MEDIUM_RECTANGLE, // 300x250, IAB Medium Rectangle, Tablets
      FULL_BANNER     , // 468x60 , IAB Full-Size Banner, Tablets
      LEADERBOARD     , // 728x90 , IAB Leaderboard     , Tablets
      SMART_BANNER    , // Varies , Smart Banner        , Phones and Tablets, https://firebase.google.com/docs/admob/android/banner#smart_banners
   #if EE_PRIVATE
      INTERSTITIAL    , // Fullscreen, Interstitial     , Phones and Tablets, https://firebase.google.com/docs/admob/android/interstitial
   #endif
   };
   enum STATE : Byte
   {
      NONE   , // ad was not yet requested by the user
      LOADING, // ad is currently being loaded
      DONE   , // ad is available
      ERROR  , // ad failed to load
   };

   AdMobClass& testMode(Bool on=true); // 'on'=if enable test mode, which means that you will receive test ads instead of real ones, and won't risk being banned from AdMob for accidental clicking on them

   // banner ad
   STATE       bannerState  (                                        )C {return _banner_state;} // get state of banner ad
   AdMobClass& bannerDel    (                                        ); // delete a banner ad
   AdMobClass& bannerCreate (C Str8 &unit_id, BANNER_TYPE type=BANNER); // create a banner ad, 'type'=type of the banner, 'unit_id'=ID of the ad (obtained when creating a new ad on the AdMob https://apps.admob.com/ website)
   AdMobClass& bannerShow   (                                        ); // show the banner ad, if the ad is not yet loaded, then it will be automatically shown once it gets loaded
   AdMobClass& bannerHide   (                                        ); // hide the banner ad
   AdMobClass& bannerVisible(Bool visible                            ); // set      banner ad visibility
   AdMobClass& bannerPos    (Int  x, Int y                           ); // set      banner ad position, x=-1..1 (-1=left, 0=center, 1=right), y=-1..1 (-1=bottom, 0=center, 1=top), default=(0, 1)
 C VecI2     & bannerPos    ()C {return _banner_pos ;}                  // get      banner ad position, x=-1..1 (-1=left, 0=center, 1=right), y=-1..1 (-1=bottom, 0=center, 1=top), default=(0, 1)
 C VecI2     & bannerSizePx ()C {return _banner_size;}                  // get      banner ad size in pixels, this becomes valid once banner has finished loading (DONE state)

   // fullscreen ad (aka interstitial ad)
   STATE       fsState (               )C {return _fs_state;} // get state of fullscreen ad
   AdMobClass& fsDel   (               ); // delete a fullscreen ad
   AdMobClass& fsCreate(C Str8 &unit_id); // create a fullscreen ad, 'unit_id'=ID of the ad (obtained when creating a new ad on the AdMob https://apps.admob.com/ website)
   AdMobClass& fsShow  (               ); // show the fullscreen ad, if the ad is not yet loaded, then it will be automatically shown once it gets loaded
#if EE_PRIVATE
   AdMobClass& fsHide   (            ); // hide the fullscreen ad
   AdMobClass& fsVisible(Bool visible); // set      fullscreen ad visibility
#endif

#if !EE_PRIVATE
private:
#endif
   STATE _banner_state, _fs_state;
   VecI2 _banner_pos, _banner_size;
  ~AdMobClass();
   AdMobClass();
}extern
   AdMob;
/******************************************************************************/
struct ChartboostClass
{
#if EE_PRIVATE
   // !! these enums must be equal to "EsenthelActivity.java" !!
#endif
   enum RESULT
   {
      INTERSTITIAL_LOADED   , // called when an interstitial has been loaded from the Chartboost API servers and cached locally
      INTERSTITIAL_LOAD_FAIL, // called when an interstitial has attempted to load from the Chartboost API servers but failed
      INTERSTITIAL_DISPLAYED, // called when an interstitial has been displayed on the screen
      INTERSTITIAL_CLOSED   , // called when an interstitial has been closed
      INTERSTITIAL_CLICKED  , // called when an interstitial has been clicked

      REWARDED_VIDEO_LOADED   , // called when a rewarded video has been loaded from the Chartboost API servers and cached locally
      REWARDED_VIDEO_LOAD_FAIL, // called when a rewarded video has attempted to load from the Chartboost API servers but failed
      REWARDED_VIDEO_DISPLAYED, // called when a rewarded video has been displayed on the screen
      REWARDED_VIDEO_CLOSED   , // called when a rewarded video has been closed
      REWARDED_VIDEO_COMPLETED, // called when a rewarded video has been viewed completely and user is eligible for reward
      REWARDED_VIDEO_CLICKED  , // called when a rewarded video has been clicked
   };

   void (*callback)(RESULT result); // pointer to a custom function that will be called with processed events, 'result'=message received at the moment

   Bool visible()C; // check if there are any ads currently visible on the screen

   ChartboostClass& interstitialLoad     ();
   ChartboostClass& interstitialShow     ();
   Bool             interstitialAvailable()C;

   ChartboostClass& rewardedVideoLoad     ();
   ChartboostClass& rewardedVideoShow     ();
   Bool             rewardedVideoAvailable()C;

#if !EE_PRIVATE
private:
#endif
  ~ChartboostClass();
   ChartboostClass();
}extern
   Chartboost;
/******************************************************************************/
