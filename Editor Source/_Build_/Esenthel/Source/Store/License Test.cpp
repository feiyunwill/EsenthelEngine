/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
bool     Demo=false;
Str      ServerLicenseKey;
/******************************************************************************
bool     Demo=true;
Str      LicenseKey, ServerLicenseKey;
Cipher1  LicenseCipher(238, 123, 68, 117, 43, 237, 67, 33, 144, 32, 234, 40, 101, 92, 179, 226, 153, 64, 199, 128, 181, 204, 9, 125, 7, 152, 30, 118, 174, 248, 148, 33, 173, 248, 57, 248, 97, 225, 63, 122, 21, 43, 155, 162, 243, 47, 144, 145, 169, 90, 85, 14, 16, 193, 127, 140, 182, 29, 147, 136, 239, 244, 253, 184, 247, 203, 253, 81, 10, 200, 174, 93, 1, 200, 202, 10, 188, 89, 149, 164, 92, 216, 182, 30, 52, 248, 21, 82, 251, 156, 153, 21, 170, 51, 80, 180, 61, 216, 138, 135, 251, 63, 193, 113, 62, 111, 81, 89, 202, 172, 50, 207, 239, 234, 228, 84, 188, 65, 56, 116, 200, 60, 80, 135, 244, 51, 165, 24, 74, 8, 146, 91, 207, 219, 51, 198, 162, 73, 25, 162, 177, 25, 224, 150, 173, 232, 135, 202, 172, 220, 125, 149, 39, 252, 151, 185, 54, 82, 224, 140, 55, 145, 0, 144, 140, 50, 240, 213, 56, 150, 158, 214, 217, 171, 208, 241, 248, 135, 157, 13, 101, 228, 78, 216, 253, 90, 137, 234, 22, 91, 220, 21, 231, 25, 40, 26, 165, 161, 97, 124, 231, 116, 99, 225, 118, 72, 210, 126, 18, 126, 105, 64, 234, 50, 246, 207, 167, 245, 223, 44, 235, 198, 224, 123, 82, 74, 225, 168, 57, 21, 106, 193, 239, 4, 1, 80, 29, 178, 138, 191, 134, 92, 66, 48, 94, 5, 80, 241, 168, 245, 123, 240, 217, 160, 73, 141);
/******************************************************************************
void SetServerLicenseKey()
{
   Str license_key;
   if(LicenseKey.is())license_key=LicenseKey; // prefer regular license key first
#if STEAM
      else if(LicenseCheck.steamSubValid())license_key=S+"Steam:"+Steam.userID();
#endif

   if(ServerLicenseKey!=license_key)
   {
      ServerLicenseKey=license_key;
      Server.licenseKey(ServerLicenseKey);
   }
}
void SetDemo()
{
   bool full=(LicenseKey.is() || LicenseCheck.steamSubValid());
   if(Demo==full)
   {
      Demo^=1;
      if(Demo!=Buy.is())if(Buy.is())Buy.del();else Buy.create();
      bool editor_network_interface=(full && Projs.editor_network_interface()); if(editor_network_interface!=EditServer.is())if(EditServer.is())EditServer.del();else EditServer.create();
      SetServerLicenseKey();
   }
}
void SetLicenseKey(C Str &key)
{
   if(LicenseKey!=key)
   {
      LicenseKey=key;
      SetServerLicenseKey();
      SetDemo();
   }
}
void SaveLicenseKey(C Str &license_key)
{
   File f; if(f.writeTry(SettingsPath+"data", &LicenseCipher))
   {
      f.cmpUIntV(0); // version
      char8 key[29]; REPAO(key)=license_key[i];
      f<<key;
   }
}
Str LoadLicenseKey()
{
   char8 key[29+1]; Zero(key);
   File f; if(f.readStdTry(SettingsPath+"data", &LicenseCipher))switch(f.decUIntV())
   {
      case 0:
      {
         f.getN(key, 29);
      }break;
   }
   return key;
}
/******************************************************************************
#if STEAM
void SteamCallback(SteamWorks.RESULT result, ULong order_id, C Str &error_message, C SteamWorks.Purchase *purchase, C SteamWorks.Subscription *subscription)
{
   Str msg;
   switch(result)
   {
      case SteamWorks.ORDER_REQUEST_FAILED      : LicenseCheck.steam_ordering=false; msg="Failed to request an order."; break;
      case SteamWorks.ORDER_USER_CANCELED       : LicenseCheck.steam_ordering=false; break;
      case SteamWorks.ORDER_FINALIZE_FAILED     : LicenseCheck.steam_ordering=false; msg="Failed to finalize the order."; break;
      case SteamWorks.ORDER_FINALIZE_OK         : LicenseCheck.steam_ordering=false; if(Steam.subscriptionState()==SteamWorks.WAITING)LicenseCheck.steam_checking=true; break; // once order was finalized re-check the status of the subscription to remove demo limitations
      case SteamWorks.SUBSCRIPTION_STATUS_FAILED: LicenseCheck.steam_checking=false; LicenseCheck.steam_sub_known=false; LicenseCheck.setSteamSub("Can't connect to Steam.\nPlease check if you have Internet connection.", false); SetDemo(); if(Demo && !LicenseCheck.checking())LicenseCheck.fadeIn(); break;
      case SteamWorks.SUBSCRIPTION_STATUS_OK    : LicenseCheck.steam_checking=false; LicenseCheck.steam_sub_known=true ; if(LicenseCheck.steam_sub_is=(subscription!=null))LicenseCheck.steam_sub=*subscription;                    SetDemo(); if(Demo && !LicenseCheck.checking())LicenseCheck.fadeIn(); break;
   }
   if(error_message.is())msg.line()+=S+"Message from Steam: "+error_message;
   if(msg.is())
   {
      if(!error_message.is())msg.line()+="Please check if you have Internet connection.";
      Gui.msgBox("Steam", msg);
   }
}
#endif
/******************************************************************************
class LicenseCheckClass : ClosableWindow
{
   class DeviceIDWindow : ClosableWindow
   {
      Text   text;
      Button copy, open, retry;
      int    item_id=-1;

      static void Copy (DeviceIDWindow &cw) {ClipSet(EsenthelStore.DeviceID());}
      static void Open (DeviceIDWindow &cw) {Explore(S+"http://www.esenthel.com/?id=store&item="+cw.item_id);}
      static void Retry(DeviceIDWindow &cw) {cw.del(); LicenseCheck.check(LicenseCheck.es[0].licenseKey(), true);}

      void create(int item_id)
      {
         T.item_id=item_id;
         Gui+=super.create(Rect_C(0, 0, 1, 0.37), "License Check"); button[2].show();
         T+=text .create(Vec2(clientWidth()/2, -0.10), S+"Please log in to Esenthel Store\nand set this Computer ID for your license:\n"+EsenthelStore.DeviceID());
         T+=copy .create(Rect_U(text.rect().down()+Vec2(-0.3, -0.11), 0.25, 0.06), "Copy ID"   ).func(Copy , T).focusable(false);
         T+=open .create(Rect_U(text.rect().down()+Vec2( 0.0, -0.11), 0.27, 0.06), "Open Store").func(Open , T).focusable(false);
         T+=retry.create(Rect_U(text.rect().down()+Vec2( 0.3, -0.11), 0.26, 0.06), "Try Again" ).func(Retry, T).focusable(false);
         text.activate(); // to disable kb focus
         activate();
      }
   }

   bool           notify=false, steam_checking=false, steam_sub_known=false, steam_sub_is=false, steam_ordering=false;
   int            steam_item_id=0;
   TextBlack      ts;
   Text           text, t_get_sub, t_cur_sub;
   TextLine       license_key;
   Button         ok, buy, buy_steam_monthly, buy_steam_yearly;
   EsenthelStore  es[ELMS(EsenthelStoreEngineLicense)];
   DeviceIDWindow window;
   Download       steam_order_id;
#if STEAM
   SteamWorks.Subscription steam_sub;
#endif

   static void BuySteamMonthly(LicenseCheckClass &lc) {lc.getSteamOrderID(1);}
   static void BuySteamYearly (LicenseCheckClass &lc) {lc.getSteamOrderID(4);}

   static void Changed(LicenseCheckClass &lc) {lc.ok.text=(ValidLicenseKey(lc.license_key()) ? "OK" : "Run as Demo");}
   static void OK     (LicenseCheckClass &lc) {SaveLicenseKey(lc.license_key()); lc.check(lc.license_key(), true);}

   bool steamSubValid()C
   {
   #if STEAM
      return steam_sub_known && steam_sub_is && steam_sub.valid();
   #else
      return false;
   #endif
   }
   bool checking()
   {
      REPA(es)if(es[i].licenseResult()==EsenthelStore.CONNECTING)return true;
      if(steam_checking)return true;
      return false;
   }
   void getSteamOrderID(int item_id)
   {
   #if STEAM
      if( Steam.overlayAvailableMsgBox())
      if(!Steam.userID())Gui.msgBox(S, "Unknown Steam User ID");else
      if(!Steam. appID())Gui.msgBox(S, "Unknown Steam App ID" );else
      {
         if(steam_item_id!=item_id)
         {
            steam_item_id=item_id;
            steam_order_id.del();
         }
         if(steam_order_id.state()==DWNL_NONE)
         {
            Memt<HTTPParam> p;
            p.New().set("u", S+Steam.userID(), HTTP_POST);
            p.New().set("a", S+Steam. appID(), HTTP_POST);
            p.New().set("i", S+     item_id  , HTTP_POST);
            steam_order_id.create("http://esenthel.com/steam_order.php", p);
         }
      }
   #endif
   }
   void testLicense(C Str &license_key) // initialize test license download
   {
      REPAO(es).licenseTest(EsenthelStoreEngineLicense[i], license_key, S, true);
   }
   void check(C Str &license_key, bool notify)
   {
      hide();
      if(ValidLicenseKey(license_key))
      {
         if(license_key!=LicenseKey)
         {
            T.notify=notify; testLicense(license_key);
         }
      }else
      {
         SetLicenseKey(S); clear();
      }
   }
   virtual Window& show()override
   {
      if(hidden())license_key.clear();
      return super.show();
   }
   void clear(bool params=true)
   {
      REPAO(es).licenseClear(params);
   }
   void setSteamSub(C Str &text, bool buy_visible)
   {
      t_cur_sub.set(S+"Current Subscription: "+text);
      buy_steam_monthly.visible(buy_visible);
      buy_steam_yearly .visible(buy_visible);
   }
   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, STEAM ? 0.79 : 0.37), "License Check").level(256); button[2].show();
      T+=text.create(Vec2(clientWidth()/2, -0.05), STEAM ? "Please enter a license key, if you already own one:" : "Please enter your license key");
      T+=license_key.create(Rect_U(text.pos()+Vec2(0, STEAM ? -0.04 : -0.05), 0.8, 0.06)).func(Changed, T).maxLength(29);
      if(!STEAM)T+=buy.create(Rect_U(license_key.rect().down()+Vec2(-0.2, -0.04), 0.3, 0.06), "Buy").func(MiscRegion.BuyLicense);
      T+=ok.create(Rect_U(license_key.rect().down()+Vec2(STEAM ? 0 : 0.2, STEAM ? -0.02 : -0.04), 0.3, 0.06)).func(OK, T); Changed(T);
      Str key=LoadLicenseKey(); if(ValidLicenseKey(key))check(key, false);
   #if STEAM
      Steam.callback=SteamCallback;
      ts.reset().align.set(0, -1);
      T+=t_get_sub.create(Vec2(clientWidth()/2, ok.rect().down().y-0.09), "-- Or get a Steam Subscription --");
      T+=t_cur_sub.create(Vec2(clientWidth()/2, t_get_sub.pos().y -0.09), S, &ts);
      T+=buy_steam_monthly.create(Rect_U(clientWidth()/2, t_cur_sub        .pos().y     -0.075, 0.9, 0.06), "Buy Monthly Subscription 11.40 USD/Month"                                 ).func(BuySteamMonthly, T);
      T+=buy_steam_yearly .create(Rect_U(clientWidth()/2, buy_steam_monthly.rect().min.y-0.030, 0.9, 0.12), "Buy Yearly Subscription 114 USD/Year\n(20% Savings - Only 9.50 USD/Month)").func(BuySteamYearly , T); buy_steam_yearly.text_size/=2;
      #include "../../../../my.h"
      switch(Steam.subscriptionState())
      {
         case SteamWorks.STEAM_NOT_INITIALIZED: setSteamSub("'SteamAPI_Init' failed.\nPlease run the App using the Steam Client.", false); break;
         case SteamWorks.STEAM_NOT_SETUP      : setSteamSub("Web API Key not specified", false); break;
         case SteamWorks. APP_ID_UNAVAILABLE  : setSteamSub("Steam App ID unknown"     , false); break;
         case SteamWorks.USER_ID_UNAVAILABLE  : setSteamSub("Steam User ID unknown"    , false); break;
         case SteamWorks.WAITING              : setSteamSub("Checking"                 , false); steam_checking=true; hide(); break;
         default                              : setSteamSub("Unknown Error"            , false); break;
      }
   #endif
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      int fail=0; REPA(es)switch(es[i].licenseResult())
      {
         case EsenthelStore.INVALID_ITEM              : Gui.msgBox(S, "Invalid Item ID"           ); SetLicenseKey(S); clear(); break;
         case EsenthelStore.INVALID_LICENSE_KEY_FORMAT: Gui.msgBox(S, "Invalid License Key Format"); SetLicenseKey(S); clear(); break;

         case EsenthelStore.CANT_CONNECT:
         {
            Gui.msgBox(S, "Can't connect to Esenthel Authentication Server.\nPlease try again in a moment or check if you have Internet connection.");
            SetLicenseKey(S); clear(false); // keep license key in memory in case "retry" would be added
         }break;

         case EsenthelStore.INVALID_RESPONSE:
         {
            Gui.msgBox(S, "Received an invalid response from Esenthel Authentication Server.\nPlease make sure that you're connected to the Internet, you're using latest Esenthel Software or try again in a moment.");
            SetLicenseKey(S); clear();
         }break;

         case EsenthelStore.LICENSE_KEY_FAIL: fail++; break;

         case EsenthelStore.DEVICE_ID_FAIL:
         {
            window.create(es[i].licenseItemID());
            SetLicenseKey(S); clear(false); // keep license key in memory because it's used for "retry"
         }break;

         case EsenthelStore.CONFIRM_CODE_FAIL:
         {
            Gui.msgBox(S, "Received an invalid confirmation code from Esenthel Authentication Server.\nPlease make sure that your computer has correct date, hour and time zone set.");
            SetLicenseKey(S); clear();
         }break;

         case EsenthelStore.OK:
         {
            if(notify)Gui.msgBox(S, "License Key OK");
            SetLicenseKey(es[i].licenseKey()); clear();
         }break;
      }

      if(fail==Elms(es)) // all tests failed
      {
         activate(); Gui.msgBox(S, "Invalid License Key");
         SetLicenseKey(S); clear();
      }
   #if STEAM
      SetDemo(); // in case subscription expires while the Editor is running
      if(visible())
      {
         if(steam_sub_known)
         {
            if(steam_sub_is)
            {
               if(steam_sub.valid())
               {
                  if(steam_sub.active)setSteamSub("Active"  , false);
                  else                setSteamSub("Canceled", false);
               }else                  setSteamSub("Expired" , true );
            }else                     setSteamSub("None"    , true );
         }
         buy_steam_monthly.enabled(steam_order_id.state()==DWNL_NONE && !steam_ordering);
         buy_steam_yearly .enabled(steam_order_id.state()==DWNL_NONE && !steam_ordering);
      }
      switch(steam_order_id.state())
      {
         case DWNL_ERROR:
         case DWNL_DONE:
         {
            ulong    order_id=0;
            TextData td; if(td.load(FileText().readMem(steam_order_id.data(), steam_order_id.size())))if(C TextNode *p=td.findNode("order_id"))order_id=p.asULong();
            steam_order_id.del();
            if(order_id)
            {
               bool monthly=(steam_item_id==1);
               switch(Steam.subscribe(order_id, steam_item_id, "Esenthel Engine License", monthly ? 1140 : 11400, monthly ? SteamWorks.MONTH : SteamWorks.YEAR, 1))
               {
                  case SteamWorks.WAITING              : steam_ordering=true; break;
                  case SteamWorks.STEAM_NOT_INITIALIZED: Gui.msgBox(S, "Steam Client not found"   ); break;
                  case SteamWorks.STEAM_NOT_SETUP      : Gui.msgBox(S, "Web API Key not specified"); break;
                  case SteamWorks. APP_ID_UNAVAILABLE  : Gui.msgBox(S, "Steam App ID unknown"     ); break;
                  case SteamWorks.USER_ID_UNAVAILABLE  : Gui.msgBox(S, "Steam User ID unknown"    ); break;
                  default                              : Gui.msgBox(S, "Unknown Error"            ); break;
               }
            }else Gui.msgBox(S, "Can't create a Steam Order ID, please try again in a moment and check if you have Internet connection.");
         }break;
      }
   #endif
   }
   void draw()
   {
   #if 0
      switch(down.state())
      {
         case DWNL_WAIT:
         case DWNL_DOWNLOAD: {TextStyleParams ts; ts.size=0.04; ts.color=BLACK; ts.shadow=0; ts.align.set(-1, 1); D.text(ts, D.w(), -D.h(), "Checking License..");} break;
      }
   #endif
   }
}
LicenseCheckClass LicenseCheck;
/******************************************************************************
class BuyClass : Window
{
   TextNoTest text;
   Button     yes, no;
   flt        time=0;

   static void Yes(ptr) {LicenseCheck.activate();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1.2, 0.49), "Buy Full Version").level(LicenseCheck.level()-1).hide(); FlagDisable(flag, WIN_MOVABLE);
      T+=text.create(Rect_C(clientWidth()/2, -0.15, clientWidth()*0.95, 0.1), "You're running a demo version.\nWould you like to purchase full version?\n\nBy purchasing the license you're contributing to making the engine better."); text.auto_line=AUTO_LINE_SPACE_SPLIT;
      T+= yes.create(Rect_C(clientWidth()*1/3, -0.35, 0.29, 0.07), "Yes").focusable(false).func(Yes);
      T+=  no.create(Rect_C(clientWidth()*2/3, -0.35, 0.29, 0.07), "No" ).focusable(false).func(HideEditAct, SCAST(GuiObj, T));
   }
   virtual Window& show()override
   {
      if(hidden())time=Time.appTime()+10;
      return super.show();
   }
   virtual Window& hide()override
   {
      if(visible())time=Time.appTime()+BuyFullVersionTime;
      return super.hide();
   }
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override
   {
      if(visible() && gpc.visible)
      {
         GuiObj *go=super.test(gpc, pos, mouse_wheel); if(!contains(go)){mouse_wheel=null; go=this;} 
         return  go;
      }
      return null;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         flt t=time-Time.appTime();
         if(t>=0){no.disabled(true ); no.text=S+"No ("+Ceil(t)+')';}
         else    {no.disabled(false); no.text=  "No";}
      }else
      {
         if(Time.appTime()>=time)activate();
      }
   }
   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         D.clip();
         Rect(-D.w(), -D.h(), D.w(), D.h()).draw(ColorAlpha(BLACK, 0.4));
         super.draw(gpc);
      }
   }
}
BuyClass Buy;
/******************************************************************************/

/******************************************************************************/
