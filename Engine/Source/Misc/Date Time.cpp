/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static CChar8 *_MonthNameShort[12]=
{
   "Jan",
   "Feb",
   "Mar",
   "Apr",
   "May",
   "Jun",
   "Jul",
   "Aug",
   "Sep",
   "Oct",
   "Nov",
   "Dec",
};
static CChar8 *_MonthName[12]=
{
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December",
};
static const Byte _MonthDays[12]=
{
   31, // Jan
   28, // Feb
   31, // Mar
   30, // Apr
   31, // May
   30, // Jun
   31, // Jul
   31, // Aug
   30, // Sep
   31, // Oct
   30, // Nov
   31, // Dec
};
static const Int MonthDaysAccumulated[12]=
{
   0,
  _MonthDays[ 0]+MonthDaysAccumulated[ 0],
  _MonthDays[ 1]+MonthDaysAccumulated[ 1],
  _MonthDays[ 2]+MonthDaysAccumulated[ 2],
  _MonthDays[ 3]+MonthDaysAccumulated[ 3],
  _MonthDays[ 4]+MonthDaysAccumulated[ 4],
  _MonthDays[ 5]+MonthDaysAccumulated[ 5],
  _MonthDays[ 6]+MonthDaysAccumulated[ 6],
  _MonthDays[ 7]+MonthDaysAccumulated[ 7],
  _MonthDays[ 8]+MonthDaysAccumulated[ 8],
  _MonthDays[ 9]+MonthDaysAccumulated[ 9],
  _MonthDays[10]+MonthDaysAccumulated[10],
};
/******************************************************************************/
// DATE TIME
/******************************************************************************/
Bool DateTime::valid()C
{
   if(InRange(second, 60) // second>=0 && second<=59
   && InRange(minute, 60) // minute>=0 && minute<=59
   && InRange(hour  , 24) // hour  >=0 && hour  <=23
   && day   >=1 && day   <=31
   && month >=1 && month <=12)
   {
      if(day<=_MonthDays[month-1]             )return true;
      if(month==2 && day==29 && LeapYear(year))return true;
   }
   return false;
}
Int DateTime::days()C
{
    Int    leap_years=(year+3)/4 - 3*((year-1)/400) - ((year-1)%400)/100 + (LeapYear(year) && month>=3);
    return year*365 + MonthDaysAccumulated[Mid(month, 1, 12)-1] + day-1 + leap_years; // each leap year has 1 extra day
}
Long DateTime::seconds()C
{
   return Long(days())*(60*60*24) + (hour*(60*60) + minute*60 + second);
}
Str DateTime::asText(Bool include_seconds)C
{
   Char8  temp[256];
   Str    s; s.reserve(4+1+2+1+2+1+2+1+2+(include_seconds ? 1+2 : 0));
   s+=TextInt(year, temp); s+='-'; s+=TextInt(month, temp, 2); s+='-'; s+=TextInt(day, temp, 2);
   s+=' '; s+=TextInt(hour, temp, 2); s+=':'; s+=TextInt(minute, temp, 2); if(include_seconds){s+=':'; s+=TextInt(second, temp, 2);}
   return s;
}
Str DateTime::asFileName(Bool include_seconds)C
{
   Char8  temp[256];
   Str    s; s.reserve(4+1+2+1+2+1+2+1+2+(include_seconds ? 1+2 : 0));
   s=TextInt(year, temp); s+='-'; s+=TextInt(month, temp, 2); s+='-'; s+=TextInt(day, temp, 2);
   s+=' '; s+=TextInt(hour, temp, 2); s+=','; s+=TextInt(minute, temp, 2); if(include_seconds){s+=','; s+=TextInt(second, temp, 2);}
   return s;
}
/******************************************************************************/
DateTime  & DateTime  ::zero() {Zero(T); return T;}
DateTimeMs& DateTimeMs::zero() {Zero(T); return T;}

DateTime& DateTime::getLocal()
{
#if WINDOWS
   SYSTEMTIME time; GetLocalTime(&time);
   second=time.wSecond;
   minute=time.wMinute;
   hour  =time.wHour;
   day   =time.wDay;
   month =time.wMonth;
   year  =time.wYear;
#else
   tm     t; time_t sec=time(null); localtime_r(&sec, &t);
   year  =t.tm_year+1900;
   month =t.tm_mon+1;
   day   =t.tm_mday;
   hour  =t.tm_hour;
   minute=t.tm_min;
   second=t.tm_sec;
#endif
   return T;
}
DateTimeMs& DateTimeMs::getLocal()
{
#if WINDOWS
   SYSTEMTIME time; GetLocalTime(&time);
   millisecond=time.wMilliseconds;
   second     =time.wSecond;
   minute     =time.wMinute;
   hour       =time.wHour;
   day        =time.wDay;
   month      =time.wMonth;
   year       =time.wYear;
#else
   tm          t; timeval tv; gettimeofday(&tv, null); localtime_r(&tv.tv_sec, &t);
   year       =t.tm_year+1900;
   month      =t.tm_mon+1;
   day        =t.tm_mday;
   hour       =t.tm_hour;
   minute     =t.tm_min;
   second     =t.tm_sec;
   millisecond=tv.tv_usec/1000;
#endif
   return T;
}

DateTime& DateTime::getUTC()
{
#if WINDOWS
   SYSTEMTIME time; GetSystemTime(&time);
   second=time.wSecond;
   minute=time.wMinute;
   hour  =time.wHour;
   day   =time.wDay;
   month =time.wMonth;
   year  =time.wYear;
#else
   tm     t; time_t sec=time(null); gmtime_r(&sec, &t);
   year  =t.tm_year+1900;
   month =t.tm_mon+1;
   day   =t.tm_mday;
   hour  =t.tm_hour;
   minute=t.tm_min;
   second=t.tm_sec;
#endif
   return T;
}
DateTimeMs& DateTimeMs::getUTC()
{
#if WINDOWS
   SYSTEMTIME time; GetSystemTime(&time);
   millisecond=time.wMilliseconds;
   second     =time.wSecond;
   minute     =time.wMinute;
   hour       =time.wHour;
   day        =time.wDay;
   month      =time.wMonth;
   year       =time.wYear;
#else
   timeval tv; gettimeofday(&tv, null);
   super::from1970s(tv.tv_sec);
        millisecond=tv.tv_usec/1000;
#endif
   return T;
}

DateTime& DateTime::incMonth()
{
   if(++month>12){month=1; year++;}
   return T;
}
DateTime& DateTime::decMonth()
{
   if(--month<1){month=12; year--;}
   return T;
}
DateTime& DateTime::incDay()
{
   if(++day>28)
   {
      if(day>MonthDays(month, year))
      {
         day=1;
         incMonth();
      }
   }
   return T;
}
DateTime& DateTime::decDay()
{
   if(day>1)day--;else
   {
      decMonth();
      day=MonthDays(month, year); // set 'day' after having new 'month'
   }
   return T;
}
DateTime& DateTime::incHour()
{
   if(++hour>=24){hour=0; incDay();}
   return T;
}
DateTime& DateTime::decHour()
{
   if(hour>0)hour--;else{hour=23; decDay();}
   return T;
}
DateTime& DateTime::incMinute()
{
   if(++minute>=60){minute=0; incHour();}
   return T;
}
DateTime& DateTime::decMinute()
{
   if(minute>0)minute--;else{minute=59; decHour();}
   return T;
}
DateTime& DateTime::incSecond()
{
   if(++second>=60){second=0; incMinute();}
   return T;
}
DateTime& DateTime::decSecond()
{
   if(second>0)second--;else{second=59; decMinute();}
   return T;
}
#if ANDROID && __ANDROID_API__<12 // Android below API 12 doesn't have 'timegm'
static SyncLock mktime_lock;
time_t timegm(struct tm *tm)
{
   SyncLocker locker(mktime_lock);
   char *tz=getenv("TZ"); setenv("TZ", "", 1);
   tzset();
   time_t ret=mktime(tm);
   if(tz)setenv("TZ", tz, 1);else unsetenv("TZ");
   tzset();
   return ret;
}
#endif
DateTime& DateTime::toUTC()
{
   if(valid())
   {
   #if WINDOWS
      SYSTEMTIME local, utc;
      local.wYear  =year;
      local.wMonth =month;
      local.wDay   =day;
      local.wHour  =hour;
      local.wMinute=minute;
      local.wSecond=second;
      local.wDayOfWeek   =0;
      local.wMilliseconds=0;
      if(TzSpecificLocalTimeToSystemTime(null, &local, &utc))
      {
         year  =utc.wYear;
         month =utc.wMonth;
         day   =utc.wDay;
         hour  =utc.wHour;
         minute=utc.wMinute;
         second=utc.wSecond;
      }
   #else
      tm t;
      t.tm_year =year -1900;
      t.tm_mon  =month-1;
      t.tm_mday =day;
      t.tm_hour =hour;
      t.tm_min  =minute;
      t.tm_sec  =second;
      t.tm_wday =0;
      t.tm_yday =0;
      t.tm_isdst=-1; // -1 means data is unavailable

      {
      #if ANDROID && __ANDROID_API__<12 // since Android (<12) doesn't have built-in 'timegm' function, we had to write one which modifies time zones, because of that, calls to 'mktime' must be surrounded by locks
         SafeSyncLocker locker(mktime_lock);
      #endif
         time_t sec=mktime(&t); gmtime_r(&sec, &t);
      }

      year  =t.tm_year+1900;
      month =t.tm_mon +1;
      day   =t.tm_mday;
      hour  =t.tm_hour;
      minute=t.tm_min;
      second=t.tm_sec;
   #endif
   }
   return T;
}
DateTime& DateTime::toLocal()
{
   if(valid())
   {
   #if WINDOWS
      SYSTEMTIME utc, local;
      utc.wYear  =year;
      utc.wMonth =month;
      utc.wDay   =day;
      utc.wHour  =hour;
      utc.wMinute=minute;
      utc.wSecond=second;
      utc.wDayOfWeek   =0;
      utc.wMilliseconds=0;
      if(SystemTimeToTzSpecificLocalTime(null, &utc, &local))
      {
         year  =local.wYear;
         month =local.wMonth;
         day   =local.wDay;
         hour  =local.wHour;
         minute=local.wMinute;
         second=local.wSecond;
      }
   #else
      tm t;
      t.tm_year =year -1900;
      t.tm_mon  =month-1;
      t.tm_mday =day;
      t.tm_hour =hour;
      t.tm_min  =minute;
      t.tm_sec  =second;
      t.tm_wday =0;
      t.tm_yday =0;
      t.tm_isdst=-1; // -1 means data is unavailable

      time_t sec=timegm(&t); localtime_r(&sec, &t);

      year  =t.tm_year+1900;
      month =t.tm_mon +1;
      day   =t.tm_mday;
      hour  =t.tm_hour;
      minute=t.tm_min;
      second=t.tm_sec;
   #endif
   }
   return T;
}
DateTime& DateTime::fromSeconds(Long s)
{
   Int sec =s%(60*60*24),
       days=s/(60*60*24);

   second=sec%60; sec/=60;
   minute=sec%60; sec/=60;
   hour  =sec   ;

   year=400*(days/ 146097); // amount of days in 400 years (including days from leap years)
             days%=146097 ;
   if(days>=36525) // amount of days in first 100 years (including days from leap years)
   {
      year+=100; days-=36525;
      year+=100*(days/36524); days%=36524; // amount of days for each following 100 years (including days from leap years)
   }
   if(LeapYear(year))
   {
      year+=4*(days/ 1461); // amount of days in 4 years (including days from leap years)
               days%=1461 ;
   }else
   {
      if(days>=1460) // amount of days in first 4 years (without days from leap years) since first 4 years in non 100 leap year don't have leap days
      {
         year+=4; days-=1460;
         year+=4*(days/ 1461); // amount of days in each following 4 years (including days from leap years)
                  days%=1461 ;
      }
   }
   if(LeapYear(year))
   {
      if(days>=366)
      {
         year++; days-=366;
         year+=days/ 365;
               days%=365;
      }
   }else
   {
      year+=days/ 365;
            days%=365;
   }

   month=1; REP(11){Int d=MonthDays(month, year); if(days>=d){month++; days-=d;}else break;}
   day  =1+days;
   return T;
}
DateTime& DateTime::fromText(C Str &t)
{
   Memt<Str> date_time; Split(date_time, t, ' ');
   if(date_time.elms()==2)
   {
      Memc<Str> date, time;
      Split(date, date_time[0], '-');
      Split(time, date_time[1], ':');
      if(date.elms()==3 && (time.elms()==2 || time.elms()==3))
      {
         year  =TextInt(date[0]);
         month =TextInt(date[1]);
         day   =TextInt(date[2]);
         hour  =TextInt(time[0]);
         minute=TextInt(time[1]);
         second=((time.elms()==3) ? TextInt(time[2]) : 0);
         return T;
      }
   }
   return zero();
}
static const Long UnixSeconds=62167219200; // 62167219200 is the number of seconds at 1st Jan 1970
Long DateTime  ::     seconds1970()C {return     seconds()-UnixSeconds;}
Long DateTimeMs::milliseconds1970()C {return seconds1970()*1000 + millisecond;}

DateTime& DateTime::from1970s (ULong  s) {return fromSeconds( s+UnixSeconds);}
DateTime& DateTime::from1970ms(ULong ms) {return from1970s  (ms/1000       );}

DateTimeMs& DateTimeMs::fromSeconds( Long  s) {T.millisecond=      0; super::fromSeconds( s     ); return T;}
DateTimeMs& DateTimeMs::from1970s  (ULong  s) {T.millisecond=      0; super::from1970s  ( s     ); return T;}
DateTimeMs& DateTimeMs::from1970ms (ULong ms) {T.millisecond=ms%1000; super::from1970s  (ms/1000); return T;}

#if APPLE
DateTime& DateTime::from(NSDate *date)
{
   if(date)from1970s(TruncL([date timeIntervalSince1970]));else zero();
   return T;
}
#endif
/******************************************************************************/
Bool DateTime::save(File &f)C {f.putMulti(second, minute, hour, day, month, year); return f.ok();}
Bool DateTime::load(File &f)  {f.getMulti(second, minute, hour, day, month, year); if(f.ok())return true; zero(); return false;}
/******************************************************************************/
Int Compare(C DateTime &d0, C DateTime &d1)
{
   Int d=d0.year  -d1.year  ; if(d<0)return -1; if(d>0)return +1;
       d=d0.month -d1.month ; if(d<0)return -1; if(d>0)return +1;
       d=d0.day   -d1.day   ; if(d<0)return -1; if(d>0)return +1;
       d=d0.hour  -d1.hour  ; if(d<0)return -1; if(d>0)return +1;
       d=d0.minute-d1.minute; if(d<0)return -1; if(d>0)return +1;
       d=d0.second-d1.second; if(d<0)return -1; if(d>0)return +1;
   return 0;
}
Int Compare(C DateTime &d0, C DateTime &d1, Int epsilon)
{
   Long d=d0.seconds()-d1.seconds();
   if(  d<-epsilon)return -1;
   if(  d> epsilon)return +1;
                   return  0;
}
Long operator+(C DateTime &d0, C DateTime &d1) {return d0.seconds()+d1.seconds();}
Long operator-(C DateTime &d0, C DateTime &d1) {return d0.seconds()-d1.seconds();}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool LeapYear (Int year           ) {return (!(year%4) && year%100) || !(year%400);}
Int  MonthDays(Int month          ) {return (month>=1 && month<=12) ? _MonthDays[month-1] : -1;}
Int  MonthDays(Int month, Int year) {return (month==2 && LeapYear(year)) ? 29 : MonthDays(month);}

CChar8* MonthNameShort(Int month) {return (month>=1 && month<=12) ? _MonthNameShort[month-1] : null;}
CChar8* MonthName     (Int month) {return (month>=1 && month<=12) ? _MonthName     [month-1] : null;}

static CChar8* YearName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default              : return "y";
      case TIME_NAME_MED   : case TIME_NAME_LONG   : return (x==1) ? " year" : " years";
      case TIME_NAME_MED_UP: case TIME_NAME_LONG_UP: return (x==1) ? " Year" : " Years";
   }
}
static CChar8* SimpleMonthName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default              : return "m";
      case TIME_NAME_MED   : case TIME_NAME_LONG   : return (x==1) ? " month" : " months";
      case TIME_NAME_MED_UP: case TIME_NAME_LONG_UP: return (x==1) ? " Month" : " Months";
   }
}
static CChar8* MonthName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default              : return "mo";
      case TIME_NAME_MED   : case TIME_NAME_LONG   : return (x==1) ? " month" : " months";
      case TIME_NAME_MED_UP: case TIME_NAME_LONG_UP: return (x==1) ? " Month" : " Months";
   }
}
static CChar8* DayName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default              : return "d";
      case TIME_NAME_MED   : case TIME_NAME_LONG   : return (x==1) ? " day" : " days";
      case TIME_NAME_MED_UP: case TIME_NAME_LONG_UP: return (x==1) ? " Day" : " Days";
   }
}
static CChar8* HourName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default              : return "h";
      case TIME_NAME_MED   : case TIME_NAME_LONG   : return (x==1) ? " hour" : " hours";
      case TIME_NAME_MED_UP: case TIME_NAME_LONG_UP: return (x==1) ? " Hour" : " Hours";
   }
}
static CChar8* MinuteName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default               : return "m";
      case TIME_NAME_MED    : return " min";
      case TIME_NAME_MED_UP : return " Min";
      case TIME_NAME_LONG   : return (x==1) ? " minute" : " minutes";
      case TIME_NAME_LONG_UP: return (x==1) ? " Minute" : " Minutes";
   }
}
static CChar8* SecondName(TIME_NAME name, Int x)
{
   switch(name)
   {
      default               : return "s";
      case TIME_NAME_MED    : return " sec";
      case TIME_NAME_MED_UP : return " Sec";
      case TIME_NAME_LONG   : return (x==1) ? " second" : " seconds";
      case TIME_NAME_LONG_UP: return (x==1) ? " Second" : " Seconds";
   }
}

Str TimeText(Long seconds, TIME_NAME name, Int parts)
{
   Str str;
   if(parts)
   {
      const Int spy=31557600, // seconds per year = 365.25 days per year * 12 months * 24 hours * 60 minutes * 60 seconds
                spm=spy/12; // seconds per month

      if(seconds<0){CHS(seconds); str+='-';}

      Int y =seconds/spy, // years
          s =seconds%spy; // can be truncated to Int
      Int mo=s/spm; s%=spm;
      Int d =s/(60*60*24),
          h =s/(60*60   )%24,
          m =s/(60      )%60;
          s =s           %60;
      Bool skip_empty=(parts<0); if(skip_empty)CHS(parts);
      if(y)
      {
         str+=y;
         str+=YearName(name, y);
         if(parts>=2 && (skip_empty ? mo : true))
         {
            str+=' ';
            str+=mo;
            str+=SimpleMonthName(name, mo);
         }
      }else
      if(mo)
      {
         str+=mo;
         str+=MonthName(name, mo);
         if(parts>=2 && (skip_empty ? d : true))
         {
            str+=' ';
            str+=d;
            str+=DayName(name, d);
         }
      }else
      if(d)
      {
         str+=d;
         str+=DayName(name, d);
         if(parts>=2 && (skip_empty ? h : true))
         {
            str+=' ';
            str+=h;
            str+=HourName(name, h);
         }
      }else
      if(h)
      {
         str+=h;
         str+=HourName(name, h);
         if(parts>=2 && (skip_empty ? m : true))
         {
            str+=' ';
            str+=m;
            str+=MinuteName(name, m);
         }
      }else
      if(m)
      {
         str+=m;
         str+=MinuteName(name, m);
         if(parts>=2 && (skip_empty ? s : true))
         {
            str+=' ';
            str+=s;
            str+=SecondName(name, s);
         }
      }else
      {
         str+=s;
         str+=SecondName(name, s);
      }
   }
   return str;
}
Str TimeTextHour(Long seconds, TIME_NAME name, Int parts)
{
   Str str;
   if(parts)
   {
      if(seconds<0){CHS(seconds); str+='-';}

      Long h=seconds/(60*60);
      Int  m=seconds/(60   )%60,
           s=seconds        %60;
      Bool skip_empty=(parts<0); if(skip_empty)CHS(parts);
      if(h)
      {
         str+=h;
         str+=HourName(name, h);
         if(parts>=2 && (skip_empty ? m : true))
         {
            str+=' ';
            str+=m;
            str+=MinuteName(name, m);
         }
      }else
      if(m)
      {
         str+=m;
         str+=MinuteName(name, m);
         if(parts>=2 && (skip_empty ? s : true))
         {
            str+=' ';
            str+=s;
            str+=SecondName(name, s);
         }
      }else
      {
         str+=s;
         str+=SecondName(name, s);
      }
   }
   return str;
}
/******************************************************************************/
}
/******************************************************************************/
