/******************************************************************************

   Use         'Skeleton' as         a base static (non-animated) skeleton, that contain bones and slots in their initial pose.
   Use 'AnimatedSkeleton' to animate a base static 'Skeleton', by applying 'Animation's on the 'AnimatedSkeleton'.

/******************************************************************************/
// STATIC SKELETON
/******************************************************************************/
enum BONE_TYPE : Byte
{
   BONE_UNKNOWN  ,
   BONE_SPINE    , // pelvis, hips, chest, torso, rib cage

   BONE_SHOULDER , // clavicle, collarbone
   BONE_UPPER_ARM, // arm
   BONE_LOWER_ARM, // forearm
   BONE_HAND     , // wrist, palm
   BONE_FINGER   ,

   BONE_UPPER_LEG, // thigh
   BONE_LOWER_LEG, // calf, crus, shin
   BONE_FOOT     , // ankle
   BONE_TOE      ,

   BONE_NECK     ,
   BONE_HEAD     ,
   BONE_JAW      ,
   BONE_TONGUE   ,
   BONE_NOSE     , // snout
   BONE_EYE      ,
   BONE_EYELID   ,
   BONE_EYEBROW  ,
   BONE_EAR      ,

   BONE_BREAST   ,
   BONE_BUTT     ,

   BONE_TAIL     ,
   BONE_WING     ,
   BONE_CAPE     , // cloak
   BONE_HAIR     ,

   BONE_NUM      , // number of BONE_TYPE elements

   // alternative names
   BONE_ARM    =BONE_UPPER_ARM,
   BONE_FOREARM=BONE_LOWER_ARM,
   BONE_THIGH  =BONE_UPPER_LEG,
   BONE_CALF   =BONE_LOWER_LEG,
};
enum BONE_FLAG
{
   BONE_RAGDOLL=1<<0, // if this option is enabled then the bone will be used to create an actor in the Ragdoll
};

struct BoneID
{
   Char8     name[32]  ; // name          , default=""
   BONE_TYPE type      ; // BONE_TYPE     , default=BONE_UNKNOWN
   SByte     type_index; // type index    , default=0, negative value means that this bone is on the left side, for example if there are 2 bones of BONE_HAND 'type', one with 'type_index'=-1 (left hand), other with 'type_index'=0 (right hand)
   Byte      type_sub  ; // type sub-index, default=0, sub-index is used to distinguish how deep in the parent<->child relation the bone is, for example if a skeleton has 2 hands each with 1 finger, each finger is made out of 3 bones (6 bones total for 2 fingers), then finger bones will have 'type_index' -1 (left) and 0 (right), and 'type_sub' set to 0, 1, 2, giving a total of 6 combinations

   BoneID& set(CChar8 *name, BONE_TYPE type=BONE_UNKNOWN, Int type_index=0, Int type_sub=0); // set name, type, type_index and type_sub

   BoneID& id()  {return T;}
 C BoneID& id()C {return T;}

   Bool operator==(C BoneID &id)C;
   Bool operator!=(C BoneID &id)C {return !(T==id);}

   BoneID() {set(null);}
};

typedef SkeletonBone SkelBone;
STRUCT2(SkeletonBone , OrientP , BoneID) // Skeleton Bone
//{
   Byte    parent         , // bone parent index             , default=0xFF, 0xFF=none
           children_offset, // offset of children in 'Skeleton.bones'
           children_num   , // number of children
           flag           ; // BONE_FLAG                     , default=0
   Flt     length         , // bone length                   , default=0.3
           width          ; // bone width                    , default=0.2
   Vec     offset         ; // bone offset applied to 'shape', default=(0, 0, 0)
   Capsule shape          ; // shape covering the bone, automatically set by 'Skeleton.setBoneShapes' method, depending on bone position, orientation, length, width and being a ragdoll bone

   // get
   Vec     to ()C {return pos  +dir  * length      ;} // get bone ending position
   Flt     toY()C {return pos.y+dir.y* length      ;} // get bone ending position Y
   Vec center ()C {return pos  +dir  *(length*0.5f);} // get bone center position
   Flt centerY()C {return pos.y+dir.y*(length*0.5f);} // get bone center position Y

   // set
   SkeletonBone& setFromTo(C Vec &from, C Vec &to); // set bone position, direction and length according to 'from' and 'to' position parameters and adjust existing 'perp' to match the new settings

   SkeletonBone& operator+=(C Vec     &v);
   SkeletonBone& operator-=(C Vec     &v);
   SkeletonBone& operator*=(C Vec     &v);
   SkeletonBone& operator*=(C Matrix3 &m);
   SkeletonBone& operator/=(C Matrix3 &m);
   SkeletonBone& operator*=(C Matrix  &m);
   SkeletonBone& operator/=(C Matrix  &m);
   SkeletonBone  operator* (C Matrix  &m)C {return SkeletonBone(T)*=m;}

   SkeletonBone& transform(C Matrix3 &matrix) {return T*=matrix;} // transform by matrix
   SkeletonBone& transform(C Matrix  &matrix) {return T*=matrix;} // transform by matrix

   SkeletonBone& mirrorX(); // mirror in X axis
#if EE_PRIVATE
   SkeletonBone& mirrorY(); // mirror in Y axis
   SkeletonBone& mirrorZ(); // mirror in Z axis

   SkeletonBone& rightToLeft(); // convert right to left hand coordinate system
#endif

   // draw
   void draw(C Color &color)C; // this can be optionally called outside of Render function

   // io
   void save(TextNode &node, C Skeleton *owner=null)C; // save as text

   SkeletonBone();
};

typedef SkeletonSlot SkelSlot;
STRUCT( SkeletonSlot , OrientP) // Skeleton Slot
//{
   Char8 name[32]; // name
   Byte  bone    , //           bone index to which slot belongs, 0xFF=none
         bone1   ; // secondary bone index to which slot belongs, 0xFF=none, for best performance this should be set to the same value as 'bone' (to have only one parent), if this is different than 'bone' then slot will be set as average based on 2 bone parents

   void setParent(Byte bone) {T.bone=T.bone1=bone;}
#if EE_PRIVATE
   SkeletonSlot& operator=(C OrientP &ornp) {SCAST(OrientP, T)=ornp; return T;}
#endif

   // io
   void save(TextNode &node, C Skeleton *owner=null)C; // save as text

   SkeletonSlot();
};
/******************************************************************************/
struct Skeleton // Animation Skeleton - base skeleton used by 'AnimatedSkeleton'
{
   Mems<SkelBone> bones; // skeleton bones
   Mems<SkelSlot> slots; // skeleton slots

   // get
   Bool is()C {return bones.elms() || slots.elms();} // if has any data

#if EE_PRIVATE
   Int boneParents(Int bone)C; // get total number of parents that a bone has, 0 if none
   Int boneLevel  (Int bone)C; // get bone level
   Int bonesSharedParent(MemPtr<Byte, 256> bones)C; // get nearest parent which all bones share, or 0xFF if none
   Int hierarchyDistance(Int bone_a, Int bone_b )C; // get hierarchy distance between bones

   SkelAnim* findSkelAnim(C Str &name)C {return _skel_anims.get(name);} // find skeleton animation, null on fail
   SkelAnim* findSkelAnim(C UID &id  )C {return _skel_anims.get(id  );} // find skeleton animation, null on fail
   SkelAnim*  getSkelAnim(C Str &name)C {return _skel_anims    (name);} // get  skeleton animation, Exit on fail
   SkelAnim*  getSkelAnim(C UID &id  )C {return _skel_anims    (id  );} // get  skeleton animation, Exit on fail
#endif
   Int       findBoneI(              BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone      index, -1   on fail
   Byte      findBoneB(              BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone byte index, 255  on fail
   SkelBone* findBone (              BONE_TYPE type, Int type_index=0, Int type_sub=0) ; // find bone           , null on fail
 C SkelBone* findBone (              BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone           , null on fail
   Int       findBoneI(CChar8 *name, BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone      index, -1   on fail, bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching
   SkelBone* findBone (CChar8 *name, BONE_TYPE type, Int type_index=0, Int type_sub=0) ; // find bone           , null on fail, bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching
 C SkelBone* findBone (CChar8 *name, BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone           , null on fail, bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching
   Int        getBoneI(              BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone      index, Exit on fail
   SkelBone&  getBone (              BONE_TYPE type, Int type_index=0, Int type_sub=0) ; // find bone           , Exit on fail
 C SkelBone&  getBone (              BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone           , Exit on fail

   Int       findBoneI(CChar8 *name)C; // find bone      index, -1   on fail
   SkelBone* findBone (CChar8 *name) ; // find bone           , null on fail
 C SkelBone* findBone (CChar8 *name)C; // find bone           , null on fail
   Int       findSlotI(CChar8 *name)C; // find slot      index, -1   on fail
   Byte      findSlotB(CChar8 *name)C; // find slot byte index, 255  on fail
   SkelSlot* findSlot (CChar8 *name) ; // find slot           , null on fail
 C SkelSlot* findSlot (CChar8 *name)C; // find slot           , null on fail
   Int        getBoneI(CChar8 *name)C; // get  bone      index, Exit on fail
   Int        getSlotI(CChar8 *name)C; // get  slot      index, Exit on fail
   SkelBone&  getBone (CChar8 *name) ; // get  bone           , Exit on fail
 C SkelBone&  getBone (CChar8 *name)C; // get  bone           , Exit on fail
   SkelSlot&  getSlot (CChar8 *name) ; // get  slot           , Exit on fail
 C SkelSlot&  getSlot (CChar8 *name)C; // get  slot           , Exit on fail

   Bool contains         (Int parent, Int child   )C; // if 'parent' bone contains 'child' bone
   Int  boneParent       (Int bone                )C; // get bone parent, if bone has no parent, or its parent index is invalid then -1 is returned
   Int  boneRoot         (Int bone                )C; // get bone root  , this iterates bones starting from 'bone' through its parents and returns the last encountered bone that has no parent, if 'bone' has no parent then 'bone' is returned, if 'bone' is not a valid bone index, then -1 is returned
   Int  findParent       (Int bone, BONE_TYPE type)C; // get index of the first parent of 'bone' which has 'type' BONE_TYPE      , -1 is returned if none were found
   Int  findRagdollParent(Int bone                )C; // get index of the first parent of 'bone' which has 'BONE_RAGDOLL' enabled, -1 is returned if none were found

   UInt memUsage()C; // get memory usage

   void getSkin(C Vec &pos, VecB4 &blend, VecB4 &matrix)C; // get bone 'blend matrix' skinning according to 'pos' position

   // transform
   Skeleton& operator+=(C Vec     &move  ) {return T.move     (move  );} // move
   Skeleton& operator*=(C Matrix3 &matrix) {return T.transform(matrix);} // transform by matrix
   Skeleton& operator*=(C Matrix  &matrix) {return T.transform(matrix);} // transform by matrix

   Skeleton& move     (                           C Vec &move); // move
   Skeleton& scale    (C Vec              &scale             ); // scale
   Skeleton& scaleMove(C Vec              &scale, C Vec &move); // scale and move
   Skeleton& transform(C Matrix3          &matrix            ); // transform by matrix
   Skeleton& transform(C Matrix           &matrix            ); // transform by matrix
   Skeleton& animate  (C AnimatedSkeleton &anim_skel         ); // transform bones according to current 'anim_skel' skeleton pose
#if EE_PRIVATE
   Skeleton& mirrorX    (); // mirror in X axis
   Skeleton& mirrorY    (); // mirror in Y axis
   Skeleton& mirrorZ    (); // mirror in Z axis
   Skeleton& rightToLeft(); // convert from right hand to left hand coordinate system
#endif

   // operations
   Skeleton& setBoneTypes (); // automatically set 'SkelBone.type, type_index, type_sub' for all bones in this Skeleton, this method should be called after making changes to skeleton bones
   Skeleton& setBoneShapes(); // automatically set 'SkelBone.shape'                      for all bones in this Skeleton, this method should be called after making changes to skeleton bones
   Bool      setBoneParent(Int child, Int parent, MemPtr<Byte, 256> old_to_new=null); // set 'parent' as the parent of 'child' bone, if 'old_to_new' is passed it will be set as bone remap "old_to_new[old_index]=new_index", false is returned if no change was made (in that case 'old_to_new' will be empty)
   Bool      removeBone   (Int i    ,             MemPtr<Byte, 256> old_to_new=null); // remove i-th bone                          , if 'old_to_new' is passed it will be set as bone remap "old_to_new[old_index]=new_index", false is returned if no change was made (in that case 'old_to_new' will be empty)
   Skeleton& add          (C Skeleton &src      , MemPtr<Byte, 256> old_to_new=null); // add  bones and slots from 'src' to self   , if 'old_to_new' is passed it will be set as bone remap "old_to_new[old_index]=new_index"
   Skeleton& addSlots     (C Skeleton &src                                         ); // add            slots from 'src' to self
   Skeleton& sortBones    (                       MemPtr<Byte, 256> old_to_new=null); // sort bones in parent<->child order and calculate children count and offsets, this is required when changing bone parents, if 'old_to_new' is passed it will be set as bone remap "old_to_new[old_index]=new_index"
#if EE_PRIVATE
   Skeleton& setBoneLengths(                                                       ); // automatically set bone lengths
   void      boneRemap     (                    C MemPtr<Byte, 256> &old_to_new    );
#endif

   // draw
   void draw(C Color &bone_color, C Color &slot_color=TRANSPARENT, Flt slot_size=0.2f)C; // draw bones and slots, this can be optionally called outside of Render function

   // io
   void operator=(C Str &name) ; // load, Exit  on fail
   void operator=(C UID &id  ) ; // load, Exit  on fail
   Bool save     (C Str &name)C; // save, false on fail
   Bool load     (C Str &name) ; // load, false on fail

   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

   void save(MemPtr<TextNode> nodes)C; // save as text

              Skeleton& del(); // delete manually
              Skeleton();
              Skeleton(C Skeleton &src); // create from 'src'
   Skeleton& operator=(C Skeleton &src); // create from 'src'

private:
   mutable Cache<SkelAnim> _skel_anims;
};
/******************************************************************************/
// ANIMATED SKELETON
/******************************************************************************/
typedef AnimatedSkeletonBone AnimSkelBone;
struct  AnimatedSkeletonBone // Bone of an Animated Skeleton
{
   // these parameters may be manually changed during animation process, they are in parent space:
   Orient   orn  ; // target   orientation
   AxisRoll rot  ; // relative rotation
   Vec      pos  , // offset   position
            scale; // scale    factor

   // these parameters may be accessed after animation and matrix updates (using 'updateMatrix' method), they are in world space:
 C Vec   & center()C {return _center;} // bone center position, this parameter may be inaccurate for bones which don't have BONE_RAGDOLL flag enabled
 C Vec   & vel   ()C {return _vel   ;} // bone velocity       , this parameter may be inaccurate for bones which don't have BONE_RAGDOLL flag enabled
 C Matrix& matrix()C {return _matrix;} // this is the transformation matrix, which transforms source bone 'SkelBone' and source 'Mesh' into their final positions (source_data * matrix = final_world_space_position)

   // operations
   void clear(         ); //           clear 'orn rot pos scale'
   void clear(Flt blend); // partially clear 'orn rot pos scale', this method is similar to 'clear()' however it does not perform full reset of the bone. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)

   void forceMatrix(C Matrix &matrix); // force usage of custom transformation 'matrix' for this bone, if used then the bone will ignore its transformations from the animations

#if EE_PRIVATE
   void operator+=(C Vec &d) {_center+=d; _matrix+=d;}
   void zero() {Zero(T);}
#endif

#if !EE_PRIVATE
private:
#endif
   Bool   _disabled, _disabled_children, _force_custom, _world_space_transform;
   Vec    _center, _vel, _fur_vel;
   Matrix _matrix, _force_custom_matrix, _world_space_transform_matrix;
};
/******************************************************************************/
typedef AnimatedSkeleton AnimSkel;
struct  AnimatedSkeleton // Animated Skeleton - used for animating meshes
{
   Flt                     fur_stiffness, // determines the speed of                fur velocities changes,    0..1  , default= 0.0001
                           fur_gravity  , // gravity which affects                  fur velocities        , -Inf..Inf, default=-1
                           fur_vel_scale; // how much does skeleton movement affect fur velocities        , -Inf..Inf, default=-0.75
             AnimSkelBone  root         ; // root transformed skeleton bone
   FixedMems<AnimSkelBone> bones        ; //      transformed skeleton bone array
   FixedMems<OrientP     > slots        ; //      transformed skeleton slot array

   // manage
   AnimatedSkeleton& del   (                                                                                         ); // delete manually
   AnimatedSkeleton& create(const_mem_addr C Skeleton *skeleton, Flt scale=1, C Matrix &initial_matrix=MatrixIdentity); // create from 'skeleton' object, 'scale'=skeleton scale, 'initial_matrix'=matrix set for the root bone (its scale is ignored)
   AnimatedSkeleton& create(         AnimatedSkeleton &src                                                           ); // create from 'src'

   // get
 C Skeleton*     skeleton(     )C {return _skeleton                          ;} // get source   skeleton
   Flt           scale   (     )C {return _scale                             ;} // get animated skeleton scale
   AnimSkelBone& boneRoot(Int i)  {return InRange(i, bones) ? bones[i] : root;} // get i-th transformed bone or root if index is out of range
 C AnimSkelBone& boneRoot(Int i)C {return ConstCast(T).boneRoot(i)           ;} // get i-th transformed bone or root if index is out of range
 C Vec   &       pos     (     )C {return root.matrix().pos                  ;} // get root position
 C Matrix&       matrix  (     )C {return root.matrix()                      ;} // get root matrix
 C Vec   &       vel     (     )C {return root.vel   ()                      ;} // get root velocity

   SkelAnim*     findSkelAnim(C Str    &name                                  )C; // find skeleton    animation, null on fail
   SkelAnim*     findSkelAnim(C UID    &id                                    )C; // find skeleton    animation, null on fail
   AnimSkelBone* findBone    (BONE_TYPE type, Int type_index=0, Int type_sub=0) ; // find bone                 , null on fail
   Int           findBoneI   (BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone        index    , -1   on fail
   Byte          findBoneB   (BONE_TYPE type, Int type_index=0, Int type_sub=0)C; // find bone   byte index    , 255  on fail
   Int           findBoneI   (CChar8   *name                                  )C; // find bone        index    , -1   on fail
   Int           findSlotI   (CChar8   *name                                  )C; // find slot        index    , -1   on fail
   Byte          findSlotB   (CChar8   *name                                  )C; // find slot   byte index    , 255  on fail
   AnimSkelBone* findBone    (CChar8   *name                                  ) ; // find bone                 , null on fail
   OrientP     * findSlot    (CChar8   *name                                  ) ; // find transformed slot     , null on fail, slot will have correct orientation after 'updateMatrix' has been called
   SkelAnim*      getSkelAnim(C Str    &name                                  )C; // get  skeleton    animation, Exit on fail
   SkelAnim*      getSkelAnim(C UID    &id                                    )C; // get  skeleton    animation, Exit on fail
   Int            getBoneI   (CChar8   *name                                  )C; // get  bone        index    , Exit on fail
   Int            getSlotI   (CChar8   *name                                  )C; // get  slot        index    , Exit on fail
   AnimSkelBone*  getBone    (CChar8   *name                                  ) ; // get  bone                 , Exit on fail
   OrientP     *  getSlot    (CChar8   *name                                  ) ; // get  transformed slot     , Exit on fail, slot will have correct orientation after 'updateMatrix' has been called

   void getMatrixes(MemPtrN<Matrix, 256> matrixes)C; // get all bone transformation matrixes, including the root bone as the first one

   // set
   AnimatedSkeleton& disable        (Int i, Bool disable); // disables/enables animation of i-th bone
   AnimatedSkeleton& disableChildren(Int i, Bool disable); // disables/enables animation of i-th bone's children
 
   AnimatedSkeleton& vel(C Vec &vel); // force custom velocity to root and all bones

   // animate
      // prepare
      AnimatedSkeleton& clear(         ); //           clear 'AnimSkelBone' bones 'orn rot pos scale', call this method once before applying all animations to prepare for animating
      AnimatedSkeleton& clear(Flt blend); // partially clear 'AnimSkelBone' bones 'orn rot pos scale',      this method is similar to 'clear()' however it does not perform full reset of the bones. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)

      // modify using animations
      AnimatedSkeleton& animate(C SkelAnim &skel_anim, Flt time, Flt blend=1, Bool replace=false); // modify 'AnimSkelBone' bones 'orn rot pos scale' according to animation object      , 'time'=time position of the animation, 'blend'=blending factor of animation (0..1), if 'replace'=true then previous animations applied to the skeleton will be multiplied by "1-blend" value making them less important
      AnimatedSkeleton& animate(C SkelAnim *skel_anim, Flt time, Flt blend=1, Bool replace=false); // modify 'AnimSkelBone' bones 'orn rot pos scale' according to animation object      , 'time'=time position of the animation, 'blend'=blending factor of animation (0..1), if 'replace'=true then previous animations applied to the skeleton will be multiplied by "1-blend" value making them less important
      AnimatedSkeleton& animate(C Str      &anim_name, Flt time, Flt blend=1, Bool replace=false); // modify 'AnimSkelBone' bones 'orn rot pos scale' according to animation file name   , 'time'=time position of the animation, 'blend'=blending factor of animation (0..1), if 'replace'=true then previous animations applied to the skeleton will be multiplied by "1-blend" value making them less important, prefer using other 'animate' methods as this one is slower
      AnimatedSkeleton& animate(C UID      &anim_id  , Flt time, Flt blend=1, Bool replace=false); // modify 'AnimSkelBone' bones 'orn rot pos scale' according to animation file name ID, 'time'=time position of the animation, 'blend'=blending factor of animation (0..1), if 'replace'=true then previous animations applied to the skeleton will be multiplied by "1-blend" value making them less important, prefer using other 'animate' methods as this one is slower
      AnimatedSkeleton& animate(C Motion   &motion   ,                        Bool replace      ); // modify 'AnimSkelBone' bones 'orn rot pos scale' according to animation motion      ,                                                                                     if 'replace'=true then previous animations applied to the skeleton will be multiplied by "1-blend" value making them less important

      AnimatedSkeleton& animateRoot(C Animation &anim, Flt time); // modify 'AnimSkelBone' root 'orn rot pos scale' according to animation object, 'time'=time position of the animation
      AnimatedSkeleton& animateRoot(C Animation *anim, Flt time); // modify 'AnimSkelBone' root 'orn rot pos scale' according to animation object, 'time'=time position of the animation

      // build final transformation matrixes (this takes into account applied animations and custom modifications)
      AnimatedSkeleton& updateMatrix        (C Matrix &root_matrix=MatrixIdentity          ); // update 'AnimSkelBone' bones 'matrix' according to bones 'orn rot pos scale' and 'root_matrix', this should be called after animating and applying manual modifications, 'root_matrix' must be normalized, this method also sets skeleton slots according to bone matrixes
      AnimatedSkeleton& updateMatrixParents (C Matrix &root_matrix               , Int bone); // update 'AnimSkelBone' bones 'matrix' according to bones 'orn rot pos scale' and 'root_matrix', update occurs only on 'bone' bone and its parents                      , 'root_matrix' must be normalized
      AnimatedSkeleton& updateMatrixChildren(                                      Int bone); // update 'AnimSkelBone' bones 'matrix' according to bones 'orn rot pos scale'                  , update occurs only on 'bone' bone and its children

      // apply custom modifications (this can be called before or after 'updateMatrix' methods)
      AnimatedSkeleton&           forceMatrix(Int bone, C Matrix  &matrix, Bool auto_update_matrixes=true); // force usage of custom transformation 'matrix' for 'bone', if used then the bone will ignore its transformations from the animations, if 'auto_update_matrixes' is set to true then 'updateMatrixChildren(bone)' will be called automatically
      AnimatedSkeleton& transformInWorldSpace(Int bone, C Matrix3 &matrix, Bool auto_update_matrixes=true); // transform bone by world space 'matrix', if 'auto_update_matrixes' is set to true then 'updateMatrixChildren(bone)' will be called automatically
      AnimatedSkeleton& transformInWorldSpace(Int bone, C Matrix  &matrix, Bool auto_update_matrixes=true); // transform bone by world space 'matrix', if 'auto_update_matrixes' is set to true then 'updateMatrixChildren(bone)' will be called automatically

   // update
   void updateVelocities(Bool according_to_physics_step=true, Bool ragdoll_bones_only=true); // update velocities 'vel, bone.vel' and bone center positions 'bone.center' according to root and bone matrixes 'matrix bone[].matrix'. This needs to be called once per frame after all animations, modifications and matrix updates ('updateMatrix'). 'according_to_physics_step' if update the velocities according to the physics update time step in this frame, if it's set to false, then the velocities will always be calculated. 'ragdoll_bones_only'=if calculate precise velocities only for bones which have 'BONE_RAGDOLL' flag enabled

   // transform
   void move  (C Vec &delta); // move the whole skeleton                                                    , this method is to be used for distant      position modifications, it will not affect bone velocities, you can call this method optionally after matrix updates ('updateMatrix')
   void offset(C Vec &delta); // apply offset to root matrix, bone matrixes, and transformed slots by vector, this method is to be used for smooth local position modifications, it will     affect bone velocities, you can call this method optionally after matrix updates ('updateMatrix')

   // draw
   void setMatrix()C; // set active rendering matrixes and velocities ('matrix, bone.matrix, vel, bone.vel') to the GPU shader data, call this right before drawing skinned meshes (when using mesh draw methods which don't accept matrix or skeleton parameter, if they do accept such parameters, then those methods will automatically set proper matrixes and you don't need to call 'setMatrix' manually)
   void draw     (C Color &bone_color, C Color &slot_color=TRANSPARENT)C; // draw animated bones and slots, this can be optionally called outside of Render function

   // io
   Bool save(File &f)C; // save, does not include current animation pose, false on fail
   Bool load(File &f) ; // load, does not include current animation pose, false on fail

#if EE_PRIVATE
   void zero();
   void setFurVel()C; // set fur velocities
   Int  minBones ()C {return Min(bones.elms(), skeleton()->bones.elms());} // !! this does not check for "skeleton!=null" !!
   Int  minSlots ()C {return Min(slots.elms(), skeleton()->slots.elms());} // !! this does not check for "skeleton!=null" !!
   AnimatedSkeleton& animateExactTime(C SkelAnim &skel_anim, Flt time); // this will not apply fraction for time, this function is needed when adjusting animations to make sure we process exact keyframes based on time, but still with looping support
#endif

   AnimatedSkeleton();

#if !EE_PRIVATE
private:
#endif
   Bool      _updated_vel;
   Flt       _scale;
 C Skeleton *_skeleton;
   struct Instance
   {
      Int solid, blend, shadow;
      Instance() {solid=blend=shadow=-1;}
   }mutable _instance;
};
/******************************************************************************/
struct BoneMap
{
#if EE_PRIVATE
   void create(C Skeleton &skeleton); // create from 'skeleton'
   Int  alloc (Int bones, Int name_size); // allocate memory and return its total size

   Bool is()C {return _bones>0;} // if has any data

   Int    nameSize (     )C; // get size needed for bone names
   Char8* nameStart(     )C; // get start of the bone name memory
  CChar8* name     (Int i)C; // get i-th bone name

   Int find(CChar8 *name                                              )C; // find bone, -1 on fail
   Int find(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C; // find bone, -1 on fail

   Bool same(C Skeleton &skeleton)C; // if contains the same data as the 'skeleton'

   void    remap(                    C MemPtr<Byte, 256> &old_to_new                    ) ; //     remap
   void setRemap(C Skeleton &skeleton, MemPtr<Byte, 256>  old_to_new, Bool by_name=false)C; // set remap that maps from current bone mapping to 'skeleton' bone mapping, 'by_name'=if remap by name only and ignore type/indexes

   Bool save(File &f)C;
   Bool load(File &f) ;

   Bool saveOld1(File &f)C;
   Bool loadOld1(File &f) ;
   Bool saveOld (File &f)C;
   Bool loadOld (File &f) ;
#endif

          void       del();
         ~BoneMap() {del();}
          BoneMap() {_bone=null; _bones=0;}
          BoneMap(C BoneMap &src);
   void operator=(C BoneMap &src);

private:
#if EE_PRIVATE
   struct Bone
   {
      BONE_TYPE type       ;
      SByte     type_index ;
      Byte      type_sub   ;
      Byte      parent     ;
      U16       name_offset;
   };
   Bone *_bone; // right after '_bone' array, array of bone names is allocated (using Char8)
#else
   Ptr _bone;
#endif
   Int _bones;
};
/******************************************************************************/
extern Cache<Skeleton> Skeletons; // Skeleton Cache
/******************************************************************************/
CChar8* BoneName(BONE_TYPE type); // get name of the specified bone type
/******************************************************************************/
#if EE_PRIVATE
   #define VIRTUAL_ROOT_BONE 1 // if set to 1, then: mesh.vtx.matrix=0 is a special virtual root bone, and bone indexes start from 1, matrix=bone+1 -> matrix=1 is bone=0, matrix=2 is bone=1, ..; if VIRTUAL_ROOT_BONE set to 0, then there's no virtual root bone, and vtx matrix is the same as bone index, currently only VIRTUAL_ROOT_BONE==1 is supported so don't change !!
#endif
/******************************************************************************/
