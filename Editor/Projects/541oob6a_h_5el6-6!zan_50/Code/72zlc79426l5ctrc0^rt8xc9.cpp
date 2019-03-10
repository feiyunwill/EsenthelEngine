/******************************************************************************/
Game.ObjMap<Game.Chr> Chrs;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.viewRange(30).shadowSoft(1);

   Cam.at.set(16, 0, 16);
   Cam.yaw   =-PI_4;
   Cam.pitch =-0.5;
   Cam.dist  =4;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Chrs, OBJ_CHR)
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
   Game.World.update(Cam.at);

   if(Chrs.elms())
   {
      Game.Chr &chr=Chrs[0];

      if(Kb.bp(KB_1))chr.ragdollEnable (); // switch to ragdoll
      if(Kb.bp(KB_2))chr.ragdollDisable(); // switch to skeleton animation

      if(Kb.bp(KB_Q))if(chr.ragdollBlend())if(Ragdoll.Bone *bone=chr.ragdoll.findBone("Head" ))bone.actor.addVel(Vec(0, 0, 3));
      if(Kb.bp(KB_W))if(chr.ragdollBlend())if(Ragdoll.Bone *bone=chr.ragdoll.findBone("Body" ))bone.actor.addVel(Vec(0, 0, 3));
      if(Kb.bp(KB_E))if(chr.ragdollBlend())if(Ragdoll.Bone *bone=chr.ragdoll.findBone("FootR"))bone.actor.addVel(Vec(0, 0, 4));
      if(Kb.bp(KB_R))if(chr.ragdollBlend())if(Ragdoll.Bone *bone=chr.ragdoll.findBone("HandR"))bone.actor.addVel(Vec(0, 0, 4));

      Cam.setSpherical(Cam.at, Cam.yaw-Ms.d().x, Cam.pitch+Ms.d().y, 0, Max(0.1, Cam.dist*ScaleFactor(Ms.wheel()*-0.2))).updateVelocities().set();
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

   if(Ms.b(0))
   {
      Renderer.setDepthForDebugDrawing();
      Physics.draw();
   }

   D.text(0, 0.9, "Press 1, 2 to switch between skeleton<->ragdoll animation");
   D.text(0, 0.8, "Press q, w, e, r to simulate hits");
}
/******************************************************************************/
