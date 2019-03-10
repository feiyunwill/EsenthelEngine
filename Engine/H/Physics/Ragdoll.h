/******************************************************************************

   Use 'Ragdoll' to simplify management of character physical bones representation.

   It can automatically create bone 'Actors' linked with 'Joints' from a skeleton,
      and handle blending with the 'AnimatedSkeleton' animations.

/******************************************************************************/
struct Ragdoll // Physical Ragdoll, set of Bone Actors linked with Joints
{
   struct Bone // ragdoll bone
   {
      Char8 name[32]; // name
      Actor actor   ; // actor

   #if !EE_PRIVATE
   private:
   #endif
      Byte skel_bone, rbon_parent;
   };

   // manage
   Ragdoll& del      (                                                                  ); // delete
   Ragdoll& create   (C AnimatedSkeleton &anim_skel, Flt density=1, Bool kinematic=false); // create from animated skeleton, 'kinematic'=if create the bone actors as kinematic (additionally this will not create joints), Exit  on fail
   Bool     createTry(C AnimatedSkeleton &anim_skel, Flt density=1, Bool kinematic=false); // create from animated skeleton, 'kinematic'=if create the bone actors as kinematic (additionally this will not create joints), false on fail

   // operations
   Ragdoll& fromSkel     (C AnimatedSkeleton &anim_skel, C Vec &vel=VecZero, Bool immediate_even_for_kinematic_ragdoll=false); // set   ragdoll  from skeleton          , 'anim_skel' must have its matrixes updated, 'anim_skel' must be set to the same skeleton which ragdoll was created from, setting ragdoll bone matrixes is done using 'Actor::kinematicMoveTo' for kinematic ragdolls and 'Actor::matrix' for non kinematic ragdolls, however since 'Actor::kinematicMoveTo' doesn't set the matrixes immediately, you can set 'immediate_even_for_kinematic_ragdoll' to true, which will force setting the bone matrixes immediately using 'Actor::matrix' method.
   Ragdoll&   toSkel     (  AnimatedSkeleton &anim_skel                                                                     ); // set   skeleton from ragdoll                                                       , 'anim_skel' must be set to the same skeleton which ragdoll was created from
   Ragdoll&   toSkelBlend(  AnimatedSkeleton &anim_skel, Flt blend                                                          ); // blend ragdoll animations into skeleton, 'anim_skel' must have its matrixes updated, 'anim_skel' must be set to the same skeleton which ragdoll was created from

   // get / set
   Bool is()C {return _skel!=null;} // if  created

   Int   bones(     )C {return _bones.elms();} // get number of bones
   Bone& bone (Int i)  {return _bones[i]    ;} // get i-th      bone
 C Bone& bone (Int i)C {return _bones[i]    ;} // get i-th      bone

   Vec       pos        ()C;   Ragdoll& pos        (C Vec    &pos      ); // get/set ragdoll position, position is taken from the main bone
   Vec       vel        ()C;   Ragdoll& vel        (C Vec    &vel      ); // get/set ragdoll velocity, velocity is taken from the main bone, however setting velocity applies to all ragdoll bones equally
   Flt        damping   ()C;   Ragdoll&  damping   (Flt       damping  ); // get/set linear  damping, 0..Inf, default=0.05
   Flt       adamping   ()C;   Ragdoll& adamping   (Flt       damping  ); // get/set angular damping, 0..Inf, default=0.05
   Bool      kinematic  ()C;   Ragdoll& kinematic  (Bool      on       ); // get/set if kinematic, only dynamic actors (with mass!=0) can be changed into kinematic actors
   Bool      gravity    ()C;   Ragdoll& gravity    (Bool      on       ); // get/set if gravity is enabled for this ragdoll
   Bool      ray        ()C;   Ragdoll& ray        (Bool      on       ); // get/set if this ragdoll should be included when performing ray tests
   Bool      collision  ()C;   Ragdoll& collision  (Bool      on       ); // get/set if this ragdoll should collide with other actors in the world
   Bool      sleep      ()C;   Ragdoll& sleep      (Bool      sleep    ); // get/set sleeping
   Flt       sleepEnergy()C;   Ragdoll& sleepEnergy(Flt       energy   ); // get/set the amount of energy below the ragdoll is put to sleep, default=0.1
   Bool      ccd        ()C;   Ragdoll& ccd        (Bool      on       ); // get/set continuous collision detection
   Ptr       user       ()C;   Ragdoll& user       (Ptr       user     ); // get/set user data
   Ptr       obj        ()C;   Ragdoll& obj        (Ptr       obj      ); // get/set pointer to object containing the ragdoll
   Byte      group      ()C;   Ragdoll& group      (Byte      group    ); // get/set collision group (0..31, ACTOR_GROUP)
   Byte      dominance  ()C;   Ragdoll& dominance  (Byte      dominance); // get/set dominance index (0..31, default=0), for more information about dominance please check comments on 'Physics.dominance' method
   PhysMtrl* material   ()C;   Ragdoll& material   (PhysMtrl *material ); // get/set physics material (use 'null' for default material)

   Ragdoll& active(Bool   on                     ); // set if active by calling 'Actor::active' on all ragdoll bone actors
   Ragdoll& ignore(Actor &actor, Bool ignore=true); // ignore collisions with 'actor'

   Int   findBoneI(CChar8 *name); // find ragdoll bone index, -1   on fail
   Bone* findBone (CChar8 *name); // find ragdoll bone      , null on fail
   Int    getBoneI(CChar8 *name); // get  ragdoll bone index, Exit on fail
   Bone&  getBone (CChar8 *name); // get  ragdoll bone      , Exit on fail

   Int findBoneIndexFromSkelBone (Byte skel_bone_index)C; // find ragdoll bone index, from skeleton bone index, -1 on fail
   Int findBoneIndexFromVtxMatrix(Byte    matrix_index)C; // find ragdoll bone index, from vertex matrix index, -1 on fail

   // draw
   void draw(C Color &color=WHITE)C; // this can be optionally called outside of Render function

   // io
   Bool saveState(File &f, Bool include_matrix_vel=true)C; // save ragdoll state (following data is not  saved: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material), false on fail, 'include_matrix_vel'=include current bone matrixes and velocities
   Bool loadState(File &f                              ) ; // load ragdoll state (following data is not loaded: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material), false on fail, typically you should first create a Ragdoll and then call this method to set its state according to data from the file

#if EE_PRIVATE
   void zero();
#endif

  ~Ragdoll() {del();}
   Ragdoll();

#if !EE_PRIVATE
private:
#endif
   Flt         _scale ;
 C Skeleton   *_skel  ;
   Mems<Bone > _bones ;
   Memc<Int  > _resets;
   Memc<Joint> _joints;
   Aggregate   _aggr  ;
};
/******************************************************************************/
inline Int Elms(C Ragdoll &ragdoll) {return ragdoll.bones();}
/******************************************************************************/
