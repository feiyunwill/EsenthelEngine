/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   On PhysX because Controller's rotation is frozen using joints,
      it may sometimes appear to "rotate" (be out of sync/jitter/jerky)

   PhysX may not report all faces of a mesh when sweeping.

/******************************************************************************/
void Controller::zero()
{
   fall_control=false;
   height_crouched=step_height=0;
  _on_ground=_crouched=_jumping=_flying=false;
  _radius=_height=_height_cur=_time_in_air=_time_jump=0;
  _vel_prev=_step_normal=_shape_offset=_ground_plane.pos=_ground_plane.normal=0;
}
Controller::Controller() {zero();}
/******************************************************************************/
Controller& Controller::del()
{
   actor.del();
   zero(); return T;
}
Controller& Controller::createCapsule(Flt radius, Flt height, C Vec &pos, C Vec *anchor)
{
   del();

   T._radius              =Max(Physics.minShapeRadius(), Min(radius, (height-Physics.minCapsuleEdge())*0.5f));
   T._height=T._height_cur=Max(2*T._radius+Physics.minCapsuleEdge(), height);
      height_crouched     =T._height*0.67f;
      step_height         =T._radius;
  _shape_offset           =(anchor ? pos-*anchor : 0);
   actor.create   (Capsule(T._radius, T._height, pos), 1, anchor)
        .freezeRot(true)
        .material (&Physics.mtrl_ctrl)
        .group    (AG_CONTROLLER);
   return T;
}
/******************************************************************************/
#if PHYSX
void Controller::capsuleHeight(Flt height)
{
   PxShape *shape; if(actor._actor)if(actor._actor->getShapes(&shape, 1))
   {
      PxCapsuleGeometry capsule; if(shape->getCapsuleGeometry(capsule))
      {
         T._height_cur=Max(2*T._radius+Physics.minCapsuleEdge(), height);
         capsule.halfHeight=T._height_cur*0.5f-T._radius;
         shape->setGeometry(capsule);
         PxTransform	pose=shape->getLocalPose(); pose.p=Physx.vec(_shape_offset + Vec(0, (T._height_cur-T._height)*0.5f, 0)); shape->setLocalPose(pose);
         Physx.world->resetFiltering(*actor._actor); // this call is needed, because when standing up from crouched position, the controller jumps
      }
   }
}
void Controller::radius(Flt radius)
{
   PxShape *shape; if(actor._actor)if(actor._actor->getShapes(&shape, 1))
   {
      PxCapsuleGeometry capsule; if(shape->getCapsuleGeometry(capsule))
      {
         T._radius=Mid(radius, Physics.minShapeRadius(), (T._height-Physics.minCapsuleEdge())*0.5f);
         capsule.radius=T._radius;
         shape->setGeometry(capsule);
         capsuleHeight(T._height_cur);
      }
   }
}
#else
void Controller::capsuleHeight(Flt height)
{
   if(actor._actor)if(btCapsuleShape *shape=CAST(btCapsuleShape, actor._actor->getCollisionShape()))
   {
      T._height_cur=Max(2*T._radius+Physics.minCapsuleEdge(), height);
      shape->setImplicitShapeDimensions(btVector3(T._radius, T._height_cur*0.5f-T._radius, T._radius));
      Flt com_offset           =(T._height-T._height_cur)*0.5f, // center of mass offset (to match PhysX settings)
          new_actor_offset     =-T._shape_offset.y+com_offset,
          delta                =actor._actor->offset.pos.y-new_actor_offset;
      actor._actor->offset_com.y=com_offset;
      actor._actor->offset.pos.y=new_actor_offset;
      actor._actor->translate(btVector3(0, delta, 0));
      if(Bullet.world)
      {
         Bullet.world->updateSingleAabb(actor._actor);
         Bullet.world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(actor._actor->getBroadphaseHandle(), Bullet.world->getDispatcher());
      }
   }
}
void Controller::radius(Flt radius)
{
   if(actor._actor)if(btCapsuleShape *shape=CAST(btCapsuleShape, actor._actor->getCollisionShape()))
   {
      T._radius=Mid(radius, Physics.minShapeRadius(), (T._height-Physics.minCapsuleEdge())*0.5f);
      shape->setImplicitShapeDimensions(btVector3(T._radius, T._height_cur*0.5f-T._radius, T._radius));
      if(Bullet.world)
      {
         Bullet.world->updateSingleAabb(actor._actor);
         Bullet.world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(actor._actor->getBroadphaseHandle(), Bullet.world->getDispatcher());
      }
   }
}
#endif
/******************************************************************************/
void Controller::flying(Bool on)
{
   if(flying()!=on)
   {
     _flying=on;
      actor.gravity(!flying());
   }
}
/******************************************************************************/
struct ControllerGroundCallback : PhysHitCallback
{
   Bool        on_ground, do_slide, test_slide;
   Flt         min_y, max_y, slide_y, slide_radius2, ground_frac, slide_frac;
   Vec2        center_xz;
   Vec         slide_normal;
#if DEBUG // on debug also store slide position in case we want to display it
   Vec         slide_pos;
#endif
   Controller &ctrl;

   INLINE Bool closerGround(C PhysHit &phys_hit)C
   {
      if(!on_ground)return true;
      return phys_hit.frac<ground_frac; // just check which one is closer
   }
   INLINE Bool walkable(C PhysHit &phys_hit, UInt group_mask)C
   {
      return (group_mask&Physics.ctrl_ground_group_force) // always stand on this group
          ||((group_mask&Physics.ctrl_ground_group_allow) //        stand only when angle is correct
      #if PHYSX // PhysX may not report all faces of a mesh when sweeping (but it was noticed that it has preference to vertical faces, so ignore 'face_nrm' in case it points to some tiny faces with horizontal 'plane.normal')
           && phys_hit.plane.normal.y>=Physics.ctrl_ground_slope // if angle is ok
      #else // Bullet reports all faces so we can use 'face_nrm'
           && phys_hit.    face_nrm.y>=Physics.ctrl_ground_slope // if angle is ok
      #endif
         );
   }

   virtual Bool hit(PhysHit &phys_hit)
   {
      if(phys_hit.collision)
      {
         UInt group_mask=(1<<phys_hit.group);

         // detect ground plane
         if( phys_hit.plane.pos.y>=min_y
         &&  phys_hit.plane.pos.y<=max_y
         &&  walkable(phys_hit, group_mask)
         )
         {
            if(closerGround(phys_hit) // if closer than previous detection
            && !ctrl.actor.ignored(phys_hit))
            {
                  on_ground      =true;
               ctrl._ground_plane=phys_hit.plane;
                     ground_frac =phys_hit.frac;
            }
         }else // detect if we should slide
         if(test_slide
         && (!phys_hit.dynamic || (group_mask&Physics.ctrl_slide_group_horizontal)) // don't slide against dynamic objects (or slide if it's specified for horizontal)
         &&  phys_hit.plane.pos.y>=slide_y // only if above lower ball
         && (!do_slide || phys_hit.frac<slide_frac) // if closer than previous detection
         &&  Dist2(phys_hit.plane.pos.xz(), center_xz)<=slide_radius2 // if within controller radius
         && !ctrl.actor.ignored(phys_hit)) // if not ignored
         {
            do_slide    =true;
            slide_normal=phys_hit.plane.normal; if(group_mask&Physics.ctrl_slide_group_horizontal){slide_normal.y=0; slide_normal.normalize();} // slide only horizontally against desired groups
            slide_frac  =phys_hit.frac;
         #if DEBUG
            slide_pos   =phys_hit.plane.pos;
         #endif
         }
      }
      return true;
   }

   ControllerGroundCallback(Controller &ctrl, C Vec &center) : ctrl(ctrl)
   {
      Flt ctrl_bottom=center.y-ctrl.heightCur()*0.5f; // don't operate on 'ctrl_shape.h' because it was extended

      T.on_ground    =false;
      T.do_slide     =false;
      T.test_slide   =true;
      T.center_xz    =center.xz();
      T.slide_radius2=Sqr(        ctrl.radius()+Physics.skin()*6);
      T.slide_y      =ctrl_bottom+ctrl.radius()-Physics.skin()*2; // add ctrl radius (important when walking up the stairs)
      T.min_y        =ctrl_bottom              -Physics.skin()*PHYS_API(12, 8);
      T.max_y        =ctrl_bottom+ctrl.step_height;
   }
};
struct ControllerCeilCallback : PhysHitCallback
{
   Bool        cuts;
   Controller &ctrl;

   virtual Bool hit(PhysHit &phys_hit)
   {
      if(phys_hit.collision
      && !ctrl.actor.ignored(phys_hit)){cuts=true; return false;}
      return true;
   }

   ControllerCeilCallback(Controller &ctrl) : ctrl(ctrl) {cuts=false;}
};
/******************************************************************************/
void Controller::update(C Vec &velocity, Bool crouch, Flt jump)
{
   if(actor.is())
   {
      Vec vel=velocity;
      if(!actor.sleep())
      {
         if(_jumping) // update jumping
         {
            if((_time_jump+=Time.d())>0.2f){_jumping=false; _time_jump=0;}
         }else
         if(Physics.updated()) // enter only when physics was updated
         {
            // detect if on ground
            Flt     width=Physics.skin()*2;
            Capsule shape(radius(), heightCur(), center()); // faster than "actor.shape(false)"
            shape.pos.y-=(height()-heightCur())*0.5f; // align to bottom when crouching
            shape.extend(-width); // perform negative extend to make shape smaller, so it will properly detect collisions with contacting shapes (closer than skin width)
            Vec     v(vel.x, 0, vel.z); v.normalize(); v*=width*16; v.y=width*-2-0.1f; // horizontal*16 because when moving almost perpendicular to wall (||) because of smaller radius collision with wall would not be detected, vertical*2 because we need to travel both the radius that we've decreased, and secondary radius for testing (also subtract extra in case the ground is going down, but we're moving forward, so we still can detect the decreasing ground)
            ControllerGroundCallback cgc(T, shape.pos);
            Physics.sweep(shape, v, cgc, Physics.collisionGroups(AG_CONTROLLER));
            if(onGround() && !cgc.on_ground) // if in the previous frame we were on ground but now no ground was detected, try performing additional sweep test in backwards direction
            {
               cgc.test_slide=false; // we don't want to test sliding in backwards dir
               CHS(v.x);
               CHS(v.z);
               Physics.sweep(shape, v, cgc, Physics.collisionGroups(AG_CONTROLLER));
            }

            if(cgc.do_slide)
            {
               Flt d=Dot(vel, cgc.slide_normal); if(d<0)vel-=d*cgc.slide_normal;
            }
            if(_on_ground=cgc.on_ground)
            {
              _step_normal=((_ground_plane.normal.y<=0.01f) ? !(_ground_plane.normal+Vec(0, 1.01f, 0)) : _ground_plane.normal); // +1.01 in case _step_normal.y==-1 so normalize will be vertical and not zero
            }
         }

         if(_on_ground)_time_in_air=0;else _time_in_air+=Time.d();
      }

      // update actor velocities
      if(flying())
      {
         actor.addVel(vel*Time.d());
        _vel_prev.zero();
      }else
      {
         if(onGround() || fall_control)
         {
            if(Physics.updated()) // process only when physics were updated, important because we may do 'Actor.addVel' instead of 'Actor.vel' to set new velocity, which in case physics didn't get simulated it would add delta multiple times (example: actor_vel(0, 0, 0), target_vel(1, 0, 0), addVel(target_vel-actor_vel) addVel(target_vel-actor_vel) addVel(target_vel-actor_vel) would add delta 3x times in case physics was not simulated between the calls)
            {
               // adjust velocity on ground plane
               if(onGround())
               {
                  if(Flt length=vel.length())
                     if(_step_normal.y) // if "_step_normal.y" would be zero then 'PointOnPlaneRay' would return NaN values
                  {
                     Vec v=PointOnPlaneRay(vel, _step_normal, Vec(0, 1, 0));
                     if(!(jump && v.y<0)) // don't align to ground level if we're jumping and the ground is below us
                     {
                        v.setLength(length);
                        vel=v;
                     }
                  }
               }

               // adjust velocity on want
               Vec v=actor.vel(), actor_vel=v;
               Int s=Sign(v.y); if(s==Sign(_vel_prev.y))if(Sign(v.y-=_vel_prev.y)!=s)v.y=0;

               {
                  Vec2 vc_xz=            v.xz(); if(Flt vc_xz_len=vc_xz.normalize()){ // current velocity of the actor
                  Vec2 vp_xz=    _vel_prev.xz();    Flt vp_xz_len=vp_xz.normalize();  // previously set velocity
                  Flt  dot  =Dot(vc_xz, vp_xz);
                  if(  dot>0) // if they are in the same direction
                  { // decrease only what we've manually set
                     Flt  power_of_decrease=vp_xz_len*dot,
                          scale_factor     =Max(0, vc_xz_len-power_of_decrease)/vc_xz_len;
                     v.x*=scale_factor;
                     v.z*=scale_factor;
                  }}
                  v+=vel;
               }

               // store previous velocity
               if(onGround())
               {
                 _vel_prev=v; // =vel; this should be set to 'vel' however with PhysX and processing physics in background thread this won't work, because in codes above ("actor.vel()") we expect actual Actor physical velocity, however because we manually set 'vel' below, it will force velocity to the parameter that we give there, because PhysX queues that kind of commands if simulation is already running, after simulation has ended it will process the queue and override physical velocity with our given parameter (we can't use "actor.addVel(target-actor.vel())" to set relative change, because it was tested and it didn't work, worked badly when walking against a wall)
                  actor.vel(v); // set desired velocity
               }else // fall control (here use relative velocity, it's less precise, however it won't cancel out the gravity)
               {
                 _vel_prev=vel;
                  actor.addVel(v-actor_vel);
               }
            }
         }else // falling
         {
            actor.addVel(vel*Time.d());
           _vel_prev.zero();
         }

         // jump
         if(jump && !_jumping && (onGround() || timeInAir()<=0.2f)) // if on ground or just lost ground (allow 0.2s tolerance)
         {
            actor.addVel(Vec(0, jump, 0));
           _on_ground=false;
           _jumping  =true;
         }
      }

      // crouch
      if(crouch!=T.crouched())
      {
         if(crouch) // if want to crouch
         {
            capsuleHeight(height_crouched);
            T._crouched=true;
         }else // if want to stand up
         {
            ControllerCeilCallback ccc(T);
            Flt  h_diff=height()-heightCur();
            Ball ball(radius(), center()); ball.pos.y+=height()*0.5f - radius() - h_diff; // set ball as crouched controller upper ball
            Physics.sweep(ball, Vec(0, h_diff, 0), ccc, Physics.collisionGroups(AG_CONTROLLER)); // check if there's anything blocking
            if(!ccc.cuts)
            {
               capsuleHeight(height());
               T._crouched=false;
            }
         }
      }

      // handle slope sliding
      if(Physics._css && Physics.updated())
      {
         Bool freeze=(onGround() && _ground_plane.normal.y<1-EPS && velocity.length2()<=Sqr(0.1f));
         switch(Physics._css)
         {
            case CSS_MATERIALS: actor.material  ( freeze              ? &Physics.mtrl_ctrl_stop : &Physics.mtrl_ctrl); break;
            case CSS_GRAVITY  : actor.gravity   ((freeze || flying()) ?  false                  :  true             ); break;
            case CSS_FREEZE_XZ: actor.freezePosX(freeze).freezePosZ(freeze);                                           break;
         }
      }
   }
}
/******************************************************************************/
#pragma pack(push, 1)
struct ControllerDesc
{
   enum FLAG
   {
      CROUCHED    =1<<0,
      ON_GROUND   =1<<1,
      JUMPING     =1<<2,
      FLYING      =1<<3,
      FALL_CONTROL=1<<4,
   };

   Byte  flags;
   Flt   radius, height, height_cur, height_crouched, step_height, time_in_air, time_jump;
   Vec   vel_prev, step_normal, shape_offset;
   Plane ground_plane;
};
#pragma pack(pop)

Bool Controller::save(File &f)C
{
   f.cmpUIntV(0); // version

   ControllerDesc desc;

  _Unaligned(desc.flags, (_crouched     ? ControllerDesc::CROUCHED     : 0)
                        |(_on_ground    ? ControllerDesc::ON_GROUND    : 0)
                        |(_jumping      ? ControllerDesc::JUMPING      : 0)
                        |(_flying       ? ControllerDesc::FLYING       : 0)
                        |( fall_control ? ControllerDesc::FALL_CONTROL : 0));

   Unaligned(desc.radius         , _radius);
   Unaligned(desc.height         , _height);
   Unaligned(desc.height_cur     , _height_cur);
   Unaligned(desc.height_crouched,  height_crouched);
   Unaligned(desc.step_height    ,  step_height);
   Unaligned(desc.time_in_air    , _time_in_air);
   Unaligned(desc.time_jump      , _time_jump);

   Unaligned(desc.vel_prev    , _vel_prev);
   Unaligned(desc.step_normal , _step_normal);
   Unaligned(desc.shape_offset, _shape_offset);

   Unaligned(desc.ground_plane, _ground_plane);

   f<<desc;
   if(actor.saveState(f))
      return f.ok();
   return false;
}
Bool Controller::load(File &f)
{
   del();

   switch(f.decUIntV()) // version
   {
      case 0:
      {
         ControllerDesc desc; if(f.getFast(desc))
         {
           _crouched    =FlagTest(Unaligned(desc.flags), ControllerDesc::CROUCHED    );
           _on_ground   =FlagTest(Unaligned(desc.flags), ControllerDesc::ON_GROUND   );
           _jumping     =FlagTest(Unaligned(desc.flags), ControllerDesc::JUMPING     );
           _flying      =FlagTest(Unaligned(desc.flags), ControllerDesc::FLYING      );
            fall_control=FlagTest(Unaligned(desc.flags), ControllerDesc::FALL_CONTROL);

            Unaligned(_radius         , desc.radius);
            Unaligned(_height         , desc.height);
            Unaligned(_height_cur     , desc.height_cur);
            Unaligned( height_crouched, desc.height_crouched);
            Unaligned( step_height    , desc.step_height);
            Unaligned(_time_in_air    , desc.time_in_air);
            Unaligned(_time_jump      , desc.time_jump);

            Unaligned(_vel_prev    , desc.vel_prev);
            Unaligned(_step_normal , desc.step_normal);
            Unaligned(_shape_offset, desc.shape_offset);

            Unaligned(_ground_plane, desc.ground_plane);

            actor.create(Capsule(_radius, _height, _shape_offset), 1, &VecZero);
            if(actor.loadState(f))
            {
               actor.group  (AG_CONTROLLER); // reset group in case if the enum value was changed
               capsuleHeight(_height_cur);
               if(f.ok())return true;
            }
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
