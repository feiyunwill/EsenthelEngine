/******************************************************************************

   Use 'LightDir'   to add a directional light onto the scene
   Use 'LightPoint' to add a point       light onto the scene
   Use 'LightSqr'   to add a point       light onto the scene (with small range)
   Use 'LightCone'  to add a cone        light onto the scene

   Access 'CurrentLight' to gain informations about the current light which is being applied.

/******************************************************************************/
enum LIGHT_TYPE : Byte // Light Type
{
   LIGHT_NONE , // none
   LIGHT_DIR  , // directional
   LIGHT_POINT, // point
   LIGHT_SQR  , // point with small range
   LIGHT_CONE , // cone
};
/******************************************************************************/
struct LightDir // Directional Light
{
   Vec dir         , // direction          , normalized vector
       color       ; // color              , (0,0,0) .. (1,1,1)
   Flt vol         , // volumetric amount  , (0..1  )
       vol_exponent, // volumetric exponent, (0..Inf)
       vol_steam   ; // volumetric steam   , (0..1  )

   void add(Bool shadow=true, CPtr light_src=null); // add light to scene, this needs to be called only in RM_PREPARE mode, 'shadow'=if shadowing enabled, 'light_src'=custom pointer to light source (which can be later accessed from "CurrentLight.src")
   void set(                                     ); // use only outside Renderer rendering, before drawing any shade'able meshes

   LightDir() {}
   LightDir(C Vec &dir, C Vec &color=Vec(1, 1, 1), Flt vol=0, Flt vol_exponent=1, Flt vol_steam=0.5f) {T.dir=dir; T.color=color; T.vol=vol; T.vol_exponent=vol_exponent; T.vol_steam=vol_steam;}

#if EE_PRIVATE
   Bool toScreenRect(Rect &rect)C {rect=D.viewRect(); return true;}
#endif
};
/******************************************************************************/
struct LightPoint // Point Light
{
   Flt  power  , // power             , (0..Inf), determines light range
        vol    , // volumetric amount , (0..Inf)
        vol_max; // volumetric maximum, (0..1  )
   VecD pos    ; // position          ,
   Vec  color  ; // color             , (0,0,0) .. (1,1,1)

   Flt  range(                                            )C; // get affected range according to light's 'power'
   void add  (Flt shadow_opacity=1.0f, CPtr light_src=null) ; // add light to scene, this needs to be called only in RM_PREPARE mode, 'shadow_opacity'=opacity of shadows (0..1) where value 0 disables shadowing, value 1 sets full shadows, and values between allow for manual blending the shadows, 'light_src'=custom pointer to light source (which can be later accessed from "CurrentLight.src")

   LightPoint() {}
   LightPoint(Flt power, C VecD &pos, C Vec &color=Vec(1, 1, 1), Flt vol=0, Flt vol_max=0.5f) {T.power=power; T.pos=pos; T.color=color; T.vol=vol; T.vol_max=vol_max;}

#if EE_PRIVATE
   void  set(Flt shadow_opacity);
   BallM asBall()C {return BallM(range(), pos);}
   Bool  toScreenRect(Rect &rect)C {return ToFullScreenRect(asBall(), rect);}
#endif
};
/******************************************************************************/
struct LightSqr // Point Light with small range
{
   Flt  range  , // range             , (0..Inf)
        vol    , // volumetric amount , (0..Inf)
        vol_max; // volumetric maximum, (0..1  )
   VecD pos    ; // position          ,
   Vec  color  ; // color             , (0,0,0) .. (1,1,1)

   void add(Flt shadow_opacity=1.0f, CPtr light_src=null); // add light to scene, this needs to be called only in RM_PREPARE mode, 'shadow_opacity'=opacity of shadows (0..1) where value 0 disables shadowing, value 1 sets full shadows, and values between allow for manual blending the shadows, 'light_src'=custom pointer to light source (which can be later accessed from "CurrentLight.src")

   LightSqr() {}
   LightSqr(Flt range, C VecD &pos, C Vec &color=Vec(1, 1, 1), Flt vol=0, Flt vol_max=0.5f) {T.range=range; T.pos=pos; T.color=color; T.vol=vol; T.vol_max=vol_max;}

#if EE_PRIVATE
   void  set(Flt shadow_opacity);
   BallM asBall()C {return BallM(range, pos);}
   Bool  toScreenRect(Rect &rect)C {return ToFullScreenRect(asBall(), rect);}
#endif
};
/******************************************************************************/
struct LightCone // Cone Light
{
   Flt      falloff, // light falloff     , (0..1  ), default=0.5
            vol    , // volumetric amount , (0..Inf)
            vol_max; // volumetric maximum, (0..1  )
   Vec      color  ; // color             , (0,0,0) .. (1,1,1)
   PyramidM pyramid; // pyramid           , determines orientation of the light

   void add(Flt shadow_opacity=1.0f, CPtr light_src=null, Image *image=null, Flt image_scale=1, C Color &image_add=TRANSPARENT, Flt image_specular=0); // add light to scene, this needs to be called only in RM_PREPARE mode, 'shadow_opacity'=opacity of shadows (0..1) where value 0 disables shadowing, value 1 sets full shadows, and values between allow for manual blending the shadows, 'light_src'=custom pointer to light source (which can be later accessed from "CurrentLight.src"), 'image'=dynamic lightmap, 'image_add'=add color to dynamic lightmap, 'image_scale'=scale dynamic lightmap, 'image_specular'=specular of dynamic lightmap

   LightCone() {}
   LightCone(Flt length, C VecD &pos, C Vec &dir, C Vec &color=Vec(1, 1, 1), Flt vol=0, Flt vol_max=0.5f);

#if EE_PRIVATE
   void set(Flt shadow_opacity);
   Bool toScreenRect(Rect &rect)C {return ToFullScreenRect(pyramid, rect);}
#endif
};
/******************************************************************************/
struct Light
{
   LIGHT_TYPE type          ; // light type
   Bool       shadow        ; // if shadowing enabled
   Flt        shadow_opacity; // opacity of shadows
   Color      image_add     ; // dynamic lightmap color add
   Flt        image_specular, // dynamic lightmap specular
              image_scale   ; // dynamic lightmap scale
   Rect       rect          ; // on screen rectangle affected by light
   CPtr       src           ; // custom pointer to light source
   Image     *image         ; // dynamic lightmap
   union
   {
      LightDir   dir  ; // directional light, valid when "type==LIGHT_DIR"
      LightPoint point; // point       light, valid when "type==LIGHT_POINT"
      LightSqr   sqr  ; // point       light, valid when "type==LIGHT_SQR"
      LightCone  cone ; // cone        light, valid when "type==LIGHT_CONE"
   };

   // get / set
   Flt  range()C; // get light range    (this is equal to 0       for directional lights)
   Flt  vol  ()C; // get light volumetric amount
   VecD pos  ()C; // get light position (this is equal to (0,0,0) for directional lights)

   Light() {} // needed because of union

#if EE_PRIVATE
   Bool toScreenRect(Rect &rect)C;

   void scalePower(Flt scale);

   void set();

   void set(LightDir   &light,               Bool shadow        , CPtr light_src);
   void set(LightPoint &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src);
   void set(LightSqr   &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src);
   void set(LightCone  &light, C Rect &rect, Flt  shadow_opacity, CPtr light_src);

   void draw       ();
   void drawForward(Image *dest, ALPHA_MODE alpha);
#endif
}extern
   CurrentLight; // this contains information about the light which is currently rendered
/******************************************************************************/
#if EE_PRIVATE
#define SHADOW_MAP_DIR_RANGE_MUL 8

extern Memc<Light> Lights;

void  ShutLight ();
void  InitLight ();
void LimitLights();
void  SortLights();
void  DrawLights();
#endif
/******************************************************************************/
