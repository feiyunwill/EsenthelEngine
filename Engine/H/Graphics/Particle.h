/******************************************************************************

   Use 'Particles' for simplified particles management (updating and drawing).

   Use 'RawParticles' for batched drawing of thousands of particles that are updated manually.

   Use 'DrawParticle' functions for drawing each particle manually.

/******************************************************************************/
enum PARTICLE_SRC : Byte // Particles Source Type
{
   PARTICLE_NONE                 , // none
   PARTICLE_STATIC_SHAPE         , //  static shape                    , 'Particles.shape' transformed by 'Particles.matrix'
   PARTICLE_DYNAMIC_SHAPE        , // dynamic shape                    , 'Shape'
   PARTICLE_DYNAMIC_SHAPES       , // dynamic shapes                   , 'Shape' array
   PARTICLE_DYNAMIC_ORIENTP      , // dynamic point                    , 'OrientP'
   PARTICLE_DYNAMIC_SKELETON     , // dynamic skeleton                 , 'AnimatedSkeleton'
   PARTICLE_DYNAMIC_MESH         , // dynamic mesh                     , 'Mesh' transformed by 'Particles.matrix'
   PARTICLE_DYNAMIC_MESH_SKELETON, // dynamic mesh animated by skeleton, 'Mesh' transformed by 'AnimatedSkeleton'
};
/******************************************************************************/
struct Particle // Single Particle
{
   Byte palette_y  , // palette y coordinate for 'Particles.palette_image'
        image_index; // image index for animated particle images, used when "image_speed<=0 && (image_x_frames>1 || image_y_frames>1)"
   Flt  life       , // current life
        life_max   , // maximum life
        radius     , // radius
        ang_vel    ; // angular velocity
   Vec  pos        , // position
        vel        ; // velocity

   Particle() {Zero(T);}
};
/******************************************************************************/
struct Particles // Set of Particles
{
   Mems<Particle> p; // particles

   Bool  reborn              , // if reborn particles after their death                                                , false/true, default=true
         smooth_fade         , // false for (quick fade-in and slow fade-out) or true for (smooth fade-in and fade-out), false/true, default=false (this is ignored if 'opacity_func' is specified)
         motion_affects_alpha; // if motion stretching affects opacity                                                 , false/true, default=true
   Byte  glow                ; // glow amount                                                                          ,    0..255 , default=0     (this is valid only for particles rendered in RM_BLEND mode when 'palette'=false)
   Color color               ; // color                                                                                ,           ,               (this is used when the color table isn't set)

   Flt radius       , // Particle radius at its creation time                 , 0..Inf,           (this is the  initial value of 'Particle.radius' at its creation time)
       radius_random, // Particle radius random factor                        , 0..Inf, default=0 (this affects initial value of 'Particle.radius', 0 value keeps it constant, 1 value can make it 2x smaller to 2x bigger, 2 value can make it 3x smaller to 3x bigger)
       radius_growth, // Particle radius growth factor                        , 0..Inf, default=1 (this affects 'Particle.radius' in each frame, where a value of 0..1 decreases the radius, 1 keeps it constant, and 1..Inf increases the radius)
       offset_range , // Particle position offset range applied during drawing, 0..Inf, default=0 (this simulates the range of random position offsets for each particle during drawing, value of 0 is the fastest and disables the offset usage)
       offset_speed , // Particle position offset speed applied during drawing, 0..Inf, default=1 (this simulates the speed of random potision offsets for each particle during drawing)
       life         , // Particle life                                        , 0..Inf,           (this is the life of a single particle 'Particle.max_life')
       life_random  ; // Particle life random factor                          , 0..Inf,           (this affects 'Particle.max_life', 0 value keeps it constant, 1 value can make it 2x shorter to 2x longer, 2 value can make it 3x shorter to 3x longer)

   Flt glue        , // moves particles along with the source movement,                0..1            , default=0    (this affects each single particle position according to source generation object movement, value of 0 disables position adjustment and doesn't require any additional calculations)
       damping     , // Particle velocity damping                     ,                0..1            , default=0    (this dampens particles velocity in each frame)
       ang_vel     , // angular  velocity range                       ,                0..Inf          , default=0
       vel_random  ; // random   velocity applied at particle creation,                0..Inf          , default=0
   Vec vel_constant, // constant velocity applied at particle creation, (-Inf,-Inf,-Inf)..(Inf,Inf,Inf), default=(0,0,0)
       accel       ; // acceleration      applied each frame          , (-Inf,-Inf,-Inf)..(Inf,Inf,Inf), default=(0,0,0)

   Flt emitter_life_max , // maximum life of the Particles Emitter              ,    0..Inf             , default=0 (this specifies  the maximum life of the Particles Set, value <=0 specifies infinite life, value >0 specifies finite life)
       emitter_life     , // current life of the Particles Emitter              ,    0..emitter_life_max, default=0 (this specifies  the current life of the Particles Set, this value is increased only if the Particles Set has finite life)
       fade_in          , // fade in  time                                      ,    0..Inf             , default=1 (this affects    the global  opacity of all particles , this value is used      only if the Particles Set has finite life)
       fade_out         , // fade out time                                      ,    0..Inf             , default=1 (this affects    the global  opacity of all particles , this value is used      only if the Particles Set has finite life)
       radius_scale_base, // Particle radius global scale applied during drawing, -Inf..Inf             , default=1 (this is the base          global scale of all particles radius applied during drawing only) final particle radius value = "Particle.radius * radiusScale()"
       radius_scale_time; // Particle radius global scale applied during drawing, -Inf..Inf             , default=0 (this is the time relative global scale of all particles radius applied during drawing only) final particle radius value = "Particle.radius * radiusScale()"

   Matrix matrix      ; // particle generation matrix                          ,           , default=MatrixIdentity (this matrix is used for following source types: PARTICLE_STATIC_SHAPE, PARTICLE_DYNAMIC_MESH)
   Shape  shape       ; // particle generation shape                           ,           , default=Ball(1)        (this shape  is used for following source types: PARTICLE_STATIC_SHAPE)
   Bool   inside_shape; // if generate particles inside shape or on its surface, false/true, default=true

   ImagePtr image         ; // particle image
   UShort   image_x_frames, // number of animation frames in image width , 1..65535, default=1
            image_y_frames; // number of animation frames in image height, 1..65535, default=1
   Flt      image_speed   ; // image     animation speed                 , 0..Inf  , default=1

   ImagePtr palette_image; // if specified, then particle colors will be multiplied by this image, x coordinate of the image is based on particle life, y coordinate of the image is taken from 'Particle.palette_y', the mode of this image must be equal to IMAGE_SOFT (to avoid unnecessary locking)

   Flt hard_depth_offset; // 0..1, default=0, this value is used when particles are not drawn with softing (D.particlesSoft is false or not supported), in that case each particle will be offsetted towards the camera by its radius multiplied by this factor, decreasing the chance of particles being occluded

   Flt (*opacity_func)(Flt life_frac); // pointer to a custom function (may be null) used to calculate opacity of a single particle, based on its life, if this is null then a default function is used based on 'smooth_fade', this member is not saved in 'save/load' methods, default=null

   // set / get
   Flt         fade       (             )C {return _fade                                               ;} // get current fading value according to 'fade_in fade_out emitter_life_max', this should be used for accessing the fade value of particle sets with finite life (emitter_life_max>0)
   Flt         opacity    (Vec *pos=null)C;                                                               // get average particles opacity (0..1) and position (this can be useful for example in applying lights basing on particles average opacity and position)
   Flt         radiusScale(             )C {return emitter_life*radius_scale_time + radius_scale_base  ;} // get current radius scale
   Bool        alive      (             )C {return emitter_life_max<=0 || emitter_life<emitter_life_max;} // if  Particle Set is  alive
   Bool        is         (             )C {return _src_type!=PARTICLE_NONE                            ;} // if  Particle Set was created
   RENDER_MODE renderMode (             )C;                                                               // get RENDER_MODE in which Particle Set should be drawn (can be RM_BLEND, RM_PALETTE or RM_PALETTE1)

   Particles& palette     (Bool palette);   Bool palette     ()C {return _palette      ;} // if draw particles in palette mode (RM_PALETTE or RM_PALETTE1) instead of blend mode (RM_BLEND)        , false/true, default=false (particles in palette mode use only Alpha component from the Texture, while blend mode uses all RGBA channels from the Texture)
   Particles& paletteIndex(Byte index  );   Byte paletteIndex()C {return _palette_index;} // specifies which      palette mode (RM_PALETTE or RM_PALETTE1) should be used when 'palette' is enabled,    0..1   , defailt=0     (if 'palette' is enabled then this will specify whether RM_PALETTE index=0 or RM_PALETTE1 index=1 should be used)

   // manage
   Particles& del   (                                                                  ); // delete
   Particles& create(C ImagePtr  &image, C Color &color, Int elms, Flt radius, Flt life); // create from image object
   Particles& create(C Particles &src                                                  ); // create from 'src'

   // particle generation source
   PARTICLE_SRC sourceType(                                                                          )C {return _src_type;} // get particle generation source type
   Particles&   source    (C Shape            & static_shape                                         );                     // set particle generation source to a  static shape                     (this  shape    will     be copied to the Particles object, and can be modified manually via 'Particles.shape')
   Particles&   source    (C Shape            *dynamic_shape                                         );                     // set particle generation source to a dynamic shape                     (this  shape    will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_shape   '                   , you'll need to make sure that the memory address of 'dynamic_shape'                       will remain constant throughout the life of Particles)
   Particles&   source    (C Shape            *dynamic_shapes  , Int  shapes                         );                     // set particle generation source to   dynamic shapes                    (these shapes   will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_shapes  '                   , you'll need to make sure that the memory address of 'dynamic_shapes'                      will remain constant throughout the life of Particles)
   Particles&   source    (C OrientP          *dynamic_point                                         );                     // set particle generation source to a dynamic point                     (this  point    will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_point   '                   , you'll need to make sure that the memory address of 'dynamic_point'                       will remain constant throughout the life of Particles)
   Particles&   source    (C AnimatedSkeleton *dynamic_skeleton, Bool ragdoll_bones_only             );                     // set particle generation source to a dynamic skeleton                  (this  skeleton will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_skeleton'                   , you'll need to make sure that the memory address of 'dynamic_skeleton'                    will remain constant throughout the life of Particles)
   Particles&   source    (C MeshPtr          &dynamic_mesh                                          );                     // set particle generation source to a dynamic mesh                      (this  mesh     will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_mesh    '                   , you'll need to make sure that the memory address of 'dynamic_mesh'                        will remain constant throughout the life of Particles)
   Particles&   source    (C MeshPtr          &dynamic_mesh    , C AnimatedSkeleton *dynamic_skeleton);                     // set particle generation source to a dynamic mesh animated by skeleton (these objects  will not be copied to the Particles object, which means that the Particles object will access each frame the memory address of 'dynamic_mesh' and 'dynamic_skeleton', you'll need to make sure that the memory address of 'dynamic_mesh' and 'dynamic_skeleton' will remain constant throughout the life of Particles)

   // operations
#if EE_PRIVATE
   Particles& setRenderMode(); // set _render_mode according to current particle settings
   void       zero         ();
#endif
   void       reset    (Int i          ); // reset i-th particle                             , if 'reborn' is false then the particle will be set as dead
   Particles& reset    (               ); // reset  all particles using 'reset(Int i)' method, if 'reborn' is false then the particle will be set as dead
   Particles& resetFull(               ); // reset 'emitter_life' and all particles          , this method ignores 'reborn' and always sets particles to alive
   Bool       update   (Flt dt=Time.d()); // update all particles, returns false if Particles Set has died "!alive()", true if it's still alive (or has infinite life), you can use this information to delete the particles if they're no longer needed, 'dt'=time delta used for updating the particles

   // draw
   void draw(Flt opacity=1)C; // draw, 'opacity'=custom opacity multiplier, this method should be called only in RM_PALETTE, RM_PALETTE1 and RM_BLEND rendering modes, doesn't use automatic Frustum culling

   // io
   Bool save(  File &f   , Bool include_particles, CChar *path=null)C; // save, does not include  saving dynamic sources, false on fail, 'include_particles'=if include each single particle data in saving (if not, then they will be set randomly when loading), 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path)
   Bool load(  File &f                           , CChar *path=null) ; // load, does not include loading dynamic sources, false on fail                                                                                                                          , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path)
   Bool save(C Str  &name, Bool include_particles                  )C; // save, does not include  saving dynamic sources, false on fail, 'include_particles'=if include each single particle data in saving (if not, then they will be set randomly when loading)
   Bool load(C Str  &name                                          ) ; // load, does not include loading dynamic sources, false on fail

  ~Particles() {del();}
   Particles();

#if !EE_PRIVATE
private:
#endif
   Bool         _palette      ;
   Byte         _palette_index;
   PARTICLE_SRC _src_type     ;
   Flt          _fade         ;
   RENDER_MODE  _render_mode  ;
   CPtr         _src_ptr      ; Int _src_elms;
   MeshPtr      _src_mesh     ;
   Byte        *_src_help     ;
   Matrix       _matrix_prev  ;
   NO_COPY_CONSTRUCTOR(Particles);
};
/******************************************************************************/
struct RawParticles // buffered particles
{
   struct Particle
   {
      Vec   pos   , // position
            vel   ; // velocity
      Color color ; // color
      Flt   radius, // radius
            angle ; // angle
   };

   Bool     motion_affects_alpha; // if motion stretching affects opacity, default=true
   Byte     glow                ; // glow amount, 0..255, default=0 (this is valid only for particles rendered in RM_BLEND mode when 'palette'=false)
   ImagePtr image               ; // particle image

   // manage
   RawParticles& del   (                 ); // delete manually
   RawParticles& create(C ImagePtr &image); // create with 'image'

   // get / set
   RENDER_MODE renderMode()C; // get RENDER_MODE in which particles should be drawn (can be RM_BLEND, RM_PALETTE or RM_PALETTE1)
#if EE_PRIVATE
   RawParticles& setRenderMode();
   void          zero         ();
#endif

   RawParticles& palette     (Bool palette);   Bool palette     ()C {return _palette      ;} // if draw particles in palette mode (RM_PALETTE or RM_PALETTE1) instead of blend mode (RM_BLEND)        , false/true, default=false (particles in palette mode use only Alpha component from the Texture, while blend mode uses all RGBA channels from the Texture)
   RawParticles& paletteIndex(Byte index  );   Byte paletteIndex()C {return _palette_index;} // specifies which      palette mode (RM_PALETTE or RM_PALETTE1) should be used when 'palette' is enabled,    0..1   , defailt=0     (if 'palette' is enabled then this will specify whether RM_PALETTE index=0 or RM_PALETTE1 index=1 should be used)

   // operations
   RawParticles& set(C Particle *particle, Int particles); // set from 'particle' array of 'particles'

   // draw
   void draw()C; // draw, this method should be called only in RM_PALETTE, RM_PALETTE1 and RM_BLEND rendering modes, doesn't use automatic Frustum culling

  ~RawParticles() {del();}
   RawParticles();

private:
   Bool        _palette;
   Byte        _palette_index;
   Int         _particles, _max_particles;
   RENDER_MODE _render_mode;
   VtxBuf      _vb;
   IndBuf      _ib;
};
/******************************************************************************/
// manually draw a set of particles
Bool DrawParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha                      ); // call this at start of drawing particles, 'glow'=glow amount 0..255, 'motion_affects_alpha'=if motion stretching affects opacity, false on fail, this function should be called only in RM_PALETTE, RM_PALETTE1 and RM_BLEND rendering modes
void DrawParticleAdd  (C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel); // call this repeatedly for each particle
void DrawParticleEnd  (                                                                          ); // call this after drawing  all  particles

inline void DrawParticle(C Image &image, Byte glow, C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel) // draw a single particle
{
   if(DrawParticleBegin(image, glow, true))
   {
      DrawParticleAdd(color, opacity, radius, angle, pos, vel);
      DrawParticleEnd();
   }
}

// manually draw a set of animated particles
Bool DrawAnimatedParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha, Int x_frames, Int y_frames     ); // call this at start of drawing animated particles, 'glow'=glow amount 0..255, 'motion_affects_alpha'=if motion stretching affects opacity, 'x_frames'=number of frames in image width, 'y_frames'=number of frames in image height, false on fail, this function should be called only in RM_PALETTE, RM_PALETTE1 and RM_BLEND rendering modes
void DrawAnimatedParticleAdd  (C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel, Flt frame); // call this repeatedly for each particle, 'frame'=current frame (0..1)
void DrawAnimatedParticleEnd  (                                                                                     ); // call this after drawing  all  particles

Flt ParticleOpacity(Flt particle_life, Flt particle_life_max, Bool particles_smooth_fade);   // calculate opacity of a single particle by specifying its current life 'Particle.life', maximum life 'Particle.life_max' and if smooth fade 'Particles.smooth_fade' is enabled
/******************************************************************************/
extern Cache<Particles> ParticlesCache; // Particles Cache

inline Int Elms(C Particles &particles) {return particles.p.elms();}
/******************************************************************************/
#if EE_PRIVATE
void ShutParticles();
#endif
/******************************************************************************/
