/******************************************************************************/
Memc<Actor> actors;
Vehicle     vehicle;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_WORK_IN_BACKGROUND|APP_FULL_TOGGLE;
#ifdef DEBUG
   App.flag|=APP_MEM_LEAKS|APP_BREAKPOINT_ON_ERROR;
#endif
   Cam.dist=10;
   Cam.pitch=-0.4;
   D.viewFov(DegToRad(60)).mode(App.desktopW()*0.8, App.desktopH()*0.8);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   actors.New().create(Plane(Vec(0, -1, 0), Vec(0, 1, 0))); // create ground

   Vehicle.Params params; // setup vehicle params
   flt x=0.9, y=-0.5, z=1.8;
   params.wheel[WHEEL_LEFT_FRONT ].pos.set(-x, y,  z);
   params.wheel[WHEEL_RIGHT_FRONT].pos.set( x, y,  z);
   params.wheel[WHEEL_LEFT_REAR  ].pos.set(-x, y, -z);
   params.wheel[WHEEL_RIGHT_REAR ].pos.set( x, y, -z);
   PhysBody body; body.parts.New().create(Box(2, 1, 4)); body.setBox(); // create vehicle body
   vehicle.create(body, params, 1, 1); // create vehicle

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
   Physics.startSimulation().stopSimulation(); // update physics

   // set vehicle input
   flt forward=Kb.b(KB_W),
       back   =Kb.b(KB_S),
       angle  =Kb.b(KB_D)-Kb.b(KB_A);

   flt accel, brake;
   if(vehicle.speed()<=1 && back>forward) // if moving at slow speed and want to go backward
   {
      accel=-back   ; // set negative acceleration
      brake= forward; // set braking
   }else
   {
      accel=forward;
      brake=back;
   }

   vehicle.angle(angle).accel(accel).brake(brake);

   // setup camera
   Cam.at =vehicle.pos();
   Cam.yaw=Angle(vehicle.matrix().z.xz())-PI_2;
   Cam.setSpherical().updateVelocities(CAM_ATTACH_ACTOR).set();

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   Physics.draw();
   
   // draw wheels
   SetMatrix();
   REP(WHEEL_NUM)
   {
      WHEEL_TYPE wheel =WHEEL_TYPE(i);
      Matrix     matrix=vehicle.wheelMatrix(wheel); matrix.draw();
      Tube(vehicle.wheelRadius(wheel), 0.2, matrix.pos, matrix.x).draw();
   }
}
/******************************************************************************/
