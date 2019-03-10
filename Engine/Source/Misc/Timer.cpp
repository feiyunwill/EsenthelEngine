/******************************************************************************

   Flt Error for big values, obtained using the formula:
      Dbl sec=1, min=60*sec, hour=60*min, day=hour*24, month=day*30, year=month*12;
      Flt t=.., t1=t; IncRealByBit(t1); Flt dt=t1-t;
   1 day  : 0.0078125s
   1 month: 0.25s
   1 year : 2.0s
   2 years: 4.0s
   4 years: 8.0s
   8 years: 16.0s

   /******************************************************************************

   Slow to Fast (1000000 calls)
      -clock          (0.036s)
      -timeGetTime    (0.018s)
      -GetTickCount   (0.004s)
      -GetTickCount64 (0.004s)

   Precision:
      -clock          ( 1ms)
      -timeGetTime    ( 1ms)
      -GetTickCount   (16ms)
      -GetTickCount64 (16ms)

   Measured with:
      Dbl t0[]={Time.curTime(), clock(), timeGetTime(), GetTickCount(), GetTickCount64()};
      REP(1000)
      {
         Dbl t1[]={Time.curTime(), clock(), timeGetTime(), GetTickCount(), GetTickCount64()};

         Dbl td[]={(t1[0]-t0[0])*1000, t1[1]-t0[1], t1[2]-t0[2], t1[3]-t0[3], t1[4]-t0[4]};

         LogName(S);
         LogConsole();
         LogN(S+td[0]+' '+td[1]+' '+td[2]+' '+td[3]+' '+td[4]);

         Copy(t0, t1);
      }

/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
TimeClass Time;
/******************************************************************************/
// TIME
/******************************************************************************/
TimeClass::TimeClass()
{
  _pc=false;
  _skip=0;
  _start_time_ms=_frame=_frames_drawn=0;
  _start_time=0;
  _fps=_fps_time=_state_update=_state_draw=0;
  _rd_2=_rd_1=_rd=_ad=_d=_rt=_at=_st=_t=0;
  _time_mul=0;
  _speed=1;
}
/******************************************************************************/
Dbl TimeClass::curTime()C
{
#if WINDOWS
   if(_pc)
   {
      LARGE_INTEGER pc; QueryPerformanceCounter(&pc);
      return       (pc.QuadPart-_start_time)*_time_mul;
   }
   #if WINDOWS_OLD
      return (timeGetTime()-_start_time_ms)*_time_mul;
   #elif WINDOWS_NEW
      return (clock()-_start_time_ms)*_time_mul;
   #endif
#elif APPLE
   return (mach_absolute_time()-_start_time)*_time_mul;
#elif LINUX || ANDROID
   struct timespec time; clock_gettime(CLOCK_MONOTONIC, &time);
   return (ULong(time.tv_sec)*1000000000+time.tv_nsec-_start_time)/1000000000.;
#elif WEB
   return (emscripten_get_now()-_start_time)/1000;
#endif
}
UInt TimeClass::curTimeMs()C
{
#if WINDOWS_OLD
   return timeGetTime()-_start_time_ms;
#elif WINDOWS_NEW
   return clock()-_start_time_ms; ASSERT(CLOCKS_PER_SEC==1000);
#elif APPLE
   return TruncU((mach_absolute_time()-_start_time)*_time_mul*1000);
#elif LINUX || ANDROID
   struct timespec time; clock_gettime(CLOCK_MONOTONIC, &time);
   return TruncU((ULong(time.tv_sec)*1000000000+time.tv_nsec-_start_time)*(1000/1000000000.));
#elif WEB
   return TruncU(emscripten_get_now()-_start_time);
#endif
}
/******************************************************************************/
void TimeClass::speed(Flt speed)
{
   if(T._speed!=speed)
   {
      T._speed=speed;
      SpeedSound();
   }
}
/******************************************************************************/
void TimeClass::skipUpdate(Byte frames           ) {MAX(_skip, frames);}
void TimeClass::wait      (Int       milliseconds)
{
   if(milliseconds>0)
   {
   #if WINDOWS
      Sleep(milliseconds);
   #else
      usleep(milliseconds*1000);
   #endif
   }
}
/******************************************************************************/
void (*SteamSetTime)(); // have to use variable so we don't call 'Steam' manually (because that would force require DLL), but rather Steam methods set this pointer
void TimeClass::create()
{
   if(LogInit)LogN("TimeClass.create");
#if WINDOWS
   LARGE_INTEGER value;
   if(_pc=(QueryPerformanceFrequency(&value)!=0))
   {
     _time_mul=1.0/value.QuadPart; // ticks per second
      QueryPerformanceCounter(&value); _start_time=value.QuadPart;
   }else
   {
     _time_mul=1.0/1000;
   }
   #if WINDOWS_OLD
     _start_time_ms=timeGetTime();
   #elif WINDOWS_NEW
     _start_time_ms=clock();
   #endif
#elif APPLE
   mach_timebase_info_data_t info;
   mach_timebase_info(&info);
  _time_mul  =info.numer/1000000000./info.denom; // use info and convert from nanoseconds to seconds
  _start_time=mach_absolute_time();
#elif LINUX || ANDROID
   struct timespec time; clock_gettime(CLOCK_MONOTONIC, &time);
  _start_time=ULong(time.tv_sec)*1000000000+time.tv_nsec;
#elif WEB
  _start_time=TruncUL(emscripten_get_now());
#endif

   if(SteamSetTime)SteamSetTime();
}
/******************************************************************************/
void TimeClass::update()
{
   // this is no longer needed after adding 'new_rd' test below: if(!_rt)skipUpdate(); // if it's the first time we're updating the timer, then skip setting time delta's (if we won't do this, then the time delta would be equal to loading the whole application, which can take sometimes few seconds, that's too much for frame time delta)

   Dbl cur_time=curTime(), new_rd=cur_time-_rt;

   // detect slow downs
   if(!_skip && new_rd>_rd*3) // if new delta is 3x bigger than old one, then most probably there's some system slow down or app loaded something, so skip update, it's important to check this on RD and then set it either way, because if we would be checking D or AD then skipping could go on and on (for example if we've just entered more complex level, and rendering takes now longer in each frame, then all subsequent frames would have delta much bigger than originally, and it would never stop skipping, however with RD we're just checking first frame of sudden drop, which then all next frames will be processed normally without skipping)
      if(D.sync() ? new_rd>(Abs(_rd-_rd_1)+Abs(_rd_1-_rd_2)+Abs(_rd-_rd_2))*5 : true) // when sync is enabled check also the deviations between frame deltas, because sometimes rendering can be jerky (1st frame is 15ms, 2nd frame is 2ms and 3rd is 48ms and so on) so by checking the amount of deviations we're preventing slow downs because of jerky rendering
        _skip=true;

  _rt=cur_time; _rd_2=_rd_1; _rd_1=_rd; _rd=new_rd; // set new values

   if(_skip)_skip--;else _ad=_sv_ad.update(_rd);
   _d =_ad*_speed;
  _at+=_ad;
  _st+=_ad;
   _t+= _d;

  _frame++;
  _frames_drawn++; if((_fps_time+=_rd)>=1)
   {
     _fps         =_frames_drawn/_fps_time;
     _fps_time    =0;
     _frames_drawn=0;
   }
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool EventOccurred(Flt event_time, Flt start_time, Flt dt)
{
   return start_time<=event_time && start_time+dt>event_time;
}
Bool EventBetween(Flt event_time_from, Flt event_time_to, Flt start_time, Flt dt)
{
   return start_time<=event_time_to && start_time+dt>=event_time_from;
}
/******************************************************************************/
}
/******************************************************************************/
