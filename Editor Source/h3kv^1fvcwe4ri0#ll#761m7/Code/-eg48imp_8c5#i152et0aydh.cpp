/******************************************************************************/
class MiscRegion : Region
{
   Button    hide_proj, vid_opt, move_misc, play;
   ComboBox  menu, build;
   TextWhite online_ts;
   Text      online;
   byte      screenshot=0;
   VecI2     pos(0, MiscOnTop);

   static void Play       (MiscRegion &mr) {CodeEdit.play       ();}
   static void Debug      (MiscRegion &mr) {CodeEdit.debug      ();}
   static void RunToCursor(MiscRegion &mr) {CodeEdit.runToCursor();}
   static void Build      (MiscRegion &mr) {CodeEdit.build      ();}
   static void Rebuild    (MiscRegion &mr) {CodeEdit.rebuild    ();}
   static void Clean      (MiscRegion &mr) {CodeEdit.clean      ();}
   static void CleanAll   (MiscRegion &mr) {CodeEdit.cleanAll   ();}
   static void Stop       (MiscRegion &mr) {CodeEdit.stop       ();}
   static void OpenIDE    (MiscRegion &mr) {CodeEdit.openIDE    ();}

   static void ExportTXT          (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_TXT    );}
   static void ExportCPP          (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_CPP    );}
 /*static void ExportVS2008       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2008 );}
   static void ExportVS2010       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2010 );}
   static void ExportVS2012       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2012 );}
   static void ExportVS2013       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2013 );}*/
   static void ExportVS2015       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2015 );}
   static void ExportVS2017       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2017 );}
   static void ExportAndroid      (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_ANDROID);}
   static void ExportAndroidData  (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_ANDROID, true);}
   static void ExportXcode        (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_XCODE  );}
   static void ExportXcodeDataiOS (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_XCODE  , true);}
   static void ExportLinuxMake    (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_LINUX_MAKE    );}
   static void ExportLinuxNetBeans(MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_LINUX_NETBEANS);}
   static void ExportData         (MiscRegion &mr) {.Publish.exportData();}
   static void ExportDataAndroid  (MiscRegion &mr) {.Publish.exportData(Edit.EXE_APK);}
   static void ExportDataiOS      (MiscRegion &mr) {.Publish.exportData(Edit.EXE_IOS);}

   static void CodeExplore(MiscRegion &mr) {Proj.codeExplore();}
   static void CodeImport (MiscRegion &mr) {Proj.codeImport (true);}
   static void CodeExport (MiscRegion &mr) {Proj.codeExport (true);}

   static void ConfigDebug  (MiscRegion &mr) {CodeEdit.configDebug(true );}
   static void ConfigRelease(MiscRegion &mr) {CodeEdit.configDebug(false);}
   static void Config32     (MiscRegion &mr) {CodeEdit.config32Bit(true );}
   static void Config64     (MiscRegion &mr) {CodeEdit.config32Bit(false);}
   static void ConfigDX9    (MiscRegion &mr) {CodeEdit.configDX9  (true );}
   static void ConfigDX11   (MiscRegion &mr) {CodeEdit.configDX9  (false);}
   static void ConfigEXE    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_EXE  );}
   static void ConfigDLL    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_DLL  );}
   static void ConfigLIB    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_LIB  );}
   static void ConfigNEW    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_NEW  );}
   static void ConfigAPK    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_APK  );}
   static void ConfigMAC    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_MAC  );}
   static void ConfigIOS    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_IOS  );}
   static void ConfigLinux  (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_LINUX);}
   static void ConfigWeb    (MiscRegion &mr) {CodeEdit.configEXE  (Edit.EXE_WEB  );}

   static void ProjList       (bool all_saved=true, ptr=null) {if(all_saved)StateProjectList.set(StateFadeTime);}
   static void ProjList       (MiscRegion &mr) {SaveChanges(ProjList);}
   static void   HideProj     (MiscRegion &mr) {Proj.visible(!mr.hide_proj());}
   static void ToggleProj     (MiscRegion &mr) {mr.hide_proj.push();}
   static void ToggleTheater  (MiscRegion &mr) {Proj.theater.push();}
   static void ToggleRemoved  (MiscRegion &mr) {Proj.show_removed.push();}
   static void   FindProj     (MiscRegion &mr) {if(mr.hide_proj())mr.hide_proj.push(); Proj.filter.selectAll().activate();}
   static void   ModeClose    (MiscRegion &mr) {Mode.closeActive(false);}
   static void   Mode0        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(0))tab.push();}
   static void   Mode1        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(1))tab.push();}
   static void   Mode2        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(2))tab.push();}
   static void   Mode3        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(3))tab.push();}
   static void   Mode4        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(4))tab.push();}
   static void   Mode5        (MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(5))tab.push();}
   static void VidOpt         (MiscRegion &mr) {.VidOpt.visibleToggleActivate();}
   static void BuyLicense     (ptr           ) {Explore("http://www.esenthel.com/?id=store&cat=0");}
   static void Help           (MiscRegion &mr) {Explore("http://www.esenthel.com/forum/");}
   static void Fullscreen     (MiscRegion &mr) {D.toggle();}
   static void Screenshot     (MiscRegion &mr) {mr.screenshot=5;}
   static void CalcShow       (MiscRegion &mr) {Calculator.visibleToggleActivate();}
   static void VerifyElms     (MiscRegion &mr) {}
   static void ProjSettings   (MiscRegion &mr) {.ProjSettings.display();}
   static void Publish        (MiscRegion &mr) {PublishDo();}
   static void PublishEsProj  (MiscRegion &mr) {if(!PublishEsProjIO.is())PublishEsProjIO.create(EsenthelProjectExt, S, SystemPath(SP_DESKTOP), null, .PublishEsProjAs); PublishEsProjIO.save().name(CleanFileName(Proj.name));}
 //static void SetLicense     (MiscRegion &mr) {LicenseCheck.activate();}
   static void AppStore       (MiscRegion &mr) {.AppStore.activate();}
   static void EraseRemoved   (MiscRegion &mr) {.EraseRemoved.activate();}
   static void MergeSimMtrl   (MiscRegion &mr) {MSM         .activate();}
   static void DetectSimTex   (MiscRegion &mr) {DST         .activate();}
   static void CreateMtrls    (MiscRegion &mr) {.CreateMtrls.activate();}
   static void DataSizeStats  (MiscRegion &mr) {SizeStats   .display(S);}
   static void UnusedMaterials(MiscRegion &mr) {SizeStats   .displayUnusedMaterials();}
   static void ShowTexDownsize(MiscRegion &mr) {TexDownsize .toggle();}
   static void SyncCodes      (MiscRegion &mr) {Proj.syncCodes();}
   static void EditorOptions  (MiscRegion &mr) {CodeEdit.visibleOptions      (!CodeEdit.visibleOptions      ());}
   static void CodeOutput     (MiscRegion &mr) {CodeEdit.visibleOutput       (!CodeEdit.visibleOutput       ());}
   static void CodeDevLog     (MiscRegion &mr) {CodeEdit.visibleAndroidDevLog(!CodeEdit.visibleAndroidDevLog());}
   static void Quit           (MiscRegion &mr) {App.close();}
   static void About          (MiscRegion &mr)
   {
      Str about=S+AppName+(STEAM ? " Steam Edition" : "")+"\nEngine Build: "+ENGINE_BUILD+", Editor Build: "+EE_APP_BUILD+", Network Protocol: "+ClientServerVersion;
      about+='\n'; about+=D.apiName();
      if(D.shaderModel()!=SM_GL)about+=S+" Shader Model "+D.shaderModelName();
      about+=(X64 ? ", 64-bit" : ", 32-bit");
      switch(Physics.engine())
      {
         case PHYS_ENGINE_PHYSX : about+=", PhysX" ; break;
         case PHYS_ENGINE_BULLET: about+=", Bullet"; break;
      }
      about+="\n\n";
      Str s=Cpu.name(); if(s.is())s+=", "; s+=S+Cpu.threads()+" HW Threads"; if(s.is())about.line()+=S+"CPU: "+s;
          s=D.deviceName(); if(s.is()){if(D.deviceMemory()>0)s+=S+", "+FileSize(D.deviceMemory(), '.')+" RAM"; about.line()+=S+"GPU: "+s;}
      MemStats m; m.get(); if(m.total_phys>0)about.line()+=S+"RAM: "+FileSize(m.total_phys, '.');
          s=OSName(); if(s.is())about.line()+=S+"Operating System: "+s;
          s=OSUserName(); if(s.is())about.line()+=S+"System User Name: "+s;
      #if DESKTOP
          s=S+App.desktopW()+'x'+App.desktopH(); if(s.is())about.line()+=S+"Desktop Resolution: "+s;
      #endif
          s=S+D.resW()+'x'+D.resH(); if(s.is())about.line()+=S+"Application Resolution: "+s;
      Dialog &dialog=Gui.getMsgBox("about");
      Gui+=dialog.create("System Information", about, ConstCast(Str("OK")));
      dialog.buttons[0].func(Hide, SCAST(GuiObj, dialog));
      dialog.activate();
   }
   static void Move(MiscRegion &mr) {mr.setPos(VecI2(Ms.pos().x>0, Ms.pos().y>0));}

   void setPos(C VecI2 &pos)
   {
      if(T.pos!=pos)
      {
         T.pos=pos;
         ScreenChanged();
      }
   }
   void create()
   {
      Node<MenuElm> menu_menu;
      {
         menu_menu.New().create("Help"           , Help      , T).display(MLTC(null, PL, u"Pomoc"        , DE, u"Hilfe"          , RU, u"Помощь"         , PO, u"Ajuda"             )); //.desc(MLT("Show Esenthel Engine help documentation", PL,"Wyświetl pomoc Esenthel Engine", DE,"Zeige Esenthel Engine Hilfe Dokumentation", RU,"Показать документацию Esenthel Engine", PO,"Mostrar a documentaçăo de ajuda do Esenthel Engine"));
         menu_menu.New().create("About"          , About     , T);
         menu_menu.New().create("Video Options"  , VidOpt    , T).kbsc(KbSc(KB_F12  )).display(MLTC(null, PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video"   )).desc(MLT("Change video options", PL,"Zmień opcje grafiki", DE,"Ändert die Grafik Optionen", RU,"Изменить видео настройки", PO,"Mudar as opçőes de video")).flag(MENU_HIDDEN);
         menu_menu.New().create("Fullscreen"     , Fullscreen, T).kbsc(KbSc(KB_F11  )).display(MLTC(null, PL, u"Pełny Ekran"  , DE, u"Vollbild"       , RU, u"Полноэкранный"  , PO, u"Ecră inteiro"      )).desc(MLT("Toggle fullscreen mode", PL,"Zmień pełny ekran", DE, "Wechseln zu Fullscreen Modus", RU,"Переключить полноэкранный режим", PO,"Accionar modo de ecră inteiro"));
         menu_menu.New().create("Screenshot"     , Screenshot, T).kbsc(KbSc(KB_PRINT)).display(MLTC(null, RU, u"Скриншот")).desc("Take a screenshot and save it on the Desktop"/*MLTC(u"Take a screenshot and save it to \"ScreenShot\" folder inside the Editor", PL, u"Pobierz ekran oraz zapisz go do katalogu \"ScreenShot\"", DE, u"Macht einen Screenshot und speichert ihn in \"ScreenShot\"", RU, u"Сделать скриншот и сохранить в \"ScreenShot\"", PO, u"Captar um screenshot e guardar em \"ScreenShot\"")*/);
         menu_menu.New().create("Calculator"     , CalcShow  , T).kbsc(KbSc(KB_EQUAL, KBSC_CTRL_CMD));
       //menu_menu.New().create("Verify Elements", VerifyElms, T).desc("Verify if project elemets are accessing elements that haven't been removed.");
       //menu_menu.New().create(STEAM ? "License Key / Steam Subscription" : "License Key", SetLicense, T).desc("Set Your License Key");
      #if !STEAM // Valve doesn't allow this
         menu_menu.New().create("Esenthel Store" , AppStore  , T).desc("Open Esenthel Store where you can buy and sell items");
      #endif
       //menu_menu.New().create("Opened Files"   , ViewOpened, T).kbsc(KbSc(KB_3, KBSC_ALT)).flag(MENU_TOGGLABLE);
         menu_menu++;
         menu_menu.New().create("Editor Options"              , EditorOptions  , T).kbsc(KbSc(KB_F9));
         menu_menu.New().create("Project Settings"            , ProjSettings   , T);
         menu_menu.New().create("Erase Removed Elements"      , EraseRemoved   , T);
         menu_menu.New().create("Merge Similar Materials"     , MergeSimMtrl   , T);
         menu_menu.New().create("Create Materials from Images", CreateMtrls    , T).desc("This option allows for automated creating Material Elements out of Image Elements and Image Files");
         menu_menu.New().create("Data Size Statistics"        , DataSizeStats  , T);
         menu_menu.New().create("Show Unused Materials"       , UnusedMaterials, T);
         menu_menu.New().create("Detect Similar Textures"     , DetectSimTex   , T);
         menu_menu.New().create("Material Texture Downsize"   , ShowTexDownsize, T).kbsc(KbSc(KB_D, KBSC_CTRL_CMD|KBSC_ALT));
         menu_menu++;
         menu_menu.New().create("Synchronize Codes (Experimental)", SyncCodes, T).desc("This will initiate synchronizing codes between client and server.\nFirst all codes will be received from server.\nThen any changes in codes will be merged on the client side.\nUpon completion of merging code changes all of them will be sent to the server.");
         menu_menu++;
         menu_menu.New().create("Project List"   , ProjList     , T).kbsc(KbSc(KB_ESC, KBSC_SHIFT              )).desc("Go back to the project list");
         menu_menu.New().create("Exit"           , Quit         , T).kbsc(KbSc(KB_F4 , KBSC_ALT                )).display(MLTC(null, PL, u"Wyjdź", DE, u"Beenden", RU, u"Выход", PO, u"Sair")).desc(MLT("Exit application", PL,"Wyjdź z programu", DE,"Beendet die Anwendung", RU,"Выйти и закрыть программу", PO,"Sair da aplicaçăo"));
         menu_menu.New().create("Toggle Project" , ToggleProj   , T).kbsc(KbSc(KB_1  , KBSC_ALT                )).flag(MENU_HIDDEN);
         menu_menu.New().create("Toggle Theater" , ToggleTheater, T).kbsc(KbSc(KB_4  , KBSC_ALT                )).flag(MENU_HIDDEN);
         menu_menu.New().create("Toggle Removed" , ToggleRemoved, T).kbsc(KbSc(KB_R  , KBSC_ALT                )).flag(MENU_HIDDEN);
         menu_menu.New().create("Find in Project", FindProj     , T).kbsc(KbSc(KB_F  , KBSC_CTRL_CMD|KBSC_SHIFT)).flag(MENU_HIDDEN);

         menu_menu.New().create("Mode Close", ModeClose, T).kbsc(KbSc(KB_F3 , KBSC_ALT|KBSC_REPEAT)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 0"    , Mode0    , T).kbsc(KbSc(KB_F1 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 1"    , Mode1    , T).kbsc(KbSc(KB_F2 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 2"    , Mode2    , T).kbsc(KbSc(KB_F3 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 3"    , Mode3    , T).kbsc(KbSc(KB_F4 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 4"    , Mode4    , T).kbsc(KbSc(KB_F5 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         menu_menu.New().create("Mode 5"    , Mode5    , T).kbsc(KbSc(KB_F6 , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
      }
      Node<MenuElm> build_menu;
      {
         build_menu.New().create("Play"         , Play , T).kbsc(KbSc(KB_F5)).flag(MENU_HIDDEN);
      #if WINDOWS
         build_menu.New().create("Debug"        , Debug, T).kbsc(KbSc(KB_F5 , KBSC_CTRL_CMD)).desc("Play active application from within Visual Studio allowing to debug when needed");
      #elif MAC
       //build_menu.New().create("Debug"        , Debug, T).kbsc(KbSc(KB_F5 , KBSC_CTRL_CMD)).desc("Play active application from within Xcode allowing to debug when needed");
      #elif LINUX
       //build_menu.New().create("Debug"        , Debug, T).kbsc(KbSc(KB_F5 , KBSC_CTRL_CMD)).desc("Play active application from within NetBeans allowing to debug when needed");
      #endif
       // TODO: 
       //build_menu.New().create("Run To Cursor", RunToCursor, T).kbsc(KbSc(KB_F10, KBSC_CTRL_CMD));
         build_menu.New().create("Build"        , Build   , T).kbsc(KbSc(KB_F7)).desc("Build application but don't start it");
         build_menu.New().create("Rebuild"      , Rebuild , T).kbsc(KbSc(KB_F7, KBSC_CTRL_CMD)).desc("Delete all temporary files that were created during building process and build again");
         build_menu.New().create("Clean"        , Clean   , T).kbsc(KbSc(KB_F7, KBSC_CTRL_CMD|KBSC_ALT)).desc("Delete all temporary files that were created during building process of this project");
         build_menu.New().create("Clean All"    , CleanAll, T).desc("Delete all temporary files that were created during building process of all projects");
         build_menu.New().create("Stop"         , Stop    , T).kbsc(KbSc(KB_PAUSE)).desc("Stop any build in progress");
      #if WINDOWS
         build_menu.New().create("Open in Visual Studio", OpenIDE, T).kbsc(KbSc(KB_F8)).desc("Export the project to C++ files, and open them in Visual Studio");
      #elif MAC
         build_menu.New().create("Open in Xcode"        , OpenIDE, T).kbsc(KbSc(KB_F8)).desc("Export the project to C++ files, and open them in Xcode");
      #elif LINUX
         build_menu.New().create("Open in NetBeans"     , OpenIDE, T).kbsc(KbSc(KB_F8)).desc("Export the project to C++ files, and open them in NetBeans");
      #endif
         build_menu.New().create(  "Publish"                      , Publish      , T).kbsc(KbSc(KB_F8, KBSC_CTRL_CMD           )).desc("Use this option when you want to publish your application publicly.\nThis will create your application packaged with all needed data.\nApplication will always be compiled in release mode, even if debug is currently selected.");
         build_menu.New().create(S+"Save as *."+EsenthelProjectExt, PublishEsProj, T).kbsc(KbSc(KB_F8, KBSC_CTRL_CMD|KBSC_SHIFT)).desc(S+"Export your project into a single *."+EsenthelProjectExt+" file which can be easily imported by other users.");
         {
            Node<MenuElm> &Export=(build_menu+="Export");
            Export.New().create("Text"                                            , ExportTXT          , T);
            Export.New().create("C++"                                             , ExportCPP          , T);
          /*Export.New().create("C++ with Visual Studio 2008 project"             , ExportVS2008       , T);
            Export.New().create("C++ with Visual Studio 2010 project"             , ExportVS2010       , T);
            Export.New().create("C++ with Visual Studio 2012 project"             , ExportVS2012       , T);
            Export.New().create("C++ with Visual Studio 2013 project"             , ExportVS2013       , T);*/
            Export.New().create("C++ with Visual Studio 2015 project"             , ExportVS2015       , T);
            Export.New().create("C++ with Visual Studio 2017 project"             , ExportVS2017       , T);
            Export.New().create("C++ with Android project"                        , ExportAndroid      , T);
            Export.New().create("C++ with Android project and Project data"       , ExportAndroidData  , T);
            Export.New().create("C++ with Xcode project"                          , ExportXcode        , T);
            Export.New().create("C++ with Xcode project and Project data for iOS" , ExportXcodeDataiOS , T);
            Export.New().create("C++ with Linux Make project"                     , ExportLinuxMake    , T);
            Export.New().create("C++ with Linux NetBeans project"                 , ExportLinuxNetBeans, T);
            Export.New().create("Project data"                                    , ExportData         , T);
            Export.New().create("Project data optimized for Android"              , ExportDataAndroid  , T);
            Export.New().create("Project data optimized for iOS"                  , ExportDataiOS      , T);
            {
               Node<MenuElm> &code=(Export+="Code Synchronization"); code.desc("Import/Export Project Source Code which can be synchronized using 3rd party tools.");
               code.New().create("Explore", CodeExplore, T).desc("Open Project Source Code Synchronization folder.");
               code.New().create("Import" , CodeImport , T).desc("Import Project Source Code from Code Synchronization folder to Esenthel Project.\n\nYou can use this option to import source code from folder that was synchronized using 3rd party tools.");
               code.New().create("Export" , CodeExport , T).desc("Export Project Source Code from Esenthel Project to Code Synchronization Folder.\n\nYou can use this option to export source code and then synchronize it using 3rd party tools.");
            }
         }
         build_menu++;
         build_menu.New().create("View Output"            , CodeOutput, T).kbsc(KbSc(KB_2, KBSC_ALT)).flag(MENU_TOGGLABLE);
         build_menu.New().create("View Android Device Log", CodeDevLog, T).kbsc(KbSc(KB_3, KBSC_ALT)).flag(MENU_TOGGLABLE);
         build_menu++;
         build_menu.New().create("Debug"  , ConfigDebug  , T).flag(MENU_TOGGLABLE);
         build_menu.New().create("Release", ConfigRelease, T).flag(MENU_TOGGLABLE);
         build_menu++;
      #if WINDOWS
         build_menu.New().create("64-bit", Config64, T).flag(MENU_TOGGLABLE);
         build_menu.New().create("32-bit", Config32, T).flag(MENU_TOGGLABLE);
         build_menu++;
         build_menu.New().create("DirectX 11", ConfigDX11, T).flag(MENU_TOGGLABLE);
         build_menu.New().create("DirectX 9" , ConfigDX9 , T).flag(MENU_TOGGLABLE);
         build_menu++;
      #endif
         build_menu.New().create("Windows EXE"      , ConfigEXE  , T).flag(MENU_TOGGLABLE);
         build_menu.New().create("Windows DLL"      , ConfigDLL  , T).flag(MENU_TOGGLABLE);
       //build_menu.New().create("Windows LIB"      , ConfigLIB  , T).flag(MENU_TOGGLABLE);
         build_menu.New().create("Windows Universal", ConfigNEW  , T).flag(MENU_TOGGLABLE);
         build_menu.New().create("Android APK"      , ConfigAPK  , T).flag(MENU_TOGGLABLE);
      #if WINDOWS
         build_menu.New().create("Web"        , ConfigWeb  , T).flag(MENU_TOGGLABLE);
      #elif MAC
         build_menu.New().create("Mac APP"    , ConfigMAC  , T).flag(MENU_TOGGLABLE);
         build_menu.New().create("iOS APP"    , ConfigIOS  , T).flag(MENU_TOGGLABLE);
      #elif LINUX
         build_menu.New().create("Linux"      , ConfigLinux, T).flag(MENU_TOGGLABLE);
      #endif
         /*if(ScriptsSupported())
         {
            build_menu++;
            build_menu.New().create("Play using Esenthel Compiler", PlayEsenthelCompiler).kbsc(KbSc(KB_P, KBSC_CTRL_CMD));
         }
      #if DEBUG
         build_menu++;
         build_menu.New().create("CreateFuncList", CreateFuncList);
      #endif*/
      }

      online_ts.reset().size=0.036;
      flt w=0.55, h=0.06;
      Gui+=super    .create(Rect_LU(0, 0,                      w,    h)).skin(&DarkSkin, false).hide().disabled(true); kb_lit=false;
      T  +=hide_proj.create(Rect_LU(0, 0,                  0.090, 0.06), "<<").func(HideProj, T).focusable(false).desc("Hide Project\nKeyboard Shortcut: Alt+1"); hide_proj.mode=BUTTON_TOGGLE;
      T  +=menu     .create(Rect_LU(hide_proj.rect().ru(), 0.060, 0.06), menu_menu).skin(&NoComboBoxImage).focusable(false).desc("Menu"); menu.text="M"; menu.text_align=0; menu.flag|=COMBOBOX_CONST_TEXT;
      T  +=vid_opt  .create(Rect_LU(menu     .rect().ru(), 0.060, 0.06)).func(VidOpt, T).focusable(false).desc(S+MLTC(u"Video Options", PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video")+"\nKeyboard Shortcut: F12"); vid_opt.image="Gui/Misc/display.img"; vid_opt.mode=BUTTON_TOGGLE;
      T  +=online   .create(Rect_LU(vid_opt  .rect().ru(), 0.155, 0.06), "Offline", &online_ts);
      T  +=play     .create(Rect_LU(online   .rect().ru(), 0.060, 0.06)).func(Play, T).focusable(false).desc("Play active application\nKeyboard Shortcut: F5"); play.image="Gui/arrow_right_big.img";
      T  +=build    .create(Rect_LU(play     .rect().ru(), 0.125, 0.06)).setData(build_menu).focusable(false); build.text="Build"; build.text_size*=0.64; FlagDisable(build.flag, COMBOBOX_MOUSE_WHEEL); build.flag|=COMBOBOX_CONST_TEXT;
      Gui+=move_misc.create(Rect_LU(0, 0, h, h)).func(Move, T).hide().focusable(false).desc("Move bar around the screen"); move_misc.image="Gui/Misc/move.img"; move_misc.mode=BUTTON_CONTINUOUS;
      CodeEdit.configChangedDebug();
      CodeEdit.configChanged32Bit();
      CodeEdit.configChangedDX9  ();
      CodeEdit.configChangedEXE  ();
      CodeEdit.visibleChangedOptions      ();
      CodeEdit.visibleChangedOpenedFiles  ();
      CodeEdit.visibleChangedOutput       ();
      CodeEdit.visibleChangedAndroidDevLog();
   }
   void resize()
   {
      Rect screen(-D.w(), -D.h(), D.w(), D.h());
      Vec2 cur=rect().lerp(pos.x, pos.y),
        target(screen.lerp(pos.x, pos.y));
      move(target-cur);
         cur=move_misc.rect().lerp(  pos.x, 0);
      target=          rect().lerp(1-pos.x, 0);
      move_misc.move(target-cur);
   }
   void updateMove()
   {
      if(Mode()!=MODE_GUI)setPos(VecI2(0, MiscOnTop));
      move_misc.visible(visible() && Mode()==MODE_GUI);
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(screenshot && !--screenshot)Renderer.screenShots(SystemPath(SP_DESKTOP).tailSlash(true)+"Esenthel Editor ScreenShots/", "bmp");
   }
   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         if(Proj.visible() && Proj.rect().max.x>rect().max.x+EPS)
            if(Mode()==MODE_OBJ || Mode()==MODE_ANIM || Mode()==MODE_WORLD || Mode()==MODE_TEX_DOWN)
         {
            D.clip(gpc.clip);
            Rect    r=rect(); r.setX(r.max.x, Proj.rect().max.x);
            Color col=BackgroundColor(); //if(GuiSkin *skin=getSkin()){col=skin.region.normal_color; col.a=255;}
            r.draw(col);
         }
         super.draw(gpc);
      }
   }
   virtual MiscRegion& show()override {super.show(); updateMove(); return T;}
   virtual MiscRegion& hide()override {super.hide(); updateMove(); return T;}
}
MiscRegion Misc;
/******************************************************************************/
