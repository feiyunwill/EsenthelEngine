/******************************************************************************/
void InitPre()
{
   EE_INIT();
   SimulateTouches(true); // enable simulating the 'Touches' behavior using mouse for systems which don't support touch screens
}
/******************************************************************************/
bool Init()
{
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
   if(Kb.ctrl())Time.wait(100); // sleep so the screen won't update so frequently, and it's easier to notice the text
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(GREY);
   D.text (0, 0.9, S+"Active Touches: "+Touches.elms()); // draw amount of active touches
   D.text (0, 0.8, S+"Accelerometer: "+Accelerometer()); // draw accelerometer value
   D.text (0, 0.7, S+"Gyroscope: "    +    Gyroscope()); // draw gyroscope     value

   Str s;
   FREPA(Touches) // draw all touches
   {
      Touch &t=Touches[i];
      Circle(0.04, t.startPos()).draw(ColorAlpha(GREEN, 0.5)); // draw initial position
      Circle(0.04, t.pos     ()).draw(ColorAlpha(RED  , 0.5)); // draw current position

      // store information in text
      s+=S+"Pos: "  +t.pos      ()+" Delta: "+t.d   ()+"\n";
      s+=S+"Start: "+t.startTime()+" Life: " +t.life()+"\n";
      if(t.on())s+="ON,";
      if(t.pd())s+="PD,";
      if(t.rs())s+="RS,";
      if(t.db())s+="DB,";
      s+="\n";
      if(t.tapped())s+="TAPPED";
      s+="\n";
   }
   D.text(Rect(-D.w(), -D.h(), D.w(), 0.7), s);
   D.text(0, -0.9, "Hold Control to draw the text slower");
}
/******************************************************************************/
