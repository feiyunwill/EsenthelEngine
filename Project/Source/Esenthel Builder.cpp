/******************************************************************************/
#include "stdafx.h"
using namespace Edit;
/******************************************************************************/
      Str VSPath=""; // leave empty for auto-detect or type full path like this: "C:/Program Files (x86)/Microsoft Visual Studio 14.0"
      Str AndroidNDKPath=""; // leave empty for auto-detect or type full path like this: "C:/Progs/AndroidNDK"
      Str EngineDataPath="Data/"; // this will get updated to full path
      Str EditorDataPath="Editor Data/"; // this will get updated to full path
      Str EnginePath="Engine/"; // this will get updated to full path
      Str EditorPath="Editor/"; // this will get updated to full path
      Str EditorSourcePath="Editor Source/_Build_/Esenthel/"; // this will get updated to full path
      Str ThirdPartyLibsPath="ThirdPartyLibs/"; // this will get updated to full path
      Str EsenthelPath; // this will be set to Esenthel Root full path
const Str      VSEngineProject="EsenthelEngine.sln";
const Str      VSEditorProject="Project.sln";
const Str   XcodeEngineProject="EsenthelEngine.xcodeproj";
const Str   XcodeEditorProject="Project.xcodeproj";
const Str   LinuxEngineProject="Linux/"; // if not empty then should include tail slash
const Str   LinuxEditorProject=""; // if not empty then should include tail slash
const Str AndroidProject="Android/"; // if not empty then should include tail slash
const Str OptionsFileName="Esenthel Builder.txt";
/******************************************************************************/
struct TaskBase;
MemcThreadSafe<TaskBase*> Todo;

struct TaskBase
{
   CChar8  *name, *desc;
   void   (*func)();
   Bool     on, immediate;

   void call () {if(func)func();}
   void queue() {if(immediate)call();else Todo.include(this);}
};
struct Task
{
   CheckBox  cb;
   Button   _queue;
   TaskBase *task;

   static void Queue(Task &task) {task.queue();}
          void queue(          ) {if(task)task->queue();}

   void create(TaskBase &tb)
   {
      task=&tb;
      Gui+= cb   .create().set(tb.on);
      Gui+=_queue.create(tb.name).func(Queue, T).desc(tb.desc);
   }
   void pos(Flt x, Flt y)
   {
      cb   .rect(Rect_L(x     , y, 0.05, 0.05));
     _queue.rect(Rect_L(x+0.06, y, 0.45, 0.05));
      Flt tw=_queue.textWidth(), rw=_queue.rect().w()-0.01; if(tw>rw)_queue.text_size*=rw/tw;
   }
};
/******************************************************************************/
static CChar8 *Physics_t[]=
{
   "PhysX (Recommended)", // 0
   "Bullet"             , // 1
}; ASSERT(PHYS_ENGINE_PHYSX==0 && PHYS_ENGINE_BULLET==1);

void PhysXCompileWindows(Ptr=null);
void PhysXCompileMac    (Ptr=null);
void PhysXCompileiOS    (Ptr=null);
void PhysXCompileLinux  (Ptr=null);
void PhysXCompileAndroid(Ptr=null);
void PhysXCompileWeb    (Ptr=null);
void PhysXCompileAll    (Ptr)
{
   if(WINDOWS)PhysXCompileWindows();
   if(MAC    )PhysXCompileMac    ();
   if(MAC    )PhysXCompileiOS    ();
   if(LINUX  )PhysXCompileLinux  ();
              PhysXCompileAndroid();
   if(WINDOWS)PhysXCompileWeb    ();
}

STRUCT(OptionsClass , ClosableWindow)
//{
   Text   t_vs_path, t_ndk_path, t_aac, t_physics;
   TextLine vs_path,   ndk_path;
   CheckBox aac;
   ComboBox physics;
   Button   physx_register, physx_download, physx_copy, physx_compile_win, physx_compile_mac, physx_compile_ios, physx_compile_linux, physx_compile_android, physx_compile_web, physx_compile_all, physx_del_temp, physx_del_lib, physx_recycle_all;

   static void  OptionChanged(OptionsClass &options) {options.saveConfig();}
   static void PhysicsChanged(OptionsClass &options)
   {
      if(options.physics()==PHYS_ENGINE_PHYSX)Gui.msgBox(S, "PhysX License doesn't allow its source/libraries to be redistributed. In order to compile the Engine with PhysX support, you will have to download and compile it using the buttons below.");
      options.setVis();
      options.saveConfig();
   }
   static void PhysXRegister(OptionsClass &options) {Explore("https://developer.nvidia.com/gameworks-source-github");} // https://developer.nvidia.com/developer-program/signup, https://developer.nvidia.com/content/apply-access-nvidia-physx-source-code
   static void PhysXDownload(OptionsClass &options) {Explore("https://github.com/NVIDIAGameWorks/PhysX-3.4");}
   static void PhysXCopy    (OptionsClass &options) {Explore(ThirdPartyLibsPath+"PhysX");}
   static void PhysXDelTemp (Ptr)
   {
      // Windows
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win64/.vs");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win64/Debug");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win64/x64");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win64/Emscripten");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win64/vc140.pdb");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win32/.vs");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win32/Debug");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win32/Win32");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/vc14win32/vc140.pdb");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win64/.vs");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win64/Debug");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win64/x64");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win64/Emscripten");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win64/vc140.pdb");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win32/.vs");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win32/Debug");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win32/Win32");
      FDel(ThirdPartyLibsPath+"PhysX/PxShared/src/compiler/vc14win32/vc140.pdb");
      // FIXME delete web/ARM/UWP

      // Android, keep other *.o files as they're needed for linking with the Engine
      FDel(ThirdPartyLibsPath+"PhysX/Android/obj/local/arm64-v8a/libPhysX.a");
      FDel(ThirdPartyLibsPath+"PhysX/Android/obj/local/armeabi-v7a/libPhysX.a");
      FDel(ThirdPartyLibsPath+"PhysX/Android/obj/local/x86/libPhysX.a");

      // Mac/iOS
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_osx64/build");
      FDel(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_ios64/build");

      // Linux is not deleted because *.o files are needed for linking with the Engine and *.a for Project
   }
   static void PhysXRecycle(Ptr)
   {
      for(FileFind ff(ThirdPartyLibsPath+"PhysX"); ff(); )if(ff.name!="Dummy" && ff.name!="Android")if(!FRecycle(ff.pathName()))ErrorDel(ff.pathName());
      FDelDirs(ThirdPartyLibsPath+"PhysX/Android/obj");
   }

   void setVis()
   {
      physx_register       .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_download       .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_copy           .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_win    .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_mac    .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_ios    .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_linux  .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_android.visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_web    .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_compile_all    .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_del_temp       .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_del_lib        .visible(physics()==PHYS_ENGINE_PHYSX);
      physx_recycle_all    .visible(physics()==PHYS_ENGINE_PHYSX);
   }
   void create()
   {
      Gui+=super::create(Rect_C(0, 0, 1.4, 0.60), "Options").hide(); button[2].show();
      Flt y=-0.05, h=0.06;
      T+=t_vs_path .create(Vec2(0.19, y), "Visual Studio Path"); T+= vs_path.create(Rect_L(0.38, y, 1.0, 0.05)); y-=h;
      T+=t_ndk_path.create(Vec2(0.19, y),   "Android NDK Path"); T+=ndk_path.create(Rect_L(0.38, y, 1.0, 0.05)); y-=h;
      T+=t_aac     .create(Vec2(0.19, y),   "Use Patented AAC"); T+=aac.create(Rect_L(0.38, y, 0.05, 0.05)).func(OptionChanged, T).desc("If support AAC decoding which is currently covered by patents.\nIf disabled then playback of AAC sounds will not be available."); y-=h;
      T+=t_physics .create(Vec2(0.19, y),     "Physics Engine"); T+=physics .create(Rect_L(0.38, y, 1.0, 0.05), Physics_t, Elms(Physics_t)).func(PhysicsChanged, T).set(PHYS_ENGINE_BULLET, QUIET); y-=h;
      T+=physx_register.create(Rect_L(                            0.02, y, 0.50, 0.05), "1. Register to Access PhysX").func(PhysXRegister, T);
      T+=physx_download.create(Rect_L(physx_register.rect().max.x+0.01, y, 0.35, 0.05), "2. Download PhysX").func(PhysXDownload, T);
      T+=physx_copy    .create(Rect_L(physx_download.rect().max.x+0.01, y, 0.35, 0.05), "3. Place PhysX here").func(PhysXCopy, T); y-=h;
      Flt x=0.02;
      if(WINDOWS){T+=physx_compile_win    .create(Rect_L(x, y, 0.48, 0.05), "Compile PhysX for Windows").func(PhysXCompileWindows); x=physx_compile_win    .rect().max.x+0.01;}
      if(MAC    ){T+=physx_compile_mac    .create(Rect_L(x, y, 0.41, 0.05), "Compile PhysX for Mac"    ).func(PhysXCompileMac    ); x=physx_compile_mac    .rect().max.x+0.01;}
      if(MAC    ){T+=physx_compile_ios    .create(Rect_L(x, y, 0.41, 0.05), "Compile PhysX for iOS"    ).func(PhysXCompileiOS    ); x=physx_compile_ios    .rect().max.x+0.01;}
      if(LINUX  ){T+=physx_compile_linux  .create(Rect_L(x, y, 0.43, 0.05), "Compile PhysX for Linux"  ).func(PhysXCompileLinux  ); x=physx_compile_linux  .rect().max.x+0.01;}
                 {T+=physx_compile_android.create(Rect_L(x, y, 0.46, 0.05), "Compile PhysX for Android").func(PhysXCompileAndroid); x=physx_compile_android.rect().max.x+0.01;}
    //if(WINDOWS){T+=physx_compile_web    .create(Rect_L(x, y, 0.40, 0.05), "Compile PhysX for Web"    ).func(PhysXCompileWeb    ); x=physx_compile_web    .rect().max.x+0.01;} FIXME
      y-=h;
                  T+=physx_compile_all    .create(Rect_C(clientWidth()/2, y, clientWidth()-0.02*2, 0.05), "Compile PhysX for All").func(PhysXCompileAll);
      y-=h*1.5;
      T+=physx_del_temp   .create(Rect_L(                            0.02, y, 0.46, 0.05), "Delete PhysX Temporaries"   ).func(PhysXDelTemp).desc("This will delete all temporary files that were generated during compilation process, they're not needed anymore after successful compilation and can be deleted to free up disk space.");
    //T+=physx_del_lib    .create(Rect_L(physx_del_temp.rect().max.x+0.01, y, 0.47, 0.05), "Delete PhysX Libraries"     ).func(PhysXDelLib ).desc("This will delete all libraries that were generated during compilation process, you can use this if you don't want to use PhysX anymore.");
      T+=physx_recycle_all.create(Rect_L(physx_del_temp.rect().max.x+0.01, y, 0.48, 0.05), "Recycle Entire PhysX folder").func(PhysXRecycle).desc("This will move all PhysX folder contents into the Recycle Bin, you can use this if you don't want to use PhysX anymore or you want to make a fresh reinstall of PhysX to its latest version.");
      load();
      loadConfig();
      saveConfig(); // always save
      setVis();
   }
   Bool any()C
   {
      return vs_path().is() || ndk_path().is();
   }
   void load()
   {
      TextData data; if(data.load(OptionsFileName))
      {
         if(TextParam *param=data.findNode("Visual Studio Path")) vs_path.set(param->asText());
         if(TextParam *param=data.findNode(  "Android NDK Path"))ndk_path.set(param->asText());
      }
   }
   void save()C
   {
      if(any())
      {
         TextData data;
         if( vs_path().is())data.nodes.New().set("Visual Studio Path",  vs_path());
         if(ndk_path().is())data.nodes.New().set(  "Android NDK Path", ndk_path());
         data.save(OptionsFileName);
      }else
      {
         FDelFile(OptionsFileName);
      }
   }
   static Str EsenthelConfig() {return EnginePath+"Esenthel Config.h";}
   void loadConfig()
   {
      FileText f; if(f.read(EsenthelConfig()))
      {
         Str s; f.getAll(s);
         aac    .set(Contains(s, "SUPPORT_AAC 1"), QUIET);
         physics.set(Contains(s, "PHYSX 1") ? PHYS_ENGINE_PHYSX : PHYS_ENGINE_BULLET, QUIET); // remember that PhysX can be enabled only for certain platforms, so check if there's at least one "PHYSX 1" occurrence
      }else
      {
         activate(); // if no config available then show options
      }
   }
   void saveConfig()C
   {
      Str s=S+"// File automatically generated by \""+App.name()+"\"\n";
      s+=S+"#define SUPPORT_AAC "+aac()+'\n';
    //s+="#define PHYSX "+(physics()==PHYS_ENGINE_PHYSX)+'\n';
      if(physics()==PHYS_ENGINE_PHYSX)
      {
         s+="#if !IOS_SIMULATOR && !WINDOWS_NEW && !WEB\n"
            "   #define PHYSX 1\n"
            "#else\n"
            "   #define PHYSX 0\n"
            "#endif\n";
      }else s+="#define PHYSX 0\n";
      FileText f; f.writeMem(ANSI).putText(s);
      Bool changed; if(!OverwriteOnChange(f, EsenthelConfig(), &changed)){ErrorWrite(EsenthelConfig()); return;}
      if(  changed)FTimeUTC(EnginePath+"H/_/Esenthel Config.h", DateTime().getUTC()); // set modification time to the default config as well, to make sure to force rebuild (this is needed for VS and possibly others too)
   }
}Options;
/******************************************************************************/
Str DevEnvPath(C Str &vs_path)
{
   if(vs_path.is())
   {
      Str devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\devenv.exe"   ; if(FExistSystem(devenv))return devenv; // pro
          devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\VCExpress.exe"; if(FExistSystem(devenv))return devenv; // express
          devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\WDExpress.exe"; if(FExistSystem(devenv))return devenv; // express for Windows Desktop
   }
   return S;
}
Str DevEnvPath()
{
   if(Options.vs_path().is()){Str p=DevEnvPath(Options.vs_path()); if(p.is())return p;}
   if(           VSPath.is()){Str p=DevEnvPath(         VSPath  ); if(p.is())return p;}
   return S;
}
Str NDKBuildPath(C Str &ndk_path)
{
   if(ndk_path.is())
   {
      Str ndk_build=Str(ndk_path).tailSlash(true)+PLATFORM("ndk-build.cmd", "ndk-build");
      if(FExistSystem(ndk_build))return ndk_build;
   }
   return S;
}
Str NDKBuildPath()
{
   if(Options.ndk_path().is()){Str p=NDKBuildPath(Options.ndk_path()); if(p.is())return p;}
   if(    AndroidNDKPath.is()){Str p=NDKBuildPath(  AndroidNDKPath  ); if(p.is())return p;}
   return S;
}
Str ARPath()
{
#if WINDOWS
   return EnginePath+AndroidProject+"cygwin/ar.exe";
#else
   return "ar";
#endif
}
/******************************************************************************/
Bool shut; // if 'Shut' called
Int  failed, // number of failures (like compilation thread failes)
     done;

SyncLock lock;

// BUILD OUTPUT
Memc<Str> data,     // list data
          data_new; // data currently added on a secondary thread, and to be added to the main data on the main thread
List<Str> list;     // list
Region    region;   // region
Button    clear, copy;

struct Build
{
   Str  exe, params, log;
   void (*pre)(), (*func)();
   
   Build& set   (C Str &exe, C Str &params, C Str &log=S) {T.exe=exe; T.params=params; T.log=log; return T;}
   Build& set   (void func()                            ) {T.func=func;                           return T;}
   Build& setPre(void func()                            ) {T.pre =func;                           return T;}

   Build() {pre=func=null;}

   static Str GetOutput(ConsoleProcess &cp, C Str &log, Str &processed)
   {
      Str out=cp.get();
      if(log.is())
      {
         FileText f; if(f.read(log))
         {
            Str all=f.getAll();
            out+=SkipStart(all, processed);
            processed=all;
         }
      }
      return out;
   }
   Bool run()
   {
      Bool ok=true;
      if(pre)pre();
      if(exe.is())
      {
         FDelFile(log);
         ConsoleProcess cp; cp.create(exe, params);
         Str processed, output; Memc<Str> outs;

         if(log.is())output="Compile start\n";

         for(; cp.active(); )
         {
            if(shut){cp.kill(); break;}
            cp.wait(25);
            output+=GetOutput(cp, log, processed);
            Split(outs, output, '\n');
            if(outs.elms()>=2)
            {
               SyncLocker locker(lock);
               for(; outs.elms()>=2; )
               {
                  Swap(data_new.New(), outs[0]); outs.remove(0, true);
               }
               output=outs.last();
            }
         }

         ok=(cp.exitCode()==0);

         output+=GetOutput(cp, log, processed);
         ok|=(Contains(output, "========== Build: 1 succeeded") || Contains(output, "========== Build: 0 succeeded, 0 failed, 1 up-to-date"));
         Split(outs, output, '\n');
         {
            SyncLocker locker(lock);
            FREPA(outs)if(outs[i].is())Swap(data_new.New(), outs[i]);
         }

         cp.del();

         if(!ok)
         {
            // fail
            Gui.msgBox("Error", "Compilation failed.\nTasks aborted.");
            Todo.clear();
         }
         FDelFile(log);
      }
      if(ok && func)func();
      return ok;
   }
};
/******************************************************************************/
Memb<Build> build_requests; // builds requested to be processed on secondary threads
Threads     build_threads;

Memx<Task> Tasks;
CheckBox   All;
Button     DoSel, OptionsButton;
/******************************************************************************/
void OptionsToggle(Ptr) {Options.visibleToggleActivate();}
void ToggleAll(Ptr) {REPAO(Tasks).cb.set(All());}
void CopyOutput(Ptr) {Str s; FREPA(data){s+=data[i]; s+='\n';} ClipSet(s);}
void Clear(Ptr) {list.setData(data.clear());}
void DoSelected(Ptr) {FREPA(Tasks)if(Tasks[i].cb())Tasks[i].queue();}
void BuildRun(Build &build, Ptr user, Int thread_index) {build.run();}
/******************************************************************************/
CChar8 *separator="/******************************************************************************/";
CChar8 *copyright="/******************************************************************************\r\n"
                  " * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *\r\n"
                  " * Esenthel Engine (http://www.esenthel.com) header file.                     *\r\n";
/******************************************************************************/
FILE_LIST_MODE Header(C FileFind &ff, Ptr)
{
   if(ff.type==FSTD_FILE)if(ff.name!="Esenthel Config.h")
   {
      Str name=ff.pathName();

      // read from source file
      FileText f; f.read(name);
      Meml<Str> src; for(; !f.end(); )src.New()=f.fullLine();

      // remove empty line followed by "#ifdef"
      //SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && Contains(src[_next_], "#ifdef"))src.remove(i);

      // hide EE_PRIVATE sections
      enum STACK_TYPE
      {
         STACK_NONE       ,
         STACK_PRIVATE    ,
         STACK_NOT_PRIVATE,
      };
      Memb<STACK_TYPE> stack  ; // #if stack
      Int              level=0; // EE_PRIVATE depth level
      for(MemlNode *node=src.first(); node;)
      {
         MemlNode *next=node->next();
         
       C Str &s    =src[node];
         Str  first=SkipWhiteChars(s);
         if(first.first()=='#') // if preprocessor command
         {
            if(Starts(first, "#if")) // #if #ifdef #ifndef
            {
               if(Starts(first, "#if EE_PRIVATE"))
               {
                  stack.add(STACK_PRIVATE);
                  level++;
                  src.remove(node);
               }else
               if(Starts(first, "#if !EE_PRIVATE"))
               {
                  stack.add(STACK_NOT_PRIVATE);
                  src.remove(node);
               }else
               {
                  stack.add(STACK_NONE);
                  if(level)src.remove(node);
               }
            }else
            if(Starts(first, "#el")) // #else #elif
            {
               if(stack.elms() && stack.last()==STACK_PRIVATE)
               {
                  stack.last()=STACK_NOT_PRIVATE; // #if !EE_PRIVATE
                  level--;
                  src.remove(node);
               }else
               if(stack.elms() && stack.last()==STACK_NOT_PRIVATE)
               {
                  stack.last()=STACK_PRIVATE;
                  level++;
                  src.remove(node);
               }else
               {
                  if(level)src.remove(node);
               }
            }else
            if(Starts(first, "#endif"))
            {
               if(stack.elms())
               {
                  if(stack.last() || level)src.remove(node);
                  if(stack.last()==STACK_PRIVATE)
                  {
                     level--;
                     if(!level)
                     {
                        if(next->prev())
                        {
                           Str a=src[next->prev()],
                               b=src[next        ];

                           if(!SkipWhiteChars(a).is() && Contains(b, separator))src.remove(next->prev());else // remove empty line followed  by /**/
                           if(!SkipWhiteChars(b).is() && Contains(a, separator))                              // remove empty line preceeded by /**/
                           {
                              MemlNode *temp=next->next(); src.remove(next); next=temp;
                           }
                        }
                     }
                  }
                  stack.removeLast();
               }else
               {
                  Exit(S+"Invalid #endif at \""+name+'"');
               }
            }else
            {
               if(level)src.remove(node);
            }
         }else
         {
            if(level)src.remove(node);
         }

         node=next;
      }

      // remove double lines /**/
      SMFREP(src)if(_next_ && Contains(src[i], separator) && Equal(src[i], src[_next_]))src.remove(i);

      // remove double empty lines
      SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && !SkipWhiteChars(src[_next_]).is())src.remove(i);

      // remove empty line followed by "}"
      SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && Starts(SkipWhiteChars(src[_next_]), "}"))src.remove(i);

      // remove empty line preceeded by "}"
      SMFREP(src)if(!SkipWhiteChars(src[i]).is() && i->prev() && Starts(SkipWhiteChars(src[i->prev()]), "{"))src.remove(i);

      // write to destination file
      Str dest=EditorPath+"Bin/EsenthelEngine/"+SkipStartPath(name, EnginePath+"H"); FCreateDirs(GetPath(dest));
                f.write  (dest);
                f.putText(copyright);
      MFREP(src)f.putLine(src[i]);
                f.del    ();
      FTimeUTC(dest, ff.modify_time_utc);
   }
   return FILE_LIST_CONTINUE;
}
/******************************************************************************/
static void Copy(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_DIFFERENT)
{
   if(!FCopy(src, dest, overwrite))
   {
      Gui.msgBox("Error", S+"Can't copy:\n\""+src+"\"\nto \n\""+dest+"\".\nTasks aborted.");
      Todo.clear();
   }
}
static void Del(C Str &name)
{
   if(FExistSystem(name) && !FDel(name))
   {
      Gui.msgBox("Error", S+"Can't delete:\n\""+name+"\".\nTasks aborted.");
      Todo.clear();
   }
}
static void ReplaceDir(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_DIFFERENT)
{
   if(!FReplaceDir(src, dest, overwrite))
   {
      Gui.msgBox("Error", S+"Can't copy:\n\""+src+"\"\nto \n\""+dest+"\".\nTasks aborted.");
      Todo.clear();
   }
}
void CleanEngineLinuxDo()
{
   FDel(EnginePath+LinuxEngineProject+"build");
   FDel(EnginePath+LinuxEngineProject+"dist");
   FDel(EnginePath+"stdafx.h.gch");
   FDel(EnginePath+"stdafx.h.pch");
}
void CleanEditorLinuxDo()
{
   FDel(EditorSourcePath+LinuxEditorProject+"build");
   FDel(EditorSourcePath+"stdafx.h.gch");
   FDel(EditorSourcePath+"stdafx.h.pch");
}
void CleanLinuxDo()
{
   CleanEngineLinuxDo();
   CleanEditorLinuxDo();
}
void CleanEngineWebDo()
{
   FDel(EnginePath+"Emscripten");
}
void MakeWebLibs()
{
   Str dest=EditorPath+"Bin/EsenthelEngine.bc";
   FDelFile(dest);

   Str params;
   params.space()+=S+'"'+EnginePath        +"EsenthelEngine.bc\"";
 //params.space()+=S+'"'+ThirdPartyLibsPath+"Brotli/Windows/Emscripten/Release/Brotli.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/BulletCollision.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/BulletDynamics.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/LinearMath.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"FDK-AAC/Windows/Emscripten/Release/FDK-AAC.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Flac/Windows/Emscripten/Release/FLAC.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"JpegTurbo/Web/Emscripten/Release/turbojpeg-static.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZ4/Windows/Emscripten/Release/LZ4.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZHAM/Windows/Emscripten/Release/LZHAM.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZMA/Windows/Emscripten/Release/Lzma.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Ogg/Windows/Emscripten/Release/Ogg.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/celt.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/opus.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/silk_common.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/silk_float.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/file/win32/VS2010/Emscripten/Release/opusfile.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Png/Windows/Emscripten/LIB Release/Png.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Recast/Windows/Emscripten/Release/Recast.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Snappy/Windows/Emscripten/Release/snappy.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"SQLite/Windows/Emscripten/Release/SQLite.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Theora/Windows/Emscripten/Release/Theora.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Vorbis/Windows/Emscripten/Release/Vorbis.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Vorbis/Windows/Emscripten/Release/Vorbis File.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Webp/Windows/Emscripten/Release/WebP.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Zlib/Windows/Emscripten/Release/Zlib.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Zstd/Windows/bin/Emscripten/Release/zstdlib.bc\"";
 //if(Options.physics()==PHYS_ENGINE_PHYSX)params.space()+=S+'"'+ThirdPartyLibsPath+"PhysX/../PhysX.bc\""; FIXME

   params.space()+=S+"-o \""+dest+'"';
   Build().set("emcc.bat", params).run();
   if(!FExistSystem(dest))Gui.msgBox("Error", "Can't create Esenthel Web Lib BitCode");
}
/******************************************************************************/
void TestDirForObjFiles(C Str &name, Str &obj_files)
{
   if(FileFind(name, "o")()) // if it has any *.o files inside
      obj_files.space()+=Replace(UnixPath(name+"/*.o"), " ", "\\ ");
}
FILE_LIST_MODE GatherObjFiles(C FileFind &ff, Str &obj_files)
{
   if(ff.type==FSTD_DIR)TestDirForObjFiles(ff.pathName(), obj_files);
   return FILE_LIST_CONTINUE;
}
void MakeLinuxLibs()
{
   Str engine_lib=EditorPath+"Bin/EsenthelEngine.a";
   FDelFile(engine_lib);

   // get a list of all possible libraries
   Memc<Str> lib_paths;
   lib_paths.add(EnginePath+LinuxEngineProject+"build/Release"); // Esenthel Engine
   lib_paths.add(ThirdPartyLibsPath+"PVRTC/PVRTex/Linux_x86_64/Static/build/Release"); // PVRTC (because it's stored inside a separate folder)
   lib_paths.add(ThirdPartyLibsPath+"VP/Linux"); // VP (because it's manually built)
   if(Options.physics()==PHYS_ENGINE_PHYSX)lib_paths.add(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/linux64/build"); // PhysX (because it's stored inside a separate folder and should be linked only if PhysX is selected as physics engine)
   // iterate all Third Party Libs
   for(FileFind ff(ThirdPartyLibsPath); ff(); )if(ff.type==FSTD_DIR)lib_paths.add(ff.pathName()+"/Linux/build/Release");

   // get all *.o files from those paths
   Str obj_files; FREPA(lib_paths){TestDirForObjFiles(lib_paths[i], obj_files); FList(lib_paths[i], GatherObjFiles, obj_files);}

   // make lib
   Build().set(ARPath(), S+"-q \""+UnixPath(engine_lib)+"\" "+obj_files).run();
   if(!FExistSystem(engine_lib))Gui.msgBox("Error", "Can't create Esenthel Linux Lib");
}
Str EngineAndroidLibName(C Str &abi) {return EditorPath+"Bin/Android/EsenthelEngine-"+abi+".a";}
void MakeAndroidLibs(C Str &abi)
{
   Str engine_lib=EngineAndroidLibName(abi);
   FDelFile(engine_lib);

   // get a list of all possible libraries
   Memc<Str> lib_paths;
   lib_paths.add(EnginePath+AndroidProject+"obj/local/"+abi); // Esenthel Engine
   // iterate all Third Party Libs
   for(FileFind ff(ThirdPartyLibsPath); ff(); )if(ff.type==FSTD_DIR)
      if(Options.physics()==PHYS_ENGINE_PHYSX || ff.name!="PhysX") // link PhysX only if we want it
         lib_paths.add(ff.pathName()+"/Android/obj/local/"+abi);

   // get all *.o files from those paths
   Str obj_files; FREPA(lib_paths){TestDirForObjFiles(lib_paths[i], obj_files); FList(lib_paths[i], GatherObjFiles, obj_files);}

   // make lib
   if(FExistSystem(engine_lib))Gui.msgBox("Error", S+"Can't remove Esenthel Android "+abi+" Lib");else
   {
      Str params=S+"-q \""+UnixPath(engine_lib)+"\" "+obj_files;
      if(WINDOWS && params.length()>=32000)Gui.msgBox("Warning", "Command Line may exceed Windows 32768 character limit");
      if(Build().set(ARPath(), params).run())
      {
         if(!FExistSystem(engine_lib))Gui.msgBox("Error", S+"Can't create Esenthel Android "+abi+" Lib");
      }else
      {
         Explore(engine_lib); // creating archive can sometimes fail, in that case there's a temp generated file that needs to be deleted manually
      }
   }
}
void MakeAndroidLibs()
{
   // #AndroidArchitecture
   MakeAndroidLibs("armeabi-v7a");
   MakeAndroidLibs("arm64-v8a");
#if 0
   MakeAndroidLibs("x86");
#else
   FDelFile(EngineAndroidLibName("x86"));
#endif
}
/******************************************************************************/
void UpdateHeaders()
{
   // process engine headers
   FDelDirs(EditorPath+"Bin/EsenthelEngine"); // delete
   FList(EnginePath+"H", Header); // convert
}
/******************************************************************************/
// ENGINE PAK
/******************************************************************************/
static Bool DesktopEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   return true;
}
static Bool UniversalEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   if(EqualPath(name, "Shader/3")
   || EqualPath(name, "Shader/GL")
   || base=="World Editor"
   )return false;
   return true;
}
static Bool MobileEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   if(EqualPath(name, "Shader/3")
   || EqualPath(name, "Shader/4")
   || base=="World Editor"
   )return false;
   return true;
}
static DateTime LatestTime(C Pak &pak)
{
   DateTime time; time.zero();
   REPA(pak)
   {
    C PakFile &pf=pak.file(i);
      if(!(pf.flag&PF_STD_DIR)) // not a directory
         if(pf.modify_time_utc>time)time=pf.modify_time_utc;
   }
   return time;
}
void CreateEnginePak()
{
   // Desktop "Engine.pak"
   Str engine_pak_name=EditorPath+"Bin/Engine.pak";
   Pak engine_pak; if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_NONE, 9, DesktopEnginePakFilter))Gui.msgBox("Error", "Can't create Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Windows Universal "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Universal/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_NONE, 9, UniversalEnginePakFilter))Gui.msgBox("Error", "Can't create Universal Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Mobile "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Mobile/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_NONE, 9, MobileEnginePakFilter))Gui.msgBox("Error", "Can't create Mobile Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Web "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Web/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_LZ4, 9, MobileEnginePakFilter))Gui.msgBox("Error", "Can't create Web Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files
}
/******************************************************************************/
// EDITOR PAK
/******************************************************************************/
Bool FilterEditorPak(C Str &name)
{
   Str engine=EngineDataPath+SkipStartPath(name, EditorDataPath);
   if(FileInfoSystem(engine).type==FSTD_FILE) // if file already exists in the Engine Data (Engine.pak) then skip (skip only files)
   {
      FRecycle(name); // remove to prevent uploading to GitHub
      return false;
   }
   return true;
}
void CreateEditorPak()
{
   Str editor_pak_name=EditorPath+"Bin/Editor.pak";
   Pak editor_pak; if(!editor_pak.create(EditorDataPath, editor_pak_name, PAK_SHORTEN, null, null, COMPRESS_NONE, 9, FilterEditorPak))Gui.msgBox("Error", "Can't create Editor.pak");
   DateTime editor_pak_time; editor_pak_time.zero();
   if(editor_pak.totalFiles()) // adjust pak date to latest date of its files
   {
      REPA(editor_pak)
      {
       C PakFile &pf=editor_pak.file(i);
         if(!(pf.flag&PF_STD_DIR)) // not a directory
         {
            DateTime dt=pf.modify_time_utc;
            if(dt>editor_pak_time)editor_pak_time=dt;
         }
      }
      FTimeUTC(editor_pak_name, editor_pak_time);
   }
}
/******************************************************************************/
void CompileVS(C Str &project, C Str &config, C Str &platform, void func()=null, void pre()=null)
{
   Str msbuild=MSBuildPath(Options.vs_path().is() ? Options.vs_path() : VSPath); if(FExistSystem(msbuild) && !Contains(platform, "Emscripten") && !Contains(platform, "Web")) // MSBuild is not compatible with Emscripten
   {
      build_threads.queue(build_requests.New().set(msbuild, MSBuildParams(project, config, platform)).set(func).setPre(pre), BuildRun);
   }else
   {
      Str devenv=DevEnvPath();
      if(!devenv.is()){Options.show(); Gui.msgBox("Error", "Visual Studio Path unknown");}else
      {
         Str log=S+GetPath(App.exe())+"/vs_build_"+Replace(config+platform, ' ', '_')+".txt";
         build_threads.queue(build_requests.New().set(devenv, VSBuildParams(project, config, platform, log), log).set(func).setPre(pre), BuildRun);
      }
   }
}
void CleanXcode(C Str &project, C Str &config, C Str &platform, C Str &sdk=S)
{
   build_threads.queue(build_requests.New().set("xcodebuild", XcodeBuildCleanParams(project, config, platform, sdk)), BuildRun);
}
void CompileXcode(C Str &project, C Str &config, C Str &platform, C Str &sdk=S, void func()=null)
{
   build_threads.queue(build_requests.New().set("xcodebuild", XcodeBuildParams(project, config, platform, sdk)).set(func), BuildRun);
}
void CompileLinux(C Str &project, C Str &config, void func()=null)
{
   build_threads.queue(build_requests.New().set("make", LinuxBuildParams(project, config)).set(func), BuildRun);
}
/******************************************************************************/
void CopyEngineWindows64GL              () {Copy(EnginePath+"EsenthelEngine64GL.lib"  , EditorPath+"Bin/EsenthelEngine64DX11.lib");} // copy as DX11
void CopyEngineWindows64DX9             () {Copy(EnginePath+"EsenthelEngine64DX9.lib" , EditorPath+"Bin/EsenthelEngine64DX9.lib");}
void CopyEngineWindows32DX9             () {Copy(EnginePath+"EsenthelEngine32DX9.lib" , EditorPath+"Bin/EsenthelEngine32DX9.lib");}
void CopyEngineWindows64DX11            () {Copy(EnginePath+"EsenthelEngine64DX11.lib", EditorPath+"Bin/EsenthelEngine64DX11.lib");}
void CopyEngineWindows32DX11            () {Copy(EnginePath+"EsenthelEngine32DX11.lib", EditorPath+"Bin/EsenthelEngine32DX11.lib");}
void CopyEngineWindowsUniversal64DX11   () {Copy(EnginePath+"EsenthelEngineUniversal64DX11.lib"   , EditorPath+"Bin/EsenthelEngineUniversal64DX11.lib");}
void CopyEngineWindowsUniversal32DX11   () {Copy(EnginePath+"EsenthelEngineUniversal32DX11.lib"   , EditorPath+"Bin/EsenthelEngineUniversal32DX11.lib");}
void CopyEngineWindowsUniversalArm32DX11() {Copy(EnginePath+"EsenthelEngineUniversalArm32DX11.lib", EditorPath+"Bin/EsenthelEngineUniversalArm32DX11.lib");}

void CopyEngineDebugWindows64DX9             () {Copy(EnginePath+"EsenthelEngineDebug64DX9.lib" , EditorPath+"Bin/EsenthelEngine64DX9.lib");}
void CopyEngineDebugWindows32DX9             () {Copy(EnginePath+"EsenthelEngineDebug32DX9.lib" , EditorPath+"Bin/EsenthelEngine32DX9.lib");}
void CopyEngineDebugWindows64DX11            () {Copy(EnginePath+"EsenthelEngineDebug64DX11.lib", EditorPath+"Bin/EsenthelEngine64DX11.lib");}
void CopyEngineDebugWindows32DX11            () {Copy(EnginePath+"EsenthelEngineDebug32DX11.lib", EditorPath+"Bin/EsenthelEngine32DX11.lib");}
void CopyEngineDebugWindowsUniversal64DX11   () {Copy(EnginePath+"EsenthelEngineDebugUniversal64DX11.lib"   , EditorPath+"Bin/EsenthelEngineUniversal64DX11.lib");}
void CopyEngineDebugWindowsUniversal32DX11   () {Copy(EnginePath+"EsenthelEngineDebugUniversal32DX11.lib"   , EditorPath+"Bin/EsenthelEngineUniversal32DX11.lib");}
void CopyEngineDebugWindowsUniversalArm32DX11() {Copy(EnginePath+"EsenthelEngineDebugUniversalArm32DX11.lib", EditorPath+"Bin/EsenthelEngineUniversalArm32DX11.lib");}

void CompileEngineWindows64GL              () {CompileVS(EnginePath+VSEngineProject, "Release GL"            , "1) 64 bit", CopyEngineWindows64GL);}
void CompileEngineWindows64DX9             () {CompileVS(EnginePath+VSEngineProject, "Release DX9"           , "1) 64 bit", CopyEngineWindows64DX9);}
void CompileEngineWindows32DX9             () {CompileVS(EnginePath+VSEngineProject, "Release DX9"           , "2) 32 bit", CopyEngineWindows32DX9);}
void CompileEngineWindows64DX11            () {CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "1) 64 bit", CopyEngineWindows64DX11);}
void CompileEngineWindows32DX11            () {CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "2) 32 bit", CopyEngineWindows32DX11);}
void CompileEngineWindowsUniversal64DX11   () {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "1) 64 bit", CopyEngineWindowsUniversal64DX11);}
void CompileEngineWindowsUniversal32DX11   () {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "2) 32 bit", CopyEngineWindowsUniversal32DX11);}
void CompileEngineWindowsUniversalArm32DX11() {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "3) ARM"   , CopyEngineWindowsUniversalArm32DX11);}
void CompileEngineWeb                      () {CompileVS(EnginePath+VSEngineProject, "Release GL"            , "4) Web");}

void CompileEngineDebugWindows64DX9             () {CompileVS(EnginePath+VSEngineProject, "Debug DX9"           , "1) 64 bit", CopyEngineDebugWindows64DX9);}
void CompileEngineDebugWindows32DX9             () {CompileVS(EnginePath+VSEngineProject, "Debug DX9"           , "2) 32 bit", CopyEngineDebugWindows32DX9);}
void CompileEngineDebugWindows64DX11            () {CompileVS(EnginePath+VSEngineProject, "Debug DX11"          , "1) 64 bit", CopyEngineDebugWindows64DX11);}
void CompileEngineDebugWindows32DX11            () {CompileVS(EnginePath+VSEngineProject, "Debug DX11"          , "2) 32 bit", CopyEngineDebugWindows32DX11);}
void CompileEngineDebugWindowsUniversal64DX11   () {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "1) 64 bit", CopyEngineDebugWindowsUniversal64DX11);}
void CompileEngineDebugWindowsUniversal32DX11   () {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "2) 32 bit", CopyEngineDebugWindowsUniversal32DX11);}
void CompileEngineDebugWindowsUniversalArm32DX11() {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "3) ARM"   , CopyEngineDebugWindowsUniversalArm32DX11);}

void  DelEditorExe          () {FDelFile(EditorSourcePath+"Esenthel.exe");} // VS has a bug that it won't rebuild the EXE if no changes were made in source (so if EXE was built with DX9 lib, and then we're compiling for DX11, then it may not be relinked)
void CopyEditorWindows64DX11() {    Copy(EditorSourcePath+"Esenthel.exe", EditorPath+"Esenthel.exe");}
void CopyEditorWindows32DX9 () {    Copy(EditorSourcePath+"Esenthel.exe", EditorPath+"Esenthel 32 DX9.exe");}

void CompileEditorWindows64DX11() {CompileVS(EditorSourcePath+VSEditorProject, "Release DX11", "1) 64 bit", CopyEditorWindows64DX11, DelEditorExe);}
void CompileEditorWindows32DX9 () {CompileVS(EditorSourcePath+VSEditorProject, "Release DX9" , "2) 32 bit", CopyEditorWindows32DX9 , DelEditorExe);}

void CleanEngineWeb    () {build_threads.queue(build_requests.New().set(CleanEngineWebDo), BuildRun);}
void            WebLibs() {build_threads.queue(build_requests.New().set(MakeWebLibs     ), BuildRun);}

void   CleanEngineApple() {CleanXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine Device");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine Simulator", "iphonesimulator");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine Device");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine Simulator", "iphonesimulator");}
void   CleanEditorMac  () {CleanXcode(EditorSourcePath+XcodeEditorProject, "Release", "Mac");}

void CopyEngineMac         () {Copy(EnginePath+"Build/Release/EsenthelEngine.a"                          , EditorPath+"Bin/EsenthelEngine.a");}
void CopyEngineiOS         () {Copy(EnginePath+"Build/Release-iphoneos/EsenthelEngine Device.a"          , EditorPath+"Bin/EsenthelEngine Device.a");}
void CopyEngineiOSSimulator() {Copy(EnginePath+"Build/Release-iphonesimulator/EsenthelEngine Simulator.a", EditorPath+"Bin/EsenthelEngine Simulator.a");}

void CopyEngineDebugMac         () {Copy(EnginePath+"Build/Debug/EsenthelEngine.a"                          , EditorPath+"Bin/EsenthelEngine.a");}
void CopyEngineDebugiOS         () {Copy(EnginePath+"Build/Debug-iphoneos/EsenthelEngine Device.a"          , EditorPath+"Bin/EsenthelEngine Device.a");}
void CopyEngineDebugiOSSimulator() {Copy(EnginePath+"Build/Debug-iphonesimulator/EsenthelEngine Simulator.a", EditorPath+"Bin/EsenthelEngine Simulator.a");}

void CopyEditorMac() {ReplaceDir(EditorSourcePath+"Esenthel.app", EditorPath+"Esenthel.app");}

void CompileEngineMac              () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine"          , ""               , CopyEngineMac);}
void CompileEngineiOS              () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine Device"   , ""               , CopyEngineiOS);}
void CompileEngineiOSSimulator     () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "EsenthelEngine Simulator", "iphonesimulator", CopyEngineiOSSimulator);}
void CompileEngineDebugMac         () {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine"          , ""               , CopyEngineDebugMac);}
void CompileEngineDebugiOS         () {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine Device"   , ""               , CopyEngineDebugiOS);}
void CompileEngineDebugiOSSimulator() {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "EsenthelEngine Simulator", "iphonesimulator", CopyEngineDebugiOSSimulator);}
void CompileEditorMac              () {CompileXcode(EditorSourcePath+XcodeEditorProject, "Release", "Mac"                     , ""               , CopyEditorMac);}

void    CopyEditorLinux    () {Copy(EditorSourcePath+"Esenthel", EditorPath+"Esenthel");}
void         CleanLinux    () {build_threads.queue(build_requests.New().set(CleanLinuxDo), BuildRun);}
void CompileEngineLinux    () {CompileLinux(EnginePath      +LinuxEngineProject, "Release");}
void CompileEditorLinux    () {CompileLinux(EditorSourcePath+LinuxEditorProject, "Release", CopyEditorLinux);}
void              LinuxLibs() {build_threads.queue(build_requests.New().set(MakeLinuxLibs), BuildRun);}

void CleanEngineAndroid()
{
   FDelDirs(EnginePath+AndroidProject+"obj");
}
void CompileEngineAndroid()
{
   Str ndk_build=NDKBuildPath();
   if(!ndk_build.is()){Options.show(); Gui.msgBox("Error", "Android NDK Path unknown");}else
   {
      Int build_threads_num=Cpu.threads();
      build_threads.queue(build_requests.New().set(ndk_build, S+"-j"+build_threads_num+" -C \""+EnginePath+AndroidProject+"\""), BuildRun);
   }
}
void AndroidLibs() {build_threads.queue(build_requests.New().set(MakeAndroidLibs), BuildRun);}

void  EnginePak() {build_threads.queue(build_requests.New().set(CreateEnginePak), BuildRun);}
void  EditorPak() {build_threads.queue(build_requests.New().set(CreateEditorPak), BuildRun);}
void    Headers() {build_threads.queue(build_requests.New().set(UpdateHeaders  ), BuildRun);}
void CodeEditorData()
{ // must be done on the main thread
   CE.create    (null, true); // don't use GUI because we don't need it, also it would require "Editor.pak" which may not be available yet
   CE.genSymbols(EditorPath+"Bin");
   CE.del       ();
}

TaskBase CompileEngineWindowsTB[]=
{
   {null, null, CompileEngineWindows32DX9},
   {null, null, CompileEngineWindows64DX9},
   {null, null, CompileEngineWindows32DX11},
   {null, null, CompileEngineWindows64DX11},
   {null, null, CompileEngineWindowsUniversal64DX11},
   {null, null, CompileEngineWindowsUniversal32DX11},
   {null, null, CompileEngineWindowsUniversalArm32DX11},
};
TaskBase CompileEngineDebugWindowsTB[]=
{
   {null, null, CompileEngineDebugWindows32DX9},
   {null, null, CompileEngineDebugWindows64DX9},
   {null, null, CompileEngineDebugWindows32DX11},
   {null, null, CompileEngineDebugWindows64DX11},
   {null, null, CompileEngineDebugWindowsUniversal64DX11},
   {null, null, CompileEngineDebugWindowsUniversal32DX11},
   {null, null, CompileEngineDebugWindowsUniversalArm32DX11},
};
TaskBase CompileEngineAppleTB[]=
{
   {null, null, CompileEngineMac},
   {null, null, CompileEngineiOS},
   {null, null, CompileEngineiOSSimulator},
};
TaskBase CompileEngineDebugAppleTB[]=
{
   {null, null, CompileEngineDebugMac},
   {null, null, CompileEngineDebugiOS},
   {null, null, CompileEngineDebugiOSSimulator},
};
void CompileEngineWindows     () {FREPAO(CompileEngineWindowsTB     ).queue();}
void CompileEngineDebugWindows() {FREPAO(CompileEngineDebugWindowsTB).queue();}
void CompileEngineApple       () {FREPAO(CompileEngineAppleTB       ).queue();}
void CompileEngineDebugApple  () {FREPAO(CompileEngineDebugAppleTB  ).queue();}

TaskBase CompileEditorWindowsTB[]=
{
   {null, null, CompileEditorWindows64DX11},
   {null, null, CompileEditorWindows32DX9 },
};
void CompileEditorWindows() {FREPAO(CompileEditorWindowsTB).queue();}
/******************************************************************************/
// PHYSX
/******************************************************************************/
static Bool PatchPhysXProject(C Str &path)
{
   for(FileFind ff(path, "vcxproj"); ff(); )
   {
      FileText f; if(f.read(ff.pathName()))
      {
         Str so=f.getAll(), s=so; Bool r=Contains(so, '\r');
         if(r)s.replace('\r', '\0');

         // VS 2017
         s=Replace(s, "<PlatformToolset>v140</PlatformToolset>"                         , "<PlatformToolset>v141_xp</PlatformToolset>"           , true, true);
         s=Replace(s, "<TreatWarningAsError>true</TreatWarningAsError>"                 , "<TreatWarningAsError>false</TreatWarningAsError>"     , true, true);
         // reduce size
         s=Replace(s, "<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>", "<DebugInformationFormat>None</DebugInformationFormat>", true, true);
         s=Replace(s, " /Fd\"$(TargetDir)\\$(TargetName).pdb\""                         , S                                                      , true, true); // one file has this specified in "LowLevelCloth" project which results in PDB missing notification on compilation
         s=Replace(s, " /Zi"                                                            , S                                                      , true, true); // one file has this specified in "LowLevelCloth" project which results in PDB missing notification on compilation
         // static lib
      #if PHYSX_DLL
         if(ff.name=="PhysX.vcxproj"
         || ff.name=="PhysXCommon.vcxproj"
         || ff.name=="PhysXCooking.vcxproj"
         || ff.name=="PxFoundation.vcxproj")
         {
            s=Replace(s, "<ConfigurationType>StaticLibrary</ConfigurationType>", "<ConfigurationType>DynamicLibrary</ConfigurationType>", true, true);
            s=Replace(s, "<TargetExt>.lib</TargetExt>"                         , "<TargetExt>.dll</TargetExt>"                          , true, true);
         }
         s=Replace(s, ".rc\">\n\t\t\t<ExcludedFromBuild>true</ExcludedFromBuild>\n\t\t</", ".rc\">\n\t\t</");
         s=Replace(s, ".rc\">\n      <ExcludedFromBuild>true</ExcludedFromBuild>\n    </", ".rc\">\n    </");
         s=Replace(s, ".rc\"><ExcludedFromBuild>true</ExcludedFromBuild></"              , ".rc\"></"      );
      #else
         s=Replace(s, "<ConfigurationType>DynamicLibrary</ConfigurationType>", "<ConfigurationType>StaticLibrary</ConfigurationType>", true, true);
         s=Replace(s, "<TargetExt>.dll</TargetExt>"                          , "<TargetExt>.lib</TargetExt>"                         , true, true);

         s=Replace(s, ".rc\">\n\t\t</", ".rc\">\n\t\t\t<ExcludedFromBuild>true</ExcludedFromBuild>\n\t\t</"); // projects have "*.rc" files included, because of which linking them into EsenthelEngine.lib file will fail, so we have to disable them
         s=Replace(s, ".rc\">\n    </", ".rc\">\n      <ExcludedFromBuild>true</ExcludedFromBuild>\n    </");
         s=Replace(s, ".rc\"></"      , ".rc\"><ExcludedFromBuild>true</ExcludedFromBuild></"              );
      #endif
         // debug
         s=Replace(s, "$(ProjectName)DEBUG", "$(ProjectName)");
         s=Replace(s, "DEBUG</TargetName>", "</TargetName>");
         s=Replace(s, "DEBUG.lib</OutputFile>", ".lib</OutputFile>");
         s=Replace(s, "DEBUG_x64</TargetName>", "_x64</TargetName>");
         s=Replace(s, "DEBUG_x64.lib</OutputFile>", "_x64.lib</OutputFile>");

         s=Replace(s, "$(ProjectName)CHECKED", "$(ProjectName)");
         s=Replace(s, "CHECKED</TargetName>", "</TargetName>");
         s=Replace(s, "CHECKED.lib</OutputFile>", ".lib</OutputFile>");
         s=Replace(s, "CHECKED_x64</TargetName>", "_x64</TargetName>");
         s=Replace(s, "CHECKED_x64.lib</OutputFile>", "_x64.lib</OutputFile>");

         s=Replace(s, "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>", "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>");
         s=Replace(s, "_DEBUG", "NDEBUG", true, true);
         
         if(r)s=Replace(s, "\n", "\r\n");
         if(so!=s)
         {
            f.writeMem(f.encoding()).putText(s);
            if(!SafeOverwrite(f, ff.pathName()))return ErrorWrite(ff.pathName());
         }
      }
   }
   return true;
}
static void CopyPhysXWindows64()
{
   Str files[]=
   {
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win64/PhysX3_x64.dll",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win64/PhysX3Common_x64.dll",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win64/PhysX3Cooking_x64.dll",
      ThirdPartyLibsPath+"PhysX/PxShared/bin/vc14win64/PxFoundation_x64.dll",
   };
   if(PHYSX_DLL)FREPA(files)Copy(files[i], EditorPath+"Bin/"+GetBase(files[i]));
   else         FREPA(files)Del (          EditorPath+"Bin/"+GetBase(files[i]));
}
static void CopyPhysXWindows32()
{
   Str files[]=
   {
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win32/PhysX3_x86.dll",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win32/PhysX3Common_x86.dll",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/vc14win32/PhysX3Cooking_x86.dll",
      ThirdPartyLibsPath+"PhysX/PxShared/bin/vc14win32/PxFoundation_x86.dll",
   };
   if(PHYSX_DLL)FREPA(files)Copy(files[i], EditorPath+"Bin/"+GetBase(files[i]));
   else         FREPA(files)Del (          EditorPath+"Bin/"+GetBase(files[i]));
}
static void CopyPhysXLinux()
{
   Str files[]=
   {
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/linux64/libPhysX3_x64.so",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/linux64/libPhysX3Common_x64.so",
      ThirdPartyLibsPath+"PhysX/PhysX/Bin/linux64/libPhysX3Cooking_x64.so",
      ThirdPartyLibsPath+"PhysX/PxShared/bin/linux64/libPxFoundation_x64.so",
   };
   if(PHYSX_DLL)FREPA(files)Copy(files[i], EditorPath+"Bin/"+GetBase(files[i]));
   else         FREPA(files)Del (          EditorPath+"Bin/"+GetBase(files[i]));
}
static void CompilePhysXWindows64() {Str path=ThirdPartyLibsPath+"PhysX/", projects=path+"PhysX/Source/compiler/vc14win64/"; if(PatchPhysXProject(projects) && PatchPhysXProject(path+"PxShared/src/compiler/vc14win64"))CompileVS(projects+"PhysX.sln", "Release", S, CopyPhysXWindows64);}
static void CompilePhysXWindows32() {Str path=ThirdPartyLibsPath+"PhysX/", projects=path+"PhysX/Source/compiler/vc14win32/"; if(PatchPhysXProject(projects) && PatchPhysXProject(path+"PxShared/src/compiler/vc14win32"))CompileVS(projects+"PhysX.sln", "Release", S, CopyPhysXWindows32);}
TaskBase CompilePhysXWindowsTB[]=
{
   {null, null, CompilePhysXWindows64},
   {null, null, CompilePhysXWindows32},
};
static Bool PhysXCheck()
{
   Str path=ThirdPartyLibsPath+"PhysX/", sub=path+"PhysX-3.4-master/", physx="PhysX", physx_ver="PhysX_3.4", shared="PxShared";
   if(FExistSystem(sub))
   {
      if(FExistSystem(sub+physx_ver))
      {
         FDelDirs(sub+physx_ver+"/Media");
       //FDelDirs(sub+physx_ver+"/Samples");
       //FDelDirs(sub+physx_ver+"/Snippets");

         if(FExistSystem(path+physx) && !FDelDirs(path+physx))return ErrorDel(path+physx);
         if(!FRename(sub+physx_ver, path+physx))return ErrorMove(sub+physx_ver, path+physx);
      }
      if(FExistSystem(sub+shared))
      {
         if(FExistSystem(path+shared) && !FDelDirs(path+shared))return ErrorDel(path+shared);
         if(!FRename(sub+shared, path+shared))return ErrorMove(sub+shared, path+shared);
      }
      if(FExistSystem(sub+"Externals/nvToolsExt"))
      {
         FCreateDirs(path+"Externals");
         if(!FRename(sub+"Externals/nvToolsExt", path+"Externals/nvToolsExt"))return ErrorMove(sub+"Externals/nvToolsExt", path+"Externals/nvToolsExt");
      }
      if(!FDelDirs(sub))return ErrorDel(sub);
   }
   if(FExistSystem(path+physx_ver))
   {
      if(FExistSystem(path+physx) && !FDelDirs(path+physx))return ErrorDel(path+physx);
      if(!FRename(path+physx_ver, path+physx))return ErrorMove(path+physx_ver, path+physx);
   }
   if(!FExistSystem(path+physx))return ErrorRead(path+physx);

   // FIXME remove these in the future
   FileText f;
   if(f.read(path+"PhysX/Include/common/PxPhysXCommonConfig.h"))
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "\n//#error Visual studio 2017 is not supported because of a compiler bug, support will be enabled once a fix is out.",
                     "\n#error Visual studio 2017 is not supported because of a compiler bug, support will be enabled once a fix is out."); // revert back the change made by older version of Esenthel Builder
      s=Replace(s, "\n#error Visual studio 2017 is not supported because of a compiler bug, support will be enabled once a fix is out.",
                   "\n#error This version of Visual Studio is not supported because of a compiler bug, you will have to install at least VS 2017 version 15.5, at the time of writing this, it can be downloaded only from the Preview link - https://www.visualstudio.com/vs/preview/");
      s=Replace(s, "#if (PX_VC == 15) && PX_WINDOWS", "#if PX_WINDOWS && _MSC_VER<1912"); // replace check for VS 2017 with a check for _MSC_VER (1912 = VS 2017 15.5)
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Include/common/PxPhysXCommonConfig.h"))return ErrorWrite(path+"PhysX/Include/common/PxPhysXCommonConfig.h");
      }
   }
   if(f.read(path+"PhysX/Source/PhysX/src/device/windows/PhysXIndicatorWindows.cpp")) // https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/50
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "\n#include <VersionHelpers.h>", "\n#undef _MSC_VER//#include <VersionHelpers.h>"); // this header is not available when compiling for WinXP support
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Source/PhysX/src/device/windows/PhysXIndicatorWindows.cpp"))return ErrorWrite(path+"PhysX/Source/PhysX/src/device/windows/PhysXIndicatorWindows.cpp");
      }
   }
   if(f.read(path+"PhysX/Source/LowLevelCloth/src/neon/NeonSolverKernel.cpp")) // https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/45
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, ", PxProfileZone", ", profile::PxProfileZone");
      s=Replace(s, "factory, profileZone)", "factory)");
      s=Replace(s, "__ARM_NEON__", "__ARM_NEON");
      s=Replace(s, "\n#error", "\n//#error");
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Source/LowLevelCloth/src/neon/NeonSolverKernel.cpp"))return ErrorWrite(path+"PhysX/Source/LowLevelCloth/src/neon/NeonSolverKernel.cpp");
      }
   }
   if(f.read(path+"PhysX/Source/LowLevelCloth/src/neon/NeonCollision.cpp")) // https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/45
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "__ARM_NEON__", "__ARM_NEON");
      s=Replace(s, "\n#error", "\n//#error");
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Source/LowLevelCloth/src/neon/NeonCollision.cpp"))return ErrorWrite(path+"PhysX/Source/LowLevelCloth/src/neon/NeonCollision.cpp");
      }
   }
   if(f.read(path+"PhysX/Source/LowLevelCloth/src/neon/NeonSelfCollision.cpp")) // https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/45
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "__ARM_NEON__", "__ARM_NEON");
      s=Replace(s, "\n#error", "\n//#error");
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Source/LowLevelCloth/src/neon/NeonSelfCollision.cpp"))return ErrorWrite(path+"PhysX/Source/LowLevelCloth/src/neon/NeonSelfCollision.cpp");
      }
   }
   if(f.read(path+"PhysX/Source/PhysXCooking/src/convex/ConvexMeshBuilder.cpp")) // https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/49
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "\t*dest++ = trig16[1];", "\t//*dest++ = trig16[1];");
      s=Replace(s, "\t*dest++ = trig16[2];", "\t//*dest++ = trig16[2];");
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PhysX/Source/PhysXCooking/src/convex/ConvexMeshBuilder.cpp"))return ErrorWrite(path+"PhysX/Source/PhysXCooking/src/convex/ConvexMeshBuilder.cpp");
      }
   }
   if(f.read(path+"PxShared/src/foundation/include/windows/PsWindowsIntrinsics.h")) // FIXME report this
   {
      Str so=f.getAll(), s=so;
      s=Replace(s, "\n#include <mmintrin.h>", "\n//#include <mmintrin.h>");
      if(so!=s)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, path+"PxShared/src/foundation/include/windows/PsWindowsIntrinsics.h"))return ErrorWrite(path+"PxShared/src/foundation/include/windows/PsWindowsIntrinsics.h");
      }
   }
   
   // static lib
   Str files[]=
   {
      path+"PhysX/Include/cooking/Pxc.h",
      path+"PhysX/Include/common/PxPhysXCommonConfig.h",
      path+"PxShared/include/foundation/PxPreprocessor.h",
   };
   FREPA(files)if(f.read(files[i]))
   {
      Str s=f.getAll(); Int l=s.length();
   #if PHYSX_DLL
      s=Replace(s, " //__declspec(dllexport)", " __declspec(dllexport)");
      s=Replace(s, " //__declspec(dllimport)", " __declspec(dllimport)");
   #else
      s=Replace(s, " __declspec(dllexport)", " //__declspec(dllexport)");
      s=Replace(s, " __declspec(dllimport)", " //__declspec(dllimport)");
   #endif
      if(s.length()!=l)
      {
         f.writeMem(f.encoding()).putText(s);
         if(!SafeOverwrite(f, files[i]))return ErrorWrite(files[i]);
      }
   }

   return true;
}
void PhysXCompileWindows(Ptr) {if(PhysXCheck())FREPAO(CompilePhysXWindowsTB).queue();}

static void PhysXCompileMac1() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_osx64/PhysX.xcodeproj", "release", "PhysX");}
static void PhysXCompileMac2() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_osx64/PhysX.xcodeproj", "release", "PhysXCooking");}
static void PhysXCompileMac3() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_osx64/PhysX.xcodeproj", "release", "PhysXExtensions");}
static void PhysXCompileMac4() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_osx64/PhysX.xcodeproj", "release", "PhysXVehicle");}
TaskBase PhysXCompileMacTB[]=
{
   {null, null, PhysXCompileMac1},
   {null, null, PhysXCompileMac2},
   {null, null, PhysXCompileMac3},
   {null, null, PhysXCompileMac4},
};
static void PhysXCompileiOS1() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_ios64/PhysX.xcodeproj", "release", "PhysX");}
static void PhysXCompileiOS2() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_ios64/PhysX.xcodeproj", "release", "PhysXCooking");}
static void PhysXCompileiOS3() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_ios64/PhysX.xcodeproj", "release", "PhysXExtensions");}
static void PhysXCompileiOS4() {CompileXcode(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/xcode_ios64/PhysX.xcodeproj", "release", "PhysXVehicle");}
TaskBase PhysXCompileiOSTB[]=
{
   {null, null, PhysXCompileiOS1},
   {null, null, PhysXCompileiOS2},
   {null, null, PhysXCompileiOS3},
   {null, null, PhysXCompileiOS4},
};
void PhysXCompileMac(Ptr) {if(PhysXCheck())FREPAO(PhysXCompileMacTB).queue();}
void PhysXCompileiOS(Ptr) {if(PhysXCheck())FREPAO(PhysXCompileiOSTB).queue();}

static Str PhysXAndroidFiles, PhysXAndroidFolders;
static FILE_LIST_MODE ListPhysXAndroidFiles(C FileFind &ff, Ptr user)
{
   if(ff.type==FSTD_DIR)
   {
      if(ff.name=="Windows" // skip files inside windows folders (they're meant only for Windows OS)
      || ff.name=="Snippets"
      || ff.name=="Samples"
      || ff.name=="avx"
      || ff.name=="PhysXCharacterKinematic"
      || Ends(ff.pathName().replace('\\', '/'), "/pvd/src") // skip PVD as it's not used
      )return FILE_LIST_SKIP;
      
    //if(FileFind(ff.pathName(), "cpp")())PhysXAndroidFiles  +=S+" \\\n\t"  +UnixPath(GetRelativePath(ThirdPartyLibsPath+"PhysX/Android", ff.pathName()))+"/*.cpp"; this doesn't work
      if(FileFind(ff.pathName(), "h"  )()
      || ff.name=="include"              )PhysXAndroidFolders+=S+" \\\n\t-I"+UnixPath(GetRelativePath(ThirdPartyLibsPath+"PhysX/Android", ff.pathName()));
   }else
   if(ff.type==FSTD_FILE && GetExt(ff.name)=="cpp"
   && !Contains(SkipStartPath(ff.pathName(), ThirdPartyLibsPath), "serialization", false, true)
   )PhysXAndroidFiles+=S+" \\\n\t"+UnixPath(GetRelativePath(ThirdPartyLibsPath+"PhysX/Android/jni", ff.pathName()));
   return FILE_LIST_CONTINUE;
}
static void PhysXCompileAndroid()
{
   Str ndk_build=NDKBuildPath();
   if(!ndk_build.is()){Options.show(); Gui.msgBox("Error", "Android NDK Path unknown");}else
   {
      PhysXAndroidFiles  .clear();
      PhysXAndroidFolders.clear();
      FList(ThirdPartyLibsPath+"PhysX/PhysX"   , ListPhysXAndroidFiles);
      FList(ThirdPartyLibsPath+"PhysX/PxShared", ListPhysXAndroidFiles);
      Str s=S+
"LOCAL_PATH := $(call my-dir)\n"
"include $(CLEAR_VARS)\n"
"LOCAL_MODULE       := PhysX\n"
"LOCAL_CPP_FEATURES := # rtti exceptions\n"
"LOCAL_ARM_NEON     := true # force NEON usage for all files\n"
"\n"
"LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer -Wno-invalid-offsetof -Wno-invalid-noreturn -DPX_PHYSX_STATIC_LIB -DPX_COOKING -DPX_SUPPORT_PVD=0 -DNDEBUG"+PhysXAndroidFolders+"\n"
"\n"
//"LOCAL_SRC_FILES := $(wildcard"+PhysXAndroidFiles+" \\\n)\n" this doesn't work
"LOCAL_SRC_FILES :="+PhysXAndroidFiles+"\n"
"\n"
"include $(BUILD_STATIC_LIBRARY)\n";
      FileText f; f.writeMem(ANSI).putText(s); // have to use ANSI because UTF8 for some reason fails
      if(!OverwriteOnChange(f, ThirdPartyLibsPath+"PhysX/Android/jni/Android.mk")){ErrorWrite(ThirdPartyLibsPath+"PhysX/Android/jni/Android.mk"); return;}
      Int build_threads_num=Cpu.threads();
      build_threads.queue(build_requests.New().set(ndk_build, S+"-j"+build_threads_num+" -C \""+ThirdPartyLibsPath+"PhysX/Android"+"\""), BuildRun);
   }
}
TaskBase PhysXCompileAndroidTB[]=
{
   {null, null, PhysXCompileAndroid},
};
void PhysXCompileAndroid(Ptr) {if(PhysXCheck())FREPAO(PhysXCompileAndroidTB).queue();}

static void PhysXCompileLinux()
{
   for(FileFind ff(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/linux64", "mk"); ff(); )
   {
      FileText f; if(f.read(ff.pathName()))
      {
         Str so, s; f.getAll(so); s=so;
         s=Replace(s, " -Werror", S, true, true);
      #if !PHYSX_DLL
       //s=Replace(s, " -shared", S, true, true);
         s=Replace(s, ".so"  , ".a", true, true);
         Str name=GetExtNot(SkipStart(ff.name, "Makefile.")); // "Makefile.LowLevel.mk" -> "LowLevel"
         s=Replace(s, S+"$(CXX) -shared $("+name+"_release_obj) $("+name+"_release_lflags) -lc -o $@", S+"@$(AR) rcs $("+name+"_release_bin) $("+name+"_release_obj)");
      #endif
         if(s!=so)
         {
            f.writeMem(f.encoding()).putText(s);
            if(!SafeOverwrite(f, ff.pathName())){ErrorWrite(ff.pathName()); Tasks.clear();}
         }
      }
   }
   build_threads.queue(build_requests.New().set("make", LinuxBuildParams(ThirdPartyLibsPath+"PhysX/PhysX/Source/compiler/linux64", S)+" release").set(CopyPhysXLinux), BuildRun);
}
TaskBase PhysXCompileLinuxTB[]=
{
   {null, null, PhysXCompileLinux},
};
void PhysXCompileLinux(Ptr) {if(PhysXCheck())FREPAO(PhysXCompileLinuxTB).queue();}
void PhysXCompileWeb(Ptr)
{
   // FIXME
   if(PhysXCheck()){}
}
/******************************************************************************/
#define ANDROID_DEFAULT WINDOWS
#define     WEB_DEFAULT false

TaskBase TaskBases[]=
{
#if WINDOWS
   {"Compile Windows 64 DX11"        , "Compile the Engine in Release mode for Windows 64-bit DX10+ only"              , CompileEngineWindows64DX11     , false},
   {"Compile Windows All"            , "Compile the Engine in Release mode for all Windows targets"                    , CompileEngineWindows           , true , true},
   {"Compile Windows 64 DX11 (Debug)", "Compile the Engine in Debug mode for Windows 64-bit DX10+ only"                , CompileEngineDebugWindows64DX11, false},
   {"Compile Windows All (Debug)"    , "Compile the Engine in Debug mode for all Windows targets"                      , CompileEngineDebugWindows      , false, true},
#elif APPLE
   {"Clean Apple"               , "Clean temporary files generated during Engine compilation for Apple platforms"       ,   CleanEngineApple        , false},
   {"Compile Mac"               , "Compile the Engine in Release mode for Mac only"                                     , CompileEngineMac          , false},
   {"Compile iOS"               , "Compile the Engine in Release mode for iOS only"                                     , CompileEngineiOS          , false},
   {"Compile Apple"             , "Compile the Engine in Release mode for all Apple platforms (Mac, iOS, iOS Simulator)", CompileEngineApple        , true , true},
   {"Compile Mac (Debug)"       , "Compile the Engine in Debug mode for Mac only"                                       , CompileEngineDebugMac     , false},
   {"Compile iOS (Debug)"       , "Compile the Engine in Debug mode for iOS only"                                       , CompileEngineDebugiOS     , false},
   {"Compile Apple (Debug)"     , "Compile the Engine in Debug mode for all Apple platforms (Mac, iOS, iOS Simulator)"  , CompileEngineDebugApple   , false, true},
#elif LINUX
   {"Clean Linux"               , "Clean temporary files generated during Engine/Editor compilation for Linux platform",   CleanLinux              , false},
   {"Compile Linux"             , "Compile the Engine in Release mode for Linux"                                       , CompileEngineLinux        , true },
   {"Make Linux Libs"           , "Make the Engine Linux Lib from the compilation result to the Editor Bin folder"     , LinuxLibs                 , true },
#endif
   {"Clean Android"             , "Clean temporary files generated during Engine compilation for Android"              ,   CleanEngineAndroid      , false},
   {"Compile Android"           , "Compile the Engine in Release mode for Android"                                     , CompileEngineAndroid      , ANDROID_DEFAULT},
   {"Make Android Libs"         , "Make the Engine Android Libs from the compilation result to the Editor Bin folder"  , AndroidLibs               , ANDROID_DEFAULT},
#if WINDOWS
   {"Clean Web"                 , "Clean temporary files generated during Engine compilation for the Web"              ,   CleanEngineWeb          , false},
   {"Compile Web"               , "Compile the Engine for Web"                                                         , CompileEngineWeb          , WEB_DEFAULT},
   {"Make Web Libs"             , "Make the Engine Web Lib from the compilation result to the Editor Bin folder"       , WebLibs                   , WEB_DEFAULT},
#endif
   {"Copy Headers"              , "Copy cleaned Engine Headers from the Engine folder to the Editor folder.\nCleaning removes all 'EE_PRIVATE' sections from the headers."                                                                    , Headers       , true },
   {"Create \"Code Editor.dat\"", "Create \"Code Editor.dat\" file needed for Code Editor in the Engine's Editor.\nThis data is generated from the Engine headers in the Editor Bin folder, which are generated in the \"Copy Headers\" step.", CodeEditorData, true },
   {"Create \"Engine.pak\""     , "Create \"Engine.pak\" file from the \"Data\" folder into the Editor Bin folder"       , EnginePak               , true },
   {"Create \"Editor.pak\""     , "Create \"Editor.pak\" file from the \"Editor Data\" folder into the Editor Bin folder", EditorPak               , true },
#if WINDOWS
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorWindows64DX11, false},
#elif APPLE
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorMac          , false},
#elif LINUX
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorLinux        , false},
#endif
};
Memc<TaskBase> CustomTasks;
/******************************************************************************/
void Resize(Flt=0, Flt=0)
{
   D.scale(D.screenH()/Flt(D.resH())*(1000./1080));
   region.rect(Rect(-D.w()+0.53, -D.h(), D.w(), D.h()));
   clear.rect(Rect_RU(region.rect().ru()-Vec2(region.slidebarSize(), 0), 0.2, 0.05));
   copy .rect(Rect_RU(clear.rect().lu(), 0.2, 0.05));

   Flt x=-D.w()+0.02, y=D.h()-0.10, h=0.067;
   OptionsButton.rect(Rect_L(x+0.06, y, 0.45, 0.05)); y-=h;
   y-=0.02;
   FREPA(Tasks){Tasks[i].pos(x, y); y-=h;}
   y-=0.02;

     All.rect(Rect_L(x     , y, 0.05, 0.05));
   DoSel.rect(Rect_L(x+0.06, y, 0.45, 0.05));
}
/******************************************************************************/
void InitPre()
{
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE|APP_WORK_IN_BACKGROUND|APP_FULL_TOGGLE;
   App.x=1;
   App.y=1;
#if DEBUG
   App.flag|=APP_BREAKPOINT_ON_ERROR|APP_MEM_LEAKS|APP_CALLSTACK_ON_ERROR;
#endif
   D.screen_changed=Resize;
   Flt scale=D.screenH()/1080.0f;
   D.mode(800*scale, 630*scale);
   App.name("Esenthel Builder");

   for(Str path=GetPath(App.exe()); ; path=GetPath(path))
   {
      if(!path.is())Exit("Can't find Esenthel Root Path");
      if(FExistSystem(path+"/Data")
      && FExistSystem(path+"/Editor Data")
      && FExistSystem(path+"/Editor")
      && FExistSystem(path+"/Engine"))
      {
         EsenthelPath=path.tailSlash(true);
         if(!FullPath(    EngineDataPath))    EngineDataPath=EsenthelPath+    EngineDataPath;
         if(!FullPath(    EditorDataPath))    EditorDataPath=EsenthelPath+    EditorDataPath;
         if(!FullPath(        EnginePath))        EnginePath=EsenthelPath+        EnginePath;
         if(!FullPath(        EditorPath))        EditorPath=EsenthelPath+        EditorPath;
         if(!FullPath(  EditorSourcePath))  EditorSourcePath=EsenthelPath+  EditorSourcePath;
         if(!FullPath(ThirdPartyLibsPath))ThirdPartyLibsPath=EsenthelPath+ThirdPartyLibsPath;
         break;
      }
   }

   DataPath(EngineDataPath);
}
void SetPaths()
{
   // setup VS path if not specified
   if(!VSPath.is())
   {
      Memc<VisualStudioInstallation> installs; GetVisualStudioInstallations(installs); REPA(installs) // go from the end to try the latest version first
      {
       C VisualStudioInstallation &install=installs[i]; if(CheckVisualStudio(install.ver))if(DevEnvPath(install.path).is())
         {
            VSPath=install.path; break;
         }
      }
   }

   // setup Android NDK path if not specified
   if(!AndroidNDKPath.is())
   {
      Str AndroidSDKPath=GetPath(GetRegStr(RKG_LOCAL_MACHINE, "Software/Android SDK Tools/Path"));
      if( AndroidSDKPath.is())for(FileFind ff(AndroidSDKPath); ff(); )
      {
         Str ndk_path=ff.pathName(); if(NDKBuildPath(ndk_path).is())
         {
            AndroidNDKPath=ndk_path; break;
         }
      }
   }
}
Bool Init()
{
   SetPaths();
   Options.create();
   Gui+=region.create();
   ListColumn lc[]=
   {
      ListColumn(DATA_STR, 0, SIZE(Str), LCW_MAX_DATA_PARENT, u"Output"),
   };
   region+=list.create(lc, Elms(lc)); list.elmHeight(0.037).textSize(0.037); FlagDisable(list.flag, LIST_SORTABLE); region.slidebar[0].scrollOptions(0.75f); region.slidebar[1].scrollOptions(0.75f);
   Gui+=copy .create("Copy" ).func(CopyOutput);
   Gui+=clear.create("Clear").func(Clear);

   Gui+=OptionsButton.create("Options").func(OptionsToggle);
#ifdef CUSTOM_TASKS
   CUSTOM_TASKS
#endif
   FREPA(TaskBases  )Tasks.New().create(TaskBases  [i]);
   FREPA(CustomTasks)Tasks.New().create(CustomTasks[i]);
   Gui+=All  .create().func(ToggleAll).set(true, QUIET);
   Gui+=DoSel.create("Do Selected").func(DoSelected).desc("Execute all selected tasks in order");

   Resize();
   build_threads.create(true, Cpu.threads(), 0, "BuildThreads");
   return true;
}
/******************************************************************************/
void Shut()
{
   Options.save();
   shut=true;
   build_threads.del();
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   if(!App.active())Time.wait(1);

   if(!build_threads.queued() && Todo.elms())
   {
      App.stayAwake(AWAKE_SYSTEM); // don't sleep when building
      done++; // start with small progress
      WindowSetProgress(Flt(done)/(Todo.elms()+done));
      if(TaskBase *t=Todo.popFirst())t->call();
   }
   if(done && !build_threads.queued() && !Todo.elms()) // finished all tasks
   {
      done=0;
      WindowSetNormal();
      WindowFlash    ();
      App.stayAwake  (AWAKE_OFF); // allow sleeping
   }

   if(data_new.elms())
   {
      Bool at_end=region.slidebar[1].wantedAtEnd(0.02f);
      SyncLocker locker(lock);
      FREPA(data_new)Swap(data.New(), data_new[i]);
      data_new.clear();
      list.setData(data);

      if(at_end)region.scrollEndY();
   }

   Gui.update();

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   TextStyleParams ts; ts.size=0.055; ts.align.set(1, -1); D.text(ts, -D.w()+0.01, D.h()-0.01, S+"Busy:"+build_threads.queued()+", Queued:"+Todo.elms());
   Gui.draw();
}
/******************************************************************************/
