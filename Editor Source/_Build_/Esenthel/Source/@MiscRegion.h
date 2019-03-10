/******************************************************************************/
/******************************************************************************/
class MiscRegion : Region
{
   Button    hide_proj, vid_opt, move_misc, play;
   ComboBox  menu, build;
   TextWhite online_ts;
   Text      online;
   byte      screenshot;
   VecI2     pos;

   static void Play       (MiscRegion &mr);
   static void Debug      (MiscRegion &mr);
   static void RunToCursor(MiscRegion &mr);
   static void Build      (MiscRegion &mr);
   static void Rebuild    (MiscRegion &mr);
   static void Clean      (MiscRegion &mr);
   static void CleanAll   (MiscRegion &mr);
   static void Stop       (MiscRegion &mr);
   static void OpenIDE    (MiscRegion &mr);

   static void ExportTXT          (MiscRegion &mr);
   static void ExportCPP          (MiscRegion &mr);
 /*static void ExportVS2008       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2008 );}
   static void ExportVS2010       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2010 );}
   static void ExportVS2012       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2012 );}
   static void ExportVS2013       (MiscRegion &mr) {CodeEdit.Export(Edit.EXPORT_VS2013 );}*/
   static void ExportVS2015       (MiscRegion &mr);
   static void ExportVS2017       (MiscRegion &mr);
   static void ExportAndroid      (MiscRegion &mr);
   static void ExportAndroidData  (MiscRegion &mr);
   static void ExportXcode        (MiscRegion &mr);
   static void ExportXcodeDataiOS (MiscRegion &mr);
   static void ExportLinuxMake    (MiscRegion &mr);
   static void ExportLinuxNetBeans(MiscRegion &mr);
   static void ExportData         (MiscRegion &mr);
   static void ExportDataAndroid  (MiscRegion &mr);
   static void ExportDataiOS      (MiscRegion &mr);

   static void CodeExplore(MiscRegion &mr);
   static void CodeImport (MiscRegion &mr);
   static void CodeExport (MiscRegion &mr);

   static void ConfigDebug  (MiscRegion &mr);
   static void ConfigRelease(MiscRegion &mr);
   static void Config32     (MiscRegion &mr);
   static void Config64     (MiscRegion &mr);
   static void ConfigDX9    (MiscRegion &mr);
   static void ConfigDX11   (MiscRegion &mr);
   static void ConfigEXE    (MiscRegion &mr);
   static void ConfigDLL    (MiscRegion &mr);
   static void ConfigLIB    (MiscRegion &mr);
   static void ConfigNEW    (MiscRegion &mr);
   static void ConfigAPK    (MiscRegion &mr);
   static void ConfigMAC    (MiscRegion &mr);
   static void ConfigIOS    (MiscRegion &mr);
   static void ConfigLinux  (MiscRegion &mr);
   static void ConfigWeb    (MiscRegion &mr);

   static void ProjList       (bool all_saved=true, ptr=null);
   static void ProjList       (MiscRegion &mr);             
   static void   HideProj     (MiscRegion &mr);             
   static void ToggleProj     (MiscRegion &mr);             
   static void ToggleTheater  (MiscRegion &mr);             
   static void ToggleRemoved  (MiscRegion &mr);             
   static void   FindProj     (MiscRegion &mr);             
   static void   ModeClose    (MiscRegion &mr);             
   static void   Mode0        (MiscRegion &mr);             
   static void   Mode1        (MiscRegion &mr);             
   static void   Mode2        (MiscRegion &mr);             
   static void   Mode3        (MiscRegion &mr);             
   static void   Mode4        (MiscRegion &mr);             
   static void   Mode5        (MiscRegion &mr);             
   static void VidOpt         (MiscRegion &mr);             
   static void BuyLicense     (ptr           );             
   static void Help           (MiscRegion &mr);             
   static void Fullscreen     (MiscRegion &mr);             
   static void Screenshot     (MiscRegion &mr);             
   static void CalcShow       (MiscRegion &mr);             
   static void VerifyElms     (MiscRegion &mr);  
   static void ProjSettings   (MiscRegion &mr);  
   static void Publish        (MiscRegion &mr);  
   static void PublishEsProj  (MiscRegion &mr);  
 //static void SetLicense     (MiscRegion &mr) {LicenseCheck.activate();}
   static void AppStore       (MiscRegion&mr);
   static void EraseRemoved   (MiscRegion&mr);
   static void MergeSimMtrl   (MiscRegion&mr);
   static void DetectSimTex   (MiscRegion&mr);
   static void CreateMtrls    (MiscRegion&mr);
   static void DataSizeStats  (MiscRegion&mr);
   static void UnusedMaterials(MiscRegion&mr);
   static void ShowTexDownsize(MiscRegion&mr);
   static void SyncCodes      (MiscRegion&mr);
   static void EditorOptions  (MiscRegion&mr);
   static void CodeOutput     (MiscRegion&mr);
   static void CodeDevLog     (MiscRegion&mr);
   static void Quit           (MiscRegion&mr);
   static void About          (MiscRegion&mr);
   static void Move(MiscRegion &mr);        

   void setPos(C VecI2 &pos);
   void create();
   void resize();
   void updateMove();
   virtual void update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;
   virtual MiscRegion& show()override;     
   virtual MiscRegion& hide()override;     

public:
   MiscRegion();
};
/******************************************************************************/
/******************************************************************************/
extern MiscRegion Misc;
/******************************************************************************/
