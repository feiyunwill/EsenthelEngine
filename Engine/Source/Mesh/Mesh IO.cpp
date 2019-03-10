/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_MSHB CC4('M','S','H','B')
#define CC4_MSHR CC4('M','S','H','R')
#define CC4_MSHP CC4('M','S','H','P')
#define CC4_MSHL CC4('M','S','H','L')
#define CC4_MESH CC4('M','E','S','H')
#define CC4_MSHG CC4('M','S','H','G')
/******************************************************************************/
XMaterial::XMaterial()
{
   cull         =true;
   flip_normal_y=false;
   technique    =MTECH_DEFAULT;
   color        =1;
   ambient      =0;
   specular     =0;
   sss          =0;
   glow         =0;
   rough        =1;
   bump         =0.03f;
   tex_scale    =1.0f;
   det_scale    =4;
   det_power    =0.3f;
   reflection   =0.2f;
}
void XMaterial::del()
{
   T=XMaterial();
}
void XMaterial::createFrom(C Material &src)
{
   cull          =src.cull;
   technique     =src.technique;
   color         =src.color;
   ambient       =src.ambient;
   specular      =src.specular;
   sss           =src.sss;
   glow          =src.glow;
   rough         =src.rough;
   bump          =src.bump;
   tex_scale     =src.tex_scale;
   det_scale     =src.det_scale;
   det_power     =src.det_power;
   reflection    =src.reflect;
        color_map=src.        base_0.name();
       normal_map=src.        base_1.name();
 detail_color_map=src.    detail_map.name();
   reflection_map=src.reflection_map.name();
        light_map=src.     light_map.name();
}
void XMaterial::copyParamsTo(Material &mtrl)C
{
   mtrl.cull     =cull;
   mtrl.technique=technique;
   mtrl.color    =color;
   mtrl.ambient  =ambient;
   mtrl.specular =specular.max();
   mtrl.sss      =sss;
   mtrl.glow     =glow;
   mtrl.rough    =rough;
   mtrl.bump     =bump;
   mtrl.tex_scale=tex_scale;
   mtrl.det_scale=det_scale;
   mtrl.det_power=det_power;
   mtrl.reflect  =reflection;
   mtrl.validate();
}
static void FixPath(Str &name, Str &path)
{
   if(name.is() && !FExistSystem(name))
   {
      Str test;
      if(!FullPath(name)) // relative
      {
         test=NormalizePath(path+name);
         if(FExistSystem(test)){name=test; return;}
      }

      test=path+GetBase(name); // skip the path and leave only file name
      if(FExistSystem(test)){name=test; return;}

    //name.clear(); don't clear but leave as it was
   }
}
void XMaterial::fixPath(Str path)
{
   path.tailSlash(true);
   FixPath(          color_map, path);
   FixPath(          alpha_map, path);
   FixPath(          light_map, path);
   FixPath(           bump_map, path);
   FixPath(         normal_map, path);
   FixPath(       specular_map, path);
   FixPath(     reflection_map, path);
   FixPath(   detail_color_map, path);
   FixPath(    detail_bump_map, path);
   FixPath(  detail_normal_map, path);
   FixPath(detail_specular_map, path);
}
Bool XMaterial::save(File &f)C
{
   f.cmpUIntV(0); // version
   f<<cull<<flip_normal_y<<technique<<color<<ambient<<specular
    <<name<<color_map<<alpha_map<<bump_map<<glow_map<<light_map<<normal_map<<specular_map<<reflection_map<<detail_color_map<<detail_bump_map<<detail_normal_map<<detail_specular_map;
   return f.ok();
}
Bool XMaterial::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>cull>>flip_normal_y>>technique>>color>>ambient>>specular
          >>name>>color_map>>alpha_map>>bump_map>>glow_map>>light_map>>normal_map>>specular_map>>reflection_map>>detail_color_map>>detail_bump_map>>detail_normal_map>>detail_specular_map;
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// MSHB
/******************************************************************************/
Bool MeshBase::saveData(File &f)C
{
   f.cmpUIntV(3); // version

   UInt flag =T.flag (); f.putUInt (flag );
   Int  vtxs =T.vtxs (); f.cmpUIntV(vtxs );
   Int  edges=T.edges(); f.cmpUIntV(edges);
   Int  tris =T.tris (); f.cmpUIntV(tris );
   Int  quads=T.quads(); f.cmpUIntV(quads);

   if(flag&VTX_POS     )f.putN(vtx.pos     (), vtxs);
   if(flag&VTX_NRM     )f.putN(vtx.nrm     (), vtxs);
   if(flag&VTX_TAN     )f.putN(vtx.tan     (), vtxs);
   if(flag&VTX_BIN     )f.putN(vtx.bin     (), vtxs);
   if(flag&VTX_HLP     )f.putN(vtx.hlp     (), vtxs);
   if(flag&VTX_TEX0    )f.putN(vtx.tex0    (), vtxs);
   if(flag&VTX_TEX1    )f.putN(vtx.tex1    (), vtxs);
   if(flag&VTX_TEX2    )f.putN(vtx.tex2    (), vtxs);
   if(flag&VTX_MATRIX  )f.putN(vtx.matrix  (), vtxs);
   if(flag&VTX_BLEND   )f.putN(vtx.blend   (), vtxs);
   if(flag&VTX_SIZE    )f.putN(vtx.size    (), vtxs);
   if(flag&VTX_MATERIAL)f.putN(vtx.material(), vtxs);
   if(flag&VTX_COLOR   )f.putN(vtx.color   (), vtxs);
   if(flag&VTX_FLAG    )f.putN(vtx.flag    (), vtxs);

   if(flag&EDGE_NRM)f.putN(edge.nrm(), edges);
   if(flag& TRI_NRM)f.putN(tri .nrm(), tris );
   if(flag&QUAD_NRM)f.putN(quad.nrm(), quads);

   if(flag&EDGE_FLAG)f.putN(edge.flag(), edges);
   if(flag& TRI_FLAG)f.putN(tri .flag(), tris );
   if(flag&QUAD_FLAG)f.putN(quad.flag(), quads);

   if(flag&EDGE_ID)f.putN(edge.id(), edges);
   if(flag& TRI_ID)f.putN(tri .id(), tris );
   if(flag&QUAD_ID)f.putN(quad.id(), quads);

   if(flag&EDGE_IND     )IndSave(f, edge.ind    (), edges*2, vtxs);
   if(flag& TRI_IND     )IndSave(f, tri .ind    (), tris *3, vtxs);
   if(flag&QUAD_IND     )IndSave(f, quad.ind    (), quads*4, vtxs);
   if(flag&EDGE_ADJ_FACE)IndSave(f, edge.adjFace(), edges*2); // can't use size reduction for adjacent faces because they can contain -1 and SIGN_BIT
   if(flag& TRI_ADJ_FACE)IndSave(f, tri .adjFace(), tris *3); // can't use size reduction for adjacent faces because they can contain -1 and SIGN_BIT
   if(flag&QUAD_ADJ_FACE)IndSave(f, quad.adjFace(), quads*4); // can't use size reduction for adjacent faces because they can contain -1 and SIGN_BIT
   if(flag& TRI_ADJ_EDGE)IndSave(f, tri .adjEdge(), tris *3); // can't use size reduction for adjacent edges because they can contain -1
   if(flag&QUAD_ADJ_EDGE)IndSave(f, quad.adjEdge(), quads*4); // can't use size reduction for adjacent edges because they can contain -1

   return f.ok();
}
/******************************************************************************/
Bool MeshBase::loadData(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 3:
      {
         UInt flag =f.getUInt ();
         Int  vtxs =f.decUIntV(),
              edges=f.decUIntV(),
              tris =f.decUIntV(),
              quads=f.decUIntV();
         create(vtxs, edges, tris, quads, flag);

         if(flag&VTX_POS     )f.getN(vtx.pos     (), vtxs);
         if(flag&VTX_NRM     )f.getN(vtx.nrm     (), vtxs);
         if(flag&VTX_TAN     )f.getN(vtx.tan     (), vtxs);
         if(flag&VTX_BIN     )f.getN(vtx.bin     (), vtxs);
         if(flag&VTX_HLP     )f.getN(vtx.hlp     (), vtxs);
         if(flag&VTX_TEX0    )f.getN(vtx.tex0    (), vtxs);
         if(flag&VTX_TEX1    )f.getN(vtx.tex1    (), vtxs);
         if(flag&VTX_TEX2    )f.getN(vtx.tex2    (), vtxs);
         if(flag&VTX_MATRIX  )f.getN(vtx.matrix  (), vtxs);
         if(flag&VTX_BLEND   )f.getN(vtx.blend   (), vtxs);
         if(flag&VTX_SIZE    )f.getN(vtx.size    (), vtxs);
         if(flag&VTX_MATERIAL)f.getN(vtx.material(), vtxs);
         if(flag&VTX_COLOR   )f.getN(vtx.color   (), vtxs);
         if(flag&VTX_FLAG    )f.getN(vtx.flag    (), vtxs);

         if(flag&EDGE_NRM)f.getN(edge.nrm(), edges);
         if(flag& TRI_NRM)f.getN(tri .nrm(), tris );
         if(flag&QUAD_NRM)f.getN(quad.nrm(), quads);

         if(flag&EDGE_FLAG)f.getN(edge.flag(), edges);
         if(flag& TRI_FLAG)f.getN(tri .flag(), tris );
         if(flag&QUAD_FLAG)f.getN(quad.flag(), quads);

         if(flag&EDGE_ID)f.getN(edge.id(), edges);
         if(flag& TRI_ID)f.getN(tri .id(), tris );
         if(flag&QUAD_ID)f.getN(quad.id(), quads);

         if(flag&EDGE_IND     )IndLoad(f, edge.ind    (), edges*2);
         if(flag& TRI_IND     )IndLoad(f, tri .ind    (), tris *3);
         if(flag&QUAD_IND     )IndLoad(f, quad.ind    (), quads*4);
         if(flag&EDGE_ADJ_FACE)IndLoad(f, edge.adjFace(), edges*2);
         if(flag& TRI_ADJ_FACE)IndLoad(f, tri .adjFace(), tris *3);
         if(flag&QUAD_ADJ_FACE)IndLoad(f, quad.adjFace(), quads*4);
         if(flag& TRI_ADJ_EDGE)IndLoad(f, tri .adjEdge(), tris *3);
         if(flag&QUAD_ADJ_EDGE)IndLoad(f, quad.adjEdge(), quads*4);

         if(f.ok())return true;
      }break;

      case 2:
      {
         UInt flag =f.getUInt();
         Int  vtxs =f.getInt (),
              edges=f.getInt (),
              tris =f.getInt (),
              quads=f.getInt ();
         create(vtxs, edges, tris, quads, flag&~ID_ALL);

         if(flag&VTX_POS     )f.getN(vtx.pos     (), vtxs);
         if(flag&VTX_NRM     )f.getN(vtx.nrm     (), vtxs);
         if(flag&VTX_TAN     )f.getN(vtx.tan     (), vtxs);
         if(flag&VTX_BIN     )f.getN(vtx.bin     (), vtxs);
         if(flag&VTX_HLP     )f.getN(vtx.hlp     (), vtxs);
         if(flag&VTX_TEX0    )f.getN(vtx.tex0    (), vtxs);
         if(flag&VTX_TEX1    )f.getN(vtx.tex1    (), vtxs);
         if(flag&VTX_TEX2    )f.getN(vtx.tex2    (), vtxs);
         if(flag&VTX_MATRIX  )f.getN(vtx.matrix  (), vtxs);
         if(flag&VTX_BLEND   )f.getN(vtx.blend   (), vtxs);
         if(flag&VTX_SIZE    )f.getN(vtx.size    (), vtxs);
         if(flag&VTX_MATERIAL)f.getN(vtx.material(), vtxs);
         if(flag&VTX_COLOR   )f.getN(vtx.color   (), vtxs);
         if(flag&VTX_FLAG    )f.getN(vtx.flag    (), vtxs);

         if(flag&EDGE_NRM)f.getN(edge.nrm(), edges);
         if(flag& TRI_NRM)f.getN(tri .nrm(), tris );
         if(flag&QUAD_NRM)f.getN(quad.nrm(), quads);

         if(flag&EDGE_FLAG)f.getN(edge.flag(), edges);
         if(flag& TRI_FLAG)f.getN(tri .flag(), tris );
         if(flag&QUAD_FLAG)f.getN(quad.flag(), quads);

         if(flag&EDGE_ID)f.skip(SIZE(VecI2)*edges);
         if(flag& TRI_ID)f.skip(SIZE(VecI2)*tris );
         if(flag&QUAD_ID)f.skip(SIZE(VecI2)*quads);

         if(flag&EDGE_IND     )IndLoad(f, edge.ind    (), edges*2);
         if(flag& TRI_IND     )IndLoad(f, tri .ind    (), tris *3);
         if(flag&QUAD_IND     )IndLoad(f, quad.ind    (), quads*4);
         if(flag&EDGE_ADJ_FACE)IndLoad(f, edge.adjFace(), edges*2);
         if(flag& TRI_ADJ_FACE)IndLoad(f, tri .adjFace(), tris *3);
         if(flag&QUAD_ADJ_FACE)IndLoad(f, quad.adjFace(), quads*4);
         if(flag& TRI_ADJ_EDGE)IndLoad(f, tri .adjEdge(), tris *3);
         if(flag&QUAD_ADJ_EDGE)IndLoad(f, quad.adjEdge(), quads*4);

         if(f.ok())return true;
      }break;

      case 1:
      {
         UInt flag =f.getUInt();
         Int  vtxs =f.getInt (),
              edges=f.getInt (),
              tris =f.getInt (),
              quads=f.getInt ();
         create(vtxs, edges, tris, quads, flag&~ID_ALL);

         if(flag&VTX_POS     )f.getN(vtx.pos     (), vtxs);
         if(flag&VTX_NRM     )f.getN(vtx.nrm     (), vtxs);
         if(flag&VTX_TAN     )f.getN(vtx.tan     (), vtxs);
         if(flag&VTX_BIN     )f.getN(vtx.bin     (), vtxs);
         if(flag&VTX_HLP     )f.getN(vtx.hlp     (), vtxs);
         if(flag&VTX_TEX0    )f.getN(vtx.tex0    (), vtxs);
         if(flag&VTX_TEX1    )f.getN(vtx.tex1    (), vtxs);
         if(flag&VTX_TEX2    )f.getN(vtx.tex2    (), vtxs);
         if(flag&VTX_MATRIX  )f.getN(vtx.matrix  (), vtxs);
         if(flag&VTX_BLEND   )f.getN(vtx.blend   (), vtxs); if(vtx.blend())REP(vtxs){Flt f=(Flt&)vtx.blend(i); Byte b=RoundPos(f*255); vtx.blend(i).set(b, 255-b, 0, 0);}
         if(flag&VTX_SIZE    )f.getN(vtx.size    (), vtxs);
         if(flag&VTX_MATERIAL)f.getN(vtx.material(), vtxs);
         if(flag&VTX_COLOR   )f.getN(vtx.color   (), vtxs);
         if(flag&VTX_FLAG    )f.getN(vtx.flag    (), vtxs);

         if(flag&EDGE_NRM)f.getN(edge.nrm(), edges);
         if(flag& TRI_NRM)f.getN(tri .nrm(), tris );
         if(flag&QUAD_NRM)f.getN(quad.nrm(), quads);

         if(flag&EDGE_FLAG)f.getN(edge.flag(), edges);
         if(flag& TRI_FLAG)f.getN(tri .flag(), tris );
         if(flag&QUAD_FLAG)f.getN(quad.flag(), quads);

         if(flag&EDGE_ID)f.skip(SIZE(VecI2)*edges);
         if(flag& TRI_ID)f.skip(SIZE(VecI2)*tris );
         if(flag&QUAD_ID)f.skip(SIZE(VecI2)*quads);

         if(flag&EDGE_IND     )IndLoad(f, edge.ind    (), edges*2);
         if(flag& TRI_IND     )IndLoad(f, tri .ind    (), tris *3);
         if(flag&QUAD_IND     )IndLoad(f, quad.ind    (), quads*4);
         if(flag&EDGE_ADJ_FACE)IndLoad(f, edge.adjFace(), edges*2);
         if(flag& TRI_ADJ_FACE)IndLoad(f, tri .adjFace(), tris *3);
         if(flag&QUAD_ADJ_FACE)IndLoad(f, quad.adjFace(), quads*4);
         if(flag& TRI_ADJ_EDGE)IndLoad(f, tri .adjEdge(), tris *3);
         if(flag&QUAD_ADJ_EDGE)IndLoad(f, quad.adjEdge(), quads*4);

         if(f.ok())return true;
      }break;

      case 0:
      {
         f.getByte(); // part of old U16 version byte

         UInt flag =f.getUInt();
         Int  vtxs =f.getInt (),
              edges=f.getInt (),
              tris =f.getInt (),
              quads=f.getInt ();
         create(vtxs, edges, tris, quads, flag&~ID_ALL);

         if(flag&VTX_POS     )f.getN(vtx.pos     (), vtxs);
         if(flag&VTX_NRM     )f.getN(vtx.nrm     (), vtxs);
         if(flag&VTX_TAN     )f.getN(vtx.tan     (), vtxs);
         if(flag&VTX_BIN     )f.getN(vtx.bin     (), vtxs);
         if(flag&VTX_HLP     )f.getN(vtx.hlp     (), vtxs);
         if(flag&VTX_TEX0    )f.getN(vtx.tex0    (), vtxs);
         if(flag&VTX_TEX1    )f.getN(vtx.tex1    (), vtxs);
         if(flag&VTX_TEX2    )f.getN(vtx.tex2    (), vtxs);
         if(flag&VTX_MATRIX  )f.getN(vtx.matrix  (), vtxs);
         if(flag&VTX_BLEND   )f.getN(vtx.blend   (), vtxs); if(vtx.blend())REP(vtxs){Flt f=(Flt&)vtx.blend(i); Byte b=RoundPos(f*255); vtx.blend(i).set(b, 255-b, 0, 0);}
         if(flag&VTX_SIZE    )f.getN(vtx.size    (), vtxs);
         if(flag&VTX_MATERIAL)f.getN(vtx.material(), vtxs);
         if(flag&VTX_COLOR   )f.getN(vtx.color   (), vtxs); if(vtx.color())REP(vtxs)Swap(vtx.color(i).r, vtx.color(i).b); // convert BGRA -> RGBA
         if(flag&VTX_FLAG    )f.getN(vtx.flag    (), vtxs);

         if(flag&EDGE_NRM)f.getN(edge.nrm(), edges);
         if(flag& TRI_NRM)f.getN(tri .nrm(), tris );
         if(flag&QUAD_NRM)f.getN(quad.nrm(), quads);

         if(flag&EDGE_FLAG)f.getN(edge.flag(), edges);
         if(flag& TRI_FLAG)f.getN(tri .flag(), tris );
         if(flag&QUAD_FLAG)f.getN(quad.flag(), quads);

         if(flag&EDGE_ID)f.skip(SIZE(VecI2)*edges);
         if(flag& TRI_ID)f.skip(SIZE(VecI2)*tris );
         if(flag&QUAD_ID)f.skip(SIZE(VecI2)*quads);

         if(flag&EDGE_IND     )IndLoad(f, edge.ind    (), edges*2);
         if(flag& TRI_IND     )IndLoad(f, tri .ind    (), tris *3);
         if(flag&QUAD_IND     )IndLoad(f, quad.ind    (), quads*4);
         if(flag&EDGE_ADJ_FACE)IndLoad(f, edge.adjFace(), edges*2);
         if(flag& TRI_ADJ_FACE)IndLoad(f, tri .adjFace(), tris *3);
         if(flag&QUAD_ADJ_FACE)IndLoad(f, quad.adjFace(), quads*4);
         if(flag& TRI_ADJ_EDGE)IndLoad(f, tri .adjEdge(), tris *3);
         if(flag&QUAD_ADJ_EDGE)IndLoad(f, quad.adjEdge(), quads*4);

         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
Bool MeshBase::save(File &f)C
{
   f.putUInt(CC4_MSHB);
   return saveData(f);
}
Bool MeshBase::load(File &f, CChar *path)
{
   switch(f.getUInt())
   {
      case CC4_MSHB: return loadData(f);

      case CC4_MESH:
      {
         Mesh mesh; if(mesh.loadData(f, path))
         {
            create(mesh);
            return true;
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
Bool MeshBase::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool MeshBase::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
// MSHR
/******************************************************************************/
Bool MeshRender::saveData(File &f)C
{
   f.cmpUIntV(5); // version
   if(_vb.save(f))
   if(_ib.save(f))
   {
      if(_vb._vtx_num || _ib._ind_num)
      {
         f.putMulti(_storage, _flag, _bone_splits).putN(_bone_split, _bone_splits);
      }
      return f.ok();
   }
   return false;
}
static void Fix(MeshRender &m, Bool color, Bool bone_weight)
{
   Int col_ofs=(color       ? m.vtxOfs(VTX_COLOR) : -1),
     blend_ofs=(bone_weight ? m.vtxOfs(VTX_BLEND) : -1);
   if( col_ofs>=0 || blend_ofs>=0)if(Byte *vtx=m.vtxLock())
   {
      REP(m._vb.vtxs())
      {
         if(  col_ofs>=0){Color &c=*(Color*)(vtx+  col_ofs); Swap(c.r, c.b);}
         if(blend_ofs>=0){VecB4 &b=*(VecB4*)(vtx+blend_ofs); Flt f=(Flt&)b; Byte i=RoundPos(f*255); b.set(i, 255-i, 0, 0);}
         vtx+=m._vb._vtx_size;
      }
      m.vtxUnlock();
   }
}
Bool MeshRender::loadData(File &f)
{
   del(); switch(f.decUIntV()) // version
   {
      case 5:
      {
         if(_vb.load(f))
         if(_ib.load(f))
         {
            if(_vb._vtx_num || _ib._ind_num)
            {
               f.getMulti(_storage, _flag, _bone_splits);
               f.getN(Alloc(_bone_split, _bone_splits), _bone_splits); // !! keep as a separate command because if f.get().get() are merged then 'Alloc' may be called before the first get !!
              _tris=_ib._ind_num/3;
               adjustToPlatform();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!
            }
          //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh.loadData)
            if(f.ok())return true;
         }
      }break;

      case 4:
      {
         if(_vb.load(f))
         if(_ib.load(f))
         {
            if(_vb._vtx_num || _ib._ind_num)
            {
               f>>_storage>>_flag>>_bone_splits; f.getN(Alloc(_bone_split, _bone_splits), _bone_splits);
              _tris=_ib._ind_num/3;
               adjustToPlatform();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!
               Fix(T, false, true);
            }
          //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh.loadData)
            if(f.ok())return true;
         }
      }break;

      case 3:
      {
         if(_vb.load(f))
         if(_ib.load(f))
         {
            if(_vb._vtx_num || _ib._ind_num)
            {
               Bool compress, dx10; f>>compress>>dx10; _storage=(compress?MSHR_COMPRESS:0)|(dx10?MSHR_SIGNED:MSHR_BONE_SPLIT);
               f>>_flag>>_bone_splits; f.getN(Alloc(_bone_split, _bone_splits), _bone_splits);
              _tris=_ib._ind_num/3;
               adjustToPlatform();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!
               Fix(T, true, true);
            }
          //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh.loadData)
            if(f.ok())return true;
         }
      }break;

      case 2:
      {
         if(_vb.load(f))
         if(_ib.load(f))
         {
            if(_vb._vtx_num || _ib._ind_num)
            {
               Bool compress; f>>compress; _storage=MSHR_BONE_SPLIT|(compress?MSHR_COMPRESS:0);
               f>>_flag>>_bone_splits; f.getN(Alloc(_bone_split, _bone_splits), _bone_splits);
              _tris=_ib._ind_num/3;
               adjustToPlatform();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!
               Fix(T, true, true);
            }
          //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh.loadData)
            if(f.ok())return true;
         }
      }break;

      case 1:
      {
         Int vtxs,   tris; f>>vtxs>>tris;
         if( vtxs && tris)
         {
            Bool bit16       =!f.getBool();
            Int  vtx_size    = f.getInt ();
            Int  tri_ind_size= f.getInt ();
            Int  bone_splits = f.getInt ();
            UInt flag        = f.getUInt();

            if(_vb.createNum(vtx_size, vtxs ))
            if(_ib.create   (tris*3  , bit16))
            {
               T._storage=MSHR_BONE_SPLIT;
               T._tris   =tris;
               T._flag   =flag;
                                             if(!f.getN(Alloc(_bone_split, T._bone_splits=bone_splits), bone_splits))goto error;
               Ptr data=vtxLock(LOCK_WRITE); if(!f.get (data, vtxs*vtxSize()   ))goto error; vtxUnlock();
                   data=indLock(LOCK_WRITE); if(!f.get (data, tris*tri_ind_size))goto error; indUnlock();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!

               Fix(T, true, true);

               MeshBase mshb(T); create(mshb, ~0, false);
             //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh::loadData)
               if(f.ok())return true;
            }
         }else if(f.ok())return true;
      }break;

      case 0:
      {
         f.getByte(); // old ver
         Int vtxs,   tris; f>>vtxs>>tris;
         if( vtxs && tris)
         {
            Bool bit16       =!f.getBool();
            Int  vtx_size    = f.getInt ();
            Int  tri_ind_size= f.getInt ();
            UInt flag        = f.getUInt();

            if(_vb.createNum(vtx_size, vtxs ))
            if(_ib.create   (tris*3  , bit16))
            {
               T._storage=0;
               T._tris   =tris;
               T._flag   =flag;

               Ptr data=vtxLock(LOCK_WRITE); if(!f.get(data, vtxs*vtxSize()   ))goto error; vtxUnlock();
                   data=indLock(LOCK_WRITE); if(!f.get(data, tris*tri_ind_size))goto error; indUnlock();
               if(!setVF())goto error; // !! call at the end (when have VB IB and flag/storage) !!

               Fix(T, true, true);

               MeshBase mshb(T); create(mshb, ~0, false);
             //if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData(); don't free here because skinning information may be needed for Mesh-Skeleton link (in Mesh.loadData)
               if(f.ok())return true;
            }
         }else if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}

Bool MeshRender::save(File &f)C
{
   f.putUInt(CC4_MSHR);
   return saveData(f);
}
Bool MeshRender::load(File &f)
{
   if(f.getUInt()==CC4_MSHR)return loadData(f);
   del(); return false;
}

Bool MeshRender::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool MeshRender::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
// MESH PART
/******************************************************************************/
Bool MeshPart::saveData(File &f, CChar *path)C
{
   f.cmpUIntV(7);
   f.putStr(name).putMulti(part_flag, _vtx_heightmap, _draw_mask, _draw_mask_enum_id);
   f.putAsset(_materials[0].id());
   f.putAsset(_materials[1].id());
   f.putAsset(_materials[2].id());
   if(_variation .save    (f, path))
   if(_variations.save    (f, path))
   if( base      .saveData(f))
   if( render    .saveData(f))
      return f.ok();
   return false;
}
static void SetOldFlag(MeshPart &part, Byte flag)
{
   if(flag&0x1)part.part_flag|=MSHP_NO_PHYS_BODY;
 //if(flag&0x2)floor
   if(flag&0x4)part.part_flag|=MSHP_HIDDEN;
}
Bool MeshPart::loadData(File &f, CChar *path)
{
   del(); switch(f.decUIntV()) // version
   {
      case 7:
      {
         f.getStr(name).getMulti(part_flag, _vtx_heightmap, _draw_mask, _draw_mask_enum_id);
        _materials[0].require(f.getAssetID(), path);
        _materials[1].require(f.getAssetID(), path);
        _materials[2].require(f.getAssetID(), path);
         if(_variation .load(f, path))
         if(_variations.load(f, path))
         if( base      .loadData(f))
         if( render    .loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 6:
      {
         f._getStr2(name).getMulti(part_flag, _vtx_heightmap, _draw_mask, _draw_mask_enum_id);
        _materials[0].require(f.getAssetID(), path);
        _materials[1].require(f.getAssetID(), path);
        _materials[2].require(f.getAssetID(), path);
         if(_variation .load(f, path))
         if(_variations.load(f, path))
         if( base      .loadData(f))
         if( render    .loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 5:
      {
         f>>name; f.getMulti(part_flag, _vtx_heightmap, _draw_mask, _draw_mask_enum_id);
        _materials[0].require(f.getAssetID(), path);
        _materials[1].require(f.getAssetID(), path);
        _materials[2].require(f.getAssetID(), path);
         if(_variation .load(f, path))
         if(_variations.load(f, path))
         if( base      .loadData(f))
         if( render    .loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 4:
      {
         f>>name; f.getMulti(part_flag, _vtx_heightmap, _draw_mask, _draw_mask_enum_id);
        _variation.material    .require(f._getStr(), path);
                  _materials[0].require(f._getStr(), path);
                  _materials[1].require(f._getStr(), path);
                  _materials[2].require(f._getStr(), path);
         if(base  .loadData(f))
         if(render.loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 3:
      {
         f>>name; f.skip(1); SetOldFlag(T, f.getByte()); f>>_vtx_heightmap;
        _variation.material    .require(f._getStr(), path);
                  _materials[0].require(f._getStr(), path);
                  _materials[1].require(f._getStr(), path);
                  _materials[2].require(f._getStr(), path);
         if(base  .loadData(f))
         if(render.loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 2:
      {
         f>>name; f.skip(1); SetOldFlag(T, f.getByte()); f.skip(1); f>>_vtx_heightmap; f.skip(20);
        _variation.material    .require(f._getStr(), path);
                  _materials[0].require(f._getStr(), path);
                  _materials[1].require(f._getStr(), path);
                  _materials[2].require(f._getStr(), path);
         if(base  .loadData(f))
         if(render.loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 1:
      {
         f>>name; f.skip(1); SetOldFlag(T, f.getByte()); f.skip(20);
        _variation.material    .require(f._getStr(), path);
                  _materials[0].require(f._getStr(), path);
                  _materials[1].require(f._getStr(), path);
                  _materials[2].require(f._getStr(), path);
         if(base  .loadData(f))
         if(render.loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;

      case 0:
      {
         Char8 name[16]; f>>name; f.skip(1); SetOldFlag(T, f.getByte()); f.skip(20); Set(T.name, name);
        _variation.material    .require(f._getStr8(), path);
                  _materials[0].require(f._getStr8(), path);
                  _materials[1].require(f._getStr8(), path);
                  _materials[2].require(f._getStr8(), path);
         if(base  .loadData(f))
         if(render.loadData(f))
            if(f.ok()){setUMM().setShader(0); return true;}
      }break;
   }
   del(); return false;
}
Bool MeshPart::loadOld(File &f, CChar *path)
{
   del();

   Char8 name[16]; f>>name; f.skip(1); SetOldFlag(T, f.getByte()); Set(T.name, name);
  _variation.material    .require(f._getStr8(), path);
            _materials[0].require(f._getStr8(), path);
            _materials[1].require(f._getStr8(), path);
            _materials[2].require(f._getStr8(), path);
   if(base  .loadData(f))
   if(render.loadData(f))
      if(f.ok()){setUMM().setShader(0); return true;}
   del(); return false;
}
Bool MeshPart::loadOld0(File &f, CChar *path)
{
   del();

   Char8 name[16], material[80];
   f>>name; f.skip(1); SetOldFlag(T, f.getByte()); f>>material;
   Set(T.name, name); _variation.material.require(material, path);
   if(base  .loadData(f))
   if(render.loadData(f))
      if(f.ok()){setUMM().setShader(0); return true;}
   del(); return false;
}
Bool MeshPart::save(File &f, CChar *path)C {   f.putUInt(   CC4_MSHP); return saveData(f, path);                     }
Bool MeshPart::load(File &f, CChar *path)  {if(f.getUInt()==CC4_MSHP)  return loadData(f, path); del(); return false;}
/******************************************************************************/
// MSHL
/******************************************************************************/
Bool MeshLod::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(0), Int(parts.elms()), dist2); // version
   FREPA(T)if(!parts[i].saveData(f, path))return false;
   return f.ok();
}
Bool MeshLod::loadData(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         create(f.getInt());
         f>>dist2;
         FREPA(T)if(!parts[i].loadData(f, path))goto error;
         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
Bool MeshLod::loadOld(File &f, CChar *path)
{
   create(f.getInt());
   FREPA(T)if(!parts[i].loadOld(f, path))goto error;
   if(f.ok())return true;
error:
   del(); return false;
}
Bool MeshLod::save(File &f, CChar *path)C {   f.putUInt(   CC4_MSHL); return saveData(f);                     }
Bool MeshLod::load(File &f, CChar *path)  {if(f.getUInt()==CC4_MSHL)  return loadData(f); del(); return false;}
/******************************************************************************/
// MESH
/******************************************************************************/
Bool Mesh::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(8), ext, lod_center); // version
   if(_bone_map  .save(f))
   if(_variations.save(f))
   {
      f.cmpUIntV(_lods.elms()); FREP(lods())if(!lod(i).saveData(f, path))return false;
      f.putAsset(Enums    .id(drawGroupEnum())); // save after LOD's, so when loading and setting 'drawGroupEnum' it can operate on LOD's
      f.putAsset(Skeletons.id(skeleton     ())); // save after LOD's, so when loading and setting 'skeleton'      it can operate on LOD's
      return f.ok();
   }
   return false;
}
Bool Mesh::loadData(File &f, CChar *path)
{
   Box box; del(); switch(f.decUIntV()) // version
   {
      case 8:
      {
         f.getMulti(ext, lod_center);

         if(!_bone_map  .load(f))goto error;
         if(!_variations.load(f))goto error;
        _lods.setNum(f.decUIntV()); FREP(lods())if(!lod(i).loadData(f, path))goto error;

         drawGroupEnum(Enums    (f.getAssetID(), path), false);
         skeleton     (Skeletons(f.getAssetID(), path));
      }break;

      case 7:
      {
         f.getMulti(box, lod_center); ext=box;

        _lods.setNum(f.decUIntV()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
         if(!_bone_map  .load(f))goto error;
         if(!_variations.load(f))goto error;

         skeleton     (Skeletons(f.getAssetID(), path));
         drawGroupEnum(Enums    (f.getAssetID(), path), false);
      }break;

      case 6:
      {
         f.getMulti(box, lod_center); ext=box;

        _lods.setNum(f.decUIntV()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
         if(!_bone_map  .loadOld1(f))goto error;
         if(!_variations.load    (f))goto error;

         skeleton     (Skeletons(f.getAssetID(), path));
         drawGroupEnum(Enums    (f.getAssetID(), path), false);
      }break;

      case 5:
      {
         f.getMulti(box, lod_center); ext=box;

        _lods.setNum(f.getByte()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
         if(!_bone_map.loadOld(f))goto error;

         skeleton     (Skeletons(f._getStr(), path));
         drawGroupEnum(Enums    (f._getStr(), path), false);
      }break;

      case 4:
      {
         f>>box>>lod_center; ext=box;

        _lods.setNum(f.getByte()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
         if(!_bone_map.loadOld(f))goto error;

         skeleton(Skeletons(f._getStr(), path));
      }break;

      case 3:
      {
         f>>box>>lod_center; ext=box;
        _lods.setNum(f.getByte()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
      }break;

      case 2:
      {
         f.getByte();
         f>>box; ext=box; lod_center=ext.pos;
        _lods.setNum(f.getByte()); FREP(lods())if(!lod(i).loadData(f, path))goto error;
      }break;

      case 1:
      {
         f.getByte();
         f>>box; ext=box; lod_center=ext.pos;
        _lods.setNum(f.getByte()); FREP(lods())if(!lod(i).loadOld(f, path))goto error;
      }break;

      case 0:
      {
         f.getByte();
         create(f.getInt());
         f>>box; ext=box; lod_center=ext.pos;
         FREPA(parts)if(!parts[i].loadOld0(f))goto error;
      }break;

      default: goto error;
   }
   if(f.ok())
   {
      if(App.flag&APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA)freeOpenGLESData();
      REPAO(_lods).setShader(i+1); // by default all shaders were set with lod_index=0, so now reset them with correct index
      return true;
   }
error:;
   del(); return false;
}
/******************************************************************************/
Bool Mesh::save(File &f, CChar *path)C
{
   f.putUInt(CC4_MESH);
   return saveData(f,  path);
}
Bool Mesh::load(File &f, CChar *path)
{
   switch(f.getUInt()) // type of mesh
   {
      case CC4_MESH: return loadData(f, path);

      case CC4_MSHB: if(create(1).parts[0].base.loadData(f))
      {
         setBox();
         return true;
      }break;

      case CC4_MSHR: if(create(1).parts[0].render.loadData(f))
      {
         setBox();
         return true;
      }break;

      case CC4_MSHL: if(del().MeshLod::loadData(f, path))
      {
         setBox();
         return true;
      }break;

      case CC4_MSHG:
      {
         MeshGroup mshg; if(mshg.loadData(f, path))
         {
            create(mshg);
            return true;
         }
      }break;
   }
   del(); return false;
}

Bool Mesh::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Mesh::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
// MESH GROUP
/******************************************************************************/
Bool MeshGroup::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(3), ext); // version
   f.cmpUIntV(meshes.elms()); FREPA(T)if(!meshes[i].saveData(f, path))return false;
   return f.ok();
}
/******************************************************************************/
Bool MeshGroup::loadData(File &f, CChar *path)
{
   Box box; switch(f.decUIntV()) // version
   {
      case 3:
      {
         f>>ext;
         meshes.setNum(f.decUIntV()); FREPA(T)if(!meshes[i].loadData(f, path))goto error;
         if(f.ok())return true;
      }break;

      case 2:
      {
         f>>box; ext=box;
         meshes.setNum(f.decUIntV()); FREPA(T)if(!meshes[i].loadData(f, path))goto error;
         if(f.ok())return true;
      }break;

      case 1:
      {
         f>>box; ext=box;
         meshes.setNum(f.decUIntV()); FREPA(T)
         {
            Mesh &mesh=meshes[i];
            if(f.decUIntV()==0)if(mesh.loadData(f, path))
            {
               mesh.variationKeep(mesh.variationFind(f.getUInt()));
               continue;
            }
            goto error;
         }
         if(f.ok())return true;
      }break;

      case 0:
      {
         f.getByte(); // old U16 version part
         meshes.setNum(f.getInt());
         f>>box; ext=box;
         FREPA(T)if(!meshes[i].loadData(f, path))goto error;
         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
Bool MeshGroup::save(File &f, CChar *path)C
{
   f.putUInt(CC4_MSHG);
   return saveData(f,  path);
}
Bool MeshGroup::load(File &f, CChar *path)
{
   switch(f.getUInt())
   {
      case CC4_MSHG: return loadData(f, path);

      case CC4_MESH: if(del().meshes.New().loadData(f, path))
      {
         ext=meshes[0].ext;
         return true;
      }break;
   }
   del(); return false;
}
Bool MeshGroup::loadAdd(File &f, CChar *path)
{
   if(!meshes.elms())return load(f, path);

   MeshGroup mshg; if(mshg.load(f, path))
   {
      if(mshg.meshes.elms())
      {
         ext|=mshg.ext;
         Int start=meshes.addNum(mshg.meshes.elms());
         FREPA(mshg)Swap(meshes[start+i], mshg.meshes[i]);
      }
      return true;
   }
   return false;
}
Bool MeshGroup::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool MeshGroup::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
// MSHB TXT
/******************************************************************************
Bool MeshBase::saveTxt(FileText &f)C
{
   Int temp=PrecisionFlt; PrecisionFlt=9;
   if(vtxs ())f.put("vtxs  = ", vtxs ());
   if(edges())f.put("edges = ", edges());
   if(tris ())f.put("tris  = ", tris ());
   if(quads())f.put("quads = ", quads());

   if(vtx.pos     ()){f.putLine("VTX_POS"     ); f++; FREPA(vtx)f.put(S, vtx.pos     (i)); f--;}
   if(vtx.nrm     ()){f.putLine("VTX_NRM"     ); f++; FREPA(vtx)f.put(S, vtx.nrm     (i)); f--;}
   if(vtx.tan     ()){f.putLine("VTX_TAN"     ); f++; FREPA(vtx)f.put(S, vtx.tan     (i)); f--;}
   if(vtx.bin     ()){f.putLine("VTX_BIN"     ); f++; FREPA(vtx)f.put(S, vtx.bin     (i)); f--;}
   if(vtx.hlp     ()){f.putLine("VTX_HLP"     ); f++; FREPA(vtx)f.put(S, vtx.hlp     (i)); f--;}
   if(vtx.tex0    ()){f.putLine("VTX_TEX0"    ); f++; FREPA(vtx)f.put(S, vtx.tex0    (i)); f--;}
   if(vtx.tex1    ()){f.putLine("VTX_TEX1"    ); f++; FREPA(vtx)f.put(S, vtx.tex1    (i)); f--;}
   if(vtx.tex2    ()){f.putLine("VTX_TEX2"    ); f++; FREPA(vtx)f.put(S, vtx.tex2    (i)); f--;}
   if(vtx.matrix  ()){f.putLine("VTX_MATRIX"  ); f++; FREPA(vtx)f.put(S, vtx.matrix  (i)); f--;}
   if(vtx.blend   ()){f.putLine("VTX_BLEND"   ); f++; FREPA(vtx)f.put(S, vtx.blend   (i)); f--;}
   if(vtx.size    ()){f.putLine("VTX_SIZE"    ); f++; FREPA(vtx)f.put(S, vtx.size    (i)); f--;}
   if(vtx.material()){f.putLine("VTX_MATERIAL"); f++; FREPA(vtx)f.put(S, vtx.material(i)); f--;}
   if(vtx.color   ()){f.putLine("VTX_COLOR"   ); f++; FREPA(vtx)f.put(S, vtx.color   (i)); f--;}
   if(vtx.flag    ()){f.putLine("VTX_FLAG"    ); f++; FREPA(vtx)f.put(S, vtx.flag    (i)); f--;}

   if(edge.ind ()){f.putLine("EDGE_IND" ); f++; FREPA(edge)f.put(S, edge.ind (i)); f--;}
   if(edge.nrm ()){f.putLine("EDGE_NRM" ); f++; FREPA(edge)f.put(S, edge.nrm (i)); f--;}
   if(edge.flag()){f.putLine("EDGE_FLAG"); f++; FREPA(edge)f.put(S, edge.flag(i)); f--;}
   if(edge.id  ()){f.putLine("EDGE_ID"  ); f++; FREPA(edge)f.put(S, edge.id  (i)); f--;}

   if(tri.ind ()){f.putLine("TRI_IND" ); f++; FREPA(tri)f.put(S, tri.ind (i)); f--;}
   if(tri.nrm ()){f.putLine("TRI_NRM" ); f++; FREPA(tri)f.put(S, tri.nrm (i)); f--;}
   if(tri.flag()){f.putLine("TRI_FLAG"); f++; FREPA(tri)f.put(S, tri.flag(i)); f--;}
   if(tri.id  ()){f.putLine("TRI_ID"  ); f++; FREPA(tri)f.put(S, tri.id  (i)); f--;}

   if(quad.ind ()){f.putLine("QUAD_IND" ); f++; FREPA(quad)f.put(S, quad.ind (i)); f--;}
   if(quad.nrm ()){f.putLine("QUAD_NRM" ); f++; FREPA(quad)f.put(S, quad.nrm (i)); f--;}
   if(quad.flag()){f.putLine("QUAD_FLAG"); f++; FREPA(quad)f.put(S, quad.flag(i)); f--;}
   if(quad.id  ()){f.putLine("QUAD_ID"  ); f++; FREPA(quad)f.put(S, quad.id  (i)); f--;}

   PrecisionFlt=temp;
   return true;
}
Bool MeshBase::loadTxt(FileText &f)
{
   del(); for(; f.level(); )
   {
      if(f.cur("vtxs" )){exclude( VTX_ALL); f.get(vtx ._elms);}else
      if(f.cur("edges")){exclude(EDGE_ALL); f.get(edge._elms);}else
      if(f.cur("tris" )){exclude( TRI_ALL); f.get(tri ._elms);}else
      if(f.cur("quads")){exclude(QUAD_ALL); f.get(quad._elms);}else

      if(f.cur("VTX_POS"     ) && f.getIn()){include(VTX_POS     ); FREPA(vtx)f.get(vtx.pos     (i)); f.getOut();}else
      if(f.cur("VTX_NRM"     ) && f.getIn()){include(VTX_NRM     ); FREPA(vtx)f.get(vtx.nrm     (i)); f.getOut();}else
      if(f.cur("VTX_TAN"     ) && f.getIn()){include(VTX_TAN     ); FREPA(vtx)f.get(vtx.tan     (i)); f.getOut();}else
      if(f.cur("VTX_BIN"     ) && f.getIn()){include(VTX_BIN     ); FREPA(vtx)f.get(vtx.bin     (i)); f.getOut();}else
      if(f.cur("VTX_HLP"     ) && f.getIn()){include(VTX_HLP     ); FREPA(vtx)f.get(vtx.hlp     (i)); f.getOut();}else
      if(f.cur("VTX_TEX0"    ) && f.getIn()){include(VTX_TEX0    ); FREPA(vtx)f.get(vtx.tex0    (i)); f.getOut();}else
      if(f.cur("VTX_TEX1"    ) && f.getIn()){include(VTX_TEX1    ); FREPA(vtx)f.get(vtx.tex1    (i)); f.getOut();}else
      if(f.cur("VTX_TEX2"    ) && f.getIn()){include(VTX_TEX2    ); FREPA(vtx)f.get(vtx.tex2    (i)); f.getOut();}else
      if(f.cur("VTX_MATRIX"  ) && f.getIn()){include(VTX_MATRIX  ); FREPA(vtx)f.get(vtx.matrix  (i)); f.getOut();}else
      if(f.cur("VTX_BLEND"   ) && f.getIn()){include(VTX_BLEND   ); FREPA(vtx)f.get(vtx.blend   (i)); f.getOut();}else
      if(f.cur("VTX_SIZE"    ) && f.getIn()){include(VTX_SIZE    ); FREPA(vtx)f.get(vtx.size    (i)); f.getOut();}else
      if(f.cur("VTX_MATERIAL") && f.getIn()){include(VTX_MATERIAL); FREPA(vtx)f.get(vtx.material(i)); f.getOut();}else
      if(f.cur("VTX_COLOR"   ) && f.getIn()){include(VTX_COLOR   ); FREPA(vtx)f.get(vtx.color   (i)); f.getOut();}else
      if(f.cur("VTX_FLAG"    ) && f.getIn()){include(VTX_FLAG    ); FREPA(vtx)f.get(vtx.flag    (i)); f.getOut();}else

      if(f.cur("EDGE_IND") && f.getIn()){include(EDGE_IND); FREPA(edge)f.get(edge.ind(i)); f.getOut();}else
      if(f.cur("EDGE_NRM") && f.getIn()){include(EDGE_NRM); FREPA(edge)f.get(edge.nrm(i)); f.getOut();}else
      if(f.cur("EDGE_ID" ) && f.getIn()){include(EDGE_ID ); FREPA(edge)f.get(edge.id (i)); f.getOut();}else

      if(f.cur("TRI_IND") && f.getIn()){include(TRI_IND); FREPA(tri)f.get(tri.ind(i)); f.getOut();}else
      if(f.cur("TRI_NRM") && f.getIn()){include(TRI_NRM); FREPA(tri)f.get(tri.nrm(i)); f.getOut();}else
      if(f.cur("TRI_ID" ) && f.getIn()){include(TRI_ID ); FREPA(tri)f.get(tri.id (i)); f.getOut();}else

      if(f.cur("QUAD_IND") && f.getIn()){include(QUAD_IND); FREPA(quad)f.get(quad.ind(i)); f.getOut();}else
      if(f.cur("QUAD_NRM") && f.getIn()){include(QUAD_NRM); FREPA(quad)f.get(quad.nrm(i)); f.getOut();}else
      if(f.cur("QUAD_ID" ) && f.getIn()){include(QUAD_ID ); FREPA(quad)f.get(quad.id (i)); f.getOut();}else

      if(f.cur("EDGE_FLAG") && f.getIn()){include(EDGE_FLAG); FREPA(edge)edge.flag(i)=f.getUInt(); f.getOut();}else
      if(f.cur( "TRI_FLAG") && f.getIn()){include( TRI_FLAG); FREPA(tri )tri .flag(i)=f.getUInt(); f.getOut();}else
      if(f.cur("QUAD_FLAG") && f.getIn()){include(QUAD_FLAG); FREPA(quad)quad.flag(i)=f.getUInt(); f.getOut();}
   }
   return false;
}
Bool MeshBase::saveTxt(C Str &name)C
{
   FileText f; if(f.write(name))
   {
      f.putLine("MeshBase");
      f++;
      Bool ok=saveTxt(f);
      f--;
      return ok;
   }
   return false;
}
Bool MeshBase::loadTxt(C Str &name)
{
   FileText f; if(f.read(name))for(; f.level(); )if(f.cur("MeshBase") && f.getIn())return loadTxt(f);
   del(); return false;
}
/******************************************************************************/
// MSHP TXT
/******************************************************************************
Bool MeshPart::saveTxt(FileText &f, CChar *path)C
{
                   f.putName("name        = ", name);
                   f.put    ("flag        = ", flag);
   if(_material[0])f.putName("material[0] = ", _material[0].name(path));
   if(_material[1])f.putName("material[1] = ", _material[1].name(path));
   if(_material[2])f.putName("material[2] = ", _material[2].name(path));
   if(_material[3])f.putName("material[3] = ", _material[3].name(path));
                   f.putLine("MeshBase");
   f++;
   Bool ok=base.saveTxt(f);
   f--;
   return ok;
}
Bool MeshPart::loadTxt(FileText &f, CChar *path)
{
   del();
   for(; f.level(); )
   {
      if(f.cur("name"))Set(name, f.getName());else
      if(f.cur("flag"))    flag= f.getInt () ;else
      if(f.cur("material[0]") || f.cur("material"))_material[0].require(f.getName(), path);else
      if(f.cur("material[1]")                     )_material[1].require(f.getName(), path);else
      if(f.cur("material[2]")                     )_material[2].require(f.getName(), path);else
      if(f.cur("material[3]")                     )_material[3].require(f.getName(), path);else
      if(f.cur("MeshBase") && f.getIn())base.loadTxt(f);
   }
   setUMM().setShader(0);
   return true;
}
/******************************************************************************/
// MESH TXT
/******************************************************************************
void Mesh::saveTxt(FileText &f, CChar *path)C
{
   f.put("parts      = ", parts.elms());
   f.put("box        = ", box         );
   f.put("lod_center = ", lod_center  );
   FREPA(parts)
   {
      f.putLine("MeshPart");
      f++;
      parts[i].saveTxt(f, path);
      f--;
   }
}
/******************************************************************************
void Mesh::loadTxt(FileText &f, CChar *path)
{
   del();

   Bool is_box       =false,
        is_lod_center=false;
   for(; f.level(); )
   {
      if(f.cur("box"       )){f.get(box       ); is_box       =true;}else
      if(f.cur("lod_center")){f.get(lod_center); is_lod_center=true;}else
      if(f.cur("MeshPart"  ) && f.getIn())parts.New().loadTxt(f, path);
   }
   if(!is_box       )setBox();
   if(!is_lod_center)lod_center=box.center();
}
/******************************************************************************
Bool Mesh::saveTxt(C Str &name)C
{
   FileText f; if(f.write(name))
   {
      f.putLine("Mesh");
      f++;
      saveTxt(f, _GetPath(name));
      f--;
      return true;
   }
   return false;
}
/******************************************************************************
Bool Mesh::loadTxt(C Str &name)
{
   FileText f; if(f.read(name))for(; f.level(); )if(f.cur("Mesh") && f.getIn())
   {
      loadTxt(f, _GetPath(name));
      return true;
   }
   del(); return false;
}
/******************************************************************************/
// MSHG TXT
/******************************************************************************
void MeshGroup::saveTxt(FileText &f, CChar *path)C
{
   f.put("meshes = ", meshes.elms());
   f.put("box    = ", box          );
   FREPA(T)
   {
      f.putLine("Mesh");
      f++;
      meshes[i].saveTxt(f, path);
      f--;
   }
}
/******************************************************************************
void MeshGroup::loadTxt(FileText &f, CChar *path)
{
   del();

   for(; f.level(); )
   {
      if(f.cur("Mesh") && f.getIn())meshes.New().loadTxt(f, path);
   }
   setBox(false);
}
/******************************************************************************
Bool MeshGroup::saveTxt(C Str &name)C
{
   FileText f; if(f.write(name))
   {
      f.putLine("MeshGroup");
      f++;
      saveTxt(f, _GetPath(name));
      f--;
      return true;
   }
   return false;
}
/******************************************************************************
Bool MeshGroup::loadTxt(C Str &name)
{
   FileText f; if(f.read(name))for(; f.level(); )if(f.cur("MeshGroup") && f.getIn())
   {
      loadTxt(f, _GetPath(name));
      return true;
   }
   del(); return false;
}
/******************************************************************************/
void MeshBase  ::operator=(C Str &name) {if(!load(name))Exit(MLT(S+"Can't load mesh \""+name+"\"", PL,S+u"Nie można wczytać modelu \""+name+"\""));}
void MeshRender::operator=(C Str &name) {if(!load(name))Exit(MLT(S+"Can't load mesh \""+name+"\"", PL,S+u"Nie można wczytać modelu \""+name+"\""));}
void Mesh      ::operator=(C Str &name) {if(!load(name))Exit(MLT(S+"Can't load mesh \""+name+"\"", PL,S+u"Nie można wczytać modelu \""+name+"\""));}
void MeshGroup ::operator=(C Str &name) {if(!load(name))Exit(MLT(S+"Can't load mesh \""+name+"\"", PL,S+u"Nie można wczytać modelu \""+name+"\""));}
/******************************************************************************/
// IMPORT
/******************************************************************************/
Bool Import(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, MemPtr<Str> bone_names, Bool all_nodes_as_bones)
{
   if(mesh    )mesh    ->del();
   if(skeleton)skeleton->del();
   animations         .clear();
   materials          .clear();
   part_material_index.clear();
   bone_names         .clear();

   Str ext=GetExt(name);
   if(ext=="fbx"  )return ImportFBX(name, mesh, skeleton, animations, materials, part_material_index, bone_names, all_nodes_as_bones);
   if(ext=="obj"  )return ImportOBJ(name, mesh,                       materials, part_material_index);
   if(ext=="mesh" || ext=="mshb" || ext=="mshr" || ext=="mshg")
   {
      if(mesh && mesh->load(name))
      {
         if(mesh->skeleton())if(CChar *skel_name=Skeletons.name(mesh->skeleton())){mesh->skeleton()->load(skel_name); mesh->skeleton(mesh->skeleton());} // because Import is used to load/reload, and because Skeleton is cached, then force reload skeleton from file, because it's possible that skel file was changed, however due to caching it will not be reloaded
         if(skeleton && mesh->skeleton())*skeleton=*mesh->skeleton();
         if(materials || part_material_index)
         {
            Memt<Material*> mesh_mtrls;
            FREPD(l, mesh->lods()) // gather all materials
            {
             C MeshLod &lod=mesh->lod(l);
               FREPA(lod)if(Material *m=lod.parts[i].material()())mesh_mtrls.include(m);
            }
            if(materials)FREPA(mesh_mtrls) // create 'XMaterial's
            {
               XMaterial &xm=materials.New();
               xm.createFrom(*mesh_mtrls[i]);
               xm.name=GetBaseNoExt(Materials.name(mesh_mtrls[i]));
            }
            if(part_material_index)FREPD(l, mesh->lods()) // set 'part_material_index'
            {
             C MeshLod &lod=mesh->lod(l);
               FREPA(lod)part_material_index.add(mesh_mtrls.find(lod.parts[i].material()()));
            }
         }
         return true;
      }
   }else
   if(ext=="skel"){if(skeleton)return skeleton->load(name);}else
   if(ext=="anim"){if(animations && animations.New().anim.load(name)){animations.last().name=GetBaseNoExt(name); return true;}}else
   if(ext=="mtrl")
   {
      Material mtrl; if(materials)if(mtrl.load(name))
      {
         XMaterial &m=materials.New();
         m.createFrom(mtrl);
         m.name=GetBaseNoExt(name);
         return true;
      }
   }else
   if(ext=="dae")
   {
      if(ImportDAE(name, mesh, skeleton, animations ? &animations.New() : null, materials, part_material_index))
      {
         if(animations.elms() && !animations.first().anim.is())animations.clear();
         return true;
      }
   }else
   if(ext=="b3d")
   {
      if(ImportB3D(name, mesh, skeleton, animations ? &animations.New() : null, materials, part_material_index))
      {
         if(animations.elms() && !animations.first().anim.is())animations.clear();
         return true;
      }
   }else
   if(ext=="ms3d")
   {
      if(ImportMS3D(name, mesh, skeleton, animations ? &animations.New() : null, materials, part_material_index))
      {
         if(animations.elms() && !animations.first().anim.is())animations.clear();
         return true;
      }
   }else
   if(ext=="bvh")
   {
      if(ImportBVH(name, skeleton, animations ? &animations.New() : null))
      {
         if(animations.elms() && !animations.first().anim.is())animations.clear();
         return true;
      }
   }else
   if(ext=="ase"  ){return ImportASE(name, mesh,                       materials, part_material_index);}else
   if(ext=="3ds"  ){return Import3DS(name, mesh,                       materials, part_material_index);}else
   if(ext=="psk"  ){return ImportPSK(name, mesh, skeleton,             materials, part_material_index);}else
   if(ext=="psa"  ){return ImportPSA(name,       skeleton, animations                                );}else
   if(ext=="ascii"){return ImportXPS(name, mesh, skeleton,             materials, part_material_index);}
   return false;
}
/******************************************************************************
void RemoveNubBones(Mesh *mesh, Skeleton &skeleton, C MemPtr<Animation*> &animations)
{
   Memt<Byte, 256> old_to_new;
   REPA(skeleton.bones)
   {
      SkelBone &bone=skeleton.bones[i]; if(Ends(bone.name, "Nub") && !bone.children_num)
      {
         // remove animation bones
         REPAD(a, animations)
            if(Animation *anim=animations[a])
               REPA(anim->bones)
                  if(Equal(anim->bones[a].name, bone.name))anim->bones.remove(a, true);

         // remove skeleton bone
         if(skeleton.removeBone(i, old_to_new))
         {
            // adjust mesh vertex skinning
            if(mesh)mesh->boneRemap(old_to_new);
         }
      }
   }
}
/******************************************************************************/
void CleanMesh(Mesh &mesh)
{
   REPD(l, mesh.lods())
   {
      MeshLod &lod=mesh.lod(l);
      REPA(lod)
      {
         MeshPart &part=lod.parts[i];
         MeshBase &base=part.base;
         if(base.vtx.color())REPA(base.vtx)
         {
          C Color &c=base.vtx.color(i);
            if(c.r<253 || c.g<253 || c.b<253)goto valid_color;
         }
         base.exclude(VTX_COLOR);
      valid_color:;
      }
   }
}
/******************************************************************************/
void ShutMesh() {Meshes.del();}
/******************************************************************************/
}
/******************************************************************************/
