/******************************************************************************/
ImagePtr logo;
MeshPtr  mesh;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist=1;
   Cam.yaw=PI;

   D.ambientPower(0);
}
/******************************************************************************/
bool Init()
{
   logo=UID(1119600675, 1212460399, 80010661, 526665178);
   mesh=ObjectPtr(UID(2919624831, 1261075521, 753053852, 3651670215))->mesh();
   Cam.at=mesh->box.center();
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
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         mesh->draw(MatrixIdentity);

         LightPoint(5, Vec(0, 1, 1)).add();
      }break;
   }
}
void Draw()
{
   // setup background
   D.clear(WHITE);
   logo->drawFs();
   Rect_C(0, 0, D.w()*2, D.h()*2).draw(Color(255,255,255,192));

   // enable/disable combine mode before/after calling the renderer
   Renderer.combine=true ; Renderer(Render);
   Renderer.combine=false; 
}
/******************************************************************************/
