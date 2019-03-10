/******************************************************************************

   Here we'll present how to detect camera collisions

/******************************************************************************/
class Player : Game.Chr
{
   virtual bool update()
   {
      if(action)
      {
         if(Kb.b(KB_W) || Kb.b(KB_S) || Kb.b(KB_A) || Kb.b(KB_D) || Kb.b(KB_Q) || Kb.b(KB_E))actionBreak();
      }

      if(!action)
      {
         // turn & move
         input.turn.x=Kb.b(KB_Q)-Kb.b(KB_E);
         input.turn.y=Kb.b(KB_T)-Kb.b(KB_G);
         input.move.x=Kb.b(KB_D)-Kb.b(KB_A);
         input.move.z=Kb.b(KB_W)-Kb.b(KB_S);
         input.move.y=Kb.b(KB_SPACE)-Kb.b(KB_LSHIFT);

         // dodge, crouch, walk, jump
         input.dodge = Kb.bd(KB_D)-Kb.bd(KB_A);
         input.crouch= Kb.b (KB_LSHIFT);
         input.walk  = Kb.b (KB_LCTRL );
         input.jump  =(Kb.bp(KB_SPACE ) ? 3.5 : 0);
      }

      return super.update();
   }
}
/******************************************************************************/
Camera desired_camera; // create a helper desired camera

Game.ObjMap<Player> Players;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.viewRange(30).shadowSoft(1);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Players, OBJ_CHR)
             .New        (UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Game.World.update(Cam.at);

   // setup the camera
   if(Players.elms()) // if we have at least one player
   {
      // first setup the desired camera as in the previous tutorials
      desired_camera.yaw  -=Ms.d().x;           // update camera yaw   angle according to mouse delta x
      desired_camera.pitch+=Ms.d().y;           // update camera pitch angle according to mouse delta y
      Clamp(desired_camera.pitch, -PI_2, PI_4); // clamp to possible camera pitch angle
      desired_camera.dist=Max(1.0, desired_camera.dist*ScaleFactor(Ms.wheel()*-0.2)); // update camera distance according to mouse wheel
      desired_camera.at  =Players[0].ctrl.center();
      desired_camera.setSpherical(); // set as spherical from current values, this will set the camera's matrix (desired_camera.matrix)

      // now what we'll do is cast a small sized Ball from starting position to target camera destination
      // we'll stop the ball at first contact point, and set camera at that place

      // create a helper ball which will be used for collision detection
      Ball ball(0.1, desired_camera.at); // we place it at starting point (where the camera is looking at)

      // now we'll move the ball in the direction where the camera should be
      Physics.move(ball, desired_camera.matrix.pos-ball.pos, ~IndexToFlag(AG_CONTROLLER)); // use physics movement to move the ball as far as it can go without any collisions (ignore collisions with controllers)

      // now the ball.pos is located at either maximum movement distance or at nearest collision point
      // having ball's position we can now set the final camera position
      Cam.setPosDir(ball.pos, desired_camera.matrix.z, desired_camera.matrix.y); // we'll use 'desired_camera.matrix' directions which were set in 'setSpherical' camera method

      Cam.updateVelocities().set(); // update camera velocities and activate it
   }
   else // when no player on the scene
   {
      Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // default camera handling actions
   }

   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw();
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, "Try to move the camera around");
   D.text(0, 0.8, "and see how it gets blocked by the terrain");
}
/******************************************************************************/
