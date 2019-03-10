/******************************************************************************

   Here we'll present how to properly use different camera modes

/******************************************************************************/
// Define viewing modes:
enum VIEW_MODE // Viewing Mode
{
   VIEW_FPP, // First Person
   VIEW_TPP, // Third Person
   VIEW_ISO, // Isometric
   VIEW_NUM, // number of view modes
}
VIEW_MODE View=VIEW_TPP; // current VIEW_MODE
/******************************************************************************/
class Player : Game.Chr
{
   bool update()
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

         // mouse turn
         if(View!=VIEW_ISO) // don't use mouse turning when in Isometric mode
         {
            Flt max=DegToRad(900)*Time.d();
            angle.x-=Mid(Ms.d().x*1.7, -max, max);
            angle.y+=Mid(Ms.d().y*1.7, -max, max);
         }
      }

      return super.update();
   }

   virtual uint drawPrepare()
   {
      uint draw_mask=0xFFFFFFFF; // set all groups enabled by default
      bool hide_head=(View==VIEW_FPP && mesh); // disable drawing head when we're in FPP mode
      if(hide_head)FlagDisable(draw_mask, IndexToFlag(DG_CHR_HEAD)); // clear head draw group flag
      SetDrawMask(draw_mask); // set draw mask
      uint modes=super.drawPrepare(); // call default drawing after setting the mask
      SetDrawMask(); // reset default mask
      return modes;
   }
}
/******************************************************************************/
Game.ObjMap<Game.Item> Items;
Game.ObjMap<Player   > Players;
/******************************************************************************/
void InitPre()
{
   App.flag=APP_FULL_TOGGLE;
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.viewRange(30).shadowSoft(1);

   Cam.at.set(16, 0, 16);
   Cam.yaw   =-PI_4;
   Cam.pitch =-0.5;
   Cam.dist  =4;
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
void UpdateCamera()
{
   // set next camera mode when Tab pressed
   if(Kb.bp(KB_TAB))
   {
      View=VIEW_MODE((View+1)%VIEW_NUM);

      if(View==VIEW_ISO) // when set to isometric view
      {
         Cam.dist =   10; // set bigger camera distance at start
         Cam.pitch=-PI_4; // set starting camera pitch angle
      }
   }

   // setup the camera
   if(Players.elms()) // if we have at least one player
   {
      // set camera depending on current view mode
      switch(View)
      {
         case VIEW_FPP:
         {
          C OrientP *head=Players[0].skel.getSlot("head"); // obtain player "head" skeleton slot (this was created in Object Editor)
            Cam.setPosDir(head.pos, head.dir, head.perp); // set camera from 'head' position, direction and perpendicular to direction
         }break;

         case VIEW_TPP:
         {
            Cam.dist=Max(1.0, Cam.dist*ScaleFactor(Ms.wheel()*-0.1)); // update camera distance according to mouse wheel
            Cam.setSpherical(Players[0].ctrl.center()+Vec(0, 0.5, 0), Players[0].angle.x, Players[0].angle.y, 0, Cam.dist); // set spherical camera looking at player position with given player angles
         }break;

         default: // VIEW_ISO
         {
            Cam.yaw  -=Ms.d().x; // update camera yaw   angle according to mouse delta x
            Cam.pitch+=Ms.d().y; // update camera pitch angle according to mouse delta y
            Clamp(Cam.pitch, -PI_2, 0); // clamp to possible camera pitch angle
            Cam.dist  =Max(1.0, Cam.dist*ScaleFactor(Ms.wheel()*-0.1)); // update camera distance according to mouse wheel
            Cam.setSpherical(Players[0].pos(), Cam.yaw, Cam.pitch, 0, Cam.dist); // set spherical camera looking at player using camera angles
         }break;
      }

      // after setting camera position and angles:
      Cam.updateVelocities().set(); // update camera velocities and activate it
   }else // when no player on the scene
   {
      Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // default camera handling actions
   }
}
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Game.World.update(Cam.at);
   UpdateCamera();
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
   D.text  (0, 0.9, "Press Tab to switch camera modes");
}
/******************************************************************************/
