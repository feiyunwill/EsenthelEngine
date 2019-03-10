/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace MS3D{ // so that local structures are unique
/******************************************************************************

   AngleQuaternion   (x, y, z) works the same like Quaternion.set(-x,-y,-z)
   R_ConcatTransforms(a, b, c) works the same like c=b*a

   performed modifications on ms3d source files:
      removed #include <vector>

      std::vector -> std__vector

	   msModel::Load(const char *filename) -> Load(C Str &filename);

      msModel::SetupJoints() -> SetupJoints(bool set_tangents);

      fopen(filename, "rb") ->
         #if WINDOWS
	         FILE *fp = _wfopen(filename, L"rb");
         #else
	         FILE *fp = fopen(UnixPathUTF8(filename), "r");
         #endif

      renamed *.cpp -> *.cpp.h so by default they will not be added to compilation
/******************************************************************************/
#define USE_MS3D_ANIMATION_METHODS 0
#pragma warning(push)
#pragma warning(disable:4996) // '_wfopen': This function or variable may be unsafe. Consider using _wfopen_s instead
#pragma warning(disable:4267) // 64bit - conversion from 'size_t' to 'Int', possible loss of data
#include "../../../../ThirdPartyLibs/MilkShape3D/mathlib.cpp.h"
#include "../../../../ThirdPartyLibs/MilkShape3D/msModel.cpp.h"
#pragma warning(pop)
/******************************************************************************/
static void ToMatrix(Matrix3 &m, float a[3][4])
{
   m.x.set(a[0][0], a[1][0], a[2][0]);
   m.y.set(a[0][1], a[1][1], a[2][1]);
   m.z.set(a[0][2], a[1][2], a[2][2]);
}
static void ToMatrix(Matrix &m, float a[3][4])
{
   m.x  .set(a[0][0], a[1][0], a[2][0]);
   m.y  .set(a[0][1], a[1][1], a[2][1]);
   m.z  .set(a[0][2], a[1][2], a[2][2]);
   m.pos.set(a[0][3], a[1][3], a[2][3]);
}
static void FromMatrix(Matrix &m, float a[3][4])
{
   a[0][0]=m.x  .x; a[1][0]=m.x  .y; a[2][0]=m.x  .z;
   a[0][1]=m.y  .x; a[1][1]=m.y  .y; a[2][1]=m.y  .z;
   a[0][2]=m.z  .x; a[1][2]=m.z  .y; a[2][2]=m.z  .z;
   a[0][3]=m.pos.x; a[1][3]=m.pos.y; a[2][3]=m.pos.z;
}
static void SetSkin(VecB4 &matrix, VecB4 &blend, msModel &ms3d, ms3d_vertex_t &vertex, MemtN<Byte, 256> &old_to_new, Skeleton *skeleton)
{
   int indices[4], weights[4]; ms3d.FillJointIndicesAndWeights(&vertex, indices, weights);

   MemtN<IndexWeight, 256> skin; // div by 100 not needed because weight is normalized in 'SetSkin'
   if(weights[0]>0 && InRange(indices[0], old_to_new))skin.New().set(old_to_new[indices[0]], weights[0]/* /100.0f */);
   if(weights[1]>0 && InRange(indices[1], old_to_new))skin.New().set(old_to_new[indices[1]], weights[1]/* /100.0f */);
   if(weights[2]>0 && InRange(indices[2], old_to_new))skin.New().set(old_to_new[indices[2]], weights[2]/* /100.0f */);
   if(weights[3]>0 && InRange(indices[3], old_to_new))skin.New().set(old_to_new[indices[3]], weights[3]/* /100.0f */);

   SetSkin(skin, matrix, blend, skeleton);
}
static void CreateSkeleton(Skeleton &skeleton, msModel &ms3d, Int bones, XAnimation *animation)
{
   ms3d.SetupJoints(USE_MS3D_ANIMATION_METHODS);

   skeleton.bones.setNum(bones);
   FREP(bones)
   {
      ms3d_joint_t &joint=*ms3d.GetJoint(i);
      SkelBone     &sbon =skeleton.bones[i];
      Set(sbon.name, joint.name);
          sbon.parent=(InRange(joint.parentIndex, bones) ? joint.parentIndex : 0xFF);

      Matrix m; ToMatrix(m, joint.matGlobalSkeleton);

      sbon.perp=m.y  ;
      sbon.dir =m.x  ;
      sbon.pos =m.pos;
   }
   skeleton.setBoneLengths();

   // animation
   if(animation)
   {
      animation->fps=ms3d.GetAnimationFps();
      Animation &anim=animation->anim;
      anim.length(ms3d.GetTotalFrames(), false);
      anim.bones.setNum(bones);
      FREPA(anim.bones)
      {
       C SkelBone     &sbon        =skeleton.bones[i];
         AnimBone     &abon        =anim    .bones[i];
         Bool          parent      =(sbon.parent!=0xFF);
         Matrix3       parent_matrix_inv; if(parent)skeleton.bones[sbon.parent].inverse(parent_matrix_inv);
         ms3d_joint_t &joint       =         *ms3d.GetJoint(                i)        ,
                      *joint_parent=(parent ? ms3d.GetJoint(joint.parentIndex) : null);

         abon.set(sbon.name);
         abon.orns.setNumZero(joint.rotationKeys.size());
         abon.poss.setNumZero(joint.positionKeys.size());

         // setup rotations
         FREPA(abon.orns)
         {
            AnimKeys::Orn   &orn  =abon .orns        [i];
          C ms3d_keyframe_t &frame=joint.rotationKeys[i];
            orn.time=frame.time;

            Orient    sbon_src =sbon             ; // src in global space
            if(parent)sbon_src*=parent_matrix_inv; // src in parent space

         #if USE_MS3D_ANIMATION_METHODS
            // calculating animations through ms3d functions (very slow, because all bones need to be calculated)
            // from available global matrixes for each bone we calculate as follow:
            // target orientation in parent space = joint.matGlobal / jointParent.matGlobal

         //if(orn.time>40)continue; // calculating animations for all bones and frames is very slow, so for tests limit to only few frames

            ms3d.SetFrame(frame.time);
            // opis : R_ConcatTransforms(parentJoint->matGlobal, joint->matLocal, joint->matGlobal);
            // czyli: joint->matGlobal = joint->matLocal * parentJoint->matGlobal;

            Matrix3 m; ToMatrix(m, joint.matGlobal);
            orn.orn.perp=m.y;
            orn.orn.dir =m.x; // dest in global space

            if(parent)
            {
               Orient parent_dest; ToMatrix(m, joint_parent->matGlobal);
               parent_dest.perp=m.y;
               parent_dest.dir =m.x; // parent_dest in global space

               orn.orn/=Matrix3(parent_dest); // dest in parent space
            }
         #else
            // code from ms3d classes
            {
               vec4_t quat            ;  AngleQuaternion(frame.key,  quat); // calculate the quaternion basing on the angles
               float  matAnimate[3][4]; QuaternionMatrix(quat, matAnimate); // calculate the transformation matrix basing on the quaternion
             //matAnimate[0][3]=pos[0];
             //matAnimate[1][3]=pos[1]; we don't care about positions
             //matAnimate[2][3]=pos[2];

               // animate the local joint matrix using: matLocal = matLocalSkeleton * matAnimate
               R_ConcatTransforms(joint.matLocalSkeleton, matAnimate, joint.matLocal); // set target matrix
            }

            Matrix3 m; ToMatrix(m, joint.matLocal); // convert the matrix to easier form

            if(parent)
            {
               // formula discovered by trial and error (observing what should we receive when using USE_MS3D_ANIMATION_METHODS - orn.orn / parent_dest, according to what we have in joint.matLocal)
               orn.orn.perp.x=-m.y.z;
               orn.orn.perp.y= m.y.y;
               orn.orn.perp.z= m.y.x;

               orn.orn.dir.x=-m.x.z;
               orn.orn.dir.y= m.x.y;
               orn.orn.dir.z= m.x.x;
            }else
            {
               orn.orn.perp=m.y;
               orn.orn.dir =m.x; // dest in global space
            }
         #endif
         }

         // setup positions
         FREPA(abon.poss)
         {
            AnimKeys::Pos   &pos  =abon .poss        [i];
          C ms3d_keyframe_t &frame=joint.positionKeys[i];
            pos.time=frame.time;

            // code from ms3d classes
            {
               float matAnimate[3][4]; // we don't care about orientations
               matAnimate[0][3]=frame.key[0];
               matAnimate[1][3]=frame.key[1];
               matAnimate[2][3]=frame.key[2];

               // animate the local joint matrix using: matLocal = matLocalSkeleton * matAnimate
               R_ConcatTransforms(joint.matLocalSkeleton, matAnimate, joint.matLocal); // set target matrix
            }

            Matrix M_l ; ToMatrix(M_l , joint.matLocal        );
            Matrix M_ls; ToMatrix(M_ls, joint.matLocalSkeleton);
            pos.pos=M_l.pos-M_ls.pos;
            if(parent)
            {
               Matrix3  p_gs; ToMatrix(p_gs, joint_parent->matGlobalSkeleton);
               pos.pos*=p_gs;
               pos.pos*=parent_matrix_inv;
            }
         }
      }

      // rescale in time
      if(Flt fps=ms3d.GetAnimationFps())anim.length(anim.length()/fps, true);

      // process
      anim.setTangents().removeUnused().setRootMatrix();
   }
}
/******************************************************************************/
} // namespace MS3D
/******************************************************************************/
Bool ImportMS3D(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh     )mesh     ->del();
   if(skeleton )skeleton ->del();
   if(animation)animation->del();
   materials          .clear();
   part_material_index.clear();

#if !WEB // MilkShape can load only from a stdio file
   using namespace MS3D;

   msModel ms3d; if(ms3d.Load(name))
   {
      Int material_num=ms3d.GetNumMaterials(),
          vtxs        =ms3d.GetNumVertices (),
          tris        =ms3d.GetNumTriangles(),
          bones       =ms3d.GetNumJoints   ();

      // materials
      if(materials)FREP(material_num)
      {
         XMaterial       &xm =materials.New();
       C ms3d_material_t &mat=*ms3d.GetMaterial(i);

         xm.ambient .set(mat.emissive[0], mat.emissive[1], mat.emissive[2]);
         xm.color   .set(mat.diffuse [0], mat.diffuse [1], mat.diffuse [2], mat.transparency);
         xm.specular.set(mat.specular[0], mat.specular[1], mat.specular[2]);

         xm.name     =mat.name    ;
         xm.color_map=mat.texture ;
         xm.alpha_map=mat.alphamap;

         xm.fixPath(GetPath(name));
      }

      // skeleton
      MemtN<Byte, 256> old_to_new;
      Skeleton temp, *skel=(skeleton ? skeleton : (mesh || animation) ? &temp : null); // if skel not specified, but we want mesh or animation, then we have to process it
      if(skel){CreateSkeleton(*skel, ms3d, bones, animation); skel->sortBones(old_to_new); if(VIRTUAL_ROOT_BONE)REPAO(old_to_new)++;} // 'sortBones' must be called before 'SetSkin'

      // mesh
      if(mesh)
      {
         mesh->create(ms3d.GetNumGroups()); FREPA(*mesh)
         {
            ms3d_group_t &group=*ms3d. GetGroup(i);
            MeshPart     &part = mesh->parts[i];
            MeshBase     &base = part. base;

            if(part_material_index)part_material_index.add(InRange(group.materialIndex, material_num) ? group.materialIndex : -1);

            Set(part.name, group.name);
            base.create(group.triangleIndices.size()*3, 0, group.triangleIndices.size(), 0, VTX_POS|VTX_NRM|VTX_TEX0|TRI_IND|(bones ? VTX_SKIN : 0));
            FREPA(base.tri)
            {
               Int t=group.triangleIndices[i]; if(!InRange(t, tris))goto invalid;
               ms3d_triangle_t &triangle=*ms3d.GetTriangle(t); REPA(triangle.vertexIndices)if(!InRange(triangle.vertexIndices[i], vtxs))goto invalid;
               ms3d_vertex_t   &vertex0 =*ms3d.GetVertex  (triangle.vertexIndices[0]);
               ms3d_vertex_t   &vertex1 =*ms3d.GetVertex  (triangle.vertexIndices[1]);
               ms3d_vertex_t   &vertex2 =*ms3d.GetVertex  (triangle.vertexIndices[2]);

               Int v=i*3;
               base.vtx.pos(v+0).set(vertex0.vertex[0], vertex0.vertex[1], vertex0.vertex[2]);
               base.vtx.pos(v+1).set(vertex1.vertex[0], vertex1.vertex[1], vertex1.vertex[2]);
               base.vtx.pos(v+2).set(vertex2.vertex[0], vertex2.vertex[1], vertex2.vertex[2]);

               base.vtx.nrm(v+0).set(triangle.vertexNormals[0][0], triangle.vertexNormals[0][1], triangle.vertexNormals[0][2]);
               base.vtx.nrm(v+1).set(triangle.vertexNormals[1][0], triangle.vertexNormals[1][1], triangle.vertexNormals[1][2]);
               base.vtx.nrm(v+2).set(triangle.vertexNormals[2][0], triangle.vertexNormals[2][1], triangle.vertexNormals[2][2]);

               base.vtx.tex0(v+0).set(triangle.s[0], triangle.t[0]);
               base.vtx.tex0(v+1).set(triangle.s[1], triangle.t[1]);
               base.vtx.tex0(v+2).set(triangle.s[2], triangle.t[2]);

               if(base.vtx.matrix() && base.vtx.blend())
               {
                  SetSkin(base.vtx.matrix(v+0), base.vtx.blend(v+0), ms3d, vertex0, old_to_new, skel);
                  SetSkin(base.vtx.matrix(v+1), base.vtx.blend(v+1), ms3d, vertex1, old_to_new, skel);
                  SetSkin(base.vtx.matrix(v+2), base.vtx.blend(v+2), ms3d, vertex2, old_to_new, skel);
               }

               base.tri.ind(i).set(v+0, v+1, v+2);
            }
            
            base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
         }
         mesh->setBox();
      }

      // remove nub bones
    //if(skel){Animation* anim=(animation ? &animation->anim : null); RemoveNubBones(mesh, *skel, anim);}

      // finalize
      if(skel     ) skel     ->mirrorX().setBoneTypes();
      if(animation) animation->anim.mirrorX().setBoneTypeIndexesFromSkeleton(*skel);
      if(mesh     ){mesh     ->mirrorX().skeleton(skel).skeleton(null); CleanMesh(*mesh);}
      if(skeleton ){skel     ->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}

      return true;
   }
#endif
invalid:
   if(mesh)mesh->del();
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
