/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
MiscRegion Misc;
/******************************************************************************/

/******************************************************************************/
   void MiscRegion::Play(MiscRegion &mr) {CodeEdit.play       ();}
   void MiscRegion::Debug(MiscRegion &mr) {CodeEdit.debug      ();}
   void MiscRegion::RunToCursor(MiscRegion &mr) {CodeEdit.runToCursor();}
   void MiscRegion::Build(MiscRegion &mr) {CodeEdit.build      ();}
   void MiscRegion::Rebuild(MiscRegion &mr) {CodeEdit.rebuild    ();}
   void MiscRegion::Clean(MiscRegion &mr) {CodeEdit.clean      ();}
   void MiscRegion::CleanAll(MiscRegion &mr) {CodeEdit.cleanAll   ();}
   void MiscRegion::Stop(MiscRegion &mr) {CodeEdit.stop       ();}
   void MiscRegion::OpenIDE(MiscRegion &mr) {CodeEdit.openIDE    ();}
   void MiscRegion::ExportTXT(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_TXT    );}
   void MiscRegion::ExportCPP(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_CPP    );}
   void MiscRegion::ExportVS2015(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_VS2015 );}
   void MiscRegion::ExportVS2017(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_VS2017 );}
   void MiscRegion::ExportAndroid(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_ANDROID);}
   void MiscRegion::ExportAndroidData(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_ANDROID, true);}
   void MiscRegion::ExportXcode(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_XCODE  );}
   void MiscRegion::ExportXcodeDataiOS(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_XCODE  , true);}
   void MiscRegion::ExportLinuxMake(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_LINUX_MAKE    );}
   void MiscRegion::ExportLinuxNetBeans(MiscRegion &mr) {CodeEdit.Export(Edit::EXPORT_LINUX_NETBEANS);}
   void MiscRegion::ExportData(MiscRegion &mr) {::Publish.exportData();}
   void MiscRegion::ExportDataAndroid(MiscRegion &mr) {::Publish.exportData(Edit::EXE_APK);}
   void MiscRegion::ExportDataiOS(MiscRegion &mr) {::Publish.exportData(Edit::EXE_IOS);}
   void MiscRegion::CodeExplore(MiscRegion &mr) {Proj.codeExplore();}
   void MiscRegion::CodeImport(MiscRegion &mr) {Proj.codeImport (true);}
   void MiscRegion::CodeExport(MiscRegion &mr) {Proj.codeExport (true);}
   void MiscRegion::ConfigDebug(MiscRegion &mr) {CodeEdit.configDebug(true );}
   void MiscRegion::ConfigRelease(MiscRegion &mr) {CodeEdit.configDebug(false);}
   void MiscRegion::Config32(MiscRegion &mr) {CodeEdit.config32Bit(true );}
   void MiscRegion::Config64(MiscRegion &mr) {CodeEdit.config32Bit(false);}
   void MiscRegion::ConfigDX9(MiscRegion &mr) {CodeEdit.configDX9  (true );}
   void MiscRegion::ConfigDX11(MiscRegion &mr) {CodeEdit.configDX9  (false);}
   void MiscRegion::ConfigEXE(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_EXE  );}
   void MiscRegion::ConfigDLL(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_DLL  );}
   void MiscRegion::ConfigLIB(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_LIB  );}
   void MiscRegion::ConfigNEW(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_NEW  );}
   void MiscRegion::ConfigAPK(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_APK  );}
   void MiscRegion::ConfigMAC(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_MAC  );}
   void MiscRegion::ConfigIOS(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_IOS  );}
   void MiscRegion::ConfigLinux(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_LINUX);}
   void MiscRegion::ConfigWeb(MiscRegion &mr) {CodeEdit.configEXE  (Edit::EXE_WEB  );}
   void MiscRegion::ProjList(bool all_saved, ptr) {if(all_saved)StateProjectList.set(StateFadeTime);}
   void MiscRegion::ProjList(MiscRegion &mr) {SaveChanges(ProjList);}
   void   MiscRegion::HideProj(MiscRegion &mr) {Proj.visible(!mr.hide_proj());}
   void MiscRegion::ToggleProj(MiscRegion &mr) {mr.hide_proj.push();}
   void MiscRegion::ToggleTheater(MiscRegion &mr) {Proj.theater.push();}
   void MiscRegion::ToggleRemoved(MiscRegion &mr) {Proj.show_removed.push();}
   void   MiscRegion::FindProj(MiscRegion &mr) {if(mr.hide_proj())mr.hide_proj.push(); Proj.filter.selectAll().activate();}
   void   MiscRegion::ModeClose(MiscRegion &mr) {Mode.closeActive(false);}
   void   MiscRegion::Mode0(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(0))tab->push();}
   void   MiscRegion::Mode1(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(1))tab->push();}
   void   MiscRegion::Mode2(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(2))tab->push();}
   void   MiscRegion::Mode3(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(3))tab->push();}
   void   MiscRegion::Mode4(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(4))tab->push();}
   void   MiscRegion::Mode5(MiscRegion &mr) {if(Tab *tab=Mode.visibleTab(5))tab->push();}
   void MiscRegion::VidOpt(MiscRegion &mr) {::VidOpt.visibleToggleActivate();}
   void MiscRegion::BuyLicense(ptr           ) {Explore("http://www.esenthel.com/?id=store&cat=0");}
   void MiscRegion::Help(MiscRegion &mr) {Explore("http://www.esenthel.com/forum/");}
   void MiscRegion::Fullscreen(MiscRegion &mr) {D.toggle();}
   void MiscRegion::Screenshot(MiscRegion &mr) {mr.screenshot=5;}
   void MiscRegion::CalcShow(MiscRegion &mr) {Calculator.visibleToggleActivate();}
   void MiscRegion::VerifyElms(MiscRegion &mr) {}
   void MiscRegion::ProjSettings(MiscRegion &mr) {::ProjSettings.display();}
   void MiscRegion::Publish(MiscRegion &mr) {PublishDo();}
   void MiscRegion::PublishEsProj(MiscRegion &mr) {if(!PublishEsProjIO.is())PublishEsProjIO.create(EsenthelProjectExt, S, SystemPath(SP_DESKTOP), null, ::PublishEsProjAs); PublishEsProjIO.save().name(CleanFileName(Proj.name));}
   void MiscRegion::AppStore(MiscRegion &mr) {::AppStore.activate();}
   void MiscRegion::EraseRemoved(MiscRegion &mr) {::EraseRemoved.activate();}
   void MiscRegion::MergeSimMtrl(MiscRegion &mr) {MSM         .activate();}
   void MiscRegion::DetectSimTex(MiscRegion &mr) {DST         .activate();}
   void MiscRegion::CreateMtrls(MiscRegion &mr) {::CreateMtrls.activate();}
   void MiscRegion::DataSizeStats(MiscRegion &mr) {SizeStats   .display(S);}
   void MiscRegion::UnusedMaterials(MiscRegion &mr) {SizeStats   .displayUnusedMaterials();}
   void MiscRegion::ShowTexDownsize(MiscRegion &mr) {TexDownsize .toggle();}
   void MiscRegion::SyncCodes(MiscRegion &mr) {Proj.syncCodes();}
   void MiscRegion::EditorOptions(MiscRegion &mr) {CodeEdit.visibleOptions      (!CodeEdit.visibleOptions      ());}
   void MiscRegion::CodeOutput(MiscRegion &mr) {CodeEdit.visibleOutput       (!CodeEdit.visibleOutput       ());}
   void MiscRegion::CodeDevLog(MiscRegion &mr) {CodeEdit.visibleAndroidDevLog(!CodeEdit.visibleAndroidDevLog());}
   void MiscRegion::Quit(MiscRegion &mr) {App.close();}
   void MiscRegion::About(MiscRegion &mr)
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
   void MiscRegion::Move(MiscRegion &mr) {mr.setPos(VecI2(Ms.pos().x>0, Ms.pos().y>0));}
   void MiscRegion::setPos(C VecI2 &pos)
   {
      if(T.pos!=pos)
      {
         T.pos=pos;
         ScreenChanged();
      }
   }
   void MiscRegion::create()
   {
      Node<MenuElm> menu_menu;
      {
         menu_menu.New().create("Help"           , Help      , T).display(MLTC(null, PL, u"Pomoc"        , DE, u"Hilfe"          , RU, u"Помощь"         , PO, u"Ajuda"             )); //.desc(MLT("Show Esenthel Engine help documentation", PL,"Wyświetl pomoc Esenthel Engine", DE,"Zeige Esenthel Engine Hilfe Dokumentation", RU,"Показать документацию Esenthel Engine", PO,"Mostrar a documentaçăo de ajuda do Esenthel Engine"));
         menu_menu.New().create("About"          , About     , T);
         menu_menu.New().create("Video Options"  , VidOpt    , T).kbsc(KbSc(KB_F12  )).display(MLTC(null, PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video"   )).desc(MLT("Change video options", PL,u"Zmień opcje grafiki", DE,u"Ändert die Grafik Optionen", RU,u"Изменить видео настройки", PO,u"Mudar as opçőes de video")).flag(MENU_HIDDEN);
         menu_menu.New().create("Fullscreen"     , Fullscreen, T).kbsc(KbSc(KB_F11  )).display(MLTC(null, PL, u"Pełny Ekran"  , DE, u"Vollbild"       , RU, u"Полноэкранный"  , PO, u"Ecră inteiro"      )).desc(MLT("Toggle fullscreen mode", PL,u"Zmień pełny ekran", DE, "Wechseln zu Fullscreen Modus", RU,u"Переключить полноэкранный режим", PO,u"Accionar modo de ecră inteiro"));
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
         menu_menu.New().create("Exit"           , Quit         , T).kbsc(KbSc(KB_F4 , KBSC_ALT                )).display(MLTC(null, PL, u"Wyjdź", DE, u"Beenden", RU, u"Выход", PO, u"Sair")).desc(MLT("Exit application", PL,u"Wyjdź z programu", DE,"Beendet die Anwendung", RU,u"Выйти и закрыть программу", PO,u"Sair da aplicaçăo"));
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

      online_ts.reset().size=0.036f;
      flt w=0.55f, h=0.06f;
      Gui+=::EE::Region::create(Rect_LU(0, 0,                      w,    h)).skin(&DarkSkin, false).hide().disabled(true); kb_lit=false;
      T  +=hide_proj.create(Rect_LU(0, 0,                  0.090f, 0.06f), "<<").func(HideProj, T).focusable(false).desc("Hide Project\nKeyboard Shortcut: Alt+1"); hide_proj.mode=BUTTON_TOGGLE;
      T  +=menu     .create(Rect_LU(hide_proj.rect().ru(), 0.060f, 0.06f), menu_menu).skin(&NoComboBoxImage).focusable(false).desc("Menu"); menu.text="M"; menu.text_align=0; menu.flag|=COMBOBOX_CONST_TEXT;
      T  +=vid_opt  .create(Rect_LU(menu     .rect().ru(), 0.060f, 0.06f)).func(VidOpt, T).focusable(false).desc(S+MLTC(u"Video Options", PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video")+"\nKeyboard Shortcut: F12"); vid_opt.image="Gui/Misc/display.img"; vid_opt.mode=BUTTON_TOGGLE;
      T  +=online   .create(Rect_LU(vid_opt  .rect().ru(), 0.155f, 0.06f), "Offline", &online_ts);
      T  +=play     .create(Rect_LU(online   .rect().ru(), 0.060f, 0.06f)).func(Play, T).focusable(false).desc("Play active application\nKeyboard Shortcut: F5"); play.image="Gui/arrow_right_big.img";
      T  +=build    .create(Rect_LU(play     .rect().ru(), 0.125f, 0.06f)).setData(build_menu).focusable(false); build.text="Build"; build.text_size*=0.64f; FlagDisable(build.flag, COMBOBOX_MOUSE_WHEEL); build.flag|=COMBOBOX_CONST_TEXT;
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
   void MiscRegion::resize()
   {
      Rect screen(-D.w(), -D.h(), D.w(), D.h());
      Vec2 cur=rect().lerp(pos.x, pos.y),
        target(screen.lerp(pos.x, pos.y));
      move(target-cur);
         cur=move_misc.rect().lerp(  pos.x, 0);
      target=          rect().lerp(1-pos.x, 0);
      move_misc.move(target-cur);
   }
   void MiscRegion::updateMove()
   {
      if(Mode()!=MODE_GUI)setPos(VecI2(0, MiscOnTop));
      move_misc.visible(visible() && Mode()==MODE_GUI);
   }
   void MiscRegion::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(screenshot && !--screenshot)Renderer.screenShots(SystemPath(SP_DESKTOP).tailSlash(true)+"Esenthel Editor ScreenShots/", "bmp");
   }
   void MiscRegion::draw(C GuiPC &gpc)
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
         ::EE::Region::draw(gpc);
      }
   }
   MiscRegion& MiscRegion::show(){::EE::GuiObj::show(); updateMove(); return T;}
   MiscRegion& MiscRegion::hide(){::EE::GuiObj::hide(); updateMove(); return T;}
MiscRegion::MiscRegion() : screenshot(0), pos(0, MiscOnTop) {}

/******************************************************************************/
