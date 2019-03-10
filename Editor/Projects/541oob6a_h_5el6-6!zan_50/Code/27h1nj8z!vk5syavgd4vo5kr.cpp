/******************************************************************************/
ObjectPtr        chr;
AnimatedSkeleton skel;
flt              blend; // blending value (0..1)
bool             walk ; // if want to walk
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
   skel.create(chr->mesh()->skeleton(), 1.0); // create controlled skeleton from skeleton, with 1.0 scale
   Cam.at=chr->mesh()->ext.center;
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
   if(Kb.bp(KB_SPACE))walk^=1; // change 'walk' when space pressed

   // adjust blend value
   if(walk)
   {
      blend+=Time.d()*2;
      if(blend>1)blend=1;
   }else
   {
      blend-=Time.d()*2;
      if(blend<0)blend=0;
   }

   // set animations
   {   
      skel.clear  (); // clear controlled skeleton animation
      skel.animate(UID(1746491013, 1251372253, 3930150308, 1129258799), Time.time(),   blend); // animate with "walk" animation, current time position and "  blend" blending weight
      skel.animate(UID(3451023509, 1199057736, 3840847270, 4048373429), Time.time(), 1-blend); // animate with "run"  animation, current time position and "1-blend" blending weight
      skel.updateMatrix(MatrixIdentity).updateVelocities(); // update controlled skeleton animation and velocities
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
         chr->mesh()->draw(skel); // get mesh from cache and render it with controlled skeleton

         LightDir(Vec(0, 0, -1), 1-D.ambientColor()).add();
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
      skel.draw(RED); // draw controlled skeleton
   }

   D.text(0, 0.9, S+"Press space to toggle blending (Walk: "+walk+", Blend: "+blend+')');
   D.text(0, 0.8, "Hold control to display animation skeleton");
}
/******************************************************************************/
