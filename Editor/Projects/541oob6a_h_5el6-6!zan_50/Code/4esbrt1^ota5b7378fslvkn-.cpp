/******************************************************************************/
Actor ground,
      box   ;
Joint joint ;
Vec   joint_pos,
      joint_dir;
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
   ground .create(Box(15, 1, 15, Vec(0, -2, 0)), 0);

   Box_D b(2, 2, 0.2);
   box.create(b); // create a box from 'b'

   joint_pos=b.cornerLDF(); // set joint position  to box left-down-front corner
   joint_dir.set(1, 0, 0);  // set joint direction to right
   joint.createSliding(box, null, joint_pos, joint_dir, 0, 1.5); // create a sliding joint in order to attach 'box' actor at 'joint_pos' world position, 'joint_dir' direction and 0 .. 1.5 moving limits

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

   if(Kb.b(KB_LEFT ))box.addVel(Vec(-Time.d(), 0, 0)); // add left    velocity
   if(Kb.b(KB_RIGHT))box.addVel(Vec( Time.d(), 0, 0)); // add right   velocity
   if(Kb.b(KB_UP   ))box.addVel(Vec(0, 0,  Time.d())); // add forward velocity which will not affect the actor
   if(Kb.b(KB_DOWN ))box.addVel(Vec(0, 0, -Time.d())); // add bacward velocity which will not affect the actor

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();
   
   SetMatrix();                                     // set identity matrix
   joint_pos.draw(RED);                             // draw joint position
   D.line(GREEN, joint_pos, joint_pos+joint_dir*3); // draw a line from joint position and it's direction

   D.text(0, 0.9, S+"Press Left / Right to change velocity");
   D.text(0, 0.8, S+"Press Up / Down to change velocity, the object won't move because of the joint");
}
/******************************************************************************/
