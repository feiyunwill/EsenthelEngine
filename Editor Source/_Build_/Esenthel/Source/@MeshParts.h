/******************************************************************************/
/******************************************************************************/
class MeshParts : Window
{
   class RenamePart : ClosableWindow
   {
      int      index; // number of elements before this one with similar name
      Str      name;
      TextLine textline;

      static void Hide(RenamePart &rp);

      void create();
      void activate(int part);
      virtual void update(C GuiPC &gpc)override;

public:
   RenamePart();
   };
   class Part
   {
      bool   removed;
      Color  color;
      int    index;
      Button remove;

      void setColor();

public:
   Part();
   };
   class WeldVtxPos : ClosableWindow
   {
      Text     text;
      TextLine textline;

      void create();
      virtual void update(C GuiPC &gpc)override;
   };

   static Str Removed(C Part &p);
   static Str Name   (C Part &p);
   static Str Vtxs   (C Part &p);
   static Str Tris   (C Part &p);
   static Str Mtrl   (C Part &p);
   static Str VtxsN  (C Part &p);
   static Str TrisN  (C Part &p);

   static void Refresh     (MeshParts &parts);
   static void Locate      (MeshParts &parts);
   static void ListChanging(MeshParts &parts);
   static void ListChanged (MeshParts &parts);
   static void EditChanged (MeshParts &parts);
   static void HiddenToggle(Part &p);
   static void Rename();
   void renamePart(int index, C Str &old_name, C Str &new_name);
   static void Erase();
   static void Remove();
   static void Restore();
   static void Hide();
   static void Focus();    
   static void Duplicate();
   static void CopyMem();
   static void NewLod();
   static void Merge();

   MemberDesc md_vtxs, md_tris;
   Memx<Part> data;
   List<Part> list;
   Region     region;
   Button     show_removed, edit_selected, locate;
   Menu       menu;
   RenamePart rename;
   WeldVtxPos weld_vtx;
   ComboBox   add;

   virtual Rect sizeLimit()C override;
              C Rect& rect();                    
   virtual MeshParts& rect(C Rect &rect)override;
   // PART is the same as ABS
   bool partSel   (int part)C; // if part        is selected
   bool partOp    (int part)C; // if part should be operated on
   int  visToPart (int vis )C;
   int  partToVis (int part)C;
   void highlight (int part); 
   void clicked   (int part); 
   void erasedPart(int part);
   void addedPart(int part);
   void selParts(C MemPtr<int>&parts);
   void newMesh    ();              
   void modeChanged();
   void skinChanged();

   void showMenu(C Vec2 &pos);

   static void AddBox();
   static void AddBall();
   static void AddBall2();
   static void AddBallIco();
   static void AddCapsule();
   static void AddTube();
   static void AddCone();
   static void AddTorus();
   static void AddMemory();

   MeshParts& create();
   void refresh();
   virtual void update(C GuiPC &gpc)override;
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
