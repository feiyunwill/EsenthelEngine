/******************************************************************************/
struct Panel
{
   Bool         center_stretch, // if stretch center image  to fit the whole rectangle, default=false
                  side_stretch, // if stretch side   images to fit the whole side     , default=false, this affects 'top_image', 'bottom_image', 'left_right_image', 'top_corner_image' and 'bottom_corner_image'
                center_shadow ; // if include a shadow underneath the center          , default=false, if shadows are enabled then normally they will be drawn around the center image, however if 'center_shadow' is enabled, then shadows will be also draw underneath the center
   Byte         shadow_opacity; // shadow        opacity, 0..255, default=170
   Color        center_color  , // center        color  , default=WHITE, this is affects 'center_image' and 'panel_image'
                border_color  , // border        color  , default=WHITE, this is affects 'border_image'
                  side_color  , // side image    color  , default=WHITE, this is affects 'top_image', 'bottom_image', 'left_right_image', 'top_corner_image' and 'bottom_corner_image'
                  blur_color  ; // blur          color  , default=TRANSPARENT
   Flt          shadow_radius , // shadow        radius , default=0.035
                shadow_offset , // shadow        offset , default=0
                border_size   , // border        size   , default=0
                center_scale  , // center        scale  , default=1
                   top_size   , // top           size   , default=0.1
                bottom_size   , // bottom        size   , default=0.1
            left_right_size   , // left/right    size   , default=0.1
            top_corner_size   , // top    corner size   , default=0.1
         bottom_corner_size   , // bottom corner size   , default=0.1
                   top_offset , // top           offset , default=0
                bottom_offset ; // bottom        offset , default=0
   Vec2     left_right_offset , // left/right    offset , default=Vec2(0, 0)
            top_corner_offset , // top    corner offset , default=Vec2(0, 0)
         bottom_corner_offset ; // bottom corner offset , default=Vec2(0, 0)
   ImagePtr     center_image  , // center        image  , default=null
                border_image  , // border        image  , default=null
                   top_image  , // top           image  , default=null
                bottom_image  , // bottom        image  , default=null
            left_right_image  , // left/right    image  , default=null
            top_corner_image  , // top    corner image  , default=null
         bottom_corner_image  ; // bottom corner image  , default=null
   PanelImagePtr panel_image  ; // panel         image  , default=null

   // get
   void extendedRect           (C Rect &rect, Rect &extended )C; // get rectangle containing panel drawn at 'rect' rectangle
   void innerPadding           (C Rect &rect, Rect &padding  )C;
   void defaultInnerPadding    (              Rect &padding  )C;
   void defaultInnerPaddingSize(              Vec2 &padd_size)C;
#if EE_PRIVATE
   Bool pixelBorder()C; // if this panel draws a 1 pixel border
   Bool getSideScale(C Rect &rect, Flt &scale)C;
#endif

   // operations
   void reset      (         ); // reset to default values
   void scaleBorder(Flt scale); // scale border size, top, bottom, side and corner images according to specified parameter

   // draw
   virtual void draw(                C Rect &rect)C; // draw at rectangle
   virtual void draw(C Color &color, C Rect &rect)C; // draw at rectangle with specified color multiplier

   void drawLines(C Color &line_color, C Rect &rect)C; // draw lines

   // io
   void operator=(C Str &name) ; // load, Exit  on fail
   Bool save     (C Str &name)C; // save, false on fail
   Bool load     (C Str &name) ; // load, false on fail

   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   Panel() {reset();}
};
/******************************************************************************/
DECLARE_CACHE(Panel, Panels, PanelPtr); // 'Panels' cache storing 'Panel' objects which can be accessed by 'PanelPtr' pointer
/******************************************************************************/
