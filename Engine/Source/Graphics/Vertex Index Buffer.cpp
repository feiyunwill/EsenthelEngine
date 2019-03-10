/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************

   GL_ES does not support VI._vtx_drawing because:
      -'glDrawElements' does not have place for '_vtx_drawing'
      -'glDrawElementsBaseVertex' is needed but requires GLES 3.2 TODO: add in the future

   'glMapBufferRange' requires GL3.0 and GLES3.0

   Can't use GL_MAP_PERSISTENT_BIT because it requires GL4.4+

   GL ES may not support Half / F16 / GL_HALF_FLOAT / Float16 !!

/******************************************************************************/
// define following using defines instead of enum, so they can be used in preprocessor
#if DX9 || DX11 || DX12
   #define MEM     (32*1024)
   #define BUFFERS 1
#elif GL
   #define GL_LOCK_SUB                 0
   #define GL_LOCK_SUB_RESET_PART      1
   #define GL_LOCK_SUB_RESET_FULL      2
   #define GL_LOCK_SUB_RESET_PART_FROM 3
   #define GL_LOCK_SUB_RESET_FULL_FROM 4
   #define GL_LOCK_SUB_RING            5
   #define GL_LOCK_SUB_RING_RESET      6
   #define GL_LOCK_SUB_RING_RESET_FROM 7
   #define GL_LOCK_MAP                 8
   #define GL_LOCK_MAP_RING            9
   #define GL_LOCK_NUM                10

   // TODO: test on newer hardware
   #if WINDOWS // tested on GeForce 650m GT
      #define MEM        (32*1024)
      #define BUFFERS    1
      #define GL_VI_LOCK GL_LOCK_MAP_RING
   #elif MAC // tested on MacBook Air Intel HD 5000
      #define MEM        (32*1024)
      #define BUFFERS    1
      #define GL_VI_LOCK GL_LOCK_SUB_RESET_PART_FROM // perf. similar to GL_LOCK_MAP
   #elif LINUX // untested
      #define MEM        (32*1024)
      #define BUFFERS    1
      #define GL_VI_LOCK GL_LOCK_MAP_RING
   #elif ANDROID // tested on Note 4
      #define MEM        (32*1024)
      #define BUFFERS    1
      #define GL_VI_LOCK GL_LOCK_SUB_RESET_PART_FROM // perf. similar to GL_LOCK_MAP (however on GLES2 map is not supported and defaults to slow GL_LOCK_SUB)
   #elif IOS // tested on iPad Mini 2
      #define MEM        (8*1024)
      #define BUFFERS    1024
      #define GL_VI_LOCK GL_LOCK_SUB
   #elif WEB // untested
      #define MEM        (32*1024)
      #define BUFFERS    1
      #define GL_VI_LOCK GL_LOCK_SUB_RESET_PART_FROM // Map is not available on WebGL - https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.14
   #else
      #error
   #endif
#endif

#if GL
   #define BUFFERS_USE BUFFERS // 'BUFFERS' is the max amount of buffers allocated at app startup, but 'BUFFERS_USE' is the amount we want to use (can be lower for resting)

   #define GL_DYNAMIC GL_STREAM_DRAW // same performance as GL_DYNAMIC_DRAW

   #define GL_WRITE_ONLY 0x88B9
#endif

#if 0 // Test
   #pragma message("!! Warning: Use this only for debugging !!")

   // set defaults
   #ifndef BUFFERS
      #define BUFFERS 1
   #endif
   #ifndef    GL_VI_LOCK
      #define GL_VI_LOCK GL_LOCK_SUB
   #endif
   Int VIMem    =MEM;
   Int VIBuffers=BUFFERS;
   Int VILock   =GL_VI_LOCK;

   // clear defaults and make them variable
   #undef MEM
   #undef BUFFERS
   #undef BUFFERS_USE
   #undef GL_VI_LOCK
   #define MEM         VIMem
   #define BUFFERS     1024
   #define BUFFERS_USE VIBuffers
   #define GL_VI_LOCK  VILock

   void VISet(Int mem, Int buffers, Int lock)
   {
      Clamp(mem    , 1024, 1024*1024);
      Clamp(buffers, 1, BUFFERS);
      Clamp(lock   , 0, GL_LOCK_NUM-1);
      if(VIMem!=mem || VIBuffers!=buffers || VILock!=lock)
      {
         VI.lost(); // call before changing params
         VIMem    =mem;
         VIBuffers=buffers;
         VILock   =lock;
         VI.reset();
      }
   }
#endif

#if GL
   #define GL_RING (GL_VI_LOCK==GL_LOCK_SUB_RING || GL_VI_LOCK==GL_LOCK_SUB_RING_RESET || GL_VI_LOCK==GL_LOCK_SUB_RING_RESET_FROM || GL_VI_LOCK==GL_LOCK_MAP_RING)

   static INLINE Bool IsMap(Bool dynamic) {return GL_VI_LOCK>=GL_LOCK_MAP && D.notShaderModelGLES2() && dynamic;}
#endif

#if BUFFERS>1
   #if GL
      static VtxFormatGL *VF;
   #endif
   static VtxBuf VB[BUFFERS], *VI_vb=&VB[0];
 //static IndBuf IB[BUFFERS], *VI_ib=&IB[0];
#endif
/******************************************************************************/
static Int Compare(C VtxFormatKey &a, C VtxFormatKey &b)
{
   if(a.flag    <b.flag    )return -1;
   if(a.flag    >b.flag    )return +1;
   if(a.compress<b.compress)return -1;
   if(a.compress>b.compress)return +1;
                            return  0;
}
static Bool Create(VtxFormat &vf, C VtxFormatKey &key, Ptr) {return vf.create(key.flag, key.compress);}
/******************************************************************************/
IndBuf    IndBuf16384Quads, IndBufBorder, IndBufPanel, IndBufPanelEx, IndBufRectBorder, IndBufRectShaded;
VtxIndBuf VI;
ThreadSafeMap<VtxFormatKey, VtxFormat> VtxFormats(Compare, Create);
/******************************************************************************
255.0 scales (and not 256.0) are correct:
   https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059%28v=vs.85%29.aspx
   https://msdn.microsoft.com/en-us/library/windows/desktop/bb322868%28v=vs.85%29.aspx
The following correspond to 8-bit mapping of -1..1 float
/******************************************************************************/
#define UNSIGNED_N1  0 // -1
#define UNSIGNED_0 128 // ~0, there is no exact zero, because 127/255.0=0.498039216, 128/255.0=0.501960784, and both are offsetted by -0.5, giving -0.001960784 and 0.001960784, however 128 is chosen to achieve same value as signed version, because converting signed/unsigned is done by adding 128, then Byte(UNSIGNED_0+128)==0 give signed zero, an exact match
#define UNSIGNED_1 255 // +1
#define UNSIGNED_ZP(x) ((x)>=UNSIGNED_0) // if zero or positive

#define SIGNED_N1 128 // -1, this could also be 129 because both values give -1, however we want same value as unsigned version, because converting signed/unsigned is done by adding 128
#define SIGNED_0    0 //  0
#define SIGNED_1  127 // +1
#define SIGNED_ZP(x) ((x)<=SIGNED_1) // if zero or positive

Vec  UByte4ToNrm(C VecB4 &v) {return !Vec(UByteToSFlt(v.x), UByteToSFlt(v.y), UByteToSFlt(v.z));}
void UByte4ToTan(C VecB4 &v, Vec *tan, Vec *bin, C Vec *nrm)
{
   Vec t=UByte4ToNrm(v);
   if( tan)*tan=t;
   if( bin)
   {
      if(nrm)*bin=Cross(*nrm, t)*SignBool(UNSIGNED_ZP(v.w));
      else   *bin=PerpN(t);
   }
}
VecB4 NrmToUByte4(C Vec &v) {return VecB4(SFltToUByte(v.x), SFltToUByte(v.y), SFltToUByte(v.z), UNSIGNED_0);}
VecB4 TanToUByte4(C Vec &v) {return VecB4(SFltToUByte(v.x), SFltToUByte(v.y), SFltToUByte(v.z), UNSIGNED_1);}
VecB4 TBNToUByte4(C Vec *tan, C Vec *bin, C Vec *nrm)
{
   Vec tan_vec;
   if(tan       )tan_vec=*tan;else
   if(bin && nrm)tan_vec=Cross(*bin, *nrm);else return VecB4(UNSIGNED_0, UNSIGNED_0, UNSIGNED_0, UNSIGNED_0);

   VecB4  t=NrmToUByte4(tan_vec); t.w=((tan && bin && nrm && Dot(*tan, Cross(*bin, *nrm))<0) ? UNSIGNED_N1 : UNSIGNED_1);
   return t;
}

Vec  SByte4ToNrm(C VecB4 &v) {return !Vec(SByteToSFlt(v.x), SByteToSFlt(v.y), SByteToSFlt(v.z));}
void SByte4ToTan(C VecB4 &v, Vec *tan, Vec *bin, C Vec *nrm)
{
   Vec t=SByte4ToNrm(v);
   if( tan)*tan=t;
   if( bin)
   {
      if(nrm)*bin=Cross(*nrm, t)*SignBool(SIGNED_ZP(v.w));
      else   *bin=PerpN(t);
   }
}
VecB4 NrmToSByte4(C Vec &v) {return VecB4(SFltToSByte(v.x), SFltToSByte(v.y), SFltToSByte(v.z), SIGNED_0);}
VecB4 TanToSByte4(C Vec &v) {return VecB4(SFltToSByte(v.x), SFltToSByte(v.y), SFltToSByte(v.z), SIGNED_1);}
VecB4 TBNToSByte4(C Vec *tan, C Vec *bin, C Vec *nrm)
{
   Vec tan_vec;
   if(tan       )tan_vec=*tan;else
   if(bin && nrm)tan_vec=Cross(*bin, *nrm);else return VecB4(SIGNED_0, SIGNED_0, SIGNED_0, SIGNED_0);

   VecB4  t=NrmToSByte4(tan_vec); t.w=((tan && bin && nrm && Dot(*tan, Cross(*bin, *nrm))<0) ? SIGNED_N1 : SIGNED_1);
   return t;
}
/******************************************************************************/
// VERTEX FORMAT
/******************************************************************************/
#if DX9
static inline void Set(D3DVERTEXELEMENT9 &ve, Int offset, Byte type, Byte usage, Byte index=0)
{
   ve.Stream=0;
   ve.Offset=offset;
   ve.Type  =type;
   ve.Method=D3DDECLMETHOD_DEFAULT;
   ve.Usage =usage;
   ve.UsageIndex=index;
}
static inline void End(D3DVERTEXELEMENT9 &ve)
{
   ve.Stream=0xFF;
   ve.Offset=0;
   ve.Type  =D3DDECLTYPE_UNUSED;
   ve.Method=0;
   ve.Usage =0;
   ve.UsageIndex=0;
}
Bool SetVtxFormatFromVtxDecl(IDirect3DVertexDeclaration9 *vf, D3DVERTEXELEMENT9 (&ve)[MAX_FVF_DECL_SIZE])
{
   UINT elements=0;
   return vf ? OK(vf->GetDeclaration(ve, &elements)) : false;
}
#elif DX11
static inline void Set(D3D11_INPUT_ELEMENT_DESC &ve, Int offset, DXGI_FORMAT format, CChar8 *name, Byte index=0)
{
   ve.AlignedByteOffset=offset;
   ve.Format=format;
   ve.InputSlot=0;
   ve.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
   ve.InstanceDataStepRate=0;
   ve.SemanticIndex=index;
   ve.SemanticName=name;
}
#endif
/******************************************************************************/
VtxFormat& VtxFormat::del()
{
   if(vf)
   {
      SafeSyncLocker lock(D._lock);
      if(D._vf==vf)D.vf(null);
   #if DX9 || DX11
      if(vf){if(D.created())vf->Release(); vf=null;} // clear while in lock
   #elif GL
      Delete(vf);
   #endif
   }
   return T;
}
#if DX9
Bool VtxFormat::create(D3DVERTEXELEMENT9 ve[])
{
   SyncLocker locker(D._lock);
   del();
   if(D3D)return OK(D3D->CreateVertexDeclaration(ve, &vf));
   return false;
}
#elif DX11
// following 'VS_Code' is a byte code of a vertex shader from "DX10+ Input Layout.cpp", look around the sources for 'VS_Code' name on how to obtain the code (the code needs to be updated everytime VtxInput is changed)
static Byte VS_Code[784]={68,88,66,67,63,223,95,34,222,170,1,4,254,68,111,21,173,124,17,124,1,0,0,0,16,3,0,0,5,0,0,0,52,0,0,0,128,0,0,0,28,2,0,0,80,2,0,0,148,2,0,0,82,68,69,70,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,0,0,0,0,4,254,255,0,145,0,0,28,0,0,0,77,105,99,114,111,115,111,102,116,32,40,82,41,32,72,76,83,76,32,83,104,97,100,101,114,32,67,111,109,112,105,108,101,114,32,49,48,46,49,0,73,83,71,78,148,1,0,0,13,0,0,0,8,0,0,0,64,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,15,15,0,0,64,1,0,0,1,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,7,0,0,0,73,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,2,0,0,0,7,0,0,0,80,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,3,0,0,0,15,0,0,0,88,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,4,0,0,0,3,0,0,0,88,1,0,0,1,0,0,0,0,0,0,0,3,0,0,0,5,0,0,0,3,0,0,0,88,1,0,0,2,0,0,0,0,0,0,0,3,0,0,0,6,0,0,0,3,0,0,0,97,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,7,0,0,0,1,0,0,0,103,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,8,0,0,0,15,0,0,0,116,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,9,0,0,0,15,0,0,0,128,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,10,0,0,0,15,0,0,0,128,1,0,0,1,0,0,0,0,0,0,0,3,0,0,0,11,0,0,0,15,0,0,0,134,1,0,0,0,0,0,0,8,0,0,0,1,0,0,0,12,0,0,0,1,0,0,0,80,79,83,73,84,73,79,78,0,78,79,82,77,65,76,0,84,65,78,71,69,78,84,0,84,69,88,67,79,79,82,68,0,80,83,73,90,69,0,66,76,69,78,68,73,78,68,73,67,69,83,0,66,76,69,78,68,87,69,73,71,72,84,0,67,79,76,79,82,0,83,86,95,73,110,115,116,97,110,99,101,73,68,0,79,83,71,78,44,0,0,0,1,0,0,0,8,0,0,0,32,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,0,15,0,0,0,83,86,95,80,111,115,105,116,105,111,110,0,83,72,68,82,60,0,0,0,64,0,1,0,15,0,0,0,95,0,0,3,242,16,16,0,0,0,0,0,103,0,0,4,242,32,16,0,0,0,0,0,1,0,0,0,54,0,0,5,242,32,16,0,0,0,0,0,70,30,16,0,0,0,0,0,62,0,0,1,83,84,65,84,116,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
Bool VtxFormat::create(D3D11_INPUT_ELEMENT_DESC ve[], Int elms)
{
 //SyncLocker locker(D._lock); lock not needed for DX11 'D3D'
   del();
   if(D3D)return OK(D3D->CreateInputLayout(ve, elms, VS_Code, Elms(VS_Code), &vf));
   return false;
}
#elif GL
GL_VTX_SEMANTIC VtxSemanticToIndex(Int semantic)
{
   if(D._max_vtx_attribs<=semantic) // if GPU doesn't support this semantic
   {
      switch(semantic)
      {
         case GL_VTX_MATERIAL: semantic=GL_VTX_BONE    ; break; // re-use the same index, this assumes that material is used only for terrain, and no terrain is skinned
         case GL_VTX_HLP     : semantic=GL_VTX_TEX1    ; break; // re-use the same index, this assumes that (helper position used for animating tree-leafs/grass) will not have tex1
         case GL_VTX_SIZE    : semantic=GL_VTX_WEIGHT  ; break; // re-use the same index, this assumes that (helper position used for animating tree-leafs/grass) will not have bone weights
         case GL_VTX_TEX2    : semantic=GL_VTX_MATERIAL; break; // re-use the same index, this assumes that shaders using 3rd set of UV's don't use material blending
      }
      if(D._max_vtx_attribs<=semantic)semantic=D._max_vtx_attribs-1; // if it still doesn't support it
   }
   return GL_VTX_SEMANTIC(semantic);
}
void BindIndexBuffer(UInt buf)
{
   if(D.notShaderModelGLES2())glBindVertexArray(0); // !! have to clear VAO before calling "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER" to make sure it won't modify it, not needed for 'GL_ARRAY_BUFFER' because those are bound only with 'glVertexAttribPointer' !!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
}
void VtxFormatGL::del()
{
   if(vao)
   {
      SafeSyncLocker lock(D._lock); if(D.created())glDeleteVertexArrays(1, &vao); vao=0; // this method will be called only on the main thread (in the 'VtxFormats.del'), so it's OK to always call 'glDeleteVertexArrays', clear while in lock
   }
   elms.del();
}
Bool VtxFormatGL::create(C MemPtrN<Elm, 32> &elms)
{
   T.elms    =elms;
   T.vtx_size=0;
   FREPA(T.elms)
   {
      Byte             component_size;
      switch(T.elms[i].component_type)
      {
         case GL_BYTE      : case GL_UNSIGNED_BYTE : component_size=1; break;
         case GL_SHORT     : case GL_UNSIGNED_SHORT: component_size=2; break;
         case GL_HALF_FLOAT:                         component_size=2; break;
         case GL_INT       : case GL_UNSIGNED_INT  : component_size=4; break;
         case GL_FLOAT     :                         component_size=4; break;
      #ifdef  GL_DOUBLE
         case GL_DOUBLE    :                         component_size=8; break;
      #endif
         default           : Exit("Unrecognized OpenGL Component Type"); return false;
      }
      T.elms[i].semantic=VtxSemanticToIndex(T.elms[i].semantic);
      T.vtx_size+=component_size*T.elms[i].component_num;
   }
   return true;
}
void VtxFormatGL::disable()C
{
   REPA(elms)glDisableVertexAttribArray(elms[i].semantic);
}
void VtxFormatGL::enableSet()C
{
   REPA(elms)
   {
    C Elm &ve=elms[i];
      glEnableVertexAttribArray(ve.semantic);
      glVertexAttribPointer    (ve.semantic, ve.component_num, ve.component_type, ve.component_normalize, vtx_size, (CPtr)ve.offset);
   }
}
void VtxFormatGL::bind(C VtxBuf &vb) // this is called only on the main thread
{
   if(D.notShaderModelGLES2()) // VAO
   {
   #if GL_LOCK
      SyncLocker lock(D._lock);
   #endif
      if(!vao)
      {
         glGenVertexArrays(1, &vao);
         if(!vao)Exit("Can't create VAO");
      }
      glBindVertexArray(vao);
      // no need to do any disabling, because once vtx format is created, its members are always the same, we just need to 'enableSet' to the new VBO
      vb.set(); // have to call this first
      enableSet();
      glBindVertexArray(0); // disable VAO so binding IB will not modify this VAO, this is not strictly needed, because all IB bindings use either 'BindIndexBuffer' which clears VAO, or are done during drawing when correct VAO is already set, but leave it just in case
   }
}
Bool VtxFormat::create(C MemPtrN<VtxFormatGL::Elm, 32> &elms) {if(!vf)New(vf); return vf->create(elms);}
void VtxFormat::bind(C VtxBuf &vb) {if(vf)vf->bind(vb);}
#endif
Bool VtxFormat::create(UInt flag, UInt compress)
{
   // TODO: use R10G10B10A2 for compressed Nrm and Tan, however there's no signed format for that, so probably have to forget it
#if DX9
   D3DVERTEXELEMENT9 ve[MAX_FVF_DECL_SIZE];
   Int i=0, ofs=0;

                                     if(flag& VTX_POS     ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT3   , D3DDECLUSAGE_POSITION    , 0); ofs+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_NRM    ){if(flag& VTX_NRM     ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_NORMAL         ); ofs+=SIZE(VecB4);}}
   else                             {if(flag& VTX_NRM     ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT3   , D3DDECLUSAGE_NORMAL         ); ofs+=SIZE(Vec  );}}

   if(compress&VTX_COMPRESS_TAN_BIN){if(flag&VTX_TAN_BIN  ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_TANGENT        ); ofs+=SIZE(VecB4);}}
   else                             {if(flag&VTX_TAN      ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT3   , D3DDECLUSAGE_TANGENT        ); ofs+=SIZE(Vec  );}
                                     if(flag&VTX_BIN      ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT3   , D3DDECLUSAGE_BINORMAL       ); ofs+=SIZE(Vec  );}}

                                     if(flag& VTX_HLP     ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT3   , D3DDECLUSAGE_POSITION    , 1); ofs+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_TEX_8  ){if(flag& VTX_TEX0    ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_TEXCOORD    , 0); ofs+=SIZE(VecB4);}
                                     if(flag& VTX_TEX1    ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_TEXCOORD    , 1); ofs+=SIZE(VecB4);}
                                     if(flag& VTX_TEX2    ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_TEXCOORD    , 2); ofs+=SIZE(VecB4);}}else
   if(compress&VTX_COMPRESS_TEX    ){if(flag& VTX_TEX0    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT16_2, D3DDECLUSAGE_TEXCOORD    , 0); ofs+=SIZE(VecH2);}
                                     if(flag& VTX_TEX1    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT16_2, D3DDECLUSAGE_TEXCOORD    , 1); ofs+=SIZE(VecH2);}
                                     if(flag& VTX_TEX2    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT16_2, D3DDECLUSAGE_TEXCOORD    , 2); ofs+=SIZE(VecH2);}}else
                                    {if(flag& VTX_TEX0    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT2   , D3DDECLUSAGE_TEXCOORD    , 0); ofs+=SIZE(Vec2 );}
                                     if(flag& VTX_TEX1    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT2   , D3DDECLUSAGE_TEXCOORD    , 1); ofs+=SIZE(Vec2 );}
                                     if(flag& VTX_TEX2    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT2   , D3DDECLUSAGE_TEXCOORD    , 2); ofs+=SIZE(Vec2 );}}

                                     if(flag& VTX_MATRIX  ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4   , D3DDECLUSAGE_BLENDINDICES   ); ofs+=SIZE(VecB4);}
                                     if(flag& VTX_BLEND   ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_BLENDWEIGHT    ); ofs+=SIZE(VecB4);}
                                     if(flag& VTX_SIZE    ){Set(ve[i++], ofs, D3DDECLTYPE_FLOAT1   , D3DDECLUSAGE_PSIZE          ); ofs+=SIZE(Flt  );}
                                     if(flag& VTX_MATERIAL){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_COLOR       , 0); ofs+=SIZE(VecB4);}
                                     if(flag& VTX_COLOR   ){Set(ve[i++], ofs, D3DDECLTYPE_UBYTE4N  , D3DDECLUSAGE_COLOR       , 1); ofs+=SIZE(Color);}
                                                            End(ve[i  ]);
   return create(ve);
#elif DX11
   D3D11_INPUT_ELEMENT_DESC ve[D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT];
   const DXGI_FORMAT        nrm_tan=(D.meshStorageSigned() ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_UNORM);
   Int i=0, ofs=0;
                                     Set(ve[i++], ofs, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION"    , 0); if(flag&VTX_POS     )ofs+=SIZE(Vec  );

   if(compress&VTX_COMPRESS_NRM    ){Set(ve[i++], ofs, nrm_tan                    , "NORMAL"      , 0); if(flag&VTX_NRM     )ofs+=SIZE(VecB4);}
   else                             {Set(ve[i++], ofs, DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL"      , 0); if(flag&VTX_NRM     )ofs+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_TAN_BIN){Set(ve[i++], ofs, nrm_tan                    , "TANGENT"     , 0); if(flag&VTX_TAN_BIN )ofs+=SIZE(VecB4);}
   else                             {Set(ve[i++], ofs, DXGI_FORMAT_R32G32B32_FLOAT, "TANGENT"     , 0); if(flag&VTX_TAN     )ofs+=SIZE(Vec  );
                                     Set(ve[i++], ofs, DXGI_FORMAT_R32G32B32_FLOAT, "BINORMAL"    , 0); if(flag&VTX_BIN     )ofs+=SIZE(Vec  );}

                                     Set(ve[i++], ofs, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION"    , 1); if(flag&VTX_HLP     )ofs+=SIZE(Vec  );

   if(compress&VTX_COMPRESS_TEX_8  ){Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "TEXCOORD"    , 0); if(flag&VTX_TEX0    )ofs+=SIZE(VecB4);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "TEXCOORD"    , 1); if(flag&VTX_TEX1    )ofs+=SIZE(VecB4);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "TEXCOORD"    , 2); if(flag&VTX_TEX2    )ofs+=SIZE(VecB4);}else
   if(compress&VTX_COMPRESS_TEX    ){Set(ve[i++], ofs, DXGI_FORMAT_R16G16_FLOAT   , "TEXCOORD"    , 0); if(flag&VTX_TEX0    )ofs+=SIZE(VecH2);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R16G16_FLOAT   , "TEXCOORD"    , 1); if(flag&VTX_TEX1    )ofs+=SIZE(VecH2);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R16G16_FLOAT   , "TEXCOORD"    , 2); if(flag&VTX_TEX2    )ofs+=SIZE(VecH2);}else
                                    {Set(ve[i++], ofs, DXGI_FORMAT_R32G32_FLOAT   , "TEXCOORD"    , 0); if(flag&VTX_TEX0    )ofs+=SIZE(Vec2 );
                                     Set(ve[i++], ofs, DXGI_FORMAT_R32G32_FLOAT   , "TEXCOORD"    , 1); if(flag&VTX_TEX1    )ofs+=SIZE(Vec2 );
                                     Set(ve[i++], ofs, DXGI_FORMAT_R32G32_FLOAT   , "TEXCOORD"    , 2); if(flag&VTX_TEX2    )ofs+=SIZE(Vec2 );}

                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UINT  , "BLENDINDICES", 0); if(flag&VTX_MATRIX  )ofs+=SIZE(VecB4);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "BLENDWEIGHT" , 0); if(flag&VTX_BLEND   )ofs+=SIZE(VecB4);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R32_FLOAT      , "PSIZE"       , 0); if(flag&VTX_SIZE    )ofs+=SIZE(Flt  );
                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "COLOR"       , 0); if(flag&VTX_MATERIAL)ofs+=SIZE(VecB4);
                                     Set(ve[i++], ofs, DXGI_FORMAT_R8G8B8A8_UNORM , "COLOR"       , 1); if(flag&VTX_COLOR   )ofs+=SIZE(Color);
   return create(ve, i);
#elif GL
   MemtN<VtxFormatGL::Elm, 32> ve;
   Int ofs=0;
                                     if(flag&VTX_POS     ){ve.New().set(GL_VTX_POS     , 3, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_NRM    ){if(flag&VTX_NRM     ){ve.New().set(GL_VTX_NRM     , 4, GL_BYTE         , true , ofs); ofs+=SIZE(VecB4);}}
   else                             {if(flag&VTX_NRM     ){ve.New().set(GL_VTX_NRM     , 3, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec  );}}

   if(compress&VTX_COMPRESS_TAN_BIN){if(flag&VTX_TAN_BIN ){ve.New().set(GL_VTX_TAN     , 4, GL_BYTE         , true , ofs); ofs+=SIZE(VecB4);}}
   else                             {if(flag&VTX_TAN     ){ve.New().set(GL_VTX_TAN     , 3, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec  );}
                                     if(flag&VTX_BIN     ){                                                                ofs+=SIZE(Vec  );}}

                                     if(flag&VTX_HLP     ){ve.New().set(GL_VTX_HLP     , 3, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_TEX_8  ){if(flag&VTX_TEX0    ){ve.New().set(GL_VTX_TEX0    , 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(VecB4);}
                                     if(flag&VTX_TEX1    ){ve.New().set(GL_VTX_TEX1    , 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(VecB4);}
                                     if(flag&VTX_TEX2    ){ve.New().set(GL_VTX_TEX2    , 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(VecB4);}}else
   if(compress&VTX_COMPRESS_TEX    ){if(flag&VTX_TEX0    ){ve.New().set(GL_VTX_TEX0    , 2, GL_HALF_FLOAT   , false, ofs); ofs+=SIZE(VecH2);}
                                     if(flag&VTX_TEX1    ){ve.New().set(GL_VTX_TEX1    , 2, GL_HALF_FLOAT   , false, ofs); ofs+=SIZE(VecH2);}
                                     if(flag&VTX_TEX2    ){ve.New().set(GL_VTX_TEX2    , 2, GL_HALF_FLOAT   , false, ofs); ofs+=SIZE(VecH2);}}else
                                    {if(flag&VTX_TEX0    ){ve.New().set(GL_VTX_TEX0    , 2, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec2 );}
                                     if(flag&VTX_TEX1    ){ve.New().set(GL_VTX_TEX1    , 2, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec2 );}
                                     if(flag&VTX_TEX2    ){ve.New().set(GL_VTX_TEX2    , 2, GL_FLOAT        , false, ofs); ofs+=SIZE(Vec2 );}}

                                     if(flag&VTX_MATRIX  ){ve.New().set(GL_VTX_BONE    , 4, GL_UNSIGNED_BYTE, false, ofs); ofs+=SIZE(VecB4);}
                                     if(flag&VTX_BLEND   ){ve.New().set(GL_VTX_WEIGHT  , 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(VecB4);}
                                     if(flag&VTX_SIZE    ){ve.New().set(GL_VTX_SIZE    , 1, GL_FLOAT        , false, ofs); ofs+=SIZE(Flt  );}
                                     if(flag&VTX_MATERIAL){ve.New().set(GL_VTX_MATERIAL, 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(VecB4);}
                                     if(flag&VTX_COLOR   ){ve.New().set(GL_VTX_COLOR   , 4, GL_UNSIGNED_BYTE, true , ofs); ofs+=SIZE(Color);}
   return create(ve);
#endif
   return false;
}
/******************************************************************************/
// VERTEX BUFFER
/******************************************************************************/
VtxBuf& VtxBuf::del()
{
   unlock();
   if(_buf)
   {
   #if DX9 || GL_LOCK // lock not needed for DX11 'Release'
      SafeSyncLocker lock(D._lock);
   #endif

	#if DX9 || DX11
      if(_buf){if(D.created())_buf->Release(); _buf=null;} // clear while in lock
   #elif GL
      if(D.created())glDeleteBuffers(1, &_buf); _buf=0; // clear while in lock
      if(!IsMap(_dynamic))Free(_data);
   #endif
   }
   Zero(T); return T;
}
IndBuf& IndBuf::del()
{
   unlock();
   if(_buf)
   {
   #if DX9 || GL_LOCK // lock not needed for DX11 'Release'
      SafeSyncLocker lock(D._lock);
   #endif

   #if DX9 || DX11
      if(_buf){if(D.created())_buf->Release(); _buf=null;} // clear while in lock
   #elif GL
      if(D.created())glDeleteBuffers(1, &_buf); _buf=0; // clear while in lock
      if(!IsMap(_dynamic))Free(_data);
   #endif
   }
   Zero(T); return T;
}
/******************************************************************************/
Bool VtxBuf::createRaw(Int memory_size, Bool dynamic, CPtr data)
{
#if 0 // we can't do this here, because 'createRaw' does not set any info about 'vtxs', 'vtxSize' or 'memUsage', and in 'VI._vb' those params always change, instead these checks are in 'createNum'
   if(memUsage()==memory_size && T._dynamic==dynamic && !_lock_count) // if the buffer is already created and matches what we want
      if(!data || setFrom(data, memory_size))return true; // if 'setFrom' failed then try creating new below
#endif

   if(memory_size<=0){del(); return !memory_size;}

#if DX9 || GL_LOCK // lock not needed for DX11 'D3D'
   SyncLocker locker(D._lock);
#endif

   del();

   if(D.created())
   {
      T._dynamic=dynamic;
   #if DX9
      if(OK(D3D->CreateVertexBuffer(memory_size, dynamic ? D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY : 0, 0, dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &_buf, null)))
         if(!data || setFrom(data, memory_size))return true;
   #elif DX11
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth          =memory_size;
      desc.Usage              =(dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);
      desc.CPUAccessFlags     =(dynamic ? D3D11_CPU_ACCESS_WRITE : 0);
      desc.BindFlags          =D3D11_BIND_VERTEX_BUFFER;
      desc.MiscFlags          =0;
      desc.StructureByteStride=0;
      D3D11_SUBRESOURCE_DATA res_data, *initial_data=null;
      if(data)
      {
         res_data.pSysMem=data;
         res_data.SysMemPitch=res_data.SysMemSlicePitch=0;
         initial_data=&res_data;
      }
      if(OK(D3D->CreateBuffer(&desc, initial_data, &_buf)))return true;
   #elif GL
      glGenBuffers(1, &_buf);
      if(_buf)
      {
         glBindBuffer(GL_ARRAY_BUFFER, _buf);
         glBufferData(GL_ARRAY_BUFFER, memory_size, data, dynamic ? GL_DYNAMIC : GL_STATIC_DRAW);
         glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
         if(!IsMap(dynamic) && (dynamic || GL_ES)) // never allocate for Map, allocate for dynamic or for GL_ES for which we can't read memory from GL
         {
            Alloc(_data, CeilGL(memory_size)); if(data)CopyFast(_data, data, memory_size);
         }
         return true;
      }
   #endif
   }
   del(); return false;
}
Bool IndBuf::create(Int indexes, Bool bit16, Bool dynamic, CPtr data)
{
   if(T._ind_num==indexes && T.bit16()==bit16 && T._dynamic==dynamic && !_lock_count) // if the buffer is already created and matches what we want
      if(!data || setFrom(data))return true; // if 'setFrom' failed then try creating new below

   if(indexes<=0){del(); return !indexes;}

#if DX9 || GL_LOCK // lock not needed for DX11 'D3D'
   SyncLocker locker(D._lock);
#endif

   del();
   
   if(D.created())
   {
      T._dynamic=dynamic;
      Int memory_size=indexes*(bit16 ? 2 : 4);
   #if DX9
      if(OK(D3D->CreateIndexBuffer(memory_size, dynamic ? D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY : 0, bit16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &_buf, null)))
      {
         T._bit16  =bit16  ;
         T._ind_num=indexes;
         if(!data || setFrom(data))return true;
      }
   #elif DX11
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth     =memory_size;
      desc.Usage         =(dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);
      desc.CPUAccessFlags=(dynamic ? D3D11_CPU_ACCESS_WRITE : 0);
      desc.BindFlags     =D3D11_BIND_INDEX_BUFFER;
      desc.MiscFlags     =0;
      desc.StructureByteStride=0;
      D3D11_SUBRESOURCE_DATA res_data, *initial_data=null;
      if(data)
      {
         res_data.pSysMem=data;
         res_data.SysMemPitch=res_data.SysMemSlicePitch=0;
         initial_data=&res_data;
      }
      if(OK(D3D->CreateBuffer(&desc, initial_data, &_buf)))
      {
         T._bit16  =bit16  ;
         T._ind_num=indexes;
         return true;
      }
   #elif GL
      glGenBuffers(1, &_buf);
      if(_buf)
      {
         BindIndexBuffer(_buf);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, memory_size, data, dynamic ? GL_DYNAMIC : GL_STATIC_DRAW);
         glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
         T._bit16  =bit16  ;
         T._ind_num=indexes;
         if(!IsMap(dynamic) && (dynamic || GL_ES)) // never allocate for Map, allocate for dynamic or for GL_ES for which we can't read memory from GL
         {
            Alloc(_data, CeilGL(memory_size)); if(data)CopyFast(_data, data, memory_size);
         }
         return true;
      }
   #endif
   }
   del(); return false;
}
/******************************************************************************/
Bool VtxBuf::createNum(Int vtx_size, Int vtx_num, Bool dynamic, CPtr data)
{
   if(vtxSize()==vtx_size && vtxs()==vtx_num && T._dynamic==dynamic && !_lock_count) // if the buffer is already created and matches what we want
      if(!data || setFrom(data, memUsage()))return true; // if 'setFrom' failed then try creating new below

   if(createRaw(vtx_size*vtx_num, dynamic, data))
   {
      T._vtx_size=vtx_size;
      T._vtx_num =vtx_num ;
      return true;
   }
   return false;
}
Bool VtxBuf::create(Int vertexes, UInt flag, UInt compress, Bool dynamic)
{
   Int size=0;
                                     if(flag&VTX_POS     )size+=SIZE(Vec  );

   if(compress&VTX_COMPRESS_NRM    ){if(flag&VTX_NRM     )size+=SIZE(VecB4);}
   else                             {if(flag&VTX_NRM     )size+=SIZE(Vec  );}

   if(compress&VTX_COMPRESS_TAN_BIN){if(flag&VTX_TAN_BIN )size+=SIZE(VecB4);}
   else                             {if(flag&VTX_TAN     )size+=SIZE(Vec  );
                                     if(flag&VTX_BIN     )size+=SIZE(Vec  );}

                                     if(flag&VTX_HLP     )size+=SIZE(Vec  );

   if(compress&VTX_COMPRESS_TEX_8  ){if(flag&VTX_TEX0    )size+=SIZE(VecB4);
                                     if(flag&VTX_TEX1    )size+=SIZE(VecB4);
                                     if(flag&VTX_TEX2    )size+=SIZE(VecB4);}else
   if(compress&VTX_COMPRESS_TEX    ){if(flag&VTX_TEX0    )size+=SIZE(VecH2);
                                     if(flag&VTX_TEX1    )size+=SIZE(VecH2);
                                     if(flag&VTX_TEX2    )size+=SIZE(VecH2);}else
                                    {if(flag&VTX_TEX0    )size+=SIZE(Vec2 );
                                     if(flag&VTX_TEX1    )size+=SIZE(Vec2 );
                                     if(flag&VTX_TEX2    )size+=SIZE(Vec2 );}

                                     if(flag&VTX_MATRIX  )size+=SIZE(VecB4);
                                     if(flag&VTX_BLEND   )size+=SIZE(VecB4);
                                     if(flag&VTX_SIZE    )size+=SIZE(Flt  );
                                     if(flag&VTX_MATERIAL)size+=SIZE(VecB4);
                                     if(flag&VTX_COLOR   )size+=SIZE(Color);

   return createNum(size, vertexes, dynamic);
}
/******************************************************************************/
Bool VtxBuf::create(C VtxBuf &src, Int dynamic)
{
   if(this==&src)return true;

   {
      CPtr src_data=null; if(src.is() && !(src_data=src.lockRead()))goto error;
      Bool ok=createNum(src.vtxSize(), src.vtxs(), (dynamic>=0) ? dynamic!=0 : src._dynamic, src_data);
      if(src_data)src.unlock();
      return ok;
   }

error:
   del(); return false;
}
Bool IndBuf::create(C IndBuf &src, Int dynamic)
{
   if(this==&src)return true;

   {
      CPtr src_data=null; if(src.is() && !(src_data=src.lockRead()))goto error;
      Bool ok=create(src._ind_num, src.bit16(), (dynamic>=0) ? dynamic!=0 : src._dynamic, src_data);
      if(src_data)src.unlock();
      return ok;
   }

error:
   del(); return false;
}
/******************************************************************************/
Byte* VtxBuf::lockDynamic()
{
   DEBUG_ASSERT(_buf && !_lock_mode && _dynamic, "VtxBuf.lockDynamic");
   LOCK_MODE lock=(VI._vtx_drawing ? LOCK_APPEND : LOCK_WRITE);
#if DX9
   Ptr data=null; _buf->Lock(0, 0, &data, (lock==LOCK_WRITE) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE); T._data=(Byte*)data;
#elif DX11
   D3D11_MAPPED_SUBRESOURCE map;
   if(OK(D3DC->Map(_buf, 0, (lock==LOCK_WRITE) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &map)))_data=(Byte*)map.pData;
#elif GL
   if(IsMap(true))
   {
                                                                          glBindBuffer    (GL_ARRAY_BUFFER, _buf);
      if(!GL_RING)                                           _data=(Byte*)glMapBufferRange(GL_ARRAY_BUFFER,       0, MEM       , GL_MAP_WRITE_BIT|GL_MAP_FLUSH_EXPLICIT_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
      if( GL_RING){UInt offset=VI._vtx_drawing*_vtx_size; if(_data=(Byte*)glMapBufferRange(GL_ARRAY_BUFFER,  offset, MEM-offset, GL_MAP_WRITE_BIT|GL_MAP_FLUSH_EXPLICIT_BIT|(lock==LOCK_WRITE ? GL_MAP_INVALIDATE_BUFFER_BIT : /*made things slower GL_MAP_INVALIDATE_RANGE_BIT|*/GL_MAP_UNSYNCHRONIZED_BIT)))_data-=offset;}
   }else
   {
      // data for dynamic is already allocated and ready to use
   }
#endif
   if(_data)
   {
      T._lock_mode =lock;
      T._lock_count=1;
   }
   return _data;
}
void VtxBuf::unlockDynamic()
{
   DEBUG_ASSERT((_lock_mode==LOCK_WRITE || _lock_mode==LOCK_APPEND) && _lock_count==1 && _dynamic, "VtxBuf.unlockDynamic");
#if DX9
  _buf->Unlock(); _data=null;
#elif DX11
   D3DC->Unmap(_buf, 0); _data=null;
#elif GL
   glBindBuffer(GL_ARRAY_BUFFER, _buf);
   if(IsMap(true))
   {
      glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, VI._vtx_queued*_vtx_size); // we always start from zero, because values are relative to locked range
      glUnmapBuffer           (GL_ARRAY_BUFFER);
     _data=null;
   }else
   switch(GL_VI_LOCK)
   {
      default                         :                                                                                                                                                   glBufferSubData(GL_ARRAY_BUFFER,      0, VI._vtx_queued*_vtx_size, _data       );  break;
      case GL_LOCK_SUB_RESET_PART     :                           glBufferData(GL_ARRAY_BUFFER, VI._vtx_queued*_vtx_size,  null, GL_DYNAMIC);                                             glBufferSubData(GL_ARRAY_BUFFER,      0, VI._vtx_queued*_vtx_size, _data       );  break;
      case GL_LOCK_SUB_RESET_FULL     :                           glBufferData(GL_ARRAY_BUFFER,                      MEM,  null, GL_DYNAMIC);                                             glBufferSubData(GL_ARRAY_BUFFER,      0, VI._vtx_queued*_vtx_size, _data       );  break;
      case GL_LOCK_SUB_RESET_PART_FROM:                           glBufferData(GL_ARRAY_BUFFER, VI._vtx_queued*_vtx_size, _data, GL_DYNAMIC);                                                                                                                                break;
      case GL_LOCK_SUB_RESET_FULL_FROM:                           glBufferData(GL_ARRAY_BUFFER,                      MEM, _data, GL_DYNAMIC);                                                                                                                                break;
      case GL_LOCK_SUB_RING           :                                                                                                           {UInt offset=VI._vtx_drawing*_vtx_size; glBufferSubData(GL_ARRAY_BUFFER, offset, VI._vtx_queued*_vtx_size, _data+offset);} break;
      case GL_LOCK_SUB_RING_RESET     : if(_lock_mode==LOCK_WRITE)glBufferData(GL_ARRAY_BUFFER,                      MEM,  null, GL_DYNAMIC);     {UInt offset=VI._vtx_drawing*_vtx_size; glBufferSubData(GL_ARRAY_BUFFER, offset, VI._vtx_queued*_vtx_size, _data+offset);} break;
      case GL_LOCK_SUB_RING_RESET_FROM: if(_lock_mode==LOCK_WRITE)glBufferData(GL_ARRAY_BUFFER,                      MEM, _data, GL_DYNAMIC);else {UInt offset=VI._vtx_drawing*_vtx_size; glBufferSubData(GL_ARRAY_BUFFER, offset, VI._vtx_queued*_vtx_size, _data+offset);} break;
   }
 //glFlush(); no need to flush because it meant to be called only on the main thread for VI
#endif
  _lock_mode =LOCK_NONE;
  _lock_count=0;
}
Byte* VtxBuf::lock(LOCK_MODE lock)
{
   if(lock && _buf)
   {
      SyncLocker locker(D._lock);
      if(!_lock_mode) // first lock
      {
      #if DX9
         Ptr data=null; _buf->Lock(0, 0, &data, (lock==LOCK_WRITE) ? D3DLOCK_DISCARD : (lock==LOCK_APPEND) ? D3DLOCK_NOOVERWRITE : (lock==LOCK_READ) ? D3DLOCK_READONLY : 0);
         T._data=(Byte*)data;
      #elif DX11
         if(_dynamic)
         {
            if(lock==LOCK_WRITE || lock==LOCK_APPEND)
            {
               D3D11_MAPPED_SUBRESOURCE map;
               if(OK(D3DC->Map(_buf, 0, (lock==LOCK_WRITE) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &map)))_data=(Byte*)map.pData;
            }else
            {
               // get from GPU
               D3D11_BUFFER_DESC desc;
               desc.ByteWidth          =_vtx_num*_vtx_size;
               desc.Usage              =D3D11_USAGE_STAGING;
               desc.CPUAccessFlags     =D3D11_CPU_ACCESS_READ;
               desc.BindFlags          =0;
               desc.MiscFlags          =0;
               desc.StructureByteStride=0;

               ID3D11Buffer *temp=null; if(OK(D3D->CreateBuffer(&desc, null, &temp)))
               {
                  D3DC->CopyResource(temp, _buf);
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, 0, D3D11_MAP_READ, 0, &map)))
                  {
                     CPtr src=map.pData; if(OK(D3DC->Map(_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
                     {
                       _data=(Byte*)map.pData;
                        CopyFast(_data, src, desc.ByteWidth);
                     }
                     D3DC->Unmap(temp, 0);
                  }
                  RELEASE(temp);
               }
            }
         }else
         {
            if(lock==LOCK_WRITE || lock==LOCK_APPEND)Alloc(_data, _vtx_size*_vtx_num);else
            {
               // get from GPU
               D3D11_BUFFER_DESC desc;
               desc.ByteWidth     =_vtx_size*_vtx_num;
               desc.Usage         =D3D11_USAGE_STAGING;
               desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
               desc.BindFlags     =0;
               desc.MiscFlags     =0;
               desc.StructureByteStride=0;

               ID3D11Buffer *temp=null; if(OK(D3D->CreateBuffer(&desc, null, &temp)))
               {
                  D3DC->CopyResource(temp, _buf);
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, 0, D3D11_MAP_READ, 0, &map)))
                  {
                     CopyFast(Alloc(_data, desc.ByteWidth), map.pData, desc.ByteWidth);
                     D3DC->Unmap(temp, 0);
                  }
                  RELEASE(temp);
               }
            }
         }
      #elif GL
         if(IsMap(_dynamic))
         {
                        glBindBuffer    (GL_ARRAY_BUFFER, _buf);
           _data=(Byte*)glMapBufferRange(GL_ARRAY_BUFFER, 0, _vtx_size*_vtx_num, (lock==LOCK_WRITE) ? GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT : (lock==LOCK_APPEND) ? GL_MAP_WRITE_BIT : (lock==LOCK_READ) ? GL_MAP_READ_BIT : GL_MAP_READ_BIT|GL_MAP_WRITE_BIT);
         }else
         {
   	   #if GL_ES
      	   // data is already allocated and ready to use
         #else
            if(!_data)Alloc(_data, CeilGL(_vtx_size*_vtx_num));
            if(lock!=LOCK_WRITE && lock!=LOCK_APPEND) // read
            {
               glBindBuffer      (GL_ARRAY_BUFFER, _buf);
               glGetBufferSubData(GL_ARRAY_BUFFER, 0, _vtx_size*_vtx_num, _data);
            }
         #endif
         }
      #endif
         if(_data)
         {
            T._lock_mode =lock;
            T._lock_count=1;
            return _data;
         }
      }else
      if(CompatibleLock(_lock_mode, lock)){_lock_count++; return _data;} // there was already a lock, just increase the counter and return success
   }
   return null;
}
Ptr IndBuf::lock(LOCK_MODE lock)
{
   if(lock && _buf)
   {
      SyncLocker locker(D._lock);
      if(!_lock_mode) // first lock
      {
      #if DX9
         Ptr data=null; _buf->Lock(0, 0, &data, (lock==LOCK_WRITE) ? D3DLOCK_DISCARD : (lock==LOCK_APPEND) ? D3DLOCK_NOOVERWRITE : (lock==LOCK_READ) ? D3DLOCK_READONLY : 0);
         T._data=(Byte*)data;
      #elif DX11
         if(_dynamic)
         {
            if(lock==LOCK_WRITE || lock==LOCK_APPEND)
            {
               D3D11_MAPPED_SUBRESOURCE map;
               if(OK(D3DC->Map(_buf, 0, (lock==LOCK_WRITE) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &map)))_data=(Byte*)map.pData;
            }else
            {
               // get from GPU
               D3D11_BUFFER_DESC desc;
               desc.ByteWidth          =_ind_num*(bit16() ? 2 : 4);
               desc.Usage              =D3D11_USAGE_STAGING;
               desc.CPUAccessFlags     =D3D11_CPU_ACCESS_READ;
               desc.BindFlags          =0;
               desc.MiscFlags          =0;
               desc.StructureByteStride=0;

               ID3D11Buffer *temp=null; if(OK(D3D->CreateBuffer(&desc, null, &temp)))
               {
                  D3DC->CopyResource(temp, _buf);
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, 0, D3D11_MAP_READ, 0, &map)))
                  {
                     CPtr src=map.pData; if(OK(D3DC->Map(_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
                     {
                       _data=(Byte*)map.pData;
                        CopyFast(_data, src, desc.ByteWidth);
                     }
                     D3DC->Unmap(temp, 0);
                  }
                  RELEASE(temp);
               }
            }
         }else
         {
            if(lock==LOCK_WRITE || lock==LOCK_APPEND)Alloc(_data, _ind_num*(bit16() ? 2 : 4));else
            {
               // get from GPU
               D3D11_BUFFER_DESC desc;
               desc.ByteWidth          =_ind_num*(bit16() ? 2 : 4);
               desc.Usage              =D3D11_USAGE_STAGING;
               desc.CPUAccessFlags     =D3D11_CPU_ACCESS_READ;
               desc.BindFlags          =0;
               desc.MiscFlags          =0;
               desc.StructureByteStride=0;

               ID3D11Buffer *temp=null; if(OK(D3D->CreateBuffer(&desc, null, &temp)))
               {
                  D3DC->CopyResource(temp, _buf);
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, 0, D3D11_MAP_READ, 0, &map)))
                  {
                     CopyFast(Alloc(_data, desc.ByteWidth), map.pData, desc.ByteWidth);
                     D3DC->Unmap(temp, 0);
                  }
                  RELEASE(temp);
               }
            }
         }
      #elif GL
         if(IsMap(_dynamic))
         {
                         BindIndexBuffer(_buf);
           _data=(Byte*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, _ind_num*(bit16() ? 2 : 4), (lock==LOCK_WRITE) ? GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT : (lock==LOCK_APPEND) ? GL_MAP_WRITE_BIT : (lock==LOCK_READ) ? GL_MAP_READ_BIT : GL_MAP_READ_BIT|GL_MAP_WRITE_BIT);
         }else
         {
   	   #if GL_ES
      	   // data is already allocated and ready to use
         #else
            if(!_data)Alloc(_data, CeilGL(_ind_num*(bit16() ? 2 : 4)));
            if(lock!=LOCK_WRITE && lock!=LOCK_APPEND) // read
            {
                  BindIndexBuffer(_buf);
               glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _ind_num*(bit16() ? 2 : 4), _data);
            }
         #endif
         }
      #endif
         if(_data)
         {
            T._lock_mode =lock;
            T._lock_count=1;
            return _data;
         }
      }else
      if(CompatibleLock(_lock_mode, lock)){_lock_count++; return _data;} // there was already a lock, just increase the counter and return success
   }
   return null;
}

void VtxBuf::unlock()
{
   if(_lock_count>0)
   {
      SafeSyncLocker lock(D._lock);
      if(_lock_count>0)if(!--_lock_count)
      {
      #if DX9
         if(D.created())_buf->Unlock(); _data=null;
      #elif DX11
         if(_dynamic)
         {
            if(D3DC)D3DC->Unmap(_buf, 0); _data=null;
         }else
         {
            if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_buf, 0, null, _data, 0, 0); Free(_data);
         }
      #elif GL
         if(IsMap(_dynamic))
         {
            if(D.created())
            {
               glBindBuffer (GL_ARRAY_BUFFER, _buf);
               glUnmapBuffer(GL_ARRAY_BUFFER);
            }
           _data=null;
         }else
         {
            if(_lock_mode!=LOCK_READ && D.created())
            {
               glBindBuffer   (GL_ARRAY_BUFFER, _buf);
               glBufferSubData(GL_ARRAY_BUFFER, 0, _vtx_size*_vtx_num, _data);
               glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
            }
            if(!GL_ES && !_dynamic)Free(_data); // keep data for GLES and dynamic
         }
      #endif
        _lock_mode=LOCK_NONE;
      }
   }
}
void IndBuf::unlock()
{
   if(_lock_count>0)
   {
      SafeSyncLocker lock(D._lock);
      if(_lock_count>0)if(!--_lock_count)
      {
      #if DX9
         if(D.created())_buf->Unlock(); _data=null;
      #elif DX11
         if(_dynamic)
         {
            if(D3DC)D3DC->Unmap(_buf, 0); _data=null;
         }else
         {
            if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_buf, 0, null, _data, 0, 0); Free(_data);
         }
      #elif GL
         if(IsMap(_dynamic))
         {
            if(D.created())
            {
               BindIndexBuffer(_buf);
                 glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            }
           _data=null;
         }else
         {
            if(_lock_mode!=LOCK_READ && D.created())
            {
               BindIndexBuffer(_buf);
               glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _ind_num*(bit16() ? 2 : 4), _data);
               glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
            }
            if(!GL_ES && !_dynamic)Free(_data); // keep data for GLES and dynamic
         }
      #endif
        _lock_mode=LOCK_NONE;
      }
   }
}
C Byte* VtxBuf::lockRead()C {return ConstCast(T).  lock(LOCK_READ);}
  CPtr  IndBuf::lockRead()C {return ConstCast(T).  lock(LOCK_READ);}
  void  VtxBuf::unlock  ()C {       ConstCast(T).unlock();}
  void  IndBuf::unlock  ()C {       ConstCast(T).unlock();}
/******************************************************************************/
Bool VtxBuf::setFrom(CPtr data, Int size)
{
   if(data)if(Ptr dest=lock(LOCK_WRITE))
   {
      CopyFast(dest, data, size);
      unlock();
      return true;
   }
   return false;
}
Bool IndBuf::setFrom(CPtr data)
{
   if(data)if(Ptr dest=lock(LOCK_WRITE))
   {
      CopyFast(dest, data, memUsage());
      unlock();
      return true;
   }
   return false;
}
/******************************************************************************/
#if GL_ES
void VtxBuf::freeOpenGLESData() {if(!_lock_mode && !IsMap(_dynamic))Free(_data);}
void IndBuf::freeOpenGLESData() {if(!_lock_mode && !IsMap(_dynamic))Free(_data);}
#else
void VtxBuf::freeOpenGLESData() {}
void IndBuf::freeOpenGLESData() {}
#endif
/******************************************************************************/
Bool VtxBuf::save(File &f)C
{
   f.putMulti(Byte(0), _vtx_size, _vtx_num); // version
   if(_vtx_size && _vtx_num)
   {
      if(CPtr data=lockRead()){f.put(data, memUsage()); unlock();}else return false;
   }
   return f.ok();
}
Bool VtxBuf::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         Int vtx_size, vtx_num; f.getMulti(vtx_size, vtx_num); if(f.ok())
         {
            Memt<Byte> temp; if(vtx_size>0 && vtx_num>0 && !temp.setNum(vtx_size*vtx_num).loadRawDataFast(f))goto error;
            return createNum(vtx_size, vtx_num, false, temp.dataNull());
         }
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
Bool IndBuf::save(File &f)C
{
   f.putMulti(Byte(0), _bit16, _ind_num); // version
   if(_ind_num)
   {
      if(CPtr data=lockRead()){f.put(data, memUsage()); unlock();}else return false;
   }
   return f.ok();
}
Bool IndBuf::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         Bool bit16; Int ind_num; f.getMulti(bit16, ind_num); if(f.ok())
         {
            Memt<Byte> temp; if(ind_num>0 && !temp.setNum(ind_num*(bit16 ? 2 : 4)).loadRawDataFast(f))goto error;
            return create(ind_num, bit16, false, temp.dataNull());
         }
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
// INDEX BUFFER
/******************************************************************************/
IndBuf& IndBuf::setTri(Int i, Int v0, Int v1, Int v2)
{
   if(_data)
   {
      i*=3; // index number
      if(InRange(i+2, _ind_num))
      {
         if(bit16()){U16 *t=((U16*)_data)+i; t[0]=v0; t[1]=v1; t[2]=v2;}
         else       {U32 *t=((U32*)_data)+i; t[0]=v0; t[1]=v1; t[2]=v2;}
      }
   }
   return T;
}
IndBuf& IndBuf::setQuad(Int i, Int v0, Int v1, Int v2, Int v3)
{
   if(_data)
   {
      i*=6; // index number
      if(InRange(i+5, _ind_num))
      {
         if(bit16()){U16 *t=((U16*)_data)+i; t[0]=v0; t[1]=v1; t[2]=v3; t[3]=v3; t[4]=v1; t[5]=v2;}
         else       {U32 *t=((U32*)_data)+i; t[0]=v0; t[1]=v1; t[2]=v3; t[3]=v3; t[4]=v1; t[5]=v2;}
      }
   }
   return T;
}
/******************************************************************************/
// VERTEX INDEX BUFFER
/******************************************************************************/
VtxIndBuf::VtxIndBuf() {}
void VtxIndBuf::lost()
{
   unlockVtx(); _vb.del();
 //unlockInd(); _ib.del();
#if BUFFERS>1
   REPAO(VB).del();
 //REPAO(IB).del();
#endif
}
void VtxIndBuf::reset()
{
  _mem_max=MEM;
  _vtx_queued=_vtx_drawing=_vtx_drawing_raw=0;
//_ind_queued=0;
   if(_vb.createRaw(MEM          ,       true))
 //if(_ib.create   (MEM/SIZE(U16), true, true)) // divide by size of 1 index
   {
   #if BUFFERS>1
      REPAO(VB).createRaw(MEM          ,       true);
    //REPAO(IB).create   (MEM/SIZE(U16), true, true);
   #endif
      clear();

     _vf2D_flat     .bind(_vb);
     _vf2D_col      .bind(_vb);
     _vf2D_tex      .bind(_vb);
     _vf2D_tex_col  .bind(_vb);
     _vf2D_tex2     .bind(_vb);
     _vf2D_font     .bind(_vb);
     _vf3D_flat     .bind(_vb);
     _vf3D_col      .bind(_vb);
     _vf3D_tex      .bind(_vb);
     _vf3D_tex_col  .bind(_vb);
     _vf3D_bilb     .bind(_vb);
     _vf3D_bilb_anim.bind(_vb);
     _vf3D_laser    .bind(_vb);
     _vf3D_cloth    .bind(_vb);
     _vf3D_simple   .bind(_vb);
     _vf3D_standard .bind(_vb);
     _vf3D_full     .bind(_vb);
      return;
   }
   Exit("Can't create Vertex/Index Buffer");
}
void VtxIndBuf::del()
{
   lost();

  _vf2D_flat     .del();
  _vf2D_col      .del();
  _vf2D_tex      .del();
  _vf2D_tex_col  .del();
  _vf2D_tex2     .del();
  _vf2D_font     .del();
  _vf3D_flat     .del();
  _vf3D_col      .del();
  _vf3D_tex      .del();
  _vf3D_tex_col  .del();
  _vf3D_bilb     .del();
  _vf3D_bilb_anim.del();
  _vf3D_laser    .del();
  _vf3D_cloth    .del();
  _vf3D_simple   .del();
  _vf3D_standard .del();
  _vf3D_full     .del();
}
void VtxIndBuf::create()
{
#pragma warning(push)
#pragma warning(disable:4838)
#if DX9
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DFlat, pos), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
      D3DDECL_END()}; _vf2D_flat.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DCol, pos  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx2DCol, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf2D_col.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DTex, pos), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx2DTex, tex), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
      D3DDECL_END()}; _vf2D_tex.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DTexCol, pos  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx2DTexCol, tex  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx2DTexCol, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf2D_tex_col.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DTex2, pos   ), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx2DTex2, tex[0]), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx2DTex2, tex[1]), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
      D3DDECL_END()}; _vf2D_tex2.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx2DFont, pos  ), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx2DFont, tex  ), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx2DFont, shade), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE   , 0},
      D3DDECL_END()}; _vf2D_font.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DFlat, pos), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
      D3DDECL_END()}; _vf3D_flat.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DCol, pos  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DCol, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf3D_col.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DTex, pos), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DTex, tex), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
      D3DDECL_END()}; _vf3D_tex.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DTexCol, pos  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DTexCol, tex  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx3DTexCol, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf3D_tex_col.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DBilb, pos      ), D3DDECLTYPE_FLOAT3   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
      #if GPU_HALF_SUPPORTED
         {0, OFFSET(Vtx3DBilb, vel_angle), D3DDECLTYPE_FLOAT16_4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT , 0},
      #else
         {0, OFFSET(Vtx3DBilb, vel_angle), D3DDECLTYPE_FLOAT4   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT , 0},
      #endif
         {0, OFFSET(Vtx3DBilb, tex      ), D3DDECLTYPE_UBYTE4N  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx3DBilb, size     ), D3DDECLTYPE_FLOAT1   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE   , 0},
         {0, OFFSET(Vtx3DBilb, color    ), D3DDECLTYPE_UBYTE4N  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf3D_bilb.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DBilbAnim, pos      ), D3DDECLTYPE_FLOAT3   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
      #if GPU_HALF_SUPPORTED
         {0, OFFSET(Vtx3DBilbAnim, vel_angle), D3DDECLTYPE_FLOAT16_4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT , 0},
      #else
         {0, OFFSET(Vtx3DBilbAnim, vel_angle), D3DDECLTYPE_FLOAT4   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT , 0},
      #endif
         {0, OFFSET(Vtx3DBilbAnim, tex      ), D3DDECLTYPE_UBYTE4N  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx3DBilbAnim, size     ), D3DDECLTYPE_FLOAT1   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE   , 0},
         {0, OFFSET(Vtx3DBilbAnim, color    ), D3DDECLTYPE_UBYTE4N  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
         {0, OFFSET(Vtx3DBilbAnim, frame    ), D3DDECLTYPE_FLOAT1   , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
      D3DDECL_END()}; _vf3D_bilb_anim.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DLaser, pos), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DLaser, nrm), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL  , 0},
      D3DDECL_END()}; _vf3D_laser.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Cloth::Vtx, pos), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Cloth::Vtx, nrm), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL  , 0},
         {0, OFFSET(Cloth::Vtx, tex), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
      D3DDECL_END()}; _vf3D_cloth.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DSimple, pos  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DSimple, nrm  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL  , 0},
         {0, OFFSET(Vtx3DSimple, tex  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx3DSimple, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf3D_simple.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DStandard, pos  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, OFFSET(Vtx3DStandard, nrm  ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL  , 0},
         {0, OFFSET(Vtx3DStandard, tan  ), D3DDECLTYPE_FLOAT4 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT , 0},
         {0, OFFSET(Vtx3DStandard, tex  ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, OFFSET(Vtx3DStandard, color), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 1},
      D3DDECL_END()}; _vf3D_standard.create(ve);
   }
   {
      D3DVERTEXELEMENT9 ve[]={
         {0, OFFSET(Vtx3DFull, pos     ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION    , 0},
         {0, OFFSET(Vtx3DFull, hlp     ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION    , 1},
         {0, OFFSET(Vtx3DFull, nrm     ), D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL      , 0},
         {0, OFFSET(Vtx3DFull, tan     ), D3DDECLTYPE_FLOAT4 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT     , 0},
         {0, OFFSET(Vtx3DFull, tex0    ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD    , 0},
         {0, OFFSET(Vtx3DFull, tex1    ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD    , 1},
         {0, OFFSET(Vtx3DFull, tex2    ), D3DDECLTYPE_FLOAT2 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD    , 2},
         {0, OFFSET(Vtx3DFull, color   ), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR       , 1},
         {0, OFFSET(Vtx3DFull, material), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR       , 0},
         {0, OFFSET(Vtx3DFull, matrix  ), D3DDECLTYPE_UBYTE4 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
         {0, OFFSET(Vtx3DFull, blend   ), D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT , 0},
         {0, OFFSET(Vtx3DFull, size    ), D3DDECLTYPE_FLOAT1 , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE       , 0},
      D3DDECL_END()}; _vf3D_full.create(ve);
   }
#elif DX11
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DFlat, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_flat.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DCol, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx2DCol, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_col.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTex, tex), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_tex.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTexCol, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTexCol, tex  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx2DTexCol, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_tex_col.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTex2, pos   ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTex2, tex[0]), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DTex2, tex[1]), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                         0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_tex2.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DFont, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx2DFont, tex  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0, OFFSET(Vtx2DFont, shade), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                        0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf2D_font.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DFlat, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                      0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_flat.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DCol, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DCol, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_col.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DTex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DTex, tex), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                     0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_tex.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DTexCol, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DTexCol, tex  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DTexCol, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_tex_col.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DBilb, pos      ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      #if GPU_HALF_SUPPORTED
         {"TANGENT"     , 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, OFFSET(Vtx3DBilb, vel_angle), D3D11_INPUT_PER_VERTEX_DATA, 0},
      #else
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, OFFSET(Vtx3DBilb, vel_angle), D3D11_INPUT_PER_VERTEX_DATA, 0},
      #endif
         {"TEXCOORD"    , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DBilb, tex      ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0, OFFSET(Vtx3DBilb, size     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DBilb, color    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_bilb.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DBilbAnim, pos      ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      #if GPU_HALF_SUPPORTED
         {"TANGENT"     , 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, OFFSET(Vtx3DBilbAnim, vel_angle), D3D11_INPUT_PER_VERTEX_DATA, 0},
      #else
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, OFFSET(Vtx3DBilbAnim, vel_angle), D3D11_INPUT_PER_VERTEX_DATA, 0},
      #endif
         {"TEXCOORD"    , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DBilbAnim, tex      ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32_FLOAT         , 0, OFFSET(Vtx3DBilbAnim, frame    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0, OFFSET(Vtx3DBilbAnim, size     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                                0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DBilbAnim, color    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_bilb_anim.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DLaser, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DLaser, nrm), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_laser.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Cloth::Vtx, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Cloth::Vtx, nrm), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Cloth::Vtx, tex), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                       0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_cloth.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DSimple, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DSimple, nrm  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DSimple, tex  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                          0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DSimple, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_simple.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DStandard, pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DStandard, nrm  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, OFFSET(Vtx3DStandard, tan  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DStandard, tex  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DStandard, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_standard.create(ve, Elms(ve));
   }
   {
      D3D11_INPUT_ELEMENT_DESC ve[]=
      {
         {"POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DFull, pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"POSITION"    , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DFull, hlp     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, OFFSET(Vtx3DFull, nrm     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TANGENT"     , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, OFFSET(Vtx3DFull, tan     ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DFull, tex0    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 1, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DFull, tex1    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD"    , 2, DXGI_FORMAT_R32G32_FLOAT      , 0, OFFSET(Vtx3DFull, tex2    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"PSIZE"       , 0, DXGI_FORMAT_R32_FLOAT         , 0, OFFSET(Vtx3DFull, size    ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT     , 0, OFFSET(Vtx3DFull, matrix  ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"BLENDWEIGHT" , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DFull, blend   ), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DFull, material), D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR"       , 1, DXGI_FORMAT_R8G8B8A8_UNORM    , 0, OFFSET(Vtx3DFull, color   ), D3D11_INPUT_PER_VERTEX_DATA, 0},
      };
     _vf3D_full.create(ve, Elms(ve));
   }
#elif GL
   MemtN<VtxFormatGL::Elm, 32> ve;
   {
      ve.New().set(GL_VTX_POS, 2, GL_FLOAT, false, OFFSET(Vtx2DFlat, pos));
     _vf2D_flat.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 2, GL_FLOAT        , false, OFFSET(Vtx2DCol, pos  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx2DCol, color));
     _vf2D_col.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS , 2, GL_FLOAT, false, OFFSET(Vtx2DTex, pos));
      ve.New().set(GL_VTX_TEX0, 2, GL_FLOAT, false, OFFSET(Vtx2DTex, tex));
     _vf2D_tex.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 2, GL_FLOAT        , false, OFFSET(Vtx2DTexCol, pos  ));
      ve.New().set(GL_VTX_TEX0 , 2, GL_FLOAT        , false, OFFSET(Vtx2DTexCol, tex  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx2DTexCol, color));
     _vf2D_tex_col.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS , 2, GL_FLOAT, false, OFFSET(Vtx2DTex2, pos   ));
      ve.New().set(GL_VTX_TEX0, 2, GL_FLOAT, false, OFFSET(Vtx2DTex2, tex[0]));
      ve.New().set(GL_VTX_TEX1, 2, GL_FLOAT, false, OFFSET(Vtx2DTex2, tex[1]));
     _vf2D_tex2.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS , 2, GL_FLOAT, false, OFFSET(Vtx2DFont, pos  ));
      ve.New().set(GL_VTX_TEX0, 2, GL_FLOAT, false, OFFSET(Vtx2DFont, tex  ));
      ve.New().set(GL_VTX_SIZE, 1, GL_FLOAT, false, OFFSET(Vtx2DFont, shade));
     _vf2D_font.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS, 3, GL_FLOAT, false, OFFSET(Vtx3DFlat, pos));
     _vf3D_flat.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DCol, pos  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DCol, color));
     _vf3D_col.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS , 3, GL_FLOAT, false, OFFSET(Vtx3DTex, pos));
      ve.New().set(GL_VTX_TEX0, 2, GL_FLOAT, false, OFFSET(Vtx3DTex, tex));
     _vf3D_tex.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DTexCol, pos  ));
      ve.New().set(GL_VTX_TEX0 , 2, GL_FLOAT        , false, OFFSET(Vtx3DTexCol, tex  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DTexCol, color));
     _vf3D_tex_col.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DBilb, pos      ));
   #if GPU_HALF_SUPPORTED
      ve.New().set(GL_VTX_TAN  , 4, GL_HALF_FLOAT   , false, OFFSET(Vtx3DBilb, vel_angle));
   #else
      ve.New().set(GL_VTX_TAN  , 4, GL_FLOAT        , false, OFFSET(Vtx3DBilb, vel_angle));
   #endif
      ve.New().set(GL_VTX_TEX0 , 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DBilb, tex      ));
      ve.New().set(GL_VTX_SIZE , 1, GL_FLOAT        , false, OFFSET(Vtx3DBilb, size     ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DBilb, color    ));
     _vf3D_bilb.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DBilbAnim, pos      ));
   #if GPU_HALF_SUPPORTED
      ve.New().set(GL_VTX_TAN  , 4, GL_HALF_FLOAT   , false, OFFSET(Vtx3DBilbAnim, vel_angle));
   #else
      ve.New().set(GL_VTX_TAN  , 4, GL_FLOAT        , false, OFFSET(Vtx3DBilbAnim, vel_angle));
   #endif
      ve.New().set(GL_VTX_TEX0 , 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DBilbAnim, tex      ));
      ve.New().set(GL_VTX_SIZE , 1, GL_FLOAT        , false, OFFSET(Vtx3DBilbAnim, size     ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DBilbAnim, color    ));
      ve.New().set(GL_VTX_TEX1 , 1, GL_FLOAT        , false, OFFSET(Vtx3DBilbAnim, frame    ));
     _vf3D_bilb_anim.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS, 3, GL_FLOAT, false, OFFSET(Vtx3DLaser, pos));
      ve.New().set(GL_VTX_NRM, 3, GL_FLOAT, false, OFFSET(Vtx3DLaser, nrm));
     _vf3D_laser.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS , 3, GL_FLOAT, false, OFFSET(Cloth::Vtx, pos));
      ve.New().set(GL_VTX_NRM , 3, GL_FLOAT, false, OFFSET(Cloth::Vtx, nrm));
      ve.New().set(GL_VTX_TEX0, 2, GL_FLOAT, false, OFFSET(Cloth::Vtx, tex));
     _vf3D_cloth.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DSimple, pos  ));
      ve.New().set(GL_VTX_NRM  , 3, GL_FLOAT        , false, OFFSET(Vtx3DSimple, nrm  ));
      ve.New().set(GL_VTX_TEX0 , 2, GL_FLOAT        , false, OFFSET(Vtx3DSimple, tex  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DSimple, color));
     _vf3D_simple.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS  , 3, GL_FLOAT        , false, OFFSET(Vtx3DStandard, pos  ));
      ve.New().set(GL_VTX_NRM  , 3, GL_FLOAT        , false, OFFSET(Vtx3DStandard, nrm  ));
      ve.New().set(GL_VTX_TAN  , 4, GL_FLOAT        , false, OFFSET(Vtx3DStandard, tan  ));
      ve.New().set(GL_VTX_TEX0 , 2, GL_FLOAT        , false, OFFSET(Vtx3DStandard, tex  ));
      ve.New().set(GL_VTX_COLOR, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DStandard, color));
     _vf3D_standard.create(ve); ve.clear();
   }
   {
      ve.New().set(GL_VTX_POS     , 3, GL_FLOAT        , false, OFFSET(Vtx3DFull, pos     ));
      ve.New().set(GL_VTX_HLP     , 3, GL_FLOAT        , false, OFFSET(Vtx3DFull, hlp     ));
      ve.New().set(GL_VTX_NRM     , 3, GL_FLOAT        , false, OFFSET(Vtx3DFull, nrm     ));
      ve.New().set(GL_VTX_TAN     , 4, GL_FLOAT        , false, OFFSET(Vtx3DFull, tan     ));
      ve.New().set(GL_VTX_TEX0    , 2, GL_FLOAT        , false, OFFSET(Vtx3DFull, tex0    ));
      ve.New().set(GL_VTX_TEX1    , 2, GL_FLOAT        , false, OFFSET(Vtx3DFull, tex1    ));
      ve.New().set(GL_VTX_TEX2    , 2, GL_FLOAT        , false, OFFSET(Vtx3DFull, tex2    ));
      ve.New().set(GL_VTX_COLOR   , 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DFull, color   ));
      ve.New().set(GL_VTX_MATERIAL, 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DFull, material));
      ve.New().set(GL_VTX_BONE    , 4, GL_UNSIGNED_BYTE, false, OFFSET(Vtx3DFull, matrix  ));
      ve.New().set(GL_VTX_WEIGHT  , 4, GL_UNSIGNED_BYTE, true , OFFSET(Vtx3DFull, blend   ));
      ve.New().set(GL_VTX_SIZE    , 1, GL_FLOAT        , false, OFFSET(Vtx3DFull, size    ));
     _vf3D_full.create(ve); ve.clear();
   }
#endif
#pragma warning(pop)

   reset();
}
/******************************************************************************/
#if BUFFERS<=1
   Bool VtxIndBuf::  lockVtx() {if(!VI._vtx_buf)VI._vtx_buf=      VI._vb.  lockDynamic(); return VI._vtx_buf!=null; }
 //Bool VtxIndBuf::  lockInd() {if(!VI._ind_buf)VI._ind_buf=(U16*)VI._ib.  lockDynamic(); return VI._ind_buf!=null; }

   void VtxIndBuf::unlockVtx() {if( VI._vtx_buf){                 VI._vb.unlockDynamic();        VI._vtx_buf =null;}}
 //void VtxIndBuf::unlockInd() {if( VI._ind_buf){                 VI._ib.unlockDynamic();        VI._ind_buf =null;}}
#else
   Bool VtxIndBuf::lockVtx()
   {
      if(!VI._vtx_buf)
      {
         VI_vb++; if(VI_vb>=VB+BUFFERS_USE)VI_vb=VB; // proceed to the next one, and go back to start if reached the end
         VI_vb->_vtx_size=VI._vb._vtx_size; // copy current settings
         VI_vb->_vtx_num =VI._vb._vtx_num ; // copy current settings
         VI    ._vtx_buf =VI_vb->lockDynamic();
         VI_vb->set(  );
      #if GL
         D     .vf (VF); // set VF after VBO on OpenGL
      #endif
      }
      return VI._vtx_buf!=null;
   }
   /*Bool VtxIndBuf::lockInd()
   {
      if(!VI._ind_buf)
      {
         VI_ib++; if(VI_ib>=IB+BUFFERS_USE)VI_ib=IB;
         VI._ind_buf=(U16*)VI_ib->lockDynamic();
         VI_ib->set();
      }
      return VI._ind_buf!=null;
   }*/

   void VtxIndBuf::unlockVtx() {if(VI._vtx_buf){VI_vb->unlockDynamic(); VI._vtx_buf=null;}}
 //void VtxIndBuf::unlockInd() {if(VI._ind_buf){VI_ib->unlockDynamic(); VI._ind_buf=null;}}
#endif
/******************************************************************************/
void VtxIndBuf::setType(VI_TYPE vtx_type, UInt flag)
{
   Shader    *shader;
   VtxFormat *vf;
   switch(VI._vtx_type=vtx_type)
   {
      case VI_2D_FLAT     : VI._vb._vtx_size=SIZE(Vtx2DFlat    ); shader=Sh.h_Draw2DFlat                                                                                       ; vf=&VI._vf2D_flat     ; D.depth(false); D.cull(false); break;
      case VI_2D_COL      : VI._vb._vtx_size=SIZE(Vtx2DCol     ); shader=Sh.h_Draw2DCol                                                                                        ; vf=&VI._vf2D_col      ; D.depth(false); D.cull(false); break;
      case VI_2D_TEX      : VI._vb._vtx_size=SIZE(Vtx2DTex     ); shader=((flag&VI_SP_COL) ? Sh.h_Draw2DTexC : Sh.h_Draw2DTex)                                                 ; vf=&VI._vf2D_tex      ; D.depth(false); D.cull(false); break;
      case VI_2D_TEX_COL  : VI._vb._vtx_size=SIZE(Vtx2DTexCol  ); shader=Sh.h_Draw2DTexCol                                                                                     ; vf=&VI._vf2D_tex_col  ; D.depth(false); D.cull(false); break;
      case VI_2D_TEX2     : VI._vb._vtx_size=SIZE(Vtx2DTex2    ); shader=Sh.h_DrawMask                                                                                         ; vf=&VI._vf2D_tex2     ; D.depth(false); D.cull(false); break;
      case VI_2D_FONT     : VI._vb._vtx_size=SIZE(Vtx2DFont    ); shader=Sh.h_Font                                                                                             ; vf=&VI._vf2D_font     ; D.depth(false); D.cull(false); break;
      case VI_3D_FLAT     : VI._vb._vtx_size=SIZE(Vtx3DFlat    ); shader=Sh.h_Draw3DFlat                                                                                       ; vf=&VI._vf3D_flat     ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_COL      : VI._vb._vtx_size=SIZE(Vtx3DCol     ); shader=Sh.h_Draw3DCol                                                                                        ; vf=&VI._vf3D_col      ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_TEX      : VI._vb._vtx_size=SIZE(Vtx3DTex     ); shader=Sh.h_Draw3DTex   [FlagTest(VI._user_flag, VI_ALPHA_TEST)][FlagTest(VI._user_flag, VI_FOG) && Fog.draw]; vf=&VI._vf3D_tex      ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_TEX_COL  : VI._vb._vtx_size=SIZE(Vtx3DTexCol  ); shader=Sh.h_Draw3DTexCol[FlagTest(VI._user_flag, VI_ALPHA_TEST)][FlagTest(VI._user_flag, VI_FOG) && Fog.draw]; vf=&VI._vf3D_tex_col  ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_BILB     : VI._vb._vtx_size=SIZE(Vtx3DBilb    ); shader=Sh.h_Bilb                                                                                             ; vf=&VI._vf3D_bilb     ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_BILB_ANIM: VI._vb._vtx_size=SIZE(Vtx3DBilbAnim); shader=null                                                                                                  ; vf=&VI._vf3D_bilb_anim; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_LASER    : VI._vb._vtx_size=SIZE(Vtx3DLaser   ); shader=null                                                                                                  ; vf=&VI._vf3D_laser    ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_SIMPLE   : VI._vb._vtx_size=SIZE(Vtx3DSimple  ); shader=Sh.h_Simple                                                                                           ; vf=&VI._vf3D_simple   ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_STANDARD : VI._vb._vtx_size=SIZE(Vtx3DStandard); shader=Sh.h_Simple                                                                                           ; vf=&VI._vf3D_standard ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      case VI_3D_FULL     : VI._vb._vtx_size=SIZE(Vtx3DFull    ); shader=null                                                                                                  ; vf=&VI._vf3D_full     ; D.depth(true ); D.cull(FlagTest(VI._user_flag, VI_CULL)); break;
      default             : return;
   }

#if GL
   #if BUFFERS<=1
      if(D.notShaderModelGLES2())glBindVertexArray(vf->vf->vao); // !! OpenGL requires setting VAO before VB and IB !! calling 'glBindVertexArray' on GLES2 would crash
   #else
      SetDefaultVAO(); // when using multiple buffers we need to set default 'VAO' because we will use 'D.vf' later, for which we need 'VAO' only
   #endif
#endif

   if(!VI._shader)VI._shader=shader;

      VI._vb._vtx_num=Min(0xFFFF, MEM                / VI._vb._vtx_size); // limit to 0xFFFF because of U16 indexing (drawIndexed)
      VI._vtx_drawing=    DivCeil(VI._vtx_drawing_raw, VI._vb._vtx_size);
      VI._flag       =flag;
   if(VI._quad_ind   =FlagTest(flag, VI_QUAD_IND))IndBuf16384Quads.set(); // !! set after 'glBindVertexArray' !!
   if(flag&VI_LINE)
   {
      if(flag&VI_STRIP){VI._prim_type=GPU_API(D3DPT_LINESTRIP, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, GL_LINE_STRIP); VI._ind_div=1; VI._ind_sub=1;}
      else             {VI._prim_type=GPU_API(D3DPT_LINELIST , D3D11_PRIMITIVE_TOPOLOGY_LINELIST , GL_LINES     ); VI._ind_div=2; VI._ind_sub=0;}
   }else
   {
      if(flag&VI_STRIP){VI._prim_type=GPU_API(D3DPT_TRIANGLESTRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, GL_TRIANGLE_STRIP); VI._ind_div=1; VI._ind_sub=2;}
      else             {VI._prim_type=GPU_API(D3DPT_TRIANGLELIST , D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST , GL_TRIANGLES     ); VI._ind_div=3; VI._ind_sub=0;}
   }

   SetVtxNrmMulAdd(false);
#if BUFFERS<=1
   if(!GL || D.shaderModelGLES2()) // for GL we juse use VAO which already sets VB and VF (because VB's are bound to VAO during 'VI.reset')
   {
      VI._vb.set(); // OpenGL requires setting VB after VAO but before VF, DX10+ requires calling this after setting '_vtx_size'
    //VI._ib.set(); we don't use index buffers here, all possible index buffers are set elsewhere ('IndBuf16384Quads' is above, and custom are in 'flushIndexed')
      D.vf(vf->vf);
   }
#else
   #if DX9 || DX11
      D.vf(vf->vf);
   #elif GL
      VF=vf->vf; // remember for future use
   #endif

   if(VI._vtx_buf) // if we already have a vtx buffer locked, then we need to adjust it
   {
      VI_vb->_vtx_size=VI._vb._vtx_size; // copy current settings
      VI_vb->_vtx_num =VI._vb._vtx_num ; // copy current settings
      VI_vb->set(); // make sure that it's active (some other vtx buffer could have been activated in the meantime)
   #if GL
      D     .vf (VF); // set VF after VBO on OpenGL
   #endif
   }
#endif
}
void VtxIndBuf::clear()
{
   VI._vtx_type  =VI_NONE;
   VI._vtx_queued=0; // clear can be called by engine method to stop VI in progress (like in light drawing when detected that the rectangle doesn't intersect with the viewport), so clear any current vertexes for drawing
   VI._image     =null;
   VI._shader    =null;

   // we've changed textures so we need to clear material
   MaterialLast    =null;
   MaterialLast4[0]=null;

   if(VI._user_flag)
   {
      if(VI._user_flag&VI_CUSTOM_DEPTH_WRITE)D.depthWrite(VI._depth_write); // reset depth writing
      if(VI._user_flag&VI_CUSTOM_TEX_WRAP   ) // reset the sampler
      {
      #if DX11
         if(D._sampler2D)SamplerLinearClamp.setPS(SSI_DEFAULT);
         else            SamplerAnisotropic.setPS(SSI_DEFAULT);
      #else
         Sh.h_ImageCol[0]->_sampler=null;
      #endif
      }
      VI._user_flag=0; // clear the user flag
   }

   SetVtxNrmMulAdd(true);
}
void VtxIndBuf::setFirst(VI_TYPE vtx_type, UInt flag)
{
   if(!VI._vtx_type)VI.setType(vtx_type, flag);
}
void VtxIndBuf::end()
{
   flush();
   clear();
}
/******************************************************************************/
void VtxIndBuf::shader(Shader *shader) {VI._shader=shader;}
void VtxIndBuf::image(C Image *image)
{
   if(VI._image!=image)
   {
      flush(); // first flush what's already available, after that make the change
      VI._image=image;
      Sh.h_ImageCol[0]->set(image);
      Sh.h_ImageColMS ->set(image);
   }
}
void VtxIndBuf::color(C Color &color)
{
   if(VI._color!=color)
   {
      flush(); // first flush what's already available, after that make the change
      VI._color=color;
   }
   Sh.h_Color[0]->set(color);
}
void VtxIndBuf::color2(C Color &color)
{
   Sh.h_Color[1]->set(color);
}
void VtxIndBuf::cull      (Bool cull) {FlagSet(VI._user_flag, VI_CULL      , cull);}
void VtxIndBuf::alphaTest (Bool on  ) {FlagSet(VI._user_flag, VI_ALPHA_TEST, on  );}
void VtxIndBuf::fog       (Bool on  ) {FlagSet(VI._user_flag, VI_FOG       , on  );}
void VtxIndBuf::depthWrite(Bool on  ) {if(!(VI._user_flag&VI_CUSTOM_DEPTH_WRITE)){VI._depth_write=D._depth_write; VI._user_flag|=VI_CUSTOM_DEPTH_WRITE;} D.depthWrite(on);}

void VtxIndBuf::clamp()
{
   FlagEnable(VI._user_flag, VI_CUSTOM_TEX_WRAP);
#if DX11
   SamplerLinearClamp.setPS(SSI_DEFAULT);
#else
   Sh.h_ImageCol[0]->_sampler=&SamplerLinearClamp;
#endif
}
void VtxIndBuf::wrap()
{
   FlagEnable(VI._user_flag, VI_CUSTOM_TEX_WRAP);
#if DX11
   SamplerLinearWrap.setPS(SSI_DEFAULT);
#else
   Sh.h_ImageCol[0]->_sampler=&SamplerLinearWrap;
#endif
}
void VtxIndBuf::wrapX()
{
   FlagEnable(VI._user_flag, VI_CUSTOM_TEX_WRAP);
#if DX11
   SamplerLinearWCC.setPS(SSI_DEFAULT);
#else
   Sh.h_ImageCol[0]->_sampler=&SamplerLinearWCC;
#endif
}
void VtxIndBuf::wrapY()
{
   FlagEnable(VI._user_flag, VI_CUSTOM_TEX_WRAP);
#if DX11
   SamplerLinearCWC.setPS(SSI_DEFAULT);
#else
   Sh.h_ImageCol[0]->_sampler=&SamplerLinearCWC;
#endif
}
/******************************************************************************/
#if DX11
   #if 0
      extern void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY pt);
   #else
      INLINE void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY pt) {D3DC->IASetPrimitiveTopology(pt);}
   #endif
#endif
void VtxIndBuf::flush()
{
   if(VI._vtx_queued>0)
   {
      unlockVtx();
      if(VI._shader)
      {
         VI._shader->begin();
         if(VI._quad_ind) // this is always TRIANGLELIST
         {
            UInt quads=Unsigned(VI._vtx_queued)/4;
         #if DX9
            D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, VI._vtx_drawing, 0, VI._vtx_queued, 0, quads*2); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
         #elif DX11
          //if(VI._prim_type!=D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)SetPrimitiveTopology(VI._prim_type); must be after 'shader->begin', not needed since 'quad_ind' always uses TRIANGLELIST mode
            D3DC->DrawIndexed(quads*(2*3), 0, VI._vtx_drawing); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
         #elif GL_ES
            glDrawElements(GL_TRIANGLES, quads*(2*3), IndBuf16384Quads.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null);
         #elif GL
            glDrawElementsBaseVertex(GL_TRIANGLES, quads*(2*3), IndBuf16384Quads.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null, VI._vtx_drawing); if(GL_RING){VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;}
         #endif
         }else
         {
         #if DX9
            D3D->DrawPrimitive(VI._prim_type, VI._vtx_drawing, Max(0, VI._vtx_queued/VI._ind_div-VI._ind_sub)); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
         #elif DX11
            if(VI._prim_type!=D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)SetPrimitiveTopology(VI._prim_type); // must be after 'shader->begin'
            D3DC->Draw(VI._vtx_queued, VI._vtx_drawing); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
         #elif GL_ES
            glDrawArrays(VI._prim_type, 0, VI._vtx_queued);
         #elif GL
            glDrawArrays(VI._prim_type, VI._vtx_drawing, VI._vtx_queued); if(GL_RING){VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;}
         #endif
         }
         ShaderEnd();
      }
      VI._vtx_queued=0;
   }
}
void VtxIndBuf::flushIndexed(IndBuf &ib, Int ind_num)
{
   if(VI._vtx_queued>0)
   {
      unlockVtx();
      if(UInt(ind_num)<=UInt(ib._ind_num) && VI._shader) // inclusive 'InRange'
      {
         ib.set(); VI._shader->begin();
      #if DX9
         D3D->DrawIndexedPrimitive(VI._prim_type, VI._vtx_drawing, 0, VI._vtx_queued, 0, ind_num/VI._ind_div); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
      #elif DX11
         if(VI._prim_type!=D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)SetPrimitiveTopology(VI._prim_type); // must be after 'shader->begin'
         D3DC->DrawIndexed(ind_num, 0, VI._vtx_drawing); VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;
      #elif GL_ES
         glDrawElements(VI._prim_type, ind_num, ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null);
      #elif GL
         glDrawElementsBaseVertex(VI._prim_type, ind_num, ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null, VI._vtx_drawing); if(GL_RING){VI._vtx_drawing+=VI._vtx_queued; VI._vtx_drawing_raw=VI._vtx_drawing*VI._vb._vtx_size;}
      #endif
         ShaderEnd();
      }
      VI._vtx_queued=0;
   }
}
Ptr VtxIndBuf::addVtx(Int vtxs)
{
   Int total_vtxs =vtxs+VI._vtx_drawing+VI._vtx_queued;
   if( total_vtxs> VI._vb._vtx_num             ){flush(); unlockVtx(); VI._vtx_drawing=VI._vtx_drawing_raw=0; total_vtxs=vtxs;} // if vertexes don't fit then flush all queued, it is important that 'unlockVtx' is called !! it is because in 'flush' it will be called only if there are vertexes queued, and in case when we've already locked buffer for purpose of adding new vertexes, however that was interrupted with 'clear', then the buffer remains locked, however it may be locked using LOCK_APPEND, but for this case we always need to rewrite it from start using LOCK_WRITE, to notify the driver that we're going to overwrite existing data, so call 'unlockVtx' to make sure that next lock is OK
   if( total_vtxs<=VI._vb._vtx_num && lockVtx()){Ptr vtx=VI._vtx_buf+(VI._vtx_queued+VI._vtx_drawing)*VI._vb._vtx_size; VI._vtx_queued+=vtxs; return vtx;}
   return null;
}
/*U16* VtxIndBuf::addInd(Int inds)
{
   if(inds+VI._ind_queued<=VI._ib._ind_num && lockInd()){U16 *ind=VI._ind_buf+VI._ind_queued; VI._ind_queued+=inds; return ind;}
   return null;
}
Int VtxIndBuf::add(Int vtxs, Ptr &vtx, Int inds, U16* &ind)
{
   if(vtxs+VI._vtx_queued> VI._vb._vtx_num || inds+VI._ind_queued> VI._ib._ind_num)drawIndexed();
   if(vtxs+VI._vtx_queued<=VI._vb._vtx_num && inds+VI._ind_queued<=VI._ib._ind_num && lockVtx() && lockInd())
   {
      Int ret=VI._vtx_queued;
          vtx=VI._vtx_buf+VI._vtx_queued*VI._vb._vtx_size; VI._vtx_queued+=vtxs; 
          ind=VI._ind_buf+VI._ind_queued                 ; VI._ind_queued+=inds;
      return ret;
   }
   return -1;
}
/******************************************************************************/
void VtxIndBuf::dot(C Vec2 &pos, Flt r)
{
   setFirst(VI_2D_FLAT, VI_QUAD_IND);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(4))
   {
      v[0].pos.set(pos.x-r, pos.y+r);
      v[1].pos.set(pos.x+r, pos.y+r);
      v[2].pos.set(pos.x+r, pos.y-r);
      v[3].pos.set(pos.x-r, pos.y-r);
   }
}
void VtxIndBuf::dot(C Color &color, C Vec2 &pos, Flt r)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos.set(pos.x-r, pos.y+r);
      v[1].pos.set(pos.x+r, pos.y+r);
      v[2].pos.set(pos.x+r, pos.y-r);
      v[3].pos.set(pos.x-r, pos.y-r);
   }
}
void VtxIndBuf::dot(                C Vec &pos, Flt r) {Vec2 screen; if(PosToScreenM(pos, screen))dot(       screen, r);}
void VtxIndBuf::dot(C Color &color, C Vec &pos, Flt r) {Vec2 screen; if(PosToScreenM(pos, screen))dot(color, screen, r);}
/******************************************************************************/
void VtxIndBuf::rect(C Rect &rect)
{
   setFirst(VI_2D_FLAT, VI_QUAD_IND);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.max.x, rect.min.y);
      v[3].pos.set(rect.min.x, rect.min.y);
   }
}
void VtxIndBuf::rect(C Color &color, C Rect &rect)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.max.x, rect.min.y);
      v[3].pos.set(rect.min.x, rect.min.y);
   }
}
/******************************************************************************/
void VtxIndBuf::rectL(C Rect &rect)
{
   setFirst(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(8))
   {
      // drawing lines needs adjustments
      Rect r(rect.min.x+D._pixel_size_2.x, rect.min.y+D._pixel_size_2.y,
             rect.max.x-D._pixel_size_2.x, rect.max.y-D._pixel_size_2.y);
      v[0].pos.set(r.min.x, r.max.y); v[1].pos.set(r.max.x, r.max.y);
      v[2].pos.set(r.max.x, r.max.y); v[3].pos.set(r.max.x, r.min.y);
      v[4].pos.set(r.max.x, r.min.y); v[5].pos.set(r.min.x, r.min.y);
      v[6].pos.set(r.min.x, r.min.y); v[7].pos.set(r.min.x, r.max.y);
   }
}
void VtxIndBuf::rectL(C Color &color, C Rect &rect)
{
   setFirst(VI_2D_COL, VI_LINE);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(8))
   {
      // drawing lines needs adjustments
      Rect r(rect.min.x+D._pixel_size_2.x, rect.min.y+D._pixel_size_2.y,
             rect.max.x-D._pixel_size_2.x, rect.max.y-D._pixel_size_2.y);
      v[0].color=v[1].color=v[2].color=v[3].color=v[4].color=v[5].color=v[6].color=v[7].color=color;
      v[0].pos.set(r.min.x, r.max.y); v[1].pos.set(r.max.x, r.max.y);
      v[2].pos.set(r.max.x, r.max.y); v[3].pos.set(r.max.x, r.min.y);
      v[4].pos.set(r.max.x, r.min.y); v[5].pos.set(r.min.x, r.min.y);
      v[6].pos.set(r.min.x, r.min.y); v[7].pos.set(r.min.x, r.max.y);
   }
}
/******************************************************************************/
void VtxIndBuf::rectShadedX(C Color &color0, C Color &color1, C Rect &rect)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[3].color=color0;
      v[1].color=v[2].color=color1;
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.max.x, rect.min.y);
      v[3].pos.set(rect.min.x, rect.min.y);
   }
}
void VtxIndBuf::rectShadedY(C Color &color0, C Color &color1, C Rect &rect)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[1].color=color0;
      v[2].color=v[3].color=color1;
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.max.x, rect.min.y);
      v[3].pos.set(rect.min.x, rect.min.y);
   }
}
/******************************************************************************/
void VtxIndBuf::line(C Vec2 &a, C Vec2 &b)
{
   setFirst(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(2))
   {
      v[0].pos=a;
      v[1].pos=b;
   }
}
void VtxIndBuf::line(C Vec2 &a, C Vec2 &b, Flt width)
{
   Vec2 perp=Perp(b-a); perp.setLength(width);
   setFirst(VI_2D_FLAT, VI_QUAD_IND);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(4))
   {
      v[0].pos.set(a.x-perp.x, a.y-perp.y);
      v[1].pos.set(b.x-perp.x, b.y-perp.y);
      v[2].pos.set(b.x+perp.x, b.y+perp.y);
      v[3].pos.set(a.x+perp.x, a.y+perp.y);
   }
}
void VtxIndBuf::line(C Vec &a, C Vec &b)
{
   setFirst(VI_3D_FLAT, VI_LINE);
   if(Vtx3DFlat *v=(Vtx3DFlat*)addVtx(2))
   {
      v[0].pos=a;
      v[1].pos=b;
   }
}
void VtxIndBuf::line(C Color &color, C Vec2 &a, C Vec2 &b)
{
   setFirst(VI_2D_COL, VI_LINE);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(2))
   {
      v[0].color=v[1].color=color;
      v[0].pos=a;
      v[1].pos=b;
   }
}
void VtxIndBuf::line(C Color &color, C Vec &a, C Vec &b)
{
   setFirst(VI_3D_COL, VI_LINE);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(2))
   {
      v[0].color=v[1].color=color;
      v[0].pos=a;
      v[1].pos=b;
   }
}
void VtxIndBuf::line(C Color &col_a, C Color &col_b, C Vec2 &a, C Vec2 &b)
{
   setFirst(VI_2D_COL, VI_LINE);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(2))
   {
      v[0].pos=a; v[0].color=col_a;
      v[1].pos=b; v[1].color=col_b;
   }
}
void VtxIndBuf::line(C Color &col_a, C Color &col_b, C Vec &a, C Vec &b)
{
   setFirst(VI_3D_COL, VI_LINE);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(2))
   {
      v[0].pos=a; v[0].color=col_a;
      v[1].pos=b; v[1].color=col_b;
   }
}
/******************************************************************************/
void VtxIndBuf::tri(C Tri2 &tri)
{
   setFirst(VI_2D_FLAT);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(3))
   {
      v[0].pos=tri.p[0];
      v[1].pos=tri.p[1];
      v[2].pos=tri.p[2];
   }
}
void VtxIndBuf::tri(C Tri &tri)
{
   setFirst(VI_3D_FLAT);
   if(Vtx3DFlat *v=(Vtx3DFlat*)addVtx(3))
   {
      v[0].pos=tri.p[0];
      v[1].pos=tri.p[1];
      v[2].pos=tri.p[2];
   }
}
void VtxIndBuf::tri(C Vec2 &a, C Vec2 &b, C Vec2 &c)
{
   setFirst(VI_2D_FLAT);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(3))
   {
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
   }
}
void VtxIndBuf::tri(C Vec &a, C Vec &b, C Vec &c)
{
   setFirst(VI_3D_FLAT);
   if(Vtx3DFlat *v=(Vtx3DFlat*)addVtx(3))
   {
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
   }
}
void VtxIndBuf::tri(C Color &color, C Vec2 &a, C Vec2 &b, C Vec2 &c)
{
   setFirst(VI_2D_COL);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(3))
   {
      v[0].color=v[1].color=v[2].color=color;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
   }
}
void VtxIndBuf::tri(C Color &color, C Vec &a, C Vec &b, C Vec &c)
{
   setFirst(VI_3D_COL);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(3))
   {
      v[0].color=v[1].color=v[2].color=color;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
   }
}
void VtxIndBuf::tri(C Color &col_a, C Color &col_b, C Color &col_c, C Vec2 &a, C Vec2 &b, C Vec2 &c)
{
   setFirst(VI_2D_COL);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(3))
   {
      v[0].pos=a; v[0].color=col_a;
      v[1].pos=b; v[1].color=col_b;
      v[2].pos=c; v[2].color=col_c;
   }
}
void VtxIndBuf::tri(C Color &col_a, C Color &col_b, C Color &col_c, C Vec &a, C Vec &b, C Vec &c)
{
   setFirst(VI_3D_COL);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(3))
   {
      v[0].pos=a; v[0].color=col_a;
      v[1].pos=b; v[1].color=col_b;
      v[2].pos=c; v[2].color=col_c;
   }
}
/******************************************************************************/
void VtxIndBuf::quad(C Quad2 &quad)
{
   setFirst(VI_2D_FLAT, VI_QUAD_IND);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(4))
   {
      v[0].pos=quad.p[0];
      v[1].pos=quad.p[1];
      v[2].pos=quad.p[2];
      v[3].pos=quad.p[3];
   }
}
void VtxIndBuf::quad(C Quad &quad)
{
   setFirst(VI_3D_FLAT, VI_QUAD_IND);
   if(Vtx3DFlat *v=(Vtx3DFlat*)addVtx(4))
   {
      v[0].pos=quad.p[0];
      v[1].pos=quad.p[1];
      v[2].pos=quad.p[2];
      v[3].pos=quad.p[3];
   }
}
void VtxIndBuf::quad(C Vec2 &a, C Vec2 &b, C Vec2 &c, C Vec2 &d)
{
   setFirst(VI_2D_FLAT, VI_QUAD_IND);
   if(Vtx2DFlat *v=(Vtx2DFlat*)addVtx(4))
   {
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
void VtxIndBuf::quad(C Vec &a, C Vec &b, C Vec &c, C Vec &d)
{
   setFirst(VI_3D_FLAT, VI_QUAD_IND);
   if(Vtx3DFlat *v=(Vtx3DFlat*)addVtx(4))
   {
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
void VtxIndBuf::quad(C Color &color, C Quad2 &quad)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos=quad.p[0];
      v[1].pos=quad.p[1];
      v[2].pos=quad.p[2];
      v[3].pos=quad.p[3];
   }
}
void VtxIndBuf::quad(C Color &color, C Quad &quad)
{
   setFirst(VI_3D_COL, VI_QUAD_IND);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos=quad.p[0];
      v[1].pos=quad.p[1];
      v[2].pos=quad.p[2];
      v[3].pos=quad.p[3];
   }
}
void VtxIndBuf::quad(C Color &color, C Vec2 &a, C Vec2 &b, C Vec2 &c, C Vec2 &d)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
void VtxIndBuf::quad(C Color &color, C Vec &a, C Vec &b, C Vec &c, C Vec &d)
{
   setFirst(VI_3D_COL, VI_QUAD_IND);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(4))
   {
      v[0].color=v[1].color=v[2].color=v[3].color=color;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
void VtxIndBuf::quad(C Color &c0, C Color &c1, C Vec2 &a, C Vec2 &b, C Vec2 &c, C Vec2 &d)
{
   setFirst(VI_2D_COL, VI_QUAD_IND);
   if(Vtx2DCol *v=(Vtx2DCol*)addVtx(4))
   {
      v[0].color=v[3].color=c0;
      v[1].color=v[2].color=c1;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
void VtxIndBuf::quad(C Color &c0, C Color &c1, C Vec &a, C Vec &b, C Vec &c, C Vec &d)
{
   setFirst(VI_3D_COL, VI_QUAD_IND);
   if(Vtx3DCol *v=(Vtx3DCol*)addVtx(4))
   {
      v[0].color=v[3].color=c0;
      v[1].color=v[2].color=c1;
      v[0].pos=a;
      v[1].pos=b;
      v[2].pos=c;
      v[3].pos=d;
   }
}
/******************************************************************************/
void VtxIndBuf::image(C Rect &screen_rect)
{
   setFirst(VI_2D_TEX, VI_QUAD_IND|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.min.x, screen_rect.min.y);
      if(VI._image && VI._image->partial())
      {
         v[0].tex.x=v[3].tex.x=0;
         v[1].tex.x=v[2].tex.x=VI._image->_part.x;
         v[0].tex.y=v[1].tex.y=0;
         v[2].tex.y=v[3].tex.y=VI._image->_part.y;
      }else
      {
         v[0].tex.set(0, 0);
         v[1].tex.set(1, 0);
         v[2].tex.set(1, 1);
         v[3].tex.set(0, 1);
      }
   }
}
void VtxIndBuf::imagePart(C Rect &screen_rect, C Rect &tex_rect)
{
   setFirst(VI_2D_TEX, VI_QUAD_IND|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.min.x, screen_rect.min.y);
      if(VI._image && VI._image->partial())
      {
         v[0].tex.x=v[3].tex.x=tex_rect.min.x*VI._image->_part.x;
         v[1].tex.x=v[2].tex.x=tex_rect.max.x*VI._image->_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*VI._image->_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*VI._image->_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.max.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.min.x, tex_rect.max.y);
      }
   }
}
void VtxIndBuf::font(C Rect &screen_rect, C Rect &tex_rect)
{
   setFirst(VI_2D_FONT, VI_QUAD_IND);
   if(Vtx2DFont *v=(Vtx2DFont*)addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y); v[0].shade=( 1+0.2f)*2.5f+0.5f;
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y); v[1].shade=( 1+0.2f)*2.5f+0.5f;
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y); v[2].shade=(-1+0.2f)*2.5f+0.5f;
      v[3].pos.set(screen_rect.min.x, screen_rect.min.y); v[3].shade=(-1+0.2f)*2.5f+0.5f;
      if(VI._image && VI._image->partial())
      {
         v[0].tex.x=v[3].tex.x=tex_rect.min.x*VI._image->_part.x;
         v[1].tex.x=v[2].tex.x=tex_rect.max.x*VI._image->_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*VI._image->_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*VI._image->_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.max.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.min.x, tex_rect.max.y);
      }
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx2DTex &a, C Vtx2DTex &b, C Vtx2DTex &c)
{
   setFirst(VI_2D_TEX);
   if(Vtx2DTex *v=(Vtx2DTex*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx2DTex &a, C Vtx2DTex &b, C Vtx2DTex &c, C Vtx2DTex &d)
{
   setFirst(VI_2D_TEX, VI_QUAD_IND);
   if(Vtx2DTex *v=(Vtx2DTex*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx2DTexCol &a, C Vtx2DTexCol &b, C Vtx2DTexCol &c)
{
   setFirst(VI_2D_TEX_COL);
   if(Vtx2DTexCol *v=(Vtx2DTexCol*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx2DTexCol &a, C Vtx2DTexCol &b, C Vtx2DTexCol &c, C Vtx2DTexCol &d)
{
   setFirst(VI_2D_TEX_COL, VI_QUAD_IND);
   if(Vtx2DTexCol *v=(Vtx2DTexCol*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx3DTex &a, C Vtx3DTex &b, C Vtx3DTex &c)
{
   setFirst(VI_3D_TEX);
   if(Vtx3DTex *v=(Vtx3DTex*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx3DTex &a, C Vtx3DTex &b, C Vtx3DTex &c, C Vtx3DTex &d)
{
   setFirst(VI_3D_TEX, VI_QUAD_IND);
   if(Vtx3DTex *v=(Vtx3DTex*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx3DTexCol &a, C Vtx3DTexCol &b, C Vtx3DTexCol &c)
{
   setFirst(VI_3D_TEX_COL);
   if(Vtx3DTexCol *v=(Vtx3DTexCol*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx3DTexCol &a, C Vtx3DTexCol &b, C Vtx3DTexCol &c, C Vtx3DTexCol &d)
{
   setFirst(VI_3D_TEX_COL, VI_QUAD_IND);
   if(Vtx3DTexCol *v=(Vtx3DTexCol*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx3DSimple &a, C Vtx3DSimple &b, C Vtx3DSimple &c)
{
   setFirst(VI_3D_SIMPLE);
   if(Vtx3DSimple *v=(Vtx3DSimple*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx3DSimple &a, C Vtx3DSimple &b, C Vtx3DSimple &c, C Vtx3DSimple &d)
{
   setFirst(VI_3D_SIMPLE, VI_QUAD_IND);
   if(Vtx3DSimple *v=(Vtx3DSimple*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx3DStandard &a, C Vtx3DStandard &b, C Vtx3DStandard &c)
{
   setFirst(VI_3D_STANDARD);
   if(Vtx3DStandard *v=(Vtx3DStandard*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      if(VI._image && VI._image->partial())REP(3)v[i].tex*=VI._image->_part.xy;
   }
}
void VtxIndBuf::face(C Vtx3DStandard &a, C Vtx3DStandard &b, C Vtx3DStandard &c, C Vtx3DStandard &d)
{
   setFirst(VI_3D_STANDARD, VI_QUAD_IND);
   if(Vtx3DStandard *v=(Vtx3DStandard*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
      if(VI._image && VI._image->partial())REP(4)v[i].tex*=VI._image->_part.xy;
   }
}
/******************************************************************************/
void VtxIndBuf::face(C Vtx3DFull &a, C Vtx3DFull &b, C Vtx3DFull &c)
{
   setFirst(VI_3D_FULL);
   if(Vtx3DFull *v=(Vtx3DFull*)addVtx(3))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
   }
}
void VtxIndBuf::face(C Vtx3DFull &a, C Vtx3DFull &b, C Vtx3DFull &c, C Vtx3DFull &d)
{
   setFirst(VI_3D_FULL, VI_QUAD_IND);
   if(Vtx3DFull *v=(Vtx3DFull*)addVtx(4))
   {
      v[0]=a;
      v[1]=b;
      v[2]=c;
      v[3]=d;
   }
}
/******************************************************************************/
void InitVtxInd()
{
   if(LogInit)LogN("InitVtxInd");
   if(!D._can_draw)return;
   VI.create();

   Int quads=16384; // 16384 is the maximum number of quads that still use 16-bit indexes (16384*4 == 65536)
   IndBuf16384Quads.create(quads*6, true, false);
   if(U16 *ind=(U16*)IndBuf16384Quads.lock(LOCK_WRITE))
   {
      Int p=0; REP(quads){ind[0]=p; ind[1]=p+1; ind[2]=p+3; ind[3]=p+3; ind[4]=p+1; ind[5]=p+2; p+=4; ind+=6;}
      IndBuf16384Quads.unlock();
   }else Exit("Can't create Index Buffer");

      IndBufBorder.create(4*2*3, true, false);
   if(IndBufBorder.lock(LOCK_WRITE))
   {
      IndBufBorder.setTri(0, 0,1,4);
      IndBufBorder.setTri(1, 1,5,4);
      IndBufBorder.setTri(2, 1,2,5);
      IndBufBorder.setTri(3, 2,6,5);
      IndBufBorder.setTri(4, 3,0,7);
      IndBufBorder.setTri(5, 0,4,7);
      IndBufBorder.setTri(6, 8,3,9);
      IndBufBorder.setTri(7, 3,7,9);
      IndBufBorder.unlock();
   }else Exit("Can't create Index Buffer");

      IndBufPanel.create(3*3*2*3, true, false);
   if(IndBufPanel.lock(LOCK_WRITE))
   {
      IndBufPanel.setTri( 0,  0, 1, 4); // top
      IndBufPanel.setTri( 1,  1, 5, 4);
      IndBufPanel.setTri( 2,  1, 2, 5);
      IndBufPanel.setTri( 3,  2, 6, 5);
      IndBufPanel.setTri( 4,  2, 3, 6);
      IndBufPanel.setTri( 5,  3, 7, 6);
      IndBufPanel.setTri( 6,  8, 9,12); // bottom
      IndBufPanel.setTri( 7,  9,13,12);
      IndBufPanel.setTri( 8,  9,10,13);
      IndBufPanel.setTri( 9, 10,14,13);
      IndBufPanel.setTri(10, 10,11,14);
      IndBufPanel.setTri(11, 11,15,14);
      IndBufPanel.setTri(12,  4, 5, 8); // mid-left
      IndBufPanel.setTri(13,  5, 9, 8); // mid-left
      IndBufPanel.setTri(14,  6, 7,10); // mid-right
      IndBufPanel.setTri(15,  7,11,10); // mid-right
      IndBufPanel.setTri(16,  5, 6, 9); // mid-center (this must be last so we can draw just borders)
      IndBufPanel.setTri(17,  6,10, 9); // mid-center (this must be last so we can draw just borders)
      IndBufPanel.unlock();
   }else Exit("Can't create Index Buffer");

      IndBufPanelEx.create(3*3*2*3, true, false);
   if(IndBufPanelEx.lock(LOCK_WRITE))
   {
      IndBufPanelEx.setTri( 0,  0, 1, 4); // top
      IndBufPanelEx.setTri( 1,  1, 5, 4);
      IndBufPanelEx.setTri( 2,  1, 2, 5);
      IndBufPanelEx.setTri( 3,  2, 6, 5);
      IndBufPanelEx.setTri( 4,  2, 3, 6);
      IndBufPanelEx.setTri( 5,  3, 7, 6);
      IndBufPanelEx.setTri( 6, 10,14,16); // bottom
      IndBufPanelEx.setTri( 7, 14,17,16);
      IndBufPanelEx.setTri( 8, 14,15,17);
      IndBufPanelEx.setTri( 9, 15,18,17);
      IndBufPanelEx.setTri(10, 15,13,18);
      IndBufPanelEx.setTri(11, 13,19,18);
      IndBufPanelEx.setTri(12,  4, 8,10); // mid-left
      IndBufPanelEx.setTri(13,  8,11,10); // mid-left
      IndBufPanelEx.setTri(14,  9, 7,12); // mid-right
      IndBufPanelEx.setTri(15,  7,13,12); // mid-right
      IndBufPanelEx.setTri(16,  8, 9,11); // mid-center (this must be last so we can draw just borders)
      IndBufPanelEx.setTri(17,  9,12,11); // mid-center (this must be last so we can draw just borders)
      IndBufPanelEx.unlock();
   }else Exit("Can't create Index Buffer");

      IndBufRectBorder.create(4*2*3, true, false);
   if(IndBufRectBorder.lock(LOCK_WRITE))
   {
      IndBufRectBorder.setTri(0, 0,1,4);
      IndBufRectBorder.setTri(1, 1,5,4);
      IndBufRectBorder.setTri(2, 1,2,5);
      IndBufRectBorder.setTri(3, 2,6,5);
      IndBufRectBorder.setTri(4, 2,3,6);
      IndBufRectBorder.setTri(5, 3,7,6);
      IndBufRectBorder.setTri(6, 3,0,7);
      IndBufRectBorder.setTri(7, 0,4,7);
      IndBufRectBorder.unlock();
   }else Exit("Can't create Index Buffer");

      IndBufRectShaded.create(5*2*3, true, false);
   if(IndBufRectShaded.lock(LOCK_WRITE))
   {
      IndBufRectShaded.setTri(0, 0,1,4);
      IndBufRectShaded.setTri(1, 1,5,4);
      IndBufRectShaded.setTri(2, 1,2,5);
      IndBufRectShaded.setTri(3, 2,6,5);
      IndBufRectShaded.setTri(4, 2,3,6);
      IndBufRectShaded.setTri(5, 3,7,6);
      IndBufRectShaded.setTri(6, 3,0,7);
      IndBufRectShaded.setTri(7, 0,4,7);
      IndBufRectShaded.setTri(8, 4,5,6); // middle
      IndBufRectShaded.setTri(9, 6,7,4); // middle
      IndBufRectShaded.unlock();
   }else Exit("Can't create Index Buffer");
}
void ShutVtxInd()
{
   IndBufRectShaded.del();
   IndBufRectBorder.del();
   IndBufPanel     .del();
   IndBufPanelEx   .del();
   IndBufBorder    .del();
   IndBuf16384Quads.del();
   VI              .del();
   VtxFormats      .del();
}
/******************************************************************************/
}
/******************************************************************************/
