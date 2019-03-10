/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#if WINDOWS_NEW
using namespace concurrency;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;
#elif MAC
static PasteboardRef Pasteboard;
#elif ANDROID || WEB
static Str Clipboard;
#endif
const UID UIDZero(0, 0, 0, 0);
/******************************************************************************/
Int Compare(C CyclicUShort &a, C CyclicUShort &b) {return (a==b) ? 0 : (a<b) ? -1 : +1;}
Int Compare(C CyclicUInt   &a, C CyclicUInt   &b) {return (a==b) ? 0 : (a<b) ? -1 : +1;}
/******************************************************************************/
Byte FltToByteScale (Flt  x) {return Mid(RoundPos(x*256)-1, 0, 255);}
Byte FltToByteScale2(Flt  x) {return Mid(RoundPos(x*128)-1, 0, 255);}
Flt  ByteScaleToFlt (Byte x) {return (x+1)/256.0f;} // 0 -> 1/256, 127 -> 128/256 = 0.5, 255 -> 256/256 = 1.0
Flt  ByteScale2ToFlt(Byte x) {return (x+1)/128.0f;} // 0 -> 1/128, 127 -> 128/128 = 1.0, 255 -> 256/128 = 2.0

Int ByteScaleRes(Int res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return Max(1, DivRound(res*byte_scale1, 256u));
}
VecI2 ByteScaleRes(C VecI2 &res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return VecI2(Max(1, DivRound(res.x*byte_scale1, 256u)),
                Max(1, DivRound(res.y*byte_scale1, 256u)));
}

Int ByteScale2Res(Int res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return Max(1, DivRound(res*byte_scale1, 128u));
}
VecI2 ByteScale2Res(C VecI2 &res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return VecI2(Max(1, DivRound(res.x*byte_scale1, 128u)),
                Max(1, DivRound(res.y*byte_scale1, 128u)));
}
/******************************************************************************
Int ByteScaleResCeil(Int res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return DivCeil(res*byte_scale1, 256u); // because of 'DivCeil' we don't need Max(1, ..)
}
VecI2 ByteScaleResCeil(C VecI2 &res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return VecI2(DivCeil(res.x*byte_scale1, 256u),  // because of 'DivCeil' we don't need Max(1, ..)
                DivCeil(res.y*byte_scale1, 256u)); // because of 'DivCeil' we don't need Max(1, ..)
}

Int ByteScale2ResCeil(Int res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return DivCeil(res*byte_scale1, 128u); // because of 'DivCeil' we don't need Max(1, ..)
}
VecI2 ByteScale2ResCeil(C VecI2 &res, Byte byte_scale)
{
   UInt byte_scale1=byte_scale+1;
   return VecI2(DivCeil(res.x*byte_scale1, 128u),  // because of 'DivCeil' we don't need Max(1, ..)
                DivCeil(res.y*byte_scale1, 128u)); // because of 'DivCeil' we don't need Max(1, ..)
}
/******************************************************************************/
static void TestCyclic()
{
   CyclicUInt a=0, b=0;
   DYNAMIC_ASSERT( (a>=b), "Cyclic");
   DYNAMIC_ASSERT(!(a> b), "Cyclic");
   DYNAMIC_ASSERT( (a<=b), "Cyclic");
   DYNAMIC_ASSERT(!(a< b), "Cyclic");
   DYNAMIC_ASSERT( (a==b), "Cyclic");
   DYNAMIC_ASSERT(!(a!=b), "Cyclic");
   DYNAMIC_ASSERT( (b>=a), "Cyclic");
   DYNAMIC_ASSERT(!(b> a), "Cyclic");
   DYNAMIC_ASSERT( (b<=a), "Cyclic");
   DYNAMIC_ASSERT(!(b< a), "Cyclic");
   DYNAMIC_ASSERT( (b==a), "Cyclic");
   DYNAMIC_ASSERT(!(b!=a), "Cyclic");

   a=UINT_MAX;
   DYNAMIC_ASSERT(!(a>=b), "Cyclic");
   DYNAMIC_ASSERT(!(a> b), "Cyclic");
   DYNAMIC_ASSERT( (a<=b), "Cyclic");
   DYNAMIC_ASSERT( (a< b), "Cyclic");
   DYNAMIC_ASSERT(!(a==b), "Cyclic");
   DYNAMIC_ASSERT( (a!=b), "Cyclic");
   DYNAMIC_ASSERT( (b>=a), "Cyclic");
   DYNAMIC_ASSERT( (b> a), "Cyclic");
   DYNAMIC_ASSERT(!(b<=a), "Cyclic");
   DYNAMIC_ASSERT(!(b< a), "Cyclic");
   DYNAMIC_ASSERT(!(b==a), "Cyclic");
   DYNAMIC_ASSERT( (b!=a), "Cyclic");

   a++;
   a++;
   DYNAMIC_ASSERT(a.v==1 , "Cyclic");
   DYNAMIC_ASSERT( (a>=b), "Cyclic");
   DYNAMIC_ASSERT( (a> b), "Cyclic");
   DYNAMIC_ASSERT(!(a<=b), "Cyclic");
   DYNAMIC_ASSERT(!(a< b), "Cyclic");
   DYNAMIC_ASSERT(!(a==b), "Cyclic");
   DYNAMIC_ASSERT( (a!=b), "Cyclic");
   DYNAMIC_ASSERT(!(b>=a), "Cyclic");
   DYNAMIC_ASSERT(!(b> a), "Cyclic");
   DYNAMIC_ASSERT( (b<=a), "Cyclic");
   DYNAMIC_ASSERT( (b< a), "Cyclic");
   DYNAMIC_ASSERT(!(b==a), "Cyclic");
   DYNAMIC_ASSERT( (b!=a), "Cyclic");
}
/******************************************************************************/
SmoothValue ::SmoothValue (SMOOTH_VALUE_MODE mode) {Zero(T); T._mode=mode;}
SmoothValue2::SmoothValue2(SMOOTH_VALUE_MODE mode) {Zero(T); T._mode=mode;}

void SmoothValue ::mode(SMOOTH_VALUE_MODE mode) {if(T._mode!=mode){T._mode=mode; init(_history[0]);}}
void SmoothValue2::mode(SMOOTH_VALUE_MODE mode) {if(T._mode!=mode){T._mode=mode; init(_history[0]);}}

void SmoothValue ::init(  Flt   start_value) {REPAO(_history)=start_value;}
void SmoothValue2::init(C Vec2 &start_value) {REPAO(_history)=start_value;}

Flt SmoothValue::update(Flt current_value)
{
   Flt value;
   switch(_mode)
   {
      default: return _history[0]=current_value;

   #if 1
      case SV_WEIGHT2: _history[_pos]=current_value; value=0;          REP(2)value+=BlendSmoothCube(i/2.0f)*_history[        (_pos-i)&1]; (++_pos)&=1; return value/BlendSmoothCubeSumHalf(2);
      case SV_WEIGHT3: _history[_pos]=current_value; value=0; _pos+=3; REP(3)value+=BlendSmoothCube(i/3.0f)*_history[Unsigned(_pos-i)%3]; (++_pos)%=3; return value/BlendSmoothCubeSumHalf(3); // have to increase '_pos' before the loop to avoid negative numbers for %
      case SV_WEIGHT4: _history[_pos]=current_value; value=0;          REP(4)value+=BlendSmoothCube(i/4.0f)*_history[        (_pos-i)&3]; (++_pos)&=3; return value/BlendSmoothCubeSumHalf(4);
      case SV_WEIGHT8: _history[_pos]=current_value; value=0;          REP(8)value+=BlendSmoothCube(i/8.0f)*_history[        (_pos-i)&7]; (++_pos)&=7; return value/BlendSmoothCubeSumHalf(8);
   #else
      Flt power;
      case SV_WEIGHT2: _history[_pos]=current_value; value=power=0;          REP(2){Flt p=BlendSmoothCube(i/2.0f); value+=p*_history[        (_pos-i)&1]; power+=p;} (++_pos)&=1; return value/power;
      case SV_WEIGHT3: _history[_pos]=current_value; value=power=0; _pos+=3; REP(3){Flt p=BlendSmoothCube(i/3.0f); value+=p*_history[Unsigned(_pos-i)%3]; power+=p;} (++_pos)%=3; return value/power; // have to increase '_pos' before the loop to avoid negative numbers for %
      case SV_WEIGHT4: _history[_pos]=current_value; value=power=0;          REP(4){Flt p=BlendSmoothCube(i/4.0f); value+=p*_history[        (_pos-i)&3]; power+=p;} (++_pos)&=3; return value/power;
      case SV_WEIGHT8: _history[_pos]=current_value; value=power=0;          REP(8){Flt p=BlendSmoothCube(i/8.0f); value+=p*_history[        (_pos-i)&7]; power+=p;} (++_pos)&=7; return value/power;
   #endif

      case SV_AVERAGE2: _history[_pos]=current_value; value=0; REP(2)value+=_history[i]; (++_pos)&=1; return value/2;
      case SV_AVERAGE3: _history[_pos]=current_value; value=0; REP(3)value+=_history[i]; (++_pos)%=3; return value/3;
      case SV_AVERAGE4: _history[_pos]=current_value; value=0; REP(4)value+=_history[i]; (++_pos)&=3; return value/4;
      case SV_AVERAGE8: _history[_pos]=current_value; value=0; REP(8)value+=_history[i]; (++_pos)&=7; return value/8;
   }
}
Vec2 SmoothValue2::update(C Vec2 &current_value)
{
   Vec2 value;
   switch(_mode)
   {
      default: return _history[0]=current_value;

   #if 1
      case SV_WEIGHT2: _history[_pos]=current_value; value=0;          REP(2)value+=BlendSmoothCube(i/2.0f)*_history[        (_pos-i)&1]; (++_pos)&=1; return value/BlendSmoothCubeSumHalf(2);
      case SV_WEIGHT3: _history[_pos]=current_value; value=0; _pos+=3; REP(3)value+=BlendSmoothCube(i/3.0f)*_history[Unsigned(_pos-i)%3]; (++_pos)%=3; return value/BlendSmoothCubeSumHalf(3); // have to increase '_pos' before the loop to avoid negative numbers for %
      case SV_WEIGHT4: _history[_pos]=current_value; value=0;          REP(4)value+=BlendSmoothCube(i/4.0f)*_history[        (_pos-i)&3]; (++_pos)&=3; return value/BlendSmoothCubeSumHalf(4);
      case SV_WEIGHT8: _history[_pos]=current_value; value=0;          REP(8)value+=BlendSmoothCube(i/8.0f)*_history[        (_pos-i)&7]; (++_pos)&=7; return value/BlendSmoothCubeSumHalf(8);
   #else
      Flt power;
      case SV_WEIGHT2: _history[_pos]=current_value; value=power=0;          REP(2){Flt p=BlendSmoothCube(i/2.0f); value+=p*_history[        (_pos-i)&1]; power+=p;} (++_pos)&=1; return value/power;
      case SV_WEIGHT3: _history[_pos]=current_value; value=power=0; _pos+=3; REP(3){Flt p=BlendSmoothCube(i/3.0f); value+=p*_history[Unsigned(_pos-i)%3]; power+=p;} (++_pos)%=3; return value/power; // have to increase '_pos' before the loop to avoid negative numbers for %
      case SV_WEIGHT4: _history[_pos]=current_value; value=power=0;          REP(4){Flt p=BlendSmoothCube(i/4.0f); value+=p*_history[        (_pos-i)&3]; power+=p;} (++_pos)&=3; return value/power;
      case SV_WEIGHT8: _history[_pos]=current_value; value=power=0;          REP(8){Flt p=BlendSmoothCube(i/8.0f); value+=p*_history[        (_pos-i)&7]; power+=p;} (++_pos)&=7; return value/power;
   #endif

      case SV_AVERAGE2: _history[_pos]=current_value; value=0; REP(2)value+=_history[i]; (++_pos)&=1; return value/2;
      case SV_AVERAGE3: _history[_pos]=current_value; value=0; REP(3)value+=_history[i]; (++_pos)%=3; return value/3;
      case SV_AVERAGE4: _history[_pos]=current_value; value=0; REP(4)value+=_history[i]; (++_pos)&=3; return value/4;
      case SV_AVERAGE8: _history[_pos]=current_value; value=0; REP(8)value+=_history[i]; (++_pos)&=7; return value/8;
   }
}
/******************************************************************************/
SmoothValueTime ::SmoothValueTime (Flt history_time) {Zero(T); historyTime(history_time);}
SmoothValueTime2::SmoothValueTime2(Flt history_time) {Zero(T); historyTime(history_time);}

SmoothValueTime& SmoothValueTime::historyTime(Flt time)
{
   Flt history_time=Max(time, 0.0f);
   T._step_time    =Max(history_time/Elms(_history), EPS); // to avoid div by zero
   return T;
}
SmoothValueTime2& SmoothValueTime2::historyTime(Flt time)
{
   Flt history_time=Max(time, 0.0f);
   T._step_time    =Max(history_time/Elms(_history), EPS); // to avoid div by zero
   return T;
}

Flt SmoothValueTime::update(Flt current_value, Flt dt)
{
   if(dt>0)
   {
      Flt left=_step_time-_time, process=Min(left, dt);
          _history[_pos]+=current_value*process; // add to current step
          _time         +=dt;
      MIN(_time_total   +=dt, (Elms(_history)-1)*_step_time); // -1 because "_history[_pos]" (current) and "_history[(_pos+1)%elms]" (oldest) are partial
      if(Int steps=Trunc(_time/_step_time))
      {
        _time-=steps*_step_time;
         if(steps>1) // following code section is needed only if there are more than 1 steps
         {
            steps--; // this step was already processed above

            // process full steps
            MIN(steps, Elms(_history)-1); // don't process more than we can actually handle, limit to one less because we're adding "last step part" on the bottom
            Flt val_step_time=current_value*_step_time;
            REP(steps)
            {
               (++_pos)&=Elms(_history)-1; ASSERT(IS_POW_2(ELMS(_history)));
              _history[_pos]=val_step_time;
            }
         }

         // process last step part
         (++_pos)&=Elms(_history)-1; ASSERT(IS_POW_2(ELMS(_history)));
        _history[_pos]=current_value*_time;
      }
   }
   if(_time_total)
   {
      Int    oldest=(_pos+1)&(Elms(_history)-1); ASSERT(IS_POW_2(ELMS(_history)));
      Flt    val=0; REPA(_history)val+=_history[i]; val-=_history[oldest]*(_time/_step_time); // '_history[oldest]' was added as full, however we need it to be "_history[oldest]*(1-_time/_step_time)" so just subtract "_history[oldest]*(_time/_step_time)"
      return val/_time_total;
   }
   return 0;
}
Vec2 SmoothValueTime2::update(C Vec2 &current_value, Flt dt)
{
   if(dt>0)
   {
      Flt left=_step_time-_time, process=Min(left, dt);
          _history[_pos]+=current_value*process; // add to current step
          _time         +=dt;
      MIN(_time_total   +=dt, (Elms(_history)-1)*_step_time); // -1 because "_history[_pos]" (current) and "_history[(_pos+1)%elms]" (oldest) are partial
      if(Int steps=Trunc(_time/_step_time))
      {
        _time-=steps*_step_time;
         if(steps>1) // following code section is needed only if there are more than 1 steps
         {
            steps--; // this step was already processed above

            // process full steps
            MIN(steps, Elms(_history)-1); // don't process more than we can actually handle, limit to one less because we're adding "last step part" on the bottom
            Vec2 val_step_time=current_value*_step_time;
            REP(steps)
            {
               (++_pos)&=Elms(_history)-1; ASSERT(IS_POW_2(ELMS(_history)));
              _history[_pos]=val_step_time;
            }
         }

         // process last step part
         (++_pos)&=Elms(_history)-1; ASSERT(IS_POW_2(ELMS(_history)));
        _history[_pos]=current_value*_time;
      }
   }
   if(_time_total)
   {
      Int    oldest=(_pos+1)&(Elms(_history)-1); ASSERT(IS_POW_2(ELMS(_history)));
      Vec2   val=0; REPA(_history)val+=_history[i]; val-=_history[oldest]*(_time/_step_time); // '_history[oldest]' was added as full, however we need it to be "_history[oldest]*(1-_time/_step_time)" so just subtract "_history[oldest]*(_time/_step_time)"
      return val/_time_total;
   }
   return 0;
}
/******************************************************************************
flt delta=target-value;
flt accel_time;
flt vel_mid=velocity+accel*accel_time; // velocity at middle point (after acceleration and before braking)
flt brake_time=vel_mid/accel;
flt brake_dist=accel/-2*brake_time*brake_time + vel_mid*brake_time;
flt remaining_dist=delta - accel/2*accel_time*accel_time - velocity*accel_time;
brake_dist=remaining_dist;
accel/-2*brake_time*brake_time + vel_mid*brake_time = delta - accel/2*accel_time*accel_time - velocity*accel_time;
accel/-2*Sqr((velocity+accel*accel_time)/accel) + (velocity+accel*accel_time)*(velocity+accel*accel_time)/accel = target-value - accel/2*accel_time*accel_time - velocity*accel_time;
accel/-2*Sqr(velocity/accel+accel_time) + (velocity*velocity + accel*accel*accel_time*accel_time + 2*velocity*accel*accel_time)/accel + value-target + accel/2*accel_time*accel_time + velocity*accel_time = 0;
accel/-2*(velocity*velocity/accel/accel + accel_time*accel_time + 2*velocity/accel*accel_time) + velocity*velocity/accel + accel*accel_time*accel_time + 2*velocity*accel_time + value-target + accel/2*accel_time*accel_time + velocity*accel_time = 0;
velocity*velocity/accel/-2 + accel_time*accel_time*accel/-2 - velocity*accel_time + velocity*velocity/accel + accel*accel_time*accel_time + 2*velocity*accel_time + value-target + accel/2*accel_time*accel_time + velocity*accel_time = 0;
accel_time*accel_time*accel/-2 + accel*accel_time*accel_time + accel/2*accel_time*accel_time - velocity*accel_time + 2*velocity*accel_time + velocity*accel_time + velocity*velocity/accel/-2 + velocity*velocity/accel + value-target = 0;
accel_time*accel_time*(accel/-2 + accel + accel/2) + accel_time*(-velocity + 2*velocity + velocity) + velocity*velocity/accel*(1/-2 + 1) + value-target = 0;
accel_time*accel_time*accel + accel_time*2*velocity + velocity*velocity/accel*0.5 + value-target = 0; */
/******************************************************************************/
void SmoothValueSettings::reset(Flt max_accel) {time_delta=Time.d(); T.max_accel=max_accel; max_velocity=FLT_MAX;}
/******************************************************************************/
void SmoothValueAccel::update(Flt target, C SmoothValueSettings &settings)
{
   if(value!=target || velocity)
      if(Flt t=settings.time_delta)
   {
      Flt accel;
      if(target>value)accel= settings.max_accel;else
      if(target<value)accel=-settings.max_accel;else
      if(velocity<0  )accel= settings.max_accel;else
                      accel=-settings.max_accel;
         
      Flt a=accel, b=2*velocity, c=velocity*velocity/accel*0.5f + value-target, d=Sqr(b)-4*a*c;
    //if(d>=0)
      {
       //Flt accel_time=(-b+Sign(a)*SqrtFast(d))/(2*a);
         Flt accel_time=-b; if(d>0)accel_time+=Sign(a)*SqrtFast(d); accel_time/=2*a;
         if( accel_time>0) // accelerate
         {
            MIN(accel_time, t);

            // value    += accel/2*t*t + velocity*t
            // velocity += accel  *t
            Flt old_velocity=velocity;
            velocity=Mid(old_velocity+accel*accel_time, -settings.max_velocity, settings.max_velocity);
            value+=Avg(old_velocity, velocity)*accel_time;
         /*#if DEBUG
            Flt brake_time=velocity/accel;
            Flt brake_dist=accel/-2*brake_time*brake_time + velocity*brake_time;
            Flt remaining_dist=target-value;
            if(accel_time>EPS)D.text(0, 0, S+"accel");
         #endif*/
            t-=accel_time;
         }
         if(t>0) // brake
         {
            Flt old_velocity=velocity;
            velocity=old_velocity-accel*t;
            if(Sign(old_velocity)!=Sign(velocity))
            {
               velocity=0;
               value=target;
            }else value+=Avg(old_velocity, velocity)*t;
         /*#if DEBUG
            D.text(0, -0.1, S+"slow");
         #endif*/
         }
      }
   }
}
void SmoothValueAccel2::update(C Vec2 &target, C SmoothValueSettings &settings)
{
   if(value!=target || velocity.any())
      if(settings.time_delta)
   {
      Vec2 accel;
      FREP(2)
         if(target  .c[i]>value.c[i])accel.c[i]= settings.max_accel;else
         if(target  .c[i]<value.c[i])accel.c[i]=-settings.max_accel;else
         if(velocity.c[i]<0         )accel.c[i]= settings.max_accel;else
                                     accel.c[i]=-settings.max_accel;

      Vec2 a=accel, b=2*velocity, c=velocity*velocity/accel*0.5f + value-target, d=Sqr(b)-4*a*c, accel_time=-b;
      FREP(2)if(d.c[i]>0)accel_time.c[i]+=Sign(a.c[i])*SqrtFast(d.c[i]);
      accel_time/=2*a;

      /*
      Flt accel_time=(-b+Sign(a)*SqrtFast(d))/(2*a);
      Flt brake_time=velocity/accel + accel_time
      Flt total_time=accel_time+brake_time;
      Flt total_time=accel_time*2 + velocity/accel
      total_time1=total_time2
      accel_time*2 + velocity/accel = 
      (-2*velocity+Sqrt(Sqr(b)-4*a*c))/(2*a)*2 + velocity/accel =
      (-2*velocity+Sqrt(4*velocity*velocity-4*accel*(velocity*velocity/accel/2 + value-target)))/accel + velocity/accel =
      (-2*velocity+Sqrt(4*velocity*velocity - 2*velocity*velocity -4*accel*(value-target)))/accel + velocity/accel =
      (-2*velocity + Sqrt(2*velocity*velocity -4*accel*(value-target)) + velocity)/accel =
      (Sqrt(2*velocity*velocity-4*accel*(value-target))-velocity)/accel =
      (Sqrt(2*velocity_x*velocity_x-4*accel_x*(value_x-target_x))-velocity_x)/accel_x = (Sqrt(2*velocity_y*velocity_y-4*accel_y*(value_y-target_y))-velocity_y)/accel_y
      (Sqrt(2*velocity_x*velocity_x-4*accel_x*(value_x-target_x))-velocity_x)*accel_y = (Sqrt(2*velocity_y*velocity_y-4*accel_y*(value_y-target_y))-velocity_y)*accel_x
      accel_x*accel_x + accel_y*accel_y = max_accel*max_accel
      accel_y*accel_y = max_accel*max_accel - accel_x*accel_x
      accel_y = Sqrt(max_accel*max_accel - accel_x*accel_x)
      (Sqrt(2*velocity_x*velocity_x-4*accel_x*(value_x-target_x))-velocity_x)*Sqrt(max_accel*max_accel - accel_x*accel_x) = (Sqrt(2*velocity_y*velocity_y-4*Sqrt(max_accel*max_accel - accel_x*accel_x)*(value_y-target_y))-velocity_y)*accel_x
      */

      Vec2 brake_time=velocity/accel; // (velocity+accel*Max(0, accel_time))/accel
      FREP(2)if(accel_time.c[i]>0)brake_time.c[i]+=accel_time.c[i];
      Vec2 total_time=accel_time+brake_time;

      accel*=Sqr(total_time); // TODO: this is an approximation
      accel.setLength(settings.max_accel);

//Flt x=(Sqrt(2*velocity.x*velocity.x-4*accel.x*(value.x-target.x))-velocity.x)/accel.x,
//    y=(Sqrt(2*velocity.y*velocity.y-4*accel.y*(value.y-target.y))-velocity.y)/accel.y;

      a=accel; c=velocity*velocity/accel*0.5f + value-target; d=Sqr(b)-4*a*c; accel_time=-b;
      FREP(2)if(d.c[i]>0)accel_time.c[i]+=Sign(a.c[i])*SqrtFast(d.c[i]);
      accel_time/=2*a;

//brake_time=velocity/accel; // (velocity+accel*Max(0, accel_time))/accel
//FREP(2)if(accel_time.c[i]>0)brake_time.c[i]+=accel_time.c[i];
//total_time=accel_time+brake_time;

      FREP(2)if(Flt ac=accel.c[i])
      {
         Flt t=settings.time_delta, act=accel_time.c[i], &velocity=T.velocity.c[i], &value=T.value.c[i];
         if(act>0) // accelerate
         {
            MIN(act, t);
            Flt old_velocity=velocity;
            velocity=Mid(old_velocity+ac*act, -settings.max_velocity, settings.max_velocity);
            value+=Avg(old_velocity, velocity)*act;
            t-=act;
         }
         if(t>0) // brake
         {
            Flt old_velocity=velocity;
            velocity=old_velocity-ac*t;
            if(Sign(old_velocity)!=Sign(velocity))
            {
               velocity=0;
               value=target.c[i];
            }else value+=Avg(old_velocity, velocity)*t;
         }
      }
   }
}
void SmoothValueAccel3::update(C Vec &target, C SmoothValueSettings &settings)
{
   if(value!=target || velocity.any())
      if(settings.time_delta)
   {
      Vec accel;
      FREP(3)
         if(target  .c[i]>value.c[i])accel.c[i]= settings.max_accel;else
         if(target  .c[i]<value.c[i])accel.c[i]=-settings.max_accel;else
         if(velocity.c[i]<0         )accel.c[i]= settings.max_accel;else
                                     accel.c[i]=-settings.max_accel;

      Vec a=accel, b=2*velocity, c=velocity*velocity/accel*0.5f + value-target, d=Sqr(b)-4*a*c, accel_time=-b;
      FREP(3)if(d.c[i]>0)accel_time.c[i]+=Sign(a.c[i])*SqrtFast(d.c[i]);
      accel_time/=2*a;

      Vec brake_time=velocity/accel; // (velocity+accel*Max(0, accel_time))/accel
      FREP(3)if(accel_time.c[i]>0)brake_time.c[i]+=accel_time.c[i];
      Vec total_time=accel_time+brake_time;

      accel*=Sqr(total_time); // TODO: this is an approximation
      accel.setLength(settings.max_accel);

      a=accel; c=velocity*velocity/accel*0.5f + value-target; d=Sqr(b)-4*a*c; accel_time=-b;
      FREP(3)if(d.c[i]>0)accel_time.c[i]+=Sign(a.c[i])*SqrtFast(d.c[i]);
      accel_time/=2*a;

      FREP(3)if(Flt ac=accel.c[i])
      {
         Flt t=settings.time_delta, act=accel_time.c[i], &velocity=T.velocity.c[i], &value=T.value.c[i];
         if(act>0) // accelerate
         {
            MIN(act, t);
            Flt old_velocity=velocity;
            velocity=Mid(old_velocity+ac*act, -settings.max_velocity, settings.max_velocity);
            value+=Avg(old_velocity, velocity)*act;
            t-=act;
         }
         if(t>0) // brake
         {
            Flt old_velocity=velocity;
            velocity=old_velocity-ac*t;
            if(Sign(old_velocity)!=Sign(velocity))
            {
               velocity=0;
               value=target.c[i];
            }else value+=Avg(old_velocity, velocity)*t;
         }
      }
   }
}
/******************************************************************************/
#if LINUX || ANDROID
static struct DevRandom
{
   Bool initialized; // additional helper member used in case 'DevRandom' gets called before its constructor or after its destructor, this is assumed to be false at app startup (because it belongs to static global 'DR' variable)
   int  fd;

   static int getFD()
   {
      int fd=open("/dev/urandom", O_RDONLY|O_NONBLOCK); // 'urandom' is lower precision but non-blocking
//if(fd<0)fd=open("/dev/random" , O_RDONLY|O_NONBLOCK); this often returns -1 when reading, so don't use
      return fd;
   }
   Bool set(Ptr data, Int size) // assumes that "data!=null"
   {
      Int r;
      if(initialized)r=read(fd, data, size);else
      { // if object was not yet initialized or was already destroyed
         r=0;
         int temp_fd =getFD(); // use temporary file descriptor
         if( temp_fd>=0){r=read(temp_fd, data, size); close(temp_fd);} // close it afterwards
      }
      return r==size;
   }

   DevRandom() {fd=getFD();     initialized=(fd>=0);}
  ~DevRandom() {if(initialized){initialized=false  ; close(fd); fd=-1;}}
}DR;
#endif
UID& UID::randomize()
{
#if WINDOWS_OLD
   #if 1 // 9x slower, more secure (MAC not included), totally random
      UuidCreate(&guid());
   #else // 9x faster, less secure (MAC     included), ID's are sequential (they increase by 1), better don't use this
      UuidCreateSequential(&guid());
   #endif
   return T;
#elif WINDOWS_NEW
   CoCreateGuid(&guid());
   return T;
#elif APPLE
   if(CFUUIDRef uuid=CFUUIDCreate(kCFAllocatorDefault))
   {
      ASSERT(SIZE(CFUUIDBytes)==SIZE(T));
      CFUUIDBytes uuid_bytes=CFUUIDGetUUIDBytes(uuid);
      T=(UID&)uuid_bytes;
      CFRelease(uuid);
      return T;
   }
#elif LINUX || ANDROID
   if(DR.set(this, SIZE(T)))return T;
#elif WEB
   if(EM_ASM_INT(var crypto=window.crypto || window.msCrypto; return typeof(crypto)!=='undefined' && typeof(crypto.getRandomValues)!=='undefined'))
   {
      EM_ASM
      ({
         var rnd=new Uint32Array(4); (window.crypto || window.msCrypto).getRandomValues(rnd);
         for(var i=0; i<4; i++)setValue($0+i*4, rnd[i], 'i32');
      }, i);
      return T;
   }
#endif

#if X64
   REPAO(T.l)=Random.l();
#else
   REPAO(T.i)=Random();
#endif

   return T;
}
UID& UID::randomizeValid()
{
   for(;;)
   {
      randomize();
      if(valid())return T;
   }
}
StrO UID::  asHex(           )C {return TextHexMem(      this, SIZE(T), false);}
Bool UID::fromHex(C Str &text)  {return TextHexMem(text, this, SIZE(T)       );}
#if 0
StrO UID::asHexRev()C
{
   Char8 temp[256], out[33];
      Set(out, TextHex(l[1], temp, 16));
   Append(out, TextHex(l[0], temp, 16));
   return out;
}
Bool UID::fromHexRev(C Str &text)
{
   Bool  ok  =(text.length()==32); // 32 (0..F) 4-bit digs = 128-bit total
   Byte *dest=b+15; // start from last byte because first characters represent most significant values
   Int   i   =0;
   for(Int src=0, process=Min(SIZEI(T), (text.length()+1)/2); i<process; ) // "(length+1)/2" because we want to process all existing characters so that "5?" gets converted to 0x50 even though '?' is invalid
   {
      Int hi=CharInt(text[src++]); if(!InRange(hi, 16)){                 ok=false; break;} hi<<=4;
      Int lo=CharInt(text[src++]); if(!InRange(lo, 16)){*dest--=hi; i++; ok=false; break;} // set 'dest' from 'hi' only and increase 'i' to mark it as processed
     *dest--=(lo|hi); i++;
   }
   for(; i<SIZE(T); i++)*dest--=0; // clear unprocessed with zeros
   return ok;
}
#endif
StrO UID::asCString()C
{
   return StrO()+"UID("+i[0]+", "+i[1]+", "+i[2]+", "+i[3]+')';
}
Bool UID::fromCString(C Str &text)
{
   if(CChar *t=_SkipWhiteChars(text))
   {
      if(Equal(t[0], 'U') && Equal(t[1], 'I') && Equal(t[2], 'D'))t=_SkipWhiteChars(t+3); // skip "UID"
      if(t && t[0]=='(')
      {
         t++;
         CalcValue value;
         Int commas=0; for(CChar *temp=t; ; ){Char c=*temp++; if(!c)break; if(c==',')commas++;}
         if( commas==16-1) // bytes
         {
            FREPA(b)
            {
               t=TextValue(t, value); if(i<commas)t=_SkipChar(TextPos(t, ',')); if(!value.type)goto error; b[i]=value.asUInt();
            }
         }else
         if(commas==4-1) // uints
         {
            FREPA(T.i)
            {
               t=TextValue(t, value); if(i<commas)t=_SkipChar(TextPos(t, ',')); if(!value.type)goto error; T.i[i]=value.asUInt();
            }
         }else
         if(commas==2-1) // ulongs
         {
            FREPA(l)
            {
               t=TextValue(t, value, false); if(i<commas)t=_SkipChar(TextPos(t, ',')); if(!value.type)goto error; l[i]=value.asULong(); // have to disable reals, because otherwise some ULong's could get converted and lose precise value
            }
         }else goto error;
         if(t=_SkipWhiteChars(t))if(t[0]==')')return true;
         return false; // leave contents on fail
      }
   }
error:
   zero(); return false;
}

StrO UID::  asFileName(           )C {return _EncodeFileName(      T);}
Bool UID::fromFileName(C Str &text)  {return  DecodeFileName(text, T);}

StrO UID::asCanonical()C
{
   StrO s; s.reserve(32+4);
   for(Int i= 4-1; i>= 0; i--){Byte b=T.b[i]; s+=Digits16[b>>4]; s+=Digits16[b&15];} s+='-';
   for(Int i= 6-1; i>= 4; i--){Byte b=T.b[i]; s+=Digits16[b>>4]; s+=Digits16[b&15];} s+='-';
   for(Int i= 8-1; i>= 6; i--){Byte b=T.b[i]; s+=Digits16[b>>4]; s+=Digits16[b&15];} s+='-';
   for(Int i= 8  ; i< 10; i++){Byte b=T.b[i]; s+=Digits16[b>>4]; s+=Digits16[b&15];} s+='-';
   for(Int i=10  ; i< 16; i++){Byte b=T.b[i]; s+=Digits16[b>>4]; s+=Digits16[b&15];}
   return s;
}
Bool UID::fromCanonical(C Str &text)
{
   if(text.length()==32+4)
   {
      CChar *t=text();
      for(Int i= 4-1; i>= 0; i--){Int hi=CharInt(*t++); Int lo=CharInt(*t++); b[i]=lo|(hi<<4); if(!InRange(hi, 16) || !InRange(lo, 16))goto error;} if(*t++!='-')goto error;
      for(Int i= 6-1; i>= 4; i--){Int hi=CharInt(*t++); Int lo=CharInt(*t++); b[i]=lo|(hi<<4); if(!InRange(hi, 16) || !InRange(lo, 16))goto error;} if(*t++!='-')goto error;
      for(Int i= 8-1; i>= 6; i--){Int hi=CharInt(*t++); Int lo=CharInt(*t++); b[i]=lo|(hi<<4); if(!InRange(hi, 16) || !InRange(lo, 16))goto error;} if(*t++!='-')goto error;
      for(Int i= 8  ; i< 10; i++){Int hi=CharInt(*t++); Int lo=CharInt(*t++); b[i]=lo|(hi<<4); if(!InRange(hi, 16) || !InRange(lo, 16))goto error;} if(*t++!='-')goto error;
      for(Int i=10  ; i< 16; i++){Int hi=CharInt(*t++); Int lo=CharInt(*t++); b[i]=lo|(hi<<4); if(!InRange(hi, 16) || !InRange(lo, 16))goto error;}
      return true;
   }
error:
   zero(); return false;
}

Bool UID::fromText(C Str &text)
{
   Str t=text;
   Int end=TextPosI(t, ')' ); if(end>=0)t.clip(end+1); // remove everything after ')' , like comments UID(..)   /* this is ...
       end=TextPosI(t, '"' ); if(end>=0)t.clip(end  ); // remove everything until '"' , like comments "bvnbasd" /* this is ...
       end=TextPosI(t, '\n'); if(end>=0)t.clip(end  ); // remove everything until '\n', this is needed in case there are multiple ID's per line (for example used in PARAM_ID_ARRAY)
   t.removeOuterWhiteChars(); if(t.first()=='"')t.remove(0); // remove first quotation (ending is done above)

   if(       fromHex(t   ))return true; // 32 char
   if(   fromCString(t   ))return true;
   if(DecodeFileName(t, T))return true; // 24 char
   zero(); return false;
}
UID& UID::operator+=(Int i)
{
   ULong temp=T.l[0]; T.l[0]+=i;
   if(i>0){if(T.l[0]<temp)T.l[1]++;}else
   if(i<0){if(T.l[0]>temp)T.l[1]--;}
   return T;
}
UID& UID::operator-=(Int i)
{
   ULong temp=T.l[0]; T.l[0]-=i;
   if(i<0){if(T.l[0]<temp)T.l[1]++;}else
   if(i>0){if(T.l[0]>temp)T.l[1]--;}
   return T;
}
UID& UID::operator+=(UInt i) {ULong temp=T.l[0]; T.l[0]+=i; if(T.l[0]<temp)T.l[1]++; return T;}
UID& UID::operator-=(UInt i) {ULong temp=T.l[0]; T.l[0]-=i; if(T.l[0]>temp)T.l[1]--; return T;}
/******************************************************************************/
// '_returned' could be a Memc<VecI2> holding ranges instead of single values, which would result in smaller memory usage, however also in smaller performance, because returning values in "Return" method would require iterating all ranges
IDGenerator::~IDGenerator()
{
  _created=0; // set '_created' to zero, so when calling 'Return' after destructor was called, it will get ignored (this can happen if calling 'Return' from a destructor of a secondary object, after destructor of 'IDGenerator' was already called)
}
IDGenerator::IDGenerator()
{
  _created=0;
}
UInt IDGenerator::New() // create new ID
{
   if(_returned.elms())return _returned.pop(); // if we have an ID that was created and later returned, then give it
   return _created++; // create new one
}
void IDGenerator::Return(UInt id) // return ID so it can be re-used later
{
   if(id<_created) // if this ID fits in range of what was created
   {
      if(_returned.elms()==_created-1) // if returned all that were created
      { // reset counter
        _created=0;
        _returned.clear();
      }else
      if(id==_created-1)_created--; // if this is the one that was created most recently
      else       _returned.add(id); // add to the list of returned ID's
   }
}
/******************************************************************************/
#if DESKTOP
static Char LogFile[MAX_LONG_PATH]={'l', 'o', 'g', '.', 't', 'x', 't', '\0'}; // use Char array to allow working even after app is being destroyed (destructors called)
#else
static Char LogFile[MAX_LONG_PATH];
#endif
static Bool LogThreadID, LogDate, LogTime, LogCurTime, LogConsoleOn;
void LogConsole(Bool on)
{
#if WINDOWS_OLD
   if(LogConsoleOn!=on)
   {
      Bool active=App.active();
      if(!on)LogConsoleOn=!( FreeConsole()!=0);else
      if(    LogConsoleOn= (AllocConsole()!=0))
      {
         SetConsoleCtrlHandler(null, true); // this disables closing the console via Ctrl+C
         if(HWND hwnd=GetConsoleWindow())if(HMENU menu=GetSystemMenu(hwnd, false))DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND); // this disables the Close button on the console window because on Windows OS once it's clicked, then entire App gets terminated immediately
         FILE *f;
         f=null; freopen_s(&f, "CONIN$" , "rb", stdin );
         f=null; freopen_s(&f, "CONOUT$", "wb", stdout);
         f=null; freopen_s(&f, "CONOUT$", "wb", stderr);
         if(active)WindowActivate(); // opening console switches focus to it, so if the app was active, then reactivate it so we won't lose the focus
      }
   }
#endif
}
static SyncLock LogLock;
void LogName(C Str &name) {Set(LogFile, name);}
Str  LogName(           ) {return   LogFile ;}
void LogDel (           ) {FDelFile(LogFile);}
void Log    (C Str &text)
{
   if(text.is())
   {
      // set text
      Str t;
   #if HAS_THREADS
      if(LogThreadID)
      {
         t="ThreadID: ";
         UIntPtr thread_id=GetThreadId();
         t+=((thread_id==App.threadID()) ? "Main      (0x"
                                         : "Secondary (0x");
         t+=TextHex(thread_id, SIZE(thread_id)*2);
         t+="), ";
      }
   #endif
      if(LogDate || LogTime)
      {
         DateTime dt; dt.getLocal();
         if(LogDate && LogTime)t+=S+"DateTime ("+dt.asText();else
         if(LogDate           )t+=S+"Date ("    +TextInt(dt.year   )+'-'+TextInt(dt.month , 2)+'-'+TextInt(dt.day   , 2);else
                               t+=S+"Time ("    +TextInt(dt.hour, 2)+':'+TextInt(dt.minute, 2)+':'+TextInt(dt.second, 2);
         t+="), ";
      }
      if(LogCurTime)
      {
         t+=S+"CurTime ("+Time.curTime()+"), ";
      }
      t+=text;

      // write to system log
   #if WINDOWS
      OutputDebugString(t);
      #if WINDOWS_OLD
         if(LogConsoleOn)fputs(Str8(t), stdout);
      #endif
   #elif APPLE
      if(NSStringAuto str=t)NSLog(@"%@", str());
   #elif LINUX
      fputs(UTF8(t), stdout); fflush(stdout); // without the flush messages won't be displayed immediately
   #elif ANDROID
      Memc<Str> lines=Split(t, '\n'); // android has limit for too long messages
      FREPA(lines){Str8 line=UTF8(lines[i]); if(line.is())__android_log_write(ANDROID_LOG_INFO, "Esenthel", line.is() ? line : " ");} // '__android_log_write' will crash if text is null or ""
   #elif WEB
      fputs(UTF8(t), stdout);
   #endif

      // write to file log
      if(Is(LogFile))
      {
         FileText f; SyncLocker lock(LogLock); if(f.append(LogFile)) // use lock to prevent 2 threads appending the same file at the same time
         {
         #if LINUX
            f.fix_new_line=false; // on Linux disable Win style line endings because its Text Editor doesn't display them correctly in some cases
         #endif
            f.putText(t);
         }
      }
   }
}
void LogN(C Str &text)
{
   Log(text+"\n");
}
void LogShow(Bool thread_id, Bool date, Bool time, Bool cur_time) {LogThreadID=thread_id; LogDate=date; LogTime=time; LogCurTime=cur_time;}
/******************************************************************************/
UInt Ceil2      (UInt  x       ) {return (x+  1)&(~  1);}
UInt Ceil4      (UInt  x       ) {return (x+  3)&(~  3);}
UInt Ceil8      (UInt  x       ) {return (x+  7)&(~  7);}
UInt Ceil16     (UInt  x       ) {return (x+ 15)&(~ 15);}
UInt Ceil32     (UInt  x       ) {return (x+ 31)&(~ 31);}
UInt Ceil64     (UInt  x       ) {return (x+ 63)&(~ 63);}
UInt Ceil128    (UInt  x       ) {return (x+127)&(~127);}
UInt CeilPow2   (UInt  x       ) {UInt b=         1; for(; b<x && b<0x80000000; )b<<=1; return b;}
UInt FloorPow2  (UInt  x       ) {UInt b=0x80000000; for(; b>x                ; )b>>=1; return b;}
UInt NearestPow2(UInt  x       ) {UInt fp2=FloorPow2(x); if(fp2!=0x80000000 && x>fp2+(fp2>>1))fp2<<=1; return fp2;} // must be > to return correct value for "x==1"
Bool IsPow2     (UInt  x       ) {return !(x&(x-1));}
Int  Log2Ceil   (UInt  x       ) {Int  i=BitHi(x); if(x>(1u  <<i))i++; return i;}
Int  Log2Ceil   (ULong x       ) {Int  i=BitHi(x); if(x>(1ull<<i))i++; return i;}
UInt Shl        (UInt  x, Int i) {return (i>=0) ? ((i<32) ? x<<i : 0) : ((i>-32) ? x>>-i : 0);}
UInt Shr        (UInt  x, Int i) {return (i>=0) ? ((i<32) ? x>>i : 0) : ((i>-32) ? x<<-i : 0);}
UInt Rol        (UInt  x, Int i) {i&=31; return (x<<i) | (x>>(32-i));}
UInt Ror        (UInt  x, Int i) {i&=31; return (x>>i) | (x<<(32-i));}

Int BitHi(UInt x)
{
#if WINDOWS
   if(!x)return 0; DWORD i; _BitScanReverse(&i, x); return i;
#elif 1
   return x ? 31^__builtin_clz(x) : 0; // 31^__builtin_clz(x)==31-__builtin_clz(x)
#else
   Int i=0; for(UInt bit=16; bit; bit>>=1)if(x>=(1<<bit)){i|=bit; x>>=bit;} return i;
#endif
}
Int BitHi(ULong x)
{
#if WINDOWS && X64
   if(!x)return 0; DWORD i; _BitScanReverse64(&i, x); return i;
#elif !WINDOWS
   return x ? 63^__builtin_clzll(x) : 0; // 63^__builtin_clzll(x)==63-__builtin_clzll(x)
#else
   Int i=0; for(UInt bit=32; bit; bit>>=1)if(x>=(1ull<<bit)){i|=bit; x>>=bit;} return i;
#endif
}

Int BitLo(UInt x)
{
#if WINDOWS
   if(!x)return 31; DWORD i; _BitScanForward(&i, x); return i;
#elif 1
   return x ? __builtin_ctz(x) : 31;
#else
   Int i=0; for(UInt bit=16; bit; bit>>=1)if(!(x&((1<<bit)-1))){i|=bit; x>>=bit;} return i;
#endif
}
Int BitLo(ULong x)
{
#if WINDOWS && X64
   if(!x)return 63; DWORD i; _BitScanForward64(&i, x); return i;
#elif !WINDOWS
   return x ? __builtin_ctzll(x) : 63;
#else
   Int i=0; for(UInt bit=32; bit; bit>>=1)if(!(x&((1ull<<bit)-1))){i|=bit; x>>=bit;} return i;
#endif
}

Int ByteHi(UInt x)
{
   if(x>0xFFFFFF)return 4;
   if(x>0x00FFFF)return 3;
   if(x>0x0000FF)return 2;
   if(x>0x000000)return 1;
                 return 0;
}
Int ByteHi(ULong x)
{
   if(x>0xFFFFFFFFFFFFFF)return 8;
   if(x>0x00FFFFFFFFFFFF)return 7;
   if(x>0x0000FFFFFFFFFF)return 6;
   if(x>0x000000FFFFFFFF)return 5;
   if(x>0x00000000FFFFFF)return 4;
   if(x>0x0000000000FFFF)return 3;
   if(x>0x000000000000FF)return 2;
   if(x>0x00000000000000)return 1;
                         return 0;
}
/******************************************************************************/
Int MidMod(Int x, Int min, Int max)
{
   return min+Mod(x-min, max-min+1);
}
/******************************************************************************/
#if WINDOWS_NEW
struct ClipSetter
{
   Windows::ApplicationModel::DataTransfer::DataPackage ^content;

   ClipSetter(Windows::ApplicationModel::DataTransfer::DataPackage ^content)
   {
      T.content=content;
      try // can crash if app not yet initialized
      {
         auto task=create_task(Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
         {
            Windows::ApplicationModel::DataTransfer::Clipboard::SetContent(T.content);
         })));
         task.wait();
      }
      catch(...){}
   }
};
struct ClipGetter
{
   Str  text;
   Bool ok;
   Windows::ApplicationModel::DataTransfer::DataPackageView ^content;

   ClipGetter()
   {
      Bool main_thread=App.mainThread();
      if(  main_thread) // 'GetContent' is single threaded and can be called only on the main thread
      {
         try // can crash if app not focused
         {
            content=Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
         }
         catch(...){}
      }else
      {
         try // can crash if app not yet initialized
         {
            auto task=create_task(Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
            {
               try // can crash if app not focused
               {
                  content=Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
               }
               catch(...){}
            })));
            task.wait();
         }
         catch(...){}
      }
      if(content && content->Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::Text))
      {
         auto task=create_task(content->GetTextAsync());
         if(main_thread)
         {
            ok=false;
            task.then([this](Platform::String ^text)
            {
               T.text=text->Data();
               ok=true;
            });
            App.loopUntil(ok);
         }else text=task.get()->Data();
      }
   }
};
#endif
Bool ClipSet(C Str &text, Bool fix_new_line)
{
#if WINDOWS_OLD
   if(OpenClipboard(null))
   {
      EmptyClipboard();
      Str temp; C Str *t=&text; if(fix_new_line)t=&(temp=FixNewLine(text));
	   Int max_length=t->length()+1;
	   if( max_length>1)if(HGLOBAL buf=GlobalAlloc(GMEM_MOVEABLE, max_length*SIZE(Char)))
      {
         Set((Char*)GlobalLock(buf), (*t)(), max_length); GlobalUnlock(buf);
         SetClipboardData(CF_UNICODETEXT, buf);
       //GlobalFree(buf); this shouldn't be called after SetClipboardData
      }
	   CloseClipboard();
	   return true;
	}
	return false;
#elif WINDOWS_NEW
   auto content=ref new Windows::ApplicationModel::DataTransfer::DataPackage;
   content->SetText(ref new Platform::String(fix_new_line ? FixNewLine(text) : text));
   if(App.mainThread())Windows::ApplicationModel::DataTransfer::Clipboard::SetContent(content);else ClipSetter cs(content);
   return true;
#elif MAC
   PasteboardClear(Pasteboard);
   if(text.is())
   {
      PasteboardSynchronize(Pasteboard);
      CFDataRef data=CFDataCreate(kCFAllocatorDefault, (UInt8*)text(), text.length()*SIZE(Char));
      PasteboardPutItemFlavor(Pasteboard, (PasteboardItemID)1, CFSTR("public.utf16-plain-text"), data, 0);
      if(data)CFRelease(data);
   }
   return true;
#elif LINUX
   // TODO: this text will disappear once the application gets closed
   if(XDisplay && App.hwnd())
      if(Atom FIND_ATOM(UTF8_STRING))
   {
      Atom FIND_ATOM(CLIPBOARD);
      Str8 utf=UTF8(text);
      int  ok=XChangeProperty(XDisplay, DefaultRootWindow(XDisplay), XA_CUT_BUFFER0, UTF8_STRING, 8, PropModeReplace, (const unsigned char*)utf(), utf.length());
      if(CLIPBOARD && XGetSelectionOwner(XDisplay, CLIPBOARD )!=App.Hwnd())XSetSelectionOwner(XDisplay, CLIPBOARD , App.Hwnd(), CurrentTime);
      if(             XGetSelectionOwner(XDisplay, XA_PRIMARY)!=App.Hwnd())XSetSelectionOwner(XDisplay, XA_PRIMARY, App.Hwnd(), CurrentTime);
      return ok==1;
   }
   return false;
#elif IOS
   Bool   ok=false; if(NSStringAuto str=text)if(UIPasteboard *pasteboard=[UIPasteboard generalPasteboard]){[pasteboard setString:str]; ok=true;}
   return ok;
#elif ANDROID
   JNI jni;
   if( jni && ClipboardManager) // system clipboard supported
   {
   #if 1
      if(JMethodID setText=jni->GetMethodID(ClipboardManagerClass, "setText", "(Ljava/lang/CharSequence;)V"))
      if(JString jtext=JString(jni, text))
      {
         jni->CallVoidMethod(ClipboardManager, setText, jtext());
         return true;
      }
   #else
      if(JClass ClipDataClass=JClass(jni, "android/content/ClipData"))
      if(JMethodID newPlainText=jni->GetStaticMethodID(ClipDataClass, "newPlainText", "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;"))
      if(JString label=JString(jni, "Esenthel"))
      if(JString jtext=JString(jni, text))
      if(JObject ClipData=JObject(jni, jni->CallStaticObjectMethod(ClipDataClass, newPlainText, label(), jtext())))
      if(JMethodID setPrimaryClip=jni->GetMethodID(ClipboardManagerClass, "setPrimaryClip", "(Landroid/content/ClipData;)V"))
      {
         jni->CallVoidMethod(ClipboardManager, setPrimaryClip, ClipData());
         return true;
      }
   #endif
      return false;
   }
   Clipboard=text;
   return true;
#else
	Clipboard=text;
   return true;
#endif
}
Str ClipGet(Bool fix_new_line)
{
   Str s;
#if WINDOWS_OLD
   if(IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(null))
   {
      if(HANDLE hData=GetClipboardData(CF_UNICODETEXT))
      {
         s=(Char*)GlobalLock  (hData);
                  GlobalUnlock(hData);
         if(fix_new_line)s.replace('\r', '\0');
      }
      CloseClipboard();
   }
#elif WINDOWS_NEW
   ClipGetter cg; if(fix_new_line)cg.text.replace('\r', '\0');
   return     cg.text;
#elif MAC
   #if 1
      if(NSPasteboard *pasteboard=[NSPasteboard generalPasteboard])
         return [pasteboard stringForType:NSPasteboardTypeString]; // don't release this as it will crash
   #else // this does not properly support new lines (when accessed from Xcode for example)
      Bool found=false;
      PasteboardSynchronize(Pasteboard);
      ItemCount itemCount; PasteboardGetItemCount(Pasteboard, &itemCount);
      for(UInt32 itemIndex=1; itemIndex<=itemCount && !found; itemIndex++)
      {
         PasteboardItemID itemID         ; PasteboardGetItemIdentifier(Pasteboard, itemIndex, &itemID);
         CFArrayRef       flavorTypeArray; PasteboardCopyItemFlavors(Pasteboard, itemID, &flavorTypeArray);
         CFIndex          flavorCount=CFArrayGetCount(flavorTypeArray);
         
         for(CFIndex flavorIndex=0; flavorIndex<flavorCount && !found; flavorIndex++)
         {
            CFStringRef flavorType=(CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);
            if(UTTypeConformsTo(flavorType, CFSTR("public.utf16-plain-text")))
            {
               found=true;
               CFDataRef flavorData; PasteboardCopyItemFlavorData(Pasteboard, itemID, flavorType, &flavorData);
               CFIndex   flavorDataSize=CFDataGetLength(flavorData);
               Char     *src           =(Char*)CFDataGetBytePtr(flavorData);
               REP(flavorDataSize/SIZE(Char))s+=*src++;
               if(flavorData)CFRelease(flavorData);
            }
         }
         if(flavorTypeArray)CFRelease(flavorTypeArray);
      }
   #endif
#elif LINUX
   if(XDisplay)
      if(Atom FIND_ATOM(CLIPBOARD))
      if(Atom FIND_ATOM(UTF8_STRING))
   {
      Atom    selection;
      XWindow owner=XGetSelectionOwner(XDisplay, CLIPBOARD);
      if(!owner || owner==App.Hwnd() || !App.hwnd())
      {
         owner=DefaultRootWindow(XDisplay);
         selection=XA_CUT_BUFFER0;
      }else
      {
         owner=App.Hwnd();
         Atom GET_ATOM(EE_SELECTION); selection=EE_SELECTION;
         XConvertSelection(XDisplay, CLIPBOARD, UTF8_STRING, EE_SELECTION, owner, CurrentTime);
         XSync(XDisplay, false);
         REP(1024) // attempts to check if window got selection, otherwise get window property will fail (this is still needed even though 'XSync' is used)
         {
            XEvent event; if(XCheckTypedWindowEvent(XDisplay, App.Hwnd(), SelectionNotify, &event))break;
            usleep(1);
         }
      }
      Atom           type;
      int            format=0;
      unsigned long  items=0;
      unsigned long  overflow=0;
      unsigned char *data=null;
      if(!XGetWindowProperty(XDisplay, owner, selection, 0, INT_MAX/4, false, UTF8_STRING, &type, &format, &items, &overflow, &data))
         if(type==UTF8_STRING)s=FromUTF8((char*)data);
      if(data)XFree(data);
   }
#elif IOS
   if(UIPasteboard *pasteboard=[UIPasteboard generalPasteboard])return pasteboard.string;
#elif ANDROID
   JNI jni;
   if( jni && ClipboardManager) // system clipboard supported
   {
   #if 1
      if(JMethodID getText=jni->GetMethodID(ClipboardManagerClass, "getText", "()Ljava/lang/CharSequence;"))
      if(JString text=JString(jni, jni->CallObjectMethod(ClipboardManager, getText)))
         return text.str();
   #else
      if(JClass ClipDataClass=JClass(jni, "android/content/ClipData"))
      if(JMethodID getPrimaryClip=jni->GetMethodID(ClipboardManagerClass, "getPrimaryClip", "()Landroid/content/ClipData;"))
      if(JObject ClipData=JObject(jni, jni->CallObjectMethod(ClipboardManager, getPrimaryClip)))
      if(JMethodID getItemAt=jni->GetMethodID(ClipDataClass, "getItemAt", "(I)Landroid/content/ClipData$Item;"))
      if(JObject ClipDataItem=JObject(jni, jni->CallObjectMethod(ClipData, getItemAt, jint(0))))
      if(JClass ClipDataItemClass=JClass(jni, ClipDataItem))
      if(JMethodID getText=jni->GetMethodID(ClipDataItemClass, "getText", "()Ljava/lang/CharSequence;"))
      if(JString text=JString(jni, jni->CallObjectMethod(ClipDataItem, getText)))
         return text.str();
   #endif
   }
   return Clipboard;
#else
	return Clipboard;
#endif
   return s;
}
/******************************************************************************/
VecI4 OSVerNumber()
{
#if WINDOWS_OLD
   #if 1 // faster
      DWORD ver=GetVersion();
      return VecI4(LOBYTE(LOWORD(ver)), HIBYTE(LOWORD(ver)), 0, 0);
   #else // slower
      OSVERSIONINFOEX v; Zero(v);
      v.dwOSVersionInfoSize=SIZE(v);
      if(GetVersionEx((OSVERSIONINFO*)&v))return VecI4(v.dwMajorVersion, v.dwMinorVersion, 0, 0);
   #endif
#elif WINDOWS_NEW
   return VecI4(10, 0, 0, 0);
#elif ANDROID
   return VecI4(AndroidSDK, 0, 0, 0);
#elif MAC
   // TODO:
#elif IOS
   if(NSString *ver=[[UIDevice currentDevice] systemVersion]) // don't release this
   {
      Str v=ver; // this returns for example "8.1.3"
      v.replace('.', ',');
      return TextVecI4(v);
   }
#endif
   return 0;
}
OS_VER OSVer()
{
#if WINDOWS_OLD
   OSVERSIONINFOEX v; Zero(v);
   v.dwOSVersionInfoSize=SIZE(v);
   if(GetVersionEx((OSVERSIONINFO*)&v))
   {
      if(v.dwMajorVersion==10)
      {
         if(v.dwMinorVersion==0)return (v.wProductType==VER_NT_WORKSTATION) ? WINDOWS_10 : WINDOWS_SERVER_2016;
      }else
      if(v.dwMajorVersion==6)
      {
         if(v.dwMinorVersion==0)return (v.wProductType==VER_NT_WORKSTATION) ? WINDOWS_VISTA : WINDOWS_SERVER_2008;
         if(v.dwMinorVersion==1)return (v.wProductType==VER_NT_WORKSTATION) ? WINDOWS_7     : WINDOWS_SERVER_2008_R2;
         if(v.dwMinorVersion==2)return (v.wProductType==VER_NT_WORKSTATION) ? WINDOWS_8     : WINDOWS_SERVER_2012;
         if(v.dwMinorVersion==3)return (v.wProductType==VER_NT_WORKSTATION) ? WINDOWS_8/*_1*/     : WINDOWS_SERVER_2012_R2;
      }else
      if(v.dwMajorVersion==5)
      {
         if(v.dwMinorVersion==2)
         {
            if(v.wProductType==VER_NT_WORKSTATION)return WINDOWS_XP_64;
            if(GetSystemMetrics(SM_SERVERR2)     )return WINDOWS_SERVER_2003_R2;
                                                  return WINDOWS_SERVER_2003;
         }else
         if(v.dwMinorVersion==1)return WINDOWS_XP  ;else
         if(v.dwMinorVersion==0)return WINDOWS_2000;
      }
   }
   return WINDOWS_UNKNOWN;
#elif WINDOWS_NEW
   return WINDOWS_10;
#elif MAC
   return OS_MAC;
#elif LINUX
   return OS_LINUX;
#elif ANDROID
   // SDK API levels taken from:
   // http://developer.android.com/guide/appendix/api-levels.html
   // http://en.wikipedia.org/wiki/Android_version_history#Version_history_by_API_level
   if(AndroidSDK>=28)return ANDROID_PIE;
   if(AndroidSDK>=26)return ANDROID_OREO;
   if(AndroidSDK>=24)return ANDROID_NOUGAT;
   if(AndroidSDK>=23)return ANDROID_MARSHMALLOW;
   if(AndroidSDK>=21)return ANDROID_LOLLIPOP;
   if(AndroidSDK>=19)return ANDROID_KIT_KAT;
   if(AndroidSDK>=16)return ANDROID_JELLY_BEAN;
   if(AndroidSDK>=14)return ANDROID_ICE_CREAM_SANDWICH;
   if(AndroidSDK>=11)return ANDROID_HONEYCOMB;
   if(AndroidSDK>= 9)return ANDROID_GINGERBREAD;
                     return ANDROID_UNKNOWN;
#elif IOS
   return OS_IOS;
#elif WEB
   switch(EM_ASM_INT
   (
      if(navigator.platform.indexOf('Win')==0)return 0;
      if(navigator.platform.indexOf('Mac')==0)return 1;
      if(navigator.appVersion.indexOf('Android')>=0)return 3; // !! test this before Linux !!
      if(navigator.platform.indexOf('Linux')==0)return 2;
      if(navigator.platform.indexOf('iPhone')==0)return 4;
      return -1;
   ))
   {
      case 0: return WINDOWS_UNKNOWN;
      case 1: return OS_MAC;
      case 2: return OS_LINUX;
      case 3: return ANDROID_UNKNOWN;
      case 4: return OS_IOS;
   }
#endif
   return OS_UNKNOWN;
}
OS_VER OSGroup(OS_VER ver)
{
   if(OSWindows(ver))return WINDOWS_UNKNOWN;
   if(OSAndroid(ver))return ANDROID_UNKNOWN;
                     return ver;
}
CChar8* OSName(OS_VER ver)
{
   switch(ver)
   {
      default                        : return "Unknown"; // OS_UNKNOWN
      case WINDOWS_UNKNOWN           : return "Windows";
      case WINDOWS_2000              : return "Windows 2000";
      case WINDOWS_XP                : return "Windows XP";
      case WINDOWS_XP_64             : return "Windows XP 64";
      case WINDOWS_VISTA             : return "Windows Vista";
      case WINDOWS_7                 : return "Windows 7";
      case WINDOWS_8                 : return "Windows 8";
    //case WINDOWS_8_1               : return "Windows 8.1";
      case WINDOWS_10                : return "Windows 10";
      case WINDOWS_SERVER_2003       : return "Windows Server 2003";
      case WINDOWS_SERVER_2003_R2    : return "Windows Server 2003 R2";
      case WINDOWS_SERVER_2008       : return "Windows Server 2008";
      case WINDOWS_SERVER_2008_R2    : return "Windows Server 2008 R2";
      case WINDOWS_SERVER_2012       : return "Windows Server 2012";
      case WINDOWS_SERVER_2012_R2    : return "Windows Server 2012 R2";
      case WINDOWS_SERVER_2016       : return "Windows Server 2016";
      case OS_MAC                    : return "Mac";
      case OS_LINUX                  : return "Linux";
      case ANDROID_UNKNOWN           : return "Android";
      case ANDROID_GINGERBREAD       : return "Android Gingerbread";
      case ANDROID_HONEYCOMB         : return "Android Honeycomb";
      case ANDROID_ICE_CREAM_SANDWICH: return "Android Ice Cream Sandwich";
      case ANDROID_JELLY_BEAN        : return "Android Jelly Bean";
      case ANDROID_KIT_KAT           : return "Android Kit Kat";
      case ANDROID_LOLLIPOP          : return "Android Lollipop";
      case ANDROID_MARSHMALLOW       : return "Android Marshmallow";
      case ANDROID_NOUGAT            : return "Android Nougat";
      case ANDROID_OREO              : return "Android Oreo";
      case ANDROID_PIE               : return "Android Pie";
      case OS_IOS                    : return "iOS";
   }
}
Bool OSWindows(OS_VER ver) {return ver>=WINDOWS_UNKNOWN && ver<=WINDOWS_SERVER_2016;}
Bool OSMac    (OS_VER ver) {return ver==OS_MAC;}
Bool OSLinux  (OS_VER ver) {return ver==OS_LINUX;}
Bool OSAndroid(OS_VER ver) {return ver>=ANDROID_UNKNOWN && ver<=ANDROID_PIE;}
Bool OSiOS    (OS_VER ver) {return ver==OS_IOS;}
/******************************************************************************/
#if WINDOWS_NEW
static struct UserNameGetter
{
   Str      name;
   Bool     is;
   SyncLock lock;

 C Str& get(Bool wait=true)
   {
      if(!is && App.hwnd()) // !! we can call this only after window was created, otherwise this will crash !!
      {
         if(App.mainThread()) // 'User::FindAllAsync' can be called only on the main thread
         {
            create_task(User::FindAllAsync(UserType::LocalUser, UserAuthenticationStatus::LocallyAuthenticated)).then([this](IVectorView<User^>^ users)
            {
               if(users->Size>0)
               {
                  auto properties = ref new Vector<Platform::String^>();
                  properties->Append(KnownUserProperties::FirstName);
                  properties->Append(KnownUserProperties:: LastName);
                  create_task(users->First()->Current->GetPropertiesAsync(properties->GetView())).then([this](IPropertySet^ values)
                  {
                     if(auto first_name=safe_cast<Platform::String^>(values->Lookup(KnownUserProperties::FirstName)))
                     if(auto  last_name=safe_cast<Platform::String^>(values->Lookup(KnownUserProperties:: LastName)))
                     {
                        SyncLocker locker(lock); // sync to avoid modifying the name on multiple threads
                        if(!is){name=first_name->Data(); name.space()+=last_name->Data(); is=true;} // set 'is' at the end, once 'name' is ready, because as soon as 'is' is true, then other threads may access the name, also set 'is' here as well even though we're setting it later, to avoid modifying/reading the name on multiple threads
                     }
                     is=true; // set this in case there's no FirstName/LastName available
                  });
               }else is=true; // set this in case there are no users available
            });
         }else
         {
            auto task=create_task(Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
            {
               // we're on the main thread now, so just call 'get' again, but this time don't wait
               get(false); // !! we can't and don't need to wait here, as we only want to request user data so it can be obtained back on the caller thread, if we wait, then crash will occur, because this is actually called inside a Windows Callback on the main thread, and if we wait then 'loopUntil' would call Windows Callbacks again
            })));
         }

         if(wait)
            if(App.mainThread())App.loopUntil(is, true);else for(; !is; )Time.wait(1); // wait because app may have to wait a long time until user agrees to provide permission which would cause full CPU usage
      }
      return name;
   }
}UserName;
#elif ANDROID
static SyncLock AndroidUserNameLock;
static Str      AndroidUserName;
#endif
Str OSUserName(Bool short_name)
{
#if WINDOWS_OLD
   wchar_t user_name[256+1]; DWORD size=Elms(user_name);
   if(short_name ? GetUserName(user_name, &size) : GetUserNameEx(NameDisplay, user_name, &size))return user_name;
   return S;
#elif WINDOWS_NEW // calling this method will result in asking for permission on the screen
   return UserName.get();
#elif MAC
   Char8 name[MAX_UTF_PATH]; name[0]=0;
   #if 0
      getlogin_r(name, Elms(name));
   #else
      if(CFStringRef cf_name=CSCopyUserName(short_name))
      {
         CFStringGetCString(cf_name, name, Elms(name), kCFStringEncodingUTF8);
         CFRelease(cf_name);
      }
   #endif
   return FromUTF8(name);
#elif LINUX
   __uid_t uid=geteuid();
   passwd pw, *pwp;
   Char8 temp[4096];
   if(!getpwuid_r(uid, &pw, temp, SIZE(temp), &pwp))
   {
      if(!short_name) // Linux may add unnecessary commas at the end "name,,,"
      {
         for(Int i=SetReturnLength(temp, pw.pw_gecos); i>0 && temp[i-1]==','; )temp[--i]='\0';
         return FromUTF8(temp);
      }
      return FromUTF8(pw.pw_name);
   }
   return S;
#elif IOS
	return [[UIDevice currentDevice] name]; // don't release this NSString as it causes errors
#elif ANDROID
   if(!AndroidUserName.is())
   {
      SyncLocker locker(AndroidUserNameLock);
      if(!AndroidUserName.is())
      {
         RequirePermission(PERMISSION_USER_NAME);

         JNI jni;
         if(jni && Activity)
         if(JClass AccountManagerClass=JClass(jni, "android/accounts/AccountManager"))
         if(JClass AccountClass=JClass(jni, "android/accounts/Account"))
         if(JMethodID get=jni->GetStaticMethodID(AccountManagerClass, "get", "(Landroid/content/Context;)Landroid/accounts/AccountManager;"))
         if(JObject mgr=JObject(jni, jni->CallStaticObjectMethod(AccountManagerClass, get, Activity)))
         if(JMethodID getAccountsByType=jni->GetMethodID(AccountManagerClass, "getAccountsByType", "(Ljava/lang/String;)[Landroid/accounts/Account;"))
         if(JString com_google=JString(jni, "com.google"))
         if(JObjectArray accounts=JObjectArray(jni, jni->CallObjectMethod(mgr, getAccountsByType, com_google())))
         {
            Int length=accounts.elms();
            if( length>=1)
            if(JObject account=JObject(jni, accounts[0]))
            if(JFieldID name=jni->GetFieldID(AccountClass, "name", "Ljava/lang/String;"))
            if(JString n=JString(jni, jni->GetObjectField(account, name)))
               AndroidUserName=n.str();
         }
      }
   }
   return AndroidUserName;
#else
   return S;
#endif
}
/******************************************************************************/
static struct Locale
{
   LANG_TYPE lang;
   CChar8   *code;
}locale[]=
{
   {(LANG_TYPE)LANG_AFRIKAANS , "af"},
   {(LANG_TYPE)LANG_ARABIC    , "ar"},
   {(LANG_TYPE)LANG_ARMENIAN  , "hy"},
   {(LANG_TYPE)LANG_BELARUSIAN, "be"},
   {(LANG_TYPE)LANG_BULGARIAN , "bg"},
   {(LANG_TYPE)LANG_CHINESE   , "zh"},
   {(LANG_TYPE)LANG_CROATIAN  , "hr"},
   {(LANG_TYPE)LANG_CZECH     , "cs"},
   {(LANG_TYPE)LANG_DANISH    , "da"},
   {(LANG_TYPE)LANG_DUTCH     , "nl"},
   {(LANG_TYPE)LANG_ENGLISH   , "en"},
   {(LANG_TYPE)LANG_ESTONIAN  , "et"},
   {(LANG_TYPE)LANG_FILIPINO  , "tl"},
   {(LANG_TYPE)LANG_FINNISH   , "fi"},
   {(LANG_TYPE)LANG_FRENCH    , "fr"},
   {(LANG_TYPE)LANG_GALICIAN  , "gl"},
   {(LANG_TYPE)LANG_GERMAN    , "de"},
   {(LANG_TYPE)LANG_GREEK     , "el"},
   {(LANG_TYPE)LANG_HEBREW    , "he"},
   {(LANG_TYPE)LANG_HUNGARIAN , "hu"},
   {(LANG_TYPE)LANG_HINDI     , "hi"},
   {(LANG_TYPE)LANG_ICELANDIC , "is"},
   {(LANG_TYPE)LANG_INDONESIAN, "id"},
   {(LANG_TYPE)LANG_IRISH     , "ga"},
   {(LANG_TYPE)LANG_ITALIAN   , "it"},
   {(LANG_TYPE)LANG_JAPANESE  , "ja"},
   {(LANG_TYPE)LANG_KOREAN    , "ko"},
   {(LANG_TYPE)LANG_LATVIAN   , "lv"},
   {(LANG_TYPE)LANG_LITHUANIAN, "lt"},
   {(LANG_TYPE)LANG_MALAY     , "ms"},
   {(LANG_TYPE)LANG_MONGOLIAN , "mn"},
   {(LANG_TYPE)LANG_NORWEGIAN , "nb"},
   {(LANG_TYPE)LANG_NORWEGIAN , "nn"},
   {(LANG_TYPE)LANG_PERSIAN   , "fa"},
   {(LANG_TYPE)LANG_POLISH    , "pl"},
   {(LANG_TYPE)LANG_PORTUGUESE, "pt"},
   {(LANG_TYPE)LANG_RUSSIAN   , "ru"},
   {(LANG_TYPE)LANG_ROMANIAN  , "ro"},
   {(LANG_TYPE)LANG_SERBIAN   , "sr"},
   {(LANG_TYPE)LANG_SPANISH   , "es"},
   {(LANG_TYPE)LANG_SLOVAK    , "sk"},
   {(LANG_TYPE)LANG_SLOVENIAN , "sl"},
   {(LANG_TYPE)LANG_SWAHILI   , "sw"},
   {(LANG_TYPE)LANG_SWEDISH   , "sv"},
   {(LANG_TYPE)LANG_TAMIL     , "ta"},
   {(LANG_TYPE)LANG_THAI      , "th"},
   {(LANG_TYPE)LANG_TURKISH   , "tr"},
   {(LANG_TYPE)LANG_UKRAINIAN , "uk"},
   {(LANG_TYPE)LANG_VIETNAMESE, "vi"},
   {(LANG_TYPE)LANG_XHOSA     , "xh"},
   {(LANG_TYPE)LANG_ZULU      , "zu"},
};
LANG_TYPE LanguageCode(C Str &lang)
{
   if(lang.is())FREPA(locale)if(Starts(lang, locale[i].code))return locale[i].lang;
   return LANG_UNKNOWN;
}
CChar8* LanguageCode(LANG_TYPE lang)
{
   if(lang)FREPA(locale)if(locale[i].lang==lang)return locale[i].code;
   return null;
}
LANG_TYPE OSLanguage()
{
#if WINDOWS_OLD
   return LANG_TYPE(GetSystemDefaultLangID()&0xFF);
#elif WINDOWS_NEW
   if(auto langs=Windows::System::UserProfile::GlobalizationPreferences::Languages)
      if(langs->Size>0)return LanguageCode(langs->GetAt(0)->Data());
#elif APPLE
   LANG_TYPE lang=LANG_UNKNOWN;
   if(CFArrayRef langs=CFLocaleCopyPreferredLanguages())
   {
      FREP(CFArrayGetCount(langs))
      {
         CFStringRef l=(CFStringRef)CFArrayGetValueAtIndex(langs, i);
         Char8       name[MAX_UTF_PATH]; CFStringGetCString(l, name, Elms(name), kCFStringEncodingUTF8);
         if(lang=LanguageCode(name))break;
      }
      CFRelease(langs);
   }
   return lang;
#elif ANDROID
   if(AndroidApp && AndroidApp->config)
   {
      char code[3]={0, 0, 0}; AConfiguration_getLanguage(AndroidApp->config, code); // getLanguage fills only first 2 characters (they will not be zero-terminated)
      return LanguageCode(code);
   }
#elif LINUX || WEB
   return LanguageCode(setlocale(LC_CTYPE, ""));
#endif
   return LANG_UNKNOWN;
}
/******************************************************************************/
Str LanguageSpecific(LANG_TYPE lang)
{
   switch(lang)
   {
      case PL        : return u"ĄĆĘŁŃÓŚŻŹąćęłńóśżź";
      case DE        : return u"äÄëËöÖüÜßẞ";
      case FR        : return u"àÀáÁâÂãÃäÄåÅæÆçÇèÈéÉêÊëËìÌíÍîÎïÏðÐñÑòÒóÓôÔõÕöÖøØùÙúÚûÛüÜýÝ";
      case RU        : return u"аАбБвВгГдДеЕёЁжЖзЗиИйЙкКлЛмМнНоОпПрРсСтТуУфФхХцЦчЧшШщЩъЪыЫьЬэЭюЮяЯ";
      case PO        : return u"áÁàÀâÂãÃăĂçÇęĘéÉêÊíÍóÓőŐõÕôÔŕŔúÚ";
      case LANG_GREEK: return u"αΑβΒγΓδΔεΕζΖηΗθΘιΙκΚλΛμΜνΝξΞοΟπΠρΡσΣτΤυΥφΦχΧψΨωΩ";
      case LANG_THAI : return u"กขฃคฅฆงจฉชซฌญฎฏฐฑฒณดตถทธนบปผฝพฟภมยรฤลฦวศษสหฬอฮฯะัาำิีึืฺุู฿เแโใไๅๆ็่้๊๋์ํ๎๏๐๑๒๓๔๕๖๗๘๙๚๛"; // https://en.wikipedia.org/wiki/Thai_alphabet#Unicode

      case CN:
      {
         // ranges provided by: Chinese company Moli
         Str s; s.reserve(0xFFFF);
         for(Int i=0x3400; i<=0x4DB5; i++)s+=Char(i);
         for(Int i=0x4E00; i<=0x9FCB; i++)s+=Char(i);
         for(Int i=0xF900; i<=0xFAD9; i++)s+=Char(i);
         for(Int i=0x3000; i<=0x303F; i++)s+=Char(i); // punctuation (like '。' and 'FullWidthSpace')
         return s;
      }
      
      case JP:
      {
         // ranges taken from: http://en.wikipedia.org/wiki/Japanese_writing_system
         Str s; s.reserve(0xFFFF);
         for(Int i=0x4E00; i<=0x9FBF; i++)s+=Char(i); // Kanji
         for(Int i=0x3040; i<=0x309F; i++)s+=Char(i); // Hiragana
         for(Int i=0x30A0; i<=0x30FF; i++)s+=Char(i); // Katakana
         for(Int i=0x3000; i<=0x303F; i++)s+=Char(i); // punctuation (like '。' and 'FullWidthSpace')
         return s;
      }
      
      case KO:
      {
         // ranges taken from: http://en.wikipedia.org/wiki/Hangul
         Str s; s.reserve(0xFFFF);
         for(Int i=0xAC00; i<=0xD7AF; i++)s+=Char(i);
         for(Int i=0x1100; i<=0x11FF; i++)s+=Char(i);
         for(Int i=0x3130; i<=0x318F; i++)s+=Char(i);
         for(Int i=0x3200; i<=0x32FF; i++)s+=Char(i);
         for(Int i=0xA960; i<=0xA97F; i++)s+=Char(i);
         for(Int i=0xD7B0; i<=0xD7FF; i++)s+=Char(i);
         for(Int i=0xFF00; i<=0xFFEF; i++)s+=Char(i);
         return s;
      }
   }
   return S;
}
/******************************************************************************/
static Bool SpecialLink(C Str &name)
{
// http://, https://, ftp://, mailto://, tel://, fb://, steam://, ms-settings://
   FREPA(name)
   {
      Char c=name()[i]; // () avoids range checks
      if(c>='a' && c<='z'
      || c>='A' && c<='Z'
      || c>='0' && c<='9'
      || c=='-')continue;
      if(c==':')return name[i+1]=='/' && name[i+2]=='/';
      break;
   }
   return false;
}
Bool Explore(C Str &name, Bool select)
{
   FileInfoSystem fi(name);
   Bool file=(fi.type==FSTD_FILE);
#if MAC
   file|=(fi.type==FSTD_DIR && Equal(_GetExt(name), "app")); // on Mac applications are actually folders
#endif
   if(fi.type && (select || file)) // always select if this is a file
   {
   #if WINDOWS_OLD
      if(auto item=ILCreateFromPath(MakeFullPath(name).replace('/', '\\'))) // 'MakeFullPath' is needed in case we're exploring relative to current dir, because even though 'FileInfoSystem' succeeds, the 'ILCreateFromPath' will fail
      {
         Bool   ok=OK(SHOpenFolderAndSelectItems(item, 0, 0, 0)); ILFree(item);
         return ok;
      }
   #elif MAC
      Bool ok=false; 
      Str  full=UnixPath(MakeFullPath(name));
      if(NSStringAuto ns_name=full)
      if(NSStringAuto ns_path=GetPath(full))
         ok=[[NSWorkspace sharedWorkspace] selectFile:ns_name inFileViewerRootedAtPath:ns_path];
      return ok;
   #elif LINUX
      // TODO: Linux Explore select
   #endif
   }
   return (fi.type || SpecialLink(name)) ? Run(file ? GetPath(name) : name) : false; // this function was designed to never run programs so open the parent folder when a file is detected
}
Bool Run(C Str &name, C Str &params, Bool hidden, Bool as_admin)
{
   if(name.is())
   {
   #if WINDOWS_OLD
      if(SpecialLink(name))return IntPtr(ShellExecute(null,                       L"open",     name                  , params,                null  , hidden ? SW_HIDE : SW_SHOWNORMAL))>32;
                           return IntPtr(ShellExecute(null, as_admin ? L"runas" : L"open", Str(name).tailSlash(false), params, WChar(_GetPath(name)), hidden ? SW_HIDE : SW_SHOWNORMAL))>32; // always remove tail slash, because if trying to open a local folder "Folder/" with slash at the end then it will fail (however if full path is used, or no tail slash, then it will work)
   #elif WINDOWS_NEW
      if(SpecialLink(name))return Windows::System::Launcher::LaunchUriAsync(ref new Windows::Foundation::Uri(ref new Platform::String(name)))!=null;
      switch(FileInfoSystem(name).type)
      {
         case FSTD_FILE:
         {
            create_task(Windows::Storage::StorageFile::GetFileFromPathAsync(ref new Platform::String(WindowsPath(name)))).then([](task<Windows::Storage::StorageFile^> task) // 'WindowsPath' must be used or exception will occur when using '/' instead of '\'
            {
               try{Windows::System::Launcher::LaunchFileAsync(task.get());} catch(...){}
            });
         }return true;

         case FSTD_DIR:
         {
            create_task(Windows::Storage::StorageFolder::GetFolderFromPathAsync(ref new Platform::String(WindowsPath(name)))).then([](task<Windows::Storage::StorageFolder^> task) // 'WindowsPath' must be used or exception will occur when using '/' instead of '\'
            {
               try{Windows::System::Launcher::LaunchFolderAsync(task.get());} catch(...){}
            });
         }return true;
      }
   #elif LINUX
      Str8 command; FileInfoSystem fi(name);
      if(SpecialLink(name) || fi.type==FSTD_DRIVE || fi.type==FSTD_DIR)command="xdg-open"; // special links and folders/drives can be opened only with "xdg-open" command
      command.space()+='"'; command+=UnixPathUTF8(name); command+='"';
      if(params.is())command.space()+=UnixPathUTF8(params);
      command+=" &"; // normally 'system' is blocking on Linux, but adding " &" makes the call non-blocking
      return system(command)!=0;
   #elif MAC
      Str8 command; Bool open=false; FileInfoSystem fi(name);
      if(SpecialLink(name) || fi.type==FSTD_DRIVE || fi.type==FSTD_DIR){open=true; command="open"; if(fi.type==FSTD_DIR && Equal(_GetExt(name), "app"))command.space()+="--new"; if(hidden)command.space()+="--hide";} // special links and folders/drives can be opened only with "open" command, use --new for apps so multiple instances can be opened
      command.space()+='"'; command+=UnixPathUTF8(name); command+='"';
      if(params.is()){if(open)command.space()+="--args"; command.space()+=UnixPathUTF8(params);}
      return system(command)!=0;
   #elif IOS
      Bool ok=false;
    //if(SpecialLink(name)) open everything through 'openURL' as there's no other way
      if(NSURLAuto url=name)
      {
      #if 0
         [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil]; ok=true; // this 'openURL' is async and can't return success, which is now available in 'completionHandler' that will be executed on main thread, since we can't wait, just assume OK
      #else
         ok=[[UIApplication sharedApplication] openURL:url]; // deprecated
      #endif
      }
      return ok;
   #elif ANDROID
      Str path=name;
      if(IsSlash(path[0]) && !Contains(path, ':'))path=S+"file://"+path; // "/sdcard/DCIM/" -> "file:///sdcard/DCIM/"

      JNI jni;
      if(jni && ActivityClass)
      if(JClass uriClass=JClass(jni, "android/net/Uri"))
      if(JClass intentClass=JClass(jni, "android/content/Intent"))
      if(JString urlStr=JString(jni, UnixPathUTF8(path)))
      if(JMethodID parse=jni->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;"))
      if(JObject uri=JObject(jni, jni->CallStaticObjectMethod(uriClass, parse, urlStr())))
      if(JMethodID intentCtor=jni->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;)V"))
      if(JFieldID actionViewField=jni->GetStaticFieldID(intentClass, "ACTION_VIEW", "Ljava/lang/String;"))
      if(JObject actionView=JObject(jni, jni->GetStaticObjectField(intentClass, actionViewField)))
      if(JObject intent=JObject(jni, jni->NewObject(intentClass, intentCtor, actionView())))
      if(JMethodID startActivity=jni->GetMethodID(ActivityClass, "startActivity", "(Landroid/content/Intent;)V"))
      {
         if(Starts(path, "file:"))
         {
            Str ext=GetExt(path), mime;
            if( ext.is())switch(ExtType(ext))
            {
               case EXT_TEXT : mime= "text/*"; break;
               case EXT_IMAGE: mime="image/*"; break;
               case EXT_VIDEO: mime="video/*"; break;
               case EXT_SOUND: mime="audio/*"; break;
            }
            if(mime.is())
            if(JMethodID setDataAndType=jni->GetMethodID(intentClass, "setDataAndType", "(Landroid/net/Uri;Ljava/lang/String;)Landroid/content/Intent;"))
            if(JString jmime=JString(jni, mime))
            {
               JObject temp=JObject(jni, jni->CallObjectMethod(intent, setDataAndType, uri(), jmime()));
                                         jni->CallVoidMethod(Activity, startActivity, intent());
               if(!jni->ExceptionCheck())return true; jni->ExceptionClear(); // Java exception "ActivityNotFound" can occur
            }
         }

         if(JMethodID setData=jni->GetMethodID(intentClass, "setData", "(Landroid/net/Uri;)Landroid/content/Intent;"))
         {
            JObject temp=JObject(jni, jni->CallObjectMethod(intent, setData, uri()));
                                      jni->CallVoidMethod(Activity, startActivity, intent());
            if(!jni->ExceptionCheck())return true; jni->ExceptionClear(); // Java exception "ActivityNotFound" can occur
         }
      }
   #elif WEB
      return JavaScriptRunI(S+"window.open(\""+CString(name)+"\")")!=0;
   #endif
      return false;
   }
   return true;
}
/******************************************************************************/
Bool OpenAppSettings()
{
#if IOS
   if(NSURL *url=[NSURL URLWithString:UIApplicationOpenSettingsURLString])
   {
      [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
    //NSURL from 'URLWithString' should not be released as it causes crash
      return true;
   }
#elif ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID openAppSettings=jni->GetStaticMethodID(ActivityClass, "openAppSettings", "()Z"))
         return jni->CallStaticBooleanMethod(ActivityClass, openAppSettings);
#endif
   return false;
}
/******************************************************************************/
#if ANDROID
static CChar8* AndroidPermissions[]=
{
   "android.permission.WRITE_EXTERNAL_STORAGE", // 0
   "android.permission.ACCESS_FINE_LOCATION"  , // 1
   "android.permission.RECORD_AUDIO"          , // 2
   "android.permission.GET_ACCOUNTS"          , // 3
};
ASSERT(PERMISSION_EXTERNAL_STORAGE==0 && PERMISSION_LOCATION==1 && PERMISSION_SOUND_RECORD==2 && PERMISSION_USER_NAME==3 && PERMISSION_NUM==4);
#endif
Bool HasPermission(PERMISSION permission)
{
#if ANDROID
   if(Activity && InRange(permission, AndroidPermissions))
   {
      JNI jni;
      if(jni)
      if(JClass ContextClass=JClass(jni, "android/content/Context"))
      if(JMethodID checkSelfPermission=jni->GetMethodID(ContextClass, "checkSelfPermission", "(Ljava/lang/String;)I"))
      if(JString t=JString(jni, AndroidPermissions[permission]))
         return jni->CallIntMethod(Activity, checkSelfPermission, t())==0; // PackageManager.PERMISSION_GRANTED=0
   }
   return true; // if permission checking not available, then assume we have it
#else
   return true;
#endif
}
void GetPermission(PERMISSION permission)
{
#if ANDROID
   if(InRange(permission, AndroidPermissions))
   {
      JNI jni;
      if(jni && Activity && ActivityClass)
      if(JMethodID requestPermissions=jni->GetMethodID(ActivityClass, "requestPermissions", "([Ljava/lang/String;I)V"))
      if(JObjectArray permissions=JObjectArray(jni, 1))
      {
         permissions.set(0, AndroidPermissions[permission]);
         jni->CallVoidMethod(Activity, requestPermissions, permissions(), jint(0));
      }
   }
#endif
}
static Int PermissionAsked; ASSERT(PERMISSION_NUM<=32); // each permission is stored in separate bit
void RequirePermission(PERMISSION permission)
{
   if(!HasPermission(permission) && !(AtomicOr(PermissionAsked, 1<<permission)&(1<<permission)))GetPermission(permission); // check if haven't asked yet, because on Android 6.0.1 Galaxy Note 4 if selecting "don't ask again" and then "Deny" then app will continuously try to re-initialize itself every frame and eventually crash
}
/******************************************************************************/
#if WEB
void    JavaScriptRun (CChar8 *code) {       emscripten_run_script       (code);}
Int     JavaScriptRunI(CChar8 *code) {return emscripten_run_script_int   (code);}
CChar8* JavaScriptRunS(CChar8 *code) {return emscripten_run_script_string(code);}
#else
void    JavaScriptRun (CChar8 *code) {}
Int     JavaScriptRunI(CChar8 *code) {return 0;}
CChar8* JavaScriptRunS(CChar8 *code) {return null;}
#endif
void    JavaScriptRun (CChar *code) {       JavaScriptRun (UTF8(code));}
Int     JavaScriptRunI(CChar *code) {return JavaScriptRunI(UTF8(code));}
CChar8* JavaScriptRunS(CChar *code) {return JavaScriptRunS(UTF8(code));}
/******************************************************************************/
Bool CreateShortcut(C Str &file, C Str &shortcut, C Str &desc, C Str &icon)
{
#if WINDOWS_OLD
   Bool ret=false;
   IShellLink *link; if(OK(CoCreateInstance(CLSID_ShellLink, null, CLSCTX_INPROC_SERVER, IID_IShellLink, (Ptr*)&link)))
   {
      IPersistFile *pf; if(OK(link->QueryInterface(IID_IPersistFile, (Ptr*)&pf)))
      {
         if(OK(link->SetPath            (               file  ))
         && OK(link->SetWorkingDirectory(WChar(_GetPath(file)))))
         {
                         link->SetDescription (desc);
            if(icon.is())link->SetIconLocation(icon, 0);
            ret=OK(pf->Save(shortcut+".lnk", TRUE));
         }
         pf->Release();
      }
      link->Release();
   }
   return ret;
#elif MAC
/*#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
   require "file://" prefix - which means full path is required
   Bool ok=false;
   if(NSURLAuto src =file)
   if(NSURLAuto dest=shortcut)
      if(NSData *bookmarkData=[src() bookmarkDataWithOptions:NSURLBookmarkCreationSuitableForBookmarkFile includingResourceValuesForKeys:nil relativeToURL:nil error:null])
   {
      ok=[NSURL writeBookmarkData:bookmarkData toURL:dest options:0 error:null];
      [bookmarkData release];
   }
   return ok;
#endif*/
   FSRef                src_ref; if(FSPathMakeRef((UInt8*)(UnixPathUTF8(file)()), &src_ref, null)!=noErr)return false;
   FSCatalogInfo   catalog_info; if(FSGetCatalogInfo(&src_ref, kFSCatInfoFinderInfo, &catalog_info, null, null, null)!=noErr)return false;
   AppleFileInfo     *file_info=(AppleFileInfo*)(&catalog_info.finderInfo);
   OSType             file_type=file_info->fileType,
   creator_type=file_info->fileCreator;
   IconRef             icon_ref; GetIconRefFromFileInfo(&src_ref, 0, null, kFSCatInfoFinderInfo, &catalog_info, kIconServicesNormalUsageFlag, &icon_ref, null);
   IconFamilyHandle icon_family; IconRefToIconFamily(icon_ref, kSelectorAllAvailableData, &icon_family);
   AliasHandle            alias; FSNewAliasMinimal(&src_ref, &alias); if(!alias)return false;
   FSRef             parent_ref; if(FSPathMakeRef((UInt8*)(UnixPathUTF8(GetPath(shortcut))()), &parent_ref, null)!=noErr)return false;
   Char         alias_name[256]; Set(alias_name, GetBase(shortcut));
   FSRef              alias_ref; FSCreateResFile(&parent_ref, Length(alias_name), (UniChar*)alias_name, 0, null, &alias_ref, null); if(!FSIsFSRefValid(&alias_ref))return false;

   // create resource
   ResFileRefNum file_ref=FSOpenResFile(&alias_ref, fsRdWrPerm);
   UseResFile  (file_ref);
   AddResource ((Handle)alias, 'alis', 0, null);
   AddResource ((Handle)icon_family, kIconFamilyType, kCustomIconResource, "\p");
   CloseResFile(file_ref);

   // adjust alias
   if(FSGetCatalogInfo(&alias_ref, kFSCatInfoFinderInfo, &catalog_info, null, null, null)!=noErr)return false;
   file_info=(AppleFileInfo*)(&catalog_info.finderInfo);
   file_info->finderFlags|= kIsAlias|kHasCustomIcon;
   file_info->finderFlags&=~kHasBeenInited;
   file_info->fileType    = file_type;
   file_info->fileCreator = creator_type;
   if(FSSetCatalogInfo(&alias_ref, kFSCatInfoFinderInfo, &catalog_info)!=noErr)return false;

   return true;
#elif LINUX
   FileText ft; if(ft.write(shortcut+".desktop", UTF_8_NAKED)) // Linux will fail if BOM is present
   {
      ft.putLine("[Desktop Entry]");
      ft.putLine("Encoding=UTF-8");
      ft.putLine("Type=Application");
      ft.putLine("Terminal=false");
      ft.putLine(S+"Name="+GetBaseNoExt(shortcut));
      if(desc.is())ft.putLine(S+"GenericName="+         Replace(desc, '\n', '\0')     );
      if(icon.is())ft.putLine(S+"Icon=\""     +UnixPath(Replace(icon, '\n', '\0'))+'"');
      if(file.is())ft.putLine(S+"Exec=\""     +UnixPath(Replace(file, '\n', '\0'))+'"');
      return true;
   }
#endif
	return false;
}
/******************************************************************************/
Bool AssociateFileType(Str extension, Str application_path, Str application_id, Str extension_desc, Str custom_icon)
{
   Bool ok=false;
   if(extension[0]=='.')extension.remove(0); // ".ext" -> "ext"
   if(extension.is() && application_path.is() && application_id.is() && application_id[0]!='.')
   {
   #if WINDOWS_OLD
      extension       =S+'.'+extension;
      application_id +=extension;
      application_path=S+'"'+Replace(application_path, '/', '\\')+"\" \"%1\"";
      custom_icon     =Replace(custom_icon, '/', '\\');

      ok=true;
      Bool changed=false;
      Str  name=S+"Software/Classes/"+application_id+"/shell/open/command/"; if(!Equal(GetRegStr(RKG_LOCAL_MACHINE, name), application_path, true))if(SetRegStr(RKG_LOCAL_MACHINE, name, application_path))changed=true;else ok=false;
           name=S+"Software/Classes/"+extension     +"/"                   ; if(!Equal(GetRegStr(RKG_LOCAL_MACHINE, name), application_id  , true))if(SetRegStr(RKG_LOCAL_MACHINE, name, application_id  ))changed=true;else ok=false;
           name=S+"Software/Classes/"+application_id+"/"                   ; if(!Equal(GetRegStr(RKG_LOCAL_MACHINE, name), extension_desc  , true))if(SetRegStr(RKG_LOCAL_MACHINE, name, extension_desc  ))changed=true; // optional
           name=S+"Software/Classes/"+application_id+"/DefaultIcon/"       ; if(!Equal(GetRegStr(RKG_LOCAL_MACHINE, name), custom_icon     , true))if(SetRegStr(RKG_LOCAL_MACHINE, name, custom_icon     ))changed=true; // optional
    /*if(ok) // some more advanced stuff for vista and newer
      {
         IApplicationAssociationRegistration *aar=null;
         CoCreateInstance(CLSID_ApplicationAssociationRegistration, null, CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistration), (Ptr*)&aar);
         if(aar)
         {
            ok=OK(aar->SetAppAsDefault(application_id, extension, AT_FILEEXTENSION));
            aar->Release();
         }
      }*/

      if(ok && changed)SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, null, null);
   #endif
   }
   return ok;
}
/******************************************************************************/
EXTENSION_TYPE ExtType(C Str &ext)
{
   if(ext=="txt" || ext=="xml" || ext=="htm"  || ext=="html" || ext=="php" || ext=="cpp"  || ext=="c"    || ext=="h"    || ext=="java" || ext=="cs"  || ext=="m"   || ext=="mm"   || ext=="cxx"  || ext=="cc"   || ext=="mk"    )return EXT_TEXT;
   if(ext=="bmp" || ext=="jpg" || ext=="jpeg" || ext=="png"  || ext=="tga" || ext=="tif"  || ext=="tiff" || ext=="dds"  || ext=="psd"  || ext=="gif" || ext=="ico" || ext=="cur"  || ext=="icns" || ext=="webp" || ext=="bpg"   )return EXT_IMAGE;
   if(ext=="mp3" || ext=="ogg" || ext=="wma"  || ext=="wav"  || ext=="m4a" || ext=="flac" || ext=="opus" || ext=="weba"                                                                                                         )return EXT_SOUND;
   if(ext=="3ds" || ext=="ase" || ext=="obj"  || ext=="ms3d" || ext=="b3d" || ext=="dae"  || ext=="fbx"  || ext=="psk"  || ext=="psa"  || ext=="bvh"                                                                            )return EXT_MESH;
   if(ext=="avi" || ext=="mpg" || ext=="mpeg" || ext=="mp4"  || ext=="m4v" || ext=="mkv"  || ext=="wmv"  || ext=="rmvb" || ext=="divx" || ext=="ogm" || ext=="ogv" || ext=="webm" || ext=="vob"  || ext=="flv"  || ext=="theora")return EXT_VIDEO;

   // Esenthel formats
   if(ext=="img" )return EXT_IMAGE;
   if(ext=="mesh")return EXT_MESH;
 //if(ext=="mtrl")return EXT_MATERIAL;
 //if(ext=="phys")return EXT_PHYS;

   return EXT_NONE;
}
/******************************************************************************/
Bool HasDrive(CChar *path)
{
   if(path)
   {
      if(((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))  &&  path[1]==':')return true; // C: Windows Style
      if(IsSlash(path[0]))return true; // / Unix style
   }
   return false;
}
Bool HasDrive(CChar8 *path)
{
   if(path)
   {
      if(((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))  &&  path[1]==':')return true; // C: Windows Style
      if(IsSlash(path[0]))return true; // / Unix style
   }
   return false;
}

Bool IsDrive(CChar *path)
{
   if(path)
   {
      if(((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))  &&  path[1]==':'  &&  (!path[2] || (IsSlash(path[2]) && !path[3])))return true; // C: or C:\ Windows Style
      if(IsSlash(path[0])) // on Unix we can also have "/Volumes/.."
      {
         if(!path[1])return true;
         if(Starts(path+1, "Volumes") && IsSlash(path[8]) && (path[9] && !IsSlash(path[9])))for(path+=10; ; )
         {
            Char c=*path++;
            if( !c       )return true;
            if(IsSlash(c))return (*path)==0;
         }
      }
   }
   return false;
}
Bool IsDrive(CChar8 *path)
{
   if(path)
   {
      if(((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))  &&  path[1]==':'  &&  (!path[2] || (IsSlash(path[2]) && !path[3])))return true; // C: or C:\ Windows Style
      if(IsSlash(path[0])) // on Unix we can also have "/Volumes/.."
      {
         if(!path[1])return true;
         if(Starts(path+1, "Volumes") && IsSlash(path[8]) && (path[9] && !IsSlash(path[9])))for(path+=10; ; )
         {
            Char c=*path++;
            if( !c       )return true;
            if(IsSlash(c))return (*path)==0;
         }
      }
   }
   return false;
}

Bool HasRemote(CChar  *path) {return path && path[0]=='\\' && path[1]=='\\';}
Bool HasRemote(CChar8 *path) {return path && path[0]=='\\' && path[1]=='\\';}
Bool FullPath (CChar  *path) {return HasDrive(path) || HasRemote(path);}
Bool FullPath (CChar8 *path) {return HasDrive(path) || HasRemote(path);}
/******************************************************************************/
// GET
/******************************************************************************/
// TODO: all of these impose limits which could break some functionalities requiring very long strings
CChar* _GetBase(CChar *name, Bool tail_slash, Char (&dest)[MAX_LONG_PATH])
{
   if(name)
   {
      if(IsDrive(name))
      {
         if(!tail_slash) // if want to remove tail slash
         {
            if(Int length=Length(name))if(IsSlash(name[length-1]))
            {
               if(length=SetReturnLength(dest, name))if(IsSlash(dest[length-1]))dest[--length]=0;
               return dest;
            }
         }
         return name;
      }
      if(Int length=Length(name))
      {
         Bool ends_with_slash=IsSlash(name[length-1]); if(ends_with_slash)length--;
         for(; length--; )if(IsSlash(name[length]))break;
         if(!tail_slash && ends_with_slash) // if want to remove tail slash and ends with one then we need to copy to 'dest'
         {
            if(length=SetReturnLength(dest, name+length+1))if(IsSlash(dest[length-1]))dest[--length]=0;
            return dest;
         }else
         {
            return name+length+1;
         }
      }
   }
   return null;
}
/******************************************************************************/
CChar* _GetBaseNoExt(CChar *name, Char (&dest)[MAX_LONG_PATH])
{
   if(name)
   {
      if(IsDrive(name))
      {
         if(Int length=Length(name))if(IsSlash(name[length-1]))
         {
            if(length=SetReturnLength(dest, name))if(IsSlash(dest[length-1]))dest[--length]=0;
            return dest;
         }
         return name;
      }
      if(Int length=Length(name))
      {
         Bool ends_with_slash=IsSlash(name[length-1]); if(ends_with_slash)length--;
         for(; length--; )if(IsSlash(name[length]))break;
         if(length=SetReturnLength(dest, name+length+1))
         {
            if(IsSlash(dest[length-1]))dest[--length]=0;
            for(; length--; )if(dest[length]=='.'){dest[length]=0; break;}
         }
         return dest;
      }
   }
   return null;
}
/******************************************************************************/
CChar* _GetExt(CChar *name, Char (&dest)[MAX_LONG_PATH])
{
   if(Int length=Length(name))
   {
      if(IsSlash(name[length-1])) // if ends with slash then we need to copy to 'dest'
      {
         for(length--; length--; )
         {
            Char c=name[length];
            if(IsSlash(c))break;
            if(c=='.')
            {
               if(length=SetReturnLength(dest, name+length+1))if(IsSlash(dest[length-1]))dest[--length]=0;
               return dest;
            }
         }
      }else for(; length--; )
      {
         Char c=name[length];
         if(IsSlash(c))break;
         if(c=='.')return name+length+1;
      }
   }
   return null;
}
/******************************************************************************/
CChar* _GetExtNot(CChar *name, Char (&dest)[MAX_LONG_PATH])
{
   if(Int length=SetReturnLength(dest, name))
   {
      if(IsSlash(dest[length-1]))dest[--length]=0;
      for(; length--; )
      {
         Char c=dest[length];
         if(IsSlash(c))break;
         if(c=='.'){dest[length]=0; break;}
      }
   }
   return dest;
}
/******************************************************************************/
CChar* _GetPath(CChar *name, Int tail_slash, Char (&dest)[MAX_LONG_PATH])
{
   if(!IsDrive(name))
      if(Int length=SetReturnLength(dest, name))
   {
      Bool   ends_with_slash=IsSlash(dest[length-1]); if(ends_with_slash)length--;
      Bool insert_tail_slash=((tail_slash<0) ? ends_with_slash : tail_slash!=0);
      for(; length--; )if(IsSlash(dest[length]))
      {
         if(length<=1 && HasRemote(dest)) // reached remote "\\*" or "\\"
         {
            if(!dest[2])break; // if there's nothing after remote, then always return empty string
            dest[2]=0; // return "\\"
         }else
         if(length)dest[length+insert_tail_slash]=0;else dest[1]=0;
         return dest;
      }
   }
   dest[0]=0; return dest;
}
/******************************************************************************/
Char8* _GetStart(CChar8 *name, Char8 (&dest)[MAX_LONG_PATH]) // this function always returns 'dest' so we can use 'Char8' instead of 'CChar8' in case we want to modify it
{
   Int i=0;
   if(Is(name))for(; i<Elms(dest)-1; i++)
   {
      Char8 c=*name++; if(!c || IsSlash(c))break;
      dest[i]=c;
   }
   dest[i]=0; return dest;
}
Char* _GetStart(CChar *name, Char (&dest)[MAX_LONG_PATH]) // this function always returns 'dest' so we can use 'Char8' instead of 'CChar8' in case we want to modify it
{
   Int i=0;
   if(Is(name))for(; i<Elms(dest)-1; i++)
   {
      Char c=*name++; if(!c || IsSlash(c))break;
      dest[i]=c;
   }
   dest[i]=0; return dest;
}
/******************************************************************************/
CChar* _GetStartNot(CChar *name)
{
   if(Is(name))for(;;)
   {
      Char c=*name++; if(!c)return null;
      if(IsSlash(c))return name;
   }
   return null;
}
/******************************************************************************/
Str GetBase     (C Str &name) {return _GetBase     (name);}
Str GetBaseNoExt(C Str &name) {return _GetBaseNoExt(name);}
Str GetExt      (C Str &name) {return _GetExt      (name);}
Str GetExtNot   (C Str &name) {return _GetExtNot   (name);}
Str GetPath     (C Str &name) {return _GetPath     (name);}
Str GetStart    (C Str &name) {return _GetStart    (name);}
Str GetStartNot (C Str &name) {return _GetStartNot (name);}
/******************************************************************************/
Str GetRelativePath(Str src, Str dest)
{
   if(FullPath(dest) || FullPath(src)) // if any path is full path
      if(GetStart(dest)!=GetStart(src)) // not on the same drive
         return dest; // need to return full path

   // eat common path
   for(;;)
   {
      Str start=GetStart(dest);
      if( start.length()==dest.length() || start!=GetStart(src))break;
      dest=GetStartNot(dest);
      src =GetStartNot(src );
   }

   for(; src.is(); )
   {
      src =GetPath(src);
      dest.insert(0, "..\\"); // dest=S+"..\\"+dest;
   }
   return dest;
}
/******************************************************************************/
#if WINDOWS_OLD
static const KNOWNFOLDERID FOLDERID_OneDrive  ={0xA52BBA46, 0xE9E1, 0x435f, {0xB3, 0xD9, 0x28, 0xDA, 0xA6, 0x48, 0xC0, 0xF6}},
                           FOLDERID_SavedGames={0x4C5C32FF, 0xBB9D, 0x43b0, {0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4}};

static Str GetKnownFolderPath(C KNOWNFOLDERID &folder_id)
{
#if SUPPORT_WINDOWS_XP
   #define KF_FLAG_CREATE    0x00008000
   #define KF_FLAG_NO_ALIAS  0x00001000
   if(HMODULE shell=GetModuleHandle(L"Shell32.dll"))
   if(HRESULT (STDAPICALLTYPE *SHGetKnownFolderPath)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath)=(decltype(SHGetKnownFolderPath))GetProcAddress(shell, "SHGetKnownFolderPath")) // available on Vista+
#endif
   {
      ASSERT( KF_FLAG_CREATE  ==0x00008000
           && KF_FLAG_NO_ALIAS==0x00001000);
      PWSTR sg_path=null; SHGetKnownFolderPath(folder_id, KF_FLAG_CREATE|KF_FLAG_NO_ALIAS, null, &sg_path);
      if(   sg_path)
      {
         Str    path=sg_path; CoTaskMemFree(sg_path);
         return path;
      }
   }
   return S;
}
#endif
Str SystemPath(SYSTEM_PATH type)
{
#if WINDOWS_OLD
   wchar_t path[MAX_PATH]; path[0]=0; // 'SHGetFolderPath' requires MAX_PATH
   UInt out;
   switch(type)
   {
      case SP_ONE_DRIVE: return GetKnownFolderPath(FOLDERID_OneDrive);

      case SP_SAVED_GAMES:
      {
         Str path=GetKnownFolderPath(FOLDERID_SavedGames); if(path.is())return path; // if unknown then fallback to SP_DOCUMENTS
      } // !! no break on purpose !!
      case SP_DOCUMENTS      : out=CSIDL_MYDOCUMENTS     ; break;

      case SP_DESKTOP        : out=CSIDL_DESKTOPDIRECTORY; break;
      case SP_PROG_FILES     : out=CSIDL_PROGRAM_FILES   ; break;
      case SP_MENU           : out=CSIDL_STARTMENU       ; break;
      case SP_MENU_PROG      : out=CSIDL_PROGRAMS        ; break;
      case SP_STARTUP        : out=CSIDL_STARTUP         ; break;
      case SP_APP_DATA       : out=CSIDL_APPDATA         ; break;
      case SP_APP_DATA_PUBLIC: out=CSIDL_APPDATA         ; break;
      case SP_ALL_APP_DATA   : out=CSIDL_COMMON_APPDATA  ; break;
      case SP_FAVORITES      : out=CSIDL_FAVORITES       ; break;
      case SP_SYSTEM         : out=CSIDL_SYSTEM          ; break;
      default                : return S;
   }
   SHGetFolderPath(0, out, 0, 0, path); ASSERT(Elms(path)==MAX_PATH); // !! SHGetFolderPath requires MAX_PATH !!
   if(type==SP_SAVED_GAMES && path[0])Append(WChar(path), "\\My Games", Elms(path));
   return path;
#elif WINDOWS_NEW
   Windows::Storage::StorageFolder ^folder;
   switch(type)
   {
      case SP_APP_DATA       :
      case SP_APP_DATA_PUBLIC:
      case SP_SAVED_GAMES    : folder=Windows::Storage::ApplicationData::Current->LocalFolder; break;

      case SP_DOCUMENTS: folder=Windows::Storage::KnownFolders::DocumentsLibrary; break;
   }
   if(folder)return folder->Path->Data();
#elif LINUX
   Char8 path[MAX_UTF_PATH]; path[0]=0;
   switch(type)
   {
      case SP_DESKTOP        : MergePath(path, getenv("HOME"), "Desktop"  ); break;
      case SP_DOCUMENTS      : MergePath(path, getenv("HOME"), "Documents"); break;
      case SP_SAVED_GAMES    :
      case SP_APP_DATA       :
      case SP_APP_DATA_PUBLIC: Set(path, getenv("HOME")); break;
      case SP_ALL_APP_DATA   : return "/home";
      case SP_TRASH:
      {
         if(char *XDG_DATA_HOME=getenv("XDG_DATA_HOME"))MergePath(path, XDG_DATA_HOME, "Trash");else
         if(char *         HOME=getenv(         "HOME"))MergePath(path,          HOME, ".local\\share\\Trash");
      }break;
   }
   return Replace(FromUTF8(path), '/', '\\');
#elif MAC
   OSType out;
   switch(type)
   {
      case SP_DESKTOP        : out=kDesktopFolderType           ; break;
      case SP_PROG_FILES     : out=kApplicationsFolderType      ; break;
      case SP_SAVED_GAMES    : out=kApplicationSupportFolderType; break;
      case SP_APP_DATA       : out=kApplicationSupportFolderType; break;
      case SP_APP_DATA_PUBLIC: out=kApplicationSupportFolderType; break;
      case SP_DOCUMENTS      : out=kDocumentsFolderType         ; break;
      case SP_FAVORITES      : out=kFavoritesFolderType         ; break;
      case SP_FRAMEWORKS     : out=kFrameworksFolderType        ; break;
      default                : return S;
   }
   Char8 path[MAX_UTF_PATH]; path[0]=0;
   FSRef fs;
   if(FSFindFolder(kUserDomain, out, true, &fs)==noErr)
      if(CFURLRef url=CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &fs))
   {
      CFURLGetFileSystemRepresentation(url, true, (UInt8*)path, Elms(path));
      CFRelease(url);
   }
   return Replace(FromUTF8(path), '/', '\\');
#elif IOS
	NSSearchPathDirectory out;
   switch(type)
   {
      case SP_DESKTOP        : out=NSDesktopDirectory    ; break;
      case SP_DOCUMENTS      : out=NSDocumentDirectory   ; break;
      case SP_SAVED_GAMES    : out=NSDocumentDirectory   ; break;
      case SP_APP_DATA       : out=NSDocumentDirectory   ; break;
      case SP_APP_DATA_PUBLIC: out=NSDocumentDirectory   ; break;
      case SP_PROG_FILES     : out=NSApplicationDirectory; break;
      default                : return S;
   }
   if(NSFileManager *fileManager=[NSFileManager defaultManager])
      if(NSArray *paths=NSSearchPathForDirectoriesInDomains(out, NSUserDomainMask, YES))
         if([paths count])return Replace([paths objectAtIndex:0], '/', '\\'); // don't release anything as it causes bad access
#elif ANDROID
   switch(type)
   {
      case SP_DOCUMENTS      : return AndroidAppDataPath;
      case SP_SAVED_GAMES    : return AndroidAppDataPublicPath.is() ? AndroidAppDataPublicPath : AndroidAppDataPath;
      case SP_APP_DATA       : return AndroidAppDataPath;
      case SP_APP_DATA_PUBLIC: return AndroidAppDataPublicPath;
      case SP_PUBLIC         : return AndroidPublicPath;
      case SP_SD_CARD        : return AndroidSDCardPath;
   }
#elif WEB
   switch(type)
   {
      case SP_DOCUMENTS      :
      case SP_SAVED_GAMES    :
      case SP_APP_DATA       :
      case SP_APP_DATA_PUBLIC: return "/data";
   }
#endif
   return S;
}
/******************************************************************************/
Str AndroidExpansionFileName(Int version, Bool main)
{
#if ANDROID
   if(AndroidPublicPath.is() && AndroidPackageName.is())
      return AndroidPublicPath+"/Android/obb/"+AndroidPackageName+'/'+(main ? "main" : "patch")+'.'+version+'.'+AndroidPackageName+".obb";
#endif
   return S;
}
/******************************************************************************/
Str NormalizePath(C Str &path)
{
   Str s;
   if(CChar *p=path)for(; p[0]; )
   {
      if(p[0]=='.' && p[1]=='.' && (IsSlash(p[2]) || p[2]=='\0') // if starts from going back
      && s.length()>=2) // is not Unix root "/../" and is not empty "../"
      {
         s.removeLast(); // remove last '/' or '\'
         for(; s.is() && !IsSlash(s.last()); )s.removeLast(); // remove last path part
         p+=2; if(*p)p++; // skip .. and / if there's one
      }else
      for(;;) // default name
      {
         Char c=*p++; if(!c        )goto end;
           s+=c;      if(IsSlash(c))break;
      }
   }
end:;
   return s;
}
/******************************************************************************/
Str CleanFileName(C Str &name)
{
   Str temp; temp.reserve(name.length());
   FREPA(name)switch(Char c=name[i]) // skip disallowed characters
   {
      case '/' :
      case '\\':
      case ':' :
      case '*' :
      case '?' :
      case '"' :
      case '<' :
      case '>' :
      case '|' : break;
      default  : if(U16(c)>31)temp+=c; break;
   }
   temp.removeOuterWhiteChars(); // on Windows names can't end with spaces (but they can start with spaces), on Windows names can start with spaces (but remove them anyway, because it's a non-standard naming convention to start with a space)
   REPA(temp)if(temp[i]!='.' && temp[i]!=' ')return temp; // on Windows name must contain at least one character which is not space or dot (".", "..", ". .", "......", ". . ." - all of these are invalid names)
   return S;
}
/******************************************************************************/
// DO NOT CHANGE !!
static const     Char8 CleanFileNameArray[]={'!', '#', '^', '-', '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
static const     Byte  CleanFileNameIndex[]={255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 3, 255, 255, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255, 255, 255, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 255, 255, 255, 2, 4, 255, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}; ASSERT(Elms(CleanFileNameIndex)==256);
static constexpr Int   CleanFileNameElms   =Elms(CleanFileNameArray);
/******************************************************************************
static Int CompareChar(C Char8 &a, C Char8 &b) {return EE::Compare(a, b);}
void MakeCleanFileNameArray()
{
   Str s;
#if 1 // 41 chars, this is enough to store 6-chars per 4-bytes
   s+="!#^-_";
   for(Int i='a'; i<='z'; i++)s+=Char8(i);
   for(Int i='0'; i<='9'; i++)s+=Char8(i);
#else // 57 chars
   for(Int i='a'; i<='z'; i++)s+=Char8(i);
   for(Int i='0'; i<='9'; i++)s+=Char8(i);
   s+=" `~!@#$%^&*()_-+=[]{};'\\:\"|,./<>?";
   s.replace(' ', '\0'); // skip space
   s.replace('.', '\0'); // skip . to avoid setting extensions
   s.replace('`', '\0'); // skip ` because it's easy to mix up with '
#endif
   s=CleanFileName(s);
   Memc<Char8> cs; FREPA(s)cs.include(s[i]); cs.sort(CompareChar);
   s.clear(); FREPA(cs){if(s.is())s+=", "; s+='\''; if(cs[i]=='\'' || cs[i]=='\\')s+='\\'; s+=cs[i]; s+='\'';}
   s.line();
   for(Int i=0; i<256; i++){if(s.is() && s.last()!='\n')s+=", "; s+=Byte(cs.find(CaseDown(Char8(i))));} // use Byte cast so -1 -> 255
   s.line();
   ClipSet(s);
}
void Test()
{
   REPA(CleanFileNameArray)DEBUG_ASSERT(CleanFileNameIndex[(Byte)CleanFileNameArray[i]]==i, "err");
}
/******************************************************************************/
CChar* _EncodeFileName(C UID &id, Char (&name)[24+1]) // 24 chars needed + nul terminate
{
   Int  n=0;
C UInt *src=id.i;
   REP(SIZE(id)/4)
   {
      ASSERT(CleanFileNameElms==41); // this requires 6 steps per UInt
      UInt u=*src++; REPD(step, 6)
      {
         name[n++]=CleanFileNameArray[u%CleanFileNameElms]; u/=CleanFileNameElms;
      }
   }
   name[n]='\0';
   return name;
}
Bool DecodeFileName(CChar *src, UID &id)
{
   if(src)
   {
      UInt *d=id.i; REP(SIZE(id)/4)
      {
         ASSERT(CleanFileNameElms==41); // this requires 6 steps per UInt
         UInt u=0, mul=1; REPD(step, 6)
         {
            UShort c=*src++               ; if(!InRange(c, CleanFileNameIndex)){invalid_char: *d++=u; for(; --i>=0; )*d++=0; return false;} // clear remaining data
            Byte   v=CleanFileNameIndex[c]; if(!InRange(v, CleanFileNameElms ))goto invalid_char;
            u+=v*mul; mul*=CleanFileNameElms;
         }
         *d++=u;
      }
      if(!*src)return true;
   }else id.zero();
   return false;
}
/******************************************************************************/
Str  EncodeFileName(           CPtr src, Int size) {Str dest; EncodeFileName(dest, src, size); return dest;}
void EncodeFileName(Str &dest, CPtr src, Int size)
{
   dest.clear();
   if(Byte *s=(Byte*)src)if(size>0)
   {
      dest.reserve(DivCeil4(size)*6); // 6 chars per 4 bytes
      UInt u, max;
      REP(UInt(size)/4)
      {
         for(u=*(UInt*)s, max=UINT_MAX; ; ){dest.alwaysAppend(CleanFileNameArray[u%CleanFileNameElms]); max/=CleanFileNameElms; if(!max)break; u/=CleanFileNameElms;}
         s+=4;
      }
      switch(size&3)
      {
         case  1: u=*      s           ; max=0x0000FF; break;
         case  2: u=*(U16*)s           ; max=0x00FFFF; break;
         case  3: u=*(U16*)s|(s[2]<<16); max=0xFFFFFF; break;
         default: goto end;
      }
      for(;;){dest.alwaysAppend(CleanFileNameArray[u%CleanFileNameElms]); max/=CleanFileNameElms; if(!max)break; u/=CleanFileNameElms;}
   end:;
   }
}
Bool DecodeFileName(C Str &src, Ptr dest, Int size)
{
   if(Byte *d=(Byte*)dest)if(size>0)
   {
      UInt src_pos=0, u, max, mul;
      for(; size>=4; )
      {
         u=0; max=UINT_MAX; mul=1; for(;;)
         {
            U16  c=       src[src_pos++]; if(!InRange(c, CleanFileNameIndex)){invalid_char: *(UInt*)d=u; d+=4; size-=4; REP(size)*d++=0; return false;}
            Byte i=CleanFileNameIndex[c]; if(!InRange(i, CleanFileNameElms ))goto invalid_char;
            u+=i*mul; max/=CleanFileNameElms; if(!max)break; mul*=CleanFileNameElms;
         }
         *(UInt*)d=u; d+=4; size-=4;
      }
      Bool error=false; switch(size)
      {
         case  1: max=0x0000FF; break;
         case  2: max=0x00FFFF; break;
         case  3: max=0xFFFFFF; break;
         default: goto end;
      }
      u=0; mul=1; for(;;)
      {
         U16  c=       src[src_pos++]; if(!InRange(c, CleanFileNameIndex)){error=true; break;}
         Byte i=CleanFileNameIndex[c]; if(!InRange(i, CleanFileNameElms )){error=true; break;}
         u+=i*mul; max/=CleanFileNameElms; if(!max)break; mul*=CleanFileNameElms;
      }
      switch(size)
      {
         case 1: *      d=u       ;               break;
         case 2: *(U16*)d=u       ;               break;
         case 3: *(U16*)d=u&0xFFFF; d[2]=(u>>16); break;
      }
   end:;
      if(error || src.length()!=src_pos)return false; // if there are still characters left then it means that the string is bigger than expected, leave contents on fail
   }
   return true;
}
/******************************************************************************/
Str  EncodeRaw(            CPtr src, Int size) {Str dest; EncodeRaw(dest, src, size); return dest;}
void EncodeRaw(Str8 &dest, CPtr src, Int size)
{
   dest.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   if(Byte *s=(Byte*)src)if(size>0)
   {
      dest.reserve(size);
      CopyFast(dest._d.data(), src, size);
               dest._d[dest._length=size]='\0'; // set ending zero
   }
}
void EncodeRaw(Str &dest, CPtr src, Int size)
{
   dest.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
   if(Byte *s=(Byte*)src)if(size>0)
   {
      Int length=Ceil2(size)/2; // align to full words
      dest.reserve(length);
               CopyFast(dest._d.data(), src, size);
      if(size&1)((Byte*)dest._d.data())[size]=0; // Char can hold 2 bytes, but if we don't set that Char fully from source, then clear last byte to zero, yes this should be "[size]" and not "[size-1]" (because we've copied 'size' bytes but still have 1 left in last Char)
                        dest._d[dest._length=length]='\0'; // set ending zero
   }
}
Bool DecodeRaw(C Str8 &src, Ptr dest, Int size)
{
   if(src.length()!=size)return false;
   if(Byte *d=(Byte*)dest)CopyFast(dest, src(), size);
   return true;
}
Bool DecodeRaw(C Str &src, Ptr dest, Int size)
{
   Int length=Ceil2(size)/2; // align to full words
   if(src.length()!=length)return false;
   if(Byte *d=(Byte*)dest)CopyFast(dest, src(), size);
   return true;
}
/******************************************************************************/
Bool ValidEmail(C Str &email) // "user@domain.xxx"
{
   if(    email.length() >=5  // 1@3.5
   && U16(email.first ())>32  // not space, tab, new line, ..
   && U16(email.last  ())>32) // not space, tab, new line, ..
   {
      Int at =TextPosI(email, '@');
      if( at>=1 && TextPosI(email()+at+1, '@')<0) // if found '@' at 1 or higher position and there's no other '@' after
      {
         Int dot =TextPosI(email()+at+1, '.');
         if( dot>=1) // if found '.' at 1 or higher position
            if(email.length()>at+2+dot) // if there's something after the dot
               return true;
      }
   }
   return false;
}
Bool ValidURL(C Str &url) // "http://domain.com"
{
   if(StartsPath(url, "http://")
   || StartsPath(url, "https://"))
   {
      Int domain =TextPosI(url, ':');
      if( domain>=0)
      {
         domain+=3; // skip "://", in above case to position "domain.com"
         Int dot =TextPosI(url()+domain, '.');
         if( dot>=1) // if found '.' at 1 or higher position
            if(url.length()>domain+dot+1) // if there's something after the dot
               return true;
      }
   }
   return false;
}
Bool ValidLicenseKey(C Str &key) // "XXXXX-XXXXX-XXXXX-XXXXX-XXXXX" 5*5X + 4*-
{
   if(key.length()!=5*5+4)return false;
   if(key[5]!='-' || key[11]!='-' || key[17]!='-' || key[23]!='-')return false;
   REP(5)
   {
      Int offset=i*6;
      REP(5)
      {
         Char c=key[offset+i];
         if(!((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')))return false;
      }
   }
   return true;
}
/******************************************************************************/
UID DeviceID(Bool per_user)
{
   if(!D.deviceName().is()) // display device name may be not available yet
   {
      SafeSyncLocker locker(D._lock);
   #if WINDOWS_OLD
      if(IDirect3D9 *d3d=Direct3DCreate9(D3D_SDK_VERSION))
      {
         D3DADAPTER_IDENTIFIER9 id; if(OK(d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id)))D._device_name=id.Description;
         RELEASE(d3d);
      }
   #elif WINDOWS_NEW
      IDXGIFactory1 *factory=null; CreateDXGIFactory1(__uuidof(IDXGIFactory1), (Ptr*)&factory); if(factory)
      {
         IDXGIAdapter *adapter=null; factory->EnumAdapters(0, &adapter); if(adapter)
         {
            DXGI_ADAPTER_DESC desc; if(OK(adapter->GetDesc(&desc)))D._device_name=desc.Description;
            adapter->Release();
         }
         factory->Release();
      }
   #elif APPLE // Mac/iOS already have a context
      D._device_name=(CChar8*)glGetString(GL_RENDERER);
   #elif LINUX
      int attribs[]=
      {
         GLX_X_RENDERABLE , true,
         GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
         GLX_RENDER_TYPE  , GLX_RGBA_BIT,
         GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
       //GLX_RED_SIZE     , 8,
       //GLX_GREEN_SIZE   , 8,
       //GLX_BLUE_SIZE    , 8,
       //GLX_ALPHA_SIZE   , 8,
       //GLX_DEPTH_SIZE   , 24,
       //GLX_STENCIL_SIZE , 8,
       //GLX_DOUBLEBUFFER , true,
         NULL
      };
      int count=0; if(GLXFBConfig *fbc=glXChooseFBConfig(XDisplay, DefaultScreen(XDisplay), attribs, &count))
      {
         if(count>=1 && fbc[0])
            if(GLXContext context=glXCreateNewContext(XDisplay, fbc[0], GLX_RGBA_TYPE, null, true))
         {
            XSync(XDisplay, false); // Forcibly wait on any resulting X errors
            if(glXMakeCurrent(XDisplay, NULL, context))
            {
               D._device_name=(CChar8*)glGetString(GL_RENDERER); for(; D._device_name.last()==' '; )D._device_name.removeLast(); // Linux may have unnecessary spaces at the end
               glXMakeCurrent(XDisplay, NULL, null);
            }
            glXDestroyContext(XDisplay, context);
         }
         XFree(fbc);
      }
   #elif ANDROID
      if(EGLDisplay display=eglGetDisplay(EGL_DEFAULT_DISPLAY))
      {
         if(eglInitialize(display, null, null)==EGL_TRUE)
         {
            EGLint attribs[]=
            {
               EGL_SURFACE_TYPE   , EGL_WINDOW_BIT,
               EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
               EGL_NONE
            };
            EGLConfig config=0;
            EGLint    num_configs=0;
            if(eglChooseConfig(display, attribs, &config, 1, &num_configs)==EGL_TRUE)
               if(num_configs>=1)
                  if(EGLSurface surface=eglCreateWindowSurface(display, config, AndroidApp->window, null))
            {
               EGLint ctx_attribs[]=
               {
                  EGL_CONTEXT_CLIENT_VERSION, 2, // try OpenGL ES 2.0 context
                  EGL_NONE
               };
               if(EGLContext context=eglCreateContext(display, config, null, ctx_attribs))
               {
                  if(eglMakeCurrent(display, surface, surface, context)==EGL_TRUE)
                  {
                     D._device_name=(CChar8*)glGetString(GL_RENDERER);
                     eglMakeCurrent(display, null, null, null);
                  }
                  eglDestroyContext(display, context);
               }
               eglDestroySurface(display, surface);
            }
         }
         eglTerminate(display);
      }
   #endif
      if(!D.deviceName().is())D._device_name="None"; // to avoid calling above everytime
   }

   Str id=S
     +Cpu.name()+'\n'
     +D.deviceName()+'\n'
     +OSName(OSGroup())+'\n'
     +DeviceManufacturer()+'\n'
     +DeviceModel()+'\n'
     +DeviceSerialNumber()+'\n'
     +(per_user ? OSUserName() : S)+'\n';

   MD5 hash;
   hash.update(id);
#if ANDROID && 0 // don't do this because of "app-signing key" dependency
   if(per_user){ULong android_id=AndroidID(); hash.update(&android_id, SIZE(android_id));} // 'AndroidID' depends on app-signing key, user and device
#endif
   ULong  mac=GetMac(); hash.update(&mac, SIZE(mac));
   return hash();
}
Str DeviceManufacturer()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID method=jni->GetStaticMethodID(ActivityClass, "manufacturer", "()Ljava/lang/String;"))
      if(JString str=JString(jni, jni->CallStaticObjectMethod(ActivityClass, method)))
         return str.str();
#elif APPLE
   return "Apple";
#endif
   return S;
}
Str DeviceModel()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID method=jni->GetStaticMethodID(ActivityClass, "model", "()Ljava/lang/String;"))
      if(JString str=JString(jni, jni->CallStaticObjectMethod(ActivityClass, method)))
         return str.str();
#endif
   return S;
}
Str8 DeviceSerialNumber()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID method=jni->GetStaticMethodID(ActivityClass, "serial", "()Ljava/lang/String;"))
      if(JString str=JString(jni, jni->CallStaticObjectMethod(ActivityClass, method)))
         return str.str();
#endif
   return S;
}
ULong AndroidID()
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
      if(JMethodID method=jni->GetStaticMethodID(ActivityClass, "androidID", "()Ljava/lang/String;"))
      if(JString str=JString(jni, jni->CallStaticObjectMethod(ActivityClass, method)))
   {
      Char8 id[2+16+1]; // 0x + 0000000000000000 + nul
      Set(id, "0x"); Append(id, str.str());
      return TextULong(id);
   }
#endif
   return 0;
}
/******************************************************************************/
Str MicrosoftWindowsStoreLink(C Str &app_id) {return S+"https://www.microsoft.com/store/apps/"         +app_id;}
Str         AppleAppStoreLink(C Str &app_id) {return S+"https://itunes.apple.com/app/id"               +app_id;}
Str       GooglePlayStoreLink(C Str &app_id) {return S+"https://play.google.com/store/apps/details?id="+app_id;}
/******************************************************************************/
Str CString(C Str &str)
{
   Str temp; temp.reserve(str.length());
   FREPA(str)
   {
      Char   c=str()[i]; // () avoids range check
      switch(c)
      {
         case '\\':             // !! no break on purpose !!
         case '"' : temp+='\\'; // !! no break on purpose !!
         default  : temp+=c    ; break;
         case '\n': temp+="\\n"; break;
         case '\r': temp+="\\r"; break;
         case '\t': temp+="\\t"; break;
         case '\0': temp+="\\0"; break;
      }
   }
   return temp;
}
Str XmlString(C Str &str)
{
   Str temp; temp.reserve(str.length());
   FREPA(str)
   {
      Char c=str()[i]; // () avoids range check
      if(c=='&' )temp+="&amp;" ;else
      if(c=='<' )temp+="&lt;"  ;else
      if(c=='>' )temp+="&gt;"  ;else
      if(c=='\'')temp+="&apos;";else
      if(c=='"' )temp+="&quot;";else
                 temp+=c;
   }
   return temp;
}
Str DecodeXmlString(C Str &str)
{
   Str temp; temp.reserve(str.length());
   FREPA(str)
   {
      Char c=str()[i]; // () avoids range check
      if(  c=='&') // special
      {
         if(str[i+1]=='l' && str[i+2]=='t' &&                                   str[i+3]==';'){temp+='<' ; i+=3; continue;}
         if(str[i+1]=='g' && str[i+2]=='t' &&                                   str[i+3]==';'){temp+='>' ; i+=3; continue;}
         if(str[i+1]=='a' && str[i+2]=='p' && str[i+3]=='o' && str[i+4]=='s' && str[i+5]==';'){temp+='\''; i+=5; continue;}
         if(str[i+1]=='q' && str[i+2]=='u' && str[i+3]=='o' && str[i+4]=='t' && str[i+5]==';'){temp+='"' ; i+=5; continue;}
         if(str[i+1]=='a' && str[i+2]=='m' && str[i+3]=='p'                  && str[i+4]==';'){temp+='&' ; i+=4; continue;}
      }
      temp+=c;
   }
   return temp;
}
/******************************************************************************/
static CChar* GetPath2(CChar *name, Char (&dest)[MAX_LONG_PATH]=ConstCast(TempChar<MAX_LONG_PATH>()).c)
{
   if(Int length=SetReturnLength(dest, name))for(; length--; )if(IsSlash(dest[length])){dest[length]=0; return dest;}
   dest[0]=0; return dest;
}
static CChar* GetBase2(CChar *name)
{
   if(Int length=Length(name))
   {
      for(; length--; )if(IsSlash(name[length]))break;
      return name+length+1;
   }
   return null;
}

REG_KEY_TYPE GetReg(REG_KEY_GROUP reg_key_group, C Str &name, Memc<Byte> *data)
{
   if(data)data->clear();
   REG_KEY_TYPE type=REG_KEY_NONE;

#if WINDOWS_OLD
   HKEY group, key;
   switch(reg_key_group)
   {
      case RKG_CLASSES_ROOT : group=HKEY_CLASSES_ROOT ; break;
      case RKG_CURRENT_USER : group=HKEY_CURRENT_USER ; break;
      case RKG_LOCAL_MACHINE: group=HKEY_LOCAL_MACHINE; break;
      case RKG_USERS        : group=HKEY_USERS        ; break;
      default               : return REG_KEY_NONE;
   }
   if(RegOpenKeyExW(group, Replace(GetPath2(name), '/', '\\'), 0, KEY_QUERY_VALUE                , &key)!=ERROR_SUCCESS)
#if X64 // try 32-bit
   if(RegOpenKeyExW(group, Replace(GetPath2(name), '/', '\\'), 0, KEY_QUERY_VALUE|KEY_WOW64_32KEY, &key)!=ERROR_SUCCESS)return type;
#else   // try 64-bit
   if(RegOpenKeyExW(group, Replace(GetPath2(name), '/', '\\'), 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &key)!=ERROR_SUCCESS)return type;
#endif

   CChar *base=GetBase2(name);
   DWORD  dwType, dwSize;
   if(RegQueryValueExW(key, WChar(base), null, &dwType, null, &dwSize)==ERROR_SUCCESS)
   {
      switch(dwType)
      {
         case REG_DWORD : type=REG_KEY_U32   ; break;
         case REG_QWORD : type=REG_KEY_U64   ; break;
         case REG_SZ    : type=REG_KEY_STRING; break;
         case REG_BINARY: type=REG_KEY_DATA  ; break;
      }
      if(type && data)
      {
         data->setNum(dwSize);
         if(RegQueryValueExW(key, WChar(base), null, null, data->data(), &dwSize)!=ERROR_SUCCESS)data->clear();
      }
   }
   RegCloseKey(key);
#endif
   return type;
}
Str GetRegStr(REG_KEY_GROUP reg_key_group, C Str &name, Bool *success)
{
   Memc<Byte> data;
   switch(GetReg(reg_key_group, name, &data))
   {
      case REG_KEY_STRING:
      {
         if(success)*success=true;
         return (Char*)data.data();
      }break;
   }
   if(success)*success=false;
   return S;
}
UInt GetRegUInt(REG_KEY_GROUP reg_key_group, C Str &name, Bool *success)
{
   Memc<Byte> data;
   switch(GetReg(reg_key_group, name, &data))
   {
      case REG_KEY_U32:
      case REG_KEY_U64:
      {
         if(success)*success=true;
         return *(UInt*)data.data();
      }break;
   }
   if(success)*success=false;
   return 0;
}
static Bool SetReg(REG_KEY_GROUP reg_key_group, C Str &name, REG_KEY_TYPE reg_key_type, CPtr data, Int size)
{
   Bool ok=false;
#if WINDOWS_OLD
   UInt type;
   HKEY key;
   switch(reg_key_type)
   {
      case REG_KEY_DATA  : type=REG_BINARY; break;
      case REG_KEY_STRING: type=REG_SZ    ; break;
      case REG_KEY_U32   : type=REG_DWORD ; break;
      case REG_KEY_U64   : type=REG_QWORD ; break;
      default            : return false;
   }
   switch(reg_key_group)
   {
      case RKG_CLASSES_ROOT : key=HKEY_CLASSES_ROOT ; break;
      case RKG_CURRENT_USER : key=HKEY_CURRENT_USER ; break;
      case RKG_LOCAL_MACHINE: key=HKEY_LOCAL_MACHINE; break;
      case RKG_USERS        : key=HKEY_USERS        ; break;
      default               : return false;
   }
   if(RegCreateKeyEx(key, Replace(GetPath2(name), '/', '\\'), 0, null, 0, KEY_WRITE, null, &key, null)==ERROR_SUCCESS)
   {
      if(RegSetValueEx(key, WChar(GetBase2(name)), 0, type, (const BYTE*)data, size)==ERROR_SUCCESS)
      {
         ok=true;
      }
      RegCloseKey(key);
   }
#endif
   return ok;
}
Bool SetRegStr(REG_KEY_GROUP reg_key_group, C Str &name, C Str &value)
{
   return value.is() ? SetReg(reg_key_group, name, REG_KEY_STRING, value(), (value.length()+1)*SIZE(Char))  // include null-terminated zero, and multiply by size of Char
                     : SetReg(reg_key_group, name, REG_KEY_STRING, null   ,                             0); // this is OK for empty string
}
Bool SetRegUInt(REG_KEY_GROUP reg_key_group, C Str &name, UInt value)
{
   return SetReg(reg_key_group, name, REG_KEY_U32, &value, SIZE(value));
}
Bool SetRegData(REG_KEY_GROUP reg_key_group, C Str &name, CPtr data, Int size)
{
   return SetReg(reg_key_group, name, REG_KEY_DATA, data, size);
}
/******************************************************************************/
UID FileNameID(C Str &name)
{
   UID id; if(name.is() && DecodeFileName(_GetBase(name), id))return id;
   return UIDZero;
}
/******************************************************************************/
VecI4 FileVersion(C Str &name)
{
#if WINDOWS_OLD
   if(UInt size=GetFileVersionInfoSize(name, null))
   {
      Memt<Byte> data; data.setNum(size);
      if(GetFileVersionInfo(name, 0, data.elms(), data.data()))
      {
         VS_FIXEDFILEINFO *info=null;
         UINT              info_size=0;
         if(VerQueryValue(data.data(), L"\\", (Ptr*)&info, &info_size))if(info && info_size==SIZE(VS_FIXEDFILEINFO))
         {
            UInt MajorVersion  =HIWORD(info->dwFileVersionMS),
                 MinorVersion  =LOWORD(info->dwFileVersionMS),
                 BuildNumber   =HIWORD(info->dwFileVersionLS),
                 RevisionNumber=LOWORD(info->dwFileVersionLS);
            return VecI4(MajorVersion, MinorVersion, BuildNumber, RevisionNumber);
         }
      }
   }
#endif
   return -1;
}
/******************************************************************************/
Int TextPatch::diffLength()C
{
   Int d=0;
   REPA(diffs)
   {
    C Diff &diff=diffs[i]; if(diff.mode!=EQUAL)d+=diff.text.length();
   }
   return d;
}
Int Difference(C Str &a, C Str &b)
{
   Memt<TextPatch> patches; Merge(a, b, b, patches);
   Int d=0; REPA(patches)d+=patches[i].diffLength(); return d;
}

#if 1 // use EE-base Qt
struct QStringList;

typedef UShort ushort;
typedef UInt   uint;

T2(A, B) struct QPair : std__pair<A, B>
{
   QPair() {}
   QPair(C A &a, C B &b) : std__pair<A, B>(a, b) {}
};

T1(TYPE) struct QVector : std__vector<TYPE>
{
   QVector() {}
   QVector(Int elms) : std__vector<TYPE>(elms) {}
};

T1(TYPE) struct QList
{
   Memx<TYPE> _; // use 'Memx' because some codes assume that elements are in const memory address

   int size ()C {return _.elms();}
   int count()C {return _.elms();}

   bool   empty()C {return _.elms()<=0;}
   bool isEmpty()C {return _.elms()<=0;}

   TYPE& operator[](int i)  {return _[i];}
 C TYPE& operator[](int i)C {return _[i];}

   TYPE& first()  {return _.first();}
 C TYPE& first()C {return _.first();}

   TYPE& last()  {return _.last();}
 C TYPE& last()C {return _.last();}

   TYPE& front()  {return _.first();}
 C TYPE& front()C {return _.first();}

   TYPE& back()  {return _.last();}
 C TYPE& back()C {return _.last();}

   TYPE value(int i)C {return InRange(i, _) ? _[i] : TYPE();}

   void clear() {_.clear();}

   void  append(C TYPE &value) {_.New  ( )=value;}
   void prepend(C TYPE &value) {_.NewAt(0)=value;}

   void removeFirst() {_.removeValid(0, true);}
   void removeLast () {_.removeLast (       );}

   void        operator+=(C       TYPE  &value)  {_.New()=value;}
   void        operator+=(C QList<TYPE> &other)  {FREPA(other)T+=other[i];}
   QList<TYPE> operator+ (C QList<TYPE> &other)C {QList<TYPE> temp=T; temp+=other; return temp;}
};
T1(TYPE) static inline Int Elms(C QList<TYPE> &q) {return q.size();}

T1(TYPE) struct QStack : QList<TYPE>
{
   void  push(C TYPE &data) {QList<TYPE>::_.New()=data;}
   TYPE& top (            ) {return QList<TYPE>::_.last();}
   TYPE  pop (            ) {TYPE last=QList<TYPE>::_.last(); QList<TYPE>::_.removeLast(); return last;}
};

T1(TYPE) struct QMutableListIterator
{
   QList<TYPE> &list;
   int          index;
   bool         dir;

   bool hasNext    ()C {return InRange(index  , list);}
   bool hasPrevious()C {return InRange(index-1, list);}

   void toFront() {index=0;}

   TYPE& next    () {dir=true ; return list[index++];}
   TYPE& previous() {dir=false; return list[--index];}

   void remove() {list._.removeValid(dir ? --index : index, true);}
   void setValue(C TYPE &value) {list._[dir ? index-1 : index]=value;}
   void insert  (C TYPE &value) {list._.NewAt(index++)=value; dir=true;}

   QMutableListIterator(QList<TYPE> &list) : list(list), index(0), dir(true) {}
};

struct QChar
{
   Char _;
   
   ushort  unicode()C {return _;}
   ushort& unicode()  {return (ushort&)_;} ASSERT(SIZE(Char)==SIZE(ushort));
   Char8   toAscii()C {return Char16To8Fast(_);} // we can assume that Str was already initialized

   bool operator==(C QChar &c)C {return _==c._;}
   bool operator!=(C QChar &c)C {return _!=c._;}

   bool isControl       ()C {return           _=='\r' || _=='\n';}
   bool isSpace         ()C {return _==' ' || _=='\t' || _=='\n' || _=='\v' || _=='\f' || _=='\r';}
   bool isLetterOrNumber()C {return CharType(_)==CHART_CHAR;}

   QChar(        ) {_='\0';}
   QChar(Char8  c) {_=c;}
   QChar(Char   c) {_=c;}
   QChar(ushort c) {_=c;}
};
struct QString
{
   enum SplitBehavior
   {
      KeepEmptyParts,
      SkipEmptyParts,
   };

   Str _;

   QString(            )          {}
   QString(QChar      c) : _(c._) {}
   QString(CChar8     c) : _(c  ) {}
   QString(CChar      c) : _(c  ) {}
   QString(CChar8    *t) : _(t  ) {}
   QString(CChar     *t) : _(t  ) {}
   QString(C Str8    &s) : _(s  ) {}
   QString(C Str     &s) : _(s  ) {}
   QString(C QString &s) : _(s._) {}

   int     length()C {return _.length();}
 C QChar* unicode()C {return (QChar*)_();} ASSERT(SIZE(Char)==SIZE(QChar));

   QString  operator+ (C QChar   &c)C {return _+c._;}
   QString  operator+ (C QString &s)C {return _+s._;}
   QString& operator+=(   Char8   c)  {_+=c  ; return T;}
   QString& operator+=(   Char    c)  {_+=c  ; return T;}
   QString& operator+=(C QChar   &c)  {_+=c._; return T;}
   QString& operator+=(C QString &s)  {_+=s._; return T;}

   QString& append(C QChar   &c) {T+=c; return T;}
   QString& append(C QString &s) {T+=s; return T;}

   bool operator==(C QString &s)C {return  Equal(_, s._, true);}
   bool operator!=(C QString &s)C {return !Equal(_, s._, true);}

   bool isNull ()C {return length()<=0;} // _()==null;} check the same as 'isEmpty' because of different string implementation in EE vs Qt
   bool isEmpty()C {return length()<=0;}

   bool   endsWith(C QString &s)C {return Ends  (_, s._, true);}
   bool startsWith(C QString &s)C {return Starts(_, s._, true);}

   bool isBlankLineEnd()C // regexp "\\n\\r?\\n$"
   {
      int i=_.length()-1; // last char pos
      if(_[i]=='\n')
      {
         i--;
         if(_[i]=='\r')i--;
         if(_[i]=='\n')return true;
      }
      return false;
   }
   bool isBlankLineStart()C // regexp "^\\r?\\n\\r?\\n"
   {
      int i=0; // first char pos
      if(_[i]=='\r')i++;
      if(_[i]=='\n')
      {
         i++;
         if(_[i]=='\r')i++;
         if(_[i]=='\n')return true;
      }
      return false;
   }

   int toInt()C {return TextInt(_);}

   QString left(int n)C
   {
      if(n<0 || n>length())n=length();
      Str s; s.reserve(n); FREP(n)s+=_[i]; return s;
   }
   QString right(int n)C
   {
      if(n<0 || n>length())n=length();
      Str s; s.reserve(n); int o=length()-n; FREP(n)s+=_[o+i]; return s;
   }
   QString mid(int position, int n=-1)C
   {
      Clamp(position, 0, length());
      int left=length()-position; if(n<0 || n>left)n=left;
      Str s; s.reserve(n); FREP(n)s+=_[position+i]; return s;
   }
   QChar& operator[](int i)  {return (QChar&)(_()[i]);}
 C QChar  operator[](int i)C {return          _  [i] ;}

   int indexOf(C QString &s, int from=0)C
   {
      // following code matches Qt (do not change)
      if(from<0)from+=length();
      if(uint(s.length()+from)>(uint)length())return -1;
      if(!s.length())return from;
      if(!  length())return -1;

      Clamp(from, 0, length());
      int index=TextPosI(_()+from, s._, true);
      return (index>=0) ? index+from : -1;
   }
   int lastIndexOf(C QString &s, int from=-1)C
   {
      // following code matches Qt (do not change)
      if(from<0)from+=length();
      if(from==length() && s.length()==0)return from;
      int delta=length()-s.length();
      if(from<0 || from>=length() || delta<0)return -1;
      if(from>delta)from=delta;

      if(!s.length())return from; // this line was added for Qt compatibility (do not change)
      for(; from>=0; from--)if(Starts(_()+from, s._(), true))return from;
      return -1;
   }
   void     replace( Char  from,  Char  to) {_=Replace(_, from, to);}
   QString& replace(C Str &from, C Str &to) {_=Replace(_, from, to); return T;}

   QStringList split(Char c, SplitBehavior behavior=KeepEmptyParts)C;

   static QString number(int   n) {return TextInt(n);}
   static QString number(float n) {return TextFlt(n);}
};
static inline Int Elms(C QString &q) {return q.length();}
/*struct QUrl
{
   static QString toPercentEncoding(C QString &input, C QString &exclude=QString());
   {
      Bool normal[256]; Zero(normal);
      for(int i=0x61; i<=0x7A; i++)normal[i]=true; // ALPHA
      for(int i=0x41; i<=0x5A; i++)normal[i]=true; // ALPHA
      for(int i=0x30; i<=0x39; i++)normal[i]=true; // DIGIT
      normal[0x2D]=true; // -
      normal[0x2E]=true; // .
      normal[0x5F]=true; // _
      normal[0x7E]=true; // ~
      REPA(exclude)if(InRange(exclude[i]._, normal))normal[exclude[i]._]=true;

      QString out;
      FREPA(input)
      {
         QChar c=input[i];
         Byte bc=Byte(c._);
         if(InRange(c._, normal) ? normal[bc] : true)out+=c;else
         {
            out+='%';
            out+=Digits16[bc>> 4];
            out+=Digits16[bc&0xF];
         }
      }
      return out;
   }
   static QString fromPercentEncoding(C QString &input)
   {
      QString out;
      FREPA(input)
      {
         Char c=input._[i];
         if(c!='%')out+=c;else
         {
            UShort h=input._[++i];
            UShort l=input._[++i];

            if(h>='0' && h<='9')h-='0'   ;else
            if(h>='a' && h<='f')h-='a'-10;else
            if(h>='A' && h<='F')h-='A'-10;else continue;

            if(l>='0' && l<='9')l-='0'   ;else
            if(l>='a' && l<='f')l-='a'-10;else
            if(l>='A' && l<='F')l-='A'-10;else continue;

            out+=Char((h<<4)|l);
         }
      }
      return out;
   }
};
struct QRegExp
{
};*/
struct QStringList : QList<QString>
{
   QStringList& operator<<(C QString &s) {append(s); return T;}
};
QStringList QString::split(Char c, SplitBehavior behavior)C
{
   Memc<Str> s=Split(_, c);
   QStringList list;
   FREPA(s)
      if(behavior==KeepEmptyParts || s[i].is())
         list<<s[i];
   return list;
}
struct QVariant
{
   QString     string;
   QStringList stringlist;

   static QVariant fromValue(C QString     &s ) {QVariant v; v.string    =s ; return v;}
   static QVariant fromValue(C QStringList &sl) {QVariant v; v.stringlist=sl; return v;}

 C QString    & toString    ()C {return string    ;}
 C QStringList& toStringList()C {return stringlist;}
};
T2(KEY, DATA) struct QMap
{
   bool contains(C KEY &key)C {return ConstCast(_).find(key)!=null;}

   DATA value(C KEY &key, C DATA &defaultValue=DATA(0)) {if(DATA *data=_.find(key))return *data; return defaultValue;}
   void insert(C KEY &key, C DATA &value) {if(DATA *data=_.get(key))*data=value;}

   QMap& operator=(C QMap &map) {_=map._; return T;}

   QMap();
   QMap(C QMap &map);

private:
   Map<KEY, DATA> _;
};
static Int CompareQString(C QString &a, C QString &b) {return CompareCS(a._, b._);}
static Int CompareQChar  (C QChar   &a, C QChar   &b) {return Compare  ((UShort)a._, (UShort)b._);}

template<> QMap<QString, int>::QMap(           ) : _(CompareQString, null) {}
template<> QMap<QString, int>::QMap(C QMap &map) : _(CompareQString, null) {T=map;}
template<> QMap<QChar  , int>::QMap(           ) : _(CompareQChar  , null) {}
template<> QMap<QChar  , int>::QMap(C QMap &map) : _(CompareQChar  , null) {T=map;}

//} // close EE namespace to include "std" headers in global namespace (not needed since no headers are actually loaded in the 'diff_match_patch' at the moment)
   #undef DELETE
   #include "../ThirdPartyLibs/Diff Match Patch/diff_match_patch.cpp"
//namespace EE{

Str Merge(C Str &base, C Str &a, C Str &b, MemPtr<TextPatch> patches, Int timeout)
{
   if(!patches)
   {
      if(Equal(base, a, true))return b; // if 'a' didn't apply any changes then return 'b'
      if(Equal(base, b, true))return a; // if 'b' didn't apply any changes then return 'a'
      if(Equal(   a, b, true))return a; // if 'a' and 'b' had the same changes applied then they're the same
   }

   diff_match_patch dmp; dmp.Diff_Timeout=((timeout<=0) ? 0.0f : timeout/1000.0f);
   QList<Patch> dmp_patches=dmp.patch_make(base, a);
 C QPair<QString, QVector<bool> > &out=dmp.patch_apply(dmp_patches, b);
   if(patches)
   {
      patches.setNum(Min(dmp_patches.size(), out.second.size()));
      FREPA(patches)
      {
           C Patch &dmp_patch=dmp_patches[i];
         TextPatch &    patch=    patches[i];
         patch.ok         =out.second[i];
         patch.base_offset=dmp_patch.start1; patch.base_length=dmp_patch.length1;
         patch.   a_offset=dmp_patch.start2; patch.   a_length=dmp_patch.length2;
         patch.diffs.setNum(dmp_patch.diffs.size());
         FREPA(patch.diffs)
         {
                     C Diff &dmp_diff=dmp_patch.diffs[i];
            TextPatch::Diff &    diff=    patch.diffs[i];
            diff.text=dmp_diff.text._;
            switch(dmp_diff.operation)
            {
               case DELETE: diff.mode=TextPatch::DEL  ; break;
               case INSERT: diff.mode=TextPatch::ADD  ; break;
               default    : diff.mode=TextPatch::EQUAL; break;
            }
         }
      }
   }
   return out.first._;
}
#elif 0 // use native Qt

} // close EE namespace to include "std" headers in global namespace (not needed since no headers are actually loaded in the 'diff_match_patch' at the moment)
   #undef DELETE
   #undef T
   #undef C
   #undef T1
   #undef T2
   #undef T3
   #include "Libs/Diff Match Patch/diff_match_patch (original).cpp"
   #define T (*this)
   #define C   const
namespace EE{

QString QStr(C Str     &s) {QString q; FREPA(s)q+=QChar(s[i]); return q;}
Str     QStr(C QString &q) {return (CChar*)q.unicode();}

Str Merge(C Str &base, C Str &a, C Str &b, MemPtr<TextPatch> patches, Int timeout)
{
   if(!patches)
   {
      if(Equal(base, a, true))return b; // if 'a' didn't apply any changes then return 'b'
      if(Equal(base, b, true))return a; // if 'b' didn't apply any changes then return 'a'
      if(Equal(   a, b, true))return a; // if 'a' and 'b' had the same changes applied then they're the same
   }

   diff_match_patch dmp; dmp.Diff_Timeout=((timeout<=0) ? 0.0f : timeout/1000.0f);
   QList<Patch> dmp_patches=dmp.patch_make(QStr(base), QStr(a));
 C QPair<QString, QVector<bool> > &out=dmp.patch_apply(dmp_patches, QStr(b));
   if(patches)
   {
      patches.setNum(Min(dmp_patches.size(), out.second.size()));
      FREPA(patches)
      {
           C Patch &dmp_patch=dmp_patches[i];
         TextPatch &    patch=    patches[i];
         patch.ok         =out.second[i];
         patch.base_offset=dmp_patch.start1; patch.base_length=dmp_patch.length1;
         patch.   a_offset=dmp_patch.start2; patch.   a_length=dmp_patch.length2;
         patch.diffs.setNum(dmp_patch.diffs.size());
         FREPA(patch.diffs)
         {
                     C Diff &dmp_diff=dmp_patch.diffs[i];
            TextPatch::Diff &    diff=    patch.diffs[i];
            diff.text=QStr(dmp_diff.text);
            switch(dmp_diff.operation)
            {
               case DELETE: diff.mode=TextPatch::DEL  ; break;
               case INSERT: diff.mode=TextPatch::ADD  ; break;
               default    : diff.mode=TextPatch::EQUAL; break;
            }
         }
      }
   }
   return QStr(out.first);
}
#else // use STL port

#if WINDOWS
#pragma warning(disable:4267) // 64bit - conversion from 'size_t' to 'Int', possible loss of data
struct std__string
{
   static INLINE int     wmemcmp(CChar8 *a, CChar8 *b, size_type length) {for(; 0<length; ++a, ++b, --length)if(*a!=*b)return (*a<*b) ? -1 : +1; return 0;}
   static INLINE CChar8* wmemchr(CChar8 *t, CChar8  c, size_type length) {for(; 0<length; ++t,      --length)if(*t== c)return t; return null;}

   struct traits_type
   {
      static int compare(CChar8 *a, CChar8 *b, size_type length) {return wmemcmp(a, b, length);}
   };

   static const size_type npos=-1;

   typedef Char8 value_type;
   typedef size_t size_type;
   typedef CChar8* const_pointer;
   typedef  Char8* iterator;

   Str8 str;

   std__string operator+( Char8         c)C {return str+c    ;}
   std__string operator+( Char          c)C {return str+c    ;}
   std__string operator+(CChar8        *t)C {return str+t    ;}
   std__string operator+(CChar         *t)C {return str+t    ;}
   std__string operator+(C Str8        &s)C {return str+s    ;}
   std__string operator+(C Str         &s)C {return str+s    ;}
   std__string operator+(C std__string &s)C {return str+s.str;}

   std__string& operator+=( Char8         c) {str+=c    ; return T;}
   std__string& operator+=( Char          c) {str+=c    ; return T;}
   std__string& operator+=(CChar8        *t) {str+=t    ; return T;}
   std__string& operator+=(CChar         *t) {str+=t    ; return T;}
   std__string& operator+=(C Str8        &s) {str+=s    ; return T;}
   std__string& operator+=(C Str         &s) {str+=s    ; return T;}
   std__string& operator+=(C std__string &s) {str+=s.str; return T;}

   bool operator==(CChar8        *t)C {return Equal(str, t    , true);}
   bool operator==(CChar         *t)C {return Equal(str, t    , true);}
   bool operator==(C Str8        &s)C {return Equal(str, s    , true);}
   bool operator==(C Str         &s)C {return Equal(str, s    , true);}
   bool operator==(C std__string &s)C {return Equal(str, s.str, true);}

   bool operator!=(CChar8        *t)C {return !Equal(str, t    , true);}
   bool operator!=(CChar         *t)C {return !Equal(str, t    , true);}
   bool operator!=(C Str8        &s)C {return !Equal(str, s    , true);}
   bool operator!=(C Str         &s)C {return !Equal(str, s    , true);}
   bool operator!=(C std__string &s)C {return !Equal(str, s.str, true);}

   iterator begin() {return iterator(c_str());}
   iterator end  () {return iterator(c_str()+size());}

   Bool       empty()C {return !str.is    ();}
   CChar8*    c_str()C {return  str       ();}
   size_type   size()C {return  str.length();}
   size_type length()C {return  str.length();}
   Char8 operator[](Int i     )C {return str[i];}
   void       clear(          )  {str.clear();}
   void      resize(int length)  {if(length<=size())str.clip(length);else str.reserve(length);}
	bool     _Inside(CChar8  *s)C {if(!s || s<c_str() || c_str()+size()<=s)return false;else return true;}

   std__string substr(size_type start, size_type length=npos)C {if(length==npos)length=T.size()-start; Str8 s; s.reserve(length); FREP(length)s+=T[start+i]; return s;}

   int compare(size_type offset, size_type length, C std__string &s)C {return compare(offset, length, s, 0, npos);}
   int compare(size_type offset, size_type length, C std__string &s, size_type offset2, size_type length2)C
   {
      if(s.size()-offset2<length2)length2=s.size()-offset2;
      return compare(offset, length, s.c_str()+offset2, length2);
   }
   int compare(size_type offset, size_type length, CChar8 *s, size_type length2)C
   {
      if(size()-offset<length)length=size()-offset;
      size_type c=wmemcmp(c_str()+offset, s, (length<length2) ? length : length2);
      return (c ? (int)c : (length<length2) ? -1 : (length==length2) ? 0 : +1);
   }

   void swap(std__string &s) {Swap(T, s);}

   size_type find(C std__string &s, size_type offset=0)C {return find(s.c_str(), offset, s.size() );}
   size_type find(  CChar8      *s, size_type offset=0)C {return find(s        , offset, Length(s));}
   size_type find(  CChar8      *s, size_type offset, size_type length)C
   {
      if(length==0 && offset<=size())return offset;
      size_type i;
      if(offset<size() && length<=(i=size()-offset))
      {
         CChar8 *u, *v;
         for(i-=length-1, v=c_str()+offset; u=wmemchr(v, *s, i); i-=u-v+1, v=u+1)
            if(wmemcmp(u, s, length)==0)return u-c_str();
      }
      return npos;
   }

   size_type rfind(C std__string &s, size_type offset=0)C {return rfind(s.c_str(), offset, s.size() );}
   size_type rfind(  CChar8      *s, size_type offset=0)C {return rfind(s        , offset, Length(s));}
   size_type rfind(  CChar8      *s, size_type offset, size_type length)C
   {
      if(length==0)return (offset<size()) ? offset : size();
      if(length<=size())
      {
         CChar8 *u=c_str()+(offset<size()-length ? offset : size()-length);
         for(; ; --u)
            if(*u==*s && wmemcmp(u, s, length)==0)return u-c_str();
            else if(u==c_str())break;
      }
      return npos;
   }

	std__string& append(const std__string& s, size_type offset, size_type length)
	{
		size_type left=s.size()-offset; if(left<length)length=left;
		FREP(length)str+=s[offset+i];
		return T;
	}
	std__string& append(CChar8 *s, size_type length)
	{
		if(_Inside(s))return append(T, s-c_str(), length);
      FREP(length)str+=s[i];
		return T;
	}

   std__string(CChar8 *t, Int length) {FREP(length)str+=t[i];}
   std__string(CChar  *t, Int length) {FREP(length)str+=t[i];}
   std__string(CChar8 *t) : str(t) {}
   std__string(CChar  *t) : str(t) {}
   std__string(C Str8 &s) : str(s) {}
   std__string(C Str  &s) : str(s) {}
   std__string() {}
};
struct std__wstring
{
   static INLINE int    wmemcmp(CChar *a, CChar *b, size_type length) {for(; 0<length; ++a, ++b, --length)if(*a!=*b)return (*a<*b) ? -1 : +1; return 0;}
   static INLINE CChar* wmemchr(CChar *t, CChar  c, size_type length) {for(; 0<length; ++t,      --length)if(*t== c)return t; return null;}

   struct traits_type
   {
      static int compare(CChar *a, CChar *b, size_type length) {return wmemcmp(a, b, length);}
   };

   static const size_type npos=-1;

   typedef Char  value_type;
   typedef size_t size_type;
   typedef CChar* const_pointer;
   typedef  Char* iterator;

   Str str;

   std__wstring operator+( Char8          c)C {return str+c    ;}
   std__wstring operator+( Char           c)C {return str+c    ;}
   std__wstring operator+(CChar8         *t)C {return str+t    ;}
   std__wstring operator+(CChar          *t)C {return str+t    ;}
   std__wstring operator+(C Str8         &s)C {return str+s    ;}
   std__wstring operator+(C Str          &s)C {return str+s    ;}
   std__wstring operator+(C std__wstring &s)C {return str+s.str;}

   std__wstring& operator+=( Char8          c) {str+=c    ; return T;}
   std__wstring& operator+=( Char           c) {str+=c    ; return T;}
   std__wstring& operator+=(CChar8         *t) {str+=t    ; return T;}
   std__wstring& operator+=(CChar          *t) {str+=t    ; return T;}
   std__wstring& operator+=(C Str8         &s) {str+=s    ; return T;}
   std__wstring& operator+=(C Str          &s) {str+=s    ; return T;}
   std__wstring& operator+=(C std__wstring &s) {str+=s.str; return T;}

   bool operator==(CChar8         *t)C {return Equal(str, t    , true);}
   bool operator==(CChar          *t)C {return Equal(str, t    , true);}
   bool operator==(C Str8         &s)C {return Equal(str, s    , true);}
   bool operator==(C Str          &s)C {return Equal(str, s    , true);}
   bool operator==(C std__wstring &s)C {return Equal(str, s.str, true);}

   bool operator!=(CChar8         *t)C {return !Equal(str, t    , true);}
   bool operator!=(CChar          *t)C {return !Equal(str, t    , true);}
   bool operator!=(C Str8         &s)C {return !Equal(str, s    , true);}
   bool operator!=(C Str          &s)C {return !Equal(str, s    , true);}
   bool operator!=(C std__wstring &s)C {return !Equal(str, s.str, true);}

   iterator begin() {return iterator(c_str());}
   iterator end  () {return iterator(c_str()+size());}

   Bool       empty()C {return !str.is    ();}
   CChar*     c_str()C {return  str       ();}
   size_type   size()C {return  str.length();}
   size_type length()C {return  str.length();}
   Char  operator[](Int i     )C {return str[i];}
   void       clear(          )  {str.clear();}
   void      resize(int length)  {if(length<=size())str.clip(length);else str.reserve(length);}
	bool     _Inside(CChar   *s)C {if(!s || s<c_str() || c_str()+size()<=s)return false;else return true;}

   std__wstring substr(size_type start, size_type length=npos)C {if(length==npos)length=T.size()-start; Str s; s.reserve(length); FREP(length)s+=T[start+i]; return s;}

   int compare(size_type offset, size_type length, C std__wstring &s)C {return compare(offset, length, s, 0, npos);}
   int compare(size_type offset, size_type length, C std__wstring &s, size_type offset2, size_type length2)C
   {
      if(s.size()-offset2<length2)length2=s.size()-offset2;
      return compare(offset, length, s.c_str()+offset2, length2);
   }
   int compare(size_type offset, size_type length, CChar *s, size_type length2)C
   {
      if(size()-offset<length)length=size()-offset;
      size_type c=wmemcmp(c_str()+offset, s, (length<length2) ? length : length2);
      return (c ? (int)c : (length<length2) ? -1 : (length==length2) ? 0 : +1);
   }

   void swap(std__wstring &s) {Swap(T, s);}

   size_type find(C std__wstring &s, size_type offset=0)C {return find(s.c_str(), offset, s.size() );}
   size_type find(  CChar        *s, size_type offset=0)C {return find(s        , offset, Length(s));}
   size_type find(  CChar        *s, size_type offset, size_type length)C
   {
      if(length==0 && offset<=size())return offset;
      size_type i;
      if(offset<size() && length<=(i=size()-offset))
      {
         CChar *u, *v;
         for(i-=length-1, v=c_str()+offset; u=wmemchr(v, *s, i); i-=u-v+1, v=u+1)
            if(wmemcmp(u, s, length)==0)return u-c_str();
      }
      return npos;
   }

   size_type rfind(C std__wstring &s, size_type offset=0)C {return rfind(s.c_str(), offset, s.size() );}
   size_type rfind(  CChar        *s, size_type offset=0)C {return rfind(s        , offset, Length(s));}
   size_type rfind(  CChar        *s, size_type offset, size_type length)C
   {
      if(length==0)return (offset<size()) ? offset : size();
      if(length<=size())
      {
         CChar *u=c_str()+(offset<size()-length ? offset : size()-length);
         for(; ; --u)
            if(*u==*s && wmemcmp(u, s, length)==0)return u-c_str();
            else if(u==c_str())break;
      }
      return npos;
   }

	std__wstring& append(const std__wstring& s, size_type offset, size_type length)
	{
		size_type left=s.size()-offset; if(left<length)length=left;
		FREP(length)str+=s[offset+i];
		return T;
	}
	std__wstring& append(CChar *s, size_type length)
	{
		if(_Inside(s))return append(T, s-c_str(), length);
      FREP(length)str+=s[i];
		return T;
	}

   std__wstring(CChar8 *t, Int length) {FREP(length)str+=t[i];}
   std__wstring(CChar  *t, Int length) {FREP(length)str+=t[i];}
   std__wstring(CChar8 *t) : str(t) {}
   std__wstring(CChar  *t) : str(t) {}
   std__wstring(C Str8 &s) : str(s) {}
   std__wstring(C Str  &s) : str(s) {}
   std__wstring() {}
};

   // Include Google Diff Match Patch Library
   } // close EE namespace to include "std" headers in global namespace
   #undef C
   #undef DELETE
   // don't include unwanted headers
   //#define _STDEXCEPT_
   #define _ITERATOR_
   #define _MEMORY_
   #define _STRING_
   #define _INC_STRING
   #define _XSTRING_
   #if WINDOWS
      #include <xmemory>
   #endif
   namespace std // use custom strings to avoid linker error on VS2010 when using VS2008 lib
   {
      typedef std__string   string;
      typedef std__wstring wstring;
   }
#else
   } // close EE namespace to include "std" headers in global namespace
   #include <string>
   typedef std:: string std__string ;
   typedef std::wstring std__wstring;
#endif
   // changes:
   // std::vector -> std__vector
   // std::wcstol -> TextInt (because of Android compile errors)
   #include "Libs/Diff Match Patch Old/diff_match_patch.h"
   #define C const
   namespace EE{

Str Merge(C Str &base, C Str &a, C Str &b, MemPtr<TextPatch> patches, Int timeout)
{
   std__wstring Base=(base ? base() : u""), A=(a ? a() : u""), B=(b ? b() : u""); // std::string crashes on null
   diff_match_patch<std__wstring> dmp; dmp.Diff_Timeout=((timeout<=0) ? 0.0f : timeout/1000.0f);
   diff_match_patch<std__wstring>::Patches dmp_patches=dmp.patch_make(Base, A);
   std::pair<std__wstring, std__vector<bool> > out=dmp.patch_apply(dmp_patches, B);
   if(patches)
   {
      patches.setNum(Min((Int)dmp_patches.size(), out.second.size()));
      std::list<diff_match_patch<std__wstring>::Patch>::iterator start=dmp_patches.begin();
      FREPA(patches)
      {
         diff_match_patch<std__wstring>::Patch &dmp_patch=*start++;
         TextPatch                             &    patch= patches[i];
         patch.ok=out.second[i];
         patch.base_offset=dmp_patch.start1; patch.base_length=dmp_patch.length1;
         patch.   a_offset=dmp_patch.start2; patch.   a_length=dmp_patch.length2;
         patch.diffs.setNum((Int)dmp_patch.diffs.size());
         std::list<diff_match_patch<std__wstring>::Diff>::iterator start=dmp_patch.diffs.begin();
         FREPA(patch.diffs)
         {
            diff_match_patch<std__wstring>::Diff &dmp_diff=*start++;
            TextPatch::Diff                      &    diff=patch.diffs[i];
            diff.text=dmp_diff.text.c_str();
            switch(dmp_diff.operation)
            {
               case diff_match_patch<std__wstring>::DELETE: diff.mode=TextPatch::DEL  ; break;
               case diff_match_patch<std__wstring>::INSERT: diff.mode=TextPatch::ADD  ; break;
               default                                    : diff.mode=TextPatch::EQUAL; break;
            }
         }
      }
   }
   return out.first.c_str();
}
#endif
/******************************************************************************/
Bool Error     (C Str &msg             ) {Gui.msgBox(S, msg); return false;}
Bool ErrorDel  (C Str &file            ) {return Error(S+"Error deleting\n\""+ file+'"');}
Bool ErrorRead (C Str &file            ) {return Error(S+"Error accessing\n\""+ file+'"');}
Bool ErrorWrite(C Str &file            ) {return Error(S+"Error writing to\n\""+file+'"');}
Bool ErrorCopy (C Str &src, C Str &dest) {return Error(S+"Error copying\n\""+src+"\"\nto\n\""+dest+'"');}
Bool ErrorMove (C Str &src, C Str &dest) {return Error(S+"Error moving\n\""+src+"\"\nto\n\""+dest+'"');}
/******************************************************************************

   It's important to note, that Code Signing does not modify section offsets, sizes or data.

/******************************************************************************/
#if WINDOWS_OLD // create a Hash section for verification purposes
#define EE_SECTION_NAME "Hash" // name will be clamped to 8 chars
#pragma data_seg(EE_SECTION_NAME) // this will create a custom 'ExeSection'
ULong Hash=0; // have to specify a value (=0), because otherwise the section will not be created, and can't do "static" because it will be removed
#pragma data_seg()
static void Set(ExeSection &section, C IMAGE_SECTION_HEADER &img_section, Bool x64)
{
   ASSERT(SIZE(img_section.Name)==8 && ELMS(img_section.Name)==8 && SIZE(section.name)==9);
   CopyFast(section.name, img_section.Name, 8); section.name[8]='\0'; // !! Copy instead of Set because 'img_section.Name' may not be null-terminated !!
   if(     Equal(section.name,         ".data", true))section.type=ExeSection::VARIABLE     ;else
   if(     Equal(section.name,        ".rdata", true)
|| !x64 && Equal(section.name,         ".text", true))section.type=ExeSection::CONST_PROCESS;else // most likely this changes because of virtual function table reallocation during EXE loading
   if(     Equal(section.name, EE_SECTION_NAME, true))section.type=ExeSection::HASH         ;else // EE storage of hash
                                                      section.type=ExeSection::CONSTANT     ;
}
#endif
Bool ParseProcess(MemPtr<ExeSection> sections)
{
#if WINDOWS_OLD
   if(HINSTANCE hInstance=GetModuleHandle(null))
   {
    C IMAGE_DOS_HEADER     *img_dos     =(PIMAGE_DOS_HEADER)hInstance;
    C IMAGE_NT_HEADERS     *img_nt      =(PIMAGE_NT_HEADERS)(UIntPtr(hInstance)+img_dos->e_lfanew);
    C IMAGE_SECTION_HEADER *img_sections=(PIMAGE_SECTION_HEADER)(img_nt+1);
      sections.setNum(img_nt->FileHeader.NumberOfSections); FREPA(sections)
      {
       C IMAGE_SECTION_HEADER &img_section=img_sections[i];
         ExeSection &section=sections[i];
         Set(section, img_section, X64);
         section.offset=img_section.VirtualAddress+UIntPtr(hInstance);
         section.size  =img_section.Misc.VirtualSize;
      }
      return true;
   }
#endif
   sections.clear(); return false;
}
Bool ParseExe(File &f, MemPtr<ExeSection> sections)
{
#if WINDOWS_OLD
   #define IMAGE_DOS_SIGNATURE 0x5A4D     // MZ
   #define IMAGE_NT_SIGNATURE  0x00004550 // PE00

   ULong offset=f.pos();
   IMAGE_DOS_HEADER img_dos; f>>img_dos;
   if(img_dos.e_magic==IMAGE_DOS_SIGNATURE)
   if(f.pos(img_dos.e_lfanew))
   if(f.getUInt()==IMAGE_NT_SIGNATURE)
   {
      IMAGE_FILE_HEADER img_file; f>>img_file;
      switch(img_file.Machine)
      {
         case IMAGE_FILE_MACHINE_AMD64: {IMAGE_OPTIONAL_HEADER64 img_optional; f>>img_optional;} break;
         case IMAGE_FILE_MACHINE_I386 : {IMAGE_OPTIONAL_HEADER32 img_optional; f>>img_optional;} break;
         default                      : goto error;
      }
      if(f.ok())
      {
         Bool x64=(img_file.Machine==IMAGE_FILE_MACHINE_AMD64);
         sections.setNum(img_file.NumberOfSections);
         FREPA(sections)
         {
            IMAGE_SECTION_HEADER img_section; f>>img_section;
            ExeSection &section=sections[i];
            Set(section, img_section, x64);
            section.offset=img_section.PointerToRawData+offset;
            section.size  =img_section.Misc.VirtualSize;
         }
         if(f.ok())return true;
      }
   }
error:;
#endif
   sections.clear(); return false;
}
Bool ParseExe(C Str &name, MemPtr<ExeSection> sections)
{
   File f; if(f.readTry(name))return ParseExe(f, sections);
   sections.clear(); return false;
}
  Int         FindSectionNameI  (C MemPtr<ExeSection> &sections, CChar8 *name  ) {REPA(sections)if(Equal(sections[i].name, name, true))return i; return -1;}
  Int         FindSectionOffsetI(C MemPtr<ExeSection> &sections, CPtr    offset) {REPA(sections)if(      sections[i].contains(offset ))return i; return -1;}
C ExeSection* FindSectionName   (C MemPtr<ExeSection> &sections, CChar8 *name  ) {return sections.addr(FindSectionNameI  (sections, name  ));}
C ExeSection* FindSectionOffset (C MemPtr<ExeSection> &sections, CPtr    offset) {return sections.addr(FindSectionOffsetI(sections, offset));}
/******************************************************************************/
void InitMisc()
{
#if MAC
   PasteboardCreate(kPasteboardClipboard, &Pasteboard);
#endif
#if DEBUG
   TestCyclic();
   for(Int range=1; range<=8; range++)
   {
      Flt power=0; for(Int i=     0; i<=range; i++)power+=BlendSmoothCube(i/Flt(range)); DYNAMIC_ASSERT(Equal(power, BlendSmoothCubeSumHalf(range)), "BlendSmoothCubeSumHalf");
          power=0; for(Int i=-range; i<=range; i++)power+=BlendSmoothCube(i/Flt(range)); DYNAMIC_ASSERT(Equal(power, BlendSmoothCubeSum    (range)), "BlendSmoothCubeSum");
   }
#endif
}
/******************************************************************************/
#if APPLE
Boolean GetDictionaryBoolean(CFDictionaryRef dict, const void *key)
{
   if(dict)if(CFBooleanRef ref=(CFBooleanRef)CFDictionaryGetValue(dict, key))return CFBooleanGetValue(ref);  
   return false;  
}  
long GetDictionaryLong(CFDictionaryRef dict, const void *key)
{
   long   value=0; if(dict)if(CFNumberRef ref=(CFNumberRef)CFDictionaryGetValue(dict, key))CFNumberGetValue(ref, kCFNumberLongType, &value);
   return value;
}  
#endif
/******************************************************************************/
}
/******************************************************************************/
