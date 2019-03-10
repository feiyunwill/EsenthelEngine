/******************************************************************************/
Mesh box , // mesh box
     ball; // mesh ball
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=3;

   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(3), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN)          ; // create mesh ball

   // set mesh materials, rendering versions and bounding boxes
   box .material(material).setRender().setBox();
   ball.material(material).setRender().setBox();

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
         box .draw(MatrixIdentity);
         ball.draw(MatrixIdentity);

         LightPoint(10, Vec(Cos(Time.time()), Sin(Time.time()), -1.5)).add();
      }break;
      
      // since we want to render shadows we have to process additional rendering mode 'RM_SHADOW' (render shadows through shadow maps)
      // here we will render all meshes which cast shadows
      case RM_SHADOW:
      {
         ball.drawShadow(MatrixIdentity);
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   D.text(0, 0.9, S+"Fps "+Time.fps()); // show number of fps
}
/******************************************************************************/
