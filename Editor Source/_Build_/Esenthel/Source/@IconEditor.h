/******************************************************************************/
/******************************************************************************/
class IconEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      ElmIcon data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   static void Render();
          void render();

   Matrix matrix();              
   Box    box   ();              
   static flt GetDist(C Box&box); // avoid changing this func as it will break old icon settings
   flt getDist(); // !! assumes that viewport and obj are already set !!

   static void Draw(Viewport &viewport);
          void draw(bool final=false);
   void makeIcon();

   bool             make_do;
   UID              elm_id;
   Elm             *elm;
   bool             changed, changed_file, no_scale;
   flt              prop_max_x;
   ObjectPtr        obj;
   AnimatedSkeleton skel;
   SkelAnim         skel_anim;
   Button           undo, redo, locate, make;
   ViewportSkin     viewport;
   IconSettings     settings;
   Property        *var, *nos;
   int                             mesh_variation;
   Memc<ParamEditor::MeshVariation> mesh_variations;
   Edit::Undo<Change> undos;   void undoVis();

   ElmIcon* data()C;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void Object   (  IconEditor &ie, C Str &text);
   static Str  Object   (C IconEditor &ie             );
   static void Variation(  IconEditor &ie, C Str &text);
   static Str  Variation(C IconEditor &ie             );
   static void Settings (  IconEditor &ie, C Str &text);
   static Str  Settings (C IconEditor &ie             );
   static void Animation(  IconEditor &ie, C Str &text);
   static Str  Animation(C IconEditor &ie             );
   static void AnimPos  (  IconEditor &ie, C Str &text);
   static Str  AnimPos  (C IconEditor &ie             );

   static void Make  (IconEditor &editor);
   static void Undo  (IconEditor &editor);
   static void Redo  (IconEditor &editor);
   static void Locate(IconEditor &editor);

   void setObj();
   void meshVariationChanged();
   void create();
   void toGui(); 

   virtual IconEditor& hide     (            )  override;
   virtual Rect        sizeLimit(            )C override;
   virtual IconEditor& rect     (C Rect &rect)  override;

   void flush();
   void setChanged(bool file=false);
   void reloadSettings();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   IconEditor();
};
/******************************************************************************/
/******************************************************************************/
extern IconEditor IconEdit;
/******************************************************************************/
