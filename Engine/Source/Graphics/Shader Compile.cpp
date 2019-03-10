/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
/******************************************************************************

   If in the future compilation no longer is based on D3DX9 then remove:
      ThirdPartyLibs\D3DX9
      ThirdPartyLibs\D3DX11

/******************************************************************************/
#if WINDOWS
#define THIS void
#undef  MAX_FVF_DECL_SIZE
#include "../../../ThirdPartyLibs/begin.h"
#if WINDOWS_OLD
#include "../../../ThirdPartyLibs/D3DX9/d3dx9shader.h"
#endif
#include "../../../ThirdPartyLibs/D3DX11/inc/d3dx11effect.h"
#include "../../../ThirdPartyLibs/end.h"
#endif
namespace EE{
/******************************************************************************/
#define CC4_SHDR CC4('S','H','D','R')
/******************************************************************************/
enum SHADER_TYPE
{
   SHADER_GL  ,
   SHADER_DX9 ,
   SHADER_DX11,
};
/******************************************************************************/
#define FLAGS_DX9     (1 ? (D3DXSHADER_OPTIMIZATION_LEVEL3|D3DXSHADER_NO_PRESHADER|D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY) : (D3DXSHADER_OPTIMIZATION_LEVEL0|D3DXSHADER_NO_PRESHADER|D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY)) // D3DXSHADER_SKIPOPTIMIZATION made "Ambient Occlusion" shader don't compile
#define FLAGS_DX11    (D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY|D3DCOMPILE_OPTIMIZATION_LEVEL3|D3DCOMPILE_NO_PRESHADER)
#define SHOW_GLSL_SRC 0

#if   0 // Test #1: shader size 6.61 MB, engine load+render 1 frame = 0.40s on Windows, decompression 10x faster
   #define COMPRESS_GL       COMPRESS_LZ4
   #define COMPRESS_GL_LEVEL 11
#elif 0 // Test #2: Shaders compiled in: 50.527s, load+draw 1 frame: 0.86s, Main size 346 KB
   #define COMPRESS_GL       COMPRESS_ZSTD
   #define COMPRESS_GL_LEVEL 99
#elif 1 // Test #1: shader size 5.15 MB, engine load+render 1 frame = 0.45s on Windows, decompression 10x slower, Test #2: Shaders compiled in: 50.791s, load+draw 1 frame: 0.88s, Main size 337 KB
   #define COMPRESS_GL       COMPRESS_LZMA
   #define COMPRESS_GL_LEVEL 9 // shader files are small, so we can use high compression level and still get small dictionary size / memory usage
#else // shader size was slightly bigger than LZMA, and loading all shader techs was bit slower
   #define COMPRESS_GL       COMPRESS_LZHAM
   #define COMPRESS_GL_LEVEL 5
#endif
   #define COMPRESS_GL_MT    true
/******************************************************************************/
#pragma pack(push, 1)
struct ConstantIndex9
{
   Byte   count;
   UShort start, offset, src_index;

         void set(Int start, Int count, Int offset, Int src_index) {_Unaligned(T.start, start); _Unaligned(T.count, count); _Unaligned(T.offset, offset); _Unaligned(T.src_index, src_index); DYNAMIC_ASSERT(T.start==start && T.count==count && T.offset==offset && T.src_index==src_index, S+"Constant index out of range: "+start+", "+count+", "+offset+", "+src_index);}
   ConstantIndex9(Int start, Int count, Int offset, Int src_index) {set(start, count, offset, src_index);}
   ConstantIndex9() {}
};
struct TextureIndex9
{
   UShort bind_index, src_index;

        void set(Int bind_index, Int src_index) {_Unaligned(T.bind_index, bind_index); _Unaligned(T.src_index, src_index); DYNAMIC_ASSERT(T.bind_index==bind_index && T.src_index==src_index, "Constant index out of range");}
   TextureIndex9(Int bind_index, Int src_index) {set(bind_index, src_index);}
   TextureIndex9() {}
};
struct ConstantIndex
{
   Byte  bind_index;
   UShort src_index;

        void set(Int bind_index, Int src_index) {_Unaligned(T.bind_index, bind_index); _Unaligned(T.src_index, src_index); DYNAMIC_ASSERT(T.bind_index==bind_index && T.src_index==src_index, "Constant index out of range");}
   ConstantIndex(Int bind_index, Int src_index) {set(bind_index, src_index);}
   ConstantIndex() {}
};
#pragma pack(pop)

#if WINDOWS_OLD
static Int Compare(C Shader9::Constant &a, C Shader9::Constant &b) {return Compare(a.start, b.start);}
#endif
static Int Compare(C ShaderBufferParams &a, C ShaderBufferParams &b) {return ComparePtr(a.buffer, b.buffer);} // sort by buffer pointer, because that's the only thing we can access from 'Shader.Constant'
static Int Compare(C ShaderBufferParams &a,   ShaderBuffer*C     &b) {return ComparePtr(a.buffer, b       );} // sort by buffer pointer, because that's the only thing we can access from 'Shader.Constant'
static Int Compare(  ShaderImage*C      &a,   ShaderImage*C      &b) {return ComparePtr(a       , b       );} // sort by image  pointer, because that's the only thing we can access from 'Shader.Texture'

static Int GetIndex(C Memc<ShaderImage*      > & images, ShaderImage  *image ) {Int index; if(! images.binarySearch(image , index, Compare))Exit("Image not found in Shader" ); return index;}
static Int GetIndex(C Map <Str8, ShaderParam > & params, ShaderParam  *param ) {Int index  =    params. dataToIndex(param     ); if(index<0)Exit("Param not found in Shader" ); return index;}
static Int GetIndex(C Memc<ShaderBufferParams> &buffers, ShaderBuffer *buffer) {Int index; if(!buffers.binarySearch(buffer, index, Compare))Exit("Buffer not found in Shader"); return index;}

static UShort AsUShort(Int i) {DYNAMIC_ASSERT(InRange(i, USHORT_MAX+1), "Value too big to be represented as UShort"); return i;}

static ShaderImage * Get(Int i, C MemtN<ShaderImage *, 256> &images ) {if(!InRange(i, images ))Exit("Invalid ShaderImage index" ); return  images[i];}
static ShaderParam * Get(Int i, C MemtN<ShaderParam *, 256> &params ) {if(!InRange(i, params ))Exit("Invalid ShaderParam index" ); return  params[i];}
static ShaderBuffer* Get(Int i, C MemtN<ShaderBuffer*, 256> &buffers) {if(!InRange(i, buffers))Exit("Invalid ShaderBuffer index"); return buffers[i];}

#if DEBUG
static C Str8& Name(ShaderImage  &image                                  ) {C Str8 *name=ShaderImages .dataToKey(&image ); if(!name)Exit("Can't find ShaderImage name" ); return *name;}
static C Str8& Name(ShaderParam  &param, C Map<Str8, ShaderParam> &params) {C Str8 *name=      params .dataToKey(&param ); if(!name)Exit("Can't find ShaderParam name" ); return *name;}
static C Str8& Name(ShaderBuffer &buffer                                 ) {C Str8 *name=ShaderBuffers.dataToKey(&buffer); if(!name)Exit("Can't find ShaderBuffer name"); return *name;}
#else
static C Str8& Name(ShaderImage  &image                                  ) {return ShaderImages .dataInMapToKey(image );}
static C Str8& Name(ShaderParam  &param, C Map<Str8, ShaderParam> &params) {return       params .dataInMapToKey(param );}
static C Str8& Name(ShaderBuffer &buffer                                 ) {return ShaderBuffers.dataInMapToKey(buffer);}
#endif
/******************************************************************************/
// INCLUDE
/******************************************************************************/
#if WINDOWS
struct ShaderPath
{
   Char path[MAX_LONG_PATH];
};
/******************************************************************************/
#if WINDOWS_OLD
struct Include9 : ID3DXInclude // 'Include9' should not have 'root' because unlike DX10 we're loading first file from path, and not from memory, also on DX9 'pParentData' is buggy and sometimes gives null
{
   Memc<Str> paths; // use this because 'pParentData' can be null

   HRESULT __stdcall Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
   {
     *ppData=null;
     *pBytes=0;

      File f;
      Str  path=GetPath(pFileName);
      if(!FullPath(path))
         if(ShaderPath *parent=(pParentData ? (ShaderPath*)pParentData-1 : null))
            path=NormalizePath(Str(parent->path).tailSlash(true)+path);
         else // if DX9 fails to provide 'pParentData' we must use previously detected paths
      {
         REPA(paths)if(f.readStdTry(paths[i]+pFileName)){path=GetPath(paths[i]+pFileName); break;}
      }

      if(f.is() || f.readStdTry(path.tailSlash(true)+GetBase(pFileName)))
      {
         Byte *data=Alloc<Byte>(SIZEU(ShaderPath)+f.size());
         Set(((ShaderPath*)data)->path, path);
         paths.include(path.tailSlash(true));
         data+=SIZE(ShaderPath);
         f.get(data, f.size());
        *ppData=data;
        *pBytes=f.size();
         return S_OK;
      }
      return -1;
   }
   HRESULT __stdcall Close(LPCVOID pData)
   {
      if(pData)
      {
         Byte *data=((Byte*)pData)-SIZE(ShaderPath);
         Free( data);
      }
      return S_OK;
   }

   Include9(C Str &src) {}
};
#endif
/******************************************************************************/
struct Include11 : ID3DInclude
{
   ShaderPath root;

   HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
   {
     *ppData=null;
     *pBytes=0;

      Str path=GetPath(pFileName);
      if(!FullPath(path))
         if(ShaderPath *parent=(pParentData ? (ShaderPath*)pParentData-1 : &root))
            path=NormalizePath(Str(parent->path).tailSlash(true)+path);

      File f; if(f.readStdTry(path.tailSlash(true)+GetBase(pFileName)))
      {
         Byte *data=Alloc<Byte>(SIZEU(ShaderPath)+f.size());
         Set(((ShaderPath*)data)->path, path);
         data+=SIZE(ShaderPath);
         f.get(data, f.size());
        *ppData=data;
        *pBytes=f.size();
         return S_OK;
      }
      return -1;
   }
   HRESULT __stdcall Close(LPCVOID pData)
   {
      if(pData)
      {
         Byte *data=((Byte*)pData)-SIZE(ShaderPath);
         Free( data);
      }
      return S_OK;
   }

   Include11(C Str &src)
   {
      Set(root.path, GetPath(src));
   }
};
#endif
/******************************************************************************/
// ERRORS
/******************************************************************************/
#if WINDOWS_OLD
static void Error(ID3DXBuffer* &error, Str *messages)
{
   if(error)
   {
      if(messages)messages->line()+=(Char8*)error->GetBufferPointer();
      RELEASE(error);
   }
}
#endif
#if WINDOWS
static void Error(ID3D10Blob* &error, Str *messages)
{
   if(error)
   {
      if(messages)messages->line()+=(Char8*)error->GetBufferPointer();
      RELEASE(error);
   }
}
#endif
/******************************************************************************/
// EFFECT CREATION
/******************************************************************************/
#if WINDOWS_OLD
static ID3DXEffect* CreateEffect9(C Str &src, C MemPtr<ShaderMacro> &macros, Str *messages)
{
#if DX9
   ID3DXEffect         *effect  =null;
   ID3DXBuffer         *buffer  =null, *error=null;
   ID3DXEffectCompiler *compiler=null;

   if(messages)messages->clear();
   Mems<D3DXMACRO> d3d_macros; d3d_macros.setNum(macros.elms()+1);
   FREPA(macros){D3DXMACRO &m=d3d_macros[i]; m.Name=macros[i].name; m.Definition=macros[i].definition;} Zero(d3d_macros.last());

   Include9 include(src); // let this exist in case 'CompileEffect' uses it
   D3DXCreateEffectCompilerFromFile(src, d3d_macros.data(), &include, FLAGS_DX9, &compiler, &error); Error(error, messages);
   if(compiler)
   {
      compiler->CompileEffect(FLAGS_DX9, &buffer, &error); Error(error, messages);
      RELEASE(compiler);
   }

   if(buffer)
   {
      {
         SyncLocker lock(D._lock);
         D3DXCreateEffect(GetD3D9(), buffer->GetBufferPointer(), buffer->GetBufferSize(), null, null, 0, null, &effect, &error); Error(error, messages);
      }
      RELEASE(buffer);
      if(!effect && messages)messages->line()+="Compilation succeeded but creating effect (D3DXCreateEffect) failed.";
   }

   return effect;
#elif DX11
   if(messages)messages->clear();

   File f; if(!f.readTry(src)){if(messages)messages->line()+="Failed to open file."; return false;}
   Mems<Byte> data; data.setNum(f.size()); if(!f.get(data.data(), f.size())){if(messages)messages->line()+="Failed to read from file."; return false;} f.del(); // release the file handle after reading

   Mems<D3D_SHADER_MACRO> d3d_macros; d3d_macros.setNum(macros.elms()+1);
   FREPA(macros){D3D_SHADER_MACRO &m=d3d_macros[i]; m.Name=macros[i].name; m.Definition=macros[i].definition;} Zero(d3d_macros.last());

   ID3D10Blob *buffer=null, *error=null;
   D3DCompile(data.data(), data.elms(), null, d3d_macros.data(), &Include11(src), null, "fx_2_0", FLAGS_DX11, 0, &buffer, &error); Error(error, messages);

   ID3DXEffect *effect=null;
   if(buffer)
   {
      {
         ID3DXBuffer *error=null;
         SyncLocker lock(D._lock);
         D3DXCreateEffect(GetD3D9(), buffer->GetBufferPointer(), buffer->GetBufferSize(), null, null, 0, null, &effect, &error); Error(error, messages);
      }
      RELEASE(buffer);
      if(!effect && messages)messages->line()+="Compilation succeeded but creating effect (D3DXCreateEffect) failed.";
   }
   return effect;
#else
   return null;
#endif
}
#endif
/******************************************************************************/
// TRANSLATION
/******************************************************************************/
#if WINDOWS_OLD
static void AddTranslation9(ShaderParam &sp, ID3DXEffect *effect, D3DXHANDLE par, C D3DXPARAMETER_DESC &par_desc)
{
   if(par_desc.Class==D3DXPC_SCALAR || par_desc.Class==D3DXPC_VECTOR) // for example Flt f,f[]; Vec2 v,v[]; Vec v,v[]; Vec4 v,v[];
   {
      if(par_desc.Rows!=1)Exit("Shader Param Rows!=1");
      if(par_desc.Type==D3DXPT_FLOAT)// || par_desc.Type==D3DXPT_INT || par_desc.Type==D3DXPT_BOOL)
      {
         FREP(Max(1, par_desc.Elements)) // array size
         {
            sp._full_translation.New().set(sp._cpu_data_size, sp._gpu_data_size, SIZE(Flt)*par_desc.Columns);
            sp._cpu_data_size+=SIZE(Flt )*par_desc.Columns;
            sp._gpu_data_size+=SIZE(Vec4);
         }
      }else Exit(S+"Unhandled Shader Parameter Type for \""+par_desc.Name+'"');
   }else
   if(par_desc.Class==D3DXPC_MATRIX_COLUMNS)
   {
      Exit("Need to test D3DXPC_MATRIX_COLUMNS translation");
      if(par_desc.Rows   >4)Exit("Shader Param Matrix Rows>4");
      if(par_desc.Columns>4)Exit("Shader Param Matrix Cols>4");
      if(par_desc.Type!=D3DXPT_FLOAT)Exit(S+"Unhandled Shader Parameter Type for \""+par_desc.Name+'"');
      FREP(Max(1, par_desc.Elements)) // array size
      {
         FREPD(y, par_desc.Columns)
         FREPD(x, par_desc.Rows   )sp._full_translation.New().set(sp._cpu_data_size+SIZE(Flt)*(y+x*par_desc.Columns), sp._gpu_data_size+SIZE(Flt)*(x+y*4), SIZE(Flt));
         sp._cpu_data_size+=SIZE(Flt)*par_desc.Rows*par_desc.Columns;
         sp._gpu_data_size+=SIZE(Flt)*            4*par_desc.Columns;
      }
   }else
   if(par_desc.Class==D3DXPC_MATRIX_ROWS)
   {
      if(par_desc.Rows   >4)Exit("Shader Param Matrix Rows>4");
      if(par_desc.Columns>4)Exit("Shader Param Matrix Cols>4");
      if(par_desc.Type!=D3DXPT_FLOAT)Exit(S+"Unhandled Shader Parameter Type for \""+par_desc.Name+'"');
      FREP(Max(1, par_desc.Elements)) // array size
      {
         FREPD(y, par_desc.Columns)
         FREPD(x, par_desc.Rows   )sp._full_translation.New().set(sp._cpu_data_size+SIZE(Flt)*(y+x*par_desc.Columns), sp._gpu_data_size+SIZE(Flt)*(x+y*4), SIZE(Flt));
         sp._cpu_data_size+=SIZE(Flt)*par_desc.Rows*par_desc.Columns;
         sp._gpu_data_size+=SIZE(Flt)*            4*par_desc.Columns;
      }
   }else
   if(par_desc.Class==D3DXPC_STRUCT)
   {
      FREP(Max(1, par_desc.Elements)) // array size
      {
         D3DXHANDLE elm=(par_desc.Elements ? effect->GetParameterElement(par, i) : par);
         FREP(par_desc.StructMembers) // number of members
         {
            D3DXHANDLE         child=effect->GetParameter(elm, i);
            D3DXPARAMETER_DESC par_desc;
            effect->GetParameterDesc(child, &par_desc);
            AddTranslation9(sp, effect, child, par_desc);
         }
      }
   }else
   {
      Exit("Unrecognized Shader Parameter Class");
   }
}
#endif
/******************************************************************************/
#if DX11
static void AddTranslation11(ShaderParam &sp, ID3DX11EffectVariable *par, C D3DX11_EFFECT_VARIABLE_DESC &var_desc, C D3DX11_EFFECT_TYPE_DESC &par_desc)
{
   if(par_desc.Elements<=1) // array size
   {
      if(par_desc.Class==D3D10_SVC_SCALAR || par_desc.Class==D3D10_SVC_VECTOR) // for example: Flt f,f[]; Vec2 v,v[]; Vec v,v[]; Vec4 v,v[];
      {
         if(par_desc.Rows!=1)Exit("Shader Param Rows!=1");
         if(par_desc.Type==D3D10_SVT_FLOAT)
         {
            sp._full_translation.New().set(sp._cpu_data_size, var_desc.BufferOffset, SIZE(Flt)*par_desc.Columns);
            sp._cpu_data_size+=SIZE(Flt)*par_desc.Columns;
         }else Exit(S+"Unhandled Shader Parameter Type for \""+var_desc.Name+'"');
      }else
      if(par_desc.Class==D3D10_SVC_MATRIX_COLUMNS)
      {
         if(par_desc.Rows   >4)Exit("Shader Param Matrix Rows>4");
         if(par_desc.Columns>4)Exit("Shader Param Matrix Cols>4");
         if(par_desc.Type!=D3D10_SVT_FLOAT)Exit(S+"Unhandled Shader Parameter Type for \""+var_desc.Name+'"');

         FREPD(y, par_desc.Columns)
         FREPD(x, par_desc.Rows   )sp._full_translation.New().set(sp._cpu_data_size+SIZE(Flt)*(y+x*par_desc.Columns), var_desc.BufferOffset+SIZE(Flt)*(x+y*4), SIZE(Flt));
         sp._cpu_data_size+=SIZE(Flt)*par_desc.Rows*par_desc.Columns;
      }else
      if(par_desc.Class==D3D10_SVC_STRUCT)
      {
         FREP(par_desc.Members) // number of members
         {
            ID3DX11EffectVariable *member=par->GetMemberByIndex(i);
            D3DX11_EFFECT_VARIABLE_DESC var_desc; member->GetDesc(&var_desc);
            D3DX11_EFFECT_TYPE_DESC     par_desc; member->GetType()->GetDesc(&par_desc);
            AddTranslation11(sp, member, var_desc, par_desc);
            RELEASE(member);
         }
      }
   }else
   {
      FREP(par_desc.Elements)
      {
         ID3DX11EffectVariable *elm=par->GetElement(i);
         D3DX11_EFFECT_VARIABLE_DESC var_desc; elm->GetDesc(&var_desc);
         D3DX11_EFFECT_TYPE_DESC     par_desc; elm->GetType()->GetDesc(&par_desc);
         AddTranslation11(sp, elm, var_desc, par_desc);
         RELEASE(elm);
      }
   }
}
#endif
/******************************************************************************/
#if WINDOWS_OLD
static void AddRegisters(Mems<Shader9::Constant> &constants, ShaderParam &sp, LPD3DXCONSTANTTABLE ct, D3DXHANDLE constant, C D3DXCONSTANT_DESC &desc, Int &gpu_data_offset)
{
   if(desc.Class==D3DXPC_SCALAR || desc.Class==D3DXPC_VECTOR) // for example: Flt f,f[]; Vec2 v,v[]; Vec v,v[]; Vec4 v,v[];
   {
      if(desc.RegisterCount)constants.New().set(desc.RegisterIndex, desc.RegisterCount, sp._data+gpu_data_offset, sp);
      gpu_data_offset+=desc.Elements*SIZE(Vec4); // array size
   }else
   if(desc.Class==D3DXPC_MATRIX_COLUMNS)
   {
      if(desc.RegisterCount)constants.New().set(desc.RegisterIndex, desc.RegisterCount, sp._data+gpu_data_offset, sp);
      gpu_data_offset+=desc.Elements*SIZE(Vec4)*desc.Columns; // array size
   }else
   if(desc.Class==D3DXPC_STRUCT)
   {
      FREP(desc.Elements) // array size
      {
         D3DXHANDLE elm=ct->GetConstantElement(constant, i);
         FREP(desc.StructMembers) // members count
         {
            D3DXHANDLE        child=ct->GetConstant(elm, i);
            D3DXCONSTANT_DESC desc[16];
            UINT              count=Elms(desc);
            ct->GetConstantDesc(child, desc, &count); if(count!=1)Exit("count!=1");
            AddRegisters(constants, sp, ct, child, desc[0], gpu_data_offset);
         }
      }
   }else
   {
      Exit("Unrecognized Shader Parameter Class");
   }
}
#endif
/******************************************************************************/
// SAVE
/******************************************************************************/
#if WINDOWS_OLD
static Bool ShaderSave(C Str &name, C Map<Str8, ShaderParam> &params, C Memc<ShaderImage*> &images, C Memc<ShaderVS9> &vs, C Memc<ShaderPS9> &ps, C Memc<Shader9> &techs)
{
   File f; if(f.writeTry(name))
   {
      f.putUInt (CC4_SHDR  ); // cc4
      f.putByte (SHADER_DX9); // type
      f.cmpUIntV(0         ); // version

      // params
      f.cmpUIntV(params.elms());
      FREPAO(    params).save(f, params.key(i));

      // images
      f.cmpUIntV(images.elms());
      FREPA(images)f.putStr(Name(*images[i]));

      if(vs    .save(f)) // shaders
      if(ps    .save(f))
      if(techs .save(f, params, images)) // techniques
         if(f.flushOK())return true;

      f.del(); FDelFile(name);
   }
   return false;
}
#endif
#if DX11
static Bool ShaderSave(C Str &name, C Memc<ShaderBufferParams> &buffers, C Memc<ShaderImage*> &images, C Memc<ShaderVS11> &vs, C Memc<ShaderHS11> &hs, C Memc<ShaderDS11> &ds, C Memc<ShaderPS11> &ps, C Memc<Shader11> &techs)
{
   File f; if(f.writeTry(name))
   {
      f.putUInt (CC4_SHDR   ); // cc4
      f.putByte (SHADER_DX11); // type
      f.cmpUIntV(0          ); // version

      // constants
      f.cmpUIntV(buffers.elms()); FREPA(buffers)
      {
       C ShaderBufferParams &buf=buffers[i];

         // constant buffer
         f.putStr(Name(*buf.buffer)).cmpUIntV(buf.buffer->size()).putSByte(buf.index); DYNAMIC_ASSERT(buf.index>=-1 && buf.index<=127, "buf.index out of range");

         // params
         if(!buf.params.save(f))return false;
      }

      // images
      f.cmpUIntV(images.elms());
      FREPA(images)f.putStr(Name(*images[i]));

      if(vs   .save(f)) // shaders
      if(hs   .save(f))
      if(ds   .save(f))
      if(ps   .save(f))
      if(techs.save(f, buffers, images)) // techniques
         if(f.flushOK())return true;

      f.del(); FDelFile(name);
   }
   return false;
}
#endif
static Bool ShaderSave(C Str &name, C Map<Str8, ShaderParam> &params, C Memc<ShaderImage*> &images, C Memc<ShaderVSGL> &vs, C Memc<ShaderPSGL> &ps, C Memc<ShaderGL> &techs)
{
   File f; if(f.writeTry(name))
   {
      f.putUInt (CC4_SHDR ); // cc4
      f.putByte (SHADER_GL); // type
      f.cmpUIntV(0        ); // version

      // params
      f.cmpUIntV(params.elms());
      FREPAO(    params).save(f, params.key(i));

      // images
      f.cmpUIntV(images.elms());
      FREPA(images)f.putStr(Name(*images[i]));

      if(vs    .save(f)) // shaders
      if(ps    .save(f))
      if(techs .save(f, params, images)) // techniques
         if(f.flushOK())return true;

      f.del(); FDelFile(name);
   }
   return false;
}
/******************************************************************************/
// COMPILE
/******************************************************************************/
#if WINDOWS_OLD
static Int Compare(C Shader9  &a, C Shader9  &b) {return CompareCS(a.name, b.name);}
#endif
#if WINDOWS
static Int Compare(C Shader11 &a, C Shader11 &b) {return CompareCS(a.name, b.name);}
#endif
static Int Compare(C ShaderGL &a, C ShaderGL &b) {return CompareCS(a.name, b.name);}
static Int Compare(C ShaderGL &a, C Str8     &b) {return CompareCS(a.name, b     );}
/******************************************************************************/
static Bool ShaderCompile9(C Str &src, C Str &dest, C MemPtr<ShaderMacro> &macros, Str *messages)
{
#if WINDOWS_OLD
   if(ID3DXEffect *effect=CreateEffect9(src, macros, messages))
   {
      ShaderFile             shader;
      Map<Str8, ShaderParam> params(CompareCS);
      Memc<ShaderImage*>     images;
      Memc<ShaderVS9>        vs    ;
      Memc<ShaderPS9>        ps    ;
      Memc<Shader9>          techs ;

      D3DXEFFECT_DESC desc; effect->GetDesc(&desc);

      // build list of images and parameters
      FREP(desc.Parameters)if(D3DXHANDLE par=effect->GetParameter(null, i))
      {
         D3DXPARAMETER_DESC par_desc; effect->GetParameterDesc(par, &par_desc);

         if(par_desc.Class==D3DXPC_OBJECT)
         {
            switch(par_desc.Type)
            {
             //case D3DXPT_TEXTURE: break;

               case D3DXPT_SAMPLER:
               case D3DXPT_SAMPLER1D:
               case D3DXPT_SAMPLER2D:
               case D3DXPT_SAMPLER3D:
               case D3DXPT_SAMPLERCUBE:
                  images.add(ShaderImages(Str8Temp(par_desc.Name))); break;
            }
         }else
         {
            ShaderParam &sp=*params(Str8Temp(par_desc.Name));
            if(sp.is())Exit(S+"Shader parameter \""+par_desc.Name+"\" listed more than once");else // if wasn't yet created
            {
               sp._owns_data=true;
               sp._elements =par_desc.Elements;

               AddTranslation9(sp, effect, par, par_desc);
               sp.optimize(); // required for setting default value

               if(sp._cpu_data_size!=par_desc.Bytes)Exit("Incorrect Shader Param Size.\nPlease contact Developer.");
               if(sp._gpu_data_size<SIZE(Vec4))Exit("Shader Param Size < SIZE(Vec4)"); // some functions assume that '_gpu_data_size' is at least as big as 'Vec4' to set values without checking for size

               // alloc data
               AllocZero(sp._data, sp._gpu_data_size);
              *Alloc    (sp._changed)=true;
                         sp._constant_count=sp.fullConstantCount();

               // set default value
               Vec4 temp[1024]; if(OK(effect->GetValue(par, temp, SIZE(temp))))sp.set(Ptr(temp), sp._cpu_data_size);
            }
         }
      }
      images.sort(Compare); // once we have all images for this file, sort them, so we can use binary search later while saving techniques when looking for image indexes

      // build list of techniques
      IDirect3DDevice9 *d3d=GetD3D9();
      FREP(desc.Techniques)
      {
         D3DXHANDLE         tech_handle=effect->GetTechnique    (i);
         D3DXTECHNIQUE_DESC tech_desc;  effect->GetTechniqueDesc(                tech_handle    , &tech_desc); if(tech_desc.Passes!=1)Exit("Technique pass count!=1");
         D3DXPASS_DESC      pass_desc;  effect->GetPassDesc     (effect->GetPass(tech_handle, 0), &pass_desc);
         Mems<Byte>         vs_data, ps_data;

         Shader9 &tech=techs.New(); tech.name=tech_desc.Name;

         // get shader data
         {
            SyncLocker lock(D._lock);
            UInt size;
            IDirect3DVertexShader9 *vs=null; if(!OK(d3d->CreateVertexShader(pass_desc.pVertexShaderFunction, &vs)))Exit(S+"Can't create Vertex Shader from Shader file \""+src+"\" to file \""+dest+"\"");
            IDirect3DPixelShader9  *ps=null; if(!OK(d3d->CreatePixelShader (pass_desc. pPixelShaderFunction, &ps)))Exit(S+ "Can't create Pixel Shader from Shader file \""+src+"\" to file \""+dest+"\"");
            vs->GetFunction(null, &size); vs->GetFunction(vs_data.setNum(size).data(), &size); RELEASE(vs);
            ps->GetFunction(null, &size); ps->GetFunction(ps_data.setNum(size).data(), &size); RELEASE(ps);
         }

         // store shaders
         if(vs_data.elms()){FREPA(vs)if(vs[i].data.elms()==vs_data.elms() && EqualMem(vs[i].data.data(), vs_data.data(), vs_data.elms())){tech.vs_index=i; break;} if(tech.vs_index<0){tech.vs_index=vs.elms(); Swap(vs.New().data, vs_data);}}
         if(ps_data.elms()){FREPA(ps)if(ps[i].data.elms()==ps_data.elms() && EqualMem(ps[i].data.data(), ps_data.data(), ps_data.elms())){tech.ps_index=i; break;} if(tech.ps_index<0){tech.ps_index=ps.elms(); Swap(ps.New().data, ps_data);}}

         FREPD(shader, 2) // vs + ps
         {
            LPD3DXCONSTANTTABLE ct=null; D3DXGetShaderConstantTable(shader ? pass_desc.pPixelShaderFunction : pass_desc.pVertexShaderFunction, &ct);
            if(ct)
            {
               Mems<Shader9::Constant> &constants=(shader ? tech.ps_constants : tech.vs_constants);
               D3DXCONSTANTTABLE_DESC desc; ct->GetDesc(&desc);
               FREP(desc.Constants)
               {
                  D3DXHANDLE        constant=ct->GetConstant(null, i);
                  D3DXCONSTANT_DESC const_desc[16];
                  UINT              count=Elms(const_desc);
                  ct->GetConstantDesc(constant, const_desc, &count); if(count!=1)Exit("count!=1");

                  switch(const_desc[0].RegisterSet)
                  {
                     case D3DXRS_SAMPLER:
                     {
                        Int index=(shader ? const_desc[0].RegisterIndex : D3DVERTEXTEXTURESAMPLER0+const_desc[0].RegisterIndex);
                        if(!InRange(index, MAX_DX9_TEXTURES))Exit(S+"Texture index: "+index+", is too big");
                        tech.textures.New().set(index, *ShaderImages(Str8Temp(const_desc[0].Name)));
                     }break;

                     case D3DXRS_FLOAT4:
                     {
                        if(ShaderParam *sp=params.find(Str8Temp(const_desc[0].Name)))
                        {
                           Int gpu_data_offset=0;
                           AddRegisters(constants, *sp, ct, constant, const_desc[0], gpu_data_offset);
                        }else
                        {
                           Exit(S+"Technique \""+tech_desc.Name+"\" uses \""+const_desc[0].Name+"\" parameter which was not found in the parameter list.");
                        }
                     }break;

                     default: Exit(S+"Shader Param \""+const_desc[0].Name+"\"is of unsupported type.\nPlease change to float based."); break;
                  }
               }
               RELEASE(ct);

               // join registers
               constants.sort(Compare);
               REPA(constants)if(i)
               {
                  Shader9::Constant &p=constants[i-1],
                                    &n=constants[i  ];
                  if(p.start+p.count==n.start && (Vec4*)p.data+p.count==n.data && p.changed==n.changed) {p.count+=n.count; constants.remove(i, true);}
               }
            }
         }
      }

      {SyncLocker lock(D._lock); RELEASE(effect);}

      techs.sort(Compare);

      return ShaderSave(dest, params, images, vs, ps, techs);
   }
#endif
   return false;
}
/******************************************************************************/
static Bool ShaderCompile11(C Str &src, C Str &dest, C MemPtr<ShaderMacro> &macros, Str *messages)
{
#if DX11
   if(messages)messages->clear();

   File f; if(!f.readTry(src)){if(messages)messages->line()+="Failed to open file."; return false;}
   Mems<Byte> data; data.setNum(f.size()); if(!f.get(data.data(), f.size())){if(messages)messages->line()+="Failed to read from file."; return false;} f.del(); // release the file handle after reading

   ID3D10Blob *buffer=null, *error=null;
   Mems<D3D_SHADER_MACRO> d3d_macros; d3d_macros.setNum(macros.elms()+1); FREPA(macros){D3D_SHADER_MACRO &m=d3d_macros[i]; m.Name=macros[i].name; m.Definition=macros[i].definition;} Zero(d3d_macros.last());
   D3DCompile(data.data(), data.elms(), null, d3d_macros.data(), &Include11(src), null, "fx_5_0", FLAGS_DX11, 0, &buffer, &error); Error(error, messages);

   ID3DX11Effect *effect=null;
   if(buffer)
   {
    //SyncLocker lock(D._lock); lock not needed for DX11 'D3D'
      D3DX11CreateEffectFromMemory(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, D3D, &effect);
   }

   if(effect)
   {
      ShaderFile               shader;
      Memc<ShaderBufferParams> buffers;
      Memc<ShaderImage*>       images;
      Memc<ShaderVS11>         vs;
      Memc<ShaderHS11>         hs;
      Memc<ShaderDS11>         ds;
      Memc<ShaderPS11>         ps;
      Memc<Shader11>           techs;

      D3DX11_EFFECT_DESC desc; effect->GetDesc(&desc);

      // build list of parameters
      FREP(desc.ConstantBuffers)if(ID3DX11EffectConstantBuffer *cb=effect->GetConstantBufferByIndex(i))
      {
         ID3D11Buffer               *buffer=null; cb    ->GetConstantBuffer(&buffer);
         D3D11_BUFFER_DESC           bd         ; buffer->GetDesc(&bd);
         D3DX11_EFFECT_VARIABLE_DESC desc       ; cb    ->GetDesc(&desc);
         RELEASE(buffer);

         // create Constant Buffer
         ShaderBuffers.lock();
         ShaderBuffer       &buf       =*ShaderBuffers(Str8Temp(desc.Name));
         ShaderBufferParams &buf_params=buffers.New(); buf_params.buffer=&buf; buf_params.index=((desc.Flags&D3DX11_EFFECT_VARIABLE_EXPLICIT_BIND_POINT) ? desc.ExplicitBindPoint : -1);
         if(!buf.is()) // not yet initialized
         {
            buf.create(bd.ByteWidth);
            cb->GetRawValue(buf.data, 0, buf.size()); buf.changed=true; // set default value
            if(buf_params.index>=0){SyncLocker lock(D._lock); buf.bind(buf_params.index);}
         }else
         {
            if(buf.size()!=bd.ByteWidth)Exit(S+"Constant Buffer \""+desc.Name+"\" already exists in constant buffer Map however with different size");
            buf.bindCheck(buf_params.index);
         }
         ShaderBuffers.unlock();

         // set all members
         D3DX11_EFFECT_TYPE_DESC type; cb->GetType()->GetDesc(&type);
         FREP(type.Members)
         {
            ID3DX11EffectVariable      *member=cb->GetMemberByIndex(i);
            D3DX11_EFFECT_VARIABLE_DESC desc; member->GetDesc(&desc);
            D3DX11_EFFECT_TYPE_DESC     type; member->GetType()->GetDesc(&type);
            ShaderParamName            &sp=buf_params.params.New();
            if(sp.is())Exit(S+"Shader parameter \""+desc.Name+"\" listed more than once");else // if wasn't yet created
            {
               sp. name     = desc.Name;
               sp._owns_data= false;
               sp._changed  =&buf .changed;
               sp._data     = buf .data; // not yet offsetted
               sp._elements = type.Elements;

               AddTranslation11(sp, member, desc, type);
               sp._gpu_data_size=type.UnpackedSize;
             //sp._constant_count= unused on DX10+

               if(sp._cpu_data_size!=type.  PackedSize
               || sp._gpu_data_size!=type.UnpackedSize)Exit("Incorrect Shader Param size.\nPlease contact Developer.");
               if(sp._gpu_data_size+sp._full_translation[0].gpu_offset>buf.size())Exit("Shader Param does not fit in Constant Buffer.\nPlease contact Developer.");
             //if(SIZE(Vec4)       +sp._full_translation[0].gpu_offset>buf.size())Exit("Shader Param does not fit in Constant Buffer.\nPlease contact Developer."); some functions assume that '_gpu_data_size' is at least as big as 'Vec4' to set values without checking for size, !! this is not needed and shouldn't be called because in DX10+ Shader Params are stored in Shader Buffers, and 'ShaderBuffer' already allocates padding for Vec4
            }
         }
         RELEASE(cb);
      }
      buffers.sort(Compare); // once we have all buffers for this file, sort them, so we can use binary search later while saving techniques when looking for buffer indexes

      // build list of textures/samplers
      FREP(desc.GlobalVariables)if(ID3DX11EffectVariable *var=effect->GetVariableByIndex(i))
      {
         D3DX11_EFFECT_VARIABLE_DESC desc; var->GetDesc(&desc);
         D3DX11_EFFECT_TYPE_DESC     type; var->GetType()->GetDesc(&type);

         if(type.Type==D3D10_SVT_TEXTURE2D || type.Type==D3D10_SVT_TEXTURE3D || type.Type==D3D10_SVT_TEXTURECUBE || type.Type==D3D10_SVT_TEXTURE2DMS)
            images.add(ShaderImages(Str8Temp(desc.Name)));
         RELEASE(var);
      }
      images.sort(Compare); // once we have all images for this file, sort them, so we can use binary search later while saving techniques when looking for image indexes

      // build list of techniques
      FREP(desc.Techniques)if(ID3DX11EffectTechnique *tech_handle=effect->GetTechniqueByIndex(i))
      {
         D3DX11_TECHNIQUE_DESC tech_desc; tech_handle->GetDesc            (&tech_desc); if(tech_desc.Passes!=1)Exit("Technique pass count!=1"); ID3DX11EffectPass *pass=tech_handle->GetPassByIndex(0);
         D3DX11_PASS_DESC      pass_desc; pass       ->GetDesc            (&pass_desc);
         D3DX11_PASS_SHADER_DESC vs_desc; pass       ->GetVertexShaderDesc(&  vs_desc); D3DX11_EFFECT_SHADER_DESC vsd; Zero(vsd); if(vs_desc.pShaderVariable)vs_desc.pShaderVariable->GetShaderDesc(0, &vsd);
         D3DX11_PASS_SHADER_DESC hs_desc; pass       ->GetHullShaderDesc  (&  hs_desc); D3DX11_EFFECT_SHADER_DESC hsd; Zero(hsd); if(hs_desc.pShaderVariable)hs_desc.pShaderVariable->GetShaderDesc(0, &hsd);
         D3DX11_PASS_SHADER_DESC ds_desc; pass       ->GetDomainShaderDesc(&  ds_desc); D3DX11_EFFECT_SHADER_DESC dsd; Zero(dsd); if(ds_desc.pShaderVariable)ds_desc.pShaderVariable->GetShaderDesc(0, &dsd);
         D3DX11_PASS_SHADER_DESC ps_desc; pass       ->GetPixelShaderDesc (&  ps_desc); D3DX11_EFFECT_SHADER_DESC psd; Zero(psd); if(ps_desc.pShaderVariable)ps_desc.pShaderVariable->GetShaderDesc(0, &psd);

         Shader11 &tech=techs.New(); tech.name=tech_desc.Name;

         // get shader data
         Mems<Byte> vs_data; vs_data.setNum(vsd.BytecodeLength).copyFrom(vsd.pBytecode);
         Mems<Byte> hs_data; hs_data.setNum(hsd.BytecodeLength).copyFrom(hsd.pBytecode);
         Mems<Byte> ds_data; ds_data.setNum(dsd.BytecodeLength).copyFrom(dsd.pBytecode);
         Mems<Byte> ps_data; ps_data.setNum(psd.BytecodeLength).copyFrom(psd.pBytecode);

#if DEBUG && 0 // use this for generation of a basic Vertex Shader which can be used for Input Layout creation (see 'DX10_INPUT_LAYOUT' and 'VS_Code')
   Str t=S+"static Byte VS_Code["+vs_data.elms()+"]={";
   FREPA(vs_data){if(i)t+=','; t+=vs_data[i];}
   t+="};\n";
   ClipSet(t);
   Exit(t);
#endif

         // store shaders
         if(vs_data.elms()){FREPA(vs)if(vs[i].data.elms()==vs_data.elms() && EqualMem(vs[i].data.data(), vs_data.data(), vs_data.elms())){tech.vs_index=i; break;} if(tech.vs_index<0){tech.vs_index=vs.elms(); Swap(vs.New().data, vs_data);}}
         if(hs_data.elms()){FREPA(hs)if(hs[i].data.elms()==hs_data.elms() && EqualMem(hs[i].data.data(), hs_data.data(), hs_data.elms())){tech.hs_index=i; break;} if(tech.hs_index<0){tech.hs_index=hs.elms(); Swap(hs.New().data, hs_data);}}
         if(ds_data.elms()){FREPA(ds)if(ds[i].data.elms()==ds_data.elms() && EqualMem(ds[i].data.data(), ds_data.data(), ds_data.elms())){tech.ds_index=i; break;} if(tech.ds_index<0){tech.ds_index=ds.elms(); Swap(ds.New().data, ds_data);}}
         if(ps_data.elms()){FREPA(ps)if(ps[i].data.elms()==ps_data.elms() && EqualMem(ps[i].data.data(), ps_data.data(), ps_data.elms())){tech.ps_index=i; break;} if(tech.ps_index<0){tech.ps_index=ps.elms(); Swap(ps.New().data, ps_data);}}

         FREPD(shader, 4) // vs, hs, ds, ps
         {
            Mems<Shader11::Buffer >   & buffers=((shader==0) ? tech.vs_buffers  : (shader==1) ? tech.hs_buffers  : (shader==2) ? tech.ds_buffers  : tech.ps_buffers );
            Mems<Shader11::Texture>   &textures=((shader==0) ? tech.vs_textures : (shader==1) ? tech.hs_textures : (shader==2) ? tech.ds_textures : tech.ps_textures);
            D3DX11_EFFECT_SHADER_DESC &sd      =((shader==0) ?      vsd         : (shader==1) ?      hsd         : (shader==2) ?      dsd         :      psd        );
            if(sd.pBytecode)
            {
               ID3D11ShaderReflection *reflection=null; D3DReflect(sd.pBytecode, sd.BytecodeLength, IID_ID3D11ShaderReflection, (Ptr*)&reflection); if(!reflection)Exit("Failed to get reflection");
               D3D11_SHADER_DESC desc; reflection->GetDesc(&desc);

               FREP(desc.BoundResources)
               {
                  D3D11_SHADER_INPUT_BIND_DESC desc; reflection->GetResourceBindingDesc(i, &desc);
                  switch(desc.Type)
                  {
                     case D3D10_SIT_CBUFFER: if(!InRange(desc.BindPoint, MAX_SHADER_BUFFERS))Exit(S+"Constant Buffer index: "+desc.BindPoint+", is too big");  buffers.New().set(desc.BindPoint, *ShaderBuffers(Str8Temp(desc.Name))); break;
                     case D3D10_SIT_TEXTURE: if(!InRange(desc.BindPoint, MAX_TEXTURES      ))Exit(S+"Texture index: "        +desc.BindPoint+", is too big"); textures.New().set(desc.BindPoint, *ShaderImages (Str8Temp(desc.Name))); break;
                  }
               }

               RELEASE(reflection);
            }
         }
         RELEASE(tech_handle);
      }

      {SyncLocker lock(D._lock); RELEASE(effect);}

      techs.sort(Compare);

      return ShaderSave(dest, buffers, images, vs, hs, ds, ps, techs);
   }
#endif
   return false;
}
/******************************************************************************/
#if GL && !GL_ES && WINDOWS
static Str8 RemoveR       (CChar8 *text) {return Replace(text, '\r', '\0');}
static Str8 RemoveComments(CChar8 *text)
{
   Str8 s; s.reserve(Length(text));
   if(text)for(Int in_comment=0; ; ) // 0=no, 1=in line "//", 2=in big "/* */"
   {
      Char8 c=*text++; if(!c)break;
      switch(in_comment)
      {
         default:
         {
            if(c=='/')
            {
               if(text[0]=='/'){in_comment=1; text++; break;} // don't process '/' again
               if(text[0]=='*'){in_comment=2; text++; break;} // don't process '*' again, important to treat /*/ not as closure
            }
            s+=c;
         }break;
         case 1: if(c=='\n'               ){in_comment=0; s.line();} break;
         case 2: if(c=='*' && text[0]=='/'){in_comment=0;   text++;} break; // don't process '*' again, important to treat /*..*/* not as new comment
      }
   }
   return s;
}
static Str8 GetToken(CChar8* &text)
{
   Str8 s;
   if(text)for(;;)
   {
      Char8 c=*text;
      if(CharType(c)==CHART_CHAR){s+=c; text++;}else // append name
      if(!s.is() && c>32){text++; return c;}else     // return operator
      if( s.is() || c>32 || c==0)break;else text++;  // continue
   }
   return s;
}
static Str8 RemoveUnusedStructs(CChar8 *text)
{
   Str8 s; s.reserve(Length(text));
   if(text)for(;;)
   {
      if(Starts(text, "struct", true, true))
      {
         CChar8 *token=text+6; // Length("struct") -> 6
         Str8    name =GetToken(token);
         if(name.is() && !Contains(token, name, true, true))
         {
            for(Int level=0; ; )
            {
               Char8 c=*token++; if(!c)break;
               if(c=='{')   ++level;
               if(c=='}')if(--level<=0)
               {
                  if(GetToken(token)==';'){text=token; goto next;}
                  break;
               }
            }
         }
      }
      {
         Char8 c=*text++; if(!c)break;
         s+=c;
      }
   next:;
   }
   return s;
}
static Str8 RemoveSpaces(CChar8 *text)
{
   Str8 s; s.reserve(Length(text));
   Bool possible_preproc=true, preproc=false;
   if(text && text[0])for(Char8 last=0;;)
   {
      if(text[0]==' '
      && (CharType(last)!=CHART_CHAR || CharType(text[1])!=CHART_CHAR)
      && (preproc ? last!=')' && text[1]!='(' : true) // don't remove spaces around brackets when in preprocessor mode "#define X(a) a" would turn into "#define X(a)a", same thing for "#define X (X+1)" would turn into "#define X(X+1)"
      && (last!='/' || text[1]!='*')  // don't remove spaces around / * because this could trigger comment mode
      && (last!='*' || text[1]!='/')  // don't remove spaces around * / because this could trigger comment mode
      )
      {
         text+=1;
         last =s.last();
      }else
      {
         last=*text++; if(!last)break;
         s+=last;
         if(last=='\n'){possible_preproc=true; preproc=false;}else
         if(last!=' ' && last!='\t' && last!='#')possible_preproc=false;else
         if(last=='#' && possible_preproc)preproc=true;
      }
   }
   return s;
}
static Str8 RemoveEmptyLines(CChar8 *text)
{
   Str8 s; s.reserve(Length(text));
   if(text)for(Char8 last='\n'; ; )
   {
      Char8 c=*text++; if(!c)break;
      if(c=='\n' && (last=='\n' || !*text))continue;
      s+=c; last=c;
   }
   return s;
}
static Str8 Clean(C Str8 &text)
{
   return RemoveEmptyLines(RemoveSpaces(RemoveUnusedStructs(RemoveComments(RemoveR(text)))));
}
/******************************************************************************/
static struct Varyings
{
   CChar8 *from, *to;
}varyings[]=
{
   // vertex shader
   {"gl_FrontColor"         , "GL_Col" },
   {"gl_FrontSecondaryColor", "GL_Col1"},

   // pixel shader
   {"gl_Color"         , "GL_Col" },
   {"gl_SecondaryColor", "GL_Col1"},

   // both
   {"gl_TexCoord[0]", "GL_Tex0"},
   {"gl_TexCoord[1]", "GL_Tex1"},
   {"gl_TexCoord[2]", "GL_Tex2"},
   {"gl_TexCoord[3]", "GL_Tex3"},
   {"gl_TexCoord[4]", "GL_Tex4"},
   {"gl_TexCoord[5]", "GL_Tex5"},
   {"gl_TexCoord[6]", "GL_Tex6"},
   {"gl_TexCoord[7]", "GL_Tex7"},
   {"gl_TexCoord[8]", "GL_Tex8"},
};
static Str8 CleanCGShader(CChar8 *shader)
{
   Int      length=Length(shader);
   Str8     cleaned; cleaned.reserve(length);
   FileText f; f.readMem(shader, length);
   for(Str8 line; !f.end(); )
   {
      f.getLine(line);
      if(line.is() && !Starts(line, "//") && !Starts(line, "#extension GL_NV_fragdepth") && line!="attribute ivec4 ATTR15;")
      {
         cleaned+=line;
         cleaned+='\n';
      }
   }

   Char8 temp[256], dest[256];

   // rename CG style samplers, example "//var sampler2DSHADOW ShdMap :  : _TMP214 : -1 : 1"
   for(CChar8 *t=shader; t=TextPos(t, "//var sampler2DSHADOW ", true, true); )
   {
      t+=22;
      if(CChar8 *cg=TextPos(t, " :  : "))
      {
         Int name_length=cg-t+1; if(name_length>=Elms(dest))Exit("name too long");
         Set(dest, t, name_length);
         if(t=_SkipWhiteChars(cg+6))
         {
            Int i=0; for(;;)
            {
               Char8 c=*t++;
               if(i>=Elms(temp)-1)Exit(S+"Uniform name too long"); // leave room for '\0'
               if(!c || c=='[' || c==',' || c==':' || c==';' || c=='=' || WhiteChar(c))break;
               temp[i++]=c;
            }
            if(i)
            {
               temp[i]='\0';
               cleaned=Replace(cleaned, temp, dest, true, true);
            }
         }
      }
   }
   // rename CG style uniforms "_name" to "name"
   for(CChar8 *t=shader; t=TextPos(t, "uniform ", true, true); )
      if(t=_SkipWhiteChars(TextPos(_SkipWhiteChars(t+8), ' ')))
   {
      Int i=0; for(;;)
      {
         Char8 c=*t++;
         if(i>=Elms(temp)-1)Exit(S+"Uniform name too long"); // leave room for '\0'
         if(!c || c=='[' || c==',' || c==':' || c==';' || c=='=' || WhiteChar(c))break;
         temp[i++]=c;
      }
      temp[i]='\0';
      if(temp[0]!='_')Exit(S+"Uniform \""+temp+"\" doesn't start with '_'");
      cleaned=Replace(cleaned, temp, temp+1, true, true);
   }

   // user clip plane, CG doesn't support 'gl_ClipDistance', so instead 'gl_BackSecondaryColor' is used, after CG compiles successfully, 'gl_BackSecondaryColor' is replaced with 'gl_ClipDistance'
   cleaned=Replace(cleaned, "gl_BackSecondaryColor.x", "gl_ClipDistance[0]", true, true); // replace dummy "gl_BackSecondaryColor" with "gl_ClipDistance[0]"

   // vtx.instance() is hardcoded as "uint _instance:ATTR15", because CG doesn't support "gl_InstanceID" or "SV_InstanceID" semantics
   cleaned=Replace(cleaned, "int(ATTR15.x)", "gl_InstanceID", true);

   // replace built-in varyings, and keep only those that are used
   REPA(varyings)if(Contains(cleaned, varyings[i].from))cleaned=S+"varying vec4 "+varyings[i].to+";\n"+Replace(cleaned, varyings[i].from, varyings[i].to); // "varying vec4 GL_Col;"

   return Clean(cleaned);
}
static Str StrInclude(FileText &f, C Str &parent)
{
   Str out;
   for(; !f.end(); )
   {
      C Str &line=f.fullLine();
      if(Starts(line, "#include"))
      {
         Str file=StrInside(line, '"', '"', true, true);
         if(!FullPath(file))file=GetPath(parent).tailSlash(true)+file; file=NormalizePath(file);
         FileText f; if(!f.read(file))Exit(S+"Error opening file: \""+file+"\"");
         out+=StrInclude(f, file);
      }else
      {
         out+=line;
      }
      out+='\n';
   }
   return out;
}
static Str StrInclude(C Str &str, C Str &parent)
{
   return StrInclude(FileText().writeMem().putText(str).rewind(), parent);
}
static struct FromTo
{
   CChar8 *from, *to;
}ee_glsl[]=
{
   {"inline ", ""     },
   {"in out" , "inout"},

   {"Bool" , "bool" },
   {"Int"  , "int"  },
   {"UInt" , "uint" },
   {"Flt"  , "float"},
   {"VecI2", "ivec2"},
   {"VecI" , "ivec3"},
   {"VecI4", "ivec4"},
   {"Vec2" , "vec2" },
   {"Vec"  , "vec3" },
   {"Vec4" , "vec4" },

   {"Dot"      , "dot"},
   {"Cross"    , "cross"},
   {"Sign"     , "sign"},
   {"Abs"      , "abs"},
   {"Min"      , "min"},
   {"Max"      , "max"},
   {"Mid"      , "clamp"},
   {"Frac"     , "fract"},
   {"Round"    , "round"},
   {"Trunc"    , "trunc"},
   {"Floor"    , "floor"},
   {"Ceil"     , "ceil"},
   {"Sqrt"     , "sqrt"},
   {"Normalize", "normalize"},
   {"Pow"      , "pow"},
   {"Sin"      , "sin"},
   {"Cos"      , "cos"},
   {"Lerp"     , "mix"},
   {"Length"   , "length"},

   {"Image"    , "sampler2D"},
   {"Image3D"  , "sampler3D"},
   {"ImageCube", "samplerCube"},

   {"Tex"    , "texture2D"},
   {"Tex3D"  , "texture3D"},
   {"TexCube", "textureCube"},

   {"O_vtx", "gl_Position"},
};

/*
 "= MP Vec(..)" code is not supported on Mali (Samsung Galaxy S2)

 LP, MP, HP always point to lowp, mediump, highp

https://github.com/mattdesl/lwjgl-basics/wiki/GLSL-Versions

GL  GLSL
2.0 110
2.1 120
3.0 130
3.1 140
3.2 150
3.3 330
4.0 400

2.0ES 100 es
3.0ES 300 es

*/

static Str8 GLSLVSShader(Str8 code)
{
   Bool texture2DLod=Contains(code, "texture2DLod", true, true),
        texture3DLod=Contains(code, "texture3DLod", true, true);
   return S
      +"#ifdef GL_ES\n" // GLSL may not support "#if GL_ES" if GL_ES is not defined
      +   "#define LP lowp\n"
      +   "#define MP mediump\n"
      +   "#define HP highp\n"
      +   "precision HP float;\n"
      +   "precision HP int;\n"
      +"#else\n"
      +   "#define LP\n"
      +   "#define MP\n"
      +   "#define HP\n"
      +"#endif\n"
      +"#if __VERSION__>=300\n" // this includes desktop GL3.3(330) and GLES3(300)
      +   (texture2DLod ? "#define texture2DLod textureLod\n" : "") // 'texture2DLod' is actually 'textureLod' on GL3
      +   (texture3DLod ? "#define texture3DLod textureLod\n" : "") // 'texture3DLod' is actually 'textureLod' on GL3
      +   "#define attribute in\n"
      +   "#define varying out\n"
      +"#else\n"
      +   "#define gl_InstanceID 0\n" // instancing not supported on <GL3
      +"#endif\n"
      +code;
}
static Str8 GLSLPSShader(Str8 code, Bool force_hp)
{
   // replace "shadow2DProj(..).x" with "shadow2DProj(..)"
   for(Int offset=0; ; )
   {
   again:
      Int i=TextPosI(code()+offset, "shadow2DProj(", true, true); if(i<0)break; offset+=i+12; // 12=Length("shadow2DProj")
      for(Int level=0; offset<code.length(); offset++)switch(code[offset])
      {
         case '(':     ++level; break;
         case ')': if(!--level)
         {
            offset++;
            if(code[offset]=='.')code.remove(offset, 2); // remove ".x"
            goto again;
         }break;
      }
   }

   Bool dd=(Contains(code, "dFdx", true, true) // detect if uses 'ddx/ddy' functions
         || Contains(code, "dFdy", true, true)),
       mrt= Contains(code, "#extension GL_ARB_draw_buffers:require", true, true), // detect if uses MRT
       rt0=(Contains(code, "gl_FragData[0]", true, true) || Contains(code, "gl_FragColor", true, true)),
       rt1= Contains(code, "gl_FragData[1]", true, true),
       rt2= Contains(code, "gl_FragData[2]", true, true),
       rt3= Contains(code, "gl_FragData[3]", true, true),
       sampler2DShadow=Contains(code, "sampler2DShadow", true, true),
       sampler3D      =Contains(code, "sampler3D"      , true, true),       texture2D      =Contains(code, "texture2D"      , true, true),
       texture2DLod   =Contains(code, "texture2DLod"   , true, true),
       texture3D      =Contains(code, "texture3D"      , true, true),
       texture3DLod   =Contains(code, "texture3DLod"   , true, true),
       textureCube    =Contains(code, "textureCube"    , true, true),
       shadow2D       =Contains(code, "shadow2D"       , true, true),
       shadow2DProj   =Contains(code, "shadow2DProj"   , true, true);

   if(mrt)code=Replace(code, "#extension GL_ARB_draw_buffers:require\n", S, true, true); // CG may generate this after some other commands, however compiling on Radeon will fail, because it needs to be at start "syntax error: #extension must always be before any non-preprocessor tokens", so remove it and place at the start manually

   if(rt0)code=Replace(Replace(code, "gl_FragData[0]", "RT0", true, true), "gl_FragColor", "RT0", true, true);
   if(rt1)code=        Replace(code, "gl_FragData[1]", "RT1", true, true);
   if(rt2)code=        Replace(code, "gl_FragData[2]", "RT2", true, true);
   if(rt3)code=        Replace(code, "gl_FragData[3]", "RT3", true, true);

   return S // extensions must be listed before any non-preprocessor codes or compilation will fail on Samsung Galaxy S3 with error "Extension directive must occur before any non-preprocessor tokens"
      +      "#extension GL_EXT_shader_texture_lod:enable\n"         // without this, pixel/fragment shaders using TexLod  will not work on Mobile GLES2
      +      "#extension GL_EXT_shadow_samplers:enable\n"            // without this, pixel/fragment shaders using shadows will not work on Mobile GLES2
      +(dd ? "#extension GL_OES_standard_derivatives:enable\n" : "") // without this, pixel/fragment shaders using ddx/ddy will not work on Mobile GLES2
      // set things after extensions
      +"#ifdef GL_ES\n" // GLSL may not support "#if GL_ES" if GL_ES is not defined
      +   "#define LP lowp\n"
      +   "#define MP mediump\n"
      +   "#define HP highp\n"
      +   (force_hp        ? "precision HP float;\n"           : "precision MP float;\n")
      +   (force_hp        ? "precision HP int;\n"             : "precision MP int;\n")
      +   (force_hp        ? "precision HP sampler2D;\n"       : "") // may be needed for depth textures
      +   (sampler2DShadow ? "precision LP sampler2DShadow;\n" : "")
      +   (sampler3D       ? "precision LP sampler3D;\n"       : "")
      +   "#if __VERSION__<300\n"
      +      (texture2DLod ? "#define texture2DLod texture2DLodEXT\n" : "") // 'texture2DLod' is actually 'texture2DLodEXT' on GLES2
      +      (shadow2DProj ? "#define shadow2DProj shadow2DProjEXT\n" : "") // 'shadow2DProj' is actually 'shadow2DProjEXT' on GLES2
      +   "#endif\n"
      +"#else\n"
      +   "#define LP\n"
      +   "#define MP\n"
      +   "#define HP\n"
      +"#endif\n"
      +"#if __VERSION__>=300\n" // this includes desktop GL3.3(330) and GLES3(300)
      +   (texture2D    ? "#define texture2D texture\n"        : "") // 'texture2D'    is actually 'texture'     on GL3
      +   (texture2DLod ? "#define texture2DLod textureLod\n"  : "") // 'texture2DLod' is actually 'textureLod'  on GL3
      +   (texture3D    ? "#define texture3D texture\n"        : "") // 'texture3D'    is actually 'texture'     on GL3
      +   (texture3DLod ? "#define texture3DLod textureLod\n"  : "") // 'texture3DLod' is actually 'textureLod'  on GL3
      +   (textureCube  ? "#define textureCube texture\n"      : "") // 'textureCube'  is actually 'texture'     on GL3
      +   (shadow2D     ? "#define shadow2D texture\n"         : "") // 'shadow2D'     is actually 'texture'     on GL3
      +   (shadow2DProj ? "#define shadow2DProj textureProj\n" : "") // 'shadow2DProj' is actually 'textureProj' on GL3
      +   (rt0 ? "layout(location=0) out HP vec4 RT0;\n" : "")
      +   (rt1 ? "layout(location=1) out HP vec4 RT1;\n" : "")
      +   (rt2 ? "layout(location=2) out HP vec4 RT2;\n" : "")
      +   (rt3 ? "layout(location=3) out HP vec4 RT3;\n" : "")
      +   "#define varying in\n"
      +"#else\n"
      +   (rt0 ? "#define RT0 gl_FragData[0]\n" : "")
      +   (rt1 ? "#define RT1 gl_FragData[1]\n" : "")
      +   (rt2 ? "#define RT2 gl_FragData[2]\n" : "")
      +   (rt3 ? "#define RT3 gl_FragData[3]\n" : "")
      +   "#define gl_InstanceID 0\n" // instancing not supported on <GL3
      +"#endif\n"
      +code;
}
static Str8 GetVarName(C Str8 &text, Int offset)
{
   Str8 name;
   for(Int i=offset; i<text.length(); i++)
   {
      Char8 c=text[i]; if(!c || c==',' || c==';' || c=='[' || c==' ' || c=='}')break; // "variable;", "var1, var2;", "var[100];", ..
      name+=c;
   }
   return name;
}
static void SpecifyCGPrecisionModifiers(Str8 &vs_code, Str8 &ps_code, Bool vs_hp, Bool ps_hp) // OpenGL ES will fail if vertex and pixel shaders both use the same uniforms but with different precision modifiers, that's why we need to make sure that if same uniform or class is used between shaders then they have the same precision specified
{
   Bool max_hp=(vs_hp || ps_hp); // maximum precision of shaders
   if(vs_hp!=ps_hp) // shaders operate on different precisions
      FREPD(s, 2) // shader (0=vertex, 1=pixel)
   {
      Str8 &code =(s ? ps_code : vs_code), // current code
           &code2=(s ? vs_code : ps_code); // other   code
      Bool  hp   =(s ? ps_hp   : vs_hp  ), // current precision
            hp2  =(s ? vs_hp   : ps_hp  ); // other   precision
      for(Int uniform_index=0; ; uniform_index++)
      {
         Int uniform_pos =TextPosIN(code, "uniform", uniform_index, true, true); if(uniform_pos<0)break; // find all "uniform" occurences (like "uniform float Var", "uniform MaterialClass Material")
         Int uniform_type=uniform_pos+8; // Length("uniform ")->8
         if(!Starts(code()+uniform_type, "HP"       , true, true)
         && !Starts(code()+uniform_type, "MP"       , true, true)
         && !Starts(code()+uniform_type, "LP"       , true, true)  // if precision was not yet specified
         && !Starts(code()+uniform_type, "sampler1D", true, true)
         && !Starts(code()+uniform_type, "sampler2D", true, true)
         && !Starts(code()+uniform_type, "sampler3D", true, true)) // not a sampler
         {
            Int  uniform_name=TextPosI  (code()+uniform_type, ' '); if(uniform_name<0)continue; uniform_name+=1+uniform_type; // space + type offset
            Str8     var_name=GetVarName(code, uniform_name);
            if(Starts(code()+uniform_type, "float", true, true)
            || Starts(code()+uniform_type, "vec2" , true, true)
            || Starts(code()+uniform_type, "vec3" , true, true)
            || Starts(code()+uniform_type, "vec4" , true, true)) // basic type
            {
               Str8 var_def; for(Int i=uniform_pos; i<uniform_name+var_name.length(); i++)var_def+=code[i];
               Int  code2_var_def_pos=TextPosI(code2, var_def, true, true); // find "uniform type name" string in other shader
               if(  code2_var_def_pos>=0) // if other shader also uses this variable
               {
                  code .insert(uniform_type                                , max_hp ? "HP " : "MP "); // add precision modifier "uniform precision type name"
                  code2.insert(code2_var_def_pos+(uniform_type-uniform_pos), max_hp ? "HP " : "MP "); // add precision modifier "uniform precision type name"
               }
            }else
            {
               Str8 type_name=GetVarName(code, uniform_type), // struct, for example "MaterialClass"
                    type_def =S8+"struct "+type_name;
               Int  code2_struct_pos=TextPosI(code2, type_def, true, true); // find "struct MaterialClass" in other shader
               if(  code2_struct_pos>=0) // if other shader also uses this class
               {
                  Int  code2_struct_end=TextPosI(code2()+code2_struct_pos, '}'); if(code2_struct_end<0)continue; code2_struct_end+=code2_struct_pos;
                  Str8 struct_def; for(Int i=code2_struct_pos; i<=code2_struct_end; i++)struct_def+=code2[i]; // 'struct_def' now contains "struct MaterialClass{..}"
                  Int  code_struct_pos=TextPosI(code, struct_def, true, true); if(code_struct_pos<0)continue; // proceed only if first code has exact same struct definition
                  
                  Bool changed=false;
                  for(Int member_start=0; member_start<struct_def.length(); member_start++)
                     if(Starts(struct_def()+member_start, "\nfloat", true, true)
                     || Starts(struct_def()+member_start, "\nvec2" , true, true)
                     || Starts(struct_def()+member_start, "\nvec3" , true, true)
                     || Starts(struct_def()+member_start, "\nvec4" , true, true)) // basic type (include '\n' character to skip those with specified precision)
                  {
                     Int  member_name=       TextPosI  (struct_def()+member_start, ' '); if(member_name<0)continue; member_name+=1+member_start; // space + start offset
                     Str8    var_name=S8+'.'+GetVarName(struct_def, member_name); // ".member"
                     Bool used =Contains(code , var_name, true, true), // if used in       code
                          used2=Contains(code2, var_name, true, true); // if used in other code
                     Bool var_hp=((used && used2) ? (hp || hp2) // used by both shaders
                                 : used           ?  hp         // used by first
                                 :         used2  ?        hp2  // used by second
                                 : true);                       // used by none
                     struct_def.insert(member_start+1, var_hp ? "HP " : "MP "); // add precision modifier "\nprecision type name"
                     changed=true;
                  }
                  if(changed)
                  {
                     code .remove(code_struct_pos , code2_struct_end-code2_struct_pos+1).insert(code_struct_pos , struct_def);
                     code2.remove(code2_struct_pos, code2_struct_end-code2_struct_pos+1).insert(code2_struct_pos, struct_def);
                  }
               }
            }
         }
      }
   }
}
T1(TYPE) static Int StoreShader(C Str8 &code, Memc<TYPE> &shaders, File &src, File &temp)
{
   src.reset().putStr(code).pos(0);
   if(!Compress(src, temp.reset(), COMPRESS_GL, COMPRESS_GL_LEVEL, COMPRESS_GL_MT))Exit("Can't compress shader");
   REPA(shaders)
   {
      Mems<Byte> &shader_data=shaders[i].data;
      if(shader_data.elms()==temp.size())
      {
         File data; data.readMem(shader_data.data(), shader_data.elms());
         temp.pos(0); if(temp.equal(data))return i;
      }
   }
   Mems<Byte> &data=shaders.New().data;
   temp.pos(0); temp.get(data.setNum(temp.size()).data(), temp.size());
   return shaders.elms()-1;
}
struct ParamMember
{
   Int          gpu_offset;
   Str8         name;
   ShaderParam *sp;

   void set(C Str8 &name, ShaderParam &sp, Int gpu_offset) {T.name=name; T.sp=&sp; T.gpu_offset=gpu_offset;}

   ParamMember() {sp=null; gpu_offset=0;}
};
static C ParamMember* Find(C Memc<ParamMember> &pm, CChar8 *name)
{
   REPA(pm)if(Equal(pm[i].name, name, true))return &pm[i];
   return null;
}
static Bool FindNameReplacement(Char8 (&temp)[1024], CChar8 *glsl_name, CChar8 *code)
{
   Set(temp, " :  : _"); Append(temp, glsl_name);
   if(CChar8 *t=TextPos(code, temp, true, true))for(CChar8 *name=t-1; name>code; name--)if(*name==' ')
   {
      Set(temp, name+1, t-name);
      return true;
   }
   return false;
}

#include <Cg/cg.h>
#include <Cg/cgGL.h>

static void AddTranslationGL(ShaderParam &sp, CGparameter par, Memc<ParamMember> &pm)
{
   CChar8 *pname=cgGetParameterName(par);

   if(cgGetArraySize(par, 0)<=1) // array size
   {
      CGparameterclass pclass=cgGetParameterClass        (par);
      CGtype           type  =cgGetParameterType         (par);
   #if 0
      Int              rsize =cgGetParameterResourceSize (par);
      CChar8          *sem   =cgGetParameterSemantic     (par);
      CGresource       bres  =cgGetParameterBaseResource (par);
      CGresource       res   =cgGetParameterResource     (par);
      CChar8          *rname =cgGetParameterResourceName (par);
      Int              resi  =cgGetParameterResourceIndex(par);
      CGtype           rtype =cgGetParameterResourceType (par);
      CGenum           var   =cgGetParameterVariability  (par);
      Int              bi    =cgGetParameterBufferIndex  (par);
      Int              bo    =cgGetParameterBufferOffset (par);
      Int              pi    =cgGetParameterIndex        (par);
   #endif

      if(pclass==CG_PARAMETERCLASS_SCALAR || pclass==CG_PARAMETERCLASS_VECTOR)
      {
         pm.New().set(pname, sp, sp._gpu_data_size);

         if(type==CG_FLOAT  || type==CG_HALF  || type==CG_FIXED ){sp._full_translation.New().set(sp._cpu_data_size, sp._gpu_data_size, SIZE(Flt )); sp._cpu_data_size+=SIZE(Flt ); sp._gpu_data_size+=SIZE(Flt );}else
         if(type==CG_FLOAT2 || type==CG_HALF2 || type==CG_FIXED2){sp._full_translation.New().set(sp._cpu_data_size, sp._gpu_data_size, SIZE(Vec2)); sp._cpu_data_size+=SIZE(Vec2); sp._gpu_data_size+=SIZE(Vec2);}else
         if(type==CG_FLOAT3 || type==CG_HALF3 || type==CG_FIXED3){sp._full_translation.New().set(sp._cpu_data_size, sp._gpu_data_size, SIZE(Vec )); sp._cpu_data_size+=SIZE(Vec ); sp._gpu_data_size+=SIZE(Vec );}else
         if(type==CG_FLOAT4 || type==CG_HALF4 || type==CG_FIXED4){sp._full_translation.New().set(sp._cpu_data_size, sp._gpu_data_size, SIZE(Vec4)); sp._cpu_data_size+=SIZE(Vec4); sp._gpu_data_size+=SIZE(Vec4);}else
            Exit(S+"Unhandled Shader Parameter Type for \""+pname+'"');
      }else
      if(pclass==CG_PARAMETERCLASS_MATRIX)
      {
         pm.New().set(pname, sp, sp._gpu_data_size);

         Int rows   =cgGetParameterRows   (par),
             columns=cgGetParameterColumns(par);

         FREPD(y, columns)
         FREPD(x, rows   )sp._full_translation.New().set(sp._cpu_data_size+SIZE(Flt)*(y+x*columns), sp._gpu_data_size+SIZE(Flt)*(x+y*rows), SIZE(Flt));
         sp._cpu_data_size+=SIZE(Flt)*rows*columns;
         sp._gpu_data_size+=SIZE(Flt)*rows*columns;
      }else
      if(pclass==CG_PARAMETERCLASS_STRUCT)
      {
         for(CGparameter member=cgGetFirstStructParameter(par); member; member=cgGetNextParameter(member))AddTranslationGL(sp, member, pm);
      }
   }else
   {
      FREP(cgGetArraySize(par, 0))
      {
         CGparameter elm=cgGetArrayParameter(par, i);
         AddTranslationGL(sp, elm, pm);
      }
   }
}
struct CGCONTEXT
{
   CGcontext _;

   operator CGcontext() {return _;}

  ~CGCONTEXT() {cgDestroyContext(_);}
   CGCONTEXT()
   {
      if(_=cgCreateContext())
      {
      #ifdef CG_BEHAVIOR_CURRENT
         cgSetContextBehavior(_, CG_BEHAVIOR_CURRENT);
      #endif
       //cgSetAutoCompile(_, CG_COMPILE_IMMEDIATE);
         cgGLRegisterStates(_);
      }
   }
   Str error()
   {
      CGerror error;
      CChar8 *string=cgGetLastErrorString(&error);
      switch(error)
      {
         case CG_NO_ERROR: break;
         //case CG_INVALID_PARAMETER_ERROR:
         //case CG_NON_NUMERIC_PARAMETER_ERROR: break;
      
         //case CG_COMPILER_ERROR: return S+string+'\n'+cgGetLastListing(T); break;

         default:
         {
            Str s=string; s.line()+=cgGetLastListing(T);
            if(string=(CChar8*)glGetString(GL_PROGRAM_ERROR_STRING_ARB))
            {
               int loc; glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &loc);
               s.line()+=string;
            }
            return s;
         }break;
      }
      return S;
   }
};
struct CGEFFECT
{
   CGeffect _;

   operator CGeffect() {return _;}

           ~CGEFFECT() {cgDestroyEffect(_);}
   explicit CGEFFECT(CGeffect fx) {_=fx;}
};
#endif
static Bool ShaderCompileGL(Str name, C Str &dest, C MemPtr<ShaderMacro> &macros, Str *messages, C MemPtr<ShaderGLSL> &stg)
{
#if GL && !GL_ES && WINDOWS
   Memc<Str8   > args_str; FREPA(  macros)args_str.add(S+"-D"+macros[i].name+'='+macros[i].definition);
   Memc<CChar8*> args    ; FREPA(args_str)args    .add(args_str[i]); args.add("-O3"); args.add((CChar8*)null);

   name=NormalizePath(name); if(!FExistSystem(name))name=DataPath()+name;
   Bool                    ok=false;
   Map <Str8, ShaderParam> params(CompareCS);
   Memc<ShaderImage*>      images;
   Memc<ShaderVSGL>        vs;
   Memc<ShaderPSGL>        ps;
   Memc<ShaderGL>          techs;
   CGCONTEXT context; if(context)
   {
      CGEFFECT effect(cgCreateEffectFromFile(context, UnixPathUTF8(name), args.data())); if(effect)
      {
         Memc<ParamMember> pm;

         // add parameters
         for(CGparameter par=cgGetFirstEffectParameter(effect); par; par=cgGetNextParameter(par))
         {
            CChar8          *pname =cgGetParameterName        (par);
            CGparameterclass pclass=cgGetParameterClass       (par);
          //Int              rsize =cgGetParameterResourceSize(par);
            if(pclass==CG_PARAMETERCLASS_SAMPLER)images.add(ShaderImages(Str8Temp(pname)));else
            if(pclass!=CG_PARAMETERCLASS_OBJECT )
            {
               ShaderParam &sp=*params(Str8Temp(pname));
               if(sp.is())Exit(S+"Shader parameter \""+pname+"\" listed more than once");else // if wasn't yet created
               {
                  sp._owns_data=true;
                  sp._elements =cgGetArraySize(par, 0);

                  AddTranslationGL(sp, par, pm);
                  sp.optimize(); // required for setting default value

                  MAX(sp._gpu_data_size, SIZEI(Vec4)); // in OpenGL, parameters must be stored without padding
                //if (sp._gpu_data_size!=rsize    )Exit("Incorrect Shader Param Size.\nPlease contact Developer.");
                  if (sp._gpu_data_size<SIZE(Vec4))Exit("Shader Param Size < SIZE(Vec4)"); // some functions assume that '_gpu_data_size' is at least as big as 'Vec4' to set values without checking for size

                  // alloc data
                  AllocZero(sp._data, sp._gpu_data_size);
                 *Alloc    (sp._changed)=true;
                            sp._constant_count=sp.fullConstantCount();

                  // set default value
                  Flt temp[4*1024]; if(Int elms=cgGetParameterDefaultValuefc(par, Elms(temp), temp))sp.set(Ptr(temp), elms*SIZE(Flt));
               }
            }
         }
         images.sort(Compare); // once we have all images for this file, sort them, so we can use binary search later while saving techniques when looking for image indexes

         Str8 vs_code, ps_code, glsl_params;
         File src, temp; src.writeMem(); temp.writeMem();

         // techniques
         Int tech_num=0; for(CGtechnique technique=cgGetFirstTechnique(effect); technique; technique=cgGetNextTechnique(technique), tech_num++); // get number of all techniques
         Int t       =1; for(CGtechnique technique=cgGetFirstTechnique(effect); technique; technique=cgGetNextTechnique(technique), t++       )
         {
            CChar8   *tech_name=cgGetTechniqueName(technique);
            ShaderGL &tech=techs.New(); tech.name=tech_name;
         #if DEBUG
            LogN(S+"Processing tech \""+tech_name+"\" "+t+'/'+tech_num);
         #endif

            // try to find GLSL replacement
            Bool cg=true;
            REPA(stg)if(Equal(tech.name, stg[i].tech_name, true))
            {
             C ShaderGLSL &st=stg[i];
               Str file; FileText f; if(f.read(name))for(; !f.end(); )file+=f.getLine()+'\n';
                   file  =StrInside(file, S+"@GROUP \""+st.group_name+'"', "@GROUP_END" , true, true); if(!file  .is())Exit(S+"@GROUP \""+st.group_name+"\" @GROUP_END not defined");
               Str shared=StrInside(file,   "@SHARED"                    , "@SHARED_END", true, true); if(!shared.is())Exit(  "@SHARED @SHARED_END not defined"                    ); shared=       StrInclude(shared, name);
               Str vs    =StrInside(file,   "@VS"                        , "@VS_END"    , true, true); if(!vs    .is())Exit(  "@VS @VS_END not defined"                            ); vs    =shared+StrInclude(vs    , name);
               Str ps    =StrInside(file,   "@PS"                        , "@PS_END"    , true, true); if(!ps    .is())Exit(  "@PS @PS_END not defined"                            ); ps    =shared+StrInclude(ps    , name);
               vs=Clean(vs);
               ps=Clean(ps);
               REPA(ee_glsl)
               {
                  vs=Replace(vs, ee_glsl[i].from, ee_glsl[i].to, true, true);
                  ps=Replace(ps, ee_glsl[i].from, ee_glsl[i].to, true, true);
               }
               glsl_params.clear(); FREPA(st.params)glsl_params+=S+"#define "+st.params[i].name+' '+st.params[i].definition+'\n';
               cg=false;
               vs_code=GLSLVSShader(glsl_params+vs);
               ps_code=GLSLPSShader(glsl_params+ps, false); // default hand written GLSL shaders to low precision
               break;
            }

            CChar8 *vs_cg_code=null, *ps_cg_code=null;
            if(cg) // if haven't found GLSL replacement, then use CG version
            {
               CGpass    pass =cgGetFirstPass  (technique);
               CGprogram cg_vs=cgGetPassProgram(pass, CG_VERTEX_DOMAIN  );
               CGprogram cg_ps=cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
               vs_cg_code=cgGetProgramString(cg_vs, CG_COMPILED_PROGRAM); if(!Is(vs_cg_code)){if(messages)messages->line()+=S+"Empty Vertex Shader Code in Technique \""+tech_name+"\""+context.error(); return false;}
               ps_cg_code=cgGetProgramString(cg_ps, CG_COMPILED_PROGRAM); if(!Is(ps_cg_code)){if(messages)messages->line()+=S+"Empty Pixel Shader Code in Technique \"" +tech_name+"\""+context.error(); return false;}

               Bool ps_hp=true; // default all CG shaders to high precision, alternatively use (cgGetNamedTechniqueAnnotation(technique, "ForceHP")!=null;

               vs_code=GLSLVSShader(CleanCGShader(vs_cg_code));
               ps_code=GLSLPSShader(CleanCGShader(ps_cg_code), ps_hp);

               // specify precision modifiers
               SpecifyCGPrecisionModifiers(vs_code, ps_code, true, ps_hp);
            }

         #if DEBUG
            if(SHOW_GLSL_SRC)LogN(S+"// VERTEX SHADER\n\n"+vs_code+"\n\n// PIXEL SHADER\n\n"+ps_code);
         #endif

            // get shader data
            tech.vs_index=StoreShader(vs_code, vs, src, temp);
            tech.ps_index=StoreShader(ps_code, ps, src, temp);

            // get GLSL->ShaderParam names
            Str glsl_messages;
            tech.compile(vs, ps, &glsl_messages); // compilation is required in order to get list of used parameters and their corresponding names and register address, don't clean the shader codes because they'll be needed for saving
            if(!tech.prog)Exit(S+"Error compiling GLSL program:\n"+glsl_messages);
            Int  program_params=0; glGetProgramiv(tech.prog, GL_ACTIVE_UNIFORMS, &program_params);
            FREP(program_params)
            {
               Char8  glsl_name[1024], temp_name[1024]; glsl_name[0]=0;
               Int    size=0;
               GLenum type;
               glGetActiveUniform(tech.prog, i, Elms(glsl_name), null, &size, &type, glsl_name);

               Bool ok=false;
               if(type==GL_SAMPLER_2D || type==GL_SAMPLER_3D || type==GL_SAMPLER_CUBE || type==GL_SAMPLER_2D_SHADOW) // Image
               {
                  REPA(images)if(Equal(ShaderImages.dataInMapToKey(*images[i]), glsl_name, true))
                  {
                   //tech.glsl_images.add(images[i]);
                     ok=true;
                     break;
                  }
               }else // ShaderParam
               {
                C ParamMember *p=Find(pm, glsl_name);
                  if(!p)
                  {
                     if(cg)
                     {
                        if(FindNameReplacement(temp_name, glsl_name, vs_cg_code)
                        || FindNameReplacement(temp_name, glsl_name, ps_cg_code))p=Find(pm, temp_name);
                     }else
                     if(Ends(glsl_name, "[0]"))
                     {
                        Set(temp_name, glsl_name, Length(glsl_name)-2);
                        p=Find(pm, temp_name);
                     }
                  }
                  if(p)
                  {
                     tech.glsl_params.New().set(p->gpu_offset, *p->sp, Equal(glsl_name, ShaderParams.dataInMapToKey(*p->sp), true) ? null : glsl_name); // if 'glsl_name' is exactly the same as ShaderParam name, then skip it so we don't have to save it. in other cases we need to store the 'glsl_name' because it can contain "obj.member" things, which we can't detect
                     ok=true;
                  }
               }
               if(!ok)
               {
               #if DEBUG
                  Log(vs_code);
                  Log(ps_code);
               #endif
                  Exit(S+"GLSL Parameter \""+glsl_name+"\" not found");
               }
            }

            // free
            if(tech.prog){SyncLocker locker(D._lock); glDeleteProgram(tech.prog); tech.prog=0;} // clear while in lock
         }
         ok=true;
      }else if(messages)messages->line()+=S+"Can't create CG effect: "+context.error();
   }else if(messages)messages->line()+="Can't create CG context";
   if(ok)
   {
      // sort any verify
      techs.sort(Compare);
      REPA(stg)if(!techs.binaryHas(stg[i].tech_name, Compare))Exit(S+"Shader doesn't have "+stg[i].tech_name);

      // save
      return ShaderSave(dest, params, images, vs, ps, techs);
   }
#endif
   return false;
}
/******************************************************************************/
Bool ShaderCompileTry(C Str &src, C Str &dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros, C MemPtr<ShaderGLSL> &stg, Str *messages)
{
   if(model==SM_UNKNOWN)return false;

   Memc<ShaderMacro> temp; temp=macros;
   switch(model)
   {
      case SM_UNKNOWN: return false;

      case SM_GL_ES_2:
      case SM_GL_ES_3:
      case SM_GL     : temp.New().set("MODEL", "SM_GL"); break;

      case SM_3      : temp.New().set("MODEL", "SM_3" ); break;
      default        : temp.New().set("MODEL", "SM_4" ); break;
   }
   if(model>=SM_GL_ES_2 && model<=SM_GL)return ShaderCompileGL(src, dest, temp, messages, stg);
   if(                     model==SM_3 )return ShaderCompile9 (src, dest, temp, messages);
   if(                     model>=SM_4 )return ShaderCompile11(src, dest, temp, messages);
   return false;
}
/******************************************************************************/
void ShaderCompile(C Str &src, C Str &dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros, C MemPtr<ShaderGLSL> &stg)
{
   Str messages;
   if(!ShaderCompileTry(src, dest, model, macros, stg, &messages))
   {
   #if !WINDOWS
      if(model==SM_3)Exit("Can't compile DX9 Shaders when not using Windows engine version");
   #endif
   #if !DX11
      if(model>=SM_4)Exit("Can't compile DX10+ Shaders when not using DX10+ engine version");
   #endif
   #if DX9 || DX11
      if(model>=SM_GL_ES_2 && model<=SM_GL)Exit("Can't compile OpenGL Shaders when not using OpenGL engine version");
   #endif
      Exit(S+"Error compiling shader\n\""+src+"\"\nto file\n\""+dest+"\"."+(messages.is() ? S+"\n\nCompilation Messages:\n"+messages : S));
   }
}
Bool ShaderCompileTry(Str src, Str dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros, Str *messages) {return ShaderCompileTry(src, dest, model, macros, null, messages);}
void ShaderCompile   (Str src, Str dest, SHADER_MODEL model, C MemPtr<ShaderMacro> &macros               ) {       ShaderCompile   (src, dest, model, macros, null          );}
/******************************************************************************/
// IO
/******************************************************************************/
#if WINDOWS_OLD
Bool Shader9::save(File &f, C Map<Str8, ShaderParam> &params, C Memc<ShaderImage*> &images)C
{
   // name
   f.putStr(name).putMulti(vs_index, ps_index);

   // textures
   f.cmpUIntV(textures.elms()); FREPA(textures)f<<TextureIndex9(textures[i].index, GetIndex(images, textures[i].image));

   // constants
   f.cmpUIntV(vs_constants.elms());
   FREPA(     vs_constants)
   {
    C Constant &c=vs_constants[i]; f<<ConstantIndex9(c.start, c.count, UIntPtr(c.data)-UIntPtr(c.sp->_data), GetIndex(params, c.sp));
   }
   f.cmpUIntV(ps_constants.elms());
   FREPA(     ps_constants)
   {
    C Constant &c=ps_constants[i]; f<<ConstantIndex9(c.start, c.count, UIntPtr(c.data)-UIntPtr(c.sp->_data), GetIndex(params, c.sp));
   }
   return f.ok();
}
#if DX9
Bool Shader9::load(File &f, C MemtN<ShaderParam*, 256> &params, C MemtN<ShaderImage*, 256> &images)
{
   // name
   f.getStr(name).getMulti(vs_index, ps_index);

   // textures
   textures.setNum(f.decUIntV()); FREPA(textures)
   {
      Texture &t=textures[i]; TextureIndex9 ci; f>>ci; t.index=ci.bind_index; if(!InRange(t.index, MAX_DX9_TEXTURES))Exit(S+"Texture index: "+t.index+", is too big"); t.image=Get(ci.src_index, images);
   }

   // constants
   vs_constants.setNum(f.decUIntV()); FREPA(vs_constants)
   {
      Constant &c=vs_constants[i]; ConstantIndex9 ci; f>>ci; 
      c.sp   =Get(ci.src_index, params); c.changed=c.sp->_changed;
      c.start=ci.start;
      c.count=ci.count;
      c.data =c.sp->_data+ci.offset;
      c.final_count=((c.count<c.sp->fullConstantCount()) ? &c.count : &c.sp->_constant_count); // if this constant is not using full 'ShaderParam' size then keep its own count, otherwise point to the 'ShaderParam' count in case it can get decreased later (for example 'ViewMatrix' after 'SetMatrixCount'), use 'fullConstantCount' instead of '_constant_count' in case it already got modified
   #if CACHE_DX9_CONSTANTS
      DYNAMIC_ASSERT(c.start+c.count<=MAX_DX9_SHADER_CONSTANT/SIZE(Vec4), "Shader Constant out of range");
   #endif
   }
   ps_constants.setNum(f.decUIntV()); FREPA(ps_constants)
   {
      Constant &c=ps_constants[i]; ConstantIndex9 ci; f>>ci; 
      c.sp   =Get(ci.src_index, params); c.changed=c.sp->_changed;
      c.start=ci.start;
      c.count=ci.count;
      c.data =c.sp->_data+ci.offset;
      c.final_count=((c.count<c.sp->fullConstantCount()) ? &c.count : &c.sp->_constant_count); // if this constant is not using full 'ShaderParam' size then keep its own count, otherwise point to the 'ShaderParam' count in case it can get decreased later (for example 'ViewMatrix' after 'SetMatrixCount'), use 'fullConstantCount' instead of '_constant_count' in case it already got modified
   #if CACHE_DX9_CONSTANTS
      DYNAMIC_ASSERT(c.start+c.count<=MAX_DX9_SHADER_CONSTANT/SIZE(Vec4), "Shader Constant out of range");
   #endif
   }

   if(f.ok())return true;
   /*del();*/ return false;
}
#endif
#endif
/******************************************************************************/
#if DX11
static void SaveBuffers(File &f, C Mems<Shader11::Buffer> &constants, C Memc<ShaderBufferParams> &file_buffers, MemtN<UShort, 256> &all)
{
   MemtN<ConstantIndex, 256> save;
   FREPA(constants)
   {
      Int buffer_index=GetIndex(file_buffers, constants[i].buffer); // index of buffer in 'file_buffers' array
    C ShaderBufferParams &buffer=file_buffers[buffer_index];
      if(buffer.index<0) // here we have to save only buffers that don't have a constant bind point index
         save.New().set(constants[i].index, buffer_index); // save to which index this buffer should be bound for this shader, and index of buffer in 'file_buffers' array
      all.binaryInclude(AsUShort(buffer_index), Compare);
   }
   save.saveRaw(f);
}
Bool Shader11::save(File &f, C Memc<ShaderBufferParams> &buffers, C Memc<ShaderImage*> &images)C
{
   // name
   f.putStr(name).putMulti(vs_index, hs_index, ds_index, ps_index);

   // images
   f.cmpUIntV(vs_textures.elms()); FREPA(vs_textures)f<<ConstantIndex(vs_textures[i].index, GetIndex(images, vs_textures[i].image));
   f.cmpUIntV(hs_textures.elms()); FREPA(hs_textures)f<<ConstantIndex(hs_textures[i].index, GetIndex(images, hs_textures[i].image));
   f.cmpUIntV(ds_textures.elms()); FREPA(ds_textures)f<<ConstantIndex(ds_textures[i].index, GetIndex(images, ds_textures[i].image));
   f.cmpUIntV(ps_textures.elms()); FREPA(ps_textures)f<<ConstantIndex(ps_textures[i].index, GetIndex(images, ps_textures[i].image));

   // buffers
   MemtN<UShort, 256> all;
   SaveBuffers(f, vs_buffers, buffers, all);
   SaveBuffers(f, hs_buffers, buffers, all);
   SaveBuffers(f, ds_buffers, buffers, all);
   SaveBuffers(f, ps_buffers, buffers, all);
   all.saveRaw(f);

   return f.ok();
}
#if DEBUG
static void Test(Shader11::Buffer &b)
{
   switch(b.index)
   {
      case SBI_GLOBAL    : if(Name(*b.buffer)!="Global"   )error: Exit(S+"Invalid Shader Constant Index "+b.index+' '+Name(*b.buffer)); break;
      case SBI_OBJ_MATRIX: if(Name(*b.buffer)!="ObjMatrix")goto error; break;
      case SBI_OBJ_VEL   : if(Name(*b.buffer)!="ObjVel"   )goto error; break;
      case SBI_MESH      : if(Name(*b.buffer)!="Mesh"     )goto error; break;
      case SBI_MATERIAL  : if(Name(*b.buffer)!="Material" )goto error; break;
      case SBI_VIEWPORT  : if(Name(*b.buffer)!="Viewport" )goto error; break;
      case SBI_COLOR     : if(Name(*b.buffer)!="Color"    )goto error; break;
   }ASSERT(SBI_NUM==7);
}
#endif
Bool Shader11::load(File &f, C MemtN<ShaderBuffer*, 256> &file_buffers, C MemtN<ShaderImage*, 256> &images)
{
   // name
   f.getStr(name).getMulti(vs_index, hs_index, ds_index, ps_index);

   // textures
   vs_textures.setNum(f.decUIntV()); FREPA(vs_textures){Texture &t=vs_textures[i]; ConstantIndex ci; f>>ci; t.index=ci.bind_index; if(!InRange(t.index, MAX_TEXTURES))Exit(S+"Texture index: "+t.index+", is too big"); t.image=Get(ci.src_index, images);}
   hs_textures.setNum(f.decUIntV()); FREPA(hs_textures){Texture &t=hs_textures[i]; ConstantIndex ci; f>>ci; t.index=ci.bind_index; if(!InRange(t.index, MAX_TEXTURES))Exit(S+"Texture index: "+t.index+", is too big"); t.image=Get(ci.src_index, images);}
   ds_textures.setNum(f.decUIntV()); FREPA(ds_textures){Texture &t=ds_textures[i]; ConstantIndex ci; f>>ci; t.index=ci.bind_index; if(!InRange(t.index, MAX_TEXTURES))Exit(S+"Texture index: "+t.index+", is too big"); t.image=Get(ci.src_index, images);}
   ps_textures.setNum(f.decUIntV()); FREPA(ps_textures){Texture &t=ps_textures[i]; ConstantIndex ci; f>>ci; t.index=ci.bind_index; if(!InRange(t.index, MAX_TEXTURES))Exit(S+"Texture index: "+t.index+", is too big"); t.image=Get(ci.src_index, images);}

   // buffers
   vs_buffers.setNum(f.decUIntV()); FREPA(vs_buffers){Buffer &b=vs_buffers[i]; ConstantIndex ci; f>>ci; b.index=ci.bind_index; if(!InRange(b.index, MAX_SHADER_BUFFERS))Exit(S+"Buffer index: "+b.index+", is too big"); b.buffer=Get(ci.src_index, file_buffers);}
   hs_buffers.setNum(f.decUIntV()); FREPA(hs_buffers){Buffer &b=hs_buffers[i]; ConstantIndex ci; f>>ci; b.index=ci.bind_index; if(!InRange(b.index, MAX_SHADER_BUFFERS))Exit(S+"Buffer index: "+b.index+", is too big"); b.buffer=Get(ci.src_index, file_buffers);}
   ds_buffers.setNum(f.decUIntV()); FREPA(ds_buffers){Buffer &b=ds_buffers[i]; ConstantIndex ci; f>>ci; b.index=ci.bind_index; if(!InRange(b.index, MAX_SHADER_BUFFERS))Exit(S+"Buffer index: "+b.index+", is too big"); b.buffer=Get(ci.src_index, file_buffers);}
   ps_buffers.setNum(f.decUIntV()); FREPA(ps_buffers){Buffer &b=ps_buffers[i]; ConstantIndex ci; f>>ci; b.index=ci.bind_index; if(!InRange(b.index, MAX_SHADER_BUFFERS))Exit(S+"Buffer index: "+b.index+", is too big"); b.buffer=Get(ci.src_index, file_buffers);}
      buffers.setNum(f.decUIntV()); FREPA(   buffers)              buffers[i]=Get(f.getUShort(), file_buffers);

#if DEBUG && 1
   //#pragma message("!! Warning: Use this only for debugging !!")
   REPA(vs_buffers)Test(vs_buffers[i]);
   REPA(hs_buffers)Test(hs_buffers[i]);
   REPA(ds_buffers)Test(ds_buffers[i]);
   REPA(ps_buffers)Test(ps_buffers[i]);
#endif

   if(f.ok())return true;
   /*del();*/ return false;
}
#endif
/******************************************************************************/
#if GL
void ShaderGL::GLSLParam::set(Int gpu_offset, ShaderParam &param, C Str8 &glsl_name) {T.gpu_offset=gpu_offset; T.param=&param; DYNAMIC_ASSERT(T.gpu_offset==gpu_offset, "gpu_offset out of range"); T.glsl_name=glsl_name;}
Bool ShaderGL::save(File &f, C Map<Str8, ShaderParam> &params, C Memc<ShaderImage*> &images)C
{
   f.putStr(name);
   f.putMulti(vs_index, ps_index);
   f.cmpUIntV(glsl_params.elms()); FREPA(glsl_params)f<<glsl_params[i].gpu_offset<<AsUShort(GetIndex(params, glsl_params[i].param))<<glsl_params[i].glsl_name;
 //f.cmpUIntV(glsl_images.elms()); FREPA(glsl_images)f<<                           AsUShort(GetIndex(images, glsl_images[i]      ));
   return f.ok();
}
Bool ShaderGL::load(File &f, C MemtN<ShaderParam*, 256> &params, C MemtN<ShaderImage*, 256> &images)
{
   f.getStr(name);
   f.getMulti(vs_index, ps_index);
   glsl_params.setNum(f.decUIntV()); FREPA (glsl_params){GLSLParam &param=glsl_params[i]; f>>param.gpu_offset; param.param=Get(f.getUShort(), params); f>>param.glsl_name;}
 //glsl_images.setNum(f.decUIntV()); FREPAO(glsl_images)=                                                                  Get(f.getUShort(), images);
   if(f.ok())return true;
   /*del();*/ return false;
}
#endif
/******************************************************************************/
static Bool HasData(Byte *data, Int size)
{
   FREP(size)if(data[i])return true;
   return false;
}
static void SaveTranslation(C Mems<ShaderParam::Translation> &translation, File &f, Int elms)
{
   if(elms<=1)translation.saveRaw(f);else
   {
      UShort single_translations=translation.elms()/elms,
         gpu_offset=translation[single_translations].gpu_offset-translation[0].gpu_offset,
         cpu_offset=translation[single_translations].cpu_offset-translation[0].cpu_offset;
      f.putMulti(gpu_offset, cpu_offset, single_translations);
      FREP(single_translations)f<<translation[i]; // save 1st element translation
   }
}
static void LoadTranslation(MemPtr<ShaderParam::Translation> translation, File &f, Int elms)
{
   if(elms<=1)translation.loadRaw(f);else
   {
      translation.clear();
      UShort single_translations, gpu_offset, cpu_offset; f.getMulti(gpu_offset, cpu_offset, single_translations);
      FREP(  single_translations)f>>translation.New(); // load 1st element translation
      for(Int e=1, co=0, go=0; e<elms; e++) // add rest of the elements
      {
         co+=cpu_offset; // element offset
         go+=gpu_offset; // element offset
         FREP(single_translations)
         {
            ShaderParam::Translation &t=translation.New(); // create and store reference !! memory address changes, do not perform adding new element and referencing previous element in one line of code !!
            t=translation[i];
            t.cpu_offset+=co;
            t.gpu_offset+=go;
         }
      }
   }
}
static void LimitTranslation(ShaderParam &sp)
{
   Memt<ShaderParam::Translation> translation;
   FREPA(sp._full_translation) // go from the start
   {
      ShaderParam::Translation &t=sp._full_translation[i];
      Int size=t.elm_size; // copy to temp var in case original is unsigned
      Int end=Min(t.cpu_offset+size, sp._cpu_data_size); MIN(size, end-t.cpu_offset);
          end=Min(t.gpu_offset+size, sp._gpu_data_size); MIN(size, end-t.gpu_offset);
      if(size>0){t.elm_size=size; translation.add(t);}
   }
   sp._full_translation=translation;
}
Bool ShaderParam::save(File &f, C Str8 &name)C
{
   Byte *param_data=_data+_full_translation[0].gpu_offset; // in DX10+, '_data' when saving, points to Shader Constant Buffer data, that's why we need to use offset

   f.putStr(name).putMulti(_cpu_data_size, _gpu_data_size, _elements); // name+info
   SaveTranslation(_full_translation, f, _elements);                   // translation
   if(HasData(param_data, _gpu_data_size))                             // data
   {
      f.putBool(true);
      f.put(param_data, _gpu_data_size);
   }else
   {
      f.putBool(false);
   }
   return f.ok();
}
/******************************************************************************/
static void ExitParam(C Str &param_name, C Str &shader_name)
{
#if GL && VARIABLE_MAX_MATRIX
   if(D.meshBoneSplit())if(param_name=="ViewMatrix" || param_name=="ObjVel" || param_name=="FurVel")return; // allow ViewMatrix and ObjVel to differ, because they're dynamically resized depending on GPU capabilities
#endif
   Exit(S+"Shader Param \""+param_name+"\"\nfrom Shader File \""+shader_name+"\"\nAlready exists in Shader Constants Map but with different parameters.\nThis means that some of your shaders were compiled with different headers.\nPlease recompile your shaders.");
}
Bool ShaderFile::load(C Str &name)
{
   del();

   Str8 temp_str;
   File f; if(f.readTry(Sh.path+name))
   {
      if(f.getUInt()==CC4_SHDR) // cc4
      {
         switch(f.getByte()) // type
         {
         #if DX9
            case SHADER_DX9:
            {
               switch(f.decUIntV()) // version
               {
                  case 0:
                  {
                     // params
                     MemtN<ShaderParam*, 256> params; params.setNum(f.decUIntV());
                     ShaderParams.lock();
                     FREPA(params)
                     {
                        f.getStr(temp_str); ShaderParam &sp=*ShaderParams(temp_str); params[i]=&sp;
                        if(!sp.is()) // wasn't yet created
                        {
                           sp._owns_data=true;
                           f.getMulti(sp._cpu_data_size, sp._gpu_data_size, sp._elements); // info
                           LoadTranslation(sp._full_translation, f, sp._elements);         // translation
                           Alloc(sp._data, sp._gpu_data_size);                             // data
                           Alloc(sp._changed                );
                                 sp._constant_count=sp.fullConstantCount();
                           if(f.getBool())f.get(sp._data, sp._gpu_data_size); // load default value
                           else           Zero (sp._data, sp._gpu_data_size); // zero default value
                           sp.optimize();
                        }else // verify if it's identical to previously created
                        {
                           Memt<ShaderParam::Translation> translation;
                           Int cpu_data_size, gpu_data_size, elements; f.getMulti(cpu_data_size, gpu_data_size, elements);
                           if(sp._cpu_data_size!=cpu_data_size                            // check cpu size
                           || sp._gpu_data_size!=gpu_data_size                            // check gpu size
                           || sp._elements     !=elements     )ExitParam(temp_str, name); // check number of elements
                           LoadTranslation(translation, f, sp._elements);                 // translation
                           if(f.getBool())f.skip(sp._gpu_data_size);                      // ignore default value

                           // check translation
                           if(                  translation.elms()!=sp._full_translation.elms())ExitParam(temp_str, name);
                           FREPA(translation)if(translation[i]    !=sp._full_translation[i]    )ExitParam(temp_str, name);
                        }
                     }
                     ShaderParams.unlock();

                     // images
                     MemtN<ShaderImage*, 256> images; images.setNum(f.decUIntV());
                     FREPA(images){f.getStr(temp_str); images[i]=ShaderImages(temp_str);}

                     // shaders
                     if(_vs     .load(f))
                     if(_ps     .load(f))
                     if(_shaders.load(f, params, images))
                        if(f.ok())return true;
                  }break;
               }
            }break;
         #elif DX11
            case SHADER_DX11:
            {
               switch(f.decUIntV()) // version
               {
                  case 0:
                  {
                     // buffers
                     MemtN<ShaderBuffer*, 256> buffers; buffers.setNum(f.decUIntV());
                     ShaderBuffers.lock();
                     ShaderParams .lock();
                     FREPA(buffers)
                     {
                        // buffer
                        f.getStr(temp_str); ShaderBuffer &sb=*ShaderBuffers(temp_str); buffers[i]=&sb;
                        if(!sb.is()) // wasn't yet created
                        {
                           sb.create(f.decUIntV());
                           Int index=f.getSByte(); if(index>=0){SyncLocker lock(D._lock); sb.bind(index);}
                        }else // verify if it's identical to previously created
                        {
                           if(sb.size()!=f.decUIntV())ExitParam(temp_str, name);
                           sb.bindCheck(f.getSByte());
                        }

                        // params
                        REP(f.decUIntV())
                        {
                           f.getStr(temp_str); ShaderParam &sp=*ShaderParams(temp_str);
                           if(!sp.is()) // wasn't yet created
                           {
                              sp._owns_data= false;
                              sp._data     = sb.data;
                              sp._changed  =&sb.changed;
                              f.getMulti(sp._cpu_data_size, sp._gpu_data_size, sp._elements); // info
                            //sp._constant_count=                                             // unused on DX10+
                              LoadTranslation(sp._full_translation, f, sp._elements);         // translation
                              Int offset=sp._full_translation[0].gpu_offset; sp._data+=offset; REPAO(sp._full_translation).gpu_offset-=offset; // apply offset
                              if(f.getBool())f.get(sp._data, sp._gpu_data_size);              // load default value, no need to zero in other case, because data is stored in ShaderBuffer's, and they're always zeroed at start
                              sp.optimize(); // optimize
                           }else // verify if it's identical to previously created
                           {
                              Int cpu_data_size, gpu_data_size, elements; f.getMulti(cpu_data_size, gpu_data_size, elements);
                              Memt<ShaderParam::Translation> translation;
                              if(sp._changed      !=&sb.changed                               // check matching Constant Buffer
                              || sp._cpu_data_size!= cpu_data_size                            // check cpu size
                              || sp._gpu_data_size!= gpu_data_size                            // check gpu size
                              || sp._elements     != elements     )ExitParam(temp_str, name); // check number of elements
                              LoadTranslation(translation, f, sp._elements);                  // translation
                              Int offset=translation[0].gpu_offset; REPAO(translation).gpu_offset-=offset; // apply offset
                              if(f.getBool())f.skip(sp._gpu_data_size);                       // ignore default value

                              // check translation
                              if(                  translation.elms()!=sp._full_translation.elms())ExitParam(temp_str, name);
                              FREPA(translation)if(translation[i]    !=sp._full_translation[i]    )ExitParam(temp_str, name);
                           }
                        }
                     }
                     ShaderParams .unlock();
                     ShaderBuffers.unlock();

                     // images
                     MemtN<ShaderImage*, 256> images; images.setNum(f.decUIntV());
                     FREPA(images){f.getStr(temp_str); images[i]=ShaderImages(temp_str);}

                     // shaders
                     if(_vs     .load(f))
                     if(_hs     .load(f))
                     if(_ds     .load(f))
                     if(_ps     .load(f))
                     if(_shaders.load(f, buffers, images))
                        if(f.ok())return true;
                  }break;
               }
            }break;
         #elif GL
            case SHADER_GL:
            {
               switch(f.decUIntV()) // version
               {
                  case 0:
                  {
                     // params
                     MemtN<ShaderParam*, 256> params; params.setNum(f.decUIntV());
                     ShaderParams.lock();
                     FREPA(params)
                     {
                        f.getStr(temp_str); ShaderParam &sp=*ShaderParams(temp_str); params[i]=&sp;
                        if(!sp.is()) // wasn't yet created
                        {
                           sp._owns_data=true;
                           f.getMulti(sp._cpu_data_size, sp._gpu_data_size, sp._elements); // info
                           LoadTranslation(sp._full_translation, f, sp._elements);         // translation
                           Alloc(sp._data, sp._gpu_data_size);                             // data
                           Alloc(sp._changed                );
                           if(f.getBool())f.get(sp._data, sp._gpu_data_size);              // load default value
                           else           Zero (sp._data, sp._gpu_data_size);              // zero default value
                        #if VARIABLE_MAX_MATRIX
                           when enabling VARIABLE_MAX_MATRIX then shaders need to be recompiled with "MAX_MATRIX 256" (for CG and GLSL) because we're only reducing in 'LimitTranslation' and when replacing shader codes
                           if(D.meshBoneSplit())
                           {
                              if(temp_str=="ViewMatrix")
                              {
                                 sp._cpu_data_size=
                                 sp._gpu_data_size=SIZE(GpuMatrix)*D.maxShaderMatrixes();
                                 LimitTranslation(sp);
                              }else
                              if(temp_str=="ObjVel")
                              {
                                 sp._cpu_data_size=
                                 sp._gpu_data_size=SIZE(Vec)*D.maxShaderMatrixes();
                                 LimitTranslation(sp);
                              }else
                              if(temp_str=="FurVel")
                              {
                                 sp._cpu_data_size=
                                 sp._gpu_data_size=SIZE(Vec)*D.maxShaderMatrixes();
                                 LimitTranslation(sp);
                              }
                           }
                        #endif
                           sp._constant_count=sp.fullConstantCount();
                           sp.optimize();
                        }else // verify if it's identical to previously created
                        {
                           Int cpu_data_size, gpu_data_size, elements; f.getMulti(cpu_data_size, gpu_data_size, elements);
                           Memt<ShaderParam::Translation> translation;
                           if(sp._cpu_data_size!=cpu_data_size                            // check cpu size
                           || sp._gpu_data_size!=gpu_data_size                            // check gpu size
                           || sp._elements     !=elements     )ExitParam(temp_str, name); // check number of elements
                           LoadTranslation(translation, f, elements);                     // translation
                           if(f.getBool())f.skip(gpu_data_size);                          // ignore default value

                           // check translation
                           if(                  translation.elms()!=sp._full_translation.elms()) ExitParam(temp_str, name);else
                           FREPA(translation)if(translation[i]    !=sp._full_translation[i]    ){ExitParam(temp_str, name); break;}
                        }
                     }
                     ShaderParams.unlock();

                     // images
                     MemtN<ShaderImage*, 256> images; images.setNum(f.decUIntV());
                     FREPA(images){f.getStr(temp_str); images[i]=ShaderImages(temp_str);}

                     // shaders
                     if(_vs     .load(f))
                     if(_ps     .load(f))
                     if(_shaders.load(f, params, images))
                        if(f.ok())return true;
                  }break;
               }
            }break;
         #endif
         }
      }
   }
//error:
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
