/******************************************************************************/
ObjectPtr        chr;
AnimatedSkeleton skel;
Vec              pos;
flt              angle;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.1);
}
/******************************************************************************/
bool Init()
{
   Cam.dist =8;
   Cam.yaw  = PI;
   Cam.pitch=-PI_6;
   D.viewFov(DegToRad(30));

   chr=UID(2919624831, 1261075521, 753053852, 3651670215); if(!chr->mesh())Exit("Object has no mesh");
   skel.create(chr->mesh()->skeleton(), 1.0);
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
   Cam.transformByMouse(0.1, 100, CAMH_ROT|CAMH_ZOOM);

   // set animations
   {   
      skel.clear(); // clear skeleton animation

      // move forward
      if(Kb.b(KB_W))
      {
         if(SkelAnim *skel_anim=skel.getSkelAnim(UID(962456806, 1119456482, 1164047292, 3485233072)))
         {
            // apply animation to the skeleton
            skel.animate(*skel_anim, Time.time());
            
            // apply transform to the player position
            RevMatrix transform; skel_anim.animation().getRootTransform(transform, Time.time()-Time.d(), Time.time()); // get root transform between last and current frame
         #if 0 // full formula
            Matrix m; m.setRotateY(angle).move(pos); m*=transform; pos=m.pos;
         #else // simplified formula
            pos+=transform.pos*Matrix3().setRotateY(angle);
         #endif
         }
      }

      // turn left/right
      if(Kb.b(KB_Q))angle-=Time.ad()*2.5;
      if(Kb.b(KB_E))angle+=Time.ad()*2.5;

      // construct matrix
      Matrix m; m.setRotateY(angle).move(pos);

      // finalize
      skel.updateMatrix(m).updateVelocities();
   }

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         LightDir(ActiveCam.matrix.z, 1-D.ambientColor()).add(); // set light

         chr->mesh()->draw(skel); // draw mesh with controlled skeleton matrixes
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   Renderer.setDepthForDebugDrawing();
   SetMatrix();
   Plane(0, Vec(0, 1, 0)).drawInfiniteByResolution(GREEN, 96);

   D.text(0, 0.9, "Hold W to move forward");
   D.text(0, 0.8, "Hold Q E to turn left/right");
}
/******************************************************************************/
