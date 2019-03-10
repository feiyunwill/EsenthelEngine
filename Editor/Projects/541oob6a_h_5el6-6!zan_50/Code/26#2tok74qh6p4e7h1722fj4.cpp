/******************************************************************************/
class Obj
{
   Matrix matrix; // matrix
   Vec       vel, // linear  velocity
         ang_vel; // angular velocity
}

Obj  object[12];
Mesh box ,
     ball;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_MAXIMIZABLE;

   Ms.hide();
   Ms.clip(null, 1);

   D.ambientPower(0                   )
    .motionMode  (MOTION_CAMERA_OBJECT)  // enable   motion blur
    .motionScale (0.08                ); // increase motion blur scale
}
/******************************************************************************/
bool Init()
{
   Cam.dist=3;

   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(4  ), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball.parts.New().base.create(Ball(0.2), VTX_TEX0|VTX_NRM|VTX_TAN)          ; // create mesh ball

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
   
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT)); // move camera on right mouse button

   // change motion blur mode when keys pressed
   if(Kb.bp(KB_1))D.motionMode(MOTION_NONE);
   if(Kb.bp(KB_2))D.motionMode(MOTION_CAMERA);
   if(Kb.bp(KB_3))D.motionMode(MOTION_CAMERA_OBJECT);

   // update ball object matrixes and calculate velocity changes automatically
   flt speed=4;
   REPA(object)
   {
      // calculate new matrix
      Vec2   v; CosSin(v.x, v.y, i*PI2/Elms(object) + Time.time()*speed); // calculate sine and cosine of angle
      Matrix new_matrix(Vec(v.x, 0, v.y));                                // create 'new_matrix' with initial position

      // calculate velocity changes according to old and new matrix
      GetVel(object[i].vel, object[i].ang_vel, object[i].matrix, new_matrix);

      // store new matrix
      object[i].matrix=new_matrix;
   }

   return true;
}
/******************************************************************************/
void Render() // rendering method
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         box.draw(MatrixIdentity, Vec(0, 0, 0));                                    // box is rendered with identity matrix and (0,0,0) velocity
         REPA(object)ball.draw(object[i].matrix, object[i].vel, object[i].ang_vel); // draw ball objects with their matrix and velocities

         LightPoint(25, Vec(0, 3, 0)).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, S+"Fps "+Time.fps()); // show number of fps
   D.text(0, 0.8, "Press 1, 2, 3 keys for different Motion Blur modes");
   switch(D.motionMode())
   {
      case MOTION_NONE         : D.text(0, 0.7, "No Motion Blur"  ); break;
      case MOTION_CAMERA       : D.text(0, 0.7, "Camera only"     ); break;
      case MOTION_CAMERA_OBJECT: D.text(0, 0.7, "Camera + Objects"); break;
   }
}
/******************************************************************************/
