/******************************************************************************/
const bool TestMode=true; // !! WARNING: if you disable test mode, and click on your own Ads, you may be banned from AdMob !! Do not click on your own Ads !!
/******************************************************************************/
void InitPre()
{
   EE_INIT();
#if !ANDROID && !IOS
   #error Advertisements are not supported on this platform
#endif

   if(TestMode)AdMob.testMode(true); // enable test mode
   
   AdMob.bannerCreate("ca-app-pub-".., AdMobClass.BANNER); // create banner from your AdMob Banner Unit ID, which is obtained from https://apps.admob.com/#monetize
   AdMob.bannerShow(); // display the Ad once it finishes loading
}
/******************************************************************************/
bool Init()
{
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   Str state; switch(AdMob.bannerState())
   {
      case AdMobClass.NONE   : state="None"   ; break;
      case AdMobClass.LOADING: state="Loading"; break;
      case AdMobClass.DONE   : state="Done"   ; break;
      case AdMobClass.ERROR  : state="Error"  ; break;
   }
   D.text(0,  0  , S+"Banner State: "+state);
   D.text(0, -0.1, S+"Banner Size: " +AdMob.bannerSizePx());
}
/******************************************************************************/
