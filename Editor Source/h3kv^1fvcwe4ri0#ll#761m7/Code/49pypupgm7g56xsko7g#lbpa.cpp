/******************************************************************************

   Publishing is best done in separate state so:
      -we don't allow editing   project elements during paking
      -we don't allow receiving project elements from the server during paking

   Windows:
      Engine.pak
      Project.pak
      EngineEmbed.pak (for engine data embedded into EXE, used when 'appEmbedEngineData')
      App.pak         (for app specific data embedded into EXE, used always             )

   Android:
      Engine.pak
      Project.pak (will contain only app specific data when "!appPublishProjData")

/******************************************************************************/
class ImageGenerate
{
   Str      src_mtrl, dest_base_0;
   DateTime time;

   void set(C Str &src_mtrl, C Str &dest_base_0, C DateTime &time) {T.src_mtrl=src_mtrl; T.dest_base_0=dest_base_0; T.time=time;}

   void process()
   {
      Image       base_0;
      MaterialPtr mtrl=src_mtrl;
      if(mtrl && MergeBaseTextures(base_0, *mtrl)){File f; if(base_0.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest_base_0, &time);}}
   }
}
class ImageConvert
{
   enum FAMILY
   {
      ELM_IMAGE      ,
      ELM_IMAGE_ATLAS,
      ELM_FONT       ,
      ELM_PANEL_IMAGE,
      IMAGE          ,
   }
   bool       pow2     =false,
              clamp    =true ,
              alpha_lum=false,
              has_color=true ,
              has_alpha=true ,
           ignore_alpha=false, 
            mtrl_base_1=false;
   byte        downsize=    0;
   int         mip_maps=   -1,
               max_size=    0;
   VecI2           size=    0;
   FAMILY      family  =ELM_IMAGE;
   Str        src, dest       ; // src and dest files
   DateTime   time            ; // date time to set after converting
   int        type=-1, mode=-1;

   static SyncLock Lock;

   void set(C Str &src, C Str &dest, C DateTime &time, int type, bool ignore_alpha, bool clamp, bool mtrl_base_1=false, byte downsize=0, int max_size=0)
   {
      T.family=IMAGE; T.src=src; T.dest=dest; T.time=time; T.type=type; T.ignore_alpha=ignore_alpha; T.clamp=clamp; T.mtrl_base_1=mtrl_base_1; T.downsize=downsize; T.max_size=max_size;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImage &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE; T.src=src; T.dest=dest; T.time=time; T.pow2=data.pow2(); T.mip_maps=(data.mipMaps() ? 0 : 1); T.alpha_lum=data.alphaLum(); T.has_color=data.hasColor(); T.has_alpha=data.hasAlpha3(); T.ignore_alpha=data.ignoreAlpha(); T.type=type; T.mode=data.mode; T.size=data.size;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmIcon &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE; T.src=src; T.dest=dest; T.time=time; T.pow2=false; T.has_color=data.hasColor(); T.has_alpha=data.hasAlpha(); T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImageAtlas &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE_ATLAS; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmFont &data, IMAGE_TYPE type)
   {
      T.family=ELM_FONT; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmPanelImage &data, IMAGE_TYPE type)
   {
      T.family=ELM_PANEL_IMAGE; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }

   static bool SkipOptimize(int &type, DateTime &time) // skip formats which are slow to convert
   {
      if(PublishSkipOptimize())if(type==IMAGE_BC7 || type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4 || type==IMAGE_ETC1 || type==IMAGE_ETC2 || type==IMAGE_ETC2_A1 || type==IMAGE_ETC2_A8){type=-1; time.decDay(); return true;} // use default type and set previous date, so the file will be regenerated next time
      return false;
   }

   void process(C bool *stop=null)C
   {
      DateTime time=T.time; if(!time.valid())time=FileInfo(src).modify_time_utc; // 'time' could've been empty if the file didn't exist yet at the moment of setting up this object (this can happen for dynamically generated textures)
      int type=T.type; bool skip=SkipOptimize(type, time);
      SyncLockerEx locker(Lock, type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4); // PVRTC texture compression is already multi-threaded and uses a lot of memory, so allow only one at a time
      if(stop && *stop)return;
      skip|=SkipOptimize(type, time); // call this again after the 'locker' got unlocked
      PublishPVRTC pvrtc(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4);
      switch(family)
      {
         case ELM_IMAGE:
         {
            Image image; if(image.ImportTry(src))if(EditToGameImage(image, image, pow2, alpha_lum, ElmImage.COMPRESSED, mode, mip_maps, has_color, has_alpha, ignore_alpha, size, &type))
            {
               File f; if(image.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_IMAGE_ATLAS:
         {
            ImageAtlas atlas; if(atlas.load(src))
            {
               FREPA(atlas.images)
               {
                  Image &image=atlas.images[i];
                  VecI2   size=image.size(), old=size; if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4)size=CeilPow2(size.max()); // image must be square for PVRTC. Unlike for regular textures, for atlases it's better to use 'CeilPow2', to make sure we don't make textures much smaller than original.
                  if(image.copyTry(image, size.x, size.y, -1, type, mode, mip_maps, FILTER_BEST, clamp, true))
                  {
                  #if 0 // don't do any adjustments because we may want to detect image proportions based on 'trimmed_size' (Esenthel RTS does that)
                     if(size!=old) // if size is different then adjust the parts
                        REPAD(p, atlas.parts) // iterate all parts
                     {
                        ImageAtlas.Part &part=atlas.parts[p];
                        if(part.image_index==i) // if this part is stored in converted image
                        {
                           part.original_size=Round(Vec2(part.original_size*size)/old);
                           part. trimmed_size=Round(Vec2(part. trimmed_size*size)/old);
                           part. trim_pos    =Round(Vec2(part. trim_pos    *size)/old);
                           part.center_offset.set(part.trim_pos.x-part.original_size.x*0.5, -part.trim_pos.y+part.original_size.y*0.5);
                        }
                     }
                  #endif
                  }
               }
               File f; if(atlas.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_FONT:
         {
            Font font; if(font.load(src))
            {
               font.imageType(IMAGE_TYPE(type));
               File f; if(font.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_PANEL_IMAGE:
         {
            PanelImage panel_image; if(panel_image.load(src))
            {
               VecI2 size=panel_image.image.size(); if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4)size=CeilPow2(size.max()); // image must be square for PVRTC. Unlike for regular textures, for panel images it's better to use 'CeilPow2', to make sure we don't make textures much smaller than original.
               panel_image.image.copyTry(panel_image.image, size.x, size.y, -1, type, -1, -1, FILTER_BEST, true, true);
               File f; if(panel_image.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case IMAGE:
         {
            if(skip) // just copy
            {
               locker.on(); // when just copying then limit to only one thread
               File f; if(f.readTry(src))SafeOverwrite(f, dest, &time);
            }else
            if(C ImagePtr &image=src)
            {
               Image temp, *s=image();
               VecI  size=s.size3(); if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4)size.xy=NearestPow2(size.xy.avgI()); // image must be square for PVRTC
               if(downsize) // downsize
               {
                  size.x=Max(1, size.x>>downsize);
                  size.y=Max(1, size.y>>downsize);
                  size.z=Max(1, size.z>>downsize);
               }
               if(max_size>0) // limit size
               {
                  MIN(size.x, max_size);
                  MIN(size.y, max_size);
                  MIN(size.z, max_size);
               }
               int mip_maps=T.mip_maps, mode=T.mode;
               if(ignore_alpha && NeedFixAlpha(*s, IMAGE_TYPE(type))) // if we won't need alpha
               {
                  if(mip_maps<0)mip_maps=((s.mipMaps()==1) ? 1 : 0); // source will have now only one mip-map so we can't use "-1", auto-detect instead
                  if(mode    <0)mode    =s.mode();                   // source will now be as IMAGE_SOFT      so we can't use "-1", auto-detect instead
                  if(type    <0)type    =s.type();                   // source will now be as IMAGE_R8G8B8    so we can't use "-1", auto-detect instead
                  if(s.copyTry(temp, -1, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT, 1))s=&temp;
               }
               if(s.copyTry(temp, size.x, size.y, size.z, type, mode, mip_maps, FILTER_BEST, clamp, false, false, mtrl_base_1))
               {
                  File f; if(temp.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
               }
            }
         }break;
      }
   }
}
/******************************************************************************/
class PublishResult : ClosableWindow
{
   TextNoTest text;
   Button     ok, size_stats;
   Str        path;

   static void OK       (PublishResult &pr) {pr.del();}
   static void SizeStats(PublishResult &pr) {pr.del(); .SizeStats.display(pr.path, Publish.cipher());}

   void display(C Str &text)
   {
      path=(PublishProjectDataPath.is() ? PublishProjectDataPath : PublishBinPath);

      Gui+=super.create(Rect_C(0, 0, 1, 0.34), "Publishing succeeded"); button[2].show();
      T+=ok        .create(Rect_D(clientWidth()*(PublishEsProj ? 2 : 1)/4, -clientHeight()+0.04, 0.3, 0.06), "OK"        ).func(OK       , T);
      T+=size_stats.create(Rect_D(clientWidth()*                     3 /4, -clientHeight()+0.04, 0.3, 0.06), "Size Stats").func(SizeStats, T).hidden(PublishEsProj);
      T+=T.text    .create(Rect(0.02, ok.rect().max.y, clientWidth()-0.02, 0), text);
      activate();
   }
}
class PublishPVRTC
{
   bool on;

   PublishPVRTC(bool on) : on(on) {if(on)AtomicInc(PublishPVRTCUse);}
  ~PublishPVRTC(       )          {if(on)AtomicDec(PublishPVRTCUse);}
}
enum PUBLISH_STAGE
{
   PUBLISH_MTRL_SIMPLIFY,
   PUBLISH_TEX_OPTIMIZE ,
   PUBLISH_PUBLISH      ,
}
/******************************************************************************/
class PublishClass
{
   ProjectCipher cipher;
   PakProgress   progress;

   WindowIO      export_data;
   Edit.EXE_TYPE export_data_exe=Edit.EXE_EXE;

   static void ExportData(C Str &name, PublishClass &publish) {StartPublish(S, publish.export_data_exe, Edit.BUILD_PUBLISH, true, name);}

   void create()
   {
      export_data.create("pak", S, S, ExportData, ExportData, T);
   }
   void exportData(Edit.EXE_TYPE exe=Edit.EXE_EXE) {export_data_exe=exe; export_data.save();}
}
PublishClass Publish;

State               StatePublish(UpdatePublish, DrawPublish, InitPublish, ShutPublish);
Memb<PakFileData>   PublishFiles;
Memc<ImageGenerate> PublishGenerate;
Memc<ImageConvert>  PublishConvert;
Memc<Mems<byte>>    PublishFileData; // for file data allocated dynamically
SyncLock            PublishLock;
bool                PublishOk, PublishNoCompile, PublishOpenIDE, PublishDataAsPak, PublishDataOnly, PublishEsProj;
int                 PublishAreasLeft, PublishPVRTCUse;
PUBLISH_STAGE       PublishStage;
Str                 PublishPath,
                    PublishBinPath, // "Bin/" path (must include tail slash)
                    PublishProjectDataPath, // "Project.pak" path
                    PublishExePath, 
                    PublishErrorMessage;
Button              PublishSkipOptimize;
ComboBox            PublishPVRTCQuality;
Text                PublishPVRTCQualityText;
TextWhite           PublishPVRTCQualityTextStyle;
Edit.EXE_TYPE       PublishExeType  =Edit.EXE_EXE;
Edit.BUILD_MODE     PublishBuildMode=Edit.BUILD_BUILD;
PublishResult       PublishRes;
WindowIO            PublishEsProjIO;
/******************************************************************************/
bool PublishDataNeeded(Edit.EXE_TYPE exe, Edit.BUILD_MODE mode) {return exe==Edit.EXE_NEW || exe==Edit.EXE_APK || exe==Edit.EXE_IOS;}
/******************************************************************************/
void PublishDo()
{
   if(Demo)
   {
      Gui.msgBox(S, "Demo version doesn't allow publishing."); return;
      if(Proj.cipher || Proj.compress_type){Gui.msgBox(S, "Encryption and Compression are not available in the demo version"); return;}
   }
   CodeEdit.publish();
}
void PublishEsProjAs(C Str &path, ptr user)
{
   StartPublish(S, Edit.EXE_EXE, Edit.BUILD_PUBLISH, true, path, false, true);
}
bool StartPublish(C Str &exe_name, Edit.EXE_TYPE exe_type, Edit.BUILD_MODE build_mode, bool no_compile=false, C Str &custom_project_data_path=S, bool open_ide=false, bool es_proj=false)
{
   PublishRes.del();

   PublishExePath  =exe_name;
   PublishExeType  =exe_type;
   PublishBuildMode=build_mode;
   PublishNoCompile=no_compile;
   PublishOpenIDE  =open_ide;
   PublishEsProj   =es_proj;
   PublishDataOnly =(no_compile && custom_project_data_path.is());
   PublishPath           .clear();
   PublishBinPath        .clear();
   PublishProjectDataPath.clear();

   if(PublishEsProj)Publish.cipher.clear(); // EsenthelProject is never encrypted
   else             Publish.cipher.set(Proj);

   if(PublishEsProj)CodeEdit.saveChanges();

   if(PublishDataOnly || PublishEsProj) // data only
   {
      PublishPath           =custom_project_data_path; FCreateDirs(GetPath(PublishPath));
      PublishProjectDataPath=custom_project_data_path;
      PublishDataAsPak=true;
   }else // executable
   {
      bool physx_dll=(PHYSX_DLL && Physics.engine()==PHYS_ENGINE_PHYSX && CodeEdit.appPublishPhysxDll());
      if(exe_type==Edit.EXE_EXE || exe_type==Edit.EXE_DLL)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true); if(!FDelInside(PublishPath)){Gui.msgBox(S, S+"Can't delete \""+PublishPath+'"'); return false;}
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FExistSystem(PublishPath) && !FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData() || physx_dll) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;}
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            Memt<Str> files;
            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.add("Engine.pak");
            if(physx_dll)
            {
               cchar8 *physx_files32[]=
               {
                  "PhysX3_x86.dll",
                  "PhysX3Common_x86.dll",
                  "PhysX3Cooking_x86.dll",
                  "PxFoundation_x86.dll",
               };
               cchar8 *physx_files64[]=
               {
                  "PhysX3_x64.dll",
                  "PhysX3Common_x64.dll",
                  "PhysX3Cooking_x64.dll",
                  "PxFoundation_x64.dll",
               };
               if(CodeEdit.config32Bit())FREPA(physx_files32)files.add(physx_files32[i]);
               else                      FREPA(physx_files64)files.add(physx_files64[i]);
            }
            FREPA(files)if(!FCopy(BinPath().tailSlash(true)+files[i], PublishBinPath+GetBase(files[i]))){Gui.msgBox(S, S+"Can't copy \""+files[i]+'"'); return false;}
         }
         if(CodeEdit.appPublishSteamDll())
         {
            cchar8 *name=(CodeEdit.config32Bit() ? "steam_api.dll" : "steam_api64.dll");
            if(!FCopy(S+"Code/Windows/"+name, PublishPath+name)){Gui.msgBox(S, S+"Can't copy \""+name+'"'); return false;}
         }
         if(CodeEdit.appPublishOpenVRDll())
         {
            cchar8 *name=(CodeEdit.config32Bit() ? "openvr_api.32.dll" : "openvr_api.64.dll");
            if(!FCopy(S+"Code/Windows/"+name, PublishPath+"openvr_api.dll")){Gui.msgBox(S, S+"Can't copy \""+name+'"'); return false;}
         }
         if(PublishExePath.is())if(!FCopy(PublishExePath, PublishPath+GetBase(PublishExePath))){Gui.msgBox(S, S+"Can't copy \""+GetBase(PublishExePath)+'"'); return false;}
      }else
      if(exe_type==Edit.EXE_NEW)
      {
         PublishDataAsPak=true; // always set to true because files inside exe can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.windowsProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      if(exe_type==Edit.EXE_WEB)
      {
         if(build_mode==Edit.BUILD_PLAY)
         {
            PublishPath=GetPath(GetPath(PublishExePath)).tailSlash(true); // convert from "../Emscripten/Debug/App.*" to "../Emscripten/" so we can reuse the same "Engine.pak" and "Project.pak" for Debug/Release configurations
         }else
         {
            PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true); if(!FDelInside(PublishPath)){Gui.msgBox(S, S+"Can't delete \""+PublishPath+'"'); return false;}
            if(!FExistSystem(PublishPath) && !FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         }
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(1/*!CodeEdit.appEmbedEngineData()*/ || CodeEdit.appPublishProjData()) // if want to store something in "Bin" folder, embed engine data should be ignored (we always need engine.pak for web)
         {
            PublishBinPath=PublishPath/*+"Bin\\"*/; if(!FExistSystem(PublishBinPath) && !FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;}
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            Memt<Str> files;
            if(1/*!CodeEdit.appEmbedEngineData()*/ && PublishDataAsPak)files.add("Web/Engine.pak"); // embed engine data should be ignored (we always need Engine.pak for web)
            FREPA(files)if(!FCopy(BinPath().tailSlash(true)+files[i], PublishBinPath+GetBase(files[i]))){Gui.msgBox(S, S+"Can't copy \""+files[i]+'"'); return false;}
         }
         if(PublishExePath.is())
         {
            if(!FCopy(GetExtNot(PublishExePath)+".Esenthel.html", PublishPath+GetBaseNoExt(PublishExePath)+".html")){Gui.msgBox(S, S+"Can't copy \""+GetBaseNoExt(PublishExePath)+".html\""); return false;}
            if(!FCopy(GetExtNot(PublishExePath)+".js"           , PublishPath+GetBaseNoExt(PublishExePath)+".js"  )){Gui.msgBox(S, S+"Can't copy \""+GetBaseNoExt(PublishExePath)+".js\""  ); return false;}
            if(!FCopy(GetExtNot(PublishExePath)+".wasm"         , PublishPath+GetBaseNoExt(PublishExePath)+".wasm")){Gui.msgBox(S, S+"Can't copy \""+GetBaseNoExt(PublishExePath)+".wasm\""); return false;}
         }
      }else
      if(exe_type==Edit.EXE_MAC)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true); if(!FDelInside(PublishPath)){Gui.msgBox(S, S+"Can't delete \""+PublishPath+'"'); return false;}
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FExistSystem(PublishPath) && !FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData()) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;}
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            Memt<Str> files;
            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.add("Engine.pak");
            FREPA(files)if(!FCopy(BinPath().tailSlash(true)+files[i], PublishBinPath+files[i])){Gui.msgBox(S, S+"Can't copy \""+files[i]+'"'); return false;}
         }
         if(PublishExePath.is())if((FileInfoSystem(PublishExePath).type==FSTD_FILE) ? !FCopy(PublishExePath, PublishPath+GetBase(PublishExePath)) : !FCopyDir(PublishExePath, PublishPath+GetBase(PublishExePath))){Gui.msgBox(S, S+"Can't copy \""+GetBase(PublishExePath)+'"'); return false;}
      }else
      if(exe_type==Edit.EXE_LINUX)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true); if(!FDelInside(PublishPath)){Gui.msgBox(S, S+"Can't delete \""+PublishPath+'"'); return false;}
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FExistSystem(PublishPath) && !FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData() || physx_dll || CodeEdit.appPublishSteamDll() || CodeEdit.appPublishOpenVRDll()) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;}
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            Memt<Str> files;
            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.add("Engine.pak");
            if(physx_dll)
            {
               cchar8 *physx_files[]=
               {
                  "libPhysX3_x64.so",
                  "libPhysX3Common_x64.so",
                  "libPhysX3Cooking_x64.so",
                  "libPxFoundation_x64.so",
               };
               FREPA(physx_files)files.add(physx_files[i]);
            }
            if(CodeEdit.appPublishSteamDll ())files.add("libsteam_api.so");
            if(CodeEdit.appPublishOpenVRDll())files.add("libopenvr_api.so");
            FREPA(files)if(!FCopy(BinPath().tailSlash(true)+files[i], PublishBinPath+files[i])){Gui.msgBox(S, S+"Can't copy \""+files[i]+'"'); return false;}
         }
         if(PublishExePath.is())if((FileInfoSystem(PublishExePath).type==FSTD_FILE) ? !FCopy(PublishExePath, PublishPath+GetBase(PublishExePath)) : !FCopyDir(PublishExePath, PublishPath+GetBase(PublishExePath))){Gui.msgBox(S, S+"Can't copy \""+GetBase(PublishExePath)+'"'); return false;}
      }else
      if(exe_type==Edit.EXE_APK)
      {
         PublishDataAsPak=true; // always set to true because files inside APK (assets) can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.androidProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      if(exe_type==Edit.EXE_IOS)
      {
         PublishDataAsPak=true; // always set to true because files inside iOS APP can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.iOSProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else {Gui.msgBox(S, "Invalid application type."); return false;}
   }

   if(build_mode!=Edit.BUILD_PUBLISH) // if there's no need to wait for fully complete data (for example we want to play/debug game on mobile which requires PAK creation)
   {
      Proj.flush(); // flush what we've changed
      SetPublishFiles(PublishFiles, PublishGenerate, PublishConvert, PublishFileData); // detect files for packing
      if(!PublishGenerate.elms() && !PublishConvert.elms()) // if there are no elements to generate and convert
         if(PublishDataAsPak) // if we're creating a pak
            if(!PublishProjectDataPath.is() // if we don't want to create project data pak (no file)
            || FileInfoSystem(PublishProjectDataPath).modify_time_utc>CodeEdit.appEmbedSettingsTime() && PakEqual(PublishFiles, PublishProjectDataPath, Publish.cipher())){PublishSuccess(); return true;} // or if pak is similar to what we want then exit already
   }

   StatePublish.set(StateFadeTime);
   return true;
}
/******************************************************************************/
void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index)
{
   if(!Publish.progress.stop)
   {
      ThreadMayUseGPUData();
      generate.process();
      {SyncLocker locker(PublishLock); Publish.progress.progress+=1.0/PublishGenerate.elms();}
   }
}
void ImageConvertProcess(ImageConvert &convert, ptr user, int thread_index)
{
   if(!Publish.progress.stop)
   {
      ThreadMayUseGPUData();
      convert.process(&Publish.progress.stop);
      {SyncLocker locker(PublishLock); Publish.progress.progress+=1.0/PublishConvert.elms();}
   }
}
bool PublishFunc(Thread &thread)
{
   // generate
   PublishStage=PUBLISH_MTRL_SIMPLIFY; Publish.progress.progress=0; WorkerThreads.process1(PublishGenerate, ImageGenerateProcess);

   // convert
   PublishStage=PUBLISH_TEX_OPTIMIZE; Publish.progress.progress=0; WorkerThreads.process1(PublishConvert, ImageConvertProcess);

   // pak
   PublishStage=PUBLISH_PUBLISH; Publish.progress.progress=0;
   if(PublishDataAsPak)
   {
      if(!PublishProjectDataPath.is())PublishOk=true;else
                                      PublishOk=PakCreate(PublishFiles, PublishProjectDataPath, PAK_SET_HASH, Publish.cipher(), PublishEsProj ? EsenthelProjectCompression : Proj.compress_type, PublishEsProj ? EsenthelProjectCompressionLevel : Proj.compress_level, &PublishErrorMessage, &Publish.progress);
   }else
   {
      Memc<Str> dest_paths;
      FREPA(PublishFiles)
      {
         if(Publish.progress.stop)return false;
       C PakFileData &pfd=PublishFiles[i];
         if(pfd.type!=FSTD_DIR)
         {
            Str dest=PublishBinPath+pfd.name, dest_path=GetPath(dest);
            if(dest_paths.binaryInclude(dest_path, ComparePathCI))FCreateDirs(dest_path); // create path only once
            if(!FCopy(pfd.data.name, dest))
            {
               PublishErrorMessage="Error copying file.";
               if(!FExist(pfd.data.name))PublishErrorMessage.line()+="File not found.";
               PublishErrorMessage.line()+=S+"File: \""+pfd.name+'"';
               if(Elm *elm=Proj.findElm(GetFileNameID(pfd.name)))PublishErrorMessage.line()+=S+"Element: \""+Proj.elmFullName(elm)+'"';
               return false;
            }
         }
         Publish.progress.progress=flt(i)/PublishFiles.elms();
      }
      PublishOk=true;
   }
   return false;
}
/******************************************************************************/
class Texture
{
   UID id, // texture id
       src_tex_id=UIDZero; // if this is a dynamically generated texture, then 'src_tex_id' points to the original texture from which it was created
   bool  uses_alpha=false, // if uses alpha channel
            keep_hq=false, // if keep high quality and don't convert to low quality format
        mtrl_base_1=false, // if this is material base_1 texture
         regenerate=false; // if this texture needs to be regenerated
   byte        downsize=0; // blur mip map range

   Texture& downSize(int size) {MAX(downsize, size); return T;}

   static int CompareTex(C Texture &tex, C UID &tex_id) {return Compare(tex.id, tex_id);}
}
Texture* GetTexture(MemPtr<Texture> textures, C UID &tex_id)
{
   if(tex_id.valid())
   {
      int index; if(textures.binarySearch(tex_id, index, Texture.CompareTex))return &textures[index];
      Texture &tex=textures.NewAt(index); tex.id=tex_id;
      return  &tex;
   }
   return null;
}
void AddPublishFiles(Memt<Elm*> &elms, MemPtr<PakFileData> files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert)
{
   Memx<Texture> publish_texs; // textures to be published, need to use Memx because below pointers are stored

 C bool android=(PublishExeType==Edit.EXE_APK), iOS=(PublishExeType==Edit.EXE_IOS), web=(PublishExeType==Edit.EXE_WEB);

   // elements
   FREPA(elms)if(Elm *elm=elms[i])if(ElmPublish(elm.type))
   {
      if(elm.type==ELM_WORLD) // world
      {
         Str world_path=EncodeFileName(elm.id), world_edit_path_src, world_game_path_src;

         PakFileData &pfd=files.New(); pfd.name=world_path; pfd.type=FSTD_DIR; // world folder

         if(Proj.getWorldPaths(elm.id, world_edit_path_src, world_game_path_src))
            if(WorldVer *ver=Proj.worldVerGet(elm.id))
         {
            world_path.tailSlash(true);

            PakFileData &pfd=files.New(); // world settings
            pfd.name    =world_path         +"Settings";
            pfd.data.set(world_game_path_src+"Settings");

            Str     area_path=world_path         +"Area\\",
                src_area_path=world_game_path_src+"Area\\",
                waypoint_path=world_path         +"Waypoint\\",
            src_waypoint_path=world_game_path_src+"Waypoint\\";
            FREPA(ver.areas) // process in order to avoid re-sorting
            {
               Str src=src_area_path+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
               {
                  PakFileData &pfd=files.New(); // world area
                  pfd.name=area_path+ver.areas.lockedKey(i);
                  pfd.data.set(src);
               }
            }
            FREPA(ver.waypoints) // process in order to avoid re-sorting
            {
               Str file_id=EncodeFileName(ver.waypoints.lockedKey(i));
               if(FExist(src_waypoint_path+file_id)) // waypoint game file can be deleted when it was removed so we need to check if it exists
               {
                  PakFileData &pfd=files.New(); // world waypoint
                  pfd.name    =    waypoint_path+file_id;
                  pfd.data.set(src_waypoint_path+file_id);
               }
            }
         }
      }else
      if(elm.type==ELM_MINI_MAP) // mini map
      {
         Str mini_map_path=EncodeFileName(elm.id), mini_map_game_path_src, mini_map_formats_path;
         PakFileData &pfd=files.New(); pfd.name=mini_map_path; pfd.type=FSTD_DIR; // mini map folder
         if(ElmMiniMap *data=elm.miniMapData())
            if(MiniMapVer *ver=Proj.miniMapVerGet(elm.id))
         {
            mini_map_path.tailSlash(true);
            mini_map_game_path_src=Proj.gamePath(elm.id).tailSlash(true);

            PakFileData &pfd=files.New(); // mini map settings
            pfd.name    =mini_map_path         +"Settings";
            pfd.data.set(mini_map_game_path_src+"Settings");

            IMAGE_TYPE dest_type=IMAGE_NONE;
            if(android || iOS) // convert for mobile, desktop/web already has IMAGE_BC1 chosen
            {
               dest_type=(android ? (AndroidETC2 ? IMAGE_ETC2 : IMAGE_ETC1) : IMAGE_PVRTC1_4);
               mini_map_formats_path=Proj.formatPath(elm.id, FormatSuffix(dest_type));
               mini_map_formats_path.tailSlash(true);
               FCreateDirs(mini_map_formats_path);
            }

            FREPA(ver.images) // process in order to avoid re-sorting
            {
               PakFileData &pfd=files.New(); // mini map image
               pfd.name    =mini_map_path         +ver.images[i];
               pfd.data.set(mini_map_game_path_src+ver.images[i]);
               if(dest_type) // convert
               {
                  Str        src_name=pfd.data.name,
                            dest_name=mini_map_formats_path+ver.images[i];
                  pfd.data.set(dest_name); // adjust pak file path
                  FileInfo src_fi(src_name);
                  if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
                     convert.New().set(src_name, dest_name, src_fi.modify_time_utc, dest_type, true, true); // create new conversion
               }
            }
         }
      }else // regular files
      {
         PakFileData &pfd=files.New();
         pfd.name         =       EncodeFileName(elm.id);
         pfd.data.set(             Proj.gamePath(elm.id));
         pfd.compress_mode=(ElmPublishNoCompress(elm.type) ? COMPRESS_DISABLE : COMPRESS_ENABLE);

         if(elm.type==ELM_MTRL)if(ElmMaterial *data=elm.mtrlData()) // material
         {
            bool regenerate=false,
                 uses_tex_bump=data.usesTexBump(), 
                 uses_tex_glow=data.usesTexGlow();
            byte downsize  =((android || iOS) ? data.downsize_tex_mobile : 0);
            UID  base_0_tex=data.base_0_tex, src_tex=UIDZero,
                 base_1_tex=data.base_1_tex;

            // simplify material
            if(base_0_tex.valid() && base_1_tex.valid() && Proj.materialSimplify(PublishExeType))
            {
               uses_tex_bump=uses_tex_glow=false; // these textures are removed when merging

               // adjust base texture ID's
                  src_tex=base_0_tex; // mark it as dynamically generated texture
               base_0_tex=MergedBaseTexturesID(base_0_tex, base_1_tex);
               base_1_tex.zero();

               // make a copy of the material with adjusted textures
               Str src_name=pfd.data.name,
                  dest_name=Proj.formatPath(elm.id, FormatSuffixSimple());
               pfd.data.set(dest_name); // adjust pak file path
               FileInfo src_fi(src_name);
               if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               {
                  MaterialPtr mtrl=src_name; // use 'MaterialPtr' to access it from cache if available
                  Material    temp=*mtrl;
                  temp.base_0=Proj.texPath(base_0_tex); // the texture will be initially saved to 'texDynamicPath' however in PAK it will be created in 'texPath' so save the reference there
                  temp.base_1=null;
                  // adjust specular related parameters
                  {
                     flt  avg_specular=0.5;
                     Vec4 avg; if(mtrl->base_1)if(mtrl->base_1->stats(null, null, &avg))avg_specular=avg.z; // specular is packed in BLUE channel, #MaterialTextureChannelOrder
                     temp.specular*=avg_specular;
                     temp.reflect *=avg_specular;
                  }
                  if(temp.technique==MTECH_DEFAULT)temp.glow=0; // disable glow if it's possible that there was a glow map
                  File f; temp.save(f.writeMem(), Proj.game_path); f.pos(0); SafeOverwrite(f, dest_name, &src_fi.modify_time_utc);
               }

               // merge textures
               FileInfo src_base_0(Proj.texPath       (data.base_0_tex)); // get modify time of the original texture in case it was modified later (for example due to mip map blur)
               Str     dest_base_0=Proj.texDynamicPath(     base_0_tex) ; // get path of the merged texture
               if(Compare(src_base_0.modify_time_utc, FileInfoSystem(dest_base_0).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               {
                  generate.New().set(src_name, dest_base_0, src_base_0.modify_time_utc);
                  regenerate=true; // this tex will be regenerated
               }
            }

            // !! 'GetTexture' needs to be called always because it adds texture to publish list !!
            bool hq=(iOS && data.texQualityiOS());
            Texture *t0; if(        t0=GetTexture(publish_texs,          base_0_tex)){t0.downSize(downsize); if(ForceHQMtrlBase0 || hq)t0.keep_hq=true; t0.src_tex_id=src_tex; t0.regenerate|=regenerate;}
            Texture *t1; if(        t1=GetTexture(publish_texs,          base_1_tex)){t1.downSize(downsize); if(ForceHQMtrlBase1      )t1.keep_hq=true; t1.mtrl_base_1=true;}
                         if(Texture *t=GetTexture(publish_texs, data.    detail_tex)){t .downSize(downsize); if(ForceHQMtrlDetail     )t .keep_hq=true; if(!RemoveMtrlDetailBump)t.uses_alpha=true;} // Detail uses Alpha for bump unless it's removed
                         if(Texture *t=GetTexture(publish_texs, data.     macro_tex)) t .downSize(downsize); // doesn't use Alpha, 'GetTexture' needs to be called
                         if(Texture *t=GetTexture(publish_texs, data.     light_tex)) t .downSize(downsize); // doesn't use Alpha, 'GetTexture' needs to be called
                         if(Texture *t=GetTexture(publish_texs, data.reflection_tex)){}                      // doesn't use Alpha, 'GetTexture' needs to be called

            // check which base textures use Alpha Channel, #MaterialTextureChannelOrder
            if(t1) // having 'base_1' texture means that 'base_0' alpha channel is bump intensity and 'base_1' is alpha channel opacity
            {
               if(t0)if(                       uses_tex_bump)t0.uses_alpha=true; // Alpha used for bump
                     if(data.usesTexAlpha() || uses_tex_glow)t1.uses_alpha=true; // Alpha used for opacity/glow
            }else // 'base_1' is not present, meaning that 'base_0' alpha channel can contain opacity
            if(t0)
            {
               if(data.usesTexAlpha())t0.uses_alpha=true; // Alpha used for opacity
            }
         }

         if(elm.type==ELM_WATER_MTRL)if(ElmWaterMtrl *data=elm.waterMtrlData()) // water material
         {
            // !! 'GetTexture' needs to be called always because it adds texture to publish list !!
            Texture *t0; if(        t0=GetTexture(publish_texs, data.    base_0_tex)){if(ForceHQMtrlBase0)t0.keep_hq=true;} // doesn't use Alpha
            Texture *t1; if(        t1=GetTexture(publish_texs, data.    base_1_tex)){if(ForceHQMtrlBase1)t1.keep_hq=true; t1.mtrl_base_1=true;} // doesn't use Alpha
                         if(Texture *t=GetTexture(publish_texs, data.reflection_tex)){} // doesn't use Alpha, 'GetTexture' needs to be called

            // check which base textures use Alpha Channel, #MaterialTextureChannelOrder
            if(t1) // having 'base_1' texture means that 'base_0' alpha channel is bump intensity and 'base_1' is alpha channel opacity
            {
               if(t0)if(data.usesTexBump())t0.uses_alpha=true; // Alpha used for bump
            }
         }

         // try optimizing images for target platform
         if(elm.type==ELM_IMAGE) // image
            if(android || iOS || web) // desktop platform already has the best format chosen through 'EditToGameImage' and 'ImageProps'
               if(ElmImage *data=elm.imageData())
                  if(IMAGE_TYPE dest_type=(android ? data.androidType() : iOS ? data.iOSType() : data.webType())) // want to use custom type
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.editPath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from edit path to get better quality)
         }

         // try optimizing icons for target platform
         if(elm.type==ELM_ICON) // icon
            if(android || iOS || web) // desktop platform already has the best format chosen through 'EditToGameImage' and 'ImageProps'
               if(ElmIcon *data=elm.iconData())
                  if(IMAGE_TYPE dest_type=(android ? data.androidType(&Proj) : iOS ? data.iOSType(&Proj) : data.webType(&Proj))) // want to use custom type
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because icons have only game version)
         }

         // try optimizing atlases for target platform
         if(elm.type==ELM_IMAGE_ATLAS) // image atlas
            if(android || iOS || (web && !WebBC7)) // desktop platform already has the best format chosen during image atlas creation
               if(ElmImageAtlas *data=elm.imageAtlasData())
                  if(IMAGE_TYPE dest_type=(android ? IMAGE_ETC2_A8 : iOS ? IMAGE_PVRTC1_4 : IMAGE_BC3)) // we assume that atlas images contain transparency
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because image atlases have only game version)
         }

         // try optimizing fonts for target platform
         if(elm.type==ELM_FONT) // font
            if(android || iOS || (web && !WebBC7)) // desktop platform already has the best format chosen during font creation
               if(ElmFont *data=elm.fontData())
                  if(IMAGE_TYPE dest_type=((android || iOS) ? IMAGE_ETC2_A8 : IMAGE_BC3)) // for Android/iOS GL_ES 3.0+ use IMAGE_ETC2_A8, don't use IMAGE_PVRTC1_2/IMAGE_PVRTC1_4 because the quality is too low
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because fonts have only game version)
         }

         // try optimizing panel images for target platform
         if(elm.type==ELM_PANEL_IMAGE) // panel image
            if(android || iOS || (web && !WebBC7)) // desktop platform already has the best format chosen during image atlas creation
               if(ElmPanelImage *data=elm.panelImageData())
                  if(IMAGE_TYPE dest_type=(android ? IMAGE_ETC2_A8 : iOS ? IMAGE_PVRTC1_4 : IMAGE_BC3)) // we assume that atlas images contain transparency
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because image atlases have only game version)
         }
      }
   }

   // textures
   int tex_not_found=0;
   FREPA(publish_texs)
   {
    C Texture &tex=publish_texs[i]; const bool dynamic=tex.src_tex_id.valid();
      if(Proj.texs.binaryHas(tex.id, Compare) || dynamic)
      {
         PakFileData &pfd=files.New();
         pfd.name=S+"Tex/"+EncodeFileName(tex.id); // dest name
         pfd.data.set(dynamic ? Proj.texDynamicPath(tex.id) : Proj.texPath(tex.id)); // src name

         // change type
         int change_type=-1;
         if(android)change_type=(tex.uses_alpha ? IMAGE_ETC2_A8 : (AndroidETC2 || tex.keep_hq) ? IMAGE_ETC2     : IMAGE_ETC1    );else
         if(iOS    )change_type=(                                                 tex.keep_hq  ? IMAGE_PVRTC1_4 : IMAGE_PVRTC1_2);else
         if(web    )change_type=(WebBC7 ? ((tex.uses_alpha || tex.keep_hq) ?        -1 : IMAGE_BC1)      // texture could have alpha, however if we're not using it, then reduce to BC1 because it's only 4-bit per pixel
                                        :   tex.uses_alpha                 ? IMAGE_BC3 : IMAGE_BC1);else // if BC7 not supported for Web, then use BC3
       //if(!tex.uses_alpha && !tex.keep_hq )change_type=IMAGE_BC1;else // texture could have alpha, however if we're not using it, then reduce to BC1 because it's only 4-bit per pixel, actually don't do this because it would require calling 'ImageLoadHeader' which is an IO operation and could be slow for many textures
            {}

         // change size
         int max_size=INT_MAX; //((tex.max_size>0) ? tex.max_size : INT_MAX);

         if(change_type>=0 || max_size!=INT_MAX || tex.downsize) // if any change is desired
         {
            // detect if we actually need to retype/resize
            if((change_type>=0 || max_size!=INT_MAX) && !tex.downsize) // in this detection we can change only 'change_type' and 'max_size', however if 'downsize' is set, then we will always convert, so no need to check this
            {
               ImageHeader header; if(ImageLoadHeader(dynamic ? Proj.texPath(tex.src_tex_id) : pfd.data.name, header)) // if this is a dynamically generated texture then get the header from its source, as the dynamic texture may not exist yet
               {
                  if(header.type      ==change_type)change_type=-1     ; // if image already is of that type , then disable retyping
                  if(header.size.max()<=   max_size)   max_size=INT_MAX; // if image size fits into the limit, then disable resizing
               }
            }

            if(change_type>=0 || max_size!=INT_MAX || tex.downsize)
            {
               Str           src_name=pfd.data.name,
                            dest_name=Proj.texFormatPath(tex.id, FormatSuffix(IMAGE_TYPE(change_type)), tex.downsize);
               pfd.data.set(dest_name); // adjust pak file path
               FileInfo src_fi(src_name);
               if(tex.regenerate // if texture is going to be regenerated in this process, then always allow converting it
               || Compare(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc, 1)) // if different (compare just modify time, because sizes will always be different due to different formats)
                  convert.New().set(src_name, dest_name, src_fi.modify_time_utc, change_type, !tex.uses_alpha, false, tex.mtrl_base_1, tex.downsize, max_size); // create new conversion
            }
         }
      }else
      {
         tex_not_found++;
      }
   }
   if(tex_not_found)Gui.msgBox(S, S+tex_not_found+" Texture"+CountS(tex_not_found)+" were not found");
}
/******************************************************************************/
void SetPublishFiles(Memb<PakFileData> &files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Memc<Mems<byte>> &file_data)
{
   files    .clear();
   generate .clear();
   convert  .clear();
   file_data.clear();

   if(PublishEsProj) // publish as *.EsenthelProject
   {
      Project temp; temp=Proj;
      Memc<UID> remove; Proj.floodRemoved(remove, Proj.root); remove.sort(Compare);
      REPA(temp.elms)if(remove.binaryHas(temp.elms[i].id, Compare))temp.elms.removeValid(i, true);
      temp.getTextures(temp.texs); // keep only used textures

      // project "Data" file
      {
         File file; temp.save(file.writeMem()); file.pos(0);
         Mems<byte>  &data=file_data.New(); data.setNum(file.size()).loadRawData(file);
         PakFileData &pfd =files.New();
         pfd.name="Data";
         pfd.modify_time_utc.getUTC();
         pfd.data.set(data.data(), data.elms());
      }

      // elements
      FREPA(temp.elms)
      {
         Elm &elm=temp.elms[i];
         if(elm.type==ELM_WORLD)
         {
            Str world_edit_path=S+"Edit/"+EncodeFileName(elm.id),
                world_game_path=S+"Game/"+EncodeFileName(elm.id),
                world_edit_path_src, world_game_path_src;

            {PakFileData &pfd=files.New(); pfd.name=world_edit_path; pfd.type=FSTD_DIR;} // world edit folder
            {PakFileData &pfd=files.New(); pfd.name=world_game_path; pfd.type=FSTD_DIR;} // world game folder

            if(Proj.getWorldPaths(elm.id, world_edit_path_src, world_game_path_src))
               if(WorldVer *ver=Proj.worldVerGet(elm.id))
            {
               world_edit_path.tailSlash(true);
               world_game_path.tailSlash(true);

               { // world edit data
                  PakFileData &pfd=files.New();
                  pfd.name    =world_edit_path    +"Data";
                  pfd.data.set(world_edit_path_src+"Data");
               }
               { // world game settings
                  PakFileData &pfd=files.New();
                  pfd.name    =world_game_path    +"Settings";
                  pfd.data.set(world_game_path_src+"Settings");
               }

               // areas, waypoints and water
               Str area_game_path    =world_game_path    +"Area\\",
                   area_edit_path    =world_edit_path    +"Area\\",
                   area_game_path_src=world_game_path_src+"Area\\",
                   area_edit_path_src=world_edit_path_src+"Area\\",
               waypoint_game_path    =world_game_path    +"Waypoint\\",
               waypoint_edit_path    =world_edit_path    +"Waypoint\\",
               waypoint_game_path_src=world_game_path_src+"Waypoint\\",
               waypoint_edit_path_src=world_edit_path_src+"Waypoint\\",
                   lake_edit_path    =world_edit_path    +"Lake\\",
                   lake_edit_path_src=world_edit_path_src+"Lake\\",
                  river_edit_path    =world_edit_path    +"River\\",
                  river_edit_path_src=world_edit_path_src+"River\\";
               FREPA(ver.areas) // areas, process in order to avoid re-sorting
               {
                  Str src;

                  // edit
                  src=area_edit_path_src+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name=area_edit_path+ver.areas.lockedKey(i);
                     pfd.data.set(src);
                  }
                  // game
                  src=area_game_path_src+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name=area_game_path+ver.areas.lockedKey(i);
                     pfd.data.set(src);
                  }
               }
               FREPA(ver.waypoints) // waypoints, process in order to avoid re-sorting
               {
                  Str file_id=EncodeFileName(ver.waypoints.lockedKey(i));
                  { // edit
                     PakFileData &pfd=files.New();
                     pfd.name    =waypoint_edit_path    +file_id;
                     pfd.data.set(waypoint_edit_path_src+file_id);
                  }
                  if(FExist(waypoint_game_path_src+file_id)) // waypoint game file can be deleted when it was removed so we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name    =waypoint_game_path    +file_id;
                     pfd.data.set(waypoint_game_path_src+file_id);
                  }
               }
               FREPA(ver.lakes) // lakes, process in order to avoid re-sorting
               {
                  Str      file_id=EncodeFileName(ver.lakes.lockedKey(i));
                  PakFileData &pfd=files.New();
                  pfd.name    =lake_edit_path    +file_id;
                  pfd.data.set(lake_edit_path_src+file_id);
               }
               FREPA(ver.rivers) // rivers, process in order to avoid re-sorting
               {
                  Str      file_id=EncodeFileName(ver.rivers.lockedKey(i));
                  PakFileData &pfd=files.New();
                  pfd.name    =river_edit_path    +file_id;
                  pfd.data.set(river_edit_path_src+file_id);
               }
            }
         }else
         if(elm.type==ELM_MINI_MAP) // mini-map
         {
            { // edit
               PakFileData &pfd=files.New();
               pfd.name    =S+"Edit/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.editPath(elm.id));
            }
            { // game
               Str mini_map_path=S+"Game/"+EncodeFileName(elm.id), mini_map_game_path_src;
               PakFileData &pfd=files.New(); pfd.name=mini_map_path; pfd.type=FSTD_DIR; // mini map folder
               if(ElmMiniMap *data=elm.miniMapData())
                  if(MiniMapVer *ver=Proj.miniMapVerGet(elm.id))
               {
                  mini_map_path.tailSlash(true);
                  mini_map_game_path_src=Proj.gamePath(elm.id).tailSlash(true);

                  PakFileData &pfd=files.New(); // mini map settings
                  pfd.name    =mini_map_path         +"Settings";
                  pfd.data.set(mini_map_game_path_src+"Settings");

                  FREPA(ver.images) // process in order to avoid re-sorting
                  {
                     PakFileData &pfd=files.New(); // mini map image
                     pfd.name    =mini_map_path         +ver.images[i];
                     pfd.data.set(mini_map_game_path_src+ver.images[i]);
                  }
               }
            }
         }else // regular element
         {
            if(elm.type==ELM_CODE)
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Code/"+EncodeFileName(elm.id)+CodeExt; // keep extension so when using copy elms to another project, we have consistency between copying from both *.EsenthelProject and normal projects
               pfd.data.set(           Proj.codePath(elm.id));
               // don't save code base
            }
            if(ElmEdit(elm.type))
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Edit/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.editPath(elm.id));
               pfd.compress_mode=((elm.type==ELM_IMAGE) ? COMPRESS_DISABLE : COMPRESS_ENABLE); // in Edit folder images are stored using JPG/PNG/WEBP
            }
            if(ElmGame(elm.type))
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Game/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.gamePath(elm.id));
               pfd.compress_mode=((elm.type==ELM_VIDEO) ? COMPRESS_DISABLE : COMPRESS_ENABLE); // videos are already compressed, some sounds may be stored as WAV, so try to compress them if possible
            }
         }
      }

      // textures
      FREPA(temp.texs)
      {
       C UID &tex_id=temp.texs[i];
         PakFileData &pfd=files.New();
         pfd.name    =S+"Game/Tex/"+EncodeFileName(tex_id);
         pfd.data.set(                Proj.texPath(tex_id));
      }
   }else
   {
      // Engine.pak files (add this as first to preserve order of loading "Engine.pak" and then "Project.pak", in case some project files overwrite engine files)
      if(!CodeEdit.appEmbedEngineData() && !PublishDataAsPak)
      {
         Pak engine; if(engine.load(BinPath().tailSlash(true)+"Engine.pak"))FREPA(engine)
         {
          C PakFile     &pf  =engine.file    (i );
          C Str         &name=engine.fullName(pf);
            PakFileData &pfd =files.New();
            pfd.name    =name;
            pfd.data.set(name);
            pfd.type=pf.type();
            pfd.modify_time_utc=pf.modify_time_utc;
            pfd.xxHash64_32=pf.data_xxHash64_32;
         }
      }

      // Project files
      if(CodeEdit.appPublishProjData() || PublishDataOnly)
      {
         Memt<Elm*> elms; FREPA(Proj.elms) // process in order
         {
            Elm &elm=Proj.elms[i]; if(elm.finalPublish() && ElmPublish(elm.type))elms.add(&elm);
         }
         AddPublishFiles(elms, files, generate, convert);
      }else
      if(PublishExeType==Edit.EXE_NEW || PublishExeType==Edit.EXE_APK || PublishExeType==Edit.EXE_IOS) // for Windows New, Android and iOS if Project data is not included, then include only App data
      {
         Memt<Elm*> elms; Proj.getActiveAppElms(elms);
         AddPublishFiles(elms, files, generate, convert);
      }
   }
}
void GetPublishFiles(Memb<PakFileData> &files) // this is to be called outside of publishing just to get a list of files
{
   Memc<ImageGenerate> generate;
   Memc<ImageConvert > convert;
   Memc<Mems<byte>   > file_data;
   PublishDataAsPak=true;
   PublishDataOnly =true;
   PublishExeType  =Edit.EXE_EXE;
   PublishEsProj   =false;
   SetPublishFiles(files, generate, convert, file_data);
}
cchar8 *PVRTCQuality[]=
{
   "Lowest - Fastest",
   "Low - Fast",
   "Normal",
   "High - Slow",
   "Highest - Slowest",
};
void PVRTCQualityChanged(ptr) {SetPVRTCQuality(PublishPVRTCQuality());}
/******************************************************************************/
bool InitPublish()
{
   PublishOk=false;
   PublishErrorMessage.clear();
   Publish.progress.reset();
   SetKbExclusive();
   Proj.pause(); // this will also flush all unsaved data which is crucial for publishing
   PublishAreasLeft=Proj.worldAreasToRebuild();
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   PublishPVRTCQualityTextStyle.reset().align.set(0, 1); PublishPVRTCQualityTextStyle.size=0.055;
   Gui+=PublishSkipOptimize    .create(Rect_C(0, -0.20, 0.45, 0.08), "Skip for now").focusable(false); PublishSkipOptimize.mode=BUTTON_TOGGLE;
   Gui+=PublishPVRTCQuality    .create(Rect_C(0, -0.40, 0.45, 0.06), PVRTCQuality, Elms(PVRTCQuality)).set(GetPVRTCQuality(), QUIET).func(PVRTCQualityChanged).focusable(false);
   Gui+=PublishPVRTCQualityText.create(PublishPVRTCQuality.rect().up(), "PVRTC Compression Quality:", &PublishPVRTCQualityTextStyle);
   return true;
}
void ShutPublish()
{
   PublishCancel();
   UpdateThread           .del(); // delete the thread first
   UpdateProgress         .del();
   PublishFiles           .del();
   PublishGenerate        .del();
   PublishConvert         .del();
   PublishFileData        .del();
   PublishSkipOptimize    .del();
   PublishPVRTCQuality    .del();
   PublishPVRTCQualityText.del();
   Proj.resume();
   WindowSetNormal();
   WindowFlash();
   if(!PublishOk)Gui.msgBox("Publishing Failed", PublishErrorMessage);
}
/******************************************************************************/
void PublishSuccess(C Str &open_path, C Str &text)
{
   PublishRes.display(text);
   Explore(open_path);
}
void PublishSuccess()
{
   if(PublishOpenIDE)
   {
      CodeEdit.CodeEditorInterface.openIDE();
   }else
   if(PublishDataNeeded(PublishExeType, PublishBuildMode) && !PublishNoCompile) // we've published data, now we need to compile the code
   {
      CodeEdit.codeDo(PublishBuildMode);
   }else
   if(PublishExeType==Edit.EXE_WEB && PublishBuildMode==Edit.BUILD_PLAY)
   {
      Run("emrun", S+'"'+PublishPath+GetBase(PublishExePath)+'"');
   }else
   {
      Str text;
      if(PublishProjectDataPath.is())text=S+(PublishEsProj ? "Project size: " : "Project data size: ")+FileSize(FSize(PublishProjectDataPath));
      else                           text="Publishing succeeded";
      PublishSuccess(PublishPath, text);
   }
}
void PublishCancel() {UpdateThread.stop(); Publish.progress.stop=true; WindowSetWorking();}
/******************************************************************************/
bool UpdatePublish()
{
   if(Kb.bp(KB_ESC))PublishCancel();
   if(Publish.progress.stop) // when wanting to stop then wait until thread finishes
   {
      if(!UpdateThread.active())
      {
         SetProjectState();
         PublishOk=false; if(!PublishErrorMessage.is())PublishErrorMessage="Publishing breaked on user request";
      }
   }else
   {
      if(!UpdateThread.created())
      {
         UpdateProgress.set(PublishAreasLeft-Proj.worldAreasToRebuild(), PublishAreasLeft);
         Builder.update(false);
         if(Builder.finished())
         {
            WorldEdit.flush(); // flush any world areas that were builded
            SetPublishFiles(PublishFiles, PublishGenerate, PublishConvert, PublishFileData);
            UpdateThread.create(PublishFunc);
         }
      }else
      if(!UpdateThread.active()) // finished
      {
         SetProjectState();
         if(PublishOk)PublishSuccess();
      }else
      UpdateProgress.set(Publish.progress.progress);
      WindowSetProgress(UpdateProgress());
   }
   int sleep=1000/30;
   if(!App.maximized())REPA(MT)if(MT.b(i))if(!MT.guiObj(i) || MT.guiObj(i)==Gui.desktop()){WindowMove(MT.pixelDelta(i).x, MT.pixelDelta(i).y); sleep=1;}
   Time.wait(sleep);
       Gui.update();
    Server.update(null, true); // it's very important to set 'busy' so no commands are processed during publishing
   if(Ms.bp(3))WindowToggle();
   PublishSkipOptimize    .visible(PublishStage==PUBLISH_TEX_OPTIMIZE && !Publish.progress.stop);
   PublishPVRTCQuality    .visible(PublishSkipOptimize.visible() && !PublishSkipOptimize() && PublishPVRTCUse);
   PublishPVRTCQualityText.visible(PublishPVRTCQuality.visible());
   return true;
}
/******************************************************************************/
void DrawPublish()
{
   D.clear(BackgroundColor());
   if(Publish.progress.stop)
   {
      D.text(0, 0.05, (PublishPVRTCUse ? "Waiting for PVRTC to finish" : "Stopping"));
   }else
   {
      if(!UpdateThread.created())
      {
         D.text(0, 0.05, S+"Waiting for "+Proj.worldAreasToRebuild()+" world areas to finish building");
      }else
      {
         Str text; switch(PublishStage)
         {
            case PUBLISH_MTRL_SIMPLIFY: text="Simplifying Materials"; break;
            case PUBLISH_TEX_OPTIMIZE : text=(PublishSkipOptimize() ? PublishPVRTCUse ? "Waiting for PVRTC to finish" : "Copying Textures" : "Optimizing Textures"); break;
            default                   : text=(PublishEsProj ? "Compressing Project" : "Publishing Project"); break;
         }
         D.text(0, 0.05, text);
      }
      GuiPC gpc;
      gpc.visible=gpc.enabled=true; 
      gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
      gpc.offset.zero();
      UpdateProgress.draw(gpc);
      D.clip();
   }
   if(PublishPVRTCUse)
   {
      TextStyleParams ts; ts.align.set(0, -1); ts.size=0.05; D.text(ts, Gui.desktop().rect(), "Compressing PVRTC (iOS Texture Format) - this may take a while.\nMaking sure textures look beautiful and use little space.");
   }
   Gui.draw();
}
/******************************************************************************/
