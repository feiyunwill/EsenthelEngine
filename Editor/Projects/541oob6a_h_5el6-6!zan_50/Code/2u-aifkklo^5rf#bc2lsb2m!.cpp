/******************************************************************************/
class SoundCapture : SoundRecord
{
   short data[64*1024]; // buffer for storing samples
   int   pos=0;         // current buffer position (in bytes, this is not an index)

   virtual void receivedData(CPtr data, Int size)override // this method will be called upon receiving audio data, it may get called on secondary thread
   {
      int   pos =T.pos;
      byte *dest=(byte*)T.data+pos;
    C byte *src =(byte*)  data;
      for(; size>0; )
      {
         int copy=Min(size, (int)SIZE(T.data)-pos);
         Copy(dest, src, copy);
         size-=copy;
         src +=copy;
         pos +=copy;
         dest+=copy;
         if(pos>=SIZE(T.data))
         {
            pos =0;
            dest=(byte*)T.data;
         }
      }
      AtomicSet(T.pos, pos);
   }
   void draw()
   {
      // draw data
      VI.color(GREEN);
      REPA(data)VI.dot(Vec2(Lerp(-D.w(), D.w(), flt(i)/Elms(data)), data[i]*D.h()/32768), 0.005);
      VI.end();

      // draw current position
      D.lineY(RED, Lerp(-D.w(), D.w(), flt(pos)/SIZE(data)), -D.h(), D.h());
   }
}
SoundCapture SC;
Memc<SoundRecord.Device> SoundRecordDevices;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   SoundRecord.GetDevices(SoundRecordDevices); // get list of sound record devices
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
   if(!SC.is() && !SC.create(null, 16, 1, 44100)) // try creating in each frame in case we have to wait for a permission
   {
      if(!ANDROID)Exit("Failed to record sound"); // use default (null) sound record device
   }
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   SC.draw();

   flt y=D.h();
                            D.text(0, y-=0.1, "Sound Record Devices Found:");
   FREPA(SoundRecordDevices)D.text(0, y-=0.1, SoundRecordDevices[i].name);
}
/******************************************************************************/
