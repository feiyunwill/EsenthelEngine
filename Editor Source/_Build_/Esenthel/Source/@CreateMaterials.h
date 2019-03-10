/******************************************************************************/
/******************************************************************************/
class CreateMaterials : PropWin
{
   class Src
   {
      Str name, // display name
          file; // for files
      UID id; // for elements

      Src& set(C UID &id  );
      Src& set(C Str &file);

public:
   Src();
   };
   static int CompareSrcPath(C Src &a, C Src &b);
   static int CompareSrcID  (C Src &a, C Src &b);

   static void Create  (CreateMaterials &cm);
          void createDo();

   static void Process(Src &src, ptr user, int thread_index);
          void process(Src &src);

   Memc<Src> data;
   List<Src> list;
   Region    region;
   Button    create_button;
   Text      text;
   Str       path, shared_path;
   UID       parent_id;
   SyncLock  lock;

   void clearProj();
   void create();
   virtual CreateMaterials& show()override;
   virtual void update(C GuiPC &gpc)override;

   void drop(C Str &name);
   void drop(C MemPtr<Str> &names, GuiObj *obj, C Vec2 &screen_pos);
   void drag(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);

public:
   CreateMaterials();
};
/******************************************************************************/
/******************************************************************************/
extern CreateMaterials CreateMtrls;
/******************************************************************************/
