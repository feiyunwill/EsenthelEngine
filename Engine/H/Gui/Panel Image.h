/******************************************************************************/
struct PanelImageParams
{
   struct Light
   {
      Bool enabled               ; //   if light is enabled                  , default=false
      Flt  intensity             , //      light intensity          ,  0..1  , default=0.5
           back                  , // back light intensity          , -1..1  , default=0.25
           highlight             , //  highlight                    ,  0..1  , default=0
           highlight_cut         , //  highlight cut-off            ,  0..1  , default=0.8
           specular              , //      specular intensity       ,  0..1  , default=1.0 (this is multiplied by section specular intensity)
           specular_back         , // back specular intensity       , -1..1  , default=0
           specular_exp          , //      specular exponent        ,  0..Inf, default=32
           specular_highlight    , //      specular highligt        ,  0..1  , default=0
           specular_highlight_cut; //      specular highligt cut-off,  0..1  , default=0.8
      Vec2 angle                 ; //      light angle              , -1..1  , default=(0, 0.4)

      Light();

   #if EE_PRIVATE
      Bool save(File &f)C;
      Bool load(File &f) ;
   #endif
   };

   struct ImageParams
   {
      enum MODE : Byte
      {
         MULTIPLY  , // result = src *             image
         SCALE     , // result = src * ScaleFactor(image-0.5)
         ADD       , // result = src +             image
         ADD_SIGNED, // result = src +            (image-0.5)
         BLEND     , // result = Blend(src, image)
      };
      MODE mode      ; // default=MULTIPLY
      Bool blur_clamp; // if use clamping when blurring the image, default=false
      Int  blur      ; // blur range (in pixels),    0..256, default=0
      Flt  uv_scale  , // image uv scale        ,    0..Inf, default=1
           uv_warp   , // image uv warp by depth,    0..1  , default=0
           intensity ; // intensity             ,    0..1  , default=0
      Vec2 uv_offset ; // image uv offset       , -Inf..Inf, default=(0, 0)

   #if EE_PRIVATE
      Bool is()C {return !Equal(intensity, 0);}

      Bool save(File &f)C;
      Bool load(File &f) ;
   #endif

      ImageParams();
   };

   struct Section
   {
      Flt    size                , // section size      , this parameter is unused for section #0,  0..1, default=0
             top_offset          , // section top offset, this parameter is unused for section #0, -1..1, default=0
             round_depth         , // if use round depth or linear depth, 0..1, 0=linear, 1=round, default=1
             outer_depth         , // for section #0 this specifies the outer ring depth   , for section #1 this specifies center ring depth, 0..1, default=1
             inner_depth         , // 0..1, default=1
             inner_distance      , // for section #0 this specifies the inner ring distance, for section #1 this specifies center ring distance, 0..1, default=0.5
             specular            , // section specular intensity, 0..1, default=0 (this is multiplied by light specular intensity)
             reflection_intensity; // intensity of reflection image, used when "reflection!=null", 0..1, default=0.3
      Vec2   smooth_depth        ; // depth smoothing intensity, x=horizontal, y=vertical, 0..1, default=(0, 0)
      Color  color,                    // section     color, default=WHITE
             outer_color, inner_color, // outer/inner color, default=WHITE
             color_top, color_bottom, color_left, color_right, // top/bottom/left/right color, default=WHITE
             outer_border_color, inner_border_color, prev_border_color; // border colors, default=(0, 0, 0, 128), (255, 255, 255, 64), TRANSPARENT
    C Image *depth_overlay, *color_overlay, *reflection; // images, default=null
      ImageParams depth_overlay_params, color_overlay_params, depth_noise, color_noise; // image parameters

      Section();

   #if EE_PRIVATE
      Bool save(File &f)C;
      Bool load(File &f) ;
   #endif
   };

   Bool    cut_left, cut_right, cut_bottom, cut_top, // if cut-off the sides, default=false
           cut_corners[2][2] , // if cut-off the corners, default=false
     force_uniform_stretch[2]; // if force uniform stretching [0]-horizontal, [1]=vertical, default=(false, false)
   Int     resolution        , // image resolution (in pixels), 1..Inf, default=64
           width, height     ; // image size multipliers, 1..8, default=(1, 1)
   Flt     round_corners     , //  0..1, default=0.5
           cut_corner_slope  , //  0..2, default=1
           cut_corner_amount , //  0..1, default=1
           left_slope        , // -1..1, default=0
           right_slope       , // -1..1, default=0
           border_size       , //  0..1, default=2.0/resolution (which is  1 pixel  of the panel image)
           outer_glow_spread , //  0..1, default=0.5
           outer_glow_radius , //  0..1, border_size*12         (which is 12 pixels of the panel image)
           inner_glow_radius , //  0..1, border_size*12         (which is 12 pixels of the panel image)
           light_ambient     , // ambient lighting, 0..1, default=0.5
           depth             , // depth multiplier, 0..1, default=1
           round_depth       , // if use round depth or linear depth, 0..1, 0=linear, 1=round, default=1
           inner_distance    , // inner ring distance used for calculating depth, 0..1, default=0.5
           shadow_radius     , // 0..1, default=0
           shadow_opacity    , // 0..1, default=0.5
           shadow_spread     , // 0..1, default=0.5
           max_side_stretch  ; // affects maximum allowed stretching of the side parts of the image, 0..Inf, default=0.2
   Vec2    smooth_depth      , // depth smoothing intensity, x=horizontal, y=vertical, 0..1, default=(0, 0)
           extend            ; // extend side outwards,  0..1, default=0
   Rect    extend_inner_padd ; // extend inner padding, -1..1, default=0
   Light   lights  [2]       ; // lights  for this panel image
   Section sections[2]       ; // sections in this panel image
   Color   color,              // global color, default=WHITE
           color_top, color_bottom, color_left, color_right, // global top/bottom/left/right color, default=WHITE
           outer_glow_color, inner_glow_color; // outer/inner glow colors, default=TRANSPARENT

   Flt             images_size, // 3x3 images size multiplier                , 0..Inf, default=0.5
                    top_height, // height of the                top    images, 0..1  , default=1
                 bottom_height, // height of the                bottom images, 0..1  , default=1
             left_right_width , // width  of the left and right middle images, 0..1  , default=1
             top_corner_width , // width  of the left and right top    images, 0..1  , default=1
          bottom_corner_width ; // width  of the left and right bottom images, 0..1  , default=1
 C Image *top_image, *bottom_image, *center_image, *left_image, *right_image, *top_left_image, *top_right_image, *bottom_left_image, *bottom_right_image; // 3x3 images, default=null

   Bool save(File &f)C; // save parameters to   file, false on fail, this does not save image pointers
   Bool load(File &f) ; // load parameters from file, false on fail

   PanelImageParams();
};
/******************************************************************************/
struct PanelImage
{
   Image image;

   // manage
   void create(C PanelImageParams &params, Image *depth_map=null, Int super_sample=4, FILTER_TYPE filter=FILTER_BEST, Threads *threads=null); // create 'PanelImage' from specified 'params', 'depth_map'=optional parameter allowing to obtain the depth map of the panel for debugging purposes, 'super_sample'=super sampling used for creating the Panel Image (higher value increases the quality of the image, but makes it slower to create it), 'filter'=filter used when down-sampling the super sampled image to normal size (unused when super sampling was not enabled), 'threads'=optionally pass a created 'Threads' object to create the 'PanelImage' in multi-threaded mode

   // get
   Bool  extendedRect           (C Rect &rect, Rect &extended )C; // returns true if 'extended' is different than 'rect'
   void  innerPadding           (C Rect &rect, Rect &padding  )C;                         // get         inner padding of 'rect' rectangle 'PanelImage'
 C Rect& defaultInnerPadding    (                             )C {return _inner_padding;} // get default inner padding
   void  defaultInnerPaddingSize(              Vec2 &padd_size)C;

   // draw
   void draw(                                    C Rect &rect)C; // draw at specified 'rect' screen rectangle
   void draw(C Color &color, C Color &color_add, C Rect &rect)C; // draw at specified 'rect' screen rectangle with custom colors

   void drawVertical(                                    C Rect &rect)C; // draw at specified 'rect' screen rectangle in vertical mode
   void drawVertical(C Color &color, C Color &color_add, C Rect &rect)C; // draw at specified 'rect' screen rectangle in vertical mode with custom colors

   void drawFrac        (C Color &color, C Color &color_add, C Rect &rect, Flt frac_x, Bool include_padding)C; // draw at specified 'rect' screen rectangle with custom colors and horizontally limited to 'frac_x' (0..1) fraction, 'include_padding'=if take padding into account when processing the fraction
   void drawVerticalFrac(C Color &color, C Color &color_add, C Rect &rect, Flt frac_y, Bool include_padding)C; // draw at specified 'rect' screen rectangle with custom colors and   vertically limited to 'frac_y' (0..1) fraction, 'include_padding'=if take padding into account when processing the fraction

   void drawBaseLines   (C Color &line_color, C Rect &rect)C; // draw lines on unscaled image
   void drawScaledLines (C Color &line_color, C Rect &rect)C; // draw lines on   scaled image
   void drawInnerPadding(C Color &line_color, C Rect &rect)C; // draw lines of  inner padding

   // io
   Bool save(C Str  &name)C; // save PanelImage to   file, false on fail
   Bool load(C Str  &name) ; // load PanelImage from file, false on fail
   Bool save(  File &f   )C; // save PanelImage to   file, false on fail
   Bool load(  File &f   ) ; // load PanelImage from file, false on fail

   void del(); // delete manually, this can be called optionally to release panel image memory
   PanelImage();
#if EE_PRIVATE
   void drawBorders         (C Color &color, C Color &color_add, C Rect &rect)C; // draw only borders
   Bool getSideScale        (C Rect &rect, Flt &scale)C;
   Bool getSideScaleVertical(C Rect &rect, Flt &scale)C;
   void zero();
#else
private:
#endif
   Bool _same_x, _padd_any, _force_uniform_stretch[2];
   Flt  _size_x[3][2], _size_y[2], _tex_x[3][2], _tex_y[2]; // [y][x]
   Vec2 _side_size, _padd, _tex_left_top, _tex_right_bottom;
   Rect _inner_padding;
};
/******************************************************************************/
DECLARE_CACHE(PanelImage, PanelImages, PanelImagePtr); // 'PanelImages' cache storing 'PanelImage' objects which can be accessed by 'PanelImagePtr' pointer
/******************************************************************************/
