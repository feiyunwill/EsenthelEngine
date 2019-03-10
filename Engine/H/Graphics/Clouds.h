/******************************************************************************/
struct LayeredClouds
{
   struct Layer // Cloud Layer
   {
      Color    color   ; // texture color   ,          , default=WHITE
      Flt      scale   ; // texture scale   ,    0..Inf, default={0.35, 0.41, 0.50, 0.62}
      Vec2     position, // texture position, -Inf..Inf,
               velocity; // texture velocity, -Inf..Inf, default={0.010, 0.008, 0.006, 0.004}
      ImagePtr image   ; // texture
   };

   Bool  merge_with_sky, // if draw clouds together with the sky, this can result in better performance, but supports only up to 1 layer of clouds and Sky.frac() must be set to 1, this also results in drawing astronomical objects on top of clouds, default=false (true for Mobile)
         draw_in_mirror; // if draw clouds in mirrors, such as water reflections, default=false
   Layer layer[4]      ; // layer array

   // set / get
   LayeredClouds& set            (Byte active_layers, C ImagePtr &image=null);   Int  layers         ()C {return _layers ;} // set/get number of active layers 0..4, if 'image'!=null layers will have 'image' set as their texture
   LayeredClouds& frac           (Flt  frac                                 );   Flt  frac           ()C {return _frac   ;} // set/get cloud viewport fraction  , 0..1  , default=0.9 , fraction of the Viewport range where clouds start (1 is the fastest)
   LayeredClouds& scaleY         (Flt  scale                                );   Flt  scaleY         ()C {return _scale_y;} // set/get y scaling                , 1..2  , default=1.05, setting this value higher than 1 helps covering the empty gap between flat ground and the clouds
   LayeredClouds& rayMaskContrast(Flt  contrast                             );   Flt  rayMaskContrast()C {return _rmc    ;} // set/get sun rays masking contrast, 1..Inf, default=4   , this is used when "Sun.rays_mode==SUN_RAYS_HIGH"

   void update(); // update layers, this needs to be called once per frame to update the cloud texture animation movement (move the layer texture positions according to velocities)

#if !EE_PRIVATE
private:
#endif
   Byte       _layers;
   Flt        _frac, _scale_y, _rmc;
   MeshRender _mshr;

   LayeredClouds();
#if EE_PRIVATE
   void del      ();
   void create   ();
   void commit   ();
   void draw     ();
   void shadowMap();
   Bool wantDepth()C;
#endif
};
/******************************************************************************/
struct VolumetricCloud
{
   struct Settings
   {
      // cloud opacity formula = "LerpRS(noise_min, noise_max, noise)*density"
      Byte detail     ; // number of extra details, 0..3
      Flt  density    , // density scale factor, 0..1
           noise_min  , // noise value at which a cloud is formed      (start of the cloud), -1..1, currently ignored for object clouds
           noise_max  , // noise value at which a cloud is fully dense (end   of the cloud), -1..1, currently ignored for object clouds
           brightness , // cloud brightness, 0..1
           ambient    , // ambient light   , 0..1
           light_power; // light power     , 0..1
      Vec  light_pos  ; // light position

   #if EE_PRIVATE
      void zero() {Zero(T);}
   #endif

      Settings() {detail=3; density=0.75f; noise_min=0.35f; noise_max=1.0f; brightness=0.8f; ambient=D.ambientPower(); light_power=Sun.light_color.max(); light_pos=Sun.pos;}
   };

   // get
   Int width ()C {return _image.h();}
   Int height()C {return _image.w();}
   Int depth ()C {return _image.d();}

   // manage
   void del();
   void create        (Int size , Int height,            Int frequency, const_mem_addr Threads *threads=null, UInt seed=0, Flt noise_gain=0.555f);
   void createAsObject(Int width, Int height, Int depth, Flt frequency, const_mem_addr Threads *threads=null, UInt seed=0, Flt noise_gain=0.555f);

   // operations
   void update(C Settings &settings);

   // draw
   void draw(Flt size, C VecD &pos); // draw object cloud with specified 'size' and 'pos', this should be called only for custom 'VolumetricCloud' objects, and not for the global 'Clouds.volumetric'

   VolumetricCloud();
  ~VolumetricCloud() {del();}

#if !EE_PRIVATE
private:
#endif
   struct Voxel
   {
   #if 0 // Flt - slower (37 fps) but best quality
      Flt density;
      #if EE_PRIVATE
         void set(Flt density)  {T.density=density;}
         Flt  get(           )C {return density;}
         Flt  positive(      )C {return density*0.5f+0.5f;}
      #endif
   #else // Byte - faster (48 fps) slightly lower quality (but not noticeable)
      Byte density;
      #if EE_PRIVATE
         void set(Flt density)  {T.density=SFltToUByte(density);}
         Flt  get(           )C {return    UByteToSFlt(density);}
         Flt  positive(      )C {return     ByteToFlt (density);}
      #endif
   #endif
   };
   struct Src
   {
      VecI2 dir;
      Flt   mul;
   };
   SimplexNoise _noise;
   Settings     _cur, _build;
   Bool         _build_finished, _creating, _object;
   Byte         _building, _srcs;
   Int          _pitch, _level;
   Flt          _noise_scale, _noise_gain, _noise_mul, _noise_add, _sqr_density, _dy2;
   VecI         _tile;
   Src          _src[3];
   Voxel       *_voxels;
   Flt         *_light;
   Byte        *_image_data;
   Threads     *_threads;
   Image        _image;
#if EE_PRIVATE
   Voxel& voxel(Int x, Int y, Int z) {return _voxels[y + x*_image.w() + z*_pitch];}
   void createEx(Int width, Int height, Int depth, Flt frequency, const_mem_addr Threads *threads, UInt seed, Flt noise_gain, Bool object);
   void zero();
   void setDensity();
   void setDensityRow(Int z);
   void setImageRow  (Int z);
   void build();
   void cancelCreate();
   void cancelBuild ();
   void  checkBuild ();
#endif
   NO_COPY_CONSTRUCTOR(VolumetricCloud);
};
/******************************************************************************/
struct VolumetricClouds
{
   VolumetricCloud cloud;

   Bool draw_in_mirror; // if draw clouds in mirrors, such as water reflections, default=false
   Int  res_h         ; // max resolution height for the clouds render target, 1..Inf, default=540 (1080/2)

   Flt size     , // cloud dome size in meters     ,                0..Inf          , default=100
       curve    , // cloud dome curviness          ,                0..1            , default=0.05
       tex_scale, // texture coordinates scale     ,                0..Inf          , default=0.5
       shadow   ; // shadow intensity              ,                0..1            , default=0.35
   Vec color    , // cloud color                   ,          (0,0,0)..(1,1,1)      , default=(1,1,1)
       pos      ; // world space position in meters, (-Inf,-Inf,-Inf)..(Inf,Inf,Inf), default=(0,0,0)

#if !EE_PRIVATE
private:
#endif
   VolumetricClouds();
#if EE_PRIVATE
   void del      () {cloud.del();}
   Bool drawable ()C;
   void draw     ();
   void shadowMap();
   Bool wantDepth()C {return drawable();}
#endif
};
/******************************************************************************/
struct AllClouds
{
               Bool draw;
      LayeredClouds layered;
   VolumetricClouds volumetric;

#if EE_PRIVATE
   void del      ()  {layered.del(); volumetric.del();}
   void create   ()  {draw=true; layered.create();}
   void drawAll  ();
   void shadowMap()  {layered.shadowMap(); volumetric.shadowMap();}
   Bool wantDepth()C {return draw && (layered.wantDepth() || volumetric.wantDepth());}
#endif
}extern
   Clouds;
/******************************************************************************/
