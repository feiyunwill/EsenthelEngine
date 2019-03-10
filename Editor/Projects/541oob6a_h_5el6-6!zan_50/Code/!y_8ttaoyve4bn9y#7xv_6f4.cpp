/******************************************************************************/
Actor ground,
      object;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, true);
}
bool Init()
{
   Cam.dist=8;
   Physics.create(EE_PHYSX_DLL_PATH);

   ground.create(Box (8, 1, 8, Vec(0, -2, 0)), 0);
   object.create(Ball(1, Vec(0, 1, 0)));

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
   Cam.transformByMouse(0.1, 30, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // move camera on right mouse button

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
      Ball    ball(0.7, Vec(0, 3, 0));
      Vec     move(Sin(Time.time())*3, -3, 0);
      PhysHit phys_hit;
      bool    collision=Physics.sweep(ball, move, &phys_hit);
      
      SetMatrix(); // reset drawing matrix
      
      ball.draw(YELLOW); // draw ball at original position

      (ball+move).draw(GREY); // draw ball at desired position

      if(collision)move*=phys_hit.frac; // limit movement if collision encountered

      D.line(WHITE, ball.pos, ball.pos+move); // draw the movement line

      (ball+move).draw(collision ? RED : BLUE); // draw ball at destination position
   }
}
/******************************************************************************/
