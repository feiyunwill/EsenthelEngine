/******************************************************************************

   This tutorial shows how to control which Mesh Parts are displayed on the screen.

/******************************************************************************/
MeshPtr mesh;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_MAXIMIZABLE|APP_MINIMIZABLE;
   Ms.hide();
   Ms.clip(null, 1);
   Sky.atmospheric();
   Cam.at.set(0, 1, 0);
   Cam.yaw=PI;
}
/******************************************************************************/
bool Init()
{
   mesh=ObjectPtr(UID(2919624831, 1261075521, 753053852, 3651670215))->mesh(); // load warrior mesh
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
   Cam.transformByMouse(0.01, 50, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT));
   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         if(mesh)
         {
            SetDrawMask(Kb.b(KB_SPACE) ? IndexToFlag(DG_CHR_HEAD) : IndexToFlag(DG_CHR_DEFAULT)); // allow drawing only head or default group
            mesh->draw(MatrixIdentity); // draw mesh after setting the mask
            SetDrawMask(); // reset default draw mask
         }

         LightDir(Cam.matrix.z, 1-D.ambientColor()).add();
      }break;
   }
}
/******************************************************************************/
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, "Hold Space to change which parts are drawn");
}
/******************************************************************************/
