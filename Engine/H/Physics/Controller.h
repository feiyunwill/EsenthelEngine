/******************************************************************************

   Use 'Controller' to simplify management of character 'Actor' (a physical object).
   It can handle basic movement, crouching, jumping and flying.

/******************************************************************************/
struct Controller // Character Controller
{
   Bool  fall_control   ; // if have full control over movement when falling  , default=false
   Flt   height_crouched, // desired height when crouched                     , default="height*0.67" (set in create methods)
         step_height    ; // max step height on which the Controller can climb, default="radius"      (set in create methods)
   Actor actor          ; // actor, has default group set according to ACTOR_GROUP

   // get
   Bool   onGround   ()C {return _on_ground                ;} // if character is on ground
   Bool   crouched   ()C {return _crouched                 ;} // if character is crouched
   Bool   flying     ()C {return _flying                   ;} // if character is in flying mode
   Flt    radius     ()C {return _radius                   ;} // character radius
   Flt    height     ()C {return _height                   ;} // character height
   Flt    heightCur  ()C {return _height_cur               ;} // current character height (this can be affected by crouching/standing)
   Flt    timeInAir  ()C {return _time_in_air              ;} // get current amount of time while in air, as soon as player stands on ground this value is reset to zero
 C Plane& groundPlane()C {return _ground_plane             ;} // ground contact plane, this is valid when character is on ground
 C Vec  & shapeOffset()C {return _shape_offset             ;} // get capsule shape offset in the actor local space                           , crouching does not affect the result of this method
   Vec    center     ()C {return  actor.pos()+_shape_offset;} // get controller center (this is the center of actor's capsule in world space), crouching does not affect the result of this method

   // manage
   Controller& del          (                                                      ); // delete manually
   Controller& createCapsule(Flt radius, Flt height, C Vec &pos, C Vec *anchor=null); // create from capsule
   Controller& create       (C Capsule &capsule                , C Vec *anchor=null) {return createCapsule(capsule.r, capsule.h, capsule.pos, anchor);} // create from capsule shape, here 'capsule.up' vector is ignored, because the Controller requires that the capsule up vector is always set to Vec(0,1,0)

   // operations
   void flying(Bool on                               ); // set if flying mode enabled
   void radius(Flt  radius                           ); // set actor capsule radius
   void update(C Vec &velocity, Bool crouch, Flt jump); // update, 'velocity'=wanted velocity, 'crouch'=if want to crouch, 'jump'=jumping velocity !! don't call between Physics.startSimulation and Physics.stopSimulation !!

   // io
   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

   Controller();

#if !EE_PRIVATE
private:
#endif
   Bool  _on_ground, _crouched, _jumping, _flying;
   Flt   _radius, _height, _height_cur, _time_in_air, _time_jump;
   Vec   _vel_prev, _step_normal, _shape_offset;
   Plane _ground_plane;
#if EE_PRIVATE
   void capsuleHeight(Flt height);
   void zero();
#endif
};
/******************************************************************************/
