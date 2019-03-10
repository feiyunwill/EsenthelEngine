/******************************************************************************

   'MeshRender' is the hardware version of the 'MeshBase'.
   It contains vertexes and triangles.
   It is stored on the GPU memory.

/******************************************************************************/
#if EE_PRIVATE
enum MSHR_FLAG
{
   MSHR_COMPRESS  =1<<0, // Nrm,Tan,Bin are compressed into VecB4
   MSHR_SIGNED    =1<<1, // compressed VecB4 is stored as Signed Byte
   MSHR_BONE_SPLIT=1<<2, // vertex buffer is split into bone splits
};
#endif
/******************************************************************************/
struct MeshRender // Mesh Renderable (Hardware: contains Vertexes + Triangles)
{
   // manage
   MeshRender& del();
#if EE_PRIVATE
   void zero     ();
   Bool create   (Int vtxs, Int tris,             UInt flag       ,                      Bool compress=true);
   Bool createRaw(C MeshBase    &src  ,           UInt flag_and=~0, Bool optimize=true , Bool compress=true);
   Bool create   (C MeshBase    &src  ,           UInt flag_and=~0, Bool optimize=true , Bool compress=true);
   Bool create   (C MeshRender  &src                                                                       );
   Bool create   (C MeshRender  *src[], Int elms, UInt flag_and=~0, Bool optimize=false, Bool compress=true); // create from 'src' array, 'flag_and'=MESH_BASE_FLAG
#endif

   // get
   Bool is  ()C {return vtxs() || tris();} // if  has any data
   Int  vtxs()C {return _vb.vtxs()      ;} // get number of vertexes
   Int  tris()C {return    _tris        ;} // get number of triangles

   UInt memUsage(                )C {return _vb.memUsage()+_ib.memUsage();} // get memory usage of the mesh (in bytes)
   Int  vtxSize (                )C {return _vb.vtxSize ()               ;} // get size of a single vertex
   Int  vtxOfs  (UInt elm        )C;                                        // get offset of a specified vertex component in the vertex data, -1 if not found, 'elm'=one of MESH_BASE_FLAG enums
   Bool indBit16(                )C {return _ib.bit16()                  ;} // if  indices are 16-bit (false for 32-bit)
   UInt    flag (                )C {return _flag                        ;} // get MESH_BASE_FLAG that this mesh has
   Bool getBox  (Box &box        )C;                                        // get box encapsulating the mesh, this method iterates through all vertexes, false on fail (if no vertexes are present)
   Flt     area (Vec *center=null)C;                                        // get surface area of all mesh faces, 'center'=if specified then it will be calculated as the average surface center

   // transform
   void scaleMove(C Vec &scale, C Vec &move=VecZero);

   // texture transform
   void texMove  (C Vec2 &move , Byte tex_index=0);
   void texScale (C Vec2 &scale, Byte tex_index=0);
   void texRotate(  Flt   angle, Byte tex_index=0);

   // operations
 C Byte* vtxLockedData(                              )C {return _vb.lockedData(    );} //    get vertex data if it's already locked, null on fail
 C Byte* vtxLockedElm (UInt elm                      )C;                               //    get vertex data if it's already locked offsetted by specified vertex component in the vertex data according to 'vtxOfs' method, 'elm'=one of MESH_BASE_FLAG enums, null on fail (if the vertex buffer is not locked or if the component was not found)
   Byte* vtxLock      (LOCK_MODE lock=LOCK_READ_WRITE)  {return _vb.lock      (lock);} //   lock vertex data and return it, this method may be used to directly modify values of hardware mesh vertexes after getting their offset in the data by using 'vtxOfs' method (currently you should use it only for 'VTX_POS' as 'Vec', 'VTX_TEX' as 'Vec2' and 'VTX_COLOR' as 'Color' components, as others may be stored in compressed format), null on fail
 C Byte* vtxLockRead  (                              )C {return _vb.lockRead  (    );} //   lock vertex data and return it, this method may be used to directly access values of hardware mesh vertexes after getting their offset in the data by using 'vtxOfs' method (currently you should use it only for 'VTX_POS' as 'Vec', 'VTX_TEX' as 'Vec2' and 'VTX_COLOR' as 'Color' components, as others may be stored in compressed format), null on fail
   void  vtxUnlock    (                              )C {       _vb.unlock    (    );} // unlock vertex data

   CPtr indLockedData(                              )C {return _ib.  lockedData(    );} //    get index data if it's already locked, null on fail
    Ptr indLock      (LOCK_MODE lock=LOCK_READ_WRITE)  {return _ib.  lock      (lock);} //   lock index data and return it, this method may be used to directly access values of hardware mesh indexes, null on fail
   CPtr indLockRead  (                              )C {return _ib.  lockRead  (    );} //   lock index data and return it, this method may be used to directly modify values of hardware mesh indexes, null on fail
   void indUnlock    (                              )C {       _ib.unlock      (    );} // unlock index data

#if EE_PRIVATE
   Int  triIndSize      ()C {return indBit16() ? SIZE(U16)*3 : SIZE(U32)*3;} // get triangle indexes size
   Bool storageCompress ()C {return FlagTest(_storage, MSHR_COMPRESS  );}
   Bool storageSigned   ()C {return FlagTest(_storage, MSHR_SIGNED    );}
   Bool storageBoneSplit()C {return FlagTest(_storage, MSHR_BONE_SPLIT);}

   void     setUsedBones(Bool (&bones)[256])C;
   void includeUsedBones(Bool (&bones)[256])C;

   // draw
#if DX9
   void drawFull     (                         )C {D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vtxs(),           0, tris());}
   void drawRange    (Int tris                 )C {D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vtxs(),           0, tris  );}
   void drawRange    (Int tris, Int start_index)C {D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vtxs(), start_index, tris  );}
   void drawInstanced(Int instances            )C {}
#elif DX11
   void drawFull     (                         )C {D3DC->DrawIndexed         (_ib._ind_num,           0, 0);}
   void drawRange    (Int tris                 )C {D3DC->DrawIndexed         (tris*3      ,           0, 0);}
   void drawRange    (Int tris, Int start_index)C {D3DC->DrawIndexed         (tris*3      , start_index, 0);}
   void drawInstanced(Int instances            )C {D3DC->DrawIndexedInstanced(_ib._ind_num,   instances, 0, 0, 0);}
#elif GL
   void drawFull     (                         )C {glDrawElements         (GL_TRIANGLES, _ib._ind_num, _ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,                                   null);}
   void drawRange    (Int tris                 )C {glDrawElements         (GL_TRIANGLES,       tris*3, _ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,                                   null);}
   void drawRange    (Int tris, Int start_index)C {glDrawElements         (GL_TRIANGLES,       tris*3, _ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, Ptr(start_index*(_ib.bit16() ? 2 : 4)));}
   void drawInstanced(Int instances            )C {glDrawElementsInstanced(GL_TRIANGLES, _ib._ind_num, _ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,                                   null, instances);}
#endif

#if MAY_NEED_BONE_SPLITS
          void draw   ()C;              // this method supports BoneSplit's
          void drawFur()C;              // this method supports BoneSplit's
#else
   INLINE void draw   ()C {drawFull();} // BoneSplit's aren't used
   INLINE void drawFur()C {drawFull();} // BoneSplit's aren't used
#endif

   void drawBoneHighlight(Int bone, Shader *shader)C;

   // operations
   Bool        setVF();
 C MeshRender& set()C;

   MeshRender& optimize        (Bool faces=true, Bool vertexes=true); // this method will re-order elements for best rendering performance, 'faces'=if re-order faces, 'vertexes'=if re-order vertexes
   MeshRender& freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)
   void        adjustToPlatform();

   // io
#if EE_PRIVATE
   Bool saveData(File &f)C; // save binary, false on fail
   Bool loadData(File &f) ; // load binary, false on fail
#endif
   void operator=(C Str  &name) ; // load binary, Exit  on fail
   Bool save     (C Str  &name)C; // save binary, false on fail
   Bool load     (C Str  &name) ; // load binary, false on fail
   Bool save     (  File &f   )C; // save binary, false on fail
   Bool load     (  File &f   ) ; // load binary, false on fail

   explicit MeshRender(C MeshBase &mshb, UInt flag_and=~0, Bool optimize=true) : MeshRender() {create(mshb, flag_and, optimize);}

   struct BoneSplit
   {
      Byte split_to_real[256];
      Int  vtxs, tris, bones;

      Int realToSplit (Int bone)C; // -1 on fail
      Int realToSplit0(Int bone)C; //  0 on fail
   };
#endif

              ~MeshRender() {del();}
               MeshRender();
               MeshRender(C MeshRender &src);
   MeshRender& operator =(C MeshRender &src); // create from 'src'
   MeshRender& operator+=(C MeshRender &src); // add         'src'

#if !EE_PRIVATE
private:
#endif
   VtxBuf _vb;
   IndBuf _ib;
   Bool   _vao_reset;
   Byte   _storage;
   Int    _tris, _bone_splits;
   UInt   _flag;
#if EE_PRIVATE
   BoneSplit *_bone_split;
   GPU_API(IDirect3DVertexDeclaration9 *_vf, ID3D11InputLayout *_vf, union{UInt _vao; VtxFormatGL *_vf;});
#else
   Ptr    _bone_split, _vf;
#endif
};
/******************************************************************************/
