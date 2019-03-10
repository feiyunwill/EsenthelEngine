/******************************************************************************/
Mesh box ,
     ball;
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
   Cam.dist=3;

   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(4), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN)          ; // create mesh ball

   // set mesh materials, rendering versions and bounding boxes
   box .material(material).setRender().setBox();
   ball.material(material).setRender().setBox();

   return true;
}
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // move camera on right mouse button
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

         LightPoint(25, Vec(0, 3, 0)).add();
      }break;
   }
}
void Draw()
{
   // render to main viewport
   D.viewRect(null); // set full viewport
   Renderer(Render);

   // render to another viewport
   {
      // set new viewport
      Rect rect(-D.w(), -D.h(), 0, 0); // setup viewport rectangle to left bottom quarter
           rect.extend(-0.05);         // extend with negative value, to make it smaller
      D.viewRect(rect);                // commit new viewport rectangle

      // render everything once again
      Renderer(Render);
   }
}
/******************************************************************************/
