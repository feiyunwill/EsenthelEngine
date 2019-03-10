/******************************************************************************/
flt hour=5; // current hour
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   D.ambientPower(0.2).mode(App.desktopW()*0.75, App.desktopH()*0.8);

   Cam.dist=1;
   Cam.at.set(16, 4, 2);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .New(UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   // set clouds
   Clouds.layered.set(3, UID(4008816583, 1281666034, 2893213598, 24618113));
   REPAO(Clouds.layered.layer).velocity*=4;

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
   Game.World.update(Cam.at);

   // update hour
   hour+=Time.d()*2;

   // set sun position
   CosSin(Sun.pos.x, Sun.pos.y, hour/24*PI2-PI_2); Sun.pos.z=0;
   Sun.pos*=Matrix3().setRotateY(PI-PI_4);

   // tweak the sun to rise earlier, and set later
   Sun.pos.y+=0.4;
   Sun.pos.normalize();

   // set sun color
   Sun.light_color=(1-D.ambientColor())*Sat(Cbrt(Sun.pos.y));

   // set sun rays and highlight
   flt rays=Sqrt(Sun.pos.y);
   Sun.rays_color     =Lerp(0.02, 0.14, rays);
   Sun.highlight_front=Lerp(0.80, 0.20, rays);

   // set sky color
   flt sky=Lerp(0.13, 1.0, Cbrt(Max(0, Sun.pos.y)));
   Sky.atmosphericHorizonColor(Vec4(Vec(0.32, 0.46, 0.58)*sky, 1.0));
   Sky.atmosphericSkyColor    (Vec4(Vec(0.16, 0.36, 0.54)*sky, 1.0));

   // update clouds
   Clouds.layered.update();
   REP(Clouds.layered.layers())Clouds.layered.layer[i].color=ColorBrightness(sky);

   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw();
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, S+"Hour "+(Trunc(hour)%24));
}
/******************************************************************************/
