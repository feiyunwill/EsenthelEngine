/******************************************************************************

   'Environment' holds information about graphics options like:
      ambient, bloom, clouds, fog, sky, sun.

/******************************************************************************/
struct Environment
{
   struct Ambient
   {
      Bool on               ; // if enabled       ,     true/false    , default=true
      Vec  color            , // ambient     color, (0,0,0) .. (1,1,1), default=(0.4, 0.4, 0.4)
           night_shade_color; // night shade color, (0,0,0) .. (1,1,1), default=(0.0, 0.0, 0.0)

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Ambient() {reset();}
   };

   struct Bloom
   {
      Bool on      , // if enabled     , true/false, default=true
           half    , // half/quarter   , true/false, default=true, this specifies whether bloom should be calculated using half of quarter sized render targets (half is more precise but slower, quarter is more blurred)
           saturate, // saturation     , true/false, default=true
           maximum ; // maximum filter , true/false, default=false
      Byte blurs   ; // number of blurs,    0..4   , default=1
      Flt  original, // original color ,    0..Inf , default=1.0
           scale   , // bloom scale    ,    0..Inf , default=0.5
           cut     ; // bloom cutoff   ,    0..Inf , default=0.3

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Bloom() {reset();}
   };

   struct Clouds
   {
      struct Layer // Cloud Layer
      {
         Flt      scale   ; // texture scale   ,         0..Inf      , default={0.35, 0.41, 0.50, 0.62}
         Vec2     velocity; // texture velocity,      -Inf..Inf      , default={0.010, 0.008, 0.006, 0.004}
         Vec4     color   ; // texture color   , (0,0,0,0)..(1,1,1,1), default=(1,1,1,1)
         ImagePtr image   ; // texture         ,                       default=null
      };

      Bool  on               ; // if enabled, true/false, default=true
      Flt   vertical_scale   , // vertical texture scaling ,    1..2   , default=1.05, setting this value higher than 1 helps covering the empty gap between flat ground and the clouds
            ray_mask_contrast; // sun rays masking contrast,    1..Inf , default=4   , this is used when "Sun.rays_mode==SUN_RAYS_HIGH"
      Layer layers[4]        ; // layer array

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Clouds() {reset();}
   };

   struct Fog
   {
      Bool on        , // if enabled        ,    true/false   , default=false
           affect_sky; // if fog affects sky,    true/false   , default=false
      Flt  density   ; // fog density       ,       0..1      , default=0.02
      Vec  color     ; // fog color         , (0,0,0)..(1,1,1), default=(0.5, 0.5, 0.5)

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Fog() {reset();}
   };

   struct Sky
   {
      Bool on  ; // if enabled  , true/false, default=true
      Flt  frac; // sky fraction,    0..1   , default=0.8, (1 is the fastest), fraction of the Viewport range where the sky starts

      Flt      atmospheric_density_exponent , // atmospheric density exponent,            0..1                   , default=1.0, (1 is the fastest)
               atmospheric_horizon_exponent ; // atmospheric horizon exponent,            0..Inf                 , default=3.5, (this affects at what height the horizon color will be selected instead of the sky color)
      Vec4     atmospheric_horizon_color    , // atmospheric horizon color   ,    (0,0,0,0)..(1,1,1,1)           , default=(0.32, 0.46, 0.58, 1.0) here alpha specifies opacity to combine with star map when used
               atmospheric_sky_color        ; // atmospheric sky     color   ,    (0,0,0,0)..(1,1,1,1)           , default=(0.16, 0.36, 0.54, 0.9) here alpha specifies opacity to combine with star map when used
      ImagePtr atmospheric_stars            ; // atmospheric star map        , image must be in IMAGE_CUBE format, default=null
      Matrix3  atmospheric_stars_orientation; // atmospheric star orientation,       must be normalized          , default=MatrixIdentity3

      ImagePtr skybox; // skybox image, default=null, when specified then it will be used instead of atmospheric sky

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Sky() {reset();}
   };

   struct Sun
   {
      Bool     on             , // if enabled                  ,        true/false         , default=true
               blend          ; // if use blending for image   ,        true/false         , default=true, if true then 'image' will be applied using alpha blending, if false then 'image' will be added onto the screen
      Byte     glow           ; // glow amount                 ,           0..255          , default=128 , total glow amount is equal to ('image' alpha channel * 'image_color' alpha component * 'glow')
      Flt      size           , // image size                  ,           0..1            , default=0.15
               highlight_front, // highlight on atmospheric sky,           0..Inf          , default=0.10
               highlight_back ; // highlight on atmospheric sky,           0..Inf          , default=0.07
      Vec      pos            , // position on sky sphere      , its length must be equal 1, default=!Vec(-1, 1, -1)
               light_color    , // light color                 ,     (0,0,0)..(1,1,1)      , default=(0.7, 0.7, 0.7), value of (0, 0, 0) disables light casting
                rays_color    ; // rays  color                 ,     (0,0,0)..(1,1,1)      , default=(0.05, 0.05, 0.05)
      Vec4     image_color    ; // image color                 ,   (0,0,0,0)..(1,1,1,1)    , default=(1, 1, 1, 1)
      ImagePtr image          ; // image                                                   , default=null

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Sun() {reset();}
   };

   Ambient ambient;
   Bloom   bloom  ;
   Clouds  clouds ;
   Fog     fog    ;
   Sky     sky    ;
   Sun     sun    ;

   // set / get
   void set  ()C; // apply these settings to graphics                     (this will call 'set'   on all members of this class)
   void get  () ; // get current graphics settings and store them in self (this will call 'get'   on all members of this class)
   void reset() ; // reset to default values                              (this will call 'reset' on all members of this class)

   // io
   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   Bool save(C Str &name)C; // save to   file, false on fail
   Bool load(C Str &name) ; // load from file, false on fail
};
/******************************************************************************/
DECLARE_CACHE(Environment, Environments, EnvironmentPtr); // 'Environments' cache storing 'Environment' objects which can be accessed by 'EnvironmentPtr' pointer
/******************************************************************************/
