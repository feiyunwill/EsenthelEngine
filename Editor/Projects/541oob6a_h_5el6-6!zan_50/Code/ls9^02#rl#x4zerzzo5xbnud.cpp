/******************************************************************************/
Mesh mesh;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   Cam.dist=2;

   mesh.parts.New().base.create(Ball(0.25), VTX_TEX0|VTX_NRM|VTX_TAN);
   mesh.material(UID(2123216029, 1141820639, 615850919, 3316401700)).setRender().setBox();

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
   Cam.transformByMouse(1.5, 10, CAMH_ZOOM|CAMH_ROT);
   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         Matrix m; m.setPos(0, 0, Sin(Time.time()*2));
         if(Frustum(mesh, m))mesh.draw(m);

         LightDir(Vec(0, 0, 1)).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   
   Renderer.setDepthForDebugDrawing(); // make sure depth buffer is correct for custom rendering outside of 'Render' function
   D.textDepth(true, Cam.dist); // set depth of text as 'Cam.dist' from the camera
   D.text(0, 0, "3D Text");
   D.textDepth(false);
}
/******************************************************************************/
