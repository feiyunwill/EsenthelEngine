/******************************************************************************/
/******************************************************************************/
class ImportTerrainClass : ClosableWindow
{
   class GuiImage2 : ImageSkin
   {
      bool     mono;
      int      channel;
      Image    image_sw, image_hw;
      WindowIO wio;

      static void Load(C Str &name, GuiImage2 &img);
             void load(C Str &name);

      GuiImage2& create(C Rect &rect, bool mono=false);
      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;

public:
   GuiImage2();
   };
   class MaterialChannel : GuiImage
   {
      Button      remove;
      MaterialPtr mtrl;
      UID         mtrl_id;

      static void Clear(MaterialChannel &mc);
             void clear();
      void set(Elm &elm);
      MaterialChannel& create(C Rect &rect, Color rect_color, C Str &desc);
      virtual void update(C GuiPC &gpc)override;

public:
   MaterialChannel();
   };

   Tabs  pos_mode, height, mtrl, color, mtrl_channel, height_mode, mtrl_mode, color_mode;
   bool  color_img_alpha;
   flt   mtrl_blend, color_blend;
   VecI2 area, radius, size, target;
   Vec2  height_range;
   GuiImage2 height_image, mtrl_image, color_image;
   Memx<Property> area_p, radius_p, size_p, target_p, height_prop, mtrl_prop, color_prop;
   Button import_b;
   TextBlack ts, ts_l, ts_r;
   Text            mtrl_channels_t, select_t, world_area_coords_t;
   MaterialChannel mtrl_channels[4];

   static void HeightModeChanged(ImportTerrainClass &it);
   static void    ChannelChanged(ImportTerrainClass &it);
   static void            Import(ImportTerrainClass &it);
          void            import();

   // get
   Vec2 heightRange();    
   bool  areaRectValid()C;
   RectI areaRect()C;
   // manage
   void create();
   // operations
   void clearProj();
   void erasing(C UID &elm_id);
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);
   virtual void update(C GuiPC &gpc)override;

public:
   ImportTerrainClass();
};
/******************************************************************************/
/******************************************************************************/
extern ImportTerrainClass ImportTerrain;
/******************************************************************************/
