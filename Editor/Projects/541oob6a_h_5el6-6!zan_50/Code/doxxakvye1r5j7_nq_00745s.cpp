/******************************************************************************/
Mesh     mesh  ; // sample mesh
ImagePtr skybox; // sky box texture
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=6;

   // create mesh
   mesh.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN);
   mesh.material(UID(2123216029, 1141820639, 615850919, 3316401700)).setRender().setBox();

   // load skybox
   skybox=UID(2000723734, 1340256668, 421298842, 213478118);

   // set default sky
   Sky.atmospheric();

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
   Cam.transformByMouse(2.5, 18, CAMH_ZOOM|CAMH_ROT);

   if(Kb.bp(KB_1))Sky.clear      (      ); // disable sky
   if(Kb.bp(KB_2))Sky.atmospheric(      ); // set atmospheric sky
   if(Kb.bp(KB_3))Sky.skybox     (skybox); // set sky from skybox

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         REPD(x, 3)
         REPD(z, 3)mesh.draw(Matrix(1, Vec(x-1, 0, z-1)*3));

         LightDir(!Vec(1, -1, 1)).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, "Press 1, 2, 3 for different skies");
}
/******************************************************************************/
