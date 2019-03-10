/******************************************************************************

   Use 'InterpolatorTime' to calculate time step fraction between sequentially received data.

   Use 'AngularInterpolator' to calculate linear interpolation between sequentially received angle values.
   Use  'LinearInterpolator' to calculate linear interpolation between sequentially received       values.
   Use  'SplineInterpolator' to calculate spline interpolation between sequentially received       values.

   One 'InterpolatorTime' can be paired with one or more value interpolators ('AngularInterpolator', 'LinearInterpolator', 'SplineInterpolator')

   It can be used like this:

   Define interpolator variables:
      InterpolatorTime time_interp ; // 'time_interp' time interpolator paired with 'player_angle' and 'player_pos'
      InterpolatorA    player_angle; // linear angular     interpolation
      Interpolator3    player_pos  ; // linear 3D position interpolation

   Game loop:
   {
      // process received data from the network
      for(; received_data; )
      {
         InterpolatorTemp temp; // some helper temporary data

         // !! 'add' of 'InterpolatorTime' must be called first !!
         time_interp.add(received_data.duration, temp);

         // now call 'add' for value interpolators with the same 'temp'
         player_angle   .add(received_data.angle   , temp);
         player_position.add(received_data.position, temp);
      }

      // update interpolators once per frame
      InterpolatorTemp temp; // some helper temporary data

      // !! 'update' of 'InterpolatorTime' must be called first !!
      time_interp.update(duration, temp);

      // now call 'update' for value interpolators with the same 'temp'
      player_angle   .update(temp);
      player_position.update(temp);
   }

/******************************************************************************/
struct InterpolatorTime
{
   Bool is()C {return _elms!=0;} // if have any value

   // operations
   void add   (Flt duration, InterpolatorTemp &temp); // call this method when received new data !! before calling 'add'    method of other classes ('AngularInterpolator', 'LinearInterpolator', 'SplineInterpolator') !! 'duration'=for how long this data should be valid
   void update(              InterpolatorTemp &temp); // call this method once per frame         !! before calling 'update' method of other classes ('AngularInterpolator', 'LinearInterpolator', 'SplineInterpolator') !!

   InterpolatorTime& reset(); // reset to initial settings

   InterpolatorTime() {reset();}

#if !EE_PRIVATE
private:
#endif
   Byte _elms, _count;
   Flt  _time, _cur_duration, _next_duration, _left;
};
/******************************************************************************/
T1(TYPE) struct AngularInterpolator
{
   // get
 C TYPE& operator()()C {return _value;} // get current value

   // operations
   void add   (C TYPE &value, C InterpolatorTemp &temp); // call this method when received new data !! after calling 'InterpolatorTime.add'    with the same 'temp' !!, 'value'=received data value
   void update(               C InterpolatorTemp &temp); // call this method once per frame         !! after calling 'InterpolatorTime.update' with the same 'temp' !!

   AngularInterpolator();

#if !EE_PRIVATE
private:
#endif
   TYPE _value, _prev, _cur, _next;
};
/******************************************************************************/
T1(TYPE) struct LinearInterpolator
{
   // get
 C TYPE& operator()()C {return _value;} // get current value

   // operations
   void add   (C TYPE &value, C InterpolatorTemp &temp); // call this method when received new data !! after calling 'InterpolatorTime.add'    with the same 'temp' !!, 'value'=received data value
   void update(               C InterpolatorTemp &temp); // call this method once per frame         !! after calling 'InterpolatorTime.update' with the same 'temp' !!

   LinearInterpolator();

#if !EE_PRIVATE
private:
#endif
   TYPE _value, _prev, _cur, _next;
};
/******************************************************************************/
T1(TYPE) struct SplineInterpolator
{
   // get
 C TYPE& operator()()C {return _value;} // get current value

   // operations
   void add   (C TYPE &value, C InterpolatorTemp &temp); // call this method when received new data !! after calling 'InterpolatorTime.add'    with the same 'temp' !!, 'value'=received data value
   void update(               C InterpolatorTemp &temp); // call this method once per frame         !! after calling 'InterpolatorTime.update' with the same 'temp' !!

   SplineInterpolator();

#if !EE_PRIVATE
private:
#endif
   TYPE _value, _prev2, _prev, _cur, _next;
};
/******************************************************************************/
struct InterpolatorTemp
{
   Byte op;
   Flt  frac;
};
/******************************************************************************/
typedef AngularInterpolator<Flt> InterpolatorA;

typedef LinearInterpolator<Flt > Interpolator1;
typedef LinearInterpolator<Vec2> Interpolator2;
typedef LinearInterpolator<Vec > Interpolator3;
typedef LinearInterpolator<Vec4> Interpolator4;

typedef SplineInterpolator<Flt > Interpolator1S;
typedef SplineInterpolator<Vec2> Interpolator2S;
typedef SplineInterpolator<Vec > Interpolator3S;
typedef SplineInterpolator<Vec4> Interpolator4S;
/******************************************************************************/
