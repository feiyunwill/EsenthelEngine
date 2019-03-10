/******************************************************************************

   Use these functions to achieve post processing effects.

/******************************************************************************/
struct RippleFx // Ripple Effect
{
   Flt  xx, xy     , // matrix transformation parameters, default=(60, 40)
        yx, yy     , // matrix transformation parameters, default=(60, 35)
        step       , // effect time                     , default=Time.appTime()
        power      , // ripple power                    , default=0.005
        alpha_scale, // alpha  scale                    , default=0
        alpha_add  ; // alpha  add                      , default=1
   Vec2 center     ; // image  center position in UV coordinates (0..1, 0..1), default=(0.5, 0.5)

   // final_ripple_pixel_alpha = DistanceFrom(center)*alpha_scale + alpha_add

   RippleFx& reset(); // reset to default values
   void      draw (C Image &image, C Rect &rect); // draw 'image' using ripple effect, 'rect'=screen rectangle

   RippleFx() {reset();}
};
/******************************************************************************/
struct TitlesFx // Titles Swirl Effect
{
   Flt step  , // time     (0..Inf,                 default=Time.appTime()*2)
       center, // center y (in UV coordinates 0..1, default=0.5  )
       range , // range of sharp    visibility     (default=0.4  )
       smooth, // range of smoothed visibility     (default=0.1  )
       swirl ; // swirl amount                     (default=0.015)

   TitlesFx& reset(); // reset to default values
   void      draw (C Image &image); // draw 'image' using titles swirl effect

   TitlesFx() {reset();}
};
/******************************************************************************/
STRUCT(ColorMatrix , Matrix) // Color Transformation Matrix
//{
   ColorMatrix& setRGB (       ); // set RGB             transformation
   ColorMatrix& setRBG (       ); // set RBG             transformation
   ColorMatrix& setGRB (       ); // set GRB             transformation
   ColorMatrix& setGBR (       ); // set GBR             transformation
   ColorMatrix& setBRG (       ); // set BRG             transformation
   ColorMatrix& setBGR (       ); // set BGR             transformation
   ColorMatrix& setMono(       ); // set monochrome      transformation
   ColorMatrix& setHue (Flt hue); // set approximate hue transformation, 'hue'=hue adjustment (0..1)

   void draw(Flt alpha=1); // apply full-screen color transformation, 'alpha'=effect opacity (0..1)
};
void ColorTransHB (Flt hue,                 Flt brightness, Flt alpha=1); // apply full-screen correct Hue-           Brightness color transformation, 'hue'=hue adjustment (0..1),                                            , 'brightness'=brightness adjustment (0..Inf)
void ColorTransHSB(Flt hue, Flt saturation, Flt brightness, Flt alpha=1); // apply full-screen correct Hue-Saturation-Brightness color transformation, 'hue'=hue adjustment (0..1), 'saturation'=saturation adjustment (0..Inf), 'brightness'=brightness adjustment (0..Inf), 'alpha'=effect opacity (0..1)
/******************************************************************************/
void FadeFx      (C Image &image, Flt time , Image *fade_modified=null               ); // draw 'image' with Fade        Effect, 'time' =effect time  (0..1  ), 'fade_modifier'=optional fade modifier texture (only its alpha channel is used)
void WaveFx      (                Flt time , Flt    scale                            ); // apply full-screen Wave        Effect, 'time' =effect time  (0..Inf), 'scale'=wave scale (0..1)
void RadialBlurFx(                Flt scale, Flt    alpha=1, C Vec2 &center=Vec2(0,0)); // apply full-screen Radial Blur Effect, 'scale'=blur   scale (0..Inf), 'alpha'=effect opacity (0..1), 'center'=screen point center of fading
/******************************************************************************/
