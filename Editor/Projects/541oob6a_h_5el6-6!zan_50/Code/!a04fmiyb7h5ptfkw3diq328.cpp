/******************************************************************************

   Item slots are special points attached to skeletons.

   They can be used to render items in correct places.

   Skeleton slots can be accessed after animating skeleton through 'AnimatedSkeleton.getSlot' method,
   which returns reference to OrientP class which is an orientation and position.

/******************************************************************************/
ObjectPtr        chr, item;
AnimatedSkeleton skel;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.ambientPower(0.1);
   Ms.hide();
   Ms.clip(null, true);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=1;
   Cam.yaw =PI;

   chr =UID(2919624831, 1261075521, 753053852, 3651670215); // load character
   item=UID(3865949516, 1110691029, 816879537, 746462228); // load item
   if(!chr ->mesh())Exit("Object has no mesh");
   if(!item->mesh())Exit("Object has no mesh");
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
   skel.clear().animate(UID(1746491013, 1251372253, 3930150308, 1129258799), Time.time()).updateMatrix(MatrixIdentity).updateVelocities();

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

         // draw mesh
         chr->mesh()->draw(skel);

         // draw item in right hand
         {
          C OrientP *hand_r=skel.getSlot("HandR");
            Matrix   m;
            m.setPosDir(hand_r.pos, hand_r.perp, hand_r.dir) // set position and directions according to skeleton slot
             .scaleOrn(0.35f);                               // scale down the matrix orientation a little, making the item smaller
            item->mesh()->draw(m);                           // render item with matrix
         }
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   // draw skeleton slots
   D.clearDepth();
   SetMatrix();
   skel.getSlot("HandR").draw(RED);
   skel.getSlot("HandL").draw(RED);
   skel.getSlot("Head" ).draw(RED);
}
/******************************************************************************/
