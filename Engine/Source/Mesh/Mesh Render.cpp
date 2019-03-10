/******************************************************************************/
#include "stdafx.h"
#define VAO_EXCLUSIVE HAS_THREADS // if VAO's can be processed only on the main thread - https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object "Note: VAOs cannot be shared between OpenGL contexts"
/******************************************************************************/
namespace EE{
#if GL && VAO_EXCLUSIVE
static Memc<UInt> VAOs; // list of released VAO's !! must be handled only under D._lock !! we could optionally 'glDeleteVertexArrays' them at app shut down but it's not needed
#endif
/******************************************************************************/
Int MeshRender::BoneSplit::realToSplit0(Int bone)C {return Max(0, realToSplit(bone));}
Int MeshRender::BoneSplit::realToSplit (Int bone)C
{
   REP(bones)if(split_to_real[i]==bone)return i;
   return -1;
}
/******************************************************************************/
T1(TYPE) static INLINE void Set(Byte *&v, Int i, C TYPE *t) {if(t){*(TYPE*)v=t[i]; v+=SIZE(TYPE);}}
T1(TYPE) static INLINE void Set(Byte *&v,        C TYPE &t) {      *(TYPE*)v=t   ; v+=SIZE(TYPE); }
/******************************************************************************/
void MeshRender::zero()
{
#if GL && VAO_EXCLUSIVE
  _vao_reset=false;
#endif
  _storage=0;
  _tris=_bone_splits=0;
  _flag=0;
  _bone_split=null;
  _vf=null;
}
MeshRender::MeshRender(                 )                {zero();}
MeshRender::MeshRender(C MeshRender &src) : MeshRender() {T=src;}
/******************************************************************************/
Int MeshRender::vtxOfs(UInt elm)C
{
   Int ofs=0;
   if(storageCompress())
   {
      if(_flag&VTX_POS     ){if(elm&VTX_POS     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_NRM     ){if(elm&VTX_NRM     )return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_TAN_BIN ){if(elm&VTX_TAN_BIN )return ofs; ofs+=SIZE(VecB4);} // in compressed mode Tan and Bin are merged together
      if(_flag&VTX_HLP     ){if(elm&VTX_HLP     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_TEX0    ){if(elm&VTX_TEX0    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_TEX1    ){if(elm&VTX_TEX1    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_TEX2    ){if(elm&VTX_TEX2    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_MATRIX  ){if(elm&VTX_MATRIX  )return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_BLEND   ){if(elm&VTX_BLEND   )return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_SIZE    ){if(elm&VTX_SIZE    )return ofs; ofs+=SIZE(Flt  );}
      if(_flag&VTX_MATERIAL){if(elm&VTX_MATERIAL)return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_COLOR   ){if(elm&VTX_COLOR   )return ofs; ofs+=SIZE(VecB4);}
   }else
   {
      if(_flag&VTX_POS     ){if(elm&VTX_POS     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_NRM     ){if(elm&VTX_NRM     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_TAN     ){if(elm&VTX_TAN     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_BIN     ){if(elm&VTX_BIN     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_HLP     ){if(elm&VTX_HLP     )return ofs; ofs+=SIZE(Vec  );}
      if(_flag&VTX_TEX0    ){if(elm&VTX_TEX0    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_TEX1    ){if(elm&VTX_TEX1    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_TEX2    ){if(elm&VTX_TEX2    )return ofs; ofs+=SIZE(Vec2 );}
      if(_flag&VTX_MATRIX  ){if(elm&VTX_MATRIX  )return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_BLEND   ){if(elm&VTX_BLEND   )return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_SIZE    ){if(elm&VTX_SIZE    )return ofs; ofs+=SIZE(Flt  );}
      if(_flag&VTX_MATERIAL){if(elm&VTX_MATERIAL)return ofs; ofs+=SIZE(VecB4);}
      if(_flag&VTX_COLOR   ){if(elm&VTX_COLOR   )return ofs; ofs+=SIZE(VecB4);}
   }
   return -1;
}
/******************************************************************************/
MeshRender& MeshRender::del()
{
#if GL
   if(D.notShaderModelGLES2() && _vao) // delete VAO
   {
      SafeSyncLocker lock(D._lock);
      if(_vao)
      {
         if(D.created())
         {
         #if VAO_EXCLUSIVE
            if(!App.mainThread())VAOs.add(_vao);else // if this is not the main thread, then store it in container for future re-use
         #endif
               glDeleteVertexArrays(1, &_vao); // we can delete it
         }
        _vao=0; // clear while in lock
      }
   }
#endif
  _vb.del();
  _ib.del();
   Free(_bone_split);
   zero(); return T;
}
Bool MeshRender::setVF()
{
#if GL
   if(D.notShaderModelGLES2()) // create VAO
   {
      if(D.created())
      {
      #if VAO_EXCLUSIVE
         if(!App.mainThread())_vao_reset=true;else // if this is not the main thread, then we have to reset it later
      #endif
         {
            VtxFormatGL *temp=VtxFormats(VtxFormatKey(_flag, storageCompress() ? VTX_COMPRESS_NRM_TAN_BIN : 0))->vf; if(!temp)return false;
            if(!_vao)
            {
               SyncLocker lock(D._lock);
            #if VAO_EXCLUSIVE
               if(VAOs.elms())_vao=VAOs.pop();else // re-use if we have any
            #endif
               {
                  glGenVertexArrays(1, &_vao); // create new one
                  if(!_vao)return false;
               }
            }
            glBindVertexArray(_vao);
           _vb.set(); // these must be set after 'glBindVertexArray' and before 'enableSet'
           _ib.set(); // these must be set after 'glBindVertexArray' and before 'enableSet'
            REP(GL_VTX_NUM)glDisableVertexAttribArray(i); // first disable all, in case this VAO was set with other data before
            temp->enableSet(); // enable and set new data with VB and IB already set
         #if VAO_EXCLUSIVE
           _vao_reset=false; // we've just set it now, so clear reset
         #else
            glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread, no need to flush on exclusive mode, because there all VAO's are used only on the main thread
         #endif
         #if 0 // !! Don't do this, instead every time we want to bind some IB we use 'BindIndexBuffer' which disables VAO, also this method 'setVF' requires to have VAO already bound at the end so we don't have to bind it again in 'MeshRender.set' !!
            glBindVertexArray(0); // disable VAO so binding IB will not modify this VAO
         #endif
         }
         return true; // return success in both cases (!mainThread=reset later, and mainThread=created VAO)
      }
   }else
#endif
   {
     _vf=VtxFormats(VtxFormatKey(_flag, storageCompress() ? VTX_COMPRESS_NRM_TAN_BIN : 0))->vf;
   }
   return _vf!=null;
}
Bool MeshRender::create(Int vtxs, Int tris, UInt flag, Bool compress)
{ // avoid deleting at the start, instead, check if some members already match
   UInt compress_flag=(compress ? VTX_COMPRESS_NRM_TAN_BIN : 0);
   Bool same_format=(flag==T.flag() && compress==storageCompress()); // !! this must check for all parameters which are passed into 'VtxFormatKey' !!

   if((same_format && _vb.vtxs()==vtxs && !_vb._lock_mode) || _vb.create(vtxs  , flag         , compress_flag)) // do a separate check for '_vb' because its faster than 'create' method which may do some more checks for vtx size
   if(                                                        _ib.create(tris*3, vtxs<=0x10000               ))
   {
      T._storage=(compress ? MSHR_COMPRESS : 0)|(D.meshStorageSigned() ? MSHR_SIGNED : 0)|(D.meshBoneSplit() ? MSHR_BONE_SPLIT : 0);
      T._tris   =tris;
      T._flag   =flag;
      Free(_bone_split); _bone_splits=0;
      if(GL && D.notShaderModelGLES2())return setVF(); // set VAO
      return same_format || setVF(); // skip setting VF if we already have same format
   }
   del(); return false;
}
Bool MeshRender::createRaw(C MeshBase &src, UInt flag_and, Bool optimize, Bool compress)
{
   if(create(src.vtxs(), src.trisTotal(), src.flag()&flag_and&VTX_MSHR, compress))
   {
      if(Byte *v=vtxLock(LOCK_WRITE))
      {
         if(storageCompress())
         {
            if((flag()&(VTX_MSHR&~VTX_MATERIAL))==(VTX_POS|VTX_NRM|VTX_COLOR)) // optimized version for heightmaps (most heightmaps will have VTX_COLOR due to ambient occlusion) it's included because heightmaps can be created at runtime
            {
             C Vec   *vtx_pos     =                         src.vtx.pos     ();
             C Vec   *vtx_nrm     =                         src.vtx.nrm     ();
             C VecB4 *vtx_material=((flag()&VTX_MATERIAL) ? src.vtx.material() : null);
             C Color *vtx_color   =                         src.vtx.color   ();
               VecB4 (*NrmToByte4)(C Vec &v)=(storageSigned() ? NrmToSByte4 : NrmToUByte4);
               if(vtx_material)REPA(src.vtx)
               {
                  Set(v,            *vtx_pos     ++ );
                  Set(v, NrmToByte4(*vtx_nrm     ++));
                  Set(v,            *vtx_material++ );
                  Set(v,            *vtx_color   ++ );
               }else REPA(src.vtx)
               {
                  Set(v,            *vtx_pos  ++ );
                  Set(v, NrmToByte4(*vtx_nrm  ++));
                  Set(v,            *vtx_color++ );
               }
            }else
            {
             C Vec   *vtx_pos     =((flag()&VTX_POS     ) ? src.vtx.pos     () : null);
             C Vec   *vtx_nrm     =((flag()&VTX_NRM     ) ? src.vtx.nrm     () : null);
             C Vec   *vtx_tan     =((flag()&VTX_TAN_BIN ) ? src.vtx.tan     () : null);
             C Vec   *vtx_bin     =((flag()&VTX_TAN_BIN ) ? src.vtx.bin     () : null);
             C Vec   *vtx_hlp     =((flag()&VTX_HLP     ) ? src.vtx.hlp     () : null);
             C Vec2  *vtx_tex0    =((flag()&VTX_TEX0    ) ? src.vtx.tex0    () : null);
             C Vec2  *vtx_tex1    =((flag()&VTX_TEX1    ) ? src.vtx.tex1    () : null);
             C Vec2  *vtx_tex2    =((flag()&VTX_TEX2    ) ? src.vtx.tex2    () : null);
             C VecB4 *vtx_matrix  =((flag()&VTX_MATRIX  ) ? src.vtx.matrix  () : null);
             C VecB4 *vtx_blend   =((flag()&VTX_BLEND   ) ? src.vtx.blend   () : null);
             C Flt   *vtx_size    =((flag()&VTX_SIZE    ) ? src.vtx.size    () : null);
             C VecB4 *vtx_material=((flag()&VTX_MATERIAL) ? src.vtx.material() : null);
             C Color *vtx_color   =((flag()&VTX_COLOR   ) ? src.vtx.color   () : null);

               FREPA(src.vtx)
               {
                  Set(v, i, vtx_pos);
                  if(storageSigned())
                  {
                     if(vtx_nrm           )Set(v, NrmToSByte4(vtx_nrm[i]));
                     if(vtx_tan || vtx_bin)Set(v, TBNToSByte4(vtx_tan ? &vtx_tan[i] : null, vtx_bin ? &vtx_bin[i] : null, vtx_nrm ? &vtx_nrm[i] : null));
                  }else
                  {
                     if(vtx_nrm           )Set(v, NrmToUByte4(vtx_nrm[i]));
                     if(vtx_tan || vtx_bin)Set(v, TBNToUByte4(vtx_tan ? &vtx_tan[i] : null, vtx_bin ? &vtx_bin[i] : null, vtx_nrm ? &vtx_nrm[i] : null));
                  }
                  Set(v, i, vtx_hlp     );
                  Set(v, i, vtx_tex0    );
                  Set(v, i, vtx_tex1    );
                  Set(v, i, vtx_tex2    );
                  Set(v, i, vtx_matrix  );
                  Set(v, i, vtx_blend   );
                  Set(v, i, vtx_size    );
                  Set(v, i, vtx_material);
                  Set(v, i, vtx_color   );
               }
            }
         }else
         {
          C Vec   *vtx_pos     =((flag()&VTX_POS     ) ? src.vtx.pos     () : null);
          C Vec   *vtx_nrm     =((flag()&VTX_NRM     ) ? src.vtx.nrm     () : null);
          C Vec   *vtx_tan     =((flag()&VTX_TAN     ) ? src.vtx.tan     () : null);
          C Vec   *vtx_bin     =((flag()&VTX_BIN     ) ? src.vtx.bin     () : null);
          C Vec   *vtx_hlp     =((flag()&VTX_HLP     ) ? src.vtx.hlp     () : null);
          C Vec2  *vtx_tex0    =((flag()&VTX_TEX0    ) ? src.vtx.tex0    () : null);
          C Vec2  *vtx_tex1    =((flag()&VTX_TEX1    ) ? src.vtx.tex1    () : null);
          C Vec2  *vtx_tex2    =((flag()&VTX_TEX2    ) ? src.vtx.tex2    () : null);
          C VecB4 *vtx_matrix  =((flag()&VTX_MATRIX  ) ? src.vtx.matrix  () : null);
          C VecB4 *vtx_blend   =((flag()&VTX_BLEND   ) ? src.vtx.blend   () : null);
          C Flt   *vtx_size    =((flag()&VTX_SIZE    ) ? src.vtx.size    () : null);
          C VecB4 *vtx_material=((flag()&VTX_MATERIAL) ? src.vtx.material() : null);
          C Color *vtx_color   =((flag()&VTX_COLOR   ) ? src.vtx.color   () : null);

            FREPA(src.vtx)
            {
               Set(v, i, vtx_pos     );
               Set(v, i, vtx_nrm     );
               Set(v, i, vtx_tan     );
               Set(v, i, vtx_bin     );
               Set(v, i, vtx_hlp     );
               Set(v, i, vtx_tex0    );
               Set(v, i, vtx_tex1    );
               Set(v, i, vtx_tex2    );
               Set(v, i, vtx_matrix  );
               Set(v, i, vtx_blend   );
               Set(v, i, vtx_size    );
               Set(v, i, vtx_material);
               Set(v, i, vtx_color   );
            }
         }
         vtxUnlock();
      }

      if(Ptr ind=indLock(LOCK_WRITE))
      {
         SetFaceIndex(ind, src.tri.ind(), src.tris(), src.quad.ind(), src.quads(), _ib.bit16());
         indUnlock();
      }

      if(optimize)T.optimize();
      return true;
   }
   return false;
}
struct SplitPart
{
   Int  matrixes, temps;
   Bool matrix_used[256];
   Byte temp_matrix[4*4]; // max 4 quad_verts * 4 matrixes_per_vert

   void addTemp(Byte matrix)
   {
      REP(temps)if(temp_matrix[i]==matrix)return; // if already added then don't add anymore
      if(!InRange(temps, temp_matrix))Exit("SplitPart.addTemp"); // shouldn't happen
      temp_matrix[temps++]=matrix; // add to helper array
   }
   Bool canFit(VecB4 *matrix, VecB4 *weight, Int elms)
   {
      temps=0; // set helper number to zero
      REP(elms)
      {
         VecB4 m=matrix[i], w=weight[i];
         if(!matrix_used[m.x] && w.x)addTemp(m.x);
         if(!matrix_used[m.y] && w.y)addTemp(m.y);
         if(!matrix_used[m.z] && w.z)addTemp(m.z);
         if(!matrix_used[m.w] && w.w)addTemp(m.w);
      }
      return (matrixes+temps)<=MAX_MATRIX_HWMIN; // if amount of used matrixes along with new to be added is in range of supported matrixes by the gpu
   }
   void add(VecB4 *matrix, VecB4 *weight, Int elms)
   {
      REP(elms)
      {
         VecB4 m=matrix[i], w=weight[i];
         if(!matrix_used[m.x] && w.x){matrix_used[m.x]=true; matrixes++;}
         if(!matrix_used[m.y] && w.y){matrix_used[m.y]=true; matrixes++;}
         if(!matrix_used[m.z] && w.z){matrix_used[m.z]=true; matrixes++;}
         if(!matrix_used[m.w] && w.w){matrix_used[m.w]=true; matrixes++;}
      }
   }

   SplitPart() {matrixes=0; Zero(matrix_used);}
};
Bool MeshRender::create(C MeshBase &src, UInt flag_and, Bool optimize, Bool compress)
{
   if((flag_and&VTX_MATRIX) && src.vtx.matrix())REPA(src.vtx)
   {
    C VecB4 &m=src.vtx.matrix(i);
      if(m.x>=MAX_MATRIX_HWMIN  // we exceed the limit of available matrixes, so we need to create in parts
      || m.y>=MAX_MATRIX_HWMIN  // MAX_MATRIX_HWMIN must be checked instead of MAX_MATRIX_HW because we're preparing splits for all platforms (in affected platforms we would then just adjust vertex matrixes instead of recalculating the splits)
      || m.z>=MAX_MATRIX_HWMIN
      || m.w>=MAX_MATRIX_HWMIN)
      {
         Memb<SplitPart> splits;
         Memc<MeshBase > mshbs ;
         Mems<Int>  tri_split;  tri_split.setNum(src.tris ()); Memt<Bool>  tri_is;  tri_is.setNum(src.tris ());
         Mems<Int> quad_split; quad_split.setNum(src.quads()); Memt<Bool> quad_is; quad_is.setNum(src.quads());

         // set split index for each triangle and quad
         VecB4 weight[4]; // 3 vtxs in a tri and 4 vtxs in a quad
         REPAO(weight).set(255, 255, 255, 255); // assume that all are used
         FREPA(src.tri) // add in original order
         {
          C VecI &ind     = src.tri.ind(i);
            VecB4 matrix[]={src.vtx.matrix(ind.x), src.vtx.matrix(ind.y), src.vtx.matrix(ind.z)};
            if(src.vtx.blend())REPA(ind)weight[i]=src.vtx.blend(ind.c[i]); 
            Int   s=0; for(; s<splits.elms(); s++)if(splits[s].canFit(matrix, weight, Elms(matrix)))break; if(!InRange(s, splits))s=splits.addNum(1);
            splits[s].add(matrix, weight, Elms(matrix));
            tri_split[i]=s;
         }
         FREPA(src.quad) // add in original order
         {
          C VecI4 &ind     = src.quad.ind(i);
            VecB4  matrix[]={src.vtx.matrix(ind.x), src.vtx.matrix(ind.y), src.vtx.matrix(ind.z), src.vtx.matrix(ind.w)};
            if(src.vtx.blend())REPA(ind)weight[i]=src.vtx.blend(ind.c[i]); 
            Int    s=0; for(; s<splits.elms(); s++)if(splits[s].canFit(matrix, weight, Elms(matrix)))break; if(!InRange(s, splits))s=splits.addNum(1);
            splits[s].add(matrix, weight, Elms(matrix));
            quad_split[i]=s;
         }

         // create mesh and splits
         BoneSplit *bone_splits=AllocZero<BoneSplit>(splits.elms()); // AllocZero to zero all maps
         const Bool bone_split =D.meshBoneSplit();
         FREPAD(s, splits)
         {
            // copy mesh
            MeshBase &mshb=mshbs.New();
            REPA(src.tri ) tri_is[i]=( tri_split[i]==s);
            REPA(src.quad)quad_is[i]=(quad_split[i]==s);
            src.copyFace(mshb, null, tri_is, quad_is, flag_and);
            mshb.quadToTri(); // we need to call this at this stage, so triangle indexes will be correct for creating 1 mesh from split meshes

            // set split
            SplitPart &split=     splits[s];
            BoneSplit &bs   =bone_splits[s];
            Byte       real_to_split[256]; Zero(real_to_split);

            bs.vtxs =mshb.vtxs();
            bs.tris =mshb.tris();
            bs.bones=0;
            FREPA(split.matrix_used)if(split.matrix_used[i])
            {
                   real_to_split[i]=bs.bones;
               bs.split_to_real [bs.bones]=i;
               bs.bones++;
            }

            // remap vertex matrixes
            if(bone_split)REPA(mshb.vtx)
            {
               VecB4 &matrix=mshb.vtx.matrix(i);
               matrix.x=real_to_split[matrix.x];
               matrix.y=real_to_split[matrix.y];
               matrix.z=real_to_split[matrix.z];
               matrix.w=real_to_split[matrix.w];
            }
         }

         MeshBase temp; temp.create(mshbs.data(), mshbs.elms());
         if(createRaw(temp, ~0, false, compress)) // don't optimize yet, wait until splits are set
         {
            Free(T._bone_split)=bone_splits; T._bone_splits=splits.elms(); // remove old splits and replace them with new ones
            if(optimize)T.optimize();
            return true;
         }

         // free
         Free(bone_splits);
         return false;
      }
   }
   return createRaw(src, flag_and, optimize, compress);
}
Bool MeshRender::create(C MeshRender *src[], Int elms, UInt flag_and, Bool optimize, Bool compress)
{
   // check for bone splits
   REP(elms)if(C MeshRender *mesh=src[i])if(mesh->_bone_splits) // if any of the meshes have bone splits
   { // we need to convert to MeshBase first
      Memt<MeshBase> base; base.setNum(elms);
      REPA(base)if(C MeshRender *mesh=src[i])base[i].create(*mesh, flag_and);
      MeshBase all; all.create(base.data(), base.elms());
      return create(all, flag_and, optimize, compress);
   }

   // do fast merge
   Int  vtxs=0, tris=0;
   UInt flag_all=0;
   REP(elms)if(C MeshRender *mesh=src[i])
   {
      vtxs+=mesh->vtxs();
      tris+=mesh->tris();
      flag_all|=mesh->flag();
   }
   Bool ok=true;
   MeshRender temp; if(temp.create(vtxs, tris, flag_all&flag_and, compress)) // operate on 'temp' in case 'this' belongs to one of 'src' meshes
   {
      // vertexes
      if(Byte *v=temp.vtxLock(LOCK_WRITE))
      {
         FREP(elms)if(C MeshRender *mesh=src[i])
         {
            if(C Byte *src=mesh->vtxLockRead())
            {
               if(temp.flag()==mesh->flag() && temp.vtxSize()==mesh->vtxSize() && temp.storageCompress()==mesh->storageCompress())
               {
                  Int size=mesh->vtxs()*mesh->vtxSize();
                  CopyFast(v, src, size);
                  v+=size;
               }else
               {
                  Int vtx_pos     =mesh->vtxOfs(VTX_POS     ),
                      vtx_nrm     =mesh->vtxOfs(VTX_NRM     ),
                      vtx_tan     =mesh->vtxOfs(VTX_TAN     ),
                      vtx_bin     =mesh->vtxOfs(VTX_BIN     ),
                      vtx_hlp     =mesh->vtxOfs(VTX_HLP     ),
                      vtx_tex0    =mesh->vtxOfs(VTX_TEX0    ),
                      vtx_tex1    =mesh->vtxOfs(VTX_TEX1    ),
                      vtx_tex2    =mesh->vtxOfs(VTX_TEX2    ),
                      vtx_matrix  =mesh->vtxOfs(VTX_MATRIX  ),
                      vtx_blend   =mesh->vtxOfs(VTX_BLEND   ),
                      vtx_size    =mesh->vtxOfs(VTX_SIZE    ),
                      vtx_material=mesh->vtxOfs(VTX_MATERIAL),
                      vtx_color   =mesh->vtxOfs(VTX_COLOR   );
                  REP(mesh->vtxs())
                  {
                     if(temp.flag()&VTX_POS)if(vtx_pos>=0)Set(v, *(Vec*)(src+vtx_pos));else Set(v, VecZero);
                     if(temp.flag()&VTX_NRM)
                     {
                        if(temp.storageCompress())
                        {
                           if(vtx_nrm>=0)if(mesh->storageCompress())Set(v, *(VecB4*)(src+vtx_nrm));else Set(v, (temp.storageSigned() ? NrmToSByte4 : NrmToUByte4)(*(Vec*)(src+vtx_nrm)));else Set(v, VecB4(temp.storageSigned() ? 0 : 128));
                        }else
                        {
                           if(vtx_nrm>=0)if(mesh->storageCompress())Set(v, (mesh->storageSigned() ? SByte4ToNrm : UByte4ToNrm)(*(VecB4*)(src+vtx_nrm)));else Set(v, *(Vec*)(src+vtx_nrm));else Set(v, VecZero);
                        }
                     }
                     if(temp.flag()&VTX_TAN_BIN)
                     {
                        if(temp.storageCompress()) // set as 1 packed VecB4 TanBin
                        {
                           if(vtx_tan>=0)
                           {
                              if(mesh->storageCompress())Set(v, *(VecB4*)(src+vtx_tan));else
                              {
                                 Set(v, (temp.storageSigned() ? TBNToSByte4 : TBNToUByte4)((vtx_tan>=0) ? (Vec*)(src+vtx_tan) : null, (vtx_bin>=0) ? (Vec*)(src+vtx_bin) : null, (vtx_nrm>=0) ? (Vec*)(src+vtx_nrm) : null));
                              }
                           }else Set(v, VecB4(temp.storageSigned() ? 0 : 128));
                        }else
                        {
                           if(temp.flag()&VTX_TAN)
                           {
                              if(vtx_tan>=0)
                              {
                                 if(!mesh->storageCompress())Set(v, *(Vec*)(src+vtx_tan));else
                                 {
                                    Set(v, (mesh->storageSigned() ? SByte4ToNrm : UByte4ToNrm)(*(VecB4*)(src+vtx_tan)));
                                 }
                              }else Set(v, VecZero);
                           }
                           if(temp.flag()&VTX_BIN)
                           {
                              if(vtx_bin>=0)
                              {
                                 if(!mesh->storageCompress())Set(v, *(Vec*)(src+vtx_bin));else
                                 {
                                    Vec bin;
                                    (mesh->storageSigned() ? SByte4ToTan : UByte4ToTan) (*(VecB4*)(src+vtx_bin), null, &bin, (vtx_nrm>=0) ? &(mesh->storageSigned() ? SByte4ToNrm : UByte4ToNrm)(*(VecB4*)(src+vtx_nrm)) : null);
                                    Set(v, bin);
                                 }
                              }else Set(v, VecZero);
                           }
                        }
                     }
                     if(temp.flag()&VTX_HLP     )if(vtx_hlp     >=0)Set(v, *(Vec  *)(src+vtx_hlp     ));else Set(v, VecZero);
                     if(temp.flag()&VTX_TEX0    )if(vtx_tex0    >=0)Set(v, *(Vec2 *)(src+vtx_tex0    ));else Set(v, Vec2(0));
                     if(temp.flag()&VTX_TEX1    )if(vtx_tex1    >=0)Set(v, *(Vec2 *)(src+vtx_tex1    ));else Set(v, Vec2(0));
                     if(temp.flag()&VTX_TEX2    )if(vtx_tex2    >=0)Set(v, *(Vec2 *)(src+vtx_tex2    ));else Set(v, Vec2(0));
                     if(temp.flag()&VTX_MATRIX  )if(vtx_matrix  >=0)Set(v, *(VecB4*)(src+vtx_matrix  ));else Set(v, VecB4(  0, 0, 0, 0));
                     if(temp.flag()&VTX_BLEND   )if(vtx_blend   >=0)Set(v, *(VecB4*)(src+vtx_blend   ));else Set(v, VecB4(255, 0, 0, 0));
                     if(temp.flag()&VTX_SIZE    )if(vtx_size    >=0)Set(v, *(Flt  *)(src+vtx_size    ));else Set(v, Flt(0));
                     if(temp.flag()&VTX_MATERIAL)if(vtx_material>=0)Set(v, *(VecB4*)(src+vtx_material));else Set(v, VecB4(255, 0, 0, 0));
                     if(temp.flag()&VTX_COLOR   )if(vtx_color   >=0)Set(v, *(Color*)(src+vtx_color   ));else Set(v, WHITE);
                     src+=mesh->vtxSize();
                  }
               }
               mesh->vtxUnlock();
            }else ok=false;
         }
         temp.vtxUnlock();
      }else ok=false;

      // indexes
      if(Ptr ind=temp.indLock(LOCK_WRITE))
      {
         vtxs=0;
         VecUS *ind16=(temp.indBit16() ?        (VecUS*)ind : null);
         VecI  *ind32=(temp.indBit16() ? null : (VecI *)ind);
         FREP(elms)if(C MeshRender *mesh=src[i])
         {
            if(CPtr src=mesh->indLockRead())
            {
               if(mesh->indBit16())
               {
                C VecUS *s=(C VecUS*)src;
                  if(ind16)REP(mesh->tris())*ind16++=(*s++)+vtxs;else
                  if(ind32)REP(mesh->tris())*ind32++=(*s++)+vtxs;
               }else
               {
                C VecI *s=(C VecI*)src;
                  if(ind16)REP(mesh->tris())*ind16++=(*s++)+vtxs;else
                  if(ind32)REP(mesh->tris())*ind32++=(*s++)+vtxs;
               }
               mesh->indUnlock();
            }else ok=false;
            vtxs+=mesh->vtxs();
         }
         temp.indUnlock();
      }else ok=false;
      if(ok)
      {
         if(optimize)temp.optimize();
         Swap(temp, T);
      }else del();
      return ok;
   }
   return false;
}
Bool MeshRender::create(C MeshRender &src)
{
   if(this==&src)return true;

   del();

   if(_vb.create(src._vb))
   if(_ib.create(src._ib))
   {
     _storage=src._storage;
     _tris   =src._tris   ;
     _flag   =src._flag   ;
      if(GL && D.notShaderModelGLES2())setVF();else _vf=src._vf; // VAO

      // copy splits
      Alloc(_bone_split,     _bone_splits= src._bone_splits);
      CopyN(_bone_split, src._bone_split , src._bone_splits);
      return true;
   }
   return false;
}
#if 0 // DX9
ID3DXMesh* MeshRender::createDx9Mesh()
{
   Bool       ok  =false;
   ID3DXMesh *mesh=null;

   D._lock.on();
   if(Ptr src_vtx=vtxLock(LOCK_READ))
   {
      if(Ptr src_ind=indLock(LOCK_READ))
      {
         D3DVERTEXELEMENT9 ve[MAXD3DDECLLENGTH+1];
         if(SetVtxFormatFromVtxDecl(_vf, ve))
         {
            if(OK(D3DXCreateMesh(tris(), vtxs(), D3DXMESH_32BIT|D3DXMESH_SYSTEMMEM, ve, D3D, &mesh)))
            {
               DWORD *dest_id;
               Ptr    dest_vtx, dest_ind;
               if(OK(mesh->LockVertexBuffer(0, &dest_vtx)))
               {
                  if(OK(mesh->LockIndexBuffer(0, &dest_ind)))
                  {
                     if(OK(mesh->LockAttributeBuffer(0, &dest_id)))
                     {
                        // copy data
                        D._lock.off();
                                       CopyFast  (dest_vtx, src_vtx, mesh->GetNumVertices()*mesh->GetNumBytesPerVertex()); // vtx
                        if(_ib.bit16())Copy16To32(dest_ind, src_ind, mesh->GetNumFaces   ()*3                           ); // tri
                        else           Copy32To32(dest_ind, src_ind, mesh->GetNumFaces   ()*3                           ); // tri
                        FREP(_bone_splits)REPD(j, _bone_split[i].tris)*dest_id++=i;                                        // id
                        D._lock.on();

                        ok=true;

                        mesh->UnlockAttributeBuffer();
                     }
                     mesh->UnlockIndexBuffer();
                  }
                  mesh->UnlockVertexBuffer();
               }
            }
         }
         indUnlock();
      }
      vtxUnlock();
   }
   if(!ok)RELEASE(mesh);
   D._lock.off();

   return mesh;
}
#endif
/******************************************************************************/
C Byte* MeshRender::vtxLockedElm(UInt elm)C
{
   if(C Byte *data=vtxLockedData())
   {
      Int ofs=vtxOfs(elm);
      if( ofs>=0)return data+ofs;
   }
   return null;
}
/******************************************************************************/
// GET
/******************************************************************************/
Bool MeshRender::getBox(Box &box)C
{
   Int pos =vtxOfs(VTX_POS);
   if( pos>=0)if(C Byte *vtx=vtxLockRead())
   {
      vtx+=pos;
      box=*(Vec*)vtx; REP(vtxs()-1){vtx+=vtxSize(); box|=*(Vec*)vtx;}
      vtxUnlock();
      return true;
   }
   box.zero(); return false;
}
Flt MeshRender::area(Vec *center)C
{
   if(center)center->zero();
   Flt area=0;
   Int pos =vtxOfs(VTX_POS);
   if( pos>=0)if(C Byte *vtx=vtxLockRead())
   {
      vtx+=pos;
      if(CPtr ind=indLockRead())
      {
         if(indBit16())
         {
          C VecUS *tri=(C VecUS*)ind;
            REP(tris())
            {
               Tri t(*(Vec*)(vtx+tri->x*vtxSize()), *(Vec*)(vtx+tri->y*vtxSize()), *(Vec*)(vtx+tri->z*vtxSize())); tri++;
               Flt a=t.area();
                          area  +=a;
               if(center)*center+=a*t.center();
            }
         }else
         {
          C VecI *tri=(C VecI*)ind;
            REP(tris())
            {
               Tri t(*(Vec*)(vtx+tri->x*vtxSize()), *(Vec*)(vtx+tri->y*vtxSize()), *(Vec*)(vtx+tri->z*vtxSize())); tri++;
               Flt a=t.area();
                          area  +=a;
               if(center)*center+=a*t.center();
            }
         }
         indUnlock();
      }
      vtxUnlock();
   }
   if(center && area)*center/=area;
   return area;
}
/******************************************************************************/
// SET
/******************************************************************************
void MeshRender::setNormal()
{
   Int ofs_pos=vtxOfs(VTX_POS),
       ofs_nrm=vtxOfs(VTX_NRM);
   if( ofs_pos>=0 && ofs_nrm>=0)
   if(Byte *vtx=vtxLock())
   {
      if(Ptr ind=indLock(LOCK_READ))
      {
         Byte *vtx_pos=vtx+ofs_pos,
              *vtx_nrm=vtx+ofs_nrm;
         REP(vtxs())((Vec*)(vtx_nrm+i*vtxSize()))->zero();
         if(_ib.bit16())
         {
            U16 *d=(U16*)ind;
            REP(tris)
            {
               Vec &p0=*(Vec*)(vtx_pos+d[0]*vtxSize()),
                   &p1=*(Vec*)(vtx_pos+d[1]*vtxSize()),
                   &p2=*(Vec*)(vtx_pos+d[2]*vtxSize()),
                   &n0=*(Vec*)(vtx_nrm+d[0]*vtxSize()),
                   &n1=*(Vec*)(vtx_nrm+d[1]*vtxSize()),
                   &n2=*(Vec*)(vtx_nrm+d[2]*vtxSize()),
                   nrm=GetNormal      (p0, p1, p2);
               Flt  a0=AbsAngleBetween(p2, p0, p1),
                    a1=AbsAngleBetween(p0, p1, p2), a2=PI-a0-a1;
               n0+=a0*nrm;
               n1+=a1*nrm;
               n2+=a2*nrm;
               d+=3;
            }
         }else
         {
            U32 *d=(U32*)ind;
            REP(tris)
            {
               Vec &p0=*(Vec*)(vtx_pos+d[0]*vtxSize()),
                   &p1=*(Vec*)(vtx_pos+d[1]*vtxSize()),
                   &p2=*(Vec*)(vtx_pos+d[2]*vtxSize()),
                   &n0=*(Vec*)(vtx_nrm+d[0]*vtxSize()),
                   &n1=*(Vec*)(vtx_nrm+d[1]*vtxSize()),
                   &n2=*(Vec*)(vtx_nrm+d[2]*vtxSize()),
                   nrm=GetNormal      (p0, p1, p2);
               Flt  a0=AbsAngleBetween(p2, p0, p1),
                    a1=AbsAngleBetween(p0, p1, p2), a2=PI-a0-a1;
               n0+=a0*nrm;
               n1+=a1*nrm;
               n2+=a2*nrm;
               d+=3;
            }
         }
         REP(vtxs())((Vec*)(vtx_nrm+i*vtxSize()))->normalize();
         indUnlock();
      }
      vtxUnlock();
   }
}

// this code assumes that vertexes have been created in order, and as squares
void MeshRender::setNormalHeightmap(Int x, Int y)
{
   Int ofs_pos=vtxOfs(VTX_POS),
       ofs_nrm=vtxOfs(VTX_NRM);
   if( ofs_pos>=0 && ofs_nrm>=0)
   if(Byte *vtx=vtxLock())
   {
      Byte *vtx_pos=vtx+ofs_pos,
           *vtx_nrm=vtx+ofs_nrm;
      // center
      for(Int sy=1; sy<y-1; sy++)
      for(Int sx=1; sx<x-1; sx++)
      {
         Vec &l=*(Vec*)(vtx_pos+(sx + sy*x -1)*vtxSize()),
             &r=*(Vec*)(vtx_pos+(sx + sy*x +1)*vtxSize()),
             &d=*(Vec*)(vtx_pos+(sx + sy*x -x)*vtxSize()),
             &u=*(Vec*)(vtx_pos+(sx + sy*x +x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(sx + sy*x   )*vtxSize());
         n.x=(l.y-r.y)*x;
         n.z=(d.y-u.y)*y;
         n.y=2;
         n.normalize();
      }
      // left
      for(Int sy=1; sy<y-1; sy++)
      {
         Vec &c=*(Vec*)(vtx_pos+(sy*x   )*vtxSize()),
             &r=*(Vec*)(vtx_pos+(sy*x +1)*vtxSize()),
             &u=*(Vec*)(vtx_pos+(sy*x +x)*vtxSize()),
             &d=*(Vec*)(vtx_pos+(sy*x -x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(sy*x   )*vtxSize());
         n.x=(c.y-r.y)*(x*2);
         n.z=(d.y-u.y)*y;
         n.y=2;
         n.normalize();
      }
      // right
      for(Int sy=1; sy<y-1; sy++)
      {
         Vec &l=*(Vec*)(vtx_pos+(x-1 + sy*x -1)*vtxSize()),
             &c=*(Vec*)(vtx_pos+(x-1 + sy*x   )*vtxSize()),
             &u=*(Vec*)(vtx_pos+(x-1 + sy*x +x)*vtxSize()),
             &d=*(Vec*)(vtx_pos+(x-1 + sy*x -x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(x-1 + sy*x   )*vtxSize());
         n.x=(l.y-c.y)*(x*2);
         n.z=(d.y-u.y)*y;
         n.y=2;
         n.normalize();
      }
      // down
      for(Int sx=1; sx<x-1; sx++)
      {
         Vec &l=*(Vec*)(vtx_pos+(sx -1)*vtxSize()),
             &r=*(Vec*)(vtx_pos+(sx +1)*vtxSize()),
             &c=*(Vec*)(vtx_pos+(sx   )*vtxSize()),
             &u=*(Vec*)(vtx_pos+(sx +x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(sx   )*vtxSize());
         n.x=(l.y-r.y)*x;
         n.z=(c.y-u.y)*(y*2);
         n.y=2;
         n.normalize();
      }
      // up
      for(Int sx=1; sx<x-1; sx++)
      {
         Vec &l=*(Vec*)(vtx_pos+(sx + (y-1)*x -1)*vtxSize()),
             &r=*(Vec*)(vtx_pos+(sx + (y-1)*x +1)*vtxSize()),
             &d=*(Vec*)(vtx_pos+(sx + (y-1)*x -x)*vtxSize()),
             &c=*(Vec*)(vtx_pos+(sx + (y-1)*x   )*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(sx + (y-1)*x   )*vtxSize());
         n.x=(l.y-r.y)*x;
         n.z=(d.y-c.y)*(y*2);
         n.y=2;
         n.normalize();
      }
      // left-down
      {
         Vec &c=*(Vec*)(vtx_pos            ),
             &r=*(Vec*)(vtx_pos+1*vtxSize()),
             &u=*(Vec*)(vtx_pos+x*vtxSize()),
             &n=*(Vec*)(vtx_nrm            );
         n.x=(c.y-r.y)*x;
         n.z=(c.y-u.y)*y;
         n.y=1;
         n.normalize();
      }
      // right-down
      {
         Vec &c=*(Vec*)(vtx_pos+(x-1   )*vtxSize()),
             &l=*(Vec*)(vtx_pos+(x-1 -1)*vtxSize()),
             &u=*(Vec*)(vtx_pos+(x-1 +x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(x-1   )*vtxSize());
         n.x=(l.y-c.y)*x;
         n.z=(c.y-u.y)*y;
         n.y=1;
         n.normalize();
      }
      // left-up
      {
         Vec &c=*(Vec*)(vtx_pos+((y-1)*x   )*vtxSize()),
             &r=*(Vec*)(vtx_pos+((y-1)*x +1)*vtxSize()),
             &d=*(Vec*)(vtx_pos+((y-1)*x -x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+((y-1)*x   )*vtxSize());
         n.x=(c.y-r.y)*x;
         n.z=(d.y-c.y)*y;
         n.y=1;
         n.normalize();
      }
      // right-up
      {
         Vec &c=*(Vec*)(vtx_pos+(x-1 + (y-1)*x   )*vtxSize()),
             &l=*(Vec*)(vtx_pos+(x-1 + (y-1)*x -1)*vtxSize()),
             &d=*(Vec*)(vtx_pos+(x-1 + (y-1)*x -x)*vtxSize()),
             &n=*(Vec*)(vtx_nrm+(x-1 + (y-1)*x   )*vtxSize());
         n.x=(l.y-c.y)*x;
         n.z=(d.y-c.y)*y;
         n.y=1;
         n.normalize();
      }
      vtxUnlock();
   }
}
void MeshRender::setNormalHeightmap(Image &height,Image *l,Image *r,Image *b,Image *f)
{
   Int ofs_pos=vtxOfs(VTX_POS),
       ofs_nrm=vtxOfs(VTX_NRM);
   if( ofs_pos>=0 && ofs_nrm>=0)if(Byte *vtx=vtxLock())
   {
      Int dx=height.x(),
          dy=height.y();
      Vec *vtx_pos=(Vec*)(vtx+ofs_pos),
          *vtx_nrm=(Vec*)(vtx+ofs_nrm);
      REP(vtxs())
      {
         Int x=Round(vtx_pos->x*(dx-1)),
             y=Round(vtx_pos->z*(dy-1));

         if(x==0   )vtx_nrm->x=(l ? l    ->pixelF(dx-2, y)-height.pixelF(1,y) : (height.pixelF(   0, y)-height.pixelF(   1, y))*2);else
         if(x==dx-1)vtx_nrm->x=(r ? height.pixelF(dx-2, y)-r    ->pixelF(1,y) : (height.pixelF(dx-2, y)-height.pixelF(dx-1, y))*2);else
                    vtx_nrm->x=                                                 (height.pixelF( x-1, y)-height.pixelF( x+1, y))   ;

         if(y==0   )vtx_nrm->z=(b ? b    ->pixelF(x, dy-2)-height.pixelF(x,1) : (height.pixelF(x,    0)-height.pixelF(x,    1))*2);else
         if(y==dy-1)vtx_nrm->z=(f ? height.pixelF(x, dy-2)-f    ->pixelF(x,1) : (height.pixelF(x, dy-2)-height.pixelF(x, dy-1))*2);else
                    vtx_nrm->z=                                                 (height.pixelF(x,  y-1)-height.pixelF(x,  y+1))   ;

         vtx_nrm->x*=dx;
         vtx_nrm->z*=dy;
         vtx_nrm->y = 2;
         vtx_nrm->normalize();

         vtx_pos=(Vec*)(((Byte*)vtx_pos)+vtxSize());
         vtx_nrm=(Vec*)(((Byte*)vtx_nrm)+vtxSize());
      }
      vtxUnlock();
   }
}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
void MeshRender::texMove(C Vec2 &move, Byte tex_index)
{
   if(InRange(tex_index, 3) && move.any())
   {
      Int pos =vtxOfs((tex_index==0) ? VTX_TEX0 : (tex_index==1) ? VTX_TEX1 : VTX_TEX2);
      if( pos>=0)if(Byte *vtx=vtxLock())
      {
         vtx+=pos; REP(vtxs()){*(Vec2*)vtx+=move; vtx+=vtxSize();}
         vtxUnlock();
      }
   }
}
void MeshRender::texScale(C Vec2 &scale, Byte tex_index)
{
   if(InRange(tex_index, 3) && scale!=1)
   {
      Int pos =vtxOfs((tex_index==0) ? VTX_TEX0 : (tex_index==1) ? VTX_TEX1 : VTX_TEX2);
      if( pos>=0)if(Byte *vtx=vtxLock())
      {
         vtx+=pos; REP(vtxs()){*(Vec2*)vtx*=scale; vtx+=vtxSize();}
         vtxUnlock();
      }
   }
}
void MeshRender::texRotate(Flt angle, Byte tex_index)
{
   if(InRange(tex_index, 3) && angle)
   {
      Int pos =vtxOfs((tex_index==0) ? VTX_TEX0 : (tex_index==1) ? VTX_TEX1 : VTX_TEX2);
      if( pos>=0)if(Byte *vtx=vtxLock())
      {
         Flt cos, sin; CosSin(cos, sin, angle);
         vtx+=pos; REP(vtxs()){((Vec2*)vtx)->rotateCosSin(cos, sin); vtx+=vtxSize();}
         vtxUnlock();
      }
   }
}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
void MeshRender::scaleMove(C Vec &scale, C Vec &move)
{
   Int pos=vtxOfs(VTX_POS),
       hlp=vtxOfs(VTX_HLP);
   if(Byte *vtx=vtxLock())
   {
      REP(vtxs())
      {
         if(pos>=0){Vec &v=*(Vec*)(&vtx[pos]); v=v*scale+move;}
         if(hlp>=0){Vec &v=*(Vec*)(&vtx[hlp]); v=v*scale+move;}
         vtx+=vtxSize();
      }
      vtxUnlock();
   }
}
/*void MeshRender::transform(Matrix3 &matrix)
{
   if(mesh)
   {
      Int pos=vtxOfs(VTX_POS),
          nrm=vtxOfs(VTX_NRM),
          tan=vtxOfs(VTX_TAN),
          bin=vtxOfs(VTX_BIN),
          hlp=vtxOfs(VTX_HLP);
      if(Byte *vtx=vtxLock())
      {
         Matrix3 matrix_n=matrix; matrix_n.inverseScale();
         REP(vtxs())
         {
            if(pos>=0)*(Vec*)(&vtx[pos])*=matrix  ;
            if(hlp>=0)*(Vec*)(&vtx[hlp])*=matrix  ;
            if(nrm>=0)*(Vec*)(&vtx[nrm])*=matrix_n; normalize..?
            if(tan>=0)*(Vec*)(&vtx[tan])*=matrix_n;
            if(bin>=0)*(Vec*)(&vtx[bin])*=matrix_n;
            vtx+=vtxSize();
         }
         vtxUnlock();
      }
   }
}
void MeshRender::transform(Matrix &matrix)
{
   if(mesh)
   {
      Int pos=vtxOfs(VTX_POS),
          nrm=vtxOfs(VTX_NRM),
          tan=vtxOfs(VTX_TAN),
          bin=vtxOfs(VTX_BIN),
          hlp=vtxOfs(VTX_HLP);
      if(Byte *vtx=vtxLock())
      {
         Matrix3 matrix_n=matrix; matrix_n.inverseScale();
         REP(vtxs())
         {
            if(pos>=0)*(Vec*)(&vtx[pos])*=matrix  ;
            if(hlp>=0)*(Vec*)(&vtx[hlp])*=matrix  ;
            if(nrm>=0)*(Vec*)(&vtx[nrm])*=matrix_n; normalize?
            if(tan>=0)*(Vec*)(&vtx[tan])*=matrix_n;
            if(bin>=0)*(Vec*)(&vtx[bin])*=matrix_n;
            vtx+=vtxSize();
         }
         vtxUnlock();
      }
   }
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void MeshRender::adjustToPlatform()
{
   const Bool bone_split       =D.meshBoneSplit();
         Bool change_signed    =(storageCompress() && T.storageSigned   ()!=D.meshStorageSigned() && (_flag&(VTX_NRM|VTX_TAN))),
              change_bone_split=(_bone_split       && T.storageBoneSplit()!=bone_split            && (_flag&(VTX_MATRIX     )));

   if(change_signed || change_bone_split)
   {
      if(Byte *vtx=vtxLock())
      {
         Int nrm_ofs=vtxOfs(VTX_NRM   ),
             tan_ofs=vtxOfs(VTX_TAN   ),
            bone_ofs=vtxOfs(VTX_MATRIX);

         if(change_signed && (nrm_ofs>=0 || tan_ofs>=0))
         {
            Byte *v=vtx;
            REP(_vb.vtxs())
            {
            #if DEBUG // avoid debug runtime checks
               if(nrm_ofs>=0){VecB4 &v4=((VecB4&)v[nrm_ofs]); REPAO(v4.c)=(v4.c[i]+128)&0xFF;}
               if(tan_ofs>=0){VecB4 &v4=((VecB4&)v[tan_ofs]); REPAO(v4.c)=(v4.c[i]+128)&0xFF;}
            #else
               if(nrm_ofs>=0)((VecB4&)v[nrm_ofs])+=128;
               if(tan_ofs>=0)((VecB4&)v[tan_ofs])+=128;
            #endif
               v+=_vb._vtx_size;
            }
         }

         if(change_bone_split && bone_ofs>=0)
         {
            Byte *v=vtx+bone_ofs;
            FREP(_bone_splits)
            {
               BoneSplit &split=_bone_split[i];
               FREP(split.vtxs)
               {
                  VecB4 &matrix=*(VecB4*)v;
                  if(bone_split)
                  {
                     matrix.x=split.realToSplit0(matrix.x);
                     matrix.y=split.realToSplit0(matrix.y);
                     matrix.z=split.realToSplit0(matrix.z);
                     matrix.w=split.realToSplit0(matrix.w);
                  }else
                  {
                     matrix.x=split.split_to_real[matrix.x];
                     matrix.y=split.split_to_real[matrix.y];
                     matrix.z=split.split_to_real[matrix.z];
                     matrix.w=split.split_to_real[matrix.w];
                  }
                  v+=_vb._vtx_size;
               }
            }
         }

         vtxUnlock();

         FlagSet(_storage, MSHR_SIGNED    , D.meshStorageSigned());
         FlagSet(_storage, MSHR_BONE_SPLIT, bone_split           );
      }
   }
}
void MeshRender::    setUsedBones(Bool (&bones)[256])C {Zero(bones); includeUsedBones(bones);}
void MeshRender::includeUsedBones(Bool (&bones)[256])C
{
   Int matrix_ofs =vtxOfs(VTX_MATRIX);
   if( matrix_ofs>=0)if(C Byte *vtx=vtxLockRead())
   {
      Int    blend_ofs=vtxOfs(VTX_BLEND);
    C Byte *vtx_matrix=vtx+matrix_ofs, *vtx_blend=((blend_ofs>=0) ? vtx+blend_ofs : null);
      if(_bone_split && storageBoneSplit())FREP(_bone_splits)
      {
       C MeshRender::BoneSplit &split=_bone_split[i];
         FREP(split.vtxs)
         {
            REP(4) // 4 bytes in VecB4
            {
               Byte bone=split.split_to_real[vtx_matrix[i]];
               if(  bone)
               {
                  bone--;
                  if(vtx_blend ? vtx_blend[i] : true)bones[bone]=true;
               }
            }
                         vtx_matrix+=vtxSize();
            if(vtx_blend)vtx_blend +=vtxSize();
         }
      }else
      REP(vtxs())
      {
         REP(4) // 4 bytes in VecB4
         {
            Byte bone=vtx_matrix[i];
            if(  bone)
            {
               bone--;
               if(vtx_blend ? vtx_blend[i] : true)bones[bone]=true;
            }
         }
                      vtx_matrix+=vtxSize();
         if(vtx_blend)vtx_blend +=vtxSize();
      }
      vtxUnlock();
   }
}
/******************************************************************************/
C MeshRender& MeshRender::set()C
{
#if GL
   if(D.notShaderModelGLES2()) // VAO
   {
   #if VAO_EXCLUSIVE
      if(_vao_reset)
      {
         if(!ConstCast(T).setVF())Exit("Can't create VAO");
      }else // 'setVF' will already 'glBindVertexArray' the VAO
   #endif
         glBindVertexArray(_vao);
   }else
#endif
      {_vb.set(); _ib.set(); D.vf(_vf);} // OpenGL requires setting VF after VBO
   return T;
}
/******************************************************************************/
MeshRender& MeshRender::freeOpenGLESData()
{
  _vb.freeOpenGLESData();
  _ib.freeOpenGLESData();
   return T;
}
MeshRender& MeshRender::operator =(C MeshRender &src) {create(src); return T;}
MeshRender& MeshRender::operator+=(C MeshRender &src)
{
   if(src.is())
   {
    C MeshRender *meshes[]={this, &src};
      create(meshes, Elms(meshes));
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
