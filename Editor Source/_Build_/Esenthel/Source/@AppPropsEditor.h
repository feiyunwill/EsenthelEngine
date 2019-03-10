/******************************************************************************/
/******************************************************************************/
class AppPropsEditor : PropWin
{
   enum ORIENT
   {
      ORIENT_PORTRAIT        ,
      ORIENT_LANDSCAPE       ,
      ORIENT_ALL             ,
      ORIENT_ALL_NO_DOWN     ,
      ORIENT_PORTRAIT_LOCKED ,
      ORIENT_LANDSCAPE_LOCKED,
   };
   static cchar8 *OrientName[]
;
   static NameDesc EmbedEngine[]
;
   static NameDesc StorageName[]
; ASSERT(Edit::STORAGE_INTERNAL==0 && Edit::STORAGE_EXTERNAL==1 && Edit::STORAGE_AUTO==2);
   static ORIENT FlagToOrient(uint flag);
   static uint OrientToFlag(ORIENT orient);

   class AppImage : ImageSkin
   {
      Button      remove;
      Image       image_2d;
      ImagePtr    game_image;
      MemberDesc  md, md_time;

      static void Remove(AppImage &ai);

      void setImage();
      void setImage(C UID &image_id);
      AppImage& create(C MemberDesc &md, C MemberDesc &md_time, GuiObj &parent, C Rect &rect);
   };

   UID       elm_id;
   Elm      *elm;
   bool      changed, changed_headers;
   Property *p_icon, *p_image_portrait, *p_image_landscape, *p_notification_icon;
   AppImage    icon     ,    image_portrait     ,    image_landscape     ,    notification_icon;
   Tabs      platforms, platforms2;

   static void Changed             (C Property &prop);
   static void GetAndroidLicenseKey(  ptr           );
   static void GetFacebookAppID    (  ptr           );
   static void GetChartboostApp    (  ptr           );

   static void DirsWin                     (  AppPropsEditor &ap, C Str &text);
   static Str  DirsWin                     (C AppPropsEditor &ap             );
   static void DirsNonWin                  (  AppPropsEditor &ap, C Str &text);
   static Str  DirsNonWin                  (C AppPropsEditor &ap             );
   static void HeadersWin                  (  AppPropsEditor &ap, C Str &text);
   static Str  HeadersWin                  (C AppPropsEditor &ap             );
   static void HeadersMac                  (  AppPropsEditor &ap, C Str &text);
   static Str  HeadersMac                  (C AppPropsEditor &ap             );
   static void HeadersLinux                (  AppPropsEditor &ap, C Str &text);
   static Str  HeadersLinux                (C AppPropsEditor &ap             );
   static void HeadersAndroid              (  AppPropsEditor &ap, C Str &text);
   static Str  HeadersAndroid              (C AppPropsEditor &ap             );
   static void HeadersiOS                  (  AppPropsEditor &ap, C Str &text);
   static Str  HeadersiOS                  (C AppPropsEditor &ap             );
   static void LibsWindows                 (  AppPropsEditor &ap, C Str &text);
   static Str  LibsWindows                 (C AppPropsEditor &ap             );
   static void LibsMac                     (  AppPropsEditor &ap, C Str &text);
   static Str  LibsMac                     (C AppPropsEditor &ap             );
   static void LibsLinux                   (  AppPropsEditor &ap, C Str &text);
   static Str  LibsLinux                   (C AppPropsEditor &ap             );
   static void LibsAndroid                 (  AppPropsEditor &ap, C Str &text);
   static Str  LibsAndroid                 (C AppPropsEditor &ap             );
   static void LibsiOS                     (  AppPropsEditor &ap, C Str &text);
   static Str  LibsiOS                     (C AppPropsEditor &ap             );
   static void Package                     (  AppPropsEditor &ap, C Str &text);
   static Str  Package                     (C AppPropsEditor &ap             );
   static void AndroidLicenseKey           (  AppPropsEditor &ap, C Str &text);
   static Str  AndroidLicenseKey           (C AppPropsEditor &ap             );
   static void Build                       (  AppPropsEditor &ap, C Str &text);
   static Str  Build                       (C AppPropsEditor &ap             );
   static void LocationUsageReason         (  AppPropsEditor &ap, C Str &text);
   static Str  LocationUsageReason         (C AppPropsEditor &ap             );
   static void FacebookAppID               (  AppPropsEditor &ap, C Str &text);
   static Str  FacebookAppID               (C AppPropsEditor &ap             );
   static void ChartboostAppIDiOS          (  AppPropsEditor &ap, C Str &text);
   static Str  ChartboostAppIDiOS          (C AppPropsEditor &ap             );
   static void ChartboostAppSignatureiOS   (  AppPropsEditor &ap, C Str &text);
   static Str  ChartboostAppSignatureiOS   (C AppPropsEditor &ap             );
   static void ChartboostAppIDGoogle       (  AppPropsEditor &ap, C Str &text);
   static Str  ChartboostAppIDGoogle       (C AppPropsEditor &ap             );
   static void ChartboostAppSignatureGoogle(  AppPropsEditor &ap, C Str &text);
   static Str  ChartboostAppSignatureGoogle(C AppPropsEditor &ap             );
   static void Storage                     (  AppPropsEditor &ap, C Str &text);
   static Str  Storage                     (C AppPropsEditor &ap             );
   static void GuiSkin                     (  AppPropsEditor &ap, C Str &text);
   static Str  GuiSkin                     (C AppPropsEditor &ap             );
   static void EmbedEngineData             (  AppPropsEditor &ap, C Str &text);
   static Str  EmbedEngineData             (C AppPropsEditor &ap             );
   static void PublishProjData             (  AppPropsEditor &ap, C Str &text);
   static Str  PublishProjData             (C AppPropsEditor &ap             );
   static void PublishDataAsPak            (  AppPropsEditor &ap, C Str &text);
   static Str  PublishDataAsPak            (C AppPropsEditor &ap             );
   static void AndroidExpansion            (  AppPropsEditor &ap, C Str &text);
   static Str  AndroidExpansion            (C AppPropsEditor &ap             );
   static void PublishPhysxDll             (  AppPropsEditor &ap, C Str &text);
   static Str  PublishPhysxDll             (C AppPropsEditor &ap             );
   static void PublishSteamDll             (  AppPropsEditor &ap, C Str &text);
   static Str  PublishSteamDll             (C AppPropsEditor &ap             );
   static void PublishOpenVRDll            (  AppPropsEditor &ap, C Str &text);
   static Str  PublishOpenVRDll            (C AppPropsEditor &ap             );
 //static void WindowsCodeSign             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.windowsCodeSign(TextBool(text)).windows_code_sign_time.getUTC();}}
 //static Str  WindowsCodeSign             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.windowsCodeSign(); return S;}
   static void Orientation                 (  AppPropsEditor &ap, C Str &text);
   static Str  Orientation                 (C AppPropsEditor &ap             );

   static cchar8 *platforms_t[]
;
   static cchar8 *platforms2_t[]
;

   void create();
   void toGui();

   virtual AppPropsEditor& hide()override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id);                                
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);

public:
   AppPropsEditor();
};
/******************************************************************************/
/******************************************************************************/
extern AppPropsEditor AppPropsEdit;
/******************************************************************************/
