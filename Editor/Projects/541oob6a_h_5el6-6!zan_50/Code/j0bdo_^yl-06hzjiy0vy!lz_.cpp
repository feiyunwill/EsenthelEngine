/******************************************************************************/
ObjectPtr        chr;
AnimatedSkeleton skel;
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
   Cam.dist=1;
   Cam.yaw =PI;

   chr=UID(2919624831, 1261075521, 753053852, 3651670215); // load character
   if(!chr->mesh())Exit("Object has no mesh");
   skel.create(chr->mesh()->skeleton(), 1.0); // create animated skeleton from skeleton, with 1.0 scale
   Cam.at=chr->mesh()->ext.pos;
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
   Cam.transformByMouse(0.1, 10, CAMH_ROT|CAMH_ZOOM);

   // set animations
   {   
      skel.clear();      // clear skeleton animation
      if(Kb.b(KB_SPACE)) // when space pressed
      {
         skel.animate(UID(1746491013, 1251372253, 3930150308, 1129258799), Time.time()); // animate with "walk" animation and current time position
      }
      skel.updateMatrix    (MatrixIdentity); // update skeleton animation matrixes
      skel.updateVelocities(              ); // update skeleton bone velocities (this is needed for Motion Blur effect)
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
         LightDir(Vec(0, 0, -1), 1-D.ambientColor()).add(); // set light

         chr->mesh()->draw(skel); // draw mesh with skeleton matrixes
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   if(Kb.ctrl()) // when control pressed
   {
      D.clearDepth();
      SetMatrix();    // restore default matrix
      skel.draw(RED); // draw skeleton
   }

   D.text(0, 0.9, "Hold space to animate");
   D.text(0, 0.8, "Hold control to display animation skeleton");
}
/******************************************************************************/
