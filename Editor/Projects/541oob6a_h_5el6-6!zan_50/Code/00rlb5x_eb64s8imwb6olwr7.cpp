/******************************************************************************/
Game.ObjMap<Game.Chr> Chrs;

Decal decal; // decal pointing character destination target
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.viewRange(50);
   Cam.dist = 10;
   Cam.yaw  =-PI_4;
   Cam.pitch=-PI_3;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   decal.terrain_only=true;
   decal.color.set(1, 1, 0, 1);
   decal.material(UID(204785467, 1276492407, 1878381189, 3934085962));

   Game.World.activeRange(D.viewRange())
             .setObjType (Chrs, OBJ_CHR)
             .New        (UID(3199326727, 1331546700, 2335466931, 1420158981)) // load "path" world
             .update     (Cam.at);
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

   // move player
   if(Ms.bp(0) && Chrs.elms()) // on LMB pressed
   {
      Vec     pos, dir; ScreenToPosDir(Ms.pos(), pos, dir); // convert screen mouse position to world position and direction
      PhysHit phys_hit;
      if(Physics.ray(pos, dir*D.viewRange(), &phys_hit)) // if ray-test hit something
      {
         Chrs[0].actionMoveTo(phys_hit.plane.pos); // order character to move to that location

         decal.matrix.setPosDir(phys_hit.plane.pos, Vec(0, 1, 0));
      }
   }

   // rotate camera
   if(Ms.b(1))
   {
      Cam.yaw  -=Ms.d().x;
      Cam.pitch+=Ms.d().y;
   }
   if(Chrs.elms())Cam.setSpherical(Chrs[0].pos(), Cam.yaw, Cam.pitch, 0, Cam.dist*ScaleFactor(Ms.wheel()*-0.2)).updateVelocities().set();

   // rotate decal around its z axis
   decal.matrix.rotateZL(Time.d());

   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw();
   
   switch(Renderer())
   {
      case RM_BLEND:
      {
         if(Chrs.elms())if(Chrs[0].action==Game.ACTION_MOVE_TO)decal.drawStatic();
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   // show world path
   if(Kb.b(KB_SPACE))Game.World.path().draw(64, 0.5); // if space pressed

   // show player path
   if(Chrs.elms())
   {
      Memc<Vec> &path=Chrs[0].path; if(path.elms())
      {
         D.clearDepth();
         SetMatrix();
         VI.color(YELLOW); REPA(path         )VI.dot (path[i]           ); VI.end();
         VI.color(YELLOW); REP (path.elms()-1)VI.line(path[i], path[i+1]); VI.end();
      }
   }

   // informations
   D.text(0, 0.9, "Press LMB to move player, RMB to rotate camera");
   D.text(0, 0.8, "Press Space to show World Path Meshes");
}
/******************************************************************************/
