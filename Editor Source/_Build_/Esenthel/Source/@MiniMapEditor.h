/******************************************************************************/
/******************************************************************************/
class MiniMapEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      ElmMiniMap data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   const int         pixel_border; // additional number of pixels to draw while creating mini maps (helps in making bloom/glow objects more smooth at the edges of mini map images)
   flt               prop_max_x;
   UID               elm_id;
   Elm              *elm;
   bool              changed;
   int               progress;
   VecI2             image_pos;
   RectI             images;
   ViewportSkin      viewport;
   EnvironmentPtr    env;
   Button            undo, redo, locate, make;
   Edit::Undo<Change> undos;   void undoVis();

   static void Render();
          void render();

   int areasPerImage();
   UID       worldID();
   flt  miniMapWidth(); // required width in meters for loaded areas
   int   totalImages();

   static flt ShadowStep(int i, int num);

   static void Draw(Viewport &viewport);
          void draw(bool final=false);

   static void Make(MiniMapEditor &mme);

   static cchar8 *ImageSizes[]
;
   static cchar8 *AreasPerImage[]
;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void World   (  MiniMapEditor &mme, C Str &text);
   static Str  World   (C MiniMapEditor &mme             );
   static void Env     (  MiniMapEditor &mme, C Str &text);
   static Str  Env     (C MiniMapEditor &mme             );
   static void ArPerImg(  MiniMapEditor &mme, C Str &text);
   static Str  ArPerImg(C MiniMapEditor &mme             );
   static void Size    (  MiniMapEditor &mme, C Str &text);
   static Str  Size    (C MiniMapEditor &mme             );

   static void Undo  (MiniMapEditor &editor);
   static void Redo  (MiniMapEditor &editor);
   static void Locate(MiniMapEditor &editor);

   ElmMiniMap* data()C;
   MiniMapVer*  ver()C;

   void makeDo();
   void init(bool center=false);
   bool step();
   void create();
   virtual Rect sizeLimit()C override;               
   virtual MiniMapEditor& rect(C Rect &rect)override;

   void flush();
   void setAreas(int border=4);
   void reloadAreas();
   void reloadWorld();
   void setChanged(bool world=false, bool areas=false);
   void set(Elm *elm);
           void           activate(Elm *elm);        
           void           toggle  (Elm *elm);        
   virtual MiniMapEditor& hide    (        )override;

   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   MiniMapEditor();
};
/******************************************************************************/
/******************************************************************************/
extern MiniMapEditor MiniMapEdit;
/******************************************************************************/
