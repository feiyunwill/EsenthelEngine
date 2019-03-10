/******************************************************************************

   Use 'Time' to access:
      -current time
      -current frame time
      -current frame number
      -modify game time speed

/******************************************************************************/
struct TimeClass
{
   UInt     frame ()C {return _frame;} // current frame number
   Flt      fps   ()C {return _fps  ;} // current number of frames per second
   Flt      d     ()C {return _d    ;} // game        time delta             (                    frame time duration,     modified by game 'speed',     affected by 'smooth' 'skipUpdate' and application pauses)
   Flt     ad     ()C {return _ad   ;} // application time delta             (                    frame time duration, NOT modified by game 'speed',     affected by 'smooth' 'skipUpdate' and application pauses)
   Flt     rd     ()C {return _rd   ;} // real        time delta             (                    frame time duration, NOT modified by game 'speed', NOT affected by 'smooth' 'skipUpdate' and application pauses)
   Dbl      time  ()C {return _t    ;} // game        time in current frame  (     seconds since application  started,     modified by game 'speed',     affected by 'smooth' 'skipUpdate' and application pauses)
   Dbl   appTime  ()C {return _at   ;} // application time in current frame  (     seconds since application  started, NOT modified by game 'speed',     affected by 'smooth' 'skipUpdate' and application pauses)
   Dbl stateTime  ()C {return _st   ;} // state       time in current frame  (     seconds since last 'State' started, NOT modified by game 'speed',     affected by 'smooth' 'skipUpdate' and application pauses)
   Dbl  realTime  ()C {return _rt   ;} // real        time in current frame  (     seconds since application  started, NOT modified by game 'speed', NOT affected by 'smooth' 'skipUpdate' and application pauses)
   Dbl   curTime  ()C;                 // real        time in current moment (     seconds since application  started, NOT modified by game 'speed', NOT affected by 'smooth' 'skipUpdate' and application pauses, this method always calculates the time when called)
   UInt  curTimeMs()C;                 // real        time in current moment (milliseconds since application  started, NOT modified by game 'speed', NOT affected by 'smooth' 'skipUpdate' and application pauses, this is a little faster method than 'curTime' but returns time in milliseconds)

   Flt               speed     (                      )C {return _speed;}            // get game time speed (<1 slower, 1 default, >1 faster)
   void              speed     (Flt              speed);                             // set game time speed (<1 slower, 1 default, >1 faster) and modify sound speeds (except VOLUME_MUSIC and VOLUME_AMBIENT volume groups)
   SMOOTH_VALUE_MODE smooth    (                      )C {return _sv_ad.mode(    );} // get time delta smoothing, default=SV_WEIGHT4
   void              smooth    (SMOOTH_VALUE_MODE mode)  {       _sv_ad.mode(mode);} // set time delta smoothing
   void              wait      (Int       milliseconds);                             // pause the current thread and wait 'milliseconds' time (this is equal to calling the system 'Sleep' function)
   void              skipUpdate(Byte          frames=1);                             // call this method to notify that timer should skip updating frame time for following 'frames', use this method after slow one time methods, like loading data

   Flt stateUpdateTime()C {return _state_update;} // get CPU time needed to process active application State Update in last frame
   Flt stateDrawTime  ()C {return _state_draw  ;} // get CPU time needed to process active application State Draw   in last frame

#if !EE_PRIVATE
private:
#endif
   Bool        _pc;
   Byte        _skip;
   UInt        _start_time_ms, _frame, _frames_drawn;
   ULong       _start_time;
   Flt         _fps, _fps_time, _state_update, _state_draw;
   Dbl         _rd_2, _rd_1, _rd, _ad, _d, _rt, _at, _st, _t, _speed, _time_mul;
   SmoothValue _sv_ad;

   TimeClass();
#if EE_PRIVATE
   void create();
   void update();
#endif
}extern
   Time; // Application Time Access
/******************************************************************************/
Bool EventOccurred(Flt event_time                        , Flt start_time, Flt dt); // if event has occurred , 'event_time'     =position in time       ,                                        'start_time'=time at the start of the frame, 'dt'=time delta
Bool EventBetween (Flt event_time_from, Flt event_time_to, Flt start_time, Flt dt); // if event is  occurring, 'event_time_from'=position of event start, 'event_time_to'=position of event end, 'start_time'=time at the start of the frame, 'dt'=time delta
/******************************************************************************/
