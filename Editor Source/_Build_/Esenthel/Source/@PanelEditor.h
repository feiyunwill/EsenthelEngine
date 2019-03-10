/******************************************************************************/
/******************************************************************************/
class PanelEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      EditPanel data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };
   class GuiPanel : GuiCustom
   {
      flt clip_x;

      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;

      virtual void draw(C GuiPC &gpc)override;

public:
   GuiPanel();
   };

   UID               elm_id;
   Elm              *elm;
   bool              changed;
   EditPanel         edit;
       PanelPtr      game;
    GuiPanel         gui_panel;
   Button            undo, redo, locate, show_lines,
                     set_corner_size,
                     set_top_size,
                     set_bottom_size,
                     set_side_size;
   Edit::Undo<Change> undos;   void undoVis();

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void ParamsCenterStretch(EditPanel &e, C Str &t);
   static void ParamsSideStretch  (EditPanel &e, C Str &t);

   static void ParamsCenterColor(EditPanel &e, C Str &t);
   static void ParamsBorderColor(EditPanel &e, C Str &t);
   static void ParamsSideColor  (EditPanel &e, C Str &t);
   static void ParamsBlurColor  (EditPanel &e, C Str &t);

   static void ParamsShadowOpacity(EditPanel &e, C Str &t);
   static void ParamsShadowRadius (EditPanel &e, C Str &t);
   static void ParamsShadowOffset (EditPanel &e, C Str &t);
   static void ParamsShadowCenter (EditPanel &e, C Str &t);

   static void ParamsBorderSize        (EditPanel &e, C Str &t);
   static void ParamsCenterScale       (EditPanel &e, C Str &t);
   static void ParamsTopSize           (EditPanel &e, C Str &t);
   static void ParamsTopOffset         (EditPanel &e, C Str &t);
   static void ParamsBottomSize        (EditPanel &e, C Str &t);
   static void ParamsBottomOffset      (EditPanel &e, C Str &t);
   static void ParamsLeftRightSize     (EditPanel &e, C Str &t);
   static void ParamsLeftRightOffset   (EditPanel &e, C Str &t);
   static void ParamsTopCornerSize     (EditPanel &e, C Str &t);
   static void ParamsTopCornerOffset   (EditPanel &e, C Str &t);
   static void ParamsBottomCornerSize  (EditPanel &e, C Str &t);
   static void ParamsBottomCornerOffset(EditPanel &e, C Str &t);

   static Str ParamsCenterImage      (C EditPanel &e);
   static Str ParamsBorderImage      (C EditPanel &e);
   static Str ParamsTopImage         (C EditPanel &e);
   static Str ParamsBottomImage      (C EditPanel &e);
   static Str ParamsLeftRightImage   (C EditPanel &e);
   static Str ParamsTopCornerImage   (C EditPanel &e);
   static Str ParamsBottomCornerImage(C EditPanel &e);
   static Str ParamsPanelImage       (C EditPanel &e);

   static void ParamsCenterImage      (EditPanel &e, C Str &t);
   static void ParamsBorderImage      (EditPanel &e, C Str &t);
   static void ParamsTopImage         (EditPanel &e, C Str &t);
   static void ParamsBottomImage      (EditPanel &e, C Str &t);
   static void ParamsLeftRightImage   (EditPanel &e, C Str &t);
   static void ParamsTopCornerImage   (EditPanel &e, C Str &t);
   static void ParamsBottomCornerImage(EditPanel &e, C Str &t);
   static void ParamsPanelImage       (EditPanel &e, C Str &t);

   static void SetCornerSize(PanelEditor &editor);
   static void SetTopSize(PanelEditor &editor);
   static void SetBottomSize(PanelEditor &editor);
   static void SetSideSize(PanelEditor &editor);

   static void Undo  (PanelEditor &editor);
   static void Redo  (PanelEditor &editor);
   static void Locate(PanelEditor &editor);

   void create();
   void toGame();
   void toGui ();

   virtual PanelEditor& hide     (            )  override;
   virtual Rect         sizeLimit(            )C override;
   virtual PanelEditor& rect     (C Rect &rect)  override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id);                              
   void drag(Memc<UID> &elms, GuiObj*obj, C Vec2&screen_pos);

public:
   PanelEditor();
};
/******************************************************************************/
/******************************************************************************/
extern PanelEditor PanelEdit;
/******************************************************************************/
