/******************************************************************************/
class SinSound : SoundCallback
{
   flt sin_angle=0;

   virtual bool create(byte &bits, byte &channels, int &frequency, long &raw_length, int &bit_rate)override // this will be called upon first usage of the callback, Warning: this may get called on a secondary thread
   {
      bits=16;
      channels=1;
      frequency=44100;
      return true;
   }

   virtual int set(ptr data, int size)override // this will be called when 'data' needs to be filled with uncompressed sound data, Warning: this may get called on a secondary thread
   {
      short *sample=(short*)data; // we operate on 16-bit values so process them as 'short' (which are 16-bit integers)
      int shorts=size/SIZE(short); // number of shorts to process
      REP(shorts)
      {
         *sample++=Sin(sin_angle)*32767;
         sin_angle+=0.04;
      }
      return size; // return number of bytes written
   }

   virtual bool raw(long raw)override {return true;} // position was requested to be changed, however we don't need to do anything in here, Warning: this may get called on a secondary thread

   virtual void del()override {} // this is called when the sound using this callback gets deleted, Warning: this may get called on a secondary thread
}
SinSound ss;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   SoundPlay(ss); // play sound using callback

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text(0, 0, "This tutorial plays a sound with data created dynamically by the application");
}
/******************************************************************************/
