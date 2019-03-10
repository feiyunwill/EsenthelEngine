/******************************************************************************/
Game.ObjMap<Game.Item> Items; // container for item      objects
Game.ObjMap<Game.Chr > Chrs;  // container for character objects
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

   // set world active range to match the view range
   Game.World.activeRange(D.viewRange());

   // we need to tell the world 'which class handles which object type'
   // this is done by assigning memory containers to certain Object Types
   Game.World.setObjType(Items, OBJ_ITEM)  // set 'Items' memory container for 'OBJ_ITEM' objects
             .setObjType(Chrs , OBJ_CHR ); // set 'Chrs'  memory container for 'OBJ_CHR'  objects

   // now when the engine is set up properly we can start a 'new game' with a builded world
   Game.World.New(UID(4053788456, 1284500709, 3533893555, 3086486877)); // create the world by giving path to world

   if(Game.World.settings().environment) // if the world has environment settings (like sun, ambient, sky, ..)
      Game.World.settings().environment->set(); // then set them

   // when the world is set it doesn't actually load all the terrain and objects into memory
   // it loads only information about them
   // you need to tell the world which terrain and objects you need to use at the moment
   // to do that call:
   Game.World.update(Cam.at); // which updates world to use only terrain and objects at given position, here camera position is used

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del(); // worlds need to be manually deleted
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   Game.World.update(Cam.at); // update the world to given position

   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw(); // draw world (this is done outside of 'switch(Renderer())' because world automatically detects active rendering mode)
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
