/******************************************************************************/
Mesh box;
Vec  point[32]; // laser can be drawn out of an array of points, this is such an array which will be filled with circle coordinates

// instead of drawing one laser, we will render few of them, each with it's own vertical position
class Laser
{
   flt position_y,
       velocity_y;
}
Laser laser[]=
{
   {0,  0.4},
   {0,  0.6},
   {0,  0.8},
   {0, -0.4},
   {0, -0.6},
   {0, -0.8},
};

ElectricityFx electricity; // electricity effect
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

   // create standard meshes and materials
   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box.parts.New().base.create(Box(4), VTX_TEX0|VTX_NRM|VTX_TAN).reverse();
   box.material(material).setRender().setBox();

   // setup point array for laser
   REPA(point)
   {
      flt angle=Lerp(PI_4, -PI-PI_4, i/flt(Elms(point)-1)); // linearly interpolate angle from 'i' (0..point elements)
      point[i].set(Cos(angle), 0, Sin(angle));              // set point positions on a circle
   }

   // setup electricity effect control points
   electricity.original.New().set(-1, 0, -1);
   electricity.original.New().set( 0, 0,  1);
   electricity.original.New().set( 1, 0, -1);

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
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT)); // move camera on right mouse button

   // update laser positions
   REPA(laser)
   {
      Laser &l=laser[i];

       // update lasers position
       l.position_y+=l.velocity_y*Time.d();

      // if position is too high or to low then change the sign of velocity, so the laser will start moving in a different direction
      if(l.position_y>1 || l.position_y<-1)CHS(l.velocity_y); 
   }

   // update electricity
   electricity.update();

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         box.draw(MatrixIdentity);

         LightPoint(25, Vec(0, 3, 0)).add();
      }break;
   }

   // draw laser and electricity in RM_AMBIENT and RM_SOLID modes
   switch(Renderer())
   {
      case RM_AMBIENT:
      case RM_SOLID  :
      {
         // draw laser
         REPA(laser)
         {
            SetMatrix(Matrix(Vec(0, laser[i].position_y, 0)));                               // setup matrix for the laser
            DrawLaser(Color(128, 0, 0, 128), Color(128, 0, 0, 128), 1, 0.005, false, point); // here alpha determines glow amount
         }

         // draw electricity
         SetMatrix(Matrix(Vec(0, -1.5, 1)));
         electricity.draw();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
