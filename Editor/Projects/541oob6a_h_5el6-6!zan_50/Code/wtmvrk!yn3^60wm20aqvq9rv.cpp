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

         // mouse turn
         flt max=DegToRad(900)*Time.d();
         angle.x-=Mid(Ms.d().x*1.7, -max, max);
         angle.y+=Mid(Ms.d().y*1.7, -max, max);
      }

      return super.update();
   }

   virtual uint drawPrepare() // extend drawing to include requesting for RM_OVERLAY mode
   {
      return super.drawPrepare() | IndexToFlag(RM_OVERLAY);
   }
   virtual void drawOverlay() // extend drawing to include rendering shadow
   {
      blob_shadow.matrix.setPosDir(ctrl.center()-Vec(0, ctrl.height()/2, 0), Vec(0, 1, 0)).scaleOrn(0.9);
      blob_shadow.drawStatic(0.4);
   }
}
/******************************************************************************/
class Item : Game.Item
{
   virtual uint drawPrepare() // extend drawing to include requesting for RM_OVERLAY mode
   {
      return super.drawPrepare() | IndexToFlag(RM_OVERLAY);
   }
   virtual void drawOverlay() // extend drawing to include rendering shadow
   {
      if(mesh)
      {
         Box box=mesh->box*matrix();
         blob_shadow.matrix.setPosDir(box.down(), Vec(0, 1, 0)).scaleOrn(Avg(box.w(), box.d())*0.7);
         blob_shadow.drawStatic(0.4);
      }
   }
}
/******************************************************************************/
Game.ObjMap<Item  > Items;
Game.ObjMap<Player> Players;

Decal blob_shadow; // decal used for drawing the fast shadow
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   Cam.dist=3;
   D.ambientPower(0.3);
   D.shadowMode(SHADOW_NONE); // disable high quality shadows
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType(Players, OBJ_CHR)
             .setObjType(Items  , OBJ_ITEM)
             .New(UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   // setup blob shadow decal
   blob_shadow.terrain_only=true;
   blob_shadow.material(UID(2583141124, 1087004963, 3177178549, 2752130458));

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
void SetCamera()
{
   if(Players.elms())
   {
      Cam.dist=Max(1.0, Cam.dist*ScaleFactor(Ms.wheel()*-0.1));
      Cam.setSpherical(Players[0].pos()+Vec(0, 0.5, 0), Players[0].angle.x, Players[0].angle.y, 0, Cam.dist);
      Cam.updateVelocities().set();
   }else
   {
      Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
   }
}
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Game.World.update(Cam.at);

   SetCamera();

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
}
/******************************************************************************/
