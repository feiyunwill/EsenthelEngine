/******************************************************************************/
class Player
{
   Ball             bounding_ball(0); // bounding ball covering the character
   MeshPtr          body            , // main body mesh
                    armor           , // armor     mesh
                    pants           , // pants     mesh
                    boots           , // boots     mesh
                    gauntlets       ; // gauntlets mesh
   AnimatedSkeleton skel            ; // animated skeleton

   // update
   void update()
   {
      // set skeleton animation
      skel.clear()
          .animate(UID(3342756396, 1285211226, 524185523, 499551746)  , Time.time()) // animate with "left fist" animation
          .animate(UID(2565101787, 1333856922, 4140992683, 1310585985), Time.time()) // animate with "right fist" animation
          .animate(UID(1746491013, 1251372253, 3930150308, 1129258799), Time.time()) // animate with "walk" animation
          .updateMatrix(MatrixIdentity)
          .updateVelocities();

      // set bounding ball
      if(body)bounding_ball.setAnimated(body->box, skel);else bounding_ball.set(2, skel.pos());
   }

   // draw
   void drawPrepare()
   {
      if(Frustum(bounding_ball)) // if player is visible in frustum
      {
         // add the meshes to the draw list
         if(armor    )armor    ->draw(skel);
         if(pants    )pants    ->draw(skel);
         if(boots    )boots    ->draw(skel);
         if(gauntlets)gauntlets->draw(skel);
         if(body     )body     ->draw(skel);
      }
   }
   void drawShadow()
   {
      if(Frustum(bounding_ball)) // if player is visible in current shadow frustum
      {
         // add the meshes to the shadow draw list
         if(armor    )armor    ->drawShadow(skel);
         if(pants    )pants    ->drawShadow(skel);
         if(boots    )boots    ->drawShadow(skel);
         if(gauntlets)gauntlets->drawShadow(skel);
         if(body     )body     ->drawShadow(skel);
      }
   }

   void createDefault()
   {
      // set meshes
      body     =ObjectPtr(UID(2919624831, 1261075521, 753053852, 3651670215))->mesh();
      armor    =ObjectPtr(UID(2031909702, 1196392633, 2341974683, 615558807))->mesh();
      pants    =ObjectPtr(UID(4099872651, 1316584110, 3048579493, 1166951837))->mesh();
      boots    =ObjectPtr(UID(3279260457, 1135538771, 751762862, 1888362313))->mesh();
      gauntlets=ObjectPtr(UID(3702527436, 1166112370, 2441684388, 556763633))->mesh();

      // create skeleton and set initial matrixes
      skel.create(body->skeleton());
   }
}
Player player;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   D.ambientPower(0.3)
    .shadowJitter(true).shadowSoft(1).shadowMapSize(2048)
    .viewFrom(0.005).viewRange(5);

   Cam.yaw  = 2.7;
   Cam.pitch=-0.3;
   Cam.dist = 1.2;
}
/******************************************************************************/
bool Init()
{
   // initialize sky & sun
   Sky.atmospheric();
   Sun.image=UID(1275694243, 1199742097, 1108828586, 1055787228);
   Sun.pos=!Vec(1, 1, 1);
   Sun.light_color=1-D.ambientColor();

   // setup player data
   player.createDefault();

   if(player.body)Cam.at=player.body->box.center(); // adjust camera to mesh center

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.01, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   player.update();

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE: player.drawPrepare(); break;
      case RM_SHADOW : player.drawShadow (); break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
