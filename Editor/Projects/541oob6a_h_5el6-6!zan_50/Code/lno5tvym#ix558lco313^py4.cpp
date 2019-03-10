/******************************************************************************/
MeshPtr Earth;
/******************************************************************************/
bool OnWorld(GuiObj *go) {return !go || go==Gui.desktop();}
void Resize(flt old_width=0, flt old_height=0)
{
   D.viewFov(DegToRad(30), (D.w()>D.h()) ? FOV_Y : FOV_X);
}
/******************************************************************************/
void InitPre()
{
   App.flag=APP_MAXIMIZABLE|APP_MINIMIZABLE|APP_RESIZABLE|APP_AUTO_FREE_OPEN_GL_ES_DATA;
   EE_INIT();
   D.ambientPower(0.25);
   D.screen_changed=Resize;
   D.highPrecNrmCalc(true);
   Cam.dist =5;
   Cam.yaw  = LocationLongitude();
   Cam.pitch=-LocationLatitude ();
   Cam.setSpherical();
   Resize();
}
/******************************************************************************/
bool Init()
{
   LocationRefresh(5); // refresh GPS every 5 seconds
   ObjectPtr obj=UID(3453681148, 1260303761, 794833293, 2782304748);
   Earth=obj->mesh();
   Sun.image=UID(1275694243, 1199742097, 1108828586, 1055787228);
   Sun.rays_mode=SUN_RAYS_OFF;
   Sun.light_color=1-D.ambientColor();
   Sky.skybox(UID(540301137, 1134104252, 1741669259, 1349485002));
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

   // rotate and zoom with mouse and touch
   Touch *t[2]={null, null}; int num=0; FREPA(Touches)if(Touches[i].on() && OnWorld(Touches[i].guiObj()) && (Touches[i].stylus() ? Touches[i].life()>0.05 || Touches[i].selecting() : true)){t[num]=&Touches[i]; num++; if(!InRange(num, t))break;} // get first two touches on the screen (for styluses require also minimum time to avoid single tap jitter)
   Vec2   d=0; flt rot=0;
   if(Ms.b(0))d+=Ms.d();
   if(num==1)d+=t[0].ad();else
   if(num==2)
   {
      Vec2 prev_a=t[0].pos()-t[0].d(), cur_a=t[0].pos(),
           prev_b=t[1].pos()-t[1].d(), cur_b=t[1].pos();
      d       +=(t[0].ad()+t[1].ad())/2;
      rot     +=AngleDelta ( Angle(prev_a- prev_b), Angle(cur_a- cur_b));
      Cam.dist*=ScaleFactor((Dist (prev_a, prev_b)- Dist (cur_a, cur_b))*1.7);
   }
   Cam.dist*=ScaleFactor(Ms.wheel()*-0.2);
   Clamp(Cam.dist, 3, 15);
   d*=Cam.dist*0.3;
   Vec gyro=Gyroscope()*Time.ad(); if(0)gyro.xy.chs();
   switch(App.orientation())
   {
      case DIR_RIGHT: gyro.xy.rotate(-PI_2); break;
      case DIR_LEFT : gyro.xy.rotate( PI_2); break;
      case DIR_DOWN : gyro.xy.rotate( PI  ); break;
   }
   Vec axis=Cam.matrix.y*d.x - Cam.matrix.x*d.y - Cam.matrix.z*rot - gyro*Cam.matrix.orn();
   if(flt len=axis.normalize())
   {
      Cam.matrix.orn()*=Matrix3().setRotate(axis, len);
      Vec angles=Cam.matrix.angles();
      Cam.roll =-angles.z;
      Cam.yaw  =-angles.y;
      Cam.pitch=-angles.x;
   }
   Cam.setSpherical().updateVelocities().set();

   DateTime dt; dt.getLocal(); // get local time
   flt hour     =dt.hour+(dt.minute+dt.second/60.0)/60.0; // convert that to hour [0..24)
       hour    -=12; // we want zero sun offset at 12th hour
   flt day_frac =hour/24; // convert that to day fraction (24h per day)
   flt day_angle=day_frac*PI2; // convert that to angle
   flt sun_angle=DegToRad(LocationLongitude())-day_angle;
   CosSin(Sun.pos.x, Sun.pos.z, sun_angle-PI_2); Sun.pos.y=0;

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         if(Earth)Earth->draw(MatrixIdentity);
      }break;
   }
}
void Draw()
{
   Renderer(Render);

   // draw our position
   if(LocationTimeUTC().valid()) // if available
   {
      SetMatrix();
      flt lat=DegToRad(LocationLatitude()), lon=DegToRad(LocationLongitude());
      Vec pos; CosSin(pos.x, pos.z, lon-PI_2);
      flt xz ; CosSin(xz   , pos.y, lat); pos.x*=xz; pos.z*=xz;
      if(!CutsEdgeBall(pos, Cam.matrix.pos, Ball(1-0.01)))pos.draw(RED); // draw only if the position line of sight is not blocked by the earth
   }
}
/******************************************************************************/
