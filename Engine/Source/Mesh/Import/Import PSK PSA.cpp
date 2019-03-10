/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace PSK_PSA{ // so local structures are unique
/******************************************************************************/
#pragma pack(push, 4) // PSK/PSA require 4 byte packing
/******************************************************************************/
struct VChunkHeader
{
   Char8 ChunkID[20]; // String ID of up to 19 chars (usually zero-terminated)
   Int	TypeFlag;    // Flags/reserved
   Int   DataSize;    // Size per struct following
   Int   DataCount;   // Number of structs
};
struct VJointPos // A bone: an orientation, and a position, all relative to their parent.
{
   Quaternion Orientation;
   Vec        Position;

   Flt Length; //  For collision testing / debugging drawing.  (unused)
   Flt XSize;
   Flt YSize;
   Flt ZSize;
};
struct VBone // Raw data bone
{
   Char8     Name[64];    //
   UInt      Flags;       // Reserved
   Int 	    NumChildren; // Children  (not used)
   Int       ParentIndex; // 0/null if this is the root bone
   VJointPos BonePos;     // Reference position
};
struct AnimInfoBinary
{
   Char8 Name [64]; // Animation's name
   Char8 Group[64]; // Animation's group name	

   Int TotalBones;          // TotalBones * NumRawFrames is number of animation keys to digest.
   Int RootInclude;         // 0 none 1 included 	(unused)
   Int KeyCompressionStyle; // Reserved: variants in tradeoffs for compression.
   Int KeyQuotum;           // Max key quotum for compression	
   Flt KeyReduction;        // desired 
   Flt TrackTime;           // explicit - can be overridden by the animation rate
   Flt AnimRate;            // frames per second.
   Int StartBone;           // - Reserved: for partial animations (unused)
   Int FirstRawFrame;       //
   Int NumRawFrames;        // NumRawFrames and AnimRate dictate tracktime...
};
struct VQuatAnimKey // An animation key.
{
   Vec        Position;    // Relative to parent.
   Quaternion Orientation; // Relative to parent.
   Flt        Time;        // The duration until the next key (end key wraps to first...)
};
struct VPoint // Points: regular FVectors 
{
   Vec Point; 
};
struct VVertex // Vertex with texturing info, akin to Hoppe's 'Wedge' concept - import only.
{
   UShort PointIndex; // Index into the 3d point table.
   Flt    U, V;       // Texture U, V coordinates.
   Byte   MatIndex;   // At runtime, this one will be implied by the face that's pointing to us.
   Byte   Reserved;   // Top secret.
};
struct VTriangle // Textured triangle.
{
   UShort WedgeIndex[3];	 // Point to three vertices in the vertex list.
   Byte   MatIndex;	       // Materials can be anything.
   Byte   AuxMatIndex;     // Second material (unused).
   UInt   SmoothingGroups; // 32-bit flag for smoothing groups.
};
struct VMaterial // Raw data material
{
   Char8 MaterialName[64];
   Int	TextureIndex;  // Texture index ('multiskin index')
   UInt  PolyFlags;     // ALL poly's with THIS material will have this flag.
   Int	AuxMaterial;   // Reserved: index into another material, eg. detailtexture/shininess/whatever.
   UInt  AuxFlags;      // Reserved: auxiliary flags 
   Int	LodBias;       // Material-specific lod bias (unused)
   Int	LodStyle;      // Material-specific lod style (unused)
};
struct VRawBoneInfluence // Raw data bone influence. Just weight, vertex, and Bone, sorted later.
{
   Flt Weight;
   Int PointIndex;
   Int BoneIndex;
};
/******************************************************************************/
static void CreateSkeleton(Skeleton &skeleton, Memc<VBone> &bones)
{
   skeleton.bones.setNum(bones.elms());
   FREPA(bones)
   {
      SkelBone &sbon=skeleton.bones[i];
      VBone    &ubon=bones[i];

      sbon.parent=((i && InRange(ubon.ParentIndex, skeleton.bones)) ? ubon.ParentIndex : 0xFF);

      if(!i)ubon.BonePos.Orientation.w*=-1; Matrix3 orient=ubon.BonePos.Orientation;
      if(!i)ubon.BonePos.Orientation.w*=-1;

      sbon.pos =ubon.BonePos.Position;
      sbon.dir =orient.z;
      sbon.perp=orient.y;

      if(InRange(sbon.parent, skeleton.bones))sbon*=(Matrix)skeleton.bones[sbon.parent];

      Set(sbon.name, ubon.Name); REP(Length(sbon.name))if(sbon.name[i]==' ')sbon.name[i]=0;else break; // remove useless spaces
   }

   FREPA(bones)skeleton.bones[i].dir=skeleton.bones[i].cross(); // do this at the end, because sbon transformation by matrix relies on original value
   skeleton.setBoneLengths();
}
/******************************************************************************/
#pragma pack(pop)
/******************************************************************************/
} // namespace PSK_PSA
/******************************************************************************/
Bool ImportPSK(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh    )mesh    ->del();
   if(skeleton)skeleton->del();
   materials          .clear();
   part_material_index.clear();

   using namespace PSK_PSA;

   File f; if(f.readTry(name))
   {
      VChunkHeader   general_header; f>>  general_header;
      VChunkHeader    points_header; f>>   points_header; Memc<VPoint           >  points; if(   points_header.DataCount<0 ||    points_header.DataCount && SIZE(VPoint           )!=   points_header.DataSize)return false;  points.setNum(   points_header.DataCount); f.getN( points.data(),  points.elms());
      VChunkHeader   vertexs_header; f>>  vertexs_header; Memc<VVertex          > vertexs; if(  vertexs_header.DataCount<0 ||   vertexs_header.DataCount && SIZE(VVertex          )!=  vertexs_header.DataSize)return false; vertexs.setNum(  vertexs_header.DataCount); f.getN(vertexs.data(), vertexs.elms());
      VChunkHeader     faces_header; f>>    faces_header; Memc<VTriangle        >   faces; if(    faces_header.DataCount<0 ||     faces_header.DataCount && SIZE(VTriangle        )!=    faces_header.DataSize)return false;   faces.setNum(    faces_header.DataCount); f.getN(  faces.data(),   faces.elms());
      VChunkHeader materials_header; f>>materials_header; Memc<VMaterial        >   mtrls; if(materials_header.DataCount<0 || materials_header.DataCount && SIZE(VMaterial        )!=materials_header.DataSize)return false;   mtrls.setNum(materials_header.DataCount); f.getN(  mtrls.data(),   mtrls.elms());
      VChunkHeader     bones_header; f>>    bones_header; Memc<VBone            >   bones; if(    bones_header.DataCount<0 ||     bones_header.DataCount && SIZE(VBone            )!=    bones_header.DataSize)return false;   bones.setNum(    bones_header.DataCount); f.getN(  bones.data(),   bones.elms());
      VChunkHeader     skins_header; f>>    skins_header; Memc<VRawBoneInfluence>   skins; if(    skins_header.DataCount<0 ||     skins_header.DataCount && SIZE(VRawBoneInfluence)!=    skins_header.DataSize)return false;   skins.setNum(    skins_header.DataCount); f.getN(  skins.data(),   skins.elms());
      if(!f.ok())return false;

      // skeleton
      MemtN<Byte, 256> old_to_new;
      Skeleton temp, *skel=(skeleton ? skeleton : mesh ? &temp : null); // if skel not specified, but we want mesh, then we have to process it
      if(skel){CreateSkeleton(*skel, bones); skel->sortBones(old_to_new); if(VIRTUAL_ROOT_BONE)REPAO(old_to_new)++;} // 'sortBones' must be called before 'SetSkin'

      // mesh
      if(mesh)
      {
         // skinning
         Memc< Memc<IndexWeight> > vtx_skin; if(skins.elms())
         {
            vtx_skin.setNum(points.elms());
            REPA(skins)
            {
             C VRawBoneInfluence &skin=skins[i];
               if(InRange(skin.PointIndex, vtx_skin) && InRange(skin.BoneIndex, old_to_new))vtx_skin[skin.PointIndex].New().set(old_to_new[skin.BoneIndex], skin.Weight);
            }
         }

         MeshBase base(faces.elms()*3, 0, faces.elms(), 0, VTX_TEX0|(skins.elms() ? VTX_SKIN : 0)|TRI_ID);

         FREPA(faces)
         {
          C VTriangle &vtri =faces[i]; REPA(vtri.WedgeIndex)if(!InRange(vtri.WedgeIndex[i], vertexs))return false;
          C VVertex   &vvtx0=vertexs[vtri.WedgeIndex[0]],
                      &vvtx1=vertexs[vtri.WedgeIndex[1]],
                      &vvtx2=vertexs[vtri.WedgeIndex[2]];

            if(!InRange(vvtx0.PointIndex, points)
            || !InRange(vvtx1.PointIndex, points)
            || !InRange(vvtx2.PointIndex, points))return false;

            Int a=i*3, b=a+1, c=b+1;

            base.vtx.pos(a)=points[vvtx0.PointIndex].Point;
            base.vtx.pos(b)=points[vvtx1.PointIndex].Point;
            base.vtx.pos(c)=points[vvtx2.PointIndex].Point;

            base.vtx.tex0(a).set(vvtx0.U, vvtx0.V);
            base.vtx.tex0(b).set(vvtx1.U, vvtx1.V);
            base.vtx.tex0(c).set(vvtx2.U, vvtx2.V);

            if(skins.elms())
            {
               SetSkin(vtx_skin[vvtx0.PointIndex], base.vtx.matrix(a), base.vtx.blend(a), skel);
               SetSkin(vtx_skin[vvtx1.PointIndex], base.vtx.matrix(b), base.vtx.blend(b), skel);
               SetSkin(vtx_skin[vvtx2.PointIndex], base.vtx.matrix(c), base.vtx.blend(c), skel);
            }

            base.tri.id (i)=vtri.MatIndex;
            base.tri.ind(i).set(c, b, a); // face indexes need to be reversed
         }

         base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1).setVtxDup().setNormals().copyId(*mesh); // use small epsilon in case mesh is scaled down

         // material indexes
         if(part_material_index)FREPA(*mesh)part_material_index.add(i);
      }

      // materials
      if(materials)
      {
         FREPA(mtrls)
         {
            XMaterial &xm=materials.New();
            xm.name=mtrls[i].MaterialName;
         }
         FileText f; if(f.read(GetPath(name)+"\\X_ModelInfo_"+GetBaseNoExt(name)+".LOG"))for(; !f.end(); )
         {
            if(Starts(f.getLine(), "= materials ="))
            {
               Int index=-1;
               for(Str line; !f.end(); )
               {
                  f.getLine(line);
                  Int pos=TextPosI(line, "* Index:"); if(pos>=0)index++;
                      pos=TextPosI(line, "Original Bitmap:"); if(pos>=0 && InRange(index, materials))
                  {
                     Str tex=SkipStart(line()+pos, "Original Bitmap:");
                     pos=TextPosI(tex, "Path:"); if(pos>=0)tex.clip(pos);
                     materials[index].color_map=tex.removeOuterWhiteChars();
                     materials[index].fixPath(GetPath(name));
                  }
               }
            }
         }
      }

      // remove nub bones
    //if(skel)RemoveNubBones(mesh, *skel, null);

      // finalize
      if(skel    ) skel->rightToLeft().setBoneTypes(); // 'setBoneTypes' must be called after 'sortBones' and 'rightToLeft' and before 'Mesh.skeleton'
      if(mesh    ){mesh->rightToLeft().skeleton(skel).skeleton(null).setBox(); CleanMesh(*mesh);}
      if(skeleton){skel->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}

      return true;
   }
   return false;
}
Bool ImportPSA(C Str &name, Skeleton *skeleton, MemPtr<XAnimation> animations)
{
   if(skeleton)skeleton->del();
   animations.clear();

   using namespace PSK_PSA;

   File f; if(f.readTry(name))
   {
      VChunkHeader  general_header; f>> general_header;
      VChunkHeader    bones_header; f>>   bones_header; Memc<VBone         > bones; if(   bones_header.DataCount<0 ||    bones_header.DataCount && SIZE(VBone         )!=   bones_header.DataSize)return false; bones.setNum(   bones_header.DataCount); f.getN(bones.data(), bones.elms());
      VChunkHeader     anim_header; f>>    anim_header; Memc<AnimInfoBinary> anims; if(    anim_header.DataCount<0 ||     anim_header.DataCount && SIZE(AnimInfoBinary)!=    anim_header.DataSize)return false; anims.setNum(    anim_header.DataCount); f.getN(anims.data(), anims.elms());
      VChunkHeader raw_keys_header; f>>raw_keys_header; Memc<VQuatAnimKey  > keys ; if(raw_keys_header.DataCount<0 || raw_keys_header.DataCount && SIZE(VQuatAnimKey  )!=raw_keys_header.DataSize)return false;  keys.setNum(raw_keys_header.DataCount); f.getN( keys.data(),  keys.elms());
      if(!f.ok())return false;

      // create skeleton
      Skeleton temp, *skel=(skeleton ? skeleton : animations ? &temp : null); // if skel not specified, but we want animations, then we have to process it
      if(skel)
      {
         CreateSkeleton(*skel, bones);

         // create animations
         if(animations)FREPA(anims)
         {
          C AnimInfoBinary &anim=anims[i];
            if(anim.TotalBones==skel->bones.elms()
            && anim.FirstRawFrame>=0 // can be zero
            && anim.NumRawFrames>0 // if zero then ignore
            && (anim.FirstRawFrame+anim.NumRawFrames)*anim.TotalBones<=keys.elms()) // frames are in range
            {
               XAnimation &xanimation=animations.New();
                Animation & animation=xanimation.anim;

               xanimation.fps =anim.AnimRate;
               xanimation.name=anim.Name;
                animation.bones.setNum(skel->bones.elms());
               FREPA(skel->bones)
               {
                  Flt       time=0;
                  VBone    &ubon=          bones[i];
                  SkelBone &sbon=skel    ->bones[i];
                  AnimBone &abon=animation.bones[i]; abon.set(sbon.name);
                  Bool      parent      =(sbon.parent!=0xFF);
                  Matrix3   parent_matrix     , parent_matrix_inv,
                            parent_matrix_temp, parent_matrix_temp_inv; // this is skel.bone(sbon.parent) before changing the xz
                  if(parent)
                  {
                     parent_matrix     =skel->bones[sbon.parent];
                     parent_matrix_temp=Orient(-skel->bones[sbon.parent].cross(), skel->bones[sbon.parent].perp);
                     parent_matrix     .inverse(parent_matrix_inv     , true);
                     parent_matrix_temp.inverse(parent_matrix_temp_inv, true);
                  }

                  abon.poss.setNumZero(anim.NumRawFrames);
                  abon.orns.setNumZero(anim.NumRawFrames);

                  FREPD(k, anim.NumRawFrames)
                  {
                     VQuatAnimKey  &key=keys[(k+anim.FirstRawFrame)*anim.TotalBones + i];
                     AnimKeys::Pos &pos=abon.poss[k]; pos.time=time;
                     AnimKeys::Orn &orn=abon.orns[k]; orn.time=time;
                     animation.length(Max(animation.length(), time), false);

                     // orientation
                     {
                        // set rotation
                        orn.orn=sbon;

                        // animate
                        {
                           if(!i){ubon.BonePos.Orientation.w*=-1; key.Orientation.w*=-1;}
                           if(parent)orn.orn*=parent_matrix_temp_inv                                                   ; // transform bone from global space to local space relative to parent
                                     orn.orn*=GetTransform((Matrix3)ubon.BonePos.Orientation, (Matrix3)key.Orientation); // perform transformation from source to target matrix, looks like 'frame' is the target matrix
                           if(parent)orn.orn*=parent_matrix_temp                                                       ; // transform bone from local space to global space
                           if(!i){ubon.BonePos.Orientation.w*=-1; key.Orientation.w*=-1;}
                        }

                        //
                        if(parent)orn.orn*=parent_matrix_inv;
                     }

                     // position
                     {
                        pos.pos=key.Position-ubon.BonePos.Position;
                        if(parent)
                        {
                           pos.pos*=parent_matrix_temp;
                           pos.pos*=parent_matrix_inv ;
                        }
                     }

                     time+=key.Time;
                  }
               }

               // rescale in time
               if(xanimation.fps>0)animation.length(animation.length()/xanimation.fps, true);

               // override length
               if(anim.TrackTime>0)animation.length((xanimation.fps>0) ? anim.TrackTime/xanimation.fps : anim.TrackTime, false);

               // process
               animation.setTangents().removeUnused().rightToLeft(*skel).setRootMatrix();
            }else
            {
               // invalid data
            }
         }

         // remove nub bones
       //Memt<Animation*> anims; REPA(animations)anims.add(&animations[i].anim); RemoveNubBones(null, *skel, anims);

         skel->sortBones().rightToLeft().setBoneTypes();
         REPAO(animations).anim.setBoneTypeIndexesFromSkeleton(*skel);
         if(skeleton){skel->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}
      }

      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
