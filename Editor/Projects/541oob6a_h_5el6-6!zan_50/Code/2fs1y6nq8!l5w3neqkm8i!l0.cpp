/******************************************************************************/
Actor      ground   ,
           actor[10];
Controller ctrl     ; // controller - it's meant to be used as a 'character actor'
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
bool Init()
{
   Cam    .dist=4;
   Physics.create(EE_PHYSX_DLL_PATH);
   ground .create(Box(15, 1, 15, Vec(0, -2, 0)), 0);

   // create random actors
   REPA(actor)switch(Random(3))
   {
      case 0: actor[i].create(Box    (RandomF(0.1, 0.5),                    Random(Box(10, 1, 10)))); break;
      case 1: actor[i].create(Ball   (RandomF(0.1, 0.5),                    Random(Box(10, 1, 10)))); break;
      case 2: actor[i].create(Capsule(RandomF(0.1, 0.2), RandomF(0.5, 1.0), Random(Box(10, 1, 10)))); break;
   }

   // create controller
   ctrl.create(Capsule(0.4, 1.7));
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

   // update controller (move on WSAD keys, crouch on Shift key, jump on Space key)
   Flt s=3;
   Vec vel(0, 0, 0);
   if(Kb.b(KB_W))vel+=!PointOnPlane(Cam.matrix.z, Vec(0, 1, 0))*s;
   if(Kb.b(KB_S))vel-=!PointOnPlane(Cam.matrix.z, Vec(0, 1, 0))*s;
   if(Kb.b(KB_A))vel-=!PointOnPlane(Cam.matrix.x, Vec(0, 1, 0))*s;
   if(Kb.b(KB_D))vel+=!PointOnPlane(Cam.matrix.x, Vec(0, 1, 0))*s;
   ctrl.update(vel, Kb.shift(), Kb.bp(KB_SPACE) ? 3.5 : 0);

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();
}
/******************************************************************************/
