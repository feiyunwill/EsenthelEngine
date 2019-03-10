/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// FIXME: support MatrixM (it would be best if we keep Matrix for all bones, but only global offset as VecD somehow) however it's unlikely
/******************************************************************************/
// ANIM SKEL BONE
/******************************************************************************/
void AnimatedSkeletonBone::clear()
{
   orn  .zero();
   rot  .zero();
   pos  .zero();
   scale.zero();
#if HAS_ANIM_COLOR
   color.set(1);
#endif
  _force_custom=_world_space_transform=false;
}
void AnimatedSkeletonBone::clear(Flt blend)
{
   if(blend>0)
   {
      if(blend>=1)clear();else
      {
         Flt    blend1=1-blend;
         orn  *=blend1;
         rot  *=blend1;
         pos  *=blend1;
         scale*=blend1;
      #if HAS_ANIM_COLOR
         color*=blend1; color+=blend; // color=Vec4(1)*blend + color*(1-blend);
      #endif
         if(_world_space_transform)_world_space_transform_matrix.identity(blend);
      }
   }
}
void AnimatedSkeletonBone::forceMatrix(C Matrix &matrix)
{
  _force_custom       =true;
  _force_custom_matrix=matrix;
}
/******************************************************************************/
// ANIM SKEL
/******************************************************************************/
void AnimatedSkeleton::zero()
{
   fur_stiffness=0;
   fur_gravity  =0;
   fur_vel_scale=0;
   root.zero();

  _updated_vel=false;
  _scale      =0;
  _skeleton   =null;
}
AnimatedSkeleton::AnimatedSkeleton() {zero();}
AnimatedSkeleton& AnimatedSkeleton::del()
{
   bones.del();
   slots.del();
   zero(); return T;
}
AnimatedSkeleton& AnimatedSkeleton::create(C Skeleton *skeleton, Flt scale, C Matrix &initial_matrix) // !! 'initial_matrix' can be 'root._matrix' !!
{
     _updated_vel=false;
      T._scale   =scale;
   if(T._skeleton=skeleton)
   {
      bones.setNum(skeleton->bones.elms()); REPAO(bones).zero();
      slots.setNum(skeleton->slots.elms()); REPAO(slots).zero();
   }else
   {
      bones.clear();
      slots.clear();
   }

   fur_stiffness= 0.0001f;
   fur_gravity  =-1;
   fur_vel_scale=-0.75f;

   Matrix temp=initial_matrix; // copy in case 'initial_matrix' is 'root._matrix'
   root.zero();
   root._center=temp.pos;
   root._matrix=temp;
   root._matrix.normalize(scale);

   return T;
}
AnimatedSkeleton& AnimatedSkeleton::create(AnimatedSkeleton &src)
{
   if(&src!=this)
   {
     _scale   =src._scale;
     _skeleton=src._skeleton;
      bones   =src. bones;
      slots   =src. slots;

      root=src.root;

      fur_gravity  =src.fur_gravity  ;
      fur_stiffness=src.fur_stiffness;
      fur_vel_scale=src.fur_vel_scale;

     _updated_vel=src._updated_vel;
   }
   return T;
}
/******************************************************************************/
// GET
/******************************************************************************/
AnimSkelBone* AnimatedSkeleton::findBone    (BONE_TYPE type, Int type_index, Int type_sub)  {return skeleton() ? bones.addr(skeleton()->findBoneI(type, type_index, type_sub)) : null;}
Int           AnimatedSkeleton::findBoneI   (BONE_TYPE type, Int type_index, Int type_sub)C {return skeleton() ?            skeleton()->findBoneI(type, type_index, type_sub)  :   -1;}
Byte          AnimatedSkeleton::findBoneB   (BONE_TYPE type, Int type_index, Int type_sub)C {return skeleton() ?            skeleton()->findBoneB(type, type_index, type_sub)  :  255;}
SkelAnim*     AnimatedSkeleton::findSkelAnim(C Str  &name)C {return skeleton() ? skeleton()->findSkelAnim(name) : null;}
SkelAnim*     AnimatedSkeleton::findSkelAnim(C UID  &id  )C {return skeleton() ? skeleton()->findSkelAnim(id  ) : null;}
Int           AnimatedSkeleton::findBoneI   (CChar8 *name)C {return skeleton() ? skeleton()->findBoneI   (name) : -1  ;}
Int           AnimatedSkeleton::findSlotI   (CChar8 *name)C {return skeleton() ? skeleton()->findSlotI   (name) : -1  ;}
Byte          AnimatedSkeleton::findSlotB   (CChar8 *name)C {return skeleton() ? skeleton()->findSlotB   (name) : 255 ;}
AnimSkelBone* AnimatedSkeleton::findBone    (CChar8 *name)  {return bones.addr(  findBoneI   (name));}
OrientP     * AnimatedSkeleton::findSlot    (CChar8 *name)  {return slots.addr(  findSlotI   (name));}
AnimSkelBone* AnimatedSkeleton:: getBone    (CChar8 *name)  {return bones.addr(   getBoneI   (name));}
OrientP     * AnimatedSkeleton:: getSlot    (CChar8 *name)  {return slots.addr(   getSlotI   (name));}
Int           AnimatedSkeleton:: getBoneI   (CChar8 *name)C {Int       i        =findBoneI   (name); if(i<0                       )Exit(S+    "Bone \""+name          +"\" not found in skeleton \""+Skeletons.name(skeleton())+"\"."); return i        ;}
Int           AnimatedSkeleton:: getSlotI   (CChar8 *name)C {Int       i        =findSlotI   (name); if(i<0                       )Exit(S+    "Slot \""+name          +"\" not found in skeleton \""+Skeletons.name(skeleton())+"\"."); return i        ;}
SkelAnim*     AnimatedSkeleton:: getSkelAnim(C Str  &name)C {SkelAnim *skel_anim=findSkelAnim(name); if(!skel_anim && name.is   ())Exit(S+"SkelAnim \""+name          +"\" not found in skeleton \""+Skeletons.name(skeleton())+"\"."); return skel_anim;}
SkelAnim*     AnimatedSkeleton:: getSkelAnim(C UID  &id  )C {SkelAnim *skel_anim=findSkelAnim(id  ); if(!skel_anim && id  .valid())Exit(S+"SkelAnim \""+id.asCString()+"\" not found in skeleton \""+Skeletons.name(skeleton())+"\"."); return skel_anim;}
/******************************************************************************/
// SET
/******************************************************************************/
AnimatedSkeleton& AnimatedSkeleton::disable(Int i, Bool disable)
{
   if(InRange(i, bones))bones[i]._disabled=disable;
   return T;
}
static void DisableChildren(AnimatedSkeleton &anim_skel, Int i, Bool disable)
{
   if(C SkelBone *bone=anim_skel.skeleton()->bones.addr(i))
      REP(bone->children_num)
   {
      Int child_i=bone->children_offset+i;
      if(InRange(child_i, anim_skel.bones))
      {
         anim_skel.bones[child_i]._disabled=disable;
         DisableChildren(anim_skel, child_i, disable);
      }
   }
}
AnimatedSkeleton& AnimatedSkeleton::disableChildren(Int i, Bool disable)
{
   if(InRange(i, bones))
   {
      Bool &disabled =bones[i]._disabled_children;
      if(   disabled!=disable)
      {
         disabled=disable;
         if(skeleton())DisableChildren(T, i, disable); // test for 'skeleton' only once here and not everytime in 'DisableChildren'
      }
   }
   return T;
}
/******************************************************************************/
// ANIMATE
/******************************************************************************/
AnimatedSkeleton& AnimatedSkeleton::clear()
{
         root  .clear();
   REPAO(bones).clear();
   return T;
}
AnimatedSkeleton& AnimatedSkeleton::clear(Flt blend)
{
         root  .clear(blend);
   REPAO(bones).clear(blend);
   return T;
}
struct AnimParamsEx : AnimParams
{
   Bool replace;
   Flt  blend  ,
        blend1 ;

   AnimParamsEx(C Animation &animation, Flt time, Flt blend, Bool replace) : AnimParams(animation, time)
   {
      T.replace=      replace;
      T.blend  =      blend ; // allow ranges >1 for example for big relative rotations
      T.blend1 =Sat(1-blend); // this needs to be clamped to 0..1 range because old keyframes are multiplied by this
   }
};
static void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParamsEx &params)
{
   if(!asbon._disabled)
   {
      // orientation
      if(keys.orns.elms())
      {
         Orient &bone_orn=asbon.orn, orn; keys.orn(orn, params);
         if(params.replace)bone_orn*=params.blend1;
                           bone_orn+=params.blend*orn;
      }

      // position
      if(keys.poss.elms())
      {
         Vec &bone_pos=asbon.pos, pos; keys.pos(pos, params);
         if(params.replace)bone_pos*=params.blend1;
                           bone_pos+=params.blend*pos;
      }

      // scale
      if(keys.scales.elms())
      {
         Vec &bone_scale=asbon.scale, scale; keys.scale(scale, params);
         if(params.replace)bone_scale*=params.blend1;
                           bone_scale+=params.blend*scale;
      }

   #if HAS_ANIM_ROT
      // rotation
      if(keys.rots.elms())
      {
         AxisRoll &bone_rot=asbon.rot, rot; keys.rot(rot, params);
         if(params.replace)bone_rot.v4()*=params.blend1;
                           bone_rot.v4()+=params.blend*rot.v4();
      }
   #endif

   #if HAS_ANIM_COLOR
      // color
      if(keys.colors.elms())
      {
         Vec4 &bone_color=asbon.color, color; keys.color(color, params);
       /*if(params.replace)*/bone_color*=params.blend1; // colors are always replaced
                             bone_color+=params.blend*color;
      }
   #endif
   }
}
static void AnimRoot(AnimatedSkeleton &anim_skel, C Animation *animation, Flt time, Flt blend, Bool replace)
{
 //if(blend>EPS_ANIM_BLEND) // this is already checked in methods calling this function
      if(animation)
   {
      AnimParamsEx params(*animation, time, blend, replace);
      Animate(anim_skel.root, animation->keys, params); // animate root
   }
}
/******************************************************************************/
AnimatedSkeleton& AnimatedSkeleton::animate(C SkelAnim &skel_anim, Flt time, Flt blend, Bool replace)
{
   if(blend>EPS_ANIM_BLEND)
      if(C Animation *animation=skel_anim.animation())
   {
      AnimParamsEx params(*animation, time, blend, replace);

    //Animate(root, animation->keys, params); // animate root - this is no longer done here, instead, root animations need to be processed manually
      REPA(animation->bones)                  // animate bones
      {
         Byte sbon=skel_anim.abonToSbon(i);
         if(InRange(sbon, bones))Animate(bones[sbon], animation->bones[i], params);
      }
   }
   return T;
}

AnimatedSkeleton& AnimatedSkeleton::animateRoot(C Animation *anim, Flt time) {if(anim)animateRoot(*anim, time); return T;}
AnimatedSkeleton& AnimatedSkeleton::animateRoot(C Animation &anim, Flt time)
{
   AnimParamsEx params(anim, time, 1, false);
   Animate(root, anim.keys, params); // animate root
   return T;
}
AnimatedSkeleton& AnimatedSkeleton::animateExactTime(C SkelAnim &skel_anim, Flt time) // this will not apply fraction for time, this function is needed when adjusting animations to make sure we process exact keyframes based on time, but still with looping support
{
   if(C Animation *animation=skel_anim.animation())
   {
      AnimParamsEx params(*animation, time, 1, false); params.time=time; // re-apply time to remove possible fraction

      Animate(root, animation->keys, params); // animate root
      REPA(animation->bones)                  // animate bones
      {
         Byte sbon=skel_anim.abonToSbon(i);
         if(InRange(sbon, bones))Animate(bones[sbon], animation->bones[i], params);
      }
   }
   return T;
}
AnimatedSkeleton& AnimatedSkeleton::animate(C SkelAnim *skel_anim, Flt time, Flt blend, Bool replace) {if(skel_anim                                )              T.animate(*skel_anim             ,        time,            blend  , replace); return T;}
AnimatedSkeleton& AnimatedSkeleton::animate(C Motion   &motion   ,                      Bool replace) {if(motion   .is   ()                        )              T.animate(*motion.skel_anim      , motion.time, motion.animBlend(), replace); return T;}
AnimatedSkeleton& AnimatedSkeleton::animate(C Str      &anim_name, Flt time, Flt blend, Bool replace) {if(anim_name.is   () && blend>EPS_ANIM_BLEND)if(skeleton())T.animate(*getSkelAnim(anim_name),        time,            blend  , replace);else AnimRoot(T, Animations(anim_name), time, blend, replace); return T;} // in these methods check 'blend' first to avoid unnecessary animation loads
AnimatedSkeleton& AnimatedSkeleton::animate(C UID      &anim_id  , Flt time, Flt blend, Bool replace) {if(anim_id  .valid() && blend>EPS_ANIM_BLEND)if(skeleton())T.animate(*getSkelAnim(anim_id  ),        time,            blend  , replace);else AnimRoot(T, Animations(anim_id  ), time, blend, replace); return T;} // in these methods check 'blend' first to avoid unnecessary animation loads
/******************************************************************************/
static void UpdateRootBoneMatrix(AnimatedSkeleton &anim_skel, C Matrix &body_matrix)
{
   AnimSkelBone &bone=anim_skel.root;

   if(bone._disabled)
   {
   #if 0 // slower
      bone._matrix.setScale(anim_skel._scale).mul(body_matrix);
   #else
      bone._matrix=body_matrix;
      bone._matrix.orn().scale(anim_skel._scale);
   #endif
   }else
   {
      Orient &bone_orn=bone.orn; // we can modify it directly, because we're just calling 'fix' on it

      // rotation
      if(bone.rot.any())
      {
         Vec axis =bone.rot.axis;
         Flt angle=axis.normalize();

         if(bone.rot.roll)
         {
            bone._matrix.orn().setRotateZ(bone.rot.roll)
                              .rotate    (axis, angle  );
         }else
         {
            bone._matrix.orn().setRotate(axis, angle);
         }

         if(bone_orn.fix()) // orientation
         {
            bone._matrix.orn()*=Matrix3(bone_orn);
         }
      }else
      if(bone_orn.fix()) // orientation
      {
         bone._matrix.orn()=bone_orn;
      }else
      {
         // only position/scale
         bone._matrix=body_matrix;

         // apply skeleton scale
         if(anim_skel._scale!=1)bone._matrix.orn().scale(anim_skel._scale);

         // apply animation position
         if(bone.pos.any())bone._matrix.pos+=bone.pos*bone._matrix.orn();

         // apply animation scale
         if(bone.scale.any())
         {
            bone._matrix.x*=ScaleFactor(bone.scale.x);
            bone._matrix.y*=ScaleFactor(bone.scale.y);
            bone._matrix.z*=ScaleFactor(bone.scale.z);
         }
         return;
      }

      // set scale
      if(bone.scale.any())
      {
         bone._matrix.x*=ScaleFactor(bone.scale.x);
         bone._matrix.y*=ScaleFactor(bone.scale.y);
         bone._matrix.z*=ScaleFactor(bone.scale.z);
      }

      // set position
                             bone._matrix.pos=bone.pos;
      if(anim_skel._scale!=1)bone._matrix.scale(anim_skel._scale);
                             bone._matrix.mul(body_matrix);
   }
}
static void UpdateBoneMatrix(AnimatedSkeleton &anim_skel, Int i)
{
   AnimSkelBone &bone                   =anim_skel.            bones[i];
     C SkelBone &sbon                   =anim_skel.skeleton()->bones[i];
     C SkelBone *parent                 =anim_skel.skeleton()->bones.addr(sbon.parent);
       Matrix   &parent_transform_matrix=anim_skel.            boneRoot  (sbon.parent)._matrix;

   if(bone._force_custom)bone._matrix=bone._force_custom_matrix;else
   if(bone._disabled    )bone._matrix=  parent_transform_matrix;else
   {
      Matrix3 parent_matrix; if(parent)parent_matrix=*parent;
      Orient  bone_orn=bone.orn;

   /* Animation Formula:

      #1 Rotation

         Vec axis =bone.rot.axis   ; // rotation in parent space
         Flt angle=axis.normalize();
         if(parent)axis*=parent_matrix; // rotation in world space

         bone.matrix.setPos(-sbon.pos               )
                    .rot   ( sbon.dir               , bone.rot.roll)
                    .rot   ( axis                   , angle)
                    .move  ( sbon.pos               )
                    .mul   ( parent_transform_matrix);

      #2 Orientation

         Orient src =sbon    ;             // current orientation in world  space
         Orient dest=bone_orn; dest.fix(); // target  orientation in parent space
         if(parent)dest*=parent_matrix;    // target  orientation in world  space
         Matrix3 transform=GetTransform(src, dest);

         bone.matrix.setPos(-sbon.pos)
                    .mul   (transform)
                    .move  ( sbon.pos)
                    .mul   (parent_transform_matrix);
   */

      // rotation
      if(bone.rot.any())
      {
         Vec axis =bone.rot.axis;       // rotation in parent space
         Flt angle=axis.normalize();
         if(parent)axis*=parent_matrix; // rotation in world  space

         if(bone.rot.roll)
         {
            bone._matrix.orn().setRotate(sbon.dir, bone.rot.roll)
                              .rotate   (axis    , angle        );
         }else
         {
            bone._matrix.orn().setRotate(axis, angle);
         }

         if(bone_orn.fix()) // orientation
         {
            if(parent)bone_orn.mul(parent_matrix, true); // transform target orientation from parent space to world space
            Matrix3 transform; GetTransform(transform, sbon, bone_orn); bone._matrix.orn()*=transform;
         }
      }else
      if(bone_orn.fix()) // orientation
      {
         if(parent)bone_orn.mul(parent_matrix, true); // transform target orientation from parent space to world space
         GetTransform(bone._matrix.orn(), sbon, bone_orn);
      }else
      {
         // set scale
         if(bone.scale.any())
         {
            bone._matrix.orn().identity();
            bone._matrix.orn().scale(sbon.cross(), ScaleFactor(bone.scale.x));
            bone._matrix.orn().scale(sbon.perp   , ScaleFactor(bone.scale.y));
            bone._matrix.orn().scale(sbon.dir    , ScaleFactor(bone.scale.z));
            goto scale_set;
         }

         // only position
         bone._matrix=parent_transform_matrix;
         if(bone.pos.any())
         {
         #if 1 // pos relative to parent
            bone._matrix.pos+=(parent ? bone.pos*parent_matrix : bone.pos)*parent_transform_matrix.orn();
         #else
            bone._matrix.pos+=bone.pos*parent_transform_matrix.orn();
         #endif
         }
         goto matrix_set;
      }

      // set scale
      if(bone.scale.any())
      {
         Orient sbon_transformed=sbon; sbon_transformed.mul(bone._matrix.orn(), true);
         bone._matrix.orn().scale(sbon_transformed.cross(), ScaleFactor(bone.scale.x));
         bone._matrix.orn().scale(sbon_transformed.perp   , ScaleFactor(bone.scale.y));
         bone._matrix.orn().scale(sbon_transformed.dir    , ScaleFactor(bone.scale.z));
      }
   scale_set:

      // set position
      bone._matrix.anchor(sbon.pos);
   #if 1 // pos relative to parent
      if(bone.pos.any())bone._matrix.pos+=(parent ? bone.pos*parent_matrix : bone.pos);
   #else
      bone._matrix.pos+=bone.pos;
   #endif

      bone._matrix*=parent_transform_matrix;
   }
matrix_set:

   // world space transformation
   if(bone._world_space_transform)bone._matrix.transformAtPos(sbon.pos*bone._matrix, bone._world_space_transform_matrix);
}
static void UpdateSlot(AnimatedSkeleton &anim_skel, Int i)
{
 C SkelSlot &skel_slot=anim_skel.skeleton()->slots[i];
   OrientP  &     slot=anim_skel.            slots[i];
   slot=skel_slot;
   slot.mul(anim_skel.boneRoot(skel_slot.bone).matrix(), true);
   if(skel_slot.bone!=skel_slot.bone1)
   {
      OrientP secondary=skel_slot;
      secondary.mul(anim_skel.boneRoot(skel_slot.bone1).matrix(), true);
      slot+=secondary;
      slot.fix();
      slot.pos*=0.5f;
   }
}
/******************************************************************************/
AnimatedSkeleton& AnimatedSkeleton::updateMatrix(C Matrix &body_matrix)
{
   UpdateRootBoneMatrix(T, body_matrix);
   if(skeleton()) // test 'skeleton' once here, and not everytime in 'UpdateBoneMatrix' and 'UpdateSlot'
   {
      Int min_bones=minBones(); FREP(min_bones )UpdateBoneMatrix(T, i); // process bones in order to update parents first
                                 REP(minSlots())UpdateSlot      (T, i); // order is not important, because slots are attached to bones (not slots)
   }
   return T;
}
/******************************************************************************/
static void UpdateBoneMatrixRecursiveUp(AnimatedSkeleton &anim_skel, Int i)
{
   Byte parent=anim_skel.skeleton()->bones[i].parent;
   if(  parent<i)UpdateBoneMatrixRecursiveUp(anim_skel, parent); // first update parents, "parent<i" means that parent is !=0xFF (!= <null>), parent fits in minBones range and this prevents infinite loops (looped parent cycle)
                 UpdateBoneMatrix           (anim_skel,      i); // now   update self
}
AnimatedSkeleton& AnimatedSkeleton::updateMatrixParents(C Matrix &body_matrix, Int bone)
{
                                              UpdateRootBoneMatrix       (T, body_matrix); // first update root
   if(skeleton() && InRange(bone, minBones()))UpdateBoneMatrixRecursiveUp(T, bone       ); // now   update parents and self, test 'skeleton' once here, and not everytime in 'UpdateBoneMatrixRecursiveUp'
   return T;
}
/******************************************************************************/
static void UpdateBoneMatrixRecursiveDown(AnimatedSkeleton &anim_skel, Int i, Int min_bones)
{
 C SkelBone &bone=anim_skel.skeleton()->bones[i];
   UpdateBoneMatrix(anim_skel, i); // first update self
   for(Int i=Min(bone.children_offset+bone.children_num, min_bones); --i>=bone.children_offset; ) // now update children
      UpdateBoneMatrixRecursiveDown(anim_skel, i, min_bones);
}
AnimatedSkeleton& AnimatedSkeleton::updateMatrixChildren(Int bone) // this updates 'bone' too
{
   if(skeleton()) // test 'skeleton' once here, and not everytime in 'UpdateBoneMatrixRecursiveDown' and 'UpdateSlot'
   {
      Int min_bones=minBones(); if(InRange(bone, min_bones))
      {
                        UpdateBoneMatrixRecursiveDown(T, bone, min_bones);
         REP(minSlots())UpdateSlot                   (T, i              ); // update slots once bones are ready (because slots are attached to bones)
      }
   }
   return T;
}
/******************************************************************************/
AnimatedSkeleton& AnimatedSkeleton::forceMatrix(Int bone, C Matrix &matrix, Bool auto_update_matrixes)
{
   if(InRange(bone, bones))
   {
      bones[bone].forceMatrix(matrix);
      if(auto_update_matrixes)updateMatrixChildren(bone); // this will update 'bone' too
   }
   return T;
}
AnimatedSkeleton& AnimatedSkeleton::transformInWorldSpace(Int bone, C Matrix3 &matrix, Bool auto_update_matrixes)
{
   if(InRange(bone, bones))
   {
      AnimSkelBone &b=bones[bone];

      if(b._world_space_transform)b._world_space_transform_matrix*=matrix;else // if there was already a world transform, then adjust it
      {
         b._world_space_transform       =true;
         b._world_space_transform_matrix=matrix;
      }

      if(auto_update_matrixes)updateMatrixChildren(bone); // this will update 'bone' too
   }
   return T;
}
AnimatedSkeleton& AnimatedSkeleton::transformInWorldSpace(Int bone, C Matrix &matrix, Bool auto_update_matrixes)
{
   if(InRange(bone, bones))
   {
      AnimSkelBone &b=bones[bone];

      if(b._world_space_transform)b._world_space_transform_matrix*=matrix;else // if there was already a world transform, then adjust it
      {
         b._world_space_transform       =true;
         b._world_space_transform_matrix=matrix;
      }

      if(auto_update_matrixes)updateMatrixChildren(bone); // this will update 'bone' too
   }
   return T;
}
/******************************************************************************/
static Vec FurVel(C Vec &vel, Flt fur_vel_scale, Flt fur_gravity)
{
   Vec    fur_vel=vel*fur_vel_scale; fur_vel.y+=fur_gravity; fur_vel.clipLength(0.92f);
   return fur_vel;
}
AnimatedSkeleton& AnimatedSkeleton::vel(C Vec &vel)
{
   Vec fur_vel=FurVel(vel, fur_vel_scale, fur_gravity);
   root._vel    =    vel;
   root._fur_vel=fur_vel;
   REPA(bones)
   {
      AnimSkelBone &bone=bones[i];
      bone._vel    =    vel;
      bone._fur_vel=fur_vel;
   }
   return T;
}
void AnimatedSkeleton::updateVelocities(Bool according_to_physics_step, Bool ragdoll_bones_only)
{
   if(!_updated_vel) // not yet updated
   {
                                   root    ._center=pos();
      if(skeleton())REP(minBones())bones[i]._center=skeleton()->bones[i].center()*bones[i].matrix();
     _updated_vel=true;
   }else
   {
      Flt time_mul;
      Vec  fur_vel;

      if(according_to_physics_step && Physics.created())
      {
         if(!       Physics.updated    ())return;
         time_mul=1/Physics.updatedTime();
      }else
      {
         time_mul=((Time.d()>EPS) ? 1/Time.d() : 1);
      }

      // root
      root._vel    =(pos()-root._center)*time_mul;
      root._center = pos(); fur_vel=FurVel(vel(), fur_vel_scale, fur_gravity);
      AdjustValTime( root._fur_vel, fur_vel, fur_stiffness);

      // bones
      if(skeleton())
      {
         Int min_bones=minBones(); FREP(min_bones) // order is important (parents first)
         {
              C SkelBone & sbon=skeleton()->bones[i];
            AnimSkelBone &asbon=            bones[i];
            if(!ragdoll_bones_only || (sbon.flag&BONE_RAGDOLL))
            {
               Vec         p=    sbon.center() *asbon.matrix();
               asbon._vel   =(p-asbon.center())*time_mul      ; fur_vel=FurVel(asbon.vel(), fur_vel_scale, fur_gravity);
               asbon._center= p;
               AdjustValTime(asbon._fur_vel, fur_vel, fur_stiffness);
            }else // inherit values from the parent
            {
               AnimSkelBone &parent=boneRoot(sbon.parent);
               asbon._center =parent.  center();
               asbon._vel    =parent.     vel();
               asbon._fur_vel=parent._fur_vel  ;
            }
         }
      }
   }
}
/******************************************************************************/
void AnimatedSkeleton::move(C Vec &d)
{
         root  +=d;
   REPAO(bones)+=d;
   REPAO(slots)+=d;
}
void AnimatedSkeleton::offset(C Vec &d)
{
         root  ._matrix+=d;
   REPAO(bones)._matrix+=d;
   REPAO(slots)        +=d;
}
/******************************************************************************/
void AnimatedSkeleton::getMatrixes(MemPtrN<Matrix, 256> matrixes)C
{
   matrixes.setNum(bones.elms()+1);
              matrixes[  0]=         matrix();
   REPA(bones)matrixes[i+1]=bones[i].matrix();
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void AnimatedSkeleton::draw(C Color &bone_color, C Color &slot_color)C
{
   if(bone_color.a && skeleton())REP  (minBones())(skeleton()->bones[i]*bones[i].matrix()).draw(bone_color);
   if(slot_color.a              )REPAO(                                 slots            ).draw(slot_color);
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool AnimatedSkeleton::save(File &f)C
{
   f.putMulti(Byte(0), _scale, matrix()); // version
   f.putAsset(Skeletons.id(skeleton()));
   f.putMulti(fur_stiffness, fur_gravity, fur_vel_scale);
   return f.ok();
}
Bool AnimatedSkeleton::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f.getMulti(_scale, root._matrix); create(Skeletons(f.getAssetID()), _scale, root._matrix);
         f.getMulti(fur_stiffness, fur_gravity, fur_vel_scale);
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
