/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
UpdaterClass Updater;
/******************************************************************************/
UpdateWindowClass UpdateWindow;
/******************************************************************************/
bool UpdateDo()
{
   Physics.del(); // del physics and unload DLL's

   bool inside=StartsPath(App.exe(), Updater.path);
   Str  old=App.exe(); // remember old app name

#if WINDOWS
   // move exe to temporary file so 'FMoveDir' will succeed for the exe as well
   if(inside)
   {
      Str rel=SkipStartPath(App.exe(), Updater.path);
      if(FExistSystem(Updater.update_path+rel)) // if there exists a replacement for that file
      {
         if(!App.renameSelf(Updater.path+"Esenthel Old."+GetExt(old))){Gui.msgBox(S, "Can't update self"); if(InstallerMode)StateInstall.set(StateFadeTime);else StateProjectList.set(StateFadeTime); return true;}
         App.deleteSelfAtExit(); // delete the "Esenthel Old" file
      }
   }
#endif

   // remove those that are not wanted (do this first, before replacing files)
   FREPA(Updater.local_remove) // go from start to recycle folders first
   {
      Str full=Updater.path+Updater.local_files[Updater.local_remove[i]].full_name;
   #if WINDOWS
      if(EqualPath(full, App.exe())){App.deleteSelfAtExit(); continue;} // on Windows we can't recycle self
   #endif
      FRecycle(full);
   }

   // replace all files
   FMoveDir(Updater.update_path, Updater.path);

   if(inside && FExistSystem(old))RunAtExit=old;

   if(InstallerMode)
   {
      Str ext;
   #if WINDOWS
      ext=".exe";
   #elif MAC
      ext=".app";
   #elif LINUX
      ext="";
   #else 
      #error unknown platform
   #endif
      CreateShortcut(Updater.path+"Esenthel"+ext, SystemPath(SP_DESKTOP  ).tailSlash(true)+"Esenthel");
      CreateShortcut(Updater.path+"Esenthel"+ext, SystemPath(SP_MENU_PROG).tailSlash(true)+"Esenthel");
   }

   // finish
   if(RunAtExit.is())
   {
      // if we run app on exit, then no messages need to be displayed
   }else // we won't run any app
   {
      Explore(Updater.path); // open folder so user can see what's happened there
      WindowHide(); // hide window because 'WindowMsgBox' causes a pause
      /*if(inside)*/WindowMsgBox("Success", InstallerMode ? "Esenthel has installed successfully." : "Esenthel has updated successfully."); // show confirmation using OS msg box
      //else        Gui.msgBox("Success", "Esenthel has updated properly.");
   }
   /*if(inside)*/return false; // close the application
   //else StateInstall.set(StateFadeTime); // if we're not inside then it means we're an installer
}
/******************************************************************************/
State StateUpdate(UpdateUpdate, DrawUpdate, InitUpdate, ShutUpdate);
Str   UpdateMessage;
/******************************************************************************/
bool InitUpdate()
{
   WindowSetWorking();
   SetKbExclusive();
   Proj.close();

   // auto close all processes in path folder except this one - App.exe
   Memc<uint> proc; ProcList(proc);
   REPA(proc)if(proc[i]!=App.processID())
   {
      Str proc_name=ProcName(proc[i]);
      if(StartsPath(proc_name, Updater.path))ProcClose(proc_name);
   }
   UpdateMessage.clear();

   return true;
}
void ShutUpdate()
{
   
}
bool UpdateUpdate()
{
   if(Kb.bp(KB_ESC) || Kb.bp(KB_NAV_BACK))
   {
      if(!InstallerMode)StateProjectList.set(StateFadeTime);
   }
   Time.wait(1000/30);
   Gui.update();
   Server.update(null, true);
   if(Ms.bp(3))WindowToggle();

   UpdateMessage.clear();
   Memc<uint> proc; ProcList(proc);
   REPA(proc)if(proc[i]!=App.processID())
   {
      Str proc_name=ProcName(proc[i]);
      if(StartsPath(proc_name, Updater.path))
      {
         if(!UpdateMessage.is())UpdateMessage+="Waiting for applications to exit:";
         UpdateMessage.line()+=proc_name;
      }
   }
   if(!UpdateMessage.is()) // if there are no active apps then perform update
      return UpdateDo();

   return true;
}
void DrawUpdate()
{
   D.clear(BackgroundColor());
   Gui.draw();
   D.text(Rect(-D.w(), -D.h(), D.w(), D.h()), UpdateMessage);
}
/******************************************************************************/
State    StateInstall(UpdateInstall, DrawInstall, InitInstall, ShutInstall);
WindowIO InstallIO;
bool     InstallerMode=false;
/******************************************************************************/
Str InstallPath()
{
   Str path=InstallIO.path(); path.tailSlash(true)+=InstallIO.subPath();
   if(InstallIO.textline().is())
   {
      if(FullPath(InstallIO.textline()))path=InstallIO.textline();else path.tailSlash(true)+=InstallIO.textline();
      path=Replace(NormalizePath(path), '/', '\\');
   }
   if(path.tailSlash(false).is())
   {
      Str end=GetBase(path); if(!Contains(end, "Esenthel"))path.tailSlash(true)+="Esenthel";
   }
   return path;
}
void SelectInstall(C Str &path, ptr)
{
   if(!InstallPath().is())InstallIO.activate();else Updater.create(); // reactivate if user selected empty/invalid path
}
void ResizeInstall()
{
   flt w=Min(0.6f, D.w());
   InstallIO.rect(Rect(-w, -D.h(), w, D.h()-0.17f));
}
/******************************************************************************/
bool InitInstall()
{
   SetKbExclusive();
   if(!InstallIO.is())
   {
      InstallIO.create(S, S, S, SelectInstall, SelectInstall).modeDirSelect(); FlagDisable(InstallIO.flag, WIN_MOVABLE|WIN_RESIZABLE);
      InstallIO.button[2].func(MiscRegion::Quit, Misc).hide();
      InstallIO.cancel   .func(MiscRegion::Quit, Misc);
      InstallIO.activate();
      ResizeInstall();
   }
   Gui+=UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
   Gui+=InstallIO;
   return true;
}
void ShutInstall()
{
   UpdateProgress.del();
   Gui-=InstallIO;
}
bool UpdateInstall()
{
   if(!App.active())Time.wait(1000/30);else if(!D.sync())Time.wait(1000/100);
   Gui.update();
   if(!InstallIO.visible() && !Updater.updating())
   {
      if(Updater.ready) // success
      {
         StateUpdate.set(StateFadeTime);
      }else // failed
      {
         InstallIO.activate();
         WindowSetError(Updater.progress());
      }
   }
   if(!InstallIO.visible())WindowSetProgress(Updater.progress());
   UpdateProgress.set(Updater.progress());
   if(Ms.bp(3))WindowToggle();
   return true;
}
void DrawInstall()
{
   D.clear(BackgroundColor());
   TextStyleParams ts;
   ts.size=0.055f; D.text(ts, 0, D.h()-0.05f, InstallIO.visible() ? "Please select path for Esenthel installation" : "Installing to");
   ts.size=0.045f; D.text(ts, 0, D.h()-0.11f, S+"\""+InstallPath()+'"');
   Gui.draw();
}
/******************************************************************************/

/******************************************************************************/
   cchar8 *UpdaterClass::TutorialsProjID="541oob6a_h_5el6-6!zan_50";
   const int UpdaterClass::MaxDownloadAttempts=3;
/******************************************************************************/
   bool UpdaterClass::CreateFailedDownload(int &failed, C Str &file, ptr user) {failed=0; return true;}
   FILE_LIST_MODE UpdaterClass::Filter(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;

      Str rel=SkipStartPath(ff.pathName(), updater.path);
      if( rel=="Settings.txt"
      ||  rel=="Server Settings"
      ||  rel=="Projects (Server)"
      ||  rel=="Esenthel Old.exe"
      ||  rel=="Esenthel Old.exe.bat" // used by 'App.deleteSelfAtExit'
      ||  rel=="Esenthel Old.app"
      ||  rel=="Esenthel Old"
      ||  rel=="Bin/Store.dat"
      ||  rel=="Bin/Code Editor.font"
      ||  EqualPath(rel, "Bin/Update"))return FILE_LIST_SKIP; // skip these elements

      if(StartsPath(rel, "Projects"))
      {
         Str project=SkipStartPath(rel, "Projects");
         if( project.is())
         {
            if(!StartsPath(project, TutorialsProjID))return FILE_LIST_SKIP; // list only default "Tutorials" project
         }
      }

      updater.local_files.New().set(rel, ff);
      return FILE_LIST_CONTINUE;
   }
   FILE_LIST_MODE UpdaterClass::FilterUpdate(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;
      updater.local_files.New().set(SkipStartPath(ff.pathName(), updater.update_path), ff);
      return FILE_LIST_CONTINUE;
   }
   FILE_LIST_MODE UpdaterClass::HasUpdate(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;
      Str rel=SkipStartPath(ff.pathName(), updater.update_path);
      if(StartsPath(rel, "Projects"))
      {
         Str project=SkipStartPath(rel, "Projects");
         if(!project.is()            )return FILE_LIST_CONTINUE; // keep checking inside but don't report "Projects"  folder  as an update
         if( project==TutorialsProjID)return FILE_LIST_SKIP    ; // don't check inside   and don't report "Tutorials" project as an update
      }
      updater.has_update=true; // this is a file other than "Projects" folder and "Tutorials" project, which means that it's a new update, report that we have an update and stop processing
      return FILE_LIST_BREAK;
   }
   bool UpdaterClass::hasUpdate()
   {
      if(0) // don't notify about updates if they're just about file removal
         if(C Pak *pak=patcher.index())
            REPA(local_remove) // iterate all files for removal
      {
         C Str &name=local_files[local_remove[i]].full_name;
         if(!pak->find(name))if(FExistSystem(path+name))return true; // if a file is not present on the server but exists locally
      }
      has_update=false; FList(update_path, HasUpdate, T); // check if there are any files to update (except "Tutorials" project)
      return has_update;
   }
   bool UpdaterClass::updating() {return thread.active();}
   flt  UpdaterClass::progress() {if(long total=patcher.filesSize())return dbl(patcher.progress())/total; return 0;}
   bool UpdaterClass::Update(Thread &thread)
   {
      if(InstallerMode)App.stayAwake(AWAKE_SYSTEM);
      bool ok=((UpdaterClass*)thread.user)->update();
      if(InstallerMode)App.stayAwake(AWAKE_OFF);
      return ok;
   }
   bool UpdaterClass::update() // !! this is called on secondary thread !!
   {
      // download latest ver
      Str upload_name;
   #if WINDOWS
      upload_name="Esenthel";
   #elif MAC
      upload_name="Esenthel Mac";
   #elif LINUX
      upload_name="Esenthel Linux";
   #else 
      #error unknown platform
   #endif
      failed_download.clear();
      patcher.create("http://www.esenthel.com/download/Patcher", upload_name);
      patcher.downloadIndex();

      // check local ver (do this before update ver)
      path=(InstallerMode ? InstallPath() : GetPath(App.exe())).tailSlash(true);
      FList(path, Filter, T); if(thread.wantStop())return false;

      // check local update ver (do this after original ver so these files will overwrite previous info)
      update_path=path+"Bin\\Update\\";
      FList(update_path, FilterUpdate, T); if(thread.wantStop())return false;

      // wait for patcher index
      for(; ; Time.wait(1))
      {
         if(thread.wantStop())return false;
         if(patcher.indexState()==DWNL_ERROR){if(InstallerMode)Gui.msgBox(S, "Can't access Esenthel Server"); return false;}
         if(patcher.indexState()==DWNL_DONE )break;
      }

      // process what to remove/download
      Memc<int> server_download;
      if(!patcher.compare(local_files, local_remove, server_download))return false;
      bool deleted=false; REPA(local_remove)deleted|=FDel(update_path+local_files[local_remove[i]].full_name); // from leafs to root to delete files first (we're operating on temporary files here so there's no need for 'FRecycle')
      if(  deleted) // if deleted any file then list local files again, in case now we have them up to date
      {
         local_files.clear();
         FList(       path, Filter      , T); if(thread.wantStop())return false;
         FList(update_path, FilterUpdate, T); if(thread.wantStop())return false;
         if(!patcher.compare(local_files, local_remove, server_download))return false;
      }
      FREPA(server_download)       patcher.downloadFile(server_download[i]); // from root to download folders first
       REPA( local_remove  )FDel(update_path+local_files[local_remove  [i]].full_name); // from leafs to root to delete files first (we're operating on temporary files here so there's no need for 'FRecycle')

      // download files
      bool ok=true;
      for(; ; )
      {
         if(thread.wantStop()){patcher.del(); return false;}
         Patcher::Downloaded download;
         if(patcher.getNextDownload(download))
         {
            if(download.success)
            {
               Str full=update_path+download.full_name;
               switch(download.type)
               {
                  case FSTD_DIR: FCreateDirs(full); break;

                  case FSTD_LINK:
                  {
                     FCreateDirs(GetPath(full));
                     FDelFile(full);
                     if(CreateSymLink(full, DecodeSymLink(download.data)))FTimeUTC(full, download.modify_time_utc);else{Gui.msgBox("Error", S+"Can't write to:\n\""+full+'"'); patcher.del(); return false;}
                  }break;

                  case FSTD_FILE:
                  {
                     FCreateDirs(GetPath(full));
                     if(!SafeOverwrite(download.data, full, &download.modify_time_utc)){Gui.msgBox("Error", S+"Can't write to:\n\""+full+"\"\nTry running as administrator."); patcher.del(); return false;}
                  }break;
               }
            }else
            {
               int &failed =*failed_download(download.full_name);
               if(++failed>=MaxDownloadAttempts)
               {
                  ok=false;
                  if(InstallerMode){Gui.msgBox(S, S+"Error downloading file:\n\""+download.full_name+"\"\nPlease try again.\nUsing the same path will resume the download."); patcher.del(); return false;}
               }else // try again
               {
                  patcher.downloadFile(download.index);
               }
            }
         }else
         if(!patcher.filesLeft())break;else Time.wait(1);
      }

      // update
      if(ok) // if all succeeded
         if(InstallerMode || hasUpdate()) // there is an actual update
      {
         show=ready=true;
      }

      return false;
   }
   void UpdaterClass::create()
   {
      del();
   #if DESKTOP && !STEAM
      #if DEBUG
         if(ForceInstaller>=-1)
      #endif
            thread.create(Update, this);
   #endif
   }
   void UpdaterClass::del()
   {
      thread.del(); // delete the thread first
      ready=show=false;
      patcher     .del();
      local_remove.del();
      local_files .del();
   }
  UpdaterClass::~UpdaterClass() {del();}
   void UpdateWindowClass::ApplyDo(bool all_saved, ptr) {if(all_saved){UpdateWindow.hide(); if(Updater.ready)StateUpdate.set(StateFadeTime);}}
   void UpdateWindowClass::ShowChanges(UpdateWindowClass &uw) {Explore("http://www.esenthel.com/forum/forumdisplay.php?fid=8");}
   void UpdateWindowClass::Apply(UpdateWindowClass &uw) {if(StateActive==&StateProject)SaveChanges(ApplyDo);else ApplyDo();}
   void UpdateWindowClass::create()
   {
      Gui+=::EE::Window::create(Rect_C(0, 0, 1, 0.48f)).barVisible(false).hide(); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      T  +=text        .create(Vec2  (clientWidth()/2, -0.19f), "An update to Esenthel is available.\nWould you like to apply it now?\n\nWarning: Applying update will restore the default\n\"Tutorials\" project to its original state.\nAny changes you've made to it will be lost.");
      T  +=apply       .create(Rect_D(clientWidth()*1/6, -clientHeight()+0.04f, 0.26f, 0.06f), "Apply"       ).focusable(false).func(Apply, T);
      T  +=show_changes.create(Rect_D(clientWidth()/2  , -clientHeight()+0.04f, 0.32f, 0.06f), "Show Changes").focusable(false).func(ShowChanges, T);
      T  +=not_now     .create(Rect_D(clientWidth()*5/6, -clientHeight()+0.04f, 0.26f, 0.06f), "Not Now"     ).focusable(false).func(HideEditAct, SCAST(GuiObj, T));
   }
   GuiObj* UpdateWindowClass::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
      return (alpha()>=1) ? ::EE::Window::test(gpc, pos, mouse_wheel) : null; // check for alpha to avoid accidental clicks when window suddenly appears
   }
   void UpdateWindowClass::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Updater.show)
      {
         Updater.show=false;
         fadeIn();
      }
   }
UpdaterClass::UpdaterClass() : ready(false), show(false), has_update(false), failed_download(ComparePathCI, CreateFailedDownload) {}

/******************************************************************************/
