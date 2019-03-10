/******************************************************************************/
class ElmApp : ElmData
{
   enum FLAG
   {
      EMBED_ENGINE_DATA     =1<<0,
      PUBLISH_PROJ_DATA     =1<<1,
      PUBLISH_PHYSX_DLL     =1<<2,
      PUBLISH_DATA_AS_PAK   =1<<3,
      ANDROID_EXPANSION     =1<<4,
      PUBLISH_STEAM_DLL     =1<<5,
      PUBLISH_OPEN_VR_DLL   =1<<6,
      EMBED_ENGINE_DATA_FULL=1<<7,
    //WINDOWS_CODE_SIGN     =1<< ,
   };
   Str               dirs_windows, dirs_nonwindows,
                     headers_windows, headers_mac, headers_linux, headers_android, headers_ios,
                     libs_windows, libs_mac, libs_linux, libs_android, libs_ios,
                     package, android_license_key, location_usage_reason, cb_app_id_ios, cb_app_signature_ios, cb_app_id_google, cb_app_signature_google;
   int               build;
   ulong             fb_app_id;
   Edit::STORAGE_MODE storage;
   byte              supported_orientations, // DIR_FLAG
                     flag;
   UID               icon, notification_icon,
                     image_portrait, image_landscape, // splash screen
                     gui_skin;
   TimeStamp         dirs_windows_time, dirs_nonwindows_time,
                     headers_windows_time, headers_mac_time, headers_linux_time, headers_android_time, headers_ios_time,
                     fb_app_id_time, cb_app_id_ios_time, cb_app_signature_ios_time, cb_app_id_google_time, cb_app_signature_google_time,
                     libs_windows_time, libs_mac_time, libs_linux_time, libs_android_time, libs_ios_time,
                     package_time, android_license_key_time, location_usage_reason_time, build_time, storage_time, supported_orientations_time,
                     embed_engine_data_time, publish_proj_data_time, publish_physx_dll_time, publish_steam_dll_time, publish_open_vr_dll_time, publish_data_as_pak_time, android_expansion_time,
                     icon_time, notification_icon_time, image_portrait_time, image_landscape_time, gui_skin_time;

   // get
   bool equal(C ElmApp &src)C;
   bool newer(C ElmApp &src)C;

   virtual bool mayContain(C UID &id)C override;

   int     embedEngineData(     )C;
   ElmApp& embedEngineData(int e); 

   bool publishProjData ()C;   ElmApp& publishProjData (bool on);
   bool publishPhysxDll ()C;   ElmApp& publishPhysxDll (bool on);
   bool publishSteamDll ()C;   ElmApp& publishSteamDll (bool on);
   bool publishOpenVRDll()C;   ElmApp& publishOpenVRDll(bool on);
   bool publishDataAsPak()C;   ElmApp& publishDataAsPak(bool on);
   bool androidExpansion()C;   ElmApp& androidExpansion(bool on);
 //bool windowsCodeSign ()C {return FlagTest(flag, WINDOWS_CODE_SIGN  );}   ElmApp& windowsCodeSign (bool on) {FlagSet(flag, WINDOWS_CODE_SIGN  , on); return T;}

   // operations
   virtual void newData()override;
   uint undo(C ElmApp &src);
   uint sync(C ElmApp &src, bool manual);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   class StorageMode
   {
      Edit::STORAGE_MODE mode;
      cchar8           *name;
   };
   static StorageMode StorageModes[]
;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmApp();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
