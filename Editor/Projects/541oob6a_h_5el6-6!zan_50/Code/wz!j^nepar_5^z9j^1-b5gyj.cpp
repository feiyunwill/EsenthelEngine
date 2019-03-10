/******************************************************************************

   Define your custom player character class basing on already defined Game.Chr
   Game.Chr is a class which handles the most basic character methods
   including : movement, animations, physics and actions

/******************************************************************************/
class Player : Game.Chr // extend character class by defining a player class based on the character
{
   virtual Bool update() // here we'll update the player (please note that this is a virtual method)
   {
      // here we update character input according to mouse and keyboard
      // before we set the input, we need to check if the character isn't controlled by an automatic action
      if(action)
      {
         // if it's controlled by an action we leave the input with no changes,
         // however we can optionally break that action, by pressing for example movement keys
         if(Kb.b(KB_W) || Kb.b(KB_S) || Kb.b(KB_A) || Kb.b(KB_D) || Kb.b(KB_Q) || Kb.b(KB_E))actionBreak();
      }

      if(!action) // if the character isn't controlled by an automatic action, we can set the input
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
         input.jump  =(Kb.bp(KB_SPACE) ? 3.5 : 0);

         // mouse turn
         Flt max=DegToRad(900)*Time.d();
         angle.x-=Mid(Ms.d().x*1.7, -max, max);
         angle.y+=Mid(Ms.d().y*1.7, -max, max);
      }

      return super.update(); // call Game.Chr.update on which Player is based on
   }
}
/******************************************************************************/
Actor  ground; // ground actor
Player player; // player
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   Cam.dist=2;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);
   ground .create(Box(15, 1, 15, Vec(0, -2, 0)), 0);

   player.create(*ObjectPtr(UID(2919624831, 1261075521, 753053852, 3651670215))); // create player from object parameters

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

   Physics.startSimulation().stopSimulation();

   player.update(); // update player

   Cam.setSpherical(player.ctrl.actor.pos()+Vec(0, 1, 0), player.angle.x, player.angle.y, 0, Cam.dist*ScaleFactor(Ms.wheel()*-0.2)).updateVelocities().set(); // update camera according to player angles and mouse wheel

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         player.drawPrepare();

         LightDir(!(Cam.matrix.x-Cam.matrix.y+Cam.matrix.z)).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   Renderer.setDepthForDebugDrawing();
   ground.draw(); // draw ground actor
}
/******************************************************************************/
