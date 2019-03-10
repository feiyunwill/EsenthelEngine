/******************************************************************************

   Use 'Motion' to simplify applying a single animation on a skeleton.

/******************************************************************************/
struct Motion // Animation Motion, helper class for playing a single Animation
{
   Flt       time     , // animation time  position
             blend    ; // animation blend value
   SkelAnim *skel_anim; // skeleton animation pointer

   // manage
   Motion& clear(                                             ); // clear animation
   Motion& set  (AnimatedSkeleton &anim_skel, C Str &anim_name); // set   animation, 'anim_skel'=animated skeleton to be used with this motion, 'anim_name'=animation file name
   Motion& set  (AnimatedSkeleton &anim_skel, C UID &anim_id  ); // set   animation, 'anim_skel'=animated skeleton to be used with this motion, 'anim_id'  =animation file name ID

   // get
   Bool is()C {return skel_anim!=null;} // if Motion is valid

 C Animation* animation  ()C {return skel_anim ? skel_anim->animation() : null   ;} // get animation
   UID        animationID()C {return skel_anim ? skel_anim->id       () : UIDZero;} // get animation ID

   Flt animBlend()C {return SmoothCube(blend);} // get blend factor used for animating

   Int eventCount(CChar8 *name)C {return skel_anim ? skel_anim->eventCount(name) : 0;} // get number of events with specified name in this animation

   Bool eventAfter   (CChar8 *name            )C; // if  motion 'time' is after 'name' event (in the animation)
   Bool eventAfter   (Flt     event_time      )C; // if  motion 'time' is after        event time
   Bool eventOccurred(CChar8 *name            )C; // if                         'name' event (in the animation) occurred in current frame
   Bool eventOccurred(Flt     event_time      )C; // if                                event time               occurred in current frame
   Bool eventBetween (CChar8 *from, CChar8 *to)C; // if  motion 'time' is between event 'from' and 'to'
   Bool eventBetween (Flt     from, Flt     to)C; // if  motion 'time' is between event 'from' and 'to' times
   Flt  eventProgress(CChar8 *from, CChar8 *to)C; // get motion 'time' progress between 'from' and 'to'      , 0 on fail
   Flt  eventProgress(Flt     from, Flt     to)C; // get motion 'time' progress between 'from' and 'to' times, 0 on fail

   // update
   Bool updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed=1); // update (blend in -> play animation -> blend out), returns false when animation finished playing and blending out
   Bool updateIn  (Flt blend_in_speed,                      Flt time_speed=1); // update (blend in -> play animation             ), returns false when animation finished playing
   Bool updateOut (                    Flt blend_out_speed                  ); // update (                              blend out), returns false when animation finished             blending out

   // io
   Bool save(File &f                               )C; // save, false on fail
   Bool load(File &f, C AnimatedSkeleton &anim_skel) ; // load, false on fail

   Motion() {clear();}

private:
   Flt time_prev, time_delta;
};
/******************************************************************************/
