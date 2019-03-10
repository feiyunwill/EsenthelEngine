/******************************************************************************/
class CodeView : Region, Edit::CodeEditorInterface
{
   Memx<EEItem > items;
   Memc<EEItem*> items_sorted;

   virtual void configChangedDebug()override;
   virtual void configChanged32Bit()override;
   virtual void configChangedDX9()override;
   virtual void configChangedEXE()override;
   virtual void visibleChangedOptions      ()override;
   virtual void visibleChangedOpenedFiles  ()override;
   virtual void visibleChangedOutput       ()override;
   virtual void visibleChangedAndroidDevLog()override;

   virtual UID projectID()override;

   virtual UID               appID                              ()override; 
   virtual Str               appName                            ()override; 
   virtual Str               appDirsWindows                     ()override; 
   virtual Str               appDirsNonWindows                  ()override; 
   virtual Str               appHeadersWindows                  ()override; 
   virtual Str               appHeadersMac                      ()override; 
   virtual Str               appHeadersLinux                    ()override; 
   virtual Str               appHeadersAndroid                  ()override; 
   virtual Str               appHeadersiOS                      ()override; 
   virtual Str               appLibsWindows                     ()override; 
   virtual Str               appLibsMac                         ()override; 
   virtual Str               appLibsLinux                       ()override; 
   virtual Str               appLibsAndroid                     ()override; 
   virtual Str               appLibsiOS                         ()override; 
   virtual Str               appPackage                         ()override; 
   virtual Str               appLicenseKey                      ()override; 
   virtual Str               appLocationUsageReason             ()override; 
   virtual Int               appBuild                           ()override; 
   virtual ulong             appFacebookAppID                   ()override; 
   virtual Str               appChartboostAppIDiOS              ()override; 
   virtual Str               appChartboostAppSignatureiOS       ()override; 
   virtual Str               appChartboostAppIDGooglePlay       ()override; 
   virtual Str               appChartboostAppSignatureGooglePlay()override; 
   virtual Edit::STORAGE_MODE appPreferredStorage                ()override;
   virtual UInt              appSupportedOrientations           ()override; 
   virtual UID               appGuiSkin                         ()override; 
   virtual int               appEmbedEngineData                 ()override; 
   virtual Cipher*           appEmbedCipher                     ()override; 
   virtual COMPRESS_TYPE     appEmbedCompress                   ()override; 
   virtual int               appEmbedCompressLevel              ()override; 
   virtual DateTime          appEmbedSettingsTime               ()override;  // return Max of all params affecting PAK's
   virtual Bool              appPublishProjData                 ()override; 
   virtual Bool              appPublishPhysxDll                 ()override; 
   virtual Bool              appPublishSteamDll                 ()override; 
   virtual Bool              appPublishOpenVRDll                ()override; 
   virtual Bool              appPublishDataAsPak                ()override; 
   virtual Bool              appAndroidExpansion                ()override; 
 //virtual Bool              appWindowsCodeSign                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.windowsCodeSign ()     ; return super.appWindowsCodeSign();}
   virtual ImagePtr          appIcon                            ()override;
   virtual ImagePtr          appImagePortrait                   ()override;
   virtual ImagePtr          appImageLandscape                  ()override;
   virtual ImagePtr          appNotificationIcon                ()override;

   static void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index);
   static void ImageConvertProcess (ImageConvert  &convert , ptr user, int thread_index);

   virtual void appSpecificFiles(MemPtr<PakFileData> files)override;
   virtual void appInvalidProperty(C Str &msg)override;

   virtual void validateActiveSources()override;

   virtual Rect menuRect()override;
   virtual Rect sourceRect()override;

   virtual Str sourceProjPath(C UID &id)override;
   virtual Edit::ERROR_TYPE sourceDataLoad(C UID &id, Str &data)override;
   virtual Bool sourceDataSave(C UID &id, C Str &data)override;
   virtual void sourceChanged(bool activate=false)override;
   virtual Bool elmValid    (C UID &id              )override;                    
   virtual Str  elmBaseName (C UID &id              )override;                    
   virtual Str  elmFullName (C UID &id              )override;                    
   virtual void elmHighlight(C UID &id, C Str  &name)override;                    
   virtual void elmOpen     (C UID &id              )override;                     // don't toggle currently opened source because it will close it
   virtual void elmLocate   (C UID &id              )override;                    
   virtual void elmPreview  (C UID &id, C Vec2 &pos, bool mouse, C Rect&clip)override;
   virtual Str idToText(C UID &id, Bool *valid)override;                     // if we're in code then it already displays an ID, no need to write the same ID on top of that

   virtual void getProjPublishElms(Memc<ElmLink> &elms)override;

   Str importPaths(C Str &path)C;

   void drag(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);
   void drop(C MemPtr<Str> &names, GuiObj *obj, C Vec2 &screen_pos);

   bool selected()C;      
   void selectedChanged();
   void flush();   // do nothing because sources need to be saved manually

   void overwriteChanges();
   void sourceTitleChanged(C UID &id); // call if name or "modified state" changed
   void sourceRename(C UID &id);
   bool sourceDataSet(C UID &id, C Str &data);

   void cleanAll();

   static int Compare(C Enum::Elm &a, C Enum::Elm &b);
   void makeAuto(bool publish=false);

   void codeDo(Edit::BUILD_MODE mode);
   void buildDo(Edit::BUILD_MODE mode);
   void build  ();                   
   void publish();                   
   void play   ();                   
   void debug  ();                   
   void rebuild();                    // override to call custom 'build'
   void openIDE();
   bool Export(Edit::EXPORT_MODE mode, bool data=false);

   virtual void publishSuccess(C Str &exe_name, Edit::EXE_TYPE exe_type, Edit::BUILD_MODE build_mode, C UID &project_id)override; // code compiled successfully

   virtual CodeView& del()override;
   static int CompareItem(EEItem *C &a, EEItem *C &b);                         
   static int CompareItem(EEItem *C &a,     C Str &b);                         
   void createItems(Memx<EEItem> &dest, C Memx<Edit::Item>&src, EEItem*parent);
   void create(GuiObj &parent);
   void resize(bool full=true);
   void activate(Elm *elm);
   void toggle(Elm *elm);
   void toggle(EEItem *item);
   void erasing(C UID &elm_id);
   void kbSet();             

   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
   virtual void    update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;
   void drawPreview(ListElm *list_elm);
};
/******************************************************************************/
/******************************************************************************/
extern CodeView CodeEdit;
/******************************************************************************/
