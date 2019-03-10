/******************************************************************************/
/******************************************************************************/
class SizeStatistics : ClosableWindow
{
   class ElmListElm // project element as list element
   {
      bool     proj_elm; // if this is a project element or some other file
      cchar8  *type_name;
      Str      name, path;
      UID      id;
      ELM_TYPE type;
      uint     size, size_compressed;

      Str finalName()C;

      void setElm(C UID &id, uint size, uint size_compressed);
      void setTex(C Str &name, C Str &path, uint size, uint size_compressed);

      static Str FinalName(C ElmListElm &elm);
      static Str Size     (C ElmListElm &elm);
      static Str SizeCmpr (C ElmListElm &elm);

public:
   ElmListElm();
   };

   Memc<ElmListElm> data;
   List<ElmListElm> list;
   Menu             menu;
   Memc<UID>        menu_elm;
   Region           region;
   ViewportSkin     preview;
   MemberDesc       sort_size[2];
   Str              path;
   Pak              pak;
   Image            image;
   UID              image_id;
   int              packed_col;

   static void SelectSel(SizeStatistics &ss);

   static void DrawPreview(Viewport &viewport);
          void drawPreview();

   void release();
   virtual SizeStatistics& del ()override;
   virtual SizeStatistics& hide()override;

   virtual SizeStatistics& rect(C Rect &rect)override;
   void create();
   void refresh(C Str &path, Cipher *cipher);
   void display(C Str &path, Cipher *cipher=null);
   void displayUnusedMaterials();
   virtual void update(C GuiPC &gpc)override;

public:
   SizeStatistics();
};
/******************************************************************************/
/******************************************************************************/
extern SizeStatistics SizeStats;
/******************************************************************************/
