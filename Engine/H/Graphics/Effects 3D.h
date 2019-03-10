/******************************************************************************

   This is a set of special effects.

/******************************************************************************/
struct Decal // Decal
{
   Bool   terrain_only; // if affect only the terrain mesh, true/false, default=false
   Vec4   color       ; // decal color                                , default=Vec4(1, 1, 1, 1)
   Matrix matrix      ; // decal matrix                               , default=MatrixIdentity

   // get / set
   Decal& material (C MaterialPtr &material);   C MaterialPtr& material()C {return _material;} // set/get material
   Decal& setMatrix(C Matrix &object_world_matrix, C Matrix &decal_world_matrix);              // set     matrix to be used as animated matrix from current 'object_world_matrix' and desired 'decal_world_matrix' 
#if EE_PRIVATE
   void   setShader();
   void   zero     ();
   void   del      ();
#endif

   // draw
   void drawStatic  (                               Flt alpha=1)C; // draw as static decal         , 'alpha'=opacity, automatically uses Frustum culling, can be called in following render modes : RM_OVERLAY, RM_BLEND, RM_PALETTE, RM_PALETTE1
   void drawAnimated(C Matrix &object_world_matrix, Flt alpha=1)C; // draw with given object matrix, 'alpha'=opacity, automatically uses Frustum culling, can be called in following render modes : RM_OVERLAY, RM_BLEND, RM_PALETTE, RM_PALETTE1

   // io
   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

   Decal();

private:
   MaterialPtr _material;
   Shader     *_shader[2][2]; // [F][P] F-Fullscreen, P-Palette
};
/******************************************************************************/
struct ElectricityFx // Electricity Effect
{
   Color     color          , // color                                       , default=Color(64, 96, 176, 255), here alpha is the glow amount
             middle_color   ; // color rendered in the middle of lines       , default=WHITE                  , here alpha is the glow amount
   Flt       middle_exponent, // middle color exponent                       , default=5
             radius         , // 3d line radius                              , default=0.008 m
             random_step    , // distance between newly created random points, default=0.1   m
             random_radius  , // how far points can be randomly moved        , default=0.1   m
             frequency      ; // time intervals between position updates     , default=0.03  s
   Memc<Vec> original       ; // original control points

   void update();
   void draw  (); // draw using active matrix, this should be called in RM_SOLID and RM_AMBIENT modes

   ElectricityFx();

private:
   Flt       time;
   Memc<Vec> randomized;
};
/******************************************************************************/
struct BlendObject // extendable class for depth-sorted rendering of graphics in RM_BLEND rendering mode
{
   void scheduleDrawBlend(C VecD &pos); // call this method inside RM_PREPARE rendering mode to schedule automatic call to 'drawBlend' method, 'pos'=object position (used for correct depth-sorting calculation)

   virtual void drawBlend() {} // extend this method to draw custom graphics for RM_BLEND rendering mode, this method will be automatically called inside RM_BLEND rendering mode if 'scheduleDrawBlend' was called previously
};
/******************************************************************************/
void DrawLaser(C Color &color, C Color &middle_color, Flt middle_exponent, Flt radius, Bool sharp_ending, C MemPtr<Vec> &points); // draw laser effect using active matrix onto the screen, 'points'=array of laser points, this should be called in RM_SOLID and RM_AMBIENT modes
/******************************************************************************/
void DrawVelocityBlur(Flt power, C Ball &ball); // draw velocity blur to the screen, this is used for blurring explosion effects when using motion blur, this can be called only in RM_BLEND mode
/******************************************************************************/
