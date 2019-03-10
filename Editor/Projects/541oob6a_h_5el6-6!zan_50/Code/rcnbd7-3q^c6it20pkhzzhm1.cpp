/******************************************************************************

   In this tutorial we'll present multiple viewports with different cameras,
   We'll use 'Viewport.user' to set camera's index

/******************************************************************************/
Viewport viewport[2];
Camera   camera  [2];
/******************************************************************************/
void ViewportDraw(Viewport &viewport) // function wich will be called when drawing the Gui Viewport element
{
   int camera_index=Mid((uintptr)viewport.user, 0, Elms(camera)-1); // get camera index and clamp it for safety, in case the user data is invalid

   // activate camera
   camera[camera_index].set();

   // simple rendering
   D.clear(BLACK);
   SetMatrix(MatrixIdentity);
   Box(1, Vec(0,0,0)).draw(WHITE);
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // initialize cameras
   camera[0].setSpherical(Vec(0,0,0), 0, 0, 0, 3);
   camera[1].setSpherical(Vec(0,0,0), 0, 0, 0, 3);

   Gui+=viewport[0].create(Rect_C(-0.5, 0, 0.9, 0.9), ViewportDraw, (ptr)0); // create a viewport with 'user=0' to mark camera[0] usage
   Gui+=viewport[1].create(Rect_C( 0.5, 0, 0.9, 0.9), ViewportDraw, (ptr)1); // create a viewport with 'user=1' to mark camera[1] usage

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
   Gui.update();
   
   // now we'll modify camera angles and distance
   // first we'll detect which camera to update
   {
      Camera *cam=NULL; // start with none

      // 'Gui.viewport' specifies current Viewport under mouse cursor
      if(Gui.ms()==&viewport[0])cam=&camera[0];else
      if(Gui.ms()==&viewport[1])cam=&camera[1];

      if(cam) // if found a camera
      {
         // modify camera values by mouse movement
         if(Ms.b(0)) // only when mouse button pressed
         {
            cam->yaw  -=Ms.dc().x;
            cam->pitch+=Ms.dc().y;
         }

         // modify camera distance by mouse wheel
         if(Ms.wheel()<0)cam->dist*=1.2;
         if(Ms.wheel()>0)cam->dist/=1.2;

         cam->setSpherical(); // apply changes by calling 'setSpherical' which sets camera matrix from current values
      }
   }

   // update all camera velocities, this is needed when using Motion Blur effect
   REPA(camera)camera[i].updateVelocities();

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(WHITE);
   Gui.draw (     );
}
/******************************************************************************/
