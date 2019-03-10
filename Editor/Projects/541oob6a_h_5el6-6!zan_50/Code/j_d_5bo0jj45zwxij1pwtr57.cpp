/******************************************************************************/
Game.Obj           *Lit=null; // highlighted world object
Game.ObjMap<Item  > Items   ;
Game.ObjMap<Player> Players ;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.viewRange(70).mode(App.desktopW()*0.8, App.desktopH()*0.8);
   Cam.at.set(16, 0, 16);
   Cam.dist = 10;
   Cam.pitch=-PI_6;
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

   InvGui.create();

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
void GetWorldObjectUnderCursor()
{
   Lit=null;
   if(!Gui.ms() || Gui.ms()==Gui.desktop())
   {
      Vec     pos, dir; ScreenToPosDir(Ms.pos(), pos, dir);
      PhysHit phys_hit; if(Physics.ray(pos, dir*D.viewRange(), &phys_hit, ~IndexToFlag(AG_CONTROLLER)))Lit=(Game.Obj*)phys_hit.obj;
   }
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   if(Players.elms())Cam.at=Players[0].ctrl.center();
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_ROT:0));

   if(Kb.bp(KB_F2))Game.World.save("save.sav");
   if(Kb.bp(KB_F3))Game.World.load("save.sav");

   Game.World.update(Cam.at);
   Gui.update();
   GetWorldObjectUnderCursor();

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
   D.text(0, 0.9, "Click on the club to pick it up");
   Gui   .draw();
   InvGui.draw();
}
/******************************************************************************/
