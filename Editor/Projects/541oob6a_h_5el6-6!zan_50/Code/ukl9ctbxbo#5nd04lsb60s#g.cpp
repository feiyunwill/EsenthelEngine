/******************************************************************************/
Actor ground,
      ball  ,
      box   ;
Joint joint     ; // joint, used to connect two actors together, or attach one to a fixed point in the world
Vec   joint_pos , // joint position (used only for drawing)
      joint_axis; // joint axis     (used only for drawing)
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
bool Init()
{
   Cam.dist =4;
   Cam.pitch=-0.2;

   Physics.create(EE_PHYSX_DLL_PATH);
   ground .create(Box_U(15, 1, 15, Vec(0,0,0)), 0);
   ball   .create(Ball (0.3, Vec(-1, 0.3, 0))).vel(Vec(5, 0, 0)); // create a ball on ground and set its initial velocity to right

   Box_D b(0.1, 2, 2, Vec(2, 0, 0));
   box.create(b).ignore(ground); // create a box from 'b' and ignore collisions with 'ground' actor

   joint_pos=b.cornerLDF(); // set joint position to box left-down-front corner
   joint_axis.set(0, 1, 0); // set joint axis     to up
   joint.createHinge(box, null, joint_pos, joint_axis); // create a hinge joint in order to attach 'box' actor at 'joint_pos' world position and 'joint_axis' axis

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
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   Physics.startSimulation().stopSimulation();

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();

   joint_pos.draw(RED);                              // draw joint position
   D.line(GREEN, joint_pos, joint_pos+joint_axis*3); // draw a line from joint position and it's axis
}
/******************************************************************************/
