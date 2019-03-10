/******************************************************************************/
Actor            ground , // ground actor
                 box[3] ; // stairs
ObjectPtr        chr    ;
AnimatedSkeleton skel   ; // animated skeleton
Ragdoll          ragdoll; // ragdoll
/******************************************************************************/
void SetPose()
{
   skel.clear().updateMatrix(Matrix().setRotateX(0.45).move(Vec(0, 3, 0))).updateVelocities(); // set skeleton animation to default pose and custom matrix

   ragdoll.fromSkel(skel); // setup ragdoll from skeleton animation
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist =5;
   Cam.pitch=-0.3;
   Cam.yaw  =PI;
}
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   ground.create(Box_U(15, 1, 15, Vec(0, -1, 0)), 0);
   REPA(box)box[i].create(Box(1, Vec(0, i*0.5-0.5, i*-0.4-0.3)), 0);

   chr=UID(2919624831, 1261075521, 753053852, 3651670215); // load character
   if(!chr->mesh())Exit("Object has no mesh");
   skel.create(chr->mesh()->skeleton(), 1.7); // create controlled skeleton from skeleton, with 1.7 scale
   ragdoll.create(skel);                      // create ragdoll from skeleton

   SetPose();

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
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // move camera on right mouse button

   Physics.startSimulation().stopSimulation();

   ragdoll.toSkel(skel); // set skeleton from ragdoll

   if(Kb.bp(KB_SPACE))SetPose();

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         LightDir(Cam.matrix.z).add();

         chr->mesh()->draw(skel);
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   Renderer.setDepthForDebugDrawing();
   Physics.draw();

   D.text(0, 0.9, "Press Space to reset simulation");
}
/******************************************************************************/
