/******************************************************************************/
enum ALPHA_MODE : Byte // Alpha Blending Modes
{
   ALPHA_NONE     , // Color:set     , Alpha:set
   ALPHA_BLEND    , // Color:blend   , Alpha:increase
   ALPHA_BLEND_DEC, // Color:blend   , Alpha:decrease
   ALPHA_ADD      , // Color:add     , Alpha:add
   ALPHA_MUL      , // Color:multiply, Alpha:multiply

   ALPHA_MERGE        , // Color:blend alpha premultiplied, Alpha:increase
   ALPHA_ADDBLEND_KEEP, // Color:add blended              , Alpha:keep
   ALPHA_ADD_KEEP     , // Color:add                      , Alpha:keep
   ALPHA_BLEND_FACTOR , // Color:blend                    , Alpha:blend with factor
   ALPHA_ADD_FACTOR   , // Color:add                      , Alpha:add   with factor
#if EE_PRIVATE
   ALPHA_SETBLEND_SET  , // Color:set blended, Alpha:set
   ALPHA_FACTOR        , // Color:factor     , Alpha:factor
   ALPHA_NONE_ADD      , // Color:set        , Alpha:add
   ALPHA_INVERT        , // invert destination
   ALPHA_FONT          , // Color:clear type, Alpha:increase
   ALPHA_FONT_DEC      , // Color:clear type, Alpha:decrease
 //ALPHA_NONE_WRITE_A  , // no blending, write only to A
 //ALPHA_NONE_WRITE_RGB, // no blending, write only to RGB
 //ALPHA_NONE_COVERAGE , // no blending with Alpha To Coverage
 //ALPHA_ADD_COVERAGE  , // Color:add   with Alpha To Coverage
   ALPHA_NUM           , // number of alpha blending modes
#endif
};
/******************************************************************************/
#if EE_PRIVATE
enum BIAS_MODE : Byte
{
   BIAS_ZERO   ,
   BIAS_SHADOW ,
   BIAS_OVERLAY,
   BIAS_NUM    ,
};
enum STENCIL_MODE : Byte
{
   STENCIL_NONE          ,
   STENCIL_ALWAYS_SET    ,
   STENCIL_MSAA_SET      ,
   STENCIL_MSAA_TEST     ,
   STENCIL_EDGE_SOFT_SET ,
   STENCIL_EDGE_SOFT_TEST,
   STENCIL_TERRAIN_TEST  ,
   STENCIL_WATER_SET     ,
   STENCIL_WATER_TEST    ,
   STENCIL_OUTLINE_SET   ,
   STENCIL_OUTLINE_TEST  ,
   STENCIL_NUM           ,
};
enum STENCIL_REF : Byte
{
   STENCIL_REF_ZERO     =   0,
   STENCIL_REF_MSAA     =0x01, // Multi-Sampled Pixel
   STENCIL_REF_EDGE_SOFT=0x02, // Edge Soften
   STENCIL_REF_TERRAIN  =0x04, // Terrain (to apply decals)
   STENCIL_REF_WATER    =0x08, // Water
   STENCIL_REF_OUTLINE  =0x10, // Outline
};
#endif
struct DisplayState // Display States Control, this class methods can be called by the use of 'D' display object
{
          ALPHA_MODE alpha      (                    )C {return _alpha;} // get alpha blending mode
   static ALPHA_MODE alpha      (ALPHA_MODE alpha    );                  // set alpha blending mode and return previous mode
   static void       clip       (C Rect    *rect=null);                  // set rectangle clipping, null for none
   static void       clip       (C Rect    &rect     ) {clip(&rect);}    // set rectangle clipping
   static void       depthLock  (Bool       on       );                  // set    depth buffer usage and lock it
   static void       depthUnlock(                    );                  // unlock depth buffer usage
   static Bool       depthWrite (Bool       on       );                  // set    depth buffer writing and return previous state
   static Bool       lineSmooth (Bool       on       );                  // set line smoothing (anti-aliasing) and return previous state, enabling this option can significantly reduce performance of drawing lines, default=false

#if EE_PRIVATE
   #define COL_WRITE_R GPU_API(D3DCOLORWRITEENABLE_RED  , D3D11_COLOR_WRITE_ENABLE_RED  , 0x1)
   #define COL_WRITE_G GPU_API(D3DCOLORWRITEENABLE_GREEN, D3D11_COLOR_WRITE_ENABLE_GREEN, 0x2)
   #define COL_WRITE_B GPU_API(D3DCOLORWRITEENABLE_BLUE , D3D11_COLOR_WRITE_ENABLE_BLUE , 0x4)
   #define COL_WRITE_A GPU_API(D3DCOLORWRITEENABLE_ALPHA, D3D11_COLOR_WRITE_ENABLE_ALPHA, 0x8)

      #define FUNC_NEVER      GPU_API(D3DCMP_NEVER       , D3D11_COMPARISON_NEVER        , GL_NEVER  )
      #define FUNC_ALWAYS     GPU_API(D3DCMP_ALWAYS      , D3D11_COMPARISON_ALWAYS       , GL_ALWAYS )
   #if REVERSE_DEPTH
      #define FUNC_LESS       GPU_API(D3DCMP_GREATER     , D3D11_COMPARISON_GREATER      , GL_GREATER)
      #define FUNC_LESS_EQUAL GPU_API(D3DCMP_GREATEREQUAL, D3D11_COMPARISON_GREATER_EQUAL, GL_GEQUAL )
      #define FUNC_GREATER    GPU_API(D3DCMP_LESS        , D3D11_COMPARISON_LESS         , GL_LESS   )
   #else
      #define FUNC_LESS       GPU_API(D3DCMP_LESS        , D3D11_COMPARISON_LESS         , GL_LESS   )
      #define FUNC_LESS_EQUAL GPU_API(D3DCMP_LESSEQUAL   , D3D11_COMPARISON_LESS_EQUAL   , GL_LEQUAL )
      #define FUNC_GREATER    GPU_API(D3DCMP_GREATER     , D3D11_COMPARISON_GREATER      , GL_GREATER)
   #endif

   #define COL_WRITE_RGB  (COL_WRITE_R|COL_WRITE_G|COL_WRITE_B            )
   #define COL_WRITE_RGBA (COL_WRITE_R|COL_WRITE_G|COL_WRITE_B|COL_WRITE_A)

   #define MAX_DX9_TEXTURES             261  // D3DVERTEXTEXTURESAMPLER3+1
   #define MAX_DX9_SHADER_CONSTANT (4096+32) // memory size (not number of constants), should be 4096 (256 Vec4's), however for unknown reason some shaders use more
   #define MAX_TEXTURES                  20  // keep this low because of 'texClear'
   #define MAX_SHADER_BUFFERS            16

   void setDeviceSettings();
   void clearShader      ();
   void del              ();
   void create           ();

   static void depth        (Bool      on    );
   static void depthAllow   (Bool      on    );
   static void depthFunc    (UInt      func  );
   static void wire         (Bool      on    );
   static void cull         (Bool      on    );
   static void cullGL       (                );
   static void alphaFactor  (C Color  &factor);
   static void clipForce    (C RectI  &rect  );
   static void clipAllow    (C RectI  &rect  );
   static void clipAllow    (Bool      on    );
   static void clipPlane    (Bool      on    );
   static void clipPlane    (C PlaneM &plane );
   static void colWrite     (Byte      color_mask, Byte index=0);
   static void colWriteAllow(Byte      color_mask);
   static void sampleMask   (UInt      mask  );
   static void viewportForce(C RectI  &rect  );
   static void viewport     (C RectI  &rect, Bool allow_proj_matrix_update=true);
   static void vf           (GPU_API(IDirect3DVertexDeclaration9, ID3D11InputLayout, VtxFormatGL) *vf);
   static void texVS        (Int index, GPU_API(IDirect3DBaseTexture9*, ID3D11ShaderResourceView*, UInt) tex);
   static void texHS        (Int index, GPU_API(IDirect3DBaseTexture9*, ID3D11ShaderResourceView*, UInt) tex);
   static void texDS        (Int index, GPU_API(IDirect3DBaseTexture9*, ID3D11ShaderResourceView*, UInt) tex);
   static void texPS        (Int index, GPU_API(IDirect3DBaseTexture9*, ID3D11ShaderResourceView*, UInt) tex);
   static void texClear     (           GPU_API(IDirect3DBaseTexture9*, ID3D11ShaderResourceView*, UInt) tex);
   static void texBind      (UInt mode, UInt tex); // needs to be called on OpenGL instead of calling 'glBindTexture'
   static void bias         (BIAS_MODE    bias);
   static void stencil      (STENCIL_MODE mode);
   static void stencilRef   (Byte         ref );
   static void stencil      (STENCIL_MODE mode, Byte ref);
   static void depth2DOn    (Bool background=false); // this enables processing pixels only in foreground or background (depending on depth buffer value)
   static void depth2DOff   (                     ); //     disables processing pixels only in foreground or background 
   static void sampler2D    ();
   static void sampler3D    ();
   static void samplerShadow();
   static void set2D        ();
   static void set3D        ();
   static void fbo          (UInt fbo);
   #if IOS
          Bool mainFBO      ()C; // on iOS there's only one custom FBO used, so we have to check active targets manually
   #else
          Bool mainFBO      ()C {return _fbo==0;}
   #endif
#endif

#if !EE_PRIVATE
private:
#endif
   ALPHA_MODE _alpha;
   Bool       _depth_lock, _depth, _depth_write, _cull, _line_smooth, _wire, _clip, _clip_allow, _clip_real, _clip_plane_allow, _sampler2D;
   Byte       _col_write[4], _stencil, _stencil_ref, _bias;
   UInt       _depth_func, _sampler_filter[3], _sampler_address, _sample_mask, _fbo;
   RectI      _viewport, _clip_recti;
   Rect       _clip_rect;
   Color      _alpha_factor;
   Vec4       _alpha_factor_v4, _clip_plane;
#if EE_PRIVATE
   GPU_API(IDirect3DVertexDeclaration9, ID3D11InputLayout, VtxFormatGL) *_vf;
#else
   Ptr        _vf;
#endif
protected:
   DisplayState();
};
/******************************************************************************/
