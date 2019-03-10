/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static Flt InterpolatorFrac(Flt frac)
{
   // allow prediction for frac 1..2
#if 0 // linear
   if(frac>2) // but when frac goes beyond that
   {
   #if 1 // then go back to last known position in frac 2..3
      frac=((frac>=3) ? 1 : 2*2-frac); // go back
   #else
      frac=2; // limit prediction
   #endif
   }
#else // quadratic
   if(frac>1) // convert frac from 1 .. 2 .. 3 to: 1 .. 1.5 .. 1 using quadratic function
   {
      // x-x*x/2, where x=frac-1
      //frac=frac-1 - (frac-1)*(frac-1)/2+1;
      //frac=frac - (frac*frac + 1 - 2*frac)/2;
      frac=((frac>=3) ? 1 : -0.5f*frac*frac + 2*frac - 0.5f); // -0.5*x*x + 2*x - 0.5
   }
#endif
   return frac;
}
InterpolatorTime& InterpolatorTime::reset()
{
  _elms=_count=0;
  _time=_cur_duration=_next_duration=0;
  _left=FLT_MAX;
   return T;
}
void InterpolatorTime::add(Flt duration, InterpolatorTemp &temp)
{
   // check this first
 //if(_elms>=2) check not needed since in worst case the value will be negative and ignored
   {
      Flt left=_next_duration+_cur_duration-_time-Time.rd(); // calculate how much time we had left without this call
      MIN(_left, left); // minimize time left
     _count=Min(_count+1, 255); // increase counter
   }

   switch(_elms)
   {
      case 0: _elms=1; temp.op=0; break; // we now have 'prev'
      case 1: _elms=2; _time=0; _cur_duration=duration; temp.op=1; break; // we now have 'prev'+'cur', we can start interpolation, so reset time
      case 2:
      {
         if(_time>_cur_duration) // check if we're currently predicting (we don't have 'next' and we've exceeded 'cur')
         {
           _time=0; _cur_duration=duration; temp.op=2; // reset time progress
         }else
         {
           _elms=3; _next_duration=duration; temp.op=3;
         }
      }break;
      case 3: _next_duration+=duration; temp.op=3; break;
   }
}
void InterpolatorTime::update(InterpolatorTemp &temp)
{
   temp.op=0;
  _time+=Time.rd();
   if(_elms==3 && _time>_cur_duration)
   {
      temp.op=1;
     _elms =2;
     _time-=_cur_duration;
     _cur_duration=_next_duration; _next_duration=0;
      if(_count>=4) // higher number increases smoothness, but is slower to respond to network delays
      {
        _count=0;
         if(_left>0) // if we're too slow, then speedup
         {
            Flt time=_cur_duration-_time; // actual time left
            if( time>0)
            {
               Flt target=Max(0, time-_left), // desired time left
                   mul=target/time; // scale factor
                      _time*=mul;
              _cur_duration*=mul;
            }
         }
        _left=FLT_MAX;
      }
   }

   // do this last
   temp.frac=(_cur_duration ? InterpolatorFrac(_time/_cur_duration) : 0);
}
/******************************************************************************
struct HistoryInterpolator2
{
   // get
   Vec2 operator()()C;

   // operations
   void add(C Vec2 &value, Flt duration);
   void update();

   HistoryInterpolator2();

#if !EE_PRIVATE
private:
#endif
   struct Elm
   {
      Vec2 value;
      Flt  duration;
   };
   Byte _elms, _count;
   Flt  _time, _left; _left could be encoded in history[0].time or use separate InterpolatorTime?
   Elm  _history[3];
};
/******************************************************************************
HistoryInterpolator2::HistoryInterpolator2()
{
  _elms=_count=0;
  _time=0;
  _left=FLT_MAX;
   REPA(_history)
   {
     _history[i].value.zero();
     _history[i].duration=0;
   }
}
Vec2 HistoryInterpolator2::operator()()C
{
   if(_elms<=1)return _history[0].value;
   Flt frac=InterpolatorFrac(_time/_history[1].duration); // fraction between #0 and #1
   return Lerp(_history[0].value, _history[1].value, frac);
}
void HistoryInterpolator2::add(C Vec2 &value, Flt duration)
{
   // check this first
 //if(_elms>=2) check not needed since in worst case the value will be negative and ignored
   {
      Flt left=-_time-Time.rd(); for(Int i=_elms-1; i>=1; i--)left+=_history[i].duration; // calculate how much time we had left without this call
      MIN(_left, left); // minimize time left
     _count=Min(_count+1, 255); // increase counter
   }

   constexpr Int total=ELMS(_history);
   if(_elms>=total)
   {
      // find element with shortest duration (the most insignificant)
      Int i=2; // start from elm #2 to leave those currently used for interpolation intact
      Int shortest=i; // assume the first is the shortest
      Flt shortest_duration=_history[shortest].duration; // remember shortest duration
      for(; ++i<total; )
      {
         Flt d=_history[i].duration;
         if(d<shortest_duration)
         {
            shortest_duration=d;
            shortest=i;
         }
      }
      if(InRange(shortest+1, total)) // if next one is in the history
      {
         MoveFastN(&_history[shortest], &_history[shortest+1], total-shortest-1); // remove the shortest from history
        _history[shortest].duration+=shortest_duration; // increase duration of the next one
      }else                duration+=shortest_duration; // next one is the new one
     _elms--;
   }else
   if(_elms==1)_time=0;else
   if(1 // this code makes adjustment to prevent jumps, if we're predicting, then it will adjust the history, so instead of jumping, we will interpolate from current (predicted) position into the new one
   && _elms==2 && _time>_history[1].duration) // check if we're currently predicting
   {
   #if 1 // if we're too fast then we have to slow down
     _history[0].value=T(); // start interpolating from current position
     _time=0; // reset time progress
     _elms--;
   #else
      // adjust old history so after adding new value, the interpolated one will be the same as now
      Vec2 current=T(); // calculate current position
     _time-=_history[1].duration;
   //_elms =2; is already 2
     _history[1].value   =value;
     _history[1].duration=duration;
      Flt frac=T.frac();
      // now we have to make sure that T()==current
      // Lerp(_history[0].value, _history[1].value, frac)==current
      // _history[0].value*(1-frac) + _history[1].value*frac == current
      // _history[0].value*(1-frac) == current - _history[1].value*frac
      // _history[0].value == (current - _history[1].value*frac) / (1-frac)
      if(Flt div=1-frac)_history[0].value=(current-_history[1].value*frac)/div; // if frac!=1
      else              _history[0].value=value;                                //    frac==1
   #if 0
      #pragma message("!! Warning: Use this only for debugging !!")
      DEBUG_ASSERT(Equal(current, T()), "Intepolator predict error");
   #endif
      return;
   #endif
   }

   Elm &elm=_history[_elms++];
   elm.value   =value;
   elm.duration=duration;
}
void HistoryInterpolator2::update()
{
  _time+=Time.rd();
   if(_elms>2 && _time>_history[1].duration)
   {
      Int i=1;
   again:
     _time-=_history[i].duration; _elms--;
      if(_elms>2 && _time>_history[i+1].duration){i++; goto again;}
      MoveFastN(&_history[0], &_history[i], _elms);
      if(_count>=4) // higher number increases smoothness, but is slower to respond to network delays
      {
        _count=0;
         if(_left>0) // if we're too slow, then speedup
         {
            Flt time=_history[1].duration-_time; for(Int i=2; i<_elms; i++)time+=_history[i].duration; // actual time left
            if( time>0)
            {
               Flt target=Max(0, time-_left), // desired time left
                   mul=target/time; // scale factor
                                                        _time*=mul;
               for(Int i=1; i<_elms; i++)_history[i].duration*=mul;
            }
         }
        _left=FLT_MAX;
      }
   }
}
/******************************************************************************
struct InterpolatorTime
{
   // get
   Flt  operator()()C {return _frac ;} // get current time fraction (0..1) between received data
   Bool empty     ()C {return _empty;} // if is empty (no data was yet received)

   // operations
   void update(); // call this method at least once per frame before using this object for methods of other classes ('AngularInterpolator', 'LinearInterpolator', 'SplineInterpolator')
   void step  (); // call this method when received new data, however after calling 'step' method  of other classes ('AngularInterpolator', 'LinearInterpolator', 'SplineInterpolator')

   InterpolatorTime& reset(); // reset to initial settings

   InterpolatorTime() {reset();}

private:
   Flt  _frac, _latency;
   Dbl  _last_receive_time;
   Bool _empty;
};
T1(TYPE) struct AngularInterpolator
{
   // get
 C TYPE& operator()()C {return _c;} // get current value

   // operations
   AngularInterpolator& update(               C InterpolatorTime &time); // update current value according to history and current time, call once per frame
   void                 step  (C TYPE &value, C InterpolatorTime &time); // call this method when received new data

   void clearHistory() {_c=_h[0]=_h[1];} // clear value history, you can call this method when new value is drastically different than previous values in order to disable interpolation between old values (for example when interpolating player position which has suddenly teleported to a distant location, interpolator would normally return his position as interpolation between original and new position, however after calling this method new position will be returned immediately without interpolation with old position)

   AngularInterpolator() {_h[0]=_h[1]=_c=0;}

private:
   TYPE _c, _h[2];
};
T1(TYPE) struct LinearInterpolator
{
   // get
 C TYPE& operator()()C {return _c;} // get current value

   // operations
   LinearInterpolator& update(               C InterpolatorTime &time); // update current value according to history and current time, call once per frame
   void                step  (C TYPE &value, C InterpolatorTime &time); // call this method when received new data

   void clearHistory() {_c=_h[0]=_h[1];} // clear value history, you can call this method when new value is drastically different than previous values in order to disable interpolation between old values (for example when interpolating player position which has suddenly teleported to a distant location, interpolator would normally return his position as interpolation between original and new position, however after calling this method new position will be returned immediately without interpolation with old position)

   LinearInterpolator() {_h[0]=_h[1]=_c=0;}

private:
   TYPE _c, _h[2];
};
T1(TYPE) struct SplineInterpolator
{
   // get
 C TYPE& operator()()C {return _c;} // get current value

   // operations
   SplineInterpolator& update(               C InterpolatorTime &time); // update current value according to history and current time, call once per frame
   void                step  (C TYPE &value, C InterpolatorTime &time); // call this method when received new data

   void clearHistory() {_c=_h[0]=_h[1]=_h[3]=_h[2];} // clear value history, you can call this method when new value is drastically different than previous values in order to disable interpolation between old values (for example when interpolating player position which has suddenly teleported to a distant location, interpolator would normally return his position as interpolation between original and new position, however after calling this method new position will be returned immediately without interpolation with old position)

   SplineInterpolator() {_h[0]=_h[1]=_h[2]=_h[3]=_c=0;}

private:
   TYPE _c, _h[4];
};
InterpolatorTime& InterpolatorTime::reset()
{
  _empty=true;
  _frac=_latency=0;
  _last_receive_time=0;
   return T;
}
void InterpolatorTime::update()
{
   if(_latency)
   {
      Flt delta=Time.realTime()-_last_receive_time;
     _frac=Min(delta/_latency, 1.25f);
   }
}
void InterpolatorTime::step()
{
   Flt latency=Time.realTime()-_last_receive_time;
   if(_last_receive_time)
   {
      T._latency=Lerp(T._latency, latency, (latency>T._latency) ? 0.9f : 0.1f);
   }else
   {
      T._latency=Time.rd();
   }
   T._last_receive_time=Time.realTime();
   T._frac =0;
   T._empty=false;
}
/******************************************************************************
struct _Interpolator // don't use this class, use 'Interpolator' instead
{
   struct _Elm
   {
      Flt  x;
    //TYPE y;
   };

   Bool loop , // if elements are looped (last interpolates to first)                       , default=false
        clamp; // if clamp value when accessing element out of range and looping is disabled, default=true
   Flt  min_x, max_x; // minimum and maximum possible values of 'x' (used only when looping)

   Int  elms ()C {return _elms.elms();} // get number of elements
   void clear()  {_elms.clear(); min_x=FLT_MAX; max_x=-FLT_MAX;} // clear elements and reset min/max ranges

private:
   Memc<_Elm> _elms;
  _Elm* _add (Flt x                                                         ) ;
   void _get (Flt x,             Int &prev, Int &next,             Flt &frac)C;
   void _get4(Flt x, Int &prev2, Int &prev, Int &next, Int &next2, Flt &frac)C;
  _Interpolator();
   T1(TYPE) friend struct Interpolator;
};
T1(TYPE) struct Interpolator : _Interpolator
{
   struct Elm : _Elm
   {
      TYPE y;
   };

   Interpolator& add(Flt x, C TYPE &y) ; // create new element at 'x' position
   Elm&   operator[](Int i           )C; // get i-th element
   TYPE   operator()(Flt x           )C; // get linear interpolated value at 'x' position
   TYPE       smooth(Flt x           )C; // get smooth interpolated value at 'x' position (using 4 value, hermite spline interpolation)

   Interpolator& operator=(C Interpolator &src);

   Interpolator();
   Interpolator(C Interpolator &src);
};
inline Int Elms(C _Interpolator &interpolator) {return interpolator.elms();}
/******************************************************************************
T1(TYPE)           Interpolator<TYPE>     &  Interpolator<TYPE>::add       (Flt x, C TYPE &y)  {if(typename Interpolator<TYPE>::Elm *elm=(typename Interpolator<TYPE>::Elm*)_add(x))elm->y=y; return T;}
T1(TYPE)  typename Interpolator<TYPE>::Elm&  Interpolator<TYPE>::operator[](Int i           )C {return (typename Interpolator<TYPE>::Elm&)_elms[i];}
T1(TYPE)                        TYPE         Interpolator<TYPE>::operator()(Flt x           )C
{
   Int prev, next; Flt frac; _get(x, prev, next, frac);
   if( prev==next)
   {
      if(prev<0)return 0;
      return T[prev].y;
   }
   return Lerp(T[prev].y, T[next].y, frac);
}
T1(TYPE)  TYPE  Interpolator<TYPE>::smooth(Flt x)C
{
   Int prev2, prev, next, next2; Flt frac; _get4(x, prev2, prev, next, next2, frac);
   if( prev==next)
   {
      if(prev<0)return 0;
      return T[prev].y;
   }
   return Lerp4((prev2<0) ? T[prev].y*2-T[next].y : T[prev2].y, T[prev].y, T[next].y, (next2<0) ? T[next].y*2-T[prev].y : T[next2].y, frac);
}
T1(TYPE)  Interpolator<TYPE>&  Interpolator<TYPE>::operator=(C Interpolator &src)
{
   loop =src.loop;
   clamp=src.clamp;
   min_x=src.min_x;
   max_x=src.max_x;
  _elms.setNum(src.elms());
   FREPAO(T)=src[i];
   return T;
}
T1(TYPE)  Interpolator<TYPE>::Interpolator(                   )                  {_elms.replaceClass<Elm>();}
T1(TYPE)  Interpolator<TYPE>::Interpolator(C Interpolator &src) : Interpolator() {T=src;}
/******************************************************************************
static Int CompareInterpolator(C _Interpolator::_Elm &elm, C Flt &x) {return Compare(elm.x, x);}

_Interpolator::_Interpolator()
{
   loop=false; clamp=true; min_x=FLT_MAX; max_x=-FLT_MAX;
}
_Interpolator::_Elm* _Interpolator::_add(Flt x)
{
   Int i; if(!_elms.binarySearch(x, i, CompareInterpolator)){_Elm &e=_elms.NewAt(i); e.x=x; MAX(max_x, x); MIN(min_x, x); return &e;}
   return null;
}
void _Interpolator::_get(Flt x, Int &prev, Int &next, Flt &frac)C // get interpolated value at 'x' position
{
   switch(elms())
   {
      case  0: prev=next=-1; break; // set both to invalid
      case  1: prev=next= 0; break; // set both to first
      default:
      {
         if(loop)x=Frac(x-min_x, max_x-min_x)+min_x;
         if(_elms.binarySearch(x, next, CompareInterpolator))prev=next;else // if found precisely
         {
            prev=next-1;
            if(!InRange(next, T)) // no next
            {
               if(loop)
               {
                  next=0; // set next as first
                  if(Flt length=(max_x-_elms.last().x)+(_elms.first().x-min_x))frac=(x-_elms.last().x)/length;else prev=next=elms()-1;
               }else
               if(clamp)prev=next=elms()-1;else
               {
                  prev--;
                  next--;
                  frac=LerpR(_elms[prev].x, _elms[next].x, x); // this will be >1
               }
            }else
            if(!InRange(prev, T)) // no previous
            {
               if(loop)
               {
                  prev=elms()-1; // set prev as last
                  if(Flt length=(max_x-_elms.last().x)+(_elms.first().x-min_x))frac=(1-(_elms.first().x-x))/length;else prev=next=0;
               }else
               if(clamp)prev=next=0;else
               {
                  prev++;
                  next++;
                  frac=LerpR(_elms[prev].x, _elms[next].x, x); // this will be <0
               }
            }else
            {
               frac=LerpR(_elms[prev].x, _elms[next].x, x); // this will be 0..1
            }
         }
      }break;
   }
}
void _Interpolator::_get4(Flt x, Int &prev2, Int &prev, Int &next, Int &next2, Flt &frac)C
{
  _get(x, prev, next, frac);
   if(prev!=next)
   {
      prev2=prev-1; if(prev2<0) // invalid
      {
         if(loop )prev2=elms()-1;else // set prev as last
         if(clamp)prev2=prev;else
                  prev2=-1;
      }
      next2=next+1; if(next2>=elms()) // invalid
      {
         if(loop )next2=0;else // set next as first
         if(clamp)next2=next;else
                  next2=-1;
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
