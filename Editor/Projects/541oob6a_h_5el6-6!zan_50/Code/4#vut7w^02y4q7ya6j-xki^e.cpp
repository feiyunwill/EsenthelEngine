/******************************************************************************/
Threads threads;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.5).viewRange(200);

   Cam.dist =1;
   Cam.yaw  =2.8;
   Cam.pitch=0.3;
   Cam.at.set(0, 0, 0);
}
bool Init()
{
   Sky.atmospheric();
   Sun.image=UID(1275694243, 1199742097, 1108828586, 1055787228);
   Sun.light_color=1-D.ambientColor();

   // create background threads
   threads.create(false, Cpu.threads()-1, -1);

   // create cloud image
   Clouds.volumetric.cloud.create(256, 32, 8, &threads);

   return true;
}
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.01, 500, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   VolumetricCloud.Settings settings;
   Clouds.volumetric.cloud.update(settings);
   Clouds.volumetric.pos.x+=Time.d()*0.02;

   return true;
}
/******************************************************************************/
void Render()
{
   /*switch(Renderer())
   {
   }*/
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
