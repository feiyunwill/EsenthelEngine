/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
static SkelAnim* GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim)
{
   if(obj)if(Param *param=obj->findParam(anim))return anim_skel.findSkelAnim(param->asID());
   return null;
}
void Chr::SkelAnimCache::set(AnimatedSkeleton &anim_skel, C ObjectPtr &obj)
{
   // set default animations
   fist_l   =GetAnim(anim_skel, obj, "anim fist left");
   fist_r   =GetAnim(anim_skel, obj, "anim fist right");
   stand    =GetAnim(anim_skel, obj, "anim stand");
   crouch   =GetAnim(anim_skel, obj, "anim crouch");
   turn_l   =GetAnim(anim_skel, obj, "anim turn left");
   turn_r   =GetAnim(anim_skel, obj, "anim turn right");
   walk     =GetAnim(anim_skel, obj, "anim walk");
   run      =GetAnim(anim_skel, obj, "anim run");
   crouch_f =GetAnim(anim_skel, obj, "anim crouch forward");
   crouch_l =GetAnim(anim_skel, obj, "anim crouch left");
   crouch_r =GetAnim(anim_skel, obj, "anim crouch right");
   strafe_wl=GetAnim(anim_skel, obj, "anim walk left");
   strafe_wr=GetAnim(anim_skel, obj, "anim walk right");
   strafe_l =GetAnim(anim_skel, obj, "anim run left");
   strafe_r =GetAnim(anim_skel, obj, "anim run right");
   dodge_l  =GetAnim(anim_skel, obj, "anim dodge left");
   dodge_r  =GetAnim(anim_skel, obj, "anim dodge right");
   floating =GetAnim(anim_skel, obj, "anim float");

   // detect indexes of skeleton bones
   head  =anim_skel.findBoneI(BONE_HEAD);
   neck  =anim_skel.findBoneI(BONE_NECK);
   body  =anim_skel.findBoneI(BONE_SPINE);
   arm_lu=anim_skel.findBoneI(BONE_UPPER_ARM, -1);
   arm_ru=anim_skel.findBoneI(BONE_UPPER_ARM,  0);
   arm_ld=anim_skel.findBoneI(BONE_FOREARM  , -1);
   arm_rd=anim_skel.findBoneI(BONE_FOREARM  ,  0);
   leg_lu=anim_skel.findBoneI(BONE_UPPER_LEG, -1);
   leg_ru=anim_skel.findBoneI(BONE_UPPER_LEG,  0);
   leg_ld=anim_skel.findBoneI(BONE_LOWER_LEG, -1);
   leg_rd=anim_skel.findBoneI(BONE_LOWER_LEG,  0);
   hand_l=anim_skel.findBoneI(BONE_HAND     , -1);
   hand_r=anim_skel.findBoneI(BONE_HAND     ,  0);
   toe_l =anim_skel.findBoneI(BONE_TOE      , -1);
   toe_r =anim_skel.findBoneI(BONE_TOE      ,  0);
   if(C Skeleton *skel=anim_skel.skeleton())
   {
      Int spines=0; REPA(skel->bones)if(skel->bones[i].type==BONE_SPINE)MAX(spines, skel->bones[i].type_sub); // skeletons may have many spine bones, calculate how many
      body_u=anim_skel.findBoneI(BONE_SPINE, 0, (spines+1)/2); // chose spine bone in the middle
   }else body_u=-1;
}
/******************************************************************************/
Flt Chr::animateFaceBlend() // facial animation blending value, dependent on distance from camera, returns value 0..1 (0 disables facial animation)
{
   Flt distance        =Dist2(pos(), ActiveCam.matrix.pos),
       full_blend_range=Sqr  (3),
         no_blend_range=Sqr  (4);

   return LerpRS(no_blend_range, full_blend_range, distance);
}
Bool Chr::animateFingers() // if animate hand/foot fingers, dependent on distance from camera, returns value true/false (false disables fingers animation)
{
   Flt    distance2 =Dist2(pos(), ActiveCam.matrix.pos);
   return distance2<=Sqr  (14);
}
Flt Chr::animateAlignFeet() // how much to align feet on to the ground, returns value 0..1 (0 disables feet aligning)
{
   if(anim.fly<1-EPS_ANIM_BLEND) // don't align when flying
   {
      Flt strafe=   (anim.stop_move)*(  anim.straight_strafe),
          crouch=(1-(anim.stop_move)*(1-anim.   stand_crouch))*Sat(anim.stand_crouch*10);

      Flt align=strafe;

      align =Lerp(align, 1.0f, crouch);
      align*=1-anim.fly;
      return align;
   }
   return 0;
}
/******************************************************************************/
static void AlignFeet(Chr &chr, Flt step) // align character feet by 'step'=0..1 factor
{
   if(step>EPS_ANIM_BLEND && chr.sac.toe_l>=0 && chr.sac.toe_r>=0)
   {
      Flt y = chr.ctrl.center().y-chr.ctrl.height()*0.5f,                                                // get controller bottom Y position
          yl=(chr.skel.skeleton()->bones[chr.sac.toe_l].pos * chr.skel.bones[chr.sac.toe_l].matrix()).y, // get left  foot bottom Y position
          yr=(chr.skel.skeleton()->bones[chr.sac.toe_r].pos * chr.skel.bones[chr.sac.toe_r].matrix()).y; // get right foot bottom Y position
      chr.skel.offset(Vec(0, (y-Min(yl, yr)+0.03f)*step, 0));                                            // offset the animation skeleton according to feet placement
   }
}
/******************************************************************************/
void Chr::updateAnimationAnim() // update all 'Chr::anim' parameters
{
   Bool set=(anim.stop_move==0.0f); // 'set' is a helper value for setting animation blending values (which are in range 0..1) immediately to 1
   Flt  dt = Time.d()*anim.speed  ; // animations time delta

   // optimizations
   {
      Bool disable;

      // disable fingers animations
      disable=(animateFingers()==0);
      skel.disable        (sac.toe_l , disable);
      skel.disable        (sac.toe_r , disable);
      skel.disableChildren(sac.hand_l, disable);
      skel.disableChildren(sac.hand_r, disable);

      // disable facial animations
      disable=(animateFaceBlend()<=0);
      skel.disableChildren(sac.head, disable);
   }

   // turn
   {
      AdjustValBool(anim.b_turn_l, input.turn.x>0 && !ctrl.flying(), Time.d()*15, Time.d()*4);
      AdjustValBool(anim.b_turn_r, input.turn.x<0 && !ctrl.flying(), Time.d()*15, Time.d()*4);

      if(!anim.b_turn_l)anim.t_turn_l=0;else anim.t_turn_l+=Abs(input_turn.x)*Time.d();
      if(!anim.b_turn_r)anim.t_turn_r=0;else anim.t_turn_r+=Abs(input_turn.x)*Time.d();
   }

   // walk <-> run
   Bool running=false;
   if(input.move.x || input.move.z || (input.move.y && ctrl.flying()))
   {
      running=!(input.walk || input.crouch || ctrl.crouched());
      AdjustValBoolSet(anim.walk_run, running, set, Time.d()*6);
   }

   // forward <-> back
   if(input.move.z)
   {
      AdjustValBoolSet(anim.forward_back, input.move.z<0, set, Time.d()*4);
   }

   // strafe
   {
                                      AdjustValDir    (anim.strafe_yaw     ,  input.move.z*input.move.x*!ctrl.flying(),      Time.d()*6);
      if(input.move.x                )AdjustValBoolSet(anim.left_right     ,  input.move.x==1                         , set, Time.d()*6);
      if(input.move.x || input.move.z)AdjustValBoolSet(anim.straight_strafe, !input.move.z && input.move.x            , set, Time.d()*6);
   }

   // stand <-> crouch
   AdjustValBool(anim.stand_crouch, input.crouch || ctrl.crouched(), Time.d()*2, Time.d()*1.5f);

   // stop <-> move
   AdjustValBool(anim.stop_move, (input.move.x || input.move.z) && !ctrl.flying(), Time.d()*6);

   // lean
   {
      Flt desired_lean=Mid(input_turn.x, -1.0f, 1.0f)*anim.stop_move*Lerp(0.035f, 0.12f, anim.walk_run)*!ctrl.flying();
      AdjustValTime(anim.lean, desired_lean, 0.000001f);
   }

   // flying
   {
      AdjustValTime(anim.fly, ctrl.flying(), 0.001f);
      if(anim.fly>EPS_ANIM_BLEND)
      {
         Bool fly_full_body=(input.move.z==1 && running);

         Flt desired_strafe=(fly_full_body ?     input_move.x*PI_4 : 0),
             desired_x     =(fly_full_body ? 0 : input_move.x*-0.3f   ),
             desired_z     =(fly_full_body ? (PI_2-0.2f)-Max(input_move.y, -0.5f)*PI_4 : input_move.z*0.3f); // use 'Max' to avoid "neck breaks"

         AdjustValTime(anim.fly_x        , desired_x                            , 0.02f);
         AdjustValTime(anim.fly_z        , desired_z                            , 0.02f);
         AdjustValTime(anim.fly_strafe   , desired_strafe                       , 0.02f);
         AdjustValTime(anim.fly_full_body, fly_full_body                        , 0.02f);
         AdjustValTime(anim.fly_turn     , fly_full_body ? input.turn.x*PI_6 : 0, 0.05f);
      }else
      {
         anim.fly_x        =0;
         anim.fly_z        =0;
         anim.fly_strafe   =0;
         anim.fly_turn     =0;
         anim.fly_full_body=0;
      }
   }

   if(set)anim.time =0 ;
   else   anim.time+=dt;
}
/******************************************************************************/
static Flt AnimTime(Chr &chr) // get global animation time for the character according to current time and character's 'unique' value
{
   return Time.time()+chr.anim.unique*10;
}
void Chr::animate()
{
   // animation blend values
   Flt b_stand   =           (1-anim.stand_crouch   )*(1-anim.stop_move ),
       b_crc_stop=           (  anim.stand_crouch   )*(1-anim.stop_move ),
       b_crc_move=           (  anim.stand_crouch   )*(  anim.stop_move ),
       b_turn_l  =           (1-anim.stand_crouch   )*(1-anim.stop_move )*(  anim.b_turn_l),
       b_turn_r  =           (1-anim.stand_crouch   )*(1-anim.stop_move )*(  anim.b_turn_r),
       b_walk    =           (1-anim.stand_crouch   )*(  anim.stop_move )*(1-anim.walk_run),
       b_run     =           (1-anim.stand_crouch   )*(  anim.stop_move )*(  anim.walk_run),
       b_walk_z  =b_walk    *(1-anim.straight_strafe)                    ,
       b_run_z   =b_run     *(1-anim.straight_strafe)                    ,
       b_crc_z   =b_crc_move*(1-anim.straight_strafe)                    ,
       b_walk_f  =b_walk_z  *(1-anim.forward_back   )                    ,
       b_walk_b  =b_walk_z  *(  anim.forward_back   )                    ,
       b_run_f   =b_run_z   *(1-anim.forward_back   )                    ,
       b_run_b   =b_run_z   *(  anim.forward_back   )                    ,
       b_crc_f   =b_crc_z   *(1-anim.forward_back   )                    ,
       b_crc_b   =b_crc_z   *(  anim.forward_back   )                    ,
       b_crc_l   =b_crc_move*(  anim.straight_strafe)*(1-anim.left_right),
       b_crc_r   =b_crc_move*(  anim.straight_strafe)*(  anim.left_right),
       b_walk_l  =b_walk    *(  anim.straight_strafe)*(1-anim.left_right),
       b_walk_r  =b_walk    *(  anim.straight_strafe)*(  anim.left_right),
       b_run_l   =b_run     *(  anim.straight_strafe)*(1-anim.left_right),
       b_run_r   =b_run     *(  anim.straight_strafe)*(  anim.left_right);

   b_turn_l=LerpSmoothPow(0, 1, b_turn_l, 2.5f);
   b_turn_r=LerpSmoothPow(0, 1, b_turn_r, 2.5f);

   // detect "step" sound (event when character foot landed on the ground while moving)
   if(ctrl.onGround() && anim.stop_move>0.25f)
   {
      Flt       blend    =0;
      SkelAnim *skel_anim=null;

      // find most significant movement animation
      if(b_crc_z >blend){blend=b_crc_z ; skel_anim=sac.crouch_f ;}
      if(b_walk_z>blend){blend=b_walk_z; skel_anim=sac.walk     ;}
      if(b_run_z >blend){blend=b_run_z ; skel_anim=sac.run      ;}
      if(b_crc_l >blend){blend=b_crc_l ; skel_anim=sac.crouch_l ;}
      if(b_crc_r >blend){blend=b_crc_r ; skel_anim=sac.crouch_r ;}
      if(b_walk_l>blend){blend=b_walk_l; skel_anim=sac.strafe_wl;}
      if(b_walk_r>blend){blend=b_walk_r; skel_anim=sac.strafe_wr;}
      if(b_run_l >blend){blend=b_run_l ; skel_anim=sac.strafe_l ;}
      if(b_run_r >blend){blend=b_run_r ; skel_anim=sac.strafe_r ;}

      // check if "step" event occurred
      if(skel_anim && blend>0.25f)
      {
         Flt dt=Time.d()*anim.speed;
         if(skel_anim->eventOccurred("step", anim.time-dt, dt))animateStepOccurred(); // anim.time was already updated before this method was called so we need to set it back (-dt)
      }
   }

   Flt time=AnimTime(T);

   // animate the skeleton
              skel.clear()
                  .animate(sac.fist_r   , time         , 1)
                  .animate(sac.fist_l   , time         , 1)
                  .animate(sac.stand    , time         , b_stand)
                  .animate(sac.crouch   , time         , b_crc_stop)
                  .animate(sac.turn_l   , anim.t_turn_l, b_turn_l, true)
                  .animate(sac.turn_r   , anim.t_turn_r, b_turn_r, true)
                  .animate(sac.walk     , anim.time    , b_walk_f)
                  .animate(sac.walk     ,-anim.time    , b_walk_b)
                  .animate(sac.run      , anim.time    , b_run_f)
                  .animate(sac.run      ,-anim.time    , b_run_b)
                  .animate(sac.crouch_f , anim.time    , b_crc_f)
                  .animate(sac.crouch_f ,-anim.time    , b_crc_b)
                  .animate(sac.crouch_l , anim.time    , b_crc_l)
                  .animate(sac.crouch_r , anim.time    , b_crc_r)
                  .animate(sac.strafe_wl, anim.time    , b_walk_l)
                  .animate(sac.strafe_wr, anim.time    , b_walk_r)
                  .animate(sac.strafe_l , anim.time    , b_run_l)
                  .animate(sac.strafe_r , anim.time    , b_run_r)
                  .animate(sac.floating , time         , anim.fly, true);
   if(dodging)skel.animate((dodging<0) ? sac.dodge_l : sac.dodge_r, 1-dodge_step, Sqrt(Sin(dodge_step*PI)), true);
}
/******************************************************************************/
void Chr::updateAnimation()
{
   updateAnimationAnim();

   // first animate using only basic animations (standing or crouching) to detect 'foot_offset' (it will be used to align the whole body matrix while crouching so that the foot will always be in constant position - "attached" to certain position on the ground)
   Bool use_foot_offset=false;
   Vec      foot_offset;
   if(anim.fly<1-EPS_ANIM_BLEND         // only when not flying
   && sac.toe_l>=0                      // left foot was detected
   && anim.stand_crouch>EPS_ANIM_BLEND) // crouching
   {
      use_foot_offset=true;

      Flt time    =AnimTime(T);
      Vec pos_foot=skel.skeleton()->bones[sac.toe_l].pos;
      skel.clear  ();
      skel.animate(sac.stand , time, 1                      ).updateMatrixParents(MatrixIdentity, sac.toe_l); foot_offset=            pos_foot*skel.bones[sac.toe_l].matrix();
      skel.animate(sac.crouch, time, anim.stand_crouch, true).updateMatrixParents(MatrixIdentity, sac.toe_l); foot_offset=foot_offset-pos_foot*skel.bones[sac.toe_l].matrix(); foot_offset.y=0;
   }

   // set animations
   animate();

   // manually adjust body, neck and head bones (according to look angles, movement, ...)
   Int body_sign      =((sac.body>=0) ? Sign(skel.skeleton()->bones[sac.body].perp.z) : 1);
   Flt body_pitch     =angle.y*body_sign;
       body_pitch    *=1-anim.fly_full_body*anim.fly; // when flying with full body, disable body pitch bending, and rotate main matrix instead
   Flt body_pitch_part=body_pitch/(4+anim.fly), // body_pitch rotation is split into 4 bones rotations (body,body_u,neck,head), when flying also the main matrix is used (making a total of 5 splits)
       head_neck_pitch=body_pitch_part + anim.fly*anim.fly_z*0.5f*body_sign; // add flying adjustment split into 2 bones
   {
      Flt body_roll     =(-PI_4/4)*anim.strafe_yaw, // body roll is split into 4 bones (body, body_u, neck, head)
          head_neck_roll=body_roll;//-angle_head/2

      Vec     v;
      Matrix3 m;

      // body
      v=skel.root.orn.cross(); if(!v.any())v.x=1; CHS(v.z); // body x-axis
      if(sac.body>=0){m.setRotate(v, body_pitch_part*-body_sign).rotateY(body_roll); skel.bones[sac.body].orn*=m;}

      // body_u
      if(sac.head>=0)
      {
         v=skel.bones[sac.head].orn.cross(); // lucky guess for upper bones rotation
         if(sac.body_u>=0){m.setRotate(v, body_pitch_part).rotateZ(body_roll); skel.bones[sac.body_u].orn*=m;}
         
         // neck + head
         m.setRotate(v, head_neck_pitch).rotateZ(head_neck_roll);
         if(sac.neck>=0)skel.bones[sac.neck].orn*=m;
                        skel.bones[sac.head].orn*=m;
                        skel.bones[sac.head].rot.axis.z-=anim.fly_strafe*anim.fly;
      }
   }

   // main matrix
   Matrix m;
   {
      Flt body_pitch_rest=body_pitch-4*body_pitch_part; // part of the 'body_pitch' which was left to set in the main matrix

      // orientation
      m.orn().setRotateY( anim.fly*anim.fly_turn + anim.strafe_yaw*PI_4)
             .   rotateZ( anim.lean*(1-anim.straight_strafe)                       + anim.fly*(anim.fly_x - anim.fly_strafe           )                             )
             .   rotateX( anim.lean*(  anim.straight_strafe)*(anim.left_right*2-1) + anim.fly*(anim.fly_z - anim.fly_full_body*angle.y) + body_pitch_rest*-body_sign)
             .   rotateY(-angle.x);

      // position
      m.pos =ctrl.actor.pos()                               ; // set initial position from the controller actor
      m.pos+=ctrl.shapeOffset() - ctrl.shapeOffset()*m.orn(); // this operation forces the custom 'm.orn' rotations to be done relative to the center of character controller

      if(use_foot_offset)
      {
                foot_offset*=m.orn();
         m.pos+=foot_offset;
      }

      // adjust legs flying fake movement
      if(anim.fly>EPS_ANIM_BLEND)
      {
         Vec vec   =ctrl.actor.vel(); vec.y=-6;
         Flt angle =AbsAngleBetween(vec, Vec(0,-1,0));
             angle*=0.2f;
             angle =Log2(angle+1);
             angle*=(1-anim.fly_full_body)*anim.fly;
         if( angle>EPS)
         {
            MIN(angle, 0.4f);
            Vec axis=CrossN(vec, Vec(0,-1,0))/m.orn()*angle;

            if(sac.leg_lu>=0)skel.bones[sac.leg_lu].rot.axis+=axis;
            if(sac.leg_ru>=0)skel.bones[sac.leg_ru].rot.axis+=axis;
            if(axis.x>EPS)
            {
               if(sac.leg_ld>=0)skel.bones[sac.leg_ld].rot.axis.x+=axis.x;
               if(sac.leg_rd>=0)skel.bones[sac.leg_rd].rot.axis.x+=axis.x;
            }
         }
      }
   }

   // set the skeleton according to the matrix
   animateUpdateMatrix(m);

   // align feet
   AlignFeet(T, animateAlignFeet());

   // blend the ragdoll with the skeleton
   if(ragdoll_mode==RAGDOLL_PART)
   {
      Flt length=0.34f;
         ragdoll_time+=Time.d();
      if(ragdoll_time>=length)
      {
         ragdoll_mode=RAGDOLL_NONE;
         ragdoll.active(false);
      }else
      {
         Flt step =ragdoll_time/length,
             blend=1-step*step; if(step<0.1f)blend*=step/0.1f;
         ragdoll.toSkelBlend(skel, blend*0.45f);
      }
   }
}
/******************************************************************************/
}}
/******************************************************************************/
