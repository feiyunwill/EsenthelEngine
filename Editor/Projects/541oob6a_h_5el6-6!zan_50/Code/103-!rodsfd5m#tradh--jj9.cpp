/******************************************************************************/
Game.ObjMap<Game.Item> Items;

File SaveGame; // file used to store world state
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist =10;
   Cam.yaw  =-PI_4;
   Cam.pitch=-0.5;
   Cam.at.set(16, 0, 16);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType(Items, OBJ_ITEM)
             .New(UID(3829896559, 1088474000, 4168763052, 388400472));
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
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   if(Kb.bp(KB_F2))                  Game.World.save(SaveGame.writeMem());  // save game to file in memory
   if(Kb.bp(KB_F3)){SaveGame.pos(0); Game.World.load(SaveGame           );} // reset file position and load game from it

   Game.World.update(Cam.at);

   // add some velocity to objects
   if(Kb.bp(KB_SPACE))REPA(Items)Items[i].actor.addVel(5*Random.dir(Vec(0, 1, 0), 0.7));

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
   D.text(0, 0.9, "Press F2 to save and F3 to load");
   D.text(0, 0.8, "Press Space to add velocity");
}
/******************************************************************************/
