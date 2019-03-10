/******************************************************************************/
struct ShaderImage // Shader Image
{
 C Image* get(                   )C {return   _image  ;}
   void   set(C Image      *image)  {T._image= image  ;}
   void   set(C Image      &image)  {T._image=&image  ;}
   void   set(C ImageRTPtr &image)  {T._image= image();}
 //void   set(C ImagePtr   &image)  {T._image= image();} this is not safe, as 'ShaderImage' does not store 'ImagePtr' for performance reasons

   ShaderImage() {_image=null; _sampler=null;}

#if !EE_PRIVATE
private:
#endif
 C Image *_image;
#if EE_PRIVATE
   struct Sampler
   {
   #if DX9
      D3DTEXTUREFILTERTYPE filter [2];
      D3DTEXTUREADDRESS    address[3];

      void set(Int index);
      void del() {}
   #elif DX11
      ID3D11SamplerState *state;

      Bool is()C {return state!=null;}

      void del      ();
      Bool createTry(D3D11_SAMPLER_DESC &desc);
      void create   (D3D11_SAMPLER_DESC &desc);
      void setVS    (Int index);
      void setHS    (Int index);
      void setDS    (Int index);
      void setPS    (Int index);
      void set      (Int index);

      Sampler() {state=null;}
     ~Sampler() {del();}
   #elif GL
      UInt filter [2];
      UInt address[3];

      void del() {}
   #endif
   };
   Sampler *_sampler;

   #if DX9
      INLINE IDirect3DBaseTexture9   * getBase()C {return _image ? _image->_base : null;}
   #elif DX11
      INLINE ID3D11ShaderResourceView* getSRV ()C {return _image ? _image->_srv  : null;}
   #endif
#else
   Ptr    _sampler;
#endif
};
/******************************************************************************/
struct ShaderParam // Shader Parameter
{
            void set(  Bool     b               ); // set boolean  value
            void set(  Int      i               ); // set integer  value
            void set(  Flt      f               ); // set float    value
            void set(  Dbl      d               ); // set double   value
            void set(C Vec2    &v               ); // set vector2D value
            void set(C VecD2   &v               ); // set vector2D value
            void set(C VecI2   &v               ); // set vector2D value
            void set(C Vec     &v               ); // set vector3D value
            void set(C VecD    &v               ); // set vector3D value
            void set(C VecI    &v               ); // set vector3D value
            void set(C Vec4    &v               ); // set vector4D value
            void set(C VecD4   &v               ); // set vector4D value
            void set(C VecI4   &v               ); // set vector4D value
            void set(C Color   &color           ); // set vector4D value
            void set(C Rect    &rect            ); // set vector4D value
            void set(C Matrix3 &matrix          ); // set matrix3  value
            void set(C Matrix  &matrix          ); // set matrix   value
            void set(C MatrixM &matrix          ); // set matrix   value
            void set(C Matrix4 &matrix          ); // set matrix4  value
            void set(C Vec     *v     , Int elms); // set vector3D array
            void set(C Vec4    *v     , Int elms); // set vector4D array
            void set(C Matrix  *matrix, Int elms); // set matrix   array
            void set(  CPtr     data  , Int size); // set memory
   T1(TYPE) void set(C TYPE    &data            ) {set((CPtr)&data, SIZE(data));}
         #if EE_PRIVATE
            void set    (C Vec       &v              , Int elm ); // set vector3D array element value
            void set    (C Vec4      &v              , Int elm ); // set vector4D array element value
            void set    (C Matrix    &matrix         , Int elm ); // set matrix   array element value
            void fromMul(C Matrix    &a, C Matrix  &b          ); // set matrix                 value from "a*b"
            void fromMul(C Matrix    &a, C MatrixM &b          ); // set matrix                 value from "a*b"
            void fromMul(C MatrixM   &a, C MatrixM &b          ); // set matrix                 value from "a*b"
            void fromMul(C Matrix    &a, C Matrix  &b, Int elm ); // set matrix   array element value from "a*b"
            void fromMul(C Matrix    &a, C MatrixM &b, Int elm ); // set matrix   array element value from "a*b"
            void fromMul(C MatrixM   &a, C MatrixM &b, Int elm ); // set matrix   array element value from "a*b"
            void set    (C GpuMatrix &matrix                   ); // set matrix                 value
            void set    (C GpuMatrix &matrix         , Int elm ); // set matrix   array element value
            void set    (C GpuMatrix *matrix         , Int elms); // set matrix   array

            void setConditional(C Flt  &f         ); // set float                  value only if it's different
            void setConditional(C Vec2 &v         ); // set vector2D               value only if it's different
            void setConditional(C Vec  &v         ); // set vector3D               value only if it's different
            void setConditional(C Vec4 &v         ); // set vector4D               value only if it's different
            void setConditional(C Vec  &v, Int elm); // set vector3D array element value only if it's different
            void setConditional(C Rect &r         ); // set vector4D               value only if it's different

            void setSafe(C Vec4 &v); // set from vector4D value, but limit the actual size copied based on 'ShaderParam' size
         #endif

#if EE_PRIVATE
   struct Translation
   {
      Int cpu_offset, gpu_offset, elm_size;

      Bool operator!=(C Translation &trans)C {return T.cpu_offset!=trans.cpu_offset || T.gpu_offset!=trans.gpu_offset || T.elm_size!=trans.elm_size;}

      void set(Int cpu_offset, Int gpu_offset, Int elm_size) {T.cpu_offset=cpu_offset; T.gpu_offset=gpu_offset; T.elm_size=elm_size;}
   };

   Byte *_data;
   Int   _cpu_data_size, _gpu_data_size, _elements, _constant_count;
   Bool *_changed, _owns_data;
   Mems<Translation> _full_translation, _optimized_translation;

   Bool is()C {return _cpu_data_size>0;}
   Int  fullConstantCount()C {return Ceil16(_gpu_data_size)/16;} // number of Vec4's, SIZE(Vec4)==16, DX9 constants operate on Vec4, DX10+ does not use this, for OpenGL this will work only for Vec4's

   INLINE void setChanged() {*_changed=true;}
          void optimize();
          void initAsElement(ShaderParam &parent, Int index);

   Bool save(File &f, C Str8 &name)C;

   INLINE GpuMatrix* asGpuMatrix() {return (GpuMatrix*)_data;}

  ~ShaderParam();
   ShaderParam();

   NO_COPY_CONSTRUCTOR(ShaderParam);
#endif
};
/******************************************************************************/
struct ShaderParamChange // Shader Parameter Change
{
   ShaderParam *param; // parameter to change
   Vec4         value; // value     to change to

   ShaderParamChange& set(  Bool  b) {value.x  =b; return T;}
   ShaderParamChange& set(  Int   i) {value.x  =i; return T;}
   ShaderParamChange& set(  Flt   f) {value.x  =f; return T;}
   ShaderParamChange& set(C Vec2 &v) {value.xy =v; return T;}
   ShaderParamChange& set(C Vec  &v) {value.xyz=v; return T;}
   ShaderParamChange& set(C Vec4 &v) {value    =v; return T;}

   ShaderParamChange& set(ShaderParam *param) {T.param=param; return T;}

   ShaderParamChange() {param=null; value.zero();}
};
/******************************************************************************/
#if EE_PRIVATE
struct ShaderBuffer // Constant Buffer
{
   struct Buffer
   {
      GPU_API(Ptr, ID3D11Buffer*, Ptr) buffer; // keep this is first member because it's used most often
      Int                              size  ;

      void del   ();
      void create(Int size);

      Bool is()C {return size>0;}
      void zero() {buffer=null; size=0;}

     ~Buffer() {del ();}
      Buffer() {zero();}
      // intentionally keep copy constructor as raw member copy, because we expect this behavior
   };

   Buffer       buffer; // keep this is first member because it's used most often
   Byte        *data;
   Bool         changed;
   Mems<Buffer> parts;

   Bool is         ()C {return buffer.is();}
   Int  size       ()C {return parts.elms() ? parts[0].size : buffer.size;} // first part always has the full size, so use it if available, because 'buffer' can be set to a smaller part
   void create     (Int size );
   void update     (         );
   void bind       (Int index);
   void bindCheck  (Int index);
   void    setPart (Int part );
   void createParts(C Int *elms, Int elms_num);

   ShaderBuffer();
  ~ShaderBuffer();

   NO_COPY_CONSTRUCTOR(ShaderBuffer);
};
STRUCT(ShaderParamName , ShaderParam)
//{
   Str8 name;

   Bool save(File &f)C {return super::save(f, name);}
};
struct ShaderBufferParams
{
   ShaderBuffer         *buffer;
   Int                   index;
   Mems<ShaderParamName> params;
};
#endif
/******************************************************************************/
struct ShaderShader
{
#if EE_PRIVATE
   Mems<Byte> data;

   void clean() {data.del();}

   Bool save(File &f)C {return data.saveRaw(f);}
   Bool load(File &f)  {return data.loadRaw(f);}
#endif
};
#if EE_PRIVATE
/******************************************************************************/
#if WINDOWS_OLD
STRUCT(ShaderVS9 , ShaderShader)
//{
   IDirect3DVertexShader9 *vs;

   IDirect3DVertexShader9* create();

  ~ShaderVS9();
   ShaderVS9() {vs=null;}
   NO_COPY_CONSTRUCTOR(ShaderVS9);
};
STRUCT(ShaderPS9 , ShaderShader)
//{
   IDirect3DPixelShader9 *ps;

   IDirect3DPixelShader9* create();

  ~ShaderPS9();
   ShaderPS9() {ps=null;}
   NO_COPY_CONSTRUCTOR(ShaderPS9);
};
#endif
/******************************************************************************/
#if WINDOWS
STRUCT(ShaderVS11 , ShaderShader)
//{
   ID3D11VertexShader *vs;

   ID3D11VertexShader* create();

  ~ShaderVS11();
   ShaderVS11() {vs=null;}
   NO_COPY_CONSTRUCTOR(ShaderVS11);
};
STRUCT(ShaderHS11 , ShaderShader)
//{
   ID3D11HullShader *hs;

   ID3D11HullShader* create();

  ~ShaderHS11();
   ShaderHS11() {hs=null;}
   NO_COPY_CONSTRUCTOR(ShaderHS11);
};
STRUCT(ShaderDS11 , ShaderShader)
//{
   ID3D11DomainShader *ds;

   ID3D11DomainShader* create();

  ~ShaderDS11();
   ShaderDS11() {ds=null;}
   NO_COPY_CONSTRUCTOR(ShaderDS11);
};
STRUCT(ShaderPS11 , ShaderShader)
//{
   ID3D11PixelShader *ps;

   ID3D11PixelShader* create();

  ~ShaderPS11();
   ShaderPS11() {ps=null;}
   NO_COPY_CONSTRUCTOR(ShaderPS11);
};
#endif
/******************************************************************************/
STRUCT(ShaderVSGL , ShaderShader)
//{
   UInt vs;

   UInt create(Bool clean, Str *messages);
   Str  source();

  ~ShaderVSGL();
   ShaderVSGL() {vs=0;}
   NO_COPY_CONSTRUCTOR(ShaderVSGL);
};
STRUCT(ShaderPSGL , ShaderShader)
//{
   UInt ps;

   UInt create(Bool clean, Str *messages);
   Str  source();

  ~ShaderPSGL();
   ShaderPSGL() {ps=0;}
   NO_COPY_CONSTRUCTOR(ShaderPSGL);
};
/******************************************************************************/
#if WINDOWS_OLD
struct Shader9
{
   struct Texture
   {
      Int          index;
      ShaderImage *image;

      void set(Int index, ShaderImage &image) {T.index=index; T.image=&image;}
   };
   const_mem_addr struct Constant
   {
      Int          start, count, *final_count; // 'final_count'=points to either 'Constant.count' or 'ShaderParam._constant_count' (for example 'ShaderParam ViewMatrix _constant_count' can be limited depending on 'SetMatrixCount')
      Ptr          data;
      Bool        *changed;
      ShaderParam *sp;

      void set(Int start, Int count, Ptr data, ShaderParam &sp) {T.start=start; T.count=count; T.final_count=&T.count; T.data=data; T.changed=sp._changed; T.sp=&sp;}
   };

   IDirect3DVertexShader9 *vs;
   IDirect3DPixelShader9  *ps;
   Str8           name;
   Int            vs_index, ps_index;
   Mems<Texture > textures;
   Mems<Constant> vs_constants, ps_constants;

   Bool validate (ShaderFile &shader, Str *messages=null);
   void commit   ();
   void commitTex();
   void start    ();
   void begin    ();
   Bool save     (File &f, C Map<Str8, ShaderParam>   &params, C Memc <ShaderImage*     > &images)C;
   Bool load     (File &f, C MemtN<ShaderParam*, 256> &params, C MemtN<ShaderImage*, 256> &images);

   Shader9();
};
#endif
/******************************************************************************/
#if WINDOWS
struct Shader11
{
   struct Texture
   {
      Int          index;
      ShaderImage *image;

      void set(Int index, ShaderImage &image) {T.index=index; T.image=&image;}
   };
   struct Buffer
   {
      Int           index ;
      ShaderBuffer *buffer;

      void set(Int index, ShaderBuffer &buffer) {T.index=index; T.buffer=&buffer;}
   };

   ID3D11VertexShader *vs;
   ID3D11HullShader   *hs;
   ID3D11DomainShader *ds;
   ID3D11PixelShader  *ps;
   Str8                name;
   Int                 vs_index   , hs_index   , ds_index   , ps_index   ;
   Mems<Texture      > vs_textures, hs_textures, ds_textures, ps_textures;
   Mems<      Buffer > vs_buffers , hs_buffers , ds_buffers , ps_buffers ;
   Mems<ShaderBuffer*>    buffers;

   Bool validate (ShaderFile &shader, Str *messages=null);
   void commit   ();
   void commitTex();
   void start    ();
   void begin    ();
   Bool save     (File &f, C Memc <ShaderBufferParams> &buffers, C Memc <ShaderImage*     > &images)C;
   Bool load     (File &f, C MemtN<ShaderBuffer*, 256> &buffers, C MemtN<ShaderImage*, 256> &images);

   Shader11();
};
#endif
/******************************************************************************/
struct ShaderGL
{
   struct Texture
   {
      Int          index;
      ShaderImage *image;

      void set(Int index, ShaderImage &image) {T.index=index; T.image=&image;}
   };
   const_mem_addr struct Constant
   {
   #if GL
      typedef void (PLATFORM(WINAPI,) *glUniformPtr) (GLint location, GLsizei count, const GLfloat *value);
   #endif

      Int          index, count, *final_count; // 'final_count'=points to either 'Constant.count' or 'ShaderParam._constant_count' (for example 'ShaderParam ViewMatrix _constant_count' can be limited depending on 'SetMatrixCount')
      Ptr          data;
      Bool        *changed;
      ShaderParam *sp;
      GPU_API(Ptr, Ptr, glUniformPtr) uniform;

      void set(Int index, Int count, Ptr data, ShaderParam &sp) {T.index=index; T.count=count; T.final_count=&T.count; T.data=data; T.changed=sp._changed; T.sp=&sp;}
   };
   struct GLSLParam
   {
      U16          gpu_offset;
      ShaderParam *param;
      Str8         glsl_name;

      void set(Int gpu_offset, ShaderParam &param, C Str8 &glsl_name);
   };

   Str8            name;
   Int             vs_index, ps_index;
   UInt            vs, ps, prog;
   Mems<Texture>   textures;
   Mems<Constant>  constants;
   Mems<GLSLParam> glsl_params;

   Str  source   ();
   UInt compileEx(MemPtr<ShaderVSGL> vs_array, MemPtr<ShaderPSGL> ps_array, Bool clean, ShaderFile *shader, Str *messages);
   void compile  (MemPtr<ShaderVSGL> vs_array, MemPtr<ShaderPSGL> ps_array, Str *messages);
   Bool validate (ShaderFile &shader, Str *messages=null);
   void commit   ();
   void commitTex();
   void start    ();
   void begin    ();
   Bool save     (File &f, C Map  <Str8, ShaderParam> &params, C Memc <ShaderImage*     > &images)C;
   Bool load     (File &f, C MemtN<ShaderParam*, 256> &params, C MemtN<ShaderImage*, 256> &images);

   ShaderGL();
  ~ShaderGL();
};
/******************************************************************************/
struct ShaderBase
{
   Int shader_draw; // index of this shader in 'ShaderDraws' container

   INLINE Shader& asShader       () {return   (Shader& )T;}
   INLINE Shader& asForwardShader() {return **(Shader**)(((Byte*)this)+Renderer._frst_light_offset);}
   INLINE Shader& asBlendShader  () {return **(Shader**)(((Byte*)this)+Renderer._blst_light_offset);}

   INLINE Shader& getShader(Bool forward)
   {
      if(forward)return asForwardShader();
      else       return asShader       ();
   }
   INLINE Shader& getBlendShader(Bool blst)
   {
      if(blst)return asBlendShader();
      else    return asShader     ();
   }

   void unlink() {shader_draw=-1;}

   ShaderBase() {unlink();}
};
struct Shader : ShaderBase, GPU_API(Shader9, Shader11, ShaderGL)
{
#else
struct Shader
{
#endif
   void draw(C Image      *image=null, C Rect *rect=null        );                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(C Image      &image     , C Rect *rect=null        ) {draw(&image  , rect     );} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(C ImageRTPtr &image     , C Rect *rect=null        ) {draw( image(), rect     );} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(C Image      *image     , C Rect *rect, C Rect &tex);                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates
   void draw(C Image      &image     , C Rect *rect, C Rect &tex) {draw(&image  , rect, tex);} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates
   void draw(C ImageRTPtr &image     , C Rect *rect, C Rect &tex) {draw( image(), rect, tex);} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Col' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates
};
/******************************************************************************/
struct ShaderFile // Shader File
{
   // get
   Shader* first(            ); // first shader, null on fail
   Shader*  find(C Str8 &name); // find  shader, null on fail
   Shader*   get(C Str8 &name); //  get  shader, Exit on fail
#if EE_PRIVATE
   Shader*  find(C Str8 &name, Str *messages); // find shader, put error messages into 'messages', null on fail
#endif

   // manage
   void del();

   // io
   Bool load(C Str &name); // load, false on fail

   ShaderFile();
  ~ShaderFile() {del();}

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   #if DX9
      Mems<ShaderVS9   > _vs;
      Mems<ShaderShader> _hs;
      Mems<ShaderShader> _ds;
      Mems<ShaderPS9   > _ps;
   #elif DX11
      Mems<ShaderVS11> _vs;
      Mems<ShaderHS11> _hs;
      Mems<ShaderDS11> _ds;
      Mems<ShaderPS11> _ps;
   #elif GL
      Mems<ShaderVSGL  > _vs;
      Mems<ShaderShader> _hs;
      Mems<ShaderShader> _ds;
      Mems<ShaderPSGL  > _ps;
   #endif
#else
   Mems<ShaderShader> _vs, _hs, _ds, _ps;
#endif
   Mems<Shader      > _shaders;
   NO_COPY_CONSTRUCTOR(ShaderFile);
};
/******************************************************************************/
#if EE_PRIVATE
struct FRSTKey // Forward Rendering Shader Techniques Key
{
   Byte skin, materials, textures, bump_mode, alpha_test, light_map, detail, rflct, color, mtrl_blend, heightmap, fx, tess;

   FRSTKey() {Zero(T);}
};
struct FRST : ShaderBase // Forward Rendering Shader Techniques
{
   Bool all_passes;
   Shader
      *none,               // no light
      *dir , * dir_shd[6], // directional light, [MapNum]
      *pnt , * pnt_shd   , // point       light
      *sqr , * sqr_shd   , // square      light
      *cone, *cone_shd   ; // cone        light
};
/******************************************************************************/
struct BLSTKey // Blend Light Shader Techniques
{
   Byte skin, color, textures, bump_mode, alpha_test, alpha, light_map, rflct, fx, per_pixel;

   BLSTKey() {Zero(T);}
};
struct BLST // Blend Light Shader Techniques
{
   Shader *dir[7];
};
/******************************************************************************/
extern GPU_API(Shader9, Shader11, ShaderGL) *ShaderCur;
extern ThreadSafeMap<FRSTKey, FRST        > Frsts        ; // Forward Rendering Shader Techniques
extern ThreadSafeMap<BLSTKey, BLST        > Blsts        ; // Blend   Light     Shader Techniques
extern ThreadSafeMap<Str8   , ShaderImage > ShaderImages ; // Shader Images
extern ThreadSafeMap<Str8   , ShaderParam > ShaderParams ; // Shader Parameters
extern ThreadSafeMap<Str8   , ShaderBuffer> ShaderBuffers; // Shader Constant Buffers
#endif
extern Cache<ShaderFile> ShaderFiles; // Shader File Cache
/******************************************************************************/
struct ShaderMacro // macro used for shader compilation
{
   Str8 name      ,
        definition;

   void set(C Str8 &name, C Str8 &definition) {T.name=name; T.definition=definition;}
};
#if EE_PRIVATE
struct ShaderGLSL
{
   Str8              group_name, tech_name;
   Memc<ShaderMacro> params;

   ShaderGLSL& set(C Str8 &group_name, C Str8 &tech_name ) {T.group_name=group_name; T.tech_name=tech_name; return T;}
   ShaderGLSL& par(C Str8 &name      , C Str8 &definition) {params.New().set(name, definition);             return T;}
};
#endif
/******************************************************************************/
// shader image
ShaderImage* FindShaderImage(CChar8 *name); // find shader image, null on fail (shader image can be returned only after loading a shader which contains the image)
ShaderImage*  GetShaderImage(CChar8 *name); // find shader image, Exit on fail (shader image can be returned only after loading a shader which contains the image)

// shader parameter
ShaderParam* FindShaderParam(CChar8 *name); // find shader parameter, null on fail (shader parameter can be returned only after loading a shader which contains the parameter)
ShaderParam*  GetShaderParam(CChar8 *name); // find shader parameter, Exit on fail (shader parameter can be returned only after loading a shader which contains the parameter)

         inline void SPSet(CChar8 *name,   Bool     b               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(b           );} // set boolean  value
         inline void SPSet(CChar8 *name,   Int      i               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(i           );} // set integer  value
         inline void SPSet(CChar8 *name,   Flt      f               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(f           );} // set float    value
         inline void SPSet(CChar8 *name,   Dbl      d               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(d           );} // set double   value
         inline void SPSet(CChar8 *name, C Vec2    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C VecD2   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C VecI2   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C Vec     &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C VecD    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C VecI    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C Vec4    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C VecD4   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C VecI4   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C Color   &color           ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(color       );} // set vector4D value
         inline void SPSet(CChar8 *name, C Rect    &rect            ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(rect        );} // set vector4D value
         inline void SPSet(CChar8 *name, C Matrix3 &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix   value
         inline void SPSet(CChar8 *name, C Matrix  &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix   value
         inline void SPSet(CChar8 *name, C Matrix4 &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix4  value
         inline void SPSet(CChar8 *name, C Vec     *v     , Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v     , elms);} // set vector3D array
         inline void SPSet(CChar8 *name, C Vec4    *v     , Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v     , elms);} // set vector4D array
         inline void SPSet(CChar8 *name, C Matrix  *matrix, Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix, elms);} // set matrix   array
         inline void SPSet(CChar8 *name,   CPtr     data  , Int size) {if(ShaderParam *sp=FindShaderParam(name))sp->set(data  , size);} // set memory
T1(TYPE) inline void SPSet(CChar8 *name, C TYPE    &data            ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(data        );} // set memory

#if EE_PRIVATE
ShaderBuffer* FindShaderBuffer(CChar8 *name);
ShaderBuffer*  GetShaderBuffer(CChar8 *name);

   #if DX9
             void ShaderEnd();
   #else
      INLINE void ShaderEnd() {}
   #endif
#endif

// compile
#if EE_PRIVATE
Bool ShaderCompileTry(C Str &src, C Str &dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros, C MemPtr<ShaderGLSL> &stg=null, Str *messages=null); // compile shader from 'src' file to 'dest' using additional 'macros', false on fail, 'messages'=optional parameter which will receive any messages that occurred during compilation
void ShaderCompile   (C Str &src, C Str &dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros, C MemPtr<ShaderGLSL> &stg                         ); // compile shader from 'src' file to 'dest' using additional 'macros', Exit  on fail
#endif
Bool ShaderCompileTry(Str src, Str dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros=null, Str *messages=null); // compile shader from 'src' file to 'dest' using additional 'macros', false on fail, 'messages'=optional parameter which will receive any messages that occurred during compilation
void ShaderCompile   (Str src, Str dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros=null                    ); // compile shader from 'src' file to 'dest' using additional 'macros', Exit  on fail
/******************************************************************************/
