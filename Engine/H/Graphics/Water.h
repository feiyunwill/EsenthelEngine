/******************************************************************************/
#if EE_PRIVATE
#define WATER_SMOOTH 0.1f
#endif
/******************************************************************************/
struct WaterMtrl // Water Material
{
   Flt density               , //                 density                       ,       0..1      , default=0.3
       density_add           , // factor added to density                       ,       0..1      , default=0.45
       density_underwater    , //                 density when viewed underwater,       0..1      , default=0.02
       density_underwater_add, // factor added to density when viewed underwater,       0..1      , default=0.6
       scale_color           , // scale color  texture                          ,       0..Inf    , default=200
       scale_normal          , // scale normal texture                          ,       0..Inf    , default=10
       scale_bump            , // scale bump   texture                          ,       0..Inf    , default=100
       rough                 , // roughness                                     ,       0..Inf    , default=1
       reflect_tex           , // reflection power of texture                   ,       0..1      , default=0.1
       reflect_world         , // reflection power of world                     ,       0..1      , default=0.18 (0 is the fastest)
       refract               , // refraction power                              ,       0..1      , default=0.10 (0 is the fastest)
       refract_reflection    , // refraction of the reflection                  ,       0..1      , default=0.06
       refract_underwater    , // refraction when viewed underwater             ,       0..1      , default=0.01 (0 is the fastest)
       specular              , // specular amount                               ,       0..Inf    , default=1.5
       wave_scale            , // vertical wave scale                           ,       0..1      , default=0.25 (0 is the fastest)
       fresnel_pow           , // fresnel term power                            ,       0..Inf    , default=5.5
       fresnel_rough         ; // fresnel term roughness                        ,       0..Inf    , default=4
   Vec fresnel_color         , // fresnel term color                            , (0,0,0)..(1,1,1), default=(0.10, 0.10, 0.10)
       color                 , // color                                         , (0,0,0)..(1,1,1), default=(0.42, 0.50, 0.58)
       color_underwater0     , // color when viewed underwater on surface       , (0,0,0)..(1,1,1), default=(0.26, 0.35, 0.42)
       color_underwater1     ; // color when viewed underwater deep             , (0,0,0)..(1,1,1), default=(0.10, 0.20, 0.30)

   // set / get
   WaterMtrl&      colorMap(C ImagePtr &image);   C ImagePtr&      colorMap()C {return _color_map     ;} // set/get color      map
   WaterMtrl&     normalMap(C ImagePtr &image);   C ImagePtr&     normalMap()C {return _normal_map    ;} // set/get normal     map
   WaterMtrl& reflectionMap(C ImagePtr &image);   C ImagePtr& reflectionMap()C {return _reflection_map;} // set/get reflection map

   // operations
   WaterMtrl& reset   (); // reset to default values
   WaterMtrl& validate(); // this needs to be called after manually changing the parameters/textures
#if EE_PRIVATE
   void set();
#endif

   // io
   Bool save(C Str &name)C; // save, false on fail
   Bool load(C Str &name) ; // load, false on fail

   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   WaterMtrl();

#if !EE_PRIVATE
private:
#endif
   ImagePtr _color_map, _normal_map, _reflection_map, _bump;
};
/******************************************************************************/
DECLARE_CACHE(WaterMtrl, WaterMtrls, WaterMtrlPtr); // Water Material Cache
#if EE_PRIVATE
extern WaterMtrl   *WaterMtrlLast; // Last set Water Material
extern WaterMtrlPtr WaterMtrlNull;
#endif
/******************************************************************************/
STRUCT(WaterClass , WaterMtrl) // Main water control
//{
   Bool   draw                 , // if draw the water plane      , true/false, default=false                  , it can be drawn only after setting 'draw' to true, and setting valid water images in parameters
          reflection_allow     , // if allow rendering reflection, true/false, default=true (false for Mobile)
          reflection_shadows   ; // if use shadows in reflection , true/false, default=false                  , to have shadows in reflection you must also enable global shadowing, and set reflection renderer != RT_SIMPLE
   Byte   reflection_resolution; // reflection resolution        ,    0..4   , default=2                      , the bigger value the worse quality but faster rendering
   PlaneM plane                ; // water plane                  ,           , default=(pos(0,0,0), normal(0,1,0))

   WaterClass& reflectionRenderer(RENDER_TYPE type);   RENDER_TYPE reflectionRenderer()C {return _reflect_renderer;} // set/get Renderer used for rendering the reflaction , default=RT_SIMPLE
   WaterClass& max1Light         (Bool          on);   Bool        max1Light         ()C {return _max_1_light     ;} // set/get if use only up to 1 light for water surface, default=true  (this greatly increases water rendering performance, however allows only 1 directional light affecting water surface), this affects only RT_DEFERRED renderer, all other renderers are always limited to only 1 directional light

   WaterClass& update(C Vec2 &vel); // update wave movement, 'vel'=velocity

#if EE_PRIVATE
   enum MODE : Byte
   {
      MODE_DRAW      ,
      MODE_REFLECTION,
      MODE_UNDERWATER,
   };

   Bool    ocean ();
   Shader* shader();

   void del           ();
   void create        ();
   void prepare       ();
   void begin         (); // this is called just before       WaterPlane and WaterMesh drawing
   void end           (); // this is called at the end of all WaterPlane and WaterMesh drawing
   void under         (C PlaneM &plane, WaterMtrl &mtrl); // set if camera is under custom water plane
   void setImages     (Image *src, Image *depth);
   void endImages     ();
   void drawSurfaces  ();
   void setEyeViewport();
#endif

#if !EE_PRIVATE
private:
#endif
   Bool         _max_1_light, _draw_plane_surface, _use_secondary_rt, _began, _swapped_ds;
   Byte         _mode, _shader_shadow, _shader_soft;
   Flt          _under_step;
   Vec2         _offset, _y_mul_add;
   Quad         _quad;
   PlaneM       _under_plane;
   MeshRender   _mshr;
   Image        _bump_temp;
   WaterMtrlPtr _under_mtrl;
   RENDER_TYPE  _reflect_renderer;

   WaterClass();
}extern
   Water; // water
/******************************************************************************/
struct WaterMesh // manually specified water mesh, water meshes don't support wave scales and custom density on surface (density is taken from 'Water')
{
   Flt depth; // water depth, default=3

   // get
   Bool          is      (                           )C {return _mshb.is();} // if created
   Bool          lake    (                           )C {return _lake     ;} // if it's a lake
   Bool          under   (C Vec &pos, Flt *depth=null)C;                     // if 'pos' position is under water, 'depth'=optional parameter to receive the depth of position when it is under water
 C MeshBase    & mesh    (                           )C {return _mshb     ;} // get water mesh
 C Box         & box     (                           )C {return _box      ;} // get water box (not including depth)
 C WaterMtrlPtr& material(                           )C {return _material ;} // get water material
#if EE_PRIVATE
   Shader      * shader     ()C;
   WaterMtrl   * getMaterial()C {return _material ? _material() : &Water;}
   void          zero();
#endif

   // manage
   void del();
   void create(C MeshBase &src, Bool lake, Flt depth=3, C WaterMtrlPtr &material=null); // create from 'src' mesh

   // draw
   void draw()C; // draw water area, this should be called only in RM_WATER mode, automatically uses frustum culling

   // io
   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   WaterMesh();

#if !EE_PRIVATE
private:
#endif
   Bool         _lake    ;
   Box          _box     ;
   MeshBase     _mshb    ;
   MeshRender   _mshr    ;
   WaterMtrlPtr _material;
};
/******************************************************************************/
