/******************************************************************************/
/******************************************************************************/
class DetectSimilarTextures : PropWin
{
   static const bool Decompress; // performance was the same for both options, so keep 'Decompress' as false, to be able to process bigger sizes for better precision
   static const int  MaxTexSize; // a very big project can have ~7000 textures, total memory for that would be (7000*128*128)>>20 = 109 MB when using BC7, which is acceptable memory usage

   class Data
   {
      UID id;
      Str name;
      
      void set(C UID &id);

public:
   Data();
   };
   class Pair
   {
      UID a, b;

      void set(C UID &a, C UID &b);
   };
   class UIDEx : UID
   {
      bool mtrl_base_1;

public:
   UIDEx();
   };
   class ImageEx
   {
      bool        mtrl_base_1;
      Mems<Image> mips; // since we can't lock multiple mip-maps at the same time, we need to store them as separate images
      
      void create(bool mtrl_base_1, C Image &src);

public:
   ImageEx();
   };
   Memc<Data>                  data;
   List<Data>                  list;
   Region                      region;
   Progress                    progress;
   Threads                     threads;
   Thread                      io_thread;
   uintptr                     io_thread_id;
   Memc<UIDEx>                 proj_texs;
   ThreadSafeMap<UID, ImageEx> loaded_texs; // make thread-safe just in case
   Memc<Pair>                  similar_pair;
   SyncLock                    similar_pair_lock;
   bool                        pause;
   int                         compared;

   bool mtrl_base_1   ;
   flt  avg_difference,
        similar       ,
        similar_dif   ;

   static void CompareImage(UID &a_id, UID &b_id, int thread_index);
   static int ImageLoad(ImageHeader &header, C Str &name);
   static bool IOThread(Thread &t);                     
          bool ioThread();

   static void Changed(C Property &prop);
   void reset();
  ~DetectSimilarTextures();
   void stop2();
   void stop();
   virtual DetectSimilarTextures& show()override;
   virtual DetectSimilarTextures& hide()override;
   void clearProj();         
   void addSimilar(C UID &a, C UID &b);
   void addSimilarAll(C UID &a, C UID &b);
   virtual Rect sizeLimit()C override;                       
                          C Rect& rect()C;                   
   virtual DetectSimilarTextures& rect(C Rect &rect)override;
   static void CurChanged(DetectSimilarTextures &dst);
   void create();
   virtual void update(C GuiPC &gpc)override;

public:
   DetectSimilarTextures();
};
/******************************************************************************/
/******************************************************************************/
extern DetectSimilarTextures DST;
/******************************************************************************/
