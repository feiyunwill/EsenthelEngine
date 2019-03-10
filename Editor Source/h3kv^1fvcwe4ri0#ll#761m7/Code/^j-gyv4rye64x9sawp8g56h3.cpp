/******************************************************************************/
class EEItem
{
   bool         opened=false;
   byte         flag=0; // ELM_FLAG
   ELM_TYPE     type=ELM_NONE;
   ImagePtr     icon;
   Str          base_name, full_name;
   EEItem      *parent=null;
   Memx<EEItem> children;
}
class CodeView : Region, Edit.CodeEditorInterface
{
   Memx<EEItem > items;
   Memc<EEItem*> items_sorted;

   virtual void configChangedDebug()override
   {
      Misc.build.menu("Debug"  ,  configDebug(), QUIET);
      Misc.build.menu("Release", !configDebug(), QUIET);
   }
   virtual void configChanged32Bit()override
   {
      Misc.build.menu("32-bit",  config32Bit(), QUIET);
      Misc.build.menu("64-bit", !config32Bit(), QUIET);
   }
   virtual void configChangedDX9()override
   {
      Misc.build.menu("DirectX 11", !configDX9(), QUIET);
      Misc.build.menu("DirectX 9" ,  configDX9(), QUIET);
   }
   virtual void configChangedEXE()override
   {
      Misc.build.menu("Windows EXE"      , configEXE()==Edit.EXE_EXE  , QUIET);
      Misc.build.menu("Windows DLL"      , configEXE()==Edit.EXE_DLL  , QUIET);
      Misc.build.menu("Windows LIB"      , configEXE()==Edit.EXE_LIB  , QUIET);
      Misc.build.menu("Windows Universal", configEXE()==Edit.EXE_NEW  , QUIET);
      Misc.build.menu("Android APK"      , configEXE()==Edit.EXE_APK  , QUIET);
      Misc.build.menu("Mac APP"          , configEXE()==Edit.EXE_MAC  , QUIET);
      Misc.build.menu("iOS APP"          , configEXE()==Edit.EXE_IOS  , QUIET);
      Misc.build.menu("Linux"            , configEXE()==Edit.EXE_LINUX, QUIET);
      Misc.build.menu("Web"              , configEXE()==Edit.EXE_WEB  , QUIET);
   }
   virtual void visibleChangedOptions      ()override {Misc.build.menu("View Options"           , visibleOptions      (), QUIET);}
   virtual void visibleChangedOpenedFiles  ()override {}
   virtual void visibleChangedOutput       ()override {Misc.build.menu("View Output"            , visibleOutput       (), QUIET);}
   virtual void visibleChangedAndroidDevLog()override {Misc.build.menu("View Android Device Log", visibleAndroidDevLog(), QUIET);}

   virtual UID projectID()override {return Proj.id;}

   virtual UID               appID                              ()override {if(Elm *app=Proj.findElm(Proj.curApp()))                                  return app.id                          ; return super.appID();}
   virtual Str               appName                            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))                                  return app.name                        ; return super.appName();}
   virtual Str               appDirsWindows                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_windows           ; return super.appDirsWindows();}
   virtual Str               appDirsNonWindows                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_nonwindows        ; return super.appDirsNonWindows();}
   virtual Str               appHeadersWindows                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_windows        ; return super.appHeadersWindows();}
   virtual Str               appHeadersMac                      ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_mac            ; return super.appHeadersMac();}
   virtual Str               appHeadersLinux                    ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_linux          ; return super.appHeadersLinux();}
   virtual Str               appHeadersAndroid                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_android        ; return super.appHeadersAndroid();}
   virtual Str               appHeadersiOS                      ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_ios            ; return super.appHeadersiOS();}
   virtual Str               appLibsWindows                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_windows           ; return super.appLibsWindows();}
   virtual Str               appLibsMac                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_mac               ; return super.appLibsMac();}
   virtual Str               appLibsLinux                       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_linux             ; return super.appLibsLinux();}
   virtual Str               appLibsAndroid                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_android           ; return super.appLibsAndroid();}
   virtual Str               appLibsiOS                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_ios               ; return super.appLibsiOS();}
   virtual Str               appPackage                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.package                ; return super.appPackage();}
   virtual Str               appLicenseKey                      ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.android_license_key    ; return super.appLicenseKey();}
   virtual Str               appLocationUsageReason             ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.location_usage_reason  ; return super.appLocationUsageReason();}
   virtual Int               appBuild                           ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.build                  ; return super.appBuild();}
   virtual ulong             appFacebookAppID                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.fb_app_id              ; return super.appFacebookAppID();}
   virtual Str               appChartboostAppIDiOS              ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_id_ios          ; return super.appChartboostAppIDiOS();}
   virtual Str               appChartboostAppSignatureiOS       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_signature_ios   ; return super.appChartboostAppSignatureiOS();}
   virtual Str               appChartboostAppIDGooglePlay       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_id_google       ; return super.appChartboostAppIDGooglePlay();}
   virtual Str               appChartboostAppSignatureGooglePlay()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_signature_google; return super.appChartboostAppSignatureGooglePlay();}
   virtual Edit.STORAGE_MODE appPreferredStorage                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.storage                ; return super.appPreferredStorage();}
   virtual UInt              appSupportedOrientations           ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.supported_orientations ; return super.appSupportedOrientations();}
   virtual UID               appGuiSkin                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.gui_skin               ; return super.appGuiSkin();}
   virtual int               appEmbedEngineData                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.embedEngineData ()     ; return super.appEmbedEngineData();}
   virtual Cipher*           appEmbedCipher                     ()override {static ProjectCipher cipher; /*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return cipher.set(Proj)(); return super.appEmbedCipher();}
   virtual COMPRESS_TYPE     appEmbedCompress                   ()override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return Proj.compress_type              ; return super.appEmbedCompress();}
   virtual int               appEmbedCompressLevel              ()override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return Proj.compress_level             ; return super.appEmbedCompressLevel();}
   virtual DateTime          appEmbedSettingsTime               ()override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return Max(Max(Proj.compress_type_time, Proj.compress_level_time), Max(Proj.cipher_time, Proj.cipher_key_time)).asDateTime(); return super.appEmbedSettingsTime();} // return Max of all params affecting PAK's
   virtual Bool              appPublishProjData                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishProjData ()     ; return super.appPublishProjData();}
   virtual Bool              appPublishPhysxDll                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishPhysxDll ()     ; return super.appPublishPhysxDll();}
   virtual Bool              appPublishSteamDll                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishSteamDll ()     ; return super.appPublishSteamDll();}
   virtual Bool              appPublishOpenVRDll                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishOpenVRDll()     ; return super.appPublishOpenVRDll();}
   virtual Bool              appPublishDataAsPak                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishDataAsPak()     ; return super.appPublishDataAsPak();}
   virtual Bool              appAndroidExpansion                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.androidExpansion()     ; return super.appAndroidExpansion();}
 //virtual Bool              appWindowsCodeSign                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.windowsCodeSign ()     ; return super.appWindowsCodeSign();}
   virtual ImagePtr          appIcon                            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.icon             .valid())return ImagePtr().get(Proj.gamePath(app_data.icon             )); return super.appIcon();}
   virtual ImagePtr          appImagePortrait                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.image_portrait   .valid())return ImagePtr().get(Proj.gamePath(app_data.image_portrait   )); return super.appImagePortrait();}
   virtual ImagePtr          appImageLandscape                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.image_landscape  .valid())return ImagePtr().get(Proj.gamePath(app_data.image_landscape  )); return super.appImageLandscape();}
   virtual ImagePtr          appNotificationIcon                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.notification_icon.valid())return ImagePtr().get(Proj.gamePath(app_data.notification_icon)); return super.appNotificationIcon();}

   static void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index) {ThreadMayUseGPUData(); generate.process();}
   static void ImageConvertProcess (ImageConvert  &convert , ptr user, int thread_index) {ThreadMayUseGPUData(); convert .process();}

   virtual void appSpecificFiles(MemPtr<PakFileData> files)override
   {
      Memc<ImageGenerate> generate;
      Memc<ImageConvert>  convert;
      Memt<Elm*>          app_elms; Proj.getActiveAppElms(app_elms);
      AddPublishFiles(app_elms, files, generate, convert);
      // all generations/conversions need to be processed here so 'files' point correctly
      WorkerThreads.process1(generate, ImageGenerateProcess);
      WorkerThreads.process1(convert , ImageConvertProcess );
   }
   virtual void appInvalidProperty(C Str &msg)override
   {
      if(Elm *app=Proj.findElm(Proj.curApp()))AppPropsEdit.set(app); Gui.msgBox(S, msg);
   }

   virtual void validateActiveSources()override {Proj.activateSources(1);}

   virtual Rect menuRect()override
   {
      Rect r(-D.w(), -D.h(), D.w(), D.h());
      if(         Mode.visibleTabs())MIN(r.max.y,          Mode.rect().min.y);
      if(     MtrlEdit.visible    ())MIN(r.max.x,      MtrlEdit.rect().min.x);
      if(WaterMtrlEdit.visible    ())MIN(r.max.x, WaterMtrlEdit.rect().min.x);

      if(Misc.visible() && Misc.pos.y==(int)CodeMenuOnTop && !(Mode.visibleTabs() && Misc.pos.y))
      {
         if(Misc.move_misc.visible())
         {
            if(Misc.pos.x==0)MAX(r.min.x, Misc.move_misc.rect().max.x);
            else             MIN(r.max.x, Misc.move_misc.rect().min.x);
         }else
         {
            if(Misc.pos.x==0)MAX(r.min.x, Misc.rect().max.x);
            else             MIN(r.max.x, Misc.rect().min.x);
         }
      }else if(Proj.visible())MAX(r.min.x, Proj.rect().max.x);

      return r;
   }
   virtual Rect sourceRect()override {return EditRect();}

   virtual Str sourceProjPath(C UID &id)override
   {
      Str path;
      if(Elm *elm=Proj.findElm(id))
         for(path=CleanFileName(elm.name); elm=Proj.findElm(elm.parent_id); ) // always add name of first element
      {
         if(elm.type!=ELM_APP && elm.type!=ELM_LIB && elm.type!=ELM_FOLDER)continue; // don't add name of a parent if it's not a folder
         if(elm.type==ELM_APP)return path; // return before adding app name
         path=CleanFileName(elm.name).tailSlash(true)+path;
         if(elm.type==ELM_LIB)return path; // return after  adding lib name
      }
      return S; // parent is not app and not lib, then return empty string
   }
   virtual Edit.ERROR_TYPE sourceDataLoad(C UID &id, Str &data)override
   {
      if(Elm *elm=Proj.findElm(id))
      {
         if(ElmCode *code_data=elm.codeData())
         {
            Edit.ERROR_TYPE error=LoadCode(data, Proj.codePath(id));
            if(error==Edit.EE_ERR_FILE_NOT_FOUND && !code_data.ver)return Edit.EE_ERR_ELM_NOT_DOWNLOADED; // if file not found and ver not set yet, then report as not yet downloaded
            return error;
         }
         return Edit.EE_ERR_ELM_NOT_CODE;
      }
      return Edit.EE_ERR_ELM_NOT_FOUND;
   }
   virtual Bool sourceDataSave(C UID &id, C Str &data)override
   {
      if(Elm *elm=Proj.findElm(id))if(ElmCode *code_data=elm.codeData())if(SaveCode(data, Proj.codePath(id)))
      {
         code_data.newVer();
         code_data.from(data);
         return true;
      }
      return false;
   }
   virtual void sourceChanged(bool activate=false)override
   {
      Mode.tabAvailable(MODE_CODE, sourceCurIs());
      if(activate && sourceCurIs()){Mode.set(MODE_CODE); HideBig();} // set mode in case Code Editor triggers source opening
      SetTitle();
      if(activate)Proj.refresh();
   }
   virtual Bool elmValid    (C UID &id              )override {return Proj.findElm(id)!=null;}
   virtual Str  elmBaseName (C UID &id              )override {if(Elm *elm=Proj.findElm(id))return CleanFileName(elm.name); return S;}
   virtual Str  elmFullName (C UID &id              )override {return Proj.elmFullName(id);}
   virtual void elmHighlight(C UID &id, C Str  &name)override {Proj.elmHighlight(id, name);}
   virtual void elmOpen     (C UID &id              )override {if(Elm *elm=Proj.findElm(id))if(!(selected() && id==sourceCurId()))Proj.elmToggle(elm);} // don't toggle currently opened source because it will close it
   virtual void elmLocate   (C UID &id              )override {Proj.elmLocate(id);}
   virtual void elmPreview  (C UID &id, C Vec2 &pos, bool mouse, C Rect &clip)override
   {
      if(Elm *elm=Proj.findElm(id))
      {
         Rect_LU r(pos, D.h()*0.65); if(mouse)r-=D.pixelToScreenSize(VecI2(0, 32)); // avg mouse cursor height
         if(r.max.x>clip.max.x){r.min.x-=r.max.x-clip.max.x; r.max.x=clip.max.x;} if(r.min.x<clip.min.x){r.max.x+=clip.min.x-r   .min.x; r.min.x=clip.min.x;}
         if(r.min.y<clip.min.y){r+=Vec2(0, pos.y-r.min.y+0.01)                 ;} if(r.max.y>clip.max.y){r.min.y-=   r.max.y-clip.max.y; r.max.y=clip.max.y;}
         Preview.draw(*elm, r);
      }
   }
   virtual Str idToText(C UID &id, Bool *valid)override {return Proj.idToText(id, valid, ProjectEx.ID_SKIP_UNKNOWN);} // if we're in code then it already displays an ID, no need to write the same ID on top of that

   virtual void getProjPublishElms(Memc<ElmLink> &elms)override
   {
      elms.clear(); FREPA(Proj.elms) // process in order
      {
         Elm &elm=Proj.elms[i];
         if(elm.finalPublish() && ElmPublish(elm.type) && ElmVisible(elm.type))elms.New().set(elm.id, Proj.elmFullName(&elm), Proj.elmIcon(elm.type));
      }
   }

   Str importPaths(C Str &path)C {return super.importPaths() ? GetRelativePath(GetPath(App.exe()), path) : path;}

   void drag(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(selected())
      {
         Memc<UID> temp;
         FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmPublish(elm.type))temp.add(elm.id); // filter out non-publishable elements
         super.paste(temp, obj, screen_pos);
      }
   }
   void drop(C MemPtr<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(selected())
      {
         Str text;
         FREPA(names)
         {
            if(i)text+=", ";
            text+='"';
            text+=Replace(names[i], '\\', '/');
            text+='"';
         }
         super.paste(text, obj, screen_pos);
      }
   }

   bool selected()C {return Mode()==MODE_CODE;}
   void selectedChanged() {menuEnabled(selected());}
   void flush() {} // do nothing because sources need to be saved manually

   void overwriteChanges()
   {
      REPA(Proj.elms)if(Proj.elms[i].type==ELM_CODE)sourceOverwrite(Proj.elms[i].id);
   }
   void sourceTitleChanged(C UID &id) // call if name or "modified state" changed
   {
      if(selected() && id==sourceCurId())SetTitle();
   }
   void sourceRename(C UID &id)
   {
      super.sourceRename(id);
      sourceTitleChanged(id);
   }
   bool sourceDataSet(C UID &id, C Str &data)
   {
      if(super.sourceDataSet(id, data)){sourceTitleChanged(id); return true;}
      return false;
   }

   void cleanAll()
   {
      super.cleanAll(); // first call super to stop any build in progress
      FDelDirs(ProjectsPath.tailSlash(true)+ProjectsPublishPath);
   }

   static int Compare(C Enum.Elm &a, C Enum.Elm &b) {return .Compare(a.name, b.name, true);}
   void makeAuto(bool publish=false)
   {
      if(Proj.valid())
      {
         Memt<Enum.Elm> obj_types; REPA(Proj.existing_obj_classes)if(Elm *elm=Proj.findElm(Proj.existing_obj_classes[i]))obj_types.New().set(NameToEnum(elm.name), elm.id); obj_types.sort(Compare);
         int max_length=0; REPA(obj_types)MAX(max_length, Length(obj_types[i].name));

         Str data;
         data ="/******************************************************************************/\n";
         // generate header
         data+="// THIS IS AN AUTOMATICALLY GENERATED FILE\n";
         data+="// THIS FILE WAS GENERATED BY ESENTHEL EDITOR, ACCORDING TO PROJECT SETTINGS\n";
         data+="/******************************************************************************/\n";
         data+="// CONSTANTS\n";
         data+="/******************************************************************************/\n";
         // generate constants
         data+=S+"#define    STEAM   "+appPublishSteamDll ()+" // if Application properties have Steam enabled\n" ; // display it here even if it's just Auto.cpp and doesn't affect other codes, so that the user sees the macro and can be aware of it
         data+=S+"#define    OPEN_VR "+appPublishOpenVRDll()+" // if Application properties have OpenVR enabled\n"; // display it here even if it's just Auto.cpp and doesn't affect other codes, so that the user sees the macro and can be aware of it
         data+=S+"const bool EE_PUBLISH          =" +TextBool(publish)+"; // this is set to true when compiling for publishing\n";
         data+=S+"const bool EE_ENGINE_EMBED     =("+TextBool(appEmbedEngineData()!=0)+" && !WINDOWS_NEW && !MOBILE && !WEB); // this is set to true when \"Embed Engine Data\" was enabled in application settings, this is always disabled for WindowsNew, Mobile and Web builds\n";
         data+=S+"const bool EE_ANDROID_EXPANSION=" +TextBool(appAndroidExpansion())+"; // this is set to true when auto-download of Android Expansion Files is enabled\n";
       //data+=S+"cchar *C   EE_SDK_PATH         =                                     \""+Replace(SDKPath()                             , '\\', '/').tailSlash(false)+"\";\n";
         data+=S+"cchar *C   EE_PHYSX_DLL_PATH   =((WINDOWS_NEW || MOBILE || WEB) ? null           : EE_PUBLISH ? u\"Bin\"             : u\""+Replace(BinPath()                             , '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C   EE_ENGINE_PATH      =((WINDOWS_NEW || MOBILE || WEB) ? u\"Engine.pak\"  : EE_PUBLISH ? u\"Bin/Engine.pak\"  : u\""+Replace(BinPath().tailSlash(true)+"Engine.pak", '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C   EE_PROJECT_PATH     =((WINDOWS_NEW || MOBILE || WEB) ? u\"Project.pak\" : EE_PUBLISH ? u\"Bin/Project.pak\" : u\""+Replace(Proj.game_path                        , '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C   EE_PROJECT_NAME     =u\""+CString(Proj.name)+"\";\n";
         data+=S+"cchar *C   EE_APP_NAME         =u\""+CString(appName())+"\";\n";
         data+=S+"const int  EE_APP_BUILD        ="+appBuild()+";\n";
         data+=S+"const UID  EE_GUI_SKIN         ="+appGuiSkin().asCString()+";\n";
         if(cchar8 *cipher_class=(InRange(Proj.cipher, CIPHER_NUM) ? CipherText[Proj.cipher].clazz : null))
         {
            data+=S+cipher_class+"   _EE_PROJECT_CIPHER   ("; FREPA(Proj.cipher_key){if(i)data+=", "; data+=Proj.cipher_key[i];} data+=");\n";
            data+=S+      "Cipher *C  EE_PROJECT_CIPHER   =&_EE_PROJECT_CIPHER;\n";
         }else
         {
            data+=S+"Cipher *C  EE_PROJECT_CIPHER   =null;\n";
         }
         data+="/******************************************************************************/\n";
         data+="// FUNCTIONS\n";
         data+="/******************************************************************************/\n";
         // functions
         data+="void EE_INIT(bool load_engine_data=true, bool load_project_data=true)\n";
         data+="{\n";
         data+="   App.name(EE_APP_NAME); // set application name\n";
if(appGuiSkin().valid())data+="   Gui.default_skin=EE_GUI_SKIN; // set default Gui Skin\n"; // override only if specified
         data+="   EE_INIT_OBJ_TYPE(); // initialize object type enum\n";
         data+="   LoadEmbeddedPaks(EE_PROJECT_CIPHER); // load data embedded in Application executable file\n";
         data+="   if(load_engine_data )if(!EE_ENGINE_EMBED)Paks.add(EE_ENGINE_PATH); // load engine data\n";
         data+="   if(load_project_data) // load project data\n";
         data+="   {\n";
         data+="      if(WINDOWS_NEW || MOBILE || WEB || EE_PUBLISH)Paks.add(EE_PROJECT_PATH, EE_PROJECT_CIPHER);else DataPath(EE_PROJECT_PATH);\n";
         data+="      if(ANDROID && EE_ANDROID_EXPANSION)\n";
         data+="      {\n";
         data+="         REP(EE_APP_BUILD+1)if(Paks.addTry(AndroidExpansionFileName(i), EE_PROJECT_CIPHER))goto added;\n";
         data+="         Exit(\"Can't load Project Data\");\n";
         data+="      added:;\n";
         data+="      }\n";
         data+="   }\n";
         data+="}\n";
         data+="void EE_INIT_OBJ_TYPE() // this function will setup 'ObjType' enum used for object types\n";
         data+="{\n";
         if(obj_types.elms())
         {
            data+="   Enum.Elm elms[]= // list of Editor created Object Classes\n";
            data+="   {\n";
            FREPA(obj_types)
            {
               int length=Length(obj_types[i].name);
               data+=S+"      {\""+CString(obj_types[i].name)+'"'; REP(max_length-length)data+=' '; data+=", "; data+=obj_types[i].id.asCString(); data+="},\n";
            }
            data+="   };\n";
            data+="   ObjType.create(\"OBJ_TYPE\", elms); // create 'ObjType' enum\n";
         }
         data+="}\n";
         data+="/******************************************************************************/\n";
         data+="// ENUMS\n";
         data+="/******************************************************************************/\n";
         // generate enums
         // OBJ_TYPE
         data+="enum OBJ_TYPE // Object Class Types\n";
         data+="{\n";
         FREPA(obj_types)
         {
            int length=Length(obj_types[i].name);
            data+=S+"   "+obj_types[i].name; REP(max_length-length)data+=' '; data+=",\n";
         }
         data+="}\n";
         // custom enums
         FREPA(Proj.existing_enums)
            if(Enum *e=Enums.get(Proj.gamePath(Proj.existing_enums[i])))
            if(Elm *elm=Proj.findElm(Proj.existing_enums[i]))
            if(ElmEnum *enum_data=elm.enumData())
         {
            data+=S+"enum "+e.name;
            switch(enum_data.type)
            {
               case EditEnums.TYPE_1: data+=" : byte"  ; break;
               case EditEnums.TYPE_2: data+=" : ushort"; break;
               case EditEnums.TYPE_4: data+=" : uint"  ; break;
            }
            data+="\n{\n";
            int max_length=0; REPA(*e)MAX(max_length, Length((*e)[i].name));
            FREPA(*e)
            {
               int length=Length((*e)[i].name);
               data+=S+"   "+(*e)[i].name; REP(max_length-length)data+=' '; data+=",\n";
            }
            data+="}\n";
         }
         if(1 || Proj.existing_enums.elms())data+="/******************************************************************************/\n";
         sourceAuto(data);
      }
   }

   void codeDo(Edit.BUILD_MODE mode)
   {
      switch(mode)
      {
         case Edit.BUILD_BUILD  :                 super.build  ();                  break;
         case Edit.BUILD_PUBLISH: makeAuto(true); super.publish(); makeAuto(false); break;
         case Edit.BUILD_PLAY   :                 super.play   ();                  break;
         case Edit.BUILD_DEBUG  :                 super.debug  ();                  break;
      }
   }
   void buildDo(Edit.BUILD_MODE mode)
   {
      if(Demo && (configEXE()==Edit.EXE_NEW || configEXE()==Edit.EXE_APK || configEXE()==Edit.EXE_IOS) && !configDebug()){Gui.msgBox(S, "Demo version doesn't allow creating Mobile applications in Release configuration."); return;}
      if(Demo && configEXE()==Edit.EXE_WEB){Gui.msgBox(S, "Demo version doesn't allow creating Web applications."); return;}
      if(PublishDataNeeded(configEXE(), mode))StartPublish(S, configEXE(), mode);else 
      {
         Proj.flush(); // flush in case we want to play with latest data
         codeDo(mode);
      }
   }
   void build  () {buildDo(Edit.BUILD_BUILD  );}
   void publish() {buildDo(Edit.BUILD_PUBLISH);}
   void play   () {buildDo(Edit.BUILD_PLAY   );}
   void debug  () {buildDo(Edit.BUILD_DEBUG  );}
   void rebuild() {clean(); rebuildSymbols(); build();} // override to call custom 'build'
   void openIDE()
   {
      if(Demo && configEXE()==Edit.EXE_WEB){Gui.msgBox(S, "Demo version doesn't allow creating Web applications."); return;}
      if(PublishDataNeeded(configEXE(), Edit.BUILD_PUBLISH))StartPublish(S, configEXE(), Edit.BUILD_PUBLISH, true, S, true);else // we need to create project data pak first
      {
         Proj.flush(); // flush in case we want to play with latest data
         super.openIDE();
      }
   }
   bool Export(Edit.EXPORT_MODE mode, bool data=false)
   {
      if(Demo && configEXE()==Edit.EXE_WEB){Gui.msgBox(S, "Demo version doesn't allow creating Web applications."); return false;}

      bool ok=false;
      makeAuto(true); // before exporting, reset auto header to force EE_PUBLISH as true, important because exported projects are meant to be distributed to other computers, and compiled for publishing (such as EE Editor), in such case they can't be using paths from this computer, therefore publishing will make them use target paths
      if(ok=super.Export(mode))if(data)
      {
         Edit.EXE_TYPE exe=(Edit.EXE_TYPE)-1;
         switch(mode)
         {
            case Edit.EXPORT_EXE    : exe=configEXE(); break;
            case Edit.EXPORT_ANDROID: exe=Edit.EXE_APK; break;
            case Edit.EXPORT_XCODE  : exe=Edit.EXE_IOS; break;

            case Edit.EXPORT_VS     :
            case Edit.EXPORT_VS2015 :
            case Edit.EXPORT_VS2017 : exe=Edit.EXE_NEW; break;
         }
         if(exe>=0 && PublishDataNeeded(exe, Edit.BUILD_PUBLISH))StartPublish(S, exe, Edit.BUILD_PUBLISH, true);
      }
      makeAuto(false); // restore after exporting
      return ok;
   }

   virtual void publishSuccess(C Str &exe_name, Edit.EXE_TYPE exe_type, Edit.BUILD_MODE build_mode, C UID &project_id)override // code compiled successfully
   {
      if(Proj.id==project_id && StateActive==&StateProject)
      {
         if(PublishDataNeeded(exe_type, build_mode)) // if data was already published then package is ready
         {
            PublishSuccess(exe_name, S+"File name: "+GetBase(exe_name)+"\nFile size: "+FileSize(FSize(exe_name)));
         }else // proceed to data publishing
         {
            StartPublish(exe_name, exe_type, build_mode);
         }
      }
   }

   virtual CodeView& del()override
   {
      CodeEditorInterface.del();
                   Region.del();
      return T;
   }
   static int CompareItem(EEItem *C &a, EEItem *C &b) {return ComparePathNumber(a.full_name, b.full_name);}
   static int CompareItem(EEItem *C &a,     C Str &b) {return ComparePathNumber(a.full_name, b          );}
   void createItems(Memx<EEItem> &dest, C Memx<Edit.Item> &src, EEItem *parent)
   {
      FREPA(src)
      {
       C Edit.Item &s=src[i];
            EEItem &d=dest.New();
         d.base_name=s.base_name;
         d.full_name=s.full_name;
         d.type     =(s.children.elms() ? ELM_FOLDER : ELM_CODE);
         d.icon     =((d.type==ELM_FOLDER) ? Proj.icon_folder : Proj.icon_code);
         d.parent   =parent;
         if(d.full_name.is())items_sorted.binaryInclude(&d, CompareItem);
         createItems(d.children, s.children, &d);
      }
   }
   void create(GuiObj &parent)
   {
      parent+=Region.create();
      CodeEditorInterface.create(parent, CodeMenuOnTop);
      EEItem &ee=items.New();
      ee.base_name="Esenthel Engine";
      ee.type=ELM_LIB;
      ee.icon=Proj.icon_lib;
      createItems(ee.children, CodeEditorInterface.items(), &ee);
      if(C TextNode *code=Settings.findNode("Code"))loadSettings(*code);
   }
   void resize(bool full=true)
   {
      rect(EditRect());
      if(full)CodeEditorInterface.resize();
   }
   void activate(Elm *elm)
   {
      if(elm && elm.type==ELM_CODE)
      {
         sourceCur(elm.id); Mode.set(MODE_CODE);
      }
   }
   void toggle(Elm *elm)
   {
      if(elm && elm.type==ELM_CODE)
      {
         if(sourceCurId()!=elm.id)sourceCur(elm.id);else if(selected())close();else Mode.set(MODE_CODE);
      }
   }
   void toggle(EEItem *item)
   {
      if(item && item.type==ELM_CODE && item.full_name.is())
      {
         if(!EqualPath(sourceCurName(), item.full_name))sourceCur(item.full_name);else if(selected())close();else Mode.set(MODE_CODE);
      }
   }
   void erasing(C UID &elm_id) {sourceRemove(elm_id);}
   void kbSet() {CodeEditorInterface.kbSet();}

   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}
   virtual void    update(C GuiPC &gpc)override
   {
    //Region             .update(gpc);
      CodeEditorInterface.update(StateActive==&StateProject);
   }
   virtual void draw(C GuiPC &gpc)override
   {
   }
   void drawPreview(ListElm *list_elm)
   {
      if(EEItem *item=list_elm.item)
         if(item.type==ELM_CODE)
            if(item.full_name.is())
               if(selected() ? !EqualPath(sourceCurName(), item.full_name) : true) // don't preview source which is currently being edited
                  sourceDrawPreview(item.full_name);

      if(Elm *elm=list_elm.elm)
         if(ElmCode *code_data=elm.codeData())
            if(selected() ? sourceCurId()!=elm.id : true) // don't preview source which is currently being edited
               sourceDrawPreview(elm.id);
   }
}
CodeView CodeEdit;
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
   }
   static cchar8 *OrientName[]=
   { 
      "Portrait",
      "Landscape",
      "Portrait and Landscape",
      "Portrait and Landscape (No Down)",
      "Locked Portrait",
      "Locked Landscape",
   };
   static NameDesc EmbedEngine[]=
   {
      {u"No"     , u"Engine data will not be embedded in the Application executable, instead, a standalone \"Engine.pak\" file will get created, which the Application has to load at startup."},
      {u"2D Only", u"Engine data will be embedded in the Application executable, and standalone \"Engine.pak\" file will not get created.\nHowever this option embeds only simple data used for 2D graphics only, shaders used for 3D graphics will not be included, if your application will attempt to render 3D graphics an error will occur."},
      {u"Full"   , u"Engine data will be embedded in the Application executable, and standalone \"Engine.pak\" file will not get created.\nAll engine data will get embedded, including support for 2D and 3D graphics."},
   };
   static NameDesc StorageName[]=
   {
      {u"Internal", u"The application must be installed on the internal device storage only. If this is set, the application will never be installed on the external storage. If the internal storage is full, then the system will not install the application."},
      {u"External", u"The application prefers to be installed on the external storage (SD card). There is no guarantee that the system will honor this request. The application might be installed on internal storage if the external media is unavailable or full, or if the application uses the forward-locking mechanism (not supported on external storage). Once installed, the user can move the application to either internal or external storage through the system settings."},
      {u"Auto"    , u"The application may be installed on the external storage, but the system will install the application on the internal storage by default. If the internal storage is full, then the system will install it on the external storage. Once installed, the user can move the application to either internal or external storage through the system settings."},
   }; ASSERT(Edit.STORAGE_INTERNAL==0 && Edit.STORAGE_EXTERNAL==1 && Edit.STORAGE_AUTO==2);
   static ORIENT FlagToOrient(uint flag)
   {
      bool portrait =FlagTest(flag, DIRF_Y),
           landscape=FlagTest(flag, DIRF_X);
      if( flag==(DIRF_X|DIRF_UP)  )return ORIENT_ALL_NO_DOWN     ;
      if((flag&DIRF_X)==DIRF_RIGHT)return ORIENT_LANDSCAPE_LOCKED;
      if((flag&DIRF_Y)==DIRF_UP   )return ORIENT_PORTRAIT_LOCKED ;
      if( landscape && !portrait  )return ORIENT_LANDSCAPE       ;
      if(!landscape &&  portrait  )return ORIENT_PORTRAIT        ;
                                   return ORIENT_ALL             ;
   }
   static uint OrientToFlag(ORIENT orient)
   {
      switch(orient)
      {
         default                     : return DIRF_X|DIRF_Y ; // ORIENT_ALL
         case ORIENT_ALL_NO_DOWN     : return DIRF_X|DIRF_UP;
         case ORIENT_PORTRAIT        : return        DIRF_Y ;
         case ORIENT_LANDSCAPE       : return DIRF_X        ;
         case ORIENT_PORTRAIT_LOCKED : return DIRF_UP       ;
         case ORIENT_LANDSCAPE_LOCKED: return DIRF_RIGHT    ;
      }
   }

   class AppImage : ImageSkin
   {
      Button      remove;
      Image       image_2d;
      ImagePtr    game_image;
      MemberDesc  md, md_time;

      static void Remove(AppImage &ai) {ai.setImage(UIDZero);}

      void setImage()
      {
         ElmApp *app=(AppPropsEdit.elm ? AppPropsEdit.elm.appData() : null);
         UID     id=md.asUID(app);
         if(id.valid())game_image=Proj.gamePath(id);else game_image=null;
         image=game_image; image_2d.del();
         if(image && image->is() && image->mode()!=IMAGE_2D)
         {
            VecI2 size=image->size();
            if(size.x>256)size=size*256/size.x;
            if(size.y>256)size=size*256/size.y;
            image->copyTry(image_2d, Max(size.x, 1), Max(size.y, 1), 1, ImageTI[image->type()].compressed ? IMAGE_R8G8B8A8 : image->type(), IMAGE_2D, 1, FILTER_BEST, true, true);
            image=&image_2d;
         }
         remove.visible(id.valid());
         desc(S+"\""+Proj.elmFullName(id)+"\"\nDrag and drop an image here");
      }
      void setImage(C UID &image_id)
      {
         if(ElmApp *app=(AppPropsEdit.elm ? AppPropsEdit.elm.appData() : null))
         {
            md.fromUID(app, image_id); md_time.as<TimeStamp>(app).getUTC(); AppPropsEdit.setChanged();
            setImage();
         }
      }
      AppImage& create(C MemberDesc &md, C MemberDesc &md_time, GuiObj &parent, C Rect &rect)
      {
         T.md=md;
         T.md_time=md_time;
         parent+=super.create(rect); fit=true;
         parent+=remove.create(Rect_RU(rect.ru(), 0.045, 0.045)).func(Remove, T); remove.image="Gui/close.img";
         return T;
      }
   }

   UID       elm_id=UIDZero;
   Elm      *elm=null;
   bool      changed=false, changed_headers=false;
   Property *p_icon=null, *p_image_portrait=null, *p_image_landscape=null, *p_notification_icon;
   AppImage    icon     ,    image_portrait     ,    image_landscape     ,    notification_icon;
   Tabs      platforms, platforms2;

   static void Changed             (C Property &prop) {AppPropsEdit.setChanged();}
   static void GetAndroidLicenseKey(  ptr           ) {Explore("https://play.google.com/apps/publish/");}
   static void GetFacebookAppID    (  ptr           ) {Explore("https://developers.facebook.com/apps");}
   static void GetChartboostApp    (  ptr           ) {Explore("https://dashboard.chartboost.com/tools/sdk");}

   static void DirsWin                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_windows=text; app_data.dirs_windows_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsWin                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_windows; return S;}
   static void DirsNonWin                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_nonwindows=text; app_data.dirs_nonwindows_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsNonWin                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_nonwindows; return S;}
   static void HeadersWin                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_windows=text; app_data.headers_windows_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersWin                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_windows; return S;}
   static void HeadersMac                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_mac=text; app_data.headers_mac_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersMac                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_mac; return S;}
   static void HeadersLinux                (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_linux=text; app_data.headers_linux_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersLinux                (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_linux; return S;}
   static void HeadersAndroid              (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_android=text; app_data.headers_android_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersAndroid              (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_android; return S;}
   static void HeadersiOS                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_ios=text; app_data.headers_ios_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersiOS                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_ios; return S;}
   static void LibsWindows                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_windows=text; app_data.libs_windows_time.getUTC();}}
   static Str  LibsWindows                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_windows; return S;}
   static void LibsMac                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_mac=text; app_data.libs_mac_time.getUTC();}}
   static Str  LibsMac                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_mac; return S;}
   static void LibsLinux                   (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_linux=text; app_data.libs_linux_time.getUTC();}}
   static Str  LibsLinux                   (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_linux; return S;}
   static void LibsAndroid                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_android=text; app_data.libs_android_time.getUTC();}}
   static Str  LibsAndroid                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_android; return S;}
   static void LibsiOS                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_ios=text; app_data.libs_ios_time.getUTC();}}
   static Str  LibsiOS                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_ios; return S;}
   static void Package                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.package=text; app_data.package_time.getUTC();}}
   static Str  Package                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.package; return S;}
   static void AndroidLicenseKey           (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.android_license_key=text; app_data.android_license_key_time.getUTC();}}
   static Str  AndroidLicenseKey           (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.android_license_key; return S;}
   static void Build                       (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.build=TextInt(text); app_data.build_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  Build                       (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.build; return S;}
   static void LocationUsageReason         (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.location_usage_reason=text; app_data.location_usage_reason_time.getUTC();}}
   static Str  LocationUsageReason         (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.location_usage_reason; return S;}
   static void FacebookAppID               (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.fb_app_id=TextULong(text); app_data.fb_app_id_time.getUTC();}}
   static Str  FacebookAppID               (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.fb_app_id)return app_data.fb_app_id; return S;}
   static void ChartboostAppIDiOS          (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_id_ios=text; app_data.cb_app_id_ios_time.getUTC();}}
   static Str  ChartboostAppIDiOS          (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.cb_app_id_ios)return app_data.cb_app_id_ios; return S;}
   static void ChartboostAppSignatureiOS   (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_signature_ios=text; app_data.cb_app_signature_ios_time.getUTC();}}
   static Str  ChartboostAppSignatureiOS   (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.cb_app_signature_ios)return app_data.cb_app_signature_ios; return S;}
   static void ChartboostAppIDGoogle       (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_id_google=text; app_data.cb_app_id_google_time.getUTC();}}
   static Str  ChartboostAppIDGoogle       (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.cb_app_id_google)return app_data.cb_app_id_google; return S;}
   static void ChartboostAppSignatureGoogle(  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_signature_google=text; app_data.cb_app_signature_google_time.getUTC();}}
   static Str  ChartboostAppSignatureGoogle(C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.cb_app_signature_google)return app_data.cb_app_signature_google; return S;}
   static void Storage                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.storage=Edit.STORAGE_MODE(TextInt(text)); app_data.storage_time.getUTC();}}
   static Str  Storage                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.storage; return S;}
   static void GuiSkin                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.gui_skin=Proj.findElmID(text, ELM_GUI_SKIN); app_data.gui_skin_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  GuiSkin                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return Proj.elmFullName(app_data.gui_skin); return S;}
   static void EmbedEngineData             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.embedEngineData(TextInt(text)).embed_engine_data_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  EmbedEngineData             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.embedEngineData(); return S;}
   static void PublishProjData             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishProjData(TextBool(text)).publish_proj_data_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishProjData             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishProjData(); return S;}
   static void PublishDataAsPak            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishDataAsPak(TextBool(text)).publish_data_as_pak_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishDataAsPak            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishDataAsPak(); return S;}
   static void AndroidExpansion            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.androidExpansion(TextBool(text)).android_expansion_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  AndroidExpansion            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.androidExpansion(); return S;}
   static void PublishPhysxDll             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishPhysxDll(TextBool(text)).publish_physx_dll_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishPhysxDll             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishPhysxDll(); return S;}
   static void PublishSteamDll             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishSteamDll(TextBool(text)).publish_steam_dll_time.getUTC(); if(ap.elm_id==Proj.curApp()){CodeEdit.makeAuto(); CodeEdit.rebuildSymbols();}}}
   static Str  PublishSteamDll             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishSteamDll(); return S;}
   static void PublishOpenVRDll            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishOpenVRDll(TextBool(text)).publish_open_vr_dll_time.getUTC(); if(ap.elm_id==Proj.curApp()){CodeEdit.makeAuto(); CodeEdit.rebuildSymbols();}}}
   static Str  PublishOpenVRDll            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishOpenVRDll(); return S;}
 //static void WindowsCodeSign             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.windowsCodeSign(TextBool(text)).windows_code_sign_time.getUTC();}}
 //static Str  WindowsCodeSign             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.windowsCodeSign(); return S;}
   static void Orientation                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.supported_orientations=OrientToFlag(ORIENT(TextInt(text))); app_data.supported_orientations_time.getUTC();}}
   static Str  Orientation                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return FlagToOrient(app_data.supported_orientations); return S;}

   static cchar8 *platforms_t[]=
   {
      "Windows",
      "Mac",
      "Linux",
      "Android",
      "iOS",
   };
   static cchar8 *platforms2_t[]=
   {
      "On Windows",
      "Non Windows",
   };

   void create()
   {
      flt h=0.05;
      ListColumn name_desc_column[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      PropEx &ihw      =add("Include Headers"         , MemberDesc(DATA_STR                               ).setFunc(HeadersWin                  , HeadersWin                  )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      PropEx &ihm      =add("Include Headers"         , MemberDesc(DATA_STR                               ).setFunc(HeadersMac                  , HeadersMac                  )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      PropEx &ihl      =add("Include Headers"         , MemberDesc(DATA_STR                               ).setFunc(HeadersLinux                , HeadersLinux                )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      PropEx &iha      =add("Include Headers"         , MemberDesc(DATA_STR                               ).setFunc(HeadersAndroid              , HeadersAndroid              )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      PropEx &ihi      =add("Include Headers"         , MemberDesc(DATA_STR                               ).setFunc(HeadersiOS                  , HeadersiOS                  )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      PropEx &ilw      =add("Include Libraries"       , MemberDesc(DATA_STR                               ).setFunc(LibsWindows                 , LibsWindows                 )).desc("Type full paths to lib file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.lib | C:\\Lib2\\Main.lib");
      PropEx &ilm      =add("Include Libraries"       , MemberDesc(DATA_STR                               ).setFunc(LibsMac                     , LibsMac                     )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      PropEx &ill      =add("Include Libraries"       , MemberDesc(DATA_STR                               ).setFunc(LibsLinux                   , LibsLinux                   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      PropEx &ila      =add("Include Libraries"       , MemberDesc(DATA_STR                               ).setFunc(LibsAndroid                 , LibsAndroid                 )).desc("Type full paths to lib file names.\nSeparate each with | for example:\nC:\\Lib1\\XXX.a | C:\\Lib2\\libXXX.so\n\n$(TARGET_ARCH_ABI) can be used in the path, which will be replaced with target architecture (such as armeabi-v7a, arm64-v8a, x86, x86_64), for example:\nC:\\Path\\XXX-$(TARGET_ARCH_ABI).a\nC:\\Path\\$(TARGET_ARCH_ABI)\\libXXX.so");
      PropEx &ili      =add("Include Libraries"       , MemberDesc(DATA_STR                               ).setFunc(LibsiOS                     , LibsiOS                     )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      PropEx &cb_ai_g  =add("Chartboost App ID"       , MemberDesc(MEMBER(ElmApp, cb_app_id_google       )).setFunc(ChartboostAppIDGoogle       , ChartboostAppIDGoogle       )).desc("Chartboost Application ID");
      PropEx &cb_as_g  =add("Chartboost App Signature", MemberDesc(MEMBER(ElmApp, cb_app_signature_google)).setFunc(ChartboostAppSignatureGoogle, ChartboostAppSignatureGoogle)).desc("Chartboost Application Signature");
      PropEx &google_lk=add("License Key"             , MemberDesc(DATA_STR                               ).setFunc(AndroidLicenseKey           , AndroidLicenseKey           )).desc("Google Play app license key.\nThis key is used for verification of purchases in Google Play Store, and for downloading Expansion Files hosted in Google Play.\nYou can obtain this key from \"Google Play Developer Console website \\ Your App \\ Services & APIs \\ YOUR LICENSE KEY FOR THIS APPLICATION\".\nUpon providing your license key, all purchases will be automatically verified and only those that pass the verification test will be returned.\nIf you don't specify your key then all purchases will be listed without any verification and you will not be able to download Expansion Files.");
      PropEx &storage  =add("Preferred Storage"       , MemberDesc(DATA_INT                               ).setFunc(Storage                     , Storage                     )).setEnum().desc("Preferred installation location for the application\n\nInternal - The application must be installed on the internal device storage only. If this is set, the application will never be installed on the external storage. If the internal storage is full, then the system will not install the application.\n\nExternal - The application prefers to be installed on the external storage (SD card). There is no guarantee that the system will honor this request. The application might be installed on internal storage if the external media is unavailable or full, or if the application uses the forward-locking mechanism (not supported on external storage). Once installed, the user can move the application to either internal or external storage through the system settings.\n\nAuto - The application may be installed on the external storage, but the system will install the application on the internal storage by default. If the internal storage is full, then the system will install it on the external storage. Once installed, the user can move the application to either internal or external storage through the system settings."); storage.combobox.setColumns(name_desc_column, Elms(name_desc_column)).setData(StorageName, Elms(StorageName)).menu.list.setElmDesc(MEMBER(NameDesc, desc));
      PropEx &cb_ai_i  =add("Chartboost App ID"       , MemberDesc(MEMBER(ElmApp, cb_app_id_ios          )).setFunc(ChartboostAppIDiOS          , ChartboostAppIDiOS          )).desc("Chartboost Application ID");
      PropEx &cb_as_i  =add("Chartboost App Signature", MemberDesc(MEMBER(ElmApp, cb_app_signature_ios   )).setFunc(ChartboostAppSignatureiOS   , ChartboostAppSignatureiOS   )).desc("Chartboost Application Signature");
      PropEx &loc_usage=add("Location Usage Reason"   , MemberDesc(DATA_STR                               ).setFunc(LocationUsageReason         , LocationUsageReason         )).desc("Reason for accessing the user's location information.\nThis is needed for iOS (on other platforms this is ignored).\nThis will be displayed on the user screen when trying to access the Location.");
      PropEx &idw      =add("Include Directories"     , MemberDesc(DATA_STR                               ).setFunc(DirsWin                     , DirsWin                     )).desc("Type full paths to additional include directories.\nThis is used when compiling from the Windows platform.\nSeparate each with | for example:\nC:\\Lib1 | C:\\Lib2");
      PropEx &idn      =add("Include Directories"     , MemberDesc(DATA_STR                               ).setFunc(DirsNonWin                  , DirsNonWin                  )).desc("Type full paths to additional include directories.\nThis is used when compiling from platform other than Windows.\nSeparate each with | for example:\n/Lib2 | /Lib2");
      PropEx &first    =add("Package Name"            , MemberDesc(DATA_STR                               ).setFunc(Package                     , Package                     )).desc("Application package name.\nMust be in following format: \"com.company_name.app_name\"\nWhere 'company_name' is the name of developer/company,\nand 'app_name' is the name of the application.\n\nThe package name should be unique.\nThe name parts may contain uppercase or lowercase letters 'A' through 'Z', numbers, hyphens '-' and underscores '_'.\n\nOnce you publish your application, you cannot change the package name.\nThe package name defines your application's identity,\nso if you change it, then it is considered to be a different application\nand users of the previous version cannot update to the new version.");
                        add("Build Number"            , MemberDesc(DATA_INT                               ).setFunc(Build                       , Build                       )).desc("Application build number.\nUsed to identify the version of the application.\nThis must be specified in order for the application to update correctly through online stores.\nTypically you should increase this value by 1 when making each new release.").min(1).mouseEditSpeed(2);
    Property &fb_app_id=add("Facebook App ID"         , MemberDesc(MEMBER(ElmApp, fb_app_id              )).setFunc(FacebookAppID               , FacebookAppID               )).desc("Facebook Application ID").mouseEditDel();
                        add("Supported Orientations"  , MemberDesc(DATA_INT                               ).setFunc(Orientation                 , Orientation                 )).setEnum(OrientName, Elms(OrientName)).desc("Supported orientations for mobile platforms");
                        add("Default Gui Skin"        , MemberDesc(MEMBER(ElmApp, gui_skin               )).setFunc(GuiSkin                     , GuiSkin                     )).elmType(ELM_GUI_SKIN).desc("Set default Gui Skin used by this Application.\nGui Skin will be loaded during Application Engine initialization stage.");
      PropEx &embed    =add("Embed Engine Data"       , MemberDesc(DATA_INT                               ).setFunc(EmbedEngineData             , EmbedEngineData             )).setEnum().desc("If embed engine data into the application executable file, so it doesn't require separate \"Engine.pak\" file.\nThis option is recommended for applications that want to be distributed as standalone executables without any additional files.\nThis option is ignored for Mobile and Web builds.\nDefault value for this option is \"No\"."); embed.combobox.setColumns(name_desc_column, Elms(name_desc_column)).setData(EmbedEngine, Elms(EmbedEngine)).menu.list.setElmDesc(MEMBER(NameDesc, desc));
                        add("Android Expansion Files" , MemberDesc(DATA_BOOL                              ).setFunc(AndroidExpansion            , AndroidExpansion            )).desc("If automatically download Android Expansion Files from Google Play for this APK before starting the App.\nWhen this option is enabled then the engine will automatically download any Expansion Files hosted on Google Play that are needed for this Android APK.\nThe application will be started only after the Expansion Files have been downloaded, which means that once your game codes are executed, the Expansion Files will already be in place.\nYou can access the Expansion Files from code using 'AndroidExpansionFileName' function.\nMain Expansion File will be automatically loaded in 'EE_INIT' function.\nTypically when enabling this option you should disable \"Publish Project Data\" so the Project Data is not included in the APK, and manually export the Data using \"Build\\Export\\Project data optimized for Android\".\nDefault value for this option is false.");
                        add("Publish Project Data"    , MemberDesc(DATA_BOOL                              ).setFunc(PublishProjData             , PublishProjData             )).desc("If include project data when publishing the application.\nDisable this if your application will not initially include the data, but will download it manually later.\nDefault value for this option is true.");
                        add("Publish Data as PAK's"   , MemberDesc(DATA_BOOL                              ).setFunc(PublishDataAsPak            , PublishDataAsPak            )).desc("If archive data files into PAK's.\nDisable this option if you plan to upload the application using Uploader tool, in which case it's better that the files are stored separately (instead of archived).\nThen you can make your Installer to download files from the server, and archive them as PAK's.\nDefault value for this option is true.");
           if(PHYSX_DLL)add("Publish PhysX DLL's"     , MemberDesc(DATA_BOOL                              ).setFunc(PublishPhysxDll             , PublishPhysxDll             )).desc("If include PhysX DLL files when publishing the application.\nDisable this if your application doesn't use physics, or it will download the files manually later.\nThis option is used only for EXE and DLL targets.\nDefault value for this option is true.");
                        add("Publish Steam DLL"       , MemberDesc(DATA_BOOL                              ).setFunc(PublishSteamDll             , PublishSteamDll             )).desc("If include Steam DLL file when publishing the application.\nEnable this only if your application uses Steam API.\nDefault value for this option is false.\nBased on this option, \"STEAM\" C++ macro will be set to 0 or 1.");
                        add("Publish OpenVR DLL"      , MemberDesc(DATA_BOOL                              ).setFunc(PublishOpenVRDll            , PublishOpenVRDll            )).desc("If include OpenVR DLL file when publishing the application.\nEnable this only if your application uses OpenVR API.\nDefault value for this option is false.\nBased on this option, \"OPEN_VR\" C++ macro will be set to 0 or 1.");
                      //add("Windows Code Sign"       , MemberDesc(DATA_BOOL                              ).setFunc(WindowsCodeSign             , WindowsCodeSign             )).desc("If automatically sign the application when publishing for Windows EXE platform.\nWindows signtool.exe must be installed together with your Microsoft Windows Authenticode Digital Signature in the Certificate Store.\nSign tool will be used with the /a option making it to choose the best certificate out of all available.");
                p_icon=&add("Icon"             );
      p_image_portrait=&add("Portrait Image"   );
     p_image_landscape=&add("Landscape Image"  );
   p_notification_icon=&add("Notification Icon");
      autoData(this);

      Rect rect=super.create(S, Vec2(0.02, -0.02), 0.04, h, 0.85); super.changed(Changed); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      Vec2 pos(0.02, -0.02);
      flt  th=google_lk.textline.rect().h();
      T+=platforms.create(platforms_t, Elms(platforms_t)).valid(true).set(0).rect(Rect_LU(pos, clientWidth()-0.04, 0.05)); pos.y-=0.05;
      pos.y-=0.005+h/2;
      ihw.pos(pos).parent(platforms.tab(0));
      ihm.pos(pos).parent(platforms.tab(1));
      ihl.pos(pos).parent(platforms.tab(2));
      iha.pos(pos).parent(platforms.tab(3));
      ihi.pos(pos).parent(platforms.tab(4));
      pos.y-=h;
      ilw.pos(pos).parent(platforms.tab(0));
      ilm.pos(pos).parent(platforms.tab(1));
      ill.pos(pos).parent(platforms.tab(2));
      ila.pos(pos).parent(platforms.tab(3));
      ili.pos(pos).parent(platforms.tab(4));
      pos.y-=h;
      cb_ai_g.pos(pos).parent(platforms.tab(3)); platforms.tab(3)+=cb_ai_g.button.create(Rect_RU(cb_ai_g.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_ai_g.textline.rect(Rect(cb_ai_g.textline.rect().ld(), cb_ai_g.button.rect().lu()));
      cb_ai_i.pos(pos).parent(platforms.tab(4)); platforms.tab(4)+=cb_ai_i.button.create(Rect_RU(cb_ai_i.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_ai_i.textline.rect(Rect(cb_ai_i.textline.rect().ld(), cb_ai_i.button.rect().lu()));
      pos.y-=h;
      cb_as_g.pos(pos).parent(platforms.tab(3)); platforms.tab(3)+=cb_as_g.button.create(Rect_RU(cb_as_g.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_as_g.textline.rect(Rect(cb_as_g.textline.rect().ld(), cb_as_g.button.rect().lu()));
      cb_as_i.pos(pos).parent(platforms.tab(4)); platforms.tab(4)+=cb_as_i.button.create(Rect_RU(cb_as_i.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_as_i.textline.rect(Rect(cb_as_i.textline.rect().ld(), cb_as_i.button.rect().lu()));
      pos.y-=h;
      google_lk.pos(pos).parent(platforms.tab(3)); platforms.tab(3)+=google_lk.button.create(Rect_RU(google_lk.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetAndroidLicenseKey); google_lk.textline.rect(Rect(google_lk.textline.rect().ld(), google_lk.button.rect().lu()));
      loc_usage.pos(pos).parent(platforms.tab(4));
      pos.y-=h;
      storage.pos(pos).parent(platforms.tab(3));
      pos.y-=h*1.5;
      T+=platforms2.create(platforms2_t, Elms(platforms2_t)).valid(true).set(0).rect(Rect_LU(pos, clientWidth()-0.04, 0.05)); pos.y-=0.05;
      pos.y-=0.005+h/2;
      idw.pos(pos).parent(platforms2.tab(0)); idn.pos(pos).parent(platforms2.tab(1)); pos.y-=h;
      pos.y-=h;

      for(int i=props.validIndex(&first); i<props.elms(); i++, pos.y-=h)props[i].pos(pos);
      T+=fb_app_id.button.create(Rect_RU(fb_app_id.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetFacebookAppID); fb_app_id.textline.rect(Rect(fb_app_id.textline.rect().ld(), fb_app_id.button.rect().lu()));

      p_image_portrait   .move(Vec2(rect.w()  /3, h  ));
      p_image_landscape  .move(Vec2(rect.w()*2/3, h*2));
      p_notification_icon.move(Vec2(rect.w()*2/3, h*8));
      icon             .create(MEMBER(ElmApp, icon             ), MEMBER(ElmApp,              icon_time), T, Rect_LU(p_icon             .name.rect().left()-Vec2(0, h/2), 0.3));
      image_portrait   .create(MEMBER(ElmApp, image_portrait   ), MEMBER(ElmApp,    image_portrait_time), T, Rect_LU(p_image_portrait   .name.rect().left()-Vec2(0, h/2), 0.3));
      image_landscape  .create(MEMBER(ElmApp, image_landscape  ), MEMBER(ElmApp,   image_landscape_time), T, Rect_LU(p_image_landscape  .name.rect().left()-Vec2(0, h/2), 0.3));
      notification_icon.create(MEMBER(ElmApp, notification_icon), MEMBER(ElmApp, notification_icon_time), T, Rect_LU(p_notification_icon.name.rect().left()-Vec2(0, h/2), 0.15));
      clientRect(Rect_C(0, 0, 1.3, -icon.rect().min.y+0.02));
   }
   void toGui()
   {
      super.toGui();
      icon             .setImage();
      image_portrait   .setImage();
      image_landscape  .setImage();
      notification_icon.setImage();
   }

   virtual AppPropsEditor& hide()override {set(null); super.hide(); return T;}

   void flush()
   {
      if(elm && changed)
      {
       //if(ElmApp *data=elm.appData())data.newVer(); Server.setElmLong(elm.id); // modify just before saving/sending in case we've received data from server after edit, don't send to server as apps/codes are synchronized on demand
         if(changed_headers && elm_id==Proj.curApp())Proj.activateSources(1); // if changed headers on active app then rebuild symbols
      }
      changed=false;
      changed_headers=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmApp *data=elm.appData())data.newVer();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_APP)elm=null;
      if(T.elm!=elm)
      {
         flush();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
         if(elm)setTitle(S+'"'+elm.name+"\" Settings");
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)toGui();
      if(elm_id==Proj.curApp())CodeEdit.makeAuto();
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(obj==&icon || obj==&image_portrait || obj==&image_landscape || obj==&notification_icon)REPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmImageLike(elm.type))
      {
         if(obj==&icon             )icon             .setImage(elm.id);
         if(obj==&image_portrait   )image_portrait   .setImage(elm.id);
         if(obj==&image_landscape  )image_landscape  .setImage(elm.id);
         if(obj==&notification_icon)notification_icon.setImage(elm.id);
         break;
      }
   }
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(obj==&icon || obj==&image_portrait || obj==&image_landscape || obj==&notification_icon)
         Gui.msgBox(S, "Application images must be set from project elements and not from files.\nPlease drag and drop your file to the project, then select \"Disable Publishing\" so that it will not additionally increase the game data size.\nThen drag and drop the project element onto the application image slot.");
   }
}
AppPropsEditor AppPropsEdit;
/******************************************************************************/
