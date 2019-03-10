/******************************************************************************/
/******************************************************************************/
class ImageEditor : PropWin
{
   static void Render(); // we just want the skybox, so no need to draw anything else

   class GuiImage2 : ImageSkin
   {
      Camera cam;

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
      virtual void    update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;
   };
   class CubeFace : ImageSkin
   {
      Vec2     ofs;
      DIR_ENUM face;

      CubeFace& create(DIR_ENUM dir, int x, int y);
      virtual void draw(C GuiPC &gpc)override;

public:
   CubeFace();
   };
   class Change : Edit::_Undo::Change
   {
      ElmImage data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   UID         elm_id;
   Elm        *elm;
   bool        changed, no_scale, source;
   int         channels;
   Button      undo, redo, locate;
   GuiImage2   gui_image;
   Image      edit_image, image_2d;
   ImagePtr   game_image;
   CubeFace   cube_faces[6];
   Property  *width, *height, *type, *mip_maps, *mem_size, *chn, *nos, *src;
   Edit::Undo<Change> undos;   void undoVis();                             

   ElmImage* data()C;

   class ImageMode
   {
      IMAGE_MODE mode;
      cchar8    *name;
      cchar     *desc;
   };
   static ImageMode ImageModes[]
;
   static cchar8 *Channels[]
;
   static int ChannelsElms;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void Type    (  ImageEditor &ie, C Str &t);
   static Str  Type    (C ImageEditor &ie          );
   static void Mode    (  ImageEditor &ie, C Str &t);
   static Str  Mode    (C ImageEditor &ie          );
   static void MipMaps (  ImageEditor &ie, C Str &t);
   static Str  MipMaps (C ImageEditor &ie          );
   static void Pow2    (  ImageEditor &ie, C Str &t);
   static Str  Pow2    (C ImageEditor &ie          );
   static void Width   (  ImageEditor &ie, C Str &t);
   static Str  Width   (C ImageEditor &ie          );
   static void Height  (  ImageEditor &ie, C Str &t);
   static Str  Height  (C ImageEditor &ie          );
   static void AlphaLum(  ImageEditor &ie, C Str &t);
   static Str  AlphaLum(C ImageEditor &ie          );

   static void Undo  (ImageEditor &editor);
   static void Redo  (ImageEditor &editor);
   static void Locate(ImageEditor &editor);

   void setMipMap(bool on);

   void create();
   void make2D();
   void setInfo();

   virtual void update(C GuiPC &gpc)override;
   virtual ImageEditor& hide     (            )  override;
   virtual Rect         sizeLimit(            )C override;
   virtual ImageEditor& rect     (C Rect &rect)  override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   class ImageName
   {
      Str name, file;
      void set(C Str &name, C Str &file);
   };
   int FaceToIndex(DIR_ENUM dir);
   void setImage(Memc<ImageName> &images, GuiObj *obj);
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);
   void erasing(C UID &elm_id);                              

public:
   ImageEditor();
};
/******************************************************************************/
/******************************************************************************/
extern ImageEditor ImageEdit;
/******************************************************************************/
