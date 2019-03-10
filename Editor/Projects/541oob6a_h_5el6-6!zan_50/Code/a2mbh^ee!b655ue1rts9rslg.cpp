/******************************************************************************/
Actor ground,
      box   ,
      ball  ;
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

   // create physics
   Physics.create(EE_PHYSX_DLL_PATH); // create physics by specifying the path to physx dll's

   // create actors
   ground.create(Box (15, 1, 15, Vec(0.0, -2, 0)), 0); // create ground actor from Box and density=0 (which means it's a static actor - will not move)
   box   .create(Box (0.3      , Vec(0.1,  1, 0)));
   ball  .create(Ball(0.3      , Vec(0.0,  0, 0)));

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
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|CAMH_ROT);

   // physics update
   {
      Physics.startSimulation(); // start frame simulation

      // physics simulation takes some time, so you can do some calculations here
      // important : you may not modify actors between Physics.startSimulation() and Physics.stopSimulation()

      Physics.stopSimulation(); // get results of frame simulation
   }

   flt s=3;

   // add forces to ball
   if(Kb.b(KB_UP   ))ball.addForce(Vec( 0, 0,  1)*s);
   if(Kb.b(KB_DOWN ))ball.addForce(Vec( 0, 0, -1)*s);
   if(Kb.b(KB_LEFT ))ball.addForce(Vec(-1, 0,  0)*s);
   if(Kb.b(KB_RIGHT))ball.addForce(Vec( 1, 0,  0)*s);

   // add forces to ball according to camera
   if(Kb.b(KB_W))ball.addForce( !PointOnPlane(Cam.matrix.z, Vec(0, 1, 0))*s);
   if(Kb.b(KB_S))ball.addForce(-!PointOnPlane(Cam.matrix.z, Vec(0, 1, 0))*s);
   if(Kb.b(KB_A))ball.addForce(-!PointOnPlane(Cam.matrix.x, Vec(0, 1, 0))*s);
   if(Kb.b(KB_D))ball.addForce( !PointOnPlane(Cam.matrix.x, Vec(0, 1, 0))*s);

   // add velocity to ball
   if(Kb.bp(KB_SPACE))ball.addVel(Vec(0, 3, 0));
   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw (); // draw physical actors
}
/******************************************************************************/
