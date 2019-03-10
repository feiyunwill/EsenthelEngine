/******************************************************************************/
/******************************************************************************/
class NewLodClass : ClosableWindow
{
   bool           preview, simplified_valid, processed_ready, keep_border, draw_at_distance, finished, stop;
   flt            intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, draw_distance, scale;
   uint           src_id, simplified_src_id, change_id, simplified_change_id;
   Property      *quality, *preview_prop, *draw_at_distance_prop;
   Memx<Property> props;
   TextBlack      ts;
   Button         ok, from_file;
   WindowIO       win_io;
   MeshLod        src, processed, simplified;
   Thread         thread;
   SyncLock       lock;

   static flt NextLodDist(flt dist); // make next LOD distance 2x bigger, and at least at 2 meters away

   static void FromFile(NewLodClass &nl);
   static void    PreviewToggle(NewLodClass &nl);
   static void DrawAtDistToggle(NewLodClass &nl);
   static void OK(NewLodClass &nl);
   static void ChangedParams(C Property &prop);
          void changedParams();

   static void Import  (C Str &name, ptr=null);     
   static void ImportEx(C Str &name, int lod_index);

   bool needRebuild()C;

   static bool Simplify(Thread &thread);
          bool simplify();

   void startThread();
   void createSrc();
           NewLodClass& activate()override;
   virtual NewLodClass& hide    ()override;

  ~NewLodClass();                // delete thread before other members
   NewLodClass& del   ()override;
   NewLodClass& create();
   virtual void update(C GuiPC &gpc)override;

public:
   NewLodClass();
};
/******************************************************************************/
/******************************************************************************/
extern NewLodClass NewLod;
/******************************************************************************/
