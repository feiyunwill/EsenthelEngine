/******************************************************************************/
/******************************************************************************/
class MeshAOClass : ClosableWindow
{
   static const cchar8 *Func_t[]
; ASSERT(MAF_FULL==0 && MAF_QUARTIC==1 && MAF_SQUARE==2 && MAF_LINEAR==3 && MAF_LINEAR_REV==4 && MAF_SQUARE_REV==5);

   bool           preview, baked_valid, processed_ready, finished;
   flt            strength, bias, max, ray_length, scale;
   MESH_AO_FUNC   func;
   uint           src_id, baked_src_id, change_id, baked_change_id;
   Property      *info, *preview_prop;
   Memx<Property> props;
   TextBlack      ts;
   Button         ok;
   MeshLod        src, processed, baked;
   Thread         thread;
   SyncLock       lock;

   static void PreviewToggle(MeshAOClass &editor);
   static void OK           (MeshAOClass &editor);
   static void ChangedParams(C Property &prop); 
          void changedParams();

   bool needRebuild()C;

   static bool Process(Thread &thread);
          bool process();

   void startThread();
   void createSrc();
           MeshAOClass& activate()override;
   virtual MeshAOClass& hide    ()override;

  ~MeshAOClass();                // delete thread before other members
   MeshAOClass& del   ()override;
   MeshAOClass& create();
   virtual void update(C GuiPC &gpc)override;

public:
   MeshAOClass();
};
/******************************************************************************/
/******************************************************************************/
extern MeshAOClass MeshAO;
/******************************************************************************/
