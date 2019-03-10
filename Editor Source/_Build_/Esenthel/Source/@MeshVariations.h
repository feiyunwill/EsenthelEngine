/******************************************************************************/
/******************************************************************************/
class MeshVariations : Window
{
   class RenameVariation : ClosableWindow
   {
      int      index; // number of elements before this one with similar name
      Str      name;
      TextLine textline;

      static void Hide(RenameVariation &rv);

      void create();
      void activate(int variation);
      virtual void update(C GuiPC &gpc)override;

public:
   RenameVariation();
   };
   class Variation
   {
      bool   removed;
      Color  color;
      int    index;
      Button remove;

      void setColor();

public:
   Variation();
   };
   static Str Removed(C Variation &p);
   static Str Name   (C Variation &v);

   static void Remove(Variation &v);
   static void Remove();
   static void Rename();
   void renameVariation(int index, C Str &old_name, C Str &new_name);
   static void ClearName();
   void newVariation(C Str *name=null);
   static void Add(MeshVariations &mv);
   static void ListChanged(MeshVariations &mv);

   class ListVariation : List<Variation>
   {
      int getPos(C Vec2 &pos);

      virtual void draw(C GuiPC &gpc)override;
   };

   Memx<Variation> data;
   ListVariation   list;
   Button          add;
   Region          region;
   Menu            menu;
   RenameVariation rename;

   virtual Rect sizeLimit()C override;
                   C Rect& rect();                   
   virtual MeshVariations& rect(C Rect&rect)override;

   void newMesh();           
   void showMenu(C Vec2&pos);

   MeshVariations& create();
   static void DragVariations(MeshVariations &mv, GuiObj *obj, C Vec2 &screen_pos);
          void dragVariations(                    GuiObj *obj, C Vec2 &screen_pos);
   void erasedVariation(int variation);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   void refresh();
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
