/******************************************************************************/
class Chr : Game.Chr
{
   virtual uint drawPrepare() // extend drawing to include requesting for RM_BEHIND mode
   {
      return super.drawPrepare() | IndexToFlag(RM_BEHIND);
   }
   virtual void drawBehind() // extend drawing to include rendering "behind effect"
   {
      if(mesh)if(Frustum(Ball().setAnimated(mesh->ext, skel)))
      {
         SetBehindBias(mesh->ext.h()*skel.scale());
         mesh->drawBehind(Color(64, 128, 255, 255), Color(255, 255, 255, 0), skel);
      }
   }
}
/******************************************************************************/
Game.ObjMap<      Chr> Chrs;
Game.ObjMap<Game.Item> Items;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   Cam.dist=3;
   D.ambientPower(0.3);
   D.mode(App.desktopW()*0.8, App.desktopH()*0.8);
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
   if(Chrs.elms())
   {
      Cam.setSpherical(Chrs[0].ctrl.center(), Time.time(), 0, 0, 17).updateVelocities().set();
   }
   else
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
