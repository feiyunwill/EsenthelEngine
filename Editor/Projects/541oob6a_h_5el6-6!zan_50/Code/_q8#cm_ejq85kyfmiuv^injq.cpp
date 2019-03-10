/******************************************************************************/
Game.ObjMap<Player   > Players;
Game.ObjMap<Game.Item> Items;
SaveGame               SG;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.viewRange(70);
   Cam.dist=3;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Items  , OBJ_ITEM)
             .setObjType (Players, OBJ_CHR )
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
   if(Players.elms())
   {
      Cam.dist=Max(1.0, Cam.dist*ScaleFactor(Ms.wheel()*-0.1));
      Cam.setSpherical(Players[0].ctrl.center()+Vec(0, 0.5, 0), Players[0].angle.x, Players[0].angle.y, 0, Cam.dist);
      Cam.updateVelocities().set();
   }
   else // when no player on the scene
   {
      Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // default camera handling actions
   }
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   // change worlds on key press
   if(Kb.bp(KB_1))SG.changeWorld(UID(3692432343, 1256681024, 2863582642, 4182715660));
   if(Kb.bp(KB_2))SG.changeWorld(UID(4053788456, 1284500709, 3533893555, 3086486877));

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
   D.text(0, 0.9, "Press 1, 2 keys to change worlds");
   D.text(0, 0.8, S+"Current World: \""+Game.World.name()+'"');
}
/******************************************************************************/
