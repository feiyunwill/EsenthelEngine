/******************************************************************************/
/******************************************************************************/
class TextStyleEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      EditTextStyle data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };
   class TextClip : TextNoTest
   {
      virtual void draw(C GuiPC &gpc)override;
   };

   UID              elm_id;
   Elm             *elm;
   bool             changed;
   EditTextStyle    edit;
       TextStylePtr game;
       TextClip     text;
   Panel            panel;
   Button           undo, redo, locate;
   Property        *font_prop;
   Edit::Undo<Change> undos;   void undoVis();

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void ParamsShadow(EditTextStyle &e, C Str &t);
   static void ParamsShade (EditTextStyle &e, C Str &t);
   static void ParamsColor (EditTextStyle &e, C Str &t);
   static void ParamsSelect(EditTextStyle &e, C Str &t);
   static void ParamsAlign (EditTextStyle &e, C Str &t);
   static void ParamsSize  (EditTextStyle &e, C Str &t);
   static void ParamsSpace (EditTextStyle &e, C Str &t);
   static void ParamsFont  (EditTextStyle &e, C Str &t);
   static Str ParamsFont(C EditTextStyle &e);

   static void Undo  (TextStyleEditor &editor);
   static void Redo  (TextStyleEditor &editor);
   static void Locate(TextStyleEditor &editor);

   void create();
   void toGame();
   void toGui ();

   virtual TextStyleEditor& hide     (            )  override;
   virtual Rect             sizeLimit(            )C override;
   virtual TextStyleEditor& rect     (C Rect &rect)  override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 
   void fontChanged();
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);

public:
   TextStyleEditor();
};
/******************************************************************************/
/******************************************************************************/
extern TextStyleEditor TextStyleEdit;
/******************************************************************************/
