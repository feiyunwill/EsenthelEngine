/******************************************************************************/
BackgroundLoader BL;
/******************************************************************************/
Game.ObjMap<Game.Chr > Chrs;
Game.ObjMap<Game.Item> Items;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   Cam.at.set(16, 0, 16);
   Cam.pitch=-0.5;
   Cam.dist=15;
   D.ambientPower(0.3);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Items, OBJ_ITEM)
             .setObjType (Chrs , OBJ_CHR )
             .New        (UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   BL.start(); // create background loader

   return true;
}
/******************************************************************************/
void Shut()
{
   BL.del(); // background loader must be manually deleted
   Game.World.del();
}
/******************************************************************************/
void SetCamera()
{
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
}
bool Update()
{
   if(BL.update())return true; // this needs to be at the start of 'Update' function

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
   if(BL.draw())return; // this needs to be at the start of 'Draw' function

   Renderer(Render);
}
/******************************************************************************/
