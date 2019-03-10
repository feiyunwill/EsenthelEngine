/******************************************************************************/
DestructMesh destruct; // since currently there is no way to create 'DestructMesh' from the Editor, we need to create it manually in codes
File         SaveGame;
/******************************************************************************/
class Destructible : Game.Destructible
{
   virtual void setUnsavedParams()
   {
      destruct_mesh=&destruct; // set which destructible mesh we want to use
      super.setUnsavedParams();
   }
   virtual void create(Object &obj)
   {
      super.create(obj);
      toStatic(); // default to static mode at the start
   }
}
Game.ObjMap<Destructible> Destructibles;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Physics.create(EE_PHYSX_DLL_PATH);

   Cam.dist =10;
   Cam.yaw  =-PI_4;
   Cam.pitch=-0.5;
   Cam.at.set(16, 0, 16);
}
/******************************************************************************/
bool Init()
{
   // create a destruct mesh
   Object obj;
   obj.base(UID(1134879343, 1157937325, 4232119955, 3140023117));
   obj.type(true, ObjType.elmID(OBJ_DESTRUCTIBLE)); // override type to set DESTRUCTIBLE class
   if(!obj.mesh())Exit("object has no mesh");
   destruct.create(*obj.mesh(), 8, UID(2519161163, 1078307672, 184317862, 2816697732), 1, 32);

   // load world
   Game.World.activeRange(D.viewRange())
             .setObjType(Destructibles, OBJ_DESTRUCTIBLE)
             .New(UID(3692432343, 1256681024, 2863582642, 4182715660));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   Game.World.objCreate(obj, Matrix().setPos(Cam.at));

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

   if(Kb.bp(KB_F2))                  Game.World.save(SaveGame.writeMem());  // save game to file in memory
   if(Kb.bp(KB_F3)){SaveGame.pos(0); Game.World.load(SaveGame           );} // reset file position and load game from it

   Cam.transformByMouse(0.01, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
   Game.World.update(Cam.at);

   if(Kb.bp(KB_SPACE))
   {
      REPA(Destructibles)Destructibles[i].toPieces(); // first convert all destructible objects into pieces, this may create more objects in the world
      REPA(Destructibles) // now adjust velocities to all objects
      {
         Destructible &d=Destructibles[i];
         REPAO(d.actors).vel(Random(Ball(12)));
      }
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
   if(Kb.b(KB_LCTRL))
   {
      Renderer.setDepthForDebugDrawing();
      Physics.draw();
   }
   D.text(0, 0.9, "Press Space to Explode");
}
/******************************************************************************/
