/******************************************************************************/
/******************************************************************************/
class LicenseCheckClass : ClosableWindow
{
   class DeviceIDWindow : ClosableWindow
   {
      Text   text;
      Button copy, open, retry;
      int    item_id;

      static void Copy (DeviceIDWindow &cw);
      static void Open (DeviceIDWindow &cw);
      static void Retry(DeviceIDWindow &cw);

      void create(int item_id);

public:
   DeviceIDWindow();
   };

   bool           notify, steam_checking, steam_sub_known, steam_sub_is, steam_ordering;
   int            steam_item_id;
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

   static void BuySteamMonthly(LicenseCheckClass &lc);
   static void BuySteamYearly (LicenseCheckClass &lc);

   static void Changed(LicenseCheckClass &lc);
   static void OK     (LicenseCheckClass &lc);

   bool steamSubValid()C;
   bool checking();
   void getSteamOrderID(int item_id);
   void testLicense(C Str &license_key); // initialize test license download
   void check(C Str &license_key, bool notify);
   virtual Window& show()override;
   void clear(bool params=true);
   void setSteamSub(C Str &text, bool buy_visible);
   void create();
   virtual void update(C GuiPC &gpc)override;
   void draw();

public:
   LicenseCheckClass();
};
/******************************************************************************/
/******************************************************************************/
extern LicenseCheckClass LicenseCheck;
/******************************************************************************/
