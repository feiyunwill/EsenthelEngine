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
   skel.create(chr->mesh()->skeleton(), 1.0); // create controlled skeleton from skeleton, with 1.0 scale
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
      skel.clear().animate(UID(1746491013, 1251372253, 3930150308, 1129258799), Time.time()); // set default walking animation

      Orient &head=skel.getBone("head").orn;            // get head bone orientation
      head*=Matrix3().setRotateZ(Sin(Time.time()*1.2)); // rotate head orientation according to time

      skel.updateMatrix(MatrixIdentity) // update all matrixes
          .updateVelocities();          // update all bone velocities
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
         LightDir(Vec(0, 0, -1), 1-D.ambientColor()).add();

         chr->mesh()->draw(skel);
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
