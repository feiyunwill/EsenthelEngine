/******************************************************************************

   Use 'DateTime' to handle time and date information.

/******************************************************************************/
struct DateTime
{
   Byte second, // 0..59
        minute, // 0..59
        hour  , // 0..23
        day   , // 1..31
        month ; // 1..12
   Int  year  ;

   // get
   Bool valid      (                         )C; // if  current date time is valid
   Int  days       (                         )C; // get    days since 1 January    0 year
   Long seconds    (                         )C; // get seconds since 1 January    0 year
   Long seconds1970(                         )C; // get seconds since 1 January 1970 year (Unix Time)
   Str  asText     (Bool include_seconds=true)C; // get date in text      format "YYYY-MM-DD HH:MM:SS"
   Str  asFileName (Bool include_seconds=true)C; // get date in file name format "YYYY-MM-DD HH,MM,SS"

   // set
   DateTime& zero       (        ); // set date and time to zero
   DateTime& getLocal   (        ); // set from current DateTime (local time zone)
   DateTime& getUTC     (        ); // set from current DateTime (UTC   time zone)
   DateTime& incMonth   (        ); // increase by 1 month
   DateTime& decMonth   (        ); // decrease by 1 month
   DateTime& incDay     (        ); // increase by 1 day
   DateTime& decDay     (        ); // decrease by 1 day
   DateTime& incHour    (        ); // increase by 1 hour
   DateTime& decHour    (        ); // decrease by 1 hour
   DateTime& incMinute  (        ); // increase by 1 minute
   DateTime& decMinute  (        ); // decrease by 1 minute
   DateTime& incSecond  (        ); // increase by 1 second
   DateTime& decSecond  (        ); // decrease by 1 second
   DateTime& toUTC      (        ); // convert from local to UTC   time zone
   DateTime& toLocal    (        ); // convert from UTC   to local time zone
   DateTime& fromSeconds( Long  s); // set date from      seconds since 1 January    0 year
   DateTime& from1970s  (ULong  s); // set date from      seconds since 1 January 1970 year (Unix Time)
   DateTime& from1970ms (ULong ms); // set date from milliseconds since 1 January 1970 year
   DateTime& fromText   (C Str &t); // set date from text format "YYYY-MM-DD HH:MM:SS" ("YYYY-MM-DD HH:MM" format is also supported), 'zero' method is called on fail
#if EE_PRIVATE && APPLE
   DateTime& from       (NSDate *date); // set date from milliseconds since 1 January 1970 year
#endif

   // io
   Bool save(File &f)C; // false on fail
   Bool load(File &f) ; // false on fail
};
STRUCT(DateTimeMs , DateTime) // DateTime uncluding milliseconds
//{
   UShort millisecond; // 0..999

   Long milliseconds1970()C; // get milliseconds since 1 January 1970 year

   DateTimeMs& zero       (        ); // set date and time to zero
   DateTimeMs& getLocal   (        ); // set from current DateTime (local time zone)
   DateTimeMs& getUTC     (        ); // set from current DateTime (UTC   time zone)
   DateTimeMs& fromSeconds( Long  s); // set date from      seconds since 1 January    0 year
   DateTimeMs& from1970s  (ULong  s); // set date from      seconds since 1 January 1970 year (Unix Time)
   DateTimeMs& from1970ms (ULong ms); // set date from milliseconds since 1 January 1970 year
};
/******************************************************************************/
// compare
       Int  Compare   (C DateTime &d0, C DateTime &d1             ); // compare
       Int  Compare   (C DateTime &d0, C DateTime &d1, Int epsilon); // compare using 'epsilon' for seconds tolerance
inline Bool operator==(C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)==0;} // if equal
inline Bool operator!=(C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)!=0;} // if not equal
inline Bool operator>=(C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)>=0;} // if greater or equal
inline Bool operator<=(C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)<=0;} // if smaller or equal
inline Bool operator> (C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)> 0;} // if greater
inline Bool operator< (C DateTime &d0, C DateTime &d1             ) {return Compare(d0, d1)< 0;} // if smaller

Long operator+(C DateTime &d0, C DateTime &d1); // return sum             of DateTime seconds
Long operator-(C DateTime &d0, C DateTime &d1); // return difference between DateTime seconds

Bool LeapYear (Int year           ); // check if 'year' is a leap year
Int  MonthDays(Int month          ); // return number of days in a month, 'month'=1..12, -1 on fail
Int  MonthDays(Int month, Int year); // return number of days in a month, 'month'=1..12, -1 on fail, this makes additional check to the 'year' if it's a leap year

CChar8* MonthNameShort(Int month); // get month short name (Jan    , Feb     , Mar  , ..), 'month'=1..12, null on fail
CChar8* MonthName     (Int month); // get month full  name (January, February, March, ..), 'month'=1..12, null on fail

enum TIME_NAME
{
   TIME_NAME_SHORT  , // short  names, lower case
   TIME_NAME_MED    , // medium names, lower case
   TIME_NAME_MED_UP , // medium names, upper case
   TIME_NAME_LONG   , // long   names, lower case
   TIME_NAME_LONG_UP, // long   names, upper case
};
Str TimeText    (Long seconds, TIME_NAME name=TIME_NAME_SHORT, Int parts=-2); // convert seconds to string, Sample Usage: TimeText    (61) -> "1m 1s"
Str TimeTextHour(Long seconds, TIME_NAME name=TIME_NAME_SHORT, Int parts=-2); // convert seconds to string, Sample Usage: TimeTextHour(61) -> "1m 1s", this works like 'TimeText' but does not display "years months days", but only "hours minutes seconds"
/******************************************************************************/
