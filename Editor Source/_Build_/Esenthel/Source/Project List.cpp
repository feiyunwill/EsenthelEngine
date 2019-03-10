/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
Projects Projs;
Str      ProjectsPath; // must include tailSlash
State    StateProjectList(UpdateProjectList, DrawProjectList, InitProjectList, ShutProjectList);
/******************************************************************************/
NewProjWin NewProj;
/******************************************************************************/
RenameProjWin RenameProj;
/******************************************************************************/
RemoveProjWin RemoveProj;
/******************************************************************************/
RegisterWin Register;
/******************************************************************************/
ChangePassWin ChangePass;
/******************************************************************************/
bool InitProjectList()
{
   SetKbExclusive();
   Proj.close();
   Projs.show();
   Projs.refresh();
   NewProj.create();
   RenameProj.create();
   RemoveProj.create();
   Register.create();
   ChangePass.create();
   if(CmdLine.is())
   {
      if(GetExt(CmdLine)==EsenthelProjectExt)CopyElms.display(CmdLine);
      CmdLine.clear();
   }
   return true;
}
void ShutProjectList()
{
   Projs.hide();
   NewProj.del();
   RenameProj.del();
   RemoveProj.del();
   Register.del();
   ChangePass.del();
   ProjectLock.del();
}
/******************************************************************************/
bool UpdateProjectList()
{
   Gui   .update();
   Server.update(null, false);
   Projs .update();
   if(Ms.bp(3))WindowToggle();
   return true;
}
void DrawProjectList()
{
   D.clear(BackgroundColorLight());
   Gui.draw();
}
/******************************************************************************/

/******************************************************************************/
   ::Projects::Elm* Projects::findProj(C UID &id) {if(id.valid())REPA(proj_data)if(proj_data[i].id==id)return &proj_data[i]; return null;}
   void Projects::SelectPath(             Projects &projs) {projs.proj_path_io.path(S, projs.proj_path()).activate();}
   void Projects::SelectPath(C Str &name, Projects &projs) {projs.proj_path.set(name);}
   void Projects::PathExplore(             Projects &projs) {Explore(ProjectsPath);}
   void Projects::PathChanged(             Projects &projs) {ProjectsPath=Str(projs.proj_path()).tailSlash(true); CodeEdit.projectsBuildPath(ProjectsPath+ProjectsBuildPath); projs.refresh();}
   void Projects::Connect(             Projects &projs) {projs.connectDo();}
   void Projects::Disconnect(             Projects &projs) {Server.logout();}
   void Projects::ForgotPass(             Projects &projs) {projs.forgotPass();}
   void Projects::ChangePass(             Projects &projs) {::ChangePass.activate(false);}
   void Projects::NewProj(             Projects &projs) {::NewProj.activate();}
   void Projects::ImportProj(Projects &projs) {if(!projs.import_proj_io.is())projs.import_proj_io.create(EsenthelProjectExt, S, S, ImportProj); projs.import_proj_io.activate();}
   void Projects::ImportProj(C Str &name, ptr user) {CopyElms.display(name);}
   void Projects::ImportProj_1_0(Projects &projs) {if(!projs.import_proj_1_0_io.is())projs.import_proj_1_0_io.create(S, S, S, ImportProj_1_0).modeDirSelect(); projs.import_proj_1_0_io.activate(); Gui.msgBox(S, "Please specify path to project data created with Esenthel Engine 1.0");}
   void Projects::ImportProj_1_0(C Str &name, ptr user)
   {
      if(!ProjectsPath.is())Gui.msgBox(S, "Path for Projects is invalid");else
      if(FileInfoSystem(name).type==FSTD_DIR){ImportSrc=name; StateImport.set(StateFadeTime);}
   }
   void Projects::HostEditorServer(Projects &projs)
   {
      if(projs.editor_network_interface()) // want to host
      {
         if(Demo)Gui.msgBox(S, "Editor Network Interface is not available in the demo version."); // can't host
         else EditServer.create();
      }else
      {
         EditServer.del();
      }
   }
   void Projects::Open() {if(Elm *proj=Projs.findProj(Projs.proj_menu_id))Projs     .open    (*proj);}
   void Projects::Rename() {if(Elm *proj=Projs.findProj(Projs.proj_menu_id))RenameProj.activate(*proj);}
   void Projects::Remove() {if(Elm *proj=Projs.findProj(Projs.proj_menu_id))RemoveProj.activate(*proj);}
   void Projects::ExploreProj() {if(Elm *proj=Projs.findProj(Projs.proj_menu_id))Explore            ( proj->path, true);}
   void Projects::CompareProj()
   {
      Memc<UID> ids; FREPA(Projs.proj_list.sel)if(Elm *proj=Projs.proj_list.absToData(Projs.proj_list.sel[i]))if(proj->local_name.is())ids.add(proj->id);
      CompareProjs.compare(ids);
   }
   void Projects::ToggleSynchronize(Elm &elm) {Projs.toggleSynchronizeProj(elm.id, elm.name);}
   void Projects::create()
   {
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Elm,  local_name), LCW_DATA, "Name"          ), // 0
         ListColumn(MEMBER(Elm, server_name), LCW_DATA, "Name on Server"), // 1
         ListColumn(DATA_NONE, 0, 0,          LCW_DATA, "Synchronize"   ), // 2
      };
      lc[2].sort=&(proj_list_sync_sort=MEMBER(Elm, sync_val));
      lc[2].desc("If automatically synchronize a project with the Server after opening it.");
      Node<MenuElm> menu_menu;
      {
         menu_menu.New().create("Help"          , MiscRegion::Help      , Misc).display(MLTC(null, PL, u"Pomoc", DE, u"Hilfe", RU, u"Помощь", PO, u"Ajuda")); //.desc(MLT("Show Esenthel Engine help documentation", PL,"Wyświetl pomoc Esenthel Engine", DE,"Zeige Esenthel Engine Hilfe Dokumentation", RU,"Показать документацию Esenthel Engine", PO,"Mostrar a documentaçăo de ajuda do Esenthel Engine"));
         menu_menu.New().create("About"         , MiscRegion::About     , Misc);
         menu_menu.New().create("Video Options" , MiscRegion::VidOpt    , Misc).kbsc(KbSc(KB_F12  )).display(MLTC(null, PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video")).desc(MLTC(u"Change video options", PL, u"Zmień opcje grafiki", DE, u"Ändert die Grafik Optionen", RU, u"Изменить видео настройки", PO, u"Mudar as opçőes de video")).flag(MENU_HIDDEN);
         menu_menu.New().create("Fullscreen"    , MiscRegion::Fullscreen, Misc).kbsc(KbSc(KB_F11  )).display(MLTC(null, PL, u"Pełny Ekran"  , DE, u"Vollbild"       , RU, u"Полноэкранный"  , PO, u"Ecră inteiro"   )).desc(MLTC(u"Toggle fullscreen mode", PL, u"Zmień pełny ekran", DE, u"Wechseln zu Fullscreen Modus", RU, u"Переключить полноэкранный режим", PO, u"Accionar modo de ecră inteiro"));
         menu_menu.New().create("Screenshot"    , MiscRegion::Screenshot, Misc).kbsc(KbSc(KB_PRINT)).display(MLTC(null, RU, u"Скриншот")).desc("Take a screenshot and save it on the Desktop"/*MLTC(u"Take a screenshot and save it to \"ScreenShot\" folder inside the Editor", PL, u"Pobierz ekran oraz zapisz go do katalogu \"ScreenShot\"", DE, u"Macht einen Screenshot und speichert ihn in \"ScreenShot\"", RU, u"Сделать скриншот и сохранить в \"ScreenShot\"", PO, u"Captar um screenshot e guardar em \"ScreenShot\"")*/);
         menu_menu.New().create("Calculator"    , MiscRegion::CalcShow  , Misc).kbsc(KbSc(KB_EQUAL, KBSC_CTRL_CMD));
       //menu_menu.New().create(STEAM ? "License Key / Steam Subscription" : "License Key", MiscRegion.SetLicense, Misc).desc("Set Your License Key");
      #if !STEAM // Valve doesn't allow this
         menu_menu.New().create("Esenthel Store", MiscRegion::AppStore  , Misc).desc("Open Esenthel Store where you can buy and sell items");
      #endif
         menu_menu++;
         menu_menu.New().create("Exit"          , MiscRegion::Quit      , Misc).kbsc(KbSc(KB_F4, KBSC_ALT)).display(MLTC(null, PL, u"Wyjdź", DE, u"Beenden", RU, u"Выход", PO, u"Sair")).desc(MLTC(u"Exit application", PL, u"Wyjdź z programu", DE, u"Beendet die Anwendung", RU, u"Выйти и закрыть программу", PO, u"Sair da aplicaçăo"));
      }

      ts_left.reset().align.set(1, 0);
      ts_left_down=ts_left; ts_left_down.align.set(1, 1);
      ts_small_left_up=ts_left; ts_small_left_up.size*=0.8f; ts_small_left_up.align.set(1, -1);
      t_proj_path      .create("Path for Projects", &ts_left_down);
      t_server_login   .create("Server Login", &ts_left_down);
      t_server         .create("Server Address", &ts_left);
      t_logged_in      .create("Logged in as", &ts_left);
      t_email          .create("E-mail", &ts_left);
      t_pass           .create("Password", &ts_left);
      t_path_desc      .create("This is the path where files of all your projects will be kept.", &ts_small_left_up);
      t_login_desc     .create("Log in to the server to create games in collaborative mode with other people (this is optional).", &ts_small_left_up).auto_line=AUTO_LINE_SPACE_SPLIT;
      t_projects       .create("Projects");
      t_editor_network_interface     .create("Editor Network Interface", &ts_left_down);
      t_editor_network_interface_desc.create("Allow programs to connect to this editor and access your projects (this is optional). This allows you to create and use addons to the editor. Only programs from this computer will be able to connect.", &ts_small_left_up).auto_line=AUTO_LINE_SPACE_SPLIT;
        editor_network_interface     .create("Allow Incoming Connections").func(HostEditorServer, T).mode=BUTTON_TOGGLE;
      proj_path_explore.create("Explore").func(PathExplore, T);
      proj_path        .create(ProjectsPath).func(PathChanged, T, true).maxLength(1024); // set immediate change in case we set this through Editor Network Interface, and we need the change to be performed immediately
      proj_path_sel    .create("...").func(SelectPath, T);
      server           .create().maxLength(256);
      email            .create().maxLength(256);
      pass             .create().maxLength(16).password(true);
      connect          .create(   "Connect").func(   Connect, T);
      disconnect       .create("Disconnect").func(Disconnect, T);
      forgot_pass      .create("Forgot Password").func(ForgotPass, T);
      change_pass      .create("Change Password").func(ChangePass, T);
      proj_region      .create()+=proj_list.create(lc, Elms(lc)).elmHeight(0.06f).setElmDesc(MEMBER(Elm, desc)); proj_list.sort_column[0]=0; proj_list.cur_mode=LCM_ALWAYS; proj_list.flag|=LIST_MULTI_SEL;
      new_proj         .create("New Project").func(NewProj, T);
      import_proj      .create(S+"Import *."+EsenthelProjectExt).func(ImportProj, T);
      import_proj_1_0  .create("Import 1.0 Project").func(ImportProj_1_0, T).desc("Import project data created with Esenthel Engine 1.0");
      menu             .create(menu_menu).skin(&NoComboBoxImage).focusable(false).desc("Menu"); menu.text="M"; menu.text_align=0; menu.flag|=COMBOBOX_CONST_TEXT;
      vid_opt          .create().func(MiscRegion::VidOpt, Misc).focusable(false).desc(S+MLTC(u"Video Options", PL, u"Opcje Grafiki", DE, u"Grafik Optionen", RU, u"Настройки видео", PO, u"Opçőes de Video")+"\nKeyboard Shortcut: F12"); vid_opt.image="Gui/Misc/display.img"; vid_opt.mode=BUTTON_TOGGLE;

      proj_path_io.create().io(SelectPath, SelectPath, T).modeDirSelect();
   }
   void Projects::resize()
   {
      flt y=D.h()-0.1f, h=0.06f;
      proj_path        .rect(Rect_LU(-D.w()+0.05f, y, D.w()*0.9f, h));
      proj_path_explore.rect(Rect_RD(proj_path.rect().ru(), 0.25f, h));
      proj_path_sel    .rect(Rect_LU(proj_path.rect().ru(), proj_path.rect().h()));
      t_proj_path      .pos (proj_path.rect().lu());
      t_path_desc      .pos (proj_path.rect().ld());
      
      flt x=D.w()*0.41f, s=h*1.2f, w=D.w()*0.42f;
      server        .rect(Rect_LU(x, y, w, h)); t_server.pos(server.rect().left()-Vec2(0.3f, 0)); y-=s;
      email         .rect(Rect_LU(x, y, w, h)); t_email .pos(email .rect().left()-Vec2(0.3f, 0)); y-=s;
      pass          .rect(Rect_LU(x, y, w, h)); t_pass  .pos(pass  .rect().left()-Vec2(0.3f, 0)); y-=s;
         connect    .rect(Rect_U (Lerp(t_pass.rect().min.x, pass.rect().max.x, 0.25f), y, 0.3f, h));
      disconnect    .rect(Rect_U (Lerp(t_pass.rect().min.x, pass.rect().max.x, 0.25f), y, 0.3f, h));
      change_pass   .rect(Rect_U (Lerp(t_pass.rect().min.x, pass.rect().max.x, 0.75f), y, 0.4f, h));
      forgot_pass   .rect(Rect_U (Lerp(t_pass.rect().min.x, pass.rect().max.x, 0.75f), y, 0.4f, h)); y-=s;
      t_logged_in   .pos (t_email.pos());
      t_server_login.pos (server.rect().lu());
      
      t_login_desc.rect(Rect(t_pass.rect().min.x, y, pass.rect().max.x, y));

        editor_network_interface     .rect(Rect_LU(t_path_desc.pos()-Vec2(0, h*2.5f), 0.65f, h));
      t_editor_network_interface     .pos (editor_network_interface.rect().lu());
      t_editor_network_interface_desc.rect(Rect_LU(editor_network_interface.rect().ld(), proj_path.rect().w(), 0));

      if(TextStyle *text_style=t_login_desc.getTextStyle())
         proj_region.rect(Rect(-D.w()+0.05f, -D.h()+0.06f, D.w()-0.05f, t_login_desc.rect().min.y-0.08f-text_style->lineHeight()*text_style->textLines(t_login_desc(), t_login_desc.rect().w(), t_login_desc.auto_line)));
      t_projects     .pos (proj_region.rect().up()+Vec2(0, 0.03f));
      new_proj       .rect(Rect_RD(proj_region.rect().ru(), 0.3f , h));
      import_proj    .rect(Rect_RU(new_proj   .rect().lu(), 0.55f, h));
      import_proj_1_0.rect(Rect_RU(import_proj.rect().lu(), 0.4f , h));

      menu   .rect(Rect_LD(-D.w()+0.05f, -D.h(), 0.06f, 0.06f));
      vid_opt.rect(Rect_LU(menu.rect().ru(), menu.size()));
   }
   void Projects::forgotPass()
   {
      SockAddr sa;
      if(!sa.fromText(server()))Gui.msgBox(S, "Invalid server address.");else
      if(!ValidEmail (email ()))Gui.msgBox(S, "Invalid e-mail address.");else
      {
         Server.forgotPass(sa, email());
      }
   }
   void Projects::connectDo()
   {
      SockAddr sa;
      if(!sa.fromText(server()))Gui.msgBox(S, "Invalid server address.");else
      if(!ValidEmail (email ()))Gui.msgBox(S, "Invalid e-mail address.");else
      if(!ValidPass  (pass  ()))Gui.msgBox(S, "Password length must be in range of 4-16 characters and may not contain unicode characters.");else
      {
         Server.connect(sa, email(), pass());
      }
   }
   void Projects::refresh()
   {
      if(proj_list.is())
      {
         UID sel_proj_id=UIDZero; if(Elm *elm=proj_list())sel_proj_id=elm->id;
         proj_data.clear();

         // add local projects
         Project temp; for(FileFind ff(ProjectsPath); ff(); )if(temp.isProject(ff))
         {
            Elm &proj=proj_data.New();
            proj.id  =temp.id;
            proj.name=proj.local_name=temp.name;
            proj.path=ff.pathName().tailSlash(true);
            proj.desc=S+"Project folder name \""+ff.name+'"';
            proj.sync_val=temp.synchronize;
            proj_list.addChild(proj.synchronize.create(Rect_C(proj_list.columnWidth(2)/2, proj_list.elmHeight()/-2, proj_list.elmHeight()*(0.95f/2)), temp.synchronize).func(ToggleSynchronize, proj).desc("If automatically synchronize this project with the Server after opening it."), proj_data.elms()-1, 2);
         }

         // add server projects
         FREPA(Server.projects)
         {
          C Project &server_proj=Server.projects[i];
            if(Elm *project=findProj(server_proj.id)) // if project is already listed locally
            {
               project->server_name=server_proj.name; // only add server name
            }else
            { // fill new data
               Elm &proj=proj_data.New();
               proj.id  =server_proj.id;
               proj.name=proj.server_name=server_proj.name;
               proj.path=ProjectsPath+EncodeFileName(proj.id).tailSlash(true);
               // don't set 'desc' about folder name because it's not stored yet on the disk locally
            }
         }

         proj_list.setData(proj_data);
         selectProj(sel_proj_id, false);
      }
   }
   void Projects::selectProj(C UID &proj_id, bool kb_focus)
   {
      if(Elm *proj=findProj(proj_id))
      {
         proj_list.setCur(proj_list.dataToVis(proj));
         proj_list.scrollTo(proj_list.cur, false, 0.5f);
      }
      if(kb_focus)proj_list.kbSet();
   }
   bool Projects::newProj(C Str &name)
   {
      Project proj; proj.id.randomizeValid(); proj.name=CleanFileName(name); proj.path=ProjectsPath+EncodeFileName(proj.id).tailSlash(true);
      if(!ProjectsPath.is(       ))Gui.msgBox(S, "Path for Projects is invalid");else
      if(!ValidFileName(proj.name))Gui.msgBox(S, "Project name can't be empty, can't start with dot and can't contain following symbols \\ / : * ? \" < > |");else
      if(!FCreateDirs  (proj.path))Gui.msgBox(S, "Can't write in \"Path for Projects\".\nTry choosing a different path."
      #if WINDOWS
         "\nOn Windows you can't write to \"Program Files\" folder."
      #endif
      );else
      if(!proj.save(proj.path+"Data"))Gui.msgBox(S, "Error saving project data");else
      {
         refresh();
         selectProj(proj.id);
         return true;
      }
      return false;
   }
   bool Projects::toggleSynchronizeProj(C UID &proj_id, C Str &name)
   {
      Project proj; Str error; switch(proj.open(proj_id, name, ProjectsPath+EncodeFileName(proj_id), error))
      {
         case LOAD_NEWER : Gui.msgBox(S,   "This project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again."); break;
         case LOAD_LOCKED: Gui.msgBox(S,   "This project appears to be already opened in another instance of the Editor.\nIf it isn't, then please try re-opening it manually first."); break;
         case LOAD_ERROR : Gui.msgBox(S, S+"This project failed to load."+(error.is() ? '\n' : '\0')+error); break;

         case LOAD_OK    :
         case LOAD_EMPTY : proj.synchronize^=1; proj.close(); refresh(); return true;
      }
      return false;
   }
   bool Projects::renameProj(C UID &proj_id, Str name)
   {
      name=CleanFileName(name);
      if(!ValidFileName(name))Gui.msgBox(S, "Project name can't be empty, can't start with dot and can't contain following symbols \\ / : * ? \" < > |");else
      {
         Project proj; Str error; switch(proj.open(proj_id, name, ProjectsPath+EncodeFileName(proj_id), error))
         {
            case LOAD_NEWER : Gui.msgBox(S,   "This project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again."); break;
            case LOAD_LOCKED: Gui.msgBox(S,   "This project appears to be already opened in another instance of the Editor.\nIf it isn't, then please try re-opening it manually first."); break;
            case LOAD_ERROR : Gui.msgBox(S, S+"This project failed to load."+(error.is() ? '\n' : '\0')+error); break;

            case LOAD_OK    :
            case LOAD_EMPTY : proj.name=name; proj.close(); refresh(); selectProj(proj_id); return true;
         }
      }
      return false;
   }
   bool Projects::removeProj(C UID &proj_id)
   {
      Str path=ProjectsPath+EncodeFileName(proj_id);
      if(!FRecycle(path))Gui.msgBox(S, "Error when removing the project");else
      {
         refresh(); return true;
      }
      return false;
   }
   bool Projects::open(Elm &proj, bool ignore_lock)
   {
      Str path=ProjectsPath+EncodeFileName(proj.id);
      if(!(FExistSystem(path) || FCreateDirs(path)))Gui.msgBox(S, S+"Can't write to \""+GetPath(path)+"\"");else
      {
         Str error; LOAD_RESULT result=Proj.open(proj.id, proj.name, path, error, ignore_lock);
         if(result==LOAD_NEWER )Gui.msgBox(S,   "This project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again.");else
         if(result==LOAD_ERROR )Gui.msgBox(S, S+"This project failed to load."+(error.is() ? '\n' : '\0')+error);else
         if(result==LOAD_LOCKED)ProjectLock.create(proj.id);else
         {
            SetProjectState();
            return true;
         }
      }
      return false;
   }
   void Projects::show()
   {
      Gui+=t_proj_path   ;
      Gui+=t_server_login;
      Gui+=t_server      ;
      Gui+=t_email       ;
      Gui+=t_pass        ;
      Gui+=t_path_desc   ;
      Gui+=t_login_desc  ;
      Gui+=t_logged_in   ;
      Gui+=t_projects    ;
      Gui+=t_editor_network_interface;
      Gui+=t_editor_network_interface_desc;
      Gui+=proj_path_explore;
      Gui+=proj_path     ;
      Gui+=proj_path_sel ;
      Gui+=server        ;
      Gui+=email         ;
      Gui+=pass          ;
      Gui+=connect       ;
      Gui+=disconnect    ;
      Gui+=forgot_pass   ;
      Gui+=change_pass   ;
      Gui+=editor_network_interface;
      Gui+=proj_region   ;
      Gui+=new_proj      ;
      Gui+=import_proj   ;
   #if 0
      Gui+=import_proj_1_0;
   #endif
      Gui+=menu          ;
      Gui+=vid_opt       ;
        proj_path_io.enabled(true);
      import_proj_io.enabled(true);
  import_proj_1_0_io.enabled(true);
      proj_list.kbSet();
   }
   void Projects::hide()
   {
      Gui-=t_proj_path   ;
      Gui-=t_server_login;
      Gui-=t_server      ;
      Gui-=t_email       ;
      Gui-=t_pass        ;
      Gui-=t_path_desc   ;
      Gui-=t_login_desc  ;
      Gui-=t_logged_in   ;
      Gui-=t_projects    ;
      Gui-=t_editor_network_interface;
      Gui-=t_editor_network_interface_desc;
      Gui-=proj_path_explore;
      Gui-=proj_path     ;
      Gui-=proj_path_sel ;
      Gui-=server        ;
      Gui-=email         ;
      Gui-=pass          ;
      Gui-=connect       ;
      Gui-=disconnect    ;
      Gui-=forgot_pass   ;
      Gui-=change_pass   ;
      Gui-=editor_network_interface;
      Gui-=proj_region   ;
      Gui-=new_proj      ;
      Gui-=import_proj   ;
      Gui-=import_proj_1_0;
      Gui-=menu          ;
      Gui-=vid_opt       ;
      Gui-=proj_menu     ;
        proj_path_io.hide().disabled(true);
      import_proj_io.hide().disabled(true);
  import_proj_1_0_io.hide().disabled(true);
   }
   void Projects::projMenu(C Vec2 &pos, bool touch)
   {
      Node<MenuElm> node;
      if(proj_list.sel.elms()==1)if(Elm *proj=proj_list.absToData(proj_list.sel[0]))
      {
         proj_menu_id=proj->id;
         node.New().create("Open", Open);
         if(proj->local_name.is())
         {
            node.New().create("Rename" , Rename);
            node.New().create("Remove" , Remove);
            node.New().create("Explore", ExploreProj);
         }
      }
      int locals=0; REPA(proj_list.sel)if(Elm *proj=proj_list.absToData(proj_list.sel[i]))locals+=proj->local_name.is();
      if( locals>1)node.New().create("Compare", CompareProj);
      if(node.children.elms())
      {
         Gui+=proj_menu.create().setSize(touch).setData(node).posRU(pos).activate();
      }
   }
   void Projects::update()
   {
      t_logged_in .visible ( Server.loggedIn());
      t_email     .visible (!Server.loggedIn());
      t_pass      .visible (!Server.loggedIn());
      t_login_desc.visible (!Server.loggedIn());
      server      .disabled( Server.loggedIn());
      email       .disabled( Server.loggedIn());
      pass        .visible (!Server.loggedIn());
      connect     .visible (!Server.loggedIn());
      disconnect  .visible ( Server.loggedIn());
      forgot_pass .visible (!Server.loggedIn());
      change_pass .visible ( Server.loggedIn());
      proj_list.columnVisible(1, Server.loggedIn()); // name on server
      proj_list.columnVisible(2, Server.loggedIn()); // synchronize

      if(Kb.k(KB_ENTER))
      {
         if(Gui.kb()==&server || Gui.kb()==&email || Gui.kb()==&pass)connect.push();else
         if(Gui.kb()==&proj_list){if(Elm *proj=proj_list())open(*proj);}
      }

      REPA(MT)if(MT.guiObj(i)==&proj_list)
      {
         if(MT.bd(i   ))if(Elm *proj=proj_list()){open(*proj); break;}
         if(MT.bp(i, 1)
         || MT.b (i   ) && MT.touch(i) && MT.life(i)>=LongPressTime && !MT.dragging(i) && !Gui.menu())
         {
            proj_list.cur=proj_list.screenToVisY(MT.pos(i).y);
            if(!proj_list.sel.has(proj_list.visToAbs(proj_list.cur)))proj_list.setCur(proj_list.cur);
            projMenu(MT.pos(i), MT.touch(i)!=null); break;
         }
      }
   }
   void NewProjWin::OK(NewProjWin &npw) {if(Projs.newProj(npw.name()))npw.button[2].push();}
   void NewProjWin::create()
   {
      Gui+= ::EE::Window::create(Rect_C(0, 0, 0.75f, 0.37f), "New Project").hide(); button[2].func(HideProjsAct, SCAST(GuiObj, T)).show();
      T  +=t_name.create(Vec2  (clientWidth()/2, -0.05f), "Project Name");
      T  +=  name.create(Rect_C(clientWidth()/2, -0.12f, 0.65f, 0.06f));
      T  +=    ok.create(Rect_C(clientWidth()/2, -0.23f, 0.3f, 0.06f), "OK").func(OK, T);
   }
   void NewProjWin::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); OK(T);}
   }
   void RenameProjWin::OK(RenameProjWin &rpw) {if(Projs.renameProj(rpw.proj_id, rpw.name()))rpw.button[2].push();}
   void RenameProjWin::activate(Projects::Elm &proj)
   {
      setTitle(S+"Rename Project \""+proj.local_name+"\"");
      ::EE::GuiObj::activate();
      proj_id=proj.id;
      name.set(proj.local_name).selectAll().activate();
   }
   void RenameProjWin::create()
   {
      Gui+= ::EE::Window::create(Rect_C(0, 0, 0.75f, 0.37f), "Rename Project").hide(); button[2].func(HideProjsAct, SCAST(GuiObj, T)).show();
      T  +=t_name.create(Vec2  (clientWidth()/2, -0.05f), "New Project Name");
      T  +=  name.create(Rect_C(clientWidth()/2, -0.12f, 0.65f, 0.06f));
      T  +=    ok.create(Rect_C(clientWidth()/2, -0.23f, 0.3f, 0.06f), "OK").func(OK, T);
   }
   void RenameProjWin::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); OK(T);}
   }
   void RemoveProjWin::OK(RemoveProjWin &rpw) {Projs.removeProj(rpw.proj_id); rpw.button[2].push();}
   void RemoveProjWin::activate(Projects::Elm &proj)
   {
      setTitle(S+"Delete Project \""+proj.local_name+"\"");
      ::EE::GuiObj::activate();
      proj_id=proj.id;
   }
   void RemoveProjWin::create()
   {
      Gui+= ::EE::Window::create(Rect_C(0, 0, 0.75f, 0.37f), "Delete Project").hide(); button[2].func(HideProjsAct, SCAST(GuiObj, T)).show();
      T  +=t_name.create(Rect_C(clientWidth()/2, -0.11f, 0.7f, 0.0f), "Are you sure you wish to delete selected project from your disk?"); t_name.auto_line=AUTO_LINE_SPACE_SPLIT;
      T  +=    ok.create(Rect_C(clientWidth()/2, -0.23f, 0.3f, 0.06f), "OK").func(OK, T);
   }
   void RegisterWin::OK(RegisterWin &rw) {Server.Register(rw.name()); rw.hide();}
   void RegisterWin::create()
   {
      Gui+= ::EE::Window::create(Rect_C(0, 0, 0.8f, 0.5f), "Register").hide(); button[2].show();
      T  +=t_name.create(Vec2  (clientWidth()/2, -0.12f), "E-mail was not found.\nWould you like to register?\n\nYour Name");
      T  +=  name.create(Rect_C(clientWidth()/2, -0.25f, 0.65f, 0.06f)).maxLength(64);
      T  +=    ok.create(Rect_C(clientWidth()/2, -0.36f, 0.3f, 0.06f), "OK").func(OK, T);
   }
   void RegisterWin::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){OK(T); Kb.eatKey();}
   }
   void ChangePassWin::ChangePass(ChangePassWin &cpw) {cpw.changePass();}
   void ChangePassWin::changePass()
   {
      if(!ValidPass(new_pass()))Gui.msgBox(S, "Invalid New Password");else
      if(use_key)
      {
         if(!cur().length())Gui.msgBox(S, "Invalid key");else
         {
            uint key=TextUInt(cur());
            Server.changePass(Server.conn_addr, Server.conn_email, new_pass(), key);
            hide();
         }
      }else
      if(Server.loggedIn())
      {
         if(!ValidPass(cur()))Gui.msgBox(S, "Invalid Current Password");else
         if(cur()==new_pass())Gui.msgBox(S, "Passwords are the same");else
         {
            ClientSendChangePass(Server, Server.logged_email, new_pass(), cur());
         }
      }
   }
   void ChangePassWin::activate(bool use_key)
   {
      ::EE::GuiObj::activate();
      T.use_key=use_key;
      t_cur.set(use_key ? "Key" : "Current Password");
        cur.clear().activate();
      new_pass.clear();
      if(use_key)Gui.msgBox(S, "Unique key has been sent to your email address.\nPlease look into your inbox, and use that key to change your password.");
   }
   void ChangePassWin::create()
   {
      Gui+=::EE::Window::create(Rect_C(0, 0, 0.8f, 0.5f), "Change Password").hide(); button[2].show();
      T+=t_cur     .create(Vec2  (clientWidth()/2, -0.05f));
      T+=  cur     .create(Rect_C(clientWidth()/2, -0.12f, 0.65f, 0.06f)).maxLength(16);
      T+=t_new_pass.create(Vec2  (clientWidth()/2, -0.20f), "New Password");
      T+=  new_pass.create(Rect_C(clientWidth()/2, -0.26f, 0.65f, 0.06f)).maxLength(16);
      T+=  ok      .create(Rect_C(clientWidth()/2, -0.36f, 0.4f, 0.06f), "Change Password").func(ChangePass, T);
   }
   void ChangePassWin::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){Kb.eatKey(); changePass();}
   }
Projects::Projects() : proj_menu_id(UIDZero) {}

Projects::Elm::Elm() : sync_val(-1) {}

RenameProjWin::RenameProjWin() : proj_id(UIDZero) {}

RemoveProjWin::RemoveProjWin() : proj_id(UIDZero) {}

/******************************************************************************/
