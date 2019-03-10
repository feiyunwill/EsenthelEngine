/******************************************************************************/
/******************************************************************************/
class ImporterClass
{
   enum MODE
   {
      UPDATE, // reload target from source file
      ANIM  , // import animations only to target
      CLOTH , // import as cloth to target
      ADD   , // add source to target
   };
   class Import
   {
      class MaterialEx : XMaterial
      {
         Material mtrl;
         Image    base_0, base_1, detail, macro, reflection, light;
         UID      base_0_id, base_1_id, detail_id, macro_id, reflection_id, light_id;

         void copyTo(EditMaterial &dest, C TimeStamp &time=TimeStamp().getUTC())C;
         void check(C Str &path, Str &tex);
         void process(C Str &path);

public:
   MaterialEx();
      };
      class ImageEx : Image
      {
         bool cube;
         File raw;

public:
   ImageEx();
      };

      int              status; // -1=in progress, 0=fail, 1=ok
      bool             has_loop, cancel, remember_result, has_color, has_alpha, ignore_anims;
      ELM_TYPE         type;
      MODE             mode;
      UID              elm_id, parent_id;
      Str              file, force_name;
      Mesh             mesh;
      Skeleton         skel;
      Memc<XAnimation> anims;
      Memc<MaterialEx> mtrls;
      Memc<int>        part_mtrl_index;
      Memc<Str>        bone_names;
      Memc<ImageEx>    images;
      File             raw;
      EditFont         edit_font;
      Font             font;
      Str              code;

      Str nodeName(int i)C;
      Str nodeUID (int i)C; // unique string identifying a node !! needs to be the same as 'EditSkeleton.nodeUID' !!

      Import& set(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name, bool remember_result);

      static bool ApplyVolume(ptr data, int size, flt vol);

      bool import(); // !! this is called on a secondary thread !!

public:
   Import();
   };

   static void ImportDo(Import &import, ImporterClass &importer, int thread_index);
   class ImportElm
   {
      UID  elm_id;
      bool remember_result;
      
      void set(C UID &elm_id, bool remember_result=false);
   };

   Memx<Import>                 imports     ; // elements that are being imported right now, Memx due to const mem addr for processing on threads
   Memc<ImportElm>              import_queue; // elements to import
   Map<UID, Edit::RELOAD_RESULT> import_results; // doesn't need to be thread-safe
   Memc<UID>                    texs; // this is a copy of 'Project.texs' that includes textures currently being imported in 'threads'
   Threads                      threads;
   SyncLock                     lock;
   Menu                         import_menu;
   Memc<Str>                    import_files;
   UID                          import_target;
   MODE                         import_mode;

  ~ImporterClass(); // delete threads before other members

   // get
   bool busy()C;

   // manage
   static void ImportFull   (ImporterClass &ic); // import normally
   static void ImportReplace(ImporterClass &ic);
   static void ImportAnim   (ImporterClass &ic);
   static void ImportCloth  (ImporterClass &ic);
   static void ImportAdd    (ImporterClass &ic);

   void create();

   // operations
   void stop();

   void importNew(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name=S, bool remember_result=false);
   void importSpecialFile(C Str &file);
   void importSpecialDir(C Str &path);
   void importSpecial(MODE mode);
   void import(Elm &target, Memc<Str> &files, C Vec2 &screen_pos);

   bool includeTex(C UID &tex_id);
   void excludeTex(C UID &tex_id);
   void clearProj();
   void opened(Project &proj, ElmNode &node);
   Import* findImport(C UID &elm_id);
   ImportElm* findImportQueue(C UID &elm_id);
   bool inQueue(C UID &elm_id);
   void cancelImports(C MemPtr<UID> &sorted_elm_ids);
   void cancel(C MemPtr<UID> &elm_ids);
   void getResult(C MemPtr<UID> &elms, MemPtr<Edit::IDParam<Edit::RELOAD_RESULT> > results);
   void clearImportResults(C MemPtr<UID> &elm_ids);
   void forgetResult(C MemPtr<UID> &elm_ids);
   void reload(C MemPtr<UID> &elm_ids, bool remember_result);
   void investigate(Elm &elm);
   void investigate(ElmNode &node);
   void investigate(Memc<UID> &elm_ids);
   void processUpdate(Import &import);
   void processCloth(Import &import);
   void processAnim(Import &import);
   void processAdd(Import &import);

   // update
   void update();

public:
   ImporterClass();
};
/******************************************************************************/
/******************************************************************************/
extern ImporterClass Importer;
/******************************************************************************/
