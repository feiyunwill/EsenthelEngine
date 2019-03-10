/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Bool ImportXPS(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh    )mesh    ->del();
   if(skeleton)skeleton->del();
   materials          .clear();
   part_material_index.clear();

   FileText f; if(f.read(name))
   {
      Str s;
      MemtN<IndexWeight, 256> skin;
      MemtN<Byte       , 256> old_to_new;
      f.fullLine(s); Int bones=TextInt(s); if(bones<0)goto error;
      Skeleton temp, *skel=(skeleton ? skeleton : mesh ? &temp : null); // if skel not specified, but we want mesh, then we have to process it
      if(skel)skel->bones.setNum(bones);
      FREP(bones)
      {
         SkelBone *bone=(skel ? &skel->bones[i] : null);
         f.fullLine(s); if(bone)Set(bone->name, s);
         f. getLine(s); if(bone){Int parent=TextInt(s); bone->parent=(InRange(parent, skel->bones) ? parent : 0xFF);}
         f. getLine(s); if(bone)bone->pos=TextVec(s);
      }
      if(skel)
      {
         skel->mirrorX().sortBones(old_to_new).setBoneTypes(); if(VIRTUAL_ROOT_BONE)REPAO(old_to_new)++; // 'mirrorX' must be called before 'setBoneTypes', 'sortBones' must be called before 'setBoneTypes' and 'SetSkin'
      }
      f.getLine(s); Int parts=TextInt(s); if(parts<0)goto error;
      if(part_material_index){part_material_index.setNum(parts); REPAO(part_material_index)=i;}
      if(materials          ){          materials.setNum(parts); REPAO(materials          ).name=i;} // name is needed so Editor can create multiple materials instead of trying to reuse just one
      if(mesh               )         mesh->parts.setNum(parts);
      if(mesh || materials  )FREP(parts)
      {
         MeshPart *part=(mesh ? &mesh->parts[i] : null);
         f.fullLine(s); if(part)Set(part->name, s);
         f. getLine(s); Int uv_layers=TextInt(s); if(uv_layers<0)goto error;
         f. getLine(s); Int  textures=TextInt(s); if(textures <0)goto error;
         FREPD(t, textures)
         {
            f.fullLine(s); // tex file name
            if(materials)
            {
               XMaterial &mtrl=materials[i]; switch(t)
               {
                  case 0: mtrl.   color_map=s; break;
                  case 1: mtrl.specular_map=s; break;
                  case 2: mtrl.  normal_map=s; break;
                  case 3: mtrl.specular_map=s; break;
               }
            }
            f.getLine(s); Int uv_layer_index=TextInt(s); if(uv_layer_index<0)goto error;
         }
         if(materials)materials[i].fixPath(GetPath(name));
         f.getLine(s); Int vtxs=TextInt(s); if(vtxs<0)goto error;
         if(part)
         {
            MeshBase &base=part->base.create(vtxs, 0, 0, 0, VTX_POS|VTX_NRM|VTX_COLOR|(bones ? VTX_SKIN : 0)|((uv_layers>=1) ? VTX_TEX0 : 0)|((uv_layers>=2) ? VTX_TEX1 : 0));
            FREP(vtxs)
            {
               f.getLine(s); base.vtx.pos  (i)= TextVec  (s);
               f.getLine(s); base.vtx.nrm  (i)=!TextVec  (s);
               f.getLine(s); base.vtx.color(i)= TextColor(s);
               if( uv_layers>=1){f. getLine(s); base.vtx.tex0(i)=TextVec2(s);}
               if( uv_layers>=2){f. getLine(s); base.vtx.tex1(i)=TextVec2(s);}
               REP(uv_layers- 2) f.skipLine( ); // skip unprocessed
               if(bones>0)
               {
                  f.getLine(s); VecB4 m=TextVecB4(s);
                  f.getLine(s); Vec4  b=TextVec4 (s);
                  FREPA(m)if(b.c[i] && InRange(m.c[i], old_to_new))skin.New().set(old_to_new[m.c[i]], b.c[i]/* /255.0f not needed because weight is normalized in 'SetSkin' */);
                  SetSkin(skin, base.vtx.matrix(i), base.vtx.blend(i), skel); skin.clear();
               }
            }
         }else REP(vtxs*(3+uv_layers+(bones>0)*2))f.skipLine();

         f.getLine(s); Int tris=TextInt(s); if(tris<0)goto error;
         if(part)
         {
            MeshBase &base=part->base; base.tri._elms=tris; base.include(TRI_IND);
            Bool invalid=false;
            FREP(tris)
            {
               f.getLine(s); VecI &t=base.tri.ind(i); t=TextVecI(s).reverse(); REPA(t)if(!InRange(t.c[i], vtxs)){t.zero(); invalid=true; break;}
            }
            if(invalid)base.removeDegenerateFaces(0);
         }else REP(tris)f.skipLine();
      }

      if(mesh    ){mesh->mirrorX().skeleton(skel).skeleton(null).setBox(); CleanMesh(*mesh);}
      if(skeleton){skel->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}
      return true;
   }
error:
   if(mesh)mesh->del();
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
