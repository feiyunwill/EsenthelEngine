/******************************************************************************/
Actor ground,
      box;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
bool Init()
{
   Cam.dist=4;
   Physics.create(EE_PHYSX_DLL_PATH);

   ground.create(Box(15, 1, 15, Vec(0, -2, 0)), 0);
   box   .create(Box(1, Vec(0, 1, 0)));

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
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // move camera on right mouse button

   Physics.startSimulation().stopSimulation();

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();

   // test if a shape collides with some actors
   {
      Ball ball(0.7, Vec(Sin(Time.time())*3, 0, 0));
      bool collision=Physics.cuts(ball);

      SetMatrix(); // reset drawing matrix
      ball.draw(collision ? RED : BLUE);
   }
}
/******************************************************************************/
