/******************************************************************************/
Vec2 force(0, 0);
/******************************************************************************/
void SetVibration(C Vec2 &new_force)
{
   if(Joypads.elms() && Joypads[0].supportsVibrations())
   {
      force=new_force;
      Clamp(force.x, -1, 1);
      Clamp(force.y, -1, 1);

      Joypads[0].vibration(force);
   }
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
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

   if(Kb.k(KB_LEFT ))SetVibration(force+Vec2(-0.2,  0  ));
   if(Kb.k(KB_RIGHT))SetVibration(force+Vec2( 0.2,  0  ));
   if(Kb.k(KB_DOWN ))SetVibration(force+Vec2( 0  , -0.2));
   if(Kb.k(KB_UP   ))SetVibration(force+Vec2( 0  ,  0.2));

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   
   if(!Joypads.elms()                 )D.text(0, 0, "No Joypads Detected");else
   if(!Joypads[0].supportsVibrations())D.text(0, 0, "Joypad #0 doesn't support Vibrations");else
   {
      D.text(0, 0.9, "Press Keyboard Arrow Keys to Change Applied Force");
      D.text(0, 0  , S+"Applied Force: "+force);
   }
}
/******************************************************************************/
