/******************************************************************************/
/******************************************************************************/
class TimeStamp
{
   static const long Start, // 63524217600 is the number of seconds at 1st Jan 2013 (approximate time of the first application version)
                     Unix ; // 62167219200 is the number of seconds at 1st Jan 1970

   uint u;

   bool is()C; // if was set

   uint text()C; // this method is used when saving to text, it can be replaced in the future to something like "Str text()C {return ..;}"

   TimeStamp& operator--(   );
   TimeStamp& operator--(int);
   TimeStamp& operator++(   );
   TimeStamp& operator++(int);

   TimeStamp& zero  ();
   TimeStamp& getUTC(); // set to current time
   TimeStamp& now   (); // set to current time and make sure that it's newer than the previous time

   TimeStamp& fromUnix(long u);

   bool old(C TimeStamp &now=TimeStamp().getUTC())C; // if this timestamp is older than 'now'

   bool operator==(C TimeStamp &t)C; // if equal
   bool operator!=(C TimeStamp &t)C; // if not equal
   bool operator>=(C TimeStamp &t)C; // if greater or equal
   bool operator<=(C TimeStamp &t)C; // if smaller or equal
   bool operator> (C TimeStamp &t)C; // if greater
   bool operator< (C TimeStamp &t)C; // if smaller

   TimeStamp& operator+=(int i);
   TimeStamp& operator-=(int i);

   TimeStamp operator+(int i);
   TimeStamp operator-(int i);

   long operator-(C TimeStamp &t)C;

   DateTime asDateTime()C;

   TimeStamp(   int      i );
   TimeStamp(  uint      u );
   TimeStamp(  long      l );
   TimeStamp(C DateTime &dt);
   TimeStamp(C Str      &t ); // this method is used when loading from text, it can be replaced in the future

   static int Compare(C TimeStamp &a, C TimeStamp &b);

public:
   TimeStamp();
};
/******************************************************************************/
/******************************************************************************/
extern TimeStamp     CurTime;
/******************************************************************************/
