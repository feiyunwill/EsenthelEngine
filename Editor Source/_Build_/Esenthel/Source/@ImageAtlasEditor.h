/******************************************************************************/
/******************************************************************************/
class ImageAtlasEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      ElmImageAtlas data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };
   class ListElm
   {
      bool   removed;
      Color  color;
      UID    img_id;
      Button remove;

      void setColor();                         
      void operator=(C ElmImageAtlas::Img&src);

      static int ComparePath(C ListElm &a, C ListElm &b);

public:
   ListElm();
   };

   Region        region;
   Memx<ListElm> list_data; // Memx because of Button
   List<ListElm> list;
   Button        make, undo, redo, locate, show_removed;
   UID           elm_id;
   Elm          *elm;
   bool          changed, changed_file;
   Edit::Undo<Change> undos;   void undoVis();

   static void ShowRemoved(ImageAtlasEditor &iae);
   static void Make       (ImageAtlasEditor &iae);

   static Str ElmFullName(C ListElm &le);
   static Str ElmName    (C ListElm &le);

   static int CompareSource(C ImageAtlas::Source &a, C ImageAtlas::Source &b);

   static void MipMaps(  ImageAtlasEditor &iae, C Str &t);
   static Str  MipMaps(C ImageAtlasEditor &iae          );

   static void Undo  (ImageAtlasEditor &editor);
   static void Redo  (ImageAtlasEditor &editor);
   static void Locate(ImageAtlasEditor &editor);

   ElmImageAtlas* data()C;

   GuiObj& desc(C Str &desc)override;
   void makeDo();

   void create();
   virtual Rect sizeLimit()C override;                 
   virtual ImageAtlasEditor& rect(C Rect&rect)override;

   void skinChanged();
   void flush();
   void setChanged(bool file=false);
   static void Remove(ListElm &le); 
          void remove(ListElm &le);
   void toGui();
   void set(Elm *elm);
           void              activate(Elm *elm);        
           void              toggle  (Elm *elm);        
   virtual ImageAtlasEditor& hide    (        )override;

   void elmChanged(C UID &elm_id);
   void drag(Memc<UID> &elms, GuiObj*obj, C Vec2&screen_pos);
   void erasing(C UID &elm_id);                            

public:
   ImageAtlasEditor();
};
/******************************************************************************/
/******************************************************************************/
extern ImageAtlasEditor ImageAtlasEdit;
/******************************************************************************/
