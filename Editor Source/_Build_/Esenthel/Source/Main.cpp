/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
/******************************************************************************

      Objects must use lock/unlock before modifying its elements (because background-thread-world-building loads area object 'Object.phys' read-only)
       Meshes must use lock/unlock before modifying its elements (because background-thread-world-building accesses mesh        data       read-only)
   PhysBodies must use lock/unlock before modifying its elements (because background-thread-world-building accesses phys bodies data       read-only)

/******************************************************************************/
bool        Initialized=false;
Str         SettingsPath, RunAtExit;
Environment DefaultEnvironment;
Threads     WorkerThreads, BuilderThreads, BackgroundThreads;
Str         CmdLine;
/******************************************************************************/
void ScreenChanged(flt old_width, flt old_height)
{
   VidOpt .setScale();
   VidOpt   .resize();
   Misc     .resize();
   Proj     .resize();
   Projs    .resize();
   Mode     .resize();
    MtrlEdit.resize(); WaterMtrlEdit.resize(); // resize after 'Proj'
     ObjEdit.resize();
    AnimEdit.resize();
   WorldEdit.resize();
 GuiSkinEdit.resize();
     GuiEdit.resize();
    CodeEdit.resize();
    CopyElms.resize();
  GameScreenChanged();
      ResizeInstall();
}
bool DisplayReset()
{
   SetKbExclusive(); // call because it depends on fullscreen mode
   return true;
}
void SetShader()
{
  Game::World.setShader();
   WorldEdit.setShader();
     ObjEdit.setShader();
 TexDownsize.setShader();
}
void Drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
{
        AppStore.drop(names, obj, screen_pos); // process this before CopyElms, so it can be uploaded to the Store
        CopyElms.drop(names, obj, screen_pos); // process this first so it will handle importing EsenthelProject's
            Proj.drop(names, obj, screen_pos);
        MtrlEdit.drop(names, obj, screen_pos);
   WaterMtrlEdit.drop(names, obj, screen_pos);
         ObjEdit.drop(names, obj, screen_pos);
   ImportTerrain.drop(names, obj, screen_pos);
       ImageEdit.drop(names, obj, screen_pos);
           Brush.drop(names, obj, screen_pos);
    AppPropsEdit.drop(names, obj, screen_pos);
       SplitAnim.drop(names, obj, screen_pos);
        CodeEdit.drop(names, obj, screen_pos);
     CreateMtrls.drop(names, obj, screen_pos);
   if(obj && obj->type()==GO_TEXTLINE && names.elms())obj->asTextLine().set(names[0]);
}
bool SaveChanges(void (*after_save_close)(bool all_saved, ptr user), ptr user)
{
   Memc<Edit::SaveChanges::Elm> elms;
   CodeEdit.saveChanges(elms);
   CodeEdit.saveChanges(elms, after_save_close, user);
   return !elms.elms();
}
void SetExit(bool all_saved, ptr) {if(all_saved)StateExit.set();}
void Quit() {if(StateActive==&StatePublish)PublishCancel();else SaveChanges(SetExit);}
void Resumed()
{
   if(StateActive==&StateProjectList
   && !(Projs.proj_list.contains(Gui.ms()) && Gui.ms()->type()==GO_CHECKBOX)) // if we're activating application by clicking on a project list checkbox, then don't refresh, because it would lose focus and don't get toggled, or worse (another project would be toggled due to refresh)
      Projs.refresh(); // refresh list of project when window gets focused in case we've copied some projects to the Editor
}
void ReceiveData(cptr data, int size, ptr hwnd_sender)
{
   File f; f.readMem(data, size);
   Str s=f.getStr();
   if(GetExt(s)==EsenthelProjectExt)CopyElms.display(s);
}
void SetTitle()
{
   Str title;
   if(Mode()==MODE_CODE)title=CodeEdit.title();
   if(title.is())title=S+AppName+" - "+title;else title=AppName;
   WindowSetText(title);
}
void SetKbExclusive()
{
   Kb.exclusive(D.full() || StateActive==&StateGame || (StateActive==&StateProject && (Mode()==MODE_OBJ || Mode()==MODE_ANIM || Mode()==MODE_WORLD || Mode()==MODE_TEX_DOWN)) || Theater.visible());
}
void SetProjectState()
{
   (Proj.valid() ? Proj.needUpdate() ? StateProjectUpdate : StateProject : StateProjectList).set(StateFadeTime); // if there's a project opened then go back to its editing, otherwise go to project list
}
Rect EditRect(bool modes)
{
   Rect r(-D.w(), -D.h(), D.w(), D.h());
   if(modes && Mode.visibleTabs())MIN(r.max.y,          Mode.rect().min.y+D.pixelToScreenSize(Vec2(0, 0.5f)).y);
   if(         Proj.visible    ())MAX(r.min.x,          Proj.rect().max.x-D.pixelToScreenSize(Vec2(0.5f, 0)).x);
   if(     MtrlEdit.visible    ())MIN(r.max.x,      MtrlEdit.rect().min.x);
   if(WaterMtrlEdit.visible    ())MIN(r.max.x, WaterMtrlEdit.rect().min.x);
   return r;
}
Environment& CurrentEnvironment() {if(Environment *env=EnvEdit.cur())return *env; return DefaultEnvironment;}
/******************************************************************************/
void InitPre()
{
   if(App.cmdLine().is()) // if given command line param
   {
      Memc<uint> proc; ProcList(proc);
      REPA(proc)if(proc[i]!=App.processID())
      {
         Str proc_name=GetBase(ProcName(proc[i]));
         if(proc_name=="Esenthel.exe" || proc_name=="Esenthel 32 DX9.exe")
            if(ptr hwnd=ProcWindow(proc[i]))
         {
            File f; f.writeMem().putStr(RemoveQuotes(App.cmdLine())).pos(0);
            Memt<byte> temp; temp.setNum(f.left()); f.get(temp.data(), temp.elms());
            WindowSendData(temp.data(), temp.elms(), hwnd);
            App.flag=APP_EXIT_IMMEDIATELY;
            return;
         }
      }
   }
   
   ASSERT(ELM_NUM==(int)Edit::ELM_NUM); // they must match exactly
   Str path=GetPath(App.exe()).tailSlash(true);
   InstallerMode=(STEAM ? false : !(FExistSystem(path+"Bin/Code Editor.dat") && FExistSystem(path+"Bin/Engine.pak") && FExistSystem(path+"Bin/Editor.pak") && FExistSystem(path+"Bin/Mobile/Engine.pak") && FExistSystem(path+"Bin/EsenthelEngine/EsenthelEngine.h") && FExistSystem(path+"Bin/Android/Ant")));
   App.x=App.y=0;
   App.receive_data=ReceiveData;
   D.screen_changed=ScreenChanged;

   App.flag|=APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE|APP_WORK_IN_BACKGROUND|APP_RESIZABLE;
   EE_INIT(false, false);
#if DEBUG
   App.flag|=APP_BREAKPOINT_ON_ERROR|APP_MEM_LEAKS|APP_CALLSTACK_ON_ERROR;
   Paks.add(EE_ENGINE_PATH);
   Paks.add(GetPath(EE_ENGINE_PATH).tailSlash(true)+"Editor.pak");
   if(ForceInstaller<0)InstallerMode=false;else if(ForceInstaller>0)InstallerMode=true;
   SetFbxDllPath(GetPath(EE_ENGINE_PATH).tailSlash(true)+"FBX32.dll", GetPath(EE_ENGINE_PATH).tailSlash(true)+"FBX64.dll");
#endif
   if(InstallerMode)
   {
      App.name(InstallerName);
   #if WINDOWS
      App.flag|=APP_ALLOW_NO_GPU;
   #endif
      D.mode(App.desktopW()*0.4f, App.desktopH()*0.6f);
      Renderer.type(RT_SIMPLE);
   }else
   {
      SupportCompressAll();
   #if LINUX
      SettingsPath=SystemPath(SP_APP_DATA).tailSlash(true)+".esenthel\\Editor\\";
   #elif DESKTOP
      SettingsPath=SystemPath(SP_APP_DATA).tailSlash(true)+"Esenthel\\Editor\\";
   #else
      SettingsPath=SystemPath(SP_APP_DATA).tailSlash(true);
   #endif
      FCreateDirs(SettingsPath);
      ProjectsPath=MakeFullPath("Projects\\"); CodeEdit.projectsBuildPath(ProjectsPath+ProjectsBuildPath);
      App.name(AppName);
      App.flag|=APP_FULL_TOGGLE;
      App.drop=Drop;
      App.quit=Quit;
      App.resumed=Resumed;
      D.secondaryOpenGLContexts(Cpu.threads()*3); // worker threads + importer threads + manually called threads
      D.drawNullMaterials(true);
      D.set_shader=SetShader;
      D.reset=DisplayReset;
      D.mode(App.desktopW()*0.8f, App.desktopH()*0.8f);
      D.highPrecNrmCalc(true);
   #if !DEBUG
      Paks.add("Bin/Engine.pak");
      Paks.add("Bin/Editor.pak");
   #endif
      Sky.atmospheric();
      InitElmOrder();
      LoadSettings();
      VidOpt.ctor(); // init before applying video settings
      ApplyVideoSettings();
   }
       WorkerThreads.create(true);
      BuilderThreads.create(false);
   BackgroundThreads.create(false, Cpu.threads()-1);
}
/******************************************************************************/
bool Init()
{
   if(InstallerMode)StateInstall.set();else
   {
      Updater     .create();
      UpdateWindow.create();

      if(!Physics.created())Physics.create(EE_PHYSX_DLL_PATH).timestep(PHYS_TIMESTEP_VARIABLE);

      const flt delay_remove=10;
      Meshes      .delayRemove(delay_remove);
      PhysBodies  .delayRemove(delay_remove);
      WaterMtrls  .delayRemove(delay_remove);
      Materials   .delayRemove(delay_remove);
      Fonts       .delayRemove(delay_remove);
      ImageAtlases.delayRemove(delay_remove);
      Images      .delayRemove(delay_remove);
      PanelImages .delayRemove(delay_remove);
      Panels      .delayRemove(delay_remove);
      TextStyles  .delayRemove(delay_remove);
      GuiSkins    .delayRemove(delay_remove);
          Objects .delayRemove(delay_remove);
      EditObjects .delayRemove(delay_remove);
      Environments.delayRemove(delay_remove);

      Images        .mode(CACHE_DUMMY);
      ImageAtlases  .mode(CACHE_DUMMY);
      Materials     .mode(CACHE_DUMMY);
      Meshes        .mode(CACHE_DUMMY);
      PhysMtrls     .mode(CACHE_DUMMY);
      PhysBodies    .mode(CACHE_DUMMY);
      Skeletons     .mode(CACHE_DUMMY);
      Animations    .mode(CACHE_DUMMY);
      ParticlesCache.mode(CACHE_DUMMY);
      Objects       .mode(CACHE_DUMMY);
      EditObjects   .mode(CACHE_DUMMY);
      Fonts         .mode(CACHE_DUMMY);
      Enums         .mode(CACHE_DUMMY);
      WaterMtrls    .mode(CACHE_DUMMY);
      PanelImages   .mode(CACHE_DUMMY);
      Panels        .mode(CACHE_DUMMY);
      TextStyles    .mode(CACHE_DUMMY);
      GuiSkins      .mode(CACHE_DUMMY);
      Environments  .mode(CACHE_DUMMY);

      InitGui();
   #if 0
      LicenseCheck.create(); // create before 'Buy'
      Buy.create(); // create after 'LicenseCheck'
   #endif
      ReloadElm.create();
      SplitAnim.create();
      CopyElms.create();
      SizeStats.create();
      CompareProjs.create();
      ProjSettings.create();
      Publish.create();
      EraseRemoved.create();
      ElmProps.create();
      ObjClassEdit.create();
      RenameSlot.create();
      RenameBone.create();
      RenameEvent.create();
      NewWorld.create();
      Projs.create();
      Importer.create();
      ImportTerrain.create();
      EnumEdit.create();
      FontEdit.create();
      PanelImageEdit.create();
      PhysMtrlEdit.create();
      TextStyleEdit.create();
      AppPropsEdit.create();
      SoundEdit.create();
      VideoEdit.create();
      ImageEdit.create();
      ImageAtlasEdit.create();
      MiniMapEdit.create();
      IconSettsEdit.create();
      IconEdit.create();
      PanelEdit.create();
      EnvEdit.create();
      SetMtrlColor.create();
      MulSoundVolume.create();
      Gui+=Calculator.create(Rect_C(0, 0, 1.0f, 0.235f)).hide();
      AppStore.create();
      MSM.create();
      DST.create();
      CreateMtrls.create();
      ConvertToAtlas.create();
      ConvertToDeAtlas.create();
      MtrlEdit.create(); WaterMtrlEdit.create(); // create before 'Mode' so it's below Code Editor ouput
      Theater.create(); // create before 'Mode' so it's below Code Editor ouput
      Proj.create(); // create before 'Mode' 

      Mode.create(); // create after 'MtrlEdit'
      Misc.create(); // create after 'MtrlEdit' so 'preview_big' doesn't overlap

      Preview.create(); // create after all elements to display preview on top
      RenameElm.create(); // create after 'Preview'
      ReplaceName.create();

      Proj.outer_region.moveAbove(Mode.close); // put above Mode so it's above obj/world/code edit (but below code editor output), so project region resize can be detected better (so it's on top of fullscreen editors)
      MtrlEdit.preview_big.moveAbove(Mode.close); WaterMtrlEdit.preview_big.moveAbove(Mode.close); Theater.moveAbove(Mode.close); // put above Mode so it's above obj/world/code edit (but below code editor output)

      StateProjectList.set();
      Sun.image="Gui/Misc/sun.img"; DefaultEnvironment.get(); // before 'ApplySettings'
      ApplySettings(); // !! after 'DefaultEnvironment' !!
      VidOpt.create(); // !! after 'ApplySettings' !!
      SetKbExclusive();
      AssociateFileType(EsenthelProjectExt, App.exe(), "Esenthel.Editor", "Esenthel Project", App.exe());
      CmdLine=RemoveQuotes(App.cmdLine());
      Initialized=true;
   }
   ScreenChanged();
   return true;
}
void Shut()
{
   // delete threaded objects
           NewLod.del();
           MeshAO.del();
         FontEdit.del();
   PanelImageEdit.del();

   ShutGui();
   Proj.close();
   if(Initialized)SaveSettings(); // save before deletion
   CodeEdit         .del();
   Updater          .del();
       WorkerThreads.del();
      BuilderThreads.del();
   BackgroundThreads.del();
   if(RunAtExit.is())Run(RunAtExit, S, false, App.elevated());
}
/******************************************************************************/
bool Update() {return false;}
void Draw  () {}
/******************************************************************************/

/******************************************************************************/
