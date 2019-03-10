/******************************************************************************/
extern State               StatePublish;
extern Memb<PakFileData>   PublishFiles;
extern Memc<ImageGenerate> PublishGenerate;
extern Memc<ImageConvert>  PublishConvert;
extern Memc<Mems<byte> >    PublishFileData;
extern SyncLock            PublishLock;
extern bool                PublishOk, PublishNoCompile, PublishOpenIDE, PublishDataAsPak, PublishDataOnly, PublishEsProj;
extern int                 PublishAreasLeft, PublishPVRTCUse;
extern PUBLISH_STAGE       PublishStage;
extern Str                 PublishPath,
                    PublishBinPath, // "Bin/" path (must include tail slash)
                    PublishProjectDataPath, // "Project.pak" path
                    PublishExePath, 
                    PublishErrorMessage;
extern Button              PublishSkipOptimize;
extern ComboBox            PublishPVRTCQuality;
extern Text                PublishPVRTCQualityText;
extern TextWhite           PublishPVRTCQualityTextStyle;
extern Edit::EXE_TYPE       PublishExeType  ;
extern Edit::BUILD_MODE     PublishBuildMode;
extern WindowIO            PublishEsProjIO;
extern cchar8 *PVRTCQuality[]
;
/******************************************************************************/
bool PublishDataNeeded(Edit::EXE_TYPE exe, Edit::BUILD_MODE mode);
void PublishDo();
void PublishEsProjAs(C Str &path, ptr user);
bool StartPublish(C Str &exe_name, Edit::EXE_TYPE exe_type, Edit::BUILD_MODE build_mode, bool no_compile=false, C Str &custom_project_data_path=S, bool open_ide=false, bool es_proj=false);
void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index);
void ImageConvertProcess(ImageConvert &convert, ptr user, int thread_index);
bool PublishFunc(Thread &thread);
Texture* GetTexture(MemPtr<Texture> textures, C UID &tex_id);
void AddPublishFiles(Memt<Elm*> &elms, MemPtr<PakFileData> files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert);
void SetPublishFiles(Memb<PakFileData> &files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Memc<Mems<byte> > &file_data);
void GetPublishFiles(Memb<PakFileData> &files);
void PVRTCQualityChanged(ptr);
bool InitPublish();
void ShutPublish();
void PublishSuccess(C Str &open_path, C Str &text);
void PublishSuccess();
void PublishCancel();
bool UpdatePublish();
void DrawPublish();
/******************************************************************************/
