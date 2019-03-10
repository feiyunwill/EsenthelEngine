/******************************************************************************/
/******************************************************************************/
class EnumEditor : ClosableWindow
{
   class Enum
   {
      bool   removed;
      Color  color;
      flt    offset;
      UID    enum_id;
      Str    name;
      Button remove;

      void setColor();               
      void operator=(C EditEnum&src);

public:
   Enum();
   };
   class RenameWindow : ClosableWindow
   {
      UID      enum_id;
      TextLine textline;

      void create();
      void activate(C UID &enum_id, C Str &name);
      virtual void update(C GuiPC &gpc)override;

public:
   RenameWindow();
   };
   class ListEnum : List<Enum>
   {
      int getPos(C Vec2 &pos);

      virtual void draw(C GuiPC &gpc)override;
   };
   class Change : Edit::_Undo::Change
   {
      EditEnums enums;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   static cchar8 *Type_t[]
; ASSERT(EditEnums::DEFAULT==0 && EditEnums::TYPE_1==1 && EditEnums::TYPE_2==2 && EditEnums::TYPE_4==3);

   Region       region;
   Memx<Enum>   data; // Memx because of Button
   ListEnum     list;
   Button       undo, redo, locate, new_enum, show_removed;
   TextBlack    ts;
   Text         type_t;
   ComboBox     type;
   UID          elm_id;
   Elm         *elm;
   bool         changed;
   EditEnums    enums;
   RenameWindow rename_window;
   Edit::Undo<Change> undos;   void undoVis();

   static void NewEnum(EnumEditor &ee);
          void newEnum();
   bool rename(C UID &enum_id, C Str &name);

   static void Undo  (EnumEditor &editor);
   static void Redo  (EnumEditor &editor);
   static void Locate(EnumEditor &editor);

   static void ShowRemoved(EnumEditor &editor);

   static void TypeChanged(EnumEditor &editor);

   void create();
   virtual Rect   sizeLimit(            )C override;
   virtual EnumEditor& rect(C Rect &rect)  override;

   void skinChanged();
   void flush();
   void setChanged();
   static void Remove(Enum &e);
          void remove(Enum &e);
   void toGui();
   void set(Elm *elm);
           void        activate(Elm *elm);                               
           void        toggle  (Elm *elm);                               
   virtual EnumEditor& hide    (        )override;                       
   static void DragEnums(EnumEditor &ee, GuiObj *obj, C Vec2&screen_pos);
          void dragEnums(                GuiObj *obj, C Vec2&screen_pos);
   virtual void update(C GuiPC &gpc)override;
   void elmChanged(C UID &enum_id);
   void erasing(C UID &elm_id);  

public:
   EnumEditor();
};
/******************************************************************************/
/******************************************************************************/
extern EnumEditor EnumEdit;
/******************************************************************************/
