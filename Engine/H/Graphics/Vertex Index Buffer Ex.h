/******************************************************************************/
#if EE_PRIVATE
enum VTX_COMPRESS_FLAG
{
   VTX_COMPRESS_NRM    =1<<0, // compress "Vec  normal           " to "VecB4 normal "
   VTX_COMPRESS_TAN_BIN=1<<1, // compress "Vec  tangent, binormal" to "VecB4 tan_bin"
   VTX_COMPRESS_TEX    =1<<2, // compress "Vec2 tex              " to "VecH2 tex    "
   VTX_COMPRESS_TEX_8  =1<<3, // compress "Vec2 tex              " to "VecB4 tex    "

   VTX_COMPRESS_NRM_TAN_BIN=VTX_COMPRESS_NRM|VTX_COMPRESS_TAN_BIN,
};

Vec   UByte4ToNrm   (C VecB4 &v                                    ); // ( 0..255) -> (-1..1  )
void  UByte4ToTan   (C VecB4 &v,   Vec *tan,   Vec *bin, C Vec *nrm); // ( 0..255) -> (-1..1  )
VecB4    NrmToUByte4(C Vec   &v                                    ); // (-1..1  ) -> ( 0..255)
VecB4    TanToUByte4(C Vec   &v                                    ); // (-1..1  ) -> ( 0..255)
VecB4    TBNToUByte4(            C Vec *tan, C Vec *bin, C Vec *nrm); // (-1..1  ) -> ( 0..255)

Vec   SByte4ToNrm   (C VecB4 &v                                    ); // (-128..127) -> (-  1..1  )
void  SByte4ToTan   (C VecB4 &v,   Vec *tan,   Vec *bin, C Vec *nrm); // (-128..127) -> (-  1..1  )
VecB4    NrmToSByte4(C Vec   &v                                    ); // (-  1..1  ) -> (-128..127)
VecB4    TanToSByte4(C Vec   &v                                    ); // (-  1..1  ) -> (-128..127)
VecB4    TBNToSByte4(            C Vec *tan, C Vec *bin, C Vec *nrm); // (-  1..1  ) -> (-128..127)
/******************************************************************************/
struct VtxFormatKey
{
   UInt flag    , // MESH_BASE_FLAG
        compress; // VTX_COMPRESS_FLAG

   VtxFormatKey() {}
   VtxFormatKey(UInt flag, UInt compress) {T.flag=flag; T.compress=compress;}
};
#if DX9
Bool SetVtxFormatFromVtxDecl(IDirect3DVertexDeclaration9 *vf, D3DVERTEXELEMENT9 (&ve)[MAX_FVF_DECL_SIZE]);
#elif GL
enum GL_VTX_SEMANTIC : Byte // !! must be in sync with all "ATTR*" mentions in the engine !!
{
   GL_VTX_POS     , // 0
   GL_VTX_NRM     , // 1
   GL_VTX_TAN     , // 2
   GL_VTX_TEX0    , // 3
   GL_VTX_TEX1    , // 4, used for lightmaps
   GL_VTX_BONE    , // 5
   GL_VTX_WEIGHT  , // 6
   GL_VTX_COLOR   , // 7
   // Galaxy Tab 2 (PowerVR SGX 540) has only 8 vertex attributes
   GL_VTX_MATERIAL, // 8
   GL_VTX_HLP     , // 9
   // Galaxy Tab 3/4 have only 10 vertex attributes
   GL_VTX_SIZE    , // 10
   GL_VTX_TEX2    , // 11
   GL_VTX_NUM     , // 12
};
GL_VTX_SEMANTIC VtxSemanticToIndex(Int semantic);
struct VtxFormatGL
{
   struct Elm
   {
      GL_VTX_SEMANTIC semantic;
      Byte            component_num;
      Bool            component_normalize;
      GLenum          component_type;
      UInt            offset;

      void set(GL_VTX_SEMANTIC semantic, Byte component_num, GLenum component_type, Bool component_normalize, UInt offset) {T.semantic=semantic; T.component_num=component_num; T.component_type=component_type; T.component_normalize=component_normalize; T.offset=offset;}
   };
   Mems<Elm> elms;
   UInt      vtx_size, vao;

   void del   ();
   Bool create(C MemPtrN<Elm, 32> &elms);

   void disable   ()C;
   void  enableSet()C;
   void       bind(C VtxBuf &vb);

  ~VtxFormatGL() {del();}
   VtxFormatGL() {vtx_size=vao=0;}
};
#endif
#endif
struct VtxFormat // Vertex Format
{
   VtxFormat& del();

#if EE_PRIVATE
   #if DX9
      Bool create(D3DVERTEXELEMENT9 ve[]);
   #elif DX11
      Bool create(D3D11_INPUT_ELEMENT_DESC ve[], Int elms);
   #elif GL
      Bool create(C MemPtrN<VtxFormatGL::Elm, 32> &elms);
   #endif
   Bool create(UInt flag, UInt compress); // 'flag'=MESH_BASE_FLAG, 'compress'=VTX_COMPRESS_FLAG
#if GL
          void bind(C VtxBuf &vb);
#else
   INLINE void bind(C VtxBuf &vb) {}
#endif
#endif

  ~VtxFormat() {del();}
   VtxFormat() {vf=null;}

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   GPU_API(IDirect3DVertexDeclaration9, ID3D11InputLayout, VtxFormatGL) *vf;
#else
   Ptr vf;
#endif
   NO_COPY_CONSTRUCTOR(VtxFormat);
};
/******************************************************************************/
struct VtxBuf // Vertex Buffer
{
   // manage
   VtxBuf& del();

   // get
   Bool is      ()C {return _buf!=NULL        ;} // if  created
   Int  vtxs    ()C {return _vtx_num          ;} // get number of vertexes
   Int  vtxSize ()C {return          _vtx_size;} // get size of a single vertex
   UInt memUsage()C {return _vtx_num*_vtx_size;} // get memory usage of the vertex buffer (in bytes)

   // operations
 C Byte*  lockedData()C {return _data;}
   Byte*  lock      (LOCK_MODE lock=LOCK_READ_WRITE);
 C Byte*  lockRead  ()C;
   void unlock      () ;
   void unlock      ()C;

#if EE_PRIVATE
   Byte*  lockDynamic(); // !! this is to be called only for VI buffers !!
   void unlockDynamic(); // !! this is to be called only for VI buffers !!

   Bool setFrom(CPtr data, Int size); // false on fail

   // manage
   Bool createRaw(Int memory_size                       , Bool dynamic=false, CPtr data=null); //                                                      false on fail
   Bool createNum(Int vtx_size, Int vtx_num             , Bool dynamic=false, CPtr data=null); //                                                      false on fail
   Bool create   (Int vtx_num , UInt flag, UInt compress, Bool dynamic=false                ); // 'flag'=MESH_BASE_FLAG, 'compress'=VTX_COMPRESS_FLAG, false on fail
   Bool create   (C VtxBuf &src                         , Int  dynamic=-1                   ); // create from 'src'    ,                               false on fail

   void freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // draw
   #if DX9
      void set(Int stream=0)C {D3D->SetStreamSource(stream, _buf, 0, _vtx_size);}
   #elif DX11
      void set(Int stream=0)C {UInt stride=_vtx_size, offset=0; D3DC->IASetVertexBuffers(stream, 1, &_buf, &stride, &offset);}
   #elif GL
      void set(Int stream=0)C {glBindBuffer(GL_ARRAY_BUFFER, _buf);}
   #endif

   // io
   Bool save(File &f)C;
   Bool load(File &f) ;
#endif

  ~VtxBuf() {del ( );}
   VtxBuf() {Zero(T);}

#if !EE_PRIVATE
private:
#endif
   Bool      _dynamic;
   LOCK_MODE _lock_mode;
   Int       _vtx_size, _vtx_num, _lock_count;
   Byte     *_data;
#if EE_PRIVATE
   GPU_API(IDirect3DVertexBuffer9 *_buf, ID3D11Buffer *_buf, union{UInt _buf; Ptr buf_ptr;});
#else
   Ptr       _buf;
#endif
   NO_COPY_CONSTRUCTOR(VtxBuf);
};
/******************************************************************************/
struct IndBuf // Index Buffer
{
   // manage
   IndBuf& del();

   // get
   Bool is      ()C {return _ind_num>0;}
   Bool bit16   ()C {return           _bit16         ;} // if indices are 16-bit (false for 32-bit)
   UInt memUsage()C {return _ind_num*(_bit16 ? 2 : 4);} // get memory usage of the index buffer (in bytes)

   // operations
   CPtr   lockedData()C {return _data;}
    Ptr   lock      (LOCK_MODE lock=LOCK_READ_WRITE);
   CPtr   lockRead  ()C;
   void unlock      () ;
   void unlock      ()C;

#if EE_PRIVATE
   Bool setFrom(CPtr data); // false on fail

   Bool create(Int indexes, Bool bit16, Bool dynamic=false, CPtr data=null); //                    false on fail
   Bool create(C IndBuf &src          , Int  dynamic=-1                   ); // create from 'src', false on fail

   IndBuf& setTri (Int i, Int v0, Int v1, Int v2);
   IndBuf& setQuad(Int i, Int v0, Int v1, Int v2, Int v3);

   void freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // draw
   #if DX9
      void set()C {D3D->SetIndices(_buf);}
   #elif DX11
      void set()C {D3DC->IASetIndexBuffer(_buf, _bit16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);}
   #elif GL
      void set()C {glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buf);}
   #endif

   // io
   Bool save(File &f)C;
   Bool load(File &f) ;
#endif

  ~IndBuf() {del ( );}
   IndBuf() {Zero(T);}

#if !EE_PRIVATE
private:
#endif
   Bool      _dynamic, _bit16;
   LOCK_MODE _lock_mode;
   Int       _ind_num, _lock_count;
   Byte     *_data;
#if EE_PRIVATE
   GPU_API(IDirect3DIndexBuffer9 *_buf, ID3D11Buffer *_buf, union{UInt _buf; Ptr buf_ptr;});
#else
   Ptr       _buf;
#endif
   NO_COPY_CONSTRUCTOR(IndBuf);
};
/******************************************************************************/
#if EE_PRIVATE
void InitVtxInd();
void ShutVtxInd();

#if GL
   INLINE void SetDefaultVAO() {if(D.notShaderModelGLES2())glBindVertexArray(VAO);}
#else
   INLINE void SetDefaultVAO() {}
#endif

void BindIndexBuffer(UInt buf);

extern ThreadSafeMap<VtxFormatKey, VtxFormat> VtxFormats;
extern IndBuf                                 IndBuf16384Quads, IndBufBorder, IndBufPanel, IndBufPanelEx, IndBufRectBorder, IndBufRectShaded;
#endif
/******************************************************************************/
