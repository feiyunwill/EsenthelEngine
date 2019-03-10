/******************************************************************************/
Actor ground,
      ball,
      box;
PhysMtrl bouncy;
bool triggered;
/******************************************************************************/
void ReportTrigger(ActorInfo &trigger, ActorInfo &actor, PHYS_CONTACT contact)
{
   triggered=true;
}
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

   bouncy.create().bounciness(1).bouncinessMode(PhysMtrl.MODE_MAX).damping(0);

   ground.create(Box(15, 1, 15, Vec(0, -3, 0)), 0);

   ball.create(Ball(0.5, Vec(0, 2, 0))).material(&bouncy); // adjust material to bouncy

   box.create(Box(0.5, Vec(0, 0, 0))).trigger(true).gravity(false); // set box as trigger

   Physics.reportTrigger(ReportTrigger); // set callback function

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

   triggered=false; // set no trigger before the simulation
   Physics.startSimulation().stopSimulation();

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();
   if(triggered)D.text(0, 0.9, "triggered");
}
/******************************************************************************/
