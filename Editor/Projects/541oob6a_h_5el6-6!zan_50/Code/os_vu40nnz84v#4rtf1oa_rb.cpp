/******************************************************************************/
Game.ObjMap<Game.ObjParticles > ObjParticles;
Game.ObjMap<Game.ObjLightPoint> ObjLightPoints;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist =16;
   Cam.pitch=-0.5;
   Cam.at.set(16, 0, 16);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType(ObjParticles  , OBJ_PARTICLES)
             .setObjType(ObjLightPoints, OBJ_LIGHT_POINT)
             .New(UID(1231502889, 1242727556, 1117354931, 2573298583));
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

   Game.World.update(Cam.at);

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
