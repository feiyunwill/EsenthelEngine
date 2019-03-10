/******************************************************************************/
Mesh box        , // mesh box
     ball       ; // mesh ball
Vec  ball_pos[8]; // ball positions
flt  mouse_yaw  ; // camera yaw from mouse
bool initialized; // if settings were initialized
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_FULL_TOGGLE;
   Ms.hide();
   Ms.clip(null, 1);

   // apps that use VR must manually initialize it, using either 'OpenVRInit' or 'OculusRiftInit'
#if 1 // initialize using OpenVR SDK
   if(VR.OpenVRInit())
#else // initialize using Oculus Rift SDK
   if(VR.OculusRiftInit())
#endif
   {
      // <- VR initialized
   }
}
/******************************************************************************/
bool Init()
{
   // create meshes
   box  .parts.New().base.create(Box (1   ), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball .parts.New().base.create(Ball(0.15), VTX_TEX0|VTX_NRM|VTX_TAN); // create mesh ball

   // set mesh materials, rendering versions and bounding boxes
   box .material(UID(2123216029, 1141820639, 615850919, 3316401700)).setRender().setBox();
   ball.material(UID(2123216029, 1141820639, 615850919, 3316401700)).setRender().setBox();

   // set random positions
   REPAO(ball_pos)=Random(Box(0.9));

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

   if(!initialized && VR.active())
   {
      initialized=true;
      D.eyeDistance(VR.eyeDistance()); // set recommended Eye Distance
      D.viewFov(VR.fov()); // set recommended Field of View
   }

   if(Kb.bp(KB_R))VR.recenter();

   // set camera based on VR input
   mouse_yaw-=Ms.d().x;
   Vec angles=VR.matrix().angles();
   Cam.at   = VR.matrix().pos*Matrix3().setRotateY(-mouse_yaw);
   Cam.yaw  =-angles.y+mouse_yaw;
   Cam.pitch=-angles.x;
   Cam.roll =-angles.z;
   Cam.dist =0;
   Cam.setSpherical().updateVelocities().set();

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         // solid objects
                       box .draw(MatrixIdentity);
         REPA(ball_pos)ball.draw(Matrix(ball_pos[i]));
         LightSqr(2, Vec(0, 0.8, 0)).add();
      }break;

      case RM_SHADOW:
      {
         REPA(ball_pos)ball.drawShadow(Matrix(ball_pos[i]));
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   if(VR.active())
   {
      D.text(0, 0.8, VR.name());
      D.text(0, 0.7, S+/*VR.res().x+'x'+VR.res().y+", "+*/Round(VR.refreshRate())+"Hz");
      D.text(0, 0.6, "Press 'R' to recenter VR");
   }else
   {
      D.text(0, 0, "VR not detected");
   }
}
/******************************************************************************/
