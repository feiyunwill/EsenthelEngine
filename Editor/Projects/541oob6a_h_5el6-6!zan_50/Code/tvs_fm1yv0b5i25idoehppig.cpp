/******************************************************************************/
Game.ObjMap<Item> Items;
UID               bullet_mtrl_id=UID(3025446042, 1259577522, 3090061708, 273635186);
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.3);

   Cam.at.set(12.3, 1, 12.5);
   Cam.dist = 0.01;
   Cam.pitch=-0.5;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Items, OBJ_ITEM)
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
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   // update the camera FPP 'WSAD' style
   if(Kb.b(KB_A     ))Cam.at-=Cam.matrix.x*Time.d()*2;
   if(Kb.b(KB_D     ))Cam.at+=Cam.matrix.x*Time.d()*2;
   if(Kb.b(KB_S     ))Cam.at-=Cam.matrix.z*Time.d()*2;
   if(Kb.b(KB_W     ))Cam.at+=Cam.matrix.z*Time.d()*2;
   if(Kb.b(KB_LSHIFT))Cam.at-=Vec(0, 1, 0)*Time.d()*2;
   if(Kb.b(KB_SPACE ))Cam.at+=Vec(0, 1, 0)*Time.d()*2;
   Cam.transformByMouse(0.01, 0.01, CAMH_ROT); // allow only rotation

   Game.World.update(Cam.at);

   if(Ms.bp(0)) // on LMB pressed
   {
      // calculate world position and direction vectors
      Vec     pos, dir; ScreenToPosDir(Vec2(0, 0), pos, dir);
      PhysHit phys_hit; if(Physics.ray(pos, dir*D.viewRange(), &phys_hit)) // if ray test hit an actor
      {
         if(phys_hit.obj) // if the actor comes from an object
         {
            if(Item *item=CAST(Item, (Game.Obj*)phys_hit.obj)) // if the object is an item
            {
               item->addBulletHole(phys_hit.plane.pos, phys_hit.plane.normal, dir); // call item method to add a bullet hole
            }
         }
         else // the actor doesn't have an object set, so most probably it's a terrain actor
         {
            // add a decal to the world terrain
            Game.World.terrainAddDecal(WHITE, bullet_mtrl_id, Matrix().setPosDir(phys_hit.plane.pos, phys_hit.plane.normal).scaleOrn(0.05));
         }
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
   D.text(0, 0.9, "Press LMB to shoot");

   // draw simple crosshair
   D.lineX(WHITE, 0, -0.08, 0.08);
   D.lineY(WHITE, 0, -0.08, 0.08);
}
/******************************************************************************/
