/******************************************************************************/
Actor ground,
      ball  ,
      actor ;
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

   Physics.create(EE_PHYSX_DLL_PATH); // create physics by specifying the path to physx dll's
   ground .create(Box (15, 1, 15, Vec(0, -2, 0)), 0);
   ball   .create(Ball(0.3, Vec(0, 1.3, 0)));

   ActorShapes shapes;                           // actor shapes
   shapes.add(Box (0.2                       )); // add box  to 'shapes'
   shapes.add(Ball(0.2,     Vec(-0.3, 0.2, 0))); // add ball to 'shapes'
   shapes.add(Ball(0.2,     Vec( 0.3, 0.2, 0))); // add ball to 'shapes'
   actor.create(shapes).pos(Vec( 0.0, 0.3, 0) ); // create actor from 'shapes'

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

   Physics.startSimulation().stopSimulation();

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();
}
/******************************************************************************/
